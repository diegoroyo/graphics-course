#pragma once

#include "camera/medium.h"
#include "math/geometry.h"
#include "math/rgbcolor.h"

struct PointLight {
    const Vec4 point;
    const RGBColor emission;
    const MediumPtr medium;
    PointLight(const Vec4 &_point, const RGBColor &_emission,
               const MediumPtr _medium = Medium::air)
        : point(_point), emission(_emission), medium(_medium) {}
};

struct Photon {
    // Can't be const due to KdTree's nth_element function
    // (photon vector needs to be sorted)
    Vec4 point;
    Vec4 inDirection;
    RGBColor flux;
    Photon(const Vec4 &_point, const Vec4 &_inDirection, const RGBColor &_flux)
        : point(_point), inDirection(_inDirection), flux(_flux) {}
};