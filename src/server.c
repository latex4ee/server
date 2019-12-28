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

#include "server.h"

typedef enum HTTP_METHOD {
	HTTP_GET,
	HTTP_POST
} HTTP_METHOD_T;
const char * HTTP_METHOD_STR[] = {
	"GET", "POST"
};
typedef enum MIMETYPE {
	MIMETYPE_NONE  = 0x00,
	MIMETYPE_PLAIN = 0x01,
	MIMETYPE_HTML  = 0x02,
	MIMETYPE_PNG   = 0x03,
	MIMETYPE_PDF   = 0x04,
	N_MIMETYPES = 5
} MIMETYPE_T;

const char * MIMETYPE_STR[] = {
	"text/plain", "text/plain", "text/html",
	"image/png", "application/pdf"
};

typedef struct file_sig_mimet {
	MIMETYPE_T type;
	const uint8_t signature[8];
	const size_t signature_len;
} file_sig_mimet_t;

const file_sig_mimet_t FILE_SIGS[] = {
	{ MIMETYPE_PNG , { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A /* PNG */}, 8},
	{ MIMETYPE_PDF , {'%', 'P', 'D', 'F', '-' /* PDF */}, 5},
	{ MIMETYPE_NONE, {}, 0}
};

typedef struct conn_info {
	int conn_type;
	char* ans_str;
	struct MHD_PostProcessor* post_processor;
} conn_info_t;

const char * WWW_ROOT = "./mock/";
const int WWW_ROOT_LEN = 7; 

const char* greeting_page_fmt = 
	"<html><body><h1>Welcome %s</h1></body></html>";
const char* err_405_fmt = 
	"<html><body>The method [%s] isn't allowed!</body></html>";
const char* gen_500_str = 
	"<html><body>An internal server error has occurred!</body></html>";

const size_t MAXNAMESIZE = 20;
const size_t MAXANSWERSIZE = 200;
const size_t POSTBUFFERSIZE = 1024;

int print_out_key(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
	printf("%s: %s\n", key, value);
	return MHD_YES;
}

