#include "preferences.h"

#include "options.h"

#include <algorithm>

#include "SDL.h"
#include "art.h"
#include "color.h"
#include "combat.h"
#include "combat_ai.h"
#include "dbox.h"
#include "debug.h"
#include "delay.h"
#include "draw.h"
#include "game.h"
#include "game_mouse.h"
#include "game_sound.h"
#include "graph_lib.h"
#include "input.h"
#include "kb.h"
#include "math.h"
#include "memory.h"
#include "message.h"
#include "mouse.h"
#include "offsets.h"
#include "palette.h"
#include "scripts.h"
#include "settings.h"
#include "svga.h"
#include "text_font.h"
#include "text_object.h"
#include "window_manager.h"

namespace fallout {

static void _SetSystemPrefs();
static void _SaveSettings();
static void _RestoreSettings();
static void preferencesSetDefaults(bool a1);
static void _JustUpdate_();
static void _UpdateThing(int index);
int _SavePrefs(bool save);
static int preferencesWindowInit();
static int preferencesWindowFree();
static void _DoThing(int eventCode);

// 0x50C168
static const double dbl_50C168 = 1.17999267578125;

// 0x50C170
static const double dbl_50C170 = 0.01124954223632812;

// 0x50C178
static const double dbl_50C178 = -0.01124954223632812;

// 0x50C180
static const double dbl_50C180 = 1.17999267578125;

// 0x50C2D0
static const double dbl_50C2D0 = -1.0;

// 0x50C2D8
static const double dbl_50C2D8 = 0.2;

// 0x50C2E0
static const double dbl_50C2E0 = 2.0;

// 0x5197CC
static const int gPreferencesWindowFrmIds[PREFERENCES_WINDOW_FRM_COUNT] = {
    240, // prefscrn.frm - options screen
    241, // prfsldof.frm - options screen
    242, // prfbknbs.frm - options screen
    243, // prflknbs.frm - options screen
    244, // prfxin.frm - options screen
    245, // prfxout.frm - options screen
    246, // prefcvr.frm - options screen
    247, // prfsldon.frm - options screen
    8, // lilredup.frm - little red button up
    9, // lilreddn.frm - little red button down
    172, // autoup.frm - toggle switch up
    6365 // prfdial.frm - large dial
};

// 0x6637E8
static MessageList gPreferencesMessageList;

// 0x663840
static MessageListItem gPreferencesMessageListItem;

static MessageList gFissionMessageList;

static MessageListItem gFissionMessageListItem;

// 0x6638C8
static double gPreferencesTextBaseDelay2;

// 0x6638D0
static double gPreferencesBrightness1;

// 0x6638D8
static double gPreferencesBrightness2;

// 0x6638E0
static double gPreferencesTextBaseDelay1;

// 0x6638E8
static double gPreferencesMouseSensitivity1;

// 0x6638F0
static double gPreferencesMouseSensitivity2;

// 0x6638F8
static unsigned char* gPreferencesWindowBuffer;

// 0x663904
static int gPreferencesWindow;

// 0x663924
static int gPreferencesGameDifficulty2;

// 0x663928
static int gPreferencesCombatDifficulty2;

// 0x66392C
static int gPreferencesViolenceLevel2;

// 0x663930
static int gPreferencesTargetHighlight2;

// 0x663934
static int gPreferencesCombatLooks2;

// 0x663938
static int gPreferencesCombatMessages2;

// 0x66393C
static int gPreferencesCombatTaunts2;

// 0x663940
static int gPreferencesLanguageFilter2;

// 0x663944
static int gPreferencesRunning2;

// 0x663948
static int gPreferencesSubtitles2;

// 0x66394C
static int gPreferencesItemHighlight2;

// 0x663950
static int gPreferencesCombatSpeed2;

// 0x663954
static int gPreferencesPlayerSpeedup2;

// 0x663958
static int gPreferencesMasterVolume2;

// 0x66395C
static int gPreferencesMusicVolume2;

// 0x663960
static int gPreferencesSoundEffectsVolume2;

// 0x663964
static int gPreferencesSpeechVolume2;

// 0x663970
static int gPreferencesSoundEffectsVolume1;

// 0x663974
static int gPreferencesSubtitles1;

// 0x663978
static int gPreferencesLanguageFilter1;

// 0x66397C
static int gPreferencesSpeechVolume1;

// 0x663980
static int gPreferencesMasterVolume1;

// 0x663984
static int gPreferencesPlayerSpeedup1;

// 0x663988
static int gPreferencesCombatTaunts1;

// 0x663990
static int gPreferencesMusicVolume1;

// 0x663998
static int gPreferencesRunning1;

// 0x66399C
static int gPreferencesCombatSpeed1;

// 0x6639A0
static int _plyrspdbid;

// 0x6639A4
static int gPreferencesItemHighlight1;

// 0x6639A8
static bool _changed;

static bool _graphics_changed;

static bool _widescreen_changed;

static bool _play_area_changed;

// 0x6639AC
static int gPreferencesCombatMessages1;

// 0x6639B0
static int gPreferencesTargetHighlight1;

// 0x6639B4
static int gPreferencesCombatDifficulty1;

// 0x6639B8
static int gPreferencesViolenceLevel1;

// 0x6639BC
static int gPreferencesGameDifficulty1;

// 0x6639C0
static int gPreferencesCombatLooks1;

static int gPreferencesFullscreen1;
static int gPreferencesFullscreen2;

static int gPreferencesHighQuality1;
static int gPreferencesHighQuality2;

static int gPreferencesPreserveAspect1;
static int gPreferencesPreserveAspect2;

static int gPreferencesSquarePixels1;
static int gPreferencesSquarePixels2;

static int gPreferencesStretchEnabled1;
static int gPreferencesStretchEnabled2;

static int gPreferencesWidescreen1;
static int gPreferencesWidescreen2;

static int gPreferencesPlayArea1;
static int gPreferencesPlayArea2;

// Added for offsets handling
static PreferencesOffsets gOffsets;

// 0x5197F8
static PreferenceDescription gPreferenceDescriptions[PREF_COUNT] = {
    { 3, 0, 76, 71, 0, 0, { 203, 204, 205, 0 }, 0, GAME_CONFIG_GAME_DIFFICULTY_KEY, 0, 0, &gPreferencesGameDifficulty1 },
    { 3, 0, 76, 149, 0, 0, { 206, 204, 208, 0 }, 0, GAME_CONFIG_COMBAT_DIFFICULTY_KEY, 0, 0, &gPreferencesCombatDifficulty1 },
    { 4, 0, 76, 226, 0, 0, { 214, 215, 204, 216 }, 0, GAME_CONFIG_VIOLENCE_LEVEL_KEY, 0, 0, &gPreferencesViolenceLevel1 },
    { 3, 0, 76, 309, 0, 0, { 202, 201, 213, 0 }, 0, GAME_CONFIG_TARGET_HIGHLIGHT_KEY, 0, 0, &gPreferencesTargetHighlight1 },
    { 2, 0, 76, 387, 0, 0, { 202, 201, 0, 0 }, 0, GAME_CONFIG_COMBAT_LOOKS_KEY, 0, 0, &gPreferencesCombatLooks1 },
    { 2, 0, 299, 74, 0, 0, { 211, 212, 0, 0 }, 0, GAME_CONFIG_COMBAT_MESSAGES_KEY, 0, 0, &gPreferencesCombatMessages1 },
    { 2, 0, 299, 141, 0, 0, { 202, 201, 0, 0 }, 0, GAME_CONFIG_COMBAT_TAUNTS_KEY, 0, 0, &gPreferencesCombatTaunts1 },
    { 2, 0, 299, 207, 0, 0, { 202, 201, 0, 0 }, 0, GAME_CONFIG_LANGUAGE_FILTER_KEY, 0, 0, &gPreferencesLanguageFilter1 },
    { 2, 0, 299, 271, 0, 0, { 209, 219, 0, 0 }, 0, GAME_CONFIG_RUNNING_KEY, 0, 0, &gPreferencesRunning1 },
    { 2, 0, 299, 338, 0, 0, { 202, 201, 0, 0 }, 0, GAME_CONFIG_SUBTITLES_KEY, 0, 0, &gPreferencesSubtitles1 },
    { 2, 0, 299, 404, 0, 0, { 202, 201, 0, 0 }, 0, GAME_CONFIG_ITEM_HIGHLIGHT_KEY, 0, 0, &gPreferencesItemHighlight1 },
    { 2, 0, 299, 74, 0, 0, { 225, 226, 0, 0 }, 0, GAME_CONFIG_FULLSCREEN, 0, 0, &gPreferencesFullscreen1 },
    { 2, 0, 299, 404, 0, 0, { 227, 228, 0, 0 }, 0, GAME_CONFIG_WIDESCREEN, 0, 0, &gPreferencesWidescreen1 },
    { 2, 0, 299, 338, 0, 0, { 229, 230, 0, 0 }, 0, GAME_CONFIG_STRETCH_ENABLED, 0, 0, &gPreferencesStretchEnabled1 },
    { 2, 0, 299, 207, 0, 0, { 231, 232, 0, 0 }, 0, GAME_CONFIG_PRESERVE_ASPECT, 0, 0, &gPreferencesPreserveAspect1 },
    { 2, 0, 299, 141, 0, 0, { 233, 234, 0, 0 }, 0, GAME_CONFIG_HIGH_QUALITY, 0, 0, &gPreferencesHighQuality1 },
    { 2, 0, 299, 271, 0, 0, { 235, 236, 0, 0 }, 0, GAME_CONFIG_SQUARE_PIXELS, 0, 0, &gPreferencesSquarePixels1 },
    { 4, 0, 299, 271, 0, 0, { 237, 238, 239, 240 }, 0, GAME_CONFIG_PLAY_AREA, 0, 0, &gPreferencesPlayArea1 },
    { 2, 0, 374, 50, 0, 0, { 207, 210, 0, 0 }, 0, GAME_CONFIG_COMBAT_SPEED_KEY, 0.0, 50.0, &gPreferencesCombatSpeed1 },
    { 3, 0, 374, 125, 0, 0, { 217, 209, 218, 0 }, 0, GAME_CONFIG_TEXT_BASE_DELAY_KEY, 1.0, 6.0, nullptr },
    { 4, 0, 374, 196, 0, 0, { 202, 221, 209, 222 }, 0, GAME_CONFIG_MASTER_VOLUME_KEY, 0, 32767.0, &gPreferencesMasterVolume1 },
    { 4, 0, 374, 247, 0, 0, { 202, 221, 209, 222 }, 0, GAME_CONFIG_MUSIC_VOLUME_KEY, 0, 32767.0, &gPreferencesMusicVolume1 },
    { 4, 0, 374, 298, 0, 0, { 202, 221, 209, 222 }, 0, GAME_CONFIG_SNDFX_VOLUME_KEY, 0, 32767.0, &gPreferencesSoundEffectsVolume1 },
    { 4, 0, 374, 349, 0, 0, { 202, 221, 209, 222 }, 0, GAME_CONFIG_SPEECH_VOLUME_KEY, 0, 32767.0, &gPreferencesSpeechVolume1 },
    { 2, 0, 374, 400, 0, 0, { 207, 223, 0, 0 }, 0, GAME_CONFIG_BRIGHTNESS_KEY, 1.0, 1.17999267578125, nullptr },
    { 2, 0, 374, 451, 0, 0, { 207, 218, 0, 0 }, 0, GAME_CONFIG_MOUSE_SENSITIVITY_KEY, 1.0, 2.5, nullptr },
};

static FrmImage _preferencesFrmImages[PREFERENCES_WINDOW_FRM_COUNT];
static int _oldFont;

bool preferencesLoadOffsetsFromConfig(PreferencesOffsets* offsets, bool isWidescreen)
{
    return loadOffsetsFromConfig<PreferencesOffsets>(
        offsets,
        isWidescreen,
        "preferences",
        gPreferencesOffsets640,
        gPreferencesOffsets800,
        applyConfigToPreferencesOffsets);
}

void preferencesWriteDefaultOffsetsToConfig(bool isWidescreen, const PreferencesOffsets* defaults)
{
    const char* section = isWidescreen ? "preferences800" : "preferences640";

    // Window dimensions
    configSetInt(&gGameConfig, section, "width", defaults->width);
    configSetInt(&gGameConfig, section, "height", defaults->height);

    // Primary preferences
    configSetInt(&gGameConfig, section, "primaryColumnX", defaults->primaryColumnX);
    configSetInt(&gGameConfig, section, "primaryKnobX", defaults->primaryKnobX);
    configSetIntArray(&gGameConfig, section, "primaryKnobY", defaults->primaryKnobY, PRIMARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "primaryLabelY", defaults->primaryLabelY, PRIMARY_PREF_COUNT);

    // Secondary preferences
    configSetInt(&gGameConfig, section, "secondaryColumnX", defaults->secondaryColumnX);
    configSetInt(&gGameConfig, section, "secondaryKnobX", defaults->secondaryKnobX);
    configSetIntArray(&gGameConfig, section, "secondaryKnobY", defaults->secondaryKnobY, SECONDARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "secondaryLabelY", defaults->secondaryLabelY, SECONDARY_PREF_COUNT);

    // Tertiary preferences
    configSetInt(&gGameConfig, section, "tertiaryColumnX", defaults->tertiaryColumnX);
    configSetInt(&gGameConfig, section, "tertiaryKnobX", defaults->tertiaryKnobX);
    configSetIntArray(&gGameConfig, section, "tertiaryKnobY", defaults->tertiaryKnobY, TERTIARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "tertiaryLabelY", defaults->tertiaryLabelY, TERTIARY_PREF_COUNT);

    // Range preferences
    configSetInt(&gGameConfig, section, "rangeColumnX", defaults->rangeColumnX);
    configSetInt(&gGameConfig, section, "rangeKnobX", defaults->rangeKnobX);
    configSetIntArray(&gGameConfig, section, "rangeKnobY", defaults->rangeKnobY, RANGE_PREF_COUNT);

    // Label positions
    configSetInt(&gGameConfig, section, "primLabelColX", defaults->primLabelColX);
    configSetInt(&gGameConfig, section, "secLabelColX", defaults->secLabelColX);
    configSetInt(&gGameConfig, section, "terLabelColX", defaults->terLabelColX);
    configSetInt(&gGameConfig, section, "rangLabelColX", defaults->rangLabelColX);
    configSetIntArray(&gGameConfig, section, "labelX", defaults->labelX, PRIMARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "secondaryLabelX", defaults->secondaryLabelX, SECONDARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "tertiaryLabelX", defaults->tertiaryLabelX, TERTIARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "quaternaryLabelX", defaults->quaternarylabelX, QUATERNARY_PREF_COUNT);

    // Range control
    configSetInt(&gGameConfig, section, "rangeStartX", defaults->rangeStartX);
    configSetInt(&gGameConfig, section, "rangeWidth", defaults->rangeWidth);
    configSetInt(&gGameConfig, section, "knobWidth", defaults->knobWidth);
    configSetIntArray(&gGameConfig, section, "rangeLabelX", defaults->rangeLabelX, 5); // Now 5 elements

    // Blit dimensions
    configSetInt(&gGameConfig, section, "primaryBlitWidth", defaults->primaryBlitWidth);
    configSetInt(&gGameConfig, section, "primaryBlitHeight", defaults->primaryBlitHeight);
    configSetInt(&gGameConfig, section, "secondaryBlitWidth", defaults->secondaryBlitWidth);
    configSetInt(&gGameConfig, section, "secondaryBlitHeight", defaults->secondaryBlitHeight);
    configSetInt(&gGameConfig, section, "tertiaryBlitWidth", defaults->tertiaryBlitWidth);
    configSetInt(&gGameConfig, section, "tertiaryBlitHeight", defaults->tertiaryBlitHeight);
    configSetInt(&gGameConfig, section, "quaternaryBlitWidth", defaults->quaternaryBlitWidth);
    configSetInt(&gGameConfig, section, "quaternaryBlitHeight", defaults->quaternaryBlitHeight);
    configSetInt(&gGameConfig, section, "rangeBlitWidth", defaults->rangeBlitWidth);
    configSetInt(&gGameConfig, section, "rangeBlitHeight", defaults->rangeBlitHeight);

    // Title and buttons
    configSetInt(&gGameConfig, section, "titleTextX", defaults->titleTextX);
    configSetInt(&gGameConfig, section, "titleTextY", defaults->titleTextY);
    configSetInt(&gGameConfig, section, "defaultLabelX", defaults->defaultLabelX);
    configSetInt(&gGameConfig, section, "defaultLabelY", defaults->defaultLabelY);
    configSetInt(&gGameConfig, section, "doneLabelX", defaults->doneLabelX);
    configSetInt(&gGameConfig, section, "doneLabelY", defaults->doneLabelY);
    configSetInt(&gGameConfig, section, "cancelLabelX", defaults->cancelLabelX);
    configSetInt(&gGameConfig, section, "cancelLabelY", defaults->cancelLabelY);
    configSetInt(&gGameConfig, section, "speedLabelX", defaults->speedLabelX);
    configSetInt(&gGameConfig, section, "speedLabelY", defaults->speedLabelY);

    configSetInt(&gGameConfig, section, "defaultButtonX", defaults->defaultButtonX);
    configSetInt(&gGameConfig, section, "defaultButtonY", defaults->defaultButtonY);
    configSetInt(&gGameConfig, section, "doneButtonX", defaults->doneButtonX);
    configSetInt(&gGameConfig, section, "doneButtonY", defaults->doneButtonY);
    configSetInt(&gGameConfig, section, "cancelButtonX", defaults->cancelButtonX);
    configSetInt(&gGameConfig, section, "cancelButtonY", defaults->cancelButtonY);
    configSetInt(&gGameConfig, section, "playerSpeedCheckboxX", defaults->playerSpeedCheckboxX);
    configSetInt(&gGameConfig, section, "playerSpeedCheckboxY", defaults->playerSpeedCheckboxY);

    // Hit detection
    configSetInt(&gGameConfig, section, "primaryKnobHitX", defaults->primaryKnobHitX);
    configSetInt(&gGameConfig, section, "primaryKnobHitY", defaults->primaryKnobHitY);
    configSetInt(&gGameConfig, section, "secondaryKnobHitX", defaults->secondaryKnobHitX);
    configSetInt(&gGameConfig, section, "secondaryKnobHitY", defaults->secondaryKnobHitY);
    configSetInt(&gGameConfig, section, "tertiaryKnobHitX", defaults->tertiaryKnobHitX);
    configSetInt(&gGameConfig, section, "tertiaryKnobHitY", defaults->tertiaryKnobHitY);
    configSetInt(&gGameConfig, section, "quaternaryKnobHitX", defaults->quaternaryKnobHitX);
    configSetInt(&gGameConfig, section, "quaternaryKnobHitY", defaults->quaternaryKnobHitY);
    configSetInt(&gGameConfig, section, "rangeSliderMinX", defaults->rangeSliderMinX);
    configSetInt(&gGameConfig, section, "rangeSliderMaxX", defaults->rangeSliderMaxX);
    configSetInt(&gGameConfig, section, "rangeSliderWidth", defaults->rangeSliderWidth);
    configSetInt(&gGameConfig, section, "primaryButtonOffsetY", defaults->primaryButtonOffsetY);
    configSetInt(&gGameConfig, section, "secondaryButtonOffsetY", defaults->secondaryButtonOffsetY);
    configSetInt(&gGameConfig, section, "tertiaryButtonOffsetY", defaults->tertiaryButtonOffsetY);
    configSetInt(&gGameConfig, section, "quaternaryButtonOffsetY", defaults->quaternaryButtonOffsetY);
    configSetInt(&gGameConfig, section, "rangeButtonOffsetY", defaults->rangeButtonOffsetY);

    // Text and range options
    configSetDouble(&gGameConfig, section, "textBaseDelayScale", defaults->textBaseDelayScale);
    configSetInt(&gGameConfig, section, "rangeLabel4Option1X", defaults->rangeLabel4Option1X);
    configSetInt(&gGameConfig, section, "rangeLabel4Option2X", defaults->rangeLabel4Option2X);

    // Position arrays
    configSetIntArray(&gGameConfig, section, "row1Ytab", defaults->row1Ytab, PRIMARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "row2Ytab", defaults->row2Ytab, SECONDARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "row2bYtab", defaults->row2bYtab, TERTIARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "rowdialYtab", defaults->rowdialYtab, QUATERNARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "row3Ytab", defaults->row3Ytab, RANGE_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "optionXOffsets", defaults->optionXOffsets, PRIMARY_OPTION_VALUE_COUNT);
    configSetIntArray(&gGameConfig, section, "optionYOffsets", defaults->optionYOffsets, PRIMARY_OPTION_VALUE_COUNT);
    configSetIntArray(&gGameConfig, section, "secondaryOptionXOffsets", defaults->secondaryOptionXOffsets, SECONDARY_OPTION_VALUE_COUNT);
    configSetIntArray(&gGameConfig, section, "tertiaryOptionYOffsets", defaults->tertiaryOptionYOffsets, TERTIARY_OPTION_VALUE_COUNT);
    configSetIntArray(&gGameConfig, section, "quaternaryXOffsets", defaults->quaternaryXOffsets, QUATERNARY_OPTION_VALUE_COUNT);
    configSetIntArray(&gGameConfig, section, "quaternaryYOffsets", defaults->quaternaryYOffsets, QUATERNARY_OPTION_VALUE_COUNT);
    configSetIntArray(&gGameConfig, section, "primaryLabelYValues", defaults->primaryLabelYValues, PRIMARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "secondaryLabelYValues", defaults->secondaryLabelYValues, SECONDARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "tertiaryLabelYValues", defaults->tertiaryLabelYValues, TERTIARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "quaternaryLabelYValues", defaults->quaternaryLabelYValues, QUATERNARY_PREF_COUNT);

    // Button and slider offsets
    configSetInt(&gGameConfig, section, "primaryButtonMinXOffset", defaults->primaryButtonMinXOffset);
    configSetInt(&gGameConfig, section, "primaryButtonMaxXOffset", defaults->primaryButtonMaxXOffset);
    configSetInt(&gGameConfig, section, "secondaryButtonXOffset", defaults->secondaryButtonXOffset);
    configSetInt(&gGameConfig, section, "tertiaryButtonXOffset", defaults->tertiaryButtonXOffset);
    configSetInt(&gGameConfig, section, "quaternaryButtonMinXOffset", defaults->quaternaryButtonMinXOffset);
    configSetInt(&gGameConfig, section, "quaternaryButtonMaxXOffset", defaults->quaternaryButtonMaxXOffset);
    configSetInt(&gGameConfig, section, "rangeThumbLeftOffset", defaults->rangeThumbLeftOffset);
    configSetInt(&gGameConfig, section, "rangeThumbRightOffset", defaults->rangeThumbRightOffset);
    configSetDouble(&gGameConfig, section, "rangeSliderScale", defaults->rangeSliderScale);

    // Save preference positions
    for (int i = 0; i < PREF_COUNT; i++) {
        char key[64];
        snprintf(key, sizeof(key), "preferencePositions%dX", i);
        configSetInt(&gGameConfig, section, key, defaults->preferencePositions[i].x);
        snprintf(key, sizeof(key), "preferencePositions%dY", i);
        configSetInt(&gGameConfig, section, key, defaults->preferencePositions[i].y);
    }
}

void applyPlayAreaResolution()
{
    if (GameMode::isInGameMode(GameMode::kPreferences)) { // only apply play area settings when in preference screen (otherwise get graphics bug)
        SDL_DisplayMode dm;
        int displayIndex = 0; // Primary display

        switch (gPreferencesPlayArea1) {
        case 0: // Default
            settings.graphics.game_width = 640;
            settings.graphics.game_height = 480;
            break;
        case 1: // Normal
            settings.graphics.game_width = 800;
            settings.graphics.game_height = 500;
            break;
        case 2: // Large - 70% of screen size
        case 3: // Massive - Full screen size
            if (SDL_GetCurrentDisplayMode(displayIndex, &dm) == 0) {
                float scale = (gPreferencesPlayArea1 == 2) ? 0.7f : 1.0f;

                // Calculate target dimensions while maintaining screen aspect ratio
                settings.graphics.game_width = (int)roundf(dm.w * scale);
                settings.graphics.game_height = (int)roundf(dm.h * scale);
            } else {
                // Fallback if SDL query fails
                settings.graphics.game_width = (gPreferencesPlayArea1 == 2) ? 1280 : 1920;
                settings.graphics.game_height = (gPreferencesPlayArea1 == 2) ? 720 : 1080;
            }
            break;
        default: // Fallback
            settings.graphics.game_width = 800;
            settings.graphics.game_height = 500;
            break;
        }
    }
}

int preferencesInit()
{
    for (int index = 0; index < 11; index++) {
        gPreferenceDescriptions[index].direction = 0;
    }

    // Check if we should write defaults
    int writeOffsets = 0;
    if (configGetInt(&gGameConfig, "debug", "write_offsets", &writeOffsets) && writeOffsets) {
        preferencesWriteDefaultOffsetsToConfig(false, &gPreferencesOffsets640);
        preferencesWriteDefaultOffsetsToConfig(true, &gPreferencesOffsets800);
        configSetInt(&gGameConfig, "debug", "write_offsets", 0);
        gameConfigSave();
    }

    // Load preferences from config
    if (!preferencesLoadOffsetsFromConfig(&gOffsets, gameIsWidescreen())) {
        gOffsets = gameIsWidescreen() ? gPreferencesOffsets800 : gPreferencesOffsets640;
    }

    _SetSystemPrefs();

    return 0;
}

// 0x492AA8
static void _SetSystemPrefs()
{
    preferencesSetDefaults(false);

    gPreferencesGameDifficulty1 = settings.preferences.game_difficulty;
    gPreferencesCombatDifficulty1 = settings.preferences.combat_difficulty;
    gPreferencesViolenceLevel1 = settings.preferences.violence_level;
    gPreferencesTargetHighlight1 = settings.preferences.target_highlight;
    gPreferencesCombatMessages1 = settings.preferences.combat_messages;

    gPreferencesCombatLooks1 = settings.preferences.combat_looks;
    gPreferencesCombatTaunts1 = settings.preferences.combat_taunts;
    gPreferencesLanguageFilter1 = settings.preferences.language_filter;
    gPreferencesRunning1 = settings.preferences.running;
    gPreferencesSubtitles1 = settings.preferences.subtitles;
    gPreferencesItemHighlight1 = settings.preferences.item_highlight;

    gPreferencesFullscreen1 = settings.graphics.fullscreen;
    gPreferencesHighQuality1 = settings.graphics.high_quality;
    gPreferencesPreserveAspect1 = settings.graphics.preserve_aspect;
    gPreferencesSquarePixels1 = settings.graphics.square_pixels;
    gPreferencesStretchEnabled1 = settings.graphics.stretch_enabled;
    gPreferencesWidescreen1 = settings.graphics.widescreen;

    gPreferencesPlayArea1 = settings.graphics.play_area;

    gPreferencesCombatSpeed1 = settings.preferences.combat_speed;
    gPreferencesTextBaseDelay1 = settings.preferences.text_base_delay;
    gPreferencesPlayerSpeedup1 = settings.preferences.player_speedup;
    gPreferencesMasterVolume1 = settings.sound.master_volume;
    gPreferencesMusicVolume1 = settings.sound.music_volume;
    gPreferencesSoundEffectsVolume1 = settings.sound.sndfx_volume;
    gPreferencesSpeechVolume1 = settings.sound.speech_volume;
    gPreferencesBrightness1 = settings.preferences.brightness;
    gPreferencesMouseSensitivity1 = settings.preferences.mouse_sensitivity;

    _JustUpdate_();
}

// 0x493054
static void _SaveSettings()
{
    gPreferencesGameDifficulty2 = gPreferencesGameDifficulty1;
    gPreferencesCombatDifficulty2 = gPreferencesCombatDifficulty1;
    gPreferencesViolenceLevel2 = gPreferencesViolenceLevel1;
    gPreferencesTargetHighlight2 = gPreferencesTargetHighlight1;
    gPreferencesCombatLooks2 = gPreferencesCombatLooks1;

    gPreferencesCombatMessages2 = gPreferencesCombatMessages1;
    gPreferencesCombatTaunts2 = gPreferencesCombatTaunts1;
    gPreferencesLanguageFilter2 = gPreferencesLanguageFilter1;
    gPreferencesRunning2 = gPreferencesRunning1;
    gPreferencesSubtitles2 = gPreferencesSubtitles1;
    gPreferencesItemHighlight2 = gPreferencesItemHighlight1;

    gPreferencesFullscreen2 = gPreferencesFullscreen1;
    gPreferencesHighQuality2 = gPreferencesHighQuality1;
    gPreferencesPreserveAspect2 = gPreferencesPreserveAspect1;
    gPreferencesSquarePixels2 = gPreferencesSquarePixels1;
    gPreferencesStretchEnabled2 = gPreferencesStretchEnabled1;
    gPreferencesWidescreen2 = gPreferencesWidescreen1;

    gPreferencesPlayArea2 = gPreferencesPlayArea1;

    gPreferencesCombatSpeed2 = gPreferencesCombatSpeed1;
    gPreferencesPlayerSpeedup2 = gPreferencesPlayerSpeedup1;
    gPreferencesMasterVolume2 = gPreferencesMasterVolume1;
    gPreferencesTextBaseDelay2 = gPreferencesTextBaseDelay1;
    gPreferencesMusicVolume2 = gPreferencesMusicVolume1;
    gPreferencesBrightness2 = gPreferencesBrightness1;
    gPreferencesSoundEffectsVolume2 = gPreferencesSoundEffectsVolume1;
    gPreferencesMouseSensitivity2 = gPreferencesMouseSensitivity1;
    gPreferencesSpeechVolume2 = gPreferencesSpeechVolume1;
}

// 0x493128
static void _RestoreSettings()
{
    gPreferencesGameDifficulty1 = gPreferencesGameDifficulty2;
    gPreferencesCombatDifficulty1 = gPreferencesCombatDifficulty2;
    gPreferencesViolenceLevel1 = gPreferencesViolenceLevel2;
    gPreferencesTargetHighlight1 = gPreferencesTargetHighlight2;
    gPreferencesCombatLooks1 = gPreferencesCombatLooks2;

    gPreferencesCombatMessages1 = gPreferencesCombatMessages2;
    gPreferencesCombatTaunts1 = gPreferencesCombatTaunts2;
    gPreferencesLanguageFilter1 = gPreferencesLanguageFilter2;
    gPreferencesRunning1 = gPreferencesRunning2;
    gPreferencesSubtitles1 = gPreferencesSubtitles2;
    gPreferencesItemHighlight1 = gPreferencesItemHighlight2;

    gPreferencesFullscreen1 = gPreferencesFullscreen2;
    gPreferencesHighQuality1 = gPreferencesHighQuality2;
    gPreferencesPreserveAspect1 = gPreferencesPreserveAspect2;
    gPreferencesSquarePixels1 = gPreferencesSquarePixels2;
    gPreferencesStretchEnabled1 = gPreferencesStretchEnabled2;
    gPreferencesWidescreen1 = gPreferencesWidescreen2;

    gPreferencesPlayArea1 = gPreferencesPlayArea2;

    gPreferencesCombatSpeed1 = gPreferencesCombatSpeed2;
    gPreferencesPlayerSpeedup1 = gPreferencesPlayerSpeedup2;
    gPreferencesMasterVolume1 = gPreferencesMasterVolume2;
    gPreferencesTextBaseDelay1 = gPreferencesTextBaseDelay2;
    gPreferencesMusicVolume1 = gPreferencesMusicVolume2;
    gPreferencesBrightness1 = gPreferencesBrightness2;
    gPreferencesSoundEffectsVolume1 = gPreferencesSoundEffectsVolume2;
    gPreferencesMouseSensitivity1 = gPreferencesMouseSensitivity2;
    gPreferencesSpeechVolume1 = gPreferencesSpeechVolume2;

    _JustUpdate_();
}

// 0x492F60
static void preferencesSetDefaults(bool a1)
{
    gPreferencesGameDifficulty1 = 1;
    gPreferencesCombatDifficulty1 = COMBAT_DIFFICULTY_NORMAL;
    gPreferencesViolenceLevel1 = VIOLENCE_LEVEL_MAXIMUM_BLOOD;
    gPreferencesTargetHighlight1 = TARGET_HIGHLIGHT_TARGETING_ONLY;
    gPreferencesCombatLooks1 = 0;

    gPreferencesCombatMessages1 = 1;
    gPreferencesCombatTaunts1 = 1;
    gPreferencesLanguageFilter1 = 0;
    gPreferencesRunning1 = 0;
    gPreferencesSubtitles1 = 0;
    gPreferencesItemHighlight1 = 1;

    gPreferencesFullscreen1 = 1;
    gPreferencesHighQuality1 = 0;
    gPreferencesPreserveAspect1 = 1;
    gPreferencesSquarePixels1 = 0;
    gPreferencesStretchEnabled1 = 1;
    gPreferencesWidescreen1 = 1;

    gPreferencesPlayArea1 = 1;

    gPreferencesCombatSpeed1 = 0;
    gPreferencesPlayerSpeedup1 = 0;
    gPreferencesTextBaseDelay1 = 3.5;
    gPreferencesBrightness1 = 1.0;
    gPreferencesMouseSensitivity1 = 1.0;
    gPreferencesMasterVolume1 = 22281;
    gPreferencesMusicVolume1 = 22281;
    gPreferencesSoundEffectsVolume1 = 22281;
    gPreferencesSpeechVolume1 = 22281;

    if (a1) {
        for (int index = 0; index < PREF_COUNT; index++) {
            _UpdateThing(index);
        }
        _win_set_button_rest_state(_plyrspdbid, gPreferencesPlayerSpeedup1, 0);
        windowRefresh(gPreferencesWindow);
        _changed = true;
    }
}

// 0x4931F8
static void _JustUpdate_()
{
    gPreferencesGameDifficulty1 = std::clamp(gPreferencesGameDifficulty1, 0, 2);
    gPreferencesCombatDifficulty1 = std::clamp(gPreferencesCombatDifficulty1, 0, 2);
    gPreferencesViolenceLevel1 = std::clamp(gPreferencesViolenceLevel1, 0, 3);
    gPreferencesTargetHighlight1 = std::clamp(gPreferencesTargetHighlight1, 0, 2);
    gPreferencesCombatMessages1 = std::clamp(gPreferencesCombatMessages1, 0, 1);
    gPreferencesCombatLooks1 = std::clamp(gPreferencesCombatLooks1, 0, 1);
    gPreferencesCombatTaunts1 = std::clamp(gPreferencesCombatTaunts1, 0, 1);
    gPreferencesLanguageFilter1 = std::clamp(gPreferencesLanguageFilter1, 0, 1);
    gPreferencesRunning1 = std::clamp(gPreferencesRunning1, 0, 1);
    gPreferencesSubtitles1 = std::clamp(gPreferencesSubtitles1, 0, 1);
    gPreferencesItemHighlight1 = std::clamp(gPreferencesItemHighlight1, 0, 1);

    gPreferencesFullscreen1 = std::clamp(gPreferencesFullscreen1, 0, 1);
    gPreferencesHighQuality1 = std::clamp(gPreferencesHighQuality1, 0, 1);
    gPreferencesPreserveAspect1 = std::clamp(gPreferencesPreserveAspect1, 0, 1);
    gPreferencesSquarePixels1 = std::clamp(gPreferencesSquarePixels1, 0, 1);
    gPreferencesStretchEnabled1 = std::clamp(gPreferencesStretchEnabled1, 0, 1);
    gPreferencesWidescreen1 = std::clamp(gPreferencesWidescreen1, 0, 1);

    gPreferencesPlayArea1 = std::clamp(gPreferencesPlayArea1, 0, 3);

    gPreferencesCombatSpeed1 = std::clamp(gPreferencesCombatSpeed1, 0, 50);
    gPreferencesPlayerSpeedup1 = std::clamp(gPreferencesPlayerSpeedup1, 0, 1);
    gPreferencesTextBaseDelay1 = std::clamp(gPreferencesTextBaseDelay1, 1.0, 6.0); // fixed for proper save/restore
    gPreferencesMasterVolume1 = std::clamp(gPreferencesMasterVolume1, 0, VOLUME_MAX);
    gPreferencesMusicVolume1 = std::clamp(gPreferencesMusicVolume1, 0, VOLUME_MAX);
    gPreferencesSoundEffectsVolume1 = std::clamp(gPreferencesSoundEffectsVolume1, 0, VOLUME_MAX);
    gPreferencesSpeechVolume1 = std::clamp(gPreferencesSpeechVolume1, 0, VOLUME_MAX);
    gPreferencesBrightness1 = std::clamp(gPreferencesBrightness1, 1.0, 1.17999267578125);
    gPreferencesMouseSensitivity1 = std::clamp(gPreferencesMouseSensitivity1, 1.0, 2.5);

    textObjectsSetBaseDelay(gPreferencesTextBaseDelay1);
    gameMouseLoadItemHighlight();

    double textLineDelay = (gPreferencesTextBaseDelay1 + (-1.0)) * 0.2 * 2.0;
    textLineDelay = std::clamp(textLineDelay, 0.0, 2.0);

    textObjectsSetLineDelay(textLineDelay);
    aiMessageListReloadIfNeeded();
    _scr_message_free();
    gameSoundSetMasterVolume(gPreferencesMasterVolume1);
    backgroundSoundSetVolume(gPreferencesMusicVolume1);
    soundEffectsSetVolume(gPreferencesSoundEffectsVolume1);
    speechSetVolume(gPreferencesSpeechVolume1);
    mouseSetSensitivity(gPreferencesMouseSensitivity1);
    colorSetBrightness(gPreferencesBrightness1);
    applyPlayAreaResolution();
}

// for testing background blitting location
void fillRectWithColor(unsigned char* buffer, int pitch, int x, int y, int width, int height, unsigned char color)
{
    for (int j = 0; j < height; j++) {
        memset(buffer + pitch * (y + j) + x, color, width);
    }
}

// 0x491A68
static void _UpdateThing(int index)
{
    fontSetCurrent(101);

    PreferenceDescription* meta = &(gPreferenceDescriptions[index]);
    int pitch = gOffsets.width; // Use offset width for pitch

    // Get position from offsets struct instead of meta
    Point pos = gOffsets.preferencePositions[index];
    int knobX = pos.x;
    int knobY = pos.y;

    if (index >= FIRST_PRIMARY_PREF && index <= LAST_PRIMARY_PREF) {
        int primaryOptionIndex = index - FIRST_PRIMARY_PREF;

        int localOffsets[PRIMARY_PREF_COUNT];
        memcpy(localOffsets, gOffsets.primaryLabelYValues, sizeof(localOffsets));

        blitBufferToBuffer(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_BACKGROUND].getData() + pitch * localOffsets[primaryOptionIndex] + gOffsets.labelX[0],
            gOffsets.primaryBlitWidth,
            gOffsets.primaryBlitHeight,
            pitch,
            gPreferencesWindowBuffer + pitch * localOffsets[primaryOptionIndex] + gOffsets.labelX[0],
            pitch);

        for (int valueIndex = 0; valueIndex < meta->valuesCount; valueIndex++) {
            const char* text = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, meta->labelIds[valueIndex]);

            char copy[100]; // TODO: Size is probably wrong.
            strcpy(copy, text);

            // Use knobX from offsets instead of meta->knobX
            int x = knobX + gOffsets.optionXOffsets[valueIndex];
            int len = fontGetStringWidth(copy);
            switch (valueIndex) {
            case 0:
                x -= fontGetStringWidth(copy);
                meta->minX = x;
                break;
            case 1:
                x -= len / 2;
                meta->maxX = x + len;
                break;
            case 2:
            case 3:
                meta->maxX = x + len;
                break;
            }

            char* p = copy;
            while (*p != '\0' && *p != ' ') {
                p++;
            }

            // Use knobY from offsets instead of meta->knobY
            int y = knobY + gOffsets.optionYOffsets[valueIndex];
            const char* s;
            if (*p != '\0') {
                *p = '\0';
                fontDrawText(gPreferencesWindowBuffer + pitch * y + x, copy, pitch, pitch, _colorTable[18979]);
                s = p + 1;
                y += fontGetLineHeight();
            } else {
                s = copy;
            }

            fontDrawText(gPreferencesWindowBuffer + pitch * y + x, s, pitch, pitch, _colorTable[18979]);
        }

