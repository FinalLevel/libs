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

namespace fl {
	namespace chrono {
		class Time
		{
		public:
			Time();
			const time_t unix() const
			{
				return _unix;
			};
			void update();
			void set(const time_t unix)
			{
				_unix = unix;
			}
		private:
			time_t _unix;
		};
		
	};
};

#endif	// __FL_TIME_HPP
