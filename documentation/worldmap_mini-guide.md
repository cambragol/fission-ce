---
layout: modpage
title: Worldmap Mini-guide
include_in_header: false
include_in_footer: false
---

# FISSION Worldmap Modding Mini-Guide  
> Add Custom Encounter Tables, Named Encounters, and Tile Overrides

This guide assumes you have a working mod folder (e.g., `mods/mod_mytown.dat/` from the Quick Start Guide). You'll extend the worldmap with new random encounters, custom critter groups, and even override specific map tiles.

---

## 1. What You Need

Your existing mod folder: `mods/mod_mytown.dat/`. (a folder for development, compress into actual .dat for release)

You will create **one or more** of these files:

- `data/worldmap_mytown.txt` – defines encounter tables, named encounters, and tile overrides
- `text/english/game/worldmap_mytown.msg` – provides custom encounter description text

---

## 2. Create the Worldmap Definition File

**File:** `mods/mod_mytown/data/worldmap_mytown.txt`

This file can contain three types of sections:

### A. Encounter Tables

```
[Encounter Table 0]
lookup_name=My_Desert_Table
maps=Desert Encounter 4, Desert Encounter 5
enc_00=chance:10%,Map:MyMap,Enc:player,MyRaiders
enc_01=chance:20%,Map:Desert Encounter 1,Enc:MyRaiders
```

**Fields explained:**

- **`lookup_name`** – Unique name for this table. Used by subtiles to reference it.
- **`maps`** – Comma‑separated list of map lookup names to use for encounters (from `maps.txt` or `maps_mytown.txt`).
- **`enc_XX`** – Encounter entry definitions (XX = 00–40):
  - `chance` – Percentage chance (e.g., `5%`).
  - `Map` – Which map to load (must match a `lookup_name` from `maps.txt`).
  - `Enc` – Named encounter(s) to spawn. Can list multiple, separated by commas.
  - `counter` – (optional) Max times this encounter can trigger (`-1` = unlimited).
  - `special` – (optional) Marks this as a special encounter (creates a map marker).

### B. Named Encounters

```
[Encounter: MyRaiders]
type_00=ratio:100%, pid:16777317, Script:764, Item:(2-6)365
position=Wedge, Spacing:2, distance:15

[Encounter: MyBoss]
type_00=ratio:50%, pid:16777221
type_01=ratio:50%, pid:16777222, distance:5
```

**Fields explained:**

- **`type_XX`** – Each line defines a critter or group:
  - `ratio` – Percentage of total critters this type represents.
  - `pid` – Prototype ID (full ID for critters, short ID for items).
  - `Script` – Script index (1‑based, from `scripts.lst` or `scripts_mytown.lst`).
  - `Item` – Item to equip or add, with optional quantity range.
  - `distance` – Spawn distance from player.
  - `tile` – Specific tile number (override).
- **`position`** – Formation type: `Surrounding`, `Straight_Line`, `Double_Line`, `Wedge`, `Cone`, `Huddle`.
- **`spacing`** – Distance between units in the formation.
- **`team_num`** – Team ID for combat (default = -1 = hostile to player).

### C. Tile Overrides

```
[Tile 5]
art_idx=350
walk_mask_name=mynewmask
encounter_difficulty=-10
3_3=Desert,No_Fill,Common,Common,Common,My_Desert_Table
6_5=Mountain,Fill_N,Rare,Rare,Rare,My_Mountain_Table
```

**Fields explained:**

- **`art_idx`** – Override the tile's appearance (FRM index from `art/intrface/`).
- **`walk_mask_name`** – Override the walkability mask (from `data/*.msk`).
- **`encounter_difficulty`** – Modifier to player's Outdoorsman skill for detecting encounters.
- **`X_Y`** – Subtile coordinates (X = 0..6, Y = 0..5). Value format:
  - `terrain` – `Desert`, `Mountain`, `Ocean`, `Forest`, etc. (from `worldmap.txt`).
  - `fill` – `No_Fill`, `Fill_N`, `Fill_S`, `Fill_E`, `Fill_W`, `Fill_NW`, `Fill_NE`, `Fill_SW`, `Fill_SE`.
  - `morning_chance` – Encounter frequency: `None`, `Rare`, `Uncommon`, `Common`, `Frequent`, `Forced`.
  - `afternoon_chance` – Same as above.
  - `night_chance` – Same as above.
  - `type` – The `lookup_name` of the encounter table to use.

