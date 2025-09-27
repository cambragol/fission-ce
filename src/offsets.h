// offsets.h

#pragma once
#include "config.h"
#include "game_config.h"
#include "preferences.h"

namespace fallout {

struct MainMenuOffsets {
    // Text
    int copyrightX;
    int copyrightY;
    int versionX;
    int versionY;
    int hashX;
    int hashY;
    int buildDateX;
    int buildDateY;

    // Buttons
    int buttonBaseX;
    int buttonBaseY;
    int buttonTextOffsetX;
    int buttonTextOffsetY;

    // Window
    int width;
    int height;
};

struct CharacterEditorOffsets {
    // Window
    int windowWidth;
    int windowHeight;

    // Buttons
    int nameButtonX;
    int nameButtonY;
    int tagSkillsButtonX;
    int tagSkillsButtonY;
    int printButtonX;
    int printButtonY;
    int doneButtonX;
    int doneButtonY;
    int cancelButtonX;
    int cancelButtonY;
    int optionalTraitsLeftButtonX;
    int optionalTraitsRightButtonX;
    int optionalTraitsButtonY;
    int specialStatsButtonX;

    // Primary stat button positions
    int primaryStatY[7];

    // Adjustment slider
    int skillValueAdjustmentSliderY;

    // Folder view
    int folderViewOffsetY;

    // Karma folder
    int karmaFolderTopLine;

    // Text positions
    int charPointsTextX;
    int charPointsTextY;
    int charPointsValueX;
    int charPointsValueY;
    int optionalTraitsTextX;
    int optionalTraitsTextY;
    int tagSkillsTextX;
    int tagSkillsTextY;

    // Button positions
    int sliderPlusX;
    int sliderPlusY;
    int folderButtonX;
    int folderButtonY;
    int optionsButtonX;
    int optionsButtonY;
    int doneButtonGraphicX;
    int doneButtonGraphicY;
    int cancelButtonGraphicX;
    int cancelButtonGraphicY;

    // Folder view positions
    int folderBackgroundX;
    int folderBackgroundY;
    int folderBackgroundWidth;
    int folderBackgroundHeight;
    int folderSelectedX;
    int folderSelectedY;

    // PC stats positions
    int pcStatsX;
    int pcStatsY;
    int pcStatsWidth;
    int pcStatsHeight;

    // Primary stats positions
    int primaryStatBigNumberX;
    int primaryStatDescriptionX;

    // Derived stats positions
    int derivedStatsTopX;
    int derivedStatsTopY;
    int derivedStatsTopWidth;
    int derivedStatsTopHeight;
    int derivedStatsBottomX;
    int derivedStatsBottomY;
    int derivedStatsBottomWidth;
    int derivedStatsBottomHeight;
    int derivedStatsLabelX;
    int derivedStatsValueX;

    // Skills background
    int skillsBackgroundX;
    int skillsBackgroundWidth;

    // Skills label
    int skillsLabelX;
    int skillsLabelY;

    // Skills points label
    int skillsPointsLabelX;
    int skillsPointsLabelY;

    // Tag skills label
    int tagSkillsLabelX;
    int tagSkillsLabelY;

    // Skills points value
    int skillsPointsValueX;
    int skillsPointsValueY;

    // Skills list positions
    int skillsListStartY;
    int skillsListNameX;
    int skillsListValueX;

    // Slider position
    int skillsListSliderX;

    // Card background
    int cardBackgroundX;
    int cardBackgroundY;
    int cardBackgroundWidth;

    // Name window offset
    int nameWindowOffsetY;

    // Character points value (in stat adjustment)
    int charPointsAdjustX;
    int charPointsAdjustY;

    // Info areas (button regions)
    int statsAreaX;
    int statsAreaY;
    int statsAreaWidth;
    int statsAreaHeight;

    int charPointsAreaX;
    int charPointsAreaY;
    int charPointsAreaWidth;
    int charPointsAreaHeight;

    int optionalTraitsTitleX;
    int optionalTraitsTitleY;
    int optionalTraitsTitleWidth;
    int optionalTraitsTitleHeight;

    int optionalTraitsListX;
    int optionalTraitsListY;
    int optionalTraitsListWidth;
    int optionalTraitsListHeight;

    int pcStatsFolderAreaX;
    int pcStatsFolderAreaY;
    int pcStatsFolderAreaWidth;
    int pcStatsFolderAreaHeight;

    int derivedStatsTopAreaX;
    int derivedStatsTopAreaY;
    int derivedStatsTopAreaWidth;
    int derivedStatsTopAreaHeight;

