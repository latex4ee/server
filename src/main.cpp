#include <stdlib.h>
#include <stdint.h>

#include "Server.h"
#include "Log.h"

const uint16_t PORT = 8080;
const int MAX_PENDING_CONNECTIONS = 5;

using namespace texserver;

int main(int argc, char const *argv[])
{
	log::Log logger = log::Log("logs/status.log", log::LogLevel::trace);
	Server s = Server(PORT);

	return EXIT_SUCCESS;
}
