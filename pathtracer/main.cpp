// Scene descriptions: (may vary as they are changed a lot)
// Scene 0: (may vary) Cornell box (two spheres)
// Scene 1: (may vary) Cornell box (different contents, using model)
// Scene 2: (don't change) UVMaterial properties test (diamond ore wall)
// Scene 3: (don't change) Portal scene
#ifndef SCENE_NUMBER
#define SCENE_NUMBER 0
#endif

#include <iostream>
#include <memory>
#include "camera/camera.h"
#include "camera/medium.h"
#include "io/plymodel.h"
#include "scene/figures.h"
#include "scene/material.h"
#include "scene/scene.h"
#include "scene/uvmaterial.h"
#include "pathtracer.h"

/// test purposes ///

bool out(const Vec4 &pos) {
    // scene 0 cornell box test purposes
    return pos.x < -5.01f || pos.x > 2.01f || pos.y < -2.01f || pos.y > 2.01f ||
           pos.z < -2.01f || pos.z > 2.01f;
}

void addDebugHits(FigurePtrVector &root, Ray ray, const EventPtr &event,
                  const MaterialPtr &material) {
    FigurePtr rootNode = FigurePtr(new Figures::BVNode(root));

    // Debug spheres on ray hit
    RayHit hit;
    FigurePtrVector extra;
    int n = 0;
    while (n < 1000 && rootNode->intersection(ray, hit) && !out(hit.point)) {
        extra.push_back(
            FigurePtr(new Figures::Sphere(material, hit.point, 0.05f)));
        std::cout << "Ray hits at " << hit.point << " w/ normal " << hit.normal
                  << std::endl;
        while (!event->nextRay(ray, hit, ray)) {
            std::cout << "Event's next ray is invalid. Trying again..."
                      << std::endl;
        }
        std::cout << "New ray comes from " << ray.origin << " w/ dir "
                  << ray.direction << std::endl;
        n++;
    }
    if (n != 1000) {
        std::cout << "Ray number " << n << " failed on point " << hit.point
                  << std::endl;
    }
    root.insert(root.end(), extra.begin(), extra.end());
}

/// test purposes ///

