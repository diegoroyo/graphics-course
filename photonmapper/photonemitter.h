#pragma once

#include "photonkdtree.h"
#include "camera/film.h"
#include "scene/figures.h"
#include "scene/scene.h"
#include "scene/figures.h"

class PhotonEmitter {
    PhotonKdTreeBuilder photons;
    const int ppa;

    void traceRay(Ray ray, const Scene &scene, RGBColor flux);

   public:
    // photons per area
    PhotonEmitter(int _ppa) : ppa(_ppa) {}

    void emitPointLight(const Scene& scene, const PointLight& light);
    void emitAreaLight(const Scene& scene, const FigurePtr& light);

    PhotonKdTree getPhotonTree() {
        return photons.build();
    }
    
    PPMImage debugPhotonsImage(const Film &film);
};