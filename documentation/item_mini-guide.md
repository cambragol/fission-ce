---
layout: modpage
title: Item Mini-guide
include_in_header: false
include_in_footer: false
---

# FISSION Item Modding Mini-Guide
> Add a Custom Weapon, Armor, or Misc Item

This guide assumes you have a working mod folder (e.g., `mods/mod_mytown.dat/` from the Quick Start Guide). You’ll add a new item that can be placed in the world, given to the player, or used in scripts.

---

## 1. What You Need

Your existing mod folder: `mods/mod_mytown.dat/`. (a folder for development, compress into actual .dat for release)

You will create **two new files** for your item:

- `proto/items/items_mytown.lst` – lists your custom item file(s)
- `text/english/game/pro_item_mytown.msg` – provides name and description

Plus the actual **`.pro` file**  – you can export it from the **Mapper** or copy and modify an existing one.

---

## 2. Create the Proto Definition File

**File:** `mods/mod_mytown/proto/items/items_mytown.lst`

```
myweapon.pro
```

Each line is the name of a `.pro` file (without path). You can add multiple lines for several items.

### Optional Overrides

After the filename, you can add `key=value` pairs to override certain fields:

```
myweapon.pro fid=120 inventory_fid=250
```

| Override | Example | Meaning |
|----------|---------|---------|
| `fid` | `120` | Replace the ground/in‑world FID (art frame index) |
| `inventory_fid` | `250` | Replace the inventory icon FID |
| `script` | `42` | Override script index (1‑based, as in `scripts.lst`) |

---

## 3. Create the Message File

**File:** `mods/mod_mytown/text/english/game/pro_item_mytown.msg`

Each item gets **two consecutive entries**: name and description.

```
{0}{}{Laser Sword}
{1}{}{A deadly energy blade.}
{2}{}{Plasma Rifle}
{3}{}{Fires superheated plasma bolts.}
```

- The first item (`myweapon.pro`) will use offsets `0` (name) and `1` (description).
- The second item (second line in `.lst`) would use offsets `2` and `3`, etc.

> **Note:** The base ID for these messages is generated automatically from your mod name (`mytown`) and the type (`item`). You don’t need to calculate anything.

---

## 4. Where to Put the `.pro` File

The `.pro` file must be placed in the **same folder** as the `.lst` file:

```
mods/mod_mytown/
        └─ proto/
            └─ items/
                ├─ items_mytown.lst
                └─ myweapon.pro
```

**How to get a `.pro` file:**
- **Option A:** Use the **Mapper** → open the Proto Editor, create a new item, save it as `myweapon.pro` in the mod’s `proto/items/` folder.
- **Option B:** Copy an existing vanilla `.pro` (e.g., from `proto/items/`) into your mod folder and rename it. The PID inside the `.pro` file is **ignored** – FISSION generates its own stable PID from the mod name and filename.

---

## 5. Test Your Item

1. Run the game once.
2. Check `data/lists/proto_list.txt` – your item should appear under “ITEM MOD PROTOS”.
3. Find the generated PID (e.g., `16843034`). You can now use it in scripts:

```
variable item_pid;
item_pid := 16843034;   // use the number from proto_list.txt
create_object(item_pid, 0, 0);
```

Or give it to the player:

```
add_obj_to_inven(dude_obj, item_pid);
```

---

## 6. Important Notes
PID generation is stable – the same mod name + proto name always produce the same PID. You can safely hardcode the PID from proto_list.txt in your scripts.

The PID inside the .pro file is ignored – do not worry about what number it shows.

Hash collisions – if two different mods accidentally generate the same PID, a warning message box appears and the second item is skipped. Rename your mod or proto to resolve.

Message IDs – each item uses two consecutive IDs (name, description). The base ID is allocated per mod, so you never need to assign numbers manually.

---

## 7. Quick Troubleshooting

| Problem | Likely Fix |
|---------|-------------|
| Item not in `proto_list.txt` | Check `.lst` filename matches `items_mytown.lst` (with your mod name). |
| Name shows “Error!” | The `.msg` file is missing or offsets are wrong. Use `{0}` for first item’s name, `{1}` for its description. |
| “Hash collision” popup | Another mod uses the same PID. Change your mod name or rename the proto file. |
| Art doesn’t show | Override `fid` or `inventory_fid` to a valid FRM index, or make sure the art file exists. |