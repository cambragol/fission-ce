#include "pipboy.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "art.h"
#include "automap.h"
#include "color.h"
#include "combat.h"
#include "config.h"
#include "critter.h"
#include "cycle.h"
#include "dbox.h"
#include "debug.h"
#include "delay.h"
#include "draw.h"
#include "game.h"
#include "game_mouse.h"
#include "game_movie.h"
#include "game_sound.h"
#include "geometry.h"
#include "input.h"
#include "interface.h"
#include "kb.h"
#include "map.h"
#include "memory.h"
#include "message.h"
#include "mouse.h"
#include "object.h"
#include "party_member.h"
#include "platform_compat.h"
#include "queue.h"
#include "random.h"
#include "scripts.h"
#include "settings.h"
#include "sfall_config.h"
#include "stat.h"
#include "svga.h"
#include "text_font.h"
#include "window_manager.h"
#include "word_wrap.h"
#include "worldmap.h"

namespace fallout {

#define DIR_SEPARATOR '/'

#define PIPBOY_WINDOW_WIDTH (640)
#define PIPBOY_WINDOW_HEIGHT (480)

#define PIPBOY_WINDOW_DAY_X (20)
#define PIPBOY_WINDOW_DAY_Y (17)

#define PIPBOY_WINDOW_MONTH_X (46)
#define PIPBOY_WINDOW_MONTH_Y (18)

#define PIPBOY_WINDOW_YEAR_X (83)
#define PIPBOY_WINDOW_YEAR_Y (17)

#define PIPBOY_WINDOW_TIME_X (155)
#define PIPBOY_WINDOW_TIME_Y (17)

#define PIPBOY_HOLODISK_LINES_MAX (35)

#define PIPBOY_WINDOW_CONTENT_VIEW_X (254)
#define PIPBOY_WINDOW_CONTENT_VIEW_Y (46)
#define PIPBOY_WINDOW_CONTENT_VIEW_WIDTH (374)
#define PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT (410)

#define PIPBOY_IDLE_TIMEOUT (120000)
#define PIPBOY_RAND_MAX (32767)

#define PIPBOY_BOMB_COUNT (16)

#define PIPBOY_KEY_UP 1030
#define PIPBOY_KEY_DOWN 1031
#define PIPBOY_KEY_RIGHT 1033
#define PIPBOY_KEY_LEFT 1034
#define PIPBOY_KEY_SELECT 1032

// constants for setting lines per page in pagination functions
const int PIPBOY_STATUS_QUEST_LINES = 19;
const int PIPBOY_STATUS_HOLODISK_LINES = 19;
const int PIPBOY_AUTOMAP_LINES = 19;
const int PIPBOY_AUTOMAP_SUB_LINES = 5;
const int PIPBOY_STATUS_QUESTLIST_LINES = 12;

// global for selected item in lists
int gPipboySelectedItem = 0;
int gPipboySelectedIndex = 0; // Currently selected item index (0-based)
int gPipboyMaxSelectableItems = 0; // Maximum selectable items on current page
bool gPipboyKeyboardMode = false; // Are we in keyboard navigation mode?

typedef enum PipboyColumn {
    PIPBOY_COLUMN_NONE = 0,
    PIPBOY_COLUMN_QUESTS,
    PIPBOY_COLUMN_HOLODISKS,
} PipboyColumn;

static PipboyColumn gPipboyCurrentColumn = PIPBOY_COLUMN_QUESTS;

typedef enum Holiday {
    HOLIDAY_NEW_YEAR,
    HOLIDAY_VALENTINES_DAY,
    HOLIDAY_FOOLS_DAY,
    HOLIDAY_SHIPPING_DAY,
    HOLIDAY_INDEPENDENCE_DAY,
    HOLIDAY_HALLOWEEN,
    HOLIDAY_THANKSGIVING_DAY,
    HOLIDAY_CRISTMAS,
    HOLIDAY_COUNT,
} Holiday;

// Options used to render Pipboy texts.
typedef enum PipboyTextOptions {
    // Specifies that text should be rendered in the center of the Pipboy
    // monitor.
    //
    // This option is mutually exclusive with other alignment options.
    PIPBOY_TEXT_ALIGNMENT_CENTER = 0x02,

    // Specifies that text should be rendered in the beginning of the right
    // column in two-column layout.
    //
    // This option is mutually exclusive with other alignment options.
    PIPBOY_TEXT_ALIGNMENT_RIGHT_COLUMN = 0x04,

    // Specifies that text should be rendered in the center of the left column.
    //
    // This option is mutually exclusive with other alignment options.
    PIPBOY_TEXT_ALIGNMENT_LEFT_COLUMN_CENTER = 0x10,

    // Specifies that text should be rendered in the center of the right
    // column.
    //
    // This option is mutually exclusive with other alignment options.
    PIPBOY_TEXT_ALIGNMENT_RIGHT_COLUMN_CENTER = 0x20,

    // Specifies that text should rendered with underline.
    PIPBOY_TEXT_STYLE_UNDERLINE = 0x08,

    // Specifies that text should rendered with strike-through line.
    PIPBOY_TEXT_STYLE_STRIKE_THROUGH = 0x40,

    // Specifies that text should be rendered with no (minimal) indentation.
    PIPBOY_TEXT_NO_INDENT = 0x80,
} PipboyTextOptions;

typedef enum PipboyRestDuration {
    PIPBOY_REST_DURATION_TEN_MINUTES,
    PIPBOY_REST_DURATION_THIRTY_MINUTES,
    PIPBOY_REST_DURATION_ONE_HOUR,
    PIPBOY_REST_DURATION_TWO_HOURS,
    PIPBOY_REST_DURATION_THREE_HOURS,
    PIPBOY_REST_DURATION_FOUR_HOURS,
    PIPBOY_REST_DURATION_FIVE_HOURS,
    PIPBOY_REST_DURATION_SIX_HOURS,
    PIPBOY_REST_DURATION_UNTIL_MORNING,
    PIPBOY_REST_DURATION_UNTIL_NOON,
    PIPBOY_REST_DURATION_UNTIL_EVENING,
    PIPBOY_REST_DURATION_UNTIL_MIDNIGHT,
    PIPBOY_REST_DURATION_UNTIL_HEALED,
    PIPBOY_REST_DURATION_UNTIL_PARTY_HEALED,
    PIPBOY_REST_DURATION_COUNT,
    PIPBOY_REST_DURATION_COUNT_WITHOUT_PARTY = PIPBOY_REST_DURATION_COUNT - 1,
} PipboyRestDuration;

typedef enum PipboyFrm {
    PIPBOY_FRM_LITTLE_RED_BUTTON_UP,
    PIPBOY_FRM_LITTLE_RED_BUTTON_DOWN,
    PIPBOY_FRM_NUMBERS,
    PIPBOY_FRM_BACKGROUND,
    PIPBOY_FRM_NOTE,
    PIPBOY_FRM_MONTHS,
    PIPBOY_FRM_NOTE_NUMBERS,
    PIPBOY_FRM_ALARM_DOWN,
    PIPBOY_FRM_ALARM_UP,
    PIPBOY_FRM_LOGO,
    PIPBOY_FRM_BOMB,
    PIPBOY_FRM_COUNT,
} PipboyFrm;

// Provides metadata information on quests.
//
// Loaded from `data/quests.txt`.
typedef struct QuestDescription {
    int location;
    int description;
    int gvar;
    int displayThreshold;
    int completedThreshold;
} QuestDescription;

// Provides metadata information on holodisks.
//
// Loaded from `data/holodisk.txt`.
typedef struct HolodiskDescription {
    int gvar;
    int name;
    int description;
} HolodiskDescription;

typedef struct HolidayDescription {
    short month;
    short day;
    short textId;
} HolidayDescription;

typedef struct STRUCT_664350 {
    char* name;
    short field_4;
    short field_6;
} STRUCT_664350;

typedef struct PipboyBomb {
    int x;
    int y;
    float field_8;
    float field_C;
    unsigned char field_10;
} PipboyBomb;

typedef void PipboyRenderProc(int a1);

static int pipboyWindowInit(int intent);
static void pipboyWindowFree();
static void _pip_init_();
static void pipboyDrawNumber(int value, int digits, int x, int y);
static void pipboyDrawDate();
static void pipboyDrawText(const char* text, int a2, int a3);
static int _save_pipboy(File* stream);
static void pipboyWindowHandleStatus(int userInput);
static void pipboyWindowRenderQuestLocationList(int a1);
static void pipboyWindowQuestList(int a1);
static void pipboyRenderHolodiskText();
static int pipboyWindowRenderHolodiskList(int a1);
static int _qscmp(const void* a1, const void* a2);
static void pipboyWindowHandleAutomaps(int a1);
static int _PrintAMelevList(int a1);
static int _PrintAMList(int a1);
static void pipboyHandleVideoArchive(int a1);
static int pipboyRenderVideoArchive(int a1);
static void pipboyHandleAlarmClock(int eventCode);
static void pipboyWindowRenderRestOptions(int a1);
static void pipboyDrawHitPoints();
static void pipboyWindowCreateButtons(int a1, int a2, bool a3);
static void pipboyWindowDestroyButtons();
static bool pipboyRest(int hours, int minutes, int kind);
static bool _Check4Health(int minutes);
static bool _AddHealth();
static void _ClacTime(int* hours, int* minutes, int wakeUpHour);
static int pipboyRenderScreensaver();
static int questInit();
static void questFree();
static int questDescriptionCompare(const void* a1, const void* a2);
static int holodiskInit();
static void holodiskFree();

// 0x496FC0
const Rect gPipboyWindowContentRect = {
    PIPBOY_WINDOW_CONTENT_VIEW_X,
    PIPBOY_WINDOW_CONTENT_VIEW_Y,
    PIPBOY_WINDOW_CONTENT_VIEW_X + PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
    PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
};

// 0x496FD0
const int gPipboyFrmIds[PIPBOY_FRM_COUNT] = {
    8,
    9,
    82,
    127,
    128,
    129,
    130,
    131,
    132,
    133,
    226,
};

// 0x51C128
QuestDescription* gQuestDescriptions = nullptr;

// 0x51C12C
int gQuestsCount = 0;

// 0x51C130
HolodiskDescription* gHolodiskDescriptions = nullptr;

// 0x51C134
int gHolodisksCount = 0;

int gPipboySelectedQuestIndex = 0;
int gPipboySelectedHolodiskIndex = 0;

// Number of rest options available.
//
// 0x51C138
int gPipboyRestOptionsCount = PIPBOY_REST_DURATION_COUNT;

// 0x51C13C
bool gPipboyWindowIsoWasEnabled = false;

// 0x51C140
const HolidayDescription gHolidayDescriptions[HOLIDAY_COUNT] = {
    { 1, 1, 100 },
    { 2, 14, 101 },
    { 4, 1, 102 },
    { 7, 4, 104 },
    { 10, 6, 103 },
    { 10, 31, 105 },
    { 11, 28, 106 },
    { 12, 25, 107 },
};

// 0x51C170
PipboyRenderProc* _PipFnctn[5] = {
    pipboyWindowHandleStatus,
    pipboyWindowHandleAutomaps,
    pipboyHandleVideoArchive,
    pipboyHandleAlarmClock,
    pipboyHandleAlarmClock,
};

// 0x664338
MessageListItem gPipboyMessageListItem;

// pipboy.msg
//
// 0x664348
MessageList gPipboyMessageList = { 0, nullptr };

// 0x664350
STRUCT_664350 _sortlist[AUTOMAP_MAP_COUNT];

// quests.msg
//
// 0x664410
MessageList gQuestsMessageList;

// 0x664418
int gPipboyQuestLocationsCount;

// 0x66441C
unsigned char* gPipboyWindowBuffer;

// 0x66444C
int gPipboyWindowHolodisksCount;

// 0x664450
int gPipboyMouseY;

// 0x664454
int gPipboyMouseX;

// 0x664458
unsigned int gPipboyLastEventTimestamp;

// Index of the last page when rendering holodisk content.
//
// 0x66445C
int gPipboyHolodiskLastPage;

// 0x664460
int _HotLines[22];

// 0x6644B8
int _button;

// 0x6644BC
int gPipboyPreviousMouseX;

// 0x6644C0
int gPipboyPreviousMouseY;

// 0x6644C4
int gPipboyWindow;

// 0x6644F4
int _holodisk;

// 0x6644F8
int gPipboyWindowButtonCount;

// 0x6644FC
int gPipboyWindowOldFont;

// 0x664500
bool _proc_bail_flag;

// 0x664504
int main_sub_mode;

// 0x664508
int gPipboyTab;

// automap location count
int _location_count;

// automap location map count
int _map_count;

// adjusted index for pagination
int realIndex;

// 0x664510
int gPipboyWindowButtonStart;

// 0x664514
int gPipboyCurrentLine;

// 0x664518
int _rest_time;

// 0x66451C
int _amcty_indx;

// current page for holodisk entry pagination
int _view_page;

// current page for main automap pagination
int _view_page_automap_main;

// current page for automap maps pagination
int _view_page_automap_sub;

// current page for main status pagination
int _view_page_quest;

// current page for questlist pagination
int _view_page_questlist;

// current page for main holodisk pagination
int _view_page_holodisk;

// 0x664524
int gPipboyLinesCount;

// 0x664528
unsigned char _hot_back_line;

// 0x664529
unsigned char _holo_flag;

// 0x66452A
unsigned char _stat_flag;

void handlePipboyPageNavigation(
    int mouseX,
    int threshold,
    int* viewPage,
    int totalPages,
    void (*handleAutomaps)(int),
    void (*updatePage)(void));

static int gPipboyPrevTab;

static int totalPages; // for tracking between pipboyWindowHandleAutomaps and _PrintAMelevList/_PrintAMList and others for pagination

static int gPipboyWindowQuestsCurrentPageCount; // kludge for tracking number of buttons for 'status' page entries

static bool pipboy_available_at_game_start = false;

static FrmImage _pipboyFrmImages[PIPBOY_FRM_COUNT];

// Quest mod system globals
ModQuestInfo gModQuests[MOD_QUEST_MAX - MOD_QUEST_START];
int gModQuestCount = 0;
char gQuestModNames[TOTAL_QUEST_MAX][64];

static void pipboyRedrawStatusPageWithSelection()
{
    // Clear the content area
    blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
        PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
        PIPBOY_WINDOW_WIDTH,
        gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_WIDTH);

    if (gPipboyLinesCount >= 0) {
        gPipboyCurrentLine = 0;
    }

    // Render quest locations
    pipboyWindowRenderQuestLocationList(-1);

    if (gPipboyQuestLocationsCount == 0) {
        const char* text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 203);
        pipboyDrawText(text, 0, _colorTable[992]);
    }

    // Render holodisk list
    gPipboyWindowHolodisksCount = pipboyWindowRenderHolodiskList(-1);

    windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
}

static int pipboyGetSelectionColor(bool isKeyboardSelected, bool isMouseSelected)
{
    if (isKeyboardSelected && gPipboyKeyboardMode) {
        return _colorTable[32747]; // Bright green for keyboard selection
    } else if (isMouseSelected) {
        return _colorTable[32747]; // Bright green for mouse hover
    } else {
        return _colorTable[992]; // Normal color
    }
}

// Debug function to activate test quests
static void debugActivateTestQuests()
{
    // Set GVARs for our test quests
    // This will make them appear in the Pip-Boy

    char debugMsg[1024];
    int offset = 0;

    offset += snprintf(debugMsg, sizeof(debugMsg),
        "Activating test quests for debugging:\n\n");

    // Scraptown Main Quest (GVAR 900) - set to 1 (active)
    int oldVal900 = gGameGlobalVars[900];
    gGameGlobalVars[900] = 1;
    offset += snprintf(debugMsg + offset, sizeof(debugMsg) - offset,
        "GVAR 900: %d -> %d (Active)\n", oldVal900, gGameGlobalVars[900]);

    // Scraptown Side Quest (GVAR 901) - set to 2 (completed)
    int oldVal901 = gGameGlobalVars[901];
    gGameGlobalVars[901] = 2;
    offset += snprintf(debugMsg + offset, sizeof(debugMsg) - offset,
        "GVAR 901: %d -> %d (Completed)\n", oldVal901, gGameGlobalVars[901]);

    // Hightown Side Quest (GVAR 902) - set to 1 (active)
    int oldVal902 = gGameGlobalVars[902];
    gGameGlobalVars[902] = 1;
    offset += snprintf(debugMsg + offset, sizeof(debugMsg) - offset,
        "GVAR 902: %d -> %d (Active)\n\n", oldVal902, gGameGlobalVars[902]);

    offset += snprintf(debugMsg + offset, sizeof(debugMsg) - offset,
        "Test quests activated!\n");
    offset += snprintf(debugMsg + offset, sizeof(debugMsg) - offset,
        "- Scraptown: 1 active, 1 completed\n");
    offset += snprintf(debugMsg + offset, sizeof(debugMsg) - offset,
        "- Hightown: 1 active\n");

    showMesageBox(debugMsg);
}

// Helper function to find city index in _sortlist
static int findCityIndexInSortlist(int cityIndex, int elevation)
{
    for (int i = 0; i < _map_count; i++) {
        if (_sortlist[i].field_4 == elevation && _sortlist[i].field_6 == cityIndex) {
            return i;
        }
    }
    return 0; // Default to first item
}

