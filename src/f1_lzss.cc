#include "f1_lzss.h"

#include <string.h>

namespace fallout {

// 4116 > 0x1000 + max chunk length (18), matching the CE buffer size.
static unsigned char ringBuffer[4116];
static int ringBufferIndex;

int f1LzssDecode(FILE* in, unsigned char* dest, unsigned int length)
{
    unsigned char* curr = dest;
    unsigned char decodeBuffer[1024];
    unsigned char* decodePos = decodeBuffer;
    unsigned char* decodeEnd = decodeBuffer;
    unsigned int bytesLeft = length; // compressed bytes left to READ from file
    unsigned int consumed = 0; // compressed bytes CONSUMED from buffer

    memset(ringBuffer, ' ', 4078);
    ringBufferIndex = 4078;

    while (consumed < length) {
        // Refill the decode buffer when we're running low.
        if (bytesLeft != 0 && (decodeEnd - decodePos) <= 16) {
            if (decodePos == decodeEnd) {
                decodeEnd = decodeBuffer;
            } else {
                memmove(decodeBuffer, decodePos, decodeEnd - decodePos);
                decodeEnd = decodeBuffer + (decodeEnd - decodePos);
            }
            decodePos = decodeBuffer;

            size_t toRead = 1024 - (decodeEnd - decodeBuffer);
            if (toRead > bytesLeft) toRead = bytesLeft;
            size_t read = fread(decodeEnd, 1, toRead, in);
            if (read == 0) break;
            decodeEnd += read;
            bytesLeft -= read;
        }

        if (decodePos >= decodeEnd) break;

        unsigned char control = *decodePos++;
        consumed++;

        for (int bit = 0; bit < 8; bit++) {
            if (consumed >= length) break;

            if (control & (1 << bit)) {
                // Literal byte.
                if (decodePos >= decodeEnd) goto done;
                unsigned char b = *decodePos++;
                consumed++;
                *curr++ = b;
                ringBuffer[ringBufferIndex] = b;
                ringBufferIndex = (ringBufferIndex + 1) & 0xFFF;
            } else {
                // Back-reference pair: 12-bit offset, 4-bit length (+3).
                if (decodeEnd - decodePos < 2) goto done;
                if (consumed + 2 > length) goto done;

                unsigned char lo = *decodePos++;
                unsigned char hi = *decodePos++;
                consumed += 2;

                int dictOffset = lo | ((hi & 0xF0) << 4);
                int chunkLen = (hi & 0x0F) + 3;
                for (int k = 0; k < chunkLen; k++) {
                    int di = (dictOffset + k) & 0xFFF;
                    unsigned char b = ringBuffer[di];
                    *curr++ = b;
                    ringBuffer[ringBufferIndex] = b;
                    ringBufferIndex = (ringBufferIndex + 1) & 0xFFF;
                }
            }
        }
    }

done:
    return (int)(curr - dest);
}

} // namespace fallout