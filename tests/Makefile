PRJ_ROOT=..

include ../Makefile.inc

OPFLAGS = -O3
LOG_OBJ=log_test.o $(LOBJ)
BOST_FORMAT_TEST=boost_format_test.o
EVENT_QUEUE_OBJ=event_queue_test.o $(LOBJ)

TEST_LIBS=-lboost_unit_test_framework-mt -lboost_filesystem-mt

all: log event_queue
	./log_test
	./event_queue

log: $(LOG_OBJ)
	$(LINKER) -g $(OPFLAGS) -o log_test $(LOG_OBJ) $(LIBDIRS) $(LIBS) $(TEST_LIBS) 

event_queue: $(EVENT_QUEUE_OBJ)
	$(LINKER) -g $(OPFLAGS) -o event_queue $(EVENT_QUEUE_OBJ) $(LIBDIRS) $(LIBS) $(TEST_LIBS) 

boost_format_test: $(BOST_FORMAT_TEST)
	$(LINKER) -g $(OPFLAGS) -o boost_format_test $(BOST_FORMAT_TEST) $(LIBDIRS) $(LIBS)

%.o: %.cpp
	$(CC) -c $< -o $@ $(CPPFLAGS) $(OPFLAGS)

include $(wildcard *.d)

clean::
	rm -f *.o *.d
	$(make) -C ../ clean
update::
	$(make) -C ../ update


