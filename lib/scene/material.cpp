#include "material.h"

// Calculate reflection of incoming ray with respect to normal
// used, for example, in perfect specular brdf
inline Vec4 reflectDirection(const Vec4 &incoming, const Vec4 &normal) {
    return incoming - normal * dot(incoming, normal) * 2.0f;
}

// Generate a valid orthonormal base with normal as z
inline Vec4 baseFromNormal(const Vec4 &normal, Vec4 &x, Vec4 &y, Vec4 &z) {
    z = normal;
    if (std::fabs(z.x) > std::fabs(z.y)) {
        x = Vec4(z.z, 0.0f, z.x * -1.0f, 0.0f).normalize();
    } else {
        x = Vec4(0.0f, z.z * -1.0f, z.y, 0.0f).normalize();
    }
    y = cross(x, z);
}

/// Phong Diffuse ///

bool PhongDiffuse::nextRay(const Ray &inRay, const RayHit &hit, Ray &outRay) {
    // Local base to hit point
    Vec4 x, y, z;
    baseFromNormal(hit.normal, x, y, z);
    Mat4 cob = Mat4::changeOfBasis(x, y, z, Vec4());
    Vec4 outDirection = Random::CosHemisphere(cob);
    outRay = inRay.copy(hit.point, outDirection, hit);
    return true;
}

RGBColor PhongDiffuse::applyMonteCarlo(const RGBColor &lightIn,
                                       const RayHit &hit, const Vec4 &wi,
                                       const Vec4 &wo) const {
    return lightIn * this->kd * (1.0f / this->prob);
}

RGBColor PhongDiffuse::applyNextEvent(const RGBColor &lightIn,
                                      const RayHit &hit, const Vec4 &wi,
                                      const Vec4 &wo) const {
    return lightIn * this->kd * (1.0f / M_PI);
}

/// Phong Specular ///

bool PhongSpecular::nextRay(const Ray &inRay, const RayHit &hit, Ray &outRay) {
    // Random inclination & azimuth
    float randIncl = Random::ZeroOne();
    float randAzim = Random::ZeroOne();
    // Phong Specular lobe sampling
    float incl = acosf(powf(randIncl, 1.0f / (this->alpha + 1.0f)));
    float azim = 2 * M_PI * randAzim;

    // Local base to hit point
    Vec4 reflectNormal = reflectDirection(inRay.direction, hit.normal);
    Vec4 x, y, z;
    baseFromNormal(hit.normal, x, y, z);
    Mat4 cob = Mat4::changeOfBasis(x, y, z, Vec4());
    Vec4 outDirection = cob * Vec4(sinf(incl) * cosf(azim),
                                   sinf(incl) * sinf(azim), cosf(incl), 0.0f);
    outRay = inRay.copy(hit.point, outDirection, hit);
    if (dot(outDirection, hit.normal) < 1e-6f) {
        return false;
    } else {
        return true;
    }
}

RGBColor PhongSpecular::applyMonteCarlo(const RGBColor &lightIn,
                                        const RayHit &hit, const Vec4 &wi,
                                        const Vec4 &wo) const {
    Vec4 wr = reflectDirection(wi, hit.normal);
    float outCos = dot(wr, hit.normal);
    // add epsilon to prevent negative sqrts
    float outSin = sqrtf(1.0f + 1e-5f - outCos * outCos);
    float inCos = dot(wi, hit.normal) * -1.0f;
    // add epsilon to prevent negative sqrts
    float inSin = sqrtf(1.0f + 1e-5f - inCos * inCos);
    return lightIn * (inCos * inSin * (this->alpha + 2.0f) *
                      (1.0f / ((this->alpha + 1) + outSin)));
}

RGBColor PhongSpecular::applyNextEvent(const RGBColor &lightIn,
                                       const RayHit &hit, const Vec4 &wi,
                                       const Vec4 &wo) const {
    // cosine should be between light out direction (wo = inDirection * -1.0f)
    // which is saved in nextRay and light refraction direction (wr)
    Vec4 wr = reflectDirection(wi.normalize() * -1.0f, hit.normal);
    float cosInclR = dot(wr, wo);
    return lightIn * this->prob * fabs(powf(cosInclR, this->alpha)) *
           ((this->alpha + 2.0f) / (2.0f * M_PI));
}

/// Perfect Specular (delta BRDF) ///

bool PerfectSpecular::nextRay(const Ray &inRay, const RayHit &hit,
                              Ray &outRay) {
    Vec4 outDirection = reflectDirection(inRay.direction, hit.normal);
    outRay = inRay.copy(hit.point, outDirection, hit);
    return true;
}

RGBColor PerfectSpecular::applyMonteCarlo(const RGBColor &lightIn,
                                          const RayHit &hit, const Vec4 &wi,
                                          const Vec4 &wo) const {
    // cos and ksp terms optimized out (cos * ksp) / (cos * ksp)
    return lightIn;
}

RGBColor PerfectSpecular::applyNextEvent(const RGBColor &lightIn,
                                         const RayHit &hit, const Vec4 &wi,
                                         const Vec4 &wo) const {
    // edge case of delta function ignored
    // outDirection == reflectDirection(inDirecion, hit.normal)
    return RGBColor::Black;
}

/// Perfect Refraction (delta BTDF) ///

bool PerfectRefraction::isFresnelDisabled = false;

