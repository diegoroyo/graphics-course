#pragma once

#include "math/geometry.h"
#include "math/rgbcolor.h"

struct PointLight {
    Vec4 point;
    RGBColor emission;
    PointLight(const Vec4 &_point, const RGBColor &_emission)
        : point(_point), emission(_emission) {}
};

struct Photon {
    Vec4 point;
    Vec4 inDirection;
    RGBColor flux;
    Photon(const Vec4 &_point, const Vec4 &_inDirection, const RGBColor &_flux)
        : point(_point), inDirection(_inDirection), flux(_flux) {}
};