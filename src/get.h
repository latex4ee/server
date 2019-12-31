#ifndef LATEX4EE_GET_H
#define LATEX4EE_GET_H

#include <stddef.h>
#include <microhttpd.h>

#include "server.h"

int handle_get(
		conn_info_t* cls, struct MHD_Connection *connection, const char *url,
		void **con_cls);

#endif
