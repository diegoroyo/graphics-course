#pragma once

#include "../lib/geometry.h"
#include "../lib/rgbcolor.h"

class RayHit {
   public:
    Vec4 point;
    float distance;
    RGBColor color;
};