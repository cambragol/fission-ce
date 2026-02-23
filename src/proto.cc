#include "proto.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#define DIR_SEPARATOR '/'

#include "art.h"
#include "character_editor.h"
#include "combat.h"
#include "config.h"
#include "critter.h"
#include "debug.h"
#include "dialog.h"
#include "game.h"
#include "game_movie.h"
#include "interface.h"
#include "map.h"
#include "memory.h"
#include "object.h"
#include "perk.h"
#include "settings.h"
#include "skill.h"
#include "stat.h"
#include "trait.h"
#include "window_manager.h"

namespace fallout {

static int objectCritterCombatDataRead(CritterCombatData* data, File* stream);
static int objectCritterCombatDataWrite(CritterCombatData* data, File* stream);
static int _proto_update_gen(Object* obj);
static int _proto_header_load();
static int protoItemDataRead(ItemProtoData* item_data, int type, File* stream);
static int protoSceneryDataRead(SceneryProtoData* scenery_data, int type, File* stream);
static int protoRead(Proto* buf, File* stream);
static int protoItemDataWrite(ItemProtoData* item_data, int type, File* stream);
static int protoSceneryDataWrite(SceneryProtoData* scenery_data, int type, File* stream);
static int protoWrite(Proto* buf, File* stream);
static int _proto_load_pid(int pid, Proto** out_proto);
static int _proto_find_free_subnode(int type, Proto** out_ptr);
static void _proto_remove_some_list(int type);
static void _proto_remove_list(int type);
static int _proto_new_id(int type);

// These functions extend the vanilla proto system to support mods.
// Mod PIDs use range 0x0000C8-0xFFFFFF (lower 24 bits).

static bool pid_is_modded(int pid);
static uint32_t proto_hash_string(const char* str);
static int generate_mod_proto_pid(const char* mod_name, const char* proto_name, int proto_type);

// Registry management
static void mod_proto_registry_add(const ModProtoEntry* entry);
static ModProtoEntry* mod_proto_registry_find_by_pid(int pid);
static ModProtoEntry* mod_proto_registry_find_by_key(const char* key);
static void name_to_pid_registry_add(const char* key, int pid);
static int name_to_pid_registry_find(const char* key);

// Loading
static void load_single_mod_proto_list(const char* list_path, const char* mod_name,
    int proto_type, const char* proto_type_name);
static void load_mod_proto_list(int proto_type, const char* proto_type_name);
static void load_mod_proto_messages_from_file(const char* full_path, const char* filename);
void load_mod_proto_messages(); // Public API
int protoGetModPid(const char* mod_name, const char* proto_name, int proto_type); // Public API

// Debug/Reporting
static void protoGenerateModProtoListDebug();

// 0x50CF3C
static char _aProto_0[] = "proto\\";

// 0x50D1B0
static char _aDrugStatSpecia[] = "Drug Stat (Special)";

// 0x50D1C4
static char _aNone_1[] = "None";

// 0x51C18C
char _cd_path_base[COMPAT_MAX_PATH];

// 0x51C290
static ProtoList _protoLists[11] = {
    { nullptr, nullptr, 0, 1 },
    { nullptr, nullptr, 0, 1 },
    { nullptr, nullptr, 0, 1 },
    { nullptr, nullptr, 0, 1 },
    { nullptr, nullptr, 0, 1 },
    { nullptr, nullptr, 0, 1 },
    { nullptr, nullptr, 0, 1 },
    { nullptr, nullptr, 0, 0 },
    { nullptr, nullptr, 0, 0 },
    { nullptr, nullptr, 0, 0 },
    { nullptr, nullptr, 0, 0 },
};

// 0x51C340
static const size_t _proto_sizes[11] = {
    sizeof(ItemProto), // 0x84
    sizeof(CritterProto), // 0x1A0
    sizeof(SceneryProto), // 0x38
    sizeof(WallProto), // 0x24
    sizeof(TileProto), // 0x1C
    sizeof(MiscProto), // 0x1C
    0,
    0,
    0,
    0,
    0,
};

// 0x51C36C
static int _protos_been_initialized = 0;

// obj_dude_proto
// 0x51C370
static CritterProto gDudeProto = {
    0x1000000,
    -1,
    0x1000001,
    0,
    0,
    0x20000000,
    0,
    -1,
    0,
    { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 18, 0, 0, 0, 0, 0, 0, 0, 0, 100, 0, 0, 0, 23, 0 },
    { 0 },
    { 0 },
    0,
    0,
    0,
    0,
    -1,
    0,
    0,
};

// 0x51C534
static char* _proto_path_base = _aProto_0;

// 0x51C538
static int _init_true = 0;

// 0x51C53C
static int _retval = 0;

// 0x66452C
static char* _mp_perk_code_None;

// 0x664530
static char* _mp_perk_code_strs[PERK_COUNT];

// 0x66470C
static char* _mp_critter_stats_list;

// 0x664710
static char* _critter_stats_list_None;

// 0x664714
static char* _critter_stats_list_strs[STAT_COUNT];

// Message list by object type
// 0 - pro_item.msg
// 1 - pro_crit.msg
// 2 - pro_scen.msg
// 3 - pro_wall.msg
// 4 - pro_tile.msg
// 5 - pro_misc.msg
//
// 0x6647AC
static MessageList _proto_msg_files[6];

// 0x6647DC
static char* gRaceTypeNames[RACE_TYPE_COUNT];

// 0x6647E4
static char* gSceneryTypeNames[SCENERY_TYPE_COUNT];

// proto.msg
//
// 0x6647FC
MessageList gProtoMessageList;

// 0x664804
static char* gMaterialTypeNames[MATERIAL_TYPE_COUNT];

// "<None>" from proto.msg
//
// 0x664824
char* _proto_none_str;

// 0x664828
static char* gBodyTypeNames[BODY_TYPE_COUNT];

// 0x664834
char* gItemTypeNames[ITEM_TYPE_COUNT];

// 0x66484C
static char* gDamageTypeNames[DAMAGE_TYPE_COUNT];

// 0x66486C
static char* gCaliberTypeNames[CALIBER_TYPE_COUNT];

// Perk names.
//
// 0x6648B8
static char** _perk_code_strs;

// Stat names.
//
// 0x6648BC
static char** _critter_stats_list;

// 0x51C540 (example new address - pick an unused one)
static ModProtoEntry* _mod_proto_entries = nullptr;
static int _mod_proto_entries_size = 0;
static int _mod_proto_entries_capacity = 0;

static NameToPidEntry* _name_to_pid_entries = nullptr;
static int _name_to_pid_entries_size = 0;
static int _name_to_pid_entries_capacity = 0;

// Collision tracking variables
static int _mod_proto_collision_count = 0;
static char _mod_proto_collision_log[4096] = ""; // Buffer for collision log

// ============================================================================
// Core Helper Functions (used everywhere)
// ============================================================================

// Count .pro lines in .lst files.
//
// 0x4A08E0
static int _proto_header_load()
{
    for (int index = 0; index < 6; index++) {
        ProtoList* ptr = &(_protoLists[index]);
        ptr->head = nullptr;
        ptr->tail = nullptr;
        ptr->length = 0;
        ptr->max_entries_num = 1;

        char path[COMPAT_MAX_PATH];
        proto_make_path(path, index << 24);
        strcat(path, "\\");
        strcat(path, artGetObjectTypeName(index));
        strcat(path, ".lst");

        File* stream = fileOpen(path, "rt");
        if (stream == nullptr) {
            return -1;
        }

        int ch = '\0';
        while (1) {
            ch = fileReadChar(stream);
            if (ch == -1) {
                break;
            }

            if (ch == '\n') {
                ptr->max_entries_num++;
            }
        }

        if (ch != '\n') {
            ptr->max_entries_num++;
        }

        fileClose(stream);
    }

    return 0;
}

// 0x4A0AEC
static int protoItemDataRead(ItemProtoData* item_data, int type, File* stream)
{
    switch (type) {
    case ITEM_TYPE_ARMOR:
        if (fileReadInt32(stream, &(item_data->armor.armorClass)) == -1)
            return -1;
        if (fileReadInt32List(stream, item_data->armor.damageResistance, 7) == -1)
            return -1;
        if (fileReadInt32List(stream, item_data->armor.damageThreshold, 7) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->armor.perk)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->armor.maleFid)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->armor.femaleFid)) == -1)
            return -1;

        return 0;
    case ITEM_TYPE_CONTAINER:
        if (fileReadInt32(stream, &(item_data->container.maxSize)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->container.openFlags)) == -1)
            return -1;

        return 0;
    case ITEM_TYPE_DRUG:
        if (fileReadInt32(stream, &(item_data->drug.stat[0])) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->drug.stat[1])) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->drug.stat[2])) == -1)
            return -1;
        if (fileReadInt32List(stream, item_data->drug.amount, 3) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->drug.duration1)) == -1)
            return -1;
        if (fileReadInt32List(stream, item_data->drug.amount1, 3) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->drug.duration2)) == -1)
            return -1;
        if (fileReadInt32List(stream, item_data->drug.amount2, 3) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->drug.addictionChance)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->drug.withdrawalEffect)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->drug.withdrawalOnset)) == -1)
            return -1;

        return 0;
    case ITEM_TYPE_WEAPON:
        if (fileReadInt32(stream, &(item_data->weapon.animationCode)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.minDamage)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.maxDamage)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.damageType)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.maxRange1)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.maxRange2)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.projectilePid)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.minStrength)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.actionPointCost1)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.actionPointCost2)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.criticalFailureType)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.perk)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.rounds)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.caliber)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.ammoTypePid)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->weapon.ammoCapacity)) == -1)
            return -1;
        if (fileReadUInt8(stream, &(item_data->weapon.soundCode)) == -1)
            return -1;

        return 0;
    case ITEM_TYPE_AMMO:
        if (fileReadInt32(stream, &(item_data->ammo.caliber)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->ammo.quantity)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->ammo.armorClassModifier)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->ammo.damageResistanceModifier)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->ammo.damageMultiplier)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->ammo.damageDivisor)) == -1)
            return -1;

        return 0;
    case ITEM_TYPE_MISC:
        if (fileReadInt32(stream, &(item_data->misc.powerTypePid)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->misc.powerType)) == -1)
            return -1;
        if (fileReadInt32(stream, &(item_data->misc.charges)) == -1)
            return -1;

        return 0;
    case ITEM_TYPE_KEY:
        if (fileReadInt32(stream, &(item_data->key.keyCode)) == -1)
            return -1;

        return 0;
    }

    return 0;
}

// 0x4A0ED0
static int protoSceneryDataRead(SceneryProtoData* scenery_data, int type, File* stream)
{
    switch (type) {
    case SCENERY_TYPE_DOOR:
        if (fileReadInt32(stream, &(scenery_data->door.openFlags)) == -1)
            return -1;
        if (fileReadInt32(stream, &(scenery_data->door.keyCode)) == -1)
            return -1;

        return 0;
    case SCENERY_TYPE_STAIRS:
        if (fileReadInt32(stream, &(scenery_data->stairs.field_0)) == -1)
            return -1;
        if (fileReadInt32(stream, &(scenery_data->stairs.field_4)) == -1)
            return -1;

        return 0;
    case SCENERY_TYPE_ELEVATOR:
        if (fileReadInt32(stream, &(scenery_data->elevator.type)) == -1)
            return -1;
        if (fileReadInt32(stream, &(scenery_data->elevator.level)) == -1)
            return -1;

        return 0;
    case SCENERY_TYPE_LADDER_UP:
    case SCENERY_TYPE_LADDER_DOWN:
        if (fileReadInt32(stream, &(scenery_data->ladder.field_0)) == -1)
            return -1;

        return 0;
    case SCENERY_TYPE_GENERIC:
        if (fileReadInt32(stream, &(scenery_data->generic.field_0)) == -1)
            return -1;

        return 0;
    }

    return 0;
}

// read .pro file
// 0x4A0FA0
static int protoRead(Proto* proto, File* stream)
{
    if (fileReadInt32(stream, &(proto->pid)) == -1)
        return -1;
    if (fileReadInt32(stream, &(proto->messageId)) == -1)
        return -1;
    if (fileReadInt32(stream, &(proto->fid)) == -1)
        return -1;

    switch (PID_TYPE(proto->pid)) {
    case OBJ_TYPE_ITEM:
        if (fileReadInt32(stream, &(proto->item.lightDistance)) == -1)
            return -1;
        if (_db_freadInt(stream, &(proto->item.lightIntensity)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->item.flags)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->item.extendedFlags)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->item.sid)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->item.type)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->item.material)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->item.size)) == -1)
            return -1;
        if (_db_freadInt(stream, &(proto->item.weight)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->item.cost)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->item.inventoryFid)) == -1)
            return -1;
        if (fileReadUInt8(stream, &(proto->item.field_80)) == -1)
            return -1;
        if (protoItemDataRead(&(proto->item.data), proto->item.type, stream) == -1)
            return -1;

        return 0;
    case OBJ_TYPE_CRITTER:
        if (fileReadInt32(stream, &(proto->critter.lightDistance)) == -1)
            return -1;
        if (_db_freadInt(stream, &(proto->critter.lightIntensity)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->critter.flags)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->critter.extendedFlags)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->critter.sid)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->critter.headFid)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->critter.aiPacket)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->critter.team)) == -1)
            return -1;

        if (protoCritterDataRead(stream, &(proto->critter.data)) == -1)
            return -1;

        return 0;
    case OBJ_TYPE_SCENERY:
        if (fileReadInt32(stream, &(proto->scenery.lightDistance)) == -1)
            return -1;
        if (_db_freadInt(stream, &(proto->scenery.lightIntensity)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->scenery.flags)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->scenery.extendedFlags)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->scenery.sid)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->scenery.type)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->scenery.field_2C)) == -1)
            return -1;
        if (fileReadUInt8(stream, &(proto->scenery.field_34)) == -1)
            return -1;
        if (protoSceneryDataRead(&(proto->scenery.data), proto->scenery.type, stream) == -1)
            return -1;
        return 0;
    case OBJ_TYPE_WALL:
        if (fileReadInt32(stream, &(proto->wall.lightDistance)) == -1)
            return -1;
        if (_db_freadInt(stream, &(proto->wall.lightIntensity)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->wall.flags)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->wall.extendedFlags)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->wall.sid)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->wall.material)) == -1)
            return -1;

        return 0;
    case OBJ_TYPE_TILE:
        if (fileReadInt32(stream, &(proto->tile.flags)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->tile.extendedFlags)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->tile.sid)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->tile.material)) == -1)
            return -1;

        return 0;
    case OBJ_TYPE_MISC:
        if (fileReadInt32(stream, &(proto->misc.lightDistance)) == -1)
            return -1;
        if (_db_freadInt(stream, &(proto->misc.lightIntensity)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->misc.flags)) == -1)
            return -1;
        if (fileReadInt32(stream, &(proto->misc.extendedFlags)) == -1)
            return -1;

        return 0;
    }

    return -1;
}

