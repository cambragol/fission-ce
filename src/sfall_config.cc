#include "sfall_config.h"

#include "art.h"
#include "db.h"
#include "debug.h"
#include "file_find.h"
#include "game_version.h"
#include "memory.h"
#include "platform_compat.h"
#include "proto.h"
#include "scan_unimplemented.h"
#include "settings.h"
#include "string_parsers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace fallout {

#define DIR_SEPARATOR '/'

bool gModConfigInitialized = false;
Config gModConfig;
ModInfo gLoadedMods[MAX_LOADED_MODS];
int gLoadedModsCount = 0;

// Runtime defaults for every key modConfigInit writes into gModConfig.
// This is the single operational source of truth: the compiled-in
// MOD_CONFIG_DEFAULT_* constants are used only to seed the base block
// below, and per-game overrides live inside modDefaultsGet(). Everything
// that reads a default reads it from the struct this function returns.
//
// Adding a new default: add the field here, set it in the base block,
// and add the corresponding configSetX call in modConfigInit. No new
// #define needed.
typedef struct ModDefaults {
    // Start date
    int start_year;
    int start_month;
    int start_day;
    const char* starting_map;

    // Movie timers
    int movie_timer_artimer1;
    int movie_timer_artimer2;
    int movie_timer_artimer3;
    int movie_timer_artimer4;

    // Pip-Boy
    int pipboy_available_at_gamestart;

    // Criticals
    int override_criticals_mode;
    const char* override_criticals_file;

    // Premades
    const char* premade_characters_file_names;
    const char* premade_characters_face_fids;

    // Player models
    const char* dude_native_look_jumpsuit_male;
    const char* dude_native_look_jumpsuit_female;
    const char* dude_native_look_tribal_male;
    const char* dude_native_look_tribal_female;

    // Main menu
    int main_menu_big_font_color;
    int main_menu_credits_offset_x;
    int main_menu_credits_offset_y;
    int main_menu_font_color;
    int main_menu_offset_x;
    int main_menu_offset_y;

    const char* version_string;
    int worldmap_trail_markers;

    // Karma
    const char* karma_frms;
    const char* karma_points;

    // Explosives
    int dynamite_min_damage;
    int dynamite_max_damage;
    int plastic_explosive_min_damage;
    int plastic_explosive_max_damage;

    // Damage / skills
    const char* city_reputation_list;
    const char* unarmed_file;
    int damage_mod_formula;
    bool bonus_hth_damage_fix;
    int use_lockpick_frm;
    int use_steal_frm;
    int use_traps_frm;
    int use_first_aid_frm;
    int use_doctor_frm;
    int use_science_frm;
    int use_repair_frm;
    bool science_repair_target_type;
    bool game_dialog_fix;
    const char* tweaks_file;
    bool game_dialog_gender_words;
    bool town_map_hotkeys_fix;
    const char* extra_message_lists;
    const char* patch_file;
    const char* books_file;
    const char* elevators_file;
    const char* console_output_file;
    int use_walk_distance;

    // Burst
    bool burst_mod_enabled;
    int burst_mod_center_multiplier;
    int burst_mod_center_divisor;
    int burst_mod_target_multiplier;
    int burst_mod_target_divisor;

    // Interface bar
    bool iface_bar_mode;
    int iface_bar_width;
    int iface_bar_side_art;
    bool iface_bar_sides_ori;

    // Vock floats
    int float_audio_channels;
    int float_distance_per_perception;
    int float_obstruction_dampening;
    int float_eviction_policy;
    bool float_text_scramble;
    int float_text_scramble_distance_per_perception;
    const char* float_text_scramble_chars;
    bool voiced_floats;
    bool float_censor_bleep;
    int float_volume;

    // Scripts
    const char* ini_config_folder;
    const char* global_script_paths;
} ModDefaults;

