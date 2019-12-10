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
            hit.material = this->getMaterial(hit.point);
            hit.enters = dot(this->normal, ray.direction) > 1e-5f;
            hit.normal = hit.enters ? this->normal * -1.0f : this->normal;
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
    float thc = sqrtf(radius2 - d2);
    if (tca - thc < 1e-4f) {
        // First hit is behind the camera
        if (tca + thc < 1e-4f) {
            // Second hit is behind the camera, doesn't intersect
            return false;
        } else {
            // Return second hit (from inside the sphere)
            hit.distance = tca + thc;
            hit.point = ray.project(tca + thc);
            hit.material = this->material;
            hit.normal = (this->center - hit.point).normalize();
            hit.enters = false;
            return true;
        }
    } else {
        // Return first hit (from outside the sphere)
        hit.distance = tca - thc;
        hit.point = ray.project(tca - thc);
        hit.material = this->material;
        hit.normal = (hit.point - this->center).normalize();
        hit.enters = true;
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
    if (std::abs(a) < 1e-6f) {
        return false;  // This ray is parallel to this triangle.
    }
    f = 1.0f / a;
    s = ray.origin - this->v0;
    u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f) {
        return false;
    }
    q = cross(s, this->edge0);
    v = f * dot(ray.direction, q);
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }
    // At this stage we can compute t to find out where the intersection point
    // is on the line.
    float t = f * dot(this->edge1, q);
    if (t > 1e-6f) {
        // Ray intersection
        hit.distance = t;
        hit.point = ray.project(t);
        // Calculate texture coordinates
        Vec4 b = getBarycentric(hit.point);
        float tex0 = uv0[0] * b.x + uv1[0] * b.y + uv2[0] * b.z;
        float tex1 = uv0[1] * b.x + uv1[1] * b.y + uv2[1] * b.z;
        // Clamp between 0-1
        tex0 = tex0 < 1e-6f ? 0.0f : tex0;
        tex0 = tex0 > 1.0f - 1e-6f ? 1.0f - 1e-6f : tex0;
        tex1 = tex1 < 1e-6f ? 0.0f : tex1;
        tex1 = tex1 > 1.0f - 1e-6f ? 1.0f - 1e-6f : tex1;
        hit.material = this->model->material(tex0, tex1);
        // Calculate normal as it was a plane
        hit.enters = dot(this->normal, ray.direction) > 1e-5f;
        hit.normal = hit.enters ? this->normal * -1.0f : this->normal;
        return true;
    } else {
        // Intersection is behind the camera
        return false;
    }
}

// https://tavianator.com/fast-branchless-raybounding-box-intersections-part-2-nans/
bool Box::intersection(const Ray &ray, RayHit &hit) const {
    // find intersection point for each of the 6 planes that define the box
    float t1, t2;
    float tmin = std::numeric_limits<float>::max() * -1.0f;
    float tmax = std::numeric_limits<float>::max();
    for (int i = 0; i < 3; i++) {
        t1 = (bb0.raw[i] - ray.origin.raw[i]) * ray.invDirection.raw[i];
        t2 = (bb1.raw[i] - ray.origin.raw[i]) * ray.invDirection.raw[i];
        tmin = std::fmax(tmin, std::fmin(t1, t2));
        tmax = std::fmin(tmax, std::fmax(t1, t2));
    }

    // Ray can have 2 hits: tmin contains first one, tmax the second one
    if (tmax > std::fmax(tmin, 0.0f)) {
        // If first hit is behind the camera, take the second one
        hit.distance = tmin < 0.0f ? tmax : tmin;
        hit.point = ray.project(hit.distance);
        hit.material = this->material;
        return true;
    } else {
        // No hit
        return false;
    }
}

bool BVNode::intersection(const Ray &ray, RayHit &hit) const {
    // Check if ray doesn't hit box
    if (!this->alwaysHits && !this->bbox->intersection(ray, hit)) {
        return false;
    }
    float minDistance = std::numeric_limits<float>::max();
    // Intersect with all figures in scene
    for (auto const &figure : this->children) {
        RayHit figureHit;
        if (figure->intersection(ray, figureHit) &&
            figureHit.distance < minDistance) {
            // Only save intersection if hits the closest object
            minDistance = figureHit.distance;
            hit = figureHit;
        }
    }
    return minDistance != std::numeric_limits<float>::max();
}

bool KdTreeNode::intersection(const Ray &ray, RayHit &hit) const {
    // Check if it only intersects with one box
    RayHit firstPeek, secondPeek;
    if (!leftChild->peek(ray, firstPeek)) {
        return rightChild->intersection(ray, hit);
    }
    if (!rightChild->peek(ray, secondPeek)) {
        return leftChild->intersection(ray, hit);
    }
    // Determine order of intersections
    FigurePtr first = leftChild, second = rightChild;
    if (secondPeek.distance < firstPeek.distance) {
        std::swap(first, second);
        std::swap(firstPeek, secondPeek);
    }
    // Intersect with boxes
    if (!first->intersection(ray, hit)) {
        // Doesn't intersect with first box, return second
        return second->intersection(ray, hit);
    } else {
        // First intersection is saved in hit
        // If first hit was closer that second peek,
        // we don't have to check the second child
        RayHit secondHit;
        if (secondPeek.distance < hit.distance &&
            second->intersection(ray, secondHit) &&
            secondHit.distance < hit.distance) {
            // Second hit was closer than first, should be replaced
            std::swap(hit, secondHit);
        }
        return true;
    }
}

}  // namespace Figures