// 0x4A1390
static int protoItemDataWrite(ItemProtoData* item_data, int type, File* stream)
{
    switch (type) {
    case ITEM_TYPE_ARMOR:
        if (fileWriteInt32(stream, item_data->armor.armorClass) == -1)
            return -1;
        if (fileWriteInt32List(stream, item_data->armor.damageResistance, 7) == -1)
            return -1;
        if (fileWriteInt32List(stream, item_data->armor.damageThreshold, 7) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->armor.perk) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->armor.maleFid) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->armor.femaleFid) == -1)
            return -1;

        return 0;
    case ITEM_TYPE_CONTAINER:
        if (fileWriteInt32(stream, item_data->container.maxSize) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->container.openFlags) == -1)
            return -1;

        return 0;
    case ITEM_TYPE_DRUG:
        if (fileWriteInt32(stream, item_data->drug.stat[0]) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->drug.stat[1]) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->drug.stat[2]) == -1)
            return -1;
        if (fileWriteInt32List(stream, item_data->drug.amount, 3) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->drug.duration1) == -1)
            return -1;
        if (fileWriteInt32List(stream, item_data->drug.amount1, 3) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->drug.duration2) == -1)
            return -1;
        if (fileWriteInt32List(stream, item_data->drug.amount2, 3) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->drug.addictionChance) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->drug.withdrawalEffect) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->drug.withdrawalOnset) == -1)
            return -1;

        return 0;
    case ITEM_TYPE_WEAPON:
        if (fileWriteInt32(stream, item_data->weapon.animationCode) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.maxDamage) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.minDamage) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.damageType) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.maxRange1) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.maxRange2) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.projectilePid) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.minStrength) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.actionPointCost1) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.actionPointCost2) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.criticalFailureType) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.perk) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.rounds) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.caliber) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.ammoTypePid) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->weapon.ammoCapacity) == -1)
            return -1;
        if (fileWriteUInt8(stream, item_data->weapon.soundCode) == -1)
            return -1;

        return 0;
    case ITEM_TYPE_AMMO:
        if (fileWriteInt32(stream, item_data->ammo.caliber) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->ammo.quantity) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->ammo.armorClassModifier) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->ammo.damageResistanceModifier) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->ammo.damageMultiplier) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->ammo.damageDivisor) == -1)
            return -1;

        return 0;
    case ITEM_TYPE_MISC:
        if (fileWriteInt32(stream, item_data->misc.powerTypePid) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->misc.powerType) == -1)
            return -1;
        if (fileWriteInt32(stream, item_data->misc.charges) == -1)
            return -1;

        return 0;
    case ITEM_TYPE_KEY:
        if (fileWriteInt32(stream, item_data->key.keyCode) == -1)
            return -1;

        return 0;
    }

    return 0;
}

// 0x4A16E4
static int protoSceneryDataWrite(SceneryProtoData* scenery_data, int type, File* stream)
{
    switch (type) {
    case SCENERY_TYPE_DOOR:
        if (fileWriteInt32(stream, scenery_data->door.openFlags) == -1)
            return -1;
        if (fileWriteInt32(stream, scenery_data->door.keyCode) == -1)
            return -1;

        return 0;
    case SCENERY_TYPE_STAIRS:
        if (fileWriteInt32(stream, scenery_data->stairs.field_0) == -1)
            return -1;
        if (fileWriteInt32(stream, scenery_data->stairs.field_4) == -1)
            return -1;

        return 0;
    case SCENERY_TYPE_ELEVATOR:
        if (fileWriteInt32(stream, scenery_data->elevator.type) == -1)
            return -1;
        if (fileWriteInt32(stream, scenery_data->elevator.level) == -1)
            return -1;

        return 0;
    case SCENERY_TYPE_LADDER_UP:
    case SCENERY_TYPE_LADDER_DOWN:
        if (fileWriteInt32(stream, scenery_data->ladder.field_0) == -1)
            return -1;

        return 0;
    case SCENERY_TYPE_GENERIC:
        if (fileWriteInt32(stream, scenery_data->generic.field_0) == -1)
            return -1;

        return 0;
    }

    return 0;
}

// 0x4A17B4
static int protoWrite(Proto* proto, File* stream)
{
    if (fileWriteInt32(stream, proto->pid) == -1)
        return -1;
    if (fileWriteInt32(stream, proto->messageId) == -1)
        return -1;
    if (fileWriteInt32(stream, proto->fid) == -1)
        return -1;

    switch (PID_TYPE(proto->pid)) {
    case OBJ_TYPE_ITEM:
        if (fileWriteInt32(stream, proto->item.lightDistance) == -1)
            return -1;
        if (_db_fwriteLong(stream, proto->item.lightIntensity) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->item.flags) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->item.extendedFlags) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->item.sid) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->item.type) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->item.material) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->item.size) == -1)
            return -1;
        if (_db_fwriteLong(stream, proto->item.weight) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->item.cost) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->item.inventoryFid) == -1)
            return -1;
        if (fileWriteUInt8(stream, proto->item.field_80) == -1)
            return -1;
        if (protoItemDataWrite(&(proto->item.data), proto->item.type, stream) == -1)
            return -1;

        return 0;
    case OBJ_TYPE_CRITTER:
        if (fileWriteInt32(stream, proto->critter.lightDistance) == -1)
            return -1;
        if (_db_fwriteLong(stream, proto->critter.lightIntensity) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->critter.flags) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->critter.extendedFlags) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->critter.sid) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->critter.headFid) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->critter.aiPacket) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->critter.team) == -1)
            return -1;
        if (protoCritterDataWrite(stream, &(proto->critter.data)) == -1)
            return -1;

        return 0;
    case OBJ_TYPE_SCENERY:
        if (fileWriteInt32(stream, proto->scenery.lightDistance) == -1)
            return -1;
        if (_db_fwriteLong(stream, proto->scenery.lightIntensity) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->scenery.flags) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->scenery.extendedFlags) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->scenery.sid) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->scenery.type) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->scenery.field_2C) == -1)
            return -1;
        if (fileWriteUInt8(stream, proto->scenery.field_34) == -1)
            return -1;
        if (protoSceneryDataWrite(&(proto->scenery.data), proto->scenery.type, stream) == -1)
            return -1;
    case OBJ_TYPE_WALL:
        if (fileWriteInt32(stream, proto->wall.lightDistance) == -1)
            return -1;
        if (_db_fwriteLong(stream, proto->wall.lightIntensity) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->wall.flags) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->wall.extendedFlags) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->wall.sid) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->wall.material) == -1)
            return -1;

        return 0;
    case OBJ_TYPE_TILE:
        if (fileWriteInt32(stream, proto->tile.flags) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->tile.extendedFlags) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->tile.sid) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->tile.material) == -1)
            return -1;

        return 0;
    case OBJ_TYPE_MISC:
        if (fileWriteInt32(stream, proto->misc.lightDistance) == -1)
            return -1;
        if (_db_fwriteLong(stream, proto->misc.lightIntensity) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->misc.flags) == -1)
            return -1;
        if (fileWriteInt32(stream, proto->misc.extendedFlags) == -1)
            return -1;

        return 0;
    }

    return -1;
}

// 0x4A1C3C
static int _proto_load_pid(int pid, Proto** protoPtr)
{
    char path[COMPAT_MAX_PATH];
    proto_make_path(path, pid);
    strcat(path, "\\");

    if (_proto_list_str(pid, path + strlen(path)) == -1) {
        return -1;
    }

    File* stream = fileOpen(path, "rb");
    if (stream == nullptr) {
        debugPrint("\nError: Can't fopen proto!\n");
        *protoPtr = nullptr;
        return -1;
    }

    if (_proto_find_free_subnode(PID_TYPE(pid), protoPtr) == -1) {
        fileClose(stream);
        return -1;
    }

    if (protoRead(*protoPtr, stream) != 0) {
        fileClose(stream);
        return -1;
    }

    fileClose(stream);
    return 0;
}

// 0x4A1D98
static int _proto_find_free_subnode(int type, Proto** protoPtr)
{
    Proto* proto = (Proto*)internal_malloc(proto_size(type));
    *protoPtr = proto;
    if (proto == nullptr) {
        return -1;
    }

    ProtoList* protoList = &(_protoLists[type]);
    ProtoListExtent* protoListExtent = protoList->tail;

    if (protoList->head != nullptr) {
        if (protoListExtent->length == PROTO_LIST_EXTENT_SIZE) {
            ProtoListExtent* newExtent = protoListExtent->next = (ProtoListExtent*)internal_malloc(sizeof(ProtoListExtent));
            if (protoListExtent == nullptr) {
                internal_free(proto);
                *protoPtr = nullptr;
                return -1;
            }

            newExtent->length = 0;
            newExtent->next = nullptr;

            protoList->tail = newExtent;
            protoList->length++;

            protoListExtent = newExtent;
        }
    } else {
        protoListExtent = (ProtoListExtent*)internal_malloc(sizeof(ProtoListExtent));
        if (protoListExtent == nullptr) {
            internal_free(proto);
            *protoPtr = nullptr;
            return -1;
        }

        protoListExtent->next = nullptr;
        protoListExtent->length = 0;

        protoList->length = 1;
        protoList->tail = protoListExtent;
        protoList->head = protoListExtent;
    }

    protoListExtent->proto[protoListExtent->length] = proto;
    protoListExtent->length++;

    return 0;
}

// Evict top most proto cache block.
//
// 0x4A2040
static void _proto_remove_some_list(int type)
{
    ProtoList* protoList = &(_protoLists[type]);
    ProtoListExtent* protoListExtent = protoList->head;
    if (protoListExtent != nullptr) {
        protoList->length--;
        protoList->head = protoListExtent->next;

        for (int index = 0; index < protoListExtent->length; index++) {
            internal_free(protoListExtent->proto[index]);
        }

        internal_free(protoListExtent);
    }
}

// Clear proto cache of given type.
//
// 0x4A2094
static void _proto_remove_list(int type)
{
    ProtoList* protoList = &(_protoLists[type]);

    ProtoListExtent* curr = protoList->head;
    while (curr != nullptr) {
        ProtoListExtent* next = curr->next;
        for (int index = 0; index < curr->length; index++) {
            internal_free(curr->proto[index]);
        }
        internal_free(curr);
        curr = next;
    }

    protoList->head = nullptr;
    protoList->tail = nullptr;
    protoList->length = 0;
}

// 0x4A21DC
static int _proto_new_id(int type)
{
    int result = _protoLists[type].max_entries_num;
    _protoLists[type].max_entries_num = result + 1;

    return result;
}

// 0x49EEB8
static int objectCritterCombatDataRead(CritterCombatData* data, File* stream)
{
    if (fileReadInt32(stream, &(data->damageLastTurn)) == -1)
        return -1;
    if (fileReadInt32(stream, &(data->maneuver)) == -1)
        return -1;
    if (fileReadInt32(stream, &(data->ap)) == -1)
        return -1;
    if (fileReadInt32(stream, &(data->results)) == -1)
        return -1;
    if (fileReadInt32(stream, &(data->aiPacket)) == -1)
        return -1;
    if (fileReadInt32(stream, &(data->team)) == -1)
        return -1;
    if (fileReadInt32(stream, &(data->whoHitMeCid)) == -1)
        return -1;

    return 0;
}

// 0x49EF40
static int objectCritterCombatDataWrite(CritterCombatData* data, File* stream)
{
    if (fileWriteInt32(stream, data->damageLastTurn) == -1)
        return -1;
    if (fileWriteInt32(stream, data->maneuver) == -1)
        return -1;
    if (fileWriteInt32(stream, data->ap) == -1)
        return -1;
    if (fileWriteInt32(stream, data->results) == -1)
        return -1;
    if (fileWriteInt32(stream, data->aiPacket) == -1)
        return -1;
    if (fileWriteInt32(stream, data->team) == -1)
        return -1;
    if (fileWriteInt32(stream, data->whoHitMeCid) == -1)
        return -1;

    return 0;
}

