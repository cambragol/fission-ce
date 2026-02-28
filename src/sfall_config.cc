#include "sfall_config.h"

#include "db.h"
#include "platform_compat.h"
#include "scan_unimplemented.h"
#include <stdio.h>
#include <string.h>

namespace fallout {

#define DIR_SEPARATOR '/'

bool gSfallConfigInitialized = false;
Config gSfallConfig;

bool sfallConfigInit(int argc, char** argv)
{
    if (gSfallConfigInitialized) {
        return false;
    }

    if (!configInit(&gSfallConfig)) {
        return false;
    }

    // Initialize defaults – grouped by category

    // Mod Settings
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_START_YEAR, 2241);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_START_MONTH, 6);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_START_DAY, 24);
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_STARTING_MAP_KEY, "");
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER1, 90);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER2, 180);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER3, 270);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER4, 360);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PIPBOY_AVAILABLE_AT_GAMESTART, 0);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_OVERRIDE_CRITICALS_MODE_KEY, 2);
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_OVERRIDE_CRITICALS_FILE_KEY, "");
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PREMADE_CHARACTERS_FILE_NAMES_KEY, "");
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PREMADE_CHARACTERS_FACE_FIDS_KEY, "");
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_MALE_KEY, "");
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE_KEY, "");
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_MALE_KEY, "");
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_FEMALE_KEY, "");
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_BIG_FONT_COLOR_KEY, 0);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_CREDITS_OFFSET_X_KEY, 0);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_CREDITS_OFFSET_Y_KEY, 0);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_FONT_COLOR_KEY, 0);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_OFFSET_X_KEY, 0);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_OFFSET_Y_KEY, 0);
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_VERSION_STRING, "");
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_WORLDMAP_TRAIL_MARKERS, 0);

    // Game Fixes
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_WALK_DISTANCE, 5);

    // Files and Paths
    configSetString(&gSfallConfig, SFALL_CONFIG_SCRIPTS_KEY, SFALL_CONFIG_INI_CONFIG_FOLDER, "");
    configSetString(&gSfallConfig, SFALL_CONFIG_SCRIPTS_KEY, SFALL_CONFIG_GLOBAL_SCRIPT_PATHS, "");
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_CONFIG_FILE, "");
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PATCH_FILE, "");
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_EXTRA_MESSAGE_LISTS_KEY, "");
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BOOKS_FILE_KEY, "");
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_ELEVATORS_FILE_KEY, "");
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_CONSOLE_OUTPUT_FILE_KEY, "");

    // Mods (Burst)
    configSetBool(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_ENABLED_KEY, false);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_CENTER_MULTIPLIER_KEY, SFALL_CONFIG_BURST_MOD_DEFAULT_CENTER_MULTIPLIER);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_CENTER_DIVISOR_KEY, SFALL_CONFIG_BURST_MOD_DEFAULT_CENTER_DIVISOR);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_TARGET_MULTIPLIER_KEY, SFALL_CONFIG_BURST_MOD_DEFAULT_TARGET_MULTIPLIER);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_TARGET_DIVISOR_KEY, SFALL_CONFIG_BURST_MOD_DEFAULT_TARGET_DIVISOR);

    // Others (Interface Bar, etc.)
    configSetBool(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_IFACE_BAR_MODE, true);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_IFACE_BAR_WIDTH, 800);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_IFACE_BAR_SIDE_ART, 0);
    configSetBool(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_IFACE_BAR_SIDES_ORI, false);

    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "data%c%s",
        DIR_SEPARATOR, SFALL_CONFIG_FILE_NAME);

    auto configChecker = ConfigChecker(gSfallConfig, SFALL_CONFIG_FILE_NAME);

    configRead(&gSfallConfig, path, true); // true = is Database (dat)

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
            configRead(&gSfallConfig, modPath, true);
        }

        fileNameListFree(&modFiles, modFileCount);
    }

    configParseCommandLineArguments(&gSfallConfig, argc, argv);
    configChecker.check(gSfallConfig);
    gSfallConfigInitialized = true;

    return true;
}

void sfallConfigExit()
{
    if (gSfallConfigInitialized) {
        configFree(&gSfallConfig);
        gSfallConfigInitialized = false;
    }
}

} // namespace fallout
