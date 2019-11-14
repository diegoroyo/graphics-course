#include "figures.h"

namespace Figures {

bool Plane::intersection(const Ray &ray, RayHit &hit) const {
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
bool Sphere::intersection(const Ray &ray, RayHit &hit) const {
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

Triangle::Triangle(const PLYModel *_model, int _v0i, int _v1i, int _v2i)
    : model(_model),
      v0(_model->vert(_v0i)),
      v1(_model->vert(_v1i)),
      v2(_model->vert(_v2i)),
      uv0(_model->uv(_v0i)),
      uv1(_model->uv(_v1i)),
      uv2(_model->uv(_v2i)),
      edge0(v1 - v0),
      edge1(v2 - v0),
      normal(cross(edge0, edge1).normalize()) {}

// https://github.com/ssloy/tinyrenderer/wiki/Lesson-2:-Triangle-rasterization-and-back-face-culling
Vec4 Triangle::getBarycentric(const Vec4 &p) const {
    // Triangle abc, find barycentric coordinates of p
    Vec4 ab = edge0;
    Vec4 ac = edge1;
    Vec4 pa = v0 - p;
    // Find u, v s.t. u*ab + v*ac + pa = 0
    // To solve (u v 1)'*(abx acx pax) = 0 and (u v 1)'*(aby acy pay) = 0,
    // result is cross product of both (scaled so z = 1: x/z, y/z, z/z = 1)
    Vec4 vx = Vec4(ab.x, ac.x, pa.x, 0.0f);
    Vec4 vy = Vec4(ab.y, ac.y, pa.y, 0.0f);
    Vec4 vcross = cross(vx, vy);
    // p is inside the triangle if (u, v, 1-u-v) are all greater than 0
    // u = vcross.x / vcross.z, v = vcross.y / vcross.z
    // In other words: x, y & z have the same sign (so u and v are positive)
    // also (x + y) <= z (so 1-u-v is also positive)
    // Finally, if z = 0 the triangle is degenerate and shouldn't be drawn
    Vec4 coords(-1.0f, -1.0f, -1.0f, 0.0f);
    if (std::abs(vcross.z) > 1e-6f) {  // check degenerate triangle
        coords.y = vcross.x / vcross.z;
        coords.z = vcross.y / vcross.z;
        // add small epsilon to prevent float precision errors
        coords.x = 1.0f + 1e-5f - (vcross.x + vcross.y) / vcross.z;
    }
    return coords;
}

// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
bool Triangle::intersection(const Ray &ray, RayHit &hit) const {
    Vec4 h, s, q;
    float a, f, u, v;
    h = cross(ray.direction, this->edge1);
    a = dot(this->edge0, h);
    if (std::abs(a) < 1e-6) {
        return false;  // This ray is parallel to this triangle.
    }
    f = 1.0 / a;
    s = ray.origin - this->v0;
    u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) {
        return false;
    }
    q = cross(s, this->edge0);
    v = f * dot(ray.direction, q);
    if (v < 0.0 || u + v > 1.0) {
        return false;
    }
    // At this stage we can compute t to find out where the intersection point
    // is on the line.
    float t = f * dot(this->edge1, q);
    if (t > 1e-6) {
        // Ray intersection
        hit.distance = t;
        hit.point = ray.project(t);
        Vec4 b = getBarycentric(hit.point);
        if (b.x > 1e-6f) {
            float tex0 = uv0[0] * b.x + uv1[0] * b.y + uv2[0] * b.z;
            float tex1 = uv0[1] * b.x + uv1[1] * b.y + uv2[1] * b.z;
            hit.color = model->emission(tex0, tex1);
        } else {
            hit.color = RGBColor::Black;
        }
        return true;
    } else {
        // Intersection is behind the camera
        return false;
    }
}

}  // namespace Figures
