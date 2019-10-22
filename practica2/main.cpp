#include "ppmimage.h"

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <ppm_image_in> [ <ppm_image_out ]" << std::endl;
        return 1;
    }

    // Nombre del fichero de entrada
    std::string filenameIn = argv[1];
    // Nombre del fichero de salida (puede ir por args o no)
    std::string filenameOut;
    if (argc > 2) {
        // Coger fichero de salida por args
        filenameOut = argv[2];
    } else {
        // Sacar nombre del fichero de filenameIn
        // example.ppm -> example_ldr.ppm
        filenameOut = filenameIn.substr(0, filenameIn.size() - 4) + "_ldr.ppm";
    }

    // Convertir HDR a LDR
    PPMImage hdr, ldr;
    hdr.readFile(filenameIn.c_str());
    hdr.applyToneMap(ldr);
    ldr.writeFile(filenameOut.c_str());

    return 0;
}