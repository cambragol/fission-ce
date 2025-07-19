#include "art.h"

#include <stdio.h>

#include "db.h" // for fileOpen, fileClose
#include "xfile.h" // for File type
#define DIR_SEPARATOR '/'

#include <stdlib.h>
#include <string.h>

#include "animation.h"
#include "debug.h"
#include "draw.h"
#include "game.h"
#include "memory.h"
#include "object.h"
#include "proto.h"
#include "settings.h"
#include "sfall_config.h"

namespace fallout {

typedef struct ArtListDescription {
    int flags;
    char name[16];
    char* fileNames; // dynamic array of null terminated strings FILENAME_LENGTH bytes long each
    void* field_18;
    int fileNamesLength; // number of entries in list
} ArtListDescription;

typedef struct HeadDescription {
    int goodFidgetCount;
    int neutralFidgetCount;
    int badFidgetCount;
} HeadDescription;

static int artReadList(const char* path, char** out_arr, int* out_count);
static int artCacheGetFileSizeImpl(int fid, int* out_size);
static int artCacheReadDataImpl(int fid, int* sizePtr, unsigned char* data);
static void artCacheFreeImpl(void* ptr);
static int artReadFrameData(unsigned char* data, File* stream, int count, int* paddingPtr);
static int artReadHeader(Art* art, File* stream);
static int artGetDataSize(Art* art);
static int paddingForSize(int size);

// 0x5002D8
static char gDefaultJumpsuitMaleFileName[] = "hmjmps";

// 0x05002E0
static char gDefaultJumpsuitFemaleFileName[] = "hfjmps";

// 0x5002E8
static char gDefaultTribalMaleFileName[] = "hmwarr";

// 0x5002F0
static char gDefaultTribalFemaleFileName[] = "hfprim";

// 0x510738
static ArtListDescription gArtListDescriptions[OBJ_TYPE_COUNT] = {
    { 0, "items", nullptr, nullptr, 0 },
    { 0, "critters", nullptr, nullptr, 0 },
    { 0, "scenery", nullptr, nullptr, 0 },
    { 0, "walls", nullptr, nullptr, 0 },
    { 0, "tiles", nullptr, nullptr, 0 },
    { 0, "misc", nullptr, nullptr, 0 },
    { 0, "intrface", nullptr, nullptr, 0 },
    { 0, "inven", nullptr, nullptr, 0 },
    { 0, "heads", nullptr, nullptr, 0 },
    { 0, "backgrnd", nullptr, nullptr, 0 },
    { 0, "skilldex", nullptr, nullptr, 0 },
};

// This flag denotes that localized arts should be looked up first. Used
// together with [gArtLanguage].
//
// 0x510898
static bool gArtLanguageInitialized = false;

// 0x51089C
static const char* _head1 = "gggnnnbbbgnb";

// 0x5108A0
static const char* _head2 = "vfngfbnfvppp";

// Current native look base fid.
//
// 0x5108A4
int _art_vault_guy_num = 0;

// Base fids for unarmored dude.
//
// Outfit file names:
// - tribal: "hmwarr", "hfprim"
// - jumpsuit: "hmjmps", "hfjmps"
//
// NOTE: This value could have been done with two separate arrays - one for
// tribal look, and one for jumpsuit look. However in this case it would have
// been accessed differently in 0x49F984, which clearly uses look type as an
// index, not gender.
//
// 0x5108A8
int _art_vault_person_nums[DUDE_NATIVE_LOOK_COUNT][GENDER_COUNT];

// Index of "grid001.frm" in tiles.lst.
//
// 0x5108B8
static int _art_mapper_blank_tile = 1;

// Non-english language name.
//
// This value is used as a directory name to display localized arts.
//
// 0x56C970
static char gArtLanguage[32];

// 0x56C990
Cache gArtCache;

// 0x56C9E4
static char _art_name[COMPAT_MAX_PATH];

// head_info
// 0x56CAE8
static HeadDescription* gHeadDescriptions;

// anon_alias
// 0x56CAEC
static int* _anon_alias;

// artCritterFidShouldRunData
// 0x56CAF0
static int* gArtCritterFidShoudRunData;

// Number of “core” entries per objectType, before we append variants
static int gArtOriginalCount[OBJ_TYPE_COUNT] = { 0 };

int artGetFidWithVariant(int objectType, int baseId, const char* suffix, bool useVariant)
{
    if (useVariant) {
        int variantId = artFindVariant(objectType, baseId, suffix);
        if (variantId >= 0) {
            return buildFid(objectType, variantId, 0, 0, 0);
        }
    }
    return buildFid(objectType, baseId, 0, 0, 0);
}

// Collect all File* streams for a given relative path using VFS fileOpen
static int collectAllCopies(const char* relativePath, File*** outStreams)
{
    File** streams = NULL;
    int count = 0;
    File* f = fileOpen(relativePath, "rt");
    if (f) {
        streams = (File**)internal_realloc(streams, sizeof(File*) * (count + 1));
        streams[count++] = f;
    }
    *outStreams = streams;
    return count;
}

// Read a single .lst stream into flat FILENAME_LENGTH-byte entries
static int readListFromStream(File* stream, char** outNames)
{
    char buffer[256];
    char* names = nullptr;
    int count = 0;

    while (fileReadString(buffer, sizeof(buffer), stream)) {
        char* end = strpbrk(buffer, " ,;\r\t\n");
        if (end)
            *end = '\0';

        names = (char*)internal_realloc(names, (count + 1) * FILENAME_LENGTH);
        strncpy(names + count * FILENAME_LENGTH, buffer, FILENAME_LENGTH - 1);
        names[count * FILENAME_LENGTH + FILENAME_LENGTH - 1] = '\0';
        count++;
    }

    *outNames = names;
    return count;
}

// Merge multiple layers of flat‑name lists by highest‑priority last
static void mergeArtLayers(char** outNames, int* outCount, char* layerNames[], int layerCounts[], int layers)
{
    int maxLen = 0;
    for (int i = 0; i < layers; i++) {
        if (layerCounts[i] > maxLen)
            maxLen = layerCounts[i];
    }

    char* merged = (char*)internal_malloc(maxLen * FILENAME_LENGTH);
    int mCount = maxLen;

    for (int idx = 0; idx < maxLen; idx++) {
        char* slot = merged + idx * FILENAME_LENGTH;
        slot[0] = '\0'; // Initialize as empty

        // Highest priority last (override order)
        for (int lyr = layers - 1; lyr >= 0; lyr--) {
            if (idx < layerCounts[lyr]) {
                char* entry = layerNames[lyr] + idx * FILENAME_LENGTH;
                if (entry[0] != '\0') {
                    strncpy(slot, entry, FILENAME_LENGTH - 1);
                    slot[FILENAME_LENGTH - 1] = '\0';
                    break;
                }
            }
        }
    }

    *outNames = merged;
    *outCount = mCount;
}

int artFindVariant(int objectType, int baseIndex, const char* suffix)
{
    if (objectType < 0 || objectType >= OBJ_TYPE_COUNT)
        return -1;

    ArtListDescription* desc = &gArtListDescriptions[objectType];
    if (baseIndex < 0 || baseIndex >= desc->fileNamesLength)
        return -1;

    // Get base filename
    const char* baseName = desc->fileNames + baseIndex * FILENAME_LENGTH;

    // Extract base without extension
    char base[FILENAME_LENGTH];
    strncpy(base, baseName, FILENAME_LENGTH - 1);
    base[FILENAME_LENGTH - 1] = '\0';

    char* ext = strrchr(base, '.');
    if (ext && compat_stricmp(ext, ".frm") == 0) {
        *ext = '\0'; // Remove extension
    }

    // Build expected variant name
    char expected[FILENAME_LENGTH];
    int len = snprintf(expected, sizeof(expected), "%s%s.frm", base, suffix);
    if (len >= FILENAME_LENGTH) {
        debugPrint("Variant name too long: %s%s", base, suffix);
        return -1;
    }

    // Search variant section
    for (int i = gArtOriginalCount[objectType]; i < desc->fileNamesLength; i++) {
        const char* candidate = desc->fileNames + i * FILENAME_LENGTH;
        if (compat_stricmp(candidate, expected) == 0) {
            return i;
        }
    }

    return -1;
}

// 0x418840
int artInit()
{
    char path[COMPAT_MAX_PATH];
    File* stream;
    char string[200];

    // Initialize art cache
    int cacheSize = settings.system.art_cache_size;
    if (!cacheInit(&gArtCache,
            artCacheGetFileSizeImpl,
            artCacheReadDataImpl,
            artCacheFreeImpl,
            cacheSize << 20)) {
        debugPrint("cache_init failed in art_init\n");
        return -1;
    }

    // Language override
    const char* language = settings.system.language.c_str();
    if (compat_stricmp(language, ENGLISH) != 0) {
        strcpy(gArtLanguage, language);
        gArtLanguageInitialized = true;
    }

    // Process each object type
    for (int objectType = 0; objectType < OBJ_TYPE_COUNT; objectType++) {
        ArtListDescription* desc = &gArtListDescriptions[objectType];
        desc->flags = 0;

        // Build the .lst path: "art/<name>/<name>.lst"
        snprintf(path, sizeof(path),
            "art%c%s%c%s.lst",
            DIR_SEPARATOR,
            desc->name,
            DIR_SEPARATOR,
            desc->name);

        // Collect VFS .lst streams
        File** streams = NULL;
        int layerCount = collectAllCopies(path, &streams);

        if (layerCount <= 1) {
            // Single or no layer: direct load fallback
            if (artReadList(path,
                    &desc->fileNames,
                    &desc->fileNamesLength)
                != 0) {
                debugPrint("art_read_lst fallback failed for %s\n", path);
                // Cleanup and return failure
                if (streams) {
                    if (layerCount == 1)
                        fileClose(streams[0]);
                    internal_free(streams);
                }
                cacheFree(&gArtCache);
                return -1;
            }
            gArtOriginalCount[objectType] = desc->fileNamesLength;

            if (streams) {
                if (layerCount == 1)
                    fileClose(streams[0]);
                internal_free(streams);
            }
        } else {
            // Multiple layers: read each, then merge
            if (layerCount <= 0) {
                debugPrint("art_read_lst failed: no layers for %s\n", path);
                internal_free(streams);
                cacheFree(&gArtCache);
                return -1;
            }

            int useCount = layerCount > 16 ? 16 : layerCount;
            char* layerNames[16];
            int layerCounts[16];

            for (int i = 0; i < useCount; i++) {
                layerCounts[i] = readListFromStream(streams[i], &layerNames[i]);
                fileClose(streams[i]);
            }
            internal_free(streams);

            char* mergedNames = NULL;
            int mergedCount = 0;
            mergeArtLayers(&mergedNames, &mergedCount, layerNames, layerCounts, useCount);

            gArtOriginalCount[objectType] = mergedCount;
            desc->fileNames = mergedNames;
            desc->fileNamesLength = mergedCount;

            for (int i = 0; i < useCount; i++) {
                internal_free(layerNames[i]);
            }
        }

        // Append *_800.frm variants
        const char* suffix = "_800.frm";
        size_t suffixLen = strlen(suffix);

        // Build the VFS pattern
        char pattern[COMPAT_MAX_PATH];
        snprintf(pattern, sizeof(pattern),
            "art%c%s%c*.frm",
            DIR_SEPARATOR,
            desc->name,
            DIR_SEPARATOR);

        char** vfsFiles = NULL;
        int vfsCount = fileNameListInit(pattern, &vfsFiles, 0, 0);

        if (vfsCount > 0) {
            int origCount = gArtOriginalCount[objectType];
            int newCount = origCount;
            char* names = desc->fileNames;
            int currentSize = desc->fileNamesLength;

            for (int vi = 0; vi < vfsCount; vi++) {
                const char* fn = vfsFiles[vi];
                size_t len = strlen(fn);

                // Check if file matches variant pattern
                if (len <= suffixLen || compat_stricmp(fn + len - suffixLen, suffix) != 0) {
                    continue;
                }

                // Extract base name
                char base[FILENAME_LENGTH] = { 0 };
                size_t baseLen = len - suffixLen;
                if (baseLen >= FILENAME_LENGTH)
                    baseLen = FILENAME_LENGTH - 1;
                strncpy(base, fn, baseLen);
                base[baseLen] = '\0';

                // Find matching base entry
                for (int i = 0; i < origCount; i++) {
                    const char* slot = names + i * FILENAME_LENGTH;
                    if (compat_strnicmp(slot, base, baseLen) == 0) {
                        // Resize array if needed
                        if (newCount >= currentSize) {
                            currentSize += 10;
                            char* newNames = (char*)internal_realloc(names, currentSize * FILENAME_LENGTH);
                            if (!newNames)
                                break;
                            names = newNames;
                        }

                        // Add variant
                        char* dest = names + newCount * FILENAME_LENGTH;
                        strncpy(dest, fn, FILENAME_LENGTH - 1);
                        dest[FILENAME_LENGTH - 1] = '\0';
                        newCount++;
                        break;
                    }
                }
            }

            // Update if we added variants
            if (newCount > origCount) {
                desc->fileNames = names;
                desc->fileNamesLength = newCount;
            }

            fileNameListFree(&vfsFiles, vfsCount);
        }
    }

    _anon_alias = (int*)internal_malloc(sizeof(*_anon_alias) * gArtListDescriptions[OBJ_TYPE_CRITTER].fileNamesLength);
    if (_anon_alias == nullptr) {
        gArtListDescriptions[OBJ_TYPE_CRITTER].fileNamesLength = 0;
        debugPrint("Out of memory for anon_alias in art_init\n");
        cacheFree(&gArtCache);
        return -1;
    }

    gArtCritterFidShoudRunData = (int*)internal_malloc(sizeof(*gArtCritterFidShoudRunData) * gArtListDescriptions[1].fileNamesLength);
    if (gArtCritterFidShoudRunData == nullptr) {
        gArtListDescriptions[OBJ_TYPE_CRITTER].fileNamesLength = 0;
        debugPrint("Out of memory for artCritterFidShouldRunData in art_init\n");
        cacheFree(&gArtCache);
        return -1;
    }

    for (int critterIndex = 0; critterIndex < gArtListDescriptions[OBJ_TYPE_CRITTER].fileNamesLength; critterIndex++) {
        gArtCritterFidShoudRunData[critterIndex] = 0;
    }

    snprintf(path, sizeof(path), "%s%s%s\\%s.lst", _cd_path_base, "art\\", gArtListDescriptions[OBJ_TYPE_CRITTER].name, gArtListDescriptions[OBJ_TYPE_CRITTER].name);

    stream = fileOpen(path, "rt");
    if (stream == nullptr) {
        debugPrint("Unable to open %s in art_init\n", path);
        cacheFree(&gArtCache);
        return -1;
    }

    // SFALL: Modify player model settings.
    char* jumpsuitMaleFileName = nullptr;
    configGetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_MALE_KEY, &jumpsuitMaleFileName);
    if (jumpsuitMaleFileName == nullptr || jumpsuitMaleFileName[0] == '\0') {
        jumpsuitMaleFileName = gDefaultJumpsuitMaleFileName;
    }

