#pragma once

#include <cmath>
#include <memory>

// Implementation of different tone-mapping functions
// first set-up the function you want, then you can use it
// with tonemapfunction.map(input)

class ToneMapper {
   private:
    // Base for all tone-mapping functions
    class Function {
       public:
        virtual float map(float in) { return in; }
    };
    // Clamps values higher than max
    // values lower than max are put in a gamma curve
    class FClampGamma : public Function {
        float max;
        float gamma;

       public:
        FClampGamma(float _max, float _gamma = 1.0f) : max(_max), gamma(_gamma) {}
        float map(float in) override {
            if (in > max) {
                // clamp
                return max;
            } else {
                // equalize and put in gamma curve (x^gamma)
                return std::pow(in / max, gamma);
            }
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
    float map(float in) { return f->map(in); }
};