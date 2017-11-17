########################################################
#
#  Makefile for drsosc, drscl and drs_exam 
#  executables under linux
#
#  Requires wxWidgets 2.8.9 or newer
#
########################################################
ROOTPRESENT = YES
# CXX = g++-mp-4.9
# check for OS
OS            = $(shell uname)
ifeq ($(OS),Darwin)
DOS           = OS_DARWIN
else
DOS           = OS_LINUX
endif

CFLAGS        = -g -O2 -Wall -Wuninitialized -fno-strict-aliasing -Iinc -I/usr/local/inc -D$(DOS) -DHAVE_USB -DHAVE_LIBUSB10 
LIBS          = -L/opt/local/lib -lpthread -lutil -lusb-1.0

ifeq ($(OS),Darwin)
CFLAGS        +=  -I/opt/local/include
endif         

CPP_OBJ       = DRS.o averager.o
OBJECTS       = musbstd.o mxml.o strlcpy.o

ifeq ($(ROOTPRESENT),YES)
CFLAGS += -DROOTEXISTS
CFLAGS +=  $(shell root-config --cflags)
LIBS   += $(shell root-config --glibs)
endif

ifeq ($(OS),Darwin)
all: drs_exam_multi
else
all: drs_exam_multi
endif




drs_exam: $(OBJECTS) DRS.o averager.o drs_exam.o
	$(CXX) $(CFLAGS) $(OBJECTS) DRS.o averager.o drs_exam.o -o drs_exam $(LIBS) $(WXLIBS)

drs_exam_multi: $(OBJECTS) DRS.o averager.o drs_exam_multi.o
	$(CXX) $(CFLAGS) $(OBJECTS) DRS.o averager.o drs_exam_multi.o -o drs_exam_multi $(LIBS) $(WXLIBS)


drs_exam.o: src/drs_exam.cpp inc/mxml.h inc/DRS.h
	$(CXX) $(CFLAGS) -c $<

drs_exam_multi.o: drs_exam_multi.cpp inc/mxml.h inc/DRS.h
	$(CXX) $(CFLAGS) -c $<

$(CPP_OBJ): %.o: src/%.cpp inc/%.h inc/mxml.h inc/DRS.h
	$(CXX) $(CFLAGS) $(WXFLAGS) -c $<

$(OBJECTS): %.o: src/%.c inc/mxml.h inc/DRS.h
	$(CXX) $(CFLAGS) -c $<

clean:
	rm -f *.o drscl drsosc