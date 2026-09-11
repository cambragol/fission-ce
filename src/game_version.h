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

} // namespace fallout

#endif