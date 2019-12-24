#include "Log.h"

using namespace texserver::log;
Log::Log(const char* path, LogLevel min_log_level)
{
	_sinks.emplace_back(Sink(path, min_log_level));
}

Log::~Log(){}

template <class T>
void Log::_log_prints(LogLevel loglevel, std::string fmt, std::initializer_list<T> args){
	for ( auto s : _sinks) {
		if(s.getLogLevel() > loglevel)
			s << fmt << args;
	}

}
template <class T>
void Log::debug(std::string fmt, std::initializer_list<T> args){_log_prints(LogLevel::debug, fmt, args);}
template <class T>
void Log::trace(std::string fmt, std::initializer_list<T> args){_log_prints(LogLevel::debug, fmt, args);}
template <class T>
void Log::info (std::string fmt, std::initializer_list<T> args){_log_prints(LogLevel::debug, fmt, args);}
template <class T>
void Log::warn (std::string fmt, std::initializer_list<T> args){_log_prints(LogLevel::debug, fmt, args);}
template <class T>
void Log::error(std::string fmt, std::initializer_list<T> args){_log_prints(LogLevel::debug, fmt, args);}
