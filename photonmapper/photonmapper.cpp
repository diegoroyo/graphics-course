#include "photonmapper.h"

RGBColor PhotonMapper::directLightMedium(const Scene &scene, const RayHit &hit,
                                         const Vec4 &wo) const {
    RGBColor result(0.0f, 0.0f, 0.0f);
    // Check all lights in the scene
    for (const auto &light : scene.lights) {
        Vec4 wi = light.point - hit.point;
        float norm = wi.module();
        wi = wi * (1.0f / norm);
        RayHit hit;
        Ray ray(light.point, wi * -1.0f, scene.air);
        // Check if theres direct view from light to point
        if (scene.intersection(ray, hit) &&
            std::abs(hit.distance - norm) < 1e-5f &&
            dot(hit.normal, ray.direction) < 1e-5f) {
            // Add light's emission to the result
            RGBColor inEmission = light.emission * (1.0f / (norm * norm));
            inEmission = HomAmbMedium::applyLight(inEmission, ray, hit);
            inEmission = HomIsoMedium::rayMarch(inEmission, ray, hit, volume,
                                                kvNeighbours);
            result = result + hit.material->evaluate(inEmission, hit, wi, wo) *
                                  dot(hit.normal, wi);
        }
    }
    return result * (1.0f / (4.0f * M_PI));
}

RGBColor PhotonMapper::treeSearch(const PhotonKdTree &tree, const int kNN,
                                  const RayHit &hit,
                                  const Vec4 &outDirection) const {
    if (tree.empty() || kNN == 0) {
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

// TODO quitar cuando este seguro de que se calcula bien la energia
RGBColor totalDirect(0.0f, 0.0f, 0.0f), totalIndirect(0.0f, 0.0f, 0.0f);
int total = 0;

RGBColor PhotonMapper::traceRay(const Ray &ray, const Scene &scene,
                                const int level) const {
    RayHit hit;
    Vec4 outDirection = ray.direction * -1.0f;
    if (scene.intersection(ray, hit)) {
        // Light emitted by hit object
        RGBColor emitLight(0.0f, 0.0f, 0.0f);
        if (hit.material->emitsLight) {
            emitLight = hit.material->emission;
        }
        // Check for delta surfaces
        EventPtr delta = hit.material->getFirstDelta();
        Ray nextRay;
        if (delta != nullptr && delta->nextRay(ray, hit, nextRay)) {
            // Delta event, emitted light doesn't matter
            if (level > MAX_LEVEL) {
                // Don't go too far in recursion
                return RGBColor::Black;
            }
            RGBColor next = traceRay(nextRay, scene, level + 1) * delta->prob;
            next = HomAmbMedium::applyLight(next, ray, hit);
            next = HomIsoMedium::rayMarch(next, ray, hit, volume, kvNeighbours);
            return next;
        }
        // Indirect light (normal + caustic)
        RGBColor indirectLight =
            treeSearch(photons, kNeighbours, hit, outDirection);
        RGBColor causticLight =
            treeSearch(caustics, kcNeighbours, hit, outDirection);
        // Direct light on point using scene
        RGBColor directLight(0.0f, 0.0f, 0.0f);
        if (directShadowRays) {
            directLight = directLightMedium(scene, hit, outDirection);
        }
        // TODO quitar cuando este seguro de que se calcula bien
        totalDirect = totalDirect + directLight;
        totalIndirect = totalIndirect + indirectLight;
        total = total + 1;
        RGBColor res = emitLight + indirectLight + causticLight + directLight;
        res = HomAmbMedium::applyLight(res, ray, hit);
        res = HomIsoMedium::rayMarch(res, ray, hit, volume, kvNeighbours);
        return res;
    }
    // Didn't hit with anything on the scene
    return scene.backgroundColor;
}

void PhotonMapper::tracePixel(const int px, const int py, const Film &film,
                              const Scene &scene) {
    RGBColor pixelColor(0.0f, 0.0f, 0.0f);
    Vec4 pixelCenter = film.getPixelCenter(px, py);
    for (int p = 0; p < ppp; ++p) {
        float randX = Random::ZeroOne();
        float randY = Random::ZeroOne();
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