#include "automap.h"

#include <stdio.h>
#include <string.h>
#include <cmath>

#include <algorithm>

#include "animation.h"
#include "art.h"
#include "color.h"
#include "combat.h"
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
#include "mouse.h"
#include "object.h"
#include "platform_compat.h"
#include "settings.h"
#include "svga.h"
#include "text_font.h"
#include "tile.h"
#include "window_manager.h"

namespace fallout {

#define AUTOMAP_OFFSET_COUNT (AUTOMAP_MAP_COUNT * ELEVATION_COUNT)

#define AUTOMAP_WINDOW_WIDTH (519)
#define AUTOMAP_WINDOW_HEIGHT (480)

#define MINIMAP_WINDOW_WIDTH (250)
#define MINIMAP_WINDOW_HEIGHT (250)

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
    AUTOMAP_FRM_MINIMAP,
    AUTOMAP_FRM_MINIMAP_SWITCH_UP,
    AUTOMAP_FRM_MINIMAP_SWITCH_DOWN,
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

/**
 * Map discovery list: -1 = undiscovered, 0 = discovered/available
 * Initialized for vanilla maps (0-159), mod maps (160-1999) are set to -1
 * Mods can use automapSetDisplayMap() to make their maps available.
 */
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
    4193, // minimap.frm - minimap window
    5958, // autoup_m.frm - minimap up toggle
    4838, // autodn_m.frm - minimap down toggle
};

// 0x5108C4
static int gAutomapFlags = 0;

// 0x56CB18
static AutomapHeader gAutomapHeader;

static int zoom = 2;
static bool gUseNewAutomapProjection = false;
static bool strictVanilla = false;
static double original_scale = 0.5;

// 0x56D2A0
static AutomapEntry gAutomapEntry;

static int automapUpdateEntry(int map, int elevation, const char* tempPath)
{
    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s\\%s", "MAPS", AUTOMAP_DB);

    File* oldStream = fileOpen(path, "rb");
    if (oldStream == nullptr) {
        return -1;
    }

    File* newStream = fileOpen(tempPath, "wb");
    if (newStream == nullptr) {
        fileClose(oldStream);
        return -1;
    }

    // Load header from old file
    if (automapLoadHeader(oldStream) == -1) {
        fileClose(oldStream);
        fileClose(newStream);
        return -1;
    }

    int entryOffset = gAutomapHeader.offsets[map][elevation];

    // Write version 2 header to new file
    gAutomapHeader.version = 2;
    if (automapSaveHeader(newStream) == -1) {
        fileClose(oldStream);
        fileClose(newStream);
        return -1;
    }

    // Copy all entries, replacing the one we're updating
    for (int m = 0; m < AUTOMAP_MAP_COUNT; m++) {
        for (int e = 0; e < ELEVATION_COUNT; e++) {
            int offset = gAutomapHeader.offsets[m][e];
            if (offset <= 0) {
                continue; // Skip negative or zero offsets
            }

            if (m == map && e == elevation) {
                // This is the entry we're updating - write the new one
                long currentPos = fileTell(newStream);
                gAutomapHeader.offsets[m][e] = currentPos;
                if (automapSaveEntry(newStream) == -1) {
                    fileClose(oldStream);
                    fileClose(newStream);
                    return -1;
                }
            } else {
                // Copy the old entry
                if (fileSeek(oldStream, offset, SEEK_SET) == -1) {
                    fileClose(oldStream);
                    fileClose(newStream);
                    return -1;
                }

                int dataSize;
                if (fileReadInt32(oldStream, &dataSize) == -1) {
                    fileClose(oldStream);
                    fileClose(newStream);
                    return -1;
                }

                // Go back to read the whole entry
                fileSeek(oldStream, offset, SEEK_SET);

                long currentPos = fileTell(newStream);
                gAutomapHeader.offsets[m][e] = currentPos;

                if (_copy_file_data(oldStream, newStream, dataSize + 5) == -1) {
                    fileClose(oldStream);
                    fileClose(newStream);
                    return -1;
                }
            }
        }
    }

    // Update header with new offsets
    fileSeek(newStream, 0, SEEK_SET);
    if (automapSaveHeader(newStream) == -1) {
        fileClose(oldStream);
        fileClose(newStream);
        return -1;
    }

    fileClose(oldStream);
    fileClose(newStream);

    return 0;
}

/**
 * Converts entire automap database from version 1 to version 2 format.
 * This is a one-time operation performed when saving an old save for the first time.
 *
 * Adjusts all offsets by 22080 bytes to account for the larger header.
 * Preserves all existing automap data while adding support for mod maps.
 *
 * @return 0 on success, -1 on error
 */
