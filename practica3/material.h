#pragma once

#include "rgbcolor.h"

// typedef for material pointers
#include <memory>
class Material;
typedef std::shared_ptr<Material> MaterialPtr;

class Material {
   public:
    const bool emitsLight;
    const RGBColor emission;
    const RGBColor kd;
    const float ks;
    const float alpha;
    const float ksp;
    const float krp;
    const float mediumRefraction;

   private:
    constexpr Material(bool _emitsLight, const RGBColor &_emission,
                       const RGBColor &_kd, float _ks, float _alpha, float _ksp,
                       float _krp, float _mediumRefraction)
        : emitsLight(_emitsLight),
          emission(_emission),
          kd(_kd),
          ks(_ks),
          alpha(_alpha),
          ksp(_ksp),
          krp(_krp),
          mediumRefraction(_mediumRefraction) {}

   public:
    static MaterialPtr light(const RGBColor &_emission) {
        return MaterialPtr(new Material(true, _emission, RGBColor::Black, 0.0f,
                                        0.0f, 0.0f, 0.0f, 0.0f));
    }

    static MaterialPtr phong(const RGBColor &_kd, float _ks,
                             float _alpha = 1.0f, float _ksp = 0.0f,
                             float _krp = 0.0f,
                             float _mediumRefraction = 0.0f) {
        return MaterialPtr(new Material(false, RGBColor::Black, _kd, _ks,
                                        _alpha, _ksp, _krp, _mediumRefraction));
    }

    static MaterialPtr none() { return nullptr; }
};