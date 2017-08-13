# Build the app test objects
#
CC = g++
CFLAGS  = -g -Wall
STD = -std=c++11
TESTS = transform_test.o device_test.o scene_test.o
INCL_DIRS = -I../../src/model -I../../src/app -I../../src/api

default: $(TESTS)

transform_test.o: transform_test.cpp
	$(CC) $(CFLAGS) -o $@ -c $^ $(STD) $(INCL_DIRS)

device_test.o: device_test.cpp
	$(CC) $(CFLAGS) -o $@ -c $^ $(STD) $(INCL_DIRS)

scene_test.o: scene_test.cpp
	$(CC) $(CFLAGS) -o $@ -c $^ $(STD) $(INCL_DIRS)
