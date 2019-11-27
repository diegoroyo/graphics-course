#include "camera.h"

// Prints fancy progress bar to stdout
void printProgress(const std::chrono::nanoseconds &beginTime, float progress) {
    // Compare endTime to beginTime
    std::chrono::nanoseconds endTime =
        std::chrono::system_clock::now().time_since_epoch();
    int secs = (endTime - beginTime).count() / 1000000000L;
    int eta = secs * (1.0f - progress) / progress;
    int tenPcts = progress / 0.1f;  // bar like [####      ] this
    std::printf(
        " Progress: %.3f %% [%s] (Time %d:%02d:%02d, ETA %d:%02d:%02d)      \r",
        progress * 100.0f,
        (std::string(tenPcts, '#') + std::string(10 - tenPcts, ' ')).c_str(),
        secs / 3600, (secs / 60) % 60, secs % 60,  // hh:mm:dd current time
        eta / 3600, (eta / 60) % 60, eta % 60);    // hh:mm:dd estimated time
    std::cout << std::flush;
}

// Generate random real number from 0..1
inline float random01() {
    static std::random_device rd;
    static std::mt19937 mtgen(rd());
    static std::uniform_real_distribution<float> random01(0.0f, 1.0f);
    return random01(mtgen);
}

/// Camera ///

inline Vec4 Camera::cameraToWorld(const Vec4 &v) {
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

        float event = random01();

        if (event < hit.material->color.max()) {
            // event < kd: difuso

            // Random inclination & azimuth
            float randIncl = random01();
            float randAzim = random01();
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
        float randX = random01();
        float randY = random01();
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
    const Vec4 firstDirection = cameraToWorld(Vec4(-1.0f, 1.0f, 1.0f, 0.0f));
    // Distance to next horizontal/vertical pixel
    const Vec4 deltaX = cameraToWorld(Vec4(2.0f / width, 0, 0.0f, 0.0f));
    const Vec4 deltaY = cameraToWorld(Vec4(0, -2.0f / height, 0.0f, 0.0f));

    // Spawn one core per thread and make them consume work as they finish
    int numPixels = width * height;
    volatile std::atomic<int> nextPixel(0);
    int cores = std::thread::hardware_concurrency();  // max no. of threads
    std::vector<std::future<void>> threadFutures;  // waits for them to finish
    for (int core = 0; core < cores; core++) {
        threadFutures.emplace_back(std::async([&]() {
            while (true) {
                // Get next job ID (or stop if there aren't any)
                int pixelIndex = nextPixel++;
                if (pixelIndex >= numPixels) {
                    break;
                }
                // Get direction to center of pixel
                int x = pixelIndex % width;
                int y = pixelIndex / width;
                Vec4 direction = firstDirection + deltaX * x + deltaY * y;
                // Generate rpp rays and store mean in result image
                result.setPixel(x, y,
                                tracePixel(direction, deltaX, deltaY, rpp,
                                           sceneRootNode, backgroundColor));
            }
        }));
    }

    bool finished = false;
    auto beginTime = std::chrono::system_clock::now().time_since_epoch();
    while (!finished) {
        printProgress(beginTime, nextPixel / (float)numPixels);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        for (auto &future : threadFutures) {
            if (future.wait_for(std::chrono::seconds(0)) ==
                std::future_status::ready) {
                finished = true;
                printProgress(beginTime, 1.0f);
                break;
            }
        }
    }

    // Result is saved in PPM image's data
    return result;
}