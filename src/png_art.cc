#include "png_art.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "art.h"
#include "color.h"
#include "debug.h"
#include "memory.h"
#include "xfile.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

namespace fallout {

// Some art (endgame slides, death screens, help screen) ships with a
// sibling .pal file that replaces the global color.pal for that asset.
// The engine loads it at runtime; we need to do the same at export time
// (so the PNG's PLTE reflects the real colors) and at load time (so the
// RGB pixels round-trip back to the correct indices).

static const int kPaletteRgbSize = 768;    // 256 * 3
static const int kPaletteTableSize = 32768; // 2^15

// Given "art\intrface\end001.frm" (or with forward slashes), writes
// "art\intrface\end001.pal". Returns false if the input doesn't end in
// ".frm" or the output would overflow.
static bool pngDerivePalettePath(const char* frmPath,
                                 char* outPath, size_t outSize)
{
    if (frmPath == nullptr || outPath == nullptr || outSize < 5) {
        return false;
    }

    size_t len = strlen(frmPath);
    if (len < 4
        || compat_stricmp(frmPath + len - 4, ".frm") != 0) {
        return false;
    }

    if (len + 1 > outSize) {
        return false;
    }

    memcpy(outPath, frmPath, len - 4);
    memcpy(outPath + len - 4, ".pal", 5); // includes NUL
    return true;
}

bool pngLoadSiblingPalette(const char* frmPath,
                           unsigned char* rgbOut,
                           unsigned char* tableOut)
{
    if (frmPath == nullptr) {
        return false;
    }

    char palPath[COMPAT_MAX_PATH];
    if (!pngDerivePalettePath(frmPath, palPath, sizeof(palPath))) {
        return false;
    }

    File* stream = fileOpen(palPath, "rb");
    if (stream == nullptr) {
        return false;
    }

    bool ok = true;

    if (rgbOut != nullptr) {
        if (fileRead(rgbOut, 1, kPaletteRgbSize, stream) != kPaletteRgbSize) {
            ok = false;
        }
    } else {
        if (fileSeek(stream, kPaletteRgbSize, SEEK_SET) != 0) {
            ok = false;
        }
    }

    if (ok && tableOut != nullptr) {
        if (fileRead(tableOut, 1, kPaletteTableSize, stream) != kPaletteTableSize) {
            ok = false;
        }
    }

    fileClose(stream);
    return ok;
}

static int pngPadForSize(int size)
{
    return (sizeof(int) - size % sizeof(int)) % sizeof(int);
}

void pngMetaReset(PngArtMeta* meta)
{
    if (!meta) return;
    memset(meta, 0, sizeof(*meta));
}

// ---------------------------------------------------------------------------
// Metadata parsing
// ---------------------------------------------------------------------------

static bool pngParseInt(const char* str, int* out)
{
    if (!str || !*str) return false;
    char* end;
    long v = strtol(str, &end, 0);
    if (end == str) return false;
    *out = (int)v;
    return true;
}

int pngParseMetaTokens(const char* tokens, PngArtMeta* meta)
{
    if (!tokens || !meta) return -1;

    const char* p = tokens;
    while (*p) {
        while (*p && (isspace((unsigned char)*p) || *p == ','))
            p++;
        if (!*p) break;

        const char* keyStart = p;
        while (*p && *p != '=' && !isspace((unsigned char)*p) && *p != ',')
            p++;
        size_t keyLen = (size_t)(p - keyStart);
        if (*p != '=') {
            while (*p && !isspace((unsigned char)*p) && *p != ',')
                p++;
            continue;
        }
        p++;
        const char* valStart = p;
        while (*p && !isspace((unsigned char)*p) && *p != ',')
            p++;
        size_t valLen = (size_t)(p - valStart);

        char key[16] = { 0 };
        char val[32] = { 0 };
        if (keyLen >= sizeof(key)) keyLen = sizeof(key) - 1;
        if (valLen >= sizeof(val)) valLen = sizeof(val) - 1;
        memcpy(key, keyStart, keyLen);
        memcpy(val, valStart, valLen);

        int v;
        if (strcmp(key, "fps") == 0 && pngParseInt(val, &v)) {
            meta->fps = v;
            meta->hasFps = true;
            meta->hasAnyMeta = true;
        } else if (strcmp(key, "action") == 0 && pngParseInt(val, &v)) {
            meta->actionFrame = v;
            meta->hasActionFrame = true;
            meta->hasAnyMeta = true;
        } else if (strcmp(key, "rot") == 0 && pngParseInt(val, &v)) {
            meta->rotations = v;
            meta->hasRotations = true;
            meta->hasAnyMeta = true;
        } else if (strcmp(key, "frames") == 0 && pngParseInt(val, &v)) {
            meta->frames = v;
            meta->hasFrames = true;
            meta->hasAnyMeta = true;
        } else if (strcmp(key, "fw") == 0 && pngParseInt(val, &v)) {
            meta->frameWidth = v;
            meta->hasFrameSize = true;
            meta->hasAnyMeta = true;
        } else if (strcmp(key, "fh") == 0 && pngParseInt(val, &v)) {
            meta->frameHeight = v;
            meta->hasFrameSize = true;
            meta->hasAnyMeta = true;
        } else if (strcmp(key, "ox") == 0 && pngParseInt(val, &v)) {
            meta->offsetX = v;
            meta->hasOffset = true;
            meta->hasAnyMeta = true;
        } else if (strcmp(key, "oy") == 0 && pngParseInt(val, &v)) {
            meta->offsetY = v;
            meta->hasOffset = true;
            meta->hasAnyMeta = true;
        }
        // Unknown tokens are silently ignored.
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Vanilla FRM header reader (for inheritance)
// ---------------------------------------------------------------------------

#define MAX_INHERITED_FRAMES 256

struct InheritedFrmInfo {
    bool valid;
    int framesPerSecond;
    int actionFrame;
    int frameCount;
    int rotations;
    int xOffsets[ROTATION_COUNT];
    int yOffsets[ROTATION_COUNT];
    int perFrameX[ROTATION_COUNT][MAX_INHERITED_FRAMES];
    int perFrameY[ROTATION_COUNT][MAX_INHERITED_FRAMES];
    int perFrameW[ROTATION_COUNT][MAX_INHERITED_FRAMES];
    int perFrameH[ROTATION_COUNT][MAX_INHERITED_FRAMES];
};

static bool pngReadFrmHeader(const char* path, InheritedFrmInfo* info)
{
    memset(info, 0, sizeof(*info));

    File* stream = fileOpen(path, "rb");
    if (!stream) return false;

    int field0;
    short fps = 0, action = 0, frameCount = 0;
    short xOffsets[ROTATION_COUNT] = { 0 };
    short yOffsets[ROTATION_COUNT] = { 0 };
    int dataOffsets[ROTATION_COUNT] = { 0 };
    int dataSize = 0;

    bool ok = fileReadInt32(stream, &field0) != -1 && fileReadInt16(stream, &fps) != -1 && fileReadInt16(stream, &action) != -1 && fileReadInt16(stream, &frameCount) != -1 && fileReadInt16List(stream, xOffsets, ROTATION_COUNT) != -1 && fileReadInt16List(stream, yOffsets, ROTATION_COUNT) != -1 && fileReadInt32List(stream, dataOffsets, ROTATION_COUNT) != -1 && fileReadInt32(stream, &dataSize) != -1;

    if (!ok) {
        fileClose(stream);
        return false;
    }

    info->valid = true;
    info->framesPerSecond = fps;
    info->actionFrame = action;
    info->frameCount = frameCount;

    int blocks = 0;
    int prev = -1;
    for (int i = 0; i < ROTATION_COUNT; i++) {
        if (dataOffsets[i] != prev) {
            blocks++;
            prev = dataOffsets[i];
        }
    }
    info->rotations = (blocks > 1) ? ROTATION_COUNT : 1;

    for (int i = 0; i < ROTATION_COUNT; i++) {
        info->xOffsets[i] = xOffsets[i];
        info->yOffsets[i] = yOffsets[i];
    }

    // Read per-frame x/y from each rotation. The FRM header is 62 bytes
    // on disk, followed by frameCount frames per distinct rotation.
    // Each frame on disk: 12-byte header (w, h, size, x, y) then `size`
    // bytes of pixel data. No padding between frames in the file.
    const int kFrmHeaderBytes = 62;
    fileSeek(stream, kFrmHeaderBytes, SEEK_SET);

    int maxFrames = (frameCount < MAX_INHERITED_FRAMES)
        ? frameCount
        : MAX_INHERITED_FRAMES;

    for (int rot = 0; rot < ROTATION_COUNT; rot++) {
        if (rot > 0 && dataOffsets[rot] == dataOffsets[rot - 1]) {
            for (int f = 0; f < maxFrames; f++) {
                info->perFrameX[rot][f] = info->perFrameX[rot - 1][f];
                info->perFrameY[rot][f] = info->perFrameY[rot - 1][f];
            }
            continue;
        }

        for (int f = 0; f < frameCount; f++) {
            short fw = 0, fh = 0, fx = 0, fy = 0;
            int fsize = 0;

            if (fileReadInt16(stream, &fw) == -1
                || fileReadInt16(stream, &fh) == -1
                || fileReadInt32(stream, &fsize) == -1
                || fileReadInt16(stream, &fx) == -1
                || fileReadInt16(stream, &fy) == -1) {
                fileClose(stream);
                return false;
            }

            if (f < maxFrames) {
                info->perFrameX[rot][f] = fx;
                info->perFrameY[rot][f] = fy;
                info->perFrameW[rot][f] = fw;
                info->perFrameH[rot][f] = fh;
            }

            if (fsize > 0) {
                fileSeek(stream, fsize, SEEK_CUR);
            }
        }
    }

    fileClose(stream);
    return true;
}

static bool pngResolveLayout(int pngW, int pngH,
    const PngArtMeta* meta,
    const InheritedFrmInfo* frm,
    int* outRotations, int* outFrames,
    int* outFrameW, int* outFrameH)
{
    int rotations = 1;
    int frames = 1;
    int frameW = 0, frameH = 0;

    // frames: meta > inherited FRM > 1
    if (meta && meta->hasFrames && meta->frames > 0) {
        frames = meta->frames;
    } else if (frm && frm->valid && frm->frameCount > 0) {
        frames = frm->frameCount;
    }

    // rotations: meta > inherited FRM > 1
    if (meta && meta->hasRotations && meta->rotations > 0) {
        rotations = meta->rotations;
    } else if (frm && frm->valid && frm->rotations > 0) {
        rotations = frm->rotations;
    }

    // frame size: meta > derived from grid
    if (meta && meta->hasFrameSize) {
        if (meta->frameWidth > 0) frameW = meta->frameWidth;
        if (meta->frameHeight > 0) frameH = meta->frameHeight;
    }
    if (frameW <= 0 || frameH <= 0) {
        if (frames > 0 && rotations > 0
            && pngW % frames == 0 && pngH % rotations == 0) {
            frameW = pngW / frames;
            frameH = pngH / rotations;
        } else {
            // Fall back to one image = one frame, one rotation.
            frames = 1;
            rotations = 1;
            frameW = pngW;
            frameH = pngH;
        }
    }

    if (rotations <= 0 || rotations > ROTATION_COUNT) return false;
    if (frames <= 0 || frames > 4096) return false;
    if (frameW <= 0 || frameH <= 0) return false;
    if (frameW * frames != pngW) return false;
    if (frameH * rotations != pngH) return false;

    *outRotations = rotations;
    *outFrames = frames;
    *outFrameW = frameW;
    *outFrameH = frameH;
    return true;
}

// Computes the exact buffer size required to hold the Art structure
// produced by pngReadArt for a given PNG + FRM pair. Both pngGetArtSize
// and pngReadArt call this so the size reported to the cache and the size
// actually written can never drift apart.
static int pngComputeArtBufferSize(int rotations, int frames,
    int frameW, int frameH,
    const InheritedFrmInfo* frm)
{
    int total = sizeof(Art);

    for (int rot = 0; rot < rotations; rot++) {
        for (int f = 0; f < frames; f++) {
            int origW = frameW;
            int origH = frameH;
            if (frm && frm->valid
                && rot < ROTATION_COUNT
                && f < MAX_INHERITED_FRAMES) {
                if (frm->perFrameW[rot][f] > 0) origW = frm->perFrameW[rot][f];
                if (frm->perFrameH[rot][f] > 0) origH = frm->perFrameH[rot][f];
            }
            int fs = origW * origH;
            total += sizeof(ArtFrame) + fs + pngPadForSize(fs);
        }
    }

    // artGetDataSize adds (sizeof(int) - 1) * frames for each index where
    // dataOffsets changes. Our layout has `rotations` distinct offsets plus
    // one zero-offset slot if rotations < ROTATION_COUNT.
    int distinct = rotations;
    if (distinct < ROTATION_COUNT) distinct += 1;
    if (distinct > ROTATION_COUNT) distinct = ROTATION_COUNT;
    total += (sizeof(int) - 1) * frames * distinct;

    return total;
}

// Reads an entire file through the game's file system into a heap buffer.
// Caller owns the buffer and must free it with internal_free.
static unsigned char* pngSlurpGameFile(const char* path, int* outSize)
{
    File* stream = fileOpen(path, "rb");
    if (stream == nullptr) {
        debugPrint("pngSlurpGameFile: fileOpen failed for %s\n", path);
        return nullptr;
    }

    int size = fileGetSize(stream);
    if (size <= 0) {
        fileClose(stream);
        debugPrint("pngSlurpGameFile: bad size %d for %s\n", size, path);
        return nullptr;
    }

    unsigned char* buf = (unsigned char*)internal_malloc(size);
    if (buf == nullptr) {
        fileClose(stream);
        debugPrint("pngSlurpGameFile: out of memory for %s (%d bytes)\n",
            path, size);
        return nullptr;
    }

    if (fileRead(buf, size, 1, stream) != 1) {
        internal_free(buf);
        fileClose(stream);
        debugPrint("pngSlurpGameFile: fileRead failed for %s\n", path);
        return nullptr;
    }

    fileClose(stream);
    *outSize = size;
    return buf;
}

// stbi_info variant that reads through the game's file system.
// Returns 1 on success, 0 on failure (matching stbi_info semantics).
static int pngInfoFromGameFile(const char* path, int* w, int* h, int* channels)
{
    int size = 0;
    unsigned char* buf = pngSlurpGameFile(path, &size);
    if (buf == nullptr) {
        return 0;
    }
    int ok = stbi_info_from_memory(buf, size, w, h, channels);
    internal_free(buf);
    if (!ok) {
        debugPrint("pngInfoFromGameFile: stbi_info_from_memory failed for %s: %s\n",
            path, stbi_failure_reason());
    }
    return ok;
}

// stbi_load variant that reads through the game's file system.
static unsigned char* pngLoadFromGameFile(const char* path,
    int* w, int* h, int* channels,
    int reqComp)
{
    int size = 0;
    unsigned char* buf = pngSlurpGameFile(path, &size);
    if (buf == nullptr) {
        return nullptr;
    }
    unsigned char* pixels = stbi_load_from_memory(buf, size,
        w, h, channels, reqComp);
    internal_free(buf);
    if (pixels == nullptr) {
        debugPrint("pngLoadFromGameFile: stbi_load_from_memory failed for %s: %s\n",
            path, stbi_failure_reason());
    }
    return pixels;
}

int pngGetArtSize(const char* pngPath, const PngArtMeta* meta,
    const char* inheritFromPath, int* outSize)
{
    int w = 0, h = 0, channels = 0;
    if (pngInfoFromGameFile(pngPath, &w, &h, &channels) != 1) {
        debugPrint("pngGetArtSize: could not read %s\n", pngPath);
        return -1;
    }

    InheritedFrmInfo frm = { 0 };
    if (inheritFromPath) {
        pngReadFrmHeader(inheritFromPath, &frm);
    }

    int rotations, frames, frameW, frameH;
    if (!pngResolveLayout(w, h, meta, frm.valid ? &frm : nullptr,
            &rotations, &frames, &frameW, &frameH)) {
        debugPrint("pngGetArtSize: bad layout for %s (%dx%d)\n", pngPath, w, h);
        return -1;
    }

    *outSize = pngComputeArtBufferSize(rotations, frames, frameW, frameH,
        frm.valid ? &frm : nullptr);
    return 0;
}

int pngReadArt(const char* pngPath, unsigned char* data, int dataSize,
               const PngArtMeta* meta, const char* inheritFromPath)
{
    int w = 0, h = 0, channels = 0;
    unsigned char* pixels = pngLoadFromGameFile(pngPath, &w, &h, &channels, 4);
    if (!pixels) {
        debugPrint("pngReadArt: failed to load %s\n", pngPath);
        return -1;
    }

    InheritedFrmInfo frm = { 0 };
    if (inheritFromPath) {
        pngReadFrmHeader(inheritFromPath, &frm);
    }

    int rotations, frames, frameW, frameH;
    if (!pngResolveLayout(w, h, meta, frm.valid ? &frm : nullptr,
                          &rotations, &frames, &frameW, &frameH)) {
        stbi_image_free(pixels);
        return -1;
    }

    int total = pngComputeArtBufferSize(rotations, frames, frameW, frameH,
                                        frm.valid ? &frm : nullptr);
    if (total > dataSize) {
        debugPrint("pngReadArt: buffer too small (%d < %d)\n", dataSize, total);
        stbi_image_free(pixels);
        return -1;
    }

    // Metadata resolution for header fields
    int fps = 10;
    int actionFrame = 0;
    if (frm.valid) {
        fps = frm.framesPerSecond;
        actionFrame = frm.actionFrame;
    }
    if (meta && meta->hasFps) fps = meta->fps;
    if (meta && meta->hasActionFrame) actionFrame = meta->actionFrame;

    int offsetX = 0, offsetY = 0;
    if (meta && meta->hasOffset) {
        offsetX = meta->offsetX;
        offsetY = meta->offsetY;
    }

    // Local palette
    // Some art (endgame slides, death screens, help screen) has a sibling
    // .pal that replaces color.pal for that asset. If we don't use it here,
    // the recovered indices will be wrong. Load it once, fall back to the
    // global table if no sibling .pal exists.
    unsigned char localColorTable[kPaletteTableSize];
    const unsigned char* colorTable = _colorTable;
    if (inheritFromPath != nullptr
        && pngLoadSiblingPalette(inheritFromPath, nullptr, localColorTable)) {
        colorTable = localColorTable;
        debugPrint("pngReadArt: using local palette for %s\n", inheritFromPath);
    }

    // Write Art header
    Art* art = (Art*)data;
    memset(art, 0, sizeof(*art));
    art->field_0 = 0;
    art->framesPerSecond = fps;
    art->actionFrame = actionFrame;
    art->frameCount = frames;

    if (frm.valid) {
        for (int i = 0; i < ROTATION_COUNT; i++) {
            art->xOffsets[i] = frm.xOffsets[i];
            art->yOffsets[i] = frm.yOffsets[i];
        }
    }

    // Write frames
    int writeOffset = 0;
    int dataBytes = 0;

    for (int rot = 0; rot < ROTATION_COUNT; rot++) {
        art->padding[rot] = 0;
        if (rot >= rotations) {
            art->dataOffsets[rot] = art->dataOffsets[0];
            continue;
        }

        art->dataOffsets[rot] = writeOffset;

        for (int f = 0; f < frames; f++) {
            int origW = frameW;
            int origH = frameH;
            int origX = 0;
            int origY = 0;
            if (frm.valid && rot < ROTATION_COUNT && f < MAX_INHERITED_FRAMES) {
                if (frm.perFrameW[rot][f] > 0) origW = frm.perFrameW[rot][f];
                if (frm.perFrameH[rot][f] > 0) origH = frm.perFrameH[rot][f];
                origX = frm.perFrameX[rot][f];
                origY = frm.perFrameY[rot][f];
            }
            if (meta && meta->hasOffset) {
                origX = meta->offsetX;
                origY = meta->offsetY;
            }

            int origSize = origW * origH;
            int framePad = pngPadForSize(origSize);

            ArtFrame* frame = (ArtFrame*)(data + sizeof(Art) + writeOffset);
            frame->width  = origW;
            frame->height = origH;
            frame->size   = origSize;
            frame->x      = origX;
            frame->y      = origY;

            unsigned char* dst = (unsigned char*)frame + sizeof(ArtFrame);

            int srcCellX = f * frameW;
            int srcCellY = rot * frameH;

            for (int y = 0; y < origH; y++) {
                const unsigned char* srcRow = pixels
                    + ((size_t)(srcCellY + y) * (size_t)w + srcCellX) * 4;
                unsigned char* dstRow = dst + (size_t)y * (size_t)origW;

                for (int x = 0; x < origW; x++) {
                    unsigned char r8 = srcRow[x * 4 + 0];
                    unsigned char g8 = srcRow[x * 4 + 1];
                    unsigned char b8 = srcRow[x * 4 + 2];
                    unsigned char a8 = srcRow[x * 4 + 3];

                    if (a8 < 128) {
                        dstRow[x] = 0;
                    } else {
                        int rgb15 = ((r8 >> 3) << 10) | ((g8 >> 3) << 5) | (b8 >> 3);
                        dstRow[x] = colorTable[rgb15];
                    }
                }
            }

            writeOffset += sizeof(ArtFrame) + origSize + framePad;
            dataBytes += sizeof(ArtFrame) + origSize + framePad;
        }
    }

    art->dataSize = dataBytes;

    stbi_image_free(pixels);
    return 0;
}

} // namespace fallout