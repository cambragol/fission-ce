---
layout: modpage
title: Critter Mini-guide
include_in_header: false
include_in_footer: false
---

# FISSION Critter Modding Mini-Guide
> Add a Custom Critter (NPC, Creature, or Enemy)

This guide assumes you have a working mod folder (e.g., `mods/mod_mytown.dat/` from the Quick Start Guide). You’ll add a new NPC that can be placed on maps, fight, talk, or follow the player.

---

## 1. What You Need

Your existing mod folder: `mods/mod_mytown.dat/` (a folder for development, compress into actual .dat for release)

You will create **two new files** for your critter:

- `proto/critters/critters_mytown.lst` – lists your custom critter file(s)
- `text/english/game/pro_crit_mytown.msg` – provides name and description

Plus the actual **`.pro` file**  – you can export it from the **Mapper** or copy and modify an existing one.

---

## 2. Create the Proto Definition File

**File:** `mods/mod_mytown/proto/critters/critters_mytown.lst`

```
myguard.pro
```

Each line is the name of a `.pro` file (without path). You can add multiple lines for several critters.

### Optional Overrides

After the filename, you can add `key=value` pairs to override certain fields:

```
myguard.pro fid=450 ai=3 script=12
```

| Override | Example | Applies To | Meaning |
|----------|---------|------------|---------|
| `fid` | `450` | Critter (or Item) | Replace the creature’s base appearance (art frame index). For items, this changes the **world model** (ground sprite). |
| `ai` | `3` | Critter | Override AI packet index (1‑based, from `ai.txt` or `ai_mytown.txt`). |
| `script` | `12` | Critter/Item/Scenery | Override script index (1‑based, from `scripts.lst` or `scripts_mytown.lst`). |
| `male_fid` | `23` | Armor only | Set the appearance frame index for **male** characters wearing this armor. |
| `female_fid` | `24` | Armor only | Set the appearance frame index for **female** characters wearing this armor. |

