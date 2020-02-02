#pragma once

#include <random>
#include "math/geometry.h"

namespace Random {

// Generate random real number from 0..1
inline float ZeroOne() {
    static std::random_device rd;
    static std::mt19937 mtgen(rd());
    static std::uniform_real_distribution<float> random01(0.0f, 1.0f);
    return random01(mtgen);
}

// Random cos-weighted point on unit hemisphere given cob matrix
inline Vec4 CosHemisphere(const Mat4 &cob) {
    float incl = acosf(sqrtf(ZeroOne()));
    float azim = 2 * M_PI * ZeroOne();
    return cob * Vec4(sinf(incl) * cosf(azim), sinf(incl) * sinf(azim),
                      cosf(incl), 0.0f);
}

// Random point on unit sphere
inline Vec4 Sphere() {
    float incl = acosf(1.0f - 2.0f * ZeroOne());
    float azim = 2 * M_PI * ZeroOne();
    return Vec4(sinf(incl) * cosf(azim), sinf(incl) * sinf(azim), cosf(incl),
                0.0f);
}

};  // namespace Random