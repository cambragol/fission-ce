#ifndef FALLOUT_PNG_ART_H_
#define FALLOUT_PNG_ART_H_

namespace fallout {

// Metadata for a PNG-based art asset. All fields are optional; unset
// fields fall back to inherited vanilla metadata or sensible defaults.
struct PngArtMeta {
    int fps;
    int actionFrame;
    int rotations;
    int frames;
    int frameWidth;
    int frameHeight;
    int offsetX;
    int offsetY;
    bool hasFps;
    bool hasActionFrame;
    bool hasRotations;
    bool hasFrames;
    bool hasFrameSize;
    bool hasOffset;
    bool hasAnyMeta;
};

void pngMetaReset(PngArtMeta* meta);

// Parse "fps=10 rot=6 frames=8" style tokens (whitespace or commas).
// Returns 0 on success; sets hasAnyMeta if at least one token parsed.
int pngParseMetaTokens(const char* tokens, PngArtMeta* meta);

// Returns the exact buffer size needed by pngReadArt.
// If inheritFromPath is non-null, metadata is read from that FRM.
int pngGetArtSize(const char* pngPath, const PngArtMeta* meta,
    const char* inheritFromPath, int* outSize);

// Writes an Art structure into `data`. `dataSize` must be at least
// the value reported by pngGetArtSize.
int pngReadArt(const char* pngPath, unsigned char* data, int dataSize,
    const PngArtMeta* meta, const char* inheritFromPath);

// Loads the sibling .pal for an FRM path if one exists. On success, fills
// `rgbOut` with 768 bytes (6-bit per channel) and/or `tableOut` with
// 32768 bytes (15-bit RGB -> palette index lookup). Caller may pass nullptr
// for either. Returns true on success.
bool pngLoadSiblingPalette(const char* frmPath,
                           unsigned char* rgbOut,
                           unsigned char* tableOut);

} // namespace fallout

#endif