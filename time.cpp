///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Time functions wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <strings.h>
#include <string.h>
#include <ctype.h>
#include "time.hpp"


using namespace fl::chrono;

Time::Time()
	: _unix(time(NULL))
{
	
}

void Time::update()
{
	_unix = time(NULL);
}

time_t Time::parseHttpDate(const char *value, const size_t valueLen)
{
	//Fri, 23 Apr 2014 19:55:07 GMT
	struct tm tm;
	bzero(&tm, sizeof(struct tm));

	const char *p = value;
	const char *endValue = value + valueLen;
	while (p < endValue) {
		if ((*p != ' ') && (*p != '\t'))
			break;
		p++;
	}
	if (p >= endValue)
		return 0;
	p = (char*)memchr(p, ',', endValue - p);
	if (!p)
		return 0;
	
	p++;
	char *endP = NULL;
	tm.tm_mday = strtoul(p, &endP, 10);
	if (endP >= endValue)
		return 0;
	p = endP + 1;
	endP = (char*)memchr(p, ' ', endValue - p);
	if (!endP || (endP >= endValue))
		return 0;
	switch (tolower(*p))
	{
	case 'j':
		if (tolower(*(p + 1)) == 'a')
			tm.tm_mon = 0;
		else if (tolower(*(p + 2)) == 'n')
			tm.tm_mon = 5;
		else if (tolower(*(p + 2)) == 'l')
			tm.tm_mon = 6;
	break;
	case 'f':
		tm.tm_mon = 1;
	break;
	case 'm':
		if (tolower(*(p + 2)) == 'r')
			tm.tm_mon = 2;
		else if (tolower(*(p + 2)) == 'y')
			tm.tm_mon = 4;
	break;
	case 'a':
		if (tolower(*(p + 1)) == 'p')
			tm.tm_mon = 3;
		else if (tolower(*(p + 1)) == 'u')
			tm.tm_mon = 7;
	break;
	case 's':
		tm.tm_mon = 8;
	break;
	case 'o':
		tm.tm_mon = 9;
	break;
	case 'n':
		tm.tm_mon = 10;
	break;
	case 'd':
		tm.tm_mon = 11;
	break;
	default:
		return 0;
	};

	p = endP + 1;
	endP = NULL;
	tm.tm_year = strtoul(p, &endP, 10) - 1900;
	if (!endP || (endP >= endValue))
		return 0;
	p = endP + 1;
	endP = NULL;
	tm.tm_hour = strtoul(p, &endP, 10);
	if (!endP || (endP >= endValue))
		return 0;
	p = endP + 1;
	endP = NULL;
	tm.tm_min = strtoul(p, &endP, 10);
	if (!endP || (endP >= endValue))
		return 0;
	p = endP + 1;
	endP = NULL;
	tm.tm_sec = strtoul(p, &endP, 10);
	if (!endP || (endP >= endValue))
		return 0;
	p = endP + 1;

	tm.tm_isdst = -1;
	
#ifndef __CYGWIN__
	int lastLength = endValue - p;
	if (lastLength == 3) {
		if (!memcmp(p, "GMT", 3))
			tm.tm_zone = "GMT";
		else if (!memcmp(p, "UTC", 3))
			tm.tm_zone = "UTC";
	}
#else
#warning "Error: HAVE_TM_ZONE not supported"
#endif
	return timegm(&tm);
}

ETime::ETime()
{
	update();
}

void ETime::update()
{
	Time::update();
	localtime_r(&_unix, &_timeStruct);	
}