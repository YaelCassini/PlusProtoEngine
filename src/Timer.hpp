// Author: Peiyao Li
// Date:   Mar 1 2023
#pragma once
#include <iostream>
#include <chrono>
#include "Global.hpp"
// Timer
class Timer {
private:
    std::chrono::time_point<std::chrono::steady_clock> _start, _end;
public:
    void start();
    void end();
    void printTimeCost(const std::string& task);
};