        int value = *(meta->valuePtr);
        // Use knobX/Y from offsets instead of meta
        blitBufferToBufferTrans(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_PRIMARY_SWITCH].getData() + (46 * 47) * value,
            46, 47, 46,
            gPreferencesWindowBuffer + pitch * knobY + knobX,
            pitch);
    } else if (index >= FIRST_SECONDARY_PREF && index <= LAST_SECONDARY_PREF) {
        int secondaryOptionIndex = index - FIRST_SECONDARY_PREF;

        int localOffsets[SECONDARY_PREF_COUNT];
        memcpy(localOffsets, gOffsets.secondaryLabelYValues, sizeof(localOffsets));

        blitBufferToBuffer(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_BACKGROUND].getData() + pitch * localOffsets[secondaryOptionIndex] + gOffsets.secondaryLabelX[0],
            gOffsets.secondaryBlitWidth,
            gOffsets.secondaryBlitHeight,
            pitch,
            gPreferencesWindowBuffer + pitch * localOffsets[secondaryOptionIndex] + gOffsets.secondaryLabelX[0],
            pitch);

        // Secondary options are booleans, so it's index is also it's value.
        for (int value = 0; value < 2; value++) {
            const char* text = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, meta->labelIds[value]);

            int x;
            if (value) {
                // Use knobX from offsets instead of meta->knobX
                x = knobX + gOffsets.secondaryOptionXOffsets[value];
                meta->maxX = x + fontGetStringWidth(text);
            } else {
                // Use knobX from offsets instead of meta->knobX
                x = knobX + gOffsets.secondaryOptionXOffsets[value] - fontGetStringWidth(text);
                meta->minX = x;
            }
            // Use knobY from offsets instead of meta->knobY
            fontDrawText(gPreferencesWindowBuffer + pitch * (knobY - 5) + x, text, pitch, pitch, _colorTable[18979]);
        }

        int value = *(meta->valuePtr);
        if (index == PREF_COMBAT_MESSAGES) {
            value ^= 1;
        }
        // Use knobX/Y from offsets instead of meta
        blitBufferToBufferTrans(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_SECONDARY_SWITCH].getData() + (22 * 25) * value,
            22, 25, 22,
            gPreferencesWindowBuffer + pitch * knobY + knobX,
            pitch);
    } else if ((index >= FIRST_TERTIARY_PREF && index <= LAST_TERTIARY_PREF) && gameIsWidescreen()) {
        int tertiaryOptionIndex = index - FIRST_TERTIARY_PREF;
        int localOffsets[TERTIARY_PREF_COUNT];
        memcpy(localOffsets, gOffsets.tertiaryLabelYValues, sizeof(localOffsets));

        // Use this to match the original blit area
        /*int x = gOffsets.tertiaryLabelX[tertiaryOptionIndex];
        int y = localOffsets[tertiaryOptionIndex]; // this could use KnobY - to lock the background blit to the button
        int width = gOffsets.tertiaryBlitWidth;
        int height = gOffsets.tertiaryBlitHeight;
        unsigned char color = 231; // Bright test color, adjust as needed*/

        blitBufferToBuffer(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_BACKGROUND].getData() + pitch * /*knobY*/ localOffsets[tertiaryOptionIndex] + gOffsets.tertiaryLabelX[tertiaryOptionIndex],
            gOffsets.tertiaryBlitWidth,
            gOffsets.tertiaryBlitHeight,
            pitch,
            gPreferencesWindowBuffer + pitch * /*knobY*/ localOffsets[tertiaryOptionIndex] + gOffsets.tertiaryLabelX[tertiaryOptionIndex],
            pitch);

        // fillRectWithColor(gPreferencesWindowBuffer, pitch, x, y, width, height, color);

        // Tertiary options are booleans, so it's index is also it's value.
        for (int value = 0; value < 2; value++) {
            const char* text = getmsg(&gFissionMessageList, &gFissionMessageListItem, meta->labelIds[value]);

            char copy[100]; // TODO: Verify buffer size is sufficient
            strcpy(copy, text);

            // Position calculation for hit detection
            int x;
            if (value) {
                // Use knobX from offsets instead of meta->knobX
                x = knobX + 23;
                meta->maxX = x + fontGetStringWidth(text);
            } else {
                // Use knobX from offsets instead of meta->knobX
                x = knobX - 23;
                meta->minX = x;
            }

            // Get centering area
            int centerAreaX = gOffsets.tertiaryLabelX[tertiaryOptionIndex];
            int centerAreaWidth = gOffsets.tertiaryBlitWidth;

            // Vertical positioning
            int y = knobY - gOffsets.tertiaryOptionYOffsets[value];

            // Multi-line handling
            char* p = copy;
            while (*p != '\0' && *p != ' ') {
                p++;
            }

            // Handle multi-line text
            if (*p != '\0') {
                // Split into two lines at first space
                *p = '\0';

                // First line
                int textWidth1 = fontGetStringWidth(copy);
                int centeredX1 = centerAreaX + (centerAreaWidth - textWidth1) / 2 + 4;
                fontDrawText(
                    gPreferencesWindowBuffer + pitch * y + centeredX1,
                    copy,
                    pitch,
                    pitch,
                    _colorTable[18979]);

                // Second line
                const char* secondLine = p + 1;
                int textWidth2 = fontGetStringWidth(secondLine);
                int centeredX2 = centerAreaX + (centerAreaWidth - textWidth2) / 2 + 4;
                y += fontGetLineHeight(); // Move down for second line

                fontDrawText(
                    gPreferencesWindowBuffer + pitch * y + centeredX2,
                    secondLine,
                    pitch,
                    pitch,
                    _colorTable[18979]);
            } else {
                // Single line text
                int textWidth = fontGetStringWidth(copy);
                int centeredX = centerAreaX + (centerAreaWidth - textWidth) / 2 + 4;

                fontDrawText(
                    gPreferencesWindowBuffer + pitch * y + centeredX,
                    copy,
                    pitch,
                    pitch,
                    _colorTable[18979]);
            }
        }

        int value = *(meta->valuePtr);
        int switchValue = 1 - value; // reverse orientation of switches, so up is 'on'
        // Use switchX/Y from offsets instead of meta
        blitBufferToBufferTrans(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_TOGGLE_BUTTON_UP].getData() + (_preferencesFrmImages[PREFERENCES_WINDOW_FRM_TOGGLE_BUTTON_UP].getWidth() * (_preferencesFrmImages[PREFERENCES_WINDOW_FRM_TOGGLE_BUTTON_UP].getHeight() / 2)) * switchValue,
            _preferencesFrmImages[PREFERENCES_WINDOW_FRM_TOGGLE_BUTTON_UP].getWidth(),
            _preferencesFrmImages[PREFERENCES_WINDOW_FRM_TOGGLE_BUTTON_UP].getHeight() / 2,
            _preferencesFrmImages[PREFERENCES_WINDOW_FRM_TOGGLE_BUTTON_UP].getWidth(),
            gPreferencesWindowBuffer + pitch * knobY + knobX,
            pitch);

    } else if (index == PREF_PLAY_AREA && gameIsWidescreen()) { // Single Play Area Dial - background blit and button blit
        int quaternaryOptionIndex = index - FIRST_QUATERNARY_PREF;

        int localOffsets[QUATERNARY_PREF_COUNT];
        memcpy(localOffsets, gOffsets.quaternaryLabelYValues, sizeof(localOffsets));

        // Use this to match the original blit area
        /*int x = gOffsets.quaternarylabelX[0];
        int y = localOffsets[quaternaryOptionIndex]; // this could use KnobY - to lock the background blit to the button
        int width = gOffsets.quaternaryBlitWidth;
        int height = gOffsets.quaternaryBlitHeight;
        unsigned char color = 231; // Bright test color, adjust as needed*/

        blitBufferToBuffer(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_BACKGROUND].getData() + pitch * /*knobY*/ localOffsets[quaternaryOptionIndex] + gOffsets.quaternarylabelX[0],
            gOffsets.quaternaryBlitWidth,
            gOffsets.quaternaryBlitHeight,
            pitch,
            gPreferencesWindowBuffer + pitch * /*knobY*/ localOffsets[quaternaryOptionIndex] + gOffsets.quaternarylabelX[0],
            pitch);

        // fillRectWithColor(gPreferencesWindowBuffer, pitch, x, y, width, height, color);

        for (int valueIndex = 0; valueIndex < meta->valuesCount; valueIndex++) {
            const char* text = getmsg(&gFissionMessageList, &gFissionMessageListItem, meta->labelIds[valueIndex]);

            char copy[100]; // TODO: Size is probably wrong.
            strcpy(copy, text);

            // Use knobX from offsets instead of meta->knobX
            int x = knobX + gOffsets.quaternaryXOffsets[valueIndex];
            int len = fontGetStringWidth(copy);
            switch (valueIndex) {
            case 0:
                x -= fontGetStringWidth(copy);
                meta->minX = x;
                break;
            case 1:
                x -= len / 2;
                meta->maxX = x + len;
                break;
            case 2:
            case 3:
                meta->maxX = x + len;
                break;
            }

            char* p = copy;
            while (*p != '\0' && *p != ' ') {
                p++;
            }

            // Use knobY from offsets instead of meta->knobY
            int y = knobY + gOffsets.quaternaryYOffsets[valueIndex];
            const char* s;
            if (*p != '\0') {
                *p = '\0';
                fontDrawText(gPreferencesWindowBuffer + pitch * y + x, copy, pitch, pitch, _colorTable[18979]);
                s = p + 1;
                y += fontGetLineHeight();
            } else {
                s = copy;
            }

            fontDrawText(gPreferencesWindowBuffer + pitch * y + x, s, pitch, pitch, _colorTable[18979]);
        }

        int value = *(meta->valuePtr);
        // Use knobX/Y from offsets instead of meta
        blitBufferToBufferTrans(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_DIAL].getData() + (54 * 56) * value,
            54, 56, 54,
            gPreferencesWindowBuffer + pitch * knobY + knobX,
            pitch);
    } else if (index >= FIRST_RANGE_PREF && index <= LAST_RANGE_PREF) {
        // Use knobY from offsets instead of meta->knobY
        int yPos = knobY + gOffsets.rangeButtonOffsetY;
        blitBufferToBuffer(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_BACKGROUND].getData() + pitch * yPos + gOffsets.rangeStartX,
            gOffsets.rangeBlitWidth,
            gOffsets.rangeBlitHeight,
            pitch,
            gPreferencesWindowBuffer + pitch * yPos + gOffsets.rangeStartX,
            pitch);

        switch (index) {
        case PREF_COMBAT_SPEED: {
            double value = *meta->valuePtr;
            value = std::clamp(value, 0.0, 50.0);
            int x = (int)((value - meta->minValue) * gOffsets.rangeSliderWidth / (meta->maxValue - meta->minValue) + gOffsets.rangeStartX);
            // Use knobY from offsets instead of meta->knobY
            blitBufferToBufferTrans(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_KNOB_OFF].getData(),
                21, 12, 21,
                gPreferencesWindowBuffer + pitch * knobY + x,
                pitch);
            break;
        }
        case PREF_TEXT_BASE_DELAY: {
            gPreferencesTextBaseDelay1 = std::clamp(gPreferencesTextBaseDelay1, 1.0, 6.0);
            int x = (int)((6.0 - gPreferencesTextBaseDelay1) * gOffsets.textBaseDelayScale + gOffsets.rangeStartX);
            // Use knobY from offsets instead of meta->knobY
            blitBufferToBufferTrans(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_KNOB_OFF].getData(),
                21, 12, 21,
                gPreferencesWindowBuffer + pitch * knobY + x,
                pitch);

            double value = (gPreferencesTextBaseDelay1 - 1.0) * 0.2 * 2.0;
            value = std::clamp(value, 0.0, 2.0);
            textObjectsSetBaseDelay(gPreferencesTextBaseDelay1);
            textObjectsSetLineDelay(value);
            break;
        }
        case PREF_MASTER_VOLUME:
        case PREF_MUSIC_VOLUME:
        case PREF_SFX_VOLUME:
        case PREF_SPEECH_VOLUME: {
            double value = *meta->valuePtr;
            value = std::clamp(value, meta->minValue, meta->maxValue);
            int x = (int)((value - meta->minValue) * gOffsets.rangeSliderWidth / (meta->maxValue - meta->minValue) + gOffsets.rangeStartX);
            // Use knobY from offsets instead of meta->knobY
            blitBufferToBufferTrans(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_KNOB_OFF].getData(),
                21, 12, 21,
                gPreferencesWindowBuffer + pitch * knobY + x,
                pitch);

            switch (index) {
            case PREF_MASTER_VOLUME:
                gameSoundSetMasterVolume(gPreferencesMasterVolume1);
                break;
            case PREF_MUSIC_VOLUME:
                backgroundSoundSetVolume(gPreferencesMusicVolume1);
                break;
            case PREF_SFX_VOLUME:
                soundEffectsSetVolume(gPreferencesSoundEffectsVolume1);
                break;
            case PREF_SPEECH_VOLUME:
                speechSetVolume(gPreferencesSpeechVolume1);
                break;
            }
            break;
        }
        case PREF_BRIGHTNESS: {
            gPreferencesBrightness1 = std::clamp(gPreferencesBrightness1, 1.0, 1.17999267578125);
            int x = (int)((gPreferencesBrightness1 - meta->minValue) * (gOffsets.rangeSliderWidth / (meta->maxValue - meta->minValue)) + gOffsets.rangeStartX);
            // Use knobY from offsets instead of meta->knobY
            blitBufferToBufferTrans(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_KNOB_OFF].getData(),
                21, 12, 21,
                gPreferencesWindowBuffer + pitch * knobY + x,
                pitch);
            colorSetBrightness(gPreferencesBrightness1);
            break;
        }
        case PREF_MOUSE_SENSITIVIY: {
            gPreferencesMouseSensitivity1 = std::clamp(gPreferencesMouseSensitivity1, 1.0, 2.5);
            int x = (int)((gPreferencesMouseSensitivity1 - meta->minValue) * (gOffsets.rangeSliderWidth / (meta->maxValue - meta->minValue)) + gOffsets.rangeStartX);
            // Use knobY from offsets instead of meta->knobY
            blitBufferToBufferTrans(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_KNOB_OFF].getData(),
                21, 12, 21,
                gPreferencesWindowBuffer + pitch * knobY + x,
                pitch);
            mouseSetSensitivity(gPreferencesMouseSensitivity1);
            break;
        }
        }

        for (int optionIndex = 0; optionIndex < meta->valuesCount; optionIndex++) {
            const char* str = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, meta->labelIds[optionIndex]);

            int x;
            switch (optionIndex) {
            case 0:
                x = gOffsets.rangeLabelX[0];
                // TODO: Incomplete.
                break;
            case 1:
                switch (meta->valuesCount) {
                case 2:
                    x = gOffsets.rangeLabelX[3] - fontGetStringWidth(str);
                    break;
                case 3:
                    x = gOffsets.rangeLabelX[1] - fontGetStringWidth(str) / 2 - 2;
                    break;
                case 4:
                    x = gOffsets.rangeLabelX[4] + fontGetStringWidth(str) / 2 - 8;
                    break;
                }
                break;
            case 2:
                switch (meta->valuesCount) {
                case 3:
                    x = gOffsets.rangeLabelX[3] - fontGetStringWidth(str);
                    break;
                case 4:
                    x = gOffsets.rangeLabelX[2] - fontGetStringWidth(str) - 4;
                    break;
                }
                break;
            case 3:
                x = gOffsets.rangeLabelX[3] - fontGetStringWidth(str);
                break;
            }
            // Use knobY from offsets instead of meta->knobY
            fontDrawText(gPreferencesWindowBuffer + pitch * (knobY - 12) + x, str, pitch, pitch, _colorTable[18979]);
        }
    } else {
        // return false;
    }

    // TODO: Incomplete.

    // return true;
}

