#include "camera.h"

Camera::Camera(const Vec4 _origin, const Vec4 _forward, const Vec4 _up, int width, int height) : origin(_origin),
forward(_forward), up(_up){
    forward=forward.normalize();
    up=up.normalize();
    right=cross(up,forward);
    //TODO: calcular base de la camara en coordenadas del mundo
}

Vec4 Camera::cameraToWorld(const Vec4 &v) {
    // static to it doesn't construct the matrix more than once
    static Mat4 cob(right, up, forward, origin);
    return cob * v;
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays
PPMImage Camera::render(int width, int height, int rpp,
                        const FigurePtr &sceneRootNode,
                        const RGBColor &backgroundColor) {
    // Initialize image with width/height and bg color
    PPMImage result(width, height);
    result.fillPixels(backgroundColor);

    double ratio=(double)width/(double)height;

    up.normalize();
    right.normalize();
    right=right*ratio;

    // Iterate through [-1, 1] * [-1, 1] (camera's local space)
    // First point is at ((-1 + 1/w)*ratio, 1 - 1/h, 1) in local space
    Vec4 direction = cameraToWorld(
            Vec4((-1.0f + 1.0f / width )*ratio, 1.0f - 1.0f / height, 1.0f, 0.0f));


    // Distance to next horizontal/vertical pixel
    const Vec4 deltaX = cameraToWorld(Vec4((2.0f / width)*ratio, 0, 0.0f, 0.0f));
    const Vec4 deltaY = cameraToWorld(Vec4(0, -2.0f / height, 0.0f, 0.0f));
    for (int y = 0; y < height; ++y) {
        Vec4 originalDirection = direction;
        for (int x = 0; x < width; ++x) {
            // Ray from camera's origin to pixel's center
            Ray cameraRay(this->origin, direction.normalize());
            RayHit hit;
            if (sceneRootNode->intersection(cameraRay, hit)) {
                result.setPixel(x, y, hit.color);
            }
            // Iterate thorugh next x pixel
            direction = direction + deltaX;
        }
        // Reset X value, iterate through next y pixel
        direction = originalDirection + deltaY;
    }

    // Result is saved in PPM image's data
    return result;
}