    int derivedStatsBottomAreaX;
    int derivedStatsBottomAreaY;
    int derivedStatsBottomAreaWidth;
    int derivedStatsBottomAreaHeight;

    int skillsTitleAreaX;
    int skillsTitleAreaY;
    int skillsTitleAreaWidth;
    int skillsTitleAreaHeight;

    int skillsListAreaX;
    int skillsListAreaY;
    int skillsListAreaWidth;
    int skillsListAreaHeight;

    int skillPointsAreaX;
    int skillPointsAreaY;
    int skillPointsAreaWidth;
    int skillPointsAreaHeight;

    // Card display
    int cardImageX;
    int cardImageY;
    int cardTitleX;
    int cardTitleY;
    int cardAttributesOffsetY;
    int cardDividerY;
    int cardDescriptionX;
    int cardDescriptionStartY;

    // Folder button thresholds
    int folderKarmaThresholdX;
    int folderKillsThresholdX;

    // Optional traits
    int optionalTraitsBackgroundX;
    int optionalTraitsBackgroundY;
    int optionalTraitsBackgroundWidth;
    int optionalTraitsLeftColumnX;
    int optionalTraitsRightColumnX;
    int optionalTraitsStartY;

    // New folder view scroll buttons
    int folderScrollUpButtonX;
    int folderScrollUpButtonY;
    int folderScrollDownButtonX;
    int folderScrollDownButtonY;

    // New folder text positions
    int folderTextX;
    int folderKillsNumberX;

    // New folder vertical positions
    int folderViewStartY;

    // Y offset for several areas
    int infoButtonOffsetY;
    int sliderOffsetY;

    int perksTitleX;
    int karmaTitleX;
    int killsTitleX;
};

struct CharacterSelectorOffsets {
    // Window dimensions
    int width;
    int height;

    // Background
    int backgroundX;
    int backgroundY;
    int backgroundWidth;
    int backgroundHeight;

    // Buttons
    int previousButtonX;
    int previousButtonY;
    int nextButtonX;
    int nextButtonY;
    int takeButtonX;
    int takeButtonY;
    int modifyButtonX;
    int modifyButtonY;
    int createButtonX;
    int createButtonY;
    int backButtonX;
    int backButtonY;

    // Text positions
    int nameMidX;
    int primaryStatMidX;
    int secondaryStatMidX;
    int bioX;
    int textBaseY;
    int faceY;

    // Face position
    int faceX;

    // Bio rendering
    int bioMaxY; // 260 in both versions
};

struct LoadSaveOffsets {
    // Window
    int windowWidth;
    int windowHeight;

    // Preview
    int previewWidth;
    int previewHeight;
    int previewX;
    int previewY;
    int previewCoverX;
    int previewCoverY;

    // Title and Text
    int titleTextX;
    int titleTextY;
    int doneLabelX;
    int doneLabelY;
    int cancelLabelX;
    int cancelLabelY;

    // Buttons
    int doneButtonX;
    int doneButtonY;
    int cancelButtonX;
    int cancelButtonY;
    int arrowUpX;
    int arrowUpY;
    int arrowDownX;
    int arrowDownY;

    // Slot List Area
    int slotListAreaX;
    int slotListAreaY;
    int slotListAreaWidth;
    int slotListAreaHeight;

    // Comment Window
    int commentWindowX;
    int commentWindowY;

    // Slot List
    int slotListX;
    int slotListY;
    int slotListWidth;
    int slotListBottomOffset;

    // Info Box
    int infoBoxX;
    int infoBoxY;
    int infoBoxWidth;
    int infoBoxHeight;

    // Info Box Text Positions
    int characterNameX;
    int characterNameY;
    int gameDateX;
    int gameDateY;
    int locationX;
    int locationY;

    int nextPageButtonX;
    int nextPageButtonY;
    int nextPageButtonWidth;
    int nextPageButtonHeight;
    int prevPageButtonX;
    int prevPageButtonY;
    int prevPageButtonWidth;
    int prevPageButtonHeight;

    int infoBoxTextBlockY; // Vertical position for the entire text block

    // Cover image parameters
    int coverWidth; // Source image width (275 for 640, 330 for 800)
    int coverHeight; // Source image height (173 for 640, 188 for 800)
    int coverX; // Blit X position (340 for 640, 421 for 800)
    int coverY; // Blit Y position (39 for 640, 34 for 800)
    int coverPitch; // Source pitch (usually same as width)

    int slotTextPadding; // Spaces inside brackets (0 for 640, 1 for 800)

    // Pagination text positions
    int backTextOffsetX; // X offset from slotListAreaX for BACK text
    int moreTextOffsetX; // X offset from slotListAreaX for MORE text
};

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

