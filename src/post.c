#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <microhttpd.h>

#include "post.h"
#include "server.h"

static const char * gen_500_str = 
	"<html><body>An internal server error has occurred!</body></html>";
static const char * file_complete_str =
	"<html><body>The upload has been completed.</body></html>";
static const char * file_exists_str =
	"<html><body>That file already exists on this server.</body></html>";

int iterate_post(
		void* con_info_cls, enum MHD_ValueKind kind, const char *key,
		const char* filename, const char* content_type,
		const char* transfer_encoding, const char* data, uint64_t off, size_t size)
{
	FILE* fp;
	conn_info_t* con_info = con_info_cls;
	// Default to reporting internal server error
	con_info->status_str  = gen_500_str;
	con_info->status_code = MHD_HTTP_INTERNAL_SERVER_ERROR;
	if (0 == strcmp(key, "file"))
	{
		if(NULL == con_info->file || 0 == con_info->file)
		{
			if (NULL != (fp = fopen(filename, "rb")))
			{
				fclose(fp);
				con_info->status_str = file_exists_str;
				con_info->status_code = MHD_HTTP_FORBIDDEN;
				return MHD_NO;
			}
			con_info->file = fopen(filename, "ab");
			if (!con_info->file) return MHD_NO;
		}
		if (size > 0)
		{
			if(!fwrite(data, size, sizeof(char), con_info->file))
				return MHD_NO;
		}
		con_info->status_str = file_complete_str;
		con_info->status_code = MHD_HTTP_OK;

		return MHD_YES;
	}
	else // Unrecognised key
	{
		syslog(LOG_WARNING, "Unrecognised POST key ``%s'' send", key);
		return MHD_NO;
	}
}
