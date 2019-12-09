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

UVMaterialPtr UVMaterialBuilder::build() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            texturePtr->data[y][x] = (*builderPtr)[y][x].build();
        }
    }
    return texturePtr;
}