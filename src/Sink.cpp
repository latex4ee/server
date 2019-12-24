#include "Sink.h"

using namespace texserver::log;

Sink::Sink(const std::string& path, LogLevel min_log_level)
{
	_stream = std::make_unique<std::ofstream>(std::ofstream(path));
	_min_log_level = min_log_level;
}
Sink::~Sink(){}

