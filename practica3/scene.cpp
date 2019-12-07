#include "scene.h"

RGBColor Scene::directLight(const Vec4& point) const {
    RGBColor result = RGBColor::Black;
    // Check all lights in the scene
    for (auto light : lights) {
        Vec4 vector = point - light.point;
        float norm = vector.module();
        RayHit hit;
        Ray ray(light.point, vector * (1.0f / norm));
        // Check if theres direct view from light to point
        if (this->root->intersection(ray, hit) &&
            std::abs(hit.distance - norm) < 1e-5) {
            // Add light's emission to the result
            RGBColor inEmission = light.emission * (1.0f / (norm * norm));
            result = result + inEmission;
        }
    }
    return result;
}