#include "game_content.h"

namespace fallout {

// Defaults match the F2 compile-time maxima. Each is reduced by the
// corresponding subsystem init when it discovers the game's real count.
int gPerkCount = 0;
int gStatCount = 0;
int gSkillCount = 0;
int gTraitCount = 0;

} // namespace fallout