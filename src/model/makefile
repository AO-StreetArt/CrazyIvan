# Build the model classes
#
OBJS = transform.o user_device.o scene.o
INCL_DIRS = -I. -I$(CURDIR)/../ -I/usr/local/include

default: $(OBJS)

transform.o: transform.cpp include/transform.h
	$(CC) $(CFLAGS) -o $@ -c transform.cpp $(STD) $(INCL_DIRS)

user_device.o: user_device.cpp include/user_device.h transform.o
	$(CC) $(CFLAGS) -o $@ -c user_device.cpp $(STD) $(INCL_DIRS)

scene.o: scene.cpp include/scene.h transform.o user_device.o
	$(CC) $(CFLAGS) -o $@ -c scene.cpp $(STD) $(INCL_DIRS)
