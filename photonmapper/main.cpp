//
// Photon mapper - Informática gráfica 2019-20
// Diego Royo (740388@unizar.es)
//
// There are 4 predefined scenes in the code.
// Some of them may require extra files (textures, models, etc.)
// To change scene, modify SCENE_NUMBER's value
//

// Scene descriptions:
// Scene 0: Cornell box w/ 3 diffuse spheres (scene 2 in class' code)
// Scene 1: Cornell box w/ delta BSDF spheres (scene 1 in class' code)
// Scene 2: Cornell box w/ concentrical spheres (scene 4 in class' code)
// Scene 3: Cornell box w/ phong materials
#ifndef SCENE_NUMBER
#define SCENE_NUMBER 0
#endif

#include "camera/camera.h"
#include "camera/film.h"
#include "camera/homambmedium.h"
#include "filter.h"
#include "homisomedium.h"
#include "math/geometry.h"
#include "photonemitter.h"
#include "photonmapper.h"
#include "scene/light.h"

/// test purposes ///

void testKdTreeKNN() {
    // Simple KdTree KNearestNeighbours search
    Vec4 point(-5.0f, 5.0f, 0.0f, 0.0f);  // search knn of point

    // Create KdTree and show distances for validation
    PhotonKdTreeBuilder builder;
    std::vector<Photon> photonsSource = {
        Photon(Vec4(2.0f, 3.0f, 0.0f, 0.0f), Vec4(0.0f), RGBColor::White),
        Photon(Vec4(5.0f, 4.0f, 0.0f, 0.0f), Vec4(0.0f), RGBColor::White),
        Photon(Vec4(9.0f, 6.0f, 0.0f, 0.0f), Vec4(0.0f), RGBColor::White),
        Photon(Vec4(4.0f, 7.0f, 0.0f, 0.0f), Vec4(0.0f), RGBColor::White),
        Photon(Vec4(8.0f, 1.0f, 0.0f, 0.0f), Vec4(0.0f), RGBColor::White),
        Photon(Vec4(7.0f, 2.0f, 0.0f, 0.0f), Vec4(0.0f), RGBColor::White)};
    for (const Photon& photon : photonsSource) {
        std::cout << photon.point << ": " << (photon.point - point).module()
                  << std::endl;
        builder.add(photon);
    }
    PhotonKdTree tree = builder.build();

    // std::cout << tree << std::endl; // show tree

    // Perform kNN search and show results
    std::vector<const Photon*> photonsNN;
    tree.searchNN(photonsNN, point, 3);
    for (const Photon* photon : photonsNN) {
        std::cout << photon->point << std::endl;
    }
}

/// test purposes ///

int main(int argc, char** argv) {
    if (argc < 9) {
        std::cerr << "Usage: " << argv[0]
                  << " -w <width> -h <height> -p <ppp> -o <out_ppm>"
                  << std::endl;
        std::cerr << std::endl;
        std::cerr << "-w Output image width" << std::endl;
        std::cerr << "-h Output image height" << std::endl;
        std::cerr << "-p Paths per pixel" << std::endl;
        std::cerr << "-o Output file (PPM format)" << std::endl;
        return 1;
    }

    // Read options
    int width, height, ppp;
    std::string filenameOut;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0) {
            width = std::stoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-h") == 0) {
            height = std::stoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-p") == 0) {
            ppp = std::stoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-o") == 0) {
            filenameOut = argv[i + 1];
            i++;
        }
    }

    /// Paritcipative media configuration ///
    // Medium::air = HomAmbMedium::create(  // homogeneous ambient
    //     1.0f, 0.6f, 0.4f,       // refractive index, extinction, scattering
    //     RGBColor::White * 20.0f  // in-scattering constant
    // );
    // Medium::air = HomIsoMedium::create(  // homogeneous isotropic
    //     1.0f, 0.3f, 0.2f,                // refractive index,
    //     0.1f                             // delta_d for ray marching
    // );
    /// Participative media configuration ///

    /// Emitting configuration ///
    int photonsGlobal = 10000;
    bool useCausticMap = true;
    int photonsCaustic = 10000;
    bool useVolumeMap = true;
    int photonsVolume = 20000;
    int numRays = 5000;
    bool storeDirectLight = false;
    /// Emiting configuration ///

    /// Estimating configuration ///
    bool debugGlobal = false;  // overrides photon mapping
    bool debugCaustic = false;
    bool debugVolume = false;
    int kGlobal = 70;  // kNN search for maps
    int kCaustic = 30;
    int kVolume = 100;
    FilterPtr filter(new Filter());
    // FilterPtr filter(new ConeFilter());
    /// Estimating configuration ///

