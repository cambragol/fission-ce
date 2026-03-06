#include "game.h"
#include "platform/git_version.h"

#include <stdio.h>
#include <string.h>

#include "actions.h"
#include "animation.h"
#include "art.h"
#include "automap.h"
#include "character_editor.h"
#include "character_selector.h"
#include "color.h"
#include "combat.h"
#include "combat_ai.h"
#include "critter.h"
#include "cycle.h"
#include "db.h"
#include "dbox.h"
#include "debug.h"
#include "display_monitor.h"
#include "draw.h"
#include "endgame.h"
#include "font_manager.h"
#include "game_dialog.h"
#include "game_memory.h"
#include "game_mouse.h"
#include "game_movie.h"
#include "game_sound.h"
#include "input.h"
#include "interface.h"
#include "inventory.h"
#include "item.h"
#include "kb.h"
#include "loadsave.h"
#include "map.h"
#include "memory.h"
#include "mouse.h"
#include "movie.h"
#include "movie_effect.h"
#include "object.h"
#include "options.h"
#include "palette.h"
#include "party_member.h"
#include "perk.h"
#include "pipboy.h"
#include "platform_compat.h"
#include "preferences.h"
#include "proto.h"
#include "queue.h"
#include "random.h"
#include "scripts.h"
#include "settings.h"
#include "sfall_arrays.h"
#include "sfall_config.h"
#include "sfall_ext.h"
#include "sfall_global_scripts.h"
#include "sfall_global_vars.h"
#include "sfall_ini.h"
#include "sfall_lists.h"
#include "skill.h"
#include "skilldex.h"
#include "stat.h"
#include "svga.h"
#include "text_font.h"
#include "tile.h"
#include "trait.h"
#include "version.h"
#include "window_manager.h"
#include "window_manager_private.h"
#include "worldmap.h"

namespace fallout {

#define DIR_SEPARATOR '/'

#define HELP_SCREEN_WIDTH (640)
#define HELP_SCREEN_HEIGHT (480)

#define SPLASH_WIDTH (640)
#define SPLASH_HEIGHT (480)
#define SPLASH_COUNT (10)

static int gameLoadGlobalVars();
static int gameTakeScreenshot(int width, int height, unsigned char* buffer, unsigned char* palette);
static void gameFreeGlobalVars();
static void showHelp();
static int gameDbInit();
static void showSplash();
static int loadModGlobalVars();
static void generateGVarReport();

// 0x501C9C
static char _aGame_0[] = "game\\";

// 0x5020B8
static char _aBuildDate[] = _BUILD_DATE;
static char _aBuildHash[] = _BUILD_HASH;

// 0x5186B4
static bool gGameUiDisabled = false;

// 0x5186B8
static int gGameState = GAME_STATE_0;

// 0x5186BC
static bool gIsMapper = false;

// 0x5186C0
int* gGameGlobalVars = nullptr;

// 0x5186C4
int gGameGlobalVarsLength = 0;

// global for all strictVanilla controls
bool gStrictVanillaEnabled = false;

// 0x5186C8
const char* asc_5186C8 = _aGame_0;

// 0x5186CC
int _game_user_wants_to_quit = 0;

int gGameGlobalVarsVanillaCount = 0;

typedef struct {
    char modName[64];
    char symbol[128];
    int index;
    int defaultValue;
} ModGVarInfo;

static ModGVarInfo gModGVarInfos[MOD_GVAR_COUNT];
static int gModGVarInfoCount = 0;
static bool gGVarCollisionOccurred = false;
static char gGVarCollisionDetails[MOD_GVAR_COUNT][256] = { { 0 } };
static char gModGVarSlotOwners[MOD_GVAR_COUNT][128]; // for collision messages

// misc.msg
//
// 0x58E940
MessageList gMiscMessageList;

// CE: Sonora folks like to store objects in global variables.
static void** gGameGlobalPointers = nullptr;

// 0x442580
int gameInitWithOptions(const char* windowTitle, bool isMapper, int font, int flags, int argc, char** argv)
{
    char path[COMPAT_MAX_PATH];

    if (gameMemoryInit() == -1) {
        return -1;
    }

    // Sfall config should be initialized before game config, since it can
    // override it's file name.
    sfallConfigInit(argc, argv);

    settingsInit(isMapper, argc, argv);

    gStrictVanillaEnabled = settings.sfall_misc.strict_vanilla;
    gIsMapper = isMapper;

    if (gameDbInit() == -1) {
        settingsExit(false);
        sfallConfigExit();
        return -1;
    }

    // Message list repository is considered a specialized file manager, so
    // it should be initialized early in the process.
    messageListRepositoryInit();

    programWindowSetTitle(windowTitle);
    _initWindow(1, flags);
    paletteInit();

    const char* language = settings.system.language.c_str();
    if (compat_stricmp(language, FRENCH) == 0) {
        keyboardSetLayout(KEYBOARD_LAYOUT_FRENCH);
    } else if (compat_stricmp(language, GERMAN) == 0) {
        keyboardSetLayout(KEYBOARD_LAYOUT_GERMAN);
    } else if (compat_stricmp(language, ITALIAN) == 0) {
        keyboardSetLayout(KEYBOARD_LAYOUT_ITALIAN);
    } else if (compat_stricmp(language, SPANISH) == 0) {
        keyboardSetLayout(KEYBOARD_LAYOUT_SPANISH);
    }

    // SFALL: Allow to skip splash screen
    int skipOpeningMovies = 0;
    configGetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_SKIP_OPENING_MOVIES_KEY, &skipOpeningMovies);

    // load preferences before Splash screen to get proper brightness
    if (_init_options_menu() != 0) {
        debugPrint("Failed on init_options_menu\n");
        return -1;
    }

    debugPrint(">init_options_menu\n");

    if ((!gIsMapper && skipOpeningMovies < 2) || gStrictVanillaEnabled) {

        if (gameIsWidescreen()) {
            resizeContent(800, 500);
        } else {
            resizeContent(640, 480);
        }
        // resizeContent(640, 480);
        showSplash();
    }

    // CE: Handle debug mode (exactly as seen in `mapper2.exe`).
    const char* debugMode = settings.debug.mode.c_str();
    if (compat_stricmp(debugMode, "environment") == 0) {
        _debug_register_env();
    } else if (compat_stricmp(debugMode, "screen") == 0) {
        _debug_register_screen();
    } else if (compat_stricmp(debugMode, "log") == 0) {
        _debug_register_log("debug.log", "wt");
    } else if (compat_stricmp(debugMode, "mono") == 0) {
        _debug_register_mono();
    } else if (compat_stricmp(debugMode, "gnw") == 0) {
        _debug_register_func(_win_debug);
    }

    interfaceFontsInit();
    fontManagerAdd(&gModernFontManager);
    fontSetCurrent(font);

    screenshotHandlerConfigure(KEY_F12, gameTakeScreenshot);

    tileDisable();

    randomInit();
    badwordsInit();
    skillsInit();
    statsInit();

    if (partyMembersInit() != 0) {
        debugPrint("Failed on partyMember_init\n");
        return -1;
    }

    perksInit();
    traitsInit();
    itemsInit();
    queueInit();
    critterInit();
    aiInit();
    inventoryResetDude();

    if (gameSoundInit() != 0) {
        debugPrint("Sound initialization failed.\n");
    }

    debugPrint(">gsound_init\t");

    movieInit();
    debugPrint(">initMovie\t\t");

    if (gameMoviesInit() != 0) {
        debugPrint("Failed on gmovie_init\n");
        return -1;
    }

    debugPrint(">gmovie_init\t");

    if (movieEffectsInit() != 0) {
        debugPrint("Failed on moviefx_init\n");
        return -1;
    }

    debugPrint(">moviefx_init\t");

    if (isoInit() != 0) {
        debugPrint("Failed on iso_init\n");
        return -1;
    }

    debugPrint(">iso_init\t");

    if (gameMouseInit() != 0) {
        debugPrint("Failed on gmouse_init\n");
        return -1;
    }

