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

static bool _message_find(MessageList* msg, int num, int* out_index);
static bool _message_add(MessageList* msg, MessageListItem* new_entry);
static bool _message_parse_number(int* out_num, const char* str);
static int _message_load_field(File* file, char* str);

static MessageList* messageListRepositoryLoad(const char* path);

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
//
// 0x5195A4
static char* _message_error_str = _Error_1;

// Temporary message list item text used during filtering badwords.
//
// 0x63207C
static char _bad_copy[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];

static MessageListRepositoryState* _messageListRepositoryState;

// Helper function for mod Holodisk conversion
bool messageListAddEntry(MessageList* msg, int num, const char* text)
{
    MessageListItem entry;
    entry.num = num;
    entry.text = internal_strdup(text);
    entry.audio = internal_strdup("");
    entry.flags = 0;
    
    bool result = _message_add(msg, &entry);
    
    if (!result) {
        internal_free(entry.text);
        internal_free(entry.audio);
    }
    
    return result;
}

// Stable hash function for generating consistent message IDs for mod content
// Uses case-insensitive DJB2 hash to ensure consistent IDs across different systems
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

// Generate stable message ID for mod content in the 0x8000-0xFFFF range
// Uses mod_name + message_key to create unique, reproducible IDs
uint32_t generate_mod_message_id(const char* mod_name, const char* message_key)
{
    char composite_key[256];
    snprintf(composite_key, sizeof(composite_key), "%s:%s", mod_name, message_key);
    uint32_t hash = stable_hash(composite_key);
    return 0x8000 + (hash % 0x7FFF); // 0x8000-0xFFFF range
}