    char* jumpsuitFemaleFileName = nullptr;
    configGetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE_KEY, &jumpsuitFemaleFileName);
    if (jumpsuitFemaleFileName == nullptr || jumpsuitFemaleFileName[0] == '\0') {
        jumpsuitFemaleFileName = gDefaultJumpsuitFemaleFileName;
    }

    char* tribalMaleFileName = nullptr;
    configGetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_MALE_KEY, &tribalMaleFileName);
    if (tribalMaleFileName == nullptr || tribalMaleFileName[0] == '\0') {
        tribalMaleFileName = gDefaultTribalMaleFileName;
    }

    char* tribalFemaleFileName = nullptr;
    configGetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_FEMALE_KEY, &tribalFemaleFileName);
    if (tribalFemaleFileName == nullptr || tribalFemaleFileName[0] == '\0') {
        tribalFemaleFileName = gDefaultTribalFemaleFileName;
    }

    char* critterFileNames = gArtListDescriptions[OBJ_TYPE_CRITTER].fileNames;
    for (int critterIndex = 0; critterIndex < gArtListDescriptions[OBJ_TYPE_CRITTER].fileNamesLength; critterIndex++) {
        if (compat_stricmp(critterFileNames, jumpsuitMaleFileName) == 0) {
            _art_vault_person_nums[DUDE_NATIVE_LOOK_JUMPSUIT][GENDER_MALE] = critterIndex;
        } else if (compat_stricmp(critterFileNames, jumpsuitFemaleFileName) == 0) {
            _art_vault_person_nums[DUDE_NATIVE_LOOK_JUMPSUIT][GENDER_FEMALE] = critterIndex;
        }

        if (compat_stricmp(critterFileNames, tribalMaleFileName) == 0) {
            _art_vault_person_nums[DUDE_NATIVE_LOOK_TRIBAL][GENDER_MALE] = critterIndex;
            _art_vault_guy_num = critterIndex;
        } else if (compat_stricmp(critterFileNames, tribalFemaleFileName) == 0) {
            _art_vault_person_nums[DUDE_NATIVE_LOOK_TRIBAL][GENDER_FEMALE] = critterIndex;
        }

        critterFileNames += FILENAME_LENGTH;
    }

    for (int critterIndex = 0; critterIndex < gArtListDescriptions[OBJ_TYPE_CRITTER].fileNamesLength; critterIndex++) {
        if (!fileReadString(string, sizeof(string), stream)) {
            break;
        }

        char* sep1 = strchr(string, ',');
        if (sep1 != nullptr) {
            _anon_alias[critterIndex] = atoi(sep1 + 1);

            char* sep2 = strchr(sep1 + 1, ',');
            if (sep2 != nullptr) {
                gArtCritterFidShoudRunData[critterIndex] = atoi(sep2 + 1);
            } else {
                gArtCritterFidShoudRunData[critterIndex] = 0;
            }
        } else {
            _anon_alias[critterIndex] = _art_vault_guy_num;
            gArtCritterFidShoudRunData[critterIndex] = 1;
        }
    }

    fileClose(stream);

    char* tileFileNames = gArtListDescriptions[OBJ_TYPE_TILE].fileNames;
    for (int tileIndex = 0; tileIndex < gArtListDescriptions[OBJ_TYPE_TILE].fileNamesLength; tileIndex++) {
        if (compat_stricmp(tileFileNames, "grid001.frm") == 0) {
            _art_mapper_blank_tile = tileIndex;
        }
        tileFileNames += FILENAME_LENGTH;
    }

    gHeadDescriptions = (HeadDescription*)internal_malloc(sizeof(*gHeadDescriptions) * gArtListDescriptions[OBJ_TYPE_HEAD].fileNamesLength);
    if (gHeadDescriptions == nullptr) {
        gArtListDescriptions[OBJ_TYPE_HEAD].fileNamesLength = 0;
        debugPrint("Out of memory for head_info in art_init\n");
        cacheFree(&gArtCache);
        return -1;
    }

    snprintf(path, sizeof(path), "%s%s%s\\%s.lst", _cd_path_base, "art\\", gArtListDescriptions[OBJ_TYPE_HEAD].name, gArtListDescriptions[OBJ_TYPE_HEAD].name);

    stream = fileOpen(path, "rt");
    if (stream == nullptr) {
        debugPrint("Unable to open %s in art_init\n", path);
        cacheFree(&gArtCache);
        return -1;
    }

    for (int headIndex = 0; headIndex < gArtListDescriptions[OBJ_TYPE_HEAD].fileNamesLength; headIndex++) {
        if (!fileReadString(string, sizeof(string), stream)) {
            break;
        }

        char* sep1 = strchr(string, ',');
        if (sep1 != nullptr) {
            *sep1 = '\0';
        } else {
            sep1 = string;
        }

        char* sep2 = strchr(sep1, ',');
        if (sep2 != nullptr) {
            *sep2 = '\0';
        } else {
            sep2 = sep1;
        }

        gHeadDescriptions[headIndex].goodFidgetCount = atoi(sep1 + 1);

        char* sep3 = strchr(sep2, ',');
        if (sep3 != nullptr) {
            *sep3 = '\0';
        } else {
            sep3 = sep2;
        }

        gHeadDescriptions[headIndex].neutralFidgetCount = atoi(sep2 + 1);

        char* sep4 = strpbrk(sep3 + 1, " ,;\t\n");
        if (sep4 != nullptr) {
            *sep4 = '\0';
        }

        gHeadDescriptions[headIndex].badFidgetCount = atoi(sep3 + 1);
    }

    fileClose(stream);

    return 0;
}

