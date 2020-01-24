#pragma once

#include <chrono>
#include <iostream>

// Prints fancy progress bar to stdout
void printProgress(const std::chrono::nanoseconds &beginTime, float progress);