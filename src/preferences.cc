#include "preferences.h"

#include "options.h"

#include <algorithm>

#include "art.h"
#include "color.h"
#include "combat.h"
#include "combat_ai.h"
#include "debug.h"
#include "delay.h"
#include "draw.h"
#include "game.h"
#include "game_mouse.h"
#include "game_sound.h"
#include "graph_lib.h"
#include "input.h"
#include "kb.h"
#include "memory.h"
#include "message.h"
#include "mouse.h"
#include "palette.h"
#include "scripts.h"
#include "settings.h"
#include "svga.h"
#include "text_font.h"
#include "text_object.h"
#include "window_manager.h"

namespace fallout {

/*#define PREFERENCES_WINDOW_WIDTH 640
#define PREFERENCES_WINDOW_HEIGHT 480

#define PRIMARY_OPTION_VALUE_COUNT 4
#define SECONDARY_OPTION_VALUE_COUNT 2*/

/*typedef enum Preference {
    PREF_GAME_DIFFICULTY,
    PREF_COMBAT_DIFFICULTY,
    PREF_VIOLENCE_LEVEL,
    PREF_TARGET_HIGHLIGHT,
    PREF_COMBAT_LOOKS,
    PREF_COMBAT_MESSAGES,
    PREF_COMBAT_TAUNTS,
    PREF_LANGUAGE_FILTER,
    PREF_RUNNING,
    PREF_SUBTITLES,
    PREF_ITEM_HIGHLIGHT,
    PREF_COMBAT_SPEED,
    PREF_TEXT_BASE_DELAY,
    PREF_MASTER_VOLUME,
    PREF_MUSIC_VOLUME,
    PREF_SFX_VOLUME,
    PREF_SPEECH_VOLUME,
    PREF_BRIGHTNESS,
    PREF_MOUSE_SENSITIVIY,
    PREF_COUNT,
    FIRST_PRIMARY_PREF = PREF_GAME_DIFFICULTY,
    LAST_PRIMARY_PREF = PREF_COMBAT_LOOKS,
    PRIMARY_PREF_COUNT = LAST_PRIMARY_PREF - FIRST_PRIMARY_PREF + 1,
    FIRST_SECONDARY_PREF = PREF_COMBAT_MESSAGES,
    LAST_SECONDARY_PREF = PREF_ITEM_HIGHLIGHT,
    SECONDARY_PREF_COUNT = LAST_SECONDARY_PREF - FIRST_SECONDARY_PREF + 1,
    FIRST_RANGE_PREF = PREF_COMBAT_SPEED,
    LAST_RANGE_PREF = PREF_MOUSE_SENSITIVIY,
    RANGE_PREF_COUNT = LAST_RANGE_PREF - FIRST_RANGE_PREF + 1,
} Preference;

typedef enum PreferencesWindowFrm {
    PREFERENCES_WINDOW_FRM_BACKGROUND,
    // Knob (for range preferences)
    PREFERENCES_WINDOW_FRM_KNOB_OFF,
    // 4-way switch (for primary preferences)
    PREFERENCES_WINDOW_FRM_PRIMARY_SWITCH,
    // 2-way switch (for secondary preferences)
    PREFERENCES_WINDOW_FRM_SECONDARY_SWITCH,
    PREFERENCES_WINDOW_FRM_CHECKBOX_ON,
    PREFERENCES_WINDOW_FRM_CHECKBOX_OFF,
    PREFERENCES_WINDOW_FRM_6,
    // Active knob (for range preferences)
    PREFERENCES_WINDOW_FRM_KNOB_ON,
    PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_UP,
    PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_DOWN,
    PREFERENCES_WINDOW_FRM_COUNT,
} PreferencesWindowFrm;

typedef struct PreferenceDescription {
    // The number of options.
    short valuesCount;

    // Direction of rotation:
    // 0 - clockwise (incrementing value),
    // 1 - counter-clockwise (decrementing value)
    short direction;
    short knobX;
    short knobY;
    // Min x coordinate of the preference control bounding box.
    short minX;
    // Max x coordinate of the preference control bounding box.
    short maxX;
    short labelIds[PRIMARY_OPTION_VALUE_COUNT];
    int btn;
    char name[32];
    double minValue;
    double maxValue;
    int* valuePtr;
} PreferenceDescription;*/

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

// 0x48FBD0
/*const int _row1Ytab[PRIMARY_PREF_COUNT] = {
    48,
    125,
    203,
    286,
    363,
};

// 0x48FBDA
const int _row2Ytab[SECONDARY_PREF_COUNT] = {
    49,
    116,
    181,
    247,
    313,
    380,
};

// 0x48FBE6
const int _row3Ytab[RANGE_PREF_COUNT] = {
    19,
    94,
    165,
    216,
    268,
    319,
    369,
    420,
};

// x offsets for primary preferences from the knob position
// 0x48FBF6
const short word_48FBF6[PRIMARY_OPTION_VALUE_COUNT] = {
    2,
    25,
    46,
    46,
};

// y offsets for primary preference option values from the knob position
// 0x48FBFE
const short word_48FBFE[PRIMARY_OPTION_VALUE_COUNT] = {
    10,
    -4,
    10,
    31,
};

// x offsets for secondary prefrence option values from the knob position
// 0x48FC06
const short word_48FC06[SECONDARY_OPTION_VALUE_COUNT] = {
    4,
    21,
};

// y offsets for secondary preferences
// 0x48FC30
const int dword_48FC30[SECONDARY_PREF_COUNT] = {
    66, // combat messages
    133, // combat taunts
    200, // language filter
    264, // running
    331, // subtitles
    397, // item highlight
};

// y offsets for primary preferences
// 0x48FC1C
const int dword_48FC1C[PRIMARY_PREF_COUNT] = {
    66, // game difficulty
    143, // combat difficulty
    222, // violence level
    304, // target highlight
    382, // combat looks
};*/

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
};

