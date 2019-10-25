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
    static const constexpr float LDR_RESOLUTION = 255.0f;

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

    // Initialize image metadata with empty data vector
    void initialize(const int _w, const int _h, const int _c, const float _max);
    // Read PPM file, store in same object
    bool readFile(const char* filename);
    // Write PPM contents to file (ldr = normalize color resolution to 255)
    // ldr assumes that image data is on 0..1 range
    bool writeFile(const char* filename, bool ldr = false) const;

    //Transforms pixel [x,y] from RGB to CIE-L*ab formmat
    void RGB2Lab(int x, int y);
    //Transforms pixel [x,y] from CIE-L*ab to RGB formmat
    void Lab2RGB(int x, int y);

    void applyToneMap(PPMImage& result, ToneMapper &tm);
};