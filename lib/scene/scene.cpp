#include "scene.h"

bool Scene::intersection(const Ray& ray, RayHit& hit) const {
    // Intersect with scene's root element (contains all figures)
    return this->root->intersection(ray, hit);
}

RGBColor Scene::directLight(const RayHit& hit, const Vec4& wo) const {
    RGBColor result(0.0f, 0.0f, 0.0f);
    // Check all lights in the scene
    for (const auto& light : lights) {
        Vec4 wi = hit.point - light.point;
        float norm = wi.module();
        wi = wi.normalize();
        RayHit hit;
        Ray ray(light.point, wi, this->air);
        // Check if theres direct view from light to point
        if (this->intersection(ray, hit) &&
            std::abs(hit.distance - norm) < 1e-3f &&
            dot(hit.normal, ray.direction) < 1e-5f) {
            // Add light's emission to the result
            RGBColor inEmission = light.emission * (1.0f / (norm * norm)) *
                                  dot(hit.normal, wi) * -1.0f;
            result = result + hit.material->evaluate(inEmission, hit, wi, wo);
        }
    }
    return result;
}