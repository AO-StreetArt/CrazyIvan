# Build the processor classes
#
CC = g++
CFLAGS  = -g -Wall
STD = -std=c++11
OBJS = device_query_helper.o scene_query_helper.o algorithm_query_helper.o
INCL_DIRS = -I. -I$(CURDIR)/../../ -I/usr/local/include 

default: $(OBJS)

device_query_helper.o: include/device_query_helper.h device_query_helper.cpp
	$(CC) $(CFLAGS) -o $@ -c device_query_helper.cpp $(STD) $(INCL_DIRS)

scene_query_helper.o: include/scene_query_helper.h scene_query_helper.cpp device_query_helper.o
	$(CC) $(CFLAGS) -o $@ -c scene_query_helper.cpp $(STD) $(INCL_DIRS)

algorithm_query_helper.o: include/algorithm_query_helper.h algorithm_query_helper.cpp scene_query_helper.o
	$(CC) $(CFLAGS) -o $@ -c algorithm_query_helper.cpp $(STD) $(INCL_DIRS)
