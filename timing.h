#ifndef TIMING_H
#define TIMING_H

class Timer
{

    private:
        int startTicks;
        int pausedTicks;
        bool started;
        bool paused;

    public:
        Timer();

        void Start();
        void Stop();
        void Pause();
        void Resume();

        int GetTicks();
        void SetPausedTicks(int ticks);
        bool IsStarted();
        bool IsPaused();
};

#endif
