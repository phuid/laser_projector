SRCS=soft_i2c.c soft_lcd.c encoder.cpp
HEAD=soft_i2c.h soft_lcd.h config.hpp encoder.hpp menu.hpp
LIBS=-lwiringPi
CFLAGS=-Wall -g
# -Wextra

all: main

main: $(SRCS) $(HEAD) main.cpp
	g++ $(CFLAGS) $(SRCS) $(LIBS) -o UI main.cpp

run: $(SRCS) $(HEAD) main.cpp
	g++ $(CFLAGS) $(SRCS) $(LIBS) -o UI main.cpp && sudo ./UI

clean:
	rm UI
