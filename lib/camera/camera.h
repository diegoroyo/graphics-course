#pragma once

#include <chrono>
#include <cmath>
#include <future>
#include <memory>
#include <string>
#include "camera/film.h"
#include "camera/progress.h"
#include "camera/raytracer.h"
#include "io/ppmimage.h"
#include "math/geometry.h"
#include "math/random.h"
#include "scene/figures.h"
#include "scene/scene.h"

class Camera {
    const Film film;
    const RayTracerPtr rayTracer;
    const float dofRadius;  // 0.0f makes a pinhole camera,
                            // >0.0f defines a hole of given radius

   public:
    Camera(const Film &_film, const RayTracerPtr &_rayTracer,
           const float _dofRadius = 0.0f)
        : film(_film), rayTracer(_rayTracer), dofRadius(_dofRadius) {}

    // Trace ppp rays from all the pixels of the film
    void tracePixels(const Scene &scene) const;

    // Store result in said filename
    void storeResult(const std::string &filename) const;
};