// 0x418EB8
void artReset()
{
}

// 0x418EBC
void artExit()
{
    cacheFree(&gArtCache);

    internal_free(_anon_alias);
    internal_free(gArtCritterFidShoudRunData);

    for (int index = 0; index < OBJ_TYPE_COUNT; index++) {
        internal_free(gArtListDescriptions[index].fileNames);
        gArtListDescriptions[index].fileNames = nullptr;

        internal_free(gArtListDescriptions[index].field_18);
        gArtListDescriptions[index].field_18 = nullptr;
    }

    internal_free(gHeadDescriptions);
}

// 0x418F1C
char* artGetObjectTypeName(int objectType)
{
    return objectType >= OBJ_TYPE_ITEM && objectType < OBJ_TYPE_COUNT ? gArtListDescriptions[objectType].name : nullptr;
}

// 0x418F34
int artIsObjectTypeHidden(int objectType)
{
    return objectType >= OBJ_TYPE_ITEM && objectType < OBJ_TYPE_COUNT ? gArtListDescriptions[objectType].flags & 1 : 0;
}

// 0x418F7C
int artGetFidgetCount(int headFid)
{
    if (FID_TYPE(headFid) != OBJ_TYPE_HEAD) {
        return 0;
    }

    int head = headFid & 0xFFF;

    if (head > gArtListDescriptions[OBJ_TYPE_HEAD].fileNamesLength) {
        return 0;
    }

    HeadDescription* headDescription = &(gHeadDescriptions[head]);

    int fidget = (headFid & 0xFF0000) >> 16;
    switch (fidget) {
    case FIDGET_GOOD:
        return headDescription->goodFidgetCount;
    case FIDGET_NEUTRAL:
        return headDescription->neutralFidgetCount;
    case FIDGET_BAD:
        return headDescription->badFidgetCount;
    }
    return 0;
}

