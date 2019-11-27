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
#include "material.h"
#include "plymodel.h"
#include "ray.h"
#include "rayhit.h"

namespace Figures {

// Basic figure, doesn't represent any geometric objects
class Figure {
   public:
    // Has to be able to intersect with a ray
    virtual bool intersection(const Ray &ray, RayHit &hit) const = 0;
    // Check hit distance with bounding box (if the figure doesn't have
    // a bounding box, unlike KdTreeNode, it defaults to intersection)
    virtual bool peek(const Ray &ray, RayHit &hit) const {
        return this->intersection(ray, hit);
    }
};

class Sphere : public Figure {
    const Vec4 center;
    const float radius;
    const Material material;

   public:
    Sphere(const Material _material, const Vec4 &_center, float _radius)
        : material(_material), center(_center), radius(_radius) {}
    bool intersection(const Ray &ray, RayHit &hit) const override;
};

class Plane : public Figure {
    const Vec4 normal;
    const float distToOrigin;
    const Material material;

   public:
    Plane(const Material _material, const Vec4 &_normal, float _distToOrigin)
        : material(_material), normal(_normal), distToOrigin(_distToOrigin) {}
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
    const Material material;

   public:
    Box(const Vec4 &_bb0, const Vec4 &_bb1)
        : material(Material::none()), bb0(_bb0), bb1(_bb1) {}
    Box(const Material _material, const Vec4 &_bb0, const Vec4 &_bb1)
        : material(_material), bb0(_bb0), bb1(_bb1) {}
    bool intersection(const Ray &ray, RayHit &hit) const override;
};

// Bounding volume node: has a bounding box that envolves all its children,
// accelerates checks by first checking against the bbox. Then, only if it
// hits, it checks against all its children
class BVNode : public Figure {
    const bool alwaysHits;  // override bbox check
    const FigurePtr bbox;
    const FigurePtrVector children;

   public:
    BVNode(const FigurePtrVector &_children)
        : alwaysHits(true), bbox(nullptr), children(_children) {}
    BVNode(const FigurePtrVector &_children, const FigurePtr &_bbox)
        : alwaysHits(false), bbox(_bbox), children(_children) {}
    bool intersection(const Ray &ray, RayHit &hit) const override;
    bool peek(const Ray &ray, RayHit &hit) const override {
        // shouldn't be called if alwaysHits = true
        return bbox->intersection(ray, hit);
    }
};

// k-d tree node: acceleration structure that has a bbox and two children
// performs special checks to minimize checking intersections on branches
class KdTreeNode : public Figure {
    const FigurePtr bbox;
    const FigurePtr leftChild, rightChild;

   public:
    KdTreeNode(const FigurePtr &_leftChild, const FigurePtr &_rightChild,
               const FigurePtr &_bbox)
        : bbox(_bbox), leftChild(_leftChild), rightChild(_rightChild) {}
    bool intersection(const Ray &ray, RayHit &hit) const override;
    bool peek(const Ray &ray, RayHit &hit) const override {
        return bbox->intersection(ray, hit);
    }
};

}  // namespace Figures