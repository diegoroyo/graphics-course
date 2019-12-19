#include "material.h"

// Calculate reflection of incoming ray with respect to normal
// used, for example, in perfect specular brdf
inline Vec4 reflectDirection(const Vec4 &incoming, const Vec4 &normal) {
    return incoming - normal * dot(incoming, normal) * 2.0f;
}

/// Phong Diffuse ///

bool PhongDiffuse::nextRay(const Vec4 &inDirection, const RayHit &hit,
                           Ray &outRay) {
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
    Vec4 outDirection = cob * Vec4(sinf(incl) * cosf(azim),
                                   sinf(incl) * sinf(azim), cosf(incl), 0.0f);
    outRay = Ray(hit.point, outDirection);
    return true;
}

RGBColor PhongDiffuse::applyBRDF(const RGBColor &lightIn) const {
    return lightIn * this->kd * (1.0f / this->prob);
}

RGBColor PhongDiffuse::applyDirect(const RGBColor &lightIn, const RayHit &hit,
                                   const Vec4 &wi) const {
    return lightIn * this->kd * (1.0f / M_PI);
}

/// Phong Specular ///

bool PhongSpecular::nextRay(const Vec4 &inDirection, const RayHit &hit,
                            Ray &outRay) {
    // Random inclination & azimuth
    float randIncl = random01();
    float randAzim = random01();
    // Phong Specular lobe sampling
    float incl = acosf(powf(randIncl, 1.0f / (this->alpha + 1.0f)));
    float azim = 2 * M_PI * randAzim;

    // Save for next iteration
    this->tempInCos = dot(inDirection, hit.normal) * -1.0f;
    this->tempOutDirection = inDirection * -1.0f;

    // Local base to hit point
    Vec4 z = reflectDirection(inDirection, hit.normal);
    Vec4 x = cross(z, inDirection).normalize();
    Vec4 y = cross(z, x);
    Mat4 cob = Mat4::changeOfBasis(x, y, z, Vec4());
    Vec4 outDirection = cob * Vec4(sinf(incl) * cosf(azim),
                                   sinf(incl) * sinf(azim), cosf(incl), 0.0f);
    outRay = Ray(hit.point, outDirection);
    float outCos = dot(outDirection, hit.normal);
    this->tempOutSin = sqrtf(1.0f - outCos * outCos);
    if (outCos < 1e-6f) {
        return false;
    } else {
        return true;
    }
}

RGBColor PhongSpecular::applyBRDF(const RGBColor &lightIn) const {
    float outSin = this->tempOutSin;
    float inCos = this->tempInCos;
    float inSin = sqrtf(1.0f - inCos * inCos);
    return lightIn * (inCos * inSin * (this->alpha + 2.0f) *
                      (1.0f / ((this->alpha + 1) + outSin)));
}

RGBColor PhongSpecular::applyDirect(const RGBColor &lightIn, const RayHit &hit,
                                    const Vec4 &wi) const {
    // cosine should be between light out direction (wo = inDirection * -1.0f)
    // which is saved in nextRay and light refraction direction (wr)
    Vec4 wo = this->tempOutDirection;
    Vec4 wr = reflectDirection(wi.normalize() * -1.0f, hit.normal);
    float cosInclR = dot(wr, wo);
    return lightIn * this->prob * fabs(powf(cosInclR, this->alpha)) *
           ((this->alpha + 2.0f) / (2.0f * M_PI));
}

/// Perfect Specular (delta BRDF) ///

bool PerfectSpecular::nextRay(const Vec4 &inDirection, const RayHit &hit,
                              Ray &outRay) {
    Vec4 outDirection = reflectDirection(inDirection, hit.normal);
    outRay = Ray(hit.point, outDirection);
    return true;
}

RGBColor PerfectSpecular::applyBRDF(const RGBColor &lightIn) const {
    // cos and ksp terms optimized out (cos * ksp) / (cos * ksp)
    return lightIn;
}

RGBColor PerfectSpecular::applyDirect(const RGBColor &lightIn,
                                      const RayHit &hit, const Vec4 &wi) const {
    // edge case of delta function ignored
    // outDirection == reflectDirection(inDirecion, hit.normal)
    return RGBColor::Black;
}

/// Perfect Refraction (delta BTDF) ///

