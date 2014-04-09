noinst_LIBRARIES = libfl.a
libfl_a_SOURCES = thread.cpp cond_mutex.cpp time_thread.cpp buffer.cpp buffer.hpp util.cpp read_write_lock.cpp dir.cpp \
  network_buffer.cpp bstring.cpp file.cpp socket.cpp accept_thread.cpp log.cpp \
  event_queue.cpp thread.cpp mutex.cpp event_thread.cpp time.cpp http_event.cpp timer_event.cpp
libfl_a_LIBADD = $(LDADD)
libfl_a_CPPFLAGS = $(AM_CPPFLAGS)


if NEED_MYSQL
  libfl_a_SOURCES += mysql.cpp
  libfl_a_CPPFLAGS += $(MYSQL_INCLUDE)
endif

check_PROGRAMS = libfl_test
libfl_test_SOURCES = tests/test.cpp tests/buffer_test.cpp tests/util_test.cpp tests/dir_test.cpp \
  tests/bstring_test.cpp tests/file_test.cpp tests/socket_test.cpp tests/event_thread_test.cpp tests/thread_test.cpp \
  tests/log_test.cpp tests/event_queue_test.cpp tests/http_event_test.cpp
libfl_test_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_UNIT_TEST_FRAMEWORK_LIB) $(BOOST_FILESYSTEM_LIB) $(MYSQL_LDFLAGS)
libfl_test_LDADD = $(LDADD) libfl.a
libfl_test_CPPFLAGS = $(AM_CPPFLAGS)

if NEED_MYSQL
  libfl_test_SOURCES += tests/mysql_test.cpp
  libfl_test_CPPFLAGS += $(MYSQL_INCLUDE)
endif

TESTS = libfl_test

