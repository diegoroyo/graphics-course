#pragma once

#include "camera/raytracer.h"
#include "io/ppmimage.h"

// monte-carlo based pathtracer
class PathTracer : public RayTracer {
    PPMImage render;
    const int ppp;

    // Trace the path followed by the cameraRay (multiple hits etc)
    RGBColor traceRay(const Ray &ray, const Scene &scene) const;

   public:
    PathTracer(int _ppp, const Film &film)
        : ppp(_ppp),
          render(film.width, film.height, std::numeric_limits<int>::max()) {}

    void tracePixel(const int px, const int py, const Film &film,
                    const Scene &scene) override;

    PPMImage &result() override;
};