//
// Created by Yasmina on 30/10/2019.
//

#include "figures.h"

namespace Figures {

    // todo: create class Ray??
Vec4 Plane::intersection(const Vec4 &rayO, const Vec4 &rayDir) {
    // Check if rayDir is perpendicular to plane normal (dot product is near 0)
    if (std::abs(dot(rayDir, this->normal)) < 1e-5f) {
        // Ray direction and plane are parallel, they don't meet
        return Vec4(0.0f, 0.0f, 0.0f, 0.0f);
    } else {
        // Ray intersects with plane, return intersection point
        float alpha = (this->distToOrigin - dot(this->normal, rayO)) /
                      dot(rayDir, this->normal);
        if (alpha <= 0.0f) {
            // Intersection is behind the camera, it isn't visible
            return Vec4(0.0f, 0.0f, 0.0f, 0.0f);
        } else {
            // Intersection in front of the camera
            return rayO + rayDir * alpha;
        }
    }
}

// Ray-sphere intersection algorithm source:
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
Vec4 Sphere::intersection(const Vec4 &rayO, const Vec4 &rayDir) {
    Vec4 l = this->center - rayO;
    float tca = dot(l, rayDir);
    if (tca < 0) {
        // Sphere is behind the camera, no intersection
        return Vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    float d2 = dot(l, l) - tca * tca;
    float radius2 = this->radius * this->radius;
    if (d2 > radius2) {
        // Ray misses the sphere (d > radius)
        return Vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    float thc = std::sqrt(radius2 - d2);
    if (tca - thc < 0.0f) {
        // First hit is behind the camera
        if (tca + thc < 0.0f) {
            // Second hit is behind the camera, doesn't intersect
            return Vec4(0.0f, 0.0f, 0.0f, 0.0f);
        } else {
            // Return second hit
            return rayO + rayDir * (tca + thc);
        }
    } else {
        // Return first hit
        return rayO + rayDir * (tca - thc);
    }
}

}  // namespace Figures
