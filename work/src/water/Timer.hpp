#pragma once

/**
 * Tracks how much time has passed.
 * Loops back to 0 when reached max time.
 */
class Timer
{
private:
    long long previous_time_;
    long long current_time_;

    long long getCurrentTime();

public:
    Timer();

    /**
     * Gets the delta time since the last time calling this
     */
    float getDelta();
};