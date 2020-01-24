#include "photonemitter.h"

// Debug settings
// #define DEBUG_ONE_CORE  // don't use multithreading

void PhotonEmitter::savePhoton(const Photon &photon, const bool isCaustic) {
    if (isCaustic) {
        this->caustics.add(photon);
    } else {
        this->photons.add(photon);
    }
}

void PhotonEmitter::traceRay(Ray ray, const Scene &scene, RGBColor flux,
                             const bool isCaustic) {
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
            this->savePhoton(Photon(hit.point, ray.direction, flux), isCaustic);
            return;
        }
        // Select event for next photon
        event = hit.material->selectEvent();
        // Save INCOMING flux to the point
        if (event == nullptr || !event->nextRay(ray, hit, nextRay)) {
            this->savePhoton(Photon(hit.point, ray.direction, flux), isCaustic);
            return;
        }
        if (!event->isDelta) {
            this->savePhoton(Photon(hit.point, ray.direction, flux), isCaustic);
        }
        // Apply event and modify flux and ray
        flux =
            event->applyMonteCarlo(flux, hit, ray.direction, nextRay.direction);
        ray = nextRay;
    }
}

void PhotonEmitter::traceRays(
    const int totalPhotons, const RGBColor &emission,
    const std::function<Vec4()> &fOrigin,
    const std::function<Vec4(const Vec4 &)> &fDirection,
    const MediumPtr &medium, const Scene &scene) {
    // Spawn one core per thread and make them consume work as they finish
    volatile std::atomic<int> photonsEmitted(0);
#ifdef DEBUG_ONE_CORE
    int cores = 1;  // only one core, for debug purposes
#else
    int cores = std::thread::hardware_concurrency();  // max no. of threads
#endif
    std::vector<std::future<void>> threadFutures;  // waits for them to finish
    for (int core = 0; core < cores; core++) {
        threadFutures.emplace_back(std::async([&]() {
            while (true) {
                // Get next job ID (or stop if there aren't any)
                int currentRay = photonsEmitted++;
                if (currentRay >= totalPhotons) {
                    break;
                }
                Vec4 origin = fOrigin();
                Vec4 direction = fDirection(origin);
                // Generate photons for the point light
                traceRay(Ray(origin, direction, medium), scene, emission);
            }
        }));
    }

    bool finished = false;
    auto beginTime = std::chrono::system_clock::now().time_since_epoch();
    printProgress(beginTime, 0.0f);
    while (!finished) {
        printProgress(beginTime, photonsEmitted / (float)totalPhotons);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        for (auto &future : threadFutures) {
            if (future.wait_for(std::chrono::seconds(0)) ==
                std::future_status::ready) {
                finished = true;
                printProgress(beginTime, 1.0f);
                break;
            }
        }
    }
    std::cout << std::endl;  // space for more progress bars
}

void PhotonEmitter::emitPointLight(const Scene &scene,
                                   const PointLight &light) {
    int totalPhotons = light.emission.max() * (1.0f / lpp);
    RGBColor emission = light.emission * (lpp / totalPhotons);
    const auto fOrigin = [&light]() { return light.point; };
    const auto fDirection = [](const Vec4 &point) {
        // Inclination & azimuth for uniform cosine sampling
        float incl = acosf(1.0f - 2.0f * random01());
        float azim = 2.0f * M_PI * random01();
        return Vec4(sinf(incl) * cosf(azim), sinf(incl) * sinf(azim),
                    cosf(incl), 0.0f);
    };
    traceRays(totalPhotons, emission, fOrigin, fDirection, light.medium, scene);
}

void PhotonEmitter::emitAreaLight(const Scene &scene, const FigurePtr &figure,
                                  const RGBColor &emission,
                                  const MediumPtr &medium) {
    float area = figure->getTotalArea();
    int totalPhotons = emission.max() * (area / lpp);
    RGBColor photonEmission = emission * (lpp / totalPhotons);
    const auto fOrigin = [&figure]() { return figure->randomPoint(); };
    const auto fDirection = [&figure](const Vec4 &point) {
        return figure->randomDirection(point);
    };
    traceRays(totalPhotons, photonEmission, fOrigin, fDirection, medium, scene);
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