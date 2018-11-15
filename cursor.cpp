#include "base.h"
#include "cursor.h"
#include <cstdio>


const char *arrow[] = {
  /* width height num_colors chars_per_pixel */
  "    32    32        3            1",
  /* colors */
  "X c #000000",
  ". c #ffffff",
  "  c None",
  /* pixels */
  "X                               ",
  "XX                              ",
  "X.X                             ",
  "X..X                            ",
  "X...X                           ",
  "X....X                          ",
  "X.....X                         ",
  "X......X                        ",
  "X.......X                       ",
  "X........X                      ",
  "X.....XXXXX                     ",
  "X..X..X                         ",
  "X.X X..X                        ",
  "XX  X..X                        ",
  "X    X..X                       ",
  "     X..X                       ",
  "      X..X                      ",
  "      X..X                      ",
  "       XX                       ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "                                ",
  "0,0"
};


const char *crosshair[] = {
  /* width height num_colors chars_per_pixel */
  "    32    32        3            1",
  /* colors */
  "X c #000000",
  ". c #ffffff",
  "  c None",
  /* pixels */
  "                                ",
  "                X               ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                 .              ",
  " XXXXXXXXXXXXXX X XXXXXXXXXXXXXX",
  "  ..............   .............",
  "                X               ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "                X.              ",
  "16,16"
};

SDL_Cursor *InitGameCursor( const int cursor_id)
{
    int i, row, col;
    Uint8 data[4*32];
    Uint8 mask[4*32];
    int hot_x, hot_y;

    const char** image;

    switch (cursor_id)
    {
        case CURSOR_ARROW:
            image=arrow;
            break;
        case CURSOR_CROSSHAIR:
            image=crosshair;
            break;
        default:
            image=arrow;
    }

    i = -1;
    for ( row=0; row<32; ++row )
    {
        for ( col=0; col<32; ++col )
        {
            if ( col % 8 )
            {
                data[i] <<= 1;
                mask[i] <<= 1;
            }
            else
            {
                ++i;
                data[i] = mask[i] = 0;
            }

            switch (image[4+row][col])
            {
                case 'X':
                    data[i] |= 0x01;
                    mask[i] |= 0x01;
                    break;
                case '.':
                    mask[i] |= 0x01;
                    break;
                case ' ':
                    break;
            }
        }
    }
  sscanf(image[4+row], "%d,%d", &hot_x, &hot_y);
  return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}
