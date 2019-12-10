#pragma once

#include <memory>
#include <vector>
#include "material.h"
#include "ppmimage.h"
typedef std::vector<std::vector<MaterialPtr>> Texture;
typedef std::vector<std::vector<MaterialBuilder>> TextureBuilder;
typedef std::shared_ptr<TextureBuilder> TextureBuilderPtr;
class UVMaterial;
typedef std::shared_ptr<UVMaterial> UVMaterialPtr;

// Extends Material class for PLY models
// wxh matrix of materials which is UV mapped to model

// helper class for UVMaterial
class UVMaterialBuilder {
   private:
    const UVMaterialPtr texturePtr;      // result texture
    const TextureBuilderPtr builderPtr;  // texture being built
    float accumProb;                     // accumulated probability
    const int height, width;             // dimensions of texture

    UVMaterialBuilder(int _width, int _height, const UVMaterialPtr &_texturePtr,
                      const TextureBuilderPtr &_builderPtr)
        : texturePtr(_texturePtr),
          builderPtr(_builderPtr),
          accumProb(0.0f),
          height(_height),
          width(_width) {}
    friend class UVMaterial;

    void addBRDF(const BRDFPtr &brdf);

   public:
    // Two variants of each function: texture UV mapping and constant
    // Same coefficientes for all the model
    UVMaterialBuilder addPhongDiffuse(const RGBColor &kd);
    UVMaterialBuilder addPhongSpecular(const float ks, const float alpha);
    UVMaterialBuilder addPerfectSpecular(const float ksp);
    UVMaterialBuilder addPerfectRefraction(const float krp,
                                           const float mediumRefractiveIndex);
    // UV mapped for the model
    UVMaterialBuilder addPhongDiffuse(const char *diffuseFilename);
    UVMaterialBuilder addPerfectSpecular(const char *specularFilename);

    UVMaterialPtr build();
    UVMaterialPtr buildOverrideLights(const char *emissionFilename,
                                      const float emissionFactor);
};

class UVMaterial {
   public:
    int width, height;
    Texture data;

   private:
    UVMaterial(int _width, int _height) : width(_width), height(_height) {
        data.resize(height);
        for (int y = 0; y < height; y++) {
            data[y].resize(width, Material::none());
        }
    }

   public:
    static UVMaterialBuilder builder(int width = 1, int height = 1) {
        UVMaterialPtr texturePtr = UVMaterialPtr(new UVMaterial(width, height));
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
        return UVMaterialBuilder(width, height, texturePtr, builderPtr);
    }

    inline MaterialPtr get(const float uvx, const float uvy) const {
        int x = std::min(width - 1, (int)(uvx * this->width));
        int y = std::min(height - 1, (int)(uvy * this->height));
        return this->data[y][x];
    }
};