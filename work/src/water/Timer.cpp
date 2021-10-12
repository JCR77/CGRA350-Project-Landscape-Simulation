#include "Timer.hpp"

#include <chrono>
#include <iostream>

using namespace std;

Timer::Timer()
{
    previous_time_ = getCurrentTime();
    current_time_ = getCurrentTime();
}

/**
 * Gets the current time in milliseconds
 */
long long Timer::getCurrentTime()
{
    using namespace std::chrono;
    milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return ms.count();
}

/**
 * Gets the delta time (in seconds) that have passed since the last
 * time calling this function.
 */
float Timer::getDelta()
{
    current_time_ = getCurrentTime();
    float delta = float(current_time_ - previous_time_) / 1000.f;
    previous_time_ = current_time_;
    return delta;
}