#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include <microhttpd.h>

#include "get.h"
#include "post.h"
#include "server.h"

typedef enum HTTP_METHOD {
	HTTP_GET,
	HTTP_POST
} HTTP_METHOD_T;
const char * HTTP_METHOD_STR[] = {
	"GET", "POST"
};

static const char * greeting_page_fmt = 
	"<html><body><h1>Welcome %s</h1></body></html>";
static const char * err_405_fmt = 
	"<html><body>The method [%s] isn't allowed!</body></html>";
static const char * gen_500_str = 
	"<html><body>An internal server error has occurred!</body></html>";
static const char * busy_str =
	"<html><body>This server is busy, please try again later.</body></html>";
static const char * file_complete_str =
	"<html><body>The upload has been completed.</body></html>";
static const char * file_exists_str =
	"<html><body>That file already exists on this server.</body></html>";
static const char * ask_file_fmt =
"<html>\
<body>\
	Upload a file please. It will be carefully stored.<br/>\
	There are %u clients uploading at the moment.<br/>\
	<form action=\"filepost\" method=\"post\" enctype=\"multipart/form-data\">\
		<input name=\"file\" type=\"file\"/>\
		<input type=\"submit\" value=\" Send \"/>\
	</form>\
</body>\
</html>";
static const size_t MAXCLIENTS = 2;
static const size_t MAXNAMESIZE = 20;
static const size_t MAXANSWERSIZE = 200;
static const size_t POSTBUFFERSIZE = 1024;

static unsigned int n_clients_uploading = 0;

static int send_page (
		struct MHD_Connection* connection, const char * page, int status_code)
{
	int ret;
	struct MHD_Response* response = 
		MHD_create_response_from_buffer( 
				strlen(page), (void*) page, MHD_RESPMEM_MUST_COPY);
	if (!response) return MHD_NO;
	MHD_add_response_header (response, 
			MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");
	ret = MHD_queue_response(connection, status_code, response);
	MHD_destroy_response(response);
	return ret;
}


void request_completed( void* cls, struct MHD_Connection* connection, void **con_cls,
		enum MHD_RequestTerminationCode toe)
{
	conn_info_t* con_info = *con_cls;

	if(NULL == con_info) return;
	if(con_info->conn_type == HTTP_POST)
	{
		if(NULL != con_info->post_processor)
		{
			MHD_destroy_post_processor(con_info->post_processor);
			n_clients_uploading--;
		}
		if(con_info->file)    fclose(con_info->file);
	}
	free(con_info);
	*con_cls = NULL;
}

static int print_out_key(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
	printf("%s: %s\n", key, value);
	return MHD_YES;
}

static int answer_to_connection(
		void* cls, struct MHD_Connection *connection, const char *url,
		const char* method, const char* version, const char * upload_data,
		size_t *upload_data_size, void **con_cls)
{
	printf("New %s request for %s using %s\n", method, url, version);
	//MHD_get_connection_values( connection, MHD_HEADER_KIND, &print_out_key, NULL);

	if ( 0 == strcmp(method, "GET"))
	{
		/*
		char buf[1024];
		sprintf(buf, ask_file_fmt, n_clients_uploading);
		return send_page(connection, buf, MHD_HTTP_OK);
		*/
		return handle_get((conn_info_t*)cls, connection, url, *con_cls);
	}
	if(NULL == *con_cls)
	{
		if (n_clients_uploading >= MAXCLIENTS)
			return send_page(connection, busy_str, MHD_HTTP_SERVICE_UNAVAILABLE);

		conn_info_t* con_info = malloc(sizeof(conn_info_t));
		if (NULL == con_info) return MHD_NO;
		con_info->file = 0;
		// If new request is POST, postprocessor created now
		if (0==strcmp(method, "POST"))
		{
			con_info->post_processor = MHD_create_post_processor(
					connection, POSTBUFFERSIZE, iterate_post, (void*) con_info);
			if (NULL == con_info->post_processor)
			{
				free(con_info);
				return MHD_NO;
			}
			n_clients_uploading++; // Have this specific to the filepost action

			con_info->conn_type = HTTP_POST;
			con_info->status_code = MHD_HTTP_OK;
			con_info->status_str  = file_complete_str;
		}
		else con_info->conn_type = HTTP_GET; // FIXME: is this right?
		*con_cls = (void*) con_info;
		return MHD_YES;
	}

	if ( 0 == strcmp(method, "POST"))
	{
		conn_info_t* con_info = *con_cls;
		if(*upload_data_size != 0)
		{
			MHD_post_process(
					con_info->post_processor, upload_data, *upload_data_size);
			*upload_data_size = 0;
			return MHD_YES;
		}
		else
		return send_page(connection, con_info->status_str, con_info->status_code);
	}
	return send_page(connection, gen_500_str, MHD_HTTP_BAD_REQUEST);
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
	daemon = MHD_start_daemon(
			MHD_USE_INTERNAL_POLLING_THREAD 
			, port
			, &on_client_connect, NULL
			, &answer_to_connection, NULL
			, MHD_OPTION_NOTIFY_COMPLETED, &request_completed, NULL
			, MHD_OPTION_END);
	if (NULL == daemon)
	{
		syslog(LOG_ERR, "MHD daemon failed to start. Exiting latex4ee server...");
		return EXIT_FAILURE;
	}
	getchar();
	MHD_stop_daemon(daemon);
	return EXIT_SUCCESS;
}
