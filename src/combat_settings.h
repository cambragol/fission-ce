// combat_settings.h
#pragma once

namespace fallout {

// Array sizes are fixed by the game and must match their source counterparts:
//   hitLocationPenalty[9]       -> HIT_LOCATION_COUNT
//   lightThreshold[3]           -> three light bands
//   lightPenalty[3]             -> three light bands
//   critEffectThreshold[5]      -> five critical-hit effects
//   critFailureThreshold[4]     -> four critical-failure effects
//   runAwayHp[6]                -> six run-away modes (excludes "never")
//   partyMemberDistances[5]     -> DISTANCE_COUNT
struct CombatSettings {
    // [combat]
    int hitLocationPenalty[9];
    int difficultyToHitEasy;
    int difficultyToHitHard;
    int difficultyDamageEasy;
    int difficultyDamageHard;
    int rangedPerceptionLongRange;
    int rangedPerceptionScope;
    int rangedPerceptionDefault;
    int scopeMinEffective;
    int rangedDistanceBlindMult;
    int rangedDistanceNormalMult;
    int lofCritterPenalty;
    int oneHanderTwoHandedPenalty;
    int oneHanderOneHandedBonus;
    int minStrengthPenalty;
    int weaponHandlingBonus;
    int accurateBonus;
    int multihexToHitBonus;
    int lightThreshold[3];
    int lightPenalty[3];
    int blindToHitPenalty;
    int knockedToHitBonus;
    int toHitCap;
    int bypassThresholdPercent;
    int bypassDrPercent;
    int penetrateThresholdPercent;
    int finesseDrBonus;
    int bonusRangedDamagePerRank;
    int damageDivisor;
    int livingAnatomyBonus;
    int pyromaniacBonus;
    int knockbackDivisorDefault;
    int knockbackDivisorPerk;
    int stonewallChance;
    int stonewallDivisor;
    int rangedMissAccuracyDivisor;
    int rangedMissDeadChance;
    int safetyIntThreshold;
    int safetyRadiusReduction;

    // [criticals]
    int critEffectThreshold[5];
    int critFailureThreshold[4];
    int critFailureLuckPenalty;
    int playerGraceDays;
    int critFailureSelfDamageMin;
    int critFailureSelfDamageMax;

    // [ai]
    int runAwayHp[6];
    int partyMemberDistances[5];
    int maxTeammateDistance;
    int aiSearchPerceptionBonus;
    int aiBestWeaponDamageDiff;
    int aiReloadAttempts;
    int calledShotIntEasy;
    int calledShotIntNormal;
    int calledShotIntHard;
    int areaAttackToHitSure;
    int areaAttackToHitCareful;
    int areaAttackToHitAbsolutelySure;
    int secondaryModeInt;
    int secondaryModeDistance;
    int minMoveDistance;
    int runApThresholdDiv;
    int friendlyFireIntPenalty;
    int friendlyDeadDistance;
    int perceptionSightMult;
    int perceptionCombatMult;
    int sneak120Bonus;
    int sneakDistanceDiv;
    int sneakingStateNum;
    int sneakingStateDen;
    int transGlassDiv;

    // [chem]
    int chemStimsHurtLittle;
    int chemStimsHurtLots;
    int chemStimsHp;
    int chemSometimesChance;
    int chemAnytimeChance;
    int chemAlwaysChance;
    int chemRandomDrugPicks;
    int chemApCost;
};

extern CombatSettings gCombatSettings;

void combatSettingsInit();

} // namespace fallout