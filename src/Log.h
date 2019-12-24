#ifndef TEXSERVER_LOG_H
#define TEXSERVER_LOG_H

#include <initializer_list>
#include <list>

#include "LogLevel.h"
#include "Sink.h"

namespace texserver{namespace log{
	class Log
	{
		private:
			std::list<Sink&> _sinks;
			template <class T>
			void _log_prints(LogLevel loglevel, std::string fmt, std::initializer_list<T> args);

		public:
			template <class T>
			void debug(std::string fmt, std::initializer_list<T> args);
			template <class T>
			void trace(std::string fmt, std::initializer_list<T> args);
			template <class T>
			void info (std::string fmt, std::initializer_list<T> args);
			template <class T>
			void warn (std::string fmt, std::initializer_list<T> args);
			template <class T>
			void error(std::string fmt, std::initializer_list<T> args);
			Log(const char* path, LogLevel min_log_level);
			~Log();
	};
}}

#endif
