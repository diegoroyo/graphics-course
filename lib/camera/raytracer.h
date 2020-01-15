#pragma once

#include <memory>
class RayTracer;
typedef std::shared_ptr<RayTracer> RayTracerPtr;

#include "camera/film.h"
#include "camera/ray.h"
#include "scene/scene.h"

// abstract class that handles rays with scene
class RayTracer {
   public:
    virtual void tracePixel(const int px, const int py, const Film &film,
                            const Scene &scene) = 0;
    virtual PPMImage &result() = 0;
};