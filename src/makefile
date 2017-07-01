# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
CC = g++
CFLAGS  = -g -Wall
STD = -std=c++11
OBJS = configuration_manager.o globals.o ivan_log.o message_processor.o query_helper.o scene.o

default: $(OBJS)

query_helper.o: query_helper.cpp include/query_helper.h
	$(CC) $(CFLAGS) -o $@ -c query_helper.cpp $(STD) -I.

message_processor.o: message_processor.cpp include/message_processor.h query_helper.o
	$(CC) $(CFLAGS) -o $@ -c message_processor.cpp $(STD) -I.

configuration_manager.o: configuration_manager.cpp include/configuration_manager.h
	$(CC) $(CFLAGS) -o $@ -c configuration_manager.cpp $(STD)

scene.o: scene.cpp include/scene.h Scene.pb.cc Scene.pb.h
	$(CC) $(CFLAGS) -o $@ -c scene.cpp $(STD) -I.

globals.o: globals.cpp include/globals.h scene.o configuration_manager.o message_processor.o
	$(CC) $(CFLAGS) -o $@ -c globals.cpp $(STD) -I.

ivan_log.o: ivan_log.cpp include/ivan_log.h
	$(CC) $(CFLAGS) -o $@ -c ivan_log.cpp $(STD)
