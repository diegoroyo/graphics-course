#include "camera.h"

// Prints fancy progress bar to stdout
void printProgress(const std::chrono::nanoseconds &beginTime, float progress) {
    // Compare endTime to beginTime
    std::chrono::nanoseconds endTime =
        std::chrono::system_clock::now().time_since_epoch();
    int secs = (endTime - beginTime).count() / 1000000000L;
    int eta = secs * (1.0f - progress) / progress;
    int fivePcts = progress / 0.05f;  // bar like [####      ] this
    std::printf(
        " Progress: %.3f %% [%s] (Time %d:%02d:%02d, ETA %d:%02d:%02d)      \r",
        progress * 100.0f,
        (std::string(fivePcts, '#') + std::string(20 - fivePcts, ' ')).c_str(),
        secs / 3600, (secs / 60) % 60, secs % 60,  // hh:mm:dd current time
        eta / 3600, (eta / 60) % 60, eta % 60);    // hh:mm:dd estimated time
    std::cout << std::flush;
}

/// Camera ///

inline Vec4 Camera::cameraToWorld(const Vec4 &v) {
    // static to it doesn't construct the matrix more than once
    static Mat4 cob(right, up, forward, origin);
    return cob * v;
}

RGBColor Camera::tracePath(const Ray &cameraRay, const Scene &scene,
                           const RGBColor &backgroundColor) {
    // Ray from camera's origin to pixel's center
    RayHit hit;
    if (scene.root->intersection(cameraRay, hit)) {
        // Special case: hit a light
        if (hit.material->emitsLight) {
// Return the light emission
#ifdef DEBUG_PATH
            std::cout << "Light hit on point " << hit.point << " with normal "
                      << hit.normal << std::endl;
#endif
            return hit.material->emission;
        }

        // Calculate russian roulette event
        BRDFPtr event = hit.material->selectEvent();
        RGBColor eventLight = RGBColor::Black;
        if (event != nullptr) {
#ifdef DEBUG_PATH
            std::cout << "Event on point " << hit.point << " with normal "
                      << hit.normal << std::endl;
#endif
            Vec4 nextDirection = event->nextRay(cameraRay.direction, hit);
            return event->applyBRDF(tracePath(Ray(hit.point, nextDirection),
                                              scene, backgroundColor));
#ifdef DEBUG_PATH
        } else {
            std::cout << "Path died :(" << std::endl;
#endif
        }

        // Calculate direct light from point light sources
        return eventLight + scene.directLight(hit.point);
    }
#ifdef DEBUG_PATH
    std::cout << "Ray didn't collide with anything" << std::endl;
#endif
    return backgroundColor;
}

RGBColor Camera::tracePixel(const Vec4 &d0, const Vec4 &deltaX,
                            const Vec4 &deltaY, int rpp, const Scene &scene,
                            const RGBColor &backgroundColor) {
    RGBColor pixelColor(backgroundColor);
    for (int r = 0; r < rpp; ++r) {
        float randX = random01();
        float randY = random01();
        Vec4 direction = d0 + deltaX * randX + deltaY * randY;
        // Trace ray and store mean in result
        Ray ray(this->origin, direction.normalize());
#ifdef DEBUG_PATH
        std::cout << std::endl << "> Ray begins" << std::endl;
#endif
        RGBColor rayColor = tracePath(ray, scene, backgroundColor);
        pixelColor = pixelColor + rayColor * (1.0f / rpp);
    }
    return pixelColor;
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays
PPMImage Camera::render(int width, int height, int rpp, const Scene &scene,
                        const RGBColor &backgroundColor) {
    // Initialize image with width/height and bg color
    PPMImage result(width, height, std::numeric_limits<int>::max());
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
                                           scene, backgroundColor));
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

    // Set result's max value to the render's max value
    float max = 0.0f;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            max = std::max(max, result.getPixel(x, y).max());
        }
    }
    result.setMax(max);

    // Result is saved in PPM image's data
    return result;
}