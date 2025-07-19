#include "character_selector.h"

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <vector>

#include "art.h"
#include "character_editor.h"
#include "color.h"
#include "critter.h"
#include "db.h"
#include "debug.h"
#include "draw.h"
#include "game.h"
#include "game_sound.h"
#include "input.h"
#include "kb.h"
#include "memory.h"
#include "message.h"
#include "mouse.h"
#include "object.h"
#include "palette.h"
#include "platform_compat.h"
#include "preferences.h"
#include "proto.h"
#include "settings.h"
#include "sfall_config.h"
#include "skill.h"
#include "stat.h"
#include "svga.h"
#include "text_font.h"
#include "trait.h"
#include "window_manager.h"

namespace fallout {

// most defines replaced by offsets
#define CS_WINDOW_WIDTH (640)
#define CS_WINDOW_HEIGHT (480)

#define CS_WINDOW_BACKGROUND_X (40)
#define CS_WINDOW_BACKGROUND_Y (30)
#define CS_WINDOW_BACKGROUND_WIDTH (560)
#define CS_WINDOW_BACKGROUND_HEIGHT (300)

#define CS_WINDOW_PREVIOUS_BUTTON_X (292)
#define CS_WINDOW_PREVIOUS_BUTTON_Y (320)

#define CS_WINDOW_NEXT_BUTTON_X (318)
#define CS_WINDOW_NEXT_BUTTON_Y (320)

#define CS_WINDOW_TAKE_BUTTON_X (81)
#define CS_WINDOW_TAKE_BUTTON_Y (323)

#define CS_WINDOW_MODIFY_BUTTON_X (435)
#define CS_WINDOW_MODIFY_BUTTON_Y (320)

#define CS_WINDOW_CREATE_BUTTON_X (80)
#define CS_WINDOW_CREATE_BUTTON_Y (425)

#define CS_WINDOW_BACK_BUTTON_X (461)
#define CS_WINDOW_BACK_BUTTON_Y (425)

#define CS_WINDOW_NAME_MID_X (318)
#define CS_WINDOW_PRIMARY_STAT_MID_X (362)
#define CS_WINDOW_SECONDARY_STAT_MID_X (379)
#define CS_WINDOW_BIO_X (438)

typedef enum PremadeCharacter {
    PREMADE_CHARACTER_NARG,
    PREMADE_CHARACTER_CHITSA,
    PREMADE_CHARACTER_MINGUN,
    PREMADE_CHARACTER_COUNT,
} PremadeCharacter;

typedef struct PremadeCharacterDescription {
    char fileName[20];
    int face;
    char field_18[20];
} PremadeCharacterDescription;

static bool characterSelectorWindowInit();
static void characterSelectorWindowFree();
static bool characterSelectorWindowRefresh();
static bool characterSelectorWindowRenderFace();
static bool characterSelectorWindowRenderStats();
static bool characterSelectorWindowRenderBio();
static bool characterSelectorWindowFatalError(bool result);

static void premadeCharactersLocalizePath(char* path);

// 0x51C84C
static int gCurrentPremadeCharacter = PREMADE_CHARACTER_NARG;

// 0x51C850
static PremadeCharacterDescription gPremadeCharacterDescriptions[PREMADE_CHARACTER_COUNT] = {
    { "premade\\combat", 201, "VID 208-197-88-125" },
    { "premade\\stealth", 202, "VID 208-206-49-229" },
    { "premade\\diplomat", 203, "VID 208-206-49-227" },
};

// Added for offsets handling
static CharacterSelectorOffsets gOffsets;

const CharacterSelectorOffsets gCharSelectorOffsets640 = {
    // Window
    640, 480,

    // Background
    40, 30, 560, 300,

    // Buttons
    292, 320, // Previous
    318, 320, // Next
    81, 323, // Take
    435, 320, // Modify
    80, 425, // Create
    461, 425, // Back

    // Text positions
    318, // nameMidX
    362, // primaryStatMidX
    379, // secondaryStatMidX
    438, // bioX
    40, // textBaseY
    23, // faceY

    // Face position
    27, // faceX

    // Bio rendering
    260, // bioMaxY
};

const CharacterSelectorOffsets gCharSelectorOffsets800 = {
    // Window
    800, 500,

    // Background
    40, 30, 820, 300,

    // Buttons
    374, 330, // Previous
    397, 330, // Next
    100, 333, // Take
    583, 330, // Modify
    100, 435, // Create
    607, 435, // Back

    // Text positions
    396, // nameMidX
    462, // primaryStatMidX
    479, // secondaryStatMidX
    538, // bioX
    50, // textBaseY
    33, // faceY

    // Face position
    67, // faceX

    // Bio rendering
    260, // bioMaxY (same as 640)
};

// 0x51C8D4
static int gPremadeCharacterCount = PREMADE_CHARACTER_COUNT;

// 0x51C7F8
static int gCharacterSelectorWindow = -1;

// 0x51C7FC
static unsigned char* gCharacterSelectorWindowBuffer = nullptr;

// 0x51C800
static unsigned char* gCharacterSelectorBackground = nullptr;

// 0x51C804
static int gCharacterSelectorWindowPreviousButton = -1;

// 0x51C810
static int gCharacterSelectorWindowNextButton = -1;

// 0x51C81C
static int gCharacterSelectorWindowTakeButton = -1;

// 0x51C828
static int gCharacterSelectorWindowModifyButton = -1;

// 0x51C834
static int gCharacterSelectorWindowCreateButton = -1;

// 0x51C840
static int gCharacterSelectorWindowBackButton = -1;

static FrmImage _takeButtonNormalFrmImage;
static FrmImage _takeButtonPressedFrmImage;
static FrmImage _modifyButtonNormalFrmImage;
static FrmImage _modifyButtonPressedFrmImage;
static FrmImage _createButtonNormalFrmImage;
static FrmImage _createButtonPressedFrmImage;
static FrmImage _backButtonNormalFrmImage;
static FrmImage _backButtonPressedFrmImage;
static FrmImage _nextButtonNormalFrmImage;
static FrmImage _nextButtonPressedFrmImage;
static FrmImage _previousButtonNormalFrmImage;
static FrmImage _previousButtonPressedFrmImage;

static std::vector<PremadeCharacterDescription> gCustomPremadeCharacterDescriptions;

bool characterSelectorLoadOffsetsFromConfig(CharacterSelectorOffsets* offsets, bool isWidescreen)
{
    const char* section = isWidescreen ? "charselect800" : "charselect640";
    const CharacterSelectorOffsets* fallback = isWidescreen ? &gCharSelectorOffsets800 : &gCharSelectorOffsets640;

    // Initialize with fallback values
    *offsets = *fallback;

    // Load all values from config
    configGetInt(&gGameConfig, section, "width", &offsets->width);
    configGetInt(&gGameConfig, section, "height", &offsets->height);

    // Background
    configGetInt(&gGameConfig, section, "backgroundX", &offsets->backgroundX);
    configGetInt(&gGameConfig, section, "backgroundY", &offsets->backgroundY);
    configGetInt(&gGameConfig, section, "backgroundWidth", &offsets->backgroundWidth);
    configGetInt(&gGameConfig, section, "backgroundHeight", &offsets->backgroundHeight);

    // Buttons
    configGetInt(&gGameConfig, section, "previousButtonX", &offsets->previousButtonX);
    configGetInt(&gGameConfig, section, "previousButtonY", &offsets->previousButtonY);
    configGetInt(&gGameConfig, section, "nextButtonX", &offsets->nextButtonX);
    configGetInt(&gGameConfig, section, "nextButtonY", &offsets->nextButtonY);
    configGetInt(&gGameConfig, section, "takeButtonX", &offsets->takeButtonX);
    configGetInt(&gGameConfig, section, "takeButtonY", &offsets->takeButtonY);
    configGetInt(&gGameConfig, section, "modifyButtonX", &offsets->modifyButtonX);
    configGetInt(&gGameConfig, section, "modifyButtonY", &offsets->modifyButtonY);
    configGetInt(&gGameConfig, section, "createButtonX", &offsets->createButtonX);
    configGetInt(&gGameConfig, section, "createButtonY", &offsets->createButtonY);
    configGetInt(&gGameConfig, section, "backButtonX", &offsets->backButtonX);
    configGetInt(&gGameConfig, section, "backButtonY", &offsets->backButtonY);

    // Text positions
    configGetInt(&gGameConfig, section, "nameMidX", &offsets->nameMidX);
    configGetInt(&gGameConfig, section, "primaryStatMidX", &offsets->primaryStatMidX);
    configGetInt(&gGameConfig, section, "secondaryStatMidX", &offsets->secondaryStatMidX);
    configGetInt(&gGameConfig, section, "bioX", &offsets->bioX);
    configGetInt(&gGameConfig, section, "textBaseY", &offsets->textBaseY);
    configGetInt(&gGameConfig, section, "faceY", &offsets->faceY);

    // Face position
    configGetInt(&gGameConfig, section, "faceX", &offsets->faceX);

    // Bio rendering
    configGetInt(&gGameConfig, section, "bioMaxY", &offsets->bioMaxY);

    return true;
}

void characterSelectorWriteDefaultOffsetsToConfig(bool isWidescreen, const CharacterSelectorOffsets* defaults)
{
    const char* section = isWidescreen ? "charselect800" : "charselect640";

    // Window dimensions
    configSetInt(&gGameConfig, section, "width", defaults->width);
    configSetInt(&gGameConfig, section, "height", defaults->height);

    // Background
    configSetInt(&gGameConfig, section, "backgroundX", defaults->backgroundX);
    configSetInt(&gGameConfig, section, "backgroundY", defaults->backgroundY);
    configSetInt(&gGameConfig, section, "backgroundWidth", defaults->backgroundWidth);
    configSetInt(&gGameConfig, section, "backgroundHeight", defaults->backgroundHeight);

    // Buttons
    configSetInt(&gGameConfig, section, "previousButtonX", defaults->previousButtonX);
    configSetInt(&gGameConfig, section, "previousButtonY", defaults->previousButtonY);
    configSetInt(&gGameConfig, section, "nextButtonX", defaults->nextButtonX);
    configSetInt(&gGameConfig, section, "nextButtonY", defaults->nextButtonY);
    configSetInt(&gGameConfig, section, "takeButtonX", defaults->takeButtonX);
    configSetInt(&gGameConfig, section, "takeButtonY", defaults->takeButtonY);
    configSetInt(&gGameConfig, section, "modifyButtonX", defaults->modifyButtonX);
    configSetInt(&gGameConfig, section, "modifyButtonY", defaults->modifyButtonY);
    configSetInt(&gGameConfig, section, "createButtonX", defaults->createButtonX);
    configSetInt(&gGameConfig, section, "createButtonY", defaults->createButtonY);
    configSetInt(&gGameConfig, section, "backButtonX", defaults->backButtonX);
    configSetInt(&gGameConfig, section, "backButtonY", defaults->backButtonY);

    // Text positions
    configSetInt(&gGameConfig, section, "nameMidX", defaults->nameMidX);
    configSetInt(&gGameConfig, section, "primaryStatMidX", defaults->primaryStatMidX);
    configSetInt(&gGameConfig, section, "secondaryStatMidX", defaults->secondaryStatMidX);
    configSetInt(&gGameConfig, section, "bioX", defaults->bioX);
    configSetInt(&gGameConfig, section, "textBaseY", defaults->textBaseY);
    configSetInt(&gGameConfig, section, "faceY", defaults->faceY);

    // Face position
    configSetInt(&gGameConfig, section, "faceX", defaults->faceX);

    // Bio rendering
    configSetInt(&gGameConfig, section, "bioMaxY", defaults->bioMaxY);
}

// 0x4A71D0
int characterSelectorOpen()
{
    if (!characterSelectorWindowInit()) {
        return 0;
    }

    bool cursorWasHidden = cursorIsHidden();
    if (cursorWasHidden) {
        mouseShowCursor();
    }

    colorPaletteLoad("color.pal");
    paletteFadeTo(_cmap);

    int rc = 0;
    bool done = false;
    while (!done) {
        sharedFpsLimiter.mark();

        if (_game_user_wants_to_quit != 0) {
            break;
        }

        int keyCode = inputGetInput();

        switch (keyCode) {
        case KEY_MINUS:
        case KEY_UNDERSCORE:
            brightnessDecrease();
            break;
        case KEY_EQUAL:
        case KEY_PLUS:
            brightnessIncrease();
            break;
        case KEY_UPPERCASE_B:
        case KEY_LOWERCASE_B:
        case KEY_ESCAPE:
            rc = 3;
            done = true;
            break;
        case KEY_UPPERCASE_C:
        case KEY_LOWERCASE_C:
            _ResetPlayer();
            if (characterEditorShow(1) == 0) {
                rc = 2;
                done = true;
            } else {
                characterSelectorWindowRefresh();
            }

            break;
        case KEY_UPPERCASE_M:
        case KEY_LOWERCASE_M:
            if (!characterEditorShow(1)) {
                rc = 2;
                done = true;
            } else {
                characterSelectorWindowRefresh();
            }

            break;
        case KEY_UPPERCASE_T:
        case KEY_LOWERCASE_T:
            rc = 2;
            done = true;

            break;
        case KEY_F10:
            showQuitConfirmationDialog();
            break;
        case KEY_ARROW_LEFT:
            soundPlayFile("ib2p1xx1");
            // FALLTHROUGH
        case 500:
            gCurrentPremadeCharacter -= 1;
            if (gCurrentPremadeCharacter < 0) {
                gCurrentPremadeCharacter = gPremadeCharacterCount - 1;
            }

            characterSelectorWindowRefresh();
            break;
        case KEY_ARROW_RIGHT:
            soundPlayFile("ib2p1xx1");
            // FALLTHROUGH
        case 501:
            gCurrentPremadeCharacter += 1;
            if (gCurrentPremadeCharacter >= gPremadeCharacterCount) {
                gCurrentPremadeCharacter = 0;
            }

            characterSelectorWindowRefresh();
            break;
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    paletteFadeTo(gPaletteBlack);
    characterSelectorWindowFree();

    if (cursorWasHidden) {
        mouseHideCursor();
    }

    return rc;
}

// 0x4A7468
static bool characterSelectorWindowInit()
{
    if (gCharacterSelectorWindow != -1) {
        return false;
    }

    // Check if we should write defaults
    int writeOffsets = 0;
    if (configGetInt(&gGameConfig, "debug", "write_offsets", &writeOffsets) && writeOffsets) {
        characterSelectorWriteDefaultOffsetsToConfig(false, &gCharSelectorOffsets640);
        characterSelectorWriteDefaultOffsetsToConfig(true, &gCharSelectorOffsets800);
        configSetInt(&gGameConfig, "debug", "write_offsets", 0);
        gameConfigSave();
    }

    // Load offsets directly into global struct
    characterSelectorLoadOffsetsFromConfig(&gOffsets, gameIsWidescreen());

    int characterSelectorWindowX = (screenGetWidth() - gOffsets.width) / 2;
    int characterSelectorWindowY = (screenGetHeight() - gOffsets.height) / 2;
    gCharacterSelectorWindow = windowCreate(
        characterSelectorWindowX,
        characterSelectorWindowY,
        gOffsets.width,
        gOffsets.height,
        _colorTable[0],
        0);
    if (gCharacterSelectorWindow == -1) {
        return characterSelectorWindowFatalError(false);
    }

    gCharacterSelectorWindowBuffer = windowGetBuffer(gCharacterSelectorWindow);
    if (gCharacterSelectorWindowBuffer == nullptr) {
        return characterSelectorWindowFatalError(false);
    }

    FrmImage backgroundFrmImage;
    int backgroundFid = artGetFidWithVariant(OBJ_TYPE_INTERFACE, 174, "_800", gameIsWidescreen());
    if (!backgroundFrmImage.lock(backgroundFid)) {
        return characterSelectorWindowFatalError(false);
    }

    // Blit full background using offsets
    blitBufferToBuffer(
        backgroundFrmImage.getData(),
        gOffsets.width,
        gOffsets.height,
        gOffsets.width,
        gCharacterSelectorWindowBuffer,
        gOffsets.width);

    // Allocate and blit background area using offsets
    gCharacterSelectorBackground = (unsigned char*)internal_malloc(
        gOffsets.backgroundWidth * gOffsets.backgroundHeight);
    if (gCharacterSelectorBackground == nullptr) {
        return characterSelectorWindowFatalError(false);
    }

    blitBufferToBuffer(
        backgroundFrmImage.getData() + gOffsets.width * gOffsets.backgroundY + gOffsets.backgroundX,
        gOffsets.backgroundWidth,
        gOffsets.backgroundHeight,
        gOffsets.width,
        gCharacterSelectorBackground,
        gOffsets.backgroundWidth);

    int fid;

    // Setup "Previous" button.
    fid = buildFid(OBJ_TYPE_INTERFACE, 122, 0, 0, 0);
    if (!_previousButtonNormalFrmImage.lock(fid)) {
        return characterSelectorWindowFatalError(false);
    }

    fid = buildFid(OBJ_TYPE_INTERFACE, 123, 0, 0, 0);
    if (!_previousButtonPressedFrmImage.lock(fid)) {
        return characterSelectorWindowFatalError(false);
    }

    gCharacterSelectorWindowPreviousButton = buttonCreate(
        gCharacterSelectorWindow,
        gOffsets.previousButtonX,
        gOffsets.previousButtonY,
        20,
        18,
        -1,
        -1,
        -1,
        500,
        _previousButtonNormalFrmImage.getData(),
        _previousButtonPressedFrmImage.getData(),
        nullptr,
        0);
    if (gCharacterSelectorWindowPreviousButton == -1) {
        return characterSelectorWindowFatalError(false);
    }
    buttonSetCallbacks(gCharacterSelectorWindowPreviousButton, _gsound_med_butt_press, _gsound_med_butt_release);

    // Setup "Next" button.
    fid = buildFid(OBJ_TYPE_INTERFACE, 124, 0, 0, 0);
    if (!_nextButtonNormalFrmImage.lock(fid)) {
        return characterSelectorWindowFatalError(false);
    }

    fid = buildFid(OBJ_TYPE_INTERFACE, 125, 0, 0, 0);
    if (!_nextButtonPressedFrmImage.lock(fid)) {
        return characterSelectorWindowFatalError(false);
    }

    gCharacterSelectorWindowNextButton = buttonCreate(
        gCharacterSelectorWindow,
        gOffsets.nextButtonX,
        gOffsets.nextButtonY,
        20,
        18,
        -1,
        -1,
        -1,
        501,
        _nextButtonNormalFrmImage.getData(),
        _nextButtonPressedFrmImage.getData(),
        nullptr,
        0);
    if (gCharacterSelectorWindowNextButton == -1) {
        return characterSelectorWindowFatalError(false);
    }
    buttonSetCallbacks(gCharacterSelectorWindowNextButton, _gsound_med_butt_press, _gsound_med_butt_release);

    // Setup "Take" button.
    fid = buildFid(OBJ_TYPE_INTERFACE, 8, 0, 0, 0);
    if (!_takeButtonNormalFrmImage.lock(fid)) {
        return characterSelectorWindowFatalError(false);
    }

    fid = buildFid(OBJ_TYPE_INTERFACE, 9, 0, 0, 0);
    if (!_takeButtonPressedFrmImage.lock(fid)) {
        return characterSelectorWindowFatalError(false);
    }

    gCharacterSelectorWindowTakeButton = buttonCreate(
        gCharacterSelectorWindow,
        gOffsets.takeButtonX,
        gOffsets.takeButtonY,
        15,
        16,
        -1,
        -1,
        -1,
        KEY_LOWERCASE_T,
        _takeButtonNormalFrmImage.getData(),
        _takeButtonPressedFrmImage.getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (gCharacterSelectorWindowTakeButton == -1) {
        return characterSelectorWindowFatalError(false);
    }
    buttonSetCallbacks(gCharacterSelectorWindowTakeButton, _gsound_red_butt_press, _gsound_red_butt_release);

    // Setup "Modify" button.
    fid = buildFid(OBJ_TYPE_INTERFACE, 8, 0, 0, 0);
    if (!_modifyButtonNormalFrmImage.lock(fid)) {
        return characterSelectorWindowFatalError(false);
    }

    fid = buildFid(OBJ_TYPE_INTERFACE, 9, 0, 0, 0);
    if (!_modifyButtonPressedFrmImage.lock(fid)) {
        return characterSelectorWindowFatalError(false);
    }

    gCharacterSelectorWindowModifyButton = buttonCreate(
        gCharacterSelectorWindow,
        gOffsets.modifyButtonX,
        gOffsets.modifyButtonY,
        15,
        16,
        -1,
        -1,
        -1,
        KEY_LOWERCASE_M,
        _modifyButtonNormalFrmImage.getData(),
        _modifyButtonPressedFrmImage.getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (gCharacterSelectorWindowModifyButton == -1) {
        return characterSelectorWindowFatalError(false);
    }
    buttonSetCallbacks(gCharacterSelectorWindowModifyButton, _gsound_red_butt_press, _gsound_red_butt_release);

    // Setup "Create" button.
    fid = buildFid(OBJ_TYPE_INTERFACE, 8, 0, 0, 0);
    if (!_createButtonNormalFrmImage.lock(fid)) {
        return characterSelectorWindowFatalError(false);
    }

    fid = buildFid(OBJ_TYPE_INTERFACE, 9, 0, 0, 0);
    if (!_createButtonPressedFrmImage.lock(fid)) {
        return characterSelectorWindowFatalError(false);
    }

    gCharacterSelectorWindowCreateButton = buttonCreate(
        gCharacterSelectorWindow,
        gOffsets.createButtonX,
        gOffsets.createButtonY,
        15,
        16,
        -1,
        -1,
        -1,
        KEY_LOWERCASE_C,
        _createButtonNormalFrmImage.getData(),
        _createButtonPressedFrmImage.getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (gCharacterSelectorWindowCreateButton == -1) {
        return characterSelectorWindowFatalError(false);
    }
    buttonSetCallbacks(gCharacterSelectorWindowCreateButton, _gsound_red_butt_press, _gsound_red_butt_release);

    // Setup "Back" button.
    fid = buildFid(OBJ_TYPE_INTERFACE, 8, 0, 0, 0);
    if (!_backButtonNormalFrmImage.lock(fid)) {
        return characterSelectorWindowFatalError(false);
    }

    fid = buildFid(OBJ_TYPE_INTERFACE, 9, 0, 0, 0);
    if (!_backButtonPressedFrmImage.lock(fid)) {
        return characterSelectorWindowFatalError(false);
    }

    gCharacterSelectorWindowBackButton = buttonCreate(
        gCharacterSelectorWindow,
        gOffsets.backButtonX,
        gOffsets.backButtonY,
        15,
        16,
        -1,
        -1,
        -1,
        KEY_ESCAPE,
        _backButtonNormalFrmImage.getData(),
        _backButtonPressedFrmImage.getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (gCharacterSelectorWindowBackButton == -1) {
        return characterSelectorWindowFatalError(false);
    }
    buttonSetCallbacks(gCharacterSelectorWindowBackButton, _gsound_red_butt_press, _gsound_red_butt_release);
    gCurrentPremadeCharacter = PREMADE_CHARACTER_NARG;

    windowRefresh(gCharacterSelectorWindow);

    if (!characterSelectorWindowRefresh()) {
        return characterSelectorWindowFatalError(false);
    }

    return true;
}

// 0x4A7AD4
static void characterSelectorWindowFree()
{
    if (gCharacterSelectorWindow == -1) {
        return;
    }

    if (gCharacterSelectorWindowPreviousButton != -1) {
        buttonDestroy(gCharacterSelectorWindowPreviousButton);
        gCharacterSelectorWindowPreviousButton = -1;
    }

    _previousButtonNormalFrmImage.unlock();
    _previousButtonPressedFrmImage.unlock();

    if (gCharacterSelectorWindowNextButton != -1) {
        buttonDestroy(gCharacterSelectorWindowNextButton);
        gCharacterSelectorWindowNextButton = -1;
    }

    _nextButtonNormalFrmImage.unlock();
    _nextButtonPressedFrmImage.unlock();

    if (gCharacterSelectorWindowTakeButton != -1) {
        buttonDestroy(gCharacterSelectorWindowTakeButton);
        gCharacterSelectorWindowTakeButton = -1;
    }

    _takeButtonNormalFrmImage.unlock();
    _takeButtonPressedFrmImage.unlock();

    if (gCharacterSelectorWindowModifyButton != -1) {
        buttonDestroy(gCharacterSelectorWindowModifyButton);
        gCharacterSelectorWindowModifyButton = -1;
    }

    _modifyButtonNormalFrmImage.unlock();
    _modifyButtonPressedFrmImage.unlock();

    if (gCharacterSelectorWindowCreateButton != -1) {
        buttonDestroy(gCharacterSelectorWindowCreateButton);
        gCharacterSelectorWindowCreateButton = -1;
    }

    _createButtonNormalFrmImage.unlock();
    _createButtonPressedFrmImage.unlock();

    if (gCharacterSelectorWindowBackButton != -1) {
        buttonDestroy(gCharacterSelectorWindowBackButton);
        gCharacterSelectorWindowBackButton = -1;
    }

    _backButtonNormalFrmImage.unlock();
    _backButtonPressedFrmImage.unlock();

    if (gCharacterSelectorBackground != nullptr) {
        internal_free(gCharacterSelectorBackground);
        gCharacterSelectorBackground = nullptr;
    }

    windowDestroy(gCharacterSelectorWindow);
    gCharacterSelectorWindow = -1;
}

// 0x4A7D58
static bool characterSelectorWindowRefresh()
{
    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s.gcd", gCustomPremadeCharacterDescriptions[gCurrentPremadeCharacter].fileName);
    premadeCharactersLocalizePath(path);

    if (_proto_dude_init(path) == -1) {
        debugPrint("\n ** Error in dude init! **\n");
        return false;
    }

    blitBufferToBuffer(gCharacterSelectorBackground,
        gOffsets.backgroundWidth,
        gOffsets.backgroundHeight,
        gOffsets.backgroundWidth,
        gCharacterSelectorWindowBuffer + gOffsets.width * gOffsets.backgroundY + gOffsets.backgroundX,
        gOffsets.width);

    bool success = false;
    if (characterSelectorWindowRenderFace()) {
        if (characterSelectorWindowRenderStats()) {
            success = characterSelectorWindowRenderBio();
        }
    }

    windowRefresh(gCharacterSelectorWindow);

    return success;
}

static bool characterSelectorWindowRenderFace()
{
    bool success = false;

    FrmImage faceFrmImage;
    int faceFid = buildFid(OBJ_TYPE_INTERFACE, gCustomPremadeCharacterDescriptions[gCurrentPremadeCharacter].face, 0, 0, 0);
    if (faceFrmImage.lock(faceFid)) {
        unsigned char* data = faceFrmImage.getData();
        if (data != nullptr) {
            int width = faceFrmImage.getWidth();
            int height = faceFrmImage.getHeight();
            // Use offset-based position
            int offset = gOffsets.width * gOffsets.faceY + gOffsets.faceX;
            blitBufferToBufferTrans(data, width, height, width,
                gCharacterSelectorWindowBuffer + offset,
                gOffsets.width);
            success = true;
        }
        faceFrmImage.unlock();
    }

    return success;
}

static bool characterSelectorWindowRenderStats()
{
    char* str;
    char text[260];
    int length;
    int value;
    MessageListItem messageListItem;

    int oldFont = fontGetCurrent();
    fontSetCurrent(101);

    fontGetCharacterWidth(0x20);

    int vh = fontGetLineHeight();
    int y = gOffsets.textBaseY;

    // NAME
    str = objectGetName(gDude);
    strcpy(text, str);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.nameMidX - (length / 2),
        text, 160, gOffsets.width, _colorTable[992]);

    // STRENGTH
    y += vh + vh + vh;

    value = critterGetStat(gDude, STAT_STRENGTH);
    str = statGetName(STAT_STRENGTH);

    snprintf(text, sizeof(text), "%s %02d", str, value);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX - length,
        text, length, gOffsets.width, _colorTable[992]);

    str = statGetValueDescription(value);
    snprintf(text, sizeof(text), "  %s", str);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX,
        text, length, gOffsets.width, _colorTable[992]);

    // PERCEPTION
    y += vh;

    value = critterGetStat(gDude, STAT_PERCEPTION);
    str = statGetName(STAT_PERCEPTION);

    snprintf(text, sizeof(text), "%s %02d", str, value);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX - length,
        text, length, gOffsets.width, _colorTable[992]);

    str = statGetValueDescription(value);
    snprintf(text, sizeof(text), "  %s", str);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX,
        text, length, gOffsets.width, _colorTable[992]);

    // ENDURANCE
    y += vh;

    value = critterGetStat(gDude, STAT_ENDURANCE);
    str = statGetName(STAT_ENDURANCE);

    snprintf(text, sizeof(text), "%s %02d", str, value);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX - length,
        text, length, gOffsets.width, _colorTable[992]);

    str = statGetValueDescription(value);
    snprintf(text, sizeof(text), "  %s", str);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX,
        text, length, gOffsets.width, _colorTable[992]);

