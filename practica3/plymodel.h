#pragma once

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include "figures.h"
#include "geometry.h"
#include "rgbcolor.h"
#include "uvmaterial.h"

class PLYModel {
   private:
    std::vector<Vec4> verts;
    std::vector<std::array<int, 3>> faces;
    std::vector<std::array<float, 2>> uvs;

    const UVMaterialPtr uvMaterial;

    // Find bounding box of all faces whose indexes are in findex
    // Box defined as 2 points: min (bb0) and max (bb1)
    void getBoundingBox(const std::vector<int> &findex, Vec4 &bb0,
                        Vec4 &bb1) const;
    // Returns a FigurePtr containing either:
    // - numIterations == 0: BVNode whose children are triangles
    // - numIterations > 0: Divide model in half on its biggest axis,
    //                      return KdTreeNode with those two children
    FigurePtr divideNode(const FigurePtrVector &triangles,
                         const std::vector<int> &findex,
                         std::vector<Vec4 *>::iterator &vbegin,
                         std::vector<Vec4 *>::iterator &vend,
                         int numIterations);

   public:
    PLYModel(const char *filename, const UVMaterialPtr &uvMaterial);

    inline int nverts() const { return verts.size(); }
    inline int nfaces() const { return faces.size(); }
    inline Vec4 vert(int i) const { return verts[i]; }
    inline std::array<int, 3> face(int i) const { return faces[i]; }
    // Get UV coordinates for a given pixel
    inline std::array<float, 2> uv(int i) const { return uvs[i]; }
    // Diffuse texture RGB color given UV coordinates
    MaterialPtr material(float uvx, float uvy) const;

    // Apply model matrix to all vertices
    void transform(const Mat4 &modelMatrix);

    // Get FigurePtr representing the model
    FigurePtr getFigure(int subdivisions = 1);
};