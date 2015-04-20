///////////////////////////////////////////////////////////////////////////////
//
// Copyright Denys Misko <gdraal@gmail.com>, Final Level, 2014.
// Distributed under BSD (3-Clause) License (See
// accompanying file LICENSE)
//
// Description: Event worker threads system unit tests
///////////////////////////////////////////////////////////////////////////////

#include <boost/test/unit_test.hpp>

#include <sys/timerfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <vector>

#include "event_thread.hpp"
#include "exception.hpp"


using namespace fl::events;

typedef std::vector<int> TIDList;

const int STDIN_EVENT = 1;
const int STDOUT_EVENT = 2;
const int SOCKET_EVENT = 3;

TIDList finishedTrueCalls;
TIDList finishedFalseCalls;

class WorkTestEvent : public WorkEvent
{
public:
	static int unfinisedEvents;
	WorkTestEvent(const int id, const int descr)
		: WorkEvent(descr, id), _id(id), _called(false), _finished(false)
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
		else  if (_id == STDOUT_EVENT) {
			if (!_called) {
				_called = true;
				_timeOutTime = time(NULL) + 100;
				return CHANGE;
			}
			else {
				return  FINISHED;
			}
		}	
		else
			return SKIP;
	}
	virtual bool isFinished()
	{
		if (_finished)
		{
			finishedTrueCalls.push_back(_id);
			return true;
		}
		else
		{
			finishedFalseCalls.push_back(_id);
			_finished = true;
			return false;
		}
	}
	int _id;
	bool _called;
	bool _finished;
};

class MockTimeEvent : public EPollWorkerGroup::UpdateTimeEvent
{
public:
	static const int TIC_TIME = 10000000;
	MockTimeEvent()
	{
		itimerspec tm;
		bzero(&tm, sizeof(tm));
		tm.it_value.tv_sec = 0;
		tm.it_value.tv_nsec = TIC_TIME;
		tm.it_interval.tv_sec = 0; // make call every 1/10 second
		tm.it_interval.tv_nsec = TIC_TIME;
		if (timerfd_settime(_descr, 0, &tm, 0))
			throw fl::exceptions::Error("Cannot call  timerfd_settime in UpdateTimeEvent");	
	}
	
	static int callTimes;
	virtual const ECallResult call(const TEvents events)
	{
		callTimes++;
		
		uint64_t readBuf;
		read(_descr, &readBuf, sizeof(readBuf));
	
		EPollWorkerGroup::curTime.set(EPollWorkerGroup::curTime.unix() + 1);
		return Event::SKIP;
	}
};

int MockTimeEvent::callTimes = 0;
int WorkTestEvent::unfinisedEvents = 0;

BOOST_AUTO_TEST_SUITE( WorkerThreadSystem )

BOOST_AUTO_TEST_CASE( WorkerThread )
{
	try
	{
		const u_int32_t QUEUE_LENGTH = 1000;
		const u_int32_t WORKER_STACK_SIZE = 100000;
		EPollWorkerThread worker(QUEUE_LENGTH, NULL, WORKER_STACK_SIZE);
		
		BOOST_CHECK( worker.ctrl(new MockTimeEvent()) != false);

		int fd = socket(PF_INET, SOCK_STREAM, 0);
		BOOST_CHECK(fd > 0 );
		BOOST_CHECK( worker.addConnection(new WorkTestEvent(SOCKET_EVENT, fd), NULL) != false);
		BOOST_CHECK( worker.addConnection(new WorkTestEvent(STDIN_EVENT, fileno(stdin)), NULL) != false);
		

		struct timespec tim;
		tim.tv_sec = 0;
		tim.tv_nsec = MockTimeEvent::TIC_TIME * 4;
		nanosleep(&tim , NULL);
	
			// check allocation - deallocation
		BOOST_CHECK(WorkTestEvent::unfinisedEvents == 0);
		
		// check timer calling
		BOOST_CHECK(MockTimeEvent::callTimes >= 2);
		
		// check isFinish order
		BOOST_CHECK(finishedFalseCalls.size() == 2);
		//BOOST_CHECK((finishedFalseCalls[0] == STDIN_EVENT) && (finishedFalseCalls[1] == SOCKET_EVENT));
		BOOST_CHECK(finishedTrueCalls.size() == 1);
		BOOST_CHECK(finishedTrueCalls[0] == STDIN_EVENT);
		worker.finish();
		worker.cancel();
		worker.waitMe();	
		close(fd);

	} catch (...) {
		BOOST_CHECK_NO_THROW(throw);
	}
}				

BOOST_AUTO_TEST_SUITE_END()