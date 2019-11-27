#pragma once

#include "../lib/geometry.h"
#include "material.h"

// Used to save information about figure-ray intersections
struct RayHit {
    Vec4 point;
    float distance;
    const Material* material;
    Vec4 normal;
};