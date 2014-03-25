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
#include "bstring.hpp"
using namespace fl::fs;
using fl::strings::BString;

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

BOOST_AUTO_TEST_CASE( FileTouch )
{
	srand(time(NULL));
	BString fileName;
	fileName.sprintfSet("/tmp/fl_libs_file_touch_test%u", rand());
	time_t setTime = time(NULL) - 10; // current time - 10 seconds
	BOOST_REQUIRE(File::touch(fileName.data(), setTime));
	struct stat fStat;
	BOOST_REQUIRE(lstat(fileName.data(), &fStat) == 0);
	BOOST_REQUIRE(fStat.st_mtim.tv_sec == setTime);
}

BOOST_AUTO_TEST_CASE( FileTrancate )
{
	File file;
	file.createUnlinkedTmpFile("/tmp");
	file.write("1234", 4);
	
	BOOST_CHECK( file.fileSize() == 4 );
	
	BOOST_CHECK( file.truncate(3) );
	BOOST_CHECK( file.fileSize() == 3 );
}

BOOST_AUTO_TEST_SUITE_END()