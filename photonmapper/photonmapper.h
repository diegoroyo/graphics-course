#pragma once

#include "camera/homambmedium.h"
#include "camera/raytracer.h"
#include "filter.h"
#include "io/ppmimage.h"
#include "photonemitter.h"
#include "photonkdtree.h"

class PhotonMapper : public RayTracer {
    static const int MAX_LEVEL = 100;
    const int shotRays;
    const int ppp, kNeighbours, kcNeighbours, kvNeighbours;
    const PhotonKdTree photons, caustics, volume;
    const bool directShadowRays;
    PPMImage render;
    FilterPtr filter;

    // Special direct light calculations (participative media)
    RGBColor directLightMedium(const Scene &scene, const RayHit &hit,
                               const Vec4 &wo) const;

    // Search kNN photons on given tree and return radiance estimate
    RGBColor treeSearch(const PhotonKdTree &tree, const int kNN,
                        const RayHit &hit, const Vec4 &outDirection) const;

    // Trace the path followed by the cameraRay (multiple hits etc)
    RGBColor traceRay(const Ray &ray, const Scene &scene,
                      const int level = 1) const;

   public:
    PhotonMapper(int _ppp, const Film &film, PhotonEmitter &_emitter,
                 int _kNeighbours, int _kcNeighbours, int _kvNeighbours,
                 const FilterPtr &_filter)
        : shotRays(_emitter.shotRays),
          ppp(_ppp),
          directShadowRays(!_emitter.hasDirectLight()),
          kNeighbours(_kNeighbours),
          kcNeighbours(_kcNeighbours),
          kvNeighbours(_kvNeighbours),
          render(film.width, film.height, std::numeric_limits<int>::max()),
          photons(_emitter.getPhotonsTree()),
          caustics(_emitter.getCausticsTree()),
          volume(_emitter.getVolumeTree()),
          filter(_filter) {}

    void tracePixel(const int px, const int py, const Film &film,
                    const Scene &scene) override;

    PPMImage &result() override;
};