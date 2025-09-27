#include "offsets.h"

#include "config.h"

namespace fallout {

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

const CharacterEditorOffsets gCharEditorOffsets640 = {
    // Window
    640, // windowWidth
    480, // windowHeight

    // Buttons
    15, // nameButtonX
    0, // nameButtonY
    347, // tagSkillsButtonX
    26, // tagSkillsButtonY
    363, // printButtonX
    454, // printButtonY
    475, // doneButtonX
    454, // doneButtonY
    571, // cancelButtonX
    454, // cancelButtonY
    23, // optionalTraitsLeftButtonX
    298, // optionalTraitsRightButtonX
    352, // optionalTraitsButtonY
    149, // specialStatsButtonX

    // Primary stat button Y positions
    { 37, 70, 103, 136, 169, 202, 235 },

    // Adjustment slider
    27, // skillValueAdjustmentSliderY

    // Folder view
    0, // folderViewOffsetY (TBD)
    0, // karmaFolderTopLine (TBD)

    // Text positions
    14, // charPointsTextX
    286, // charPointsTextY
    126, // charPointsValueX
    282, // charPointsValueY
    52, // optionalTraitsTextX
    326, // optionalTraitsTextY
    422, // tagSkillsTextX
    233, // tagSkillsTextY

    // Button positions
    614, // sliderPlusX
    20, // sliderPlusY
    11, // folderButtonX
    327, // folderButtonY
    344, // optionsButtonX
    454, // optionsButtonY
    456, // doneButtonGraphicX
    454, // doneButtonGraphicY
    553, // cancelButtonGraphicX
    454, // cancelButtonGraphicY

    // Folder view positions
    34, // folderBackgroundX
    360, // folderBackgroundY
    280, // folderBackgroundWidth
    120, // folderBackgroundHeight
    11, // folderSelectedX
    327, // folderSelectedY

    // PC stats positions
    32, // pcStatsX
    280, // pcStatsY
    124, // pcStatsWidth
    32, // pcStatsHeight

    // Primary stats positions
    58, // primaryStatBigNumberX
    103, // primaryStatDescriptionX

    // Derived stats positions
    194, // derivedStatsTopX
    46, // derivedStatsTopY
    118, // derivedStatsTopWidth
    108, // derivedStatsTopHeight
    194, // derivedStatsBottomX
    179, // derivedStatsBottomY
    116, // derivedStatsBottomWidth
    130, // derivedStatsBottomHeight
    194, // derivedStatsLabelX
    288, // derivedStatsValueX (for Hit Points)

    // Skills background
    370, // skillsBackgroundX
    270, // skillsBackgroundWidth

    // Skills label
    380, // skillsLabelX
    5, // skillsLabelY

    // Skills points label
    400, // skillsPointsLabelX
    233, // skillsPointsLabelY

    // Tag skills label
    422, // tagSkillsLabelX
    233, // tagSkillsLabelY

    // Skills points value
    522, // skillsPointsValueX
    228, // skillsPointsValueY

    // Skills list positions
    27, // skillsListStartY
    380, // skillsListNameX
    573, // skillsListValueX

    // Slider position
    592, // skillsListSliderX

    // Card background
    345, // cardBackgroundX
    267, // cardBackgroundY
    277, // cardBackgroundWidth

    // Name window offset
    0, // nameWindowOffsetY

    // Character points value (in stat adjustment)
    126, // charPointsAdjustX
    282, // charPointsAdjustY

    // Info areas
    19, // statsAreaX
    38, // statsAreaY
    125, // statsAreaWidth
    227, // statsAreaHeight

    28, // charPointsAreaX
    280, // charPointsAreaY
    124, // charPointsAreaWidth
    32, // charPointsAreaHeight

    52, // optionalTraitsTitleX
    324, // optionalTraitsTitleY
    169, // optionalTraitsTitleWidth
    20, // optionalTraitsTitleHeight

    47, // optionalTraitsListX
    353, // optionalTraitsListY
    245, // optionalTraitsListWidth
    100, // optionalTraitsListHeight

    28, // pcStatsFolderAreaX
    363, // pcStatsFolderAreaY
    283, // pcStatsFolderAreaWidth
    105, // pcStatsFolderAreaHeight

    191, // derivedStatsTopAreaX
    41, // derivedStatsTopAreaY
    122, // derivedStatsTopAreaWidth
    110, // derivedStatsTopAreaHeight

    191, // derivedStatsBottomAreaX
    175, // derivedStatsBottomAreaY
    122, // derivedStatsBottomAreaWidth
    135, // derivedStatsBottomAreaHeight

    376, // skillsTitleAreaX
    5, // skillsTitleAreaY
    223, // skillsTitleAreaWidth
    20, // skillsTitleAreaHeight

    370, // skillsListAreaX
    27, // skillsListAreaY
    223, // skillsListAreaWidth
    195, // skillsListAreaHeight

    396, // skillPointsAreaX
    228, // skillPointsAreaY
    171, // skillPointsAreaWidth
    25, // skillPointsAreaHeight

    // Card display
    484, // cardImageX
    309, // cardImageY
    348, // cardTitleX
    272, // cardTitleY
    268, // cardAttributesOffsetY
    272, // cardDividerY
    348, // cardDescriptionX
    315, // cardDescriptionStartY

    // Folder button thresholds
    110, // folderKarmaThresholdX
    208, // folderKillsThresholdX

    // Optional traits
    47, // optionalTraitsBackgroundX
    353, // optionalTraitsBackgroundY
    245, // optionalTraitsBackgroundWidth
    47, // optionalTraitsLeftColumnX
    199, // optionalTraitsRightColumnX
    353, // optionalTraitsStartY

    // New scroll buttons
    317, // folderScrollUpButtonX
    364, // folderScrollUpButtonY
    317, // folderScrollDownButtonX
    365, // folderScrollDownButtonY (calculated at runtime)

    // New text positions
    34, // folderTextX
    314, // folderKillsNumberX

    // New vertical positions
    364, // folderViewStartY

    0, // infoButtonOffsetY
    16, // sliderOffsetY

    61, // perksTitleX
    159, // karmaTitleX
    257 // killsTitleX
};

const CharacterEditorOffsets gCharEditorOffsets800 = {
    // Window
    800, // windowWidth
    500, // windowHeight

    // Buttons
    15, // nameButtonX
    10, // nameButtonY
    427, // tagSkillsButtonX
    36, // tagSkillsButtonY
    450, // printButtonX
    464, // printButtonY
    570, // doneButtonX
    464, // doneButtonY
    695, // cancelButtonX
    464, // cancelButtonY
    30, // optionalTraitsLeftButtonX
    378, // optionalTraitsRightButtonX
    362, // optionalTraitsButtonY
    182, // specialStatsButtonX

    // Primary stat button Y positions
    { 47, 80, 113, 146, 179, 212, 245 },

    // Adjustment slider
    37, // skillValueAdjustmentSliderY

    // Folder view
    0, // folderViewOffsetY (TBD)
    0, // karmaFolderTopLine (TBD)

    // Text positions
    34, // charPointsTextX
    296, // charPointsTextY
    161, // charPointsValueX
    292, // charPointsValueY
    62, // optionalTraitsTextX
    336, // optionalTraitsTextY
    492, // tagSkillsTextX
    243, // tagSkillsTextY

    // Button positions
    762, // sliderPlusX
    30, // sliderPlusY
    16, // folderButtonX
    337, // folderButtonY
    425, // optionsButtonX
    464, // optionsButtonY
    545, // doneButtonGraphicX
    464, // doneButtonGraphicY
    670, // cancelButtonGraphicX
    464, // cancelButtonGraphicY

    // Folder view positions
    38, // folderBackgroundX
    370, // folderBackgroundY
    354, // folderBackgroundWidth
    120, // folderBackgroundHeight
    16, // folderSelectedX
    337, // folderSelectedY

    // PC stats positions
    40, // pcStatsX
    290, // pcStatsY
    151, // pcStatsWidth
    40, // pcStatsHeight

    // Primary stats positions
    64, // primaryStatBigNumberX
    110, // primaryStatDescriptionX

    // Derived stats positions
    234, // derivedStatsTopX
    56, // derivedStatsTopY
    176, // derivedStatsTopWidth
    114, // derivedStatsTopHeight
    234, // derivedStatsBottomX
    189, // derivedStatsBottomY
    116, // derivedStatsBottomWidth
    130, // derivedStatsBottomHeight
    234, // derivedStatsLabelX
    328, // derivedStatsValueX (for Hit Points)

    // Skills background
    450, // skillsBackgroundX
    340, // skillsBackgroundWidth

    // Skills label
    460, // skillsLabelX
    15, // skillsLabelY

    // Skills points label
    480, // skillsPointsLabelX
    243, // skillsPointsLabelY

    // Tag skills label
    492, // tagSkillsLabelX
    243, // tagSkillsLabelY

    // Skills points value
    602, // skillsPointsValueX
    238, // skillsPointsValueY

    // Skills list positions
    37, // skillsListStartY
    460, // skillsListNameX
    725, // skillsListValueX

    // Slider position
    740, // skillsListSliderX

    // Card background
    425, // cardBackgroundX
    277, // cardBackgroundY
    330, // cardBackgroundWidth

    // Name window offset
    10, // nameWindowOffsetY

    // Character points value (in stat adjustment)
    161, // charPointsAdjustX
    292, // charPointsAdjustY

    // Info areas
    25, // statsAreaX
    48, // statsAreaY
    150, // statsAreaWidth
    227, // statsAreaHeight

    34, // charPointsAreaX
    290, // charPointsAreaY
    159, // charPointsAreaWidth
    32, // charPointsAreaHeight

    61, // optionalTraitsTitleX
    334, // optionalTraitsTitleY
    312, // optionalTraitsTitleWidth
    20, // optionalTraitsTitleHeight

    52, // optionalTraitsListX
    363, // optionalTraitsListY
    312, // optionalTraitsListWidth
    100, // optionalTraitsListHeight

    28, // pcStatsFolderAreaX
    373, // pcStatsFolderAreaY
    283, // pcStatsFolderAreaWidth
    105, // pcStatsFolderAreaHeight

    222, // derivedStatsTopAreaX
    51, // derivedStatsTopAreaY
    175, // derivedStatsTopAreaWidth
    110, // derivedStatsTopAreaHeight

    222, // derivedStatsBottomAreaX
    185, // derivedStatsBottomAreaY
    175, // derivedStatsBottomAreaWidth
    135, // derivedStatsBottomAreaHeight

    456, // skillsTitleAreaX
    15, // skillsTitleAreaY
    290, // skillsTitleAreaWidth
    20, // skillsTitleAreaHeight

    450, // skillsListAreaX
    37, // skillsListAreaY
    290, // skillsListAreaWidth
    195, // skillsListAreaHeight

    488, // skillPointsAreaX
    238, // skillPointsAreaY
    171, // skillPointsAreaWidth
    25, // skillPointsAreaHeight

    // Card display
    614, // cardImageX
    324, // cardImageY
    438, // cardTitleX
    287, // cardTitleY
    283, // cardAttributesOffsetY
    287, // cardDividerY
    438, // cardDescriptionX
    330, // cardDescriptionStartY

    // Folder button thresholds
    136, // folderKarmaThresholdX
    259, // folderKillsThresholdX

    // Optional traits
    57, // optionalTraitsBackgroundX
    363, // optionalTraitsBackgroundY
    395, // optionalTraitsBackgroundWidth
    57, // optionalTraitsLeftColumnX
    269, // optionalTraitsRightColumnX
    363, // optionalTraitsStartY

    // New scroll buttons
    397, // folderScrollUpButtonX
    374, // folderScrollUpButtonY
    397, // folderScrollDownButtonX
    375, // folderScrollDownButtonY (calculated at runtime)

    // New text positions
    40, // folderTextX
    395, // folderKillsNumberX

    // New vertical positions
    374, // folderViewStartY

    10, // infoButtonOffsetY
    26, // sliderOffsetY

    69, // perksTitleX
    195, // karmaTitleX
    315 // killsTitleX

};

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
    40, 30, 800, 300,

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
    47, // faceX