// Returns the complete set of runtime defaults for the currently running
// game. IS_FALLOUT_1() is authoritative here because gameDbInit() has
// already opened master.dat (which sets the version) before
// modConfigInit() is called.
static ModDefaults modDefaultsGet()
{
    ModDefaults d = {};

    // Shared base block
    // Every FISSION game starts from these values. Edit here to change a
    // default for every game at once. Values are seeded from the
    // MOD_CONFIG_DEFAULT_* constants so that behavior is unchanged from
    // before this refactor.
    //
    // NOTE: start_month and start_day are ZERO-BASED. Month 6 = July,
    // day 24 = the 25th. This trips up every override; check both when
    // adding a new game.
    d.start_year = MOD_CONFIG_DEFAULT_START_YEAR;
    d.start_month = MOD_CONFIG_DEFAULT_START_MONTH;
    d.start_day = MOD_CONFIG_DEFAULT_START_DAY;
    d.starting_map = MOD_CONFIG_DEFAULT_STARTING_MAP;

    d.movie_timer_artimer1 = MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER1;
    d.movie_timer_artimer2 = MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER2;
    d.movie_timer_artimer3 = MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER3;
    d.movie_timer_artimer4 = MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER4;

    d.pipboy_available_at_gamestart = MOD_CONFIG_DEFAULT_PIPBOY_AVAILABLE_AT_GAMESTART;

    d.override_criticals_mode = MOD_CONFIG_DEFAULT_OVERRIDE_CRITICALS_MODE;
    d.override_criticals_file = MOD_CONFIG_DEFAULT_OVERRIDE_CRITICALS_FILE;

    d.premade_characters_file_names = MOD_CONFIG_DEFAULT_PREMADE_CHARACTERS_FILE_NAMES;
    d.premade_characters_face_fids = MOD_CONFIG_DEFAULT_PREMADE_CHARACTERS_FACE_FIDS;

    d.dude_native_look_jumpsuit_male = MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_JUMPSUIT_MALE;
    d.dude_native_look_jumpsuit_female = MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE;
    d.dude_native_look_tribal_male = MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_TRIBAL_MALE;
    d.dude_native_look_tribal_female = MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_TRIBAL_FEMALE;

    d.main_menu_big_font_color = MOD_CONFIG_DEFAULT_MAIN_MENU_BIG_FONT_COLOR;
    d.main_menu_credits_offset_x = MOD_CONFIG_DEFAULT_MAIN_MENU_CREDITS_OFFSET_X;
    d.main_menu_credits_offset_y = MOD_CONFIG_DEFAULT_MAIN_MENU_CREDITS_OFFSET_Y;
    d.main_menu_font_color = MOD_CONFIG_DEFAULT_MAIN_MENU_FONT_COLOR;
    d.main_menu_offset_x = MOD_CONFIG_DEFAULT_MAIN_MENU_OFFSET_X;
    d.main_menu_offset_y = MOD_CONFIG_DEFAULT_MAIN_MENU_OFFSET_Y;

    d.version_string = MOD_CONFIG_DEFAULT_VERSION_STRING;
    d.worldmap_trail_markers = MOD_CONFIG_DEFAULT_WORLDMAP_TRAIL_MARKERS;

    d.karma_frms = MOD_CONFIG_DEFAULT_KARMA_FRMS;
    d.karma_points = MOD_CONFIG_DEFAULT_KARMA_POINTS;

    d.dynamite_min_damage = MOD_CONFIG_DEFAULT_DYNAMITE_MIN_DAMAGE;
    d.dynamite_max_damage = MOD_CONFIG_DEFAULT_DYNAMITE_MAX_DAMAGE;
    d.plastic_explosive_min_damage = MOD_CONFIG_DEFAULT_PLASTIC_EXPLOSIVE_MIN_DAMAGE;
    d.plastic_explosive_max_damage = MOD_CONFIG_DEFAULT_PLASTIC_EXPLOSIVE_MAX_DAMAGE;

    d.city_reputation_list = MOD_CONFIG_DEFAULT_CITY_REPUTATION_LIST;
    d.unarmed_file = MOD_CONFIG_DEFAULT_UNARMED_FILE;
    d.damage_mod_formula = MOD_CONFIG_DEFAULT_DAMAGE_MOD_FORMULA;
    d.bonus_hth_damage_fix = MOD_CONFIG_DEFAULT_BONUS_HTH_DAMAGE_FIX;
    d.use_lockpick_frm = MOD_CONFIG_DEFAULT_USE_LOCKPICK_FRM;
    d.use_steal_frm = MOD_CONFIG_DEFAULT_USE_STEAL_FRM;
    d.use_traps_frm = MOD_CONFIG_DEFAULT_USE_TRAPS_FRM;
    d.use_first_aid_frm = MOD_CONFIG_DEFAULT_USE_FIRST_AID_FRM;
    d.use_doctor_frm = MOD_CONFIG_DEFAULT_USE_DOCTOR_FRM;
    d.use_science_frm = MOD_CONFIG_DEFAULT_USE_SCIENCE_FRM;
    d.use_repair_frm = MOD_CONFIG_DEFAULT_USE_REPAIR_FRM;
    d.science_repair_target_type = MOD_CONFIG_DEFAULT_SCIENCE_REPAIR_TARGET_TYPE;
    d.game_dialog_fix = MOD_CONFIG_DEFAULT_GAME_DIALOG_FIX;
    d.tweaks_file = MOD_CONFIG_DEFAULT_TWEAKS_FILE;
    d.game_dialog_gender_words = MOD_CONFIG_DEFAULT_GAME_DIALOG_GENDER_WORDS;
    d.town_map_hotkeys_fix = MOD_CONFIG_DEFAULT_TOWN_MAP_HOTKEYS_FIX;
    d.extra_message_lists = MOD_CONFIG_DEFAULT_EXTRA_MESSAGE_LISTS;
    d.patch_file = MOD_CONFIG_DEFAULT_PATCH_FILE;
    d.books_file = MOD_CONFIG_DEFAULT_BOOKS_FILE;
    d.elevators_file = MOD_CONFIG_DEFAULT_ELEVATORS_FILE;
    d.console_output_file = MOD_CONFIG_DEFAULT_CONSOLE_OUTPUT_FILE;
    d.use_walk_distance = MOD_CONFIG_DEFAULT_USE_WALK_DISTANCE;

    d.burst_mod_enabled = MOD_CONFIG_DEFAULT_BURST_MOD_ENABLED;
    d.burst_mod_center_multiplier = MOD_CONFIG_DEFAULT_BURST_MOD_CENTER_MULTIPLIER;
    d.burst_mod_center_divisor = MOD_CONFIG_DEFAULT_BURST_MOD_CENTER_DIVISOR;
    d.burst_mod_target_multiplier = MOD_CONFIG_DEFAULT_BURST_MOD_TARGET_MULTIPLIER;
    d.burst_mod_target_divisor = MOD_CONFIG_DEFAULT_BURST_MOD_TARGET_DIVISOR;

    d.iface_bar_mode = MOD_CONFIG_DEFAULT_IFACE_BAR_MODE;
    d.iface_bar_width = MOD_CONFIG_DEFAULT_IFACE_BAR_WIDTH;
    d.iface_bar_side_art = MOD_CONFIG_DEFAULT_IFACE_BAR_SIDE_ART;
    d.iface_bar_sides_ori = MOD_CONFIG_DEFAULT_IFACE_BAR_SIDES_ORI;

    d.float_audio_channels = MOD_CONFIG_DEFAULT_FLOAT_AUDIO_CHANNELS;
    d.float_distance_per_perception = MOD_CONFIG_DEFAULT_FLOAT_DISTANCE_PER_PERCEPTION;
    d.float_obstruction_dampening = MOD_CONFIG_DEFAULT_FLOAT_OBSTRUCTION_DAMPENING;
    d.float_eviction_policy = MOD_CONFIG_DEFAULT_FLOAT_EVICTION_POLICY;
    d.float_text_scramble = MOD_CONFIG_DEFAULT_FLOAT_TEXT_SCRAMBLE;
    d.float_text_scramble_distance_per_perception = MOD_CONFIG_DEFAULT_FLOAT_TEXT_SCRAMBLE_DISTANCE_PER_PERCEPTION;
    d.float_text_scramble_chars = MOD_CONFIG_DEFAULT_FLOAT_TEXT_SCRAMBLE_CHARS;
    d.voiced_floats = MOD_CONFIG_DEFAULT_VOICED_FLOATS;
    d.float_censor_bleep = MOD_CONFIG_DEFAULT_FLOAT_CENSOR_BLEEP;
    d.float_volume = MOD_CONFIG_DEFAULT_FLOAT_VOLUME;

    d.ini_config_folder = MOD_CONFIG_DEFAULT_INI_CONFIG_FOLDER;
    d.global_script_paths = MOD_CONFIG_DEFAULT_GLOBAL_SCRIPT_PATHS;

    // Per-game overrides
    // Only list fields that differ from the base block above. Everything
    // not mentioned inherits the base value.
    //
    if (IS_FALLOUT_1()) {
        d.start_year = 2161;
        d.start_month = 11; // zero-based: 11 = December
        d.start_day = 4; // zero-based: 4 = the 5th
        d.worldmap_trail_markers = true;
    }

    return d;
}

