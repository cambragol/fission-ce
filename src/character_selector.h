#ifndef CHARACTER_SELECTOR_H
#define CHARACTER_SELECTOR_H

namespace fallout {

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

int characterSelectorOpen();

void premadeCharactersInit();
void premadeCharactersExit();
bool characterSelectorLoadOffsetsFromConfig(CharacterSelectorOffsets* offsets, bool isWidescreen);
void characterSelectorWriteDefaultOffsetsToConfig(bool isWidescreen, const CharacterSelectorOffsets* defaults);

extern CharacterSelectorOffsets gCurrentCharSelectorOffsets;

} // namespace fallout

#endif /* CHARACTER_SELECTOR_H */
