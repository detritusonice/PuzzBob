#ifndef GLOBALS_H
#define GLOBALS_H

extern SDL_Window* glbWindow;
extern SDL_Renderer* glbRenderer;

extern TTF_Font *glbFont;
extern TTF_Font *glbMessageFont;

extern SDL_Cursor* glbGameCursor;

extern SDL_Rect *screenRect;
extern SDL_Rect *guiBox;

class TextureManager;
extern TextureManager* glbTexManager;

class ScreenEffect;
extern ScreenEffect* glbEffect;

class RandomGenerator;
extern RandomGenerator *randGen;

class GameSettings;
extern GameSettings* gameSettings;

extern float guiUnit;

extern char glbLogMsg[];

extern Mix_Music* glbMusic;

class LevelStats;
extern LevelStats* playerStats;

class Profiler;
extern Profiler* glbProfiler;

extern bool isLegitimateCopy;
extern Mix_Chunk* glbSfx[NUM_SOUND_EFFECTS];

extern char glbAppPath[];

#endif