// Comparison function for qsort
static int compareStrings(const void* a, const void* b)
{
    return strcmp((const char*)a, (const char*)b);
}

// Scan the mods folder and return a list of base names (without .dat)
static int scanModsFolder(char modList[][MOD_INFO_MAX_NAME], int maxEntries)
{
    const char* modsPath = "mods";
    char pattern[COMPAT_MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s%c*", modsPath, DIR_SEPARATOR);
    int count = 0;
    DirectoryFileFindData findData;
    if (fileFindFirst(pattern, &findData)) {
        do {
            const char* name = fileFindGetName(&findData);
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;
            size_t len = strlen(name);
            // Check if the entry ends with ".dat" (case-insensitive)
            if (len >= 4 && compat_stricmp(name + len - 4, ".dat") == 0) {
                // Extract base name (without .dat)
                size_t baseLen = len - 4;
                char base[MOD_INFO_MAX_NAME];
                if (baseLen >= MOD_INFO_MAX_NAME) baseLen = MOD_INFO_MAX_NAME - 1;
                strncpy(base, name, baseLen);
                base[baseLen] = '\0';
                // Only accept if the base name starts with "mod_"
                if (strncmp(base, "mod_", 4) == 0 && count < maxEntries) {
                    // Remove "mod_" prefix to get internal name
                    char* internal = base + 4;
                    strncpy(modList[count], internal, MOD_INFO_MAX_NAME - 1);
                    modList[count][MOD_INFO_MAX_NAME - 1] = '\0';
                    count++;
                }
            }
        } while (fileFindNext(&findData));
        findFindClose(&findData);
    }
    return count;
}

static void extractDatName(const char* path, char* out, size_t outSize)
{
    const char* base = strrchr(path, DIR_SEPARATOR);
    if (!base)
        base = path;
    else
        base++;
    const char* dot = strchr(base, '.');
    size_t len = dot ? (dot - base) : strlen(base);
    if (len >= outSize) len = outSize - 1;
    strncpy(out, base, len);
    out[len] = '\0';
}

static int readModOrderFile(char orderList[][MOD_INFO_MAX_NAME], int maxEntries)
{
    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "mods%cmods_order.txt", DIR_SEPARATOR);
    FILE* f = compat_fopen(path, "r");
    if (!f) return 0;
    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), f) && count < maxEntries) {
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        char* start = line;
        while (*start == ' ')
            start++;
        if (*start == '#' || *start == ';' || *start == '\0') continue;
        // Remove .dat extension if present
        char* dot = strstr(start, ".dat");
        if (dot) *dot = '\0';
        strncpy(orderList[count], start, MOD_INFO_MAX_NAME - 1);
        orderList[count][MOD_INFO_MAX_NAME - 1] = '\0';
        count++;
    }
    fclose(f);
    return count;
}

static void writeModOrderFile(const char orderList[][MOD_INFO_MAX_NAME], int count)
{
    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "mods%cmods_order.txt", DIR_SEPARATOR);
    compat_mkdir("mods");
    FILE* f = compat_fopen(path, "w");
    if (!f) return;
    // Write header
    fprintf(f, "# FISSION mods_order.txt\n");
    fprintf(f, "#\n");
    fprintf(f, "# Mods (both directories and .dat files) are loaded in the order they appear below.\n");
    fprintf(f, "# Under FISSION's modular system, mods DO NOT usually need to be ordered,\n");
    fprintf(f, "# because assets are extended via hashed lists. However, if you want one mod\n");
    fprintf(f, "# to override another, place the overriding mod LOWER in this list.\n");
    fprintf(f, "#\n");
    fprintf(f, "# Lines beginning with '#' or ';' are ignored. Empty lines are also ignored.\n");
    fprintf(f, "\n");
    for (int i = 0; i < count; i++) {
        fprintf(f, "%s.dat\n", orderList[i]);
    }
    fclose(f);
}

