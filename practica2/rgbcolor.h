#pragma once

#include <iostream>
#include <cassert>

class RGBColor {
   public:
    union {
        struct {
            float r, g, b;
        };
        struct {
            float x, y, z;
        } xyz;
        struct {
            float l, a, b;
        } lab;
    };
    RGBColor() : r(0.0f), g(0.0f), b(0.0f) {}
    RGBColor(const RGBColor &color) = default;
    RGBColor(float _r, float _g, float _b) : r(_r), g(_g), b(_b) {}
    void setValues(const RGBColor &other);
    void setValues(const float r, const float g, const float b);
    inline RGBColor operator*(const float i) const {
        return RGBColor(r * i, g * i, b * i);
    }
    // Transformation from RGB to CIE-L*ab format
    RGBColor rgb2lab(float max) const;
    // Transformation from CIE-L*ab to RGB format
    RGBColor lab2rgb(float max) const;
    friend std::ostream &operator<<(std::ostream &s, const RGBColor &c);

    // Colores predefinidos
    static const RGBColor Black;
    static const RGBColor White;
    static const RGBColor Red;
    static const RGBColor Green;
    static const RGBColor Blue;
    static const RGBColor Yellow;
    static const RGBColor Magenta;
    static const RGBColor Cyan;
};