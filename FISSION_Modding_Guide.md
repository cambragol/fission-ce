Fallout FISSION Modding System: Complete Modder's Guide
=======================================================

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

6. [Asset System: Scripts, Art, and Protos](#6-asset-system-scripts-art-and-protos)
   - [6.1 Overview](#61-overview)
   - [6.2 The .lst File Patterns](#62-the-lst-file-patterns)
   - [6.3 Special Art Features](#63-special-art-features)
   - [6.4 Asset Index Ranges and Layout](#64-asset-index-ranges-and-layout)
   - [6.5 Loading Process](#65-loading-process)
   - [6.6 Critical Rules for Modders](#66-critical-rules-for-modders)
   - [6.7 Generated Reports](#67-generated-reports)
   - [6.8 Common Scenarios and Examples](#68-common-scenarios-and-examples)
   - [6.9 Integration Example](#69-integration-example)

7. [Debugging & Reports](#7-debugging--reports)
   - [7.1 Generated Reports](#71-generated-reports)
   - [7.2 Report Formats](#72-report-formats)
     - [7.2.1 Quest Report Format](#721-quest-report-format)
     - [7.2.2 Holodisk Report Format](#722-holodisk-report-format)
     - [7.2.3 Message Report Format](#723-message-report-format)
     - [7.2.4 Asset Report Format](#724-asset-report-format)
   - [7.3 Error Detection](#73-error-detection)

8. [Modder Guidelines](#8-modder-guidelines)
   - [8.1 Step-by-Step Mod Creation](#81-step-by-step-mod-creation)
   - [8.2 Testing Checklist](#82-testing-checklist)
     - [8.2.1 Areas & Maps](#821-areas--maps)
     - [8.2.2 Quests](#822-quests)
     - [8.2.3 Holodisks](#823-holodisks)
     - [8.2.4 Scripts, Art, and Protos](#824-scripts-art-and-protos)
     - [8.2.5 General Testing](#825-general-testing)

9. [Known Limitations & Workarounds](#9-known-limitations--workarounds)
   - [9.1 Quest-Specific Limitations](#91-quest-specific-limitations)
   - [9.2 Holodisk-Specific Limitations](#92-holodisk-specific-limitations)
   - [9.3 Asset-Specific Limitations](#93-asset-specific-limitations)
   - [9.4 General Limitations](#94-general-limitations)

10. [Quest System Specifics](#10-quest-system-specifics)
    - [10.1 Architecture Summary](#101-architecture-summary)
    - [10.2 Modder Workflow](#102-modder-workflow)
    - [10.3 Critical Notes for Modders](#103-critical-notes-for-modders)
    - [10.4 Integration Points](#104-integration-points)

11. [Holodisk System Specifics](#11-holodisk-system-specifics)
    - [11.1 Overview](#111-overview)
    - [11.2 File Structure](#112-file-structure)
      - [11.2.1 Holodisk Definition File](#1121-holodisk-definition-file)
      - [11.2.2 Message File Format](#1122-message-file-format)
    - [11.3 How It Works](#113-how-it-works)
      - [11.3.1 Loading Process](#1131-loading-process)
      - [11.3.2 Conversion Example](#1132-conversion-example)
    - [11.4 Script Usage](#114-script-usage)
    - [11.5 Special Notes](#115-special-notes)
      - [11.5.1 Empty Lines](#1151-empty-lines)
      - [11.5.2 Pagination](#1152-pagination)
      - [11.5.3 Localization](#1153-localization)
      - [11.5.4 GVAR Management](#1154-gvar-management)
    - [11.6 Complete Example Mod](#116-complete-example-mod)
    - [11.7 Troubleshooting Holodisks](#117-troubleshooting-holodisks)
    - [11.8 Best Practices](#118-best-practices)

12. [Proto System](#12-proto-system)
    - [12.1 Overview](#121-overview)
    - [12.2 File Structure](#122-file-structure)
      - [12.2.1 Proto Definition Files](#1221-proto-definition-files)
      - [12.2.2 List File Format](#1222-list-file-format)
      - [12.2.3 Message File Format for Protos](#1223-message-file-format-for-protos)
    - [12.3 PID Generation](#123-pid-generation)
      - [12.3.1 PID Format](#1231-pid-format)
      - [12.3.2 PID Ranges by Type](#1232-pid-ranges-by-type)
    - [12.4 PID Generation vs. .pro File Contents](#124-pid-generation-vs-pro-file-contents)
      - [12.4.1 How It Works](#1241-how-it-works)
      - [12.4.2 What This Means for Modders](#1242-what-this-means-for-modders)
      - [12.4.3 Example Workflow](#1243-example-workflow)
    - [12.5 Creating Mod Protos](#125-creating-mod-protos)
    - [12.6 Using Mod Protos in Scripts](#126-using-mod-protos-in-scripts)
      - [12.6.1 Creating Objects with Mod PIDs](#1261-creating-objects-with-mod-pids)
      - [12.6.2 Getting Proto Information](#1262-getting-proto-information)
      - [12.6.3 Checking Proto Properties](#1263-checking-proto-properties)
    - [12.7 Generated Reports](#127-generated-reports)
    - [12.8 Hash Collision Handling](#128-hash-collision-handling)
    - [12.9 Special Considerations](#129-special-considerations)
      - [12.9.1 Proto File Format](#1291-proto-file-format)
      - [12.9.2 Art Requirements](#1292-art-requirements)
      - [12.9.3 Message Loading](#1293-message-loading)
      - [12.9.4 Save Game Compatibility](#1294-save-game-compatibility)
    - [12.10 Complete Example](#1210-complete-example)
    - [12.11 Best Practices](#1211-best-practices)
    - [12.12 Troubleshooting Protos](#1212-troubleshooting-protos)
    - [12.13 Integration Points](#1213-integration-points)

13. [Troubleshooting](#13-troubleshooting)
    - [13.1 Common Issues](#131-common-issues)
    - [13.2 Debug Commands](#132-debug-commands)
    - [13.3 Debugging Steps](#133-debugging-steps)

14. [Appendix A: Quick Reference](#14-appendix-a-quick-reference)
    - [14.1 File Naming](#141-file-naming)
    - [14.2 Key Formats](#142-key-formats)
    - [14.3 ID Ranges](#143-id-ranges)
    - [14.4 Critical Rules](#144-critical-rules)

15. [Appendix B: Example Mod Structure](#15-appendix-b-example-mod-structure)
    - [15.1 Complete Example Mod](#151-complete-example-mod)
    - [15.2 Installation and Testing](#152-installation-and-testing)
    - [15.3 Notes for Modders](#153-notes-for-modders)
    - [15.4 Generated IDs Reference](#154-generated-ids-reference)
    - [15.5 Example Script Usage](#155-example-script-usage)

* * * * *

1\. System Overview
-------------------

### 1.1 Key Features

The Fallout 2 FISSION modding system provides a framework for adding new content without modifying base game files:

-   **Stable ID assignment** - Same filenames get same IDs every time
-   **Modular organization** - Keep mods in separate, self-contained files
-   **Automatic report generation** - Get all generated IDs in easy-to-read reports
-   **Backward compatibility** - Works with vanilla saves and content
-   **Localization support** - Multiple language support with English fallback
-   **Vanilla format conversion** - Your mod content converted to game format automatically

### 1.2 Supported Content Types

1.  **Maps** - Game locations with multiple elevations
2.  **Areas** - World map regions containing multiple maps
3.  **Messages** - All text content (names, descriptions, dialog)
4.  **Quests** - Quest definitions with automatic description linking
5.  **Holodisks** - Holodisk definitions with multi-page text
6.  **Scripts** - Game behavior scripts (.int files)
7.  **Art** - Visual assets (.frm files for interface, items, critters, etc.)
8.  **Protos** - Game objects (items, critters, scenery, walls, tiles, misc)

* * * * *

2\. Core Architecture
---------------------

### 2.1 ID Ranges

| Content Type | Vanilla Range | Mod Range | Notes | |--------------|---------------|-----------|-------| | Scripts | 0-? (varies) | vanillaCount-4095 | 4096 total slots, check reports |
| Art Items | 0-? (varies) | 4096-8191 | Extended range for mods |
| Art Interface | 0-? (varies) | 4096-8191 | Extended range for mods |
| Art Critters | 0-? (varies) | 4096-8191 | Extended range for mods |
| Items Protos | 0x00XXXXXX | 0x0000C8-0xFFFFFF | Type 0, hash-based PIDs |
| Critters Protos | 0x01XXXXXX | 0x0100C8-0xFFFFFF | Type 1, hash-based PIDs |
| Scenery Protos | 0x02XXXXXX | 0x0200C8-0xFFFFFF | Type 2, hash-based PIDs |
| Quests | 0-199 | 200-999 | Hash-based |
| Maps | 0-199 | 200-1999 | Hash-based |
| Areas | 0-199 | 200-999 | Hash-based |
| Holodisks | 0-? (by GVAR) | 50000+ | Message IDs, blocks of 500 |
| Messages | 0-32767 | 32768-65535 | Hash-based |

### 2.2 Key Principles

1.  **Deterministic indexing** - Same inputs always produce same outputs
2.  **Case-insensitive** - System normalizes everything to lowercase
3.  **Fail on collision** - Popup warnings prevent silent overwrites
4.  **Separate concerns** - Config, art, scripts, messages in different files
5.  **Automatic linking** - Quests auto-link to their descriptions
6.  **Vanilla conversion** - Mod content converted to vanilla format at load time
7.  **Stable asset placement** - Scripts, art, protos get consistent IDs
8.  **Protected vanilla ranges** - Original game content remains untouched

* * * * *

3\. File Structure & Naming Conventions
---------------------------------------

### 3.1 Complete Directory Structure

```
Fallout 2 Game Directory/\
├── data/ # Core configuration\
│ ├── city_{modname}.txt # Area definitions\
│ ├── maps_{modname}.txt # Map definitions\
│ ├── quests_{modname}.txt # Quest definitions\
│ └── holodisk_{modname}.txt # Holodisk definitions\
├── text/ # Localization files\
│ └── english/\
│ └── game/\
│ └── messages_{modname}.txt # All text content\
├── art/ # Art assets\
│ ├── intrface/ # Interface art\
│ │ ├── mod_{modname}.lst\
│ │ └── *.frm\
│ ├── items/ # Item art\
│ │ ├── mod_{modname}.lst\
│ │ └── *.frm\
│ ├── critters/ # Critter art\
│ │ ├── mod_{modname}.lst\
│ │ └── *.frm\
│ └── ... other art types\
├── scripts/ # Script files\
│ ├── scripts_{modname}.lst\
│ ├── *.int\
│ └── {language}/ # Localized scripts (optional)\
├── proto/ # Proto definitions\
│ ├── items/ # Item protos\
│ │ ├── items_{modname}.lst\
│ │ └── *.pro\
│ ├── critters/ # Critter protos\
│ │ ├── critters_{modname}.lst\
│ │ └── *.pro\
│ └── ... other proto types\
├── dialog/ # Script dialog messages (optional)\
│ └── *.msg\
├── lists/ # Generated reports\
└── ... (other game folders)
```

### 3.2 Required FISSION Files

For a complete mod, you typically need these files:

1.  `data/city_{modname}.txt` - Area definitions
2.  `data/maps_{modname}.txt` - Map definitions
3.  `data/quests_{modname}.txt` - Quest definitions
4.  `data/holodisk_{modname}.txt` - Holodisk definitions (optional)
5.  `text/english/game/messages_{modname}.txt` - Message/text content
6.  `scripts/scripts_{modname}.lst` - Script list
7.  Art `.lst` files in respective `art/` subdirectories
8.  Proto `.lst` files in respective `proto/` subdirectories

### 3.3 File Naming Rules

-   **All lowercase recommended** (system normalizes to lowercase)
-   **No spaces or special characters** except underscore
-   **Mod name must be consistent** across all files
-   **Map names ≤8 characters** (DOS 8.3 limitation for saves)

Example for mod "wasteland":
-   `data/city_wasteland.txt`
-   `data/maps_wasteland.txt`
-   `data/quests_wasteland.txt`
-   `scripts/scripts_wasteland.lst`
-   `art/intrface/mod_wasteland.lst`

### 3.4 Important Notes

1.  **Scripts, Art, Protos use `.lst` files** - List your assets, system assigns IDs
2.  **Message files need language directories**:
    - English: `text/english/game/messages_{modname}.txt`
    - German: `text/german/game/messages_{modname}.txt`
    - etc.
3.  **Vanilla files remain unchanged** - Mod files are added alongside them
4.  **Check generated reports** in `data/lists/` for your actual IDs

* * * * *

4\. Configuration Files
-----------------------

### 4.1 Area Configuration (`city_{modname}.txt`)

Defines world map areas. Each area can contain multiple maps.

#### 4.1.1 Format

```
[Area 0] # Section number (sequential)\
area_name = SCRAPTOWN # MUST be uppercase, unique identifier\
world_pos = 360,290 # World map coordinates\
start_state = On # On, Off, or Secret\
size = Medium # Small, Medium, or Large\
townmap_art_idx = 156 # Art index for town map\
townmap_label_art_idx = 368 # Art index for town map label

Entrances - links maps to this area
===================================

Format: State,X,Y,MapLookupName,MapIndex,Unknown,Elevation
==========================================================

entrance_0 = On,110,220,SCRAPTOWN1,-1,-1,0\
entrance_1 = On,235,250,SCRAPTOWN2,-1,-1,0
```

#### 4.1.2 Rules

-   `area_name` must be unique across all mods
-   Use uppercase for consistency
-   Maximum 8 characters recommended (not enforced but good practice)

### 4.2 Map Configuration (`maps_{modname}.txt`)

Defines individual game maps.

#### 4.2.1 Format

```
[Map 987] # Section number (can be any number)\
lookup_name = Scraptown1 # Unique identifier, referenced by entrances in city file\
map_name = scrapa # MUST be ≤8 characters (DOS 8.3 limitation), matches .map filename\
music = fs_grand # Music track\
saved = Yes # Yes/No - whether game saves here\
automap = yes # Yes/No - shows on automap\
dead_bodies_age = No # Yes/No (optional)\
can_rest_here = Yes,No,No # Three values for elevations 0,1,2 (optional)

Ambient sound effects (optional)
================================

ambient_sfx = gntlwin1:50, gntlwind:50

Random start points (optional)
==============================

random_start_point_0 = elev:0,tile_num:12345:elev:0,tile_num:23456
```

**Note:** The connection between areas and maps is made through:
- `entrance_X` in city file references `lookup_name` in maps file
- Example: `entrance_0 = On,110,220,Scraptown1,-1,-1,0` references `lookup_name = Scraptown1`

#### 4.2.2 Critical Rules

1.  `map_name` ≤ 8 characters - DOS 8.3 limitation for save games
2.  `lookup_name` must be unique across all mods and referenced by `entrance_X` in city file
3.  `map_name` must match your actual .map filename (without extension)
4.  Section numbers can be any value (987, 152, 445, etc. in examples) - they don't need to be sequential

### 4.3 Quest Configuration (`quests_{modname}.txt`)

Defines quests using vanilla format with enhanced mod support.

#### 4.3.1 Format

Format: location, description, gvar, displayThreshold, completedThreshold
=========================================================================

Note: 'description' field is IGNORED for mod quests - replaced by generated message ID
======================================================================================

```
1500, 0, 79, 1, 2\
1500, 0, 80, 1, 3
```

#### 4.3.2 Rules

1.  One quest per line in CSV format
2.  Comments start with `#` and are ignored
3.  Location - Area index (must be valid area, vanilla or mod)
4.  Description field - Ignored for mod quests (replaced by generated message ID)
5.  GVAR - Global variable tracking quest state
6.  Thresholds - Display and completion values

### 4.4 Holodisk Configuration (`holodisk_{modname}.txt`)

Defines holodisks by their GVAR (global variable) numbers.

#### 4.4.1 Format

```
900 # First holodisk - appears when GVAR 900 = 1\
901 # Second holodisk - appears when GVAR 901 = 1
```

#### 4.4.2 Rules

-   One GVAR per line
-   Comments start with `#`
-   GVARs should be unique (900+ recommended for mods)
-   GVAR must be set to 1 to make holodisk appear in Pip-Boy

* * * * *

5\. Message System
------------------

### 5.1 File Structure (`messages_{modname}.txt`)

Contains all text for your mod, organized by section. Keys must match exactly as shown!

#### 5.1.1 Format

```
[map] # World map and automap text\
area_name:SCRAPTOWN = Scraptown # Area display name\
lookup_name:Scraptown1:0 = Scraptown Gate # Map + elevation display name

[worldmap] # Town map entrance labels\
entrance_0:SCRAPTOWN = Scraptown Gate

[quests] # Quest descriptions\
quest:0 = The people of Scraptown are having trouble...

[pipboy] # Holodisk text\
holodisk:0:name = Important Data Disk\
holodisk:0:line:0 = This holodisk contains critical information\
holodisk:0:line:1 = END-PAR # Paragraph break\
holodisk:0:line:2 = END-DISK # Required

[proto_myquest] # Proto names and descriptions\
myquest:mygun:name = Custom Plasma Rifle\
myquest:mygun:desc = A modified plasma rifle.
```

#### 5.1.2 Critical Format Rules

1.  Section headers must match exactly: `[map]`, `[worldmap]`, `[quests]`, `[PIPBOY]`
2.  Case-sensitive keys: Use exact formats shown (lowercase for area_name/lookup_name/entrance_X/holodisk)
3.  No trailing spaces in section headers or keys

### 5.2 Key Formats Explained

These formats match the vanilla message file structure for consistency:

-   Vanilla uses `area_name:` for area names
-   Vanilla uses `lookup_name:` for map names
-   Vanilla uses `entrance_X:` for town map labels
-   Extended with `[quests]` section for mod quests
-   Extended with `[PIPBOY]` section for mod holodisks
-   Extended with `[proto_{modname}]` section for proto names/descriptions

### 5.3 ID Generation Examples

The system generates message IDs automatically:

- Area name: Generates ID based on `area_name:AREA_NAME`
- Map name: Generates ID based on `lookup_name:MAPNAME:ELEVATION`
- Quest description: Generates ID based on `quest:INDEX`
- All IDs are consistent - same key = same ID every time

### 5.4 Common Mistakes to Avoid

-   **WRONG**: `AREA:SCRAPTOWN` (uppercase, wrong prefix)
-   **WRONG**: `MAP:SCRAPTOWN1:0` (wrong prefix)
-   **CORRECT**: `area_name:SCRAPTOWN`
-   **CORRECT**: `lookup_name:SCRAPTOWN1:0`
-   **CORRECT**: `quest:0` (lowercase, not `QUEST:0`)

### 5.5 Validation Tips

1.  Check vanilla `messages.txt` for reference formats
2.  Use exact lowercase prefixes shown above
3.  Test with minimal mod to verify format
4.  Check generated reports - wrong formats won't appear

Important: The system looks for exact key formats. If your keys don't match, messages won't load and you'll get "Error" text in-game.

* * * * *

6\. Asset System: Scripts, Art, and Protos
-----------------------------------------

### 6.1 Overview

FISSION extends Fallout 2's asset loading with a unified system that supports:
- **Supplementary `.lst` files** load alongside vanilla lists
- **Stable, deterministic indexing** - Same filenames get same IDs every time
- **Vanilla protection** - Original game assets remain untouched
- **Automatic variant detection** for widescreen/HD assets
- **Special remapping feature** for art assets
- **Automatic collision detection** with popup warnings

**Key Concept**: The system maintains three types of art assets:
1. **Vanilla assets** (0-4095) - Original game assets, protected
2. **Variant assets** (automatic) - HD/widescreen versions, auto-loaded when detected
3. **Mod assets** (4096-8191) - Your custom content via `.lst` files

### 6.2 The .lst File Patterns

Each asset type has specific directory and naming conventions:

#### 6.2.1 Scripts

```
scripts/scripts_{modname}.lst # Mod script list\
scripts/{scriptname}.int # Compiled script\
scripts/{language}/{scriptname}.int # Optional localized version
```

Example `scripts_myquest.lst`:

```
mytown_[guard.int](https://guard.int/)\
mytown_[door.int](https://door.int/) #local_vars=3\
special_gun
```

#### 6.2.2 Art Assets (Multiple Types)

```
art/{type}/mod_{modname}.lst # Mod art list (type = intrface, items, critters, etc.)\
art/{type}/{artname}.frm # Art file\
art/{type}/fission.lst # Special: Always loads first (if exists)
```

Example `art/intrface/mod_myquest.lst`:

```
mybutton.frm\
myicon.frm
```

#### 6.2.3 Protos (Multiple Types)

```
proto/{type}/{type}_{modname}.lst # Mod proto list (type = items, critters, scenery, etc.)\
proto/{type}/{protoname}.pro # Proto file
```

Example `proto/items/items_myquest.lst`:

```
mygun\
specialarmor
```

#### 6.2.4 Variant Art (Automatic)

```
art/{type}/{basename}{suffix}.frm # Auto-detected when suffix matches config
```

Example: `art/intrface/button_ok_800.frm` (suffix `_800` for 800x600 widescreen)

### 6.3 Special Art Features

#### 6.3.1 Variant System (Automatic HD/Widescreen)
The system automatically detects and loads variant art for widescreen/HD support:

**How Variants Work:**
1. **Automatic detection**: System scans `art/{type}/` for files matching pattern `*{suffix}.frm`
2. **Suffix configuration**: Set in `fallout2.cfg` (default: `_800` for 800x600)
3. **Matching logic**: Variant must match a base vanilla asset name
   - Base: `button_ok.frm`
   - Variant: `button_ok_800.frm` (suffix `_800`)
4. **Automatic assignment**: Variants get special slots after vanilla assets

**Example Configuration (`fallout2.cfg`):**

```
[graphics]\
widescreen_variant_suffix=_800
```

#### 6.3.2 Art Remapping
Art supports a special remapping syntax to redirect vanilla assets:

```
@original_name=new_path/filename.frm
```

Example in `mod_myquest.lst`:

```
@plasma.frm=items/hd/plasma_hd.frm # Redirects vanilla plasma.frm to HD version\
mybutton.frm # Adds new art
```

**Remapping Rules:**
- Only works for vanilla assets (indices 0-4095)
- Does NOT create new slots
- Updates path reference only
- Shows as `»` in `art_list.txt` report

#### 6.3.3 Art Loading Priority
1. **`fission.lst`** - Always loads first (if present)
2. **Vanilla assets** - Loaded from base lists
3. **Variant assets** - Auto-detected and loaded
4. **`mod_*.lst` files** - Loaded alphabetically

### 6.4 Asset Index Ranges and Layout

#### 6.4.1 Complete Art Index Layout

```
0 - (vanillaCount-1): Vanilla assets (protected, original game)\
vanillaCount - (vanillaCount+variantCount-1): Variant assets (auto-detected HD/widescreen)\
4096 - 8191: Mod assets (your custom content via .lst files)
```

**Example from `art_list.txt` report:**

```
[INTRFACE] (850 assets)\
Vanilla: 600 assets | Variants: 50 assets | Mods: 200 assets

* * * * *

Slot Ranges:\
Vanilla: 0-599\
Variants: 600-649\
Mods: 4096-8191

* * * * *
```

#### 6.4.2 Variant Slot Assignment
Variant slots are assigned **dynamically** based on detection:
1. System scans `art/{type}/` directory for `*{suffix}.frm` files
2. For each matching variant file:
   - Extracts base name (removes suffix)
   - Finds matching vanilla asset index
   - Allocates next available variant slot
   - Links variant to vanilla index internally

**Important**: Variant indices are **not stable** - they depend on detection order and file system.

#### 6.4.3 Script Index Range

```
0 - (vanillaCount-1): Vanilla scripts (protected)\
vanillaCount - 4095: Mod scripts (deterministic from filename)
```

**Total**: 4096 script slots maximum (0-4095)

#### 6.4.4 Art Mod Index Range (Different from Scripts!)

```
4096 - 8191: Mod art assets (deterministic from filename)
```

**Total**: 8192 art slots maximum (0-8191), with mods in **extended range 4096-8191**

#### 6.4.5 Key Differences Between Systems

| System | Total Slots | Vanilla Range | Variant Range | Mod Range | Notes |
|--------|------------|---------------|---------------|-----------|-------|
| Scripts | 4096 | 0-vanillaCount | (none) | vanillaCount-4095 | Shared array |
| Art | 8192 | 0-4095 | vanillaCount-(vanillaCount+variantCount-1) | 4096-8191 | Extended array |
| Protos | 0xFFFFFF | Type-dependent | (none) | Hash-based | Full PID system |

**Critical Distinction**: Art mods use **4096-8191** range, NOT the same 0-4095 range as scripts!

### 6.5 Loading Process

#### 6.5.1 General Loading Flow
1. **Vanilla assets** load from base lists (`scripts.lst`, `art/*.lst`, etc.)
2. **Variant detection** (art only) - scans directories for `*{suffix}.frm` files
3. **Mod discovery** finds `*_{modname}.lst` and `mod_*.lst` files
4. **Alphabetical processing** for consistent loading order
5. **For each mod asset**:
   - Calculate stable index based on filename
   - Check for collisions (index already occupied)
   - **If collision**: Show popup warning, skip asset
   - **If free**: Assign to calculated slot

#### 6.5.2 Collision Handling

Hash Collision Popup Example:\

```
┌─────────────────────────────────────┐\
│ ART SLOT COLLISION DETECTED!        │\
│                                     │\
│ New asset: mygun.frm                │\
│ Target slot: 4120                   │\
│ Existing asset: othermod_gun.frm    │\
│                                     │\
│ To resolve: Rename your art file to │\
│ change its namespace.               │\
│                                     │\
│ The asset 'mygun.frm' will NOT be   │\
│ loaded.                             │\
└─────────────────────────────────────┘
```

**Important**: Unlike quests/maps, asset collisions cause the asset to be **skipped entirely**, not overwritten.

### 6.6 Critical Rules for Modders

#### 6.6.1 File Naming and Placement
1. **Art uses `mod_*.lst`**, NOT `{type}_*.lst` (different from scripts/protos!)
2. **Scripts use `scripts_*.lst`** in `scripts/` directory
3. **Protos use `{type}_*.lst`** in respective `proto/{type}/` directories
4. **Case normalization**: All names converted to lowercase for indexing
5. **File placement**: Assets must be in correct directories (`.frm` in `art/`, `.pro` in `proto/`, etc.)

#### 6.6.2 Index Range Awareness
6. **Script range**: 0-4095 total, mods in upper portion (`vanillaCount-4095`)
7. **Art range**: 0-8191 total, mods in **4096-8191** (extended range)
8. **Art variants**: Auto-detected, placed after vanilla assets
9. **Proto PIDs**: Full 24-bit range, type-dependent

#### 6.6.3 Collision and Compatibility
10. **Collision resolution**: Rename asset file to change its index
11. **Art remapping**: Use `@original=new_path` syntax only for vanilla redirects
12. **Variant creation**: Name files as `{basename}{suffix}.frm` (e.g., `button_800.frm`)
13. **Localization**: Optional language subdirectories
14. **Check reports**: Always verify IDs in generated reports before using them

### 6.7 Generated Reports

#### 6.7.1 Essential Reports in `data/lists/`

| Report | Contents | Key Information |
|--------|----------|-----------------|
| `scripts_list.txt` | All scripts | Indices (0-4095), mod assignments, local vars |
| `art_list.txt` | All art assets | Indices (0-8191), variants, remaps (`»`), collisions (`#`) |
| `proto_list.txt` | All protos | PIDs, types, names, descriptions, mod assignments |

#### 6.7.2 Reading Art List Reports

```
[INTRFACE] (850 assets)\
Vanilla: 600 assets | Variants: 50 assets | Mods: 200 assets

* * * * *

Slot Ranges:\
Vanilla: 0-599\
Variants: 600-649\
Mods: 4096-8191

* * * * *

VANILLA ASSETS:\
0: button_ok.frm\
1: button_cancel.frm\
...

VARIANT ASSETS:\
600: button_ok_800.frm # Auto-detected widescreen variant\
601: button_cancel_800.frm

MOD ASSETS:\
4096: mybutton.frm\
4097: otherbutton.frm

--- CONFLICT DETAILS ---

4120: COLLISION: existing_gun.frm vs mygun.frm - SKIPPED
========================================================

» 150: REMAP: plasma.frm -> items/hd/plasma_hd.frm
```

#### 6.7.3 Report Legend
- **No prefix**: Normal loaded asset
- **`#`**: Hash collision (needs fixing)
- **`»`**: Vanilla asset remapped to new path
- **`!`**: Other conflict/issue

### 6.8 Common Scenarios and Examples

#### 6.8.1 Adding New Weapon with All Assets
1. **Create art**: `art/items/mod_myquest.lst` + `mygun.frm`
2. **Create proto**: `proto/items/items_myquest.lst` + `mygun.pro` (references art index)
3. **Create script**: `scripts/scripts_myquest.lst` + `mygun.int`
4. **Add proto messages** to `messages_myquest.txt`:

```
[proto_myquest]\
myquest:mygun:name = Custom Plasma Rifle\
myquest:mygun:desc = A modified plasma rifle.
```

5\. **Run game**, check reports:
- `art_list.txt`: Find art index (e.g., 4120)
- `proto_list.txt`: Find PID (e.g., 0x00DB240E)
- `scripts_list.txt`: Find script index (e.g., 450)

#### 6.8.2 Creating HD Variants for Vanilla Assets
1. **Create variant files** with suffix (e.g., `_800`):

```
art/items/plasma_800.frm\
art/items/laser_800.frm
```

2\. **Configure suffix** in `fallout2.cfg`:

```
[graphics]\
widescreen_variant_suffix=_800
```

3\. **System auto-detects** and loads variants
4. **Check `art_list.txt`**: Variants appear in variant range (600-649 in example)

#### 6.8.3 Redirecting Vanilla Art (HD Upgrade)

In art/items/mod_myquest.lst
============================

```
@plasma.frm=items/hd/plasma_hd.frm\
@laser.frm=items/hd/laser_hd.frm
```

Adds new assets
===============

```
mycustomgun.frm
```

#### 6.8.4 Handling Hash Collisions
If collision occurs:
1. **Note conflicting names** from popup
2. **Rename your file**: `mygun.frm` → `mymod_gun.frm`
3. **Update `.lst` file** with new name
4. **New index** will (likely) go to different free slot

### 6.9 Integration Example

```
// After checking reports:
// art_list.txt shows: 4120: mygun.frm
// proto_list.txt shows: 0x00DB240E: mygun
// scripts_list.txt shows: 450: mygun

// Create the weapon in script
int gun_pid = 0x00DB240E;  // From proto_list.txt
obj = create_object(gun_pid, tile, elevation);

// The weapon will:
// - Use art index 4120 for display
// - Execute script 450 for behavior
// - Show name/desc from message file

// In critter proto (.pro file):
// Script Index field: 450 (from scripts_list.txt)
// Art FID: Would reference art index 4120

// In mapper or .map file:
// Object PID: 0x00DB240E
// Script: 450

Remember: Always check generated reports for actual IDs before using them. The system ensures same filenames = same IDs every time, but you must verify the exact numbers.

* * * * *
```

7\. Debugging & Reports
-----------------------

### 7.1 Generated Reports

The system automatically creates these files in `data/lists/`:

#### Core Configuration Reports (Essential for all mods)
1.  `area_list.txt` - All areas with slots and mod assignments
2.  `maps_list.txt` - All maps with slots, types, and override info
3.  `quests_list.txt` - All quests with slots, mod assignments, and generated message IDs

#### Asset Reports (For scripts, art, protos)
4.  `scripts_list.txt` - All scripts with indices, mod assignments, and local variable counts
5.  `art_list.txt` - All art assets with indices, types, and file references
6.  `proto_list.txt` - All protos with PIDs, types, names, and mod assignments

#### Message System Reports (Text content)
7.  `messages_MAP_list.txt` - All mod messages in map.msg with IDs (area/map names)
8.  `messages_WORLDMAP_list.txt` - All mod messages in worldmap.msg with IDs (entrance labels)
9.  `messages_QUESTS_list.txt` - All mod quest descriptions with IDs
10. `messages_PIPBOY_list.txt` - All mod messages in pipboy.msg with IDs (including holodisk messages)

#### Specialized Reports
11. `holodisks_list.txt` - All holodisks with GVARs, name IDs, and mod assignments

**Usage notes - recommended checking order:**
1. **First build**: Check `area_list.txt`, `maps_list.txt` to verify your areas/maps loaded
2. **Add assets**: Check `scripts_list.txt`, `art_list.txt`, `proto_list.txt` for IDs to use in other files
3. **Add text**: Check message reports to verify your text appears correctly
4. **Final check**: Verify `quests_list.txt` and `holodisks_list.txt` for integrated content

Reports are generated every time the game loads with mods present.

### 7.2 Report Formats

#### 7.2.1 Quest Report Format (`quests_list.txt`)

```
==============================================================================\
Fallout Fission - Quest System Report\
==============================================================================\
Generated IDs for mod quests and their message IDs.

Quest ID Range: 200-999 (mod quests)\
Message ID Range: 32768-65535 (stable hash-based)\
==============================================================================

MOD QUESTS:\
Quest ID | Mod | Description Message ID | Quest Key

* * * * *

200 | myquest | 34120 | myquest:0\
201 | myquest | 34121 | myquest:1

MOD QUEST DETAILS:

* * * * *

Quest 200: myquest:0\
Mod: myquest\
Message ID: Desc=34120\
Game Data: Location=1500, GVAR=79\
Thresholds: Display=1, Complete=2

SUMMARY:\
Total Mod Quests: 2\
Base Quests: 150
```

#### 7.2.2 Holodisk Report Format (`holodisks_list.txt`)

```
==============================================================================\
Fallout 2 Fission - Holodisk System Report\
==============================================================================\
All holodisks (vanilla and mod) converted to vanilla format.

Format: Index | GVAR | Name ID | Desc ID | Type | Name

* * * * *

Summary: 11 total holodisks (9 vanilla, 2 mod)

0 |   45 |     610 |     611 | Vanilla | Vault 13 Holodisk
...

10 | 900 | 50000 | 50001 | MOD | Test Holodisk 1\
11 | 901 | 50500 | 50501 | MOD | Test Holodisk 2

MOD HOLODISK DETAILS:\
====================\
Mod holodisks use the following ID blocks:

Block 0: IDs 50000-50499 (GVAR 900) - Test Holodisk 1\
Block 1: IDs 50500-50999 (GVAR 901) - Test Holodisk 2
```

#### 7.2.3 Message Report Format (`messages_PIPBOY_list.txt`)

```
==============================================================================\
Fallout Fission - PIPBOY Messages\
==============================================================================\
Generated IDs for mod message references in scripts.

Message ID Range: 32768-65535 (stable hash-based)\
Usage: display_msg(ID); // Reference in scripts\
==============================================================================

MOD MESSAGES (Custom Content):

| ID | Text Preview |
| --- | --- |
| 33685 | Another test holodisk with more content. |
| 33686 | This demonstrates multiple holodisks per mod. |
| 33687 | END-PAR |
| 33688 | Paragraph breaks work too! |
| 33689 | END-DISK |
| 38279 | Test Holodisk 2 |
| 54832 | This is a test holodisk for the mod system. |
| 54833 | It should appear in the Pip-Boy under Data. |
| 54835 | This text was loaded from a mod file. |
| 54836 | END-DISK |
| 59451 | Test Holodisk 1 |

SUMMARY:\
Total Mod Messages: 10\
Base Messages: 626
```

#### 7.2.4 Asset Report Format (`art_list.txt`)

```
==============================================================================\
Fallout Fission - Art Asset Report\
==============================================================================\
This report shows how art assets are loaded - essential for mod debugging, and\
finding IDs for mod art assets.

Key Features:

-   Vanilla assets: Protected in lower slots

-   Variant assets: HD versions in protected dedicated slots

-   Mod assets: Your content in remaining slots via filename indexing

-   Use '@original=new_path' to redirect vanilla assets

Conflict Markers:\
» Remapped vanilla asset

Hash collision (needs fixing)
=============================

Quick Tips:

-   See end of file for Conflict details

-   Fix # collisions by renaming files

-   Use » remaps only for necessary path changes

-   List new assets in mod_*.lst files\
    ==============================================================================

Report Generated: 2024-01-15 14:30:45

[INTRFACE] (850 assets)\
Vanilla: 600 assets | Variants: 50 assets | Mods: 200 assets

* * * * *

Slot Ranges:\
Vanilla: 0-599\
Variants: 600-649\
Mods: 4096-8191

* * * * *

VANILLA ASSETS:\
0: button_ok.frm\
1: button_cancel.frm\
...

VARIANT ASSETS:\
600: button_ok_800.frm\
601: button_cancel_800.frm

MOD ASSETS:\
4096: mybutton.frm\
4097: otherbutton.frm

--- CONFLICT DETAILS ---

4120: COLLISION: existing_gun.frm vs mygun.frm - SKIPPED
========================================================

» 150: REMAP: plasma.frm -> items/hd/plasma_hd.frm
```

### 7.3 Error Detection

-   **Slot collisions**: Clear popup error with resolution steps
-   **Missing quest messages**: Quest shows "Error" in PipBoy
-   **Missing holodisk messages**: Holodisk shows "Error" when clicked
-   **Format errors**: Reports malformed quest/holodisk lines
-   **Hash inconsistencies**: System ensures consistent indexing
-   **DOS 8.3 violations**: Warning popup for map names >8 characters
-   **Missing area references**: Warns when `city_name` doesn't match any area
-   **Asset collisions**: Popup warnings with skip/rename instructions

* * * * *

8\. Modder Guidelines
---------------------

### 8.1 Step-by-Step Mod Creation

1.  **Choose a mod name** (e.g., `myquest`)

2.  **Create area file** (`city_myquest.txt`):
    ```
    [Area 0]
    area_name = MYTOWN
    world_pos = 400,300
    start_state = On
    size = Medium
    entrance_0 = On,100,200,MYTOWN1,-1,-1,0
    ```

3.  **Create map file** (`maps_myquest.txt`):
    ```
    [Map 0]
    lookup_name = MYTOWN1
    map_name = mytown1      # ≤8 characters!
    city_name = MYTOWN      # Must match area_name
    saved = Yes
    automap = yes
    ```

4.  **Create quest file** (`quests_myquest.txt`):
    ```
    # location, description, gvar, displayThreshold, completedThreshold
    1500, 0, 79, 1, 2
    1500, 0, 80, 1, 3
    ```

5.  **Create holodisk file** (`holodisk_myquest.txt`):
    ```
    900
    901
    ```

6.  **Create message file** (`messages_myquest.txt`):
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

7.  **Create art list files** (one per art type):

    - Interface art: `art/intrface/mod_myquest.lst`
      ```
      mybutton.frm
      myicon.frm
      ```

    - Item art: `art/items/mod_myquest.lst`
      ```
      mygun.frm
      myarmor.frm
      ```

    - Critter art: `art/critters/mod_myquest.lst`
      ```
      newmutant.frm
      ```

    - Place corresponding `.frm` files in same directories

8.  **Create script list file** (`scripts/scripts_myquest.lst`):
    ```
    # List of script files (.int extension optional)
    mytown_guard.int
    mytown_door #local_vars=3
    special_gun
    ```
    - Compile `.ssl` source files to `.int` format
    - Place `.int` files in `scripts/` directory
    - Optional localization: `scripts/english/` for English-specific versions

9.  **Create proto list files** (one per proto type):
    - Items: `proto/items/items_myquest.lst`
      ```
      mygun
      specialarmor
      ```
    - Critters: `proto/critters/critters_myquest.lst`
      ```
      newcritter
      ```
    - Scenery: `proto/scenery/scenery_myquest.lst`
      ```
      mydoor
      ```
    - Create corresponding `.pro` files using mapper.exe or as templates

10. **Add proto messages** to your message file:
    ```
    [proto_myquest]
    myquest:mygun:name = Custom Plasma Rifle
    myquest:mygun:desc = A modified plasma rifle.
    myquest:newcritter:name = Mutant Guardian
    myquest:newcritter:desc = A heavily armed mutant.
    ```

11. **Place all files** in correct directories

12. **Run the game** - check generated reports for IDs:
    - `data/lists/art_list.txt` - Art indices
    - `data/lists/scripts_list.txt` - Script indices
    - `data/lists/proto_list.txt` - Proto PIDs
    - Other reports for quest, map, area IDs

13. **Use IDs in your mod**:
    ```
    // Display area name
    display_msg(34120);

 // Set quest state
    op_set_quest(200, 1);

 // Create critter with mod PID and script
    critter = create_object_sid(0x01A1B2C3, 12345, 0, 450);

 // Create item with mod art and proto
    gun = create_object(0x00DB240E, dude_tile, dude_elevation);

 // Use art index for display
    display_msg(art_get_name(1560));

 // Give holodisk to player
    set_global_var(900, 1);
    display_msg("You found a holodisk!");
    ```

### 8.2 Testing Checklist

#### 8.2.1 Areas & Maps
-   Area appears on world map with correct name
-   Map can be entered and saves work
-   Automap shows correct map name
-   Town map shows correct entrance labels

#### 8.2.2 Quests
-   Quest appears in PipBoy with correct description
-   Quest state changes work (`op_set_quest/op_get_quest`)
-   Quest completes at correct threshold
-   Generated IDs match between quest and message reports
-   No "Error" text in quest descriptions

#### 8.2.3 Holodisks
-   Holodisk appears in Pip-Boy Data section when GVAR=1
-   Holodisk text displays correctly when clicked
-   Pagination works (Back/More buttons if needed)
-   END-PAR markers create paragraph breaks
-   END-DISK marker is present (auto-added if missing)
-   No "Error" text in holodisk content

#### 8.2.4 Scripts, Art, and Protos
-   Scripts appear in `scripts_list.txt` with correct indices
-   Art appears in `art_list.txt` with correct indices
-   Protos appear in `proto_list.txt` with generated PIDs
-   No collision warnings during loading
-   Art displays correctly in-game
-   Protos can be created and used in scripts
-   Scripts execute correctly when triggered

#### 8.2.5 General Testing
-   Generated reports contain all expected IDs
-   Save games load correctly
-   No popup warnings during loading
-   Localization works (if provided)
-   Multiple mods work together without conflicts

* * * * *

9\. Known Limitations & Workarounds
------------------------------------

### 9.1 Quest-Specific Limitations

1.  **Description Field Override**:
    -   Problem: Vanilla quests use description field as message ID offset
    -   Solution: Mod quests use generated message IDs automatically
    -   Workaround: Always use `[quests]` section in message files

2.  **Quest Location Validation**:
    -   Problem: Quest location must reference valid area index
    -   Solution: System doesn't validate location references
    -   Workaround: Test thoroughly, check `area_list.txt` for valid indices

3.  **GVAR Conflicts**:
    -   Problem: Multiple mods could use same GVAR
    -   Solution: No automatic GVAR allocation
    -   Workaround: Document GVAR usage in mod readme, use 900+ range

### 9.2 Holodisk-Specific Limitations

1.  **Empty Lines Skipped**:
    -   Problem: Message system skips empty values
    -   Solution: Use `**END-PAR**` for paragraph breaks
    -   Workaround: Add single space for truly blank lines (not recommended)

2.  **GVAR Range**:
    -   Problem: No reserved GVAR range for mods
    -   Solution: Use 900+ to avoid vanilla conflicts
    -   Workaround: Check vanilla GVAR list before choosing numbers

3.  **Text Length**:
    -   Problem: No hard limit, but very long holodisks may cause issues
    -   Solution: Use multiple holodisks for very long content
    -   Workaround: Split content across multiple holodisks

### 9.3 Asset-Specific Limitations

1.  **Script Hash Collisions**:
    -   Problem: Different script names can index to same slot
    -   Solution: Scripts are skipped with warning popup (NOT auto-resolved)
    -   Workaround: Rename script file to change its index

2.  **Limited Script Slots**:
    -   Problem: Only 4096 total script slots available
    -   Solution: Vanilla scripts protected in lower range
    -   Workaround: Be efficient with script usage; reuse scripts where possible

3.  **Art Extended Range**:
    -   Problem: Art mods use 4096-8191, different from scripts
    -   Solution: System automatically uses extended range
    -   Workaround: Always check `art_list.txt` for actual indices

4.  **Variant Index Instability**:
    -   Problem: Variant indices depend on detection order
    -   Solution: Variants auto-detected, not stable indexed
    -   Workaround: Don't reference variant indices directly in scripts

### 9.4 General Limitations

1.  **DOS 8.3 Filename Limitation**:
    -   Problem: `map_name` >8 characters breaks save games
    -   Workaround: Always use ≤8 character map names
    -   System: Warning popup alerts modders

2.  **Save Game Compatibility**:
    -   Problem: Adding mods breaks old saves if they reference missing content
    -   Solution: Test with new saves first
    -   Workaround: Document mod dependencies clearly

3.  **Case Sensitivity**:
    -   Issue: Different file systems handle case differently
    -   Solution: System normalizes everything to lowercase
    -   Recommendation: Use uppercase in configs for readability

4.  **Loading Order Dependencies**:
    -   Issue: Areas must load before maps that reference them
    -   Solution: Fixed loading order in engine
    -   Implementation: Areas load, then maps, then validate links

5.  **Quest-Message Link Validation**:
    -   Issue: No automatic check that quest messages exist
    -   Handling: Quest shows "Error" if message missing
    -   Resolution: Ensure `[quests]` section exists with correct keys

* * * * *

10\. Quest System Specifics
---------------------------

### 10.1 Architecture Summary

1.  **Dual ID System**:
    -   Quest ID (200-999): For `op_set_quest/op_get_quest` operations
    -   Message ID (32768-65535): For quest descriptions via `display_msg()`

2.  **Automatic Linking**:
    ```
    Modder creates:                     System generates:          Modder uses in scripts:
    --------------------                ------------------         -----------------------
    quests_MyMod.txt  --indexing-->     Quest ID: 200              op_set_quest(200, 1)
                                        Message ID: 34120          display_msg(34120)
                                        |
    messages_MyMod.txt --same-index->   Message ID: 34120
      [quests]                          |
      quest:0 = "Find item"             └── Auto-linked!
    ```

3.  **Backward Compatibility**:
    -   Vanilla quests: Use original description field as message offset
    -   Mod quests: Use generated message ID automatically
    -   Single code path handles both transparently

### 10.2 Modder Workflow

1.  Create `quests_{modname}.txt` with quest definitions
2.  Create `messages_{modname}.txt` with `[quests]` section
3.  Use exact key format: `quest:0`, `quest:1`, etc. (lowercase!)
4.  Check `quests_list.txt` for generated Quest IDs
5.  Check `messages_QUESTS_list.txt` for generated Message IDs
6.  Use Quest IDs for state management, Message IDs for display

### 10.3 Critical Notes for Modders

1.  **Key format is critical**: `quest:0` not `QUEST:0` (lowercase!)
2.  **Index is per-mod**: First quest in file = `quest:0`, second = `quest:1`, etc.
3.  **Message IDs are stable**: Same mod+key = same ID every time
4.  **Check reports**: Always verify generated IDs match between quests and messages

### 10.4 Integration Points

1.  **Script Engine**: Uses quest IDs (200-999) for state management
2.  **PipBoy UI**: Uses message IDs (32768-65535) for description display
3.  **Save System**: Quest states saved via engine's existing mechanism
4.  **Report System**: Cross-references quests with their message IDs

* * * * *

markdown

11\. Holodisk System Specifics
------------------------------

### 11.1 Overview

The holodisk system allows modders to add new holodisks that are automatically converted to vanilla format at load time. Like other systems, it uses stable indexing and integrates seamlessly with the Pip-Boy interface.

### 11.2 File Structure

#### 11.2.1 Holodisk Definition File (`data/holodisk_{modname}.txt`)

Contains the GVAR (global variable) numbers for each holodisk. Each line should contain one GVAR number.

Format:

```
900 # First holodisk - appears when GVAR 900 = 1\
901 # Second holodisk - appears when GVAR 901 = 1
```

Rules:

-   One GVAR per line
-   Comments start with `#`
-   GVARs should be unique (900+ recommended for mods)
-   GVAR must be set to 1 to make holodisk appear in Pip-Boy

#### 11.2.2 Message File Format

Contains the holodisk text in the `[PIPBOY]` section. The `[HOLODISKS]` section is also supported for backward compatibility.

Format:

```
[PIPBOY]\
holodisk:0:name = Important Data Disk\
holodisk:0:line:0 = This holodisk contains critical information\
holodisk:0:line:1 = about the secret facility.\
holodisk:0:line:2 = END-PAR # Paragraph break (optional)\
holodisk:0:line:3 = The entrance is hidden behind the waterfall.\
holodisk:0:line:4 = Coordinates: 38.8977° N, 77.0365° W\
holodisk:0:line:5 = END-DISK # Required (auto-added if missing)
```

Key Formats:

-   Name: `holodisk:{index}:name`
-   Text lines: `holodisk:{index}:line:{line_number}`
-   END-DISK: Marks end of holodisk (required, auto-added if missing)
-   END-PAR: Creates paragraph break (optional)
-   Index starts at 0 for first holodisk in file

Important: Empty values are skipped. Use `**END-PAR**` for blank lines instead of empty strings.

### 11.3 How It Works

#### 11.3.1 Loading Process

1.  Vanilla holodisks load from `data/holodisk.txt` (GVAR, name ID, description ID)
2.  Mod holodisks load from `data/holodisk_*.txt` files
3.  For each mod holodisk:
    -   Extract mod name from filename
    -   Generate a block of 500 consecutive IDs starting at 50000
    -   Look up name and lines from message file
    -   Add to message list with consecutive numeric IDs
    -   Add to holodisk array in vanilla format

#### 11.3.2 Conversion Example

```
holodisk_test2.txt (GVAR 900) + messages_test2.txt\
↓\
Converted to vanilla format:

-   Name: ID 50000

-   Line 0: ID 50001

-   Line 1: ID 50002

-   END-DISK: ID 50003

-   Added to holodisk array: GVAR=900, name=50000, description=50001
```

### 11.4 Script Usage

Give holodisk to player
=======================
```
set_global_var(900, 1)\
display_msg("You found a holodisk!")
```

Check if player has holodisk
============================
```
if global_var(900):\
display_msg("You already have this holodisk.")
```

### 11.5 Special Notes

#### 11.5.1 Empty Lines

The message loading system skips empty values. To create blank lines:

-   Use `**END-PAR**` for paragraph breaks
-   Or use a single space: `holodisk:0:line:2 =` (not recommended)

#### 11.5.2 Pagination

Holodisks use vanilla pagination (35 lines per page). The system automatically:

-   Calculates total pages
-   Adds Back/More buttons as needed
-   Works identically for all holodisks (vanilla and mod)

#### 11.5.3 Localization

Create message files in language folders:

-   English: `data/text/english/game/messages_{modname}.txt`
-   German: `data/text/german/game/messages_{modname}.txt`
-   etc.

#### 11.5.4 GVAR Management

-   GVARs must be unique across all mods
-   Recommended range: 900-999 for small mods, higher for larger mods
-   Set GVAR to 1 to give holodisk, 0 to remove (though typically not removed)

### 11.6 Complete Example Mod

File: `data/holodisk_myquest.txt`

```
900\
901
```

File: `data/text/english/game/messages_myquest.txt`

```
[pipboy]\
holodisk:0:name = Research Data\
holodisk:0:line:0 = Project: Nightfall\
holodisk:0:line:1 = Status: Active\
holodisk:0:line:2 = END-PAR\
holodisk:0:line:3 = Notes: Subjects show increased aggression.\
holodisk:0:line:4 = Recommend immediate termination.\
holodisk:0:line:5 = END-DISK

holodisk:1:name = Security Log\
holodisk:1:line:0 = Unauthorized access detected.\
holodisk:1:line:1 = All systems on high alert.\
holodisk:1:line:2 = END-DISK
```

Script to give holodisks:

In a map script
===============

```
procedure map_enter_p_proc begin\
if not(global_var(900)) then\
set_global_var(900, 1)\
display_msg("Found research data holodisk.")\
end\
end
```

### 11.7 Troubleshooting Holodisks

| Problem | Solution |
| --- | --- |
| Holodisk not appearing | Check GVAR is set to 1 before opening Pip-Boy |
| "Error" text when clicked | Verify message file has correct `[PIPBOY]` section |
| Missing lines | Ensure no empty values; use `**END-PAR**` for blanks |
| Wrong mod name | All files must share same mod name prefix |
| No END-DISK marker | System adds automatically, but include for clarity |
| GVAR conflict | Use unique GVARs (check vanilla GVAR list) |

### 11.8 Best Practices

1.  Test thoroughly: Set GVARs and verify holodisks appear
2.  Use meaningful GVARs: 900, 901, etc. for easy reference
3.  Include END-DISK: Even though auto-added, it's good practice
4.  Check reports: Verify holodisks appear in generated list
5.  Localize: Create message files for all supported languages

* * * * *

12\. Proto System
-----------------

### 12.1 Overview

The proto system allows modders to add new items, critters, scenery, walls, tiles, and miscellaneous objects using the same stable indexing as other systems. All mod protos are automatically integrated into the game's existing proto management system.

### 12.2 File Structure

#### 12.2.1 Proto Definition Files

Mod protos use a two-file system:

```
Fallout 2 Game Directory/\
├── proto/ # Proto definitions\
│ ├── items/ # Item proto directory\
│ │ ├── items_{modname}.lst # List of item protos\
│ │ └── {protoname}.pro # Individual proto files\
│ ├── critters/ # Critter proto directory\
│ │ ├── critters_{modname}.lst\
│ │ └── {protoname}.pro\
│ ├── scenery/ # Scenery proto directory\
│ │ ├── scenery_{modname}.lst\
│ │ └── {protoname}.pro\
│ └── ... (walls, tiles, misc similarly)
```

#### 12.2.2 List File Format (`{type}_{modname}.lst`)

Each line contains one proto name. You can include or omit the .pro extension - both work:

items_testmod.lst:

Format 1: Without extension (cleaner)
=====================================
```
testitem\
advancedgun\
specialarmor
```

Format 2: With extension (vanilla style - also works)
=====================================================
```
[weapon.pro](https://weapon.pro/)\
[armor.pro](https://armor.pro/)\
[drug.pro](https://drug.pro/)
```

critters_testmod.lst:
```
supermutant_elite\
deathclaw_alpha
```

#### 12.2.3 Message File Format for Protos

Add proto messages to your existing `messages_{modname}.txt` file in a section containing "proto" or "pro_":

In messages_testmod.txt
=======================
```
[proto_testmod] # Any section containing 'proto' or 'pro_'\
testmod:testitem:name = Test Mod Item\
testmod:testitem:desc = A test item created by the mod system.\
testmod:advancedgun:name = Advanced Plasma Rifle\
testmod:advancedgun:desc = An upgraded plasma rifle with enhanced damage.
```

Alternative format (using filename mod name):
=============================================
```
testitem:name = Test Mod Item\
testitem:desc = A test item created by the mod system.
```

Message Key Formats:

-   Full format: `{modname}:{protoname}:name` and `{modname}:{protoname}:desc`
-   Short format: `{protoname}:name` and `{protoname}:desc` (uses filename for mod name)

### 12.3 PID Generation

#### 12.3.1 PID Format

-   Vanilla protos: Use indices 0-199 (`0x000000-0x0000C7`) in lower 24 bits
-   Mod protos: Use indices 200-16777215 (`0x0000C8-0xFFFFFF`) via deterministic indexing
-   Full PID: `(type << 24) | index`

Example PID Calculation:
```
// For mod "testmod" with proto "testitem" of type ITEM (0x00)\
Composite key: "testmod:testitem"\
Index: deterministic_index("testmod:testitem") = 0xDB240E\
Final PID: (ITEM_TYPE << 24) | 0xDB240E = 0x00DB240E
```

#### 12.3.2 PID Ranges by Type

-   Items: `0x00XXXXXX` (type 0)
-   Critters: `0x01XXXXXX` (type 1)
-   Scenery: `0x02XXXXXX` (type 2)
-   Walls: `0x03XXXXXX` (type 3)
-   Tiles: `0x04XXXXXX` (type 4)
-   Misc: `0x05XXXXXX` (type 5)

### 12.4 PID Generation vs. .pro File Contents

#### 12.4.1 How It Works

Important: The PID stored in a `.pro` file's first 4 bytes is ignored for mod protos. Our system generates stable PIDs based on mod and proto names.

1.  File Creation: When you create `testitem.pro` with mapper.exe, it might save with PID `0x00000001`
2.  Loading: Our system reads this file, but then:
    -   Generates a new PID: `0x00DB240E` (index of `"testmod:testitem"`)
    -   Ignores the `0x00000001` from the file
    -   Uses `0x00DB240E` for all game operations

#### 12.4.2 What This Means for Modders

-   Don't worry about the PID in the .pro file
-   Do use the PIDs from `proto_list.txt`
-   Don't try to edit .pro file PIDs manually
-   Do use consistent mod/proto names

#### 12.4.3 Example Workflow

1\. Create [testitem.pro](https://testitem.pro/) with mapper.exe (PID = 0x00000001 in file)
===========================================================================================

2\. System loads it, generates PID = 0x00DB240E
===============================================

3\. Check proto_list.txt:
=========================

PID: 0x00DB240E | Mod: testmod | Proto: testitem
================================================

4\. Use 0x00DB240E in scripts, not 0x00000001
=============================================

Script Example - Correct vs. Wrong:

```
// WRONG - using .pro file's internal PID\
int wrong_pid = 0x00000001; // What mapper.exe saved\
obj = create_object(wrong_pid, ...); // Won't work as expected

// CORRECT - using generated PID from proto_list.txt\
int correct_pid = 0x00DB240E; // From proto_list.txt\
obj = create_object(correct_pid, ...); // Works perfectly
```

### 12.5 Creating Mod Protos

Step-by-Step Guide:

1.  Create .lst file for your proto type:

    ```
    # proto/items/items_mymod.lst
    mycustomgun
    specialammo
    uniquearmor
    ```

2.  Create .pro files for each proto (use mapper.exe or existing .pro files as templates):

    ```
    proto/items/mycustomgun.pro
    proto/items/specialammo.pro
    proto/items/uniquearmor.pro
    ```

3.  Add messages in your message file:

    ```
    [proto_mymod]
    mymod:mycustomgun:name = Custom Plasma Pistol
    mymod:mycustomgun:desc = A custom-made plasma pistol.
    mymod:specialammo:name = Enhanced Microfusion Cells
    mymod:specialammo:desc = High-capacity microfusion cells.
    ```

4.  Place files in correct directories
5.  Run game and check `data/lists/proto_list.txt` for generated PIDs

### 12.6 Using Mod Protos in Scripts

#### 12.6.1 Creating Objects with Mod PIDs

```
// Create a mod item at player's feet\
int mod_pid = 0x00DB240E; // From proto_list.txt\
obj = create_object(mod_pid, dude_tile, dude_elevation);

// Add to inventory\
add_mult_objs_to_inven(dude, obj, 1);
```

#### 12.6.2 Getting Proto Information

```
// Get proto name for display\
char* item_name = protoGetName(0x00DB240E);\
display_msg(item_name);

// Get proto description\
char* item_desc = protoGetDescription(0x00DB240E);\
display_msg(item_desc);
```

#### 12.6.3 Checking Proto Properties

```
// Check if item can be picked up\
if (_proto_action_can_pickup(0x00DB240E)) {\
display_msg("You can pick this up");\
}

// Check if item can be used\
if (_proto_action_can_use(0x00DB240E)) {\
display_msg("You can use this item");\
}
```

### 12.7 Generated Reports

The system generates `data/lists/proto_list.txt` with complete information:

```
==============================================================================\
Fallout 2 FISSION - Mod Proto Report\
==============================================================================

MOD PROTO STATISTICS:

* * * * *

Total Mod Protos: 3

By Type:\
items : 2\
critters: 1

ITEMS MOD PROTOS:

* * * * *

PID: 0x00DB240E (14394894)\
Mod: testmod\
Proto: testitem\
File: [testitem.pro](https://testitem.pro/)\
Message: ID 1\
Name: Test Mod Item\
Desc: A test item created by the mod system.\
Item Type: Misc Item\
Weight: 10, Cost: 0

NAME-TO-PID REGISTRY (for message file parsing):

* * * * *

testmod:testitem -> 0x00DB240E\
testmod:advancedgun -> 0x00F8A31C
```

### 12.8 Hash Collision Handling

If two different mods generate the same PID (hash collision), the system:

1.  Shows immediate popup warning with details of the collision
2.  Skips the colliding proto (it will not be loaded)
3.  Logs collision details in the report

Example collision warning:

```
HASH COLLISION WARNING!

Your mod 'mymod' proto 'mygun'\
generated PID 0x00DB240E which is already used by:\
Mod 'othermod' proto 'theirgun'

This proto will be skipped.\
Rename your mod or proto to fix.
```

### 12.9 Special Considerations

#### 12.9.1 Proto File Format

-   .pro files must be in the exact same format as vanilla protos
-   Use mapper.exe to create or edit .pro files
-   Ensure FIDs (Frame IDs) point to existing art

#### 12.9.2 Art Requirements

-   Each proto must have valid art references
-   Art uses its own stable index system
-   Ensure `art/` directory has corresponding .FRM files

#### 12.9.3 Message Loading

-   Proto messages load from the same message files as other content
-   Sections can be named `[proto_{modname}]`, `[pro_items]`, or any containing "proto" or "pro_"
-   Messages are linked automatically via the name-to-PID registry

#### 12.9.4 Save Game Compatibility

-   Mod protos are saved in the same format as vanilla protos
-   Object PIDs in save games reference the generated mod PID
-   Loading saves without the mod will cause missing object errors

### 12.10 Complete Example

File: `proto/items/items_wasteland.lst`

```
plasmacaster\
ripper2\
stimpak_advanced
```

File: `proto/items/plasmacaster.pro` (created with mapper.exe)

File: `text/english/game/messages_wasteland.txt`

```
[proto_wasteland]\
wasteland:plasmacaster:name = Improved Plasma Caster\
wasteland:plasmacaster:desc = A modified plasma caster with enhanced range.

wasteland:ripper2:name = Ripper Mk II\
wasteland:ripper2:desc = An upgraded ripper with serrated blades.

wasteland:stimpak_advanced:name = Advanced Stimpak\
wasteland:stimpak_advanced:desc = Heals more damage than standard stimpaks.
```

Script usage:

```
// Give player the mod item\
int plasmacaster_pid = 0x00A1B2C3; // From proto_list.txt\
obj = create_object(plasmacaster_pid, dude_tile, dude_elevation);\
add_mult_objs_to_inven(dude, obj, 1);

// Display item name\
display_msg(protoGetName(plasmacaster_pid));
```

### 12.11 Best Practices

1.  Test thoroughly: Create objects with mod PIDs and verify they work
2.  Check reports: Always verify generated PIDs in proto_list.txt
3.  Unique names: Use unique mod and proto names to avoid hash collisions
4.  Backup saves: Test with new saves before using existing ones
5.  Art preparation: Ensure all art assets exist before creating protos
6.  Message testing: Verify proto names/descriptions display correctly

### 12.12 Troubleshooting Protos

| Problem | Solution |
| --- | --- |
| Proto shows "Error" name | Check message file has correct proto section and keys |
| Proto not appearing | Verify .lst and .pro files are in correct directories |
| PID collisions | Rename mod or proto to generate different index |
| Missing art | Ensure art files exist and FIDs are correct |
| Save game errors | Test with new saves first; mod protos may not load without mod |
| Wrong item type | Verify .pro file has correct type byte |
| Message not loading | Ensure section contains "proto" or "pro_" |

### 12.13 Integration Points

1.  Object System: Mod protos work with all existing object functions
2.  Inventory: Can be added to inventory like vanilla items
3.  Combat: Weapon and armor protos work in combat system
4.  Scripts: All script functions accept mod proto PIDs
5.  Save/Load: Objects with mod PIDs save and load correctly
6.  UI: Proto names/descriptions display in all interfaces

* * * * *

13\. Troubleshooting
--------------------

### 13.1 Common Issues

#### Core System Issues
| Problem | Likely Cause | Solution | |---------|-------------|----------| | Quest shows "Error" | Missing `[quests]` section or wrong key | Ensure `messages_*.txt` has `[quests]` section with `quest:0` (lowercase) |
| Holodisk not in Pip-Boy | GVAR not set to 1 | Ensure `set_global_var(GVAR, 1)` is called before opening Pip-Boy |
| Holodisk shows "Error" when clicked | Missing `[PIPBOY]` section or wrong key | Check message file has `[PIPBOY]` section with correct keys |
| Area not on world map | `area_name` doesn't match `city_name` | Ensure exact match (case-insensitive but be consistent) |
| Map name shows "Error" | Message key doesn't match generated ID | Check `messages_*.txt` format matches key generation |
| Save games fail | `map_name` >8 characters | Shorten map_name to ≤8 chars |
| Town map labels missing | Wrong section or key format | Use `[worldmap]` section with `entrance_X:{AREA_NAME}` |
#### Script, Art, and Proto Issues
| Problem | Likely Cause | Solution | |---------|-------------|----------| | Script not in `scripts_list.txt` | Index collision or slot >4095 | Rename script file to change index; check for popup warnings |
| Script loads but doesn't execute | Missing `.int` file or wrong filename | Ensure `.int` file exists and name matches entry in `.lst` file |
| Art shows as black/transparent | Missing `.frm` file or wrong FID | Verify `.frm` file exists and FID in `.pro` matches `art_list.txt` |
| Proto shows "Error" name/desc | Missing proto section in messages | Ensure message file has `[proto_{modname}]` section with correct keys |
| Proto not in `proto_list.txt` | Index collision or missing `.pro` file | Rename proto or ensure `.pro` file exists and is in correct directory |
| PID collision warning popup | Index conflict with another mod | Rename mod or proto to change index namespace |
| Script references wrong art | Using wrong index from `art_list.txt` | Double-check art index in `art_list.txt` before using in scripts |
| Localized scripts not loading | Missing `scripts/{language}/` directory | Create language directory with all required files or remove for fallback |
| `.lst` file ignored | Wrong naming convention | Use correct pattern: `scripts_{modname}.lst`, `mod_{modname}.lst`, `items_{modname}.lst` |
#### General Issues
| Problem | Likely Cause | Solution | |---------|-------------|----------| | Mod not loading | File naming mismatch | All files must share same mod name prefix |
| Missing lines in holodisk | Empty values skipped | Use `**END-PAR**` for blank lines |
| Wrong description | Key mismatch (case or index) | Use exact format: lowercase `quest:{index}` or `holodisk:{index}:name` |
| Quest ID collisions | Index collision with another mod | Rename mod file to change namespace |

### 13.2 Debug Commands

To be added later.

### 13.3 Debugging Steps

1.  **Check generated reports** in `data/lists/` - verify all assets appear
2.  **Verify index consistency** - same filenames should produce same IDs
3.  **Test minimal mod** - create simplest possible mod to isolate issues
4.  **Check file permissions** - ensure game can write to `data/lists/`
5.  **Validate file formats** - use text editor with visible whitespace
6.  **Verify GVAR values** - ensure GVARs are set to 1 before opening Pip-Boy
7.  **Check loading order** - areas must load before maps, art before protos

* * * * *

14\. Appendix A: Quick Reference
--------------------------------

### 14.1 File Naming

#### Core Files:
- Areas: `city_{modname}.txt`
- Maps: `maps_{modname}.txt`
- Quests: `quests_{modname}.txt`
- Holodisks: `holodisk_{modname}.txt`
- Messages: `messages_{modname}.txt`

#### Asset List Files:
- Scripts: `scripts/scripts_{modname}.lst`
- Interface Art: `art/intrface/mod_{modname}.lst`
- Item Art: `art/items/mod_{modname}.lst`
- Critter Art: `art/critters/mod_{modname}.lst`
- Items Protos: `proto/items/items_{modname}.lst`
- Critters Protos: `proto/critters/critters_{modname}.lst`
- Scenery Protos: `proto/scenery/scenery_{modname}.lst`
- Walls Protos: `proto/walls/walls_{modname}.lst`
- Tiles Protos: `proto/tiles/tiles_{modname}.lst`
- Misc Protos: `proto/misc/misc_{modname}.lst`

#### Asset Files:
- Compiled Scripts: `{scriptname}.int`
- Art Files: `{artname}.frm`
- Proto Files: `{protoname}.pro`
- Dialog Messages: `dialog/{scriptname}.msg`

### 14.2 Key Formats

#### Message Keys:
- Area names: `area_name:{AREA_NAME}`
- Map names: `lookup_name:{LOOKUP_NAME}:{ELEVATION}`
- Entrance labels: `entrance_{INDEX}:{AREA_NAME}`
- Quest descriptions: `quest:{INDEX}`
- Holodisk names: `holodisk:{INDEX}:name`
- Holodisk lines: `holodisk:{INDEX}:line:{LINE_NUMBER}`

#### Proto Message Keys:
- Full format: `{modname}:{protoname}:name` and `{modname}:{protoname}:desc`
- Short format: `{protoname}:name` and `{protoname}:desc` (uses filename for mod name)

### 14.3 ID Ranges

| Content Type | Vanilla Range | Mod Range | Notes | |--------------|---------------|-----------|-------| | Scripts | 0-? (varies) | vanillaCount-4095 | Deterministic from filename, max 4096 total |
| Items Art | 0-? (varies) | 4096-8191 | Extended range, check `art_list.txt` |
| Critters Art | 0-? (varies) | 4096-8191 | Extended range, check `art_list.txt` |
| Interface Art | 0-? (varies) | 4096-8191 | Extended range, check `art_list.txt` |
| Items Protos | 0x00XXXXXX | 0x0000C8-0xFFFFFF | Type 0, deterministic PIDs |
| Critters Protos | 0x01XXXXXX | 0x0100C8-0xFFFFFF | Type 1, deterministic PIDs |
| Scenery Protos | 0x02XXXXXX | 0x0200C8-0xFFFFFF | Type 2, deterministic PIDs |
| Walls Protos | 0x03XXXXXX | 0x0300C8-0xFFFFFF | Type 3, deterministic PIDs |
| Tiles Protos | 0x04XXXXXX | 0x0400C8-0xFFFFFF | Type 4, deterministic PIDs |
| Misc Protos | 0x05XXXXXX | 0x0500C8-0xFFFFFF | Type 5, deterministic PIDs |
| Quests | 0-199 | 200-999 | Deterministic |
| Maps | 0-199 | 200-1999 | Deterministic |
| Areas | 0-199 | 200-999 | Deterministic |
| Holodisks | 0-? (by GVAR) | 50000+ | Message IDs, blocks of 500 |
| Messages | 0-32767 | 32768-65535 | Deterministic |
### 14.4 Critical Rules

#### Core Rules:
1.  Map names ≤8 characters (DOS 8.3 limitation)
2.  Quest keys lowercase: `quest:0`
3.  Holodisk keys lowercase: `holodisk:0:name`
4.  Area names uppercase in configs
5.  Consistent mod name across all files
6.  Message keys use exact lowercase prefixes

#### Asset Rules:
7.  Script `.lst` files must be in `scripts/` directory
8.  Art `.lst` files use `mod_{modname}.lst` naming in respective `art/` subdirectories
9.  Proto `.lst` files use `{type}_{modname}.lst` naming in respective `proto/` subdirectories
10. Script names in `.lst` files determine their stable index
11. Art filenames in `.lst` files determine their stable index
12. Proto names in `.lst` files determine their stable PID
13. Index collisions skip assets - rename files to resolve
14. Check generated reports (`*_list.txt`) for actual IDs before using them
15. Localized assets go in language subdirectories (optional)

#### Proto-Specific Rules:
16. `.pro` file internal PIDs are ignored - system generates stable PIDs
17. Proto messages must be in sections containing "proto" or "pro_"
18. Use PIDs from `proto_list.txt`, not from `.pro` file contents

* * * * *

15\. Appendix B: Example Mod Structure
--------------------------------------

### 15.1 Complete Example Mod

```
MyFirstMod/\
├── data/\
│ ├── city_myfirst.txt\
│ ├── maps_myfirst.txt\
│ ├── quests_myfirst.txt\
│ └── holodisk_myfirst.txt\
├── text/\
│ └── english/\
│ └── game/\
│ └── messages_myfirst.txt\
├── art/\
│ ├── intrface/\
│ │ ├── mod_myfirst.lst\
│ │ ├── mybutton.frm\
│ │ └── myicon.frm\
│ ├── items/\
│ │ ├── mod_myfirst.lst\
│ │ └── mygun.frm\
│ └── critters/\
│ ├── mod_myfirst.lst\
│ └── newmutant.frm\
├── scripts/\
│ ├── scripts_myfirst.lst\
│ ├── myfirst_[gun.int](https://gun.int/)\
│ ├── myfirst_[door.int](https://door.int/)\
│ └── english/ # Localized version (optional)\
│ ├── scripts_myfirst.lst\
│ └── myfirst_[gun.int](https://gun.int/)\
├── proto/\
│ ├── items/\
│ │ ├── items_myfirst.lst\
│ │ ├── [mygun.pro](https://mygun.pro/)\
│ │ └── [specialarmor.pro](https://specialarmor.pro/)\
│ ├── critters/\
│ │ ├── critters_myfirst.lst\
│ │ └── [newcritter.pro](https://newcritter.pro/)\
│ └── scenery/\
│ ├── scenery_myfirst.lst\
│ └── [mydoor.pro](https://mydoor.pro/)\
├── dialog/ # Script dialog messages (optional)\
│ └── myfirst_gun.msg\
└── README.txt
```

### 15.2 Installation and Testing

1.  **Copy all files** to the correct directories in your Fallout 2 install
2.  **Run the game** and check these generated reports in `data/lists/`:
    - `scripts_list.txt` - Check script indices (e.g., 450, 451)
    - `art_list.txt` - Check art indices for interface, items, critters
    - `proto_list.txt` - Check generated PIDs (e.g., 0x00DB240E, 0x01A1B2C3)
    - `quests_list.txt` - Check quest IDs and message IDs
    - `holodisks_list.txt` - Check holodisk details
    - `area_list.txt` - Check area information
    - `maps_list.txt` - Check map information
3.  **Test in-game**:
    - Travel to coordinates 100,100 on world map to see your area
    - Enter the area to see your map
    - Open Pip-Boy to see quests and holodisks
    - Check inventory for the test item (if created by script)
    - Verify art displays correctly (buttons, icons, items)
    - Test script execution for doors, critters, etc.

### 15.3 Notes for Modders

1.  **Replace example IDs** with actual IDs from your generated reports
2.  **Use mapper.exe** to create or edit `.pro` files for protos
3.  **Test with new saves** before using existing saves
4.  **Check all reports** after loading to verify everything worked
5.  **Use uppercase** for area names in config files for consistency
6.  **Remember map_name ≤8 characters** (DOS 8.3 limitation)
7.  **Localization is optional** - system falls back to default if missing

### 15.4 Generated IDs Reference

After running with this mod, check these reports for actual IDs:

#### From `scripts_list.txt`:

```
MOD SCRIPTS:\
450: myfirst_gun (local_vars=0)\
451: myfirst_door (local_vars=3)
```

#### From `art_list.txt`:

```
INTERFACE ART:\
1560: mybutton.frm (type: interface)\
1561: myicon.frm (type: interface)

ITEMS ART:\
2450: mygun.frm (type: item)

CRITTERS ART:\
3320: newmutant.frm (type: critter)
```

#### From `proto_list.txt`:

```
ITEMS MOD PROTOS:\
PID: 0x00DB240E (14394894)\
Mod: myfirst\
Proto: mygun\
Name: Custom Plasma Rifle\
Desc: A modified plasma rifle.

CRITTERS MOD PROTOS:\
PID: 0x01A1B2C3 (27762499)\
Mod: myfirst\
Proto: newcritter\
Name: Mutant Guardian\
Desc: A heavily armed mutant.
```

#### From `quests_list.txt`:

```
MOD QUESTS:\
Quest ID | Mod | Description Message ID | Quest Key

* * * * *

200 | myfirst | 34120 | myfirst:0\
201 | myfirst | 34121 | myfirst:1
```

#### From `holodisks_list.txt`:

```
MOD HOLODISK DETAILS:\
Block 0: IDs 50000-50499 (GVAR 900) - Research Data\
Block 1: IDs 50500-50999 (GVAR 901) - Security Log
```

### 15.5 Example Script Usage

```
// Give player the mod item using PID from proto_list.txt\
int gun_pid = 0x00DB240E; // From proto_list.txt\
obj = create_object(gun_pid, dude_tile, dude_elevation);\
add_mult_objs_to_inven(dude, obj, 1);

// Display item name using proto message\
display_msg(protoGetName(gun_pid));

// Create critter with mod script (index from scripts_list.txt)\
critter = create_object_sid(0x01A1B2C3, 12345, 0, 450);

// Use art index for custom interface\
display_msg(art_get_name(1560));

// Set quest state\
set_quest(200, 1);

// Display quest description\
display_msg(34120); // From messages_QUESTS_list.txt

// Give holodisk to player\
set_global_var(900, 1);
```

Use these exact IDs in your scripts for a fully working mod!

* * * * *

*Document Version: 2.0 - Modder-Focused Refactor*\
*Last Updated: Fallout 2 FISSION*\
*For more information, see the generated reports in `data/lists/` after running the game.*