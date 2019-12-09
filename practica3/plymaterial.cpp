#include "plymaterial.h"

void PLYMaterialBuilder::addBRDF(const BRDFPtr &brdf) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            (*builderPtr)[y][x].add(brdf);
        }
    }
}

/// Constant (same BRDF for all model) builders ///

PLYMaterialBuilder PLYMaterialBuilder::addPhongDiffuse(const RGBColor &kd) {
    addBRDF(BRDFPtr(new PhongDiffuse(kd)));
    return *this;
}

PLYMaterialBuilder PLYMaterialBuilder::addPhongSpecular(const float ks,
                                                        const float alpha) {
    addBRDF(BRDFPtr(new PhongSpecular(ks, alpha)));
    return *this;
}

PLYMaterialBuilder PLYMaterialBuilder::addPerfectSpecular(const float ksp) {
    addBRDF(BRDFPtr(new PerfectSpecular(ksp)));
    return *this;
}

PLYMaterialBuilder PLYMaterialBuilder::addPerfectRefraction(
    const float krp, const float mediumRefractiveIndex) {
    addBRDF(BRDFPtr(new PerfectRefraction(krp, mediumRefractiveIndex)));
    return *this;
}

/// Texture UV mapping builders ///

PLYMaterialBuilder PLYMaterialBuilder::addPhongDiffuse(
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

PLYMaterialPtr PLYMaterialBuilder::build() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            texturePtr->data[y][x] = (*builderPtr)[y][x].build();
        }
    }
    return texturePtr;
}