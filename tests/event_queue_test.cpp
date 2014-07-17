///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Event queue system unit tests
///////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <boost/test/unit_test.hpp>

#include "event_queue.hpp"
using namespace fl::events;


class TestEvent : public Event
{
public:
	TestEvent()
		: Event(socket(PF_INET, SOCK_STREAM, 0)), wasCalled(false)
	{
		_events = E_OUTPUT;
	}
	~TestEvent()
	{
		close(_descr);
	}
	virtual const ECallResult call(const TEvents events)
	{
		wasCalled = true;
		return CHANGE;
	}
	bool wasCalled;
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
		TestEvent ev;
		BOOST_CHECK(epoll.ctrl(&ev) != false);
		BOOST_CHECK(epoll.dispatch(1) != false);
		BOOST_CHECK(epoll.callActive(changedEvents, endedEvents) != false);
		BOOST_CHECK(changedEvents.empty() == false);
		BOOST_CHECK(endedEvents.empty() == true);
		BOOST_CHECK(ev.wasCalled == true);
	);
}

BOOST_AUTO_TEST_SUITE_END()
				