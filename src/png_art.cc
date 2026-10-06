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
        while (*p && (isspace((unsigned char)*p) || *p == ',')) p++;
        if (!*p) break;

        const char* keyStart = p;
        while (*p && *p != '=' && !isspace((unsigned char)*p) && *p != ',') p++;
        size_t keyLen = (size_t)(p - keyStart);
        if (*p != '=') {
            while (*p && !isspace((unsigned char)*p) && *p != ',') p++;
            continue;
        }
        p++;
        const char* valStart = p;
        while (*p && !isspace((unsigned char)*p) && *p != ',') p++;
        size_t valLen = (size_t)(p - valStart);

        char key[16] = { 0 };
        char val[32] = { 0 };
        if (keyLen >= sizeof(key)) keyLen = sizeof(key) - 1;
        if (valLen >= sizeof(val)) valLen = sizeof(val) - 1;
        memcpy(key, keyStart, keyLen);
        memcpy(val, valStart, valLen);

        int v;
        if      (strcmp(key, "fps")    == 0 && pngParseInt(val, &v)) { meta->fps = v;         meta->hasFps = true;         meta->hasAnyMeta = true; }
        else if (strcmp(key, "action") == 0 && pngParseInt(val, &v)) { meta->actionFrame = v; meta->hasActionFrame = true; meta->hasAnyMeta = true; }
        else if (strcmp(key, "rot")    == 0 && pngParseInt(val, &v)) { meta->rotations = v;   meta->hasRotations = true;   meta->hasAnyMeta = true; }
        else if (strcmp(key, "frames") == 0 && pngParseInt(val, &v)) { meta->frames = v;      meta->hasFrames = true;      meta->hasAnyMeta = true; }
        else if (strcmp(key, "fw")     == 0 && pngParseInt(val, &v)) { meta->frameWidth = v;  meta->hasFrameSize = true;   meta->hasAnyMeta = true; }
        else if (strcmp(key, "fh")     == 0 && pngParseInt(val, &v)) { meta->frameHeight = v; meta->hasFrameSize = true;   meta->hasAnyMeta = true; }
        else if (strcmp(key, "ox")     == 0 && pngParseInt(val, &v)) { meta->offsetX = v;     meta->hasOffset = true;      meta->hasAnyMeta = true; }
        else if (strcmp(key, "oy")     == 0 && pngParseInt(val, &v)) { meta->offsetY = v;     meta->hasOffset = true;      meta->hasAnyMeta = true; }
        // Unknown tokens are silently ignored.
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Vanilla FRM header reader (for inheritance)
// ---------------------------------------------------------------------------

struct InheritedFrmInfo {
    bool valid;
    int framesPerSecond;
    int actionFrame;
    int frameCount;
    int xOffsets[ROTATION_COUNT];
    int yOffsets[ROTATION_COUNT];
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

    bool ok =
        fileReadInt32(stream, &field0) != -1 &&
        fileReadInt16(stream, &fps) != -1 &&
        fileReadInt16(stream, &action) != -1 &&
        fileReadInt16(stream, &frameCount) != -1 &&
        fileReadInt16List(stream, xOffsets, ROTATION_COUNT) != -1 &&
        fileReadInt16List(stream, yOffsets, ROTATION_COUNT) != -1 &&
        fileReadInt32List(stream, dataOffsets, ROTATION_COUNT) != -1 &&
        fileReadInt32(stream, &dataSize) != -1;

    fileClose(stream);

    if (!ok) return false;

    info->valid = true;
    info->framesPerSecond = fps;
    info->actionFrame = action;
    info->frameCount = frameCount;
    for (int i = 0; i < ROTATION_COUNT; i++) {
        info->xOffsets[i] = xOffsets[i];
        info->yOffsets[i] = yOffsets[i];
    }
    return true;
}

