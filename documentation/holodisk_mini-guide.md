---
layout: modpage
title: Holodisk Mini-guide
include_in_header: false
include_in_footer: false
---

# FISSION Holodisk Mini-Guide  
> Add Readable Holodisks to the Pip‑Boy

This guide assumes you have a working mod folder (e.g., `mods/mod_mytown.dat/` from the Quick Start Guide). You’ll add one or more holodisks that appear in the Pip‑Boy’s “DATA” section.

---

## 1. What You Need

Your existing mod folder: `mods/mod_mytown.dat/`. (a folder for development, compress into actual .dat for release)

You will create **two new files per holodisk**:

- `data/holodisk_mytown.txt` – lists each holodisk with its GVAR and a unique block key
- `text/english/game/holodisk_mytown_<BlockKey>.msg` – contains the title and text lines

---

## 2. Create the Holodisk Definition File

**File:** `mods/mod_mytown/data/holodisk_mytown.txt`

Format: GVAR, BlockKey
```
79, mynote
80, another_note
```

- **GVAR** – Global variable that controls whether the holodisk is visible (set to non‑zero in scripts).
- **BlockKey** – Unique identifier for this holodisk inside your mod (letters, numbers, underscores). It becomes part of the message filename.

---

## 3. Create the Holodisk Message File

**File:** `mods/mod_mytown/text/english/game/holodisk_mytown_mynote.msg`

```
{0}{}{A Mysterious Note}
{1}{}{I found this behind the old safe...}
{2}{}{The combination is 42-17-36.}
{3}{}{END-DISK}
```

**Rules:**
- `{0}` – Title (shown in the holodisk list).
- `{1}`, `{2}`, … – Text lines (each line becomes a separate paragraph).
- Last line must be `**END-DISK**` – the reader stops here.
- You can have up to 99 lines (excluding the title). If you need more, split into multiple holodisks.

> **Note:** The filename must match the pattern `holodisk_<ModName>_<BlockKey>.msg`.  
> Example: `holodisk_mytown_mynote.msg`

---

## 4. Where to Put Everything

```
mods/mod_mytown/
        ├─ data/
        │ ├─ holodisk_mytown.txt
        │ └─ ... (other files)
        └─ text/
            └─ english/
                └─ game/
                    ├─ holodisk_mytown_mynote.msg
                    ├─ holodisk_mytown_another_note.msg
                    └─ ...
```

---

## 5. Activate a Holodisk from a Script

Set the corresponding GVAR to any non‑zero value. For example, in your map or global script:

```
set_global_var(79, 1);   // Makes the "A Mysterious Note" appear
```

Once the GVAR is set, the holodisk will immediately appear in the Pip‑Boy’s DATA list.

---

## 6. Test Your Holodisk
Run the game and load your save (or start a new game).

Set the GVAR via script.

Open the Pip‑Boy (P key), click the 'Status' button.

You should see the holodisk title(s) listed on the right side. Click one to read the content.

If something goes wrong, check data/lists/holodisks_list.txt – it lists every holodisk (vanilla + mod) with its base ID and title.

---

## 7. Important Notes
Block allocation – Each holodisk automatically gets 100 consecutive message IDs (0 = title, 1–99 = text, END-DISK required). You never need to assign numbers.

Stable IDs – The same ModName + BlockKey always produce the same base ID. Your saves will work across game sessions.

GVARs – Choose a GVAR that is not used by other mods or vanilla. Or assign your own GVAR in `gvar_mytown.txt`

End marker – Always end with **END-DISK**. Without it, the reader may crash or show garbage.

---

## 8. Quick Troubleshooting

| Problem | Likely Fix |
|---------|-------------|
| Holodisk not in list | GVAR not set, or `holodisk_<modname>.txt` has wrong format. Check `holodisks_list.txt`. |
| Title shows “Error!” | Message file missing or misnamed. Must be `holodisk_<ModName>_<BlockKey>.msg`. |
| Text stops early | Missing `**END-DISK**` line at the end of the `.msg` file. |
| “Hash collision” popup | Another mod uses the same `ModName`+`BlockKey`. Change your `BlockKey`. |