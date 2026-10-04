---
layout: modpage
title: Mod
include_in_header: true
include_in_mod_header: true
include_in_footer: true
---

# Fallout FISSION Modding: Quick Start Guide  
> Get Your First Mod Running in About 10 Minutes

## 1. What is FISSION?

FISSION is a new engine for Fallout 2 that lets you add new content **without editing original game files**. You simply add your files inside a mod folder (or `.dat` archive) inside the `mods/` directory.

**Key Benefits:**
- No original file editing – just add new files
- Automatic, stable ID assignment – no manual number wrangling
- Built‑in testing reports – see exactly what the game loaded
- Works with vanilla saves – your mods are optional

## 2. Your First 'Location' Mod in 4 Steps

### Before You Start: Pick a Mod Name
Choose a short, unique name. Example: `mytown`  
This name goes in **all** your filenames. It will be used to generate unique, stable IDs for your mod’s assets.

### Step 1: Create Your Map
1. **Put FISSION** in your game directory and run it once – this creates the `mods/` folder.
2. **Open Mapper.exe** (original Fallout 2 location).
3. **Create a new map** or use a simple test map.
4. **Save it** with a name **8 characters or less** (e.g., `mytown1.map`).  
   ***Critical***: Map names longer than 8 characters will fail to load.
5. **Create a mod folder** inside `mods/` called `mod_mytown` (or `mod_mytown.dat` if you prefer a single archive).
6. **Export the `.map` file** to:  
   `mods/mod_mytown/data/MAPS/mytown1.map`  
   *(The `MAPS` folder must be uppercase.)*

> *Note:* If your map uses map‑level global variables (e.g., for quest flags), the Mapper will create a .GAM file with the same base name as your .map file. Place it alongside the .map file in data/MAPS/. This is not required for the basic example.

### Step 2: Create These 4 Essential Files

#### File 1: `mods/mod_mytown/data/city_mytown.txt`

*(Defines your new location on the world map)*
```
[Area 0]
area_name = MYTOWN ; UPPERCASE, globally unique name
world_pos = 400,300 ; Where it appears on world map (x,y)
start_state = on ; on, off
size = medium ; small, medium, large (lowercase)
entrance_0 = on,100,200,MYMAP,-1,-1,0
```

**`entrance_0` field format (comma‑separated, in order):**

1. **`state`** – `on` or `off` (whether this entrance is available). Example: `on`
2. **`x`** – X coordinate of the entrance button on the town map (in pixels). Example: `100`
3. **`y`** – Y coordinate of the entrance button on the town map (in pixels). Example: `200`
4. **`lookup_name`** – Must match a `lookup_name` in your `maps_<modname>.txt` file. Example: `MYMAP`
5. **`elevation`** – Which map elevation to use (`-1` = any, or `0`, `1`, `2`). Example: `-1`
6. **`tile`** – Starting tile on the map (`-1` = random valid start point). Example: `-1`
7. **`rotation`** – Facing direction when entering (`0` = north, `2` = east, `4` = south, `6` = west). Example: `0`

#### File 2: `mods/mod_mytown/data/maps_mytown.txt` 

*(Defines your actual game map)*
```
[Map 0]
lookup_name = MYMAP ; Matches entrance_0 above
map_name = mytown1 ; Must match .map filename *and* interal .map name
saved = yes ; yes or no (lowercase)
automap = yes ; yes or no (lowercase)
music = fs_grand ; Optional: music track name
```

**Connection:** `entrance_0` in `city_mytown.txt` references `lookup_name` in `maps_mytown.txt`.

#### File 3: `mods/mod_mytown/text/english/game/map_mytown.msg`

*(Map elevation names + area name)*
```
# Map elevation names (3 per map, sequential)
{0}{}{Town Gate}
{1}{}{Town Interior}
{2}{}{Town Basement}

# Area name (starts at 1500)
{1500}{}{My Town}
```

***Important***: Each map uses **three consecutive IDs** (elevations 0,1,2).  
If you add a second map, its elevations start at `{3}{}{...}`, `{4}{}{...}`, `{5}{}{...}`.

#### File 4: `mods/mod_mytown/text/english/game/worldmap_mytown.msg`

*(Town map entrance labels – one per entrance)*
```
{0}{}{My Town - Main Gate}
```

Each `entrance_X` gets one label, in the same order as defined in `city_mytown.txt`.

---

## 3. Where to Put All Files