// ---------------------------------------------------------------------------
// Metadata resolution: fills in rotations/frames/frame size
// ---------------------------------------------------------------------------

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

    // rotations: meta > 1  (do NOT inherit from FRM; PNGs are rotation-agnostic
    // unless the modder explicitly says otherwise).
    if (meta && meta->hasRotations && meta->rotations > 0) {
        rotations = meta->rotations;
    }

    // frame size: meta > derived from grid
    if (meta && meta->hasFrameSize) {
        if (meta->frameWidth  > 0) frameW = meta->frameWidth;
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
    *outFrames    = frames;
    *outFrameW    = frameW;
    *outFrameH    = frameH;
    return true;
}

// ---------------------------------------------------------------------------
// Size computation (must match artGetDataSize exactly)
// ---------------------------------------------------------------------------

static int pngComputeTotalSize(int rotations, int frames, int frameSize)
{
    int framePaddedSize = sizeof(ArtFrame) + frameSize + pngPadForSize(frameSize);
    int artDataSize = rotations * frames * framePaddedSize;

    // artGetDataSize adds (sizeof(int) - 1) * frames for each index where
    // dataOffsets changes. For our layout:
    //   rot 0..rotations-1 : distinct cumulative offsets
    //   rot rotations      : offset 0 (distinct from previous if rotations > 0)
    //   rot rotations+1..5 : offset 0 (same as previous)
    int distinct = rotations;
    if (distinct < ROTATION_COUNT) distinct += 1;
    if (distinct > ROTATION_COUNT) distinct = ROTATION_COUNT;

    int extra = (sizeof(int) - 1) * frames * distinct;
    return sizeof(Art) + artDataSize + extra;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

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

    *outSize = pngComputeTotalSize(rotations, frames, frameW * frameH);
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

    int frameSize = frameW * frameH;
    int framePaddedSize = sizeof(ArtFrame) + frameSize + pngPadForSize(frameSize);
    int total = pngComputeTotalSize(rotations, frames, frameSize);
    if (total > dataSize) {
        debugPrint("pngReadArt: buffer too small (%d < %d)\n", dataSize, total);
        stbi_image_free(pixels);
        return -1;
    }

    // ---- Metadata resolution for header fields ----
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

    // ---- Write Art header ----
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

    // ---- Write frames ----
    int writeOffset = 0;    // bytes of frame data written so far
    int dataBytes = 0;

    for (int rot = 0; rot < ROTATION_COUNT; rot++) {
        art->padding[rot] = 0;
        if (rot >= rotations) {
            // Reuse rotation 0 for all higher rotations (common for
            // static/1-rotation assets).
            art->dataOffsets[rot] = art->dataOffsets[0];
            continue;
        }

        art->dataOffsets[rot] = writeOffset;

        int fx = (meta && meta->hasOffset) ? offsetX : 0;
        int fy = (meta && meta->hasOffset) ? offsetY : 0;

        for (int f = 0; f < frames; f++) {
            ArtFrame* frame = (ArtFrame*)(data + sizeof(Art) + writeOffset);
            frame->width  = frameW;
            frame->height = frameH;
            frame->size   = frameSize;
            frame->x      = fx;
            frame->y      = fy;

            unsigned char* dst = (unsigned char*)frame + sizeof(ArtFrame);

            int srcX = f * frameW;
            int srcY = rot * frameH;

            for (int y = 0; y < frameH; y++) {
                const unsigned char* srcRow =
                    pixels + ((srcY + y) * w + srcX) * 4;
                unsigned char* dstRow = dst + y * frameW;
                for (int x = 0; x < frameW; x++) {
                    unsigned char r8 = srcRow[x * 4 + 0];
                    unsigned char g8 = srcRow[x * 4 + 1];
                    unsigned char b8 = srcRow[x * 4 + 2];
                    unsigned char a8 = srcRow[x * 4 + 3];

                    if (a8 < 128) {
                        dstRow[x] = 0; // transparent index
                    } else {
                        int r5 = r8 >> 3;
                        int g5 = g8 >> 3;
                        int b5 = b8 >> 3;
                        int rgb15 = (r5 << 10) | (g5 << 5) | b5;
                        dstRow[x] = _colorTable[rgb15];
                    }
                }
            }

            writeOffset += framePaddedSize;
            dataBytes += framePaddedSize;
        }
    }

    art->dataSize = dataBytes;

    stbi_image_free(pixels);
    return 0;
}

} // namespace fallout