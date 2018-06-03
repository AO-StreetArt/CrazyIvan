# Build the processor classes
#
OBJS = json_scene_list.o
INCL_DIRS = -I. -I$(CURDIR)/../ -I/usr/local/include

default: $(OBJS)

json_scene_list.o: json_scene_list.cpp include/json_scene_list.h include/scene_list.h include/scene_list_interface.h
	$(CC) $(CFLAGS) -o $@ -c json_scene_list.cpp $(STD) $(INCL_DIRS)
