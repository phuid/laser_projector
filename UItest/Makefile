INCS=soft_i2c.c soft_lcd.c
HEAD=soft_i2c.h soft_lcd.h
LIBS=-lwiringPi
CFLAGS=-Wall -Wextra -g


all: main

main: $(INCS) $(HEAD) main.cpp
	g++ $(CFLAGS) -o UItest main.cpp $(INCS) $(LIBS)

run: $(INCS) $(HEAD) main.cpp
	g++ $(CFLAGS) -o UItest main.cpp $(INCS) $(LIBS) && sudo ./UItest

clean:
	rm UItest
