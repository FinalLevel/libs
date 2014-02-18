PRJ_ROOT=..

include ../Makefile.inc

OPFLAGS = 
TEST_OBJ=socket_test.o event_thread_test.o thread_test.o log_test.o event_queue_test.o test.o $(LOBJ)
BOST_FORMAT_TEST=boost_format_test.o

TEST_LIBS=-lboost_unit_test_framework-mt -lboost_filesystem-mt

all: test
	./test

test: $(TEST_OBJ)
	$(LINKER) -g $(OPFLAGS) -o test $(TEST_OBJ) $(LIBDIRS) $(LIBS) $(TEST_LIBS) 

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


