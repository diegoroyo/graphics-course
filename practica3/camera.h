#pragma once

#include <memory>
#include "../lib/geometry.h"
#include "../lib/ppmimage.h"
#include "figures.h"

class Camera {
    Vec4 origin, forward, up, right;

    // Convert camera's local space to world space
    Vec4 cameraToWorld(const Vec4 &v);

   public:
    Camera(Vec4 _origin, Vec4 _forward, Vec4 _up, Vec4 _right)
        : origin(_origin), forward(_forward), up(_up), right(_right) {}

    Camera(Vec4 _origin, Vec4 _forward, Vec4 _up, int width, int height);

    // Generate an image render (see implementation)
    PPMImage render(int width, int height, int rpp,
                    const FigurePtr &sceneRootNode,
                    const RGBColor &backgroundColor);
};
