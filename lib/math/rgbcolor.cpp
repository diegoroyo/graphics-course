#include "rgbcolor.h"
#include <cmath>

const RGBColor RGBColor::Black(0.0f, 0.0f, 0.0f);
const RGBColor RGBColor::White(1.0f, 1.0f, 1.0f);
const RGBColor RGBColor::Red(1.0f, 0.0f, 0.0f);
const RGBColor RGBColor::Green(0.0f, 1.0f, 0.0f);
const RGBColor RGBColor::Blue(0.0f, 0.0f, 1.0f);
const RGBColor RGBColor::Yellow(1.0f, 1.0f, 0.0f);
const RGBColor RGBColor::Magenta(1.0f, 0.0f, 1.0f);
const RGBColor RGBColor::Cyan(0.0f, 1.0f, 1.0f);

void RGBColor::setValues(const RGBColor &other) {
    this->r = other.r;
    this->g = other.g;
    this->b = other.b;
}

void RGBColor::setValues(const float r, const float g, const float b) {
    this->r = r;
    this->g = g;
    this->b = b;
}

RGBColor RGBColor::rgb2lab(float max) const {
    RGBColor rgb, lab;
    float x, y, z;

    // Convert RGB to [0..1] range
    rgb = *this * (1.0f / max);

    // Apply gamma correction
    if (rgb.r > 0.04045f) {
        rgb.r = std::pow(((rgb.r + 0.055f) / 1.055f), 2.4f);
    } else {
        rgb.r /= 12.92f;
    }
    if (rgb.g > 0.04045f) {
        rgb.g = std::pow(((rgb.g + 0.055f) / 1.055f), 2.4f);
    } else {
        rgb.g /= 12.92f;
    }
    if (rgb.b > 0.04045f) {
        rgb.b = std::pow(((rgb.b + 0.055f) / 1.055f), 2.4f);
    } else {
        rgb.b /= 12.92f;
    }

    // Convert modified RGB to XYZ
    x = rgb.r * 0.4124f + rgb.g * 0.3576f + rgb.b * 0.1805f;
    y = rgb.r * 0.2126f + rgb.g * 0.7152f + rgb.b * 0.0722f;
    z = rgb.r * 0.0193f + rgb.g * 0.1192f + rgb.b * 0.9505f;

    // Scale to reference white (sRGB uses D50)
    x = x * 100.0f / 96.4212f;
    // y does not need correcting
    z = z * 100.0f / 82.5188f;

    // Convert XYZ to Lab
    if (x > 0.008856f) {
        x = std::pow(x, 1.0f / 3.0f);
    } else {
        x = 7.787f * x + 16.0f / 116.0f;
    }
    if (y > 0.008856f) {
        y = std::pow(y, 1.0f / 3.0f);
    } else {
        y = 7.787f * y + 16.0f / 116.0f;
    }
    if (z > 0.008856f) {
        z = std::pow(z, 1.0f / 3.0f);
    } else {
        z = 7.787f * z + 16.0f / 116.0f;
    }

    lab.lab.l = (116.0f * y) - 16.0f;  // L
    lab.lab.a = 500.0f * (x - y);      // a
    lab.lab.b = 200.0f * (y - z);      // b

    // Lab's luminance attribute comes in [0..100] range
    // Re-scale it to [0..max] range for later tone mapping
    lab.lab.l = lab.lab.l * max / 100.0f;

    return lab;
}

RGBColor RGBColor::lab2rgb(float max) const {
    RGBColor rgb, lab;
    float x, y, z, x3, y3, z3;

    // Re-scale luminance attribute from [0..1] to [0..100]
    lab = *this;
    lab.lab.l = lab.lab.l * 100.0f;

    // Convert Lab to XYZ data
    y = (lab.lab.l + 16.0f) / 116.0f;
    y3 = std::pow(y, 3);
    x = (lab.lab.a / 500.0f) + y;
    x3 = std::pow(x, 3);
    z = y - (lab.lab.b / 200.0f);
    z3 = std::pow(z, 3);

    if (y3 > 0.008856f) {
        y = y3;
    } else {
        y = (y - 16.0f / 116.0f) / 7.787f;
    }
    if (x3 > 0.008856f) {
        x = x3;
    } else {
        x = (x - 16.0f / 116.0f) / 7.787f;
    }
    if (z3 > 0.008856f) {
        z = z3;
    } else {
        z = (z - 16.0f / 116.0f) / 7.787f;
    }

    // Correct to reference white D50
    x = x * 96.4212f / 100.0f;
    // y does not need correcting
    z = z * 82.5188f / 100.0f;

    rgb.r = x * 3.2406f + y * -1.5372f + z * -0.4986f;
    rgb.g = x * -0.9689f + y * 1.8758f + z * 0.0415f;
    rgb.b = x * 0.0557f + y * -0.2040f + z * 1.0570f;

    // Re-apply gamma correction
    if (rgb.r > 0.0031308f) {
        rgb.r = 1.055f * std::pow(rgb.r, 1.0f / 2.4f) - 0.055f;
    } else {
        rgb.r *= 12.92f;
    }
    if (rgb.g > 0.0031308f) {
        rgb.g = 1.055f * std::pow(rgb.g, 1.0f / 2.4f) - 0.055f;
    } else {
        rgb.g *= 12.92f;
    }
    if (rgb.b > 0.0031308f) {
        rgb.b = 1.055f * std::pow(rgb.b, 1.0f / 2.4f) - 0.055f;
    } else {
        rgb.b *= 12.92f;
    }

    // Clamp values to [0..1] range
    rgb.r = std::fmax(0.0f, rgb.r);
    rgb.r = std::fmin(1.0f, rgb.r);
    rgb.g = std::fmax(0.0f, rgb.g);
    rgb.g = std::fmin(1.0f, rgb.g);
    rgb.b = std::fmax(0.0f, rgb.b);
    rgb.b = std::fmin(1.0f, rgb.b);

    return rgb;
}

std::ostream &operator<<(std::ostream &s, const RGBColor &c) {
    s << "(" << c.r << ", " << c.g << ", " << c.b << ")";
    return s;
}