    // Bio rendering
    260, // bioMaxY (same as 640)
};

const LoadSaveOffsets gLoadSaveOffsets640 = {
    // Window
    640, // windowWidth
    480, // windowHeight

    // Preview
    224, // previewWidth
    133, // previewHeight
    366, // previewX
    58, // previewY
    340, // previewCoverX
    39, // previewCoverY

    // Title and Text
    48, // titleTextX
    27, // titleTextY
    410, // doneLabelX
    348, // doneLabelY
    515, // cancelLabelX
    348, // cancelLabelY

    // Buttons
    391, // doneButtonX
    349, // doneButtonY
    495, // cancelButtonX
    349, // cancelButtonY
    35, // arrowUpX
    58, // arrowUpY
    35, // arrowDownX
    79, // arrowDownY

    // Slot List Area
    55, // slotListAreaX
    87, // slotListAreaY
    230, // slotListAreaWidth
    348, // slotListAreaHeight

    // Comment Window
    169, // commentWindowX
    116, // commentWindowY

    // Slot List
    55, // slotListX
    87, // slotListY
    230, // slotListWidth
    40, // slotListBottomOffset

    // Info Box
    396, // infoBoxX
    253, // infoBoxY
    164, // infoBoxWidth
    60, // infoBoxHeight

    // Info Box Text Positions
    396, // characterNameX
    253, // characterNameY
    397, // gameDateX
    270, // gameDateY
    399, // locationX
    287, // locationY

    // Page Navigation Buttons
    195, // nextPageButtonX
    425, // nextPageButtonY
    85, // nextPageButtonWidth
    10, // nextPageButtonHeight
    55, // prevPageButtonX
    425, // prevPageButtonY
    125, // prevPageButtonWidth
    10, // prevPageButtonHeight

    // Text Block Position
    253, // infoBoxTextBlockY

    // Cover Image Parameters
    275, // coverWidth
    173, // coverHeight
    340, // coverX
    39, // coverY
    275, // coverPitch

    // Slot Text Padding
    0, // slotTextPadding

    // Pagination text positions
    40, // backTextOffsetX
    155 // moreTextOffsetX
};