// 0x49F73C
static int _proto_update_gen(Object* obj)
{
    Proto* proto;

    if (!_protos_been_initialized) {
        return -1;
    }

    ObjectData* data = &(obj->data);
    data->inventory.length = 0;
    data->inventory.capacity = 0;
    data->inventory.items = nullptr;

    if (protoGetProto(obj->pid, &proto) == -1) {
        return -1;
    }

    switch (PID_TYPE(obj->pid)) {
    case OBJ_TYPE_ITEM:
        switch (proto->item.type) {
        case ITEM_TYPE_CONTAINER:
            data->flags = 0;
            break;
        case ITEM_TYPE_WEAPON:
            data->item.weapon.ammoQuantity = proto->item.data.weapon.ammoCapacity;
            data->item.weapon.ammoTypePid = proto->item.data.weapon.ammoTypePid;
            break;
        case ITEM_TYPE_AMMO:
            data->item.ammo.quantity = proto->item.data.ammo.quantity;
            break;
        case ITEM_TYPE_MISC:
            data->item.misc.charges = proto->item.data.misc.charges;
            break;
        case ITEM_TYPE_KEY:
            data->item.key.keyCode = proto->item.data.key.keyCode;
            break;
        }
        break;
    case OBJ_TYPE_SCENERY:
        switch (proto->scenery.type) {
        case SCENERY_TYPE_DOOR:
            data->scenery.door.openFlags = proto->scenery.data.door.openFlags;
            break;
        case SCENERY_TYPE_STAIRS:
            data->scenery.stairs.destinationBuiltTile = proto->scenery.data.stairs.field_0;
            data->scenery.stairs.destinationMap = proto->scenery.data.stairs.field_4;
            break;
        case SCENERY_TYPE_ELEVATOR:
            data->scenery.elevator.type = proto->scenery.data.elevator.type;
            data->scenery.elevator.level = proto->scenery.data.elevator.level;
            break;
        case SCENERY_TYPE_LADDER_UP:
        case SCENERY_TYPE_LADDER_DOWN:
            data->scenery.ladder.destinationMap = proto->scenery.data.ladder.field_0;
            break;
        }
        break;
    case OBJ_TYPE_MISC:
        if (isExitGridPid(obj->pid)) {
            data->misc.tile = -1;
            data->misc.elevation = 0;
            data->misc.rotation = 0;
            data->misc.map = -1;
        }
        break;
    default:
        break;
    }

    return 0;
}

// ============================================================================
// Core Proto Initialization Functions
// ============================================================================

// 0x49EB2C
int proto_item_init(Proto* proto, int a2)
{
    int v1 = a2 & 0xFFFFFF;

    proto->item.pid = -1;
    proto->item.messageId = 100 * v1;
    proto->item.fid = buildFid(OBJ_TYPE_ITEM, v1 - 1, 0, 0, 0);
    if (!artExists(proto->item.fid)) {
        proto->item.fid = buildFid(OBJ_TYPE_ITEM, 0, 0, 0, 0);
    }
    proto->item.lightDistance = 0;
    proto->item.lightIntensity = 0;
    proto->item.flags = 0xA0000008;
    proto->item.extendedFlags = 0xA000;
    proto->item.sid = -1;
    proto->item.type = ITEM_TYPE_MISC;
    proto_item_subdata_init(proto, proto->item.type);
    proto->item.material = 1;
    proto->item.size = 1;
    proto->item.weight = 10;
    proto->item.cost = 0;
    proto->item.inventoryFid = -1;
    proto->item.field_80 = '0';

    return 0;
}

// 0x49EBFC
int proto_item_subdata_init(Proto* proto, int type)
{
    int index;

    switch (type) {
    case ITEM_TYPE_ARMOR:
        proto->item.data.armor.armorClass = 0;

        for (index = 0; index < DAMAGE_TYPE_COUNT; index++) {
            proto->item.data.armor.damageResistance[index] = 0;
            proto->item.data.armor.damageThreshold[index] = 0;
        }

        proto->item.data.armor.perk = -1;
        proto->item.data.armor.maleFid = -1;
        proto->item.data.armor.femaleFid = -1;
        break;
    case ITEM_TYPE_CONTAINER:
        proto->item.data.container.openFlags = 0;
        proto->item.data.container.maxSize = 250;
        proto->item.extendedFlags |= 0x800;
        break;
    case ITEM_TYPE_DRUG:
        proto->item.data.drug.stat[0] = STAT_STRENGTH;
        proto->item.data.drug.stat[1] = -1;
        proto->item.data.drug.stat[2] = -1;
        proto->item.data.drug.amount[0] = 0;
        proto->item.data.drug.amount[1] = 0;
        proto->item.data.drug.amount[2] = 0;
        proto->item.data.drug.duration1 = 0;
        proto->item.data.drug.amount1[0] = 0;
        proto->item.data.drug.amount1[1] = 0;
        proto->item.data.drug.amount1[2] = 0;
        proto->item.data.drug.duration2 = 0;
        proto->item.data.drug.amount2[0] = 0;
        proto->item.data.drug.amount2[1] = 0;
        proto->item.data.drug.amount2[2] = 0;
        proto->item.data.drug.addictionChance = 0;
        proto->item.data.drug.withdrawalEffect = 0;
        proto->item.data.drug.withdrawalOnset = 0;
        proto->item.extendedFlags |= 0x1000;
        break;
    case ITEM_TYPE_WEAPON:
        proto->item.data.weapon.animationCode = 0;
        proto->item.data.weapon.minDamage = 0;
        proto->item.data.weapon.maxDamage = 0;
        proto->item.data.weapon.damageType = 0;
        proto->item.data.weapon.maxRange1 = 0;
        proto->item.data.weapon.maxRange2 = 0;
        proto->item.data.weapon.projectilePid = -1;
        proto->item.data.weapon.minStrength = 0;
        proto->item.data.weapon.actionPointCost1 = 0;
        proto->item.data.weapon.actionPointCost2 = 0;
        proto->item.data.weapon.criticalFailureType = 0;
        proto->item.data.weapon.perk = -1;
        proto->item.data.weapon.rounds = 0;
        proto->item.data.weapon.caliber = 0;
        proto->item.data.weapon.ammoTypePid = -1;
        proto->item.data.weapon.ammoCapacity = 0;
        proto->item.data.weapon.soundCode = 0;
        break;
    case ITEM_TYPE_AMMO:
        proto->item.data.ammo.caliber = 0;
        proto->item.data.ammo.quantity = 20;
        proto->item.data.ammo.armorClassModifier = 0;
        proto->item.data.ammo.damageResistanceModifier = 0;
        proto->item.data.ammo.damageMultiplier = 1;
        proto->item.data.ammo.damageDivisor = 1;
        break;
    case ITEM_TYPE_MISC:
        proto->item.data.misc.powerTypePid = -1;
        proto->item.data.misc.powerType = 20;
        break;
    case ITEM_TYPE_KEY:
        proto->item.data.key.keyCode = -1;
        proto->item.extendedFlags |= 0x1000;
        break;
    }

    return 0;
}

// 0x49EDB4
int proto_critter_init(Proto* proto, int pid)
{
    if (!_protos_been_initialized) {
        return -1;
    }

    int num = pid & 0xFFFFFF;

    proto->pid = -1;
    proto->messageId = 100 * num;
    proto->fid = buildFid(OBJ_TYPE_CRITTER, num - 1, 0, 0, 0);
    proto->critter.lightDistance = 0;
    proto->critter.lightIntensity = 0;
    proto->critter.flags = 0x20000000;
    proto->critter.extendedFlags = 0x6000;
    proto->critter.sid = -1;
    proto->critter.data.flags = 0;
    proto->critter.data.bodyType = 0;
    proto->critter.headFid = -1;
    proto->critter.aiPacket = 1;
    if (!artExists(proto->fid)) {
        proto->fid = buildFid(OBJ_TYPE_CRITTER, 0, 0, 0, 0);
    }

    CritterProtoData* data = &(proto->critter.data);
    data->experience = 60;
    data->killType = 0;
    data->damageType = 0;
    protoCritterDataResetStats(data);
    protoCritterDataResetSkills(data);

    return 0;
}

// 0x49FBBC
int proto_scenery_init(Proto* proto, int pid)
{
    int num = pid & 0xFFFFFF;

    proto->scenery.pid = -1;
    proto->scenery.messageId = 100 * num;
    proto->scenery.fid = buildFid(OBJ_TYPE_SCENERY, num - 1, 0, 0, 0);
    if (!artExists(proto->scenery.fid)) {
        proto->scenery.fid = buildFid(OBJ_TYPE_SCENERY, 0, 0, 0, 0);
    }
    proto->scenery.lightDistance = 0;
    proto->scenery.lightIntensity = 0;
    proto->scenery.flags = 0;
    proto->scenery.extendedFlags = 0x2000;
    proto->scenery.sid = -1;
    proto->scenery.type = SCENERY_TYPE_GENERIC;
    proto_scenery_subdata_init(proto, proto->scenery.type);
    proto->scenery.field_2C = -1;
    proto->scenery.field_34 = '0';

    return 0;
}

// 0x49FC74
int proto_scenery_subdata_init(Proto* proto, int type)
{
    switch (type) {
    case SCENERY_TYPE_DOOR:
        proto->scenery.data.door.openFlags = 0;
        proto->scenery.extendedFlags |= 0x800;
        break;
    case SCENERY_TYPE_STAIRS:
        proto->scenery.data.stairs.field_0 = -1;
        proto->scenery.data.stairs.field_4 = -1;
        proto->scenery.extendedFlags |= 0x800;
        break;
    case SCENERY_TYPE_ELEVATOR:
        proto->scenery.data.elevator.type = -1;
        proto->scenery.data.elevator.level = -1;
        proto->scenery.extendedFlags |= 0x800;
        break;
    case SCENERY_TYPE_LADDER_UP:
        proto->scenery.data.ladder.field_0 = -1;
        proto->scenery.extendedFlags |= 0x800;
        break;
    case SCENERY_TYPE_LADDER_DOWN:
        proto->scenery.data.ladder.field_0 = -1;
        proto->scenery.extendedFlags |= 0x800;
        break;
    }

    return 0;
}

// 0x49FCFC
int proto_wall_init(Proto* proto, int pid)
{
    int num = pid & 0xFFFFFF;

    proto->wall.pid = -1;
    proto->wall.messageId = 100 * num;
    proto->wall.fid = buildFid(OBJ_TYPE_WALL, num - 1, 0, 0, 0);
    if (!artExists(proto->wall.fid)) {
        proto->wall.fid = buildFid(OBJ_TYPE_WALL, 0, 0, 0, 0);
    }
    proto->wall.lightDistance = 0;
    proto->wall.lightIntensity = 0;
    proto->wall.flags = 0;
    proto->wall.extendedFlags = 0x2000;
    proto->wall.sid = -1;
    proto->wall.material = 1;

    return 0;
}

// 0x49FD84
int proto_tile_init(Proto* proto, int pid)
{
    int num = pid & 0xFFFFFF;

    proto->tile.pid = -1;
    proto->tile.messageId = 100 * num;
    proto->tile.fid = buildFid(OBJ_TYPE_TILE, num - 1, 0, 0, 0);
    if (!artExists(proto->tile.fid)) {
        proto->tile.fid = buildFid(OBJ_TYPE_TILE, 0, 0, 0, 0);
    }
    proto->tile.flags = 0;
    proto->tile.extendedFlags = 0x2000;
    proto->tile.sid = -1;
    proto->tile.material = 1;

    return 0;
}

// 0x49FDFC
int proto_misc_init(Proto* proto, int pid)
{
    int num = pid & 0xFFFFFF;

    proto->misc.pid = -1;
    proto->misc.messageId = 100 * num;
    proto->misc.fid = buildFid(OBJ_TYPE_MISC, num - 1, 0, 0, 0);
    if (!artExists(proto->misc.fid)) {
        proto->misc.fid = buildFid(OBJ_TYPE_MISC, 0, 0, 0, 0);
    }
    proto->misc.lightDistance = 0;
    proto->misc.lightIntensity = 0;
    proto->misc.flags = 0;
    proto->misc.extendedFlags = 0;

    return 0;
}

// 0x49FE74
int proto_copy_proto(int srcPid, int dstPid)
{
    int srcType;
    int dstType;
    Proto* src;
    Proto* dst;

    srcType = PID_TYPE(srcPid);
    dstType = PID_TYPE(dstPid);
    if (srcType != dstType) {
        return -1;
    }

    if (protoGetProto(srcPid, &src) == -1) {
        return -1;
    }

    if (protoGetProto(dstPid, &dst) == -1) {
        return -1;
    }

    memcpy(dst, src, _proto_sizes[srcType]);
    dst->pid = dstPid;

    return 0;
}

// 0x49FEDC
bool proto_is_subtype(Proto* proto, int subtype)
{
    if (subtype == -1) {
        return true;
    }

    switch (PID_TYPE(proto->pid)) {
    case OBJ_TYPE_ITEM:
        return proto->item.type == subtype;
    case OBJ_TYPE_SCENERY:
        return proto->scenery.type == subtype;
    }

    return false;
}

