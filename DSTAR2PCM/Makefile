CC      ?= gcc
CXX     ?= g++
CFLAGS  ?= -g -O3 -Wall -std=c++0x -pthread
LIBS    = -lm -lpulse -lpulse-simple -lgpiod
LDFLAGS ?= -g

OBJECTS =   Conf.o Log.o ModeConv.o DSTARNetwork.o SerialController.o StopWatch.o Timer.o UDPSocket.o Utils.o DSTAR2PCM.o

all:		DSTAR2PCM

DSTAR2PCM:	$(OBJECTS)
		$(CXX) $(OBJECTS) $(CFLAGS) $(LIBS) -o DSTAR2PCM

%.o: %.cpp
		$(CXX) $(CFLAGS) -c -o $@ $<

clean:
		$(RM) DSTAR2PCM *.o *.d *.bak *~
 
