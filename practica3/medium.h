#pragma once

#include <memory>
struct Medium;
typedef std::shared_ptr<Medium> MediumPtr;

// Used to save properties (refractive index, participation/scattering)
// about the medium
struct Medium {
    float refractiveIndex;

   private:
    constexpr Medium(float _refractiveIndex)
        : refractiveIndex(_refractiveIndex) {}

   public:
    static MediumPtr air;
    static MediumPtr create(float _refractiveIndex) {
        return MediumPtr(new Medium(_refractiveIndex));
    }
};