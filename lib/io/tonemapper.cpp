#include "tonemapper.h"

/// Tone-mapping functions ///

// float ToneMapper::Function::map(float in) const { return in; }

float ToneMapper::FClampGamma::map(float in) const {
    if (in > max) {
        // clamp
        return 1.0f;
    } else {
        // equalize and put in gamma curve (x^gamma)
        return std::pow(in / max, gamma);
    }
}

ToneMapper::FReinhard02::FReinhard02(const PPMImage &_image, bool _useLab,
                                     float _minWhite, float _alpha)
    : useLab(_useLab), minWhiteSq(_minWhite * _minWhite) {
    float sum = 0.0;
    for (int y = 0; y < _image.height; y++) {
        for (int x = 0; x < _image.width; x++) {
            RGBColor pixel = _image.getPixel(x, y);
            float l;
            if (useLab) {
                l = pixel.rgb2lab(_image.max).lab.l;
            } else {
                l = 0.2126 * pixel.r + 0.7152 * pixel.g + 0.0722 * pixel.b;
            }
            sum += logf(DELTA + l);
        }
    }
    alphaMean = _alpha / expf(sum / (_image.height * _image.width));
}

float ToneMapper::FReinhard02::map(float in) const {
    float lm = in * alphaMean;
    return (lm * (1.0f + lm / minWhiteSq)) / (1.0f + lm);
}

/// Tone-mapping constructors ///

ToneMapper ToneMapper::CLAMP_1() {
    ToneMapper tm(new ToneMapper::FClampGamma(1.0f));
    return tm;
}

ToneMapper ToneMapper::EQUALIZE_CLAMP(float max) {
    ToneMapper tm(new ToneMapper::FClampGamma(max));
    return tm;
}

ToneMapper ToneMapper::CLAMP_GAMMA(float max, float gamma) {
    ToneMapper tm(new ToneMapper::FClampGamma(max, gamma));
    return tm;
}

ToneMapper ToneMapper::REINHARD_02(const PPMImage &image, bool useLab,
                                   float minWhite, float alpha) {
    ToneMapper tm(new ToneMapper::FReinhard02(image, useLab, minWhite, alpha));
    return tm;
}

float ToneMapper::map(float in) const { return f->map(in); }