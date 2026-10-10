#include "game_config.h"
#include "mod_config.h"

#include <SDL_filesystem.h>
#include <stdio.h>
#include <string.h>

#include "db.h"
#include "main.h"
#include "platform_compat.h"
#include "scan_unimplemented.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

namespace fallout {

static void gameConfigResolvePath(const char* section, const char* key);

// A flag indicating if [gGameConfig] was initialized.
//
// 0x5186D0
bool gGameConfigInitialized = false;

// fission.cfg
//
// 0x58E950
Config gGameConfig;

// NOTE: There are additional 4 bytes following this array at 0x58EA7C, which
// probably means it's size is 264 bytes.
//
// 0x58E978
char gGameConfigFilePath[COMPAT_MAX_PATH];

// Inits main game config.
//
// [isMapper] is a flag indicating whether we're initing config for a main
// game, or a mapper. This value is `false` for the game itself.
//
// [argc] and [argv] are command line arguments. The engine assumes there is
// at least 1 element which is executable path at index 0. There is no
// additional check for [argc], so it will crash if you pass NULL, or an empty
// array into [argv].
//
// The executable path from [argv] is used resolve path to `fission.cfg`,
// which should be in the same folder. This function provide defaults if
// `fission.cfg` is not present, or cannot be read for any reason.
//
// Finally, this function merges key-value pairs from [argv] if any, see
// [configParseCommandLineArguments] for expected format.
//
// 0x444570
bool gameConfigInit(bool isMapper, int argc, char** argv)
{
    if (gGameConfigInitialized) {
        return false;
    }

    if (!configInit(&gGameConfig)) {
        return false;
    }

    // Initialize defaults. Values come from the GAME_CONFIG_DEFAULT_ defines in game_config.h
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_EXECUTABLE_KEY, GAME_CONFIG_DEFAULT_EXECUTABLE);
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_DAT_KEY, GAME_CONFIG_DEFAULT_MASTER_DAT);
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_PATCHES_KEY, GAME_CONFIG_DEFAULT_MASTER_PATCHES);
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_DAT_KEY, GAME_CONFIG_DEFAULT_CRITTER_DAT);
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_PATCHES_KEY, GAME_CONFIG_DEFAULT_CRITTER_PATCHES);
#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE
    // iOS path: resolve fission.dat from the app bundle's Resources directory.
    // The working directory is already Documents at this point, so a relative
    // path would never resolve. SDL_GetBasePath() returns the bundle's
    // Resources path on iOS.
    {
        char* basePath = SDL_GetBasePath();
        if (basePath != NULL) {
            char fissionPath[COMPAT_MAX_PATH];
            snprintf(fissionPath, sizeof(fissionPath), "%sfission.dat", basePath);
            configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FISSION_DAT_KEY, fissionPath);
            SDL_free(basePath);
        } else {
            // Fallback: allow a user-supplied fission.dat in Documents.
            configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FISSION_DAT_KEY, GAME_CONFIG_DEFAULT_FISSION_DAT);
        }
    }