    debugPrint(">gmouse_init\t");

    if (protoInit() != 0) {
        debugPrint("Failed on proto_init\n");
        return -1;
    }

    debugPrint(">proto_init\t");

    animationInit();
    debugPrint(">anim_init\t");

    if (scriptsInit() != 0) {
        debugPrint("Failed on scr_init\n");
        return -1;
    }

    debugPrint(">scr_init\t");

    if (gameLoadGlobalVars() != 0) {
        debugPrint("Failed on game_load_info\n");
        return -1;
    }
    loadModGlobalVars();

    debugPrint(">game_load_info\t");

    if (_scr_game_init() != 0) {
        debugPrint("Failed on scr_game_init\n");
        return -1;
    }

    debugPrint(">scr_game_init\t");

    if (wmWorldMap_init() != 0) {
        debugPrint("Failed on wmWorldMap_init\n");
        return -1;
    }

    debugPrint(">wmWorldMap_init\t");

    characterEditorInit();
    debugPrint(">CharEditInit\t");

    pipboyInit();
    debugPrint(">pip_init\t\t");

    _InitLoadSave();
    lsgInit();
    debugPrint(">InitLoadSave\t");

    if (gameDialogInit() != 0) {
        debugPrint("Failed on gdialog_init\n");
        return -1;
    }

    debugPrint(">gdialog_init\t");

    if (combatInit() != 0) {
        debugPrint("Failed on combat_init\n");
        return -1;
    }

    debugPrint(">combat_init\t");

    if (automapInit() != 0) {
        debugPrint("Failed on automap_init\n");
        return -1;
    }

    debugPrint(">automap_init\t");

    if (!messageListInit(&gMiscMessageList)) {
        debugPrint("Failed on message_init\n");
        return -1;
    }

    debugPrint(">message_init\t");

    snprintf(path, sizeof(path), "%s%s", asc_5186C8, "misc.msg");

    if (!messageListLoad(&gMiscMessageList, path)) {
        debugPrint("Failed on message_load\n");
        return -1;
    }

    debugPrint(">message_load\t");

    if (scriptsDisable() != 0) {
        debugPrint("Failed on scr_disable\n");
        return -1;
    }

    debugPrint(">scr_disable\t");

    if (endgameDeathEndingInit() != 0) {
        debugPrint("Failed on endgameDeathEndingInit");
        return -1;
    }

    debugPrint(">endgameDeathEndingInit\n");

    // SFALL
    premadeCharactersInit();

    if (!sfall_gl_vars_init()) {
        debugPrint("Failed on sfall_gl_vars_init");
        return -1;
    }

    if (!sfallListsInit()) {
        debugPrint("Failed on sfallListsInit");
        return -1;
    }

    if (!sfallArraysInit()) {
        debugPrint("Failed on sfallArraysInit");
        return -1;
    }

    if (!sfall_gl_scr_init()) {
        debugPrint("Failed on sfall_gl_scr_init");
        return -1;
    }

    char* customConfigBasePath;
    configGetString(&gSfallConfig, SFALL_CONFIG_SCRIPTS_KEY, SFALL_CONFIG_INI_CONFIG_FOLDER, &customConfigBasePath);
    sfall_ini_set_base_path(customConfigBasePath);

    messageListRepositorySetStandardMessageList(STANDARD_MESSAGE_LIST_MISC, &gMiscMessageList);

    return 0;
}

// 0x442B84
void gameReset()
{
    tileDisable();
    paletteReset();
    randomReset();
    skillsReset();
    statsReset();
    perksReset();
    traitsReset();
    itemsReset();
    queueExit();
    animationReset();
    lsgInit();
    critterReset();
    aiReset();
    inventoryResetDude();
    gameSoundReset();
    _movieStop();
    movieEffectsReset();
    gameMoviesReset();
    isoReset();
    gameMouseReset();
    protoReset();
    _scr_reset();
    gameLoadGlobalVars();
    scriptsReset();
    wmWorldMap_reset();
    partyMembersReset();
    characterEditorInit();
    pipboyReset();
    _ResetLoadSave();
    gameDialogReset();
    combatReset();
    _game_user_wants_to_quit = 0;
    automapReset();
    _init_options_menu();

    // SFALL
    sfall_gl_vars_reset();
    sfallListsReset();
    messageListRepositoryReset();
    sfallArraysReset();
    sfall_gl_scr_reset();
}

// 0x442C34
void gameExit()
{
    debugPrint("\nGame Exit\n");

    // SFALL
    sfall_gl_scr_exit();
    sfallArraysExit();
    sfallListsExit();
    sfall_gl_vars_exit();
    premadeCharactersExit();

    tileDisable();
    messageListRepositorySetStandardMessageList(STANDARD_MESSAGE_LIST_MISC, nullptr);
    messageListFree(&gMiscMessageList);
    combatExit();
    gameDialogExit();
    _scr_game_exit();

    // NOTE: Uninline.
    gameFreeGlobalVars();

    scriptsExit();
    animationExit();
    protoExit();
    gameMouseExit();
    isoExit();
    movieEffectsExit();
    movieExit();
    gameSoundExit();
    aiExit();
    critterExit();
    itemsExit();
    queueExit();
    perksExit();
    statsExit();
    skillsExit();
    traitsExit();
    randomExit();
    badwordsExit();
    automapExit();
    paletteExit();
    wmWorldMap_exit();
    partyMembersExit();
    endgameDeathEndingExit();
    interfaceFontsExit();
    windowClose();
    messageListRepositoryExit();
    dbExit();
    settingsExit(true);
    sfallConfigExit();
}