> **Note:** Tile overrides are **partial** – you only need to specify the subtiles you want to change. All other subtiles keep their original values.

---

## 3. Create the Message File (Optional)

**File:** `mods/mod_mytown/text/english/game/worldmap_mytown.msg`

For custom encounter descriptions, add entries starting at **local number 100**:

```
{100}{}{You spot a group of raiders looting a caravan.}
{101}{}{A massive radscorpion emerges from the sand!}
{102}{}{You find a wounded traveller in need of help.}
```

**Rules:**

- Local numbers 0–99 are reserved for **entrance labels** (if your mod adds towns).
- Local numbers 100–499 are for **encounter entry texts**.
- Each table gets **50 consecutive IDs**: Table 0 uses 100–149, Table 1 uses 150–199, etc.
- The first entry (`enc_00`) of Table 0 uses local number 100, `enc_01` uses 101, etc.

> **Why 100?** To avoid overlap with entrance labels (0–99). If your mod doesn't add entrances, you can start at 0 instead, but we recommend the standard offset.

---

## 4. Where to Put Everything

```
mods/mod_mytown/
    ├─ data/
    │   ├─ worldmap_mytown.txt
    │   └─ ... (other files)
    └─ text/
        └─ english/
            └─ game/
                ├─ worldmap_mytown.msg
                └─ ... (other message files)
```

---

## 5. Test Your Worldmap Content

1. Run the game and load your save (or start a new game).
2. Check the debug reports in `data/lists/`:
   - `encounter_tables_list.txt` – shows all tables (vanilla + mod) with their stable slots.
   - `named_encounters_list.txt` – shows all named encounters (vanilla + mod).
3. Travel to the tile/subtile you overrode and trigger encounters. Your custom table should appear.

**If your encounter doesn't trigger:**

- Verify the subtile's `type` field matches your table's `lookup_name`.
- Ensure the table has at least one `enc_XX` entry with a valid `Map`.
- Check that `Map` references an existing map lookup name (from `maps.txt` or `maps_mytown.txt`).

---

## 6. Important Notes

- **Stable IDs** – All encounter tables and named encounters get **deterministic hashed slots** based on your mod name and the table/encounter name. This means they won't change between game sessions or conflict with other mods.

- **Partial Tile Overrides** – You only need to specify subtiles you want to change. All other subtiles on that tile remain original.

- **Message Blocks** – The base ID for your mod's worldmap messages is computed automatically from your mod name. You only provide local numbers in the `.msg` file.

- **Collision Detection** – If two mods accidentally generate the same hash slot, a warning message box appears and the conflicting table/encounter is skipped. Rename your mod or the table/encounter to resolve it.

- **Vanilla Encounter Tables** – You can override existing tables by using the same `lookup_name` as a vanilla table (e.g., `Desert`). Your table will replace it globally.

---

## 7. Quick Troubleshooting

| Problem | Likely Fix |
|---------|-------------|
| Table not in `encounter_tables_list.txt` | Check `worldmap_mytown.txt` syntax. Missing `lookup_name` or malformed `enc_XX`. |
| Named encounter not in `named_encounters_list.txt` | Section name must be `[Encounter: Name]`. Check spelling and brackets. |
| Encounter doesn't trigger | Subtile's `type` doesn't match your table's `lookup_name`. Or the table has no valid `Map`. |
| Message shows "Error!" | Missing `worldmap_mytown.msg` file, or local number is outside 100–499 range. |
| Tile override not applied | Check tile index is valid (0–19). Or `art_idx` points to a non‑existent FRM. |
| "Hash collision" popup | Another mod uses the same slot. Change your mod name or table/encounter name. |

---

## 8. What's Next?

Once your worldmap content works, combine it with other modding features:

- Add **custom maps** via `maps_mytown.txt` to use as encounter locations.
- Add **custom critters** via `critters_mytown.lst` to populate your encounters.
- Add **custom areas** via `city_mytown.txt` to place towns or special locations on the map.

The full FISSION modding suite gives you complete control over the worldmap ecosystem!