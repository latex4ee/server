#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <microhttpd.h>

#include <syslog.h>

#include "server.h"

int print_out_key(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
	printf("%s: %s\n", key, value);
	return MHD_YES;
}

int on_connection(void* cls, struct MHD_Connection *connection, const char *url,
		const char* method, const char* version, const char * upload_data,
		size_t *upload_data_size, void **con_cls)
{
	printf("New %s request for %s using %s\n", method, url, version);

	MHD_get_connection_values( connection, MHD_HEADER_KIND, &print_out_key, NULL);

	struct MHD_Response *response;
	int ret;
	const char * not_found_fmt = 
		"<html><body><b>%s</b> was not found on this server</body></html>\r\n";
	const char * page_fmt = "<hmtl><body>This is <b>%s</b><br/>%s</body></html>\r\n";
	char * page = (char*) malloc(200);
	const char * files[] = { "/", "/index.htm", "/index.html", "/another.html"};
	for (int i = 0; i < 4; i++)
	{
		if (strncmp(url, files[i], 200)==0)
		{
			switch(i)
			{
				case 0: case 1: case 2:
					snprintf(page, 200, page_fmt, "index.html" , 
							"<a href='/another.html'>Here</a> is another page.");
					break;
				case 3:
					snprintf(page, 200, page_fmt, "another.html", "");
					break;
			}
			response = MHD_create_response_from_buffer( 
					strlen(page)
					, (void*)page
					, MHD_RESPMEM_MUST_FREE);
			MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");
			ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
			MHD_destroy_response(response);
			return ret;
		}
	}
	snprintf(page, 200, not_found_fmt, url);
	response = MHD_create_response_from_buffer( strlen(page), (void*)page,
			MHD_RESPMEM_MUST_FREE);
	MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");
	ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
	MHD_destroy_response(response);
	return ret;
}

static int on_client_connect(void *cls, const struct sockaddr *addr, socklen_t addrlen)
{
	char * ip_addr = malloc(INET6_ADDRSTRLEN);
	uint16_t port;
	if(((struct sockaddr_in*)addr)->sin_family == AF_INET)
	{
		ip_addr = (char *)inet_ntop( AF_INET, &((struct sockaddr_in*)addr)->sin_addr
				, ip_addr, (socklen_t)INET_ADDRSTRLEN);
		port = ntohs(((struct sockaddr_in*)addr)->sin_port);
	}
	else if(((struct sockaddr_in6*)addr)->sin6_family == AF_INET6)
    {
    	ip_addr = (char *)inet_ntop( AF_INET6, &((struct sockaddr_in6*)addr)->sin6_addr
		, ip_addr, (socklen_t)INET6_ADDRSTRLEN);
		port = ntohs(((struct sockaddr_in6*)addr)->sin6_port);
	}
	else
	{
		syslog(LOG_WARNING, "sockaddr struct not AF_INET or AF_INET6. Refusing to connect");
		free(ip_addr);
		return MHD_NO;
	}

	printf("Connection from %s:%d\n", ip_addr, port);

	free(ip_addr);
	return MHD_YES;
}

int server_init(uint16_t port)
{
	struct MHD_Daemon *daemon;
	daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, port
			, &on_client_connect, NULL
			, &on_connection, NULL, MHD_OPTION_END);
	if (NULL == daemon)
	{
		syslog(LOG_ERR, "MHD daemon failed to start. Exiting latex4ee server...");
		return EXIT_FAILURE;
	}
	getchar();
	MHD_stop_daemon(daemon);
	return EXIT_SUCCESS;
}
