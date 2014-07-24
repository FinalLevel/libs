///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: FileLock class unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include "file_lock.hpp"
#include "util.hpp"

using namespace fl::fs;
using namespace fl::utils;

BOOST_AUTO_TEST_SUITE( fs_FileLock )

BOOST_AUTO_TEST_CASE( FileLockConstuctionDestruction )
{
	static const std::string fileName("/tmp/fl_fs_filock_test");
	static const std::string lockFileName("/tmp/fl_fs_filock_test.lock");
	try
	{
		FileLock lock(fileName.c_str(), 1);
		BOOST_CHECK(fileExists(lockFileName.c_str()));
		BOOST_CHECK_THROW( FileLock lock2(fileName.c_str(), 0), FileLock::Error);
		BOOST_CHECK(lock.checkKey());
	} catch (...) {
		BOOST_CHECK_NO_THROW(throw);
	}
	BOOST_CHECK(fileExists(lockFileName.c_str()) == false);
}

BOOST_AUTO_TEST_SUITE_END()

