#include "interface.h"

#include <algorithm>
#include <stdio.h>
#include <string.h>

#include "animation.h"
#include "art.h"
#include "automap.h"
#include "color.h"
#include "combat.h"
#include "config.h"
#include "critter.h"
#include "cycle.h"
#include "debug.h"
#include "display_monitor.h"
#include "draw.h"
#include "endgame.h"
#include "game.h"
#include "game_mouse.h"
#include "game_sound.h"
#include "geometry.h"
#include "input.h"
#include "item.h"
#include "kb.h"
#include "memory.h"
#include "mouse.h"
#include "object.h"
#include "platform_compat.h"
#include "proto.h"
#include "proto_instance.h"
#include "proto_types.h"
#include "settings.h"
#include "skill.h"
#include "stat.h"
#include "svga.h"
#include "text_font.h"
#include "tile.h"
#include "window_manager.h"

namespace fallout {

// The width of connectors in the indicator box.
//
// There are male connectors on the left, and female connectors on the right.
// When displaying series of boxes they appear to be plugged into a chain.
#define INDICATOR_BOX_CONNECTOR_WIDTH 3

// The maximum number of indicator boxes the indicator bar can display.
//
// For unknown reason this number is 6, even though there are only 5 different
// indicator types. In addition to that, default screen width 640px cannot hold
// 6 boxes 130px each.
#define INDICATOR_SLOTS_COUNT (6)

#define INTERFACE_ITEM_ACTION_BUTTON_WIDTH 188
#define INTERFACE_ITEM_ACTION_BUTTON_HEIGHT 67

// The values of it's members are offsets to beginning of numbers in
// numbers.frm.
typedef enum InterfaceNumbersColor {
    INTERFACE_NUMBERS_COLOR_WHITE = 0,
    INTERFACE_NUMBERS_COLOR_YELLOW = 120,
    INTERFACE_NUMBERS_COLOR_RED = 240,
} InterfaceNumbersColor;

// Available indicators.
//
// Indicator boxes in the bar are displayed according to the order of this enum.
typedef enum Indicator {
    INDICATOR_ADDICT,
    INDICATOR_SNEAK,
    INDICATOR_LEVEL,
    INDICATOR_POISONED,
    INDICATOR_RADIATED,
    INDICATOR_COUNT,
} Indicator;

// Provides metadata about indicator boxes.
typedef struct IndicatorDescription {
    // An identifier of title in `intrface.msg`.
    int title;

    // A flag denoting this box represents something harmful to the player. It
    // affects color of the title.
    bool isBad;

    // Prerendered indicator data.
    //
    // This value is provided at runtime during indicator box initialization.
    // It includes indicator box background with it's title positioned in the
    // center and is green colored if indicator is good, or red otherwise, as
    // denoted by [isBad] property.
    unsigned char* data;
} IndicatorDescription;

typedef struct InterfaceItemState {
    Object* item;
    unsigned char isDisabled;
    unsigned char isWeapon;
    int primaryHitMode;
    int secondaryHitMode;
    int action;
    int itemFid;
} InterfaceItemState;

static int _intface_redraw_items_callback(Object* _, Object* __);
static int _intface_change_fid_callback(Object* _, Object* __);
static void interfaceBarSwapHandsAnimatePutAwayTakeOutSequence(int previousWeaponAnimationCode, int weaponAnimationCode);
static int intface_init_items();
static int interfaceBarRefreshMainAction();
static int endTurnButtonInit();
static int endTurnButtonFree();
static int endCombatButtonInit();
static int endCombatButtonFree();
static void interfaceUpdateAmmoBar(int x, int ratio);
static int _intface_item_reload();
static void interfaceDrawActionButtonOverlay(unsigned char* data, int width, int height, int pitch, int upX, int upY, int darkenColor, unsigned char* upBuffer, unsigned char* downBuffer);
static void interfaceRenderCounterAnimationStep(unsigned char* src, unsigned char* dest, int delay, Rect* numbersRect, bool refreshMouse);
static void interfaceRenderCounter(int x, int y, int previousValue, int value, int offset, int delay);
static int intface_fatal_error(int rc);
static int indicatorBarInit();
static void interfaceBarFree();
static void indicatorBarReset();
static int indicatorBoxCompareByPosition(const void* a, const void* b);
static void indicatorBarRender(int count);
static bool indicatorBarAdd(int indicator);

static void interfaceBarSize();
static void sidePanelsInit();
static void sidePanelsExit();
static void sidePanelsHide();
static void sidePanelsShow();
static void sidePanelsDraw(const char* path, int win, bool isLeading);

static void minimapHide();
static void minimapShow();

static void interfaceRefreshSlot(int hand, unsigned char* upBuffer, unsigned char* downBuffer);
static void interfaceRefreshActionButtons();
static int interfaceCycleItemActionForHand(int hand);

static void _intface_cycle_item_action_left(int, int);
static void _intface_cycle_item_action_right(int, int);

// 0x518F08
static bool gInterfaceBarInitialized = false;

// 0x518F0C
static bool gInterfaceBarSwapHandsInProgress = false;

// 0x518F10
static bool gInterfaceBarEnabled = false;

// 0x518F14
static bool gInterfaceBarHidden = false;

// 0x518F18
static int gInventoryButton = -1;

// 0x518F24
static int gOptionsButton = -1;

// 0x518F30
static int gSkilldexButton = -1;

// 0x518F40
static int gMapButton = -1;

// 0x518F50
static int gPipboyButton = -1;

// 0x518F5C
static int gCharacterButton = -1;

// 0x518F68
static int gSingleAttackButton = -1;

// 0x518F78
static int gInterfaceCurrentHand = HAND_LEFT;

// 0x518F7C
static Rect gInterfaceBarMainActionRect;

// 0x518F8C
static int gChangeHandsButton = -1;

// 0x518F9C
static bool gInterfaceBarEndButtonsIsVisible = false;

// Combat mode curtains rect.
//
// 0x518FA0
static Rect gInterfaceBarEndButtonsRect;

// 0x518FB0
static int gEndTurnButton = -1;

// 0x518FBC
static int gEndCombatButton = -1;

// 0x518FD4
static Rect gInterfaceBarActionPointsBarRect;

// 0x518FE8
static IndicatorDescription gIndicatorDescriptions[INDICATOR_COUNT] = {
    { 102, true, nullptr }, // ADDICT
    { 100, false, nullptr }, // SNEAK
    { 101, false, nullptr }, // LEVEL
    { 103, true, nullptr }, // POISONED
    { 104, true, nullptr }, // RADIATED
};

// 0x519024
int gInterfaceBarWindow = -1;

// 0x519028
static int gIndicatorBarWindow = -1;

// Last hit points rendered in interface.
//
// Used to animate changes.
//
// 0x51902C
static int gInterfaceLastRenderedHitPoints = 0;

// Last color used to render hit points in interface.
//
// Used to animate changes.
//
// 0x519030
static int gInterfaceLastRenderedHitPointsColor = INTERFACE_NUMBERS_COLOR_RED;

// Last armor class rendered in interface.
//
// Used to animate changes.
//
// 0x519034
static int gInterfaceLastRenderedArmorClass = 0;

// Each slot contains one of indicators or -1 if slot is empty.
//
// 0x5970E0
static int gIndicatorSlots[INDICATOR_SLOTS_COUNT];

// 0x5970F8
static InterfaceItemState gInterfaceItemStates[HAND_COUNT];

// 0x597138
static bool gIndicatorBarIsVisible;

// 0x597154
static unsigned char _itemButtonDown[INTERFACE_ITEM_ACTION_BUTTON_WIDTH * INTERFACE_ITEM_ACTION_BUTTON_HEIGHT];

// 0x59A2B4
static unsigned char _itemButtonUp[INTERFACE_ITEM_ACTION_BUTTON_WIDTH * INTERFACE_ITEM_ACTION_BUTTON_HEIGHT];

// 0x59D3F4
static unsigned char* gInterfaceWindowBuffer;

// A slice of main interface background containing 10 shadowed action point
// dots. In combat mode individual colored dots are rendered on top of this
// background.
//
// This buffer is initialized once and does not change throughout the game.
//
// 0x59D40C
static unsigned char gInterfaceActionPointsBarBackground[90 * 5];

static FrmImage _inventoryButtonNormalFrmImage;
static FrmImage _inventoryButtonPressedFrmImage;
static FrmImage _optionsButtonNormalFrmImage;
static FrmImage _optionsButtonPressedFrmImage;
static FrmImage _skilldexButtonNormalFrmImage;
static FrmImage _skilldexButtonPressedFrmImage;
static FrmImage _skilldexButtonMaskFrmImage;
static FrmImage _mapButtonNormalFrmImage;
static FrmImage _mapButtonPressedFrmImage;
static FrmImage _mapButtonMaskFrmImage;
static FrmImage _pipboyButtonNormalFrmImage;
static FrmImage _pipboyButtonPressedFrmImage;
static FrmImage _characterButtonNormalFrmImage;
static FrmImage _characterButtonPressedFrmImage;
static FrmImage _changeHandsButtonNormalFrmImage;
static FrmImage _changeHandsButtonPressedFrmImage;
static FrmImage _changeHandsButtonMaskFrmImage;
static FrmImage _itemButtonNormalFrmImage;
static FrmImage _itemButtonPressedFrmImage;
static FrmImage _itemButtonDisabledFrmImage;
static FrmImage _endTurnButtonNormalFrmImage;
static FrmImage _endTurnButtonPressedFrmImage;
static FrmImage _endCombatButtonNormalFrmImage;
static FrmImage _endCombatButtonPressedFrmImage;
static FrmImage _numbersFrmImage;
static FrmImage _greenLightFrmImage;
static FrmImage _yellowLightFrmImage;
static FrmImage _redLightFrmImage;
static FrmImage backgroundFrmImage;

int gInterfaceBarContentOffset = 0;
int gInterfaceBarWidth = 800; // will fall back to 640 if screen width is too narrow or asset is absent
bool gInterfaceBarIsWide = false;
int gRightSideShift = 0;
int gAPBarXOffset = 0;   // manual offset for action points bar position

static int gLeftAttackButton = -1;
static int gRightAttackButton = -1;
static Rect gInterfaceBarLeftActionRect;
static Rect gInterfaceBarRightActionRect;

// Dual mode per-hand button buffers
static unsigned char* gLeftButtonUp = nullptr;
static unsigned char* gLeftButtonDown = nullptr;
static unsigned char* gRightButtonUp = nullptr;
static unsigned char* gRightButtonDown = nullptr;

static int gInterfaceSidePanelsLeadingWindow = -1;
static int gInterfaceSidePanelsTrailingWindow = -1;

static void _intface_cycle_item_action_left(int, int) {
    interfaceCycleItemActionForHand(HAND_LEFT);
}

static void _intface_cycle_item_action_right(int, int) {
    interfaceCycleItemActionForHand(HAND_RIGHT);
}

static void _intface_handle_left_click(int, int) {
    if (gInterfaceCurrentHand == HAND_LEFT) {
    } else {
        interfaceBarSwapHands(true);   // swap to left
    }
}

static void _intface_handle_right_click(int, int) {
    if (gInterfaceCurrentHand == HAND_RIGHT) {
    } else {
        interfaceBarSwapHands(true);   // swap to right
    }
}

// intface_init
// 0x45D880
int interfaceInit()
{
    int fid;

    if (gInterfaceBarWindow != -1) {
        return -1;
    }

    interfaceBarSize();

    // Temporary rectangles - will be updated after we know the mode
    gInterfaceBarActionPointsBarRect = { 316 + gInterfaceBarContentOffset, 14, 406 + gInterfaceBarContentOffset, 19 };
    gInterfaceBarEndButtonsRect = { 580 + gInterfaceBarContentOffset, 38, 637 + gInterfaceBarContentOffset, 96 };
    gInterfaceBarMainActionRect = { 267 + gInterfaceBarContentOffset, 26, 455 + gInterfaceBarContentOffset, 93 };

    gInterfaceBarInitialized = true;

    // Offsets for dual mode
    const int BASE_OFFSET = 160;                // base shift for left group (inventory, options, left action button)
    const int RIGHT_EXTRA_SHIFT = 198;          // extra shift for right group (skilldex, map, etc.)
    const int RIGHT_ACTION_BUTTON_GAP = 12;     // gap between left and right action buttons

    // Combined offset for drawing functions (used by HP, AC, AP, ammo, end buttons)
    gRightSideShift = BASE_OFFSET + RIGHT_EXTRA_SHIFT;

    int backgroundFid;

    if (settings.enhancements.strict_vanilla) {
        // ----- VANILLA MODE -----
        backgroundFid = artGetFidWithVariant(OBJ_TYPE_INTERFACE, 16, gInterfaceBarIsWide);
        if (!backgroundFrmImage.lock(backgroundFid)) {
            return intface_fatal_error(-1);
        }
        gRightSideShift = 0;
        gAPBarXOffset = 0;   // no extra offset for AP bar in vanilla
    } else {
        // ----- DUAL MODE: uses custom wide graphic -----
        backgroundFid = buildFid(OBJ_TYPE_INTERFACE, 6285, 0, 0, 0);
        if (!backgroundFrmImage.lock(backgroundFid)) {
            return intface_fatal_error(-1);
        }
        gInterfaceBarWidth = backgroundFrmImage.getWidth();
        gInterfaceBarContentOffset = 0;          // no automatic centering - window is centered by its X position
        gInterfaceBarIsWide = true;

        // Manual offset for AP bar lights
        gAPBarXOffset = -144;

        // Update rectangles for dual mode
        gInterfaceBarActionPointsBarRect = {
            316 + gRightSideShift + gAPBarXOffset,
            14,
            406 + gRightSideShift + gAPBarXOffset,
            19
        };
        gInterfaceBarEndButtonsRect = {
            580 + gRightSideShift,
            38,
            637 + gRightSideShift,
            96
        };
        // Main action button rectangle uses only BASE_OFFSET
        gInterfaceBarMainActionRect = { 267 + BASE_OFFSET, 26, 455 + BASE_OFFSET, 93 };
    }

    int interfaceBarWindowX = (screenGetWidth() - gInterfaceBarWidth) / 2;
    int interfaceBarWindowY = screenGetHeight() - INTERFACE_BAR_HEIGHT;

    gInterfaceBarWindow = windowCreate(interfaceBarWindowX, interfaceBarWindowY, gInterfaceBarWidth, INTERFACE_BAR_HEIGHT, _colorTable[0], WINDOW_HIDDEN | WINDOW_DRAGGABLE_BY_BACKGROUND);
    if (gInterfaceBarWindow == -1) {
        return intface_fatal_error(-1);
    }

    gInterfaceWindowBuffer = windowGetBuffer(gInterfaceBarWindow);
    if (gInterfaceWindowBuffer == nullptr) {
        return intface_fatal_error(-1);
    }

    blitBufferToBuffer(backgroundFrmImage.getData(), gInterfaceBarWidth, INTERFACE_BAR_HEIGHT - 1, gInterfaceBarWidth, gInterfaceWindowBuffer, gInterfaceBarWidth);

    // ---------- BUTTON CREATION ----------
    // Load all necessary graphics (same for both modes)
    fid = buildFid(OBJ_TYPE_INTERFACE, 47, 0, 0, 0);
    if (!_inventoryButtonNormalFrmImage.lock(fid)) return intface_fatal_error(-1);
    fid = buildFid(OBJ_TYPE_INTERFACE, 46, 0, 0, 0);
    if (!_inventoryButtonPressedFrmImage.lock(fid)) return intface_fatal_error(-1);

    fid = buildFid(OBJ_TYPE_INTERFACE, 18, 0, 0, 0);
    if (!_optionsButtonNormalFrmImage.lock(fid)) return intface_fatal_error(-1);
    fid = buildFid(OBJ_TYPE_INTERFACE, 17, 0, 0, 0);
    if (!_optionsButtonPressedFrmImage.lock(fid)) return intface_fatal_error(-1);

    fid = buildFid(OBJ_TYPE_INTERFACE, 6, 0, 0, 0);
    if (!_skilldexButtonNormalFrmImage.lock(fid)) return intface_fatal_error(-1);
    fid = buildFid(OBJ_TYPE_INTERFACE, 7, 0, 0, 0);
    if (!_skilldexButtonPressedFrmImage.lock(fid)) return intface_fatal_error(-1);
    fid = buildFid(OBJ_TYPE_INTERFACE, 6, 0, 0, 0);
    if (!_skilldexButtonMaskFrmImage.lock(fid)) return intface_fatal_error(-1);

    fid = buildFid(OBJ_TYPE_INTERFACE, 13, 0, 0, 0);
    if (!_mapButtonNormalFrmImage.lock(fid)) return intface_fatal_error(-1);
    fid = buildFid(OBJ_TYPE_INTERFACE, 10, 0, 0, 0);
    if (!_mapButtonPressedFrmImage.lock(fid)) return intface_fatal_error(-1);
    fid = buildFid(OBJ_TYPE_INTERFACE, 13, 0, 0, 0);
    if (!_mapButtonMaskFrmImage.lock(fid)) return intface_fatal_error(-1);

    fid = buildFid(OBJ_TYPE_INTERFACE, 59, 0, 0, 0);
    if (!_pipboyButtonNormalFrmImage.lock(fid)) return intface_fatal_error(-1);
    fid = buildFid(OBJ_TYPE_INTERFACE, 58, 0, 0, 0);
    if (!_pipboyButtonPressedFrmImage.lock(fid)) return intface_fatal_error(-1);

    fid = buildFid(OBJ_TYPE_INTERFACE, 57, 0, 0, 0);
    if (!_characterButtonNormalFrmImage.lock(fid)) return intface_fatal_error(-1);
    fid = buildFid(OBJ_TYPE_INTERFACE, 56, 0, 0, 0);
    if (!_characterButtonPressedFrmImage.lock(fid)) return intface_fatal_error(-1);

    fid = buildFid(OBJ_TYPE_INTERFACE, 32, 0, 0, 0);
    if (!_itemButtonNormalFrmImage.lock(fid)) return intface_fatal_error(-1);
    fid = buildFid(OBJ_TYPE_INTERFACE, 31, 0, 0, 0);
    if (!_itemButtonPressedFrmImage.lock(fid)) return intface_fatal_error(-1);
    fid = buildFid(OBJ_TYPE_INTERFACE, 73, 0, 0, 0);
    if (!_itemButtonDisabledFrmImage.lock(fid)) return intface_fatal_error(-1);

    memcpy(_itemButtonUp, _itemButtonNormalFrmImage.getData(), sizeof(_itemButtonUp));
    memcpy(_itemButtonDown, _itemButtonPressedFrmImage.getData(), sizeof(_itemButtonDown));

    if (settings.enhancements.strict_vanilla) {
        // ----- VANILLA: single action button + swap button -----
        const int baseOffset = gInterfaceBarContentOffset;

        gInventoryButton = buttonCreate(gInterfaceBarWindow,
            211 + baseOffset, 40,
            32, 21, -1, -1, -1, KEY_LOWERCASE_I,
            _inventoryButtonNormalFrmImage.getData(),
            _inventoryButtonPressedFrmImage.getData(), nullptr, 0);
        buttonSetCallbacks(gInventoryButton, _gsound_med_butt_press, _gsound_med_butt_release);

        gOptionsButton = buttonCreate(gInterfaceBarWindow,
            210 + baseOffset, 61,
            34, 34, -1, -1, -1, KEY_LOWERCASE_O,
            _optionsButtonNormalFrmImage.getData(),
            _optionsButtonPressedFrmImage.getData(), nullptr, 0);
        buttonSetCallbacks(gOptionsButton, _gsound_med_butt_press, _gsound_med_butt_release);

        gSkilldexButton = buttonCreate(gInterfaceBarWindow,
            523 + baseOffset, 6,
            22, 21, -1, -1, -1, KEY_LOWERCASE_S,
            _skilldexButtonNormalFrmImage.getData(),
            _skilldexButtonPressedFrmImage.getData(),
            nullptr, BUTTON_FLAG_TRANSPARENT);
        buttonSetMask(gSkilldexButton, _skilldexButtonMaskFrmImage.getData());
        buttonSetCallbacks(gSkilldexButton, _gsound_med_butt_press, _gsound_med_butt_release);

        gMapButton = buttonCreate(gInterfaceBarWindow,
            526 + baseOffset, 39,
            41, 19, -1, -1, -1, KEY_TAB,
            _mapButtonNormalFrmImage.getData(),
            _mapButtonPressedFrmImage.getData(),
            nullptr, BUTTON_FLAG_TRANSPARENT);
        buttonSetMask(gMapButton, _mapButtonMaskFrmImage.getData());
        buttonSetCallbacks(gMapButton, _gsound_med_butt_press, _gsound_med_butt_release);

        gPipboyButton = buttonCreate(gInterfaceBarWindow,
            526 + baseOffset, 77,
            41, 19, -1, -1, -1, KEY_LOWERCASE_P,
            _pipboyButtonNormalFrmImage.getData(),
            _pipboyButtonPressedFrmImage.getData(),
            nullptr, 0);
        buttonSetMask(gPipboyButton, _mapButtonMaskFrmImage.getData());
        buttonSetCallbacks(gPipboyButton, _gsound_med_butt_press, _gsound_med_butt_release);

        gCharacterButton = buttonCreate(gInterfaceBarWindow,
            526 + baseOffset, 58,
            41, 19, -1, -1, -1, KEY_LOWERCASE_C,
            _characterButtonNormalFrmImage.getData(),
            _characterButtonPressedFrmImage.getData(),
            nullptr, 0);
        buttonSetMask(gCharacterButton, _mapButtonMaskFrmImage.getData());
        buttonSetCallbacks(gCharacterButton, _gsound_med_butt_press, _gsound_med_butt_release);

        gSingleAttackButton = buttonCreate(gInterfaceBarWindow, 267 + baseOffset, 26,
            INTERFACE_ITEM_ACTION_BUTTON_WIDTH, INTERFACE_ITEM_ACTION_BUTTON_HEIGHT,
            -1, -1, -1, -20, _itemButtonUp, _itemButtonDown, nullptr, BUTTON_FLAG_TRANSPARENT);
        if (gSingleAttackButton == -1) return intface_fatal_error(-1);
        buttonSetRightMouseCallbacks(gSingleAttackButton, -1, KEY_LOWERCASE_N, nullptr, nullptr);
        buttonSetCallbacks(gSingleAttackButton, _gsound_lrg_butt_press, _gsound_lrg_butt_release);

        // Swap hands button
        fid = buildFid(OBJ_TYPE_INTERFACE, 6, 0, 0, 0);
        if (!_changeHandsButtonNormalFrmImage.lock(fid)) return intface_fatal_error(-1);
        fid = buildFid(OBJ_TYPE_INTERFACE, 7, 0, 0, 0);
        if (!_changeHandsButtonPressedFrmImage.lock(fid)) return intface_fatal_error(-1);
        fid = buildFid(OBJ_TYPE_INTERFACE, 6, 0, 0, 0);
        if (!_changeHandsButtonMaskFrmImage.lock(fid)) return intface_fatal_error(-1);
        gChangeHandsButton = buttonCreate(gInterfaceBarWindow, 218 + baseOffset, 6,
            22, 21, -1, -1, -1, KEY_LOWERCASE_B,
            _changeHandsButtonNormalFrmImage.getData(),
            _changeHandsButtonPressedFrmImage.getData(),
            nullptr, BUTTON_FLAG_TRANSPARENT);
        if (gChangeHandsButton == -1) return intface_fatal_error(-1);
        buttonSetMask(gChangeHandsButton, _changeHandsButtonMaskFrmImage.getData());
        buttonSetCallbacks(gChangeHandsButton, _gsound_med_butt_press, _gsound_med_butt_release);

    } else {
        // ----- DUAL MODE: two action buttons, no swap button (will repurpose later) -----

        // Left group (inventory, options) - use BASE_OFFSET
        gInventoryButton = buttonCreate(gInterfaceBarWindow,
            211 + BASE_OFFSET, 40,
            32, 21, -1, -1, -1, KEY_LOWERCASE_I,
            _inventoryButtonNormalFrmImage.getData(),
            _inventoryButtonPressedFrmImage.getData(), nullptr, 0);
        buttonSetCallbacks(gInventoryButton, _gsound_med_butt_press, _gsound_med_butt_release);

        gOptionsButton = buttonCreate(gInterfaceBarWindow,
            210 + BASE_OFFSET, 61,
            34, 34, -1, -1, -1, KEY_LOWERCASE_O,
            _optionsButtonNormalFrmImage.getData(),
            _optionsButtonPressedFrmImage.getData(), nullptr, 0);
        buttonSetCallbacks(gOptionsButton, _gsound_med_butt_press, _gsound_med_butt_release);

        // Action buttons (left and right) - use BASE_OFFSET
        int leftActionX = 267 + BASE_OFFSET;
        int rightActionX = leftActionX + INTERFACE_ITEM_ACTION_BUTTON_WIDTH + RIGHT_ACTION_BUTTON_GAP;

        // Allocate per-hand button buffers
        int bufferSize = INTERFACE_ITEM_ACTION_BUTTON_WIDTH * INTERFACE_ITEM_ACTION_BUTTON_HEIGHT;
        gLeftButtonUp = (unsigned char*)internal_malloc(bufferSize);
        gLeftButtonDown = (unsigned char*)internal_malloc(bufferSize);
        gRightButtonUp = (unsigned char*)internal_malloc(bufferSize);
        gRightButtonDown = (unsigned char*)internal_malloc(bufferSize);
        if (!gLeftButtonUp || !gLeftButtonDown || !gRightButtonUp || !gRightButtonDown) {
            return intface_fatal_error(-1);
        }
        memcpy(gLeftButtonUp, _itemButtonNormalFrmImage.getData(), bufferSize);
        memcpy(gLeftButtonDown, _itemButtonPressedFrmImage.getData(), bufferSize);
        memcpy(gRightButtonUp, _itemButtonNormalFrmImage.getData(), bufferSize);
        memcpy(gRightButtonDown, _itemButtonPressedFrmImage.getData(), bufferSize);

        // Left action button
        gLeftAttackButton = buttonCreate(gInterfaceBarWindow, leftActionX, 26,
            INTERFACE_ITEM_ACTION_BUTTON_WIDTH, INTERFACE_ITEM_ACTION_BUTTON_HEIGHT,
            -1, -1, -1, -20,
            gLeftButtonUp, gLeftButtonDown, nullptr, BUTTON_FLAG_TRANSPARENT);
        buttonSetCallbacks(gLeftAttackButton, _gsound_lrg_butt_press, _intface_handle_left_click);
        buttonSetRightMouseCallbacks(gLeftAttackButton, -1, KEY_LOWERCASE_N, nullptr, _intface_cycle_item_action_left);

        // Right action button
        gRightAttackButton = buttonCreate(gInterfaceBarWindow, rightActionX, 26,
            INTERFACE_ITEM_ACTION_BUTTON_WIDTH, INTERFACE_ITEM_ACTION_BUTTON_HEIGHT,
            -1, -1, -1, -20,
            gRightButtonUp, gRightButtonDown, nullptr, BUTTON_FLAG_TRANSPARENT);
        buttonSetCallbacks(gRightAttackButton, _gsound_lrg_butt_press, _intface_handle_right_click);
        buttonSetRightMouseCallbacks(gRightAttackButton, -1, KEY_LOWERCASE_N, nullptr, _intface_cycle_item_action_right);

        // Refresh rectangles for action buttons
        gInterfaceBarLeftActionRect = { leftActionX, 26, leftActionX + INTERFACE_ITEM_ACTION_BUTTON_WIDTH - 1, 26 + INTERFACE_ITEM_ACTION_BUTTON_HEIGHT - 1 };
        gInterfaceBarRightActionRect = { rightActionX, 26, rightActionX + INTERFACE_ITEM_ACTION_BUTTON_WIDTH - 1, 26 + INTERFACE_ITEM_ACTION_BUTTON_HEIGHT - 1 };

        // Right group (skilldex, map, pipboy, character) - use gRightSideShift
        gSkilldexButton = buttonCreate(gInterfaceBarWindow,
            523 + gRightSideShift, 6,
            22, 21, -1, -1, -1, KEY_LOWERCASE_S,
            _skilldexButtonNormalFrmImage.getData(),
            _skilldexButtonPressedFrmImage.getData(),
            nullptr, BUTTON_FLAG_TRANSPARENT);
        buttonSetMask(gSkilldexButton, _skilldexButtonMaskFrmImage.getData());
        buttonSetCallbacks(gSkilldexButton, _gsound_med_butt_press, _gsound_med_butt_release);

        gMapButton = buttonCreate(gInterfaceBarWindow,
            526 + gRightSideShift, 39,
            41, 19, -1, -1, -1, KEY_TAB,
            _mapButtonNormalFrmImage.getData(),
            _mapButtonPressedFrmImage.getData(),
            nullptr, BUTTON_FLAG_TRANSPARENT);
        buttonSetMask(gMapButton, _mapButtonMaskFrmImage.getData());
        buttonSetCallbacks(gMapButton, _gsound_med_butt_press, _gsound_med_butt_release);

        gPipboyButton = buttonCreate(gInterfaceBarWindow,
            526 + gRightSideShift, 77,
            41, 19, -1, -1, -1, KEY_LOWERCASE_P,
            _pipboyButtonNormalFrmImage.getData(),
            _pipboyButtonPressedFrmImage.getData(),
            nullptr, 0);
        buttonSetMask(gPipboyButton, _mapButtonMaskFrmImage.getData());
        buttonSetCallbacks(gPipboyButton, _gsound_med_butt_press, _gsound_med_butt_release);

        gCharacterButton = buttonCreate(gInterfaceBarWindow,
            526 + gRightSideShift, 58,
            41, 19, -1, -1, -1, KEY_LOWERCASE_C,
            _characterButtonNormalFrmImage.getData(),
            _characterButtonPressedFrmImage.getData(),
            nullptr, 0);
        buttonSetMask(gCharacterButton, _mapButtonMaskFrmImage.getData());
        buttonSetCallbacks(gCharacterButton, _gsound_med_butt_press, _gsound_med_butt_release);
    }

    // ---------- Common initialization (numbers, lights, etc.) ----------
    fid = buildFid(OBJ_TYPE_INTERFACE, 82, 0, 0, 0);
    if (!_numbersFrmImage.lock(fid)) return intface_fatal_error(-1);
    fid = buildFid(OBJ_TYPE_INTERFACE, 83, 0, 0, 0);
    if (!_greenLightFrmImage.lock(fid)) return intface_fatal_error(-1);
    fid = buildFid(OBJ_TYPE_INTERFACE, 84, 0, 0, 0);
    if (!_yellowLightFrmImage.lock(fid)) return intface_fatal_error(-1);
    fid = buildFid(OBJ_TYPE_INTERFACE, 85, 0, 0, 0);
    if (!_redLightFrmImage.lock(fid)) return intface_fatal_error(-1);

    // AP bar background blit
    int apBarOffset = settings.enhancements.strict_vanilla ? gInterfaceBarContentOffset : gRightSideShift + gAPBarXOffset;
    blitBufferToBuffer(gInterfaceWindowBuffer + gInterfaceBarWidth * 14 + 316 + apBarOffset,
        90, 5, gInterfaceBarWidth, gInterfaceActionPointsBarBackground, 90);

    if (indicatorBarInit() == -1) {
        return intface_fatal_error(-1);
    }

    gInterfaceCurrentHand = HAND_LEFT;
    intface_init_items();
    displayMonitorInit();
    sidePanelsInit();

    gInterfaceBarEnabled = true;
    gInterfaceBarInitialized = false;
    gInterfaceBarHidden = true;

    return 0;
}

// 0x45E3D0
void interfaceReset()
{
    interfaceBarEnable();

    // NOTE: Uninline.
    interfaceBarHide();

    indicatorBarRefresh();
    displayMonitorReset();

    // NOTE: Uninline a seemingly inlined routine.
    indicatorBarReset();

    gInterfaceCurrentHand = 0;
}

// 0x45E440
void interfaceFree()
{
    if (gInterfaceBarWindow != -1) {
        // SFALL
        sidePanelsExit();

        displayMonitorExit();

        _redLightFrmImage.unlock();
        _yellowLightFrmImage.unlock();
        _greenLightFrmImage.unlock();

        _numbersFrmImage.unlock();

        // Swap button resources (only in vanilla mode)
        if (settings.enhancements.strict_vanilla) {
            if (gChangeHandsButton != -1) {
                buttonDestroy(gChangeHandsButton);
                gChangeHandsButton = -1;
            }
            _changeHandsButtonMaskFrmImage.unlock();
            _changeHandsButtonPressedFrmImage.unlock();
            _changeHandsButtonNormalFrmImage.unlock();
        }

        // Action button(s) destruction
        if (settings.enhancements.strict_vanilla) {
            if (gSingleAttackButton != -1) {
                buttonDestroy(gSingleAttackButton);
                gSingleAttackButton = -1;
            }
        } else {
            if (gLeftAttackButton != -1) {
                buttonDestroy(gLeftAttackButton);
                gLeftAttackButton = -1;
            }
            if (gRightAttackButton != -1) {
                buttonDestroy(gRightAttackButton);
                gRightAttackButton = -1;
            }
        }

        if (gLeftButtonUp) internal_free(gLeftButtonUp);
        if (gLeftButtonDown) internal_free(gLeftButtonDown);
        if (gRightButtonUp) internal_free(gRightButtonUp);
        if (gRightButtonDown) internal_free(gRightButtonDown);
        gLeftButtonUp = gLeftButtonDown = gRightButtonUp = gRightButtonDown = nullptr;

        _itemButtonDisabledFrmImage.unlock();
        _itemButtonPressedFrmImage.unlock();
        _itemButtonNormalFrmImage.unlock();

        if (gCharacterButton != -1) {
            buttonDestroy(gCharacterButton);
            gCharacterButton = -1;
        }

        _characterButtonPressedFrmImage.unlock();
        _characterButtonNormalFrmImage.unlock();

        if (gPipboyButton != -1) {
            buttonDestroy(gPipboyButton);
            gPipboyButton = -1;
        }

        _pipboyButtonPressedFrmImage.unlock();
        _pipboyButtonNormalFrmImage.unlock();

        if (gMapButton != -1) {
            buttonDestroy(gMapButton);
            gMapButton = -1;
        }

        _mapButtonMaskFrmImage.unlock();
        _mapButtonPressedFrmImage.unlock();
        _mapButtonNormalFrmImage.unlock();

        if (gSkilldexButton != -1) {
            buttonDestroy(gSkilldexButton);
            gSkilldexButton = -1;
        }

        _skilldexButtonMaskFrmImage.unlock();
        _skilldexButtonPressedFrmImage.unlock();
        _skilldexButtonNormalFrmImage.unlock();

        if (gOptionsButton != -1) {
            buttonDestroy(gOptionsButton);
            gOptionsButton = -1;
        }

        _optionsButtonPressedFrmImage.unlock();
        _optionsButtonNormalFrmImage.unlock();

        if (gInventoryButton != -1) {
            buttonDestroy(gInventoryButton);
            gInventoryButton = -1;
        }

        _inventoryButtonPressedFrmImage.unlock();
        _inventoryButtonNormalFrmImage.unlock();
        backgroundFrmImage.unlock();

        if (gInterfaceBarWindow != -1) {
            windowDestroy(gInterfaceBarWindow);
            gInterfaceBarWindow = -1;
        }
    }

    interfaceBarFree();
}

// 0x45E860
int interfaceLoad(File* stream)
{
    if (gInterfaceBarWindow == -1) {
        if (interfaceInit() == -1) {
            return -1;
        }
    }

    bool interfaceBarEnabled;
    if (fileReadBool(stream, &interfaceBarEnabled) == -1)
        return -1;

    bool interfaceBarHidden;
    if (fileReadBool(stream, &interfaceBarHidden) == -1)
        return -1;

    int interfaceCurrentHand;
    if (fileReadInt32(stream, &interfaceCurrentHand) == -1)
        return -1;

    bool interfaceBarEndButtonsIsVisible;
    if (fileReadBool(stream, &interfaceBarEndButtonsIsVisible) == -1)
        return -1;

    if (!gInterfaceBarEnabled) {
        interfaceBarEnable();
    }

    if (interfaceBarHidden) {
        // NOTE: Uninline.
        interfaceBarHide();
    } else {
        interfaceBarShow();
    }

    interfaceRenderHitPoints(false);
    interfaceRenderArmorClass(false);

    gInterfaceCurrentHand = interfaceCurrentHand;

    interfaceUpdateItems(false, INTERFACE_ITEM_ACTION_DEFAULT, INTERFACE_ITEM_ACTION_DEFAULT);

    if (interfaceBarEndButtonsIsVisible != gInterfaceBarEndButtonsIsVisible) {
        if (interfaceBarEndButtonsIsVisible) {
            interfaceBarEndButtonsShow(false);
        } else {
            interfaceBarEndButtonsHide(false);
        }
    }

    if (!interfaceBarEnabled) {
        interfaceBarDisable();
    }

    indicatorBarRefresh();

    windowRefresh(gInterfaceBarWindow);

    return 0;
}

// 0x45E988
int interfaceSave(File* stream)
{
    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    if (fileWriteBool(stream, gInterfaceBarEnabled) == -1)
        return -1;
    if (fileWriteBool(stream, gInterfaceBarHidden) == -1)
        return -1;
    if (fileWriteInt32(stream, gInterfaceCurrentHand) == -1)
        return -1;
    if (fileWriteBool(stream, gInterfaceBarEndButtonsIsVisible) == -1)
        return -1;

    return 0;
}

// NOTE: Inlined.
//
// 0x45E9E0
void interfaceBarHide()
{
    if (gInterfaceBarWindow != -1) {
        if (!gInterfaceBarHidden) {
            windowHide(gInterfaceBarWindow);
            gInterfaceBarHidden = true;
        }
    }

    // SFALL
    sidePanelsHide();
    minimapHide();

    indicatorBarRefresh();
}

// 0x45EA10
void interfaceBarShow()
{
    if (gInterfaceBarWindow != -1) {
        if (gInterfaceBarHidden) {
            interfaceUpdateItems(false, INTERFACE_ITEM_ACTION_DEFAULT, INTERFACE_ITEM_ACTION_DEFAULT);
            interfaceRenderHitPoints(false);
            interfaceRenderArmorClass(false);
            windowShow(gInterfaceBarWindow);
            sidePanelsShow();
            minimapShow();
            gInterfaceBarHidden = false;
        }
    }

    // SFALL
    sidePanelsShow();

    indicatorBarRefresh();
}

// 0x45EA64
void interfaceBarEnable()
{
    if (!gInterfaceBarEnabled) {
        buttonEnable(gInventoryButton);
        buttonEnable(gOptionsButton);
        buttonEnable(gSkilldexButton);
        buttonEnable(gMapButton);
        buttonEnable(gPipboyButton);
        buttonEnable(gCharacterButton);

        if (settings.enhancements.strict_vanilla) {
            if (gInterfaceItemStates[gInterfaceCurrentHand].isDisabled == 0) {
                buttonEnable(gSingleAttackButton);
            }
        } else {
            // Dual mode: enable both action buttons if not disabled
            if (gInterfaceItemStates[HAND_LEFT].isDisabled == 0 && gLeftAttackButton != -1) {
                buttonEnable(gLeftAttackButton);
            }
            if (gInterfaceItemStates[HAND_RIGHT].isDisabled == 0 && gRightAttackButton != -1) {
                buttonEnable(gRightAttackButton);
            }
        }

        buttonEnable(gEndTurnButton);
        buttonEnable(gEndCombatButton);
        displayMonitorEnable();

        gInterfaceBarEnabled = true;
    }
}

void interfaceBarDisable()
{
    if (gInterfaceBarEnabled) {
        displayMonitorDisable();
        buttonDisable(gInventoryButton);
        buttonDisable(gOptionsButton);
        buttonDisable(gSkilldexButton);
        buttonDisable(gMapButton);
        buttonDisable(gPipboyButton);
        buttonDisable(gCharacterButton);

        if (settings.enhancements.strict_vanilla) {
            if (gInterfaceItemStates[gInterfaceCurrentHand].isDisabled == 0) {
                buttonDisable(gSingleAttackButton);
            }
        } else {
            if (gLeftAttackButton != -1) buttonDisable(gLeftAttackButton);
            if (gRightAttackButton != -1) buttonDisable(gRightAttackButton);
        }

        buttonDisable(gEndTurnButton);
        buttonDisable(gEndCombatButton);
        gInterfaceBarEnabled = false;
    }
}

// 0x45EB90
bool interfaceBarEnabled()
{
    return gInterfaceBarEnabled;
}

// 0x45EB98
void interfaceBarRefresh()
{
    if (gInterfaceBarWindow != -1) {
        interfaceUpdateItems(false, INTERFACE_ITEM_ACTION_DEFAULT, INTERFACE_ITEM_ACTION_DEFAULT);
        interfaceRenderHitPoints(false);
        interfaceRenderArmorClass(false);
        indicatorBarRefresh();
        windowRefresh(gInterfaceBarWindow);
    }
    indicatorBarRefresh();
}

// Render hit points.
//
// 0x45EBD8
void interfaceRenderHitPoints(bool animate)
{
    if (gInterfaceBarWindow == -1) {
        return;
    }

    int hp = critterGetHitPoints(gDude);
    int maxHp = critterGetStat(gDude, STAT_MAXIMUM_HIT_POINTS);

    int red = (int)((double)maxHp * 0.25);
    int yellow = (int)((double)maxHp * 0.5);

    int color;
    if (hp < red) {
        color = INTERFACE_NUMBERS_COLOR_RED;
    } else if (hp < yellow) {
        color = INTERFACE_NUMBERS_COLOR_YELLOW;
    } else {
        color = INTERFACE_NUMBERS_COLOR_WHITE;
    }

    int transitionPoints[4];
    int transitionColors[3];
    int count = 1;

    transitionPoints[0] = gInterfaceLastRenderedHitPoints;
    transitionColors[0] = gInterfaceLastRenderedHitPointsColor;

    if (gInterfaceLastRenderedHitPointsColor != color) {
        if (hp >= gInterfaceLastRenderedHitPoints) {
            if (gInterfaceLastRenderedHitPoints < red && hp >= red) {
                transitionPoints[count] = red;
                transitionColors[count] = INTERFACE_NUMBERS_COLOR_YELLOW;
                count += 1;
            }

            if (gInterfaceLastRenderedHitPoints < yellow && hp >= yellow) {
                transitionPoints[count] = yellow;
                transitionColors[count] = INTERFACE_NUMBERS_COLOR_WHITE;
                count += 1;
            }
        } else {
            if (gInterfaceLastRenderedHitPoints >= yellow && hp < yellow) {
                transitionPoints[count] = yellow;
                transitionColors[count] = INTERFACE_NUMBERS_COLOR_YELLOW;
                count += 1;
            }

            if (gInterfaceLastRenderedHitPoints >= red && hp < red) {
                transitionPoints[count] = red;
                transitionColors[count] = INTERFACE_NUMBERS_COLOR_RED;
                count += 1;
            }
        }
    }

    transitionPoints[count] = hp;

    int x = 473 + gInterfaceBarContentOffset + gRightSideShift;

    if (animate) {
        int delay = 250 / (abs(gInterfaceLastRenderedHitPoints - hp) + 1);
        for (int index = 0; index < count; index++) {
            interfaceRenderCounter(x, 40, transitionPoints[index], transitionPoints[index + 1], transitionColors[index], delay);
        }
    } else {
        interfaceRenderCounter(x, 40, gInterfaceLastRenderedHitPoints, hp, color, 0);
    }

    gInterfaceLastRenderedHitPoints = hp;
    gInterfaceLastRenderedHitPointsColor = color;
}

// Render armor class.
//
// 0x45EDA8
void interfaceRenderArmorClass(bool animate)
{
    int armorClass = critterGetStat(gDude, STAT_ARMOR_CLASS);

    int delay = 0;
    if (animate) {
        delay = 250 / (abs(gInterfaceLastRenderedArmorClass - armorClass) + 1);
    }

    int x = 473 + gInterfaceBarContentOffset + gRightSideShift;
    interfaceRenderCounter(x, 75, gInterfaceLastRenderedArmorClass, armorClass, 0, delay);

    gInterfaceLastRenderedArmorClass = armorClass;
}

// 0x45EE0C
void interfaceRenderActionPoints(int actionPointsLeft, int bonusActionPoints)
{
    unsigned char* frmData;

    if (gInterfaceBarWindow == -1) {
        return;
    }

    int x = 316 + gInterfaceBarContentOffset + gRightSideShift;

    // Manual tweak for dual mode - adjust the value as needed
    if (!settings.enhancements.strict_vanilla) {
        x += gAPBarXOffset; // lights are centered in dual mode so need manual adjustment
    }

    blitBufferToBuffer(gInterfaceActionPointsBarBackground, 90, 5, 90,
        gInterfaceWindowBuffer + 14 * gInterfaceBarWidth + x, gInterfaceBarWidth);

    if (actionPointsLeft == -1) {
        frmData = _redLightFrmImage.getData();
        actionPointsLeft = 10;
        bonusActionPoints = 0;
    } else {
        frmData = _greenLightFrmImage.getData();

        if (actionPointsLeft < 0) {
            actionPointsLeft = 0;
        }

        if (actionPointsLeft > 10) {
            actionPointsLeft = 10;
        }

        if (bonusActionPoints >= 0) {
            if (actionPointsLeft + bonusActionPoints > 10) {
                bonusActionPoints = 10 - actionPointsLeft;
            }
        } else {
            bonusActionPoints = 0;
        }
    }

    int index;
    for (index = 0; index < actionPointsLeft; index++) {
        blitBufferToBuffer(frmData, 5, 5, 5,
            gInterfaceWindowBuffer + 14 * gInterfaceBarWidth + x + index * 9, gInterfaceBarWidth);
    }

    for (; index < (actionPointsLeft + bonusActionPoints); index++) {
        blitBufferToBuffer(_yellowLightFrmImage.getData(), 5, 5, 5,
            gInterfaceWindowBuffer + 14 * gInterfaceBarWidth + x + index * 9, gInterfaceBarWidth);
    }

    if (!gInterfaceBarInitialized) {
        windowRefreshRect(gInterfaceBarWindow, &gInterfaceBarActionPointsBarRect);
    }
}

// 0x45EF6C
int interfaceGetCurrentHitMode(int* hitMode, bool* aiming)
{
    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    *aiming = false;

    switch (gInterfaceItemStates[gInterfaceCurrentHand].action) {
    case INTERFACE_ITEM_ACTION_PRIMARY_AIMING:
        *aiming = true;
        // FALLTHROUGH
    case INTERFACE_ITEM_ACTION_PRIMARY:
        *hitMode = gInterfaceItemStates[gInterfaceCurrentHand].primaryHitMode;
        return 0;
    case INTERFACE_ITEM_ACTION_SECONDARY_AIMING:
        *aiming = true;
        // FALLTHROUGH
    case INTERFACE_ITEM_ACTION_SECONDARY:
        *hitMode = gInterfaceItemStates[gInterfaceCurrentHand].secondaryHitMode;
        return 0;
    }

    return -1;
}

// 0x45EFEC
int interfaceUpdateItems(bool animated, int leftItemAction, int rightItemAction)
{
    if (isoIsDisabled()) {
        animated = false;
    }

    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    Object* oldCurrentItem = gInterfaceItemStates[gInterfaceCurrentHand].item;

    InterfaceItemState* leftItemState = &(gInterfaceItemStates[HAND_LEFT]);
    Object* item1 = critterGetItem1(gDude);
    if (item1 == leftItemState->item && leftItemState->item != nullptr) {
        if (leftItemState->item != nullptr) {
            leftItemState->isDisabled = dudeIsWeaponDisabled(item1);
            leftItemState->itemFid = itemGetInventoryFid(item1);
        }
    } else {
        Object* oldItem = leftItemState->item;
        int oldAction = leftItemState->action;

        leftItemState->item = item1;

        if (item1 != nullptr) {
            leftItemState->isDisabled = dudeIsWeaponDisabled(item1);
            leftItemState->primaryHitMode = HIT_MODE_LEFT_WEAPON_PRIMARY;
            leftItemState->secondaryHitMode = HIT_MODE_LEFT_WEAPON_SECONDARY;
            leftItemState->isWeapon = itemGetType(item1) == ITEM_TYPE_WEAPON;

            if (leftItemAction == INTERFACE_ITEM_ACTION_DEFAULT) {
                if (leftItemState->isWeapon != 0) {
                    leftItemState->action = INTERFACE_ITEM_ACTION_PRIMARY;
                } else {
                    leftItemState->action = INTERFACE_ITEM_ACTION_USE;
                }
            } else {
                leftItemState->action = leftItemAction;
            }

            leftItemState->itemFid = itemGetInventoryFid(item1);
        } else {
            leftItemState->isDisabled = 0;
            leftItemState->isWeapon = 1;
            leftItemState->action = INTERFACE_ITEM_ACTION_PRIMARY;
            leftItemState->itemFid = -1;

            // SFALL
            leftItemState->primaryHitMode = unarmedGetPunchHitMode(false);
            leftItemState->secondaryHitMode = unarmedGetPunchHitMode(true);

            // SFALL: Keep selected attack mode.
            // CE: Implementation is different.
            if (oldItem == nullptr) {
                leftItemState->action = oldAction;
            }
        }
    }

    InterfaceItemState* rightItemState = &(gInterfaceItemStates[HAND_RIGHT]);

    Object* item2 = critterGetItem2(gDude);
    if (item2 == rightItemState->item && rightItemState->item != nullptr) {
        if (rightItemState->item != nullptr) {
            rightItemState->isDisabled = dudeIsWeaponDisabled(rightItemState->item);
            rightItemState->itemFid = itemGetInventoryFid(rightItemState->item);
        }
    } else {
        Object* oldItem = rightItemState->item;
        int oldAction = rightItemState->action;

        rightItemState->item = item2;

        if (item2 != nullptr) {
            rightItemState->isDisabled = dudeIsWeaponDisabled(item2);
            rightItemState->primaryHitMode = HIT_MODE_RIGHT_WEAPON_PRIMARY;
            rightItemState->secondaryHitMode = HIT_MODE_RIGHT_WEAPON_SECONDARY;
            rightItemState->isWeapon = itemGetType(item2) == ITEM_TYPE_WEAPON;

            if (rightItemAction == INTERFACE_ITEM_ACTION_DEFAULT) {
                if (rightItemState->isWeapon != 0) {
                    rightItemState->action = INTERFACE_ITEM_ACTION_PRIMARY;
                } else {
                    rightItemState->action = INTERFACE_ITEM_ACTION_USE;
                }
            } else {
                rightItemState->action = rightItemAction;
            }
            rightItemState->itemFid = itemGetInventoryFid(item2);
        } else {
            rightItemState->isDisabled = 0;
            rightItemState->isWeapon = 1;
            rightItemState->action = INTERFACE_ITEM_ACTION_PRIMARY;
            rightItemState->itemFid = -1;

            // SFALL
            rightItemState->primaryHitMode = unarmedGetKickHitMode(false);
            rightItemState->secondaryHitMode = unarmedGetKickHitMode(true);

            // SFALL: Keep selected attack mode.
            // CE: Implementation is different.
            if (oldItem == nullptr) {
                rightItemState->action = oldAction;
            }
        }
    }

    if (animated) {
        Object* newCurrentItem = gInterfaceItemStates[gInterfaceCurrentHand].item;
        if (newCurrentItem != oldCurrentItem) {
            int animationCode = 0;
            if (newCurrentItem != nullptr) {
                if (itemGetType(newCurrentItem) == ITEM_TYPE_WEAPON) {
                    animationCode = weaponGetAnimationCode(newCurrentItem);
                }
            }

            interfaceBarSwapHandsAnimatePutAwayTakeOutSequence((gDude->fid & 0xF000) >> 12, animationCode);

            return 0;
        }
    }

    interfaceRefreshActionButtons();

    return 0;
}

// 0x45F404
int interfaceBarSwapHands(bool animated)
{
    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    gInterfaceCurrentHand = 1 - gInterfaceCurrentHand;

    if (animated) {
        Object* item = gInterfaceItemStates[gInterfaceCurrentHand].item;
        int animationCode = 0;
        if (item != nullptr) {
            if (itemGetType(item) == ITEM_TYPE_WEAPON) {
                animationCode = weaponGetAnimationCode(item);
            }
        }

        interfaceBarSwapHandsAnimatePutAwayTakeOutSequence((gDude->fid & 0xF000) >> 12, animationCode);
    } else {
        interfaceBarRefreshMainAction();
    }

    int mode = gameMouseGetMode();
    if (mode == GAME_MOUSE_MODE_CROSSHAIR || mode == GAME_MOUSE_MODE_USE_CROSSHAIR) {
        gameMouseSetMode(GAME_MOUSE_MODE_MOVE);
    }

    return 0;
}

// 0x45F4B4
int interfaceGetItemActions(int* leftItemAction, int* rightItemAction)
{
    *leftItemAction = gInterfaceItemStates[HAND_LEFT].action;
    *rightItemAction = gInterfaceItemStates[HAND_RIGHT].action;
    return 0;
}

// 0x45F4E0
int interfaceCycleItemAction()
{
    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    InterfaceItemState* itemState = &(gInterfaceItemStates[gInterfaceCurrentHand]);

    int oldAction = itemState->action;
    if (itemState->isWeapon != 0) {
        bool done = false;
        while (!done) {
            itemState->action++;
            switch (itemState->action) {
            case INTERFACE_ITEM_ACTION_PRIMARY:
                done = true;
                break;
            case INTERFACE_ITEM_ACTION_PRIMARY_AIMING:
                if (critterCanAim(gDude, itemState->primaryHitMode)) {
                    done = true;
                }
                break;
            case INTERFACE_ITEM_ACTION_SECONDARY:
                if (itemState->secondaryHitMode != HIT_MODE_PUNCH
                    && itemState->secondaryHitMode != HIT_MODE_KICK
                    && weaponGetAttackTypeForHitMode(itemState->item, itemState->secondaryHitMode) != ATTACK_TYPE_NONE) {
                    done = true;
                }
                break;
            case INTERFACE_ITEM_ACTION_SECONDARY_AIMING:
                if (itemState->secondaryHitMode != HIT_MODE_PUNCH
                    && itemState->secondaryHitMode != HIT_MODE_KICK
                    && weaponGetAttackTypeForHitMode(itemState->item, itemState->secondaryHitMode) != ATTACK_TYPE_NONE
                    && critterCanAim(gDude, itemState->secondaryHitMode)) {
                    done = true;
                }
                break;
            case INTERFACE_ITEM_ACTION_RELOAD:
                if (ammoGetCapacity(itemState->item) != ammoGetQuantity(itemState->item)) {
                    done = true;
                }
                break;
            case INTERFACE_ITEM_ACTION_COUNT:
                itemState->action = INTERFACE_ITEM_ACTION_USE;
                break;
            }
        }
    }

    if (oldAction != itemState->action) {
        interfaceBarRefreshMainAction();
    }

    return 0;
}

// Cycles the attack/reload mode for a specific hand (left or right)
static int interfaceCycleItemActionForHand(int hand)
{
    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    InterfaceItemState* itemState = &(gInterfaceItemStates[hand]);

    int oldAction = itemState->action;
    if (itemState->isWeapon != 0) {
        bool done = false;
        while (!done) {
            itemState->action++;
            switch (itemState->action) {
            case INTERFACE_ITEM_ACTION_PRIMARY:
                done = true;
                break;
            case INTERFACE_ITEM_ACTION_PRIMARY_AIMING:
                if (critterCanAim(gDude, itemState->primaryHitMode)) {
                    done = true;
                }
                break;
            case INTERFACE_ITEM_ACTION_SECONDARY:
                if (itemState->secondaryHitMode != HIT_MODE_PUNCH
                    && itemState->secondaryHitMode != HIT_MODE_KICK
                    && weaponGetAttackTypeForHitMode(itemState->item, itemState->secondaryHitMode) != ATTACK_TYPE_NONE) {
                    done = true;
                }
                break;
            case INTERFACE_ITEM_ACTION_SECONDARY_AIMING:
                if (itemState->secondaryHitMode != HIT_MODE_PUNCH
                    && itemState->secondaryHitMode != HIT_MODE_KICK
                    && weaponGetAttackTypeForHitMode(itemState->item, itemState->secondaryHitMode) != ATTACK_TYPE_NONE
                    && critterCanAim(gDude, itemState->secondaryHitMode)) {
                    done = true;
                }
                break;
            case INTERFACE_ITEM_ACTION_RELOAD:
                if (ammoGetCapacity(itemState->item) != ammoGetQuantity(itemState->item)) {
                    done = true;
                }
                break;
            case INTERFACE_ITEM_ACTION_COUNT:
                itemState->action = INTERFACE_ITEM_ACTION_USE;
                break;
            }
        }
    }

    if (oldAction != itemState->action) {
        // Refresh only this slot, not both
        if (settings.enhancements.strict_vanilla) {
            interfaceRefreshSlot(hand, _itemButtonUp, _itemButtonDown);
        } else {
            if (hand == HAND_LEFT)
                interfaceRefreshSlot(hand, gLeftButtonUp, gLeftButtonDown);
            else
                interfaceRefreshSlot(hand, gRightButtonUp, gRightButtonDown);
        }
    }

    return 0;
}

// 0x45F5EC
void _intface_use_item()
{
    if (gInterfaceBarWindow == -1) return;

    InterfaceItemState* ptr = &(gInterfaceItemStates[gInterfaceCurrentHand]);

    if (ptr->isWeapon != 0) {
        if (ptr->action == INTERFACE_ITEM_ACTION_RELOAD) {
            if (isInCombat()) {
                int hitMode = gInterfaceCurrentHand == HAND_LEFT
                    ? HIT_MODE_LEFT_WEAPON_RELOAD
                    : HIT_MODE_RIGHT_WEAPON_RELOAD;

                int actionPointsRequired = itemGetActionPointCost(gDude, hitMode, false);
                if (actionPointsRequired <= gDude->data.critter.combat.ap) {
                    if (_intface_item_reload() == 0) {
                        if (actionPointsRequired > gDude->data.critter.combat.ap) {
                            gDude->data.critter.combat.ap = 0;
                        } else {
                            gDude->data.critter.combat.ap -= actionPointsRequired;
                        }
                        interfaceRenderActionPoints(gDude->data.critter.combat.ap, _combat_free_move);
                    }
                }
            } else {
                _intface_item_reload();
            }
        } else {
            gameMouseSetCursor(MOUSE_CURSOR_CROSSHAIR);
            gameMouseSetMode(GAME_MOUSE_MODE_CROSSHAIR);
            if (!isInCombat()) {
                _combat(nullptr);
            }
        }
    } else if (_proto_action_can_use_on(ptr->item->pid)) {
        gameMouseSetCursor(MOUSE_CURSOR_USE_CROSSHAIR);
        gameMouseSetMode(GAME_MOUSE_MODE_USE_CROSSHAIR);
    } else if (_obj_action_can_use(ptr->item)) {
        if (isInCombat()) {
            int actionPointsRequired = itemGetActionPointCost(gDude, ptr->secondaryHitMode, false);
            if (actionPointsRequired <= gDude->data.critter.combat.ap) {
                objectUseItem(gDude, ptr->item);
                interfaceUpdateItems(false, INTERFACE_ITEM_ACTION_DEFAULT, INTERFACE_ITEM_ACTION_DEFAULT);
                if (actionPointsRequired > gDude->data.critter.combat.ap) {
                    gDude->data.critter.combat.ap = 0;
                } else {
                    gDude->data.critter.combat.ap -= actionPointsRequired;
                }

                interfaceRenderActionPoints(gDude->data.critter.combat.ap, _combat_free_move);
            }
        } else {
            objectUseItem(gDude, ptr->item);
            interfaceUpdateItems(false, INTERFACE_ITEM_ACTION_DEFAULT, INTERFACE_ITEM_ACTION_DEFAULT);
        }
    }
}

// 0x45F7FC
int interfaceGetCurrentHand()
{
    return gInterfaceCurrentHand;
}

// 0x45F804
int interfaceGetActiveItem(Object** itemPtr)
{
    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    *itemPtr = gInterfaceItemStates[gInterfaceCurrentHand].item;

    return 0;
}

// 0x45F838
int _intface_update_ammo_lights()
{
    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    InterfaceItemState* p = &(gInterfaceItemStates[gInterfaceCurrentHand]);

    int ratio = 0;

    if (p->isWeapon != 0) {
        int maximum = ammoGetCapacity(p->item);
        if (maximum > 0) {
            int current = ammoGetQuantity(p->item);
            ratio = (int)((double)current / (double)maximum * 70.0);
        }
    } else {
        if (itemGetType(p->item) == ITEM_TYPE_MISC) {
            int maximum = miscItemGetMaxCharges(p->item);
            if (maximum > 0) {
                int current = miscItemGetCharges(p->item);
                ratio = (int)((double)current / (double)maximum * 70.0);
            }
        }
    }

    int x = 463 + gInterfaceBarContentOffset + gRightSideShift;
    interfaceUpdateAmmoBar(x, ratio);

    return 0;
}

// 0x45F96C
void interfaceBarEndButtonsShow(bool animated)
{
    if (gInterfaceBarWindow == -1) {
        return;
    }

    if (gInterfaceBarEndButtonsIsVisible) {
        return;
    }

    int fid = buildFid(OBJ_TYPE_INTERFACE, 104, 0, 0, 0);
    CacheEntry* handle;
    Art* art = artLock(fid, &handle);
    if (art == nullptr) {
        return;
    }

    int frameCount = artGetFrameCount(art);
    soundPlayFile("iciboxx1");

    int x = 580 + gInterfaceBarContentOffset + gRightSideShift;
    Rect rect = gInterfaceBarEndButtonsRect; // already shifted in interfaceInit

    if (animated) {
        unsigned int delay = 1000 / artGetFramesPerSecond(art);
        int time = 0;
        int frame = 0;
        while (frame < frameCount) {
            sharedFpsLimiter.mark();

            if (getTicksSince(time) >= delay) {
                unsigned char* src = artGetFrameData(art, frame, 0);
                if (src != nullptr) {
                    blitBufferToBuffer(src, 57, 58, 57,
                        gInterfaceWindowBuffer + gInterfaceBarWidth * 38 + x,
                        gInterfaceBarWidth);
                    windowRefreshRect(gInterfaceBarWindow, &rect);
                }

                time = getTicks();
                frame++;
            }
            gameMouseRefresh();

            renderPresent();
            sharedFpsLimiter.throttle();
        }
    } else {
        unsigned char* src = artGetFrameData(art, frameCount - 1, 0);
        blitBufferToBuffer(src, 57, 58, 57,
            gInterfaceWindowBuffer + gInterfaceBarWidth * 38 + x,
            gInterfaceBarWidth);
        windowRefreshRect(gInterfaceBarWindow, &rect);
    }

    artUnlock(handle);

    gInterfaceBarEndButtonsIsVisible = true;
    endTurnButtonInit();
    endCombatButtonInit();
    interfaceBarEndButtonsRenderRedLights();
}

// 0x45FAC0
void interfaceBarEndButtonsHide(bool animated)
{
    if (gInterfaceBarWindow == -1) {
        return;
    }

    if (!gInterfaceBarEndButtonsIsVisible) {
        return;
    }

    int fid = buildFid(OBJ_TYPE_INTERFACE, 104, 0, 0, 0);
    CacheEntry* handle;
    Art* art = artLock(fid, &handle);
    if (art == nullptr) {
        return;
    }

    endTurnButtonFree();
    endCombatButtonFree();
    soundPlayFile("icibcxx1");

    int x = 580 + gInterfaceBarContentOffset + gRightSideShift;
    Rect rect = gInterfaceBarEndButtonsRect; // already shifted

    if (animated) {
        unsigned int delay = 1000 / artGetFramesPerSecond(art);
        unsigned int time = 0;
        int frame = artGetFrameCount(art);

        while (frame != 0) {
            sharedFpsLimiter.mark();

            if (getTicksSince(time) >= delay) {
                unsigned char* src = artGetFrameData(art, frame - 1, 0);
                unsigned char* dest = gInterfaceWindowBuffer + gInterfaceBarWidth * 38 + x;
                if (src != nullptr) {
                    blitBufferToBuffer(src, 57, 58, 57, dest, gInterfaceBarWidth);
                    windowRefreshRect(gInterfaceBarWindow, &rect);
                }

                time = getTicks();
                frame--;
            }
            gameMouseRefresh();

            renderPresent();
            sharedFpsLimiter.throttle();
        }
    } else {
        unsigned char* dest = gInterfaceWindowBuffer + gInterfaceBarWidth * 38 + x;
        unsigned char* src = artGetFrameData(art, 0, 0);
        blitBufferToBuffer(src, 57, 58, 57, dest, gInterfaceBarWidth);
        windowRefreshRect(gInterfaceBarWindow, &rect);
    }

    artUnlock(handle);
    gInterfaceBarEndButtonsIsVisible = false;
}

// 0x45FC04
void interfaceBarEndButtonsRenderGreenLights()
{
    if (gInterfaceBarEndButtonsIsVisible) {
        buttonEnable(gEndTurnButton);
        buttonEnable(gEndCombatButton);

        FrmImage lightsFrmImage;
        // endltgrn.frm - green lights around end turn/combat window
        int lightsFid = buildFid(OBJ_TYPE_INTERFACE, 109, 0, 0, 0);
        if (!lightsFrmImage.lock(lightsFid)) {
            return;
        }

        soundPlayFile("icombat2");

        int x = 580 + gInterfaceBarContentOffset + gRightSideShift;
        blitBufferToBufferTrans(lightsFrmImage.getData(), 57, 58, 57,
            gInterfaceWindowBuffer + 38 * gInterfaceBarWidth + x,
            gInterfaceBarWidth);
        windowRefreshRect(gInterfaceBarWindow, &gInterfaceBarEndButtonsRect);
    }
}

// 0x45FC98
void interfaceBarEndButtonsRenderRedLights()
{
    if (gInterfaceBarEndButtonsIsVisible) {
        buttonDisable(gEndTurnButton);
        buttonDisable(gEndCombatButton);

        FrmImage lightsFrmImage;
        // endltred.frm - red lights around end turn/combat window
        int lightsFid = buildFid(OBJ_TYPE_INTERFACE, 110, 0, 0, 0);
        if (!lightsFrmImage.lock(lightsFid)) {
            return;
        }

        soundPlayFile("icombat1");

        int x = 580 + gInterfaceBarContentOffset + gRightSideShift;
        blitBufferToBufferTrans(lightsFrmImage.getData(), 57, 58, 57,
            gInterfaceWindowBuffer + 38 * gInterfaceBarWidth + x,
            gInterfaceBarWidth);
        windowRefreshRect(gInterfaceBarWindow, &gInterfaceBarEndButtonsRect);
    }
}

// NOTE: Inlined.
//
// 0x45FD2C
static int intface_init_items()
{
    // FIXME: For unknown reason these values initialized with -1. It's never
    // checked for -1, so I have no explanation for this.
    gInterfaceItemStates[HAND_LEFT].item = (Object*)-1;
    gInterfaceItemStates[HAND_RIGHT].item = (Object*)-1;

    return 0;
}

// 0x45FD88
static int interfaceBarRefreshMainAction()
{
    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    buttonEnable(gSingleAttackButton);

    InterfaceItemState* itemState = &(gInterfaceItemStates[gInterfaceCurrentHand]);
    int actionPoints = -1;
    const int overlayPaddingX = 7;
    const int overlayTopY = 7;
    const int overlayBottomY = INTERFACE_ITEM_ACTION_BUTTON_HEIGHT - overlayPaddingX;

    if (itemState->isDisabled == 0) {
        memcpy(_itemButtonUp, _itemButtonNormalFrmImage.getData(), sizeof(_itemButtonUp));
        memcpy(_itemButtonDown, _itemButtonPressedFrmImage.getData(), sizeof(_itemButtonDown));

        if (itemState->isWeapon == 0) {
            int fid;
            if (_proto_action_can_use_on(itemState->item->pid)) {
                // USE ON
                fid = buildFid(OBJ_TYPE_INTERFACE, 294, 0, 0, 0);
            } else if (_obj_action_can_use(itemState->item)) {
                // USE
                fid = buildFid(OBJ_TYPE_INTERFACE, 292, 0, 0, 0);
            } else {
                fid = -1;
            }

            if (fid != -1) {
                FrmImage useTextFrmImage;
                if (useTextFrmImage.lock(fid)) {
                    int width = useTextFrmImage.getWidth();
                    int height = useTextFrmImage.getHeight();
                    unsigned char* data = useTextFrmImage.getData();
                    interfaceDrawActionButtonOverlay(data, width, height, width, INTERFACE_ITEM_ACTION_BUTTON_WIDTH - overlayPaddingX - width, overlayTopY, 59641, _itemButtonUp, _itemButtonDown);
                }

                actionPoints = itemGetActionPointCost(gDude, itemState->primaryHitMode, false);
            }
        } else {
            int primaryFid = -1;
            int bullseyeFid = -1;
            int hitMode = -1;

            // NOTE: This value is decremented at 0x45FEAC, probably to build
            // jump table.
            switch (itemState->action) {
            case INTERFACE_ITEM_ACTION_PRIMARY_AIMING:
                bullseyeFid = buildFid(OBJ_TYPE_INTERFACE, 288, 0, 0, 0);
                // FALLTHROUGH
            case INTERFACE_ITEM_ACTION_PRIMARY:
                hitMode = itemState->primaryHitMode;
                break;
            case INTERFACE_ITEM_ACTION_SECONDARY_AIMING:
                bullseyeFid = buildFid(OBJ_TYPE_INTERFACE, 288, 0, 0, 0);
                // FALLTHROUGH
            case INTERFACE_ITEM_ACTION_SECONDARY:
                hitMode = itemState->secondaryHitMode;
                break;
            case INTERFACE_ITEM_ACTION_RELOAD:
                actionPoints = itemGetActionPointCost(gDude, gInterfaceCurrentHand == HAND_LEFT ? HIT_MODE_LEFT_WEAPON_RELOAD : HIT_MODE_RIGHT_WEAPON_RELOAD, false);
                primaryFid = buildFid(OBJ_TYPE_INTERFACE, 291, 0, 0, 0);
                break;
            }

            if (bullseyeFid != -1) {
                FrmImage bullseyeFrmImage;
                if (bullseyeFrmImage.lock(bullseyeFid)) {
                    int width = bullseyeFrmImage.getWidth();
                    int height = bullseyeFrmImage.getHeight();
                    unsigned char* data = bullseyeFrmImage.getData();
                    interfaceDrawActionButtonOverlay(data, width, height, width, INTERFACE_ITEM_ACTION_BUTTON_WIDTH - overlayPaddingX - width, overlayBottomY - height, 59641, _itemButtonUp, _itemButtonDown);
                }
            }

            if (hitMode != -1) {
                actionPoints = weaponGetActionPointCost(gDude, hitMode, bullseyeFid != -1);

                int id;
                int anim = critterGetAnimationForHitMode(gDude, hitMode);
                switch (anim) {
                case ANIM_THROW_PUNCH:
                    switch (hitMode) {
                    case HIT_MODE_STRONG_PUNCH:
                        id = 432; // strong punch
                        break;
                    case HIT_MODE_HAMMER_PUNCH:
                        id = 425; // hammer punch
                        break;
                    case HIT_MODE_HAYMAKER:
                        id = 428; // lightning punch
                        break;
                    case HIT_MODE_JAB:
                        id = 421; // chop punch
                        break;
                    case HIT_MODE_PALM_STRIKE:
                        id = 423; // dragon punch
                        break;
                    case HIT_MODE_PIERCING_STRIKE:
                        id = 424; // force punch
                        break;
                    default:
                        id = 42; // punch
                        break;
                    }
                    break;
                case ANIM_KICK_LEG:
                    switch (hitMode) {
                    case HIT_MODE_STRONG_KICK:
                        id = 430; // skick.frm - strong kick text
                        break;
                    case HIT_MODE_SNAP_KICK:
                        id = 431; // snapkick.frm - snap kick text
                        break;
                    case HIT_MODE_POWER_KICK:
                        id = 429; // cm_pwkck.frm - roundhouse kick text
                        break;
                    case HIT_MODE_HIP_KICK:
                        id = 426; // hipk.frm - kip kick text
                        break;
                    case HIT_MODE_HOOK_KICK:
                        id = 427; // cm_hookk.frm - jump kick text
                        break;
                    case HIT_MODE_PIERCING_KICK: // cm_prckk.frm - death blossom kick text
                        id = 422;
                        break;
                    default:
                        id = 41; // kick.frm - kick text
                        break;
                    }
                    break;
                case ANIM_THROW_ANIM:
                    id = 117; // throw
                    break;
                case ANIM_THRUST_ANIM:
                    id = 45; // thrust
                    break;
                case ANIM_SWING_ANIM:
                    id = 44; // swing
                    break;
                case ANIM_FIRE_SINGLE:
                    id = 43; // single
                    break;
                case ANIM_FIRE_BURST:
                case ANIM_FIRE_CONTINUOUS:
                    id = 40; // burst
                    break;
                }

                primaryFid = buildFid(OBJ_TYPE_INTERFACE, id, 0, 0, 0);
            }

            if (primaryFid != -1) {
                FrmImage primaryFrmImage;
                if (primaryFrmImage.lock(primaryFid)) {
                    int width = primaryFrmImage.getWidth();
                    int height = primaryFrmImage.getHeight();
                    unsigned char* data = primaryFrmImage.getData();
                    interfaceDrawActionButtonOverlay(data, width, height, width, INTERFACE_ITEM_ACTION_BUTTON_WIDTH - overlayPaddingX - width, overlayTopY, 59641, _itemButtonUp, _itemButtonDown);
                }
            }
        }
    }

    if (actionPoints >= 0 && actionPoints < 10) {
        // movement point text
        int apFid = buildFid(OBJ_TYPE_INTERFACE, 289, 0, 0, 0);

        FrmImage apFrmImage;
        if (apFrmImage.lock(apFid)) {
            int width = apFrmImage.getWidth();
            int height = apFrmImage.getHeight();
            unsigned char* data = apFrmImage.getData();

            interfaceDrawActionButtonOverlay(data, width, height, width, overlayPaddingX, overlayBottomY - height, 59641, _itemButtonUp, _itemButtonDown);

            int offset = width + overlayPaddingX;

            FrmImage apNumbersFrmImage;
            // movement point numbers - ten numbers 0 to 9, each 10 pixels wide.
            int apNumbersFid = buildFid(OBJ_TYPE_INTERFACE, 290, 0, 0, 0);
            if (apNumbersFrmImage.lock(apNumbersFid)) {
                int width = apNumbersFrmImage.getWidth();
                int height = apNumbersFrmImage.getHeight();
                unsigned char* data = apNumbersFrmImage.getData();

                interfaceDrawActionButtonOverlay(data + actionPoints * 10, 10, height, width, overlayPaddingX + offset, overlayBottomY - height, 59641, _itemButtonUp, _itemButtonDown);
            }
        }
    } else {
        memcpy(_itemButtonUp, _itemButtonDisabledFrmImage.getData(), sizeof(_itemButtonUp));
        memcpy(_itemButtonDown, _itemButtonDisabledFrmImage.getData(), sizeof(_itemButtonDown));
    }

    if (itemState->itemFid != -1) {
        FrmImage itemFrmImage;
        if (itemFrmImage.lock(itemState->itemFid)) {
            int width = itemFrmImage.getWidth();
            int height = itemFrmImage.getHeight();
            unsigned char* data = itemFrmImage.getData();

            int itemIconX = (INTERFACE_ITEM_ACTION_BUTTON_WIDTH - width) / 2;
            int itemIconY = (INTERFACE_ITEM_ACTION_BUTTON_HEIGHT - height) / 2;
            interfaceDrawActionButtonOverlay(data, width, height, width, itemIconX, itemIconY, 63571, _itemButtonUp, _itemButtonDown);
        }
    }

    if (!gInterfaceBarInitialized) {
        _intface_update_ammo_lights();

        windowRefreshRect(gInterfaceBarWindow, &gInterfaceBarMainActionRect);

        if (itemState->isDisabled != 0) {
            buttonDisable(gSingleAttackButton);
        } else {
            buttonEnable(gSingleAttackButton);
        }
    }

    return 0;
}

// helper for interfaceBarRefreshMainAction to draw action button overlays (action text, AP cost, item icon)
static void interfaceDrawActionButtonOverlay(unsigned char* data, int width, int height, int pitch, int upX, int upY, int darkenColor, unsigned char* upBuffer, unsigned char* downBuffer)
{
    blitBufferToBufferTrans(data, width, height, pitch, upBuffer + INTERFACE_ITEM_ACTION_BUTTON_WIDTH * upY + upX, INTERFACE_ITEM_ACTION_BUTTON_WIDTH);

    // everything on the action button is 2px higher and darkened when pressed
    int downY = upY - 2;
    int downHeight = height;
    if (downY < 0) {
        downY = 0;
        downHeight -= 2;
    }

    if (downHeight > 0) {
        _dark_trans_buf_to_buf(data, width, downHeight, pitch, downBuffer, upX + 1, downY, INTERFACE_ITEM_ACTION_BUTTON_WIDTH, darkenColor);
    }
}

// 0x460658
static int _intface_redraw_items_callback(Object* _, Object* __)
{
    interfaceBarRefreshMainAction();
    return 0;
}

// 0x460660
static int _intface_change_fid_callback(Object* _, Object* __) {
    gInterfaceBarSwapHandsInProgress = false;    
    return 0;
}

// 0x46066C
static void interfaceBarSwapHandsAnimatePutAwayTakeOutSequence(int previousWeaponAnimationCode, int weaponAnimationCode)
{
    gInterfaceBarSwapHandsInProgress = true;

    reg_anim_clear(gDude);
    reg_anim_begin(ANIMATION_REQUEST_RESERVED);
    animationRegisterSetLightDistance(gDude, 4, 0);

    if (previousWeaponAnimationCode != 0) {
        const char* sfx = sfxBuildCharName(gDude, ANIM_PUT_AWAY, CHARACTER_SOUND_EFFECT_UNUSED);
        animationRegisterPlaySoundEffect(gDude, sfx, 0);
        animationRegisterAnimate(gDude, ANIM_PUT_AWAY, 0);
    }

    // TODO: Get rid of cast.
    animationRegisterCallbackForced(nullptr, nullptr, (AnimationCallback*)_intface_redraw_items_callback, -1);

    Object* item = gInterfaceItemStates[gInterfaceCurrentHand].item;
    if (item != nullptr && item->lightDistance > 4) {
        animationRegisterSetLightDistance(gDude, item->lightDistance, 0);
    }

    if (weaponAnimationCode != 0) {
        animationRegisterTakeOutWeapon(gDude, weaponAnimationCode, -1);
    } else {
        int fid = buildFid(OBJ_TYPE_CRITTER, artGetIndex(gDude->fid), ANIM_STAND, 0, gDude->rotation + 1);
        animationRegisterSetFid(gDude, fid, -1);
    }

    // TODO: Get rid of cast.
    animationRegisterCallbackForced(nullptr, nullptr, (AnimationCallback*)_intface_change_fid_callback, -1);

    if (reg_anim_end() == -1) {
        return;
    }

    bool interfaceBarWasEnabled = gInterfaceBarEnabled;

    interfaceBarDisable();
    _gmouse_disable(0);

    gameMouseSetCursor(MOUSE_CURSOR_WAIT_WATCH);

    while (gInterfaceBarSwapHandsInProgress) {
        sharedFpsLimiter.mark();

        if (_game_user_wants_to_quit) {
            break;
        }

        inputGetInput();

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    gameMouseSetCursor(MOUSE_CURSOR_NONE);

    _gmouse_enable();

    if (interfaceBarWasEnabled) {
        interfaceBarEnable();
    }
}

// 0x4607E0
static int endTurnButtonInit()
{
    int fid;

    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    if (!gInterfaceBarEndButtonsIsVisible) {
        return -1;
    }

    fid = buildFid(OBJ_TYPE_INTERFACE, 105, 0, 0, 0);
    if (!_endTurnButtonNormalFrmImage.lock(fid)) {
        return -1;
    }

    fid = buildFid(OBJ_TYPE_INTERFACE, 106, 0, 0, 0);
    if (!_endTurnButtonPressedFrmImage.lock(fid)) {
        return -1;
    }

    int x = 590 + gInterfaceBarContentOffset + gRightSideShift;
    gEndTurnButton = buttonCreate(gInterfaceBarWindow, x, 43, 38, 22, -1, -1, -1, 32,
        _endTurnButtonNormalFrmImage.getData(),
        _endTurnButtonPressedFrmImage.getData(), nullptr, 0);
    if (gEndTurnButton == -1) {
        return -1;
    }

    _win_register_button_disable(gEndTurnButton, _endTurnButtonNormalFrmImage.getData(), _endTurnButtonNormalFrmImage.getData(), _endTurnButtonNormalFrmImage.getData());
    buttonSetCallbacks(gEndTurnButton, _gsound_med_butt_press, _gsound_med_butt_release);

    return 0;
}

// 0x4608C4
static int endTurnButtonFree()
{
    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    if (gEndTurnButton != -1) {
        buttonDestroy(gEndTurnButton);
        gEndTurnButton = -1;
    }

    _endTurnButtonNormalFrmImage.unlock();
    _endTurnButtonPressedFrmImage.unlock();

    return 0;
}

// 0x460940
static int endCombatButtonInit()
{
    int fid;

    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    if (!gInterfaceBarEndButtonsIsVisible) {
        return -1;
    }

    fid = buildFid(OBJ_TYPE_INTERFACE, 107, 0, 0, 0);
    if (!_endCombatButtonNormalFrmImage.lock(fid)) {
        return -1;
    }

    fid = buildFid(OBJ_TYPE_INTERFACE, 108, 0, 0, 0);
    if (!_endCombatButtonPressedFrmImage.lock(fid)) {
        return -1;
    }

    int x = 590 + gInterfaceBarContentOffset + gRightSideShift;
    gEndCombatButton = buttonCreate(gInterfaceBarWindow, x, 65, 38, 22, -1, -1, -1, 13,
        _endCombatButtonNormalFrmImage.getData(),
        _endCombatButtonPressedFrmImage.getData(), nullptr, 0);
    if (gEndCombatButton == -1) {
        return -1;
    }

    _win_register_button_disable(gEndCombatButton, _endCombatButtonNormalFrmImage.getData(), _endCombatButtonNormalFrmImage.getData(), _endCombatButtonNormalFrmImage.getData());
    buttonSetCallbacks(gEndCombatButton, _gsound_med_butt_press, _gsound_med_butt_release);

    return 0;
}

// 0x460A24
static int endCombatButtonFree()
{
    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    if (gEndCombatButton != -1) {
        buttonDestroy(gEndCombatButton);
        gEndCombatButton = -1;
    }

    _endCombatButtonNormalFrmImage.unlock();
    _endCombatButtonPressedFrmImage.unlock();

    return 0;
}

// 0x460AA0
static void interfaceUpdateAmmoBar(int x, int ratio)
{
    if ((ratio & 1) != 0) {
        ratio -= 1;
    }

    unsigned char* dest = gInterfaceWindowBuffer + gInterfaceBarWidth * 26 + x;

    for (int index = 70; index > ratio; index--) {
        *dest = 14;
        dest += gInterfaceBarWidth;
    }

    while (ratio > 0) {
        *dest = 196;
        dest += gInterfaceBarWidth;

        *dest = 14;
        dest += gInterfaceBarWidth;

        ratio -= 2;
    }

    if (!gInterfaceBarInitialized) {
        Rect rect;
        rect.left = x;
        rect.top = 26;
        rect.right = x + 1;
        rect.bottom = 26 + 70;
        windowRefreshRect(gInterfaceBarWindow, &rect);
    }
}

// 0x460B20
static int _intface_item_reload()
{
    if (gInterfaceBarWindow == -1) {
        return -1;
    }

    bool wasReloaded = false;
    while (weaponAttemptReload(gDude, gInterfaceItemStates[gInterfaceCurrentHand].item) != -1) {
        wasReloaded = true;
    }

    interfaceCycleItemAction();
    interfaceUpdateItems(false, INTERFACE_ITEM_ACTION_DEFAULT, INTERFACE_ITEM_ACTION_DEFAULT);

    if (!wasReloaded) {
        return -1;
    }

    const char* sfx = sfxBuildWeaponName(WEAPON_SOUND_EFFECT_READY, gInterfaceItemStates[gInterfaceCurrentHand].item, HIT_MODE_RIGHT_WEAPON_PRIMARY, nullptr);
    soundPlayFile(sfx);

    return 0;
}

// internal helper for interfaceRenderCounter
static void interfaceRenderCounterAnimationStep(unsigned char* src, unsigned char* dest, int delay, Rect* numbersRect, bool refreshMouse)
{
    blitBufferToBuffer(src, 9, 17, 360, dest, gInterfaceBarWidth);

    if (refreshMouse) {
        _mouse_info();
        gameMouseRefresh();
    }

    renderPresent();
    inputBlockForTocks(delay);
    windowRefreshRect(gInterfaceBarWindow, numbersRect);
}

// Renders animated counters (AP and HP in the interface bar)
//
// [delay] is an animation delay.
// [previousValue] is only meaningful for animation.
// [offset] = 0 - grey, 120 - yellow, 240 - red.
//
// 0x460BA0
static void interfaceRenderCounter(int x, int y, int previousValue, int value, int offset, int delay)
{
    if (value > 999) {
        value = 999;
    } else if (value < -999) {
        value = -999;
    }

    unsigned char* numbers = _numbersFrmImage.getData() + offset;
    unsigned char* dest = gInterfaceWindowBuffer + gInterfaceBarWidth * y;

    unsigned char* downSrc = numbers + 90;
    unsigned char* upSrc = numbers + 99;
    unsigned char* minusSrc = numbers + 108;
    unsigned char* plusSrc = numbers + 114;

    unsigned char* signDest = dest + x;
    unsigned char* hundredsDest = dest + x + 6;
    unsigned char* tensDest = dest + x + 6 + 9;
    unsigned char* onesDest = dest + x + 6 + 9 * 2;

    int normalizedSign;
    int normalizedValue;
    if (gInterfaceBarInitialized || delay == 0) {
        normalizedSign = value >= 0 ? 1 : -1;
        normalizedValue = abs(value);
    } else {
        normalizedSign = previousValue >= 0 ? 1 : -1;
        normalizedValue = previousValue;
    }

    int ones = normalizedValue % 10;
    int tens = (normalizedValue / 10) % 10;
    int hundreds = normalizedValue / 100;

    blitBufferToBuffer(numbers + 9 * hundreds, 9, 17, 360, hundredsDest, gInterfaceBarWidth);
    blitBufferToBuffer(numbers + 9 * tens, 9, 17, 360, tensDest, gInterfaceBarWidth);
    blitBufferToBuffer(numbers + 9 * ones, 9, 17, 360, onesDest, gInterfaceBarWidth);
    blitBufferToBuffer(normalizedSign >= 0 ? plusSrc : minusSrc, 6, 17, 360, signDest, gInterfaceBarWidth);

    if (!gInterfaceBarInitialized) {
        Rect numbersRect = { x, y, x + 33, y + 17 };
        windowRefreshRect(gInterfaceBarWindow, &numbersRect);
        if (delay != 0) {
            int change = value - previousValue >= 0 ? 1 : -1;
            int previousValueSign = previousValue >= 0 ? 1 : -1;
            int animationStep = change * previousValueSign;
            while (previousValue != value) {
                if ((hundreds | tens | ones) == 0) {
                    animationStep = 1;
                }

                interfaceRenderCounterAnimationStep(upSrc, onesDest, delay, &numbersRect, true);

                ones += animationStep;

                if (ones > 9 || ones < 0) {
                    interfaceRenderCounterAnimationStep(upSrc, tensDest, delay, &numbersRect, true);

                    tens += animationStep;
                    ones -= 10 * animationStep;
                    if (tens == 10 || tens == -1) {
                        interfaceRenderCounterAnimationStep(upSrc, hundredsDest, delay, &numbersRect, true);

                        hundreds += animationStep;
                        tens -= 10 * animationStep;
                        if (hundreds == 10 || hundreds == -1) {
                            hundreds -= 10 * animationStep;
                        }

                        interfaceRenderCounterAnimationStep(downSrc, hundredsDest, delay, &numbersRect, true);
                    }

                    interfaceRenderCounterAnimationStep(downSrc, tensDest, delay, &numbersRect, false);
                }

                interfaceRenderCounterAnimationStep(downSrc, onesDest, delay, &numbersRect, true);

                previousValue += change;

                blitBufferToBuffer(numbers + 9 * hundreds, 9, 17, 360, hundredsDest, gInterfaceBarWidth);
                blitBufferToBuffer(numbers + 9 * tens, 9, 17, 360, tensDest, gInterfaceBarWidth);
                blitBufferToBuffer(numbers + 9 * ones, 9, 17, 360, onesDest, gInterfaceBarWidth);

                blitBufferToBuffer(previousValue >= 0 ? plusSrc : minusSrc, 6, 17, 360, signDest, gInterfaceBarWidth);
                _mouse_info();
                gameMouseRefresh();
                renderPresent();
                inputBlockForTocks(delay);
                windowRefreshRect(gInterfaceBarWindow, &numbersRect);
            }
        }
    }
}

// NOTE: Inlined.
//
// 0x461128
static int intface_fatal_error(int rc)
{
    interfaceFree();

    return rc;
}

// 0x461134
static int indicatorBarInit()
{
    int oldFont = fontGetCurrent();

    if (gIndicatorBarWindow != -1) {
        return 0;
    }

    MessageList messageList;
    MessageListItem messageListItem;
    int rc = 0;
    if (!messageListInit(&messageList)) {
        rc = -1;
    }

    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s%s", asc_5186C8, "intrface.msg");

    if (rc != -1) {
        if (!messageListLoad(&messageList, path)) {
            rc = -1;
        }
    }

    if (rc == -1) {
        debugPrint("\nINTRFACE: Error indicator box messages! **\n");
        return -1;
    }

    FrmImage indicatorBoxFrmImage;
    int indicatorBoxFid = buildFid(OBJ_TYPE_INTERFACE, 126, 0, 0, 0);
    if (!indicatorBoxFrmImage.lock(indicatorBoxFid)) {
        debugPrint("\nINTRFACE: Error initializing indicator box graphics! **\n");
        messageListFree(&messageList);
        return -1;
    }

    for (int index = 0; index < INDICATOR_COUNT; index++) {
        IndicatorDescription* indicatorDescription = &(gIndicatorDescriptions[index]);

        indicatorDescription->data = (unsigned char*)internal_malloc(INDICATOR_BOX_WIDTH * INDICATOR_BOX_HEIGHT);
        if (indicatorDescription->data == nullptr) {
            debugPrint("\nINTRFACE: Error initializing indicator box graphics! **");

            while (--index >= 0) {
                internal_free(gIndicatorDescriptions[index].data);
            }

            messageListFree(&messageList);

            return -1;
        }
    }

    fontSetCurrent(101);

    for (int index = 0; index < INDICATOR_COUNT; index++) {
        IndicatorDescription* indicator = &(gIndicatorDescriptions[index]);

        char text[1024];
        strcpy(text, getmsg(&messageList, &messageListItem, indicator->title));

        int color = indicator->isBad ? _colorTable[31744] : _colorTable[992];

        memcpy(indicator->data, indicatorBoxFrmImage.getData(), INDICATOR_BOX_WIDTH * INDICATOR_BOX_HEIGHT);

        // NOTE: For unknown reason it uses 24 as a height of the box to center
        // the title. One explanation is that these boxes were redesigned, but
        // this value was not changed. On the other hand 24 is
        // [INDICATOR_BOX_HEIGHT] + [INDICATOR_BOX_CONNECTOR_WIDTH]. Maybe just
        // a coincidence. I guess we'll never find out.
        int y = (24 - fontGetLineHeight()) / 2;
        int x = (INDICATOR_BOX_WIDTH - fontGetStringWidth(text)) / 2;
        fontDrawText(indicator->data + INDICATOR_BOX_WIDTH * y + x, text, INDICATOR_BOX_WIDTH, INDICATOR_BOX_WIDTH, color);
    }

    gIndicatorBarIsVisible = true;
    indicatorBarRefresh();

    messageListFree(&messageList);
    fontSetCurrent(oldFont);

    return 0;
}

// 0x461454
static void interfaceBarFree()
{
    if (gIndicatorBarWindow != -1) {
        windowDestroy(gIndicatorBarWindow);
        gIndicatorBarWindow = -1;
    }

    for (int index = 0; index < INDICATOR_COUNT; index++) {
        IndicatorDescription* indicatorBoxDescription = &(gIndicatorDescriptions[index]);
        if (indicatorBoxDescription->data != nullptr) {
            internal_free(indicatorBoxDescription->data);
            indicatorBoxDescription->data = nullptr;
        }
    }
}

// NOTE: This function is not referenced in the original code.
//
// 0x4614A0
static void indicatorBarReset()
{
    if (gIndicatorBarWindow != -1) {
        windowDestroy(gIndicatorBarWindow);
        gIndicatorBarWindow = -1;
    }

    gIndicatorBarIsVisible = true;
}

// Updates indicator bar.
//
// 0x4614CC
int indicatorBarRefresh()
{
    if (gInterfaceBarWindow != -1 && gIndicatorBarIsVisible && !gInterfaceBarHidden) {
        for (int index = 0; index < INDICATOR_SLOTS_COUNT; index++) {
            gIndicatorSlots[index] = -1;
        }

        int count = 0;

        if (dudeHasState(DUDE_STATE_SNEAKING)) {
            if (indicatorBarAdd(INDICATOR_SNEAK)) {
                ++count;
            }
        }

        if (dudeHasState(DUDE_STATE_LEVEL_UP_AVAILABLE)) {
            if (indicatorBarAdd(INDICATOR_LEVEL)) {
                ++count;
            }
        }

        if (dudeHasState(DUDE_STATE_ADDICTED)) {
            if (indicatorBarAdd(INDICATOR_ADDICT)) {
                ++count;
            }
        }

        if (critterGetPoison(gDude) > POISON_INDICATOR_THRESHOLD) {
            if (indicatorBarAdd(INDICATOR_POISONED)) {
                ++count;
            }
        }

        if (critterGetRadiation(gDude) > RADATION_INDICATOR_THRESHOLD) {
            if (indicatorBarAdd(INDICATOR_RADIATED)) {
                ++count;
            }
        }

        if (count > 1) {
            qsort(gIndicatorSlots, count, sizeof(*gIndicatorSlots), indicatorBoxCompareByPosition);
        }

        if (gIndicatorBarWindow != -1) {
            windowDestroy(gIndicatorBarWindow);
            gIndicatorBarWindow = -1;
        }

        if (count != 0) {
            Rect interfaceBarWindowRect;
            windowGetRect(gInterfaceBarWindow, &interfaceBarWindowRect);

            gIndicatorBarWindow = windowCreate(interfaceBarWindowRect.left,
                screenGetHeight() - INTERFACE_BAR_HEIGHT - INDICATOR_BOX_HEIGHT,
                (INDICATOR_BOX_WIDTH - INDICATOR_BOX_CONNECTOR_WIDTH) * count,
                INDICATOR_BOX_HEIGHT,
                _colorTable[0],
                0);
            indicatorBarRender(count);
            windowRefresh(gIndicatorBarWindow);
        }

        return count;
    }

    if (gIndicatorBarWindow != -1) {
        windowDestroy(gIndicatorBarWindow);
        gIndicatorBarWindow = -1;
    }

    return 0;
}

// 0x461624
static int indicatorBoxCompareByPosition(const void* a, const void* b)
{
    int indicatorBox1 = *(int*)a;
    int indicatorBox2 = *(int*)b;

    if (indicatorBox1 == indicatorBox2) {
        return 0;
    } else if (indicatorBox1 < indicatorBox2) {
        return -1;
    } else {
        return 1;
    }
}

// Renders indicator boxes into the indicator bar window.
//
// 0x461648
static void indicatorBarRender(int count)
{
    if (gIndicatorBarWindow == -1) {
        return;
    }

    if (count == 0) {
        return;
    }

    int windowWidth = windowGetWidth(gIndicatorBarWindow);
    unsigned char* windowBuffer = windowGetBuffer(gIndicatorBarWindow);

    // The initial number of connections is 2 - one is first box to the screen
    // boundary, the other is female socket (initially empty). Every displayed
    // box adds one more connection (it is "plugged" into previous box and
    // exposes it's own empty female socket).
    int connections = 2;

    // The width of displayed indicator boxes as if there were no connections.
    int unconnectedIndicatorsWidth = 0;

    // The X offset to display next box.
    int x = 0;

    // The first box is connected to the screen boundary, so we have to clamp
    // male connectors on the left.
    int connectorWidthCompensation = INDICATOR_BOX_CONNECTOR_WIDTH;

    for (int index = 0; index < count; index++) {
        int indicator = gIndicatorSlots[index];
        IndicatorDescription* indicatorDescription = &(gIndicatorDescriptions[indicator]);

        blitBufferToBufferTrans(indicatorDescription->data + connectorWidthCompensation,
            INDICATOR_BOX_WIDTH - connectorWidthCompensation,
            INDICATOR_BOX_HEIGHT,
            INDICATOR_BOX_WIDTH,
            windowBuffer + x, windowWidth);

        connectorWidthCompensation = 0;

        unconnectedIndicatorsWidth += INDICATOR_BOX_WIDTH;
        x = unconnectedIndicatorsWidth - INDICATOR_BOX_CONNECTOR_WIDTH * connections;
        connections++;
    }
}

// Adds indicator to the indicator bar.
//
// Returns `true` if indicator was added, or `false` if there is no available
// space in the indicator bar.
//
// 0x4616F0
static bool indicatorBarAdd(int indicator)
{
    for (int index = 0; index < INDICATOR_SLOTS_COUNT; index++) {
        if (gIndicatorSlots[index] == -1) {
            gIndicatorSlots[index] = indicator;
            return true;
        }
    }

    debugPrint("\nINTRFACE: no free bar box slots!\n");

    return false;
}

// 0x461740
bool indicatorBarShow()
{
    bool oldIsVisible = gIndicatorBarIsVisible;
    gIndicatorBarIsVisible = true;

    indicatorBarRefresh();

    return oldIsVisible;
}

// 0x461760
bool indicatorBarHide()
{
    bool oldIsVisible = gIndicatorBarIsVisible;
    gIndicatorBarIsVisible = false;

    indicatorBarRefresh();

    return oldIsVisible;
}

static void interfaceBarSize()
{
    if (screenGetWidth() > 640 && gInterfaceBarWidth <= screenGetWidth()) {
        gInterfaceBarContentOffset = gInterfaceBarWidth - 640;
        gInterfaceBarIsWide = true;
    } else {
        gInterfaceBarContentOffset = 0;
        gInterfaceBarWidth = 640;
        gInterfaceBarIsWide = false;
    }
}

static void sidePanelsInit()
{
    if (settings.mod_settings.iface_bar_mode) {
        return;
    }

    if (settings.mod_settings.iface_bar_side_art == 0) {
        return;
    }

    if (gInterfaceBarWidth >= screenGetWidth()) {
        return;
    }

    Rect windowRect;
    windowGetRect(gInterfaceBarWindow, &windowRect);

    gInterfaceSidePanelsLeadingWindow = windowCreate(0, windowRect.top, windowRect.left, windowRect.bottom - windowRect.top + 1, 0, WINDOW_HIDDEN | WINDOW_DONT_MOVE_TOP);
    gInterfaceSidePanelsTrailingWindow = windowCreate(windowRect.right + 1, windowRect.top, screenGetWidth() - windowRect.right - 1, windowRect.bottom - windowRect.top + 1, 0, WINDOW_HIDDEN | WINDOW_DONT_MOVE_TOP);

    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "art\\intrface\\HR_IFACELFT%d.frm", settings.mod_settings.iface_bar_side_art);
    sidePanelsDraw(path, gInterfaceSidePanelsLeadingWindow, true);

    snprintf(path, sizeof(path), "art\\intrface\\HR_IFACERHT%d.frm", settings.mod_settings.iface_bar_side_art);
    sidePanelsDraw(path, gInterfaceSidePanelsTrailingWindow, false);
}

static void sidePanelsExit()
{
    if (gInterfaceSidePanelsTrailingWindow != -1) {
        windowDestroy(gInterfaceSidePanelsTrailingWindow);
        gInterfaceSidePanelsTrailingWindow = -1;
    }

    if (gInterfaceSidePanelsLeadingWindow != -1) {
        windowDestroy(gInterfaceSidePanelsLeadingWindow);
        gInterfaceSidePanelsLeadingWindow = -1;
    }
}

static void sidePanelsHide()
{
    if (gInterfaceSidePanelsLeadingWindow != -1) {
        windowHide(gInterfaceSidePanelsLeadingWindow);
    }

    if (gInterfaceSidePanelsTrailingWindow != -1) {
        windowHide(gInterfaceSidePanelsTrailingWindow);
    }
}

static void sidePanelsShow()
{
    if (gInterfaceSidePanelsLeadingWindow != -1) {
        windowShow(gInterfaceSidePanelsLeadingWindow);
    }

    if (gInterfaceSidePanelsTrailingWindow != -1) {
        windowShow(gInterfaceSidePanelsTrailingWindow);
    }
}

// Hide show minimap with interface
static void minimapHide()
{
    if (gAutomapWindow != -1) {
        windowHide(gAutomapWindow);
    }
}

static void minimapShow()
{
    if (gAutomapWindow != -1) {
        windowShow(gAutomapWindow);
    }
}

static void sidePanelsDraw(const char* path, int win, bool isLeading)
{
    Art* image = artLoad(path);
    if (image == nullptr) {
        return;
    }

    unsigned char* imageData = artGetFrameData(image, 0, 0);

    int imageWidth = artGetWidth(image, 0, 0);
    int imageHeight = artGetHeight(image, 0, 0);

    int windowWidth = windowGetWidth(win);
    int windowHeight = windowGetHeight(win);

    int width = std::min(imageWidth, windowWidth);

    if (!settings.mod_settings.iface_bar_sides_ori && isLeading) {
        imageData += imageWidth - width;
    }

    if (settings.mod_settings.iface_bar_sides_ori && !isLeading) {
        imageData += imageWidth - width;
    }

    blitBufferToBufferStretch(imageData,
        width,
        imageHeight,
        imageWidth,
        windowGetBuffer(win),
        windowWidth,
        windowHeight,
        windowWidth);

    internal_free(image);
}

// NOTE: Follows Sfall implementation of `GetCurrentAttackMode`. It slightly
// differs from `interfaceGetCurrentHitMode` (can return one of `reload` hit
// modes, the default is `punch`).
//
// 0x45EF6C
bool interface_get_current_attack_mode(int* hit_mode)
{
    if (gInterfaceBarWindow == -1) {
        return false;
    }

    switch (gInterfaceItemStates[gInterfaceCurrentHand].action) {
    case INTERFACE_ITEM_ACTION_PRIMARY_AIMING:
    case INTERFACE_ITEM_ACTION_PRIMARY:
        *hit_mode = gInterfaceItemStates[gInterfaceCurrentHand].primaryHitMode;
        break;
    case INTERFACE_ITEM_ACTION_SECONDARY_AIMING:
    case INTERFACE_ITEM_ACTION_SECONDARY:
        *hit_mode = gInterfaceItemStates[gInterfaceCurrentHand].secondaryHitMode;
        break;
    case INTERFACE_ITEM_ACTION_RELOAD:
        *hit_mode = gInterfaceCurrentHand == HAND_LEFT
            ? HIT_MODE_LEFT_WEAPON_RELOAD
            : HIT_MODE_RIGHT_WEAPON_RELOAD;
        break;
    default:
        *hit_mode = HIT_MODE_PUNCH;
        break;
    }

    return true;
}

static void interfaceRefreshSlot(int hand, unsigned char* upBuffer, unsigned char* downBuffer)
{
    if (gInterfaceBarWindow == -1) return;

    int bufferSize = INTERFACE_ITEM_ACTION_BUTTON_WIDTH * INTERFACE_ITEM_ACTION_BUTTON_HEIGHT;
    InterfaceItemState* itemState = &(gInterfaceItemStates[hand]);
    int actionPoints = -1;
    const int overlayPaddingX = 7;
    const int overlayTopY = 7;
    const int overlayBottomY = INTERFACE_ITEM_ACTION_BUTTON_HEIGHT - overlayPaddingX;

    // Determine which button ID this slot corresponds to (for enable/disable)
    int buttonId = -1;
    if (settings.enhancements.strict_vanilla) {
        if (hand == gInterfaceCurrentHand) buttonId = gSingleAttackButton;
    } else {
        if (hand == HAND_LEFT) buttonId = gLeftAttackButton;
        else buttonId = gRightAttackButton;
    }

    // Prepare the up/down images (copy from the original button graphics)
    if (itemState->isDisabled != 0) {
        memcpy(upBuffer, _itemButtonDisabledFrmImage.getData(), bufferSize);
        memcpy(downBuffer, _itemButtonDisabledFrmImage.getData(), bufferSize);
    } else {
        memcpy(upBuffer, _itemButtonNormalFrmImage.getData(), bufferSize);
        memcpy(downBuffer, _itemButtonPressedFrmImage.getData(), bufferSize);
    }

    // Draw overlays (action text, bullseye, AP cost)
    if (itemState->isDisabled == 0) {
        if (itemState->isWeapon == 0) {
            // Non?weapon: USE or USE ON
            int fid = -1;
            if (_proto_action_can_use_on(itemState->item->pid)) {
                fid = buildFid(OBJ_TYPE_INTERFACE, 294, 0, 0, 0);
            } else if (_obj_action_can_use(itemState->item)) {
                fid = buildFid(OBJ_TYPE_INTERFACE, 292, 0, 0, 0);
            }
            if (fid != -1) {
                FrmImage textFrm;
                if (textFrm.lock(fid)) {
                    interfaceDrawActionButtonOverlay(
                        textFrm.getData(),
                        textFrm.getWidth(),
                        textFrm.getHeight(),
                        textFrm.getWidth(),
                        INTERFACE_ITEM_ACTION_BUTTON_WIDTH - overlayPaddingX - textFrm.getWidth(),
                        overlayTopY,
                        59641,
                        upBuffer, downBuffer);
                }
            }
            actionPoints = itemGetActionPointCost(gDude, itemState->primaryHitMode, false);
        } else {
            // Weapon: determine which mode is active and draw accordingly
            int bullseyeFid = -1;
            int primaryFid = -1;
            int hitMode = -1;

            switch (itemState->action) {
            case INTERFACE_ITEM_ACTION_PRIMARY_AIMING:
                bullseyeFid = buildFid(OBJ_TYPE_INTERFACE, 288, 0, 0, 0);
                // fallthrough
            case INTERFACE_ITEM_ACTION_PRIMARY:
                hitMode = itemState->primaryHitMode;
                break;
            case INTERFACE_ITEM_ACTION_SECONDARY_AIMING:
                bullseyeFid = buildFid(OBJ_TYPE_INTERFACE, 288, 0, 0, 0);
                // fallthrough
            case INTERFACE_ITEM_ACTION_SECONDARY:
                hitMode = itemState->secondaryHitMode;
                break;
            case INTERFACE_ITEM_ACTION_RELOAD:
                actionPoints = itemGetActionPointCost(
                    gDude,
                    hand == HAND_LEFT ? HIT_MODE_LEFT_WEAPON_RELOAD : HIT_MODE_RIGHT_WEAPON_RELOAD,
                    false);
                primaryFid = buildFid(OBJ_TYPE_INTERFACE, 291, 0, 0, 0);
                break;
            }

            if (bullseyeFid != -1) {
                FrmImage bullseyeFrm;
                if (bullseyeFrm.lock(bullseyeFid)) {
                    interfaceDrawActionButtonOverlay(
                        bullseyeFrm.getData(),
                        bullseyeFrm.getWidth(),
                        bullseyeFrm.getHeight(),
                        bullseyeFrm.getWidth(),
                        INTERFACE_ITEM_ACTION_BUTTON_WIDTH - overlayPaddingX - bullseyeFrm.getWidth(),
                        overlayBottomY - bullseyeFrm.getHeight(),
                        59641,
                        upBuffer, downBuffer);
                }
            }

            if (hitMode != -1) {
                actionPoints = weaponGetActionPointCost(gDude, hitMode, bullseyeFid != -1);

                int anim = critterGetAnimationForHitMode(gDude, hitMode);
                int id = 42; // default punch

                // Map animation to interface text graphic ID
                switch (anim) {
                case ANIM_THROW_PUNCH:
                    switch (hitMode) {
                    case HIT_MODE_STRONG_PUNCH: id = 432; break;
                    case HIT_MODE_HAMMER_PUNCH: id = 425; break;
                    case HIT_MODE_HAYMAKER: id = 428; break;
                    case HIT_MODE_JAB: id = 421; break;
                    case HIT_MODE_PALM_STRIKE: id = 423; break;
                    case HIT_MODE_PIERCING_STRIKE: id = 424; break;
                    default: id = 42; break;
                    }
                    break;
                case ANIM_KICK_LEG:
                    switch (hitMode) {
                    case HIT_MODE_STRONG_KICK: id = 430; break;
                    case HIT_MODE_SNAP_KICK: id = 431; break;
                    case HIT_MODE_POWER_KICK: id = 429; break;
                    case HIT_MODE_HIP_KICK: id = 426; break;
                    case HIT_MODE_HOOK_KICK: id = 427; break;
                    case HIT_MODE_PIERCING_KICK: id = 422; break;
                    default: id = 41; break;
                    }
                    break;
                case ANIM_THROW_ANIM: id = 117; break;
                case ANIM_THRUST_ANIM: id = 45; break;
                case ANIM_SWING_ANIM: id = 44; break;
                case ANIM_FIRE_SINGLE: id = 43; break;
                case ANIM_FIRE_BURST:
                case ANIM_FIRE_CONTINUOUS: id = 40; break;
                }

                primaryFid = buildFid(OBJ_TYPE_INTERFACE, id, 0, 0, 0);
            }

            if (primaryFid != -1) {
                FrmImage primaryFrm;
                if (primaryFrm.lock(primaryFid)) {
                    interfaceDrawActionButtonOverlay(
                        primaryFrm.getData(),
                        primaryFrm.getWidth(),
                        primaryFrm.getHeight(),
                        primaryFrm.getWidth(),
                        INTERFACE_ITEM_ACTION_BUTTON_WIDTH - overlayPaddingX - primaryFrm.getWidth(),
                        overlayTopY,
                        59641,
                        upBuffer, downBuffer);
                }
            }
        }
    }

    // Draw AP cost if applicable
    if (actionPoints >= 0 && actionPoints < 10) {
        int apFid = buildFid(OBJ_TYPE_INTERFACE, 289, 0, 0, 0);
        FrmImage apFrm;
        if (apFrm.lock(apFid)) {
            interfaceDrawActionButtonOverlay(
                apFrm.getData(),
                apFrm.getWidth(),
                apFrm.getHeight(),
                apFrm.getWidth(),
                overlayPaddingX,
                overlayBottomY - apFrm.getHeight(),
                59641,
                upBuffer, downBuffer);

            int offset = apFrm.getWidth() + overlayPaddingX;
            int apNumbersFid = buildFid(OBJ_TYPE_INTERFACE, 290, 0, 0, 0);
            FrmImage apNumFrm;
            if (apNumFrm.lock(apNumbersFid)) {
                interfaceDrawActionButtonOverlay(
                    apNumFrm.getData() + actionPoints * 10,
                    10,
                    apNumFrm.getHeight(),
                    apNumFrm.getWidth(),
                    overlayPaddingX + offset,
                    overlayBottomY - apNumFrm.getHeight(),
                    59641,
                    upBuffer, downBuffer);
            }
        }
    }

    // Draw item icon (weapon or item)
    if (itemState->itemFid != -1) {
        FrmImage itemFrm;
        if (itemFrm.lock(itemState->itemFid)) {
            int iconX = (INTERFACE_ITEM_ACTION_BUTTON_WIDTH - itemFrm.getWidth()) / 2;
            int iconY = (INTERFACE_ITEM_ACTION_BUTTON_HEIGHT - itemFrm.getHeight()) / 2;
            interfaceDrawActionButtonOverlay(
                itemFrm.getData(),
                itemFrm.getWidth(),
                itemFrm.getHeight(),
                itemFrm.getWidth(),
                iconX,
                iconY,
                63571,
                upBuffer, downBuffer);
        }
    }

    // Enable/disable the button based on item state
    if (buttonId != -1) {
        if (itemState->isDisabled != 0)
            buttonDisable(buttonId);
        else
            buttonEnable(buttonId);
    }

    // Refresh the on-screen area of this button
    if (!gInterfaceBarInitialized) {
        Rect rect;
        if (settings.enhancements.strict_vanilla) {
            rect = gInterfaceBarMainActionRect;
        } else {
            if (hand == HAND_LEFT)
                rect = gInterfaceBarLeftActionRect;
            else
                rect = gInterfaceBarRightActionRect;
        }
        _intface_update_ammo_lights();
        windowRefreshRect(gInterfaceBarWindow, &rect);
    }
}

// Refreshes the action button(s) based on current mode
static void interfaceRefreshActionButtons()
{
    if (settings.enhancements.strict_vanilla) {
        interfaceRefreshSlot(gInterfaceCurrentHand, _itemButtonUp, _itemButtonDown);
    } else {
        interfaceRefreshSlot(HAND_LEFT, gLeftButtonUp, gLeftButtonDown);
        interfaceRefreshSlot(HAND_RIGHT, gRightButtonUp, gRightButtonDown);
    }
}

} // namespace fallout