#if SCENE_NUMBER == 0 || SCENE_NUMBER == 1 || SCENE_NUMBER == 2 || \
    SCENE_NUMBER == 3
    Vec4 origin(-4.5f, 0.0f, 0.0, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);
#endif

    // Configure scene
    Film film(width, height, origin, forward, up);
    PhotonEmitter emitter(photonsGlobal, storeDirectLight, numRays);
    if (useCausticMap) {
        emitter.setCaustic(photonsCaustic);
    }
    if (useVolumeMap) {
        emitter.setVolume(photonsVolume);
    }
    // Maximum light value (use this instead of constant numbers)
    float maxLight = 5000.0f;
    // Disable fresnel events for russian roulette (less noise for low ppp)
    PerfectRefraction::disableFresnel();

// shortcuts for getting figure pointers
#define plane(normal, dist, material) \
    FigurePtr(new Figures::FlatPlane(normal, dist, material))
#define sphere(material, pos, radius) \
    FigurePtr(new Figures::Sphere(material, pos, radius))

// shortcuts for materials
#define phongDiffuse(kd) EventPtr(new PhongDiffuse(kd))
#define phongSpecular(ks, alpha) EventPtr(new PhongSpecular(ks, alpha))
#define perfectSpecular(ksp) EventPtr(new PerfectSpecular(ksp))
#define perfectRefraction(krp, medium) \
    EventPtr(new PerfectRefraction(krp, medium))

    // Models (maybe with scattering events)
    MediumPtr glass = Medium::create(1.5f);
    // MediumPtr glass = HomAmbMedium::create(  // homogeneous ambient
    //     1.5f, 0.4f, 0.2f,        // refractive index, extinction, scattering
    //     RGBColor::Cyan * 100.0f  // in-scattering constant
    // );
    UVMaterialPtr transparentTexture =
        UVMaterial::builder(1, 1).addPerfectRefraction(0.9f, glass).build();
    PLYModel teapotModel("ply/teapot.ply", transparentTexture);
    teapotModel.transform(
        Mat4::translation(0.0f, -0.5f, 0.0f) * Mat4::rotationX(M_PI_2 * -1.0f) *
        Mat4::rotationZ(M_PI_2) * Mat4::scale(0.4f, 0.4f, 0.4f));
    FigurePtr teapot = teapotModel.getFigure(8);

    // Materials for walls
    MaterialPtr whiteLight =
        Material::light(RGBColor(maxLight, maxLight, maxLight));
    UVMaterialPtr whiteAreaLight = UVMaterial::fill(1, 1, whiteLight);
    MaterialPtr whiteDiffuse =
        Material::builder().add(phongDiffuse(RGBColor::White * 0.85f)).build();
    MaterialPtr greenDiffuse =
        Material::builder()
            .add(phongDiffuse(RGBColor(0.085f, 0.85f, 0.085f)))
            .build();
    MaterialPtr redDiffuse =
        Material::builder()
            .add(phongDiffuse(RGBColor(0.85f, 0.085f, 0.085f)))
            .build();
    MaterialPtr orangeDiffuse =
        Material::builder()
            .add(phongDiffuse(RGBColor(0.85f, 0.6f, 0.02f)))
            .build();
    MaterialPtr magentaPhong =
        Material::builder()
            .add(phongSpecular(0.3f, 10.0f))
            .add(phongDiffuse(RGBColor(0.6f, 0.05f, 0.6f)))
            .build();
    MaterialPtr yellowPhong =
        Material::builder()
            .add(phongSpecular(0.3f, 100.0f))
            .add(phongDiffuse(RGBColor(0.6f, 0.6f, 0.05f)))
            .build();
    MaterialPtr mirror =
        Material::builder().add(perfectSpecular(0.85f)).build();

    // Trasparent medium (maybe with events, see above)
    MaterialPtr transparent =
        Material::builder().add(perfectRefraction(0.85f, glass)).build();

    // Rectangle area light on top of cornell box
    FigurePtr light = FigurePtr(new Figures::TexturedPlane(
        Vec4(0.0f, -1.0f, 0.0f, 0.0f), -1.99f, whiteAreaLight,
        Vec4(0.5f, 1.99f, -0.5f, 0.0f), Vec4(0.0f, 0.0f, 1.0f, 0.0f),
        Vec4(-1.0f, 0.0f, 0.0f, 0.0f), false));

    // Build scene to BVH root node
    FigurePtrVector sceneElements = {
        // Cornell box walls
        // light,
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f, whiteDiffuse),
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f, whiteDiffuse),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f, whiteDiffuse),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f, greenDiffuse),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f, redDiffuse),
    // Cornell box content
    // teapot,
