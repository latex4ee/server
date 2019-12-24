#ifndef TEXSERVER_SINK_H
#define TEXSERVER_SINK_H

#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include "LogLevel.h"

namespace texserver{ namespace log{
	class Sink
	{
		private:
			std::unique_ptr<std::ofstream> _stream;
			LogLevel _min_log_level;

		public:
			LogLevel getLogLevel() {return _min_log_level;}

			Sink(const std::string& path, LogLevel min_log_level);
			~Sink();

			template <typename T>
			std::ostream& operator<<(const T& data)
			{
				*_stream << "TIME" << "\t" << "LEVEL" << "\t" << data;
				return *_stream;
			}
	};
}}
#endif
