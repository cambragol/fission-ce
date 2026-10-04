---
layout: modpage
title: Quest Mini-guide
include_in_header: false
include_in_footer: false
---

# FISSION Quest Modding Mini-Guide  
> Add a Quest to Your Existing Location Mod

This guide assumes you have a working mod folder (e.g., `mods/mod_mytown.dat/` from the Quick Start Guide). You'll add a simple quest that appears in the Pip‑Boy.

---

## 1. What You Need

Your existing mod folder: `mods/mod_mytown.dat/`. (a folder for development, compress into actual .dat for release)

You will create **two new files**:

- `data/quests_mytown.txt` – defines quest metadata
- `text/english/game/quests_mytown.msg` – provides quest description text

---

## 2. Create the Quest Definition File

**File:** `mods/mod_mytown/data/quests_mytown.txt`

Format: location, description, gvar, displayThreshold, completedThreshold
```
1500, 0, 79, 1, 2
```

**Fields explained:**

- **`location`** (e.g., `1500`) – The **area index** of the location this quest belongs to. Find this number in `data/lists/area_list.txt` under the "Slot" column for your area.
- **`description`** (e.g., `0`) – **Ignored** – FISSION auto‑generates the description ID from the `.msg` file.
- **`gvar`** (e.g., `79`) – Global variable used to track quest state (choose any unused GVAR).
- **`displayThreshold`** (e.g., `1`) – Minimum quest state to show in Pip‑Boy (`1` = unstarted/available).
- **`completedThreshold`** (e.g., `2`) – Quest state at which it’s considered completed (`2` = finished).

> **Tip:** You can find your area’s `location` ID in `data/lists/area_list.txt` after running the game once.

---

## 3. Create the Quest Message File

**File:** `mods/mod_mytown/text/english/game/quests_mytown.msg`

```
{0}{}{Find the Lost Artifact}
```

- The `{0}` is the **description ID** – FISSION automatically links this to your quest definition.
- The text will appear in the Pip‑Boy quest list.

If you want multiple quests, add more entries:

```
{0}{}{Find the Lost Artifact}
{1}{}{Rescue the Trader}
```

Each quest in `quests_mytown.txt` gets one description, in the same order.

---

## 4. Update Your Mod Folder Structure

Add the two new files:

```
mods/mod_mytown/
├─ data/
│   ├─ city_mytown.txt (existing)
│   ├─ maps_mytown.txt (existing)
│   ├─ quests_mytown.txt <- NEW
│   └─ MAPS/
│       └─ mytown1.map (existing)
└─ text/
    └─ english/
        └─ game/
            ├─ map_mytown.msg (existing)
            ├─ worldmap_mytown.msg (existing)
            └─ quests_mytown.msg <- NEW
```

---

## 5. Test Your Quest

1. Run the game and load your save (or start a new game).
2. Open the Pip‑Boy (`P` key) and go to the **Quests** tab.
3. You should see “Find the Lost Artifact” (or your custom text).

If it doesn’t appear, check:
- `data/lists/quests_list.txt` – your quest should be listed there.
- Global variable `79` – you can set it to `1` via script or debug to mark the quest as active.

---

## 6. Controlling Quest State from Scripts

Use the following script functions (in your map or global scripts):

| Action | Script Code | Global Var Value |
|--------|-------------|------------------|
| Start quest | `set_global_var(GVAR, 1)` | 1 |
| Complete quest | `set_global_var(GVAR, 2)` | 2 |
| Fail quest | `set_global_var(GVAR, 3)` | 3 |
| Check if active | `get_global_var(GVAR) == 1` | – |
| Check if complete | `get_global_var(GVAR) == 2` | – |

Where `GVAR` is the number you chose in the `gvar` field of `quests_mytown.txt`.

**Example script snippet (standard Fallout 2 script syntax):**

```
// Start the quest
set_global_var(79, 1);

// Complete the quest
set_global_var(79, 2);

// Fail the quest
set_global_var(79, 3);

// Check quest state
if (get_global_var(79) == 2) then
    // Quest is complete
endif
```

**Finding `quest_id`:**  
After running the game once, check `data/lists/quests_list.txt`. Each quest has a generated ID (e.g., `5000`). Use that number in your scripts.

Example script snippet:

```
// Start the quest when player enters a certain area
set_global_var(5000, 1);
```

---

## 7. Notes
Quest descriptions are not editable per quest state – they remain the same whether the quest is active or completed.

You can reuse the same global variable across multiple quests if you manage states carefully, but it’s safer to use a unique GVAR for each quest.

The location field is only for grouping quests under a specific area in some tools; it doesn’t affect gameplay.

Now your location mod has a working quest! Combine with scripts to give the player objectives and rewards.
