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

namespace testLog
{
	const char * const testLog1 = "/tmp/fl_log_test1.txt";
	const char * const testLog2 = "/tmp/fl_log_test2.txt";

	class TestLogSystem : public log::LogSystem
	{
		public:

			TestLogSystem()
				: LogSystem("test_log")
			{
					_targets.push_back(new log::FileTarget(testLog1));
					_targets.push_back(new log::FileTarget(testLog2));
			}
			static bool log(
				const size_t target, 
				const int level, 
				const time_t curTime, 
				struct tm *ct, 
				const char *fmt, 
				va_list args
			)
			{
				static TestLogSystem testLog;
				return testLog._log(target, level, curTime, ct, fmt, args);
			}
	};
	
	using fl::log::Log;
	using namespace fl::log::ELogLevel;
	typedef Log<true, ELogLevel::FATAL, TestLogSystem> Fatal;
	typedef Log<true, ELogLevel::ERROR, TestLogSystem> Error;
	typedef Log<true, ELogLevel::WARNING, TestLogSystem> Warning;
};


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
	BOOST_CHECK_NO_THROW( 
		testLog::Fatal::L("FATAL_TAG_%s_%u_%s\n", "TEST", 1, "STACK");
		testLog::Error::L("FATAL_TAG_%s_%u_%s\n", "TEST", 1, "STACK");
		testLog::Warning::L("FATAL_TAG_%s_%u_%s\n", "TEST", 1, "STACK");
	);
	BOOST_CHECK( fs::file_size(testLog::testLog1) == fs::file_size(testLog::testLog2) );
	unlink(testLog::testLog1);
	unlink(testLog::testLog2);
}

BOOST_AUTO_TEST_SUITE_END()
