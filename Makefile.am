noinst_LIBRARIES = libfl.a
libfl_a_SOURCES = thread.cpp cond_mutex.cpp time_thread.cpp buffer.cpp buffer.hpp util.cpp read_write_lock.cpp dir.cpp \
  network_buffer.cpp bstring.cpp file.cpp socket.cpp accept_thread.cpp log.cpp http_answer.cpp \
  event_queue.cpp thread.cpp mutex.cpp event_thread.cpp time.cpp http_event.cpp timer_event.cpp webdav_interface.cpp \
  nomos.cpp file_lock.cpp program_option.cpp worker_thread.cpp

libfl_a_LIBADD = $(LDADD)
libfl_a_CPPFLAGS = $(AM_CPPFLAGS)


if NEED_MYSQL
  libfl_a_SOURCES += mysql.cpp mysql_pool.cpp
  libfl_a_CPPFLAGS += $(MYSQL_INCLUDE)
endif

if NEED_OPENSSL
  libfl_a_SOURCES += sha1.cpp sha256.cpp
  libfl_a_CPPFLAGS += $(OPENSSL_INCLUDES)
endif

if NEED_SQLITE
  libfl_a_SOURCES += sqlite.cpp
  libfl_a_CPPFLAGS += $(SQLITE3_CFLAGS)
endif

check_PROGRAMS = libfl_test
libfl_test_SOURCES = tests/test.cpp tests/buffer_test.cpp tests/util_test.cpp tests/dir_test.cpp \
  tests/bstring_test.cpp tests/file_test.cpp tests/socket_test.cpp tests/event_thread_test.cpp tests/thread_test.cpp \
  tests/event_queue_test.cpp tests/http_event_test.cpp tests/http_answer_test.cpp \
  tests/webdav_interface_test.cpp tests/time_test.cpp tests/file_lock_test.cpp tests/program_option_test.cpp
libfl_test_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_UNIT_TEST_FRAMEWORK_LIB)  $(MYSQL_LDFLAGS) $(OPENSSL_LDFLAGS) \
  $(SQLITE3_LDFLAGS)
libfl_test_LDADD = $(LDADD) libfl.a $(OPENSSL_LIBS)
libfl_test_CPPFLAGS = $(AM_CPPFLAGS)

if NEED_MYSQL
  libfl_test_SOURCES += tests/mysql_test.cpp
  libfl_test_CPPFLAGS += $(MYSQL_INCLUDE)
endif

if NEED_OPENSSL
  libfl_test_SOURCES += tests/sha1_test.cpp
  libfl_test_CPPFLAGS += $(OPENSSL_INCLUDES)
endif

if NEED_SQLITE
  libfl_test_SOURCES += tests/sqlite_test.cpp
  libfl_test_CPPFLAGS += $(SQLITE3_CFLAGS)
endif

TESTS = libfl_test

