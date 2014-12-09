// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: /dev/urandom wrapper class implementation
///////////////////////////////////////////////////////////////////////////////

#include "urandom.hpp"
#include "log.hpp"

using namespace fl::utils;

URandom::URandom()
{
	if (!_fd.open("/dev/urandom", O_RDONLY)) {
		log::Fatal::L("Can't open /dev/urandom\n");
		throw std::exception();
	}
}

bool URandom::getBlock(void *data, const size_t size)
{
	if (_fd.read(data, size) == size) {
		return true;	
	} else {
		return false;
	}
}
