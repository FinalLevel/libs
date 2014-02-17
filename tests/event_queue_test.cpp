///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Event queue system unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 

#include "event_queue.hpp"

using boost::test_tools::output_test_stream;

using namespace fl::events;


class TestEvent : public Event
{
public:
	TestEvent(output_test_stream &output)
		: Event(fileno(stdout)), output(output)
	{
		_events = E_OUTPUT;
	}
	virtual const ECallResult call(const TEvents events)
	{
		output << "TestEvent::call";
		return CHANGE;
	}
	output_test_stream &output;
};

BOOST_AUTO_TEST_SUITE( EPollTest )

BOOST_AUTO_TEST_CASE(testEPoll)
{
	EPoll::TEventVector changedEvents;
	EPoll::TEventVector endedEvents;

	BOOST_TEST_MESSAGE("Test epoll creation & test event's call function calling");
	const int QUEUE_LENGTH = 100;
	BOOST_CHECK_NO_THROW (
		EPoll epoll(QUEUE_LENGTH);
		output_test_stream output;
		TestEvent ev(output);
		BOOST_CHECK(epoll.ctrl(&ev) != false);
		BOOST_CHECK(epoll.dispatch(1) != false);
		BOOST_CHECK(epoll.callActive(changedEvents, endedEvents) != false);
		BOOST_CHECK(changedEvents.empty() == false);
		BOOST_CHECK(endedEvents.empty() == true);
		BOOST_CHECK(output.is_equal("TestEvent::call"));
	);
}

BOOST_AUTO_TEST_SUITE_END()
				