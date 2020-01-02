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
	CONF_KV_T* kv_tmp = config;
	for(; INI_KEY_INVALID_KEY != kv_tmp->key; kv_tmp++)
	{
		printf("KEYVAL STRUCT AT `%p', KEY `%d', VALUE `%s'.\n",
			   	kv_tmp, kv_tmp->key, kv_tmp->value);
		printf("%s:%d=%d\n", __FILE__, __LINE__, kv_tmp->key);
		printf("%s:%d=%s\n", __FILE__, __LINE__, kv_tmp->value);
		printf("%s:%d=%s\n", __FILE__, __LINE__, ini_keys_str[0]);
		printf("%s:%d=%s\n", __FILE__, __LINE__, ini_keys_str[(int)kv_tmp->key]);
		printf("%s:%d=%s\n", __FILE__, __LINE__, ini_keys_str[kv_tmp->key]);
		printf("CONFIG: %s=%s\n", ini_keys_str[kv_tmp->key], kv_tmp->value);
	}
	free(config);

	//server_init(PORT);

	closelog();
	return EXIT_SUCCESS;
}
