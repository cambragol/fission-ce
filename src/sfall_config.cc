#include "sfall_config.h"

#include "art.h"
#include "db.h"
#include "file_find.h"
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
            // Check if the entry ends with ".dat" (case?insensitive)
            if (len >= 4 && compat_stricmp(name + len - 4, ".dat") == 0) {
                // Extract base name (without .dat)
                size_t baseLen = len - 4;
                char base[MOD_INFO_MAX_NAME];
                if (baseLen >= MOD_INFO_MAX_NAME) baseLen = MOD_INFO_MAX_NAME - 1;
                strncpy(base, name, baseLen);
                base[baseLen] = '\0';

                // Only accept if the base name starts with "mod_"
                if (strncmp(base, "mod_", 4) == 0 && count < maxEntries) {
                    strncpy(modList[count], base, MOD_INFO_MAX_NAME - 1);
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
    // Find the last separator
    const char* base = strrchr(path, DIR_SEPARATOR);
    if (!base)
        base = path;
    else
        base++;
    // Copy until '.'
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

void modConfigWriteOrderFromLoadedMods()
{
    char orderList[MAX_LOADED_MODS][MOD_INFO_MAX_NAME];
    int validCount = 0;
    for (int i = 0; i < gLoadedModsCount; i++) {
        if (gLoadedMods[i].datName[0] != '\0') {
            strncpy(orderList[validCount], gLoadedMods[i].datName, MOD_INFO_MAX_NAME - 1);
            orderList[validCount][MOD_INFO_MAX_NAME - 1] = '\0';
            validCount++;
        }
    }
    writeModOrderFile(orderList, validCount);
}

static int getModPresenceIndex(const char* modName)
{
    if (!modName || !modName[0]) return -1;
    // Pass the base name directly (same as engine does for .lst entries)
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

// Extract mod_info section into ModInfo struct
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
            } else {
                break;
            }
        }
    }
}

