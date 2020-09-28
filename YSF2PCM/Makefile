CC      ?= gcc
CXX     ?= g++
CFLAGS  ?= -g -O3 -Wall -std=c++0x -pthread
LIBS    = -lm -lpthread -lmd380_vocoder -lpulse -lpulse-simple -lgpiod
LDFLAGS ?= -g

OBJECTS =   APRSReader.o APRSWriter.o APRSWriterThread.o Conf.o CRC.o DTMF.o Golay24128.o \
			GPS.o Log.o ModeConv.o Mutex.o SHA256.o StopWatch.o Sync.o TCPSocket.o Thread.o \
			Timer.o UDPSocket.o Utils.o WiresX.o YSF2PCM.o YSFConvolution.o YSFFICH.o \
			YSFNetwork.o YSFPayload.o

all:		YSF2PCM

YSF2PCM:	$(OBJECTS)
		$(CXX) $(OBJECTS) $(CFLAGS) $(LIBS) -o YSF2PCM -Xlinker --section-start=.firmware=0x0800C000 -Xlinker  --section-start=.sram=0x20000000

%.o: %.cpp
		$(CXX) $(CFLAGS) -c -o $@ $<

clean:
		$(RM) YSF2PCM *.o *.d *.bak *~
 