// 0x6637E8
static MessageList gPreferencesMessageList;

// 0x663840
static MessageListItem gPreferencesMessageListItem;

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

static int gPreferencesWidescreen1;
static int gPreferencesWidescreen2;

// Added for offsets handling
static PreferencesOffsets gOffsets;

// 0x5197F8
static PreferenceDescription gPreferenceDescriptions[PREF_COUNT] = {
    { 3, 0, 76, 71, 0, 0, { 203, 204, 205, 0 }, 0, GAME_CONFIG_GAME_DIFFICULTY_KEY, 0, 0, &gPreferencesGameDifficulty1 },
    { 3, 0, 76, 149, 0, 0, { 206, 204, 208, 0 }, 0, GAME_CONFIG_COMBAT_DIFFICULTY_KEY, 0, 0, &gPreferencesCombatDifficulty1 },
    { 4, 0, 76, 226, 0, 0, { 214, 215, 204, 216 }, 0, GAME_CONFIG_VIOLENCE_LEVEL_KEY, 0, 0, &gPreferencesViolenceLevel1 },
    { 3, 0, 76, 309, 0, 0, { 202, 201, 213, 0 }, 0, GAME_CONFIG_TARGET_HIGHLIGHT_KEY, 0, 0, &gPreferencesTargetHighlight1 },
    { 2, 0, 76, 387, 0, 0, { 202, 201, 0, 0 }, 0, GAME_CONFIG_WIDESCREEN, 0, 0, &gPreferencesWidescreen1 },
    //{ 2, 0, 76, 387, 0, 0, { 202, 201, 0, 0 }, 0, GAME_CONFIG_COMBAT_LOOKS_KEY, 0, 0, &gPreferencesCombatLooks1 },
    { 2, 0, 299, 74, 0, 0, { 211, 212, 0, 0 }, 0, GAME_CONFIG_COMBAT_MESSAGES_KEY, 0, 0, &gPreferencesCombatMessages1 },
    { 2, 0, 299, 141, 0, 0, { 202, 201, 0, 0 }, 0, GAME_CONFIG_COMBAT_TAUNTS_KEY, 0, 0, &gPreferencesCombatTaunts1 },
    { 2, 0, 299, 207, 0, 0, { 202, 201, 0, 0 }, 0, GAME_CONFIG_LANGUAGE_FILTER_KEY, 0, 0, &gPreferencesLanguageFilter1 },
    { 2, 0, 299, 271, 0, 0, { 209, 219, 0, 0 }, 0, GAME_CONFIG_RUNNING_KEY, 0, 0, &gPreferencesRunning1 },
    { 2, 0, 299, 338, 0, 0, { 202, 201, 0, 0 }, 0, GAME_CONFIG_SUBTITLES_KEY, 0, 0, &gPreferencesSubtitles1 },
    { 2, 0, 299, 404, 0, 0, { 202, 201, 0, 0 }, 0, GAME_CONFIG_ITEM_HIGHLIGHT_KEY, 0, 0, &gPreferencesItemHighlight1 },
    { 2, 0, 374, 50, 0, 0, { 207, 210, 0, 0 }, 0, GAME_CONFIG_COMBAT_SPEED_KEY, 0.0, 50.0, &gPreferencesCombatSpeed1 },
    { 3, 0, 374, 125, 0, 0, { 217, 209, 218, 0 }, 0, GAME_CONFIG_TEXT_BASE_DELAY_KEY, 1.0, 6.0, nullptr },
    { 4, 0, 374, 196, 0, 0, { 202, 221, 209, 222 }, 0, GAME_CONFIG_MASTER_VOLUME_KEY, 0, 32767.0, &gPreferencesMasterVolume1 },
    { 4, 0, 374, 247, 0, 0, { 202, 221, 209, 222 }, 0, GAME_CONFIG_MUSIC_VOLUME_KEY, 0, 32767.0, &gPreferencesMusicVolume1 },
    { 4, 0, 374, 298, 0, 0, { 202, 221, 209, 222 }, 0, GAME_CONFIG_SNDFX_VOLUME_KEY, 0, 32767.0, &gPreferencesSoundEffectsVolume1 },
    { 4, 0, 374, 349, 0, 0, { 202, 221, 209, 222 }, 0, GAME_CONFIG_SPEECH_VOLUME_KEY, 0, 32767.0, &gPreferencesSpeechVolume1 },
    { 2, 0, 374, 400, 0, 0, { 207, 223, 0, 0 }, 0, GAME_CONFIG_BRIGHTNESS_KEY, 1.0, 1.17999267578125, nullptr },
    { 2, 0, 374, 451, 0, 0, { 207, 218, 0, 0 }, 0, GAME_CONFIG_MOUSE_SENSITIVITY_KEY, 1.0, 2.5, nullptr },
};