static void syncModsOrderFile(void)
{
    // Ensure the mods folder exists
    compat_mkdir("mods");

    // Scan the folder for all mod entries (files/folders ending with .dat)
    char folderMods[MAX_LOADED_MODS][MOD_INFO_MAX_NAME];
    int folderCount = scanModsFolder(folderMods, MAX_LOADED_MODS);

    // Add default entries for any mod not already in gLoadedMods
    for (int i = 0; i < folderCount; i++) {
        const char* base = folderMods[i];
        int found = 0;
        for (int j = 0; j < gLoadedModsCount; j++) {
            if (strcmp(gLoadedMods[j].datName, base) == 0) {
                found = 1;
                break;
            }
        }
        if (!found && gLoadedModsCount < MAX_LOADED_MODS) {
            ModInfo defaultInfo;
            memset(&defaultInfo, 0, sizeof(defaultInfo));
            defaultInfo.enabled = true;
            strncpy(defaultInfo.name, base, MOD_INFO_MAX_NAME - 1);
            defaultInfo.name[MOD_INFO_MAX_NAME - 1] = '\0';
            strncpy(defaultInfo.display_name, base, MOD_INFO_MAX_NAME - 1);
            defaultInfo.display_name[MOD_INFO_MAX_NAME - 1] = '\0';
            strncpy(defaultInfo.description, "No description available", MOD_INFO_MAX_DESC - 1);
            strncpy(defaultInfo.author, "Unknown", MOD_INFO_MAX_AUTHOR - 1);
            strncpy(defaultInfo.datName, base, MOD_INFO_MAX_NAME - 1);
            defaultInfo.dependencyCount = 0;
            // filePath remains empty (no .cfg file)
            gLoadedMods[gLoadedModsCount++] = defaultInfo;
        }
    }

    // Recompute icon_index for all mods (including newly added ones)
    for (int i = 0; i < gLoadedModsCount; i++) {
        gLoadedMods[i].icon_index = getModPresenceIndex(gLoadedMods[i].name);
    }

    // Read current mod order file (if any)
    char orderList[MAX_LOADED_MODS][MOD_INFO_MAX_NAME];
    int orderCount = readModOrderFile(orderList, MAX_LOADED_MODS);

    // Collect new mods (present in folder but not in order file)
    char newMods[MAX_LOADED_MODS][MOD_INFO_MAX_NAME];
    int newModsCount = 0;
    for (int i = 0; i < folderCount; i++) {
        const char* base = folderMods[i];
        int inOrder = 0;
        for (int j = 0; j < orderCount; j++) {
            if (strcmp(orderList[j], base) == 0) {
                inOrder = 1;
                break;
            }
        }
        if (!inOrder && newModsCount < MAX_LOADED_MODS) {
            strncpy(newMods[newModsCount], base, MOD_INFO_MAX_NAME - 1);
            newMods[newModsCount][MOD_INFO_MAX_NAME - 1] = '\0';
            newModsCount++;
        }
    }

    // Append new mods in alphabetical order
    if (newModsCount > 0) {
        qsort(newMods, newModsCount, MOD_INFO_MAX_NAME, compareStrings);
        for (int i = 0; i < newModsCount && orderCount < MAX_LOADED_MODS; i++) {
            strncpy(orderList[orderCount], newMods[i], MOD_INFO_MAX_NAME - 1);
            orderList[orderCount][MOD_INFO_MAX_NAME - 1] = '\0';
            orderCount++;
        }
    }

    // Write the (possibly updated) order file
    if (orderCount > 0) {
        writeModOrderFile(orderList, orderCount);
    } else if (folderCount > 0) {
        // No order file existed and there are mods - create a sorted list
        char newOrderList[MAX_LOADED_MODS][MOD_INFO_MAX_NAME];
        int newCount = 0;
        for (int i = 0; i < folderCount && newCount < MAX_LOADED_MODS; i++) {
            strncpy(newOrderList[newCount], folderMods[i], MOD_INFO_MAX_NAME - 1);
            newOrderList[newCount][MOD_INFO_MAX_NAME - 1] = '\0';
            newCount++;
        }
        qsort(newOrderList, newCount, MOD_INFO_MAX_NAME, compareStrings);
        writeModOrderFile(newOrderList, newCount);
        // Update orderList and orderCount for reordering
        orderCount = newCount;
        memcpy(orderList, newOrderList, orderCount * MOD_INFO_MAX_NAME);
    }

    // Reorder gLoadedMods according to the final orderList
    if (orderCount > 0) {
        ModInfo newList[MAX_LOADED_MODS];
        int newCount = 0;

        // Add mods in the order specified by the order file
        for (int i = 0; i < orderCount; i++) {
            for (int j = 0; j < gLoadedModsCount; j++) {
                if (strcmp(gLoadedMods[j].datName, orderList[i]) == 0) {
                    newList[newCount++] = gLoadedMods[j];
                    break;
                }
            }
        }

        // Add remaining mods (not in order file) in their original order
        for (int i = 0; i < gLoadedModsCount; i++) {
            int found = 0;
            for (int j = 0; j < orderCount; j++) {
                if (strcmp(gLoadedMods[i].datName, orderList[j]) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                newList[newCount++] = gLoadedMods[i];
            }
        }

        // Replace the global list with the sorted one
        memcpy(gLoadedMods, newList, newCount * sizeof(ModInfo));
        gLoadedModsCount = newCount;
    }
}

bool modConfigInit(int argc, char** argv)
{
    if (gModConfigInitialized) {
        return false;
    }

    if (!configInit(&gModConfig)) {
        return false;
    }

    // Initialize defaults – grouped by category

    // Mod Settings
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_YEAR, MOD_CONFIG_DEFAULT_START_YEAR);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_MONTH, MOD_CONFIG_DEFAULT_START_MONTH);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_DAY, MOD_CONFIG_DEFAULT_START_DAY);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_STARTING_MAP_KEY, MOD_CONFIG_DEFAULT_STARTING_MAP);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER1, MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER1);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER2, MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER2);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER3, MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER3);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER4, MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER4);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PIPBOY_AVAILABLE_AT_GAMESTART, MOD_CONFIG_DEFAULT_PIPBOY_AVAILABLE_AT_GAMESTART);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_OVERRIDE_CRITICALS_MODE_KEY, MOD_CONFIG_DEFAULT_OVERRIDE_CRITICALS_MODE);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_OVERRIDE_CRITICALS_FILE_KEY, MOD_CONFIG_DEFAULT_OVERRIDE_CRITICALS_FILE);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PREMADE_CHARACTERS_FILE_NAMES_KEY, MOD_CONFIG_DEFAULT_PREMADE_CHARACTERS_FILE_NAMES);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PREMADE_CHARACTERS_FACE_FIDS_KEY, MOD_CONFIG_DEFAULT_PREMADE_CHARACTERS_FACE_FIDS);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_MALE_KEY, MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_JUMPSUIT_MALE);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE_KEY, MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_MALE_KEY, MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_TRIBAL_MALE);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_FEMALE_KEY, MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_TRIBAL_FEMALE);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_BIG_FONT_COLOR_KEY, MOD_CONFIG_DEFAULT_MAIN_MENU_BIG_FONT_COLOR);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_X_KEY, MOD_CONFIG_DEFAULT_MAIN_MENU_CREDITS_OFFSET_X);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_Y_KEY, MOD_CONFIG_DEFAULT_MAIN_MENU_CREDITS_OFFSET_Y);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_FONT_COLOR_KEY, MOD_CONFIG_DEFAULT_MAIN_MENU_FONT_COLOR);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_OFFSET_X_KEY, MOD_CONFIG_DEFAULT_MAIN_MENU_OFFSET_X);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_OFFSET_Y_KEY, MOD_CONFIG_DEFAULT_MAIN_MENU_OFFSET_Y);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_VERSION_STRING, MOD_CONFIG_DEFAULT_VERSION_STRING);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_WORLDMAP_TRAIL_MARKERS, MOD_CONFIG_DEFAULT_WORLDMAP_TRAIL_MARKERS);

    // Additional Mod Settings (recently added defaults)
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_KARMA_FRMS_KEY, MOD_CONFIG_DEFAULT_KARMA_FRMS);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_KARMA_POINTS_KEY, MOD_CONFIG_DEFAULT_KARMA_POINTS);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DYNAMITE_MIN_DAMAGE_KEY, MOD_CONFIG_DEFAULT_DYNAMITE_MIN_DAMAGE);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DYNAMITE_MAX_DAMAGE_KEY, MOD_CONFIG_DEFAULT_DYNAMITE_MAX_DAMAGE);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PLASTIC_EXPLOSIVE_MIN_DAMAGE_KEY, MOD_CONFIG_DEFAULT_PLASTIC_EXPLOSIVE_MIN_DAMAGE);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PLASTIC_EXPLOSIVE_MAX_DAMAGE_KEY, MOD_CONFIG_DEFAULT_PLASTIC_EXPLOSIVE_MAX_DAMAGE);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_CITY_REPUTATION_LIST_KEY, MOD_CONFIG_DEFAULT_CITY_REPUTATION_LIST);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_UNARMED_FILE_KEY, MOD_CONFIG_DEFAULT_UNARMED_FILE);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DAMAGE_MOD_FORMULA_KEY, MOD_CONFIG_DEFAULT_DAMAGE_MOD_FORMULA);
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BONUS_HTH_DAMAGE_FIX_KEY, MOD_CONFIG_DEFAULT_BONUS_HTH_DAMAGE_FIX);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_LOCKPICK_FRM_KEY, MOD_CONFIG_DEFAULT_USE_LOCKPICK_FRM);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_STEAL_FRM_KEY, MOD_CONFIG_DEFAULT_USE_STEAL_FRM);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_TRAPS_FRM_KEY, MOD_CONFIG_DEFAULT_USE_TRAPS_FRM);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_FIRST_AID_FRM_KEY, MOD_CONFIG_DEFAULT_USE_FIRST_AID_FRM);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_DOCTOR_FRM_KEY, MOD_CONFIG_DEFAULT_USE_DOCTOR_FRM);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_SCIENCE_FRM_KEY, MOD_CONFIG_DEFAULT_USE_SCIENCE_FRM);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_REPAIR_FRM_KEY, MOD_CONFIG_DEFAULT_USE_REPAIR_FRM);
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_SCIENCE_REPAIR_TARGET_TYPE_KEY, MOD_CONFIG_DEFAULT_SCIENCE_REPAIR_TARGET_TYPE);
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_GAME_DIALOG_FIX_KEY, MOD_CONFIG_DEFAULT_GAME_DIALOG_FIX);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_TWEAKS_FILE_KEY, MOD_CONFIG_DEFAULT_TWEAKS_FILE);
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_GAME_DIALOG_GENDER_WORDS_KEY, MOD_CONFIG_DEFAULT_GAME_DIALOG_GENDER_WORDS);
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_TOWN_MAP_HOTKEYS_FIX_KEY, MOD_CONFIG_DEFAULT_TOWN_MAP_HOTKEYS_FIX);

    // Game Fixes
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_WALK_DISTANCE, MOD_CONFIG_DEFAULT_USE_WALK_DISTANCE);

    // Files and Paths
    configSetString(&gModConfig, MOD_CONFIG_SCRIPTS_KEY, MOD_CONFIG_INI_CONFIG_FOLDER, MOD_CONFIG_DEFAULT_INI_CONFIG_FOLDER);
    configSetString(&gModConfig, MOD_CONFIG_SCRIPTS_KEY, MOD_CONFIG_GLOBAL_SCRIPT_PATHS, MOD_CONFIG_DEFAULT_GLOBAL_SCRIPT_PATHS);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PATCH_FILE, MOD_CONFIG_DEFAULT_PATCH_FILE);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_EXTRA_MESSAGE_LISTS_KEY, MOD_CONFIG_DEFAULT_EXTRA_MESSAGE_LISTS);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BOOKS_FILE_KEY, MOD_CONFIG_DEFAULT_BOOKS_FILE);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_ELEVATORS_FILE_KEY, MOD_CONFIG_DEFAULT_ELEVATORS_FILE);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_CONSOLE_OUTPUT_FILE_KEY, MOD_CONFIG_DEFAULT_CONSOLE_OUTPUT_FILE);

    // Mods (Burst)
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_ENABLED_KEY, MOD_CONFIG_DEFAULT_BURST_MOD_ENABLED);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_CENTER_MULTIPLIER_KEY, MOD_CONFIG_BURST_MOD_DEFAULT_CENTER_MULTIPLIER);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_CENTER_DIVISOR_KEY, MOD_CONFIG_BURST_MOD_DEFAULT_CENTER_DIVISOR);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_TARGET_MULTIPLIER_KEY, MOD_CONFIG_BURST_MOD_DEFAULT_TARGET_MULTIPLIER);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_TARGET_DIVISOR_KEY, MOD_CONFIG_BURST_MOD_DEFAULT_TARGET_DIVISOR);

    // Others (Interface Bar, etc.)
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_MODE, MOD_CONFIG_DEFAULT_IFACE_BAR_MODE);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_WIDTH, MOD_CONFIG_DEFAULT_IFACE_BAR_WIDTH);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_SIDE_ART, MOD_CONFIG_DEFAULT_IFACE_BAR_SIDE_ART);
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_SIDES_ORI, MOD_CONFIG_DEFAULT_IFACE_BAR_SIDES_ORI);

    // Read all .cfg files and collect metadata + settings
    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "data%c%s", DIR_SEPARATOR, MOD_CONFIG_FILE_NAME);

    auto configChecker = ConfigChecker(gModConfig, MOD_CONFIG_FILE_NAME);

    // Pass 1: Collect metadata from all .cfg files (temporary configs)

    // Main mod.cfg
    Config tempConfig;
    configInit(&tempConfig);
    configRead(&tempConfig, path, true);
    ModInfo mainInfo;
    extractModInfo(&tempConfig, &mainInfo);
    if (mainInfo.name[0] != '\0' && gLoadedModsCount < MAX_LOADED_MODS) {
        strncpy(mainInfo.filePath, path, COMPAT_MAX_PATH - 1);
        mainInfo.filePath[COMPAT_MAX_PATH - 1] = '\0';
        extractDatName(path, mainInfo.datName, MOD_INFO_MAX_NAME);
        gLoadedMods[gLoadedModsCount++] = mainInfo;
    }
    configFree(&tempConfig);

    // mod_*.cfg files
    char searchPattern[COMPAT_MAX_PATH];
    snprintf(searchPattern, sizeof(searchPattern), "data%c%s", DIR_SEPARATOR, "mod_*.cfg");

    char** modFiles = nullptr;
    int modFileCount = fileNameListInit(searchPattern, &modFiles);

    if (modFileCount > 0) {
        for (int i = 0; i < modFileCount; i++) {
            char modPath[COMPAT_MAX_PATH];
            snprintf(modPath, sizeof(modPath), "data%c%s", DIR_SEPARATOR, modFiles[i]);

            Config modTemp;
            configInit(&modTemp);
            configRead(&modTemp, modPath, true);
            ModInfo info;
            extractModInfo(&modTemp, &info);
            if (info.name[0] != '\0' && gLoadedModsCount < MAX_LOADED_MODS) {
                strncpy(info.filePath, modPath, COMPAT_MAX_PATH - 1);
                info.filePath[COMPAT_MAX_PATH - 1] = '\0';
                extractDatName(modPath, info.datName, MOD_INFO_MAX_NAME);
                gLoadedMods[gLoadedModsCount++] = info;
            }
            configFree(&modTemp);
        }
    }

    // Compute stable index for each mod's icon/presence marker
    for (int i = 0; i < gLoadedModsCount; i++) {
        ModInfo* info = &gLoadedMods[i];
        info->icon_index = getModPresenceIndex(info->name);
    }

    // Synchronize the mods folder with the order file and gLoadedMods
    syncModsOrderFile();
    modConfigReadEnabledFlags();

    // Pass 2: Read all .cfg files in the sorted order (main first, then mods)
    configRead(&gModConfig, path, true); // main mod.cfg

    // Read mods in the order they appear in gLoadedMods (already sorted)
    for (int i = 0; i < gLoadedModsCount; i++) {
        const ModInfo* info = &gLoadedMods[i];
        if (info->filePath[0] != '\0' && strcmp(info->filePath, path) != 0) {
            configRead(&gModConfig, info->filePath, true);
        }
    }

    // Free the file list if it was allocated
    if (modFileCount > 0) {
        fileNameListFree(&modFiles, modFileCount);
    }

    // Command line arguments and config checker
    configParseCommandLineArguments(&gModConfig, argc, argv);
    configChecker.check(gModConfig);

    // read mod settings into main settings
    settingsFromModConfig();

    writeModFidsFile();

    gModConfigInitialized = true;

    return true;
}