// Hash function for mod quest slot allocation (DJB2 hash)
static uint32_t questHashString(const char* str)
{
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        c = tolower(c);
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

// Calculate consistent slot for a mod quest
static uint16_t questCalculateModSlot(const char* questKey, uint32_t modNamespace, int questIndexInMod)
{
    // Combine mod namespace with quest-specific information for consistent hashing
    char combinedKey[256];
    snprintf(combinedKey, sizeof(combinedKey), "%s|%u|%d", questKey, modNamespace, questIndexInMod);

    uint32_t hash = questHashString(combinedKey);

    // Map to mod quest range (200-999)
    uint16_t slot = MOD_QUEST_START + (hash % (MOD_QUEST_MAX - MOD_QUEST_START));

    return slot;
}

// Get mod namespace from filename
static uint32_t questGetModNamespace(const char* filename)
{
    char baseName[COMPAT_MAX_PATH];
    const char* lastSlash = strrchr(filename, DIR_SEPARATOR);
    const char* nameStart = lastSlash ? lastSlash + 1 : filename;

    // Remove extension if present
    strncpy(baseName, nameStart, sizeof(baseName) - 1);
    baseName[sizeof(baseName) - 1] = '\0';

    char* dot = strrchr(baseName, '.');
    if (dot) *dot = '\0';

    return questHashString(baseName);
}

// Parse a single quest line from quests.txt format
static bool parseQuestLine(const char* line, int* location, int* description, int* gvar,
    int* displayThreshold, int* completedThreshold)
{
    // Format: location, description, gvar, displayThreshold, completedThreshold
    // Example: 1500, 100, 79, 1, 2
    return sscanf(line, "%d, %d, %d, %d, %d",
               location, description, gvar, displayThreshold, completedThreshold)
        == 5;
}

// Initialize quest mod system
static void questModInit()
{
    // Clear mod quest tracking
    memset(gModQuests, 0, sizeof(gModQuests));
    gModQuestCount = 0;

    // Clear mod names array
    memset(gQuestModNames, 0, sizeof(gQuestModNames));
}

// Helper function to get mod quest info by quest ID
static ModQuestInfo* getModQuestInfo(int questId)
{
    if (questId < MOD_QUEST_START || questId >= MOD_QUEST_MAX) {
        return nullptr;
    }

    for (int i = 0; i < gModQuestCount; i++) {
        if (gModQuests[i].questId == questId) {
            return &gModQuests[i];
        }
    }

    return nullptr;
}

// Helper function to get quest description message ID
static int getQuestDescriptionMessageId(int questId)
{
    if (questId < MOD_QUEST_START) {
        // Vanilla quest: use the description field directly
        // (which contains a message ID relative to quests.msg)
        return gQuestDescriptions[questId].description;
    } else {
        // Mod quest: the description field already contains the hashed message ID
        return gQuestDescriptions[questId].description;
    }
}

// master navigation function to handle main and sub navigation
static void renderNavigationButtons(int _view_page, int totalPages, bool isSubPage)
{

    if (gPipboyLinesCount >= 0) {
        gPipboyCurrentLine = gPipboyLinesCount; // sets navigation to bottom of page
    }

    if (totalPages == 1) {
        // Single-page layout: Show a centered "Back" button only in sub-page mode
        if (isSubPage) {
            const char* text1 = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 201);
            pipboyDrawText(text1, PIPBOY_TEXT_ALIGNMENT_CENTER, _colorTable[992]);
        }
        return; // no button if not subpage (default behavior)
    }

    if (isSubPage) {
        // Sub-page navigation (Back always on left, Done/More on right)
        const char* text1 = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 201);
        pipboyDrawText(text1, PIPBOY_TEXT_ALIGNMENT_LEFT_COLUMN_CENTER, _colorTable[992]);

        const char* text2 = (_view_page >= totalPages - 1)
            ? getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 214) // Done
            : getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 200); // More
        pipboyDrawText(text2, PIPBOY_TEXT_ALIGNMENT_RIGHT_COLUMN_CENTER, _colorTable[992]);

    } else {
        // Main-page navigation (Back only appears after first page, More only before last)
        if (_view_page > 0) {
            const char* text1 = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 201);
            pipboyDrawText(text1, PIPBOY_TEXT_ALIGNMENT_LEFT_COLUMN_CENTER, _colorTable[992]);
        } else {
            // 'greyed out' buttons - not clickable - just for style
            const char* text1 = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 201);
            pipboyDrawText(text1, PIPBOY_TEXT_ALIGNMENT_LEFT_COLUMN_CENTER, _colorTable[8804]);
        }
        if (_view_page < totalPages - 1) {
            const char* text2 = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 200);
            pipboyDrawText(text2, PIPBOY_TEXT_ALIGNMENT_RIGHT_COLUMN_CENTER, _colorTable[992]);
        } else {
            // 'greyed out' buttons - not clickable - just for style
            const char* text2 = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 200);
            pipboyDrawText(text2, PIPBOY_TEXT_ALIGNMENT_RIGHT_COLUMN_CENTER, _colorTable[8804]);
        }
    }
}

// 0x497004
int pipboyOpen(int intent)
{
    if (!wmMapPipboyActive() && !pipboy_available_at_game_start) {
        // You aren't wearing the pipboy!
        const char* text = getmsg(&gMiscMessageList, &gPipboyMessageListItem, 7000);
        showDialogBox(text, nullptr, 0, 192, 135, _colorTable[32328], nullptr, _colorTable[32328], 1);
        return 0;
    }

    ScopedGameMode gm(GameMode::kPipboy);

    intent = pipboyWindowInit(intent);
    if (intent == -1) {
        return -1;
    }

    mouseGetPositionInWindow(gPipboyWindow, &gPipboyPreviousMouseX, &gPipboyPreviousMouseY);
    gPipboyLastEventTimestamp = getTicks();

    while (true) {
        sharedFpsLimiter.mark();

        int keyCode = inputGetInput();

        if (intent == PIPBOY_OPEN_INTENT_REST) {
            keyCode = 504;
            intent = PIPBOY_OPEN_INTENT_UNSPECIFIED;
        }

        mouseGetPositionInWindow(gPipboyWindow, &gPipboyMouseX, &gPipboyMouseY);

        if (keyCode != -1 || gPipboyMouseX != gPipboyPreviousMouseX || gPipboyMouseY != gPipboyPreviousMouseY) {
            gPipboyLastEventTimestamp = getTicks();
            gPipboyPreviousMouseX = gPipboyMouseX;
            gPipboyPreviousMouseY = gPipboyMouseY;
        } else {
            if (getTicks() - gPipboyLastEventTimestamp > PIPBOY_IDLE_TIMEOUT) {
                pipboyRenderScreensaver();

                gPipboyLastEventTimestamp = getTicks();
                mouseGetPositionInWindow(gPipboyWindow, &gPipboyPreviousMouseX, &gPipboyPreviousMouseY);
            }
        }

        if (keyCode == KEY_CTRL_Q || keyCode == KEY_CTRL_X || keyCode == KEY_F10) {
            showQuitConfirmationDialog();
            break;
        }

        // SFALL: Close with 'Z'.
        if (keyCode == 503 || keyCode == KEY_ESCAPE || keyCode == KEY_UPPERCASE_P || keyCode == KEY_LOWERCASE_P || keyCode == KEY_UPPERCASE_Z || keyCode == KEY_LOWERCASE_Z || _game_user_wants_to_quit != 0) {
            break;
        }

        // Handle Return key - if in keyboard mode, use for selection; otherwise exit
        if (keyCode == KEY_RETURN) {
            if (gPipboyKeyboardMode) {
                // In keyboard mode, use Return for selection
                _PipFnctn[gPipboyTab](PIPBOY_KEY_SELECT); // Send select event
            } else {
                // Not in keyboard mode, exit Pipboy
                break;
            }
            continue;
        }

        // Arrow key support for pagination
        if (keyCode == KEY_ARROW_LEFT) {
            // Left arrow - go to previous page OR switch to quest column
            _PipFnctn[gPipboyTab](PIPBOY_KEY_LEFT); // New event for column switching
            continue;
        } else if (keyCode == KEY_ARROW_RIGHT) {
            // Right arrow - go to next page OR switch to holodisk column
            _PipFnctn[gPipboyTab](PIPBOY_KEY_RIGHT); // New event for column switching
            continue;
        } else if (keyCode == KEY_ARROW_UP) {
            // Up arrow - navigate up in lists
            _PipFnctn[gPipboyTab](PIPBOY_KEY_UP); // New event for up arrow
            continue;
        } else if (keyCode == KEY_ARROW_DOWN) {
            // Down arrow - navigate down in lists
            _PipFnctn[gPipboyTab](PIPBOY_KEY_DOWN); // New event for down arrow
            continue;
        } else if (keyCode == KEY_RETURN || keyCode == KEY_UPPERCASE_E || keyCode == KEY_LOWERCASE_E) {
            // Enter/E key - select highlighted item
            _PipFnctn[gPipboyTab](PIPBOY_KEY_SELECT); // New event for Enter/select
            continue;
        }

        if (keyCode == KEY_F12) {
            takeScreenshot();
        } else if (keyCode >= 500 && keyCode <= 504) {
            // CE: Save previous tab selected so that the underlying handlers
            // (alarm clock in particular) can fallback if something goes wrong.
            gPipboyPrevTab = gPipboyTab;

            gPipboyTab = keyCode - 500;
            _view_page_automap_main = 0; // ensures button click to automaps renders first page
            _view_page_quest = 0; // ensures button click to status renders first page
            _view_page_holodisk = 0; // ensures button click to status renders first page
            _PipFnctn[gPipboyTab](1024);
        } else if (keyCode >= 505 && keyCode <= 527) {
            _PipFnctn[gPipboyTab](keyCode - 506);
        } else if (keyCode == 528) {
            _PipFnctn[gPipboyTab](1025);
        } else if (keyCode == KEY_PAGE_DOWN) {
            _PipFnctn[gPipboyTab](1026);
        } else if (keyCode == KEY_PAGE_UP) {
            _PipFnctn[gPipboyTab](1027);
        }

        if (_proc_bail_flag) {
            break;
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    pipboyWindowFree();

    return 0;
}

int pipboyMessageListInit()
{
    pipboyMessageListFree();

    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s%s", asc_5186C8, "pipboy.msg");

    if (!(messageListLoadWithMods(&gPipboyMessageList, path, "PIPBOY"))) {
        return -1;
    }

    return 0;
}

void pipboyMessageListFree()
{
    if (gPipboyMessageList.entries != nullptr) {
        messageListFree(&gPipboyMessageList);
    }
    messageListInit(&gPipboyMessageList);
}

// 0x497228
static int pipboyWindowInit(int intent)
{
    gPipboyWindowIsoWasEnabled = isoDisable();

    colorCycleDisable();
    gameMouseObjectsHide();
    indicatorBarHide();
    gameMouseSetCursor(MOUSE_CURSOR_ARROW);

    gPipboyRestOptionsCount = PIPBOY_REST_DURATION_COUNT_WITHOUT_PARTY;

    if (_getPartyMemberCount() > 1 && partyIsAnyoneCanBeHealedByRest()) {
        gPipboyRestOptionsCount = PIPBOY_REST_DURATION_COUNT;
    }

    gPipboyWindowOldFont = fontGetCurrent();
    fontSetCurrent(101);

    _proc_bail_flag = 0;
    _rest_time = 0;
    gPipboyCurrentLine = 0;
    gPipboyWindowButtonCount = 0;
    gPipboyLinesCount = PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT / fontGetLineHeight() - 1;
    gPipboyWindowButtonStart = 0;
    _hot_back_line = 0;

    if (holodiskInit() == -1) {
        return -1;
    }

    if (pipboyMessageListInit() == -1) {
        return -1;
    }

    int index;
    for (index = 0; index < PIPBOY_FRM_COUNT; index++) {
        int fid = buildFid(OBJ_TYPE_INTERFACE, gPipboyFrmIds[index], 0, 0, 0);
        if (!_pipboyFrmImages[index].lock(fid)) {
            break;
        }
    }

    if (index != PIPBOY_FRM_COUNT) {
        debugPrint("\n** Error loading pipboy graphics! **\n");

        while (--index >= 0) {
            _pipboyFrmImages[index].unlock();
        }

        return -1;
    }

    int pipboyWindowX = (screenGetWidth() - PIPBOY_WINDOW_WIDTH) / 2;
    int pipboyWindowY = (screenGetHeight() - PIPBOY_WINDOW_HEIGHT) / 2;
    gPipboyWindow = windowCreate(pipboyWindowX, pipboyWindowY, PIPBOY_WINDOW_WIDTH, PIPBOY_WINDOW_HEIGHT, _colorTable[0], WINDOW_MODAL | WINDOW_TRANSPARENT | WINDOW_DRAGGABLE_BY_BACKGROUND);
    if (gPipboyWindow == -1) {
        debugPrint("\n** Error opening pipboy window! **\n");
        for (int index = 0; index < PIPBOY_FRM_COUNT; index++) {
            _pipboyFrmImages[index].unlock();
        }
        return -1;
    }

    gPipboyWindowBuffer = windowGetBuffer(gPipboyWindow);
    memcpy(gPipboyWindowBuffer, _pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData(), PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_HEIGHT);

    pipboyDrawNumber(gameTimeGetHour(), 4, PIPBOY_WINDOW_TIME_X, PIPBOY_WINDOW_TIME_Y);
    pipboyDrawDate();

    int alarmButton = buttonCreate(gPipboyWindow,
        124,
        13,
        _pipboyFrmImages[PIPBOY_FRM_ALARM_UP].getWidth(),
        _pipboyFrmImages[PIPBOY_FRM_ALARM_UP].getHeight(),
        -1,
        -1,
        -1,
        504,
        _pipboyFrmImages[PIPBOY_FRM_ALARM_UP].getData(),
        _pipboyFrmImages[PIPBOY_FRM_ALARM_DOWN].getData(),
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    if (alarmButton != -1) {
        buttonSetCallbacks(alarmButton, _gsound_med_butt_press, _gsound_med_butt_release);
    }

    int y = 340;
    int eventCode = 500;
    int yOffsets[] = { 27, 27, 29, 25, 27 }; // handles slighlty misaligned buttons on background
    for (int index = 0; index < 5; index += 1) {
        if (index != 1) {
            int btn = buttonCreate(gPipboyWindow,
                53,
                y,
                _pipboyFrmImages[PIPBOY_FRM_LITTLE_RED_BUTTON_UP].getWidth(),
                _pipboyFrmImages[PIPBOY_FRM_LITTLE_RED_BUTTON_UP].getHeight(),
                -1,
                -1,
                -1,
                eventCode,
                _pipboyFrmImages[PIPBOY_FRM_LITTLE_RED_BUTTON_UP].getData(),
                _pipboyFrmImages[PIPBOY_FRM_LITTLE_RED_BUTTON_DOWN].getData(),
                nullptr,
                BUTTON_FLAG_TRANSPARENT);
            if (btn != -1) {
                buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
            }

            eventCode += 1;
        }

        y += yOffsets[index];
    }

    if (intent == PIPBOY_OPEN_INTENT_REST) {
        if (!_critter_can_obj_dude_rest()) {
            blitBufferToBufferTrans(
                _pipboyFrmImages[PIPBOY_FRM_LOGO].getData(),
                _pipboyFrmImages[PIPBOY_FRM_LOGO].getWidth(),
                _pipboyFrmImages[PIPBOY_FRM_LOGO].getHeight(),
                _pipboyFrmImages[PIPBOY_FRM_LOGO].getWidth(),
                gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * 156 + 323,
                PIPBOY_WINDOW_WIDTH);

            int month;
            int day;
            int year;
            gameTimeGetDate(&month, &day, &year);

            int holiday = 0;
            for (; holiday < HOLIDAY_COUNT; holiday += 1) {
                const HolidayDescription* holidayDescription = &(gHolidayDescriptions[holiday]);
                if (holidayDescription->month == month && holidayDescription->day == day) {
                    break;
                }
            }

            if (holiday != HOLIDAY_COUNT) {
                const HolidayDescription* holidayDescription = &(gHolidayDescriptions[holiday]);
                const char* holidayName = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, holidayDescription->textId);
                char holidayNameCopy[256];
                strcpy(holidayNameCopy, holidayName);

                int len = fontGetStringWidth(holidayNameCopy);
                fontDrawText(gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * (_pipboyFrmImages[PIPBOY_FRM_LOGO].getHeight() + 174) + 6 + _pipboyFrmImages[PIPBOY_FRM_LOGO].getWidth() / 2 + 323 - len / 2,
                    holidayNameCopy,
                    350,
                    PIPBOY_WINDOW_WIDTH,
                    _colorTable[992]);
            }

            windowRefresh(gPipboyWindow);

            soundPlayFile("iisxxxx1");

            const char* text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 215);
            showDialogBox(text, nullptr, 0, 192, 135, _colorTable[32328], nullptr, _colorTable[32328], DIALOG_BOX_LARGE);

            intent = PIPBOY_OPEN_INTENT_UNSPECIFIED;
        }
    } else {
        blitBufferToBufferTrans(
            _pipboyFrmImages[PIPBOY_FRM_LOGO].getData(),
            _pipboyFrmImages[PIPBOY_FRM_LOGO].getWidth(),
            _pipboyFrmImages[PIPBOY_FRM_LOGO].getHeight(),
            _pipboyFrmImages[PIPBOY_FRM_LOGO].getWidth(),
            gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * 156 + 323,
            PIPBOY_WINDOW_WIDTH);

        int month;
        int day;
        int year;
        gameTimeGetDate(&month, &day, &year);

        int holiday;
        for (holiday = 0; holiday < HOLIDAY_COUNT; holiday += 1) {
            const HolidayDescription* holidayDescription = &(gHolidayDescriptions[holiday]);
            if (holidayDescription->month == month && holidayDescription->day == day) {
                break;
            }
        }

        if (holiday != HOLIDAY_COUNT) {
            const HolidayDescription* holidayDescription = &(gHolidayDescriptions[holiday]);
            const char* holidayName = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, holidayDescription->textId);
            char holidayNameCopy[256];
            strcpy(holidayNameCopy, holidayName);

            int length = fontGetStringWidth(holidayNameCopy);
            fontDrawText(gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * (_pipboyFrmImages[PIPBOY_FRM_LOGO].getHeight() + 174) + 6 + _pipboyFrmImages[PIPBOY_FRM_LOGO].getWidth() / 2 + 323 - length / 2,
                holidayNameCopy,
                350,
                PIPBOY_WINDOW_WIDTH,
                _colorTable[992]);
        }

        windowRefresh(gPipboyWindow);
    }

    if (questInit() == -1) {
        return -1;
    }

    soundPlayFile("pipon");
    windowRefresh(gPipboyWindow);

    return intent;
}

// 0x497828
static void pipboyWindowFree()
{
    if (settings.debug.show_script_messages) {
        debugPrint("\nScript <Map Update>");
    }

    scriptsExecMapUpdateProc();

    windowDestroy(gPipboyWindow);

    pipboyMessageListFree();

    // NOTE: Uninline.
    holodiskFree();

    for (int index = 0; index < PIPBOY_FRM_COUNT; index++) {
        _pipboyFrmImages[index].unlock();
    }

    pipboyWindowDestroyButtons();

    fontSetCurrent(gPipboyWindowOldFont);

    if (gPipboyWindowIsoWasEnabled) {
        isoEnable();
    }

    colorCycleEnable();
    indicatorBarShow();
    gameMouseSetCursor(MOUSE_CURSOR_ARROW);
    interfaceBarRefresh();

    // NOTE: Uninline.
    questFree();
}

// NOTE: Collapsed.
//
// 0x497918
static void _pip_init_()
{
    // SFALL: Make the pipboy available at the start of the game.
    // CE: The implementation is slightly different. SFALL has two values for
    // making the pipboy available at the start of the game. When the option is
    // set to (1), the `MOVIE_VSUIT` is automatically marked as viewed (the suit
    // grants the pipboy, see `wmMapPipboyActive`). Doing so exposes that movie
    // in the "Video Archives" section of the pipboy, which is likely an
    // undesired side effect. When the option is set to (2), the check is simply
    // bypassed. CE implements only the latter approach, as it does not have any
    // side effects.
    int value = 0;
    if (configGetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PIPBOY_AVAILABLE_AT_GAMESTART, &value)) {
        pipboy_available_at_game_start = value == 1 || value == 2;
    }
}

// NOTE: Uncollapsed 0x497918.
//
// pip_init
void pipboyInit()
{
    _pip_init_();
}

// NOTE: Uncollapsed 0x497918.
void pipboyReset()
{
    _pip_init_();
}

// 0x49791C
static void pipboyDrawNumber(int value, int digits, int x, int y)
{
    int offset = PIPBOY_WINDOW_WIDTH * y + x + 9 * (digits - 1);

    for (int index = 0; index < digits; index++) {
        blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_NUMBERS].getData() + 9 * (value % 10), 9, 17, 360, gPipboyWindowBuffer + offset, PIPBOY_WINDOW_WIDTH);
        offset -= 9;
        value /= 10;
    }
}

// 0x4979B4
static void pipboyDrawDate()
{
    int day;
    int month;
    int year;

    gameTimeGetDate(&month, &day, &year);
    pipboyDrawNumber(day, 2, PIPBOY_WINDOW_DAY_X, PIPBOY_WINDOW_DAY_Y);

    blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_MONTHS].getData() + 435 * (month - 1), 29, 14, 29, gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_MONTH_Y + PIPBOY_WINDOW_MONTH_X, PIPBOY_WINDOW_WIDTH);

    pipboyDrawNumber(year, 4, PIPBOY_WINDOW_YEAR_X, PIPBOY_WINDOW_YEAR_Y);
}

