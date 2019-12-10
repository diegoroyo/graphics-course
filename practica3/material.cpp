#include "material.h"

/// Phong Diffuse ///

bool PhongDiffuse::nextRay(const Vec4 &inDirection, const RayHit &hit,
                           Vec4 &outDirection) {
    // Random inclination & azimuth
    float randIncl = random01();
    float randAzim = random01();
    // Inclination & azimuth for uniform cosine sampling
    float incl = acosf(sqrtf(randIncl));
    float azim = 2 * M_PI * randAzim;

    // Local base to hit point
    Vec4 z = hit.normal;
    Vec4 x = cross(z, inDirection).normalize();
    Vec4 y = cross(z, x);
    Mat4 cob = Mat4::changeOfBasis(x, y, z, Vec4());
    outDirection = cob * Vec4(sinf(incl) * cosf(azim), sinf(incl) * sinf(azim),
                              cosf(incl), 0.0f);
    return true;
}

RGBColor PhongDiffuse::applyBRDF(const RGBColor &lightIn) const {
    return lightIn * this->kdProb;
}

/// Phong Specular ///

bool PhongSpecular::nextRay(const Vec4 &inDirection, const RayHit &hit,
                            Vec4 &outDirection) {
    // Random inclination & azimuth
    float randIncl = random01();
    float randAzim = random01();
    // Phong Specular lobe sampling
    float incl = acosf(powf(randIncl, 1.0f / (this->alpha + 1)));
    float azim = 2 * M_PI * randAzim;

    // Save for next iteration
    this->tempAzimIncCos = dot(inDirection, hit.normal) * -1.0f;
    this->tempIncl = incl;

    // Local base to hit point
    Vec4 z = (hit.normal * 2.0f + inDirection).normalize();
    Vec4 x = cross(z, inDirection).normalize();
    Vec4 y = cross(z, x);
    Mat4 cob = Mat4::changeOfBasis(x, y, z, Vec4());
    outDirection = cob * Vec4(sinf(incl) * cosf(azim), sinf(incl) * sinf(azim),
                              cosf(incl), 0.0f);
    this->tempIncl = dot(outDirection, hit.normal);
    if (tempIncl < 1e-6f) {
        return false;
    } else {
        return true;
    }
}

RGBColor PhongSpecular::applyBRDF(const RGBColor &lightIn) const {
    float incl = this->tempIncl;
    float azimIncCos = this->tempAzimIncCos;
    float azimIncSin = sqrtf(1.0f - azimIncCos * azimIncCos);
    return lightIn * (azimIncCos * azimIncSin * (this->alpha + 2.0f) *
                      (1.0f / ((this->alpha + 1) + sinf(incl))));
}

/// Perfect Specular (delta BRDF) ///

bool PerfectSpecular::nextRay(const Vec4 &inDirection, const RayHit &hit,
                              Vec4 &outDirection) {
    outDirection =
        inDirection - hit.normal * dot(inDirection, hit.normal) * 2.0f;
    return true;
}

RGBColor PerfectSpecular::applyBRDF(const RGBColor &lightIn) const {
    return lightIn * (1.0f / this->prob);
}

/// Perfect Refraction (delta BTDF) ///

// https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
bool PerfectRefraction::nextRay(const Vec4 &inDirection, const RayHit &hit,
                                Vec4 &outDirection) {
    // Incoming ray's cosine and sine with respect to hit.normal
    float incCos = dot(inDirection, hit.normal) * -1.0f;
    float incSin = sqrtf(1.0f - incCos * incCos);
    // Index of Refraction ratio (depends if ray enters medium or leaves)
    float factor = hit.enters ? 1.0f / this->mediumRefractiveIndex
                              : this->mediumRefractiveIndex;
    // Angle of outgoing ray
    float outSin = incSin * factor;
    if (outSin > 1.0f) {
        // Out angle is greater than critical angle (see reference)
        return false;
    }
    float theta2 = asinf(outSin);
    // m is perpendicular to normal (see reference for details)
    Vec4 m = (inDirection + hit.normal * incCos) * (1.0f / incSin);
    outDirection = hit.normal * -1.0f * cosf(theta2) + m * sinf(theta2);
    return true;
}

RGBColor PerfectRefraction::applyBRDF(const RGBColor &lightIn) const {
    return lightIn * (1.0f / this->prob);
}

/// Material ///

MaterialBuilder MaterialBuilder::add(const BRDFPtr &brdf) {
    ptr->brdfs.push_back(brdf);
    accumProb += brdf->prob;
    ptr->probs.push_back(accumProb);
    if (accumProb >= 1.0f) {
        std::cout << "Warning: material has event probabilities"
                  << " that sum higher than 1" << std::endl;
        std::cout << "Probabilities are: ";
        for (float p : ptr->probs) {
            std::cout << p << " ";
        }
        std::cout << std::endl;
    }
    return *this;
}

BRDFPtr Material::selectEvent() {
    float event = random01();
    for (int i = 0; i < this->probs.size(); i++) {
        if (event < this->probs[i]) {
            return this->brdfs[i];
        }
    }
    return nullptr;
}