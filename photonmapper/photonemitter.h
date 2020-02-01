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
    // Energy per photon
    const float epp;
    const bool storeDirectLight;
    PhotonKdTreeBuilder photons, caustics, volume;

    void savePhoton(const Photon& photon, const bool isCaustic);
    void traceRay(Ray ray, const Scene& scene, RGBColor flux);
    void traceRays(const int totalPhotons, const RGBColor& emission,
                   const std::function<Vec4()>& funOrigin,
                   const std::function<Vec4(const Vec4&)>& funDirection,
                   const MediumPtr& medium, const Scene& scene);

   public:
    PhotonEmitter(int nPhotons, int nCaustics, int nVolume,
                  bool _storeDirectLight, float _epp = 1.0f)
        : photons(nPhotons),
          caustics(nCaustics),
          volume(nVolume),
          storeDirectLight(_storeDirectLight),
          epp(_epp) {}

    void emitPointLight(const Scene& scene, const PointLight& light);
    void emitAreaLight(const Scene& scene, const FigurePtr& light,
                       const RGBColor& emission, const MediumPtr& medium);

    float energyPerPhoton() const { return epp; }
    bool hasDirectLight() const { return storeDirectLight; }
    PhotonKdTree getPhotonsTree() { return photons.build(); }
    PhotonKdTree getCausticsTree() { return caustics.build(); }

    PPMImage debugPhotonsImage(const Film& film);
};