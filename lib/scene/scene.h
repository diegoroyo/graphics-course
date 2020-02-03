#pragma once

class Scene;

#include <vector>
#include "camera/medium.h"
#include "camera/ray.h"
#include "camera/rayhit.h"
#include "math/geometry.h"
#include "math/rgbcolor.h"
#include "scene/figures.h"
#include "scene/light.h"

class Scene {
   public:
    const FigurePtr root;
    std::vector<PointLight> lights;
    const MediumPtr air;
    float maxLightEmission;
    const RGBColor backgroundColor;

    Scene(const FigurePtr &_rootNode,
          const RGBColor &_backgroundColor = RGBColor::Black,
          const float _maxLightEmission = 0.0f)
        : root(_rootNode),
          lights(),
          air(Medium::air),
          maxLightEmission(_maxLightEmission),
          backgroundColor(_backgroundColor) {}

    // Add a new light to the scene
    void light(const Vec4 &point, const RGBColor &emission) {
        lights.push_back(PointLight(point, emission));
        if (emission.max() > maxLightEmission) {
            maxLightEmission = emission.max();
        }
    }

    bool intersection(const Ray &ray, RayHit &hit) const;
    // Calculate direct light incoming from point lights
    virtual RGBColor directLight(const RayHit &hit, const Vec4 &wo) const;
};