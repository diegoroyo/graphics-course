#pragma once

#include "photonkdtree.h"
#include "camera/film.h"
#include "scene/figures.h"
#include "scene/scene.h"
#include "scene/figures.h"

class PhotonEmitter {
    // Stop photons that have whose current energy / original energy
    // ratio is less than CUT_PCT
    const float CUT_PCT = 0.1f;
    // Luminance per photon
    const float lpp;
    PhotonKdTreeBuilder photons;

    void traceRay(Ray ray, const Scene &scene, RGBColor flux);

   public:
    PhotonEmitter(float _lpp = 1.0f) : lpp(_lpp) {}

    void emitPointLight(const Scene& scene, const PointLight& light);
    void emitAreaLight(const Scene& scene, const FigurePtr& light);

    PhotonKdTree getPhotonTree() {
        return photons.build();
    }
    
    PPMImage debugPhotonsImage(const Film &film);
};