// 0x497A40
static void pipboyDrawText(const char* text, int flags, int color)
{
    if ((flags & PIPBOY_TEXT_STYLE_UNDERLINE) != 0) {
        color |= FONT_UNDERLINE;
    }

    int left = 8;
    if ((flags & PIPBOY_TEXT_NO_INDENT) != 0) {
        left -= 7;
    }

    int length = fontGetStringWidth(text);

    if ((flags & PIPBOY_TEXT_ALIGNMENT_CENTER) != 0) {
        left = (350 - length) / 2;
    } else if ((flags & PIPBOY_TEXT_ALIGNMENT_RIGHT_COLUMN) != 0) {
        left += 175;
    } else if ((flags & PIPBOY_TEXT_ALIGNMENT_LEFT_COLUMN_CENTER) != 0) {
        left += 86 - length + 16;
    } else if ((flags & PIPBOY_TEXT_ALIGNMENT_RIGHT_COLUMN_CENTER) != 0) {
        left += 260 - length;
    }

    fontDrawText(gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * (gPipboyCurrentLine * fontGetLineHeight() + PIPBOY_WINDOW_CONTENT_VIEW_Y) + PIPBOY_WINDOW_CONTENT_VIEW_X + left, text, PIPBOY_WINDOW_WIDTH, PIPBOY_WINDOW_WIDTH, color);

    if ((flags & PIPBOY_TEXT_STYLE_STRIKE_THROUGH) != 0) {
        int top = gPipboyCurrentLine * fontGetLineHeight() + 49;
        bufferDrawLine(gPipboyWindowBuffer, PIPBOY_WINDOW_WIDTH, PIPBOY_WINDOW_CONTENT_VIEW_X + left, top, PIPBOY_WINDOW_CONTENT_VIEW_X + left + length, top, color);
    }

    if (gPipboyCurrentLine < gPipboyLinesCount) {
        gPipboyCurrentLine += 1;
    }
}

// Handles rendering the pagination text at the top-right for _PrintAMelevList and _PrintAMList and others
static void renderPagination(int currentPage, int totalPages)
{
    if (totalPages > 1) {
        const char* of = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 212);
        char formattedText[60]; // Ensure sufficient size
        snprintf(formattedText, sizeof(formattedText), "%d %s %d", currentPage + 1, of, totalPages);

        int len = fontGetStringWidth(of);
        fontDrawText(gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * 47 + 616 + 604 - len, formattedText, 350, PIPBOY_WINDOW_WIDTH, _colorTable[992]);
    }
}

// Function for page navigation - Back, More, Done at bottom of page
void handlePipboyPageNavigation(
    int mouseX,
    int threshold,
    int* viewPage,
    int totalPages,
    void (*handle_outofRange)(int),
    void (*updatePage)(void))
{
    // Blit animation effect
    blitBufferToBuffer(
        _pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * 436 + 254,
        350, 20, PIPBOY_WINDOW_WIDTH,
        gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * 436 + 254,
        PIPBOY_WINDOW_WIDTH);

    bool isRightButton = mouseX > threshold;

    // Check boundaries and return to automap menu if in submenu (range is blocked for main navigation)
    if ((isRightButton && *viewPage >= totalPages - 1) || (!isRightButton && *viewPage <= 0)) {
        handle_outofRange(1024);
        return;
    }

    // Adjust the view page
    *viewPage += isRightButton ? 1 : -1;

    // Update the page content
    updatePage();
}

// NOTE: Collapsed.
//
// 0x497BD4
static int _save_pipboy(File* stream)
{
    return 0;
}

// NOTE: Uncollapsed 0x497BD4.
int pipboySave(File* stream)
{
    return _save_pipboy(stream);
}

// NOTE: Uncollapsed 0x497BD4.
int pipboyLoad(File* stream)
{
    return _save_pipboy(stream);
}

static void pipboyRedrawStatusContent()
{
    // Clear the content area
    blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
        PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
        PIPBOY_WINDOW_WIDTH,
        gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_WIDTH);

    if (gPipboyLinesCount >= 0) {
        gPipboyCurrentLine = 0;
    }

    // Render quest locations
    pipboyWindowRenderQuestLocationList(-1);

    if (gPipboyQuestLocationsCount == 0) {
        const char* text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 203);
        pipboyDrawText(text, 0, _colorTable[992]);
    }

    // Render holodisk list
    pipboyWindowRenderHolodiskList(-1);

    windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
}

