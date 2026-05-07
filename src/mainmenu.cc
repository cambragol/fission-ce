#include "mainmenu.h"

#include <ctype.h>

#include "art.h"
#include "color.h"
#include "dbox.h"
#include "debug.h"
#include "draw.h"
#include "game.h"
#include "game_sound.h"
#include "input.h"
#include "kb.h"
#include "memory.h"
#include "mouse.h"
#include "offsets.h"
#include "palette.h"
#include "platform_compat.h"
#include "preferences.h"
#include "settings.h"
#include "sfall_config.h"
#include "svga.h"
#include "text_font.h"
#include "version.h"
#include "window_manager.h"
#include "word_wrap.h"

#include "platform/git_version.h"

namespace fallout {

// Constants for mod list window (based on perk dialog)
#define MOD_WINDOW_WIDTH 573
#define MOD_WINDOW_HEIGHT 230
#define MOD_WINDOW_X 0
#define MOD_WINDOW_Y 0

// List area
#define MOD_LIST_X 45
#define MOD_LIST_Y 43
#define MOD_LIST_WIDTH 192
#define MOD_LIST_HEIGHT 110
#define MOD_MAX_MOD_LINES 8

// Detail area
#define MOD_ICON_X 413
#define MOD_ICON_Y 64
#define MOD_TEXT_X 280
#define MOD_NAME_Y 27
#define MOD_DESC_Y 70
#define MOD_AUTHOR_Y 150
#define MOD_DEP_Y 180

#define MOD_DISABLED_COLOR 1313 // Dark Green
#define MOD_DISABLED_SELECTED_COLOR 15845 // Dark Orange

typedef enum MainMenuButton {
    MAIN_MENU_BUTTON_INTRO,
    MAIN_MENU_BUTTON_NEW_GAME,
    MAIN_MENU_BUTTON_LOAD_GAME,
    MAIN_MENU_BUTTON_OPTIONS,
    MAIN_MENU_BUTTON_CREDITS,
    MAIN_MENU_BUTTON_EXIT,
    MAIN_MENU_BUTTON_COUNT,
} MainMenuButton;

static int main_menu_fatal_error();
static void main_menu_play_sound(const char* fileName);

static int showModList();
static int modListDrawList();
static void modListDrawDetails(int selectedIndex);
static void modListRefresh();
static int modListHandleInput(int count);
static void modListSortDisabledToBottom();

// 0x5194F0
static int gMainMenuWindow = -1;

// 0x5194F4
static unsigned char* gMainMenuWindowBuffer = nullptr;

// 0x519504
static bool _in_main_menu = false;

// 0x519508
static bool gMainMenuWindowInitialized = false;

// 0x51950C
static unsigned int gMainMenuScreensaverDelay = 120000;

static MessageList gFissionMessageList;
static MessageListItem gFissionMessageListItem;

// Graphics for mod list screen
enum {
    MOD_GRAPHIC_UP_ARROW_OFF,
    MOD_GRAPHIC_UP_ARROW_ON,
    MOD_GRAPHIC_DOWN_ARROW_OFF,
    MOD_GRAPHIC_DOWN_ARROW_ON,
    MOD_GRAPHIC_LITTLE_RED_BUTTON_UP,
    MOD_GRAPHIC_LILTTLE_RED_BUTTON_DOWN,
    MOD_GRAPHIC_REORDER_BUTTON_OFF,
    MOD_GRAPHIC_REORDER_BUTTON_ON,
    MOD_GRAPHIC_COUNT,
};

static int gModListFrmIds[MOD_GRAPHIC_COUNT] = {
    199, // MOD_GRAPHIC_UP_ARROW_OFF
    200, // MOD_GRAPHIC_UP_ARROW_ON
    181, // MOD_GRAPHIC_DOWN_ARROW_OFF
    182, // MOD_GRAPHIC_DOWN_ARROW_ON
    8, // MOD_GRAPHIC_LITTLE_RED_BUTTON_UP
    9, // MOD_GRAPHIC_LILTTLE_RED_BUTTON_DOWN
    7527,
    4459,
};

static int gModListWindow = -1;
static unsigned char* gModListWindowBuffer = nullptr;
static int gModListTopLine = 0;
static int gModListCurrentLine = 0;
static int gModListPreviousCurrentLine = -2;

static int gModListReorderMode = 0;
static int gModListReorderButton = -1;
static int gModListOrderChanged = 0;

// For temporary mod list before confirmation when sorting
static ModInfo gModListTempMods[MAX_LOADED_MODS];
static int gModListTempCount = 0;

static FrmImage _modListBackgroundFrm;
static FrmImage _modListFrmImages[MOD_GRAPHIC_COUNT];

// 0x519510
static const int gMainMenuButtonKeyBindings[MAIN_MENU_BUTTON_COUNT] = {
    KEY_LOWERCASE_I, // intro
    KEY_LOWERCASE_N, // new game
    KEY_LOWERCASE_L, // load game
    KEY_LOWERCASE_O, // options
    KEY_LOWERCASE_C, // credits
    KEY_LOWERCASE_E, // exit
};

// 0x519528
static const int _return_values[MAIN_MENU_BUTTON_COUNT] = {
    MAIN_MENU_INTRO,
    MAIN_MENU_NEW_GAME,
    MAIN_MENU_LOAD_GAME,
    MAIN_MENU_OPTIONS,
    MAIN_MENU_CREDITS,
    MAIN_MENU_EXIT,
};

// 0x614840
static int gMainMenuButtons[MAIN_MENU_BUTTON_COUNT];

// 0x614858
static bool gMainMenuWindowHidden;

static FrmImage _mainMenuBackgroundFrmImage;
static FrmImage _mainMenuButtonNormalFrmImage;
static FrmImage _mainMenuButtonPressedFrmImage;
static FrmImage _mainMenuFissionLogoFrmImage;
static FrmImage _mainMenuFissionModLogoFrmImage;

bool mainMenuLoadOffsetsFromConfig(MainMenuOffsets* offsets, bool isWidescreen)
{
    return loadOffsetsFromConfig<MainMenuOffsets>(
        offsets,
        isWidescreen,
        "mainmenu",
        gMainMenuOffsets640,
        gMainMenuOffsets800,
        applyConfigToMainMenuOffsets);
}

// move to seperate widescreen.cc file later?
void mainMenuWriteDefaultOffsetsToConfig(bool isWidescreen, const MainMenuOffsets* defaults)
{
    const char* section = isWidescreen ? "mainmenu800" : "mainmenu640";

    // Write all default values to config
    configSetInt(&gGameConfig, section, "copyrightX", defaults->copyrightX);
    configSetInt(&gGameConfig, section, "copyrightY", defaults->copyrightY);
    configSetInt(&gGameConfig, section, "versionX", defaults->versionX);
    configSetInt(&gGameConfig, section, "versionY", defaults->versionY);
    configSetInt(&gGameConfig, section, "hashX", defaults->hashX);
    configSetInt(&gGameConfig, section, "hashY", defaults->hashY);
    configSetInt(&gGameConfig, section, "buildDateX", defaults->buildDateX);
    configSetInt(&gGameConfig, section, "buildDateY", defaults->buildDateY);
    configSetInt(&gGameConfig, section, "buttonBaseX", defaults->buttonBaseX);
    configSetInt(&gGameConfig, section, "buttonBaseY", defaults->buttonBaseY);
    configSetInt(&gGameConfig, section, "buttonTextOffsetX", defaults->buttonTextOffsetX);
    configSetInt(&gGameConfig, section, "buttonTextOffsetY", defaults->buttonTextOffsetY);
    configSetInt(&gGameConfig, section, "width", defaults->width);
    configSetInt(&gGameConfig, section, "height", defaults->height);
}

// 0x481650
int mainMenuWindowInit()
{
    int fid;
    MessageListItem msg;
    int len;
    int btn;

    if (gMainMenuWindowInitialized) {
        return 0;
    }

    if (!messageListInit(&gFissionMessageList)) {
        return -1;
    }

    char fissionPath[COMPAT_MAX_PATH];
    snprintf(fissionPath, sizeof(fissionPath), "%s%s", asc_5186C8, "fission.msg");
    if (!messageListLoad(&gFissionMessageList, fissionPath)) {
        return -1;
    }

    // Set widescreen - must be wider in both axis and set to widescreen
    const bool isWidescreen = gameIsWidescreen();

    // Check if we should write defaults or not
    int writeOffsets = 0;
    if (configGetInt(&gGameConfig, "debug", "write_offsets", &writeOffsets) && writeOffsets) {
        // Write BOTH sets of defaults
        mainMenuWriteDefaultOffsetsToConfig(false, &gMainMenuOffsets640); // 640x480 defaults
        mainMenuWriteDefaultOffsetsToConfig(true, &gMainMenuOffsets800); // 800x600 defaults

        // Disable writing and save
        configSetInt(&gGameConfig, "debug", "write_offsets", 0);
        gameConfigSave();
    }

    // Load offsets
    MainMenuOffsets gOffsets;
    mainMenuLoadOffsetsFromConfig(&gOffsets, isWidescreen);

    // user preference must be restored after overriding
    restoreUserAspectPreference();
    // resize to match SDL texture size for stretching
    resizeContent(isWidescreen ? 800 : 640, isWidescreen ? 500 : 480);

    colorPaletteLoad("color.pal");

    int mainMenuWindowX = (screenGetWidth() - gOffsets.width) / 2;
    int mainMenuWindowY = (screenGetHeight() - gOffsets.height) / 2;
    gMainMenuWindow = windowCreate(mainMenuWindowX,
        mainMenuWindowY,
        gOffsets.width,
        gOffsets.height,
        0,
        WINDOW_HIDDEN | WINDOW_MOVE_ON_TOP);
    if (gMainMenuWindow == -1) {
        // NOTE: Uninline.
        return main_menu_fatal_error();
    }

    gMainMenuWindowBuffer = windowGetBuffer(gMainMenuWindow);

    int backgroundFid = artGetFidWithVariant(OBJ_TYPE_INTERFACE, 140, gameIsWidescreen());
    if (!_mainMenuBackgroundFrmImage.lock(backgroundFid)) {
        // NOTE: Uninline.
        return main_menu_fatal_error();
    }

    blitBufferToBuffer(_mainMenuBackgroundFrmImage.getData(), gOffsets.width, gOffsets.height, gOffsets.width, gMainMenuWindowBuffer, gOffsets.width);
    _mainMenuBackgroundFrmImage.unlock();

    int oldFont = fontGetCurrent();
    fontSetCurrent(100);

    // modConfig: Allow to change font color/flags of copyright/version text
    //        It's the last byte ('3C' by default) that picks the colour used. The first byte supplies additional flags for this option
    //        0x010000 - change the color for version string only
    //        0x020000 - underline text (only for the version string)
    //        0x040000 - monospace font (only for the version string)
    int fontSettings = _colorTable[21091];
    int fontSettingsSFall = settings.mod_settings.main_menu_font_color;
    if (fontSettingsSFall && !(fontSettingsSFall & 0x010000))
        fontSettings = fontSettingsSFall & 0xFF;

    // modConfig: Allow to move copyright text
    int offsetX = settings.mod_settings.main_menu_credits_offset_x;
    int offsetY = settings.mod_settings.main_menu_credits_offset_y;

    // Copyright.
    msg.num = 20;
    if (messageListGetItem(&gMiscMessageList, &msg)) {
        windowDrawText(gMainMenuWindow, msg.text, 0, offsetX + gOffsets.copyrightX, offsetY + gOffsets.copyrightY, fontSettings | 0x06000000);
    }

    // SFALL: Make sure font settings are applied when using 0x010000 flag
    if (fontSettingsSFall)
        fontSettings = fontSettingsSFall;

    // fission.frm
    fid = buildFid(OBJ_TYPE_INTERFACE, 4503, 0, 0, 0);
    if (!_mainMenuFissionLogoFrmImage.lock(fid)) {
        return main_menu_fatal_error();
    }

    // fissmods.frm
    fid = buildFid(OBJ_TYPE_INTERFACE, 4336, 0, 0, 0);
    if (!_mainMenuFissionModLogoFrmImage.lock(fid)) {
        return main_menu_fatal_error();
    }

    // Determine if mod button should be shown
    const bool showModButton = (gLoadedModsCount > 0);

    int totalButtonsWidth = 0;
    int overlap = 5;

    if (showModButton) {
        totalButtonsWidth = _mainMenuFissionLogoFrmImage.getWidth() + _mainMenuFissionModLogoFrmImage.getWidth() - overlap;
    } else {
        totalButtonsWidth = _mainMenuFissionLogoFrmImage.getWidth();
    }

    // Fission logo button - always created
    int btnX = gOffsets.hashX - totalButtonsWidth;
    int btnY = gOffsets.hashY - 2;
    btn = buttonCreate(gMainMenuWindow,
        btnX, btnY,
        _mainMenuFissionLogoFrmImage.getWidth(),
        _mainMenuFissionLogoFrmImage.getHeight(),
        -1, -1, -1, 501,
        _mainMenuFissionLogoFrmImage.getData(),
        _mainMenuFissionLogoFrmImage.getData(),
        nullptr, BUTTON_FLAG_TRANSPARENT);

    // Mod logo button - created only when mods are present
    int modbtn = -1; // unused if not created
    if (showModButton) {
        int modbtnX = btnX + _mainMenuFissionLogoFrmImage.getWidth() - overlap;
        modbtn = buttonCreate(gMainMenuWindow,
            modbtnX, btnY,
            _mainMenuFissionModLogoFrmImage.getWidth(),
            _mainMenuFissionModLogoFrmImage.getHeight(),
            -1, -1, -1, 503,
            _mainMenuFissionModLogoFrmImage.getData(),
            _mainMenuFissionModLogoFrmImage.getData(),
            nullptr, BUTTON_FLAG_TRANSPARENT);
    }

    // Version.
    char version[VERSION_MAX];
    versionGetVersion(version, sizeof(version));
    len = fontGetStringWidth(version);
    windowDrawText(gMainMenuWindow, version, 0, gOffsets.hashX - len - totalButtonsWidth - 3, gOffsets.hashY, fontSettings | 0x06000000);

    // Hash - modified for release/fission logo
    char commitHash[VERSION_MAX] = "POWERED BY: ";
    // strcat(commitHash, _BUILD_HASH);
    len = fontGetStringWidth(commitHash);
    windowDrawText(gMainMenuWindow, commitHash, 0, gOffsets.versionX - len - totalButtonsWidth - 3, gOffsets.versionY, fontSettings | 0x06000000);

    // Build Date - removed for release
    /*char buildDate[VERSION_MAX] = "DATE: ";
    strcat(buildDate, _BUILD_DATE);
    len = fontGetStringWidth(buildDate);
    windowDrawText(gMainMenuWindow, buildDate, 0, gOffsets.buildDateX - len, gOffsets.buildDateY, fontSettings | 0x06000000);*/

    // menuup.frm
    fid = buildFid(OBJ_TYPE_INTERFACE, 299, 0, 0, 0);
    if (!_mainMenuButtonNormalFrmImage.lock(fid)) {
        // NOTE: Uninline.
        return main_menu_fatal_error();
    }

    // menudown.frm
    fid = buildFid(OBJ_TYPE_INTERFACE, 300, 0, 0, 0);
    if (!_mainMenuButtonPressedFrmImage.lock(fid)) {
        // NOTE: Uninline.
        return main_menu_fatal_error();
    }

    for (int index = 0; index < MAIN_MENU_BUTTON_COUNT; index++) {
        gMainMenuButtons[index] = -1;
    }

    // modConfig: Allow to move menu buttons via offsetX and offsetY
    offsetX = settings.mod_settings.main_menu_offset_x;
    offsetY = settings.mod_settings.main_menu_offset_y;

    for (int index = 0; index < MAIN_MENU_BUTTON_COUNT; index++) {
        gMainMenuButtons[index] = buttonCreate(gMainMenuWindow,
            offsetX + gOffsets.buttonBaseX,
            offsetY + gOffsets.buttonBaseY + index * 42 - index,
            26,
            26,
            -1,
            -1,
            1111,
            gMainMenuButtonKeyBindings[index],
            _mainMenuButtonNormalFrmImage.getData(),
            _mainMenuButtonPressedFrmImage.getData(),
            nullptr,
            BUTTON_FLAG_TRANSPARENT);
        if (gMainMenuButtons[index] == -1) {
            // NOTE: Uninline.
            return main_menu_fatal_error();
        }

        buttonSetMask(gMainMenuButtons[index], _mainMenuButtonNormalFrmImage.getData());
    }

    fontSetCurrent(104);

    // modConfig: Allow to change font color of buttons
    fontSettings = _colorTable[21091];
    fontSettingsSFall = settings.mod_settings.main_menu_big_font_color;
    if (fontSettingsSFall) {
        fontSettings = fontSettingsSFall & 0xFF;
    }

    for (int index = 0; index < MAIN_MENU_BUTTON_COUNT; index++) {
        msg.num = 9 + index;
        if (messageListGetItem(&gMiscMessageList, &msg)) {
            len = fontGetStringWidth(msg.text);
            fontDrawText(gMainMenuWindowBuffer + gOffsets.buttonTextOffsetX + offsetX + gOffsets.width * (gOffsets.buttonTextOffsetY + offsetY + 42 * index - index + 20) + 126 - (len / 2), msg.text, gOffsets.width - (126 - (len / 2)) - 1, gOffsets.width, fontSettings);
        }
    }

    fontSetCurrent(oldFont);

    gMainMenuWindowInitialized = true;
    gMainMenuWindowHidden = true;

    return 0;
}

// 0x481968
void mainMenuWindowFree()
{
    if (!gMainMenuWindowInitialized) {
        return;
    }

    for (int index = 0; index < MAIN_MENU_BUTTON_COUNT; index++) {
        // FIXME: Why it tries to free only invalid buttons?
        if (gMainMenuButtons[index] == -1) {
            buttonDestroy(gMainMenuButtons[index]);
        }
    }

    _mainMenuButtonPressedFrmImage.unlock();
    _mainMenuButtonNormalFrmImage.unlock();
    _mainMenuFissionLogoFrmImage.unlock();
    _mainMenuFissionModLogoFrmImage.unlock();

    if (gMainMenuWindow != -1) {
        windowDestroy(gMainMenuWindow);
    }

    gMainMenuWindowInitialized = false;
}

// 0x481A00
void mainMenuWindowHide(bool animate)
{
    if (!gMainMenuWindowInitialized) {
        return;
    }

    if (gMainMenuWindowHidden) {
        return;
    }

    soundContinueAll();

    if (animate) {
        paletteFadeTo(gPaletteBlack);
        soundContinueAll();
    }

    windowHide(gMainMenuWindow);
    touch_set_touchscreen_mode(false);

    gMainMenuWindowHidden = true;
}

// 0x481A48
void mainMenuWindowUnhide(bool animate)
{
    if (!gMainMenuWindowInitialized) {
        return;
    }

    if (!gMainMenuWindowHidden) {
        return;
    }

    windowShow(gMainMenuWindow);
    touch_set_touchscreen_mode(true);

    if (animate) {
        colorPaletteLoad("color.pal");
        paletteFadeTo(_cmap);
    }

    gMainMenuWindowHidden = false;
}

// 0x481AA8
int _main_menu_is_enabled()
{
    return 1;
}

static int showFissionAbout()
{
    // Info dialog (OK)

    const char* title = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 300);
    const char* bodyText = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 301);
    const char* bodyText2 = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 302);
    const char* bodyLines[] = { bodyText, bodyText2 };

    char commitHash[VERSION_MAX] = "FISSION - ";
    strcat(commitHash, _BUILD_VER);
    // len = fontGetStringWidth(commitHash);

    showDialogBox(
        commitHash,
        bodyLines,
        2,
        192, 135,
        _colorTable[32328],
        nullptr,
        _colorTable[32328],
        1 // DIALOG_BOX_OK
    );
    return 1;
}

