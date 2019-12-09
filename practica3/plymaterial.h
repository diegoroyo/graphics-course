#pragma once

#include <memory>
#include <vector>
#include "material.h"
#include "ppmimage.h"
typedef std::vector<std::vector<MaterialPtr>> Texture;
typedef std::vector<std::vector<MaterialBuilder>> TextureBuilder;
typedef std::shared_ptr<TextureBuilder> TextureBuilderPtr;
class PLYMaterial;
typedef std::shared_ptr<PLYMaterial> PLYMaterialPtr;

// Extends Material class for PLY models
// wxh matrix of materials which is UV mapped to model

// helper class for PLYMaterial
class PLYMaterialBuilder {
   private:
    const PLYMaterialPtr texturePtr;     // result texture
    const TextureBuilderPtr builderPtr;  // texture being built
    float accumProb;                     // accumulated probability
    const int height, width;             // dimensions of texture

    PLYMaterialBuilder(int _width, int _height,
                       const PLYMaterialPtr &_texturePtr,
                       const TextureBuilderPtr &_builderPtr)
        : texturePtr(_texturePtr),
          builderPtr(_builderPtr),
          accumProb(0.0f),
          height(_height),
          width(_width) {}
    friend class PLYMaterial;

    void addBRDF(const BRDFPtr &brdf);

   public:
    // Two variants of each function: texture UV mapping and constant
    // Same coefficientes for all the model
    PLYMaterialBuilder addPhongDiffuse(const RGBColor &kd);
    PLYMaterialBuilder addPhongSpecular(const float ks, const float alpha);
    PLYMaterialBuilder addPerfectSpecular(const float ksp);
    PLYMaterialBuilder addPerfectRefraction(const float krp,
                                            const float mediumRefractiveIndex);
    // UV mapped for the model
    PLYMaterialBuilder addPhongDiffuse(const char *diffuseFilename);

    PLYMaterialPtr build();
};

class PLYMaterial {
   public:
    int width, height;
    Texture data;

   private:
    PLYMaterial(int _width, int _height) : width(_width), height(_height) {
        data.resize(height);
        for (int y = 0; y < height; y++) {
            data[y].resize(width, Material::none());
        }
    }

   public:
    static PLYMaterialBuilder builder(int width = 1, int height = 1) {
        PLYMaterialPtr texturePtr =
            PLYMaterialPtr(new PLYMaterial(width, height));
        TextureBuilderPtr builderPtr =
            TextureBuilderPtr(new TextureBuilder(height));
        for (int y = 0; y < height; y++) {
            // temp. initialize all row to same builder
            (*builderPtr)[y].resize(width, Material::builder());
            for (int x = 0; x < width; x++) {
                // initialize each column to a different builder
                (*builderPtr)[y][x] = Material::builder();
            }
        }
        return PLYMaterialBuilder(width, height, texturePtr, builderPtr);
    }
};