static int automapConvertV1toV2()
{
    char oldPath[COMPAT_MAX_PATH];
    char newPath[COMPAT_MAX_PATH];
    snprintf(oldPath, sizeof(oldPath), "%s\\%s", "MAPS", AUTOMAP_DB);
    snprintf(newPath, sizeof(newPath), "%s\\%s", "MAPS", AUTOMAP_TMP);

    File* oldStream = fileOpen(oldPath, "rb");
    if (oldStream == nullptr) {
        return -1;
    }

    // Read old header
    unsigned char version;
    int dataSize;
    if (fileReadUInt8(oldStream, &version) == -1) {
        fileClose(oldStream);
        return -1;
    }

    if (version != 1) {
        fileClose(oldStream);
        return 0; // Already version 2
    }

    if (_db_freadInt(oldStream, &dataSize) == -1) {
        fileClose(oldStream);
        return -1;
    }

    int oldOffsets[480];
    if (_db_freadIntCount(oldStream, oldOffsets, 480) == -1) {
        fileClose(oldStream);
        return -1;
    }

    // Create new file
    File* newStream = fileOpen(newPath, "wb");
    if (newStream == nullptr) {
        fileClose(oldStream);
        return -1;
    }

    // Write version 2 header
    if (fileWriteUInt8(newStream, 2) == -1) {
        fileClose(oldStream);
        fileClose(newStream);
        return -1;
    }

    // Write dataSize (will update later)
    long dataSizePos = fileTell(newStream);
    if (_db_fwriteLong(newStream, 0) == -1) {
        fileClose(oldStream);
        fileClose(newStream);
        return -1;
    }

    // Write adjusted offsets for first 480 entries
    for (int i = 0; i < 480; i++) {
        int offset = oldOffsets[i];
        if (offset > 0) {
            // Adjust for new header size
            offset += (24005 - 1925); // 22080 bytes
        }
        if (_db_fwriteLong(newStream, offset) == -1) {
            fileClose(oldStream);
            fileClose(newStream);
            return -1;
        }
    }

    // Write zeros for mod maps
    for (int i = 480; i < AUTOMAP_OFFSET_COUNT; i++) {
        if (_db_fwriteLong(newStream, 0) == -1) {
            fileClose(oldStream);
            fileClose(newStream);
            return -1;
        }
    }

    // Copy all data from old file (starts at position 1925 in old file)
    if (fileSeek(oldStream, 1925, SEEK_SET) == -1) {
        fileClose(oldStream);
        fileClose(newStream);
        return -1;
    }

    unsigned char buffer[4096];
    size_t bytesRead;
    while ((bytesRead = fileRead(buffer, 1, sizeof(buffer), oldStream)) > 0) {
        if (fileWrite(buffer, 1, bytesRead, newStream) != bytesRead) {
            fileClose(oldStream);
            fileClose(newStream);
            return -1;
        }
    }

    // Update dataSize in header
    long finalSize = fileTell(newStream);
    fileSeek(newStream, dataSizePos, SEEK_SET);
    if (_db_fwriteLong(newStream, finalSize) == -1) {
        fileClose(oldStream);
        fileClose(newStream);
        return -1;
    }

    fileClose(oldStream);
    fileClose(newStream);

    // Replace old file
    char automapDbPath[512];
    snprintf(automapDbPath, sizeof(automapDbPath), "%s\\%s\\%s",
        settings.system.master_patches_path.c_str(), "MAPS", AUTOMAP_DB);

    compat_remove(automapDbPath);

    char automapTmpPath[512];
    snprintf(automapTmpPath, sizeof(automapTmpPath), "%s\\%s\\%s",
        settings.system.master_patches_path.c_str(), "MAPS", AUTOMAP_TMP);

    if (compat_rename(automapTmpPath, automapDbPath) != 0) {
        return -1;
    }

    return 0;
}

/**
 * Initializes the automap system for expanded 2000-map support.
 * Completes initialization of _displayMapList for mod maps (160-1999).
 *
 * Called once at game startup to set up automap data structures
 * and ensure backward compatibility with existing saves.
 *
 * @return 0 on success
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
 *
 * Clears automap flags and creates a fresh database
 * with expanded 2000-map format.
 *
 * @return 0 on success
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
 * Includes bounds checking for expanded map range (0-1999).
 *
 * @param map Map index to check
 * @return 0 if map is available, -1 if not available or invalid map index
 */
int _automapDisplayMap(int map)
{
    if (map < 0 || map >= AUTOMAP_MAP_COUNT) {
        return -1;
    }
    return _displayMapList[map];
}

