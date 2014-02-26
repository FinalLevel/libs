COMPILER=g++
LINKER=g++
make=make

LD=$(PRJ_ROOT)

LIBS=-lpthread -lmysqlclient
LIBDIRS=-L/usr/lib/mysql
INCLUDEDIRS=-I/usr/include/mysql -I../ -I./
CPPFLAGS=-DLINUX -D_THREAD_SAFE

LINK=$(LINKER)
LOBJ=$(LD)/buffer.o $(LD)/util.o $(LD)/read_write_lock.o $(LD)/dir.o $(LD)/network_buffer.o $(LD)/bstring.o $(LD)/file.o $(LD)/socket.o $(LD)/accept_thread.o $(LD)/log.o $(LD)/event_queue.o $(LD)/thread.o $(LD)/mutex.o $(LD)/event_thread.o $(LD)/time.o

CC=$(COMPILER) $(INCLUDEDIRS) -MD -g -Wall -std=c++0x
