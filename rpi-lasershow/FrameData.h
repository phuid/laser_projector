#include <stdint.h>

/* Format spec: https://www.ilda.com/resources/StandardsDocs/ILDA_IDTF14_rev011.pdf */

struct format_def { //all byte (and bit) numbers counted from 0 (0 = byte 1)
    int8_t X_BYTE = -1; //starting byte of 16bit int (0 = bytes 1 and 2)
    int8_t Y_BYTE = -1; //starting byte of 16bit int (2 = bytes 3 and 4)
    int8_t Z_BYTE = -1; //starting byte of 16bit int (4 = bytes 5 and 6)
    int8_t STATUS_BYTE = -1;
    int8_t COLOR_INDEX_BYTE = -1;
    int8_t R_COLOR_BYTE = -1;
    int8_t G_COLOR_BYTE = -1;
    int8_t B_COLOR_BYTE = -1;
    int8_t LAST_POINT_BIT = 7; // bit inside status byte
    int8_t LASER_ON_BIT = 6; // bit inside status byte
    int8_t NUMBER_OF_DATA_BYTES = -1;
};

class FrameData {
    
    public:
        FrameData();

    public:
        const format_def format_definitions[6] = { //before debug: if const cant be unitialised, init to null
            {/* Format 0 – 3D Coordinates with Indexed Color */
                0, // X_BYTE //starting byte of 16bit int (0 = bytes 1 and 2)
                2, // Y_BYTE //starting byte of 16bit int (2 = bytes 3 and 4)
                4, // Z_BYTE //starting byte of 16bit int (4 = bytes 5 and 6)
                6, // STATUS_BYTE
                7, // COLOR_INDEX_BYTE
                -1, // R_COLOR_BYTE
                -1, // G_COLOR_BYTE
                -1, // B_COLOR_BYTE
                7, // LAST_POINT_BIT // bit inside status byte
                6, // LASER_ON_BIT // bit inside status byte
                8, // NUMBER_OF_DATA_BYTES
            },
            {/* Format 1 – 2D Coordinates with Indexed Color */
                0, // X_BYTE //starting byte of 16bit int (0 = bytes 1 and 2)
                2, // Y_BYTE //starting byte of 16bit int (2 = bytes 3 and 4)
                -1, // Z_BYTE //starting byte of 16bit int (4 = bytes 5 and 6)
                4, // STATUS_BYTE
                5, // COLOR_INDEX_BYTE
                -1, // R_COLOR_BYTE
                -1, // G_COLOR_BYTE
                -1, // B_COLOR_BYTE
                7, // LAST_POINT_BIT // bit inside status byte
                6, // LASER_ON_BIT // bit inside status byte
                6, // NUMBER_OF_DATA_BYTES
            },
            {/* Format 2 – Color Palette for Indexed Color Frames */
                -1, // X_BYTE //starting byte of 16bit int (0 = bytes 1 and 2)
                -1, // Y_BYTE //starting byte of 16bit int (2 = bytes 3 and 4)
                -1, // Z_BYTE //starting byte of 16bit int (4 = bytes 5 and 6)
                -1, // STATUS_BYTE
                -1, // COLOR_INDEX_BYTE
                0, // R_COLOR_BYTE
                1, // G_COLOR_BYTE
                2, // B_COLOR_BYTE
                7, // LAST_POINT_BIT // bit inside status byte
                6, // LASER_ON_BIT // bit inside status byte
                3, // NUMBER_OF_DATA_BYTES
            },
            {/* Format 3 was proposed within the ILDA Technical Committee but was never approved. Therefore, format 3 is omitted in this ILDA standard. */
                -1, // X_BYTE //starting byte of 16bit int (0 = bytes 1 and 2)
                -1, // Y_BYTE //starting byte of 16bit int (2 = bytes 3 and 4)
                -1, // Z_BYTE //starting byte of 16bit int (4 = bytes 5 and 6)
                -1, // STATUS_BYTE
                -1, // COLOR_INDEX_BYTE
                -1, // R_COLOR_BYTE
                -1, // G_COLOR_BYTE
                -1, // B_COLOR_BYTE
                7, // LAST_POINT_BIT // bit inside status byte
                6, // LASER_ON_BIT // bit inside status byte
                -1, // NUMBER_OF_DATA_BYTES
            },
            {/* Format 4 – 3D Coordinates with True Color */
                0, // X_BYTE //starting byte of 16bit int (0 = bytes 1 and 2)
                2, // Y_BYTE //starting byte of 16bit int (2 = bytes 3 and 4)
                4, // Z_BYTE //starting byte of 16bit int (4 = bytes 5 and 6)
                6, // STATUS_BYTE
                -1, // COLOR_INDEX_BYTE
                9, // R_COLOR_BYTE
                8, // G_COLOR_BYTE
                7, // B_COLOR_BYTE
                7, // LAST_POINT_BIT // bit inside status byte
                6, // LASER_ON_BIT // bit inside status byte
                10, // NUMBER_OF_DATA_BYTES
            },
            {/* Format 5 – 2D Coordinates with True Color */
                0, // X_BYTE //starting byte of 16bit int (0 = bytes 1 and 2)
                2, // Y_BYTE //starting byte of 16bit int (2 = bytes 3 and 4)
                -1, // Z_BYTE //starting byte of 16bit int (4 = bytes 5 and 6)
                4, // STATUS_BYTE
                -1, // COLOR_INDEX_BYTE
                7, // R_COLOR_BYTE
                6, // G_COLOR_BYTE
                5, // B_COLOR_BYTE
                7, // LAST_POINT_BIT // bit inside status byte
                6, // LASER_ON_BIT // bit inside status byte
                8, // NUMBER_OF_DATA_BYTES
            },
        };

        int16_t x;
        int16_t y;
        int16_t z;
        uint8_t status;
        uint8_t color_index;
        uint8_t r_color;
        uint8_t g_color;
        uint8_t b_color;
        bool last_point_bit;
        bool laser_on_bit;
};
