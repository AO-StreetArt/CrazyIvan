# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
CC = g++
SLC = ar rcs
CFLAGS  = -g -Wall
STD = -std=c++11
OBJS = src/Scene.pb.cc src/ivan_log.o src/configuration_manager.o src/globals.o src/scene.o src/query_helper.o src/message_processor.o main.o
TESTS = scene_test configuration_test
LIBS = -lpthread -llog4cpp
FULL_LIBS = -laossl -lcurl -lpthread -lzmq -lneo4j-client -lssl -lcrypto -lm -llog4cpp -luuid -lhiredis `pkg-config --cflags --libs protobuf`
PROTOC = protoc
PROTO_OPTS = -I=/usr/local/include/dvs_interface

.PHONY: mksrc mktest

# -------------------------- Central Targets --------------------------------- #

all: mksrc main.o crazy_ivan

mksrc: src/api/Scene.pb.cc
	@$(MAKE) -C src

main.o: main.cpp
	$(CC) $(CFLAGS) -o $@ -c main.cpp $(STD) -Isrc/

crazy_ivan:
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(FULL_LIBS) $(STD)

src/api/Scene.pb.cc: /usr/local/include/dvs_interface/Scene.proto
	$(PROTOC) $(PROTO_OPTS) --cpp_out=src/api /usr/local/include/dvs_interface/Scene.proto
	mv src/api/Scene.pb.h src/api/include

# ------------------------------- Tests -------------------------------------- #

test: mktest $(TESTS)

mktest:
	@$(MAKE) -C test

scene_test: src/Scene.pb.cc src/ivan_log.o src/scene.o test/scene_test.o
	$(CC) $(CFLAGS) -o $@ $^ $(FULL_LIBS) $(STD)

configuration_test: src/ivan_log.o src/configuration_manager.o test/configuration_test.o
	$(CC) $(CFLAGS) -o $@ $^ $(FULL_LIBS) $(STD)

# --------------------------- Clean Project ---------------------------------- #

clean:
	$(RM) crazy_ivan *.o src/*/*.o src/*/*/*.o *~ *.log *.log.* src/*/*.pb.cc src/*/*/*.pb.h *_test test/*.o