    // Tertiary preferences column (middle-right)
    int tertiaryColumnX;
    int tertiaryKnobX;
    int tertiaryKnobY[6]; // For each secondary preference
    int tertiaryLabelY[6]; // Y positions for labels

    // Range preferences column (right)
    int rangeColumnX;
    int rangeKnobX;
    int rangeKnobY[8]; // For each range preference

    // Label positions
    int primLabelColX;
    int secLabelColX;
    int terLabelColX;
    int rangLabelColX;
    int labelX[PRIMARY_PREF_COUNT];
    int secondaryLabelX[SECONDARY_PREF_COUNT];
    int tertiaryLabelX[TERTIARY_PREF_COUNT];
    int quaternarylabelX[QUATERNARY_PREF_COUNT];

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
    int tertiaryBlitWidth;
    int tertiaryBlitHeight;
    int quaternaryBlitWidth;
    int quaternaryBlitHeight;
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
    int tertiaryKnobHitX; // +11 from knobX for secondary prefs
    int tertiaryKnobHitY; // +12 from knobY for secondary prefs
    int quaternaryKnobHitX; // +23 from knobX for quaternary prefs
    int quaternaryKnobHitY; // +21 from knobY for quaternary prefs

    // Range slider parameters
    int rangeSliderMinX; // 384 (640) / 480 (800)
    int rangeSliderMaxX; // 603 (640) / 754 (800)
    int rangeSliderWidth; // 219 (640) / 274 (800)

    // Button hitbox offsets
    int primaryButtonOffsetY; // -4 from knobY
    int secondaryButtonOffsetY; // -5 from knobY
    int tertiaryButtonOffsetY; // -5 from knobY
    int quaternaryButtonOffsetY; // -5 from knobY
    int rangeButtonOffsetY; // -12 from knobY

    double textBaseDelayScale; // 43.8 for 640, 54.8 for 800
    int rangeLabel4Option1X; // 444 for 640, 555 for 800
    int rangeLabel4Option2X; // 564 for 640, 705 for 800

    // Position arrays
    int row1Ytab[PRIMARY_PREF_COUNT]; // Primary knob Y positions
    int row2Ytab[SECONDARY_PREF_COUNT]; // Secondary knob Y positions
    int row2bYtab[TERTIARY_PREF_COUNT]; // Secondary knob Y positions
    int rowdialYtab[QUATERNARY_PREF_COUNT]; // Secondary knob Y positions
    int row3Ytab[RANGE_PREF_COUNT]; // Range knob Y positions
    int optionXOffsets[PRIMARY_OPTION_VALUE_COUNT]; // Primary option X offsets
    int optionYOffsets[PRIMARY_OPTION_VALUE_COUNT]; // Primary option Y offsets
    int secondaryOptionXOffsets[SECONDARY_OPTION_VALUE_COUNT]; // Secondary option X offsets
    int tertiaryOptionYOffsets[TERTIARY_OPTION_VALUE_COUNT]; // tertiary option Y offsets  because text labels are vertically arranged
    int quaternaryXOffsets[QUATERNARY_OPTION_VALUE_COUNT]; // Quaternary option X offsets
    int quaternaryYOffsets[QUATERNARY_OPTION_VALUE_COUNT]; // Quaternary option Y offsets
    int primaryLabelYValues[PRIMARY_PREF_COUNT]; // Primary label Y positions
    int secondaryLabelYValues[SECONDARY_PREF_COUNT]; // Secondary label Y positions
    int tertiaryLabelYValues[TERTIARY_PREF_COUNT]; // Tertiary label Y positions
    int quaternaryLabelYValues[QUATERNARY_PREF_COUNT]; // Quaternary (Dial) label Y positions

    Point preferencePositions[PREF_COUNT]; // ONLY x/y pairs

    // New offsets (no duplicates)
    int primaryButtonMinXOffset; // 9
    int primaryButtonMaxXOffset; // 37
    int secondaryButtonXOffset; // 22
    int tertiaryButtonXOffset; // 22
    int quaternaryButtonMinXOffset; // 9
    int quaternaryButtonMaxXOffset; // 37
    int rangeThumbLeftOffset; // 6
    int rangeThumbRightOffset; // 14
    double rangeSliderScale; // 219.0 or 274.0
};

struct WorldmapOffsets {
    // Window
    int windowWidth;
    int windowHeight;

    // Viewport
    int viewX; // New: Viewport X position
    int viewY; // New: Viewport Y position
    int viewWidth;
    int viewHeight;

