///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: A logging system facilities
///////////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <unistd.h>
#include <exception>
#include "log.hpp"

using namespace fl::log;

Target::ProcessInfo Target::_process;

Target::ProcessInfo::ProcessInfo()
{
	pid = getpid();
}

const char *ErrorLevelTable[ELogLevel::MAX_LOG_LEVEL] =
{
	NULL, // Unknown level
	"F", // FATAL
	"E", // ERROR
	"W", // WARNING
	" ", // INFO
};

void StdErrorTarget::log(
	const int level, 
	const char *tag, 
	const time_t curTime, 
	struct tm *ct, 
	const char *fmt, va_list args
) {
	fprintf(stderr, "[%s/%u/%02i.%02i %02i:%02i:%02i/%s] ", tag, _process.pid, ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, ErrorLevelTable[level]);
	vfprintf(stderr, fmt, args);
}

void ScreenTarget::log(
	const int level, 
	const char *tag, 
	const time_t curTime, 
	struct tm *ct, 
	const char *fmt, va_list args
) {
	printf("[%s/%u/%02i.%02i %02i:%02i:%02i/%s] ", tag, _process.pid, ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, ErrorLevelTable[level]);
	vprintf(fmt, args);	
}

FileTarget::FileTarget(const char *fileName) 
{
	_fd = fopen(fileName, "a+");
	if (!_fd) {
		printf("Cannot open log file: %s\n", fileName);
		throw std::exception();
	}
}

FileTarget::~FileTarget()
{
	fclose(_fd);
}

void FileTarget::log(
	const int level, 
	const char *tag, 
	const time_t curTime, 
	struct tm *ct, 
	const char *fmt, 
	va_list args
) {
	fprintf(_fd, "[%s/%u/%02i.%02i %02i:%02i:%02i/%s] ", tag, _process.pid, 
					ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, ErrorLevelTable[level]);
	vfprintf(_fd, fmt, args);
	fflush(_fd);
}


bool LogSystem::_log(
	const size_t target, 
	const int level, 
	const time_t curTime, 
	struct tm *ct, 
	const char *fmt, 
	va_list args
) {
	if (target < _targets.size()) {
		_targets[target]->log(level, _tag, curTime, ct, fmt, args);
		return true;
	}
	else
		return false;
}

void LogSystem::addTarget(Target *target)
{
	_targets.push_back(target);
}

void LogSystem::clearTargets()
{
	for (TTargetList::iterator target = _targets.begin(); target != _targets.begin(); target++)
		delete *target;
	_targets.clear();
}
