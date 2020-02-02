#pragma once

#include <chrono>
#include <future>
#include <memory>
#include "camera/film.h"
#include "camera/homambmedium.h"
#include "camera/progress.h"
#include "photonkdtree.h"
#include "scene/figures.h"
#include "scene/scene.h"

class PhotonEmitter {
    // Stop photons that have whose current energy / original energy
    // ratio is less than CUT_PCT
    const float CUT_PCT = 0.1f;
    // Rays shot (current & max)
    const int totalRays;
    int shotRays;
    friend class PhotonMapper;  // read shotRays
    // Whether to store photon's first hit
    const bool storeDirectLight;
    // Photon maps of different kinds
    bool wantCaustics, wantVolume;
    PhotonKdTreeBuilder photons, caustics, volume;

    void savePhoton(const Photon& photon, const bool isCaustic);
    void traceRay(Ray ray, const Scene& scene, RGBColor flux);
    void traceRays(const Scene& scene, const RGBColor& emission,
                   const MediumPtr& medium,
                   const std::function<void(Vec4&, Vec4&)>& fGetSample);

   public:
    PhotonEmitter(int _maxPhotons, bool _storeDirectLight, int _totalRays)
        : wantCaustics(false),
          wantVolume(false),
          photons(_maxPhotons),
          caustics(0),
          volume(0),
          totalRays(_totalRays),
          shotRays(0),
          storeDirectLight(_storeDirectLight) {}

    void setCaustic(const int max) {
        wantCaustics = true;
        caustics.setMax(max);
    }
    void setVolume(const int max) {
        wantVolume = true;
        volume.setMax(max);
    }
    bool isFull() const {
        return photons.isFull() && (!wantCaustics || caustics.isFull()) &&
               (!wantVolume || volume.isFull());
    }

    void emitPointLights(const Scene& scene, const MediumPtr& medium);
    void emitAreaLight(const Scene& scene, const FigurePtr& light,
                       const RGBColor& emission, const MediumPtr& medium);

    float getTotalRays() const { return totalRays; }
    bool hasDirectLight() const { return storeDirectLight; }
    PhotonKdTree getPhotonsTree() {
        std::cout << "Global map contains " << photons.photons.size()
                  << " photons" << std::endl;
        return photons.build(this->shotRays);
    }
    PhotonKdTree getCausticsTree() {
        std::cout << "Caustic map contains " << caustics.photons.size()
                  << " photons" << std::endl;
        return caustics.build(this->shotRays);
    }
    PhotonKdTree getVolumeTree() {
        std::cout << "Volume map contains " << volume.photons.size()
                  << " photons" << std::endl;
        return volume.build(this->shotRays);
    }

    PPMImage debugPhotonsImage(const Film& film, const bool doPhotons,
                               const bool doCaustics, const bool doVolume);
};