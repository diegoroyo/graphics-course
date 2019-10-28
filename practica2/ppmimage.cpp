#include "ppmimage.h"
#include <fstream>
#include "pngimage/pngimage.h"

void PPMImage::clearData(int width, int height) {
    // Reserve space for pixels
    data.resize(height);
    for (int i = 0; i < height; i++) {
        data[i].resize(width);
    }
}

void PPMImage::initialize(const int _w, const int _h, const int _c,
                          const float _max) {
    width = _w;
    height = _h;
    colorResolution = _c;
    max = _max;
    clearData(width, height);
}

bool PPMImage::readFile(const char* filename) {
    std::ifstream is(filename);
    if (!is.is_open()) {
        std::cerr << "Can't open file " << filename << std::endl;
        return false;
    }

    // Check if PPM file format is P3 (first line should be 'P3')
    char p, three;
    is >> p >> three;
    is.ignore(1);  // ignore newline
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
            std::getline(is, comment);
            if (comment.substr(0, 5).compare("#MAX=") == 0) {
                // string-to-float
                max = std::stof(comment.substr(5));
            }
        } else {
            // Read lines in order
            switch (readLines) {
                case 0:
                    is >> width >> height;
                    clearData(width, height);
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
                        data[currentHeight][i].setValues(
                            r * max / colorResolution,
                            g * max / colorResolution,
                            b * max / colorResolution);
                    }
            }
            // Increment for next iteration
            readLines++;
            // Check if all data has been read
            if (readLines - 2 == height) {
                end = true;
            }
        }
    }

    if (!end) {
        std::cerr << "PPM file ended and the reading wasn't finished"
                  << std::endl;
        return false;
    }

    return true;
}

bool PPMImage::writeFile(const char* filename, bool ldr) const {
    std::ofstream os(filename);
    if (!os.is_open()) {
        std::cerr << "Can't open file " << filename << std::endl;
        return false;
    }

    // P3 file format
    os << "P3" << std::endl;
    // Optional "MAX" comment
    if (max != 1.0f && !ldr) {
        os << "#MAX=" << max << std::endl;
    }
    // File name
    os << "# " << filename << std::endl;
    // Width, height and color resolution
    os << width << ' ' << height << std::endl;
    if (ldr) {
        os << LDR_RESOLUTION << std::endl;
    } else {
        os << colorResolution << std::endl;
    }
    // Pixel color data
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (ldr) {
                // Assumes color data is on 0..1 range
                os << int(data[y][x].r * LDR_RESOLUTION) << " "
                   << int(data[y][x].g * LDR_RESOLUTION) << " "
                   << int(data[y][x].b * LDR_RESOLUTION) << "     ";
            } else {
                // Write data in same HDR range
                os << int(data[y][x].r * colorResolution / max + 0.5f) << " "
                   << int(data[y][x].g * colorResolution / max + 0.5f) << " "
                   << int(data[y][x].b * colorResolution / max + 0.5f)
                   << "     ";
            }
        }
        os << std::endl;
    }

    return true;
}

void PPMImage::applyToneMap(PPMImage& result, ToneMapper& tm) {
    result.initialize(width, height, colorResolution, max);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Convert RGB data to Lab data
            RGBColor labColor = data[y][x].rgb2lab(max);
            // Map [0, max] luminance to [0, 1] using tm.map function
            labColor.lab.l = tm.map(labColor.lab.l);
            // Convert back to RGB
            result.data[y][x] = labColor.lab2rgb(max);
        }
    }
}

PNGImage PPMImage::convertToPNG() {
    PNGImage png(width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            png.set_pixel(x, y, data[y][x] * 255.0f);
        }
    }
    return png;
}