//
// Created by Yasmina on 30/10/2019.
//

#pragma once

#include "../lib/geometry.h"
#include "../lib/rgbcolor.h"

namespace Figures {

// Basic figure, doesn't represent any geometric objects
class Figure {
   public:
    // Has to be able to intersect with a ray
    virtual Vec4 intersection(const Vec4 &rayO, const Vec4 &rayDir) {
        return Vec4(0.0f, 0.0f, 0.0f, 0.0f);
    };
};

class Sphere : public Figure {
    Vec4 center;
    float radius;

   public:
    Sphere(const Vec4 &_center, float _radius)
        : center(_center), radius(_radius) {}
    Vec4 intersection(const Vec4 &rayO, const Vec4 &rayDir) override;
};

class Plane : public Figure {
    Vec4 normal;
    float distToOrigin;

   public:
    Plane(const Vec4 &_normal, float _distToOrigin)
        : normal(_normal), distToOrigin(_distToOrigin) {}
    Vec4 intersection(const Vec4 &rayO, const Vec4 &rayDir) override;
};

}  // namespace Figures