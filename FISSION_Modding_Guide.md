Fallout FISSION Modding System: Complete Implementation Guide
=============================================================

## Table of Contents

1. [System Overview](#1-system-overview)
   - [1.1 Key Features](#11-key-features)
   - [1.2 Supported Content Types](#12-supported-content-types)

2. [Core Architecture](#2-core-architecture)
   - [2.1 ID Ranges](#21-id-ranges)
   - [2.2 Key Principles](#22-key-principles)

3. [File Structure & Naming Conventions](#3-file-structure--naming-conventions)
   - [3.1 Complete Directory Structure](#31-complete-directory-structure)
   - [3.2 Required FISSION Files](#32-required-fission-files)
   - [3.3 File Naming Rules](#33-file-naming-rules)
   - [3.4 Important Notes](#34-important-notes)

4. [Configuration Files](#4-configuration-files)
   - [4.1 Area Configuration (`city_{modname}.txt`)](#41-area-configuration-city_modnametxt)
     - [4.1.1 Format](#411-format)
     - [4.1.2 Rules](#412-rules)
   - [4.2 Map Configuration (`maps_{modname}.txt`)](#42-map-configuration-maps_modnametxt)
     - [4.2.1 Format](#421-format)
     - [4.2.2 Critical Rules](#422-critical-rules)
   - [4.3 Quest Configuration (`quests_{modname}.txt`)](#43-quest-configuration-quests_modnametxt)
     - [4.3.1 Format](#431-format)
     - [4.3.2 Rules](#432-rules)
   - [4.4 Holodisk Configuration (`holodisk_{modname}.txt`)](#44-holodisk-configuration-holodisk_modnametxt)
     - [4.4.1 Format](#441-format)
     - [4.4.2 Rules](#442-rules)

5. [Message System](#5-message-system)
   - [5.1 File Structure (`messages_{modname}.txt`)](#51-file-structure-messages_modnametxt)
     - [5.1.1 Format](#511-format)
     - [5.1.2 Critical Format Rules](#512-critical-format-rules)
   - [5.2 Key Formats Explained](#52-key-formats-explained)
   - [5.3 ID Generation Examples](#53-id-generation-examples)
   - [5.4 Common Mistakes to Avoid](#54-common-mistakes-to-avoid)
   - [5.5 Validation Tips](#55-validation-tips)

6. [Stable ID Generation](#6-stable-id-generation)
   - [6.1 Unified Hash Function](#61-unified-hash-function)
   - [6.2 Mod Message ID Generator](#62-mod-message-id-generator)
   - [6.3 Quest-Specific ID Generation](#63-quest-specific-id-generation)
   - [6.4 Holodisk ID Generation](#64-holodisk-id-generation)
   - [6.5 Area Slot Allocation](#65-area-slot-allocation)
   - [6.6 Map Slot Allocation](#66-map-slot-allocation)
   - [6.7 Quest Slot Allocation](#67-quest-slot-allocation)

7. [Implementation Details](#7-implementation-details)
   - [7.1 Area Loading (`worldmap.cc`)](#71-area-loading-worldmapcc)
   - [7.2 Map Loading (`worldmap.cc`)](#72-map-loading-worldmapcc)
   - [7.3 Quest Loading (`pipboy.cc`)](#73-quest-loading-pipboycc)
   - [7.4 Holodisk Loading (`pipboy.cc`)](#74-holodisk-loading-pipboycc)
     - [7.4.1 Process Overview](#741-process-overview)
     - [7.4.2 Conversion Example](#742-conversion-example)
     - [7.4.3 Key Functions](#743-key-functions)
   - [7.5 Message Loading (`message.cc`)](#75-message-loading-messagecc)
   - [7.6 Runtime Retrieval](#76-runtime-retrieval)
     - [7.6.1 Quest Description Retrieval](#761-quest-description-retrieval)
     - [7.6.2 Map Name Retrieval](#762-map-name-retrieval)
     - [7.6.3 Holodisk Text Retrieval](#763-holodisk-text-retrieval)

8. [Debugging & Reports](#8-debugging--reports)
   - [8.1 Generated Reports](#81-generated-reports)
   - [8.2 Report Formats](#82-report-formats)
     - [8.2.1 Quest Report Format](#821-quest-report-format)
     - [8.2.2 Holodisk Report Format](#822-holodisk-report-format)
     - [8.2.3 Message Report Format](#823-message-report-format)
   - [8.3 Error Detection](#83-error-detection)

9. [Modder Guidelines](#9-modder-guidelines)
   - [9.1 Step-by-Step Mod Creation](#91-step-by-step-mod-creation)
   - [9.2 Testing Checklist](#92-testing-checklist)
     - [9.2.1 Areas & Maps](#921-areas--maps)
     - [9.2.2 Quests](#922-quests)
     - [9.2.3 Holodisks](#923-holodisks)
     - [9.2.4 General Testing](#924-general-testing)

10. [Known Limitations & Workarounds](#10-known-limitations--workarounds)
    - [10.1 Quest-Specific Limitations](#101-quest-specific-limitations)
    - [10.2 Holodisk-Specific Limitations](#102-holodisk-specific-limitations)
    - [10.3 General Limitations](#103-general-limitations)

11. [Quest System Specifics](#11-quest-system-specifics)
    - [11.1 Architecture Summary](#111-architecture-summary)
    - [11.2 Modder Workflow](#112-modder-workflow)
    - [11.3 Critical Notes for Modders](#113-critical-notes-for-modders)
    - [11.4 Integration Points](#114-integration-points)

12. [Holodisk System Specifics](#12-holodisk-system-specifics)
    - [12.1 Overview](#121-overview)
    - [12.2 File Structure](#122-file-structure)
      - [12.2.1 Holodisk Definition File](#1221-holodisk-definition-file)
      - [12.2.2 Message File Format](#1222-message-file-format)
    - [12.3 How It Works](#123-how-it-works)
      - [12.3.1 Loading Process](#1231-loading-process)
      - [12.3.2 Conversion Example](#1232-conversion-example)
    - [12.4 Script Usage](#124-script-usage)
    - [12.5 Special Notes](#125-special-notes)
      - [12.5.1 Empty Lines](#1251-empty-lines)
      - [12.5.2 Pagination](#1252-pagination)
      - [12.5.3 Localization](#1253-localization)
      - [12.5.4 GVAR Management](#1254-gvar-management)
    - [12.6 Complete Example Mod](#126-complete-example-mod)
    - [12.7 Troubleshooting Holodisks](#127-troubleshooting-holodisks)
    - [12.8 Best Practices](#128-best-practices)

13. [Proto System](#13-proto-system)
    - [13.1 Overview](#131-overview)
    - [13.2 File Structure](#132-file-structure)
      - [13.2.1 Proto Definition Files](#1321-proto-definition-files)
      - [13.2.2 List File Format](#1322-list-file-format)
      - [13.2.3 Message File Format for Protos](#1323-message-file-format-for-protos)
    - [13.3 PID Generation](#133-pid-generation)
      - [13.3.1 PID Format](#1331-pid-format)
      - [13.3.2 PID Ranges by Type](#1332-pid-ranges-by-type)
    - [13.4 PID Generation vs. .pro File Contents](#134-pid-generation-vs-pro-file-contents)
      - [13.4.1 How It Works](#1341-how-it-works)
      - [13.4.2 Why This Design](#1342-why-this-design)
      - [13.4.3 What This Means for Modders](#1343-what-this-means-for-modders)
      - [13.4.4 Example Workflow](#1344-example-workflow)
    - [13.5 Creating Mod Protos](#135-creating-mod-protos)
    - [13.6 Using Mod Protos in Scripts](#136-using-mod-protos-in-scripts)
      - [13.6.1 Creating Objects with Mod PIDs](#1361-creating-objects-with-mod-pids)
      - [13.6.2 Getting Proto Information](#1362-getting-proto-information)
      - [13.6.3 Checking Proto Properties](#1363-checking-proto-properties)
    - [13.7 Generated Reports](#137-generated-reports)
    - [13.8 Hash Collision Handling](#138-hash-collision-handling)
    - [13.9 Special Considerations](#139-special-considerations)
      - [13.9.1 Proto File Format](#1391-proto-file-format)
      - [13.9.2 Art Requirements](#1392-art-requirements)
      - [13.9.3 Message Loading](#1393-message-loading)
      - [13.9.4 Save Game Compatibility](#1394-save-game-compatibility)
    - [13.10 Complete Example](#1310-complete-example)
    - [13.11 Best Practices](#1311-best-practices)
    - [13.12 Troubleshooting Protos](#1312-troubleshooting-protos)
    - [13.13 Integration Points](#1313-integration-points)

14. [Troubleshooting](#14-troubleshooting)
    - [14.1 Common Issues](#141-common-issues)
    - [14.2 Debug Commands](#142-debug-commands)
    - [14.3 Debugging Steps](#143-debugging-steps)

15. [Future Expansion](#15-future-expansion)
    - [15.1 Planned Enhancements](#151-planned-enhancements)
    - [15.2 Quest System Enhancements](#152-quest-system-enhancements)
    - [15.3 Holodisk System Enhancements](#153-holodisk-system-enhancements)
    - [15.4 Integration Opportunities](#154-integration-opportunities)
    - [15.5 Development Tools Wishlist](#155-development-tools-wishlist)

16. [Appendix A: Quick Reference](#appendix-a-quick-reference)
    - [16.1 File Naming](#161-file-naming)
    - [16.2 Key Formats](#162-key-formats)
    - [16.3 ID Ranges](#163-id-ranges)
    - [16.4 Critical Rules](#164-critical-rules)

17. [Appendix B: Example Mod Structure](#appendix-b-example-mod-structure)
    - [17.1 Complete Example Mod](#171-complete-example-mod)
    - [17.2 Installation and Testing](#172-installation-and-testing)
    - [17.3 Notes for Modders](#173-notes-for-modders)
    - [17.4 Generated IDs Reference](#174-generated-ids-reference)

* * * * *

1\. System Overview
-------------------

### 1.1 Key Features

The Fallout 2 FISSION modding system provides a framework for adding new content without modifying base game files:

-   Stable, hash-based indexing ensures consistent IDs across installations

-   Modular file organization keeps mods self-contained

-   Automatic report generation helps modders reference generated IDs

-   Backward compatibility with vanilla saves and content

-   Localization support with English fallback

-   Vanilla format conversion - mod content converted to vanilla format at load time

### 1.2 Supported Content Types

1.  Maps - Game locations with multiple elevations

2.  Areas - World map regions containing multiple maps

3.  Messages - Text for area names, map names, entrance labels, quest descriptions, and holodisk text

4.  Quests - Quest definitions with automatically linked descriptions

5.  Holodisks - Holodisk definitions with multi-page text content

6.  Scripts - Already has stable ID generation (separate system)

7.  Art - Already has stable ID generation (separate system)

8.  Protos - Items, critters, scenery, walls, tiles, and misc objects

* * * * *

2\. Core Architecture
---------------------

### 2.1 ID Ranges

| Content Type | Vanilla Range | Mod Range |
| --- | --- | --- |
| Quests | 0-199 | 200-999 (hash-based) |
| Maps | 0-199 | 200-1999 (hash-based) |
| Areas | 0-199 | 200-999 (hash-based) |
| Holodisks | 0-? (by GVAR) | 50000+ (message IDs, blocks of 500) |
| Messages | 0-32767 | 32768-65535 (hash-based) |
| Protos | 0-199 | 200-16777215 (hash-based) |

### 2.2 Key Principles

1.  Deterministic hashing - Same inputs always produce same outputs

2.  Case-insensitive - System normalizes to lowercase via `tolower()`

3.  Fail on collision - Prevents silent overwrites (popup warnings)

4.  Separate concerns - Config, art, scripts, messages in different files

5.  Automatic linking - Quests auto-generate linked message IDs

6.  Vanilla conversion - Mod content converted to vanilla format at load time

* * * * *

3\. File Structure & Naming Conventions
---------------------------------------

### 3.1 Complete Directory Structure

```
Fallout 2 Game Directory/
├── data/                    # Core game data
│   ├── city_{modname}.txt   # Area definitions
│   ├── maps_{modname}.txt   # Map definitions
│   ├── quests_{modname}.txt # Quest definitions
│   └── holodisk_{modname}.txt # Holodisk definitions
├── text/                    # Localization files
│   └── english/
│       └── game/
│           └── messages_{modname}.txt # All text content
├── art/                     # Art assets (existing system)
├── scripts/                 # Script files (existing system)
├── proto/                   # Proto definitions
│   ├── items/               # Item protos
│   ├── critters/            # Critter protos
│   ├── scenery/             # Scenery protos
│   ├── walls/               # Wall protos
│   ├── tiles/               # Tile protos
│   └── misc/                # Misc object protos
├── lists/                   # Generated reports
└── ... (other game folders)
```

### 3.2 Required FISSION Files

For the FISSION modding system specifically, place these in the `data/` directory:

1.  `city_{modname}.txt` - Area definitions

2.  `maps_{modname}.txt` - Map definitions

3.  `quests_{modname}.txt` - Quest definitions

4.  `holodisk_{modname}.txt` - Holodisk definitions

5.  `messages_{modname}.txt` - Message/text content (in `data/text/english/game/`)

### 3.3 File Naming Rules

-   All lowercase recommended (system is case-insensitive but consistent)

-   No spaces or special characters except underscore

-   Mod name must be consistent across all files

Example: For a mod named "wasteland":

-   `data/city_wasteland.txt`

-   `data/maps_wasteland.txt`

-   `data/quests_wasteland.txt`

-   `data/holodisk_wasteland.txt`

-   `data/text/english/game/messages_wasteland.txt`

### 3.4 Important Notes

1.  Scripts and Art use their own stable ID generation systems (separate from FISSION)

2.  Message files must be in language-specific subdirectories:

    -   English: `data/text/english/game/messages_{modname}.txt`

    -   German: `data/text/german/game/messages_{modname}.txt`

    -   etc.

3.  Vanilla files remain unchanged - mod files are added alongside them

* * * * *

4\. Configuration Files
-----------------------

### 4.1 Area Configuration (`city_{modname}.txt`)

Defines world map areas. Each area can contain multiple maps.

#### 4.1.1 Format

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

#### 4.1.2 Rules

-   `area_name` must be unique across all mods

-   Use uppercase for consistency

-   Maximum 8 characters recommended (not enforced but good practice)

### 4.2 Map Configuration (`maps_{modname}.txt`)

Defines individual game maps.

#### 4.2.1 Format

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

#### 4.2.2 Critical Rules

1.  `map_name` ≤ 8 characters - DOS 8.3 limitation for save games

2.  `city_name` must match an `area_name` in the corresponding city file

3.  `lookup_name` must be unique across all mods

### 4.3 Quest Configuration (`quests_{modname}.txt`)

Defines quests using the vanilla `quests.txt` format with enhanced mod support.

#### 4.3.1 Format

```
# Format: location, description, gvar, displayThreshold, completedThreshold
# Note: 'description' field is IGNORED for mod quests - replaced by generated message ID
1500, 0, 79, 1, 2
1500, 0, 80, 1, 3
```

#### 4.3.2 Rules

1.  One quest per line in CSV format

2.  Comments start with `#` and are ignored

3.  Location - Area index (must be valid area, vanilla or mod)

4.  Description field - Ignored for mod quests (replaced by generated message ID)

5.  GVAR - Global variable tracking quest state

6.  Thresholds - Display and completion values

### 4.4 Holodisk Configuration (`holodisk_{modname}.txt`)

Defines holodisks by their GVAR (global variable) numbers.

#### 4.4.1 Format

```
900  # First holodisk - appears when GVAR 900 = 1
901  # Second holodisk - appears when GVAR 901 = 1
```

#### 4.4.2 Rules

-   One GVAR per line

-   Comments start with `#`

-   GVARs should be unique (900+ recommended for mods)

-   GVAR must be set to 1 to make holodisk appear in Pip-Boy

* * * * *

5\. Message System
------------------

### 5.1 File Structure (`messages_{modname}.txt`)

Contains all text for your mod, organized by section. Keys must match exactly as shown!

#### 5.1.1 Format

```
[map]                                   # World map and automap text
area_name:SCRAPTOWN = Scraptown         # Area display name
lookup_name:Scraptown1:0 = Scraptown Gate    # Map + elevation display name

[worldmap]                              # Town map entrance labels
entrance_0:SCRAPTOWN = Scraptown Gate

[quests]                                # Quest descriptions
quest:0 = The people of Scraptown are having trouble...

[pipboy]                                # Holodisk text
holodisk:0:name = Important Data Disk
holodisk:0:line:0 = This holodisk contains critical information
holodisk:0:line:1 = **END-PAR**  # Paragraph break
holodisk:0:line:2 = **END-DISK**  # Required
```

#### 5.1.2 Critical Format Rules

1.  Section headers must match exactly: `[map]`, `[worldmap]`, `[quests]`, `[PIPBOY]`

2.  Case-sensitive keys: Use exact formats shown (lowercase for area_name/lookup_name/entrance_X/holodisk)

3.  No trailing spaces in section headers or keys

### 5.2 Key Formats Explained

These formats match the vanilla message file structure for consistency:

-   Vanilla uses `area_name:` for area names

-   Vanilla uses `lookup_name:` for map names

-   Vanilla uses `entrance_X:` for town map labels

-   Extended with `[quests]` section for mod quests

-   Extended with `[PIPBOY]` section for mod holodisks

### 5.3 ID Generation Examples

```
// Area name message ID generation
generate_mod_message_id("myquest", "area_name:SCRAPTOWN")
// Returns consistent ID between 32768-65535

// Map name message ID generation
generate_mod_message_id("myquest", "lookup_name:Scraptown1:0")
// Returns consistent ID between 32768-65535
```

### 5.4 Common Mistakes to Avoid

-   WRONG: `AREA:SCRAPTOWN` (uppercase, wrong prefix)

-   WRONG: `MAP:SCRAPTOWN1:0` (wrong prefix)

-   CORRECT: `area_name:SCRAPTOWN`

-   CORRECT: `lookup_name:SCRAPTOWN1:0`

### 5.5 Validation Tips

1.  Check vanilla `messages.txt` for reference formats

2.  Use exact lowercase prefixes shown above

3.  Test with minimal mod to verify format

4.  Check generated reports - wrong formats won't appear

Important: The system looks for exact key formats. If your keys don't match, messages won't load and you'll get "Error" text in-game.

* * * * *

6\. Stable ID Generation
------------------------

### 6.1 Unified Hash Function (DJB2, Case-Insensitive)

```
uint32_t stable_hash(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        c = tolower(c);  // Critical: Case normalization
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }
    return hash;
}
```

### 6.2 Mod Message ID Generator

```
uint32_t generate_mod_message_id(const char* mod_name, const char* key) {
    char composite_key[256];
    snprintf(composite_key, sizeof(composite_key), "%s:%s", mod_name, key);
    uint32_t hash = stable_hash(composite_key);
    return 0x8000 + (hash % 0x7FFF);  // 32768-65535 range
}
```

### 6.3 Quest-Specific ID Generation

```
// In questLoadModFile():
char descKey[256];
snprintf(descKey, sizeof(descKey), "quest:%d", questIndexInThisMod);
int descMessageId = generate_mod_message_id(mod_name, descKey);
quest->description = descMessageId;  // Override file description
```

### 6.4 Holodisk ID Generation

```
// In convertAndLoadModHolodisk():
char nameKey[128];
snprintf(nameKey, sizeof(nameKey), "holodisk:%d:name", holodiskIndex);
int nameHashId = generate_mod_message_id(modName, nameKey);

char lineKey[128];
snprintf(lineKey, sizeof(lineKey), "holodisk:%d:line:%d", holodiskIndex, lineNum);
int lineHashId = generate_mod_message_id(modName, lineKey);
```

### 6.5 Area Slot Allocation

```
static uint16_t wmAreaCalculateModSlot(const char* areaName, uint32_t modNamespace) {
    char combinedKey[256];
    snprintf(combinedKey, sizeof(combinedKey), "%s|%u", areaName, modNamespace);
    uint32_t hash = stable_hash(combinedKey);
    return MOD_AREA_START + (hash % (MOD_AREA_MAX - MOD_AREA_START));
}
```

### 6.6 Map Slot Allocation

```
static uint16_t wmCalculateModMapSlot(const char* lookupName, uint32_t modNamespace) {
    char combinedKey[256];
    snprintf(combinedKey, sizeof(combinedKey), "%s|%u", lookupName, modNamespace);
    uint32_t hash = stable_hash(combinedKey);
    return MOD_MAP_START + (hash % (MOD_MAP_MAX - MOD_MAP_START));
}
```

### 6.7 Quest Slot Allocation

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

File: `wmAreaLoadModFile()`

Process:

1.  Extract mod name from filename (`city_myquest.txt` → `"myquest"`)

2.  Calculate mod namespace hash

3.  For each area section:

    -   Calculate stable slot based on area name + mod namespace

    -   Store mod name in parallel array (not in CityInfo struct)

    -   Generate area message ID: `generate_mod_message_id("myquest", "area_name:AREA_NAME")`

    -   Set `city->areaId` to generated message ID

### 7.2 Map Loading (`worldmap.cc`)

File: `wmMapLoadModFile()`

Process:

1.  Extract mod name from filename

2.  For each map section:

    -   Calculate stable slot based on lookup name + mod namespace

    -   Use `city_name` field to link to area (not integer `city` field)

    -   Look up area by name and assign map to correct area index

### 7.3 Quest Loading (`pipboy.cc`)

File: `questLoadModFile()`

Process:

1.  Extract mod name from filename (`quests_myquest.txt` → `"myquest"`)

2.  For each quest line (indexed from 0 in mod file):

    -   Generate quest key: `"{modname}:{index}"`

    -   Calculate stable quest slot using `questCalculateModSlot()`

    -   Generate description message ID: `generate_mod_message_id("myquest", "quest:{index}")`

    -   Override description field with generated message ID

    -   Store mod name in `gQuestModNames[]` for tracking

### 7.4 Holodisk Loading (`pipboy.cc`)

#### 7.4.1 Process Overview

1.  Vanilla holodisks load from `data/holodisk.txt` (GVAR, name ID, description ID)

2.  Mod holodisks load from `data/holodisk_*.txt` files

3.  For each mod holodisk:

    -   Extract mod name from filename

    -   Generate a block of 500 consecutive IDs starting at 50000

    -   Look up name and lines from message file (in `[PIPBOY]` section)

    -   Add to message list with consecutive numeric IDs

    -   Add to holodisk array in vanilla format

#### 7.4.2 Conversion Example

```
holodisk_test2.txt (GVAR 900) + messages_test2.txt
↓
Converted to vanilla format:
- Name: ID 50000
- Line 0: ID 50001
- Line 1: ID 50002
- END-DISK: ID 50003
- Added to holodisk array: GVAR=900, name=50000, description=50001
```

#### 7.4.3 Key Functions

-   `holodiskInit()`: Main initialization, loads vanilla and mod holodisks

-   `holodiskLoadModFile()`: Loads a mod holodisk file

-   `convertAndLoadModHolodisk()`: Converts a mod holodisk to vanilla format

### 7.5 Message Loading (`message.cc`)

File: `loadModFileWithSections()`

Process for quests:

1.  Extract mod name from filename

2.  Read `[quests]` section

3.  For each `quest:{index} = text` pair:

    -   Generate same message ID using `generate_mod_message_id()`

    -   Add to quest message list (`gQuestsMessageList`)

Process for holodisks:

1.  Extract mod name from filename

2.  Read `[PIPBOY]` section (or `[HOLODISKS]` for backward compatibility)

3.  For each `holodisk:{index}:name = text` and `holodisk:{index}:line:{num} = text` pair:

    -   Generate message ID using `generate_mod_message_id()`

    -   Add to pipboy message list (`gPipboyMessageList`)

### 7.6 Runtime Retrieval

#### 7.6.1 Quest Description Retrieval

```
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

#### 7.6.2 Map Name Retrieval

```
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

#### 7.6.3 Holodisk Text Retrieval

```
static void pipboyRenderHolodiskText() {
    // NO MOD CHECKS NEEDED!
    // All holodisks (vanilla and mod) are in the same format

    // ... existing vanilla rendering code works for ALL holodisks ...

    // The bug is fixed because:
    // 1. Mod holodisks have consecutive IDs like vanilla
    // 2. gPipboyHolodiskLastPage is calculated correctly for all
    // 3. Pagination works identically
}
```

* * * * *

8\. Debugging & Reports
-----------------------

### 8.1 Generated Reports

The system automatically creates these files in `data/lists/`:

1.  `quests_list.txt` - All quests with slots, mod assignments, and generated message IDs

2.  `messages_quests_list.txt` - All mod quest descriptions with IDs

3.  `messages_pipboy_list.txt` - All mod messages in pipboy.msg with IDs (including holodisk messages)

4.  `holodisks_list.txt` - All holodisks with GVARs, name IDs, and mod assignments

5.  `maps_list.txt` - All maps with slots, types, and override info

6.  `area_list.txt` - All areas with slots and mod assignments

7.  `messages_map_list.txt` - All mod messages in map.msg with IDs

8.  `messages_worldmap_list.txt` - All mod messages in worldmap.msg with IDs

### 8.2 Report Formats

#### 8.2.1 Quest Report Format (`quests_list.txt`)

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

#### 8.2.2 Holodisk Report Format (`holodisks_list.txt`)

```
==============================================================================
Fallout 2 Fission - Holodisk System Report
==============================================================================
All holodisks (vanilla and mod) converted to vanilla format.

Format: Index | GVAR | Name ID | Desc ID | Type | Name
------------------------------------------------------------------------------

Summary: 11 total holodisks (9 vanilla, 2 mod)

    0 |   45 |     610 |     611 | Vanilla | Vault 13 Holodisk
    ...
   10 |  900 |   50000 |   50001 | MOD     | Test Holodisk 1
   11 |  901 |   50500 |   50501 | MOD     | Test Holodisk 2

MOD HOLODISK DETAILS:
====================
Mod holodisks use the following ID blocks:

Block 0: IDs 50000-50499 (GVAR 900) - Test Holodisk 1
Block 1: IDs 50500-50999 (GVAR 901) - Test Holodisk 2
```

#### 8.2.3 Message Report Format (`messages_pipboy_list.txt`)

```
==============================================================================
Fallout Fission - PIPBOY Messages
==============================================================================
Generated IDs for mod message references in scripts.

Message ID Range: 32768-65535 (stable hash-based)
Usage: display_msg(ID);  // Reference in scripts
==============================================================================

MOD MESSAGES (Custom Content):
ID      | Text Preview
--------|--------------------------------------------------
33685   | Another test holodisk with more content.
33686   | This demonstrates multiple holodisks per mod.
33687   | **END-PAR**
33688   | Paragraph breaks work too!
33689   | **END-DISK**
38279   | Test Holodisk 2
54832   | This is a test holodisk for the mod system.
54833   | It should appear in the Pip-Boy under Data.
54835   | This text was loaded from a mod file.
54836   | **END-DISK**
59451   | Test Holodisk 1

SUMMARY:
Total Mod Messages: 10
Base Messages: 626
```

### 8.3 Error Detection

-   Slot collisions: Clear popup error with resolution steps

-   Missing quest messages: Quest shows "Error" in PipBoy

-   Missing holodisk messages: Holodisk shows "Error" when clicked

-   Format errors: Reports malformed quest/holodisk lines

-   Hash inconsistencies: System ensures same hash algorithm used throughout

-   DOS 8.3 violations: Warning popup for map names >8 characters

-   Missing area references: Warns when `city_name` doesn't match any area

* * * * *

9\. Modder Guidelines
---------------------

### 9.1 Step-by-Step Mod Creation

1.  Choose a mod name (e.g., `myquest`)

2.  Create area file (`city_myquest.txt`):

    ```
    [Area 0]
    area_name = MYTOWN
    world_pos = 400,300
    start_state = On
    size = Medium
    entrance_0 = On,100,200,MYTOWN1,-1,-1,0
    ```

3.  Create map file (`maps_myquest.txt`):

    ```
    [Map 0]
    lookup_name = MYTOWN1
    map_name = mytown1      # ≤8 characters!
    city_name = MYTOWN      # Must match area_name
    saved = Yes
    automap = yes
    ```

4.  Create quest file (`quests_myquest.txt`):

    ```
    # location, description, gvar, displayThreshold, completedThreshold
    1500, 0, 79, 1, 2
    1500, 0, 80, 1, 3
    ```

5.  Create holodisk file (`holodisk_myquest.txt`):

    ```
    900
    901
    ```

6.  Create message file (`messages_myquest.txt`):

    ```
    [map]
    area_name:MYTOWN = My New Town
    lookup_name:MYTOWN1:0 = Town Square

    [worldmap]
    entrance_0:MYTOWN = Main Entrance

    [quests]
    quest:0 = Find the hidden artifact
    quest:1 = Return to the elder

    [PIPBOY]
    holodisk:0:name = Research Data
    holodisk:0:line:0 = Project: Nightfall
    holodisk:0:line:1 = Status: Active
    holodisk:0:line:2 = **END-PAR**
    holodisk:0:line:3 = Notes: Subjects show increased aggression.
    holodisk:0:line:4 = Recommend immediate termination.
    holodisk:0:line:5 = **END-DISK**

    holodisk:1:name = Security Log
    holodisk:1:line:0 = Unauthorized access detected.
    holodisk:1:line:1 = All systems on high alert.
    holodisk:1:line:2 = **END-DISK**
    ```

7.  Place all files in correct directories

8.  Run the game - check generated reports for IDs

9.  Use IDs in scripts:

    ```
    // Display area name
    display_msg(34120);

    // Set quest state
    op_set_quest(200, 1);

    // Display quest description
    display_msg(34125);

    // Give holodisk to player
    set_global_var(900, 1);
    display_msg("You found a holodisk!");
    ```

### 9.2 Testing Checklist

#### 9.2.1 Areas & Maps

-   Area appears on world map with correct name

-   Map can be entered and saves work

-   Automap shows correct map name

-   Town map shows correct entrance labels

#### 9.2.2 Quests

-   Quest appears in PipBoy with correct description

-   Quest state changes work (`op_set_quest/op_get_quest`)

-   Quest completes at correct threshold

-   Generated IDs match between quest and message reports

-   No "Error" text in quest descriptions

#### 9.2.3 Holodisks

-   Holodisk appears in Pip-Boy Data section when GVAR=1

-   Holodisk text displays correctly when clicked

-   Pagination works (Back/More buttons if needed)

-   END-PAR markers create paragraph breaks

-   END-DISK marker is present (auto-added if missing)

-   No "Error" text in holodisk content

#### 9.2.4 General Testing

-   Generated reports contain all expected IDs

-   Save games load correctly

-   No popup warnings during loading

* * * * *

10\. Known Limitations & Workarounds
------------------------------------

### 10.1 Quest-Specific Limitations

1.  Description Field Override:

    -   Problem: Vanilla quests use description field as message ID offset

    -   Solution: Mod quests override with generated message IDs

    -   Workaround: Always use `[quests]` section in message files

2.  Quest Location Validation:

    -   Problem: Quest location must reference valid area index

    -   Solution: System doesn't validate location references

    -   Workaround: Test thoroughly, check `area_list.txt` for valid indices

3.  GVAR Conflicts:

    -   Problem: Multiple mods could use same GVAR

    -   Solution: No automatic GVAR allocation

    -   Workaround: Document GVAR usage in mod readme

### 10.2 Holodisk-Specific Limitations

1.  Empty Lines Skipped:

    -   Problem: Message system skips empty values

    -   Solution: Use `**END-PAR**` for paragraph breaks

    -   Workaround: Add single space for truly blank lines (not recommended)

2.  GVAR Range:

    -   Problem: No reserved GVAR range for mods

    -   Solution: Use 900+ to avoid vanilla conflicts

    -   Workaround: Check vanilla GVAR list before choosing numbers

3.  Text Length:

    -   Problem: No hard limit, but very long holodisks may cause issues

    -   Solution: Use multiple holodisks for very long content

    -   Workaround: Split content across multiple holodisks

### 10.3 General Limitations

1.  DOS 8.3 Filename Limitation:

    -   Problem: `map_name` >8 characters breaks save games

    -   Workaround: Always use ≤8 character map names

    -   System: Warning popup alerts modders

2.  Save Game Compatibility:

    -   Problem: Adding fields to structs breaks old saves

    -   Solution: Use parallel arrays instead of modifying structs

    -   Implementation: `gAreaModNames` array separate from `CityInfo`

3.  Case Sensitivity:

    -   Issue: Different file systems handle case differently

    -   Solution: System normalizes everything to lowercase

    -   Recommendation: Use uppercase in configs for readability

4.  Hash Collisions:

    -   Issue: Different mods/areas could hash to same slot

    -   Handling: Fail with clear error message and resolution steps

    -   Resolution: Rename mod file to change namespace

5.  Loading Order Dependencies:

    -   Issue: Areas must load before maps that reference them

    -   Solution: Fixed loading order in `wmConfigInit()`

    -   Implementation: Areas load, then maps, then validate links

6.  Quest-Message Link Validation:

    -   Issue: No automatic check that quest messages exist

    -   Handling: Quest shows "Error" if message missing

    -   Resolution: Ensure `[quests]` section exists with correct keys

* * * * *

11\. Quest System Specifics
---------------------------

### 11.1 Architecture Summary

1.  Dual ID System:

    -   Quest ID (200-999): For `op_set_quest/op_get_quest` operations

    -   Message ID (32768-65535): For quest descriptions via `display_msg()`

2.  Automatic Linking:

    ```
    Quest loading → generates message ID → stores in quest struct
         ↓
    Message loading → same hash → same ID → stores text
         ↓
    PipBoy display → retrieves text via stored message ID
    ```

3.  Backward Compatibility:

    -   Vanilla quests: Use original description field as message offset

    -   Mod quests: Override description field with generated message ID

    -   Single code path handles both transparently

### 11.2 Modder Workflow

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

### 11.3 Critical Notes for Modders

1.  Key format is critical: `quest:0` not `QUEST:0` (lowercase!)

2.  Index is per-mod: First quest in file = `quest:0`, second = `quest:1`, etc.

3.  Message IDs are stable: Same mod+key = same ID every time

4.  Check reports: Always verify generated IDs match between quests and messages

### 11.4 Integration Points

1.  Script Engine: Uses quest IDs (200-999) for state management

2.  PipBoy UI: Uses message IDs (32768-65535) for description display

3.  Save System: Quest states saved via engine's existing mechanism

4.  Report System: Cross-references quests with their message IDs

* * * * *

12\. Holodisk System Specifics
------------------------------

### 12.1 Overview

The holodisk system allows modders to add new holodisks that are automatically converted to vanilla format at load time. Like other systems, it uses stable hashing and integrates seamlessly with the Pip-Boy interface.

### 12.2 File Structure

#### 12.2.1 Holodisk Definition File (`data/holodisk_{modname}.txt`)

Contains the GVAR (global variable) numbers for each holodisk. Each line should contain one GVAR number.

Format:

```
900  # First holodisk - appears when GVAR 900 = 1
901  # Second holodisk - appears when GVAR 901 = 1
```

Rules:

-   One GVAR per line

-   Comments start with `#`

-   GVARs should be unique (900+ recommended for mods)

-   GVAR must be set to 1 to make holodisk appear in Pip-Boy

#### 12.2.2 Message File Format

Contains the holodisk text in the `[PIPBOY]` section. The `[HOLODISKS]` section is also supported for backward compatibility.

Format:

```
[PIPBOY]
holodisk:0:name = Important Data Disk
holodisk:0:line:0 = This holodisk contains critical information
holodisk:0:line:1 = about the secret facility.
holodisk:0:line:2 = **END-PAR**  # Paragraph break (optional)
holodisk:0:line:3 = The entrance is hidden behind the waterfall.
holodisk:0:line:4 = Coordinates: 38.8977° N, 77.0365° W
holodisk:0:line:5 = **END-DISK**  # Required (auto-added if missing)
```

Key Formats:

-   Name: `holodisk:{index}:name`

-   Text lines: `holodisk:{index}:line:{line_number}`

-   END-DISK: Marks end of holodisk (required, auto-added if missing)

-   END-PAR: Creates paragraph break (optional)

-   Index starts at 0 for first holodisk in file

Important: Empty values are skipped. Use `**END-PAR**` for blank lines instead of empty strings.

### 12.3 How It Works

#### 12.3.1 Loading Process

1.  Vanilla holodisks load from `data/holodisk.txt` (GVAR, name ID, description ID)

2.  Mod holodisks load from `data/holodisk_*.txt` files

3.  For each mod holodisk:

    -   Extract mod name from filename

    -   Generate a block of 500 consecutive IDs starting at 50000

    -   Look up name and lines from message file

    -   Add to message list with consecutive numeric IDs

    -   Add to holodisk array in vanilla format

#### 12.3.2 Conversion Example

```
holodisk_test2.txt (GVAR 900) + messages_test2.txt
↓
Converted to vanilla format:
- Name: ID 50000
- Line 0: ID 50001
- Line 1: ID 50002
- END-DISK: ID 50003
- Added to holodisk array: GVAR=900, name=50000, description=50001
```

### 12.4 Script Usage

```
# Give holodisk to player
set_global_var(900, 1)
display_msg("You found a holodisk!")

# Check if player has holodisk
if global_var(900):
    display_msg("You already have this holodisk.")
```

### 12.5 Special Notes

#### 12.5.1 Empty Lines

The message loading system skips empty values. To create blank lines:

-   Use `**END-PAR**` for paragraph breaks

-   Or use a single space: `holodisk:0:line:2 =` (not recommended)

#### 12.5.2 Pagination

Holodisks use vanilla pagination (35 lines per page). The system automatically:

-   Calculates total pages

-   Adds Back/More buttons as needed

-   Works identically for all holodisks (vanilla and mod)

#### 12.5.3 Localization

Create message files in language folders:

-   English: `data/text/english/game/messages_{modname}.txt`

-   German: `data/text/german/game/messages_{modname}.txt`

-   etc.

#### 12.5.4 GVAR Management

-   GVARs must be unique across all mods

-   Recommended range: 900-999 for small mods, higher for larger mods

-   Set GVAR to 1 to give holodisk, 0 to remove (though typically not removed)

### 12.6 Complete Example Mod

File: `data/holodisk_myquest.txt`

```
900
901
```

File: `data/text/english/game/messages_myquest.txt`

```
[pipboy]
holodisk:0:name = Research Data
holodisk:0:line:0 = Project: Nightfall
holodisk:0:line:1 = Status: Active
holodisk:0:line:2 = **END-PAR**
holodisk:0:line:3 = Notes: Subjects show increased aggression.
holodisk:0:line:4 = Recommend immediate termination.
holodisk:0:line:5 = **END-DISK**

holodisk:1:name = Security Log
holodisk:1:line:0 = Unauthorized access detected.
holodisk:1:line:1 = All systems on high alert.
holodisk:1:line:2 = **END-DISK**
```

Script to give holodisks:

```
# In a map script
procedure map_enter_p_proc begin
    if not(global_var(900)) then
        set_global_var(900, 1)
        display_msg("Found research data holodisk.")
    end
end
```

### 12.7 Troubleshooting Holodisks

| Problem | Solution |
| --- | --- |
| Holodisk not appearing | Check GVAR is set to 1 before opening Pip-Boy |
| "Error" text when clicked | Verify message file has correct `[PIPBOY]` section |
| Missing lines | Ensure no empty values; use `**END-PAR**` for blanks |
| Wrong mod name | All files must share same mod name prefix |
| No END-DISK marker | System adds automatically, but include for clarity |
| GVAR conflict | Use unique GVARs (check vanilla GVAR list) |

### 12.8 Best Practices

1.  Test thoroughly: Set GVARs and verify holodisks appear

2.  Use meaningful GVARs: 900, 901, etc. for easy reference

3.  Include END-DISK: Even though auto-added, it's good practice

4.  Check reports: Verify holodisks appear in generated list

5.  Localize: Create message files for all supported languages

* * * * *

13\. Proto System
-----------------

### 13.1 Overview

The proto system allows modders to add new items, critters, scenery, walls, tiles, and miscellaneous objects using the same stable ID generation as other systems. All mod protos are automatically integrated into the game's existing proto management system.

### 13.2 File Structure

#### 13.2.1 Proto Definition Files

Mod protos use a two-file system:

```
Fallout 2 Game Directory/
├── proto/                    # Proto definitions
│   ├── items/               # Item proto directory
│   │   ├── items_{modname}.lst    # List of item protos
│   │   └── {protoname}.pro        # Individual proto files
│   ├── critters/            # Critter proto directory
│   │   ├── critters_{modname}.lst
│   │   └── {protoname}.pro
│   ├── scenery/             # Scenery proto directory
│   │   ├── scenery_{modname}.lst
│   │   └── {protoname}.pro
│   └── ... (walls, tiles, misc similarly)
```

#### 13.2.2 List File Format (`{type}_{modname}.lst`)

Each line contains one proto name. You can include or omit the .pro extension - both work:

items_testmod.lst:

```
# Format 1: Without extension (cleaner)
testitem
advancedgun
specialarmor

# Format 2: With extension (vanilla style - also works)
weapon.pro
armor.pro
drug.pro
```

critters_testmod.lst:

```
supermutant_elite
deathclaw_alpha
```

#### 13.2.3 Message File Format for Protos

Add proto messages to your existing `messages_{modname}.txt` file in a section containing "proto" or "pro_":

```
# In messages_testmod.txt
[proto_testmod]  # Any section containing 'proto' or 'pro_'
testmod:testitem:name = Test Mod Item
testmod:testitem:desc = A test item created by the mod system.
testmod:advancedgun:name = Advanced Plasma Rifle
testmod:advancedgun:desc = An upgraded plasma rifle with enhanced damage.

# Alternative format (using filename mod name):
testitem:name = Test Mod Item
testitem:desc = A test item created by the mod system.
```

Message Key Formats:

-   Full format: `{modname}:{protoname}:name` and `{modname}:{protoname}:desc`

-   Short format: `{protoname}:name` and `{protoname}:desc` (uses filename for mod name)

### 13.3 PID Generation

#### 13.3.1 PID Format

-   Vanilla protos: Use indices 0-199 (`0x000000-0x0000C7`) in lower 24 bits

-   Mod protos: Use indices 200-16777215 (`0x0000C8-0xFFFFFF`) via deterministic hashing

-   Full PID: `(type << 24) | index`

Example PID Calculation:

c

// For mod "testmod" with proto "testitem" of type ITEM (0x00)
Composite key: "testmod:testitem"
Hash: stable_hash("testmod:testitem") = 0xDB240E
Final PID: (ITEM_TYPE << 24) | 0xDB240E = 0x00DB240E

#### 13.3.2 PID Ranges by Type

-   Items: `0x00XXXXXX` (type 0)

-   Critters: `0x01XXXXXX` (type 1)

-   Scenery: `0x02XXXXXX` (type 2)

-   Walls: `0x03XXXXXX` (type 3)

-   Tiles: `0x04XXXXXX` (type 4)

-   Misc: `0x05XXXXXX` (type 5)

### 13.4 PID Generation vs. .pro File Contents

#### 13.4.1 How It Works

Important: The PID stored in a `.pro` file's first 4 bytes is ignored for mod protos. Our system generates stable PIDs based on mod and proto names.

1.  File Creation: When you create `testitem.pro` with mapper.exe, it might save with PID `0x00000001`

2.  Loading: Our system reads this file, but then:

    -   Generates a new PID: `0x00DB240E` (hash of `"testmod:testitem"`)

    -   Ignores the `0x00000001` from the file

    -   Uses `0x00DB240E` for all game operations

#### 13.4.2 Why This Design

1.  Stability: Same names = same PID across all installations

2.  No conflicts: Mod PIDs can't collide with vanilla PIDs

3.  Simplicity: Modders don't need to edit .pro file binary data

#### 13.4.3 What This Means for Modders

-   Don't worry about the PID in the .pro file

-   Do use the PIDs from `proto_list.txt`

-   Don't try to edit .pro file PIDs manually

-   Do use consistent mod/proto names

#### 13.4.4 Example Workflow

```
# 1. Create testitem.pro with mapper.exe (PID = 0x00000001 in file)
# 2. System loads it, generates PID = 0x00DB240E
# 3. Check proto_list.txt:
#    PID: 0x00DB240E | Mod: testmod | Proto: testitem
# 4. Use 0x00DB240E in scripts, not 0x00000001
```

Script Example - Correct vs. Wrong:

```
// WRONG - using .pro file's internal PID
int wrong_pid = 0x00000001;  // What mapper.exe saved
obj = create_object(wrong_pid, ...);  // Won't work as expected

// CORRECT - using generated PID from proto_list.txt
int correct_pid = 0x00DB240E;  // From proto_list.txt
obj = create_object(correct_pid, ...);  // Works perfectly
```

### 13.5 Creating Mod Protos

Step-by-Step Guide:

1.  Create .lst file for your proto type:

    ```
    # proto/items/items_mymod.lst
    mycustomgun
    specialammo
    uniquearmor
    ```

2.  Create .pro files for each proto (use mapper.exe or existing .pro files as templates):

    ```
    proto/items/mycustomgun.pro
    proto/items/specialammo.pro
    proto/items/uniquearmor.pro
    ```

3.  Add messages in your message file:

    ```
    [proto_mymod]
    mymod:mycustomgun:name = Custom Plasma Pistol
    mymod:mycustomgun:desc = A custom-made plasma pistol.
    mymod:specialammo:name = Enhanced Microfusion Cells
    mymod:specialammo:desc = High-capacity microfusion cells.
    ```

4.  Place files in correct directories

5.  Run game and check `data/lists/proto_list.txt` for generated PIDs

### 13.6 Using Mod Protos in Scripts

#### 13.6.1 Creating Objects with Mod PIDs

c

// Create a mod item at player's feet
int mod_pid = 0x00DB240E;  // From proto_list.txt
obj = create_object(mod_pid, dude_tile, dude_elevation);

// Add to inventory
add_mult_objs_to_inven(dude, obj, 1);

#### 13.6.2 Getting Proto Information

c

// Get proto name for display
char* item_name = protoGetName(0x00DB240E);
display_msg(item_name);

// Get proto description
char* item_desc = protoGetDescription(0x00DB240E);
display_msg(item_desc);

#### 13.6.3 Checking Proto Properties

c

// Check if item can be picked up
if (_proto_action_can_pickup(0x00DB240E)) {
    display_msg("You can pick this up");
}

// Check if item can be used
if (_proto_action_can_use(0x00DB240E)) {
    display_msg("You can use this item");
}

### 13.7 Generated Reports

The system generates `data/lists/proto_list.txt` with complete information:

```
==============================================================================
Fallout 2 FISSION - Mod Proto Report
==============================================================================

MOD PROTO STATISTICS:
---------------------
Total Mod Protos: 3

By Type:
  items   : 2
  critters: 1

ITEMS MOD PROTOS:
-----------------
  PID: 0x00DB240E (14394894)
    Mod:      testmod
    Proto:    testitem
    File:     testitem.pro
    Message:  ID 1
    Name:     Test Mod Item
    Desc:     A test item created by the mod system.
    Item Type: Misc Item
    Weight:    10, Cost: 0

NAME-TO-PID REGISTRY (for message file parsing):
------------------------------------------------
  testmod:testitem       -> 0x00DB240E
  testmod:advancedgun    -> 0x00F8A31C
```

### 13.8 Hash Collision Handling

If two different mods generate the same PID (hash collision), the system:

1.  Shows immediate popup warning with details of the collision

2.  Skips the colliding proto (it will not be loaded)

3.  Logs collision details in the report

Example collision warning:

```
HASH COLLISION WARNING!

Your mod 'mymod' proto 'mygun'
generated PID 0x00DB240E which is already used by:
Mod 'othermod' proto 'theirgun'

This proto will be skipped.
Rename your mod or proto to fix.
```

### 13.9 Special Considerations

#### 13.9.1 Proto File Format

-   .pro files must be in the exact same format as vanilla protos

-   Use mapper.exe to create or edit .pro files

-   Ensure FIDs (Frame IDs) point to existing art

#### 13.9.2 Art Requirements

-   Each proto must have valid art references

-   Art uses its own stable ID system (separate from proto system)

-   Ensure `art/` directory has corresponding .FRM files

#### 13.9.3 Message Loading

-   Proto messages load from the same message files as other content

-   Sections can be named `[proto_{modname}]`, `[pro_items]`, or any containing "proto" or "pro_"

-   Messages are linked automatically via the name-to-PID registry

#### 13.9.4 Save Game Compatibility

-   Mod protos are saved in the same format as vanilla protos

-   Object PIDs in save games reference the generated mod PID

-   Loading saves without the mod will cause missing object errors

### 13.10 Complete Example

File: `proto/items/items_wasteland.lst`

```
plasmacaster
ripper2
stimpak_advanced
```

File: `proto/items/plasmacaster.pro` (created with mapper.exe)

File: `text/english/game/messages_wasteland.txt`

```
[proto_wasteland]
wasteland:plasmacaster:name = Improved Plasma Caster
wasteland:plasmacaster:desc = A modified plasma caster with enhanced range.

wasteland:ripper2:name = Ripper Mk II
wasteland:ripper2:desc = An upgraded ripper with serrated blades.

wasteland:stimpak_advanced:name = Advanced Stimpak
wasteland:stimpak_advanced:desc = Heals more damage than standard stimpaks.
```

Script usage:

```
// Give player the mod item
int plasmacaster_pid = 0x00A1B2C3;  // From proto_list.txt
obj = create_object(plasmacaster_pid, dude_tile, dude_elevation);
add_mult_objs_to_inven(dude, obj, 1);

// Display item name
display_msg(protoGetName(plasmacaster_pid));
```

### 13.11 Best Practices

1.  Test thoroughly: Create objects with mod PIDs and verify they work

2.  Check reports: Always verify generated PIDs in proto_list.txt

3.  Unique names: Use unique mod and proto names to avoid hash collisions

4.  Backup saves: Test with new saves before using existing ones

5.  Art preparation: Ensure all art assets exist before creating protos

6.  Message testing: Verify proto names/descriptions display correctly

### 13.12 Troubleshooting Protos

| Problem | Solution |
| --- | --- |
| Proto shows "Error" name | Check message file has correct proto section and keys |
| Proto not appearing | Verify .lst and .pro files are in correct directories |
| PID collisions | Rename mod or proto to generate different hash |
| Missing art | Ensure art files exist and FIDs are correct |
| Save game errors | Test with new saves first; mod protos may not load without mod |
| Wrong item type | Verify .pro file has correct type byte |
| Message not loading | Ensure section contains "proto" or "pro_" |

### 13.13 Integration Points

1.  Object System: Mod protos work with all existing object functions

2.  Inventory: Can be added to inventory like vanilla items

3.  Combat: Weapon and armor protos work in combat system

4.  Scripts: All script functions accept mod proto PIDs

5.  Save/Load: Objects with mod PIDs save and load correctly

6.  UI: Proto names/descriptions display in all interfaces

* * * * *

14\. Troubleshooting
--------------------

### 14.1 Common Issues

| Problem | Likely Cause | Solution |
| --- | --- | --- |
| Quest shows "Error" | Missing `[quests]` section or wrong key | Ensure `messages_*.txt` has `[quests]` section with `quest:0` (lowercase) |
| Holodisk not in Pip-Boy | GVAR not set to 1 | Ensure `set_global_var(GVAR, 1)` is called before opening Pip-Boy |
| Holodisk shows "Error" when clicked | Missing `[PIPBOY]` section or wrong key | Check message file has `[PIPBOY]` section with correct keys |
| Quest not in PipBoy | Invalid location or thresholds | Check location is valid area, thresholds are correct |
| Wrong description | Key mismatch (case or index) | Use exact format: lowercase `quest:{index}` or `holodisk:{index}:name` |
| Quest ID collisions | Hash collision with another mod | Rename mod file to change namespace |
| Area not on world map | `area_name` doesn't match `city_name` | Ensure exact match (case-insensitive but be consistent) |
| Map name shows "Error" | Message key doesn't match generated ID | Check `messages_*.txt` format matches key generation |
| Save games fail | `map_name` >8 characters | Shorten map_name to ≤8 chars |
| Town map labels missing | Wrong section or key format | Use `[worldmap]` section with `entrance_X:{AREA_NAME}` |
| Mod not loading | File naming mismatch | All files must share same mod name prefix |
| Missing lines in holodisk | Empty values skipped | Use `**END-PAR**` for blank lines |
| Proto shows "Error" text | Missing message file or wrong key | Ensure message file has proto section with correct keys |
| Proto not loading | .lst or .pro file missing | Check files exist in correct proto subdirectory |
| PID collision warning | Hash conflict with another mod | Rename mod or proto to change hash |
| Wrong item stats | Incorrect .pro file data | Verify .pro file created/edited with mapper.exe |
| Art not displaying | Invalid FID in .pro file | Ensure art exists and FID points to correct file |
| Save game missing proto | Mod not installed when loading save | Always install mod before loading saves with mod content |

### 14.2 Debug Commands

Add these to `config.ini` under `[debug]`:

ini

[debug]
write_offsets=1          # Write default town map offsets
generate_reports=1       # Force report generation
show_mod_loading=1       # Show mod loading popups
quest_debug=1            # Log quest loading details
holodisk_debug=1         # Log holodisk loading details

### 14.3 Debugging Steps

1.  Check generated reports in `data/lists/`

2.  Verify hash consistency - same input should produce same output

3.  Test minimal mod - create simplest possible mod to isolate issues

4.  Check file permissions - ensure game can write to `data/lists/`

5.  Validate file formats - use text editor with visible whitespace

6.  Verify GVAR values - ensure GVARs are set to 1 before opening Pip-Boy

7.  Check loading order - messages must load before holodisks

* * * * *

15\. Future Expansion
---------------------

### 15.1 Planned Enhancements

1.  Mod load order control - `mod_load_order.txt` file

2.  Dependency system - Mods requiring other mods

3.  Enhanced validation - Pre-flight check of mod files

4.  Batch conversion tools - Update old-format mods

5.  Extended ranges - More slots if community needs them

### 15.2 Quest System Enhancements

1.  Quest Dependency Tracking

2.  Quest Objective System (multiple steps per quest)

3.  Quest Reward Integration (auto-item granting)

4.  Quest Journal Enhancements (more detailed tracking)

### 15.3 Holodisk System Enhancements

1.  Holodisk Categories - Organize by type (quest, lore, etc.)

2.  Holodisk Images - Support for custom holodisk art

3.  Holodisk Playback - Audio logs (if audio system extended)

4.  Holodisk Collection Tracking - Auto-tracking of found holodisks

### 15.4 Integration Opportunities

-   Scripts: Could auto-generate quest script templates

-   Dialogs: Link dialog options to quest states

-   Items: Quest item tracking and validation

-   Proto System: Extend to items, critters, scenery

### 15.5 Development Tools Wishlist

1.  ID Calculator Tool - Pre-calculate IDs before mod creation

2.  Mod Validator - Standalone tool to check mod files

3.  Template Generator - Create starter mod structure

4.  Conflict Detector - Check for conflicts between multiple mods

* * * * *

16\. Appendix A: Quick Reference
--------------------------------

### 16.1 File Naming

-   Areas: `city_{modname}.txt`

-   Maps: `maps_{modname}.txt`

-   Quests: `quests_{modname}.txt`

-   Holodisks: `holodisk_{modname}.txt`

-   Messages: `messages_{modname}.txt`

-   Proto lists: `proto/{type}_{modname}.lst`

-   Proto files: `proto/{type}/{protoname}.pro`

### 16.2 Key Formats

-   Area names: `area_name:{AREA_NAME}`

-   Map names: `lookup_name:{LOOKUP_NAME}:{ELEVATION}`

-   Entrance labels: `entrance_{INDEX}:{AREA_NAME}`

-   Quest descriptions: `quest:{INDEX}`

-   Holodisk names: `holodisk:{INDEX}:name`

-   Holodisk lines: `holodisk:{INDEX}:line:{LINE_NUMBER}`

-   Proto names: `{modname}:{protoname}:name` or `{protoname}:name`

-   Proto descriptions: `{modname}:{protoname}:desc` or `{protoname}:desc`

### 16.3 ID Ranges

-   Quests: 200-999

-   Maps: 200-1999

-   Areas: 200-999

-   Holodisks: 50000+ (message IDs, blocks of 500)

-   Messages: 32768-65535

-   Protos: Vanilla indices 0-199, Mod indices 200-16777215

-   Proto types: Items=0x00, Critters=0x01, Scenery=0x02, Walls=0x03, Tiles=0x04, Misc=0x05

### 16.4 Critical Rules

1.  Map names ≤8 characters

2.  Quest keys lowercase: `quest:0`

3.  Holodisk keys lowercase: `holodisk:0:name`

4.  Area names uppercase in configs

5.  Consistent mod name across all files

6.  Message keys use exact lowercase prefixes: `area_name:`, `lookup_name:`, `entrance_`, `holodisk:`

7.  .lst files must be in correct proto subdirectory

8.  .pro files must match names in .lst files exactly

9.  Message sections for protos must contain "proto" or "pro_"

* * * * *

17\. Appendix B: Example Mod Structure
--------------------------------------

### 17.1 Complete Example Mod

```
MyFirstMod/
├── data/
│   ├── city_myfirst.txt
│   ├── maps_myfirst.txt
│   ├── quests_myfirst.txt
│   └── holodisk_myfirst.txt
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
├── proto/
│   ├── items/
│   │   ├── items_myfirst.lst
│   │   ├── testitem.pro
│   │   └── supergun.pro
│   └── critters/
│       ├── critters_myfirst.lst
│       └── testcritter.pro
└── README.txt
```

Area Configuration (`data/city_myfirst.txt`):

```
[Area 0]
area_name = TESTZONE
world_pos = 100,100
start_state = On
size = Small
entrance_0 = On,50,50,TESTMAP1,-1,-1,0
```

Map Configuration (`data/maps_myfirst.txt`):

```
[Map 0]
lookup_name = TESTMAP1
map_name = testmap1      # ≤8 characters!
city_name = TESTZONE     # Must match area_name above
saved = Yes
automap = yes
```

Quest Configuration (`data/quests_myfirst.txt`):

```
# Format: location, description, gvar, displayThreshold, completedThreshold
# Note: description field is ignored - replaced by generated message ID
1500, 0, 79, 1, 2
1500, 0, 80, 1, 3
```

Holodisk Configuration (`data/holodisk_myfirst.txt`):

```
900  # Appears when GVAR 900 = 1
901  # Appears when GVAR 901 = 1
```

Message File (`text/english/game/messages_myfirst.txt`):

```
[map]
area_name:TESTZONE = Test Zone
lookup_name:TESTMAP1:0 = Test Location

[worldmap]
entrance_0:TESTZONE = Main Gate

[quests]
quest:0 = Find the hidden artifact
quest:1 = Return to the elder

[pipboy]
holodisk:0:name = Test Holodisk
holodisk:0:line:0 = This is a test holodisk.
holodisk:0:line:1 = **END-DISK**

holodisk:1:name = Second Holodisk
holodisk:1:line:0 = More test content here.
holodisk:1:line:1 = **END-DISK**

[proto_myfirst]
myfirst:testitem:name = Test Mod Item
myfirst:testitem:desc = A simple test item.
myfirst:specialgun:name = Special Gun
myfirst:specialgun:desc = An upgraded weapon.
myfirst:testcritter:name = Test Critter
myfirst:testcritter:desc = A modified creature.
```

Script Example (`scripts/myfirst.int`):

```
// Give holodisks to player
procedure start begin
    // Set GVARs to give holodisks
    set_global_var(900, 1);
    set_global_var(901, 1);

    // Create and give mod item
    int item_pid = 0x00DB240E;  // Check proto_list.txt for actual PID
    obj = create_object(item_pid, dude_tile, dude_elevation);
    add_obj_to_inven(dude, obj);

    // Display message
    display_msg("Test mod initialized!");
end

// Create mod critter on map
procedure map_enter_p_proc begin
    int critter_pid = 0x01C5B2A8;  // Check proto_list.txt for actual PID
    critter = create_object(critter_pid, 12345, 0);  // Some tile number
end

// Quest script example
procedure quest_test begin
    // Start quest (check quests_list.txt for quest ID)
    set_quest(200, 1);  // Quest ID 200, state 1 (active)

    // Display quest description
    int msg_id = 34120;  // Check messages_quests_list.txt for actual ID
    display_msg(msg_id);
end
```

### 17.2 Installation and Testing

1.  Copy all files to the correct directories in your Fallout 2 install

2.  Run the game and check these generated reports in `data/lists/`:

    -   `proto_list.txt` - Shows generated proto PIDs

    -   `quests_list.txt` - Shows quest IDs and message IDs

    -   `holodisks_list.txt` - Shows holodisk details

    -   `area_list.txt` - Shows area information

    -   `maps_list.txt` - Shows map information

3.  Test in-game:

    -   Travel to coordinates 100,100 on world map to see your area

    -   Enter the area to see your map

    -   Open Pip-Boy to see quests and holodisks

    -   Check inventory for the test item

### 17.3 Notes for Modders

1.  Replace example PIDs with actual IDs from your generated reports

2.  Use mapper.exe to create `.pro` files for protos

3.  Test with new saves before using existing saves

4.  Check all reports after loading to verify everything worked

5.  Use uppercase for area names in config files for consistency

6.  Remember map_name ≤8 characters (DOS 8.3 limitation)

### 17.4 Generated IDs Reference

After running with this mod, check these reports:

From `proto_list.txt`:

```
MOD PROTO STATISTICS:
---------------------
Total Mod Protos: 3

By Type:
  items   : 2
  critters: 1

ITEMS MOD PROTOS:
-----------------
  PID: 0x00DB240E (14394894)
    Mod:      myfirst
    Proto:    testitem
    Name:     Test Mod Item
    Desc:     A simple test item.

  PID: 0x00F8A31C (16302876)
    Mod:      myfirst
    Proto:    specialgun
    Name:     Special Gun
    Desc:     An upgraded weapon.
```

From `quests_list.txt`:

```
MOD QUESTS:
Quest ID | Mod      | Description Message ID | Quest Key
----------------------------------------------------------------
200      | myfirst  | 34120                  | myfirst:0
201      | myfirst  | 34121                  | myfirst:1
```

From `holodisks_list.txt`:

```
MOD HOLODISK DETAILS:
Block 0: IDs 50000-50499 (GVAR 900) - Test Holodisk
Block 1: IDs 50500-50999 (GVAR 901) - Second Holodisk
```

Use these exact IDs in your scripts for a fully working mod!

* * * * *

*Document Version: 1.0*\
*Last Updated: Fallout 2 FISSION*\
*For more information, see the generated reports in `data/lists/` after running the game.*