const LoadSaveOffsets gLoadSaveOffsets800 = {
    // Window
    800, // windowWidth
    500, // windowHeight

    // Preview
    270, // previewWidth
    145, // previewHeight
    452, // previewX
    55, // previewY
    421, // previewCoverX
    34, // previewCoverY

    // Title and Text
    69, // titleTextX
    37, // titleTextY
    518, // doneLabelX
    358, // doneLabelY
    623, // cancelLabelX
    358, // cancelLabelY

    // Buttons
    499, // doneButtonX
    359, // doneButtonY
    603, // cancelButtonX
    359, // cancelButtonY
    56, // arrowUpX
    68, // arrowUpY
    56, // arrowDownX
    89, // arrowDownY

    // Slot List Area
    76, // slotListAreaX
    97, // slotListAreaY
    290, // slotListAreaWidth
    363, // slotListAreaHeight

    // Comment Window
    169, // commentWindowX
    116, // commentWindowY

    // Slot List
    76, // slotListX
    97, // slotListY
    230, // slotListWidth
    40, // slotListBottomOffset

    // Info Box
    477, // infoBoxX
    262, // infoBoxY
    224, // infoBoxWidth
    60, // infoBoxHeight

    // Info Box Text Positions
    477, // characterNameX
    262, // characterNameY
    479, // gameDateX
    279, // gameDateY
    480, // locationX
    296, // locationY

    // Page Navigation Buttons
    245, // nextPageButtonX
    445, // nextPageButtonY
    85, // nextPageButtonWidth
    10, // nextPageButtonHeight
    105, // prevPageButtonX
    445, // prevPageButtonY
    125, // prevPageButtonWidth
    10, // prevPageButtonHeight

    // Text Block Position
    262, // infoBoxTextBlockY

    // Cover Image Parameters
    330, // coverWidth
    188, // coverHeight
    421, // coverX
    34, // coverY
    330, // coverPitch

    // Slot Text Padding
    3, // slotTextPadding

    // Pagination text positions
    50, // backTextOffsetX
    194 // moreTextOffsetX
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

    // Tertiary preferences (middle column)
    340, // tertiaryColumnX
    340, // tertiaryKnobX
    { 49, 116, 181, 247, 313, 380 }, // tertiaryKnobY[6]
    { 66, 133, 200, 264, 331, 397 }, // tertiaryLabelY[6]

    // Range preferences (right column)
    374, // rangeColumnX
    374, // rangeKnobX
    { 19, 94, 165, 216, 268, 319, 369, 420 }, // rangeKnobY[8]

    // Label positions
    99, // primLabelColX
    206, // secLabelColX
    306, // terLabelColX
    384, // rangLabelColX
    { 23, 23, 23, 23, 23 }, // labelX[5]
    { 251, 251, 251, 251, 251, 251 }, // secondaryLabelX[6]
    { 351, 351, 351, 351, 351, 351 }, // tertiaryLabelX[6]
    { 351 }, // quaternarylabelX[1]

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
    113, // tertiaryBlitWidth
    34, // tertiaryBlitHeight
    54, // quaternaryBlitWidth
    56, // quaternaryBlitHeight
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
    11, // tertiaryKnobHitX
    12, // tertiaryKnobHitY
    27, // primaryKnobHitX
    28, // primaryKnobHitY

    // Range slider parameters
    384, // rangeSliderMinX
    603, // rangeSliderMaxX
    219, // rangeSliderWidth

    // Button hitbox offsets
    -4, // primaryButtonOffsetY
    -5, // secondaryButtonOffsetY
    -5, // tertiaryButtonOffsetY
    -4, // quaternaryButtonOffsetY
    -12, // rangeButtonOffsetY

    // Text delay and range label positions
    43.8, // textBaseDelayScale (double)
    444, // rangeLabel4Option1X
    564, // rangeLabel4Option2X

    // Position arrays
    { 48, 125, 203, 286, 363 }, // row1Ytab[5]
    { 49, 116, 181, 247, 313, 380 }, // row2Ytab[6]
    { 49, 116, 181, 247, 313, 380 }, // row2bYtab[6]
    { 49 }, // rowdialYtab[1]
    { 19, 94, 165, 216, 268, 319, 369, 420 }, // row3Ytab[8]
    { 2, 25, 46, 46 }, // optionXOffsets[4]
    { 10, -4, 10, 31 }, // optionYOffsets[4]
    { 4, 21 }, // secondaryOptionXOffsets[2]
    { 21, 4 }, // tertiaryOptionYOffsets[2]
    { 2, 25, 46, 46 }, // quaternaryXOffsets[4]
    { 10, -4, 10, 31 }, // quaternaryYOffsets[4]
    { 66, 143, 222, 304, 382 }, // primaryLabelYValues[5]
    { 66, 133, 200, 264, 331, 397 }, // secondaryLabelYValues[6]
    { 66, 133, 200, 264, 331, 397 }, // tertiaryLabelYValues[6]
    { 425 }, // quaternaryLabelYValues[1] - used for orienting background blit

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
        Point { 440, 174 }, // PREF_WINDOWED
        Point { 440, 241 }, // PREF_QUALITY
        Point { 440, 307 }, // PREF_ASPECT
        Point { 440, 371 }, // PREF_SQUARE_PIXELS
        Point { 425, 320 }, // PREF_PLAY_AREA
        Point { 440, 238 }, // PREF_STRETCHING
        Point { 440, 204 }, // PREF_WIDESCREEN
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
    22, // tertiaryButtonXOffset
    9, // primaryButtonMinXOffset
    37, // primaryButtonMaxXOffset
    6, // rangeThumbLeftOffset
    14, // rangeThumbRightOffset
    219.0 // rangeSliderScale (double)
};

