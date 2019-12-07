#pragma once

// typedef for material pointers
#include <memory>
class BRDF;
typedef std::shared_ptr<BRDF> BRDFPtr;
class Material;
typedef std::shared_ptr<Material> MaterialPtr;

#include <memory>
#include <vector>
#include "geometry.h"
#include "random.h"
#include "rayhit.h"
#include "rgbcolor.h"

/// BRDF ///

class BRDF {
   protected:
    BRDF(float _prob) : prob(_prob) {}

   public:
    const float prob;
    virtual Vec4 nextRay(const Vec4 &inDirection, RayHit &hit) = 0;
    virtual RGBColor applyBRDF(const RGBColor &lightIn) const = 0;
};

class PhongDiffuse : public BRDF {
   public:
    const RGBColor kdProb;  // kd / prob, don't need kd

    PhongDiffuse(const RGBColor &_kd)
        : BRDF(_kd.max()), kdProb(_kd * (1.0f / _kd.max())) {}
    Vec4 nextRay(const Vec4 &inDirection, RayHit &hit) override;
    RGBColor applyBRDF(const RGBColor &lightIn) const override;
};

class PhongSpecular : public BRDF {
    // temp. variables, need to save info. between nextRay and applyBRDF
    float tempAzimIncCos, tempIncl;

   public:
    const float alpha;

    PhongSpecular(float _ks, float _alpha) : BRDF(_ks), alpha(_alpha) {}
    Vec4 nextRay(const Vec4 &inDirection, RayHit &hit) override;
    RGBColor applyBRDF(const RGBColor &lightIn) const override;
};

class PerfectSpecular : public BRDF {
   public:
    PerfectSpecular(float _ksp) : BRDF(_ksp) {}
    Vec4 nextRay(const Vec4 &inDirection, RayHit &hit) override;
    RGBColor applyBRDF(const RGBColor &lightIn) const override;
};

class PerfectRefraction : public BRDF {
    const float mediumRefractiveIndex;

   public:
    PerfectRefraction(float _krp, float _mediumRefractiveIndex)
        : BRDF(_krp), mediumRefractiveIndex(_mediumRefractiveIndex) {}
    Vec4 nextRay(const Vec4 &inDirection, RayHit &hit) override;
    RGBColor applyBRDF(const RGBColor &lightIn) const override;
};

/// Material (BRDF join) ///

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

    class MaterialBuilder;  // need forward declaration
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

   private:
    class MaterialBuilder {
       private:
        MaterialPtr ptr;  // material being built
        float accumProb;  // accumulated probability

        MaterialBuilder(MaterialPtr _ptr) : ptr(_ptr), accumProb(0.0f) {}
        friend MaterialBuilder Material::builder();

       public:
        MaterialBuilder add(const BRDFPtr &brdf) {
            ptr->brdfs.push_back(brdf);
            accumProb += brdf->prob;
            ptr->probs.push_back(accumProb);
            if (accumProb >= 1.0f) {
                std::cout << "Warning: material has event probabilities"
                          << " that sum higher than 1" << std::endl;
            }
            return *this;
        }
        MaterialPtr build() { return ptr; }
    };
};