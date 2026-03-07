#ifndef FALLOUT_SETTINGS_H_
#define FALLOUT_SETTINGS_H_

#include <string>

#include "game_config.h"
#include "sfall_config.h"

namespace fallout {

struct SystemSettings {
    std::string executable = "game";
    std::string master_dat_path = "master.dat";
    std::string master_patches_path = "data";
    std::string critter_dat_path = "critter.dat";
    std::string critter_patches_path = "data";
    std::string fission_dat_path = "fission.dat";
    std::string fission_patches_path = "data";
    std::string language = ENGLISH;
    bool master_override;
    int scroll_lock = 0;
    bool interrupt_walk = true;
    int art_cache_size = 8;
    bool color_cycling = true;
    int cycle_speed_factor = 1;
    bool hashing = true;
    int splash = 0;
    int free_space = 20480;
    int times_run = 0;
};

struct PreferencesSettings {
    int game_difficulty = GAME_DIFFICULTY_NORMAL;
    int combat_difficulty = COMBAT_DIFFICULTY_NORMAL;
    int violence_level = VIOLENCE_LEVEL_MAXIMUM_BLOOD;
    int target_highlight = TARGET_HIGHLIGHT_TARGETING_ONLY;
    bool item_highlight = true;
    bool combat_looks = false;
    bool combat_messages = true;
    bool combat_taunts = true;
    bool language_filter = false;
    bool running = false;
    bool subtitles = false;
    int combat_speed = 0;
    bool player_speedup = false;
    double text_base_delay = 3.5;
    double text_line_delay = 1.399994;
    double brightness = 1.0;
    double mouse_sensitivity = 1.0;
    bool running_burning_guy = true;
};

struct SoundSettings {
    bool initialize = true;
    bool debug = false;
    bool debug_sfxc = true;
    int device = -1;
    int port = -1;
    int irq = -1;
    int dma = -1;
    bool sounds = true;
    bool music = true;
    bool speech = true;
    int master_volume = 22281;
    int music_volume = 22281;
    int sndfx_volume = 22281;
    int speech_volume = 22281;
    int cache_size = 448;
    std::string music_path1 = "sound\\music\\";
    std::string music_path2 = "sound\\music\\";
};

struct DebugSettings {
    std::string mode = "environment";
    bool show_tile_num = false;
    bool show_script_messages = false;
    bool show_load_info = false;
    bool output_map_data_info = false;
    bool write_offsets = false;
};

struct MapperSettings {
    bool override_librarian = false;
    bool librarian = false;
    bool user_art_not_protos = false;
    bool rebuild_protos = false;
    bool fix_map_objects = false;
    bool fix_map_inventory = false;
    bool ignore_rebuild_errors = false;
    bool show_pid_numbers = false;
    bool save_text_maps = false;
    bool run_mapper_as_game = false;
    bool default_f8_as_game = true;
    bool sort_script_list = false;
};

struct GraphicSettings {
    int game_width = 800;
    int game_height = 500;
    bool fullscreen = true;
    bool stretch_enabled = true;
    bool preserve_aspect = true;
    bool high_quality = false;
    bool highres_stencil = true;
    bool widescreen = true;
    bool square_pixels = false;
    int play_area = 1;
    std::string widescreen_variant_suffix = "_800";
};

struct EnhancementSettings {
    bool strict_vanilla = false;
    int auto_quick_save = 0;
    int auto_open_doors = 0;
    int gapless_music = 0;
    bool enhanced_barter = false;
    bool numbers_is_dialog = false;
    bool display_bonus_damage = false;
    bool explosion_emits_light = false;
    bool remove_criticals_time_limits = false;
    bool display_karma_changes = false;
    int skip_opening_movies = 0;
    bool mass_highlight = true;
    bool game_speed = true;
    bool minimap = false;
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
    int burst_mod_center_multiplier = MOD_CONFIG_BURST_MOD_DEFAULT_CENTER_MULTIPLIER;
    int burst_mod_center_divisor = MOD_CONFIG_BURST_MOD_DEFAULT_CENTER_DIVISOR;
    int burst_mod_target_multiplier = MOD_CONFIG_BURST_MOD_DEFAULT_TARGET_MULTIPLIER;
    int burst_mod_target_divisor = MOD_CONFIG_BURST_MOD_DEFAULT_TARGET_DIVISOR;
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
