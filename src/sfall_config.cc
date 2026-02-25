#include "sfall_config.h"

#include "platform_compat.h"
#include "scan_unimplemented.h"
#include <stdio.h>
#include <string.h>

namespace fallout {

#define SFALL_CONFIG_FILE_NAME "fallout2.cfg"

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

    // User Gameplay Improvements
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_STRICT_VANILLA, 0);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_AUTO_QUICK_SAVE, 0);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_AUTO_OPEN_DOORS, 0);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_GAPLESS_MUSIC, 0);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_WORLDMAP_TRAIL_MARKERS, 0);
    configSetBool(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_ENHANCED_BARTER, 0);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_SKIP_OPENING_MOVIES_KEY, 0);
    configSetBool(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DISPLAY_KARMA_CHANGES_KEY, false);
    configSetBool(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_REMOVE_CRITICALS_TIME_LIMITS_KEY, false);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PIPBOY_AVAILABLE_AT_GAMESTART, 0);

    // Mod Settings
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_START_YEAR, 2241);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_START_MONTH, 6);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_START_DAY, 24);
    configSetString(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_STARTING_MAP_KEY, "");
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER1, 90);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER2, 180);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER3, 270);
    configSetInt(&gSfallConfig, SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER4, 360);
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
    char* executable = argv[0];
    char* ch = strrchr(executable, '\\');
    if (ch != nullptr) {
        *ch = '\0';
        snprintf(path, sizeof(path), "%s\\%s", executable, SFALL_CONFIG_FILE_NAME);
        *ch = '\\';
    } else {
        strcpy(path, SFALL_CONFIG_FILE_NAME);
    }

    auto configChecker = ConfigChecker(gSfallConfig, "fallout2.cfg");

    configRead(&gSfallConfig, path, false);

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
