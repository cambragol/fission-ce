---
layout: modpage
title: End Game Mini-guide
include_in_header: false
include_in_footer: false
---

# FISSION Endgame Slideshow Mini‑Guide  
> Add Custom Ending Slides (Static or Panning) with Voice‑Over

This guide assumes you have a working mod folder (e.g., `mods/mod_mytown.dat/` from the Quick Start Guide). You’ll add new ending slides that appear at the end of the game, based on the player’s actions.

---

## 1. What You Need

Your existing mod folder: `mods/mod_mytown.dat/` (a folder for development, compress into actual .dat for release)

You will create **one new file**:

- `data/endgame_mytown.txt` – defines the ending slide(s)

Plus art (`.frm`) and voice‑over (`.acm` / `.wav`) files.

---

## 2. Create the Endgame Definition File

**File:** `mods/mod_mytown/data/endgame_mytown.txt`

Format: GVAR, Value, ArtNum, VoiceOverBase, Direction
```
79, 1, 999, nar_mytown_ending, 1
```

**Fields explained:**

| Field | Example | Meaning |
|-------|---------|---------|
| `GVAR` | `79` | Global variable that must **equal** `Value` for this ending to play. |
| `Value` | `1` | Required GVAR value. |
| `ArtNum` | `999` | FRM index (from `art/intrface/`) of the slide image. Supports widescreen variants with the suffix - `_800.frm`. |
| `VoiceOverBase` | `nar_mytown_ending` | Base filename of the narrator audio (no extension). |
| `Direction` | `1` | `0` = static slide, `1` = pan left->right, `-1` = pan right->left. |

When the game ends, it checks all endings in order. The **first** ending whose GVAR matches its value is played. (Only one ending is shown.)

---

## 3. Where to Put the Files

```
mods/mod_mytown/
        ├─ data/
        │   └─ endgame_mytown.txt
        ├─ art/
        │   └─ intrface/
        │       ├─ myending.frm (standard version)
        │       └─ myending_800.frm (optional widescreen variant)
        └─ narrator/
            ├─ nar_mytown_ending.acm (or .wav)
            └─ (optional language subfolder)
```

**Art notes:**
- The `ArtNum` corresponds to the **index** in `art/intrface/` (from `intrface.lst` or mod lists).
- For widescreen support, place a `_800.frm` variant (e.g., `myending_800.frm`) alongside the standard one.

---

## 4. Creating the Art and Audio

### Art (`.frm` file)
- Create a 640×480 (or 800×500 for widescreen) image.
- Convert to `.frm` using **Frame Animator** or a converter.
- Place in `art/intrface/` and add it to your mod’s `intrface_mytown.lst` (see Art Mini‑Guide).

### Voice‑Over (`.acm` / `.wav`)
- Record narration (e.g., “And so the hero returned home…”).
- Convert to `.acm`.
- Place in `narrator/` (or `narrator/<language>/` for localization).
- The file name must match `VoiceOverBase` (e.g., `nar_mytown_ending.acm`).

**Subtitles (optional):**
- Create `text/<language>/cuts/<VoiceOverBase>.txt`
- Format: each line `time:text` (time is ignored; text is displayed line by line).
- Example:

```
0:And so the story ends.
0:The wasteland will never be the same.
```

---

## 5. Triggering the Ending from Scripts

Set the appropriate GVAR to the required value before the final battle or event. For example:

```
set_global_var(79, 1);
```

When the player finishes the game (e.g., defeats the final boss, saves their vault etc.), the slideshow will play automatically if the GVAR condition is met.

---

## 6. Test Your Ending

Set the required GVAR (e.g., via debug or script).

Trigger the game ending (e.g., travel to the endgame map or use endgame debug command).

Watch your slide(s) and listen to the narration.

If the default ending plays instead, check data/lists/endgame_list.txt to verify your entry loaded and appears before any conflicting endings.

---

## 7. Important Notes

Order matters – Endings are evaluated in the order they appear in all endgame_*.txt files (sorted by filename, then file order). The first match wins.

Widescreen variants – The system automatically looks for _800.frm if widescreen mode is enabled. Use artGetFidWithVariant internally.

Panning scenes – Direction = 1 or -1 creates a panning effect over a wider image (art must be wider than the screen).

Subtitles – Enabled by the player’s subtitle preference; timing is auto‑calculated from speech duration.

No hash collisions – There is no collision system; endings are simply appended. To override a vanilla ending, ensure your GVAR condition is unique and your file loads earlier (filename order).

---

## 8. Quick Troubleshooting

| Problem | Likely Fix |
|---------|-------------|
| Ending not shown | Check `data/lists/endgame_list.txt` – your entry must appear **before** any other ending with the same GVAR/value. |
| Wrong slide shows | Another ending with a different GVAR/value matched first. Adjust your GVAR or value to be unique. |
| Art not found | Verify `ArtNum` exists in `art/intrface/` and is listed in `intrface_mytown.lst` (or vanilla). |
| Voice‑over doesn’t play | Audio file missing or wrong format. Place `.acm` in `narrator/` with correct base name. |
| Subtitles not showing | Player has subtitles disabled in options, or `.txt` file missing from `text/<language>/cuts/`. |
| Panning slide jittery | Ensure art width is at least screen width + 200 pixels for smooth panning. |