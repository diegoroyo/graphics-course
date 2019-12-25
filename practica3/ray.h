#pragma once

#include "geometry.h"
#include "medium.h"

class Ray {
   public:
    Vec4 origin;
    Vec4 direction;
    // invDireciton is also calculated for use in multiplications
    // instead of dividing by direction (slower)
    Vec4 invDirection;
    // current medium the ray is passing through
    MediumPtr medium;

    Ray() {}
    Ray(const Vec4& _origin, const Vec4& _direction, const MediumPtr& _medium)
        : origin(_origin),
          direction(_direction),
          invDirection(Vec4(1.0f / _direction.x, 1.0f / _direction.y,
                            1.0f / _direction.z, _direction.w)),
          medium(_medium) {}

    inline Vec4 project(float distance) const {
        return this->origin + this->direction * distance;
    }
};