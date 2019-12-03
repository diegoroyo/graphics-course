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

   private:
    constexpr Material(bool _emitsLight, const RGBColor &_emission,
                       const RGBColor &_kd, float _ks, float _alpha)
        : emitsLight(_emitsLight),
          emission(_emission),
          kd(_kd),
          ks(_ks),
          alpha(_alpha) {}

   public:
    static MaterialPtr light(const RGBColor &_emission) {
        return MaterialPtr(
            new Material(true, _emission, RGBColor::Black, 0.0f, 0.0f));
    }

    static MaterialPtr phong(const RGBColor &_kd, const float _ks,
                             const float _alpha) {
        return MaterialPtr(
            new Material(false, RGBColor::Black, _kd, _ks, _alpha));
    }

    static MaterialPtr none() { return nullptr; }
};