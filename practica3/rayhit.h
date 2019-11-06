#pragma once

#include "../lib/geometry.h"
#include "../lib/rgbcolor.h"

// Used to save information about figure-ray intersections
struct RayHit {
    Vec4 point;
    float distance;
    RGBColor color;
};