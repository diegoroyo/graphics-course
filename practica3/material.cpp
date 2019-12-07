#include "material.h"

/// Phong Diffuse ///

Vec4 PhongDiffuse::nextRay(const Vec4 &inDirection, RayHit &hit) {
    // Random inclination & azimuth
    float randIncl = random01();
    float randAzim = random01();
    // Inclination & azimuth for uniform cosine sampling
    float incl = acosf(sqrtf(randIncl));
    float azim = 2 * M_PI * randAzim;

    // Local base to hit point
    Vec4 z = hit.normal;
    Vec4 x = cross(z, inDirection);
    Vec4 y = cross(z, x);
    Mat4 cob = Mat4::changeOfBasis(x, y, z, hit.point);
    return cob * Vec4(sinf(incl) * cosf(azim), sinf(incl) * sinf(azim),
                      cosf(incl), 0.0f);
}

RGBColor PhongDiffuse::applyBRDF(const RGBColor &lightIn) const {
    return lightIn * this->kdProb;
}

/// Phong Specular ///

Vec4 PhongSpecular::nextRay(const Vec4 &inDirection, RayHit &hit) {
    // Random inclination & azimuth
    float randIncl = random01();
    float randAzim = random01();
    // Phong Specular lobe sampling
    float incl = acosf(powf(randIncl, 1.0f / (this->alpha + 1)));
    float azim = 2 * M_PI * randAzim;

    // Save for next iteration
    this->tempAzimIncCos = dot(inDirection, hit.normal);
    this->tempIncl = incl;

    // Local base to hit point
    Vec4 z = (hit.normal * 2.0f + inDirection).normalize();
    Vec4 x = cross(z, inDirection);
    Vec4 y = cross(z, x);
    Mat4 cob = Mat4::changeOfBasis(x, y, z, hit.point);
    return cob * Vec4(sinf(incl) * cosf(azim), sinf(incl) * sinf(azim),
                      cosf(incl), 0.0f);
}

RGBColor PhongSpecular::applyBRDF(const RGBColor &lightIn) const {
    float incl = this->tempIncl;
    float azimIncCos = this->tempAzimIncCos;
    if (azimIncCos < 1e-6f) {
        return RGBColor::Black;
    }
    float azimIncSin = sqrtf(1.0f - azimIncCos * azimIncCos);
    return lightIn * (azimIncCos * azimIncSin * (this->alpha + 2.0f) *
                      (1.0f / ((this->alpha + 1) + sinf(incl))));
}

/// Perfect Specular ///

Vec4 PerfectSpecular::nextRay(const Vec4 &inDirection, RayHit &hit) {
    return (hit.normal * 2.0f + inDirection).normalize();
}

RGBColor PerfectSpecular::applyBRDF(const RGBColor &lightIn) const {
    return lightIn * (1.0f / this->prob);
}

/// Material ///

BRDFPtr Material::selectEvent() {
    float event = random01();
    for (int i = 0; i < this->probs.size(); i++) {
        if (event < this->probs[i]) {
            return this->brdfs[i];
        }
    }
    return nullptr;
}