// 0x418FFC
void artRender(int fid, unsigned char* dest, int width, int height, int pitch)
{
    // NOTE: Original code is different. For unknown reason it directly calls
    // many art functions, for example instead of [artLock] it calls lower level
    // [cacheLock], instead of [artGetWidth] is calls [artGetFrame], then get
    // width from frame's struct field. I don't know if this was intentional or
    // not. I've replaced these calls with higher level functions where
    // appropriate.

    CacheEntry* handle;
    Art* frm = artLock(fid, &handle);
    if (frm == nullptr) {
        return;
    }

    unsigned char* frameData = artGetFrameData(frm, 0, 0);
    int frameWidth = artGetWidth(frm, 0, 0);
    int frameHeight = artGetHeight(frm, 0, 0);

    int remainingWidth = width - frameWidth;
    int remainingHeight = height - frameHeight;
    if (remainingWidth < 0 || remainingHeight < 0) {
        if (height * frameWidth >= width * frameHeight) {
            blitBufferToBufferStretchTrans(frameData,
                frameWidth,
                frameHeight,
                frameWidth,
                dest + pitch * ((height - width * frameHeight / frameWidth) / 2),
                width,
                width * frameHeight / frameWidth,
                pitch);
        } else {
            blitBufferToBufferStretchTrans(frameData,
                frameWidth,
                frameHeight,
                frameWidth,
                dest + (width - height * frameWidth / frameHeight) / 2,
                height * frameWidth / frameHeight,
                height,
                pitch);
        }
    } else {
        blitBufferToBufferTrans(frameData,
            frameWidth,
            frameHeight,
            frameWidth,
            dest + pitch * (remainingHeight / 2) + remainingWidth / 2,
            pitch);
    }

    artUnlock(handle);
}

// mapper2.exe: 0x40A03C
int art_list_str(int fid, char* name)
{
    // TODO: Incomplete.

    return -1;
}

// 0x419160
Art* artLock(int fid, CacheEntry** handlePtr)
{
    if (handlePtr == nullptr) {
        return nullptr;
    }

    Art* art = nullptr;
    cacheLock(&gArtCache, fid, (void**)&art, handlePtr);
    return art;
}