// proto_data_member
// 0x49FFD8
int protoGetDataMember(int pid, int member, ProtoDataMemberValue* value)
{
    Proto* proto;
    if (protoGetProto(pid, &proto) == -1) {
        return -1;
    }

    switch (PID_TYPE(pid)) {
    case OBJ_TYPE_ITEM:
        switch (member) {
        case ITEM_DATA_MEMBER_PID:
            value->integerValue = proto->pid;
            break;
        case ITEM_DATA_MEMBER_NAME:
            // NOTE: uninline
            value->stringValue = protoGetName(proto->scenery.pid);
            return PROTO_DATA_MEMBER_TYPE_STRING;
        case ITEM_DATA_MEMBER_DESCRIPTION:
            // NOTE: Uninline.
            value->stringValue = protoGetDescription(proto->pid);
            return PROTO_DATA_MEMBER_TYPE_STRING;
        case ITEM_DATA_MEMBER_FID:
            value->integerValue = proto->fid;
            break;
        case ITEM_DATA_MEMBER_LIGHT_DISTANCE:
            value->integerValue = proto->item.lightDistance;
            break;
        case ITEM_DATA_MEMBER_LIGHT_INTENSITY:
            value->integerValue = proto->item.lightIntensity;
            break;
        case ITEM_DATA_MEMBER_FLAGS:
            value->integerValue = proto->item.flags;
            break;
        case ITEM_DATA_MEMBER_EXTENDED_FLAGS:
            value->integerValue = proto->item.extendedFlags;
            break;
        case ITEM_DATA_MEMBER_SID:
            value->integerValue = proto->item.sid;
            break;
        case ITEM_DATA_MEMBER_TYPE:
            value->integerValue = proto->item.type;
            break;
        case ITEM_DATA_MEMBER_MATERIAL:
            value->integerValue = proto->item.material;
            break;
        case ITEM_DATA_MEMBER_SIZE:
            value->integerValue = proto->item.size;
            break;
        case ITEM_DATA_MEMBER_WEIGHT:
            value->integerValue = proto->item.weight;
            break;
        case ITEM_DATA_MEMBER_COST:
            value->integerValue = proto->item.cost;
            break;
        case ITEM_DATA_MEMBER_INVENTORY_FID:
            value->integerValue = proto->item.inventoryFid;
            break;
        case ITEM_DATA_MEMBER_WEAPON_RANGE:
            if (proto->item.type == ITEM_TYPE_WEAPON) {
                value->integerValue = proto->item.data.weapon.maxRange1;
            }
            break;
        default:
            debugPrint("\n\tError: Unimp'd data member in member in proto_data_member!");
            break;
        }
        break;
    case OBJ_TYPE_CRITTER:
        switch (member) {
        case CRITTER_DATA_MEMBER_PID:
            value->integerValue = proto->critter.pid;
            break;
        case CRITTER_DATA_MEMBER_NAME:
            // NOTE: Uninline.
            value->stringValue = protoGetName(proto->critter.pid);
            return PROTO_DATA_MEMBER_TYPE_STRING;
        case CRITTER_DATA_MEMBER_DESCRIPTION:
            // NOTE: Uninline.
            value->stringValue = protoGetDescription(proto->critter.pid);
            return PROTO_DATA_MEMBER_TYPE_STRING;
        case CRITTER_DATA_MEMBER_FID:
            value->integerValue = proto->critter.fid;
            break;
        case CRITTER_DATA_MEMBER_LIGHT_DISTANCE:
            value->integerValue = proto->critter.lightDistance;
            break;
        case CRITTER_DATA_MEMBER_LIGHT_INTENSITY:
            value->integerValue = proto->critter.lightIntensity;
            break;
        case CRITTER_DATA_MEMBER_FLAGS:
            value->integerValue = proto->critter.flags;
            break;
        case CRITTER_DATA_MEMBER_EXTENDED_FLAGS:
            value->integerValue = proto->critter.extendedFlags;
            break;
        case CRITTER_DATA_MEMBER_SID:
            value->integerValue = proto->critter.sid;
            break;
        case CRITTER_DATA_MEMBER_HEAD_FID:
            value->integerValue = proto->critter.headFid;
            break;
        case CRITTER_DATA_MEMBER_BODY_TYPE:
            value->integerValue = proto->critter.data.bodyType;
            break;
        default:
            debugPrint("\n\tError: Unimp'd data member in member in proto_data_member!");
            break;
        }
        break;
    case OBJ_TYPE_SCENERY:
        switch (member) {
        case SCENERY_DATA_MEMBER_PID:
            value->integerValue = proto->scenery.pid;
            break;
        case SCENERY_DATA_MEMBER_NAME:
            // NOTE: Uninline.
            value->stringValue = protoGetName(proto->scenery.pid);
            return PROTO_DATA_MEMBER_TYPE_STRING;
        case SCENERY_DATA_MEMBER_DESCRIPTION:
            // NOTE: Uninline.
            value->stringValue = protoGetDescription(proto->scenery.pid);
            return PROTO_DATA_MEMBER_TYPE_STRING;
        case SCENERY_DATA_MEMBER_FID:
            value->integerValue = proto->scenery.fid;
            break;
        case SCENERY_DATA_MEMBER_LIGHT_DISTANCE:
            value->integerValue = proto->scenery.lightDistance;
            break;
        case SCENERY_DATA_MEMBER_LIGHT_INTENSITY:
            value->integerValue = proto->scenery.lightIntensity;
            break;
        case SCENERY_DATA_MEMBER_FLAGS:
            value->integerValue = proto->scenery.flags;
            break;
        case SCENERY_DATA_MEMBER_EXTENDED_FLAGS:
            value->integerValue = proto->scenery.extendedFlags;
            break;
        case SCENERY_DATA_MEMBER_SID:
            value->integerValue = proto->scenery.sid;
            break;
        case SCENERY_DATA_MEMBER_TYPE:
            value->integerValue = proto->scenery.type;
            break;
        case SCENERY_DATA_MEMBER_MATERIAL:
            value->integerValue = proto->scenery.field_2C;
            break;
        default:
            debugPrint("\n\tError: Unimp'd data member in member in proto_data_member!");
            break;
        }
        break;
    case OBJ_TYPE_WALL:
        switch (member) {
        case WALL_DATA_MEMBER_PID:
            value->integerValue = proto->wall.pid;
            break;
        case WALL_DATA_MEMBER_NAME:
            // NOTE: Uninline.
            value->stringValue = protoGetName(proto->wall.pid);
            return PROTO_DATA_MEMBER_TYPE_STRING;
        case WALL_DATA_MEMBER_DESCRIPTION:
            // NOTE: Uninline.
            value->stringValue = protoGetDescription(proto->wall.pid);
            return PROTO_DATA_MEMBER_TYPE_STRING;
        case WALL_DATA_MEMBER_FID:
            value->integerValue = proto->wall.fid;
            break;
        case WALL_DATA_MEMBER_LIGHT_DISTANCE:
            value->integerValue = proto->wall.lightDistance;
            break;
        case WALL_DATA_MEMBER_LIGHT_INTENSITY:
            value->integerValue = proto->wall.lightIntensity;
            break;
        case WALL_DATA_MEMBER_FLAGS:
            value->integerValue = proto->wall.flags;
            break;
        case WALL_DATA_MEMBER_EXTENDED_FLAGS:
            value->integerValue = proto->wall.extendedFlags;
            break;
        case WALL_DATA_MEMBER_SID:
            value->integerValue = proto->wall.sid;
            break;
        case WALL_DATA_MEMBER_MATERIAL:
            value->integerValue = proto->wall.material;
            break;
        default:
            debugPrint("\n\tError: Unimp'd data member in member in proto_data_member!");
            break;
        }
        break;
    case OBJ_TYPE_TILE:
        debugPrint("\n\tError: Unimp'd data member in member in proto_data_member!");
        break;
    case OBJ_TYPE_MISC:
        switch (member) {
        case MISC_DATA_MEMBER_PID:
            value->integerValue = proto->misc.pid;
            break;
        case MISC_DATA_MEMBER_NAME:
            // NOTE: Uninline.
            value->stringValue = protoGetName(proto->misc.pid);
            return PROTO_DATA_MEMBER_TYPE_STRING;
        case MISC_DATA_MEMBER_DESCRIPTION:
            // NOTE: Uninline.
            value->stringValue = protoGetDescription(proto->misc.pid);
            // FIXME: Errornously report type as int, should be string.
            return PROTO_DATA_MEMBER_TYPE_INT;
        case MISC_DATA_MEMBER_FID:
            value->integerValue = proto->misc.fid;
            return 1;
        case MISC_DATA_MEMBER_LIGHT_DISTANCE:
            value->integerValue = proto->misc.lightDistance;
            return 1;
        case MISC_DATA_MEMBER_LIGHT_INTENSITY:
            value->integerValue = proto->misc.lightIntensity;
            break;
        case MISC_DATA_MEMBER_FLAGS:
            value->integerValue = proto->misc.flags;
            break;
        case MISC_DATA_MEMBER_EXTENDED_FLAGS:
            value->integerValue = proto->misc.extendedFlags;
            break;
        default:
            debugPrint("\n\tError: Unimp'd data member in member in proto_data_member!");
            break;
        }
        break;
    }

    return PROTO_DATA_MEMBER_TYPE_INT;
}

// ============================================================================
// Object Data Functions
// ============================================================================

// 0x49EEA4
void objectDataReset(Object* obj)
{
    // NOTE: Original code is slightly different. It uses loop to zero object
    // data byte by byte.
    memset(&(obj->data), 0, sizeof(obj->data));
}

// 0x49F004
int objectDataRead(Object* obj, File* stream)
{
    Proto* proto;
    int temp;

    Inventory* inventory = &(obj->data.inventory);
    if (fileReadInt32(stream, &(inventory->length)) == -1)
        return -1;
    if (fileReadInt32(stream, &(inventory->capacity)) == -1)
        return -1;
    // CE: Original code reads inventory items pointer which is meaningless.
    if (fileReadInt32(stream, &temp) == -1)
        return -1;

    if (PID_TYPE(obj->pid) == OBJ_TYPE_CRITTER) {
        if (fileReadInt32(stream, &(obj->data.critter.field_0)) == -1)
            return -1;
        if (objectCritterCombatDataRead(&(obj->data.critter.combat), stream) == -1)
            return -1;
        if (fileReadInt32(stream, &(obj->data.critter.hp)) == -1)
            return -1;
        if (fileReadInt32(stream, &(obj->data.critter.radiation)) == -1)
            return -1;
        if (fileReadInt32(stream, &(obj->data.critter.poison)) == -1)
            return -1;
    } else {
        if (fileReadInt32(stream, &(obj->data.flags)) == -1)
            return -1;

        if (obj->data.flags == 0xCCCCCCCC) {
            debugPrint("\nNote: Reading pud: updated_flags was un-Set!");
            obj->data.flags = 0;
        }

        switch (PID_TYPE(obj->pid)) {
        case OBJ_TYPE_ITEM:
            if (protoGetProto(obj->pid, &proto) == -1)
                return -1;

            switch (proto->item.type) {
            case ITEM_TYPE_WEAPON:
                if (fileReadInt32(stream, &(obj->data.item.weapon.ammoQuantity)) == -1)
                    return -1;
                if (fileReadInt32(stream, &(obj->data.item.weapon.ammoTypePid)) == -1)
                    return -1;
                break;
            case ITEM_TYPE_AMMO:
                if (fileReadInt32(stream, &(obj->data.item.ammo.quantity)) == -1)
                    return -1;
                break;
            case ITEM_TYPE_MISC:
                if (fileReadInt32(stream, &(obj->data.item.misc.charges)) == -1)
                    return -1;
                break;
            case ITEM_TYPE_KEY:
                if (fileReadInt32(stream, &(obj->data.item.key.keyCode)) == -1)
                    return -1;
                break;
            default:
                break;
            }

            break;
        case OBJ_TYPE_SCENERY:
            if (protoGetProto(obj->pid, &proto) == -1)
                return -1;

            switch (proto->scenery.type) {
            case SCENERY_TYPE_DOOR:
                if (fileReadInt32(stream, &(obj->data.scenery.door.openFlags)) == -1)
                    return -1;
                break;
            case SCENERY_TYPE_STAIRS:
                if (fileReadInt32(stream, &(obj->data.scenery.stairs.destinationBuiltTile)) == -1)
                    return -1;
                if (fileReadInt32(stream, &(obj->data.scenery.stairs.destinationMap)) == -1)
                    return -1;
                break;
            case SCENERY_TYPE_ELEVATOR:
                if (fileReadInt32(stream, &(obj->data.scenery.elevator.type)) == -1)
                    return -1;
                if (fileReadInt32(stream, &(obj->data.scenery.elevator.level)) == -1)
                    return -1;
                break;
            case SCENERY_TYPE_LADDER_UP:
                if (gMapHeader.version == 19) {
                    if (fileReadInt32(stream, &(obj->data.scenery.ladder.destinationBuiltTile)) == -1)
                        return -1;
                } else {
                    if (fileReadInt32(stream, &(obj->data.scenery.ladder.destinationMap)) == -1)
                        return -1;
                    if (fileReadInt32(stream, &(obj->data.scenery.ladder.destinationBuiltTile)) == -1)
                        return -1;
                }
                break;
            case SCENERY_TYPE_LADDER_DOWN:
                if (gMapHeader.version == 19) {
                    if (fileReadInt32(stream, &(obj->data.scenery.ladder.destinationBuiltTile)) == -1)
                        return -1;
                } else {
                    if (fileReadInt32(stream, &(obj->data.scenery.ladder.destinationMap)) == -1)
                        return -1;
                    if (fileReadInt32(stream, &(obj->data.scenery.ladder.destinationBuiltTile)) == -1)
                        return -1;
                }
                break;
            }

            break;
        case OBJ_TYPE_MISC:
            if (isExitGridPid(obj->pid)) {
                if (fileReadInt32(stream, &(obj->data.misc.map)) == -1)
                    return -1;
                if (fileReadInt32(stream, &(obj->data.misc.tile)) == -1)
                    return -1;
                if (fileReadInt32(stream, &(obj->data.misc.elevation)) == -1)
                    return -1;
                if (fileReadInt32(stream, &(obj->data.misc.rotation)) == -1)
                    return -1;
            }
            break;
        }
    }

    return 0;
}

