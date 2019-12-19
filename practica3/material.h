#pragma once

class Scene;  // needed forward declaration

// typedef for material pointers
#include <memory>
class BRDF;
typedef std::shared_ptr<BRDF> BRDFPtr;
class Material;
typedef std::shared_ptr<Material> MaterialPtr;

#include <vector>
#include "geometry.h"
#include "random.h"
#include "ray.h"
#include "rayhit.h"
#include "rgbcolor.h"
#include "scene.h"

/// BRDF ///
// All different BRDFs and possible interactions on intersections
// Used by materials which define figures' properties

class BRDF {
   protected:
    BRDF(float _prob) : prob(_prob) {}

   public:
    const float prob;
    virtual bool nextRay(const Vec4 &inDirection, const RayHit &hit,
                         Ray &outRay) = 0;
    virtual RGBColor applyBRDF(const RGBColor &lightIn, const Vec4 &wi,
                               const Vec4 &wo) const = 0;
    virtual RGBColor applyDirect(const RGBColor &lightIn, const RayHit &hit,
                                 const Vec4 &wi, const Vec4 &wo) const = 0;
};

class PhongDiffuse : public BRDF {
   public:
    const RGBColor kd;

    PhongDiffuse(const RGBColor &_kd) : BRDF(_kd.max()), kd(_kd) {}
    bool nextRay(const Vec4 &inDirection, const RayHit &hit,
                 Ray &outRay) override;
    RGBColor applyBRDF(const RGBColor &lightIn, const Vec4 &wi,
                       const Vec4 &wo) const override;
    RGBColor applyDirect(const RGBColor &lightIn, const RayHit &hit,
                         const Vec4 &wi, const Vec4 &wo) const override;
};

class PhongSpecular : public BRDF {
    // temp. variables, need to save info. for applyBRDF
    float tempInCos, tempOutSin;
    Vec4 tempOutDirection;

   public:
    const float alpha;

    PhongSpecular(float _ks, float _alpha) : BRDF(_ks), alpha(_alpha) {}
    bool nextRay(const Vec4 &inDirection, const RayHit &hit,
                 Ray &outRay) override;
    RGBColor applyBRDF(const RGBColor &lightIn, const Vec4 &wi,
                       const Vec4 &wo) const override;
    RGBColor applyDirect(const RGBColor &lightIn, const RayHit &hit,
                         const Vec4 &wi, const Vec4 &wo) const override;
};

class PerfectSpecular : public BRDF {
   public:
    PerfectSpecular(float _ksp) : BRDF(_ksp) {}
    bool nextRay(const Vec4 &inDirection, const RayHit &hit,
                 Ray &outRay) override;
    RGBColor applyBRDF(const RGBColor &lightIn, const Vec4 &wi,
                       const Vec4 &wo) const override;
    RGBColor applyDirect(const RGBColor &lightIn, const RayHit &hit,
                         const Vec4 &wi, const Vec4 &wo) const override;
};

class PerfectRefraction : public BRDF {
    const float mediumRefractiveIndex;

   public:
    PerfectRefraction(float _krp, float _mediumRefractiveIndex)
        : BRDF(_krp), mediumRefractiveIndex(_mediumRefractiveIndex) {}
    bool nextRay(const Vec4 &inDirection, const RayHit &hit,
                 Ray &outRay) override;
    RGBColor applyBRDF(const RGBColor &lightIn, const Vec4 &wi,
                       const Vec4 &wo) const override;
    RGBColor applyDirect(const RGBColor &lightIn, const RayHit &hit,
                         const Vec4 &wi, const Vec4 &wo) const override;
};

class Portal : public BRDF {
    const FigurePortalPtr inPortal, outPortal;

   public:
    Portal(float _kpp, const FigurePortalPtr &_inPortal,
           const FigurePortalPtr &_outPortal)
        : BRDF(_kpp), inPortal(_inPortal), outPortal(_outPortal) {}
    bool nextRay(const Vec4 &inDirection, const RayHit &hit,
                 Ray &outRay) override;
    RGBColor applyBRDF(const RGBColor &lightIn, const Vec4 &wi,
                       const Vec4 &wo) const override;
    RGBColor applyDirect(const RGBColor &lightIn, const RayHit &hit,
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
    MaterialBuilder add(const BRDFPtr &brdf);
    MaterialPtr build() { return ptr; }
};

class Material {
   public:
    const bool emitsLight;
    const RGBColor emission;
    std::vector<float> probs;    // event probability accum list
    std::vector<BRDFPtr> brdfs;  // random <= probs[i] => do event i

   private:
    Material(const RGBColor &_emission)
        : emitsLight(true), emission(_emission), brdfs() {}
    Material() : emitsLight(false), emission(RGBColor::Black), brdfs() {}

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
    BRDFPtr selectEvent();
};