// 0x419188
unsigned char* artLockFrameData(int fid, int frame, int direction, CacheEntry** handlePtr)
{
    Art* art;
    ArtFrame* frm;

    art = nullptr;
    if (handlePtr) {
        cacheLock(&gArtCache, fid, (void**)&art, handlePtr);
    }

    if (art != nullptr) {
        frm = artGetFrame(art, frame, direction);
        if (frm != nullptr) {

            return (unsigned char*)frm + sizeof(*frm);
        }
    }

    return nullptr;
}

// 0x4191CC
unsigned char* artLockFrameDataReturningSize(int fid, CacheEntry** handlePtr, int* widthPtr, int* heightPtr)
{
    *handlePtr = nullptr;

    Art* art = nullptr;
    cacheLock(&gArtCache, fid, (void**)&art, handlePtr);

    if (art == nullptr) {
        return nullptr;
    }

    // NOTE: Uninline.
    *widthPtr = artGetWidth(art, 0, 0);
    if (*widthPtr == -1) {
        return nullptr;
    }

    // NOTE: Uninline.
    *heightPtr = artGetHeight(art, 0, 0);
    if (*heightPtr == -1) {
        return nullptr;
    }

    // NOTE: Uninline.
    return artGetFrameData(art, 0, 0);
}

// 0x419260
int artUnlock(CacheEntry* handle)
{
    return cacheUnlock(&gArtCache, handle);
}

// 0x41927C
int artCacheFlush()
{
    return cacheFlush(&gArtCache);
}

// 0x4192B0
int artCopyFileName(int objectType, int id, char* dest)
{
    ArtListDescription* ptr;

    if (objectType < OBJ_TYPE_ITEM || objectType >= OBJ_TYPE_COUNT) {
        return -1;
    }

    ptr = &(gArtListDescriptions[objectType]);

    if (id >= ptr->fileNamesLength) {
        return -1;
    }

    strcpy(dest, ptr->fileNames + id * FILENAME_LENGTH);

    return 0;
}

// 0x419314
int _art_get_code(int animation, int weaponType, char* a3, char* a4)
{
    if (weaponType < 0 || weaponType >= WEAPON_ANIMATION_COUNT) {
        return -1;
    }

    if (animation >= ANIM_TAKE_OUT && animation <= ANIM_FIRE_CONTINUOUS) {
        *a4 = 'c' + (animation - ANIM_TAKE_OUT);
        if (weaponType == WEAPON_ANIMATION_NONE) {
            return -1;
        }

        *a3 = 'd' + (weaponType - 1);
        return 0;
    } else if (animation == ANIM_PRONE_TO_STANDING) {
        *a4 = 'h';
        *a3 = 'c';
        return 0;
    } else if (animation == ANIM_BACK_TO_STANDING) {
        *a4 = 'j';
        *a3 = 'c';
        return 0;
    } else if (animation == ANIM_CALLED_SHOT_PIC) {
        *a4 = 'a';
        *a3 = 'n';
        return 0;
    } else if (animation >= FIRST_SF_DEATH_ANIM) {
        *a4 = 'a' + (animation - FIRST_SF_DEATH_ANIM);
        *a3 = 'r';
        return 0;
    } else if (animation >= FIRST_KNOCKDOWN_AND_DEATH_ANIM) {
        *a4 = 'a' + (animation - FIRST_KNOCKDOWN_AND_DEATH_ANIM);
        *a3 = 'b';
        return 0;
    } else if (animation == ANIM_THROW_ANIM) {
        if (weaponType == WEAPON_ANIMATION_KNIFE) {
            // knife
            *a3 = 'd';
            *a4 = 'm';
        } else if (weaponType == WEAPON_ANIMATION_SPEAR) {
            // spear
            *a3 = 'g';
            *a4 = 'm';
        } else {
            // other -> probably rock or grenade
            *a3 = 'a';
            *a4 = 's';
        }
        return 0;
    } else if (animation == ANIM_DODGE_ANIM) {
        if (weaponType <= 0) {
            *a3 = 'a';
            *a4 = 'n';
        } else {
            *a3 = 'd' + (weaponType - 1);
            *a4 = 'e';
        }
        return 0;
    }

    *a4 = 'a' + animation;
    if (animation <= ANIM_WALK && weaponType > 0) {
        *a3 = 'd' + (weaponType - 1);
        return 0;
    }
    *a3 = 'a';

    return 0;
}

// 0x419428
char* artBuildFilePath(int fid)
{
    // Step 1: Unpack FID components
    int rotation = (fid & 0x70000000) >> 28;
    int aliasedFid = artAliasFid(fid);
    if (aliasedFid != -1) {
        fid = aliasedFid;
    }

    // Clear global buffer
    *_art_name = '\0';

    // Extract FID components
    int id = fid & 0xFFF;
    int animType = FID_ANIM_TYPE(fid);
    int weaponCode = (fid & 0xF000) >> 12;
    int objectType = FID_TYPE(fid);

    // Validate object type
    if (objectType < OBJ_TYPE_ITEM || objectType >= OBJ_TYPE_COUNT) {
        return nullptr;
    }

    // Validate art ID
    if (id >= gArtListDescriptions[objectType].fileNamesLength) {
        return nullptr;
    }

    // Calculate filename offset
    int nameOffset = id * FILENAME_LENGTH;

    // Handle special cases first
    if (objectType == OBJ_TYPE_CRITTER) { // Critters
        char animCode, weaponCodeChar;
        if (_art_get_code(animType, weaponCode, &animCode, &weaponCodeChar) == -1) {
            return nullptr;
        }

        if (rotation != 0) {
            snprintf(_art_name, sizeof(_art_name),
                "%sart\\%s\\%s%c%c.fr%c",
                _cd_path_base,
                gArtListDescriptions[objectType].name,
                gArtListDescriptions[objectType].fileNames + nameOffset,
                animCode,
                weaponCodeChar,
                rotation + '0'); // Convert to ASCII digit
        } else {
            snprintf(_art_name, sizeof(_art_name),
                "%sart\\%s\\%s%c%c.frm",
                _cd_path_base,
                gArtListDescriptions[objectType].name,
                gArtListDescriptions[objectType].fileNames + nameOffset,
                animCode,
                weaponCodeChar);
        }
    } else if (objectType == OBJ_TYPE_HEAD) { // Heads
        char genderCode = _head2[animType];
        if (genderCode == 'f') {
            snprintf(_art_name, sizeof(_art_name),
                "%sart\\%s\\%s%c%c%d.frm",
                _cd_path_base,
                gArtListDescriptions[objectType].name,
                gArtListDescriptions[objectType].fileNames + nameOffset,
                _head1[animType],
                'f', // Hardcode female identifier
                weaponCode);
        } else {
            snprintf(_art_name, sizeof(_art_name),
                "%sart\\%s\\%s%c%c.frm",
                _cd_path_base,
                gArtListDescriptions[objectType].name,
                gArtListDescriptions[objectType].fileNames + nameOffset,
                _head1[animType],
                genderCode);
        }
    } else { // All other types
        const char* fileName = gArtListDescriptions[objectType].fileNames + nameOffset;
        char basePath[COMPAT_MAX_PATH];

        // Build base path
        snprintf(basePath, sizeof(basePath),
            "%sart\\%s\\%s",
            _cd_path_base,
            gArtListDescriptions[objectType].name,
            fileName);

        // Ensure .frm extension
        size_t len = strlen(basePath);
        if (len < 4 || compat_stricmp(basePath + len - 4, ".frm") != 0) {
            // Safe concatenation
            if (len < sizeof(basePath) - 5) { // Check space for ".frm" + null
                strcat(basePath, ".frm");
            } else {
                debugPrint("Path too long: %s", basePath);
                return nullptr;
            }
        }

        // Copy to global buffer
        strncpy(_art_name, basePath, sizeof(_art_name));
        _art_name[sizeof(_art_name) - 1] = '\0'; // Ensure termination
    }

    return _art_name;
}

