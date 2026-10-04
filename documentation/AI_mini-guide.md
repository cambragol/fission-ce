---
layout: modpage
title: AI Mini-guide
include_in_header: false
include_in_footer: false
---

# FISSION Combat AI Mini‑Guide  
> Define Custom AI Packets (Dispositions, Combat Behaviour, Taunts)

This guide assumes you have a working mod folder (e.g., `mods/mod_mytown.dat/` from the Quick Start Guide). You’ll add new AI packets that control how critters behave in combat – their aggression, weapon preferences, movement, and taunts.

---

## 1. What You Need

Your existing mod folder: `mods/mod_mytown.dat/`. (a folder for development, compress into actual .dat for release)

You will create **two new files**:

- `data/ai_mytown.txt` – defines the AI packet(s) and their numeric parameters
- `text/english/game/combatai_mytown.msg` – provides the taunt messages (optional)

> **No `.lst` file is required** – the engine automatically picks up any `ai_*.txt` file.

---

## 2. Create the AI Packet Definition File

**File:** `mods/mod_mytown/data/ai_mytown.txt`

Each **section** (inside `[ ]`) defines one AI packet. The section name is the packet’s display name.

```
[MyGuard custom]
packet_num = -1
max_dist = 12
min_to_hit = 20
min_hp = 12
aggression = 80
hurt_too_much = blind,crippled_arms
secondary_freq = 4
called_freq = 5
font = 1
color = 992
outline_color = 0
chance = 50
run_start = 100
run_end = 109
move_start = 110
move_end = 119
attack_start = 120
attack_end = 129
miss_start = 130
miss_end = 139
hit_head_start = 140
hit_head_end = 149
hit_left_arm_start = 150
hit_left_arm_end = 159
hit_right_arm_start = 160
hit_right_arm_end = 169
hit_torso_start = 170
hit_torso_end = 179
hit_right_leg_start = 180
hit_right_leg_end = 189
hit_left_leg_start = 190
hit_left_leg_end = 199
hit_eyes_start = 200
hit_eyes_end = 209
hit_groin_start = 210
hit_groin_end = 219
area_attack_mode = be_sure
run_away_mode = bleeding
best_weapon = melee
distance = charge
attack_who = strongest
chem_use = stims_when_hurt_little
chem_primary_desire = 10, 11, 12
disposition = custom
body_type = human
general_type = guard
```

**Key fields explained:**

| Field | Meaning |
|-------|---------|
| `packet_num` | For mod packets, set to `-1` (auto‑assigned). |
| `disposition` | `none`, `custom`, `coward`, `defensive`, `aggressive`, `berserk`. Defines the base personality. |
| `run_away_mode` | `none`, `coward`, `finger_hurts`, `bleeding`, `not_feeling_good`, `tourniquet`, `never`. |
| `best_weapon` | `no_pref`, `melee`, `melee_over_ranged`, `ranged_over_melee`, `ranged`, `unarmed`, `unarmed_over_thrown`, `random`. |
| `distance` | `stay_close`, `charge`, `snipe`, `on_your_own`, `stay`. |
| `attack_who` | `whomever_attacking_me`, `strongest`, `weakest`, `whomever`, `closest`. |
| `chem_use` | `clean`, `stims_when_hurt_little`, `stims_when_hurt_lots`, `sometimes`, `anytime`, `always`. |

The `run_start` ... `hit_groin_end` fields define the **message ID ranges** for taunts (see next section).

---

## 3. Create the Taunt Message File (Optional)

**File:** `mods/mod_mytown/text/english/game/combatai_mytown.msg`

Each AI packet gets its own block of messages. The IDs are **relative** to the ranges in `ai_mytown.txt`.

```
{100}{}{"Get out of my sight!"}
{101}{}{"You'll pay for that!"}
...
{140}{}{"Nice try shithead!"}
...
```

- The numbers (100, 101, etc.) are **offsets**. The game adds a **base ID** (generated from your mod name + `combatai`) to these offsets.
- You do **not** need to calculate the final IDs – the engine does it for you.

If you omit the `.msg` file, the AI will still work, but no taunts will be displayed.

---

## 4. Disposition Variants (Important!)

The system can automatically generate **disposition variants** of your custom packet. Name your packet as:

```
[MyGuard custom]
...
disposition = custom
```

Then create additional sections with the same base name followed by the disposition keyword:

```
[MyGuard coward]
disposition = coward
...
[MyGuard defensive]
disposition = defensive
...
[MyGuard aggressive]
disposition = aggressive
...
[MyGuard berserk]
disposition = berserk
...
```

The engine will map these variants to the **base custom packet** ([MyGuard custom] in this case). When you change a critter’s disposition in‑game (via `aiSetDisposition`), the AI switches to the appropriate variant without needing to change the proto.

**If you don’t create variants**, the system falls back to simple sequential numbering (original Fallout 2 behaviour).

---

## 5. How to Use Your AI Packet

### In a Critter Prototype (`.pro` file) via `.lst` Override

Add an `ai=` override in your `critters_mytown.lst` file:

```
myguard.pro ai=4096
```

Where `4096` is the packet number from `data/lists/ai_list.txt`. This will set the AI packet for all instances of that critter.

### In a Script

```
set_critter_ai_packet(critter_obj, 4096);
```

### Changing Disposition (in Script)

// disposition: -1=none, 0=custom, 1=coward, 2=defensive, 3=aggressive, 4=berserk
```
ai_set_disposition(critter_obj, 3);
```

This will switch the critter to the aggressive variant of its base packet (if you created one with the same base name).

---

## 6. Find Your Packet Number
Run the game once (or reload if already running).

Open data/lists/ai_list.txt.

Look for your packet under “AI Packets Report”. The PacketNum column shows the final index.

Example:

```
Index  PacketNum  Name
0      4096       MyGuard custom
1      4097       MyGuard coward
...
```

Use that PacketNum in your prototypes or scripts.

---

## 7. Important Notes
Stable indices – The same mod name + section name always produce the same packet number. You can safely hardcode the number from the report.

Collisions – If two mods produce the same packet number, a warning appears and the second packet is skipped. Rename your section or mod to resolve.

Message IDs – The base ID for taunts is generated from your mod name + combatai. You never need to assign numbers manually.

Disposition mapping – The system looks for sections where the name ends with a disposition keyword (space + keyword). The keyword must match exactly the values in gDispositionKeys (lowercase).

Vanilla packets – You can also override existing vanilla packets by using the same packet_num (not recommended – better to create new ones).

---

## 8. Quick Troubleshooting

| Problem | Likely Fix |
|---------|-------------|
| Packet not in `ai_list.txt` | Filename must be `ai_<modname>.txt`. Check the `data/` folder. |
| Wrong packet number | Always use the `PacketNum` from the report – it is **not** the section order. |
| “Hash collision” popup | Another mod already uses that packet number. Change your mod name. |
| Taunts not showing | Message file missing or offsets don’t match the ranges in `ai_mytown.txt`. |
| Disposition switching doesn’t work | You must create variant sections with the correct disposition keywords. |
