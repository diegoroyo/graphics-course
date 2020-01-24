#pragma once

#include "camera/raytracer.h"
#include "io/ppmimage.h"
#include "photonkdtree.h"

class PhotonMapper : public RayTracer {
    const int ppp, kNeighbours, kcNeighbours;
    const PhotonKdTree photons, caustics;
    PPMImage render;

    // Trace the path followed by the cameraRay (multiple hits etc)
    RGBColor traceRay(const Ray &ray, const Scene &scene) const;

   public:
    PhotonMapper(int _ppp, const Film &film, int _kNeighbours,
                 int _kcNeighbours, const PhotonKdTree &_photons,
                 const PhotonKdTree &_caustics)
        : ppp(_ppp),
          kcNeighbours(_kcNeighbours),
          kNeighbours(_kNeighbours),
          render(film.width, film.height, std::numeric_limits<int>::max()),
          photons(_photons),
          caustics(_caustics) {}

    void tracePixel(const int px, const int py, const Film &film,
                    const Scene &scene) override;

    PPMImage &result() override;
};