const PreferencesOffsets gPreferencesOffsets640 = {
    // Window dimensions
    640, // width
    480, // height

    // Primary preferences (left column)
    76, // primaryColumnX
    76, // primaryKnobX
    { 48, 125, 203, 286, 363 }, // primaryKnobY[5]
    { 66, 143, 222, 304, 382 }, // primaryLabelY[5]

    // Secondary preferences (middle column)
    299, // secondaryColumnX
    299, // secondaryKnobX
    { 49, 116, 181, 247, 313, 380 }, // secondaryKnobY[6]
    { 66, 133, 200, 264, 331, 397 }, // secondaryLabelY[6]

    // Range preferences (right column)
    374, // rangeColumnX
    374, // rangeKnobX
    { 19, 94, 165, 216, 268, 319, 369, 420 }, // rangeKnobY[8]

    // Label positions
    99, // primLabelColX
    206, // secLabelColX
    384, // rangLabelColX
    { 23, 23, 23, 23, 23 }, // labelX[5]
    { 251, 251, 251, 251, 251, 251 }, // secondaryLabelX[6]

    // Range control parameters
    384, // rangeStartX
    219, // rangeWidth
    21, // knobWidth
    { 384, 504, 564, 624, 444 }, // rangeLabelX[5]

    // Blit dimensions
    160, // primaryBlitWidth
    54, // primaryBlitHeight
    113, // secondaryBlitWidth
    34, // secondaryBlitHeight
    240, // rangeBlitWidth
    24, // rangeBlitHeight

    // Title and buttons
    74, // titleTextX
    10, // titleTextY
    43, // defaultLabelX
    449, // defaultLabelY
    169, // doneLabelX
    449, // doneLabelY
    283, // cancelLabelX
    449, // cancelLabelY
    72, // speedLabelX
    405, // speedLabelY

    // Button positions
    23, // defaultButtonX
    450, // defaultButtonY
    148, // doneButtonX
    450, // doneButtonY
    263, // cancelButtonX
    450, // cancelButtonY

    // Checkbox position
    383, // playerSpeedCheckboxX
    68, // playerSpeedCheckboxY

    // Knob hit detection offsets
    23, // primaryKnobHitX
    21, // primaryKnobHitY
    11, // secondaryKnobHitX
    12, // secondaryKnobHitY

    // Range slider parameters
    384, // rangeSliderMinX
    603, // rangeSliderMaxX
    219, // rangeSliderWidth

    // Button hitbox offsets
    -4, // primaryButtonOffsetY
    -5, // secondaryButtonOffsetY
    -12, // rangeButtonOffsetY

    // Text delay and range label positions
    43.8, // textBaseDelayScale (double)
    444, // rangeLabel4Option1X
    564, // rangeLabel4Option2X

    // Position arrays
    { 48, 125, 203, 286, 363 }, // row1Ytab[5]
    { 49, 116, 181, 247, 313, 380 }, // row2Ytab[6]
    { 19, 94, 165, 216, 268, 319, 369, 420 }, // row3Ytab[8]
    { 2, 25, 46, 46 }, // optionXOffsets[4]
    { 10, -4, 10, 31 }, // optionYOffsets[4]
    { 4, 21 }, // secondaryOptionXOffsets[2]
    { 66, 143, 222, 304, 382 }, // primaryLabelYValues[5]
    { 66, 133, 200, 264, 331, 397 }, // secondaryLabelYValues[6]

    // Preference positions
    {
        Point { 76, 71 }, // PREF_GAME_DIFFICULTY
        Point { 76, 149 }, // PREF_COMBAT_DIFFICULTY
        Point { 76, 226 }, // PREF_VIOLENCE_LEVEL
        Point { 76, 309 }, // PREF_TARGET_HIGHLIGHT
        Point { 76, 387 }, // PREF_COMBAT_LOOKS
        Point { 299, 74 }, // PREF_COMBAT_MESSAGES
        Point { 299, 141 }, // PREF_COMBAT_TAUNTS
        Point { 299, 207 }, // PREF_LANGUAGE_FILTER
        Point { 299, 271 }, // PREF_RUNNING
        Point { 299, 338 }, // PREF_SUBTITLES
        Point { 299, 404 }, // PREF_ITEM_HIGHLIGHT
        Point { 374, 50 }, // PREF_COMBAT_SPEED
        Point { 374, 125 }, // PREF_TEXT_BASE_DELAY
        Point { 374, 196 }, // PREF_MASTER_VOLUME
        Point { 374, 247 }, // PREF_MUSIC_VOLUME
        Point { 374, 298 }, // PREF_SFX_VOLUME
        Point { 374, 349 }, // PREF_SPEECH_VOLUME
        Point { 374, 400 }, // PREF_BRIGHTNESS
        Point { 374, 451 } // PREF_MOUSE_SENSITIVIY
    },

    // New offsets
    9, // primaryButtonMinXOffset
    37, // primaryButtonMaxXOffset
    22, // secondaryButtonXOffset
    6, // rangeThumbLeftOffset
    14, // rangeThumbRightOffset
    219.0 // rangeSliderScale (double)
};