// 0x497BD8
static void pipboyWindowHandleStatus(int userInput)
{
    if (userInput == 1024) {
        pipboyWindowDestroyButtons();
        blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
            PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
            PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
            PIPBOY_WINDOW_WIDTH,
            gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
            PIPBOY_WINDOW_WIDTH);
        if (gPipboyLinesCount >= 0) {
            gPipboyCurrentLine = 0;
        }

        _holo_flag = 0;
        _holodisk = -1;
        gPipboyWindowHolodisksCount = 0;
        _view_page = 0;
        _view_page_questlist = 0;
        _stat_flag = 0;

        for (int index = 0; index < gHolodisksCount; index += 1) {
            HolodiskDescription* holodiskDescription = &(gHolodiskDescriptions[index]);
            if (gGameGlobalVars[holodiskDescription->gvar] != 0) {
                gPipboyWindowHolodisksCount += 1;
                break;
            }
        }

        // Initialize keyboard selection for main status page
        gPipboySelectedQuestIndex = 0;
        gPipboySelectedHolodiskIndex = 0;
        gPipboySelectedIndex = 0;
        gPipboyKeyboardMode = true;
        gPipboyCurrentColumn = PIPBOY_COLUMN_QUESTS; // Start in quest column

        pipboyWindowRenderQuestLocationList(-1);

        if (gPipboyQuestLocationsCount == 0) {
            const char* text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 203);
            pipboyDrawText(text, 0, _colorTable[992]);
        }

        gPipboyWindowHolodisksCount = pipboyWindowRenderHolodiskList(-1);

        windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
        pipboyWindowCreateButtons(2, gPipboyQuestLocationsCount + gPipboyWindowHolodisksCount + 1, true);
        windowRefresh(gPipboyWindow);
        return;
    }

    // Arrow key support for status page navigation
    if (userInput == 1026) { // Right arrow / Next page
        if (_stat_flag == 0 && _holo_flag == 0) {
            // Main status page
            if (_view_page_quest < totalPages - 1) {
                _view_page_quest++;
                pipboyWindowDestroyButtons();
                soundPlayFile("ib1p1xx1");
                pipboyWindowHandleStatus(1024);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                windowRefresh(gPipboyWindow);
            }
        } else if (_holo_flag == 1) {
            // Holodisk page
            if (_view_page < gPipboyHolodiskLastPage) {
                _view_page++;
                pipboyWindowDestroyButtons();
                soundPlayFile("ib1p1xx1");
                pipboyRenderHolodiskText();
                pipboyWindowCreateButtons(0, 0, true);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                windowRefresh(gPipboyWindow);
            }
        } else if (_stat_flag == 1) {
            // Quest list page
            if (_view_page_questlist < totalPages - 1) {
                _view_page_questlist++;
                soundPlayFile("ib1p1xx1");
                pipboyWindowQuestList(-1);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                windowRefresh(gPipboyWindow);
            }
        }
        return;
    } else if (userInput == 1027) { // Left arrow / Previous page
        if (_stat_flag == 0 && _holo_flag == 0) {
            // Main status page
            if (_view_page_quest > 0) {
                _view_page_quest--;
                pipboyWindowDestroyButtons();
                soundPlayFile("ib1p1xx1");
                pipboyWindowHandleStatus(1024);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                windowRefresh(gPipboyWindow);
            }
        } else if (_holo_flag == 1) {
            // Holodisk page
            if (_view_page > 0) {
                _view_page--;
                pipboyWindowDestroyButtons();
                soundPlayFile("ib1p1xx1");
                pipboyRenderHolodiskText();
                pipboyWindowCreateButtons(0, 0, true);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                windowRefresh(gPipboyWindow);
            }
        } else if (_stat_flag == 1) {
            // Quest list page
            if (_view_page_questlist > 0) {
                _view_page_questlist--;
                soundPlayFile("ib1p1xx1");
                pipboyWindowQuestList(-1);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                windowRefresh(gPipboyWindow);
            }
        }
        return;
    }

    // Handle up arrow (PIPBOY_KEY_UP) - navigate within current column
    if (userInput == PIPBOY_KEY_UP) {

        if (_stat_flag == 0 && _holo_flag == 0) {
            // Main status page
            if (!gPipboyKeyboardMode) {
                gPipboyKeyboardMode = true;
                gPipboySelectedIndex = 0;
                gPipboyCurrentColumn = PIPBOY_COLUMN_QUESTS;
            } else {
                if (gPipboyCurrentColumn == PIPBOY_COLUMN_QUESTS) {
                    // In quest column
                    if (gPipboySelectedQuestIndex > 0) {
                        gPipboySelectedQuestIndex--;
                        gPipboySelectedIndex = gPipboySelectedQuestIndex;
                        pipboyRedrawStatusPageWithSelection();
                    } else {
                        // At top of quest column, go to previous page for both lists
                        int totalQuestPages = (gPipboyQuestLocationsCount + PIPBOY_STATUS_QUEST_LINES - 1) / PIPBOY_STATUS_QUEST_LINES;

                        // Count total holodisks for pagination
                        int totalHolodisks = 0;
                        for (int index = 0; index < gHolodisksCount; index++) {
                            if (gGameGlobalVars[gHolodiskDescriptions[index].gvar] != 0) {
                                totalHolodisks++;
                            }
                        }
                        int totalHolodiskPages = (totalHolodisks + PIPBOY_STATUS_HOLODISK_LINES - 1) / PIPBOY_STATUS_HOLODISK_LINES;

                        // Go to previous page for quests if available
                        if (_view_page_quest > 0) {
                            soundPlayFile("ib1p1xx1");
                            _view_page_quest--;

                            // Calculate how many quests are on the new page
                            int maxEntriesPerPage = PIPBOY_STATUS_QUEST_LINES;
                            int startIndex = _view_page_quest * maxEntriesPerPage;
                            int endIndex = startIndex + maxEntriesPerPage;
                            if (endIndex > gPipboyQuestLocationsCount) {
                                endIndex = gPipboyQuestLocationsCount;
                            }

                            int questsOnNewPage = endIndex - startIndex;

                            // Set selection to last item on the new page
                            gPipboySelectedQuestIndex = questsOnNewPage - 1;
                            if (gPipboySelectedQuestIndex < 0) gPipboySelectedQuestIndex = 0;
                            gPipboySelectedIndex = gPipboySelectedQuestIndex;

                            // Also go to previous page for holodisks if available
                            if (_view_page_holodisk > 0) {
                                _view_page_holodisk--;
                            }

                            pipboyRedrawStatusPageWithSelection();
                        }
                    }
                } else if (gPipboyCurrentColumn == PIPBOY_COLUMN_HOLODISKS) {
                    // In holodisk column - calculate max index correctly

                    // Calculate how many holodisks are on the CURRENT page
                    int holodisksOnCurrentPage = 0;
                    int currentHolodiskCount = 0;

                    for (int index = 0; index < gHolodisksCount; index++) {
                        if (gGameGlobalVars[gHolodiskDescriptions[index].gvar] != 0) {
                            // Check if this holodisk is on the current page
                            if (currentHolodiskCount >= (_view_page_holodisk * PIPBOY_STATUS_HOLODISK_LINES) && currentHolodiskCount < ((_view_page_holodisk + 1) * PIPBOY_STATUS_HOLODISK_LINES)) {
                                holodisksOnCurrentPage++;
                            }
                            currentHolodiskCount++;
                        }
                    }

                    if (gPipboySelectedHolodiskIndex > 0) {
                        gPipboySelectedHolodiskIndex--;
                        pipboyRedrawStatusPageWithSelection();
                    } else {
                        // At top of holodisk column, go to previous page for both lists
                        int totalHolodisks = 0;
                        for (int index = 0; index < gHolodisksCount; index++) {
                            if (gGameGlobalVars[gHolodiskDescriptions[index].gvar] != 0) {
                                totalHolodisks++;
                            }
                        }

                        int totalHolodiskPages = (totalHolodisks + PIPBOY_STATUS_HOLODISK_LINES - 1) / PIPBOY_STATUS_HOLODISK_LINES;
                        int totalQuestPages = (gPipboyQuestLocationsCount + PIPBOY_STATUS_QUEST_LINES - 1) / PIPBOY_STATUS_QUEST_LINES;

                        // Go to previous page for holodisks if available
                        if (_view_page_holodisk > 0) {
                            _view_page_holodisk--;

                            soundPlayFile("ib1p1xx1");

                            // Calculate holodisks on new page
                            holodisksOnCurrentPage = 0;
                            currentHolodiskCount = 0;

                            for (int index = 0; index < gHolodisksCount; index++) {
                                if (gGameGlobalVars[gHolodiskDescriptions[index].gvar] != 0) {
                                    // Check if this holodisk is on the new page
                                    if (currentHolodiskCount >= (_view_page_holodisk * PIPBOY_STATUS_HOLODISK_LINES) && currentHolodiskCount < ((_view_page_holodisk + 1) * PIPBOY_STATUS_HOLODISK_LINES)) {
                                        holodisksOnCurrentPage++;
                                    }
                                    currentHolodiskCount++;
                                }
                            }

                            // Set to last holodisk on the new page
                            gPipboySelectedHolodiskIndex = holodisksOnCurrentPage - 1;
                            if (gPipboySelectedHolodiskIndex < 0) gPipboySelectedHolodiskIndex = 0;

                            // Also go to previous page for quests if available
                            if (_view_page_quest > 0) {
                                _view_page_quest--;
                            }

                            // rebuild entire page
                            pipboyWindowHandleStatus(1024);
                        }
                    }
                }
            }
        }
        return;
    }

    // Handle down arrow (PIPBOY_KEY_DOWN) - navigate within current column
    if (userInput == PIPBOY_KEY_DOWN) {

        if (_stat_flag == 0 && _holo_flag == 0) {
            // Main status page
            if (!gPipboyKeyboardMode) {
                gPipboyKeyboardMode = true;
                gPipboySelectedIndex = 0;
                gPipboyCurrentColumn = PIPBOY_COLUMN_QUESTS;
            } else {
                if (gPipboyCurrentColumn == PIPBOY_COLUMN_QUESTS) {
                    // In quest column
                    int maxQuestIndex = gPipboyWindowQuestsCurrentPageCount - 1;

                    if (gPipboySelectedQuestIndex < maxQuestIndex) {
                        gPipboySelectedQuestIndex++;
                        gPipboySelectedIndex = gPipboySelectedQuestIndex;
                        pipboyRedrawStatusPageWithSelection();
                    } else {
                        // At bottom of quest column, go to next page for both lists
                        int totalQuestPages = (gPipboyQuestLocationsCount + PIPBOY_STATUS_QUEST_LINES - 1) / PIPBOY_STATUS_QUEST_LINES;

                        // Count total holodisks for pagination
                        int totalHolodisks = 0;
                        for (int index = 0; index < gHolodisksCount; index++) {
                            if (gGameGlobalVars[gHolodiskDescriptions[index].gvar] != 0) {
                                totalHolodisks++;
                            }
                        }
                        int totalHolodiskPages = (totalHolodisks + PIPBOY_STATUS_HOLODISK_LINES - 1) / PIPBOY_STATUS_HOLODISK_LINES;

                        // Go to next page for quests if available
                        if (_view_page_quest < totalQuestPages - 1) {
                            soundPlayFile("ib1p1xx1");
                            _view_page_quest++;
                            gPipboySelectedQuestIndex = 0;
                            gPipboySelectedIndex = 0;

                            // Also go to next page for holodisks if available
                            if (_view_page_holodisk < totalHolodiskPages - 1) {
                                _view_page_holodisk++;
                            }

                            pipboyWindowHandleStatus(1024);
                        }
                    }
                } else if (gPipboyCurrentColumn == PIPBOY_COLUMN_HOLODISKS) {
                    // In holodisk column - calculate max index correctly

                    // Calculate how many holodisks are on the current page
                    int holodisksOnCurrentPage = 0;
                    int currentHolodiskCount = 0;

                    for (int index = 0; index < gHolodisksCount; index++) {
                        if (gGameGlobalVars[gHolodiskDescriptions[index].gvar] != 0) {
                            // Check if this holodisk is on the current page
                            if (currentHolodiskCount >= (_view_page_holodisk * PIPBOY_STATUS_HOLODISK_LINES) && currentHolodiskCount < ((_view_page_holodisk + 1) * PIPBOY_STATUS_HOLODISK_LINES)) {
                                holodisksOnCurrentPage++;
                            }
                            currentHolodiskCount++;
                        }
                    }

                    int maxHolodiskIndex = holodisksOnCurrentPage - 1;

                    if (gPipboySelectedHolodiskIndex < maxHolodiskIndex) {
                        gPipboySelectedHolodiskIndex++;
                        pipboyRedrawStatusPageWithSelection();
                    } else {
                        // At bottom of holodisk column, go to next page for both lists
                        int totalHolodisks = 0;
                        for (int index = 0; index < gHolodisksCount; index++) {
                            if (gGameGlobalVars[gHolodiskDescriptions[index].gvar] != 0) {
                                totalHolodisks++;
                            }
                        }

                        int totalHolodiskPages = (totalHolodisks + PIPBOY_STATUS_HOLODISK_LINES - 1) / PIPBOY_STATUS_HOLODISK_LINES;
                        int totalQuestPages = (gPipboyQuestLocationsCount + PIPBOY_STATUS_QUEST_LINES - 1) / PIPBOY_STATUS_QUEST_LINES;

                        // Go to next page for holodisks if available
                        if (_view_page_holodisk < totalHolodiskPages - 1) {
                            _view_page_holodisk++;

                            soundPlayFile("ib1p1xx1");

                            gPipboySelectedHolodiskIndex = 0;

                            // Also go to next page for quests if available
                            if (_view_page_quest < totalQuestPages - 1) {
                                _view_page_quest++;
                            }

                            // Rebuild page
                            pipboyWindowHandleStatus(1024);
                        }
                    }
                }
            }
        }
        return;
    }

    // Handle right arrow (PIPBOY_KEY_RIGHT) - switch from quests to holodisks OR next page when in holodisk column
    if (userInput == PIPBOY_KEY_RIGHT) {
        if (_holo_flag == 1) {
            // In holodisk subpage
            if (gPipboyHolodiskLastPage == 0) {
                // Only one page - return to main (like "Back" button)
                soundPlayFile("ib1p1xx1");
                _holo_flag = 0;
                pipboyWindowHandleStatus(1024);
            } else if (_view_page < gPipboyHolodiskLastPage) {
                // Not on last page - go to next page
                _view_page++;
                pipboyWindowDestroyButtons();
                soundPlayFile("ib1p1xx1");
                pipboyRenderHolodiskText();
                pipboyWindowCreateButtons(0, 0, true);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            } else {
                // On last page - return to main
                soundPlayFile("ib1p1xx1");
                _holo_flag = 0;
                pipboyWindowHandleStatus(1024);
            }
            return;
        }

        if (_stat_flag == 1) {
            // In quest list subpage
            if (totalPages <= 1) {
                // Only one page - return to main (like "Back" button)
                soundPlayFile("ib1p1xx1");
                _stat_flag = 0;
                pipboyWindowHandleStatus(1024);
            } else if (_view_page_questlist < totalPages - 1) {
                // Not on last page - go to next page
                _view_page_questlist++;
                soundPlayFile("ib1p1xx1");
                pipboyWindowQuestList(-1);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            } else {
                // On last page - return to main
                soundPlayFile("ib1p1xx1");
                _stat_flag = 0;
                pipboyWindowHandleStatus(1024);
            }
            return;
        }

        if (_stat_flag == 0 && _holo_flag == 0) {
            // Main status page
            if (!gPipboyKeyboardMode) {
                gPipboyKeyboardMode = true;
                gPipboySelectedIndex = 0;
                gPipboyCurrentColumn = PIPBOY_COLUMN_QUESTS;
            } else {
                if (gPipboyCurrentColumn == PIPBOY_COLUMN_QUESTS) {
                    // Switch to holodisk column - preserve quest index
                    gPipboyCurrentColumn = PIPBOY_COLUMN_HOLODISKS;

                    // Try to use the same index in holodisk list
                    gPipboySelectedHolodiskIndex = fmin(gPipboySelectedQuestIndex, gPipboyWindowHolodisksCount - 1);
                    if (gPipboySelectedHolodiskIndex < 0) gPipboySelectedHolodiskIndex = 0;
                    gPipboySelectedIndex = gPipboySelectedHolodiskIndex;

                    pipboyRedrawStatusPageWithSelection();
                } else if (gPipboyCurrentColumn == PIPBOY_COLUMN_HOLODISKS) {

                    // Calculate total pages for quests
                    int totalQuestPages = (gPipboyQuestLocationsCount + PIPBOY_STATUS_QUEST_LINES - 1) / PIPBOY_STATUS_QUEST_LINES;

                    // Count total holodisks for pagination
                    int totalHolodisks = 0;
                    for (int index = 0; index < gHolodisksCount; index++) {
                        HolodiskDescription* holodisk = &(gHolodiskDescriptions[index]);
                        if (gGameGlobalVars[holodisk->gvar] != 0) {
                            totalHolodisks++;
                        }
                    }
                    int totalHolodiskPages = (totalHolodisks + PIPBOY_STATUS_HOLODISK_LINES - 1) / PIPBOY_STATUS_HOLODISK_LINES;

                    // Store current column before rebuilding
                    PipboyColumn savedColumn = gPipboyCurrentColumn;

                    if (_view_page_quest < totalQuestPages - 1 || _view_page_holodisk < totalHolodiskPages - 1) {

                        // Go to next page for quests if available
                        if (_view_page_quest < totalQuestPages - 1) {
                            _view_page_quest++;
                        }

                        // Go to next page for holodisks if available
                        if (_view_page_holodisk < totalHolodiskPages - 1) {
                            _view_page_holodisk++;
                        }

                        soundPlayFile("ib1p1xx1");

                        // Update selection to first item in holodisk column
                        gPipboySelectedHolodiskIndex = 0;
                        gPipboySelectedIndex = 0;

                        // Rebuild the page - this resets column to quests
                        pipboyWindowHandleStatus(1024);

                        // Restore the column to holodisk after rebuild
                        gPipboyCurrentColumn = savedColumn;
                        gPipboyKeyboardMode = true; // Ensure keyboard mode is on
                        gPipboySelectedIndex = gPipboySelectedHolodiskIndex;

                        // Now redraw with holodisk column selected
                        pipboyRedrawStatusPageWithSelection();
                    } else {
                        // do nothing, last page
                    }
                }
            }
        }
        return;
    }

    // Handle left arrow (PIPBOY_KEY_LEFT) - switch from holodisks to quests OR previous page when in quest column
    if (userInput == PIPBOY_KEY_LEFT) {
        if (_holo_flag == 1) {
            // In holodisk subpage
            if (gPipboyHolodiskLastPage == 0) {
                // Only one page - return to main (like "Back" button)
                soundPlayFile("ib1p1xx1");
                _holo_flag = 0;
                pipboyWindowHandleStatus(1024);
            } else if (_view_page > 0) {
                // Not on first page - go to previous page
                _view_page--;
                pipboyWindowDestroyButtons();
                soundPlayFile("ib1p1xx1");
                pipboyRenderHolodiskText();
                pipboyWindowCreateButtons(0, 0, true);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            } else {
                // On first page - return to main
                soundPlayFile("ib1p1xx1");
                _holo_flag = 0;
                pipboyWindowHandleStatus(1024);
            }
            return;
        }

        if (_stat_flag == 1) {
            // In quest list subpage
            if (totalPages <= 1) {
                // Only one page - return to main (like "Back" button)
                soundPlayFile("ib1p1xx1");
                _stat_flag = 0;
                pipboyWindowHandleStatus(1024);
            } else if (_view_page_questlist > 0) {
                // Not on first page - go to previous page
                _view_page_questlist--;
                soundPlayFile("ib1p1xx1");
                pipboyWindowQuestList(-1);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            } else {
                // On first page - return to main
                soundPlayFile("ib1p1xx1");
                _stat_flag = 0;
                pipboyWindowHandleStatus(1024);
            }
            return;
        }

        if (_stat_flag == 0 && _holo_flag == 0) {
            // Main status page
            if (!gPipboyKeyboardMode) {
                gPipboyKeyboardMode = true;
                gPipboySelectedIndex = 0;
                gPipboyCurrentColumn = PIPBOY_COLUMN_QUESTS;
            } else {
                if (gPipboyCurrentColumn == PIPBOY_COLUMN_HOLODISKS) {
                    // Switch to quest column - preserve holodisk index
                    gPipboyCurrentColumn = PIPBOY_COLUMN_QUESTS;

                    // Try to use the same index in quest list
                    gPipboySelectedQuestIndex = fmin(gPipboySelectedHolodiskIndex, gPipboyWindowQuestsCurrentPageCount - 1);
                    if (gPipboySelectedQuestIndex < 0) gPipboySelectedQuestIndex = 0;
                    gPipboySelectedIndex = gPipboySelectedQuestIndex;

                    pipboyRedrawStatusPageWithSelection();
                } else if (gPipboyCurrentColumn == PIPBOY_COLUMN_QUESTS) {

                    if (_view_page_quest > 0 || _view_page_holodisk > 0) {

                        // Go to previous page for quests if available
                        if (_view_page_quest > 0) {
                            _view_page_quest--;
                        }

                        // Go to previous page for holodisks if available
                        if (_view_page_holodisk > 0) {
                            _view_page_holodisk--;
                        }
                        soundPlayFile("ib1p1xx1");

                        // Calculate how many quests are on the new page
                        int maxEntriesPerPage = PIPBOY_STATUS_QUEST_LINES;
                        int startIndex = _view_page_quest * maxEntriesPerPage;
                        int endIndex = startIndex + maxEntriesPerPage;
                        if (endIndex > gPipboyQuestLocationsCount) {
                            endIndex = gPipboyQuestLocationsCount;
                        }

                        int questsOnNewPage = endIndex - startIndex;

                        // Set quest selection to last item on the new page (for left arrow)
                        gPipboySelectedQuestIndex = questsOnNewPage - 1;
                        if (gPipboySelectedQuestIndex < 0) gPipboySelectedQuestIndex = 0;
                        gPipboySelectedIndex = gPipboySelectedQuestIndex;

                        // rebuild entire page
                        pipboyWindowHandleStatus(1024);
                    } else {
                        // first page, do nothing
                    }
                }
            }
        }
        return;
    }

    // Handle Enter/select (PIPBOY_KEY_SELECT)
    if (userInput == PIPBOY_KEY_SELECT) {
        if (_stat_flag == 0 && _holo_flag == 0) {
            // Main status page
            if (gPipboyKeyboardMode) {
                if (gPipboyCurrentColumn == PIPBOY_COLUMN_QUESTS) {
                    // Selected a quest location
                    if (gPipboyWindowQuestsCurrentPageCount > 0 && gPipboySelectedQuestIndex < gPipboyWindowQuestsCurrentPageCount) {

                        soundPlayFile("ib1p1xx1");

                        // Convert page-relative index (0-based) to global index (1-based)
                        int globalIndex = (_view_page_quest * PIPBOY_STATUS_QUEST_LINES) + gPipboySelectedQuestIndex + 1;

                        // Clear and highlight the selected quest
                        blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
                            PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
                            PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
                            PIPBOY_WINDOW_WIDTH,
                            gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
                            PIPBOY_WINDOW_WIDTH);

                        pipboyWindowRenderQuestLocationList(globalIndex);
                        windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                        inputPauseForTocks(200);

                        // Enter quest list view
                        _stat_flag = 1;
                        gPipboyKeyboardMode = false; // Disable keyboard mode for quest list
                        pipboyWindowQuestList(globalIndex);
                    }
                } else if (gPipboyCurrentColumn == PIPBOY_COLUMN_HOLODISKS) {
                    // Selected a holodisk
                    if (gPipboyWindowHolodisksCount > 0 && gPipboySelectedHolodiskIndex < gPipboyWindowHolodisksCount) {

                        soundPlayFile("ib1p1xx1");

                        // gPipboySelectedHolodiskIndex is now 0-based within current holodisk page
                        int holodiskIndex = gPipboySelectedHolodiskIndex;

                        // Calculate the real index in the holodisk list (considering pagination)
                        int realIndex = (_view_page_holodisk * PIPBOY_STATUS_HOLODISK_LINES) + holodiskIndex;

                        // Find the actual holodisk in the global list
                        _holodisk = 0;
                        int foundIndex = 0;
                        for (; foundIndex < gHolodisksCount; foundIndex++) {
                            HolodiskDescription* holodiskDescription = &(gHolodiskDescriptions[foundIndex]);
                            if (gGameGlobalVars[holodiskDescription->gvar] > 0) {
                                if (_holodisk == realIndex) {
                                    break;
                                }
                                _holodisk++;
                            }
                        }
                        _holodisk = foundIndex;

                        blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
                            PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
                            PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
                            PIPBOY_WINDOW_WIDTH,
                            gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
                            PIPBOY_WINDOW_WIDTH);

                        // Highlight the selected holodisk (convert to 1-based for rendering)
                        pipboyWindowRenderHolodiskList(holodiskIndex + 1);
                        windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                        inputPauseForTocks(200);

                        // Enter holodisk view
                        pipboyWindowDestroyButtons();
                        pipboyRenderHolodiskText();
                        _holo_flag = 1;
                        gPipboyKeyboardMode = false; // Disable keyboard mode for holodisk view
                    }
                }
            }
        }
        return;
    }

    if (_stat_flag == 0 && _holo_flag == 0) { // handles bottom (more/back) navigation of main status page

        if (userInput == 1025 || userInput <= -1) {
            if (userInput < 1025 || userInput > 1027) {
                return;
            }
            // Ensure navigation stays within valid page range
            if ((_view_page_quest <= 0 && gPipboyMouseX < 459) || (_view_page_quest >= totalPages - 1 && gPipboyMouseX >= 459)) {
                return; // Prevent navigation if already at min/max page (and click)
            }

            // Destroy old buttons before changing pages
            pipboyWindowDestroyButtons();
            soundPlayFile("ib1p1xx1");
            handlePipboyPageNavigation( // handle changing status pages
                gPipboyMouseX,
                459, &_view_page_quest,
                totalPages,
                pipboyWindowHandleStatus,
                []() {
                    pipboyWindowHandleStatus(1024);
                    windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                });

            // Destroy old buttons before changing pages
            pipboyWindowDestroyButtons();
            handlePipboyPageNavigation( // handle changing holodisk pages
                gPipboyMouseX,
                459, &_view_page_holodisk,
                totalPages,
                pipboyWindowHandleStatus,
                []() {
                    pipboyWindowCreateButtons(2, gPipboyQuestLocationsCount + gPipboyWindowHolodisksCount + 1, true); // Ensure new buttons match the new page
                    pipboyWindowHandleStatus(1024);
                    windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                });
        }

        if (gPipboyQuestLocationsCount != 0 && gPipboyWindowQuestsCurrentPageCount >= userInput && gPipboyMouseX < 429) {
            soundPlayFile("ib1p1xx1");
            blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
                PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
                PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
                PIPBOY_WINDOW_WIDTH,
                gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
                PIPBOY_WINDOW_WIDTH);
            pipboyWindowRenderQuestLocationList(userInput); // Keep for location choice highlighting
            windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            inputPauseForTocks(200);
            _stat_flag = 1;
        } else {
            if (gPipboyWindowHolodisksCount != 0 && gPipboyWindowHolodisksCount >= userInput && gPipboyMouseX > 429) {
                soundPlayFile("ib1p1xx1");
                _holodisk = 0;
                int realIndex = (_view_page_holodisk * PIPBOY_STATUS_HOLODISK_LINES) + (userInput); // Adjust userInput for pagination
                int index = 0;
                for (; index < gHolodisksCount; index += 1) {
                    HolodiskDescription* holodiskDescription = &(gHolodiskDescriptions[index]);
                    if (gGameGlobalVars[holodiskDescription->gvar] > 0) {
                        if (realIndex - 1 == _holodisk) { // use realIndex instead of userInput
                            break;
                        }
                        _holodisk += 1;
                    }
                }
                _holodisk = index;

                blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
                    PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
                    PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
                    PIPBOY_WINDOW_WIDTH,
                    gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
                    PIPBOY_WINDOW_WIDTH);

                pipboyWindowRenderHolodiskList(userInput); // Keep for holodisk choice highlighting
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                inputPauseForTocks(200);
                pipboyWindowDestroyButtons();
                pipboyRenderHolodiskText();
                _holo_flag = 1;
            }
        }
    }

    if (_stat_flag == 0) { // holodisk handling
        if (_holo_flag == 0 || userInput < 1025 || userInput > 1027) {
            return;
        }

        if (gPipboyMouseX > 395 && gPipboyMouseX < 459 && gPipboyHolodiskLastPage != 0) { // prevents click between back/more buttons, except on one page holodisks
            return;
        }

        // Destroy old buttons before changing pages
        pipboyWindowDestroyButtons();
        soundPlayFile("ib1p1xx1");
        handlePipboyPageNavigation( // new holodisk navigation handling
            gPipboyMouseX,
            459, &_view_page,
            gPipboyHolodiskLastPage + 1,
            pipboyWindowHandleStatus,
            []() {
                pipboyRenderHolodiskText();
                pipboyWindowCreateButtons(0, 0, true); // Ensure new buttons match the new page
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            });

    } else {

        realIndex = (_view_page_quest * PIPBOY_STATUS_QUEST_LINES) + (userInput); // Adjust for pagination

        // Clicking the bottom nav
        if (userInput == 1025) {

            if (gPipboyMouseX > 395 && gPipboyMouseX < 459 && totalPages > 1) { // prevents click between back/more buttons, except on one page holodisks
                return;
            }

            soundPlayFile("ib1p1xx1");
            handlePipboyPageNavigation( // handle quest list bottom navigation
                gPipboyMouseX,
                459, &_view_page_questlist,
                totalPages,
                pipboyWindowHandleStatus,
                []() {
                    // pass -1 to render last chosen location (but now updated with _view_pages_questlist)
                    pipboyWindowQuestList(-1);
                    windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                });
        }

        // Clicking a quest location
        if (userInput <= gPipboyQuestLocationsCount) {
            pipboyWindowQuestList(realIndex);
        }
    }
}

static void pipboyWindowQuestList(int selectedLocationIndex)
{
    // Declare and initialize local variables
    static int lastLocation = -1; // Store the last location passed, initialized to -1
    const int maxEntriesPerPage = PIPBOY_STATUS_QUESTLIST_LINES;

    int displayedQuests = 0;
    int totalQuests = 0;
    int number = 1 + (_view_page_questlist * maxEntriesPerPage);

    // If -1 is passed, use the last location
    if (selectedLocationIndex == -1 && lastLocation != -1) {
        selectedLocationIndex = lastLocation; // Re-use the last location
    } else if (selectedLocationIndex != -1) {
        lastLocation = selectedLocationIndex; // Update the last location with the new one
    }

    // NEW: Find the target location ID for the selected location index
    int targetLocationId = -1;
    int locationCount = 0;

    // Build the same location list as in pipboyWindowRenderQuestLocationList
    // to find which location ID corresponds to selectedLocationIndex
    for (int i = 0; i < gQuestsCount; i++) {
        QuestDescription* quest = &(gQuestDescriptions[i]);

        // Skip empty quests
        if (quest->location == 0) {
            continue;
        }

        // Skip if quest doesn't meet display threshold
        if (quest->displayThreshold > gGameGlobalVars[quest->gvar]) {
            continue;
        }

        // Check if this is a new location (not already counted)
        bool newLocation = true;
        for (int j = 0; j < i; j++) {
            QuestDescription* prevQuest = &(gQuestDescriptions[j]);
            if (prevQuest->location != 0 && prevQuest->displayThreshold <= gGameGlobalVars[prevQuest->gvar] && prevQuest->location == quest->location) {
                newLocation = false;
                break;
            }
        }

        if (newLocation) {
            locationCount++;

            // Is this the selected location?
            if (locationCount == selectedLocationIndex) {
                targetLocationId = quest->location;
                break;
            }
        }
    }

    if (targetLocationId == -1) {
        // Fallback: Show no quests if location not found
        pipboyWindowDestroyButtons();
        windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
        return;
    }

    // Clear previous buttons
    pipboyWindowDestroyButtons();

    // Redraw the window background
    blitBufferToBuffer(
        _pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
        PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
        PIPBOY_WINDOW_WIDTH,
        gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_WIDTH);

    // Initialize the current line for pipboy display
    if (gPipboyLinesCount >= 0) {
        gPipboyCurrentLine = 0;
    }

    if (gPipboyLinesCount >= 1) {
        gPipboyCurrentLine = 1;
    }

    // For quest list page, disable keyboard mode (no selectable items)
    gPipboyKeyboardMode = false;

    // Display the location header
    const char* text1 = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 210);
    const char* text2 = getmsg(&gMapMessageList, &gPipboyMessageListItem, targetLocationId);
    char formattedText[1024];
    snprintf(formattedText, sizeof(formattedText), "%s %s", text2, text1);
    pipboyDrawText(formattedText, PIPBOY_TEXT_STYLE_UNDERLINE, _colorTable[992]);

    if (gPipboyLinesCount >= 3) {
        gPipboyCurrentLine = 3;
    }

    // Pagination logic
    int startIndex = _view_page_questlist * maxEntriesPerPage;
    int endIndex = startIndex + maxEntriesPerPage;

    // NEW: Count total quests for this location (for pagination)
    for (int i = 0; i < gQuestsCount; i++) {
        QuestDescription* quest = &(gQuestDescriptions[i]);

        // Skip empty quests
        if (quest->location == 0 || quest->description == 0) {
            continue;
        }

        // Skip quests not in target location
        if (quest->location != targetLocationId) {
            continue;
        }

        // Skip if quest doesn't meet display threshold
        if (gGameGlobalVars[quest->gvar] < quest->displayThreshold) {
            continue;
        }

        totalQuests++;
    }

    // Display quests for the target location
    int questsDisplayed = 0;

    for (int i = 0; i < gQuestsCount; i++) {
        QuestDescription* questDescription = &(gQuestDescriptions[i]);

        // Skip empty quests
        if (questDescription->location == 0 || questDescription->description == 0) {
            continue;
        }

        // Skip quests not in target location
        if (questDescription->location != targetLocationId) {
            continue;
        }

        // Skip if quest doesn't meet display threshold
        if (gGameGlobalVars[questDescription->gvar] < questDescription->displayThreshold) {
            continue;
        }

        // Pagination: Skip quests before startIndex
        if (questsDisplayed < startIndex) {
            questsDisplayed++;
            continue;
        }

        // Stop if we've reached the page limit
        if (questsDisplayed >= endIndex) {
            break;
        }

        // Display the quest
        const char* text = getmsg(&gQuestsMessageList, &gPipboyMessageListItem, questDescription->description);
        char formattedText[1024];
        snprintf(formattedText, sizeof(formattedText), "%d. %s", number, text);
        number++;

        short beginnings[WORD_WRAP_MAX_COUNT];
        short count;
        if (wordWrap(formattedText, 350, beginnings, &count) == 0) {
            for (int line = 0; line < count - 1; line++) {
                char* beginning = formattedText + beginnings[line];
                char* ending = formattedText + beginnings[line + 1];
                char c = *ending;
                *ending = '\0';

                int flags;
                int color;
                if (gGameGlobalVars[questDescription->gvar] < questDescription->completedThreshold) {
                    flags = 0;
                    color = _colorTable[992];
                } else {
                    flags = PIPBOY_TEXT_STYLE_STRIKE_THROUGH;
                    color = _colorTable[8804];
                }

                pipboyDrawText(beginning, flags, color);
                *ending = c;
                gPipboyCurrentLine++;
            }
        } else {
            debugPrint("\n ** Word wrap error in pipboy! **\n");
        }

        questsDisplayed++;
    }

    // Calculate total pages for pagination
    totalPages = (totalQuests + maxEntriesPerPage - 1) / maxEntriesPerPage;

    // Render navigation buttons for pagination
    renderPagination(_view_page_questlist, totalPages);

    // Call for back/more button navigation
    renderNavigationButtons(_view_page_questlist, totalPages, true);

    // Refresh the window after the updates
    windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);

    // Create buttons for the bottom navigation
    pipboyWindowCreateButtons(0, 0, true);
}

