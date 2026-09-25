#ifndef GAME_VERSION_H
#define GAME_VERSION_H

namespace fallout {

typedef enum FalloutVersion {
    FALLOUT_VERSION_UNKNOWN = 0,
    FALLOUT_VERSION_1 = 1,
    FALLOUT_VERSION_2 = 2,
} FalloutVersion;

// The version of the game data currently loaded.
//
// Defaults to FALLOUT_VERSION_2 because that is FISSION's native behavior.
// Gets switched to FALLOUT_VERSION_1 the first time a classic-format
// (Fallout 1) DAT is successfully parsed. Never goes back.
FalloutVersion falloutVersionGet();
void falloutVersionSet(FalloutVersion version);
bool falloutVersionIsFallout1();

// Convenience macros so call sites read clearly.
#define IS_FALLOUT_1() (falloutVersionGet() == FALLOUT_VERSION_1)
#define IS_FALLOUT_2() (falloutVersionGet() == FALLOUT_VERSION_2)

// Resolves a game-data path to the mode-appropriate location.
// F2, Sonora, Nevada, and other total conversions keep their data at
// data/<name>, matching the path modders expect when they drop
// data/<name>_<mod>.txt alongside the base file.
#define GAME_DATA_PATH(name) \
    (IS_FALLOUT_1() ? "data\\fallout1\\" name : "data\\" name)

// Resolves a game message path to the mode-appropriate location.
// F2/Sonora/Nevada messages live at game/<name>.msg. F1-specific
// overrides and additions live at game/fallout1/<name>.msg inside
// fission.dat so they don't overwrite other games' message files.
#define GAME_MSG_PATH(name) \
    (IS_FALLOUT_1() ? "game\\fallout1\\" name : "game\\" name)

} // namespace fallout

#endif