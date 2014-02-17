///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Logging system unit tests
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 
#include <boost/filesystem/operations.hpp>

using boost::test_tools::output_test_stream;
namespace fs = boost::filesystem;

#include "log.hpp"

using namespace fl;

BOOST_AUTO_TEST_SUITE( logTest )

BOOST_AUTO_TEST_CASE(testBasicLog)
{
	BOOST_TEST_MESSAGE("Test default loging");
	
	FILE *o = stdout;
	const char * const logFileName =  "/tmp/fl_log_stdout.txt";
	stdout = fopen(logFileName, "w");
	
	BOOST_CHECK(stdout != NULL);

	// simple printf stack test
	log::Fatal::L("FATAL_TAG_%s_%u_%s\n", "TEST", 1, "STACK");
	
	// check info hiding
	const char LONG_HIDDEN_INFO[] = "LOG_INFO_TAG 123234242343434354354365464565465465465465655654654654654654645645654654\n";
	log::Info::L(LONG_HIDDEN_INFO);
	
	fclose(stdout);
	stdout = o;
	BOOST_CHECK( !fs::is_empty(logFileName) );
	BOOST_CHECK( fs::file_size(logFileName) < sizeof(LONG_HIDDEN_INFO));
	
	//unlink(logFileName);
}

BOOST_AUTO_TEST_CASE(testCompositeLog)
{
	log::LogSystem::clearTargets();
	const char * const testLog1 = "/tmp/fl_log_test1.txt";
	const char * const testLog2 = "/tmp/fl_log_test2.txt";
	BOOST_CHECK_NO_THROW( log::LogSystem::addTarget(new log::FileTarget(testLog1)) );
	BOOST_CHECK_NO_THROW( log::LogSystem::addTarget(new log::FileTarget(testLog2)) );
	
	log::Fatal::L("FATAL_TAG_%s_%u_%s\n", "TEST", 1, "STACK");
	log::Error::L("FATAL_TAG_%s_%u_%s\n", "TEST", 1, "STACK");
	log::Warning::L("FATAL_TAG_%s_%u_%s\n", "TEST", 1, "STACK");
	log::LogSystem::clearTargets();
	
	BOOST_CHECK( fs::file_size(testLog1) == fs::file_size(testLog2) );
	unlink(testLog1);
	unlink(testLog2);
}

BOOST_AUTO_TEST_SUITE_END()
