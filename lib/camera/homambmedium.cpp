#include "homambmedium.h"

RGBColor HomAmbMedium::fApplyLight(const RGBColor &light, const Ray &ray,
                                   const RayHit &hit) const {
    float d = hit.distance;
    float expTerm = expf(-1.0f * d * kExtinction);
    float inScatterTerm = kScattering * (1.0f - expTerm) / kExtinction;
    RGBColor inScatter = inScatterConstant * inScatterTerm;
    return light * expTerm + inScatter;
}