#include "camera.h"

float Camera::random01() const {
    static std::random_device rd;
    static std::mt19937 mtgen(rd());
    static std::uniform_real_distribution<float> random01(0.0f, 1.0f);
    return random01(mtgen);
}

Vec4 Camera::cameraToWorld(const Vec4 &v) {
    // static to it doesn't construct the matrix more than once
    static Mat4 cob(right, up, forward, origin);
    return cob * v;
}

RGBColor Camera::tracePath(const Ray &cameraRay, const FigurePtr &sceneRootNode,
                           const RGBColor &backgroundColor) {
    // Ray from camera's origin to pixel's center
    RayHit hit;
    if (sceneRootNode->intersection(cameraRay, hit)) {
        if (hit.material->emitsLight) {
            return hit.material->color;
        }

        float event = this->random01();  // random 0..1

        if (event < hit.material->color.max()) {
            // event < kd: difuso

            // Random inclination & azimuth
            float randIncl = this->random01();
            float randAzim = this->random01();
            float incl = std::acos(std::sqrt(randIncl));
            float azim = 2 * M_PI * randAzim;
            /*
                PARA LA PROX. VEZ
                incl = arccos(rand_incl^(1/alpha+1))
                azim = 2*pi*rand_azim
            */

            // Local base to hit point
            // TODO poner valor a la normal en figures.cpp (en el resto de
            // cosas)
            Vec4 z = hit.normal;
            Vec4 x = cross(z, cameraRay.direction);
            Vec4 y = cross(z, x);
            Mat4 cob = Mat4::changeOfBasis(x, y, z, hit.point);
            Vec4 rayDirection = cob * Vec4(std::sin(incl) * std::cos(azim),
                                           std::sin(incl) * std::sin(azim),
                                           std::cos(incl), 0.0f);

            return hit.material->color *
                   tracePath(Ray(hit.point, rayDirection.normalize()),
                             sceneRootNode, backgroundColor);
        } else {
            return RGBColor::Black;
        }
    }
    return backgroundColor;
}

RGBColor Camera::tracePixel(const Vec4 &d0, const Vec4 &deltaX,
                            const Vec4 &deltaY, int rpp,
                            const FigurePtr &sceneRootNode,
                            const RGBColor &backgroundColor) {
    RGBColor pixelColor(backgroundColor);
    for (int r = 0; r < rpp; ++r) {
        float randX = this->random01();
        float randY = this->random01();
        Vec4 direction = d0 + deltaX * randX + deltaY * randY;
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
    PPMImage result(width, height, 255, 10000.0f);
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
            RGBColor pixelColor =
                tracePixel(direction, deltaX, deltaY, rpp, sceneRootNode, backgroundColor);
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