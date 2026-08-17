#include "draw.h"

#include <climits>
#include <string.h>

#include "color.h"
#include "svga.h"

namespace fallout {

// 0x4D2FC0
void bufferDrawLine(unsigned char* buf, int pitch, int x1, int y1, int x2, int y2, int color)
{
    int temp;
    int dx;
    int dy;
    unsigned char* p1;
    unsigned char* p2;
    unsigned char* p3;
    unsigned char* p4;

    if (x1 == x2) {
        if (y1 > y2) {
            temp = y1;
            y1 = y2;
            y2 = temp;
        }

        p1 = buf + pitch * y1 + x1;
        p2 = buf + pitch * y2 + x2;
        while (p1 <= p2) {
            *p1 = color;
            *p2 = color;
            p1 += pitch;
            p2 -= pitch;
        }
    } else {
        if (x1 > x2) {
            temp = x1;
            x1 = x2;
            x2 = temp;

            temp = y1;
            y1 = y2;
            y2 = temp;
        }

        p1 = buf + pitch * y1 + x1;
        p2 = buf + pitch * y2 + x2;
        if (y1 == y2) {
            memset(p1, color, p2 - p1);
        } else {
            dx = x2 - x1;

            int rowStep;
            int middlePointOffset;
            int midX = x1 + (x2 - x1) / 2;
            if (y1 <= y2) {
                dy = y2 - y1;
                rowStep = pitch;
                middlePointOffset = midX + ((y2 - y1) / 2 + y1) * pitch;
            } else {
                dy = y1 - y2;
                rowStep = -pitch;
                middlePointOffset = midX + (y1 - (y1 - y2) / 2) * pitch;
            }

            p3 = buf + middlePointOffset;
            p4 = p3;

            if (dx <= dy) {
                int midpointError = dx - (dy / 2);
                int remainingSteps = dy / 4;
                while (true) {
                    *p1 = color;
                    *p2 = color;
                    *p3 = color;
                    *p4 = color;

                    if (remainingSteps == 0) {
                        break;
                    }

                    if (midpointError >= 0) {
                        p3++;
                        p2--;
                        p4--;
                        p1++;
                        midpointError -= dy;
                    }

                    p3 += rowStep;
                    p2 -= rowStep;
                    p4 -= rowStep;
                    p1 += rowStep;
                    midpointError += dx;

                    remainingSteps--;
                }
            } else {
                int midpointError = dy - (dx / 2);
                int remainingSteps = dx / 4;
                while (true) {
                    *p1 = color;
                    *p2 = color;
                    *p3 = color;
                    *p4 = color;

                    if (remainingSteps == 0) {
                        break;
                    }

                    if (midpointError >= 0) {
                        p3 += rowStep;
                        p2 -= rowStep;
                        p4 -= rowStep;
                        p1 += rowStep;
                        midpointError -= dx;
                    }

                    p3++;
                    p2--;
                    p4--;
                    p1++;
                    midpointError += dy;

                    remainingSteps--;
                }
            }
        }
    }
}

// 0x4D31A4
void bufferDrawRect(unsigned char* buf, int pitch, int left, int top, int right, int bottom, int color)
{
    bufferDrawLine(buf, pitch, left, top, right, top, color);
    bufferDrawLine(buf, pitch, left, bottom, right, bottom, color);
    bufferDrawLine(buf, pitch, left, top, left, bottom, color);
    bufferDrawLine(buf, pitch, right, top, right, bottom, color);
}

// 0x4D322C
void bufferDrawRectShadowed(unsigned char* buf, int pitch, int left, int top, int right, int bottom, int ltColor, int rbColor)
{
    bufferDrawLine(buf, pitch, left, top, right, top, ltColor);
    bufferDrawLine(buf, pitch, left, bottom, right, bottom, rbColor);
    bufferDrawLine(buf, pitch, left, top, left, bottom, ltColor);
    bufferDrawLine(buf, pitch, right, top, right, bottom, rbColor);
}

// 0x4D33F0
void blitBufferToBufferStretch(unsigned char* src, int srcWidth, int srcHeight, int srcPitch, unsigned char* dest, int destWidth, int destHeight, int destPitch)
{
    int stepX = (destWidth << 16) / srcWidth;
    int stepY = (destHeight << 16) / srcHeight;

    for (int srcY = 0; srcY < srcHeight; srcY += 1) {
        int startDestY = (srcY * stepY) >> 16;
        int endDestY = ((srcY + 1) * stepY) >> 16;

        unsigned char* currSrc = src + srcPitch * srcY;
        for (int srcX = 0; srcX < srcWidth; srcX += 1) {
            int startDestX = (srcX * stepX) >> 16;
            int endDestX = ((srcX + 1) * stepX) >> 16;

            for (int destY = startDestY; destY < endDestY; destY += 1) {
                unsigned char* currDest = dest + destPitch * destY + startDestX;
                for (int destX = startDestX; destX < endDestX; destX += 1) {
                    *currDest++ = *currSrc;
                }
            }

            currSrc++;
        }
    }
}

void blitBufferToBufferStretchTrans(unsigned char* src, int srcWidth, int srcHeight, int srcPitch,
    unsigned char* dest, int destWidth, int destHeight, int destPitch)
{
    // Calculate source rectangle size for each destination pixel (box filter)
    int stepX = (srcWidth << 16) / destWidth;
    int stepY = (srcHeight << 16) / destHeight;

    extern unsigned char _cmap[768];

    for (int dy = 0; dy < destHeight; ++dy) {
        int sy_start = (dy * stepY) >> 16;
        int sy_end = ((dy + 1) * stepY) >> 16;
        if (sy_end >= srcHeight) sy_end = srcHeight;
        if (sy_start >= sy_end) continue;

        for (int dx = 0; dx < destWidth; ++dx) {
            int sx_start = (dx * stepX) >> 16;
            int sx_end = ((dx + 1) * stepX) >> 16;
            if (sx_end >= srcWidth) sx_end = srcWidth;
            if (sx_start >= sx_end) continue;

            // Accumulate RGB from all opaque source pixels in the box
            long long totalR = 0, totalG = 0, totalB = 0;
            int opaqueCount = 0;

            for (int sy = sy_start; sy < sy_end; ++sy) {
                unsigned char* row = src + sy * srcPitch;
                for (int sx = sx_start; sx < sx_end; ++sx) {
                    int idx = row[sx];
                    if (idx != 0) {
                        totalR += _cmap[idx * 3] * 4;
                        totalG += _cmap[idx * 3 + 1] * 4;
                        totalB += _cmap[idx * 3 + 2] * 4;
                        opaqueCount++;
                    }
                }
            }

            if (opaqueCount == 0) {
                continue; // fully transparent block – leave destination unchanged
            }

            int r = (int)(totalR / opaqueCount);
            int g = (int)(totalG / opaqueCount);
            int b = (int)(totalB / opaqueCount);

            // Clamp (values are already 0?252, but safe)
            if (r < 0)
                r = 0;
            else if (r > 255)
                r = 255;
            if (g < 0)
                g = 0;
            else if (g > 255)
                g = 255;
            if (b < 0)
                b = 0;
            else if (b > 255)
                b = 255;

            // Find nearest palette colour
            int bestIdx = 0;
            int bestDist = INT_MAX;
            for (int i = 0; i < 256; ++i) {
                int pr = _cmap[i * 3] * 4;
                int pg = _cmap[i * 3 + 1] * 4;
                int pb = _cmap[i * 3 + 2] * 4;
                int dr = r - pr;
                int dg = g - pg;
                int db = b - pb;
                int dist = dr * dr + dg * dg + db * db;
                if (dist < bestDist) {
                    bestDist = dist;
                    bestIdx = i;
                    if (dist == 0) break;
                }
            }

            dest[dy * destPitch + dx] = (unsigned char)bestIdx;
        }
    }
}

// 0x4D36D4
void blitBufferToBuffer(unsigned char* src, int width, int height, int srcPitch, unsigned char* dest, int destPitch)
{
    srcCopy(dest, destPitch, src, srcPitch, width, height);
}

// 0x4D3704
void blitBufferToBufferTrans(unsigned char* src, int width, int height, int srcPitch, unsigned char* dest, int destPitch)
{
    transSrcCopy(dest, destPitch, src, srcPitch, width, height);
}

// 0x4D387C
void bufferFill(unsigned char* buf, int width, int height, int pitch, int value)
{
    int y;

    for (y = 0; y < height; y++) {
        memset(buf, value, width);
        buf += pitch;
    }
}

// 0x4D38E0
void _buf_texture(unsigned char* buf, int width, int height, int pitch, void* texture, int xOffset, int yOffset)
{
    // Intended to tile GNW window texture data when window color is 256.
    // In current CE code paths this is effectively dormant because
    // `_GNW_texture` is never populated and callers fall back to flat fills.
    (void)buf;
    (void)width;
    (void)height;
    (void)pitch;
    (void)texture;
    (void)xOffset;
    (void)yOffset;
    // TODO: Incomplete.
}

// 0x4D3A48
void _lighten_buf(unsigned char* buf, int width, int height, int pitch)
{
    int skip = pitch - width;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char color = *buf;
            *buf++ = intensityColorTable[color][147];
        }
        buf += skip;
    }
}