// 0x442D44
int gameHandleKey(int eventCode, bool isInCombatMode)
{
    // NOTE: Uninline.
    if (gameGetState() == GAME_STATE_5) {
        _gdialogSystemEnter();
    }

    if (eventCode == -1) {
        if ((mouseGetEvent() & MOUSE_EVENT_WHEEL) != 0) {
            int wheelX;
            int wheelY;
            mouseGetWheel(&wheelX, &wheelY);

            int dx = 0;
            if (wheelX > 0) {
                dx = 1;
            } else if (wheelX < 0) {
                dx = -1;
            }

            int dy = 0;
            if (wheelY > 0) {
                dy = -1;
            } else if (wheelY < 0) {
                dy = 1;
            }

            mapScroll(dx, dy);
        }
        return 0;
    }

    if (eventCode == -2) {
        int mouseState = mouseGetEvent();
        int mouseX;
        int mouseY;
        mouseGetPosition(&mouseX, &mouseY);

        if ((mouseState & MOUSE_EVENT_LEFT_BUTTON_DOWN) != 0) {
            if ((mouseState & MOUSE_EVENT_LEFT_BUTTON_REPEAT) == 0) {
                if (mouseX == _scr_size.left || mouseX == _scr_size.right
                    || mouseY == _scr_size.top || mouseY == _scr_size.bottom) {
                    _gmouse_clicked_on_edge = true;
                } else {
                    _gmouse_clicked_on_edge = false;
                }
            }
        } else {
            if ((mouseState & MOUSE_EVENT_LEFT_BUTTON_UP) != 0) {
                _gmouse_clicked_on_edge = false;
            }
        }

        _gmouse_handle_event(mouseX, mouseY, mouseState);
        return 0;
    }

    if (_gmouse_is_scrolling()) {
        return 0;
    }

    switch (eventCode) {
    case -20:
        if (interfaceBarEnabled()) {
            _intface_use_item();
        }
        break;
    case -2:
        if (1) {
            int mouseEvent = mouseGetEvent();
            int mouseX;
            int mouseY;
            mouseGetPosition(&mouseX, &mouseY);

            if ((mouseEvent & MOUSE_EVENT_LEFT_BUTTON_DOWN) != 0) {
                if ((mouseEvent & MOUSE_EVENT_LEFT_BUTTON_REPEAT) == 0) {
                    if (mouseX == _scr_size.left || mouseX == _scr_size.right
                        || mouseY == _scr_size.top || mouseY == _scr_size.bottom) {
                        _gmouse_clicked_on_edge = true;
                    } else {
                        _gmouse_clicked_on_edge = false;
                    }
                }
            } else {
                if ((mouseEvent & MOUSE_EVENT_LEFT_BUTTON_UP) != 0) {
                    _gmouse_clicked_on_edge = false;
                }
            }

            _gmouse_handle_event(mouseX, mouseY, mouseEvent);
        }
        break;
    case KEY_CTRL_Q:
    case KEY_CTRL_X:
    case KEY_F10:
        soundPlayFile("ib1p1xx1");
        showQuitConfirmationDialog();
        break;
    case KEY_TAB:
        if (interfaceBarEnabled()
            && gPressedPhysicalKeys[SDL_SCANCODE_LALT] == 0
            && gPressedPhysicalKeys[SDL_SCANCODE_RALT] == 0) {
            soundPlayFile("ib1p1xx1");
            automapShow(true, false);
        }
        break;
    case KEY_CTRL_P:
        soundPlayFile("ib1p1xx1");
        showPause(false);
        break;
    case KEY_UPPERCASE_A:
    case KEY_LOWERCASE_A:
        if (interfaceBarEnabled()) {
            if (!isInCombatMode) {
                _combat(nullptr);
            }
        }
        break;
    case KEY_UPPERCASE_N:
    case KEY_LOWERCASE_N:
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");
            interfaceCycleItemAction();
        }
        break;
    case KEY_UPPERCASE_M:
    case KEY_LOWERCASE_M:
        gameMouseCycleMode();
        break;
    case KEY_UPPERCASE_B:
    case KEY_LOWERCASE_B:
        // change active hand
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");
            interfaceBarSwapHands(true);
        }
        break;
    case KEY_UPPERCASE_C:
    case KEY_LOWERCASE_C:
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");
            bool isoWasEnabled = isoDisable();
            characterEditorShow(false);
            if (isoWasEnabled) {
                isoEnable();
            }
        }
        break;
    case KEY_UPPERCASE_I:
    case KEY_LOWERCASE_I:
        // open inventory
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");
            inventoryOpen();
        }
        break;
    case KEY_ESCAPE:
    case KEY_UPPERCASE_O:
    case KEY_LOWERCASE_O:
        // options
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");
            showOptions();
        }
        break;
    case KEY_UPPERCASE_P:
    case KEY_LOWERCASE_P:
        // pipboy
        if (interfaceBarEnabled()) {
            if (isInCombatMode) {
                soundPlayFile("iisxxxx1");

                // Pipboy not available in combat!
                MessageListItem messageListItem;
                char title[128];
                strcpy(title, getmsg(&gMiscMessageList, &messageListItem, 7));
                showDialogBox(title, nullptr, 0, 192, 116, _colorTable[32328], nullptr, _colorTable[32328], 0);
            } else {
                soundPlayFile("ib1p1xx1");
                pipboyOpen(PIPBOY_OPEN_INTENT_UNSPECIFIED);
            }
        }
        break;
    case KEY_UPPERCASE_S:
    case KEY_LOWERCASE_S:
        // skilldex
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");

            int mode = -1;

            // NOTE: There is an `inc` for this value to build jump table which
            // is not needed.
            int rc = skilldexOpen();

            // Remap Skilldex result code to action.
            switch (rc) {
            case SKILLDEX_RC_ERROR:
                debugPrint("\n ** Error calling skilldex_select()! ** \n");
                break;
            case SKILLDEX_RC_SNEAK:
                _action_skill_use(SKILL_SNEAK);
                break;
            case SKILLDEX_RC_LOCKPICK:
                mode = GAME_MOUSE_MODE_USE_LOCKPICK;
                break;
            case SKILLDEX_RC_STEAL:
                mode = GAME_MOUSE_MODE_USE_STEAL;
                break;
            case SKILLDEX_RC_TRAPS:
                mode = GAME_MOUSE_MODE_USE_TRAPS;
                break;
            case SKILLDEX_RC_FIRST_AID:
                mode = GAME_MOUSE_MODE_USE_FIRST_AID;
                break;
            case SKILLDEX_RC_DOCTOR:
                mode = GAME_MOUSE_MODE_USE_DOCTOR;
                break;
            case SKILLDEX_RC_SCIENCE:
                mode = GAME_MOUSE_MODE_USE_SCIENCE;
                break;
            case SKILLDEX_RC_REPAIR:
                mode = GAME_MOUSE_MODE_USE_REPAIR;
                break;
            default:
                break;
            }

            if (mode != -1) {
                gameMouseSetCursor(MOUSE_CURSOR_USE_CROSSHAIR);
                gameMouseSetMode(mode);
            }
        }
        break;
    case KEY_UPPERCASE_Z:
    case KEY_LOWERCASE_Z:
        if (interfaceBarEnabled()) {
            if (isInCombatMode) {
                soundPlayFile("iisxxxx1");

                // Pipboy not available in combat!
                MessageListItem messageListItem;
                char title[128];
                strcpy(title, getmsg(&gMiscMessageList, &messageListItem, 7));
                showDialogBox(title, nullptr, 0, 192, 116, _colorTable[32328], nullptr, _colorTable[32328], 0);
            } else {
                soundPlayFile("ib1p1xx1");
                pipboyOpen(PIPBOY_OPEN_INTENT_REST);
            }
        }
        break;
    case KEY_HOME:
        if (gDude->elevation != gElevation) {
            mapSetElevation(gDude->elevation);
        }

        if (gIsMapper) {
            tileSetCenter(gDude->tile, TILE_SET_CENTER_REFRESH_WINDOW);
        } else {
            _tile_scroll_to(gDude->tile, 2);
        }

        break;
    case KEY_1:
    case KEY_EXCLAMATION:
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");
            gameMouseSetCursor(MOUSE_CURSOR_USE_CROSSHAIR);
            _action_skill_use(SKILL_SNEAK);
        }
        break;
    case KEY_2:
    case KEY_AT:
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");
            gameMouseSetCursor(MOUSE_CURSOR_USE_CROSSHAIR);
            gameMouseSetMode(GAME_MOUSE_MODE_USE_LOCKPICK);
        }
        break;
    case KEY_3:
    case KEY_NUMBER_SIGN:
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");
            gameMouseSetCursor(MOUSE_CURSOR_USE_CROSSHAIR);
            gameMouseSetMode(GAME_MOUSE_MODE_USE_STEAL);
        }
        break;
    case KEY_4:
    case KEY_DOLLAR:
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");
            gameMouseSetCursor(MOUSE_CURSOR_USE_CROSSHAIR);
            gameMouseSetMode(GAME_MOUSE_MODE_USE_TRAPS);
        }
        break;
    case KEY_5:
    case KEY_PERCENT:
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");
            gameMouseSetCursor(MOUSE_CURSOR_USE_CROSSHAIR);
            gameMouseSetMode(GAME_MOUSE_MODE_USE_FIRST_AID);
        }
        break;
    case KEY_6:
    case KEY_CARET:
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");
            gameMouseSetCursor(MOUSE_CURSOR_USE_CROSSHAIR);
            gameMouseSetMode(GAME_MOUSE_MODE_USE_DOCTOR);
        }
        break;
    case KEY_7:
    case KEY_AMPERSAND:
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");
            gameMouseSetCursor(MOUSE_CURSOR_USE_CROSSHAIR);
            gameMouseSetMode(GAME_MOUSE_MODE_USE_SCIENCE);
        }
        break;
    case KEY_8:
    case KEY_ASTERISK:
        if (interfaceBarEnabled()) {
            soundPlayFile("ib1p1xx1");
            gameMouseSetCursor(MOUSE_CURSOR_USE_CROSSHAIR);
            gameMouseSetMode(GAME_MOUSE_MODE_USE_REPAIR);
        }
        break;
    case KEY_MINUS:
    case KEY_UNDERSCORE:
        brightnessDecrease();
        break;
    case KEY_EQUAL:
    case KEY_PLUS:
        brightnessIncrease();
        break;
    case KEY_COMMA:
    case KEY_LESS:
        if (reg_anim_begin(ANIMATION_REQUEST_RESERVED) == 0) {
            animationRegisterRotateCounterClockwise(gDude);
            reg_anim_end();
        }
        break;
    case KEY_DOT:
    case KEY_GREATER:
        if (reg_anim_begin(ANIMATION_REQUEST_RESERVED) == 0) {
            animationRegisterRotateClockwise(gDude);
            reg_anim_end();
        }
        break;
    case KEY_SLASH:
    case KEY_QUESTION:
        if (1) {
            soundPlayFile("ib1p1xx1");

            int month;
            int day;
            int year;
            gameTimeGetDate(&month, &day, &year);

            MessageList messageList;
            if (messageListInit(&messageList)) {
                char path[COMPAT_MAX_PATH];
                snprintf(path, sizeof(path), "%s%s", asc_5186C8, "editor.msg");

                if (messageListLoad(&messageList, path)) {
                    MessageListItem messageListItem;
                    messageListItem.num = 500 + month - 1;
                    if (messageListGetItem(&messageList, &messageListItem)) {
                        char* time = gameTimeGetTimeString();

                        char date[128];
                        snprintf(date, sizeof(date), "%s: %d/%d %s", messageListItem.text, day, year, time);

                        displayMonitorAddMessage(date);
                    }
                }

                messageListFree(&messageList);
            }
        }
        break;
    case KEY_F1:
        soundPlayFile("ib1p1xx1");
        showHelp();
        break;
    case KEY_F2:
        gameSoundSetMasterVolume(gameSoundGetMasterVolume() - 2047);
        break;
    case KEY_F3:
        gameSoundSetMasterVolume(gameSoundGetMasterVolume() + 2047);
        break;
    case KEY_CTRL_S:
    case KEY_F4:
        soundPlayFile("ib1p1xx1");
        if (lsgSaveGame(1) == -1) {
            debugPrint("\n ** Error calling SaveGame()! **\n");
        }
        break;
    case KEY_CTRL_L:
    case KEY_F5:
        soundPlayFile("ib1p1xx1");
        if (lsgLoadGame(LOAD_SAVE_MODE_NORMAL) == -1) {
            debugPrint("\n ** Error calling LoadGame()! **\n");
        }
        break;
    case KEY_F6:
        if (1) {
            soundPlayFile("ib1p1xx1");

            int rc = lsgSaveGame(LOAD_SAVE_MODE_QUICK);
            if (rc == -1) {
                debugPrint("\n ** Error calling SaveGame()! **\n");
            } else if (rc == 1) {
                MessageListItem messageListItem;
                // Quick save game successfully saved.
                char* msg = getmsg(&gMiscMessageList, &messageListItem, 5);
                displayMonitorAddMessage(msg);
            }
        }
        break;
    case KEY_F7:
        if (1) {
            soundPlayFile("ib1p1xx1");

            int rc = lsgLoadGame(LOAD_SAVE_MODE_QUICK);
            if (rc == -1) {
                debugPrint("\n ** Error calling LoadGame()! **\n");
            } else if (rc == 1) {
                MessageListItem messageListItem;
                // Quick load game successfully loaded.
                char* msg = getmsg(&gMiscMessageList, &messageListItem, 4);
                displayMonitorAddMessage(msg);
            }
        }
        break;
    case KEY_CTRL_V:
        if (1) {
            soundPlayFile("ib1p1xx1");

            char version[VERSION_MAX];
            versionGetVersion(version, sizeof(version));
            displayMonitorAddMessage(version);
            displayMonitorAddMessage(_aBuildHash);
            displayMonitorAddMessage(_aBuildDate);
        }
        break;
    case KEY_ARROW_LEFT:
        mapScroll(-1, 0);
        break;
    case KEY_ARROW_RIGHT:
        mapScroll(1, 0);
        break;
    case KEY_ARROW_UP:
        mapScroll(0, -1);
        break;
    case KEY_ARROW_DOWN:
        mapScroll(0, 1);
        break;
    }

    return 0;
}