// 0x492CB0
int _SavePrefs(bool save)
{
    settings.preferences.game_difficulty = gPreferencesGameDifficulty1;
    settings.preferences.combat_difficulty = gPreferencesCombatDifficulty1;
    settings.preferences.violence_level = gPreferencesViolenceLevel1;
    settings.preferences.target_highlight = gPreferencesTargetHighlight1;
    settings.preferences.combat_messages = gPreferencesCombatMessages1;
    settings.preferences.combat_looks = gPreferencesCombatLooks1;
    settings.preferences.combat_taunts = gPreferencesCombatTaunts1;
    settings.preferences.language_filter = gPreferencesLanguageFilter1;
    settings.preferences.running = gPreferencesRunning1;
    settings.preferences.subtitles = gPreferencesSubtitles1;
    settings.preferences.item_highlight = gPreferencesItemHighlight1;

    settings.graphics.fullscreen = gPreferencesFullscreen1;
    settings.graphics.high_quality = gPreferencesHighQuality1;
    settings.graphics.preserve_aspect = gPreferencesPreserveAspect1;
    settings.graphics.square_pixels = gPreferencesSquarePixels1;
    settings.graphics.stretch_enabled = gPreferencesStretchEnabled1;
    settings.graphics.widescreen = gPreferencesWidescreen1;

    settings.graphics.play_area = gPreferencesPlayArea1;

    settings.preferences.combat_speed = gPreferencesCombatSpeed1;
    settings.preferences.text_base_delay = gPreferencesTextBaseDelay1;

    double textLineDelay = (gPreferencesTextBaseDelay1 + dbl_50C2D0) * dbl_50C2D8 * dbl_50C2E0;
    if (textLineDelay >= 0.0) {
        if (textLineDelay > dbl_50C2E0) {
            textLineDelay = 2.0;
        }

        settings.preferences.text_line_delay = textLineDelay;
    } else {
        settings.preferences.text_line_delay = 0.0;
    }

    settings.preferences.player_speedup = gPreferencesPlayerSpeedup1;
    settings.sound.master_volume = gPreferencesMasterVolume1;
    settings.sound.music_volume = gPreferencesMusicVolume1;
    settings.sound.sndfx_volume = gPreferencesSoundEffectsVolume1;
    settings.sound.speech_volume = gPreferencesSpeechVolume1;

    settings.preferences.brightness = gPreferencesBrightness1;
    settings.preferences.mouse_sensitivity = gPreferencesMouseSensitivity1;

    if (save) {
        settingsSave();
    }

    return 0;
}

