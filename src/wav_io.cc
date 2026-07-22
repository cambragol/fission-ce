#include "wav_io.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"
#include "debug.h"
#include "platform_compat.h"

namespace fallout {

#define MAX_WAV_HANDLES 32

typedef struct WavHandle {
    File* file; // VFS file handle
    long dataOffset;
    long dataSize;
    int sampleRate;
    int bitsPerSample;
    int channels;
} WavHandle;

static WavHandle* gWavHandles[MAX_WAV_HANDLES] = { 0 };

static int allocateWavHandle(WavHandle* wh)
{
    for (int i = 0; i < MAX_WAV_HANDLES; i++) {
        if (gWavHandles[i] == nullptr) {
            gWavHandles[i] = wh;
            return i + 1;
        }
    }
    return 0;
}

static WavHandle* getWavHandle(int fd)
{
    int index = fd - 1;
    if (index < 0 || index >= MAX_WAV_HANDLES) {
        return nullptr;
    }
    return gWavHandles[index];
}

static void freeWavHandle(int fd)
{
    int index = fd - 1;
    if (index >= 0 && index < MAX_WAV_HANDLES) {
        gWavHandles[index] = nullptr;
    }
}

int wavOpen(const char* filePath, int* sampleRate)
{
    debugPrint("wavOpen: Trying to open %s\n", filePath);

    File* file = fileOpen(filePath, "rb");
    if (!file) {
        debugPrint("wavOpen: Failed to open file via VFS\n");
        return -1;
    }
    debugPrint("wavOpen: File opened via VFS successfully!\n");

    // Read RIFF header
    char riff[5];
    uint32_t riffSize;
    if (fileRead(riff, 1, 4, file) != 4) {
        fileClose(file);
        return -1;
    }
    riff[4] = '\0';
    if (fileRead(&riffSize, 4, 1, file) != 1) {
        fileClose(file);
        return -1;
    }

    if (strcmp(riff, "RIFF") != 0) {
        debugPrint("wavOpen: Not a valid RIFF file\n");
        fileClose(file);
        return -1;
    }

    char wave[5];
    if (fileRead(wave, 1, 4, file) != 4) {
        fileClose(file);
        return -1;
    }
    wave[4] = '\0';

    if (strcmp(wave, "WAVE") != 0) {
        debugPrint("wavOpen: Not a WAVE file\n");
        fileClose(file);
        return -1;
    }

    WavHandle* wh = (WavHandle*)calloc(1, sizeof(WavHandle));
    if (!wh) {
        fileClose(file);
        return -1;
    }
    wh->file = file;
    wh->dataOffset = 0;
    wh->dataSize = 0;

    while (1) {
        char chunkId[5];
        uint32_t chunkSize;
        if (fileRead(chunkId, 1, 4, file) != 4) break;
        chunkId[4] = '\0';
        if (fileRead(&chunkSize, 4, 1, file) != 1) break;

        if (strcmp(chunkId, "fmt ") == 0) {
            uint16_t audioFormat, numChannels, blockAlign, bitsPerSample;
            uint32_t sampleRate32, byteRate;
            if (fileRead(&audioFormat, 2, 1, file) != 1) break;
            if (fileRead(&numChannels, 2, 1, file) != 1) break;
            if (fileRead(&sampleRate32, 4, 1, file) != 1) break;
            if (fileRead(&byteRate, 4, 1, file) != 1) break;
            if (fileRead(&blockAlign, 2, 1, file) != 1) break;
            if (fileRead(&bitsPerSample, 2, 1, file) != 1) break;

            if (audioFormat != 1) {
                debugPrint("wavOpen: Only PCM format supported (got %d)\n", audioFormat);
                break;
            }

            wh->sampleRate = sampleRate32;
            wh->bitsPerSample = bitsPerSample;
            wh->channels = numChannels;
            *sampleRate = sampleRate32;

            debugPrint("wavOpen: rate=%d, bits=%d, channels=%d\n",
                wh->sampleRate, wh->bitsPerSample, wh->channels);

            long remaining = chunkSize - 16;
            if (remaining > 0) fileSeek(file, remaining, SEEK_CUR);
        } else if (strcmp(chunkId, "data") == 0) {
            wh->dataOffset = fileTell(file);
            wh->dataSize = chunkSize;
            debugPrint("wavOpen: data chunk at offset %ld, size %ld\n",
                wh->dataOffset, wh->dataSize);
            break;
        } else {
            fileSeek(file, chunkSize, SEEK_CUR);
        }
    }

    if (wh->dataSize == 0) {
        debugPrint("wavOpen: No data chunk found\n");
        fileClose(file);
        free(wh);
        return -1;
    }

    fileSeek(file, wh->dataOffset, SEEK_SET);

    int fd = allocateWavHandle(wh);
    if (fd == 0) {
        debugPrint("wavOpen: Too many WAV files open\n");
        fileClose(file);
        free(wh);
        return -1;
    }

    debugPrint("wavOpen: Success! fd=%d\n", fd);
    return fd;
}

int wavClose(int fd)
{
    WavHandle* wh = getWavHandle(fd);
    if (!wh) return -1;
    fileClose(wh->file);
    free(wh);
    freeWavHandle(fd);
    return 0;
}

int wavRead(int fd, void* buf, unsigned int size)
{
    WavHandle* wh = getWavHandle(fd);
    if (!wh) return -1;

    long current = fileTell(wh->file);
    long remaining = wh->dataOffset + wh->dataSize - current;
    if (remaining <= 0) return 0;
    if ((long)size > remaining) size = (unsigned int)remaining;

    return fileRead(buf, 1, size, wh->file);
}

long wavSeek(int fd, long offset, int origin)
{
    WavHandle* wh = getWavHandle(fd);
    if (!wh) return -1;

    long newPos;
    switch (origin) {
    case SEEK_SET:
        newPos = wh->dataOffset + offset;
        break;
    case SEEK_CUR:
        newPos = fileTell(wh->file) + offset;
        break;
    case SEEK_END:
        newPos = wh->dataOffset + wh->dataSize + offset;
        break;
    default:
        return -1;
    }

    if (newPos < wh->dataOffset || newPos > wh->dataOffset + wh->dataSize)
        return -1;

    if (fileSeek(wh->file, newPos, SEEK_SET) != 0) {
        return -1;
    }

    return newPos - wh->dataOffset;
}

long wavTell(int fd)
{
    WavHandle* wh = getWavHandle(fd);
    if (!wh) return -1;
    return fileTell(wh->file) - wh->dataOffset;
}

long wavGetSize(int fd)
{
    WavHandle* wh = getWavHandle(fd);
    if (!wh) return -1;
    return wh->dataSize;
}

} // namespace fallout