// 0x49F428
int objectDataWrite(Object* obj, File* stream)
{
    Proto* proto;

    ObjectData* data = &(obj->data);
    if (fileWriteInt32(stream, data->inventory.length) == -1)
        return -1;
    if (fileWriteInt32(stream, data->inventory.capacity) == -1)
        return -1;
    // CE: Original code writes inventory items pointer, which is meaningless.
    if (fileWriteInt32(stream, 0) == -1)
        return -1;

    if (PID_TYPE(obj->pid) == OBJ_TYPE_CRITTER) {
        if (fileWriteInt32(stream, data->flags) == -1)
            return -1;
        if (objectCritterCombatDataWrite(&(obj->data.critter.combat), stream) == -1)
            return -1;
        if (fileWriteInt32(stream, data->critter.hp) == -1)
            return -1;
        if (fileWriteInt32(stream, data->critter.radiation) == -1)
            return -1;
        if (fileWriteInt32(stream, data->critter.poison) == -1)
            return -1;
    } else {
        if (fileWriteInt32(stream, data->flags) == -1)
            return -1;

        switch (PID_TYPE(obj->pid)) {
        case OBJ_TYPE_ITEM:
            if (protoGetProto(obj->pid, &proto) == -1)
                return -1;

            switch (proto->item.type) {
            case ITEM_TYPE_WEAPON:
                if (fileWriteInt32(stream, data->item.weapon.ammoQuantity) == -1)
                    return -1;
                if (fileWriteInt32(stream, data->item.weapon.ammoTypePid) == -1)
                    return -1;
                break;
            case ITEM_TYPE_AMMO:
                if (fileWriteInt32(stream, data->item.ammo.quantity) == -1)
                    return -1;
                break;
            case ITEM_TYPE_MISC:
                if (fileWriteInt32(stream, data->item.misc.charges) == -1)
                    return -1;
                break;
            case ITEM_TYPE_KEY:
                if (fileWriteInt32(stream, data->item.key.keyCode) == -1)
                    return -1;
                break;
            }
            break;
        case OBJ_TYPE_SCENERY:
            if (protoGetProto(obj->pid, &proto) == -1)
                return -1;

            switch (proto->scenery.type) {
            case SCENERY_TYPE_DOOR:
                if (fileWriteInt32(stream, data->scenery.door.openFlags) == -1)
                    return -1;
                break;
            case SCENERY_TYPE_STAIRS:
                if (fileWriteInt32(stream, data->scenery.stairs.destinationBuiltTile) == -1)
                    return -1;
                if (fileWriteInt32(stream, data->scenery.stairs.destinationMap) == -1)
                    return -1;
                break;
            case SCENERY_TYPE_ELEVATOR:
                if (fileWriteInt32(stream, data->scenery.elevator.type) == -1)
                    return -1;
                if (fileWriteInt32(stream, data->scenery.elevator.level) == -1)
                    return -1;
                break;
            case SCENERY_TYPE_LADDER_UP:
                if (fileWriteInt32(stream, data->scenery.ladder.destinationMap) == -1)
                    return -1;
                if (fileWriteInt32(stream, data->scenery.ladder.destinationBuiltTile) == -1)
                    return -1;
                break;
            case SCENERY_TYPE_LADDER_DOWN:
                if (fileWriteInt32(stream, data->scenery.ladder.destinationMap) == -1)
                    return -1;
                if (fileWriteInt32(stream, data->scenery.ladder.destinationBuiltTile) == -1)
                    return -1;
                break;
            default:
                break;
            }
            break;
        case OBJ_TYPE_MISC:
            if (isExitGridPid(obj->pid)) {
                if (fileWriteInt32(stream, data->misc.map) == -1)
                    return -1;
                if (fileWriteInt32(stream, data->misc.tile) == -1)
                    return -1;
                if (fileWriteInt32(stream, data->misc.elevation) == -1)
                    return -1;
                if (fileWriteInt32(stream, data->misc.rotation) == -1)
                    return -1;
            }
            break;
        default:
            break;
        }
    }

    return 0;
}

// 0x49F8A0
int _proto_update_init(Object* obj)
{
    if (!_protos_been_initialized) {
        return -1;
    }

    if (obj == nullptr) {
        return -1;
    }

    if (obj->pid == -1) {
        return -1;
    }

    memset(&(obj->data), 0, sizeof(ObjectData));

    if (PID_TYPE(obj->pid) != OBJ_TYPE_CRITTER) {
        return _proto_update_gen(obj);
    }

    ObjectData* data = &(obj->data);
    data->inventory.length = 0;
    data->inventory.capacity = 0;
    data->inventory.items = nullptr;
    _combat_data_init(obj);
    data->critter.hp = critterGetStat(obj, STAT_MAXIMUM_HIT_POINTS);
    data->critter.combat.ap = critterGetStat(obj, STAT_MAXIMUM_ACTION_POINTS);
    critterUpdateDerivedStats(obj);
    obj->data.critter.combat.whoHitMe = nullptr;

    Proto* proto;
    if (protoGetProto(obj->pid, &proto) != -1) {
        data->critter.combat.aiPacket = proto->critter.aiPacket;
        data->critter.combat.team = proto->critter.team;
    }

    return 0;
}

// ============================================================================
// Player/Dude Functions
// ============================================================================

// 0x49F984
int _proto_dude_update_gender()
{
    Proto* proto;
    if (protoGetProto(0x1000000, &proto) == -1) {
        return -1;
    }

    int nativeLook = DUDE_NATIVE_LOOK_TRIBAL;
    if (gameMovieIsSeen(MOVIE_VSUIT)) {
        nativeLook = DUDE_NATIVE_LOOK_JUMPSUIT;
    }

    int frmId;
    if (critterGetStat(gDude, STAT_GENDER) == GENDER_MALE) {
        frmId = _art_vault_person_nums[nativeLook][GENDER_MALE];
    } else {
        frmId = _art_vault_person_nums[nativeLook][GENDER_FEMALE];
    }

    _art_vault_guy_num = frmId;

    if (critterGetArmor(gDude) == nullptr) {
        int v1 = 0;
        if (critterGetItem2(gDude) != nullptr || critterGetItem1(gDude) != nullptr) {
            v1 = (gDude->fid & 0xF000) >> 12;
        }

        int fid = buildFid(OBJ_TYPE_CRITTER, _art_vault_guy_num, 0, v1, 0);
        objectSetFid(gDude, fid, nullptr);
    }

    proto->fid = buildFid(OBJ_TYPE_CRITTER, _art_vault_guy_num, 0, 0, 0);

    return 0;
}

// proto_dude_init
// 0x49FA64
int _proto_dude_init(const char* path)
{
    gDudeProto.fid = buildFid(OBJ_TYPE_CRITTER, _art_vault_guy_num, 0, 0, 0);

    if (_init_true) {
        _obj_inven_free(&(gDude->data.inventory));
    }

    _init_true = 1;

    Proto* proto;
    if (protoGetProto(0x1000000, &proto) == -1) {
        return -1;
    }

    protoGetProto(gDude->pid, &proto);

    _proto_update_init(gDude);
    gDude->data.critter.combat.aiPacket = 0;
    gDude->data.critter.combat.team = 0;
    _ResetPlayer();

    if (gcdLoad(path) == -1) {
        _retval = -1;
    }

    proto->critter.data.baseStats[STAT_DAMAGE_RESISTANCE_EMP] = 100;
    proto->critter.data.bodyType = 0;
    proto->critter.data.experience = 0;
    proto->critter.data.killType = 0;
    proto->critter.data.damageType = 0;

    _proto_dude_update_gender();
    inventoryResetDude();

    if ((gDude->flags & OBJECT_FLAT) != 0) {
        _obj_toggle_flat(gDude, nullptr);
    }

    if ((gDude->flags & OBJECT_NO_BLOCK) != 0) {
        gDude->flags &= ~OBJECT_NO_BLOCK;
    }

    critterUpdateDerivedStats(gDude);
    critterAdjustHitPoints(gDude, 10000);

    if (_retval) {
        debugPrint("\n ** Error in proto_dude_init()! **\n");
    }

    return 0;
}

// 0x4A22C0
int _ResetPlayer()
{
    Proto* proto;
    protoGetProto(gDude->pid, &proto);

    pcStatsReset();
    protoCritterDataResetStats(&(proto->critter.data));

    // SFALL: Fix base EMP DR not being properly initialized.
    proto->critter.data.baseStats[STAT_DAMAGE_RESISTANCE_EMP] = 100;

    critterReset();
    characterEditorReset();
    protoCritterDataResetSkills(&(proto->critter.data));
    skillsReset();
    perksReset();
    traitsReset();
    critterUpdateDerivedStats(gDude);
    return 0;
}

// ============================================================================
// Core Public API Functions
// ============================================================================

// 0x49E270
void proto_make_path(char* path, int pid)
{
    strcpy(path, _cd_path_base);
    strcat(path, _proto_path_base);
    if (pid != -1) {
        strcat(path, artGetObjectTypeName(PID_TYPE(pid)));
    }
}

// Append proto file name to proto_path from proto.lst.
//
// 0x49E758
int _proto_list_str(int pid, char* proto_path)
{
    if (pid == -1) {
        return -1;
    }

    if (proto_path == nullptr) {
        return -1;
    }

    char path[COMPAT_MAX_PATH];
    proto_make_path(path, pid);
    strcat(path, "\\");
    strcat(path, artGetObjectTypeName(PID_TYPE(pid)));
    strcat(path, ".lst");

    File* stream = fileOpen(path, "rt");

    int i = 1;
    char string[256];
    while (fileReadString(string, sizeof(string), stream)) {
        if (i == (pid & 0xFFFFFF)) {
            break;
        }

        i++;
    }

    fileClose(stream);

    if (i != (pid & 0xFFFFFF)) {
        return -1;
    }

    char* pch = strchr(string, ' ');
    if (pch != nullptr) {
        *pch = '\0';
    }

    pch = strpbrk(string, "\r\n");
    if (pch != nullptr) {
        *pch = '\0';
    }

    strcpy(proto_path, string);

    return 0;
}

// 0x49E984
size_t proto_size(int type)
{
    return type >= 0 && type < OBJ_TYPE_COUNT ? _proto_sizes[type] : 0;
}

// 0x49E99C
bool _proto_action_can_use(int pid)
{
    Proto* proto;
    if (protoGetProto(pid, &proto) == -1) {
        return false;
    }

    if ((proto->item.extendedFlags & 0x0800) != 0) {
        return true;
    }

    if (PID_TYPE(pid) == OBJ_TYPE_ITEM && proto->item.type == ITEM_TYPE_CONTAINER) {
        return true;
    }

    return false;
}

// 0x49E9DC
bool _proto_action_can_use_on(int pid)
{
    Proto* proto;
    if (protoGetProto(pid, &proto) == -1) {
        return false;
    }

    if ((proto->item.extendedFlags & 0x1000) != 0) {
        return true;
    }

    if (PID_TYPE(pid) == OBJ_TYPE_ITEM && proto->item.type == ITEM_TYPE_DRUG) {
        return true;
    }

    return false;
}

// 0x49EA24
bool _proto_action_can_talk_to(int pid)
{
    Proto* proto;
    if (protoGetProto(pid, &proto) == -1) {
        return false;
    }

    if (PID_TYPE(pid) == OBJ_TYPE_CRITTER) {
        return true;
    }

    if (proto->critter.extendedFlags & 0x4000) {
        return true;
    }

    return false;
}

// Likely returns true if item with given pid can be picked up.
//
// 0x49EA5C
int _proto_action_can_pickup(int pid)
{
    if (PID_TYPE(pid) != OBJ_TYPE_ITEM) {
        return false;
    }

    Proto* proto;
    if (protoGetProto(pid, &proto) == -1) {
        return false;
    }

    if (proto->item.type == ITEM_TYPE_CONTAINER) {
        return (proto->item.extendedFlags & 0x8000) != 0;
    }

    return true;
}

/**
 * @brief Retrieves a message for a proto with mod support.
 *
 * Extended version that first checks mod proto messages before falling back
 * to vanilla message lookup. This allows mods to override or add messages
 * for their protos.
 *
 * @param pid The PID to get a message for.
 * @param message The message type (PROTOTYPE_MESSAGE_NAME or DESCRIPTION).
 * @return Pointer to the message string, or _proto_none_str if not found.
 */
char* protoGetMessage(int pid, int message)
{

    // First, try to get from mod proto message system
    if (pid_is_modded(pid)) {
        char* mod_msg = messageListGetModProtoMessage(pid, message);

        if (mod_msg) {
            return mod_msg;
        }
    }
    char* v1 = _proto_none_str;

    Proto* proto;
    if (protoGetProto(pid, &proto) != -1) {
        if (proto->messageId != -1) {
            MessageList* messageList = &(_proto_msg_files[PID_TYPE(pid)]);

            MessageListItem messageListItem;
            messageListItem.num = proto->messageId + message;
            if (messageListGetItem(messageList, &messageListItem)) {
                v1 = messageListItem.text;
            }
        }
    }

    return v1;
}

// 0x49EAFC
char* protoGetName(int pid)
{
    if (pid == 0x1000000) {
        return critterGetName(gDude);
    }

    return protoGetMessage(pid, PROTOTYPE_MESSAGE_NAME);
}

// 0x49EB1C
char* protoGetDescription(int pid)
{
    return protoGetMessage(pid, PROTOTYPE_MESSAGE_DESCRIPTION);
}

/**
 * @brief Retrieves a proto by PID with mod support.
 *
 * Extended version of the original protoGetProto that supports mod PIDs.
 * If the PID is a mod PID, it loads the proto from the mod file system
 * instead of the vanilla directories. Mod protos are cached like vanilla ones.
 *
 * @param pid The PID to look up.
 * @param protoPtr Output parameter for the proto pointer.
 * @return 0 on success, -1 on failure.
 */
int protoGetProto(int pid, Proto** protoPtr)
{
    *protoPtr = nullptr;

    if (pid == -1) {
        return -1;
    }

    if (pid == 0x1000000) {
        *protoPtr = (Proto*)&gDudeProto;
        return 0;
    }

    // Check if it's a mod PID
    if (pid_is_modded(pid)) {
        // Look up in mod registry
        ModProtoEntry* entry = mod_proto_registry_find_by_pid(pid);
        if (!entry) {
            return -1; // Mod proto not found
        }

        // Try to find in existing cache first
        ProtoList* protoList = &(_protoLists[PID_TYPE(pid)]);
        ProtoListExtent* protoListExtent = protoList->head;
        while (protoListExtent != nullptr) {
            for (int index = 0; index < protoListExtent->length; index++) {
                Proto* proto = (Proto*)protoListExtent->proto[index];
                if (pid == proto->pid) {
                    *protoPtr = proto;
                    return 0;
                }
            }
            protoListExtent = protoListExtent->next;
        }

        // Not in cache, load from mod file
        File* stream = fileOpen(entry->proto_path, "rb");
        if (stream == nullptr) {
            debugPrint("Error: Can't open mod proto file: %s\n", entry->proto_path);
            return -1;
        }

        // Find or allocate cache slot
        if (_proto_find_free_subnode(PID_TYPE(pid), protoPtr) == -1) {
            fileClose(stream);
            return -1;
        }

        // Read proto data
        if (protoRead(*protoPtr, stream) != 0) {
            fileClose(stream);
            return -1;
        }

        fileClose(stream);
        return 0;
    }

    ProtoList* protoList = &(_protoLists[PID_TYPE(pid)]);
    ProtoListExtent* protoListExtent = protoList->head;
    while (protoListExtent != nullptr) {
        for (int index = 0; index < protoListExtent->length; index++) {
            Proto* proto = (Proto*)protoListExtent->proto[index];
            if (pid == proto->pid) {
                *protoPtr = proto;
                return 0;
            }
        }
        protoListExtent = protoListExtent->next;
    }

    if (protoList->head != nullptr && protoList->tail != nullptr) {
        if (PROTO_LIST_EXTENT_SIZE * protoList->length - (PROTO_LIST_EXTENT_SIZE - protoList->tail->length) > PROTO_LIST_MAX_ENTRIES) {
            _proto_remove_some_list(PID_TYPE(pid));
        }
    }

    return _proto_load_pid(pid, protoPtr);
}