const PreferencesOffsets gPreferencesOffsets800 = {
    // Window dimensions
    800, // width
    500, // height

    // Primary preferences (left column)
    76, // primaryColumnX
    76, // primaryKnobX
    { 50, 131, 211, 299, 380 }, // primaryKnobY[5]
    { 70, 151, 234, 320, 402 }, // primaryLabelY[5]

    // Secondary preferences (middle column)
    299, // secondaryColumnX
    299, // secondaryKnobX
    { 50, 119, 187, 256, 325, 395 }, // secondaryKnobY[6]
    { 71, 139, 209, 277, 347, 415 }, // secondaryLabelY[6]

    // Tertiary preferences (middle column)
    390, // tertiaryColumnX
    390, // tertiaryKnobX
    { 50, 119, 187, 256, 325, 395 }, // tertiaryKnobY[6]
    { 71, 139, 209, 277, 347, 415 }, // tertiaryLabelY[6]

    // Range preferences (right column)
    535, // rangeColumnX
    535, // rangeKnobX
    { 20, 99, 172, 225, 279, 332, 384, 437 }, // rangeKnobY[8]

    // Label positions
    99, // primLabelColX
    206, // secLabelColX
    447, // terLabelColX
    545, // rangLabelColX
    { 23, 23, 23, 23, 23 }, // labelX[5]
    { 251, 251, 251, 251, 251, 251 }, // secondaryLabelX[6]
    { 375, 450, 375, 450, 375, 450 }, // tertiaryLabelX[6]
    { 375 }, // quaternarylabelX[1]

    // Range control parameters
    545, // rangeStartX
    219, // rangeWidth
    21, // knobWidth
    { 546, 655, 715, 780, 595 }, // rangeLabelX[5]

    // Blit dimensions
    160, // primaryBlitWidth
    54, // primaryBlitHeight
    113, // secondaryBlitWidth
    34, // secondaryBlitHeight
    63, // tertiaryBlitWidth
    75, // tertiaryBlitHeight
    140, // quaternaryBlitWidth
    61, // quaternaryBlitHeight
    240, // rangeBlitWidth
    24, // rangeBlitHeight

    // Title and buttons
    150, // titleTextX
    10, // titleTextY
    121, // defaultLabelX
    468, // defaultLabelY
    247, // doneLabelX
    468, // doneLabelY
    361, // cancelLabelX
    468, // cancelLabelY
    72, // speedLabelX
    570, // speedLabelY

    // Button positions
    101, // defaultButtonX
    468, // defaultButtonY
    226, // doneButtonX
    468, // doneButtonY
    341, // cancelButtonX
    468, // cancelButtonY

    // Checkbox position
    545, // playerSpeedCheckboxX
    68, // playerSpeedCheckboxY

    // Knob hit detection offsets
    23, // primaryKnobHitX
    21, // primaryKnobHitY
    11, // secondaryKnobHitX
    12, // secondaryKnobHitY
    25, // tertiaryKnobHitX
    20, // tertiaryKnobHitY
    27, // primaryKnobHitX
    28, // primaryKnobHitY

    // Range slider parameters
    545, // rangeSliderMinX
    764, // rangeSliderMaxX
    219, // rangeSliderWidth

    // Button hitbox offsets
    -4, // primaryButtonOffsetY
    -5, // secondaryButtonOffsetY
    -15, // tertiaryButtonOffsetY
    -4, // quaternaryButtonOffsetY
    -12, // rangeButtonOffsetY

    // Text delay and range label positions
    43.8, // textBaseDelayScale (double)
    555, // rangeLabel4Option1X
    705, // rangeLabel4Option2X

    // Position arrays
    { 50, 131, 211, 299, 380 }, // row1Ytab[5]
    { 50, 119, 187, 256, 325, 395 }, // row2Ytab[6]
    { 50, 152, 257, 361, 470, 470 }, // row2bYtab[6]
    { 49 }, // rowdialYtab[1]
    { 20, 99, 172, 225, 279, 332, 384, 437 }, // row3Ytab[8]
    { 2, 25, 46, 46 }, // optionXOffsets[4] -big knobs labels x
    { 10, -4, 10, 31 }, // optionYOffsets[4] -big knobs labels y
    { 4, 21 }, // secondaryOptionXOffsets[2] -small knob labels x
    { -44, 10 }, // tertiaryOptionYOffsets[2] - must serve as Y offset
    { 7, -15, 55, 55 }, // quaternaryXOffsets[4] -big dial labels x
    { 40, -2, -2, 40 }, // quaternaryYOffsets[4] -big dial labels y
    { 70, 151, 234, 320, 402 }, // primaryLabelYValues[5]
    { 71, 139, 209, 277, 347, 415 }, // secondaryLabelYValues[6]
    { 70, 70, 173, 173, 278, 278 }, // tertiaryLabelYValues[6] - used for orienting background blit
    { 385 }, // quaternaryLabelYValues[1] - used for orienting background blit

    // Preference positions
    {
        Point { 76, 74 }, // PREF_GAME_DIFFICULTY
        Point { 76, 157 }, // PREF_COMBAT_DIFFICULTY
        Point { 76, 237 }, // PREF_VIOLENCE_LEVEL
        Point { 76, 324 }, // PREF_TARGET_HIGHLIGHT
        Point { 76, 406 }, // PREF_COMBAT_LOOKS
        Point { 299, 76 }, // PREF_COMBAT_MESSAGES
        Point { 299, 147 }, // PREF_COMBAT_TAUNTS
        Point { 299, 216 }, // PREF_LANGUAGE_FILTER
        Point { 299, 284 }, // PREF_RUNNING
        Point { 299, 354 }, // PREF_SUBTITLES
        Point { 299, 423 }, // PREF_ITEM_HIGHLIGHT
        Point { 385, 80 }, // PREF_WINDOWED
        Point { 460, 80 }, // REF_WIDESCREEN
        Point { 385, 183 }, // PREF_STRETCHING
        Point { 460, 183 }, // PREF_ASPECT
        Point { 385, 288 }, // PREF_QUALITY
        Point { 460, 288 }, // PREF_SQUARE_PIXELS
        Point { 415, 390 }, // PREF_PLAY_AREA
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
    42, // tertiaryButtonXOffset
    9, // primaryButtonMinXOffset
    37, // primaryButtonMaxXOffset
    6, // rangeThumbLeftOffset
    14, // rangeThumbRightOffset
    219.0 // rangeSliderScale (double)
};

const WorldmapOffsets gWorldmapOffsets640 = {
    // Window dimensions
    640, // windowWidth
    480, // windowHeight

    // Viewport position and size
    22, // viewX (left edge of viewport area)
    21, // viewY (top edge of viewport area)
    450, // viewWidth (width of scrollable map area)
    443, // viewHeight (height of scrollable map area)

    // UI Elements - dial control
    532, // dialX (position of worldmap dial)
    48, // dialY

    // UI Elements - globe and car indicators
    495, // globeOverlayX (position of globe graphic)
    330, // globeOverlayY
    514, // carX (position of car graphic)
    336, // carY
    499, // carOverlayX (position of car selection highlight)
    330, // carOverlayY
    500, // carFuelBarX (position of fuel gauge)
    339, // carFuelBarY
    70, // carFuelBarHeight

    // Scrollable destination list area
    501, // scrollAreaX (left edge of destination list)
    135, // scrollAreaY (top edge of destination list)

    // Destination List
    508, // destListX
    138, // destListFirstY
    27, // destListSpacing

    // Scroll Buttons
    480, // scrollUpX
    137, // scrollUpY
    480, // scrollDownX
    152, // scrollDownY

    // Town/World Switch
    519, // townWorldSwitchX
    439, // townWorldSwitchY

    // Date Display
    487, // dateDisplayX
    12, // dateDisplayY
    143, // dateDisplayWidth (630-487)

    // Viewport Boundaries
    449, // viewportMaxX
    442, // viewportMaxY

    // City Name Drawing
    464, // cityNameMaxY

    545, // subtileViewportMaxX
    465, // subtileViewportMaxY

    0, // townMapBgX (not used)
    0, // townMapBgY (not used)
    95, // townMapImageX
    22, // townMapImageY
    0, // townMapButtonXOffset
    0, // townMapButtonYOffset
    0, // townMapLabelXOffset
    4, // townMapLabelYOffset

    450, // townBackgroundWidth
    442, // townBackgroundHeight

    203, // mapcenterX
    200 // mapcenterY
};

const WorldmapOffsets gWorldmapOffsets800 = {
    // Window dimensions
    800, // windowWidth (widescreen width)
    500, // windowHeight (widescreen height)

    // Viewport position and size
    22, // viewX (same X position as 640x480)
    21, // viewY (same Y position as 640x480)
    610, // viewWidth (wider viewport for widescreen)
    463, // viewHeight (taller viewport for widescreen)

    // UI Elements - dial control
    692, // dialX (shifted right for widescreen)
    48, // dialY (same vertical position)

    // UI Elements - globe and car indicators
    655, // globeOverlayX (shifted right proportionally)
    330, // globeOverlayY (same vertical position)
    674, // carX (shifted right proportionally)
    336, // carY (same vertical position)
    659, // carOverlayX (shifted right proportionally)
    330, // carOverlayY (same vertical position)
    660, // carFuelBarX (shifted right proportionally)
    339, // carFuelBarY (same vertical position)
    70, // carFuelBarHeight

    // Scrollable destination list area
    661, // scrollAreaX (shifted right for widescreen)
    135, // scrollAreaY (same vertical position)

    // Destination List
    668, // destListX (+160)
    138, // destListFirstY (same)
    27, // destListSpacing (same)

    // Scroll Buttons
    640, // scrollUpX (+160)
    137, // scrollUpY (same)
    640, // scrollDownX (+160)
    152, // scrollDownY (same)

    // Town/World Switch
    679, // townWorldSwitchX (+160)
    439, // townWorldSwitchY (same)

    // Date Display
    647, // dateDisplayX (+160)
    12, // dateDisplayY (same)
    143, // dateDisplayWidth (same)

    // Viewport Boundaries
    631, // viewportMaxX (hardcoded)
    485, // viewportMaxY (hardcoded)

    // City Name Drawing
    485, // cityNameMaxY

    // Subtile Drawing Boundaries
    632, // subtileViewportMaxX (hardcoded in 800x500)
    485, // subtileViewportMaxY (hardcoded in 800x500)

    78, // townMapBgX
    10, // townMapBgY
    100, // townMapImageX
    31, // townMapImageY
    78, // townMapButtonXOffset
    10, // townMapButtonYOffset
    78, // townMapLabelXOffset
    14, // townMapLabelYOffset

    610, // townBackgroundWidth
    462, // townBackgroundHeight

    254, // mapcenterX
    208 // mapcenterY
};

void applyConfigToMainMenuOffsets(Config* config, const char* section, MainMenuOffsets* offsets)
{
    configGetInt(config, section, "copyrightX", &offsets->copyrightX);
    configGetInt(config, section, "copyrightY", &offsets->copyrightY);
    configGetInt(config, section, "versionX", &offsets->versionX);
    configGetInt(config, section, "versionY", &offsets->versionY);
    configGetInt(config, section, "hashX", &offsets->hashX);
    configGetInt(config, section, "hashY", &offsets->hashY);
    configGetInt(config, section, "buildDateX", &offsets->buildDateX);
    configGetInt(config, section, "buildDateY", &offsets->buildDateY);
    configGetInt(config, section, "buttonBaseX", &offsets->buttonBaseX);
    configGetInt(config, section, "buttonBaseY", &offsets->buttonBaseY);
    configGetInt(config, section, "buttonTextOffsetX", &offsets->buttonTextOffsetX);
    configGetInt(config, section, "buttonTextOffsetY", &offsets->buttonTextOffsetY);
    configGetInt(config, section, "width", &offsets->width);
    configGetInt(config, section, "height", &offsets->height);
}

void applyConfigToCharacterEditorOffsets(Config* config, const char* section, CharacterEditorOffsets* offsets)
{
    // Window dimensions
    configGetInt(config, section, "windowWidth", &offsets->windowWidth);
    configGetInt(config, section, "windowHeight", &offsets->windowHeight);

    // Buttons
    configGetInt(config, section, "nameButtonX", &offsets->nameButtonX);
    configGetInt(config, section, "nameButtonY", &offsets->nameButtonY);
    configGetInt(config, section, "tagSkillsButtonX", &offsets->tagSkillsButtonX);
    configGetInt(config, section, "tagSkillsButtonY", &offsets->tagSkillsButtonY);
    configGetInt(config, section, "printButtonX", &offsets->printButtonX);
    configGetInt(config, section, "printButtonY", &offsets->printButtonY);
    configGetInt(config, section, "doneButtonX", &offsets->doneButtonX);
    configGetInt(config, section, "doneButtonY", &offsets->doneButtonY);
    configGetInt(config, section, "cancelButtonX", &offsets->cancelButtonX);
    configGetInt(config, section, "cancelButtonY", &offsets->cancelButtonY);
    configGetInt(config, section, "optionalTraitsLeftButtonX", &offsets->optionalTraitsLeftButtonX);
    configGetInt(config, section, "optionalTraitsRightButtonX", &offsets->optionalTraitsRightButtonX);
    configGetInt(config, section, "optionalTraitsButtonY", &offsets->optionalTraitsButtonY);
    configGetInt(config, section, "specialStatsButtonX", &offsets->specialStatsButtonX);

    // Primary stat Y (array of 7)
    configGetIntArray(config, section, "primaryStatY", offsets->primaryStatY, 7);

    // Skill‐adjustment slider
    configGetInt(config, section, "skillValueAdjustmentSliderY", &offsets->skillValueAdjustmentSliderY);

    // Folder view
    configGetInt(config, section, "folderViewOffsetY", &offsets->folderViewOffsetY);
    configGetInt(config, section, "karmaFolderTopLine", &offsets->karmaFolderTopLine);

    // Text positions
    configGetInt(config, section, "charPointsTextX", &offsets->charPointsTextX);
    configGetInt(config, section, "charPointsTextY", &offsets->charPointsTextY);
    configGetInt(config, section, "charPointsValueX", &offsets->charPointsValueX);
    configGetInt(config, section, "charPointsValueY", &offsets->charPointsValueY);
    configGetInt(config, section, "optionalTraitsTextX", &offsets->optionalTraitsTextX);
    configGetInt(config, section, "optionalTraitsTextY", &offsets->optionalTraitsTextY);
    configGetInt(config, section, "tagSkillsTextX", &offsets->tagSkillsTextX);
    configGetInt(config, section, "tagSkillsTextY", &offsets->tagSkillsTextY);

    // Slider & folder buttons
    configGetInt(config, section, "sliderPlusX", &offsets->sliderPlusX);
    configGetInt(config, section, "sliderPlusY", &offsets->sliderPlusY);
    configGetInt(config, section, "folderButtonX", &offsets->folderButtonX);
    configGetInt(config, section, "folderButtonY", &offsets->folderButtonY);
    configGetInt(config, section, "optionsButtonX", &offsets->optionsButtonX);
    configGetInt(config, section, "optionsButtonY", &offsets->optionsButtonY);
    configGetInt(config, section, "doneButtonGraphicX", &offsets->doneButtonGraphicX);
    configGetInt(config, section, "doneButtonGraphicY", &offsets->doneButtonGraphicY);
    configGetInt(config, section, "cancelButtonGraphicX", &offsets->cancelButtonGraphicX);
    configGetInt(config, section, "cancelButtonGraphicY", &offsets->cancelButtonGraphicY);

    // Folder view decorations
    configGetInt(config, section, "folderBackgroundX", &offsets->folderBackgroundX);
    configGetInt(config, section, "folderBackgroundY", &offsets->folderBackgroundY);
    configGetInt(config, section, "folderBackgroundWidth", &offsets->folderBackgroundWidth);
    configGetInt(config, section, "folderBackgroundHeight", &offsets->folderBackgroundHeight);
    configGetInt(config, section, "folderSelectedX", &offsets->folderSelectedX);
    configGetInt(config, section, "folderSelectedY", &offsets->folderSelectedY);

    // PC stats panel
    configGetInt(config, section, "pcStatsX", &offsets->pcStatsX);
    configGetInt(config, section, "pcStatsY", &offsets->pcStatsY);
    configGetInt(config, section, "pcStatsWidth", &offsets->pcStatsWidth);
    configGetInt(config, section, "pcStatsHeight", &offsets->pcStatsHeight);

    // Primary stats labels
    configGetInt(config, section, "primaryStatBigNumberX", &offsets->primaryStatBigNumberX);
    configGetInt(config, section, "primaryStatDescriptionX", &offsets->primaryStatDescriptionX);

    // Derived stats panel
    configGetInt(config, section, "derivedStatsTopX", &offsets->derivedStatsTopX);
    configGetInt(config, section, "derivedStatsTopY", &offsets->derivedStatsTopY);
    configGetInt(config, section, "derivedStatsTopWidth", &offsets->derivedStatsTopWidth);
    configGetInt(config, section, "derivedStatsTopHeight", &offsets->derivedStatsTopHeight);
    configGetInt(config, section, "derivedStatsBottomX", &offsets->derivedStatsBottomX);
    configGetInt(config, section, "derivedStatsBottomY", &offsets->derivedStatsBottomY);
    configGetInt(config, section, "derivedStatsBottomWidth", &offsets->derivedStatsBottomWidth);
    configGetInt(config, section, "derivedStatsBottomHeight", &offsets->derivedStatsBottomHeight);
    configGetInt(config, section, "derivedStatsLabelX", &offsets->derivedStatsLabelX);
    configGetInt(config, section, "derivedStatsValueX", &offsets->derivedStatsValueX);

    // New folder scroll buttons & text
    configGetInt(config, section, "folderScrollUpButtonX", &offsets->folderScrollUpButtonX);
    configGetInt(config, section, "folderScrollUpButtonY", &offsets->folderScrollUpButtonY);
    configGetInt(config, section, "folderScrollDownButtonX", &offsets->folderScrollDownButtonX);
    configGetInt(config, section, "folderScrollDownButtonY", &offsets->folderScrollDownButtonY);
    configGetInt(config, section, "folderTextX", &offsets->folderTextX);
    configGetInt(config, section, "folderKillsNumberX", &offsets->folderKillsNumberX);
    configGetInt(config, section, "folderViewStartY", &offsets->folderViewStartY);

    // Misc offsets
    configGetInt(config, section, "infoButtonOffsetY", &offsets->infoButtonOffsetY);
    configGetInt(config, section, "sliderOffsetY", &offsets->sliderOffsetY);
    configGetInt(config, section, "perksTitleX", &offsets->perksTitleX);
    configGetInt(config, section, "karmaTitleX", &offsets->karmaTitleX);
    configGetInt(config, section, "killsTitleX", &offsets->killsTitleX);
}

void applyConfigToCharacterSelectorOffsets(Config* config, const char* section, CharacterSelectorOffsets* offsets)
{
    // Window
    configGetInt(config, section, "width", &offsets->width);
    configGetInt(config, section, "height", &offsets->height);

    // Background
    configGetInt(config, section, "backgroundX", &offsets->backgroundX);
    configGetInt(config, section, "backgroundY", &offsets->backgroundY);
    configGetInt(config, section, "backgroundWidth", &offsets->backgroundWidth);
    configGetInt(config, section, "backgroundHeight", &offsets->backgroundHeight);

    // Buttons
    configGetInt(config, section, "previousButtonX", &offsets->previousButtonX);
    configGetInt(config, section, "previousButtonY", &offsets->previousButtonY);
    configGetInt(config, section, "nextButtonX", &offsets->nextButtonX);
    configGetInt(config, section, "nextButtonY", &offsets->nextButtonY);
    configGetInt(config, section, "takeButtonX", &offsets->takeButtonX);
    configGetInt(config, section, "takeButtonY", &offsets->takeButtonY);
    configGetInt(config, section, "modifyButtonX", &offsets->modifyButtonX);
    configGetInt(config, section, "modifyButtonY", &offsets->modifyButtonY);
    configGetInt(config, section, "createButtonX", &offsets->createButtonX);
    configGetInt(config, section, "createButtonY", &offsets->createButtonY);
    configGetInt(config, section, "backButtonX", &offsets->backButtonX);
    configGetInt(config, section, "backButtonY", &offsets->backButtonY);

    // Text positions
    configGetInt(config, section, "nameMidX", &offsets->nameMidX);
    configGetInt(config, section, "primaryStatMidX", &offsets->primaryStatMidX);
    configGetInt(config, section, "secondaryStatMidX", &offsets->secondaryStatMidX);
    configGetInt(config, section, "bioX", &offsets->bioX);
    configGetInt(config, section, "textBaseY", &offsets->textBaseY);
    configGetInt(config, section, "faceY", &offsets->faceY);

    // Face position
    configGetInt(config, section, "faceX", &offsets->faceX);

    // Bio rendering
    configGetInt(config, section, "bioMaxY", &offsets->bioMaxY);
}

void applyConfigToLoadSaveOffsets(Config* config, const char* section, LoadSaveOffsets* offsets)
{
    // Window
    configGetInt(config, section, "windowWidth", &offsets->windowWidth);
    configGetInt(config, section, "windowHeight", &offsets->windowHeight);

    // Preview pane
    configGetInt(config, section, "previewWidth", &offsets->previewWidth);
    configGetInt(config, section, "previewHeight", &offsets->previewHeight);
    configGetInt(config, section, "previewX", &offsets->previewX);
    configGetInt(config, section, "previewY", &offsets->previewY);
    configGetInt(config, section, "previewCoverX", &offsets->previewCoverX);
    configGetInt(config, section, "previewCoverY", &offsets->previewCoverY);

    // Title & text labels
    configGetInt(config, section, "titleTextX", &offsets->titleTextX);
    configGetInt(config, section, "titleTextY", &offsets->titleTextY);
    configGetInt(config, section, "doneLabelX", &offsets->doneLabelX);
    configGetInt(config, section, "doneLabelY", &offsets->doneLabelY);
    configGetInt(config, section, "cancelLabelX", &offsets->cancelLabelX);
    configGetInt(config, section, "cancelLabelY", &offsets->cancelLabelY);

    // Buttons
    configGetInt(config, section, "doneButtonX", &offsets->doneButtonX);
    configGetInt(config, section, "doneButtonY", &offsets->doneButtonY);
    configGetInt(config, section, "cancelButtonX", &offsets->cancelButtonX);
    configGetInt(config, section, "cancelButtonY", &offsets->cancelButtonY);
    configGetInt(config, section, "arrowUpX", &offsets->arrowUpX);
    configGetInt(config, section, "arrowUpY", &offsets->arrowUpY);
    configGetInt(config, section, "arrowDownX", &offsets->arrowDownX);
    configGetInt(config, section, "arrowDownY", &offsets->arrowDownY);

    // Slot list region
    configGetInt(config, section, "slotListAreaX", &offsets->slotListAreaX);
    configGetInt(config, section, "slotListAreaY", &offsets->slotListAreaY);
    configGetInt(config, section, "slotListAreaWidth", &offsets->slotListAreaWidth);
    configGetInt(config, section, "slotListAreaHeight", &offsets->slotListAreaHeight);

    // Comment window
    configGetInt(config, section, "commentWindowX", &offsets->commentWindowX);
    configGetInt(config, section, "commentWindowY", &offsets->commentWindowY);

    // Slot list itself
    configGetInt(config, section, "slotListX", &offsets->slotListX);
    configGetInt(config, section, "slotListY", &offsets->slotListY);
    configGetInt(config, section, "slotListWidth", &offsets->slotListWidth);
    configGetInt(config, section, "slotListBottomOffset", &offsets->slotListBottomOffset);

    // Info box
    configGetInt(config, section, "infoBoxX", &offsets->infoBoxX);
    configGetInt(config, section, "infoBoxY", &offsets->infoBoxY);
    configGetInt(config, section, "infoBoxWidth", &offsets->infoBoxWidth);
    configGetInt(config, section, "infoBoxHeight", &offsets->infoBoxHeight);

    // Info text positions
    configGetInt(config, section, "characterNameX", &offsets->characterNameX);
    configGetInt(config, section, "characterNameY", &offsets->characterNameY);
    configGetInt(config, section, "gameDateX", &offsets->gameDateX);
    configGetInt(config, section, "gameDateY", &offsets->gameDateY);
    configGetInt(config, section, "locationX", &offsets->locationX);
    configGetInt(config, section, "locationY", &offsets->locationY);

    // Page nav buttons
    configGetInt(config, section, "nextPageButtonX", &offsets->nextPageButtonX);
    configGetInt(config, section, "nextPageButtonY", &offsets->nextPageButtonY);
    configGetInt(config, section, "nextPageButtonWidth", &offsets->nextPageButtonWidth);
    configGetInt(config, section, "nextPageButtonHeight", &offsets->nextPageButtonHeight);
    configGetInt(config, section, "prevPageButtonX", &offsets->prevPageButtonX);
    configGetInt(config, section, "prevPageButtonY", &offsets->prevPageButtonY);
    configGetInt(config, section, "prevPageButtonWidth", &offsets->prevPageButtonWidth);
    configGetInt(config, section, "prevPageButtonHeight", &offsets->prevPageButtonHeight);

    // Info‐box text block
    configGetInt(config, section, "infoBoxTextBlockY", &offsets->infoBoxTextBlockY);

    // Cover image parameters
    configGetInt(config, section, "coverWidth", &offsets->coverWidth);
    configGetInt(config, section, "coverHeight", &offsets->coverHeight);
    configGetInt(config, section, "coverX", &offsets->coverX);
    configGetInt(config, section, "coverY", &offsets->coverY);
    configGetInt(config, section, "coverPitch", &offsets->coverPitch);

    // Slot text padding & pagination text
    configGetInt(config, section, "slotTextPadding", &offsets->slotTextPadding);
    configGetInt(config, section, "backTextOffsetX", &offsets->backTextOffsetX);
    configGetInt(config, section, "moreTextOffsetX", &offsets->moreTextOffsetX);
}

void applyConfigToPreferencesOffsets(Config* config, char const* section, PreferencesOffsets* offsets)
{
    // Window
    configGetInt(config, section, "width", &offsets->width);
    configGetInt(config, section, "height", &offsets->height);

    // Primary preferences (left column)
    configGetInt(config, section, "primaryColumnX", &offsets->primaryColumnX);
    configGetInt(config, section, "primaryKnobX", &offsets->primaryKnobX);
    configGetIntArray(config, section, "primaryKnobY", offsets->primaryKnobY, PRIMARY_PREF_COUNT);
    configGetIntArray(config, section, "primaryLabelY", offsets->primaryLabelY, PRIMARY_PREF_COUNT);

    // Secondary preferences (middle column)
    configGetInt(config, section, "secondaryColumnX", &offsets->secondaryColumnX);
    configGetInt(config, section, "secondaryKnobX", &offsets->secondaryKnobX);
    configGetIntArray(config, section, "secondaryKnobY", offsets->secondaryKnobY, SECONDARY_PREF_COUNT);
    configGetIntArray(config, section, "secondaryLabelY", offsets->secondaryLabelY, SECONDARY_PREF_COUNT);

    // ─── NEW: Tertiary preferences (middle‑right column) ───
    configGetInt(config, section, "tertiaryColumnX", &offsets->tertiaryColumnX);
    configGetInt(config, section, "tertiaryKnobX", &offsets->tertiaryKnobX);
    configGetIntArray(config, section, "tertiaryKnobY", offsets->tertiaryKnobY, TERTIARY_PREF_COUNT);
    configGetIntArray(config, section, "tertiaryLabelY", offsets->tertiaryLabelY, TERTIARY_PREF_COUNT);

    // Range preferences (right column)
    configGetInt(config, section, "rangeColumnX", &offsets->rangeColumnX);
    configGetInt(config, section, "rangeKnobX", &offsets->rangeKnobX);
    configGetIntArray(config, section, "rangeKnobY", offsets->rangeKnobY, RANGE_PREF_COUNT);

    // Label columns
    configGetInt(config, section, "primLabelColX", &offsets->primLabelColX);
    configGetInt(config, section, "secLabelColX", &offsets->secLabelColX);
    configGetInt(config, section, "terLabelColX", &offsets->terLabelColX); // <— tertiary
    configGetInt(config, section, "rangLabelColX", &offsets->rangLabelColX);

    // Label X‑arrays
    configGetIntArray(config, section, "labelX", offsets->labelX, PRIMARY_PREF_COUNT);
    configGetIntArray(config, section, "secondaryLabelX", offsets->secondaryLabelX, SECONDARY_PREF_COUNT);
    configGetIntArray(config, section, "tertiaryLabelX", offsets->tertiaryLabelX, TERTIARY_PREF_COUNT); // <— tertiary

    // Range control
    configGetInt(config, section, "rangeStartX", &offsets->rangeStartX);
    configGetInt(config, section, "rangeWidth", &offsets->rangeWidth);
    configGetInt(config, section, "knobWidth", &offsets->knobWidth);
    configGetIntArray(config, section, "rangeLabelX", offsets->rangeLabelX, RANGE_PREF_COUNT);

    // Blit dimensions
    configGetInt(config, section, "primaryBlitWidth", &offsets->primaryBlitWidth);
    configGetInt(config, section, "primaryBlitHeight", &offsets->primaryBlitHeight);
    configGetInt(config, section, "secondaryBlitWidth", &offsets->secondaryBlitWidth);
    configGetInt(config, section, "secondaryBlitHeight", &offsets->secondaryBlitHeight);
    configGetInt(config, section, "tertiaryBlitWidth", &offsets->tertiaryBlitWidth); // <— tertiary
    configGetInt(config, section, "tertiaryBlitHeight", &offsets->tertiaryBlitHeight); // <— tertiary
    configGetInt(config, section, "rangeBlitWidth", &offsets->rangeBlitWidth);
    configGetInt(config, section, "rangeBlitHeight", &offsets->rangeBlitHeight);

    // Title & button labels
    configGetInt(config, section, "titleTextX", &offsets->titleTextX);
    configGetInt(config, section, "titleTextY", &offsets->titleTextY);
    configGetInt(config, section, "defaultLabelX", &offsets->defaultLabelX);
    configGetInt(config, section, "defaultLabelY", &offsets->defaultLabelY);
    configGetInt(config, section, "doneLabelX", &offsets->doneLabelX);
    configGetInt(config, section, "doneLabelY", &offsets->doneLabelY);
    configGetInt(config, section, "cancelLabelX", &offsets->cancelLabelX);
    configGetInt(config, section, "cancelLabelY", &offsets->cancelLabelY);
    configGetInt(config, section, "speedLabelX", &offsets->speedLabelX);
    configGetInt(config, section, "speedLabelY", &offsets->speedLabelY);

    // Button positions
    configGetInt(config, section, "defaultButtonX", &offsets->defaultButtonX);
    configGetInt(config, section, "defaultButtonY", &offsets->defaultButtonY);
    configGetInt(config, section, "doneButtonX", &offsets->doneButtonX);
    configGetInt(config, section, "doneButtonY", &offsets->doneButtonY);
    configGetInt(config, section, "cancelButtonX", &offsets->cancelButtonX);
    configGetInt(config, section, "cancelButtonY", &offsets->cancelButtonY);

    // Checkbox
    configGetInt(config, section, "playerSpeedCheckboxX", &offsets->playerSpeedCheckboxX);
    configGetInt(config, section, "playerSpeedCheckboxY", &offsets->playerSpeedCheckboxY);

    // ─── Hit‑area offsets ───
    configGetInt(config, section, "primaryKnobHitX", &offsets->primaryKnobHitX);
    configGetInt(config, section, "primaryKnobHitY", &offsets->primaryKnobHitY);
    configGetInt(config, section, "secondaryKnobHitX", &offsets->secondaryKnobHitX);
    configGetInt(config, section, "secondaryKnobHitY", &offsets->secondaryKnobHitY);
    configGetInt(config, section, "tertiaryKnobHitX", &offsets->tertiaryKnobHitX); // <— tertiary
    configGetInt(config, section, "tertiaryKnobHitY", &offsets->tertiaryKnobHitY); // <— tertiary

    // Range slider parameters
    configGetInt(config, section, "rangeSliderMinX", &offsets->rangeSliderMinX);
    configGetInt(config, section, "rangeSliderMaxX", &offsets->rangeSliderMaxX);
    configGetInt(config, section, "rangeSliderWidth", &offsets->rangeSliderWidth);

    // Button hitbox offsets
    configGetInt(config, section, "primaryButtonOffsetY", &offsets->primaryButtonOffsetY);
    configGetInt(config, section, "secondaryButtonOffsetY", &offsets->secondaryButtonOffsetY);
    configGetInt(config, section, "tertiaryButtonOffsetY", &offsets->tertiaryButtonOffsetY); // <— tertiary
    configGetInt(config, section, "rangeButtonOffsetY", &offsets->rangeButtonOffsetY);

    // Text delay & extra labels
    configGetDouble(config, section, "textBaseDelayScale", &offsets->textBaseDelayScale);
    configGetInt(config, section, "rangeLabel4Option1X", &offsets->rangeLabel4Option1X);
    configGetInt(config, section, "rangeLabel4Option2X", &offsets->rangeLabel4Option2X);

    // Position arrays
    configGetIntArray(config, section, "row1Ytab", offsets->row1Ytab, PRIMARY_PREF_COUNT);
    configGetIntArray(config, section, "row2Ytab", offsets->row2Ytab, SECONDARY_PREF_COUNT);
    configGetIntArray(config, section, "row2bYtab", offsets->row2bYtab, TERTIARY_PREF_COUNT); // <— tertiary
    configGetIntArray(config, section, "row3Ytab", offsets->row3Ytab, RANGE_PREF_COUNT);

    configGetIntArray(config, section, "optionXOffsets", offsets->optionXOffsets, PRIMARY_OPTION_VALUE_COUNT);
    configGetIntArray(config, section, "optionYOffsets", offsets->optionYOffsets, PRIMARY_OPTION_VALUE_COUNT);
    configGetIntArray(config, section, "secondaryOptionXOffsets", offsets->secondaryOptionXOffsets, SECONDARY_OPTION_VALUE_COUNT);
    configGetIntArray(config, section, "tertiaryOptionYOffsets", offsets->tertiaryOptionYOffsets, TERTIARY_OPTION_VALUE_COUNT); // <— tertiary

    // Label‑value arrays
    configGetIntArray(config, section, "primaryLabelYValues", offsets->primaryLabelYValues, PRIMARY_PREF_COUNT);
    configGetIntArray(config, section, "secondaryLabelYValues", offsets->secondaryLabelYValues, SECONDARY_PREF_COUNT);
    configGetIntArray(config, section, "tertiaryLabelYValues", offsets->tertiaryLabelYValues, TERTIARY_PREF_COUNT); // <— tertiary

    // Preference‑position [x,y] pairs
    for (int i = 0; i < PREF_COUNT; i++) {
        char key[64];
        snprintf(key, sizeof(key), "preferencePositions%dX", i);
        configGetInt(config, section, key, &offsets->preferencePositions[i].x);
        snprintf(key, sizeof(key), "preferencePositions%dY", i);
        configGetInt(config, section, key, &offsets->preferencePositions[i].y);
    }
}

void applyConfigToWorldmapOffsets(Config* config, const char* section, WorldmapOffsets* offsets)
{
    // Window size
    configGetInt(config, section, "windowWidth", &offsets->windowWidth);
    configGetInt(config, section, "windowHeight", &offsets->windowHeight);

    // Viewport
    configGetInt(config, section, "viewX", &offsets->viewX);
    configGetInt(config, section, "viewY", &offsets->viewY);
    configGetInt(config, section, "viewWidth", &offsets->viewWidth);
    configGetInt(config, section, "viewHeight", &offsets->viewHeight);

    // UI elements
    configGetInt(config, section, "dialX", &offsets->dialX);
    configGetInt(config, section, "dialY", &offsets->dialY);
    configGetInt(config, section, "scrollUpX", &offsets->scrollUpX);
    configGetInt(config, section, "scrollUpY", &offsets->scrollUpY);
    configGetInt(config, section, "scrollDownX", &offsets->scrollDownX);
    configGetInt(config, section, "scrollDownY", &offsets->scrollDownY);
    configGetInt(config, section, "globeOverlayX", &offsets->globeOverlayX);
    configGetInt(config, section, "globeOverlayY", &offsets->globeOverlayY);
    configGetInt(config, section, "carX", &offsets->carX);
    configGetInt(config, section, "carY", &offsets->carY);
    configGetInt(config, section, "carOverlayX", &offsets->carOverlayX);
    configGetInt(config, section, "carOverlayY", &offsets->carOverlayY);
    configGetInt(config, section, "carFuelBarX", &offsets->carFuelBarX);
    configGetInt(config, section, "carFuelBarY", &offsets->carFuelBarY);
    configGetInt(config, section, "carFuelBarHeight", &offsets->carFuelBarHeight);
    configGetInt(config, section, "townWorldSwitchX", &offsets->townWorldSwitchX);
    configGetInt(config, section, "townWorldSwitchY", &offsets->townWorldSwitchY);

    // Scroll area
    configGetInt(config, section, "scrollAreaX", &offsets->scrollAreaX);
    configGetInt(config, section, "scrollAreaY", &offsets->scrollAreaY);

    // Destination list
    configGetInt(config, section, "destListX", &offsets->destListX);
    configGetInt(config, section, "destListFirstY", &offsets->destListFirstY);
    configGetInt(config, section, "destListSpacing", &offsets->destListSpacing);

    // Date display
    configGetInt(config, section, "dateDisplayX", &offsets->dateDisplayX);
    configGetInt(config, section, "dateDisplayY", &offsets->dateDisplayY);
    configGetInt(config, section, "dateDisplayWidth", &offsets->dateDisplayWidth);

    // Viewport boundaries
    configGetInt(config, section, "viewportMaxX", &offsets->viewportMaxX);
    configGetInt(config, section, "viewportMaxY", &offsets->viewportMaxY);

    // City name drawing
    configGetInt(config, section, "cityNameMaxY", &offsets->cityNameMaxY);

    // Subtile boundaries
    configGetInt(config, section, "subtileViewportMaxX", &offsets->subtileViewportMaxX);
    configGetInt(config, section, "subtileViewportMaxY", &offsets->subtileViewportMaxY);

    // Town map positions
    configGetInt(config, section, "townMapBgX", &offsets->townMapBgX);
    configGetInt(config, section, "townMapBgY", &offsets->townMapBgY);
    configGetInt(config, section, "townMapImageX", &offsets->townMapImageX);
    configGetInt(config, section, "townMapImageY", &offsets->townMapImageY);
    configGetInt(config, section, "townMapButtonXOffset", &offsets->townMapButtonXOffset);
    configGetInt(config, section, "townMapButtonYOffset", &offsets->townMapButtonYOffset);
    configGetInt(config, section, "townMapLabelXOffset", &offsets->townMapLabelXOffset);
    configGetInt(config, section, "townMapLabelYOffset", &offsets->townMapLabelYOffset);

    configGetInt(config, section, "townBackgroundWidth", &offsets->townBackgroundWidth);
    configGetInt(config, section, "townBackgroundHeight", &offsets->townBackgroundHeight);

    // Map center
    configGetInt(config, section, "mapcenterX", &offsets->mapcenterX);
    configGetInt(config, section, "mapcenterY", &offsets->mapcenterY);
}

} // namespace fallout