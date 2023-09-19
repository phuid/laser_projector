#include "Points.h"
#include <stdint.h>

#include <fstream>
using namespace std;

// Format spec: https://www.ilda.com/resources/StandardsDocs/ILDA_IDTF14_rev011.pdf

class Frame {

    public:
        Frame();
        bool getNext(std::ifstream& file, Points* points, uint8_t format_code);

    private:
        bool isLastPoint(char* bytes, FrameData data, uint8_t format_code);
        bool getBit(char b, int bitNumber);
        int map(int x, int in_min, int in_max, int out_min, int out_max);

    public:
        static const int NUMBER_OF_HEADER_BYTES = 32;

};
