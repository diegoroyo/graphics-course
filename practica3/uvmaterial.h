#pragma once

// forward declarations
#include <memory>
#include <vector>
class Material;
typedef std::shared_ptr<Material> MaterialPtr;
class MaterialBuilder;
typedef std::vector<std::vector<MaterialPtr>> Texture;
typedef std::vector<std::vector<MaterialBuilder>> TextureBuilder;
typedef std::shared_ptr<TextureBuilder> TextureBuilderPtr;
class UVMaterial;
typedef std::shared_ptr<UVMaterial> UVMaterialPtr;

#include "material.h"
#include "medium.h"
#include "ppmimage.h"

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

    void addEvent(const EventPtr &event);

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
    UVMaterialBuilder addPortal(const char *portalFilename,
                                const FigurePortalPtr &inPortal,
                                const FigurePortalPtr &outPortal);

    UVMaterialPtr build();
};

class UVMaterial {
   public:
    int width, height;
    Texture data;

   private:
    UVMaterial(int _width, int _height);

   public:
    static UVMaterialBuilder builder(int width, int height);

    inline MaterialPtr get(const float uvx, const float uvy) const {
        int x = std::min(width - 1, (int)(uvx * this->width));
        int y = std::min(height - 1, (int)(uvy * this->height));
        return this->data[y][x];
    }

    void override(const char *maskFilename, const MaterialPtr &material);
    void overrideLights(const char *emissionFilename,
                        const float emissionFactor,
                        const float minColor = 0.0f);
};