// 0x4A1E90
int proto_new(int* pid, int type)
{
    Proto* proto;

    if (_proto_find_free_subnode(type, &proto) == -1) {
        return -1;
    }

    *pid = _proto_new_id(type) | (type << 24);
    switch (type) {
    case OBJ_TYPE_ITEM:
        proto_item_init(proto, *pid);
        proto->item.pid = *pid;
        break;
    case OBJ_TYPE_CRITTER:
        proto_critter_init(proto, *pid);
        proto->critter.pid = *pid;
        break;
    case OBJ_TYPE_SCENERY:
        proto_scenery_init(proto, *pid);
        proto->scenery.pid = *pid;
        break;
    case OBJ_TYPE_WALL:
        proto_wall_init(proto, *pid);
        proto->wall.pid = *pid;
        break;
    case OBJ_TYPE_TILE:
        proto_tile_init(proto, *pid);
        proto->tile.pid = *pid;
        break;
    case OBJ_TYPE_MISC:
        proto_misc_init(proto, *pid);
        proto->misc.pid = *pid;
        break;
    default:
        return -1;
    }

    return 0;
}

// Clear all proto cache.
//
// 0x4A20F4
void _proto_remove_all()
{
    for (int index = 0; index < 6; index++) {
        _proto_remove_list(index);
    }
}

// 0x4A2214
int proto_max_id(int type)
{
    return _protoLists[type].max_entries_num;
}

// 0x4A1B30
int _proto_save_pid(int pid)
{
    Proto* proto;
    if (protoGetProto(pid, &proto) == -1) {
        return -1;
    }

    char path[260];
    proto_make_path(path, pid);
    strcat(path, "\\");

    _proto_list_str(pid, path + strlen(path));

    File* stream = fileOpen(path, "wb");
    if (stream == nullptr) {
        return -1;
    }

    int rc = protoWrite(proto, stream);

    fileClose(stream);

    return rc;
}

// ============================================================================
// Mod System Helper Functions
// ============================================================================

static bool pid_is_modded(int pid)
{
    return (pid & 0xFFFFFF) >= MOD_PROTO_START;
}

/**
 * @brief String hash function for stable PID generation.
 *
 * Uses the DJB2 hash algorithm (multiply by 33) to generate stable
 * hashes from mod/proto names. Case-insensitive.
 *
 * @param str The string to hash.
 * @return 32-bit hash value.
 */
static uint32_t proto_hash_string(const char* str)
{
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        c = tolower(c);
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

/**
 * @brief Generates a stable PID for a mod proto.
 *
 * Creates a PID in the mod range (0x800000-0xFFFFFF) based on a hash of
 * the mod and proto names. Ensures stable IDs across game runs.
 *
 * @param mod_name Name of the mod (e.g., "testmod").
 * @param proto_name Name of the proto (e.g., "testitem").
 * @param proto_type Object type (OBJ_TYPE_ITEM, etc.).
 * @return Generated PID with type in high byte and hash-based index.
 */
static int generate_mod_proto_pid(const char* mod_name, const char* proto_name, int proto_type)
{
    char composite_key[256];
    snprintf(composite_key, sizeof(composite_key), "%s:%s", mod_name, proto_name);

    uint32_t hash = proto_hash_string(composite_key);

    // Generate PID in mod range: type<<24 | (MOD_PROTO_START + hash_within_range)
    uint32_t index = MOD_PROTO_START + (hash % (MOD_PROTO_MAX - MOD_PROTO_START + 1));
    return (proto_type << 24) | index;
}

/**
 * @brief Adds a mod proto entry to the global registry.
 *
 * This function stores information about a loaded mod proto for later lookup.
 * Entries are stored in a dynamically growing array.
 *
 * @param entry Pointer to the ModProtoEntry to add (will be copied).
 */
static void mod_proto_registry_add(const ModProtoEntry* entry)
{
    // Grow array if needed
    if (_mod_proto_entries_size >= _mod_proto_entries_capacity) {
        int new_capacity = _mod_proto_entries_capacity == 0 ? 16 : _mod_proto_entries_capacity * 2;
        ModProtoEntry* new_entries = (ModProtoEntry*)internal_realloc(_mod_proto_entries, new_capacity * sizeof(ModProtoEntry));
        if (!new_entries) {
            return;
        }
        _mod_proto_entries = new_entries;
        _mod_proto_entries_capacity = new_capacity;
    }

    // Copy the entry
    _mod_proto_entries[_mod_proto_entries_size] = *entry;
    _mod_proto_entries_size++;
}

/**
 * @brief Finds a mod proto entry by its PID.
 *
 * Searches the mod proto registry for an entry with the given PID.
 * Used when a mod proto is accessed by its PID.
 *
 * @param pid The PID to search for (must be a mod PID).
 * @return Pointer to the ModProtoEntry if found, nullptr otherwise.
 */
static ModProtoEntry* mod_proto_registry_find_by_pid(int pid)
{
    for (int i = 0; i < _mod_proto_entries_size; i++) {
        if (_mod_proto_entries[i].pid == pid) {
            return &_mod_proto_entries[i];
        }
    }
    return nullptr;
}

/**
 * @brief Finds a mod proto entry by composite key (mod:proto).
 *
 * Searches the mod proto registry for an entry with the given mod:proto key.
 * Used when looking up a proto by its human-readable name.
 *
 * @param key Composite key in format "modname:protoname".
 * @return Pointer to the ModProtoEntry if found, nullptr otherwise.
 */
static ModProtoEntry* mod_proto_registry_find_by_key(const char* key)
{
    for (int i = 0; i < _mod_proto_entries_size; i++) {
        char composite_key[256];
        snprintf(composite_key, sizeof(composite_key), "%s:%s",
            _mod_proto_entries[i].mod_name,
            _mod_proto_entries[i].proto_name);
        if (compat_stricmp(composite_key, key) == 0) {
            return &_mod_proto_entries[i];
        }
    }
    return nullptr;
}

/**
 * @brief Adds a name-to-PID mapping to the registry.
 *
 * Creates a mapping from composite key (mod:proto) to PID for reverse lookup.
 * Used when parsing message files to find PIDs for mod protos.
 *
 * @param key Composite key in format "modname:protoname".
 * @param pid The PID associated with this key.
 */
static void name_to_pid_registry_add(const char* key, int pid)
{
    // Grow array if needed
    if (_name_to_pid_entries_size >= _name_to_pid_entries_capacity) {
        int new_capacity = _name_to_pid_entries_capacity == 0 ? 16 : _name_to_pid_entries_capacity * 2;
        NameToPidEntry* new_entries = (NameToPidEntry*)internal_realloc(_name_to_pid_entries,
            new_capacity * sizeof(NameToPidEntry));
        if (!new_entries) {
            debugPrint("ERROR: Failed to grow name-to-PID registry array!\n");
            return;
        }
        _name_to_pid_entries = new_entries;
        _name_to_pid_entries_capacity = new_capacity;
    }

    // Add the entry
    _name_to_pid_entries[_name_to_pid_entries_size].key = internal_strdup(key);
    _name_to_pid_entries[_name_to_pid_entries_size].pid = pid;
    _name_to_pid_entries_size++;
}

/**
 * @brief Finds a PID by composite key.
 *
 * Looks up a PID using the composite key (mod:proto).
 * Primarily used when parsing message files to resolve mod proto names to PIDs.
 *
 * @param key Composite key in format "modname:protoname".
 * @return The PID if found, -1 otherwise.
 */
static int name_to_pid_registry_find(const char* key)
{
    for (int i = 0; i < _name_to_pid_entries_size; i++) {
        if (_name_to_pid_entries[i].key && compat_stricmp(_name_to_pid_entries[i].key, key) == 0) {
            return _name_to_pid_entries[i].pid;
        }
    }
    return -1;
}

// ============================================================================
// Mod Loading Functions
// ============================================================================

/**
 * @brief Loads a single mod proto list file.
 *
 * Parses a mod-specific .lst file (e.g., items_testmod.lst) to discover
 * mod protos. Each line contains a proto filename (without .pro extension).
 *
 * @param list_path Full path to the .lst file.
 * @param mod_name Name of the mod (extracted from filename).
 * @param proto_type Object type (OBJ_TYPE_ITEM, etc.).
 * @param proto_type_name String name of the type (for debugging).
 */
static void load_single_mod_proto_list(const char* list_path, const char* mod_name,
    int proto_type, const char* proto_type_name)
{
    File* stream = fileOpen(list_path, "rt");
    if (!stream) {
        debugPrint("Failed to open mod proto list: %s\n", list_path);
        return;
    }

    char line[256];
    int line_num = 0;

    while (fileReadString(line, sizeof(line), stream)) {
        line_num++;

        // Remove line endings
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        char* cr = strchr(line, '\r');
        if (cr) *cr = '\0';

        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // Parse proto name - JUST the filename, no display name
        char proto_name[128] = { 0 };
        strncpy(proto_name, line, sizeof(proto_name) - 1);

        // Remove .pro extension if present
        char* dot = strrchr(proto_name, '.');
        if (dot && (strcmp(dot, ".pro") == 0 || strcmp(dot, ".PRO") == 0)) {
            *dot = '\0';
        }

        // Also remove any trailing spaces
        char* end = proto_name + strlen(proto_name) - 1;
        while (end > proto_name && isspace(*end)) {
            *end = '\0';
            end--;
        }

        // Generate PID for this mod proto
        int pid = generate_mod_proto_pid(mod_name, proto_name, proto_type);

        // Check for hash collisions with existing mod protos
        ModProtoEntry* existing_entry = mod_proto_registry_find_by_pid(pid);
        if (existing_entry != nullptr) {
            // HASH COLLISION DETECTED - CRITICAL WARNING
            char collision_msg[512];
            snprintf(collision_msg, sizeof(collision_msg),
                "HASH COLLISION WARNING!\n\n"
                "Your mod '%s' proto '%s'\n"
                "generated PID 0x%08X which is already used by:\n"
                "Mod '%s' proto '%s'\n\n"
                "This proto will be skipped.\n"
                "Rename your mod or proto to fix.",
                mod_name, proto_name, pid,
                existing_entry->mod_name, existing_entry->proto_name);

            showMesageBox(collision_msg);

            // Log for the report
            _mod_proto_collision_count++;
            char log_entry[256];
            snprintf(log_entry, sizeof(log_entry),
                "COLLISION: Mod '%s' proto '%s' (PID: 0x%08X) conflicts with Mod '%s' proto '%s'\n",
                mod_name, proto_name, pid,
                existing_entry->mod_name, existing_entry->proto_name);
            strncat(_mod_proto_collision_log, log_entry,
                sizeof(_mod_proto_collision_log) - strlen(_mod_proto_collision_log) - 1);

            debugPrint("Hash collision! Skipping %s:%s (PID: 0x%08X)\n",
                mod_name, proto_name, pid);
            continue; // Skip this proto due to collision
        }

        // Build path to .pro file
        char pro_path[COMPAT_MAX_PATH];
        char list_dir[COMPAT_MAX_PATH];
        strcpy(list_dir, list_path);
        char* last_slash = strrchr(list_dir, DIR_SEPARATOR);
        if (last_slash) {
            *(last_slash + 1) = '\0';
        }

        snprintf(pro_path, sizeof(pro_path), "%s%s.pro", list_dir, proto_name);

        // Check if .pro file exists
        File* pro_test = fileOpen(pro_path, "rb");
        if (!pro_test) {
            debugPrint("Warning: Mod proto file not found: %s\n", pro_path);
            continue;
        }
        fileClose(pro_test);

        // Create and store entry
        ModProtoEntry entry;
        entry.pid = pid;
        entry.mod_name = internal_strdup(mod_name);
        entry.proto_name = internal_strdup(proto_name);
        entry.proto_path = internal_strdup(pro_path);
        entry.type = proto_type;

        // Add to registry
        mod_proto_registry_add(&entry);

        // Create composite key for reverse lookup
        char composite_key[256];
        snprintf(composite_key, sizeof(composite_key), "%s:%s", mod_name, proto_name);
        name_to_pid_registry_add(composite_key, pid);
    }

    fileClose(stream);
}

/**
 * @brief Loads all mod proto lists for a given object type.
 *
 * Searches for mod-specific .lst files in the proto directory for a type
 * (e.g., proto/items/items_*.lst). Each found file represents a mod
 * adding protos of that type.
 *
 * @param proto_type Object type (OBJ_TYPE_ITEM, etc.).
 * @param proto_type_name String name of the type (e.g., "items").
 */
static void load_mod_proto_list(int proto_type, const char* proto_type_name)
{
    char search_pattern[COMPAT_MAX_PATH];
    char path[COMPAT_MAX_PATH];

    // Build path to proto directory for this type
    proto_make_path(path, proto_type << 24); // Gets us: {base}/proto/{type}/

    // {type}_{modname}.lst (e.g., items_testmod.lst)
    snprintf(search_pattern, sizeof(search_pattern), "%s%c%s_*.lst",
        path, DIR_SEPARATOR, proto_type_name);

    char** mod_files = nullptr;
    int file_count = fileNameListInit(search_pattern, &mod_files, 0, 0);

    for (int i = 0; i < file_count; i++) {
        // Extract mod name from filename: {type}_{modname}.lst -> {modname}
        const char* filename = mod_files[i];
        char mod_name[64] = { 0 };

        // Skip type name and underscore
        // Filename is like "items_testmod.lst", proto_type_name is "items"
        const char* mod_name_start = filename + strlen(proto_type_name) + 1; // Skip "items_"

        // Copy mod name without .lst extension
        const char* dot = strrchr(mod_name_start, '.');
        if (dot) {
            size_t mod_name_len = dot - mod_name_start;
            if (mod_name_len > 0 && mod_name_len < sizeof(mod_name)) {
                strncpy(mod_name, mod_name_start, mod_name_len);
                mod_name[mod_name_len] = '\0';
            }
        } else {
            // No extension? Use the whole thing
            strncpy(mod_name, mod_name_start, sizeof(mod_name) - 1);
        }

        if (strlen(mod_name) == 0) {
            continue; // Skip invalid names
        }

        // Build full path to the list file
        char list_path[COMPAT_MAX_PATH];
        snprintf(list_path, sizeof(list_path), "%s%c%s",
            path, DIR_SEPARATOR, mod_files[i]);

        load_single_mod_proto_list(list_path, mod_name, proto_type, proto_type_name);
    }

    if (file_count > 0) {
        fileNameListFree(&mod_files, 0);
    }
}

/**
 * @brief Parses a mod message file for proto messages.
 *
 * Reads a mod-specific message file (messages_*.txt) and extracts
 * name/description messages for mod protos. Messages are added to
 * the message repository for later lookup.
 *
 * @param full_path Full path to the message file.
 * @param filename Just the filename (for mod name extraction).
 */
static void load_mod_proto_messages_from_file(const char* full_path, const char* filename)
{
    File* stream = fileOpen(full_path, "rt");
    if (!stream) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Could not open mod proto messages file: %s", full_path);
        showMesageBox(msg);
        return;
    }

    // Extract mod name from filename (messages_xxx.txt -> xxx)
    char mod_name[64] = { 0 };
    const char* prefix = "messages_";
    const char* suffix = ".txt";

    if (strncmp(filename, prefix, strlen(prefix)) == 0) {
        size_t filename_len = strlen(filename);
        size_t mod_name_len = filename_len - strlen(prefix) - strlen(suffix);

        if (mod_name_len > 0 && mod_name_len < sizeof(mod_name)) {
            strncpy(mod_name, filename + strlen(prefix), mod_name_len);
            mod_name[mod_name_len] = '\0';
        }
    }

    char line[256];
    char current_section[64] = "";
    bool in_proto_section = false;

    while (fileReadString(line, sizeof(line) - 1, stream)) {
        // Remove line endings
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';
        char* cr = strchr(line, '\r');
        if (cr) *cr = '\0';

        // Skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // Check for section header
        if (line[0] == '[') {
            char* line_end = line + strlen(line) - 1;
            while (line_end > line && isspace(*line_end)) {
                *line_end = '\0';
                line_end--;
            }

            if (*line_end == ']') {
                // Extract section name
                char section_name[64];
                strncpy(section_name, line + 1, line_end - line - 1);
                section_name[line_end - line - 1] = '\0';

                // Trim whitespace
                char* start = section_name;
                while (*start && isspace(*start))
                    start++;
                char* end = start + strlen(start) - 1;
                while (end > start && isspace(*end))
                    *end-- = '\0';

                strncpy(current_section, start, sizeof(current_section) - 1);

                // Check if this is a proto section
                in_proto_section = (strstr(current_section, "proto") != nullptr) || (strstr(current_section, "pro_") != nullptr);

                continue;
            }
        }

        // Only process if we're in a proto section
        if (!in_proto_section) {
            continue;
        }

        // Parse key=value line for proto messages
        char* separator = strchr(line, '=');
        if (!separator) {
            continue;
        }

        *separator = '\0';
        char* key = line;
        char* value = separator + 1;

        // Trim whitespace
        while (*key && isspace(*key))
            key++;
        while (*value && isspace(*value))
            value++;

        char* end = key + strlen(key) - 1;
        while (end > key && isspace(*end))
            *end-- = '\0';

        end = value + strlen(value) - 1;
        while (end > value && isspace(*end))
            *end-- = '\0';

        if (*key && *value) {
            // Parse key format: could be "testitem:name" or "testmod:testitem:name"
            char* colon1 = strchr(key, ':');
            if (!colon1) {
                continue;
            }

            *colon1 = '\0';
            char* colon2 = strchr(colon1 + 1, ':');

            char key_mod_name[64] = { 0 };
            char proto_name[128] = { 0 };
            char message_type_str[16] = { 0 };

            if (colon2) {
                // Format: mod:proto:type
                *colon2 = '\0';
                strncpy(key_mod_name, key, sizeof(key_mod_name) - 1);
                strncpy(proto_name, colon1 + 1, sizeof(proto_name) - 1);
                strncpy(message_type_str, colon2 + 1, sizeof(message_type_str) - 1);
            } else {
                // Format: proto:type (use file's mod name)
                strncpy(key_mod_name, mod_name, sizeof(key_mod_name) - 1);
                strncpy(proto_name, key, sizeof(proto_name) - 1);
                strncpy(message_type_str, colon1 + 1, sizeof(message_type_str) - 1);
            }

            // Determine message type
            int message_type = -1;
            if (strcmp(message_type_str, "name") == 0 || strcmp(message_type_str, "0") == 0) {
                message_type = PROTOTYPE_MESSAGE_NAME;
            } else if (strcmp(message_type_str, "desc") == 0 || strcmp(message_type_str, "1") == 0) {
                message_type = PROTOTYPE_MESSAGE_DESCRIPTION;
            } else {
                continue;
            }

            if (message_type >= 0 && key_mod_name[0] && proto_name[0]) {
                // Look up PID for this mod proto
                char composite_key[256];
                snprintf(composite_key, sizeof(composite_key), "%s:%s", key_mod_name, proto_name);

                int pid = name_to_pid_registry_find(composite_key);
                if (pid != -1) {
                    // Add message to repository
                    messageListAddModProtoMessage(pid, message_type, value);
                }
            }
        }
    }

    fileClose(stream);
}

