#pragma once

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
    // Clamp all incoming values to [0, 1] range
    class FClamp1 : public Function {
       public:
        float map(float in) override { return in > 1.0f ? 1.0f : in; }
    };
    // Equalize [0, max] range to [0, 1]
    class FEqualize : public Function {
        float max;
       public:
        FEqualize(float _max) : max(_max) {}
        float map(float in) override { return in / max; }
    };

    ToneMapper();
    ToneMapper(ToneMapper::Function *_f) : f(_f) {}

    std::unique_ptr<Function> f;

   public:
    static ToneMapper CLAMP_1() {
        ToneMapper tm(new ToneMapper::FClamp1());
        return tm;
    }
    static ToneMapper EQUALIZE(float max) {
        ToneMapper tm(new ToneMapper::FEqualize(max));
        return tm;
    }
    float map(float in) { return f->map(in); }
};