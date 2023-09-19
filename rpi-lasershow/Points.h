#pragma once
#include <stdint.h>

class Points {  
    public:
        Points();
        bool next();

    public:
        static const short MAX_POINTS = 3000;

        short store[MAX_POINTS*3];
        uint8_t format_code;
        short size = 0;
        short index;
};
