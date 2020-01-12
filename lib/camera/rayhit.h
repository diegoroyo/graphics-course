#pragma once

struct RayHit;

#include "math/geometry.h"
#include "scene/material.h"

// Used to save information about figure-ray intersections
struct RayHit {
    Vec4 point;
    float distance;
    MaterialPtr material;
    Vec4 normal;
    bool enters;
};