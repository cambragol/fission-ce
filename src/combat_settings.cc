// combat_settings.cpp
#include "combat_settings.h"

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "debug.h"
#include "game.h"
#include "game_version.h"

namespace fallout {

CombatSettings gCombatSettings;

// Built-in fallbacks. Used only if combat.txt is missing, or a key is absent.
// The .txt file is authoritative on a healthy install.
static void setDefaultCombatSettings(CombatSettings& s)
{
    // [combat]
    s.hitLocationPenalty[0] = -40; // head
    s.hitLocationPenalty[1] = -30; // left_arm
    s.hitLocationPenalty[2] = -30; // right_arm
    s.hitLocationPenalty[3] = 0;   // torso
    s.hitLocationPenalty[4] = -20; // right_leg
    s.hitLocationPenalty[5] = -20; // left_leg
    s.hitLocationPenalty[6] = -60; // eyes
    s.hitLocationPenalty[7] = -30; // groin
    s.hitLocationPenalty[8] = 0;   // uncalled

    s.difficultyToHitEasy = -20;
    s.difficultyToHitHard = 20;
    s.difficultyDamageEasy = 75;
    s.difficultyDamageHard = 125;

    s.rangedPerceptionLongRange = 4;
    s.rangedPerceptionScope = 5;
    s.rangedPerceptionDefault = 2;
    s.scopeMinEffective = 8;
    s.rangedDistanceBlindMult = -12;
    s.rangedDistanceNormalMult = -4;
    s.lofCritterPenalty = 10;

    s.oneHanderTwoHandedPenalty = -40;
    s.oneHanderOneHandedBonus = 20;
    s.minStrengthPenalty = 20;
    s.weaponHandlingBonus = 3;
    s.accurateBonus = 20;
    s.multihexToHitBonus = 15;

    s.lightThreshold[0] = 26214;
    s.lightThreshold[1] = 39321;
    s.lightThreshold[2] = 52428;
    s.lightPenalty[0] = -40;
    s.lightPenalty[1] = -25;
    s.lightPenalty[2] = -10;

    s.blindToHitPenalty = -25;
    s.knockedToHitBonus = 40;
    s.toHitCap = 95;

    s.bypassThresholdPercent = 20;
    s.bypassDrPercent = 20;
    s.penetrateThresholdPercent = 20;
    s.finesseDrBonus = 30;
    s.bonusRangedDamagePerRank = 2;
    s.damageDivisor = 2;
    s.livingAnatomyBonus = 5;
    s.pyromaniacBonus = 5;

    s.knockbackDivisorDefault = 10;
    s.knockbackDivisorPerk = 5;
    s.stonewallChance = 50;
    s.stonewallDivisor = 2;

    s.rangedMissAccuracyDivisor = 3;
    s.rangedMissDeadChance = 5;

    s.safetyIntThreshold = 5;
    s.safetyRadiusReduction = 1;

    // [criticals]
    s.critEffectThreshold[0] = 20;
    s.critEffectThreshold[1] = 45;
    s.critEffectThreshold[2] = 70;
    s.critEffectThreshold[3] = 90;
    s.critEffectThreshold[4] = 100;

    s.critFailureThreshold[0] = 20;
    s.critFailureThreshold[1] = 50;
    s.critFailureThreshold[2] = 75;
    s.critFailureThreshold[3] = 95;
    s.critFailureLuckPenalty = 5;
    s.playerGraceDays = 6;
    s.critFailureSelfDamageMin = 1;
    s.critFailureSelfDamageMax = 5;

    // [ai]
    s.runAwayHp[0] = 0;
    s.runAwayHp[1] = 25;
    s.runAwayHp[2] = 40;
    s.runAwayHp[3] = 60;
    s.runAwayHp[4] = 75;
    s.runAwayHp[5] = 100;

    s.partyMemberDistances[0] = 5;
    s.partyMemberDistances[1] = 7;
    s.partyMemberDistances[2] = 7;
    s.partyMemberDistances[3] = 7;
    s.partyMemberDistances[4] = 50000;

    s.maxTeammateDistance = 5;
    s.aiSearchPerceptionBonus = 5;
    s.aiBestWeaponDamageDiff = 5;
    s.aiReloadAttempts = 10;

    s.calledShotIntEasy = 7;
    s.calledShotIntNormal = 5;
    s.calledShotIntHard = 3;

    s.areaAttackToHitSure = 85;
    s.areaAttackToHitCareful = 50;
    s.areaAttackToHitAbsolutelySure = 95;

    s.secondaryModeInt = 6;
    s.secondaryModeDistance = 10;
    s.minMoveDistance = 1;
    s.runApThresholdDiv = 2;

    s.friendlyFireIntPenalty = 2;
    s.friendlyDeadDistance = 10;

    s.perceptionSightMult = 5;
    s.perceptionCombatMult = 2;
    s.sneak120Bonus = -1;
    s.sneakDistanceDiv = 4;
    s.sneakingStateNum = 2;
    s.sneakingStateDen = 3;
    s.transGlassDiv = 2;

    // [chem]
    s.chemStimsHurtLittle = 60;
    s.chemStimsHurtLots = 30;
    s.chemStimsHp = 50;
    s.chemSometimesChance = 25;
    s.chemAnytimeChance = 75;
    s.chemAlwaysChance = 100;
    s.chemRandomDrugPicks = 3;
    s.chemApCost = 2;
}

// Reads an int. If missing, leaves `value` unchanged. If out of range, clamps
// and logs. Never writes a value outside [min, max].
static void getIntClamped(Config* cfg, const char* section, const char* key,
                          int min, int max, int& value)
{
    int parsed;
    if (!configGetInt(cfg, section, key, &parsed)) {
        return;
    }
    if (parsed < min || parsed > max) {
        int clamped = parsed < min ? min : max;
        debugPrint("combat.txt: [%s] %s = %d out of range [%d, %d], clamped to %d\n",
                   section, key, parsed, min, max, clamped);
        parsed = clamped;
    }
    value = parsed;
}

// Force a strictly increasing sequence. Used for critical thresholds and
// light bands, where an unordered set would silently change game behaviour.
static void enforceIncreasing(int* arr, int count, int min, int max)
{
    int prev = min - 1;
    for (int i = 0; i < count; i++) {
        if (arr[i] < min) arr[i] = min;
        if (arr[i] > max) arr[i] = max;
        if (arr[i] <= prev) arr[i] = prev + 1;
        if (arr[i] > max) arr[i] = max;
        prev = arr[i];
    }
}

static const char* kHitLocationNames[9] = {
    "head", "left_arm", "right_arm", "torso",
    "right_leg", "left_leg", "eyes", "groin", "uncalled",
};

void combatSettingsInit()
{
    setDefaultCombatSettings(gCombatSettings);

    Config config;
    if (!configInit(&config)) {
        debugPrint("combat.txt: configInit failed, using defaults\n");
        return;
    }

    if (!configRead(&config, GAME_DATA_PATH("combat.txt"), true)) {
        debugPrint("combat.txt: not found, using defaults\n");
        configFree(&config);
        return;
    }

    // ---- [combat] ----
    for (int i = 0; i < 9; i++) {
        char key[64];
        snprintf(key, sizeof(key), "hit_location_penalty_%s", kHitLocationNames[i]);
        getIntClamped(&config, "combat", key, -200, 200, gCombatSettings.hitLocationPenalty[i]);
    }

    getIntClamped(&config, "combat", "difficulty_to_hit_easy", -100, 0, gCombatSettings.difficultyToHitEasy);
    getIntClamped(&config, "combat", "difficulty_to_hit_hard", 0, 100, gCombatSettings.difficultyToHitHard);
    getIntClamped(&config, "combat", "difficulty_damage_easy", 1, 500, gCombatSettings.difficultyDamageEasy);
    getIntClamped(&config, "combat", "difficulty_damage_hard", 1, 500, gCombatSettings.difficultyDamageHard);

    getIntClamped(&config, "combat", "ranged_perception_long_range", 0, 20, gCombatSettings.rangedPerceptionLongRange);
    getIntClamped(&config, "combat", "ranged_perception_scope", 0, 20, gCombatSettings.rangedPerceptionScope);
    getIntClamped(&config, "combat", "ranged_perception_default", 0, 20, gCombatSettings.rangedPerceptionDefault);
    getIntClamped(&config, "combat", "scope_min_effective", 0, 50, gCombatSettings.scopeMinEffective);
    getIntClamped(&config, "combat", "ranged_distance_blind_mult", -50, 0, gCombatSettings.rangedDistanceBlindMult);
    getIntClamped(&config, "combat", "ranged_distance_normal_mult", -50, 0, gCombatSettings.rangedDistanceNormalMult);
    getIntClamped(&config, "combat", "lof_critter_penalty", 0, 50, gCombatSettings.lofCritterPenalty);

    getIntClamped(&config, "combat", "one_hander_two_handed_penalty", -200, 0, gCombatSettings.oneHanderTwoHandedPenalty);
    getIntClamped(&config, "combat", "one_hander_one_handed_bonus", 0, 100, gCombatSettings.oneHanderOneHandedBonus);
    getIntClamped(&config, "combat", "min_strength_penalty", 0, 100, gCombatSettings.minStrengthPenalty);
    getIntClamped(&config, "combat", "weapon_handling_bonus", 0, 10, gCombatSettings.weaponHandlingBonus);
    getIntClamped(&config, "combat", "accurate_bonus", 0, 100, gCombatSettings.accurateBonus);
    getIntClamped(&config, "combat", "multihex_to_hit_bonus", 0, 100, gCombatSettings.multihexToHitBonus);

    getIntClamped(&config, "combat", "light_threshold_1", 0, 65536, gCombatSettings.lightThreshold[0]);
    getIntClamped(&config, "combat", "light_threshold_2", 0, 65536, gCombatSettings.lightThreshold[1]);
    getIntClamped(&config, "combat", "light_threshold_3", 0, 65536, gCombatSettings.lightThreshold[2]);
    enforceIncreasing(gCombatSettings.lightThreshold, 3, 0, 65536);

    getIntClamped(&config, "combat", "light_penalty_1", -100, 0, gCombatSettings.lightPenalty[0]);
    getIntClamped(&config, "combat", "light_penalty_2", -100, 0, gCombatSettings.lightPenalty[1]);
    getIntClamped(&config, "combat", "light_penalty_3", -100, 0, gCombatSettings.lightPenalty[2]);

    getIntClamped(&config, "combat", "blind_to_hit_penalty", -100, 0, gCombatSettings.blindToHitPenalty);
    getIntClamped(&config, "combat", "knocked_to_hit_bonus", 0, 100, gCombatSettings.knockedToHitBonus);
    getIntClamped(&config, "combat", "to_hit_cap", 0, 200, gCombatSettings.toHitCap);

    getIntClamped(&config, "combat", "bypass_threshold_percent", 0, 100, gCombatSettings.bypassThresholdPercent);
    getIntClamped(&config, "combat", "bypass_dr_percent", 0, 100, gCombatSettings.bypassDrPercent);
    getIntClamped(&config, "combat", "penetrate_threshold_percent", 0, 100, gCombatSettings.penetrateThresholdPercent);
    getIntClamped(&config, "combat", "finesse_dr_bonus", -100, 100, gCombatSettings.finesseDrBonus);
    getIntClamped(&config, "combat", "bonus_ranged_damage_per_rank", 0, 20, gCombatSettings.bonusRangedDamagePerRank);
    getIntClamped(&config, "combat", "damage_divisor", 1, 20, gCombatSettings.damageDivisor);
    getIntClamped(&config, "combat", "living_anatomy_bonus", 0, 100, gCombatSettings.livingAnatomyBonus);
    getIntClamped(&config, "combat", "pyromaniac_bonus", 0, 100, gCombatSettings.pyromaniacBonus);

    getIntClamped(&config, "combat", "knockback_divisor_default", 1, 100, gCombatSettings.knockbackDivisorDefault);
    getIntClamped(&config, "combat", "knockback_divisor_perk", 1, 100, gCombatSettings.knockbackDivisorPerk);
    getIntClamped(&config, "combat", "stonewall_chance", 0, 100, gCombatSettings.stonewallChance);
    getIntClamped(&config, "combat", "stonewall_divisor", 1, 20, gCombatSettings.stonewallDivisor);

    getIntClamped(&config, "combat", "ranged_miss_accuracy_divisor", 1, 100, gCombatSettings.rangedMissAccuracyDivisor);
    getIntClamped(&config, "combat", "ranged_miss_dead_chance", 0, 100, gCombatSettings.rangedMissDeadChance);

    getIntClamped(&config, "combat", "safety_int_threshold", 0, 10, gCombatSettings.safetyIntThreshold);
    getIntClamped(&config, "combat", "safety_radius_reduction", 0, 10, gCombatSettings.safetyRadiusReduction);

    // ---- [criticals] ----
    getIntClamped(&config, "criticals", "effect_threshold_1", 0, 100, gCombatSettings.critEffectThreshold[0]);
    getIntClamped(&config, "criticals", "effect_threshold_2", 0, 100, gCombatSettings.critEffectThreshold[1]);
    getIntClamped(&config, "criticals", "effect_threshold_3", 0, 100, gCombatSettings.critEffectThreshold[2]);
    getIntClamped(&config, "criticals", "effect_threshold_4", 0, 100, gCombatSettings.critEffectThreshold[3]);
    getIntClamped(&config, "criticals", "effect_threshold_5", 0, 100, gCombatSettings.critEffectThreshold[4]);
    enforceIncreasing(gCombatSettings.critEffectThreshold, 5, 0, 100);

    getIntClamped(&config, "criticals", "failure_threshold_1", 0, 100, gCombatSettings.critFailureThreshold[0]);
    getIntClamped(&config, "criticals", "failure_threshold_2", 0, 100, gCombatSettings.critFailureThreshold[1]);
    getIntClamped(&config, "criticals", "failure_threshold_3", 0, 100, gCombatSettings.critFailureThreshold[2]);
    getIntClamped(&config, "criticals", "failure_threshold_4", 0, 100, gCombatSettings.critFailureThreshold[3]);
    enforceIncreasing(gCombatSettings.critFailureThreshold, 4, 0, 100);

    getIntClamped(&config, "criticals", "failure_luck_penalty", 0, 20, gCombatSettings.critFailureLuckPenalty);
    getIntClamped(&config, "criticals", "player_grace_days", 0, 100, gCombatSettings.playerGraceDays);
    getIntClamped(&config, "criticals", "failure_self_damage_min", 0, 100, gCombatSettings.critFailureSelfDamageMin);
    getIntClamped(&config, "criticals", "failure_self_damage_max", 0, 100, gCombatSettings.critFailureSelfDamageMax);
    if (gCombatSettings.critFailureSelfDamageMin > gCombatSettings.critFailureSelfDamageMax) {
        int t = gCombatSettings.critFailureSelfDamageMin;
        gCombatSettings.critFailureSelfDamageMin = gCombatSettings.critFailureSelfDamageMax;
        gCombatSettings.critFailureSelfDamageMax = t;
    }

    // ---- [ai] ----
    for (int i = 0; i < 6; i++) {
        char key[32];
        snprintf(key, sizeof(key), "run_away_hp_%d", i);
        getIntClamped(&config, "ai", key, 0, 100, gCombatSettings.runAwayHp[i]);
    }

    for (int i = 0; i < 5; i++) {
        char key[32];
        snprintf(key, sizeof(key), "party_distance_%d", i);
        getIntClamped(&config, "ai", key, 0, 1000000, gCombatSettings.partyMemberDistances[i]);
    }

    getIntClamped(&config, "ai", "max_teammate_distance", 0, 100, gCombatSettings.maxTeammateDistance);
    getIntClamped(&config, "ai", "search_perception_bonus", 0, 20, gCombatSettings.aiSearchPerceptionBonus);
    getIntClamped(&config, "ai", "best_weapon_damage_diff", 0, 100, gCombatSettings.aiBestWeaponDamageDiff);
    getIntClamped(&config, "ai", "reload_attempts", 1, 100, gCombatSettings.aiReloadAttempts);

    getIntClamped(&config, "ai", "called_shot_int_easy", 0, 10, gCombatSettings.calledShotIntEasy);
    getIntClamped(&config, "ai", "called_shot_int_normal", 0, 10, gCombatSettings.calledShotIntNormal);
    getIntClamped(&config, "ai", "called_shot_int_hard", 0, 10, gCombatSettings.calledShotIntHard);

    getIntClamped(&config, "ai", "area_attack_to_hit_sure", 0, 100, gCombatSettings.areaAttackToHitSure);
    getIntClamped(&config, "ai", "area_attack_to_hit_careful", 0, 100, gCombatSettings.areaAttackToHitCareful);
    getIntClamped(&config, "ai", "area_attack_to_hit_absolutely_sure", 0, 100, gCombatSettings.areaAttackToHitAbsolutelySure);

    getIntClamped(&config, "ai", "secondary_mode_int", 0, 10, gCombatSettings.secondaryModeInt);
    getIntClamped(&config, "ai", "secondary_mode_distance", 0, 100, gCombatSettings.secondaryModeDistance);
    getIntClamped(&config, "ai", "min_move_distance", 0, 100, gCombatSettings.minMoveDistance);
    getIntClamped(&config, "ai", "run_ap_threshold_div", 1, 10, gCombatSettings.runApThresholdDiv);

    getIntClamped(&config, "ai", "friendly_fire_int_penalty", 0, 20, gCombatSettings.friendlyFireIntPenalty);
    getIntClamped(&config, "ai", "friendly_dead_distance", 0, 100, gCombatSettings.friendlyDeadDistance);

    getIntClamped(&config, "ai", "perception_sight_mult", 0, 20, gCombatSettings.perceptionSightMult);
    getIntClamped(&config, "ai", "perception_combat_mult", 0, 20, gCombatSettings.perceptionCombatMult);
    getIntClamped(&config, "ai", "sneak_120_bonus", -100, 0, gCombatSettings.sneak120Bonus);
    getIntClamped(&config, "ai", "sneak_distance_div", 1, 20, gCombatSettings.sneakDistanceDiv);
    getIntClamped(&config, "ai", "sneaking_state_num", 0, 100, gCombatSettings.sneakingStateNum);
    getIntClamped(&config, "ai", "sneaking_state_den", 1, 100, gCombatSettings.sneakingStateDen);
    getIntClamped(&config, "ai", "trans_glass_div", 1, 20, gCombatSettings.transGlassDiv);

    // ---- [chem] ----
    getIntClamped(&config, "chem", "stims_hurt_little_hp", 0, 100, gCombatSettings.chemStimsHurtLittle);
    getIntClamped(&config, "chem", "stims_hurt_lots_hp", 0, 100, gCombatSettings.chemStimsHurtLots);
    getIntClamped(&config, "chem", "stims_hp", 0, 100, gCombatSettings.chemStimsHp);
    getIntClamped(&config, "chem", "sometimes_chance", 0, 100, gCombatSettings.chemSometimesChance);
    getIntClamped(&config, "chem", "anytime_chance", 0, 100, gCombatSettings.chemAnytimeChance);
    getIntClamped(&config, "chem", "always_chance", 0, 100, gCombatSettings.chemAlwaysChance);
    getIntClamped(&config, "chem", "random_drug_picks", 1, 32, gCombatSettings.chemRandomDrugPicks);
    getIntClamped(&config, "chem", "ap_cost", 0, 100, gCombatSettings.chemApCost);

    configFree(&config);
}

} // namespace fallout