// https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
bool PerfectRefraction::nextRay(const Vec4 &inDirection, const RayHit &hit,
                                Ray &outRay) {
    // Incoming ray's cosine and sine with respect to hit.normal
    float incCos = dot(inDirection, hit.normal) * -1.0f;
    float incSin = sqrtf(1.0f - incCos * incCos);
    // Index of Refraction ratio (depends if ray enters medium or leaves)
    float n1 = hit.enters ? 1.0f : this->mediumRefractiveIndex;
    float n2 = hit.enters ? this->mediumRefractiveIndex : 1.0f;
    float factor = n1 / n2;
    // Angle of outgoing ray
    float outSin = incSin * factor;
    if (outSin > 1.0f) {
        // Out angle is greater than critical angle (see reference)
        // By fresnel laws, ray is reflected
        Vec4 outDirection = reflectDirection(inDirection, hit.normal);
        outRay = Ray(hit.point, outDirection);
        return true;
    }

    // Calculate Kr, fresnel law says that light comes from two sources:
    // Kr comes from reflection, 1 - Kr comes from refraction
    float outCos = sqrtf(1.0f - outSin * outSin);
    // See reference for Kr calculation
    float rs = (n2 * incCos - n1 * outCos) / (n2 * incCos + n1 * outCos);
    float rp = (n2 * outCos - n1 * incCos) / (n2 * outCos + n1 * incCos);
    float kr = (rs * rs + rp * rp) / 2;  // mean of squares
    // select a random event (like roussian roulette)
    // perfect specular has probability kr, perfect refraction 1 - kr
    if (random01() < kr) {
        // specular
        Vec4 outDirection = reflectDirection(inDirection, hit.normal);
        outRay = Ray(hit.point, outDirection);
        return true;
    } else {
        // refraction
        float theta2 = asinf(outSin);
        // m is perpendicular to normal (see reference for details)
        Vec4 m = (inDirection + hit.normal * incCos) * (1.0f / incSin);
        Vec4 outDirection =
            hit.normal * -1.0f * cosf(theta2) + m * sinf(theta2);
        outRay = Ray(hit.point, outDirection);
        return true;
    }
}

RGBColor PerfectRefraction::applyBRDF(const RGBColor &lightIn) const {
    // cos and krp terms optimized out (cos * krp) / (cos * krp)
    // also it doesn't matter if the ray wasn't refracted and it was
    // reflected instead, because both interactions return the same
    return lightIn;
}

RGBColor PerfectRefraction::applyDirect(const RGBColor &lightIn,
                                        const RayHit &hit,
                                        const Vec4 &wi) const {
    // edge case ignored because refraction uses delta functions
    return RGBColor::Black;
}

/// Portal ///

bool Portal::nextRay(const Vec4 &inDirection, const RayHit &hit, Ray &outRay) {
    // Out portal basis
    Vec4 x = this->outPortal->uvX.normalize();
    Vec4 y = this->outPortal->uvY.normalize();
    Vec4 z = cross(x, y);

    // Out direction in global coords
    Mat4 cobIn = Mat4::changeOfBasis(this->inPortal->uvX.normalize(),
                                     this->inPortal->uvY.normalize(),
                                     hit.normal, this->inPortal->uvOrigin)
                     .inverse();

    Mat4 cob = Mat4::changeOfBasis(x, y, z, this->outPortal->uvOrigin);
    Vec4 coords = cobIn * inDirection;
    coords.z *= -1.0f;
    coords.x *= -1.0f;
    Vec4 outDirection = cob * coords;

    // Position coordinates in local base to in portal
    Vec4 d = hit.point - this->inPortal->uvOrigin;
    float bx =
        this->inPortal->uvX.module() - dot(d, this->inPortal->uvX.normalize());
    float by = dot(d, this->inPortal->uvY.normalize());
    Vec4 outPoint = cob * Vec4(bx, by, 0.0f, 1.0f);

    outRay = Ray(outPoint, outDirection);
    return true;
}

RGBColor Portal::applyBRDF(const RGBColor &lightIn) const {
    // technically ray didn't hit on a surface, no brdf required
    return lightIn;
}

RGBColor Portal::applyDirect(const RGBColor &lightIn, const RayHit &hit,
                             const Vec4 &wi) const {
    // ignore direct light
    return RGBColor::Black;
}

/// Material ///

MaterialBuilder MaterialBuilder::add(const BRDFPtr &brdf) {
    ptr->brdfs.push_back(brdf);
    accumProb += brdf->prob;
    ptr->probs.push_back(accumProb);
    if (accumProb >= 1.0f) {
        std::cout << "Warning: material has event probabilities"
                  << " that sum higher than 1" << std::endl;
        std::cout << "Accum. probabilities are: ";
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