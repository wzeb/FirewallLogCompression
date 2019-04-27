######################################
#
######################################
#source file
SOURCE  := $(wildcard *.cpp) $(wildcard */*.cpp)
OBJS    := $(patsubst %.cpp,%.o,$(SOURCE))
TARGET  := main

CC      := g++
LIBS    := -g -L/usr/local/lib -lyaml-cpp
LDFLAGS :=
DEFINES :=
INCLUDE := -I/usr/local/include
CFLAGS  := -g -Wall -O3 $(DEFINES) $(INCLUDE)
CXXFLAGS:= $(CFLAGS) -DHAVE_CONFIG_H -std=c++11
  
$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)
	
#i think you should do anything here
.PHONY : clean print
           
clean :
	rm -fr $(OBJS)
	rm -fr $(TARGET)
print :
	echo $(SOURCE)