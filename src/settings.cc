#include "settings.h"

namespace fallout {

static void settingsFromConfig();
static void settingsToConfig();
static void settingsRead(const char* section, const char* key, std::string& value);
static void settingsRead(const char* section, const char* key, int& value);
static void settingsRead(const char* section, const char* key, bool& value);
static void settingsRead(const char* section, const char* key, double& value);
static void modSettingsRead(const char* section, const char* key, std::string& value);
static void modSettingsRead(const char* section, const char* key, int& value);
static void modSettingsRead(const char* section, const char* key, bool& value);
static void settingsWrite(const char* section, const char* key, std::string& value);
static void settingsWrite(const char* section, const char* key, int& value);
static void settingsWrite(const char* section, const char* key, bool& value);
static void settingsWrite(const char* section, const char* key, double& value);

Settings settings;

bool settingsInit(bool isMapper, int argc, char** argv)
{
    if (!gameConfigInit(isMapper, argc, argv)) {
        return false;
    }

    settingsFromConfig();

    return true;
}

bool settingsSave()
{
    settingsToConfig();
    return gameConfigSave();
}

bool settingsExit(bool shouldSave)
{
    if (shouldSave) {
        settingsToConfig();
    }

    return gameConfigExit(shouldSave);
}

static void settingsFromConfig()
{
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_EXECUTABLE_KEY, settings.system.executable);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_DAT_KEY, settings.system.master_dat_path);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_PATCHES_KEY, settings.system.master_patches_path);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_DAT_KEY, settings.system.critter_dat_path);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_PATCHES_KEY, settings.system.critter_patches_path);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FISSION_DAT_KEY, settings.system.fission_dat_path);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FISSION_PATCHES_KEY, settings.system.fission_patches_path);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_LANGUAGE_KEY, settings.system.language);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_OVERRIDE_KEY, settings.system.master_override);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_SCROLL_LOCK_KEY, settings.system.scroll_lock);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_INTERRUPT_WALK_KEY, settings.system.interrupt_walk);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_ART_CACHE_SIZE_KEY, settings.system.art_cache_size);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_COLOR_CYCLING_KEY, settings.system.color_cycling);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CYCLE_SPEED_FACTOR_KEY, settings.system.cycle_speed_factor);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_HASHING_KEY, settings.system.hashing);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_SPLASH_KEY, settings.system.splash);
    settingsRead(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FREE_SPACE_KEY, settings.system.free_space);

    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_GAME_DIFFICULTY_KEY, settings.preferences.game_difficulty);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_DIFFICULTY_KEY, settings.preferences.combat_difficulty);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_VIOLENCE_LEVEL_KEY, settings.preferences.violence_level);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TARGET_HIGHLIGHT_KEY, settings.preferences.target_highlight);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_ITEM_HIGHLIGHT_KEY, settings.preferences.item_highlight);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_LOOKS_KEY, settings.preferences.combat_looks);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_MESSAGES_KEY, settings.preferences.combat_messages);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_TAUNTS_KEY, settings.preferences.combat_taunts);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_LANGUAGE_FILTER_KEY, settings.preferences.language_filter);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_RUNNING_KEY, settings.preferences.running);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_SUBTITLES_KEY, settings.preferences.subtitles);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_SPEED_KEY, settings.preferences.combat_speed);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_PLAYER_SPEED_KEY, settings.preferences.player_speedup);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TEXT_BASE_DELAY_KEY, settings.preferences.text_base_delay);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TEXT_LINE_DELAY_KEY, settings.preferences.text_line_delay);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_BRIGHTNESS_KEY, settings.preferences.brightness);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_MOUSE_SENSITIVITY_KEY, settings.preferences.mouse_sensitivity);
    settingsRead(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_RUNNING_BURNING_GUY_KEY, settings.preferences.running_burning_guy);

    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_INITIALIZE_KEY, settings.sound.initialize);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DEBUG_KEY, settings.sound.debug);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DEBUG_SFXC_KEY, settings.sound.debug_sfxc);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DEVICE_KEY, settings.sound.device);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_PORT_KEY, settings.sound.port);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_IRQ_KEY, settings.sound.irq);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DMA_KEY, settings.sound.dma);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SOUNDS_KEY, settings.sound.sounds);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_KEY, settings.sound.music);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SPEECH_KEY, settings.sound.speech);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MASTER_VOLUME_KEY, settings.sound.master_volume);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_VOLUME_KEY, settings.sound.music_volume);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SNDFX_VOLUME_KEY, settings.sound.sndfx_volume);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SPEECH_VOLUME_KEY, settings.sound.speech_volume);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_CACHE_SIZE_KEY, settings.sound.cache_size);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH1_KEY, settings.sound.music_path1);
    settingsRead(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH2_KEY, settings.sound.music_path2);

    settingsRead(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_MODE_KEY, settings.debug.mode);
    settingsRead(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_TILE_NUM_KEY, settings.debug.show_tile_num);
    settingsRead(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_SCRIPT_MESSAGES_KEY, settings.debug.show_script_messages);
    settingsRead(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_LOAD_INFO_KEY, settings.debug.show_load_info);
    settingsRead(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_OUTPUT_MAP_DATA_INFO_KEY, settings.debug.output_map_data_info);
    settingsRead(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_WRITE_OFFSETS, settings.debug.write_offsets);

    settingsRead(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_GAME_WIDTH, settings.graphics.game_width);
    settingsRead(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_GAME_HEIGHT, settings.graphics.game_height);
    settingsRead(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_FULLSCREEN, settings.graphics.fullscreen);
    settingsRead(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_STRETCH_ENABLED, settings.graphics.stretch_enabled);
    settingsRead(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_PRESERVE_ASPECT, settings.graphics.preserve_aspect);
    settingsRead(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_HIGH_QUALITY, settings.graphics.high_quality);
    settingsRead(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_ENABLE_HIRES_STENCIL, settings.graphics.highres_stencil);
    settingsRead(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_WIDESCREEN, settings.graphics.widescreen);
    settingsRead(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_SQUARE_PIXELS, settings.graphics.square_pixels);
    settingsRead(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_PLAY_AREA, settings.graphics.play_area);
    settingsRead(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_VARIANT_SUFFIX, settings.graphics.widescreen_variant_suffix);

    settingsRead(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_OVERRIDE_LIBRARIAN_KEY, settings.mapper.override_librarian);
    settingsRead(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_LIBRARIAN_KEY, settings.mapper.librarian);
    settingsRead(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_USE_ART_NOT_PROTOS_KEY, settings.mapper.user_art_not_protos);
    settingsRead(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_REBUILD_PROTOS_KEY, settings.mapper.rebuild_protos);
    settingsRead(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_FIX_MAP_OBJECTS_KEY, settings.mapper.fix_map_objects);
    settingsRead(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_FIX_MAP_INVENTORY_KEY, settings.mapper.fix_map_inventory);
    settingsRead(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_IGNORE_REBUILD_ERRORS_KEY, settings.mapper.rebuild_protos);
    settingsRead(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SHOW_PID_NUMBERS_KEY, settings.mapper.show_pid_numbers);
    settingsRead(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SAVE_TEXT_MAPS_KEY, settings.mapper.save_text_maps);
    settingsRead(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_RUN_MAPPER_AS_GAME_KEY, settings.mapper.run_mapper_as_game);
    settingsRead(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_DEFAULT_F8_AS_GAME_KEY, settings.mapper.default_f8_as_game);
    settingsRead(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SORT_SCRIPT_LIST_KEY, settings.mapper.sort_script_list);

    // User Gameplay Enhancements
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_STRICT_VANILLA, settings.enhancements.strict_vanilla);
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_AUTO_QUICK_SAVE, settings.enhancements.auto_quick_save);
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_AUTO_OPEN_DOORS, settings.enhancements.auto_open_doors);
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_GAPLESS_MUSIC, settings.enhancements.gapless_music);
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_ENHANCED_BARTER, settings.enhancements.enhanced_barter);
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_NUMBERS_IS_DIALOG_KEY, settings.enhancements.numbers_is_dialog);
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_DISPLAY_BONUS_DAMAGE_KEY, settings.enhancements.display_bonus_damage);
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_EXPLOSION_EMITS_LIGHT_KEY, settings.enhancements.explosion_emits_light);
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_REMOVE_CRITICALS_TIME_LIMITS_KEY, settings.enhancements.remove_criticals_time_limits);
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_DISPLAY_KARMA_CHANGES_KEY, settings.enhancements.display_karma_changes);
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_SKIP_OPENING_MOVIES_KEY, settings.enhancements.skip_opening_movies);
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_MASS_HIGHLIGHT, settings.enhancements.mass_highlight);
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_GAME_SPEED, settings.enhancements.game_speed);
    settingsRead(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_MINIMAP, settings.enhancements.minimap);
}

