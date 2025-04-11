# Makefile for compiling BClean example

CXX = g++
CXXFLAGS = -std=c++17 -I../include -I..

SRCS = \
    ../src/Compensative.cpp \
    ../src/UserConstraints.cpp \
    ../src/BNStructure.cpp \
    ../BayesianClean.cpp \
    ../dataset.cpp \
    ../src/CompensativeParameter.cpp \
    beers.cpp

OBJS = $(SRCS:.cpp=.o)

TARGET = beers

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)

clean:
	rm -f $(TARGET) ../src/*.o *.o