> **Important:** `male_fid` and `female_fid` **only apply to armor items** (`ITEM_TYPE_ARMOR`). If you are adding a custom armor, these overrides control how it looks when equipped on male/female critters. The values are raw art indices (e.g., from `art\critters\`), not full FIDs – the engine builds the correct FID automatically.

> **Why overrides?** They let you reuse a base `.pro` file (e.g., a “human guard” template) but give it a unique look, AI, or behavior using mod assets. Currently the mapper.exe will not support assets in beyond the vanilla games base.

---

## 3. Create the Message File

**File:** `mods/mod_mytown/text/english/game/pro_crit_mytown.msg`

Each critter gets **two consecutive entries**: name then description.

```
{0}{}{Town Guard}
{1}{}{A well-armed protector of the settlement.}
{2}{}{Bandit Leader}
{3}{}{The ruthless boss of a raider gang.}
```

- The first critter (`myguard.pro`) will use offsets `0` (name) and `1` (description).
- The second critter (second line in `.lst`) would use offsets `2` and `3`, etc.

> **Note:** The base ID for these messages is generated automatically from your mod name (`mytown`) and the type (`crit`). You don’t need to calculate anything.

---

## 4. Where to Put the `.pro` File

The `.pro` file must be placed in the **same folder** as the `.lst` file:

```
mods/mod_mytown/
        └─  proto/
            └─  critters/
                    ├─ critters_mytown.lst
                    └─  myguard.pro
```

**How to get a `.pro` file:**
- **Option A:** Use the **Mapper** - open the Proto Editor, create a new critter, save it as `myguard.pro` in the mod’s `proto/critters/` folder.
- **Option B:** Copy an existing vanilla `.pro` (e.g., from `proto/critters/`) into your mod folder and rename it. The PID inside the `.pro` file is **ignored** – FISSION generates its own stable PID from the mod name and filename.

---

## 5. Test Your Critter

1. Run the game once (or reload if already running).
2. Check `data/lists/proto_list.txt` – your critter should appear under “CRITTER MOD PROTOS”.
3. Find the generated PID (e.g., `33686018`). You can now use it in scripts:

```
variable npc_pid;
npc_pid := 33686018;   // use the number from proto_list.txt
create_object(npc_pid, tile, elevation);
```

---

## 6. Important Notes

- **PID generation is stable** – the same mod name + proto name always produce the same PID. You can safely hardcode the PID from proto_list.txt in your scripts.

- **PID Ignored** The PID inside the .pro file is ignored – do not worry about what number it shows.

- **Hash collisions** – if two different mods accidentally generate the same PID, a warning message box appears and the second critter is skipped. Rename your mod or proto to resolve.

- **Message IDs** – each critter uses two consecutive IDs (name, description). The base ID is allocated per mod, so you never need to assign numbers manually.

- **AI packets** – Standard AI packets are defined in ai.txt (vanilla: 0 = none, 1 = coward, 2 = aggressive, etc.). You can also add custom AI packets via ai_mytown.txt.

---

## 7. Quick Troubleshooting

| Problem | Likely Fix |
|---------|-------------|
| Critter not in `proto_list.txt` | Check `.lst` filename matches `critters_mytown.lst` (with your mod name). |
| Name shows “Error!” | The `.msg` file is missing or offsets are wrong. Use `{0}` for first critter’s name, `{1}` for its description. |
| “Hash collision” popup | Another mod uses the same PID. Change your mod name or rename the proto file. |
| Art doesn’t show | Override `fid` to a valid FRM index, or make sure the art file exists. |
| NPC doesn’t fight | Check the `ai` override or the AI packet set in the `.pro` file. |
| Armor appearance not updating on NPCs | This is a known engine limitation. The override works for inventory display and the player character; for NPCs, you need engine‑side changes (see above). |

---

## 8. Dialog Heads and Backgrounds (FISSION-specific)

You can give any NPC a talking head and a custom background **without editing the NPC's script**. This is useful for adding heads to existing characters, companions, or flavor NPCs.

### What You Need

- **Head art files** (`.frm`) in `art/heads/`
- A **`heads_<mod>.lst`** file in `art/heads/`
- *(Optional)* **Background art files** (`.frm`) in `art/backgrnd/`
- *(Optional)* **Global variables** defined via `gvar_<mod>.txt` for dynamic backgrounds

### 8.1 Head File Naming (Vanilla Convention)

The engine uses the **base name** of your head and appends two-letter codes to build the actual filenames.

| Animation | Male Suffix | Female Suffix | Example |
|-----------|-------------|---------------|---------|
| Neutral talking | `np` | `nf` | `myheadnp.frm` |
| Good fidget | `gvf` + frame | `gff` + frame | `myheadgvf1.frm` |
| Bad fidget | `bvf` + frame | `bff` + frame | `myheadbvf1.frm` |

> **Note:** Vanilla Fallout 2 uses these suffixes (for example, `myronnp.frm`). The engine does **not** support custom remapping for dialog heads.

### 8.2 Create `heads_<mod>.lst`

Place this file in `art/heads/`.

**Syntax**

```text
<basename>,<good_count>,<neutral_count>,<bad_count> npc_script=<script_name> [bg_gvar=<gvar_index>] [background=<bg_index>]
```

- **`basename`** — Base name of the head (for example, `myhead`).
- **`good_count`**, **`neutral_count`**, **`bad_count`** — Number of fidget frames for each expression (usually `3`).
- **`npc_script`** — **Required.** Script basename without `.int`.
- **`bg_gvar`** — *(Optional)* Global variable index for a dynamic background.
- **`background`** — *(Optional)* Static background index used as a fallback.

**Example**

```text
myhead,3,3,3 npc_script=FCMer bg_gvar=6991 background=5
```

This assigns the head **`myhead`** to any NPC using the script **`FCMer`**. If global variable `6991` contains a value other than `-1`, that value becomes the background index. Otherwise, background `5` is used.

> If both `bg_gvar` and `background` are omitted, the NPC uses its normal dialog background.

### 8.3 Finding Script Names

FISSION generates a list of every loaded script:

```text
data/lists/scripts_list.txt
```

Use the script's **base name** (without `.int`) as the value for `npc_script=`.

### 8.4 Finding Global Variable Indices

If you define globals in `gvar_<mod>.txt`, FISSION assigns them stable numeric indices. You can find them in:

```text
data/lists/gvars_list.txt
```

Use the numeric index in `bg_gvar=`.

### 8.5 Background Indices

Backgrounds are FRM files listed in `backgrnd.lst` (or `backgrnd_<mod>.lst`).

- Place custom background FRMs in `art/backgrnd/`.
- List them in `backgrnd_<mod>.lst`.
- The background index is its position in the list, starting from **0**.

### 8.6 Precedence (Important)

FISSION applies dialog heads and backgrounds in this order:

1. **Head assignment** — If an NPC matches a `npc_script=` entry, that head is used.
2. **Dynamic background** — `bg_gvar` overrides the static background whenever its value is **not** `-1`.
3. **Static background** — Used only when no dynamic background is available.

### 8.7 Example Workflow

1. Create your head art files (`myheadnp.frm`, `myheadgvf1.frm`, etc.) in `art/heads/`.
2. Create `art/heads/heads_mymod.lst`:

```text
myhead,3,3,3 npc_script=FCMer background=2
```

3. *(Optional)* Define a GVAR in `gvar_mymod.txt`:

```text
MY_BG = -1
```

4. Find its assigned index (for example, `6991`) in `gvars_list.txt` and update the entry:

```text
myhead,3,3,3 npc_script=FCMer bg_gvar=6991 background=2
```

5. Change the background from any script:

```c
set_global_var(6991, 5);   // Background index 5
```

The next time the player talks to that NPC, background **5** will be displayed automatically.

### 8.8 Notes

- Works with **any NPC**, including companions, merchants, and quest givers.
- The NPC's script does **not** need to call `set_head()` or `set_background()`.
- Multiple NPCs can share the same head by adding multiple `npc_script=` entries.
- Head assignment is based on **script name**, not PID, making it more reliable when different NPCs share the same prototype.


## 9. What's Next?

Once your custom NPC is working, give it a unique combat personality with the [AI Mini-Guide](https://cambragol.github.io/fallout-fission/AI_mini-guide). You can define aggression, weapon preferences, taunts, and even disposition variants (coward, defensive, aggressive, berserk) that respond to the `ai_set_disposition` script command.