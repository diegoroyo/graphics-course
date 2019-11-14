#include <iostream>
#include <memory>
#include "camera.h"
#include "figures.h"
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
    // Vec4 origin(0.0f, 0.0f, -2.0f, 1.0f), forward(0.0f, 0.0f, 1.0f, 0.0f),
    //     up(0.0f, 1.0f, 0.0f, 0.0f), right(1.0f, 0.0f, 0.0f, 0.0f);
    Vec4 origin(1.2f, -1.2f, 0.0f, 1.0f), forward(-1.0f, 1.0f, 0.0f, 0.0f),
        up(0.0f, 0.0f, 1.0f, 0.0f), right(0.707f, 0.707f, 0.0f, 0.0f);
    Camera camera(origin, forward, up, right);

// shortcuts for getting figure pointers
#define plane(color, normal, dist) \
    std::shared_ptr<Figures::Figure>(new Figures::Plane(color, normal, dist))
#define sphere(color, pos, radius) \
    std::shared_ptr<Figures::Figure>(new Figures::Sphere(color, pos, radius))

    FigurePtrVector scene = {
        // plane(RGBColor::Red, Vec4(0.0f, -1.0f, 1.0f, 0.0f), 5.0f),
        // plane(RGBColor::Blue, Vec4(0.0f, 1.0f, 1.0f, 0.0f), 5.0f),
        // sphere(RGBColor::Green, Vec4(0.0f, -0.2f, 0.0f, 1.0f), 0.5f),
        sphere(RGBColor::Yellow, Vec4(-0.3f, 0.0f, 0.0f, 1.0f), 0.2f)
    };

    PLYModel spaceship("ply/spaceship");
    spaceship.addTriangles(scene);

#undef plane
#undef sphere

    // Generate render using argument options and save as PPM
    PPMImage render = camera.render(width, height, rpp, scene, RGBColor::Black);
    render.writeFile(filenameOut.c_str());

    return 0;
}