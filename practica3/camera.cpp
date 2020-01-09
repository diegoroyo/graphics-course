#include "camera.h"

// Debug settings
// #define DEBUG_PATH      // show path's hits and moment of stopping
// #define DEBUG_ONE_CORE  // don't use multithreading (for print debugging)

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

inline Vec4 Camera::cameraToWorld(const Vec4 &v) const {
    // static to it doesn't construct the matrix more than once
    static Mat4 cob(right, up, forward, origin);
    return cob * v;
}

inline Vec4 Camera::getDoFDisplacement() const {
    if (dofRadius == 0.0f) {
        return Vec4();
    } else {
        float r = dofRadius * sqrtf(random01());
        Mat4 rotZ = Mat4::rotationZ(random01() * 2.0f * M_PI);
        return cameraToWorld(rotZ * Vec4(0.0f, r, 0.0f, 0.0f));
    }
}

RGBColor Camera::tracePath(const Ray &cameraRay, const Scene &scene,
                           const RGBColor &backgroundColor) const {
    // Ray from camera's origin to pixel's center
    RayHit hit;
    if (scene.intersection(cameraRay, hit)) {
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
        EventPtr event = hit.material->selectEvent();
        // Only calculate direct light if event is not perfect refraction
        Ray nextRay;
        if (event != nullptr && event->nextRay(cameraRay, hit, nextRay)) {
#ifdef DEBUG_PATH
            std::cout << "Event on point " << hit.point << " with normal "
                      << hit.normal << std::endl;
#endif
            RGBColor directLight =
                scene.directLight(hit, nextRay.direction, event);
            RGBColor nextEventLight = event->applyMonteCarlo(
                tracePath(nextRay, scene, backgroundColor), hit,
                cameraRay.direction, nextRay.direction);
            return nextEventLight + directLight;
#ifdef DEBUG_PATH
        } else {
            std::cout << "Path died :(" << std::endl;
#endif
        }

        // Path died for one reason or another
        return RGBColor::Black;
    }
#ifdef DEBUG_PATH
    std::cout << "Ray didn't collide with anything" << std::endl;
#endif
    return backgroundColor;
}

RGBColor Camera::tracePixel(const Vec4 &d0, const Vec4 &deltaX,
                            const Vec4 &deltaY, int ppp, const Scene &scene,
                            const RGBColor &backgroundColor) const {
    RGBColor pixelColor(backgroundColor);
    for (int p = 0; p < ppp; ++p) {
        float randX = random01();
        float randY = random01();
        Vec4 dof = this->getDoFDisplacement();
        Vec4 direction = d0 + deltaX * randX + deltaY * randY;
        // Trace ray and store mean in result
        Ray ray(this->origin + dof, (direction - dof).normalize(), scene.air);
#ifdef DEBUG_PATH
        std::cout << std::endl << "> Ray begins" << std::endl;
#endif
        RGBColor rayColor = tracePath(ray, scene, backgroundColor);
        if (rayColor.max() > scene.maxLightEmission) {
            // std::cout << "El rayo devuelve " << rayColor << std::endl;
            rayColor = rayColor * (scene.maxLightEmission / rayColor.max());
            // std::cout << "Despues del check es " << rayColor << std::endl;
        }
        pixelColor = pixelColor + rayColor * (1.0f / ppp);
    }
    return pixelColor;
}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays
PPMImage Camera::render(int width, int height, int ppp, const Scene &scene,
                        const RGBColor &backgroundColor) const {
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
#ifdef DEBUG_ONE_CORE
    int cores = 1;  // only one core, for debug purposes
#else
    int cores = std::thread::hardware_concurrency();  // max no. of threads
#endif
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
                // Generate ppp rays and store mean in result image
                // std::cout << ">>> Pixel " << x << ", " << y << std::endl;
                result.setPixel(x, y,
                                tracePixel(direction, deltaX, deltaY, ppp,
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
    result.setMax(result.calculateMax());

    // Result is saved in PPM image's data
    return result;
}