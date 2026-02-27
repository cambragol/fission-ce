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
};

// May need to revist defaults on some of these
struct SfallMainSettings {
    // none for now
};

struct SfallMiscSettings {
    std::string dude_native_look_jumpsuit_male = "";
    std::string dude_native_look_jumpsuit_female = "";
    std::string dude_native_look_tribal_male = "";
    std::string dude_native_look_tribal_female = "";
    int start_year = 2241;
    int start_month = 6;
    int start_day = 24;
    int main_menu_big_font_color = 0;
    int main_menu_credits_offset_x = 0;
    int main_menu_credits_offset_y = 0;
    int main_menu_font_color = 0;
    int main_menu_offset_x = 0;
    int main_menu_offset_y = 0;
    std::string starting_map = "";
    std::string karma_frms = "";
    std::string karma_points = "";
    int override_criticals_mode = 2;
    std::string override_criticals_file = "";
    std::string books_file = "";
    std::string elevators_file = "";
    std::string console_output_file = "";
    std::string premade_characters_file_names = "";
    std::string premade_characters_face_fids = "";
    bool burst_mod_enabled = false;
    int burst_mod_center_multiplier = SFALL_CONFIG_BURST_MOD_DEFAULT_CENTER_MULTIPLIER;
    int burst_mod_center_divisor = SFALL_CONFIG_BURST_MOD_DEFAULT_CENTER_DIVISOR;
    int burst_mod_target_multiplier = SFALL_CONFIG_BURST_MOD_DEFAULT_TARGET_MULTIPLIER;
    int burst_mod_target_divisor = SFALL_CONFIG_BURST_MOD_DEFAULT_TARGET_DIVISOR;
    int dynamite_min_damage = 30;
    int dynamite_max_damage = 50;
    int plastic_explosive_min_damage = 40;
    int plastic_explosive_max_damage = 80;
    int movie_timer_artimer1 = 90;
    int movie_timer_artimer2 = 180;
    int movie_timer_artimer3 = 270;
    int movie_timer_artimer4 = 360;
    std::string city_reputation_list = "";
    std::string unarmed_file = "";
    int damage_mod_formula = 0;
    bool bonus_hth_damage_fix = true;
    int use_lockpick_frm = 293;
    int use_steal_frm = 293;
    int use_traps_frm = 293;
    int use_first_aid_frm = 293;
    int use_doctor_frm = 293;
    int use_science_frm = 293;
    int use_repair_frm = 293;
    bool science_repair_target_type = false;
    bool game_dialog_fix = true;
    std::string tweaks_file = "";
    bool game_dialog_gender_words = false;
    bool town_map_hotkeys_fix = true;
    std::string extra_message_lists = "";
    std::string version_string = "";
    std::string config_file = "";
    std::string patch_file = "";
    int pipboy_available_at_gamestart = 0;
    int use_walk_distance = 5;
    bool iface_bar_mode = true;
    int iface_bar_width = 800;
    int iface_bar_side_art = 0;
    bool iface_bar_sides_ori = false;
    int worldmap_trail_markers = 0;
};

struct SfallScriptsSettings {
    std::string ini_config_folder = "";
    std::string global_script_paths = "";
};

struct Settings {
    SystemSettings system;
    PreferencesSettings preferences;
    SoundSettings sound;
    DebugSettings debug;
    MapperSettings mapper;
    GraphicSettings graphics;
    EnhancementSettings enhancements;
    SfallMainSettings sfall_main; // [sfall_main] section
    SfallMiscSettings sfall_misc; // [sfall_misc] section
    SfallScriptsSettings sfall_scripts; // [sfall_scripts] section
};

extern Settings settings;

bool settingsInit(bool isMapper, int argc, char** argv);
bool settingsSave();
bool settingsExit(bool shouldSave);

} // namespace fallout

#endif /* FALLOUT_SETTINGS_H_ */
