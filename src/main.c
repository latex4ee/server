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
	long l;
	for(const CONF_KV_T* kv_tmp = config; INI_KEY_INVALID_KEY != kv_tmp->key; kv_tmp++)
	{
		printf("CONFIG: %s=", ini_keys_str[kv_tmp->key]);
		switch(kv_tmp->key)
		{
			case INI_KEY_PORT:
				if(0 >= (l = config_lookup_key_long(config, kv_tmp->key)))
					break;
				printf("%ld", l);
				break;
			default:
				printf("%s", kv_tmp->value);
				break;
		}
		printf("\n");
	}

	server_init(config);
	
	free(config);

	closelog();
	return EXIT_SUCCESS;
}