static void extractModInfo(Config* config, ModInfo* info)
{
    memset(info, 0, sizeof(ModInfo));
    info->enabled = true;
    char* name = nullptr;
    if (configGetString(config, "mod_info", "name", &name) && name) {
        strncpy(info->name, name, MOD_INFO_MAX_NAME - 1);
        info->name[MOD_INFO_MAX_NAME - 1] = '\0';
    }
    char* displayName = nullptr;
    if (configGetString(config, "mod_info", "display_name", &displayName) && displayName) {
        strncpy(info->display_name, displayName, MOD_INFO_MAX_NAME - 1);
        info->display_name[MOD_INFO_MAX_NAME - 1] = '\0';
    } else {
        // fallback to the internal name (which may be empty if no name was given)
        strncpy(info->display_name, info->name, MOD_INFO_MAX_NAME - 1);
        info->display_name[MOD_INFO_MAX_NAME - 1] = '\0';
    }
    char* desc = nullptr;
    if (configGetString(config, "mod_info", "description", &desc) && desc) {
        strncpy(info->description, desc, MOD_INFO_MAX_DESC - 1);
        info->description[MOD_INFO_MAX_DESC - 1] = '\0';
    }
    char* author = nullptr;
    if (configGetString(config, "mod_info", "author", &author) && author) {
        strncpy(info->author, author, MOD_INFO_MAX_AUTHOR - 1);
        info->author[MOD_INFO_MAX_AUTHOR - 1] = '\0';
    }
    char* deps = nullptr;
    if (configGetString(config, "mod_info", "dependencies", &deps) && deps) {
        std::vector<std::string> tokens = splitString(deps, ',');
        info->dependencyCount = 0;
        for (const auto& token : tokens) {
            if (info->dependencyCount < MOD_INFO_MAX_DEP) {
                strncpy(info->dependencies[info->dependencyCount], token.c_str(), MOD_INFO_MAX_DEP_NAME - 1);
                info->dependencies[info->dependencyCount][MOD_INFO_MAX_DEP_NAME - 1] = '\0';
                info->dependencyCount++;
            } else
                break;
        }
    }
}

static int getModPresenceIndex(const char* modName)
{
    if (!modName || !modName[0]) return -1;
    return artGetStableIndex(modName);
}

