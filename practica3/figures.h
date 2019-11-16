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

#include <limits>
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
    const Vec4 v0, v1, v2;
    const std::array<float, 2> uv0, uv1, uv2;
    const Vec4 edge0, edge1, normal;
    const PLYModel *model;

    // barycentric coordinates of p inside triangle v0-v1-v2
    // see implementation for more details
    Vec4 getBarycentric(const Vec4 &p) const;

   public:
    Triangle(const PLYModel *_model, int _v0i, int _v1i, int _v2i);
    bool intersection(const Ray &ray, RayHit &hit) const override;
};

// Only Axis-Aligned Bounding Boxes (AABB), doesn't support Oriented ones, see:
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
class Box : public Figure {
    const Vec4 bb0, bb1;  // bounding box, 2 points define the cube
    const RGBColor color;

   public:
    Box(const Vec4 &_bb0, const Vec4 &_bb1)
        : color(RGBColor::Black), bb0(_bb0), bb1(_bb1) {}
    Box(const RGBColor _color, const Vec4 &_bb0, const Vec4 &_bb1)
        : color(_color), bb0(_bb0), bb1(_bb1) {}
    bool intersection(const Ray &ray, RayHit &hit) const override;
};

// k-d tree node: acceleration structure
// checks if ray collides with bounding box, and if it does
// then it checks with all its children
class KdTreeNode : public Figure {
    const bool alwaysHits;  // override bbox check (scene root node)
    const FigurePtr bbox;
    const FigurePtrVector children;

   public:
    KdTreeNode(const FigurePtrVector &_children)
        : alwaysHits(true), bbox(nullptr), children(_children) {}
    KdTreeNode(const FigurePtrVector &_children, const FigurePtr _bbox)
        : alwaysHits(false), bbox(_bbox), children(_children) {}
    bool intersection(const Ray &ray, RayHit &hit) const override;
};

}  // namespace Figures