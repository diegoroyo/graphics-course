#pragma once

#include <stdint.h>
#include "rgbcolor.h"

// Una imagen PNG está formada por varios chunks de este tipo
// http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html
// Cada chunk tiene la siguiente estructura:
//   4 bytes: longitud del fichero (Big Endian)
//   4 bytes: tipo de chunk (ver IHDR, PLTE, IDAT, IEND)
//   <longitud> bytes: datos del chunk
//   4 bytes: CRC
class PNGChunk {
   private:
    static const uint32_t CRC32_DIVISOR = 0xEDB88320;  // ver calculate_crc
    static uint32_t CRC_TABLE[256];
    uint8_t* chunkCrcDividend;

    uint32_t calculate_crc(uint8_t* stream, int streamLength);
    bool read_header(std::ifstream& is);
    bool read_data(std::ifstream& is);

   public:
    // Posibilidad de ampliar a más tipos: PLTE, etc.
    class ChunkInfo {
       public:
        virtual bool read_info(uint8_t* data, uint32_t length) = 0;
        virtual bool get_writable_info(uint8_t*& data, uint32_t& length) = 0;
    };

    // Información principal de la imagen
    class IHDRInfo : public ChunkInfo {
       private:
        // Bytes de información (ver campos públicos)
        static const int IHDR_LENGTH = 13;
        // Valores soportados
        static const int IHDR_BIT_DEPTH = 8;
        static const int IHDR_COLOR_TYPE = 2;
        static const int IHDR_COMPRESSION = 0;
        static const int IHDR_FILTER = 0;
        static const int IHDR_INTERLACE = 0;

       public:
        uint32_t width, height;
        uint8_t bitDepth, colorType, compression, filter, interlace;

        IHDRInfo() = default;
        IHDRInfo(int _width, int _height);
        bool is_supported();
        bool read_info(uint8_t* data, uint32_t length) override;
        bool get_writable_info(uint8_t*& data, uint32_t& length) override;
    };

    // Datos (colores) de la imagen
    class IDATInfo : public ChunkInfo {
       private:
        // CINF = 0, CM = 8, FLEVEL = FDICT = 0, FCHECK = 1D (mult. 31)
        static const uint16_t IDAT_ZLIB_HEADER = 0x081D;
        // Last block marker 0, block type 00 (menor a mayor peso)
        static const uint8_t IDAT_BLOCK_HEADER = 0x00;
        // Marcador de último bloque (OR con BLOCK_HEADER)
        static const uint8_t IDAT_BLOCK_LAST = 0x01;
        static const uint16_t MAX_BLOCK = 65535;  // 16b para length
        static const int ADLER_MODULO = 65521;    // ver adler_checksum

        uint8_t paeth_pred(uint8_t a, uint8_t b, uint8_t c);
        bool process_pixel_data(int width, int height, uint8_t* data,
                                int length);

       public:
        // Tamaño mínimo (headers sin datos)
        // 2 header + 5 block header | 4 block length
        static const int IDAT_LENGTH_ZLIB = 2;
        static const int IDAT_LENGTH_BLOCK = 5;
        static const int IDAT_LENGTH_CHECKSUM = 4;

        uint8_t* blockData;
        int blockLength;    // son 16b pero almacena todos bloques DEFLATE
        uint32_t checksum;  // solo valido para el ultimo chunk
        RGBColor*** pixelData;

        IDATInfo() = default;
        IDATInfo(int width, int height, RGBColor*** pixels);
        uint32_t adler_checksum(uint8_t* data, uint32_t length);
        bool process_pixels(int width, int height);
        bool read_info(uint8_t* data, uint32_t length) override;
        bool get_writable_info(uint8_t*& data, uint32_t& length) override;
    };

    class IENDInfo : public ChunkInfo {
       public:
        bool read_info(uint8_t* data, uint32_t length) override;
        bool get_writable_info(uint8_t*& data, uint32_t& length) override;
    };

    uint32_t length;
    uint8_t* chunkType;
    uint8_t* data;
    uint32_t crc;
    ChunkInfo* chunkInfo;

    PNGChunk();
    PNGChunk(ChunkInfo* _chunkInfo);
    ~PNGChunk();

    bool read_file(std::ifstream& is);
    bool write_file(std::ofstream& os);
    bool is_type(const char* type);
};