static void writeModFidsFile()
{
    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%sdata%clists%cmod_ids_list.txt", _cd_path_base, DIR_SEPARATOR, DIR_SEPARATOR);
    FILE* f = compat_fopen(path, "w");
    if (!f) return;
    fprintf(f, "# Mod IDs and Dependencies\n");
    fprintf(f, "# Generated by FISSION-CE\n");
    fprintf(f, "# Use these IDs with art_exists() to check if a mod is present.\n");
    fprintf(f, "# The ID corresponds to art/intrface/<mod_name>.frm (or custom icon file).\n\n");
    for (int i = 0; i < gLoadedModsCount; i++) {
        const ModInfo* info = &gLoadedMods[i];
        fprintf(f, "[%s]\n", info->name);
        fprintf(f, "ID = %d\n", info->icon_index);
        if (info->dependencyCount > 0) {
            fprintf(f, "Dependencies = ");
            for (int j = 0; j < info->dependencyCount; j++) {
                int depIndex = getModPresenceIndex(info->dependencies[j]);
                fprintf(f, "%s (%d)", info->dependencies[j], depIndex);
                if (j < info->dependencyCount - 1) fprintf(f, ", ");
            }
            fprintf(f, "\n");
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

// ----------------------------------------------------------------------------
// New pipe-delimited mods_order.txt functions (for full metadata)
// ----------------------------------------------------------------------------

static int readModsOrderFile(ModInfo* outMods, int maxEntries)
{
    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "mods%cmods_order.txt", DIR_SEPARATOR);
    FILE* f = compat_fopen(path, "r");
    if (!f) return 0;

    int count = 0;
    char line[2048];
    while (fgets(line, sizeof(line), f) && count < maxEntries) {
        char* nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        if (line[0] == '#' || line[0] == ';' || line[0] == '\0') continue;

        // Expect pipe separators; if not present, treat as legacy (skip)
        if (!strchr(line, '|')) continue; // not our format

        // Expect exactly 8 fields
        char* token = strtok(line, "|");
        if (!token) continue;
        int enabled = atoi(token);

        token = strtok(nullptr, "|");
        if (!token) continue;
        char datName[MOD_INFO_MAX_NAME];
        strncpy(datName, token, MOD_INFO_MAX_NAME - 1);
        datName[MOD_INFO_MAX_NAME - 1] = '\0';

        token = strtok(nullptr, "|");
        if (!token) continue;
        char internalName[MOD_INFO_MAX_NAME];
        strncpy(internalName, token, MOD_INFO_MAX_NAME - 1);
        internalName[MOD_INFO_MAX_NAME - 1] = '\0';

        token = strtok(nullptr, "|");
        if (!token) continue;
        char displayName[MOD_INFO_MAX_NAME];
        strncpy(displayName, token, MOD_INFO_MAX_NAME - 1);
        displayName[MOD_INFO_MAX_NAME - 1] = '\0';

        token = strtok(nullptr, "|");
        if (!token) continue;
        char author[MOD_INFO_MAX_AUTHOR];
        strncpy(author, token, MOD_INFO_MAX_AUTHOR - 1);
        author[MOD_INFO_MAX_AUTHOR - 1] = '\0';

        token = strtok(nullptr, "|");
        if (!token) continue;
        char description[MOD_INFO_MAX_DESC];
        strncpy(description, token, MOD_INFO_MAX_DESC - 1);
        description[MOD_INFO_MAX_DESC - 1] = '\0';

        token = strtok(nullptr, "|");
        if (!token) continue;
        char dependencies[MOD_INFO_MAX_DESC];
        if (strcmp(token, " ") == 0) {
            dependencies[0] = '\0'; // empty
        } else {
            strncpy(dependencies, token, MOD_INFO_MAX_DESC - 1);
            dependencies[MOD_INFO_MAX_DESC - 1] = '\0';
        }

        token = strtok(nullptr, "|");
        int iconIndex = token ? atoi(token) : 0;

        ModInfo* info = &outMods[count];
        memset(info, 0, sizeof(ModInfo));
        info->enabled = (enabled != 0);
        strncpy(info->datName, datName, MOD_INFO_MAX_NAME - 1);
        strncpy(info->name, internalName, MOD_INFO_MAX_NAME - 1);
        strncpy(info->display_name, displayName, MOD_INFO_MAX_NAME - 1);
        strncpy(info->author, author, MOD_INFO_MAX_AUTHOR - 1);
        strncpy(info->description, description, MOD_INFO_MAX_DESC - 1);
        info->icon_index = iconIndex;

        // Parse dependencies (comma-separated internal names)
        char* depCopy = strdup(dependencies);
        char* depToken = strtok(depCopy, ",");
        while (depToken && info->dependencyCount < MOD_INFO_MAX_DEP) {
            while (*depToken == ' ')
                depToken++;
            strncpy(info->dependencies[info->dependencyCount], depToken, MOD_INFO_MAX_DEP_NAME - 1);
            info->dependencyCount++;
            depToken = strtok(nullptr, ",");
        }
        free(depCopy);
        count++;
    }
    fclose(f);
    return count;
}

static void writeModsOrderFile(const ModInfo* mods, int count)
{
    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "mods%cmods_order.txt", DIR_SEPARATOR);
    compat_mkdir("mods");
    FILE* f = compat_fopen(path, "w");
    if (!f) return;

    fprintf(f, "# FISSION mods_order.txt (pipe-separated)\n");
    fprintf(f, "# Format: enabled|datName|internalName|displayName|author|description|dependencies|iconIndex\n");
    fprintf(f, "\n");

    for (int i = 0; i < count; i++) {
        const ModInfo* info = &mods[i];
        // Build dependencies string from internal names
        char deps[MOD_INFO_MAX_DESC] = "";
        for (int j = 0; j < info->dependencyCount; j++) {
            if (j > 0) strcat(deps, ",");
            strcat(deps, info->dependencies[j]);
        }
        const char* depsOut = (deps[0] == '\0') ? " " : deps; // space for empty dependencies
        const char* datNameOut = (info->datName[0] != '\0') ? info->datName : info->name;
        fprintf(f, "%d|%s|%s|%s|%s|%s|%s|%d\n",
            info->enabled ? 1 : 0,
            datNameOut,
            info->name,
            info->display_name,
            info->author,
            info->description,
            depsOut,
            info->icon_index);
    }
    fclose(f);
}

void modConfigWriteOrderFromLoadedMods()
{
    writeModsOrderFile(gLoadedMods, gLoadedModsCount);
}

void modConfigWriteEnabledForSlot(const char* fullPath)
{
    File* f = fileOpen(fullPath, "wb");
    if (!f) return;

    char buffer[16384];
    int pos = 0;
    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
        "# FISSION per-save mod enabled flags\n");
    pos += snprintf(buffer + pos, sizeof(buffer) - pos,
        "# Format: datName|displayName|enabled (1/0)\n");

    for (int i = 0; i < gLoadedModsCount; i++) {
        pos += snprintf(buffer + pos, sizeof(buffer) - pos,
            "%s|%s|%d\n",
            gLoadedMods[i].datName,
            gLoadedMods[i].display_name,
            gLoadedMods[i].enabled ? 1 : 0);
    }
    fileWrite(buffer, 1, pos, f);
    fileClose(f);
}