// 0x498734
static void pipboyWindowRenderQuestLocationList(int selectedQuestLocation)
{
    if (gPipboyLinesCount >= 0) {
        gPipboyCurrentLine = 0;
    }

    int flags = gPipboyWindowHolodisksCount != 0 ? PIPBOY_TEXT_ALIGNMENT_LEFT_COLUMN_CENTER : PIPBOY_TEXT_ALIGNMENT_CENTER;
    flags |= PIPBOY_TEXT_STYLE_UNDERLINE;

    // STATUS
    const char* statusText = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 202);
    pipboyDrawText(statusText, flags, _colorTable[992]);

    if (gPipboyLinesCount >= 2) {
        gPipboyCurrentLine = 2;
    }

    gPipboyQuestLocationsCount = 0;
    const char* gPipboyQuestLocations[100]; // Fix: Use const char* to match getmsg() return type

    gQuestsCount = MOD_QUEST_MAX;
    // Build the list of quest locations
    int addedLocationIds[100]; // Can hold up to 100 unique locations
    int addedCount = 0;

    // Build the list of quest locations
    for (int index = 0; index < gQuestsCount; index++) {
        QuestDescription* quest = &(gQuestDescriptions[index]);

        // Skip empty quests
        if (quest->location == 0) {
            continue;
        }

        // Skip if quest doesn't meet display threshold
        if (quest->displayThreshold > gGameGlobalVars[quest->gvar]) {
            continue;
        }

        // Check if we've already added this location
        bool duplicate = false;
        for (int j = 0; j < addedCount; j++) {
            if (addedLocationIds[j] == quest->location) {
                duplicate = true;
                break;
            }
        }

        if (duplicate) {
            continue;
        }

        const char* questLocation = getmsg(&gMapMessageList, &gPipboyMessageListItem, quest->location);
        if (questLocation != NULL) {
            gPipboyQuestLocations[gPipboyQuestLocationsCount] = questLocation;
            addedLocationIds[addedCount] = quest->location;
            gPipboyQuestLocationsCount += 1;
            addedCount += 1;
        }
    }

    // Pagination logic
    int maxEntriesPerPage = PIPBOY_STATUS_QUEST_LINES;
    totalPages = (gPipboyQuestLocationsCount + maxEntriesPerPage - 1) / maxEntriesPerPage;
    int startIndex = _view_page_quest * maxEntriesPerPage;
    int endIndex = startIndex + maxEntriesPerPage;

    // Ensure that we don't go out of bounds
    if (startIndex >= gPipboyQuestLocationsCount) {
        startIndex = gPipboyQuestLocationsCount - 1;
    }
    if (endIndex > gPipboyQuestLocationsCount) {
        endIndex = gPipboyQuestLocationsCount;
    }

    gPipboyWindowQuestsCurrentPageCount = endIndex - startIndex;

    // Update max selectable items for keyboard navigation
    if (_stat_flag == 0 && _holo_flag == 0) {
        gPipboyMaxSelectableItems = gPipboyWindowQuestsCurrentPageCount;

        // Adjust selection if it's out of bounds
        if (gPipboySelectedIndex >= gPipboyMaxSelectableItems) {
            gPipboySelectedIndex = gPipboyMaxSelectableItems - 1;
        }
    }

    // Render the current page
    for (int index = startIndex; index < endIndex; index++) {
        int pageRelativeIndex = index - startIndex;

        // Determine if this item is keyboard-selected
        bool isKeyboardSelected = (gPipboyKeyboardMode && gPipboyCurrentColumn == PIPBOY_COLUMN_QUESTS && pageRelativeIndex == gPipboySelectedQuestIndex);
        bool isMouseSelected = ((gPipboyCurrentLine - 1) / 2 == (selectedQuestLocation - 1));

        int color = pipboyGetSelectionColor(isKeyboardSelected, isMouseSelected);

        const char* questLocation = gPipboyQuestLocations[index];
        pipboyDrawText(questLocation, 0, color);
        gPipboyCurrentLine += 1;
    }

    // Call to display pagination indicator
    renderPagination(_view_page_quest, totalPages);

    // Call for back/more button navigation
    renderNavigationButtons(_view_page_quest, totalPages, false);
}

// 0x4988A0
static void pipboyRenderHolodiskText()
{
    blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
        PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
        PIPBOY_WINDOW_WIDTH,
        gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_WIDTH);

    if (gPipboyLinesCount >= 0) {
        gPipboyCurrentLine = 0;
    }

    // For holodisk content, disable keyboard mode (no selectable items)
    gPipboyKeyboardMode = false;

    HolodiskDescription* holodisk = &(gHolodiskDescriptions[_holodisk]);

    int holodiskTextId;
    int linesCount = 0;

    gPipboyHolodiskLastPage = 0;

    for (holodiskTextId = holodisk->description; holodiskTextId < holodisk->description + 500; holodiskTextId += 1) {
        const char* text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, holodiskTextId);
        if (strcmp(text, "**END-DISK**") == 0) {
            break;
        }

        linesCount += 1;
        if (linesCount >= PIPBOY_HOLODISK_LINES_MAX) {
            linesCount = 0;
            gPipboyHolodiskLastPage += 1;
        }
    }

    if (holodiskTextId >= holodisk->description + 500) {
        debugPrint("\nPIPBOY: #1 Holodisk text end not found!\n");
    }

    holodiskTextId = holodisk->description;

    if (_view_page != 0) {
        int page = 0;
        int numberOfLines = 0;
        for (; holodiskTextId < holodiskTextId + 500; holodiskTextId += 1) {
            const char* line = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, holodiskTextId);
            if (strcmp(line, "**END-DISK**") == 0) {
                debugPrint("\nPIPBOY: Premature page end in holodisk page search!\n");
                break;
            }

            numberOfLines += 1;
            if (numberOfLines >= PIPBOY_HOLODISK_LINES_MAX) {
                page += 1;
                if (page >= _view_page) {
                    break;
                }

                numberOfLines = 0;
            }
        }

        holodiskTextId += 1;

        if (holodiskTextId >= holodisk->description + 500) {
            debugPrint("\nPIPBOY: #2 Holodisk text end not found!\n");
        }
    } else {
        const char* name = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, holodisk->name);
        pipboyDrawText(name, PIPBOY_TEXT_ALIGNMENT_CENTER | PIPBOY_TEXT_STYLE_UNDERLINE, _colorTable[992]);
    }

    if (gPipboyHolodiskLastPage != 0) {
        renderPagination(_view_page, gPipboyHolodiskLastPage + 1); // pagination indicator for holodisks
    }

    if (gPipboyLinesCount >= 3) {
        gPipboyCurrentLine = 3;
    }

    for (int line = 0; line < PIPBOY_HOLODISK_LINES_MAX; line += 1) {
        const char* text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, holodiskTextId);
        if (strcmp(text, "**END-DISK**") == 0) {
            break;
        }

        if (strcmp(text, "**END-PAR**") == 0) {
            gPipboyCurrentLine += 1;
        } else {
            pipboyDrawText(text, PIPBOY_TEXT_NO_INDENT, _colorTable[992]);
        }

        holodiskTextId += 1;
    }

    // Call for back/more button navigation
    renderNavigationButtons(_view_page, gPipboyHolodiskLastPage + 1, true);

    windowRefresh(gPipboyWindow);
}

// 0x498C40
static int pipboyWindowRenderHolodiskList(int selectedHolodiskEntry)
{
    if (gPipboyLinesCount >= 2) {
        gPipboyCurrentLine = 2;
    }

    const int maxEntriesPerPage = PIPBOY_STATUS_HOLODISK_LINES;
    int knownHolodisksCount = 0;

    // Count total holodisks
    for (int index = 0; index < gHolodisksCount; index++) {
        if (gGameGlobalVars[gHolodiskDescriptions[index].gvar] != 0) {
            knownHolodisksCount++;
        }
    }

    // Calculate pagination
    int totalPages = (knownHolodisksCount + maxEntriesPerPage - 1) / maxEntriesPerPage;
    if (_view_page_holodisk >= totalPages)
        _view_page_holodisk = totalPages - 1;
    if (_view_page_holodisk < 0)
        _view_page_holodisk = 0;

    int startIdx = _view_page_holodisk * maxEntriesPerPage;
    int currentIndex = 0;

    // Render paginated holodisks
    int displayedHolodisks = 0;
    for (int index = 0; index < gHolodisksCount; index++) {
        HolodiskDescription* holodisk = &(gHolodiskDescriptions[index]);
        if (gGameGlobalVars[holodisk->gvar] == 0)
            continue;

        if (currentIndex >= startIdx && currentIndex < startIdx + maxEntriesPerPage) {
            int pageRelativeIndex = currentIndex - startIdx;

            // Check if this is keyboard selected
            bool isKeyboardSelected = (gPipboyKeyboardMode && gPipboyCurrentColumn == PIPBOY_COLUMN_HOLODISKS && pageRelativeIndex == gPipboySelectedHolodiskIndex);

            bool isMouseSelected = ((gPipboyCurrentLine - 1) / 2 == (selectedHolodiskEntry - 1));

            int color = pipboyGetSelectionColor(isKeyboardSelected, isMouseSelected);

            const char* text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, holodisk->name);
            pipboyDrawText(text, PIPBOY_TEXT_ALIGNMENT_RIGHT_COLUMN, color);
            gPipboyCurrentLine++;
            displayedHolodisks++;
        }
        currentIndex++;
    }

    if (displayedHolodisks > 0) {
        if (gPipboyLinesCount >= 0) {
            gPipboyCurrentLine = 0;
        }
        const char* text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 211); // DATA
        pipboyDrawText(text, PIPBOY_TEXT_ALIGNMENT_RIGHT_COLUMN_CENTER | PIPBOY_TEXT_STYLE_UNDERLINE, _colorTable[992]);
    }

    return displayedHolodisks;
}

// 0x498D34
static int _qscmp(const void* a1,
    const void* a2)
{
    STRUCT_664350* v1 = (STRUCT_664350*)a1;
    STRUCT_664350* v2 = (STRUCT_664350*)a2;

    return strcmp(v1->name, v2->name);
}

// 0x498D40
static void pipboyWindowHandleAutomaps(int userInput)
{
    if (userInput == 1024) { // 1024 'resets' the page (kindof)
        // Reset selection for automaps
        gPipboySelectedIndex = 0;
        gPipboyKeyboardMode = false;
        pipboyWindowDestroyButtons();
        blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
            PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
            PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
            PIPBOY_WINDOW_WIDTH,
            gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
            PIPBOY_WINDOW_WIDTH);

        const char* title = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 205);
        pipboyDrawText(title, PIPBOY_TEXT_ALIGNMENT_CENTER | PIPBOY_TEXT_STYLE_UNDERLINE, _colorTable[992]);

        // Reset city index when returning to main list
        _amcty_indx = -1;

        // Initialize keyboard selection before drawing
        gPipboySelectedIndex = 0;
        gPipboyKeyboardMode = true;

        _location_count = _PrintAMList(-1); // get main page list count

        pipboyWindowCreateButtons(2, _location_count, true); // create buttons for main page
        main_sub_mode = 0; // Set mode for handling navigation (main or sub navigation)

        gPipboyMaxSelectableItems = _location_count; // Update max items

        windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
        _view_page_automap_sub = 0; // reset subpages start at page 1(0)

        return;
    }

    if (userInput == PIPBOY_KEY_LEFT) { // Left arrow
        userInput = 1027; // Map to Page Up (previous page)
    } else if (userInput == PIPBOY_KEY_RIGHT) { // Right arrow
        userInput = 1026; // Map to Page Down (next page)
    }

    // Handle left/right arrow keys for page navigation
    if (userInput == 1026) { // Right arrow (next page)
        soundPlayFile("ib1p1xx1");

        if (main_sub_mode == 0) { // Main page
            if (_view_page_automap_main < totalPages - 1) {
                _view_page_automap_main++;

                gPipboySelectedIndex = 0;
                gPipboyKeyboardMode = true;

                pipboyWindowDestroyButtons();
                _location_count = _PrintAMList(-1);
                pipboyWindowCreateButtons(2, _location_count, true);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            }
        } else if (main_sub_mode == 1) { // Sub-page
            if (_view_page_automap_sub < totalPages - 1) {
                _view_page_automap_sub++;

                gPipboySelectedIndex = 0;
                gPipboyKeyboardMode = true;

                soundPlayFile("ib1p1xx1");
                pipboyWindowDestroyButtons();
                _map_count = _PrintAMelevList(-1);
                pipboyWindowCreateButtons(0, _map_count + 2, true);
                automapRenderInPipboyWindow(gPipboyWindow, _sortlist[0].field_6, _sortlist[0].field_4);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            } else {
                // On last page, right arrow goes back to main automap list
                soundPlayFile("ib1p1xx1");
                pipboyWindowHandleAutomaps(1024);
            }
        }
        return;
    } else if (userInput == 1027) { // Left arrow (previous page)
        soundPlayFile("ib1p1xx1");

        if (main_sub_mode == 0) { // Main page
            if (_view_page_automap_main > 0) {
                _view_page_automap_main--;

                gPipboySelectedIndex = 0;
                gPipboyKeyboardMode = true;

                pipboyWindowDestroyButtons();
                _location_count = _PrintAMList(-1);
                pipboyWindowCreateButtons(2, _location_count, true);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            }
        } else if (main_sub_mode == 1) { // Sub-page
            if (_view_page_automap_sub > 0) {
                _view_page_automap_sub--;

                gPipboySelectedIndex = 0;
                gPipboyKeyboardMode = true;

                soundPlayFile("ib1p1xx1");
                pipboyWindowDestroyButtons();
                _map_count = _PrintAMelevList(-1);
                pipboyWindowCreateButtons(0, _map_count + 2, true);
                automapRenderInPipboyWindow(gPipboyWindow, _sortlist[0].field_6, _sortlist[0].field_4);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            } else {
                // On first page, left arrow goes back to main automap list
                soundPlayFile("ib1p1xx1");
                pipboyWindowHandleAutomaps(1024);
            }
        }
        return;
    }

    // Handle up arrow (PIPBOY_KEY_UP)
    if (userInput == PIPBOY_KEY_UP) {
        if (main_sub_mode == 0) {
            // Main page up arrow
            if (gPipboySelectedIndex > 0) {
                gPipboySelectedIndex--;
                gPipboyKeyboardMode = true;
                _PrintAMList(-1);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            } else if (gPipboySelectedIndex == 0 && _view_page_automap_main > 0) {
                // Wrap to previous page
                soundPlayFile("ib1p1xx1");
                _view_page_automap_main--;
                gPipboySelectedIndex = (_view_page_automap_main + 1) * PIPBOY_AUTOMAP_LINES - 1;
                gPipboyKeyboardMode = true;
                pipboyWindowDestroyButtons();
                _location_count = _PrintAMList(-1);
                pipboyWindowCreateButtons(2, _location_count, true);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            }
        } else if (main_sub_mode == 1) {
            // Sub-page up arrow
            if (gPipboySelectedIndex > 0) {
                gPipboySelectedIndex--;
                gPipboyKeyboardMode = true;
                _PrintAMelevList(-1); // Changed from 1 to -1
                automapRenderInPipboyWindow(gPipboyWindow,
                    _sortlist[gPipboySelectedIndex].field_6,
                    _sortlist[gPipboySelectedIndex].field_4);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            } else if (gPipboySelectedIndex == 0 && _view_page_automap_sub > 0) {
                // Wrap to previous page
                _view_page_automap_sub--;
                gPipboySelectedIndex = (_view_page_automap_main + 1) * PIPBOY_AUTOMAP_LINES - 1;
                gPipboyKeyboardMode = true;
                soundPlayFile("ib1p1xx1");
                pipboyWindowDestroyButtons();
                _map_count = _PrintAMelevList(-1); // Changed from two calls to one
                automapRenderInPipboyWindow(gPipboyWindow,
                    _sortlist[gPipboySelectedIndex].field_6,
                    _sortlist[gPipboySelectedIndex].field_4);
                pipboyWindowCreateButtons(0, _map_count + 2, true);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            }
        }
        return;
    }

    // Handle down arrow (PIPBOY_KEY_DOWN)
    if (userInput == PIPBOY_KEY_DOWN) {
        if (main_sub_mode == 0) {
            // Main page down arrow
            int maxItems = _location_count;
            if (gPipboySelectedIndex < maxItems - 1) {
                gPipboySelectedIndex++;
                gPipboyKeyboardMode = true;
                _PrintAMList(-1);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            } else if (gPipboySelectedIndex == maxItems - 1 && _view_page_automap_main < totalPages - 1) {
                // Wrap to next page
                soundPlayFile("ib1p1xx1");
                _view_page_automap_main++;
                gPipboySelectedIndex = 0;
                gPipboyKeyboardMode = true;
                pipboyWindowDestroyButtons();
                _location_count = _PrintAMList(-1);
                pipboyWindowCreateButtons(2, _location_count, true);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            }
        } else if (main_sub_mode == 1) {
            // Sub-page down arrow
            int maxItems = _map_count;
            if (gPipboySelectedIndex < maxItems - 1) {
                gPipboySelectedIndex++;
                gPipboyKeyboardMode = true;
                _PrintAMelevList(-1); // Changed from 1 to -1
                automapRenderInPipboyWindow(gPipboyWindow,
                    _sortlist[gPipboySelectedIndex].field_6,
                    _sortlist[gPipboySelectedIndex].field_4);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            } else if (gPipboySelectedIndex == maxItems - 1 && _view_page_automap_sub < totalPages - 1) {
                // Wrap to next page
                _view_page_automap_sub++;
                gPipboySelectedIndex = 0;
                gPipboyKeyboardMode = true;
                soundPlayFile("ib1p1xx1");
                pipboyWindowDestroyButtons();
                _map_count = _PrintAMelevList(-1); // Changed from two calls to one
                automapRenderInPipboyWindow(gPipboyWindow,
                    _sortlist[gPipboySelectedIndex].field_6,
                    _sortlist[gPipboySelectedIndex].field_4);
                pipboyWindowCreateButtons(0, _map_count + 2, true);
                windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            }
        }
        return;
    }

    // Handle Enter (PIPBOY_KEY_SELECT) - select highlighted item
    if (userInput == PIPBOY_KEY_SELECT) {
        if (gPipboyKeyboardMode && gPipboySelectedIndex >= 0) {
            // Convert selected index to button index (1-based)
            int buttonIndex = gPipboySelectedIndex + 1;

            if (main_sub_mode == 0) {
                // Select location on main page
                if (buttonIndex <= _location_count) {
                    soundPlayFile("ib1p1xx1");
                    pipboyWindowDestroyButtons();
                    int realIndex = (_view_page_automap_main * PIPBOY_AUTOMAP_LINES) + (buttonIndex - 1);
                    _amcty_indx = _sortlist[realIndex].field_4;
                    gPipboySelectedIndex = 0;
                    gPipboyKeyboardMode = true;
                    _map_count = _PrintAMelevList(-1); // Changed from 1 to -1
                    pipboyWindowCreateButtons(0, _map_count + 2, true);
                    automapRenderInPipboyWindow(gPipboyWindow, _sortlist[0].field_6, _sortlist[0].field_4);
                    gPipboyMaxSelectableItems = _map_count;
                    windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                    main_sub_mode = 1;
                }
            } else if (main_sub_mode == 1) {
                // Select elevation on sub-page
                if (buttonIndex <= _map_count) {
                    soundPlayFile("ib1p1xx1");
                    gPipboyKeyboardMode = false; // Mouse-like selection, so turn off keyboard mode
                    _PrintAMelevList(buttonIndex);
                    automapRenderInPipboyWindow(gPipboyWindow,
                        _sortlist[buttonIndex - 1].field_6,
                        _sortlist[buttonIndex - 1].field_4);
                    windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                }
            }
        }
        return;
    }

    if (main_sub_mode == 0) { // main page mode, bottom buttons
        if (userInput == 1025 || userInput <= -1) {
            if (userInput < 1025 || userInput > 1027) {
                return;
            }
            // Ensure navigation stays within valid page range
            if ((_view_page_automap_main <= 0 && gPipboyMouseX < 459) || (_view_page_automap_main >= totalPages - 1 && gPipboyMouseX >= 459)) {
                return; // Prevent navigation if already at min/max page
            }

            // Destroy old buttons before changing pages
            pipboyWindowDestroyButtons();
            soundPlayFile("ib1p1xx1");
            handlePipboyPageNavigation(
                gPipboyMouseX,
                459, &_view_page_automap_main,
                totalPages,
                pipboyWindowHandleAutomaps,
                []() {
                    _location_count = _PrintAMList(-1);
                    pipboyWindowCreateButtons(2, _location_count, true); // Ensure new buttons match the new page
                    windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                });
        }

        if (userInput > 0 && userInput <= _location_count && main_sub_mode == 0) {
            soundPlayFile("ib1p1xx1");
            pipboyWindowDestroyButtons();
            int realIndex = (_view_page_automap_main * PIPBOY_AUTOMAP_LINES) + (userInput - 1);

            // Store the clicked city index in a local variable
            int clickedCityIndex = _sortlist[realIndex].field_4;

            // Set the global city index to the clicked city
            _amcty_indx = clickedCityIndex;

            // Reset keyboard state for new sub-page
            gPipboySelectedIndex = 0;
            gPipboyKeyboardMode = true;

            _map_count = _PrintAMelevList(-1);
            pipboyWindowCreateButtons(0, _map_count + 2, true);

            // Use the clicked city to render the map
            // Find which index in _sortlist corresponds to the clicked city
            int foundIndex = -1;
            for (int i = 0; i < _map_count; i++) {
                if (_sortlist[i].field_4 == clickedCityIndex) {
                    foundIndex = i;
                    break;
                }
            }

            // Default to first item if not found
            int displayIndex = findCityIndexInSortlist(clickedCityIndex, 0); // Default to elevation 0

            automapRenderInPipboyWindow(gPipboyWindow,
                _sortlist[displayIndex].field_6,
                _sortlist[displayIndex].field_4);

            windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            main_sub_mode = 1;
        }
    } else if (main_sub_mode == 1) { // subpage mode, bottom and subloction buttons
        if (userInput == 1025 || userInput <= -1) {
            if (userInput < 1025 || userInput > 1027) {
                return;
            }
            soundPlayFile("ib1p1xx1");
            handlePipboyPageNavigation(
                gPipboyMouseX,
                459, &_view_page_automap_sub,
                totalPages,
                pipboyWindowHandleAutomaps,
                []() {
                    pipboyWindowDestroyButtons();
                    _PrintAMelevList(1);
                    _map_count = _PrintAMelevList(1);
                    pipboyWindowCreateButtons(0, _map_count + 2, true); // create buttons for sub-locations (elevation), and back/more
                    automapRenderInPipboyWindow(gPipboyWindow, _sortlist[0].field_6, _sortlist[0].field_4);
                    windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
                });
        }

        if (userInput >= 1 && userInput <= _map_count + 3) {
            soundPlayFile("ib1p1xx1");
            gPipboyKeyboardMode = false; // mouse click turns off keyboard mode
            _PrintAMelevList(userInput);
            automapRenderInPipboyWindow(gPipboyWindow, _sortlist[userInput - 1].field_6, _sortlist[userInput - 1].field_4);
            windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
        }

        return;
    }
}

