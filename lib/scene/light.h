#pragma once

#include "math/geometry.h"
#include "math/rgbcolor.h"

struct PointLight {
    Vec4 point;
    RGBColor emission;
    PointLight(const Vec4 &_point, const RGBColor &_emission)
        : point(_point), emission(_emission) {}
};