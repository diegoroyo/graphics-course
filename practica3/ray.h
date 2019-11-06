#pragma once

#include "../lib/geometry.h"

class Ray {
   public:
    Vec4 origin;
    Vec4 direction;

    Ray(const Vec4& _origin, const Vec4& _direction)
        : origin(_origin), direction(_direction) {}

    inline Vec4 project(float distance) const {
        return this->origin + this->direction * distance;
    }
};