const PreferencesOffsets gPreferencesOffsets800 = {
    // Window dimensions
    800, // width
    500, // height

    // Primary preferences (left column)
    100, // primaryColumnX
    100, // primaryKnobX
    { 50, 131, 211, 299, 380 }, // primaryKnobY[5]
    { 70, 151, 234, 320, 402 }, // primaryLabelY[5]

    // Secondary preferences (middle column)
    380, // secondaryColumnX
    380, // secondaryKnobX
    { 50, 119, 187, 256, 325, 395 }, // secondaryKnobY[6]
    { 71, 139, 209, 277, 347, 415 }, // secondaryLabelY[6]

    // Range preferences (right column)
    468, // rangeColumnX
    468, // rangeKnobX
    { 20, 99, 172, 225, 279, 332, 384, 437 }, // rangeKnobY[8]

    // Label positions
    124, // primLabelColX
    255, // secLabelColX
    479, // rangLabelColX
    { 23, 23, 23, 23, 23 }, // labelX[5]
    { 251, 251, 251, 251, 251, 251 }, // secondaryLabelX[6]

    // Range control parameters
    480, // rangeStartX
    274, // rangeWidth
    21, // knobWidth
    { 480, 630, 705, 780, 555 }, // rangeLabelX[5]

    // Blit dimensions
    160, // primaryBlitWidth
    54, // primaryBlitHeight
    113, // secondaryBlitWidth
    34, // secondaryBlitHeight
    300, // rangeBlitWidth
    24, // rangeBlitHeight

    // Title and buttons
    110, // titleTextX
    10, // titleTextY
    54, // defaultLabelX
    468, // defaultLabelY
    211, // doneLabelX
    468, // doneLabelY
    354, // cancelLabelX
    468, // cancelLabelY
    72, // speedLabelX
    506, // speedLabelY

    // Button positions
    29, // defaultButtonX
    469, // defaultButtonY
    185, // doneButtonX
    469, // doneButtonY
    329, // cancelButtonX
    469, // cancelButtonY

    // Checkbox position
    479, // playerSpeedCheckboxX
    68, // playerSpeedCheckboxY

    // Knob hit detection offsets
    23, // primaryKnobHitX
    21, // primaryKnobHitY
    11, // secondaryKnobHitX
    12, // secondaryKnobHitY

    // Range slider parameters
    480, // rangeSliderMinX
    754, // rangeSliderMaxX
    274, // rangeSliderWidth

    // Button hitbox offsets
    -4, // primaryButtonOffsetY
    -5, // secondaryButtonOffsetY
    -12, // rangeButtonOffsetY

    // Text delay and range label positions
    54.8, // textBaseDelayScale (double)
    555, // rangeLabel4Option1X
    705, // rangeLabel4Option2X

    // Position arrays
    { 50, 131, 211, 299, 380 }, // row1Ytab[5]
    { 50, 119, 187, 256, 325, 395 }, // row2Ytab[6]
    { 20, 99, 172, 225, 279, 332, 384, 437 }, // row3Ytab[8]
    { 2, 25, 46, 46 }, // optionXOffsets[4]
    { 10, -4, 10, 31 }, // optionYOffsets[4]
    { 4, 21 }, // secondaryOptionXOffsets[2]
    { 70, 151, 234, 320, 402 }, // primaryLabelYValues[5]
    { 71, 139, 209, 277, 347, 415 }, // secondaryLabelYValues[6]

    // Preference positions
    {
        Point { 100, 74 }, // PREF_GAME_DIFFICULTY
        Point { 100, 157 }, // PREF_COMBAT_DIFFICULTY
        Point { 100, 237 }, // PREF_VIOLENCE_LEVEL
        Point { 100, 324 }, // PREF_TARGET_HIGHLIGHT
        Point { 100, 406 }, // PREF_COMBAT_LOOKS
        Point { 380, 76 }, // PREF_COMBAT_MESSAGES
        Point { 380, 147 }, // PREF_COMBAT_TAUNTS
        Point { 380, 216 }, // PREF_LANGUAGE_FILTER
        Point { 380, 284 }, // PREF_RUNNING
        Point { 380, 354 }, // PREF_SUBTITLES
        Point { 380, 423 }, // PREF_ITEM_HIGHLIGHT
        Point { 468, 53 }, // PREF_COMBAT_SPEED
        Point { 468, 131 }, // PREF_TEXT_BASE_DELAY
        Point { 468, 205 }, // PREF_MASTER_VOLUME
        Point { 468, 258 }, // PREF_MUSIC_VOLUME
        Point { 468, 311 }, // PREF_SFX_VOLUME
        Point { 468, 364 }, // PREF_SPEECH_VOLUME
        Point { 468, 417 }, // PREF_BRIGHTNESS
        Point { 468, 470 } // PREF_MOUSE_SENSITIVIY
    },

    // New offsets (AFTER preferencePositions)
    9, // primaryButtonMinXOffset
    37, // primaryButtonMaxXOffset
    22, // secondaryButtonXOffset
    6, // rangeThumbLeftOffset
    14, // rangeThumbRightOffset
    274.0 // rangeSliderScale (double)
};

static FrmImage _preferencesFrmImages[PREFERENCES_WINDOW_FRM_COUNT];
static int _oldFont;

