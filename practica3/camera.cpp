//
// Created by Yasmina on 30/10/2019.
//

#include "camera.h"

PPMImage Camera::render(int width, int height, int rpp,
                        const std::vector<Figures::Figure> &scene,
                        const RGBColor &backgroundColor) {
    PPMImage result(width, height);
    result.fillPixels(backgroundColor);

    // TODO
    // calculate all ray directions using
    // origin, forward, up, right, width, height & rpp
    // calculate first & interpolate others

    // for each (rayOrigin, rayDirection)
    //   for each figure in scene
    //     intersect ray with figure

    // the color of a pixel is the mean of the color of the rays
    // passing through that pixel
    // result.setPixel(x, y, figure.color);

    return result;
}