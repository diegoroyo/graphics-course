#include "camera.h"

Vec4 Camera::cameraToWorld(const Vec4 &v) {
    // static to it doesn't construct the matrix more than once
    static Mat4 cob(right, up, forward, origin);
    return cob * v;
}

RGBColor Camera::tracePath(const Ray &cameraRay, const FigurePtr &sceneRootNode,
                           const RGBColor &backgroundColor) {
    RGBColor result(backgroundColor);
    // Ray from camera's origin to pixel's center
    RayHit hit;
    if (sceneRootNode->intersection(cameraRay, hit)) {
        result = hit.color;
    }
    return result;
}

RGBColor Camera::tracePixel(const Vec4 &p0, const Vec4 &p1, int rpp,
                            const FigurePtr &sceneRootNode,
                            const RGBColor &backgroundColor) {
    RGBColor pixelColor(backgroundColor);
    for (int r = 0; r < rpp; ++r) {
        // TODO generate random direction between [p0, p1]
        // right now it only shoots at the center of the pixel
        Vec4 direction = (p0 + p1) * 0.5f;
        // Trace ray and store mean in result
        Ray ray(this->origin, direction.normalize());
        RGBColor rayColor = tracePath(ray, sceneRootNode, backgroundColor);
        pixelColor = pixelColor + rayColor * (1.0f / rpp);
    }
    return pixelColor;
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays
PPMImage Camera::render(int width, int height, int rpp,
                        const FigurePtr &sceneRootNode,
                        const RGBColor &backgroundColor) {
    // Initialize image with width/height and bg color
    PPMImage result(width, height);
    result.fillPixels(backgroundColor);

    // Iterate through [-1, 1] * [-1, 1] (camera's local space)
    // First point is at (-1, 1, 1) in local space (right, up, forward)
    Vec4 direction = cameraToWorld(Vec4(-1.0f, 1.0f, 1.0f, 0.0f));
    // Distance to next horizontal/vertical pixel
    const Vec4 deltaX = cameraToWorld(Vec4(2.0f / width, 0, 0.0f, 0.0f));
    const Vec4 deltaY = cameraToWorld(Vec4(0, -2.0f / height, 0.0f, 0.0f));
    for (int y = 0; y < height; ++y) {
        Vec4 originalDirection = direction;
        for (int x = 0; x < width; ++x) {
            // Trace rays to pixel [x, y] and store color result
            Vec4 p0 = direction;
            Vec4 p1 = direction + deltaX + deltaY;
            RGBColor pixelColor =
                tracePixel(p0, p1, rpp, sceneRootNode, backgroundColor);
            result.setPixel(x, y, pixelColor);
            // Iterate thorugh next x pixel
            direction = direction + deltaX;
        }
        // Reset X value, iterate through next y pixel
        direction = originalDirection + deltaY;
    }

    // Result is saved in PPM image's data
    return result;
}