// read in modConfig setting (no writing back, just reading)
void settingsFromModConfig()
{
    // Mod Settings
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_MALE_KEY, settings.mod_settings.dude_native_look_jumpsuit_male);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE_KEY, settings.mod_settings.dude_native_look_jumpsuit_female);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_MALE_KEY, settings.mod_settings.dude_native_look_tribal_male);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_FEMALE_KEY, settings.mod_settings.dude_native_look_tribal_female);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_YEAR, settings.mod_settings.start_year);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_MONTH, settings.mod_settings.start_month);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_DAY, settings.mod_settings.start_day);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_BIG_FONT_COLOR_KEY, settings.mod_settings.main_menu_big_font_color);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_X_KEY, settings.mod_settings.main_menu_credits_offset_x);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_Y_KEY, settings.mod_settings.main_menu_credits_offset_y);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_FONT_COLOR_KEY, settings.mod_settings.main_menu_font_color);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_OFFSET_X_KEY, settings.mod_settings.main_menu_offset_x);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_OFFSET_Y_KEY, settings.mod_settings.main_menu_offset_y);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_STARTING_MAP_KEY, settings.mod_settings.starting_map);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_KARMA_FRMS_KEY, settings.mod_settings.karma_frms);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_KARMA_POINTS_KEY, settings.mod_settings.karma_points);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_OVERRIDE_CRITICALS_MODE_KEY, settings.mod_settings.override_criticals_mode);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_OVERRIDE_CRITICALS_FILE_KEY, settings.mod_settings.override_criticals_file);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PREMADE_CHARACTERS_FILE_NAMES_KEY, settings.mod_settings.premade_characters_file_names);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PREMADE_CHARACTERS_FACE_FIDS_KEY, settings.mod_settings.premade_characters_face_fids);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DYNAMITE_MIN_DAMAGE_KEY, settings.mod_settings.dynamite_min_damage);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DYNAMITE_MAX_DAMAGE_KEY, settings.mod_settings.dynamite_max_damage);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PLASTIC_EXPLOSIVE_MIN_DAMAGE_KEY, settings.mod_settings.plastic_explosive_min_damage);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PLASTIC_EXPLOSIVE_MAX_DAMAGE_KEY, settings.mod_settings.plastic_explosive_max_damage);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER1, settings.mod_settings.movie_timer_artimer1);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER2, settings.mod_settings.movie_timer_artimer2);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER3, settings.mod_settings.movie_timer_artimer3);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER4, settings.mod_settings.movie_timer_artimer4);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PIPBOY_AVAILABLE_AT_GAMESTART, settings.mod_settings.pipboy_available_at_gamestart);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DAMAGE_MOD_FORMULA_KEY, settings.mod_settings.damage_mod_formula);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_LOCKPICK_FRM_KEY, settings.mod_settings.use_lockpick_frm);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_STEAL_FRM_KEY, settings.mod_settings.use_steal_frm);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_TRAPS_FRM_KEY, settings.mod_settings.use_traps_frm);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_FIRST_AID_FRM_KEY, settings.mod_settings.use_first_aid_frm);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_DOCTOR_FRM_KEY, settings.mod_settings.use_doctor_frm);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_SCIENCE_FRM_KEY, settings.mod_settings.use_science_frm);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_REPAIR_FRM_KEY, settings.mod_settings.use_repair_frm);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_SCIENCE_REPAIR_TARGET_TYPE_KEY, settings.mod_settings.science_repair_target_type);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_GAME_DIALOG_GENDER_WORDS_KEY, settings.mod_settings.game_dialog_gender_words);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_WORLDMAP_TRAIL_MARKERS, settings.mod_settings.worldmap_trail_markers);

    // Game Fixes
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_WALK_DISTANCE, settings.mod_settings.use_walk_distance);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_TOWN_MAP_HOTKEYS_FIX_KEY, settings.mod_settings.town_map_hotkeys_fix);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_GAME_DIALOG_FIX_KEY, settings.mod_settings.game_dialog_fix);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BONUS_HTH_DAMAGE_FIX_KEY, settings.mod_settings.bonus_hth_damage_fix);

    // Files and Paths
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BOOKS_FILE_KEY, settings.mod_settings.books_file);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_ELEVATORS_FILE_KEY, settings.mod_settings.elevators_file);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_CONSOLE_OUTPUT_FILE_KEY, settings.mod_settings.console_output_file);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_CITY_REPUTATION_LIST_KEY, settings.mod_settings.city_reputation_list);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_UNARMED_FILE_KEY, settings.mod_settings.unarmed_file);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_TWEAKS_FILE_KEY, settings.mod_settings.tweaks_file);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_EXTRA_MESSAGE_LISTS_KEY, settings.mod_settings.extra_message_lists);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PATCH_FILE, settings.mod_settings.patch_file);
    modSettingsRead(MOD_CONFIG_SCRIPTS_KEY, MOD_CONFIG_INI_CONFIG_FOLDER, settings.mod_scripts.ini_config_folder);
    modSettingsRead(MOD_CONFIG_SCRIPTS_KEY, MOD_CONFIG_GLOBAL_SCRIPT_PATHS, settings.mod_scripts.global_script_paths);

    // Mods (Burst)
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_ENABLED_KEY, settings.mod_settings.burst_mod_enabled);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_CENTER_MULTIPLIER_KEY, settings.mod_settings.burst_mod_center_multiplier);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_CENTER_DIVISOR_KEY, settings.mod_settings.burst_mod_center_divisor);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_TARGET_MULTIPLIER_KEY, settings.mod_settings.burst_mod_target_multiplier);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_TARGET_DIVISOR_KEY, settings.mod_settings.burst_mod_target_divisor);

    // Others
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_VERSION_STRING, settings.mod_settings.version_string);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_MODE, settings.mod_settings.iface_bar_mode);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_WIDTH, settings.mod_settings.iface_bar_width);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_SIDE_ART, settings.mod_settings.iface_bar_side_art);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_SIDES_ORI, settings.mod_settings.iface_bar_sides_ori);
    modSettingsRead(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_INVENTORY_COLUMNS_KEY, settings.mod_settings.inventory_columns);
}

