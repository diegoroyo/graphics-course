#include "photonemitter.h"

void PhotonEmitter::traceRay(Ray ray, const Scene &scene, RGBColor flux) {
    // Ignore first ray
    RayHit hit;
    if (!scene.intersection(ray, hit)) {
        return;
    }
    EventPtr event = hit.material->selectEvent();
    if (event == nullptr || !event->nextRay(ray, hit, ray)) {
        return;
    }

    // Start storing photons on the second ray
    Ray nextRay;
    // TODO modifcar umbral de corte
    while (scene.intersection(ray, hit) && flux.max() > 1e-5f) {
        if (hit.material->emitsLight) {
            // TODO comprobar si esto esta bien
            // Save INCOMING flux and ignore light
            photons.add(Photon(hit.point, ray.direction, flux));
            return;
        }
        event = hit.material->selectEvent();
        if (event == nullptr || !event->nextRay(ray, hit, nextRay)) {
            return;
        }
        // Save INCOMING flux to the point
        photons.add(Photon(hit.point, ray.direction, flux));
        // Apply event and modify flux and ray
        flux =
            event->applyMonteCarlo(flux, hit, ray.direction, nextRay.direction);
        ray = nextRay;
    }
}

void PhotonEmitter::emitPointLight(const Scene &scene,
                                   const PointLight &light) {
    for (int i = 0; i < ppa; i++) {
        // Inclination & azimuth for uniform cosine sampling
        float incl = acosf(1 - 2 * random01());
        float azim = 2 * M_PI * random01();
        Vec4 direction = Vec4(sinf(incl) * cosf(azim), sinf(incl) * sinf(azim),
                              cosf(incl), 0.0f);
        // Generate photons for the point light
        // TODO compobar el medio
        traceRay(Ray(light.point, direction, Medium::air), scene,
                 light.emission);
    }
}