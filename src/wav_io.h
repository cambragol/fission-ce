#ifndef WAV_IO_H
#define WAV_IO_H

#include <stdint.h>
#include <stdio.h>

namespace fallout {

int wavOpen(const char* filePath, int* sampleRate);
int wavClose(int fd);
int wavRead(int fd, void* buf, unsigned int size);
long wavSeek(int fd, long offset, int origin);
long wavTell(int fd);
long wavGetSize(int fd);

} // namespace fallout

#endif