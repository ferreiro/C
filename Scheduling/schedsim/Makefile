#!/usr/bin/make
TARGET=schedsim
SOURCES=main.c sched.c slist.c barrier.c  sched_rr.c sched_sjf.c sched_fcfs.c sched_prio.c 

OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
MY_INCLUDES=.
HEADERS=$(wildcard $(MY_INCLUDES)/*.h)
OS=$(shell uname)
LDFLAGS=-lpthread
#CFLAGS=-g -Wall 
CFLAGS=-g -Wall -DPOSIX_BARRIER

all: $(TARGET)
	
$(TARGET): $(OBJECTS)
	gcc  -o  $(TARGET) $(OBJECTS) $(LDFLAGS)

.SUFFIXES:      .o .cpp .h

.h:

.c.o: $(HEADERS)
	gcc  $(CFLAGS)  -I$(MY_INCLUDES) -c $< -o $@  -Wall
	
clean:
	rm -f *.o
	rm -f $(TARGET)
	rm -f  *~
	



