//
// Created by Yasmina on 30/10/2019.
//

#include "figures.h"

namespace Figures {

bool Plane::intersection(const Ray &ray, RayHit &hit) {
    // Check if rayDir is perpendicular to plane normal (dot product is near 0)
    if (std::abs(dot(ray.direction, this->normal)) < 1e-5f) {
        // Ray direction and plane are parallel, they don't meet
        return false;
    } else {
        // Ray intersects with plane, return intersection point
        float alpha = (this->distToOrigin - dot(this->normal, ray.origin)) /
                      dot(ray.direction, this->normal);
        if (alpha <= 0.0f) {
            // Intersection is behind the camera, it isn't visible
            return false;
        } else {
            // Intersection in front of the camera
            hit.distance = alpha;
            hit.point = ray.project(alpha);
            hit.color = this->color;
            return true;
        }
    }
}

// Ray-sphere intersection algorithm source:
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
bool Sphere::intersection(const Ray &ray, RayHit &hit) {
    Vec4 l = this->center - ray.origin;
    float tca = dot(l, ray.direction);
    if (tca < 0) {
        // Sphere is behind the camera, no intersection
        return false;
    }
    float d2 = dot(l, l) - tca * tca;
    float radius2 = this->radius * this->radius;
    if (d2 > radius2) {
        // Ray misses the sphere (d > radius)
        return false;
    }
    float thc = std::sqrt(radius2 - d2);
    if (tca - thc < 0.0f) {
        // First hit is behind the camera
        if (tca + thc < 0.0f) {
            // Second hit is behind the camera, doesn't intersect
            return false;
        } else {
            // Return second hit
            hit.distance = tca + thc;
            hit.point = ray.project(tca + thc);
            hit.color = this->color;
            return true;
        }
    } else {
        // Return first hit
        hit.distance = tca - thc;
        hit.point = ray.project(tca - thc);
        hit.color = this->color;
        return true;
    }
}

}  // namespace Figures
