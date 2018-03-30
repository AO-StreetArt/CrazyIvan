# Build the app test objects
#
CC = g++
CFLAGS  = -g -Wall
STD = -std=c++11
TESTS = base_query_helper_test.o
INCL_DIRS = -I../../src/model -I../../src/app -I../../src/api -I../../src/proc/query

default: $(TESTS)

base_query_helper_test.o: base_query_helper_test.cpp
	$(CC) $(CFLAGS) -o $@ -c $^ $(STD) $(INCL_DIRS)
