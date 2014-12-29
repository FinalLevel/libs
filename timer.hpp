#pragma once
#ifndef __FL_TIMER_HPP
#define	__FL_TIMER_HPP

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Timer wrapper class 
///////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <type_traits>

namespace fl {
	namespace chrono {
		class Timer
		{
		public:
			using TClockType = typename std::conditional< std::chrono::high_resolution_clock::is_steady,
				std::chrono::high_resolution_clock,
				std::chrono::steady_clock >::type;
			
			using TMilliseconds = std::chrono::milliseconds;
			
			void reset()
			{
				_startTime = TClockType::now();
			}
			
			TMilliseconds elapsed() const
			{
					return std::chrono::duration_cast<TMilliseconds>(TClockType::now() - _startTime);
			}
			template <typename T, typename Traits>
			friend std::basic_ostream<T, Traits>& operator<<(std::basic_ostream<T, Traits>& out, const Timer& timer)
			{
					return out << timer.elapsed().count();
			}
		private:
			TClockType::time_point _startTime { TClockType::now() };
		};
	};
};

#endif	// __FL_TIMER_HPP
