///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Event worker threads system unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 

#include <sys/timerfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

#include "event_thread.hpp"
#include "exception.hpp"


using boost::test_tools::output_test_stream;
using namespace fl::events;

const int SOCKET_EVENT = 3;
class WorkTestEvent : public WorkEvent
{
public:
	static int unfinisedEvents;
	WorkTestEvent(const int id, const int descr, output_test_stream *output)
		: WorkEvent(descr, id), output(output), _id(id), _called(false), _finished(false)
	{
		_events = E_OUTPUT;
		unfinisedEvents++;
	}
	virtual ~WorkTestEvent()
	{
		_op = EPOLL_CTL_DEL;
		_thread->ctrl(this);
		unfinisedEvents--;
	}
	virtual const ECallResult call(const TEvents events)
	{
		if (_id == SOCKET_EVENT) {
			if (!_called) {
				_called = true;
				return SKIP;
			}
			else {
				return  FINISHED;
			}
		}
		else {
			if (!_called) {
				_called = true;
				_timeOutTime = time(NULL) + 100;
				return CHANGE;
			}
			else {
				return  FINISHED;
			}
		}	
	}
	virtual bool isFinished()
	{
		
		if (_finished)
		{
			*output << "TestEvent::isFinished: " << _id << ":true\n";
			return true;
		}
		else
		{
			*output << "TestEvent::isFinished: " << _id << ":false\n";
			_finished = true;
			return false;
		}
	}
	output_test_stream *output;
	int _id;
	bool _called;
	bool _finished;
};

class MockTimeEvent : public EPollWorkerGroup::UpdateTimeEvent
{
public:
	MockTimeEvent(output_test_stream *output)
		: output(output)
	{
		itimerspec tm;
		bzero(&tm, sizeof(tm));
		tm.it_value.tv_sec = 0;
		tm.it_value.tv_nsec = 100000000;
		tm.it_interval.tv_sec = 0; // make call every 1/10 second
		tm.it_interval.tv_nsec = 100000000;
		if (timerfd_settime(_descr, 0, &tm, 0))
			throw fl::exceptions::Error("Cannot call  timerfd_settime in UpdateTimeEvent");	
	}
		
	virtual const ECallResult call(const TEvents events)
	{
		static int callTimes = 0;
		if (callTimes < 2)
		{
			*output << "MockTimeEvent::call: " << callTimes << '\n';
			callTimes++;
		}
		uint64_t readBuf;
		read(_descr, &readBuf, sizeof(readBuf));
	
		EPollWorkerGroup::curTime.set(EPollWorkerGroup::curTime.unix() + 1);
		return Event::SKIP;
	}
	output_test_stream *output;
};

int WorkTestEvent::unfinisedEvents = 0;

BOOST_AUTO_TEST_SUITE( WorkerThreadSystem )

BOOST_AUTO_TEST_CASE( WorkerThread )
{
	BOOST_CHECK_NO_THROW (
		const u_int32_t QUEUE_LENGTH = 1000;
		const u_int32_t WORKER_STACK_SIZE = 100000;
		EPollWorkerThread worker(QUEUE_LENGTH, NULL, WORKER_STACK_SIZE);
		
		std::unique_ptr<output_test_stream> output(new output_test_stream);
		BOOST_CHECK( worker.ctrl(static_cast<WorkEvent*>(new MockTimeEvent(output.get()))) != false);

		int fd = socket(PF_INET, SOCK_STREAM, 0);
		BOOST_CHECK(fd > 0 );
		BOOST_CHECK( worker.addConnection(new WorkTestEvent(SOCKET_EVENT, fd, output.get()), NULL) != false);

		BOOST_CHECK( worker.addConnection(new WorkTestEvent(2, fileno(stdout), output.get()), NULL) != false);
		BOOST_CHECK( worker.addConnection(new WorkTestEvent(1, fileno(stdin), output.get()), NULL) != false);
		
		int c = 0;
		while (WorkTestEvent::unfinisedEvents > 0)
		{
			struct timespec tim;
			tim.tv_sec = 0;
			tim.tv_nsec = 100000000;
			nanosleep(&tim , NULL);
			c++;
			BOOST_CHECK(c < 4);
		}
		BOOST_CHECK(output->is_equal(
	"TestEvent::isFinished: 1:false\n"
	"TestEvent::isFinished: 3:false\n"
	"MockTimeEvent::call: 0\n"
	"TestEvent::isFinished: 1:true\n"
	"MockTimeEvent::call: 1\n"));
		worker.cancel();
		worker.waitMe();
	);
}				

BOOST_AUTO_TEST_SUITE_END()