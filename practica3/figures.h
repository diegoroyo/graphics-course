//
// Created by Yasmina on 30/10/2019.
//

#pragma once

#include "../lib/geometry.h"
#include "../lib/rgbcolor.h"
#include "ray.h"
#include "rayhit.h"

namespace Figures {

// Basic figure, doesn't represent any geometric objects
class Figure {
   protected:
    Figure(RGBColor _color) : color(_color) {}

   public:
    RGBColor color;

    // Has to be able to intersect with a ray
    virtual bool intersection(const Ray &ray, RayHit &hit)=0;
};

class Sphere : public Figure {
    Vec4 center;
    float radius;

   public:
    Sphere(const RGBColor _color, const Vec4 &_center, float _radius)
        : Figure(_color), center(_center), radius(_radius) {}
    bool intersection(const Ray &ray, RayHit &hit) override;
};

class Plane : public Figure {
    Vec4 normal;
    float distToOrigin;

   public:
    Plane(const RGBColor _color, const Vec4 &_normal, float _distToOrigin)
        : Figure(_color), normal(_normal), distToOrigin(_distToOrigin) {}
    bool intersection(const Ray &ray, RayHit &hit) override;
};

}  // namespace Figures