COMPILER=g++
LINKER=g++
make=make

LD=$(PRJ_ROOT)

LIBS=-lpthread -lmysqlclient
LIBDIRS=-L/usr/lib/mysql
INCLUDEDIRS=-I/usr/include/mysql -I../ -I./
CPPFLAGS=-DLINUX -D_THREAD_SAFE

LINK=$(LINKER)
LOBJ=$(LD)/log.o

CC=$(COMPILER) $(INCLUDEDIRS) -g -Wall
