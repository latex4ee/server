#include <stdlib.h>
#include <stdint.h>
#include <syslog.h>

#include "config.h"
#include "server.h"

const int MAX_PENDING_CONNECTIONS = 5;

int main(int argc, char const *argv[])
{
#ifdef NDEBUG
	setlogmask(LOG_UPTO(LOG_NOTICE));
	unsigned int syslog_opts = LOG_CONS | LOG_PID | LOG_NDELAY;
#else
	setlogmask(LOG_UPTO(LOG_DEBUG));
	unsigned int syslog_opts = LOG_PERROR | LOG_CONS | LOG_PID | LOG_NDELAY;
#endif
	openlog("latex4ee", syslog_opts, LOG_DAEMON);
	
	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s config-file\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	CONF_KV_T* config = read_config(argv[1]);
	for(CONF_KV_T* kv_tmp = config; INI_KEY_INVALID_KEY != kv_tmp->key; kv_tmp++)
	{
		printf("CONFIG: %s=%s\n", ini_keys_str[kv_tmp->key], kv_tmp->value);
	}
	free(config);

	//server_init(PORT);

	closelog();
	return EXIT_SUCCESS;
}