void modConfigWriteEnabledFlags()
{
    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "mods%cmod_enabled.txt", DIR_SEPARATOR);
    compat_mkdir("mods");
    FILE* f = compat_fopen(path, "w");
    if (!f) return;

    fprintf(f, "# FISSION mod_enabled.txt\n");
    fprintf(f, "# Format: <mod_dat_name>.dat enabled_flag (1 = enabled, 0 = disabled)\n");
    for (int i = 0; i < gLoadedModsCount; i++) {
        fprintf(f, "%s.dat %d\n", gLoadedMods[i].datName, gLoadedMods[i].enabled ? 1 : 0);
    }
    fclose(f);
}

void modConfigReadEnabledFlags()
{
    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "mods%cmod_enabled.txt", DIR_SEPARATOR);
    FILE* f = compat_fopen(path, "r");
    if (!f) return;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char modName[MOD_INFO_MAX_NAME];
        int enabled;
        if (sscanf(line, "%127s %d", modName, &enabled) == 2) {
            // Remove .dat suffix if present
            char* dot = strstr(modName, ".dat");
            if (dot) *dot = '\0';
            // Find the mod in the global list
            for (int i = 0; i < gLoadedModsCount; i++) {
                if (strcmp(gLoadedMods[i].datName, modName) == 0) {
                    gLoadedMods[i].enabled = (enabled != 0);
                    break;
                }
            }
        }
    }
    fclose(f);
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
                    "# Format: <mod_dat_name> enabled (1/0)\n");
    for (int i = 0; i < gLoadedModsCount; i++) {
        pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                        "%s %d\n",
                        gLoadedMods[i].datName,
                        gLoadedMods[i].enabled ? 1 : 0);
    }
    fileWrite(buffer, 1, pos, f);
    fileClose(f);
}