// 0x498F30
static int _PrintAMelevList(int selectedMap)
{
    AutomapHeader* automapHeader;
    if (automapGetHeader(&automapHeader) == -1) {
        return -1;
    }

    int totalEntries = 0;
    int elevationsListSize = 0;
    const int maxEntriesPerPage = PIPBOY_AUTOMAP_SUB_LINES;

    // First pass: Count total valid entries with bounds checking
    if (_amcty_indx >= 0 && _amcty_indx < AUTOMAP_MAP_COUNT) {
        for (int elevation = 0; elevation < ELEVATION_COUNT; elevation++) {
            if (automapHeader->offsets[_amcty_indx][elevation] > 0) {
                totalEntries++;
            }
        }
    }

    int mapCount = wmMapMaxCount();
    // Cap mapCount to AUTOMAP_MAP_COUNT
    if (mapCount > AUTOMAP_MAP_COUNT) {
        mapCount = AUTOMAP_MAP_COUNT;
    }

    for (int map = 0; map < mapCount; map++) {
        if (map == _amcty_indx || _get_map_idx_same(_amcty_indx, map) == -1) {
            continue;
        }

        // Bounds check
        if (map < 0 || map >= AUTOMAP_MAP_COUNT) {
            continue;
        }

        for (int elevation = 0; elevation < ELEVATION_COUNT; elevation++) {
            if (automapHeader->offsets[map][elevation] > 0) {
                totalEntries++;
            }
        }
    }

    totalPages = (totalEntries + maxEntriesPerPage - 1) / maxEntriesPerPage;
    if (_view_page_automap_sub < 0) {
        _view_page_automap_sub = 0;
    } else if (_view_page_automap_sub >= totalPages) {
        _view_page_automap_sub = totalPages - 1;
    }

    int startIndex = _view_page_automap_sub * maxEntriesPerPage;
    int endIndex = startIndex + maxEntriesPerPage;
    int currentIndex = 0;

    // Second pass: Build the list for the selected page
    for (int elevation = 0; elevation < ELEVATION_COUNT && elevationsListSize < maxEntriesPerPage; elevation++) {
        if (_amcty_indx >= 0 && _amcty_indx < AUTOMAP_MAP_COUNT && automapHeader->offsets[_amcty_indx][elevation] > 0) {
            if (currentIndex >= startIndex && currentIndex < endIndex) {
                _sortlist[elevationsListSize].name = mapGetName(_amcty_indx, elevation);
                _sortlist[elevationsListSize].field_4 = elevation;
                _sortlist[elevationsListSize].field_6 = _amcty_indx;
                elevationsListSize++;

                // Safety check
                if (elevationsListSize >= 2000) {
                    break;
                }
            }
            currentIndex++;
        }
    }

    for (int map = 0; map < mapCount && elevationsListSize < maxEntriesPerPage; map++) {
        if (map == _amcty_indx || _get_map_idx_same(_amcty_indx, map) == -1) {
            continue;
        }

        for (int elevation = 0; elevation < ELEVATION_COUNT && elevationsListSize < maxEntriesPerPage; elevation++) {
            if (automapHeader->offsets[map][elevation] > 0) {
                if (currentIndex >= startIndex && currentIndex < endIndex) {
                    _sortlist[elevationsListSize].name = mapGetName(map, elevation);
                    _sortlist[elevationsListSize].field_4 = elevation;
                    _sortlist[elevationsListSize].field_6 = map;
                    elevationsListSize++;
                }
                currentIndex++;
            }
        }
    }

    // Update the UI
    blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
        PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
        PIPBOY_WINDOW_WIDTH,
        gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_WIDTH);

    if (gPipboyLinesCount >= 0) {
        gPipboyCurrentLine = 0;
    }

    const char* msg = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 205);
    pipboyDrawText(msg, PIPBOY_TEXT_ALIGNMENT_CENTER | PIPBOY_TEXT_STYLE_UNDERLINE, _colorTable[992]);

    if (gPipboyLinesCount >= 2) {
        gPipboyCurrentLine = 2;
    }

    const char* name = _map_get_description_idx_(_amcty_indx);
    pipboyDrawText(name, PIPBOY_TEXT_ALIGNMENT_CENTER, _colorTable[992]);

    if (gPipboyLinesCount >= 4) {
        gPipboyCurrentLine = 4;
    }

    // Set max selectable items for keyboard navigation
    gPipboyMaxSelectableItems = elevationsListSize;

    // Adjust keyboard selection if out of bounds for this page
    if (gPipboySelectedIndex >= gPipboyMaxSelectableItems) {
        gPipboySelectedIndex = gPipboyMaxSelectableItems - 1;
        if (gPipboySelectedIndex < 0) {
            gPipboySelectedIndex = 0;
        }
    }

    int selectedPipboyLine = (selectedMap - 1) * 2;

    for (int index = 0; index < elevationsListSize; index++) {
        int color;

        // Check for keyboard selection first
        if (gPipboyKeyboardMode && index == gPipboySelectedIndex) {
            color = _colorTable[32747]; // Bright green for keyboard selection
        }
        // Then check for mouse selection
        else if (gPipboyCurrentLine - 4 == selectedPipboyLine) {
            color = _colorTable[32747]; // Bright green for mouse selection
        } else {
            color = _colorTable[992]; // Normal color
        }

        pipboyDrawText(_sortlist[index].name, 0, color);
        gPipboyCurrentLine++;
    }

    // Pagination display (Top-right)
    renderPagination(_view_page_automap_sub, totalPages);

    // Call for back/more button navigation
    renderNavigationButtons(_view_page_automap_sub, totalPages, true);

    return elevationsListSize;
}

// 0x499150
static int _PrintAMList(int selectedLocation)
{

    // don't process invalid indices
    if (_amcty_indx < -1 || _amcty_indx >= AUTOMAP_MAP_COUNT) {
        _amcty_indx = -1;
    }
    AutomapHeader* automapHeader;
    if (automapGetHeader(&automapHeader) == -1) {
        return -1;
    }

    int count = 0;

    int mapCount = wmMapMaxCount();
    // Don't exceed AUTOMAP_MAP_COUNT
    if (mapCount > AUTOMAP_MAP_COUNT) {
        mapCount = AUTOMAP_MAP_COUNT;
    }

    for (int map = 0; map < mapCount; map++) {

        char* cityName = mapGetCityName(map);
        // Skip if cityName is null OR starts with "ERROR!"
        if (!cityName || (cityName && strncmp(cityName, "ERROR!", 6) == 0)) {
            continue;
        }
        // Bounds check for displayMapList
        if (map < 0 || map >= AUTOMAP_MAP_COUNT) {
            continue;
        }

        int elevation;
        for (elevation = 0; elevation < ELEVATION_COUNT; elevation++) {
            // Bounds check for offsets
            if (map < AUTOMAP_MAP_COUNT && elevation < ELEVATION_COUNT && automapHeader->offsets[map][elevation] > 0) {
                if (_automapDisplayMap(map) == 0) {
                    break;
                }
            }
        }

        if (elevation < ELEVATION_COUNT) {
            int locationExistsIndex = 0;
            if (count != 0) {
                for (int index = 0; index < count; index++) {
                    if (_is_map_idx_same(map, _sortlist[index].field_4)) {
                        break;
                    }
                    locationExistsIndex++;
                }
            }

            if (locationExistsIndex == count) {
                _sortlist[count].name = mapGetCityName(map);
                _sortlist[count].field_4 = map;
                count++;

                // Safety check - don't exceed sortlist bounds
                if (count >= 2000) { // Adjust based on actual _sortlist size
                    break;
                }
            }
        }
    }

    if (count == 0) {
        return 0;
    }

    // Sort list alphabetically
    if (count > 1) {
        qsort(_sortlist, count, sizeof(*_sortlist), _qscmp);
    }

    // Pagination calculations
    const int locationsPerPage = PIPBOY_AUTOMAP_LINES;
    totalPages = (count + locationsPerPage - 1) / locationsPerPage;

    // Ensure _view_page_automap_main is within valid range
    if (_view_page_automap_main >= totalPages) {
        _view_page_automap_main = totalPages - 1;
    } else if (_view_page_automap_main < 0) {
        _view_page_automap_main = 0;
    }

    // Clear content area
    blitBufferToBuffer(
        _pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
        PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
        PIPBOY_WINDOW_WIDTH,
        gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_WIDTH);

    // Pagination display (Top-right)
    renderPagination(_view_page_automap_main, totalPages);

    // Reset line count
    if (gPipboyLinesCount >= 0) {
        gPipboyCurrentLine = 0;
    }

    // Display header message
    const char* msg = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 205);
    pipboyDrawText(msg, PIPBOY_TEXT_ALIGNMENT_CENTER | PIPBOY_TEXT_STYLE_UNDERLINE, _colorTable[992]);

    if (gPipboyLinesCount >= 2) {
        gPipboyCurrentLine = 2;
    }

    // Calculate start and end index for pagination
    int startIdx = _view_page_automap_main * locationsPerPage;
    int endIdx = startIdx + locationsPerPage;
    if (endIdx > count) {
        endIdx = count;
    }

    // Print paginated locations with keyboard selection
    for (int index = startIdx; index < endIdx; index++) {
        int pageRelativeIndex = index - startIdx;

        int color;
        if (gPipboyKeyboardMode && pageRelativeIndex == gPipboySelectedIndex) {
            color = _colorTable[32747]; // Keyboard selected
        } else if ((gPipboyCurrentLine - 1) == selectedLocation) {
            color = _colorTable[32747]; // Mouse selected (existing)
        } else {
            color = _colorTable[992];
        }

        pipboyDrawText(_sortlist[index].name, 0, color);
        gPipboyCurrentLine++;
    }

    // Call for back/more button navigation
    renderNavigationButtons(_view_page_automap_main, totalPages, false);

    return endIdx - startIdx; // Return the number of entries on the current page
}

// 0x49932C
static void pipboyHandleVideoArchive(int a1)
{
    if (a1 == 1024) {
        pipboyWindowDestroyButtons();
        _view_page = pipboyRenderVideoArchive(-1);
        pipboyWindowCreateButtons(2, _view_page, false);
    } else if (a1 >= 0 && a1 <= _view_page) {
        soundPlayFile("ib1p1xx1");

        pipboyRenderVideoArchive(a1);

        int movie;
        for (movie = 2; movie < 16; movie++) {
            if (gameMovieIsSeen(movie)) {
                a1--;
                if (a1 <= 0) {
                    break;
                }
            }
        }

        if (movie <= MOVIE_COUNT) {
            gameMoviePlay(movie, GAME_MOVIE_FADE_IN | GAME_MOVIE_FADE_OUT | GAME_MOVIE_PAUSE_MUSIC);
        } else {
            debugPrint("\n ** Selected movie not found in list! **\n");
        }

        fontSetCurrent(101);

        gPipboyLastEventTimestamp = getTicks();
        pipboyRenderVideoArchive(-1);
    }
}

// 0x4993DC
static int pipboyRenderVideoArchive(int a1)
{
    const char* text;
    int i;
    int v12;
    int msg_num;
    int v5;
    int v8;
    int v9;

    blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
        PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
        PIPBOY_WINDOW_WIDTH,
        gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_WIDTH);

    if (gPipboyLinesCount >= 0) {
        gPipboyCurrentLine = 0;
    }

    // VIDEO ARCHIVES
    text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 206);
    pipboyDrawText(text, PIPBOY_TEXT_ALIGNMENT_CENTER | PIPBOY_TEXT_STYLE_UNDERLINE, _colorTable[992]);

    if (gPipboyLinesCount >= 2) {
        gPipboyCurrentLine = 2;
    }

    v5 = 0;
    v12 = a1 - 1;

    // 502 - Elder Speech
    // ...
    // 516 - Credits
    msg_num = 502;

    for (i = 2; i < 16; i++) {
        if (gameMovieIsSeen(i)) {
            v8 = v5++;
            if (v8 == v12) {
                v9 = _colorTable[32747];
            } else {
                v9 = _colorTable[992];
            }

            text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, msg_num);
            pipboyDrawText(text, 0, v9);

            gPipboyCurrentLine++;
        }

        msg_num++;
    }

    windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);

    return v5;
}

// 0x499518
static void pipboyHandleAlarmClock(int eventCode)
{
    if (eventCode == 1024) {
        if (_critter_can_obj_dude_rest()) {
            pipboyWindowDestroyButtons();
            pipboyWindowRenderRestOptions(0);
            pipboyWindowCreateButtons(5, gPipboyRestOptionsCount, false);
        } else {
            soundPlayFile("iisxxxx1");

            // You cannot rest at this location!
            const char* text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 215);
            showDialogBox(text, nullptr, 0, 192, 135, _colorTable[32328], nullptr, _colorTable[32328], DIALOG_BOX_LARGE);

            // CE: Restore previous tab to make sure clicks are processed by
            // appropriate handler (not the alarm clock).
            gPipboyTab = gPipboyPrevTab;
        }
    } else if (eventCode >= 4 && eventCode <= 17) {
        soundPlayFile("ib1p1xx1");

        pipboyWindowRenderRestOptions(eventCode - 3);

        int duration = eventCode - 4;
        int minutes = 0;
        int hours = 0;

        switch (duration) {
        case PIPBOY_REST_DURATION_TEN_MINUTES:
            pipboyRest(0, 10, 0);
            break;
        case PIPBOY_REST_DURATION_THIRTY_MINUTES:
            pipboyRest(0, 30, 0);
            break;
        case PIPBOY_REST_DURATION_ONE_HOUR:
        case PIPBOY_REST_DURATION_TWO_HOURS:
        case PIPBOY_REST_DURATION_THREE_HOURS:
        case PIPBOY_REST_DURATION_FOUR_HOURS:
        case PIPBOY_REST_DURATION_FIVE_HOURS:
        case PIPBOY_REST_DURATION_SIX_HOURS:
            pipboyRest(duration - 1, 0, 0);
            break;
        case PIPBOY_REST_DURATION_UNTIL_MORNING:
            _ClacTime(&hours, &minutes, 8);
            pipboyRest(hours, minutes, 0);
            break;
        case PIPBOY_REST_DURATION_UNTIL_NOON:
            _ClacTime(&hours, &minutes, 12);
            pipboyRest(hours, minutes, 0);
            break;
        case PIPBOY_REST_DURATION_UNTIL_EVENING:
            _ClacTime(&hours, &minutes, 18);
            pipboyRest(hours, minutes, 0);
            break;
        case PIPBOY_REST_DURATION_UNTIL_MIDNIGHT:
            _ClacTime(&hours, &minutes, 0);
            if (pipboyRest(hours, minutes, 0) == 0) {
                pipboyDrawNumber(0, 4, PIPBOY_WINDOW_TIME_X, PIPBOY_WINDOW_TIME_Y);
            }
            windowRefresh(gPipboyWindow);
            break;
        case PIPBOY_REST_DURATION_UNTIL_HEALED:
        case PIPBOY_REST_DURATION_UNTIL_PARTY_HEALED:
            pipboyRest(0, 0, duration);
            break;
        }

        soundPlayFile("ib2lu1x1");

        pipboyWindowRenderRestOptions(0);
    }
}

