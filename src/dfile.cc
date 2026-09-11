#include "dfile.h"

#include "window_manager.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#include <fpattern/fpattern.h>

#include "f1_lzss.h"
#include "game_version.h"
#include "platform_compat.h"

namespace fallout {

// The size of decompression buffer for reading compressed [DFile]s.
#define DFILE_DECOMPRESSION_BUFFER_SIZE (0x400)

// Specifies that [DFile] has unget character.
//
// NOTE: There is an unused function at 0x4E5894 which ungets one character and
// stores it in [ungotten]. Since that function is not used, this flag will
// never be set.
#define DFILE_HAS_UNGETC (0x01)

// Specifies that [DFile] has reached end of stream.
#define DFILE_EOF (0x02)

// Specifies that [DFile] is in error state.
//
// [dfileRewind] can be used to clear this flag.
#define DFILE_ERROR (0x04)

// Specifies that [DFile] was opened in text mode.
#define DFILE_TEXT (0x08)

// Specifies that [DFile] has unget compressed character.
#define DFILE_HAS_COMPRESSED_UNGETC (0x10)

// Specifies that [DFile] was pre-decoded into [decompressionBuffer] at open
// time. Used for LZSS (Fallout 1) entries.
#define DFILE_PREDECODED (0x20)

static int dbaseFindEntryByFilePath(const void* file, const void* entryName);
static DFile* dfileOpenInternal(DBase* dbase, const char* filename, const char* mode, DFile* dfile);
static int dfileReadCharInternal(DFile* stream);
static bool dfileReadCompressed(DFile* stream, void* ptr, size_t size);
static void dfileUngetCompressed(DFile* stream, int ch);

/**
 * Normalizes a path for .DAT file access.
 * Converts all forward slashes to backslashes and ensures consistent formatting.
 */
static char* normalizePathForDat(const char* path)
{
    if (path == nullptr) {
        return nullptr;
    }

    char* normalizedPath = (char*)malloc(strlen(path) + 1);
    if (normalizedPath == nullptr) {
        return nullptr;
    }

    strcpy(normalizedPath, path);

    // Convert all forward slashes to backslashes
    for (char* p = normalizedPath; *p != '\0'; p++) {
        if (*p == '/') {
            *p = '\\';
        }
    }

    return normalizedPath;
}

// Reads a big-endian uint32.
static int f1ReadBE32(FILE* stream, unsigned int* out)
{
    unsigned char b[4];
    if (fread(b, 1, 4, stream) != 4) return -1;
    *out = ((unsigned int)b[0] << 24)
         | ((unsigned int)b[1] << 16)
         | ((unsigned int)b[2] << 8)
         |  (unsigned int)b[3];
    return 0;
}

// qsort comparator so entries are bsearch-able, same as F2 path.
static int dbaseEntryPathCompare(const void* a, const void* b)
{
    const DBaseEntry* ea = (const DBaseEntry*)a;
    const DBaseEntry* eb = (const DBaseEntry*)b;
    return compat_stricmp(ea->path, eb->path);
}

// Parses a classic Fallout 1 .DAT file (big-endian nested assoc array).
//
// On success returns a DBase with [dataOffset] = 0 because F1 entry offsets
// are already absolute. On failure returns nullptr and frees anything it
// allocated.
static DBase* dbaseOpenFallout1(FILE* stream, int fileSize, const char* filePath)
{
    (void)fileSize;

    // --- All locals declared up front so the cleanup gotos don't skip any
    // --- initialization. ---
    unsigned int rootCount = 0;
    unsigned int rootMax = 0;
    unsigned int rootDataSize = 0;
    unsigned int rootListPtr = 0;

    char** dirNames = nullptr;

    int cap = 256;
    int count = 0;
    DBaseEntry* entries = nullptr;

    DBase* dbase = nullptr;

    unsigned int i;

    if (f1ReadBE32(stream, &rootCount) != 0) return nullptr;
    if (f1ReadBE32(stream, &rootMax) != 0) return nullptr;
    if (f1ReadBE32(stream, &rootDataSize) != 0) return nullptr;
    if (f1ReadBE32(stream, &rootListPtr) != 0) return nullptr;

    if (rootCount == 0 || rootCount > 10000) return nullptr;
    if (rootDataSize > 1024) return nullptr;

    // Directory names.
    dirNames = (char**)calloc(rootCount, sizeof(char*));
    if (dirNames == nullptr) return nullptr;

    for (i = 0; i < rootCount; i++) {
        int nameLen = fgetc(stream);
        if (nameLen <= 0 || nameLen > 255) goto err_names;

        dirNames[i] = (char*)malloc(nameLen + 1);
        if (dirNames[i] == nullptr) goto err_names;
        if (fread(dirNames[i], 1, nameLen, stream) != (size_t)nameLen) goto err_names;
        dirNames[i][nameLen] = '\0';

        if (rootDataSize != 0) {
            if (fseek(stream, rootDataSize, SEEK_CUR) != 0) goto err_names;
        }
    }

    // Growable entry list.
    entries = (DBaseEntry*)malloc(sizeof(DBaseEntry) * cap);
    if (entries == nullptr) goto err_names;

    for (i = 0; i < rootCount; i++) {
        unsigned int subCount;
        unsigned int subMax;
        unsigned int subDataSize;
        unsigned int subListPtr;

        if (f1ReadBE32(stream, &subCount) != 0) goto err_entries;
        if (f1ReadBE32(stream, &subMax) != 0) goto err_entries;
        if (f1ReadBE32(stream, &subDataSize) != 0) goto err_entries;
        if (f1ReadBE32(stream, &subListPtr) != 0) goto err_entries;

        if (subDataSize != 16) goto err_entries;   // sizeof(dir_entry)
        if (subCount > 1000000) goto err_entries;

        for (unsigned int j = 0; j < subCount; j++) {
            int nameLen = fgetc(stream);
            if (nameLen <= 0 || nameLen > 255) goto err_entries;

            char nameBuf[256];
            if (fread(nameBuf, 1, nameLen, stream) != (size_t)nameLen) goto err_entries;
            nameBuf[nameLen] = '\0';

            unsigned int flags;
            unsigned int offset;
            unsigned int length;
            unsigned int fieldC;
            if (f1ReadBE32(stream, &flags) != 0) goto err_entries;
            if (f1ReadBE32(stream, &offset) != 0) goto err_entries;
            if (f1ReadBE32(stream, &length) != 0) goto err_entries;
            if (f1ReadBE32(stream, &fieldC) != 0) goto err_entries;

            // Compose "<DIR>\\<NAME>".
            size_t dirLen = strlen(dirNames[i]);
            size_t totalLen = dirLen + 1 + (size_t)nameLen;
            char* path = (char*)malloc(totalLen + 1);
            if (path == nullptr) goto err_entries;

            memcpy(path, dirNames[i], dirLen);
            path[dirLen] = '\\';
            memcpy(path + dirLen + 1, nameBuf, nameLen);
            path[totalLen] = '\0';

            if (count >= cap) {
                int newCap = cap * 2;
                DBaseEntry* grown = (DBaseEntry*)realloc(entries, sizeof(DBaseEntry) * newCap);
                if (grown == nullptr) { free(path); goto err_entries; }
                entries = grown;
                cap = newCap;
            }

            DBaseEntry* e = &entries[count++];
            e->path = path;
            e->dataOffset = (int)offset;
            e->uncompressedSize = (int)length;
            e->dataSize = (int)fieldC;

            switch (flags & 0xF0) {
            case 0x10: e->compressionType = 2; e->compressed = 1; break; // LZSS
            case 0x20: e->compressionType = 0; e->compressed = 0; break; // raw
            case 0x40: e->compressionType = 3; e->compressed = 1; break; // chunked LZSS
            default: goto err_entries;
            }

            // For raw entries the two sizes are equal; normalize for safety.
            if (e->compressionType == 0) {
                e->dataSize = e->uncompressedSize;
            }
        }
    }

    qsort(entries, count, sizeof(DBaseEntry), dbaseEntryPathCompare);

    dbase = (DBase*)malloc(sizeof(*dbase));
    if (dbase == nullptr) goto err_entries;
    memset(dbase, 0, sizeof(*dbase));

    dbase->path = compat_strdup(filePath);
    if (dbase->path == nullptr) { free(dbase); dbase = nullptr; goto err_entries; }
    dbase->dataOffset = 0;
    dbase->entriesLength = count;
    dbase->entries = entries;
    dbase->dfileHead = nullptr;

    for (i = 0; i < rootCount; i++) free(dirNames[i]);
    free(dirNames);

    falloutVersionSet(FALLOUT_VERSION_1);
    return dbase;

err_entries:
    for (int k = 0; k < count; k++) free(entries[k].path);
    free(entries);
err_names:
    for (i = 0; i < rootCount; i++) free(dirNames[i]);
    free(dirNames);
    return nullptr;
}

// Reads .DAT file contents.
//
// 0x4E4F58
DBase* dbaseOpen(const char* filePath)
{
    assert(filePath); // "filename", "dfile.c", 74

    FILE* stream = compat_fopen(filePath, "rb");
    if (stream == nullptr) {
        fprintf(stderr, "[DB] dbaseOpen('%s') - fopen FAILED\n", filePath);
        return nullptr;
    }

    DBase* dbase = (DBase*)malloc(sizeof(*dbase));
    if (dbase == nullptr) {
        fclose(stream);
        return nullptr;
    }

    memset(dbase, 0, sizeof(*dbase));

    // Get file size.
    int fileSize = getFileSize(stream);
    fprintf(stderr, "[DB] dbaseOpen('%s') size=%d\n", filePath, fileSize);

    // --- Fallout 1 detection ---
    // F1 DATs start with a big-endian count of root directories.
    // F2/FISSION DATs start with "DAT\x1A" (0x4441541A as BE), which is
    // far above the plausible root-count range, so this check skips them.
    if (fileSize >= 16) {
        unsigned char sig[4];
        if (fseek(stream, 0, SEEK_SET) == 0 && fread(sig, 1, 4, stream) == 4) {
            unsigned int rootCount = ((unsigned int)sig[0] << 24)
                                   | ((unsigned int)sig[1] << 16)
                                   | ((unsigned int)sig[2] << 8)
                                   |  (unsigned int)sig[3];

            fprintf(stderr, "[DB] first4=%02X %02X %02X %02X  BE_rootCount=%u\n",
                    sig[0], sig[1], sig[2], sig[3], rootCount);

            if (rootCount > 0 && rootCount < 10000) {
                if (fseek(stream, 0, SEEK_SET) == 0) {
                    fprintf(stderr, "[DB] attempting F1 parse\n");
                    DBase* f1 = dbaseOpenFallout1(stream, fileSize, filePath);
                    if (f1 != nullptr) {
                        fprintf(stderr, "[DB] F1 parse succeeded\n");
                        fclose(stream);
                        return f1;
                    }
                    fprintf(stderr, "[DB] F1 parse failed, falling through to F2\n");
                }
                if (fseek(stream, 0, SEEK_SET) != 0) {
                    goto err;
                }
            }
        }
    }
    // --- end Fallout 1 detection ---

    // Reposition stream to read footer, which contains two 32-bit ints.
    if (fseek(stream, fileSize - sizeof(int) * 2, SEEK_SET) != 0) {
        goto err;
    }

    // Read the size of entries table.
    int entriesDataSize;
    if (fread(&entriesDataSize, sizeof(entriesDataSize), 1, stream) != 1) {
        goto err;
    }

    // Read the size of entire dbase content.
    int dbaseDataSize;
    if (fread(&dbaseDataSize, sizeof(dbaseDataSize), 1, stream) != 1) {
        goto err;
    }

    // Reposition stream to the beginning of the entries table.
    if (fseek(stream, fileSize - entriesDataSize - sizeof(int) * 2, SEEK_SET) != 0) {
        goto err;
    }

    if (fread(&(dbase->entriesLength), sizeof(dbase->entriesLength), 1, stream) != 1) {
        goto err;
    }

    dbase->entries = (DBaseEntry*)malloc(sizeof(*dbase->entries) * dbase->entriesLength);
    if (dbase->entries == nullptr) {
        goto err;
    }

    memset(dbase->entries, 0, sizeof(*dbase->entries) * dbase->entriesLength);

    // Read entries one by one, stopping on any error.
    int entryIndex;
    for (entryIndex = 0; entryIndex < dbase->entriesLength; entryIndex++) {
        DBaseEntry* entry = &(dbase->entries[entryIndex]);

        int pathLength;
        if (fread(&pathLength, sizeof(pathLength), 1, stream) != 1) {
            break;
        }

        entry->path = (char*)malloc(pathLength + 1);
        if (entry->path == nullptr) {
            break;
        }

        if (fread(entry->path, pathLength, 1, stream) != 1) {
            break;
        }

        entry->path[pathLength] = '\0';

        if (fread(&(entry->compressed), sizeof(entry->compressed), 1, stream) != 1) {
            break;
        }

        if (fread(&(entry->uncompressedSize), sizeof(entry->uncompressedSize), 1, stream) != 1) {
            break;
        }

        if (fread(&(entry->dataSize), sizeof(entry->dataSize), 1, stream) != 1) {
            break;
        }

        if (fread(&(entry->dataOffset), sizeof(entry->dataOffset), 1, stream) != 1) {
            break;
        }

        entry->compressionType = (entry->compressed == 1) ? 1 : 0;
    }

    if (entryIndex < dbase->entriesLength) {
        goto err;
    }

    dbase->path = compat_strdup(filePath);
    dbase->dataOffset = fileSize - dbaseDataSize;

    fprintf(stderr, "[DB] F2 parse succeeded: %d entries\n", dbase->entriesLength);

    fclose(stream);
    return dbase;

err:
    dbaseClose(dbase);
    fclose(stream);
    return nullptr;
}

// Closes [dbase], all open file handles, frees all associated resources,
// including the [dbase] itself.
//
// 0x4E5270
bool dbaseClose(DBase* dbase)
{
    assert(dbase); // "dbase", "dfile.c", 173

    DFile* curr = dbase->dfileHead;
    while (curr != nullptr) {
        DFile* next = curr->next;
        dfileClose(curr);
        curr = next;
    }

    if (dbase->entries != nullptr) {
        for (int index = 0; index < dbase->entriesLength; index++) {
            DBaseEntry* entry = &(dbase->entries[index]);
            char* entryName = entry->path;
            if (entryName != nullptr) {
                free(entryName);
            }
        }
        free(dbase->entries);
    }

    if (dbase->path != nullptr) {
        free(dbase->path);
    }

    memset(dbase, 0, sizeof(*dbase));

    free(dbase);

    return true;
}

// Custom pattern matching function for .dat files with case-insensitive matching
bool datPatternMatch(const char* pattern, const char* path)
{
    char normalizedPattern[COMPAT_MAX_PATH];
    char normalizedPath[COMPAT_MAX_PATH];

    strcpy(normalizedPattern, pattern);
    strcpy(normalizedPath, path);

    for (char* p = normalizedPath; *p != '\0'; p++) {
        if (*p == '\\')
            *p = '/';
        // Convert to lowercase for case-insensitive matching
        *p = tolower(*p);
    }

    return fpattern_match(normalizedPattern, normalizedPath);
}

// 0x4E5308
bool dbaseFindFirstEntry(DBase* dbase, DFileFindData* findFileData, const char* pattern)
{
    // Check if this is a new pattern search
    static int searchCount = 0;
    static char lastPattern[COMPAT_MAX_PATH] = "";

    if (strcmp(pattern, lastPattern) != 0) {
        searchCount++;
        strcpy(lastPattern, pattern);
    }

    for (int index = 0; index < dbase->entriesLength; index++) {
        DBaseEntry* entry = &(dbase->entries[index]);

        if (datPatternMatch(pattern, entry->path)) {
            // Store the first match for the findFileData
            strcpy(findFileData->fileName, entry->path);
            strcpy(findFileData->pattern, pattern);
            findFileData->index = index;
            return true; // Return immediately on first match
        }
    }

    return false;
}

bool dbaseFindNextEntry(DBase* dbase, DFileFindData* findFileData)
{
    for (int index = findFileData->index + 1; index < dbase->entriesLength; index++) {
        DBaseEntry* entry = &(dbase->entries[index]);

        if (datPatternMatch(findFileData->pattern, entry->path)) {
            strcpy(findFileData->fileName, entry->path);
            findFileData->index = index;
            return true;
        }
    }

    return false;
}

// 0x4E541C
bool dbaseFindClose(DBase* dbase, DFileFindData* findFileData)
{
    return true;
}

// [filelength].
//
// 0x4E5424
long dfileGetSize(DFile* stream)
{
    return stream->entry->uncompressedSize;
}

// [fclose].
//
// 0x4E542C
int dfileClose(DFile* stream)
{
    assert(stream); // "stream", "dfile.c", 253

    int rc = 0;

    if (stream->entry->compressed == 1 && stream->entry->compressionType == 0 && stream->decompressionStream != nullptr) {
        if (inflateEnd(stream->decompressionStream) != Z_OK) {
            rc = -1;
        }
    }

    if (stream->decompressionStream != nullptr) {
        free(stream->decompressionStream);
    }

    if (stream->decompressionBuffer != nullptr) {
        free(stream->decompressionBuffer);
    }

    if (stream->stream != nullptr) {
        fclose(stream->stream);
    }

    // Loop thru open file handles and find previous to remove current handle
    // from linked list.
    //
    // NOTE: Compiled code is slightly different.
    DFile* curr = stream->dbase->dfileHead;
    DFile* prev = nullptr;
    while (curr != nullptr) {
        if (curr == stream) {
            break;
        }

        prev = curr;
        curr = curr->next;
    }

    if (curr != nullptr) {
        if (prev == nullptr) {
            stream->dbase->dfileHead = stream->next;
        } else {
            prev->next = stream->next;
        }
    }

    memset(stream, 0, sizeof(*stream));

    free(stream);

    return rc;
}

// [fopen].
//
// 0x4E5504
DFile* dfileOpen(DBase* dbase, const char* filePath, const char* mode)
{
    assert(dbase); // dfile.c, 295
    assert(filePath); // dfile.c, 296
    assert(mode); // dfile.c, 297

    // NORMALIZE PATH FOR .DAT ACCESS - CRITICAL FIX
    char* normalizedPath = normalizePathForDat(filePath);
    if (normalizedPath == nullptr) {
        return nullptr;
    }

    DFile* result = dfileOpenInternal(dbase, normalizedPath, mode, nullptr);

    free(normalizedPath);
    return result;
}

// [vfprintf].
//
// 0x4E56C0
int dfilePrintFormattedArgs(DFile* stream, const char* format, va_list args)
{
    assert(stream); // "stream", "dfile.c", 368
    assert(format); // "format", "dfile.c", 369

    return -1;
}

// [fgetc].
//
// This function reports \r\n sequence as one character \n, even though it
// consumes two characters from the underlying stream.
//
// 0x4E5700
int dfileReadChar(DFile* stream)
{
    assert(stream); // "stream", "dfile.c", 384

    if ((stream->flags & DFILE_EOF) != 0 || (stream->flags & DFILE_ERROR) != 0) {
        return -1;
    }

    if ((stream->flags & DFILE_HAS_UNGETC) != 0) {
        stream->flags &= ~DFILE_HAS_UNGETC;
        return stream->ungotten;
    }

    int ch = dfileReadCharInternal(stream);
    if (ch == -1) {
        stream->flags |= DFILE_EOF;
    }

    return ch;
}

// [fgets].
//
// Both Windows (\r\n) and Unix (\n) line endings are recognized. Windows
// line ending is reported as \n.
//
// 0x4E5764
char* dfileReadString(char* string, int size, DFile* stream)
{
    assert(string); // "s", "dfile.c", 407
    assert(size); // "n", "dfile.c", 408
    assert(stream); // "stream", "dfile.c", 409

    if ((stream->flags & DFILE_EOF) != 0 || (stream->flags & DFILE_ERROR) != 0) {
        return nullptr;
    }

    char* pch = string;

    if ((stream->flags & DFILE_HAS_UNGETC) != 0) {
        *pch++ = stream->ungotten & 0xFF;
        size--;
        stream->flags &= ~DFILE_HAS_UNGETC;
    }

    // Read up to size - 1 characters one by one saving space for the null
    // terminator.
    for (int index = 0; index < size - 1; index++) {
        int ch = dfileReadCharInternal(stream);
        if (ch == -1) {
            break;
        }

        *pch++ = ch & 0xFF;

        if (ch == '\n') {
            break;
        }
    }

    if (pch == string) {
        // No character was set into the buffer.
        return nullptr;
    }

    *pch = '\0';

    return string;
}

// [fputc].
//
// 0x4E5830
int dfileWriteChar(int ch, DFile* stream)
{
    assert(stream); // "stream", "dfile.c", 437

    return -1;
}

// [fputs].
//
// 0x4E5854
int dfileWriteString(const char* string, DFile* stream)
{
    assert(string); // "s", "dfile.c", 448
    assert(stream); // "stream", "dfile.c", 449

    return -1;
}

// [fread].
//
// 0x4E58FC
size_t dfileRead(void* ptr, size_t size, size_t count, DFile* stream)
{
    assert(ptr); // "ptr", "dfile.c", 499
    assert(stream); // "stream", dfile.c, 500

    if ((stream->flags & DFILE_EOF) != 0 || (stream->flags & DFILE_ERROR) != 0) {
        return 0;
    }

    size_t remainingSize = stream->entry->uncompressedSize - stream->position;
    if ((stream->flags & DFILE_HAS_UNGETC) != 0) {
        remainingSize++;
    }

    size_t bytesToRead = size * count;
    if (remainingSize < bytesToRead) {
        bytesToRead = remainingSize;
        stream->flags |= DFILE_EOF;
    }

    size_t extraBytesRead = 0;
    if ((stream->flags & DFILE_HAS_UNGETC) != 0) {
        unsigned char* byteBuffer = (unsigned char*)ptr;
        *byteBuffer++ = stream->ungotten & 0xFF;
        ptr = byteBuffer;

        bytesToRead--;

        stream->flags &= ~DFILE_HAS_UNGETC;
        extraBytesRead = 1;
    }

    size_t bytesRead;
    if ((stream->flags & DFILE_PREDECODED) != 0) {
        if (bytesToRead != 0) {
            long src = stream->position + extraBytesRead;
            memcpy(ptr, stream->decompressionBuffer + src, bytesToRead);
        }
        bytesRead = bytesToRead + extraBytesRead;
        stream->position += bytesRead;
    } else if (stream->entry->compressed == 1) {
        if (!dfileReadCompressed(stream, ptr, bytesToRead)) {
            stream->flags |= DFILE_ERROR;
            return false;
        }
        bytesRead = bytesToRead;
    } else {
        bytesRead = fread(ptr, 1, bytesToRead, stream->stream) + extraBytesRead;
        stream->position += bytesRead;
    }

    return bytesRead / size;
}

// [fwrite].
//
// 0x4E59F8
size_t dfileWrite(const void* ptr, size_t size, size_t count, DFile* stream)
{
    assert(ptr); // "ptr", "dfile.c", 538
    assert(stream); // "stream", "dfile.c", 539

    return count - 1;
}

// [fseek].
//
// 0x4E5A74
int dfileSeek(DFile* stream, long offset, int origin)
{
    assert(stream); // "stream", "dfile.c", 569

    if ((stream->flags & DFILE_ERROR) != 0) {
        return 1;
    }

    if ((stream->flags & DFILE_PREDECODED) != 0) {
        long offsetFromBeginning;
        switch (origin) {
        case SEEK_SET: offsetFromBeginning = offset; break;
        case SEEK_CUR: offsetFromBeginning = stream->position + offset; break;
        case SEEK_END: offsetFromBeginning = stream->entry->uncompressedSize + offset; break;
        default: return 1;
        }

        if (offsetFromBeginning < 0 || offsetFromBeginning >= stream->entry->uncompressedSize) {
            return 1;
        }

        stream->position = offsetFromBeginning;
        stream->flags &= ~(DFILE_HAS_UNGETC | DFILE_EOF);
        return 0;
    }

    if ((stream->flags & DFILE_TEXT) != 0) {
        if (offset != 0 && origin != SEEK_SET) {
            // NOTE: For unknown reason this function does not allow arbitrary
            // seeks in text streams, whether compressed or not. It only
            // supports rewinding. Probably because of reading functions which
            // handle \r\n sequence as \n.
            return 1;
        }
    }

    long offsetFromBeginning;
    switch (origin) {
    case SEEK_SET:
        offsetFromBeginning = offset;
        break;
    case SEEK_CUR:
        offsetFromBeginning = stream->position + offset;
        break;
    case SEEK_END:
        offsetFromBeginning = stream->entry->uncompressedSize + offset;
        break;
    default:
        return 1;
    }

    if (offsetFromBeginning >= stream->entry->uncompressedSize) {
        return 1;
    }

    long pos = stream->position;
    if (offsetFromBeginning == pos) {
        stream->flags &= ~(DFILE_HAS_UNGETC | DFILE_EOF);
        return 0;
    }

    if (offsetFromBeginning != 0) {
        if (stream->entry->compressed == 1) {
            if (offsetFromBeginning < pos) {
                // We cannot go backwards in compressed stream, so the only way
                // is to start from the beginning.
                dfileRewind(stream);
            }

            // Consume characters one by one until we reach specified offset.
            while (offsetFromBeginning > stream->position) {
                if (dfileReadCharInternal(stream) == -1) {
                    return 1;
                }
            }
        } else {
            if (fseek(stream->stream, offsetFromBeginning - pos, SEEK_CUR) != 0) {
                stream->flags |= DFILE_ERROR;
                return 1;
            }

            // FIXME: I'm not sure what this assignment means. This field is
            // only meaningful when reading compressed streams.
            stream->compressedBytesRead = offsetFromBeginning;
        }

        stream->flags &= ~(DFILE_HAS_UNGETC | DFILE_EOF);
        return 0;
    }

    if (fseek(stream->stream, stream->dbase->dataOffset + stream->entry->dataOffset, SEEK_SET) != 0) {
        stream->flags |= DFILE_ERROR;
        return 1;
    }

    if (stream->entry->compressed == 1) {
        if (inflateEnd(stream->decompressionStream) != Z_OK) {
            stream->flags |= DFILE_ERROR;
            return 1;
        }

        stream->decompressionStream->zalloc = Z_NULL;
        stream->decompressionStream->zfree = Z_NULL;
        stream->decompressionStream->opaque = Z_NULL;
        stream->decompressionStream->next_in = stream->decompressionBuffer;
        stream->decompressionStream->avail_in = 0;

        if (inflateInit(stream->decompressionStream) != Z_OK) {
            stream->flags |= DFILE_ERROR;
            return 1;
        }
    } else {
        // FIXME: I'm not sure what this assignment means. This field is
        // only meaningful when reading compressed streams.
        stream->compressedBytesRead = 0;
    }

    stream->position = 0;
    stream->compressedBytesRead = 0;
    stream->flags &= ~(DFILE_HAS_UNGETC | DFILE_EOF);

    return 0;
}

// [ftell].
//
// 0x4E5C88
long dfileTell(DFile* stream)
{
    assert(stream); // "stream", "dfile.c", 654

    return stream->position;
}

// [rewind].
//
// 0x4E5CB0
void dfileRewind(DFile* stream)
{
    assert(stream); // "stream", "dfile.c", 664

    dfileSeek(stream, 0, SEEK_SET);

    stream->flags &= ~DFILE_ERROR;
}

// [feof].
//
// 0x4E5D10
int dfileEof(DFile* stream)
{
    assert(stream); // "stream", "dfile.c", 685

    return stream->flags & DFILE_EOF;
}

// The [bsearch] comparison callback, which is used to find [DBaseEntry] for
// specified [filePath].
//
// 0x4E5D70
static int dbaseFindEntryByFilePath(const void* file, const void* entryName)
{
    const char* filePath = (const char*)file;
    DBaseEntry* entry = (DBaseEntry*)entryName;

    return compat_stricmp(filePath, entry->path);
}

// 0x4E5D9C
static DFile* dfileOpenInternal(DBase* dbase, const char* filePath, const char* mode, DFile* dfile)
{
    DBaseEntry* entry = (DBaseEntry*)bsearch(filePath, dbase->entries, dbase->entriesLength, sizeof(*dbase->entries), dbaseFindEntryByFilePath);
    if (entry == nullptr) {
        goto err;
    }

    if (mode[0] != 'r') {
        goto err;
    }

    if (dfile == nullptr) {
        dfile = (DFile*)malloc(sizeof(*dfile));
        if (dfile == nullptr) {
            return nullptr;
        }

        memset(dfile, 0, sizeof(*dfile));
        dfile->dbase = dbase;
        dfile->next = dbase->dfileHead;
        dbase->dfileHead = dfile;
    } else {
        if (dbase != dfile->dbase) {
            goto err;
        }

        if (dfile->stream != nullptr) {
            fclose(dfile->stream);
            dfile->stream = nullptr;
        }

        dfile->compressedBytesRead = 0;
        dfile->position = 0;
        dfile->flags = 0;
    }

    dfile->entry = entry;

    // Open stream to .DAT file.
    dfile->stream = compat_fopen(dbase->path, "rb");
    if (dfile->stream == nullptr) {
        goto err;
    }

    // Relocate stream to the beginning of data for specified entry.
    if (fseek(dfile->stream, dbase->dataOffset + entry->dataOffset, SEEK_SET) != 0) {
        goto err;
    }

    if (entry->compressed == 1 && entry->compressionType == 1) {
        // ---- ZLIB (Fallout 2 / FISSION format) ----
        if (dfile->decompressionStream == nullptr) {
            dfile->decompressionStream = (z_streamp)malloc(sizeof(*dfile->decompressionStream));
            if (dfile->decompressionStream == nullptr) {
                goto err;
            }

            dfile->decompressionBuffer = (unsigned char*)malloc(DFILE_DECOMPRESSION_BUFFER_SIZE);
            if (dfile->decompressionBuffer == nullptr) {
                goto err;
            }
        }

        dfile->decompressionStream->zalloc = Z_NULL;
        dfile->decompressionStream->zfree = Z_NULL;
        dfile->decompressionStream->opaque = Z_NULL;
        dfile->decompressionStream->next_in = dfile->decompressionBuffer;
        dfile->decompressionStream->avail_in = 0;

        if (inflateInit(dfile->decompressionStream) != Z_OK) {
            goto err;
        }
    } else if (entry->compressionType == 2 || entry->compressionType == 3) {
        // ---- LZSS (Fallout 1) ----
        // Free any zlib resources from a previous use of this DFile.
        if (dfile->decompressionStream != nullptr) {
            free(dfile->decompressionStream);
            dfile->decompressionStream = nullptr;
        }
        if (dfile->decompressionBuffer != nullptr) {
            free(dfile->decompressionBuffer);
            dfile->decompressionBuffer = nullptr;
        }

        dfile->decompressionBuffer = (unsigned char*)malloc(entry->uncompressedSize);
        if (dfile->decompressionBuffer == nullptr) {
            goto err;
        }

        if (entry->compressionType == 2) {
            int decoded = f1LzssDecode(dfile->stream, dfile->decompressionBuffer, entry->dataSize);
            if (decoded != entry->uncompressedSize) {
                fprintf(stderr, "[DFO] LZSS short: '%s' got %d, want %d\n",
                        entry->path, decoded, entry->uncompressedSize);
                goto err;
            }
        } else {
            unsigned char* out = dfile->decompressionBuffer;
            int remaining = entry->uncompressedSize;

            while (remaining > 0) {
                unsigned char hdr[2];
                if (fread(hdr, 1, 2, dfile->stream) != 2) goto err;
                unsigned int h = ((unsigned int)hdr[0] << 8) | hdr[1];

                if (h & 0x8000) {
                    unsigned int rawLen = h & 0x7FFF;
                    if ((int)rawLen > remaining) rawLen = (unsigned int)remaining;
                    if (fread(out, 1, rawLen, dfile->stream) != rawLen) goto err;
                    out += rawLen;
                    remaining -= (int)rawLen;
                } else {
                    if (h == 0) goto err;
                    int decoded = f1LzssDecode(dfile->stream, out, h);
                    if (decoded <= 0 || decoded > remaining) goto err;
                    out += decoded;
                    remaining -= decoded;
                }
            }
        }

        dfile->flags |= DFILE_PREDECODED;
    } else {
        // ---- UNCOMPRESSED ----
        if (dfile->decompressionStream != nullptr) {
            free(dfile->decompressionStream);
            dfile->decompressionStream = nullptr;
        }
        if (dfile->decompressionBuffer != nullptr) {
            free(dfile->decompressionBuffer);
            dfile->decompressionBuffer = nullptr;
        }
    }

    if (mode[1] == 't') {
        dfile->flags |= DFILE_TEXT;
    }

    return dfile;

err:
    if (dfile != nullptr) {
        dfileClose(dfile);
    }

    return nullptr;
}

// 0x4E5F9C
static int dfileReadCharInternal(DFile* stream)
{
    // PRE-DECODED (F1 LZSS) path — must come first.
    if ((stream->flags & DFILE_PREDECODED) != 0) {
        if (stream->position >= stream->entry->uncompressedSize) {
            return -1;
        }

        int ch = stream->decompressionBuffer[stream->position];
        stream->position++;

        if ((stream->flags & DFILE_TEXT) != 0 && ch == '\r') {
            if (stream->position < stream->entry->uncompressedSize
                && stream->decompressionBuffer[stream->position] == '\n') {
                ch = '\n';
                stream->position++;
            }
        }

        return ch;
    }

    // ZLIB (F2) path.
    if (stream->entry->compressed == 1) {
        char ch;
        if (!dfileReadCompressed(stream, &ch, sizeof(ch))) {
            return -1;
        }

        if ((stream->flags & DFILE_TEXT) != 0) {
            if (ch == '\r') {
                char nextCh;
                if (dfileReadCompressed(stream, &nextCh, sizeof(nextCh))) {
                    if (nextCh == '\n') {
                        ch = nextCh;
                    } else {
                        dfileUngetCompressed(stream, nextCh & 0xFF);
                    }
                }
            }
        }

        return ch & 0xFF;
    }

    // RAW path.
    if (stream->position >= stream->entry->uncompressedSize) {
        return -1;
    }

    int ch = fgetc(stream->stream);
    if (ch != -1) {
        if ((stream->flags & DFILE_TEXT) != 0) {
            if (ch == '\r') {
                if (stream->position + 1 < stream->entry->uncompressedSize) {
                    int nextCh = fgetc(stream->stream);
                    if (nextCh == '\n') {
                        ch = nextCh;
                        stream->position++;
                    } else {
                        ungetc(nextCh, stream->stream);
                    }
                }
            }
        }

        stream->position++;
    }

    return ch;
}

// 0x4E6078
static bool dfileReadCompressed(DFile* stream, void* ptr, size_t size)
{
    if ((stream->flags & DFILE_HAS_COMPRESSED_UNGETC) != 0) {
        unsigned char* byteBuffer = (unsigned char*)ptr;
        *byteBuffer++ = stream->compressedUngotten & 0xFF;
        ptr = byteBuffer;

        size--;

        stream->flags &= ~DFILE_HAS_COMPRESSED_UNGETC;
        stream->position++;

        if (size == 0) {
            return true;
        }
    }

    stream->decompressionStream->next_out = (Bytef*)ptr;
    stream->decompressionStream->avail_out = size;

    do {
        if (stream->decompressionStream->avail_out == 0) {
            // Everything was decompressed.
            break;
        }

        if (stream->decompressionStream->avail_in == 0) {
            // No more unprocessed data, request next chunk.
            size_t bytesToRead = std::min(DFILE_DECOMPRESSION_BUFFER_SIZE, stream->entry->dataSize - stream->compressedBytesRead);

            if (fread(stream->decompressionBuffer, bytesToRead, 1, stream->stream) != 1) {
                break;
            }

            stream->decompressionStream->avail_in = bytesToRead;
            stream->decompressionStream->next_in = stream->decompressionBuffer;

            stream->compressedBytesRead += bytesToRead;
        }
    } while (inflate(stream->decompressionStream, Z_NO_FLUSH) == Z_OK);

    if (stream->decompressionStream->avail_out != 0) {
        // There are some data still waiting, which means there was in error
        // during decompression loop above.
        return false;
    }

    stream->position += size;

    return true;
}

// NOTE: Inlined.
//
// 0x4E613C
static void dfileUngetCompressed(DFile* stream, int ch)
{
    stream->compressedUngotten = ch;
    stream->flags |= DFILE_HAS_COMPRESSED_UNGETC;
    stream->position--;
}

} // namespace fallout
