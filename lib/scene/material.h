#pragma once

// typedef for material pointers
#include <memory>
class Event;
typedef std::shared_ptr<Event> EventPtr;
class Material;
typedef std::shared_ptr<Material> MaterialPtr;

#include <vector>
#include "camera/medium.h"
#include "camera/ray.h"
#include "camera/rayhit.h"
#include "math/geometry.h"
#include "math/random.h"
#include "math/rgbcolor.h"
#include "scene/figures.h"
#include "scene/scene.h"

/// Event ///
// All different events and possible interactions on intersections
// Used by materials which define figures' properties

class Event {
   protected:
    Event(float _prob, bool _isDelta) : prob(_prob), isDelta(_isDelta) {}

   public:
    const bool isDelta;
    const float prob;
    virtual bool nextRay(const Ray &inRay, const RayHit &hit, Ray &outRay) = 0;
    virtual RGBColor applyMonteCarlo(const RGBColor &lightIn, const RayHit &hit,
                                     const Vec4 &wi, const Vec4 &wo) const = 0;
    virtual RGBColor applyNextEvent(const RGBColor &lightIn, const RayHit &hit,
                                    const Vec4 &wi, const Vec4 &wo) const = 0;
};

class PhongDiffuse : public Event {
   public:
    const RGBColor kd;

    PhongDiffuse(const RGBColor &_kd) : Event(_kd.max(), false), kd(_kd) {}
    bool nextRay(const Ray &inRay, const RayHit &hit, Ray &outRay) override;
    RGBColor applyMonteCarlo(const RGBColor &lightIn, const RayHit &hit,
                             const Vec4 &wi, const Vec4 &wo) const override;
    RGBColor applyNextEvent(const RGBColor &lightIn, const RayHit &hit,
                            const Vec4 &wi, const Vec4 &wo) const override;
};

class PhongSpecular : public Event {
   public:
    const float alpha;

    PhongSpecular(float _ks, float _alpha) : Event(_ks, false), alpha(_alpha) {}
    bool nextRay(const Ray &inRay, const RayHit &hit, Ray &outRay) override;
    RGBColor applyMonteCarlo(const RGBColor &lightIn, const RayHit &hit,
                             const Vec4 &wi, const Vec4 &wo) const override;
    RGBColor applyNextEvent(const RGBColor &lightIn, const RayHit &hit,
                            const Vec4 &wi, const Vec4 &wo) const override;
};

class PerfectSpecular : public Event {
   public:
    PerfectSpecular(float _ksp) : Event(_ksp, true) {}
    bool nextRay(const Ray &inRay, const RayHit &hit, Ray &outRay) override;
    RGBColor applyMonteCarlo(const RGBColor &lightIn, const RayHit &hit,
                             const Vec4 &wi, const Vec4 &wo) const override;
    RGBColor applyNextEvent(const RGBColor &lightIn, const RayHit &hit,
                            const Vec4 &wi, const Vec4 &wo) const override;
};

class PerfectRefraction : public Event {
    const MediumPtr medium;
    static bool isFresnelDisabled;

   public:
    PerfectRefraction(float _krp, const MediumPtr &_medium)
        : Event(_krp, true), medium(_medium) {}
    bool nextRay(const Ray &inRay, const RayHit &hit, Ray &outRay) override;
    RGBColor applyMonteCarlo(const RGBColor &lightIn, const RayHit &hit,
                             const Vec4 &wi, const Vec4 &wo) const override;
    RGBColor applyNextEvent(const RGBColor &lightIn, const RayHit &hit,
                            const Vec4 &wi, const Vec4 &wo) const override;

    // Disable fresnel events (critical angle still applies)
    static void disableFresnel() {
        PerfectRefraction::isFresnelDisabled = true;
    }
};

class Portal : public Event {
    const FigurePortalPtr inPortal, outPortal;

   public:
    Portal(float _kpp, const FigurePortalPtr &_inPortal,
           const FigurePortalPtr &_outPortal)
        : Event(_kpp, true), inPortal(_inPortal), outPortal(_outPortal) {}
    bool nextRay(const Ray &inRay, const RayHit &hit, Ray &outRay) override;
    RGBColor applyMonteCarlo(const RGBColor &lightIn, const RayHit &hit,
                             const Vec4 &wi, const Vec4 &wo) const override;
    RGBColor applyNextEvent(const RGBColor &lightIn, const RayHit &hit,
                            const Vec4 &wi, const Vec4 &wo) const override;
};

/// Material ///
// Set of various BRDFs, where each has its own probability

// helper class for Material
class MaterialBuilder {
   private:
    MaterialPtr ptr;  // material being built
    float accumProb;  // accumulated probability

    MaterialBuilder(MaterialPtr _ptr) : ptr(_ptr), accumProb(0.0f) {}
    friend class Material;

   public:
    MaterialBuilder add(const EventPtr &event);
    MaterialPtr build() { return ptr; }
};

class Material {
   public:
    const bool emitsLight;
    const RGBColor emission;
    std::vector<float> probs;      // event probability accum list
    std::vector<EventPtr> events;  // random <= probs[i] => do event i

   private:
    Material(const RGBColor &_emission)
        : emitsLight(true), emission(_emission), events() {}
    Material() : emitsLight(false), emission(RGBColor::Black), events() {}

   public:
    // Constructor for light emitters
    static MaterialPtr light(const RGBColor &_emission) {
        return MaterialPtr(new Material(_emission));
    }

    // Constructor for normal materials
    static MaterialBuilder builder() {
        return MaterialBuilder(MaterialPtr(new Material()));
    }

    // If the item doesn't need a material
    static MaterialPtr none() { return MaterialPtr(new Material()); }

    // Roussian roulette event selector
    EventPtr selectEvent();

    // Get first delta material
    EventPtr getFirstDelta() const;

    // Evaluate whole BRDF/material
    RGBColor evaluate(const RGBColor &lightIn, const RayHit &hit,
                      const Vec4 &wi, const Vec4 &wo) const;
};