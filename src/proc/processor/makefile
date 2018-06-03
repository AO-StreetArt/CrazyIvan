# Build the processor classes
#
OBJS = base_message_processor.o crud_message_processor.o message_processor.o
INCL_DIRS = -I. -I$(CURDIR)/../../ -I/usr/local/include

default: $(OBJS)

base_message_processor.o: include/base_message_processor.h base_message_processor.cpp
	$(CC) $(CFLAGS) -o $@ -c base_message_processor.cpp $(STD) $(INCL_DIRS)

crud_message_processor.o: include/crud_message_processor.h crud_message_processor.cpp base_message_processor.o
	$(CC) $(CFLAGS) -o $@ -c crud_message_processor.cpp $(STD) $(INCL_DIRS)

message_processor.o: include/message_processor.h message_processor.cpp crud_message_processor.o
	$(CC) $(CFLAGS) -o $@ -c message_processor.cpp $(STD) $(INCL_DIRS)
