#include "uvmaterial.h"

UVMaterial::UVMaterial(int _width, int _height,
                       const MaterialPtr &fill = Material::none())
    : width(_width), height(_height) {
    data.resize(height);
    for (int y = 0; y < height; y++) {
        data[y].resize(width, fill);
    }
}

UVMaterialBuilder UVMaterial::builder(int width, int height) {
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

UVMaterialPtr UVMaterial::fill(int width, int height, const MaterialPtr &fill) {
    return UVMaterialPtr(new UVMaterial(1, 1, fill));
}

void UVMaterialBuilder::addEvent(const EventPtr &event) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            (*builderPtr)[y][x].add(event);
        }
    }
}

/// Constant (same BRDF for all model) builders ///

UVMaterialBuilder UVMaterialBuilder::addPhongDiffuse(const RGBColor &kd) {
    addEvent(EventPtr(new PhongDiffuse(kd)));
    return *this;
}

UVMaterialBuilder UVMaterialBuilder::addPhongSpecular(const float ks,
                                                      const float alpha) {
    addEvent(EventPtr(new PhongSpecular(ks, alpha)));
    return *this;
}

UVMaterialBuilder UVMaterialBuilder::addPerfectSpecular(const float ksp) {
    addEvent(EventPtr(new PerfectSpecular(ksp)));
    return *this;
}

UVMaterialBuilder UVMaterialBuilder::addPerfectRefraction(
    const float krp, const MediumPtr medium) {
    addEvent(EventPtr(new PerfectRefraction(krp, medium)));
    return *this;
}

/// Texture UV mapping builders ///

UVMaterialBuilder UVMaterialBuilder::addPhongDiffuse(
    const char *diffuseFilename) {
    PPMImage diffuse;
    diffuse.readFile(diffuseFilename);
    diffuse.flipVertically();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            RGBColor kd = diffuse.getPixel(x, y) * 0.99f;
            (*builderPtr)[y][x].add(EventPtr(new PhongDiffuse(kd)));
        }
    }
    return *this;
}

UVMaterialBuilder UVMaterialBuilder::addPhongSpecular(
    const char *specularFilename, const float alpha) {
    PPMImage specular;
    specular.readFile(specularFilename);
    specular.flipVertically();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float ks = specular.getPixel(x, y).max() * 0.99f;
            (*builderPtr)[y][x].add(EventPtr(new PhongSpecular(ks, alpha)));
        }
    }
    return *this;
}

// Note: This is hardcoded for the diamond texture (diamondore_emission.ppm)
UVMaterialBuilder UVMaterialBuilder::addPerfectSpecular(
    const char *specularFilename) {
    PPMImage specular;
    specular.readFile(specularFilename);
    specular.flipVertically();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            RGBColor color = specular.getPixel(x, y) * 0.99f;
            float meanProb = (color.r + color.g + color.b) / 3.0f;
            (*builderPtr)[y][x].add(EventPtr(new PerfectSpecular(meanProb)));
        }
    }
    return *this;
}

UVMaterialBuilder UVMaterialBuilder::addPortal(
    const char *portalFilename, const FigurePortalPtr &inPortal,
    const FigurePortalPtr &outPortal) {
    PPMImage portal;
    portal.readFile(portalFilename);
    portal.flipVertically();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float prob = portal.getPixel(x, y).max() * 0.99f;
            (*builderPtr)[y][x].add(
                EventPtr(new Portal(prob, inPortal, outPortal)));
        }
    }
    return *this;
}

UVMaterialPtr UVMaterialBuilder::build() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            texturePtr->data[y][x] = (*builderPtr)[y][x].build();
        }
    }
    return texturePtr;
}

void UVMaterial::override(const char *maskFilename,
                          const MaterialPtr &material) {
    PPMImage mask;
    mask.readFile(maskFilename);
    mask.flipVertically();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (mask.getPixel(x, y).max() > 1e-6f) {
                data[y][x] = material;
            }
        }
    }
}

void UVMaterial::overrideLights(const char *emissionFilename,
                                const float emissionFactor,
                                const float minColor) {
    PPMImage emissionMap;
    emissionMap.readFile(emissionFilename);
    emissionMap.flipVertically();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            RGBColor color = emissionMap.getPixel(x, y);
            if (color.max() > minColor) {
                data[y][x] = Material::light(color * emissionFactor);
            }
        }
    }
}