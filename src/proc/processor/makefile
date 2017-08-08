# Build the processor classes
#
CC = g++
CFLAGS  = -g -Wall
STD = -std=c++11
OBJS = message_processor.o
INCL_DIRS = -I. -I$(CURDIR)/../app/ -I$(CURDIR)/../api/ -I$(CURDIR)/../model/

default: $(OBJS)

message_processor.o: include/message_processor.h message_processor.cpp query_helper.o
	$(CC) $(CFLAGS) -o $@ -c message_processor.cpp $(STD) $(INCL_DIRS)
