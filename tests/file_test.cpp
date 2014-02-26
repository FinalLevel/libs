///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Final Level
// Author: Denys Misko <gdraal@gmail.com>
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: File class unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 

#include "file.hpp"

using namespace fl::fs;

BOOST_AUTO_TEST_SUITE( IO )

BOOST_AUTO_TEST_CASE( FileOpenRead )
{
	File file;
	BOOST_CHECK(file.open("/dev/zero", O_RDONLY));
	const ssize_t MAX_BUF_SIZE = 4;
	char buf[MAX_BUF_SIZE];
	BOOST_CHECK(file.read(buf, MAX_BUF_SIZE) == MAX_BUF_SIZE);
}

BOOST_AUTO_TEST_CASE( FileCreateTempWriteRead )
{
	File file;
	
	BOOST_CHECK( file.createUnlinkedTmpFile("/tmp") );
}

BOOST_AUTO_TEST_CASE( FileSize )
{
	File file;
	file.createUnlinkedTmpFile("/tmp");
	file.write("1234", 4);
	
	BOOST_CHECK( file.fileSize() == 4 );
}

BOOST_AUTO_TEST_SUITE_END()