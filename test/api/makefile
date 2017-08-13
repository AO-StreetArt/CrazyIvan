# Build the app test objects
#
CC = g++
CFLAGS  = -g -Wall
STD = -std=c++11
TESTS = scene_list_test.o
INCL_DIRS = -I../../src/model -I../../src/app -I../../src/api

default: $(TESTS)

scene_list_test.o: scene_list_test.cpp
	$(CC) $(CFLAGS) -o $@ -c $^ $(STD) $(INCL_DIRS)
