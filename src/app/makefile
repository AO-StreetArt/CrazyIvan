# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
CC = g++
CFLAGS  = -g -Wall
STD = -std=c++11
OBJS = configuration_manager.o globals.o ivan_log.o
INCL_DIRS = -I. -I$(CURDIR)/../app/ -I$(CURDIR)/../api/ -I$(CURDIR)/../model/ -I$(CURDIR)/../proc/processor -I$(CURDIR)/../proc/query

default: $(OBJS)

configuration_manager.o: configuration_manager.cpp include/configuration_manager.h
	$(CC) $(CFLAGS) -o $@ -c configuration_manager.cpp $(STD)

globals.o: globals.cpp include/globals.h configuration_manager.o
	$(CC) $(CFLAGS) -o $@ -c globals.cpp $(STD) $(INCL_DIRS)

ivan_log.o: ivan_log.cpp include/ivan_log.h
	$(CC) $(CFLAGS) -o $@ -c ivan_log.cpp $(STD)
