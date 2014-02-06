# Makefile for CPE464 library

CC = g++
CFLAGS = -g -Wall

OS = $(shell uname -s)
ifeq ("$(OS)", "SunOS")
	LIBS += -lsocket -lnsl
endif

PACKAGES = sendtoErr sendErr checksum
HDRS = $(shell cd networks && ls *.hpp *.h 2> /dev/null)
SRCS = $(shell find . -name "*.cpp" -o -name "*.c" 2> /dev/null)
OBJS = $(shell find . -name "*.cpp" -o -name "*.c" 2> /dev/null | sed s/\.c[p]*$$/\.o/ )
LIBNAME = cpe464

V=@

include version.mk

all: header $(OBJS) link combHeader clean
	@echo "-------------------------------"

partial: header $(OBJS)
	@echo "-------------------------------"

echo:
	@echo $(OBJS)

version:
	@echo $(BUILD_MAJOR).$(BUILD_MINOR) 

header:
	@echo "-------------------------------"
	@echo "Building $(LIBNAME) Objects"

.cpp.o:
	@echo "-------------------------------"
	@echo "  Building $@"
	$(V)$(CC) -c $(CFLAGS) $< -o $@ $(LIBS)
.c.o:
	@echo "-------------------------------"
	@echo "  Building $@"
	$(V)$(CC) -c $(CFLAGS) $< -o $@ $(LIBS)

link:
	@echo "-------------------------------"
	@echo "Linking CPE464 library"
	$(V)ar -rcv ../lib$(LIBNAME).$(BUILD_MAJOR).$(BUILD_MINOR).a $(OBJS)

combHeader:
	@echo "-------------------------------"
	@echo "Creating Master, Unified Header"
	@cd networks && cat $(HDRS) > ../../$(LIBNAME).h

# clean targets for Solaris and Linux
clean: 
	-@find $(CURDIR) -name "*.o" | xargs rm -f

clean-full: clean
	-@rm -f ../*$(LIBNAME)*.a
