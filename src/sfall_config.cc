#include "sfall_config.h"

#include "db.h"
#include "platform_compat.h"
#include "scan_unimplemented.h"
#include "settings.h"
#include <stdio.h>
#include <string.h>

namespace fallout {

#define DIR_SEPARATOR '/'

bool gModConfigInitialized = false;
Config gModConfig;

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

    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "data%c%s",
        DIR_SEPARATOR, MOD_CONFIG_FILE_NAME);

    auto configChecker = ConfigChecker(gModConfig, MOD_CONFIG_FILE_NAME);

    configRead(&gModConfig, path, true); // true = is Database (dat)

    // Load any mod override files (mod_*.cfg) from the same directory
    char searchPattern[COMPAT_MAX_PATH];
    snprintf(searchPattern, sizeof(searchPattern), "data%c%s",
        DIR_SEPARATOR, "mod_*.cfg");

    char** modFiles = nullptr;
    int modFileCount = fileNameListInit(searchPattern, &modFiles, 0, 0);

    if (modFileCount > 0) {
        // fileNameListInit returns alphabetically sorted list - can't determine priority of 'mods' for now
        for (int i = 0; i < modFileCount; i++) {
            char modPath[COMPAT_MAX_PATH];
            snprintf(modPath, sizeof(modPath), "data%c%s",
                DIR_SEPARATOR, modFiles[i]);

            // Additive reading of config settings from the mod override file
            configRead(&gModConfig, modPath, true);
        }

        fileNameListFree(&modFiles, modFileCount);
    }

    configParseCommandLineArguments(&gModConfig, argc, argv);
    configChecker.check(gModConfig);

// Helper macros for copying values
#define COPY_INT(key, field)                                                 \
    do {                                                                     \
        int val;                                                             \
        if (configGetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, key, &val)) { \
            settings.mod_settings.field = val;                               \
        }                                                                    \
    } while (0)

#define COPY_BOOL(key, field)                                                 \
    do {                                                                      \
        bool val;                                                             \
        if (configGetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, key, &val)) { \
            settings.mod_settings.field = val;                                \
        }                                                                     \
    } while (0)

#define COPY_STRING(key, field)                                                 \
    do {                                                                        \
        char* val = nullptr;                                                    \
        if (configGetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, key, &val)) { \
            settings.mod_settings.field = val;                                  \
        }                                                                       \
    } while (0)

