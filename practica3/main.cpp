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
    //Vec4 origin(2.5f, 0.0f, 2.5f, 1.0f), forward(-1.0f, 0.0f, 1.0f, 0.0f),
      //      up(0.0f, 1.5f, 0.0f, 0.0f), right(1.0f, 0.0f, 1.0f, 0.0f);
    Vec4 origin(2.0f, 0.0f, 4.0f, 1.0f), forward(-1.0f, 0.0f, 0.0f, 0.0f),
            up(0.0f, 1.0f, 0.0f, 0.0f), right(0.0f, 0.0f, 1.0f, 0.0f);

   /* Vec4 origin(1.2f, -1.2f, 0.0f, 1.0f), forward(-1.0f, 1.0f, 0.0f, 0.0f),
        up(0.0f, 0.0f, 1.0f, 0.0f), right(0.707f, 0.707f, 0.0f, 0.0f);*/


   //Camera camera(origin, forward, up, right);
    Camera camera(origin, forward, up, width,height);

// shortcuts for getting figure pointers
#define plane(color, normal, dist) \
    FigurePtr(new Figures::Plane(color, normal, dist))
#define sphere(color, pos, radius) \
    FigurePtr(new Figures::Sphere(color, pos, radius))
#define box(color, bb0, bb1) FigurePtr(new Figures::Box(color, bb0, bb1))

    // load & transform spaceship model, get scene kdtree node
  /*  PLYModel spaceshipModel("ply/spaceship");
    spaceshipModel.transform(Mat4::rotationX(1.2f) * Mat4::rotationY(-1.0f) *
                             Mat4::rotationZ(0.9f));
    FigurePtr spaceship = spaceshipModel.getFigure(4);*/

    // build scene to rootNode
    FigurePtrVector scene = {
        plane(RGBColor::Red, Vec4(0.0f, -1.0f, 1.0f, 0.0f), 5.0f),
       plane(RGBColor::Blue, Vec4(0.0f, 1.0f, 1.0f, 0.0f), 5.0f),
         sphere(RGBColor::Green, Vec4(0.0f, 0.0f, 5.0f, 1.0f), 0.5f),
        // box(RGBColor::Cyan, Vec4(-0.2f, -0.2f, -0.2f, 1.0f), Vec4(0.2f, 0.2f,
         //0.2f, 1.0f)),
        // sphere(RGBColor::Yellow, Vec4(0.0f, 0.0f, 2.5f, 1.0f), 0.25f),
        //spaceship
    };
    FigurePtr rootNode = FigurePtr(new Figures::BVNode(scene));

#undef plane
#undef sphere
#undef box

    // Generate render using argument options and save as PPM
    PPMImage render =
        camera.render(width, height, rpp, rootNode, RGBColor::Black);
    render.writeFile(filenameOut.c_str());

    return 0;
}