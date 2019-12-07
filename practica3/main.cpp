// Scene descriptions:
// Scene 0: Cornell box
// Scene 1: PLY model with empty background
#ifndef SCENE_NUMBER
#define SCENE_NUMBER 0
#endif

#define DEBUG_PATH

#include <iostream>
#include <memory>
#include "camera.h"
#include "figures.h"
#include "material.h"
#include "plymodel.h"

int main(int argc, char** argv) {
    if (argc < 9) {
        std::cerr << "Usage: " << argv[0]
                  << " -w <width> -h <height> -r <rpp> -o <out_ppm>"
                  << std::endl;
        std::cerr << std::endl;
        std::cerr << "-w Output image width" << std::endl;
        std::cerr << "-h Output image height" << std::endl;
        std::cerr << "-r Rays per pixel" << std::endl;
        std::cerr << "-o Output file (PPM format)" << std::endl;
        return 1;
    }

    // Read options
    int width, height, rpp;
    std::string filenameOut;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0) {
            width = std::stoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-h") == 0) {
            height = std::stoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-r") == 0) {
            rpp = std::stoi(argv[i + 1]);
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
    Vec4 origin(1.2f, -1.2f, 0.0f, 1.0f), forward(-1.0f, 1.0f, 0.0f, 0.0f),
        up(0.0f, 0.0f, 1.0f, 0.0f), right(0.707f, 0.707f, 0.0f, 0.0f);
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

#if SCENE_NUMBER == 1
    // load & transform spaceship model, get scene kdtree node
    PLYModel spaceshipModel("ply/spaceship");
    spaceshipModel.transform(Mat4::rotationX(1.2f) * Mat4::rotationY(-1.0f) *
                             Mat4::rotationZ(0.9f));
    FigurePtr spaceship = spaceshipModel.getFigure(4);
#endif

    MaterialPtr whiteLight =
        Material::light(RGBColor(10000.0f, 10000.0f, 10000.0f));
    MaterialPtr whiteDiffuse =
        Material::builder().add(phongDiffuse(RGBColor::White * 0.9f)).build();
    MaterialPtr greenDiffuse =
        Material::builder()
            .add(phongDiffuse(RGBColor(0.1f, 0.9f, 0.1f)))
            .build();
    MaterialPtr redDiffuse = Material::builder()
                                 .add(phongDiffuse(RGBColor(0.9f, 0.1f, 0.1f)))
                                 .build();
    MaterialPtr ballMaterial =
        Material::builder().add(perfectSpecular(0.99f)).build();
    MaterialPtr pureBlack = Material::none();

    // build scene to rootNode
    FigurePtrVector scene = {
#if SCENE_NUMBER == 0
        // Cornell box walls
        plane(whiteDiffuse, Vec4(0.0f, 1.0f, 0.0f, 0.0f), -2.0f),
        plane(whiteDiffuse, Vec4(0.0f, 1.0f, 0.0f, 0.0f), 2.0f),
        plane(ballMaterial, Vec4(1.0f, 0.0f, 0.0f, 0.0f), 2.0f),
        // plane(pureBlack, Vec4(1.0f, 0.0f, 0.0f, 0.0f), -5.0f),
        plane(redDiffuse, Vec4(0.0f, 0.0f, 1.0f, 0.0f), 2.0f),
        plane(greenDiffuse, Vec4(0.0f, 0.0f, 1.0f, 0.0f), -2.0f),
        // Cornell box content
        sphere(whiteLight, Vec4(1.25f, -1.25f, -1.0f, 1.0f), 0.75f),
        sphere(whiteDiffuse, Vec4(0.75f, -1.25f, 1.0f, 1.0f), 0.75f),
    // sphere(whiteLight, Vec4(0.5f, 2.0f, 0.0f, 1.0f), 1.0f)
#elif SCENE_NUMBER == 1
        spaceship
#endif
    };
    FigurePtr rootNode = FigurePtr(new Figures::BVNode(scene));

#undef plane
#undef sphere
#undef box

#undef phongDiffuse
#undef phongSpecular
#undef perfectSpecular

    // Generate render using argument options and save as PPM
    PPMImage render =
        camera.render(width, height, rpp, rootNode, RGBColor::Black);
    render.writeFile(filenameOut.c_str());

    return 0;
}