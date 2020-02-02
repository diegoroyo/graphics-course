#include "homisomedium.h"

void HomIsoMedium::fRayMarchEmit(const Ray &ray, const RayHit &hit,
                                 PhotonKdTreeBuilder &volume) const {
    return; // TODO
}

RGBColor HomIsoMedium::fRayMarchTrace(const RGBColor &light, const Ray &ray,
                                      const RayHit &hit) const {
    return RGBColor::Black; // TODO
}