#ifndef WAV_IO_H
#define WAV_IO_H

#include "db.h"
#include <stdint.h>
#include <stdio.h>

namespace fallout {

typedef struct WavHandle {
    File* file; // VFS file handle
    long dataOffset;
    long dataSize;
    int sampleRate;
    int bitsPerSample;
    int channels;
} WavHandle;

int wavOpen(const char* filePath, int* sampleRate);
int wavClose(int fd);
int wavRead(int fd, void* buf, unsigned int size);
long wavSeek(int fd, long offset, int origin);
long wavTell(int fd);
long wavGetSize(int fd);

WavHandle* wavGetHandle(int fd);

} // namespace fallout

#endif