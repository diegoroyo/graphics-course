#include "rgbcolor.h"

const RGBColor RGBColor::Black(0.0f, 0.0f, 0.0f);
const RGBColor RGBColor::White(1.0f, 1.0f, 1.0f);
const RGBColor RGBColor::Red(1.0f, 0.0f, 0.0f);
const RGBColor RGBColor::Green(0.0f, 1.0f, 0.0f);
const RGBColor RGBColor::Blue(0.0f, 0.0f, 1.0f);
const RGBColor RGBColor::Yellow(1.0f, 1.0f, 0.0f);
const RGBColor RGBColor::Magenta(1.0f, 0.0f, 1.0f);
const RGBColor RGBColor::Cyan(0.0f, 1.0f, 1.0f);

void RGBColor::setValues(const RGBColor& other) {
    this->r = other.r;
    this->g = other.g;
    this->b = other.b;
}

void RGBColor::setValues(const float r, const float g, const float b) {
    this->r = r;
    this->g = g;
    this->b = b;
}