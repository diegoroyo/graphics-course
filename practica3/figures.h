#pragma once

// typedef for vector used in scenes
#include <memory>
#include <vector>
namespace Figures {  // forward declaration for typedef
class Figure;
}
typedef std::shared_ptr<Figures::Figure> FigurePtr;
typedef std::vector<FigurePtr> FigurePtrVector;
class PLYModel;  // needed for function definitions

#include "../lib/geometry.h"
#include "../lib/rgbcolor.h"
#include "plymodel.h"
#include "ray.h"
#include "rayhit.h"

namespace Figures {

// Basic figure, doesn't represent any geometric objects
class Figure {
   public:
    // Has to be able to intersect with a ray
    virtual bool intersection(const Ray &ray, RayHit &hit) const = 0;
};

class Sphere : public Figure {
    const Vec4 center;
    const float radius;
    const RGBColor color;

   public:
    Sphere(const RGBColor _color, const Vec4 &_center, float _radius)
        : color(_color), center(_center), radius(_radius) {}
    bool intersection(const Ray &ray, RayHit &hit) const override;
};

class Plane : public Figure {
    const Vec4 normal;
    const float distToOrigin;
    const RGBColor color;

   public:
    Plane(const RGBColor _color, const Vec4 &_normal, float _distToOrigin)
        : color(_color), normal(_normal), distToOrigin(_distToOrigin) {}
    bool intersection(const Ray &ray, RayHit &hit) const override;
};

class Triangle : public Figure {
    const Vec4 &v0, &v1, &v2;
    const std::array<float, 2> &uv0, &uv1, &uv2;
    const Vec4 edge0, edge1, normal;
    const PLYModel *model;

    // barycentric coordinates of p inside triangle v0-v1-v2
    // see implementation for more details
    Vec4 getBarycentric(const Vec4 &p) const;

   public:
    Triangle(const PLYModel *_model, int _v0i, int _v1i, int _v2i);
    bool intersection(const Ray &ray, RayHit &hit) const override;
};

}  // namespace Figures