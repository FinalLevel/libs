#pragma once
#ifndef __FL_TIME_HPP
#define	__FL_TIME_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Time functions wrapper class
///////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <ctime>
#include <stdint.h>

namespace fl {
	namespace chrono {
		class Time
		{
		public:
			Time();
			Time(const time_t setTime)
			{
				_unix = setTime;
			}
			const time_t unix() const
			{
				return _unix;
			};
			void update();
			void set(const time_t unix)
			{
				_unix = unix;
			}
			static time_t parseHttpDate(const char *value, const size_t valueLen);
		protected:
			time_t _unix;
		};
		
		class ETime : public Time
		{
		public:
			ETime();
			ETime(const time_t setTime);
			void update();
			const tm &timeStruct() const
			{ 
				return _timeStruct; 
			};
			int yDay() const 
			{ 
				return _timeStruct.tm_yday; 
			};
			int year() const
			{
				return _timeStruct.tm_year;
			};
			int month() const
			{
				return _timeStruct.tm_mon;
			};
			int mDay() const 
			{ 
				return _timeStruct.tm_mday; 
			};
			int wDay() const
			{ 
				return _timeStruct.tm_wday;
			};
			int hour() const
			{ 
				return _timeStruct.tm_hour; 
			};
			int min() const 
			{ 
				return _timeStruct.tm_min; 
			};
			int sec() const 
			{ 
				return _timeStruct.tm_sec; 
			};
			uint32_t tDay() const;
		private:
			struct tm _timeStruct;
		};
	};
};

#endif	// __FL_TIME_HPP