bool preferencesLoadOffsetsFromConfig(PreferencesOffsets* offsets, bool isWidescreen)
{
    const char* section = isWidescreen ? "preferences800" : "preferences640";
    const PreferencesOffsets* fallback = isWidescreen ? &gPreferencesOffsets800 : &gPreferencesOffsets640;

    // Initialize with fallback values
    *offsets = *fallback;

    // Window
    configGetInt(&gGameConfig, section, "width", &offsets->width);
    configGetInt(&gGameConfig, section, "height", &offsets->height);

    // Primary preferences
    configGetInt(&gGameConfig, section, "primaryColumnX", &offsets->primaryColumnX);
    configGetInt(&gGameConfig, section, "primaryKnobX", &offsets->primaryKnobX);
    configGetIntArray(&gGameConfig, section, "primaryKnobY", offsets->primaryKnobY, PRIMARY_PREF_COUNT);
    configGetIntArray(&gGameConfig, section, "primaryLabelY", offsets->primaryLabelY, PRIMARY_PREF_COUNT);

    // Secondary preferences
    configGetInt(&gGameConfig, section, "secondaryColumnX", &offsets->secondaryColumnX);
    configGetInt(&gGameConfig, section, "secondaryKnobX", &offsets->secondaryKnobX);
    configGetIntArray(&gGameConfig, section, "secondaryKnobY", offsets->secondaryKnobY, SECONDARY_PREF_COUNT);
    configGetIntArray(&gGameConfig, section, "secondaryLabelY", offsets->secondaryLabelY, SECONDARY_PREF_COUNT);

    // Range preferences
    configGetInt(&gGameConfig, section, "rangeColumnX", &offsets->rangeColumnX);
    configGetInt(&gGameConfig, section, "rangeKnobX", &offsets->rangeKnobX);
    configGetIntArray(&gGameConfig, section, "rangeKnobY", offsets->rangeKnobY, RANGE_PREF_COUNT);

    // Label positions
    configGetInt(&gGameConfig, section, "primLabelColX", &offsets->primLabelColX);
    configGetInt(&gGameConfig, section, "secLabelColX", &offsets->secLabelColX);
    configGetInt(&gGameConfig, section, "rangLabelColX", &offsets->rangLabelColX);
    configGetIntArray(&gGameConfig, section, "labelX", offsets->labelX, PRIMARY_PREF_COUNT);
    configGetIntArray(&gGameConfig, section, "secondaryLabelX", offsets->secondaryLabelX, SECONDARY_PREF_COUNT);

    // Range control
    configGetInt(&gGameConfig, section, "rangeStartX", &offsets->rangeStartX);
    configGetInt(&gGameConfig, section, "rangeWidth", &offsets->rangeWidth);
    configGetInt(&gGameConfig, section, "knobWidth", &offsets->knobWidth);
    configGetIntArray(&gGameConfig, section, "rangeLabelX", offsets->rangeLabelX, 4);

    // Blit dimensions
    configGetInt(&gGameConfig, section, "primaryBlitWidth", &offsets->primaryBlitWidth);
    configGetInt(&gGameConfig, section, "primaryBlitHeight", &offsets->primaryBlitHeight);
    configGetInt(&gGameConfig, section, "secondaryBlitWidth", &offsets->secondaryBlitWidth);
    configGetInt(&gGameConfig, section, "secondaryBlitHeight", &offsets->secondaryBlitHeight);
    configGetInt(&gGameConfig, section, "rangeBlitWidth", &offsets->rangeBlitWidth);
    configGetInt(&gGameConfig, section, "rangeBlitHeight", &offsets->rangeBlitHeight);

    // Title and buttons
    configGetInt(&gGameConfig, section, "titleTextX", &offsets->titleTextX);
    configGetInt(&gGameConfig, section, "titleTextY", &offsets->titleTextY);
    configGetInt(&gGameConfig, section, "defaultLabelX", &offsets->defaultLabelX);
    configGetInt(&gGameConfig, section, "defaultLabelY", &offsets->defaultLabelY);
    configGetInt(&gGameConfig, section, "doneLabelX", &offsets->doneLabelX);
    configGetInt(&gGameConfig, section, "doneLabelY", &offsets->doneLabelY);
    configGetInt(&gGameConfig, section, "cancelLabelX", &offsets->cancelLabelX);
    configGetInt(&gGameConfig, section, "cancelLabelY", &offsets->cancelLabelY);
    configGetInt(&gGameConfig, section, "speedLabelX", &offsets->speedLabelX);
    configGetInt(&gGameConfig, section, "speedLabelY", &offsets->speedLabelY);
    configGetInt(&gGameConfig, section, "defaultButtonX", &offsets->defaultButtonX);
    configGetInt(&gGameConfig, section, "defaultButtonY", &offsets->defaultButtonY);
    configGetInt(&gGameConfig, section, "doneButtonX", &offsets->doneButtonX);
    configGetInt(&gGameConfig, section, "doneButtonY", &offsets->doneButtonY);
    configGetInt(&gGameConfig, section, "cancelButtonX", &offsets->cancelButtonX);
    configGetInt(&gGameConfig, section, "cancelButtonY", &offsets->cancelButtonY);
    configGetInt(&gGameConfig, section, "playerSpeedCheckboxX", &offsets->playerSpeedCheckboxX);
    configGetInt(&gGameConfig, section, "playerSpeedCheckboxY", &offsets->playerSpeedCheckboxY);

    // Hit detection
    configGetInt(&gGameConfig, section, "primaryKnobHitX", &offsets->primaryKnobHitX);
    configGetInt(&gGameConfig, section, "primaryKnobHitY", &offsets->primaryKnobHitY);
    configGetInt(&gGameConfig, section, "secondaryKnobHitX", &offsets->secondaryKnobHitX);
    configGetInt(&gGameConfig, section, "secondaryKnobHitY", &offsets->secondaryKnobHitY);
    configGetInt(&gGameConfig, section, "rangeSliderMinX", &offsets->rangeSliderMinX);
    configGetInt(&gGameConfig, section, "rangeSliderMaxX", &offsets->rangeSliderMaxX);
    configGetInt(&gGameConfig, section, "rangeSliderWidth", &offsets->rangeSliderWidth);
    configGetInt(&gGameConfig, section, "primaryButtonOffsetY", &offsets->primaryButtonOffsetY);
    configGetInt(&gGameConfig, section, "secondaryButtonOffsetY", &offsets->secondaryButtonOffsetY);
    configGetInt(&gGameConfig, section, "rangeButtonOffsetY", &offsets->rangeButtonOffsetY);

    configGetDouble(&gGameConfig, section, "textBaseDelayScale", &offsets->textBaseDelayScale);
    configGetInt(&gGameConfig, section, "rangeLabel4Option1X", &offsets->rangeLabel4Option1X);
    configGetInt(&gGameConfig, section, "rangeLabel4Option2X", &offsets->rangeLabel4Option2X);

    configGetIntArray(&gGameConfig, section, "row1Ytab", offsets->row1Ytab, PRIMARY_PREF_COUNT);
    configGetIntArray(&gGameConfig, section, "row2Ytab", offsets->row2Ytab, SECONDARY_PREF_COUNT);
    configGetIntArray(&gGameConfig, section, "row3Ytab", offsets->row3Ytab, RANGE_PREF_COUNT);

    configGetIntArray(&gGameConfig, section, "optionXOffsets", offsets->optionXOffsets, 4);
    configGetIntArray(&gGameConfig, section, "optionYOffsets", offsets->optionYOffsets, 4);
    configGetIntArray(&gGameConfig, section, "secondaryOptionXOffsets", offsets->secondaryOptionXOffsets, 2);

    configGetIntArray(&gGameConfig, section, "primaryLabelYValues", offsets->primaryLabelYValues, PRIMARY_PREF_COUNT);
    configGetIntArray(&gGameConfig, section, "secondaryLabelYValues", offsets->secondaryLabelYValues, SECONDARY_PREF_COUNT);

    configGetInt(&gGameConfig, section, "primaryButtonMinXOffset", &offsets->primaryButtonMinXOffset);
    configGetInt(&gGameConfig, section, "primaryButtonMaxXOffset", &offsets->primaryButtonMaxXOffset);
    configGetInt(&gGameConfig, section, "secondaryButtonXOffset", &offsets->secondaryButtonXOffset);
    configGetInt(&gGameConfig, section, "rangeThumbLeftOffset", &offsets->rangeThumbLeftOffset);
    configGetInt(&gGameConfig, section, "rangeThumbRightOffset", &offsets->rangeThumbRightOffset);
    configGetDouble(&gGameConfig, section, "rangeSliderScale", &offsets->rangeSliderScale);

    // Load preference positions
    for (int i = 0; i < PREF_COUNT; i++) {
        char key[64];
        snprintf(key, sizeof(key), "preferencePositions%dX", i);
        configGetInt(&gGameConfig, section, key, &offsets->preferencePositions[i].x);
        snprintf(key, sizeof(key), "preferencePositions%dY", i);
        configGetInt(&gGameConfig, section, key, &offsets->preferencePositions[i].y);
    }

    return true;
}

