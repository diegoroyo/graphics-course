#pragma once

#include <vector>
#include <functional>
#include "rgbcolor.h"
#include "tonemapper.h"

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
   private:
    // Resize data vectors
    void clearData(int width, int height);
    
   public:
    int width, height;
    int colorResolution;
    float max;
    // data[x][y] = pixel(column x, row y)
    // opposite of common matrix structure for efficient access
    std::vector<std::vector<RGBColor>> data;

    // Empty constructor
    PPMImage() : width(0), height(0), colorResolution(0), max(1.0f) {}
    // Copy w/o data (data will be added later e.g. generated LDR images)
    PPMImage(int _w, int _h, int _c = 255, float _max = 1.0f)
        : width(_w), height(_h), colorResolution(_c), max(_max) {
        clearData(width, height);
    }

    void initialize(const int _w, const int _h, const int _c, const float _max);
    bool readFile(const char* filename);
    bool writeFile(const char* filename) const;

    void applyToneMap(PPMImage& result, ToneMapper &tm);
};