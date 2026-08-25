#ifndef MOD_CONFIG_H
#define MOD_CONFIG_H

#include "config.h"
#include "platform_compat.h"

namespace fallout {

#define MOD_CONFIG_FILE_NAME "game.cfg" // set this to mod 'configuration' file, kept in dat - one level down - new name "fission.cfg"?

// Changed category headers for better crouping in .cfg file
#define MOD_CONFIG_SETTINGS_KEY "mod_settings"
#define MOD_CONFIG_SCRIPTS_KEY "mod_scripts"

// ---- game.cfg settings ----
#define MOD_CONFIG_VERSION_STRING "VersionString"
#define MOD_CONFIG_START_YEAR "StartYear"
#define MOD_CONFIG_START_MONTH "StartMonth"
#define MOD_CONFIG_START_DAY "StartDay"
#define MOD_CONFIG_MOVIE_TIMER_ARTIMER1 "MovieTimer_artimer1"
#define MOD_CONFIG_MOVIE_TIMER_ARTIMER2 "MovieTimer_artimer2"
#define MOD_CONFIG_MOVIE_TIMER_ARTIMER3 "MovieTimer_artimer3"
#define MOD_CONFIG_MOVIE_TIMER_ARTIMER4 "MovieTimer_artimer4"
#define MOD_CONFIG_PIPBOY_AVAILABLE_AT_GAMESTART "PipBoyAvailableAtGameStart"
#define MOD_CONFIG_SCIENCE_REPAIR_TARGET_TYPE_KEY "ScienceOnCritters"
#define MOD_CONFIG_USE_LOCKPICK_FRM_KEY "Lockpick"
#define MOD_CONFIG_USE_STEAL_FRM_KEY "Steal"
#define MOD_CONFIG_USE_TRAPS_FRM_KEY "Traps"
#define MOD_CONFIG_USE_FIRST_AID_FRM_KEY "FirstAid"
#define MOD_CONFIG_USE_DOCTOR_FRM_KEY "Doctor"
#define MOD_CONFIG_USE_SCIENCE_FRM_KEY "Science"
#define MOD_CONFIG_USE_REPAIR_FRM_KEY "Repair"
#define MOD_CONFIG_DYNAMITE_MIN_DAMAGE_KEY "Dynamite_DmgMin"
#define MOD_CONFIG_DYNAMITE_MAX_DAMAGE_KEY "Dynamite_DmgMax"
#define MOD_CONFIG_PLASTIC_EXPLOSIVE_MIN_DAMAGE_KEY "PlasticExplosive_DmgMin"
#define MOD_CONFIG_PLASTIC_EXPLOSIVE_MAX_DAMAGE_KEY "PlasticExplosive_DmgMax"
#define MOD_CONFIG_PREMADE_CHARACTERS_FILE_NAMES_KEY "PremadePaths" // requires parsing/modification
#define MOD_CONFIG_PREMADE_CHARACTERS_FACE_FIDS_KEY "PremadeFIDs" // requires parsing/modification
#define MOD_CONFIG_OVERRIDE_CRITICALS_MODE_KEY "OverrideCriticalTable"
#define MOD_CONFIG_OVERRIDE_CRITICALS_FILE_KEY "OverrideCriticalFile"
#define MOD_CONFIG_DAMAGE_MOD_FORMULA_KEY "DamageFormula"
#define MOD_CONFIG_STARTING_MAP_KEY "StartingMap"
#define MOD_CONFIG_KARMA_FRMS_KEY "KarmaFRMs" // requires parsing/modification
#define MOD_CONFIG_KARMA_POINTS_KEY "KarmaPoints" // requires parsing/modification
#define MOD_CONFIG_MAIN_MENU_BIG_FONT_COLOR_KEY "MainMenuBigFontColour"
#define MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_X_KEY "MainMenuCreditsOffsetX"
#define MOD_CONFIG_MAIN_MENU_CREDITS_OFFSET_Y_KEY "MainMenuCreditsOffsetY"
#define MOD_CONFIG_MAIN_MENU_FONT_COLOR_KEY "MainMenuFontColour"
#define MOD_CONFIG_MAIN_MENU_OFFSET_X_KEY "MainMenuOffsetX"
#define MOD_CONFIG_MAIN_MENU_OFFSET_Y_KEY "MainMenuOffsetY"
#define MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_MALE_KEY "MaleDefaultModel"
#define MOD_CONFIG_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE_KEY "FemaleDefaultModel"
#define MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_MALE_KEY "MaleStartModel"
#define MOD_CONFIG_DUDE_NATIVE_LOOK_TRIBAL_FEMALE_KEY "FemaleStartModel"
#define MOD_CONFIG_GAME_DIALOG_GENDER_WORDS_KEY "DialogGenderWords"
#define MOD_CONFIG_WORLDMAP_TRAIL_MARKERS "WorldMapTravelMarkers"

// game fixes - hide, but turn off with strictVanilla?
#define MOD_CONFIG_USE_WALK_DISTANCE "UseWalkDistance"
#define MOD_CONFIG_TOWN_MAP_HOTKEYS_FIX_KEY "TownMapHotkeysFix"
#define MOD_CONFIG_GAME_DIALOG_FIX_KEY "DialogueFix"
#define MOD_CONFIG_BONUS_HTH_DAMAGE_FIX_KEY "BonusHtHDamageFix"

// Own section: number of NPC floats that can play voiced audio at once. See
// AUDIO_ENGINE_SOUND_BUFFERS in audio_engine.cc, which reserves this many
// mixer buffer slots in addition to its fixed music/SFX/dialogue budget.
#define MOD_CONFIG_VOCK_FLOATS_KEY "vock-floats"
#define MOD_CONFIG_FLOAT_AUDIO_CHANNELS_KEY "FloatAudioChannels"
// Float volume (and text-scramble clarity) falls off linearly with
// distance: gain = max(0, 1 - distance/refDistance), where refDistance is
// DistancePerPerception x Perception -- full volume at the speaker's own
// tile, a straight ramp down to an exact 0.0 at refDistance, silent beyond
// it. See _gsound_calc_float_distance_factor() in game_sound.cc. The
// formula the float-speech pool originally shipped with (see commit
// fef10eb); a config-selectable choice of curve-shaped alternatives
// (vanilla-ambient-SFX-mirroring, inverse-distance, sigmoid, log) was
// explored after that and removed again in favor of always using this one,
// simpler formula -- see git history around
// FLOAT_SPEECH_DISTANCE_FORMULA_VANILLA/INVERSE/SIGMOID/LOG and the
// DistanceFormula game.cfg key if reviving any of that is ever useful.
//
// refDistance = Perception x DistancePerPerception. Was a hardcoded #define
// (FLOAT_SPEECH_DISTANCE_PER_PERCEPTION) until this key was added; default
// (2) reproduces that original value exactly.
#define MOD_CONFIG_FLOAT_DISTANCE_PER_PERCEPTION_KEY "DistancePerPerception"
// 0-100, default 0 (off): how much a solid obstacle between the speaker and
// the player dims a float, applied as a percentage multiplier on top of
// the distance falloff above -- 0 means obstacles are ignored entirely
// (today's behavior), 100 means a blocked line is fully silent, values
// between scale it down proportionally.
// Reuses the same line-of-sight raycast as the obj_can_see_obj sfall
// opcode (_make_straight_path() in animation.cc/.h), so it costs one more
// tile walk per float, same as combat already pays per shot fired. Affects
// text-scramble clarity too, since gameSoundCalcFloatClarity() shares this
// same curve -- an obstructed line reads harder to make out, same as it
// sounds harder to make out.
#define MOD_CONFIG_FLOAT_OBSTRUCTION_DAMPENING_KEY "ObstructionDampening"
// What happens when a float triggers with every pool slot already busy --
// see speechLoadFloat() in game_sound.cc for the 3 policy values.
#define MOD_CONFIG_FLOAT_EVICTION_POLICY_KEY "EvictionPolicy"
#define FLOAT_SPEECH_EVICTION_POLICY_VANILLA (0)
#define FLOAT_SPEECH_EVICTION_POLICY_OLDEST (1)
#define FLOAT_SPEECH_EVICTION_POLICY_FURTHEST (2)
// Off by default (non-vanilla): garbles a float's on-screen text by the
// same distance-based clarity that already governs its audio (see
// gameSoundCalcFloatClarity() in game_sound.cc), so a wide screen showing a
// far-off NPC's line can't just be read clearly when it wouldn't be heard
// clearly. Applies to every float's text regardless of whether that line
// has an audio file or was badword-filtered -- the scramble only ever
// touches alphabetic characters, so it can't disturb the symbols the
// badword filter already substituted in.
#define MOD_CONFIG_FLOAT_TEXT_SCRAMBLE_KEY "TextScramble"
// Character pool TextScramble draws its noise glyphs from -- see
// _scr_scramble_float_text() in scripts.cc. Any characters are accepted;
// falls back to the default pool below if left empty in game.cfg.
#define MOD_CONFIG_FLOAT_TEXT_SCRAMBLE_CHARS_KEY "TextScrambleChars"
// On by default: play a float's real voice file when it has one and the
// line is clean. Sibling of CensorBleep below -- split out so either can be
// switched off independently under the [enhancements] VockFloats master
// gate (see game_config.h).
#define MOD_CONFIG_VOICED_FLOATS_KEY "VoicedFloats"
// On by default: play a censor tone in place of a badword-filtered float's
// audio (see MESSAGE_LIST_ITEM_TEXT_FILTERED in message.h). Independent of
// VoicedFloats -- a filtered line never plays its real audio either way, so
// this only decides whether it gets a bleep or plain silence.
#define MOD_CONFIG_FLOAT_CENSOR_BLEEP_KEY "CensorBleep"
// 0-32767 (VOLUME_MIN-VOLUME_MAX, same scale as the pre-existing dialog
// speech_volume setting), default 22281: a per-float volume knob layered
// multiplicatively on top of the Sound Effects Volume Preferences slider
// (gain = Volume / VOLUME_MAX), rather than a replacement for it. Linear,
// not logarithmic -- see _gsound_calc_float_volume() in game_sound.cc.
#define MOD_CONFIG_FLOAT_VOLUME_KEY "Volume"

// files and paths - add to mod settings
#define MOD_CONFIG_INI_CONFIG_FOLDER "IniConfigFolder"
#define MOD_CONFIG_GLOBAL_SCRIPT_PATHS "GlobalScriptPaths" // requires parsing/modification
#define MOD_CONFIG_PATCH_FILE "PatchFile"
#define MOD_CONFIG_EXTRA_MESSAGE_LISTS_KEY "ExtraGameMsgFileList" // requires parsing/modification
#define MOD_CONFIG_TWEAKS_FILE_KEY "TweaksFile"
#define MOD_CONFIG_CITY_REPUTATION_LIST_KEY "CityRepsList" // requires parsing/modification
#define MOD_CONFIG_UNARMED_FILE_KEY "UnarmedFile"
#define MOD_CONFIG_BOOKS_FILE_KEY "BooksFile"
#define MOD_CONFIG_ELEVATORS_FILE_KEY "ElevatorsFile"
#define MOD_CONFIG_CONSOLE_OUTPUT_FILE_KEY "ConsoleOutputPath"

// mods - add to mod settings?
#define MOD_CONFIG_BURST_MOD_ENABLED_KEY "ComputeSprayMod"
#define MOD_CONFIG_BURST_MOD_CENTER_MULTIPLIER_KEY "ComputeSpray_CenterMult"
#define MOD_CONFIG_BURST_MOD_CENTER_DIVISOR_KEY "ComputeSpray_CenterDiv"
#define MOD_CONFIG_BURST_MOD_TARGET_MULTIPLIER_KEY "ComputeSpray_TargetMult"
#define MOD_CONFIG_BURST_MOD_TARGET_DIVISOR_KEY "ComputeSpray_TargetDiv"

// others - scrap or add to mod settings?
#define MOD_CONFIG_IFACE_BAR_MODE "IFACE_BAR_MODE"
#define MOD_CONFIG_IFACE_BAR_WIDTH "IFACE_BAR_WIDTH"
#define MOD_CONFIG_IFACE_BAR_SIDE_ART "IFACE_BAR_SIDE_ART"
#define MOD_CONFIG_IFACE_BAR_SIDES_ORI "IFACE_BAR_SIDES_ORI"

// ---- game.cfg settings default values ----
#define MOD_CONFIG_DEFAULT_START_YEAR 2241
#define MOD_CONFIG_DEFAULT_START_MONTH 6
#define MOD_CONFIG_DEFAULT_START_DAY 24
#define MOD_CONFIG_DEFAULT_STARTING_MAP ""
#define MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER1 90
#define MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER2 180
#define MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER3 270
#define MOD_CONFIG_DEFAULT_MOVIE_TIMER_ARTIMER4 360
#define MOD_CONFIG_DEFAULT_PIPBOY_AVAILABLE_AT_GAMESTART 0
#define MOD_CONFIG_DEFAULT_OVERRIDE_CRITICALS_MODE 2
#define MOD_CONFIG_DEFAULT_OVERRIDE_CRITICALS_FILE ""
#define MOD_CONFIG_DEFAULT_PREMADE_CHARACTERS_FILE_NAMES ""
#define MOD_CONFIG_DEFAULT_PREMADE_CHARACTERS_FACE_FIDS ""
#define MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_JUMPSUIT_MALE ""
#define MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_JUMPSUIT_FEMALE ""
#define MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_TRIBAL_MALE ""
#define MOD_CONFIG_DEFAULT_DUDE_NATIVE_LOOK_TRIBAL_FEMALE ""
#define MOD_CONFIG_DEFAULT_MAIN_MENU_BIG_FONT_COLOR 0
#define MOD_CONFIG_DEFAULT_MAIN_MENU_CREDITS_OFFSET_X 0
#define MOD_CONFIG_DEFAULT_MAIN_MENU_CREDITS_OFFSET_Y 0
#define MOD_CONFIG_DEFAULT_MAIN_MENU_FONT_COLOR 0
#define MOD_CONFIG_DEFAULT_MAIN_MENU_OFFSET_X 0
#define MOD_CONFIG_DEFAULT_MAIN_MENU_OFFSET_Y 0
#define MOD_CONFIG_DEFAULT_VERSION_STRING ""
#define MOD_CONFIG_DEFAULT_WORLDMAP_TRAIL_MARKERS 0

// Game fixes
#define MOD_CONFIG_DEFAULT_USE_WALK_DISTANCE 5

// vock-floats
#define MOD_CONFIG_DEFAULT_FLOAT_AUDIO_CHANNELS 8
#define MOD_CONFIG_DEFAULT_FLOAT_DISTANCE_PER_PERCEPTION 2
#define MOD_CONFIG_DEFAULT_FLOAT_OBSTRUCTION_DAMPENING 50
#define MOD_CONFIG_DEFAULT_FLOAT_EVICTION_POLICY FLOAT_SPEECH_EVICTION_POLICY_VANILLA
#define MOD_CONFIG_DEFAULT_FLOAT_TEXT_SCRAMBLE 0
#define MOD_CONFIG_DEFAULT_FLOAT_TEXT_SCRAMBLE_CHARS "#%&*~^"
#define MOD_CONFIG_DEFAULT_VOICED_FLOATS 1
#define MOD_CONFIG_DEFAULT_FLOAT_CENSOR_BLEEP 1
#define MOD_CONFIG_DEFAULT_FLOAT_VOLUME 32767

// Files and paths
#define MOD_CONFIG_DEFAULT_INI_CONFIG_FOLDER ""
#define MOD_CONFIG_DEFAULT_GLOBAL_SCRIPT_PATHS ""
#define MOD_CONFIG_DEFAULT_PATCH_FILE ""
#define MOD_CONFIG_DEFAULT_EXTRA_MESSAGE_LISTS ""
#define MOD_CONFIG_DEFAULT_BOOKS_FILE ""
#define MOD_CONFIG_DEFAULT_ELEVATORS_FILE ""
#define MOD_CONFIG_DEFAULT_CONSOLE_OUTPUT_FILE ""

// Mods (Burst)
#define MOD_CONFIG_DEFAULT_BURST_MOD_ENABLED 0
#define MOD_CONFIG_BURST_MOD_DEFAULT_CENTER_MULTIPLIER 1
#define MOD_CONFIG_BURST_MOD_DEFAULT_CENTER_DIVISOR 3
#define MOD_CONFIG_BURST_MOD_DEFAULT_TARGET_MULTIPLIER 1
#define MOD_CONFIG_BURST_MOD_DEFAULT_TARGET_DIVISOR 2

#define MOD_CONFIG_DEFAULT_IFACE_BAR_MODE 1 // map extends below/under Iface bar
#define MOD_CONFIG_DEFAULT_IFACE_BAR_WIDTH 800
#define MOD_CONFIG_DEFAULT_IFACE_BAR_SIDE_ART 0
#define MOD_CONFIG_DEFAULT_IFACE_BAR_SIDES_ORI 0
#define MOD_CONFIG_DEFAULT_KARMA_FRMS ""
#define MOD_CONFIG_DEFAULT_KARMA_POINTS ""
#define MOD_CONFIG_DEFAULT_DYNAMITE_MIN_DAMAGE 30
#define MOD_CONFIG_DEFAULT_DYNAMITE_MAX_DAMAGE 50
#define MOD_CONFIG_DEFAULT_PLASTIC_EXPLOSIVE_MIN_DAMAGE 40
#define MOD_CONFIG_DEFAULT_PLASTIC_EXPLOSIVE_MAX_DAMAGE 80
#define MOD_CONFIG_DEFAULT_CITY_REPUTATION_LIST ""
#define MOD_CONFIG_DEFAULT_UNARMED_FILE ""
#define MOD_CONFIG_DEFAULT_DAMAGE_MOD_FORMULA 0
#define MOD_CONFIG_DEFAULT_BONUS_HTH_DAMAGE_FIX 1

// Skill FRM overrides
#define MOD_CONFIG_DEFAULT_USE_LOCKPICK_FRM 293
#define MOD_CONFIG_DEFAULT_USE_STEAL_FRM 293
#define MOD_CONFIG_DEFAULT_USE_TRAPS_FRM 293
#define MOD_CONFIG_DEFAULT_USE_FIRST_AID_FRM 293
#define MOD_CONFIG_DEFAULT_USE_DOCTOR_FRM 293
#define MOD_CONFIG_DEFAULT_USE_SCIENCE_FRM 293
#define MOD_CONFIG_DEFAULT_USE_REPAIR_FRM 293
#define MOD_CONFIG_DEFAULT_SCIENCE_REPAIR_TARGET_TYPE 0
#define MOD_CONFIG_DEFAULT_GAME_DIALOG_FIX 1
#define MOD_CONFIG_DEFAULT_GAME_DIALOG_GENDER_WORDS 0
#define MOD_CONFIG_DEFAULT_TWEAKS_FILE ""
#define MOD_CONFIG_DEFAULT_TOWN_MAP_HOTKEYS_FIX 1

// Mod metadata
#define MOD_INFO_MAX_NAME 128
#define MOD_INFO_MAX_DESC 512
#define MOD_INFO_MAX_AUTHOR 128
#define MOD_INFO_MAX_ICON 128
#define MOD_INFO_MAX_DEP_NAME 64
#define MOD_INFO_MAX_DEP 16
#define MAX_LOADED_MODS 64

#define MOD_ORDER_SEPARATOR '|'

typedef struct ModInfo {
    char name[MOD_INFO_MAX_NAME]; // internal name used for .dat etc.
    char display_name[MOD_INFO_MAX_NAME]; // user-friendly name for loaded mods UI
    char description[MOD_INFO_MAX_DESC];
    char author[MOD_INFO_MAX_AUTHOR];
    char dependencies[MOD_INFO_MAX_DEP][MOD_INFO_MAX_DEP_NAME];
    int dependencyCount;
    int icon_index;
    bool enabled; // whether mod is active (default true)
    char filePath[COMPAT_MAX_PATH];
    char datName[MOD_INFO_MAX_NAME];
} ModInfo;

// Global list of loaded mods
extern ModInfo gLoadedMods[MAX_LOADED_MODS];
extern int gLoadedModsCount;

extern bool gModConfigInitialized;
extern Config gModConfig;

bool modConfigInit(int argc, char** argv);
void modConfigExit();

void modConfigWriteOrderFromLoadedMods();
void modConfigWriteEnabledForSlot(const char* slotPath);
int modConfigCheckSlotEnabledMatchEx(const char* fullPath, char* missingModName, size_t maxSize);

// Apply enabled flags from a save slot's mod_enabled.cfg to gLoadedMods
int modConfigApplySaveModConfig(const char* slotPath);

} // namespace fallout

#endif /* MOD_CONFIG_H */