// Swaps two colors in the buffer.
//
// 0x4D3A8C
void _swap_color_buf(unsigned char* buf, int width, int height, int pitch, int color1, int color2)
{
    int step = pitch - width;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int currentColor = *buf & 0xFF;
            if (currentColor == color1) {
                *buf = color2 & 0xFF;
            } else if (currentColor == color2) {
                *buf = color1 & 0xFF;
            }
            buf++;
        }
        buf += step;
    }
}

// 0x4D3AE0
void bufferOutline(unsigned char* buf, int width, int height, int pitch, int color)
{
    unsigned char* ptr = buf + pitch;

    bool cycle;
    for (int y = 0; y < height - 2; y++) {
        cycle = true;

        for (int x = 0; x < width; x++) {
            if (*ptr != 0 && cycle) {
                *(ptr - 1) = color & 0xFF;
                cycle = false;
            } else if (*ptr == 0 && !cycle) {
                *ptr = color & 0xFF;
                cycle = true;
            }

            ptr++;
        }

        ptr += pitch - width;
    }

    for (int x = 0; x < width; x++) {
        ptr = buf + x;
        cycle = true;

        for (int y = 0; y < height; y++) {
            if (*ptr != 0 && cycle) {
                // TODO: Check in debugger, might be a bug.
                *(ptr - pitch) = color & 0xFF;
                cycle = false;
            } else if (*ptr == 0 && !cycle) {
                *ptr = color & 0xFF;
                cycle = true;
            }

            ptr += pitch;
        }
    }
}

// 0x4E0DB0
void srcCopy(unsigned char* dest, int destPitch, unsigned char* src, int srcPitch, int width, int height)
{
    for (int y = 0; y < height; y++) {
        memcpy(dest, src, width);
        dest += destPitch;
        src += srcPitch;
    }
}

// 0x4E0ED5
void transSrcCopy(unsigned char* dest, int destPitch, unsigned char* src, int srcPitch, int width, int height)
{
    int destSkip = destPitch - width;
    int srcSkip = srcPitch - width;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char c = *src++;
            if (c != 0) {
                *dest = c;
            }
            dest++;
        }
        src += srcSkip;
        dest += destSkip;
    }
}

} // namespace fallout
