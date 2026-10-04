---
layout: modpage
title: Script Mini-guide
include_in_header: false
include_in_footer: false
---

# FISSION Script Mini-Guide  
> Add Custom Behavior with `.int` Scripts

This guide assumes you have a working mod folder (e.g., `mods/mod_mytown.dat/` from the Quick Start Guide). You’ll add a script that can be attached to a critter, item, or map.

---

## 1. What You Need

Your existing mod folder: `mods/mod_mytown.dat/`.

You will create **one new file** (plus the compiled script):

- `scripts/scripts_mytown.lst` – lists your script name(s) and optional local variable count
- Your compiled `.int` script file (e.g., `myscript.int`) – created with the **ScriptEditor.exe** (SSC)

> **No `.msg` file is required** – scripts use dialog messages separately (see Dialog guide).

---

## 2. Create the Script List File

**File:** `mods/mod_mytown/scripts/scripts_mytown.lst`

```
myscript.int local_vars=5
another.int
```

- Each line is a script filename (with `.int` extension).
- Optionally add `local_vars=N` to reserve local variables for the script (default 0).
- The script name (without `.int`) is used to generate a **stable script index** (slot).

**Important:** The list file **must** be named `scripts_<modname>.lst` (e.g., `scripts_mytown.lst`).  
The base `scripts.lst` is for vanilla scripts and is not modified.

---

## 3. Place Your Compiled Script

Put your `.int` file in the **same folder** as the `.lst` file:

```
mods/mod_mytown/
        └─ scripts/
            ├─ scripts_mytown.lst
            └─ myscript.int
```

**How to compile a script:**
- Write your script in **Fallout 2 script syntax** (`.ssl`).
- Compile with `ScriptEditor.exe` to produce `.int` file.
- Name the `.int` file exactly as listed (e.g., `myscript.int`).

---

## 4. Using Your Script in the Game

The script index is automatically assigned by hashing the script name.  
To find the index, run the game once and check `data/lists/scripts_list.txt`.

**Example output:**

```
MOD SCRIPTS:
    207: myscript (local_vars=5)
```

Use this index (207 in the example) when:

- Assigning a script to a **critter** (in `.pro` file or via override `script=207`)
- Assigning to an **item** or **scenery** in the mapper

**In a prototype `.lst` override:**

```
myguard.pro script=207
```

**In a map object property:** set `Script` field to `207`.

---

## 5. Test Your Script

1. Run the game once – this generates `data/lists/scripts_list.txt`.
2. Find your script index in the report.
3. Attach the script to an object (e.g., a critter in the mapper).
4. Trigger the script (e.g., talk to the critter, enter a map, etc.).

If the script doesn’t run, check:
- The `.int` file is compiled correctly and placed in the right folder.
- The script index matches the one in the report.
- Your script has the required procedure (e.g., `talk_p_proc` for dialogue).

---

## 6. Important Notes

- **Stable indices** – The same mod name + script name always produce the same index. You can safely hardcode the index from `scripts_list.txt`.
- **Hash collisions** – If two different mods produce the same index, a warning appears and the script is **not loaded**. Rename your script file to resolve.
- **Local variables** – Use `local_vars=N` to allocate N local variables. They persist across saves.
- **No message file** – Script messages are loaded from `dialog/` files using `message_str()` calls. See the Dialog guide for that.

---

## 7. Quick Troubleshooting

| Problem | Likely Fix |
|---------|-------------|
| Script not in `scripts_list.txt` | Check `.lst` filename matches `scripts_<modname>.lst` and is in the correct folder. |
| Script index shows “Error” | The `.int` file is missing or misnamed. Must match exactly the name in `.lst`. |
| “Hash collision” popup | Another mod uses the same hash. Rename your script file (change the name). |
| Script doesn’t execute | Verify the script has the correct procedure (e.g., `start`, `talk_p_proc`). |
| Local variables not working | Add `local_vars=N` to the `.lst` line. |

Now you can add custom scripts to your mod – for quests, NPC behaviors, items, and more!