// 0x493224
// Writes preferences to a save game
int preferencesSave(File* stream)
{
    float textBaseDelay = (float)gPreferencesTextBaseDelay1;
    float brightness = (float)gPreferencesBrightness1;
    float mouseSensitivity = (float)gPreferencesMouseSensitivity1;

    if (fileWriteInt32(stream, gPreferencesGameDifficulty1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesCombatDifficulty1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesViolenceLevel1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesTargetHighlight1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesCombatLooks1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesCombatMessages1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesCombatTaunts1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesLanguageFilter1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesRunning1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesSubtitles1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesItemHighlight1) == -1)
        goto err;
    // Omitted to preserve save game compatibility
    /*if (fileWriteInt32(stream, gPreferencesFullscreen1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesHighQuality1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesPreserveAspect1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesSquarePixels1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesStretchEnabled1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesWidescreen1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesPlayArea1) == -1)
        goto err;*/
    if (fileWriteInt32(stream, gPreferencesCombatSpeed1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesPlayerSpeedup1) == -1)
        goto err;
    if (fileWriteFloat(stream, textBaseDelay) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesMasterVolume1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesMusicVolume1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesSoundEffectsVolume1) == -1)
        goto err;
    if (fileWriteInt32(stream, gPreferencesSpeechVolume1) == -1)
        goto err;
    if (fileWriteFloat(stream, brightness) == -1)
        goto err;
    if (fileWriteFloat(stream, mouseSensitivity) == -1)
        goto err;

    return 0;

