#ifndef MOD_CONFIG_H
#define MOD_CONFIG_H

#include "config.h"

namespace fallout {

#define MOD_CONFIG_FILE_NAME "mod.cfg" // set this to mod 'configuration' file, kept in dat - one level down - new name "fission.cfg"?

// Changed category headers for better crouping in .cfg file
#define MOD_CONFIG_SETTINGS_KEY "mod_settings"
#define MOD_CONFIG_SCRIPTS_KEY "mod_scripts"

// mod settings
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
#define MOD_CONFIG_PREMADE_CHARACTERS_FILE_NAMES_KEY "PremadePaths"
#define MOD_CONFIG_PREMADE_CHARACTERS_FACE_FIDS_KEY "PremadeFIDs"
#define MOD_CONFIG_OVERRIDE_CRITICALS_MODE_KEY "OverrideCriticalTable"
#define MOD_CONFIG_OVERRIDE_CRITICALS_FILE_KEY "OverrideCriticalFile"
#define MOD_CONFIG_DAMAGE_MOD_FORMULA_KEY "DamageFormula"
#define MOD_CONFIG_STARTING_MAP_KEY "StartingMap"
#define MOD_CONFIG_KARMA_FRMS_KEY "KarmaFRMs"
#define MOD_CONFIG_KARMA_POINTS_KEY "KarmaPoints"
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

// files and paths - add to mod settings
#define MOD_CONFIG_INI_CONFIG_FOLDER "IniConfigFolder"
#define MOD_CONFIG_GLOBAL_SCRIPT_PATHS "GlobalScriptPaths"
#define MOD_CONFIG_PATCH_FILE "PatchFile"
#define MOD_CONFIG_EXTRA_MESSAGE_LISTS_KEY "ExtraGameMsgFileList"
#define MOD_CONFIG_TWEAKS_FILE_KEY "TweaksFile"
#define MOD_CONFIG_CITY_REPUTATION_LIST_KEY "CityRepsList"
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
#define MOD_CONFIG_BURST_MOD_DEFAULT_CENTER_MULTIPLIER 1
#define MOD_CONFIG_BURST_MOD_DEFAULT_CENTER_DIVISOR 3
#define MOD_CONFIG_BURST_MOD_DEFAULT_TARGET_MULTIPLIER 1
#define MOD_CONFIG_BURST_MOD_DEFAULT_TARGET_DIVISOR 2

// others - scrap or add to mod settings?
#define MOD_CONFIG_IFACE_BAR_MODE "IFACE_BAR_MODE"
#define MOD_CONFIG_IFACE_BAR_WIDTH "IFACE_BAR_WIDTH"
#define MOD_CONFIG_IFACE_BAR_SIDE_ART "IFACE_BAR_SIDE_ART"
#define MOD_CONFIG_IFACE_BAR_SIDES_ORI "IFACE_BAR_SIDES_ORI"

extern bool gModConfigInitialized;
extern Config gModConfig;

bool modConfigInit(int argc, char** argv);
void modConfigExit();

} // namespace fallout

#endif /* MOD_CONFIG_H */
