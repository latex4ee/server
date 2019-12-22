#ifndef TEXSERVER_SERVER_H
#define TEXSERVER_SERVER_H

#include <stdint.h>

namespace texserver{
	class Server
	{
		private:
			const int MAX_PENDING_CONNECTIONS = 5;
			
		public:
			Server(uint16_t port);
			~Server();
	};
}

#endif
