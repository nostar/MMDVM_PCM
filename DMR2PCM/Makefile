CC      ?= gcc
CXX     ?= g++
CFLAGS  ?= -g -O3 -Wall -std=c++0x -pthread
LIBS    = -lm -lpthread -lmd380_vocoder -lpulse -lpulse-simple -lgpiod
LDFLAGS ?= -g

OBJECTS = 	BPTC19696.o Conf.o CRC.o DMRData.o DMREMB.o DMREmbeddedData.o \
			DMRFullLC.o DMRLC.o DMRLookup.o DMRSlotType.o  MMDVMNetwork.o  Golay2087.o \
			Golay24128.o Hamming.o Log.o ModeConv.o Mutex.o QR1676.o RS129.o \
			SHA256.o StopWatch.o Sync.o Thread.o Timer.o UDPSocket.o Utils.o DMR2PCM.o

all:		DMR2PCM

DMR2PCM:	$(OBJECTS)
		$(CXX) $(OBJECTS) $(CFLAGS) $(LIBS) -o DMR2PCM  -Xlinker --section-start=.firmware=0x0800C000 -Xlinker  --section-start=.sram=0x20000000

%.o: %.cpp
		$(CXX) $(CFLAGS) -c -o $@ $<

clean:
		$(RM) DMR2PCM *.o *.d *.bak *~

