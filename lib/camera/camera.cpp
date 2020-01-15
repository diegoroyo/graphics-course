#include "camera.h"

// Debug settings
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
}

void Camera::storeResult(const std::string &filename) const {
    rayTracer->result().writeFile(filename.c_str());
}