#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>

#include <microhttpd.h>

#include "get.h"

// Improve these
static const char * gen_500_str = 
	"<html><body>An internal server error has occurred!</body></html>";
const char * WWW_ROOT = "./mock/";
const int WWW_ROOT_LEN = 7; 

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

int handle_get(
		conn_info_t* cls, struct MHD_Connection *connection, 
		const char *url, void **con_cls)
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
