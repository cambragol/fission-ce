#ifndef FALLOUT_PREFERENCES_H_
#define FALLOUT_PREFERENCES_H_

#include "db.h"
#include "geometry.h"

namespace fallout {

// Preference IDs
typedef enum Preference {
    // Primary preferences (5)
    PREF_GAME_DIFFICULTY,
    PREF_COMBAT_DIFFICULTY,
    PREF_VIOLENCE_LEVEL,
    PREF_TARGET_HIGHLIGHT,
    PREF_COMBAT_LOOKS,

    // Secondary preferences (6)
    PREF_COMBAT_MESSAGES,
    PREF_COMBAT_TAUNTS,
    PREF_LANGUAGE_FILTER,
    PREF_RUNNING,
    PREF_SUBTITLES,
    PREF_ITEM_HIGHLIGHT,

    // Range preferences (8)
    PREF_COMBAT_SPEED,
    PREF_TEXT_BASE_DELAY,
    PREF_MASTER_VOLUME,
    PREF_MUSIC_VOLUME,
    PREF_SFX_VOLUME,
    PREF_SPEECH_VOLUME,
    PREF_BRIGHTNESS,
    PREF_MOUSE_SENSITIVIY,

    // Special cases
    PREF_PLAYER_SPEEDUP = 19, // The checkbox preference

    // Total count (don't include in count)
    PREF_COUNT
} Preference;

// Constants for preference categories
enum {
    PRIMARY_PREF_COUNT = 5,
    SECONDARY_PREF_COUNT = 6,
    RANGE_PREF_COUNT = 8,

    FIRST_PRIMARY_PREF = PREF_GAME_DIFFICULTY,
    LAST_PRIMARY_PREF = PREF_COMBAT_LOOKS,

    FIRST_SECONDARY_PREF = PREF_COMBAT_MESSAGES,
    LAST_SECONDARY_PREF = PREF_ITEM_HIGHLIGHT,

    FIRST_RANGE_PREF = PREF_COMBAT_SPEED,
    LAST_RANGE_PREF = PREF_MOUSE_SENSITIVIY,

    PRIMARY_OPTION_VALUE_COUNT = 4,
    SECONDARY_OPTION_VALUE_COUNT = 2
};

// FRM IDs
typedef enum PreferencesWindowFrm {
    PREFERENCES_WINDOW_FRM_BACKGROUND,
    PREFERENCES_WINDOW_FRM_KNOB_OFF,
    PREFERENCES_WINDOW_FRM_PRIMARY_SWITCH,
    PREFERENCES_WINDOW_FRM_SECONDARY_SWITCH,
    PREFERENCES_WINDOW_FRM_CHECKBOX_ON,
    PREFERENCES_WINDOW_FRM_CHECKBOX_OFF,
    PREFERENCES_WINDOW_FRM_6,
    PREFERENCES_WINDOW_FRM_KNOB_ON,
    PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_UP,
    PREFERENCES_WINDOW_FRM_LITTLE_RED_BUTTON_DOWN,
    PREFERENCES_WINDOW_FRM_COUNT
} PreferencesWindowFrm;

// Preference description structure
typedef struct PreferenceDescription {
    short valuesCount;
    short direction;
    short knobX;
    short knobY;
    short minX;
    short maxX;
    short labelIds[PRIMARY_OPTION_VALUE_COUNT];
    int btn;
    char name[32];
    double minValue;
    double maxValue;
    int* valuePtr;
} PreferenceDescription;

int preferencesInit();
int doPreferences(bool animated);
int preferencesSave(File* stream);
int preferencesLoad(File* stream);
void brightnessIncrease();
void brightnessDecrease();

struct PreferencesOffsets {
    // Window
    int width;
    int height;

    // Primary preferences column (left)
    int primaryColumnX;
    int primaryKnobX;
    int primaryKnobY[5]; // For each primary preference
    int primaryLabelY[5]; // Y positions for labels

    // Secondary preferences column (middle)
    int secondaryColumnX;
    int secondaryKnobX;
    int secondaryKnobY[6]; // For each secondary preference
    int secondaryLabelY[6]; // Y positions for labels

    // Range preferences column (right)
    int rangeColumnX;
    int rangeKnobX;
    int rangeKnobY[8]; // For each range preference

