#pragma once

#include <vector>
#include "figures.h"
#include "geometry.h"
#include "medium.h"
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
    const MediumPtr air;
    float maxLightEmission;

    Scene(const FigurePtr &_rootNode, const float _maxLightEmission = 0.0f)
        : root(_rootNode),
          lights(),
          air(Medium::air),
          maxLightEmission(_maxLightEmission) {}

    // Add a new light to the scene
    void light(const Vec4 &point, const RGBColor &emission) {
        lights.push_back(PointLight(point, emission));
        if (emission.max() > maxLightEmission) {
            maxLightEmission = emission.max();
        }
    }

    // Calculate direct light incoming from point lights
    RGBColor directLight(const RayHit &hit, const Vec4 &wo,
                         const EventPtr &event) const;
};