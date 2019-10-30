//
// Created by Yasmina on 30/10/2019.
//

#include "figures.h"

Figures::Sphere::Sphere(const Vec4 &center, float radius) : center(center), radius(radius) {}

Figures::Plane::Plane(const Vec4 &normal, float distToOrigin) : normal(normal), distToOrigin(distToOrigin) {}
