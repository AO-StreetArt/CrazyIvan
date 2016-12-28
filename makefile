# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
CC = g++
SLC = ar rcs
CFLAGS  = -g -Wall
STD = -std=c++11
OBJS = src/Scene.pb.cc src/ivan_log.o src/configuration_manager.o src/globals.o src/scene.o src/query_helper.o src/message_processor.o main.o
RHEL_TESTS = scene_test_rhel configuration_test_rhel
TESTS = scene_test configuration_test
LIBS = -lpthread -llog4cpp
FULL_LIBS = -laossl -lcurl -lpthread -lzmq -lneo4j-client -lssl -lcrypto -lm -llog4cpp -luuid `pkg-config --cflags --libs protobuf hiredis`
RHEL_LIBS = -laossl -lcurl -lpthread -lzmq -lneo4j-client -lssl -lcrypto -lm -llog4cpp -luuid -lhiredis `pkg-config --cflags --libs protobuf`
PROTOC = protoc
PROTO_OPTS = -I=/usr/local/include/dvs_interface

# -------------------------- Central Targets --------------------------------- #

crazy_ivan: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(FULL_LIBS) $(STD)

rhel: $(OBJS)
	$(CC) $(CFLAGS) -o crazy_ivan $(OBJS) $(RHEL_LIBS) $(STD)

test: $(TESTS)

rhel-test: $(RHEL_TESTS)

clean: clean_local clean_tests

# ------------------------------- Tests -------------------------------------- #

scene_test: src/Scene.pb.cc src/ivan_log.o src/scene.o src/scene_test.o
	$(CC) $(CFLAGS) -o $@ $^ $(FULL_LIBS) $(STD)

scene_test_rhel: src/Scene.pb.cc src/ivan_log.o src/scene.o src/scene_test.o
	$(CC) $(CFLAGS) -o scene_test $^ $(RHEL_LIBS) $(STD)

src/scene_test.o: src/test/scene_test.cpp src/scene.cpp src/scene.h /usr/local/include/dvs_interface/Scene.proto
	$(CC) $(CFLAGS) -o $@ -c src/test/scene_test.cpp $(STD)

configuration_test: src/ivan_log.o src/configuration_manager.o src/configuration_test.o
	$(CC) $(CFLAGS) -o $@ $^ $(FULL_LIBS) $(STD)

configuration_test_rhel: src/ivan_log.o src/configuration_manager.o src/configuration_test.o
	$(CC) $(CFLAGS) -o configuration_test $^ $(RHEL_LIBS) $(STD)

src/configuration_test.o: src/test/configuration_test.cpp src/configuration_manager.cpp src/configuration_manager.h
	$(CC) $(CFLAGS) -o $@ -c src/test/configuration_test.cpp $(STD)

# ---------------------------- Main Project ---------------------------------- #

src/ivan_log.o: src/ivan_log.cpp src/ivan_log.h
	$(CC) $(CFLAGS) -o $@ -c src/ivan_log.cpp $(STD)

src/configuration_manager.o: src/configuration_manager.cpp src/configuration_manager.h
	$(CC) $(CFLAGS) -o $@ -c src/configuration_manager.cpp $(STD)

src/Scene.pb.cc: /usr/local/include/dvs_interface/Scene.proto
	$(PROTOC) $(PROTO_OPTS) --cpp_out=src /usr/local/include/dvs_interface/Scene.proto

src/scene.o: src/scene.cpp src/scene.h src/Scene.pb.cc
	$(CC) $(CFLAGS) -o $@ -c src/scene.cpp $(STD)

src/globals.o: src/globals.cpp src/globals.h
	$(CC) $(CFLAGS) -o $@ -c src/globals.cpp $(STD)

src/query_helper.o: src/query_helper.h src/query_helper.cpp
	$(CC) $(CFLAGS) -o $@ -c src/query_helper.cpp $(STD)

src/message_processor.o: src/message_processor.h src/message_processor.cpp
	$(CC) $(CFLAGS) -o $@ -c src/message_processor.cpp $(STD)

main.o: main.cpp src/ivan_utils.h src/uuid.h src/redis_locking.h src/message_processor.h
	$(CC) $(CFLAGS) -o $@ -c main.cpp $(STD)

# --------------------------- Clean Project ---------------------------------- #

clean_local:
	$(RM) crazy_ivan *.o src/*.o *~ *.log *.log.* src/*.pb.cc src/*.pb.h

clean_tests:
	$(RM) *_test
