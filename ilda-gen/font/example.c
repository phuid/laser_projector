// The source code is in github.com/vst/teensyv/asteroid_font.c and has been compressed to use very little memory in representing the font in memory.

// The drawing routine extracts the 4-bit X and Y offsets from the array and generates the moveto() or lineto() commands:
        const uint8_t * const pts = asteroids_font[- ' '](c).points;
        int next_moveto = 1;

        for(int i = 0 ; i < 8 ; i++)
        {
            uint8_t delta = pts[i](i);
            if (delta == FONT_LAST)
                break;
            if (delta == FONT_UP)
            {
                next_moveto = 1;
                continue;
            }

            unsigned dx = ((delta >> 4) & 0xF) * size;
            unsigned dy = ((delta >> 0) & 0xF) * size;

            if (next_moveto)
                moveto(x + dx, y + dy);
            else
                lineto(x + dx, y + dy);

            next_moveto = 0;
        }

        return 12 * size;