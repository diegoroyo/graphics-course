#pragma once

class ToneMapper;

#include <cmath>
#include <memory>
#include "io/ppmimage.h"
#include "math/rgbcolor.h"

// Implementation of different tone-mapping functions
// first set-up the function you want, then you can use it
// with tonemapfunction.map(input)

class ToneMapper {
   private:
    // Base for all tone-mapping functions
    class Function {
       public:
        virtual float map(float in) const = 0;
    };

    // Clamps values higher than max
    // values lower than max are put in a gamma curve
    class FClampGamma : public Function {
        const float max, gamma;

       public:
        FClampGamma(float _max, float _gamma = 1.0f)
            : max(_max), gamma(_gamma) {}
        float map(float in) const override;
    };

    // Reinhard02 tone-mapper (see paper)
    class FReinhard02 : public Function {
        const float useLab;      // LAB or RGB channels
        const float minWhiteSq;  // smallest luminance that
                                 // will be mapped to white
        float alphaMean;         // alpha / exp(mean(log(Lw)))
        const float DELTA = 1e-6f;

       public:
        FReinhard02(const PPMImage &_image, bool _useLab, float _minWhite,
                    float _alpha);
        float map(float in) const override;
    };

    ToneMapper();
    ToneMapper(ToneMapper::Function *_f) : f(_f) {}

    std::unique_ptr<Function> f;

   public:
    static ToneMapper CLAMP_1();
    static ToneMapper EQUALIZE_CLAMP(float max);
    static ToneMapper CLAMP_GAMMA(float max, float gamma = 2.2f);
    static ToneMapper REINHARD_02(const PPMImage &image, bool useLab,
                                  float minWhite, float alpha = 0.18f);

    float map(float in) const;
};