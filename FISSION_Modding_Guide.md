Fallout FISSION Modding System: Complete Implementation Guide
==========================================================

Table of Contents
-----------------

1.  System Overview

2.  Core Architecture

3.  File Structure & Naming Conventions

4.  Configuration Files

5.  Message System

6.  Stable ID Generation

7.  Implementation Details

8.  Debugging & Reports

9.  Modder Guidelines

10. Known Limitations & Workarounds

* * * * *

System Overview
---------------

The Fallout 2 FISSION modding system provides a framework for adding new maps, areas, and text content without modifying base game files. Key features:

-   Stable, hash-based indexing ensures consistent IDs across installations

-   Modular file organization keeps mods self-contained

-   Automatic report generation helps modders reference generated IDs

-   Backward compatibility with vanilla saves and content

-   Localization support with English fallback

### Supported Content Types:

1.  Maps - Game locations with multiple elevations

2.  Areas - World map regions containing multiple maps

3.  Messages - Text for area names, map names, and entrance labels

4.  Scripts - Already has stable ID generation (separate system)

5.  Art - Already has stable ID generation (separate system)

* * * * *

Core Architecture
-----------------

### ID Ranges:

text

Maps:      0-199     (Vanilla)
           200-1999  (Mods - hash-based allocation)

Areas:     0-199     (Vanilla)
           200-999   (Mods - hash-based allocation)

Messages:  0-32767   (Vanilla)
           32768-65535 (Mods - hash-based allocation)

### Key Principles:

1.  Deterministic hashing - Same inputs always produce same outputs

2.  Case-insensitive - System normalizes to lowercase

3.  Fail on collision - Prevents silent overwrites

4.  Separate concerns - Config, art, scripts, messages in different files

* * * * *

File Structure & Naming Conventions
-----------------------------------

### Required Files for a Mod:

text

data/
├── city_{modname}.txt          # Area definitions
├── maps_{modname}.txt          # Map definitions
├── text/
│   └── english/
│       └── game/
│           └── messages_{modname}.txt  # All text content
└── scripts/                    # (Existing system)
└── art/                       # (Existing system)

### File Naming Rules:

-   All lowercase recommended (system is case-insensitive but consistent)

-   No spaces or special characters except underscore

-   Mod name must be consistent across all files

-   Example: `city_myquest.txt`, `maps_myquest.txt`, `messages_myquest.txt`

* * * * *

Configuration Files
-------------------

### 1\. Area Configuration (`city_{modname}.txt`)

Defines world map areas. Each area can contain multiple maps.

#### Format:

ini

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

#### Rules:

-   `area_name` must be unique across all mods

-   Use uppercase for consistency

-   Maximum 8 characters recommended (not enforced but good practice)

* * * * *

### 2\. Map Configuration (`maps_{modname}.txt`)

Defines individual game maps.

#### Format:

ini

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

#### Critical Rules:

1.  `map_name` ≤ 8 characters - DOS 8.3 limitation for save games

2.  `city_name` must match an `area_name` in the corresponding city file

3.  `lookup_name` must be unique across all mods

* * * * *

Message System
--------------

### File Structure (`messages_{modname}.txt`)

Contains all text for your mod, organized by section.

#### Format:

ini

[map]                           # World map and automap text
AREA:SCRAPTOWN = Scraptown      # Area display name
MAP:SCRAPTOWN1:0 = Main Gate    # Map + elevation display name
MAP:SCRAPTOWN1:1 = Upper Level  # Different elevation, same map
MAP:SCRAPTOWN2:0 = Back Alley

[worldmap]                      # Town map entrance labels
ENTRANCE:SCRAPTOWN:0 = Main Town Gate
ENTRANCE:SCRAPTOWN:1 = Sewer Entrance
ENTRANCE:SCRAPTOWN:2 = Back Door

#### Key Format Rules:

1.  Section headers: `[map]` for world map, `[worldmap]` for town maps

2.  No trailing spaces in section headers

3.  Case-insensitive but use uppercase for keys for readability

