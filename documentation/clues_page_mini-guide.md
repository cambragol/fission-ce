---
layout: modpage
title: Clues System Mini-guide
include_in_header: false
include_in_footer: false
---

# FISSION Clues System Mini-Guide

> Add Readable Articles, Images, and Cross‑Links to the Pip‑Boy

This guide assumes you have a working mod folder (e.g. `mods/mod_mytown.dat/` from the Quick Start Guide). You'll add articles that appear under the **CLUES** tab in the Pip‑Boy.

---

## 1. What You Need

Your existing mod folder: `mods/mod_mytown.dat/` (a folder for development, later compressed into a `.dat` file for release).

You will create **one or more `.txt` files** - each file is a separate article. Optionally, you can also add **FRM images** to display inside articles.

---

## 2. Creating a Clues Article File

**File location:** `mods/mod_mytown/text/english/clues/` (or `text/<language>/clues/` for other languages).

**Naming:** Any filename ending with `.txt` (e.g. `my_note.txt`, `secret_plans.txt`). Duplicate titles are ignored.

### Format

- **First line:** The article title (max 255 characters).
- **Following lines:** The article content (plain text with optional markup).

### Example - `my_note.txt`

```text
A Mysterious Note

I found this note behind the old safe. It reads:

The combination is 42-17-36.

I should check the [[Security Terminal]] for more clues.
```

---

## 3. Formatting and Markup

Your article content supports the following markup:

| Markup | Effect |
|--------|--------|
| `*text*` | **Bold** text (rendered in a lighter green). |
| `_text_` | Underlined text. |
| `[[Target Title]]` | A clickable link to another article. The target must match the title of another article (case-insensitive; spaces and punctuation are preserved). |
| `[img:filename]` | Inserts an image. `filename` is the name of a `.frm` file (without extension) placed in `art/clues/`. The image is scaled to fit the Pip‑Boy screen and rendered with a green CRT effect. |

### Combining markup

You can combine bold and underline, for example:

```text
*_important_*
```

Links cannot be nested inside bold or underline.

### Example with all markup

```text
The Vault Door

[img:vault_door]

The door is sealed with a high-security lock.

I found a note saying: the code is 42-17-36.

Maybe [[The Security Terminal]] has more info.
```

---

## 4. Where to Put Everything

```text
mods/mod_mytown/
├─ art/
│  └─ clues/
│     ├─ vault_door.frm
│     └─ terminal.frm
└─ text/
   └─ english/
      └─ clues/
         ├─ my_note.txt
         ├─ security_terminal.txt
         └─ ...
```

- **Article files** go in `text/<language>/clues/` (e.g. `text/english/clues/`).
- **Images** go in `art/clues/` and must be Fallout FRM files (palette-indexed).

The system automatically converts images to green shades when displayed.

### Language fallback

If the current game language is not English and the corresponding `clues/` folder is missing, the system falls back to `text/english/clues/`.

---

## 5. Testing Your Clues

1. Run the game with your mod loaded.
2. Open the Pip‑Boy (`P`) and click the **CLUES** button (broken wire).
3. Your article titles should appear in the list.
4. Open an article to verify that links and images work correctly.

---

## 6. Important Notes

- **Title uniqueness:** Duplicate titles are ignored. Matching is case-insensitive.
- **Link resolution:** `[[Target Title]]` links are resolved at runtime. Missing targets appear as plain text and are not clickable.
- **Automatic pagination:** Articles are split into pages automatically when they exceed the Pip‑Boy screen height.
- **Image rendering:** Images are centered and displayed with a retro scanline effect using the Pip‑Boy green palette.
- **Performance:** Keep images under **350 × 410 pixels** for best results. Larger images are clipped.

---

## 7. Quick Troubleshooting

| Problem | Likely Fix |
|---------|------------|
| Article not in list | Verify the `.txt` file is in the correct `clues/` folder and has a non-empty first line. Check `clues_list.txt` for errors. |
| Title shows "Error!" | The file is missing or the first line is empty. |
| Links don't work | The target title must exactly match the destination article's title. |
| Image not showing | Ensure the `.frm` is in `art/clues/` and the filename in `[img:filename]` matches exactly (without extension). |
| Image appears too dark | Use a higher-contrast FRM. The renderer stretches contrast, but extremely dark images remain dark. |
| "Hash collision" popup | Not related to the Clues system. That message comes from other systems such as items or holodisks. |

---

## 8. Advanced: Customizing the Green Palette

The Pip‑Boy image renderer uses a hardcoded `greenPalettes[]` array in the source code. Changing the palette requires recompiling FISSION.

You can also adjust the `blackThreshold` value (default `4`) in `cluesRenderImage()` to change how very dark pixels are treated, but this is intended for engine developers rather than mod authors.