#pragma once

#include <vector>
#include "figures.h"
#include "geometry.h"
#include "rgbcolor.h"

class Scene {
    struct PointLight {
        const Vec4 point;
        const RGBColor emission;
        PointLight(const Vec4 &_point, const RGBColor &_emission)
            : point(_point), emission(_emission) {}
    };

   public:
    const FigurePtr root;
    std::vector<PointLight> lights;
    float maxLightEmission;

    Scene(const FigurePtr &_rootNode) : root(_rootNode), lights(), maxLightEmission(0.0f) {}

    // Add a new light to the scene
    void light(const Vec4 &point, const RGBColor &emission) {
        lights.push_back(PointLight(point, emission));
        if (emission.max() > maxLightEmission) {
            maxLightEmission = emission.max();
        }
    }

    // Calculate direct light incoming from point lights
    RGBColor directLight(const Vec4 &point) const;
};