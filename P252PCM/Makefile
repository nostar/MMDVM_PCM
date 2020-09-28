CC      ?= gcc
CXX     ?= g++
CFLAGS  ?= -g -O3 -Wall -std=c++0x -pthread
LIBS    = -lm -limbe_vocoder -lpulse -lpulse-simple -lgpiod
LDFLAGS ?= -g

OBJECTS =   Conf.o Log.o ModeConv.o P25Network.o StopWatch.o TCPSocket.o Timer.o \
			UDPSocket.o Utils.o P252PCM.o

all:		P252PCM

P252PCM:	$(OBJECTS)
		$(CXX) $(OBJECTS) $(CFLAGS) $(LIBS) -o P252PCM

%.o: %.cpp
		$(CXX) $(CFLAGS) -c -o $@ $<

clean:
		$(RM) P252PCM *.o *.d *.bak *~
 
