#include "pathtracer.h"

// Debug settings
// #define DEBUG_PATH      // show path's hits and moment of stopping

RGBColor PathTracer::traceRay(const Ray &ray, const Scene &scene) const {
    // Ray from camera's origin to pixel's center
    RayHit hit;
    if (scene.intersection(ray, hit)) {
        // Special case: hit a light
        if (hit.material->emitsLight) {
// Return the light emission
#ifdef DEBUG_PATH
            std::cout << "Light hit on point " << hit.point << " with normal "
                      << hit.normal << std::endl;
#endif
            return hit.material->emission;
        }

        // Calculate russian roulette event
        EventPtr event = hit.material->selectEvent();
        // Only calculate direct light if event is not perfect refraction
        Ray nextRay;
        if (event != nullptr && event->nextRay(ray, hit, nextRay)) {
#ifdef DEBUG_PATH
            std::cout << "Event on point " << hit.point << " with normal "
                      << hit.normal << std::endl;
#endif
            // Wo, direction where light is going out
            Vec4 backDir = ray.direction * -1.0f;
            // Wi, direction where light is coming in next iteration
            Vec4 nextDir = nextRay.direction * -1.0f;
            // Get direct light & next event contributions
            RGBColor directLight = scene.directLight(hit, backDir);
            RGBColor nextEventLight = event->applyMonteCarlo(
                traceRay(nextRay, scene), hit, nextDir, backDir);
            return nextEventLight + directLight;
#ifdef DEBUG_PATH
        } else {
            std::cout << "Path died :(" << std::endl;
#endif
        }

        // Path died for one reason or another
        return RGBColor::Black;
    }
#ifdef DEBUG_PATH
    std::cout << "Ray didn't collide with anything" << std::endl;
#endif
    return scene.backgroundColor;
}

void PathTracer::tracePixel(const int px, const int py, const Film &film,
                            const Scene &scene) {
    RGBColor pixelColor(0.0f, 0.0f, 0.0f);
    Vec4 pixelCenter = film.getPixelCenter(px, py);
    for (int p = 0; p < ppp; ++p) {
        float randX = Random::ZeroOne();
        float randY = Random::ZeroOne();
        Vec4 dof = film.getDoFDisplacement();
        Vec4 direction =
            pixelCenter + film.deltaX * randX + film.deltaY * randY;
        // Trace ray and store mean in result
        Ray ray(film.origin + dof, (direction - dof).normalize(), scene.air);
#ifdef DEBUG_PATH
        std::cout << std::endl << "> Ray begins" << std::endl;
#endif
        RGBColor rayColor = this->traceRay(ray, scene);
        if (rayColor.max() > scene.maxLightEmission) {
            rayColor = rayColor * (scene.maxLightEmission / rayColor.max());
        }
        pixelColor = pixelColor + rayColor * (1.0f / ppp);
    }
    this->render.setPixel(px, py, pixelColor);
}

PPMImage &PathTracer::result() {
    // Set result's max value to the render's max value
    render.setMax(render.calculateMax());
    return render;
}