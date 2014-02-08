PRJ_ROOT=..

include ../Makefile.inc

LOGGER_OBJS=test_logger.o $(LOBJ)


PROGRAMS=$(BIN_PROGRAMS) $(TEST_PROGRAMS)

all: logger

logger: $(LOGGER_OBJS)
	$(LINKER) -g -o test_logger $(LOGGER_OBJS) $(LIBDIRS) $(LIBS)

%.o: %.cpp
	$(CC) -c $< -o $@ $(CPPFLAGS) $(LOGFLAGS)

include $(wildcard *.d)

clean::
	rm -f *.o *.d
	$(make) -C ../ clean
update::
	$(make) -C ../ update