err:

    debugPrint("\nOPTION MENU: Error save option data!\n");

    return -1;
}

// 0x49340C
// Loads preference settings from a save game
int preferencesLoad(File* stream)
{
    float textBaseDelay;
    float brightness;
    float mouseSensitivity;

    preferencesSetDefaults(false);

    if (fileReadInt32(stream, &gPreferencesGameDifficulty1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesCombatDifficulty1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesViolenceLevel1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesTargetHighlight1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesCombatLooks1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesCombatMessages1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesCombatTaunts1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesLanguageFilter1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesRunning1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesSubtitles1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesItemHighlight1) == -1)
        goto err;
    // Omitted to preserve save game compatibility
    /*if (fileReadInt32(stream, &gPreferencesFullscreen1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesHighQuality1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesPreserveAspect1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesSquarePixels1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesStretchEnabled1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesWidescreen1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesPlayArea1) == -1)
        goto err;*/
    if (fileReadInt32(stream, &gPreferencesCombatSpeed1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesPlayerSpeedup1) == -1)
        goto err;
    if (fileReadFloat(stream, &textBaseDelay) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesMasterVolume1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesMusicVolume1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesSoundEffectsVolume1) == -1)
        goto err;
    if (fileReadInt32(stream, &gPreferencesSpeechVolume1) == -1)
        goto err;
    if (fileReadFloat(stream, &brightness) == -1)
        goto err;
    if (fileReadFloat(stream, &mouseSensitivity) == -1)
        goto err;

    gPreferencesBrightness1 = brightness;
    gPreferencesMouseSensitivity1 = mouseSensitivity;
    gPreferencesTextBaseDelay1 = textBaseDelay;

    // Reset settings when loading a save - otherwise loads defaults (which haven't been written yet)
    // Could fix by writting to config from options screen immediately?
    // Game originally didn't expext any of these settings to be changed in game
    // To make things work OG way, would need to save settings into save game
    // This would make save games incompatible with old games
    gPreferencesFullscreen1 = settings.graphics.fullscreen;
    gPreferencesHighQuality1 = settings.graphics.high_quality;
    gPreferencesPreserveAspect1 = settings.graphics.preserve_aspect;
    gPreferencesSquarePixels1 = settings.graphics.square_pixels;
    gPreferencesStretchEnabled1 = settings.graphics.stretch_enabled;
    gPreferencesWidescreen1 = settings.graphics.widescreen;
    gPreferencesPlayArea1 = settings.graphics.play_area;

    _JustUpdate_();
    _SavePrefs(0);

    return 0;

