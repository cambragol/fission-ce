#ifndef GAME_CONTENT_H
#define GAME_CONTENT_H

namespace fallout {

// Runtime counts of game content, discovered at init time by walking the
// message-file accessors until they run out. Each value is <= the
// corresponding compile-time *_COUNT, which remains the array-sizing max.
extern int gPerkCount;
extern int gStatCount;
extern int gSkillCount;
extern int gTraitCount;

} // namespace fallout

#endif