    // UI Elements
    int dialX;
    int dialY;
    int globeOverlayX;
    int globeOverlayY;
    int carX;
    int carY;
    int carOverlayX;
    int carOverlayY;
    int carFuelBarX;
    int carFuelBarY;
    int carFuelBarHeight;

    // Scroll Area
    int scrollAreaX; // New: Scroll area X position
    int scrollAreaY; // New: Scroll area Y position

    // Destination List
    int destListX; // X position of destination list (508 vs 668)
    int destListFirstY; // Y of first destination button (138)
    int destListSpacing; // Vertical spacing (27)

    // Scroll Buttons
    int scrollUpX;
    int scrollUpY;
    int scrollDownX;
    int scrollDownY;

    // Town/World Switch
    int townWorldSwitchX;
    int townWorldSwitchY;

    // Date Display
    int dateDisplayX; // X position (487 vs 647)
    int dateDisplayY; // Y position (12)
    int dateDisplayWidth; // Width (143 = 630-487 vs 143 = 790-647)

    // Viewport Boundaries
    int viewportMaxX; // Max X coordinate (631 for 800x500)
    int viewportMaxY; // Max Y coordinate (485 for 800x500)

    // City Name Drawing
    int cityNameMaxY; // 464 vs 485 (max Y position for city names)

    // Subtile Drawing Boundaries
    int subtileViewportMaxX; // Right edge for subtile drawing (632 in 800x500)
    int subtileViewportMaxY; // Bottom edge for subtile drawing (485 in 800x500)

    // Town Map
    int townMapBgX; // X for town map background (0 in 640x480)
    int townMapBgY; // Y for town map background (0 in 640x480)
    int townMapImageX; // X for town map image (95 vs 100)
    int townMapImageY; // Y for town map image (22 vs 31)
    int townMapButtonXOffset; // Button X offset (0 vs 78)
    int townMapButtonYOffset; // Button Y offset (0 vs 10)
    int townMapLabelXOffset; // Label X offset (0 vs 78)
    int townMapLabelYOffset; // Label Y offset (4 vs 14)

    int townBackgroundWidth; // Width of widescreen town background
    int townBackgroundHeight; // Height of widescreen town background

    int mapcenterX;
    int mapcenterY;
};

extern const MainMenuOffsets gMainMenuOffsets640;
extern const MainMenuOffsets gMainMenuOffsets800;
extern const CharacterEditorOffsets gCharEditorOffsets640;
extern const CharacterEditorOffsets gCharEditorOffsets800;
extern const CharacterSelectorOffsets gCharSelectorOffsets640;
extern const CharacterSelectorOffsets gCharSelectorOffsets800;
extern const LoadSaveOffsets gLoadSaveOffsets640;
extern const LoadSaveOffsets gLoadSaveOffsets800;
extern const PreferencesOffsets gPreferencesOffsets640;
extern const PreferencesOffsets gPreferencesOffsets800;
extern const WorldmapOffsets gWorldmapOffsets640;
extern const WorldmapOffsets gWorldmapOffsets800;

template <typename Offsets>
bool loadOffsetsFromConfig(
    Offsets* offsets,
    bool isWidescreen,
    char const* sectionBaseName,
    Offsets const& fallback640,
    Offsets const& fallback800,
    void (*applyFunc)(Config*, char const*, Offsets*))
{
    char section[32];
    if (isWidescreen) {
        snprintf(section, sizeof(section), "%s800", sectionBaseName);
        *offsets = fallback800;
    } else {
        snprintf(section, sizeof(section), "%s640", sectionBaseName);
        *offsets = fallback640;
    }

    // user cfg
    applyFunc(&gGameConfig, section, offsets);

    // modder overrides
    Config txtCfg;
    if (configInit(&txtCfg)) {
        if (configRead(&txtCfg, "data\\offsets.txt", true)) {
            applyFunc(&txtCfg, section, offsets);
        }
        configFree(&txtCfg);
    }

    return true;
}

// Per‑type “apply” declarations:
void applyConfigToMainMenuOffsets(Config* config, char const* section, MainMenuOffsets* offsets);
void applyConfigToCharacterEditorOffsets(Config* config, char const* section, CharacterEditorOffsets* offsets);
void applyConfigToCharacterSelectorOffsets(Config* config, char const* section, CharacterSelectorOffsets* offsets);
void applyConfigToPreferencesOffsets(Config* config, char const* section, PreferencesOffsets* offsets);
void applyConfigToLoadSaveOffsets(Config* config, char const* section, LoadSaveOffsets* offsets);
void applyConfigToWorldmapOffsets(Config* config, char const* section, WorldmapOffsets* offsets);

}