Your mod folder (or `.dat` archive) must mirror the game’s original structure:

```
Fallout 2/
└─ mods/
    └─ mod_mytown.dat/ <- can be a folder or actual .dat file
        ├─ data/
        │   ├─ city_mytown.txt
        │   ├─ maps_mytown.txt
        │   └─ MAPS/
        │       └─ mytown1.map
        └─ text/
            └─ english/
                └─ game/
                    ├─ map_mytown.msg
                    └─ worldmap_mytown.msg
```

**Notes:**
- The `mods/` folder is created automatically the first time you run FISSION.
- Your mod folder **must** start with `mod_` (e.g., `mod_mytown`).
- It can be a normal folder (for easy editing) or a `.dat` archive (for distribution).

---

## 3.5 (Optional) Add Mod Metadata & an Icon

FISSION can display your mod in an in‑game **Loaded Mods** mod list (accessible from the main menu FISSION icon). To give your mod a name, description, author credit, and an icon, create a simple configuration file.

### Step A: Create `mods/mod_mytown/data/mod_mytown.cfg`

```
[mod_info]
display_name = The Quick Start Town
name = mytown
description = Adds a new settlement called My Town with custom quests.
author = YourName
dependencies = anothermod, thirdmod
```

- **`display_name`** - User friendly name of your mod. Used only in the Loaded Mods window.
- **`name`** – Internal name of your mod. **Must be the same as your mod folder name without the `mod_` prefix** (e.g., `mytown`). No spaces or special characters. This is used to generate a stable ID for the mod icon.
- **`description`** – Short description (shown when the mod is selected in the list).
- **`author`** – Your name or alias.
- **`dependencies`** – (Optional) Comma‑separated list of other mod **internal names** that your mod works with or requires. This is **informational only** – it tells players which other mods they should install. The engine does not enforce this list.

> **How to actually check for another mod in your scripts:**  
> Every mod that provides an icon also gets a **stable art index** (the same index used for the icon). This index is written to `data/lists/mod_ids_list.txt` when the game runs.  
> To check if another mod (e.g., `anothermod`) is installed, use its icon’s art index in your script:
> 
> ```
> #define OBJ_TYPE_INVEN 7
>
> procedure start begin
>    if (sfall_func2("art_exists_by_index", OBJ_TYPE_INVEN, 5628)) then
>        display_msg("Art found!");
>    else
>        display_msg("Art not found.");
>    end
> end
> ```
> Replace `5628` with the actual ID from `mod_ids_list.txt`. This allows you to conditionally enable cross‑mod features.

### Step B: Add the Icon to the Interface Art List

Create an art list file for the interface category:

**File:** `mods/mod_mytown/art/intrface/intrface_mytown.lst`

```
mytown.frm
```

Each line is the filename of an interface `.frm` file (one per line). The name must match the `name` field from your `.cfg` file.

### Step C: Place the Icon File

Put your custom icon `.frm` file in the same folder as the `.lst` file:

```
mods/mod_mytown/art/intrface/mytown.frm
```

- Recommended size: **140×116** pixels (same size as perk/trait/karma fallout boy images).
- The icon appears next to your mod in the mod list.
- **This icon’s art index is what other mods will use to detect your mod’s presence.**

If you omit the icon, the mod list will show a blank space, and other mods cannot reliably detect your mod via `art_exists_by_index`.

**That’s it!** Your mod will now appear with proper metadata and an icon in the mod list, making it easier for players to identify and manage – and for other mods to conditionally integrate with yours.

---

## 4. Test Your Mod

### Test 1: Check the Reports

Run the game once, then look in `data/lists/` (inside the game’s root folder – **not** inside your mod):

- **`area_list.txt`** – Your area “MYTOWN” should be listed.
- **`maps_list.txt`** – Your map “MYMAP” should be listed.

If both appear, your mod loaded successfully!

### Test 2: Play Your Mod

1. Start a new game or load a save.
2. Enter the world map.
3. Travel to coordinates **400,300**.
4. You should see a circle labelled “My Town”.
5. Click the circle to enter – you’ll load into `mytown1.map`.
6. The location name should show “Town Gate” (elevation 0).

If you can walk around your map, your mod is working!

---

## 4.5 Example Mod (Download)

If you want to skip manual file creation and see a working example, download the pre‑built template:

**Download** <a class="pill" href="https://cambragol.github.io/fallout-fission/assets/mod_mytown.dat.zip">mod_mytown.dat.zip</a>

