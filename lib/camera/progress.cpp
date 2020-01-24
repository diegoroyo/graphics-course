#include "progress.h"

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