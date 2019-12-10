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

#include <cmath>
#include <limits>
#include "geometry.h"
#include "material.h"
#include "plymodel.h"
#include "ray.h"
#include "rayhit.h"
#include "rgbcolor.h"
#include "uvmaterial.h"

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

/// Plane ///

class Plane : public Figure {
   protected:
    const Vec4 normal;
    const float distToOrigin;
    const UVMaterialPtr uvMaterial;
    Plane(const Vec4 &_normal, float _distToOrigin)
        : normal(_normal), distToOrigin(_distToOrigin) {}

   public:
    bool intersection(const Ray &ray, RayHit &hit) const override;
    virtual MaterialPtr getMaterial(const Vec4 &hitPoint) const = 0;
};

class FlatPlane : public Plane {
    const MaterialPtr material;  // flat material

   public:
    FlatPlane(const Vec4 &_normal, float _distToOrigin,
              const MaterialPtr _material)
        : Plane(_normal, _distToOrigin), material(_material) {}
    MaterialPtr getMaterial(const Vec4 &hitPoint) const override {
        return material;
    }
};

class TexturedPlane : public Plane {
    const UVMaterialPtr uvMaterial;  // material depends on hit point
    const Vec4 uvOrigin, uvX, uvY;

   public:
    TexturedPlane(const Vec4 &_normal, float _distToOrigin,
                  const UVMaterialPtr &_uvMaterial, const Vec4 &_uvOrigin,
                  const Vec4 &_uvX, const Vec4 &_uvY)
        : Plane(_normal, _distToOrigin),
          uvMaterial(_uvMaterial),
          uvOrigin(_uvOrigin),
          uvX(_uvX),
          uvY(_uvY) {}
    MaterialPtr getMaterial(const Vec4 &hitPoint) const override {
        // TODO prettify
        Vec4 d = hitPoint - this->uvOrigin;
        float uvx = fmodf(dot(d, uvX), 1.0f);
        if (uvx < 1e-6f) uvx += 1.0f;
        uvx = std::fmax(0.0f, std::fmin(1.0f, uvx));
        float uvy = fmodf(dot(d, uvY), 1.0f);
        if (uvy < 1e-6f) uvy += 1.0f;
        uvy = std::fmax(0.0f, std::fmin(1.0f, uvy));
        return uvMaterial->get(uvx, uvy);
    }
};

/// Sphere ///

class Sphere : public Figure {
    const Vec4 center;
    const float radius;
    const MaterialPtr material;

   public:
    Sphere(const MaterialPtr _material, const Vec4 &_center, float _radius)
        : material(_material), center(_center), radius(_radius) {}
    bool intersection(const Ray &ray, RayHit &hit) const override;
};

/// Triangle ///

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

/// Box ///

// Only Axis-Aligned Bounding Boxes (AABB), doesn't support Oriented ones, see:
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
class Box : public Figure {
    const Vec4 bb0, bb1;  // bounding box, 2 points define the cube
    const MaterialPtr material;

   public:
    Box(const Vec4 &_bb0, const Vec4 &_bb1)
        : material(Material::none()), bb0(_bb0), bb1(_bb1) {}
    Box(const MaterialPtr _material, const Vec4 &_bb0, const Vec4 &_bb1)
        : material(_material), bb0(_bb0), bb1(_bb1) {}
    bool intersection(const Ray &ray, RayHit &hit) const override;
};

/// BVNode / KdTreeNode ///

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