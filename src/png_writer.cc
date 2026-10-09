#include "png_writer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

namespace fallout {

static void pngWriteU32BE(unsigned char* buf, unsigned int v)
{
    buf[0] = (unsigned char)((v >> 24) & 0xFF);
    buf[1] = (unsigned char)((v >> 16) & 0xFF);
    buf[2] = (unsigned char)((v >>  8) & 0xFF);
    buf[3] = (unsigned char)((v      ) & 0xFF);
}

static void pngWriteChunk(FILE* f, const char* type,
                          const unsigned char* data, unsigned int len)
{
    unsigned char header[8];
    pngWriteU32BE(header, len);
    memcpy(header + 4, type, 4);
    fwrite(header, 1, 8, f);
    if (len > 0) {
        fwrite(data, 1, len, f);
    }

    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, (const Bytef*)type, 4);
    if (len > 0) {
        crc = crc32(crc, data, len);
    }

    unsigned char crcbuf[4];
    pngWriteU32BE(crcbuf, (unsigned int)crc);
    fwrite(crcbuf, 1, 4, f);
}

bool pngWriteIndexed(const char* path, int width, int height,
                     const unsigned char* pixels,
                     const unsigned char* palette,
                     int transparentIndex)
{
    if (path == nullptr || width <= 0 || height <= 0
        || pixels == nullptr || palette == nullptr) {
        return false;
    }

    FILE* f = fopen(path, "wb");
    if (f == nullptr) {
        return false;
    }

    // PNG signature
    static const unsigned char sig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    fwrite(sig, 1, 8, f);

    // IHDR
    unsigned char ihdr[13];
    pngWriteU32BE(ihdr, (unsigned int)width);
    pngWriteU32BE(ihdr + 4, (unsigned int)height);
    ihdr[8]  = 8; // bit depth
    ihdr[9]  = 3; // color type: indexed
    ihdr[10] = 0; // compression method
    ihdr[11] = 0; // filter method
    ihdr[12] = 0; // interlace method
    pngWriteChunk(f, "IHDR", ihdr, 13);

    // PLTE - scale 6-bit game palette to 8-bit. v6 << 2 makes the loader's
    // 8->5-bit reduction exactly recover the original 15-bit color, so the
    // round-trip through _colorTable is stable.
    unsigned char plte[768];
    for (int i = 0; i < 256; i++) {
        plte[i * 3 + 0] = (unsigned char)(palette[i * 3 + 0] << 2);
        plte[i * 3 + 1] = (unsigned char)(palette[i * 3 + 1] << 2);
        plte[i * 3 + 2] = (unsigned char)(palette[i * 3 + 2] << 2);
    }
    pngWriteChunk(f, "PLTE", plte, 768);

    // tRNS - mark the transparent index. 255 for everything else.
    if (transparentIndex >= 0 && transparentIndex < 256) {
        unsigned char trns[256];
        memset(trns, 255, 256);
        trns[transparentIndex] = 0;
        pngWriteChunk(f, "tRNS", trns, 256);
    }

    // IDAT - raw scanlines, each prefixed with a filter byte (0 = None).
    size_t rowBytes = (size_t)width + 1;
    size_t rawSize = rowBytes * (size_t)height;

    unsigned char* raw = (unsigned char*)malloc(rawSize);
    if (raw == nullptr) {
        fclose(f);
        return false;
    }

    for (int y = 0; y < height; y++) {
        raw[y * rowBytes] = 0;
        memcpy(raw + y * rowBytes + 1, pixels + (size_t)y * width, width);
    }

    uLongf compressedSize = compressBound((uLong)rawSize);
    unsigned char* compressed = (unsigned char*)malloc(compressedSize);
    if (compressed == nullptr) {
        free(raw);
        fclose(f);
        return false;
    }

    int zres = compress2(compressed, &compressedSize, raw, (uLong)rawSize, 6);
    free(raw);

    if (zres != Z_OK) {
        free(compressed);
        fclose(f);
        return false;
    }

    pngWriteChunk(f, "IDAT", compressed, (unsigned int)compressedSize);
    free(compressed);

    // IEND
    pngWriteChunk(f, "IEND", nullptr, 0);

    fclose(f);
    return true;
}

} // namespace fallout