// Generate a targeted message report for specific message list types
void generateMessageReport(MessageList* messageList, const char* msg_type)
{
    if (!messageList || !msg_type) return;

    char reportPath[COMPAT_MAX_PATH];
    snprintf(reportPath, sizeof(reportPath), "%sdata%clists%cmessages_%s_list.txt", _cd_path_base, DIR_SEPARATOR, DIR_SEPARATOR, msg_type);

    FILE* reportFile = compat_fopen(reportPath, "wt");
    if (!reportFile) {
        return;
    }

    // Write header
    fprintf(reportFile,
        "==============================================================================\n"
        "Fallout Fission - %s Messages\n"
        "==============================================================================\n"
        "Generated IDs for mod message references in scripts.\n\n"
        "Message ID Range: 32768-65535 (stable hash-based)\n"
        "Usage: display_msg(ID);  // Reference in scripts\n"
        "==============================================================================\n\n",
        msg_type);

    // Write timestamp
    time_t now = time(0);
    struct tm* t = localtime(&now);
    fprintf(reportFile, "Report Generated: %04d-%02d-%02d %02d:%02d:%02d\n\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec);

    // Count and list only mod messages (32768-65535 range)
    int modMessageCount = 0;

    fprintf(reportFile, "MOD MESSAGES (Custom Content):\n");
    fprintf(reportFile, "ID      | Text Preview\n");
    fprintf(reportFile, "--------|--------------------------------------------------\n");

    for (int i = 0; i < messageList->entries_num; i++) {
        MessageListItem* item = &messageList->entries[i];

        // Only show mod messages in our range (32768-65535)
        if (item->num >= 32768 && item->num <= 65535) {
            modMessageCount++;

            // Create shortened preview (first 60 chars)
            char preview[61];
            strncpy(preview, item->text, 60);
            preview[60] = '\0';
            if (strlen(item->text) > 60) {
                strcpy(preview + 57, "...");
            }

            fprintf(reportFile, "%-7d | %s\n", item->num, preview);
        }
    }

    // Add summary
    fprintf(reportFile, "\nSUMMARY:\n");
    fprintf(reportFile, "Total Mod Messages: %d\n", modMessageCount);
    fprintf(reportFile, "Base Messages: %d\n", messageList->entries_num - modMessageCount);

    // Modder guidance
    fprintf(reportFile, "\nMODDER GUIDANCE:\n");
    fprintf(reportFile, "1. Use the decimal IDs above in your scripts with display_msg()\n");
    fprintf(reportFile, "2. IDs are stable - same mod+key always generates same ID\n");
    fprintf(reportFile, "3. File location: data/text/<language>/game/messages_YourMod.txt\n");
    fprintf(reportFile, "4. Format: unique_key = Your message text\n\n");

    fclose(reportFile);
}

// Load mod messages from a file with section headers
static void loadModFileWithSections(MessageList* messageList, const char* fullPath, const char* filename, const char* target_section)
{
    File* stream = fileOpen(fullPath, "rt");
    if (!stream) {
        return;
    }

    // Extract mod name from filename (messages_xxx.txt -> xxx)
    char mod_name[64] = { 0 };
    const char* prefix = "messages_";
    const char* suffix = ".txt";

    if (strncmp(filename, prefix, strlen(prefix)) == 0) {
        size_t filename_len = strlen(filename);
        size_t mod_name_len = filename_len - strlen(prefix) - strlen(suffix);

        if (mod_name_len > 0 && mod_name_len < sizeof(mod_name)) {
            strncpy(mod_name, filename + strlen(prefix), mod_name_len);
            mod_name[mod_name_len] = '\0';
        }
    }

    char line[256];
    int messages_loaded = 0;
    char current_section[64] = "";
    bool in_correct_section = false;

    while (fileReadString(line, sizeof(line) - 1, stream)) {
        // Remove line endings
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        char* cr = strchr(line, '\r');
        if (cr) *cr = '\0';

        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // Check for section header [section_name]
        if (line[0] == '[') {
            char* line_end = line + strlen(line) - 1;

            // Trim trailing whitespace
            while (line_end > line && isspace(*line_end)) {
                *line_end = '\0';
                line_end--;
            }

            if (*line_end == ']') {
                // Extract section name (remove brackets)
                char section_name[64];
                strncpy(section_name, line + 1, line_end - line - 1);
                section_name[line_end - line - 1] = '\0';

                // Trim whitespace from section name
                char* start = section_name;
                while (*start && isspace(*start))
                    start++;
                char* end = start + strlen(start) - 1;
                while (end > start && isspace(*end))
                    *end-- = '\0';

                strncpy(current_section, start, sizeof(current_section) - 1);
                current_section[sizeof(current_section) - 1] = '\0';

                // Convert to lowercase for case-insensitive comparison
                char current_section_lower[64];
                char target_section_lower[64];

                strncpy(current_section_lower, current_section, sizeof(current_section_lower));
                strncpy(target_section_lower, target_section, sizeof(target_section_lower));

                for (char* p = current_section_lower; *p; ++p)
                    *p = tolower(*p);
                for (char* p = target_section_lower; *p; ++p)
                    *p = tolower(*p);

                in_correct_section = (strcmp(current_section_lower, target_section_lower) == 0);
                continue;
            }
        }

        // Only process key=value lines if we're in the correct section
        if (!in_correct_section) {
            continue;
        }

        char* separator = strchr(line, '=');
        if (!separator) {
            continue;
        }

        *separator = '\0';
        char* key = line;
        char* value = separator + 1;

        // Trim whitespace
        while (*key && isspace(*key))
            key++;
        while (*value && isspace(*value))
            value++;

        char* end = key + strlen(key) - 1;
        while (end > key && isspace(*end))
            *end-- = '\0';

        end = value + strlen(value) - 1;
        while (end > value && isspace(*end))
            *end-- = '\0';

        if (*key && *value) {
            uint32_t message_id = generate_mod_message_id(mod_name, key);

            MessageListItem item;
            item.num = message_id;
            item.text = internal_strdup(value);
            item.audio = internal_strdup("");
            item.flags = 0;

            if (_message_add(messageList, &item)) {
                messages_loaded++;
            } else {
                // Clean up on failure
                internal_free(item.text);
                internal_free(item.audio);
            }
        }
    }

    fileClose(stream);
}

// Load mod messages from messages_*.txt files with localization support
// This allows modders to add new text content without modifying base message files
// Load mod messages from messages_*.txt files for a specific message type
static void loadModMessagesForType(MessageList* messageList, const char* msg_type)
{
    char searchPattern[COMPAT_MAX_PATH];
    char fullPath[COMPAT_MAX_PATH];

    // Always load English mods first (as base/fallback)
    snprintf(searchPattern, sizeof(searchPattern), "data\\text\\%s\\game%cmessages_*.txt",
        ENGLISH, DIR_SEPARATOR);

    char** modFiles = nullptr;
    int modFileCount = fileNameListInit(searchPattern, &modFiles, 0, 0);

    for (int i = 0; i < modFileCount; i++) {
        snprintf(fullPath, sizeof(fullPath), "data\\text\\%s\\game%c%s",
            ENGLISH, DIR_SEPARATOR, modFiles[i]);
        loadModFileWithSections(messageList, fullPath, modFiles[i], msg_type);
    }

    if (modFileCount > 0) {
        fileNameListFree(&modFiles, 0);
    }

    // Then load current language (overrides English for available translations)
    if (compat_stricmp(settings.system.language.c_str(), ENGLISH) != 0) {
        snprintf(searchPattern, sizeof(searchPattern), "data\\text\\%s\\game%cmessages_*.txt",
            settings.system.language.c_str(), DIR_SEPARATOR);

        modFileCount = fileNameListInit(searchPattern, &modFiles, 0, 0);

        for (int i = 0; i < modFileCount; i++) {
            snprintf(fullPath, sizeof(fullPath), "data\\text\\%s\\game%c%s",
                settings.system.language.c_str(), DIR_SEPARATOR, modFiles[i]);
            loadModFileWithSections(messageList, fullPath, modFiles[i], msg_type);
        }

        if (modFileCount > 0) {
            fileNameListFree(&modFiles, 0);
        }
    }
}

// Enhanced message list loader that loads both base messages and mod messages
// This is the main entry point for the mod message system
// Always generates a report since we only use this for moddable message lists
// Enhanced message list loader that loads both base messages and mod messages
bool messageListLoadWithMods(MessageList* msg, const char* path, const char* msg_type)
{
    // First load the base messages
    if (!messageListLoad(msg, path)) {
        return false;
    }

    // Then load and append mod messages
    loadModMessagesForType(msg, msg_type);

    // Generate a report of loaded mod messages
    generateMessageReport(msg, msg_type);

    return true;
}

// 0x484770
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

// message_init
// 0x48494C
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
        rc = _message_load_field(file_ptr, num);
        if (rc != 0) {
            break;
        }

        if (_message_load_field(file_ptr, audio) != 0) {
            debugPrint("\nError loading audio field.\n", localized_path);
            goto err;
        }

        if (_message_load_field(file_ptr, text) != 0) {
            debugPrint("\nError loading text field.\n", localized_path);
            goto err;
        }

        if (!_message_parse_number(&(entry.num), num)) {
            debugPrint("\nError parsing number.\n", localized_path);
            goto err;
        }

        if (!_message_add(messageList, &entry)) {
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

    if (!_message_find(msg, entry->num, &index)) {
        return false;
    }

    ptr = &(msg->entries[index]);
    entry->flags = ptr->flags;
    entry->audio = ptr->audio;
    entry->text = ptr->text;

    return true;
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

// 0x484D10
static bool _message_find(MessageList* msg, int num, int* out_index)
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
static bool _message_add(MessageList* msg, MessageListItem* new_entry)
{
    int index;
    MessageListItem* entries;
    MessageListItem* existing_entry;

    if (_message_find(msg, new_entry->num, &index)) {
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
static bool _message_parse_number(int* out_num, const char* str)
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
static int _message_load_field(File* file, char* str)
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

// 0x48504C
char* getmsg(MessageList* msg, MessageListItem* entry, int num)
{
    entry->num = num;

    if (!messageListGetItem(msg, entry)) {
        entry->text = _message_error_str;
        debugPrint("\n ** String not found @ getmsg(), MESSAGE.C **\n");
    }

    return entry->text;
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

    bool enabled = false;
    configGetBool(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_GAME_DIALOG_GENDER_WORDS_KEY, &enabled);
    if (!enabled) {
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

                strncpy(start, src, length);
                strcpy(start + length, end + 1);
            } else {
                text = sep + 1;
            }
        }
    }
}

bool messageListRepositoryInit()
{
    _messageListRepositoryState = new (std::nothrow) MessageListRepositoryState();
    if (_messageListRepositoryState == nullptr) {
        return false;
    }

    char* fileList;
    configGetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_EXTRA_MESSAGE_LISTS_KEY, &fileList);
    if (fileList != nullptr && *fileList == '\0') {
        fileList = nullptr;
    }

    char path[COMPAT_MAX_PATH];
    int nextMessageListId = 0;
    while (fileList != nullptr) {
        char* pch = strchr(fileList, ',');
        if (pch != nullptr) {
            *pch = '\0';
        }

        char* sep = strchr(fileList, ':');
        if (sep != nullptr) {
            *sep = '\0';
            nextMessageListId = atoi(sep + 1);
        }

        snprintf(path, sizeof(path), "%s\\%s.msg", "game", fileList);

        if (sep != nullptr) {
            *sep = ':';
        }

        MessageList* messageList = messageListRepositoryLoad(path);
        if (messageList != nullptr) {
            _messageListRepositoryState->persistentMessageLists[kFirstPersistentMessageListId + nextMessageListId] = messageList;
        }

        if (pch != nullptr) {
            *pch = ',';
            fileList = pch + 1;
        } else {
            fileList = nullptr;
        }

        // Sfall's implementation is a little bit odd. |nextMessageListId| can
        // be set via "key:value" pair in the config, so if the first pair is
        // "msg:12287", then this check will think it's the end of the loop.
        // In order to maintain compatibility we'll use the same approach,
        // however it looks like the whole idea of auto-numbering extra message
        // lists is a bad one. To use these extra message lists we need to
        // specify their ids from user-space scripts. Without explicitly
        // specifying message list ids as "key:value" pairs a mere change of
        // order in the config will break such scripts in an unexpected way.
        nextMessageListId++;
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

} // namespace fallout
