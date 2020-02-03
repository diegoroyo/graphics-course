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

void PhotonEmitter::traceRay(Ray ray, const Scene &scene, RGBColor flux) {
    // Save original flux
    float initialFlux = flux.max();
    // Ignore first ray
    RayHit hit;
    if (!scene.intersection(ray, hit)) {
        return;
    }
    flux = HomAmbMedium::applyLight(flux, ray, hit);
    if (HomIsoMedium::rayEmit(scene, flux, ray, hit, volume)) {
        // Absorbed, already added to volume map
        return;
    }
    // Absorption event
    EventPtr event = hit.material->selectEvent();
    if (event == nullptr || !event->nextRay(ray, hit, ray)) {
        if (storeDirectLight && hit.material->getFirstDelta() == nullptr) {
            this->savePhoton(Photon(hit.point, ray.direction, flux), false);
        }
        return;
    }
    // Arrived at destination: store & apply BSDF
    if (storeDirectLight && !event->isDelta) {
        this->savePhoton(Photon(hit.point, ray.direction, flux), false);
    }
    flux = event->applyMonteCarlo(flux, hit, ray.direction, ray.direction);

    // Start storing photons on the second ray
    Ray nextRay;
    bool wasLastCaustic = false;
    while (scene.intersection(ray, hit) && flux.max() > initialFlux * CUT_PCT) {
        // Participative media
        flux = HomAmbMedium::applyLight(flux, ray, hit);
        if (HomIsoMedium::rayEmit(scene, flux, ray, hit, volume)) {
            // Absorbed, already added to volume map
            return;
        }
        if (hit.material->emitsLight) {
            // Save INCOMING flux and ignore light
            this->savePhoton(Photon(hit.point, ray.direction, flux),
                             wasLastCaustic);
            return;
        }
        // Select event for next photon
        event = hit.material->selectEvent();
        // Save INCOMING flux to the point
        if (event == nullptr || !event->nextRay(ray, hit, nextRay)) {
            if (hit.material->getFirstDelta() == nullptr) {
                this->savePhoton(Photon(hit.point, ray.direction, flux),
                                 wasLastCaustic);
            }
            return;
        }
        if (!event->isDelta) {
            this->savePhoton(Photon(hit.point, ray.direction, flux),
                             wasLastCaustic);
        }
        // Apply event and modify flux and ray
        flux =
            event->applyMonteCarlo(flux, hit, ray.direction, nextRay.direction);
        ray = nextRay;
        wasLastCaustic = event->isDelta;
    }
}

void PhotonEmitter::traceRays(
    const Scene &scene, const RGBColor &emission, const MediumPtr &medium,
    const std::function<void(Vec4 &, Vec4 &)> &fGetSample) {
    static std::mutex mutex;
    // Spawn one core per thread and make them consume work as they finish
    volatile std::atomic<int> photonsEmitted(0), generalShots(0);
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
                if (currentRay >= totalRays || this->isFull()) {
                    break;
                }
                if (!this->photons.isFull()) {
                    generalShots++;
                }
                mutex.lock();
                Vec4 origin, direction;
                fGetSample(origin, direction);
                // Generate photons for the point light
                traceRay(Ray(origin, direction, medium), scene, emission);
                mutex.unlock();
            }
        }));
    }

    // Wait for tasks to finish
    bool finished = false;
    auto beginTime = std::chrono::system_clock::now().time_since_epoch();
    printProgress(beginTime, 0.0f);
    while (!finished) {
        float progress = photonsEmitted / (float)totalRays;
        printProgress(beginTime, std::fminf(1.0f, progress));
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

    // Save shot rays for later normalization
    this->shotRays = generalShots;  // every core adds one
}

