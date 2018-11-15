#ifndef ANTIHACK_H
#define ANTIHACK_H

unsigned int strHash(const char* s);
unsigned int imgHash(SDL_Surface* s);

#ifdef HACK_PROTECTION

    #ifdef PLATFORM_ANDROID
        bool ahCheckIntegrity();
        void ahCheckLicence();
        void ahGetParams();

    #endif // PLATFORM_ANDROID

    unsigned long long ahGetDexSum(const char* filename);

#endif // HACK_PROTECTION




#endif // ANTIHACK_H
