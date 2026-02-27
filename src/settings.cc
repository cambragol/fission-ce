#include "settings.h"

namespace fallout {

static void settingsFromConfig();
static void settingsToConfig();
static void settingsRead(const char* section, const char* key, std::string& value);
static void settingsRead(const char* section, const char* key, int& value);
static void settingsRead(const char* section, const char* key, bool& value);
static void settingsRead(const char* section, const char* key, double& value);
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

    // sfall settings
    // Mod Settings
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_MALE_KEY, settings.sfall_misc.dude_native_look_jumpsuit_male);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE_KEY, settings.sfall_misc.dude_native_look_jumpsuit_female);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_MALE_KEY, settings.sfall_misc.dude_native_look_tribal_male);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_FEMALE_KEY, settings.sfall_misc.dude_native_look_tribal_female);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_START_YEAR, settings.sfall_misc.start_year);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_START_MONTH, settings.sfall_misc.start_month);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_START_DAY, settings.sfall_misc.start_day);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_BIG_FONT_COLOR_KEY, settings.sfall_misc.main_menu_big_font_color);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_CREDITS_OFFSET_X_KEY, settings.sfall_misc.main_menu_credits_offset_x);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_CREDITS_OFFSET_Y_KEY, settings.sfall_misc.main_menu_credits_offset_y);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_FONT_COLOR_KEY, settings.sfall_misc.main_menu_font_color);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_OFFSET_X_KEY, settings.sfall_misc.main_menu_offset_x);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_OFFSET_Y_KEY, settings.sfall_misc.main_menu_offset_y);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_STARTING_MAP_KEY, settings.sfall_misc.starting_map);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_KARMA_FRMS_KEY, settings.sfall_misc.karma_frms);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_KARMA_POINTS_KEY, settings.sfall_misc.karma_points);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_OVERRIDE_CRITICALS_MODE_KEY, settings.sfall_misc.override_criticals_mode);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_OVERRIDE_CRITICALS_FILE_KEY, settings.sfall_misc.override_criticals_file);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PREMADE_CHARACTERS_FILE_NAMES_KEY, settings.sfall_misc.premade_characters_file_names);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PREMADE_CHARACTERS_FACE_FIDS_KEY, settings.sfall_misc.premade_characters_face_fids);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DYNAMITE_MIN_DAMAGE_KEY, settings.sfall_misc.dynamite_min_damage);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DYNAMITE_MAX_DAMAGE_KEY, settings.sfall_misc.dynamite_max_damage);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PLASTIC_EXPLOSIVE_MIN_DAMAGE_KEY, settings.sfall_misc.plastic_explosive_min_damage);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PLASTIC_EXPLOSIVE_MAX_DAMAGE_KEY, settings.sfall_misc.plastic_explosive_max_damage);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER1, settings.sfall_misc.movie_timer_artimer1);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER2, settings.sfall_misc.movie_timer_artimer2);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER3, settings.sfall_misc.movie_timer_artimer3);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER4, settings.sfall_misc.movie_timer_artimer4);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PIPBOY_AVAILABLE_AT_GAMESTART, settings.sfall_misc.pipboy_available_at_gamestart);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DAMAGE_MOD_FORMULA_KEY, settings.sfall_misc.damage_mod_formula);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_LOCKPICK_FRM_KEY, settings.sfall_misc.use_lockpick_frm);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_STEAL_FRM_KEY, settings.sfall_misc.use_steal_frm);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_TRAPS_FRM_KEY, settings.sfall_misc.use_traps_frm);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_FIRST_AID_FRM_KEY, settings.sfall_misc.use_first_aid_frm);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_DOCTOR_FRM_KEY, settings.sfall_misc.use_doctor_frm);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_SCIENCE_FRM_KEY, settings.sfall_misc.use_science_frm);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_REPAIR_FRM_KEY, settings.sfall_misc.use_repair_frm);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_SCIENCE_REPAIR_TARGET_TYPE_KEY, settings.sfall_misc.science_repair_target_type);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_GAME_DIALOG_GENDER_WORDS_KEY, settings.sfall_misc.game_dialog_gender_words);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_WORLDMAP_TRAIL_MARKERS, settings.sfall_misc.worldmap_trail_markers);

    // Game Fixes
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_WALK_DISTANCE, settings.sfall_misc.use_walk_distance);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_TOWN_MAP_HOTKEYS_FIX_KEY, settings.sfall_misc.town_map_hotkeys_fix);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_GAME_DIALOG_FIX_KEY, settings.sfall_misc.game_dialog_fix);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BONUS_HTH_DAMAGE_FIX_KEY, settings.sfall_misc.bonus_hth_damage_fix);

    // Files and Paths
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BOOKS_FILE_KEY, settings.sfall_misc.books_file);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_ELEVATORS_FILE_KEY, settings.sfall_misc.elevators_file);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_CONSOLE_OUTPUT_FILE_KEY, settings.sfall_misc.console_output_file);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_CITY_REPUTATION_LIST_KEY, settings.sfall_misc.city_reputation_list);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_UNARMED_FILE_KEY, settings.sfall_misc.unarmed_file);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_TWEAKS_FILE_KEY, settings.sfall_misc.tweaks_file);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_EXTRA_MESSAGE_LISTS_KEY, settings.sfall_misc.extra_message_lists);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_CONFIG_FILE, settings.sfall_misc.config_file);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PATCH_FILE, settings.sfall_misc.patch_file);
    settingsRead(SFALL_CONFIG_SCRIPTS_KEY, SFALL_CONFIG_INI_CONFIG_FOLDER, settings.sfall_scripts.ini_config_folder);
    settingsRead(SFALL_CONFIG_SCRIPTS_KEY, SFALL_CONFIG_GLOBAL_SCRIPT_PATHS, settings.sfall_scripts.global_script_paths);

    // Mods (Burst)
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_ENABLED_KEY, settings.sfall_misc.burst_mod_enabled);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_CENTER_MULTIPLIER_KEY, settings.sfall_misc.burst_mod_center_multiplier);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_CENTER_DIVISOR_KEY, settings.sfall_misc.burst_mod_center_divisor);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_TARGET_MULTIPLIER_KEY, settings.sfall_misc.burst_mod_target_multiplier);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_TARGET_DIVISOR_KEY, settings.sfall_misc.burst_mod_target_divisor);

    // Others
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_VERSION_STRING, settings.sfall_misc.version_string);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_IFACE_BAR_MODE, settings.sfall_misc.iface_bar_mode);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_IFACE_BAR_WIDTH, settings.sfall_misc.iface_bar_width);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_IFACE_BAR_SIDE_ART, settings.sfall_misc.iface_bar_side_art);
    settingsRead(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_IFACE_BAR_SIDES_ORI, settings.sfall_misc.iface_bar_sides_ori);
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

    // sfall settings
    // Mod Settings
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_MALE_KEY, settings.sfall_misc.dude_native_look_jumpsuit_male);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE_KEY, settings.sfall_misc.dude_native_look_jumpsuit_female);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_MALE_KEY, settings.sfall_misc.dude_native_look_tribal_male);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_FEMALE_KEY, settings.sfall_misc.dude_native_look_tribal_female);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_START_YEAR, settings.sfall_misc.start_year);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_START_MONTH, settings.sfall_misc.start_month);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_START_DAY, settings.sfall_misc.start_day);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_BIG_FONT_COLOR_KEY, settings.sfall_misc.main_menu_big_font_color);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_CREDITS_OFFSET_X_KEY, settings.sfall_misc.main_menu_credits_offset_x);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_CREDITS_OFFSET_Y_KEY, settings.sfall_misc.main_menu_credits_offset_y);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_FONT_COLOR_KEY, settings.sfall_misc.main_menu_font_color);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_OFFSET_X_KEY, settings.sfall_misc.main_menu_offset_x);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MAIN_MENU_OFFSET_Y_KEY, settings.sfall_misc.main_menu_offset_y);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_STARTING_MAP_KEY, settings.sfall_misc.starting_map);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_KARMA_FRMS_KEY, settings.sfall_misc.karma_frms);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_KARMA_POINTS_KEY, settings.sfall_misc.karma_points);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_OVERRIDE_CRITICALS_MODE_KEY, settings.sfall_misc.override_criticals_mode);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_OVERRIDE_CRITICALS_FILE_KEY, settings.sfall_misc.override_criticals_file);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PREMADE_CHARACTERS_FILE_NAMES_KEY, settings.sfall_misc.premade_characters_file_names);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PREMADE_CHARACTERS_FACE_FIDS_KEY, settings.sfall_misc.premade_characters_face_fids);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DYNAMITE_MIN_DAMAGE_KEY, settings.sfall_misc.dynamite_min_damage);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DYNAMITE_MAX_DAMAGE_KEY, settings.sfall_misc.dynamite_max_damage);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PLASTIC_EXPLOSIVE_MIN_DAMAGE_KEY, settings.sfall_misc.plastic_explosive_min_damage);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PLASTIC_EXPLOSIVE_MAX_DAMAGE_KEY, settings.sfall_misc.plastic_explosive_max_damage);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER1, settings.sfall_misc.movie_timer_artimer1);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER2, settings.sfall_misc.movie_timer_artimer2);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER3, settings.sfall_misc.movie_timer_artimer3);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_MOVIE_TIMER_ARTIMER4, settings.sfall_misc.movie_timer_artimer4);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PIPBOY_AVAILABLE_AT_GAMESTART, settings.sfall_misc.pipboy_available_at_gamestart);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_DAMAGE_MOD_FORMULA_KEY, settings.sfall_misc.damage_mod_formula);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_LOCKPICK_FRM_KEY, settings.sfall_misc.use_lockpick_frm);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_STEAL_FRM_KEY, settings.sfall_misc.use_steal_frm);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_TRAPS_FRM_KEY, settings.sfall_misc.use_traps_frm);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_FIRST_AID_FRM_KEY, settings.sfall_misc.use_first_aid_frm);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_DOCTOR_FRM_KEY, settings.sfall_misc.use_doctor_frm);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_SCIENCE_FRM_KEY, settings.sfall_misc.use_science_frm);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_REPAIR_FRM_KEY, settings.sfall_misc.use_repair_frm);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_SCIENCE_REPAIR_TARGET_TYPE_KEY, settings.sfall_misc.science_repair_target_type);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_GAME_DIALOG_GENDER_WORDS_KEY, settings.sfall_misc.game_dialog_gender_words);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_WORLDMAP_TRAIL_MARKERS, settings.sfall_misc.worldmap_trail_markers);

    // Game Fixes
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_USE_WALK_DISTANCE, settings.sfall_misc.use_walk_distance);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_TOWN_MAP_HOTKEYS_FIX_KEY, settings.sfall_misc.town_map_hotkeys_fix);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_GAME_DIALOG_FIX_KEY, settings.sfall_misc.game_dialog_fix);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BONUS_HTH_DAMAGE_FIX_KEY, settings.sfall_misc.bonus_hth_damage_fix);

    // Files and Paths
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BOOKS_FILE_KEY, settings.sfall_misc.books_file);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_ELEVATORS_FILE_KEY, settings.sfall_misc.elevators_file);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_CONSOLE_OUTPUT_FILE_KEY, settings.sfall_misc.console_output_file);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_CITY_REPUTATION_LIST_KEY, settings.sfall_misc.city_reputation_list);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_UNARMED_FILE_KEY, settings.sfall_misc.unarmed_file);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_TWEAKS_FILE_KEY, settings.sfall_misc.tweaks_file);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_EXTRA_MESSAGE_LISTS_KEY, settings.sfall_misc.extra_message_lists);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_CONFIG_FILE, settings.sfall_misc.config_file);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_PATCH_FILE, settings.sfall_misc.patch_file);
    settingsWrite(SFALL_CONFIG_SCRIPTS_KEY, SFALL_CONFIG_INI_CONFIG_FOLDER, settings.sfall_scripts.ini_config_folder);
    settingsWrite(SFALL_CONFIG_SCRIPTS_KEY, SFALL_CONFIG_GLOBAL_SCRIPT_PATHS, settings.sfall_scripts.global_script_paths);

    // Mods (Burst)
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_ENABLED_KEY, settings.sfall_misc.burst_mod_enabled);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_CENTER_MULTIPLIER_KEY, settings.sfall_misc.burst_mod_center_multiplier);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_CENTER_DIVISOR_KEY, settings.sfall_misc.burst_mod_center_divisor);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_TARGET_MULTIPLIER_KEY, settings.sfall_misc.burst_mod_target_multiplier);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_BURST_MOD_TARGET_DIVISOR_KEY, settings.sfall_misc.burst_mod_target_divisor);

    // Others
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_VERSION_STRING, settings.sfall_misc.version_string);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_IFACE_BAR_MODE, settings.sfall_misc.iface_bar_mode);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_IFACE_BAR_WIDTH, settings.sfall_misc.iface_bar_width);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_IFACE_BAR_SIDE_ART, settings.sfall_misc.iface_bar_side_art);
    settingsWrite(SFALL_CONFIG_MISC_KEY, SFALL_CONFIG_IFACE_BAR_SIDES_ORI, settings.sfall_misc.iface_bar_sides_ori);
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