static int automapScreenToTile(int relX, int relY, int playerTile, int winWidth, int winHeight)
{
    int uPlayer = playerTile % 200;
    int vPlayer = playerTile / 200;

    // minimap mode constants (must match renderer - move to constants/defines?)
    int clipLeft = 34, clipTop = 29, clipRight = 182, clipBottom = 192;
    int vpCenterX = winWidth / 2 - clipLeft / 2;
    int vpCenterY = winHeight / 2 - clipTop / 2;

    // Player base coordinates (same as in render)
    double playerBaseX, playerBaseY;
    if (gUseNewAutomapProjection) {
        double angleEW = 14.2, angleNS = 37.0;
        double slopeEW = tan(angleEW * M_PI / 180.0);
        double slopeNS = tan(angleNS * M_PI / 180.0);
        playerBaseX = (vPlayer - uPlayer);
        playerBaseY = uPlayer * slopeEW + vPlayer * slopeNS;
    } else {
        playerBaseX = original_scale * (-2 * uPlayer);
        playerBaseY = original_scale * (2 * vPlayer);
    }

    // Reverse zoom and centering
    double targetBaseX = (relX - vpCenterX) / (double)zoom + playerBaseX;
    double targetBaseY = (relY - vpCenterY) / (double)zoom + playerBaseY;

    // Solve for u, v
    double uDouble, vDouble;
    if (gUseNewAutomapProjection) {
        double angleEW = 14.2, angleNS = 37.0;
        double slopeEW = tan(angleEW * M_PI / 180.0);
        double slopeNS = tan(angleNS * M_PI / 180.0);
        double denom = slopeEW + slopeNS;
        if (fabs(denom) < 0.0001) return -1;
        uDouble = (targetBaseY - targetBaseX * slopeNS) / denom;
        vDouble = targetBaseX + uDouble;
    } else {
        uDouble = -targetBaseX / (2.0 * original_scale);
        vDouble =  targetBaseY / (2.0 * original_scale);
    }

    int u = (int)round(uDouble);
    int v = (int)round(vDouble);
    if (u >= 0 && u < 200 && v >= 0 && v < 200)
        return v * 200 + u;
    return -1;
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

    int window;
    // Create automap  or minimap window
    if (strictVanilla) {
        // automap
        int automapWindowX = (screenGetWidth() - AUTOMAP_WINDOW_WIDTH) / 2;
        int automapWindowY = (screenGetHeight() - AUTOMAP_WINDOW_HEIGHT) / 2;
        window = windowCreate(automapWindowX, automapWindowY, AUTOMAP_WINDOW_WIDTH, AUTOMAP_WINDOW_HEIGHT, color, WINDOW_MODAL | WINDOW_MOVE_ON_TOP | WINDOW_TRANSPARENT);
    } else {
        // minimap
        int automapWindowX = 20;
        int automapWindowY = 20;
        window = windowCreate(automapWindowX, automapWindowY, MINIMAP_WINDOW_WIDTH, MINIMAP_WINDOW_HEIGHT, color, WINDOW_MODAL | WINDOW_MOVE_ON_TOP | WINDOW_TRANSPARENT | WINDOW_DRAGGABLE_BY_BACKGROUND);
    }

    // Buttons and button parameters (set based on mode)
    int scannerBtn, cancelBtn, switchBtn1, switchBtn2, switchBtn3;
    int scannerX, scannerY, scannerWidth, scannerHeight, scannerKey;
    int cancelX, cancelY, cancelWidth, cancelHeight, cancelKey;
    int switch1X, switch1Y, switch2X, switch2Y, switch3X, switch3Y;
    int switchWidth, switchHeight;
    int switch1KeyUp, switch1KeyDown, switch2KeyUp, switch2KeyDown, switch3KeyUp, switch3KeyDown;
    int switchFrmUp, switchFrmDown; // FRM indices for switch buttons vary by mode

    if (strictVanilla) {
        // Vanilla mode – original automap layout
        scannerX = 111;
        scannerY = 454;
        scannerWidth = 15;
        scannerHeight = 16;
        scannerKey = KEY_LOWERCASE_S;
        cancelX = 277;
        cancelY = 454;
        cancelWidth = 15;
        cancelHeight = 16;
        cancelKey = KEY_ESCAPE;

        // Only one switch button (details) – others remain unused
        switch1X = 457;
        switch1Y = 340;
        switch2X = 0;
        switch2Y = 0;
        switch3X = 0;
        switch3Y = 0;
        switchWidth = 42;
        switchHeight = 74;
        switch1KeyUp = KEY_LOWERCASE_L;
        switch1KeyDown = KEY_LOWERCASE_H;
        switch2KeyUp = 0;
        switch2KeyDown = 0;
        switch3KeyUp = 0;
        switch3KeyDown = 0;
        switchFrmUp = AUTOMAP_FRM_SWITCH_UP;
        switchFrmDown = AUTOMAP_FRM_SWITCH_DOWN;

    } else {
        // minimap mode – new button layout
        scannerX = 23;
        scannerY = 217;
        scannerWidth = 15;
        scannerHeight = 16;
        scannerKey = KEY_LOWERCASE_S;
        cancelX = 141;
        cancelY = 217;
        cancelWidth = 15;
        cancelHeight = 16;
        cancelKey = KEY_ESCAPE;

        // Three switch buttons (details, zoom, projection)
        switch1X = 195;
        switch1Y = 30; // details
        switch2X = 195;
        switch2Y = 93; // zoom
        switch3X = 195;
        switch3Y = 156; // projection
        switchWidth = 42;
        switchHeight = 63;
        switch1KeyUp = KEY_LOWERCASE_L;
        switch1KeyDown = KEY_LOWERCASE_H;
        switch2KeyUp = KEY_LOWERCASE_I;
        switch2KeyDown = KEY_LOWERCASE_O;
        switch3KeyUp = KEY_LOWERCASE_D;
        switch3KeyDown = KEY_LOWERCASE_T;
        switchFrmUp = AUTOMAP_FRM_MINIMAP_SWITCH_UP;
        switchFrmDown = AUTOMAP_FRM_MINIMAP_SWITCH_DOWN;
    }

    // Scanner (always present)
    scannerBtn = buttonCreate(window, scannerX, scannerY, scannerWidth, scannerHeight,
        -1, -1, -1, scannerKey,
        frmImages[AUTOMAP_FRM_BUTTON_UP].getData(),
        frmImages[AUTOMAP_FRM_BUTTON_DOWN].getData(),
        nullptr, BUTTON_FLAG_TRANSPARENT);
    if (scannerBtn != -1) {
        buttonSetCallbacks(scannerBtn, _gsound_red_butt_press, _gsound_red_butt_release);
    }

    // Cancel (always present)
    cancelBtn = buttonCreate(window, cancelX, cancelY, cancelWidth, cancelHeight,
        -1, -1, -1, cancelKey,
        frmImages[AUTOMAP_FRM_BUTTON_UP].getData(),
        frmImages[AUTOMAP_FRM_BUTTON_DOWN].getData(),
        nullptr, BUTTON_FLAG_TRANSPARENT);
    if (cancelBtn != -1) {
        buttonSetCallbacks(cancelBtn, _gsound_red_butt_press, _gsound_red_butt_release);
    }

    // Switch buttons
    // Details (always present)
    switchBtn1 = buttonCreate(window, switch1X, switch1Y, switchWidth, switchHeight,
        -1, -1, switch1KeyUp, switch1KeyDown,
        frmImages[switchFrmUp].getData(),
        frmImages[switchFrmDown].getData(),
        nullptr, BUTTON_FLAG_TRANSPARENT | BUTTON_FLAG_0x01);
    if (switchBtn1 != -1) {
        buttonSetCallbacks(switchBtn1, _gsound_toggle_butt_press_, _gsound_toggle_butt_press_);
    }
    if ((gAutomapFlags & AUTOMAP_WTH_HIGH_DETAILS) == 0) {
        _win_set_button_rest_state(switchBtn1, 1, 0);
    }
    if (!strictVanilla) {
        // Zoom
        switchBtn2 = buttonCreate(window, switch2X, switch2Y, switchWidth, switchHeight,
            -1, -1, switch2KeyUp, switch2KeyDown,
            frmImages[switchFrmUp].getData(),
            frmImages[switchFrmDown].getData(),
            nullptr, BUTTON_FLAG_TRANSPARENT | BUTTON_FLAG_0x01);
        if (switchBtn2 != -1) {
            buttonSetCallbacks(switchBtn2, _gsound_toggle_butt_press_, _gsound_toggle_butt_press_);
        }
        // Projection
        switchBtn3 = buttonCreate(window, switch3X, switch3Y, switchWidth, switchHeight,
            -1, -1, switch3KeyUp, switch3KeyDown,
            frmImages[switchFrmUp].getData(),
            frmImages[switchFrmDown].getData(),
            nullptr, BUTTON_FLAG_TRANSPARENT | BUTTON_FLAG_0x01);
        if (switchBtn3 != -1) {
            buttonSetCallbacks(switchBtn3, _gsound_toggle_butt_press_, _gsound_toggle_butt_press_);
        }
        if (zoom == 2) {
            _win_set_button_rest_state(switchBtn2, 1, 0);
        }
        if (gUseNewAutomapProjection) {
            _win_set_button_rest_state(switchBtn3, 1, 0);
        }
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
    if (strictVanilla) {
        automapRenderInMapWindow(window, elevation, frmImages[AUTOMAP_FRM_BACKGROUND].getData(), gAutomapFlags);
    } else {
        automapRenderInMapWindow(window, elevation, frmImages[AUTOMAP_FRM_MINIMAP].getData(), gAutomapFlags);
    }

    bool isoWasEnabled;
    if (strictVanilla) {
        isoWasEnabled = isoDisable();
    }
    gameMouseSetCursor(MOUSE_CURSOR_ARROW);

    bool done = false;
    unsigned int lastUpdateTime = getTicks();
    const unsigned int UPDATE_INTERVAL_MS = 50;

    while (!done) {
        sharedFpsLimiter.mark();

        bool needsRefresh = false;

        // --- Mouse handling with click-to-move ---
        int mouseX, mouseY;
        mouseGetPosition(&mouseX, &mouseY);
        int mouseState = mouseGetEvent();

        // --- Mouse handling with click-to-move (minimap mode only) ---
        if (!strictVanilla && mouseState != 0) {
            Rect winRect;
            if (windowGetRect(window, &winRect) == 0) {
                int winX = winRect.left;
                int winY = winRect.top;
                int winW = winRect.right - winRect.left;
                int winH = winRect.bottom - winRect.top;

                int relX = mouseX - winX;
                int relY = mouseY - winY;

                if (relX >= 34 && relX <= 182 && relY >= 29 && relY <= 192) {
                    if (mouseState & MOUSE_EVENT_LEFT_BUTTON_UP) {
                        int targetTile = automapScreenToTile(relX, relY, gDude->tile, winW, winH);
                        if (targetTile != -1 && targetTile != gDude->tile) {
                            // Stop any current movement
                            reg_anim_clear(gDude);

                            int actionPoints = isInCombat() ? _combat_free_move + gDude->data.critter.combat.ap : -1;
                            bool shiftPressed = (gPressedPhysicalKeys[SDL_SCANCODE_LSHIFT] ||
                                                gPressedPhysicalKeys[SDL_SCANCODE_RSHIFT]);
                            bool running = settings.preferences.running;
                            bool shouldRun = (running && !shiftPressed) || (!running && shiftPressed);

                            reg_anim_begin(ANIMATION_REQUEST_RESERVED);
                            if (shouldRun) {
                                animationRegisterRunToTile(gDude, targetTile, gDude->elevation, actionPoints, 0);
                            } else {
                                animationRegisterMoveToTile(gDude, targetTile, gDude->elevation, actionPoints, 0);
                            }
                            reg_anim_end();

                            needsRefresh = true;
                        }
                    } else if (mouseState & MOUSE_EVENT_RIGHT_BUTTON_DOWN) {
                        _gmouse_handle_event(mouseX, mouseY, mouseState);
                    }
                }
            }
        }

        // Always pass clicks outside the automap window to the game
        if (mouseState != 0 && windowGetAtPoint(mouseX, mouseY) != window) {
            _gmouse_handle_event(mouseX, mouseY, mouseState);
        }

        // --- Keyboard handling (your existing code) ---
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
        case KEY_UPPERCASE_I:
        case KEY_LOWERCASE_I:
            zoom = 2;
            needsRefresh = true;

            break;
        case KEY_UPPERCASE_O:
        case KEY_LOWERCASE_O:
            zoom = 1;
            needsRefresh = true;
            break;
        case KEY_UPPERCASE_D:
        case KEY_LOWERCASE_D:
            gUseNewAutomapProjection = true;
            needsRefresh = true;
            break;
        case KEY_UPPERCASE_T:
        case KEY_LOWERCASE_T:
            gUseNewAutomapProjection = false;
            needsRefresh = true;
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

        // --- Throttled automap update ---
        unsigned int currentTime = getTicks();
        if (currentTime - lastUpdateTime >= UPDATE_INTERVAL_MS) {
            needsRefresh = true;
            lastUpdateTime = currentTime;
        }

        // --- Render automap if needed ---
        if (needsRefresh) {
            if (strictVanilla) {
                automapRenderInMapWindow(window, elevation,
                    frmImages[AUTOMAP_FRM_BACKGROUND].getData(),
                    gAutomapFlags);
            } else {
                automapRenderInMapWindow(window, elevation,
                    frmImages[AUTOMAP_FRM_MINIMAP].getData(),
                    gAutomapFlags);
            }
        }

        if (_game_user_wants_to_quit) break;

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    // isoWasEnabled not set for minimap mode to allow click/actions behind/outside window
    if (isoWasEnabled && strictVanilla) {
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
    // Get actual window dimensions (set in automapShow)
    int winWidth = windowGetWidth(window);
    int winHeight = windowGetHeight(window);

    static double original_scale = 0.5; //

    int color;
    if ((flags & AUTOMAP_IN_GAME) != 0) {
        color = _colorTable[8456];
    } else {
        color = _colorTable[22025];
    }

    windowFill(window, 0, 0, winWidth, winHeight, color);
    windowDrawBorder(window);

    unsigned char* windowBuffer = windowGetBuffer(window);
    blitBufferToBuffer(backgroundData, winWidth, winHeight, winWidth, windowBuffer, winWidth);

    // ===== VANILLA MODE – use original fixed rendering =====
    if (strictVanilla) {
        for (Object* obj = objectFindFirstAtElevation(elevation); obj != nullptr; obj = objectFindNextAtElevation()) {
            if (obj->tile == -1) continue;

            int tileX = obj->tile % 200;
            int tileY = obj->tile / 200;
            int objectType = FID_TYPE(obj->fid);
            unsigned char objColor = _colorTable[0];

            // Color determination (same as before)
            if ((flags & AUTOMAP_IN_GAME) != 0) {
                if (objectType == OBJ_TYPE_CRITTER && (obj->flags & OBJECT_HIDDEN) == 0
                    && (flags & AUTOMAP_WITH_SCANNER) != 0
                    && (obj->data.critter.combat.results & DAM_DEAD) == 0) {
                    objColor = _colorTable[31744];
                } else {
                    if ((obj->flags & OBJECT_SEEN) == 0) continue;
                    if (obj->pid == PROTO_ID_0x2000031)
                        objColor = _colorTable[32328];
                    else if (objectType == OBJ_TYPE_WALL)
                        objColor = _colorTable[992];
                    else if (objectType == OBJ_TYPE_SCENERY && (flags & AUTOMAP_WTH_HIGH_DETAILS) != 0
                        && obj->pid != PROTO_ID_0x2000158)
                        objColor = _colorTable[480];
                    else if (obj == gDude)
                        objColor = _colorTable[31744];
                    else
                        objColor = _colorTable[0];
                }
            } else {
                switch (objectType) {
                case OBJ_TYPE_ITEM:
                    objColor = _colorTable[6513];
                    break;
                case OBJ_TYPE_CRITTER:
                    objColor = _colorTable[28672];
                    break;
                case OBJ_TYPE_SCENERY:
                    objColor = _colorTable[448];
                    break;
                case OBJ_TYPE_WALL:
                    objColor = _colorTable[12546];
                    break;
                case OBJ_TYPE_MISC:
                    objColor = _colorTable[31650];
                    break;
                default:
                    objColor = _colorTable[0];
                    break;
                }
            }

            if (objColor == _colorTable[0]) continue;

            // Original Fallout automap formula
            int v10 = -2 * tileX - 10 + winWidth * (2 * tileY + 9) - 60;
            unsigned char* pixel = windowBuffer + v10;

            if ((flags & AUTOMAP_IN_GAME) != 0) {
                if (*pixel != _colorTable[992] || objColor != _colorTable[480]) {
                    pixel[0] = objColor;
                    pixel[1] = objColor;
                }
                if (obj == gDude) {
                    pixel[-1] = objColor;
                    pixel[-winWidth] = objColor;
                    pixel[winWidth] = objColor;
                }
            } else {
                pixel[0] = objColor;
                pixel[1] = objColor;
                pixel[winWidth] = objColor;
                pixel[winWidth + 1] = objColor;
                pixel[winWidth - 1] = objColor;
                pixel[winWidth + 2] = objColor;
                pixel[winWidth * 2] = objColor;
                pixel[winWidth * 2 + 1] = objColor;
            }
        }
    }
    // ===== MINIMAP MODE – player?centered projection with zoom & toggles =====
    else {
        // Clipping rectangle (map display area)
        int clipLeft = 34, clipTop = 29, clipRight = 182, clipBottom = 192;

        // Viewport center (used for player centering)
        int vpCenterX = winWidth / 2 - clipLeft / 2;
        int vpCenterY = winHeight / 2 - clipTop / 2;

        // Player tile
        int playerTile = gDude->tile;
        int uPlayer = playerTile % 200;
        int vPlayer = playerTile / 200;

        // Player base coordinates (before zoom)
        double playerBaseX = 0.0, playerBaseY = 0.0;
        if (gUseNewAutomapProjection) {
            double angleEW = 14.2, angleNS = 37.0;
            double slopeEW = tan(angleEW * M_PI / 180.0);
            double slopeNS = tan(angleNS * M_PI / 180.0);
            playerBaseX = (vPlayer - uPlayer);
            playerBaseY = uPlayer * slopeEW + vPlayer * slopeNS;
        } else {
            playerBaseX = original_scale * (-2 * uPlayer);
            playerBaseY = original_scale * (2 * vPlayer);
        }

        // Object loop
        for (Object* obj = objectFindFirstAtElevation(elevation); obj != nullptr; obj = objectFindNextAtElevation()) {
            if (obj->tile == -1) continue;

            int u = obj->tile % 200;
            int v = obj->tile / 200;

            // Color determination (same as before, copy from above)
            int objectType = FID_TYPE(obj->fid);
            unsigned char objColor = _colorTable[0];
            if ((flags & AUTOMAP_IN_GAME) != 0) {
                if (objectType == OBJ_TYPE_CRITTER && (obj->flags & OBJECT_HIDDEN) == 0
                    && (flags & AUTOMAP_WITH_SCANNER) != 0
                    && (obj->data.critter.combat.results & DAM_DEAD) == 0) {
                    objColor = _colorTable[31744];
                } else {
                    if ((obj->flags & OBJECT_SEEN) == 0) continue;
                    if (obj->pid == PROTO_ID_0x2000031)
                        objColor = _colorTable[32328];
                    else if (objectType == OBJ_TYPE_WALL)
                        objColor = _colorTable[992];
                    else if (objectType == OBJ_TYPE_SCENERY && (flags & AUTOMAP_WTH_HIGH_DETAILS) != 0
                        && obj->pid != PROTO_ID_0x2000158)
                        objColor = _colorTable[480];
                    else if (obj == gDude)
                        objColor = _colorTable[31744];
                    else
                        objColor = _colorTable[0];
                }
            } else {
                switch (objectType) {
                case OBJ_TYPE_ITEM:
                    objColor = _colorTable[6513];
                    break;
                case OBJ_TYPE_CRITTER:
                    objColor = _colorTable[28672];
                    break;
                case OBJ_TYPE_SCENERY:
                    objColor = _colorTable[448];
                    break;
                case OBJ_TYPE_WALL:
                    objColor = _colorTable[12546];
                    break;
                case OBJ_TYPE_MISC:
                    objColor = _colorTable[31650];
                    break;
                default:
                    objColor = _colorTable[0];
                    break;
                }
            }

            if (objColor == _colorTable[0]) continue;

            // Object base coordinates
            double objBaseX, objBaseY;
            if (gUseNewAutomapProjection) {
                double angleEW = 14.2, angleNS = 37.0;
                double slopeEW = tan(angleEW * M_PI / 180.0);
                double slopeNS = tan(angleNS * M_PI / 180.0);
                objBaseX = (v - u);
                objBaseY = u * slopeEW + v * slopeNS;
            } else {
                objBaseX = original_scale * (-2 * u);
                objBaseY = original_scale * (2 * v);
            }

            // Apply zoom and center on player
            int screenX = (int)(zoom * (objBaseX - playerBaseX) + vpCenterX + 0.5);
            int screenY = (int)(zoom * (objBaseY - playerBaseY) + vpCenterY + 0.5);

            // Clip
            if (screenX < clipLeft || screenX + 1 > clipRight || screenY < clipTop || screenY + 1 > clipBottom)
                continue;

            int v10 = screenY * winWidth + screenX;
            unsigned char* pixel = windowBuffer + v10;

            // Drawing (same as before)
            if ((flags & AUTOMAP_IN_GAME) != 0) {
                if (*pixel != _colorTable[992] || objColor != _colorTable[480]) {
                    pixel[0] = objColor;
                    pixel[1] = objColor;
                }
                if (obj == gDude) {
                    pixel[-1] = objColor;
                    pixel[-winWidth] = objColor;
                    pixel[winWidth] = objColor;
                }
            } else {
                pixel[0] = objColor;
                pixel[1] = objColor;
                pixel[winWidth] = objColor;
                pixel[winWidth + 1] = objColor;
                pixel[winWidth - 1] = objColor;
                pixel[winWidth + 2] = objColor;
                pixel[winWidth * 2] = objColor;
                pixel[winWidth * 2 + 1] = objColor;
            }
        }
    }

    // Map name and area name (same for both modes)
    int textColor = ((flags & AUTOMAP_IN_GAME) != 0) ? _colorTable[992] : _colorTable[12546];
    if (mapGetCurrentMap() != -1) {
        char* areaName = mapGetCityName(mapGetCurrentMap());
        char* mapName = mapGetName(mapGetCurrentMap(), elevation);
        if (strictVanilla) {
            windowDrawText(window, areaName, 240, 150, 380, textColor | 0x2000000);
            windowDrawText(window, mapName, 240, 150, 396, textColor | 0x2000000);
        } else {
            int areaWidth = fontGetStringWidth(areaName);
            windowDrawText(window, areaName, areaWidth, (220 - areaWidth) / 2, 182, textColor | 0x2000000);
        }
    }

    windowRefresh(window);
}

/**
 * Renders automap in pipboy window with bounds checking for expanded map range.
 * Contains a known buffer overflow bug in the original rendering loop.
 *
 * @param window Window handle for rendering
 * @param map Map index (0-1999)
 * @param elevation Elevation level (0-2)
 * @return 0 on success, -1 on error or invalid map/elevation
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
 * Saves automap data for the current location to the database.
 * Handles both new entries and updates to existing entries.
 *
 * For version 1 files, triggers automatic conversion to version 2 format.
 * For mod maps (160-1999), initializes offsets to 0 on first save.
 *
 * @return 1 on success, 0 if saving should be skipped, -1 on error
 */
int automapSaveCurrent()
{
    int map = mapGetCurrentMap();
    int elevation = gElevation;

    // Don't save for the first 3 special maps if they have -1 offsets
    if (map < 3) {
        bool shouldSave = false;
        for (int elev = 0; elev < ELEVATION_COUNT; elev++) {
            if (gAutomapHeader.offsets[map][elev] != -1) {
                shouldSave = true;
                break;
            }
        }
        if (!shouldSave) {
            return 0;
        }
    }

    int entryOffset = gAutomapHeader.offsets[map][elevation];
    if (entryOffset < 0) {
        return 0; // Negative offsets mean "don't save"
    }

    debugPrint("\nAUTOMAP: Saving AutoMap DB index %d, level %d\n", map, elevation);

    // Allocate buffers
    gAutomapEntry.data = (unsigned char*)internal_malloc(11024);
    if (gAutomapEntry.data == nullptr) {
        debugPrint("\nAUTOMAP: Error allocating data buffer!\n");
        return -1;
    }

    gAutomapEntry.compressedData = (unsigned char*)internal_malloc(11024);
    if (gAutomapEntry.compressedData == nullptr) {
        debugPrint("\nAUTOMAP: Error allocating compression buffer!\n");
        internal_free(gAutomapEntry.data);
        return -1;
    }

    // Decode current map data
    _decode_map_data(elevation);

    // Compress the data
    int compressedDataSize = graphCompress(gAutomapEntry.data, gAutomapEntry.compressedData, 10000);
    if (compressedDataSize == -1) {
        gAutomapEntry.dataSize = 10000;
        gAutomapEntry.isCompressed = 0;
    } else {
        gAutomapEntry.dataSize = compressedDataSize;
        gAutomapEntry.isCompressed = 1;
    }

    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s\\%s", "MAPS", AUTOMAP_DB);

    File* stream = fileOpen(path, "r+b");
    if (stream == nullptr) {
        debugPrint("\nAUTOMAP: Error opening automap database file!\n");
        internal_free(gAutomapEntry.data);
        internal_free(gAutomapEntry.compressedData);
        return -1;
    }

    // Load current header
    if (automapLoadHeader(stream) == -1) {
        debugPrint("\nAUTOMAP: Error reading automap database header!\n");
        fileClose(stream);
        internal_free(gAutomapEntry.data);
        internal_free(gAutomapEntry.compressedData);
        return -1;
    }

    // Check if we need to convert from version 1 to version 2
    if (gAutomapHeader.version == 1) {
        // We have a version 1 file but we're saving - need to convert to version 2
        fileClose(stream);

        // Convert the entire database
        if (automapConvertV1toV2() == -1) {
            debugPrint("\nAUTOMAP: Error converting database to version 2!\n");
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        // Reopen the now-converted file
        stream = fileOpen(path, "r+b");
        if (stream == nullptr) {
            debugPrint("\nAUTOMAP: Error reopening converted database!\n");
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        // Reload header (now version 2)
        if (automapLoadHeader(stream) == -1) {
            debugPrint("\nAUTOMAP: Error reading converted database header!\n");
            fileClose(stream);
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        // Get the offset again (may have changed due to conversion)
        entryOffset = gAutomapHeader.offsets[map][elevation];
    }

    // Now we have a version 2 database
    if (entryOffset == 0) {
        // New entry - append to end
        if (fileSeek(stream, 0, SEEK_END) == -1) {
            debugPrint("\nAUTOMAP: Error seeking to end of file!\n");
            fileClose(stream);
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        long newOffset = fileTell(stream);
        if (automapSaveEntry(stream) == -1) {
            fileClose(stream);
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        // Update header
        gAutomapHeader.offsets[map][elevation] = newOffset;
        gAutomapHeader.dataSize = fileTell(stream);

        // Write updated header
        fileRewind(stream);
        if (automapSaveHeader(stream) == -1) {
            fileClose(stream);
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        fileClose(stream);

    } else {
        // Existing entry - we need to handle size changes
        // Use a temporary file approach

        fileClose(stream);

        char tempPath[COMPAT_MAX_PATH];
        snprintf(tempPath, sizeof(tempPath), "%s\\%s", "MAPS", AUTOMAP_TMP);

        // Convert and save with the new entry
        if (automapUpdateEntry(map, elevation, tempPath) == -1) {
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }

        // Replace old file with new file
        char automapDbPath[512];
        snprintf(automapDbPath, sizeof(automapDbPath), "%s\\%s\\%s",
            settings.system.master_patches_path.c_str(), "MAPS", AUTOMAP_DB);

        compat_remove(automapDbPath);

        char automapTmpPath[512];
        snprintf(automapTmpPath, sizeof(automapTmpPath), "%s\\%s\\%s",
            settings.system.master_patches_path.c_str(), "MAPS", AUTOMAP_TMP);

        if (compat_rename(automapTmpPath, automapDbPath) != 0) {
            debugPrint("\nAUTOMAP: Error replacing database file!\n");
            internal_free(gAutomapEntry.data);
            internal_free(gAutomapEntry.compressedData);
            return -1;
        }
    }

    internal_free(gAutomapEntry.data);
    internal_free(gAutomapEntry.compressedData);

    return 1;
}

/**
 * Saves an automap entry to file stream.
 * Handles both compressed and uncompressed data formats.
 *
 * Entry format: [4-byte dataSize][1-byte isCompressed][data]
 *
 * @param stream File stream to write entry to
 * @return 0 on success, -1 on error
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
 * Handles decompression if entry was saved with compression.
 *
 * @param map Map index (0-1999)
 * @param elevation Elevation level (0-2)
 * @return 0 on success, -1 on error
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
 * Writes version 2 header with 6000 offset entries.
 *
 * @param stream File stream to write header to
 * @return 0 on success, -1 on error
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
 * Version 1: Original 160-map format (480 offsets)
 * Version 2: Expanded 2000-map format (6000 offsets)
 *
 * When loading version 1 files, offsets are kept as-is in memory to maintain compatibility.
 * Conversion to version 2 happens during the first save operation.
 *
 * @param stream File stream of automap database
 * @return 0 on success, -1 on failure
 */
static int automapLoadHeader(File* stream)
{
    // Read version
    if (fileReadUInt8(stream, &(gAutomapHeader.version)) == -1) {
        return -1;
    }

    // Read dataSize
    if (_db_freadInt(stream, &(gAutomapHeader.dataSize)) == -1) {
        return -1;
    }

    if (gAutomapHeader.version == 1) {
        // Version 1: Read 480 offsets (160 maps × 3 elevations)
        int oldOffsets[480];
        if (_db_freadIntCount(stream, oldOffsets, 480) == -1) {
            return -1;
        }

        // Copy to our header structure (first 480 entries)
        for (int i = 0; i < 480; i++) {
            ((int*)gAutomapHeader.offsets)[i] = oldOffsets[i];
        }

        // Initialize the rest (mod maps) to 0
        for (int i = 480; i < AUTOMAP_OFFSET_COUNT; i++) {
            ((int*)gAutomapHeader.offsets)[i] = 0;
        }

        // Keep as version 1 - we'll convert when we save
        return 0;

    } else if (gAutomapHeader.version == 2) {
        // Version 2: Read all 6000 offsets
        if (_db_freadIntCount(stream, (int*)gAutomapHeader.offsets, AUTOMAP_OFFSET_COUNT) == -1) {
            return -1;
        }
        return 0;
    }

    return -1;
}

/**
 * Decodes current map data into compressed automap format.
 * Converts seen walls and scenery into 2-bit packed representation.
 *
 * Each tile uses 2 bits: 0=empty, 1=wall, 2=scenery
 * Four tiles are packed into each byte (MSB first).
 *
 * @param elevation Current elevation level (0-2)
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
 * Creates a new automap database in version 2 format (2000 maps, 3 elevations each).
 * Only creates file if it doesn't already exist.
 *
 * Initializes all offsets to 0, except for the first 3 tutorial/debug maps
 * which are set to -1 (never save automap data).
 *
 * @return 0 on success, -1 on error
 */
static int automapCreate()
{
    gAutomapHeader.version = 2; // NEW FORMAT
    gAutomapHeader.dataSize = 24005; // 1 + 4 + (2000×3×4)

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
 *
 * @param automapHeaderPtr Pointer to receive header pointer
 * @return 0 on success, -1 on error
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
 * Sets whether a map should be displayed in the automap list.
 * Used by mods to integrate their maps into the automap system.
 *
 * @param map Map index (0-1999)
 * @param available True to display in automap list, false to hide
 */
void automapSetDisplayMap(int map, bool available)
{
    if (map >= 0 && map < AUTOMAP_MAP_COUNT) {
        _displayMapList[map] = available ? 0 : -1;
    }
}

} // namespace fallout