    // CHARISMA
    y += vh;

    value = critterGetStat(gDude, STAT_CHARISMA);
    str = statGetName(STAT_CHARISMA);

    snprintf(text, sizeof(text), "%s %02d", str, value);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX - length,
        text, length, gOffsets.width, _colorTable[992]);

    str = statGetValueDescription(value);
    snprintf(text, sizeof(text), "  %s", str);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX,
        text, length, gOffsets.width, _colorTable[992]);

    // INTELLIGENCE
    y += vh;

    value = critterGetStat(gDude, STAT_INTELLIGENCE);
    str = statGetName(STAT_INTELLIGENCE);

    snprintf(text, sizeof(text), "%s %02d", str, value);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX - length,
        text, length, gOffsets.width, _colorTable[992]);

    str = statGetValueDescription(value);
    snprintf(text, sizeof(text), "  %s", str);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX,
        text, length, gOffsets.width, _colorTable[992]);

    // AGILITY
    y += vh;

    value = critterGetStat(gDude, STAT_AGILITY);
    str = statGetName(STAT_AGILITY);

    snprintf(text, sizeof(text), "%s %02d", str, value);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX - length,
        text, length, gOffsets.width, _colorTable[992]);

    str = statGetValueDescription(value);
    snprintf(text, sizeof(text), "  %s", str);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX,
        text, length, gOffsets.width, _colorTable[992]);

    // LUCK
    y += vh;

    value = critterGetStat(gDude, STAT_LUCK);
    str = statGetName(STAT_LUCK);

    snprintf(text, sizeof(text), "%s %02d", str, value);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX - length,
        text, length, gOffsets.width, _colorTable[992]);

    str = statGetValueDescription(value);
    snprintf(text, sizeof(text), "  %s", str);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.primaryStatMidX,
        text, length, gOffsets.width, _colorTable[992]);

    y += vh; // blank line

    // HIT POINTS
    y += vh;

    messageListItem.num = 16;
    text[0] = '\0';
    if (messageListGetItem(&gMiscMessageList, &messageListItem)) {
        strcpy(text, messageListItem.text);
    }

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.secondaryStatMidX - length,
        text, length, gOffsets.width, _colorTable[992]);

    value = critterGetStat(gDude, STAT_MAXIMUM_HIT_POINTS);
    snprintf(text, sizeof(text), " %d/%d", critterGetHitPoints(gDude), value);

    length = fontGetStringWidth(text);
    // Use offset-based position
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.secondaryStatMidX,
        text, length, gOffsets.width, _colorTable[992]);

    // ARMOR CLASS
    y += vh;

    str = statGetName(STAT_ARMOR_CLASS);
    strcpy(text, str);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.secondaryStatMidX - length,
        text, length, gOffsets.width, _colorTable[992]);

    value = critterGetStat(gDude, STAT_ARMOR_CLASS);
    snprintf(text, sizeof(text), " %d", value);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.secondaryStatMidX,
        text, length, gOffsets.width, _colorTable[992]);

    // ACTION POINTS
    y += vh;

    messageListItem.num = 15;
    text[0] = '\0';
    if (messageListGetItem(&gMiscMessageList, &messageListItem)) {
        strcpy(text, messageListItem.text);
    }

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.secondaryStatMidX - length,
        text, length, gOffsets.width, _colorTable[992]);

    value = critterGetStat(gDude, STAT_MAXIMUM_ACTION_POINTS);
    snprintf(text, sizeof(text), " %d", value);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.secondaryStatMidX,
        text, length, gOffsets.width, _colorTable[992]);

    // MELEE DAMAGE
    y += vh;

    str = statGetName(STAT_MELEE_DAMAGE);
    strcpy(text, str);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.secondaryStatMidX - length,
        text, length, gOffsets.width, _colorTable[992]);

    value = critterGetStat(gDude, STAT_MELEE_DAMAGE);
    snprintf(text, sizeof(text), " %d", value);

    length = fontGetStringWidth(text);
    fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.secondaryStatMidX,
        text, length, gOffsets.width, _colorTable[992]);

    y += vh; // blank line

    // SKILLS
    int skills[DEFAULT_TAGGED_SKILLS];
    skillsGetTagged(skills, DEFAULT_TAGGED_SKILLS);

    for (int index = 0; index < DEFAULT_TAGGED_SKILLS; index++) {
        y += vh;

        str = skillGetName(skills[index]);
        strcpy(text, str);

        length = fontGetStringWidth(text);
        fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.secondaryStatMidX - length,
            text, length, gOffsets.width, _colorTable[992]);

        value = skillGetValue(gDude, skills[index]);
        snprintf(text, sizeof(text), " %d%%", value);

        length = fontGetStringWidth(text);
        fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.secondaryStatMidX,
            text, length, gOffsets.width, _colorTable[992]);
    }

    // TRAITS
    int traits[TRAITS_MAX_SELECTED_COUNT];
    traitsGetSelected(&(traits[0]), &(traits[1]));

    for (int index = 0; index < TRAITS_MAX_SELECTED_COUNT; index++) {
        y += vh;

        str = traitGetName(traits[index]);
        strcpy(text, str);

        length = fontGetStringWidth(text);
        fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.secondaryStatMidX - length,
            text, length, gOffsets.width, _colorTable[992]);
    }

    fontSetCurrent(oldFont);

    return true;
}

