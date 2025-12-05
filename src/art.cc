#include "art.h"

#include <stdio.h>

#include "db.h" // for fileOpen, fileClose
#include "xfile.h" // for File type
#define DIR_SEPARATOR '/'

#include <time.h>
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
#include "window_manager.h"

namespace fallout {

const int MAX_ART_INDICES = 8192;

static inline int artGetIndex(int fid)
{
    int base_index = fid & 0xFFF;
    return (fid & 0x80000000) ? base_index + 4096 : base_index;
}

typedef struct ArtListDescription {
    int flags;
    char name[16];
    void* field_18;
    char* fileNames; // Combined: vanilla + variants + mods
    int fileNamesLength; // Total entries

    int vanillaCount; // Original vanilla entries
    int variantCount; // Added variants
    int modCount; // Added mod entries

    // Collision tracking arrays
    bool usedIndices[8192]; // Tracks occupied indices
    const char* artNames[8192]; // Filenames by index for error reporting

    // Collision reporting fields
    bool collisionOccurred; // True if any collisions in this category
    char collisionDetails[8192][128]; // Collision messages per index

    bool categoryFull; // Set when no mod slots available
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

// Error message for mod naming conflicts
void showFatalError(const char* message)
{
    debugPrint("FATAL ART ERROR: %s", message);
    showMesageBox(message); // Corrected spelling
    exit(1);
}

// Helper to extract base filename without extension
static void getBaseNameWithoutExtension(char* dest, const char* path, size_t size)
{
    // Find last separator
    const char* base = strrchr(path, '/');
    if (!base)
        base = strrchr(path, '\\');
    base = base ? base + 1 : path;

    // Copy base name
    strncpy(dest, base, size - 1);
    dest[size - 1] = '\0';

    // Remove extension
    char* dot = strrchr(dest, '.');
    if (dot)
        *dot = '\0';
}

/**
 * Generates a stable index for mod assets based on filename
 *
 * Provides deterministic index assignment to ensure:
 * - Consistent asset placement
 * - Predictable FID generation
 * - No conflicts with vanilla/variant assets
 */
static int artGetStableIndex(const char* filename, int vanillaCount, int variantCount)
{
    // Step 1: Filename normalization
    // ------------------------------
    // Create a standardized representation by:
    // - Removing paths and extensions
    // - Converting to lowercase
    // - Keeping only alphanumeric characters and underscores
    // Example: "Items/Plasma_Rifle.PRO" → "plasma_rifle"
    char normalized[64] = { 0 };
    char* dest = normalized;

    // Process each character in input filename
    for (const char* src = filename; *src && dest < normalized + sizeof(normalized) - 1; src++) {
        char c = *src;

        // Preserve digits (0-9)
        if (c >= '0' && c <= '9')
            *dest++ = c;

        // Preserve lowercase letters (a-z)
        else if (c >= 'a' && c <= 'z')
            *dest++ = c;

        // Convert uppercase to lowercase (A-Z → a-z)
        else if (c >= 'A' && c <= 'Z')
            *dest++ = tolower(c);

        // Preserve underscores as word separators
        else if (c == '_')
            *dest++ = c;

        // Omit all other characters (paths, extensions, special chars)
    }
    *dest = '\0'; // Null-terminate the normalized string

    // Step 2: 'Hash' generation via base-36 conversion
    // ----------------------------------------------
    // Create a numeric fingerprint of the normalized name by
    // interpreting it as a base-36 number (digits + letters)
    uint64_t hashValue = 0;

    // Process each character in normalized name
    for (char* ptr = normalized; *ptr; ptr++) {
        // Convert character to its base-36 value:
        // - '0'-'9' → 0-9
        // - 'a'-'z' → 10-35
        int digitValue = (*ptr >= '0' && *ptr <= '9') ? *ptr - '0' : *ptr - 'a' + 10;

        // Accumulate hash: shift existing value and add new digit
        hashValue = hashValue * 36 + digitValue;
    }

    // Step 3: Index space calculation
    // -------------------------------
    // Always use extended range (4096-8191) for mod assets
    return 4096 + (hashValue % 4096); // Returns value between 4096-8191
}

int artGetFidWithVariant(int objectType, int baseId, bool useVariant)
{
    if (useVariant && objectType == OBJ_TYPE_INTERFACE) {
        int variantId = artFindVariant(objectType, baseId, settings.graphics.widescreen_variant_suffix.c_str());
        if (variantId >= 0) {
            return buildFid(objectType, variantId, 0, 0, 0);
        }
    }
    return buildFid(objectType, baseId, 0, 0, 0);
}

int artFindVariant(int objectType, int baseIndex, const char* suffix)
{
    if (objectType < 0 || objectType >= OBJ_TYPE_COUNT)
        return -1;

    ArtListDescription* desc = &gArtListDescriptions[objectType];

    // Filename search mode for mod assets
    if (baseIndex == -1) {
        for (int i = 0; i < desc->fileNamesLength; i++) {
            char* candidate = desc->fileNames + (i * FILENAME_LENGTH);
            if (compat_stricmp(candidate, suffix) == 0) {
                return i;
            }
        }
        return -1;
    }

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
    if (len >= static_cast<int>(sizeof(expected))) {
        debugPrint("Variant name too long: %s%s", base, suffix);
        return -1;
    }

    // Search variant section
    for (int i = desc->vanillaCount; i < desc->vanillaCount + desc->variantCount; i++) {
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

    int cacheSize = settings.system.art_cache_size;
    if (!cacheInit(&gArtCache, artCacheGetFileSizeImpl, artCacheReadDataImpl, artCacheFreeImpl, cacheSize << 20)) {
        debugPrint("cache_init failed in art_init\n");
        return -1;
    }

    const char* language = settings.system.language.c_str();
    if (compat_stricmp(language, ENGLISH) != 0) {
        strcpy(gArtLanguage, language);
        gArtLanguageInitialized = true;
    }

    bool critterDbSelected = false;
    for (int objectType = 0; objectType < OBJ_TYPE_COUNT; objectType++) {
        ArtListDescription* desc = &gArtListDescriptions[objectType];
        desc->flags = 0;

        desc->collisionOccurred = false;
        for (int i = 0; i < 4096; i++) {
            desc->collisionDetails[i][0] = '\0'; // Clear any previous messages
        }

        snprintf(path, sizeof(path), "%s%s%s\\%s.lst", _cd_path_base, "art\\", gArtListDescriptions[objectType].name, gArtListDescriptions[objectType].name);

        // 1. Load VANILLA assets
        if (artReadList(path, &(desc->fileNames), &(desc->fileNamesLength)) != 0) {
            debugPrint("art_read_lst failed in art_init\n");
            cacheFree(&gArtCache);
            return -1;
        }
        desc->vanillaCount = desc->fileNamesLength; // Store vanilla count

        // 2. Process Variant Assets
        // -------------------------
        // Variants are higher-resolution versions of existing assets (e.g., "_800.frm" for 800x500)
        // Variant suffix can be set in fallout2.cfg
        char suffix[32];
        snprintf(suffix, sizeof(suffix), "%s.frm",
            settings.graphics.widescreen_variant_suffix.c_str());
        size_t suffixLen = strlen(suffix);

        // Build search pattern for variant files: "art/<category>/*.frm"
        char pattern[COMPAT_MAX_PATH];
        snprintf(pattern, sizeof(pattern),
            "art%c%s%c*.frm",
            DIR_SEPARATOR,
            desc->name,
            DIR_SEPARATOR);

        // Find all matching variant files in this category
        char** foundFiles = NULL;
        int fileCount = fileNameListInit(pattern, &foundFiles, 0, 0);

        // Create a set to track which base assets already have variants
        bool* hasVariant = nullptr;
        if (desc->vanillaCount > 0) {
            hasVariant = (bool*)internal_malloc(desc->vanillaCount * sizeof(bool));
            if (hasVariant != nullptr) {
                memset(hasVariant, 0, desc->vanillaCount * sizeof(bool));
            } else {
                debugPrint("WARNING: Failed to allocate variant tracking array for %s\n", desc->name);
            }
        }

        if (fileCount > 0) {
            // Prepare to extend existing asset list
            int originalCount = desc->fileNamesLength;
            int newCount = originalCount;
            char* names = desc->fileNames;
            int currentSize = desc->fileNamesLength; // Current array capacity

            // Process each found variant file
            for (int i = 0; i < fileCount; i++) {
                const char* filename = foundFiles[i];
                size_t len = strlen(filename);

                // Skip files without the variant suffix
                if (len <= suffixLen || compat_stricmp(filename + len - suffixLen, suffix) != 0) {
                    continue;
                }

                // Extract base filename without path
                const char* baseName = strrchr(filename, DIR_SEPARATOR);
                if (!baseName)
                    baseName = filename;
                else
                    baseName++; // Skip separator

                // Create variant base name by removing resolution suffix
                // Example: "button_ok_800.frm" → "button_ok"
                char variantBase[FILENAME_LENGTH] = { 0 };
                size_t baseLen = strlen(baseName) - suffixLen;
                if (baseLen >= FILENAME_LENGTH)
                    baseLen = FILENAME_LENGTH - 1;
                strncpy(variantBase, baseName, baseLen);
                variantBase[baseLen] = '\0';

                // Check if this variant matches any existing vanilla asset
                bool matchFound = false;
                int matchedIndex = -1;

                for (int j = 0; j < originalCount; j++) {
                    const char* slot = names + j * FILENAME_LENGTH;

                    // Extract base name of existing asset
                    const char* slotName = strrchr(slot, DIR_SEPARATOR);
                    if (!slotName)
                        slotName = slot;
                    else
                        slotName++;

                    // Remove extension from existing asset
                    char slotBase[FILENAME_LENGTH];
                    strncpy(slotBase, slotName, FILENAME_LENGTH);
                    char* ext = strrchr(slotBase, '.');
                    if (ext)
                        *ext = '\0';

                    // Compare base names (case-insensitive)
                    if (compat_stricmp(slotBase, variantBase) == 0) {
                        matchFound = true;
                        matchedIndex = j;
                        break;
                    }
                }

                // Skip variants without matching base asset
                if (!matchFound)
                    continue;

                // Skip if we already have a variant for this base asset
                if (hasVariant != nullptr && hasVariant[matchedIndex]) {
                    debugPrint("Skipping duplicate variant for %s: %s\n", variantBase, filename);
                    continue;
                }

                // Mark this base asset as having a variant
                if (hasVariant != nullptr) {
                    hasVariant[matchedIndex] = true;
                }

                // Expand array if needed (grow in chunks of 10)
                if (newCount >= currentSize) {
                    currentSize += 10;
                    char* newNames = (char*)internal_realloc(names, currentSize * FILENAME_LENGTH);
                    if (!newNames)
                        break; // Abort if realloc fails
                    names = newNames;
                }

                // Add variant to asset list
                char* dest = names + newCount * FILENAME_LENGTH;
                strncpy(dest, baseName, FILENAME_LENGTH - 1);
                dest[FILENAME_LENGTH - 1] = '\0';
                newCount++;
            }

            // Update asset list if we added variants
            if (newCount > originalCount) {
                desc->fileNames = names;
                desc->fileNamesLength = newCount;
            }

            // Cleanup
            if (hasVariant) {
                internal_free(hasVariant);
            }

            // Cleanup file list
            fileNameListFree(&foundFiles, fileCount);
        }

        // Store variant count after processing
        // (Total assets now = vanilla + variants)
        desc->variantCount = desc->fileNamesLength - desc->vanillaCount;

        // MOD: Expand art lists to 8192 entries
        if (desc->fileNamesLength < MAX_ART_INDICES) {
            char* newNames = (char*)internal_realloc(desc->fileNames,
                MAX_ART_INDICES * FILENAME_LENGTH);
            if (newNames) {
                desc->fileNames = newNames;
                // Initialize new slots to empty
                for (int i = desc->fileNamesLength; i < MAX_ART_INDICES; i++) {
                    char* slot = desc->fileNames + i * FILENAME_LENGTH;
                    *slot = '\0';
                }
            }
        }
        desc->fileNamesLength = MAX_ART_INDICES;
        // 3. Load MOD Assets
        // ------------------
        // Search for mod list files in this art category using pattern:
        // <game_dir>/art/<category>/mod_*.lst
        char searchPattern[COMPAT_MAX_PATH];
        snprintf(searchPattern, sizeof(searchPattern),
            "%sart%c%s%c*.lst",
            _cd_path_base,
            DIR_SEPARATOR,
            desc->name,
            DIR_SEPARATOR);

        // Find all matching .lst files
        char** foundModFiles = nullptr;
        int modFileCount = fileNameListInit(searchPattern, &foundModFiles, 0, 0);

        // Initialize mod tracking
        desc->modCount = 0;

        // Prepare index usage tracking array
        memset(desc->usedIndices, 0, sizeof(desc->usedIndices)); // Now 8192 elements

        // Mark existing vanilla and variant indices as occupied
        for (int i = 0; i < desc->fileNamesLength; i++) {
            desc->usedIndices[i] = true;
        }

        if (modFileCount > 0) {
            // Build base directory path for this art category
            char baseDir[COMPAT_MAX_PATH];
            snprintf(baseDir, sizeof(baseDir), "%sart%c%s%c",
                _cd_path_base,
                DIR_SEPARATOR,
                desc->name,
                DIR_SEPARATOR);

            // Sort files to ensure fission.lst is processed first
            for (int i = 0; i < modFileCount - 1; i++) {
                for (int j = i + 1; j < modFileCount; j++) {
                    // Move fission.lst to the front
                    if (strcmp(foundModFiles[j], "fission.lst") == 0) {
                        char* temp = foundModFiles[i];
                        foundModFiles[i] = foundModFiles[j];
                        foundModFiles[j] = temp;
                        break;
                    }
                }
            }

            // Process each found mod list file
            for (int i = 0; i < modFileCount; i++) {
                // Skip files that don't match our expected patterns
                if (strcmp(foundModFiles[i], "fission.lst") != 0 && strncmp(foundModFiles[i], "mod_", 4) != 0) {
                    continue;
                }

                // Construct full path to mod list
                char fullPath[COMPAT_MAX_PATH];
                snprintf(fullPath, sizeof(fullPath), "%s%s", baseDir, foundModFiles[i]);

                char* modEntries = nullptr;
                int modEntryCount = 0;

                // Load assets from mod list file
                if (artReadList(fullPath, &modEntries, &modEntryCount) == 0) {
                    // Process each asset in the mod list
                    for (int j = 0; j < modEntryCount; j++) {
                        const char* modAssetName = modEntries + j * FILENAME_LENGTH;

                        // Check for remapping directive
                        if (modAssetName[0] == '@') {
                            // Parse remapping directive: "@original_name=new_path/filename.frm"
                            char originalName[FILENAME_LENGTH] = { 0 };
                            char newPath[FILENAME_LENGTH] = { 0 };
                            const char* equalSign = strchr(modAssetName, '=');

                            if (equalSign && (equalSign - modAssetName) < FILENAME_LENGTH) {
                                // Extract original name (skip '@' and copy until '=')
                                size_t nameLen = equalSign - modAssetName - 1;
                                if (nameLen > FILENAME_LENGTH - 1)
                                    nameLen = FILENAME_LENGTH - 1;
                                strncpy(originalName, modAssetName + 1, nameLen);
                                originalName[nameLen] = '\0';

                                // Extract new path (after '=')
                                strncpy(newPath, equalSign + 1, FILENAME_LENGTH - 1);
                                newPath[FILENAME_LENGTH - 1] = '\0';

                                // Find matching vanilla asset to remap
                                bool remapped = false;
                                for (int idx = 0; idx < desc->vanillaCount; idx++) {
                                    char* currentPath = desc->fileNames + idx * FILENAME_LENGTH;
                                    char currentBase[FILENAME_LENGTH];
                                    getBaseNameWithoutExtension(currentBase, currentPath, sizeof(currentBase));

                                    if (compat_stricmp(currentBase, originalName) == 0) {
                                        // Backup old path for reporting
                                        char oldPath[FILENAME_LENGTH];
                                        strncpy(oldPath, currentPath, FILENAME_LENGTH);
                                        oldPath[FILENAME_LENGTH - 1] = '\0';

                                        // Perform remapping
                                        strncpy(currentPath, newPath, FILENAME_LENGTH);
                                        currentPath[FILENAME_LENGTH - 1] = '\0';

                                        // Record remapping
                                        snprintf(desc->collisionDetails[idx], sizeof(desc->collisionDetails[idx]),
                                            "REMAP: %s -> %s", oldPath, newPath);
                                        desc->collisionOccurred = true;
                                        remapped = true;
                                        break; // Only remap first match
                                    }
                                }

                                if (!remapped) {
                                    // Log warning about failed remap
                                    debugPrint("WARNING: Remap target not found: %s\n", originalName);
                                }
                            } else {
                                debugPrint("WARNING: Invalid remap syntax: %s\n", modAssetName);
                            }
                            continue; // Skip normal processing for remap entries
                        }

                        // Normal asset processing
                        char baseName[FILENAME_LENGTH];
                        getBaseNameWithoutExtension(baseName, modAssetName, sizeof(baseName));

                        // Calculate stable index position
                        int index = artGetStableIndex(
                            baseName,
                            desc->vanillaCount,
                            desc->variantCount);

                        // Handle category capacity overflow
                        if (index == -1) {
                            desc->categoryFull = true;
                            char errorMsg[256];
                            snprintf(errorMsg, sizeof(errorMsg),
                                "Art category capacity exceeded\n\n"
                                "Category: %s\n"
                                "Vanilla assets: %d\n"
                                "Variant assets: %d\n"
                                "Total used: %d/%d\n\n" // Changed to show 8192 max
                                "Cannot add mod asset: %s\n\n"
                                "No available index slots remain.",
                                desc->name,
                                desc->vanillaCount,
                                desc->variantCount,
                                desc->vanillaCount + desc->variantCount,
                                MAX_ART_INDICES, // Show new max
                                modAssetName);
                            showFatalError(errorMsg);
                            continue;
                        }

                        // Check for index collision with existing assets
                        if (desc->usedIndices[index]) {
                            const char* existing = "(unidentified asset)";
                            char existingBase[FILENAME_LENGTH] = { 0 };

                            if (index < desc->fileNamesLength) {
                                existing = desc->fileNames + index * FILENAME_LENGTH;
                                // Extract base name of existing asset
                                getBaseNameWithoutExtension(existingBase, existing, sizeof(existingBase));
                            }

                            // Extract base name of new asset
                            char currentBase[FILENAME_LENGTH] = { 0 };
                            getBaseNameWithoutExtension(currentBase, modAssetName, sizeof(currentBase));

                            // Report as COLLISION
                            snprintf(desc->collisionDetails[index], sizeof(desc->collisionDetails[index]),
                                "COLLISION: %s (existing) vs %s (new)", existing, modAssetName);
                            desc->collisionOccurred = true;
                        }

                        // Expand asset array if needed
                        if (index >= desc->fileNamesLength) {
                            int newLength = index + 1;
                            char* newNames = (char*)internal_realloc(desc->fileNames, newLength * FILENAME_LENGTH);
                            if (newNames) {
                                // Initialize new slots as empty
                                for (int k = desc->fileNamesLength; k < newLength; k++) {
                                    char* slot = newNames + k * FILENAME_LENGTH;
                                    *slot = '\0';
                                }
                                desc->fileNames = newNames;
                                desc->fileNamesLength = newLength;
                            }
                        }

                        // Store asset at calculated index
                        char* slot = desc->fileNames + index * FILENAME_LENGTH;
                        strncpy(slot, modAssetName, FILENAME_LENGTH - 1);
                        slot[FILENAME_LENGTH - 1] = '\0';

                        // Update tracking information
                        desc->usedIndices[index] = true;
                        desc->modCount++;
                    }
                    internal_free(modEntries);
                }
            }
            fileNameListFree(&foundModFiles, modFileCount);
        }
    }

    _anon_alias = (int*)internal_malloc(MAX_ART_INDICES * sizeof(int));
    if (_anon_alias == nullptr) {
        gArtListDescriptions[OBJ_TYPE_CRITTER].fileNamesLength = 0;
        debugPrint("Out of memory for anon_alias in art_init\n");
        cacheFree(&gArtCache);
        return -1;
    }

    gArtCritterFidShoudRunData = (int*)internal_malloc(MAX_ART_INDICES * sizeof(int));
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

    // Generate a report listing all vanilla, variant and mod assets
    // including overrides and conflicts

    // Create art_list.txt in the game's root directory using direct file operations
    char artListPath[COMPAT_MAX_PATH];
    snprintf(artListPath, sizeof(artListPath), "%sdata%clists%cart_list.txt", _cd_path_base, DIR_SEPARATOR, DIR_SEPARATOR);

    FILE* artListFile = compat_fopen(artListPath, "wt");
    if (artListFile) {
        // Write concise header
        const char* header = "==============================================================================\n"
                             "Fallout Fission - Art Asset Report\n"
                             "==============================================================================\n"
                             "This report shows how art assets are loaded - essential for mod debugging, and\n"
                             "finding IDs for mod art assets.\n\n"

                             "Key Features:\n"
                             "- Vanilla assets: Protected in lower slots\n"
                             "- Variant assets: HD versions in protected dedicated slots\n"
                             "- Mod assets: Your content in remaining slots via filename hashing\n"
                             "- Use '@original=new_path' to redirect vanilla assets\n\n"

                             "Conflict Markers:\n"
                             "  »  Remapped vanilla asset\n"
                             "  #  Hash collision (needs fixing)\n\n"

                             "Quick Tips:\n"
                             "- See end of file for Conflict details\n"
                             "- Fix # collisions by renaming files\n"
                             "- Use » remaps only for necessary path changes\n"
                             "- List new assets in mod_*.lst files\n"
                             "==============================================================================\n\n";

        fputs(header, artListFile);

        // Write timestamp
        time_t now = time(0);
        struct tm* t = localtime(&now);
        fprintf(artListFile, "Report Generated: %04d-%02d-%02d %02d:%02d:%02d\n\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

        // Write asset lists for each category
        for (int objectType = 0; objectType < OBJ_TYPE_COUNT; objectType++) {
            ArtListDescription* desc = &gArtListDescriptions[objectType];
            desc->categoryFull = false; // Initialize as not full

            // Count actual mod assets (non-empty slots in mod range)
            int actualModAssets = 0;
            int modStart = desc->vanillaCount + desc->variantCount;
            for (int i = modStart; i < desc->fileNamesLength; i++) {
                if (desc->fileNames[i * FILENAME_LENGTH] != '\0') {
                    actualModAssets++;
                }
            }

            // Calculate total assets
            int totalAssets = desc->vanillaCount + desc->variantCount + actualModAssets;

            // Category header with accurate counts and ranges
            fprintf(artListFile,
                "[%s] (%d assets)\n"
                "Vanilla: %d assets | Variants: %d assets | Mods: %d assets\n"
                "------------------------------------------------------------\n"
                "Slot Ranges:\n"
                "  Vanilla: 0-%d\n"
                "  Variants: %d-%d\n"
                "  Mods: %d-%d\n"
                "------------------------------------------------------------\n",
                desc->name,
                totalAssets,
                desc->vanillaCount,
                desc->variantCount,
                actualModAssets,
                desc->vanillaCount - 1,
                desc->vanillaCount,
                desc->vanillaCount + desc->variantCount - 1,
                4096, // Start of mods
                MAX_ART_INDICES - 1); // End of mods (8191)

            // Add capacity warning if category is full
            if (desc->categoryFull) {
                fprintf(artListFile,
                    "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
                    "! CATEGORY FULL: %d/4096 slots used                      !\n"
                    "! No new mod assets can be added to this category        !\n"
                    "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n",
                    desc->vanillaCount + desc->variantCount);
            }

            // Vanilla assets
            fputs("VANILLA ASSETS:\n", artListFile);
            for (int i = 0; i < desc->vanillaCount; i++) {
                const char* filename = desc->fileNames + i * FILENAME_LENGTH;
                if (filename[0] != '\0') {
                    fprintf(artListFile, "  %5d: %s\n", i, filename);
                }
            }

            // Variant assets
            if (desc->variantCount > 0) {
                fputs("\nVARIANT ASSETS:\n", artListFile);
                for (int i = desc->vanillaCount; i < desc->vanillaCount + desc->variantCount; i++) {
                    const char* filename = desc->fileNames + i * FILENAME_LENGTH;
                    if (filename[0] != '\0') {
                        fprintf(artListFile, "  %5d: %s\n", i, filename);
                    }
                }
            }

            // Mod assets reporting
            if (desc->modCount > 0) {
                fputs("\nMOD ASSETS:\n", artListFile);
                // Only report the extended range (4096-8191)
                for (int i = 4096; i < MAX_ART_INDICES; i++) {
                    const char* filename = desc->fileNames + i * FILENAME_LENGTH;
                    if (filename[0] != '\0') {
                        fprintf(artListFile, "  %5d: %s\n", i, filename);
                    }
                }
            }

            // Add collision/remap details
            if (desc->collisionOccurred) {
                fputs("\n  --- CONFLICT DETAILS ---\n", artListFile);
                for (int i = 0; i < 4096; i++) {
                    if (desc->collisionDetails[i][0] != '\0') {
                        // Different prefix for remap vs collision
                        const char* prefix = "! ";
                        if (strstr(desc->collisionDetails[i], "REMAP:")) {
                            prefix = "» ";
                        } else if (strstr(desc->collisionDetails[i], "COLLISION:")) {
                            prefix = "# ";
                        }

                        fprintf(artListFile, "  %s%5d: %s\n", prefix, i, desc->collisionDetails[i]);
                    }
                }
            }

            // Category separator
            fputs("\n\n", artListFile);
        }

        // Add final summary
        fputs("\n=== SUMMARY ===\n", artListFile);

        // Determine if there are any remaps or collisions
        bool anyRemaps = false;
        bool anyCollisions = false;
        for (int objectType = 0; objectType < OBJ_TYPE_COUNT; objectType++) {
            ArtListDescription* desc = &gArtListDescriptions[objectType];
            for (int i = 0; i < 4096; i++) {
                if (strstr(desc->collisionDetails[i], "REMAP:")) {
                    anyRemaps = true;
                } else if (strstr(desc->collisionDetails[i], "COLLISION:")) {
                    anyCollisions = true;
                }
            }
        }

        if (anyRemaps) {
            fputs("» Remaps: Vanilla assets redirected to new paths\n", artListFile);
        }

        if (anyCollisions) {
            fputs("# Collisions: Hash conflicts detected\n"
                  "  WARNING: May cause asset loading issues!\n"
                  "  Recommendation: Rename files to resolve\n",
                artListFile);
        }

        if (!anyRemaps && !anyCollisions) {
            fputs("No conflicts detected - all assets loaded cleanly\n", artListFile);
        }

        fputs("\nLegend:\n"
              "  !  - Asset remap\n"
              "  #  - Hash collision\n"
              "  »  - Vanilla asset redirected\n",
            artListFile);

        fclose(artListFile);
    }

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
    int rotation = FID_ROTATION(fid);
    int aliasedFid = artAliasFid(fid);
    if (aliasedFid != -1) {
        fid = aliasedFid;
    }

    // Clear global buffer
    *_art_name = '\0';

    // Extract FID components
    int id = artGetIndex(fid); // Use helper function to get actual index
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
                rotation + '0');
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
                'f',
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

        // MOD FIX: Handle paths with subdirectories
        if (strchr(fileName, '/') || strchr(fileName, '\\')) {
            // File is in a subdirectory - use direct path
            snprintf(basePath, sizeof(basePath),
                "%sart\\%s\\%s",
                _cd_path_base,
                gArtListDescriptions[objectType].name,
                fileName);
        } else {
            // Standard file - append .frm if needed
            snprintf(basePath, sizeof(basePath),
                "%sart\\%s\\%s",
                _cd_path_base,
                gArtListDescriptions[objectType].name,
                fileName);

            size_t len = strlen(basePath);
            if (len < 4 || compat_stricmp(basePath + len - 4, ".frm") != 0) {
                if (len < sizeof(basePath) - 5) {
                    strcat(basePath, ".frm");
                } else {
                    debugPrint("Path too long: %s", basePath);
                    return nullptr;
                }
            }
        }

        // Copy to global buffer
        strncpy(_art_name, basePath, sizeof(_art_name));
        _art_name[sizeof(_art_name) - 1] = '\0';

        // Normalize path separators (cross-platform fix)
        for (char* p = _art_name; *p; p++) {
            if (*p == '\\')
                *p = '/';
        }
    }
    return _art_name;
}

// art_read_lst
// 0x419664
static int artReadList(const char* path, char** artListPtr, int* artListSizePtr)
{
    File* stream = fileOpen(path, "rt");
    if (!stream) {
        return -1;
    }

    // First pass: count valid non-empty lines
    int validLineCount = 0;
    char buffer[256];

    while (fileReadString(buffer, sizeof(buffer), stream)) {
        char* p = buffer;
        // Trim leading whitespace
        while (*p && isspace((unsigned char)*p))
            p++;

        // Count non-empty lines
        if (*p != '\0')
            validLineCount++;
    }

    // Handle empty file case
    if (validLineCount == 0) {
        fileClose(stream);
        return -1;
    }

    // Allocate memory for filenames
    char* artList = (char*)internal_malloc(FILENAME_LENGTH * validLineCount);
    *artListPtr = artList;
    *artListSizePtr = validLineCount;

    if (!artList) {
        fileClose(stream);
        return -1;
    }

    // Reset file pointer for second pass
    fileSeek(stream, 0, SEEK_SET);
    int storedCount = 0;

    // Second pass: extract filenames
    while (fileReadString(buffer, sizeof(buffer), stream)) {
        char* start = buffer;
        // Trim leading whitespace
        while (*start && isspace((unsigned char)*start))
            start++;

        // Skip empty lines
        if (*start == '\0')
            continue;

        // Trim trailing whitespace
        char* end = start + strlen(start) - 1;
        while (end > start && isspace((unsigned char)*end))
            end--;
        *(end + 1) = '\0';

        // Truncate at first delimiter
        char* delimiter = strpbrk(start, " ,;\r\t\n");
        if (delimiter)
            *delimiter = '\0';

        // Validate filename length
        size_t len = strlen(start);
        if (len == 0 || len >= FILENAME_LENGTH)
            continue;

        // Store filename
        strncpy(artList, start, FILENAME_LENGTH - 1);
        artList[FILENAME_LENGTH - 1] = '\0';
        artList += FILENAME_LENGTH;
        storedCount++;
    }

    // Update actual count
    *artListSizePtr = storedCount;

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

    // Preserve extended flag
    unsigned int ext_flag = fid & 0x80000000;

    if (type == OBJ_TYPE_CRITTER) {
        if (anim == ANIM_ELECTRIFY
            || anim == ANIM_BURNED_TO_NOTHING
            || anim == ANIM_ELECTRIFIED_TO_NOTHING
            || anim == ANIM_ELECTRIFY_SF
            || anim == ANIM_BURNED_TO_NOTHING_SF
            || anim == ANIM_ELECTRIFIED_TO_NOTHING_SF
            || anim == ANIM_FIRE_DANCE
            || anim == ANIM_CALLED_SHOT_PIC) {
            // Preserve extended flag in the aliased FID
            return ext_flag | (fid & 0x70000000) | ((anim << 16) & 0xFF0000) | 0x1000000 | (fid & 0xF000) | (_anon_alias[fid & 0xFFF] & 0xFFF);
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
            // Skip past "art/" to get the relative path within art directory
            const char* relativePath = artFilePath;
            if (strncmp(artFilePath, "art/", 4) == 0 || strncmp(artFilePath, "art\\", 4) == 0) {
                relativePath = artFilePath + 4;
            }

            char localizedPath[COMPAT_MAX_PATH];
            snprintf(localizedPath, sizeof(localizedPath), "art\\%s\\%s", gArtLanguage, relativePath);

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
            // Skip past "art/" to get the relative path within art directory
            const char* relativePath = artFileName;
            if (strncmp(artFileName, "art/", 4) == 0 || strncmp(artFileName, "art\\", 4) == 0) {
                relativePath = artFileName + 4;
            }

            char localizedPath[COMPAT_MAX_PATH];
            snprintf(localizedPath, sizeof(localizedPath), "art\\%s\\%s", gArtLanguage, relativePath);

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

/* FID Structure:
    3 bits for rotation
    4 bits for object type
    8 bits for animation type
    4 bits for weapon code
    12 bits for frame ID
*/
static int buildFidInternal(unsigned short frmId, unsigned char weaponCode, unsigned char animType, unsigned char objectType, unsigned char rotation)
{
    unsigned int ext_flag = 0;

    // Handle extended indices (4096-8191)
    if (frmId >= 4096 && frmId < 8192) {
        ext_flag = 0x80000000; // Set bit 31
        frmId -= 4096; // Store index relative to 0-4095 range
    } else if (frmId >= 8192) {
        // Error handling for out-of-range indices
        debugPrint("WARNING: art index %d exceeds maximum allowed value (8191)", frmId);
    }

    return ext_flag | ((rotation << 28) & 0x70000000) | (objectType << 24) | ((animType << 16) & 0xFF0000) | ((weaponCode << 12) & 0xF000) | (frmId & 0xFFF); // Last 12 bits store the index
}

// 0x419C88
int buildFid(int objectType, int frmId, int animType, int weaponCode, int rotation)
{
    // Always use rotation 0 (NE) for non-critters, for certain critter animations.
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
    // Changed: Removed static_cast since frmId is now properly handled as int
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
