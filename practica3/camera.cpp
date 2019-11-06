//
// Created by Yasmina on 30/10/2019.
//

#include "camera.h"

Vec4 Camera::cameraToWorld(const Vec4 &v) {
    static Mat4 cob(right, up, forward, origin);
    return cob * v;
}

PPMImage Camera::render(int width, int height, int rpp,
                        const std::vector<Figures::Figure> &scene,
                        const RGBColor &backgroundColor) {
    PPMImage result(width, height);
    result.fillPixels(backgroundColor);

    // In order: pixel X NCD,pixel Y NCD,pixel X Screen, pixel Y Screen
    float xNCD, yNCD, xS, yS;
    const int yScreenRange = (int)(this->up.module());
    const int xScreenRange = (int)(this->right.module());
    const int distanceToScreen = (int)(this->forward.module());
    Vec4 rayO = this->origin;

    // TODO
    // calculate all ray directions using
    // origin, forward, up, right, width, height & rpp
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays
    // calculate first & interpolate others

    for (int i = 0; i < height; ++i) {  // y
        yNCD = (i + 0.5) / height;
        yS = (yNCD * 2 * yScreenRange) - yScreenRange;
        for (int j = 0; j < width; ++j) {  // x
            xNCD = (j + 0.5) / width;
            xS = (xNCD * 2 * xScreenRange) - xScreenRange;
            // TODO  transform generated ray from camera to world basis
            Vec4 dir = Vec4(xS, yS, distanceToScreen, 0).normalize();
            this->cameraToWorld(dir);
            for (Figures::Figure f : scene) {
                RayHit hit;
                float minDistance = 999999.9f;
                if (f.intersection(Ray(this->origin, dir), hit) &&
                    hit.distance < minDistance) {
                    minDistance = hit.distance;
                    result.setPixel(j, i, hit.color);
                }
            }
        }
    }  // FIX: do all in this loop??

    // for each (rayOrigin, rayDirection)
    //   for each figure in scene
    //     intersect ray with figure

    // the color of a pixel is the mean of the color of the rays
    // passing through that pixel
    // result.setPixel(x, y, figure.color);

    return result;
}