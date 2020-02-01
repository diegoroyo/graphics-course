#pragma once

#include "camera/medium.h"
#include "camera/ray.h"
#include "camera/rayhit.h"
#include "filter.h"
#include "photonkdtree.h"

// Used to save properties (refractive index, participation/scattering)
// about the medium
struct PMedium : public Medium {
    const float kExtinction;  // absorption + scattering coeffs.
    const float deltaD;       // distance between stops in ray marching

   private:
    constexpr PMedium(float _refractiveIndex, float _kExtinction, float _deltaD)
        : Medium(_refractiveIndex, true),
          kExtinction(_kExtinction),
          deltaD(_deltaD) {}

   public:
    static MediumPtr create(float _refractiveIndex, float _kExtinction,
                            float _deltaD) {
        return MediumPtr(new PMedium(_refractiveIndex, _kExtinction, _deltaD));
    }

    void rayMarchEmit(const Ray &ray, const RayHit &hit,
                      PhotonKdTreeBuilder &volume) const;
    void rayMarchTrace(const Ray &ray, const RayHit &hit,
                       const PhotonKdTree &volume,
                       const FilterPtr &filter) const;
};