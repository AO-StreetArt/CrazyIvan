# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
CC = g++
CFLAGS  = -g -Wall
STD = -std=c++11
TESTS = configuration_test.o scene_test.o

default: $(TESTS)

configuration_test.o: configuration_test.cpp
	$(CC) $(CFLAGS) -o $@ -c $^ $(STD) -I../src/include

scene_test.o: scene_test.cpp
	$(CC) $(CFLAGS) -o $@ -c $^ $(STD) -I../src/include -I../src/
