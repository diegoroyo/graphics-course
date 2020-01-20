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
    flux = event->applyMonteCarlo(flux, hit, ray.direction, ray.direction);

    // Start storing photons on the second ray
    Ray nextRay;
    while (scene.intersection(ray, hit) && flux.max() > lpp * CUT_PCT) {
        if (hit.material->emitsLight) {
            // Save INCOMING flux and ignore light
            photons.add(Photon(hit.point, ray.direction, flux));
            return;
        }
        // Save INCOMING flux to the point
        photons.add(Photon(hit.point, ray.direction, flux));
        // Select event for next photon
        event = hit.material->selectEvent();
        if (event == nullptr || !event->nextRay(ray, hit, nextRay)) {
            return;
        }
        // Apply event and modify flux and ray
        flux =
            event->applyMonteCarlo(flux, hit, ray.direction, nextRay.direction);
        ray = nextRay;
    }
}

void PhotonEmitter::emitPointLight(const Scene &scene,
                                   const PointLight &light) {
    // Brighter lights emit more photons, but all should be of same energy
    int numPhotons = light.emission.max() * (1.0f / lpp);
    RGBColor emission = light.emission * (lpp / numPhotons);
    for (int i = 0; i < numPhotons; i++) {
        // Inclination & azimuth for uniform cosine sampling
        float incl = acosf(1.0f - 2.0f * random01());
        float azim = 2.0f * M_PI * random01();
        Vec4 direction = Vec4(sinf(incl) * cosf(azim), sinf(incl) * sinf(azim),
                              cosf(incl), 0.0f);
        // Generate photons for the point light
        traceRay(Ray(light.point, direction, light.medium), scene, emission);
    }
}

PPMImage PhotonEmitter::debugPhotonsImage(const Film &film) {
    // Black image
    PPMImage image(film.width, film.height, std::numeric_limits<int>::max());
    image.fillPixels(RGBColor::Black);

    // Film plane
    FigurePtr plane = FigurePtr(new Figures::FlatPlane(
        film.forward.normalize(),
        film.forward.module() +
            (film.origin.module() *
             dot(film.origin.normalize(), film.forward.normalize())),
        Material::none()));

    // Intersect photon origin -> film origin with plane
    for (const Photon &photon : photons.photons) {
        Ray ray(photon.point, film.origin - photon.point, Medium::air);
        RayHit hit;
        Vec4 uvOrigin = film.getPixelCenter(0, 0);
        Vec4 uvX = film.right, uvY = film.up;
        if (plane->intersection(ray, hit)) {
            Vec4 d = hit.point - uvOrigin;
            // Get UV coordinates (check intersected pixel)
            float uvx = dot(d, uvX.normalize()) / uvX.module();
            float uvy = dot(d, uvY.normalize()) / uvY.module();
            // Pixel is inside image range
            if (uvx > 0.0f && uvx < 1.0f && uvy > 0.0f && uvy < 1.0f) {
                int pixelX = std::min(film.width - 1, (int)(uvx * film.width));
                int pixelY =
                    std::min(film.height - 1, (int)(uvy * film.height));
                RGBColor color = image.getPixel(pixelX, pixelY);
                // Add flux to pixel color
                image.setPixel(pixelX, pixelY, color + photon.flux);
            }
        }
    }

    image.setMax(image.calculateMax());
    return image;
}