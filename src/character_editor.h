#ifndef CHARACTER_EDITOR_H
#define CHARACTER_EDITOR_H

#include "db.h"

namespace fallout {

extern int gCharacterEditorRemainingCharacterPoints;

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

int characterEditorShow(bool isCreationMode);
void characterEditorInit();
bool _isdoschar(int ch);
char* _strmfe(char* dest, const char* name, const char* ext);
int characterEditorSave(File* stream);
int characterEditorLoad(File* stream);
void characterEditorReset();

} // namespace fallout

#endif /* CHARACTER_EDITOR_H */
