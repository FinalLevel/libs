noinst_LIBRARIES = libfl.a
libfl_a_SOURCES = thread.cpp cond_mutex.cpp time_thread.cpp buffer.cpp buffer.hpp util.cpp read_write_lock.cpp dir.cpp \
  network_buffer.cpp bstring.cpp file.cpp socket.cpp accept_thread.cpp log.cpp \
  event_queue.cpp thread.cpp mutex.cpp event_thread.cpp time.cpp

check_PROGRAMS = libfl_test
libfl_test_SOURCES = tests/test.cpp tests/buffer_test.cpp tests/util_test.cpp tests/dir_test.cpp \
  tests/bstring_test.cpp tests/file_test.cpp tests/socket_test.cpp tests/event_thread_test.cpp tests/thread_test.cpp \
  tests/log_test.cpp tests/event_queue_test.cpp
libfl_test_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(BOOST_FILESYSTEM_LIB)
libfl_test_LDADD = $(LDADD) libfl.a

TESTS = libfl_test