// art_read_lst
// 0x419664
static int artReadList(const char* path, char** artListPtr, int* artListSizePtr)
{
    File* stream = fileOpen(path, "rt");
    if (stream == nullptr) {
        return -1;
    }

    int count = 0;
    char string[200];
    while (fileReadString(string, sizeof(string), stream)) {
        count++;
    }

    fileSeek(stream, 0, SEEK_SET);

    *artListSizePtr = count;

    char* artList = (char*)internal_malloc(FILENAME_LENGTH * count);
    *artListPtr = artList;
    if (artList == nullptr) {
        fileClose(stream);
        return -1;
    }

    while (fileReadString(string, sizeof(string), stream)) {
        char* brk = strpbrk(string, " ,;\r\t\n");
        if (brk != nullptr) {
            *brk = '\0';
        }

        strncpy(artList, string, FILENAME_LENGTH - 1);
        artList[FILENAME_LENGTH - 1] = '\0';

        artList += FILENAME_LENGTH;
    }

    fileClose(stream);

    return 0;
}

// 0x419760
int artGetFramesPerSecond(Art* art)
{
    if (art == nullptr) {
        return 10;
    }

    return art->framesPerSecond == 0 ? 10 : art->framesPerSecond;
}

// 0x419778
int artGetActionFrame(Art* art)
{
    return art == nullptr ? -1 : art->actionFrame;
}

// 0x41978C
int artGetFrameCount(Art* art)
{
    return art == nullptr ? -1 : art->frameCount;
}

// 0x4197A0
int artGetWidth(Art* art, int frame, int direction)
{
    ArtFrame* frm;

    frm = artGetFrame(art, frame, direction);
    if (frm == nullptr) {
        return -1;
    }

    return frm->width;
}

// 0x4197B8
int artGetHeight(Art* art, int frame, int direction)
{
    ArtFrame* frm;

    frm = artGetFrame(art, frame, direction);
    if (frm == nullptr) {
        return -1;
    }

    return frm->height;
}

// 0x4197D4
int artGetSize(Art* art, int frame, int direction, int* widthPtr, int* heightPtr)
{
    ArtFrame* frm;

    frm = artGetFrame(art, frame, direction);
    if (frm == nullptr) {
        if (widthPtr != nullptr) {
            *widthPtr = 0;
        }

        if (heightPtr != nullptr) {
            *heightPtr = 0;
        }

        return -1;
    }

    if (widthPtr != nullptr) {
        *widthPtr = frm->width;
    }

    if (heightPtr != nullptr) {
        *heightPtr = frm->height;
    }

    return 0;
}

// 0x419820
int artGetFrameOffsets(Art* art, int frame, int direction, int* xPtr, int* yPtr)
{
    ArtFrame* frm;

    frm = artGetFrame(art, frame, direction);
    if (frm == nullptr) {
        return -1;
    }

    *xPtr = frm->x;
    *yPtr = frm->y;

    return 0;
}

// 0x41984C
int artGetRotationOffsets(Art* art, int rotation, int* xPtr, int* yPtr)
{
    if (art == nullptr) {
        return -1;
    }

    *xPtr = art->xOffsets[rotation];
    *yPtr = art->yOffsets[rotation];

    return 0;
}

// 0x419870
unsigned char* artGetFrameData(Art* art, int frame, int direction)
{
    ArtFrame* frm;

    frm = artGetFrame(art, frame, direction);
    if (frm == nullptr) {
        return nullptr;
    }

    return (unsigned char*)frm + sizeof(*frm);
}

// 0x419880
ArtFrame* artGetFrame(Art* art, int frame, int rotation)
{
    if (rotation < 0 || rotation >= 6) {
        return nullptr;
    }

    if (art == nullptr) {
        return nullptr;
    }

    if (frame < 0 || frame >= art->frameCount) {
        return nullptr;
    }

    ArtFrame* frm = (ArtFrame*)((unsigned char*)art + sizeof(*art) + art->dataOffsets[rotation] + art->padding[rotation]);
    for (int index = 0; index < frame; index++) {
        frm = (ArtFrame*)((unsigned char*)frm + sizeof(*frm) + frm->size + paddingForSize(frm->size));
    }
    return frm;
}

// 0x4198C8
bool artExists(int fid)
{
    bool result = false;

    char* filePath = artBuildFilePath(fid);
    if (filePath != nullptr) {
        int fileSize;
        if (dbGetFileSize(filePath, &fileSize) != -1) {
            result = true;
        }
    }

    return result;
}

// NOTE: Exactly the same implementation as `artExists`.
//
// 0x419930
bool _art_fid_valid(int fid)
{
    bool result = false;

    char* filePath = artBuildFilePath(fid);
    if (filePath != nullptr) {
        int fileSize;
        if (dbGetFileSize(filePath, &fileSize) != -1) {
            result = true;
        }
    }

    return result;
}

