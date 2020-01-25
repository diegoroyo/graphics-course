#pragma once

#include <memory>
class Filter;
typedef std::shared_ptr<Filter> FilterPtr;

#include "math/geometry.h"

class Filter {
   public:
    virtual float photonTerm(const Vec4 &center, const Vec4 &photon,
                             const float r, const int kNN) const {
        return 1.0f;
    }
    virtual float kTerm(const int kNN) const { return 1.0f; }
};

class ConeFilter : public Filter {
   public:
    virtual float photonTerm(const Vec4 &center, const Vec4 &photon,
                             const float r, const int kNN) const {
        return 1.0f - ((photon - center).module() / (kNN * r));
    }
    virtual float kTerm(const int kNN) const {
        return (1.0f - (2.0f / (3.0f * kNN)));
    }
};