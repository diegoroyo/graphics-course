#pragma once

#include <cmath>
#include <memory>
#include "ppmimage.h"
#include "rgbcolor.h"

// Implementation of different tone-mapping functions
// first set-up the function you want, then you can use it
// with tonemapfunction.map(input)

class ToneMapper {
   private:
    // Base for all tone-mapping functions
    class Function {
       public:
        virtual float map(float in) const { return in; }
    };
    // Clamps values higher than max
    // values lower than max are put in a gamma curve
    class FClampGamma : public Function {
        float max;
        float gamma;

       public:
        FClampGamma(float _max, float _gamma = 1.0f)
            : max(_max), gamma(_gamma) {}
        float map(float in) const override {
            if (in > max) {
                // clamp
                return 1.0f;
            } else {
                // equalize and put in gamma curve (x^gamma)
                return std::pow(in / max, gamma);
            }
        }
    };
    // Reinhard02 tone-mapper (see paper)
    class FReinhard02 : public Function {
        float max;
        float minWhiteSq;

       public:
        FReinhard02(float _max, float _minWhite)
            : max(_max), minWhiteSq(_minWhite * _minWhite) {}
        float map(float in) const override {
            float l = in / max;
            return (l * (1.0f + l / minWhiteSq)) / (1.0f + l);
        }
    };

    ToneMapper();
    ToneMapper(ToneMapper::Function *_f) : f(_f) {}

    std::unique_ptr<Function> f;

   public:
    static ToneMapper CLAMP_1() {
        ToneMapper tm(new ToneMapper::FClampGamma(1.0f));
        return tm;
    }
    static ToneMapper EQUALIZE_CLAMP(float max) {
        ToneMapper tm(new ToneMapper::FClampGamma(max));
        return tm;
    }
    static ToneMapper CLAMP_GAMMA(float max, float gamma = 2.2f) {
        ToneMapper tm(new ToneMapper::FClampGamma(max, gamma));
        return tm;
    }
    static ToneMapper REINHARD_02(float max, float minWhite) {
        ToneMapper tm(new ToneMapper::FReinhard02(max, minWhite));
        return tm;
    }
    float map(float in) const { return f->map(in); }
};