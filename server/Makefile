CC = gcc
CXX = g++
LD = g++

CXXFLAGS = -g -ggdb -Wall -std=gnu++0x
LIBS = -lboost_system -lboost_thread -lboost_date_time -lboost_filesystem -lboost_regex \
 -lpthread -lssl -lcrypto -ldl -lrt
LIBDIRS = -L/usr/local/lib

TARGET = server
OBJS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
OBJS += $(patsubst %.cpp,%.o,$(wildcard ../common/network/*.cpp))

all: $(OBJS)
	$(LD) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS) $(LIBDIRS)

clean:
	@rm -f $(OBJS) $(TARGET)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<
