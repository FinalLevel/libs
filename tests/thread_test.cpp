///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Thread class unit tests
///////////////////////////////////////////////////////////////////////////////


#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 

#include "thread.hpp"

using boost::test_tools::output_test_stream;

using namespace fl::threads;


BOOST_AUTO_TEST_SUITE( ThreadTest )

class TestThread : public Thread
{
public:
	TestThread(output_test_stream &output)
		: output(output)
	{
		const int STACK_SIZE = 100000;
		setStackSize(STACK_SIZE);
		if (!create())
			throw std::exception();
	}
	virtual void run()
	{
		output << "TestThread::run";
	}
	output_test_stream &output;
};



BOOST_AUTO_TEST_CASE(threadCreationAndRunTest)
{
	BOOST_TEST_MESSAGE("Test Thread creation & run process");
	BOOST_CHECK_NO_THROW (
		output_test_stream output;
		TestThread thread(output);
		thread.waitMe();
		BOOST_CHECK(output.is_equal("TestThread::run"));
	);
}

BOOST_AUTO_TEST_SUITE_END()