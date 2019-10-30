//
// Created by Yasmina on 30/10/2019.
//

#include "figures.h"

Figures::Sphere::Sphere(const Vec4 &center, float radius) : center(center), radius(radius) {}

Figures::Plane::Plane(const Vec4 &normal, float distToOrigin) : normal(normal), distToOrigin(distToOrigin) {}
Vec4 Figures::Plane::Intersection(Vec4 v, Vec4 oc) {
    Vec4 p (1);
    if (dot(v,this->normal) != 0){
        //La normal y el vector v NO son perpendiculares -> existe interseccion
        float alpha = (this->distToOrigin - dot(this->normal,oc))/dot(v,this->normal);
        p= oc + v*alpha;
    }
    return p;
}

Vec4 Figures::Sphere::Intersection(Vec4 v, Vec4 oc){
    Vec4 p(1);
    float alpha;


}

