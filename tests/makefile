PROC_TEST_FILES = unit/proc/base_processor_test.cpp unit/proc/base_qhelper_test.cpp
API_TEST_FILES = unit/api/scene_list_test.cpp
MODEL_TEST_FILES = unit/model/device_test.cpp unit/model/scene_test.cpp unit/model/transform_test.cpp
USER_TEST_FILES = unit/user/account_manager_test.cpp
CACHE_TEST_FILES = unit/cache/device_cache_test.cpp integration/cache/cache_loader_test.cpp
TEST_FILES = $(API_TEST_FILES) $(MODEL_TEST_FILES) $(USER_TEST_FILES) $(CACHE_TEST_FILES) $(PROC_TEST_FILES)
INCL_DIRS = -I/usr/local/include -I. -I../src -I../src/model -I../src/api -I../src/cache -I../src/user -I../src/proc/query -I../src/proc/processor
SRC_LOC = ../src/
MODEL_LOC = $(SRC_LOC)/model
API_LOC = $(SRC_LOC)/api
QUERY_LOC = $(SRC_LOC)/proc/query
PROC_LOC = $(SRC_LOC)/proc/processor
MODEL_SUB_OBJECTS = $(MODEL_LOC)/scene.o $(MODEL_LOC)/user_device.o $(MODEL_LOC)/transform.o
API_SUB_OBJECTS = $(API_LOC)/json_scene_list.o
QUERY_SUB_OBJECTS = $(QUERY_LOC)/device_query_helper.o $(QUERY_LOC)/scene_query_helper.o $(QUERY_LOC)/algorithm_query_helper.o
PROC_SUB_OBJECTS = $(PROC_LOC)/base_message_processor.o $(PROC_LOC)/crud_message_processor.o $(PROC_LOC)/message_processor.o
SUB_OBJECTS = $(MODEL_SUB_OBJECTS) $(API_SUB_OBJECTS) $(QUERY_SUB_OBJECTS) $(PROC_SUB_OBJECTS)

tests: test_main.o
	$(CC) $(CFLAGS) test_main.o $(TEST_FILES) $(SUB_OBJECTS) -o $@ $(LIBS) $(STD) $(INCL_DIRS)

test_main.o: test_main.cpp catch.hpp
	$(CC) $(CFLAGS) test_main.cpp -c
