#pragma once

#include <chrono>
#include <thread>

class Clock
{
public:
    Clock();

    // Function used to check the elapesd time in milliseconds since the reset of the clock
    float check();
    // Function used to reset the time of the clock
    void reset();
    // Function used to wait for a certains amount of milliseconds
    static void wait(int milliseconds);

private:
    // time point of the clock
    std::chrono::time_point<std::chrono::high_resolution_clock> _start_time;
};