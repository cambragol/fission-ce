# Fallout FISSION Modding System: Complete Implementation Guide

## Table of Contents

1. [System Overview](#1-system-overview)
2. [Core Architecture](#2-core-architecture)
3. [File Structure & Naming Conventions](#3-file-structure--naming-conventions)
4. [Configuration Files](#4-configuration-files)
   - [4.1 Area Configuration](#41-area-configuration-city_modnametxt)
   - [4.2 Map Configuration](#42-map-configuration-maps_modnametxt)
   - [4.3 Quest Configuration](#43-quest-configuration-quests_modnametxt)
5. [Message System](#5-message-system)
6. [Stable ID Generation](#6-stable-id-generation)
7. [Implementation Details](#7-implementation-details)
   - [7.1 Area Loading](#71-area-loading-worldmapcc)
   - [7.2 Map Loading](#72-map-loading-worldmapcc)
   - [7.3 Quest Loading](#73-quest-loading-pipboycc)
   - [7.4 Message Loading](#74-message-loading-messagecc)
   - [7.5 Runtime Retrieval](#75-runtime-retrieval)
8. [Debugging & Reports](#8-debugging--reports)
9. [Modder Guidelines](#9-modder-guidelines)
10. [Known Limitations & Workarounds](#10-known-limitations--workarounds)
11. [Quest System Specifics](#11-quest-system-specifics)
12. [Troubleshooting](#12-troubleshooting)
13. [Future Expansion](#13-future-expansion)

---

## 1\. System Overview

The Fallout 2 FISSION modding system provides a framework for adding new maps, areas, quests, and text content without modifying base game files. Key features:

- **Stable, hash-based indexing** ensures consistent IDs across installations
- **Modular file organization** keeps mods self-contained
- **Automatic report generation** helps modders reference generated IDs
- **Backward compatibility** with vanilla saves and content
- **Localization support** with English fallback

### Supported Content Types:

1. **Maps** - Game locations with multiple elevations
2. **Areas** - World map regions containing multiple maps
3. **Messages** - Text for area names, map names, entrance labels, and quest descriptions
4. **Quests** - Quest definitions with automatically linked descriptions
5. **Scripts** - Already has stable ID generation (separate system)
6. **Art** - Already has stable ID generation (separate system)

---

## 2\. Core Architecture

### ID Ranges:

Quests: 0-199 (Vanilla)\
200-999 (Mods - hash-based allocation)

Maps: 0-199 (Vanilla)\
200-1999 (Mods - hash-based allocation)

Areas: 0-199 (Vanilla)\
200-999 (Mods - hash-based allocation)

Messages: 0-32767 (Vanilla)\
32768-65535 (Mods - hash-based allocation)

text

### Key Principles:

1. **Deterministic hashing** - Same inputs always produce same outputs
2. **Case-insensitive** - System normalizes to lowercase via `tolower()`
3. **Fail on collision** - Prevents silent overwrites (popup warnings)
4. **Separate concerns** - Config, art, scripts, messages in different files
5. **Automatic linking** - Quests auto-generate linked message IDs

---

## 3\. File Structure & Naming Conventions

### Complete Mod Directory Structure:

```
Fallout 2 Game Directory/\
├── data/ # Core game data\
│   ├── city_{modname}.txt # Area definitions\
│   ├── maps_{modname}.txt # Map definitions\
│   └── quests_{modname}.txt # Quest definitions\
├── text/ # Localization files\
│   └── english/\
│       └── game/\
│           └── messages_{modname}.txt # All text content\
├── art/ # Art assets (existing system - separate ID generation)\
├── scripts/ # Script files (existing system - separate ID generation)\
├── lists/ # Generated reports for assets indexes (art, maps, areas, messages, quests, scripts)\
└── ... (other game folders)
```

### Required Files for a FISSION Mod:

For the FISSION modding system specifically, you need these files **inside the `data/` directory**:

1. **`city_{modname}.txt`** - Area definitions (in `data/`)
2. **`maps_{modname}.txt`** - Map definitions (in `data/`)
3. **`quests_{modname}.txt`** - Quest definitions (in `data/`)
4. **`messages_{modname}.txt`** - Message/text content (in `data/text/english/game/`)

### Important Notes:

1. **Scripts and Art** use their own stable ID generation systems and are placed in separate top-level directories:
   - Scripts go in the `scripts/` folder
   - Art goes in the `art/` folder
   - These are **not** part of the FISSION file structure

2. **Message files** must be in the language-specific subdirectory:
   - English: `data/text/english/game/messages_{modname}.txt`
   - German: `data/text/german/game/messages_{modname}.txt`
   - Russian: `data/text/russian/game/messages_{modname}.txt`
   - etc.

3. **Vanilla files remain unchanged** - your mod files are added alongside them, not replacing them

### File Naming Rules:

- **All lowercase** recommended (system is case-insensitive but consistent)
- **No spaces or special characters** except underscore
- **Mod name must be consistent** across all files
- **Example:** For a mod named "wasteland":
  - `data/city_wasteland.txt`
  - `data/maps_wasteland.txt`
  - `data/quests_wasteland.txt`
  - `data/text/english/game/messages_wasteland.txt`

---

## 4. Configuration Files

### 4.1 Area Configuration (`city_{modname}.txt`)

Defines world map areas. Each area can contain multiple maps.

#### Format:

```
[Area 0]                        # Section number (sequential)
area_name = SCRAPTOWN           # MUST be uppercase, unique identifier
world_pos = 360,290             # World map coordinates
start_state = On                # On, Off, or Secret
size = Medium                   # Small, Medium, or Large
townmap_art_idx = 156           # Art index for town map
townmap_label_art_idx = 368     # Art index for town map label

# Entrances - links maps to this area
# Format: State,X,Y,MapLookupName,MapIndex,Unknown,Elevation
entrance_0 = On,110,220,SCRAPTOWN1,-1,-1,0
entrance_1 = On,235,250,SCRAPTOWN2,-1,-1,0
```

#### Rules:

-   `area_name` must be unique across all mods

-   Use uppercase for consistency

-   Maximum 8 characters recommended (not enforced but good practice)

### 4.2 Map Configuration (`maps_{modname}.txt`)

Defines individual game maps.

#### Format:

```
[Map 0]                         # Section number (sequential)
lookup_name = SCRAPTOWN1        # Unique identifier, referenced by entrances
map_name = scrapt1              # MUST be ≤8 characters (DOS 8.3 limitation)
city_name = SCRAPTOWN           # References area_name from city file
music = fs_grand                # Music track
saved = Yes                     # Yes/No - whether game saves here
automap = yes                   # Yes/No - shows on automap
dead_bodies_age = No            # Yes/No
can_rest_here = Yes,No,No       # Three values for elevations 0,1,2

# Ambient sound effects
ambient_sfx = gntlwin1:50, gntlwind:50

# Random start points (optional)
random_start_point_0 = elev:0,tile_num:12345:elev:0,tile_num:23456
```

#### Critical Rules:

1.  `map_name` ≤ 8 characters - DOS 8.3 limitation for save games

2.  `city_name` must match an `area_name` in the corresponding city file

3.  `lookup_name` must be unique across all mods

### 4.3 Quest Configuration (`quests_{modname}.txt`)

Defines quests using the vanilla `quests.txt` format with enhanced mod support.

#### Format:

```
# Format: location, description, gvar, displayThreshold, completedThreshold
# Note: 'description' field is IGNORED for mod quests - replaced by generated message ID
1500, 0, 79, 1, 2
1500, 0, 80, 1, 3
```

#### Rules:

1.  One quest per line in CSV format

2.  Comments start with `#` and are ignored

3.  Location - Area index (must be valid area, vanilla or mod)

4.  Description field - Ignored for mod quests (replaced by generated message ID)

5.  GVAR - Global variable tracking quest state

6.  Thresholds - Display and completion values

* * * * *

5\. Message System
------------------

### File Structure (`messages_{modname}.txt`)

Contains all text for your mod, organized by section. Keys must match exactly as shown!

#### Format:

```
[map]                                   # World map and automap text
area_name:SCRAPTOWN = Scraptown         # Area display name
area_name:HIGHTOWN = Hightown           # Another area display name
lookup_name:Scraptown1:0 = Scraptown Gate    # Map + elevation display name
lookup_name:Scraptown2:0 = Backdoor
lookup_name:Hightown1:0 = Highdoor
lookup_name:Hightown2:0 = Market

[worldmap]                              # Town map entrance labels
entrance_0:SCRAPTOWN = Scraptown Gate
entrance_1:SCRAPTOWN = Backdoor
entrance_0:HIGHTOWN = Highdoor
entrance_1:HIGHTOWN = Market

[quests]                                # Quest descriptions
quest:0 = The people of Scraptown are having trouble with raiders...
quest:1 = Mayor Johnson's family heirloom was lost...
quest:2 = Hightown needs a new drawbridge...
```

#### Critical Format Rules:

1.  Section headers must match exactly: `[map]`, `[worldmap]`, `[quests]`

2.  Case-sensitive keys: Use exact formats shown (lowercase for area_name/lookup_name/entrance_X)

3.  No trailing spaces in section headers or keys

4.  Key formats:

    -   Areas: `area_name:{AREA_NAME}` (lowercase `area_name:`)

    -   Maps: `lookup_name:{LOOKUP_NAME}:{ELEVATION}` (lowercase `lookup_name:`)

    -   Entrances: `entrance_{INDEX}:{AREA_NAME}` (lowercase `entrance_`)

    -   Quests: `quest:{INDEX}` (lowercase `quest:`)

#### Why These Formats?

These formats match the vanilla message file structure for consistency:

-   Vanilla uses `area_name:` for area names

-   Vanilla uses `lookup_name:` for map names

-   Vanilla uses `entrance_X:` for town map labels

-   We extended with `[quests]` section for mod quests

#### ID Generation:

cpp

// Area name message ID generation
generate_mod_message_id("myquest", "area_name:SCRAPTOWN")
// Returns consistent ID between 32768-65535

// Map name message ID generation
generate_mod_message_id("myquest", "lookup_name:Scraptown1:0")
// Returns consistent ID between 32768-65535

// Quest description message ID generation
generate_mod_message_id("myquest", "quest:0")
// Returns consistent ID between 32768-65535

#### Real-World Example from Vanilla Files:

Check vanilla `messages.txt` in `data/text/english/game/`:

```
area_name:ARROYO = Arroyo
lookup_name:ARROYO:0 = Temple
lookup_name:ARROYO:1 = Temple
entrance_0:ARROYO = Village
entrance_1:ARROYO = Temple
```

#### Matching in Code:

When retrieving names, the code uses these exact formats:

cpp

// In mapGetName():
snprintf(compositeKey, sizeof(compositeKey), "lookup_name:%s:%d", lookupName, elevation);

// In wmTownMapRefresh() (town map labels):
snprintf(messageKey, sizeof(messageKey), "entrance_%d:%s", index, areaName);

// In quest system:
snprintf(descKey, sizeof(descKey), "quest:%d", questIndexInThisMod);

#### Common Mistakes to Avoid:

1.  WRONG: `AREA:SCRAPTOWN` (uppercase, wrong prefix)

2.  WRONG: `MAP:SCRAPTOWN1:0` (wrong prefix)

3.  WRONG: `ENTRANCE:SCRAPTOWN:0` (wrong format)

4.  CORRECT: `area_name:SCRAPTOWN`

5.  CORRECT: `lookup_name:SCRAPTOWN1:0`

6.  CORRECT: `entrance_0:SCRAPTOWN`

#### Validation Tips:

1.  Check vanilla messages.txt for reference formats

2.  Use exact lowercase prefixes shown above

3.  Test with minimal mod - create simple test to verify format

4.  Check generated reports - wrong formats won't appear in reports

Important: The system looks for these exact key formats. If your keys don't match, your messages won't load, and you'll get "Error" text in-game.

* * * * *

6\. Stable ID Generation
------------------------

### Unified Hash Function (DJB2, Case-Insensitive):

cpp

uint32_t stable_hash(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        c = tolower(c);  // Critical: Case normalization
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }
    return hash;
}

### Mod Message ID Generator:

```
uint32_t generate_mod_message_id(const char* mod_name, const char* key) {
    char composite_key[256];
    snprintf(composite_key, sizeof(composite_key), "%s:%s", mod_name, key);
    uint32_t hash = stable_hash(composite_key);
    return 0x8000 + (hash % 0x7FFF);  // 32768-65535 range
}
```

### Quest-Specific ID Generation:

```
// In questLoadModFile():
char descKey[256];
snprintf(descKey, sizeof(descKey), "quest:%d", questIndexInThisMod);
int descMessageId = generate_mod_message_id(mod_name, descKey);
quest->description = descMessageId;  // Override file description
```

### Area Slot Allocation:

```
static uint16_t wmAreaCalculateModSlot(const char* areaName, uint32_t modNamespace) {
    char combinedKey[256];
    snprintf(combinedKey, sizeof(combinedKey), "%s|%u", areaName, modNamespace);
    uint32_t hash = stable_hash(combinedKey);
    return MOD_AREA_START + (hash % (MOD_AREA_MAX - MOD_AREA_START));
}
```

### Map Slot Allocation:

```
static uint16_t wmCalculateModMapSlot(const char* lookupName, uint32_t modNamespace) {
    char combinedKey[256];
    snprintf(combinedKey, sizeof(combinedKey), "%s|%u", lookupName, modNamespace);
    uint32_t hash = stable_hash(combinedKey);
    return MOD_MAP_START + (hash % (MOD_MAP_MAX - MOD_MAP_START));
}
```

### Quest Slot Allocation:

```
static uint16_t questCalculateModSlot(const char* questKey, uint32_t modNamespace, int questIndexInMod) {
    char combinedKey[256];
    snprintf(combinedKey, sizeof(combinedKey), "%s|%u|%d", questKey, modNamespace, questIndexInMod);
    uint32_t hash = stable_hash(combinedKey);  // Uses same unified hash
    return MOD_QUEST_START + (hash % (MOD_QUEST_MAX - MOD_QUEST_START));
}
```

* * * * *

7\. Implementation Details
--------------------------

### 7.1 Area Loading (`worldmap.cc`)

-   File: `wmAreaLoadModFile()`

-   Process:

    1.  Extract mod name from filename (`city_myquest.txt` → `"myquest"`)

    2.  Calculate mod namespace hash

    3.  For each area section:

        -   Calculate stable slot based on area name + mod namespace

        -   Store mod name in parallel array (not in CityInfo struct)

        -   Generate area message ID: `generate_mod_message_id("myquest", "area_name:AREA_NAME")`

        -   Set `city->areaId` to generated message ID

### 7.2 Map Loading (`worldmap.cc`)

-   File: `wmMapLoadModFile()`

-   Process:

    1.  Extract mod name from filename

    2.  For each map section:

        -   Calculate stable slot based on lookup name + mod namespace

        -   Use `city_name` field to link to area (not integer `city` field)

        -   Look up area by name and assign map to correct area index

### 7.3 Quest Loading (`pipboy.cc`)

-   File: `questLoadModFile()`

-   Process:

    1.  Extract mod name from filename (`quests_myquest.txt` → `"myquest"`)

    2.  For each quest line (indexed from 0 in mod file):

        -   Generate quest key: `"{modname}:{index}"`

        -   Calculate stable quest slot using `questCalculateModSlot()`

        -   Generate description message ID: `generate_mod_message_id("myquest", "quest:{index}")`

        -   Override description field with generated message ID

        -   Store mod name in `gQuestModNames[]` for tracking

### 7.4 Message Loading (`message.cc`)

-   File: `loadModFileWithSections()`

-   Process for quests:

    1.  Extract mod name from filename

    2.  Read `[quests]` section

    3.  For each `quest:{index} = text` pair:

        -   Generate same message ID using `generate_mod_message_id()`

        -   Add to quest message list (`gQuestsMessageList`)

### 7.5 Runtime Retrieval

```
// Quest description retrieval
static int getQuestDescriptionMessageId(int questId) {
    if (questId < MOD_QUEST_START) {
        // Vanilla quest: use description field directly
        return gQuestDescriptions[questId].description;
    } else {
        // Mod quest: description field already contains hashed message ID
        return gQuestDescriptions[questId].description;
    }
}
```

```
// Map name retrieval
char* mapGetName(int map, int elevation) {
    if (map >= MOD_MAP_START && map < MOD_MAP_MAX) {
        // Mod map: generate message ID using mod name and lookup name
        const char* lookupName = wmGetMapLookupName(map);
        int areaIndex = wmGetAreaContainingMap(map);

        if (areaIndex != -1 && lookupName != nullptr) {
            const char* modName = wmGetAreaModName(areaIndex);

            char compositeKey[256];
            snprintf(compositeKey, sizeof(compositeKey),
                     "lookup_name:%s:%d", lookupName, elevation);

            uint32_t messageId = generate_mod_message_id(modName, compositeKey);
            return getmsg(&gMapMessageList, &messageListItem, messageId);
        }
        return nullptr;
    }
    // ... vanilla handling
}
```

* * * * *

8\. Debugging & Reports
-----------------------

### Generated Reports:

The system automatically creates these files in `data/lists/`:

1.  `quests_list.txt` - All quests with slots, mod assignments, and generated message IDs

2.  `messages_quests_list.txt` - All mod quest descriptions with IDs

3.  `maps_list.txt` - All maps with slots, types, and override info

4.  `area_list.txt` - All areas with slots and mod assignments

5.  `messages_map_list.txt` - All mod messages in map.msg with IDs

6.  `messages_worldmap_list.txt` - All mod messages in worldmap.msg with IDs

### Quest Report Format (`quests_list.txt`):

```
==============================================================================
Fallout Fission - Quest System Report
==============================================================================
Generated IDs for mod quests and their message IDs.

Quest ID Range: 200-999 (mod quests)
Message ID Range: 32768-65535 (stable hash-based)
==============================================================================

MOD QUESTS:
Quest ID | Mod        | Description Message ID | Quest Key
----------------------------------------------------------------
200      | myquest    | 34120                  | myquest:0
201      | myquest    | 34121                  | myquest:1

MOD QUEST DETAILS:
-----------------
Quest 200: myquest:0
  Mod: myquest
  Message ID: Desc=34120
  Game Data: Location=1500, GVAR=79
  Thresholds: Display=1, Complete=2

SUMMARY:
Total Mod Quests: 2
Base Quests: 150
```

### Message Report Format (`messages_quests_list.txt`):

```
==============================================================================
Fallout Fission - QUESTS Messages
==============================================================================
Generated IDs for mod message references in scripts.

Message ID Range: 32768-65535 (stable hash-based)
Usage: display_msg(ID);  // Reference in scripts
==============================================================================

MOD MESSAGES (Custom Content):
ID      | Text Preview
--------|--------------------------------------------------
34120   | The people of Scraptown are having trouble with raiders...
34121   | Mayor Johnson's family heirloom was lost...

SUMMARY:
Total Mod Messages: 2
Base Messages: 502
```

### Error Detection:

-   Slot collisions: Clear popup error with resolution steps

-   Missing quest messages: Quest shows "Error" in PipBoy

-   Format errors: Reports malformed quest lines

-   Hash inconsistencies: System ensures same hash algorithm used throughout

-   DOS 8.3 violations: Warning popup for map names >8 characters

-   Missing area references: Warns when `city_name` doesn't match any area

* * * * *

9\. Modder Guidelines
---------------------

### Step-by-Step Complete Mod Creation:

1.  Choose a mod name (e.g., `myquest`)

2.  Create area file (`city_myquest.txt`):

    [Area 0]
    area_name = MYTOWN
    world_pos = 400,300
    start_state = On
    size = Medium
    entrance_0 = On,100,200,MYTOWN1,-1,-1,0

3.  Create map file (`maps_myquest.txt`):

    [Map 0]
    lookup_name = MYTOWN1
    map_name = mytown1      # ≤8 characters!
    city_name = MYTOWN      # Must match area_name
    saved = Yes
    automap = yes

4.  Create quest file (`quests_myquest.txt`):

    # location, description, gvar, displayThreshold, completedThreshold
    1500, 0, 79, 1, 2
    1500, 0, 80, 1, 3

5.  Create message file (`messages_myquest.txt`):

    [map]
    area_name:MYTOWN = My New Town
    lookup_name:MYTOWN1:0 = Town Square

    [worldmap]
    entrance_0:MYTOWN = Main Entrance

    [quests]
    quest:0 = Find the hidden artifact
    quest:1 = Return to the elder

6.  Place all files in correct directories

7.  Run the game - check generated reports for IDs

8.  Use IDs in scripts:

    javascript

    // Display area name
    display_msg(34120);

    // Set quest state
    op_set_quest(200, 1);

    // Display quest description
    display_msg(34125);

### Testing Checklist:

#### Areas & Maps:

-   Area appears on world map with correct name

-   Map can be entered and saves work

-   Automap shows correct map name

-   Town map shows correct entrance labels

#### Quests:

-   Quest appears in PipBoy with correct description

-   Quest state changes work (`op_set_quest/op_get_quest`)

-   Quest completes at correct threshold

-   Generated IDs match between quest and message reports

-   No "Error" text in quest descriptions

#### General:

-   Generated reports contain all expected IDs

-   Save games load correctly

-   No popup warnings during loading

* * * * *

10\. Known Limitations & Workarounds
------------------------------------

### Quest-Specific Limitations:

1.  Description Field Override:

    -   Problem: Vanilla quests use description field as message ID offset

    -   Solution: Mod quests override with generated message IDs

    -   Workaround: Always use `[quests]` section in message files

2.  Quest Location Validation:

    -   Problem: Quest location must reference valid area index

    -   Solution: System doesn't validate location references

    -   Workaround: Test thoroughly, check `area_list.txt` for valid indices

3.  GVAR Conflicts:

    -   Problem: Multiple mods could use same GVAR

    -   Solution: No automatic GVAR allocation

    -   Workaround: Document GVAR usage in mod readme

### General Limitations:

1.  DOS 8.3 Filename Limitation:

    -   Problem: `map_name` >8 characters breaks save games

    -   Workaround: Always use ≤8 character map names

    -   System: Warning popup alerts modders

2.  Save Game Compatibility:

    -   Problem: Adding fields to structs breaks old saves

    -   Solution: Use parallel arrays instead of modifying structs

    -   Implementation: `gAreaModNames` array separate from `CityInfo`

3.  Case Sensitivity:

    -   Issue: Different file systems handle case differently

    -   Solution: System normalizes everything to lowercase

    -   Recommendation: Use uppercase in configs for readability

4.  Hash Collisions:

    -   Issue: Different mods/areas could hash to same slot

    -   Handling: Fail with clear error message and resolution steps

    -   Resolution: Rename mod file to change namespace

5.  Loading Order Dependencies:

    -   Issue: Areas must load before maps that reference them

    -   Solution: Fixed loading order in `wmConfigInit()`

    -   Implementation: Areas load, then maps, then validate links

6.  Quest-Message Link Validation:

    -   Issue: No automatic check that quest messages exist

    -   Handling: Quest shows "Error" if message missing

    -   Resolution: Ensure `[quests]` section exists with correct keys

* * * * *

11\. Quest System Specifics
---------------------------

### Architecture Summary:

1.  Dual ID System:

    -   Quest ID (200-999): For `op_set_quest/op_get_quest` operations

    -   Message ID (32768-65535): For quest descriptions via `display_msg()`

2.  Automatic Linking:

    text

    Quest loading → generates message ID → stores in quest struct
         ↓
    Message loading → same hash → same ID → stores text
         ↓
    PipBoy display → retrieves text via stored message ID

3.  Backward Compatibility:

    -   Vanilla quests: Use original description field as message offset

    -   Mod quests: Override description field with generated message ID

    -   Single code path handles both transparently

### Modder Workflow:

```
Modder creates:                     System generates:          Modder uses in scripts:
----------------                    ------------------         -----------------------
quests_MyMod.txt  --hash-->         Quest ID: 200              op_set_quest(200, 1)
                                    Message ID: 34120          display_msg(34120)
                                    |
messages_MyMod.txt --same-hash->    Message ID: 34120
  [quests]                          |
  quest:0 = "Find item"             └── Auto-linked!
```

### Critical Notes for Modders:

1.  Key format is critical: `quest:0` not `QUEST:0` (lowercase!)

2.  Index is per-mod: First quest in file = `quest:0`, second = `quest:1`, etc.

3.  Message IDs are stable: Same mod+key = same ID every time

4.  Check reports: Always verify generated IDs match between quests and messages

### Integration Points:

1.  Script Engine: Uses quest IDs (200-999) for state management

2.  PipBoy UI: Uses message IDs (32768-65535) for description display

3.  Save System: Quest states saved via engine's existing mechanism

4.  Report System: Cross-references quests with their message IDs

* * * * *

12\. Troubleshooting
--------------------

### Common Issues:

| Problem | Likely Cause | Solution |
| --- | --- | --- |
| Quest shows "Error" | Missing `[quests]` section or wrong key | Ensure `messages_*.txt` has `[quests]` section with `quest:0` (lowercase) |
| Quest not in PipBoy | Invalid location or thresholds | Check location is valid area, thresholds are correct |
| Wrong description | Key mismatch (case or index) | Use exact format: lowercase `quest:{index}` |
| Quest ID collisions | Hash collision with another mod | Rename mod file to change namespace |
| Area not on world map | `area_name` doesn't match `city_name` | Ensure exact match (case-insensitive but be consistent) |
| Map name shows "Error" | Message key doesn't match generated ID | Check `messages_*.txt` format matches key generation |
| Save games fail | `map_name` >8 characters | Shorten map_name to ≤8 chars |
| Town map labels missing | Wrong section or key format | Use `[worldmap]` section with `entrance_X:{AREA_NAME}` |
| Mod not loading | File naming mismatch | All files must share same mod name prefix |

### Debug Commands:

Add these to `config.ini` under `[debug]`:


[debug]
write_offsets=1          # Write default town map offsets
generate_reports=1       # Force report generation
show_mod_loading=1       # Show mod loading popups
quest_debug=1            # Log quest loading details

### Debugging Steps:

1.  Check generated reports in `data/lists/`

2.  Verify hash consistency - same input should produce same output

3.  Test minimal mod - create simplest possible mod to isolate issues

4.  Check file permissions - ensure game can write to `data/lists/`

5.  Validate file formats - use text editor with visible whitespace

* * * * *

13\. Future Expansion
---------------------

### Planned Enhancements:

1.  Mod load order control - `mod_load_order.txt` file

2.  Dependency system - Mods requiring other mods

3.  Enhanced validation - Pre-flight check of mod files

4.  Batch conversion tools - Update old-format mods

5.  Extended ranges - More slots if community needs them

### Quest System Enhancements:

1.  Quest Dependency Tracking

2.  Quest Objective System (multiple steps per quest)

3.  Quest Reward Integration (auto-item granting)

4.  Quest Journal Enhancements (more detailed tracking)

### Integration Opportunities:

-   Scripts: Could auto-generate quest script templates

-   Dialogs: Link dialog options to quest states

-   Items: Quest item tracking and validation

-   Proto System: Extend to items, critters, scenery

### Development Tools Wishlist:

1.  ID Calculator Tool - Pre-calculate IDs before mod creation

2.  Mod Validator - Standalone tool to check mod files

3.  Template Generator - Create starter mod structure

4.  Conflict Detector - Check for conflicts between multiple mods

* * * * *

Appendix A: Quick Reference
---------------------------

### File Naming:

-   Areas: `city_{modname}.txt`

-   Maps: `maps_{modname}.txt`

-   Quests: `quests_{modname}.txt`

-   Messages: `messages_{modname}.txt`

### Key Formats:

-   Area names: `area_name:{AREA_NAME}`

-   Map names: `lookup_name:{LOOKUP_NAME}:{ELEVATION}`

-   Entrance labels: `entrance_{INDEX}:{AREA_NAME}`

-   Quest descriptions: `quest:{INDEX}`

### ID Ranges:

-   Quests: 200-999

-   Maps: 200-1999

-   Areas: 200-999

-   Messages: 32768-65535

### Critical Rules:

1.  Map names ≤8 characters

2.  Quest keys lowercase: `quest:0`

3.  Area names uppercase in configs

4.  Consistent mod name across all files

5.  Message keys use exact lowercase prefixes: `area_name:`, `lookup_name:`, `entrance_`

* * * * *

Appendix B: Example Mod Structure
---------------------------------

```
MyFirstMod/
├── data/
│   ├── city_myfirst.txt
│   ├── maps_myfirst.txt
│   ├── quests_myfirst.txt
├── text/
│   └── english/
│       └── game/
│           └── messages_myfirst.txt
├── art/
│   └── intrface/
│       ├── mod_myfirst.lst
│       └── myasset.frm
├── scripts/
│   ├── scripts_myfirst.lst
│   └── myscript.int
└── README.txt
```

### Minimal Working Example:

city_myfirst.txt:

```
[Area 0]
area_name = TESTZONE
world_pos = 100,100
start_state = On
size = Small
entrance_0 = On,50,50,TESTMAP1,-1,-1,0
```

maps_myfirst.txt:

```
[Map 0]
lookup_name = TESTMAP1
map_name = testmap1
city_name = TESTZONE
saved = Yes
```

quests_myfirst.txt:

```
# Find the test item
200, 0, 99, 1, 2
```

messages_myfirst.txt:

```
[map]
area_name:TESTZONE = Test Zone
lookup_name:TESTMAP1:0 = Test Location

[quests]
quest:0 = Find the test item and return it
```