Inside the zip you’ll find a folder `mod_mytown.dat/` containing the complete file structure from this guide:

- `data/city_mytown.txt`
- `data/maps_mytown.txt`
- `data/MAPS/mytown1.map`
- `text/english/game/map_mytown.msg`
- `text/english/game/worldmap_mytown.msg`

**How to use it:**  
1. Extract the zip file into your `mods/` folder (so you have `mods/mod_mytown.dat/`).  
2. Run the game – the new location “My Town” will appear on the world map at coordinates 400,300.  

You can edit the files inside to learn how each part works or use them as a starting point for your own mod.

---

## 5. Common Mistakes & Troubleshooting

| Problem | Likely Fix |
|---------|-------------|
| “Cannot load map” error | `map_name` in `maps_mytown.txt` must exactly match the `.map` filename (no extension, ≤8 chars). |
| Area not on world map | Check `area_list.txt`. If missing, your `city_mytown.txt` has a syntax error. |
| Can’t enter area | Verify `entrance_0` references the correct `lookup_name`. |
| Map name shows “Error!” | Your `map_mytown.msg` elevation offsets are wrong (must be 0,1,2 for first map). |
| No reports generated | The game didn’t load your mod – check that your mod folder is named `mod_xxx` and is inside `mods/`. |

**Remember:** All text values are case‑insensitive, but it’s safest to use **lowercase** for `on`/`off`, `yes`/`no`, `small`/`medium`/`large`.

---

## 6. What’s Next?

### Add a Second Map to the Same Area

Add another entrance in `city_mytown.txt`:

```
entrance_1 = on,200,200,MYMAP2,-1,-1,0
```

Add a new map definition in `maps_mytown.txt`:

```
[Map 1]
lookup_name = MYMAP2
map_name = mytown2
saved = yes
automap = yes
```

Extend `map_mytown.msg` with the three elevation names for the second map (offsets 3,4,5):

```
{3}{}{Underground Entrance}
{4}{}{Underground Tunnels}
{5}{}{Hidden Vault}
```

---

## 7. Final Notes

- **Mod name stability:** If you change your mod’s name after starting a game, saved games will no longer recognise your mod’s content. Choose a name once and keep it. Hashed IDs will also be changed, breaking some of your content.
- **`lookup_name` uniqueness:** Avoid using common names like “VAULT13”. Use a prefix like `MYTOWN_MAP1` to prevent accidental overrides with other mods.
- **No manual ID assignment:** FISSION calculates and assigns stable IDs automatically from your mod’s name and the order of items in your files.

---

## 8. What's Next?

Once your basic location mod is working, expand it with these additional features:

| Topic | Guide | Description |
|-------|-------|-------------|
| **Quests** | [Quest Mini‑Guide](https://cambragol.github.io/fallout-fission/quest_mini-guide) | Add Pip‑Boy tracked quests to your location |
| **Holodisks** | [Holodisk Mini‑Guide](https://cambragol.github.io/fallout-fission/holodisk_mini-guide) | Add readable text entries to the Pip‑Boy |
| **Items** | [Item Mini‑Guide](https://cambragol.github.io/fallout-fission/item_mini-guide) | Create new weapons, armor, or misc items |
| **Critters** | [NPC Mini‑Guide](https://cambragol.github.io/fallout-fission/npc_mini-guide) | Design custom NPCs with unique stats and looks |
| **Combat AI** | [AI Mini‑Guide](https://cambragol.github.io/fallout-fission/ai_mini-guide) | Define aggression, weapon preferences, and taunts |
| **Scripts** | [Script Mini‑Guide](https://cambragol.github.io/fallout-fission/script_mini-guide) | Add custom behavior with `.int` scripts |
| **GVARs** | [GVAR Mini‑Guide](https://cambragol.github.io/fallout-fission/gvar_mini-guide) | Manage persistent flags and quest states |
| **Art & FRMs** | [Art Mini‑Guide](https://cambragol.github.io/fallout-fission/art_mini-guide) | Import custom graphics (items, critters, interface) |
| **Death Endings** | [Death Endings Mini‑Guide](https://cambragol.github.io/fallout-fission/end_death_mini-guide) | Add custom narrations when the player dies |
| **Endgame Slides** | [Endgame Slides Mini‑Guide](https://cambragol.github.io/fallout-fission/end_game_mini-guide) | Create custom ending slideshows |
