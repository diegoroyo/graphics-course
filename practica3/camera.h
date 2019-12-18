#pragma once

#include <chrono>
#include <cmath>
#include <future>
#include <memory>
#include <string>
#include "figures.h"
#include "geometry.h"
#include "ppmimage.h"
#include "random.h"
#include "scene.h"

class Camera {
    const Vec4 origin, forward, up, right;
    float dofRadius;  // 0.0f makes a pinhole camera,
                      // >0.0f defines a hole of given radius

    // Convert camera's local space to world space
    Vec4 cameraToWorld(const Vec4 &v) const;

    // Get random DoF displacement (simulate bigger camera hole)
    Vec4 getDoFDisplacement() const;

    // Trace the path followed by the cameraRay (multiple hits etc)
    RGBColor tracePath(const Ray &cameraRay, const Scene &scene,
                       const RGBColor &backgroundColor) const;

    // Trace multiple (ppp) rays to a pixel defined by box [p0, p1]
    // and return mean color luminance that enters the pixel
    RGBColor tracePixel(const Vec4 &d0, const Vec4 &deltaX, const Vec4 &deltaY,
                        int ppp, const Scene &scene,
                        const RGBColor &backgroundColor) const;

   public:
    Camera(Vec4 _origin, Vec4 _forward, Vec4 _up, Vec4 _right)
        : origin(_origin),
          forward(_forward),
          up(_up),
          right(_right),
          dofRadius(0.0f) {}

    Camera(Vec4 _origin, Vec4 _forward, Vec4 _up, float _aspectRatio)
        : Camera(_origin, _forward, _up,
                 cross(_forward, _up).normalize() * _up.module() * _aspectRatio) {}

    void setDepthOfField(const float radius) {
        this->dofRadius = radius;
    }

    // Generate an image render (see implementation)
    PPMImage render(int width, int height, int ppp, const Scene &scene,
                    const RGBColor &backgroundColor) const;
};