/**
 * @brief Loads mod proto messages separately from other mod messages.
 *
 * Proto messages are handled differently than other mod messages because:
 * 1. They use a special key format: {modname}:{protoname}:name/desc
 * 2. They're stored in a separate repository indexed by PID
 * 3. They're looked up via protoGetMessage() not regular message lists
 *
 * This function should NOT be merged into messageListLoadWithMods()
 * because the loading mechanism is fundamentally different.
 */
void load_mod_proto_messages()
{
    char search_pattern[COMPAT_MAX_PATH];

    // Use the same pattern as messageListLoad but with wildcard
    snprintf(search_pattern, sizeof(search_pattern), "text%c%s%cgame%cmessages_*.txt",
        DIR_SEPARATOR, ENGLISH, DIR_SEPARATOR, DIR_SEPARATOR);

    char** mod_files = nullptr;
    int file_count = fileNameListInit(search_pattern, &mod_files, 0, 0);

    for (int i = 0; i < file_count; i++) {
        // Build full path (same pattern as messageListLoad)
        char full_path[COMPAT_MAX_PATH];
        snprintf(full_path, sizeof(full_path), "text%c%s%cgame%c%s",
            DIR_SEPARATOR, ENGLISH, DIR_SEPARATOR, DIR_SEPARATOR, mod_files[i]);
        load_mod_proto_messages_from_file(full_path, mod_files[i]);
    }

    if (file_count > 0) {
        fileNameListFree(&mod_files, 0);
    }

    // Current language override
    if (compat_stricmp(settings.system.language.c_str(), ENGLISH) != 0) {
        snprintf(search_pattern, sizeof(search_pattern), "text%c%s%cgame%cmessages_*.txt",
            DIR_SEPARATOR, settings.system.language.c_str(), DIR_SEPARATOR, DIR_SEPARATOR);

        file_count = fileNameListInit(search_pattern, &mod_files, 0, 0);

        for (int i = 0; i < file_count; i++) {
            char full_path[COMPAT_MAX_PATH];
            snprintf(full_path, sizeof(full_path), "text%c%s%cgame%c%s",
                DIR_SEPARATOR, settings.system.language.c_str(),
                DIR_SEPARATOR, DIR_SEPARATOR, mod_files[i]);
            load_mod_proto_messages_from_file(full_path, mod_files[i]);
        }

        if (file_count > 0) {
            fileNameListFree(&mod_files, 0);
        }
    }
}

/**
 * @brief Gets a PID for a mod proto by name.
 *
 * Public API function that mods/scripts can use to get the PID for a
 * mod proto by its mod and proto names. Useful for scripting integration.
 *
 * @param mod_name Name of the mod (e.g., "testmod").
 * @param proto_name Name of the proto within the mod (e.g., "testitem").
 * @param proto_type Object type (OBJ_TYPE_ITEM, etc.).
 * @return The PID if found, or a generated PID if not found (for dynamic mods).
 */
int protoGetModPid(const char* mod_name, const char* proto_name, int proto_type)
{
    char composite_key[256];
    snprintf(composite_key, sizeof(composite_key), "%s:%s", mod_name, proto_name);

    int pid = name_to_pid_registry_find(composite_key);
    if (pid != -1) {
        return pid;
    }

    // Not found, could generate it (for dynamic mod support)
    return generate_mod_proto_pid(mod_name, proto_name, proto_type);
}

// ============================================================================
// Initialization & Cleanup
// ============================================================================

// proto_init
// 0x4A0390
int protoInit()
{
    size_t len;
    MessageListItem messageListItem;
    char path[COMPAT_MAX_PATH];
    int i;

    snprintf(path, sizeof(path), "%s\\proto", settings.system.master_patches_path.c_str());
    len = strlen(path);

    compat_mkdir(path);

    strcpy(path + len, "\\critters");
    compat_mkdir(path);

    strcpy(path + len, "\\items");
    compat_mkdir(path);

    // TODO: Get rid of cast.
    proto_critter_init((Proto*)&gDudeProto, 0x1000000);

    gDudeProto.pid = 0x1000000;
    gDudeProto.fid = buildFid(OBJ_TYPE_CRITTER, 1, 0, 0, 0);

    gDude->pid = 0x1000000;
    gDude->sid = 1;

    for (i = 0; i < 6; i++) {
        _proto_remove_list(i);
    }

    _proto_header_load();

    // Load mod protos for each type (AFTER vanilla proto initialization)
    load_mod_proto_list(OBJ_TYPE_ITEM, "items");
    load_mod_proto_list(OBJ_TYPE_CRITTER, "critters");
    load_mod_proto_list(OBJ_TYPE_SCENERY, "scenery");
    load_mod_proto_list(OBJ_TYPE_WALL, "walls");
    load_mod_proto_list(OBJ_TYPE_TILE, "tiles");
    load_mod_proto_list(OBJ_TYPE_MISC, "misc");

    _protos_been_initialized = 1;

    _proto_dude_init("premade\\player.gcd");

    for (i = 0; i < 6; i++) {
        if (!messageListInit(&(_proto_msg_files[i]))) {
            debugPrint("\nError: Initing proto message files!");
            return -1;
        }
    }

    for (i = 0; i < 6; i++) {
        snprintf(path, sizeof(path), "%spro_%.4s%s", asc_5186C8, artGetObjectTypeName(i), ".msg");

        if (!messageListLoad(&(_proto_msg_files[i]), path)) {
            debugPrint("\nError: Loading proto message files!");
            return -1;
        }
    }

    // SPECIAL: Load mod proto messages separately because:
    // 1. They use different key format (mod:proto:name vs area_name:NAME)
    // 2. They go to a special mod proto message repository
    // 3. They're looked up by PID, not message ID
    load_mod_proto_messages();

    // Generate debug report
    protoGenerateModProtoListDebug();

    for (i = 0; i < 6; i++) {
        messageListRepositorySetProtoMessageList(i, &(_proto_msg_files[i]));
    }

    _mp_critter_stats_list = _aDrugStatSpecia;
    _critter_stats_list = _critter_stats_list_strs;
    _critter_stats_list_None = _aNone_1;
    for (i = 0; i < STAT_COUNT; i++) {
        _critter_stats_list_strs[i] = statGetName(i);
        if (_critter_stats_list_strs[i] == nullptr) {
            debugPrint("\nError: Finding stat names!");
            return -1;
        }
    }

    _mp_perk_code_None = _aNone_1;
    _perk_code_strs = _mp_perk_code_strs;
    for (i = 0; i < PERK_COUNT; i++) {
        _mp_perk_code_strs[i] = perkGetName(i);
        if (_mp_perk_code_strs[i] == nullptr) {
            debugPrint("\nError: Finding perk names!");
            return -1;
        }
    }

    if (!messageListInit(&gProtoMessageList)) {
        debugPrint("\nError: Initing main proto message file!");
        return -1;
    }

    snprintf(path, sizeof(path), "%sproto.msg", asc_5186C8);

    if (!messageListLoad(&gProtoMessageList, path)) {
        debugPrint("\nError: Loading main proto message file!");
        return -1;
    }

    _proto_none_str = getmsg(&gProtoMessageList, &messageListItem, 10);

    // material type names
    for (i = 0; i < MATERIAL_TYPE_COUNT; i++) {
        gMaterialTypeNames[i] = getmsg(&gProtoMessageList, &messageListItem, 100 + i);
    }

    // item type names
    for (i = 0; i < ITEM_TYPE_COUNT; i++) {
        gItemTypeNames[i] = getmsg(&gProtoMessageList, &messageListItem, 150 + i);
    }

    // scenery type names
    for (i = 0; i < SCENERY_TYPE_COUNT; i++) {
        gSceneryTypeNames[i] = getmsg(&gProtoMessageList, &messageListItem, 200 + i);
    }

    // damage code types
    for (i = 0; i < DAMAGE_TYPE_COUNT; i++) {
        gDamageTypeNames[i] = getmsg(&gProtoMessageList, &messageListItem, 250 + i);
    }

    // caliber types
    for (i = 0; i < CALIBER_TYPE_COUNT; i++) {
        gCaliberTypeNames[i] = getmsg(&gProtoMessageList, &messageListItem, 300 + i);
    }

    // race types
    for (i = 0; i < RACE_TYPE_COUNT; i++) {
        gRaceTypeNames[i] = getmsg(&gProtoMessageList, &messageListItem, 350 + i);
    }

    // body types
    for (i = 0; i < BODY_TYPE_COUNT; i++) {
        gBodyTypeNames[i] = getmsg(&gProtoMessageList, &messageListItem, 400 + i);
    }

    messageListRepositorySetStandardMessageList(STANDARD_MESSAGE_LIST_PROTO, &gProtoMessageList);

    return 0;
}

