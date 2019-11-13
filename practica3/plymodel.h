#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <vector>
#include "../lib/geometry.h"

class PLYModel {
   private:
    std::vector<Vec4> verts;
    std::vector<std::array<int, 3>> faces;

   public:
    PLYModel(const char* filename);
    constexpr int nverts() const { return verts.size(); };
    constexpr int nfaces() const { return faces.size(); }
    inline Vec4 vert(int i) const { return verts[i]; }
    inline std::array<int, 3> face(int i) const { return faces[i]; }
};