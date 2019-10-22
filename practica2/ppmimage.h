#pragma once

#include <vector>
#include "rgbcolor.h"

/*
 * PPM Image loader/saver (P3 format)
 *
 * .ppm file format:
 *
 * P3
 * <width> <height>
 * <colorResolution>
 * <r> <g> <b> <r> <g> <b>... (width)
 * <r> <g> <b>...
 * ...
 * (height)
 *
 * Comments are supported (lines starting with #)
 * Optional #MAX=<max> comment can be added
 * if you need a maximum that is different from 1
 */
class PPMImage {
   public:
    int width, height;
    int colorResolution;
    float max;
    // data[x][y] = pixel(column y, row x)
    std::vector<std::vector<RGBColor>> data;

    // Empty constructor
    PPMImage() : width(0), height(0), colorResolution(0), max(1.0f) {}
    // Copy w/o data (data will be added later e.g. generated LDR images)
    PPMImage(int _w, int _h, int _c = 255, float _max = 1.0f)
        : width(_w), height(_h), colorResolution(_c), max(_max) {
        data.resize(width);
        for (int i = 0; i < width; i++) {
            data[i].resize(height);
        }
    }

    bool readFile(const char* filename);
    bool writeFile(const char* filename);

    // TODO especificar el tone mapping operator (pasar función por parámetro?)
    // https://stackoverflow.com/questions/9410/how-do-you-pass-a-function-as-a-parameter-in-c
    // Igual es fliparse mucho
    bool applyToneMap(PPMImage& result);
    static inline float OP_CLAMPING(float in, float max) {
        return in > max ? max : in;
    }
};