// 0x419998
int _art_alias_num(int index)
{
    return _anon_alias[index];
}

// 0x4199AC
int artCritterFidShouldRun(int fid)
{
    if (FID_TYPE(fid) == OBJ_TYPE_CRITTER) {
        return gArtCritterFidShoudRunData[fid & 0xFFF];
    }

    return 0;
}

// 0x4199D4
int artAliasFid(int fid)
{
    int type = FID_TYPE(fid);
    int anim = FID_ANIM_TYPE(fid);
    if (type == OBJ_TYPE_CRITTER) {
        if (anim == ANIM_ELECTRIFY
            || anim == ANIM_BURNED_TO_NOTHING
            || anim == ANIM_ELECTRIFIED_TO_NOTHING
            || anim == ANIM_ELECTRIFY_SF
            || anim == ANIM_BURNED_TO_NOTHING_SF
            || anim == ANIM_ELECTRIFIED_TO_NOTHING_SF
            || anim == ANIM_FIRE_DANCE
            || anim == ANIM_CALLED_SHOT_PIC) {
            // NOTE: Original code is slightly different. It uses many mutually
            // mirrored bitwise operators. Probably result of some macros for
            // getting/setting individual bits on fid.
            return (fid & 0x70000000) | ((anim << 16) & 0xFF0000) | 0x1000000 | (fid & 0xF000) | (_anon_alias[fid & 0xFFF] & 0xFFF);
        }
    }

    return -1;
}

// 0x419A78
static int artCacheGetFileSizeImpl(int fid, int* sizePtr)
{
    int result = -1;

    char* artFilePath = artBuildFilePath(fid);
    if (artFilePath != nullptr) {
        bool loaded = false;
        File* stream = nullptr;

        if (gArtLanguageInitialized) {
            char* pch = strchr(artFilePath, '\\');
            if (pch == nullptr) {
                pch = artFilePath;
            }

            char localizedPath[COMPAT_MAX_PATH];
            snprintf(localizedPath, sizeof(localizedPath), "art\\%s\\%s", gArtLanguage, pch);

            stream = fileOpen(localizedPath, "rb");
        }

        if (stream == nullptr) {
            stream = fileOpen(artFilePath, "rb");
        }

        if (stream != nullptr) {
            Art art;
            if (artReadHeader(&art, stream) == 0) {
                *sizePtr = artGetDataSize(&art);
                result = 0;
            }
            fileClose(stream);
        }
    }

    return result;
}

// 0x419B78
static int artCacheReadDataImpl(int fid, int* sizePtr, unsigned char* data)
{
    int result = -1;

    char* artFileName = artBuildFilePath(fid);
    if (artFileName != nullptr) {
        bool loaded = false;
        if (gArtLanguageInitialized) {
            char* pch = strchr(artFileName, '\\');
            if (pch == nullptr) {
                pch = artFileName;
            }

            char localizedPath[COMPAT_MAX_PATH];
            snprintf(localizedPath, sizeof(localizedPath), "art\\%s\\%s", gArtLanguage, pch);

            if (artRead(localizedPath, data) == 0) {
                loaded = true;
            }
        }

        if (!loaded) {
            if (artRead(artFileName, data) == 0) {
                loaded = true;
            }
        }

        if (loaded) {
            *sizePtr = artGetDataSize((Art*)data);
            result = 0;
        }
    }

    return result;
}

// 0x419C80
static void artCacheFreeImpl(void* ptr)
{
    internal_free(ptr);
}

static int buildFidInternal(unsigned short frmId, unsigned char weaponCode, unsigned char animType, unsigned char objectType, unsigned char rotation)
{
    return ((rotation << 28) & 0x70000000) | (objectType << 24) | ((animType << 16) & 0xFF0000) | ((weaponCode << 12) & 0xF000) | (frmId & 0xFFF);
}

// 0x419C88
int buildFid(int objectType, int frmId, int animType, int weaponCode, int rotation)
{
    // Always use rotation 0 (NE) for non-critters, for certain critter animations.
    // For other critter animations, check if art for the given rotation exists, if not try rotation 1 (E) and if that also doesn't exist, then default to 0 (NE).
    if (objectType != OBJ_TYPE_CRITTER
        || animType == ANIM_FIRE_DANCE
        || animType < ANIM_FALL_BACK
        || animType > ANIM_FALL_FRONT_BLOOD) {
        rotation = ROTATION_NE;
    } else if (!artExists(buildFidInternal(frmId, weaponCode, animType, OBJ_TYPE_CRITTER, rotation))) {
        rotation = rotation != ROTATION_E
                && artExists(buildFidInternal(frmId, weaponCode, animType, OBJ_TYPE_CRITTER, ROTATION_E))
            ? ROTATION_E
            : ROTATION_NE;
    }
    return buildFidInternal(frmId, weaponCode, animType, objectType, rotation);
}

// 0x419D60
static int artReadFrameData(unsigned char* data, File* stream, int count, int* paddingPtr)
{
    unsigned char* ptr = data;
    int padding = 0;
    for (int index = 0; index < count; index++) {
        ArtFrame* frame = (ArtFrame*)ptr;

        if (fileReadInt16(stream, &(frame->width)) == -1)
            return -1;
        if (fileReadInt16(stream, &(frame->height)) == -1)
            return -1;
        if (fileReadInt32(stream, &(frame->size)) == -1)
            return -1;
        if (fileReadInt16(stream, &(frame->x)) == -1)
            return -1;
        if (fileReadInt16(stream, &(frame->y)) == -1)
            return -1;
        if (fileRead(ptr + sizeof(ArtFrame), frame->size, 1, stream) != 1)
            return -1;

        ptr += sizeof(ArtFrame) + frame->size;
        ptr += paddingForSize(frame->size);
        padding += paddingForSize(frame->size);
    }

    *paddingPtr = padding;

    return 0;
}

