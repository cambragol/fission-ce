#include "mainmenu.h"

#include <ctype.h>

#include "art.h"
#include "color.h"
#include "draw.h"
#include "game.h"
#include "game_sound.h"
#include "input.h"
#include "kb.h"
#include "memory.h"
#include "mouse.h"
#include "palette.h"
#include "platform_compat.h"
#include "preferences.h"
#include "settings.h"
#include "sfall_config.h"
#include "svga.h"
#include "text_font.h"
#include "version.h"
#include "window_manager.h"

#include "platform/git_version.h"

namespace fallout {

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

// Hardcoded offsets
const MainMenuOffsets gMainMenuOffsets640 = {
    /*copyrightX*/ 15,
    /*copyrightY*/ 460,
    /*versionX*/ 615,
    /*versionY*/ 460,
    /*hashX*/ 615,
    /*hashX*/ 450,
    /*buildDateX*/ 615,
    /*buildDateX*/ 440,
    /*buttonBaseX*/ 30,
    /*buttonBaseY*/ 19,
    /*buttonTextOffsetX*/ 0,
    /*buttonTextOffsetY*/ 0,
    640,
    480
};

const MainMenuOffsets gMainMenuOffsets800 = {
    /*copyrightX*/ 15,
    /*copyrightY*/ 480,
    /*versionX*/ 780,
    /*versionY*/ 480,
    /*hashX*/ 780,
    /*hashX*/ 470,
    /*buildDateX*/ 780,
    /*buildDateX*/ 460,
    /*buttonBaseX*/ 47,
    /*buttonBaseY*/ 45,
    /*buttonTextOffsetX*/ 17,
    /*buttonTextOffsetY*/ 26,
    800,
    500
};

// 0x614840
static int gMainMenuButtons[MAIN_MENU_BUTTON_COUNT];

// 0x614858
static bool gMainMenuWindowHidden;

static FrmImage _mainMenuBackgroundFrmImage;
static FrmImage _mainMenuButtonNormalFrmImage;
static FrmImage _mainMenuButtonPressedFrmImage;

// move to seperate widescreen.cc file later?
bool mainMenuLoadOffsetsFromConfig(MainMenuOffsets* offsets, bool isWidescreen)
{
    const char* section = isWidescreen ? "mainmenu800" : "mainmenu640";
    const MainMenuOffsets* fallback = isWidescreen ? &gMainMenuOffsets800 : &gMainMenuOffsets640;

    // Initialize with fallback values
    *offsets = *fallback;

    // Load all values from config
    configGetInt(&gGameConfig, section, "copyrightX", &offsets->copyrightX);
    configGetInt(&gGameConfig, section, "copyrightY", &offsets->copyrightY);
    configGetInt(&gGameConfig, section, "versionX", &offsets->versionX);
    configGetInt(&gGameConfig, section, "versionY", &offsets->versionY);
    configGetInt(&gGameConfig, section, "hashX", &offsets->hashX);
    configGetInt(&gGameConfig, section, "hashY", &offsets->hashY);
    configGetInt(&gGameConfig, section, "buildDateX", &offsets->buildDateX);
    configGetInt(&gGameConfig, section, "buildDateY", &offsets->buildDateY);
    configGetInt(&gGameConfig, section, "buttonBaseX", &offsets->buttonBaseX);
    configGetInt(&gGameConfig, section, "buttonBaseY", &offsets->buttonBaseY);
    configGetInt(&gGameConfig, section, "buttonTextOffsetX", &offsets->buttonTextOffsetX);
    configGetInt(&gGameConfig, section, "buttonTextOffsetY", &offsets->buttonTextOffsetY);
    configGetInt(&gGameConfig, section, "width", &offsets->width);
    configGetInt(&gGameConfig, section, "height", &offsets->height);

    return true;
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

    if (gMainMenuWindowInitialized) {
        return 0;
    }

    // Set widescreen - must be wider in both axis and set to widescreen
    const bool isWidescreen = gameIsWidescreen();

    // Check if we should write defaults
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

    int backgroundFid = artGetFidWithVariant(OBJ_TYPE_INTERFACE, 140, "_800", gameIsWidescreen());
    if (!_mainMenuBackgroundFrmImage.lock(backgroundFid)) {
        // NOTE: Uninline.
        return main_menu_fatal_error();
    }

    blitBufferToBuffer(_mainMenuBackgroundFrmImage.getData(), gOffsets.width, gOffsets.height, gOffsets.width, gMainMenuWindowBuffer, gOffsets.width);
    _mainMenuBackgroundFrmImage.unlock();

    int oldFont = fontGetCurrent();
    fontSetCurrent(100);

    // SFALL: Allow to change font color/flags of copyright/version text
    //        It's the last byte ('3C' by default) that picks the colour used. The first byte supplies additional flags for this option
    //        0x010000 - change the color for version string only
    //        0x020000 - underline text (only for the version string)
    //        0x040000 - monospace font (only for the version string)
    int fontSettings = _colorTable[21091], fontSettingsSFall = 0;
    configGetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_FONT_COLOR_KEY, &fontSettingsSFall);
    if (fontSettingsSFall && !(fontSettingsSFall & 0x010000))
        fontSettings = fontSettingsSFall & 0xFF;

    // SFALL: Allow to move copyright text
    int offsetX = 0, offsetY = 0;
    configGetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_CREDITS_OFFSET_X_KEY, &offsetX);
    configGetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_CREDITS_OFFSET_Y_KEY, &offsetY);

    // Copyright.
    msg.num = 20;
    if (messageListGetItem(&gMiscMessageList, &msg)) {
        windowDrawText(gMainMenuWindow, msg.text, 0, offsetX + gOffsets.copyrightX, offsetY + gOffsets.copyrightY, fontSettings | 0x06000000);
    }

    // SFALL: Make sure font settings are applied when using 0x010000 flag
    if (fontSettingsSFall)
        fontSettings = fontSettingsSFall;

    // Version.
    char version[VERSION_MAX];
    versionGetVersion(version, sizeof(version));
    len = fontGetStringWidth(version);
    windowDrawText(gMainMenuWindow, version, 0, gOffsets.versionX - len, gOffsets.versionY, fontSettings | 0x06000000);

    // Hash
    char commitHash[VERSION_MAX] = "BUILD HASH: ";
    strcat(commitHash, _BUILD_HASH);
    len = fontGetStringWidth(commitHash);
    windowDrawText(gMainMenuWindow, commitHash, 0, gOffsets.hashX - len, gOffsets.hashY, fontSettings | 0x06000000);

    // Build Date
    char buildDate[VERSION_MAX] = "DATE: ";
    strcat(buildDate, _BUILD_DATE);
    len = fontGetStringWidth(buildDate);
    windowDrawText(gMainMenuWindow, buildDate, 0, gOffsets.buildDateX - len, gOffsets.buildDateY, fontSettings | 0x06000000);

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

    // SFALL: Allow to move menu buttons via offsetX and offsetY
    offsetX = offsetY = 0;
    configGetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_OFFSET_X_KEY, &offsetX);
    configGetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_OFFSET_Y_KEY, &offsetY);

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

    // SFALL: Allow to change font color of buttons
    fontSettings = _colorTable[21091];
    fontSettingsSFall = 0;
    configGetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_BIG_FONT_COLOR_KEY, &fontSettingsSFall);
    if (fontSettingsSFall)
        fontSettings = fontSettingsSFall & 0xFF;

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

} // namespace fallout
