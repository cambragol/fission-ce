#include "sfall_config.h"

#include "db.h"
#include "platform_compat.h"
#include "scan_unimplemented.h"
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
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_YEAR, 2241);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_MONTH, 6);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_DAY, 24);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_STARTING_MAP_KEY, "");
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER1, 90);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER2, 180);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER3, 270);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER4, 360);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PIPBOY_AVAILABLE_AT_GAMESTART, 0);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_OVERRIDE_CRITICALS_MODE_KEY, 2);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_OVERRIDE_CRITICALS_FILE_KEY, "");
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PREMADE_CHARACTERS_FILE_NAMES_KEY, "");
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PREMADE_CHARACTERS_FACE_FIDS_KEY, "");
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_MALE_KEY, "");
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE_KEY, "");
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_MALE_KEY, "");
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_FEMALE_KEY, "");
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_BIG_FONT_COLOR_KEY, 0);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_X_KEY, 0);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_Y_KEY, 0);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_FONT_COLOR_KEY, 0);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_OFFSET_X_KEY, 0);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_OFFSET_Y_KEY, 0);
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_VERSION_STRING, "");
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_WORLDMAP_TRAIL_MARKERS, 0);

    // Game Fixes
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_WALK_DISTANCE, 5);

    // Files and Paths
    configSetString(&gModConfig, MOD_CONFIG_SCRIPTS_KEY, MOD_CONFIG_INI_CONFIG_FOLDER, "");
    configSetString(&gModConfig, MOD_CONFIG_SCRIPTS_KEY, MOD_CONFIG_GLOBAL_SCRIPT_PATHS, "");
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PATCH_FILE, "");
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_EXTRA_MESSAGE_LISTS_KEY, "");
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BOOKS_FILE_KEY, "");
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_ELEVATORS_FILE_KEY, "");
    configSetString(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_CONSOLE_OUTPUT_FILE_KEY, "");

    // Mods (Burst)
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_ENABLED_KEY, false);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_CENTER_MULTIPLIER_KEY, MOD_CONFIG_BURST_MOD_DEFAULT_CENTER_MULTIPLIER);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_CENTER_DIVISOR_KEY, MOD_CONFIG_BURST_MOD_DEFAULT_CENTER_DIVISOR);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_TARGET_MULTIPLIER_KEY, MOD_CONFIG_BURST_MOD_DEFAULT_TARGET_MULTIPLIER);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_TARGET_DIVISOR_KEY, MOD_CONFIG_BURST_MOD_DEFAULT_TARGET_DIVISOR);

    // Others (Interface Bar, etc.)
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_MODE, true);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_WIDTH, 800);
    configSetInt(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_SIDE_ART, 0);
    configSetBool(&gModConfig, MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_SIDES_ORI, false);

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
