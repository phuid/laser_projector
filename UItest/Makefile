INCS=soft_i2c.c soft_lcd.c
HEAD=soft_i2c.h soft_lcd.h encoder.hpp menu.hpp
LIBS=-lwiringPi
CFLAGS=-Wall -g
# -Wextra

all: main

main: $(INCS) $(HEAD) main.cpp
	g++ $(CFLAGS) -o UItest main.cpp $(INCS) $(LIBS)


debug: $(INCS) $(HEAD) main.cpp
	g++ $(CFLAGS) -D DEBUG -o UItest main.cpp $(INCS) $(LIBS) && sudo ./UItest

run: $(INCS) $(HEAD) main.cpp
	g++ $(CFLAGS) -o UItest main.cpp $(INCS) $(LIBS) && sudo ./UItest

clean:
	rm UItest
