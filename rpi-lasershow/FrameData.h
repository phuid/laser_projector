/* Format spec: https://www.ilda.com/resources/StandardsDocs/ILDA_IDTF14_rev011.pdf */

struct format_def { //all byte numbers counted from 0 (0 = byte 1) (DOESNT APPLY TO BITS)
    static const uint8_t X_BYTE; //starting byte of 16bit int (0 = bytes 1 and 2)
    static const uint8_t Y_BYTE; //starting byte of 16bit int (2 = bytes 3 and 4)
    static const uint8_t Z_BYTE; //starting byte of 16bit int (4 = bytes 5 and 6)
    static const uint8_t STATUS_BYTE;
    static const uint8_t COLOR_INDEX_BYTE;
    static const uint8_t R_COLOR_BYTE;
    static const uint8_t G_COLOR_BYTE;
    static const uint8_t B_COLOR_BYTE;
    static const uint8_t LAST_POINT_BIT = 8; // bit inside status byte (counted from 1 - 1 = bit 1)
    static const uint8_t LASER_ON_BIT = 7; // bit inside status byte (counted from 1 - 1 = bit 1)
    static const uint8_t NUMBER_OF_DATA_BYTES;
}

class FrameData {
    
    public:
        FrameData();

    public:
        format_def[] {
            {/* Format 0 – 3D Coordinates with Indexed Color */
                X_BYTE = 0; //starting byte of 16bit int (0 = bytes 1 and 2)
                Y_BYTE = 2; //starting byte of 16bit int (2 = bytes 3 and 4)
                Z_BYTE = 4; //starting byte of 16bit int (4 = bytes 5 and 6)
                STATUS_BYTE = 6;
                COLOR_INDEX_BYTE = 7;
                // R_COLOR_BYTE;
                // G_COLOR_BYTE;
                // B_COLOR_BYTE;
                // LAST_POINT_BIT; // bit inside status byte
                // LASER_ON_BIT; // bit inside status byte
                NUMBER_OF_DATA_BYTES = 8;
            },
            {/* Format 1 – 2D Coordinates with Indexed Color */
                X_BYTE = 0; //starting byte of 16bit int (0 = bytes 1 and 2)
                Y_BYTE = 2; //starting byte of 16bit int (2 = bytes 3 and 4)
                // Z_BYTE; //starting byte of 16bit int (4 = bytes 5 and 6)
                STATUS_BYTE = 4;
                COLOR_INDEX_BYTE = 5;
                // R_COLOR_BYTE;
                // G_COLOR_BYTE;
                // B_COLOR_BYTE;
                // LAST_POINT_BIT; // bit inside status byte
                // LASER_ON_BIT; // bit inside status byte
                NUMBER_OF_DATA_BYTES = 6;
            },
            {/* Format 2 – Color Palette for Indexed Color Frames */
                // X_BYTE; //starting byte of 16bit int (0 = bytes 1 and 2)
                // Y_BYTE; //starting byte of 16bit int (2 = bytes 3 and 4)
                // Z_BYTE; //starting byte of 16bit int (4 = bytes 5 and 6)
                // STATUS_BYTE;
                // COLOR_INDEX_BYTE;
                R_COLOR_BYTE = 0;
                G_COLOR_BYTE = 1;
                B_COLOR_BYTE = 2;
                // LAST_POINT_BIT; // bit inside status byte
                // LASER_ON_BIT; // bit inside status byte
                NUMBER_OF_DATA_BYTES = 3;
            },
            {/* Format 3 was proposed within the ILDA Technical Committee but was never approved. Therefore, format 3 is omitted in this ILDA standard. */
                // X_BYTE; //starting byte of 16bit int (0 = bytes 1 and 2)
                // Y_BYTE; //starting byte of 16bit int (2 = bytes 3 and 4)
                // Z_BYTE; //starting byte of 16bit int (4 = bytes 5 and 6)
                // STATUS_BYTE;
                // COLOR_INDEX_BYTE;
                // R_COLOR_BYTE;
                // G_COLOR_BYTE;
                // B_COLOR_BYTE;
                // LAST_POINT_BIT; // bit inside status byte
                // LASER_ON_BIT; // bit inside status byte
                // NUMBER_OF_DATA_BYTES;
            },
            {/* Format 4 – 3D Coordinates with True Color */
                X_BYTE = 0; //starting byte of 16bit int (0 = bytes 1 and 2)
                Y_BYTE = 2; //starting byte of 16bit int (2 = bytes 3 and 4)
                Z_BYTE = 4; //starting byte of 16bit int (4 = bytes 5 and 6)
                STATUS_BYTE = 6;
                // COLOR_INDEX_BYTE;
                R_COLOR_BYTE = 9;
                G_COLOR_BYTE = 8;
                B_COLOR_BYTE = 7;
                // LAST_POINT_BIT; // bit inside status byte
                // LASER_ON_BIT; // bit inside status byte
                NUMBER_OF_DATA_BYTES = 10;
            },
            {/* Format 5 – 2D Coordinates with True Color */
                X_BYTE = 0; //starting byte of 16bit int (0 = bytes 1 and 2)
                Y_BYTE = 2; //starting byte of 16bit int (2 = bytes 3 and 4)
                // Z_BYTE; //starting byte of 16bit int (4 = bytes 5 and 6)
                STATUS_BYTE = 4;
                // COLOR_INDEX_BYTE;
                R_COLOR_BYTE = 7;
                G_COLOR_BYTE = 6;
                B_COLOR_BYTE = 5;
                // LAST_POINT_BIT; // bit inside status byte
                // LASER_ON_BIT; // bit inside status byte
                NUMBER_OF_DATA_BYTES = 8;
            },
        }

        int x;
        int y;
        int z;
        bool LaserOn;
        
};
