///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Dir class unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include "dir.hpp"

using namespace fl::fs;

BOOST_AUTO_TEST_SUITE( fs_Dir )

BOOST_AUTO_TEST_CASE( DirConstructor )
{
	BOOST_CHECK_NO_THROW (
		Directory dir("/tmp");
		BOOST_CHECK(dir.next());
		BOOST_CHECK(dir.name()[0] != 0);

		// check rewind
		dir.rewind();

		BOOST_CHECK(dir.next());
		BOOST_CHECK(dir.name()[0] != 0);
	);
}

BOOST_AUTO_TEST_CASE( makeDirRecursive )
{
	BOOST_CHECK_NO_THROW (	
		BOOST_CHECK(Directory::makeDirRecursive("/tmp/test1/test2"));
		Directory dir("/tmp/test1/test2");
	);
	Directory::rmDirRecursive("/tmp/test1");
	BOOST_CHECK_THROW (	Directory dir("/tmp/test1"), Directory::Error );
}

BOOST_AUTO_TEST_SUITE_END()
