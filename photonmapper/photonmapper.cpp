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
        std::vector<const Photon *> nearest;
        float r = this->photons.searchNN(nearest, hit.point, this->kNeighbours);
        RGBColor sum(0.0f, 0.0f, 0.0f);
        // TODO no se qué clase de evento hay que coger para la BRDF,
        // habra que cambiar esto, por ahora lo dejo como evento random
        // dicen que con difuso o especular de phong va bien
        EventPtr event = hit.material->selectEvent();
        if (event == nullptr) {
            return emittedLight;
        }
        RGBColor directLight = scene.directLight(hit, outDirection, event);
        for (const Photon *photon : nearest) {
            RGBColor contrib = event->applyNextEvent(
                photon->flux, hit, photon->inDirection, outDirection);
            float filter = 1.0f; // TODO restaurar filtro de cono
            // float filter = 1.0f - ((photon->point - hit.point).module() /
            //                        (this->kNeighbours * r));
            sum = sum + contrib * filter;
        }
        float kTerm = 1.0f; // TODO restaurar filtro de cono
        // float kTerm = (1.0f - (2.0f / (3.0f * this->kNeighbours)));
        float denominator = kTerm * M_PI * r * r;
        RGBColor estimatedLight = sum * (1.0f / denominator);
        return emittedLight + estimatedLight + directLight;
    }
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