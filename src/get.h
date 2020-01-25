#ifndef LATEX4EE_GET_H
#define LATEX4EE_GET_H

#include <stddef.h>
#include <microhttpd.h>

#include "config.h"
#include "server.h"

int handle_get(CONF_KV_T* config, struct MHD_Connection *connection, const char *url);

#endif
