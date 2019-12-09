// Scene descriptions:
// Scene 0: Cornell box
// Scene 1: PLY model with empty background
#ifndef SCENE_NUMBER
#define SCENE_NUMBER 1
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

#if SCENE_NUMBER == 0
    Vec4 origin(-4.5f, 0.0f, 0.0f, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);
#elif SCENE_NUMBER == 1
    Vec4 origin(-4.5f, 0.0f, 0.0f, 1.0f), forward(2.0f, 0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);
#endif

    // Camera camera(origin, forward, up, right);
    Camera camera(origin, forward, up, width / (float)height);

// shortcuts for getting figure pointers
#define plane(material, normal, dist) \
    FigurePtr(new Figures::Plane(material, normal, dist))
#define sphere(material, pos, radius) \
    FigurePtr(new Figures::Sphere(material, pos, radius))
#define box(material, bb0, bb1) FigurePtr(new Figures::Box(material, bb0, bb1))

// shortcuts for materials
#define phongDiffuse(kd) BRDFPtr(new PhongDiffuse(kd))
#define phongSpecular(ks, alpha) BRDFPtr(new PhongSpecular(ks, alpha))
#define perfectSpecular(ksp) BRDFPtr(new PerfectSpecular(ksp))
#define perfectRefraction(krp, index) BRDFPtr(new PerfectRefraction(krp, index))

    // Add elements to scene

#if SCENE_NUMBER == 1
    // load & transform spaceship model, get scene kdtree node
    UVMaterialPtr texture =
        UVMaterial::builder()
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
    MaterialPtr ballMaterial = Material::builder()
                                   // .add(phongDiffuse(RGBColor(0.05f, 0.05f,
                                   // 0.5f))) .add(phongSpecular(0.29f, 5.0f))
                                   .add(perfectSpecular(0.05f))
                                   .add(perfectRefraction(0.9f, 1.5f))
                                   .build();
    MaterialPtr pureBlack = Material::none();

    // build scene to BVH root node
    FigurePtrVector sceneElements = {
#if SCENE_NUMBER == 0 || SCENE_NUMBER == 1
        // Cornell box walls
        plane(whiteDiffuse, Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f),
        plane(whiteLight, Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f),
        plane(whiteDiffuse, Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f),
        // plane(pureBlack, Vec4(1.0f, 0.0f, 0.0f, 0.0f), -5.0f),
        plane(redDiffuse, Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f),
        plane(greenDiffuse, Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f),
    // Cornell box content
#if SCENE_NUMBER == 0
        sphere(ballMaterial, Vec4(1.0f, -1.0f, -1.0f, 1.0f), 0.75f),
        sphere(ballMaterial, Vec4(0.5f, 0.0f, 1.0f, 1.0f), 0.75f)
#elif SCENE_NUMBER == 1
        spaceship,
        sphere(ballMaterial, Vec4(-0.5f, -1.0f, 1.0f, 1.0f), 0.5f)
#endif
#endif
    };

    Scene scene(FigurePtr(new Figures::BVNode(sceneElements)), maxLight);

    // Add points lights to the scene

#if SCENE_NUMBER == 0
    scene.light(Vec4(1.0f, 1.6f, 0.0f, 1.0f),
                RGBColor(maxLight, maxLight, maxLight));
#elif SCENE_NUMBER == 1
    // scene.light(Vec4(1.0f, 1.3f, 0.0f, 1.0f),
    //             RGBColor(maxLight, maxLight, maxLight));
#endif

#undef plane
#undef sphere
#undef box

#undef phongDiffuse
#undef phongSpecular
#undef perfectSpecular
#undef perfectRefraction

    // Generate render using argument options and save as PPM

    PPMImage render = camera.render(width, height, ppp, scene, RGBColor::Black);
    render.writeFile(filenameOut.c_str());

    return 0;
}