// https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
bool PerfectRefraction::nextRay(const Ray &inRay, const RayHit &hit,
                                Ray &outRay) {
    // Incoming ray's cosine and sine with respect to hit.normal
    float incCos = dot(inRay.direction, hit.normal) * -1.0f;
    // add epsilon to prevent negative sqrts
    float incSin = sqrtf(1.0f + 1e-5f - incCos * incCos);
    // Index of Refraction ratio (depends if ray enters medium or leaves)
    MediumPtr inMedium = hit.enters ? Medium::air : this->medium;
    MediumPtr outMedium = hit.enters ? this->medium : Medium::air;
    float n1 = inMedium->refractiveIndex;
    float n2 = outMedium->refractiveIndex;
    float factor = n1 / n2;
    // Angle of outgoing ray
    float outSin = incSin * factor;
    if (outSin > 1.0f) {
        // Out angle is greater than critical angle (see reference)
        // By fresnel laws, ray is reflected
        Vec4 outDirection = reflectDirection(inRay.direction, hit.normal);
        outRay = inRay.copy(hit.point, outDirection, hit, inMedium);
        return true;
    }

    // Calculate Kr, fresnel law says that light comes from two sources:
    // Kr comes from reflection, 1 - Kr comes from refraction
    // add epsilon to prevent negative sqrts
    float outCos = sqrtf(1.0f + 1e-5f - outSin * outSin);
    // See reference for Kr calculation
    float rs = (n2 * incCos - n1 * outCos) / (n2 * incCos + n1 * outCos);
    float rp = (n2 * outCos - n1 * incCos) / (n2 * outCos + n1 * incCos);
    float kr = 0.0f;
    if (!isFresnelDisabled) {
        kr = (rs * rs + rp * rp) / 2;  // mean of squares
    }
    // select a random event (like roussian roulette)
    // perfect specular has probability kr, perfect refraction 1 - kr
    if (Random::ZeroOne() < kr) {
        // specular
        Vec4 outDirection = reflectDirection(inRay.direction, hit.normal);
        outRay = inRay.copy(hit.point, outDirection, hit, inMedium);
        return true;
    } else {
        // refraction
        float theta2 = asinf(outSin);
        // m is perpendicular to normal (see reference for details)
        Vec4 m = (inRay.direction + hit.normal * incCos) * (1.0f / incSin);
        Vec4 outDirection =
            hit.normal * -1.0f * cosf(theta2) + m * sinf(theta2);
        outRay = inRay.copy(hit.point, outDirection, hit, outMedium);
        return true;
    }
}

RGBColor PerfectRefraction::applyMonteCarlo(const RGBColor &lightIn,
                                            const RayHit &hit, const Vec4 &wi,
                                            const Vec4 &wo) const {
    // cos and krp terms optimized out (cos * krp) / (cos * krp)
    // also it doesn't matter if the ray wasn't refracted and it was
    // reflected instead, because both interactions return the same
    return lightIn;
}

RGBColor PerfectRefraction::applyNextEvent(const RGBColor &lightIn,
                                           const RayHit &hit, const Vec4 &wi,
                                           const Vec4 &wo) const {
    // edge case ignored because refraction uses delta functions
    return RGBColor::Black;
}

/// Portal ///

bool Portal::nextRay(const Ray &inRay, const RayHit &hit, Ray &outRay) {
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
    Vec4 coords = cobIn * inRay.direction;
    coords.z *= -1.0f;
    coords.x *= -1.0f;
    Vec4 outDirection = cob * coords;

    // Position coordinates in local base to in portal
    Vec4 d = hit.point - this->inPortal->uvOrigin;
    float bx =
        this->inPortal->uvX.module() - dot(d, this->inPortal->uvX.normalize());
    float by = dot(d, this->inPortal->uvY.normalize());
    Vec4 outPoint = cob * Vec4(bx, by, 0.0f, 1.0f);

    outRay = inRay.copy(hit.point, outDirection, hit);
    return true;
}

RGBColor Portal::applyMonteCarlo(const RGBColor &lightIn, const RayHit &hit,
                                 const Vec4 &wi, const Vec4 &wo) const {
    // technically ray didn't hit on a surface, no brdf required
    return lightIn;
}

RGBColor Portal::applyNextEvent(const RGBColor &lightIn, const RayHit &hit,
                                const Vec4 &wi, const Vec4 &wo) const {
    // ignore direct light
    return RGBColor::Black;
}

/// Material ///

MaterialBuilder MaterialBuilder::add(const EventPtr &event) {
    // Ignore zero-probability events (maybe from textures)
    if (event->prob == 0.0f) {
        return *this;
    }
    // Add event & update accum probability
    ptr->events.push_back(event);
    accumProb += event->prob;
    ptr->probs.push_back(accumProb);
    if (accumProb >= 1.0f) {
        std::cout << "Warning: material has event probabilities"
                  << " that sum higher than 1 (reescaling...)" << std::endl;
        std::cout << "Accum. probabilities are: ";
        for (float p : ptr->probs) {
            std::cout << p << " ";
        }
        std::cout << std::endl;
        for (float &p : ptr->probs) {
            p = p / accumProb;
        }
    }
    return *this;
}

EventPtr Material::selectEvent() {
    float event = Random::ZeroOne();
    for (int i = 0; i < this->probs.size(); i++) {
        if (event < this->probs[i]) {
            return this->events[i];
        }
    }
    return nullptr;
}

EventPtr Material::getFirstDelta() const {
    for (const EventPtr &event : this->events) {
        if (event->isDelta) {
            return event;
        }
    }
    return nullptr;
}

RGBColor Material::evaluate(const RGBColor &lightIn, const RayHit &hit,
                            const Vec4 &wi, const Vec4 &wo) const {
    RGBColor result(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < events.size(); i++) {
        RGBColor factor(1.0f, 1.0f, 1.0f);
        factor = this->events[i]->applyNextEvent(factor, hit, wi, wo);
        result = result + lightIn * factor * (1.0f / this->probs[i]);
    }
    return result;
}