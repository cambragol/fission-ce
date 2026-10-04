---
layout: modpage
title: Global Scripts Mini-guide
include_in_header: false
include_in_footer: false
---

# FISSION Global Scripts Mini‑Guide  
> Add Persistent, Map‑Independent Behaviour (Spawn NPCs, Global Hotkeys, Worldmap Events)

This guide assumes you have a working mod folder (e.g., `mods/mod_mytown.dat/` from the Quick Start Guide). You’ll add **global scripts** that run automatically, independent of any specific map or object.

---

## 1. What Are Global Scripts?

Global scripts are special `.int` files named `gl_*.int` (e.g., `gl_mytown.int`) placed in the `scripts/` folder. They are **loaded once** when the game starts and can execute code:

- On every **map update** (every 10 seconds) and map enter/exit.
- On every **input tick** (keyboard/mouse events).
- On **worldmap updates**.
- At a **custom repeat rate** (e.g., every 60 frames).

They are perfect for **modifying existing content** without editing original maps or scripts.

---

## 2. What You Need

Your existing mod folder: `mods/mod_mytown.dat/`. (a folder for development, compress into actual .dat for release)

You will create **one new file**:

- `scripts/gl_mytown.int` – the compiled global script (source `.ssl` file).

No `.lst` file is required – any file starting with `gl` in the `scripts/` folder is automatically loaded.

---

## 3. Basic Global Script Structure

Create a file `gl_mytown.ssl` (source) and compile it with `ScriptEditor.exe`.

```
// gl_mymod.ssl
#include "..\headers\define.h"

procedure start;
procedure map_enter_p_proc;
procedure map_update_p_proc;
procedure combat_is_over_p_proc;

procedure start begin
    // Set the script to run on map updates (mode 0) and repeat every 60 frames (~1 second)
    set_global_script_type(self_obj, 0);
    set_global_script_repeat(self_obj, 60);
end

procedure map_enter_p_proc begin
    // Called every time a map is entered
    if (cur_map_index == MAP_MODOC) then begin
        // Do something when entering Modoc
    end
end

procedure map_update_p_proc begin
    // Called every 10 seconds (map update) and also at the repeat rate if set
end

procedure combat_is_over_p_proc begin
    // Called when combat ends (if script type includes this)
end
```

---

## 4. Understanding Global Script Modes

Use `set_global_script_type(program, mode)` inside `start` to choose when your script runs:

| Mode | Behaviour |
|------|-----------|
| `0` | Runs on **map updates** (every 10 seconds) and map enter/exit. |
| `1` | Runs on **every input tick** (keyboard/mouse events). |
| `2` | Runs on **worldmap updates** (while travelling). |
| `3` | Runs on **both map updates and worldmap updates**. |

**Example:** `set_global_script_type(self_obj, 1);` – your script will run every time the player presses a key or moves the mouse.

---

## 5. Setting a Repeat Rate

Use set_global_script_repeat(program, frames) to make your script run periodically (in addition to normal updates). The script’s map_update_p_proc (or the main loop) will be called every frames frames (approx. 60 frames = 1 second).

```
set_global_script_repeat(self_obj, 120);   // every 2 seconds
```

If you don’t set a repeat, the script only runs on the standard events (map enter/exit, map update, etc.).

---

## 6. Practical Example: Spawning an NPC in an Existing Map

This script adds a custom NPC to the Villa Cave (map index `MAP_VILLA_CAVE`) only once, using a global variable to prevent multiple spawns.

```
// gl_mycompanion.ssl
#include "..\headers\define.h"

procedure start;
procedure map_enter_p_proc;
procedure spawn_npc;

procedure start begin
    // Run on map updates (mode 0) – we only need map_enter_p_proc
    set_global_script_type(self_obj, 0);
end

procedure map_enter_p_proc begin
    if not is_loading_game then begin
        if (cur_map_index == MAP_VILLA_CAVE) then begin
            // Check global variable: 0 = not spawned yet
            if (get_global_var(79) == 0) then begin
                call spawn_npc;
            end
        end
    end
end

procedure spawn_npc begin
    variable critter;
    // Create the NPC at tile 12496, elevation 0, with custom script
    critter := create_object_sid(PID_MY_COMPANION, 12496, 0, SCRIPT_MyCompanion);
    // Set rotation (optional)
    anim(critter, ANIMATE_ROTATION, 3);
    // Set team (optional)
    critter_add_trait(critter, TRAIT_OBJECT, OBJECT_TEAM_NUM, TEAM_PLAYER);
    // Override AI packet (optional)
    critter_add_trait(critter, TRAIT_OBJECT, OBJECT_AI_PACKET, AI_MY_COMPANION);
    // Change appearance (if your proto uses a different FID)
    art_change_fid_num(critter, FID_MY_COMPANION);
    // Give some items
    item_add(critter, PID_W_KNIFE, 1);
    item_caps_adjust(critter, random(0, 4));

    // Set global to 1 so she doesn't spawn again
    set_global_var(79, 1);
end
```

Explanation:

* The script checks cur_map_index against the target map.

* A global variable (79) ensures the NPC is spawned only once per save game.

* is_loading_game prevents spawning during a save‑game load.

---

## 7. Compiling and Placing the Script

1. Save your `.ssl` source file.
2. Compile with `ssc.exe` to produce a `.int` file.
3. Name the `.int` file **starting with `gl`** (e.g., `gl_mycompanion.int`).
4. Place it in your mod’s `scripts/` folder:

```
mods/mod_mytown.dat/
        └─ scripts/
            └─ gl_mycompanion.int
```

No `.lst` entry is needed – the game automatically loads all `gl*.int` files.

---

## 8. Testing Your Global Script

- Run the game and load a save (or start a new game).
- Enter the target map (e.g., Villa Cave).
- The NPC should appear.
- Save, reload, and re‑enter – the NPC should **not** spawn again (thanks to the GVAR check).

If the script doesn’t run, check:
- The filename starts with `gl`.
- The script compiled without errors.
- Your global variable is set to 0 before testing.

---

## 9. Important Notes

- **Performance** – Keep global scripts lightweight. Avoid heavy loops or frequent object scans.
- **Duplicate prevention** – Always use a GVAR or object existence check to avoid spawning multiple copies.
- **Map index values** – Find map indices in `data/lists/maps_list.txt` (slot number).
- **Global script functions** – The functions `set_global_script_type` and `set_global_script_repeat` are provided by **Sfall** and are available in the standard Sfall headers (`sfall.h`). Include them if needed.
- **No automatic unloading** – Global scripts persist for the whole game session. Use GVARs to control state across saves.

---

## 10. Quick Troubleshooting

| Problem | Likely Fix |
|---------|-------------|
| Script not running | Filename must start with `gl_`. Check the `scripts/` folder. |
| NPC spawns every time | Your GVAR check is wrong, or you didn’t set the GVAR after spawning. |
| “set_global_script_type” unknown | Include `#include "..\sfall_headers\sfall.h"` or equivalent in your `.ssl`. |
| Script runs on wrong map | Verify `cur_map_index` matches the map slot from `maps_list.txt`. |
| Game crashes on map enter | Infinite loop or heavy processing. Add `is_loading_game` check and keep code simple. |