// same as proto_hash_string - later try to consolidate to single hash function
static uint32_t gvar_hash_string(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

// Combine mod name and symbol into a stable hash, with normalization
static uint32_t hashModString(const char* modName, const char* symbol) {
    char combined[256];
    char normalized[256];
    char* dst = normalized;

    // Combine with a colon separator
    snprintf(combined, sizeof(combined), "%s:%s", modName, symbol);

    // Normalize: lowercase letters, keep digits, underscore, and colon
    for (const char* src = combined; *src && dst < normalized + sizeof(normalized) - 1; src++) {
        char c = *src;
        if (c == ':') {
            *dst++ = ':';
        } else if (isalnum((unsigned char)c) || c == '_') {
            *dst++ = tolower(c);
        }
        // else skip (shouldn't occur with safe inputs)
    }
    *dst = '\0';

    return gvar_hash_string(normalized);
}

// Parse a line from a gvar definition file (vault13.gam format - handles vanilla and mod (gvar_xxx.txt) formats)
// Returns 1 if a valid GVAR definition was found, 0 otherwise.
static int parseGVarLine(char* line, char* symbol, int* defaultValue) {
    // Skip leading whitespace
    char* p = line;
    while (isspace((unsigned char)*p)) p++;
    if (*p == '\0' || *p == '#') return 0;
    // Skip C++ style comment "//"
    if (p[0] == '/' && p[1] == '/') return 0;

    // Find '='
    char* eq = strchr(p, '=');
    if (!eq) return 0;

    // Extract symbol (trim trailing spaces before '=')
    char* symEnd = eq;
    while (symEnd > p && isspace((unsigned char)symEnd[-1]))
        symEnd--;
    size_t symLen = symEnd - p;
    if (symLen == 0 || symLen >= 128) return 0;
    strncpy(symbol, p, symLen);
    symbol[symLen] = '\0';

    // Find value after '='
    char* valStart = eq + 1;
    while (isspace((unsigned char)*valStart)) valStart++;

    // Strip trailing semicolon and anything after it
    char* semicolon = strchr(valStart, ';');
    if (semicolon) *semicolon = '\0';

    *defaultValue = atoi(valStart);
    return 1;
}

// Consolidate all mod nam extractions into single helper function eventually
// Extract mod name from a filename like "gvar_MyMod.txt"
static const char* extractModNameFromGVarFile(const char* filename) {
    static char modName[64];
    modName[0] = '\0';
    if (strncmp(filename, "gvar_", 5) != 0)
        return modName;
    const char* start = filename + 5;
    const char* dot = strchr(start, '.');
    if (!dot) return modName;
    size_t len = dot - start;
    if (len >= sizeof(modName)) len = sizeof(modName) - 1;
    strncpy(modName, start, len);
    modName[len] = '\0';
    return modName;
}

// game_ui_disable
// 0x443BFC
// pass allowScrolling = 1 to allow scrolling
void gameUiDisable(int allowScrolling)
{
    if (!gGameUiDisabled) {
        gameMouseObjectsHide();
        _gmouse_disable(allowScrolling);
        keyboardDisable();
        interfaceBarDisable();
        gGameUiDisabled = true;
    }
}

// game_ui_enable
// 0x443C30
void gameUiEnable()
{
    if (gGameUiDisabled) {
        interfaceBarEnable();
        keyboardEnable();
        keyboardReset();
        _gmouse_enable();
        gameMouseObjectsShow();
        gGameUiDisabled = false;
    }
}

// game_ui_is_disabled
// 0x443C60
bool gameUiIsDisabled()
{
    return gGameUiDisabled;
}

// 0x443C68
int gameGetGlobalVar(int var)
{
    if (var < 0 || var >= gGameGlobalVarsLength) {
        debugPrint("ERROR: attempt to reference global var out of range: %d", var);
        return 0;
    }

    return gGameGlobalVars[var];
}

// 0x443C98
int gameSetGlobalVar(int var, int value)
{
    if (var < 0 || var >= gGameGlobalVarsLength) {
        debugPrint("ERROR: attempt to reference global var out of range: %d", var);
        return -1;
    }

    // SFALL: Display karma changes.
    if (var == GVAR_PLAYER_REPUTATION) {
        bool shouldDisplayKarmaChanges = false;
        configGetBool(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DISPLAY_KARMA_CHANGES_KEY, &shouldDisplayKarmaChanges);
        if (shouldDisplayKarmaChanges && !gStrictVanillaEnabled) {
            int diff = value - gGameGlobalVars[var];
            if (diff != 0) {
                char formattedMessage[80];
                if (diff > 0) {
                    snprintf(formattedMessage, sizeof(formattedMessage), "You gained %d karma.", diff);
                } else {
                    snprintf(formattedMessage, sizeof(formattedMessage), "You lost %d karma.", -diff);
                }
                displayMonitorAddMessage(formattedMessage);
            }
        }
    }

    gGameGlobalVars[var] = value;

    return 0;
}

// game_load_info
// 0x443CC8
static int gameLoadGlobalVars()
{
    if (globalVarsRead("data\\vault13.gam", "GAME_GLOBAL_VARS:", &gGameGlobalVarsLength, &gGameGlobalVars) != 0) {
        return -1;
    }

    gGameGlobalPointers = reinterpret_cast<void**>(internal_malloc(sizeof(*gGameGlobalPointers) * gGameGlobalVarsLength));
    if (gGameGlobalPointers == nullptr) {
        return -1;
    }

    memset(gGameGlobalPointers, 0, sizeof(*gGameGlobalPointers) * gGameGlobalVarsLength);

    // Store the number of vanilla GVARs for save compatibility
    gGameGlobalVarsVanillaCount = gGameGlobalVarsLength;

    return 0;
}

// Resize the global GVAR arrays to at least newLength.
// Returns 0 on success, -1 on failure.
int resizeGlobalVars(int newLength)
{
    if (newLength <= gGameGlobalVarsLength) {
        return 0; // Already big enough (we don't shrink)
    }

    // Reallocate the integer values array
    int* newVars = (int*)internal_realloc(gGameGlobalVars, sizeof(int) * newLength);
    if (newVars == nullptr) {
        return -1;
    }
    gGameGlobalVars = newVars;

    // Zero the newly added slots
    for (int i = gGameGlobalVarsLength; i < newLength; i++) {
        gGameGlobalVars[i] = 0;
    }

    // Reallocate the pointer array
    void** newPointers = (void**)internal_realloc(gGameGlobalPointers, sizeof(void*) * newLength);
    if (newPointers == nullptr) {
        // If realloc fails, we cannot proceed; revert? For simplicity, we'll just return -1.
        // In practice, failure is extremely rare. We'll leave the vars resized but pointers old.
        // To avoid inconsistency, we could free the newVars? But internal_realloc already freed the old.
        // Safer to just return -1 and rely on the caller handling it.
        return -1;
    }
    gGameGlobalPointers = newPointers;

    // Zero the new pointer slots
    for (int i = gGameGlobalVarsLength; i < newLength; i++) {
        gGameGlobalPointers[i] = nullptr;
    }

    gGameGlobalVarsLength = newLength;
    return 0;
}

static int loadModGlobalVars() {
    // Reset mod GVAR info and collision tracking
    gModGVarInfoCount = 0;
    gGVarCollisionOccurred = false;
    memset(gModGVarSlotOwners, 0, sizeof(gModGVarSlotOwners));
    memset(gGVarCollisionDetails, 0, sizeof(gGVarCollisionDetails));

    // Build search pattern: data\gvar_*.txt
    char searchPattern[COMPAT_MAX_PATH];
    snprintf(searchPattern, sizeof(searchPattern),
             "%sdata%cgvar_*.txt",
             _cd_path_base, DIR_SEPARATOR);

    // Find all matching files
    char** foundFiles = nullptr;
    int fileCount = fileNameListInit(searchPattern, &foundFiles, 0, 0);
    if (fileCount <= 0) {
        // No mod GVAR files - still generate an empty report...
        generateGVarReport();
        return 0;
    }

    // Sort files alphabetically for consistent load order
    for (int i = 0; i < fileCount - 1; i++) {
        for (int j = i + 1; j < fileCount; j++) {
            if (strcmp(foundFiles[i], foundFiles[j]) > 0) {
                char* temp = foundFiles[i];
                foundFiles[i] = foundFiles[j];
                foundFiles[j] = temp;
            }
        }
    }

    // Bitmap to track used slots in the mod range
    bool usedSlots[MOD_GVAR_COUNT] = { false };

    // Process each file
    for (int i = 0; i < fileCount; i++) {
        const char* filename = foundFiles[i];

        // Skip the base vault13.gam (handled separately)
        if (strcmp(filename, "vault13.gam") == 0)
            continue;

        // Build full path
        char filePath[COMPAT_MAX_PATH];
        snprintf(filePath, sizeof(filePath), "%sdata%c%s",
                 _cd_path_base, DIR_SEPARATOR, filename);

        File* stream = fileOpen(filePath, "rt");
        if (!stream) {
            debugPrint("Warning: Could not open mod GVAR file: %s\n", filePath);
            continue;
        }

        // Extract mod name from filename
        const char* modName = extractModNameFromGVarFile(filename);
        if (modName[0] == '\0') {
            debugPrint("Warning: Invalid mod GVAR filename (expected gvar_*.txt): %s\n", filename);
            fileClose(stream);
            continue;
        }

        debugPrint("Loading mod GVARs from %s (mod: %s)\n", filename, modName);

        char line[256];
        int lineNum = 0;
        while (fileReadString(line, sizeof(line), stream)) {
            lineNum++;
            char symbol[128];
            int defaultValue;
            if (!parseGVarLine(line, symbol, &defaultValue))
                continue;   // empty or comment

            // Compute stable hash: modName + ":" + symbol
            uint32_t hash = hashModString(modName, symbol);
            int index = MOD_GVAR_BASE + (int)(hash % MOD_GVAR_COUNT);
            int slot = index - MOD_GVAR_BASE;

            // Check for collision
            if (usedSlots[slot]) {
                gGVarCollisionOccurred = true;
                snprintf(gGVarCollisionDetails[slot], sizeof(gGVarCollisionDetails[slot]),
                        "COLLISION: mod '%s' symbol '%s' (index %d) conflicts with %s",
                        modName, symbol, index, gModGVarSlotOwners[slot]);

                // Show popup (like art system)
                char errorMsg[512];
                snprintf(errorMsg, sizeof(errorMsg),
                    "GVAR SLOT COLLISION DETECTED!\n\n"
                    "Mod: %s\n"
                    "Symbol: %s\n"
                    "Target slot: %d\n"
                    "Conflicts with: %s\n\n"
                    "To resolve: Rename your symbol or mod to change its hash.\n\n"
                    "The GVAR '%s' will NOT be loaded.",
                    modName, symbol, index, gModGVarSlotOwners[slot], symbol);
                showMesageBox(errorMsg);

                debugPrint("  Collision: skipping %s:%s -> index %d\n", modName, symbol, index);
                continue;
            }
            usedSlots[slot] = true;

            snprintf(gModGVarSlotOwners[slot], sizeof(gModGVarSlotOwners[slot]), "%s:%s", modName, symbol);

            // Ensure the global arrays are large enough to hold this index
            if (index >= gGameGlobalVarsLength) {
                if (resizeGlobalVars(index + 1) != 0) {
                    debugPrint("ERROR: Failed to expand GVAR array to index %d for mod '%s'\n",
                               index, modName);
                    usedSlots[slot] = false;   // free the slot
                    continue;
                }
            }

            // Set the default value
            gGameGlobalVars[index] = defaultValue;

            // Record mod GVAR info for the report
            if (gModGVarInfoCount < MOD_GVAR_COUNT) {
                ModGVarInfo* info = &gModGVarInfos[gModGVarInfoCount++];
                strncpy(info->modName, modName, sizeof(info->modName) - 1);
                info->modName[sizeof(info->modName) - 1] = '\0';
                strncpy(info->symbol, symbol, sizeof(info->symbol) - 1);
                info->symbol[sizeof(info->symbol) - 1] = '\0';
                info->index = index;
                info->defaultValue = defaultValue;
            }

            debugPrint("  Assigned %s:%s -> index %d (default %d)\n",
                       modName, symbol, index, defaultValue);
        }
        fileClose(stream);
    }

    fileNameListFree(&foundFiles, fileCount);

    // Generate the report
    generateGVarReport();

    return 0;
}

// 0x443CE8
int globalVarsRead(const char* path, const char* section, int* variablesListLengthPtr, int** variablesListPtr)
{
    inventoryResetDude();

    File* stream = fileOpen(path, "rt");
    if (stream == nullptr) {
        return -1;
    }

    if (*variablesListLengthPtr != 0) {
        internal_free(*variablesListPtr);
        *variablesListPtr = nullptr;
        *variablesListLengthPtr = 0;
    }

    char string[260];
    if (section != nullptr) {
        while (fileReadString(string, 258, stream)) {
            if (strncmp(string, section, 16) == 0) {
                break;
            }
        }
    }

    while (fileReadString(string, 258, stream)) {
        if (string[0] == '\n') {
            continue;
        }

        if (string[0] == '/' && string[1] == '/') {
            continue;
        }

        char* semicolon = strchr(string, ';');
        if (semicolon != nullptr) {
            *semicolon = '\0';
        }

        *variablesListLengthPtr = *variablesListLengthPtr + 1;
        *variablesListPtr = (int*)internal_realloc(*variablesListPtr, sizeof(int) * *variablesListLengthPtr);

        if (*variablesListPtr == nullptr) {
            exit(1);
        }

        char* equals = strchr(string, '=');
        if (equals != nullptr) {
            sscanf(equals + 1, "%d", *variablesListPtr + *variablesListLengthPtr - 1);
        } else {
            *variablesListPtr[*variablesListLengthPtr - 1] = 0;
        }
    }

    fileClose(stream);

    return 0;
}

static void generateGVarReport() {
    char reportPath[COMPAT_MAX_PATH];
    snprintf(reportPath, sizeof(reportPath), "%sdata%clists%cgvars_list.txt",
             _cd_path_base, DIR_SEPARATOR, DIR_SEPARATOR);

    FILE* report = compat_fopen(reportPath, "wt");
    if (!report) return;

    // Header
    const char* header =
        "==============================================================================\n"
        "Fallout 2 Fission - Global Variables (GVAR) Report\n"
        "==============================================================================\n"
        "This report shows all global variables known to the engine.\n"
        "Vanilla GVARs are loaded from data\\vault13.gam (indices 0..N-1).\n"
        "Mod GVARs are loaded from data\\gvar_*.txt and assigned stable indices\n"
        "in the range %d..%d.\n\n"
        "Use these indices in scripts, karma definitions, and other game data.\n"
        "==============================================================================\n\n";

    fprintf(report, header, MOD_GVAR_BASE, MOD_GVAR_MAX);

    // Timestamp
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    fprintf(report, "Report Generated: %04d-%02d-%02d %02d:%02d:%02d\n\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

    // Statistics
    int vanillaCount = gGameGlobalVarsVanillaCount;
    int totalIndices = gGameGlobalVarsLength;

    fprintf(report, "Vanilla GVAR count: %d (indices 0..%d)\n", vanillaCount, vanillaCount-1);
    fprintf(report, "GVAR array size: %d entries (0..%d)\n", totalIndices, totalIndices-1);
    fprintf(report, "Mod GVAR range: %d..%d\n\n", MOD_GVAR_BASE, MOD_GVAR_MAX);

    // List vanilla GVARs (only indices and current values)
    fprintf(report, "VANILLA GLOBAL VARIABLES:\n");
    fprintf(report, "Index\tValue\n");
    for (int i = 0; i < vanillaCount && i < totalIndices; i++) {
        fprintf(report, "%d\t%d\n", i, gGameGlobalVars[i]);
    }
    fprintf(report, "\n");

    // List mod GVARs with names and default values
    fprintf(report, "MOD GLOBAL VARIABLES:\n");
    if (gModGVarInfoCount > 0) {
        fprintf(report, "Index\tMod\tSymbol\tDefault\n");
        for (int i = 0; i < gModGVarInfoCount; i++) {
            ModGVarInfo* info = &gModGVarInfos[i];
            fprintf(report, "%d\t%s\t%s\t%d\n",
                    info->index, info->modName, info->symbol, info->defaultValue);
        }
    } else {
        fprintf(report, "(no mod GVARs defined)\n");
    }
    fprintf(report, "\n");

    // Collision details
    if (gGVarCollisionOccurred) {
        fprintf(report, "--- COLLISION DETAILS ---\n");
        for (int i = 0; i < MOD_GVAR_COUNT; i++) {
            if (gGVarCollisionDetails[i][0] != '\0') {
                fprintf(report, "Slot %d: %s\n", i + MOD_GVAR_BASE, gGVarCollisionDetails[i]);
            }
        }
        fprintf(report, "\n");
    }

    // Important notes
    fprintf(report,
        "=== IMPORTANT NOTES ===\n"
        "- Mod GVAR indices are STABLE and will not change between game sessions.\n"
        "- If a collision occurred, the conflicting GVAR was NOT loaded.\n"
        "- Mod GVARs are saved in a separate file (modgvars.dat) per save slot.\n"
        "- Old saves remain compatible; missing mod GVARs are initialized to defaults.\n"
        "- To reference a mod GVAR in scripts, use the index shown above.\n"
        "- To define your own mod GVARs, create a file named gvar_YourMod.txt\n"
        "  in the data directory, using the same format as vault13.gam.\n"
        "  Example line: MY_GVAR_NAME :=42;  // optional comment\n"
        );

    fclose(report);
    debugPrint("GVAR report written to %s\n", reportPath);
}

// Save all mod-range GVARs (indices MOD_GVAR_BASE .. gGameGlobalVarsLength-1) to a binary file.
// Returns 0 on success, -1 on failure.
int saveModGlobalVars(File* stream) {
    // Count how many mod GVARs exist (all indices from MOD_GVAR_BASE up to current max)
    int count = 0;
    for (int i = MOD_GVAR_BASE; i < gGameGlobalVarsLength; i++) {
        count++;
    }
    // Write count
    if (fileWrite(&count, sizeof(count), 1, stream) != 1)
        return -1;

    // Write each index and value
    for (int i = MOD_GVAR_BASE; i < gGameGlobalVarsLength; i++) {
        int idx = i;
        int val = gGameGlobalVars[i];
        if (fileWrite(&idx, sizeof(idx), 1, stream) != 1)
            return -1;
        if (fileWrite(&val, sizeof(val), 1, stream) != 1)
            return -1;
    }
    return 0;
}

// Load mod-range GVARs from a binary file and apply them.
// Assumes the GVAR array has already been expanded to at least MOD_GVAR_MAX.
// Returns 0 on success, -1 on failure.
int loadModGlobalVarsFromSave(File* stream) {
    int count;
    if (fileRead(&count, sizeof(count), 1, stream) != 1)
        return -1;

    for (int i = 0; i < count; i++) {
        int idx, val;
        if (fileRead(&idx, sizeof(idx), 1, stream) != 1)
            return -1;
        if (fileRead(&val, sizeof(val), 1, stream) != 1)
            return -1;

        // Only restore if the index is within the current mod range
        if (idx >= MOD_GVAR_BASE && idx < gGameGlobalVarsLength) {
            gGameGlobalVars[idx] = val;
        } else {
            // Index out of range � perhaps a mod was removed; ignore.
            debugPrint("Warning: mod GVAR index %d out of range - skipping.\n",
                       idx, gGameGlobalVarsLength - 1);
        }
    }
    return 0;
}

// 0x443E2C
int gameGetState()
{
    return gGameState;
}

// 0x443E34
int gameRequestState(int newGameState)
{
    switch (newGameState) {
    case GAME_STATE_0:
        newGameState = GAME_STATE_1;
        break;
    case GAME_STATE_2:
        newGameState = GAME_STATE_3;
        break;
    case GAME_STATE_4:
        newGameState = GAME_STATE_5;
        break;
    }

    if (gGameState == GAME_STATE_4 && newGameState == GAME_STATE_5) {
        return -1;
    }

    gGameState = newGameState;
    return 0;
}

// 0x443E90
void gameUpdateState()
{
    switch (gGameState) {
    case GAME_STATE_1:
        gGameState = GAME_STATE_0;
        break;
    case GAME_STATE_3:
        gGameState = GAME_STATE_2;
        break;
    case GAME_STATE_5:
        gGameState = GAME_STATE_4;
        break;
    }
}

// 0x443EF0
static int gameTakeScreenshot(int width, int height, unsigned char* buffer, unsigned char* palette)
{
    MessageListItem messageListItem;

    if (screenshotHandlerDefaultImpl(width, height, buffer, palette) != 0) {
        // Error saving screenshot.
        messageListItem.num = 8;
        if (messageListGetItem(&gMiscMessageList, &messageListItem)) {
            displayMonitorAddMessage(messageListItem.text);
        }

        return -1;
    }

    // Saved screenshot.
    messageListItem.num = 3;
    if (messageListGetItem(&gMiscMessageList, &messageListItem)) {
        displayMonitorAddMessage(messageListItem.text);
    }

    return 0;
}

// NOTE: Inlined.
//
// 0x443F50
static void gameFreeGlobalVars()
{
    gGameGlobalVarsLength = 0;
    if (gGameGlobalVars != nullptr) {
        internal_free(gGameGlobalVars);
        gGameGlobalVars = nullptr;
    }

    if (gGameGlobalPointers != nullptr) {
        internal_free(gGameGlobalPointers);
        gGameGlobalPointers = nullptr;
    }
}

// 0x443F74
static void showHelp()
{
    ScopedGameMode gm(GameMode::kHelp);

    restoreUserAspectPreference();
    resizeContent(640, 480);

    bool isoWasEnabled = isoDisable();
    gameMouseObjectsHide();

    gameMouseSetCursor(MOUSE_CURSOR_NONE);

    bool colorCycleWasEnabled = colorCycleEnabled();
    colorCycleDisable();

    // CE: Help screen uses separate color palette which is incompatible with
    // colors in other windows. Setup overlay to hide everything.
    int overlay = windowCreate(0, 0, screenGetWidth(), screenGetHeight(), 0, WINDOW_HIDDEN | WINDOW_MOVE_ON_TOP);

    int helpWindowX = (screenGetWidth() - HELP_SCREEN_WIDTH) / 2;
    int helpWindowY = (screenGetHeight() - HELP_SCREEN_HEIGHT) / 2;
    int win = windowCreate(helpWindowX, helpWindowY, HELP_SCREEN_WIDTH, HELP_SCREEN_HEIGHT, 0, WINDOW_HIDDEN | WINDOW_MOVE_ON_TOP);
    if (win != -1) {
        unsigned char* windowBuffer = windowGetBuffer(win);
        if (windowBuffer != nullptr) {
            FrmImage backgroundFrmImage;
            int backgroundFid = buildFid(OBJ_TYPE_INTERFACE, 297, 0, 0, 0);
            if (backgroundFrmImage.lock(backgroundFid)) {
                paletteSetEntries(gPaletteBlack);
                blitBufferToBuffer(backgroundFrmImage.getData(), HELP_SCREEN_WIDTH, HELP_SCREEN_HEIGHT, HELP_SCREEN_WIDTH, windowBuffer, HELP_SCREEN_WIDTH);

                colorPaletteLoad("art\\intrface\\helpscrn.pal");
                paletteSetEntries(_cmap);

                // CE: Fill overlay with darkest color in the palette. It might
                // not be completely black, but at least it's uniform.
                bufferFill(windowGetBuffer(overlay),
                    screenGetWidth(),
                    screenGetHeight(),
                    screenGetWidth(),
                    intensityColorTable[_colorTable[0]][0]);

                windowShow(overlay);
                windowShow(win);

                while (inputGetInput() == -1 && _game_user_wants_to_quit == 0) {
                    sharedFpsLimiter.mark();
                    renderPresent();
                    sharedFpsLimiter.throttle();
                }

                while (mouseGetEvent() != 0) {
                    sharedFpsLimiter.mark();

                    inputGetInput();

                    renderPresent();
                    sharedFpsLimiter.throttle();
                }

                paletteSetEntries(gPaletteBlack);
            }
        }

        windowDestroy(overlay);
        windowDestroy(win);
        colorPaletteLoad("color.pal");
        paletteSetEntries(_cmap);
    }

    if (colorCycleWasEnabled) {
        colorCycleEnable();
    }

    gameMouseObjectsShow();

    if (isoWasEnabled) {
        isoEnable();
    }
    resizeContent(screenGetWidth(), screenGetHeight(), true);
}

// 0x4440B8
int showQuitConfirmationDialog()
{
    bool isoWasEnabled = isoDisable();

    bool gameMouseWasVisible;
    if (isoWasEnabled) {
        gameMouseWasVisible = gameMouseObjectsIsVisible();
    } else {
        gameMouseWasVisible = false;
    }

    if (gameMouseWasVisible) {
        gameMouseObjectsHide();
    }

    bool cursorWasHidden = cursorIsHidden();
    if (cursorWasHidden) {
        mouseShowCursor();
    }

    int oldCursor = gameMouseGetCursor();
    gameMouseSetCursor(MOUSE_CURSOR_ARROW);

    int rc;

    // Are you sure you want to quit?
    MessageListItem messageListItem;
    messageListItem.num = 0;
    if (messageListGetItem(&gMiscMessageList, &messageListItem)) {
        rc = showDialogBox(messageListItem.text, nullptr, 0, 169, 117, _colorTable[32328], nullptr, _colorTable[32328], DIALOG_BOX_YES_NO);
        if (rc != 0) {
            _game_user_wants_to_quit = 2;
        }
    } else {
        rc = -1;
    }

    gameMouseSetCursor(oldCursor);

    if (cursorWasHidden) {
        mouseHideCursor();
    }

    if (gameMouseWasVisible) {
        gameMouseObjectsShow();
    }

    if (isoWasEnabled) {
        isoEnable();
    }

    return rc;
}

// create a folder to hold 'lists' reports
void createListsFolder()
{
    // create the "lists" folder inside the "data" directory
    const char* dataDir = "data";
    const char* listsFolderName = "lists";

    char listsFolderPath[COMPAT_MAX_PATH];
    compat_makepath(listsFolderPath, nullptr, dataDir, listsFolderName, nullptr);

    // create the lists folder
    compat_mkdir(listsFolderPath);
}

// 0x44418C
static int gameDbInit()
{
    const char* main_file_name;
    const char* patch_file_name;
    char filename[COMPAT_MAX_PATH];
    int patch_index;
    bool is_original = false;

    // Check if master.dat is the original version (multiple versions?)
    const char* master_path = settings.system.master_dat_path.c_str();
    if (*master_path != '\0') {
        FILE* f = fopen(master_path, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            is_original = (ftell(f) == 333177805);
            fclose(f);
        }
    }

    // Helper lambda to actually open the fission datafile
    auto loadFission = [&]() -> int {
        const char* main_file_name = settings.system.fission_dat_path.c_str();
        const char* patch_file_name = settings.system.fission_patches_path.c_str();
        if (*patch_file_name == '\0') {
            patch_file_name = nullptr;
        }
        int handle = dbOpen(main_file_name, 0, patch_file_name, 1);
        if (handle == -1) {
            showMesageBox(
                "Could not find the fission datafile. "
                "Please make sure the fission.dat file is in the folder "
                "that you are running FALLOUT from.");
        }
        return handle;
    };

    bool hasFission = !settings.system.fission_dat_path.empty();
    bool useMasterOverride = settings.system.master_override;

    // If master.dat is *not* the “original” AND override is *not* set,
    // then load fission.dat *before* master.dat.
    if (!is_original && !useMasterOverride && hasFission) {
        if (loadFission() == -1)
            return -1;
    }

    // Now load master.dat
    {
        const char* main_file_name = settings.system.master_dat_path.c_str();
        const char* patch_file_name = settings.system.master_patches_path.c_str();
        if (*main_file_name == '\0') {
            main_file_name = nullptr;
        }
        if (*patch_file_name == '\0') {
            patch_file_name = nullptr;
        }

        int master_db_handle = dbOpen(main_file_name, 0, patch_file_name, 1);
        if (master_db_handle == -1) {
            showMesageBox(
                "Could not find the master datafile. "
                "Please make sure the master.dat file is in the folder "
                "that you are running FALLOUT from.");
            return -1;
        }
    }

    // If master.dat *is* the original, OR if override is set,
    // then load fission.dat *after* master.dat.
    if ((is_original || useMasterOverride) && hasFission) {

        if (loadFission() == -1)
            return -1;
    }

    // Load critter.dat
    main_file_name = settings.system.critter_dat_path.c_str();
    if (*main_file_name == '\0') {
        main_file_name = nullptr;
    }

    patch_file_name = settings.system.critter_patches_path.c_str();
    if (*patch_file_name == '\0') {
        patch_file_name = nullptr;
    }

    int critter_db_handle = dbOpen(main_file_name, 0, patch_file_name, 1);
    if (critter_db_handle == -1) {
        showMesageBox(
            "Could not find the critter datafile. "
            "Please make sure the critter.dat file is in the folder "
            "that you are running FALLOUT from.");
        return -1;
    }

    // SFALL: custom patch file name.
    char* path_file_name_template = nullptr;
    configGetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PATCH_FILE, &path_file_name_template);
    if (path_file_name_template == nullptr || *path_file_name_template == '\0') {
        path_file_name_template = (char*)"patch%03d.dat";
    }

    for (patch_index = 0; patch_index < 1000; patch_index++) {
        snprintf(filename, sizeof(filename), path_file_name_template, patch_index);

        if (compat_access(filename, 0) == 0) {
            dbOpen(filename, 0, nullptr, 1);
        }
    }

    createListsFolder();

    sfallLoadMods();

    return 0;
}

// 0x444384
static void showSplash()
{
    int splash = settings.system.splash;

    char path[64];
    const char* language = settings.system.language.c_str();
    if (compat_stricmp(language, ENGLISH) != 0) {
        snprintf(path, sizeof(path), "art\\%s\\splash\\", language);
    } else {
        snprintf(path, sizeof(path), "art\\splash\\");
    }

    File* stream = nullptr;
    for (int index = 0; index < SPLASH_COUNT; index++) {
        char filePath[64];

        // First try widescreen version if in widescreen mode
        if (gameIsWidescreen()) {
            snprintf(filePath, sizeof(filePath), "%ssplash%d%s.rix", path, splash,
                settings.graphics.widescreen_variant_suffix.c_str());
            stream = fileOpen(filePath, "rb");
            if (stream != nullptr) {
                break;
            }
        }

        // If widescreen version not found or not in widescreen mode, try regular version
        snprintf(filePath, sizeof(filePath), "%ssplash%d.rix", path, splash);
        stream = fileOpen(filePath, "rb");
        if (stream != nullptr) {
            break;
        }

        splash++;
        if (splash >= SPLASH_COUNT) {
            splash = 0;
        }
    }

    if (stream == nullptr) {
        return;
    }

    unsigned char* palette = reinterpret_cast<unsigned char*>(internal_malloc(768));
    if (palette == nullptr) {
        fileClose(stream);
        return;
    }

    int version;
    fileReadInt32(stream, &version);
    if (version != 'RIX3') {
        fileClose(stream);
        return;
    }

    short width;
    fileRead(&width, sizeof(width), 1, stream);

    short height;
    fileRead(&height, sizeof(height), 1, stream);

    unsigned char* data = reinterpret_cast<unsigned char*>(internal_malloc(width * height));
    if (data == nullptr) {
        internal_free(palette);
        fileClose(stream);
        return;
    }

    paletteSetEntries(gPaletteBlack);
    fileSeek(stream, 10, SEEK_SET);
    fileRead(palette, 1, 768, stream);
    fileRead(data, 1, width * height, stream);
    fileClose(stream);

    // Fix of wrong Palette, without it this makes background bright
    // Basically just swapping first and last colors, this problem presented ONLY in F2, F1 has right palette in every splash
    memcpy(palette + (255 * 3), palette, 3);
    memset(palette, 0, 3);

    for (int i = 0; i < width * height; i++) {
        if (data[i] == 0) {
            data[i] = 255;
        } else if (data[i] == 255) {
            data[i] = 0;
        }
    }

    int screenWidth = screenGetWidth();
    int screenHeight = screenGetHeight();

    // Calculate centered position
    int x = (screenWidth - width) / 2;
    int y = (screenHeight - height) / 2;

    // Perform clean blit
    _scr_blit(data, width, height, 0, 0, width, height, x, y);
    paletteFadeTo(palette);
    inputPauseForTocks(1000); // Added for gravitas

    internal_free(data);
    internal_free(palette);

    settings.system.splash = splash + 1;
}

int gameShowDeathDialog(const char* message)
{
    bool isoWasEnabled = isoDisable();

    bool gameMouseWasVisible;
    if (isoWasEnabled) {
        gameMouseWasVisible = gameMouseObjectsIsVisible();
    } else {
        gameMouseWasVisible = false;
    }

    if (gameMouseWasVisible) {
        gameMouseObjectsHide();
    }

    bool cursorWasHidden = cursorIsHidden();
    if (cursorWasHidden) {
        mouseShowCursor();
    }

    int oldCursor = gameMouseGetCursor();
    gameMouseSetCursor(MOUSE_CURSOR_ARROW);

    int oldUserWantsToQuit = _game_user_wants_to_quit;
    _game_user_wants_to_quit = 0;

    int rc = showDialogBox(message, nullptr, 0, 169, 117, _colorTable[32328], nullptr, _colorTable[32328], DIALOG_BOX_LARGE);

    _game_user_wants_to_quit = oldUserWantsToQuit;

    gameMouseSetCursor(oldCursor);

    if (cursorWasHidden) {
        mouseHideCursor();
    }

    if (gameMouseWasVisible) {
        gameMouseObjectsShow();
    }

    if (isoWasEnabled) {
        isoEnable();
    }

    return rc;
}

void* gameGetGlobalPointer(int var)
{
    if (var < 0 || var >= gGameGlobalVarsLength) {
        debugPrint("ERROR: attempt to reference global pointer out of range: %d", var);
        return nullptr;
    }

    return gGameGlobalPointers[var];
}

int gameSetGlobalPointer(int var, void* value)
{
    if (var < 0 || var >= gGameGlobalVarsLength) {
        debugPrint("ERROR: attempt to reference global var out of range: %d", var);
        return -1;
    }

    gGameGlobalPointers[var] = value;

    return 0;
}

int GameMode::currentGameMode = 0;

void GameMode::enterGameMode(int gameMode)
{
    currentGameMode |= gameMode;
}

void GameMode::exitGameMode(int gameMode)
{
    currentGameMode &= ~gameMode;
}

bool GameMode::isInGameMode(int gameMode)
{
    return (currentGameMode & gameMode) != 0;
}

ScopedGameMode::ScopedGameMode(int gameMode)
{
    this->gameMode = gameMode;
    GameMode::enterGameMode(gameMode);
}

ScopedGameMode::~ScopedGameMode()
{
    GameMode::exitGameMode(gameMode);
}

} // namespace fallout
