INCS=soft_i2c.c soft_lcd.c
HEAD=soft_i2c.h soft_lcd.h encoder.hpp menu.hpp
LIBS=-lwiringPi
CFLAGS=-Wall -g
# -Wextra

all: main

main: $(INCS) $(HEAD) main.cpp
	g++ $(CFLAGS) $(INCS) $(LIBS) -o UI main.cpp

run: $(INCS) $(HEAD) main.cpp
	g++ $(CFLAGS) $(INCS) $(LIBS) -o UI main.cpp && sudo ./UI

clean:
	rm UI
