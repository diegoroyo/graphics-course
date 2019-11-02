//
// Created by Yasmina on 30/10/2019.
//

#pragma once

#include "../lib/geometry.h"
#include "../lib/ppmimage.h"
#include "figures.h"

class Camera {
    Vec4 origin, forward, up, right;

   public:
    Camera(Vec4 _origin, Vec4 _forward, Vec4 _up, Vec4 _right)
        : origin(_origin), forward(_forward), up(_up), right(_right) {}
        //TODO: add constructor from height and width using ImageAspectRatio


    // Generate an image render (see implementation)
    PPMImage render(int width, int height, int rpp,
                    const std::vector<Figures::Figure> &scene,
                    const RGBColor &backgroundColor);
};