// 0x4996B4
static void pipboyWindowRenderRestOptions(int a1)
{
    const char* text;

    blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
        PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
        PIPBOY_WINDOW_WIDTH,
        gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_WIDTH);

    if (gPipboyLinesCount >= 0) {
        gPipboyCurrentLine = 0;
    }

    // ALARM CLOCK
    text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 300);
    pipboyDrawText(text, PIPBOY_TEXT_ALIGNMENT_CENTER | PIPBOY_TEXT_STYLE_UNDERLINE, _colorTable[992]);

    if (gPipboyLinesCount >= 5) {
        gPipboyCurrentLine = 5;
    }

    pipboyDrawHitPoints();

    // NOTE: I don't know if this +1 was a result of compiler optimization or it
    // was written like this in the first place.
    for (int option = 1; option < gPipboyRestOptionsCount + 1; option++) {
        // 302 - Rest for ten minutes
        // ...
        // 315 - Rest until party is healed
        text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 302 + option - 1);
        int color = option == a1 ? _colorTable[32747] : _colorTable[992];

        pipboyDrawText(text, 0, color);

        gPipboyCurrentLine++;
    }

    windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
}

// 0x4997B8
static void pipboyDrawHitPoints()
{
    int max_hp;
    int cur_hp;
    char* text;
    char msg[64];
    int len;

    blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + 66 * PIPBOY_WINDOW_WIDTH + 254,
        350,
        10,
        PIPBOY_WINDOW_WIDTH,
        gPipboyWindowBuffer + 66 * PIPBOY_WINDOW_WIDTH + 254,
        PIPBOY_WINDOW_WIDTH);

    max_hp = critterGetStat(gDude, STAT_MAXIMUM_HIT_POINTS);
    cur_hp = critterGetHitPoints(gDude);
    text = getmsg(&gPipboyMessageList, &gPipboyMessageListItem, 301); // Hit Points
    snprintf(msg, sizeof(msg), "%s %d/%d", text, cur_hp, max_hp);
    len = fontGetStringWidth(msg);
    fontDrawText(gPipboyWindowBuffer + 66 * PIPBOY_WINDOW_WIDTH + 254 + (350 - len) / 2, msg, PIPBOY_WINDOW_WIDTH, PIPBOY_WINDOW_WIDTH, _colorTable[992]);
}

// 0x4998C0
static void pipboyWindowCreateButtons(int start, int count, bool a3)
{
    fontSetCurrent(101);

    int height = fontGetLineHeight();

    gPipboyWindowButtonStart = start;
    gPipboyWindowButtonCount = count;

    if (count != 0) {
        int y = start * height + PIPBOY_WINDOW_CONTENT_VIEW_Y;
        int eventCode = start + 505;
        for (int index = start; index < 22; index++) {
            if (gPipboyWindowButtonCount + gPipboyWindowButtonStart <= index) {
                break;
            }

            _HotLines[index] = buttonCreate(gPipboyWindow, 254, y, 350, height, -1, -1, -1, eventCode, nullptr, nullptr, nullptr, BUTTON_FLAG_TRANSPARENT);
            y += height * 2;
            eventCode += 1;
        }
    }

    if (a3) {
        _button = buttonCreate(gPipboyWindow, 254, height * gPipboyLinesCount + PIPBOY_WINDOW_CONTENT_VIEW_Y, 350, height, -1, -1, -1, 528, nullptr, nullptr, nullptr, BUTTON_FLAG_TRANSPARENT);
    }
}

// 0x4999C0
static void pipboyWindowDestroyButtons()
{
    if (gPipboyWindowButtonCount != 0) {
        // NOTE: There is a buffer overread bug. In original binary it leads to
        // reading continuation (from 0x6644B8 onwards), which finally destroys
        // button in `gPipboyWindow` (id #3), which corresponds to Skilldex
        // button. Other buttons can be destroyed depending on the last mouse
        // position. I was not able to replicate this exact behaviour with MSVC.
        // So here is a small fix, which is an exception to "Do not fix vanilla
        // bugs" strategy.
        int end = gPipboyWindowButtonStart + gPipboyWindowButtonCount;
        if (end > 22) {
            end = 22;
        }

        for (int index = gPipboyWindowButtonStart; index < end; index++) {
            buttonDestroy(_HotLines[index]);
        }
    }

    if (_hot_back_line) {
        buttonDestroy(_button);
    }

    gPipboyWindowButtonCount = 0;
    _hot_back_line = 0;
}

// 0x499A24
static bool pipboyRest(int hours, int minutes, int duration)
{
    gameMouseSetCursor(MOUSE_CURSOR_WAIT_WATCH);

    bool rc = false;

    if (duration == 0) {
        int hoursInMinutes = hours * 60;
        double v1 = (double)hoursInMinutes + (double)minutes;
        double v2 = v1 * (1.0 / 1440.0) * 3.5 + 0.25;
        double v3 = (double)minutes / v1 * v2;
        if (minutes != 0) {
            unsigned int gameTime = gameTimeGetTime();

            double v4 = v3 * 20.0;
            int v5 = 0;
            for (int v5 = 0; v5 < (int)v4; v5++) {
                sharedFpsLimiter.mark();

                if (rc) {
                    break;
                }

                unsigned int start = getTicks();

                unsigned int v6 = (unsigned int)((double)v5 / v4 * ((double)minutes * 600.0) + (double)gameTime);
                unsigned int nextEventTime = queueGetNextEventTime();
                if (v6 >= nextEventTime) {
                    gameTimeSetTime(nextEventTime + 1);
                    if (queueProcessEvents()) {
                        rc = true;
                        debugPrint("PIPBOY: Returning from Queue trigger...\n");
                        _proc_bail_flag = 1;
                        break;
                    }

                    if (_game_user_wants_to_quit != 0) {
                        rc = true;
                    }
                }

                if (!rc) {
                    gameTimeSetTime(v6);
                    if (inputGetInput() == KEY_ESCAPE || _game_user_wants_to_quit != 0) {
                        rc = true;
                    }

                    pipboyDrawNumber(gameTimeGetHour(), 4, PIPBOY_WINDOW_TIME_X, PIPBOY_WINDOW_TIME_Y);
                    pipboyDrawDate();
                    windowRefresh(gPipboyWindow);

                    delay_ms(50 - (getTicks() - start));
                }

                renderPresent();
                sharedFpsLimiter.throttle();
            }

            if (!rc) {
                gameTimeSetTime(gameTime + 600 * minutes);

                if (_Check4Health(minutes)) {
                    // NOTE: Uninline.
                    _AddHealth();
                }
            }

            pipboyDrawNumber(gameTimeGetHour(), 4, PIPBOY_WINDOW_TIME_X, PIPBOY_WINDOW_TIME_Y);
            pipboyDrawDate();
            pipboyDrawHitPoints();
            windowRefresh(gPipboyWindow);
        }

        if (hours != 0 && !rc) {
            unsigned int gameTime = gameTimeGetTime();
            double v7 = (v2 - v3) * 20.0;

            for (int hour = 0; hour < (int)v7; hour++) {
                sharedFpsLimiter.mark();

                if (rc) {
                    break;
                }

                unsigned int start = getTicks();

                if (inputGetInput() == KEY_ESCAPE || _game_user_wants_to_quit != 0) {
                    rc = true;
                }

                unsigned int v8 = (unsigned int)((double)hour / v7 * (hours * GAME_TIME_TICKS_PER_HOUR) + gameTime);
                unsigned int nextEventTime = queueGetNextEventTime();
                if (!rc && v8 >= nextEventTime) {
                    gameTimeSetTime(nextEventTime + 1);

                    if (queueProcessEvents()) {
                        rc = true;
                        debugPrint("PIPBOY: Returning from Queue trigger...\n");
                        _proc_bail_flag = 1;
                        break;
                    }

                    if (_game_user_wants_to_quit != 0) {
                        rc = true;
                    }
                }

                if (!rc) {
                    gameTimeSetTime(v8);

                    int healthToAdd = (int)((double)hoursInMinutes / v7);
                    if (_Check4Health(healthToAdd)) {
                        // NOTE: Uninline.
                        _AddHealth();
                    }

                    pipboyDrawNumber(gameTimeGetHour(), 4, PIPBOY_WINDOW_TIME_X, PIPBOY_WINDOW_TIME_Y);
                    pipboyDrawDate();
                    pipboyDrawHitPoints();
                    windowRefresh(gPipboyWindow);

                    delay_ms(50 - (getTicks() - start));
                }

                renderPresent();
                sharedFpsLimiter.throttle();
            }

            if (!rc) {
                gameTimeSetTime(gameTime + GAME_TIME_TICKS_PER_HOUR * hours);
            }

            pipboyDrawNumber(gameTimeGetHour(), 4, PIPBOY_WINDOW_TIME_X, PIPBOY_WINDOW_TIME_Y);
            pipboyDrawDate();
            pipboyDrawHitPoints();
            windowRefresh(gPipboyWindow);
        }
    } else if (duration == PIPBOY_REST_DURATION_UNTIL_HEALED || duration == PIPBOY_REST_DURATION_UNTIL_PARTY_HEALED) {
        int currentHp = critterGetHitPoints(gDude);
        int maxHp = critterGetStat(gDude, STAT_MAXIMUM_HIT_POINTS);
        if (currentHp != maxHp
            || (duration == PIPBOY_REST_DURATION_UNTIL_PARTY_HEALED && partyIsAnyoneCanBeHealedByRest())) {
            // First pass - healing dude is the top priority.
            int hpToHeal = maxHp - currentHp;
            int healingRate = critterGetStat(gDude, STAT_HEALING_RATE);
            int hoursToHeal = (int)((double)hpToHeal / (double)healingRate * 3.0);
            while (!rc && hoursToHeal != 0) {
                if (hoursToHeal <= 24) {
                    rc = pipboyRest(hoursToHeal, 0, 0);
                    hoursToHeal = 0;
                } else {
                    rc = pipboyRest(24, 0, 0);
                    hoursToHeal -= 24;
                }
            }

            // Second pass - attempt to heal delayed damage to dude (via poison
            // or radiation), and remaining party members. This process is
            // performed in 3 hour increments.
            currentHp = critterGetHitPoints(gDude);
            maxHp = critterGetStat(gDude, STAT_MAXIMUM_HIT_POINTS);
            hpToHeal = maxHp - currentHp;

            if (duration == PIPBOY_REST_DURATION_UNTIL_PARTY_HEALED) {
                int partyHpToHeal = partyGetMaxWoundToHealByRest();
                if (partyHpToHeal > hpToHeal) {
                    hpToHeal = partyHpToHeal;
                }
            }

            while (!rc && hpToHeal != 0) {
                currentHp = critterGetHitPoints(gDude);
                maxHp = critterGetStat(gDude, STAT_MAXIMUM_HIT_POINTS);
                hpToHeal = maxHp - currentHp;

                if (duration == PIPBOY_REST_DURATION_UNTIL_PARTY_HEALED) {
                    int partyHpToHeal = partyGetMaxWoundToHealByRest();
                    if (partyHpToHeal > hpToHeal) {
                        hpToHeal = partyHpToHeal;
                    }
                }

                rc = pipboyRest(3, 0, 0);
            }
        } else {
            // No one needs healing.
            gameMouseSetCursor(MOUSE_CURSOR_ARROW);
            return rc;
        }
    }

    if (gameTimeGetTime() > queueGetNextEventTime()) {
        if (queueProcessEvents()) {
            debugPrint("PIPBOY: Returning from Queue trigger...\n");
            _proc_bail_flag = 1;
            rc = true;
        }
    }

    pipboyDrawNumber(gameTimeGetHour(), 4, PIPBOY_WINDOW_TIME_X, PIPBOY_WINDOW_TIME_Y);
    pipboyDrawDate();
    windowRefresh(gPipboyWindow);

    gameMouseSetCursor(MOUSE_CURSOR_ARROW);

    return rc;
}

// 0x499FCC
static bool _Check4Health(int minutes)
{
    _rest_time += minutes;

    if (_rest_time < 180) {
        return false;
    }

    debugPrint("\n health added!\n");
    _rest_time = 0;

    return true;
}

// NOTE: Inlined.
static bool _AddHealth()
{
    _partyMemberRestingHeal(3);

    int currentHp = critterGetHitPoints(gDude);
    int maxHp = critterGetStat(gDude, STAT_MAXIMUM_HIT_POINTS);
    return currentHp == maxHp;
}

// Returns [hours] and [minutes] needed to rest until [wakeUpHour].
static void _ClacTime(int* hours, int* minutes, int wakeUpHour)
{
    int gameTimeHour = gameTimeGetHour();

    *hours = gameTimeHour / 100;
    *minutes = gameTimeHour % 100;

    if (*hours != wakeUpHour || *minutes != 0) {
        *hours = wakeUpHour - *hours;
        if (*hours < 0) {
            *hours += 24;
            if (*minutes != 0) {
                *hours -= 1;
                *minutes = 60 - *minutes;
            }
        } else {
            if (*minutes != 0) {
                *hours -= 1;
                *minutes = 60 - *minutes;
                if (*hours < 0) {
                    *hours = 23;
                }
            }
        }
    } else {
        *hours = 24;
    }
}

