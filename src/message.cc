#include "message.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <array>
#include <unordered_map>

#include "debug.h"
#include "memory.h"
#include "platform_compat.h"
#include "proto.h"
#include "proto_types.h"
#include "random.h"
#include "settings.h"
#include "sfall_config.h"
#include "string_parsers.h"
#include "window_manager.h"

namespace fallout {

#define BADWORD_LENGTH_MAX 80
#define DIR_SEPARATOR '/'

static constexpr int kFirstStandardMessageListId = 0;
static constexpr int kLastStandardMessageListId = kFirstStandardMessageListId + STANDARD_MESSAGE_LIST_COUNT - 1;

static constexpr int kFirstProtoMessageListId = 0x1000;
static constexpr int kLastProtoMessageListId = kFirstProtoMessageListId + PROTO_MESSAGE_LIST_COUNT - 1;

static constexpr int kFirstPersistentMessageListId = 0x2000;
static constexpr int kLastPersistentMessageListId = 0x2FFF;

static constexpr int kFirstTemporaryMessageListId = 0x3000;
static constexpr int kLastTemporaryMessageListId = 0x3FFF;

struct MessageListRepositoryState {
    std::array<MessageList*, STANDARD_MESSAGE_LIST_COUNT> standardMessageLists;
    std::array<MessageList*, PROTO_MESSAGE_LIST_COUNT> protoMessageLists;
    std::unordered_map<int, MessageList*> persistentMessageLists;
    std::unordered_map<int, MessageList*> temporaryMessageLists;
    int nextTemporaryMessageListId = kFirstTemporaryMessageListId;
};

struct ListInfo {
    const char* name;
    uint32_t base;
    uint32_t range; // size of mod ID space for this list
};

// Define the ID ranges and block sizes for each mod block type.
const ModBlockRange gModBlockRanges[] = {
    { MOD_BLOCK_MAP, 5000, 500000, 100 }, // map names (3 elevations per map)
    { MOD_BLOCK_AREA, 500001, 1000000, 30 }, // area names
    { MOD_BLOCK_PROTO, 5000, 1000000, 1000 }, // proto messages
    { MOD_BLOCK_PIPBOY, 100000, 1000000, 200 }, // Pip-Boy texts (holodisks: 200 lines each)
    { MOD_BLOCK_COMBAT, 30000, 1000000, 1000 }, // combat messages
    { MOD_BLOCK_COMBATAI, 100000, 1000000, 1000 }, // combat AI messages
    { MOD_BLOCK_QUESTS, 20000, 1000000, 100 }, // quest descriptions, 100 IDs per mod block
    { MOD_BLOCK_WORLDMAP, 20000, 1000000, 500 }, // entrance names per town and encounter messages
};

const int gModBlockRangesCount = sizeof(gModBlockRanges) / sizeof(gModBlockRanges[0]);

// Extends message loading to support mod message files (messages_*.txt)
// Mod message IDs use range 0x8000-0xFFFF (32768-65535).

static uint32_t stable_hash(const char* str);
void generateMessageReport(MessageList* messageList, const char* msg_type);

// 0x50B79C
static char _Error_1[] = "Error";

// 0x50B960
static const char* gBadwordsReplacements = "!@#$%&*@#*!&$%#&%#*%!$&%@*$@&";

// 0x519598
static char** gBadwords = nullptr;

// 0x51959C
static int gBadwordsCount = 0;

// 0x5195A0
static int* gBadwordsLengths = nullptr;

// Default text for getmsg when no entry is found.
static char* gMessageErrorStr = _Error_1;

// Temporary message list item text used during filtering badwords.
static char _bad_copy[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];

static MessageListRepositoryState* _messageListRepositoryState;

// ============================================================================
// Core Static Helper Functions
// ============================================================================

static bool _messageListFind(MessageList* msg, int num, int* out_index)
{
    int r, l, mid;
    int cmp;

    if (msg->entries_num == 0) {
        *out_index = 0;
        return false;
    }

    r = msg->entries_num - 1;
    l = 0;

    do {
        mid = (l + r) / 2;
        cmp = num - msg->entries[mid].num;
        if (cmp == 0) {
            *out_index = mid;
            return true;
        }

        if (cmp > 0) {
            l = l + 1;
        } else {
            r = r - 1;
        }
    } while (r >= l);

    if (cmp < 0) {
        *out_index = mid;
    } else {
        *out_index = mid + 1;
    }

    return false;
}

// 0x484D68
static bool _messageListAdd(MessageList* msg, MessageListItem* new_entry)
{
    int index;
    MessageListItem* entries;
    MessageListItem* existing_entry;

    if (_messageListFind(msg, new_entry->num, &index)) {
        existing_entry = &(msg->entries[index]);

        if (existing_entry->audio != nullptr) {
            internal_free(existing_entry->audio);
        }

        if (existing_entry->text != nullptr) {
            internal_free(existing_entry->text);
        }
    } else {
        if (msg->entries != nullptr) {
            entries = (MessageListItem*)internal_realloc(msg->entries, sizeof(MessageListItem) * (msg->entries_num + 1));
            if (entries == nullptr) {
                return false;
            }

            msg->entries = entries;

            if (index != msg->entries_num) {
                // Move all items below insertion point
                memmove(&(msg->entries[index + 1]), &(msg->entries[index]), sizeof(MessageListItem) * (msg->entries_num - index));
            }
        } else {
            msg->entries = (MessageListItem*)internal_malloc(sizeof(MessageListItem));
            if (msg->entries == nullptr) {
                return false;
            }
            msg->entries_num = 0;
            index = 0;
        }

        existing_entry = &(msg->entries[index]);
        existing_entry->flags = 0;
        existing_entry->audio = nullptr;
        existing_entry->text = nullptr;
        msg->entries_num++;
    }

    existing_entry->audio = internal_strdup(new_entry->audio);
    if (existing_entry->audio == nullptr) {
        return false;
    }

    existing_entry->text = internal_strdup(new_entry->text);
    if (existing_entry->text == nullptr) {
        return false;
    }

    existing_entry->num = new_entry->num;

    return true;
}

// 0x484F60
static bool _messageListParseNumber(int* out_num, const char* str)
{
    const char* ch;
    bool success;

    ch = str;
    if (*ch == '\0') {
        return false;
    }

    success = true;
    if (*ch == '+' || *ch == '-') {
        ch++;
    }

    while (*ch != '\0') {
        if (!isdigit(*ch)) {
            success = false;
            break;
        }
        ch++;
    }

    *out_num = atoi(str);
    return success;
}

// Read next message file field, the `str` should be at least
// `MESSAGE_LIST_ITEM_FIELD_MAX_SIZE` bytes long.
//
// Returns:
// 0 - ok
// 1 - eof
// 2 - mismatched delimeters
// 3 - unterminated field
// 4 - limit exceeded (> `MESSAGE_LIST_ITEM_FIELD_MAX_SIZE`)
//
// 0x484FB4
static int _messageListLoadField(File* file, char* str)
{
    int ch;
    int len;

    len = 0;

    while (1) {
        ch = fileReadChar(file);
        if (ch == -1) {
            return 1;
        }

        if (ch == '}') {
            debugPrint("\nError reading message file - mismatched delimiters.\n");
            return 2;
        }

        if (ch == '{') {
            break;
        }
    }

    while (1) {
        ch = fileReadChar(file);

        if (ch == -1) {
            debugPrint("\nError reading message file - EOF reached.\n");
            return 3;
        }

        if (ch == '}') {
            *(str + len) = '\0';
            return 0;
        }

        if (ch != '\n') {
            *(str + len) = ch;
            len++;

            if (len >= MESSAGE_LIST_ITEM_FIELD_MAX_SIZE) {
                debugPrint("\nError reading message file - text exceeds limit.\n");
                return 4;
            }
        }
    }

    return 0;
}

// ============================================================================
// Core Message List API
// ============================================================================

// message_init
bool messageListInit(MessageList* messageList)
{
    if (messageList != nullptr) {
        messageList->entries_num = 0;
        messageList->entries = nullptr;
    }
    return true;
}

// 0x484964
bool messageListFree(MessageList* messageList)
{
    int i;
    MessageListItem* entry;

    if (messageList == nullptr) {
        return false;
    }

    for (i = 0; i < messageList->entries_num; i++) {
        entry = &(messageList->entries[i]);

        if (entry->audio != nullptr) {
            internal_free(entry->audio);
        }

        if (entry->text != nullptr) {
            internal_free(entry->text);
        }
    }

    messageList->entries_num = 0;

    if (messageList->entries != nullptr) {
        internal_free(messageList->entries);
        messageList->entries = nullptr;
    }

    return true;
}

// message_load
// 0x484AA4
bool messageListLoad(MessageList* messageList, const char* path)
{
    char localized_path[COMPAT_MAX_PATH];
    File* file_ptr;
    char num[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];
    char audio[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];
    char text[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];
    int rc;
    bool success;
    MessageListItem entry;

    success = false;

    if (messageList == nullptr) {
        return false;
    }

    if (path == nullptr) {
        return false;
    }

    snprintf(localized_path, sizeof(localized_path), "%s\\%s\\%s", "text", settings.system.language.c_str(), path);

    file_ptr = fileOpen(localized_path, "rt");

    // SFALL: Fallback to english if requested localization does not exist.
    if (file_ptr == nullptr) {
        if (compat_stricmp(settings.system.language.c_str(), ENGLISH) != 0) {
            snprintf(localized_path, sizeof(localized_path), "%s\\%s\\%s", "text", ENGLISH, path);
            file_ptr = fileOpen(localized_path, "rt");
        }
    }

    if (file_ptr == nullptr) {
        return false;
    }

    entry.num = 0;
    entry.audio = audio;
    entry.text = text;

    while (1) {
        rc = _messageListLoadField(file_ptr, num);
        if (rc != 0) {
            break;
        }

        if (_messageListLoadField(file_ptr, audio) != 0) {
            debugPrint("\nError loading audio field.\n", localized_path);
            goto err;
        }

        if (_messageListLoadField(file_ptr, text) != 0) {
            debugPrint("\nError loading text field.\n", localized_path);
            goto err;
        }

        if (!_messageListParseNumber(&(entry.num), num)) {
            debugPrint("\nError parsing number.\n", localized_path);
            goto err;
        }

        if (!_messageListAdd(messageList, &entry)) {
            debugPrint("\nError adding message.\n", localized_path);
            goto err;
        }
    }

    if (rc == 1) {
        success = true;
    }

err:

    if (!success) {
        debugPrint("Error loading message file %s at offset %x.", localized_path, fileTell(file_ptr));
    }

    fileClose(file_ptr);

    return success;
}

// Load a message file where entry numbers are offsets from a base ID.
// Example: if baseId = 1000 and file contains {0}{audio}{text}, the entry is added with ID = 1000 + 0.
bool messageListLoadWithBaseOffset(MessageList* messageList, const char* path, int baseId)
{
    char localized_path[COMPAT_MAX_PATH];
    File* file_ptr;
    char num[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];
    char audio[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];
    char text[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];
    int rc;
    bool success;
    MessageListItem entry;

    success = false;

    if (messageList == nullptr) {
        return false;
    }

    if (path == nullptr) {
        return false;
    }

    snprintf(localized_path, sizeof(localized_path), "%s\\%s\\%s", "text", settings.system.language.c_str(), path);

    file_ptr = fileOpen(localized_path, "rt");

    // Fallback to english if requested localization does not exist.
    if (file_ptr == nullptr) {
        if (compat_stricmp(settings.system.language.c_str(), ENGLISH) != 0) {
            snprintf(localized_path, sizeof(localized_path), "%s\\%s\\%s", "text", ENGLISH, path);
            file_ptr = fileOpen(localized_path, "rt");
        }
    }

    if (file_ptr == nullptr) {
        return false;
    }

    entry.num = 0;
    entry.audio = audio;
    entry.text = text;

    while (1) {
        rc = _messageListLoadField(file_ptr, num);
        if (rc != 0) {
            break;
        }

        if (_messageListLoadField(file_ptr, audio) != 0) {
            debugPrint("\nError loading audio field.\n", localized_path);
            goto err;
        }

        if (_messageListLoadField(file_ptr, text) != 0) {
            debugPrint("\nError loading text field.\n", localized_path);
            goto err;
        }

        int offset;
        if (!_messageListParseNumber(&offset, num)) {
            debugPrint("\nError parsing number.\n", localized_path);
            goto err;
        }

        entry.num = baseId + offset;

        if (!_messageListAdd(messageList, &entry)) {
            debugPrint("\nError adding message.\n", localized_path);
            goto err;
        }
    }

    if (rc == 1) {
        success = true;
    }

err:

    if (!success) {
        debugPrint("Error loading message file %s at offset %x.", localized_path, fileTell(file_ptr));
    }

    fileClose(file_ptr);

    return success;
}

// 0x484C30
bool messageListGetItem(MessageList* msg, MessageListItem* entry)
{
    int index;
    MessageListItem* ptr;

    if (msg == nullptr) {
        return false;
    }

    if (entry == nullptr) {
        return false;
    }

    if (msg->entries_num == 0) {
        return false;
    }

    if (!_messageListFind(msg, entry->num, &index)) {
        return false;
    }

    ptr = &(msg->entries[index]);
    entry->flags = ptr->flags;
    entry->audio = ptr->audio;
    entry->text = ptr->text;

    return true;
}

// 0x48504C
char* getmsg(MessageList* msg, MessageListItem* entry, int num)
{
    entry->num = num;

    if (!messageListGetItem(msg, entry)) {
        entry->text = gMessageErrorStr;
        debugPrint("\n ** String not found @ getmsg(), MESSAGE.C **\n");
    }

    return entry->text;
}

// Helper function for mod Holodisk conversion
bool messageListAddEntry(MessageList* msg, int num, const char* text)
{
    MessageListItem entry;
    entry.num = num;
    entry.text = internal_strdup(text);
    entry.audio = internal_strdup("");
    entry.flags = 0;

    bool result = _messageListAdd(msg, &entry);

    if (!result) {
        internal_free(entry.text);
        internal_free(entry.audio);
    }

    return result;
}

// ============================================================================
// Badword Filtering System
// ============================================================================

int badwordsInit()
{
    File* stream = fileOpen("data\\badwords.txt", "rt");
    if (stream == nullptr) {
        return -1;
    }

    char word[BADWORD_LENGTH_MAX];

    gBadwordsCount = 0;
    while (fileReadString(word, BADWORD_LENGTH_MAX - 1, stream)) {
        gBadwordsCount++;
    }

    gBadwords = (char**)internal_malloc(sizeof(*gBadwords) * gBadwordsCount);
    if (gBadwords == nullptr) {
        fileClose(stream);
        return -1;
    }

    gBadwordsLengths = (int*)internal_malloc(sizeof(*gBadwordsLengths) * gBadwordsCount);
    if (gBadwordsLengths == nullptr) {
        internal_free(gBadwords);
        fileClose(stream);
        return -1;
    }

    fileSeek(stream, 0, SEEK_SET);

    int index = 0;
    for (; index < gBadwordsCount; index++) {
        if (!fileReadString(word, BADWORD_LENGTH_MAX - 1, stream)) {
            break;
        }

        size_t len = strlen(word);
        if (word[len - 1] == '\n') {
            len--;
            word[len] = '\0';
        }

        gBadwords[index] = internal_strdup(word);
        if (gBadwords[index] == nullptr) {
            break;
        }

        compat_strupr(gBadwords[index]);

        gBadwordsLengths[index] = len;
    }

    fileClose(stream);

    if (index != gBadwordsCount) {
        for (; index > 0; index--) {
            internal_free(gBadwords[index - 1]);
        }

        internal_free(gBadwords);
        internal_free(gBadwordsLengths);

        return -1;
    }

    return 0;
}

// 0x4848F0
void badwordsExit()
{
    for (int index = 0; index < gBadwordsCount; index++) {
        internal_free(gBadwords[index]);
    }

    if (gBadwordsCount != 0) {
        internal_free(gBadwords);
        internal_free(gBadwordsLengths);
    }

    gBadwordsCount = 0;
}

// 0x485078
bool messageListFilterBadwords(MessageList* messageList)
{
    if (messageList == nullptr) {
        return false;
    }

    if (messageList->entries_num == 0) {
        return true;
    }

    if (gBadwordsCount == 0) {
        return true;
    }

    if (!settings.preferences.language_filter) {
        return true;
    }

    int replacementsCount = strlen(gBadwordsReplacements);
    int replacementsIndex = randomBetween(1, replacementsCount) - 1;

    for (int index = 0; index < messageList->entries_num; index++) {
        MessageListItem* item = &(messageList->entries[index]);
        strcpy(_bad_copy, item->text);
        compat_strupr(_bad_copy);

        for (int badwordIndex = 0; badwordIndex < gBadwordsCount; badwordIndex++) {
            // I don't quite understand the loop below. It has no stop
            // condition besides no matching substring. It also overwrites
            // already masked words on every iteration.
            for (char* p = _bad_copy;; p++) {
                const char* substr = strstr(p, gBadwords[badwordIndex]);
                if (substr == nullptr) {
                    break;
                }

                if (substr == _bad_copy || (!isalpha(substr[-1]) && !isalpha(substr[gBadwordsLengths[badwordIndex]]))) {
                    item->flags |= MESSAGE_LIST_ITEM_TEXT_FILTERED;
                    char* ptr = item->text + (substr - _bad_copy);

                    for (int j = 0; j < gBadwordsLengths[badwordIndex]; j++) {
                        *ptr++ = gBadwordsReplacements[replacementsIndex++];
                        if (replacementsIndex == replacementsCount) {
                            replacementsIndex = 0;
                        }
                    }
                }
            }
        }
    }

    return true;
}

void messageListFilterGenderWords(MessageList* messageList, int gender)
{
    if (messageList == nullptr) {
        return;
    }

    // modConfig: bool for gender words
    if (!settings.mod_settings.game_dialog_gender_words) {
        return;
    }

    for (int index = 0; index < messageList->entries_num; index++) {
        MessageListItem* item = &(messageList->entries[index]);
        char* text = item->text;
        char* sep;

        while ((sep = strchr(text, '^')) != nullptr) {
            *sep = '\0';
            char* start = strrchr(text, '<');
            char* end = strchr(sep + 1, '>');
            *sep = '^';

            if (start != nullptr && end != nullptr) {
                char* src;
                size_t length;
                if (gender == GENDER_FEMALE) {
                    src = sep + 1;
                    length = end - sep - 1;
                } else {
                    src = start + 1;
                    length = sep - start - 1;
                }

                memmove(start, src, length);
                memmove(start + length, end + 1, strlen(end + 1) + 1);
            } else {
                text = sep + 1;
            }
        }
    }
}

/**
 * Case-insensitive DJB2 hash function for generating stable message IDs.
 * Ensures consistent IDs across different systems and runs.
 *
 * @param str Input string to hash
 * @return 32-bit hash value
 */
static uint32_t stable_hash(const char* str)
{
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        // Convert to lowercase for case-insensitive hashing
        c = tolower(c);
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

uint32_t generate_mod_block_base_id(int listId, const char* mod_name, const char* block_key)
{
    // Find the range for the requested listId
    const ModBlockRange* range = nullptr;
    for (int i = 0; i < gModBlockRangesCount; i++) {
        if (gModBlockRanges[i].listId == listId) {
            range = &gModBlockRanges[i];
            break;
        }
    }
    if (range == nullptr) {
        debugPrint("ERROR: No mod block range defined for listId %d\n", listId);
        return 0;
    }

    // Compute how many blocks fit in the range
    uint32_t range_size = range->maxId - range->startId + 1;
    uint32_t block_count = range_size / range->blockSize;
    if (block_count == 0) {
        debugPrint("ERROR: Block size %d larger than range %d-%d\n",
            range->blockSize, range->startId, range->maxId);
        return 0;
    }

    // Create composite key from mod name and block key
    char composite_key[256];
    snprintf(composite_key, sizeof(composite_key), "%s:%s", mod_name, block_key);

    // Get a stable hash (case-insensitive DJB2)
    uint32_t hash = stable_hash(composite_key);

    // Map hash to a block index
    uint32_t block_index = hash % block_count;

    // Return the start ID of that block
    return range->startId + (block_index * range->blockSize);
}

// ============================================================================
// Message List Repository (depends on core functions)
// ============================================================================

static MessageList* messageListRepositoryLoad(const char* path)
{
    MessageList* messageList = new (std::nothrow) MessageList();
    if (messageList == nullptr) {
        return nullptr;
    }

    if (!messageListInit(messageList)) {
        delete messageList;
        return nullptr;
    }

    if (!messageListLoad(messageList, path)) {
        delete messageList;
        return nullptr;
    }

    return messageList;
}

bool messageListRepositoryInit()
{
    _messageListRepositoryState = new (std::nothrow) MessageListRepositoryState();
    if (_messageListRepositoryState == nullptr) {
        return false;
    }

    const std::string& extraMsgLists = settings.mod_settings.extra_message_lists;
    if (extraMsgLists.empty()) {
        return true; // Nothing to load, but success.
    }

    std::vector<std::string> tokens = splitString(extraMsgLists);

    char path[COMPAT_MAX_PATH];
    int nextMessageListId = 0;
    for (const std::string& token : tokens) {
        std::string filePart = token;
        int listId = nextMessageListId; // Default to auto-number

        // Check for colon separator
        size_t colonPos = token.find(':');
        if (colonPos != std::string::npos) {
            filePart = token.substr(0, colonPos);
            listId = std::atoi(token.substr(colonPos + 1).c_str());
        }

        snprintf(path, sizeof(path), "%s\\%s.msg", "game", filePart.c_str());

        MessageList* messageList = messageListRepositoryLoad(path);
        if (messageList != nullptr) {
            _messageListRepositoryState->persistentMessageLists[kFirstPersistentMessageListId + listId] = messageList;
        }

        nextMessageListId = listId + 1; // Original increments after each token
        if (nextMessageListId == kLastPersistentMessageListId - kFirstPersistentMessageListId + 1) {
            break;
        }
    }

    return true;
}

void messageListRepositoryReset()
{
    for (auto& pair : _messageListRepositoryState->temporaryMessageLists) {
        messageListFree(pair.second);
        delete pair.second;
    }
    _messageListRepositoryState->temporaryMessageLists.clear();
    _messageListRepositoryState->nextTemporaryMessageListId = kFirstTemporaryMessageListId;
}

void messageListRepositoryExit()
{
    if (_messageListRepositoryState != nullptr) {
        for (auto& pair : _messageListRepositoryState->temporaryMessageLists) {
            messageListFree(pair.second);
            delete pair.second;
        }

        for (auto& pair : _messageListRepositoryState->persistentMessageLists) {
            messageListFree(pair.second);
            delete pair.second;
        }

        delete _messageListRepositoryState;
        _messageListRepositoryState = nullptr;
    }
}

void messageListRepositorySetStandardMessageList(int standardMessageList, MessageList* messageList)
{
    _messageListRepositoryState->standardMessageLists[standardMessageList] = messageList;
}

void messageListRepositorySetProtoMessageList(int protoMessageList, MessageList* messageList)
{
    _messageListRepositoryState->protoMessageLists[protoMessageList] = messageList;
}

int messageListRepositoryAddExtra(int messageListId, const char* path)
{
    if (messageListId != 0) {
        // CE: Probably there is a bug in Sfall, when |messageListId| is
        // non-zero, it is enforced to be within persistent id range. That is
        // the scripting engine is allowed to add persistent message lists.
        // Everything added/changed by scripting engine should be temporary by
        // design.
        if (messageListId < kFirstPersistentMessageListId || messageListId > kLastPersistentMessageListId) {
            return -1;
        }

        // CE: Sfall stores both persistent and temporary message lists in
        // one map, however since we've passed check above, we should only
        // check in persistent message lists.
        if (_messageListRepositoryState->persistentMessageLists.find(messageListId) != _messageListRepositoryState->persistentMessageLists.end()) {
            return 0;
        }
    } else {
        if (_messageListRepositoryState->nextTemporaryMessageListId > kLastTemporaryMessageListId) {
            return -3;
        }
    }

    MessageList* messageList = messageListRepositoryLoad(path);
    if (messageList == nullptr) {
        return -2;
    }

    if (messageListId == 0) {
        messageListId = _messageListRepositoryState->nextTemporaryMessageListId++;
    }

    _messageListRepositoryState->temporaryMessageLists[messageListId] = messageList;

    return messageListId;
}

char* messageListRepositoryGetMsg(int messageListId, int messageId)
{
    MessageList* messageList = nullptr;

    if (messageListId >= kFirstStandardMessageListId && messageListId <= kLastStandardMessageListId) {
        messageList = _messageListRepositoryState->standardMessageLists[messageListId - kFirstStandardMessageListId];
    } else if (messageListId >= kFirstProtoMessageListId && messageListId <= kLastProtoMessageListId) {
        messageList = _messageListRepositoryState->protoMessageLists[messageListId - kFirstProtoMessageListId];
    } else if (messageListId >= kFirstPersistentMessageListId && messageListId <= kLastPersistentMessageListId) {
        auto it = _messageListRepositoryState->persistentMessageLists.find(messageListId);
        if (it != _messageListRepositoryState->persistentMessageLists.end()) {
            messageList = it->second;
        }
    } else if (messageListId >= kFirstTemporaryMessageListId && messageListId <= kLastTemporaryMessageListId) {
        auto it = _messageListRepositoryState->temporaryMessageLists.find(messageListId);
        if (it != _messageListRepositoryState->temporaryMessageLists.end()) {
            messageList = it->second;
        }
    }

    MessageListItem messageListItem;
    return getmsg(messageList, &messageListItem, messageId);
}

// Builds language-aware path in "text" subfolder.
//
// 0x484CB8
bool _message_make_path(char* dest, size_t size, const char* path)
{
    if (dest == nullptr) {
        return false;
    }

    if (path == nullptr) {
        return false;
    }

    snprintf(dest, size, "%s\\%s\\%s", "text", settings.system.language.c_str(), path);

    return true;
}

// Load a message file where entries are added with one of two base IDs based on offset threshold.
// Offsets below threshold use baseIdFirst; offsets >= threshold use baseIdSecond (with threshold subtracted).
bool messageListLoadCombined(MessageList* messageList, const char* path,
    uint32_t baseIdFirst, uint32_t baseIdSecond, int threshold)
{
    char localized_path[COMPAT_MAX_PATH];
    File* file_ptr;
    char num[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];
    char audio[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];
    char text[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];
    int rc;
    bool success;
    MessageListItem entry;

    success = false;

    if (messageList == nullptr) return false;
    if (path == nullptr) return false;

    snprintf(localized_path, sizeof(localized_path), "%s\\%s\\%s", "text", settings.system.language.c_str(), path);
    file_ptr = fileOpen(localized_path, "rt");
    if (file_ptr == nullptr) {
        if (compat_stricmp(settings.system.language.c_str(), ENGLISH) != 0) {
            snprintf(localized_path, sizeof(localized_path), "%s\\%s\\%s", "text", ENGLISH, path);
            file_ptr = fileOpen(localized_path, "rt");
        }
    }
    if (file_ptr == nullptr) return false;

    entry.audio = audio;
    entry.text = text;

    while (1) {
        rc = _messageListLoadField(file_ptr, num);
        if (rc != 0) break;

        if (_messageListLoadField(file_ptr, audio) != 0) goto err;
        if (_messageListLoadField(file_ptr, text) != 0) goto err;

        int offset;
        if (!_messageListParseNumber(&offset, num)) goto err;

        if (offset < threshold) {
            entry.num = baseIdFirst + offset;
        } else {
            entry.num = baseIdSecond + (offset - threshold);
        }

        if (!_messageListAdd(messageList, &entry)) goto err;
    }

    if (rc == 1) success = true;

err:
    if (!success) {
        debugPrint("Error loading combined message file %s at offset %x.", localized_path, fileTell(file_ptr));
    }
    fileClose(file_ptr);
    return success;
}

} // namespace fallout