int handle_get(
		void* cls, struct MHD_Connection *connection, const char *url,
		const char* method, const char* version, const char * upload_data,
		size_t *upload_data_size, void **con_cls)
{
	unsigned char *buffer = NULL;
	struct MHD_Response *response;
	int fd, ret = 0;
	struct stat sbuf;

	// Strip url of leading / if it exists
	const char * tmp_url = '/' == *url ? url + 1 : url;
	int urllen = strlen(tmp_url);
	char * file_path = (char *) malloc(urllen+WWW_ROOT_LEN+1);
	strncpy(file_path, WWW_ROOT, WWW_ROOT_LEN);
	strncpy(file_path+WWW_ROOT_LEN, tmp_url, urllen);
	printf("Attempting to serve filepath %s at offset %s (full path: %s)\n",
			tmp_url, WWW_ROOT, file_path);
	if ((-1==(fd=open(file_path, O_RDONLY))) || (0 != fstat(fd, &sbuf)))
	{
		// Error accessing file
		if ( -1 != fd) close(fd);
		response = MHD_create_response_from_buffer(
				strlen(gen_500_str), (void*) gen_500_str, MHD_RESPMEM_PERSISTENT);
		if (response)
		{
			ret = MHD_queue_response( 
					connection, MHD_HTTP_INTERNAL_SERVER_ERROR,	response);
			MHD_destroy_response(response);
			return MHD_YES;
		}
		else
		{
			return MHD_NO;
		}
		if (!ret)
		{
			if (buffer) free(buffer);
			response = MHD_create_response_from_buffer(
				   	strlen(gen_500_str), (void*)gen_500_str, MHD_RESPMEM_PERSISTENT);
			if (response)
			{
				ret = MHD_queue_response (
						connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
				MHD_destroy_response(response);
				return ret;
			} else { return MHD_NO; }
		}
	}
	response = MHD_create_response_from_fd_at_offset (sbuf.st_size, fd, 0);
	uint8_t file_sig[8] = {0};
	size_t nread = read(fd, file_sig, 8);
	int mimetype = MIMETYPE_NONE;

	if(-1 == nread || nread < 8)
	{
		syslog(LOG_WARNING, "Error reading %d bytes from %s. Number read was %lu.\
				Serving file as text/plain",
				8, file_path, nread);
		mimetype = MIMETYPE_HTML;
	}
	for (int i = 0; MIMETYPE_NONE != FILE_SIGS[i].type ; i++)
	{
		int j = 0;
		for (; j < FILE_SIGS[i].signature_len; j++)
		{
			if (file_sig[j] != FILE_SIGS[i].signature[j])
				break;
		}
		if (j == FILE_SIGS[i].signature_len) // match
			mimetype = FILE_SIGS[i].type;
	}
	// FIXME
	mimetype = MIMETYPE_NONE == mimetype ? MIMETYPE_HTML : mimetype;
	MHD_add_response_header (response, 
			MHD_HTTP_HEADER_CONTENT_TYPE, MIMETYPE_STR[mimetype]);
	ret = MHD_queue_response( connection, MHD_HTTP_OK, response);
	MHD_destroy_response( response);
	return ret;
}

static int iterate_post(
		void* con_info_cls, enum MHD_ValueKind kind, const char *key,
		const char* filename, const char* content_type,
		const char* transfer_encoding, const char* data, uint64_t off, size_t size)
{
	conn_info_t* con_info = con_info_cls;
	if (0 == strcmp(key, "name"))
	{
		if((size > 0)&&(size <= MAXNAMESIZE))
		{
			char * ans_str = malloc(MAXANSWERSIZE);
			if(!ans_str) return MHD_NO;

			snprintf(ans_str, MAXANSWERSIZE, greeting_page_fmt, data);
			con_info->ans_str = ans_str;
		}
		else
			con_info->ans_str = NULL;
		return MHD_NO;
	}
	return MHD_YES;
}

void request_completed( void* cls, struct MHD_Connection* connection, void **con_cls,
		enum MHD_RequestTerminationCode toe)
{
	conn_info_t* con_info = *con_cls;

	if(NULL == con_info) return;
	if(con_info->conn_type == HTTP_POST)
	{
		MHD_destroy_post_processor(con_info->post_processor);
		if(con_info->ans_str) free(con_info->ans_str);
	}
	free(con_info);
	*con_cls = NULL;
}

static int answer_to_connection(
		void* cls, struct MHD_Connection *connection, const char *url,
		const char* method, const char* version, const char * upload_data,
		size_t *upload_data_size, void **con_cls)
{
	printf("New %s request for %s using %s\n", method, url, version);
	MHD_get_connection_values( connection, MHD_HEADER_KIND, &print_out_key, NULL);

	if(NULL == *con_cls)
	{
		conn_info_t* con_info = malloc(sizeof(conn_info_t));
		if (NULL == con_info) return MHD_NO;
		con_info->ans_str = NULL;
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
			con_info->conn_type = HTTP_POST;
		}
		else con_info->conn_type = HTTP_GET;
		*con_cls = (void*) con_info;
		return MHD_YES;
	}

	if ( 0 == strcmp(method, "GET"))
	{
		return handle_get(cls, connection, url, method, version, 
				upload_data, upload_data_size, con_cls);
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
		else if (NULL != con_info->ans_str)
		{
			struct MHD_Response *response = MHD_create_response_from_buffer(
				   	strlen(con_info->ans_str), (void*)(con_info->ans_str)
					, MHD_RESPMEM_PERSISTENT);
			int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
			MHD_destroy_response(response);
			return ret;
		}
	}
	size_t pagelen = strlen(err_405_fmt)+10;
	char * page = (char*) malloc(pagelen);
	if(NULL == page)
	{
		syslog(LOG_WARNING,
				"Failed to malloc %lu bytes of memory for 405 error page"
				, pagelen);
		struct MHD_Response *response = MHD_create_response_from_buffer(
				strlen(gen_500_str), (void*) gen_500_str
				, MHD_RESPMEM_PERSISTENT);
		int ret = MHD_queue_response(
				connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
		MHD_destroy_response(response);
		return ret;
	} 
	else
	{
		snprintf(page, pagelen, err_405_fmt, method);
		struct MHD_Response *response = MHD_create_response_from_buffer(
				pagelen, page, MHD_RESPMEM_MUST_FREE);
		int ret = MHD_queue_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED, response);
		MHD_destroy_response(response);
		return ret;
	}
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
