#pragma once
#include <chrono>
#include <deque>
#include <NetworkLib/Constants.h>

typedef std::chrono::duration<int, std::ratio<1, ticks_per_second>> frame_duration;

class Timer
{
public:
    Timer();
    ~Timer();

    // get current system time, and set that as now
    // used with GetDeltaTicks();
    void Restart();
    // returns the amount of seconds from the start of the timer
    float32 GetElapsedSeconds();
    // returns the amount of seconds from last restart
    float32 GetDeltaSeconds(); 
    // returns the amount of ticks from the start of timer
    uint32 GetElapsedTicks();
    // returns the amount of seconds that have passed, since restart.
    uint32 GetDeltaTicks();
    // returns the update fps
    float32 GetFPS();
    //Make this function cause the thread to wait until a certain time point
    //used to cap framerates 
    void WaitUntilNextTick();

    float32 TickToTime(uint32 tick);
    uint32 TimeToTick(float32 time);

private:
    std::chrono::system_clock::time_point startOfTimer;
    std::chrono::system_clock::time_point startPoint;
   

    std::deque<float32> m_deltaTimes;
};