// 0x4A0814
void protoReset()
{
    int i;

    // TODO: Get rid of cast.
    proto_critter_init((Proto*)&gDudeProto, 0x1000000);
    gDudeProto.pid = 0x1000000;
    gDudeProto.fid = buildFid(OBJ_TYPE_CRITTER, 1, 0, 0, 0);

    gDude->pid = 0x1000000;
    gDude->sid = -1;
    gDude->flags &= ~OBJECT_FLAG_0xFC000;

    for (i = 0; i < 6; i++) {
        _proto_remove_list(i);
    }

    _proto_header_load();

    _protos_been_initialized = 1;
    _proto_dude_init("premade\\player.gcd");
}

// 0x4A0898
void protoExit()
{
    int i;

    for (i = 0; i < 6; i++) {
        _proto_remove_list(i);
    }

    for (i = 0; i < 6; i++) {
        messageListRepositorySetProtoMessageList(i, nullptr);
        messageListFree(&(_proto_msg_files[i]));
    }

    messageListRepositorySetStandardMessageList(STANDARD_MESSAGE_LIST_PROTO, nullptr);
    messageListFree(&gProtoMessageList);

    // Clean up mod proto registry arrays
    if (_mod_proto_entries) {
        internal_free(_mod_proto_entries);
        _mod_proto_entries = nullptr;
        _mod_proto_entries_size = 0;
        _mod_proto_entries_capacity = 0;
    }

    if (_name_to_pid_entries) {
        for (int i = 0; i < _name_to_pid_entries_size; i++) {
            if (_name_to_pid_entries[i].key) {
                internal_free(_name_to_pid_entries[i].key);
            }
        }
        internal_free(_name_to_pid_entries);
        _name_to_pid_entries = nullptr;
        _name_to_pid_entries_size = 0;
        _name_to_pid_entries_capacity = 0;
    }

    // Print summary on exit
    if (_mod_proto_entries_size > 0) {
        debugPrint("Proto system exiting with %d mod protos loaded\n",
            _mod_proto_entries_size);
    }
}

/**
 * @brief Generates a comprehensive debug report of all loaded mod protos.
 *
 * Creates a detailed report file (data/lists/proto_list.txt) showing:
 * - All loaded mod protos organized by type
 * - Generated PIDs with hex and decimal representations
 * - Mod name and proto name for each entry
 * - File paths and message IDs
 * - Statistics and collision warnings
 *
 * This report is essential for mod debugging and script integration.
 */
static void protoGenerateModProtoListDebug()
{
    char debugPath[COMPAT_MAX_PATH];
    snprintf(debugPath, sizeof(debugPath), "./data%clists%cproto_list.txt",
        DIR_SEPARATOR, DIR_SEPARATOR);

    // Ensure directory exists
    char dirPath[COMPAT_MAX_PATH];
    snprintf(dirPath, sizeof(dirPath), "./data%clists", DIR_SEPARATOR);
    compat_mkdir(dirPath);

    FILE* debugStream = compat_fopen(debugPath, "wt");
    if (debugStream == nullptr) {
        debugPrint("\nprotoGenerateModProtoListDebug: Could not create proto_list.txt");
        return;
    }

    // Write header
    const char* header = "==============================================================================\n"
                         "Fallout 2 FISSION - Mod Proto Report\n"
                         "==============================================================================\n"
                         "This report shows all mod protos loaded by the engine - essential for mod debugging\n"
                         "and finding PIDs for mod development.\n\n"

                         "KEY CONCEPTS:\n"
                         "- Vanilla protos: Use indices 0-199 (0x000000-0x0000C7) in lower 24 bits\n"
                         "- Mod protos: Use indices 200-16777215 (0x0000C8-0xFFFFFF) via deterministic hashing\n"
                         "- PID format: (type << 24) | index (e.g., 0x00DB240E = item type with index 0xDB240E)\n"
                         "- Index >= 0x0000C8 (200) indicates a mod proto\n"
                         "- Hash collisions trigger warnings and the colliding proto is skipped\n\n"

                         "CRITICAL - HOW MOD PIDS ARE GENERATED:\n"
                         "- PIDs are generated from mod name + proto name hash (e.g., 'testmod:testitem')\n"
                         "- The PID stored in the .pro file (first 4 bytes) is IGNORED for mod protos\n"
                         "- Always use the PIDs shown in this report, NOT what's in the .pro file\n"
                         "- Same mod+proto names = same PID across all installations (STABLE)\n\n"

                         "USAGE NOTES:\n"
                         "- Use these PIDs when referencing mod protos in:\n"
                         "  * Scripts (create_object, add_obj_to_inven, etc.)\n"
                         "  * Map files (PROTO item entries)\n"
                         "  * Message files (for name/description)\n"
                         "- Mod PIDs are STABLE between game sessions\n"
                         "- Proto messages are loaded from text/{lang}/game/messages_{modname}.txt\n"
                         "- Message file format: [proto_{modname}] section with keys:\n"
                         "    {modname}:{protoname}:name = Display Name\n"
                         "    {modname}:{protoname}:desc = Description Text\n"
                         "  Alternative (short) format:\n"
                         "    {protoname}:name = Display Name\n"
                         "    {protoname}:desc = Description Text\n"
                         "==============================================================================\n\n";

    fputs(header, debugStream);

    // Write timestamp
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    fprintf(debugStream, "Report Generated: %04d-%02d-%02d %02d:%02d:%02d\n\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec);

    // Add collision section if any collisions occurred
    if (_mod_proto_collision_count > 0) {
        fprintf(debugStream, "\n=== HASH COLLISIONS DETECTED ===\n");
        fprintf(debugStream, "Total hash collisions: %d (protos skipped)\n\n", _mod_proto_collision_count);

        if (strlen(_mod_proto_collision_log) > 0) {
            fprintf(debugStream, "Collision details:\n");
            fprintf(debugStream, "%s", _mod_proto_collision_log);
        }

        fprintf(debugStream, "\nTo fix collisions, rename your mod or proto files.\n");
    }

    // Statistics
    int totalModProtos = _mod_proto_entries_size;
    int protosByType[OBJ_TYPE_COUNT] = { 0 };

    // Count mod protos by type
    for (int i = 0; i < _mod_proto_entries_size; i++) {
        int type = _mod_proto_entries[i].type;
        if (type >= 0 && type < OBJ_TYPE_COUNT) {
            protosByType[type]++;
        }
    }

    // Summary section
    fprintf(debugStream, "MOD PROTO STATISTICS:\n");
    fprintf(debugStream, "---------------------\n");
    fprintf(debugStream, "Total Mod Protos: %d\n", totalModProtos);
    fprintf(debugStream, "\nBy Type:\n");
    for (int i = 0; i < OBJ_TYPE_COUNT; i++) {
        if (protosByType[i] > 0) {
            fprintf(debugStream, "  %-8s: %d\n",
                artGetObjectTypeName(i), protosByType[i]);
        }
    }
    fprintf(debugStream, "\n");

    // PID Ranges
    fprintf(debugStream, "PID RANGES:\n");
    fprintf(debugStream, "-----------\n");
    fprintf(debugStream, "Vanilla: 0x00000000 - 0x00FFFFFF (lower 24 bits: 0x000000-0x0000C7)\n");
    fprintf(debugStream, "Mod:     0x00000000 - 0xFFFFFFFF (lower 24 bits: 0x0000C8-0xFFFFFF)\n");
    fprintf(debugStream, "\nType Bits (bits 24-31):\n");
    fprintf(debugStream, "  0x00 = Items, 0x01 = Critters, 0x02 = Scenery,\n");
    fprintf(debugStream, "  0x03 = Walls,  0x04 = Tiles,   0x05 = Misc\n");
    fprintf(debugStream, "\nExample: 0x00DB240E = Item (type 0) with mod index 0xDB240E\n");
    fprintf(debugStream, "\n");

    // Detailed mod proto listing by type
    for (int type = 0; type < OBJ_TYPE_COUNT; type++) {
        if (protosByType[type] == 0) continue;

        fprintf(debugStream, "%s MOD PROTOS:\n", artGetObjectTypeName(type));
        for (int i = 0; i < strlen(artGetObjectTypeName(type)) + 12; i++) {
            fputc('-', debugStream);
        }
        fputc('\n', debugStream);

        for (int i = 0; i < _mod_proto_entries_size; i++) {
            if (_mod_proto_entries[i].type != type) continue;

            ModProtoEntry* entry = &_mod_proto_entries[i];

            // Get proto data to display additional info
            Proto* proto = nullptr;
            bool protoLoaded = (protoGetProto(entry->pid, &proto) == 0);

            // Extract file name from path
            const char* fileName = strrchr(entry->proto_path, DIR_SEPARATOR);
            if (fileName)
                fileName++;
            else
                fileName = entry->proto_path;

            fprintf(debugStream, "  PID: 0x%08X (%d)\n", entry->pid, entry->pid);
            fprintf(debugStream, "    Mod:      %s\n", entry->mod_name);
            fprintf(debugStream, "    Proto:    %s\n", entry->proto_name);
            fprintf(debugStream, "    File:     %s\n", fileName);

            if (protoLoaded) {
                fprintf(debugStream, "    Message:  ID %d\n", proto->messageId);

                // Try to get name and description
                char* name = protoGetName(entry->pid);
                char* desc = protoGetDescription(entry->pid);

                if (name && name != _proto_none_str) {
                    fprintf(debugStream, "    Name:     %s\n", name);
                }
                if (desc && desc != _proto_none_str) {
                    // Truncate description for readability
                    char shortDesc[128];
                    strncpy(shortDesc, desc, sizeof(shortDesc) - 1);
                    shortDesc[sizeof(shortDesc) - 1] = '\0';
                    if (strlen(desc) > sizeof(shortDesc) - 1) {
                        strcpy(shortDesc + sizeof(shortDesc) - 4, "...");
                    }
                    fprintf(debugStream, "    Desc:     %s\n", shortDesc);
                }

                // Type-specific info
                switch (type) {
                case OBJ_TYPE_ITEM:
                    fprintf(debugStream, "    Item Type: %s\n",
                        proto->item.type < ITEM_TYPE_COUNT ? gItemTypeNames[proto->item.type] : "Unknown");
                    fprintf(debugStream, "    Weight:    %d, Cost: %d\n",
                        proto->item.weight, proto->item.cost);
                    break;
                case OBJ_TYPE_CRITTER:
                    fprintf(debugStream, "    FID:       0x%08X\n", proto->fid);
                    break;
                default:
                    break;
                }
            }
            fprintf(debugStream, "\n");
        }
        fprintf(debugStream, "\n");
    }

    // Name-to-PID Registry Section
    fprintf(debugStream, "NAME-TO-PID REGISTRY (for message file parsing):\n");
    for (int i = 0; i < 60; i++)
        fputc('-', debugStream);
    fputc('\n', debugStream);

    for (int i = 0; i < _name_to_pid_entries_size; i++) {
        fprintf(debugStream, "  %-40s -> 0x%08X\n",
            _name_to_pid_entries[i].key,
            _name_to_pid_entries[i].pid);
    }
    fprintf(debugStream, "\n");

    // Message Loading Section
    fprintf(debugStream, "MESSAGE LOADING:\n");
    for (int i = 0; i < 16; i++)
        fputc('-', debugStream);
    fputc('\n', debugStream);

    fprintf(debugStream, "Message files should be in: text/{language}/game/messages_{modname}.txt\n");
    fprintf(debugStream, "Format in message files:\n");
    fprintf(debugStream, "  [proto_{modname}]  (or any section containing 'proto' or 'pro_')\n");
    fprintf(debugStream, "  {modname}:{protoname}:name = Name Text\n");
    fprintf(debugStream, "  {modname}:{protoname}:desc = Description Text\n");
    fprintf(debugStream, "Alternative format (using filename mod name):\n");
    fprintf(debugStream, "  {protoname}:name = Name Text  (uses filename for mod name)\n");
    fprintf(debugStream, "\n");

    // Important notes footer
    fputs("=== IMPORTANT NOTES ===\n", debugStream);
    fputs("- Mod proto PIDs are STABLE - they won't change between game sessions\n", debugStream);
    fputs("- Hash collisions show warnings and skip the conflicting proto\n", debugStream);
    fputs("- Reference these exact PIDs in your scripts and map files\n", debugStream);
    fputs("- Use 'modname:protoname' format in message files for mod proto messages\n", debugStream);
    fputs("- Test message loading by checking the name/description in the report above\n", debugStream);
    fputs("- Mod .lst files should be named: {type}_{modname}.lst (e.g., items_mymod.lst)\n", debugStream);

    // Close file
    fclose(debugStream);
    debugPrint("\nprotoGenerateModProtoListDebug: Generated proto_list.txt with %d mod protos",
        totalModProtos);
}

} // namespace fallout
