#pragma once

class Ray;

#include "camera/medium.h"
#include "camera/rayhit.h"
#include "math/geometry.h"

#include <iostream>

class Ray {
   public:
    Vec4 origin;
    Vec4 direction;
    // invDireciton is also calculated for use in multiplications
    // instead of dividing by direction (slower)
    Vec4 invDirection;
    // current medium the ray is passing through
    MediumPtr medium;
    // to be used for homogeneous iso scattering
    float distanceWithoutEvent;

    Ray() {}
    Ray(const Vec4& _origin, const Vec4& _direction, const MediumPtr& _medium,
        float _distanceWithoutEvent = 0.0f)
        : origin(_origin),
          direction(_direction),
          invDirection(Vec4(1.0f / _direction.x, 1.0f / _direction.y,
                            1.0f / _direction.z, _direction.w)),
          medium(_medium),
          distanceWithoutEvent(_distanceWithoutEvent) {}

    inline Vec4 project(float distance) const {
        return this->origin + this->direction * distance;
    }

    inline Ray event(const float distance) const {
        return Ray(this->project(distance), this->direction, this->medium);
    }
    inline Ray copy(const Vec4& origin, const Vec4& direction,
                    const RayHit& hit) const {
        return Ray(origin, direction, this->medium,
                   this->distanceWithoutEvent + hit.distance);
    }
    inline Ray copy(const Vec4& origin, const Vec4& direction,
                    const RayHit& hit, const MediumPtr& medium) const {
        return Ray(origin, direction, medium,
                   this->distanceWithoutEvent + hit.distance);
    }
};