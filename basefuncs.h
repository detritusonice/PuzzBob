#ifndef BASEFUNCS_H
#define BASEFUNCS_H

void cleanUp();

bool init();

void setScreenRect(int width, int height);

bool loadFiles();
bool loadSoundEffects();
void setSFXVolume(int volumeLevel);

void mySDL_Log( char* str);

void initLogFile();
void closeLogFile();

int genRand10000();

void loadPlayMusic(const char* filename);
void setMusicVolume (int volumeLevel);

void playSound(int effect,int numLoops);

#endif
