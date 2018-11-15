#ifndef CURSOR_H
#define CURSOR_H

enum CURSORS
{
    CURSOR_ARROW=0,
    CURSOR_CROSSHAIR
};

SDL_Cursor *InitGameCursor(const int cursor_id);

#endif
