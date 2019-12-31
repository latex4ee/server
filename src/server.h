#ifndef LATEX4EE_SERVER_H
#define LATEX4EE_SERVER_H

#include <stdint.h>
#include <stdio.h>

typedef struct conn_info {
	int conn_type;
	FILE * file;
	int    status_code;
	char * status_str;
	struct MHD_PostProcessor* post_processor;
} conn_info_t;

int server_init(uint16_t port);

#endif
