#pragma once

#include <memory>
struct HomIsoMedium;
typedef std::shared_ptr<HomIsoMedium> HomIsoMediumPtr;

#include "camera/medium.h"
#include "camera/ray.h"
#include "camera/rayhit.h"
#include "filter.h"
#include "photonkdtree.h"

// Homogeneous isotropic scattering
struct HomIsoMedium : public Medium {
    const float kExtinction;  // absorption + scattering coeffs.
    const float kScattering;  // scattering coeff.
    const float deltaD;
    const RGBColor inScatterConstant;

   private:
    constexpr HomIsoMedium(float _refractiveIndex, float _kExtinction,
                           float _kScattering, float _deltaD)
        : Medium(_refractiveIndex),
          kExtinction(_kExtinction),
          kScattering(_kScattering),
          deltaD(_deltaD) {}

    static inline HomIsoMediumPtr cast(const MediumPtr &medium) {
        return std::dynamic_pointer_cast<HomIsoMedium>(medium);
    }

   public:
    static MediumPtr create(float _refractiveIndex, float _kExtinction,
                            float _kScattering, float _deltaD) {
        return MediumPtr(new HomIsoMedium(_refractiveIndex, _kExtinction,
                                          _kScattering, _deltaD));
    }

    static inline void rayMarchEmit(const Ray &ray, const RayHit &hit,
                                    PhotonKdTreeBuilder &volume) {
        // Participative media
        HomIsoMediumPtr pmedium = cast(ray.medium);
        if (pmedium != nullptr) {
            pmedium->fRayMarchEmit(ray, hit, volume);
        }
    }

    void fRayMarchEmit(const Ray &ray, const RayHit &hit,
                       PhotonKdTreeBuilder &volume) const;
    RGBColor fRayMarchTrace(const RGBColor &light, const Ray &ray,
                            const RayHit &hit) const;
};