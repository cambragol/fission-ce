#ifndef FALLOUT_MAINMENU_H_
#define FALLOUT_MAINMENU_H_

#include "config.h" // For Config type

namespace fallout {

typedef enum MainMenuOption {
    MAIN_MENU_INTRO,
    MAIN_MENU_NEW_GAME,
    MAIN_MENU_LOAD_GAME,
    MAIN_MENU_SCREENSAVER,
    MAIN_MENU_TIMEOUT,
    MAIN_MENU_CREDITS,
    MAIN_MENU_QUOTES,
    MAIN_MENU_EXIT,
    MAIN_MENU_SELFRUN,
    MAIN_MENU_OPTIONS,
} MainMenuOption;

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

int mainMenuWindowInit();
void mainMenuWindowFree();
void mainMenuWindowHide(bool animate);
void mainMenuWindowUnhide(bool animate);
int _main_menu_is_enabled();
int mainMenuWindowHandleEvents();
bool mainMenuLoadOffsetsFromConfig(MainMenuOffsets* offsets, bool isWidescreen);
void mainMenuWriteDefaultOffsetsToConfig(bool isWidescreen, const MainMenuOffsets* defaults);

extern const MainMenuOffsets gMainMenuOffsets640;
extern const MainMenuOffsets gMainMenuOffsets800;

} // namespace fallout

#endif /* FALLOUT_MAINMENU_H_ */
