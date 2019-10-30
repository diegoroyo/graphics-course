#pragma once

#include <stdint.h>
#include <cstring>
#include "pngchunk.h"
#include "rgbcolor.h"

// Métodos de apoyo para la lectura/escritura de imágenes PNG y su modificación
// Referencia: https://en.wikipedia.org/wiki/Portable_Network_Graphics
// De momento solo soporta imágenes sin paleta de colores ni alpha
// y con 8 bits de profundidad de color
class PNGImage {
   public:
    int width;
    int height;

    PNGImage();
    PNGImage(int width, int height,
             const RGBColor& backgroundColor = RGBColor::White);
    ~PNGImage();
    bool read_png_file(const char* filename);
    bool write_png_file(const char* filename);
    bool get_pixel(int x, int y, RGBColor& color);
    void set_pixel(int x, int y, const RGBColor& color);
    void flip_vertically();
    void flip_horizontally();

   private:
    // Todos los archivos PNG tienen comienzan con estos 8 bytes (ver
    // referencia)
    const static int HEADER_LENGTH = 8;
    const uint8_t HEADER_SIGNATURE[HEADER_LENGTH] = {0x89, 0x50, 0x4E, 0x47,
                                                     0x0D, 0x0A, 0x1A, 0x0A};

    // pixels[y][x] = color del pixel (x, y) de la imagen
    RGBColor*** pixels;
};