err:

    debugPrint("\nOPTION MENU: Error loading option data!, using defaults.\n");

    preferencesSetDefaults(false);
    _JustUpdate_();
    _SavePrefs(0);

    return -1;
}

// 0x4928E4
void brightnessIncrease()
{
    gPreferencesBrightness1 = settings.preferences.brightness;

    if (gPreferencesBrightness1 < dbl_50C168) {
        gPreferencesBrightness1 += dbl_50C170;

        if (gPreferencesBrightness1 >= 1.0) {
            if (gPreferencesBrightness1 > dbl_50C168) {
                gPreferencesBrightness1 = dbl_50C168;
            }
        } else {
            gPreferencesBrightness1 = 1.0;
        }

        colorSetBrightness(gPreferencesBrightness1);

        settings.preferences.brightness = gPreferencesBrightness1;

        settingsSave();
    }
}

// 0x4929C8
void brightnessDecrease()
{
    gPreferencesBrightness1 = settings.preferences.brightness;

    if (gPreferencesBrightness1 > 1.0) {
        gPreferencesBrightness1 += dbl_50C178;

        if (gPreferencesBrightness1 >= 1.0) {
            if (gPreferencesBrightness1 > dbl_50C180) {
                gPreferencesBrightness1 = dbl_50C180;
            }
        } else {
            gPreferencesBrightness1 = 1.0;
        }

        colorSetBrightness(gPreferencesBrightness1);

        settings.preferences.brightness = gPreferencesBrightness1;

        settingsSave();
    }
}

// 0x4908A0
static int preferencesWindowInit()
{
    int i;
    int fid;
    char* messageItemText;
    int x;
    int y;
    int width;
    int height;
    int messageItemId;
    int messageItemIdNew;
    int btn;

    if (!messageListInit(&gPreferencesMessageList)) {
        return -1;
    }

    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s%s", asc_5186C8, "options.msg");
    if (!messageListLoad(&gPreferencesMessageList, path)) {
        return -1;
    }

    if (!messageListInit(&gFissionMessageList)) {
        return -1;
    }

    char fissionPath[COMPAT_MAX_PATH];
    snprintf(fissionPath, sizeof(fissionPath), "%s%s", asc_5186C8, "fission.msg");
    if (!messageListLoad(&gFissionMessageList, fissionPath)) {
        return -1;
    }

    _oldFont = fontGetCurrent();

    _SaveSettings();

    for (i = 0; i < PREFERENCES_WINDOW_FRM_COUNT; i++) {
        // Use widescreen variants when available

        int fid = artGetFidWithVariant(OBJ_TYPE_INTERFACE, gPreferencesWindowFrmIds[i], gameIsWidescreen());

        if (!_preferencesFrmImages[i].lock(fid)) {
            fid = buildFid(OBJ_TYPE_INTERFACE, gPreferencesWindowFrmIds[i], 0, 0, 0);
            if (!_preferencesFrmImages[i].lock(fid)) {
                while (--i >= 0) {
                    _preferencesFrmImages[i].unlock();
                }
                return -1;
            }
        }
    }

    _changed = false;
    _graphics_changed = false;
    _widescreen_changed = false;
    _play_area_changed = false;

    int preferencesWindowX = (screenGetWidth() - gOffsets.width) / 2;
    int preferencesWindowY = (screenGetHeight() - gOffsets.height) / 2;
    gPreferencesWindow = windowCreate(preferencesWindowX,
        preferencesWindowY,
        gOffsets.width,
        gOffsets.height,
        256,
        WINDOW_MODAL | WINDOW_DONT_MOVE_TOP | WINDOW_TRANSPARENT);
    if (gPreferencesWindow == -1) {
        for (i = 0; i < PREFERENCES_WINDOW_FRM_COUNT; i++) {
            _preferencesFrmImages[i].unlock();
        }
        return -1;
    }

    gPreferencesWindowBuffer = windowGetBuffer(gPreferencesWindow);

    // Copy to window buffer
    memcpy(gPreferencesWindowBuffer, _preferencesFrmImages[PREFERENCES_WINDOW_FRM_BACKGROUND].getData(), gOffsets.width * gOffsets.height);

    fontSetCurrent(104);

    messageItemText = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, 100);
    fontDrawText(gPreferencesWindowBuffer + gOffsets.width * gOffsets.titleTextY + gOffsets.titleTextX, messageItemText, gOffsets.width, gOffsets.width, _colorTable[18979]);

    fontSetCurrent(103);

    messageItemId = 101;
    // Primary Prefs Main labels
    for (i = 0; i < PRIMARY_PREF_COUNT; i++) {
        messageItemText = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, messageItemId++);
        x = gOffsets.primLabelColX - fontGetStringWidth(messageItemText) / 2;
        fontDrawText(gPreferencesWindowBuffer + gOffsets.width * gOffsets.row1Ytab[i] + x, messageItemText, gOffsets.width, gOffsets.width, _colorTable[18979]);
    }

    // Secondary Prefs Main labels
    for (i = 0; i < SECONDARY_PREF_COUNT; i++) {
        messageItemText = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, messageItemId++);
        fontDrawText(gPreferencesWindowBuffer + gOffsets.width * gOffsets.row2Ytab[i] + gOffsets.secLabelColX, messageItemText, gOffsets.width, gOffsets.width, _colorTable[18979]);
    }

    if (gameIsWidescreen()) {
        // Draw tertiary preference main labels
        messageItemIdNew = 124;
        for (i = 0; i < TERTIARY_PREF_COUNT; i++) {
            messageItemText = getmsg(&gFissionMessageList, &gFissionMessageListItem, messageItemIdNew++);
            x = gOffsets.terLabelColX - fontGetStringWidth(messageItemText) / 2;
            fontDrawText(gPreferencesWindowBuffer + gOffsets.width * gOffsets.row2bYtab[i] + x,
                messageItemText, gOffsets.width, gOffsets.width, _colorTable[18979]);
        }

        // Draw quaternary dial label
        /*messageItemText = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, messageItemIdNew++);
        x = gOffsets.terLabelColX - fontGetStringWidth(messageItemText) / 2; // use terLabelColX because dial is in tertiary preferences column
        fontDrawText(gPreferencesWindowBuffer + gOffsets.width * gOffsets.rowdialYtab[0] + x, messageItemText, gOffsets.width, gOffsets.width, _colorTable[18979]);*/
    }

    // Range Prefs Main labels
    for (i = 0; i < RANGE_PREF_COUNT; i++) {
        messageItemText = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, messageItemId++);
        fontDrawText(gPreferencesWindowBuffer + gOffsets.width * gOffsets.row3Ytab[i] + gOffsets.rangLabelColX, messageItemText, gOffsets.width, gOffsets.width, _colorTable[18979]);
    }

    // DEFAULT
    messageItemText = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, 120);
    fontDrawText(gPreferencesWindowBuffer + gOffsets.width * gOffsets.defaultLabelY + gOffsets.defaultLabelX, messageItemText, gOffsets.width, gOffsets.width, _colorTable[18979]);

    // DONE
    messageItemText = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, 4);
    fontDrawText(gPreferencesWindowBuffer + gOffsets.width * gOffsets.doneLabelY + gOffsets.doneLabelX, messageItemText, gOffsets.width, gOffsets.width, _colorTable[18979]);

    // CANCEL
    messageItemText = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, 121);
    fontDrawText(gPreferencesWindowBuffer + gOffsets.width * gOffsets.cancelLabelY + gOffsets.cancelLabelX, messageItemText, gOffsets.width, gOffsets.width, _colorTable[18979]);

    // Affect Player Speed in strictVanilla mode - Affect Non-combat Speed otherwise 
    fontSetCurrent(101);
    if (gStrictVanillaEnabled){
        messageItemText = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, 122);
    } else {
        messageItemText = getmsg(&gFissionMessageList, &gFissionMessageListItem, 110);
    }
    fontDrawText(gPreferencesWindowBuffer + gOffsets.width * gOffsets.speedLabelX + gOffsets.speedLabelY, messageItemText, gOffsets.width, gOffsets.width, _colorTable[18979]);

    for (i = 0; i < PREF_COUNT; i++) {
        _UpdateThing(i);
    }

    for (i = 0; i < PREF_COUNT; i++) {
        int mouseEnterEventCode;
        int mouseExitEventCode;
        int mouseDownEventCode;
        int mouseUpEventCode;

        // Get the position from our offset struct - this is resolution-aware
        Point pos = gOffsets.preferencePositions[i];
        int knobX = pos.x;
        int knobY = pos.y;

        if (i >= FIRST_RANGE_PREF) {
            // Range preferences (sliders)
            int x = gOffsets.rangeStartX;
            int y = knobY + gOffsets.rangeButtonOffsetY;
            int width = gOffsets.rangeBlitWidth;
            int height = 23;
            gPreferenceDescriptions[i].btn = buttonCreate(
                gPreferencesWindow,
                x, y, width, height,
                532, 532, 505 + i, 532,
                nullptr, nullptr, nullptr, 32);
        } else if (i >= FIRST_QUATERNARY_PREF) {
            // Only create quaternary button in widescreen mode
            if (gameIsWidescreen()) {
                // Quaternary preference (large multi-option dial)
                int x = gPreferenceDescriptions[i].minX;
                int y = knobY + gOffsets.quaternaryButtonOffsetY;
                int width = gPreferenceDescriptions[i].maxX - x;
                int height = 56;
                gPreferenceDescriptions[i].btn = buttonCreate(
                    gPreferencesWindow,
                    x, y, width, height,
                    -1, -1, -1, 505 + i,
                    nullptr, nullptr, nullptr, 32);
            } else {
                gPreferenceDescriptions[i].btn = -1; // Mark as invalid button
            }
        } else if (i >= FIRST_TERTIARY_PREF) {
            // Only create tertiary buttons in widescreen mode
            if (gameIsWidescreen()) {
                int x = gPreferenceDescriptions[i].minX;
                int y = knobY + gOffsets.tertiaryButtonOffsetY;
                int width = gPreferenceDescriptions[i].maxX - x;
                int height = 85;
                gPreferenceDescriptions[i].btn = buttonCreate(
                    gPreferencesWindow,
                    x, y, width, height,
                    -1, -1, -1, 505 + i,
                    nullptr, nullptr, nullptr, 32);
            } else {
                gPreferenceDescriptions[i].btn = -1; // Mark as invalid button
            }
        } else if (i >= FIRST_SECONDARY_PREF) {
            // Secondary preferences (toggle buttons)
            int x = gPreferenceDescriptions[i].minX;
            int y = knobY + gOffsets.secondaryButtonOffsetY;
            int width = gPreferenceDescriptions[i].maxX - x;
            int height = 28;
            gPreferenceDescriptions[i].btn = buttonCreate(
                gPreferencesWindow,
                x, y, width, height,
                -1, -1, -1, 505 + i,
                nullptr, nullptr, nullptr, 32);
        } else {
            // Primary preferences (multi-option knobs)
            int x = gPreferenceDescriptions[i].minX;
            int y = knobY + gOffsets.primaryButtonOffsetY;
            int width = gPreferenceDescriptions[i].maxX - x;
            int height = 48;
            gPreferenceDescriptions[i].btn = buttonCreate(
                gPreferencesWindow,
                x, y, width, height,
                -1, -1, -1, 505 + i,
                nullptr, nullptr, nullptr, 32);
        }
    }

    // Player Speed Checkbox in strictVanilla mode - Affect Non-combat Speed otherwise
    _plyrspdbid = buttonCreate(gPreferencesWindow,
        gOffsets.playerSpeedCheckboxX,
        gOffsets.playerSpeedCheckboxY,
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_CHECKBOX_OFF].getWidth(),
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_CHECKBOX_ON].getHeight(),
        -1,
        -1,
        531,
        531,
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_CHECKBOX_OFF].getData(),
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_CHECKBOX_ON].getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT | BUTTON_FLAG_0x01 | BUTTON_FLAG_0x02);
    if (_plyrspdbid != -1) {
        _win_set_button_rest_state(_plyrspdbid, gPreferencesPlayerSpeedup1, 0);
    }
    buttonSetCallbacks(_plyrspdbid, _gsound_med_butt_press, _gsound_med_butt_press);

    // DEFAULT Button
    btn = buttonCreate(gPreferencesWindow,
        gOffsets.defaultButtonX,
        gOffsets.defaultButtonY,
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_UP].getWidth(),
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_DOWN].getHeight(),
        -1,
        -1,
        -1,
        533,
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_UP].getData(),
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_DOWN].getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (btn != -1) {
        buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
    }

    // DONE Button
    btn = buttonCreate(gPreferencesWindow,
        gOffsets.doneButtonX,
        gOffsets.doneButtonY,
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_UP].getWidth(),
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_DOWN].getHeight(),
        -1,
        -1,
        -1,
        504, // Note: Changed to 630 in 800p version - why did I do that..?
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_UP].getData(),
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_DOWN].getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (btn != -1) {
        buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
    }

    // CANCEL Button
    btn = buttonCreate(gPreferencesWindow,
        gOffsets.cancelButtonX,
        gOffsets.cancelButtonY,
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_UP].getWidth(),
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_DOWN].getHeight(),
        -1,
        -1,
        -1,
        534,
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_UP].getData(),
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_DOWN].getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (btn != -1) {
        buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
    }
    fontSetCurrent(101);

    windowRefresh(gPreferencesWindow);

    return 0;
}

