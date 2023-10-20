// Author: Peiyao Li
// Date:   Mar 1 2023

#include "Timer.hpp"

void Timer::start() { 
    _start = std::chrono::steady_clock::now();
}

void Timer::end() { 
    _end = std::chrono::steady_clock::now();
}

void Timer::printTimeCost(const std::string& task)
{
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(_end - _start);
    std::cout << "[INFO] " << task << " cost:" << ms.count() << " ms" << std::endl;
}