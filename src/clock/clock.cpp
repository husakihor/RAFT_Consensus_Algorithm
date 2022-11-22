#include "clock.hpp"

// ========== Clock class implementation ==========

Clock::Clock() : 
    _start_time(std::chrono::high_resolution_clock::now()) 
{}

float Clock::check()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - this->_start_time).count();
}

void Clock::reset()
{
    this->_start_time = std::chrono::high_resolution_clock::now();
}

void Clock::wait(int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}