#elif TARGET_OS_MAC
    // macOS path
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FISSION_DAT_KEY, "Fallout-Fission.app/Contents/Resources/fission.dat");
#else
    // Other Apple platform fallback
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FISSION_DAT_KEY, GAME_CONFIG_DEFAULT_FISSION_DAT);
#endif
#else
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FISSION_DAT_KEY, GAME_CONFIG_DEFAULT_FISSION_DAT);
#endif
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FISSION_PATCHES_KEY, GAME_CONFIG_DEFAULT_FISSION_PATCHES);
    configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_LANGUAGE_KEY, GAME_CONFIG_DEFAULT_LANGUAGE);
    configSetBool(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_OVERRIDE_KEY, GAME_CONFIG_DEFAULT_MASTER_OVERRIDE);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_SCROLL_LOCK_KEY, GAME_CONFIG_DEFAULT_SCROLL_LOCK);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_INTERRUPT_WALK_KEY, GAME_CONFIG_DEFAULT_INTERRUPT_WALK);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_ART_CACHE_SIZE_KEY, GAME_CONFIG_DEFAULT_ART_CACHE_SIZE);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_COLOR_CYCLING_KEY, GAME_CONFIG_DEFAULT_COLOR_CYCLING);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CYCLE_SPEED_FACTOR_KEY, GAME_CONFIG_DEFAULT_CYCLE_SPEED_FACTOR);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_HASHING_KEY, GAME_CONFIG_DEFAULT_HASHING);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_SPLASH_KEY, GAME_CONFIG_DEFAULT_SPLASH);
    configSetInt(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FREE_SPACE_KEY, GAME_CONFIG_DEFAULT_FREE_SPACE);

    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_GAME_DIFFICULTY_KEY, GAME_CONFIG_DEFAULT_GAME_DIFFICULTY);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_DIFFICULTY_KEY, GAME_CONFIG_DEFAULT_COMBAT_DIFFICULTY);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_VIOLENCE_LEVEL_KEY, GAME_CONFIG_DEFAULT_VIOLENCE_LEVEL);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TARGET_HIGHLIGHT_KEY, GAME_CONFIG_DEFAULT_TARGET_HIGHLIGHT);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_ITEM_HIGHLIGHT_KEY, GAME_CONFIG_DEFAULT_ITEM_HIGHLIGHT);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_LOOKS_KEY, GAME_CONFIG_DEFAULT_COMBAT_LOOKS);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_MESSAGES_KEY, GAME_CONFIG_DEFAULT_COMBAT_MESSAGES);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_TAUNTS_KEY, GAME_CONFIG_DEFAULT_COMBAT_TAUNTS);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_LANGUAGE_FILTER_KEY, GAME_CONFIG_DEFAULT_LANGUAGE_FILTER);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_RUNNING_KEY, GAME_CONFIG_DEFAULT_RUNNING);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_SUBTITLES_KEY, GAME_CONFIG_DEFAULT_SUBTITLES);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_SPEED_KEY, GAME_CONFIG_DEFAULT_COMBAT_SPEED);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_PLAYER_SPEED_KEY, GAME_CONFIG_DEFAULT_PLAYER_SPEED);
    configSetDouble(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TEXT_BASE_DELAY_KEY, GAME_CONFIG_DEFAULT_TEXT_BASE_DELAY);
    configSetDouble(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TEXT_LINE_DELAY_KEY, GAME_CONFIG_DEFAULT_TEXT_LINE_DELAY);
    configSetDouble(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_BRIGHTNESS_KEY, GAME_CONFIG_DEFAULT_BRIGHTNESS);
    configSetDouble(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_MOUSE_SENSITIVITY_KEY, GAME_CONFIG_DEFAULT_MOUSE_SENSITIVITY);
    configSetInt(&gGameConfig, GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_RUNNING_BURNING_GUY_KEY, GAME_CONFIG_DEFAULT_RUNNING_BURNING_GUY);

    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_INITIALIZE_KEY, GAME_CONFIG_DEFAULT_SOUND_INITIALIZE);
    configSetBool(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_DEFAULT_SOUND_DEBUG);
    configSetBool(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DEBUG_SFXC_KEY, GAME_CONFIG_DEFAULT_SOUND_DEBUG_SFXC);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DEVICE_KEY, GAME_CONFIG_DEFAULT_SOUND_DEVICE);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_PORT_KEY, GAME_CONFIG_DEFAULT_SOUND_PORT);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_IRQ_KEY, GAME_CONFIG_DEFAULT_SOUND_IRQ);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DMA_KEY, GAME_CONFIG_DEFAULT_SOUND_DMA);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SOUNDS_KEY, GAME_CONFIG_DEFAULT_SOUND_SOUNDS);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_KEY, GAME_CONFIG_DEFAULT_SOUND_MUSIC);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SPEECH_KEY, GAME_CONFIG_DEFAULT_SOUND_SPEECH);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MASTER_VOLUME_KEY, GAME_CONFIG_DEFAULT_MASTER_VOLUME);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_VOLUME_KEY, GAME_CONFIG_DEFAULT_MUSIC_VOLUME);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SNDFX_VOLUME_KEY, GAME_CONFIG_DEFAULT_SNDFX_VOLUME);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SPEECH_VOLUME_KEY, GAME_CONFIG_DEFAULT_SPEECH_VOLUME);
    configSetInt(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_CACHE_SIZE_KEY, GAME_CONFIG_DEFAULT_SOUND_CACHE_SIZE);
    configSetString(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH1_KEY, GAME_CONFIG_DEFAULT_MUSIC_PATH1);
    configSetString(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH2_KEY, GAME_CONFIG_DEFAULT_MUSIC_PATH2);

    configSetString(&gGameConfig, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_MODE_KEY, GAME_CONFIG_DEFAULT_DEBUG_MODE);
    configSetInt(&gGameConfig, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_TILE_NUM_KEY, GAME_CONFIG_DEFAULT_SHOW_TILE_NUM);
    configSetInt(&gGameConfig, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_SCRIPT_MESSAGES_KEY, GAME_CONFIG_DEFAULT_SHOW_SCRIPT_MESSAGES);
    configSetInt(&gGameConfig, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_LOAD_INFO_KEY, GAME_CONFIG_DEFAULT_SHOW_LOAD_INFO);
    configSetInt(&gGameConfig, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_OUTPUT_MAP_DATA_INFO_KEY, GAME_CONFIG_DEFAULT_OUTPUT_MAP_DATA_INFO);
    configSetInt(&gGameConfig, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_WRITE_OFFSETS, GAME_CONFIG_DEFAULT_WRITE_OFFSETS);
    configSetString(&gGameConfig, GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_EXPORT_PNG, GAME_CONFIG_DEFAULT_EXPORT_PNG);

    configSetInt(&gGameConfig, GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_GAME_WIDTH, GAME_CONFIG_DEFAULT_GAME_WIDTH);
    configSetInt(&gGameConfig, GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_GAME_HEIGHT, GAME_CONFIG_DEFAULT_GAME_HEIGHT);
    configSetBool(&gGameConfig, GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_FULLSCREEN, GAME_CONFIG_DEFAULT_FULLSCREEN);
    configSetBool(&gGameConfig, GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_STRETCH_ENABLED, GAME_CONFIG_DEFAULT_STRETCH_ENABLED);
    configSetBool(&gGameConfig, GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_PRESERVE_ASPECT, GAME_CONFIG_DEFAULT_PRESERVE_ASPECT);
    configSetBool(&gGameConfig, GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_HIGH_QUALITY, GAME_CONFIG_DEFAULT_HIGH_QUALITY);
    configSetBool(&gGameConfig, GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_ENABLE_HIRES_STENCIL, GAME_CONFIG_DEFAULT_ENABLE_HIRES_STENCIL);
    configSetBool(&gGameConfig, GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_WIDESCREEN, GAME_CONFIG_DEFAULT_WIDESCREEN);
    configSetBool(&gGameConfig, GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_SQUARE_PIXELS, GAME_CONFIG_DEFAULT_SQUARE_PIXELS);
    configSetInt(&gGameConfig, GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_PLAY_AREA, GAME_CONFIG_DEFAULT_PLAY_AREA);
    configSetString(&gGameConfig, GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_VARIANT_SUFFIX, GAME_CONFIG_DEFAULT_VARIANT_SUFFIX);

    configSetInt(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_STRICT_VANILLA, GAME_CONFIG_DEFAULT_STRICT_VANILLA);
    configSetInt(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_AUTO_QUICK_SAVE, GAME_CONFIG_DEFAULT_AUTO_QUICK_SAVE);
    configSetInt(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_AUTO_OPEN_DOORS, GAME_CONFIG_DEFAULT_AUTO_OPEN_DOORS);
    configSetInt(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_GAPLESS_MUSIC, GAME_CONFIG_DEFAULT_GAPLESS_MUSIC);
    configSetBool(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_ENHANCED_BARTER, GAME_CONFIG_DEFAULT_ENHANCED_BARTER);
    configSetInt(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_NUMBERS_IS_DIALOG_KEY, GAME_CONFIG_DEFAULT_NUMBERS_IS_DIALOG);
    configSetInt(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_DISPLAY_BONUS_DAMAGE_KEY, GAME_CONFIG_DEFAULT_DISPLAY_BONUS_DAMAGE);
    configSetInt(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_EXPLOSION_EMITS_LIGHT_KEY, GAME_CONFIG_DEFAULT_EXPLOSION_EMITS_LIGHT);
    configSetBool(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_REMOVE_CRITICALS_TIME_LIMITS_KEY, GAME_CONFIG_DEFAULT_REMOVE_CRITICALS_TIME_LIMITS);
    configSetBool(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_DISPLAY_KARMA_CHANGES_KEY, GAME_CONFIG_DEFAULT_DISPLAY_KARMA_CHANGES);
    configSetInt(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_SKIP_OPENING_MOVIES_KEY, GAME_CONFIG_DEFAULT_SKIP_OPENING_MOVIES);
    configSetBool(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_MASS_HIGHLIGHT, GAME_CONFIG_DEFAULT_MASS_HIGHLIGHT);
    configSetBool(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_GAME_SPEED, GAME_CONFIG_DEFAULT_GAME_SPEED);
    configSetBool(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_AUTO_PUSH, GAME_CONFIG_DEFAULT_AUTO_PUSH);
    configSetBool(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_MINIMAP, GAME_CONFIG_DEFAULT_MINIMAP);
    configSetInt(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_MULTI_COLUMN_INVENTORY, GAME_CONFIG_DEFAULT_MULTI_COLUMN_INVENTORY);
    configSetBool(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_NPC_ARMOR, GAME_CONFIG_DEFAULT_NPC_ARMOR);
    configSetBool(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_GREEN_MONOCHROME, GAME_CONFIG_DEFAULT_GREEN_MONOCHROME);
    configSetInt(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_INVENTORY_FILTER, GAME_CONFIG_DEFAULT_INVENTORY_FILTER);
    configSetBool(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_DISPLAY_WEIGHT, GAME_CONFIG_DEFAULT_DISPLAY_WEIGHT);
    configSetBool(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_COMPANION_INVENTORY, GAME_CONFIG_DEFAULT_COMPANION_INVENTORY);
    configSetBool(&gGameConfig, GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_VOCK_FEATURES_KEY, GAME_CONFIG_DEFAULT_VOCK_FEATURES);

    if (isMapper) {
        configSetString(&gGameConfig, GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_EXECUTABLE_KEY, "mapper");
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_OVERRIDE_LIBRARIAN_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_LIBRARIAN_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_USE_ART_NOT_PROTOS_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_REBUILD_PROTOS_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_FIX_MAP_OBJECTS_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_FIX_MAP_INVENTORY_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_IGNORE_REBUILD_ERRORS_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SHOW_PID_NUMBERS_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SAVE_TEXT_MAPS_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_RUN_MAPPER_AS_GAME_KEY, 0);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_DEFAULT_F8_AS_GAME_KEY, 1);
        configSetInt(&gGameConfig, GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SORT_SCRIPT_LIST_KEY, 0);
    }

    // CE: Detect alternative default music directory.
    char alternativeMusicPath[COMPAT_MAX_PATH];
    strcpy(alternativeMusicPath, "data\\sound\\music\\*.acm");
    compat_windows_path_to_native(alternativeMusicPath);
    compat_resolve_path(alternativeMusicPath);

    char** acms;
    int acmsLength = fileNameListInit(alternativeMusicPath, &acms);
    if (acmsLength != -1) {
        if (acmsLength > 0) {
            configSetString(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH1_KEY, "data\\sound\\music\\");
            configSetString(&gGameConfig, GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH2_KEY, "data\\sound\\music\\");
        }
        fileNameListFree(&acms, 0);
    }

    const char* configFileName = DEFAULT_GAME_CONFIG_FILE_NAME;

    // Make `fission.cfg` file path.
    char* executable = argv[0];
    char* ch = strrchr(executable, '\\');
    if (ch != nullptr) {
        *ch = '\0';
        if (isMapper) {
            snprintf(gGameConfigFilePath,
                sizeof(gGameConfigFilePath),
                "%s\\%s",
                executable,
                MAPPER_CONFIG_FILE_NAME);
        } else {
            snprintf(gGameConfigFilePath,
                sizeof(gGameConfigFilePath),
                "%s\\%s",
                executable,
                configFileName);
        }
        *ch = '\\';
    } else {
        if (isMapper) {
            strcpy(gGameConfigFilePath, MAPPER_CONFIG_FILE_NAME);
        } else {
            strcpy(gGameConfigFilePath, configFileName);
        }
    }

    auto configChecker = ConfigChecker(gGameConfig, gGameConfigFilePath);
    // Read contents of `fission.cfg` into config. The values from the file
    // will override the defaults above.
    configRead(&gGameConfig, gGameConfigFilePath, false);
    configChecker.check(gGameConfig);

    // Add key-values from command line, which overrides both defaults and
    // whatever was loaded from `fission.cfg`.
    configParseCommandLineArguments(&gGameConfig, argc, argv);

    // CE: Normalize and resolve asset bundle paths.
    gameConfigResolvePath(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_DAT_KEY);
    gameConfigResolvePath(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_PATCHES_KEY);
    gameConfigResolvePath(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_DAT_KEY);
    gameConfigResolvePath(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_PATCHES_KEY);
    gameConfigResolvePath(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_PATCHES_KEY);
    gameConfigResolvePath(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH1_KEY);
    gameConfigResolvePath(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH2_KEY);

    gGameConfigInitialized = true;

    return true;
}

