#include "../lib/ppmimage.h"
#include "../lib/tonemapper.h"

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
        filenameOut = filenameIn.substr(0, filenameIn.size() - 4) + "_ldr.png";
    }

    // Convertir HDR a LDR
    PPMImage hdr, ldr;
    hdr.readFile(filenameIn.c_str());
    // ToneMapper toneMapper = ToneMapper::CLAMP_1();
    // ToneMapper toneMapper = ToneMapper::EQUALIZE_CLAMP(hdr.max * 0.98f);
    // ToneMapper toneMapper = ToneMapper::CLAMP_GAMMA(hdr.max * 0.98f, 2.2f);
    ToneMapper toneMapper = ToneMapper::REINHARD_02(hdr.max, 1.0f);
    hdr.applyToneMap(ldr, toneMapper);
    // ldr.writeFile(filenameOut.c_str(), true);

    PNGImage pngImage = ldr.convertToPNG();
    pngImage.write_png_file(filenameOut.c_str());

    return 0;
}