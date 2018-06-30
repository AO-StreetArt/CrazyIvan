# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
export CC = g++
export CFLAGS  = -g -Wall
export STD = -std=c++11
export AOSSL_LIBS = -laossl -lcurl -lneo4j-client -lssl -lcrypto -lm -luuid
export POCO_LIBS = -lPocoNetSSL -lPocoCrypto -lPocoNet -lPocoUtil -lPocoFoundation
export LIBS = $(AOSSL_LIBS) $(POCO_LIBS) -lboost_system -lpthread
INCL_DIRS = -I. -I$(CURDIR)/src/ -I/usr/local/include
MODEL_SUB_OBJECTS = src/model/scene.o src/model/user_device.o src/model/transform.o
API_SUB_OBJECTS = src/api/json_scene_list.o
QUERY_SUB_OBJECTS = src/proc/query/device_query_helper.o src/proc/query/scene_query_helper.o src/proc/query/algorithm_query_helper.o
PROC_SUB_OBJECTS = src/proc/processor/base_message_processor.o src/proc/processor/crud_message_processor.o src/proc/processor/message_processor.o
SUB_OBJECTS = $(MODEL_SUB_OBJECTS) $(API_SUB_OBJECTS) $(QUERY_SUB_OBJECTS) $(PROC_SUB_OBJECTS)
INSTALL_CONFIG_DIR = /etc/crazyivan
INSTALL_LOG_DIR = /var/log/crazyivan
INSTALL_DIR = /usr/local/bin


.PHONY: mksrc test

# -------------------------- Central Targets --------------------------------- #

build: crazy_ivan

install: install_config install_controller install_service install_executable install_log

	# ------------------------- Build Executable --------------------------------- #

crazy_ivan: mksrc
	$(CC) $(CFLAGS) -o $@ main.cpp $(SUB_OBJECTS) $(LIBS) $(STD) $(INCL_DIRS)

# ----------------------------- Submakes ------------------------------------- #

mksrc:
	@$(MAKE) -C src

test: mksrc
	@$(MAKE) -C tests

# -------------------------- Install Scripts --------------------------------- #

install_config: mk_config_dir
	cp release/app.properties $INSTALL_CONFIG_DIR/app.properties

mk_config_dir:
	mkdir -p $INSTALL_CONFIG_DIR

install_controller:
	cp scripts/linux/ivan_controller.sh $INSTALL_DIR/ivan_controller.sh

install_service:
	cp crazyivan.service /etc/systemd/system/crazyivan.service

install_executable:
	cp crazy_ivan $INSTALL_DIR/crazy_ivan

install_log:
	mkdir /var/log/crazyivan

# --------------------------- Clean Project ---------------------------------- #

clean:
	rm crazy_ivan src/*/*.o src/*/*/*.o

reset: clean
	rm tests/test_main.o

# ------------------------- Uninstall Project -------------------------------- #

uninstall:
	rm $INSTALL_CONFIG_DIR/app.properties $INSTALL_DIR/ivan_controller.sh /etc/systemd/system/crazyivan.service $INSTALL_DIR/crazy_ivan
