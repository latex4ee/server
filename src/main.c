#include <stdlib.h>
#include <stdint.h>
#include <syslog.h>

#include "server.h"

const uint16_t PORT = 8080;
const int MAX_PENDING_CONNECTIONS = 5;

int main(int argc, char const *argv[])
{
#ifdef NDEBUG
	setlogmask(LOG_UPTO(LOG_NOTICE));
	int syslog_opts = LOG_CONS | LOG_PID | LOG_NDELAY;
#else
	setlogmask(LOG_UPTO(LOG_DEBUG));
	int syslog_opts = LOG_PERROR | LOG_CONS | LOG_PID | LOG_NDELAY;
#endif
	openlog("latex4ee", syslog_opts, LOG_DAEMON);
	
	server_init(PORT);

	closelog();
	return EXIT_SUCCESS;
}
