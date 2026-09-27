#ifndef F1_LZSS_H
#define F1_LZSS_H

#include <stdio.h>

namespace fallout {

// Decodes [length] bytes of LZSS-compressed data from [in] into [dest].
// Returns the number of bytes actually written to [dest].
//
// Ported from Fallout 1 CE: plib/db/lzss.cc (lzss_decode_to_buf).
int f1LzssDecode(FILE* in, unsigned char* dest, unsigned int length);

} // namespace fallout

#endif