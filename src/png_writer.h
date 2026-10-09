#ifndef FALLOUT_PNG_WRITER_H_
#define FALLOUT_PNG_WRITER_H_

namespace fallout {

// Writes an 8-bit indexed PNG. `pixels` is a width*height buffer of palette
// indices. `palette` is a 768-byte RGB buffer with 6-bit channel values
// (matching the game's _cmap format). `transparentIndex` marks which palette
// index is written as fully transparent in the tRNS chunk; pass -1 for none.
bool pngWriteIndexed(const char* path, int width, int height,
                     const unsigned char* pixels,
                     const unsigned char* palette,
                     int transparentIndex);

} // namespace fallout

#endif