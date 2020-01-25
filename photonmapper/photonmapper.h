#pragma once

#include "camera/raytracer.h"
#include "filter.h"
#include "io/ppmimage.h"
#include "photonkdtree.h"

class PhotonMapper : public RayTracer {
    const int ppp, kNeighbours, kcNeighbours;
    const PhotonKdTree photons, caustics;
    PPMImage render;
    FilterPtr filter;

    // Search kNN photons on given tree and return radiance estimate
    RGBColor treeSearch(const PhotonKdTree &tree, const int kNN,
                        const RayHit &hit, const Vec4 &outDirection,
                        const EventPtr &event) const;

    // Trace the path followed by the cameraRay (multiple hits etc)
    RGBColor traceRay(const Ray &ray, const Scene &scene) const;

   public:
    PhotonMapper(int _ppp, const Film &film, int _kNeighbours,
                 int _kcNeighbours, const PhotonKdTree &_photons,
                 const PhotonKdTree &_caustics, const FilterPtr &_filter)
        : ppp(_ppp),
          kcNeighbours(_kcNeighbours),
          kNeighbours(_kNeighbours),
          render(film.width, film.height, std::numeric_limits<int>::max()),
          photons(_photons),
          caustics(_caustics),
          filter(_filter) {}

    void tracePixel(const int px, const int py, const Film &film,
                    const Scene &scene) override;

    PPMImage &result() override;
};