// 0x4A8AE4
static bool characterSelectorWindowRenderBio()
{
    int oldFont = fontGetCurrent();
    fontSetCurrent(101);

    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s.bio", gCustomPremadeCharacterDescriptions[gCurrentPremadeCharacter].fileName);
    premadeCharactersLocalizePath(path);

    File* stream = fileOpen(path, "rt");
    if (stream != nullptr) {
        int y = gOffsets.textBaseY;
        int lineHeight = fontGetLineHeight();

        char string[256];
        while (fileReadString(string, 256, stream) && y < gOffsets.bioMaxY) {
            fontDrawText(gCharacterSelectorWindowBuffer + gOffsets.width * y + gOffsets.bioX,
                string,
                gOffsets.width - gOffsets.bioX,
                gOffsets.width,
                _colorTable[992]);
            y += lineHeight;
        }

        fileClose(stream);
    }

    fontSetCurrent(oldFont);

    return true;
}

// NOTE: Inlined.
//
// 0x4A8BD0
static bool characterSelectorWindowFatalError(bool result)
{
    characterSelectorWindowFree();
    return result;
}

void premadeCharactersInit()
{
    char* fileNamesString;
    configGetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PREMADE_CHARACTERS_FILE_NAMES_KEY, &fileNamesString);
    if (fileNamesString != nullptr && *fileNamesString == '\0') {
        fileNamesString = nullptr;
    }

    char* faceFidsString;
    configGetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PREMADE_CHARACTERS_FACE_FIDS_KEY, &faceFidsString);
    if (faceFidsString != nullptr && *faceFidsString == '\0') {
        faceFidsString = nullptr;
    }

    if (fileNamesString != nullptr && faceFidsString != nullptr) {
        int fileNamesLength = 0;
        for (char* pch = fileNamesString; pch != nullptr; pch = strchr(pch + 1, ',')) {
            fileNamesLength++;
        }

        int faceFidsLength = 0;
        for (char* pch = faceFidsString; pch != nullptr; pch = strchr(pch + 1, ',')) {
            faceFidsLength++;
        }

        int premadeCharactersCount = std::min(fileNamesLength, faceFidsLength);
        gCustomPremadeCharacterDescriptions.resize(premadeCharactersCount);

        for (int index = 0; index < premadeCharactersCount; index++) {
            char* pch;

            pch = strchr(fileNamesString, ',');
            if (pch != nullptr) {
                *pch = '\0';
            }

            if (strlen(fileNamesString) > 11) {
                // Sfall fails here.
                continue;
            }

            snprintf(gCustomPremadeCharacterDescriptions[index].fileName, sizeof(gCustomPremadeCharacterDescriptions[index].fileName), "premade\\%s", fileNamesString);

            if (pch != nullptr) {
                *pch = ',';
            }

            fileNamesString = pch + 1;

            pch = strchr(faceFidsString, ',');
            if (pch != nullptr) {
                *pch = '\0';
            }

            gCustomPremadeCharacterDescriptions[index].face = atoi(faceFidsString);

            if (pch != nullptr) {
                *pch = ',';
            }

            faceFidsString = pch + 1;

            gCustomPremadeCharacterDescriptions[index].field_18[0] = '\0';
        }
    }

    if (gCustomPremadeCharacterDescriptions.empty()) {
        gCustomPremadeCharacterDescriptions.resize(PREMADE_CHARACTER_COUNT);

        for (int index = 0; index < PREMADE_CHARACTER_COUNT; index++) {
            strcpy(gCustomPremadeCharacterDescriptions[index].fileName, gPremadeCharacterDescriptions[index].fileName);
            gCustomPremadeCharacterDescriptions[index].face = gPremadeCharacterDescriptions[index].face;
            strcpy(gCustomPremadeCharacterDescriptions[index].field_18, gPremadeCharacterDescriptions[index].field_18);
        }
    }

    gPremadeCharacterCount = gCustomPremadeCharacterDescriptions.size();
}

void premadeCharactersExit()
{
    gCustomPremadeCharacterDescriptions.clear();
}

static void premadeCharactersLocalizePath(char* path)
{
    if (compat_strnicmp(path, "premade\\", 8) != 0) {
        return;
    }

    const char* language = settings.system.language.c_str();
    if (compat_stricmp(language, ENGLISH) == 0) {
        return;
    }

    char localizedPath[COMPAT_MAX_PATH];
    strncpy(localizedPath, path, 8);
    strcpy(localizedPath + 8, language);
    strcpy(localizedPath + 8 + strlen(language), path + 7);

    int fileSize;
    if (dbGetFileSize(localizedPath, &fileSize) == 0) {
        strcpy(path, localizedPath);
    }
}

} // namespace fallout