static void settingsToConfig()
{
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_EXECUTABLE_KEY, settings.system.executable);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_DAT_KEY, settings.system.master_dat_path);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_PATCHES_KEY, settings.system.master_patches_path);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_DAT_KEY, settings.system.critter_dat_path);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CRITTER_PATCHES_KEY, settings.system.critter_patches_path);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FISSION_DAT_KEY, settings.system.fission_dat_path);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FISSION_PATCHES_KEY, settings.system.fission_patches_path);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_LANGUAGE_KEY, settings.system.language);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_OVERRIDE_KEY, settings.system.master_override);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_SCROLL_LOCK_KEY, settings.system.scroll_lock);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_INTERRUPT_WALK_KEY, settings.system.interrupt_walk);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_ART_CACHE_SIZE_KEY, settings.system.art_cache_size);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_COLOR_CYCLING_KEY, settings.system.color_cycling);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_CYCLE_SPEED_FACTOR_KEY, settings.system.cycle_speed_factor);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_HASHING_KEY, settings.system.hashing);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_SPLASH_KEY, settings.system.splash);
    settingsWrite(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_FREE_SPACE_KEY, settings.system.free_space);

    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_GAME_DIFFICULTY_KEY, settings.preferences.game_difficulty);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_DIFFICULTY_KEY, settings.preferences.combat_difficulty);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_VIOLENCE_LEVEL_KEY, settings.preferences.violence_level);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TARGET_HIGHLIGHT_KEY, settings.preferences.target_highlight);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_ITEM_HIGHLIGHT_KEY, settings.preferences.item_highlight);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_LOOKS_KEY, settings.preferences.combat_looks);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_MESSAGES_KEY, settings.preferences.combat_messages);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_TAUNTS_KEY, settings.preferences.combat_taunts);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_LANGUAGE_FILTER_KEY, settings.preferences.language_filter);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_RUNNING_KEY, settings.preferences.running);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_SUBTITLES_KEY, settings.preferences.subtitles);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_SPEED_KEY, settings.preferences.combat_speed);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_PLAYER_SPEED_KEY, settings.preferences.player_speedup); // also used for Affect Non-Combat Speed
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TEXT_BASE_DELAY_KEY, settings.preferences.text_base_delay);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_TEXT_LINE_DELAY_KEY, settings.preferences.text_line_delay);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_BRIGHTNESS_KEY, settings.preferences.brightness);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_MOUSE_SENSITIVITY_KEY, settings.preferences.mouse_sensitivity);
    settingsWrite(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_RUNNING_BURNING_GUY_KEY, settings.preferences.running_burning_guy);

    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_INITIALIZE_KEY, settings.sound.initialize);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DEBUG_KEY, settings.sound.debug);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DEBUG_SFXC_KEY, settings.sound.debug_sfxc);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DEVICE_KEY, settings.sound.device);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_PORT_KEY, settings.sound.port);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_IRQ_KEY, settings.sound.irq);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_DMA_KEY, settings.sound.dma);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SOUNDS_KEY, settings.sound.sounds);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_KEY, settings.sound.music);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SPEECH_KEY, settings.sound.speech);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MASTER_VOLUME_KEY, settings.sound.master_volume);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_VOLUME_KEY, settings.sound.music_volume);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SNDFX_VOLUME_KEY, settings.sound.sndfx_volume);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_SPEECH_VOLUME_KEY, settings.sound.speech_volume);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_CACHE_SIZE_KEY, settings.sound.cache_size);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH1_KEY, settings.sound.music_path1);
    settingsWrite(GAME_CONFIG_SOUND_KEY, GAME_CONFIG_MUSIC_PATH2_KEY, settings.sound.music_path2);

    settingsWrite(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_MODE_KEY, settings.debug.mode);
    settingsWrite(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_TILE_NUM_KEY, settings.debug.show_tile_num);
    settingsWrite(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_SCRIPT_MESSAGES_KEY, settings.debug.show_script_messages);
    settingsWrite(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_LOAD_INFO_KEY, settings.debug.show_load_info);
    settingsWrite(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_OUTPUT_MAP_DATA_INFO_KEY, settings.debug.output_map_data_info);
    settingsWrite(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_WRITE_OFFSETS, settings.debug.write_offsets);

    settingsWrite(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_GAME_WIDTH, settings.graphics.game_width);
    settingsWrite(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_GAME_HEIGHT, settings.graphics.game_height);
    settingsWrite(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_FULLSCREEN, settings.graphics.fullscreen);
    settingsWrite(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_STRETCH_ENABLED, settings.graphics.stretch_enabled);
    settingsWrite(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_PRESERVE_ASPECT, settings.graphics.preserve_aspect);
    settingsWrite(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_HIGH_QUALITY, settings.graphics.high_quality);
    settingsWrite(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_ENABLE_HIRES_STENCIL, settings.graphics.highres_stencil);
    settingsWrite(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_WIDESCREEN, settings.graphics.widescreen);
    settingsWrite(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_SQUARE_PIXELS, settings.graphics.square_pixels);
    settingsWrite(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_PLAY_AREA, settings.graphics.play_area);
    settingsWrite(GAME_CONFIG_GRAPHICS_KEY, GAME_CONFIG_VARIANT_SUFFIX, settings.graphics.widescreen_variant_suffix);

    settingsWrite(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_OVERRIDE_LIBRARIAN_KEY, settings.mapper.override_librarian);
    settingsWrite(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_LIBRARIAN_KEY, settings.mapper.librarian);
    settingsWrite(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_USE_ART_NOT_PROTOS_KEY, settings.mapper.user_art_not_protos);
    settingsWrite(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_REBUILD_PROTOS_KEY, settings.mapper.rebuild_protos);
    settingsWrite(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_FIX_MAP_OBJECTS_KEY, settings.mapper.fix_map_objects);
    settingsWrite(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_FIX_MAP_INVENTORY_KEY, settings.mapper.fix_map_inventory);
    settingsWrite(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_IGNORE_REBUILD_ERRORS_KEY, settings.mapper.rebuild_protos);
    settingsWrite(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SHOW_PID_NUMBERS_KEY, settings.mapper.show_pid_numbers);
    settingsWrite(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SAVE_TEXT_MAPS_KEY, settings.mapper.save_text_maps);
    settingsWrite(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_RUN_MAPPER_AS_GAME_KEY, settings.mapper.run_mapper_as_game);
    settingsWrite(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_DEFAULT_F8_AS_GAME_KEY, settings.mapper.default_f8_as_game);
    settingsWrite(GAME_CONFIG_MAPPER_KEY, GAME_CONFIG_SORT_SCRIPT_LIST_KEY, settings.mapper.sort_script_list);

    // User Gameplay Enhancements
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_STRICT_VANILLA, settings.enhancements.strict_vanilla);
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_AUTO_QUICK_SAVE, settings.enhancements.auto_quick_save);
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_AUTO_OPEN_DOORS, settings.enhancements.auto_open_doors);
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_GAPLESS_MUSIC, settings.enhancements.gapless_music);
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_ENHANCED_BARTER, settings.enhancements.enhanced_barter);
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_NUMBERS_IS_DIALOG_KEY, settings.enhancements.numbers_is_dialog);
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_DISPLAY_BONUS_DAMAGE_KEY, settings.enhancements.display_bonus_damage);
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_EXPLOSION_EMITS_LIGHT_KEY, settings.enhancements.explosion_emits_light);
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_REMOVE_CRITICALS_TIME_LIMITS_KEY, settings.enhancements.remove_criticals_time_limits);
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_DISPLAY_KARMA_CHANGES_KEY, settings.enhancements.display_karma_changes);
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_SKIP_OPENING_MOVIES_KEY, settings.enhancements.skip_opening_movies);
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_MASS_HIGHLIGHT, settings.enhancements.mass_highlight);
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_GAME_SPEED, settings.enhancements.game_speed);
    settingsWrite(GAME_CONFIG_ENHANCEMENTS_KEY, GAME_CONFIG_MINIMAP, settings.enhancements.minimap);

    // Mod Settings
    /*settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_MALE_KEY, settings.mod_settings.dude_native_look_jumpsuit_male);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE_KEY, settings.mod_settings.dude_native_look_jumpsuit_female);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_MALE_KEY, settings.mod_settings.dude_native_look_tribal_male);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_FEMALE_KEY, settings.mod_settings.dude_native_look_tribal_female);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_YEAR, settings.mod_settings.start_year);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_MONTH, settings.mod_settings.start_month);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_START_DAY, settings.mod_settings.start_day);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_BIG_FONT_COLOR_KEY, settings.mod_settings.main_menu_big_font_color);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_X_KEY, settings.mod_settings.main_menu_credits_offset_x);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_Y_KEY, settings.mod_settings.main_menu_credits_offset_y);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_FONT_COLOR_KEY, settings.mod_settings.main_menu_font_color);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_OFFSET_X_KEY, settings.mod_settings.main_menu_offset_x);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MAIN_MENU_OFFSET_Y_KEY, settings.mod_settings.main_menu_offset_y);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_STARTING_MAP_KEY, settings.mod_settings.starting_map);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_KARMA_FRMS_KEY, settings.mod_settings.karma_frms);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_KARMA_POINTS_KEY, settings.mod_settings.karma_points);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_OVERRIDE_CRITICALS_MODE_KEY, settings.mod_settings.override_criticals_mode);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_OVERRIDE_CRITICALS_FILE_KEY, settings.mod_settings.override_criticals_file);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PREMADE_CHARACTERS_FILE_NAMES_KEY, settings.mod_settings.premade_characters_file_names);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PREMADE_CHARACTERS_FACE_FIDS_KEY, settings.mod_settings.premade_characters_face_fids);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DYNAMITE_MIN_DAMAGE_KEY, settings.mod_settings.dynamite_min_damage);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DYNAMITE_MAX_DAMAGE_KEY, settings.mod_settings.dynamite_max_damage);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PLASTIC_EXPLOSIVE_MIN_DAMAGE_KEY, settings.mod_settings.plastic_explosive_min_damage);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PLASTIC_EXPLOSIVE_MAX_DAMAGE_KEY, settings.mod_settings.plastic_explosive_max_damage);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER1, settings.mod_settings.movie_timer_artimer1);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER2, settings.mod_settings.movie_timer_artimer2);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER3, settings.mod_settings.movie_timer_artimer3);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_MOVIE_TIMER_ARTIMER4, settings.mod_settings.movie_timer_artimer4);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PIPBOY_AVAILABLE_AT_GAMESTART, settings.mod_settings.pipboy_available_at_gamestart);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_DAMAGE_MOD_FORMULA_KEY, settings.mod_settings.damage_mod_formula);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_LOCKPICK_FRM_KEY, settings.mod_settings.use_lockpick_frm);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_STEAL_FRM_KEY, settings.mod_settings.use_steal_frm);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_TRAPS_FRM_KEY, settings.mod_settings.use_traps_frm);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_FIRST_AID_FRM_KEY, settings.mod_settings.use_first_aid_frm);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_DOCTOR_FRM_KEY, settings.mod_settings.use_doctor_frm);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_SCIENCE_FRM_KEY, settings.mod_settings.use_science_frm);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_REPAIR_FRM_KEY, settings.mod_settings.use_repair_frm);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_SCIENCE_REPAIR_TARGET_TYPE_KEY, settings.mod_settings.science_repair_target_type);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_GAME_DIALOG_GENDER_WORDS_KEY, settings.mod_settings.game_dialog_gender_words);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_WORLDMAP_TRAIL_MARKERS, settings.mod_settings.worldmap_trail_markers);

    // Game Fixes
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_USE_WALK_DISTANCE, settings.mod_settings.use_walk_distance);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_TOWN_MAP_HOTKEYS_FIX_KEY, settings.mod_settings.town_map_hotkeys_fix);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_GAME_DIALOG_FIX_KEY, settings.mod_settings.game_dialog_fix);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BONUS_HTH_DAMAGE_FIX_KEY, settings.mod_settings.bonus_hth_damage_fix);

    // Files and Paths
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BOOKS_FILE_KEY, settings.mod_settings.books_file);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_ELEVATORS_FILE_KEY, settings.mod_settings.elevators_file);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_CONSOLE_OUTPUT_FILE_KEY, settings.mod_settings.console_output_file);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_CITY_REPUTATION_LIST_KEY, settings.mod_settings.city_reputation_list);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_UNARMED_FILE_KEY, settings.mod_settings.unarmed_file);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_TWEAKS_FILE_KEY, settings.mod_settings.tweaks_file);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_EXTRA_MESSAGE_LISTS_KEY, settings.mod_settings.extra_message_lists);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_PATCH_FILE, settings.mod_settings.patch_file);
    settingsWrite(MOD_CONFIG_SCRIPTS_KEY, MOD_CONFIG_INI_CONFIG_FOLDER, settings.mod_scripts.ini_config_folder);
    settingsWrite(MOD_CONFIG_SCRIPTS_KEY, MOD_CONFIG_GLOBAL_SCRIPT_PATHS, settings.mod_scripts.global_script_paths);

    // Mods (Burst)
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_ENABLED_KEY, settings.mod_settings.burst_mod_enabled);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_CENTER_MULTIPLIER_KEY, settings.mod_settings.burst_mod_center_multiplier);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_CENTER_DIVISOR_KEY, settings.mod_settings.burst_mod_center_divisor);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_TARGET_MULTIPLIER_KEY, settings.mod_settings.burst_mod_target_multiplier);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_BURST_MOD_TARGET_DIVISOR_KEY, settings.mod_settings.burst_mod_target_divisor);

    // Others
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_VERSION_STRING, settings.mod_settings.version_string);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_MODE, settings.mod_settings.iface_bar_mode);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_WIDTH, settings.mod_settings.iface_bar_width);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_SIDE_ART, settings.mod_settings.iface_bar_side_art);
    settingsWrite(MOD_CONFIG_SETTINGS_KEY, MOD_CONFIG_IFACE_BAR_SIDES_ORI, settings.mod_settings.iface_bar_sides_ori);*/
}

