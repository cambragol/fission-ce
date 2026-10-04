---
layout: modpage
title: GVAR Mini-guide
include_in_header: false
include_in_footer: false
---

# FISSION GVAR Mini-Guide  
> Add Persistent Global Variables for Quest States, Flags & More

This guide assumes you have a working mod folder (e.g., `mods/mod_mytown.dat/` from the Quick Start Guide). You’ll define custom global variables (GVARs) that are saved with the game and can be used in scripts.

---

## 1. What Are GVARs?

Global variables are integer values stored in the save file. They are used for:
- Quest progress (active, completed, failed)
- Town reputation
- Permanent flags (e.g., “already talked to NPC”)
- Any other persistent data

**Vanilla GVARs** are defined in `data/vault13.gam` (indices 0...N‑1).  
**Mod GVARs** are defined in `data/gvar_<modname>.txt` and receive **stable indices** in the range `4096` ... `8191`.

---

## 2. Create Your Mod GVAR File

**File:** `mods/mod_mytown/data/gvar_mytown.txt`

Format: SYMBOL = value (or SYMBOL := value)
```
MY_QUEST_STATE = 0
TOWN_REP_MYTOWN = 0
HAS_MET_BOSS = 0
```

- Each line defines one GVAR.
- Symbols can contain letters, digits, underscores (case‑sensitive).
- Values are integers (default when the game starts).
- Comments start with `#` or `//`.

---

## 3. Where to Put the File

```
mods/mod_mytown.dat/
        └─ data/
            └─ gvar_mytown.txt
```

**No other files are required.** The game will read all `gvar_*.txt` files from the `data` folder (including inside your mod folder/dat).

---

## 4. How It Works (Behind the Scenes)

- The mod name is extracted from the filename (`mytown` from `gvar_mytown.txt`).
- For each GVAR, the engine computes a **stable hash** of `modname:symbol`.
- The hash is mapped to an index in the mod range (`4096` ... `8191`).
- If two different mods produce the same index, a popup appears and the **second GVAR is skipped**.
- The game generates a report: `data/lists/gvars_list.txt` showing every GVAR and its final index.

---

## 5. Find Your GVAR Index

1. Run the game once (or reload if already running).
2. Open `data/lists/gvars_list.txt`.
3. Look under “MOD GLOBAL VARIABLES”:

```
Index Mod Symbol Default
4096 mytown MY_QUEST_STATE 0
57345 mytown TOWN_REP_MYTOWN 0
57346 mytown HAS_MET_BOSS 0
```

Use these numbers in your scripts.

---

## 6. Use GVARs in Scripts

In your `.int` script (standard Fallout 2 script syntax):

```
// Set a GVAR
set_global_var(4096, 1);

// Check a GVAR
if (get_global_var(4096) == 2) then
    // Quest completed
endif
```

---

## 7. Important Notes
Stable indices – The same mod name + symbol always produce the same index. You can hardcode the index from the report.

Collisions – If a collision occurs, a warning popup appears and the conflicting GVAR is not loaded. Rename your symbol or mod to resolve.

Default values – When a new game starts, mod GVARs are initialised to the values in your gvar_*.txt.

Save compatibility – Mod GVARs are saved separately; removing a mod later does not corrupt the save (the values are ignored).

Vanilla GVARs – You cannot override vanilla GVAR indices; they remain in the lower range.

---

## 8. Quick Troubleshooting

| Problem | Likely Fix |
|---------|-------------|
| GVAR not in `gvars_list.txt` | Filename must be `gvar_<modname>.txt`. Check the `data/` folder. |
| Wrong index used | Always use the index from the report – it is **not** the same as the symbol order. |
| “Hash collision” popup | Another mod already uses that index. Change your symbol or mod name. |
| GVAR value resets every load | Make sure you set the GVAR with `set_global_var()` and save the game. |
| Script uses wrong value | Verify the index matches the report. Use `debug_msg` to print the value. |

Now you can add persistent data to your mod – perfect for quests, reputation, and any other flags!

