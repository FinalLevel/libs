#pragma once
#ifndef __FL_EXCEPTION_HPP__
#define	__FL_EXCEPTION_HPP__

///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: exeption classes
///////////////////////////////////////////////////////////////////////////////

#include <exception>

namespace fl {
	namespace exceptions {
		
		class Error : public std::exception
		{
		public:
			Error(const char *what)
			 : _what(what)
			{
			}
			virtual const char* what() const throw()
			{
				return _what;
			}
			virtual ~Error() throw() {};
		protected:
			const char *_what;
		};
	};
};

#endif	// __FL_EXCEPTION_HPP__
