#include "inventory.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>

#include "actions.h"
#include "animation.h"
#include "art.h"
#include "color.h"
#include "combat.h"
#include "combat_ai.h"
#include "critter.h"
#include "dbox.h"
#include "debug.h"
#include "dialog.h"
#include "display_monitor.h"
#include "draw.h"
#include "game.h"
#include "game_config.h"
#include "game_dialog.h"
#include "game_mouse.h"
#include "game_sound.h"
#include "input.h"
#include "interface.h"
#include "item.h"
#include "kb.h"
#include "light.h"
#include "map.h"
#include "memory.h"
#include "message.h"
#include "mouse.h"
#include "object.h"
#include "party_member.h"
#include "perk.h"
#include "platform_compat.h"
#include "proto.h"
#include "proto_instance.h"
#include "random.h"
#include "reaction.h"
#include "scripts.h"
#include "settings.h"
#include "sfall_config.h"
#include "skill.h"
#include "stat.h"
#include "svga.h"
#include "text_font.h"
#include "tile.h"
#include "window_manager.h"

namespace fallout {

#define INVENTORY_WINDOW_X 80
#define INVENTORY_WINDOW_Y 0

#define INVENTORY_TRADE_WINDOW_X 80
#define INVENTORY_TRADE_WINDOW_Y 290
#define INVENTORY_TRADE_WINDOW_WIDTH 480
#define INVENTORY_TRADE_WINDOW_HEIGHT 180

#define INVENTORY_LARGE_SLOT_WIDTH 90
#define INVENTORY_LARGE_SLOT_HEIGHT 61

#define INVENTORY_SLOT_WIDTH 64

// Still used for inner Trade Inventories (non-screens)
#define INVENTORY_SLOT_HEIGHT 48

#define INVENTORY_SLOT_PADDING 4

#define INVENTORY_LEFT_HAND_SLOT_X 152
#define INVENTORY_LEFT_HAND_SLOT_Y 284
#define INVENTORY_LEFT_HAND_SLOT_MAX_X (INVENTORY_LEFT_HAND_SLOT_X + INVENTORY_LARGE_SLOT_WIDTH)
#define INVENTORY_LEFT_HAND_SLOT_MAX_Y (INVENTORY_LEFT_HAND_SLOT_Y + INVENTORY_LARGE_SLOT_HEIGHT)

#define INVENTORY_RIGHT_HAND_SLOT_X 245
#define INVENTORY_RIGHT_HAND_SLOT_Y 284
#define INVENTORY_RIGHT_HAND_SLOT_MAX_X (INVENTORY_RIGHT_HAND_SLOT_X + INVENTORY_LARGE_SLOT_WIDTH)
#define INVENTORY_RIGHT_HAND_SLOT_MAX_Y (INVENTORY_RIGHT_HAND_SLOT_Y + INVENTORY_LARGE_SLOT_HEIGHT)

#define INVENTORY_ARMOR_SLOT_X 152
#define INVENTORY_ARMOR_SLOT_Y 181
#define INVENTORY_ARMOR_SLOT_MAX_X (INVENTORY_ARMOR_SLOT_X + INVENTORY_LARGE_SLOT_WIDTH)
#define INVENTORY_ARMOR_SLOT_MAX_Y (INVENTORY_ARMOR_SLOT_Y + INVENTORY_LARGE_SLOT_HEIGHT)

#define INVENTORY_TRADE_SCROLLER_Y 30
#define INVENTORY_TRADE_INNER_SCROLLER_Y 20

#define INVENTORY_TRADE_LEFT_SCROLLER_X 29
#define INVENTORY_TRADE_LEFT_SCROLLER_Y INVENTORY_TRADE_SCROLLER_Y

#define INVENTORY_TRADE_RIGHT_SCROLLER_X 388
#define INVENTORY_TRADE_RIGHT_SCROLLER_Y INVENTORY_TRADE_SCROLLER_Y

#define INVENTORY_TRADE_INNER_LEFT_SCROLLER_X 165
#define INVENTORY_TRADE_INNER_LEFT_SCROLLER_Y INVENTORY_TRADE_INNER_SCROLLER_Y

#define INVENTORY_TRADE_INNER_RIGHT_SCROLLER_X 250
#define INVENTORY_TRADE_INNER_RIGHT_SCROLLER_Y INVENTORY_TRADE_INNER_SCROLLER_Y

#define INVENTORY_TRADE_LEFT_SCROLLER_TRACKING_X 29 - INVENTORY_SLOT_PADDING
#define INVENTORY_TRADE_LEFT_SCROLLER_TRACKING_Y 20
#define INVENTORY_TRADE_LEFT_SCROLLER_TRACKING_MAX_X (INVENTORY_TRADE_LEFT_SCROLLER_TRACKING_X + INVENTORY_SLOT_WIDTH + INVENTORY_SLOT_PADDING * 2)

#define INVENTORY_TRADE_INNER_LEFT_SCROLLER_TRACKING_X 165 - INVENTORY_SLOT_PADDING
#define INVENTORY_TRADE_INNER_LEFT_SCROLLER_TRACKING_Y 20
#define INVENTORY_TRADE_INNER_LEFT_SCROLLER_TRACKING_MAX_X (INVENTORY_TRADE_INNER_LEFT_SCROLLER_TRACKING_X + INVENTORY_SLOT_WIDTH + INVENTORY_SLOT_PADDING * 2)

#define INVENTORY_TRADE_INNER_RIGHT_SCROLLER_TRACKING_X 250 - INVENTORY_SLOT_PADDING
#define INVENTORY_TRADE_INNER_RIGHT_SCROLLER_TRACKING_Y 20
#define INVENTORY_TRADE_INNER_RIGHT_SCROLLER_TRACKING_MAX_X (INVENTORY_TRADE_INNER_RIGHT_SCROLLER_TRACKING_X + INVENTORY_SLOT_WIDTH + INVENTORY_SLOT_PADDING * 2)

#define INVENTORY_TRADE_RIGHT_SCROLLER_TRACKING_X 388 - INVENTORY_SLOT_PADDING
#define INVENTORY_TRADE_RIGHT_SCROLLER_TRACKING_Y 20
#define INVENTORY_TRADE_RIGHT_SCROLLER_TRACKING_MAX_X (INVENTORY_TRADE_RIGHT_SCROLLER_TRACKING_X + INVENTORY_SLOT_WIDTH + INVENTORY_SLOT_PADDING * 2)

#define INVENTORY_LOOT_LEFT_SCROLLER_X 180
#define INVENTORY_LOOT_LEFT_SCROLLER_Y 40
#define INVENTORY_LOOT_LEFT_SCROLLER_MAX_X (INVENTORY_LOOT_LEFT_SCROLLER_X + INVENTORY_SLOT_WIDTH)

#define INVENTORY_LOOT_RIGHT_SCROLLER_X 297
#define INVENTORY_LOOT_RIGHT_SCROLLER_Y 40
#define INVENTORY_LOOT_RIGHT_SCROLLER_MAX_X (INVENTORY_LOOT_RIGHT_SCROLLER_X + INVENTORY_SLOT_WIDTH)

#define INVENTORY_SCROLLER_X 46
#define INVENTORY_SCROLLER_Y 45
#define INVENTORY_SCROLLER_MAX_X (INVENTORY_SCROLLER_X + INVENTORY_SLOT_WIDTH)

#define INVENTORY_BODY_VIEW_WIDTH 60
#define INVENTORY_BODY_VIEW_HEIGHT 100

#define INVENTORY_PC_BODY_VIEW_X 176
#define INVENTORY_PC_BODY_VIEW_Y 37
#define INVENTORY_PC_BODY_VIEW_MAX_X (INVENTORY_PC_BODY_VIEW_X + INVENTORY_BODY_VIEW_WIDTH)
#define INVENTORY_PC_BODY_VIEW_MAX_Y (INVENTORY_PC_BODY_VIEW_Y + INVENTORY_BODY_VIEW_HEIGHT)

#define INVENTORY_LOOT_RIGHT_BODY_VIEW_X 422
#define INVENTORY_LOOT_RIGHT_BODY_VIEW_Y 35

#define INVENTORY_LOOT_LEFT_BODY_VIEW_X 44
#define INVENTORY_LOOT_LEFT_BODY_VIEW_Y 35

#define INVENTORY_SUMMARY_X 296
#define INVENTORY_SUMMARY_Y 45
#define INVENTORY_SUMMARY_WIDTH 154
#define INVENTORY_SUMMARY_HEIGHT 188

#define INVENTORY_WINDOW_WIDTH 499
#define INVENTORY_USE_ON_WINDOW_WIDTH 292
#define INVENTORY_LOOT_WINDOW_WIDTH 537
#define INVENTORY_TRADE_WINDOW_WIDTH 480
#define INVENTORY_TIMER_WINDOW_WIDTH 259

#define INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH 640
#define INVENTORY_TRADE_BACKGROUND_WINDOW_HEIGHT 480
#define INVENTORY_TRADE_WINDOW_OFFSET ((INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH - INVENTORY_TRADE_WINDOW_WIDTH) / 2)

#define INVENTORY_SCROLLER_X_PAD (INVENTORY_SCROLLER_X + INVENTORY_SLOT_PADDING)
#define INVENTORY_SCROLLER_Y_PAD (INVENTORY_SCROLLER_Y + INVENTORY_SLOT_PADDING)

#define INVENTORY_LOOT_LEFT_SCROLLER_X_PAD (INVENTORY_LOOT_LEFT_SCROLLER_X + INVENTORY_SLOT_PADDING)
#define INVENTORY_LOOT_LEFT_SCROLLER_Y_PAD (INVENTORY_LOOT_LEFT_SCROLLER_Y + INVENTORY_SLOT_PADDING)

#define INVENTORY_LOOT_RIGHT_SCROLLER_X_PAD (INVENTORY_LOOT_RIGHT_SCROLLER_X + INVENTORY_SLOT_PADDING)
#define INVENTORY_LOOT_RIGHT_SCROLLER_Y_PAD (INVENTORY_LOOT_RIGHT_SCROLLER_Y + INVENTORY_SLOT_PADDING)

#define INVENTORY_TRADE_LEFT_SCROLLER_X_PAD (INVENTORY_TRADE_LEFT_SCROLLER_X + INVENTORY_SLOT_PADDING)
#define INVENTORY_TRADE_LEFT_SCROLLER_Y_PAD (INVENTORY_TRADE_LEFT_SCROLLER_Y + INVENTORY_SLOT_PADDING)

#define INVENTORY_TRADE_RIGHT_SCROLLER_X_PAD (INVENTORY_TRADE_RIGHT_SCROLLER_X + INVENTORY_SLOT_PADDING)
#define INVENTORY_TRADE_RIGHT_SCROLLER_Y_PAD (INVENTORY_TRADE_RIGHT_SCROLLER_Y + INVENTORY_SLOT_PADDING)

#define INVENTORY_TRADE_INNER_LEFT_SCROLLER_X_PAD (INVENTORY_TRADE_INNER_LEFT_SCROLLER_X + INVENTORY_SLOT_PADDING)
#define INVENTORY_TRADE_INNER_LEFT_SCROLLER_Y_PAD (INVENTORY_TRADE_INNER_LEFT_SCROLLER_Y + INVENTORY_SLOT_PADDING)

#define INVENTORY_TRADE_INNER_RIGHT_SCROLLER_X_PAD (INVENTORY_TRADE_INNER_RIGHT_SCROLLER_X + INVENTORY_SLOT_PADDING)
#define INVENTORY_TRADE_INNER_RIGHT_SCROLLER_Y_PAD (INVENTORY_TRADE_INNER_RIGHT_SCROLLER_Y + INVENTORY_SLOT_PADDING)

#define INVENTORY_NORMAL_WINDOW_PC_ROTATION_DELAY (1000U / ROTATION_COUNT)
#define INVENTORY_FRM_COUNT 16

#define MAX_SORT_PRIORITY 999

#define INVENTORY_BUTTON_LEFT 2500 // Left portrait/body view (also triggers sort in arrow mode)
#define INVENTORY_BUTTON_RIGHT 2501 // Right portrait/body view (trade/loot)
#define INVENTORY_BUTTON_TAKE_ALL 2502 // Loot "Take All" button
#define INVENTORY_BUTTON_DROP_ALL 2503 // Loot "Drop All" button (enhanced)

// Equipment slot key codes (inventory screen)
#define INVENTORY_HAND_RIGHT_KEY 2006 // Right hand slot
#define INVENTORY_HAND_LEFT_KEY 2007 // Left hand slot
#define INVENTORY_ARMOR_KEY 2008 // Armor slot

// Key code bases for inventory buttons
#define KEYCODE_GRID_BASE 1000 // Player inventory grid
#define KEYCODE_TARGET_GRID_BASE 2000 // Target inventory grid (loot/trade)
#define KEYCODE_OFFER_LEFT_BASE 2300 // Player offer table (trade)
#define KEYCODE_OFFER_RIGHT_BASE 2400 // Merchant offer table (trade)
#define KEYCODE_FILTER_BASE 8000 // Category filter buttons

// Special button key codes (not in a contiguous range)
#define BUTTON_DONE 500 // Quantity dialog "Done"
#define BUTTON_ALL 5000 // Quantity dialog "All"
#define BUTTON_PLUS 6000 // Quantity dialog "+"
#define BUTTON_MINUS 7000 // Quantity dialog "-"

#define SORT_MENU_ITEM_COUNT (sizeof(_act_sort) / sizeof(_act_sort[0]))

#define MAX_COMBINED_ITEMS 300

typedef enum InventoryArrowFrm {
    INVENTORY_ARROW_FRM_LEFT_ARROW_UP,
    INVENTORY_ARROW_FRM_LEFT_ARROW_DOWN,
    INVENTORY_ARROW_FRM_RIGHT_ARROW_UP,
    INVENTORY_ARROW_FRM_RIGHT_ARROW_DOWN,
    INVENTORY_ARROW_FRM_COUNT,
} InventoryArrowFrm;

typedef enum InventoryWindowCursor {
    INVENTORY_WINDOW_CURSOR_HAND,
    INVENTORY_WINDOW_CURSOR_ARROW,
    INVENTORY_WINDOW_CURSOR_PICK,
    INVENTORY_WINDOW_CURSOR_MENU,
    INVENTORY_WINDOW_CURSOR_BLANK,
    INVENTORY_WINDOW_CURSOR_COUNT,
} InventoryWindowCursor;

typedef enum InventoryWindowType {
    // Normal inventory window with quick character sheet.
    INVENTORY_WINDOW_TYPE_NORMAL,

    // Narrow inventory window with just an item scroller that's shown when
    // a "Use item on" is selected from context menu.
    INVENTORY_WINDOW_TYPE_USE_ITEM_ON,

    // Looting/strealing interface.
    INVENTORY_WINDOW_TYPE_LOOT,

    // Barter interface.
    INVENTORY_WINDOW_TYPE_TRADE,

    // Supplementary "Move items" window. Used to set quantity of items when
    // moving items between inventories.
    INVENTORY_WINDOW_TYPE_MOVE_ITEMS,

    // Supplementary "Set timer" window. Internally it's implemented as "Move
    // items" window but with timer overlay and slightly different adjustment
    // mechanics.
    INVENTORY_WINDOW_TYPE_SET_TIMER,

    INVENTORY_WINDOW_TYPE_COUNT,
} InventoryWindowType;

typedef struct InventoryWindowConfiguration {
    int frmId; // artId
    int width;
    int height;
    int x;
    int y;
} InventoryWindowDescription;

typedef struct InventoryCursorData {
    Art* frm;
    unsigned char* frmData;
    int width;
    int height;
    int offsetX;
    int offsetY;
    CacheEntry* frmHandle;
} InventoryCursorData;

typedef enum InventoryMoveResult {
    INVENTORY_MOVE_RESULT_FAILED,
    INVENTORY_MOVE_RESULT_CAUGHT_STEALING,
    INVENTORY_MOVE_RESULT_SUCCESS,
} InventoryMoveResult;

static int inventoryMessageListInit();
static int inventoryMessageListFree();
static bool _setup_inventory(int inventoryWindowType);
static void _exit_inventory(bool shouldEnableIso);
static void _display_inventory(int stackOffset, int draggedSlotIndex, int inventoryWindowType);
static void _display_target_inventory(int stackOffset, int dragSlotIndex, Inventory* inventory, int inventoryWindowType);
static void _display_inventory_info(Object* item, int quantity, unsigned char* dest, int pitch, bool isDragged, bool isScreen);
static void _display_body(int fid, int inventoryWindowType);
static int inventoryCommonInit();
static void inventoryCommonFree();
static void inventorySetCursor(int cursor);
static void inventoryItemSlotOnMouseEnter(int btn, int keyCode);
static void inventoryItemSlotOnMouseExit(int btn, int keyCode);
static void _inven_update_lighting(Object* activeItem);
static void _inven_pickup(int keyCode, int indexOffset);
static void _switch_hand(Object* sourceItem, Object** targetSlot, Object** sourceSlot, int itemIndex);
static void _adjust_fid();
static void inventoryRenderSummary();
static int _inven_from_button(int keyCode, Object** outItem, Object*** outItemSlot, Object** outOwner);
static void inventoryRenderItemDescription(char* string);
static void inventoryExamineItem(Object* critter, Object* item);
static void inventoryWindowOpenContextMenu(int eventCode, int inventoryWindowType);
static InventoryMoveResult _move_inventory(Object* item, int slotIndex, Object* targetObj, bool isPlanting, int quantity);
static int _barter_compute_value(Object* dude, Object* npc);
static int _barter_attempt_transaction(Object* dude, Object* offerTable, Object* npc, Object* barterTable);
static int _barter_get_quantity_moved_items(Object* item, int maxQuantity, bool fromPlayer, bool fromInventory, bool immediate);
static void _barter_move_inventory(Object* item, int quantity, int slotIndex, int indexOffset, Object* npc, Object* sourceTable, bool fromDude);
static void _barter_move_from_table_inventory(Object* item, int quantity, int slotIndex, Object* npc, Object* sourceTable, bool fromDude);
static void inventoryWindowRenderInnerInventories(int win, Object* leftTable, Object* rightTable, int draggedSlotIndex);
static void _container_enter(int keyCode, int inventoryWindowType);
static void _container_exit(int keyCode, int inventoryWindowType);
static int _drop_into_container(Object* container, Object* item, int sourceIndex, Object** itemSlot, int quantity);
static int _drop_ammo_into_weapon(Object* weapon, Object* ammo, Object** ammoItemSlot, int quantity, int keyCode, Object* ammoOwner);
static void _draw_amount(int value, int inventoryWindowType);
static int inventoryQuantitySelect(int inventoryWindowType, Object* item, int maximum, int defaultValue = 1);
static int inventoryQuantityWindowInit(int inventoryWindowType, Object* item);
static int inventoryQuantityWindowFree(int inventoryWindowType);
static void _drag_item_loop(Object* item, bool immediate);

static void inventoryPortraitOnMouseEnter(int btn, int keyCode);
static void inventoryPortraitOnMouseExit(int btn, int keyCode);
static void inventoryWindowOpenSortContextMenu(int keyCode, int inventoryWindowType);
static bool _inven_sort_inventory(Object* obj, int sortType, int inventoryWindowType);
static void _move_money_to_top(Inventory* inventory, int itemCount);
static int _compare_items_by_weight(const void* a, const void* b);
static int _compare_items_by_value(const void* a, const void* b);

static void tradeWindowUpdateScrollButtons();

static void inventoryBuildCombinedList(Object* focusOwner = nullptr);
static void transferItemToCurrentOwner(Object* item, int quantity, Object* originalOwner);
static void sortCombinedInventory(int sortType, int inventoryWindowType);
static void applyCombinedSort(int sortType);
static void movePlayerMoneyToTopCombined();
static void inventoryBuildPartyList();
void inventoryOpenWithCycling();

// 0x46E6D0
static const int gSummaryStats[7] = {
    STAT_CURRENT_HIT_POINTS,
    STAT_ARMOR_CLASS,
    STAT_DAMAGE_THRESHOLD,
    STAT_DAMAGE_THRESHOLD_LASER,
    STAT_DAMAGE_THRESHOLD_FIRE,
    STAT_DAMAGE_THRESHOLD_PLASMA,
    STAT_DAMAGE_THRESHOLD_EXPLOSION,
};

// 0x46E6EC
static const int gSummaryStats2[7] = {
    STAT_MAXIMUM_HIT_POINTS,
    -1,
    STAT_DAMAGE_RESISTANCE,
    STAT_DAMAGE_RESISTANCE_LASER,
    STAT_DAMAGE_RESISTANCE_FIRE,
    STAT_DAMAGE_RESISTANCE_PLASMA,
    STAT_DAMAGE_RESISTANCE_EXPLOSION,
};

// 0x46E708
static const int gInventoryArrowFrmIds[INVENTORY_ARROW_FRM_COUNT] = {
    122, // left arrow up
    123, // left arrow down
    124, // right arrow up
    125, // right arrow down
};

// Background FRM IDs for normal inventory (1-4 columns)
static const int gInventoryBackgroundFrmIds[5] = {
    0, // index 0 unused
    48, // 1 column (original)
    7654, // 2 columns
    7655, // 3 columns
    7656 // 4 columns
};

// Global configuration: number of columns (default 1, will be read from settings later)
static int gInventoryColumns = 2;
static const int gInventoryRows = 6; // fixed number of rows
static int gCurrentInventoryBackgroundFrm = 48; // default to original
static int gCurrentLootBackgroundFrm = 114; // default to original

// Layout structure for normal INVENTORY (computed at runtime)
struct InventoryLayout {
    int windowWidth; // total window width
    int windowHeight; // always 377
    int scrollerX; // leftmost column X
    int scrollerY; // top row Y
    int scrollerWidth; // columns * slotWidth
    int scrollerHeight; // rows * slotHeight
    int slotWidth; // 64
    int slotHeight; // 48
    int slotPadding; // 4
    int slotContentWidth; // slotWidth - 2*padding
    int slotContentHeight; // slotHeight - 2*padding
    int leftHandSlotX;
    int leftHandSlotY;
    int rightHandSlotX;
    int rightHandSlotY;
    int armorSlotX;
    int armorSlotY;
    int summaryX;
    int summaryY;
    int bodyViewX;
    int bodyViewY;
    int scrollUpButtonX;
    int scrollUpButtonY;
    int scrollDownButtonX;
    int scrollDownButtonY;
    int doneButtonX;
    int doneButtonY;
};
static InventoryLayout gLayout;

// The number of items to show in scroller.
//
// 0x519054
static int gInventorySlotsCount = 6;

// Inventory Slot dimensions - adjusted per inventory session per inventory type.
static int gInventorySlotHeight = 48;
static int gInventorySlotWidthPadded = 56;
static int gInventorySlotHeightPadded = 38;

// 0x519058
static Object* _inven_dude = nullptr;

// Probably fid of armor to display in inventory dialog.
//
// 0x51905C
static int _inven_pid = -1;

// 0x519060
static bool _inven_is_initialized = false;

// 0x519064
static int _inven_display_msg_line = 1;

// 0x519068
static const InventoryWindowDescription gInventoryWindowDescriptions[INVENTORY_WINDOW_TYPE_COUNT] = {
    { 48, INVENTORY_WINDOW_WIDTH, 377, 80, 0 },
    { 113, INVENTORY_USE_ON_WINDOW_WIDTH, 376, 80, 0 },
    { 114, INVENTORY_LOOT_WINDOW_WIDTH, 376, 80, 0 },
    { 111, INVENTORY_TRADE_WINDOW_WIDTH, 180, 80, 290 },
    { 305, INVENTORY_TIMER_WINDOW_WIDTH, 162, 140, 80 },
    { 305, INVENTORY_TIMER_WINDOW_WIDTH, 162, 140, 80 },
};

// 0x5190E0
static bool _dropped_explosive = false;

// 0x5190E4
static int gInventoryScrollUpButton = -1;

// 0x5190E8
static int gInventoryScrollDownButton = -1;

// 0x5190EC
static int gSecondaryInventoryScrollUpButton = -1;

// 0x5190F0
static int gSecondaryInventoryScrollDownButton = -1;

static int gTradeLeftUpButton = -1; // player main inventory up
static int gTradeLeftDownButton = -1; // player main inventory down
static int gTradeRightUpButton = -1; // merchant main inventory up
static int gTradeRightDownButton = -1; // merchant main inventory down
static int gTradeOfferLeftUpButton = -1; // player offer table up
static int gTradeOfferLeftDownButton = -1; // player offer table down
static int gTradeOfferRightUpButton = -1; // merchant offer table up
static int gTradeOfferRightDownButton = -1; // merchant offer table down

// 0x5190F4
static unsigned int gInventoryWindowDudeRotationTimestamp = 0;

// 0x5190F8
static int gInventoryWindowDudeRotation = 0;

// 0x5190FC
static const int gInventoryWindowCursorFrmIds[INVENTORY_WINDOW_CURSOR_COUNT] = {
    286, // pointing hand
    250, // action arrow
    282, // action pick
    283, // action menu
    266, // blank
};

// 0x519110
static Object* _last_target = nullptr;

// Sort menu state variables
static bool _inven_sort_menu_active = false;
static int _inven_sort_menu_button = -1;
static int _inven_sort_menu_x = 0;
static int _inven_sort_menu_y = 0;
static int _inven_sort_menu_selected_index = 0;

// 0x519114
static const int _act_use[4] = {
    GAME_MOUSE_ACTION_MENU_ITEM_LOOK,
    GAME_MOUSE_ACTION_MENU_ITEM_USE,
    GAME_MOUSE_ACTION_MENU_ITEM_DROP,
    GAME_MOUSE_ACTION_MENU_ITEM_CANCEL,
};

// 0x519124
static const int _act_no_use[3] = {
    GAME_MOUSE_ACTION_MENU_ITEM_LOOK,
    GAME_MOUSE_ACTION_MENU_ITEM_DROP,
    GAME_MOUSE_ACTION_MENU_ITEM_CANCEL,
};

// 0x519130
static const int _act_just_use[3] = {
    GAME_MOUSE_ACTION_MENU_ITEM_LOOK,
    GAME_MOUSE_ACTION_MENU_ITEM_USE,
    GAME_MOUSE_ACTION_MENU_ITEM_CANCEL,
};

// 0x51913C
static const int _act_nothing[2] = {
    GAME_MOUSE_ACTION_MENU_ITEM_LOOK,
    GAME_MOUSE_ACTION_MENU_ITEM_CANCEL,
};

// 0x519144
static const int _act_weap[4] = {
    GAME_MOUSE_ACTION_MENU_ITEM_LOOK,
    GAME_MOUSE_ACTION_MENU_ITEM_UNLOAD,
    GAME_MOUSE_ACTION_MENU_ITEM_DROP,
    GAME_MOUSE_ACTION_MENU_ITEM_CANCEL,
};

// 0x519154
static const int _act_weap2[3] = {
    GAME_MOUSE_ACTION_MENU_ITEM_LOOK,
    GAME_MOUSE_ACTION_MENU_ITEM_UNLOAD,
    GAME_MOUSE_ACTION_MENU_ITEM_CANCEL,
};

// 'sort' action menu
static const int _act_sort[5] = {
    GAME_MOUSE_ACTION_MENU_ITEM_SORT_DEFAULT,
    GAME_MOUSE_ACTION_MENU_ITEM_SORT_WEIGHT,
    GAME_MOUSE_ACTION_MENU_ITEM_SORT_VALUE,
    GAME_MOUSE_ACTION_MENU_ITEM_SORT_REVERSE,
    GAME_MOUSE_ACTION_MENU_ITEM_CANCEL
};

// Scroll offsets to target inventory for every container nesting level (stack).
// 0x59E7EC
static int _target_stack_offset[10];

// inventory.msg
//
// 0x59E814
static MessageList gInventoryMessageList;

static MessageList gFissionMessageList;
static MessageListItem gFissionMessageListItem;

// Current target critter or container for every nesting level (stack).
// 0x59E81C
static Object* _target_stack[10];

// Scroll offsets to main inventory for every container nesting level (stack).
// 0x59E844
static int _stack_offset[10];

// Current critter or container for every nesting level (stack).
// 0x59E86C
static Object* _stack[10];

// 0x59E894
static int _mt_wid;

// Current barter price modifier, set from scripts.
// 0x59E898
static int _barter_mod;

// 0x59E89C
static int _btable_offset;

// 0x59E8A0
static int _ptable_offset;

// 0x59E8A4
static Inventory* _ptable_pud;

// 0x59E8A8
static InventoryCursorData gInventoryCursorData[INVENTORY_WINDOW_CURSOR_COUNT];

// 0x59E934
static Object* _ptable;

// 0x59E938
static InventoryPrintItemDescriptionHandler* gInventoryPrintItemDescriptionHandler;

// 0x59E93C
static int _im_value; // "keyCode" corresponding to an inventory item "button", or -1 if nothing

static int _portrait_im_value; // keyCode for the portrait button the mouse is over, or -1

// 0x59E940
static int gInventoryCursor;

// 0x59E944
static Object* _btable;

// Current nesting level for viewing target's bag/backpack contents.
// 0x59E948
static int _target_curr_stack;

// 0x59E94C
static Inventory* _btable_pud;

// 0x59E950
static bool _inven_ui_was_disabled;

// 0x59E954
static Object* gInventoryArmor;

// 0x59E958
static Object* gInventoryLeftHandItem;

// Rotating character's fid.
//
// 0x59E95C
static int gInventoryWindowDudeFid;

// 0x59E960
static Inventory* _pud;

// 0x59E964
static int gInventoryWindow;

// item2
// 0x59E968
static Object* gInventoryRightHandItem;

// Current nesting level for viewing bag/backpack contents.
// 0x59E96C
static int _curr_stack;

// 0x59E970
static int gInventoryWindowMaxY;

// 0x59E974
static int gInventoryWindowMaxX;

// 0x59E978
static Inventory* _target_pud;

// 0x59E97C
static int _barter_back_win;

static bool _inven_redrawing_after_sort_menu = false;

// For companion inventory
static int gArmorSlotButton = -1;
static int gLeftHandSlotButton = -1;
static int gRightHandSlotButton = -1;
static FrmImage gGreySlotFrm;
static int gCurrentInvWindowType = -1;

// Tracks insult-based price increases
static int gBarterInsultIncrease = 0;

bool lootWindowOpened = false;

// Rotation tracking for quick-click sort
static Object* _last_quick_sorted_object;
static int _next_quick_sort_type = GAME_MOUSE_ACTION_MENU_ITEM_SORT_DEFAULT;

static FrmImage _inventoryFrmImages[INVENTORY_FRM_COUNT];
static FrmImage _moveFrmImages[8];

// Combined Inventory struct
typedef struct CombinedItem {
    Object* item;
    int quantity;
    Object* owner;
} CombinedItem;

// Combined Inventory globals
static CombinedItem gCombinedItems[MAX_COMBINED_ITEMS];
static int gCombinedItemCount = 0;
static bool gUseCombinedInventory = false;
static Object* gCombinedExcludeObject = nullptr;
static int gCombinedSortType = -1;
static bool gIsTradeWindow = false;
static bool gSwitchToCharacter = false;
static Object* gSwitchTarget = nullptr;
static std::vector<Object*> gPartyList;
static int gCurrentPartyIndex = 0;

static int getLeftDisplayCount()
{
    return gUseCombinedInventory ? gCombinedItemCount : _pud->length;
}

// Filter globals
static int gFilterCategory = -1; // -1 = no filter, 0-4 for Weapons, Ammo, Drugs, Misc, Keys
static int gFilteredIndices[256];
static int gFilteredCount = 0;

static int itemGetCategory(Object* item)
{
    int type = itemGetType(item);
    switch (type) {
    case ITEM_TYPE_WEAPON:
        return 0;
    case ITEM_TYPE_AMMO:
        return 1;
    case ITEM_TYPE_DRUG:
        return 2;
    case ITEM_TYPE_ARMOR:
        return 3;
    case ITEM_TYPE_MISC:
    case ITEM_TYPE_KEY:
    case ITEM_TYPE_CONTAINER:
        return 4;
    default:
        return -1;
    }
}

// Builds a list of indices into 'inventory' that match the current filter.
// Stores them in gFilteredIndices (reversed, so top of screen = last item).
// Returns the number of filtered items.
static int buildFilteredIndices(Inventory* inventory)
{
    int count = 0;
    if (gFilterCategory == -1) {
        for (int i = inventory->length - 1; i >= 0; i--) {
            gFilteredIndices[count++] = i;
        }
    } else {
        for (int i = inventory->length - 1; i >= 0; i--) {
            Object* item = inventory->items[i].item;
            if (itemGetCategory(item) == gFilterCategory) {
                gFilteredIndices[count++] = i;
            }
        }
    }
    return count;
}

// Build filtered indices for the combined inventory list.
// Stores indices into gCombinedItems (reversed) in gFilteredIndices.
// Returns the number of matching items.
static int buildFilteredCombinedIndices()
{
    int count = 0;
    if (gFilterCategory == -1) {
        for (int i = gCombinedItemCount - 1; i >= 0; i--) {
            gFilteredIndices[count++] = i;
        }
    } else {
        for (int i = gCombinedItemCount - 1; i >= 0; i--) {
            Object* item = gCombinedItems[i].item;
            if (itemGetCategory(item) == gFilterCategory) {
                gFilteredIndices[count++] = i;
            }
        }
    }
    gFilteredCount = count;
    return count;
}

// Returns the number of filtered items (combined or regular) and
// populates gFilteredIndices accordingly.
static int getFilteredCount()
{
    if (gUseCombinedInventory) {
        return buildFilteredCombinedIndices();
    } else {
        return buildFilteredIndices(_pud);
    }
}

// Returns the background FRM ID for the given inventory window type.
static int inventoryGetBackgroundFrm(int windowType)
{
    switch (windowType) {
    case INVENTORY_WINDOW_TYPE_NORMAL:
        return gCurrentInventoryBackgroundFrm;
    case INVENTORY_WINDOW_TYPE_LOOT:
        return gCurrentLootBackgroundFrm;
    case INVENTORY_WINDOW_TYPE_USE_ITEM_ON:
        return 113;
    case INVENTORY_WINDOW_TYPE_TRADE:
        return gGameDialogSpeakerIsPartyMember ? 420 : 111;
    default:
        return 48;
    }
}

// Draw weight/capacity info centered under the portrait.
static void inventoryDrawWeightInfo(unsigned char* dest, int pitch, int x, int y, Object* obj)
{
    if (settings.enhancements.strict_vanilla || !settings.enhancements.display_weight) return;

    char formattedText[20];
    formattedText[0] = '\0';

    int oldFont = fontGetCurrent();
    fontSetCurrent(101);

    int color = _colorTable[COL_LIME_GREEN];
    if (PID_TYPE(obj->pid) == OBJ_TYPE_CRITTER) {
        int cur = objectGetInventoryWeight(obj);
        int max = critterGetStat(obj, STAT_CARRY_WEIGHT);
        snprintf(formattedText, sizeof(formattedText), "%d/%d", cur, max);
        if (critterIsEncumbered(obj)) {
            color = _colorTable[COL_PURE_RED];
        }
    } else if (PID_TYPE(obj->pid) == OBJ_TYPE_ITEM && itemGetType(obj) == ITEM_TYPE_CONTAINER) {
        int cur = containerGetTotalSize(obj);
        int max = containerGetMaxSize(obj);
        snprintf(formattedText, sizeof(formattedText), "%d/%d", cur, max);
    } else {
        int weight = objectGetInventoryWeight(obj);
        snprintf(formattedText, sizeof(formattedText), "%d", weight);
    }

    int textWidth = fontGetStringWidth(formattedText);
    int textX = x + (INVENTORY_BODY_VIEW_WIDTH - textWidth) / 2;
    fontDrawText(dest + pitch * y + textX, formattedText, textWidth, pitch, color);

    fontSetCurrent(oldFont);
}

static void inventoryRefreshBodies(int inventoryWindowType)
{
    if (settings.enhancements.strict_vanilla || !settings.enhancements.display_weight)
        return;

    _display_body(-1, inventoryWindowType);
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT || inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        Object* target = nullptr;
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
            target = _target_stack[_target_curr_stack];
        } else { // TRADE
            target = _target_stack[0];
        }
        if (target != nullptr) {
            _display_body(target->fid, inventoryWindowType);
        }
    }
}

static void drawFilterBar(unsigned char* dest, int destPitch,
    int x, int y, int width)
{
    if (settings.enhancements.strict_vanilla || !settings.enhancements.inventory_filter) return;

    int oldFont = fontGetCurrent();

    fontSetCurrent(101);
    int h101 = fontGetLineHeight();
    fontSetCurrent(106);
    int h106 = fontGetLineHeight();
    fontSetCurrent(oldFont);
    int barHeight = (h101 > h106 ? h101 : h106) + 4;

    int bgFid = buildFid(OBJ_TYPE_INTERFACE, inventoryGetBackgroundFrm(gCurrentInvWindowType), 0, 0, 0);
    FrmImage bg;
    if (bg.lock(bgFid)) {
        int bgPitch = bg.getWidth();
        int srcX = x;
        int srcY = y;
        int srcWidth = width;
        int srcHeight = barHeight;

        if (gCurrentInvWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
            srcX += INVENTORY_TRADE_WINDOW_X;
        }

        if (srcX + srcWidth > bgPitch) srcWidth = bgPitch - srcX;
        if (srcY + srcHeight > bg.getHeight()) srcHeight = bg.getHeight() - srcY;

        if (srcWidth > 0 && srcHeight > 0) {
            unsigned char* src = bg.getData() + bgPitch * srcY + srcX;
            blitBufferToBuffer(src, srcWidth, srcHeight, bgPitch,
                dest + destPitch * y + x, destPitch);
        }
        bg.unlock();
    }

    const char* fullNames[] = { "Weap", "Ammo", "Drug", "Gear", "Misc" };
    const char categoryIcons[] = { 'A', 'B', 'C', 'D', 'E' };
    const int numCategories = 5;
    const int spacing = 5;
    int available = width - (numCategories - 1) * spacing;
    if (available <= 0) return;
    int perCategory = available / numCategories;

    int mode = 0;
    fontSetCurrent(101);
    int maxBracketWidth = 0, maxBaseWidth = 0, max3CharWidth = 0, max2CharWidth = 0;
    for (int i = 0; i < numCategories; i++) {
        char bracketStr[32];
        snprintf(bracketStr, sizeof(bracketStr), "[%s]", fullNames[i]);
        int w = fontGetStringWidth(bracketStr);
        if (w > maxBracketWidth) maxBracketWidth = w;
        int baseW = fontGetStringWidth(fullNames[i]);
        if (baseW > maxBaseWidth) maxBaseWidth = baseW;
        char three[16];
        strncpy(three, fullNames[i], 3);
        three[3] = '\0';
        int w3 = fontGetStringWidth(three);
        if (w3 > max3CharWidth) max3CharWidth = w3;
        char two[16];
        strncpy(two, fullNames[i], 2);
        two[2] = '\0';
        int w2 = fontGetStringWidth(two);
        if (w2 > max2CharWidth) max2CharWidth = w2;
    }
    if (maxBracketWidth <= perCategory)
        mode = 0;
    else if (maxBaseWidth <= perCategory)
        mode = 1;
    else if (max3CharWidth <= perCategory)
        mode = 2;
    else if (max2CharWidth <= perCategory)
        mode = 3;
    else
        mode = 4;

    int startX = x + 2;
    for (int i = 0; i < numCategories; i++) {
        const char* label = nullptr;
        char buffer[32];
        bool useIcon = (mode == 4);
        if (mode == 0) {
            snprintf(buffer, sizeof(buffer), "[%s]", fullNames[i]);
            label = buffer;
        } else if (mode == 1) {
            label = fullNames[i];
        } else if (mode == 2) {
            strncpy(buffer, fullNames[i], 3);
            buffer[3] = '\0';
            label = buffer;
        } else if (mode == 3) {
            strncpy(buffer, fullNames[i], 2);
            buffer[2] = '\0';
            label = buffer;
        } else {
            buffer[0] = categoryIcons[i];
            buffer[1] = '\0';
            label = buffer;
        }
        int fontToUse = useIcon ? 106 : 101;
        fontSetCurrent(fontToUse);
        int finalWidth = fontGetStringWidth(label);
        int offset = (perCategory - finalWidth) / 2;
        int drawX = startX + i * (perCategory + spacing) + offset;
        int color;
        if (gFilterCategory == i) {
            color = _colorTable[COL_LIGHT_LEMON];
        } else if (gFilterCategory != -1) {
            color = _colorTable[COL_DARK_FOREST];
        } else {
            color = _colorTable[COL_BRIGHT_LIME];
        }
        fontDrawText(dest + destPitch * (y + 2) + drawX, label, finalWidth, destPitch, color);
    }
    fontSetCurrent(oldFont);
}

static void createFilterButtons(int win, int x, int y, int width, int baseKeyCode)
{
    if (settings.enhancements.strict_vanilla || !settings.enhancements.inventory_filter) return;

    const int numCategories = 5;
    const int spacing = 6;
    int available = width - (numCategories - 1) * spacing;
    if (available <= 0) return;

    int perCategory = available / numCategories;
    int buttonHeight = fontGetLineHeight();

    for (int i = 0; i < numCategories; i++) {
        int drawX = x + i * (perCategory + spacing);
        int btn = buttonCreate(win,
            drawX, y,
            perCategory, buttonHeight,
            -1, -1, baseKeyCode + i, -1,
            nullptr, nullptr, nullptr,
            BUTTON_FLAG_TRANSPARENT);
        (void)btn;
    }
}

static int inventoryKeyToFilterCategory(int keyCode)
{
    switch (keyCode) {
    case KEY_UPPERCASE_W:
    case KEY_LOWERCASE_W:
        return 0;
    case KEY_UPPERCASE_A:
    case KEY_LOWERCASE_A:
        return 1;
    case KEY_UPPERCASE_D:
    case KEY_LOWERCASE_D:
        return 2;
    case KEY_UPPERCASE_G:
    case KEY_LOWERCASE_G:
        return 3;
    case KEY_UPPERCASE_M:
    case KEY_LOWERCASE_M:
        return 4;
    default:
        return -1;
    }
}

static void applyGreenFilterToBuffer(const unsigned char* src, int srcPitch,
    unsigned char* dest, int destPitch,
    int width, int height)
{
    int minLum = 255, maxLum = 0;
    bool hasPixels = false;
    for (int row = 0; row < height; row++) {
        const unsigned char* srcRow = src + row * srcPitch;
        for (int col = 0; col < width; col++) {
            unsigned char idx = srcRow[col];
            if (idx == 0) continue;
            unsigned char* pal = &_cmap[idx * 3];
            int lum = (pal[0] * 30 + pal[1] * 59 + pal[2] * 11) / 100;
            if (lum < minLum) minLum = lum;
            if (lum > maxLum) maxLum = lum;
            hasPixels = true;
        }
    }
    if (!hasPixels) return;
    if (minLum == maxLum) maxLum = minLum + 1;

    int greenPalette[] = {
        _colorTable[COL_BLACKISH_TEAL],
        _colorTable[COL_DARK_FOREST],
        _colorTable[COL_FOREST_GREEN_2],
        _colorTable[COL_GREEN_LIME],
        _colorTable[COL_BRIGHT_LIME],
        _colorTable[COL_LIME_GREEN],
        _colorTable[COL_LIGHT_LEMON],
        _colorTable[COL_LIGHT_SPRING_GREEN]
    };
    int numGreenShades = sizeof(greenPalette) / sizeof(greenPalette[0]);

    float darkMultiplier = 0.6f;
    int darkThreshold = 40;
    bool scanlines = true;
    bool darkenEvenRows = true;

    for (int row = 0; row < height; row++) {
        bool isEven = (row % 2 == 0);
        bool drawRow = true;
        if (scanlines && isEven) {
            if (!darkenEvenRows) {
                drawRow = false;
            }
        }
        const unsigned char* srcRow = src + row * srcPitch;
        unsigned char* destRow = dest + row * destPitch;
        for (int col = 0; col < width; col++) {
            unsigned char idx = srcRow[col];
            if (idx == 0) continue;
            unsigned char* pal = &_cmap[idx * 3];
            int lum = (pal[0] * 30 + pal[1] * 59 + pal[2] * 11) / 100;
            if (lum < 0) lum = 0;
            if (lum > 255) lum = 255;
            int stretched = (lum - minLum) * 255 / (maxLum - minLum);
            if (stretched < 0) stretched = 0;
            if (stretched > 255) stretched = 255;
            bool skipPixel = false;
            if (scanlines && isEven && darkenEvenRows) {
                int darkened = (int)(stretched * darkMultiplier);
                if (darkened < darkThreshold) {
                    skipPixel = true;
                } else {
                    stretched = darkened;
                }
            }
            if (!drawRow || skipPixel) continue;
            int shadeIdx = (stretched * (numGreenShades - 1) + 127) / 255;
            if (shadeIdx < 0) shadeIdx = 0;
            if (shadeIdx >= numGreenShades) shadeIdx = numGreenShades - 1;
            destRow[col] = (unsigned char)greenPalette[shadeIdx];
        }
    }
}

static void artRenderGreen(int fid, unsigned char* dest, int width, int height, int pitch)
{
    if (!settings.enhancements.green_monochrome || settings.enhancements.strict_vanilla) {
        artRender(fid, dest, width, height, pitch);
    } else {
        unsigned char* temp = (unsigned char*)malloc(width * height);
        if (temp == nullptr) return;
        memset(temp, 0, width * height);
        artRender(fid, temp, width, height, width);
        applyGreenFilterToBuffer(temp, width, dest, pitch, width, height);
        free(temp);
    }
}

void blitBufferToBufferGreenTrans(unsigned char* src, int srcWidth, int srcHeight, int srcPitch,
    unsigned char* dest, int destPitch)
{
    if (!settings.enhancements.green_monochrome || settings.enhancements.strict_vanilla) {
        blitBufferToBufferTrans(src, srcWidth, srcHeight, srcPitch, dest, destPitch);
    } else {
        applyGreenFilterToBuffer(src, srcPitch, dest, destPitch, srcWidth, srcHeight);
    }
}

// Computes layout based on number of columns and sets common elements
static void inventoryUpdateLayout()
{

    // Set slot height
    if (!settings.enhancements.strict_vanilla && settings.enhancements.inventory_filter) {
        gInventorySlotHeight = 47; // default
    }

    // Trade window with filter bar active: shrink height to leave room.
    if (gCurrentInvWindowType == INVENTORY_WINDOW_TYPE_TRADE
        && !settings.enhancements.strict_vanilla
        && settings.enhancements.inventory_filter) {
        gInventorySlotHeight = 44;
    }

    // Normal Inventory with filter bar active: Expand height slightly.
    if (gCurrentInvWindowType == INVENTORY_WINDOW_TYPE_NORMAL
        && !settings.enhancements.strict_vanilla
        && settings.enhancements.inventory_filter) {
        gInventorySlotHeight = 46;
    }

    // Compute padded versions.
    gInventorySlotWidthPadded = INVENTORY_SLOT_WIDTH - INVENTORY_SLOT_PADDING * 2;
    gInventorySlotHeightPadded = gInventorySlotHeight - INVENTORY_SLOT_PADDING * 2;

    // Multi-Column Layout
    // Determine number of columns and loot background from settings
    if (settings.enhancements.strict_vanilla) {
        gInventoryColumns = 1;
        gCurrentLootBackgroundFrm = 114;
    } else {
        int requested = settings.enhancements.multi_column_inventory;
        // Clamp between 1 and 4 (3 for non widescreen)
        if (requested < 1) requested = 1;
        if (requested > 4) requested = 4;
        if (!settings.graphics.widescreen && requested == 4) // restrict 4 columns to widescreen only, otherwise crash
            requested = 3;
        gInventoryColumns = requested;
        gCurrentLootBackgroundFrm = 6294;
    }

    // Compute the shift due to extra columns
    int shift = (gInventoryColumns - 1) * INVENTORY_SLOT_WIDTH;

    // Fill layout fields (use the globals we just set)
    gLayout.windowWidth = INVENTORY_WINDOW_WIDTH + shift;
    gLayout.windowHeight = 377;
    gLayout.scrollerX = INVENTORY_SCROLLER_X;
    gLayout.scrollerY = INVENTORY_SCROLLER_Y;
    gLayout.scrollerWidth = gInventoryColumns * INVENTORY_SLOT_WIDTH;
    gLayout.scrollerHeight = gInventoryRows * gInventorySlotHeight;
    gLayout.slotWidth = INVENTORY_SLOT_WIDTH;
    gLayout.slotHeight = gInventorySlotHeight;
    gLayout.slotPadding = INVENTORY_SLOT_PADDING;
    gLayout.slotContentWidth = gInventorySlotWidthPadded;
    gLayout.slotContentHeight = gInventorySlotHeightPadded;

    // Equipment slots (these are to the right of the scroller, so they shift)
    gLayout.leftHandSlotX = INVENTORY_LEFT_HAND_SLOT_X + shift;
    gLayout.leftHandSlotY = INVENTORY_LEFT_HAND_SLOT_Y;
    gLayout.rightHandSlotX = INVENTORY_RIGHT_HAND_SLOT_X + shift;
    gLayout.rightHandSlotY = INVENTORY_RIGHT_HAND_SLOT_Y;
    gLayout.armorSlotX = INVENTORY_ARMOR_SLOT_X + shift;
    gLayout.armorSlotY = INVENTORY_ARMOR_SLOT_Y;

    // Summary area
    gLayout.summaryX = INVENTORY_SUMMARY_X + shift;
    gLayout.summaryY = INVENTORY_SUMMARY_Y;

    // Character portrait
    gLayout.bodyViewX = INVENTORY_PC_BODY_VIEW_X + shift;
    gLayout.bodyViewY = INVENTORY_PC_BODY_VIEW_Y;

    // Scroll buttons (they also shift right)
    gLayout.scrollUpButtonX = 128 + shift;
    gLayout.scrollUpButtonY = 39;
    gLayout.scrollDownButtonX = 128 + shift;
    gLayout.scrollDownButtonY = 62;

    // Done button (shifts too)
    gLayout.doneButtonX = 438 + shift;
    gLayout.doneButtonY = 329;

    // Set the background FRM ID for the current column count
    if (gInventoryColumns >= 1 && gInventoryColumns <= 4 && gInventoryBackgroundFrmIds[gInventoryColumns] != 0) {
        gCurrentInventoryBackgroundFrm = gInventoryBackgroundFrmIds[gInventoryColumns];
    } else {
        gCurrentInventoryBackgroundFrm = 48; // fallback
    }
}

// 0x46E724
void inventoryResetDude()
{
    _inven_dude = gDude;
    _inven_pid = 0x1000000;
}

// inventory_msg_init
// 0x46E73C
static int inventoryMessageListInit()
{
    char path[COMPAT_MAX_PATH];

    if (!messageListInit(&gInventoryMessageList))
        return -1;

    snprintf(path, sizeof(path), "%s%s", asc_5186C8, "inventry.msg");
    if (!messageListLoad(&gInventoryMessageList, path))
        return -1;

    if (!messageListInit(&gFissionMessageList)) {
        return -1;
    }

    char fissionPath[COMPAT_MAX_PATH];
    snprintf(fissionPath, sizeof(fissionPath), "%s%s", asc_5186C8, "fission.msg");
    if (!messageListLoad(&gFissionMessageList, fissionPath)) {
        return -1;
    }

    return 0;
}

// inventory_msg_free
// 0x46E7A0
static int inventoryMessageListFree()
{
    messageListFree(&gInventoryMessageList);
    return 0;
}

// New entry point for inventories - needs Strict Vanilla wrapping
void inventoryOpenWithCycling(Object* startTarget)
{

    if (settings.enhancements.strict_vanilla || !settings.enhancements.companion_inventory) {
        // Just open the player's inventory directly
        inventoryOpen();
        return;
    }
    Object* target = (startTarget != nullptr) ? startTarget : gDude;

    while (true) {
        gSwitchToCharacter = false;
        gSwitchTarget = nullptr;

        if (target == gDude) {
            inventoryOpen();
        } else {
            inventoryOpenForCompanion(target);
        }

        if (gSwitchToCharacter && gSwitchTarget != nullptr) {
            target = gSwitchTarget;
        } else {
            break;
        }
    }

    gSwitchToCharacter = false;
    gSwitchTarget = nullptr;
}

// Main event loop for the normal (player/companion) inventory window.
// Returns 1 if the user requested to switch to another party member,
static int inventoryRunLoop(void)
{
    const int totalVisible = gInventoryRows * gInventoryColumns;

    for (;;) {
        sharedFpsLimiter.mark();

        int keyCode = inputGetInput();

        // Close with ESC or 'I'
        if (keyCode == KEY_ESCAPE || keyCode == KEY_UPPERCASE_I || keyCode == KEY_LOWERCASE_I) {
            return 0;
        }

        if (_game_user_wants_to_quit != 0) {
            return 0;
        }

        _display_body(-1, INVENTORY_WINDOW_TYPE_NORMAL);

        if (gameGetState() == GAME_STATE_5) {
            return 0;
        }

        // Party cycling (companion switching)
        if (!settings.enhancements.strict_vanilla && settings.enhancements.companion_inventory) {
            if (keyCode == KEY_ARROW_LEFT || keyCode == KEY_ARROW_RIGHT) {
                // Save who we're currently on before any list modification
                Object* currentCritter = (gCurrentPartyIndex >= 0 && gCurrentPartyIndex < (int)gPartyList.size())
                    ? gPartyList[gCurrentPartyIndex]
                    : nullptr;

                // Fail-safe: ensure player is in the list
                bool hasPlayer = false;
                for (Object* obj : gPartyList) {
                    if (obj == gDude) {
                        hasPlayer = true;
                        break;
                    }
                }
                if (!hasPlayer && gDude != nullptr) {
                    debugPrint("Player missing from party list - inserting at front.\n");
                    gPartyList.insert(gPartyList.begin(), gDude);
                    // Recalculate current index by finding who we were on before insertion
                    gCurrentPartyIndex = 0;
                    for (int i = 0; i < (int)gPartyList.size(); i++) {
                        if (gPartyList[i] == currentCritter) {
                            gCurrentPartyIndex = i;
                            break;
                        }
                    }
                }

                if (gPartyList.size() <= 1) continue;

                // Validate current index is still in bounds
                if (gCurrentPartyIndex < 0 || gCurrentPartyIndex >= (int)gPartyList.size()) {
                    gCurrentPartyIndex = 0;
                }

                int newIndex = gCurrentPartyIndex;
                if (keyCode == KEY_ARROW_LEFT) {
                    newIndex--;
                    if (newIndex < 0) newIndex = (int)gPartyList.size() - 1;
                } else {
                    newIndex++;
                    if (newIndex >= (int)gPartyList.size()) newIndex = 0;
                }

                if (newIndex == gCurrentPartyIndex) continue;

                Object* newCritter = gPartyList[newIndex];
                if (newCritter == nullptr || !critterIsActive(newCritter)) continue;

                gSwitchTarget = newCritter;
                gSwitchToCharacter = true;
                return 1; // request switch
            }
        }

        // Keyboard shortcuts
        if (keyCode == KEY_CTRL_Q || keyCode == KEY_CTRL_X) {
            showQuitConfirmationDialog();
        } else if (keyCode == KEY_HOME) {
            _stack_offset[_curr_stack] = 0;
            _display_inventory(0, -1, INVENTORY_WINDOW_TYPE_NORMAL);
        } else if (keyCode == KEY_ARROW_UP) {
            if (_stack_offset[_curr_stack] >= gInventoryColumns) {
                _stack_offset[_curr_stack] -= gInventoryColumns;
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_NORMAL);
            }
        } else if (keyCode == KEY_PAGE_UP) {
            _stack_offset[_curr_stack] -= totalVisible;
            if (_stack_offset[_curr_stack] < 0) {
                _stack_offset[_curr_stack] = 0;
            }
            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_NORMAL);
        } else if (keyCode == KEY_END) {
            int total = getFilteredCount();
            _stack_offset[_curr_stack] = total - totalVisible;
            if (_stack_offset[_curr_stack] < 0) {
                _stack_offset[_curr_stack] = 0;
            }
            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_NORMAL);
        } else if (keyCode == KEY_ARROW_DOWN) {
            int total = getFilteredCount();
            if (_stack_offset[_curr_stack] + totalVisible < total) {
                _stack_offset[_curr_stack] += gInventoryColumns;
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_NORMAL);
            }
        } else if (keyCode == KEY_PAGE_DOWN) {
            int total = getFilteredCount();
            _stack_offset[_curr_stack] += totalVisible;
            if (_stack_offset[_curr_stack] + totalVisible >= total) {
                _stack_offset[_curr_stack] = total - totalVisible;
                if (_stack_offset[_curr_stack] < 0) {
                    _stack_offset[_curr_stack] = 0;
                }
            }
            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_NORMAL);
        } else if (keyCode >= KEYCODE_FILTER_BASE && keyCode <= 8004) {
            if (!settings.enhancements.strict_vanilla && settings.enhancements.inventory_filter) {
                int category = keyCode - KEYCODE_FILTER_BASE;
                if (gFilterCategory == category) {
                    gFilterCategory = -1; // toggle off
                } else {
                    gFilterCategory = category;
                }
                soundPlayFile("ib1p1xx1");
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_NORMAL);
                inventoryRenderSummary();
                windowRefresh(gInventoryWindow);
            }
        } else if (keyCode == INVENTORY_BUTTON_LEFT) {
            if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
                inventoryWindowOpenSortContextMenu(keyCode, INVENTORY_WINDOW_TYPE_NORMAL);
            } else {
                _container_exit(keyCode, INVENTORY_WINDOW_TYPE_NORMAL);
            }
        } else {
            // Mouse handling
            if ((mouseGetEvent() & MOUSE_EVENT_RIGHT_BUTTON_DOWN) != 0) {
                if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_HAND) {
                    inventorySetCursor(INVENTORY_WINDOW_CURSOR_ARROW);
                } else if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
                    inventorySetCursor(INVENTORY_WINDOW_CURSOR_HAND);
                    inventoryRenderSummary();
                    windowRefresh(gInventoryWindow);
                }
            } else if ((mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_DOWN) != 0) {
                int totalVisibleSlots = gInventoryRows * gInventoryColumns;
                if ((keyCode >= KEYCODE_GRID_BASE && keyCode < KEYCODE_GRID_BASE + totalVisibleSlots) || (keyCode == INVENTORY_HAND_RIGHT_KEY || keyCode == INVENTORY_HAND_LEFT_KEY || keyCode == INVENTORY_ARMOR_KEY)) {
                    if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
                        inventoryWindowOpenContextMenu(keyCode, INVENTORY_WINDOW_TYPE_NORMAL);
                    } else {
                        _inven_pickup(keyCode, _stack_offset[_curr_stack]);
                    }
                }
            } else if ((mouseGetEvent() & MOUSE_EVENT_WHEEL) != 0) {
                if (mouseHitTestInWindow(gInventoryWindow, gLayout.scrollerX, gLayout.scrollerY,
                        gLayout.scrollerX + gLayout.scrollerWidth,
                        gLayout.scrollerY + gLayout.scrollerHeight)) {
                    int wheelX, wheelY;
                    mouseGetWheel(&wheelX, &wheelY);
                    if (wheelY > 0) {
                        if (_stack_offset[_curr_stack] >= gInventoryColumns) {
                            _stack_offset[_curr_stack] -= gInventoryColumns;
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_NORMAL);
                        }
                    } else if (wheelY < 0) {
                        int total = getFilteredCount();
                        if (_stack_offset[_curr_stack] + totalVisible < total) {
                            _stack_offset[_curr_stack] += gInventoryColumns;
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_NORMAL);
                        }
                    }
                }
            }
        }

        // Filter via keyboard shortcuts (W,A,D,G,M)
        if (!settings.enhancements.strict_vanilla && settings.enhancements.inventory_filter) {
            int filterCategory = inventoryKeyToFilterCategory(keyCode);
            if (filterCategory != -1) {
                if (gFilterCategory == filterCategory) {
                    gFilterCategory = -1;
                } else {
                    gFilterCategory = filterCategory;
                }
                _stack_offset[_curr_stack] = 0;
                soundPlayFile("ib1p1xx1");
                _display_inventory(0, -1, INVENTORY_WINDOW_TYPE_NORMAL);
                inventoryRenderSummary();
                windowRefresh(gInventoryWindow);
            }
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }
}

// 0x46E7B0
void inventoryOpen()
{
    if (isInCombat()) {
        if (_combat_whose_turn() != _inven_dude) {
            return;
        }
    }

    ScopedGameMode gm(GameMode::kInventory);

    gSwitchToCharacter = false;
    gSwitchTarget = nullptr;

    // Capture old skill values for Multidex animation
    int oldSkillValues[8];
    if (interfaceIsSuperWide()) {
        for (int i = 0; i < 8; i++) {
            oldSkillValues[i] = skillGetValue(gDude, gMultidexSkillIds[i]);
        }
    }

    if (inventoryCommonInit() == -1) {
        return;
    }

    if (isInCombat()) {
        if (_inven_dude == gDude) {
            int actionPointsRequired = 4 - 2 * perkGetRank(_inven_dude, PERK_QUICK_POCKETS);
            if (actionPointsRequired > 0 && actionPointsRequired > gDude->data.critter.combat.ap) {
                MessageListItem messageListItem;
                messageListItem.num = 19; // You don't have enough action points to use inventory
                if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                    displayMonitorAddMessage(messageListItem.text);
                }

                // NOTE: Uninline.
                inventoryCommonFree();

                return;
            }

            if (actionPointsRequired > 0) {
                if (actionPointsRequired > gDude->data.critter.combat.ap) {
                    gDude->data.critter.combat.ap = 0;
                } else {
                    gDude->data.critter.combat.ap -= actionPointsRequired;
                }
                interfaceRenderActionPoints(gDude->data.critter.combat.ap, _combat_free_move);
            }
        }
    }

    Object* oldArmor = critterGetArmor(_inven_dude);
    bool isoWasEnabled = _setup_inventory(INVENTORY_WINDOW_TYPE_NORMAL);

    if (!settings.enhancements.strict_vanilla && settings.enhancements.companion_inventory) {
        inventoryBuildPartyList();
        gCurrentPartyIndex = 0;
        for (size_t i = 0; i < gPartyList.size(); i++) {
            if (gPartyList[i] == _inven_dude) {
                gCurrentPartyIndex = (int)i;
                break;
            }
        }
    } else {
        // Clear party list and set index to 0 (only player inventory)
        gPartyList.clear();
        gCurrentPartyIndex = 0;
    }

    reg_anim_clear(_inven_dude);
    inventoryRenderSummary();
    _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_NORMAL);
    inventorySetCursor(INVENTORY_WINDOW_CURSOR_HAND);

    // Run the main event loop
    int switched = inventoryRunLoop();

    _inven_dude = _stack[0];
    _adjust_fid();

    if (_inven_dude == gDude) {
        Rect rect;
        objectSetFid(_inven_dude, gInventoryWindowDudeFid, &rect);
        tileWindowRefreshRect(&rect, _inven_dude->elevation);
    }

    Object* newArmor = critterGetArmor(_inven_dude);
    if (_inven_dude == gDude) {
        if (oldArmor != newArmor) {
            interfaceRenderArmorClass(true);
        }
    }

    // Animate skill changes in the Multidex skilldex
    if (interfaceIsSuperWide()) {
        multidexRefreshSkilldexAnimated(oldSkillValues);
    }

    gFilterCategory = -1;

    _exit_inventory(isoWasEnabled);

    // NOTE: Uninline.
    inventoryCommonFree();

    if (_inven_dude == gDude) {
        interfaceUpdateItems(false, INTERFACE_ITEM_ACTION_DEFAULT, INTERFACE_ITEM_ACTION_DEFAULT);
    }
}

void inventoryOpenForCompanion(Object* critter)
{

    if (settings.enhancements.strict_vanilla || !settings.enhancements.companion_inventory) {
        return;
    }
    if (critter == nullptr) return;

    // Save original inven_dude and inven_pid (should be the player)
    Object* savedDude = _inven_dude;
    int savedPid = _inven_pid;

    gSwitchToCharacter = false;
    gSwitchTarget = nullptr;

    if (isInCombat()) {
        if (_combat_whose_turn() != critter) {
            return;
        }
    }

    ScopedGameMode gm(GameMode::kInventory);

    if (inventoryCommonInit() == -1) {
        return;
    }

    // Set the target critter
    _inven_dude = critter;
    _inven_pid = critter->pid;

    bool isoWasEnabled = _setup_inventory(INVENTORY_WINDOW_TYPE_NORMAL);

    // Build party list (player + alive companions)
    if (!settings.enhancements.strict_vanilla && settings.enhancements.companion_inventory) {
        inventoryBuildPartyList();
        gCurrentPartyIndex = 0;
        for (size_t i = 0; i < gPartyList.size(); i++) {
            if (gPartyList[i] == _inven_dude) {
                gCurrentPartyIndex = (int)i;
                break;
            }
        }
    } else {
        // Clear party list and set index to 0 (only player inventory)
        gPartyList.clear();
        gCurrentPartyIndex = 0;
    }

    reg_anim_clear(_inven_dude);
    inventoryRenderSummary();
    _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_NORMAL);
    inventorySetCursor(INVENTORY_WINDOW_CURSOR_HAND);

    // Run the main event loop
    int switched = inventoryRunLoop();

    _inven_dude = _stack[0]; // adjust companion FID before restoring player
    _adjust_fid();

    // Update companion's world model
    if (_inven_dude != nullptr) {
        Rect rect;
        objectSetFid(_inven_dude, gInventoryWindowDudeFid, &rect);
        tileWindowRefreshRect(&rect, _inven_dude->elevation);
    }

    _exit_inventory(isoWasEnabled);
    inventoryCommonFree();

    // Restore original inven_dude and inven_pid
    _inven_dude = savedDude;
    _inven_pid = savedPid;
}

// 0x46EC90
static bool _setup_inventory(int inventoryWindowType)
{
    _dropped_explosive = 0;
    _curr_stack = 0;
    _stack_offset[0] = 0;
    gInventorySlotsCount = 6; // original (for loot/trade/use-on)
    gCurrentInvWindowType = inventoryWindowType;

    // Setup common settings controlled elements
    inventoryUpdateLayout();

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
        gInventorySlotsCount = gInventoryRows * gInventoryColumns; // total visible slots
    }
    _pud = &(_inven_dude->data.inventory);
    _stack[0] = _inven_dude;
    int fid;

    // Load standard disabled arrows (same for all window types)
    fid = buildFid(OBJ_TYPE_INTERFACE, 53, 0, 0, 0);
    _inventoryFrmImages[4].lock(fid);

    fid = buildFid(OBJ_TYPE_INTERFACE, 54, 0, 0, 0);
    _inventoryFrmImages[7].lock(fid);

    if (inventoryWindowType <= INVENTORY_WINDOW_TYPE_LOOT) {

        const InventoryWindowDescription* windowDescription = &(gInventoryWindowDescriptions[inventoryWindowType]);

        // Determine actual window dimensions
        int actualWidth = windowDescription->width;
        int actualHeight = windowDescription->height;
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
            actualWidth = gLayout.windowWidth;
            actualHeight = gLayout.windowHeight;
        }

        // Maintain original position in original resolution and column, otherwise center it.
        int inventoryWindowX, inventoryWindowY;
        // Only use original hard?coded position if we have 1 column and screen is exactly 640x480
        if (gInventoryColumns == 1 && screenGetWidth() == 640 && screenGetHeight() == 480) {
            inventoryWindowX = INVENTORY_WINDOW_X;
            inventoryWindowY = INVENTORY_WINDOW_Y;
        } else {
            inventoryWindowX = (screenGetWidth() - actualWidth) / 2;
            inventoryWindowY = (screenGetHeight() - actualHeight) / 2;
        }

        gInventoryWindow = windowCreate(inventoryWindowX,
            inventoryWindowY,
            actualWidth,
            actualHeight,
            257,
            WINDOW_MODAL | WINDOW_MOVE_ON_TOP | WINDOW_TRANSPARENT);
        gInventoryWindowMaxX = actualWidth + inventoryWindowX;
        gInventoryWindowMaxY = actualHeight + inventoryWindowY;

        unsigned char* dest = windowGetBuffer(gInventoryWindow);

        FrmImage backgroundFrmImage;
        int backgroundFid;
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
            backgroundFid = buildFid(OBJ_TYPE_INTERFACE, gCurrentInventoryBackgroundFrm, 0, 0, 0);
        } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
            backgroundFid = buildFid(OBJ_TYPE_INTERFACE, gCurrentLootBackgroundFrm, 0, 0, 0);
        } else {
            backgroundFid = buildFid(OBJ_TYPE_INTERFACE, windowDescription->frmId, 0, 0, 0);
        }

        if (backgroundFrmImage.lock(backgroundFid)) {
            int srcWidth = backgroundFrmImage.getWidth();
            int srcHeight = backgroundFrmImage.getHeight();
            if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
                blitBufferToBuffer(backgroundFrmImage.getData(),
                    srcWidth, srcHeight, srcWidth,
                    dest, actualWidth);
            } else {
                // For others, use the original size
                blitBufferToBuffer(backgroundFrmImage.getData(),
                    windowDescription->width,
                    windowDescription->height,
                    windowDescription->width,
                    dest,
                    actualWidth);
            }
        }

        gInventoryPrintItemDescriptionHandler = displayMonitorAddMessage;
    } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        if (_barter_back_win == -1) {
            exit(1);
        }

        gInventorySlotsCount = 3;

        // Trade inventory window is a part of game dialog, which is 640x480.
        int tradeWindowX = (screenGetWidth() - INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH) / 2 + INVENTORY_TRADE_WINDOW_X;
        int tradeWindowY = (screenGetHeight() - INVENTORY_TRADE_BACKGROUND_WINDOW_HEIGHT) / 2 + INVENTORY_TRADE_WINDOW_Y;
        gInventoryWindow = windowCreate(tradeWindowX, tradeWindowY, INVENTORY_TRADE_WINDOW_WIDTH, INVENTORY_TRADE_WINDOW_HEIGHT, 257, 0);
        gInventoryWindowMaxX = tradeWindowX + INVENTORY_TRADE_WINDOW_WIDTH;
        gInventoryWindowMaxY = tradeWindowY + INVENTORY_TRADE_WINDOW_HEIGHT;

        unsigned char* dest = windowGetBuffer(gInventoryWindow);
        unsigned char* src = windowGetBuffer(_barter_back_win);
        blitBufferToBuffer(src + INVENTORY_TRADE_WINDOW_X, INVENTORY_TRADE_WINDOW_WIDTH, INVENTORY_TRADE_WINDOW_HEIGHT, INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH, dest, INVENTORY_TRADE_WINDOW_WIDTH);

        gInventoryPrintItemDescriptionHandler = gameDialogRenderSupplementaryMessage;
    }

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
        // Create invsibile buttons representing character's inventory item
        // slots.
        for (int index = 0; index < gInventorySlotsCount; index++) {
            int btn = buttonCreate(gInventoryWindow,
                INVENTORY_LOOT_LEFT_SCROLLER_X,
                gInventorySlotHeight * (gInventorySlotsCount - index - 1) + INVENTORY_LOOT_LEFT_SCROLLER_Y,
                INVENTORY_SLOT_WIDTH,
                gInventorySlotHeight,
                999 + gInventorySlotsCount - index,
                -1,
                999 + gInventorySlotsCount - index,
                -1,
                nullptr,
                nullptr,
                nullptr,
                0);
            if (btn != -1) {
                buttonSetMouseCallbacks(btn, inventoryItemSlotOnMouseEnter, inventoryItemSlotOnMouseExit, nullptr, nullptr);
            }
        }

        int eventCode = 2005;
        int y = gInventorySlotHeight * 5 + INVENTORY_LOOT_LEFT_SCROLLER_Y;

        // Create invisible buttons representing container's inventory item
        // slots. For unknown reason it loops backwards and it's size is
        // hardcoded at 6 items.
        //
        // Original code is slightly different. It loops until y reaches -11,
        // which is a bit awkward for a loop. Probably result of some
        // optimization.
        for (int index = 0; index < 6; index++) {
            int btn = buttonCreate(gInventoryWindow,
                INVENTORY_LOOT_RIGHT_SCROLLER_X,
                y,
                INVENTORY_SLOT_WIDTH,
                gInventorySlotHeight,
                eventCode,
                -1,
                eventCode,
                -1,
                nullptr,
                nullptr,
                nullptr,
                0);
            if (btn != -1) {
                buttonSetMouseCallbacks(btn, inventoryItemSlotOnMouseEnter, inventoryItemSlotOnMouseExit, nullptr, nullptr);
            }

            eventCode -= 1;
            y -= gInventorySlotHeight;
        }
        // Create filter buttons for left panel
        if (!settings.enhancements.strict_vanilla) {
            const int numCategories = 5;
            const int spacing = 6;
            int available = INVENTORY_SLOT_WIDTH - (numCategories - 1) * spacing;
            if (available > 0) {
                createFilterButtons(gInventoryWindow,
                    INVENTORY_LOOT_LEFT_SCROLLER_X,
                    INVENTORY_LOOT_LEFT_SCROLLER_Y + gInventorySlotsCount * gInventorySlotHeight + 2,
                    INVENTORY_SLOT_WIDTH,
                    KEYCODE_FILTER_BASE);
            }

            // Repeat for right panel
            available = INVENTORY_SLOT_WIDTH - (numCategories - 1) * spacing;
            if (available > 0) {
                createFilterButtons(gInventoryWindow,
                    INVENTORY_LOOT_RIGHT_SCROLLER_X,
                    INVENTORY_LOOT_RIGHT_SCROLLER_Y + gInventorySlotsCount * gInventorySlotHeight + 2,
                    INVENTORY_SLOT_WIDTH,
                    KEYCODE_FILTER_BASE);
            }
        }
    } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        int y1 = INVENTORY_TRADE_SCROLLER_Y;
        int y2 = INVENTORY_TRADE_INNER_SCROLLER_Y;

        for (int index = 0; index < gInventorySlotsCount; index++) {
            int btn;

            // Invsibile button representing left inventory slot.
            btn = buttonCreate(gInventoryWindow,
                INVENTORY_TRADE_LEFT_SCROLLER_X,
                y1,
                INVENTORY_SLOT_WIDTH,
                gInventorySlotHeight,
                KEYCODE_GRID_BASE + index,
                -1,
                KEYCODE_GRID_BASE + index,
                -1,
                nullptr,
                nullptr,
                nullptr,
                0);
            if (btn != -1) {
                buttonSetMouseCallbacks(btn, inventoryItemSlotOnMouseEnter, inventoryItemSlotOnMouseExit, nullptr, nullptr);
            }

            // Invisible button representing right inventory slot.
            btn = buttonCreate(gInventoryWindow,
                INVENTORY_TRADE_RIGHT_SCROLLER_X,
                y1,
                INVENTORY_SLOT_WIDTH,
                gInventorySlotHeight,
                KEYCODE_TARGET_GRID_BASE + index,
                -1,
                KEYCODE_TARGET_GRID_BASE + index,
                -1,
                nullptr,
                nullptr,
                nullptr,
                0);
            if (btn != -1) {
                buttonSetMouseCallbacks(btn, inventoryItemSlotOnMouseEnter, inventoryItemSlotOnMouseExit, nullptr, nullptr);
            }

            // Invisible button representing left suggested slot.
            btn = buttonCreate(gInventoryWindow,
                INVENTORY_TRADE_INNER_LEFT_SCROLLER_X,
                y2,
                INVENTORY_SLOT_WIDTH,
                gInventorySlotHeight,
                KEYCODE_OFFER_LEFT_BASE + index,
                -1,
                KEYCODE_OFFER_LEFT_BASE + index,
                -1,
                nullptr,
                nullptr,
                nullptr,
                0);
            if (btn != -1) {
                buttonSetMouseCallbacks(btn, inventoryItemSlotOnMouseEnter, inventoryItemSlotOnMouseExit, nullptr, nullptr);
            }

            // Invisible button representing right suggested slot.
            btn = buttonCreate(gInventoryWindow,
                INVENTORY_TRADE_INNER_RIGHT_SCROLLER_X,
                y2,
                INVENTORY_SLOT_WIDTH,
                gInventorySlotHeight,
                KEYCODE_OFFER_RIGHT_BASE + index,
                -1,
                KEYCODE_OFFER_RIGHT_BASE + index,
                -1,
                nullptr,
                nullptr,
                nullptr,
                0);
            if (btn != -1) {
                buttonSetMouseCallbacks(btn, inventoryItemSlotOnMouseEnter, inventoryItemSlotOnMouseExit, nullptr, nullptr);
            }

            y1 += gInventorySlotHeight;
            y2 += gInventorySlotHeight;
        }
        // Create filter category buttons for the outer inventories (left and right)
        if (!settings.enhancements.strict_vanilla) {
            int barY = INVENTORY_TRADE_SCROLLER_Y + gInventorySlotsCount * gInventorySlotHeight;
            createFilterButtons(gInventoryWindow,
                INVENTORY_TRADE_LEFT_SCROLLER_X,
                barY,
                INVENTORY_SLOT_WIDTH,
                KEYCODE_FILTER_BASE);
            createFilterButtons(gInventoryWindow,
                INVENTORY_TRADE_RIGHT_SCROLLER_X,
                barY,
                INVENTORY_SLOT_WIDTH,
                KEYCODE_FILTER_BASE);
        }
    } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
        // Multi-column grid for normal inventory
        int keyCodeBase = KEYCODE_GRID_BASE;
        for (int row = 0; row < gInventoryRows; ++row) {
            for (int col = 0; col < gInventoryColumns; ++col) {
                int x = gLayout.scrollerX + col * gLayout.slotWidth;
                int y = gLayout.scrollerY + row * gLayout.slotHeight;
                int keyCode = keyCodeBase + row * gInventoryColumns + col;
                int btn = buttonCreate(gInventoryWindow,
                    x, y,
                    gLayout.slotWidth, gLayout.slotHeight,
                    keyCode, -1, keyCode, -1,
                    nullptr, nullptr, nullptr, 0);
                if (btn != -1) {
                    buttonSetMouseCallbacks(btn, inventoryItemSlotOnMouseEnter, inventoryItemSlotOnMouseExit, nullptr, nullptr);
                }
            }
        }
        // Create filter category buttons
        int oldFont = fontGetCurrent();
        fontSetCurrent(100);
        createFilterButtons(gInventoryWindow,
            gLayout.scrollerX,
            gLayout.scrollerY + gInventoryRows * gLayout.slotHeight + 2,
            gLayout.scrollerWidth,
            KEYCODE_FILTER_BASE);
        fontSetCurrent(oldFont);
    } else {
        // For use-on (INVENTORY_WINDOW_TYPE_USE_ITEM_ON) and any other, use original single column
        for (int index = 0; index < gInventorySlotsCount; index++) {
            int btn = buttonCreate(gInventoryWindow,
                INVENTORY_SCROLLER_X,
                gInventorySlotHeight * (gInventorySlotsCount - index - 1) + INVENTORY_SCROLLER_Y,
                INVENTORY_SLOT_WIDTH,
                gInventorySlotHeight,
                999 + gInventorySlotsCount - index,
                -1,
                999 + gInventorySlotsCount - index,
                -1,
                nullptr,
                nullptr,
                nullptr,
                0);
            if (btn != -1) {
                buttonSetMouseCallbacks(btn, inventoryItemSlotOnMouseEnter, inventoryItemSlotOnMouseExit, nullptr, nullptr);
            }
        }
        // Add filter buttons (if not strict vanilla)
        if (!settings.enhancements.strict_vanilla) {
            int barY = INVENTORY_SCROLLER_Y + gInventorySlotsCount * gInventorySlotHeight;
            createFilterButtons(gInventoryWindow,
                INVENTORY_SCROLLER_X,
                barY,
                INVENTORY_SLOT_WIDTH,
                KEYCODE_FILTER_BASE);
        }
    }

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {

        // Right hand slot
        gRightHandSlotButton = buttonCreate(gInventoryWindow,
            gLayout.rightHandSlotX, gLayout.rightHandSlotY,
            INVENTORY_LARGE_SLOT_WIDTH, INVENTORY_LARGE_SLOT_HEIGHT,
            INVENTORY_HAND_RIGHT_KEY, -1, INVENTORY_HAND_RIGHT_KEY, -1,
            nullptr, nullptr, nullptr, 0);
        if (gRightHandSlotButton != -1) {
            buttonSetMouseCallbacks(gRightHandSlotButton, inventoryItemSlotOnMouseEnter, inventoryItemSlotOnMouseExit, nullptr, nullptr);
        }

        // Left hand slot
        gLeftHandSlotButton = buttonCreate(gInventoryWindow,
            gLayout.leftHandSlotX, gLayout.leftHandSlotY,
            INVENTORY_LARGE_SLOT_WIDTH, INVENTORY_LARGE_SLOT_HEIGHT,
            INVENTORY_HAND_LEFT_KEY, -1, INVENTORY_HAND_LEFT_KEY, -1,
            nullptr, nullptr, nullptr, 0);
        if (gLeftHandSlotButton != -1) {
            buttonSetMouseCallbacks(gLeftHandSlotButton, inventoryItemSlotOnMouseEnter, inventoryItemSlotOnMouseExit, nullptr, nullptr);
        }

        // Armor slot
        gArmorSlotButton = buttonCreate(gInventoryWindow,
            gLayout.armorSlotX, gLayout.armorSlotY,
            INVENTORY_LARGE_SLOT_WIDTH, INVENTORY_LARGE_SLOT_HEIGHT,
            INVENTORY_ARMOR_KEY, -1, INVENTORY_ARMOR_KEY, -1,
            nullptr, nullptr, nullptr, 0);
        if (gArmorSlotButton != -1) {
            buttonSetMouseCallbacks(gArmorSlotButton, inventoryItemSlotOnMouseEnter, inventoryItemSlotOnMouseExit, nullptr, nullptr);
        }

        if (!gGreySlotFrm.isLocked()) {
            int fid = buildFid(OBJ_TYPE_INTERFACE, 74, 0, 0, 0); // small slot grey
            if (!gGreySlotFrm.lock(fid)) {
                debugPrint("Failed to load grey slot FRM 74\n");
            } else {
                debugPrint("Grey slot FRM 74 loaded: %dx%d\n", gGreySlotFrm.getWidth(), gGreySlotFrm.getHeight());
            }
        }
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
            if (!partyMemberCanEquipArmor(_inven_dude) && gArmorSlotButton != -1) {
                buttonDisable(gArmorSlotButton);
            }
            if (!partyMemberCanEquipWeapon(_inven_dude)) {
                if (gRightHandSlotButton != -1) buttonDisable(gRightHandSlotButton);
                if (gLeftHandSlotButton != -1) buttonDisable(gLeftHandSlotButton);
            }
        }
    }

    int btn;
    int btnX;
    int btnY;

    fid = buildFid(OBJ_TYPE_INTERFACE, 96, 0, 0, 0);
    _inventoryFrmImages[0].lock(fid);

    fid = buildFid(OBJ_TYPE_INTERFACE, 95, 0, 0, 0);
    _inventoryFrmImages[1].lock(fid);

    if (_inventoryFrmImages[0].isLocked() && _inventoryFrmImages[1].isLocked()) {
        btn = -1;
        switch (inventoryWindowType) {
        case INVENTORY_WINDOW_TYPE_NORMAL:
            // Done button
            btn = buttonCreate(gInventoryWindow,
                gLayout.doneButtonX,
                gLayout.doneButtonY,
                14,
                14,
                -1,
                -1,
                -1,
                KEY_ESCAPE,
                _inventoryFrmImages[0].getData(),
                _inventoryFrmImages[1].getData(),
                nullptr,
                BUTTON_FLAG_TRANSPARENT);
            break;
        case INVENTORY_WINDOW_TYPE_USE_ITEM_ON:
            // Cancel button
            btn = buttonCreate(gInventoryWindow,
                234,
                329,
                14,
                14,
                -1,
                -1,
                -1,
                KEY_ESCAPE,
                _inventoryFrmImages[0].getData(),
                _inventoryFrmImages[1].getData(),
                nullptr,
                BUTTON_FLAG_TRANSPARENT);
            break;
        case INVENTORY_WINDOW_TYPE_LOOT:
            // Done button
            btn = buttonCreate(gInventoryWindow,
                476,
                331,
                14,
                14,
                -1,
                -1,
                -1,
                KEY_ESCAPE,
                _inventoryFrmImages[0].getData(),
                _inventoryFrmImages[1].getData(),
                nullptr,
                BUTTON_FLAG_TRANSPARENT);
            break;
        }

        if (btn != -1) {
            buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
        }
    }

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        // Large arrow up (normal).
        fid = buildFid(OBJ_TYPE_INTERFACE, 100, 0, 0, 0);
        _inventoryFrmImages[2].lock(fid);

        // Large arrow up (pressed).
        fid = buildFid(OBJ_TYPE_INTERFACE, 101, 0, 0, 0);
        _inventoryFrmImages[3].lock(fid);

        // Load disabled up arrow (same as normal inventory)
        fid = buildFid(OBJ_TYPE_INTERFACE, 7168, 0, 0, 0);
        _inventoryFrmImages[14].lock(fid);

        if (_inventoryFrmImages[2].isLocked() && _inventoryFrmImages[3].isLocked() && _inventoryFrmImages[14].isLocked()) {
            // Left inventory up button (player)
            btn = buttonCreate(gInventoryWindow,
                111, 57, 22, 23,
                -1, -1, KEY_ARROW_UP, -1,
                _inventoryFrmImages[2].getData(),
                _inventoryFrmImages[3].getData(),
                nullptr, 0);
            if (btn != -1) {
                buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
                _win_register_button_disable(btn,
                    _inventoryFrmImages[14].getData(),
                    _inventoryFrmImages[14].getData(),
                    _inventoryFrmImages[14].getData());
                gTradeLeftUpButton = btn;
                buttonDisable(btn);
            }

            // Right inventory up button (merchant/NPC/companion)
            btn = buttonCreate(gInventoryWindow,
                342, 57, 22, 23,
                -1, -1, KEY_CTRL_ARROW_UP, -1,
                _inventoryFrmImages[2].getData(),
                _inventoryFrmImages[3].getData(),
                nullptr, 0);
            if (btn != -1) {
                buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
                _win_register_button_disable(btn,
                    _inventoryFrmImages[14].getData(),
                    _inventoryFrmImages[14].getData(),
                    _inventoryFrmImages[14].getData());
                gTradeRightUpButton = btn;
                buttonDisable(btn);
            }
        }
    } else {
        // Large up arrow (normal).
        fid = buildFid(OBJ_TYPE_INTERFACE, 49, 0, 0, 0);
        _inventoryFrmImages[2].lock(fid);

        // Large up arrow (pressed).
        fid = buildFid(OBJ_TYPE_INTERFACE, 50, 0, 0, 0);
        _inventoryFrmImages[3].lock(fid);

        if (_inventoryFrmImages[2].isLocked() && _inventoryFrmImages[3].isLocked() && _inventoryFrmImages[4].isLocked()) {
            if (inventoryWindowType != INVENTORY_WINDOW_TYPE_TRADE) {
                if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
                    btnX = gLayout.scrollUpButtonX;
                    btnY = gLayout.scrollUpButtonY;
                } else {
                    btnX = 128; // original X for loot/use-on
                    btnY = 39;
                }
                gInventoryScrollUpButton = buttonCreate(gInventoryWindow,
                    btnX,
                    gLayout.scrollUpButtonY, // Y is 39 for normal and loot
                    22,
                    23,
                    -1,
                    -1,
                    KEY_ARROW_UP,
                    -1,
                    _inventoryFrmImages[2].getData(),
                    _inventoryFrmImages[3].getData(),
                    nullptr,
                    0);
                if (gInventoryScrollUpButton != -1) {
                    _win_register_button_disable(gInventoryScrollUpButton, _inventoryFrmImages[4].getData(), _inventoryFrmImages[4].getData(), _inventoryFrmImages[4].getData());
                    buttonSetCallbacks(gInventoryScrollUpButton, _gsound_red_butt_press, _gsound_red_butt_release);
                    buttonDisable(gInventoryScrollUpButton);
                }
            }

            if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
                // Right inventory up button.
                gSecondaryInventoryScrollUpButton = buttonCreate(gInventoryWindow,
                    379,
                    39,
                    22,
                    23,
                    -1,
                    -1,
                    KEY_CTRL_ARROW_UP,
                    -1,
                    _inventoryFrmImages[2].getData(),
                    _inventoryFrmImages[3].getData(),
                    nullptr,
                    0);
                if (gSecondaryInventoryScrollUpButton != -1) {
                    _win_register_button_disable(gSecondaryInventoryScrollUpButton, _inventoryFrmImages[4].getData(), _inventoryFrmImages[4].getData(), _inventoryFrmImages[4].getData());
                    buttonSetCallbacks(gSecondaryInventoryScrollUpButton, _gsound_red_butt_press, _gsound_red_butt_release);
                    buttonDisable(gSecondaryInventoryScrollUpButton);
                }
            }
        }
    }

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        // Large dialog down button (normal)
        fid = buildFid(OBJ_TYPE_INTERFACE, 93, 0, 0, 0);
        _inventoryFrmImages[5].lock(fid);

        // Dialog down button (pressed)
        fid = buildFid(OBJ_TYPE_INTERFACE, 94, 0, 0, 0);
        _inventoryFrmImages[6].lock(fid);

        // Load large disabled down arrow
        fid = buildFid(OBJ_TYPE_INTERFACE, 5872, 0, 0, 0);
        _inventoryFrmImages[15].lock(fid);

        if (_inventoryFrmImages[5].isLocked() && _inventoryFrmImages[6].isLocked() && _inventoryFrmImages[15].isLocked()) {
            // Left inventory down button (player)
            btn = buttonCreate(gInventoryWindow,
                111, 82, 22, 23,
                -1, -1, KEY_ARROW_DOWN, -1,
                _inventoryFrmImages[5].getData(),
                _inventoryFrmImages[6].getData(),
                nullptr, 0);
            if (btn != -1) {
                buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
                _win_register_button_disable(btn,
                    _inventoryFrmImages[15].getData(),
                    _inventoryFrmImages[15].getData(),
                    _inventoryFrmImages[15].getData());
                gTradeLeftDownButton = btn;
                buttonDisable(btn); // initially disabled if list short
            }

            // Right inventory down button (merchant/NPC/companion)
            btn = buttonCreate(gInventoryWindow,
                342, 82, 22, 23,
                -1, -1, KEY_CTRL_ARROW_DOWN, -1,
                _inventoryFrmImages[5].getData(),
                _inventoryFrmImages[6].getData(),
                nullptr, 0);
            if (btn != -1) {
                buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
                _win_register_button_disable(btn,
                    _inventoryFrmImages[15].getData(),
                    _inventoryFrmImages[15].getData(),
                    _inventoryFrmImages[15].getData());
                gTradeRightDownButton = btn;
                buttonDisable(btn);
            }

            // Invisible button representing left character.
            int tradeLeftPortraitBtn = buttonCreate(_barter_back_win,
                15,
                25,
                INVENTORY_BODY_VIEW_WIDTH,
                INVENTORY_BODY_VIEW_HEIGHT,
                INVENTORY_BUTTON_LEFT,
                -1,
                INVENTORY_BUTTON_LEFT,
                -1,
                nullptr,
                nullptr,
                nullptr,
                0);
            if (tradeLeftPortraitBtn != -1) {
                buttonSetMouseCallbacks(tradeLeftPortraitBtn, inventoryPortraitOnMouseEnter,
                    inventoryPortraitOnMouseExit, nullptr, nullptr);
            }

            // Invisible button representing right character.
            int tradeRightPortraitBtn = buttonCreate(_barter_back_win,
                560,
                25,
                INVENTORY_BODY_VIEW_WIDTH,
                INVENTORY_BODY_VIEW_HEIGHT,
                INVENTORY_BUTTON_RIGHT,
                -1,
                INVENTORY_BUTTON_RIGHT,
                -1,
                nullptr,
                nullptr,
                nullptr,
                0);
            if (tradeRightPortraitBtn != -1) {
                buttonSetMouseCallbacks(tradeRightPortraitBtn, inventoryPortraitOnMouseEnter,
                    inventoryPortraitOnMouseExit, nullptr, nullptr);
            }
        }
    } else {
        // Large arrow down (normal).
        fid = buildFid(OBJ_TYPE_INTERFACE, 51, 0, 0, 0);
        _inventoryFrmImages[5].lock(fid);

        // Large arrow down (pressed).
        fid = buildFid(OBJ_TYPE_INTERFACE, 52, 0, 0, 0);
        _inventoryFrmImages[6].lock(fid);

        if (_inventoryFrmImages[5].isLocked() && _inventoryFrmImages[6].isLocked() && _inventoryFrmImages[7].isLocked()) {
            // Left inventory down button.
            gInventoryScrollDownButton = buttonCreate(gInventoryWindow,
                btnX,
                gLayout.scrollDownButtonY, // Y is 62 for normal and loot
                22,
                23,
                -1,
                -1,
                KEY_ARROW_DOWN,
                -1,
                _inventoryFrmImages[5].getData(),
                _inventoryFrmImages[6].getData(),
                nullptr,
                0);
            buttonSetCallbacks(gInventoryScrollDownButton, _gsound_red_butt_press, _gsound_red_butt_release);
            _win_register_button_disable(gInventoryScrollDownButton, _inventoryFrmImages[7].getData(), _inventoryFrmImages[7].getData(), _inventoryFrmImages[7].getData());
            buttonDisable(gInventoryScrollDownButton);

            if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
                // Invisible button representing left character.
                int leftPortraitBtn = buttonCreate(gInventoryWindow,
                    INVENTORY_LOOT_LEFT_BODY_VIEW_X,
                    INVENTORY_LOOT_LEFT_BODY_VIEW_Y,
                    INVENTORY_BODY_VIEW_WIDTH,
                    INVENTORY_BODY_VIEW_HEIGHT,
                    INVENTORY_BUTTON_LEFT,
                    -1,
                    INVENTORY_BUTTON_LEFT,
                    -1,
                    nullptr,
                    nullptr,
                    nullptr,
                    0);
                if (leftPortraitBtn != -1) {
                    buttonSetMouseCallbacks(leftPortraitBtn, inventoryPortraitOnMouseEnter,
                        inventoryPortraitOnMouseExit, nullptr, nullptr);
                }

                // Right inventory down button.
                gSecondaryInventoryScrollDownButton = buttonCreate(gInventoryWindow,
                    379,
                    62,
                    22,
                    23,
                    -1,
                    -1,
                    KEY_CTRL_ARROW_DOWN,
                    -1,
                    _inventoryFrmImages[5].getData(),
                    _inventoryFrmImages[6].getData(),
                    nullptr,
                    0);
                if (gSecondaryInventoryScrollDownButton != -1) {
                    buttonSetCallbacks(gSecondaryInventoryScrollDownButton, _gsound_red_butt_press, _gsound_red_butt_release);
                    _win_register_button_disable(gSecondaryInventoryScrollDownButton, _inventoryFrmImages[7].getData(), _inventoryFrmImages[7].getData(), _inventoryFrmImages[7].getData());
                    buttonDisable(gSecondaryInventoryScrollDownButton);
                }

                // Invisible button representing right character.
                int rightPortraitBtn = buttonCreate(gInventoryWindow,
                    INVENTORY_LOOT_RIGHT_BODY_VIEW_X,
                    INVENTORY_LOOT_RIGHT_BODY_VIEW_Y,
                    INVENTORY_BODY_VIEW_WIDTH,
                    INVENTORY_BODY_VIEW_HEIGHT,
                    INVENTORY_BUTTON_RIGHT,
                    -1,
                    INVENTORY_BUTTON_RIGHT,
                    -1,
                    nullptr,
                    nullptr,
                    nullptr,
                    0);
                if (rightPortraitBtn != -1) {
                    buttonSetMouseCallbacks(rightPortraitBtn, inventoryPortraitOnMouseEnter,
                        inventoryPortraitOnMouseExit, nullptr, nullptr);
                }
            } else {
                if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
                    int portraitBtn = buttonCreate(gInventoryWindow,
                        gLayout.bodyViewX,
                        gLayout.bodyViewY,
                        INVENTORY_BODY_VIEW_WIDTH,
                        INVENTORY_BODY_VIEW_HEIGHT,
                        INVENTORY_BUTTON_LEFT,
                        -1,
                        INVENTORY_BUTTON_LEFT,
                        -1,
                        nullptr,
                        nullptr,
                        nullptr,
                        0);
                    if (portraitBtn != -1) {
                        buttonSetMouseCallbacks(portraitBtn, inventoryPortraitOnMouseEnter,
                            inventoryPortraitOnMouseExit, nullptr, nullptr);
                    }
                } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_USE_ITEM_ON) {
                    int portraitBtn = buttonCreate(gInventoryWindow,
                        INVENTORY_PC_BODY_VIEW_X, // original X
                        INVENTORY_PC_BODY_VIEW_Y,
                        INVENTORY_BODY_VIEW_WIDTH,
                        INVENTORY_BODY_VIEW_HEIGHT,
                        INVENTORY_BUTTON_LEFT,
                        -1,
                        INVENTORY_BUTTON_LEFT,
                        -1,
                        nullptr,
                        nullptr,
                        nullptr,
                        0);
                    if (portraitBtn != -1) {
                        buttonSetMouseCallbacks(portraitBtn, inventoryPortraitOnMouseEnter,
                            inventoryPortraitOnMouseExit, nullptr, nullptr);
                    }
                }
            }
        }
    }

    if (inventoryWindowType != INVENTORY_WINDOW_TYPE_TRADE) {
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
            if (!_gIsSteal) {
                // Take all button (normal)
                fid = buildFid(OBJ_TYPE_INTERFACE, 436, 0, 0, 0);
                _inventoryFrmImages[8].lock(fid);

                // Take all button (pressed)
                fid = buildFid(OBJ_TYPE_INTERFACE, 437, 0, 0, 0);
                _inventoryFrmImages[9].lock(fid);

                if (_inventoryFrmImages[8].isLocked() && _inventoryFrmImages[9].isLocked()) {
                    // Take all button.
                    btn = buttonCreate(gInventoryWindow,
                        432,
                        204,
                        39,
                        41,
                        -1,
                        -1,
                        INVENTORY_BUTTON_TAKE_ALL,
                        -1,
                        _inventoryFrmImages[8].getData(),
                        _inventoryFrmImages[9].getData(),
                        nullptr,
                        0);
                    if (btn != -1) {
                        buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
                    }
                }

                if (!settings.enhancements.strict_vanilla) {
                    // Drop All button (up)
                    fid = buildFid(OBJ_TYPE_INTERFACE, 4913, 0, 0, 0);
                    _inventoryFrmImages[12].lock(fid);

                    // Drop All button (down)
                    fid = buildFid(OBJ_TYPE_INTERFACE, 4299, 0, 0, 0);
                    _inventoryFrmImages[13].lock(fid);

                    if (_inventoryFrmImages[12].isLocked() && _inventoryFrmImages[13].isLocked()) {
                        // Drop All button
                        btn = buttonCreate(gInventoryWindow,
                            70, // x position
                            204, // y position (same as Take All)
                            39,
                            41,
                            -1,
                            -1,
                            INVENTORY_BUTTON_DROP_ALL,
                            -1,
                            _inventoryFrmImages[12].getData(),
                            _inventoryFrmImages[13].getData(),
                            nullptr,
                            0);
                        if (btn != -1) {
                            buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
                        }
                    }
                }
            }
        }
    } else {
        // Inventory button up (normal)
        fid = buildFid(OBJ_TYPE_INTERFACE, 49, 0, 0, 0);
        _inventoryFrmImages[8].lock(fid);

        // Inventory button up (pressed)
        fid = buildFid(OBJ_TYPE_INTERFACE, 50, 0, 0, 0);
        _inventoryFrmImages[9].lock(fid);

        if (_inventoryFrmImages[8].isLocked() && _inventoryFrmImages[9].isLocked()) {
            // Left offered inventory up button.
            btn = buttonCreate(gInventoryWindow,
                118, 113, 22, 23,
                -1, -1, KEY_PAGE_UP, -1,
                _inventoryFrmImages[8].getData(),
                _inventoryFrmImages[9].getData(),
                nullptr, 0);
            if (btn != -1) {
                buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
                // Add disabled state
                _win_register_button_disable(btn,
                    _inventoryFrmImages[4].getData(),
                    _inventoryFrmImages[4].getData(),
                    _inventoryFrmImages[4].getData());
                gTradeOfferLeftUpButton = btn;
                buttonDisable(btn);
            }

            // Right offered inventory up button.
            btn = buttonCreate(gInventoryWindow,
                336, 113, 22, 23,
                -1, -1, KEY_CTRL_PAGE_UP, -1,
                _inventoryFrmImages[8].getData(),
                _inventoryFrmImages[9].getData(),
                nullptr, 0);
            if (btn != -1) {
                buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
                _win_register_button_disable(btn,
                    _inventoryFrmImages[4].getData(),
                    _inventoryFrmImages[4].getData(),
                    _inventoryFrmImages[4].getData());
                gTradeOfferRightUpButton = btn;
                buttonDisable(btn);
            }
        }

        // Inventory button down (normal)
        fid = buildFid(OBJ_TYPE_INTERFACE, 51, 0, 0, 0);
        _inventoryFrmImages[10].lock(fid);

        // Inventory button down (pressed).
        fid = buildFid(OBJ_TYPE_INTERFACE, 52, 0, 0, 0);
        _inventoryFrmImages[11].lock(fid);

        if (_inventoryFrmImages[10].isLocked() && _inventoryFrmImages[11].isLocked()) {
            // Left offered inventory down button.
            btn = buttonCreate(gInventoryWindow,
                118, 136, 22, 23,
                -1, -1, KEY_PAGE_DOWN, -1,
                _inventoryFrmImages[10].getData(),
                _inventoryFrmImages[11].getData(),
                nullptr, 0);
            if (btn != -1) {
                buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
                _win_register_button_disable(btn,
                    _inventoryFrmImages[7].getData(),
                    _inventoryFrmImages[7].getData(),
                    _inventoryFrmImages[7].getData());
                gTradeOfferLeftDownButton = btn;
                buttonDisable(btn);
            }

            // Right offered inventory down button.
            btn = buttonCreate(gInventoryWindow,
                336, 136, 22, 23,
                -1, -1, KEY_CTRL_PAGE_DOWN, -1,
                _inventoryFrmImages[10].getData(),
                _inventoryFrmImages[11].getData(),
                nullptr, 0);
            if (btn != -1) {
                buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
                _win_register_button_disable(btn,
                    _inventoryFrmImages[7].getData(),
                    _inventoryFrmImages[7].getData(),
                    _inventoryFrmImages[7].getData());
                gTradeOfferRightDownButton = btn;
                buttonDisable(btn);
            }
        }
    }

    gInventoryRightHandItem = nullptr;
    gInventoryArmor = nullptr;
    gInventoryLeftHandItem = nullptr;

    for (int index = 0; index < _pud->length; index++) {
        InventoryItem* inventoryItem = &(_pud->items[index]);
        Object* item = inventoryItem->item;
        if ((item->flags & OBJECT_IN_LEFT_HAND) != 0) {
            if ((item->flags & OBJECT_IN_RIGHT_HAND) != 0) {
                gInventoryRightHandItem = item;
            }
            gInventoryLeftHandItem = item;
        } else if ((item->flags & OBJECT_IN_RIGHT_HAND) != 0) {
            gInventoryRightHandItem = item;
        } else if ((item->flags & OBJECT_WORN) != 0) {
            gInventoryArmor = item;
        }
    }

    if (gInventoryLeftHandItem != nullptr) {
        itemRemove(_inven_dude, gInventoryLeftHandItem, 1);
    }

    if (gInventoryRightHandItem != nullptr && gInventoryRightHandItem != gInventoryLeftHandItem) {
        itemRemove(_inven_dude, gInventoryRightHandItem, 1);
    }

    if (gInventoryArmor != nullptr) {
        itemRemove(_inven_dude, gInventoryArmor, 1);
    }

    _adjust_fid();

    // Combined inventory setup
    gUseCombinedInventory = false;
    gCombinedExcludeObject = nullptr; // Excludes the target inventory (companions) from Combined Inventory

    if (!isInCombat()) {
        if (!settings.enhancements.strict_vanilla && settings.enhancements.companion_inventory) {
            if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL || inventoryWindowType == INVENTORY_WINDOW_TYPE_USE_ITEM_ON) {
                gUseCombinedInventory = true;
                gCombinedExcludeObject = nullptr;
                inventoryBuildCombinedList(_inven_dude);
            } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
                gUseCombinedInventory = true;
                gCombinedExcludeObject = _target_stack[_target_curr_stack];
                inventoryBuildCombinedList(gDude);
            } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
                gUseCombinedInventory = true;
                gCombinedExcludeObject = _target_stack[_target_curr_stack];
                inventoryBuildCombinedList(gDude);
                // Move player's money to the top (end of array) for convenience
                movePlayerMoneyToTopCombined();
            }
        } else {
            gUseCombinedInventory = false;
            gCombinedExcludeObject = nullptr;
            if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
                // Vanilla behavior: still move money to top in trade windows
                _move_money_to_top(_pud, _pud->length);
            }
        }
    }

    bool isoWasEnabled = isoDisable();

    _gmouse_disable(0);
    touch_set_touchscreen_mode(true);
    touch_set_pan_mode(true);

    return isoWasEnabled;
}

// 0x46FBD8
static void _exit_inventory(bool shouldEnableIso)
{
    _inven_dude = _stack[0];

    if (_inven_sort_menu_active) {
        if (_inven_sort_menu_button != -1) {
            buttonDestroy(_inven_sort_menu_button);
            _inven_sort_menu_button = -1;
        }
        _inven_sort_menu_active = false;
    }

    if (gInventoryLeftHandItem != nullptr) {
        gInventoryLeftHandItem->flags |= OBJECT_IN_LEFT_HAND;
        if (gInventoryLeftHandItem == gInventoryRightHandItem) {
            gInventoryLeftHandItem->flags |= OBJECT_IN_RIGHT_HAND;
        }

        itemAdd(_inven_dude, gInventoryLeftHandItem, 1);
    }

    if (gInventoryRightHandItem != nullptr && gInventoryRightHandItem != gInventoryLeftHandItem) {
        gInventoryRightHandItem->flags |= OBJECT_IN_RIGHT_HAND;
        itemAdd(_inven_dude, gInventoryRightHandItem, 1);
    }

    if (gInventoryArmor != nullptr) {
        gInventoryArmor->flags |= OBJECT_WORN;
        itemAdd(_inven_dude, gInventoryArmor, 1);
    }

    gInventoryRightHandItem = nullptr;
    gInventoryArmor = nullptr;
    gInventoryLeftHandItem = nullptr;
    gGreySlotFrm.unlock();

    for (int index = 0; index < INVENTORY_FRM_COUNT; index++) {
        _inventoryFrmImages[index].unlock();
    }

    if (shouldEnableIso) {
        isoEnable();
    }

    windowDestroy(gInventoryWindow);

    _gmouse_enable();
    touch_set_touchscreen_mode(false);
    touch_set_pan_mode(false);

    if (_dropped_explosive) {
        Attack attack;
        attackInit(&attack, gDude, nullptr, HIT_MODE_PUNCH, HIT_LOCATION_TORSO);
        attack.attackerFlags = DAM_HIT;
        attack.tile = gDude->tile;
        _compute_explosion_on_extras(&attack, 0, 0, 1);

        Object* watcher = nullptr;
        for (int index = 0; index < attack.extrasLength; index++) {
            Object* critter = attack.extras[index];
            if (critter != gDude
                && critter->data.critter.combat.team != gDude->data.critter.combat.team
                && statRoll(critter, STAT_PERCEPTION, 0, nullptr) >= ROLL_SUCCESS) {
                critterSetWhoHitMe(critter, gDude);

                if (watcher == nullptr) {
                    watcher = critter;
                }
            }
        }

        if (watcher != nullptr) {
            if (!isInCombat()) {
                CombatStartData combat;
                combat.attacker = watcher;
                combat.defender = gDude;
                combat.actionPointsBonus = 0;
                combat.accuracyBonus = 0;
                combat.damageBonus = 0;
                combat.minDamage = 0;
                combat.maxDamage = INT_MAX;
                combat.overrideAttackResults = 0;
                scriptsRequestCombat(&combat);
            }
        }

        gCombinedExcludeObject = nullptr;
        _dropped_explosive = false;
    }
    gIsTradeWindow = false;
}

// 0x46FDF4
static void _display_inventory(int stackOffset, int dragSlotIndex, int inventoryWindowType)
{
    unsigned char* windowBuffer = windowGetBuffer(gInventoryWindow);
    int pitch;
    gCurrentInvWindowType = inventoryWindowType;

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
        pitch = gLayout.windowWidth;
        int srcPitch = gLayout.windowWidth;
        int shift = (gInventoryColumns - 1) * gLayout.slotWidth;

        FrmImage backgroundFrmImage;
        int backgroundFid = buildFid(OBJ_TYPE_INTERFACE, gCurrentInventoryBackgroundFrm, 0, 0, 0);
        if (backgroundFrmImage.lock(backgroundFid)) {
            // Clear scroll view background (whole grid area) - no shift here because the grid stays at original X.
            unsigned char* src = backgroundFrmImage.getData() + srcPitch * gLayout.scrollerY + gLayout.scrollerX;
            unsigned char* dest = windowBuffer + pitch * gLayout.scrollerY + gLayout.scrollerX;
            blitBufferToBuffer(src, gLayout.scrollerWidth, gLayout.scrollerHeight, srcPitch, dest, pitch);

            // Clear armor button background (shift both source and destination)
            src = backgroundFrmImage.getData() + srcPitch * INVENTORY_ARMOR_SLOT_Y + (INVENTORY_ARMOR_SLOT_X + shift);
            dest = windowBuffer + pitch * gLayout.armorSlotY + gLayout.armorSlotX;
            blitBufferToBuffer(src, INVENTORY_LARGE_SLOT_WIDTH, INVENTORY_LARGE_SLOT_HEIGHT, srcPitch, dest, pitch);

            if (gInventoryLeftHandItem != nullptr && gInventoryLeftHandItem == gInventoryRightHandItem) {
                // Clear item1 (uses a different background?)
                FrmImage itemBackgroundFrmImage;
                int itemBackgroundFid = buildFid(OBJ_TYPE_INTERFACE, 32, 0, 0, 0);
                if (itemBackgroundFrmImage.lock(itemBackgroundFid)) {
                    int itemSrcPitch = itemBackgroundFrmImage.getWidth();
                    src = itemBackgroundFrmImage.getData() + itemSrcPitch * INVENTORY_LEFT_HAND_SLOT_Y + (INVENTORY_LEFT_HAND_SLOT_X + shift);
                    dest = windowBuffer + pitch * gLayout.leftHandSlotY + gLayout.leftHandSlotX;
                    blitBufferToBuffer(src, itemSrcPitch, itemBackgroundFrmImage.getHeight(), itemSrcPitch, dest, pitch);
                }
            } else {
                // Clear both items in one go (shift both source and destination)
                src = backgroundFrmImage.getData() + srcPitch * INVENTORY_LEFT_HAND_SLOT_Y + (INVENTORY_LEFT_HAND_SLOT_X + shift);
                dest = windowBuffer + pitch * gLayout.leftHandSlotY + gLayout.leftHandSlotX;
                blitBufferToBuffer(src, INVENTORY_LARGE_SLOT_WIDTH * 2, INVENTORY_LARGE_SLOT_HEIGHT, srcPitch, dest, pitch);
            }
        }
    } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_USE_ITEM_ON) {
        pitch = INVENTORY_USE_ON_WINDOW_WIDTH;

        FrmImage backgroundFrmImage;
        int backgroundFid = buildFid(OBJ_TYPE_INTERFACE, 113, 0, 0, 0);
        if (backgroundFrmImage.lock(backgroundFid)) {
            // Clear scroll view background.
            blitBufferToBuffer(backgroundFrmImage.getData() + pitch * INVENTORY_SCROLLER_Y + INVENTORY_SCROLLER_X,
                INVENTORY_SLOT_WIDTH,
                gInventorySlotsCount * gInventorySlotHeight,
                pitch,
                windowBuffer + pitch * INVENTORY_SCROLLER_Y + INVENTORY_SCROLLER_X,
                pitch);
        }
    } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
        pitch = INVENTORY_LOOT_WINDOW_WIDTH;

        // Clear scroll view background.
        FrmImage backgroundFrmImage;
        int backgroundFid = buildFid(OBJ_TYPE_INTERFACE, gCurrentLootBackgroundFrm, 0, 0, 0);
        if (backgroundFrmImage.lock(backgroundFid)) {
            blitBufferToBuffer(backgroundFrmImage.getData() + pitch * INVENTORY_LOOT_LEFT_SCROLLER_Y + INVENTORY_LOOT_LEFT_SCROLLER_X,
                INVENTORY_SLOT_WIDTH,
                gInventorySlotsCount * gInventorySlotHeight,
                pitch,
                windowBuffer + pitch * INVENTORY_LOOT_LEFT_SCROLLER_Y + INVENTORY_LOOT_LEFT_SCROLLER_X,
                pitch);
        }

        // Build filtered index list (using combined inventory if active)
        gFilteredCount = getFilteredCount();

        // Clamp stackOffset.
        if (stackOffset >= gFilteredCount) {
            stackOffset = 0;
            _stack_offset[_curr_stack] = 0;
        }

        // Draw items.
        int y = 0;
        for (int slotIndex = 0; slotIndex < gInventorySlotsCount; slotIndex++) {
            int filteredIndex = stackOffset + slotIndex;
            if (filteredIndex >= gFilteredCount) break;
            int actualIndex = gFilteredIndices[filteredIndex];

            Object* item;
            int quantity;
            if (gUseCombinedInventory) {
                if (actualIndex >= gCombinedItemCount) break;
                CombinedItem* ci = &gCombinedItems[actualIndex];
                item = ci->item;
                quantity = ci->quantity;
            } else {
                if (actualIndex >= _pud->length) break;
                InventoryItem* invItem = &(_pud->items[actualIndex]);
                item = invItem->item;
                quantity = invItem->quantity;
            }

            int offset = pitch * (y + INVENTORY_LOOT_LEFT_SCROLLER_Y_PAD) + INVENTORY_LOOT_LEFT_SCROLLER_X_PAD;
            int inventoryFid = itemGetInventoryFid(item);
            artRenderGreen(inventoryFid, windowBuffer + offset, gInventorySlotWidthPadded, gInventorySlotHeightPadded, pitch);
            _display_inventory_info(item, quantity, windowBuffer + offset, pitch, slotIndex == dragSlotIndex, true);
            y += gInventorySlotHeight;
        }

        // Draw filter bar (if not strict vanilla).
        if (!settings.enhancements.strict_vanilla) {
            int barY = INVENTORY_LOOT_LEFT_SCROLLER_Y + gInventorySlotsCount * gInventorySlotHeight + 2;
            drawFilterBar(windowBuffer, pitch,
                INVENTORY_LOOT_LEFT_SCROLLER_X, barY, INVENTORY_SLOT_WIDTH);
        }
    } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        pitch = INVENTORY_TRADE_WINDOW_WIDTH;

        windowBuffer = windowGetBuffer(gInventoryWindow);

        blitBufferToBuffer(windowGetBuffer(_barter_back_win) + INVENTORY_TRADE_LEFT_SCROLLER_Y * INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH + INVENTORY_TRADE_LEFT_SCROLLER_X + INVENTORY_TRADE_WINDOW_OFFSET, INVENTORY_SLOT_WIDTH, gInventorySlotHeight * gInventorySlotsCount, INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH, windowBuffer + pitch * INVENTORY_TRADE_LEFT_SCROLLER_Y + INVENTORY_TRADE_LEFT_SCROLLER_X, pitch);

    } else {
        assert(false && "Should be unreachable");
    }

    // Draw items in grid (only for normal inventory)
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {

        // Build filtered index list (using combined inventory if active)
        gFilteredCount = getFilteredCount();

        // Clamp stackOffset to valid range
        if (stackOffset >= gFilteredCount) {
            stackOffset = 0;
            _stack_offset[_curr_stack] = 0;
        }

        // Draw the grid using gFilteredIndices
        int totalVisible = gInventoryRows * gInventoryColumns;
        for (int row = 0; row < gInventoryRows; ++row) {
            for (int col = 0; col < gInventoryColumns; ++col) {
                int slotIndex = row * gInventoryColumns + col;
                int filteredIndex = stackOffset + slotIndex;
                if (filteredIndex >= getFilteredCount()) {
                    row = gInventoryRows; // break outer loop
                    break;
                }
                int actualIndex = gFilteredIndices[filteredIndex];

                Object* item;
                int quantity;
                if (gUseCombinedInventory) {
                    CombinedItem* ci = &gCombinedItems[actualIndex];
                    item = ci->item;
                    quantity = ci->quantity;
                } else {
                    InventoryItem* invItem = &(_pud->items[actualIndex]);
                    item = invItem->item;
                    quantity = invItem->quantity;
                }

                int destOffset = pitch * (gLayout.scrollerY + row * gLayout.slotHeight + gLayout.slotPadding)
                    + (gLayout.scrollerX + col * gLayout.slotWidth + gLayout.slotPadding);
                artRenderGreen(itemGetInventoryFid(item), windowBuffer + destOffset,
                    gLayout.slotContentWidth, gLayout.slotContentHeight, pitch);
                _display_inventory_info(item, quantity, windowBuffer + destOffset, pitch, slotIndex == dragSlotIndex, true);
            }
        }
        // Draw Filter Bar at bottom
        int barY = gLayout.scrollerY + gInventoryRows * gLayout.slotHeight + 2;
        int barWidth = gLayout.scrollerWidth;
        drawFilterBar(windowBuffer, pitch, gLayout.scrollerX, barY, barWidth);
    } else {
        int y = 0;
        int totalFiltered = getFilteredCount();
        for (int slotIndex = 0; slotIndex < gInventorySlotsCount; slotIndex++) {
            int filteredIndex = stackOffset + slotIndex;
            if (filteredIndex >= totalFiltered) break;
            int actualIndex = gFilteredIndices[filteredIndex];

            Object* item;
            int quantity;
            if (gUseCombinedInventory) {
                CombinedItem* ci = &gCombinedItems[actualIndex];
                item = ci->item;
                quantity = ci->quantity;
            } else {
                InventoryItem* invItem = &(_pud->items[actualIndex]);
                item = invItem->item;
                quantity = invItem->quantity;
            }

            int offset;
            if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
                offset = pitch * (y + INVENTORY_TRADE_LEFT_SCROLLER_Y_PAD) + INVENTORY_TRADE_LEFT_SCROLLER_X_PAD;
            } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
                offset = pitch * (y + INVENTORY_LOOT_LEFT_SCROLLER_Y_PAD) + INVENTORY_LOOT_LEFT_SCROLLER_X_PAD;
            } else {
                offset = pitch * (y + INVENTORY_SCROLLER_Y_PAD) + INVENTORY_SCROLLER_X_PAD;
            }

            int inventoryFid = itemGetInventoryFid(item);
            artRenderGreen(inventoryFid, windowBuffer + offset, gInventorySlotWidthPadded, gInventorySlotHeightPadded, pitch);
            _display_inventory_info(item, quantity, windowBuffer + offset, pitch, slotIndex == dragSlotIndex, true);

            y += gInventorySlotHeight;
        }

        // Draw filter bar below the scroller (for all non?normal modes)
        if (!settings.enhancements.strict_vanilla && settings.enhancements.inventory_filter) {
            int barY;
            if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
                barY = INVENTORY_TRADE_SCROLLER_Y + gInventorySlotsCount * gInventorySlotHeight;
                drawFilterBar(windowBuffer, pitch, INVENTORY_TRADE_LEFT_SCROLLER_X, barY, INVENTORY_SLOT_WIDTH);
            } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
                barY = INVENTORY_LOOT_LEFT_SCROLLER_Y + gInventorySlotsCount * gInventorySlotHeight + 2;
                drawFilterBar(windowBuffer, pitch, INVENTORY_LOOT_LEFT_SCROLLER_X, barY, INVENTORY_SLOT_WIDTH);
            } else { // Use?on
                barY = INVENTORY_SCROLLER_Y + gInventorySlotsCount * gInventorySlotHeight;
                drawFilterBar(windowBuffer, pitch, INVENTORY_SCROLLER_X, barY, INVENTORY_SLOT_WIDTH);
            }
        }
    }

    // Draw filter bar below left outer inventory (trade only)
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE && !settings.enhancements.strict_vanilla) {
        int barY = INVENTORY_TRADE_SCROLLER_Y + gInventorySlotsCount * gInventorySlotHeight;
        drawFilterBar(windowBuffer, pitch,
            INVENTORY_TRADE_LEFT_SCROLLER_X, barY,
            INVENTORY_SLOT_WIDTH);
    }

    // Update scroll buttons state (for all modes that have them)
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL
        || inventoryWindowType == INVENTORY_WINDOW_TYPE_USE_ITEM_ON
        || inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {

        if (gInventoryScrollUpButton != -1) {
            if (stackOffset < gInventoryColumns) {
                buttonDisable(gInventoryScrollUpButton);
            } else {
                buttonEnable(gInventoryScrollUpButton);
            }
        }

        if (gInventoryScrollDownButton != -1) {
            int totalVisible = (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL)
                ? gInventoryRows * gInventoryColumns
                : gInventorySlotsCount;
            int filteredCount = getFilteredCount();
            if (filteredCount - stackOffset <= totalVisible) {
                buttonDisable(gInventoryScrollDownButton);
            } else {
                buttonEnable(gInventoryScrollDownButton);
            }
        }
    } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        tradeWindowUpdateScrollButtons();
    }

    // Draw equipped items (only for normal inventory)
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
        if (gInventoryRightHandItem != nullptr) {
            int width = gInventoryRightHandItem == gInventoryLeftHandItem ? INVENTORY_LARGE_SLOT_WIDTH * 2 : INVENTORY_LARGE_SLOT_WIDTH;
            int inventoryFid = itemGetInventoryFid(gInventoryRightHandItem);
            artRender(inventoryFid, windowBuffer + pitch * gLayout.rightHandSlotY + gLayout.rightHandSlotX, width, INVENTORY_LARGE_SLOT_HEIGHT, pitch);
        }

        if (gInventoryLeftHandItem != nullptr && gInventoryLeftHandItem != gInventoryRightHandItem) {
            int inventoryFid = itemGetInventoryFid(gInventoryLeftHandItem);
            artRender(inventoryFid, windowBuffer + pitch * gLayout.leftHandSlotY + gLayout.leftHandSlotX, INVENTORY_LARGE_SLOT_WIDTH, INVENTORY_LARGE_SLOT_HEIGHT, pitch);
        }

        if (gInventoryArmor != nullptr) {
            int inventoryFid = itemGetInventoryFid(gInventoryArmor);
            artRender(inventoryFid, windowBuffer + pitch * gLayout.armorSlotY + gLayout.armorSlotX, INVENTORY_LARGE_SLOT_WIDTH, INVENTORY_LARGE_SLOT_HEIGHT, pitch);
        }
    }
    // Grey out inventory buttons for companions who can't use
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL && gGreySlotFrm.isLocked()) {
        unsigned char* greyData = gGreySlotFrm.getData();
        int greyWidth = gGreySlotFrm.getWidth();
        int greyHeight = gGreySlotFrm.getHeight();
        int greyPitch = greyWidth;
        int pitch = gLayout.windowWidth;

        // Armor slot
        if (!partyMemberCanEquipArmor(_inven_dude)) {
            unsigned char* dest = windowBuffer + pitch * gLayout.armorSlotY + gLayout.armorSlotX;
            blitBufferToBuffer(greyData, greyWidth, greyHeight, greyPitch, dest, pitch);
        }

        // Hand slots
        if (!partyMemberCanEquipWeapon(_inven_dude)) {
            unsigned char* dest = windowBuffer + pitch * gLayout.leftHandSlotY + gLayout.leftHandSlotX;
            blitBufferToBuffer(greyData, greyWidth, greyHeight, greyPitch, dest, pitch);
            dest = windowBuffer + pitch * gLayout.rightHandSlotY + gLayout.rightHandSlotX;
            blitBufferToBuffer(greyData, greyWidth, greyHeight, greyPitch, dest, pitch);
        }
    }

    windowRefresh(gInventoryWindow);
}

// Render inventory item.
//
// [stackOffset] is an index of the first visible item in the scrolling view.
// [dragSlotIndex] is an index of item being dragged (it decreases displayed number of items in inner functions).
//
// 0x47036C
static void _display_target_inventory(int stackOffset, int dragSlotIndex, Inventory* inventory, int inventoryWindowType)
{
    unsigned char* windowBuffer = windowGetBuffer(gInventoryWindow);
    gCurrentInvWindowType = inventoryWindowType;
    int pitch;
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
        pitch = INVENTORY_LOOT_WINDOW_WIDTH;

        FrmImage backgroundFrmImage;
        int fid = buildFid(OBJ_TYPE_INTERFACE, gCurrentLootBackgroundFrm, 0, 0, 0);
        if (backgroundFrmImage.lock(fid)) {
            blitBufferToBuffer(backgroundFrmImage.getData() + pitch * INVENTORY_LOOT_RIGHT_SCROLLER_Y + INVENTORY_LOOT_RIGHT_SCROLLER_X,
                INVENTORY_SLOT_WIDTH,
                gInventorySlotHeight * gInventorySlotsCount,
                pitch,
                windowBuffer + pitch * INVENTORY_LOOT_RIGHT_SCROLLER_Y + INVENTORY_LOOT_RIGHT_SCROLLER_X,
                pitch);
        }

        // Build filtered index list (if not strict vanilla)
        gFilteredCount = buildFilteredIndices(inventory);

        // Clamp stackOffset to valid range
        if (stackOffset >= gFilteredCount) {
            stackOffset = 0;
            _target_stack_offset[_target_curr_stack] = 0;
        }

        // Draw filtered items
        int y = 0;
        for (int slotIndex = 0; slotIndex < gInventorySlotsCount; slotIndex++) {
            int filteredIndex = stackOffset + slotIndex;
            if (filteredIndex >= gFilteredCount) break;
            int originalIndex = gFilteredIndices[filteredIndex];
            InventoryItem* inventoryItem = &(inventory->items[originalIndex]);

            int offset = pitch * (y + INVENTORY_LOOT_RIGHT_SCROLLER_Y_PAD) + INVENTORY_LOOT_RIGHT_SCROLLER_X_PAD;
            int inventoryFid = itemGetInventoryFid(inventoryItem->item);
            artRenderGreen(inventoryFid, windowBuffer + offset, gInventorySlotWidthPadded, gInventorySlotHeightPadded, pitch);
            _display_inventory_info(inventoryItem->item, inventoryItem->quantity, windowBuffer + offset, pitch, slotIndex == dragSlotIndex, true);
            y += gInventorySlotHeight;
        }

        // Draw filter bar (if not strict vanilla)
        if (!settings.enhancements.strict_vanilla) {
            int barY = INVENTORY_LOOT_RIGHT_SCROLLER_Y + gInventorySlotsCount * gInventorySlotHeight + 2;
            drawFilterBar(windowBuffer, pitch,
                INVENTORY_LOOT_RIGHT_SCROLLER_X, barY, INVENTORY_SLOT_WIDTH);
        }
    } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        pitch = INVENTORY_TRADE_WINDOW_WIDTH;

        unsigned char* src = windowGetBuffer(_barter_back_win);
        blitBufferToBuffer(src + INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH * INVENTORY_TRADE_RIGHT_SCROLLER_Y + INVENTORY_TRADE_RIGHT_SCROLLER_X + INVENTORY_TRADE_WINDOW_OFFSET, INVENTORY_SLOT_WIDTH, gInventorySlotHeight * gInventorySlotsCount, INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH, windowBuffer + INVENTORY_TRADE_WINDOW_WIDTH * INVENTORY_TRADE_RIGHT_SCROLLER_Y + INVENTORY_TRADE_RIGHT_SCROLLER_X, INVENTORY_TRADE_WINDOW_WIDTH);
    } else {
        assert(false && "Should be unreachable");
    }

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        gFilteredCount = buildFilteredIndices(inventory);
        if (stackOffset >= gFilteredCount) {
            stackOffset = 0;
            _target_stack_offset[_target_curr_stack] = 0;
        }
        int y = 0;
        for (int slotIndex = 0; slotIndex < gInventorySlotsCount; slotIndex++) {
            int filteredIndex = stackOffset + slotIndex;
            if (filteredIndex >= gFilteredCount) break;
            int originalIndex = gFilteredIndices[filteredIndex];
            InventoryItem* inventoryItem = &(inventory->items[originalIndex]);

            int offset = pitch * (y + INVENTORY_TRADE_RIGHT_SCROLLER_Y_PAD) + INVENTORY_TRADE_RIGHT_SCROLLER_X_PAD;
            int inventoryFid = itemGetInventoryFid(inventoryItem->item);
            artRenderGreen(inventoryFid, windowBuffer + offset, gInventorySlotWidthPadded, gInventorySlotHeightPadded, pitch);
            _display_inventory_info(inventoryItem->item, inventoryItem->quantity, windowBuffer + offset, pitch, slotIndex == dragSlotIndex, true);

            y += gInventorySlotHeight;
        }
    }

    // Draw filter bar below right outer inventory (trade only)
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE && !settings.enhancements.strict_vanilla) {
        int barY = INVENTORY_TRADE_SCROLLER_Y + gInventorySlotsCount * gInventorySlotHeight;
        drawFilterBar(windowBuffer, pitch,
            INVENTORY_TRADE_RIGHT_SCROLLER_X, barY,
            INVENTORY_SLOT_WIDTH);
    }

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
        if (gSecondaryInventoryScrollUpButton != -1) {
            if (stackOffset <= 0) {
                buttonDisable(gSecondaryInventoryScrollUpButton);
            } else {
                buttonEnable(gSecondaryInventoryScrollUpButton);
            }
        }

        if (gSecondaryInventoryScrollDownButton != -1) {
            if (gFilteredCount - stackOffset <= gInventorySlotsCount) {
                buttonDisable(gSecondaryInventoryScrollDownButton);
            } else {
                buttonEnable(gSecondaryInventoryScrollDownButton);
            }
        }
    } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        tradeWindowUpdateScrollButtons(); // updates all 8 trade buttons
    }
}

/**
 * Returns the display name of the owner of an item, if it belongs to
 * someone other than the currently viewed character.
 *
 * @param item The item to check.
 * @param currentOwner The character whose inventory is currently open.
 * @return The owner's name, or nullptr if the item belongs to currentOwner
 *         or if not using combined inventory.
 */
static const char* getOwnerDisplayName(Object* item, Object* currentOwner)
{
    if (!gUseCombinedInventory) return nullptr;
    if (item == nullptr || currentOwner == nullptr) return nullptr;

    for (int i = 0; i < gCombinedItemCount; i++) {
        if (gCombinedItems[i].item == item) {
            Object* owner = gCombinedItems[i].owner;
            if (owner != currentOwner) {
                const char* name = objectGetName(owner);
                if (name != nullptr && name[0] != '\0') {
                    return name;
                }
                return "Unknown";
            }
            break;
        }
    }
    return nullptr;
}

// Renders inventory item quantity.
//
// 0x4705A0
static void _display_inventory_info(Object* item, int quantity, unsigned char* dest, int pitch, bool isDragged, bool isScreen)
{
    int oldFont = fontGetCurrent();
    fontSetCurrent(101); // small font for quantity

    char formattedText[12];
    bool drawQuantity = false;

    if (itemGetType(item) == ITEM_TYPE_AMMO) {
        int ammoQuantity = ammoGetCapacity(item) * (quantity - 1);
        if (!isDragged) {
            ammoQuantity += ammoGetQuantity(item);
        }
        if (ammoQuantity > 99999) ammoQuantity = 99999;
        snprintf(formattedText, sizeof(formattedText), "x%d", ammoQuantity);
        drawQuantity = true;
    } else {
        if (quantity > 1) {
            int displayedQuantity = quantity;
            if (isDragged) displayedQuantity -= 1;
            if (displayedQuantity > 99999) displayedQuantity = 99999;
            snprintf(formattedText, sizeof(formattedText), "x%d", displayedQuantity);
            drawQuantity = true;
        }
    }

    if (drawQuantity) {
        if (!settings.enhancements.green_monochrome || settings.enhancements.strict_vanilla || !isScreen) {
            fontDrawText(dest, formattedText, 80, pitch, _colorTable[COL_WHITE]);
        } else {
            fontDrawText(dest, formattedText, 80, pitch, _colorTable[COL_LIME_GREEN]);
        }
    }

    // Owner label (drawn at bottom of item, overlapping, shortened when too long)
    if (gUseCombinedInventory) {
        const char* ownerName = getOwnerDisplayName(item, _inven_dude);
        if (ownerName != nullptr) {
            int ownerFont = fontGetCurrent();
            fontSetCurrent(101);
            int lineHeight = fontGetLineHeight();
            int maxWidth = gInventorySlotWidthPadded;

            char truncated[64];
            strncpy(truncated, ownerName, sizeof(truncated) - 1);
            truncated[sizeof(truncated) - 1] = '\0';

            // If the full name is too wide, truncate and add "..."
            if (fontGetStringWidth(truncated) > maxWidth) {
                int len = strlen(truncated);
                while (len > 1) {
                    truncated[len - 1] = '\0';
                    // Check if the shortened name + "..." fits
                    if (fontGetStringWidth(truncated) + fontGetStringWidth("...") <= maxWidth) {
                        strcat(truncated, "...");
                        break;
                    }
                    len--;
                }
            }

            int finalTextWidth = fontGetStringWidth(truncated);
            int x = (gInventorySlotWidthPadded - finalTextWidth) / 2;
            int y = gInventorySlotHeightPadded - lineHeight;
            fontDrawText(dest + pitch * y + x, truncated, finalTextWidth, pitch, _colorTable[992]);
            fontSetCurrent(ownerFont);
        }
    }

    fontSetCurrent(oldFont);
}

// 0x470650
static void _display_body(int fid, int inventoryWindowType)
{
    bool shouldRotate = !_inven_redrawing_after_sort_menu;

    if (fid == -1 && shouldRotate) {
        if (getTicksSince(gInventoryWindowDudeRotationTimestamp) < INVENTORY_NORMAL_WINDOW_PC_ROTATION_DELAY) {
            return;
        }

        gInventoryWindowDudeRotation += 1;
        if (gInventoryWindowDudeRotation == ROTATION_COUNT) {
            gInventoryWindowDudeRotation = 0;
        }
    }

    if (gInventoryWindowDudeRotation == ROTATION_COUNT) {
        gInventoryWindowDudeRotation = 0;
    }

    int rotations[2];
    if (fid == -1) {
        rotations[0] = gInventoryWindowDudeRotation;
        rotations[1] = ROTATION_SE;
    } else {
        rotations[0] = ROTATION_SW;
        rotations[1] = _target_stack[_target_curr_stack]->rotation;
    }

    int fids[2] = {
        gInventoryWindowDudeFid,
        fid,
    };

    // Determine which window buffer and pitch to use (same for both portraits)
    unsigned char* windowBuffer;
    int windowPitch;
    int win;
    bool isTrade = (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE);
    if (isTrade) {
        windowBuffer = windowGetBuffer(_barter_back_win);
        windowPitch = windowGetWidth(_barter_back_win);
        win = _barter_back_win;
    } else {
        windowBuffer = windowGetBuffer(gInventoryWindow);
        windowPitch = windowGetWidth(gInventoryWindow);
        win = gInventoryWindow;
    }

    for (int index = 0; index < 2; index += 1) {
        int fid = fids[index];
        if (fid == -1) {
            continue;
        }

        CacheEntry* handle;
        Art* art = artLock(fid, &handle);
        if (art == nullptr) {
            continue;
        }

        int frame = 0;
        if (index == 1) {
            frame = artGetFrameCount(art) - 1;
        }

        int rotation = rotations[index];

        unsigned char* frameData = artGetFrameData(art, frame, rotation);

        int framePitch = artGetWidth(art, frame, rotation);
        int frameWidth = std::min(framePitch, INVENTORY_BODY_VIEW_WIDTH);

        int frameHeight = artGetHeight(art, frame, rotation);
        if (frameHeight > INVENTORY_BODY_VIEW_HEIGHT) {
            frameHeight = INVENTORY_BODY_VIEW_HEIGHT;
        }

        Rect rect;
        if (isTrade) {
            if (index == 1) {
                rect.left = 560;
                rect.top = 25;
            } else {
                rect.left = 15;
                rect.top = 25;
            }

            // Clear background for trade portraits
            FrmImage bg;
            int bgFid = buildFid(OBJ_TYPE_INTERFACE, gGameDialogSpeakerIsPartyMember ? 420 : 111, 0, 0, 0);
            if (bg.lock(bgFid)) {
                blitBufferToBuffer(bg.getData() + rect.top * 640 + rect.left,
                    INVENTORY_BODY_VIEW_WIDTH,
                    INVENTORY_BODY_VIEW_HEIGHT,
                    640,
                    windowBuffer + windowPitch * rect.top + rect.left,
                    windowPitch);
                bg.unlock();
            }
        } else {
            // Non-trade: determine position and background FRM
            int bgFrmId = gCurrentLootBackgroundFrm;
            int srcXOffset = 0;
            if (index == 1) {
                if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
                    rect.left = 426; // loot right cha window (or container)
                    rect.top = 39;
                    bgFrmId = gCurrentLootBackgroundFrm;
                    srcXOffset = 538;
                } else {
                    rect.left = 297; // inventory data window? ?not used?
                    rect.top = 37;
                    bgFrmId = gCurrentInventoryBackgroundFrm;
                    srcXOffset = 229;
                }
            } else {
                if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
                    rect.left = 48; // loot left cha window
                    rect.top = 39;
                    bgFrmId = gCurrentLootBackgroundFrm;
                    srcXOffset = 0;
                } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_USE_ITEM_ON) {
                    rect.left = 176; // Use item cha window
                    rect.top = 37;
                    bgFrmId = 113;
                    srcXOffset = 292;
                } else {
                    // MULTI-COLUMN: Use shifted body view position
                    rect.left = gLayout.bodyViewX;
                    rect.top = 37;
                    bgFrmId = gCurrentInventoryBackgroundFrm;
                    srcXOffset = 0;
                }
            }

            // Clear background for non-trade portraits
            FrmImage bg;
            int bgFid = buildFid(OBJ_TYPE_INTERFACE, bgFrmId, 0, 0, 0);
            if (bg.lock(bgFid)) {
                int bgPitch = bg.getWidth();
                blitBufferToBuffer(bg.getData() + bgPitch * rect.top + rect.left + srcXOffset,
                    INVENTORY_BODY_VIEW_WIDTH,
                    INVENTORY_BODY_VIEW_HEIGHT,
                    bgPitch,
                    windowBuffer + windowPitch * rect.top + rect.left,
                    windowPitch);
                bg.unlock();
            }
        }

        rect.right = rect.left + INVENTORY_BODY_VIEW_WIDTH - 1;
        rect.bottom = rect.top + INVENTORY_BODY_VIEW_HEIGHT - 1;

        // Draw the portrait
        int destY = rect.top + (INVENTORY_BODY_VIEW_HEIGHT - frameHeight) / 2;
        int destX = rect.left + (INVENTORY_BODY_VIEW_WIDTH - frameWidth) / 2;
        blitBufferToBufferGreenTrans(frameData, frameWidth, frameHeight, framePitch,
            windowBuffer + windowPitch * destY + destX,
            windowPitch);

        // Draw weight info if appropriate (only for loot and trade)
        bool drawWeight = false;
        Object* weightObj = nullptr;

        if (index == 0) {
            // Left side: always show weight for the player
            drawWeight = true;
            weightObj = (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) ? _inven_dude : _stack[0];
        } else {
            // Right side: only show for companions or containers
            if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
                Object* target = _target_stack[0];
                if (target && (objectIsPartyMember(target) || (FID_TYPE(target->fid) == OBJ_TYPE_ITEM && itemGetType(target) == ITEM_TYPE_CONTAINER))) {
                    drawWeight = true;
                    weightObj = target;
                }
            } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
                Object* target = _target_stack[_target_curr_stack];
                if (target && (objectIsPartyMember(target) || (FID_TYPE(target->fid) == OBJ_TYPE_ITEM && itemGetType(target) == ITEM_TYPE_CONTAINER))) {
                    drawWeight = true;
                    weightObj = target;
                }
            }
        }

        if (drawWeight && weightObj != nullptr) {
            int weightY = rect.top + INVENTORY_BODY_VIEW_HEIGHT - fontGetLineHeight() - 2;
            inventoryDrawWeightInfo(windowBuffer, windowPitch, rect.left, weightY, weightObj);
        }

        windowRefreshRect(win, &rect);
        artUnlock(handle);
    }

    if (fid == -1 && shouldRotate) {
        gInventoryWindowDudeRotationTimestamp = getTicks();
    }
}

// 0x470A2C
static int inventoryCommonInit()
{
    if (inventoryMessageListInit() == -1) {
        return -1;
    }

    _inven_ui_was_disabled = gameUiIsDisabled();

    if (_inven_ui_was_disabled) {
        gameUiEnable();
    }

    gameMouseObjectsHide();

    gameMouseSetCursor(MOUSE_CURSOR_ARROW);

    int index;
    for (index = 0; index < INVENTORY_WINDOW_CURSOR_COUNT; index++) {
        InventoryCursorData* cursorData = &(gInventoryCursorData[index]);

        int fid = buildFid(OBJ_TYPE_INTERFACE, gInventoryWindowCursorFrmIds[index], 0, 0, 0);
        Art* frm = artLock(fid, &(cursorData->frmHandle));
        if (frm == nullptr) {
            break;
        }

        cursorData->frm = frm;
        cursorData->frmData = artGetFrameData(frm, 0, 0);
        cursorData->width = artGetWidth(frm, 0, 0);
        cursorData->height = artGetHeight(frm, 0, 0);
        artGetFrameOffsets(frm, 0, 0, &(cursorData->offsetX), &(cursorData->offsetY));
    }

    if (index != INVENTORY_WINDOW_CURSOR_COUNT) {
        for (; index >= 0; index--) {
            artUnlock(gInventoryCursorData[index].frmHandle);
        }

        if (_inven_ui_was_disabled) {
            gameUiDisable(0);
        }

        messageListFree(&gInventoryMessageList);

        return -1;
    }

    _inven_is_initialized = true;
    _im_value = -1;
    _portrait_im_value = -1;
    _last_quick_sorted_object = nullptr; // reset quick-sort loop

    return 0;
}

// NOTE: Inlined.
//
// 0x470B8C
static void inventoryCommonFree()
{
    for (int index = 0; index < INVENTORY_WINDOW_CURSOR_COUNT; index++) {
        artUnlock(gInventoryCursorData[index].frmHandle);
    }

    if (_inven_ui_was_disabled) {
        gameUiDisable(0);
    }

    // NOTE: Uninline.
    inventoryMessageListFree();

    _inven_is_initialized = 0;
}

// 0x470BCC
static void inventorySetCursor(int cursor)
{
    gInventoryCursor = cursor;

    if (cursor != INVENTORY_WINDOW_CURSOR_ARROW) {
        InventoryCursorData* cursorData = &(gInventoryCursorData[cursor]);
        mouseSetFrame(cursorData->frmData, cursorData->width, cursorData->height, cursorData->width, cursorData->offsetX, cursorData->offsetY, 0);
    } else {
        // We're switching to arrow mode
        // First check if we're over a portrait button
        if (_portrait_im_value != -1) {
            inventoryPortraitOnMouseEnter(-1, _portrait_im_value);
        }
        // Then check if we're over an item button
        else if (_im_value != -1) {
            inventoryItemSlotOnMouseEnter(-1, _im_value);
        } else {
            // Not over any button, show regular arrow
            InventoryCursorData* cursorData = &(gInventoryCursorData[INVENTORY_WINDOW_CURSOR_ARROW]);
            mouseSetFrame(cursorData->frmData, cursorData->width, cursorData->height, cursorData->width, cursorData->offsetX, cursorData->offsetY, 0);
        }
    }
}

// 0x470C2C
static void inventoryItemSlotOnMouseEnter(int btn, int keyCode)
{
    if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
        int x;
        int y;
        mouseGetPositionInWindow(gInventoryWindow, &x, &y);

        Object* item = nullptr;
        if (_inven_from_button(keyCode, &item, nullptr, nullptr) != 0) {
            gameMouseRenderPrimaryAction(x, y, 3, gInventoryWindowMaxX, gInventoryWindowMaxY);

            int cursorHotspotX = 0;
            int cursorHotspotY = 0;
            _gmouse_3d_pick_frame_hot(&cursorHotspotX, &cursorHotspotY);

            InventoryCursorData* cursorData = &(gInventoryCursorData[INVENTORY_WINDOW_CURSOR_PICK]);
            mouseSetFrame(cursorData->frmData, cursorData->width, cursorData->height, cursorData->width, cursorHotspotX, cursorHotspotY, 0);

            if (item != _last_target) {
                objectLookAtFunc(_stack[0], item, gInventoryPrintItemDescriptionHandler);
            }
        } else {
            InventoryCursorData* cursorData = &(gInventoryCursorData[INVENTORY_WINDOW_CURSOR_ARROW]);
            mouseSetFrame(cursorData->frmData, cursorData->width, cursorData->height, cursorData->width, cursorData->offsetX, cursorData->offsetY, 0);
        }

        _last_target = item;
    }

    _im_value = keyCode;
}

// 0x470D1C
static void inventoryItemSlotOnMouseExit(int btn, int keyCode)
{
    if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
        InventoryCursorData* cursorData = &(gInventoryCursorData[INVENTORY_WINDOW_CURSOR_ARROW]);
        mouseSetFrame(cursorData->frmData, cursorData->width, cursorData->height, cursorData->width, cursorData->offsetX, cursorData->offsetY, 0);
    }

    _im_value = -1;
}

static void inventoryPortraitOnMouseEnter(int btn, int keyCode)
{
    if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
        int x, y;
        mouseGetPositionInWindow(gInventoryWindow, &x, &y);

        // Show 'sort' icon
        gameMouseRenderPrimaryAction(x, y, GAME_MOUSE_ACTION_MENU_ITEM_SORT,
            gInventoryWindowMaxX, gInventoryWindowMaxY);

        int hotX, hotY;
        _gmouse_3d_pick_frame_hot(&hotX, &hotY);

        InventoryCursorData* cursorData = &(gInventoryCursorData[INVENTORY_WINDOW_CURSOR_PICK]);
        mouseSetFrame(cursorData->frmData, cursorData->width, cursorData->height,
            cursorData->width, hotX, hotY, 0);
    }

    _portrait_im_value = keyCode; // Track that we are over a portrait button
}

static void inventoryPortraitOnMouseExit(int btn, int keyCode)
{
    if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
        // Revert to arrow cursor
        InventoryCursorData* cursorData = &(gInventoryCursorData[INVENTORY_WINDOW_CURSOR_ARROW]);
        mouseSetFrame(cursorData->frmData, cursorData->width, cursorData->height,
            cursorData->width, cursorData->offsetX, cursorData->offsetY, 0);
    }

    _portrait_im_value = -1; // Reset when leaving portrait
}

// 0x470D5C
static void _inven_update_lighting(Object* activeItem)
{
    if (gDude == _inven_dude) {
        int lightDistance;
        if (activeItem != nullptr && activeItem->lightDistance > 4) {
            lightDistance = activeItem->lightDistance;
        } else {
            lightDistance = 4;
        }

        Rect rect;
        objectSetLight(_inven_dude, lightDistance, 0x10000, &rect);
        tileWindowRefreshRect(&rect, gElevation);
    }
}

// 0x470DB8
static void _inven_pickup(int buttonCode, int indexOffset)
{
    Object* item;
    Object** itemSlot = nullptr;
    Object* owner = nullptr;
    int count = _inven_from_button(buttonCode, &item, &itemSlot, &owner);
    if (count == 0) {
        return;
    }
    if (owner == nullptr) {
        debugPrint("_inven_pickup: owner is null! Setting to _inven_dude.\n");
        owner = _inven_dude;
    }

    int itemIndex = -1;
    Object* itemInHand = nullptr;
    Rect rect;

    // Determine if this is a hand/armor slot using the new keycodes
    bool isHandOrArmor = (buttonCode == INVENTORY_HAND_RIGHT_KEY || buttonCode == INVENTORY_HAND_LEFT_KEY || buttonCode == INVENTORY_ARMOR_KEY);
    bool pickUpFromSlot = false;
    Object* srcOwner = nullptr; // original owner of the item (for stack splitting)
    bool removedFromStack = false; // true if we removed one from a stack

    if (isHandOrArmor) {
        pickUpFromSlot = true;
        switch (buttonCode) {
        case INVENTORY_HAND_RIGHT_KEY:
            rect.left = gLayout.rightHandSlotX;
            rect.top = gLayout.rightHandSlotY;
            if (_inven_dude == gDude && interfaceGetCurrentHand() != HAND_LEFT) {
                itemInHand = item;
            }
            break;
        case INVENTORY_HAND_LEFT_KEY:
            rect.left = gLayout.leftHandSlotX;
            rect.top = gLayout.leftHandSlotY;
            if (_inven_dude == gDude && interfaceGetCurrentHand() == HAND_LEFT) {
                itemInHand = item;
            }
            break;
        case INVENTORY_ARMOR_KEY:
            rect.left = gLayout.armorSlotX;
            rect.top = gLayout.armorSlotY;
            break;
        }
    } else {
        // Grid slot handling with stack support
        itemIndex = buttonCode - KEYCODE_GRID_BASE;
        int row = itemIndex / gInventoryColumns;
        int col = itemIndex % gInventoryColumns;
        rect.left = gLayout.scrollerX + col * gLayout.slotWidth;
        rect.top = gLayout.scrollerY + row * gLayout.slotHeight;

        // Determine source item, quantity, owner
        int displayIndex = indexOffset + itemIndex; // 0-based index into the displayed list (top to bottom)
        int actualIndex;

        if (gFilterCategory != -1) {
            // Filtered: use gFilteredIndices (already reversed)
            if (displayIndex >= gFilteredCount) return;
            actualIndex = gFilteredIndices[displayIndex];
        } else {
            // No filter: we need to reverse the order because display is reversed
            int totalItems = gUseCombinedInventory ? gCombinedItemCount : _pud->length;
            if (displayIndex >= totalItems) return;
            actualIndex = totalItems - displayIndex - 1;
        }

        Object* srcItem = nullptr;
        int srcQuantity = 0;

        if (gUseCombinedInventory) {
            if (actualIndex >= gCombinedItemCount) return;
            CombinedItem* ci = &gCombinedItems[actualIndex];
            srcItem = ci->item;
            srcQuantity = ci->quantity;
            srcOwner = ci->owner;
        } else {
            if (actualIndex >= _pud->length) return;
            InventoryItem* invItem = &(_pud->items[actualIndex]);
            srcItem = invItem->item;
            srcQuantity = invItem->quantity;
            srcOwner = _inven_dude;
        }

        if (srcItem == nullptr) return;

        Object* dragItem = srcItem;

        if (srcQuantity > 1) {
            // Remove one from the stack
            if (itemRemove(srcOwner, srcItem, 1) == 0) {
                if (gUseCombinedInventory) {
                    inventoryBuildCombinedList(_inven_dude);
                }
                _display_inventory(indexOffset, -1, INVENTORY_WINDOW_TYPE_NORMAL);
                dragItem = srcItem;
                removedFromStack = true;
            } else {
                return;
            }
        } else {
            // Single item: erase the slot background
            unsigned char* windowBuffer = windowGetBuffer(gInventoryWindow);
            FrmImage backgroundFrmImage;
            int backgroundFid = buildFid(OBJ_TYPE_INTERFACE, gCurrentInventoryBackgroundFrm, 0, 0, 0);
            if (backgroundFrmImage.lock(backgroundFid)) {
                int srcPitch = gLayout.windowWidth;
                int shift = (gInventoryColumns - 1) * gLayout.slotWidth;
                int srcX = gLayout.scrollerX + (itemIndex % gInventoryColumns) * gLayout.slotWidth;
                int srcY = gLayout.scrollerY + (itemIndex / gInventoryColumns) * gLayout.slotHeight;
                unsigned char* src = backgroundFrmImage.getData() + srcPitch * srcY + srcX;
                unsigned char* dest = windowBuffer + gLayout.windowWidth * rect.top + rect.left;
                blitBufferToBuffer(src, gLayout.slotWidth, gLayout.slotHeight, srcPitch, dest, gLayout.windowWidth);
            }
            rect.right = rect.left + gLayout.slotWidth - 1;
            rect.bottom = rect.top + gLayout.slotHeight - 1;
            windowRefreshRect(gInventoryWindow, &rect);
        }

        // Use dragItem for the rest
        item = dragItem;
    }

    // Erase the slot background
    pickUpFromSlot = isHandOrArmor;
    if (pickUpFromSlot || count <= 1) {
        unsigned char* windowBuffer = windowGetBuffer(gInventoryWindow);
        int width, height;
        if (pickUpFromSlot) {
            // Hand/armor slots use large dimensions
            if (gInventoryRightHandItem != gInventoryLeftHandItem || item != gInventoryLeftHandItem) {
                height = INVENTORY_LARGE_SLOT_HEIGHT;
                width = INVENTORY_LARGE_SLOT_WIDTH;
            } else {
                // ?dual-wield case: both hands same item
                height = INVENTORY_LARGE_SLOT_HEIGHT;
                width = 180;
                rect.left = gLayout.leftHandSlotX;
                rect.top = gLayout.leftHandSlotY;
            }
        } else {
            // Grid slot: use standard slot dimensions
            width = gLayout.slotWidth;
            height = gLayout.slotHeight;
            // rect should already be set correctly for the grid slot
        }
        rect.right = rect.left + width - 1;
        rect.bottom = rect.top + height - 1;

        FrmImage backgroundFrmImage;
        int backgroundFid = buildFid(OBJ_TYPE_INTERFACE, gCurrentInventoryBackgroundFrm, 0, 0, 0);
        if (backgroundFrmImage.lock(backgroundFid)) {
            int srcPitch = gLayout.windowWidth;
            int srcX, srcY;
            if (pickUpFromSlot) {
                int handArmorShift = (gInventoryColumns > 1) ? ((gInventoryColumns - 1) * gLayout.slotWidth) : 0;
                switch (buttonCode) {
                case INVENTORY_HAND_RIGHT_KEY:
                    srcX = INVENTORY_RIGHT_HAND_SLOT_X + handArmorShift;
                    srcY = INVENTORY_RIGHT_HAND_SLOT_Y;
                    break;
                case INVENTORY_HAND_LEFT_KEY:
                    srcX = INVENTORY_LEFT_HAND_SLOT_X + handArmorShift;
                    srcY = INVENTORY_LEFT_HAND_SLOT_Y;
                    break;
                case INVENTORY_ARMOR_KEY:
                    srcX = INVENTORY_ARMOR_SLOT_X + handArmorShift;
                    srcY = INVENTORY_ARMOR_SLOT_Y;
                    break;
                default:
                    srcX = rect.left;
                    srcY = rect.top;
                    break;
                }
            } else {
                srcX = rect.left;
                srcY = rect.top;
            }
            unsigned char* src = backgroundFrmImage.getData() + srcPitch * srcY + srcX;
            unsigned char* dest = windowBuffer + gLayout.windowWidth * rect.top + rect.left;
            blitBufferToBuffer(src, width, height, srcPitch, dest, gLayout.windowWidth);
        }
        windowRefreshRect(gInventoryWindow, &rect);

        if (itemInHand != nullptr) {
            _inven_update_lighting(nullptr);
        }
    }

    // Allow ctrl-click to quick unequip or equip item
    bool immediate = false;
    _drag_item_loop(item, immediate);

    // Drop handling (common for both grid and hand/armor)
    bool itemDropped = false; // track if we successfully placed the item somewhere

    // Drop from hand slot onto the grid (or Ctrl+click from hand)
    if (isHandOrArmor && (immediate || mouseHitTestInWindow(gInventoryWindow, gLayout.scrollerX, gLayout.scrollerY, gLayout.scrollerX + gLayout.scrollerWidth, gLayout.scrollerY + gLayout.scrollerHeight))) {
        int x, y;
        mouseGetPositionInWindow(gInventoryWindow, &x, &y);
        int row = (y - gLayout.scrollerY) / gLayout.slotHeight;
        int col = (x - gLayout.scrollerX) / gLayout.slotWidth;
        if (row < 0) row = 0;
        if (row >= gInventoryRows) row = gInventoryRows - 1;
        if (col < 0) col = 0;
        if (col >= gInventoryColumns) col = gInventoryColumns - 1;
        int targetIndex = row * gInventoryColumns + col + indexOffset;
        if (!immediate && targetIndex < _pud->length) {
            Object* targetItem = _pud->items[targetIndex].item;
            if (targetItem != item) {
                if (itemGetType(targetItem) == ITEM_TYPE_CONTAINER) {
                    if (_drop_into_container(targetItem, item, itemIndex, itemSlot, count) == 0) {
                        itemIndex = 0;
                        itemDropped = true;
                    }
                } else {
                    if (_drop_ammo_into_weapon(targetItem, item, itemSlot, count, buttonCode, owner) == 0) {
                        itemIndex = 0;
                        itemDropped = true;
                    }
                }
            }
        }
        if (immediate || isHandOrArmor) {
            *itemSlot = nullptr;
            if (itemAdd(_inven_dude, item, 1)) {
                *itemSlot = item;
                itemDropped = true;
            } else if (itemSlot == &gInventoryArmor) {
                adjustCritterStatsOnArmorChange(_stack[0], item, nullptr);
                itemDropped = true;
            } else if (gInventoryRightHandItem == gInventoryLeftHandItem) {
                gInventoryLeftHandItem = nullptr;
                gInventoryRightHandItem = nullptr;
                itemDropped = true;
            }
        }
    }

    // Ctrl+click from grid - equip weapon - commented out for now, maybe for good
    /*else if (!isHandOrArmor && immediate && itemGetType(item) != ITEM_TYPE_ARMOR) {
        bool canEquip = false;
        bool itemUsed = false;

        // Non-ammo items allow equipping
        if (!itemUsed) {
            if (itemGetType(item) == ITEM_TYPE_WEAPON) {
                canEquip = partyMemberCanEquipThisWeapon(_inven_dude, item);
            } else {
                // Non-weapon, non-ammo items can be equipped by anyone
                canEquip = true;
            }

            if (canEquip) {
                // Remove from original owner if it's a companion
                if (owner != nullptr && owner != _inven_dude) {
                    itemRemove(owner, item, 1);
                    if (gUseCombinedInventory) {
                        inventoryBuildCombinedList(_inven_dude);
                    }
                }

                // Equip to appropriate hand
                if (_inven_dude == gDude) {
                    bool left = gInventoryLeftHandItem == nullptr || gInventoryRightHandItem != nullptr;
                    if (left) {
                        _switch_hand(item, &gInventoryLeftHandItem, itemSlot, buttonCode);
                    } else {
                        _switch_hand(item, &gInventoryRightHandItem, itemSlot, buttonCode);
                    }
                } else {
                    bool right = gInventoryRightHandItem == nullptr || gInventoryLeftHandItem != nullptr;
                    if (right) {
                        _switch_hand(item, &gInventoryRightHandItem, itemSlot, buttonCode);
                    } else {
                        _switch_hand(item, &gInventoryLeftHandItem, itemSlot, buttonCode);
                    }
                }
                itemDropped = true;
            } else {
                // Cannot equip (e.g., unsupported weapon)
                Object* targetOwner = (gUseCombinedInventory && srcOwner != nullptr) ? srcOwner : _inven_dude;
                itemAdd(targetOwner, item, 1);
                if (gUseCombinedInventory) {
                    inventoryBuildCombinedList(_inven_dude);
                }
                itemDropped = true; // restored
                // Optional message: "That companion cannot use that weapon."
            }
        }
    }*/

    // Drop on left hand slot
    else if (mouseHitTestInWindow(gInventoryWindow,
                 gLayout.leftHandSlotX, gLayout.leftHandSlotY,
                 gLayout.leftHandSlotX + INVENTORY_LARGE_SLOT_WIDTH,
                 gLayout.leftHandSlotY + INVENTORY_LARGE_SLOT_HEIGHT)) {
        if (partyMemberCanEquipWeapon(_inven_dude)) { // broad body-type check
            bool canEquip = false;
            if (itemGetType(item) == ITEM_TYPE_WEAPON) {
                // Weapons require animation support - critical
                canEquip = partyMemberCanEquipThisWeapon(_inven_dude, item);
            } else {
                // All other items (drugs, ammo, food, misc) can be equipped by anyone
                canEquip = true;
            }
            if (canEquip) {
                transferItemToCurrentOwner(item, count, owner);
                if (gInventoryLeftHandItem != nullptr && itemGetType(gInventoryLeftHandItem) == ITEM_TYPE_CONTAINER && gInventoryLeftHandItem != item) {
                    if (_drop_into_container(gInventoryLeftHandItem, item, itemIndex, itemSlot, count) == 0) {
                        itemDropped = true;
                    }
                } else if (gInventoryLeftHandItem == nullptr || _drop_ammo_into_weapon(gInventoryLeftHandItem, item, itemSlot, count, buttonCode, owner)) {
                    _switch_hand(item, &gInventoryLeftHandItem, itemSlot, buttonCode);
                    itemDropped = true;
                }
            }
        }
    }
    // Drop on right hand slot
    else if (mouseHitTestInWindow(gInventoryWindow,
                 gLayout.rightHandSlotX, gLayout.rightHandSlotY,
                 gLayout.rightHandSlotX + INVENTORY_LARGE_SLOT_WIDTH,
                 gLayout.rightHandSlotY + INVENTORY_LARGE_SLOT_HEIGHT)) {
        if (partyMemberCanEquipWeapon(_inven_dude)) {
            bool canEquip = false;
            if (itemGetType(item) == ITEM_TYPE_WEAPON) {
                canEquip = partyMemberCanEquipThisWeapon(_inven_dude, item);
            } else {
                canEquip = true;
            }
            if (canEquip) {
                transferItemToCurrentOwner(item, count, owner);
                if (gInventoryRightHandItem != nullptr && itemGetType(gInventoryRightHandItem) == ITEM_TYPE_CONTAINER && gInventoryRightHandItem != item) {
                    if (_drop_into_container(gInventoryRightHandItem, item, itemIndex, itemSlot, count) == 0) {
                        itemDropped = true;
                    }
                } else if (gInventoryRightHandItem == nullptr || _drop_ammo_into_weapon(gInventoryRightHandItem, item, itemSlot, count, buttonCode, owner)) {
                    _switch_hand(item, &gInventoryRightHandItem, itemSlot, buttonCode);
                    itemDropped = true;
                }
            }
        }
    }
    // Drop on armor slot
    else if ((immediate && itemGetType(item) == ITEM_TYPE_ARMOR) || mouseHitTestInWindow(gInventoryWindow, gLayout.armorSlotX, gLayout.armorSlotY, gLayout.armorSlotX + INVENTORY_LARGE_SLOT_WIDTH, gLayout.armorSlotY + INVENTORY_LARGE_SLOT_HEIGHT)) {
        if (itemGetType(item) == ITEM_TYPE_ARMOR) {
            if (partyMemberCanEquipArmor(_inven_dude)) {
                transferItemToCurrentOwner(item, count, owner);
                Object* currentArmor = gInventoryArmor;
                int itemAddResult = 0;
                if (itemIndex != -1) {
                    itemRemove(_inven_dude, item, 1);
                }
                if (gInventoryArmor != nullptr) {
                    if (itemSlot != nullptr) {
                        *itemSlot = gInventoryArmor;
                    } else {
                        gInventoryArmor = nullptr;
                        itemAddResult = itemAdd(_inven_dude, currentArmor, 1);
                    }
                } else {
                    if (itemSlot != nullptr) {
                        *itemSlot = gInventoryArmor;
                    }
                }
                if (itemAddResult != 0) {
                    gInventoryArmor = currentArmor;
                    if (itemIndex != -1) {
                        itemAdd(_inven_dude, item, 1);
                    }
                } else {
                    adjustCritterStatsOnArmorChange(_stack[0], currentArmor, item);
                    gInventoryArmor = item;
                    itemDropped = true;
                }
            }
        }
    }
    // Drop on body view (container in backpack)
    else if (mouseHitTestInWindow(gInventoryWindow,
                 gLayout.bodyViewX, gLayout.bodyViewY,
                 gLayout.bodyViewX + INVENTORY_BODY_VIEW_WIDTH,
                 gLayout.bodyViewY + INVENTORY_BODY_VIEW_HEIGHT)) {
        if (_curr_stack != 0) {
            if (_drop_into_container(_stack[_curr_stack - 1], item, itemIndex, itemSlot, count) == 0) {
                itemDropped = true;
            }
        }
    }

    // Fallback - if we removed from stack and nothing else happened, add it back
    if (!itemDropped && removedFromStack) {
        // The item was removed from its owner but never placed anywhere.
        // Add it back to the original owner (or to the player if in combined mode and owner is a companion?).
        Object* targetOwner = (gUseCombinedInventory && srcOwner != nullptr) ? srcOwner : _inven_dude;
        itemAdd(targetOwner, item, 1);
        if (gUseCombinedInventory) {
            inventoryBuildCombinedList(_inven_dude);
        }
    }

    // Final cleanup and refresh
    _adjust_fid();
    if (gUseCombinedInventory) {
        inventoryBuildCombinedList(_inven_dude);
    }
    inventoryRenderSummary();
    _display_inventory(indexOffset, -1, INVENTORY_WINDOW_TYPE_NORMAL);
    inventorySetCursor(INVENTORY_WINDOW_CURSOR_HAND);
    if (_inven_dude == gDude) {
        Object* equippedItem;
        if (interfaceGetCurrentHand() == HAND_LEFT) {
            equippedItem = critterGetItem1(_inven_dude);
        } else {
            equippedItem = critterGetItem2(_inven_dude);
        }
        if (equippedItem != nullptr) {
            _inven_update_lighting(equippedItem);
        }
    }
}

// 0x4714E0
static void _switch_hand(Object* sourceItem, Object** targetSlot, Object** sourceSlot, int itemIndex)
{
    if (*targetSlot != nullptr) {
        if (itemGetType(*targetSlot) == ITEM_TYPE_WEAPON && itemGetType(sourceItem) == ITEM_TYPE_AMMO) {
            return;
        }

        if (sourceSlot != nullptr && (sourceSlot != &gInventoryArmor || itemGetType(*targetSlot) == ITEM_TYPE_ARMOR)) {
            if (sourceSlot == &gInventoryArmor) {
                adjustCritterStatsOnArmorChange(_stack[0], gInventoryArmor, *targetSlot);
            }
            *sourceSlot = *targetSlot;
        } else {
            if (itemIndex != -1) {
                itemRemove(_inven_dude, sourceItem, 1);
            }

            Object* existingItem = *targetSlot;
            *targetSlot = nullptr;
            if (itemAdd(_inven_dude, existingItem, 1) != 0) {
                itemAdd(_inven_dude, sourceItem, 1);
                return;
            }

            itemIndex = -1;

            if (sourceSlot != nullptr) {
                if (sourceSlot == &gInventoryArmor) {
                    adjustCritterStatsOnArmorChange(_stack[0], gInventoryArmor, nullptr);
                }
                *sourceSlot = nullptr;
            }
        }
    } else {
        if (sourceSlot != nullptr) {
            if (sourceSlot == &gInventoryArmor) {
                adjustCritterStatsOnArmorChange(_stack[0], gInventoryArmor, nullptr);
            }
            *sourceSlot = nullptr;
        }
    }

    *targetSlot = sourceItem;

    if (itemIndex != -1) {
        itemRemove(_inven_dude, sourceItem, 1);
    }
}

// This function removes armor bonuses and effects granted by [oldArmor] and
// adds appropriate bonuses and effects granted by [newArmor]. Both [oldArmor]
// and [newArmor] can be NULL.
//
// 0x4715F8
void adjustCritterStatsOnArmorChange(Object* critter, Object* oldArmor, Object* newArmor)
{
    int armorClassBonus = critterGetBonusStat(critter, STAT_ARMOR_CLASS);
    int oldArmorClass = armorGetArmorClass(oldArmor);
    int newArmorClass = armorGetArmorClass(newArmor);
    critterSetBonusStat(critter, STAT_ARMOR_CLASS, armorClassBonus - oldArmorClass + newArmorClass);

    int damageResistanceStat = STAT_DAMAGE_RESISTANCE;
    int damageThresholdStat = STAT_DAMAGE_THRESHOLD;
    for (int damageType = 0; damageType < DAMAGE_TYPE_COUNT; damageType += 1) {
        int damageResistanceBonus = critterGetBonusStat(critter, damageResistanceStat);
        int oldArmorDamageResistance = armorGetDamageResistance(oldArmor, damageType);
        int newArmorDamageResistance = armorGetDamageResistance(newArmor, damageType);
        critterSetBonusStat(critter, damageResistanceStat, damageResistanceBonus - oldArmorDamageResistance + newArmorDamageResistance);

        int damageThresholdBonus = critterGetBonusStat(critter, damageThresholdStat);
        int oldArmorDamageThreshold = armorGetDamageThreshold(oldArmor, damageType);
        int newArmorDamageThreshold = armorGetDamageThreshold(newArmor, damageType);
        critterSetBonusStat(critter, damageThresholdStat, damageThresholdBonus - oldArmorDamageThreshold + newArmorDamageThreshold);

        damageResistanceStat += 1;
        damageThresholdStat += 1;
    }

    // Change companion's appearance based on armor
    if (objectIsPartyMember(critter) && !settings.enhancements.strict_vanilla && settings.enhancements.npc_armor) {
        int newFid = -1;

        if (newArmor != nullptr) {
            // Equipping: build fid from the armor's male/female base frm id
            int baseFrmId;
            if (critterGetStat(critter, STAT_GENDER) == GENDER_FEMALE) {
                baseFrmId = armorGetFemaleFid(newArmor);
            } else {
                baseFrmId = armorGetMaleFid(newArmor);
            }
            if (baseFrmId != -1) {
                // Preserve weapon animation code from the current equipped weapon
                int weaponAnimCode = 0;
                Object* weapon = critterGetItem2(critter); // right hand
                if (weapon && itemGetType(weapon) == ITEM_TYPE_WEAPON) {
                    weaponAnimCode = weaponGetAnimationCode(weapon);
                }
                newFid = buildFid(OBJ_TYPE_CRITTER, baseFrmId, 0, weaponAnimCode, critter->rotation + 1);
            }
        } else {
            // Unequipping: revert to the critter's default fid (from its proto)
            Proto* proto;
            if (protoGetProto(critter->pid, &proto) != -1) {
                newFid = proto->fid;
                // Re-apply weapon animation code if any
                Object* weapon = critterGetItem2(critter);
                if (weapon && itemGetType(weapon) == ITEM_TYPE_WEAPON) {
                    int animCode = weaponGetAnimationCode(weapon);
                    newFid = buildFid(OBJ_TYPE_CRITTER, artGetIndex(newFid), 0, animCode, critter->rotation + 1);
                }
            }
        }

        if (newFid != -1 && newFid != critter->fid) {
            Rect rect;
            objectSetFid(critter, newFid, &rect);
            tileWindowRefreshRect(&rect, critter->elevation);
        }
    }

    if (objectIsPartyMember(critter)) {
        if (oldArmor != nullptr) {
            int perk = armorGetPerk(oldArmor);
            perkRemoveEffect(critter, perk);
        }

        if (newArmor != nullptr) {
            int perk = armorGetPerk(newArmor);
            perkAddEffect(critter, perk);
        }
    }
}

// 0x4716E8
static void _adjust_fid()
{
    int fid;
    if (FID_TYPE(_inven_dude->fid) == OBJ_TYPE_CRITTER) {
        Proto* proto;

        // Start with the critter's proto FID (or vault boy as fallback)
        int inventoryFid = _art_vault_guy_num;
        if (protoGetProto(_inven_pid, &proto) != -1) {
            inventoryFid = artGetIndex(proto->fid);
        }

        // Override with armor if worn
        if (gInventoryArmor != nullptr) {
            protoGetProto(gInventoryArmor->pid, &proto);
            if (critterGetStat(_inven_dude, STAT_GENDER) == GENDER_FEMALE) {
                inventoryFid = proto->item.data.armor.femaleFid;
            } else {
                inventoryFid = proto->item.data.armor.maleFid;
            }
            if (inventoryFid == -1) {
                inventoryFid = _art_vault_guy_num;
            }
        }

        // Determine weapon animation code
        int animationCode = 0;
        if (_inven_dude == gDude) {
            // Player - use current hand
            if (interfaceGetCurrentHand() == HAND_RIGHT) {
                if (gInventoryRightHandItem != nullptr) {
                    protoGetProto(gInventoryRightHandItem->pid, &proto);
                    if (proto->item.type == ITEM_TYPE_WEAPON) {
                        animationCode = proto->item.data.weapon.animationCode;
                    }
                }
            } else {
                if (gInventoryLeftHandItem != nullptr) {
                    protoGetProto(gInventoryLeftHandItem->pid, &proto);
                    if (proto->item.type == ITEM_TYPE_WEAPON) {
                        animationCode = proto->item.data.weapon.animationCode;
                    }
                }
            }
        } else {
            // Companions - always use right hand
            if (gInventoryRightHandItem != nullptr) {
                protoGetProto(gInventoryRightHandItem->pid, &proto);
                if (proto->item.type == ITEM_TYPE_WEAPON) {
                    animationCode = proto->item.data.weapon.animationCode;
                }
            }
        }

        fid = buildFid(OBJ_TYPE_CRITTER, inventoryFid, 0, animationCode, 0);
    } else {
        fid = _inven_dude->fid;
    }

    gInventoryWindowDudeFid = fid;
}

// 0x4717E4
void inventoryOpenUseItemOn(Object* targetObj)
{
    ScopedGameMode gm(GameMode::kUseOn);

    if (inventoryCommonInit() == -1) {
        return;
    }

    bool isoWasEnabled = _setup_inventory(INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
    _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
    inventorySetCursor(INVENTORY_WINDOW_CURSOR_HAND);
    for (;;) {
        sharedFpsLimiter.mark();

        if (_game_user_wants_to_quit != 0) {
            break;
        }

        _display_body(-1, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);

        int keyCode = inputGetInput();
        switch (keyCode) {
        case KEY_HOME:
            _stack_offset[_curr_stack] = 0;
            _display_inventory(0, -1, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
            break;
        case KEY_ARROW_UP:
            if (_stack_offset[_curr_stack] > 0) {
                _stack_offset[_curr_stack] -= 1;
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
            }
            break;
        case KEY_PAGE_UP:
            _stack_offset[_curr_stack] -= gInventorySlotsCount;
            if (_stack_offset[_curr_stack] < 0) {
                _stack_offset[_curr_stack] = 0;
                _display_inventory(_stack_offset[_curr_stack], -1, 1);
            }
            break;
        case KEY_END: {
            int totalFiltered = getFilteredCount();
            _stack_offset[_curr_stack] = totalFiltered - gInventorySlotsCount;
            if (_stack_offset[_curr_stack] < 0) {
                _stack_offset[_curr_stack] = 0;
            }
            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
            break;
        }
        case KEY_ARROW_DOWN: {
            int totalFiltered = getFilteredCount();
            if (_stack_offset[_curr_stack] + gInventorySlotsCount < totalFiltered) {
                _stack_offset[_curr_stack] += 1;
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
            }
            break;
        }
        case KEY_PAGE_DOWN: {
            int totalFiltered = getFilteredCount();
            _stack_offset[_curr_stack] += gInventorySlotsCount;
            if (_stack_offset[_curr_stack] + gInventorySlotsCount >= totalFiltered) {
                _stack_offset[_curr_stack] = totalFiltered - gInventorySlotsCount;
                if (_stack_offset[_curr_stack] < 0) {
                    _stack_offset[_curr_stack] = 0;
                }
            }
            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
            break;
        }
        case INVENTORY_BUTTON_LEFT:
            if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
                // Arrow mode - sort inventory
                inventoryWindowOpenSortContextMenu(keyCode, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
            } else {
                _container_exit(keyCode, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
            }
            break;
        default:
            if ((mouseGetEvent() & MOUSE_EVENT_RIGHT_BUTTON_DOWN) != 0) {
                if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_HAND) {
                    inventorySetCursor(INVENTORY_WINDOW_CURSOR_ARROW);
                } else {
                    inventorySetCursor(INVENTORY_WINDOW_CURSOR_HAND);
                }
            } else if ((mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_DOWN) != 0) {
                if (keyCode >= KEYCODE_FILTER_BASE && keyCode <= 8004) {
                    if (!settings.enhancements.strict_vanilla && settings.enhancements.inventory_filter) {
                        int category = keyCode - KEYCODE_FILTER_BASE;
                        if (gFilterCategory == category)
                            gFilterCategory = -1;
                        else
                            gFilterCategory = category;
                        // Reset scroll offset
                        _stack_offset[_curr_stack] = 0;
                        soundPlayFile("ib1p1xx1");
                        _display_inventory(0, -1, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
                        windowRefresh(gInventoryWindow);
                    }
                }
                if (keyCode >= KEYCODE_GRID_BASE && keyCode < KEYCODE_GRID_BASE + gInventorySlotsCount) {
                    if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
                        inventoryWindowOpenContextMenu(keyCode, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
                    } else {
                        Object* item = nullptr;
                        Object* owner = nullptr;
                        int quantity = _inven_from_button(keyCode, &item, nullptr, &owner);
                        if (item != nullptr) {
                            if (isInCombat()) {
                                if (gDude->data.critter.combat.ap >= 2) {
                                    if (_action_use_an_item_on_object(gDude, targetObj, item) != -1) {
                                        int actionPoints = gDude->data.critter.combat.ap;
                                        if (actionPoints < 2) {
                                            gDude->data.critter.combat.ap = 0;
                                        } else {
                                            gDude->data.critter.combat.ap = actionPoints - 2;
                                        }
                                        interfaceRenderActionPoints(gDude->data.critter.combat.ap, _combat_free_move);
                                    }
                                }
                            } else {
                                _action_use_an_item_on_object(gDude, targetObj, item);
                            }
                            gBlockMouseUpEvent = true;
                            keyCode = KEY_ESCAPE;
                        } else {
                            keyCode = -1;
                        }
                    }
                }
            } else if ((mouseGetEvent() & MOUSE_EVENT_WHEEL) != 0) {
                if (mouseHitTestInWindow(gInventoryWindow, INVENTORY_SCROLLER_X, INVENTORY_SCROLLER_Y, INVENTORY_SCROLLER_MAX_X, gInventorySlotHeight * gInventorySlotsCount + INVENTORY_SCROLLER_Y)) {
                    int wheelX, wheelY;
                    mouseGetWheel(&wheelX, &wheelY);
                    if (wheelY > 0) {
                        if (_stack_offset[_curr_stack] > 0) {
                            _stack_offset[_curr_stack] -= 1;
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
                        }
                    } else if (wheelY < 0) {
                        int totalFiltered = getFilteredCount();
                        if (_stack_offset[_curr_stack] + gInventorySlotsCount < totalFiltered) {
                            _stack_offset[_curr_stack] += 1;
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
                        }
                    }
                }
            }
            if (!settings.enhancements.strict_vanilla && settings.enhancements.inventory_filter) {
                int filterCategory = inventoryKeyToFilterCategory(keyCode);
                if (filterCategory != -1) {
                    if (gFilterCategory == filterCategory) {
                        gFilterCategory = -1;
                    } else {
                        gFilterCategory = filterCategory;
                    }
                    _stack_offset[_curr_stack] = 0;
                    soundPlayFile("ib1p1xx1");
                    _display_inventory(0, -1, INVENTORY_WINDOW_TYPE_USE_ITEM_ON);
                    windowRefresh(gInventoryWindow);
                }
            }
        }

        if (keyCode == KEY_ESCAPE) {
            break;
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    _exit_inventory(isoWasEnabled);

    // NOTE: Uninline.
    inventoryCommonFree();
}

// 0x471B70
Object* critterGetItem2(Object* critter)
{
    int i;
    Inventory* inventory;
    Object* item;

    if (gInventoryRightHandItem != nullptr && critter == _inven_dude) {
        return gInventoryRightHandItem;
    }

    inventory = &(critter->data.inventory);
    for (i = 0; i < inventory->length; i++) {
        item = inventory->items[i].item;
        if (item->flags & OBJECT_IN_RIGHT_HAND) {
            return item;
        }
    }

    return nullptr;
}

// 0x471BBC
Object* critterGetItem1(Object* critter)
{
    int i;
    Inventory* inventory;
    Object* item;

    if (gInventoryLeftHandItem != nullptr && critter == _inven_dude) {
        return gInventoryLeftHandItem;
    }

    inventory = &(critter->data.inventory);
    for (i = 0; i < inventory->length; i++) {
        item = inventory->items[i].item;
        if (item->flags & OBJECT_IN_LEFT_HAND) {
            return item;
        }
    }

    return nullptr;
}

// 0x471C08
Object* critterGetArmor(Object* critter)
{
    int i;
    Inventory* inventory;
    Object* item;

    if (gInventoryArmor != nullptr && critter == _inven_dude) {
        return gInventoryArmor;
    }

    inventory = &(critter->data.inventory);
    for (i = 0; i < inventory->length; i++) {
        item = inventory->items[i].item;
        if (item->flags & OBJECT_WORN) {
            return item;
        }
    }

    return nullptr;
}

// 0x471CA0
Object* objectGetCarriedObjectByPid(Object* obj, int pid)
{
    Inventory* inventory = &(obj->data.inventory);

    for (int index = 0; index < inventory->length; index++) {
        InventoryItem* inventoryItem = &(inventory->items[index]);
        if (inventoryItem->item->pid == pid) {
            return inventoryItem->item;
        }

        Object* found = objectGetCarriedObjectByPid(inventoryItem->item, pid);
        if (found != nullptr) {
            return found;
        }
    }

    return nullptr;
}

// 0x471CDC
int objectGetCarriedQuantityByPid(Object* object, int pid)
{
    int quantity = 0;

    Inventory* inventory = &(object->data.inventory);
    for (int index = 0; index < inventory->length; index++) {
        InventoryItem* inventoryItem = &(inventory->items[index]);
        if (inventoryItem->item->pid == pid) {
            quantity += inventoryItem->quantity;
        }

        quantity += objectGetCarriedQuantityByPid(inventoryItem->item, pid);
    }

    return quantity;
}

// Renders character's summary of SPECIAL stats, equipped armor bonuses,
// and weapon's damage/range.
//
// 0x471D5C
static void inventoryRenderSummary()
{
    int summaryStats[7];
    memcpy(summaryStats, gSummaryStats, sizeof(summaryStats));

    int summaryStats2[7];
    memcpy(summaryStats2, gSummaryStats2, sizeof(summaryStats2));

    char formattedText[80];

    int oldFont = fontGetCurrent();
    fontSetCurrent(101);

    unsigned char* windowBuffer = windowGetBuffer(gInventoryWindow);

    FrmImage backgroundFrmImage;
    int backgroundFid = buildFid(OBJ_TYPE_INTERFACE, gCurrentInventoryBackgroundFrm, 0, 0, 0);
    if (backgroundFrmImage.lock(backgroundFid)) {
        int srcPitch = gLayout.windowWidth; // original background width
        int shift = (gInventoryColumns - 1) * gLayout.slotWidth; // same shift used in layout
        // Source X: original summary X + shift (so we take a right-shifted region)
        unsigned char* src = backgroundFrmImage.getData() + srcPitch * gLayout.summaryY + (INVENTORY_SUMMARY_X + shift);
        unsigned char* dest = windowBuffer + gLayout.windowWidth * gLayout.summaryY + gLayout.summaryX;
        blitBufferToBuffer(src, INVENTORY_SUMMARY_WIDTH, INVENTORY_SUMMARY_HEIGHT, srcPitch, dest, gLayout.windowWidth);
    }

    // Render character name.
    const char* critterName = critterGetName(_stack[0]);
    fontDrawText(windowBuffer + gLayout.windowWidth * gLayout.summaryY + gLayout.summaryX, critterName, 80, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);

    bufferDrawLine(windowBuffer,
        gLayout.windowWidth,
        gLayout.summaryX,
        3 * fontGetLineHeight() / 2 + gLayout.summaryY,
        gLayout.summaryX + 151,
        3 * fontGetLineHeight() / 2 + gLayout.summaryY,
        _colorTable[COL_LIME_GREEN]);

    MessageListItem messageListItem;

    int offset = gLayout.windowWidth * 2 * fontGetLineHeight() + gLayout.windowWidth * gLayout.summaryY + gLayout.summaryX;
    for (int stat = 0; stat < PRIMARY_STAT_COUNT; stat++) {
        messageListItem.num = stat;
        if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
            fontDrawText(windowBuffer + offset, messageListItem.text, 80, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);
        }

        int value = critterGetStat(_stack[0], stat);
        snprintf(formattedText, sizeof(formattedText), "%d", value);
        fontDrawText(windowBuffer + offset + 24, formattedText, 80, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);

        offset += gLayout.windowWidth * fontGetLineHeight();
    }

    offset -= gLayout.windowWidth * 7 * fontGetLineHeight();

    for (int index = 0; index < 7; index += 1) {
        messageListItem.num = 7 + index;
        if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
            fontDrawText(windowBuffer + offset + 40, messageListItem.text, 80, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);
        }

        if (summaryStats2[index] == -1) {
            int value = critterGetStat(_stack[0], summaryStats[index]);
            snprintf(formattedText, sizeof(formattedText), "   %d", value);
        } else {
            int value1 = critterGetStat(_stack[0], summaryStats[index]);
            int value2 = critterGetStat(_stack[0], summaryStats2[index]);
            const char* format = index != 0 ? "%d/%d%%" : "%d/%d";
            snprintf(formattedText, sizeof(formattedText), format, value1, value2);
        }

        fontDrawText(windowBuffer + offset + 104, formattedText, 80, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);

        offset += gLayout.windowWidth * fontGetLineHeight();
    }

    bufferDrawLine(windowBuffer, gLayout.windowWidth, gLayout.summaryX, 18 * fontGetLineHeight() / 2 + 48, gLayout.summaryX + 151, 18 * fontGetLineHeight() / 2 + 48, _colorTable[COL_LIME_GREEN]);
    bufferDrawLine(windowBuffer, gLayout.windowWidth, gLayout.summaryX, 26 * fontGetLineHeight() / 2 + 48, gLayout.summaryX + 151, 26 * fontGetLineHeight() / 2 + 48, _colorTable[COL_LIME_GREEN]);

    Object* itemsInHands[2] = {
        gInventoryLeftHandItem,
        gInventoryRightHandItem,
    };

    const int hitModes[2] = {
        HIT_MODE_LEFT_WEAPON_PRIMARY,
        HIT_MODE_RIGHT_WEAPON_PRIMARY,
    };

    const int secondaryHitModes[2] = {
        HIT_MODE_LEFT_WEAPON_SECONDARY,
        HIT_MODE_RIGHT_WEAPON_SECONDARY,
    };

    const int unarmedHitModes[2] = {
        HIT_MODE_PUNCH,
        HIT_MODE_KICK,
    };

    offset += gLayout.windowWidth * fontGetLineHeight();

    for (int index = 0; index < 2; index += 1) {
        Object* item = itemsInHands[index];
        if (item == nullptr) {
            formattedText[0] = '\0';

            messageListItem.num = 14; // No item
            if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                fontDrawText(windowBuffer + offset, messageListItem.text, 120, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);
            }

            offset += gLayout.windowWidth * fontGetLineHeight();

            messageListItem.num = 24; // Unarmed dmg:
            if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                // SFALL: Display the actual damage values of unarmed attacks.
                // CE: Implementation is different.
                int hitMode = unarmedHitModes[index];
                if (_stack[0] == gDude) {
                    int actions[2];
                    interfaceGetItemActions(&(actions[0]), &(actions[1]));

                    bool isSecondary = actions[index] == INTERFACE_ITEM_ACTION_SECONDARY
                        || actions[index] == INTERFACE_ITEM_ACTION_SECONDARY_AIMING;

                    if (index == HAND_LEFT) {
                        hitMode = unarmedGetPunchHitMode(isSecondary);
                    } else {
                        hitMode = unarmedGetKickHitMode(isSecondary);
                    }
                }

                // Formula is the same as in `weaponGetDamage`.
                int minDamage;
                int maxDamage;
                int bonusDamage = unarmedGetDamage(hitMode, &minDamage, &maxDamage);
                int meleeDamage = critterGetStat(_stack[0], STAT_MELEE_DAMAGE);
                // TODO: Localize unarmed attack names.
                snprintf(formattedText, sizeof(formattedText), "%s %d-%d",
                    messageListItem.text,
                    bonusDamage + minDamage,
                    bonusDamage + meleeDamage + maxDamage);
            }

            fontDrawText(windowBuffer + offset, formattedText, 120, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);

            offset += 3 * gLayout.windowWidth * fontGetLineHeight();
            continue;
        }

        const char* itemName = itemGetName(item);
        fontDrawText(windowBuffer + offset, itemName, 140, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);

        offset += gLayout.windowWidth * fontGetLineHeight();

        int itemType = itemGetType(item);
        if (itemType != ITEM_TYPE_WEAPON) {
            if (itemType == ITEM_TYPE_ARMOR) {
                messageListItem.num = 18; // (Not worn)
                if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                    fontDrawText(windowBuffer + offset, messageListItem.text, 120, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);
                }
            }

            offset += 3 * gLayout.windowWidth * fontGetLineHeight();
            continue;
        }

        // SFALL: Fix displaying secondary mode weapon range.
        int hitMode = hitModes[index];
        if (_stack[0] == gDude) {
            int actions[2];
            interfaceGetItemActions(&(actions[0]), &(actions[1]));

            bool isSecondary = actions[index] == INTERFACE_ITEM_ACTION_SECONDARY
                || actions[index] == INTERFACE_ITEM_ACTION_SECONDARY_AIMING;

            if (isSecondary) {
                hitMode = secondaryHitModes[index];
            }
        }

        int range = weaponGetRange(_stack[0], hitMode);

        int damageMin;
        int damageMax;
        weaponGetDamageMinMax(item, &damageMin, &damageMax);

        // CE: Fix displaying secondary mode weapon damage (affects throwable
        // melee weapons - knifes, spears, etc.).
        int attackType = weaponGetAttackTypeForHitMode(item, hitMode);

        formattedText[0] = '\0';

        int meleeDamage;
        if (attackType == ATTACK_TYPE_MELEE || attackType == ATTACK_TYPE_UNARMED) {
            meleeDamage = critterGetStat(_stack[0], STAT_MELEE_DAMAGE);

            // SFALL: Display melee damage without "Bonus HtH Damage" bonus.
            if (damageModGetBonusHthDamageFix() && !damageModGetDisplayBonusDamage()) {
                meleeDamage -= 2 * perkGetRank(gDude, PERK_BONUS_HTH_DAMAGE);
            }
        } else {
            meleeDamage = 0;
        }

        messageListItem.num = 15; // Dmg:
        if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
            if (attackType != 4 && range <= 1) {
                // SFALL: Display bonus damage.
                if (damageModGetBonusHthDamageFix() && damageModGetDisplayBonusDamage()) {
                    // CE: Just in case check for attack type, however it looks
                    // like we cannot be here with anything besides melee or
                    // unarmed.
                    if (_stack[0] == gDude && (attackType == ATTACK_TYPE_MELEE || attackType == ATTACK_TYPE_UNARMED)) {
                        // See explanation in `weaponGetDamage`.
                        damageMin += 2 * perkGetRank(gDude, PERK_BONUS_HTH_DAMAGE);
                    }
                }
                snprintf(formattedText, sizeof(formattedText), "%s %d-%d", messageListItem.text, damageMin, damageMax + meleeDamage);
            } else {
                MessageListItem rangeMessageListItem;
                rangeMessageListItem.num = 16; // Rng:
                if (messageListGetItem(&gInventoryMessageList, &rangeMessageListItem)) {
                    // SFALL: Display bonus damage.
                    if (damageModGetDisplayBonusDamage()) {
                        // CE: There is a bug in Sfall diplaying wrong damage
                        // bonus for melee weapons with range > 1 (spears,
                        // sledgehammers) and throwables (secondary mode).
                        if (_stack[0] == gDude && attackType == ATTACK_TYPE_RANGED) {
                            int damageBonus = 2 * perkGetRank(gDude, PERK_BONUS_RANGED_DAMAGE);
                            damageMin += damageBonus;
                            damageMax += damageBonus;
                        }
                    }

                    snprintf(formattedText, sizeof(formattedText), "%s %d-%d   %s %d", messageListItem.text, damageMin, damageMax + meleeDamage, rangeMessageListItem.text, range);
                }
            }

            fontDrawText(windowBuffer + offset, formattedText, 140, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);
        }

        offset += gLayout.windowWidth * fontGetLineHeight();

        if (ammoGetCapacity(item) > 0) {
            int ammoTypePid = weaponGetAmmoTypePid(item);

            formattedText[0] = '\0';

            messageListItem.num = 17; // Ammo:
            if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                if (ammoTypePid != -1) {
                    if (ammoGetQuantity(item) != 0) {
                        const char* ammoName = protoGetName(ammoTypePid);
                        int capacity = ammoGetCapacity(item);
                        int quantity = ammoGetQuantity(item);
                        snprintf(formattedText, sizeof(formattedText), "%s %d/%d %s", messageListItem.text, quantity, capacity, ammoName);
                    } else {
                        int capacity = ammoGetCapacity(item);
                        int quantity = ammoGetQuantity(item);
                        snprintf(formattedText, sizeof(formattedText), "%s %d/%d", messageListItem.text, quantity, capacity);
                    }
                }
            } else {
                int capacity = ammoGetCapacity(item);
                int quantity = ammoGetQuantity(item);
                snprintf(formattedText, sizeof(formattedText), "%s %d/%d", messageListItem.text, quantity, capacity);
            }

            fontDrawText(windowBuffer + offset, formattedText, 140, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);
        }

        offset += 2 * gLayout.windowWidth * fontGetLineHeight();
    }

    // Total wt:
    messageListItem.num = 20; // Total Wt:
    if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
        if (PID_TYPE(_stack[0]->pid) == OBJ_TYPE_CRITTER) {
            int carryWeight = critterGetStat(_stack[0], STAT_CARRY_WEIGHT);
            int inventoryWeight = objectGetInventoryWeight(_stack[0]);
            snprintf(formattedText, sizeof(formattedText), "%s %d/%d", messageListItem.text, inventoryWeight, carryWeight);

            int color = _colorTable[COL_LIME_GREEN];
            if (critterIsEncumbered(_stack[0])) {
                color = _colorTable[COL_PURE_RED];
            }

            fontDrawText(windowBuffer + offset + 15, formattedText, 120, gLayout.windowWidth, color);
        } else {
            int inventoryWeight = objectGetInventoryWeight(_stack[0]);
            snprintf(formattedText, sizeof(formattedText), "%s %d", messageListItem.text, inventoryWeight);

            fontDrawText(windowBuffer + offset + 30, formattedText, 80, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);
        }
    }

    fontSetCurrent(oldFont);
}

// Finds next item of given [itemType] (can be -1 which means any type of
// item).
//
// The [index] is used to control where to continue the search from, -1 - from
// the beginning.
//
// 0x472698
Object* inventoryFindByType(Object* obj, int itemType, int* indexPtr)
{
    int dummy = -1;
    if (indexPtr == nullptr) {
        indexPtr = &dummy;
    }

    *indexPtr += 1;

    Inventory* inventory = &(obj->data.inventory);

    // TODO: Refactor with for loop.
    if (*indexPtr >= inventory->length) {
        return nullptr;
    }

    while (itemType != -1 && itemGetType(inventory->items[*indexPtr].item) != itemType) {
        *indexPtr += 1;

        if (*indexPtr >= inventory->length) {
            return nullptr;
        }
    }

    return inventory->items[*indexPtr].item;
}

// Searches for an item with a given id inside given obj's inventory.
//
// 0x4726EC
Object* inventoryFindById(Object* obj, int id)
{
    if (obj->id == id) {
        return obj;
    }

    Inventory* inventory = &(obj->data.inventory);
    for (int index = 0; index < inventory->length; index++) {
        InventoryItem* inventoryItem = &(inventory->items[index]);
        Object* item = inventoryItem->item;
        if (item->id == id) {
            return item;
        }

        if (itemGetType(item) == ITEM_TYPE_CONTAINER) {
            item = inventoryFindById(item, id);
            if (item != nullptr) {
                return item;
            }
        }
    }

    return nullptr;
}

// Returns inventory item at a given index.
//
// 0x472740
Object* inventoryItemByIndex(Object* obj, int index)
{
    Inventory* inventory;

    inventory = &(obj->data.inventory);

    if (index < 0 || index >= inventory->length) {
        return nullptr;
    }

    return inventory->items[index].item;
}

// inven_wield
// 0x472758
int inventoryEquip(Object* critter, Object* item, int hand)
{
    return inventoryEquipFunc(critter, item, hand, true);
}

// 0x472768
int inventoryEquipFunc(Object* critter, Object* item, int handIndex, bool animate)
{
    if (animate) {
        if (!isoIsDisabled()) {
            reg_anim_begin(ANIMATION_REQUEST_RESERVED);
        }
    }

    int itemType = itemGetType(item);
    if (itemType == ITEM_TYPE_ARMOR) {
        Object* armor = critterGetArmor(critter);
        if (armor != nullptr) {
            armor->flags &= ~OBJECT_WORN;
        }

        item->flags |= OBJECT_WORN;

        int baseFrmId;
        if (critterGetStat(critter, STAT_GENDER) == GENDER_FEMALE) {
            baseFrmId = armorGetFemaleFid(item);
        } else {
            baseFrmId = armorGetMaleFid(item);
        }

        if (baseFrmId == -1) {
            baseFrmId = 1;
        }

        if (critter == gDude) {
            if (!isoIsDisabled()) {
                int fid = buildFid(OBJ_TYPE_CRITTER, baseFrmId, 0, (critter->fid & 0xF000) >> 12, critter->rotation + 1);
                animationRegisterSetFid(critter, fid, 0);
            }
        } else {
            adjustCritterStatsOnArmorChange(critter, armor, item);
        }
    } else {
        int hand;
        if (critter == gDude) {
            hand = interfaceGetCurrentHand();
        } else {
            hand = HAND_RIGHT;
        }

        int weaponAnimationCode = weaponGetAnimationCode(item);
        int hitModeAnimationCode = weaponGetAnimationForHitMode(item, HIT_MODE_RIGHT_WEAPON_PRIMARY);
        int fid = buildFid(OBJ_TYPE_CRITTER, artGetIndex(critter->fid), hitModeAnimationCode, weaponAnimationCode, critter->rotation + 1);
        if (!artExists(fid)) {
            debugPrint("\ninven_wield failed!  ERROR ERROR ERROR!");
            return -1;
        }

        Object* equippedItem;
        if (handIndex == HAND_RIGHT) {
            equippedItem = critterGetItem2(critter);
            item->flags |= OBJECT_IN_RIGHT_HAND;
        } else {
            equippedItem = critterGetItem1(critter);
            item->flags |= OBJECT_IN_LEFT_HAND;
        }

        Rect rect;
        if (equippedItem != nullptr) {
            equippedItem->flags &= ~OBJECT_IN_ANY_HAND;

            if (equippedItem->pid == PROTO_ID_LIT_FLARE) {
                int lightIntensity;
                int lightDistance;
                if (critter == gDude) {
                    lightIntensity = LIGHT_INTENSITY_MAX;
                    lightDistance = 4;
                } else {
                    Proto* proto;
                    if (protoGetProto(critter->pid, &proto) == -1) {
                        return -1;
                    }

                    lightDistance = proto->lightDistance;
                    lightIntensity = proto->lightIntensity;
                }

                objectSetLight(critter, lightDistance, lightIntensity, &rect);
            }
        }

        if (item->pid == PROTO_ID_LIT_FLARE) {
            int lightDistance = item->lightDistance;
            if (lightDistance < critter->lightDistance) {
                lightDistance = critter->lightDistance;
            }

            int lightIntensity = item->lightIntensity;
            if (lightIntensity < critter->lightIntensity) {
                lightIntensity = critter->lightIntensity;
            }

            objectSetLight(critter, lightDistance, lightIntensity, &rect);
            tileWindowRefreshRect(&rect, gElevation);
        }

        if (itemGetType(item) == ITEM_TYPE_WEAPON) {
            weaponAnimationCode = weaponGetAnimationCode(item);
        } else {
            weaponAnimationCode = 0;
        }

        if (hand == handIndex) {
            if ((critter->fid & 0xF000) >> 12 != 0) {
                if (animate) {
                    if (!isoIsDisabled()) {
                        const char* soundEffectName = sfxBuildCharName(critter, ANIM_PUT_AWAY, CHARACTER_SOUND_EFFECT_UNUSED);
                        animationRegisterPlaySoundEffect(critter, soundEffectName, 0);
                        animationRegisterAnimate(critter, ANIM_PUT_AWAY, 0);
                    }
                }
            }

            if (animate && !isoIsDisabled()) {
                if (weaponAnimationCode != 0) {
                    animationRegisterTakeOutWeapon(critter, weaponAnimationCode, -1);
                } else {
                    int fid = buildFid(OBJ_TYPE_CRITTER, artGetIndex(critter->fid), 0, 0, critter->rotation + 1);
                    animationRegisterSetFid(critter, fid, -1);
                }
            } else {
                int fid = buildFid(OBJ_TYPE_CRITTER, artGetIndex(critter->fid), 0, weaponAnimationCode, critter->rotation + 1);
                _dude_stand(critter, critter->rotation, fid);
            }
        }
    }

    if (animate) {
        if (!isoIsDisabled()) {
            return reg_anim_end();
        }
    }

    return 0;
}

// inven_unwield
// 0x472A54
int inventoryUnequip(Object* critter_obj, int hand)
{
    return inventoryUnequipFunc(critter_obj, hand, true);
}

// 0x472A64
int inventoryUnequipFunc(Object* critter, int hand, bool animate)
{
    int activeHand;
    Object* item;
    int fid;

    if (critter == gDude) {
        activeHand = interfaceGetCurrentHand();
    } else {
        activeHand = HAND_RIGHT; // NPC's only ever use right slot
    }

    if (hand) {
        item = critterGetItem2(critter);
    } else {
        item = critterGetItem1(critter);
    }

    if (item) {
        item->flags &= ~OBJECT_IN_ANY_HAND;
    }

    if (activeHand == hand && ((critter->fid & 0xF000) >> 12) != 0) {
        if (animate && !isoIsDisabled()) {
            reg_anim_begin(ANIMATION_REQUEST_RESERVED);

            const char* sfx = sfxBuildCharName(critter, ANIM_PUT_AWAY, CHARACTER_SOUND_EFFECT_UNUSED);
            animationRegisterPlaySoundEffect(critter, sfx, 0);

            animationRegisterAnimate(critter, ANIM_PUT_AWAY, 0);

            fid = buildFid(OBJ_TYPE_CRITTER, artGetIndex(critter->fid), 0, 0, critter->rotation + 1);
            animationRegisterSetFid(critter, fid, -1);

            return reg_anim_end();
        }

        fid = buildFid(OBJ_TYPE_CRITTER, artGetIndex(critter->fid), 0, 0, critter->rotation + 1);
        _dude_stand(critter, critter->rotation, fid);
    }

    return 0;
}

// 0x472B54
// 0x472B54
static int _inven_from_button(int keyCode, Object** outItem, Object*** outItemSlot, Object** outOwner)
{
    Object** itemSlot = nullptr;
    Object* owner = nullptr;
    Object* item = nullptr;
    int quantity = 0;

    switch (keyCode) {
    case INVENTORY_HAND_RIGHT_KEY:
        itemSlot = &gInventoryRightHandItem;
        owner = _stack[0];
        item = gInventoryRightHandItem;
        break;
    case INVENTORY_HAND_LEFT_KEY:
        itemSlot = &gInventoryLeftHandItem;
        owner = _stack[0];
        item = gInventoryLeftHandItem;
        break;
    case INVENTORY_ARMOR_KEY:
        itemSlot = &gInventoryArmor;
        owner = _stack[0];
        item = gInventoryArmor;
        break;
    default:
        itemSlot = nullptr;
        owner = nullptr;
        item = nullptr;
        quantity = 0;

        // Player inventory (left panel) - supports combined inventory and filtering
        if (keyCode >= KEYCODE_GRID_BASE && keyCode < KEYCODE_TARGET_GRID_BASE) {
            int slotIndex = keyCode - KEYCODE_GRID_BASE;

            if (gUseCombinedInventory) {
                // Combined inventory with filtering
                int filteredCount = gFilterCategory != -1 ? buildFilteredCombinedIndices() : gCombinedItemCount;
                int filteredIndex = _stack_offset[_curr_stack] + slotIndex;
                if (filteredIndex < filteredCount) {
                    int actualIndex = gFilteredIndices[filteredIndex];
                    CombinedItem* ci = &gCombinedItems[actualIndex];
                    item = ci->item;
                    owner = ci->owner;
                    quantity = ci->quantity;
                    if (owner == nullptr) {
                        owner = _inven_dude;
                    }
                }
            } else {
                // Regular inventory with filtering
                if (gFilterCategory != -1) {
                    int filteredCount = buildFilteredIndices(_pud);
                    int filteredIndex = _stack_offset[_curr_stack] + slotIndex;
                    if (filteredIndex < filteredCount) {
                        int originalIndex = gFilteredIndices[filteredIndex];
                        InventoryItem* invItem = &(_pud->items[originalIndex]);
                        item = invItem->item;
                        owner = _stack[_curr_stack];
                        quantity = invItem->quantity;
                    }
                } else {
                    int index = _stack_offset[_curr_stack] + slotIndex;
                    if (index < _pud->length) {
                        InventoryItem* invItem = &(_pud->items[_pud->length - (index + 1)]);
                        item = invItem->item;
                        owner = _stack[_curr_stack];
                        quantity = invItem->quantity;
                    }
                }
            }
        }
        // Target inventory (loot right panel) - no combined inventory here
        else if (keyCode >= KEYCODE_TARGET_GRID_BASE && keyCode < KEYCODE_OFFER_LEFT_BASE) {
            int slotIndex = keyCode - KEYCODE_TARGET_GRID_BASE;
            if (gFilterCategory != -1) {
                int filteredCount = buildFilteredIndices(_target_pud);
                int filteredIndex = _target_stack_offset[_target_curr_stack] + slotIndex;
                if (filteredIndex < filteredCount) {
                    int originalIndex = gFilteredIndices[filteredIndex];
                    InventoryItem* invItem = &(_target_pud->items[originalIndex]);
                    item = invItem->item;
                    owner = _target_stack[_target_curr_stack];
                    quantity = invItem->quantity;
                }
            } else {
                int index = _target_stack_offset[_target_curr_stack] + slotIndex;
                if (index < _target_pud->length) {
                    InventoryItem* invItem = &(_target_pud->items[_target_pud->length - (index + 1)]);
                    item = invItem->item;
                    owner = _target_stack[_target_curr_stack];
                    quantity = invItem->quantity;
                }
            }
        }
        // Player offer table (trade)
        else if (keyCode >= KEYCODE_OFFER_LEFT_BASE && keyCode < KEYCODE_OFFER_RIGHT_BASE) {
            int slotIndex = keyCode - KEYCODE_OFFER_LEFT_BASE;
            int index = _ptable_offset + slotIndex;
            if (index < _ptable_pud->length) {
                InventoryItem* invItem = &(_ptable_pud->items[_ptable_pud->length - (index + 1)]);
                item = invItem->item;
                owner = _ptable;
                quantity = invItem->quantity;
            }
        }
        // Merchant offer table (trade)
        else if (keyCode >= KEYCODE_OFFER_RIGHT_BASE) {
            int slotIndex = keyCode - KEYCODE_OFFER_RIGHT_BASE;
            int index = _btable_offset + slotIndex;
            if (index < _btable_pud->length) {
                InventoryItem* invItem = &(_btable_pud->items[_btable_pud->length - (index + 1)]);
                item = invItem->item;
                owner = _btable;
                quantity = invItem->quantity;
            }
        }
        break;
    }

    if (outItemSlot != nullptr) {
        *outItemSlot = itemSlot;
    }
    if (outItem != nullptr) {
        *outItem = item;
    }
    if (outOwner != nullptr) {
        *outOwner = owner;
    }

    if (quantity == 0 && item != nullptr) {
        quantity = 1;
    }

    return quantity;
}

// Displays item description.
//
// The [string] is mutated in the process replacing spaces back and forth
// for word wrapping purposes.
//
// inven_display_msg
// 0x472D24
static void inventoryRenderItemDescription(char* string)
{
    int oldFont = fontGetCurrent();
    fontSetCurrent(101);

    unsigned char* windowBuffer = windowGetBuffer(gInventoryWindow);
    windowBuffer += gLayout.windowWidth * gLayout.summaryY + gLayout.summaryX;

    char* c = string;
    while (c != nullptr && *c != '\0') {
        _inven_display_msg_line += 1;
        if (_inven_display_msg_line > 17) {
            debugPrint("\nError: inven_display_msg: out of bounds!");
            return;
        }

        char* space = nullptr;
        if (fontGetStringWidth(c) > 152) {
            // Look for next space.
            space = c + 1;
            while (*space != '\0' && *space != ' ') {
                space += 1;
            }

            if (*space == '\0') {
                // This was the last line containing very long word. Text
                // drawing routine will silently truncate it after reaching
                // desired length.
                fontDrawText(windowBuffer + gLayout.windowWidth * _inven_display_msg_line * fontGetLineHeight(), c, 152, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);
                return;
            }

            char* nextSpace = space + 1;
            while (true) {
                while (*nextSpace != '\0' && *nextSpace != ' ') {
                    nextSpace += 1;
                }

                if (*nextSpace == '\0') {
                    break;
                }

                // Break string and measure it.
                *nextSpace = '\0';
                if (fontGetStringWidth(c) >= 152) {
                    // Next space is too far to fit in one line. Restore next
                    // space's character and stop.
                    *nextSpace = ' ';
                    break;
                }

                space = nextSpace;

                // Restore next space's character and continue looping from the
                // next character.
                *nextSpace = ' ';
                nextSpace += 1;
            }

            if (*space == ' ') {
                *space = '\0';
            }
        }

        if (fontGetStringWidth(c) > 152) {
            debugPrint("\nError: inven_display_msg: word too long!");
            return;
        }

        fontDrawText(windowBuffer + gLayout.windowWidth * _inven_display_msg_line * fontGetLineHeight(), c, 152, gLayout.windowWidth, _colorTable[COL_LIME_GREEN]);

        if (space != nullptr) {
            c = space + 1;
            if (*space == '\0') {
                *space = ' ';
            }
        } else {
            c = nullptr;
        }
    }

    fontSetCurrent(oldFont);
}

// Examines inventory item.
//
// 0x472EB8
static void inventoryExamineItem(Object* critter, Object* item)
{
    int oldFont = fontGetCurrent();
    fontSetCurrent(101);

    unsigned char* windowBuffer = windowGetBuffer(gInventoryWindow);

    // Clear item description area.
    FrmImage backgroundFrmImage;
    int backgroundFid = buildFid(OBJ_TYPE_INTERFACE, gCurrentInventoryBackgroundFrm, 0, 0, 0);
    if (backgroundFrmImage.lock(backgroundFid)) {
        int srcPitch = gLayout.windowWidth;
        int shift = (gInventoryColumns - 1) * gLayout.slotWidth;
        unsigned char* src = backgroundFrmImage.getData() + srcPitch * gLayout.summaryY + (INVENTORY_SUMMARY_X + shift);
        unsigned char* dest = windowBuffer + gLayout.windowWidth * gLayout.summaryY + gLayout.summaryX;
        blitBufferToBuffer(src, INVENTORY_SUMMARY_WIDTH, INVENTORY_SUMMARY_HEIGHT, srcPitch, dest, gLayout.windowWidth);
    }

    // Reset item description lines counter.
    _inven_display_msg_line = 0;

    // Render item's name.
    char* itemName = objectGetName(item);
    inventoryRenderItemDescription(itemName);

    // Increment line counter to accommodate separator below.
    _inven_display_msg_line += 1;

    int lineHeight = fontGetLineHeight();

    // Draw separator.
    // SFALL: Fix separator position when item name is longer than one line.
    bufferDrawLine(windowBuffer,
        gLayout.windowWidth,
        gLayout.summaryX,
        (_inven_display_msg_line - 1) * lineHeight + lineHeight / 2 + 49,
        gLayout.summaryX + 151,
        (_inven_display_msg_line - 1) * lineHeight + lineHeight / 2 + 49,
        _colorTable[COL_LIME_GREEN]);

    // Examine item.
    objectExamineFunc(critter, item, inventoryRenderItemDescription);

    // Add weight if neccessary.
    int weight = itemGetWeight(item);
    if (weight != 0) {
        MessageListItem messageListItem;
        messageListItem.num = 540;

        if (weight == 1) {
            messageListItem.num = 541;
        }

        if (!messageListGetItem(&gProtoMessageList, &messageListItem)) {
            debugPrint("\nError: Couldn't find message!");
        }

        char formattedText[40];
        snprintf(formattedText, sizeof(formattedText), messageListItem.text, weight);
        inventoryRenderItemDescription(formattedText);
    }

    fontSetCurrent(oldFont);
}

// Checks if the given object is the player character
// Used to determine which set of messages to display
static bool _is_player_object(Object* obj)
{
    return (obj == gDude);
}

// Formats a name with proper English possessive ending
// Examples: "Sulik" -> "Sulik's", "Myron's" -> "Myron's" (unchanged)
// Empty names return "Inventory" as fallback
static void _format_possessive_name(char* dest, size_t destSize, const char* name)
{
    if (name == nullptr || strlen(name) == 0) {
        snprintf(dest, destSize, "Inventory");
        return;
    }

    // Remove trailing whitespace
    char cleanName[128];
    snprintf(cleanName, sizeof(cleanName), "%s", name);

    size_t len = strlen(cleanName);
    while (len > 0 && isspace((unsigned char)cleanName[len - 1])) {
        cleanName[--len] = '\0';
    }

    if (len == 0) {
        snprintf(dest, destSize, "Inventory");
        return;
    }

    // Check if name already ends with apostrophe
    char lastChar = cleanName[len - 1];
    if (lastChar == '\'') {
        snprintf(dest, destSize, "%s", cleanName);
        return;
    }

    // Add proper possessive ending
    if (lastChar == 's' || lastChar == 'S') {
        snprintf(dest, destSize, "%s'", cleanName);
    } else {
        snprintf(dest, destSize, "%s's", cleanName);
    }
}

// Shows the appropriate feedback message after sorting
// Uses different message IDs for player vs NPC inventories (400-404 vs 410-414)
// Trade windows display messages differently than other inventory windows
static void _show_sort_message(Object* obj, int sortType, int inventoryWindowType)
{
    if (obj == nullptr) return;

    bool isPlayer = _is_player_object(obj);

    // Map sort type to message ID (400-404 for player, 410-414 for NPC)
    int messageId = 400;

    switch (sortType) {
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_DEFAULT:
        messageId = isPlayer ? 400 : 410; // You organize your backpack by category
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_WEAPONS:
        messageId = isPlayer ? 401 : 411; // You sort your weapons to the top
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_AMMO:
        messageId = isPlayer ? 402 : 412; // You sort your ammo to the top
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_DRUGS:
        messageId = isPlayer ? 403 : 413; // You sort your drugs to the top
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_OTHER:
        messageId = isPlayer ? 404 : 414; // You sort your miscellaneous items to the top
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_WEIGHT:
        messageId = isPlayer ? 405 : 415; // You sort your inventory by weight
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_VALUE:
        messageId = isPlayer ? 406 : 416; // You sort your inventory by value
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_REVERSE:
        messageId = isPlayer ? 407 : 417; // You sort your inventory in reverse order
        break;
    }

    gFissionMessageListItem.num = messageId;

    if (messageListGetItem(&gFissionMessageList, &gFissionMessageListItem)) {
        char finalMessage[256];

        if (isPlayer) {
            snprintf(finalMessage, sizeof(finalMessage), "%s", gFissionMessageListItem.text);
        } else {
            const char* objName = objectGetName(obj);
            if (objName == nullptr) {
                objName = "Inventory";
            }

            char possessiveName[128];
            _format_possessive_name(possessiveName, sizeof(possessiveName), objName);
            snprintf(finalMessage, sizeof(finalMessage), gFissionMessageListItem.text, possessiveName);
        }

        // Trade windows use supplementary message area, others use monitor
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
            gameDialogRenderSupplementaryMessage(finalMessage);
        } else {
            displayMonitorAddMessage(finalMessage);
        }
    }
}

// Shows message when inventory has 0 or 1 items (nothing meaningful to sort)
// Uses message 460 for player, 461 for NPC/container inventories
static void _nothing_to_sort_message(Object* obj, int inventoryWindowType)
{
    if (obj == nullptr) return;

    bool isPlayer = _is_player_object(obj);
    gFissionMessageListItem.num = isPlayer ? 460 : 461;

    if (messageListGetItem(&gFissionMessageList, &gFissionMessageListItem)) {
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
            gameDialogRenderSupplementaryMessage(gFissionMessageListItem.text);
        } else {
            displayMonitorAddMessage(gFissionMessageListItem.text);
        }
    }
}

// ===========================================================================
// Enhanced default sort: Type-specific sorting within categories
// ===========================================================================

// Compare weapons: by average damage descending (highest damage at top) - grenades shunted to bottom
static int _compare_weapons_specific(const void* a, const void* b)
{
    InventoryItem* itemA = (InventoryItem*)a;
    InventoryItem* itemB = (InventoryItem*)b;

    Object* weaponA = itemA->item;
    Object* weaponB = itemB->item;

    if (weaponA == nullptr || weaponB == nullptr) {
        return 0;
    }

    // Check if actual grenade: Throwing skill + grenade damage type
    int skillA = weaponGetSkillForHitMode(weaponA, 0);
    int skillB = weaponGetSkillForHitMode(weaponB, 0);

    int damageTypeA = weaponGetDamageType(nullptr, weaponA);
    int damageTypeB = weaponGetDamageType(nullptr, weaponB);

    bool isGrenadeA = (skillA == SKILL_THROWING) && (damageTypeA == DAMAGE_TYPE_EXPLOSION || damageTypeA == DAMAGE_TYPE_PLASMA || damageTypeA == DAMAGE_TYPE_EMP);

    bool isGrenadeB = (skillB == SKILL_THROWING) && (damageTypeB == DAMAGE_TYPE_EXPLOSION || damageTypeB == DAMAGE_TYPE_PLASMA || damageTypeB == DAMAGE_TYPE_EMP);

    // Grenades at bottom of weapons section
    if (isGrenadeA && !isGrenadeB) {
        return -1; // Grenade before non-grenade = grenades at BOTTOM
    }
    if (!isGrenadeA && isGrenadeB) {
        return 1; // Non-grenade after grenade = grenades at BOTTOM
    }

    // Both same type, sort by damage
    int minDamageA, maxDamageA, minDamageB, maxDamageB;
    weaponGetDamageMinMax(weaponA, &minDamageA, &maxDamageA);
    weaponGetDamageMinMax(weaponB, &minDamageB, &maxDamageB);

    int avgDamageA = (minDamageA + maxDamageA) / 2;
    int avgDamageB = (minDamageB + maxDamageB) / 2;

    // Ascending order: lowest damage first, highest last (at TOP)
    return avgDamageA - avgDamageB;
}

// Compare ammo: by stack size descending (largest stacks at top)
static int _compare_ammo_specific(const void* a, const void* b)
{
    InventoryItem* itemA = (InventoryItem*)a;
    InventoryItem* itemB = (InventoryItem*)b;

    // Ascending order: smallest stacks first in array, largest last (at TOP)
    return itemA->quantity - itemB->quantity;
}

// Compare drugs: healing items first, then by total value descending
static int _compare_drugs_specific(const void* a, const void* b)
{
    InventoryItem* itemA = (InventoryItem*)a;
    InventoryItem* itemB = (InventoryItem*)b;

    Object* drugA = itemA->item;
    Object* drugB = itemB->item;

    if (drugA == nullptr || drugB == nullptr) {
        return 0;
    }

    // Check if healing items
    bool isHealingA = itemIsHealing(drugA->pid);
    bool isHealingB = itemIsHealing(drugB->pid);

    // Healing items first
    if (isHealingA && !isHealingB) {
        return 1; // A (healing) goes after B (non-healing) = healing at END = TOP
    }
    if (!isHealingA && isHealingB) {
        return -1; // A (non-healing) goes before B (healing) = healing at END = TOP
    }

    // Same type, sort by total value Ascending
    int valueA = itemGetCost(drugA) * itemA->quantity;
    int valueB = itemGetCost(drugB) * itemB->quantity;

    return valueA - valueB; // Ascending: lowest value first, highest last (at TOP)
}

// Compare armor: by damage resistance (normal damage type) descending
static int _compare_armor_specific(const void* a, const void* b)
{
    InventoryItem* itemA = (InventoryItem*)a;
    InventoryItem* itemB = (InventoryItem*)b;

    Object* armorA = itemA->item;
    Object* armorB = itemB->item;

    if (armorA == nullptr || armorB == nullptr) {
        return 0;
    }

    // Get damage resistance for normal damage (damageType = 0)
    int drA = armorGetDamageResistance(armorA, 0);
    int drB = armorGetDamageResistance(armorB, 0);

    // Ascending order: lowest DR first in array, highest DR last (at TOP)
    return drA - drB;
}

// Determines the next sort type in the quick-click rotation sequence
// Rotation: Default (Weapons) - Ammo - Drugs - Other - Default (cycle repeats)
static int _get_next_quick_sort_type(int current)
{
    switch (current) {
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_DEFAULT:
        return GAME_MOUSE_ACTION_MENU_ITEM_SORT_AMMO; // Skip weapons (default already does weapons at top)
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_AMMO:
        return GAME_MOUSE_ACTION_MENU_ITEM_SORT_DRUGS;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_DRUGS:
        return GAME_MOUSE_ACTION_MENU_ITEM_SORT_OTHER;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_OTHER:
        return GAME_MOUSE_ACTION_MENU_ITEM_SORT_DEFAULT; // Back to full organization
    default:
        return GAME_MOUSE_ACTION_MENU_ITEM_SORT_DEFAULT; // Fallback
    }
}

// Sort inventory by weight (heaviest to top)
static bool _sort_by_weight(Object* obj, int inventoryWindowType)
{
    if (obj == nullptr) return false;

    Inventory* inventory = &(obj->data.inventory);
    int itemCount = inventory->length;

    if (itemCount <= 1) return false;

    // Handle trade window money special case
    bool isTradeWindow = (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE);
    int itemsToSort = itemCount;

    if (isTradeWindow) {
        // Move money to top without sorting it
        _move_money_to_top(inventory, itemCount);

        // Find where money starts (at the end after moving)
        itemsToSort = itemCount;
        for (int i = 0; i < itemCount; i++) {
            if (inventory->items[i].item->pid == PROTO_ID_MONEY) {
                itemsToSort = i;
                break;
            }
        }
    }

    if (itemsToSort > 1) {
        qsort(inventory->items, itemsToSort, sizeof(InventoryItem), _compare_items_by_weight);
        return true;
    }

    return false;
}

// Sort inventory by value (most valuable items at top)
static bool _sort_by_value(Object* obj, int inventoryWindowType)
{
    if (obj == nullptr) return false;

    Inventory* inventory = &(obj->data.inventory);
    int itemCount = inventory->length;

    if (itemCount <= 1) return false;

    // Handle trade window money special case
    bool isTradeWindow = (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE);
    int itemsToSort = itemCount;

    if (isTradeWindow) {
        // Move money to top without sorting it
        _move_money_to_top(inventory, itemCount);

        // Find where money starts
        itemsToSort = itemCount;
        for (int i = 0; i < itemCount; i++) {
            if (inventory->items[i].item->pid == PROTO_ID_MONEY) {
                itemsToSort = i;
                break;
            }
        }
    }

    if (itemsToSort > 1) {
        qsort(inventory->items, itemsToSort, sizeof(InventoryItem), _compare_items_by_value);
        return true;
    }

    return false;
}

// Reverse the current order of items
static bool _sort_reverse(Object* obj, int inventoryWindowType)
{
    if (obj == nullptr) return false;

    Inventory* inventory = &(obj->data.inventory);
    int itemCount = inventory->length;

    if (itemCount <= 1) return false;

    // Handle trade window money special case
    bool isTradeWindow = (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE);

    if (isTradeWindow) {
        // For trade windows, we need to keep money at top after reverse
        // So we reverse the non-money portion, then move money back to top

        // Find where money starts
        int moneyStart = itemCount;
        for (int i = 0; i < itemCount; i++) {
            if (inventory->items[i].item->pid == PROTO_ID_MONEY) {
                moneyStart = i;
                break;
            }
        }

        // Reverse only the non-money portion
        int itemsToReverse = moneyStart;
        for (int i = 0; i < itemsToReverse / 2; i++) {
            InventoryItem temp = inventory->items[i];
            inventory->items[i] = inventory->items[itemsToReverse - 1 - i];
            inventory->items[itemsToReverse - 1 - i] = temp;
        }

        // Money stays where it was (now at end of reversed section)
        // But we need it at top, so move it
        _move_money_to_top(inventory, itemCount);

    } else {
        // For non-trade windows, simple reverse
        for (int i = 0; i < itemCount / 2; i++) {
            InventoryItem temp = inventory->items[i];
            inventory->items[i] = inventory->items[itemCount - 1 - i];
            inventory->items[itemCount - 1 - i] = temp;
        }
    }

    return true;
}

// Helper to move money to top in trade windows (reusable)
static void _move_money_to_top(Inventory* inventory, int itemCount)
{
    // Count money items
    int moneyCount = 0;
    for (int i = 0; i < itemCount; i++) {
        if (inventory->items[i].item->pid == PROTO_ID_MONEY) {
            moneyCount++;
        }
    }

    if (moneyCount == 0) return;

    // Simple bubble money to the end (top)
    for (int i = 0; i < itemCount - 1; i++) {
        for (int j = 0; j < itemCount - i - 1; j++) {
            bool currentIsMoney = (inventory->items[j].item->pid == PROTO_ID_MONEY);
            bool nextIsMoney = (inventory->items[j + 1].item->pid == PROTO_ID_MONEY);

            if (currentIsMoney && !nextIsMoney) {
                InventoryItem temp = inventory->items[j];
                inventory->items[j] = inventory->items[j + 1];
                inventory->items[j + 1] = temp;
            }
        }
    }
}

// Compares two inventory items for the default "organize" sort
// Items are primarily sorted by type (weapons > armor > ammo > drugs > misc > containers > keys)
// Within type, specific sorting approaches are taken via "_compare_TYPE_specific" functions
static int _compare_items_by_type(const void* a, const void* b)
{
    // Basic null pointer safety
    if (a == nullptr || b == nullptr) {
        return 0;
    }

    InventoryItem* itemA = (InventoryItem*)a;
    InventoryItem* itemB = (InventoryItem*)b;

    if (itemA == nullptr || itemB == nullptr) {
        return 0;
    }

    if (itemA->item == nullptr || itemB->item == nullptr) {
        return 0;
    }

    int typeA = itemGetType(itemA->item);
    int typeB = itemGetType(itemB->item);

    // Display is reversed: array[0] = bottom of screen, array[N-1] = top of screen
    // Lower priority number = higher display position (top of screen)
    int orderA = MAX_SORT_PRIORITY; // Default: bottom of screen
    int orderB = MAX_SORT_PRIORITY; // Default: bottom of screen

    // Map item types to display priority (1 = top, 7 = near bottom)
    switch (typeA) {
    case ITEM_TYPE_WEAPON:
        orderA = 1;
        break; // Top priority - weapons at top
    case ITEM_TYPE_AMMO:
        orderA = 2;
        break;
    case ITEM_TYPE_DRUG:
        orderA = 3;
        break;
    case ITEM_TYPE_ARMOR:
        orderA = 4;
        break;
    case ITEM_TYPE_MISC:
        orderA = 5;
        break;
    case ITEM_TYPE_CONTAINER:
        orderA = 6;
        break;
    case ITEM_TYPE_KEY:
        orderA = 7;
        break;
    default:
        orderA = MAX_SORT_PRIORITY;
        break; // Unknown types at very bottom
    }

    switch (typeB) {
    case ITEM_TYPE_WEAPON:
        orderB = 1;
        break;
    case ITEM_TYPE_AMMO:
        orderB = 2;
        break;
    case ITEM_TYPE_DRUG:
        orderB = 3;
        break;
    case ITEM_TYPE_ARMOR:
        orderB = 4;
        break;
    case ITEM_TYPE_MISC:
        orderB = 5;
        break;
    case ITEM_TYPE_CONTAINER:
        orderB = 6;
        break;
    case ITEM_TYPE_KEY:
        orderB = 7;
        break;
    default:
        orderB = MAX_SORT_PRIORITY;
        break;
    }

    // Different types: sort by display priority
    if (orderA != orderB) {
        // Positive return = A goes after B (higher array index)
        // Negative return = A goes before B (lower array index)
        // Since display is reversed, we want lower order numbers at END of array
        return orderB - orderA;
    }

    // Same type: apply type-specific sorting
    switch (typeA) {
    case ITEM_TYPE_WEAPON:
        return _compare_weapons_specific(a, b);
    case ITEM_TYPE_AMMO:
        return _compare_ammo_specific(a, b);
    case ITEM_TYPE_DRUG:
        return _compare_drugs_specific(a, b);
    case ITEM_TYPE_ARMOR:
        return _compare_armor_specific(a, b);
    default:
        // For other types (MISC, CONTAINER, KEY), sort alphabetically
        const char* nameA = objectGetName(itemA->item);
        const char* nameB = objectGetName(itemB->item);

        if (nameA == nullptr || nameB == nullptr) {
            if (nameA == nullptr && nameB == nullptr) return 0;
            if (nameA == nullptr) return -1; // Null names first
            return 1; // Non-null after null
        }

        return strcmp(nameA, nameB);
    }
}

// Sorts items for the "Other" category (Misc, Containers, Keys, Armor)
// These are items that don't fit in Weapons/Ammo/Drugs but still need organization
static int _compare_items_all_others(const void* a, const void* b)
{
    // Basic null pointer safety
    if (a == nullptr || b == nullptr) {
        return 0;
    }

    InventoryItem* itemA = (InventoryItem*)a;
    InventoryItem* itemB = (InventoryItem*)b;

    if (itemA == nullptr || itemB == nullptr) {
        return 0;
    }

    if (itemA->item == nullptr || itemB->item == nullptr) {
        return 0;
    }

    // For "Other", we bring Misc, Containers, and Keys to the top
    // in this specific order (Misc > Containers > Keys)
    int typeA = itemGetType(itemA->item);
    int typeB = itemGetType(itemB->item);

    // Define order for "Other" items
    // Lower number = higher priority (goes to end of array = top of display)
    int orderA = MAX_SORT_PRIORITY, orderB = MAX_SORT_PRIORITY;

    switch (typeA) {
    case ITEM_TYPE_ARMOR:
        orderA = 1;
        break; // Highest priority
    case ITEM_TYPE_MISC:
        orderA = 2;
        break;
    case ITEM_TYPE_CONTAINER:
        orderA = 3;
        break;
    case ITEM_TYPE_KEY:
        orderA = 4;
        break; // Lowest priority in "Other"
    default:
        orderA = 5;
        break; // Not in "Other" category
    }

    switch (typeB) {
    case ITEM_TYPE_ARMOR:
        orderB = 1;
        break;
    case ITEM_TYPE_MISC:
        orderB = 2;
        break;
    case ITEM_TYPE_CONTAINER:
        orderB = 3;
        break;
    case ITEM_TYPE_KEY:
        orderB = 4;
        break;
    default:
        orderB = 5;
        break;
    }

    if (orderA != orderB) {
        // REVERSED: Higher priority at END (top of display)
        return orderB - orderA;
    }

    // Same "other" category, sort by name
    const char* nameA = objectGetName(itemA->item);
    const char* nameB = objectGetName(itemB->item);

    if (nameA == nullptr || nameB == nullptr) {
        if (nameA == nullptr && nameB == nullptr) return 0;
        if (nameA == nullptr) return -1;
        return 1;
    }

    return strcmp(nameA, nameB);
}

// Simple alphabetical sort by item name
static int _compare_items_by_name(const void* a, const void* b)
{
    InventoryItem* itemA = (InventoryItem*)a;
    InventoryItem* itemB = (InventoryItem*)b;

    const char* nameA = objectGetName(itemA->item);
    const char* nameB = objectGetName(itemB->item);

    return strcmp(nameA, nameB);
}

// Main sorting function - orchestrates all sort types
// Returns true if sorting actually occurred, false if nothing changed
static bool _inven_sort_inventory(Object* obj, int sortType, int inventoryWindowType)
{
    if (obj == nullptr) {
        return false;
    }

    Inventory* inventory = &(obj->data.inventory);
    int itemCount = inventory->length;

    // Check if inventory is empty or has only 1 item
    if (itemCount == 0) {
        return false; // Empty - nothing to sort
    }

    if (itemCount <= 1) {
        return false; // 0 or 1 item - no sorting needed
    }

    // Check if we're in a trade window
    bool isTradeWindow = (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE);

    // ===================================================
    // DEFAULT SORT: Organize by type (weapons, armor, ammo, etc.)
    // ===================================================
    if (sortType == GAME_MOUSE_ACTION_MENU_ITEM_SORT_DEFAULT) {
        // In trade windows, money stays at top without being sorted
        // This preserves the original Fallout 2 trade interface behavior
        if (isTradeWindow) {
            // Count money items
            int moneyCount = 0;
            for (int i = 0; i < itemCount; i++) {
                if (inventory->items[i].item->pid == PROTO_ID_MONEY) {
                    moneyCount++;
                }
            }

            if (moneyCount > 0) {
                // Separate money from other items
                InventoryItem* moneyItems = (InventoryItem*)malloc(sizeof(InventoryItem) * moneyCount);
                InventoryItem* nonMoneyItems = (InventoryItem*)malloc(sizeof(InventoryItem) * (itemCount - moneyCount));

                int moneyIndex = 0;
                int nonMoneyIndex = 0;

                // Split items into money and non-money, preserving original order
                for (int i = 0; i < itemCount; i++) {
                    if (inventory->items[i].item->pid == PROTO_ID_MONEY) {
                        moneyItems[moneyIndex++] = inventory->items[i];
                    } else {
                        nonMoneyItems[nonMoneyIndex++] = inventory->items[i];
                    }
                }

                // Put money at the END of the array (top of display)
                // First copy non-money items (preserving their order)
                for (int i = 0; i < nonMoneyIndex; i++) {
                    inventory->items[i] = nonMoneyItems[i];
                }

                // Then copy money items (preserving their order)
                for (int i = 0; i < moneyIndex; i++) {
                    inventory->items[nonMoneyIndex + i] = moneyItems[i];
                }

                free(moneyItems);
                free(nonMoneyItems);
            }
        }

        // In trade windows, we only sort the non-money portion
        int itemsToSort = itemCount;
        if (isTradeWindow) {
            // Find where money starts (at the end)
            itemsToSort = itemCount;
            for (int i = 0; i < itemCount; i++) {
                if (inventory->items[i].item->pid == PROTO_ID_MONEY) {
                    itemsToSort = i;
                    break;
                }
            }
        }

        // Only sort if we have items to sort
        if (itemsToSort > 1) {
            qsort(inventory->items, itemsToSort, sizeof(InventoryItem), _compare_items_by_type);
            return true; // Sorting happened
        } else {
            return false; // Nothing to sort (only money or 0-1 items)
        }
    }

    // ===================================================
    // CATEGORY SORTS: Weapons, Ammo, Drugs, Other
    // ===================================================

    // Determine which item type we're looking for
    int targetType = -1;

    switch (sortType) {
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_WEAPONS:
        targetType = ITEM_TYPE_WEAPON;
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_AMMO:
        targetType = ITEM_TYPE_AMMO;
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_DRUGS:
        targetType = ITEM_TYPE_DRUG;
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_OTHER:
        // "Other" category is handled separately (Misc, Containers, Keys, Armor)
        break;
    }

    // ===================================================
    // "OTHER" CATEGORY: Misc, Containers, Keys, Armor
    // ===================================================
    if (sortType == GAME_MOUSE_ACTION_MENU_ITEM_SORT_OTHER) {
        // Count items that belong to the "Other" category
        int otherCount = 0;
        for (int i = 0; i < itemCount; i++) {
            int type = itemGetType(inventory->items[i].item);
            if (type == ITEM_TYPE_MISC || type == ITEM_TYPE_CONTAINER || type == ITEM_TYPE_KEY || type == ITEM_TYPE_ARMOR) {
                otherCount++;
            }
        }

        if (otherCount == 0) {
            // No items in the "Other" category
            gFissionMessageListItem.num = 453; // "No miscellaneous items in inventory."
            if (messageListGetItem(&gFissionMessageList, &gFissionMessageListItem)) {
                if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
                    gameDialogRenderSupplementaryMessage(gFissionMessageListItem.text);
                } else {
                    displayMonitorAddMessage(gFissionMessageListItem.text);
                }
            }
            return false; // No sorting happened
        }

        // Separate "other" items from non-"other" items
        InventoryItem* otherItems = (InventoryItem*)malloc(sizeof(InventoryItem) * otherCount);
        InventoryItem* nonOtherItems = (InventoryItem*)malloc(sizeof(InventoryItem) * (itemCount - otherCount));

        int otherIndex = 0;
        int nonOtherIndex = 0;

        for (int i = 0; i < itemCount; i++) {
            int type = itemGetType(inventory->items[i].item);
            if (type == ITEM_TYPE_MISC || type == ITEM_TYPE_CONTAINER || type == ITEM_TYPE_KEY || type == ITEM_TYPE_ARMOR) {
                otherItems[otherIndex++] = inventory->items[i];
            } else {
                nonOtherItems[nonOtherIndex++] = inventory->items[i];
            }
        }

        // Sort "other" items by type and name
        if (otherCount > 1) {
            qsort(otherItems, otherCount, sizeof(InventoryItem), _compare_items_all_others);
        }

        // Combine: non-other items (original order) + other items (sorted)
        // 1. Copy non-other items preserving order
        for (int i = 0; i < nonOtherIndex; i++) {
            inventory->items[i] = nonOtherItems[i];
        }

        // 2. Copy sorted other items
        for (int i = 0; i < otherCount; i++) {
            inventory->items[nonOtherIndex + i] = otherItems[i];
        }

        free(otherItems);
        free(nonOtherItems);

        // Handle money in trade windows (move to top after sorting)
        if (isTradeWindow) {
            // Simple bubble money to the end (top)
            for (int i = 0; i < itemCount - 1; i++) {
                for (int j = 0; j < itemCount - i - 1; j++) {
                    bool currentIsMoney = (inventory->items[j].item->pid == PROTO_ID_MONEY);
                    bool nextIsMoney = (inventory->items[j + 1].item->pid == PROTO_ID_MONEY);

                    if (currentIsMoney && !nextIsMoney) {
                        InventoryItem temp = inventory->items[j];
                        inventory->items[j] = inventory->items[j + 1];
                        inventory->items[j + 1] = temp;
                    }
                }
            }
        }

        return true; // Sorting happened
    }

    // ===================================================
    // SPECIFIC TYPE CATEGORIES: Weapons, Ammo, Drugs
    // ===================================================

    // Check if we have any items of the target type
    bool hasItemsOfType = false;
    for (int i = 0; i < itemCount; i++) {
        if (itemGetType(inventory->items[i].item) == targetType) {
            hasItemsOfType = true;
            break;
        }
    }

    if (!hasItemsOfType) {
        // Show "no items of this type" message
        int noItemsMsgId = 450; // Default: "No weapons in inventory."

        switch (sortType) {
        case GAME_MOUSE_ACTION_MENU_ITEM_SORT_WEAPONS:
            noItemsMsgId = 450; // "No weapons in inventory."
            break;
        case GAME_MOUSE_ACTION_MENU_ITEM_SORT_AMMO:
            noItemsMsgId = 451; // "No ammo in inventory."
            break;
        case GAME_MOUSE_ACTION_MENU_ITEM_SORT_DRUGS:
            noItemsMsgId = 452; // "No drugs in inventory."
            break;
        }

        gFissionMessageListItem.num = noItemsMsgId;
        if (messageListGetItem(&gFissionMessageList, &gFissionMessageListItem)) {
            if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
                gameDialogRenderSupplementaryMessage(gFissionMessageListItem.text);
            } else {
                displayMonitorAddMessage(gFissionMessageListItem.text);
            }
        }
        return false; // No sorting happened
    }

    // Count target items
    int targetCount = 0;
    for (int i = 0; i < itemCount; i++) {
        if (itemGetType(inventory->items[i].item) == targetType) {
            targetCount++;
        }
    }

    // Separate target items from non-target items
    InventoryItem* targetItems = (InventoryItem*)malloc(sizeof(InventoryItem) * targetCount);
    InventoryItem* nonTargetItems = (InventoryItem*)malloc(sizeof(InventoryItem) * (itemCount - targetCount));

    int targetIndex = 0;
    int nonTargetIndex = 0;

    for (int i = 0; i < itemCount; i++) {
        if (itemGetType(inventory->items[i].item) == targetType) {
            targetItems[targetIndex++] = inventory->items[i];
        } else {
            nonTargetItems[nonTargetIndex++] = inventory->items[i];
        }
    }

    // Sort target items alphabetically
    if (targetCount > 1) {
        qsort(targetItems, targetCount, sizeof(InventoryItem), _compare_items_by_name);
    }

    // Combine: non-target items (original order) + target items (sorted alphabetically)
    // 1. Copy non-target items preserving order
    for (int i = 0; i < nonTargetIndex; i++) {
        inventory->items[i] = nonTargetItems[i];
    }

    // 2. Copy sorted target items
    for (int i = 0; i < targetCount; i++) {
        inventory->items[nonTargetIndex + i] = targetItems[i];
    }

    free(targetItems);
    free(nonTargetItems);

    // Handle money in trade windows (move to top after sorting)
    if (isTradeWindow) {
        // Simple bubble money to the end (top)
        for (int i = 0; i < itemCount - 1; i++) {
            for (int j = 0; j < itemCount - i - 1; j++) {
                bool currentIsMoney = (inventory->items[j].item->pid == PROTO_ID_MONEY);
                bool nextIsMoney = (inventory->items[j + 1].item->pid == PROTO_ID_MONEY);

                if (currentIsMoney && !nextIsMoney) {
                    InventoryItem temp = inventory->items[j];
                    inventory->items[j] = inventory->items[j + 1];
                    inventory->items[j + 1] = temp;
                }
            }
        }
    }

    return true; // Sorting happened
}

// Opens the sort context menu for inventory windows
// Follows the same pattern as inventoryWindowOpenContextMenu for consistency
static void inventoryWindowOpenSortContextMenu(int keyCode, int inventoryWindowType)
{
    // First, determine which inventory we're sorting
    Object* inventoryToSort = nullptr;

    // Determine which inventory to sort based on keyCode
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        if (keyCode == INVENTORY_BUTTON_LEFT) {
            inventoryToSort = _stack[_curr_stack]; // Left inventory (player)
        } else if (keyCode == INVENTORY_BUTTON_RIGHT) {
            inventoryToSort = _target_stack[_target_curr_stack]; // Right inventory (NPC)
        } else if (keyCode >= KEYCODE_GRID_BASE && keyCode < KEYCODE_TARGET_GRID_BASE) {
            // Left grid slot (player inventory)
            inventoryToSort = _stack[_curr_stack];
        } else if (keyCode >= KEYCODE_TARGET_GRID_BASE && keyCode < KEYCODE_OFFER_LEFT_BASE) {
            // Right grid slot (NPC inventory)
            inventoryToSort = _target_stack[_target_curr_stack];
        } else if (keyCode >= KEYCODE_OFFER_LEFT_BASE && keyCode < KEYCODE_OFFER_RIGHT_BASE) {
            // Left offer table
            inventoryToSort = _ptable;
        } else if (keyCode >= KEYCODE_OFFER_RIGHT_BASE && keyCode < 2500) {
            // Right offer table
            inventoryToSort = _btable;
        } else {
            inventoryToSort = _stack[_curr_stack];
        }
    } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
        if (keyCode == INVENTORY_BUTTON_LEFT) {
            inventoryToSort = _stack[_curr_stack]; // Player inventory
        } else if (keyCode == INVENTORY_BUTTON_RIGHT) {
            inventoryToSort = _target_stack[_target_curr_stack]; // Container/NPC inventory
        } else if (keyCode >= KEYCODE_GRID_BASE && keyCode < KEYCODE_TARGET_GRID_BASE) {
            // Left grid slot (player inventory)
            inventoryToSort = _stack[_curr_stack];
        } else if (keyCode >= KEYCODE_TARGET_GRID_BASE && keyCode < KEYCODE_OFFER_LEFT_BASE) {
            // Right grid slot (target inventory)
            inventoryToSort = _target_stack[_target_curr_stack];
        } else {
            inventoryToSort = _stack[_curr_stack];
        }
    } else {
        // Normal or Use Item On window - only player inventory
        inventoryToSort = _stack[_curr_stack]; // Player inventory
    }

    // If we couldn't determine which inventory to sort, return
    if (inventoryToSort == nullptr) {
        return;
    }

    // PHASE 1: Click vs Hold Detection (Fallout Pattern)
    // Fallout's UI pattern: quick click = default action, hold = context menu
    int mouseState;

    // Wait for mouse release or hold - EXACTLY like inventoryWindowOpenContextMenu
    do {
        sharedFpsLimiter.mark();
        inputGetInput();

        // Update body display for normal windows (same pattern)
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
            _display_body(-1, INVENTORY_WINDOW_TYPE_NORMAL);
        }

        mouseState = mouseGetEvent();
        if ((mouseState & MOUSE_EVENT_LEFT_BUTTON_UP) != 0) {

            // QUICK CLICK: Rotating sort system

            // Reset rotation if we're sorting a different inventory
            // Reset when reloading inventory?
            if (inventoryToSort != _last_quick_sorted_object) {
                _last_quick_sorted_object = inventoryToSort;
                _next_quick_sort_type = GAME_MOUSE_ACTION_MENU_ITEM_SORT_DEFAULT;
            }

            // Use the current rotation sort type
            int sortTypeToUse = _next_quick_sort_type;

            // Update rotation for next click
            _next_quick_sort_type = _get_next_quick_sort_type(sortTypeToUse);

            bool didSort = false;
            if (gUseCombinedInventory && inventoryToSort == _stack[_curr_stack]) {
                int itemCountBefore = gCombinedItemCount;
                sortCombinedInventory(sortTypeToUse, inventoryWindowType);
                if (itemCountBefore > 1) {
                    gCombinedSortType = sortTypeToUse; // This now includes weight, value, reverse
                    didSort = true;
                } else {
                    didSort = false;
                }
            } else {
                didSort = _inven_sort_inventory(inventoryToSort, sortTypeToUse, inventoryWindowType);
            }

            if (didSort) {
                _show_sort_message(inventoryToSort, sortTypeToUse, inventoryWindowType);
            } else {
                // Check if inventory is empty or has only 1 item
                int itemCount = gUseCombinedInventory ? gCombinedItemCount : (inventoryToSort != nullptr ? inventoryToSort->data.inventory.length : 0);
                if (itemCount <= 1) {
                    _nothing_to_sort_message(inventoryToSort, inventoryWindowType);
                }
            }

            // Refresh display based on window type
            if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_TRADE);
                if (_target_stack[_target_curr_stack] != nullptr) {
                    _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
                }
                inventoryWindowRenderInnerInventories(_barter_back_win, _ptable, _btable, -1);
                _inven_redrawing_after_sort_menu = true;
                _display_body(-1, INVENTORY_WINDOW_TYPE_TRADE);
                if (_target_stack[_target_curr_stack] != nullptr) {
                    _display_body(_target_stack[_target_curr_stack]->fid, INVENTORY_WINDOW_TYPE_TRADE);
                }
                _inven_redrawing_after_sort_menu = false;
                windowRefresh(_barter_back_win);
                windowRefresh(gInventoryWindow);
            } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
                _display_inventory(_stack_offset[_curr_stack], -1, inventoryWindowType);
                if (_target_stack[_target_curr_stack] != nullptr) {
                    _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, inventoryWindowType);
                }
                _inven_redrawing_after_sort_menu = true;
                _display_body(-1, inventoryWindowType);
                if (_target_stack[_target_curr_stack] != nullptr) {
                    _display_body(_target_stack[_target_curr_stack]->fid, inventoryWindowType);
                }
                _inven_redrawing_after_sort_menu = false;
            } else {
                // Normal or Use Item On window
                _display_inventory(_stack_offset[_curr_stack], -1, inventoryWindowType);
                _inven_redrawing_after_sort_menu = true;
                _display_body(-1, inventoryWindowType);
                _inven_redrawing_after_sort_menu = false;
                if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
                    inventoryRenderSummary();
                }
                windowRefresh(gInventoryWindow);
            }

            return; // Quick click handled, exit function
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    } while ((mouseState & MOUSE_EVENT_LEFT_BUTTON_DOWN_REPEAT) != MOUSE_EVENT_LEFT_BUTTON_DOWN_REPEAT);

    // PHASE 2: Mouse was HELD - Open Context Menu

    // Hide cursor for menu mode (shows blank cursor)
    inventorySetCursor(INVENTORY_WINDOW_CURSOR_BLANK);

    // Get mouse position where user clicked
    int screenX, screenY;
    mouseGetPosition(&screenX, &screenY);

    const InventoryWindowDescription* windowDesc = &(gInventoryWindowDescriptions[inventoryWindowType]);
    InventoryCursorData* cursorData = &(gInventoryCursorData[INVENTORY_WINDOW_CURSOR_MENU]);

    // Declare variables that will be used in both branches
    int menuWindow = -1;
    int btn = -1;
    Rect buttonRect;
    int menuButtonHeight = 0;
    int menuButtonWidth = 0;

    // TRADE WINDOW: Special handling because it uses a temporary window
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        Rect bgRect;
        windowGetRect(_barter_back_win, &bgRect);

        // Calculate dynamic Y adjustment for different screen resolutions
        // Formula: 530 - (screenHeight / 2) - from original game code
        // This is needed to correctly display context menu in trade screen
        // Without context menu and mouse will be placed at top of menuWindow
        int screenHeight = _scr_size.bottom - _scr_size.top + 1;
        int yAdjustment = 530 - (screenHeight / 2);

        // Create temporary transparent window over trade window
        menuWindow = windowCreate(bgRect.left, bgRect.top,
            INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH,
            INVENTORY_TRADE_BACKGROUND_WINDOW_HEIGHT,
            258,
            WINDOW_MODAL | WINDOW_MOVE_ON_TOP | WINDOW_TRANSPARENT);

        if (menuWindow == -1) {
            inventorySetCursor(INVENTORY_WINDOW_CURSOR_ARROW);
            return;
        }

        // Clear the temporary window buffer
        memset(windowGetBuffer(menuWindow), 0,
            INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH * INVENTORY_TRADE_BACKGROUND_WINDOW_HEIGHT);

        // Apply Y adjustment for trade window only
        gameMouseSetActionMenuYAdjustment(yAdjustment);
        int absoluteRight = bgRect.left + INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH;
        int absoluteBottom = bgRect.top + 180;

        // Render the sort menu items (6 options including Cancel)
        if (gameMouseRenderActionMenuItems(screenX, screenY, _act_sort, 5,
                absoluteRight, absoluteBottom)
            != 0) {
            gameMouseSetActionMenuYAdjustment(0);
            windowDestroy(menuWindow);
            inventorySetCursor(INVENTORY_WINDOW_CURSOR_ARROW);
            return;
        }

        // Restore Y adjustment for other context menus
        gameMouseSetActionMenuYAdjustment(0);

        // Set up menu button (transparent overlay) for trade window
        int offsetX, offsetY;
        artGetRotationOffsets(cursorData->frm, 0, &offsetX, &offsetY);
        int windowRelativeX = screenX - bgRect.left;
        int windowRelativeY = screenY - bgRect.top;

        buttonRect.left = windowRelativeX - cursorData->width / 2 + offsetX;
        buttonRect.top = windowRelativeY - cursorData->height + 1 + offsetY;
        buttonRect.right = buttonRect.left + cursorData->width - 1;
        buttonRect.bottom = buttonRect.top + cursorData->height - 1;

        // Adjust button to fit trade window boundaries
        menuButtonHeight = cursorData->height;
        if (buttonRect.top + menuButtonHeight > INVENTORY_TRADE_BACKGROUND_WINDOW_HEIGHT) {
            menuButtonHeight = INVENTORY_TRADE_BACKGROUND_WINDOW_HEIGHT - buttonRect.top;
            if (menuButtonHeight < 0) menuButtonHeight = 0;
        }

        menuButtonWidth = cursorData->width;
        if (buttonRect.left < 0) {
            menuButtonWidth += buttonRect.left;
            buttonRect.left = 0;
        }
        if (buttonRect.right >= INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH) {
            menuButtonWidth -= (buttonRect.right - INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH + 1);
        }
        if (menuButtonWidth <= 0) menuButtonWidth = 1;

        // Create transparent button to capture mouse events
        btn = buttonCreate(menuWindow,
            buttonRect.left, buttonRect.top,
            menuButtonWidth, menuButtonHeight,
            -1, -1, -1, -1,
            cursorData->frmData, cursorData->frmData,
            nullptr, BUTTON_FLAG_TRANSPARENT);

        if (btn == -1) {
            windowDestroy(menuWindow);
            inventorySetCursor(INVENTORY_WINDOW_CURSOR_ARROW);
            return;
        }

        windowRefresh(menuWindow);
    }
    // Normal and loot windows (no temporary window needed)
    else {
        Rect windowRect;
        windowGetRect(gInventoryWindow, &windowRect);

        // Render sort menu items directly on the inventory window
        if (gameMouseRenderActionMenuItems(screenX, screenY, _act_sort, 5,
                windowDesc->width + windowRect.left,
                windowDesc->height + windowRect.top)
            != 0) {
            inventorySetCursor(INVENTORY_WINDOW_CURSOR_ARROW);
            return;
        }

        // Set up menu button for non-trade windows
        int offsetX, offsetY;
        artGetRotationOffsets(cursorData->frm, 0, &offsetX, &offsetY);

        buttonRect.left = screenX - windowRect.left - cursorData->width / 2 + offsetX;
        buttonRect.top = screenY - windowRect.top - cursorData->height + 1 + offsetY;
        buttonRect.right = buttonRect.left + cursorData->width - 1;
        buttonRect.bottom = buttonRect.top + cursorData->height - 1;

        menuButtonHeight = cursorData->height;
        if (buttonRect.top + menuButtonHeight > windowDesc->height) {
            menuButtonHeight = windowDesc->height - buttonRect.top;
            if (menuButtonHeight < 0) menuButtonHeight = 0;
        }

        menuButtonWidth = cursorData->width;

        // Create transparent button on the main inventory window
        btn = buttonCreate(gInventoryWindow,
            buttonRect.left, buttonRect.top,
            menuButtonWidth, menuButtonHeight,
            -1, -1, -1, -1,
            cursorData->frmData, cursorData->frmData,
            nullptr, BUTTON_FLAG_TRANSPARENT);

        // Store menu state for non-trade windows only
        // (Trade windows handle state differently with temp window)
        _inven_sort_menu_active = true;
        _inven_sort_menu_button = btn;
        _inven_sort_menu_x = screenX;
        _inven_sort_menu_y = screenY;
        _inven_sort_menu_selected_index = 0; // Default action is first

        windowRefreshRect(gInventoryWindow, &buttonRect);
    }

    // Common Menu Interaction loop
    int menuItemIndex = 0;
    int previousMouseY = screenY;
    bool menuActive = true;
    int currentWindow = (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) ? menuWindow : gInventoryWindow;

    while (menuActive) {
        sharedFpsLimiter.mark();
        inputGetInput();

        // Check for mouse release or escape key
        if ((mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_UP) != 0) {
            menuActive = false;
        }

        if (inputGetInput() == KEY_ESCAPE) {
            menuItemIndex = SORT_MENU_ITEM_COUNT - 1; // Cancel
            menuActive = false;
        }

        if (!menuActive) break;

        int currentX, currentY;
        mouseGetPosition(&currentX, &currentY);

        // Track mouse vertical movement for menu selection
        // Threshold of 10 pixels prevents accidental selection changes
        if (abs(currentY - previousMouseY) > 10) {
            if (currentY > previousMouseY && menuItemIndex < SORT_MENU_ITEM_COUNT - 1) {
                menuItemIndex++; // Move down in menu
            } else if (currentY < previousMouseY && menuItemIndex > 0) {
                menuItemIndex--; // Move up in menu
            }

            // Highlight the new menu item
            gameMouseHighlightActionMenuItemAtIndex(menuItemIndex);
            windowRefreshRect(currentWindow, &buttonRect);

            // Update state for non-trade windows
            if (inventoryWindowType != INVENTORY_WINDOW_TYPE_TRADE) {
                _inven_sort_menu_selected_index = menuItemIndex;
            }

            previousMouseY = currentY;
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    // Cleanup and Selection handling
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        buttonDestroy(btn);
        windowDestroy(menuWindow);
        // Warp mouse to original position for windowed
        if (!gameIsFullscreen()) {
            SDL_WarpMouseInWindow(gSdlWindow, screenX, screenY);
        }
        // Move mouse to original position for fullscreen
        _mouse_set_position(screenX, screenY);

        // Handle trade window selection
        // Clamp the index to the valid range (0 to 4)
        if (menuItemIndex < 0 || menuItemIndex >= SORT_MENU_ITEM_COUNT) {
            menuItemIndex = SORT_MENU_ITEM_COUNT - 1; // maps to Cancel (index 4)
        }
        int selectedAction = _act_sort[menuItemIndex];

        if (selectedAction != GAME_MOUSE_ACTION_MENU_ITEM_CANCEL) {
            // Re-determine which inventory to sort
            Object* inventoryToSort = nullptr;
            if (keyCode == INVENTORY_BUTTON_LEFT) {
                inventoryToSort = _stack[_curr_stack]; // Left inventory (player)
            } else if (keyCode == INVENTORY_BUTTON_RIGHT) {
                inventoryToSort = _target_stack[_target_curr_stack]; // Right inventory (NPC)
            } else if (keyCode >= KEYCODE_GRID_BASE && keyCode < KEYCODE_TARGET_GRID_BASE) {
                inventoryToSort = _stack[_curr_stack];
            } else if (keyCode >= KEYCODE_TARGET_GRID_BASE && keyCode < KEYCODE_OFFER_LEFT_BASE) {
                inventoryToSort = _target_stack[_target_curr_stack];
            } else if (keyCode >= KEYCODE_OFFER_LEFT_BASE && keyCode < KEYCODE_OFFER_RIGHT_BASE) {
                inventoryToSort = _ptable;
            } else if (keyCode >= KEYCODE_OFFER_RIGHT_BASE && keyCode < 2500) {
                inventoryToSort = _btable;
            }

            if (inventoryToSort != nullptr) {
                // Perform the sort based on the selected action
                bool didSort = false;

                if (gUseCombinedInventory && inventoryToSort == _stack[_curr_stack]) {
                    // Left side (combined player inventory)
                    gCombinedSortType = selectedAction;
                    sortCombinedInventory(selectedAction, inventoryWindowType);
                    didSort = true;
                } else {
                    // Right side (target inventory) or individual inventory mode
                    switch (selectedAction) {
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_DEFAULT:
                        didSort = _inven_sort_inventory(inventoryToSort, GAME_MOUSE_ACTION_MENU_ITEM_SORT_DEFAULT, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_WEAPONS:
                        didSort = _inven_sort_inventory(inventoryToSort, GAME_MOUSE_ACTION_MENU_ITEM_SORT_WEAPONS, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_AMMO:
                        didSort = _inven_sort_inventory(inventoryToSort, GAME_MOUSE_ACTION_MENU_ITEM_SORT_AMMO, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_DRUGS:
                        didSort = _inven_sort_inventory(inventoryToSort, GAME_MOUSE_ACTION_MENU_ITEM_SORT_DRUGS, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_OTHER:
                        didSort = _inven_sort_inventory(inventoryToSort, GAME_MOUSE_ACTION_MENU_ITEM_SORT_OTHER, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_WEIGHT:
                        didSort = _sort_by_weight(inventoryToSort, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_VALUE:
                        didSort = _sort_by_value(inventoryToSort, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_REVERSE:
                        didSort = _sort_reverse(inventoryToSort, inventoryWindowType);
                        break;
                    default:
                        break;
                    }
                }

                if (didSort) {
                    _show_sort_message(inventoryToSort, selectedAction, inventoryWindowType);
                    // Reset quick-click rotation to the chosen context menu option
                    // Next quick click will be the next type in rotation from this choice
                    _last_quick_sorted_object = inventoryToSort;
                    _next_quick_sort_type = _get_next_quick_sort_type(selectedAction);
                } else {
                    // Check if inventory is empty or has only 1 item
                    int itemCount = 0;
                    if (gUseCombinedInventory && inventoryToSort == _stack[_curr_stack]) {
                        // Left side (combined inventory)
                        itemCount = gCombinedItemCount;
                    } else if (inventoryToSort != nullptr) {
                        // Right side or individual inventory
                        itemCount = inventoryToSort->data.inventory.length;
                    }
                    if (itemCount <= 1) {
                        _nothing_to_sort_message(inventoryToSort, inventoryWindowType);
                    }
                }
            }
        }

        // ALWAYS refresh the display, even on cancel
        _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_TRADE);
        _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
        inventoryWindowRenderInnerInventories(_barter_back_win, _ptable, _btable, -1);

        _inven_redrawing_after_sort_menu = true;
        _display_body(-1, INVENTORY_WINDOW_TYPE_TRADE);
        _display_body(_target_stack[_target_curr_stack]->fid, INVENTORY_WINDOW_TYPE_TRADE);
        _inven_redrawing_after_sort_menu = false;

        windowRefresh(_barter_back_win);
        windowRefresh(gInventoryWindow);

    } else {
        // Non-trade windows cleanup
        buttonDestroy(btn);
        _inven_sort_menu_active = false;
        _inven_sort_menu_button = -1;

        // Restore background where menu button was
        unsigned char* windowBuffer = windowGetBuffer(gInventoryWindow);
        int srcPitch, destPitch;
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
            srcPitch = gLayout.windowWidth;
            destPitch = gLayout.windowWidth;
        } else {
            srcPitch = windowDesc->width;
            destPitch = windowDesc->width;
        }

        FrmImage backgroundFrmImage;
        int backgroundFid;
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
            backgroundFid = buildFid(OBJ_TYPE_INTERFACE, gCurrentInventoryBackgroundFrm, 0, 0, 0);
        } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
            backgroundFid = buildFid(OBJ_TYPE_INTERFACE, gCurrentLootBackgroundFrm, 0, 0, 0);
        } else {
            backgroundFid = buildFid(OBJ_TYPE_INTERFACE, windowDesc->frmId, 0, 0, 0);
        }
        if (backgroundFrmImage.lock(backgroundFid)) {
            blitBufferToBuffer(backgroundFrmImage.getData() + srcPitch * buttonRect.top + buttonRect.left,
                cursorData->width, menuButtonHeight, srcPitch,
                windowBuffer + destPitch * buttonRect.top + buttonRect.left, destPitch);
        }

        // Warp mouse to original position for windowed
        if (!gameIsFullscreen()) {
            SDL_WarpMouseInWindow(gSdlWindow, screenX, screenY);
        }
        // Move mouse to original position for fullscreen
        _mouse_set_position(screenX, screenY);
        _display_inventory(_stack_offset[_curr_stack], -1, inventoryWindowType);

        // Handle non-trade window selection
        if (menuItemIndex < 0 || menuItemIndex >= SORT_MENU_ITEM_COUNT) {
            menuItemIndex = SORT_MENU_ITEM_COUNT - 1;
        }
        int selectedAction = _act_sort[menuItemIndex];
        if (selectedAction != GAME_MOUSE_ACTION_MENU_ITEM_CANCEL) {
            // Re-determine which inventory to sort
            Object* inventoryToSort = nullptr;
            if (keyCode == INVENTORY_BUTTON_LEFT) {
                inventoryToSort = _stack[_curr_stack]; // Left side (player)
            } else if (keyCode == INVENTORY_BUTTON_RIGHT) {
                if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
                    inventoryToSort = _target_stack[_target_curr_stack]; // Right side (NPC/container)
                } else {
                    inventoryToSort = _stack[_curr_stack];
                }
            } else if (keyCode >= KEYCODE_GRID_BASE && keyCode < KEYCODE_TARGET_GRID_BASE) {
                // Left grid slot
                inventoryToSort = _stack[_curr_stack];
            } else if (keyCode >= KEYCODE_TARGET_GRID_BASE && keyCode < KEYCODE_OFFER_LEFT_BASE) {
                // Right grid slot (loot only)
                if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
                    inventoryToSort = _target_stack[_target_curr_stack];
                } else {
                    inventoryToSort = _stack[_curr_stack];
                }
            } else {
                inventoryToSort = _stack[_curr_stack];
            }
            if (inventoryToSort != nullptr) {
                bool didSort = false;
                if (gUseCombinedInventory && inventoryToSort == _stack[_curr_stack]) {
                    int itemCountBefore = gCombinedItemCount;
                    sortCombinedInventory(selectedAction, inventoryWindowType);
                    // Only store the sort type if there were at least 2 items (sorting actually happened)
                    if (itemCountBefore > 1) {
                        gCombinedSortType = selectedAction; // This includes weight, value, reverse
                        didSort = true;
                    } else {
                        didSort = false; // No items to sort - don't change gCombinedSortType
                    }
                } else {
                    // Right side or individual inventory
                    switch (selectedAction) {
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_DEFAULT:
                        didSort = _inven_sort_inventory(inventoryToSort, GAME_MOUSE_ACTION_MENU_ITEM_SORT_DEFAULT, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_WEAPONS:
                        didSort = _inven_sort_inventory(inventoryToSort, GAME_MOUSE_ACTION_MENU_ITEM_SORT_WEAPONS, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_AMMO:
                        didSort = _inven_sort_inventory(inventoryToSort, GAME_MOUSE_ACTION_MENU_ITEM_SORT_AMMO, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_DRUGS:
                        didSort = _inven_sort_inventory(inventoryToSort, GAME_MOUSE_ACTION_MENU_ITEM_SORT_DRUGS, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_OTHER:
                        didSort = _inven_sort_inventory(inventoryToSort, GAME_MOUSE_ACTION_MENU_ITEM_SORT_OTHER, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_WEIGHT:
                        didSort = _sort_by_weight(inventoryToSort, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_VALUE:
                        didSort = _sort_by_value(inventoryToSort, inventoryWindowType);
                        break;
                    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_REVERSE:
                        didSort = _sort_reverse(inventoryToSort, inventoryWindowType);
                        break;
                    default:
                        break;
                    }
                }

                if (didSort) {
                    _show_sort_message(inventoryToSort, selectedAction, inventoryWindowType);

                    // Reset quick-click rotation to the chosen context menu option
                    // Next quick click will be the next type in rotation from this choice
                    _last_quick_sorted_object = inventoryToSort;
                    _next_quick_sort_type = _get_next_quick_sort_type(selectedAction);
                } else {
                    // Check if inventory is empty or has only 1 item
                    int itemCount = 0;
                    if (gUseCombinedInventory && inventoryToSort == _stack[_curr_stack]) {
                        itemCount = gCombinedItemCount;
                    } else if (inventoryToSort != nullptr) {
                        itemCount = inventoryToSort->data.inventory.length;
                    }
                    if (itemCount <= 1) {
                        _nothing_to_sort_message(inventoryToSort, inventoryWindowType);
                    }
                }
            }
        }

        // ALWAYS refresh the display, even on cancel
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
            _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, inventoryWindowType);
        }

        _display_inventory(_stack_offset[_curr_stack], -1, inventoryWindowType);

        _inven_redrawing_after_sort_menu = true;
        _display_body(-1, inventoryWindowType);

        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT && _target_stack[_target_curr_stack] != nullptr) {
            _display_body(_target_stack[_target_curr_stack]->fid, inventoryWindowType);
        }

        _inven_redrawing_after_sort_menu = false;

        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
            inventoryRenderSummary();
        }
    }

    // Restore cursor (common to both trade and non-trade windows)
    inventorySetCursor(INVENTORY_WINDOW_CURSOR_ARROW);
}

// 0x47304C
static void inventoryWindowOpenContextMenu(int keyCode, int inventoryWindowType)
{
    Object* item;
    Object** itemSlot;
    Object* owner;

    int quantity = _inven_from_button(keyCode, &item, &itemSlot, &owner);
    if (quantity == 0) {
        return;
    }

    int itemType = itemGetType(item);

    int mouseState;
    do {
        sharedFpsLimiter.mark();

        inputGetInput();

        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
            _display_body(-1, INVENTORY_WINDOW_TYPE_NORMAL);
        }

        mouseState = mouseGetEvent();
        if ((mouseState & MOUSE_EVENT_LEFT_BUTTON_UP) != 0) {
            if (inventoryWindowType != INVENTORY_WINDOW_TYPE_NORMAL) {
                objectLookAtFunc(_stack[0], item, gInventoryPrintItemDescriptionHandler);
            } else {
                inventoryExamineItem(_stack[0], item);
            }
            windowRefresh(gInventoryWindow);
            return;
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    } while ((mouseState & MOUSE_EVENT_LEFT_BUTTON_DOWN_REPEAT) != MOUSE_EVENT_LEFT_BUTTON_DOWN_REPEAT);

    inventorySetCursor(INVENTORY_WINDOW_CURSOR_BLANK);

    unsigned char* windowBuffer = windowGetBuffer(gInventoryWindow);

    int x;
    int y;
    mouseGetPosition(&x, &y);

    int actionMenuItemsLength;
    const int* actionMenuItems;
    if (itemType == ITEM_TYPE_WEAPON && weaponCanBeUnloaded(item)) {
        if (inventoryWindowType != INVENTORY_WINDOW_TYPE_NORMAL && objectGetOwner(item) != gDude) {
            actionMenuItemsLength = 3;
            actionMenuItems = _act_weap2;
        } else {
            actionMenuItemsLength = 4;
            actionMenuItems = _act_weap;
        }
    } else {
        if (inventoryWindowType != INVENTORY_WINDOW_TYPE_NORMAL) {
            // SFALL: Fix crash when trying to open bag/backpack on the table
            // in the bartering interface.
            Object* owner = objectGetOwner(item);
            if (owner != gDude) {
                if (itemType == ITEM_TYPE_CONTAINER && (owner == _stack[_curr_stack] || owner == _target_stack[_target_curr_stack])) {
                    actionMenuItemsLength = 3;
                    actionMenuItems = _act_just_use;
                } else {
                    actionMenuItemsLength = 2;
                    actionMenuItems = _act_nothing;
                }
            } else {
                if (itemType == ITEM_TYPE_CONTAINER) {
                    actionMenuItemsLength = 4;
                    actionMenuItems = _act_use;
                } else {
                    actionMenuItemsLength = 3;
                    actionMenuItems = _act_no_use;
                }
            }
        } else {
            if (itemType == ITEM_TYPE_CONTAINER && itemSlot != nullptr) {
                actionMenuItemsLength = 3;
                actionMenuItems = _act_no_use;
            } else {
                if (_obj_action_can_use(item) || _proto_action_can_use_on(item->pid)) {
                    actionMenuItemsLength = 4;
                    actionMenuItems = _act_use;
                } else {
                    actionMenuItemsLength = 3;
                    actionMenuItems = _act_no_use;
                }
            }
        }
    }

    const InventoryWindowDescription* windowDescription = &(gInventoryWindowDescriptions[inventoryWindowType]);

    Rect windowRect;
    windowGetRect(gInventoryWindow, &windowRect);
    int inventoryWindowX = windowRect.left;
    int inventoryWindowY = windowRect.top;

    if (gameMouseRenderActionMenuItems(x, y, actionMenuItems, actionMenuItemsLength,
            windowDescription->width + inventoryWindowX,
            windowDescription->height + inventoryWindowY)
        == -1) {
        inventorySetCursor(INVENTORY_WINDOW_CURSOR_ARROW);
        return;
    }

    InventoryCursorData* cursorData = &(gInventoryCursorData[INVENTORY_WINDOW_CURSOR_MENU]);

    int offsetX;
    int offsetY;
    artGetRotationOffsets(cursorData->frm, 0, &offsetX, &offsetY);

    Rect rect;
    rect.left = x - inventoryWindowX - cursorData->width / 2 + offsetX;
    rect.top = y - inventoryWindowY - cursorData->height + 1 + offsetY;
    rect.right = rect.left + cursorData->width - 1;
    rect.bottom = rect.top + cursorData->height - 1;

    int menuButtonHeight = cursorData->height;
    if (rect.top + menuButtonHeight > windowDescription->height) {
        menuButtonHeight = windowDescription->height - rect.top;
    }

    int btn = buttonCreate(gInventoryWindow,
        rect.left,
        rect.top,
        cursorData->width,
        menuButtonHeight,
        -1,
        -1,
        -1,
        -1,
        cursorData->frmData,
        cursorData->frmData,
        nullptr,
        BUTTON_FLAG_TRANSPARENT);
    windowRefreshRect(gInventoryWindow, &rect);

    int menuItemIndex = 0;
    int previousMouseY = y;
    while ((mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_UP) == 0) {
        sharedFpsLimiter.mark();

        inputGetInput();

        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
            _display_body(-1, INVENTORY_WINDOW_TYPE_NORMAL);
        }

        int x;
        int y;
        mouseGetPosition(&x, &y);
        if (y - previousMouseY > 10 || previousMouseY - y > 10) {
            if (y >= previousMouseY || menuItemIndex <= 0) {
                if (previousMouseY < y && menuItemIndex < actionMenuItemsLength - 1) {
                    menuItemIndex++;
                }
            } else {
                menuItemIndex--;
            }
            gameMouseHighlightActionMenuItemAtIndex(menuItemIndex);
            windowRefreshRect(gInventoryWindow, &rect);
            previousMouseY = y;
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    buttonDestroy(btn);

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        // Trade window restoration
        unsigned char* src = windowGetBuffer(_barter_back_win);
        int pitch = INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH;
        blitBufferToBuffer(src + pitch * rect.top + rect.left + INVENTORY_TRADE_WINDOW_OFFSET,
            cursorData->width,
            menuButtonHeight,
            pitch,
            windowBuffer + windowDescription->width * rect.top + rect.left,
            windowDescription->width);
    } else {
        // Normal and other windows restoration
        int srcPitch, destPitch;
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
            srcPitch = gLayout.windowWidth;
            destPitch = gLayout.windowWidth;
        } else {
            srcPitch = windowDescription->width;
            destPitch = windowDescription->width;
        }

        FrmImage backgroundFrmImage;
        int backgroundFid;
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
            backgroundFid = buildFid(OBJ_TYPE_INTERFACE, gCurrentInventoryBackgroundFrm, 0, 0, 0);
        } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT) {
            backgroundFid = buildFid(OBJ_TYPE_INTERFACE, gCurrentLootBackgroundFrm, 0, 0, 0);
        } else {
            backgroundFid = buildFid(OBJ_TYPE_INTERFACE, windowDescription->frmId, 0, 0, 0);
        }
        if (backgroundFrmImage.lock(backgroundFid)) {
            blitBufferToBuffer(backgroundFrmImage.getData() + srcPitch * rect.top + rect.left,
                cursorData->width,
                menuButtonHeight,
                srcPitch,
                windowBuffer + destPitch * rect.top + rect.left,
                destPitch);
        }
    }
    // Warp mouse to original position for windowed
    if (!gameIsFullscreen()) {
        SDL_WarpMouseInWindow(gSdlWindow, x, y);
    }
    // Move mouse to original position for fullscreen
    _mouse_set_position(x, y);

    _display_inventory(_stack_offset[_curr_stack], -1, inventoryWindowType);

    int actionMenuItem = actionMenuItems[menuItemIndex];
    switch (actionMenuItem) {
    case GAME_MOUSE_ACTION_MENU_ITEM_DROP: {
        if (!item || !owner) break;

        // If the item is in a hand or armor slot
        if (itemSlot != nullptr) {
            // Adjust stats if removing armor
            if (itemSlot == &gInventoryArmor) {
                adjustCritterStatsOnArmorChange(_stack[0], item, nullptr);
            }

            // Add the item to the owner's inventory (it's not there yet)
            if (itemAdd(owner, item, 1) == 0) {
                *itemSlot = nullptr; // clear the slot
                objectDrop(owner, item); // drop from inventory to ground
            } else {
                // Failed to add (over-encumbered etc.) - keep the slot as is.
                // Optionally show a message.
            }
        }
        // If the item is in the inventory grid
        else {
            // Money
            if (item->pid == PROTO_ID_MONEY) {
                if (quantity > 1) {
                    int chosen = inventoryQuantitySelect(INVENTORY_WINDOW_TYPE_MOVE_ITEMS, item, quantity);
                    if (chosen > 0) {
                        if (chosen == 1) {
                            itemSetMoney(item, 1);
                            objectDrop(owner, item);
                        } else {
                            // Remove all but chosen from stack, then drop the remaining chosen
                            if (itemRemove(owner, item, quantity - chosen) == 0) {
                                Object* splitItem;
                                Object** dummySlot;
                                Object* dummyOwner;
                                if (_inven_from_button(keyCode, &splitItem, &dummySlot, &dummyOwner) != 0) {
                                    itemSetMoney(splitItem, chosen);
                                    objectDrop(owner, splitItem);
                                } else {
                                    // Restore if something went wrong
                                    itemAdd(owner, item, quantity - chosen);
                                }
                            }
                        }
                    }
                } else {
                    itemSetMoney(item, 1);
                    objectDrop(owner, item);
                }
            }
            // Explosive (active)
            else if (explosiveIsActiveExplosive(item->pid)) {
                _dropped_explosive = 1;
                objectDrop(owner, item);
            }
            // Regular item (non-money, non-explosive)
            else {
                int quantityToDrop = (quantity > 1)
                    ? inventoryQuantitySelect(INVENTORY_WINDOW_TYPE_MOVE_ITEMS, item, quantity)
                    : quantity;

                if (quantityToDrop != -1) {
                    if (quantityToDrop == quantity) {
                        // Drop the entire stack
                        objectDrop(owner, item);
                    } else {
                        // Drop a portion: use _inven_from_button to get the item repeatedly
                        for (int i = 0; i < quantityToDrop; i++) {
                            Object* tempItem;
                            Object** tempSlot;
                            Object* tempOwner;
                            if (_inven_from_button(keyCode, &tempItem, &tempSlot, &tempOwner) != 0) {
                                objectDrop(tempOwner, tempItem);
                            }
                        }
                    }
                }
            }
        }

        // Rebuild combined inventory if active
        if (gUseCombinedInventory) {
            inventoryBuildCombinedList(gDude);
        }

        break;
    }
    case GAME_MOUSE_ACTION_MENU_ITEM_LOOK:
        if (inventoryWindowType != INVENTORY_WINDOW_TYPE_NORMAL) {
            objectExamineFunc(_stack[0], item, gInventoryPrintItemDescriptionHandler);
        } else {
            inventoryExamineItem(_stack[0], item);
        }
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_USE:
        switch (itemType) {
        case ITEM_TYPE_CONTAINER:
            _container_enter(keyCode, inventoryWindowType);
            break;
        case ITEM_TYPE_DRUG:
            if (drugItemTakeDrug(_stack[0], item)) {
                if (itemSlot != nullptr) {
                    *itemSlot = nullptr;
                } else {
                    itemRemove(owner, item, 1);
                }

                _obj_connect(item, gDude->tile, gDude->elevation, nullptr);
                objectDestroy(item);
            }
            interfaceRenderHitPoints(true);
            break;
        case ITEM_TYPE_WEAPON:
        case ITEM_TYPE_MISC:
            if (itemSlot == nullptr) {
                itemRemove(owner, item, 1);
            }

            UseItemResultCode useResult;
            if (_obj_action_can_use(item)) {
                useResult = objectUseItemInternal(_stack[0], item);
            } else {
                useResult = objectUseItemOnInternal(_stack[0], _stack[0], item);
            }

            if (useResult == USE_ITEM_RESULT_REMOVE) {
                if (itemSlot != nullptr) {
                    *itemSlot = nullptr;
                }

                _obj_connect(item, gDude->tile, gDude->elevation, nullptr);
                objectDestroy(item);
            } else {
                if (itemSlot == nullptr) {
                    itemAdd(owner, item, 1);
                }
            }
        }
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_UNLOAD: {
        if (!item || !owner) break;

        // Ensure it's a weapon with ammo.
        if (itemGetType(item) != ITEM_TYPE_WEAPON || !weaponCanBeUnloaded(item)) break;

        // Use the original unload function to create the ammo object.
        Object* ammo = weaponUnload(item);
        if (ammo == nullptr) {
            // No ammo to unload – optionally play a sound.
            break;
        }

        // The ammo returned by weaponUnload is already disconnected and ready for inventory.
        // Add it to the owner (fallback to player, then destroy if both fail).
        if (itemAdd(owner, ammo, 1) != 0) {
            if (owner != gDude && itemAdd(gDude, ammo, 1) != 0) {
                objectDestroy(ammo);
            }
        }

        // Rebuild combined inventory if active.
        if (gUseCombinedInventory) {
            inventoryBuildCombinedList(gDude);
        }

        // Refresh the display for the current window type.
        _display_inventory(_stack_offset[_curr_stack], -1, inventoryWindowType);
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT || inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
            _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, inventoryWindowType);
        }
        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
            inventoryRenderSummary();
        }
        windowRefresh(gInventoryWindow);

        break;
    }
    default:
        break;
    }

    inventorySetCursor(INVENTORY_WINDOW_CURSOR_ARROW);

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL && actionMenuItem != GAME_MOUSE_ACTION_MENU_ITEM_LOOK) {
        inventoryRenderSummary();
    }

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_LOOT
        || inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, inventoryWindowType);
    }

    _display_inventory(_stack_offset[_curr_stack], -1, inventoryWindowType);

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_TRADE) {
        inventoryWindowRenderInnerInventories(_barter_back_win, _ptable, _btable, -1);
    }

    _adjust_fid();
}

// 0x473904
int inventoryOpenLooting(Object* looter, Object* target)
{
    int arrowFrmIds[INVENTORY_ARROW_FRM_COUNT];
    FrmImage arrowFrmImages[INVENTORY_ARROW_FRM_COUNT];
    MessageListItem messageListItem;

    memcpy(arrowFrmIds, gInventoryArrowFrmIds, sizeof(gInventoryArrowFrmIds));

    if (looter != _inven_dude || target == nullptr) {
        return 0;
    }

    ScopedGameMode gm(GameMode::kLoot);

    if (FID_TYPE(target->fid) == OBJ_TYPE_CRITTER) {
        if (critterFlagCheck(target->pid, CRITTER_NO_STEAL)) {
            messageListItem.num = 50; // You can't find anything to take from that.
            if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                displayMonitorAddMessage(messageListItem.text);
            }
            return 0;
        }
    }

    if (FID_TYPE(target->fid) == OBJ_TYPE_ITEM) {
        if (itemGetType(target) == ITEM_TYPE_CONTAINER) {
            if (target->frame == 0) {
                CacheEntry* handle;
                Art* frm = artLock(target->fid, &handle);
                if (frm != nullptr) {
                    int frameCount = artGetFrameCount(frm);
                    artUnlock(handle);
                    if (frameCount > 1) {
                        return 0;
                    }
                }
            }
        }
    }

    int sid = -1;
    if (!_gIsSteal) {
        if (objectGetSid(target, &sid) != -1) {
            scriptSetObjects(sid, looter, nullptr);
            scriptExecProc(sid, SCRIPT_PROC_PICKUP);

            Script* script;
            if (scriptGetScript(sid, &script) != -1) {
                if (script->scriptOverrides) {
                    return 0;
                }
            }
        }
    }

    if (inventoryCommonInit() == -1) {
        return 0;
    }
    lootWindowOpened = true;

    _target_pud = &(target->data.inventory);
    _target_curr_stack = 0;
    _target_stack_offset[0] = 0;
    _target_stack[0] = target;

    Object* hiddenBox = nullptr;
    if (objectCreateWithFidPid(&hiddenBox, -1, PROTO_ID_JESSE_CONTAINER) == -1) {
        return 0;
    }

    Object* item1 = nullptr;
    Object* item2 = nullptr;
    Object* armor = nullptr;

    if (_gIsSteal) {
        item1 = critterGetItem1(target);
        if (item1 != nullptr) {
            itemRemove(target, item1, 1);
        }

        item2 = critterGetItem2(target);
        if (item2 != nullptr) {
            itemRemove(target, item2, 1);
        }

        armor = critterGetArmor(target);
        if (armor != nullptr) {
            itemRemove(target, armor, 1);
        }
    }

    // Move hidden items to the hidden box *after* assigning to items (for restoration and Goris claws)
    itemMoveAllHidden(target, hiddenBox);

    bool isoWasEnabled = _setup_inventory(INVENTORY_WINDOW_TYPE_LOOT);

    Object** critters = nullptr;
    int critterCount = 0;
    int critterIndex = 0;
    if (!_gIsSteal) {
        if (FID_TYPE(target->fid) == OBJ_TYPE_CRITTER) {
            critterCount = objectListCreate(target->tile, target->elevation, OBJ_TYPE_CRITTER, &critters);
            int endIndex = critterCount - 1;
            for (int index = 0; index < critterCount; index++) {
                Object* critter = critters[index];
                if ((critter->data.critter.combat.results & (DAM_DEAD | DAM_KNOCKED_OUT)) == 0) {
                    critters[index] = critters[endIndex];
                    critters[endIndex] = critter;
                    critterCount--;
                    index--;
                    endIndex--;
                } else {
                    critterIndex++;
                }
            }

            if (critterCount == 1) {
                objectListFree(critters);
                critterCount = 0;
            }

            if (critterCount > 1) {
                int fid;
                int btn;

                // Setup left arrow button.
                fid = buildFid(OBJ_TYPE_INTERFACE, arrowFrmIds[INVENTORY_ARROW_FRM_LEFT_ARROW_UP], 0, 0, 0);
                arrowFrmImages[INVENTORY_ARROW_FRM_LEFT_ARROW_UP].lock(fid);

                fid = buildFid(OBJ_TYPE_INTERFACE, arrowFrmIds[INVENTORY_ARROW_FRM_LEFT_ARROW_DOWN], 0, 0, 0);
                arrowFrmImages[INVENTORY_ARROW_FRM_LEFT_ARROW_DOWN].lock(fid);

                if (arrowFrmImages[INVENTORY_ARROW_FRM_LEFT_ARROW_UP].isLocked() && arrowFrmImages[INVENTORY_ARROW_FRM_LEFT_ARROW_DOWN].isLocked()) {
                    btn = buttonCreate(gInventoryWindow,
                        436,
                        162,
                        20,
                        18,
                        -1,
                        -1,
                        KEY_PAGE_UP,
                        -1,
                        arrowFrmImages[INVENTORY_ARROW_FRM_LEFT_ARROW_UP].getData(),
                        arrowFrmImages[INVENTORY_ARROW_FRM_LEFT_ARROW_DOWN].getData(),
                        nullptr,
                        0);
                    if (btn != -1) {
                        buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
                    }
                }

                // Setup right arrow button.
                fid = buildFid(OBJ_TYPE_INTERFACE, arrowFrmIds[INVENTORY_ARROW_FRM_RIGHT_ARROW_UP], 0, 0, 0);
                arrowFrmImages[INVENTORY_ARROW_FRM_RIGHT_ARROW_UP].lock(fid);

                fid = buildFid(OBJ_TYPE_INTERFACE, arrowFrmIds[INVENTORY_ARROW_FRM_RIGHT_ARROW_DOWN], 0, 0, 0);
                arrowFrmImages[INVENTORY_ARROW_FRM_RIGHT_ARROW_DOWN].lock(fid);

                if (arrowFrmImages[INVENTORY_ARROW_FRM_RIGHT_ARROW_UP].isLocked() && arrowFrmImages[INVENTORY_ARROW_FRM_RIGHT_ARROW_DOWN].isLocked()) {
                    btn = buttonCreate(gInventoryWindow,
                        456,
                        162,
                        20,
                        18,
                        -1,
                        -1,
                        KEY_PAGE_DOWN,
                        -1,
                        arrowFrmImages[INVENTORY_ARROW_FRM_RIGHT_ARROW_UP].getData(),
                        arrowFrmImages[INVENTORY_ARROW_FRM_RIGHT_ARROW_DOWN].getData(),
                        nullptr,
                        0);
                    if (btn != -1) {
                        buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
                    }
                }

                for (int index = 0; index < critterCount; index++) {
                    if (target == critters[index]) {
                        critterIndex = index;
                    }
                }
            }
        }
    }

    _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
    _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_LOOT);
    _display_body(target->fid, INVENTORY_WINDOW_TYPE_LOOT);
    inventorySetCursor(INVENTORY_WINDOW_CURSOR_HAND);

    bool isCaughtStealing = false;
    int stealingXp = 0;
    int stealingXpBonus = 10;
    for (;;) {
        sharedFpsLimiter.mark();

        if (_game_user_wants_to_quit != 0) {
            break;
        }

        if (isCaughtStealing) {
            break;
        }

        int keyCode = inputGetInput();

        if (keyCode == KEY_CTRL_Q || keyCode == KEY_CTRL_X || keyCode == KEY_F10) {
            showQuitConfirmationDialog();
        }

        if (_game_user_wants_to_quit != 0) {
            break;
        }

        if (keyCode == INVENTORY_BUTTON_TAKE_ALL || keyCode == KEY_LOWERCASE_A) {
            if (!_gIsSteal) {
                if (keyCode == KEY_LOWERCASE_A) {
                    soundPlayFile("ib1p1xx1");
                }
                int maxCarryWeight = critterGetStat(looter, STAT_CARRY_WEIGHT);
                int currentWeight = objectGetInventoryWeight(looter);
                int newInventoryWeight = objectGetInventoryWeight(target);
                if (newInventoryWeight <= maxCarryWeight - currentWeight) {
                    itemMoveAll(target, looter);
                    if (gUseCombinedInventory) {
                        inventoryBuildCombinedList(gDude);
                    }
                    if (!settings.enhancements.strict_vanilla) {
                        soundPlayFile("ib1p1xx1");
                    }
                    _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
                    _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_LOOT);

                    // Force weight display update
                    gInventoryWindowDudeRotationTimestamp = 0;
                    _display_body(-1, INVENTORY_WINDOW_TYPE_LOOT);
                    if (_target_stack[_target_curr_stack] != nullptr) {
                        _display_body(_target_stack[_target_curr_stack]->fid, INVENTORY_WINDOW_TYPE_LOOT);
                    }
                } else {
                    messageListItem.num = 31; // Sorry, you cannot carry that much.
                    if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                        showDialogBox(messageListItem.text, nullptr, 0, 169, 117, _colorTable[COL_ORANGE], nullptr, _colorTable[COL_ORANGE], 0);
                    }
                }
            }
        } else if ((keyCode == INVENTORY_BUTTON_DROP_ALL || keyCode == KEY_LOWERCASE_D) && !settings.enhancements.strict_vanilla) {
            if (!_gIsSteal) {
                if (keyCode == KEY_LOWERCASE_D) {
                    soundPlayFile("ib1p1xx1");
                }
                // Move all items from player to target
                itemMoveAll(_stack[0], _target_stack[_target_curr_stack]);
                if (gUseCombinedInventory) {
                    inventoryBuildCombinedList(gDude);
                }
                _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_LOOT);

                // Force weight display update
                gInventoryWindowDudeRotationTimestamp = 0;
                _display_body(-1, INVENTORY_WINDOW_TYPE_LOOT);
                if (_target_stack[_target_curr_stack] != nullptr) {
                    _display_body(_target_stack[_target_curr_stack]->fid, INVENTORY_WINDOW_TYPE_LOOT);
                }
            }
        } else if (keyCode == KEY_ARROW_UP) {
            if (_stack_offset[_curr_stack] > 0) {
                _stack_offset[_curr_stack] -= 1;
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_LOOT);
            }
        } else if (keyCode == KEY_PAGE_UP) {
            if (critterCount != 0) {
                if (critterIndex > 0) {
                    critterIndex -= 1;
                } else {
                    critterIndex = critterCount - 1;
                }

                target = critters[critterIndex];
                _target_pud = &(target->data.inventory);
                _target_stack[0] = target;
                _target_curr_stack = 0;
                _target_stack_offset[0] = 0;
                _display_target_inventory(0, -1, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_LOOT);
                _display_body(target->fid, INVENTORY_WINDOW_TYPE_LOOT);
            }
        } else if (keyCode == KEY_ARROW_DOWN) {
            int totalFiltered = getFilteredCount();
            if (_stack_offset[_curr_stack] + gInventorySlotsCount < totalFiltered) {
                _stack_offset[_curr_stack] += 1;
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_LOOT);
            }
        } else if (keyCode == KEY_PAGE_DOWN) {
            if (critterCount != 0) {
                if (critterIndex < critterCount - 1) {
                    critterIndex += 1;
                } else {
                    critterIndex = 0;
                }

                target = critters[critterIndex];
                _target_pud = &(target->data.inventory);
                _target_stack[0] = target;
                _target_curr_stack = 0;
                _target_stack_offset[0] = 0;
                _display_target_inventory(0, -1, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_LOOT);
                _display_body(target->fid, INVENTORY_WINDOW_TYPE_LOOT);
            }
        } else if (keyCode == KEY_CTRL_ARROW_UP) {
            if (_target_stack_offset[_target_curr_stack] > 0) {
                _target_stack_offset[_target_curr_stack] -= 1;
                _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
                windowRefresh(gInventoryWindow);
            }
        } else if (keyCode == KEY_CTRL_ARROW_DOWN) {
            int filteredCount = buildFilteredIndices(_target_pud);
            if (_target_stack_offset[_target_curr_stack] + gInventorySlotsCount < filteredCount) {
                _target_stack_offset[_target_curr_stack] += 1;
                _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
                windowRefresh(gInventoryWindow);
            }
        } else if (keyCode >= INVENTORY_BUTTON_LEFT && keyCode <= INVENTORY_BUTTON_RIGHT) {
            if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
                // Arrow mode - sort inventory
                inventoryWindowOpenSortContextMenu(keyCode, INVENTORY_WINDOW_TYPE_LOOT);
            } else {
                _container_exit(keyCode, INVENTORY_WINDOW_TYPE_LOOT);
            }
        } else if (keyCode >= KEYCODE_FILTER_BASE && keyCode <= 8004) {
            if (!settings.enhancements.strict_vanilla && settings.enhancements.inventory_filter) {
                // Toggle filter
                int category = keyCode - KEYCODE_FILTER_BASE;
                if (gFilterCategory == category) {
                    gFilterCategory = -1;
                } else {
                    gFilterCategory = category;
                }
                // Reset scroll offsets for both panels
                _stack_offset[_curr_stack] = 0;
                _target_stack_offset[_target_curr_stack] = 0;
                // Refresh both panels
                soundPlayFile("ib1p1xx1");
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_LOOT);
                _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
                // Refresh character bodies
                _display_body(-1, INVENTORY_WINDOW_TYPE_LOOT);
                if (_target_stack[_target_curr_stack] != nullptr) {
                    _display_body(_target_stack[_target_curr_stack]->fid, INVENTORY_WINDOW_TYPE_LOOT);
                }
                windowRefresh(gInventoryWindow);
            }
        } else {
            if ((mouseGetEvent() & MOUSE_EVENT_RIGHT_BUTTON_DOWN) != 0) {
                if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_HAND) {
                    inventorySetCursor(INVENTORY_WINDOW_CURSOR_ARROW);
                } else {
                    inventorySetCursor(INVENTORY_WINDOW_CURSOR_HAND);
                }
            } else if ((mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_DOWN) != 0) {
                if (keyCode >= KEYCODE_GRID_BASE && keyCode < KEYCODE_GRID_BASE + gInventorySlotsCount) {
                    if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
                        inventoryWindowOpenContextMenu(keyCode, INVENTORY_WINDOW_TYPE_LOOT);
                    } else {
                        int slotIndex = keyCode - KEYCODE_GRID_BASE;
                        Object* item = nullptr;
                        Object* owner = nullptr;
                        int quantity = _inven_from_button(keyCode, &item, nullptr, &owner);
                        if (item != nullptr) {
                            _gStealCount += 1;
                            _gStealSize += itemGetSize(item);
                            InventoryMoveResult rc = _move_inventory(item, slotIndex, _target_stack[_target_curr_stack], true, quantity);
                            if (rc == INVENTORY_MOVE_RESULT_CAUGHT_STEALING) {
                                isCaughtStealing = true;
                            } else if (rc == INVENTORY_MOVE_RESULT_SUCCESS) {
                                stealingXp += stealingXpBonus;
                                stealingXpBonus += 10;
                            }
                            if (gUseCombinedInventory) {
                                inventoryBuildCombinedList(gDude);
                            }
                            _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_LOOT);
                        }
                        keyCode = -1;
                    }
                } else if (keyCode >= KEYCODE_TARGET_GRID_BASE && keyCode < KEYCODE_TARGET_GRID_BASE + gInventorySlotsCount) {
                    if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
                        inventoryWindowOpenContextMenu(keyCode, INVENTORY_WINDOW_TYPE_LOOT);
                    } else {
                        int slotIndex = keyCode - KEYCODE_TARGET_GRID_BASE;
                        // Build filtered list for target inventory
                        int filteredCount = buildFilteredIndices(_target_pud);
                        int filteredIndex = _target_stack_offset[_target_curr_stack] + slotIndex;
                        if (filteredIndex < filteredCount) {
                            int originalIndex = gFilteredIndices[filteredIndex];
                            InventoryItem* inventoryItem = &(_target_pud->items[originalIndex]);
                            _gStealCount += 1;
                            _gStealSize += itemGetSize(_stack[_curr_stack]);
                            InventoryMoveResult rc = _move_inventory(inventoryItem->item, slotIndex, _target_stack[_target_curr_stack], false, inventoryItem->quantity);
                            if (rc == INVENTORY_MOVE_RESULT_CAUGHT_STEALING) {
                                isCaughtStealing = true;
                            } else if (rc == INVENTORY_MOVE_RESULT_SUCCESS) {
                                stealingXp += stealingXpBonus;
                                stealingXpBonus += 10;
                            }
                            _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_LOOT);
                        }
                    }
                }
            } else if ((mouseGetEvent() & MOUSE_EVENT_WHEEL) != 0) {
                if (mouseHitTestInWindow(gInventoryWindow, INVENTORY_LOOT_LEFT_SCROLLER_X, INVENTORY_LOOT_LEFT_SCROLLER_Y, INVENTORY_LOOT_LEFT_SCROLLER_MAX_X, gInventorySlotHeight * gInventorySlotsCount + INVENTORY_LOOT_LEFT_SCROLLER_Y)) {
                    int wheelX, wheelY;
                    mouseGetWheel(&wheelX, &wheelY);
                    if (wheelY > 0) {
                        if (_stack_offset[_curr_stack] > 0) {
                            _stack_offset[_curr_stack] -= 1;
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_LOOT);
                        }
                    } else if (wheelY < 0) {
                        int totalFiltered = getFilteredCount();
                        if (_stack_offset[_curr_stack] + gInventorySlotsCount < totalFiltered) {
                            _stack_offset[_curr_stack] += 1;
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_LOOT);
                        }
                    }
                } else if (mouseHitTestInWindow(gInventoryWindow, INVENTORY_LOOT_RIGHT_SCROLLER_X, INVENTORY_LOOT_RIGHT_SCROLLER_Y, INVENTORY_LOOT_RIGHT_SCROLLER_MAX_X, gInventorySlotHeight * gInventorySlotsCount + INVENTORY_LOOT_RIGHT_SCROLLER_Y)) {
                    int wheelX, wheelY;
                    mouseGetWheel(&wheelX, &wheelY);
                    if (wheelY > 0) {
                        if (_target_stack_offset[_target_curr_stack] > 0) {
                            _target_stack_offset[_target_curr_stack] -= 1;
                            _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
                            windowRefresh(gInventoryWindow);
                        }
                    } else if (wheelY < 0) {
                        int totalFiltered = buildFilteredIndices(_target_pud);
                        if (_target_stack_offset[_target_curr_stack] + gInventorySlotsCount < totalFiltered) {
                            _target_stack_offset[_target_curr_stack] += 1;
                            _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
                            windowRefresh(gInventoryWindow);
                        }
                    }
                }
            }
        }
        if (!settings.enhancements.strict_vanilla && settings.enhancements.inventory_filter) {
            int filterCategory = inventoryKeyToFilterCategory(keyCode);
            if (filterCategory != -1) {
                if (gFilterCategory == filterCategory) {
                    gFilterCategory = -1;
                } else {
                    gFilterCategory = filterCategory;
                }
                _stack_offset[_curr_stack] = 0;
                _target_stack_offset[_target_curr_stack] = 0;
                soundPlayFile("ib1p1xx1");
                _display_inventory(0, -1, INVENTORY_WINDOW_TYPE_LOOT);
                _display_target_inventory(0, -1, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
                windowRefresh(gInventoryWindow);
            }
        }

        if (keyCode == KEY_ESCAPE) {
            break;
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    if (critterCount != 0) {
        objectListFree(critters);
    }

    // Move hidden items back from hidden box before restoring to hands/armor slots
    itemMoveAll(hiddenBox, target);

    if (_gIsSteal) {
        if (item1 != nullptr) {
            item1->flags |= OBJECT_IN_LEFT_HAND;
            itemAdd(target, item1, 1);
        }

        if (item2 != nullptr) {
            item2->flags |= OBJECT_IN_RIGHT_HAND;
            itemAdd(target, item2, 1);
        }

        if (armor != nullptr) {
            armor->flags |= OBJECT_WORN;
            itemAdd(target, armor, 1);
        }
    }

    objectDestroy(hiddenBox, nullptr);

    if (_gIsSteal && !isCaughtStealing && stealingXp > 0 && !objectIsPartyMember(target)) {
        stealingXp = std::min(300 - skillGetValue(looter, SKILL_STEAL), stealingXp);
        debugPrint("\n[[[%d]]]", 300 - skillGetValue(looter, SKILL_STEAL));

        // SFALL: Display actual xp received.
        int xpGained;
        pcAddExperience(stealingXp, &xpGained);

        messageListItem.num = 29; // You gain %d experience points for successfully using your Steal skill.

        if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
            char formattedText[200];
            snprintf(formattedText, sizeof(formattedText), messageListItem.text, xpGained);
            displayMonitorAddMessage(formattedText);
        }
    }

    _exit_inventory(isoWasEnabled);

    // NOTE: Uninline.
    inventoryCommonFree();

    // wrap this in strict_vanilla later
    if (_gIsSteal && isCaughtStealing && _gStealCount > 0) {
        if (objectIsPartyMember(target)) {
            // Open dialogue with the companion instead of combat
            gameDialogEnter(target, 0);
        } else if (objectGetSid(target, &sid) != -1) {
            scriptSetObjects(sid, looter, nullptr);
            scriptExecProc(sid, SCRIPT_PROC_PICKUP);
            Script* script;
            scriptGetScript(sid, &script);
        }
    }

    // Mark as opened/looted after the loot window is fully closed
    if (lootWindowOpened && target != nullptr && isObjectValid(target)) {
        int type = FID_TYPE(target->fid);
        if (type == OBJ_TYPE_ITEM && itemGetType(target) == ITEM_TYPE_CONTAINER) {
            target->flags |= OBJECT_OPENED;
        } else if (type == OBJ_TYPE_CRITTER && critterIsDead(target)) {
            target->flags |= OBJECT_OPENED;
        }
    }

    return 0;
}

// 0x4746A0
int inventoryOpenStealing(Object* thief, Object* target)
{
    if (thief == target) {
        return -1;
    }

    _gIsSteal = PID_TYPE(thief->pid) == OBJ_TYPE_CRITTER && critterIsActive(target);
    _gStealCount = 0;
    _gStealSize = 0;

    int rc = inventoryOpenLooting(thief, target);

    _gIsSteal = 0;
    _gStealCount = 0;
    _gStealSize = 0;

    return rc;
}

// 0x474708
// note: this is looting and stealing, not the inventory screen
static InventoryMoveResult _move_inventory(Object* item, int slotIndex, Object* targetObj, bool isPlanting, int quantity)
{
    bool needRefresh = true;
    Rect rect;
    Object* owner = nullptr;

    // Helper to get the actual item from the left panel
    auto getLeftItem = [&](int slotIndex, int stackOffset, Object*& outItem, int& outQuantity, Object*& outOwner) -> bool {
        if (gFilterCategory != -1) {
            // Build filtered indices for the left inventory.
            int filteredCount;
            if (gUseCombinedInventory) {
                filteredCount = buildFilteredCombinedIndices(); // fills gFilteredIndices
            } else {
                filteredCount = buildFilteredIndices(_pud);
            }
            int filteredIndex = stackOffset + slotIndex;
            if (filteredIndex >= filteredCount) {
                return false;
            }
            int actualIndex = gFilteredIndices[filteredIndex];
            if (gUseCombinedInventory) {
                if (actualIndex < 0 || actualIndex >= gCombinedItemCount) return false;
                CombinedItem* ci = &gCombinedItems[actualIndex];
                outItem = ci->item;
                outQuantity = ci->quantity;
                outOwner = ci->owner;
            } else {
                if (actualIndex < 0 || actualIndex >= _pud->length) return false;
                InventoryItem* invItem = &(_pud->items[actualIndex]);
                outItem = invItem->item;
                outQuantity = invItem->quantity;
                outOwner = _inven_dude;
            }
        } else {
            // No filter: use reversed order.
            if (gUseCombinedInventory) {
                int itemIndex = stackOffset + slotIndex;
                if (itemIndex >= gCombinedItemCount) return false;
                int actualIndex = gCombinedItemCount - (itemIndex + 1);
                CombinedItem* ci = &gCombinedItems[actualIndex];
                outItem = ci->item;
                outQuantity = ci->quantity;
                outOwner = ci->owner;
            } else {
                int actualIndex = _pud->length - (slotIndex + stackOffset + 1);
                if (actualIndex < 0 || actualIndex >= _pud->length) return false;
                InventoryItem* invItem = &(_pud->items[actualIndex]);
                outItem = invItem->item;
                outQuantity = invItem->quantity;
                outOwner = _inven_dude;
            }
        }
        return (outItem != nullptr);
    };

    // Helper to get the actual item from the right panel
    auto getRightItem = [&](int slotIndex, int stackOffset, Object*& outItem, int& outQuantity, Object*& outOwner) -> bool {
        if (gFilterCategory != -1) {
            int filteredCount = buildFilteredIndices(_target_pud);
            int filteredIndex = stackOffset + slotIndex;
            if (filteredIndex >= filteredCount) return false;
            int actualIndex = gFilteredIndices[filteredIndex];
            if (actualIndex < 0 || actualIndex >= _target_pud->length) return false;
            InventoryItem* invItem = &(_target_pud->items[actualIndex]);
            outItem = invItem->item;
            outQuantity = invItem->quantity;
            outOwner = targetObj; // source is the target object
        } else {
            int actualIndex = _target_pud->length - (slotIndex + stackOffset + 1);
            if (actualIndex < 0 || actualIndex >= _target_pud->length) return false;
            InventoryItem* invItem = &(_target_pud->items[actualIndex]);
            outItem = invItem->item;
            outQuantity = invItem->quantity;
            outOwner = targetObj;
        }
        return (outItem != nullptr);
    };

    // Retrieve the item based on directoin
    if (isPlanting) {
        // From left (player/combined) to right (target)
        rect.left = INVENTORY_LOOT_LEFT_SCROLLER_X;
        rect.top = gInventorySlotHeight * slotIndex + INVENTORY_LOOT_LEFT_SCROLLER_Y;

        if (!getLeftItem(slotIndex, _stack_offset[_curr_stack], item, quantity, owner)) {
            return INVENTORY_MOVE_RESULT_FAILED;
        }

        if (quantity > 1) {
            _display_inventory(_stack_offset[_curr_stack], slotIndex, INVENTORY_WINDOW_TYPE_LOOT);
            needRefresh = false;
        }
    } else {
        // From right (target) to left (player/combined)
        rect.left = INVENTORY_LOOT_RIGHT_SCROLLER_X;
        rect.top = gInventorySlotHeight * slotIndex + INVENTORY_LOOT_RIGHT_SCROLLER_Y;

        if (!getRightItem(slotIndex, _target_stack_offset[_target_curr_stack], item, quantity, owner)) {
            return INVENTORY_MOVE_RESULT_FAILED;
        }

        if (quantity > 1) {
            _display_target_inventory(_target_stack_offset[_target_curr_stack], slotIndex, _target_pud, INVENTORY_WINDOW_TYPE_LOOT);
            windowRefresh(gInventoryWindow);
            needRefresh = false;
        }
    }

    // Erase the single slot background if we didn't redraw the whole side
    if (needRefresh) {
        unsigned char* windowBuffer = windowGetBuffer(gInventoryWindow);
        FrmImage backgroundFrmImage;
        int backgroundFid = buildFid(OBJ_TYPE_INTERFACE, gCurrentLootBackgroundFrm, 0, 0, 0);
        if (backgroundFrmImage.lock(backgroundFid)) {
            blitBufferToBuffer(backgroundFrmImage.getData() + INVENTORY_LOOT_WINDOW_WIDTH * rect.top + rect.left,
                INVENTORY_SLOT_WIDTH, gInventorySlotHeight,
                INVENTORY_LOOT_WINDOW_WIDTH,
                windowBuffer + INVENTORY_LOOT_WINDOW_WIDTH * rect.top + rect.left,
                INVENTORY_LOOT_WINDOW_WIDTH);
        }
        rect.right = rect.left + INVENTORY_SLOT_WIDTH - 1;
        rect.bottom = rect.top + gInventorySlotHeight - 1;
        windowRefreshRect(gInventoryWindow, &rect);
    }

    bool immediate = false;
    _drag_item_loop(item, immediate);

    InventoryMoveResult result = INVENTORY_MOVE_RESULT_FAILED;
    MessageListItem messageListItem;

    if (isPlanting) {
        // Drop on right side
        if (immediate || mouseHitTestInWindow(gInventoryWindow, INVENTORY_LOOT_RIGHT_SCROLLER_X, INVENTORY_LOOT_RIGHT_SCROLLER_Y, INVENTORY_LOOT_RIGHT_SCROLLER_MAX_X, gInventorySlotHeight * gInventorySlotsCount + INVENTORY_LOOT_RIGHT_SCROLLER_Y)) {
            int quantityToMove = (quantity > 1 && !immediate)
                ? inventoryQuantitySelect(INVENTORY_WINDOW_TYPE_MOVE_ITEMS, item, quantity)
                : quantity;
            if (quantityToMove != -1) {
                if (_gIsSteal) {
                    if (skillsPerformStealing(_inven_dude, targetObj, item, true) == 0) {
                        result = INVENTORY_MOVE_RESULT_CAUGHT_STEALING;
                    }
                }
                if (result != INVENTORY_MOVE_RESULT_CAUGHT_STEALING) {
                    if (itemMove(owner, targetObj, item, quantityToMove) != -1) {
                        result = INVENTORY_MOVE_RESULT_SUCCESS;
                    } else {
                        messageListItem.num = 26; // There is no space left for that item.}
                        if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                            displayMonitorAddMessage(messageListItem.text);
                        }
                    }
                }
            }
        }
    } else {
        // Drop on left side
        if (immediate || mouseHitTestInWindow(gInventoryWindow, INVENTORY_LOOT_LEFT_SCROLLER_X, INVENTORY_LOOT_LEFT_SCROLLER_Y, INVENTORY_LOOT_LEFT_SCROLLER_MAX_X, gInventorySlotHeight * gInventorySlotsCount + INVENTORY_LOOT_LEFT_SCROLLER_Y)) {
            int quantityToMove = (quantity > 1 && !immediate)
                ? inventoryQuantitySelect(INVENTORY_WINDOW_TYPE_MOVE_ITEMS, item, quantity)
                : quantity;

            if (quantityToMove != -1) {
                if (_gIsSteal) {
                    if (skillsPerformStealing(_inven_dude, targetObj, item, false) == 0) {
                        result = INVENTORY_MOVE_RESULT_CAUGHT_STEALING;
                    }
                }

                if (result != INVENTORY_MOVE_RESULT_CAUGHT_STEALING) {
                    // Move from targetObj to the player (or combined inventory owner)
                    Object* dest = _inven_dude;
                    if (itemMove(targetObj, dest, item, quantityToMove) == 0) {
                        if ((item->flags & OBJECT_IN_RIGHT_HAND) != 0) {
                            targetObj->fid = buildFid(FID_TYPE(targetObj->fid), artGetIndex(targetObj->fid),
                                FID_ANIM_TYPE(targetObj->fid), 0, targetObj->rotation + 1);
                        }
                        targetObj->flags &= ~OBJECT_EQUIPPED;
                        result = INVENTORY_MOVE_RESULT_SUCCESS;
                    } else {
                        messageListItem.num = 25; // You cannot pick that up. You are at your maximum weight capacity.
                        if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                            displayMonitorAddMessage(messageListItem.text);
                        }
                    }
                }
            }
        }
    }

    // Rebuild combined inventory list if active
    if (gUseCombinedInventory) {
        inventoryBuildCombinedList(gDude);
    }

    // Refresh body displays (weight info)
    inventoryRefreshBodies(INVENTORY_WINDOW_TYPE_LOOT);
    inventorySetCursor(INVENTORY_WINDOW_CURSOR_HAND);

    return result;
}

static int _barter_compute_value_original(Object* dude, Object* npc)
{
    if (gGameDialogSpeakerIsPartyMember) {
        return objectGetInventoryWeight(_btable);
    }

    int cost = objectGetCost(_btable);
    int caps = itemGetTotalCaps(_btable);
    int costWithoutCaps = cost - caps;

    double perkBonus = 0.0;
    if (dude == gDude) {
        if (perkHasRank(gDude, PERK_MASTER_TRADER)) {
            perkBonus = 25.0;
        }
    }

    int partyBarter = partyGetBestSkillValue(SKILL_BARTER);
    int npcBarter = skillGetValue(npc, SKILL_BARTER);

    // TODO: Check in debugger, complex math, probably uses floats, not doubles.
    double barterModMult = (_barter_mod + 100.0 - perkBonus) * 0.01;
    double balancedCost = (160.0 + npcBarter) / (160.0 + partyBarter) * (costWithoutCaps * 2.0);
    if (barterModMult < 0) {
        // TODO: Probably 0.01 as float.
        barterModMult = 0.0099999998;
    }

    int rounded = (int)(barterModMult * balancedCost + caps);
    return rounded;
}

static int _barter_compute_value_enhanced(Object* dude, Object* npc)
{
    if (gGameDialogSpeakerIsPartyMember) {
        return objectGetInventoryWeight(_btable);
    }

    int baseTrueValue = objectGetCost(_btable);
    int caps = itemGetTotalCaps(_btable);
    int costWithoutCaps = baseTrueValue - caps;

    // Reaction modifiers
    // Ensure _barter_mod can't override skill dominance
    double perkBonus = (dude == gDude && perkHasRank(gDude, PERK_MASTER_TRADER)) ? 25.0 : 0.0;
    _barter_mod = std::clamp(_barter_mod, -35, 35); // Hard cap on reaction impact

    // Apply reaction modifiers to NPC's and PC's effective skill
    int npcBarter = skillGetValue(npc, SKILL_BARTER) + _barter_mod;
    int playerBarter = partyGetBestSkillValue(SKILL_BARTER) + perkBonus;

    // Calculate price modification
    double skillRatio = (double)(160 + npcBarter) / (160 + playerBarter);
    double priceMod = 1.15 * skillRatio;

    // Price bounds for better skill progression
    if (priceMod < 0.75)
        priceMod = 0.75; // 25% max discount
    if (priceMod > 1.6)
        priceMod = 1.6; // 60% max markup

    return (int)(costWithoutCaps * priceMod) + caps;
}

// Unified entry point
int _barter_compute_value(Object* dude, Object* npc)
{
    return (settings.enhancements.enhanced_barter && !settings.enhancements.strict_vanilla)
        ? _barter_compute_value_enhanced(dude, npc)
        : _barter_compute_value_original(dude, npc);
}

static int _barter_attempt_transaction_original(Object* dude, Object* offerTable, Object* npc, Object* barterTable)
{
    MessageListItem messageListItem;

    int weightAvailable = critterGetStat(dude, STAT_CARRY_WEIGHT) - objectGetInventoryWeight(dude);
    if (objectGetInventoryWeight(barterTable) > weightAvailable) {
        messageListItem.num = 31; // Sorry, you cannot carry that much.

        if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
            gameDialogRenderSupplementaryMessage(messageListItem.text);
        }
        return -1;
    }

    if (gGameDialogSpeakerIsPartyMember) {
        int npcWeightAvailable = critterGetStat(npc, STAT_CARRY_WEIGHT) - objectGetInventoryWeight(npc);
        if (objectGetInventoryWeight(offerTable) > npcWeightAvailable) {
            messageListItem.num = 32; // Sorry, that's too much to carry.
            if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                gameDialogRenderSupplementaryMessage(messageListItem.text);
            }
            return -1;
        }
    } else {
        bool badOffer = false;
        if (offerTable->data.inventory.length == 0) {
            badOffer = true;
        } else {
            if (itemIsQueued(offerTable)) {
                if (offerTable->pid != PROTO_ID_GEIGER_COUNTER_I || miscItemTurnOff(offerTable) == -1) {
                    badOffer = true;
                }
            }
        }

        if (!badOffer) {
            int cost = objectGetCost(offerTable);
            if (_barter_compute_value(dude, npc) > cost) {
                badOffer = true;
            }
        }

        if (badOffer) {
            messageListItem.num = 28; // No, your offer is not good enough.
            if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                gameDialogRenderSupplementaryMessage(messageListItem.text);
            }
            return -1;
        }
    }

    itemMoveAll(barterTable, dude);
    itemMoveAll(offerTable, npc);
    return 0;
}

static int _barter_attempt_transaction_enhanced(Object* dude, Object* offerTable, Object* npc, Object* barterTable)
{
    MessageListItem messageListItem;

    // Weight checks for companion trades
    int weightAvailable = critterGetStat(dude, STAT_CARRY_WEIGHT) - objectGetInventoryWeight(dude);
    if (objectGetInventoryWeight(barterTable) > weightAvailable) {
        messageListItem.num = 31; // Sorry, you cannot carry that much.
        if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
            gameDialogRenderSupplementaryMessage(messageListItem.text);
        }
        return -1;
    }

    if (gGameDialogSpeakerIsPartyMember) {
        int npcWeightAvailable = critterGetStat(npc, STAT_CARRY_WEIGHT) - objectGetInventoryWeight(npc);
        if (objectGetInventoryWeight(offerTable) > npcWeightAvailable) {
            messageListItem.num = 32; // Sorry, that's too much to carry.
            if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                gameDialogRenderSupplementaryMessage(messageListItem.text);
            }
            return -1;
        }
    } else {
        bool badOffer = false;
        if (offerTable->data.inventory.length == 0) {
            badOffer = true;
        } else if (itemIsQueued(offerTable)) {
            if (offerTable->pid == PROTO_ID_GEIGER_COUNTER_I) {
                if (miscItemTurnOff(offerTable) == -1) {
                    // Could not turn off the Geiger Counter - reject with message
                    messageListItem.num = 36; // "Turn that gadget off first. Then well get down to business."
                    if (messageListGetItem(&gFissionMessageList, &gFissionMessageListItem)) {
                        gameDialogRenderSupplementaryMessage(gFissionMessageListItem.text);
                    }
                    return -1;
                }
            } else {
                // All other active/queued items are rejected
                messageListItem.num = 37; // "I don't deal in gadgets like that. Take it off the table."
                if (messageListGetItem(&gFissionMessageList, &gFissionMessageListItem)) {
                    gameDialogRenderSupplementaryMessage(gFissionMessageListItem.text);
                }
                return -1;
            }
        }

        if (!badOffer) {
            int baseTrueValue = objectGetCost(barterTable);
            int displayedPrice = _barter_compute_value(dude, npc);
            int playerOffer = objectGetCost(offerTable);
            double perkBonus = (dude == gDude && perkHasRank(gDude, PERK_MASTER_TRADER)) ? 25.0 : 0.0;

            // Apply reaction modifiers
            int npcBarter = skillGetValue(npc, SKILL_BARTER) + _barter_mod;
            int playerBarter = partyGetBestSkillValue(SKILL_BARTER) + perkBonus;
            int barterDifference = playerBarter - npcBarter; // Range: -200 to +200

            // Dynamic threshold based on skill difference
            int minAcceptablePercent = 90 - (barterDifference * 30) / 200; // 60% to 90%
            minAcceptablePercent = std::clamp(minAcceptablePercent, 60, 90);
            int minAcceptablePrice = ((displayedPrice + gBarterInsultIncrease) * minAcceptablePercent) / 100;

            // Insult threshold scales similarly but with wider range (40-80% of base)
            int insultPercent = 80 - (barterDifference * 40) / 200; // 40% to 80%
            insultPercent = std::clamp(insultPercent, 40, 80);
            int insultThreshold = (baseTrueValue * insultPercent) / 100;

            // Calculate intermediate thresholds for additional feedback levels
            int seriousThreshold = (minAcceptablePrice + insultThreshold) / 2; // Midpoint between insult and min acceptable
            int almostDealThreshold = (minAcceptablePrice + seriousThreshold) / 2; // Midpiont between serious and min acceptable

            if (playerOffer >= displayedPrice) {
                gBarterInsultIncrease = 0;
            } else if (playerOffer >= minAcceptablePrice) {
                gBarterInsultIncrease = 0;
            } else {
                badOffer = true;

                if (playerOffer < insultThreshold) {
                    gBarterInsultIncrease += baseTrueValue * 10 / 100; // increases minAcceptablePrice by 10%
                    messageListItem.num = 33; // "Your offer is insulting."
                    if (messageListGetItem(&gFissionMessageList, &gFissionMessageListItem)) {
                        gameDialogRenderSupplementaryMessage(gFissionMessageListItem.text);
                    }
                    return -1;
                } else if (playerOffer < seriousThreshold) {
                    gBarterInsultIncrease += baseTrueValue * 2 / 100; // increases minAcceptablePrice by 2%
                    messageListItem.num = 34; // "Let's be serious here."
                    if (messageListGetItem(&gFissionMessageList, &gFissionMessageListItem)) {
                        gameDialogRenderSupplementaryMessage(gFissionMessageListItem.text);
                    }
                    return -1;
                } else if (playerOffer < almostDealThreshold) {
                    messageListItem.num = 35; // "We almost have a deal..."
                    if (messageListGetItem(&gFissionMessageList, &gFissionMessageListItem)) {
                        gameDialogRenderSupplementaryMessage(gFissionMessageListItem.text);
                    }
                    return -1;
                }

                if (gBarterInsultIncrease > baseTrueValue * 25 / 100) {
                    gBarterInsultIncrease = baseTrueValue * 25 / 100; // caps insult minAcceptablePrice increase at 25%
                }
            }
        }

        if (badOffer) {
            messageListItem.num = 28; // No, your offer is not good enough.
            if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                gameDialogRenderSupplementaryMessage(messageListItem.text);
            }
            return -1;
        }
    }

    // Successful trade
    itemMoveAll(barterTable, dude);
    itemMoveAll(offerTable, npc);
    gBarterInsultIncrease = 0; // Reset on successful trade
    return 0;
}

int _barter_attempt_transaction(Object* dude, Object* offerTable, Object* npc, Object* barterTable)
{
    return (settings.enhancements.enhanced_barter && !settings.enhancements.strict_vanilla)
        ? _barter_attempt_transaction_enhanced(dude, offerTable, npc, barterTable)
        : _barter_attempt_transaction_original(dude, offerTable, npc, barterTable);
}

static int _barter_get_quantity_moved_items(Object* item, int maxQuantity, bool fromPlayer, bool fromInventory, bool immediate)
{
    // StrictVanilla override: use original simple selection
    if (settings.enhancements.strict_vanilla) {
        if (maxQuantity <= 1) {
            return maxQuantity;
        }
        // Original behavior
        return inventoryQuantitySelect(INVENTORY_WINDOW_TYPE_MOVE_ITEMS, item, maxQuantity);
    }

    // Eenhanced logic
    if (maxQuantity <= 1) {
        return maxQuantity;
    }

    int suggestedValue = 1;
    if (item->pid == PROTO_ID_MONEY && !gGameDialogSpeakerIsPartyMember) {
        // Calculate change money automatically
        int totalCostPlayer = objectGetCost(_ptable);
        int totalCostNpc = _barter_compute_value(gDude, _target_stack[0]);
        // Actor's balance: negative - the actor must add money to balance the tables and vice versa
        int balance = fromPlayer ? totalCostPlayer - totalCostNpc : totalCostNpc - totalCostPlayer;

        if ((balance < 0 && fromInventory) || (balance > 0 && !fromInventory)) {
            suggestedValue = std::min(std::abs(balance), maxQuantity);
            if (immediate) {
                return suggestedValue;
            }
        }
    }

    if (immediate) {
        return maxQuantity;
    }

    return inventoryQuantitySelect(INVENTORY_WINDOW_TYPE_MOVE_ITEMS, item, maxQuantity, suggestedValue);
}

// clicked on an inventory in preparation of dragging or immediate transfer
static void _drag_item_loop(Object* item, bool immediate)
{
    if (immediate) {
        soundPlayFile("iputdown");
        // prevent item look from occuring after immediate move
        _im_value = -1;
        return;
    }

    FrmImage itemInventoryFrmImage;
    int itemInventoryFid = itemGetInventoryFid(item);
    if (itemInventoryFrmImage.lock(itemInventoryFid)) {
        int width = itemInventoryFrmImage.getWidth();
        int height = itemInventoryFrmImage.getHeight();
        unsigned char* data = itemInventoryFrmImage.getData();
        mouseSetFrame(data, width, height, width, width / 2, height / 2, 0);
        soundPlayFile("ipickup1");
    }

    do {
        sharedFpsLimiter.mark();

        inputGetInput();

        renderPresent();
        sharedFpsLimiter.throttle();
    } while ((mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) != 0);

    if (itemInventoryFrmImage.isLocked()) {
        itemInventoryFrmImage.unlock();
        soundPlayFile("iputdown");
    }
}

// 0x474DAC
static void _barter_move_inventory(Object* item, int quantity, int slotIndex, int indexOffset, Object* npc, Object* sourceTable, bool fromDude)
{
    Rect rect;
    Object* owner = nullptr;

    // Helper to get the actual item from the left (player) panel
    auto getLeftItem = [&](int slotIndex, int stackOffset, Object*& outItem, int& outQuantity, Object*& outOwner) -> bool {
        if (gFilterCategory != -1) {
            int filteredCount;
            if (gUseCombinedInventory) {
                filteredCount = buildFilteredCombinedIndices(); // fills gFilteredIndices
            } else {
                filteredCount = buildFilteredIndices(_pud);
            }
            int filteredIndex = stackOffset + slotIndex;
            if (filteredIndex >= filteredCount) return false;
            int actualIndex = gFilteredIndices[filteredIndex];
            if (gUseCombinedInventory) {
                if (actualIndex < 0 || actualIndex >= gCombinedItemCount) return false;
                CombinedItem* ci = &gCombinedItems[actualIndex];
                outItem = ci->item;
                outQuantity = ci->quantity;
                outOwner = ci->owner;
            } else {
                if (actualIndex < 0 || actualIndex >= _pud->length) return false;
                InventoryItem* invItem = &(_pud->items[actualIndex]);
                outItem = invItem->item;
                outQuantity = invItem->quantity;
                outOwner = _inven_dude;
            }
        } else {
            // No filter: use reversed order.
            if (gUseCombinedInventory) {
                int itemIndex = stackOffset + slotIndex;
                if (itemIndex >= gCombinedItemCount) return false;
                int actualIndex = gCombinedItemCount - (itemIndex + 1);
                CombinedItem* ci = &gCombinedItems[actualIndex];
                outItem = ci->item;
                outQuantity = ci->quantity;
                outOwner = ci->owner;
            } else {
                int actualIndex = _pud->length - (slotIndex + stackOffset + 1);
                if (actualIndex < 0 || actualIndex >= _pud->length) return false;
                InventoryItem* invItem = &(_pud->items[actualIndex]);
                outItem = invItem->item;
                outQuantity = invItem->quantity;
                outOwner = _inven_dude;
            }
        }
        return (outItem != nullptr);
    };

    // Helper to get the actual item from the right (NPC) panel
    auto getRightItem = [&](int slotIndex, int stackOffset, Object*& outItem, int& outQuantity, Object*& outOwner) -> bool {
        if (gFilterCategory != -1) {
            int filteredCount = buildFilteredIndices(_target_pud);
            int filteredIndex = stackOffset + slotIndex;
            if (filteredIndex >= filteredCount) return false;
            int actualIndex = gFilteredIndices[filteredIndex];
            if (actualIndex < 0 || actualIndex >= _target_pud->length) return false;
            InventoryItem* invItem = &(_target_pud->items[actualIndex]);
            outItem = invItem->item;
            outQuantity = invItem->quantity;
            outOwner = npc; // the source is the NPC
        } else {
            int actualIndex = _target_pud->length - (slotIndex + stackOffset + 1);
            if (actualIndex < 0 || actualIndex >= _target_pud->length) return false;
            InventoryItem* invItem = &(_target_pud->items[actualIndex]);
            outItem = invItem->item;
            outQuantity = invItem->quantity;
            outOwner = npc;
        }
        return (outItem != nullptr);
    };

    // Retrieve the item based on direction
    if (fromDude) {
        // From left (player/combined) to the offer table
        rect.left = 31;
        rect.top = gInventorySlotHeight * slotIndex + 31;

        if (!getLeftItem(slotIndex, _stack_offset[_curr_stack], item, quantity, owner)) {
            return;
        }

        // Redraw left side if quantity > 1
        if (quantity > 1) {
            _display_inventory(indexOffset, slotIndex, INVENTORY_WINDOW_TYPE_TRADE);
        } else {
            // Erase single slot background
            unsigned char* dest = windowGetBuffer(gInventoryWindow);
            unsigned char* src = windowGetBuffer(_barter_back_win);
            int pitch = INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH;
            blitBufferToBuffer(src + pitch * rect.top + rect.left + INVENTORY_TRADE_WINDOW_OFFSET,
                INVENTORY_SLOT_WIDTH, gInventorySlotHeight, pitch,
                dest + INVENTORY_TRADE_WINDOW_WIDTH * rect.top + rect.left,
                INVENTORY_TRADE_WINDOW_WIDTH);
            rect.right = rect.left + INVENTORY_SLOT_WIDTH - 1;
            rect.bottom = rect.top + gInventorySlotHeight - 1;
            windowRefreshRect(gInventoryWindow, &rect);
        }
    } else {
        // From right (NPC inventory) to the offer table
        rect.left = 389;
        rect.top = gInventorySlotHeight * slotIndex + 31;

        if (!getRightItem(slotIndex, _target_stack_offset[_target_curr_stack], item, quantity, owner)) {
            return;
        }

        if (quantity > 1) {
            _display_target_inventory(indexOffset, slotIndex, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
        } else {
            unsigned char* dest = windowGetBuffer(gInventoryWindow);
            unsigned char* src = windowGetBuffer(_barter_back_win);
            int pitch = INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH;
            blitBufferToBuffer(src + pitch * rect.top + rect.left + INVENTORY_TRADE_WINDOW_OFFSET,
                INVENTORY_SLOT_WIDTH, gInventorySlotHeight, pitch,
                dest + INVENTORY_TRADE_WINDOW_WIDTH * rect.top + rect.left,
                INVENTORY_TRADE_WINDOW_WIDTH);
            rect.right = rect.left + INVENTORY_SLOT_WIDTH - 1;
            rect.bottom = rect.top + gInventorySlotHeight - 1;
            windowRefreshRect(gInventoryWindow, &rect);
        }
    }

    bool immediate = false;
    _drag_item_loop(item, immediate);

    MessageListItem messageListItem;

    if (fromDude) {
        // Drop on the left offer table
        if (immediate || mouseHitTestInWindow(gInventoryWindow, INVENTORY_TRADE_INNER_LEFT_SCROLLER_TRACKING_X, INVENTORY_TRADE_INNER_LEFT_SCROLLER_TRACKING_Y, INVENTORY_TRADE_INNER_LEFT_SCROLLER_TRACKING_MAX_X, gInventorySlotHeight * gInventorySlotsCount + INVENTORY_TRADE_INNER_LEFT_SCROLLER_TRACKING_Y)) {
            int quantityToMove = _barter_get_quantity_moved_items(item, quantity, true, true, immediate);
            if (quantityToMove != -1) {
                if (itemMoveForce(owner, sourceTable, item, quantityToMove) == -1) {
                    messageListItem.num = 26; // There is no space left for that item.
                    if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                        displayMonitorAddMessage(messageListItem.text);
                    }
                }
            }
        }
    } else {
        // Drop on the right offer table
        if (immediate || mouseHitTestInWindow(gInventoryWindow, INVENTORY_TRADE_INNER_RIGHT_SCROLLER_TRACKING_X, INVENTORY_TRADE_INNER_RIGHT_SCROLLER_TRACKING_Y, INVENTORY_TRADE_INNER_RIGHT_SCROLLER_TRACKING_MAX_X, gInventorySlotHeight * gInventorySlotsCount + INVENTORY_TRADE_INNER_RIGHT_SCROLLER_TRACKING_Y)) {
            int quantityToMove = _barter_get_quantity_moved_items(item, quantity, false, true, immediate);
            if (quantityToMove != -1) {
                if (itemMoveForce(owner, sourceTable, item, quantityToMove) == -1) {
                    messageListItem.num = 25; // You cannot pick that up. You are at your maximum weight capacity.
                    if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                        displayMonitorAddMessage(messageListItem.text);
                    }
                }
            }
        }
    }

    // Rebuild combined list if active
    if (gUseCombinedInventory) {
        inventoryBuildCombinedList(gDude);
    }

    // Refresh body displays (weight info)
    inventoryRefreshBodies(INVENTORY_WINDOW_TYPE_TRADE);
    inventorySetCursor(INVENTORY_WINDOW_CURSOR_HAND);
}

// 0x475070
static void _barter_move_from_table_inventory(Object* item, int quantity, int slotIndex, Object* npc, Object* sourceTable, bool fromDude)
{
    Rect rect;
    if (fromDude) {
        rect.left = INVENTORY_TRADE_INNER_LEFT_SCROLLER_X_PAD;
        rect.top = gInventorySlotHeight * slotIndex + INVENTORY_TRADE_INNER_LEFT_SCROLLER_Y_PAD;
    } else {
        rect.left = INVENTORY_TRADE_INNER_RIGHT_SCROLLER_X_PAD;
        rect.top = gInventorySlotHeight * slotIndex + INVENTORY_TRADE_INNER_RIGHT_SCROLLER_Y_PAD;
    }

    if (quantity > 1) {
        if (fromDude) {
            inventoryWindowRenderInnerInventories(_barter_back_win, sourceTable, nullptr, slotIndex);
        } else {
            inventoryWindowRenderInnerInventories(_barter_back_win, nullptr, sourceTable, slotIndex);
        }
    } else {
        unsigned char* dest = windowGetBuffer(gInventoryWindow);
        unsigned char* src = windowGetBuffer(_barter_back_win);

        int pitch = INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH;
        blitBufferToBuffer(src + pitch * rect.top + rect.left + INVENTORY_TRADE_WINDOW_OFFSET, INVENTORY_SLOT_WIDTH, gInventorySlotHeight, pitch, dest + INVENTORY_TRADE_WINDOW_WIDTH * rect.top + rect.left, INVENTORY_TRADE_WINDOW_WIDTH);

        rect.right = rect.left + INVENTORY_SLOT_WIDTH - 1;
        rect.bottom = rect.top + gInventorySlotHeight - 1;
        windowRefreshRect(gInventoryWindow, &rect);
    }

    bool immediate = false;
    _drag_item_loop(item, immediate);

    MessageListItem messageListItem;

    if (fromDude) {
        if (immediate || mouseHitTestInWindow(gInventoryWindow, INVENTORY_TRADE_LEFT_SCROLLER_TRACKING_X, INVENTORY_TRADE_LEFT_SCROLLER_TRACKING_Y, INVENTORY_TRADE_LEFT_SCROLLER_TRACKING_MAX_X, gInventorySlotHeight * gInventorySlotsCount + INVENTORY_TRADE_LEFT_SCROLLER_TRACKING_Y)) {
            int quantityToMove = _barter_get_quantity_moved_items(item, quantity, true, false, immediate);
            if (quantityToMove != -1) {
                if (itemMoveForce(sourceTable, _inven_dude, item, quantityToMove) == -1) {
                    messageListItem.num = 26; // There is no space left for that item.
                    if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                        displayMonitorAddMessage(messageListItem.text);
                    }
                }
                if (gUseCombinedInventory) {
                    inventoryBuildCombinedList(gDude);
                }
            }
        }
    } else {
        if (immediate || mouseHitTestInWindow(gInventoryWindow, INVENTORY_TRADE_RIGHT_SCROLLER_TRACKING_X, INVENTORY_TRADE_RIGHT_SCROLLER_TRACKING_Y, INVENTORY_TRADE_RIGHT_SCROLLER_TRACKING_MAX_X, gInventorySlotHeight * gInventorySlotsCount + INVENTORY_TRADE_RIGHT_SCROLLER_TRACKING_Y)) {
            int quantityToMove = _barter_get_quantity_moved_items(item, quantity, false, false, immediate);
            if (quantityToMove != -1) {
                if (itemMoveForce(sourceTable, npc, item, quantityToMove) == -1) {
                    messageListItem.num = 25; // You cannot pick that up. You are at your maximum weight capacity.
                    if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                        displayMonitorAddMessage(messageListItem.text);
                    }
                }
                if (gUseCombinedInventory) {
                    inventoryBuildCombinedList(gDude);
                }
            }
        }
    }

    inventoryRefreshBodies(INVENTORY_WINDOW_TYPE_TRADE);
    inventorySetCursor(INVENTORY_WINDOW_CURSOR_HAND);
}

// 0x475334
static void inventoryWindowRenderInnerInventories(int win, Object* leftTable, Object* rightTable, int draggedSlotIndex)
{
    unsigned char* windowBuffer = windowGetBuffer(gInventoryWindow);

    int oldFont = fontGetCurrent();
    fontSetCurrent(101);

    char formattedText[80];
    int rectHeight = fontGetLineHeight() + INVENTORY_SLOT_HEIGHT * gInventorySlotsCount;

    // Clamp inner offsets to prevent out-of-bounds after item removal
    if (_ptable_pud != NULL && _ptable_offset >= _ptable_pud->length) {
        _ptable_offset = (_ptable_pud->length > 0) ? _ptable_pud->length - 1 : 0;
    }
    if (_btable_pud != NULL && _btable_offset >= _btable_pud->length) {
        _btable_offset = (_btable_pud->length > 0) ? _btable_pud->length - 1 : 0;
    }

    if (leftTable != nullptr) {
        unsigned char* src = windowGetBuffer(win);
        blitBufferToBuffer(src + INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH * INVENTORY_TRADE_INNER_LEFT_SCROLLER_Y + INVENTORY_TRADE_INNER_LEFT_SCROLLER_X_PAD + INVENTORY_TRADE_WINDOW_OFFSET, INVENTORY_SLOT_WIDTH, rectHeight + 1, INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH, windowBuffer + INVENTORY_TRADE_WINDOW_WIDTH * INVENTORY_TRADE_INNER_LEFT_SCROLLER_Y + INVENTORY_TRADE_INNER_LEFT_SCROLLER_X_PAD, INVENTORY_TRADE_WINDOW_WIDTH);

        unsigned char* dest = windowBuffer + INVENTORY_TRADE_WINDOW_WIDTH * INVENTORY_TRADE_INNER_LEFT_SCROLLER_Y_PAD + INVENTORY_TRADE_INNER_LEFT_SCROLLER_X_PAD;
        Inventory* inventory = &(leftTable->data.inventory);
        for (int index = 0; index < gInventorySlotsCount && index + _ptable_offset < inventory->length; index++) {
            InventoryItem* inventoryItem = &(inventory->items[inventory->length - (index + _ptable_offset + 1)]);
            int inventoryFid = itemGetInventoryFid(inventoryItem->item);
            artRender(inventoryFid, dest, gInventorySlotWidthPadded, gInventorySlotHeightPadded, INVENTORY_TRADE_WINDOW_WIDTH);
            _display_inventory_info(inventoryItem->item, inventoryItem->quantity, dest, INVENTORY_TRADE_WINDOW_WIDTH, index == draggedSlotIndex, false);

            dest += INVENTORY_TRADE_WINDOW_WIDTH * INVENTORY_SLOT_HEIGHT;
        }

        if (gGameDialogSpeakerIsPartyMember) {
            MessageListItem messageListItem;
            messageListItem.num = 30; // Wt.

            if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                int weight = objectGetInventoryWeight(leftTable);
                snprintf(formattedText, sizeof(formattedText), "%s %d", messageListItem.text, weight);
            }
        } else {
            int cost = objectGetCost(leftTable);
            snprintf(formattedText, sizeof(formattedText), "$%d", cost);
        }

        fontDrawText(windowBuffer + INVENTORY_TRADE_WINDOW_WIDTH * (INVENTORY_SLOT_HEIGHT * gInventorySlotsCount + INVENTORY_TRADE_INNER_LEFT_SCROLLER_Y_PAD) + INVENTORY_TRADE_INNER_LEFT_SCROLLER_X_PAD, formattedText, 80, INVENTORY_TRADE_WINDOW_WIDTH, _colorTable[COL_WHITE]);

        Rect rect;
        rect.left = INVENTORY_TRADE_INNER_LEFT_SCROLLER_X_PAD;
        rect.top = INVENTORY_TRADE_INNER_LEFT_SCROLLER_Y_PAD;
        // NOTE: Odd math, the only way to get 223 is to subtract 2.
        rect.right = INVENTORY_TRADE_INNER_LEFT_SCROLLER_X_PAD + gInventorySlotWidthPadded - 2;
        rect.bottom = rect.top + rectHeight;
        windowRefreshRect(gInventoryWindow, &rect);
    }

    if (rightTable != nullptr) {
        unsigned char* src = windowGetBuffer(win);
        blitBufferToBuffer(src + INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH * INVENTORY_TRADE_INNER_RIGHT_SCROLLER_Y + INVENTORY_TRADE_INNER_RIGHT_SCROLLER_X_PAD + INVENTORY_TRADE_WINDOW_OFFSET, INVENTORY_SLOT_WIDTH, rectHeight + 1, INVENTORY_TRADE_BACKGROUND_WINDOW_WIDTH, windowBuffer + INVENTORY_TRADE_WINDOW_WIDTH * INVENTORY_TRADE_INNER_RIGHT_SCROLLER_Y + INVENTORY_TRADE_INNER_RIGHT_SCROLLER_X_PAD, INVENTORY_TRADE_WINDOW_WIDTH);

        unsigned char* dest = windowBuffer + INVENTORY_TRADE_WINDOW_WIDTH * INVENTORY_TRADE_INNER_RIGHT_SCROLLER_Y_PAD + INVENTORY_TRADE_INNER_RIGHT_SCROLLER_X_PAD;
        Inventory* inventory = &(rightTable->data.inventory);
        for (int index = 0; index < gInventorySlotsCount && index + _btable_offset < inventory->length; index++) {
            InventoryItem* inventoryItem = &(inventory->items[inventory->length - (index + _btable_offset + 1)]);
            int inventoryFid = itemGetInventoryFid(inventoryItem->item);
            artRender(inventoryFid, dest, gInventorySlotWidthPadded, gInventorySlotHeightPadded, INVENTORY_TRADE_WINDOW_WIDTH);
            _display_inventory_info(inventoryItem->item, inventoryItem->quantity, dest, INVENTORY_TRADE_WINDOW_WIDTH, index == draggedSlotIndex, false);

            dest += INVENTORY_TRADE_WINDOW_WIDTH * INVENTORY_SLOT_HEIGHT;
        }

        if (gGameDialogSpeakerIsPartyMember) {
            MessageListItem messageListItem;
            messageListItem.num = 30; // Wt.

            if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                int weight = _barter_compute_value(gDude, _target_stack[0]);
                snprintf(formattedText, sizeof(formattedText), "%s %d", messageListItem.text, weight);
            }
        } else {
            int cost = _barter_compute_value(gDude, _target_stack[0]);
            snprintf(formattedText, sizeof(formattedText), "$%d", cost);
        }

        fontDrawText(windowBuffer + INVENTORY_TRADE_WINDOW_WIDTH * (INVENTORY_SLOT_HEIGHT * gInventorySlotsCount + INVENTORY_TRADE_INNER_RIGHT_SCROLLER_Y_PAD) + INVENTORY_TRADE_INNER_RIGHT_SCROLLER_X_PAD, formattedText, 80, INVENTORY_TRADE_WINDOW_WIDTH, _colorTable[COL_WHITE]);

        Rect rect;
        rect.left = INVENTORY_TRADE_INNER_RIGHT_SCROLLER_X_PAD;
        rect.top = INVENTORY_TRADE_INNER_RIGHT_SCROLLER_Y_PAD;
        // NOTE: Odd math, likely should be `gInventorySlotWidthPadded`.
        rect.right = INVENTORY_TRADE_INNER_RIGHT_SCROLLER_X_PAD + INVENTORY_SLOT_WIDTH;
        rect.bottom = rect.top + rectHeight;
        windowRefreshRect(gInventoryWindow, &rect);
    }

    tradeWindowUpdateScrollButtons();

    fontSetCurrent(oldFont);
}

// 0x4757F0
void inventoryOpenTrade(int win, Object* barterer, Object* playerTable, Object* bartererTable, int barterMod)
{
    ScopedGameMode gm(GameMode::kBarter);

    _barter_mod = barterMod;

    if (inventoryCommonInit() == -1) {
        return;
    }

    gIsTradeWindow = true;

    Object* armor = critterGetArmor(barterer);
    if (armor != nullptr) {
        itemRemove(barterer, armor, 1);
    }

    Object* leftHand = critterGetItem1(barterer);
    Object* rightHand = critterGetItem2(barterer);

    // Remove both hands items (if they exist)
    if (leftHand != nullptr) itemRemove(barterer, leftHand, 1);
    if (rightHand != nullptr) itemRemove(barterer, rightHand, 1);

    // If no weapons were found and the speaker is not a party member, search inventory for one weapon (original behavior)
    if (leftHand == nullptr && rightHand == nullptr && !gGameDialogSpeakerIsPartyMember) {
        Object* found = inventoryFindByType(barterer, ITEM_TYPE_WEAPON, nullptr);
        if (found != nullptr) {
            itemRemove(barterer, found, 1);
            // We'll store it as rightHand to restore it later
            rightHand = found;
        }
    }

    Object* hiddenBox = nullptr;
    if (objectCreateWithFidPid(&hiddenBox, -1, PROTO_ID_JESSE_CONTAINER) == -1) {
        return;
    }

    // Sfall: remove hidden items of barterer (relevant to Goris)
    itemMoveAllHidden(barterer, hiddenBox);

    _pud = &(_inven_dude->data.inventory);
    _btable = bartererTable;
    _ptable = playerTable;

    _ptable_offset = 0;
    _btable_offset = 0;

    _ptable_pud = &(playerTable->data.inventory);
    _btable_pud = &(bartererTable->data.inventory);

    _barter_back_win = win;
    _target_curr_stack = 0;
    _target_pud = &(barterer->data.inventory);

    _target_stack[0] = barterer;
    _target_stack_offset[0] = 0;

    // Ensure merchant's money is at the top
    _move_money_to_top(_target_pud, _target_pud->length);

    bool isoWasEnabled = _setup_inventory(INVENTORY_WINDOW_TYPE_TRADE);
    _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
    _display_inventory(_stack_offset[0], -1, INVENTORY_WINDOW_TYPE_TRADE);
    _display_body(barterer->fid, INVENTORY_WINDOW_TYPE_TRADE);
    windowRefresh(_barter_back_win);
    inventoryWindowRenderInnerInventories(win, playerTable, bartererTable, -1);

    inventorySetCursor(INVENTORY_WINDOW_CURSOR_HAND);

    int modifier;
    int npcReactionValue = reactionGetValue(barterer);
    int npcReactionType = reactionTranslateValue(npcReactionValue);
    switch (npcReactionType) {
    case NPC_REACTION_BAD:
        modifier = 25;
        break;
    case NPC_REACTION_NEUTRAL:
        modifier = 0;
        break;
    case NPC_REACTION_GOOD:
        modifier = -15;
        break;
    default:
        assert(false && "Should be unreachable");
    }

    int keyCode = -1;
    for (;;) {
        sharedFpsLimiter.mark();

        if (keyCode == KEY_ESCAPE || _game_user_wants_to_quit != 0) {
            break;
        }

        keyCode = inputGetInput();
        if (keyCode == KEY_CTRL_Q || keyCode == KEY_CTRL_X || keyCode == KEY_F10) {
            showQuitConfirmationDialog();
        }

        if (_game_user_wants_to_quit != 0) {
            break;
        }

        _barter_mod = barterMod + modifier;

        // Filter keyCode handling
        if (keyCode >= KEYCODE_FILTER_BASE && keyCode <= 8004) {
            if (!settings.enhancements.strict_vanilla && settings.enhancements.inventory_filter) {
                int category = keyCode - KEYCODE_FILTER_BASE;
                if (gFilterCategory == category)
                    gFilterCategory = -1;
                else
                    gFilterCategory = category;

                // Reset scroll offsets
                _stack_offset[_curr_stack] = 0;
                _target_stack_offset[_target_curr_stack] = 0;

                // Refresh both outer inventories
                soundPlayFile("ib1p1xx1");
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_TRADE);
                _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);

                // The inner offer tables are not filtered, so they don't need redrawing.
                windowRefresh(gInventoryWindow);
                continue;
            }
        }

        if (keyCode == KEY_LOWERCASE_T || modifier <= -30) {
            // T == return to talk
            itemMoveAll(bartererTable, barterer);
            itemMoveAll(playerTable, gDude);
            _barter_end_to_talk_to();
            break;
        } else if (keyCode == KEY_LOWERCASE_O) {
            // O == attempt offer
            if (playerTable->data.inventory.length != 0 || _btable->data.inventory.length != 0) {
                if (_barter_attempt_transaction(_inven_dude, playerTable, barterer, bartererTable) == 0) {
                    if (gUseCombinedInventory) {
                        inventoryBuildCombinedList(gDude);
                    }
                    _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
                    _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_TRADE);
                    inventoryWindowRenderInnerInventories(win, playerTable, bartererTable, -1);

                    MessageListItem messageListItem;
                    messageListItem.num = 27; // OK, that's a good trade.
                    if (!gGameDialogSpeakerIsPartyMember) {
                        if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                            gameDialogRenderSupplementaryMessage(messageListItem.text);
                        }
                    }
                }
            }
        } else if (keyCode == KEY_ARROW_UP) {
            if (_stack_offset[_curr_stack] > 0) {
                _stack_offset[_curr_stack] -= 1;
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_TRADE);
            }
        } else if (keyCode == KEY_PAGE_UP) {
            if (_ptable_offset > 0) {
                _ptable_offset -= 1;
                inventoryWindowRenderInnerInventories(win, playerTable, bartererTable, -1);
            }
        } else if (keyCode == KEY_ARROW_DOWN) {
            int totalFiltered = getFilteredCount();
            if (_stack_offset[_curr_stack] + gInventorySlotsCount < totalFiltered) {
                _stack_offset[_curr_stack] += 1;
                _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_TRADE);
            }
        } else if (keyCode == KEY_PAGE_DOWN) {
            if (_ptable_offset + gInventorySlotsCount < _ptable_pud->length) {
                _ptable_offset += 1;
                inventoryWindowRenderInnerInventories(win, playerTable, bartererTable, -1);
            }
        } else if (keyCode == KEY_CTRL_PAGE_DOWN) {
            if (_btable_offset + gInventorySlotsCount < _btable_pud->length) {
                _btable_offset++;
                inventoryWindowRenderInnerInventories(win, playerTable, bartererTable, -1);
            }
        } else if (keyCode == KEY_CTRL_PAGE_UP) {
            if (_btable_offset > 0) {
                _btable_offset -= 1;
                inventoryWindowRenderInnerInventories(win, playerTable, bartererTable, -1);
            }
        } else if (keyCode == KEY_CTRL_ARROW_UP) {
            if (_target_stack_offset[_target_curr_stack] > 0) {
                _target_stack_offset[_target_curr_stack] -= 1;
                _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
                windowRefresh(gInventoryWindow);
            }
        } else if (keyCode == KEY_CTRL_ARROW_DOWN) {
            int filteredCount = buildFilteredIndices(_target_pud);
            if (_target_stack_offset[_target_curr_stack] + gInventorySlotsCount < filteredCount) {
                _target_stack_offset[_target_curr_stack] += 1;
                _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
                windowRefresh(gInventoryWindow);
            }
        } else if (keyCode >= INVENTORY_BUTTON_LEFT && keyCode <= INVENTORY_BUTTON_RIGHT) {
            if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
                // Arrow mode - sort inventory
                inventoryWindowOpenSortContextMenu(keyCode, INVENTORY_WINDOW_TYPE_TRADE);
            } else {
                _container_exit(keyCode, INVENTORY_WINDOW_TYPE_TRADE);
            }
        } else {
            if ((mouseGetEvent() & MOUSE_EVENT_RIGHT_BUTTON_DOWN) != 0) {
                if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_HAND) {
                    inventorySetCursor(INVENTORY_WINDOW_CURSOR_ARROW);
                } else {
                    inventorySetCursor(INVENTORY_WINDOW_CURSOR_HAND);
                }
            } else if ((mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_DOWN) != 0) {
                if (keyCode >= KEYCODE_GRID_BASE && keyCode <= KEYCODE_GRID_BASE + gInventorySlotsCount) {
                    if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
                        inventoryWindowOpenContextMenu(keyCode, INVENTORY_WINDOW_TYPE_TRADE);
                    } else {
                        int slotIndex = keyCode - KEYCODE_GRID_BASE;
                        Object* item = nullptr;
                        Object* owner = nullptr;
                        int quantity = _inven_from_button(keyCode, &item, nullptr, &owner);
                        if (item != nullptr) {
                            _barter_move_inventory(item, quantity, slotIndex, _stack_offset[_curr_stack], barterer, playerTable, true);
                            _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_TRADE);
                            inventoryWindowRenderInnerInventories(win, playerTable, nullptr, -1);
                        }
                    }
                    keyCode = -1;
                } else if (keyCode >= KEYCODE_TARGET_GRID_BASE && keyCode <= KEYCODE_TARGET_GRID_BASE + gInventorySlotsCount) {
                    // merchant inventory
                    if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
                        inventoryWindowOpenContextMenu(keyCode, INVENTORY_WINDOW_TYPE_TRADE);
                        inventoryWindowRenderInnerInventories(win, nullptr, bartererTable, -1);
                    } else {
                        int slotIndex = keyCode - KEYCODE_TARGET_GRID_BASE;
                        if (slotIndex + _target_stack_offset[_target_curr_stack] < _target_pud->length) {
                            int stackOffset = _target_stack_offset[_target_curr_stack];
                            InventoryItem* inventoryItem = &(_target_pud->items[_target_pud->length - (slotIndex + stackOffset + 1)]);
                            _barter_move_inventory(inventoryItem->item, inventoryItem->quantity, slotIndex, stackOffset, barterer, bartererTable, false);
                            _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_TRADE);
                            inventoryWindowRenderInnerInventories(win, nullptr, bartererTable, -1);
                        }
                    }

                    keyCode = -1;
                } else if (keyCode >= KEYCODE_OFFER_LEFT_BASE && keyCode <= KEYCODE_OFFER_LEFT_BASE + gInventorySlotsCount) {
                    // player table (offer)
                    if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
                        inventoryWindowOpenContextMenu(keyCode, INVENTORY_WINDOW_TYPE_TRADE);
                        inventoryWindowRenderInnerInventories(win, playerTable, nullptr, -1);
                    } else {
                        int slotIndex = keyCode - KEYCODE_OFFER_LEFT_BASE;
                        if (slotIndex + _ptable_offset < _ptable_pud->length) {
                            InventoryItem* inventoryItem = &(_ptable_pud->items[_ptable_pud->length - (slotIndex + _ptable_offset + 1)]);
                            _barter_move_from_table_inventory(inventoryItem->item, inventoryItem->quantity, slotIndex, barterer, playerTable, true);
                            _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_TRADE);
                            inventoryWindowRenderInnerInventories(win, playerTable, nullptr, -1);
                        }
                    }

                    keyCode = -1;
                } else if (keyCode >= KEYCODE_OFFER_RIGHT_BASE && keyCode <= KEYCODE_OFFER_RIGHT_BASE + gInventorySlotsCount) {
                    // merchant table (offer)
                    if (gInventoryCursor == INVENTORY_WINDOW_CURSOR_ARROW) {
                        inventoryWindowOpenContextMenu(keyCode, INVENTORY_WINDOW_TYPE_TRADE);
                        inventoryWindowRenderInnerInventories(win, nullptr, bartererTable, -1);
                    } else {
                        int slotIndex = keyCode - KEYCODE_OFFER_RIGHT_BASE;
                        if (slotIndex + _btable_offset < _btable_pud->length) {
                            InventoryItem* inventoryItem = &(_btable_pud->items[_btable_pud->length - (slotIndex + _btable_offset + 1)]);
                            _barter_move_from_table_inventory(inventoryItem->item, inventoryItem->quantity, slotIndex, barterer, bartererTable, false);
                            _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_TRADE);
                            inventoryWindowRenderInnerInventories(win, nullptr, bartererTable, -1);
                        }
                    }

                    keyCode = -1;
                }
            } else if ((mouseGetEvent() & MOUSE_EVENT_WHEEL) != 0) {
                int wheelX, wheelY;
                mouseGetWheel(&wheelX, &wheelY);

                // Left outer (player inventory)
                if (mouseHitTestInWindow(gInventoryWindow,
                        INVENTORY_TRADE_LEFT_SCROLLER_TRACKING_X,
                        INVENTORY_TRADE_LEFT_SCROLLER_TRACKING_Y,
                        INVENTORY_TRADE_LEFT_SCROLLER_TRACKING_MAX_X,
                        gInventorySlotHeight * gInventorySlotsCount + INVENTORY_TRADE_LEFT_SCROLLER_TRACKING_Y)) {
                    if (wheelY > 0) {
                        if (_stack_offset[_curr_stack] > 0) {
                            _stack_offset[_curr_stack] -= 1;
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_TRADE);
                        }
                    } else if (wheelY < 0) {
                        int totalFiltered = getFilteredCount();
                        if (_stack_offset[_curr_stack] + gInventorySlotsCount < totalFiltered) {
                            _stack_offset[_curr_stack] += 1;
                            _display_inventory(_stack_offset[_curr_stack], -1, INVENTORY_WINDOW_TYPE_TRADE);
                        }
                    }
                }
                // Left inner (player offer table)
                else if (mouseHitTestInWindow(gInventoryWindow,
                             INVENTORY_TRADE_INNER_LEFT_SCROLLER_TRACKING_X,
                             INVENTORY_TRADE_INNER_LEFT_SCROLLER_TRACKING_Y,
                             INVENTORY_TRADE_INNER_LEFT_SCROLLER_TRACKING_MAX_X,
                             gInventorySlotHeight * gInventorySlotsCount + INVENTORY_TRADE_INNER_LEFT_SCROLLER_TRACKING_Y)) {
                    if (wheelY > 0) {
                        if (_ptable_offset > 0) {
                            _ptable_offset -= 1;
                            inventoryWindowRenderInnerInventories(_barter_back_win, _ptable, _btable, -1);
                        }
                    } else if (wheelY < 0) {
                        if (_ptable_offset + gInventorySlotsCount < _ptable_pud->length) {
                            _ptable_offset += 1;
                            inventoryWindowRenderInnerInventories(_barter_back_win, _ptable, _btable, -1);
                        }
                    }
                }
                // Right outer (merchant/NPC inventory)
                else if (mouseHitTestInWindow(gInventoryWindow,
                             INVENTORY_TRADE_RIGHT_SCROLLER_TRACKING_X,
                             INVENTORY_TRADE_RIGHT_SCROLLER_TRACKING_Y,
                             INVENTORY_TRADE_RIGHT_SCROLLER_TRACKING_MAX_X,
                             gInventorySlotHeight * gInventorySlotsCount + INVENTORY_TRADE_RIGHT_SCROLLER_TRACKING_Y)) {
                    if (wheelY > 0) {
                        if (_target_stack_offset[_target_curr_stack] > 0) {
                            _target_stack_offset[_target_curr_stack] -= 1;
                            _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
                            windowRefresh(gInventoryWindow);
                        }
                    } else if (wheelY < 0) {
                        int totalFiltered = buildFilteredIndices(_target_pud);
                        if (_target_stack_offset[_target_curr_stack] + gInventorySlotsCount < totalFiltered) {
                            _target_stack_offset[_target_curr_stack] += 1;
                            _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
                            windowRefresh(gInventoryWindow);
                        }
                    }
                }
                // Right inner (merchant offer table)
                else if (mouseHitTestInWindow(gInventoryWindow,
                             INVENTORY_TRADE_INNER_RIGHT_SCROLLER_TRACKING_X,
                             INVENTORY_TRADE_INNER_RIGHT_SCROLLER_TRACKING_Y,
                             INVENTORY_TRADE_INNER_RIGHT_SCROLLER_TRACKING_MAX_X,
                             gInventorySlotHeight * gInventorySlotsCount + INVENTORY_TRADE_INNER_RIGHT_SCROLLER_TRACKING_Y)) {
                    if (wheelY > 0) {
                        if (_btable_offset > 0) {
                            _btable_offset -= 1;
                            inventoryWindowRenderInnerInventories(_barter_back_win, _ptable, _btable, -1);
                        }
                    } else if (wheelY < 0) {
                        if (_btable_offset + gInventorySlotsCount < _btable_pud->length) {
                            _btable_offset += 1;
                            inventoryWindowRenderInnerInventories(_barter_back_win, _ptable, _btable, -1);
                        }
                    }
                }
            }
        }
        if (!settings.enhancements.strict_vanilla && settings.enhancements.inventory_filter) {
            int filterCategory = inventoryKeyToFilterCategory(keyCode);
            if (filterCategory != -1) {
                if (gFilterCategory == filterCategory) {
                    gFilterCategory = -1;
                } else {
                    gFilterCategory = filterCategory;
                }
                _stack_offset[_curr_stack] = 0;
                _target_stack_offset[_target_curr_stack] = 0;
                soundPlayFile("ib1p1xx1");
                _display_inventory(0, -1, INVENTORY_WINDOW_TYPE_TRADE);
                _display_target_inventory(0, -1, _target_pud, INVENTORY_WINDOW_TYPE_TRADE);
                inventoryWindowRenderInnerInventories(_barter_back_win, _ptable, _btable, -1);
                windowRefresh(gInventoryWindow);
            }
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    itemMoveAll(hiddenBox, barterer);
    objectDestroy(hiddenBox, nullptr);

    if (armor != nullptr) {
        armor->flags |= OBJECT_WORN;
        itemAdd(barterer, armor, 1);
    }

    if (leftHand != nullptr) {
        leftHand->flags |= OBJECT_IN_LEFT_HAND;
        itemAdd(barterer, leftHand, 1);
    }
    if (rightHand != nullptr) {
        rightHand->flags |= OBJECT_IN_RIGHT_HAND;
        itemAdd(barterer, rightHand, 1);
    }

    _exit_inventory(isoWasEnabled);

    // NOTE: Uninline.
    inventoryCommonFree();
}

// 0x47620C
static void _container_enter(int keyCode, int inventoryWindowType)
{
    if (keyCode >= KEYCODE_TARGET_GRID_BASE) {
        int index = _target_pud->length - (_target_stack_offset[_target_curr_stack] + keyCode - KEYCODE_TARGET_GRID_BASE + 1);
        if (index < _target_pud->length && _target_curr_stack < 9) {
            InventoryItem* inventoryItem = &(_target_pud->items[index]);
            Object* item = inventoryItem->item;
            if (itemGetType(item) == ITEM_TYPE_CONTAINER) {
                _target_curr_stack += 1;
                _target_stack[_target_curr_stack] = item;
                _target_stack_offset[_target_curr_stack] = 0;

                _target_pud = &(item->data.inventory);

                _display_body(item->fid, inventoryWindowType);
                _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, inventoryWindowType);
                windowRefresh(gInventoryWindow);
            }
        }
    } else {
        int index = _pud->length - (_stack_offset[_curr_stack] + keyCode - KEYCODE_GRID_BASE + 1);
        if (index < _pud->length && _curr_stack < 9) {
            InventoryItem* inventoryItem = &(_pud->items[index]);
            Object* item = inventoryItem->item;
            if (itemGetType(item) == ITEM_TYPE_CONTAINER) {
                _curr_stack += 1;

                _stack[_curr_stack] = item;
                _stack_offset[_curr_stack] = 0;

                _inven_dude = _stack[_curr_stack];
                _pud = &(item->data.inventory);

                _adjust_fid();
                _display_body(-1, inventoryWindowType);
                _display_inventory(_stack_offset[_curr_stack], -1, inventoryWindowType);
            }
        }
    }
}

// 0x476394
static void _container_exit(int keyCode, int inventoryWindowType)
{
    if (keyCode == INVENTORY_BUTTON_LEFT) {
        if (_curr_stack > 0) {
            _curr_stack -= 1;
            _inven_dude = _stack[_curr_stack];
            _pud = &_inven_dude->data.inventory;
            _adjust_fid();
            _display_body(-1, inventoryWindowType);
            _display_inventory(_stack_offset[_curr_stack], -1, inventoryWindowType);
        }
    } else if (keyCode == INVENTORY_BUTTON_RIGHT) {
        if (_target_curr_stack > 0) {
            _target_curr_stack -= 1;
            Object* target = _target_stack[_target_curr_stack];
            _target_pud = &(target->data.inventory);
            _display_body(target->fid, inventoryWindowType);
            _display_target_inventory(_target_stack_offset[_target_curr_stack], -1, _target_pud, inventoryWindowType);
            windowRefresh(gInventoryWindow);
        }
    }
}

// Drop item inside a container item (bag, backpack, etc.).
// 0x476464
static int _drop_into_container(Object* container, Object* item, int sourceIndex, Object** itemSlot, int quantity)
{
    int quantityToMove;
    if (quantity > 1) {
        quantityToMove = inventoryQuantitySelect(INVENTORY_WINDOW_TYPE_MOVE_ITEMS, item, quantity);
    } else {
        quantityToMove = 1;
    }

    if (quantityToMove == -1) {
        return -1;
    }

    if (sourceIndex != -1) {
        if (itemRemove(_inven_dude, item, quantityToMove) == -1) {
            return -1;
        }
    }

    int rc = itemAttemptAdd(container, item, quantityToMove);
    if (rc != 0) {
        if (sourceIndex != -1) {
            // SFALL: Fix for items disappearing from inventory when you try to
            // drag them to bag/backpack in the inventory list and are
            // overloaded.
            itemAdd(_inven_dude, item, quantityToMove);
        }
    } else {
        if (itemSlot != nullptr) {
            if (itemSlot == &gInventoryArmor) {
                adjustCritterStatsOnArmorChange(_stack[0], gInventoryArmor, nullptr);
            }
            *itemSlot = nullptr;
        }
    }

    return rc;
}

// 0x47650C
static int _drop_ammo_into_weapon(Object* weapon, Object* ammo, Object** ammoItemSlot, int quantity, int keyCode, Object* ammoOwner)
{
    if (itemGetType(weapon) != ITEM_TYPE_WEAPON) return -1;
    if (itemGetType(ammo) != ITEM_TYPE_AMMO) return -1;
    if (!weaponCanBeReloadedWith(weapon, ammo)) return -1;

    int quantityToMove;
    if (quantity > 1) {
        quantityToMove = inventoryQuantitySelect(INVENTORY_WINDOW_TYPE_MOVE_ITEMS, ammo, quantity);
    } else {
        quantityToMove = 1;
    }
    if (quantityToMove == -1) return -1;

    // Remove the weapon from its owner (player) temporarily
    int rc = itemRemove(_inven_dude, weapon, 1);

    bool isReloaded = false;
    Object* sourceItem = ammo;
    for (int i = 0; i < quantityToMove; i++) {
        // Reload the weapon with this ammo
        int rcReload = weaponReload(weapon, sourceItem);
        if (rcReload == 0) {
            // Reload succeeded – remove the ammo from its owner
            if (itemRemove(ammoOwner, sourceItem, 1) == 0) {
                // Rebuild combined inventory immediately after removal
                if (gUseCombinedInventory) {
                    inventoryBuildCombinedList(gDude);
                }
                if (ammoItemSlot != nullptr) {
                    *ammoItemSlot = nullptr;
                }
                objectDestroy(sourceItem);
                isReloaded = true;
            } else {
                // Ammo removal failed – stop further attempts
                break;
            }
            // Try to get next ammo item for multi-round reload
            if (_inven_from_button(keyCode, &sourceItem, nullptr, nullptr) == 0) {
                break;
            }
        } else if (rcReload != -1) {
            // Partial reload (e.g., some rounds loaded, some failed)
            isReloaded = true;
        }
        if (rcReload != 0) {
            break;
        }
    }

    // Re-add the weapon (even if reload partially succeeded)
    if (rc != -1) {
        itemAdd(_inven_dude, weapon, 1);
        // Rebuild combined inventory after adding weapon back
        if (gUseCombinedInventory) {
            inventoryBuildCombinedList(gDude);
        }
    }

    if (!isReloaded) {
        return -1;
    }

    // Play reload sound
    const char* sfx = sfxBuildWeaponName(WEAPON_SOUND_EFFECT_READY, weapon, HIT_MODE_RIGHT_WEAPON_PRIMARY, nullptr);
    soundPlayFile(sfx);

    return 0;
}

// 0x47664C
static void _draw_amount(int value, int inventoryWindowType)
{
    // BIGNUM.frm
    FrmImage numbersFrmImage;
    int numbersFid = buildFid(OBJ_TYPE_INTERFACE, 170, 0, 0, 0);
    if (!numbersFrmImage.lock(numbersFid)) {
        return;
    }

    Rect rect;

    int windowWidth = windowGetWidth(_mt_wid);
    unsigned char* windowBuffer = windowGetBuffer(_mt_wid);

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_MOVE_ITEMS) {
        rect.left = 125;
        rect.top = 45;
        rect.right = 195;
        rect.bottom = 69;

        int ranks[5];
        ranks[4] = value % 10;
        ranks[3] = value / 10 % 10;
        ranks[2] = value / 100 % 10;
        ranks[1] = value / 1000 % 10;
        ranks[0] = value / 10000 % 10;

        windowBuffer += rect.top * windowWidth + rect.left;

        for (int index = 0; index < 5; index++) {
            unsigned char* src = numbersFrmImage.getData() + 14 * ranks[index];
            blitBufferToBuffer(src, 14, 24, 336, windowBuffer, windowWidth);
            windowBuffer += 14;
        }
    } else {
        rect.left = 133;
        rect.top = 64;
        rect.right = 189;
        rect.bottom = 88;

        windowBuffer += windowWidth * rect.top + rect.left;
        blitBufferToBuffer(numbersFrmImage.getData() + 14 * (value / 60), 14, 24, 336, windowBuffer, windowWidth);
        blitBufferToBuffer(numbersFrmImage.getData() + 14 * (value % 60 / 10), 14, 24, 336, windowBuffer + 14 * 2, windowWidth);
        blitBufferToBuffer(numbersFrmImage.getData() + 14 * (value % 10), 14, 24, 336, windowBuffer + 14 * 3, windowWidth);
    }

    windowRefreshRect(_mt_wid, &rect);
}

// 0x47688C
static int inventoryQuantitySelect(int inventoryWindowType, Object* item, int max, int defaultValue)
{
    ScopedGameMode gm(GameMode::kCounter);

    inventoryQuantityWindowInit(inventoryWindowType, item);

    int value;
    int min;
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_MOVE_ITEMS) {
        if (max > 99999) {
            max = 99999;
        }
        min = 1;
        value = std::clamp(defaultValue, min, max);
    } else {
        value = 60;
        min = 10;
    }

    _draw_amount(value, inventoryWindowType);

    bool isTyping = false;
    for (;;) {
        sharedFpsLimiter.mark();

        int keyCode = inputGetInput();
        if (keyCode == KEY_ESCAPE) {
            inventoryQuantityWindowFree(inventoryWindowType);
            return -1;
        }

        if (keyCode == KEY_RETURN || keyCode == BUTTON_DONE) {
            if (value >= min && value <= max) {
                if (inventoryWindowType != INVENTORY_WINDOW_TYPE_SET_TIMER || value % 10 == 0) {
                    if (keyCode != BUTTON_DONE) {
                        soundPlayFile("ib1p1xx1");
                    }
                    break;
                }
            }

            soundPlayFile("iisxxxx1");

        } else if (keyCode == BUTTON_ALL || keyCode == KEY_LOWERCASE_A) {
            if (keyCode == KEY_LOWERCASE_A) {
                soundPlayFile("ib1p1xx1");
            }
            isTyping = false;
            value = max;
            _draw_amount(value, inventoryWindowType);

            if (!settings.enhancements.strict_vanilla) {
                // For move items, treat "All" as immediate confirmation
                if (inventoryWindowType == INVENTORY_WINDOW_TYPE_MOVE_ITEMS) {
                    break; // Exit loop to return the value
                }
            }
        } else if (keyCode == BUTTON_PLUS) {
            isTyping = false;
            if (value < max) {
                if (inventoryWindowType == INVENTORY_WINDOW_TYPE_MOVE_ITEMS) {
                    if ((mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) != 0) {
                        getTicks();

                        unsigned int delay = 100;
                        while ((mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) != 0) {
                            sharedFpsLimiter.mark();

                            if (value < max) {
                                value++;
                            }

                            _draw_amount(value, inventoryWindowType);
                            inputGetInput();

                            if (delay > 1) {
                                delay--;
                                inputPauseForTocks(delay);
                            }

                            renderPresent();
                            sharedFpsLimiter.throttle();
                        }
                    } else {
                        if (value < max) {
                            value++;
                        }
                    }
                } else {
                    value += 10;
                }

                _draw_amount(value, inventoryWindowType);
                continue;
            }
        } else if (keyCode == BUTTON_MINUS) {
            isTyping = false;
            if (value > min) {
                if (inventoryWindowType == INVENTORY_WINDOW_TYPE_MOVE_ITEMS) {
                    if ((mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) != 0) {
                        getTicks();

                        unsigned int delay = 100;
                        while ((mouseGetEvent() & MOUSE_EVENT_LEFT_BUTTON_REPEAT) != 0) {
                            sharedFpsLimiter.mark();

                            if (value > min) {
                                value--;
                            }

                            _draw_amount(value, inventoryWindowType);
                            inputGetInput();

                            if (delay > 1) {
                                delay--;
                                inputPauseForTocks(delay);
                            }

                            renderPresent();
                            sharedFpsLimiter.throttle();
                        }
                    } else {
                        if (value > min) {
                            value--;
                        }
                    }
                } else {
                    value -= 10;
                }

                _draw_amount(value, inventoryWindowType);
                continue;
            }
        }

        if (inventoryWindowType == INVENTORY_WINDOW_TYPE_MOVE_ITEMS) {
            if (keyCode >= KEY_0 && keyCode <= KEY_9) {
                int number = keyCode - KEY_0;
                if (!isTyping) {
                    value = 0;
                }

                value = 10 * value % 100000 + number;
                isTyping = true;

                _draw_amount(value, inventoryWindowType);
                continue;
            } else if (keyCode == KEY_BACKSPACE) {
                if (!isTyping) {
                    value = 0;
                }

                value /= 10;
                isTyping = true;

                _draw_amount(value, inventoryWindowType);
                continue;
            }
        }

        renderPresent();
        sharedFpsLimiter.throttle();
    }

    inventoryQuantityWindowFree(inventoryWindowType);

    return value;
}

// Creates move items/set timer interface.
//
// 0x476AB8
static int inventoryQuantityWindowInit(int inventoryWindowType, Object* item)
{
    const int oldFont = fontGetCurrent();
    fontSetCurrent(103);

    const InventoryWindowDescription* windowDescription = &(gInventoryWindowDescriptions[inventoryWindowType]);

    // Maintain original position in original resolution, otherwise center it.
    int quantityWindowX = screenGetWidth() != 640
        ? (screenGetWidth() - windowDescription->width) / 2
        : windowDescription->x;
    int quantityWindowY = screenGetHeight() != 480
        ? (screenGetHeight() - windowDescription->height) / 2
        : windowDescription->y;
    _mt_wid = windowCreate(quantityWindowX, quantityWindowY, windowDescription->width, windowDescription->height, 257, WINDOW_MODAL | WINDOW_MOVE_ON_TOP | WINDOW_TRANSPARENT);
    unsigned char* windowBuffer = windowGetBuffer(_mt_wid);

    FrmImage backgroundFrmImage;
    int backgroundFid = buildFid(OBJ_TYPE_INTERFACE, windowDescription->frmId, 0, 0, 0);
    if (backgroundFrmImage.lock(backgroundFid)) {
        blitBufferToBuffer(backgroundFrmImage.getData(),
            windowDescription->width,
            windowDescription->height,
            windowDescription->width,
            windowBuffer,
            windowDescription->width);
    }

    MessageListItem messageListItem;
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_MOVE_ITEMS) {
        messageListItem.num = 21; // MOVE ITEMS
        if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
            int length = fontGetStringWidth(messageListItem.text);
            fontDrawText(windowBuffer + windowDescription->width * 9 + (windowDescription->width - length) / 2, messageListItem.text, 200, windowDescription->width, _colorTable[COL_OLIVE_YELLOW]);
        }
    } else if (inventoryWindowType == INVENTORY_WINDOW_TYPE_SET_TIMER) {
        messageListItem.num = 23; // SET TIMER
        if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
            int length = fontGetStringWidth(messageListItem.text);
            fontDrawText(windowBuffer + windowDescription->width * 9 + (windowDescription->width - length) / 2, messageListItem.text, 200, windowDescription->width, _colorTable[COL_OLIVE_YELLOW]);
        }

        // Timer overlay
        FrmImage overlayFrmImage;
        int overlayFid = buildFid(OBJ_TYPE_INTERFACE, 306, 0, 0, 0);
        if (overlayFrmImage.lock(overlayFid)) {
            blitBufferToBuffer(overlayFrmImage.getData(),
                105,
                81,
                105,
                windowBuffer + 34 * windowDescription->width + 113,
                windowDescription->width);
        }
    }

    int inventoryFid = itemGetInventoryFid(item);
    artRender(inventoryFid, windowBuffer + windowDescription->width * 46 + 16, INVENTORY_LARGE_SLOT_WIDTH, INVENTORY_LARGE_SLOT_HEIGHT, windowDescription->width);

    int x;
    int y;
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_MOVE_ITEMS) {
        x = 200;
        y = 46;
    } else {
        x = 194;
        y = 64;
    }

    int fid;
    int btn;

    // Plus button
    fid = buildFid(OBJ_TYPE_INTERFACE, 193, 0, 0, 0);
    _moveFrmImages[0].lock(fid);

    fid = buildFid(OBJ_TYPE_INTERFACE, 194, 0, 0, 0);
    _moveFrmImages[1].lock(fid);

    if (_moveFrmImages[0].isLocked() && _moveFrmImages[1].isLocked()) {
        btn = buttonCreate(_mt_wid,
            x,
            y,
            16,
            12,
            -1,
            -1,
            BUTTON_PLUS,
            -1,
            _moveFrmImages[0].getData(),
            _moveFrmImages[1].getData(),
            nullptr,
            BUTTON_FLAG_TRANSPARENT);
        if (btn != -1) {
            buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
        }
    }

    // Minus button
    fid = buildFid(OBJ_TYPE_INTERFACE, 191, 0, 0, 0);
    _moveFrmImages[2].lock(fid);

    fid = buildFid(OBJ_TYPE_INTERFACE, 192, 0, 0, 0);
    _moveFrmImages[3].lock(fid);

    if (_moveFrmImages[2].isLocked() && _moveFrmImages[3].isLocked()) {
        btn = buttonCreate(_mt_wid,
            x,
            y + 11,
            17,
            12,
            -1,
            -1,
            BUTTON_MINUS,
            -1,
            _moveFrmImages[2].getData(),
            _moveFrmImages[3].getData(),
            nullptr,
            BUTTON_FLAG_TRANSPARENT);
        if (btn != -1) {
            buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
        }
    }

    fid = buildFid(OBJ_TYPE_INTERFACE, 8, 0, 0, 0);
    _moveFrmImages[4].lock(fid);

    fid = buildFid(OBJ_TYPE_INTERFACE, 9, 0, 0, 0);
    _moveFrmImages[5].lock(fid);

    if (_moveFrmImages[4].isLocked() && _moveFrmImages[5].isLocked()) {
        // Done
        btn = buttonCreate(_mt_wid,
            98,
            128,
            15,
            16,
            -1,
            -1,
            -1,
            BUTTON_DONE,
            _moveFrmImages[4].getData(),
            _moveFrmImages[5].getData(),
            nullptr,
            BUTTON_FLAG_TRANSPARENT);
        if (btn != -1) {
            buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
        }

        // Cancel
        btn = buttonCreate(_mt_wid,
            148,
            128,
            15,
            16,
            -1,
            -1,
            -1,
            KEY_ESCAPE,
            _moveFrmImages[4].getData(),
            _moveFrmImages[5].getData(),
            nullptr,
            BUTTON_FLAG_TRANSPARENT);
        if (btn != -1) {
            buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
        }
    }

    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_MOVE_ITEMS) {
        fid = buildFid(OBJ_TYPE_INTERFACE, 307, 0, 0, 0);
        _moveFrmImages[6].lock(fid);

        fid = buildFid(OBJ_TYPE_INTERFACE, 308, 0, 0, 0);
        _moveFrmImages[7].lock(fid);

        if (_moveFrmImages[6].isLocked() && _moveFrmImages[7].isLocked()) {
            messageListItem.num = 22; // ALL
            if (messageListGetItem(&gInventoryMessageList, &messageListItem)) {
                int length = fontGetStringWidth(messageListItem.text);

                // TODO: Where is y? Is it hardcoded in to 376?
                fontDrawText(_moveFrmImages[6].getData() + (94 - length) / 2 + 376, messageListItem.text, 200, 94, _colorTable[COL_OLIVE_YELLOW]);
                fontDrawText(_moveFrmImages[7].getData() + (94 - length) / 2 + 376, messageListItem.text, 200, 94, _colorTable[COL_GREENISH_BROWN]);

                btn = buttonCreate(_mt_wid,
                    120,
                    80,
                    94,
                    33,
                    -1,
                    -1,
                    -1,
                    BUTTON_ALL,
                    _moveFrmImages[6].getData(),
                    _moveFrmImages[7].getData(),
                    nullptr,
                    BUTTON_FLAG_TRANSPARENT);
                if (btn != -1) {
                    buttonSetCallbacks(btn, _gsound_red_butt_press, _gsound_red_butt_release);
                }
            }
        }
    }

    windowRefresh(_mt_wid);
    inventorySetCursor(INVENTORY_WINDOW_CURSOR_ARROW);
    fontSetCurrent(oldFont);

    return 0;
}

// 0x477030
static int inventoryQuantityWindowFree(int inventoryWindowType)
{
    int count = inventoryWindowType == INVENTORY_WINDOW_TYPE_MOVE_ITEMS ? 8 : 6;

    for (int index = 0; index < count; index++) {
        _moveFrmImages[index].unlock();
    }

    windowDestroy(_mt_wid);

    return 0;
}

// 0x477074
int inventorySetTimer(Object* item)
{
    bool isInitialized = _inven_is_initialized;

    if (!isInitialized) {
        if (inventoryCommonInit() == -1) {
            return -1;
        }
    }

    int seconds = inventoryQuantitySelect(INVENTORY_WINDOW_TYPE_SET_TIMER, item, 180);

    if (!isInitialized) {
        // NOTE: Uninline.
        inventoryCommonFree();
    }

    return seconds;
}

Object* inventoryGetTargetObject()
{
    return _target_stack[_target_curr_stack];
}

static void tradeWindowUpdateScrollButtons()
{
    // Left inventory (player)
    if (gTradeLeftUpButton != -1) {
        if (_stack_offset[_curr_stack] > 0)
            buttonEnable(gTradeLeftUpButton);
        else
            buttonDisable(gTradeLeftUpButton);
    }
    if (gTradeLeftDownButton != -1) {
        int totalItems = getFilteredCount(); // uses combined inventory & filter
        int visible = gInventorySlotsCount;
        if (totalItems - _stack_offset[_curr_stack] > visible)
            buttonEnable(gTradeLeftDownButton);
        else
            buttonDisable(gTradeLeftDownButton);
    }

    // Right inventory (merchant/NPC)
    if (gTradeRightUpButton != -1) {
        if (_target_stack_offset[_target_curr_stack] > 0)
            buttonEnable(gTradeRightUpButton);
        else
            buttonDisable(gTradeRightUpButton);
    }
    if (gTradeRightDownButton != -1) {
        int totalItems = _target_pud->length;
        if (gFilterCategory != -1) {
            // Recompute filtered count for the right inventory
            totalItems = buildFilteredIndices(_target_pud);
        }
        int visible = gInventorySlotsCount;
        if (totalItems - _target_stack_offset[_target_curr_stack] > visible)
            buttonEnable(gTradeRightDownButton);
        else
            buttonDisable(gTradeRightDownButton);
    }

    // Offer tables (inner) - these are not filtered, so they remain unchanged
    if (gTradeOfferLeftUpButton != -1) {
        if (_ptable_offset > 0)
            buttonEnable(gTradeOfferLeftUpButton);
        else
            buttonDisable(gTradeOfferLeftUpButton);
    }
    if (gTradeOfferLeftDownButton != -1) {
        int visible = gInventorySlotsCount;
        if (_ptable_pud->length - _ptable_offset > visible)
            buttonEnable(gTradeOfferLeftDownButton);
        else
            buttonDisable(gTradeOfferLeftDownButton);
    }

    if (gTradeOfferRightUpButton != -1) {
        if (_btable_offset > 0)
            buttonEnable(gTradeOfferRightUpButton);
        else
            buttonDisable(gTradeOfferRightUpButton);
    }
    if (gTradeOfferRightDownButton != -1) {
        int visible = gInventorySlotsCount;
        if (_btable_pud->length - _btable_offset > visible)
            buttonEnable(gTradeOfferRightDownButton);
        else
            buttonDisable(gTradeOfferRightDownButton);
    }
}

// Build the combined list from all party members' inventories
/**
 * Builds the combined inventory list from all active party members
 * (player + companions) for display in the left-side inventory.
 *
 * The list is ordered with companions first, then the player last.
 * Since the display renders from the end of the list, the player's items
 * will appear at the top, which matches the vanilla trade/loot behavior
 * where newly acquired items appear at the top.
 */
static void inventoryBuildCombinedList(Object* focusOwner)
{

    if (settings.enhancements.strict_vanilla || !settings.enhancements.companion_inventory) {
        gCombinedItemCount = 0;
        return;
    }
    if (focusOwner == nullptr) focusOwner = _inven_dude;
    gCombinedItemCount = 0;

    std::vector<Object*> partyMembers = get_all_party_members_objects(false);
    // Fallback if get_all_party_members_objects fails...
    if (partyMembers.empty()) {
        if (gDude != nullptr) partyMembers.push_back(gDude);
        for (int i = 0; i < 6; i++) {
            int pid = gPartyMemberPids[i];
            if (pid == -1) continue;
            if (gDude != nullptr && pid == gDude->pid) continue;
            Object* member = partyMemberFindByPid(pid);
            if (member != nullptr) partyMembers.push_back(member);
        }
    }

    // Move the focus owner to the end of the list
    auto it = std::find(partyMembers.begin(), partyMembers.end(), focusOwner);
    if (it != partyMembers.end()) {
        partyMembers.erase(it);
        partyMembers.push_back(focusOwner);
    }

    // Build combined list in this order (focus owner last)
    for (Object* member : partyMembers) {
        if (member == nullptr) continue;

        // Skip the excluded object (e.g., loot target, trade NPC)
        if (member == gCombinedExcludeObject) continue;

        // Safety checks for companions
        if (member != gDude) {
            // Skip dead companions (can't access their inventory)
            if (critterIsDead(member)) {
                debugPrint("inventoryBuildCombinedList: Skipping %s (dead)\n", objectGetName(member));
                continue;
            }
            // Ensure they're still a party member
            // (in case get_all_party_members_objects returned a stale pointer)
            if (!objectIsPartyMember(member)) {
                debugPrint("inventoryBuildCombinedList: Skipping %s (not in party)\n", objectGetName(member));
                continue;
            }
        }

        Inventory* inv = &member->data.inventory;
        if (inv == nullptr) continue;

        for (int i = 0; i < inv->length && gCombinedItemCount < MAX_COMBINED_ITEMS; i++) {
            InventoryItem* invItem = &inv->items[i];
            Object* item = invItem->item;
            if (item == nullptr) continue;
            if (item->flags & OBJECT_HIDDEN) continue;
            if (item->flags & (OBJECT_IN_LEFT_HAND | OBJECT_IN_RIGHT_HAND | OBJECT_WORN)) continue;

            gCombinedItems[gCombinedItemCount].item = item;
            gCombinedItems[gCombinedItemCount].quantity = invItem->quantity;
            gCombinedItems[gCombinedItemCount].owner = member;
            gCombinedItemCount++;
        }
    }

    // Re-apply persistent sort if active
    if (gCombinedSortType != -1 && gCombinedItemCount > 1) {
        applyCombinedSort(gCombinedSortType);
    }
    if (gIsTradeWindow) {
        movePlayerMoneyToTopCombined();
    }
}

// Move item from original owner to current character if needed
static void transferItemToCurrentOwner(Object* item, int quantity, Object* originalOwner)
{
    if (originalOwner == nullptr) {
        debugPrint("transferItemToCurrentOwner: originalOwner is null! Skipping.\n");
        return;
    }
    if (gUseCombinedInventory && originalOwner != _inven_dude) {
        if (itemRemove(originalOwner, item, quantity) == 0) {
            itemAdd(_inven_dude, item, quantity);
        } else {
            debugPrint("transferItemToCurrentOwner: failed to remove item from original owner\n");
        }
    }
}

static void movePlayerMoneyToTopCombined()
{
    if (!gUseCombinedInventory || gCombinedItemCount <= 1) return;

    // Collect indices of money items belonging to the player
    std::vector<int> moneyIndices;
    for (int i = 0; i < gCombinedItemCount; i++) {
        CombinedItem* ci = &gCombinedItems[i];
        if (ci->owner == gDude && ci->item->pid == PROTO_ID_MONEY) {
            moneyIndices.push_back(i);
        }
    }
    if (moneyIndices.empty()) return;

    // Move all non-money items to the front, preserving order
    CombinedItem* tempArray = (CombinedItem*)alloca(gCombinedItemCount * sizeof(CombinedItem));
    int writePos = 0;
    for (int i = 0; i < gCombinedItemCount; i++) {
        bool isPlayerMoney = false;
        for (int idx : moneyIndices) {
            if (i == idx) {
                isPlayerMoney = true;
                break;
            }
        }
        if (!isPlayerMoney) {
            tempArray[writePos++] = gCombinedItems[i];
        }
    }
    // Append player's money items at the end (preserving their order)
    for (int idx : moneyIndices) {
        tempArray[writePos++] = gCombinedItems[idx];
    }

    memcpy(gCombinedItems, tempArray, gCombinedItemCount * sizeof(CombinedItem));
}

// Type-priority order for combined-inventory type sort (lower = higher priority, sorts toward the top)
static int getItemTypePriority(int itemType)
{
    switch (itemType) {
    case ITEM_TYPE_WEAPON:
        return 1;
    case ITEM_TYPE_AMMO:
        return 2;
    case ITEM_TYPE_DRUG:
        return 3;
    case ITEM_TYPE_ARMOR:
        return 4;
    case ITEM_TYPE_MISC:
        return 5;
    case ITEM_TYPE_CONTAINER:
        return 6;
    case ITEM_TYPE_KEY:
        return 7;
    default:
        return MAX_SORT_PRIORITY;
    }
}

static int compareCombinedItemsByType(const void* a, const void* b)
{
    const CombinedItem* ca = (const CombinedItem*)a;
    const CombinedItem* cb = (const CombinedItem*)b;

    int typeA = itemGetType(ca->item);
    int typeB = itemGetType(cb->item);

    int orderA = getItemTypePriority(typeA);
    int orderB = getItemTypePriority(typeB);

    if (orderA != orderB) return orderB - orderA; // lower order -> end of array -> top of display

    // Same type: type-specific sorting
    switch (typeA) {
    case ITEM_TYPE_WEAPON: {
        int minA, maxA, minB, maxB;
        weaponGetDamageMinMax(ca->item, &minA, &maxA);
        weaponGetDamageMinMax(cb->item, &minB, &maxB);
        int avgA = (minA + maxA) / 2;
        int avgB = (minB + maxB) / 2;
        return avgA - avgB; // ascending -> high damage at end (top)
    }
    case ITEM_TYPE_AMMO:
        return ca->quantity - cb->quantity; // ascending -> large stacks at end (top)
    case ITEM_TYPE_DRUG: {
        bool healA = itemIsHealing(ca->item->pid);
        bool healB = itemIsHealing(cb->item->pid);
        if (healA && !healB) return 1; // healing after non-healing -> healing at end (top)
        if (!healA && healB) return -1;
        int valueA = itemGetCost(ca->item) * ca->quantity;
        int valueB = itemGetCost(cb->item) * cb->quantity;
        return valueA - valueB; // ascending ? high value at end (top)
    }
    case ITEM_TYPE_ARMOR: {
        int drA = armorGetDamageResistance(ca->item, 0);
        int drB = armorGetDamageResistance(cb->item, 0);
        return drA - drB; // ascending -> high DR at end (top)
    }
    default: {
        const char* nameA = objectGetName(ca->item);
        const char* nameB = objectGetName(cb->item);
        if (nameA == nullptr || nameB == nullptr) {
            if (nameA == nullptr && nameB == nullptr) return 0;
            if (nameA == nullptr) return -1;
            return 1;
        }
        return strcmp(nameA, nameB);
    }
    }
}

// Shared: compare items by name, used as a tiebreaker when a primary sort key is equal
static int compareItemNames(Object* itemA, Object* itemB)
{
    const char* nameA = objectGetName(itemA);
    const char* nameB = objectGetName(itemB);
    if (nameA == nullptr && nameB == nullptr) return 0;
    if (nameA == nullptr) return -1;
    if (nameB == nullptr) return 1;
    return strcmp(nameA, nameB);
}

// Shared: total stack weight/value, ascending (lightest/least valuable first);
// ties broken by name so equal-weight/value items land in a stable, repeatable order
static int compareByTotalWeight(Object* itemA, int quantityA, Object* itemB, int quantityB)
{
    int totalWeightA = itemGetWeight(itemA) * quantityA;
    int totalWeightB = itemGetWeight(itemB) * quantityB;
    if (totalWeightA != totalWeightB) {
        return totalWeightA - totalWeightB;
    }
    return compareItemNames(itemA, itemB);
}

static int compareByTotalValue(Object* itemA, int quantityA, Object* itemB, int quantityB)
{
    int totalValueA = itemGetCost(itemA) * quantityA;
    int totalValueB = itemGetCost(itemB) * quantityB;
    if (totalValueA != totalValueB) {
        return totalValueA - totalValueB;
    }
    return compareItemNames(itemA, itemB);
}

// Compare by total stack weight (heaviest at top)
static int _compare_items_by_weight(const void* a, const void* b)
{
    InventoryItem* itemA = (InventoryItem*)a;
    InventoryItem* itemB = (InventoryItem*)b;
    return compareByTotalWeight(itemA->item, itemA->quantity, itemB->item, itemB->quantity);
}

// Compare by total stack value (most valuable at top)
static int _compare_items_by_value(const void* a, const void* b)
{
    InventoryItem* itemA = (InventoryItem*)a;
    InventoryItem* itemB = (InventoryItem*)b;
    return compareByTotalValue(itemA->item, itemA->quantity, itemB->item, itemB->quantity);
}

// Compare by total weight (heaviest at end)
static int compareCombinedItemsByWeight(const void* a, const void* b)
{
    const CombinedItem* ca = (const CombinedItem*)a;
    const CombinedItem* cb = (const CombinedItem*)b;
    return compareByTotalWeight(ca->item, ca->quantity, cb->item, cb->quantity);
}

// Compare by total value (most valuable at end)
static int compareCombinedItemsByValue(const void* a, const void* b)
{
    const CombinedItem* ca = (const CombinedItem*)a;
    const CombinedItem* cb = (const CombinedItem*)b;
    return compareByTotalValue(ca->item, ca->quantity, cb->item, cb->quantity);
}

// Shared tiebreak: item of the target type sorts after non-target items; ties broken by name.
static int compareByTypeThenName(Object* itemA, Object* itemB, bool isTargetA, bool isTargetB)
{
    if (isTargetA && !isTargetB) return 1;
    if (!isTargetA && isTargetB) return -1;
    return compareItemNames(itemA, itemB);
}

// Weapons first, then by name
static int compareCombinedItemsByWeapons(const void* a, const void* b)
{
    const CombinedItem* ca = (const CombinedItem*)a;
    const CombinedItem* cb = (const CombinedItem*)b;
    return compareByTypeThenName(ca->item, cb->item,
        itemGetType(ca->item) == ITEM_TYPE_WEAPON,
        itemGetType(cb->item) == ITEM_TYPE_WEAPON);
}

// Ammo first, then by name
static int compareCombinedItemsByAmmo(const void* a, const void* b)
{
    const CombinedItem* ca = (const CombinedItem*)a;
    const CombinedItem* cb = (const CombinedItem*)b;
    return compareByTypeThenName(ca->item, cb->item,
        itemGetType(ca->item) == ITEM_TYPE_AMMO,
        itemGetType(cb->item) == ITEM_TYPE_AMMO);
}

// Drugs first, then by name
static int compareCombinedItemsByDrugs(const void* a, const void* b)
{
    const CombinedItem* ca = (const CombinedItem*)a;
    const CombinedItem* cb = (const CombinedItem*)b;
    return compareByTypeThenName(ca->item, cb->item,
        itemGetType(ca->item) == ITEM_TYPE_DRUG,
        itemGetType(cb->item) == ITEM_TYPE_DRUG);
}

// Other (Misc, Container, Key, Armor) first, then by name
static int compareCombinedItemsByOther(const void* a, const void* b)
{
    const CombinedItem* ca = (const CombinedItem*)a;
    const CombinedItem* cb = (const CombinedItem*)b;
    int typeA = itemGetType(ca->item);
    int typeB = itemGetType(cb->item);
    bool isOtherA = (typeA == ITEM_TYPE_MISC || typeA == ITEM_TYPE_CONTAINER || typeA == ITEM_TYPE_KEY || typeA == ITEM_TYPE_ARMOR);
    bool isOtherB = (typeB == ITEM_TYPE_MISC || typeB == ITEM_TYPE_CONTAINER || typeB == ITEM_TYPE_KEY || typeB == ITEM_TYPE_ARMOR);
    return compareByTypeThenName(ca->item, cb->item, isOtherA, isOtherB);
}

static void applyCombinedSort(int sortType)
{
    if (!gUseCombinedInventory) return;
    if (gCombinedItemCount <= 1) return;

    switch (sortType) {
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_DEFAULT:
        qsort(gCombinedItems, gCombinedItemCount, sizeof(CombinedItem), compareCombinedItemsByType);
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_WEAPONS:
        qsort(gCombinedItems, gCombinedItemCount, sizeof(CombinedItem), compareCombinedItemsByWeapons);
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_AMMO:
        qsort(gCombinedItems, gCombinedItemCount, sizeof(CombinedItem), compareCombinedItemsByAmmo);
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_DRUGS:
        qsort(gCombinedItems, gCombinedItemCount, sizeof(CombinedItem), compareCombinedItemsByDrugs);
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_OTHER:
        qsort(gCombinedItems, gCombinedItemCount, sizeof(CombinedItem), compareCombinedItemsByOther);
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_WEIGHT:
        qsort(gCombinedItems, gCombinedItemCount, sizeof(CombinedItem), compareCombinedItemsByWeight);
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_VALUE:
        qsort(gCombinedItems, gCombinedItemCount, sizeof(CombinedItem), compareCombinedItemsByValue);
        break;
    case GAME_MOUSE_ACTION_MENU_ITEM_SORT_REVERSE:
        for (int i = 0; i < gCombinedItemCount / 2; i++) {
            CombinedItem temp = gCombinedItems[i];
            gCombinedItems[i] = gCombinedItems[gCombinedItemCount - 1 - i];
            gCombinedItems[gCombinedItemCount - 1 - i] = temp;
        }
        break;
    default:
        return;
    }
    // After sorting, if in trade window, ensure player's money is at the end
    if (gIsTradeWindow) {
        movePlayerMoneyToTopCombined();
    }
}

static void sortCombinedInventory(int sortType, int inventoryWindowType)
{

    if (!gUseCombinedInventory) return;

    applyCombinedSort(sortType); // sorts the array in place

    // Refresh display
    _display_inventory(_stack_offset[_curr_stack], -1, inventoryWindowType);
    if (inventoryWindowType == INVENTORY_WINDOW_TYPE_NORMAL) {
        inventoryRenderSummary();
    }
    windowRefresh(gInventoryWindow);
}

static void inventoryBuildPartyList()
{

    if (settings.enhancements.strict_vanilla || !settings.enhancements.companion_inventory) {
        gPartyList.clear();
        return;
    }
    gPartyList.clear();

    // Always include the player
    if (gDude != nullptr) {
        gPartyList.push_back(gDude);
    }

    // Add all party members (excluding player) from the existing function
    std::vector<Object*> members = get_all_party_members_objects(false);
    for (Object* member : members) {
        if (member != nullptr && member != gDude && critterIsActive(member)) {
            gPartyList.push_back(member);
        }
    }
}

} // namespace fallout
