#include <iostream>
#include "camera.h"
#include "figures.h"

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

    // TODO generate a scene that makes sense and is more complex
    Vec4 origin(0.0f, 0.0f, 0.0f, 1.0f), forward(0.0f, 0.0f, 1.5f, 0.0f),
        up(0.0f, 1.0f, 0.0f, 0.0f), right(2.0f, 0.0f, 0.0f, 0.0f);

    Figures::Plane* plane1= new Figures::Plane (RGBColor::Red, Vec4(0.0f, -1.0f, 1.0f, 0.0f), 5.0f);
    Figures::Plane* plane2= new Figures::Plane (RGBColor::Blue, Vec4(0.0f, 1.0f, 1.0f, 0.0f), 5.0f);
    Figures::Sphere*  sph1= new Figures::Sphere(RGBColor::Green, Vec4(0.0f, 0.0f, 4.0f, 1.0f), 0.5f);
   Figures::Sphere* sph2 =new Figures::Sphere(RGBColor::Yellow, Vec4(0.0f, 1.0f, 4.0f, 1.0f), 0.5f);

    // Set up camera & scene with previous data
    Camera camera(origin, forward, up, right);
    std::vector<Figures::Figure*> scene = {plane1, plane2, sph1};//, sph2};

    // Generate render using argument options and save as PPM
    PPMImage render = camera.render(width, height, rpp, scene, RGBColor::Black);
    render.writeFile(filenameOut.c_str());

    return 0;
}