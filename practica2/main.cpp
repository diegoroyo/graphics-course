#include <memory>
#include "../lib/ppmimage.h"
#include "../lib/tonemapper.h"

int main(int argc, char** argv) {
    if (argc == 1) {
        std::cerr
            << "Usage: " << argv[0]
            << " <ppm_image_in> -t <tone_mapper> [ -o <ppm_image_out> ] [ -p ]"
            << std::endl;
        std::cerr << std::endl;
        std::cerr << "Tone mapper options:" << std::endl;
        std::cerr << "\t-o Specify output file name (add .ppm/.png)" << std::endl;
        std::cerr << "\t-p Output as PNG instead of PPM" << std::endl;
        std::cerr << "\t-t CLAMP_1" << std::endl;
        std::cerr << "\t   EQUALIZE_CLAMP\t\t<clamp_pct>" << std::endl;
        std::cerr << "\t   CLAMP_GAMMA\t\t<clamp_pct>\t<gamma>" << std::endl;
        std::cerr << "\t   REINHARD_02\t\t<min_white>" << std::endl;
        std::cerr << "\t     clamp_pct\t  Higher \% luminance is clamped to white"
                  << std::endl;
        std::cerr << "\t     gamma\t  Gamma value for curve (default = 2.2)"
                  << std::endl;
        std::cerr << "\t     min_white\t  Similar to clamp_pct, for Reinhard02"
                  << std::endl;
        return 1;
    }

    // Read input file
    std::string filenameIn = argv[1];
    PPMImage hdr;
    hdr.readFile(filenameIn.c_str());

    // Process options
    std::string filenameOut;
    bool generatePng = false;  // generate PNG instead of PPM
    ToneMapper toneMapper = ToneMapper::CLAMP_1();  // default tmo
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            filenameOut = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-p") == 0) {
            generatePng = true;
        } else if (strcmp(argv[i], "-t") == 0) {
            if (strcmp(argv[i + 1], "CLAMP_01") == 0) {
                toneMapper = ToneMapper::CLAMP_1();
                i = i + 1;
            } else if (strcmp(argv[i + 1], "EQUALIZE_CLAMP") == 0) {
                float maxPct = std::stof(argv[i + 2]);
                toneMapper = ToneMapper::EQUALIZE_CLAMP(hdr.max * maxPct);
                i = i + 2;
            } else if (strcmp(argv[i + 1], "CLAMP_GAMMA") == 0) {
                float maxPct = std::stof(argv[i + 2]);
                float gamma = std::stof(argv[i + 3]);
                toneMapper = ToneMapper::CLAMP_GAMMA(hdr.max * maxPct, gamma);
                i = i + 3;
            } else if (strcmp(argv[i + 1], "REINHARD_02") == 0) {
                float minWhite = std::stof(argv[i + 2]);
                toneMapper = ToneMapper::REINHARD_02(hdr.max, minWhite);
                i = i + 2;
            }
        }
    }
    // Construct default output filename
    if (filenameOut.empty()) {
        filenameOut = filenameIn.substr(0, filenameIn.size() - 4) + "_ldr";
        if (generatePng) {
            filenameOut += ".png";
        } else {
            filenameOut += ".ppm";
        }
    }

    // Convert HDR to LDR using said toneMapper
    PPMImage ldr;
    hdr.applyToneMap(ldr, toneMapper);

    if (generatePng) {
        PNGImage pngImage = ldr.convertToPNG();
        pngImage.write_png_file(filenameOut.c_str());
    } else {
        ldr.writeFile(filenameOut.c_str(), true);
    }

    return 0;
}