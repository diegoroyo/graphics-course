#include "photonmapper.h"

RGBColor PhotonMapper::traceRay(const Ray &ray, const Scene &scene) const {
    RayHit hit;
    Vec4 outDirection = ray.direction * -1.0f;
    if (scene.intersection(ray, hit)) {
        // Light emitted by hit object
        RGBColor emittedLight = RGBColor::Black;
        if (hit.material->emitsLight) {
            emittedLight = hit.material->emission;
        }
        // Indirect light: get k-nearest photons on photon tree
        std::vector<const Photon *> nearest;
        float r = this->photons.searchNN(nearest, hit.point, this->kNeighbours);
        // Select random event on surface hit
        // TODO se elige evento aleatorio con varios ppp (?)
        EventPtr event = hit.material->selectEvent();
        // Check for delta surfaces
        Ray nextRay;
        if (event == nullptr ||
            (event->isDelta && !event->nextRay(ray, hit, nextRay))) {
            return emittedLight;
        } else if (event->isDelta) {
            // Delta event, emitted light doesn't matter
            return traceRay(nextRay, scene);
        }
        // Indirect light using saved photons w/cone filter
        RGBColor sum(0.0f, 0.0f, 0.0f);
        for (const Photon *photon : nearest) {
            RGBColor contrib = event->applyNextEvent(
                photon->flux, hit, photon->inDirection, outDirection);
            float filter = 1.0f - ((photon->point - hit.point).module() /
                                   (this->kNeighbours * r));
            sum = sum + contrib * filter;
        }
        float kTerm = (1.0f - (2.0f / (3.0f * this->kNeighbours)));
        float denominator = kTerm * M_PI * r * r;
        RGBColor indirectLight = sum * (1.0f / denominator);
        // Direct light on point using scene
        // TODO luz directa usando el primer rebote de los fotones (?)
        RGBColor directLight = scene.directLight(hit, outDirection, event);
        directLight = directLight * (1.0f / (4.0f * M_PI));  // normalization
        return emittedLight + indirectLight + directLight;
    }
    // Didn't hit with anything on the scene
    return scene.backgroundColor;
}

void PhotonMapper::tracePixel(const int px, const int py, const Film &film,
                              const Scene &scene) {
    RGBColor pixelColor(0.0f, 0.0f, 0.0f);
    Vec4 pixelCenter = film.getPixelCenter(px, py);
    for (int p = 0; p < ppp; ++p) {
        float randX = random01();
        float randY = random01();
        Vec4 direction =
            pixelCenter + film.deltaX * randX + film.deltaY * randY;
        // Trace ray and store mean in result
        Ray ray(film.origin, direction.normalize(), scene.air);
        RGBColor rayColor = this->traceRay(ray, scene);
        if (rayColor.max() > scene.maxLightEmission) {
            rayColor = rayColor * (scene.maxLightEmission / rayColor.max());
        }
        pixelColor = pixelColor + rayColor * (1.0f / ppp);
    }
    this->render.setPixel(px, py, pixelColor);
}

PPMImage &PhotonMapper::result() {
    // Set result's max value to the render's max value
    render.setMax(render.calculateMax());
    return render;
}