#pragma once

// typedef for vector used in scenes
#include <memory>
#include <vector>
namespace Figures {  // forward declaration for typedef
class Figure;
class TexturedPlane;
}  // namespace Figures
typedef std::shared_ptr<Figures::Figure> FigurePtr;
typedef std::shared_ptr<Figures::TexturedPlane> FigurePortalPtr;
typedef std::vector<FigurePtr> FigurePtrVector;

#include <cmath>
#include <limits>
#include "camera/ray.h"
#include "camera/rayhit.h"
#include "io/plymodel.h"
#include "math/geometry.h"
#include "math/random.h"
#include "math/rgbcolor.h"
#include "scene/material.h"
#include "scene/uvmaterial.h"

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

    // Random point chosen in the figure's area
    virtual Vec4 randomPoint() const {
        throw std::domain_error(
            "Random point isn't implemented for this figure");
    }
    // Random direction for a given point in the figure
    virtual Vec4 randomDirection(const Vec4 &point) const {
        throw std::domain_error(
            "Random direction isn't implemented for this figure");
    }
    // Total area of the figure
    virtual float getTotalArea() const {
        throw std::domain_error("Total area isn't implemented for this figure");
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
    virtual bool getMaterial(const Vec4 &hitPoint,
                             MaterialPtr &materialPtr) const = 0;
};

class FlatPlane : public Plane {
    const MaterialPtr material;  // flat material

   public:
    FlatPlane(const Vec4 &_normal, float _distToOrigin,
              const MaterialPtr _material)
        : Plane(_normal, _distToOrigin), material(_material) {}
    bool getMaterial(const Vec4 &hitPoint,
                     MaterialPtr &materialPtr) const override {
        materialPtr = material;
        return true;
    }
};

class TexturedPlane : public Plane {
    UVMaterialPtr uvMaterial;  // material depends on hit point
    const bool infinite;       // repeat texture or not

   public:
    Vec4 uvOrigin, uvX, uvY;

    // this constructor should use setUVMaterial afterwards
    TexturedPlane(const Vec4 &_normal, float _distToOrigin,
                  bool _infinite = true)
        : Plane(_normal, _distToOrigin), infinite(_infinite) {}
    // init w/ material
    TexturedPlane(const Vec4 &_normal, float _distToOrigin,
                  const UVMaterialPtr &_uvMaterial, const Vec4 &_uvOrigin,
                  const Vec4 &_uvX, const Vec4 &_uvY, bool _infinite = true)
        : Plane(_normal, _distToOrigin),
          uvMaterial(_uvMaterial),
          uvOrigin(_uvOrigin),
          uvX(_uvX),
          uvY(_uvY),
          infinite(_infinite) {}
    bool getMaterial(const Vec4 &hitPoint,
                     MaterialPtr &materialPtr) const override;
    // special method for portals (material depends on objects)
    void setUVMaterial(const UVMaterialPtr &_uvMaterial, const Vec4 &_uvOrigin,
                       const Vec4 &_uvX, const Vec4 &_uvY);
    Vec4 randomPoint() const override;
    Vec4 randomDirection(const Vec4 &point) const override;
    float getTotalArea() const override;
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
    Vec4 randomPoint() const override;
    Vec4 randomDirection(const Vec4 &point) const override;
    float getTotalArea() const override;
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
    // no material, as boxes are only used for BVH/KdTree nodes

   public:
    Box(const Vec4 &_bb0, const Vec4 &_bb1) : bb0(_bb0), bb1(_bb1) {}
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