#include "uvmaterial.h"

void UVMaterialBuilder::addBRDF(const BRDFPtr &brdf) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            (*builderPtr)[y][x].add(brdf);
        }
    }
}

/// Constant (same BRDF for all model) builders ///

UVMaterialBuilder UVMaterialBuilder::addPhongDiffuse(const RGBColor &kd) {
    addBRDF(BRDFPtr(new PhongDiffuse(kd)));
    return *this;
}

UVMaterialBuilder UVMaterialBuilder::addPhongSpecular(const float ks,
                                                      const float alpha) {
    addBRDF(BRDFPtr(new PhongSpecular(ks, alpha)));
    return *this;
}

UVMaterialBuilder UVMaterialBuilder::addPerfectSpecular(const float ksp) {
    addBRDF(BRDFPtr(new PerfectSpecular(ksp)));
    return *this;
}

UVMaterialBuilder UVMaterialBuilder::addPerfectRefraction(
    const float krp, const float mediumRefractiveIndex) {
    addBRDF(BRDFPtr(new PerfectRefraction(krp, mediumRefractiveIndex)));
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
            (*builderPtr)[y][x].add(BRDFPtr(new PhongDiffuse(kd)));
        }
    }
    return *this;
}

UVMaterialBuilder UVMaterialBuilder::addPerfectSpecular(
    const char *specularFilename) {
    // TODO this is hardcoded for the diamond texture
    // find a way to merge phongspec, perfspec & perfrefraction onto a file
    // (RGB wont be enough as you need more channel for spec alpha & refraction
    // index)
    PPMImage specular;
    specular.readFile(specularFilename);
    specular.flipVertically();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            RGBColor color = specular.getPixel(x, y) * 0.99f;
            float meanProb = (color.r + color.g + color.b) / 3.0f;
            (*builderPtr)[y][x].add(BRDFPtr(new PerfectSpecular(meanProb)));
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

UVMaterialPtr UVMaterialBuilder::buildOverrideLights(
    const char *emissionFilename, const float emissionFactor) {
    PPMImage emissionMap;
    emissionMap.readFile(emissionFilename);
    emissionMap.flipVertically();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            RGBColor color = emissionMap.getPixel(x, y);
            if (color.max() > 1e-6f) {
                texturePtr->data[y][x] =
                    Material::light(color * emissionFactor);
            } else {
                texturePtr->data[y][x] = (*builderPtr)[y][x].build();
            }
        }
    }
    return texturePtr;
}