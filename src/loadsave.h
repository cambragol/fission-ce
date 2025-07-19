#ifndef LOAD_SAVE_GAME_H
#define LOAD_SAVE_GAME_H

namespace fallout {

typedef enum LoadSaveMode {
    // Special case - loading game from main menu.
    LOAD_SAVE_MODE_FROM_MAIN_MENU,

    // Normal (full-screen) save/load screen.
    LOAD_SAVE_MODE_NORMAL,

    // Quick load/save.
    LOAD_SAVE_MODE_QUICK,
} LoadSaveMode;

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

void _InitLoadSave();
void _ResetLoadSave();
int lsgSaveGame(int mode);
int lsgLoadGame(int mode);
bool _isLoadingGame();
void lsgInit();
int MapDirErase(const char* path, const char* extension);
int _MapDirEraseFile_(const char* a1, const char* a2);

} // namespace fallout

#endif /* LOAD_SAVE_GAME_H */
