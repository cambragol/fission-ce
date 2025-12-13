#include "automap.h"

#include <stdio.h>
#include <string.h>

#include <algorithm>

#include "art.h"
#include "color.h"
#include "config.h"
#include "dbox.h"
#include "debug.h"
#include "draw.h"
#include "game.h"
#include "game_mouse.h"
#include "game_sound.h"
#include "graph_lib.h"
#include "input.h"
#include "item.h"
#include "kb.h"
#include "map.h"
#include "memory.h"
#include "object.h"
#include "platform_compat.h"
#include "settings.h"
#include "svga.h"
#include "text_font.h"
#include "window_manager.h"

namespace fallout {

#define AUTOMAP_OFFSET_COUNT (AUTOMAP_MAP_COUNT * ELEVATION_COUNT)

#define AUTOMAP_WINDOW_WIDTH (519)
#define AUTOMAP_WINDOW_HEIGHT (480)

#define AUTOMAP_PIPBOY_VIEW_X (238)
#define AUTOMAP_PIPBOY_VIEW_Y (105)

static void automapRenderInMapWindow(int window, int elevation, unsigned char* backgroundData, int flags);
static int automapSaveEntry(File* stream);
static int automapLoadEntry(int map, int elevation);
static int automapSaveHeader(File* stream);
static int automapLoadHeader(File* stream);
static void _decode_map_data(int elevation);
static int automapCreate();
static int _copy_file_data(File* stream1, File* stream2, int length);

typedef enum AutomapFrm {
    AUTOMAP_FRM_BACKGROUND,
    AUTOMAP_FRM_BUTTON_UP,
    AUTOMAP_FRM_BUTTON_DOWN,
    AUTOMAP_FRM_SWITCH_UP,
    AUTOMAP_FRM_SWITCH_DOWN,
    AUTOMAP_FRM_COUNT,
} AutomapFrm;

typedef struct AutomapEntry {
    int dataSize;
    unsigned char isCompressed;
    unsigned char* compressedData;
    unsigned char* data;
} AutomapEntry;

// Special offset values for first three maps (tutorial/debug maps?)
// Negative values indicate these maps should never save automap data
static const int _defam[AUTOMAP_MAP_COUNT][ELEVATION_COUNT] = {
    { -1, -1, -1 },
    { -1, -1, -1 },
    { -1, -1, -1 },
};

// Map discovery list: -1 = undiscovered, 0 = discovered/available
// Initialized for vanilla maps (0-159), mod maps (160-1999) will be set to -1 in automapInit()
static int _displayMapList[AUTOMAP_MAP_COUNT] = {
    -1,
    -1,
    -1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    -1,
    -1,
    0,
    0,
    0,
    0,
    0,
    -1,
    -1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    0,
    0,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    0,
    0,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    0,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    0,
    -1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    0,
    0,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
};

// FRM IDs for automap interface graphics
static const int gAutomapFrmIds[AUTOMAP_FRM_COUNT] = {
    171, // automap.frm - automap window
    8, // lilredup.frm - little red button up
    9, // lilreddn.frm - little red button down
    172, // autoup.frm - switch up
    173, // autodwn.frm - switch down
};

// 0x5108C4
static int gAutomapFlags = 0;

// 0x56CB18
static AutomapHeader gAutomapHeader;

// 0x56D2A0
static AutomapEntry gAutomapEntry;

/**
 * Initializes the automap system for expanded 2000-map support.
 * Completes initialization of _displayMapList for mod maps (160-1999).
 */
int automapInit()
{
    gAutomapFlags = 0;
    automapCreate();
    return 0;
}

/**
 * Resets automap system to initial state.
 * Should be called when starting a new game.
 */
int automapReset()
{
    gAutomapFlags = 0;
    automapCreate();
    return 0;
}

// 0x41B81C
void automapExit()
{
    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s\\%s\\%s", settings.system.master_patches_path.c_str(), "MAPS", AUTOMAP_DB);
    compat_remove(path);
}

/**
 * Loads automap flags from save file.
 */
int automapLoad(File* stream)
{
    return fileReadInt32(stream, &gAutomapFlags);
}

/**
 * Saves automap flags to save file.
 */
int automapSave(File* stream)
{
    return fileWriteInt32(stream, gAutomapFlags);
}

/**
 * Checks if a map should be displayed in the automap list.
 * Returns 0 if available, -1 if not.
 * Includes bounds checking for expanded map range.
 */
int _automapDisplayMap(int map)
{
    if (map < 0 || map >= AUTOMAP_MAP_COUNT) {
        return -1;
    }
    return _displayMapList[map];
}

/**
 * Shows the full-screen automap interface.
 * Can be called from in-game or from pipboy.
 */
void automapShow(bool isInGame, bool isUsingScanner)
{
    ScopedGameMode gm(GameMode::kAutomap);

    int frmIds[AUTOMAP_FRM_COUNT];
    memcpy(frmIds, gAutomapFrmIds, sizeof(gAutomapFrmIds));

    // Load automap interface graphics
    FrmImage frmImages[AUTOMAP_FRM_COUNT];
    for (int index = 0; index < AUTOMAP_FRM_COUNT; index++) {
        int fid = buildFid(OBJ_TYPE_INTERFACE, frmIds[index], 0, 0, 0);
        if (!frmImages[index].lock(fid)) {
            return;
        }
    }

    int color;
    if (isInGame) {
        color = _colorTable[8456];
        _obj_process_seen();
    } else {
        color = _colorTable[22025];
    }

    // Setup UI
    int oldFont = fontGetCurrent();
    fontSetCurrent(101);
    touch_set_touchscreen_mode(true);

    // Create automap window
    int automapWindowX = (screenGetWidth() - AUTOMAP_WINDOW_WIDTH) / 2;
    int automapWindowY = (screenGetHeight() - AUTOMAP_WINDOW_HEIGHT) / 2;
    // adding WINDOW_TRANSPARENT and WINDOW_DRAGGABLE_BY_BACKGROUND for testing temporarily
    int window = windowCreate(automapWindowX, automapWindowY, AUTOMAP_WINDOW_WIDTH, AUTOMAP_WINDOW_HEIGHT, color, WINDOW_MODAL | WINDOW_MOVE_ON_TOP | WINDOW_TRANSPARENT | WINDOW_DRAGGABLE_BY_BACKGROUND);

    // Create control buttons
    int scannerBtn = buttonCreate(window,
        111,
        454,
        15,
        16,
        -1,
        -1,
        -1,
        KEY_LOWERCASE_S,
        frmImages[AUTOMAP_FRM_BUTTON_UP].getData(),
        frmImages[AUTOMAP_FRM_BUTTON_DOWN].getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (scannerBtn != -1) {
        buttonSetCallbacks(scannerBtn, _gsound_red_butt_press, _gsound_red_butt_release);
    }

    int cancelBtn = buttonCreate(window,
        277,
        454,
        15,
        16,
        -1,
        -1,
        -1,
        KEY_ESCAPE,
        frmImages[AUTOMAP_FRM_BUTTON_UP].getData(),
        frmImages[AUTOMAP_FRM_BUTTON_DOWN].getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (cancelBtn != -1) {
        buttonSetCallbacks(cancelBtn, _gsound_red_butt_press, _gsound_red_butt_release);
    }

    int switchBtn = buttonCreate(window,
        457,
        340,
        42,
        74,
        -1,
        -1,
        KEY_LOWERCASE_L,
        KEY_LOWERCASE_H,
        frmImages[AUTOMAP_FRM_SWITCH_UP].getData(),
        frmImages[AUTOMAP_FRM_SWITCH_DOWN].getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT | BUTTON_FLAG_0x01);
    if (switchBtn != -1) {
        buttonSetCallbacks(switchBtn, _gsound_toggle_butt_press_, _gsound_toggle_butt_press_);
    }

    if ((gAutomapFlags & AUTOMAP_WTH_HIGH_DETAILS) == 0) {
        _win_set_button_rest_state(switchBtn, 1, 0);
    }

    int elevation = gElevation;

    gAutomapFlags &= AUTOMAP_WTH_HIGH_DETAILS;

    if (isInGame) {
        gAutomapFlags |= AUTOMAP_IN_GAME;
    }

    if (isUsingScanner) {
        gAutomapFlags |= AUTOMAP_WITH_SCANNER;
    }

    // Render initial automap view
    automapRenderInMapWindow(window, elevation, frmImages[AUTOMAP_FRM_BACKGROUND].getData(), gAutomapFlags);

    bool isoWasEnabled = isoDisable();
    gameMouseSetCursor(MOUSE_CURSOR_ARROW);

    bool done = false;
    while (!done) {
        sharedFpsLimiter.mark();

        bool needsRefresh = false;

        // FIXME: There is minor bug in the interface - pressing H/L to toggle
        // high/low details does not update switch state.
        int keyCode = inputGetInput();
        switch (keyCode) {
        case KEY_TAB:
        case KEY_ESCAPE:
        case KEY_UPPERCASE_A:
        case KEY_LOWERCASE_A:
            done = true;
            break;
        case KEY_UPPERCASE_H:
        case KEY_LOWERCASE_H:
            if ((gAutomapFlags & AUTOMAP_WTH_HIGH_DETAILS) == 0) {
                gAutomapFlags |= AUTOMAP_WTH_HIGH_DETAILS;
                needsRefresh = true;
            }
            break;
        case KEY_UPPERCASE_L:
        case KEY_LOWERCASE_L:
            if ((gAutomapFlags & AUTOMAP_WTH_HIGH_DETAILS) != 0) {
                gAutomapFlags &= ~AUTOMAP_WTH_HIGH_DETAILS;
                needsRefresh = true;
            }
            break;
        case KEY_UPPERCASE_S:
        case KEY_LOWERCASE_S:
            if (elevation != gElevation) {
                elevation = gElevation;
                needsRefresh = true;
            }

            if ((gAutomapFlags & AUTOMAP_WITH_SCANNER) == 0) {
                Object* scanner = nullptr;

                Object* item1 = critterGetItem1(gDude);
                if (item1 != nullptr && item1->pid == PROTO_ID_MOTION_SENSOR) {
                    scanner = item1;
                } else {
                    Object* item2 = critterGetItem2(gDude);
                    if (item2 != nullptr && item2->pid == PROTO_ID_MOTION_SENSOR) {
                        scanner = item2;
                    }
                }

                if (scanner != nullptr && miscItemGetCharges(scanner) > 0) {
                    needsRefresh = true;
                    gAutomapFlags |= AUTOMAP_WITH_SCANNER;
                    miscItemConsumeCharge(scanner);
                } else {
                    soundPlayFile("iisxxxx1");

                    MessageListItem messageListItem;
                    // 17 - The motion sensor is not installed.
                    // 18 - The motion sensor has no charges remaining.
                    const char* title = getmsg(&gMiscMessageList, &messageListItem, scanner != nullptr ? 18 : 17);
                    showDialogBox(title, nullptr, 0, 165, 140, _colorTable[32328], nullptr, _colorTable[32328], 0);
                }
            }

            break;
        case KEY_CTRL_Q:
        case KEY_ALT_X:
        case KEY_F10:
            showQuitConfirmationDialog();
            break;
        case KEY_F12:
            takeScreenshot();
            break;
        }

        if (_game_user_wants_to_quit != 0) {
            break;
        }

        if (needsRefresh) {
            automapRenderInMapWindow(window, elevation, frmImages[AUTOMAP_FRM_BACKGROUND].getData(), gAutomapFlags);
            needsRefresh = false;
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    if (isoWasEnabled) {
        isoEnable();
    }

    windowDestroy(window);
    fontSetCurrent(oldFont);
    touch_set_touchscreen_mode(false);
}

/**
 * Renders automap in the full-screen map window.
 */
static void automapRenderInMapWindow(int window, int elevation, unsigned char* backgroundData, int flags)
{
    int color;
    if ((flags & AUTOMAP_IN_GAME) != 0) {
        color = _colorTable[8456];
    } else {
        color = _colorTable[22025];
    }

    windowFill(window, 0, 0, AUTOMAP_WINDOW_WIDTH, AUTOMAP_WINDOW_HEIGHT, color);
    windowDrawBorder(window);

    unsigned char* windowBuffer = windowGetBuffer(window);
    blitBufferToBuffer(backgroundData, AUTOMAP_WINDOW_WIDTH, AUTOMAP_WINDOW_HEIGHT, AUTOMAP_WINDOW_WIDTH, windowBuffer, AUTOMAP_WINDOW_WIDTH);

    for (Object* object = objectFindFirstAtElevation(elevation); object != nullptr; object = objectFindNextAtElevation()) {
        if (object->tile == -1) {
            continue;
        }

        int objectType = FID_TYPE(object->fid);
        unsigned char objectColor;

        if ((flags & AUTOMAP_IN_GAME) != 0) {
            if (objectType == OBJ_TYPE_CRITTER
                && (object->flags & OBJECT_HIDDEN) == 0
                && (flags & AUTOMAP_WITH_SCANNER) != 0
                && (object->data.critter.combat.results & DAM_DEAD) == 0) {
                objectColor = _colorTable[31744];
            } else {
                if ((object->flags & OBJECT_SEEN) == 0) {
                    continue;
                }

                if (object->pid == PROTO_ID_0x2000031) {
                    objectColor = _colorTable[32328];
                } else if (objectType == OBJ_TYPE_WALL) {
                    objectColor = _colorTable[992];
                } else if (objectType == OBJ_TYPE_SCENERY
                    && (flags & AUTOMAP_WTH_HIGH_DETAILS) != 0
                    && object->pid != PROTO_ID_0x2000158) {
                    objectColor = _colorTable[480];
                } else if (object == gDude) {
                    objectColor = _colorTable[31744];
                } else {
                    objectColor = _colorTable[0];
                }
            }
        }

        int v10 = -2 * (object->tile % 200) - 10 + AUTOMAP_WINDOW_WIDTH * (2 * (object->tile / 200) + 9) - 60;
        if ((flags & AUTOMAP_IN_GAME) == 0) {
            switch (objectType) {
            case OBJ_TYPE_ITEM:
                objectColor = _colorTable[6513];
                break;
            case OBJ_TYPE_CRITTER:
                objectColor = _colorTable[28672];
                break;
            case OBJ_TYPE_SCENERY:
                objectColor = _colorTable[448];
                break;
            case OBJ_TYPE_WALL:
                objectColor = _colorTable[12546];
                break;
            case OBJ_TYPE_MISC:
                objectColor = _colorTable[31650];
                break;
            default:
                objectColor = _colorTable[0];
            }
        }

        if (objectColor != _colorTable[0]) {
            unsigned char* v12 = windowBuffer + v10;
            if ((flags & AUTOMAP_IN_GAME) != 0) {
                if (*v12 != _colorTable[992] || objectColor != _colorTable[480]) {
                    v12[0] = objectColor;
                    v12[1] = objectColor;
                }

                if (object == gDude) {
                    v12[-1] = objectColor;
                    v12[-AUTOMAP_WINDOW_WIDTH] = objectColor;
                    v12[AUTOMAP_WINDOW_WIDTH] = objectColor;
                }
            } else {
                v12[0] = objectColor;
                v12[1] = objectColor;
                v12[AUTOMAP_WINDOW_WIDTH] = objectColor;
                v12[AUTOMAP_WINDOW_WIDTH + 1] = objectColor;

                v12[AUTOMAP_WINDOW_WIDTH - 1] = objectColor;
                v12[AUTOMAP_WINDOW_WIDTH + 2] = objectColor;
                v12[AUTOMAP_WINDOW_WIDTH * 2] = objectColor;
                v12[AUTOMAP_WINDOW_WIDTH * 2 + 1] = objectColor;
            }
        }
    }

    int textColor;
    if ((flags & AUTOMAP_IN_GAME) != 0) {
        textColor = _colorTable[992];
    } else {
        textColor = _colorTable[12546];
    }

    if (mapGetCurrentMap() != -1) {
        char* areaName = mapGetCityName(mapGetCurrentMap());
        windowDrawText(window, areaName, 240, 150, 380, textColor | 0x2000000);

        char* mapName = mapGetName(mapGetCurrentMap(), elevation);
        windowDrawText(window, mapName, 240, 150, 396, textColor | 0x2000000);
    }

    windowRefresh(window);
}

/**
 * Renders automap in pipboy window.
 * Note: Original code has a known buffer overflow bug in the rendering loop.
 */
int automapRenderInPipboyWindow(int window, int map, int elevation)
{
    // Bounds check
    if (map < 0 || map >= AUTOMAP_MAP_COUNT) {
        return -1;
    }

    if (elevation < 0 || elevation >= ELEVATION_COUNT) {
        return -1;
    }
    unsigned char* windowBuffer = windowGetBuffer(window) + 640 * AUTOMAP_PIPBOY_VIEW_Y + AUTOMAP_PIPBOY_VIEW_X;

    unsigned char wallColor = _colorTable[992];
    unsigned char sceneryColor = _colorTable[480];

    gAutomapEntry.data = (unsigned char*)internal_malloc(11024);
    if (gAutomapEntry.data == nullptr) {
        debugPrint("\nAUTOMAP: Error allocating data buffer!\n");
        return -1;
    }

    if (automapLoadEntry(map, elevation) == -1) {
        internal_free(gAutomapEntry.data);
        return -1;
    }

    int v1 = 0;
    unsigned char v2 = 0;
    unsigned char* ptr = gAutomapEntry.data;

    // FIXME: This loop is implemented incorrectly. Automap requires 400x400 px,
    // but it's top offset is 105, which gives max y 505. It only works because
    // lower portions of automap data contains zeroes. If it doesn't this loop
    // will try to set pixels outside of window buffer, which usually leads to
    // crash.
    for (int y = 0; y < HEX_GRID_HEIGHT; y++) {
        for (int x = 0; x < HEX_GRID_WIDTH; x++) {
            v1 -= 1;
            if (v1 <= 0) {
                v1 = 4;
                v2 = *ptr++;
            }

            switch ((v2 & 0xC0) >> 6) {
            case 1:
                *windowBuffer++ = wallColor;
                *windowBuffer++ = wallColor;
                break;
            case 2:
                *windowBuffer++ = sceneryColor;
                *windowBuffer++ = sceneryColor;
                break;
            default:
                windowBuffer += 2;
                break;
            }

            v2 <<= 2;
        }

        windowBuffer += 640 + 240;
    }

    internal_free(gAutomapEntry.data);

    return 0;
}

/**
 * Saves automap data for the current location.
 * Handles both vanilla and mod maps.
 */
int automapSaveCurrent()
{
    int map = mapGetCurrentMap();
    int elevation = gElevation;

    int entryOffset = gAutomapHeader.offsets[map][elevation];
    if (entryOffset < 0) {
        // Fix negative offsets for mod maps
        if (map >= 160) {
            gAutomapHeader.offsets[map][elevation] = 0;
            entryOffset = 0;
        } else {
            return 0;
        }
    }

    debugPrint("\nAUTOMAP: Saving AutoMap DB index %d, level %d\n", map, elevation);

    bool dataBuffersAllocated = false;
    gAutomapEntry.data = (unsigned char*)internal_malloc(11024);
    if (gAutomapEntry.data != nullptr) {
        gAutomapEntry.compressedData = (unsigned char*)internal_malloc(11024);
        if (gAutomapEntry.compressedData != nullptr) {
            dataBuffersAllocated = true;
        }
    }

    if (!dataBuffersAllocated) {
        debugPrint("\nAUTOMAP: Error allocating data buffers!\n");
        return -1;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s\\%s", "MAPS", AUTOMAP_DB);

    File* stream1 = fileOpen(path, "r+b");
    if (stream1 == nullptr) {
        debugPrint("\nAUTOMAP: Error opening automap database file!\n");
        debugPrint("Error continued: automap_pip_save: path: %s", path);
        internal_free(gAutomapEntry.data);
        internal_free(gAutomapEntry.compressedData);
        return -1;
    }

    if (automapLoadHeader(stream1) == -1) {
        debugPrint("\nAUTOMAP: Error reading automap database file header!\n");
        internal_free(gAutomapEntry.data);
        internal_free(gAutomapEntry.compressedData);
        fileClose(stream1);
        return -1;
    }

    _decode_map_data(elevation);

    int compressedDataSize = graphCompress(gAutomapEntry.data, gAutomapEntry.compressedData, 10000);
    if (compressedDataSize == -1) {
        gAutomapEntry.dataSize = 10000;
        gAutomapEntry.isCompressed = 0;
    } else {
        gAutomapEntry.dataSize = compressedDataSize;
        gAutomapEntry.isCompressed = 1;
    }

    if (entryOffset != 0) {
        snprintf(path, sizeof(path), "%s\\%s", "MAPS", AUTOMAP_TMP);

        File* stream2 = fileOpen(path, "wb");
        if (stream2 == nullptr) {
            debugPrint("\nAUTOMAP: Error creating temp file!\n");
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            fileClose(stream1);
            return -1;
        }

        fileRewind(stream1);

        if (_copy_file_data(stream1, stream2, entryOffset) == -1) {
            debugPrint("\nAUTOMAP: Error copying file data!\n");
            fileClose(stream1);
            fileClose(stream2);
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        if (automapSaveEntry(stream2) == -1) {
            fileClose(stream1);
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        int nextEntryDataSize;
        if (fileReadInt32(stream1, &nextEntryDataSize) == -1) {
            debugPrint("\nAUTOMAP: Error reading database #1!\n");
            fileClose(stream1);
            fileClose(stream2);
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        int automapDataSize = fileGetSize(stream1);
        if (automapDataSize == -1) {
            debugPrint("\nAUTOMAP: Error reading database #2!\n");
            fileClose(stream1);
            fileClose(stream2);
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        int nextEntryOffset = entryOffset + nextEntryDataSize + 5;
        if (automapDataSize != nextEntryOffset) {
            if (fileSeek(stream1, nextEntryOffset, SEEK_SET) == -1) {
                debugPrint("\nAUTOMAP: Error writing temp data!\n");
                fileClose(stream1);
                fileClose(stream2);
                internal_free(gAutomapEntry.data);
                internal_free(gAutomapEntry.compressedData);
                return -1;
            }

            if (_copy_file_data(stream1, stream2, automapDataSize - nextEntryOffset) == -1) {
                debugPrint("\nAUTOMAP: Error copying file data!\n");
                fileClose(stream1);
                fileClose(stream2);
                internal_free(gAutomapEntry.data);
                internal_free(gAutomapEntry.compressedData);
                return -1;
            }
        }

        int diff = gAutomapEntry.dataSize - nextEntryDataSize;
        for (int map = 0; map < AUTOMAP_MAP_COUNT; map++) {
            for (int elevation = 0; elevation < ELEVATION_COUNT; elevation++) {
                if (gAutomapHeader.offsets[map][elevation] > entryOffset) {
                    gAutomapHeader.offsets[map][elevation] += diff;
                }
            }
        }

        gAutomapHeader.dataSize += diff;

        if (automapSaveHeader(stream2) == -1) {
            fileClose(stream1);
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        fileSeek(stream2, 0, SEEK_END);
        fileClose(stream2);
        fileClose(stream1);
        internal_free(gAutomapEntry.data);
        internal_free(gAutomapEntry.compressedData);

        // NOTE: Not sure about the size.
        char automapDbPath[512];
        snprintf(automapDbPath, sizeof(automapDbPath), "%s\\%s\\%s", settings.system.master_patches_path.c_str(), "MAPS", AUTOMAP_DB);
        if (compat_remove(automapDbPath) != 0) {
            debugPrint("\nAUTOMAP: Error removing database!\n");
            return -1;
        }

        // NOTE: Not sure about the size.
        char automapTmpPath[512];
        snprintf(automapTmpPath, sizeof(automapTmpPath), "%s\\%s\\%s", settings.system.master_patches_path.c_str(), "MAPS", AUTOMAP_TMP);
        if (compat_rename(automapTmpPath, automapDbPath) != 0) {
            debugPrint("\nAUTOMAP: Error renaming database!\n");
            return -1;
        }
    } else {
        // Check file size
        bool proceed = true;
        if (fileSeek(stream1, 0, SEEK_END) != -1) {
            long fileSize = fileTell(stream1);
            if (fileSize != gAutomapHeader.dataSize) {
                // Adjust dataSize to match actual file size
                gAutomapHeader.dataSize = fileSize;
            }
        } else {
            proceed = false;
        }

        if (!proceed) {
            debugPrint("\nAUTOMAP: Error reading automap database file header!\n");
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            fileClose(stream1);
            return -1;
        }

        if (automapSaveEntry(stream1) == -1) {
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        gAutomapHeader.offsets[map][elevation] = gAutomapHeader.dataSize;
        gAutomapHeader.dataSize += gAutomapEntry.dataSize + 5;

        if (automapSaveHeader(stream1) == -1) {
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        fileSeek(stream1, 0, SEEK_END);
        fileClose(stream1);
        internal_free(gAutomapEntry.data);
        internal_free(gAutomapEntry.compressedData);
    }

    return 1;
}

/**
 * Saves an automap entry to file stream.
 * Handles both compressed and uncompressed data.
 */
static int automapSaveEntry(File* stream)
{
    unsigned char* buffer;
    if (gAutomapEntry.isCompressed == 1) {
        buffer = gAutomapEntry.compressedData;
    } else {
        buffer = gAutomapEntry.data;
    }

    if (_db_fwriteLong(stream, gAutomapEntry.dataSize) == -1) {
        goto err;
    }

    if (fileWriteUInt8(stream, gAutomapEntry.isCompressed) == -1) {
        goto err;
    }

    if (fileWriteUInt8List(stream, buffer, gAutomapEntry.dataSize) == -1) {
        goto err;
    }
    return 0;

err:
    debugPrint("\nAUTOMAP: Error writing automap database entry data!\n");
    fileClose(stream);
    return -1;
}

/**
 * Loads automap entry from database.
 */
static int automapLoadEntry(int map, int elevation)
{
    gAutomapEntry.compressedData = nullptr;

    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s\\%s", "MAPS", AUTOMAP_DB);

    bool success = true;

    File* stream = fileOpen(path, "r+b");
    if (stream == nullptr) {
        debugPrint("\nAUTOMAP: Error opening automap database file!\n");
        debugPrint("Error continued: AM_ReadEntry: path: %s", path);
        return -1;
    }

    if (automapLoadHeader(stream) == -1) {
        debugPrint("\nAUTOMAP: Error reading automap database header!\n");
        fileClose(stream);
        return -1;
    }

    if (gAutomapHeader.offsets[map][elevation] <= 0) {
        success = false;
        goto out;
    }

    if (fileSeek(stream, gAutomapHeader.offsets[map][elevation], SEEK_SET) == -1) {
        success = false;
        goto out;
    }

    if (_db_freadInt(stream, &(gAutomapEntry.dataSize)) == -1) {
        success = false;
        goto out;
    }

    if (fileReadUInt8(stream, &(gAutomapEntry.isCompressed)) == -1) {
        success = false;
        goto out;
    }

    if (gAutomapEntry.isCompressed == 1) {
        gAutomapEntry.compressedData = (unsigned char*)internal_malloc(11024);
        if (gAutomapEntry.compressedData == nullptr) {
            debugPrint("\nAUTOMAP: Error allocating decompression buffer!\n");
            fileClose(stream);
            return -1;
        }

        if (fileReadUInt8List(stream, gAutomapEntry.compressedData, gAutomapEntry.dataSize) == -1) {
            success = 0;
            goto out;
        }

        if (graphDecompress(gAutomapEntry.compressedData, gAutomapEntry.data, 10000) == -1) {
            debugPrint("\nAUTOMAP: Error decompressing DB entry!\n");
            fileClose(stream);
            return -1;
        }
    } else {
        if (fileReadUInt8List(stream, gAutomapEntry.data, gAutomapEntry.dataSize) == -1) {
            success = false;
            goto out;
        }
    }

out:

    fileClose(stream);

    if (!success) {
        debugPrint("\nAUTOMAP: Error reading automap database entry data!\n");

        return -1;
    }

    if (gAutomapEntry.compressedData != nullptr) {
        internal_free(gAutomapEntry.compressedData);
    }

    return 0;
}

/**
 * Saves automap database header with expanded 2000-map format.
 */
static int automapSaveHeader(File* stream)
{
    fileRewind(stream);

    if (fileWriteUInt8(stream, gAutomapHeader.version) == -1) {
        goto err;
    }

    if (_db_fwriteLong(stream, gAutomapHeader.dataSize) == -1) {
        goto err;
    }

    if (_db_fwriteLongCount(stream, (int*)gAutomapHeader.offsets, AUTOMAP_OFFSET_COUNT) == -1) {
        goto err;
    }

    return 0;

err:
    debugPrint("\nAUTOMAP: Error writing automap database header!\n");
    fileClose(stream);
    return -1;
}

/**
 * Loads automap database header, handling both old (160-map) and new (2000-map) formats.
 * Version 1: Original 160-map format
 * Version 2: Expanded 2000-map format
 */
static int automapLoadHeader(File* stream)
{
    // Read basic header fields
    if (fileReadUInt8(stream, &(gAutomapHeader.version)) == -1) {
        return -1;
    }
    if (_db_freadInt(stream, &(gAutomapHeader.dataSize)) == -1) {
        return -1;
    }

    // Calculate header sizes for both formats
    int oldHeaderSize = 1 + 4 + (160 * 3 * 4); // 1925
    int newHeaderSize = 1 + 4 + (AUTOMAP_MAP_COUNT * 3 * 4); // 24005

    if (gAutomapHeader.version == 1) {
        if (gAutomapHeader.dataSize >= newHeaderSize) {
            // Version 1 file with 2000-map data
            // Read all 6000 offsets
            if (_db_freadIntCount(stream, (int*)gAutomapHeader.offsets, AUTOMAP_OFFSET_COUNT) == -1) {
                return -1;
            }

            // FIX: Clean up mod map offsets (160-1999)
            for (int map = 160; map < AUTOMAP_MAP_COUNT; map++) {
                for (int elev = 0; elev < ELEVATION_COUNT; elev++) {
                    int offset = gAutomapHeader.offsets[map][elev];
                    // If offset is negative or absurdly large, set to 0
                    if (offset < 0 || offset > 10000000) {
                        gAutomapHeader.offsets[map][elev] = 0;
                    }
                }
            }

            // Convert to Version 2 in memory
            gAutomapHeader.version = 2;
        } else if (gAutomapHeader.dataSize >= oldHeaderSize) {
            // True 160-map format
            // Read 480 offsets for maps 0-159
            if (_db_freadIntCount(stream, (int*)gAutomapHeader.offsets, 480) == -1) {
                return -1;
            }

            // Adjust offsets for new header size (only for valid offsets > 0)
            int offsetAdjustment = newHeaderSize - oldHeaderSize; // 22080
            for (int i = 0; i < 480; i++) {
                int oldOffset = ((int*)gAutomapHeader.offsets)[i];
                if (oldOffset > 0) {
                    ((int*)gAutomapHeader.offsets)[i] = oldOffset + offsetAdjustment;
                }
            }

            // Initialize mod map offsets (160-1999) to 0
            for (int i = 480; i < AUTOMAP_OFFSET_COUNT; i++) {
                ((int*)gAutomapHeader.offsets)[i] = 0;
            }

            // Convert to Version 2
            gAutomapHeader.version = 2;
            gAutomapHeader.dataSize = newHeaderSize + (gAutomapHeader.dataSize - oldHeaderSize);
        } else {
            return -1;
        }
    } else if (gAutomapHeader.version == 2) {
        // Read all 6000 offsets
        if (_db_freadIntCount(stream, (int*)gAutomapHeader.offsets, AUTOMAP_OFFSET_COUNT) == -1) {
            return -1;
        }
    } else {
        return -1;
    }

    return 0;
}

/**
 * Decodes current map data into automap format.
 * Converts seen walls and scenery into a compressed representation.
 */
static void _decode_map_data(int elevation)
{
    memset(gAutomapEntry.data, 0, SQUARE_GRID_SIZE);

    _obj_process_seen();

    Object* object = objectFindFirstAtElevation(elevation);
    while (object != nullptr) {
        if (object->tile != -1 && (object->flags & OBJECT_SEEN) != 0) {
            int contentType;

            int objectType = FID_TYPE(object->fid);
            if (objectType == OBJ_TYPE_SCENERY && object->pid != PROTO_ID_0x2000158) {
                contentType = 2;
            } else if (objectType == OBJ_TYPE_WALL) {
                contentType = 1;
            } else {
                contentType = 0;
            }

            if (contentType != 0) {
                int v1 = 200 - object->tile % 200;
                int v2 = v1 / 4 + 50 * (object->tile / 200);
                int v3 = 2 * (3 - v1 % 4);
                gAutomapEntry.data[v2] &= ~(0x03 << v3);
                gAutomapEntry.data[v2] |= (contentType << v3);
            }
        }
        object = objectFindNextAtElevation();
    }
}

/**
 * Creates a new automap database with expanded 2000-map format.
 * Only creates file if it doesn't already exist.
 */
static int automapCreate()
{
    gAutomapHeader.version = 2; // NEW FORMAT
    gAutomapHeader.dataSize = 24005; // 1 + 4 + (2000󫢬)

    // Initialize ALL offsets to 0
    for (int i = 0; i < AUTOMAP_MAP_COUNT; i++) {
        for (int j = 0; j < ELEVATION_COUNT; j++) {
            gAutomapHeader.offsets[i][j] = 0;
        }
    }

    // Copy the first 3 maps from _defam (which are -1)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < ELEVATION_COUNT; j++) {
            gAutomapHeader.offsets[i][j] = _defam[i][j];
        }
    }

    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s\\%s", "MAPS", AUTOMAP_DB);

    File* stream = fileOpen(path, "wb");
    if (stream == nullptr) {
        debugPrint("\nAUTOMAP: Error creating automap database file!\n");
        return -1;
    }

    if (automapSaveHeader(stream) == -1) {
        return -1;
    }

    fileClose(stream);
    return 0;
}

/**
 * Copies data from one file stream to another.
 * Used for updating automap database format.
 */
static int _copy_file_data(File* stream1, File* stream2, int length)
{
    void* buffer = internal_malloc(0xFFFF);
    if (buffer == nullptr) {
        return -1;
    }

    // NOTE: Original code is slightly different, but does the same thing.
    while (length != 0) {
        int chunkLength = std::min(length, 0xFFFF);

        if (fileRead(buffer, chunkLength, 1, stream1) != 1) {
            break;
        }

        if (fileWrite(buffer, chunkLength, 1, stream2) != 1) {
            break;
        }

        length -= chunkLength;
    }

    internal_free(buffer);

    if (length != 0) {
        return -1;
    }

    return 0;
}

/**
 * Gets pointer to automap header structure.
 * Used by pipboy to build automap list.
 */
int automapGetHeader(AutomapHeader** automapHeaderPtr)
{
    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s\\%s", "MAPS", AUTOMAP_DB);

    File* stream = fileOpen(path, "rb");
    if (stream == nullptr) {
        debugPrint("\nAUTOMAP: Error opening database file for reading!\n");
        debugPrint("Error continued: ReadAMList: path: %s", path);
        return -1;
    }

    if (automapLoadHeader(stream) == -1) {
        debugPrint("\nAUTOMAP: Error reading automap database header pt2!\n");
        fileClose(stream);
        return -1;
    }

    fileClose(stream);

    *automapHeaderPtr = &gAutomapHeader;

    return 0;
}

/**
 * Sets whether a map should be displayed in automap list.
 * Used by mods to add their maps to the automap system.
 */
void automapSetDisplayMap(int map, bool available)
{
    if (map >= 0 && map < AUTOMAP_MAP_COUNT) {
        _displayMapList[map] = available ? 0 : -1;
    }
}

} // namespace fallout
