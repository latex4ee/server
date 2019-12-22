#ifndef TEXSERVER_MESSAGE_H
#define TEXSERVER_MESSAGE_H

#include <list>

#include "Header.h"

namespace texserver { namespace http {
		class Message
		{
			private:
				std::list<Header> general_headers;

			public:
				Message();
				~Message();
		}
	}
}
#endif
