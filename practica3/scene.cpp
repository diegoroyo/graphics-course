#include "scene.h"

RGBColor Scene::directLight(const RayHit& hit, const Vec4& wo,
                            const EventPtr& event) const {
    RGBColor result = RGBColor::Black;
    // Check all lights in the scene
    for (auto light : lights) {
        Vec4 wi = light.point - hit.point;
        float norm = wi.module();
        RayHit hit;
        Ray ray(light.point, wi * (-1.0f / norm), this->air);
        // Check if theres direct view from light to point
        if (this->root->intersection(ray, hit) &&
            std::abs(hit.distance - norm) < 1e-5) {
            // Add light's emission to the result
            RGBColor inEmission =
                light.emission * (1.0f / (norm * norm)) * dot(hit.normal, wi);
            result = result + event->applyNextEvent(inEmission, hit, wi, wo);
        }
    }
    return result;
}