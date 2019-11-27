#pragma once

#include "../lib/rgbcolor.h"

class Material {
   public:
    const bool emitsLight;
    const RGBColor color;

    constexpr Material(bool _emitsLight, const RGBColor &_color)
        : emitsLight(_emitsLight), color(_color) {}

    static constexpr Material none() {
        return Material(false, RGBColor::Black);
    }
};