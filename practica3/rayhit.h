#pragma once

#include "geometry.h"
#include "material.h"

// Used to save information about figure-ray intersections
struct RayHit {
    Vec4 point;
    float distance;
    MaterialPtr material;
    Vec4 normal;
};