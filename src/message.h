#ifndef MESSAGE_H
#define MESSAGE_H

#include <stddef.h>
#include <stdint.h>

namespace fallout {

#define MESSAGE_LIST_ITEM_TEXT_FILTERED 0x01
#define MESSAGE_LIST_ITEM_FIELD_MAX_SIZE 1024

#define MESSAGE_LIST_MAP "MAP"
#define MESSAGE_LIST_WORLDMAP "WORLDMAP"
#define MESSAGE_LIST_PROTO "PROTO"
#define MESSAGE_LIST_PIPBOY "PIPBOY"
#define MESSAGE_LIST_COMBAT "COMBAT"
#define MESSAGE_LIST_COMBATAI "COMBATAI"
#define MESSAGE_LIST_QUESTS "QUESTS"

// Mod block range identifiers (use these as listId in generate_mod_block_base_id)
#define MOD_BLOCK_MAP       1
#define MOD_BLOCK_WORLDMAP  2
#define MOD_BLOCK_PROTO     3
#define MOD_BLOCK_PIPBOY    4
#define MOD_BLOCK_COMBAT    5
#define MOD_BLOCK_COMBATAI  6
#define MOD_BLOCK_QUESTS    7
#define MOD_BLOCK_AREA      8   // world map location names


// CE: Working with standard message lists is tricky in Sfall. Many message
// lists are initialized only for the duration of appropriate modal window. This
// is not documented in Sfall and shifts too much responsibility to scripters
// (who should check game mode before accessing volatile message lists). For now
// CE only exposes persistent standard message lists:
// - combat.msg
// - combatai.msg
// - scrname.msg
// - misc.msg
// - item.msg
// - map.msg
// - proto.msg
// - script.msg
// - skill.msg
// - stat.msg
// - trait.msg
// - worldmap.msg
enum StandardMessageList {
    STANDARD_MESSAGE_LIST_COMBAT,
    STANDARD_MESSAGE_LIST_COMBAT_AI,
    STANDARD_MESSAGE_LIST_SCRNAME,
    STANDARD_MESSAGE_LIST_MISC,
    STANDARD_MESSAGE_LIST_CUSTOM,
    STANDARD_MESSAGE_LIST_INVENTORY,
    STANDARD_MESSAGE_LIST_ITEM,
    STANDARD_MESSAGE_LIST_LSGAME,
    STANDARD_MESSAGE_LIST_MAP,
    STANDARD_MESSAGE_LIST_OPTIONS,
    STANDARD_MESSAGE_LIST_PERK,
    STANDARD_MESSAGE_LIST_PIPBOY,
    STANDARD_MESSAGE_LIST_QUESTS,
    STANDARD_MESSAGE_LIST_PROTO,
    STANDARD_MESSAGE_LIST_SCRIPT,
    STANDARD_MESSAGE_LIST_SKILL,
    STANDARD_MESSAGE_LIST_SKILLDEX,
    STANDARD_MESSAGE_LIST_STAT,
    STANDARD_MESSAGE_LIST_TRAIT,
    STANDARD_MESSAGE_LIST_WORLDMAP,
    STANDARD_MESSAGE_LIST_COUNT,
};

enum {
    PROTO_MESSAGE_LIST_ITEMS,
    PROTO_MESSAGE_LIST_CRITTERS,
    PROTO_MESSAGE_LIST_SCENERY,
    PROTO_MESSAGE_LIST_TILES,
    PROTO_MESSAGE_LIST_WALLS,
    PROTO_MESSAGE_LIST_MISC,
    PROTO_MESSAGE_LIST_COUNT,
};

typedef struct MessageListItem {
    int num;
    int flags;
    char* audio;
    char* text;
} MessageListItem;

typedef struct MessageList {
    int entries_num;
    MessageListItem* entries;
} MessageList;

// Structure to define a block-allocated ID range for a specific mod block type
struct ModBlockRange {
    int listId;      // one of MOD_BLOCK_* constants
    int startId;     // first ID in the range (inclusive)
    int maxId;       // last ID in the range (inclusive)
    int blockSize;   // number of consecutive IDs per mod item (e.g., 100 for holodisks)
};

// Global array of ranges for mod block allocation
extern const ModBlockRange gModBlockRanges[];
extern const int gModBlockRangesCount;

int badwordsInit();
void badwordsExit();
bool messageListInit(MessageList* msg);
bool messageListFree(MessageList* msg);
bool messageListLoad(MessageList* msg, const char* path);
bool messageListGetItem(MessageList* msg, MessageListItem* entry);
bool _message_make_path(char* dest, size_t size, const char* path);
char* getmsg(MessageList* msg, MessageListItem* entry, int num);
bool messageListFilterBadwords(MessageList* messageList);

void messageListFilterGenderWords(MessageList* messageList, int gender);

bool messageListRepositoryInit();
void messageListRepositoryReset();
void messageListRepositoryExit();
void messageListRepositorySetStandardMessageList(int messageListId, MessageList* messageList);
void messageListRepositorySetProtoMessageList(int messageListId, MessageList* messageList);
int messageListRepositoryAddExtra(int messageListId, const char* path);
char* messageListRepositoryGetMsg(int messageListId, int messageId);

uint32_t generate_mod_message_id(const char* list_id, const char* mod_name, const char* message_key);
bool messageListLoadWithMods(MessageList* msg, const char* path, const char* msg_type);
bool messageListAddEntry(MessageList* msg, int num, const char* text);

// Generate the base ID (start of a block) for a mod item within the given block type
uint32_t generate_mod_block_base_id(int listId, const char* mod_name, const char* block_key);

bool messageListLoad(MessageList* msg, const char* path);
bool messageListLoadWithBaseOffset(MessageList* msg, const char* path, int baseId);
bool messageListLoadCombined(MessageList* messageList, const char* path,
                             uint32_t baseIdFirst, uint32_t baseIdSecond, int threshold);

} // namespace fallout

#endif /* MESSAGE_H */