void PhotonEmitter::emitPointLights(const Scene &scene,
                                    const MediumPtr &medium) {
    // Determine how many rays should be shot from each source
    float totalWeight = 0.0f;
    RGBColor totalEmission(0.0f, 0.0f, 0.0f);
    for (const PointLight &light : scene.lights) {
        float weight = light.emission.max();
        totalWeight += weight;
        totalEmission = totalEmission + light.emission;
    }
    std::vector<float> weights;
    float accumWeight = 0.0f;
    for (const PointLight &light : scene.lights) {
        float weight = light.emission.max();
        accumWeight += weight / totalWeight;
        weights.push_back(accumWeight);
    }
    // Origin & direction sampling
    const auto fGetSample = [&scene, &weights](Vec4 &point, Vec4 &direction) {
        // Shoot photons from point lights using importance weighting
        float random = Random::ZeroOne();
        for (int i = 0; i < weights.size(); i++) {
            if (random < weights[i]) {
                point = scene.lights[i].point;
            }
        }
        // Direction uniform sampling on unit sphere
        direction = Random::Sphere();
    };
    // Shoot random photons
    traceRays(scene, totalEmission, medium, fGetSample);
}

void PhotonEmitter::emitAreaLight(const Scene &scene, const FigurePtr &figure,
                                  const RGBColor &areaEmission,
                                  const MediumPtr &medium) {
    const auto fGetSample = [&figure](Vec4 &point, Vec4 &direction) {
        point = figure->randomPoint();
        direction = figure->randomDirection(point);
    };
    traceRays(scene, areaEmission, medium, fGetSample);
}

/// Debug image ///

void debugPhotons(const PhotonKdTreeBuilder &tree, const Film &film,
                  const FigurePtr &filmPlane, RGBColor color, PPMImage &image) {
    // Intersect photon origin -> film origin with plane
    for (const Photon &photon : tree.photons) {
        Ray ray(photon.point, (film.origin - photon.point).normalize(),
                Medium::air);
        RayHit hit;
        // Map filmPlane's coordinates (0.0-1.0 for XY axis)
        Vec4 uvOrigin = film.origin + film.getPixelCenter(0, 0);
        // Up vector should be upside down
        Vec4 uvX = film.right * 2.0f, uvY = film.up * -2.0f;
        if (filmPlane->intersection(ray, hit)) {
            Vec4 d = hit.point - uvOrigin;
            // Get UV coordinates (check intersected pixel)
            float uvx = dot(d, uvX.normalize()) / uvX.module();
            float uvy = dot(d, uvY.normalize()) / uvY.module();
            // Pixel is inside image range
            if (uvx > 0.0f && uvx < 1.0f && uvy > 0.0f && uvy < 1.0f) {
                int pixelX = std::min(film.width - 1, (int)(uvx * film.width));
                int pixelY =
                    std::min(film.height - 1, (int)(uvy * film.height));
                RGBColor pixelColor = image.getPixel(pixelX, pixelY);
                // color = photon.flux;  // override color
                // Add flux to pixel color
                image.setPixel(pixelX, pixelY, pixelColor + color);
            }
        }
    }
}

PPMImage PhotonEmitter::debugPhotonsImage(const Film &film,
                                          const bool doPhotons,
                                          const bool doCaustics,
                                          const bool doVolume) {
    // Black image
    PPMImage image(film.width, film.height, std::numeric_limits<int>::max());
    image.fillPixels(RGBColor::Black);

    // Film plane
    FigurePtr filmPlane = FigurePtr(new Figures::FlatPlane(
        film.forward.normalize(),
        film.forward.module() +
            (film.origin.module() *
             dot(film.origin.normalize(), film.forward.normalize())),
        Material::none()));

    if (doPhotons) {
        debugPhotons(photons, film, filmPlane, RGBColor::Cyan, image);
    }
    if (doCaustics) {
        debugPhotons(caustics, film, filmPlane, RGBColor::White, image);
    }
    if (doVolume) {
        debugPhotons(volume, film, filmPlane, RGBColor::Red, image);
    }

    image.setMax(image.calculateMax());
    return image;
}