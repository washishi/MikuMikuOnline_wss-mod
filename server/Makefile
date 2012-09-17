CC = gcc
CXX = g++
LD = g++

CXXFLAGS = -g -ggdb -Wall -std=gnu++0x -I/usr/include/cryptopp
LIBS = -lcryptopp -lboost_system -lboost_thread -lboost_date_time -lboost_filesystem -lboost_regex \
 -lpthread -lssl -ldl -lrt
LIBDIRS = -L/usr/lib -L/usr/local/lib

TARGET = server
OBJS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
OBJS += $(patsubst %.cpp,%.o,$(wildcard ../common/*.cpp))
OBJS += $(patsubst %.cpp,%.o,$(wildcard ../common/network/*.cpp))
OBJS += $(patsubst %.c,%.o,$(wildcard ../common/network/lz4/*.c))

all: stdafx.h.gch $(OBJS)
	$(LD) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS) $(LIBDIRS)

clean:
	@rm -f $(OBJS) $(TARGET) stdafx.h.gch

.cpp.o:
	$(CXX) $(CXXFLAGS) -include stdafx.h -c -o $@ $<

stdafx.h.gch:
	$(CXX) $(CXXFLAGS) stdafx.h