void preferencesWriteDefaultOffsetsToConfig(bool isWidescreen, const PreferencesOffsets* defaults)
{
    const char* section = isWidescreen ? "preferences800" : "preferences640";

    // Window
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

    // Range preferences
    configSetInt(&gGameConfig, section, "rangeColumnX", defaults->rangeColumnX);
    configSetInt(&gGameConfig, section, "rangeKnobX", defaults->rangeKnobX);
    configSetIntArray(&gGameConfig, section, "rangeKnobY", defaults->rangeKnobY, RANGE_PREF_COUNT);

    // Label positions
    configSetInt(&gGameConfig, section, "primLabelColX", defaults->primLabelColX);
    configSetInt(&gGameConfig, section, "secLabelColX", defaults->secLabelColX);
    configSetInt(&gGameConfig, section, "rangLabelColX", defaults->rangLabelColX);
    configSetIntArray(&gGameConfig, section, "labelX", defaults->labelX, PRIMARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "secondaryLabelX", defaults->secondaryLabelX, SECONDARY_PREF_COUNT);

    // Range control
    configSetInt(&gGameConfig, section, "rangeStartX", defaults->rangeStartX);
    configSetInt(&gGameConfig, section, "rangeWidth", defaults->rangeWidth);
    configSetInt(&gGameConfig, section, "knobWidth", defaults->knobWidth);
    configSetIntArray(&gGameConfig, section, "rangeLabelX", defaults->rangeLabelX, 4);

    // Blit dimensions
    configSetInt(&gGameConfig, section, "primaryBlitWidth", defaults->primaryBlitWidth);
    configSetInt(&gGameConfig, section, "primaryBlitHeight", defaults->primaryBlitHeight);
    configSetInt(&gGameConfig, section, "secondaryBlitWidth", defaults->secondaryBlitWidth);
    configSetInt(&gGameConfig, section, "secondaryBlitHeight", defaults->secondaryBlitHeight);
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
    configSetInt(&gGameConfig, section, "rangeSliderMinX", defaults->rangeSliderMinX);
    configSetInt(&gGameConfig, section, "rangeSliderMaxX", defaults->rangeSliderMaxX);
    configSetInt(&gGameConfig, section, "rangeSliderWidth", defaults->rangeSliderWidth);
    configSetInt(&gGameConfig, section, "primaryButtonOffsetY", defaults->primaryButtonOffsetY);
    configSetInt(&gGameConfig, section, "secondaryButtonOffsetY", defaults->secondaryButtonOffsetY);
    configSetInt(&gGameConfig, section, "rangeButtonOffsetY", defaults->rangeButtonOffsetY);

    configSetDouble(&gGameConfig, section, "textBaseDelayScale", defaults->textBaseDelayScale);
    configSetInt(&gGameConfig, section, "rangeLabel4Option1X", defaults->rangeLabel4Option1X);
    configSetInt(&gGameConfig, section, "rangeLabel4Option2X", defaults->rangeLabel4Option2X);

    configSetIntArray(&gGameConfig, section, "row1Ytab", defaults->row1Ytab, PRIMARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "row2Ytab", defaults->row2Ytab, SECONDARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "row3Ytab", defaults->row3Ytab, RANGE_PREF_COUNT);

    configSetIntArray(&gGameConfig, section, "optionXOffsets", defaults->optionXOffsets, 4);
    configSetIntArray(&gGameConfig, section, "optionYOffsets", defaults->optionYOffsets, 4);
    configSetIntArray(&gGameConfig, section, "secondaryOptionXOffsets", defaults->secondaryOptionXOffsets, 2);

    configSetIntArray(&gGameConfig, section, "primaryLabelYValues", defaults->primaryLabelYValues, PRIMARY_PREF_COUNT);
    configSetIntArray(&gGameConfig, section, "secondaryLabelYValues", defaults->secondaryLabelYValues, SECONDARY_PREF_COUNT);

    configSetInt(&gGameConfig, section, "primaryButtonMinXOffset", defaults->primaryButtonMinXOffset);
    configSetInt(&gGameConfig, section, "primaryButtonMaxXOffset", defaults->primaryButtonMaxXOffset);
    configSetInt(&gGameConfig, section, "secondaryButtonXOffset", defaults->secondaryButtonXOffset);
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

void applyWidescreenPreference(bool widescreen) {
    // 1) Compute and clamp your desired resolution
    int newWidth  = widescreen ? std::max(settings.graphics.game_width, 800) : 640;
    int newHeight = widescreen ? std::max(settings.graphics.game_height, 500) : 480;

    // 2) Sync into settings & .cfg
    settings.graphics.game_width  = newWidth;
    settings.graphics.game_height = newHeight;
    //settings.graphics.widescreen  = widescreen;

    configSetInt (&gGameConfig, "graphics", "game_width",  newWidth);
    configSetInt (&gGameConfig, "graphics", "game_height", newHeight);
    //configSetBool(&gGameConfig, "graphics", "widescreen", widescreen);
    gameConfigSave();

    // 3) Rebuild everything
    backgroundSoundPause();
    handleWindowSizeChanged();
    backgroundSoundResume();
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

    // Set widescreen - must be wider in both axis and set to widescreen
    const bool isWidescreen = gameIsWidescreen();

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
    gPreferencesWidescreen1 = settings.graphics.widescreen;
    //gPreferencesCombatLooks1 = settings.preferences.combat_looks;
    gPreferencesCombatTaunts1 = settings.preferences.combat_taunts;
    gPreferencesLanguageFilter1 = settings.preferences.language_filter;
    gPreferencesRunning1 = settings.preferences.running;
    gPreferencesSubtitles1 = settings.preferences.subtitles;
    gPreferencesItemHighlight1 = settings.preferences.item_highlight;
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
    gPreferencesWidescreen2 = gPreferencesWidescreen1;
    //gPreferencesCombatLooks2 = gPreferencesCombatLooks1;
    gPreferencesCombatMessages2 = gPreferencesCombatMessages1;
    gPreferencesCombatTaunts2 = gPreferencesCombatTaunts1;
    gPreferencesLanguageFilter2 = gPreferencesLanguageFilter1;
    gPreferencesRunning2 = gPreferencesRunning1;
    gPreferencesSubtitles2 = gPreferencesSubtitles1;
    gPreferencesItemHighlight2 = gPreferencesItemHighlight1;
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
    gPreferencesWidescreen1 = gPreferencesWidescreen2;
    //gPreferencesCombatLooks1 = gPreferencesCombatLooks2;
    gPreferencesCombatMessages1 = gPreferencesCombatMessages2;
    gPreferencesCombatTaunts1 = gPreferencesCombatTaunts2;
    gPreferencesLanguageFilter1 = gPreferencesLanguageFilter2;
    gPreferencesRunning1 = gPreferencesRunning2;
    gPreferencesSubtitles1 = gPreferencesSubtitles2;
    gPreferencesItemHighlight1 = gPreferencesItemHighlight2;
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
    gPreferencesCombatDifficulty1 = COMBAT_DIFFICULTY_NORMAL;
    gPreferencesViolenceLevel1 = VIOLENCE_LEVEL_MAXIMUM_BLOOD;
    gPreferencesTargetHighlight1 = TARGET_HIGHLIGHT_TARGETING_ONLY;
    gPreferencesCombatMessages1 = 1;
    gPreferencesWidescreen1 = 0;
    //gPreferencesCombatLooks1 = 0;
    gPreferencesCombatTaunts1 = 1;
    gPreferencesRunning1 = 0;
    gPreferencesSubtitles1 = 0;
    gPreferencesItemHighlight1 = 1;
    gPreferencesCombatSpeed1 = 0;
    gPreferencesPlayerSpeedup1 = 0;
    gPreferencesTextBaseDelay1 = 3.5;
    gPreferencesBrightness1 = 1.0;
    gPreferencesMouseSensitivity1 = 1.0;
    gPreferencesGameDifficulty1 = 1;
    gPreferencesLanguageFilter1 = 0;
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
    gPreferencesWidescreen1 = std::clamp(gPreferencesWidescreen1, 0, 1);
    //gPreferencesCombatLooks1 = std::clamp(gPreferencesCombatLooks1, 0, 1);
    gPreferencesCombatTaunts1 = std::clamp(gPreferencesCombatTaunts1, 0, 1);
    gPreferencesLanguageFilter1 = std::clamp(gPreferencesLanguageFilter1, 0, 1);
    gPreferencesRunning1 = std::clamp(gPreferencesRunning1, 0, 1);
    gPreferencesSubtitles1 = std::clamp(gPreferencesSubtitles1, 0, 1);
    gPreferencesItemHighlight1 = std::clamp(gPreferencesItemHighlight1, 0, 1);
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
    //applyWidescreenPreference(gPreferencesWidescreen1);
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
                meta->maxX = x;
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
    settings.graphics.widescreen = gPreferencesWidescreen1;
    //settings.preferences.combat_looks = gPreferencesCombatLooks1;
    settings.preferences.combat_taunts = gPreferencesCombatTaunts1;
    settings.preferences.language_filter = gPreferencesLanguageFilter1;
    settings.preferences.running = gPreferencesRunning1;
    settings.preferences.subtitles = gPreferencesSubtitles1;
    settings.preferences.item_highlight = gPreferencesItemHighlight1;
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
    if (fileWriteInt32(stream, gPreferencesWidescreen1) == -1)
    //if (fileWriteInt32(stream, gPreferencesCombatLooks1) == -1)
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
    if (fileReadInt32(stream, &gPreferencesWidescreen1) == -1)
    //if (fileReadInt32(stream, &gPreferencesCombatLooks1) == -1)
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
    int btn;

    if (!messageListInit(&gPreferencesMessageList)) {
        return -1;
    }

    // Determine screen mode and load offsets
    const bool isWidescreen = gameIsWidescreen();

    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s%s", asc_5186C8, "options.msg");
    if (!messageListLoad(&gPreferencesMessageList, path)) {
        return -1;
    }

    _oldFont = fontGetCurrent();

    _SaveSettings();

    for (i = 0; i < PREFERENCES_WINDOW_FRM_COUNT; i++) {
        // Use widescreen variants when available
        int fid = artGetFidWithVariant(OBJ_TYPE_INTERFACE, gPreferencesWindowFrmIds[i], "_800", isWidescreen);

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
    for (i = 0; i < PRIMARY_PREF_COUNT; i++) {
        messageItemText = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, messageItemId++);
        x = gOffsets.primLabelColX - fontGetStringWidth(messageItemText) / 2;
        fontDrawText(gPreferencesWindowBuffer + gOffsets.width * gOffsets.row1Ytab[i] + x, messageItemText, gOffsets.width, gOffsets.width, _colorTable[18979]);
    }

    for (i = 0; i < SECONDARY_PREF_COUNT; i++) {
        messageItemText = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, messageItemId++);
        fontDrawText(gPreferencesWindowBuffer + gOffsets.width * gOffsets.row2Ytab[i] + gOffsets.secLabelColX, messageItemText, gOffsets.width, gOffsets.width, _colorTable[18979]);
    }

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

    // Affect player speed
    fontSetCurrent(101);
    messageItemText = getmsg(&gPreferencesMessageList, &gPreferencesMessageListItem, 122);
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
            x = gOffsets.rangeStartX;
            y = knobY + gOffsets.rangeButtonOffsetY;
            width = gOffsets.rangeBlitWidth;
            height = 23;
            mouseEnterEventCode = 526;
            mouseExitEventCode = 526;
            mouseDownEventCode = 505 + i;
            mouseUpEventCode = 526;
        } else if (i >= FIRST_SECONDARY_PREF) {
            // Secondary preferences (toggle buttons)
            // Use offset-based values instead of gPreferenceDescriptions
            x = knobX + gOffsets.secondaryOptionXOffsets[0];
            y = knobY + gOffsets.secondaryButtonOffsetY;
            width = (knobX + gOffsets.secondaryOptionXOffsets[1]) - x;
            height = 28;
            mouseEnterEventCode = -1;
            mouseExitEventCode = -1;
            mouseDownEventCode = -1;
            mouseUpEventCode = 505 + i;
        } else {
            // Primary preferences (multi-option knobs)
            // Use offset-based values instead of gPreferenceDescriptions
            x = knobX + gOffsets.optionXOffsets[0];
            y = knobY + gOffsets.primaryButtonOffsetY;
            width = (knobX + gOffsets.optionXOffsets[3]) - x;
            height = 48;
            mouseEnterEventCode = -1;
            mouseExitEventCode = -1;
            mouseDownEventCode = -1;
            mouseUpEventCode = 505 + i;
        }

        gPreferenceDescriptions[i].btn = buttonCreate(gPreferencesWindow,
            x, y, width, height,
            mouseEnterEventCode, mouseExitEventCode,
            mouseDownEventCode, mouseUpEventCode,
            nullptr, nullptr, nullptr, 32);
    }

    // Player Speed Checkbox
    _plyrspdbid = buttonCreate(gPreferencesWindow,
        gOffsets.playerSpeedCheckboxX,
        gOffsets.playerSpeedCheckboxY,
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_CHECKBOX_OFF].getWidth(),
        _preferencesFrmImages[PREFERENCES_WINDOW_FRM_CHECKBOX_ON].getHeight(),
        -1,
        -1,
        524,
        524,
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
        527,
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
        528,
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
            rc = 1;
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
        case 527:
            preferencesSetDefaults(true);
            break;
        default:
            if (eventCode == KEY_ESCAPE || eventCode == 528 || _game_user_wants_to_quit != 0) {
                _RestoreSettings();
                rc = 0;
            } else if (eventCode >= 505 && eventCode <= 524) {
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
            *valuePtr ^= 1;
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
    } else if (preferenceIndex == 19) {
        gPreferencesPlayerSpeedup1 ^= 1;
    }

    _changed = true;
}

} // namespace fallout
