///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: A command line arguments manipulation class implementation. 
// It converts command line arguments to the vector of pairs.
///////////////////////////////////////////////////////////////////////////////

#include "program_option.hpp"

using namespace fl::utils;

ProgramOption::ProgramOption(const int argc, const char * const argv[])
{
	if (argc == 0) {
		return;
	}
	_programName = argv[0];
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') { // found option name
			int next = i + 1;
			if ((next >= argc) || (argv[next][0] == '-')) {
				_options.push_back(Option(argv[i][1], ""));
			} else if (next < argc) {
				_options.push_back(Option(argv[i][1], std::string(argv[next])));
				++i;
			}
			continue;
		}
		_options.push_back(Option(' ', std::string(argv[i])));
	}
}

ProgramOption::Option::Option(const unsigned char name, const std::string &value)
	: name(name), value(value)
{

}