// 0x492870
static int preferencesWindowFree()
{
    if (_changed) {
        _SavePrefs(1);
        _JustUpdate_();
        _combat_highlight_change();
    }

    windowDestroy(gPreferencesWindow);

    for (int index = 0; index < PREFERENCES_WINDOW_FRM_COUNT; index++) {
        _preferencesFrmImages[index].unlock();
    }

    fontSetCurrent(_oldFont);

    messageListFree(&gPreferencesMessageList);
    touch_set_touchscreen_mode(false);

    return 0;
}

static int showGraphicsConfirmationDialog(bool widescreenChanged, bool playAreaChanged)
{
    if (widescreenChanged || playAreaChanged) {
        // Warning dialog (Yes/No)
        const int titleMsgId = widescreenChanged ? 100 : 105;
        const char* title = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, titleMsgId);
        const char* bodyText = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 101);
        const char* bodyText2 = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 102);
        const char* bodyLines[] = { bodyText, bodyText2 };

        soundPlayFile("iisxxxx1");
        return showDialogBox(
            title,
            bodyLines,
            2,
            192, 160,
            _colorTable[32328],
            nullptr,
            _colorTable[32328],
            DIALOG_BOX_YES_NO);
    } else {
        // Info dialog (OK)
        const char* title = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 103);
        const char* bodyText = (const char*)getmsg(&gFissionMessageList, &gFissionMessageListItem, 104);
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
        return 1; // Always proceed after info dialog
    }
}