#if SCENE_NUMBER == 0
        sphere(redDiffuse, Vec4(-1.0f, -1.4f, -1.0f, 1.0f), 0.6f),
        sphere(whiteDiffuse, Vec4(1.0f, -1.4f, 0.0f, 1.0f), 0.6f),
        sphere(whiteDiffuse, Vec4(0.0f, -1.4f, 1.0f, 1.0f), 0.6f)
#elif SCENE_NUMBER == 1
        sphere(mirror, Vec4(0.0f, -0.8f, -1.0f, 1.0f), 0.6f),
        sphere(transparent, Vec4(0.0f, -1.25f, 1.0f, 1.0f), 0.6f)
#elif SCENE_NUMBER == 2
        sphere(orangeDiffuse, Vec4(0.0f, -0.1f, 0.0f, 1.0f), 0.5f),
        sphere(transparent, Vec4(0.0f, -0.1f, 0.0f, 1.0f), 1.1f)
#elif SCENE_NUMBER == 3
        sphere(yellowPhong, Vec4(0.0f, -1.2f, 1.0f, 1.0f), 0.8f),
        sphere(magentaPhong, Vec4(1.0f, -1.4f, -1.0f, 1.0f), 0.6f)
#endif
    };

    FigurePtr rootNode = FigurePtr(new Figures::BVNode(sceneElements));
    Scene scene(rootNode, RGBColor::Black, maxLight);

#undef plane
#undef sphere

#undef phongDiffuse
#undef phongSpecular
#undef perfectSpecular
#undef perfectRefraction

#if SCENE_NUMBER == 0 || SCENE_NUMBER == 1 || SCENE_NUMBER == 2 || \
    SCENE_NUMBER == 3
    // Add points lights to the scene
    scene.light(Vec4(0.0f, 1.9f, 0.0f, 0.0f), RGBColor::White * maxLight);
    emitter.emitPointLights(scene, Medium::air);
#endif
    // emitter.emitAreaLight(scene, light, RGBColor::White * maxLight,
    //                       Medium::air);

    if (debugGlobal || debugCaustic || debugVolume) {
        // Project photons to viewport to view stored photons
        PPMImage debug = emitter.debugPhotonsImage(film, debugGlobal,
                                                   debugCaustic, debugVolume);
        debug.writeFile(filenameOut.c_str());
    } else {
        // Radiance estimate step, save image
        RayTracerPtr mapper = RayTracerPtr(new PhotonMapper(
            ppp, film, emitter, kGlobal, kCaustic, kVolume, filter));
        Camera camera(film, mapper);
        camera.tracePixels(scene);
        camera.storeResult(filenameOut);
    }

    return 0;
}