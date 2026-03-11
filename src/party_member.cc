#include "party_member.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstdint>
#include <cctype>
#include <unordered_map>
#include <vector>

#include "animation.h"
#include "color.h"
#include "combat.h"
#include "combat_ai.h"
#include "combat_ai_defs.h"
#include "config.h"
#include "critter.h"
#include "debug.h"
#include "display_monitor.h"
#include "game.h"
#include "game_dialog.h"
#include "item.h"
#include "loadsave.h"
#include "map.h"
#include "memory.h"
#include "message.h"
#include "object.h"
#include "proto.h"
#include "proto_instance.h"
#include "queue.h"
#include "random.h"
#include "scripts.h"
#include "skill.h"
#include "stat.h"
#include "string_parsers.h"
#include "text_object.h"
#include "tile.h"
#include "window_manager.h"

namespace fallout {

#define DIR_SEPARATOR '/'

// SFALL: Enable party members with level 6 protos to reach level 6.
// CE: There are several party members who have 6 pids, but for unknown reason
// the original code cap was 5. This fix affects:
// - Dogmeat
// - Goris
// - Sulik
// - Vik
#define PARTY_MEMBER_MAX_LEVEL 6

typedef struct PartyMemberDescription {
    bool areaAttackMode[AREA_ATTACK_MODE_COUNT];
    bool runAwayMode[RUN_AWAY_MODE_COUNT];
    bool bestWeapon[BEST_WEAPON_COUNT];
    bool distanceMode[DISTANCE_COUNT];
    bool attackWho[ATTACK_WHO_COUNT];
    bool chemUse[CHEM_USE_COUNT];
    bool disposition[DISPOSITION_COUNT];
    int level_minimum;
    int level_up_every;
    int level_pids_num;
    int level_pids[PARTY_MEMBER_MAX_LEVEL];
} PartyMemberDescription;

typedef struct PartyMemberLevelUpInfo {
    int level; // party member level
    int numLevelUps; // number of PC level ups with this member in party
    int isEarly; // last level up was "early" due to successful roll
} STRU_519DBC;

typedef struct PartyMemberListItem {
    Object* object;
    Script* script;
    int* vars;
    struct PartyMemberListItem* next;
} PartyMemberListItem;

static int partyMemberGetDescription(Object* object, PartyMemberDescription** partyMemberDescriptionPtr);
static void partyMemberDescriptionInit(PartyMemberDescription* partyMemberDescription);
static int _partyMemberPrepLoadInstance(PartyMemberListItem* a1);
static int _partyMemberRecoverLoadInstance(PartyMemberListItem* a1);
static int _partyMemberNewObjID();
static int _partyMemberNewObjIDRecurseFind(Object* object, int objectId);
static int _partyMemberPrepItemSave(Object* object);
static int _partyMemberItemSave(Object* object);
static int _partyMemberItemRecover(PartyMemberListItem* a1);
static int _partyMemberClearItemList();
static int partyFixMultipleMembers();
static int _partyMemberCopyLevelInfo(Object* object, int a2);

// 0x519D9C
int gPartyMemberDescriptionsLength = 0;

// 0x519DA0
int* gPartyMemberPids = nullptr;

//
static PartyMemberListItem* _itemSaveListHead = nullptr;

// List of party members, it's length is [gPartyMemberDescriptionsLength] + 20.
//
// 0x519DA8
PartyMemberListItem* gPartyMembers = nullptr;

// Number of critters added to party.
//
// 0x519DAC
static int gPartyMembersLength = 0;

// 0x519DB0
static int _partyMemberItemCount = 20000;

// 0x519DB4
static int _partyStatePrepped = 0;

// 0x519DB8
static PartyMemberDescription* gPartyMemberDescriptions = nullptr;

// 0x519DBC
static PartyMemberLevelUpInfo* _partyMemberLevelUpInfoList = nullptr;

// 0x519DC0
static int _curID = 20000;

// Mod companion definition (includes level-up info)
struct ModPartyMember {
    uint32_t stableId;               // unique identifier (hash of modName:section)
    int pid;
    PartyMemberDescription desc;
    PartyMemberLevelUpInfo levelInfo; // level, numLevelUps, isEarly
};

// All loaded mod companions
static std::vector<ModPartyMember> gModPartyMembers;

// Map stableId -> index in gModPartyMembers (for quick lookup)
static std::unordered_map<uint32_t, int> gStableIdToModIndex;

// Map PID -> stableId (to identify mod companions at join time)
static std::unordered_map<int, uint32_t> gModPidToStableId;

// Map object -> stableId (runtime mapping for level-ups and support functions)
static std::unordered_map<Object*, uint32_t> gObjectToStableId;

// Level-up info for mod companions (key = stableId)
static std::unordered_map<uint32_t, PartyMemberLevelUpInfo> gModPartyMemberLevelUpInfo;

// Marker for save game extension
#define MOD_DATA_MARKER 0xDEADBEEF


// djb2 hash (same as used elsewhere)
static uint32_t party_hash_string(const char* str)
{
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Combine mod name and section name into a stable hash, with normalization
static uint32_t hashModPartyString(const char* modName, const char* sectionName)
{
    char combined[256];
    char normalized[256];
    char* dst = normalized;

    snprintf(combined, sizeof(combined), "%s:%s", modName, sectionName);

    for (const char* src = combined; *src && dst < normalized + sizeof(normalized) - 1; src++) {
        char c = *src;
        if (c == ':') {
            *dst++ = ':';
        } else if (isalnum((unsigned char)c) || c == '_') {
            *dst++ = tolower(c);
        }
        // else skip (shouldn't occur)
    }
    *dst = '\0';

    return party_hash_string(normalized);
}

// Extract mod name from filename like "party_MyMod.txt"
static const char* extractModNameFromPartyFile(const char* filename)
{
    static char modName[64];
    modName[0] = '\0';
    if (strncmp(filename, "party_", 6) != 0) return modName;
    const char* start = filename + 6;
    const char* dot = strchr(start, '.');
    if (!dot) return modName;
    size_t len = dot - start;
    if (len >= sizeof(modName)) len = sizeof(modName) - 1;
    strncpy(modName, start, len);
    modName[len] = '\0';
    return modName;
}

// partyMember_init
// 0x493BC0
int partyMembersInit()
{
    Config config;
    int partyMemberPid;
    char section[50];
    int baseCount = 0;
    char searchPattern[COMPAT_MAX_PATH];
    char** foundModFiles = nullptr;
    int modFileCount = 0;
    int modCount = 0;

    gPartyMemberDescriptionsLength = 0;

    if (!configInit(&config)) {
        return -1;
    }

    if (!configRead(&config, "data\\party.txt", true)) {
        goto err;
    }

    // --- First pass: count base entries ---
    snprintf(section, sizeof(section), "Party Member %d", gPartyMemberDescriptionsLength);
    while (configGetInt(&config, section, "party_member_pid", &partyMemberPid)) {
        gPartyMemberDescriptionsLength++;
        snprintf(section, sizeof(section), "Party Member %d", gPartyMemberDescriptionsLength);
    }

    baseCount = gPartyMemberDescriptionsLength;   // remember for later

    // --- Allocate base arrays ---
    gPartyMemberPids = (int*)internal_malloc(sizeof(*gPartyMemberPids) * baseCount);
    if (gPartyMemberPids == nullptr) goto err;
    memset(gPartyMemberPids, 0, sizeof(*gPartyMemberPids) * baseCount);

    gPartyMemberDescriptions = (PartyMemberDescription*)internal_malloc(sizeof(*gPartyMemberDescriptions) * baseCount);
    if (gPartyMemberDescriptions == nullptr) goto err;
    memset(gPartyMemberDescriptions, 0, sizeof(*gPartyMemberDescriptions) * baseCount);

    _partyMemberLevelUpInfoList = (PartyMemberLevelUpInfo*)internal_malloc(sizeof(*_partyMemberLevelUpInfoList) * baseCount);
    if (_partyMemberLevelUpInfoList == nullptr) goto err;
    memset(_partyMemberLevelUpInfoList, 0, sizeof(*_partyMemberLevelUpInfoList) * baseCount);

    // --- Fill base descriptions (exactly as original) ---
    for (int index = 0; index < baseCount; index++) {
        snprintf(section, sizeof(section), "Party Member %d", index);

        if (!configGetInt(&config, section, "party_member_pid", &partyMemberPid)) {
            break;
        }

        PartyMemberDescription* partyMemberDescription = &(gPartyMemberDescriptions[index]);

        gPartyMemberPids[index] = partyMemberPid;

        partyMemberDescriptionInit(partyMemberDescription);

        char* string;

        // ----- BEGIN ORIGINAL PARSING CODE -----
        if (configGetString(&config, section, "area_attack_mode", &string)) {
            while (*string != '\0') {
                int areaAttackMode;
                strParseStrFromList(&string, &areaAttackMode, gAreaAttackModeKeys, AREA_ATTACK_MODE_COUNT);
                partyMemberDescription->areaAttackMode[areaAttackMode] = true;
            }
        }

        if (configGetString(&config, section, "attack_who", &string)) {
            while (*string != '\0') {
                int attachWho;
                strParseStrFromList(&string, &attachWho, gAttackWhoKeys, ATTACK_WHO_COUNT);
                partyMemberDescription->attackWho[attachWho] = true;
            }
        }

        if (configGetString(&config, section, "best_weapon", &string)) {
            while (*string != '\0') {
                int bestWeapon;
                strParseStrFromList(&string, &bestWeapon, gBestWeaponKeys, BEST_WEAPON_COUNT);
                partyMemberDescription->bestWeapon[bestWeapon] = true;
            }
        }

        if (configGetString(&config, section, "chem_use", &string)) {
            while (*string != '\0') {
                int chemUse;
                strParseStrFromList(&string, &chemUse, gChemUseKeys, CHEM_USE_COUNT);
                partyMemberDescription->chemUse[chemUse] = true;
            }
        }

        if (configGetString(&config, section, "distance", &string)) {
            while (*string != '\0') {
                int distanceMode;
                strParseStrFromList(&string, &distanceMode, gDistanceModeKeys, DISTANCE_COUNT);
                partyMemberDescription->distanceMode[distanceMode] = true;
            }
        }

        if (configGetString(&config, section, "run_away_mode", &string)) {
            while (*string != '\0') {
                int runAwayMode;
                strParseStrFromList(&string, &runAwayMode, gRunAwayModeKeys, RUN_AWAY_MODE_COUNT);
                partyMemberDescription->runAwayMode[runAwayMode] = true;
            }
        }

        if (configGetString(&config, section, "disposition", &string)) {
            while (*string != '\0') {
                int disposition;
                strParseStrFromList(&string, &disposition, gDispositionKeys, DISPOSITION_COUNT);
                partyMemberDescription->disposition[disposition] = true;
            }
        }

        int levelUpEvery;
        if (configGetInt(&config, section, "level_up_every", &levelUpEvery)) {
            partyMemberDescription->level_up_every = levelUpEvery;

            int levelMinimum;
            if (configGetInt(&config, section, "level_minimum", &levelMinimum)) {
                partyMemberDescription->level_minimum = levelMinimum;
            }

            if (configGetString(&config, section, "level_pids", &string)) {
                while (*string != '\0' && partyMemberDescription->level_pids_num < PARTY_MEMBER_MAX_LEVEL) {
                    int levelPid;
                    strParseInt(&string, &levelPid);
                    partyMemberDescription->level_pids[partyMemberDescription->level_pids_num] = levelPid;
                    partyMemberDescription->level_pids_num++;
                }
            }
        }
        // ----- END ORIGINAL PARSING CODE -----
    }

    configFree(&config);

    // --- Load mod companions from party_*.txt files ---
    snprintf(searchPattern, sizeof(searchPattern),
        "%sdata%cparty_*.txt",
        _cd_path_base,
        DIR_SEPARATOR);

    modFileCount = fileNameListInit(searchPattern, &foundModFiles, 0, 0);

    if (modFileCount > 0) {
        // Sort files alphabetically for consistent loading order
        for (int i = 0; i < modFileCount - 1; i++) {
            for (int j = i + 1; j < modFileCount; j++) {
                if (strcmp(foundModFiles[i], foundModFiles[j]) > 0) {
                    char* temp = foundModFiles[i];
                    foundModFiles[i] = foundModFiles[j];
                    foundModFiles[j] = temp;
                }
            }
        }

        for (int i = 0; i < modFileCount; i++) {
            // Skip the base party.txt itself
            if (strcmp(foundModFiles[i], "party.txt") == 0) continue;

            const char* modName = extractModNameFromPartyFile(foundModFiles[i]);
            if (modName[0] == '\0') continue;

            // Build full path
            char filePath[COMPAT_MAX_PATH];
            snprintf(filePath, sizeof(filePath),
                "%sdata%c%s",
                _cd_path_base,
                DIR_SEPARATOR,
                foundModFiles[i]);

            Config modConfig;
            if (!configInit(&modConfig)) continue;
            if (configRead(&modConfig, filePath, true)) {
                // Parse all sections in this mod file
                int memberIdx = 0;
                snprintf(section, sizeof(section), "Party Member %d", memberIdx);
                while (configGetInt(&modConfig, section, "party_member_pid", &partyMemberPid)) {
                    // Check for duplicate PID with base companions
                    bool duplicate = false;
                    for (int b = 1; b < baseCount; b++) {   // base indices start at 1 (index 0 is dummy)
                        if (gPartyMemberPids[b] == partyMemberPid) {
                            duplicate = true;
                            break;
                        }
                    }
                    // Check for duplicate PID with previously loaded mod companions
                    if (!duplicate) {
                        for (const auto& mod : gModPartyMembers) {
                            if (mod.pid == partyMemberPid) {
                                duplicate = true;
                                break;
                            }
                        }
                    }

                    if (duplicate) {
                        debugPrint("Warning: PID %d in mod %s (section %s) already exists, skipping.\n",
                                   partyMemberPid, modName, section);
                    } else {
                        // Generate stable ID
                        uint32_t stableId = hashModPartyString(modName, section);

                        // Check for stable ID collision with existing mods
                        bool collision = false;
                        for (const auto& mod : gModPartyMembers) {
                            if (mod.stableId == stableId) {
                                collision = true;
                                break;
                            }
                        }
                        if (collision) {
                            char errorMsg[512];
                            snprintf(errorMsg, sizeof(errorMsg),
                                "PARTY MOD SLOT COLLISION DETECTED!\n\n"
                                "Mod: %s, Section: %s\n"
                                "Stable ID: %u\n"
                                "This ID is already used by another mod companion.\n"
                                "To resolve: Rename your mod file or section to change the hash.\n\n"
                                "This companion will NOT be loaded.",
                                modName, section, stableId);
                            showMesageBox(errorMsg);
                            debugPrint("\n  Collision: skipping mod companion '%s' (stableId %u)", section, stableId);
                        } else {
                            // Create new mod companion entry
                            ModPartyMember newMod;
                            memset(&newMod, 0, sizeof(newMod));
                            newMod.stableId = stableId;
                            newMod.pid = partyMemberPid;
                            partyMemberDescriptionInit(&newMod.desc);

                            char* string;

                            // ----- PARSE MOD FIELDS (same as base) -----
                            if (configGetString(&modConfig, section, "area_attack_mode", &string)) {
                                while (*string != '\0') {
                                    int mode;
                                    strParseStrFromList(&string, &mode, gAreaAttackModeKeys, AREA_ATTACK_MODE_COUNT);
                                    newMod.desc.areaAttackMode[mode] = true;
                                }
                            }

                            if (configGetString(&modConfig, section, "attack_who", &string)) {
                                while (*string != '\0') {
                                    int who;
                                    strParseStrFromList(&string, &who, gAttackWhoKeys, ATTACK_WHO_COUNT);
                                    newMod.desc.attackWho[who] = true;
                                }
                            }

                            if (configGetString(&modConfig, section, "best_weapon", &string)) {
                                while (*string != '\0') {
                                    int weapon;
                                    strParseStrFromList(&string, &weapon, gBestWeaponKeys, BEST_WEAPON_COUNT);
                                    newMod.desc.bestWeapon[weapon] = true;
                                }
                            }

                            if (configGetString(&modConfig, section, "chem_use", &string)) {
                                while (*string != '\0') {
                                    int chem;
                                    strParseStrFromList(&string, &chem, gChemUseKeys, CHEM_USE_COUNT);
                                    newMod.desc.chemUse[chem] = true;
                                }
                            }

                            if (configGetString(&modConfig, section, "distance", &string)) {
                                while (*string != '\0') {
                                    int dist;
                                    strParseStrFromList(&string, &dist, gDistanceModeKeys, DISTANCE_COUNT);
                                    newMod.desc.distanceMode[dist] = true;
                                }
                            }

                            if (configGetString(&modConfig, section, "run_away_mode", &string)) {
                                while (*string != '\0') {
                                    int mode;
                                    strParseStrFromList(&string, &mode, gRunAwayModeKeys, RUN_AWAY_MODE_COUNT);
                                    newMod.desc.runAwayMode[mode] = true;
                                }
                            }

                            if (configGetString(&modConfig, section, "disposition", &string)) {
                                while (*string != '\0') {
                                    int disp;
                                    strParseStrFromList(&string, &disp, gDispositionKeys, DISPOSITION_COUNT);
                                    newMod.desc.disposition[disp] = true;
                                }
                            }

                            int levelUpEvery;
                            if (configGetInt(&modConfig, section, "level_up_every", &levelUpEvery)) {
                                newMod.desc.level_up_every = levelUpEvery;

                                int levelMinimum;
                                if (configGetInt(&modConfig, section, "level_minimum", &levelMinimum)) {
                                    newMod.desc.level_minimum = levelMinimum;
                                }

                                if (configGetString(&modConfig, section, "level_pids", &string)) {
                                    while (*string != '\0' && newMod.desc.level_pids_num < PARTY_MEMBER_MAX_LEVEL) {
                                        int levelPid;
                                        strParseInt(&string, &levelPid);
                                        newMod.desc.level_pids[newMod.desc.level_pids_num] = levelPid;
                                        newMod.desc.level_pids_num++;
                                    }
                                }
                            }
                            // ----- END PARSE MOD FIELDS -----

                            // Initialize levelInfo to zero
                            newMod.levelInfo.level = 0;
                            newMod.levelInfo.numLevelUps = 0;
                            newMod.levelInfo.isEarly = 0;

                            // Add to containers
                            gModPartyMembers.push_back(newMod);
                            int modIndex = gModPartyMembers.size() - 1;
                            gStableIdToModIndex[stableId] = modIndex;
                            gModPidToStableId[partyMemberPid] = stableId;
                        }
                    }

                    memberIdx++;
                    snprintf(section, sizeof(section), "Party Member %d", memberIdx);
                }
            }
            configFree(&modConfig);
        }
        fileNameListFree(&foundModFiles, modFileCount);
    }

    modCount = (int)gModPartyMembers.size();

    // --- Allocate gPartyMembers with enough space for base + mod + 20 ---
    gPartyMembers = (PartyMemberListItem*)internal_malloc(sizeof(*gPartyMembers) * (baseCount + modCount + 20));
    if (gPartyMembers == nullptr) goto err;
    memset(gPartyMembers, 0, sizeof(*gPartyMembers) * (baseCount + modCount + 20));

    // gPartyMembersLength remains 0 (no companions yet; they will be added via partyMemberAdd)

    return 0;

err:
    configFree(&config);
    return -1;
}

// 0x4940E4
void partyMembersReset()
{
    for (int index = 0; index < gPartyMemberDescriptionsLength; index++) {
        _partyMemberLevelUpInfoList[index].level = 0;
        _partyMemberLevelUpInfoList[index].numLevelUps = 0;
        _partyMemberLevelUpInfoList[index].isEarly = 0;
    }
}

// 0x494134
void partyMembersExit()
{
    for (int index = 0; index < gPartyMemberDescriptionsLength; index++) {
        _partyMemberLevelUpInfoList[index].level = 0;
        _partyMemberLevelUpInfoList[index].numLevelUps = 0;
        _partyMemberLevelUpInfoList[index].isEarly = 0;
    }

    gPartyMemberDescriptionsLength = 0;

    if (gPartyMemberPids != nullptr) {
        internal_free(gPartyMemberPids);
        gPartyMemberPids = nullptr;
    }

    if (gPartyMembers != nullptr) {
        internal_free(gPartyMembers);
        gPartyMembers = nullptr;
    }

    if (gPartyMemberDescriptions != nullptr) {
        internal_free(gPartyMemberDescriptions);
        gPartyMemberDescriptions = nullptr;
    }

    if (_partyMemberLevelUpInfoList != nullptr) {
        internal_free(_partyMemberLevelUpInfoList);
        _partyMemberLevelUpInfoList = nullptr;
    }
}

// 0x4941F0
static int partyMemberGetDescription(Object* object, PartyMemberDescription** partyMemberDescriptionPtr)
{
    for (int index = 1; index < gPartyMemberDescriptionsLength; index++) {
        if (gPartyMemberPids[index] == object->pid) {
            *partyMemberDescriptionPtr = &(gPartyMemberDescriptions[index]);
            return 0;
        }
    }

    return -1;
}

static const PartyMemberDescription* getModPartyMemberDescription(Object* obj)
{
    auto it = gObjectToStableId.find(obj);
    if (it == gObjectToStableId.end()) return nullptr;

    uint32_t stableId = it->second;
    for (const auto& mod : gModPartyMembers) {
        if (mod.stableId == stableId) {
            return &mod.desc;
        }
    }
    return nullptr;
}

// 0x49425C
static void partyMemberDescriptionInit(PartyMemberDescription* partyMemberDescription)
{
    for (int index = 0; index < AREA_ATTACK_MODE_COUNT; index++) {
        partyMemberDescription->areaAttackMode[index] = 0;
    }

    for (int index = 0; index < RUN_AWAY_MODE_COUNT; index++) {
        partyMemberDescription->runAwayMode[index] = 0;
    }

    for (int index = 0; index < BEST_WEAPON_COUNT; index++) {
        partyMemberDescription->bestWeapon[index] = 0;
    }

    for (int index = 0; index < DISTANCE_COUNT; index++) {
        partyMemberDescription->distanceMode[index] = 0;
    }

    for (int index = 0; index < ATTACK_WHO_COUNT; index++) {
        partyMemberDescription->attackWho[index] = 0;
    }

    for (int index = 0; index < CHEM_USE_COUNT; index++) {
        partyMemberDescription->chemUse[index] = 0;
    }

    for (int index = 0; index < DISPOSITION_COUNT; index++) {
        partyMemberDescription->disposition[index] = 0;
    }

    partyMemberDescription->level_minimum = 0;
    partyMemberDescription->level_up_every = 0;
    partyMemberDescription->level_pids_num = 0;

    partyMemberDescription->level_pids[0] = -1;

    for (int index = 0; index < gPartyMemberDescriptionsLength; index++) {
        _partyMemberLevelUpInfoList[index].level = 0;
        _partyMemberLevelUpInfoList[index].numLevelUps = 0;
        _partyMemberLevelUpInfoList[index].isEarly = 0;
    }
}

// partyMemberAdd
// 0x494378
int partyMemberAdd(Object* object)
{
    if (gPartyMembersLength >= gPartyMemberDescriptionsLength + 20) {
        return -1;
    }

    for (int index = 0; index < gPartyMembersLength; index++) {
        PartyMemberListItem* partyMember = &(gPartyMembers[index]);
        if (partyMember->object == object || partyMember->object->pid == object->pid) {
            return 0;
        }
    }

    if (_partyStatePrepped) {
        debugPrint("\npartyMemberAdd DENIED: %s\n", critterGetName(object));
        return -1;
    }


    // Determine if this is a mod companion
    uint32_t stableId = 0;
    bool isMod = false;
    auto pidIt = gModPidToStableId.find(object->pid);
    if (pidIt != gModPidToStableId.end()) {
        isMod = true;
        stableId = pidIt->second;
    }

    if (_partyStatePrepped) {
        debugPrint("\npartyMemberAdd DENIED: %s\n", critterGetName(object));
        return -1;
    }

    PartyMemberListItem* partyMember = &(gPartyMembers[gPartyMembersLength]);
    partyMember->object = object;
    partyMember->script = nullptr;
    partyMember->vars = nullptr;

    object->id = (object->pid & 0xFFFFFF) + 18000;
    object->flags |= (OBJECT_NO_REMOVE | OBJECT_NO_SAVE);

    gPartyMembersLength++;

    // If mod companion, store mapping
    if (isMod) {
        gObjectToStableId[object] = stableId;
        // Ensure level-up info exists
        if (gModPartyMemberLevelUpInfo.find(stableId) == gModPartyMemberLevelUpInfo.end()) {
            PartyMemberLevelUpInfo info = {0, 0, 0};
            gModPartyMemberLevelUpInfo[stableId] = info;
        }
    }

    Script* script;
    if (scriptGetScript(object->sid, &script) != -1) {
        script->flags |= (SCRIPT_FLAG_0x08 | SCRIPT_FLAG_0x10);
        script->ownerId = object->id;

        object->sid = ((object->pid & 0xFFFFFF) + 18000) | (object->sid & 0xFF000000);
        script->sid = object->sid;
    }

    critterSetTeam(object, 0);
    queueRemoveEventsByType(object, EVENT_TYPE_SCRIPT);

    if (_gdialogActive()) {
        if (object == gGameDialogSpeaker) {
            _gdialogUpdatePartyStatus();
        }
    }

    return 0;
}

// partyMemberRemove
// 0x4944DC
int partyMemberRemove(Object* object)
{
    if (gPartyMembersLength == 0) {
        return -1;
    }

    if (object == nullptr) {
        return -1;
    }

    int index;
    for (index = 1; index < gPartyMembersLength; index++) {
        PartyMemberListItem* partyMember = &(gPartyMembers[index]);
        if (partyMember->object == object) {
            break;
        }
    }

    if (index == gPartyMembersLength) {
        return -1;
    }

    if (_partyStatePrepped) {
        debugPrint("\npartyMemberRemove DENIED: %s\n", critterGetName(object));
        return -1;
    }

    if (index < gPartyMembersLength - 1) {
        gPartyMembers[index].object = gPartyMembers[gPartyMembersLength - 1].object;
    }

    object->flags &= ~(OBJECT_NO_REMOVE | OBJECT_NO_SAVE);

    gPartyMembersLength--;

    // Remove from object->stableId map if present
    auto mapIt = gObjectToStableId.find(object);
    if (mapIt != gObjectToStableId.end()) {
        gObjectToStableId.erase(mapIt);
    }

    Script* script;
    if (scriptGetScript(object->sid, &script) != -1) {
        script->flags &= ~(SCRIPT_FLAG_0x08 | SCRIPT_FLAG_0x10);
    }

    queueRemoveEventsByType(object, EVENT_TYPE_SCRIPT);

    if (_gdialogActive()) {
        if (object == gGameDialogSpeaker) {
            _gdialogUpdatePartyStatus();
        }
    }

    return 0;
}

// 0x49460C
int _partyMemberPrepSave()
{
    _partyStatePrepped = 1;

    for (int index = 0; index < gPartyMembersLength; index++) {
        PartyMemberListItem* ptr = &(gPartyMembers[index]);

        if (index > 0) {
            ptr->object->flags &= ~(OBJECT_NO_REMOVE | OBJECT_NO_SAVE);
        }

        Script* script;
        if (scriptGetScript(ptr->object->sid, &script) != -1) {
            script->flags &= ~(SCRIPT_FLAG_0x08 | SCRIPT_FLAG_0x10);
        }
    }

    return 0;
}

// 0x49466C
int _partyMemberUnPrepSave()
{
    for (int index = 0; index < gPartyMembersLength; index++) {
        PartyMemberListItem* ptr = &(gPartyMembers[index]);

        if (index > 0) {
            ptr->object->flags |= (OBJECT_NO_REMOVE | OBJECT_NO_SAVE);
        }

        Script* script;
        if (scriptGetScript(ptr->object->sid, &script) != -1) {
            script->flags |= (SCRIPT_FLAG_0x08 | SCRIPT_FLAG_0x10);
        }
    }

    _partyStatePrepped = 0;

    return 0;
}

// 0x4946CC
int partyMembersSave(File* stream)
{
    // Write base party length and item count
    if (fileWriteInt32(stream, gPartyMembersLength) == -1)
        return -1;
    if (fileWriteInt32(stream, _partyMemberItemCount) == -1)
        return -1;

    // Write object IDs for all party members (excluding player)
    for (int index = 1; index < gPartyMembersLength; index++) {
        PartyMemberListItem* partyMember = &(gPartyMembers[index]);
        if (fileWriteInt32(stream, partyMember->object->id) == -1)
            return -1;
    }

    // Write base level-up info
    for (int index = 1; index < gPartyMemberDescriptionsLength; index++) {
        PartyMemberLevelUpInfo* ptr = &(_partyMemberLevelUpInfoList[index]);
        if (fileWriteInt32(stream, ptr->level) == -1)
            return -1;
        if (fileWriteInt32(stream, ptr->numLevelUps) == -1)
            return -1;
        if (fileWriteInt32(stream, ptr->isEarly) == -1)
            return -1;
    }

    // --- Mod data section ---
    // Count how many mod companions have non-zero level-up info
    int modCount = 0;
    for (const auto& kv : gModPartyMemberLevelUpInfo) {
        const auto& info = kv.second;
        if (info.level != 0 || info.numLevelUps != 0 || info.isEarly != 0)
            modCount++;
    }

    // Write marker
    if (fileWriteInt32(stream, MOD_DATA_MARKER) == -1)
        return -1;

    // Write count
    if (fileWriteInt32(stream, modCount) == -1)
        return -1;

    // Write each mod entry that has non-zero data
    for (const auto& kv : gModPartyMemberLevelUpInfo) {
        const auto& info = kv.second;
        if (info.level != 0 || info.numLevelUps != 0 || info.isEarly != 0) {
            if (fileWriteInt32(stream, (int32_t)kv.first) == -1)   // stableId
                return -1;
            if (fileWriteInt32(stream, info.level) == -1)
                return -1;
            if (fileWriteInt32(stream, info.numLevelUps) == -1)
                return -1;
            if (fileWriteInt32(stream, info.isEarly) == -1)
                return -1;
        }
    }

    return 0;
}

// 0x4947AC
int _partyMemberPrepLoad()
{
    if (_partyStatePrepped) {
        return -1;
    }

    _partyStatePrepped = 1;

    for (int index = 0; index < gPartyMembersLength; index++) {
        PartyMemberListItem* ptr_519DA8 = &(gPartyMembers[index]);
        if (_partyMemberPrepLoadInstance(ptr_519DA8) != 0) {
            return -1;
        }
    }

    return 0;
}

// partyMemberPrepLoadInstance
// 0x49480C
static int _partyMemberPrepLoadInstance(PartyMemberListItem* a1)
{
    Object* obj = a1->object;

    if (obj == nullptr) {
        debugPrint("\n  Error!: partyMemberPrepLoadInstance: No Critter Object!");
        a1->script = nullptr;
        a1->vars = nullptr;
        a1->next = nullptr;
        return 0;
    }

    if (PID_TYPE(obj->pid) == OBJ_TYPE_CRITTER) {
        obj->data.critter.combat.whoHitMe = nullptr;
    }

    Script* script;
    if (scriptGetScript(obj->sid, &script) == -1) {
        debugPrint("\n  Error!: partyMemberPrepLoadInstance: Can't find script!");
        debugPrint("\n          partyMemberPrepLoadInstance: script was: (%s)", critterGetName(obj));
        a1->script = nullptr;
        a1->vars = nullptr;
        a1->next = nullptr;
        return 0;
    }

    a1->script = (Script*)internal_malloc(sizeof(*script));
    if (a1->script == nullptr) {
        showMesageBox("\n  Error!: partyMemberPrepLoad: Out of memory!");
        exit(1);
    }

    memcpy(a1->script, script, sizeof(*script));

    if (script->localVarsCount != 0 && script->localVarsOffset != -1) {
        a1->vars = (int*)internal_malloc(sizeof(*a1->vars) * script->localVarsCount);
        if (a1->vars == nullptr) {
            showMesageBox("\n  Error!: partyMemberPrepLoad: Out of memory!");
            exit(1);
        }

        if (gMapLocalVars != nullptr) {
            memcpy(a1->vars, gMapLocalVars + script->localVarsOffset, sizeof(int) * script->localVarsCount);
        } else {
            debugPrint("\nWarning: partyMemberPrepLoadInstance: No map_local_vars found, but script references them!");
            memset(a1->vars, 0, sizeof(int) * script->localVarsCount);
        }
    }

    Inventory* inventory = &(obj->data.inventory);
    for (int index = 0; index < inventory->length; index++) {
        InventoryItem* inventoryItem = &(inventory->items[index]);
        _partyMemberItemSave(inventoryItem->item);
    }

    script->flags &= ~(SCRIPT_FLAG_0x08 | SCRIPT_FLAG_0x10);

    scriptRemove(script->sid);

    if (PID_TYPE(obj->pid) == OBJ_TYPE_CRITTER) {
        _dude_stand(obj, obj->rotation, -1);
    }

    return 0;
}

// partyMemberRecoverLoad
// 0x4949C4
int _partyMemberRecoverLoad()
{
    if (_partyStatePrepped != 1) {
        debugPrint("\npartyMemberRecoverLoad DENIED");
        return -1;
    }

    debugPrint("\n");

    for (int index = 0; index < gPartyMembersLength; index++) {
        if (_partyMemberRecoverLoadInstance(&(gPartyMembers[index])) != 0) {
            return -1;
        }

        debugPrint("[Party Member %d]: %s\n", index, critterGetName(gPartyMembers[index].object));
    }

    PartyMemberListItem* v6 = _itemSaveListHead;
    while (v6 != nullptr) {
        _itemSaveListHead = v6->next;

        _partyMemberItemRecover(v6);
        internal_free(v6);

        v6 = _itemSaveListHead;
    }

    _partyStatePrepped = 0;

    if (!_isLoadingGame()) {
        partyFixMultipleMembers();
    }

    return 0;
}

// partyMemberRecoverLoadInstance
// 0x494A88
static int _partyMemberRecoverLoadInstance(PartyMemberListItem* a1)
{
    if (a1->script == nullptr) {
        showMesageBox("\n  Error!: partyMemberRecoverLoadInstance: No script!");
        return 0;
    }

    int scriptType = SCRIPT_TYPE_CRITTER;
    if (PID_TYPE(a1->object->pid) != OBJ_TYPE_CRITTER) {
        scriptType = SCRIPT_TYPE_ITEM;
    }

    int v1 = -1;
    if (scriptAdd(&v1, scriptType) == -1) {
        showMesageBox("\n  Error!: partyMemberRecoverLoad: Can't create script!");
        exit(1);
    }

    Script* script;
    if (scriptGetScript(v1, &script) == -1) {
        showMesageBox("\n  Error!: partyMemberRecoverLoad: Can't find script!");
        exit(1);
    }

    memcpy(script, a1->script, sizeof(*script));

    int sid = (scriptType << 24) | ((a1->object->pid & 0xFFFFFF) + 18000);
    a1->object->sid = sid;
    script->sid = sid;

    script->flags &= ~(SCRIPT_FLAG_0x01 | SCRIPT_FLAG_0x04);

    internal_free(a1->script);
    a1->script = nullptr;

    script->flags |= (SCRIPT_FLAG_0x08 | SCRIPT_FLAG_0x10);

    if (a1->vars != nullptr) {
        script->localVarsOffset = mapAllocLocalVars(script->localVarsCount);
        memcpy(gMapLocalVars + script->localVarsOffset, a1->vars, sizeof(int) * script->localVarsCount);
    }

    return 0;
}

// 0x494BBC
int partyMembersLoad(File* stream)
{
    int* partyMemberObjectIds = (int*)internal_malloc(sizeof(*partyMemberObjectIds) * (gPartyMemberDescriptionsLength + 20));
    if (partyMemberObjectIds == nullptr) {
        return -1;
    }

    // Read base party length and item count
    if (fileReadInt32(stream, &gPartyMembersLength) == -1) {
        internal_free(partyMemberObjectIds);
        return -1;
    }
    if (fileReadInt32(stream, &_partyMemberItemCount) == -1) {
        internal_free(partyMemberObjectIds);
        return -1;
    }

    gPartyMembers->object = gDude;

    if (gPartyMembersLength != 0) {
        // Read object IDs
        for (int index = 1; index < gPartyMembersLength; index++) {
            if (fileReadInt32(stream, &(partyMemberObjectIds[index])) == -1) {
                internal_free(partyMemberObjectIds);
                return -1;
            }
        }

        // Match object IDs to actual objects
        for (int index = 1; index < gPartyMembersLength; index++) {
            int objectId = partyMemberObjectIds[index];

            Object* object = objectFindFirst();
            while (object != nullptr) {
                if (object->id == objectId) {
                    break;
                }
                object = objectFindNext();
            }

            if (object != nullptr) {
                gPartyMembers[index].object = object;
            } else {
                debugPrint("Couldn't find party member on map...trying to load anyway.\n");
                // Remove this slot
                if (index + 1 >= gPartyMembersLength) {
                    partyMemberObjectIds[index] = 0;
                } else {
                    memmove(&(partyMemberObjectIds[index]), &(partyMemberObjectIds[index + 1]), sizeof(*partyMemberObjectIds) * (gPartyMembersLength - (index + 1)));
                }
                index--;
                gPartyMembersLength--;
            }
        }

        if (_partyMemberUnPrepSave() == -1) {
            internal_free(partyMemberObjectIds);
            return -1;
        }
    }

    partyFixMultipleMembers();

    // Rebuild object->stableId map for mod companions (if any)
    for (int idx = 1; idx < gPartyMembersLength; idx++) {
        Object* obj = gPartyMembers[idx].object;
        auto it = gModPidToStableId.find(obj->pid);
        if (it != gModPidToStableId.end()) {
            gObjectToStableId[obj] = it->second;
        }
    }

    // Read base level-up info
    for (int index = 1; index < gPartyMemberDescriptionsLength; index++) {
        PartyMemberLevelUpInfo* levelUpInfo = &(_partyMemberLevelUpInfoList[index]);

        if (fileReadInt32(stream, &(levelUpInfo->level)) == -1) {
            internal_free(partyMemberObjectIds);
            return -1;
        }
        if (fileReadInt32(stream, &(levelUpInfo->numLevelUps)) == -1) {
            internal_free(partyMemberObjectIds);
            return -1;
        }
        if (fileReadInt32(stream, &(levelUpInfo->isEarly)) == -1) {
            internal_free(partyMemberObjectIds);
            return -1;
        }
    }

    // --- Mod data section ---
    int marker;
    if (fileReadInt32(stream, &marker) == -1) {
        // EOF reached - old save without mod data
        internal_free(partyMemberObjectIds);
        return 0;
    }

    if (marker != MOD_DATA_MARKER) {
        // Marker mismatch - this shouldn't happen in normal cases.
        // We'll assume it's an old save and seek back.
        // Use fileSeek to go back 4 bytes (the size of the marker we just read).
        fileSeek(stream, -4, SEEK_CUR);
        internal_free(partyMemberObjectIds);
        return 0;
    }

    // Marker found - read mod count
    int modCount;
    if (fileReadInt32(stream, &modCount) == -1) {
        internal_free(partyMemberObjectIds);
        return -1;
    }

    for (int i = 0; i < modCount; i++) {
        int32_t stableId;
        int level, numLevelUps, isEarly;

        if (fileReadInt32(stream, &stableId) == -1) {
            internal_free(partyMemberObjectIds);
            return -1;
        }
        if (fileReadInt32(stream, &level) == -1) {
            internal_free(partyMemberObjectIds);
            return -1;
        }
        if (fileReadInt32(stream, &numLevelUps) == -1) {
            internal_free(partyMemberObjectIds);
            return -1;
        }
        if (fileReadInt32(stream, &isEarly) == -1) {
            internal_free(partyMemberObjectIds);
            return -1;
        }

        // Verify that this stableId exists in current mod list
        bool found = false;
        for (const auto& mod : gModPartyMembers) {
            if (mod.stableId == (uint32_t)stableId) {
                found = true;
                break;
            }
        }
        if (found) {
            PartyMemberLevelUpInfo info;
            info.level = level;
            info.numLevelUps = numLevelUps;
            info.isEarly = isEarly;
            gModPartyMemberLevelUpInfo[(uint32_t)stableId] = info;
        } else {
            debugPrint("Warning: Saved mod companion with stableId %u not found, discarding.\n", (uint32_t)stableId);
        }
    }

    internal_free(partyMemberObjectIds);
    return 0;
}

// 0x494D7C
void _partyMemberClear()
{
    if (_partyStatePrepped) {
        _partyMemberUnPrepSave();
    }

    for (int index = gPartyMembersLength; index > 1; index--) {
        partyMemberRemove(gPartyMembers[1].object);
    }

    gPartyMembersLength = 1;

    _scr_remove_all();
    _partyMemberClearItemList();

    _partyStatePrepped = 0;
}

// 0x494DD0
int _partyMemberSyncPosition()
{
    int clockwiseRotation = (gDude->rotation + 2) % ROTATION_COUNT;
    int counterClockwiseRotation = (gDude->rotation + 4) % ROTATION_COUNT;

    int n = 0;
    int distance = 2;
    for (int index = 1; index < gPartyMembersLength; index++) {
        PartyMemberListItem* partyMember = &(gPartyMembers[index]);
        Object* partyMemberObj = partyMember->object;
        if ((partyMemberObj->flags & OBJECT_HIDDEN) == 0 && PID_TYPE(partyMemberObj->pid) == OBJ_TYPE_CRITTER) {
            int rotation;
            if ((n % 2) != 0) {
                rotation = clockwiseRotation;
            } else {
                rotation = counterClockwiseRotation;
            }

            int tile = tileGetTileInDirection(gDude->tile, rotation, distance / 2);
            objectAttemptPlacementPartyMember(partyMemberObj, tile, gDude->elevation);

            distance++;
            n++;
        }
    }

    return 0;
}

// Heals party members according to their healing rate.
//
// 0x494EB8
int _partyMemberRestingHeal(int hours)
{
    int healingTicks = hours / 3;
    if (healingTicks == 0) {
        return 0;
    }

    for (int index = 0; index < gPartyMembersLength; index++) {
        PartyMemberListItem* partyMember = &(gPartyMembers[index]);
        if (PID_TYPE(partyMember->object->pid) == OBJ_TYPE_CRITTER) {
            int healingRate = critterGetStat(partyMember->object, STAT_HEALING_RATE);
            critterAdjustHitPoints(partyMember->object, healingTicks * healingRate);
        }
    }

    return 1;
}

// 0x494F24
Object* partyMemberFindByPid(int pid)
{
    for (int index = 0; index < gPartyMembersLength; index++) {
        Object* object = gPartyMembers[index].object;
        if (object->pid == pid) {
            return object;
        }
    }

    return nullptr;
}

// 0x494F64
bool _isPotentialPartyMember(Object* object)
{
    for (int index = 1; index < gPartyMemberDescriptionsLength; index++) {
        if (object->pid == gPartyMemberPids[index]) {
            return true;
        }
    }
    // Check mod PIDs
    for (const auto& mod : gModPartyMembers) {
        if (object->pid == mod.pid) {
            return true;
        }
    }
    return false;
}

// Returns `true` if specified object is a party member.
//
// 0x494FC4
bool objectIsPartyMember(Object* object)
{
    if (object == nullptr) {
        return false;
    }

    if (object->id < 18000) {
        return false;
    }

    bool isPartyMember = false;

    for (int index = 0; index < gPartyMembersLength; index++) {
        if (gPartyMembers[index].object == object) {
            isPartyMember = true;
            break;
        }
    }

    return isPartyMember;
}

// Returns number of active critters in the party.
//
// 0x495010
int _getPartyMemberCount()
{
    int count = gPartyMembersLength;

    for (int index = 1; index < gPartyMembersLength; index++) {
        Object* object = gPartyMembers[index].object;

        if (PID_TYPE(object->pid) != OBJ_TYPE_CRITTER || critterIsDead(object) || (object->flags & OBJECT_HIDDEN) != 0) {
            count--;
        }
    }

    return count;
}

// 0x495070
static int _partyMemberNewObjID()
{
    Object* object;

    do {
        _curID++;

        object = objectFindFirst();
        while (object != nullptr) {
            if (object->id == _curID) {
                break;
            }

            Inventory* inventory = &(object->data.inventory);

            int index;
            for (index = 0; index < inventory->length; index++) {
                InventoryItem* inventoryItem = &(inventory->items[index]);
                Object* item = inventoryItem->item;
                if (item->id == _curID) {
                    break;
                }

                if (_partyMemberNewObjIDRecurseFind(item, _curID)) {
                    break;
                }
            }

            if (index < inventory->length) {
                break;
            }

            object = objectFindNext();
        }
    } while (object != nullptr);

    _curID++;

    return _curID;
}

// 0x4950F4
static int _partyMemberNewObjIDRecurseFind(Object* obj, int objectId)
{
    Inventory* inventory = &(obj->data.inventory);
    for (int index = 0; index < inventory->length; index++) {
        InventoryItem* inventoryItem = &(inventory->items[index]);
        if (inventoryItem->item->id == objectId) {
            return 1;
        }

        if (_partyMemberNewObjIDRecurseFind(inventoryItem->item, objectId)) {
            return 1;
        }
    }

    return 0;
}

// 0x495140
int _partyMemberPrepItemSaveAll()
{
    for (int partyMemberIndex = 0; partyMemberIndex < gPartyMembersLength; partyMemberIndex++) {
        PartyMemberListItem* partyMember = &(gPartyMembers[partyMemberIndex]);

        Inventory* inventory = &(partyMember->object->data.inventory);
        for (int inventoryItemIndex = 0; inventoryItemIndex < inventory->length; inventoryItemIndex++) {
            InventoryItem* inventoryItem = &(inventory->items[inventoryItemIndex]);
            _partyMemberPrepItemSave(inventoryItem->item);
        }
    }

    return 0;
}

// partyMemberPrepItemSaveAll
static int _partyMemberPrepItemSave(Object* object)
{
    if (object->sid != -1) {
        Script* script;
        if (scriptGetScript(object->sid, &script) == -1) {
            showMesageBox("\n  Error!: partyMemberPrepItemSaveAll: Can't find script!");
            exit(1);
        }

        script->flags |= (SCRIPT_FLAG_0x08 | SCRIPT_FLAG_0x10);
    }

    Inventory* inventory = &(object->data.inventory);
    for (int index = 0; index < inventory->length; index++) {
        InventoryItem* inventoryItem = &(inventory->items[index]);
        _partyMemberPrepItemSave(inventoryItem->item);
    }

    return 0;
}

// 0x495234
static int _partyMemberItemSave(Object* object)
{
    if (object->sid != -1) {
        Script* script;
        if (scriptGetScript(object->sid, &script) == -1) {
            showMesageBox("\n  Error!: partyMemberItemSave: Can't find script!");
            exit(1);
        }

        if (object->id < 20000) {
            script->ownerId = _partyMemberNewObjID();
            object->id = script->ownerId;
        }

        PartyMemberListItem* node = (PartyMemberListItem*)internal_malloc(sizeof(*node));
        if (node == nullptr) {
            showMesageBox("\n  Error!: partyMemberItemSave: Out of memory!");
            exit(1);
        }

        node->object = object;

        node->script = (Script*)internal_malloc(sizeof(*script));
        if (node->script == nullptr) {
            showMesageBox("\n  Error!: partyMemberItemSave: Out of memory!");
            exit(1);
        }

        memcpy(node->script, script, sizeof(*script));

        if (script->localVarsCount != 0 && script->localVarsOffset != -1) {
            node->vars = (int*)internal_malloc(sizeof(*node->vars) * script->localVarsCount);
            if (node->vars == nullptr) {
                showMesageBox("\n  Error!: partyMemberItemSave: Out of memory!");
                exit(1);
            }

            memcpy(node->vars, gMapLocalVars + script->localVarsOffset, sizeof(int) * script->localVarsCount);
        } else {
            node->vars = nullptr;
        }

        PartyMemberListItem* temp = _itemSaveListHead;
        _itemSaveListHead = node;
        node->next = temp;
    }

    Inventory* inventory = &(object->data.inventory);
    for (int index = 0; index < inventory->length; index++) {
        InventoryItem* inventoryItem = &(inventory->items[index]);
        _partyMemberItemSave(inventoryItem->item);
    }

    return 0;
}

// partyMemberItemRecover
// 0x495388
static int _partyMemberItemRecover(PartyMemberListItem* a1)
{
    int sid = -1;
    if (scriptAdd(&sid, SCRIPT_TYPE_ITEM) == -1) {
        showMesageBox("\n  Error!: partyMemberItemRecover: Can't create script!");
        exit(1);
    }

    Script* script;
    if (scriptGetScript(sid, &script) == -1) {
        showMesageBox("\n  Error!: partyMemberItemRecover: Can't find script!");
        exit(1);
    }

    memcpy(script, a1->script, sizeof(*script));

    a1->object->sid = _partyMemberItemCount | (SCRIPT_TYPE_ITEM << 24);
    script->sid = _partyMemberItemCount | (SCRIPT_TYPE_ITEM << 24);

    script->program = nullptr;
    script->flags &= ~(SCRIPT_FLAG_0x01 | SCRIPT_FLAG_0x04 | SCRIPT_FLAG_0x08 | SCRIPT_FLAG_0x10);

    _partyMemberItemCount++;

    internal_free(a1->script);
    a1->script = nullptr;

    if (a1->vars != nullptr) {
        script->localVarsOffset = mapAllocLocalVars(script->localVarsCount);
        memcpy(gMapLocalVars + script->localVarsOffset, a1->vars, sizeof(int) * script->localVarsCount);
    }

    return 0;
}

// 0x4954C4
static int _partyMemberClearItemList()
{
    while (_itemSaveListHead != nullptr) {
        PartyMemberListItem* node = _itemSaveListHead;
        _itemSaveListHead = _itemSaveListHead->next;

        if (node->script != nullptr) {
            internal_free(node->script);
        }

        if (node->vars != nullptr) {
            internal_free(node->vars);
        }

        internal_free(node);
    }

    _partyMemberItemCount = 20000;

    return 0;
}

// Returns best skill of the specified party member.
//
// 0x495520
int partyMemberGetBestSkill(Object* object)
{
    int bestSkill = SKILL_SMALL_GUNS;

    if (object == nullptr) {
        return bestSkill;
    }

    if (PID_TYPE(object->pid) != OBJ_TYPE_CRITTER) {
        return bestSkill;
    }

    int bestValue = 0;
    for (int skill = 0; skill < SKILL_COUNT; skill++) {
        int value = skillGetValue(object, skill);
        if (value > bestValue) {
            bestSkill = skill;
            bestValue = value;
        }
    }

    return bestSkill;
}

// Returns party member with highest skill level.
//
// 0x495560
Object* partyMemberGetBestInSkill(int skill)
{
    int bestValue = 0;
    Object* bestPartyMember = nullptr;

    for (int index = 0; index < gPartyMembersLength; index++) {
        Object* object = gPartyMembers[index].object;
        if ((object->flags & OBJECT_HIDDEN) == 0 && PID_TYPE(object->pid) == OBJ_TYPE_CRITTER) {
            int value = skillGetValue(object, skill);
            if (value > bestValue) {
                bestValue = value;
                bestPartyMember = object;
            }
        }
    }

    return bestPartyMember;
}

// Returns highest skill level in party.
//
// 0x4955C8
int partyGetBestSkillValue(int skill)
{
    int bestValue = 0;

    for (int index = 0; index < gPartyMembersLength; index++) {
        Object* object = gPartyMembers[index].object;
        if ((object->flags & OBJECT_HIDDEN) == 0 && PID_TYPE(object->pid) == OBJ_TYPE_CRITTER) {
            int value = skillGetValue(object, skill);
            if (value > bestValue) {
                bestValue = value;
            }
        }
    }

    return bestValue;
}

// 0x495620
static int partyFixMultipleMembers()
{
    debugPrint("\n\n\n[Party Members]:");

    // NOTE: Original code is slightly different (uses two nested loops).
    int critterCount = 0;
    Object* obj = objectFindFirst();
    while (obj != nullptr) {
        bool isPartyMember = false;
        for (int index = 1; index < gPartyMemberDescriptionsLength; index++) {
            if (obj->pid == gPartyMemberPids[index]) {
                isPartyMember = true;
                break;
            }
        }

        if (isPartyMember) {
            debugPrint("\n   PM: %s", critterGetName(obj));

            bool remove = false;
            if (obj->sid == -1) {
                remove = true;
            } else {
                // NOTE: Uninline.
                Object* partyMember = partyMemberFindByPid(obj->pid);
                if (partyMember != nullptr && partyMember != obj) {
                    if (partyMember->sid == obj->sid) {
                        obj->sid = -1;
                    }
                    remove = true;
                }
            }

            if (remove) {
                // NOTE: Uninline.
                if (obj != partyMemberFindByPid(obj->pid)) {
                    debugPrint("\nDestroying evil critter doppleganger!");

                    if (obj->sid != -1) {
                        scriptRemove(obj->sid);
                        obj->sid = -1;
                    } else {
                        if (queueRemoveEventsByType(obj, EVENT_TYPE_SCRIPT) == -1) {
                            debugPrint("\nERROR Removing Timed Events on FIX remove!!\n");
                        }
                    }

                    _combat_delete_critter(obj);

                    objectDestroy(obj, nullptr);

                    // Start over.
                    critterCount = 0;
                    obj = objectFindFirst();
                    continue;
                } else {
                    debugPrint("\nError: Attempting to destroy evil critter doppleganger FAILED!");
                }
            }
        }

        obj = objectFindNext();
    }

    for (int index = 0; index < gPartyMembersLength; index++) {
        PartyMemberListItem* partyMember = &(gPartyMembers[index]);

        Script* script;
        if (scriptGetScript(partyMember->object->sid, &script) != -1) {
            script->owner = partyMember->object;
        } else {
            debugPrint("\nError: Failed to fix party member critter scripts!");
        }
    }

    debugPrint("\nTotal Critter Count: %d\n\n", critterCount);

    return 0;
}

// 0x495870
void _partyMemberSaveProtos()
{
    for (int index = 1; index < gPartyMemberDescriptionsLength; index++) {
        int pid = gPartyMemberPids[index];
        if (pid != -1) {
            _proto_save_pid(pid);
        }
    }
}

// 0x4958B0
bool partyMemberSupportsDisposition(Object* critter, int disposition)
{
    if (critter == nullptr) {
        return false;
    }

    if (PID_TYPE(critter->pid) != OBJ_TYPE_CRITTER) {
        return false;
    }

    if (disposition == -1 || disposition > 5) {
        return false;
    }

    // Try base
    PartyMemberDescription* partyMemberDescription;
    if (partyMemberGetDescription(critter, &partyMemberDescription) == -1) {
        return false;
    }

    // Try mod
    const PartyMemberDescription* modDesc = getModPartyMemberDescription(critter);
    if (modDesc) {
        return modDesc->disposition[disposition + 1];
    }

    return partyMemberDescription->disposition[disposition + 1];
}

// 0x495920
bool partyMemberSupportsAreaAttackMode(Object* object, int areaAttackMode)
{
    if (object == nullptr) {
        return false;
    }

    if (PID_TYPE(object->pid) != OBJ_TYPE_CRITTER) {
        return false;
    }

    if (areaAttackMode >= AREA_ATTACK_MODE_COUNT) {
        return false;
    }

    // Try base
    PartyMemberDescription* partyMemberDescription;
    if (partyMemberGetDescription(object, &partyMemberDescription) == -1) {
        return false;
    }

    // Try mod
    const PartyMemberDescription* modDesc = getModPartyMemberDescription(object);
    if (modDesc) {
        return modDesc->areaAttackMode[areaAttackMode];
    }

    return partyMemberDescription->areaAttackMode[areaAttackMode];
}

// 0x495980
bool partyMemberSupportsRunAwayMode(Object* object, int runAwayMode)
{
    if (object == nullptr) {
        return false;
    }

    if (PID_TYPE(object->pid) != OBJ_TYPE_CRITTER) {
        return false;
    }

    if (runAwayMode >= RUN_AWAY_MODE_COUNT) {
        return false;
    }

    // Try base
    PartyMemberDescription* partyMemberDescription;
    if (partyMemberGetDescription(object, &partyMemberDescription) == -1) {
        return false;
    }

    // Try mod
    const PartyMemberDescription* modDesc = getModPartyMemberDescription(object);
    if (modDesc) {
        return modDesc->runAwayMode[runAwayMode + 1];
    }

    return partyMemberDescription->runAwayMode[runAwayMode + 1];
}

// 0x4959E0
bool partyMemberSupportsBestWeapon(Object* object, int bestWeapon)
{
    if (object == nullptr) {
        return false;
    }

    if (PID_TYPE(object->pid) != OBJ_TYPE_CRITTER) {
        return false;
    }

    if (bestWeapon >= BEST_WEAPON_COUNT) {
        return false;
    }

    // Try bae
    PartyMemberDescription* partyMemberDescription;
    if (partyMemberGetDescription(object, &partyMemberDescription) == -1) {
        return false;
    }

    // Try mod
    const PartyMemberDescription* modDesc = getModPartyMemberDescription(object);
    if (modDesc) {
        return modDesc->bestWeapon[bestWeapon];
    }

    return partyMemberDescription->bestWeapon[bestWeapon];
}

// 0x495A40
bool partyMemberSupportsDistance(Object* object, int distanceMode)
{
    if (object == nullptr) {
        return false;
    }

    if (PID_TYPE(object->pid) != OBJ_TYPE_CRITTER) {
        return false;
    }

    if (distanceMode >= DISTANCE_COUNT) {
        return false;
    }

    // try base
    PartyMemberDescription* partyMemberDescription;
    if (partyMemberGetDescription(object, &partyMemberDescription) == -1) {
        return false;
    }

    // Try mod
    const PartyMemberDescription* modDesc = getModPartyMemberDescription(object);
    if (modDesc) {
        return modDesc->distanceMode[distanceMode];
    }

    return partyMemberDescription->distanceMode[distanceMode];
}

// 0x495AA0
bool partyMemberSupportsAttackWho(Object* object, int attackWho)
{
    if (object == nullptr) {
        return false;
    }

    if (PID_TYPE(object->pid) != OBJ_TYPE_CRITTER) {
        return false;
    }

    if (attackWho >= ATTACK_WHO_COUNT) {
        return false;
    }

    // try base
    PartyMemberDescription* partyMemberDescription;
    if (partyMemberGetDescription(object, &partyMemberDescription) == -1) {
        return false;
    }

    // Try mod
    const PartyMemberDescription* modDesc = getModPartyMemberDescription(object);
    if (modDesc) {
        return modDesc->attackWho[attackWho];
    }

    return partyMemberDescription->attackWho[attackWho];
}

// 0x495B00
bool partyMemberSupportsChemUse(Object* object, int chemUse)
{
    if (object == nullptr) {
        return false;
    }

    if (PID_TYPE(object->pid) != OBJ_TYPE_CRITTER) {
        return false;
    }

    if (chemUse >= CHEM_USE_COUNT) {
        return false;
    }

    // try base
    PartyMemberDescription* partyMemberDescription;
    if (partyMemberGetDescription(object, &partyMemberDescription) == -1) {
        return false;
    }

    // Try mod
    const PartyMemberDescription* modDesc = getModPartyMemberDescription(object);
    if (modDesc) {
        return modDesc->chemUse[chemUse];
    }

    return partyMemberDescription->chemUse[chemUse];
}

// partyMemberIncLevels
// 0x495B60
// 0x495B60
int _partyMemberIncLevels()
{
    int i;
    PartyMemberListItem* listItem;
    Object* obj;
    PartyMemberDescription* memberDescription;
    const char* name;
    int j;
    int memberIndex;
    PartyMemberLevelUpInfo* levelUpInfo;
    int levelMod;
    char* text;
    MessageListItem msg;
    char str[260];
    Rect levelUpMessageRect;

    for (i = 1; i < gPartyMembersLength; i++) {
        listItem = &(gPartyMembers[i]);
        obj = listItem->object;

        // --- Determine if this is a base companion ---
        bool isBase = false;
        int baseIdx = -1;
        for (j = 1; j < gPartyMemberDescriptionsLength; j++) {
            if (gPartyMemberPids[j] == obj->pid) {
                isBase = true;
                baseIdx = j;
                break;
            }
        }

        if (isBase) {
            // ===== ORIGINAL BASE COMPANION LOGIC (UNCHANGED) =====
            if (partyMemberGetDescription(obj, &memberDescription) == -1) {
                // SFALL: NPC level fix.
                continue;
            }

            if (PID_TYPE(obj->pid) != OBJ_TYPE_CRITTER) {
                continue;
            }

            name = critterGetName(obj);
            debugPrint("\npartyMemberIncLevels: %s", name);

            if (memberDescription->level_up_every == 0) {
                continue;
            }

            // memberIndex already found above, but original code had a loop here.
            // We'll use baseIdx directly.
            memberIndex = baseIdx;

            if (pcGetStat(PC_STAT_LEVEL) < memberDescription->level_minimum) {
                continue;
            }

            levelUpInfo = &(_partyMemberLevelUpInfoList[memberIndex]);

            if (levelUpInfo->level >= memberDescription->level_pids_num) {
                continue;
            }

            levelUpInfo->numLevelUps++;

            levelMod = levelUpInfo->numLevelUps % memberDescription->level_up_every;
            debugPrint("pm: levelMod: %d, Lvl: %d, Early: %d, Every: %d", levelMod, levelUpInfo->numLevelUps, levelUpInfo->isEarly, memberDescription->level_up_every);

            // Party member level up with a probability that depends on how "far" we are in the current "level_up_every" progression.
            // For example, if level_up_every is 5 and NPC observed 7 level ups with the player, 5 % 7 = 2, 2 * 100 / 5 = 40 (40% probability).
            // If levelMod is 0 (so we got 5, 10, etc. levels in the example above), probability is 100% (no roll).
            // If previous level up occured "early" (due to probability roll), then we skip until we get to levelMod = 0, to begin the next cycle.

            if (levelUpInfo->isEarly != 0) {
                if (levelMod == 0) {
                    levelUpInfo->isEarly = 0;
                }
                continue;
            }

            if (levelMod != 0 && randomBetween(0, 100) > 100 * levelMod / memberDescription->level_up_every) {
                continue;
            }

            levelUpInfo->level++;
            if (levelMod != 0) {
                levelUpInfo->isEarly = 1;
            }

            if (_partyMemberCopyLevelInfo(obj, memberDescription->level_pids[levelUpInfo->level]) == -1) {
                return -1;
            }

            name = critterGetName(obj);
            // %s has gained in some abilities.
            text = getmsg(&gMiscMessageList, &msg, 9000);
            snprintf(str, sizeof(str), text, name);
            displayMonitorAddMessage(str);

            debugPrint(str);

            // Individual message
            msg.num = 9000 + 10 * memberIndex + levelUpInfo->level - 1;
            if (messageListGetItem(&gMiscMessageList, &msg)) {
                name = critterGetName(obj);
                snprintf(str, sizeof(str), msg.text, name);
                textObjectAdd(obj, str, 101, _colorTable[0x7FFF], _colorTable[0], &levelUpMessageRect);
                tileWindowRefreshRect(&levelUpMessageRect, obj->elevation);
            }
        } else {
            // ===== MOD COMPANION LOGIC (mirrors base, using mod containers) =====
            auto objIt = gObjectToStableId.find(obj);
            if (objIt == gObjectToStableId.end()) continue; // not a mod companion

            uint32_t stableId = objIt->second;

            // Find mod description and its index (for message numbering)
            const ModPartyMember* modMember = nullptr;
            int modIndex = -1;
            for (size_t idx = 0; idx < gModPartyMembers.size(); idx++) {
                if (gModPartyMembers[idx].stableId == stableId) {
                    modMember = &gModPartyMembers[idx];
                    modIndex = idx;
                    break;
                }
            }
            if (!modMember) continue;

            const PartyMemberDescription* modDesc = &modMember->desc;

            if (PID_TYPE(obj->pid) != OBJ_TYPE_CRITTER) {
                continue;
            }

            name = critterGetName(obj);
            debugPrint("\npartyMemberIncLevels: %s", name);

            if (modDesc->level_up_every == 0) {
                continue;
            }

            if (pcGetStat(PC_STAT_LEVEL) < modDesc->level_minimum) {
                continue;
            }

            auto& levelInfo = gModPartyMemberLevelUpInfo[stableId]; // creates if not exist

            if (levelInfo.level >= modDesc->level_pids_num) {
                continue;
            }

            levelInfo.numLevelUps++;

            levelMod = levelInfo.numLevelUps % modDesc->level_up_every;
            debugPrint("pm: levelMod: %d, Lvl: %d, Early: %d, Every: %d", levelMod, levelInfo.numLevelUps, levelInfo.isEarly, modDesc->level_up_every);

            // Same early?skip logic
            if (levelInfo.isEarly != 0) {
                if (levelMod == 0) {
                    levelInfo.isEarly = 0;
                }
                continue;
            }

            if (levelMod != 0 && randomBetween(0, 100) > 100 * levelMod / modDesc->level_up_every) {
                continue;
            }

            levelInfo.level++;
            if (levelMod != 0) {
                levelInfo.isEarly = 1;
            }

            int stagePid = modDesc->level_pids[levelInfo.level];
            if (_partyMemberCopyLevelInfo(obj, stagePid) == -1) {
                return -1;
            }

            name = critterGetName(obj);
            text = getmsg(&gMiscMessageList, &msg, 9000);
            snprintf(str, sizeof(str), text, name);
            displayMonitorAddMessage(str);
            debugPrint(str);

            // Individual message for mod companions (range 10000+)
            msg.num = 10000 + modIndex * 10 + levelInfo.level - 1;
            if (messageListGetItem(&gMiscMessageList, &msg)) {
                name = critterGetName(obj);
                snprintf(str, sizeof(str), msg.text, name);
                textObjectAdd(obj, str, 101, _colorTable[0x7FFF], _colorTable[0], &levelUpMessageRect);
                tileWindowRefreshRect(&levelUpMessageRect, obj->elevation);
            }
        }
    }

    return 0;
}

// 0x495EA8
static int _partyMemberCopyLevelInfo(Object* critter, int stagePid)
{
    if (critter == nullptr) {
        return -1;
    }

    if (stagePid == -1) {
        return -1;
    }

    Proto* proto;
    if (protoGetProto(critter->pid, &proto) == -1) {
        return -1;
    }

    Proto* stageProto;
    if (protoGetProto(stagePid, &stageProto) == -1) {
        return -1;
    }

    Object* item2 = critterGetItem2(critter);
    inventoryUnequipFunc(critter, 1, 0);

    Object* armor = critterGetArmor(critter);
    adjustCritterStatsOnArmorChange(critter, armor, nullptr);
    itemRemove(critter, armor, 1);

    int maxHp = critterGetStat(critter, STAT_MAXIMUM_HIT_POINTS);
    critterAdjustHitPoints(critter, maxHp);

    for (int stat = 0; stat < SPECIAL_STAT_COUNT; stat++) {
        proto->critter.data.baseStats[stat] = stageProto->critter.data.baseStats[stat];
    }

    for (int stat = 0; stat < SPECIAL_STAT_COUNT; stat++) {
        proto->critter.data.bonusStats[stat] = stageProto->critter.data.bonusStats[stat];
    }

    for (int skill = 0; skill < SKILL_COUNT; skill++) {
        proto->critter.data.skills[skill] = stageProto->critter.data.skills[skill];
    }

    critter->data.critter.hp = critterGetStat(critter, STAT_MAXIMUM_HIT_POINTS);

    if (armor != nullptr) {
        itemAdd(critter, armor, 1);
        inventoryEquip(critter, armor, 0);
    }

    if (item2 != nullptr) {
        // SFALL: Fix for party member's equipped weapon being placed in the
        // incorrect item slot after leveling up.
        inventoryEquipFunc(critter, item2, HAND_RIGHT, false);
    }

    return 0;
}

// Returns `true` if any party member that can be healed thru the rest is
// wounded.
//
// This function is used to determine if any party member needs healing thru
// the "Rest until party healed", therefore it excludes robots in the party
// (they cannot be healed by resting) and dude (he/she has it's own "Rest
// until healed" option).
//
// 0x496058
bool partyIsAnyoneCanBeHealedByRest()
{
    for (int index = 1; index < gPartyMembersLength; index++) {
        PartyMemberListItem* ptr = &(gPartyMembers[index]);
        Object* object = ptr->object;

        if (PID_TYPE(object->pid) != OBJ_TYPE_CRITTER)
            continue;
        if (critterIsDead(object))
            continue;
        if ((object->flags & OBJECT_HIDDEN) != 0)
            continue;
        if (critterGetKillType(object) == KILL_TYPE_ROBOT)
            continue;

        int currentHp = critterGetHitPoints(object);
        int maximumHp = critterGetStat(object, STAT_MAXIMUM_HIT_POINTS);
        if (currentHp < maximumHp) {
            return true;
        }
    }

    return false;
}

// Returns maximum amount of damage of any party member that can be healed thru
// the rest.
//
// 0x4960DC
int partyGetMaxWoundToHealByRest()
{
    int maxWound = 0;

    for (int index = 1; index < gPartyMembersLength; index++) {
        PartyMemberListItem* ptr = &(gPartyMembers[index]);
        Object* object = ptr->object;

        if (PID_TYPE(object->pid) != OBJ_TYPE_CRITTER)
            continue;
        if (critterIsDead(object))
            continue;
        if ((object->flags & OBJECT_HIDDEN) != 0)
            continue;
        if (critterGetKillType(object) == KILL_TYPE_ROBOT)
            continue;

        int currentHp = critterGetHitPoints(object);
        int maximumHp = critterGetStat(object, STAT_MAXIMUM_HIT_POINTS);
        int wound = maximumHp - currentHp;
        if (wound > 0) {
            if (wound > maxWound) {
                maxWound = wound;
            }
        }
    }

    return maxWound;
}

std::vector<Object*> get_all_party_members_objects(bool include_hidden)
{
    std::vector<Object*> value;
    value.reserve(gPartyMembersLength);
    for (int index = 0; index < gPartyMembersLength; index++) {
        auto object = gPartyMembers[index].object;
        if (include_hidden
            || (PID_TYPE(object->pid) == OBJ_TYPE_CRITTER
                && !critterIsDead(object)
                && (object->flags & OBJECT_HIDDEN) == 0)) {
            value.push_back(object);
        }
    }
    return value;
}

} // namespace fallout