static void settingsRead(const char* section, const char* key, std::string& value)
{
    char* v;
    if (configGetString(&gGameConfig, section, key, &v)) {
        value = v;
    }
}

static void settingsRead(const char* section, const char* key, int& value)
{
    int v;
    if (configGetInt(&gGameConfig, section, key, &v)) {
        value = v;
    }
}

static void settingsRead(const char* section, const char* key, bool& value)
{
    bool v;
    if (configGetBool(&gGameConfig, section, key, &v)) {
        value = v;
    }
}

static void settingsRead(const char* section, const char* key, double& value)
{
    double v;
    if (configGetDouble(&gGameConfig, section, key, &v)) {
        value = v;
    }
}

static void modSettingsRead(const char* section, const char* key, std::string& value)
{
    char* v;
    if (configGetString(&gModConfig, section, key, &v)) {
        value = v;
    }
}

static void modSettingsRead(const char* section, const char* key, int& value)
{
    int v;
    if (configGetInt(&gModConfig, section, key, &v)) {
        value = v;
    }
}

static void modSettingsRead(const char* section, const char* key, bool& value)
{
    bool v;
    if (configGetBool(&gModConfig, section, key, &v)) {
        value = v;
    }
}

static void settingsWrite(const char* section, const char* key, std::string& value)
{
    configSetString(&gGameConfig, section, key, value.c_str());
}

static void settingsWrite(const char* section, const char* key, int& value)
{
    configSetInt(&gGameConfig, section, key, value);
}

static void settingsWrite(const char* section, const char* key, bool& value)
{
    configSetBool(&gGameConfig, section, key, value);
}

static void settingsWrite(const char* section, const char* key, double& value)
{
    configSetDouble(&gGameConfig, section, key, value);
}

} // namespace fallout
