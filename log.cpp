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

thread_local uint32_t CustomLogFields::firstField {0};
thread_local int64_t CustomLogFields::secondField {0};

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
	"I", // INFO
	"D", // DEBUG
};

void StdErrorTarget::log(
	const int level, 
	const char *tag, 
	const time_t curTime, 
	struct tm *ct, 
	const char *fmt, va_list args
) {
	if (_process.service) {
		printf("%s%u:%s %02i.%02i %02i:%02i:%02i TR%08x %s U:%li ", _process.service, _process.serverId, tag,
			ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, CustomLogFields::firstField,
				ErrorLevelTable[level], CustomLogFields::secondField);
	} else {
		fprintf(stderr, "[%s/%u/%02i.%02i %02i:%02i:%02i/%s] ", tag, _process.pid, ct->tm_mday, ct->tm_mon+1,
			ct->tm_hour, ct->tm_min, ct->tm_sec, ErrorLevelTable[level]);
	}
	vfprintf(stderr, fmt, args);
}

void StdErrorTarget::log(
	const int level, 
	const char *fileName,
	const int lineNumber,
	const char *tag, 
	const time_t curTime, 
	struct tm *ct, 
	const char *fmt, 
	va_list args
)
{
	if (_process.service) {
		printf("%s%u:%s %02i.%02i %02i:%02i:%02i TR%08x %s U:%li [%s:%u] ", _process.service, _process.serverId, tag,
			ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, CustomLogFields::firstField,
				ErrorLevelTable[level], CustomLogFields::secondField, fileName, lineNumber);
	} else {
		fprintf(stderr, "[%s:%s:%u/%u/%02i.%02i %02i:%02i:%02i/%s] ", tag, fileName, lineNumber, _process.pid,
			ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, ErrorLevelTable[level]);
	}
	vfprintf(stderr, fmt, args);
}

ScreenTarget::ScreenTarget(const char *service, const uint32_t serverId)
{
	setService(service, serverId);
}

void ScreenTarget::log(
	const int level, 
	const char *tag, 
	const time_t curTime, 
	struct tm *ct, 
	const char *fmt, va_list args
) {
	if (_process.service) {
		printf("%s%u:%s %02i.%02i %02i:%02i:%02i TR%08x %s U:%li ", _process.service, _process.serverId, tag,
			ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, CustomLogFields::firstField,
				ErrorLevelTable[level], CustomLogFields::secondField);
	} else {
		printf("[%s/%u/%02i.%02i %02i:%02i:%02i/%s] ", tag, _process.pid, ct->tm_mday, ct->tm_mon+1,
			ct->tm_hour, ct->tm_min, ct->tm_sec, ErrorLevelTable[level]);
	}
	vprintf(fmt, args);	
}

void ScreenTarget::log(
	const int level, 
	const char *fileName,
	const int lineNumber,
	const char *tag, 
	const time_t curTime, 
	struct tm *ct, 
	const char *fmt, 
	va_list args
)
{
	if (_process.service) {
		printf("%s%u:%s %02i.%02i %02i:%02i:%02i TR%08x %s U:%li [%s:%u] ", _process.service, _process.serverId, tag,
			ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, CustomLogFields::firstField,
				ErrorLevelTable[level], CustomLogFields::secondField, fileName, lineNumber);
	} else {
		printf("[%s:%s:%u/%u/%02i.%02i %02i:%02i:%02i/%s] ", tag, fileName, lineNumber, _process.pid,
			ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, ErrorLevelTable[level]);
	}
	vprintf(fmt, args);		
}

FileTarget::FileTarget(const char *fileName, const char *service, const uint32_t serverId)
{
	setService(service, serverId);
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
	if (_process.service) {
		fprintf(_fd, "%s%u:%s %02i.%02i %02i:%02i:%02i TR%08x %s U:%li ", _process.service, _process.serverId, tag,
			ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, CustomLogFields::firstField,
				ErrorLevelTable[level], CustomLogFields::secondField);
	} else {
		fprintf(_fd, "[%s/%u/%02i.%02i %02i:%02i:%02i/%s] ", tag, _process.pid, ct->tm_mday, ct->tm_mon+1,
			ct->tm_hour, ct->tm_min, ct->tm_sec, ErrorLevelTable[level]);
	}
	vfprintf(_fd, fmt, args);
	fflush(_fd);
}

void FileTarget::log(
	const int level, 
	const char *fileName,
	const int lineNumber,
	const char *tag, 
	const time_t curTime, 
	struct tm *ct, 
	const char *fmt, 
	va_list args
)
{
	if (_process.service) {
		fprintf(_fd, "%s%u:%s %02i.%02i %02i:%02i:%02i TR%08x %s U:%li ", _process.service, _process.serverId, tag,
			ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, CustomLogFields::firstField,
				ErrorLevelTable[level], CustomLogFields::secondField);
	} else {
		fprintf(_fd, "[%s:%s:%u/%u/%02i.%02i %02i:%02i:%02i/%s] ", tag, fileName, lineNumber, _process.pid,
			ct->tm_mday, ct->tm_mon+1, ct->tm_hour, ct->tm_min, ct->tm_sec, ErrorLevelTable[level]);
	}
	vfprintf(_fd, fmt, args);
	fflush(_fd);		
}

LogSystem LogSystem::_defaultLog;

LogSystem::LogSystem()
{
	addTarget(new ScreenTarget());
}

bool LogSystem::log(
	const size_t target, 
	const int level, 
	const char * const tag,
	const time_t curTime, 
	struct tm *ct, 
	const char *fmt, 
	va_list args
) 
{
	if (target < _targets.size()) {
		_targets[target]->log(level, tag, curTime, ct, fmt, args);
		return true;
	}
	else
		return false;
}

bool LogSystem::log(
	const size_t target, 
	const int level, 
	const char *fileName,
	const int lineNumber,
	const char * const tag,
	const time_t curTime, 
	struct tm *ct, 
	const char *fmt, 
	va_list args
) 
{
	if (target < _targets.size()) {
		_targets[target]->log(level, fileName, lineNumber, tag, curTime, ct, fmt, args);
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

void LogSystem::setStdErrorOnly()
{
	clearTargets();
	addTarget(new fl::log::StdErrorTarget());
}
