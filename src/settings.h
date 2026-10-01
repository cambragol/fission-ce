#ifndef FALLOUT_SETTINGS_H_
#define FALLOUT_SETTINGS_H_

#include <string>

#include "game_config.h"
#include "mod_config.h"

namespace fallout {

struct SystemSettings {
    std::string executable = GAME_CONFIG_DEFAULT_EXECUTABLE;
    std::string master_dat_path = GAME_CONFIG_DEFAULT_MASTER_DAT;
    std::string master_patches_path = GAME_CONFIG_DEFAULT_MASTER_PATCHES;
    std::string critter_dat_path = GAME_CONFIG_DEFAULT_CRITTER_DAT;
    std::string critter_patches_path = GAME_CONFIG_DEFAULT_CRITTER_PATCHES;
    std::string fission_dat_path = GAME_CONFIG_DEFAULT_FISSION_DAT;
    std::string fission_patches_path = GAME_CONFIG_DEFAULT_FISSION_PATCHES;
    std::string language = GAME_CONFIG_DEFAULT_LANGUAGE;
    bool master_override = GAME_CONFIG_DEFAULT_MASTER_OVERRIDE;
    int scroll_lock = GAME_CONFIG_DEFAULT_SCROLL_LOCK;
    bool interrupt_walk = GAME_CONFIG_DEFAULT_INTERRUPT_WALK;
    int art_cache_size = GAME_CONFIG_DEFAULT_ART_CACHE_SIZE;
    bool color_cycling = GAME_CONFIG_DEFAULT_COLOR_CYCLING;
    int cycle_speed_factor = GAME_CONFIG_DEFAULT_CYCLE_SPEED_FACTOR;
    bool hashing = GAME_CONFIG_DEFAULT_HASHING;
    int splash = GAME_CONFIG_DEFAULT_SPLASH;
    int free_space = GAME_CONFIG_DEFAULT_FREE_SPACE;
    int times_run = 0;   // runtime state only - no config key, no define
};

struct PreferencesSettings {
    int game_difficulty = GAME_CONFIG_DEFAULT_GAME_DIFFICULTY;
    int combat_difficulty = GAME_CONFIG_DEFAULT_COMBAT_DIFFICULTY;
    int violence_level = GAME_CONFIG_DEFAULT_VIOLENCE_LEVEL;
    int target_highlight = GAME_CONFIG_DEFAULT_TARGET_HIGHLIGHT;
    int item_highlight = GAME_CONFIG_DEFAULT_ITEM_HIGHLIGHT;
    bool combat_looks = GAME_CONFIG_DEFAULT_COMBAT_LOOKS;
    bool combat_messages = GAME_CONFIG_DEFAULT_COMBAT_MESSAGES;
    bool combat_taunts = GAME_CONFIG_DEFAULT_COMBAT_TAUNTS;
    bool language_filter = GAME_CONFIG_DEFAULT_LANGUAGE_FILTER;
    bool running = GAME_CONFIG_DEFAULT_RUNNING;
    bool subtitles = GAME_CONFIG_DEFAULT_SUBTITLES;
    int combat_speed = GAME_CONFIG_DEFAULT_COMBAT_SPEED;
    bool player_speedup = GAME_CONFIG_DEFAULT_PLAYER_SPEED;
    double text_base_delay = GAME_CONFIG_DEFAULT_TEXT_BASE_DELAY;
    double text_line_delay = GAME_CONFIG_DEFAULT_TEXT_LINE_DELAY;
    double brightness = GAME_CONFIG_DEFAULT_BRIGHTNESS;
    double mouse_sensitivity = GAME_CONFIG_DEFAULT_MOUSE_SENSITIVITY;
    bool running_burning_guy = GAME_CONFIG_DEFAULT_RUNNING_BURNING_GUY;
};

struct SoundSettings {
    bool initialize = GAME_CONFIG_DEFAULT_SOUND_INITIALIZE;
    bool debug = GAME_CONFIG_DEFAULT_SOUND_DEBUG;
    bool debug_sfxc = GAME_CONFIG_DEFAULT_SOUND_DEBUG_SFXC;
    int device = GAME_CONFIG_DEFAULT_SOUND_DEVICE;
    int port = GAME_CONFIG_DEFAULT_SOUND_PORT;
    int irq = GAME_CONFIG_DEFAULT_SOUND_IRQ;
    int dma = GAME_CONFIG_DEFAULT_SOUND_DMA;
    bool sounds = GAME_CONFIG_DEFAULT_SOUND_SOUNDS;
    bool music = GAME_CONFIG_DEFAULT_SOUND_MUSIC;
    bool speech = GAME_CONFIG_DEFAULT_SOUND_SPEECH;
    int master_volume = GAME_CONFIG_DEFAULT_MASTER_VOLUME;
    int music_volume = GAME_CONFIG_DEFAULT_MUSIC_VOLUME;
    int sndfx_volume = GAME_CONFIG_DEFAULT_SNDFX_VOLUME;
    int speech_volume = GAME_CONFIG_DEFAULT_SPEECH_VOLUME;
    int cache_size = GAME_CONFIG_DEFAULT_SOUND_CACHE_SIZE;
    std::string music_path1 = GAME_CONFIG_DEFAULT_MUSIC_PATH1;
    std::string music_path2 = GAME_CONFIG_DEFAULT_MUSIC_PATH2;
};

struct DebugSettings {
    std::string mode = GAME_CONFIG_DEFAULT_DEBUG_MODE;
    bool show_tile_num = GAME_CONFIG_DEFAULT_SHOW_TILE_NUM;
    bool show_script_messages = GAME_CONFIG_DEFAULT_SHOW_SCRIPT_MESSAGES;
    bool show_load_info = GAME_CONFIG_DEFAULT_SHOW_LOAD_INFO;
    bool output_map_data_info = GAME_CONFIG_DEFAULT_OUTPUT_MAP_DATA_INFO;
    bool write_offsets = GAME_CONFIG_DEFAULT_WRITE_OFFSETS;
};

struct MapperSettings {
    bool override_librarian = GAME_CONFIG_DEFAULT_OVERRIDE_LIBRARIAN;
    bool librarian = GAME_CONFIG_DEFAULT_LIBRARIAN;
    bool user_art_not_protos = GAME_CONFIG_DEFAULT_USE_ART_NOT_PROTOS;
    bool rebuild_protos = GAME_CONFIG_DEFAULT_REBUILD_PROTOS;
    bool fix_map_objects = GAME_CONFIG_DEFAULT_FIX_MAP_OBJECTS;
    bool fix_map_inventory = GAME_CONFIG_DEFAULT_FIX_MAP_INVENTORY;
    bool ignore_rebuild_errors = GAME_CONFIG_DEFAULT_IGNORE_REBUILD_ERRORS;
    bool show_pid_numbers = GAME_CONFIG_DEFAULT_SHOW_PID_NUMBERS;
    bool save_text_maps = GAME_CONFIG_DEFAULT_SAVE_TEXT_MAPS;
    bool run_mapper_as_game = GAME_CONFIG_DEFAULT_RUN_MAPPER_AS_GAME;
    bool default_f8_as_game = GAME_CONFIG_DEFAULT_DEFAULT_F8_AS_GAME;
    bool sort_script_list = GAME_CONFIG_DEFAULT_SORT_SCRIPT_LIST;
};

struct GraphicSettings {
    int game_width = GAME_CONFIG_DEFAULT_GAME_WIDTH;
    int game_height = GAME_CONFIG_DEFAULT_GAME_HEIGHT;
    bool fullscreen = GAME_CONFIG_DEFAULT_FULLSCREEN;
    bool stretch_enabled = GAME_CONFIG_DEFAULT_STRETCH_ENABLED;
    bool preserve_aspect = GAME_CONFIG_DEFAULT_PRESERVE_ASPECT;
    bool high_quality = GAME_CONFIG_DEFAULT_HIGH_QUALITY;
    bool highres_stencil = GAME_CONFIG_DEFAULT_ENABLE_HIRES_STENCIL;
    bool widescreen = GAME_CONFIG_DEFAULT_WIDESCREEN;
    bool square_pixels = GAME_CONFIG_DEFAULT_SQUARE_PIXELS;
    int play_area = GAME_CONFIG_DEFAULT_PLAY_AREA;
    std::string widescreen_variant_suffix = GAME_CONFIG_DEFAULT_VARIANT_SUFFIX;
};

struct EnhancementSettings {
    bool strict_vanilla = GAME_CONFIG_DEFAULT_STRICT_VANILLA;
    int auto_quick_save = GAME_CONFIG_DEFAULT_AUTO_QUICK_SAVE;
    int auto_open_doors = GAME_CONFIG_DEFAULT_AUTO_OPEN_DOORS;
    int gapless_music = GAME_CONFIG_DEFAULT_GAPLESS_MUSIC;
    bool enhanced_barter = GAME_CONFIG_DEFAULT_ENHANCED_BARTER;
    bool numbers_is_dialog = GAME_CONFIG_DEFAULT_NUMBERS_IS_DIALOG;
    bool display_bonus_damage = GAME_CONFIG_DEFAULT_DISPLAY_BONUS_DAMAGE;
    bool explosion_emits_light = GAME_CONFIG_DEFAULT_EXPLOSION_EMITS_LIGHT;
    bool remove_criticals_time_limits = GAME_CONFIG_DEFAULT_REMOVE_CRITICALS_TIME_LIMITS;
    bool display_karma_changes = GAME_CONFIG_DEFAULT_DISPLAY_KARMA_CHANGES;
    int skip_opening_movies = GAME_CONFIG_DEFAULT_SKIP_OPENING_MOVIES;
    bool mass_highlight = GAME_CONFIG_DEFAULT_MASS_HIGHLIGHT;
    bool game_speed = GAME_CONFIG_DEFAULT_GAME_SPEED;
    bool auto_push = GAME_CONFIG_DEFAULT_AUTO_PUSH;
    bool minimap = GAME_CONFIG_DEFAULT_MINIMAP;
    int multi_column_inventory = GAME_CONFIG_DEFAULT_MULTI_COLUMN_INVENTORY;
    bool npc_armor = GAME_CONFIG_DEFAULT_NPC_ARMOR;
    bool green_monochrome = GAME_CONFIG_DEFAULT_GREEN_MONOCHROME;
    int inventory_filter = GAME_CONFIG_DEFAULT_INVENTORY_FILTER;
    bool display_weight = GAME_CONFIG_DEFAULT_DISPLAY_WEIGHT;
    bool companion_inventory = GAME_CONFIG_DEFAULT_COMPANION_INVENTORY;
    bool vock_floats = GAME_CONFIG_DEFAULT_VOCK_FLOATS;
};

struct ModSettings {
    std::string dude_native_look_jumpsuit_male = MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_JUMPSUIT_MALE;
    std::string dude_native_look_jumpsuit_female = MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE;
    std::string dude_native_look_tribal_male = MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_TRIBAL_MALE;
    std::string dude_native_look_tribal_female = MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_TRIBAL_FEMALE;
    int start_year = MOD_CONFIG_DEFAULT_START_YEAR;
    int start_month = MOD_CONFIG_DEFAULT_START_MONTH;
    int start_day = MOD_CONFIG_DEFAULT_START_DAY;
    int main_menu_big_font_color = MOD_CONFIG_DEFAULT_MAIN_MENU_BIG_FONT_COLOR;
    int main_menu_credits_offset_x = MOD_CONFIG_DEFAULT_MAIN_MENU_CREDITS_OFFSET_X;
    int main_menu_credits_offset_y = MOD_CONFIG_DEFAULT_MAIN_MENU_CREDITS_OFFSET_Y;
    int main_menu_font_color = MOD_CONFIG_DEFAULT_MAIN_MENU_FONT_COLOR;
    int main_menu_offset_x = MOD_CONFIG_DEFAULT_MAIN_MENU_OFFSET_X;
    int main_menu_offset_y = MOD_CONFIG_DEFAULT_MAIN_MENU_OFFSET_Y;
    std::string starting_map = MOD_CONFIG_DEFAULT_STARTING_MAP;
    std::string karma_frms = MOD_CONFIG_DEFAULT_KARMA_FRMS;
    std::string karma_points = MOD_CONFIG_DEFAULT_KARMA_POINTS;
    int override_criticals_mode = MOD_CONFIG_DEFAULT_OVERRIDE_CRITICALS_MODE;
    std::string override_criticals_file = MOD_CONFIG_DEFAULT_OVERRIDE_CRITICALS_FILE;
    std::string books_file = MOD_CONFIG_DEFAULT_BOOKS_FILE;
    std::string elevators_file = MOD_CONFIG_DEFAULT_ELEVATORS_FILE;
    std::string console_output_file = MOD_CONFIG_DEFAULT_CONSOLE_OUTPUT_FILE;
    std::string premade_characters_file_names = MOD_CONFIG_DEFAULT_PREMADE_CHARACTERS_FILE_NAMES;
    std::string premade_characters_face_fids = MOD_CONFIG_DEFAULT_PREMADE_CHARACTERS_FACE_FIDS;
    bool burst_mod_enabled = MOD_CONFIG_DEFAULT_BURST_MOD_ENABLED;
    int burst_mod_center_multiplier = MOD_CONFIG_DEFAULT_BURST_MOD_CENTER_MULTIPLIER;
    int burst_mod_center_divisor = MOD_CONFIG_DEFAULT_BURST_MOD_CENTER_DIVISOR;
    int burst_mod_target_multiplier = MOD_CONFIG_DEFAULT_BURST_MOD_TARGET_MULTIPLIER;
    int burst_mod_target_divisor = MOD_CONFIG_DEFAULT_BURST_MOD_TARGET_DIVISOR;
    int dynamite_min_damage = MOD_CONFIG_DEFAULT_DYNAMITE_MIN_DAMAGE;
    int dynamite_max_damage = MOD_CONFIG_DEFAULT_DYNAMITE_MAX_DAMAGE;
    int plastic_explosive_min_damage = MOD_CONFIG_DEFAULT_PLASTIC_EXPLOSIVE_MIN_DAMAGE;
    int plastic_explosive_max_damage = MOD_CONFIG_DEFAULT_PLASTIC_EXPLOSIVE_MAX_DAMAGE;
    int movie_timer_artimer1 = MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER1;
    int movie_timer_artimer2 = MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER2;
    int movie_timer_artimer3 = MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER3;
    int movie_timer_artimer4 = MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER4;
    std::string city_reputation_list = MOD_CONFIG_DEFAULT_CITY_REPUTATION_LIST;
    std::string unarmed_file = MOD_CONFIG_DEFAULT_UNARMED_FILE;
    int damage_mod_formula = MOD_CONFIG_DEFAULT_DAMAGE_MOD_FORMULA;
    bool bonus_hth_damage_fix = MOD_CONFIG_DEFAULT_BONUS_HTH_DAMAGE_FIX;
    int use_lockpick_frm = MOD_CONFIG_DEFAULT_USE_LOCKPICK_FRM;
    int use_steal_frm = MOD_CONFIG_DEFAULT_USE_STEAL_FRM;
    int use_traps_frm = MOD_CONFIG_DEFAULT_USE_TRAPS_FRM;
    int use_first_aid_frm = MOD_CONFIG_DEFAULT_USE_FIRST_AID_FRM;
    int use_doctor_frm = MOD_CONFIG_DEFAULT_USE_DOCTOR_FRM;
    int use_science_frm = MOD_CONFIG_DEFAULT_USE_SCIENCE_FRM;
    int use_repair_frm = MOD_CONFIG_DEFAULT_USE_REPAIR_FRM;
    bool science_repair_target_type = MOD_CONFIG_DEFAULT_SCIENCE_REPAIR_TARGET_TYPE;
    bool game_dialog_fix = MOD_CONFIG_DEFAULT_GAME_DIALOG_FIX;
    std::string tweaks_file = MOD_CONFIG_DEFAULT_TWEAKS_FILE;
    bool game_dialog_gender_words = MOD_CONFIG_DEFAULT_GAME_DIALOG_GENDER_WORDS;
    bool town_map_hotkeys_fix = MOD_CONFIG_DEFAULT_TOWN_MAP_HOTKEYS_FIX;
    std::string extra_message_lists = MOD_CONFIG_DEFAULT_EXTRA_MESSAGE_LISTS;
    std::string version_string = MOD_CONFIG_DEFAULT_VERSION_STRING;
    std::string patch_file = MOD_CONFIG_DEFAULT_PATCH_FILE;
    int pipboy_available_at_gamestart = MOD_CONFIG_DEFAULT_PIPBOY_AVAILABLE_AT_GAMESTART;
    int use_walk_distance = MOD_CONFIG_DEFAULT_USE_WALK_DISTANCE;
    bool iface_bar_mode = MOD_CONFIG_DEFAULT_IFACE_BAR_MODE;
    int iface_bar_width = MOD_CONFIG_DEFAULT_IFACE_BAR_WIDTH;
    int iface_bar_side_art = MOD_CONFIG_DEFAULT_IFACE_BAR_SIDE_ART;
    bool iface_bar_sides_ori = MOD_CONFIG_DEFAULT_IFACE_BAR_SIDES_ORI;
    int worldmap_trail_markers = MOD_CONFIG_DEFAULT_WORLDMAP_TRAIL_MARKERS;
    int float_audio_channels = MOD_CONFIG_DEFAULT_FLOAT_AUDIO_CHANNELS;
    int float_distance_per_perception = MOD_CONFIG_DEFAULT_FLOAT_DISTANCE_PER_PERCEPTION;
    int float_obstruction_dampening = MOD_CONFIG_DEFAULT_FLOAT_OBSTRUCTION_DAMPENING;
    int float_eviction_policy = MOD_CONFIG_DEFAULT_FLOAT_EVICTION_POLICY;
    bool float_text_scramble = MOD_CONFIG_DEFAULT_FLOAT_TEXT_SCRAMBLE;
    int float_text_scramble_distance_per_perception = MOD_CONFIG_DEFAULT_FLOAT_TEXT_SCRAMBLE_DISTANCE_PER_PERCEPTION;
    std::string float_text_scramble_chars = MOD_CONFIG_DEFAULT_FLOAT_TEXT_SCRAMBLE_CHARS;
    bool voiced_floats = MOD_CONFIG_DEFAULT_VOICED_FLOATS;
    bool float_censor_bleep = MOD_CONFIG_DEFAULT_FLOAT_CENSOR_BLEEP;
    int float_volume = MOD_CONFIG_DEFAULT_FLOAT_VOLUME;
};

struct ModScriptsSettings {
    std::string ini_config_folder = MOD_CONFIG_DEFAULT_INI_CONFIG_FOLDER;
    std::string global_script_paths = MOD_CONFIG_DEFAULT_GLOBAL_SCRIPT_PATHS;
};

struct Settings {
    SystemSettings system;
    PreferencesSettings preferences;
    SoundSettings sound;
    DebugSettings debug;
    MapperSettings mapper;
    GraphicSettings graphics;
    EnhancementSettings enhancements;
    ModSettings mod_settings;
    ModScriptsSettings mod_scripts;
};

extern Settings settings;

bool settingsInit(bool isMapper, int argc, char** argv);
bool settingsSave();
bool settingsExit(bool shouldSave);
void settingsFromModConfig();

} // namespace fallout

#endif /* FALLOUT_SETTINGS_H_ */