int main(int argc, char **argv) {
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

    // Set up camera & scene

#if SCENE_NUMBER == 0 || SCENE_NUMBER == 1
    Vec4 origin(-4.5f, 0.0f, 0.0f, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);
#elif SCENE_NUMBER == 2
    Vec4 origin(-4.5f, 0.0f, 0.0f, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);
#elif SCENE_NUMBER == 3
    Vec4 origin(-2.5f, 0.0f, 0.0f, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 2.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 2.0f, 0.0f);
#endif

    Film film(width, height, origin, forward, up);
    // film.setDepthOfField(0.015f);
    RayTracerPtr pathTracer = RayTracerPtr(new PathTracer(ppp, film));
    Camera camera(film, pathTracer);

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

    // Add elements to scene

#if SCENE_NUMBER == 1
    // load & transform spaceship model, get scene kdtree node
    MediumPtr glass = Medium::create(1.5f);
    UVMaterialPtr texture = UVMaterial::builder()
                                // UVMaterial::builder(512, 512)
                                // .addPhongDiffuse("ply/spaceship_diffuse.ppm")
                                .addPerfectSpecular(0.15f)
                                .addPerfectRefraction(0.8f, glass)
                                .build();

    PLYModel spaceshipModel("ply/spaceship.ply", texture);
    spaceshipModel.transform(Mat4::translation(1.0f, -0.5f, 0.0f) *
                             Mat4::rotationX(-0.7f) * Mat4::rotationY(0.5f) *
                             Mat4::rotationZ(-1.85f) *
                             Mat4::scale(2.0f, 2.0f, 2.0f));
    FigurePtr spaceship = spaceshipModel.getFigure(4);
#endif

    float maxLight = 10000.0f;

#if SCENE_NUMBER == 0 || SCENE_NUMBER == 1

    MaterialPtr whiteLight =
        Material::light(RGBColor(maxLight, maxLight, maxLight));
    MaterialPtr whiteDiffuse =
        Material::builder().add(phongDiffuse(RGBColor::White * 0.95f)).build();
    MaterialPtr whiteMirror = Material::builder()
                                  .add(phongDiffuse(RGBColor::White * 0.1f))
                                  .add(phongSpecular(0.5f, 3.0f))
                                  .add(perfectSpecular(0.35f))
                                  .build();
    MaterialPtr greenDiffuse =
        Material::builder()
            .add(phongDiffuse(RGBColor(0.0f, 0.95f, 0.0f)))
            .build();
    MaterialPtr redDiffuse = Material::builder()
                                 .add(phongDiffuse(RGBColor(0.95f, 0.0f, 0.0f)))
                                 .build();
    MediumPtr glass = Medium::create(1.5f);
    MaterialPtr transparent =
        Material::builder().add(perfectRefraction(0.95f, glass)).build();
    MaterialPtr mirror = Material::builder()
                             .add(phongSpecular(0.5f, 15.0f))
                             .add(phongDiffuse(RGBColor::White * 0.45f))
                             .build();
    MaterialPtr pureBlack = Material::none();

#elif SCENE_NUMBER == 2
    UVMaterialPtr diamondTexture =
        UVMaterial::builder(16, 16)
            .addPhongDiffuse("ply/diamondore_diffuse.ppm")
            .addPhongDiffuse("ply/diamondore_emission.ppm")
            .addPerfectSpecular("ply/diamondore_emission.ppm")
            .build();
    UVMaterialPtr stoneTexture = UVMaterial::builder(16, 16)
                                     .addPhongDiffuse("ply/stone_diffuse.ppm")
                                     .build();
    UVMaterialPtr greenWoolTexture =
        UVMaterial::builder(16, 16)
            .addPhongDiffuse("ply/greenwool_diffuse.ppm")
            .build();
    UVMaterialPtr redWoolTexture =
        UVMaterial::builder(16, 16)
            .addPhongDiffuse("ply/redwool_diffuse.ppm")
            .build();
    MediumPtr glass = Medium::create(1.5f);
    MaterialPtr transparent =
        Material::builder().add(perfectRefraction(0.98f, glass)).build();
    MaterialPtr mirror =
        Material::builder().add(perfectSpecular(0.98f)).build();
#elif SCENE_NUMBER == 3

    MaterialPtr whiteLight =
        Material::light(RGBColor(maxLight, maxLight, maxLight));
    MaterialPtr whiteDiffuse =
        Material::builder().add(phongDiffuse(RGBColor::White * 0.9f)).build();
    MaterialPtr greenDiffuse =
        Material::builder()
            .add(phongDiffuse(RGBColor(0.1f, 0.9f, 0.1f)))
            .build();
    MaterialPtr redDiffuse = Material::builder()
                                 .add(phongDiffuse(RGBColor(0.9f, 0.1f, 0.1f)))
                                 .build();

    // TODO remove
    MaterialPtr yellow = Material::builder()
                             .add(phongDiffuse(RGBColor(0.9f, 0.9f, 0.1f)))
                             .build();
    MaterialPtr purple = Material::builder()
                             .add(phongDiffuse(RGBColor(0.9f, 0.1f, 0.9f)))
                             .build();

    MediumPtr glass = Medium::create(1.5f);
    MaterialPtr transparent =
        Material::builder().add(perfectRefraction(0.9f, glass)).build();
    MaterialPtr mirror = Material::builder()
                             .add(phongSpecular(0.3f, 3.0f))
                             .add(perfectSpecular(0.6f))
                             .build();

    FigurePortalPtr bluePortal = FigurePortalPtr(new Figures::TexturedPlane(
        Vec4(0.0f, 0.0f, -1.0f, 0.0f), -1.99f, false));
    FigurePortalPtr orangePortal = FigurePortalPtr(new Figures::TexturedPlane(
        Vec4(0.0f, 0.0f, 1.0f, 0.0f), -1.99f, false));

    UVMaterialPtr bluePortalTexture =
        UVMaterial::builder(512, 512)
            // .addPhongDiffuse("ply/portal_blue_diffuse2.ppm")
            .addPortal("ply/portal_any_portal2.ppm", bluePortal, orangePortal)
            .build();
    bluePortalTexture->overrideLights("ply/portal_blue_diffuse2.ppm", maxLight,
                                      0.3f);
    bluePortalTexture->override("ply/portal_any_mask2.ppm", nullptr);
    UVMaterialPtr orangePortalTexture =
        UVMaterial::builder(512, 512)
            // .addPhongDiffuse("ply/portal_orange_diffuse2.ppm")
            .addPortal("ply/portal_any_portal2.ppm", orangePortal, bluePortal)
            .build();
    orangePortalTexture->overrideLights("ply/portal_orange_diffuse2.ppm",
                                        maxLight, 0.3f);
    orangePortalTexture->override("ply/portal_any_mask2.ppm", nullptr);

    bluePortal->setUVMaterial(bluePortalTexture, Vec4(1.5f, -2.0f, 1.99f, 1.0f),
                              Vec4(-2.0f, 0.0f, 0.0f, 0.0f),
                              Vec4(0.0f, 4.0f, 0.0f, 0.0f));
    orangePortal->setUVMaterial(
        orangePortalTexture, Vec4(-0.5f, -2.0f, -1.99f, 1.0f),
        Vec4(2.0f, 0.0f, 0.0f, 0.0f), Vec4(0.0f, 4.0f, 0.0f, 0.0f));
#endif

    // build scene to BVH root node
    FigurePtrVector sceneElements = {
#if SCENE_NUMBER == 0
        // Cornell box walls
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f, whiteDiffuse),
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f, whiteDiffuse),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f, whiteDiffuse),
        // plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), -5.0f, pureBlack),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f, redDiffuse),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f, greenDiffuse),
        // Cornell box content
        sphere(mirror, Vec4(1.0f, 0.5f, -1.0f, 1.0f), 0.75f),
        sphere(transparent, Vec4(0.5f, -0.5f, 1.0f, 1.0f), 0.75f)