// 0x490798
int doPreferences(bool animated)
{
    ScopedGameMode gm(GameMode::kPreferences);

    if (preferencesWindowInit() == -1) {
        debugPrint("\nPREFERENCE MENU: Error loading preference dialog data!\n");
        return -1;
    }

    bool cursorWasHidden = cursorIsHidden();
    if (cursorWasHidden) {
        mouseShowCursor();
    }

    touch_set_touchscreen_mode(true);

    if (animated) {
        colorPaletteLoad("color.pal");
        paletteFadeTo(_cmap);
    }

    int rc = -1;
    while (rc == -1) {
        sharedFpsLimiter.mark();

        int eventCode = inputGetInput();

        switch (eventCode) {
        case KEY_RETURN:
        case KEY_UPPERCASE_P:
        case KEY_LOWERCASE_P:
            soundPlayFile("ib1p1xx1");
            // FALLTHROUGH
        case 504:
            if (_graphics_changed) {
                int dialogResult = showGraphicsConfirmationDialog(_widescreen_changed, _play_area_changed);

                // Handle dialog results
                if (dialogResult == 1) {
                    rc = 1; // User confirmed - save and exit
                } else if (dialogResult == 0) {
                    rc = -1; // User canceled - stay in preferences
                }
            } else {
                // No graphics changes - exit immediately
                rc = 1;
            }
            break;
        case KEY_CTRL_Q:
        case KEY_CTRL_X:
        case KEY_F10:
            showQuitConfirmationDialog();
            break;
        case KEY_EQUAL:
        case KEY_PLUS:
            brightnessIncrease();
            break;
        case KEY_MINUS:
        case KEY_UNDERSCORE:
            brightnessDecrease();
            break;
        case KEY_F12:
            takeScreenshot();
            break;
        case 533:
            showGraphicsConfirmationDialog(false, false);
            preferencesSetDefaults(true);
            break;
        default:
            if (eventCode == KEY_ESCAPE || eventCode == 534 || _game_user_wants_to_quit != 0) {
                _RestoreSettings();
                rc = 0;
            } else if (eventCode >= 505 && eventCode <= 531) {
                _DoThing(eventCode);
            }
            break;
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    if (animated) {
        paletteFadeTo(gPaletteBlack);
    }

    if (cursorWasHidden) {
        mouseHideCursor();
    }

    preferencesWindowFree();

    return rc;
}

// 0x490E8C
static void _DoThing(int eventCode)
{
    int x;
    int y;
    mouseGetPositionInWindow(gPreferencesWindow, &x, &y);

    // This preference index also contains out-of-bounds value 19,
    // which is the only preference expressed as checkbox.
    int preferenceIndex = eventCode - 505;

    if (preferenceIndex >= FIRST_PRIMARY_PREF && preferenceIndex <= LAST_PRIMARY_PREF) {
        PreferenceDescription* meta = &(gPreferenceDescriptions[preferenceIndex]);
        Point pos = gOffsets.preferencePositions[preferenceIndex]; // Get position directly
        int* valuePtr = meta->valuePtr;
        int value = *valuePtr;
        bool valueChanged = false;

        // Use hit detection offsets from struct with direct position
        int v1 = pos.x + gOffsets.primaryKnobHitX;
        int v2 = pos.y + gOffsets.primaryKnobHitY;

        if (sqrt(pow((double)x - (double)v1, 2) + pow((double)y - (double)v2, 2)) > 16.0) {
            if (y > pos.y) {
                int v14 = pos.y + gOffsets.optionYOffsets[0];
                if (y >= v14 && y <= v14 + fontGetLineHeight()) {
                    if (x >= meta->minX && x <= pos.x) {
                        *valuePtr = 0;
                        meta->direction = 0;
                        valueChanged = true;
                    } else {
                        if (meta->valuesCount >= 3 && x >= pos.x + gOffsets.optionXOffsets[2] && x <= meta->maxX) {
                            *valuePtr = 2;
                            meta->direction = 0;
                            valueChanged = true;
                        }
                    }
                }
            } else {
                if (x >= pos.x + gOffsets.primaryButtonMinXOffset && x <= pos.x + gOffsets.primaryButtonMaxXOffset) {
                    *valuePtr = 1;
                    if (value != 0) {
                        meta->direction = 1;
                    } else {
                        meta->direction = 0;
                    }
                    valueChanged = true;
                }
            }

            if (meta->valuesCount == 4) {
                int v19 = pos.y + gOffsets.optionYOffsets[3];
                if (y >= v19 && y <= v19 + 2 * fontGetLineHeight() && x >= pos.x + gOffsets.optionXOffsets[3] && x <= meta->maxX) {
                    *valuePtr = 3;
                    meta->direction = 1;
                    valueChanged = true;
                }
            }
        } else {
            if (meta->direction != 0) {
                if (value == 0) {
                    meta->direction = 0;
                }
            } else {
                if (value == meta->valuesCount - 1) {
                    meta->direction = 1;
                }
            }

            if (meta->direction != 0) {
                *valuePtr = value - 1;
            } else {
                *valuePtr = value + 1;
            }

            valueChanged = true;
        }

        if (valueChanged) {
            soundPlayFile("ib3p1xx1");
            inputBlockForTocks(70);
            soundPlayFile("ib3lu1x1");
            _UpdateThing(preferenceIndex);
            windowRefresh(gPreferencesWindow);
            _changed = true;
            return;
        }
    } else if (preferenceIndex >= FIRST_SECONDARY_PREF && preferenceIndex <= LAST_SECONDARY_PREF) {
        PreferenceDescription* meta = &(gPreferenceDescriptions[preferenceIndex]);
        Point pos = gOffsets.preferencePositions[preferenceIndex]; // Get position directly
        int* valuePtr = meta->valuePtr;
        int value = *valuePtr;
        bool valueChanged = false;

        // Use hit detection offsets from struct with direct position
        int v1 = pos.x + gOffsets.secondaryKnobHitX;
        int v2 = pos.y + gOffsets.secondaryKnobHitY;

        if (sqrt(pow((double)x - (double)v1, 2) + pow((double)y - (double)v2, 2)) > 10.0) {
            int v23 = pos.y - 5;
            if (y >= v23 && y <= v23 + fontGetLineHeight() + 2) {
                if (x >= meta->minX && x <= pos.x) {
                    *valuePtr = preferenceIndex == PREF_COMBAT_MESSAGES ? 1 : 0;
                    valueChanged = true;
                } else if (x >= pos.x + gOffsets.secondaryButtonXOffset && x <= meta->maxX) {
                    *valuePtr = preferenceIndex == PREF_COMBAT_MESSAGES ? 0 : 1;
                    valueChanged = true;
                }
            }
        } else {
            *valuePtr ^= 1; // Toggle value directly
            valueChanged = true;
        }

        if (valueChanged) {
            soundPlayFile("ib2p1xx1");
            inputBlockForTocks(70);
            soundPlayFile("ib2lu1x1");
            _UpdateThing(preferenceIndex);
            windowRefresh(gPreferencesWindow);
            _changed = true;
            return;
        }
    } else if ((preferenceIndex >= FIRST_TERTIARY_PREF && preferenceIndex <= LAST_TERTIARY_PREF) && gameIsWidescreen()) {
        PreferenceDescription* meta = &(gPreferenceDescriptions[preferenceIndex]);
        Point pos = gOffsets.preferencePositions[preferenceIndex]; // Get position directly
        int* valuePtr = meta->valuePtr;
        int value = *valuePtr;
        bool valueChanged = false;

        // Use hit detection offsets from struct with direct position
        int v1 = pos.x + gOffsets.tertiaryKnobHitX;
        int v2 = pos.y + gOffsets.tertiaryKnobHitY;

        if (sqrt(pow((double)x - (double)v1, 2) + pow((double)y - (double)v2, 2)) > 20.0) {
            int labelY0 = pos.y - gOffsets.tertiaryOptionYOffsets[0]; // Top label
            int labelY1 = pos.y - gOffsets.tertiaryOptionYOffsets[1]; // Bottom label

            int labelHeight = fontGetLineHeight() + 2;

            if (y >= labelY0 && y <= labelY0 + labelHeight) {
                *valuePtr = 0;
                valueChanged = true;
            } else if (y >= labelY1 && y <= labelY1 + labelHeight) {
                *valuePtr = 1;
                valueChanged = true;
            }
        } else {
            *valuePtr ^= 1; // Toggle value directly
            valueChanged = true;
        }

        if (valueChanged) {
            soundPlayFile("toggle");
            inputBlockForTocks(70);
            _UpdateThing(preferenceIndex);
            windowRefresh(gPreferencesWindow);
            _changed = true;
            _graphics_changed = true;
            if ((preferenceIndex == FIRST_TERTIARY_PREF + 1) && (*valuePtr == 0)) {
                _widescreen_changed = true;
            } else {
                _widescreen_changed = false;
            }
            return;
        }
    } else if (preferenceIndex == FIRST_QUATERNARY_PREF) { // Play Area dial
        PreferenceDescription* meta = &(gPreferenceDescriptions[preferenceIndex]);
        Point pos = gOffsets.preferencePositions[preferenceIndex]; // Base position of preference widget
        int* currentValuePtr = meta->valuePtr; // Pointer to preference's current value
        int currentValue = *currentValuePtr; // Actual value of the preference
        bool valueChanged = false; // Flag to track if value changed

        // Calculate center position of the circular knob
        int knobCenterX = pos.x + gOffsets.quaternaryKnobHitX;
        int knobCenterY = pos.y + gOffsets.quaternaryKnobHitY;

        // Calculate distance from click point to knob center
        double distanceToKnob = sqrt(pow((double)x - (double)knobCenterX, 2) + pow((double)y - (double)knobCenterY, 2));

        // Check if click is outside the circular knob (radius 26 pixels)
        if (distanceToKnob > 26.0) {
            // Check ABOVE KNOB area for values 1 and 2
            if (y <= knobCenterY) {
                int aboveKnobY = pos.y + gOffsets.quaternaryYOffsets[1]; // Position for above-knob labels
                int aboveKnobBottom = aboveKnobY + fontGetLineHeight();

                // Check if click is within the above-knob vertical range
                if (y >= aboveKnobY && y <= aboveKnobBottom) {
                    // Left label area (value 1)
                    if (x >= meta->minX && x <= pos.x) {
                        *currentValuePtr = 1;
                        meta->direction = 0;
                        valueChanged = true;
                    }
                    // Right label area (value 2)
                    else if (x >= pos.x + gOffsets.quaternaryXOffsets[2] && x <= meta->maxX) {
                        *currentValuePtr = 2;
                        meta->direction = 0;
                        valueChanged = true;
                    }
                }
            }
            // Check BELOW KNOB area for values 0 and 3
            else {
                int belowKnobY = pos.y + gOffsets.quaternaryYOffsets[0]; // Position for below-knob labels
                int belowKnobBottom = belowKnobY + fontGetLineHeight();

                // Check if click is within below-knob vertical range
                if (y >= belowKnobY && y <= belowKnobBottom) {
                    // Left label area (value 0)
                    if (x >= meta->minX && x <= pos.x) {
                        *currentValuePtr = 0;
                        meta->direction = 0;
                        valueChanged = true;
                    }
                    // Right label area (value 3)
                    else if (x >= pos.x + gOffsets.quaternaryXOffsets[3] && x <= meta->maxX) {
                        *currentValuePtr = 3;
                        meta->direction = 1;
                        valueChanged = true;
                    }
                }
            }
        }
        // Click was INSIDE the circular knob - handle cycling
        else {
            // Handle cycling direction logic:
            // - If we were cycling but reached min value, stop cycling
            if (meta->direction != 0) {
                if (currentValue == 0) {
                    meta->direction = 0; // Stop cycling at min value
                }
            }
            // - If we reached max value, reverse cycling direction
            else {
                if (currentValue == 3) { // Max value is 3 for 4-value preferences
                    meta->direction = 1; // Start cycling backward
                }
            }

            // Apply cycling based on direction
            if (meta->direction != 0) {
                *currentValuePtr = currentValue - 1; // Cycle backward
            } else {
                *currentValuePtr = currentValue + 1; // Cycle forward
            }

            valueChanged = true;
        }

        // If value changed, handle UI updates
        if (valueChanged) {
            soundPlayFile("butin1");
            inputBlockForTocks(150);
            soundPlayFile("ib3lu1x1");
            _UpdateThing(preferenceIndex);
            windowRefresh(gPreferencesWindow);
            _changed = true;
            _graphics_changed = true;
            if (*currentValuePtr == 0) {
                _play_area_changed = true;
            } else {
                _play_area_changed = false;
            }
            return;
        }
    } else if (preferenceIndex >= FIRST_RANGE_PREF && preferenceIndex <= LAST_RANGE_PREF) {
        PreferenceDescription* meta = &(gPreferenceDescriptions[preferenceIndex]);
        int* valuePtr = meta->valuePtr;

        soundPlayFile("ib1p1xx1");

        double value;
        switch (preferenceIndex) {
        case PREF_TEXT_BASE_DELAY:
            // fixed slider handling
            value = std::clamp(6.0 - gPreferencesTextBaseDelay1 + 1, 1.0, 6.0);
            break;
        case PREF_BRIGHTNESS:
            value = gPreferencesBrightness1;
            break;
        case PREF_MOUSE_SENSITIVIY:
            value = gPreferencesMouseSensitivity1;
            break;
        default:
            value = *valuePtr;
            break;
        }

        // Calculate initial slider position
        Point pos = gOffsets.preferencePositions[preferenceIndex]; // Get position directly
        int v31 = (int)(value - meta->minValue) * (gOffsets.rangeSliderWidth / (meta->maxValue - meta->minValue)) + gOffsets.rangeStartX;
        int pitch = gOffsets.width;

        // Restore background for slider track ONLY (12px tall)
        blitBufferToBuffer(
            _preferencesFrmImages[PREFERENCES_WINDOW_FRM_BACKGROUND].getData() + pitch * pos.y + gOffsets.rangeStartX, // Use direct Y position
            gOffsets.rangeBlitWidth, // 240 (640) or 300 (800)
            12, // Fixed track height (matches knob)
            pitch,
            gPreferencesWindowBuffer + pitch * pos.y + gOffsets.rangeStartX,
            pitch);

        // Draw slider knob at calculated position
        blitBufferToBufferTrans(
            _preferencesFrmImages[PREFERENCES_WINDOW_FRM_KNOB_ON].getData(),
            21, 12, 21,
            gPreferencesWindowBuffer + pitch * pos.y + v31,
            pitch);

        windowRefresh(gPreferencesWindow);

        int sfxVolumeExample = 0;
        int speechVolumeExample = 0;
        while (true) {
            sharedFpsLimiter.mark();

            inputGetInput();

            int tick = getTicks();

            mouseGetPositionInWindow(gPreferencesWindow, &x, &y);

            if (mouseGetEvent() & 0x10) {
                soundPlayFile("ib1lu1x1");
                _UpdateThing(preferenceIndex);
                windowRefresh(gPreferencesWindow);
                renderPresent();
                _changed = true;
                return;
            }

            if (v31 + gOffsets.rangeThumbRightOffset > x) {
                if (v31 + gOffsets.rangeThumbLeftOffset > x) {
                    v31 = x - gOffsets.rangeThumbLeftOffset;
                    if (v31 < gOffsets.rangeSliderMinX) {
                        v31 = gOffsets.rangeSliderMinX;
                    }
                }
            } else {
                // FIXED: Use offset instead of hardcoded 6
                v31 = x - gOffsets.rangeThumbLeftOffset;
                if (v31 > gOffsets.rangeSliderMaxX) {
                    v31 = gOffsets.rangeSliderMaxX;
                }
            }

            // Fix for saving text delay
            double newValue;
            if (preferenceIndex == PREF_TEXT_BASE_DELAY) {
                // Inverted calculation for text delay (1.0-6.0 range)
                newValue = 6.0 - (((double)v31 - (double)gOffsets.rangeSliderMinX) * 5.0 / gOffsets.rangeSliderWidth);
            } else {
                // Standard calculation for other sliders
                newValue = ((double)v31 - (double)gOffsets.rangeSliderMinX) * (meta->maxValue - meta->minValue) / gOffsets.rangeSliderWidth + meta->minValue;
            }

            int v52 = 0;

            switch (preferenceIndex) {
            case PREF_COMBAT_SPEED:
                *meta->valuePtr = (int)newValue;
                break;
            case PREF_TEXT_BASE_DELAY:
                gPreferencesTextBaseDelay1 = newValue; // Store in 1.0-6.0 range
                break;
            case PREF_MASTER_VOLUME:
                *meta->valuePtr = (int)newValue;
                gameSoundSetMasterVolume(gPreferencesMasterVolume1);
                v52 = 1;
                break;
            case PREF_MUSIC_VOLUME:
                *meta->valuePtr = (int)newValue;
                backgroundSoundSetVolume(gPreferencesMusicVolume1);
                v52 = 1;
                break;
            case PREF_SFX_VOLUME:
                *meta->valuePtr = (int)newValue;
                soundEffectsSetVolume(gPreferencesSoundEffectsVolume1);
                v52 = 1;
                if (sfxVolumeExample == 0) {
                    soundPlayFile("butin1");
                    sfxVolumeExample = 7;
                } else {
                    sfxVolumeExample--;
                }
                break;
            case PREF_SPEECH_VOLUME:
                *meta->valuePtr = (int)newValue;
                speechSetVolume(gPreferencesSpeechVolume1);
                v52 = 1;
                if (speechVolumeExample == 0) {
                    speechLoad("narrator\\options", 12, 13, 15);
                    speechVolumeExample = 40;
                } else {
                    speechVolumeExample--;
                }
                break;
            case PREF_BRIGHTNESS:
                gPreferencesBrightness1 = newValue;
                colorSetBrightness(newValue);
                break;
            case PREF_MOUSE_SENSITIVIY:
                gPreferencesMouseSensitivity1 = newValue;
                break;
            }

            if (v52) {
                // Volume sliders - restore background including labels
                int off = gOffsets.width * (pos.y - 12) + gOffsets.rangeStartX; // Use direct Y position
                blitBufferToBuffer(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_BACKGROUND].getData() + off,
                    gOffsets.rangeBlitWidth,
                    24,
                    gOffsets.width,
                    gPreferencesWindowBuffer + off,
                    gOffsets.width);

                for (int optionIndex = 0; optionIndex < meta->valuesCount; optionIndex++) {
                    const char* str = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, meta->labelIds[optionIndex]);

                    int x;
                    switch (optionIndex) {
                    case 0:
                        x = gOffsets.rangeLabelX[0];
                        break;
                    case 1:
                        switch (meta->valuesCount) {
                        case 2:
                            x = gOffsets.rangeLabelX[3] - fontGetStringWidth(str);
                            break;
                        case 3:
                            x = gOffsets.rangeLabelX[1] - fontGetStringWidth(str) / 2 - 2;
                            break;
                        case 4:
                            x = gOffsets.rangeLabelX[4] + fontGetStringWidth(str) / 2 - 8;
                            break;
                        }
                        break;
                    case 2:
                        switch (meta->valuesCount) {
                        case 3:
                            x = gOffsets.rangeLabelX[3] - fontGetStringWidth(str);
                            break;
                        case 4:
                            x = gOffsets.rangeLabelX[2] - fontGetStringWidth(str) - 4;
                            break;
                        }
                        break;
                    case 3:
                        x = gOffsets.rangeLabelX[3] - fontGetStringWidth(str);
                        break;
                    }
                    fontDrawText(gPreferencesWindowBuffer + pitch * (pos.y - 12) + x,
                        str, pitch, pitch, _colorTable[18979]);
                }
            } else {
                // Non-volume sliders - restore only slider track
                int off = gOffsets.width * pos.y + gOffsets.rangeStartX;
                blitBufferToBuffer(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_BACKGROUND].getData() + off,
                    gOffsets.rangeBlitWidth,
                    12,
                    gOffsets.width,
                    gPreferencesWindowBuffer + off,
                    gOffsets.width);
            }

            blitBufferToBufferTrans(_preferencesFrmImages[PREFERENCES_WINDOW_FRM_KNOB_ON].getData(),
                21, 12, 21,
                gPreferencesWindowBuffer + pitch * pos.y + v31,
                pitch);
            windowRefresh(gPreferencesWindow);

            delay_ms(35 - (getTicks() - tick));

            renderPresent();
            sharedFpsLimiter.throttle();
        }
    } else if (preferenceIndex == 26) {
        gPreferencesPlayerSpeedup1 ^= 1;
    }

    _changed = true;
}

} // namespace fallout
