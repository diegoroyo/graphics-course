#include "homisomedium.h"

bool HomIsoMedium::fRayEmit(const Scene &scene, const RGBColor &light, Ray &ray,
                            RayHit &hit, PhotonKdTreeBuilder &volume) const {
    float d = -1.0f * logf(Random::ZeroOne()) / this->kExtinction;
    float total = ray.distanceWithoutEvent + hit.distance;
    if (d < total) {  // event occurs before hit
        float travelled = total - d;
        // Russian roulette
        float random = Random::ZeroOne();
        ray = ray.event(travelled);
        RGBColor transmitted = fApplyTransmittance(light, travelled);
        volume.add(Photon(ray.origin, ray.direction, transmitted));
        if (random < this->kScattering / this->kExtinction) {
            // Add to volume map
            // Scattering event: new ray
            ray.direction = Random::Sphere();
            if (scene.intersection(ray, hit)) {
                return fRayEmit(scene, transmitted, ray, hit, volume);
            } else {
                // Didn't hit with anything, "absorbed"
                return true;
            }
        } else {
            // Ray absorbed
            return true;
        }
    }
    return false;  // no event
}

RGBColor HomIsoMedium::fApplyTransmittance(const RGBColor &light,
                                           const float distance) const {
    float expTerm = expf(-1.0f * distance * kExtinction);
    return light * expTerm;
}

RGBColor HomIsoMedium::volumeSearch(const PhotonKdTree &volume, const int kNN,
                                    const Vec4 &point) const {
    if (volume.empty() || kNN == 0) {
        return RGBColor::Black;  // map is empty (e.g. caustics)
    }
    // Indirect light: get k-nearest photons on photon tree
    std::vector<const Photon *> nearest;
    float r = volume.searchNN(nearest, point, kNN);
    // Indirect light using saved photons w/cone filter
    RGBColor sum(0.0f, 0.0f, 0.0f);
    for (const Photon *photon : nearest) {
        // Photon contributes to light
        sum = sum + photon->flux;
    }
    float sphereVolume = 4.0f * M_PI * r * r * r / 3.0f;
    float phaseTerm = 1.0f / (4.0f * M_PI);  // isotropic
    return sum * (phaseTerm / sphereVolume);
}

RGBColor HomIsoMedium::fRayMarchTrace(const RGBColor &lightIn, const Ray &ray,
                                      const RayHit &hit,
                                      const PhotonKdTree &volume,
                                      const int kNN) const {
    int steps = hit.distance / this->deltaD;  // last step treated differently
    RGBColor lightOut(lightIn);
    for (int s = 1; s <= steps; s++) {
        lightOut = fApplyTransmittance(lightOut, deltaD);
        Vec4 point = ray.project(deltaD * s);
        // don't need to multiply by kScattering as volumeSearch divides by it
        lightOut = lightOut + volumeSearch(volume, kNN, point) * this->deltaD;
    }
    float rest = hit.distance - (this->deltaD * steps);
    if (rest > 1e-5f) {
        lightOut = fApplyTransmittance(lightOut, rest);
        // don't need to multiply by kScattering as volumeSearch divides by it
        lightOut = lightOut + volumeSearch(volume, kNN, hit.point) * rest;
    }
    return lightOut;
}