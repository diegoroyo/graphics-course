//
// Created by Yasmina on 30/10/2019.
//

#include "camera.h"

Vec4 Camera::cameraToWorld(const Vec4 &v) {
    static Mat4 cob(right, up, forward, origin);
    return cob * v;
}

PPMImage Camera::render(int width, int height, int rpp,
                        const std::vector<Figures::Figure*> &scene,
                        const RGBColor &backgroundColor) {
    PPMImage result(width, height);
    result.fillPixels(backgroundColor);

    // In order: pixel X NCD,pixel Y NCD,pixel X Screen, pixel Y Screen
    float xNCD, yNCD, xS, yS;
    const int yScreenRange = (this->up.module());
    const int xScreenRange = (this->right.module());
    const int distanceToScreen = (this->forward.module());
    Vec4 rayO = this->origin;

    // TODO
    // calculate all ray directions using
    // origin, forward, up, right, width, height & rpp
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays
    // calculate first & interpolate others

    Vec4 last (-1 + 1/width, 1 + 1/height , 1, 0);
    const Vec4 deltaX(2/width, 0,0,0);
    const Vec4 deltaY(0, 2/height,0,0);
    for (int y = 0; y < height; ++y) {  // y
       for (int x = 0; x < width; ++x) {  // x
            Vec4 dir = Vec4(last).normalize();
            dir=this->cameraToWorld(dir);
            float minDistance = 999999.9f;
            for (Figures::Figure* f : scene) {
                RayHit hit;
                if (f->intersection(Ray(this->origin, dir), hit) &&
                    hit.distance < minDistance) {
                    minDistance = hit.distance;
                    result.setPixel(x, y, hit.color);
                }
            }
            last = last + deltaX;
        }
       last.x=-1+1/width;
       last=last+deltaY;
    }

    // for each (rayOrigin, rayDirection)
    //   for each figure in scene
    //     intersect ray with figure

    // the color of a pixel is the mean of the color of the rays
    // passing through that pixel
    // result.setPixel(x, y, figure.color);

    return result;
}


