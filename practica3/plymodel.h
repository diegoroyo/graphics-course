#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include "../lib/geometry.h"
#include "../lib/ppmimage.h"
#include "../lib/rgbcolor.h"
#include "figures.h"

class PLYModel {
   private:
    std::vector<Vec4> verts;
    std::vector<std::array<int, 3>> faces;
    std::vector<std::array<float, 2>> uvs;

    PPMImage emissionTexture;

   public:
    PLYModel(const char* filename);

    inline int nverts() const { return verts.size(); }
    inline int nfaces() const { return faces.size(); }
    inline Vec4 vert(int i) const { return verts[i]; }
    inline std::array<int, 3> face(int i) const { return faces[i]; }
    // Get UV coordinates for a given pixel
    inline std::array<float, 2> uv(int i) const { return uvs[i]; }
    // Diffuse texture RGB color given UV coordinates
    inline RGBColor emission(float uvx, float uvy) const {
        // Round UV to nearest pixel (+0.5f)
        int pixelX = emissionTexture.width * uvx + 0.5f;
        int pixelY = emissionTexture.height * uvy + 0.5f;
        return emissionTexture.getPixel(pixelX, pixelY);
    }

    // Add triangle figures to scene vector
    void addTriangles(FigurePtrVector &scene) const;
};