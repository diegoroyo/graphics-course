#pragma once

#include <cassert>
#include <cmath>
#include <iostream>

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
    constexpr RGBColor() : r(0.0f), g(0.0f), b(0.0f) {}
    constexpr RGBColor(const RGBColor &color) = default;
    constexpr RGBColor(float _r, float _g, float _b) : r(_r), g(_g), b(_b) {}
    void setValues(const RGBColor &other);
    void setValues(const float r, const float g, const float b);
    constexpr RGBColor operator+(const RGBColor &other) const {
        return RGBColor(r + other.r, g + other.g, b + other.b);
    }
    constexpr RGBColor operator*(const RGBColor &other) const {
        return RGBColor(r * other.r, g * other.g, b * other.b);
    }
    constexpr RGBColor operator*(const float i) const {
        return RGBColor(r * i, g * i, b * i);
    }
    inline float max() const { return std::fmax(r, std::fmax(g, b)); }
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