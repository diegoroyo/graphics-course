#pragma once

#include <memory>
class Filter;
typedef std::shared_ptr<Filter> FilterPtr;

#include "camera/rayhit.h"
#include "math/geometry.h"
#include "scene/light.h"

class Filter {
   public:
    virtual float photonTerm(const RayHit &hit, const Photon &photon,
                             const float r, const int kNN) const {
        return 1.0f;
    }
    virtual float kTerm(const int kNN) const { return 1.0f; }
};

class ConeFilter : public Filter {
   public:
    virtual float photonTerm(const RayHit &hit, const Photon &photon,
                             const float r, const int kNN) const {
        return 1.0f - ((photon.point - hit.point).module() / (kNN * r));
    }
    virtual float kTerm(const int kNN) const {
        // maybe all photons have negative cos value
        if (kNN == 0) {
            return 1.0f;
        } else {
            return (1.0f - (2.0f / (3.0f * kNN)));
        }
    }
};