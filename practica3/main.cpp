#include <iostream>
#include "camera.h"
#include "figures.h"

int main(int argc, char** argv) {
    // TODO if (argc < 9)
    if (argc < 1) {
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

    // TODO practica 3
    Vec4 origenC(1, 1, 1, 1);
    Vec4 rayo(0, 1, 0, 0);
    Figures::Plane plano(Vec4(0, 1, 0, 0), 5.0f);
    Vec4 interseccion = plano.intersection(origenC, rayo);

    std::cout << interseccion << std::endl;

    Figures::Sphere sphere(Vec4(1, 5, 1, 1), 2.0f);
    interseccion = sphere.intersection(origenC, rayo);
    std::cout << interseccion << std::endl;

    return 0;
}