#include <fstream>
#include <iostream>

#include "pngchunk.h"
#include "pngimage.h"

PNGImage::PNGImage() : width(0), height(0), pixels(nullptr) {}

PNGImage::PNGImage(int width, int height, const RGBColor& backgroundColor) {
    this->width = width;
    this->height = height;
    this->pixels = new RGBColor**[height];
    for (int h = 0; h < height; h++) {
        this->pixels[h] = new RGBColor*[width];
        for (int w = 0; w < width; w++) {
            this->pixels[h][w] = new RGBColor(backgroundColor);
        }
    }
}

bool PNGImage::read_png_file(const char* filename) {
    std::ifstream is(filename, std::ios::binary);
    if (!is.is_open()) {
        std::cerr << "Can't open file " << filename << std::endl;
        is.close();
        return false;
    }
    // Comparar la cabecera del archivo para verificar que es PNG
    uint8_t header[HEADER_LENGTH];
    is.read((char*)header, HEADER_LENGTH);
    if (!is.good()) {
        std::cerr << "Can't read header for " << filename << std::endl;
        is.close();
        return false;
    } else {
        for (int i = 0; i < HEADER_LENGTH; i++) {
            if (header[i] != HEADER_SIGNATURE[i]) {
                std::cerr << "File " << filename
                          << " is not recognized as a PNG file" << std::endl;
                std::cerr << "Expected header: ";
                for (int j = 0; j < HEADER_LENGTH; j++) {
                    std::cerr << std::hex << int(HEADER_SIGNATURE[j]) << " ";
                }
                std::cerr << std::endl << "Instead got: ";
                for (int j = 0; j < HEADER_LENGTH; j++) {
                    std::cerr << std::hex << int(header[j]) << " ";
                }
                std::cerr << std::endl;
                is.close();
                return false;
            }
        }
    }

    // Leer información de la imagen, chunk a chunk
    bool endChunkRead = false;
    bool headerChunkRead = false;
    bool isImageOk = true;  // lectura ha ido bien
    int pixelX = 0;         // ej: imagen 2x2, el pixel (1, 1) (abajo derecha)
    int pixelY = 0;         // corresponde con pixelX = 1, pixelY = 1
    while (isImageOk && !endChunkRead) {
        PNGChunk chunk;
        if (!is.good() || !chunk.read_file(is)) {
            if (is.eof()) {
                std::cerr << "Warning: finished reading without IEND chunk"
                          << std::endl;
            } else {
                std::cerr << "Read invalid chunk, stopping" << std::endl;
            }
            isImageOk = false;
        } else {
            if (chunk.is_type("IHDR")) {
                PNGChunk::IHDRInfo* info =
                    dynamic_cast<PNGChunk::IHDRInfo*>(chunk.chunkInfo);
                // Solo se da soporte a imagenes PNG sencillas (solo color, sin
                // paletas ni alpha)
                if (info->is_supported()) {
                    this->width = info->width;
                    this->height = info->height;
                    headerChunkRead = true;
                } else {
                    isImageOk = false;
                }
            } else if (headerChunkRead) {
                if (chunk.is_type("IDAT")) {
                    PNGChunk::IDATInfo* info =
                        dynamic_cast<PNGChunk::IDATInfo*>(chunk.chunkInfo);
                    isImageOk = info->process_pixels(width, height);
                    this->pixels = info->pixelData;
                } else if (chunk.is_type("IEND")) {
                    endChunkRead = true;
                } else {
                    std::cerr << "Warning: unrecognized chunk type"
                              << std::endl;
                }
            } else {
                std::cerr << "Error: the first chunk is not an IHDR chunk"
                          << std::endl;
                isImageOk = false;
            }
        }
    }

    // Evitar acceso a los datos de la imagen si no se ha leido correctamente
    if (!isImageOk) {
        this->width = 0;
        this->height = 0;
    }

    return isImageOk;
}

// Escribe los chunks mínimos para poder ver la imagen
// - Chunk IHDR (24 bit RGB, color, sin paletas)
// - Chunk(s) IDAT (tamaño maximo 8192 bits)
//   24 bits por pixel (RGB)
//   Sin ningún tipo de filtrado ni compresión
// - Chunk IEND
bool PNGImage::write_png_file(const char* filename) {
    std::ofstream os(filename, std::ios::binary);
    if (!os.is_open()) {
        std::cerr << "Can't open file " << filename << std::endl;
        os.close();
        return false;
    }
    // Header archivo PNG
    os.write((char*)HEADER_SIGNATURE, HEADER_LENGTH);
    // Chunk IHDR
    PNGChunk::IHDRInfo* infoIHDR = new PNGChunk::IHDRInfo(width, height);
    PNGChunk chunkIHDR(infoIHDR);
    if (!chunkIHDR.write_file(os)) {
        std::cerr << "An error ocurred while writing the IHDR chunk"
                  << std::endl;
        return false;
    }
    // Chunk IDAT (se escribe en un unico chunk)
    PNGChunk::IDATInfo* infoIDAT =
        new PNGChunk::IDATInfo(width, height, pixels);
    PNGChunk chunkIDAT(infoIDAT);
    if (!chunkIDAT.write_file(os)) {
        std::cerr << "An error ocurred while writing the IDAT chunk"
                  << std::endl;
        return false;
    }
    // Chunk IEND
    PNGChunk::IENDInfo* infoIEND = new PNGChunk::IENDInfo();
    PNGChunk chunkIEND(infoIEND);
    if (!chunkIEND.write_file(os)) {
        std::cerr << "An error ocurred while writing the IEND chunk"
                  << std::endl;
        return false;
    }

    return true;
}

// Si devuelve true, color toma el valor del pixel
// en la posicion (x, y) de la imagen
bool PNGImage::get_pixel(int x, int y, RGBColor& color) {
    if (x >= 0 && x < this->width && y >= 0 && y < this->height) {
        color.setValues(*pixels[y][x]);
        return true;
    } else {
        return false;
    }
}

// Si las coordenadas (x, y) pertenecen a la foto (no se salen),
// modifica el color de dicho pixel
void PNGImage::set_pixel(int x, int y, const RGBColor& color) {
    if (x >= 0 && x < this->width && y >= 0 && y < this->height) {
        pixels[y][x] = new RGBColor(color);
    }
}

// Volteo vertical
void PNGImage::flip_vertically() {
    for (int x = 0; x < this->width; x++) {
        for (int y = 0; y < this->height / 2; y++) {
            RGBColor leftColor, rightColor;
            this->get_pixel(x, y, leftColor);
            this->get_pixel(x, this->height - y, rightColor);
            this->set_pixel(x, this->height - y, leftColor);
            this->set_pixel(x, y, rightColor);
        }
    }
}

// Volteo horizontal
void PNGImage::flip_horizontally() {
    for (int y = 0; y < this->height; y++) {
        for (int x = 0; x < this->width / 2; x++) {
            RGBColor leftColor, rightColor;
            this->get_pixel(x, y, leftColor);
            this->get_pixel(this->width - x, y, rightColor);
            this->set_pixel(this->width - x, y, leftColor);
            this->set_pixel(x, y, rightColor);
        }
    }
}

PNGImage::~PNGImage() {
    for (int h = 0; h < this->height; h++) {
        for (int w = 0; w < this->width; w++) {
            delete this->pixels[h][w];
        }
        delete[] this->pixels[h];
    }
    delete[] this->pixels;
}