#define COPY_STRING_SCRIPTS(key, field)                                        \
    do {                                                                       \
        char* val = nullptr;                                                   \
        if (configGetString(&gModConfig, MOD_CONFIG_SCRIPTS_KEY, key, &val)) { \
            settings.mod_scripts.field = val;                                  \
        }                                                                      \
    } while (0)

    // --- Copy all ModSettings fields into settings ---
    COPY_INT(MOD_CONFIG_START_YEAR, start_year);
    COPY_INT(MOD_CONFIG_START_MONTH, start_month);
    COPY_INT(MOD_CONFIG_START_DAY, start_day);
    COPY_STRING(MOD_CONFIG_STARTING_MAP_KEY, starting_map);
    COPY_INT(MOD_CONFIG_MOVIE_TIMER_ARTIMER1, movie_timer_artimer1);
    COPY_INT(MOD_CONFIG_MOVIE_TIMER_ARTIMER2, movie_timer_artimer2);
    COPY_INT(MOD_CONFIG_MOVIE_TIMER_ARTIMER3, movie_timer_artimer3);
    COPY_INT(MOD_CONFIG_MOVIE_TIMER_ARTIMER4, movie_timer_artimer4);
    COPY_INT(MOD_CONFIG_PIPBOY_AVAILABLE_AT_GAMESTART, pipboy_available_at_gamestart);
    COPY_INT(MOD_CONFIG_OVERRIDE_CRITICALS_MODE_KEY, override_criticals_mode);
    COPY_STRING(MOD_CONFIG_OVERRIDE_CRITICALS_FILE_KEY, override_criticals_file);
    COPY_STRING(MOD_CONFIG_PREMADE_CHARACTERS_FILE_NAMES_KEY, premade_characters_file_names);
    COPY_STRING(MOD_CONFIG_PREMADE_CHARACTERS_FACE_FIDS_KEY, premade_characters_face_fids);
    COPY_STRING(MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_MALE_KEY, dude_native_look_jumpsuit_male);
    COPY_STRING(MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE_KEY, dude_native_look_jumpsuit_female);
    COPY_STRING(MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_MALE_KEY, dude_native_look_tribal_male);
    COPY_STRING(MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_FEMALE_KEY, dude_native_look_tribal_female);
    COPY_INT(MOD_CONFIG_MAIN_MENU_BIG_FONT_COLOR_KEY, main_menu_big_font_color);
    COPY_INT(MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_X_KEY, main_menu_credits_offset_x);
    COPY_INT(MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_Y_KEY, main_menu_credits_offset_y);
    COPY_INT(MOD_CONFIG_MAIN_MENU_FONT_COLOR_KEY, main_menu_font_color);
    COPY_INT(MOD_CONFIG_MAIN_MENU_OFFSET_X_KEY, main_menu_offset_x);
    COPY_INT(MOD_CONFIG_MAIN_MENU_OFFSET_Y_KEY, main_menu_offset_y);
    COPY_STRING(MOD_CONFIG_VERSION_STRING, version_string);
    COPY_INT(MOD_CONFIG_WORLDMAP_TRAIL_MARKERS, worldmap_trail_markers);

    // Additional recently added fields
    COPY_STRING(MOD_CONFIG_KARMA_FRMS_KEY, karma_frms);
    COPY_STRING(MOD_CONFIG_KARMA_POINTS_KEY, karma_points);
    COPY_INT(MOD_CONFIG_DYNAMITE_MIN_DAMAGE_KEY, dynamite_min_damage);
    COPY_INT(MOD_CONFIG_DYNAMITE_MAX_DAMAGE_KEY, dynamite_max_damage);
    COPY_INT(MOD_CONFIG_PLASTIC_EXPLOSIVE_MIN_DAMAGE_KEY, plastic_explosive_min_damage);
    COPY_INT(MOD_CONFIG_PLASTIC_EXPLOSIVE_MAX_DAMAGE_KEY, plastic_explosive_max_damage);
    COPY_STRING(MOD_CONFIG_CITY_REPUTATION_LIST_KEY, city_reputation_list);
    COPY_STRING(MOD_CONFIG_UNARMED_FILE_KEY, unarmed_file);
    COPY_INT(MOD_CONFIG_DAMAGE_MOD_FORMULA_KEY, damage_mod_formula);
    COPY_BOOL(MOD_CONFIG_BONUS_HTH_DAMAGE_FIX_KEY, bonus_hth_damage_fix);
    COPY_INT(MOD_CONFIG_USE_LOCKPICK_FRM_KEY, use_lockpick_frm);
    COPY_INT(MOD_CONFIG_USE_STEAL_FRM_KEY, use_steal_frm);
    COPY_INT(MOD_CONFIG_USE_TRAPS_FRM_KEY, use_traps_frm);
    COPY_INT(MOD_CONFIG_USE_FIRST_AID_FRM_KEY, use_first_aid_frm);
    COPY_INT(MOD_CONFIG_USE_DOCTOR_FRM_KEY, use_doctor_frm);
    COPY_INT(MOD_CONFIG_USE_SCIENCE_FRM_KEY, use_science_frm);
    COPY_INT(MOD_CONFIG_USE_REPAIR_FRM_KEY, use_repair_frm);
    COPY_BOOL(MOD_CONFIG_SCIENCE_REPAIR_TARGET_TYPE_KEY, science_repair_target_type);
    COPY_BOOL(MOD_CONFIG_GAME_DIALOG_FIX_KEY, game_dialog_fix);
    COPY_STRING(MOD_CONFIG_TWEAKS_FILE_KEY, tweaks_file);
    COPY_BOOL(MOD_CONFIG_GAME_DIALOG_GENDER_WORDS_KEY, game_dialog_gender_words);
    COPY_BOOL(MOD_CONFIG_TOWN_MAP_HOTKEYS_FIX_KEY, town_map_hotkeys_fix);
    COPY_STRING(MOD_CONFIG_EXTRA_MESSAGE_LISTS_KEY, extra_message_lists);
    COPY_STRING(MOD_CONFIG_PATCH_FILE, patch_file);

    // Game Fixes
    COPY_INT(MOD_CONFIG_USE_WALK_DISTANCE, use_walk_distance);

    // --- Copy ModScriptsSettings fields ---
    COPY_STRING_SCRIPTS(MOD_CONFIG_INI_CONFIG_FOLDER, ini_config_folder);
    COPY_STRING_SCRIPTS(MOD_CONFIG_GLOBAL_SCRIPT_PATHS, global_script_paths);

    // Burst
    COPY_BOOL(MOD_CONFIG_BURST_MOD_ENABLED_KEY, burst_mod_enabled);
    COPY_INT(MOD_CONFIG_BURST_MOD_CENTER_MULTIPLIER_KEY, burst_mod_center_multiplier);
    COPY_INT(MOD_CONFIG_BURST_MOD_CENTER_DIVISOR_KEY, burst_mod_center_divisor);
    COPY_INT(MOD_CONFIG_BURST_MOD_TARGET_MULTIPLIER_KEY, burst_mod_target_multiplier);
    COPY_INT(MOD_CONFIG_BURST_MOD_TARGET_DIVISOR_KEY, burst_mod_target_divisor);

    // Interface bar
    COPY_BOOL(MOD_CONFIG_IFACE_BAR_MODE, iface_bar_mode);
    COPY_INT(MOD_CONFIG_IFACE_BAR_WIDTH, iface_bar_width);
    COPY_INT(MOD_CONFIG_IFACE_BAR_SIDE_ART, iface_bar_side_art);
    COPY_BOOL(MOD_CONFIG_IFACE_BAR_SIDES_ORI, iface_bar_sides_ori);

#undef COPY_INT
#undef COPY_BOOL
#undef COPY_STRING
#undef COPY_STRING_SCRIPTS

    gModConfigInitialized = true;

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
