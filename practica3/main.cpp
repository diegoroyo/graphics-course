// Scene descriptions: (may vary as they are changed a lot)
// Scene 0: (may vary) Cornell box (two spheres)
// Scene 1: (may vary) Cornell box (different contents, using model)
// Scene 2: (don't change) UVMaterial properties test (diamond ore wall)
// Scene 3: (don't change) Portal scene
#ifndef SCENE_NUMBER
#define SCENE_NUMBER 3
#endif

#include <iostream>
#include <memory>
#include "camera.h"
#include "figures.h"
#include "material.h"
#include "plymodel.h"
#include "scene.h"
#include "uvmaterial.h"

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

    // Set up camera & scene

#if SCENE_NUMBER == 0 || SCENE_NUMBER == 1
    Vec4 origin(-4.5f, 0.0f, 0.0f, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);
#elif SCENE_NUMBER == 2
    Vec4 origin(-4.5f, 0.0f, 0.0f, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);
#elif SCENE_NUMBER == 3
    Vec4 origin(-4.5f, 0.0f, 0.0f, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);
#endif

    // Camera camera(origin, forward, up, right);
    Camera camera(origin, forward, up, width / (float)height);

// shortcuts for getting figure pointers
#define plane(normal, dist, material) \
    FigurePtr(new Figures::FlatPlane(normal, dist, material))
#define sphere(material, pos, radius) \
    FigurePtr(new Figures::Sphere(material, pos, radius))

// shortcuts for materials
#define phongDiffuse(kd) BRDFPtr(new PhongDiffuse(kd))
#define phongSpecular(ks, alpha) BRDFPtr(new PhongSpecular(ks, alpha))
#define perfectSpecular(ksp) BRDFPtr(new PerfectSpecular(ksp))
#define perfectRefraction(krp, index) BRDFPtr(new PerfectRefraction(krp, index))

    // Add elements to scene

#if SCENE_NUMBER == 1
    // load & transform spaceship model, get scene kdtree node
    UVMaterialPtr texture = UVMaterial::builder()
                                // UVMaterial::builder(512, 512)
                                // .addPhongDiffuse("ply/spaceship_diffuse.ppm")
                                .addPerfectSpecular(0.15f)
                                .addPerfectRefraction(0.8f, 1.5f)
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
        Material::builder().add(phongDiffuse(RGBColor::White * 0.5f)).build();
    MaterialPtr whiteMirror = Material::builder()
                                  .add(phongDiffuse(RGBColor::White * 0.1f))
                                  .add(phongSpecular(0.5f, 3.0f))
                                  .add(perfectSpecular(0.35f))
                                  .build();
    MaterialPtr greenDiffuse =
        Material::builder()
            .add(phongDiffuse(RGBColor(0.02f, 0.5f, 0.02f)))
            .build();
    MaterialPtr redDiffuse =
        Material::builder()
            .add(phongDiffuse(RGBColor(0.5f, 0.02f, 0.02f)))
            .build();
    MaterialPtr transparent = Material::builder()
                                  .add(perfectSpecular(0.1f))
                                  .add(perfectRefraction(0.85f, 1.5f))
                                  .build();
    MaterialPtr mirror = Material::builder()
                             .add(phongSpecular(0.35f, 3.0f))
                             .add(perfectSpecular(0.6f))
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
    MaterialPtr transparent =
        Material::builder().add(perfectRefraction(0.98f, 3.0f)).build();
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

    MaterialPtr transparent = Material::builder()
                                  .add(perfectRefraction(0.9f, 1.5f))
                                  .build();
    MaterialPtr mirror = Material::builder()
                             .add(phongSpecular(0.3f, 3.0f))
                             .add(perfectSpecular(0.6f))
                             .build();

    FigurePortalPtr bluePortal = FigurePortalPtr(new Figures::TexturedPlane(
        Vec4(-1.0f, 0.0f, 0.0f, 0.0f), -1.99f, false));
    FigurePortalPtr orangePortal = FigurePortalPtr(new Figures::TexturedPlane(
        Vec4(1.0f, 0.0f, 0.0f, 0.0f), -4.99f, false));

    UVMaterialPtr bluePortalTexture =
        UVMaterial::builder(512, 512)
            // .addPhongDiffuse("ply/portal_blue_diffuse2.ppm")
            .addPortal("ply/portal_any_portal2.ppm", bluePortal, orangePortal)
            .build();
    bluePortalTexture->overrideLights("ply/portal_blue_diffuse2.ppm", maxLight, 0.3f);
    bluePortalTexture->override("ply/portal_any_mask2.ppm", nullptr);
    UVMaterialPtr orangePortalTexture =
        UVMaterial::builder(512, 512)
            // .addPhongDiffuse("ply/portal_orange_diffuse2.ppm")
            .addPortal("ply/portal_any_portal2.ppm", orangePortal, bluePortal)
            .build();
    orangePortalTexture->overrideLights("ply/portal_orange_diffuse2.ppm", maxLight, 0.3f);
    orangePortalTexture->override("ply/portal_any_mask2.ppm", nullptr);

    bluePortal->setUVMaterial(
        bluePortalTexture, Vec4(1.99f, -2.0f, -1.5f, 1.0f),
        Vec4(0.0f, 0.0f, 3.0f, 0.0f), Vec4(0.0f, 4.0f, 0.0f, 0.0f));
    orangePortal->setUVMaterial(
        orangePortalTexture, Vec4(-4.99f, -2.0f, 1.5f, 1.0f),
        Vec4(0.0f, 0.0f, -3.0f, 0.0f), Vec4(0.0f, 4.0f, 0.0f, 0.0f));
