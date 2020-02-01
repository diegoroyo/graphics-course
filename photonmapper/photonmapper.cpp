#include "photonmapper.h"

RGBColor PhotonMapper::treeSearch(const PhotonKdTree &tree, const int kNN,
                                  const RayHit &hit,
                                  const Vec4 &outDirection) const {
    if (tree.empty()) {
        return RGBColor::Black;  // map is empty (e.g. caustics)
    }
    // Indirect light: get k-nearest photons on photon tree
    std::vector<const Photon *> nearest;
    float r = tree.searchNN(nearest, hit.point, kNN);
    // Indirect light using saved photons w/cone filter
    RGBColor sum(0.0f, 0.0f, 0.0f);
    int kNNCounted = kNN;
    for (const Photon *photon : nearest) {
        if (dot(hit.normal, photon->inDirection) > -1e-5f) {
            // Positive cosine (hitting the back of a plane, sphere, etc.)
            kNNCounted--;
        } else {
            // Photon contributes to light
            RGBColor contrib = hit.material->evaluate(
                photon->flux, hit, photon->inDirection, outDirection);
            float filterTerm = this->filter->photonTerm(hit, *photon, r, kNN);
            sum = sum + contrib * filterTerm;
        }
    }
    float kTerm = this->filter->kTerm(kNNCounted);
    float denominator = kTerm * M_PI * r * r;
    return sum * (1.0f / denominator);
}

RGBColor totalDirect(0.0f, 0.0f, 0.0f), totalIndirect(0.0f, 0.0f, 0.0f);
int total = 0;

RGBColor PhotonMapper::traceRay(const Ray &ray, const Scene &scene) const {
    RayHit hit;
    Vec4 outDirection = ray.direction * -1.0f;
    if (scene.intersection(ray, hit)) {
        // Light emitted by hit object
        RGBColor emittedLight(0.0f, 0.0f, 0.0f);
        if (hit.material->emitsLight) {
            emittedLight = hit.material->emission;
        }
        // Check for delta surfaces
        EventPtr delta = hit.material->getFirstDelta();
        Ray nextRay;
        if (delta != nullptr && delta->nextRay(ray, hit, nextRay)) {
            // Delta event, emitted light doesn't matter
            return traceRay(nextRay, scene) * delta->prob;
        }
        // Indirect light (normal + caustic)
        RGBColor indirectLight =
            treeSearch(photons, kNeighbours, hit, outDirection);
        RGBColor causticLight =
            treeSearch(caustics, kcNeighbours, hit, outDirection);
        // Direct light on point using scene
        RGBColor directLight(0.0f, 0.0f, 0.0f);
        if (directShadowRays) {
            directLight = scene.directLight(hit, outDirection);
            directLight = directLight * (1.0f / (4.0f * M_PI));
        }
        totalDirect = totalDirect + directLight;
        totalIndirect = totalIndirect + indirectLight;
        total = total + 1;
        return emittedLight + indirectLight + causticLight + directLight;
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
    std::cout << "total directo: " << totalDirect * (1.0f / total)
              << "\ntotal indirecto: " << totalIndirect * (1.0f / total)
              << "\nratios: " << totalDirect.r / totalIndirect.r << ", "
              << totalDirect.g / totalIndirect.g << ", "
              << totalDirect.b / totalIndirect.b << std::endl;
    render.setMax(render.calculateMax());
    return render;
}