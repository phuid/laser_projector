#include "IldaReader.h"
#include "Frame.h"
#include "FrameData.h"
#include "Points.h"

#include <fstream>
#include <string>
using namespace std;

IldaReader::IldaReader() {}

bool IldaReader::readFile(string fileName) {

    // Open file.
    this->file = std::ifstream(fileName, std::ifstream::binary);
    if (!this->file) {
        return false;
    }

    // Read first 32 header bytes from the file and check if the first 4 bytes in ASCII are "ILDA".
    return checkHeader();
}

// Read first 32 header bytes from the file and check if the first 4 bytes in ASCII are "ILDA".
bool IldaReader::checkHeader() {
    char buf[Frame::NUMBER_OF_HEADER_BYTES];
    this->file.read(buf, Frame::NUMBER_OF_HEADER_BYTES);

    string header = "";
    for (int i = 0; i < 4; i++) {
        header += buf[i];
    }

    if (header != "ILDA") {
        // Invalid file.
        return false;
    }

    if (buf[7] != 0 && buf[7] != 1 && buf[7] != 2 && buf[7] != 4 && buf[7] != 5) {
        cerr << "Unsupported ILDA format, only supported formats from section 3.2. \"Format Codes\" from https://www.ilda.com/resources/StandardsDocs/ILDA_IDTF14_rev011.pdf" << endl;
        return false;
    }
    else {
        uint8_t this->current_format_code = buf[7];
    }

    // Return back to start of the file.
    this->file.seekg(0);

    return true;
}

bool IldaReader::getNextFrame(Points* points) {
    // Read all points from next frame.
    Frame frame;
    return frame.getNext(this->file, points, this->current_format_code);
}

void IldaReader::closeFile() {
    this->file.close();
}
