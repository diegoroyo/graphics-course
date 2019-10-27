#include "ppmimage.h"
#include <fstream>
#include <math.h>

void PPMImage::clearData(int width, int height) {
    // Reserve space for pixels
    data.resize(height);
    for (int i = 0; i < height; i++) {
        data[i].resize(width);
    }
}

void PPMImage::initialize(const int _w, const int _h, const int _c,
                          const float _max) {
    width = _w;
    height = _h;
    colorResolution = _c;
    max = _max;
    clearData(width, height);
}

bool PPMImage::readFile(const char* filename) {
    std::ifstream is(filename);
    if (!is.is_open()) {
        std::cerr << "Can't open file " << filename << std::endl;
        return false;
    }

    // Check if PPM file format is P3 (first line should be 'P3')
    char p, three;
    is >> p >> three;
    is.ignore(1);  // ignore newline
    if (p != 'P' || three != '3') {
        std::cerr << "Invalid PPM file format (first line should be 'P3')"
                  << std::endl;
        return false;
    }

    // Read file data
    int readLines = 0;
    bool end = false;
    while (!end && !is.eof()) {
        if (is.peek() == '#') {
            // Comment line (can be #MAX=<max>)
            std::string comment;
            std::getline(is, comment);
            if (comment.substr(0, 5).compare("#MAX=") == 0) {
                // string-to-float
                max = std::stof(comment.substr(5));
            }
        } else {
            // Read lines in order
            switch (readLines) {
                case 0:
                    is >> width >> height;
                    clearData(width, height);
                    break;
                case 1:
                    is >> colorResolution;
                    break;
                default:
                    int currentHeight = readLines - 2;
                    for (int i = 0; i < width; i++) {
                        int r, g, b;
                        is >> r >> g >> b;
                        // More float precision is achieved by first
                        // multiplying rgb with max and then dividing
                        data[currentHeight][i].setValues(
                            r * max / colorResolution,
                            g * max / colorResolution,
                            b * max / colorResolution);
                    }
            }
            // Increment for next iteration
            readLines++;
            // Check if all data has been read
            if (readLines - 2 == height) {
                end = true;
            }
        }
    }

    if (!end) {
        std::cerr << "PPM file ended and the reading wasn't finished"
                  << std::endl;
        return false;
    }

    return true;
}

bool PPMImage::writeFile(const char* filename, bool ldr) const {
    std::ofstream os(filename);
    if (!os.is_open()) {
        std::cerr << "Can't open file " << filename << std::endl;
        return false;
    }

    // P3 file format
    os << "P3" << std::endl;
    // Optional "MAX" comment
    if (max != 1.0f && !ldr) {
        os << "#MAX=" << max << std::endl;
    }
    // File name
    os << "# " << filename << std::endl;
    // Width, height and color resolution
    os << width << ' ' << height << std::endl;
    if (ldr) {
        os << LDR_RESOLUTION << std::endl;
    } else {
        os << colorResolution << std::endl;
    }
    // Pixel color data
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (ldr) {
                // Assumes color data is on 0..1 range
                os << int(data[y][x].r * LDR_RESOLUTION) << " "
                   << int(data[y][x].g * LDR_RESOLUTION) << " "
                   << int(data[y][x].b * LDR_RESOLUTION) << "     ";
            } else {
                // Write data in same HDR range
                os << int(data[y][x].r * colorResolution / max + 0.5f) << " "
                   << int(data[y][x].g * colorResolution / max + 0.5f) << " "
                   << int(data[y][x].b * colorResolution / max + 0.5f)
                   << "     ";
            }
        }
        os << std::endl;
    }

    return true;
}

void PPMImage::RGB2Lab(int x, int y){
    float varR,varG,varB,varX, varY, varZ, varL, vara, varb;

    varR=data[y][x].r;
    varG=data[y][x].g;
    varB=data[y][x].b;

    /*From RGB to XYZ*/
    varR > 0.04045 ? varR = pow(((varR+0.055)/1.055),2.4) : varR/=12.92;
    varG> 0.04045 ? varG = pow(((varG+0.055)/1.055),2.4) : varG/=12.92;
    varB > 0.04045 ? varB = pow(((varB+0.055)/1.055),2.4) : varB/=12.92;

    varR*=100;
    varG*=100;
    varB*=100;

    /*R->X G->Y B->Z*/
    varX = varR * 0.4124 + varG * 0.3576 + varB * 0.1805;
    varY = varR * 0.2126 + varG * 0.7152 + varB * 0.0722;
    varZ = varR * 0.0193 + varG * 0.1192 + varB * 0.9505;

    /*From XYZ to CIE-L*ab */
    varX > 0.008856 ? varX = pow(varX , 1/3) : varX= 7.787*varX+16/116;
    varY > 0.008856 ? varY = pow(varY , 1/3) : varY= 7.787*varY+16/116;
    varZ > 0.008856 ? varZ = pow(varZ , 1/3) : varZ= 7.787*varZ+16/116;

    /*X->L Y->a Z->b*/
    varL=(116 - varY)-16;
    vara=500*(varX-varY);
    varb= 200*(varY-varZ);

    /*L->r a->g b->b*/
    data[y][x].r=varL;
    data[y][x].g=vara;
    data[y][x].b=varb;

}

void PPMImage::Lab2RGB(int x, int y){
    float varR,varG,varB,varX, varY, varZ, varL, vara, varb;

    varY = ( data[y][x].r + 16 ) / 116;
    varX= data[y][x].g/ 500 + varY;
    varZ = varY - data[y][x].b / 200;

    if ( pow(varY,3)  > 0.008856 ) varY = pow(varY,3);
    else                       varY = ( varY - 16 / 116 ) / 7.787;
    if ( pow(varX,3)  > 0.008856 ) varX = pow(varX,3);
    else                       varX = ( varX - 16 / 116 ) / 7.787;
    if ( pow(varZ,3)  > 0.008856 ) varZ = pow(varZ,3);
    else                       varZ = ( varZ - 16 / 116 ) / 7.787;

    varR = varX *  3.2406 + varY * -1.5372 + varZ * -0.4986;
    varG = varX * -0.9689 + varY *  1.8758 + varZ *  0.0415;
    varB = varX *  0.0557 + varY * -0.2040 + varZ *  1.0570;

    if ( varR > 0.0031308 ) varR = 1.055 * ( pow(varR , ( 1 / 2.4 )) ) - 0.055;
    else                     varR = 12.92 * varR;
    if ( varG > 0.0031308 ) varG = 1.055 * ( pow(varG, ( 1 / 2.4 )) ) - 0.055;
    else                     varG = 12.92 * varG;
    if ( varB > 0.0031308 ) varB = 1.055 * ( pow(varB,( 1 / 2.4 )) ) - 0.055;
    else                     varB = 12.92 * varB;

    data[y][x].r = varR;
    data[y][x].g = varG;
    data[y][x].b = varB;





}

void PPMImage::applyToneMap(PPMImage& result, ToneMapper& tm) {
    result.initialize(width, height, colorResolution, max);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            RGB2Lab(x,y);

            // Map [0, max] to [0, 1] using tm.map function
            result.data[y][x].r = tm.map(data[y][x].r);
            result.data[y][x].g = tm.map(data[y][x].g);
            result.data[y][x].b = tm.map(data[y][x].b);

            Lab2RGB(x,y);
        }
    }
}