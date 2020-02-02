#pragma once

#include <memory>
struct HomAmbMedium;
typedef std::shared_ptr<HomAmbMedium> HomAmbMediumPtr;

#include "camera/medium.h"
#include "camera/ray.h"
#include "camera/rayhit.h"

// Homogeneous ambient scattering
struct HomAmbMedium : public Medium {
    const float kExtinction;  // absorption + scattering coeffs.
    const float kScattering;  // scattering coeff.
    const RGBColor inScatterConstant;

   private:
    constexpr HomAmbMedium(float _refractiveIndex, float _kExtinction,
                           float _kScattering, const RGBColor &_inScatter)
        : Medium(_refractiveIndex),
          kExtinction(_kExtinction),
          kScattering(_kScattering),
          inScatterConstant(_inScatter) {}

    RGBColor fApplyLight(const RGBColor &light, const Ray &ray,
                         const RayHit &hit) const;

   public:
    static MediumPtr create(float _refractiveIndex, float _kExtinction,
                            float _kScattering, const RGBColor &_inScatter) {
        return MediumPtr(new HomAmbMedium(_refractiveIndex, _kExtinction,
                                          _kScattering, _inScatter));
    }

    static inline void applyLight(RGBColor &light, const Ray &ray,
                                  const RayHit &hit) {
        HomAmbMediumPtr pmedium =
            std::dynamic_pointer_cast<HomAmbMedium>(ray.medium);
        if (pmedium != nullptr) {
            light = pmedium->fApplyLight(light, ray, hit);
        }
    }
};