// 0x481AEC
int mainMenuWindowHandleEvents()
{
    _in_main_menu = true;

    bool oldCursorIsHidden = cursorIsHidden();
    if (oldCursorIsHidden) {
        mouseShowCursor();
    }

    unsigned int tick = getTicks();

    int rc = -1;
    while (rc == -1) {
        sharedFpsLimiter.mark();

        int keyCode = inputGetInput();

        for (int buttonIndex = 0; buttonIndex < MAIN_MENU_BUTTON_COUNT; buttonIndex++) {
            if (keyCode == gMainMenuButtonKeyBindings[buttonIndex] || keyCode == toupper(gMainMenuButtonKeyBindings[buttonIndex])) {
                // NOTE: Uninline.
                main_menu_play_sound("nmselec1");

                rc = _return_values[buttonIndex];

                if (buttonIndex == MAIN_MENU_BUTTON_CREDITS && (gPressedPhysicalKeys[SDL_SCANCODE_RSHIFT] != KEY_STATE_UP || gPressedPhysicalKeys[SDL_SCANCODE_LSHIFT] != KEY_STATE_UP)) {
                    rc = MAIN_MENU_QUOTES;
                }

                break;
            }
        }

        if (rc == -1) {
            if (keyCode == KEY_CTRL_R) {
                rc = MAIN_MENU_SELFRUN;
                continue;
            } else if (keyCode == KEY_PLUS || keyCode == KEY_EQUAL) {
                brightnessIncrease();
            } else if (keyCode == KEY_MINUS || keyCode == KEY_UNDERSCORE) {
                brightnessDecrease();
            } else if (keyCode == KEY_UPPERCASE_D || keyCode == KEY_LOWERCASE_D) {
                rc = MAIN_MENU_SCREENSAVER;
                continue;
            } else if (keyCode == 501 || keyCode == KEY_UPPERCASE_A || keyCode == KEY_LOWERCASE_A) {
                main_menu_play_sound("nmselec0");
                showFissionAbout();
                continue;
            } else if (keyCode == 503 || keyCode == KEY_UPPERCASE_M || keyCode == KEY_LOWERCASE_M) {
                main_menu_play_sound("nmselec0");
                showModList();
                continue;
            } else if (keyCode == 1111) {
                if (!(mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT)) {
                    // NOTE: Uninline.
                    main_menu_play_sound("nmselec0");
                }
                continue;
            }
        }

        if (keyCode == KEY_ESCAPE || _game_user_wants_to_quit == 3) {
            rc = MAIN_MENU_EXIT;

            // NOTE: Uninline.
            main_menu_play_sound("nmselec1");
            break;
        } else if (_game_user_wants_to_quit == 2) {
            _game_user_wants_to_quit = 0;
        } else {
            if (getTicksSince(tick) >= gMainMenuScreensaverDelay) {
                rc = MAIN_MENU_TIMEOUT;
            }
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    if (oldCursorIsHidden) {
        mouseHideCursor();
    }

    _in_main_menu = false;

    return rc;
}

// NOTE: Inlined.
//
// 0x481C88
static int main_menu_fatal_error()
{
    mainMenuWindowFree();

    return -1;
}

// NOTE: Inlined.
//
// 0x481C94
static void main_menu_play_sound(const char* fileName)
{
    soundPlayFile(fileName);
}

// Opens the modlist window
static int showModList()
{
    // Reset state
    gModListTopLine = 0;
    gModListCurrentLine = 0;
    gModListPreviousCurrentLine = -2;
    gModListReorderMode = 0;
    gModListOrderChanged = 0;

    // Load background - adjust with final/custom graphic later
    int backgroundFid = buildFid(OBJ_TYPE_INTERFACE, 5599, 0, 0, 0);
    if (!_modListBackgroundFrm.lock(backgroundFid)) {
        debugPrint("Error loading mod list background\n");
        return -1;
    }

    // Load button graphics
    for (int i = 0; i < MOD_GRAPHIC_COUNT; i++) {
        int fid = buildFid(OBJ_TYPE_INTERFACE, gModListFrmIds[i], 0, 0, 0);
        if (!_modListFrmImages[i].lock(fid)) {
            debugPrint("Error loading button graphic %d\n", i);
            _modListBackgroundFrm.unlock();
            for (int j = 0; j < i; j++)
                _modListFrmImages[j].unlock();
            return -1;
        }
    }

    // Determine window position (always center)
    int windowX = (screenGetWidth() - MOD_WINDOW_WIDTH) / 2;
    int windowY = (screenGetHeight() - MOD_WINDOW_HEIGHT) / 2;
    gModListWindow = windowCreate(windowX, windowY, MOD_WINDOW_WIDTH, MOD_WINDOW_HEIGHT, 256,
        WINDOW_MODAL | WINDOW_MOVE_ON_TOP | WINDOW_TRANSPARENT);
    if (gModListWindow == -1) {
        debugPrint("Error creating mod list window\n");
        _modListBackgroundFrm.unlock();
        for (int i = 0; i < MOD_GRAPHIC_COUNT; i++)
            _modListFrmImages[i].unlock();
        return -1;
    }

    gModListWindowBuffer = windowGetBuffer(gModListWindow);
    memcpy(gModListWindowBuffer, _modListBackgroundFrm.getData(), MOD_WINDOW_WIDTH * MOD_WINDOW_HEIGHT);

    // Create buttons
    // Up arrow
    int btnUp = buttonCreate(gModListWindow,
        25, 46,
        _modListFrmImages[MOD_GRAPHIC_UP_ARROW_ON].getWidth(),
        _modListFrmImages[MOD_GRAPHIC_UP_ARROW_ON].getHeight(),
        -1, 574, 572, 574,
        _modListFrmImages[MOD_GRAPHIC_UP_ARROW_OFF].getData(),
        _modListFrmImages[MOD_GRAPHIC_UP_ARROW_ON].getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (btnUp != -1) {
        buttonSetCallbacks(btnUp, _gsound_red_butt_press, nullptr);
    }

    // Down arrow
    int btnDown = buttonCreate(gModListWindow,
        25, 47 + _modListFrmImages[MOD_GRAPHIC_UP_ARROW_ON].getHeight(),
        _modListFrmImages[MOD_GRAPHIC_DOWN_ARROW_ON].getWidth(),
        _modListFrmImages[MOD_GRAPHIC_DOWN_ARROW_ON].getHeight(),
        -1, 575, 573, 575,
        _modListFrmImages[MOD_GRAPHIC_DOWN_ARROW_OFF].getData(),
        _modListFrmImages[MOD_GRAPHIC_DOWN_ARROW_ON].getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (btnDown != -1) {
        buttonSetCallbacks(btnDown, _gsound_red_butt_press, nullptr);
    }

    // Done button
    int btnDone = buttonCreate(gModListWindow,
        48, 186,
        _modListFrmImages[MOD_GRAPHIC_LITTLE_RED_BUTTON_UP].getWidth(),
        _modListFrmImages[MOD_GRAPHIC_LITTLE_RED_BUTTON_UP].getHeight(),
        -1, -1, -1, 500,
        _modListFrmImages[MOD_GRAPHIC_LITTLE_RED_BUTTON_UP].getData(),
        _modListFrmImages[MOD_GRAPHIC_LILTTLE_RED_BUTTON_DOWN].getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (btnDone != -1) {
        buttonSetCallbacks(btnDone, _gsound_red_butt_press, _gsound_red_butt_release);
    }

    // Cancel button
    int btnCancel = buttonCreate(gModListWindow,
        153, 186,
        _modListFrmImages[MOD_GRAPHIC_LITTLE_RED_BUTTON_UP].getWidth(),
        _modListFrmImages[MOD_GRAPHIC_LITTLE_RED_BUTTON_UP].getHeight(),
        -1, -1, -1, 502,
        _modListFrmImages[MOD_GRAPHIC_LITTLE_RED_BUTTON_UP].getData(),
        _modListFrmImages[MOD_GRAPHIC_LILTTLE_RED_BUTTON_DOWN].getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (btnCancel != -1) {
        buttonSetCallbacks(btnCancel, _gsound_red_butt_press, _gsound_red_butt_release);
    }

    // Reorder mode toggle button (normal button, toggles mode on click)
    int reorderX = 219 - 8 - _modListFrmImages[MOD_GRAPHIC_REORDER_BUTTON_OFF].getWidth();
    int reorderY = 148;
    gModListReorderButton = buttonCreate(gModListWindow,
        reorderX, reorderY,
        _modListFrmImages[MOD_GRAPHIC_REORDER_BUTTON_OFF].getWidth(),
        _modListFrmImages[MOD_GRAPHIC_REORDER_BUTTON_OFF].getHeight(),
        -1, -1, 505, 505,
        _modListFrmImages[MOD_GRAPHIC_REORDER_BUTTON_ON].getData(),
        _modListFrmImages[MOD_GRAPHIC_REORDER_BUTTON_OFF].getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT | BUTTON_FLAG_CHECKABLE | BUTTON_FLAG_CHECK_ON_DOWN);
    if (gModListReorderButton != -1) {
        _win_set_button_rest_state(gModListReorderButton, gModListReorderMode, 0);
    }
    buttonSetCallbacks(gModListReorderButton, _gsound_med_butt_press, _gsound_med_butt_press);

    // Copy current mod list into temporary storage
    gModListTempCount = gLoadedModsCount;
    memcpy(gModListTempMods, gLoadedMods, gLoadedModsCount * sizeof(ModInfo));

    modListSortDisabledToBottom();

    // Clickable list area (invisible buttons)
    buttonCreate(gModListWindow,
        MOD_LIST_X, MOD_LIST_Y, MOD_LIST_WIDTH, MOD_LIST_HEIGHT,
        -1, -1, -1, 501,
        nullptr, nullptr, nullptr,
        BUTTON_FLAG_TRANSPARENT);

    // Set font for titles
    fontSetCurrent(103);

    // Button and window titles
    const char* mods = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 500);
    fontDrawText(gModListWindowBuffer + MOD_WINDOW_WIDTH * 16 + 49,
        mods, MOD_WINDOW_WIDTH, MOD_WINDOW_WIDTH, _colorTable[18979]);
    const char* done = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 501);
    fontDrawText(gModListWindowBuffer + MOD_WINDOW_WIDTH * 186 + 69,
        done, MOD_WINDOW_WIDTH, MOD_WINDOW_WIDTH, _colorTable[18979]);
    const char* cancel = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 502);
    fontDrawText(gModListWindowBuffer + MOD_WINDOW_WIDTH * 186 + 171,
        cancel, MOD_WINDOW_WIDTH, MOD_WINDOW_WIDTH, _colorTable[18979]);

    // Draw the mod list
    int count = modListDrawList();
    if (count > 0) {
        modListDrawDetails(gModListTopLine + gModListCurrentLine);
    }

    windowRefresh(gModListWindow);

    int rc = modListHandleInput(count);

    // Cleanup
    _modListBackgroundFrm.unlock();
    for (int i = 0; i < MOD_GRAPHIC_COUNT; i++)
        _modListFrmImages[i].unlock();
    windowDestroy(gModListWindow);

    return rc;
}

static int modListDrawList()
{
    // Clear the list area
    blitBufferToBuffer(
        _modListBackgroundFrm.getData() + MOD_WINDOW_WIDTH * MOD_LIST_Y + MOD_LIST_X,
        MOD_LIST_WIDTH, MOD_LIST_HEIGHT, MOD_WINDOW_WIDTH,
        gModListWindowBuffer + MOD_WINDOW_WIDTH * MOD_LIST_Y + MOD_LIST_X,
        MOD_WINDOW_WIDTH);

    if (gModListTempCount == 0) {
        fontSetCurrent(101);
        fontDrawText(gModListWindowBuffer + MOD_WINDOW_WIDTH * (MOD_LIST_Y + 10) + (MOD_LIST_X + 10),
            "No mods loaded", MOD_WINDOW_WIDTH, MOD_WINDOW_WIDTH, _colorTable[992]);
        return 0;
    }

    fontSetCurrent(101);
    int lineHeight = fontGetLineHeight() + 2;
    int y = MOD_LIST_Y;
    int endIndex = gModListTopLine + MOD_MAX_MOD_LINES;
    if (endIndex > gModListTempCount) endIndex = gModListTempCount;

    for (int i = gModListTopLine; i < endIndex; i++) {
        int color;
        bool selected = (i == gModListTopLine + gModListCurrentLine);

        if (!gModListTempMods[i].enabled) {
            // Disabled mods: different color if selected
            color = selected ? _colorTable[MOD_DISABLED_SELECTED_COLOR] : _colorTable[MOD_DISABLED_COLOR];
        } else {
            // Enabled mods: normal highlight or normal text
            color = selected ? _colorTable[32747] : _colorTable[992];
        }

        int x = MOD_LIST_X;
        // In reorder mode, indent the currently selected line (overrides color)
        if (gModListReorderMode && selected) {
            x += 10; // indent
            color = _colorTable[32767]; // bright white
        }
        fontDrawText(gModListWindowBuffer + MOD_WINDOW_WIDTH * y + x,
            gModListTempMods[i].display_name, MOD_WINDOW_WIDTH, MOD_WINDOW_WIDTH, color);
        y += lineHeight;
    }

    return gModListTempCount;
}

static void modListDrawDetails(int selectedIndex)
{
    if (selectedIndex < 0 || selectedIndex >= gModListTempCount) return;

    const ModInfo* info = &gModListTempMods[selectedIndex];

    // Clear the detail area (copy background)
    blitBufferToBuffer(
        _modListBackgroundFrm.getData() + 280, 293, MOD_WINDOW_HEIGHT, MOD_WINDOW_WIDTH,
        gModListWindowBuffer + 280, MOD_WINDOW_WIDTH);

    // Draw mod icon if it exists
    FrmImage iconFrm;
    int fid = buildFid(OBJ_TYPE_INTERFACE, info->icon_index, 0, 0, 0);
    if (iconFrm.lock(fid)) {
        blitBufferToBuffer(iconFrm.getData(),
            iconFrm.getWidth(), iconFrm.getHeight(), iconFrm.getWidth(),
            gModListWindowBuffer + MOD_WINDOW_WIDTH * MOD_ICON_Y + MOD_ICON_X,
            MOD_WINDOW_WIDTH);
        iconFrm.unlock();
    }

    // Save current font to restore later
    int oldFont = fontGetCurrent();

    // Mod Name
    fontSetCurrent(102);
    int nameWidth = fontGetStringWidth(info->display_name);
    int nameLineHeight = fontGetLineHeight();
    fontDrawText(gModListWindowBuffer + MOD_WINDOW_WIDTH * MOD_NAME_Y + MOD_TEXT_X,
        info->display_name, MOD_WINDOW_WIDTH, MOD_WINDOW_WIDTH, _colorTable[0]);

    // Mod Author
    fontSetCurrent(101);
    int authorLineHeight = fontGetLineHeight();
    int authorY = MOD_NAME_Y + (nameLineHeight - authorLineHeight) - 4; // vertical center
    int authorX = MOD_TEXT_X + nameWidth + 8; // small gap
    const char* by = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 503);
    int byWidth = fontGetStringWidth(by);
    fontDrawText(gModListWindowBuffer + MOD_WINDOW_WIDTH * authorY + authorX,
        by, MOD_WINDOW_WIDTH, MOD_WINDOW_WIDTH, _colorTable[0]);
    fontDrawText(gModListWindowBuffer + MOD_WINDOW_WIDTH * authorY + authorX + byWidth,
        info->author, MOD_WINDOW_WIDTH, MOD_WINDOW_WIDTH, _colorTable[0]);

    // Draw divider line below name/author
    int lineY = MOD_NAME_Y + nameLineHeight + 4; // gap below name line
    int lineStartX = MOD_TEXT_X;
    int lineEndX = MOD_TEXT_X + 260; // match text area end
    windowDrawLine(gModListWindow, lineStartX, lineY, lineEndX, lineY, _colorTable[0]);
    windowDrawLine(gModListWindow, lineStartX, lineY + 1, lineEndX, lineY + 1, _colorTable[0]);

    // Common settings for wrapped text
    int lineHeight = authorLineHeight + 2; // line spacing
    int maxDescWidth = MOD_ICON_X - MOD_TEXT_X - 8; // width available for text before hitting the icon
    char buffer[512];
    short beginnings[256];
    short beginningsCount;

    // Description (wrapped)
    strncpy(buffer, info->description, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    int descLines = 0;
    if (wordWrap(buffer, maxDescWidth, beginnings, &beginningsCount) == 0) {
        int y = lineY + 8; // start description below divider with some margin
        for (int i = 0; i < beginningsCount - 1; i++) {
            short start = beginnings[i];
            short end = beginnings[i + 1];
            char savedChar = buffer[end];
            buffer[end] = '\0';
            fontDrawText(gModListWindowBuffer + MOD_WINDOW_WIDTH * y + MOD_TEXT_X,
                buffer + start,
                MOD_WINDOW_WIDTH, MOD_WINDOW_WIDTH,
                _colorTable[0]);
            buffer[end] = savedChar;
            y += lineHeight;
            descLines++;
        }
    }

    // Dependencies (wrapped, if any)
    if (info->dependencyCount > 0) {
        // Build dependencies string
        char depString[256] = "Dependencies: ";
        for (int i = 0; i < info->dependencyCount; i++) {
            strcat(depString, info->dependencies[i]);
            if (i < info->dependencyCount - 1) strcat(depString, ", ");
        }

        strncpy(buffer, depString, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        if (wordWrap(buffer, maxDescWidth, beginnings, &beginningsCount) == 0) {
            int yDep = lineY + 8 + descLines * lineHeight + 5; // below description
            for (int i = 0; i < beginningsCount - 1; i++) {
                short start = beginnings[i];
                short end = beginnings[i + 1];
                char savedChar = buffer[end];
                buffer[end] = '\0';
                fontDrawText(gModListWindowBuffer + MOD_WINDOW_WIDTH * yDep + MOD_TEXT_X,
                    buffer + start,
                    MOD_WINDOW_WIDTH, MOD_WINDOW_WIDTH,
                    _colorTable[0]);
                buffer[end] = savedChar;
                yDep += lineHeight;
            }
        }
    }

    // Show "DISABLED" if the mod is disabled
    if (!info->enabled) {
        const char* disabledText = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 504); // DISABLED

        fontSetCurrent(101);
        // Position below description/dependencies - fixed Y coordinate - calculate later or base from bottom
        int yDisabled = 150;
        fontDrawText(gModListWindowBuffer + MOD_WINDOW_WIDTH * yDisabled + MOD_TEXT_X,
            disabledText,
            MOD_WINDOW_WIDTH, MOD_WINDOW_WIDTH,
            _colorTable[32328]); // red? colour for warning
    }

    // Restore original font
    fontSetCurrent(oldFont);
}

static void modListRefresh()
{
    modListDrawList();
    modListDrawDetails(gModListTopLine + gModListCurrentLine);
    windowRefresh(gModListWindow);
}

static void modListMoveUp()
{
    int idx = gModListTopLine + gModListCurrentLine;
    if (idx <= 0) return;
    // Do not move if either mod is disabled
    if (!gModListTempMods[idx].enabled || !gModListTempMods[idx - 1].enabled) return;
    // Swap
    ModInfo temp = gModListTempMods[idx];
    gModListTempMods[idx] = gModListTempMods[idx - 1];
    gModListTempMods[idx - 1] = temp;
    // Update selection
    if (gModListCurrentLine == 0 && gModListTopLine > 0) {
        gModListTopLine--;
    } else {
        gModListCurrentLine--;
    }
    gModListOrderChanged = 1;
    modListRefresh();
}

static void modListMoveDown()
{
    int idx = gModListTopLine + gModListCurrentLine;
    if (idx >= gModListTempCount - 1) return;
    // Do not move if either mod is disabled
    if (!gModListTempMods[idx].enabled || !gModListTempMods[idx + 1].enabled) return;
    // Swap
    ModInfo temp = gModListTempMods[idx];
    gModListTempMods[idx] = gModListTempMods[idx + 1];
    gModListTempMods[idx + 1] = temp;
    // Update selection
    if (gModListCurrentLine == MOD_MAX_MOD_LINES - 1 && gModListTopLine + MOD_MAX_MOD_LINES < gModListTempCount) {
        gModListTopLine++;
    } else if (gModListCurrentLine < MOD_MAX_MOD_LINES - 1) {
        gModListCurrentLine++;
    }
    gModListOrderChanged = 1;
    modListRefresh();
}

static void modListSortDisabledToBottom()
{
    ModInfo sorted[MAX_LOADED_MODS];
    int sortedCount = 0;
    // Copy enabled mods in current order
    for (int i = 0; i < gModListTempCount; i++) {
        if (gModListTempMods[i].enabled) {
            sorted[sortedCount++] = gModListTempMods[i];
        }
    }
    // Copy disabled mods in current order
    for (int i = 0; i < gModListTempCount; i++) {
        if (!gModListTempMods[i].enabled) {
            sorted[sortedCount++] = gModListTempMods[i];
        }
    }
    memcpy(gModListTempMods, sorted, sortedCount * sizeof(ModInfo));
    gModListTempCount = sortedCount;
}

static int modListHandleInput(int count)
{
    fontSetCurrent(101);
    int lineHeight = fontGetLineHeight() + 2;

    int rc = 0;
    while (rc == 0) {
        sharedFpsLimiter.mark();

        int keyCode = inputGetInput();
        int repCounter = 0;

        convertMouseWheelToArrowKey(&keyCode);

        if (keyCode == 500 || keyCode == KEY_RETURN) {
            if (gModListOrderChanged) {
                const char* title = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 510);
                const char* bodyText = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 511);
                const char* bodyLines[] = { bodyText };

                showDialogBox(
                    title,
                    bodyLines,
                    1,
                    192, 135,
                    _colorTable[32328],
                    nullptr,
                    _colorTable[32328],
                    1 // DIALOG_BOX_OK
                );
                memcpy(gLoadedMods, gModListTempMods, gModListTempCount * sizeof(ModInfo));
                gLoadedModsCount = gModListTempCount;
                modConfigWriteOrderFromLoadedMods();   // writes all metadata & enabled flags to mods_order.txt
                gModListOrderChanged = 0;
            }
            if (keyCode == KEY_RETURN) {
                soundPlayFile("ib1p1xx1");
            }
            rc = 1;
        } else if (keyCode == 501) { // Click on list area
            int mouseX, mouseY;
            mouseGetPositionInWindow(gModListWindow, &mouseX, &mouseY);
            int clickedLine = (mouseY - MOD_LIST_Y) / lineHeight;
            if (clickedLine >= 0 && clickedLine < MOD_MAX_MOD_LINES && (gModListTopLine + clickedLine) < count) {
                gModListCurrentLine = clickedLine;
                /*if (gModListCurrentLine == gModListPreviousCurrentLine) {
                    soundPlayFile("ib1p1xx1");
                    rc = 1;
                }*/
                // no click exit for now
                gModListPreviousCurrentLine = gModListCurrentLine;
                modListRefresh();
            }
        } else if (keyCode == 502 || keyCode == KEY_ESCAPE || _game_user_wants_to_quit != 0 || keyCode == KEY_UPPERCASE_M || keyCode == KEY_LOWERCASE_M) {
            rc = 2; // cancel
        } else {
            // Handle arrow button clicks (572 = up, 573 = down)
            // Convert arrow button clicks to arrow keys, unless in reorder mode
            if (keyCode == 572) { // Up arrow button
                if (gModListReorderMode) {
                    modListMoveUp();
                } else {
                    keyCode = KEY_ARROW_UP;
                }
            } else if (keyCode == 573) { // Down arrow button
                if (gModListReorderMode) {
                    modListMoveDown();
                } else {
                    keyCode = KEY_ARROW_DOWN;
                }
            } else if (keyCode == 505) { // Reorder toggle button
                gModListReorderMode = !gModListReorderMode;
                modListRefresh();
                continue; // skip further processing
            } else if (!gModListReorderMode && (keyCode == KEY_LOWERCASE_E || keyCode == KEY_UPPERCASE_E)) {
                int selected = gModListTopLine + gModListCurrentLine;
                if (selected >= 0 && selected < gModListTempCount) {
                    gModListTempMods[selected].enabled = !gModListTempMods[selected].enabled;
                    soundPlayFile("nmselec0");
                    gModListOrderChanged = 1;
                    modListRefresh();
                }
                continue;
            }

            switch (keyCode) {
            case KEY_ARROW_UP:
                if (gModListReorderMode) {
                    modListMoveUp();
                } else {
                    gModListPreviousCurrentLine = -2;
                    if (count > 0) {
                        if (gModListTopLine > 0) {
                            gModListTopLine--;
                        } else if (gModListCurrentLine > 0) {
                            gModListCurrentLine--;
                        }
                        modListRefresh();
                    }
                }
                break;
            case KEY_PAGE_UP:
                gModListPreviousCurrentLine = -2;
                for (int i = 0; i < MOD_MAX_MOD_LINES; i++) {
                    if (gModListTopLine > 0) {
                        gModListTopLine--;
                    } else if (gModListCurrentLine > 0) {
                        gModListCurrentLine--;
                    }
                }
                modListRefresh();
                break;
            case KEY_ARROW_DOWN:
                if (gModListReorderMode) {
                    modListMoveDown();
                } else {
                    gModListPreviousCurrentLine = -2;
                    if (count > 0) {
                        if (gModListTopLine + MOD_MAX_MOD_LINES < count) {
                            gModListTopLine++;
                        } else if (gModListCurrentLine < MOD_MAX_MOD_LINES - 1 && (gModListTopLine + gModListCurrentLine + 1) < count) {
                            gModListCurrentLine++;
                        }
                        modListRefresh();
                    }
                }
                break;
            case KEY_PAGE_DOWN:
                gModListPreviousCurrentLine = -2;
                for (int i = 0; i < MOD_MAX_MOD_LINES; i++) {
                    if (gModListTopLine + MOD_MAX_MOD_LINES < count) {
                        gModListTopLine++;
                    } else if (gModListCurrentLine < MOD_MAX_MOD_LINES - 1 && (gModListTopLine + gModListCurrentLine + 1) < count) {
                        gModListCurrentLine++;
                    }
                }
                modListRefresh();
                break;
            case KEY_HOME:
                gModListTopLine = 0;
                gModListCurrentLine = 0;
                gModListPreviousCurrentLine = -2;
                modListRefresh();
                break;
            case KEY_END:
                if (count > MOD_MAX_MOD_LINES) {
                    gModListTopLine = count - MOD_MAX_MOD_LINES;
                    gModListCurrentLine = MOD_MAX_MOD_LINES - 1;
                } else {
                    gModListCurrentLine = count - 1;
                }
                gModListPreviousCurrentLine = -2;
                modListRefresh();
                break;
            }
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }
    return rc;
}

} // namespace fallout
