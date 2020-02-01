#pragma once

#include "camera/raytracer.h"
#include "filter.h"
#include "io/ppmimage.h"
#include "photonemitter.h"
#include "photonkdtree.h"

class PhotonMapper : public RayTracer {
    const float epp;
    const int ppp, kNeighbours, kcNeighbours;
    const PhotonKdTree photons, caustics;
    const bool directShadowRays;
    PPMImage render;
    FilterPtr filter;

    // Search kNN photons on given tree and return radiance estimate
    RGBColor treeSearch(const PhotonKdTree &tree, const int kNN,
                        const RayHit &hit, const Vec4 &outDirection) const;

    // Trace the path followed by the cameraRay (multiple hits etc)
    RGBColor traceRay(const Ray &ray, const Scene &scene) const;

   public:
    PhotonMapper(int _ppp, const Film &film, PhotonEmitter &_emitter,
                 int _kNeighbours, int _kcNeighbours, const FilterPtr &_filter)
        : epp(_emitter.energyPerPhoton()),
          ppp(_ppp),
          directShadowRays(!_emitter.hasDirectLight()),
          kcNeighbours(_kcNeighbours),
          kNeighbours(_kNeighbours),
          render(film.width, film.height, std::numeric_limits<int>::max()),
          photons(_emitter.getPhotonsTree()),
          caustics(_emitter.getCausticsTree()),
          filter(_filter) {}

    void tracePixel(const int px, const int py, const Film &film,
                    const Scene &scene) override;

    PPMImage &result() override;
};