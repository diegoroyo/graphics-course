#pragma once

#include <memory>
#include "../lib/geometry.h"
#include "../lib/ppmimage.h"
#include "figures.h"

class Camera {
    Vec4 origin, forward, up, right;

    // Convert camera's local space to world space
    Vec4 cameraToWorld(const Vec4 &v);

    // Trace the path followed by the cameraRay (multiple hits etc)
    RGBColor tracePath(const Ray &cameraRay, const FigurePtr &sceneRootNode,
                       const RGBColor &backgroundColor);

    // Trace multiple (rpp) rays to a pixel defined by box [p0, p1]
    // and return mean color luminance that enters the pixel
    RGBColor tracePixel(const Vec4 &p0, const Vec4 &p1, int rpp,
                        const FigurePtr &sceneRootNode,
                        const RGBColor &backgroundColor);

   public:
    Camera(Vec4 _origin, Vec4 _forward, Vec4 _up, Vec4 _right)
        : origin(_origin), forward(_forward), up(_up), right(_right) {}

    Camera(Vec4 _origin, Vec4 _forward, Vec4 _up, float _aspectRatio)
        : origin(_origin),
          forward(_forward),
          up(_up),
          right(cross(forward, up).normalize() * up.module() * _aspectRatio) {}

    // Generate an image render (see implementation)
    PPMImage render(int width, int height, int rpp,
                    const FigurePtr &sceneRootNode,
                    const RGBColor &backgroundColor);
};
