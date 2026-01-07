#ifndef PIPBOY_H
#define PIPBOY_H

#include "db.h"
#include "message.h"

namespace fallout {

typedef enum PipboyOpenIntent {
    PIPBOY_OPEN_INTENT_UNSPECIFIED = 0,
    PIPBOY_OPEN_INTENT_REST = 1,
} PipboyOpenIntent;

// Quest mod system constants
#define BASE_QUEST_MAX 200 // Vanilla quests (0-199)
#define MOD_QUEST_START 200 // Starting slot for mod quests
#define MOD_QUEST_MAX 1000 // Maximum total quests
#define TOTAL_QUEST_MAX MOD_QUEST_MAX

// Mod quest tracking structure (kept separate for save compatibility)
typedef struct ModQuestInfo {
    int questId; // Quest ID (200-999)
    char modName[64]; // Mod file name without extension
    char questKey[64]; // Quest key for hashing
    int descMessageId; // Hashed message ID for quest description (ONLY NEED THIS)
} ModQuestInfo;

// Global arrays for mod quest tracking
extern ModQuestInfo gModQuests[MOD_QUEST_MAX - MOD_QUEST_START];
extern int gModQuestCount;
extern char gQuestModNames[TOTAL_QUEST_MAX][64]; // Mod name for each quest slot

int pipboyOpen(int intent);
void pipboyInit();
void pipboyReset();
int pipboySave(File* stream);
int pipboyLoad(File* stream);

extern MessageList gPipboyMessageList;
int pipboyMessageListInit();
void pipboyMessageListFree();

} // namespace fallout

#endif /* PIPBOY_H */