// 0x49A0C8
static int pipboyRenderScreensaver()
{
    PipboyBomb bombs[PIPBOY_BOMB_COUNT];

    mouseGetPositionInWindow(gPipboyWindow, &gPipboyPreviousMouseX, &gPipboyPreviousMouseY);

    for (int index = 0; index < PIPBOY_BOMB_COUNT; index += 1) {
        bombs[index].field_10 = 0;
    }

    _gmouse_disable(0);

    unsigned char* buf = (unsigned char*)internal_malloc(412 * 374);
    if (buf == nullptr) {
        return -1;
    }

    blitBufferToBuffer(gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
        PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
        PIPBOY_WINDOW_WIDTH,
        buf,
        PIPBOY_WINDOW_CONTENT_VIEW_WIDTH);

    blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
        PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
        PIPBOY_WINDOW_WIDTH,
        gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_WIDTH);

    int v31 = 50;
    while (true) {
        sharedFpsLimiter.mark();

        unsigned int time = getTicks();

        mouseGetPositionInWindow(gPipboyWindow, &gPipboyMouseX, &gPipboyMouseY);

        // If mouse moves, exit keyboard mode
        if (gPipboyMouseX != gPipboyPreviousMouseX || gPipboyMouseY != gPipboyPreviousMouseY) {
            gPipboyKeyboardMode = false;
            gPipboyLastEventTimestamp = getTicks();
            gPipboyPreviousMouseX = gPipboyMouseX;
            gPipboyPreviousMouseY = gPipboyMouseY;
        }

        if (inputGetInput() != -1 || gPipboyPreviousMouseX != gPipboyMouseX || gPipboyPreviousMouseY != gPipboyMouseY) {
            break;
        }

        double random = randomBetween(0, PIPBOY_RAND_MAX);

        // TODO: Figure out what this constant means. Probably somehow related
        // to PIPBOY_RAND_MAX.
        if (random < 3047.3311) {
            int index = 0;
            for (; index < PIPBOY_BOMB_COUNT; index += 1) {
                if (bombs[index].field_10 == 0) {
                    break;
                }
            }

            if (index < PIPBOY_BOMB_COUNT) {
                PipboyBomb* bomb = &(bombs[index]);
                int v27 = (350 - _pipboyFrmImages[PIPBOY_FRM_BOMB].getWidth() / 4) + (406 - _pipboyFrmImages[PIPBOY_FRM_BOMB].getHeight() / 4);
                int v5 = (int)((double)randomBetween(0, PIPBOY_RAND_MAX) / (double)PIPBOY_RAND_MAX * (double)v27);
                int v6 = _pipboyFrmImages[PIPBOY_FRM_BOMB].getHeight() / 4;
                if (PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT - v6 >= v5) {
                    bomb->x = 602;
                    bomb->y = v5 + 48;
                } else {
                    bomb->x = v5 - (PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT - v6) + PIPBOY_WINDOW_CONTENT_VIEW_X + _pipboyFrmImages[PIPBOY_FRM_BOMB].getWidth() / 4;
                    bomb->y = PIPBOY_WINDOW_CONTENT_VIEW_Y - _pipboyFrmImages[PIPBOY_FRM_BOMB].getHeight() + 2;
                }

                bomb->field_10 = 1;
                bomb->field_8 = (float)((double)randomBetween(0, PIPBOY_RAND_MAX) * (2.75 / PIPBOY_RAND_MAX) + 0.15);
                bomb->field_C = 0;
            }
        }

        if (v31 == 0) {
            blitBufferToBuffer(_pipboyFrmImages[PIPBOY_FRM_BACKGROUND].getData() + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
                PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
                PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
                PIPBOY_WINDOW_WIDTH,
                gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
                PIPBOY_WINDOW_WIDTH);
        }

        for (int index = 0; index < PIPBOY_BOMB_COUNT; index++) {
            PipboyBomb* bomb = &(bombs[index]);
            if (bomb->field_10 != 1) {
                continue;
            }

            int srcWidth = _pipboyFrmImages[PIPBOY_FRM_BOMB].getWidth();
            int srcHeight = _pipboyFrmImages[PIPBOY_FRM_BOMB].getHeight();
            int destX = bomb->x;
            int destY = bomb->y;
            int srcY = 0;
            int srcX = 0;

            if (destX >= PIPBOY_WINDOW_CONTENT_VIEW_X) {
                if (destX + _pipboyFrmImages[PIPBOY_FRM_BOMB].getWidth() >= 604) {
                    srcWidth = 604 - destX;
                    if (srcWidth < 1) {
                        bomb->field_10 = 0;
                    }
                }
            } else {
                srcX = PIPBOY_WINDOW_CONTENT_VIEW_X - destX;
                if (srcX >= _pipboyFrmImages[PIPBOY_FRM_BOMB].getWidth()) {
                    bomb->field_10 = 0;
                }
                destX = PIPBOY_WINDOW_CONTENT_VIEW_X;
                srcWidth = _pipboyFrmImages[PIPBOY_FRM_BOMB].getWidth() - srcX;
            }

            if (destY >= PIPBOY_WINDOW_CONTENT_VIEW_Y) {
                if (destY + _pipboyFrmImages[PIPBOY_FRM_BOMB].getHeight() >= 452) {
                    srcHeight = 452 - destY;
                    if (srcHeight < 1) {
                        bomb->field_10 = 0;
                    }
                }
            } else {
                if (destY + _pipboyFrmImages[PIPBOY_FRM_BOMB].getHeight() < PIPBOY_WINDOW_CONTENT_VIEW_Y) {
                    bomb->field_10 = 0;
                }

                srcY = PIPBOY_WINDOW_CONTENT_VIEW_Y - destY;
                srcHeight = _pipboyFrmImages[PIPBOY_FRM_BOMB].getHeight() - srcY;
                destY = PIPBOY_WINDOW_CONTENT_VIEW_Y;
            }

            if (bomb->field_10 == 1 && v31 == 0) {
                blitBufferToBufferTrans(
                    _pipboyFrmImages[PIPBOY_FRM_BOMB].getData() + _pipboyFrmImages[PIPBOY_FRM_BOMB].getWidth() * srcY + srcX,
                    srcWidth,
                    srcHeight,
                    _pipboyFrmImages[PIPBOY_FRM_BOMB].getWidth(),
                    gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * destY + destX,
                    PIPBOY_WINDOW_WIDTH);
            }

            bomb->field_C += bomb->field_8;
            if (bomb->field_C >= 1.0) {
                bomb->x = (int)((float)bomb->x - bomb->field_C);
                bomb->y = (int)((float)bomb->y + bomb->field_C);
                bomb->field_C = 0.0;
            }
        }

        if (v31 != 0) {
            v31 -= 1;
        } else {
            windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
            delay_ms(50 - (getTicks() - time));
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    blitBufferToBuffer(buf,
        PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
        PIPBOY_WINDOW_CONTENT_VIEW_HEIGHT,
        PIPBOY_WINDOW_CONTENT_VIEW_WIDTH,
        gPipboyWindowBuffer + PIPBOY_WINDOW_WIDTH * PIPBOY_WINDOW_CONTENT_VIEW_Y + PIPBOY_WINDOW_CONTENT_VIEW_X,
        PIPBOY_WINDOW_WIDTH);

    internal_free(buf);

    windowRefreshRect(gPipboyWindow, &gPipboyWindowContentRect);
    _gmouse_enable();

    return 0;
}

// Generate debug quest_list.txt file
static void generateQuestListDebug()
{
    char debugPath[COMPAT_MAX_PATH];
    snprintf(debugPath, sizeof(debugPath), "./data%clists%cquests_list.txt", DIR_SEPARATOR, DIR_SEPARATOR);

    FILE* debugStream = compat_fopen(debugPath, "wt");
    if (debugStream == nullptr) {
        debugPrint("\ngenerateQuestListDebug: Could not create quests_list.txt");
        return;
    }

    // Write header
    const char* header = "==============================================================================\n"
                         "Fallout 2 Fission - Quest System Report\n"
                         "==============================================================================\n"
                         "This report shows all loaded quests - essential for mod debugging and\n"
                         "finding quest IDs for script development.\n\n"

                         "Key Features:\n"
                         "- Base quests: Protected in lower slots (0-199)\n"
                         "- Mod quests: Your content in remaining slots (200-999) via deterministic hashing\n"
                         "- Hash collisions trigger popup warnings and the quest is skipped\n\n"

                         "Usage Notes:\n"
                         "- Use these quest IDs when referencing quests in scripts:\n"
                         "  - op_set_quest(ID, state)  // Set quest state\n"
                         "  - op_get_quest(ID)         // Get quest state\n"
                         "- Quest IDs are STABLE between game sessions\n"
                         "- Mod quest positions use mod filename + quest index hash for consistency\n"
                         "==============================================================================\n\n";

    fputs(header, debugStream);

    // Write timestamp
    time_t now = time(0);
    struct tm* t = localtime(&now);
    fprintf(debugStream, "Report Generated: %04d-%02d-%02d %02d:%02d:%02d\n\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec);

    // Gather statistics
    int baseCount = 0, modCount = 0;
    int maxUsedIndex = 0;

    // First pass: count quests
    for (int i = 0; i < TOTAL_QUEST_MAX; i++) {
        if (i < BASE_QUEST_MAX) {
            // Check if this is a valid base quest (non-zero location indicates initialized)
            if (gQuestDescriptions[i].location != 0) {
                baseCount++;
                if (i > maxUsedIndex) maxUsedIndex = i;
            }
        } else {
            // Check if this is a valid mod quest
            if (gQuestModNames[i][0] != '\0') {
                modCount++;
                if (i > maxUsedIndex) maxUsedIndex = i;
            }
        }
    }

    // Summary section
    fprintf(debugStream,
        "Total Quests: %d | Base: %d | Mods: %d\n"
        "Array Size: %d entries (0-%d) | Max Used Index: %d\n",
        baseCount + modCount,
        baseCount,
        modCount,
        TOTAL_QUEST_MAX, TOTAL_QUEST_MAX - 1,
        maxUsedIndex);

    // Slot ranges
    fputs("------------------------------------------------------------\n", debugStream);
    fputs("Slot Ranges:\n", debugStream);
    fprintf(debugStream,
        "  Base: 0-%d\n"
        "  Mods: %d-%d\n",
        BASE_QUEST_MAX - 1,
        MOD_QUEST_START, MOD_QUEST_MAX - 1);
    fputs("------------------------------------------------------------\n", debugStream);

    // Base quests section
    if (baseCount > 0) {
        fputs("BASE QUESTS:\n", debugStream);
        for (int i = 0; i < BASE_QUEST_MAX; i++) {
            if (gQuestDescriptions[i].location != 0) {
                QuestDescription* quest = &gQuestDescriptions[i];
                fprintf(debugStream, "  %5d: Location %d, GVAR %d, DescMsg %d\n",
                    i,
                    quest->location,
                    quest->gvar,
                    quest->description);
            }
        }
        fputs("\n", debugStream);
    }

    // Mod quests section
    if (modCount > 0) {
        fputs("MOD QUESTS:\n", debugStream);
        for (int i = MOD_QUEST_START; i < TOTAL_QUEST_MAX; i++) {
            if (gQuestModNames[i][0] != '\0') {
                QuestDescription* quest = &gQuestDescriptions[i];

                // Find mod quest info for this slot
                ModQuestInfo* modQuest = nullptr;
                for (int j = 0; j < gModQuestCount; j++) {
                    if (gModQuests[j].questId == i) {
                        modQuest = &gModQuests[j];
                        break;
                    }
                }

                fprintf(debugStream, "  %5d: %s (Mod: %s)\n", i,
                    modQuest ? modQuest->questKey : "Unknown",
                    gQuestModNames[i]);
                fprintf(debugStream, "        Location: %d, GVAR: %d, DescMsg: %d\n",
                    quest->location,
                    quest->gvar,
                    quest->description);
            }
        }
        fputs("\n", debugStream);
    } else {
        fputs("MOD QUESTS:\n", debugStream);
        fputs("  (no mod quests found)\n\n", debugStream);
    }

    // Quest details section (only if we have mod quests)
    if (modCount > 0) {
        fputs("MOD QUEST DETAILS:\n", debugStream);
        fputs("-----------------\n", debugStream);

        for (int i = 0; i < gModQuestCount; i++) {
            ModQuestInfo* modQuest = &gModQuests[i];
            QuestDescription* quest = &gQuestDescriptions[modQuest->questId];

            fprintf(debugStream, "Quest %d: %s\n", modQuest->questId, modQuest->questKey);
            fprintf(debugStream, "  Mod: %s\n", modQuest->modName);
            fprintf(debugStream, "  Message ID: Desc=%d\n", modQuest->descMessageId);
            fprintf(debugStream, "  Game Data: Location=%d, GVAR=%d\n",
                quest->location, quest->gvar);
            fprintf(debugStream, "  Thresholds: Display=%d, Complete=%d\n\n",
                quest->displayThreshold, quest->completedThreshold);
        }
    }

    // Important notes footer
    fputs("=== IMPORTANT NOTES ===\n", debugStream);

    fputs("- Quest IDs are STABLE - they won't change between game sessions\n", debugStream);
    fputs("- Mod quest positions use mod filename + quest index hash for consistency\n", debugStream);
    fputs("- Hash collisions show popup warnings and skip the conflicting quest\n", debugStream);
    fputs("- Reference these exact numbers in your scripts with op_set_quest/op_get_quest\n", debugStream);
    fputs("- Use 'quests_*.txt' naming pattern for quest mods\n", debugStream);
    fputs("\nMESSAGE FILE FORMAT:\n", debugStream);
    fputs("data/text/english/game/messages_YourMod.txt:\n", debugStream);
    fputs("  [quests]\n", debugStream);
    fputs("  quest:0 = Quest description text\n", debugStream);

    fclose(debugStream);
    debugPrint("\ngenerateQuestListDebug: Generated quests_list.txt with %d base, %d mod quests", baseCount, modCount);
}

// Load a single mod quest file
static int questLoadModFile(const char* filename)
{
    File* stream = fileOpen(filename, "rt");
    if (!stream) {
        return -1;
    }

    // Extract mod name from filename (quests_xxx.txt -> xxx)
    const char* base_filename = strrchr(filename, DIR_SEPARATOR);
    if (!base_filename)
        base_filename = filename;
    else
        base_filename++;

    char mod_name[64] = { 0 };
    const char* prefix = "quests_";
    const char* suffix = ".txt";

    if (strncmp(base_filename, prefix, strlen(prefix)) == 0) {
        size_t filename_len = strlen(base_filename);
        size_t mod_name_len = filename_len - strlen(prefix) - strlen(suffix);

        if (mod_name_len > 0 && mod_name_len < sizeof(mod_name)) {
            strncpy(mod_name, base_filename + strlen(prefix), mod_name_len);
            mod_name[mod_name_len] = '\0';
        }
    }

    uint32_t modNamespace = questGetModNamespace(filename);

    int questsLoaded = 0;
    int questIndexInThisMod = 0;
    char line[256];

    // Read file line by line (following quests.txt format)
    while (fileReadString(line, sizeof(line) - 1, stream)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r' || line[0] == ';') {
            continue;
        }

        // Remove line endings
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        char* cr = strchr(line, '\r');
        if (cr) *cr = '\0';

        // Skip empty lines after trimming
        if (line[0] == '\0') {
            continue;
        }

        // Try to parse quest line
        int location, description, gvar, displayThreshold, completedThreshold;
        if (!parseQuestLine(line, &location, &description, &gvar, &displayThreshold, &completedThreshold)) {
            // Not a quest line, might be a section header or comment
            continue;
        }

        // Generate quest key from mod name and index
        char questKey[64];
        snprintf(questKey, sizeof(questKey), "%s:%d", mod_name, questIndexInThisMod);

        // Calculate consistent slot for this mod quest
        uint16_t targetSlot = questCalculateModSlot(questKey, modNamespace, questIndexInThisMod);

        // Check if the target slot is in the mod range and not already occupied by a different mod
        if (targetSlot < MOD_QUEST_START || targetSlot >= MOD_QUEST_MAX) {
            // This should not happen, but skip if it does
            questIndexInThisMod++;
            continue;
        }

        // Check for collision with a different mod
        if (gQuestModNames[targetSlot][0] != '\0' && strcmp(gQuestModNames[targetSlot], mod_name) != 0) {

            char errorMsg[512];
            snprintf(errorMsg, sizeof(errorMsg),
                "QUEST SLOT COLLISION DETECTED!\n\n"
                "Mod file: %s\n"
                "New quest key: %s\n"
                "Target slot: %d\n"
                "Existing mod: %s\n\n"
                "To resolve: Rename your mod file to change its namespace.",
                filename, questKey, targetSlot, gQuestModNames[targetSlot]);
            showMesageBox(errorMsg);

            questIndexInThisMod++;
            continue;
        }

        // Fill in the quest description for the target slot.
        QuestDescription* quest = &gQuestDescriptions[targetSlot];

        // Generate message ID for the quest description (we only need description)
        char descKey[256];
        snprintf(descKey, sizeof(descKey), "quest:%d", questIndexInThisMod); // Keep same format!

        int descMessageId = generate_mod_message_id(mod_name, descKey);

        // IMPORTANT: For mod quests, we need to override the description field
        // with our hashed message ID. This ensures getmsg() can find the mod quest text.
        // The vanilla description number is ignored for mod quests.
        quest->location = location;
        quest->description = descMessageId; // Use the generated message ID for the description
        quest->gvar = gvar;
        quest->displayThreshold = displayThreshold;
        quest->completedThreshold = completedThreshold;

        // Store mod name
        strncpy(gQuestModNames[targetSlot], mod_name, sizeof(gQuestModNames[targetSlot]) - 1);
        gQuestModNames[targetSlot][sizeof(gQuestModNames[targetSlot]) - 1] = '\0';

        // Store in mod quest tracking array
        if (gModQuestCount < (MOD_QUEST_MAX - MOD_QUEST_START)) {
            ModQuestInfo* modQuest = &gModQuests[gModQuestCount];
            modQuest->questId = targetSlot;
            strncpy(modQuest->modName, mod_name, sizeof(modQuest->modName) - 1);
            modQuest->modName[sizeof(modQuest->modName) - 1] = '\0';

            strncpy(modQuest->questKey, questKey, sizeof(modQuest->questKey) - 1);
            modQuest->questKey[sizeof(modQuest->questKey) - 1] = '\0';

            modQuest->descMessageId = descMessageId;

            gModQuestCount++;
        }

        questsLoaded++;
        gQuestsCount++;
        questIndexInThisMod++;
    }

    fileClose(stream);
    return questsLoaded;
}

// Load all mod quest files (quests_*.txt)
static void questLoadModFiles()
{
    char searchPattern[COMPAT_MAX_PATH];
    snprintf(searchPattern, sizeof(searchPattern), "data%cquests_*.txt", DIR_SEPARATOR);

    char** foundModFiles = nullptr;
    int modFileCount = fileNameListInit(searchPattern, &foundModFiles, 0, 0);

    if (modFileCount > 0) {
        for (int i = 0; i < modFileCount; i++) {
            char fullPath[COMPAT_MAX_PATH];
            snprintf(fullPath, sizeof(fullPath), "data%c%s", DIR_SEPARATOR, foundModFiles[i]);
            questLoadModFile(fullPath);
        }
        fileNameListFree(&foundModFiles, 0);
    }
}

// 0x49A5D4
static int questInit()
{
    // Initialize mod quest system
    memset(gModQuests, 0, sizeof(gModQuests));
    gModQuestCount = 0;
    memset(gQuestModNames, 0, sizeof(gQuestModNames));

    if (gQuestDescriptions != nullptr) {
        internal_free(gQuestDescriptions);
        gQuestDescriptions = nullptr;
    }

    gQuestsCount = 0;

    messageListFree(&gQuestsMessageList);

    if (!messageListInit(&gQuestsMessageList)) {
        return -1;
    }

    // Load base and mod quest messages
    // Changed from messageListLoad to messageListLoadWithMods
    if (!messageListLoadWithMods(&gQuestsMessageList, "game\\quests.msg", "QUESTS")) {
        return -1;
    }

    // Allocate quest descriptions for all possible quests (vanilla + mod)
    gQuestDescriptions = (QuestDescription*)internal_malloc(sizeof(QuestDescription) * TOTAL_QUEST_MAX);
    if (gQuestDescriptions == nullptr) {
        return -1;
    }

    // Initialize all quest descriptions to zero
    memset(gQuestDescriptions, 0, sizeof(QuestDescription) * TOTAL_QUEST_MAX);

    File* stream = fileOpen("data\\quests.txt", "rt");
    if (stream == nullptr) {
        return -1;
    }

    // Load vanilla quests (slots 0-199)
    char line[256];
    int currentQuestIndex = 0;

    while (fileReadString(line, sizeof(line) - 1, stream)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r' || line[0] == ';') {
            continue;
        }

        // Remove line endings
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        char* cr = strchr(line, '\r');
        if (cr) *cr = '\0';

        // Skip empty lines after trimming
        if (line[0] == '\0') {
            continue;
        }

        // Parse the quest line
        int location, description, gvar, displayThreshold, completedThreshold;
        if (!parseQuestLine(line, &location, &description, &gvar, &displayThreshold, &completedThreshold)) {
            // Not a valid quest line, skip
            continue;
        }

        // Check that we don't exceed the base quest limit
        if (currentQuestIndex >= BASE_QUEST_MAX) {
            debugPrint("\nWarning: Too many quests in base quests.txt, ignoring extra.");
            break;
        }

        // Fill in the quest description
        QuestDescription* quest = &gQuestDescriptions[currentQuestIndex];
        quest->location = location;
        quest->description = description;
        quest->gvar = gvar;
        quest->displayThreshold = displayThreshold;
        quest->completedThreshold = completedThreshold;

        currentQuestIndex++;
        gQuestsCount++;
    }

    fileClose(stream);

    // Load mod quests from quests_*.txt files
    questLoadModFiles();

    // DEBUG: Activate test quests automatically
    debugActivateTestQuests();

    // Generate debug report
    generateQuestListDebug();

    return 0;
}

// 0x49A7E4
static void questFree()
{
    if (gQuestDescriptions != nullptr) {
        internal_free(gQuestDescriptions);
        gQuestDescriptions = nullptr;
    }

    gQuestsCount = 0;

    messageListFree(&gQuestsMessageList);
}

// 0x49A818
static int questDescriptionCompare(const void* a1, const void* a2)
{
    QuestDescription* v1 = (QuestDescription*)a1;
    QuestDescription* v2 = (QuestDescription*)a2;
    return v1->location - v2->location;
}

// 0x49A824
static int holodiskInit()
{
    if (gHolodiskDescriptions != nullptr) {
        internal_free(gHolodiskDescriptions);
        gHolodiskDescriptions = nullptr;
    }

    gHolodisksCount = 0;

    File* stream = fileOpen("data\\holodisk.txt", "rt");
    if (stream == nullptr) {
        return -1;
    }

    char str[256];
    while (fileReadString(str, sizeof(str), stream)) {
        const char* delim = " \t,";
        char* tok;
        HolodiskDescription entry;

        char* ch = str;
        while (isspace(*ch)) {
            ch++;
        }

        if (*ch == '#') {
            continue;
        }

        tok = strtok(ch, delim);
        if (tok == nullptr) {
            continue;
        }

        entry.gvar = atoi(tok);

        tok = strtok(nullptr, delim);
        if (tok == nullptr) {
            continue;
        }

        entry.name = atoi(tok);

        tok = strtok(nullptr, delim);
        if (tok == nullptr) {
            continue;
        }

        entry.description = atoi(tok);

        HolodiskDescription* entries = (HolodiskDescription*)internal_realloc(gHolodiskDescriptions, sizeof(*gHolodiskDescriptions) * (gHolodisksCount + 1));
        if (entries == nullptr) {
            goto err;
        }

        memcpy(&(entries[gHolodisksCount]), &entry, sizeof(*gHolodiskDescriptions));

        gHolodiskDescriptions = entries;
        gHolodisksCount++;
    }

    fileClose(stream);

    return 0;

err:

    fileClose(stream);

    return -1;
}

// 0x49A968
static void holodiskFree()
{
    if (gHolodiskDescriptions != nullptr) {
        internal_free(gHolodiskDescriptions);
        gHolodiskDescriptions = nullptr;
    }

    gHolodisksCount = 0;
}

} // namespace fallout
