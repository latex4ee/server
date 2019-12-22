#ifndef TEXSERVER_HEADER_H
#define TEXSERVER_HEADER_H

namespace texserver{namespace http{
	class Header
	{
		private:
			std::string _name;
			std::string _value;
		public:
			Header();
			~Header();
	};
}}
#endif
