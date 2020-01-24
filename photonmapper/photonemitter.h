#pragma once

#include <chrono>
#include <future>
#include "camera/film.h"
#include "camera/progress.h"
#include "photonkdtree.h"
#include "scene/figures.h"
#include "scene/scene.h"

class PhotonEmitter {
    // Stop photons that have whose current energy / original energy
    // ratio is less than CUT_PCT
    const float CUT_PCT = 0.1f;
    // Luminance per photon
    const float lpp;
    PhotonKdTreeBuilder photons;

    void traceRay(Ray ray, const Scene& scene, RGBColor flux);
    void traceRays(const int totalPhotons, const RGBColor& emission,
                   const std::function<Vec4()>& funOrigin,
                   const std::function<Vec4(const Vec4&)>& funDirection,
                   const MediumPtr& medium, const Scene& scene);

   public:
    PhotonEmitter(float _lpp = 1.0f) : lpp(_lpp) {}

    void emitPointLight(const Scene& scene, const PointLight& light);
    void emitAreaLight(const Scene& scene, const FigurePtr& light,
                       const RGBColor& emission, const MediumPtr& medium);

    PhotonKdTree getPhotonTree() { return photons.build(); }

    PPMImage debugPhotonsImage(const Film& film);
};