4.  Key formats:

    -   Areas: `AREA:{AREA_NAME}`

    -   Maps: `MAP:{LOOKUP_NAME}:{ELEVATION}`

    -   Entrances: `ENTRANCE:{AREA_NAME}:{INDEX}`

#### ID Generation:

Message IDs are generated using:

cpp

generate_mod_message_id("modname", "AREA:SCRAPTOWN")
// Returns consistent ID between 32768-65535

* * * * *

Stable ID Generation
--------------------

### Hash Functions:

cpp

// DJB2 hash (case-insensitive)
uint32_t stable_hash(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        c = tolower(c);  // Case normalization
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Mod message ID generator
uint32_t generate_mod_message_id(const char* mod_name, const char* key) {
    char composite_key[256];
    snprintf(composite_key, sizeof(composite_key), "%s:%s", mod_name, key);
    uint32_t hash = stable_hash(composite_key);
    return 0x8000 + (hash % 0x7FFF);  // 32768-65535 range
}

### Area Slot Allocation:

cpp

static uint16_t wmAreaCalculateModSlot(const char* areaName, uint32_t modNamespace) {
    char combinedKey[256];
    snprintf(combinedKey, sizeof(combinedKey), "%s|%u", areaName, modNamespace);
    // Hash and map to 200-999 range
}

### Map Slot Allocation:

cpp

static uint16_t wmCalculateModMapSlot(const char* lookupName, uint32_t modNamespace) {
    char combinedKey[256];
    snprintf(combinedKey, sizeof(combinedKey), "%s|%u", lookupName, modNamespace);
    // Hash and map to 200-1999 range
}

* * * * *

Implementation Details
----------------------

### 1\. Area Loading (`worldmap.cc`)

-   File: `wmAreaLoadModFile()`

-   Process:

    1.  Extract mod name from filename (`city_myquest.txt` → `"myquest"`)

    2.  Calculate mod namespace hash

    3.  For each area section:

        -   Calculate stable slot based on area name + mod namespace

        -   Store mod name in parallel array (not in CityInfo struct)

        -   Generate area message ID: `generate_mod_message_id("myquest", "AREA:AREA_NAME")`

        -   Set `city->areaId` to generated message ID

### 2\. Map Loading (`worldmap.cc`)

-   File: `wmMapLoadModFile()`

-   Process:

    1.  Extract mod name from filename

    2.  For each map section:

        -   Calculate stable slot based on lookup name + mod namespace

        -   Use `city_name` field to link to area (not integer `city` field)

        -   Look up area by name and assign map to correct area index

### 3\. Message Loading (`message.cc`)

-   File: `loadModFileWithSections()`

-   Process:

    1.  Extract mod name from filename

    2.  Read sections `[map]` and `[worldmap]` separately

    3.  For each key-value pair:

        -   Generate message ID using mod name and key

        -   Add to appropriate message list

### 4\. Runtime Name Resolution

-   Area names: `mapGetCityName()` → uses area's stored message ID

-   Map names: `mapGetName()` → generates `"MAP:LOOKUP:ELEVATION"` key

-   Town map labels: `wmTownMapRefresh()` → generates `"ENTRANCE:AREA:INDEX"` key

* * * * *

Debugging & Reports
-------------------

### Generated Reports:

The system automatically creates these files in the game directory:

1.  `maps_list.txt` - All maps with slots, types, and override info

2.  `area_list.txt` - All areas with slots and mod assignments

3.  `messages_map_list.txt` - All mod messages in map.msg with IDs

4.  `messages_worldmap_list.txt` - All mod messages in worldmap.msg with IDs

5.  `scripts_list.txt` - Existing system (not covered here)

6.  `art_list.txt` - Existing system (not covered here)

### Report Format Example:

text

==============================================================================
Fallout Fission - MAP Messages
==============================================================================
Generated IDs for mod message references in scripts.

Message ID Range: 32768-65535 (stable hash-based)
Usage: display_msg(ID);  // Reference in scripts
==============================================================================

MOD MESSAGES (Custom Content):
ID      | Text Preview
--------|--------------------------------------------------
34120   | Scraptown
38464   | Backdoor
40752   | Secret Exit

SUMMARY:
Total Mod Messages: 7
Base Messages: 502

### Error Detection:

-   Slot collisions: Shows clear error with resolution steps

-   Long map names: Warning popup for >8 character `map_name`

-   Missing area references: Warns when `city_name` doesn't match any area

-   Section errors: Reports malformed section headers

* * * * *

Modder Guidelines
-----------------

### Step-by-Step Mod Creation:

1.  Choose a mod name (e.g., `myquest`)

2.  Create area file (`city_myquest.txt`)

    ini

    [Area 0]
    area_name = MYTOWN
    world_pos = 400,300
    start_state = On
    size = Medium
    entrance_0 = On,100,200,MYTOWN1,-1,-1,0

3.  Create map file (`maps_myquest.txt`)

    ini

    [Map 0]
    lookup_name = MYTOWN1
    map_name = mytown1      # ≤8 characters!
    city_name = MYTOWN      # Must match area_name
    saved = Yes
    automap = yes

4.  Create message file (`data/text/english/game/messages_myquest.txt`)

    ini

    [map]
    AREA:MYTOWN = My New Town
    MAP:MYTOWN1:0 = Town Square

    [worldmap]
    ENTRANCE:MYTOWN:0 = Main Entrance

5.  Place all files in correct directories

6.  Run the game - check generated reports for IDs

7.  Use IDs in scripts (e.g., `display_msg(34120)`)

### Testing Checklist:

-   Area appears on world map with correct name

-   Map can be entered and saves work

-   Automap shows correct map name

-   Town map shows correct entrance labels

-   Generated reports contain all expected IDs

-   Save games load correctly

* * * * *

Known Limitations & Workarounds
-------------------------------

### 1\. DOS 8.3 Filename Limitation

-   Problem: `map_name` >8 characters breaks save games

-   Workaround: Always use ≤8 character map names

-   System: Warning popup alerts modders

### 2\. Save Game Compatibility

-   Problem: Adding fields to structs breaks old saves

-   Solution: Use parallel arrays instead of modifying structs

-   Implementation: `gAreaModNames` array separate from `CityInfo`

### 3\. Case Sensitivity

-   Issue: Different file systems handle case differently

-   Solution: System normalizes everything to lowercase

-   Recommendation: Use uppercase in configs for readability

### 4\. Hash Collisions

-   Issue: Different mods/areas could hash to same slot

-   Handling: Fail with clear error message and resolution steps

-   Resolution: Rename mod file to change namespace

### 5\. Loading Order Dependencies

-   Issue: Areas must load before maps that reference them

-   Solution: Fixed loading order in `wmConfigInit()`

-   Implementation: Areas load, then maps, then validate links

* * * * *

Future Expansion Points
-----------------------

### Planned Enhancements:

1.  Mod load order control - `mod_load_order.txt` file

2.  Dependency system - Mods requiring other mods

3.  Enhanced validation - Pre-flight check of mod files

4.  Batch conversion tools - Update old-format mods

5.  Extended ranges - More slots if community needs them

### Integration with Existing Systems:

-   Scripts: Already has stable ID generation (cross-reference)

-   Art: Already has stable ID generation (cross-reference)

-   Dialogs: Future expansion could use same message system

-   Items: Future expansion possible

* * * * *

Troubleshooting
---------------

### Common Issues:

| Problem | Likely Cause | Solution |
| --- | --- | --- |
| Area not on world map | `area_name` doesn't match `city_name` | Ensure exact match (case-insensitive but be consistent) |
| Map name shows "Error" | Message key doesn't match generated ID | Check `messages_*.txt` format matches key generation |
| Save games fail | `map_name` >8 characters | Shorten map_name to ≤8 chars |
| Town map labels missing | Wrong section or key format | Use `[worldmap]` section with `ENTRANCE:AREA:INDEX` |
| Mod not loading | File naming mismatch | All files must share same mod name prefix |

### Debug Commands:

Add these to `config.ini` under `[debug]`:

ini

write_offsets=1          # Write default town map offsets
generate_reports=1       # Force report generation
show_mod_loading=1       # Show mod loading popups

* * * * *