#if defined(__EMSCRIPTEN__)
// clang-format off
EM_ASYNC_JS(void, do_save_idbfs_gameconfig, (), {
    await new Promise((resolve, reject) => FS.syncfs(err => err ? reject(err) : resolve()))
});
// clang-format on
#endif

// Saves game config into `fission.cfg`.
//
// 0x444C14
bool gameConfigSave()
{
    if (!gGameConfigInitialized) {
        return false;
    }

    if (!configWrite(&gGameConfig, gGameConfigFilePath, false)) {
        return false;
    }

#if defined(__EMSCRIPTEN__)
    do_save_idbfs_gameconfig();
#endif

    return true;
}

// Frees game config, optionally saving it.
//
// 0x444C3C
bool gameConfigExit(bool shouldSave)
{
    if (!gGameConfigInitialized) {
        return false;
    }

    bool result = true;

    if (shouldSave) {
        if (!configWrite(&gGameConfig, gGameConfigFilePath, false)) {
            result = false;
        }
    }

    configFree(&gGameConfig);

    gGameConfigInitialized = false;

    return result;
}

static void gameConfigResolvePath(const char* section, const char* key)
{
    char* originalPath;
    if (!configGetString(&gGameConfig, section, key, &originalPath)) {
        return; // Key doesn't exist, nothing to resolve
    }

    // Work on a temporary buffer
    char resolvedPath[COMPAT_MAX_PATH];
    if (strlen(originalPath) >= COMPAT_MAX_PATH) {
        // Path too long - truncate
        strncpy(resolvedPath, originalPath, COMPAT_MAX_PATH - 1);
        resolvedPath[COMPAT_MAX_PATH - 1] = '\0';
    } else {
        strcpy(resolvedPath, originalPath);
    }

    compat_windows_path_to_native(resolvedPath);
    compat_resolve_path(resolvedPath);

    // Only write back if the path actually changed
    if (strcmp(originalPath, resolvedPath) != 0) {
        configSetString(&gGameConfig, section, key, resolvedPath);
    }
}

} // namespace fallout
