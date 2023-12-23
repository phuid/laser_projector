# the compiler: gcc for C program, define as g++ for C++
CC = g++

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall (warnings all) turns on most, but not all, compiler warnings
CFLAGS  = -Wall

# the build target executable:
TARGET = build/lasershow

SRCS = main.cpp lasershow.cpp ABE_ADCDACPi.cpp IldaReader.cpp Frame.cpp FrameData.cpp Points.cpp
LIBS = -lwiringPi -lzmq

all: main

main: lasershow.cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	$(RM) $(TARGET)