#elif SCENE_NUMBER == 1
        // Cornell box walls
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f, whiteDiffuse),
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f, whiteDiffuse),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f, whiteDiffuse),
        // plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), -5.0f, pureBlack),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f, redDiffuse),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f, greenDiffuse),
        spaceship,
        sphere(transparent, Vec4(-0.5f, -1.0f, 1.0f, 1.0f), 0.5f)
#elif SCENE_NUMBER == 2
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f, stoneTexture,
            Vec4(0.0f, 2.0f, 0.0f, 1.0f), Vec4(0.0f, 0.0f, 1.0f, 0.0f),
            Vec4(1.0f, 0.0f, 0.0f, 0.0f))),
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f, stoneTexture,
            Vec4(0.0f, -2.0f, 0.0f, 1.0f), Vec4(0.0f, 0.0f, -1.0f, 0.0f),
            Vec4(-1.0f, 0.0f, 0.0f, 0.0f))),
        // diamond ore wall
        FigurePtr(new Figures::TexturedPlane(
            Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f, diamondTexture,
            Vec4(2.0f, 0.0f, 0.0f, 1.0f), Vec4(0.0f, 0.0f, 1.0f, 0.0f),
            Vec4(0.0f, 1.0f, 0.0f, 0.0f))),
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f, greenWoolTexture,
            Vec4(0.0f, 0.0f, 2.0f, 1.0f), Vec4(1.0f, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, 1.0f, 0.0f, 0.0f))),
        FigurePtr(new Figures::TexturedPlane(
            Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f, redWoolTexture,
            Vec4(0.0f, 0.0f, -2.0f, 1.0f), Vec4(-1.0f, 0.0f, 0.0f, 0.0f),
            Vec4(0.0f, -1.0f, 0.0f, 0.0f))),
        sphere(mirror, Vec4(1.0f, -1.05f, -1.0f, 1.0f), 0.75f),
        sphere(transparent, Vec4(0.5f, 0.0f, 1.0f, 1.0f), 0.75f)
#elif SCENE_NUMBER == 3
        // Cornell box walls
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f, whiteDiffuse),
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f, whiteDiffuse),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f, whiteDiffuse),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), -5.0f, whiteDiffuse),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f, greenDiffuse),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f, redDiffuse),
        // Cornell box content
        bluePortal,
        orangePortal,
        sphere(mirror, Vec4(1.0f, 0.0f, 0.0f, 1.0f), 1.0f),
        sphere(mirror, Vec4(0.5f, 0.0f, 2.0f, 1.0f), 0.5f),
        sphere(mirror, Vec4(0.5f, 0.0f, -2.0f, 1.0f), 0.5f)
#endif
    };

    // Debug a random path
    // MaterialPtr debugSphere = Material::light(RGBColor::Cyan * maxLight);
    // EventPtr debugEvent = phongDiffuse(RGBColor::White);
    // addDebugHits(sceneElements, Ray(origin, forward.normalize(),
    // Medium::air),
    //              debugEvent, debugSphere);

    FigurePtr rootNode = FigurePtr(new Figures::BVNode(sceneElements));

    Scene scene(rootNode, RGBColor::Black, maxLight);

    // Add points lights to the scene

#if SCENE_NUMBER == 0 || SCENE_NUMBER == 1
    scene.light(Vec4(0.0f, 1.3f, 0.0f, 1.0f), RGBColor::White * maxLight);
#elif SCENE_NUMBER == 2
    scene.light(Vec4(0.0f, 1.5f, -1.5f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight));
    scene.light(Vec4(0.0f, 1.5f, 1.5f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight));
    scene.light(Vec4(1.5f, 1.5f, 0.0f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight));
    scene.light(Vec4(1.5f, 1.5f, -1.5f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight));
    scene.light(Vec4(1.5f, 1.5f, 1.5f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight));
#elif SCENE_NUMBER == 3
    // scene.light(Vec4(-1.0f, 1.4f, -1.8f, 1.0f),
    //             RGBColor(maxLight, maxLight, maxLight));
    // scene.light(Vec4(-1.0f, 1.4f, 1.8f, 1.0f),
    //             RGBColor(maxLight, maxLight, maxLight));
    scene.light(Vec4(1.0f, 1.4f, 0.0f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight));
#endif

#undef plane
#undef sphere

#undef phongDiffuse
#undef phongSpecular
#undef perfectSpecular
#undef perfectRefraction

    // Generate render using argument options and save as PPM
    camera.tracePixels(scene);
    pathTracer->result().writeFile(filenameOut.c_str());

    return 0;
}