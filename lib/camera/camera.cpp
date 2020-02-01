#include "camera.h"

// Debug settings
#define DEBUG_ONE_CORE  // don't use multithreading

/// Camera ///

// https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays
void Camera::tracePixels(const Scene &scene) const {
    // Spawn one core per thread and make them consume work as they finish
    int numPixels = film.width * film.height;
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
                int x = pixelIndex % film.width;
                int y = pixelIndex / film.width;
                Vec4 direction = film.getPixelCenter(x, y);
                // Generate ppp rays and store mean in result image
                rayTracer->tracePixel(x, y, film, scene);
            }
        }));
    }

    bool finished = false;
    auto beginTime = std::chrono::system_clock::now().time_since_epoch();
    printProgress(beginTime, 0.0f);
    while (!finished) {
        float progress = nextPixel / (float)numPixels;
        printProgress(beginTime, std::fminf(1.0f, progress));
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
}

void Camera::storeResult(const std::string &filename) const {
    rayTracer->result().writeFile(filename.c_str());
}