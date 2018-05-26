# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
CC = g++
CFLAGS  = -g -Wall
STD = -std=c++11
AOSSL_LIBS = -laossl -lcurl -lneo4j-client -lssl -lcrypto -lm -luuid
POCO_LIBS = -lPocoNetSSL -lPocoCrypto -lPocoNet -lPocoUtil -lPocoFoundation
LIBS = $(AOSSL_LIBS) $(POCO_LIBS)
INCL_DIRS = -I. -I$(CURDIR)/src/
MODEL_SUB_OBJECTS = src/model/scene.o src/model/user_device.o src/model/transform.o
API_SUB_OBJECTS = src/api/json_scene_list.o
QUERY_SUB_OBJECTS = src/proc/query/device_query_helper.o src/proc/query/scene_query_helper.o src/proc/query/algorithm_query_helper.o
PROC_SUB_OBJECTS = src/proc/processor/base_message_processor.o src/proc/processor/crud_message_processor.o src/proc/processor/message_processor.o
SUB_OBJECTS = $(MODEL_SUB_OBJECTS) $(API_SUB_OBJECTS) $(QUERY_SUB_OBJECTS) $(PROC_SUB_OBJECTS)

.PHONY: mksrc test

# -------------------------- Central Targets --------------------------------- #

all: crazy_ivan

mksrc:
	@$(MAKE) -C src

test: mksrc
	@$(MAKE) -C tests

crazy_ivan: mksrc
	$(CC) $(CFLAGS) -o $@ main.cpp $(SUB_OBJECTS) $(LIBS) $(STD) $(INCL_DIRS)

# --------------------------- Clean Project ---------------------------------- #

clean:
	$(RM) crazy_ivan *.o src/*/*.o src/*/*/*.o *~ *.log tests/tests