    // Label positions
    int primLabelColX;
    int secLabelColX;
    int rangLabelColX;
    int labelX[PRIMARY_PREF_COUNT];
    int secondaryLabelX[SECONDARY_PREF_COUNT];

    // Range control
    int rangeStartX;
    int rangeWidth;
    int knobWidth;
    int rangeLabelX[5];

    // Blit dimensions
    int primaryBlitWidth;
    int primaryBlitHeight;
    int secondaryBlitWidth;
    int secondaryBlitHeight;
    int rangeBlitWidth;
    int rangeBlitHeight;

    // Title text position
    int titleTextX;
    int titleTextY;

    // Button label positions
    int defaultLabelX;
    int defaultLabelY;
    int doneLabelX;
    int doneLabelY;
    int cancelLabelX;
    int cancelLabelY;
    int speedLabelX;
    int speedLabelY;

    // Button positions
    int defaultButtonX;
    int defaultButtonY;
    int doneButtonX;
    int doneButtonY;
    int cancelButtonX;
    int cancelButtonY;

    // Checkbox position
    int playerSpeedCheckboxX;
    int playerSpeedCheckboxY;

    // Knob hit detection offsets
    int primaryKnobHitX; // +23 from knobX for primary prefs
    int primaryKnobHitY; // +21 from knobY for primary prefs
    int secondaryKnobHitX; // +11 from knobX for secondary prefs
    int secondaryKnobHitY; // +12 from knobY for secondary prefs

    // Range slider parameters
    int rangeSliderMinX; // 384 (640) / 480 (800)
    int rangeSliderMaxX; // 603 (640) / 754 (800)
    int rangeSliderWidth; // 219 (640) / 274 (800)

    // Button hitbox offsets
    int primaryButtonOffsetY; // -4 from knobY
    int secondaryButtonOffsetY; // -5 from knobY
    int rangeButtonOffsetY; // -12 from knobY

    double textBaseDelayScale; // 43.8 for 640, 54.8 for 800
    int rangeLabel4Option1X; // 444 for 640, 555 for 800
    int rangeLabel4Option2X; // 564 for 640, 705 for 800

    // Position arrays
    int row1Ytab[PRIMARY_PREF_COUNT]; // Primary knob Y positions
    int row2Ytab[SECONDARY_PREF_COUNT]; // Secondary knob Y positions
    int row3Ytab[RANGE_PREF_COUNT]; // Range knob Y positions
    int optionXOffsets[PRIMARY_OPTION_VALUE_COUNT]; // Primary option X offsets
    int optionYOffsets[PRIMARY_OPTION_VALUE_COUNT]; // Primary option Y offsets
    int secondaryOptionXOffsets[SECONDARY_OPTION_VALUE_COUNT]; // Secondary option X offsets
    int primaryLabelYValues[PRIMARY_PREF_COUNT]; // Primary label Y positions
    int secondaryLabelYValues[SECONDARY_PREF_COUNT]; // Secondary label Y positions

    Point preferencePositions[PREF_COUNT]; // ONLY x/y pairs

    // New offsets (no duplicates)
    int primaryButtonMinXOffset; // 9
    int primaryButtonMaxXOffset; // 37
    int secondaryButtonXOffset; // 22
    int rangeThumbLeftOffset; // 6
    int rangeThumbRightOffset; // 14
    double rangeSliderScale; // 219.0 or 274.0
};

// Global declarations
extern PreferencesOffsets gCurrentPreferencesOffsets;
extern const PreferencesOffsets gPreferencesOffsets640;
extern const PreferencesOffsets gPreferencesOffsets800;
extern const int _row1Ytab[PRIMARY_PREF_COUNT];
extern const int _row2Ytab[SECONDARY_PREF_COUNT];
extern const int _row3Ytab[RANGE_PREF_COUNT];
extern const short word_48FBF6[PRIMARY_OPTION_VALUE_COUNT];
extern const short word_48FBFE[PRIMARY_OPTION_VALUE_COUNT];
extern const short word_48FC06[SECONDARY_OPTION_VALUE_COUNT];
extern const int dword_48FC1C[PRIMARY_PREF_COUNT];
extern const int dword_48FC30[SECONDARY_PREF_COUNT];

} // namespace fallout

#endif /* FALLOUT_PREFERENCES_H_ */