int modConfigCheckSlotEnabledMatchEx(const char* fullPath, char* missingModName, size_t maxSize)
{
    File* f = fileOpen(fullPath, "rb");
    if (!f) return 3; // No file -> old save

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

    // Parse the file into a map of mod name -> enabled flag
    struct SavedMod {
        char name[MOD_INFO_MAX_NAME];
        bool enabled;
    } savedMods[MAX_LOADED_MODS];
    int savedModsCount = 0;

    char* line = buffer;
    while (line && *line) {
        char* end = strchr(line, '\n');
        if (end) *end = '\0';

        if (line[0] != '#' && line[0] != '\0') {
            char modName[MOD_INFO_MAX_NAME];
            int enabledInt;
            if (sscanf(line, "%127s %d", modName, &enabledInt) == 2) {
                if (savedModsCount < MAX_LOADED_MODS) {
                    strncpy(savedMods[savedModsCount].name, modName, MOD_INFO_MAX_NAME - 1);
                    savedMods[savedModsCount].name[MOD_INFO_MAX_NAME - 1] = '\0';
                    savedMods[savedModsCount].enabled = (enabledInt != 0);
                    savedModsCount++;
                }
            }
        }
        line = end ? (end + 1) : nullptr;
    }
    internal_free(buffer);

    // First pass: check for missing mods (saved mod not found in global list)
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
                strncpy(missingModName, savedMods[i].name, maxSize - 1);
                missingModName[maxSize - 1] = '\0';
            }
            return 2; // Missing mod
        }
    }

    // Second pass: check for enabled flag mismatches
    for (int i = 0; i < savedModsCount; i++) {
        for (int j = 0; j < gLoadedModsCount; j++) {
            if (strcmp(gLoadedMods[j].datName, savedMods[i].name) == 0) {
                bool currentEnabled = gLoadedMods[j].enabled;
                if (currentEnabled != savedMods[i].enabled) {
                    return 1; // Mismatch
                }
                break;
            }
        }
    }

    return 0; // Perfect match
}

void modConfigExit()
{
    if (gModConfigInitialized) {
        configFree(&gModConfig);
        gModConfigInitialized = false;
    }
}

} // namespace fallout
