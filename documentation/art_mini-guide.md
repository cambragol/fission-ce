---
layout: modpage
title: Art Mini-guide
include_in_header: false
include_in_footer: false
---

# FISSION Art Mini-Guide  
> Add Custom FRM Graphics (Items, Critters, Scenery, etc.)

This guide assumes you have a working mod folder (e.g., `mods/mod_mytown.dat/` from the Quick Start Guide). ¥ You’ll add new `.frm` art files that can be used by items, critters, scenery, or interface elements.

---

## 1. What You Need

Your existing mod folder: `mods/mod_mytown.dat/`. (a folder for development, compress into actual .dat for release)

You will create **one new file** per art category:

- `art/<category>/<category>_<modname>.lst` – lists your custom `.frm` files

Plus the actual `.frm` files (created with **Frame Animator** or converted from other formats).

**Art categories** (`<category>`):

| Category | Folder | Example `.lst` |
|----------|--------|----------------|
| items | `art/items/` | `items_mytown.lst` |
| critters | `art/critters/` | `critters_mytown.lst` |
| scenery | `art/scenery/` | `scenery_mytown.lst` |
| walls | `art/walls/` | `walls_mytown.lst` |
| tiles | `art/tiles/` | `tiles_mytown.lst` |
| misc | `art/misc/` | `misc_mytown.lst` |
| intrface | `art/intrface/` | `intrface_mytown.lst` |
| inven | `art/inven/` | `inven_mytown.lst` |
| heads | `art/heads/` | `heads_mytown.lst` |
| backgrnd | `art/backgrnd/` | `backgrnd_mytown.lst` |
| skilldex | `art/skilldex/` | `skilldex_mytown.lst` |

---

## 2. Create the Art List File

**File:** `mods/mod_mytown/art/items/items_mytown.lst`

```
my_sword.frm
my_helmet.frm
```

- Each line is a filename (with `.frm` extension).
- Files can be in subfolders – include the relative path (e.g., `weapons/my_sword.frm`).
- The base name (without extension) is used to generate a **stable index** in the mod range (4096–8191).

**Optional remapping:**  
You can replace an existing vanilla asset with your own using `@` syntax:

```
@original_name = my_new.frm
```

Example: `@hmjmps = my_jumpsuit.frm` – replaces the male vault jumpsuit with your custom FRM.

---

## 3. Place Your `.frm` Files

Put the `.frm` files in the **same folder** as the `.lst` file (or in subfolders as specified):

```
mods/mod_mytown/
    └─ art/
        └─ items/
            ├─ items_mytown.lst
            ├─ my_sword.frm
            └─ my_helmet.frm
```

For critters, use `art/critters/critters_mytown.lst` and place `.frm` files there.

---

## 4. How It Works (Behind the Scenes)

- The mod name is extracted from the `.lst` filename (`mytown` from `items_mytown.lst`).
- For each asset, the engine computes a **stable hash** of the base filename.
- The hash is mapped to an index in the mod range (`4096` ... `8191`).
- If two different mods produce the same index, a popup appears and the **second asset overwrites the first** (last loaded wins).
- The game generates a report: `data/lists/art_list.txt` showing every asset and its final index.

---

## 5. Find Your Art Index

1. Run the game once.
2. Open `data/lists/art_list.txt`.
3. Look under the relevant category (e.g., `[items]`) in the **MOD ASSETS** section:

```
MOD ASSETS:
4096: my_sword.frm
4097: my_helmet.frm
```

Use these numbers when building FIDs in scripts or prototypes.

---

## 6. Use Your Art in the Game

**In a prototype `.lst` override** (for items):

```
my_sword.pro fid=4096
```

**In a script** (to create an object with custom art):

```
variable obj;
create_object_sid(proto_pid, tile, elevation, -1);
obj = self_obj;
set_obj_fid(obj, build_fid(OBJ_TYPE_ITEM, 4096, 0, 0, 0));
```

For critters, the FID is built automatically from the index – just reference the critter proto.

---

## 7. Important Notes
Stable indices – The same mod name + filename always produce the same index. You can safely hardcode the index from art_list.txt.

Overwrites – If two mods claim the same index, the last loaded mod wins. A warning is shown and logged.

Remapping – Use @original = new.frm to replace vanilla assets without changing the original .lst.

No message files – Art assets have no names or descriptions; those come from prototypes or messages.

Widescreen variants – The system automatically looks for _800.frm variants (e.g., my_sword_800.frm) if you enable widescreen mode.

---

## 8. Quick Troubleshooting

| Problem | Likely Fix |
|---------|-------------|
| Art not in `art_list.txt` | Check `.lst` filename matches `<category>_<modname>.lst` and is in the correct folder. |
| Index shows wrong file | Another mod overwrote your asset. Change your filename to get a different hash. |
| “Hash collision” popup | Another mod already uses that index. Rename your `.frm` file to change the hash. |
| Art doesn’t show in game | Verify the FID uses the correct index and object type. Check `art_list.txt` for the exact index. |
| Remapping not working | Syntax must be `@original_name = new_path.frm`. The original name is the base filename (no extension, no path). |