// 0x419E1C
static int artReadHeader(Art* art, File* stream)
{
    if (fileReadInt32(stream, &(art->field_0)) == -1)
        return -1;
    if (fileReadInt16(stream, &(art->framesPerSecond)) == -1)
        return -1;
    if (fileReadInt16(stream, &(art->actionFrame)) == -1)
        return -1;
    if (fileReadInt16(stream, &(art->frameCount)) == -1)
        return -1;
    if (fileReadInt16List(stream, art->xOffsets, ROTATION_COUNT) == -1)
        return -1;
    if (fileReadInt16List(stream, art->yOffsets, ROTATION_COUNT) == -1)
        return -1;
    if (fileReadInt32List(stream, art->dataOffsets, ROTATION_COUNT) == -1)
        return -1;
    if (fileReadInt32(stream, &(art->dataSize)) == -1)
        return -1;

    // CE: Fix malformed `frm` files with `dataSize` set to 0 in Nevada.
    if (art->dataSize == 0) {
        art->dataSize = fileGetSize(stream);
    }

    return 0;
}

// NOTE: Original function was slightly different, but never used. Basically
// it's a memory allocating variant of `artRead` (which reads data into given
// buffer). This function is useful to load custom `frm` files since `Art` now
// needs more memory then it's on-disk size (due to memory padding).
//
// 0x419EC0
Art* artLoad(const char* path)
{
    File* stream = fileOpen(path, "rb");
    if (stream == nullptr) {
        return nullptr;
    }

    Art header;
    if (artReadHeader(&header, stream) != 0) {
        fileClose(stream);
        return nullptr;
    }

    fileClose(stream);

    unsigned char* data = reinterpret_cast<unsigned char*>(internal_malloc(artGetDataSize(&header)));
    if (data == nullptr) {
        return nullptr;
    }

    if (artRead(path, data) != 0) {
        internal_free(data);
        return nullptr;
    }

    return reinterpret_cast<Art*>(data);
}

// 0x419FC0
int artRead(const char* path, unsigned char* data)
{
    File* stream = fileOpen(path, "rb");
    if (stream == nullptr) {
        return -2;
    }

    Art* art = (Art*)data;
    if (artReadHeader(art, stream) != 0) {
        fileClose(stream);
        return -3;
    }

    int currentPadding = paddingForSize(sizeof(Art));
    int previousPadding = 0;

    for (int index = 0; index < ROTATION_COUNT; index++) {
        art->padding[index] = currentPadding;

        if (index == 0 || art->dataOffsets[index - 1] != art->dataOffsets[index]) {
            art->padding[index] += previousPadding;
            currentPadding += previousPadding;
            if (artReadFrameData(data + sizeof(Art) + art->dataOffsets[index] + art->padding[index], stream, art->frameCount, &previousPadding) != 0) {
                fileClose(stream);
                return -5;
            }
        }
    }

    fileClose(stream);
    return 0;
}

// NOTE: Unused.
//
// 0x41A070
int artWriteFrameData(unsigned char* data, File* stream, int count)
{
    unsigned char* ptr = data;
    for (int index = 0; index < count; index++) {
        ArtFrame* frame = (ArtFrame*)ptr;

        if (fileWriteInt16(stream, frame->width) == -1)
            return -1;
        if (fileWriteInt16(stream, frame->height) == -1)
            return -1;
        if (fileWriteInt32(stream, frame->size) == -1)
            return -1;
        if (fileWriteInt16(stream, frame->x) == -1)
            return -1;
        if (fileWriteInt16(stream, frame->y) == -1)
            return -1;
        if (fileWrite(ptr + sizeof(ArtFrame), frame->size, 1, stream) != 1)
            return -1;

        ptr += sizeof(ArtFrame) + frame->size;
        ptr += paddingForSize(frame->size);
    }

    return 0;
}

// NOTE: Unused.
//
// 0x41A138
int artWriteHeader(Art* art, File* stream)
{
    if (fileWriteInt32(stream, art->field_0) == -1)
        return -1;
    if (fileWriteInt16(stream, art->framesPerSecond) == -1)
        return -1;
    if (fileWriteInt16(stream, art->actionFrame) == -1)
        return -1;
    if (fileWriteInt16(stream, art->frameCount) == -1)
        return -1;
    if (fileWriteInt16List(stream, art->xOffsets, ROTATION_COUNT) == -1)
        return -1;
    if (fileWriteInt16List(stream, art->yOffsets, ROTATION_COUNT) == -1)
        return -1;
    if (fileWriteInt32List(stream, art->dataOffsets, ROTATION_COUNT) == -1)
        return -1;
    if (fileWriteInt32(stream, art->dataSize) == -1)
        return -1;

    return 0;
}

// NOTE: Unused.
//
// 0x41A1E8
int artWrite(const char* path, unsigned char* data)
{
    if (data == nullptr) {
        return -1;
    }

    File* stream = fileOpen(path, "wb");
    if (stream == nullptr) {
        return -1;
    }

    Art* art = (Art*)data;
    if (artWriteHeader(art, stream) == -1) {
        fileClose(stream);
        return -1;
    }

    for (int index = 0; index < ROTATION_COUNT; index++) {
        if (index == 0 || art->dataOffsets[index - 1] != art->dataOffsets[index]) {
            if (artWriteFrameData(data + sizeof(Art) + art->dataOffsets[index] + art->padding[index], stream, art->frameCount) != 0) {
                fileClose(stream);
                return -1;
            }
        }
    }

    fileClose(stream);
    return 0;
}

static int artGetDataSize(Art* art)
{
    int dataSize = sizeof(*art) + art->dataSize;

    for (int index = 0; index < ROTATION_COUNT; index++) {
        if (index == 0 || art->dataOffsets[index - 1] != art->dataOffsets[index]) {
            // Assume worst case - every frame is unaligned and need
            // max padding.
            dataSize += (sizeof(int) - 1) * art->frameCount;
        }
    }

    return dataSize;
}

static int paddingForSize(int size)
{
    return (sizeof(int) - size % sizeof(int)) % sizeof(int);
}

FrmImage::FrmImage()
{
    _key = nullptr;
    _data = nullptr;
    _width = 0;
    _height = 0;
}

FrmImage::~FrmImage()
{
    unlock();
}

bool FrmImage::lock(unsigned int fid)
{
    if (isLocked()) {
        return false;
    }

    _data = artLockFrameDataReturningSize(fid, &_key, &_width, &_height);
    if (!_data) {
        return false;
    }

    return true;
}

void FrmImage::unlock()
{
    if (isLocked()) {
        artUnlock(_key);
        _key = nullptr;
        _data = nullptr;
        _width = 0;
        _height = 0;
    }
}

} // namespace fallout
