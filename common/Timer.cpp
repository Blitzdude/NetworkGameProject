#include "Timer.h"
#include <thread>
#include <NetworkLib/Log.h>
Timer::Timer()
    : m_deltaTimes(10)
{
    startOfTimer = std::chrono::system_clock::now();
    startPoint = std::chrono::system_clock::now();
}

Timer::~Timer()
{
}

// restarts time to the time of creation
void Timer::Restart()
{
    startPoint = std::chrono::system_clock::now();
}

// time from program start in seconds
float32 Timer::GetElapsedSeconds()
{
    std::chrono::duration<float> elapsedTime = std::chrono::system_clock::now() - startOfTimer;
    return elapsedTime.count();
}

float32 Timer::GetDeltaSeconds()
{
    std::chrono::system_clock::time_point endPoint
        = std::chrono::system_clock::now();
    std::chrono::duration<float> deltaTime = endPoint - startPoint;
    // update deltaTime vector
    m_deltaTimes.push_back(deltaTime.count());
    if (m_deltaTimes.size() > 10)
    {
        m_deltaTimes.pop_front();
    }

    return deltaTime.count();
}

uint32 Timer::GetElapsedTicks()
{
    return static_cast<uint32>(GetElapsedSeconds() * c_ticks_per_second);
}

uint32 Timer::GetDeltaTicks()
{
    return static_cast<uint32>(GetDeltaSeconds() * c_ticks_per_second);
}
float32 Timer::GetFPS()
{
    // sum all deltaTimes
    float32 l_sum = 0.0f;
    for (auto itr : m_deltaTimes)
        l_sum += itr;

    // get the average and turn it to seconds
    float32 l_seconds = 1 / (l_sum / m_deltaTimes.size());

    return l_seconds;
}
// Wait until designated time point
void Timer::WaitUntilNextTick()
{   
     auto endPoint = startPoint + frame_duration{1};
     std::this_thread::sleep_until(endPoint);
}

float32 Timer::TickToTime(uint32 tick)
{
    return tick * c_seconds_per_tick;
}

uint32 Timer::TimeToTick(float32 time)
{
    // static casting to stop compiler warning C4244
    return static_cast<uint32>(time * c_ticks_per_second);
}