#pragma once

#include <memory>
struct HomIsoMedium;
typedef std::shared_ptr<HomIsoMedium> HomIsoMediumPtr;

#include "camera/medium.h"
#include "camera/ray.h"
#include "camera/rayhit.h"
#include "filter.h"
#include "math/random.h"
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
    RGBColor volumeSearch(const PhotonKdTree &volume, const int kNN,
                          const Vec4 &point) const;

    // true if ray is absorbed
    bool fRayEmit(const Scene &scene, const RGBColor &light, Ray &ray,
                  RayHit &hit, PhotonKdTreeBuilder &volume) const;
    RGBColor fApplyTransmittance(const RGBColor &light,
                                 const float distance) const;
    RGBColor fRayMarchTrace(const RGBColor &lightIn, const Ray &ray,
                            const RayHit &hit, const PhotonKdTree &volume,
                            const int kNN) const;

   public:
    static MediumPtr create(float _refractiveIndex, float _kExtinction,
                            float _kScattering, float _deltaD) {
        return MediumPtr(new HomIsoMedium(_refractiveIndex, _kExtinction,
                                          _kScattering, _deltaD));
    }

    static inline bool rayEmit(const Scene &scene, const RGBColor &light,
                               Ray &ray, RayHit &hit,
                               PhotonKdTreeBuilder &volume) {
        // Participative media
        HomIsoMediumPtr pmedium = cast(ray.medium);
        if (pmedium != nullptr) {
            return pmedium->fRayEmit(scene, light, ray, hit, volume);
        }
        return false;
    }

    static inline RGBColor rayMarch(const RGBColor &lightIn, const Ray &ray,
                                    const RayHit &hit,
                                    const PhotonKdTree &volume, const int kNN) {
        // Participative media
        HomIsoMediumPtr pmedium = cast(ray.medium);
        if (pmedium != nullptr) {
            return pmedium->fRayMarchTrace(lightIn, ray, hit, volume, kNN);
        }
        return lightIn;
    }
};