#pragma once
#ifndef __FL_PROGRAM_OPTION_HPP__
#define	__FL_PROGRAM_OPTION_HPP__

///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: A command line arguments manipulation class. It converts command line arguments to the vector of pairs
///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>

namespace fl {
	namespace utils {
		class ProgramOption
		{
		public:
			ProgramOption(const int argc, const char *argv[]);
			struct Option 
			{
				Option(const unsigned char name, const std::string &value);
				unsigned char name;
				std::string value;
			};
			typedef std::vector<Option> TOptionVector;
			const TOptionVector &options() const
			{
				return _options;
			}
		private:

			TOptionVector _options;
			std::string _programName;
		};
	}
};

#endif	// __FL_PROGRAM_OPTION_HPP__

