#pragma once

#include <random>

// Generate random real number from 0..1
inline float random01() {
    static std::random_device rd;
    static std::mt19937 mtgen(rd());
    static std::uniform_real_distribution<float> random01(0.0f, 1.0f);
    return random01(mtgen);
}