int modConfigCheckSlotEnabledMatchEx(const char* fullPath, char* missingModName, size_t maxSize)
{
    File* f = fileOpen(fullPath, "rb");
    if (!f) return 3;
    int fileSize = fileGetSize(f);
    if (fileSize <= 0) {
        fileClose(f);
        return 3;
    }
    char* buffer = (char*)internal_malloc(fileSize + 1);
    if (!buffer) {
        fileClose(f);
        return 3;
    }
    if (fileRead(buffer, 1, fileSize, f) != fileSize) {
        internal_free(buffer);
        fileClose(f);
        return 3;
    }
    buffer[fileSize] = '\0';
    fileClose(f);

    struct SavedMod {
        char name[MOD_INFO_MAX_NAME];
        char displayName[MOD_INFO_MAX_NAME];
        bool enabled;
    } savedMods[MAX_LOADED_MODS];
    int savedModsCount = 0;

    char* line = buffer;
    while (line && *line) {
        char* end = strchr(line, '\n');
        if (end) *end = '\0';
        if (line[0] != '#' && line[0] != '\0') {
            char modName[MOD_INFO_MAX_NAME];
            char displayName[MOD_INFO_MAX_NAME] = "";
            int enabledInt;

            // Try new pipe format
            int parsed = sscanf(line, "%127[^|]|%127[^|]|%d", modName, displayName, &enabledInt);
            if (parsed == 3) {
                if (savedModsCount < MAX_LOADED_MODS) {
                    strncpy(savedMods[savedModsCount].name, modName, MOD_INFO_MAX_NAME - 1);
                    savedMods[savedModsCount].name[MOD_INFO_MAX_NAME - 1] = '\0';
                    strncpy(savedMods[savedModsCount].displayName, displayName, MOD_INFO_MAX_NAME - 1);
                    savedMods[savedModsCount].displayName[MOD_INFO_MAX_NAME - 1] = '\0';
                    savedMods[savedModsCount].enabled = (enabledInt != 0);
                    savedModsCount++;
                }
            } else if (sscanf(line, "%127s %d", modName, &enabledInt) == 2) {
                // Old format
                if (savedModsCount < MAX_LOADED_MODS) {
                    strncpy(savedMods[savedModsCount].name, modName, MOD_INFO_MAX_NAME - 1);
                    savedMods[savedModsCount].name[MOD_INFO_MAX_NAME - 1] = '\0';
                    savedMods[savedModsCount].displayName[0] = '\0';
                    savedMods[savedModsCount].enabled = (enabledInt != 0);
                    savedModsCount++;
                }
            }
        }
        line = end ? (end + 1) : nullptr;
    }
    internal_free(buffer);

    // Check for missing mods
    for (int i = 0; i < savedModsCount; i++) {
        bool found = false;
        for (int j = 0; j < gLoadedModsCount; j++) {
            if (strcmp(gLoadedMods[j].datName, savedMods[i].name) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            if (missingModName && maxSize > 0) {
                const char* friendly = savedMods[i].displayName[0] ? savedMods[i].displayName : savedMods[i].name;
                strncpy(missingModName, friendly, maxSize - 1);
                missingModName[maxSize - 1] = '\0';
            }
            return 2;
        }
    }

    // Check for enabled flag mismatches
    for (int i = 0; i < savedModsCount; i++) {
        for (int j = 0; j < gLoadedModsCount; j++) {
            if (strcmp(gLoadedMods[j].datName, savedMods[i].name) == 0) {
                if (gLoadedMods[j].enabled != savedMods[i].enabled) {
                    return 1;
                }
                break;
            }
        }
    }
    return 0;
}

int modConfigApplySaveModConfig(const char* slotPath)
{
    File* f = fileOpen(slotPath, "rb");
    if (!f) return -1;
    int fileSize = fileGetSize(f);
    if (fileSize <= 0) {
        fileClose(f);
        return -1;
    }
    char* buffer = (char*)internal_malloc(fileSize + 1);
    if (!buffer) {
        fileClose(f);
        return -1;
    }
    if (fileRead(buffer, 1, fileSize, f) != fileSize) {
        internal_free(buffer);
        fileClose(f);
        return -1;
    }
    buffer[fileSize] = '\0';
    fileClose(f);

    struct ExpectedMod {
        char datName[MOD_INFO_MAX_NAME];
        bool enabled;
    } expected[MAX_LOADED_MODS];
    int expectedCount = 0;

    char* line = buffer;
    while (line && *line) {
        char* end = strchr(line, '\n');
        if (end) *end = '\0';
        if (line[0] != '#' && line[0] != '\0') {
            char modName[MOD_INFO_MAX_NAME];
            int enabledInt;
            // Try new pipe format: datName|displayName|enabled
            if (sscanf(line, "%127[^|]|%*[^|]|%d", modName, &enabledInt) == 2) {
                if (expectedCount < MAX_LOADED_MODS) {
                    strncpy(expected[expectedCount].datName, modName, MOD_INFO_MAX_NAME - 1);
                    expected[expectedCount].enabled = (enabledInt != 0);
                    expectedCount++;
                }
            }
        }
        line = end ? (end + 1) : nullptr;
    }
    internal_free(buffer);

    // First set all current mods to disabled
    for (int i = 0; i < gLoadedModsCount; i++) {
        gLoadedMods[i].enabled = false;
    }
    // Then enable only those expected
    for (int i = 0; i < expectedCount; i++) {
        for (int j = 0; j < gLoadedModsCount; j++) {
            if (strcmp(gLoadedMods[j].datName, expected[i].datName) == 0) {
                gLoadedMods[j].enabled = expected[i].enabled;
                break;
            }
        }
    }
    return 0;
}

bool modConfigInit(int argc, char** argv)
{
    if (gModConfigInitialized) return false;
    if (!configInit(&gModConfig)) return false;

    // Resolve per-game defaults once. IS_FALLOUT_1() is authoritative here
    // because gameDbInit() has already opened master.dat and latched the
    // version.
    const ModDefaults defaults = modDefaultsGet();
    debugPrint(">>> modConfigInit: applying game defaults (fallout1=%d)\n",
        (int)IS_FALLOUT_1());

    // ---- Default settings ----
    // Every value below comes from modDefaultsGet(). The MOD_CONFIG_DEFAULT_*
    // constants are no longer read here directly — to change a default, edit
    // the base block or add a per-game override inside modDefaultsGet().
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_YEAR, defaults.start_year);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_MONTH, defaults.start_month);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_DAY, defaults.start_day);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_STARTING_MAP_KEY, defaults.starting_map);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER1, defaults.movie_timer_artimer1);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER2, defaults.movie_timer_artimer2);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER3, defaults.movie_timer_artimer3);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER4, defaults.movie_timer_artimer4);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PIPBOY_AVAILABLE_AT_GAMESTART, defaults.pipboy_available_at_gamestart);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_OVERRIDE_CRITICALS_MODE_KEY, defaults.override_criticals_mode);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_OVERRIDE_CRITICALS_FILE_KEY, defaults.override_criticals_file);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PREMADE_CHARACTERS_FILE_NAMES_KEY, defaults.premade_characters_file_names);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PREMADE_CHARACTERS_FACE_FIDS_KEY, defaults.premade_characters_face_fids);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_MALE_KEY, defaults.dude_native_look_jumpsuit_male);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE_KEY, defaults.dude_native_look_jumpsuit_female);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_MALE_KEY, defaults.dude_native_look_tribal_male);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_FEMALE_KEY, defaults.dude_native_look_tribal_female);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_BIG_FONT_COLOR_KEY, defaults.main_menu_big_font_color);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_X_KEY, defaults.main_menu_credits_offset_x);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_Y_KEY, defaults.main_menu_credits_offset_y);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_FONT_COLOR_KEY, defaults.main_menu_font_color);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_OFFSET_X_KEY, defaults.main_menu_offset_x);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_OFFSET_Y_KEY, defaults.main_menu_offset_y);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_VERSION_STRING, defaults.version_string);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_WORLDMAP_TRAIL_MARKERS, defaults.worldmap_trail_markers);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_KARMA_FRMS_KEY, defaults.karma_frms);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_KARMA_POINTS_KEY, defaults.karma_points);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DYNAMITE_MIN_DAMAGE_KEY, defaults.dynamite_min_damage);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DYNAMITE_MAX_DAMAGE_KEY, defaults.dynamite_max_damage);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PLASTIC_EXPLOSIVE_MIN_DAMAGE_KEY, defaults.plastic_explosive_min_damage);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PLASTIC_EXPLOSIVE_MAX_DAMAGE_KEY, defaults.plastic_explosive_max_damage);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_CITY_REPUTATION_LIST_KEY, defaults.city_reputation_list);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_UNARMED_FILE_KEY, defaults.unarmed_file);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DAMAGE_MOD_FORMULA_KEY, defaults.damage_mod_formula);
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BONUS_HTH_DAMAGE_FIX_KEY, defaults.bonus_hth_damage_fix);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_LOCKPICK_FRM_KEY, defaults.use_lockpick_frm);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_STEAL_FRM_KEY, defaults.use_steal_frm);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_TRAPS_FRM_KEY, defaults.use_traps_frm);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_FIRST_AID_FRM_KEY, defaults.use_first_aid_frm);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_DOCTOR_FRM_KEY, defaults.use_doctor_frm);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_SCIENCE_FRM_KEY, defaults.use_science_frm);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_REPAIR_FRM_KEY, defaults.use_repair_frm);
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_SCIENCE_REPAIR_TARGET_TYPE_KEY, defaults.science_repair_target_type);
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_GAME_DIALOG_FIX_KEY, defaults.game_dialog_fix);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_TWEAKS_FILE_KEY, defaults.tweaks_file);
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_GAME_DIALOG_GENDER_WORDS_KEY, defaults.game_dialog_gender_words);
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_TOWN_MAP_HOTKEYS_FIX_KEY, defaults.town_map_hotkeys_fix);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_WALK_DISTANCE, defaults.use_walk_distance);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_WORLDMAP_FOG_LEVEL, MOD_CONFIG_DEFAULT_FOG_LEVEL);

    // Vock floats
    configSetInt(&gModConfig, MOD_CONFIG_VOCK_FLOATS_KEY, MOD_CONFIG_FLOAT_AUDIO_CHANNELS_KEY, defaults.float_audio_channels);
    configSetInt(&gModConfig, MOD_CONFIG_VOCK_FLOATS_KEY, MOD_CONFIG_FLOAT_DISTANCE_PER_PERCEPTION_KEY, defaults.float_distance_per_perception);
    configSetInt(&gModConfig, MOD_CONFIG_VOCK_FLOATS_KEY, MOD_CONFIG_FLOAT_OBSTRUCTION_DAMPENING_KEY, defaults.float_obstruction_dampening);
    configSetInt(&gModConfig, MOD_CONFIG_VOCK_FLOATS_KEY, MOD_CONFIG_FLOAT_EVICTION_POLICY_KEY, defaults.float_eviction_policy);
    configSetBool(&gModConfig, MOD_CONFIG_VOCK_FLOATS_KEY, MOD_CONFIG_FLOAT_TEXT_SCRAMBLE_KEY, defaults.float_text_scramble);
    configSetInt(&gModConfig, MOD_CONFIG_VOCK_FLOATS_KEY, MOD_CONFIG_FLOAT_TEXT_SCRAMBLE_DISTANCE_PER_PERCEPTION_KEY, defaults.float_text_scramble_distance_per_perception);
    configSetString(&gModConfig, MOD_CONFIG_VOCK_FLOATS_KEY, MOD_CONFIG_FLOAT_TEXT_SCRAMBLE_CHARS_KEY, defaults.float_text_scramble_chars);
    configSetBool(&gModConfig, MOD_CONFIG_VOCK_FLOATS_KEY, MOD_CONFIG_VOICED_FLOATS_KEY, defaults.voiced_floats);
    configSetBool(&gModConfig, MOD_CONFIG_VOCK_FLOATS_KEY, MOD_CONFIG_FLOAT_CENSOR_BLEEP_KEY, defaults.float_censor_bleep);
    configSetInt(&gModConfig, MOD_CONFIG_VOCK_FLOATS_KEY, MOD_CONFIG_FLOAT_VOLUME_KEY, defaults.float_volume);

    // Scripts
    configSetString(&gModConfig, MOD_CONFIG_SCRIPTS_KEY, MOD_CONFIG_INI_CONFIG_FOLDER, defaults.ini_config_folder);
    configSetString(&gModConfig, MOD_CONFIG_SCRIPTS_KEY, MOD_CONFIG_GLOBAL_SCRIPT_PATHS, defaults.global_script_paths);

    // Files and paths
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PATCH_FILE, defaults.patch_file);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_EXTRA_MESSAGE_LISTS_KEY, defaults.extra_message_lists);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BOOKS_FILE_KEY, defaults.books_file);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_ELEVATORS_FILE_KEY, defaults.elevators_file);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_CONSOLE_OUTPUT_FILE_KEY, defaults.console_output_file);

    // Burst
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_ENABLED_KEY, defaults.burst_mod_enabled);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_CENTER_MULTIPLIER_KEY, defaults.burst_mod_center_multiplier);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_CENTER_DIVISOR_KEY, defaults.burst_mod_center_divisor);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_TARGET_MULTIPLIER_KEY, defaults.burst_mod_target_multiplier);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_TARGET_DIVISOR_KEY, defaults.burst_mod_target_divisor);

    // Interface bar
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_MODE, defaults.iface_bar_mode);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_WIDTH, defaults.iface_bar_width);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_SIDE_ART, defaults.iface_bar_side_art);
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_SIDES_ORI, defaults.iface_bar_sides_ori);

    // Scan mods folder
    char folderMods[MAX_LOADED_MODS][MOD_INFO_MAX_NAME];
    int folderCount = scanModsFolder(folderMods, MAX_LOADED_MODS);

    // Read existing mods_order.txt (pipe format)
    ModInfo orderMods[MAX_LOADED_MODS];
    int orderCount = readModsOrderFile(orderMods, MAX_LOADED_MODS);

    struct TempMod {
        ModInfo info;
        bool loaded;
    };
    std::vector<TempMod> allMods;

    for (int i = 0; i < orderCount; i++) {
        TempMod tm;
        tm.info = orderMods[i];
        tm.loaded = false;
        allMods.push_back(tm);
    }

    // Discover new mods (not in order file)
    for (int i = 0; i < folderCount; i++) {
        const char* datName = folderMods[i];
        bool found = false;
        for (const auto& tm : allMods) {
            if (compat_stricmp(tm.info.datName, datName) == 0) {
                found = true;
                break;
            }
        }
        if (!found && allMods.size() < MAX_LOADED_MODS) {
            char datPath[COMPAT_MAX_PATH];
            snprintf(datPath, sizeof(datPath), "mods%cmod_%s.dat", DIR_SEPARATOR, datName);
            int handle = dbOpen(datPath, nullptr);
            if (handle != -1) {
                ModInfo newInfo;
                memset(&newInfo, 0, sizeof(newInfo));
                newInfo.enabled = true;
                strncpy(newInfo.datName, datName, MOD_INFO_MAX_NAME - 1);
                strncpy(newInfo.name, datName, MOD_INFO_MAX_NAME - 1);

                char cfgPath[COMPAT_MAX_PATH];
                snprintf(cfgPath, sizeof(cfgPath), "data%cmod_%s.cfg", DIR_SEPARATOR, datName);
                Config tempCfg;
                configInit(&tempCfg);
                if (configRead(&tempCfg, cfgPath, true)) {
                    extractModInfo(&tempCfg, &newInfo);
                } else {
                    snprintf(newInfo.display_name, MOD_INFO_MAX_NAME, "%s", datName);
                    snprintf(newInfo.author, MOD_INFO_MAX_AUTHOR, "Unknown");
                    snprintf(newInfo.description, MOD_INFO_MAX_DESC, "No description provided.");
                }
                configFree(&tempCfg);
                newInfo.icon_index = artGetStableIndex(newInfo.name);

                TempMod tm;
                tm.info = newInfo;
                tm.loaded = true;
                allMods.push_back(tm);
            }
        }
    }

    // Build global list but DO NOT load any mods here
    gLoadedModsCount = allMods.size();
    for (size_t i = 0; i < allMods.size(); i++) {
        gLoadedMods[i] = allMods[i].info;
        // Fix empty datName
        if (gLoadedMods[i].datName[0] == '\0') {
            strncpy(gLoadedMods[i].datName, gLoadedMods[i].name, MOD_INFO_MAX_NAME - 1);
        }
    }

    // Write back mods_order.txt (new pipe format)
    writeModsOrderFile(gLoadedMods, gLoadedModsCount);

    // Read global game.cfg and enabled mods' configs (for settings)
    char cfgMainPath[COMPAT_MAX_PATH];
    snprintf(cfgMainPath, sizeof(cfgMainPath), "data%c%s", DIR_SEPARATOR, MOD_CONFIG_FILE_NAME);
    configRead(&gModConfig, cfgMainPath, true);

    for (int i = 0; i < gLoadedModsCount; i++) {
        const ModInfo* info = &gLoadedMods[i];
        if (info->enabled && info->name[0] != '\0') {
            char modCfgPath[COMPAT_MAX_PATH];
            snprintf(modCfgPath, sizeof(modCfgPath), "data%cgame_%s.cfg", DIR_SEPARATOR, info->name);
            configRead(&gModConfig, modCfgPath, true);
        }
    }

    configParseCommandLineArguments(&gModConfig, argc, argv);

    // Final orphan cleanup: remove any mod in gLoadedMods whose .dat file is missing
    for (int i = gLoadedModsCount - 1; i >= 0; i--) {
        char checkPath[COMPAT_MAX_PATH];
        snprintf(checkPath, sizeof(checkPath), "mods%cmod_%s.dat", DIR_SEPARATOR, gLoadedMods[i].datName);
        if (compat_access(checkPath, 0) != 0) {
            // File missing, remove this mod from list
            for (int j = i; j < gLoadedModsCount - 1; j++) {
                gLoadedMods[j] = gLoadedMods[j + 1];
            }
            gLoadedModsCount--;
        }
    }

    writeModFidsFile();

    gModConfigInitialized = true;

    settingsFromModConfig();

    // Restore CWD to data by calling dbOpen again - critical, otherwise CWD will be mods folder
    dbOpen(nullptr, settings.system.master_patches_path.c_str());

    return true;
}

void modConfigExit()
{
    if (gModConfigInitialized) {
        configFree(&gModConfig);
        gModConfigInitialized = false;
    }
}

} // namespace fallout