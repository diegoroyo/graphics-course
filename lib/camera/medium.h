#pragma once

#include <memory>
struct Medium;
typedef std::shared_ptr<Medium> MediumPtr;

// Used to save properties (refractive index, participation/scattering)
// about the medium
struct Medium {
    const float refractiveIndex;
    const bool participative;

   protected:
    constexpr Medium(float _refractiveIndex, bool _participative = false)
        : refractiveIndex(_refractiveIndex), participative(_participative) {}

   public:
    static MediumPtr air;
    static MediumPtr create(float _refractiveIndex) {
        return MediumPtr(new Medium(_refractiveIndex));
    }
};