#endif

    // build scene to BVH root node
    FigurePtrVector sceneElements = {
#if SCENE_NUMBER == 0
        // Cornell box walls
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f, whiteDiffuse),
        plane(Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f, redDiffuse),
        plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f, whiteDiffuse),
        // plane(Vec4(1.0f, 0.0f, 0.0f, 0.0f), -5.0f, pureBlack),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f, redDiffuse),
        plane(Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f, greenDiffuse),
        // Cornell box content
        sphere(mirror, Vec4(1.0f, -1.05f, -1.0f, 1.0f), 0.75f),
        sphere(transparent, Vec4(0.5f, 0.0f, 1.0f, 1.0f), 0.75f)
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
        sphere(whiteDiffuse, Vec4(0.0f, -1.0f, 0.0f, 1.0f), 0.25f),
        sphere(mirror, Vec4(1.0f, -1.2f, -1.2f, 1.0f), 0.65f),
        sphere(transparent, Vec4(0.5f, 0.0f, 1.0f, 1.0f), 0.45f)
#endif
    };

    Scene scene(FigurePtr(new Figures::BVNode(sceneElements)), maxLight);

    // Add points lights to the scene

#if SCENE_NUMBER == 0 || SCENE_NUMBER == 1
    scene.light(Vec4(-0.5f, 1.4f, -1.8f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight));
    scene.light(Vec4(-0.5f, 1.4f, 1.8f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight));
#elif SCENE_NUMBER == 2
    scene.light(Vec4(-1.0f, 1.4f, -1.8f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight));
    scene.light(Vec4(-1.0f, 1.4f, 1.8f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight));
    scene.light(Vec4(1.0f, 1.4f, 0.0f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight));
#elif SCENE_NUMBER == 3
    scene.light(Vec4(-1.0f, 1.4f, -1.8f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight) * 0.75f);
    scene.light(Vec4(-1.0f, 1.4f, 1.8f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight) * 0.75f);
    scene.light(Vec4(1.0f, 1.4f, 0.0f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight) * 0.75f);
#endif

#undef plane
#undef sphere

#undef phongDiffuse
#undef phongSpecular
#undef perfectSpecular
#undef perfectRefraction

    // Generate render using argument options and save as PPM

    PPMImage render = camera.render(width, height, ppp, scene, RGBColor::Black);
    render.writeFile(filenameOut.c_str());

    return 0;
}