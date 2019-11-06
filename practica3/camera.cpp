//
// Created by Yasmina on 30/10/2019.
//

#include "camera.h"

Vec4 Camera::cameraToWorld(const Vec4 &v) {
    // static to it doesn't construct the matrix more than once
    static Mat4 cob(right, up, forward, origin);
    return cob * v;
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays
PPMImage Camera::render(
    int width, int height, int rpp,
    const std::vector<std::shared_ptr<Figures::Figure>> &scene,
    const RGBColor &backgroundColor) {
    // Initialize image with width/height and bg color
    PPMImage result(width, height);
    result.fillPixels(backgroundColor);

    // Iterate through [-1, 1] * [-1, 1] (camera's local space)
    // First point is at (-1 + 1/w, 1 - 1/h, 1) in local space
    Vec4 local(-1.0f + 1.0f / width, 1.0f - 1.0f / height, 1.0f, 0.0f);
    // Distance to next horizontal/vertical pixel
    const Vec4 deltaX(2.0f / width, 0, 0.0f, 0.0f);
    const Vec4 deltaY(0, -2.0f / height, 0.0f, 0.0f);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Ray from camera's origin to pixel's center
            Ray cameraRay(this->origin, cameraToWorld(local).normalize());
            float minDistance = std::numeric_limits<float>::max();
            // Intersect with all figures in scene
            for (auto const &figure : scene) {
                RayHit hit;
                if (figure->intersection(cameraRay, hit) &&
                    hit.distance < minDistance) {
                    // Only save intersection if hits the closest object
                    minDistance = hit.distance;
                    result.setPixel(x, y, hit.color);
                }
            }
            // Iterate thorugh next x pixel
            local = local + deltaX;
        }
        // Reset X value, iterate through next y pixel
        local.x = -1.0f + 1.0f / width;
        local = local + deltaY;
    }

    // Result is saved in PPM image's data
    return result;
}