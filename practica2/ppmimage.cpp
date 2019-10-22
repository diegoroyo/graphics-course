#include <fstream>
#include "ppmimage.h"


bool PPMImage::readFile(const char* filename) {
    std::ifstream is(filename);
    if (!is.is_open()) {
        std::cerr << "Can't open file " << filename << std::endl;
        return false;
    }

    // Check if PPM file format is P3 (first line should be 'P3')
    char p, three;
    is >> p >> three;
    if (p != 'P' || three != '3') {
        std::cerr << "Invalid PPM file format (first line should be 'P3')"
                  << std::endl;
        return false;
    }

    // Read file data
    int readLines = 0;
    bool end = false;
    while (!end && !is.eof()) {
        if (is.peek() == '#') {
            // Comment line (can be #MAX=<max>)
            std::string comment;
            is >> comment;
            if (comment.substr(0, 5).compare("#MAX=") == 0) {
                // string-to-float
                max = std::stof(comment.substr(6));
            }
        } else {
            // Read lines in order
            switch (readLines) {
                case 0:
                    is >> width >> height;
                    // Reserve space for pixels
                    data.resize(width);
                    for (int i = 0; i < width; i++) {
                        data[i].resize(height);
                    }
                    break;
                case 1:
                    is >> colorResolution;
                    break;
                default:
                    int currentHeight = readLines - 2;
                    for (int i = 0; i < width; i++) {
                        int r, g, b;
                        is >> r >> g >> b;
                        // More float precision is achieved by first
                        // multiplying rgb with max and then dividing
                        data[i][currentHeight].setValues(
                            r * max / colorResolution,
                            g * max / colorResolution,
                            b * max / colorResolution);
                    }
            }
            // Increment for next iteration
            readLines++;
        }
    }

    if (!end) {
        std::cerr << "PPM file ended and the reading wasn't finished"
                  << std::endl;
        return false;
    }

    return true;
}

bool PPMImage::writeFile(const char* filename) {
    // TODO
    return false;
}

void PPMImage::applyToneMap(PPMImage& result) {
    PPMImage ldr(width, height, colorResolution, max);
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            // TODO hacer cosas
        }
    }
    result = ldr;
}