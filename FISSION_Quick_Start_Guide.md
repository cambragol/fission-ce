# Fallout FISSION Modding: Quick Start Guide
## Get Your First Mod Running in err... 10 Minutes!

---

## 1\. What is FISSION?

FISSION is a modding system for Fallout 2 that lets you add new content **without editing original game files**. Just add your files alongside the originals.

**Key Benefits:**
- No original file editing - just add new files
- Automatic ID assignment - no manual number wrangling
- Built-in testing reports - see what the game sees
- Works with vanilla saves - your mods are optional

---

## 2\. Your First Mod in 4 Steps

### Before You Start: Pick a Mod Name
Choose a short, unique name. Example: `myquest`
This name goes in ALL your filenames. It will be used to generate unique, stable indexes for your mod assets

### Step 1: Create Your Map
1. **Put F.I.S.S.I.O.N.** in your game directory and run once
2. **Open Mapper.exe** (original location vaires)
3. **Create a new map** or use a simple test map
4. **Save it** with a name -8 characters (e.g., `mytown1.map`)
   - **Critical**: Map name MUST be 8 characters or less!
5. **Create** a `mod_yourmod` folder in the F.I.S.S.I.O.N. created `mods` directory
6. **Export the .map file** to your `data/mods/mod_yourmod/MAPS` folder

### Step 2: Create These 6 Files

#### File 1: `mod_yourmod/data/city_yourmod.txt` (Your World Map Area)

```
[Area 0]
area_name = MYTOWN                # UPPERCASE, unique name
world_pos = 400,300               # Where it appears on world map
start_state = on                  # on or off
size = medium                     # small, medium or large
entrance_0 = On,100,200,MYTOWN1,-1,-1,0  # "MYTOWN1" links to your map's lookup_name in your `maps_yourmod.txt` 
entrance_1 = On,110,210,MYTOWN2,-1,-1,0  # "MYTOWN2" links to second map's lookup_name in your `maps_yourmod.txt`
```

#### File 2: `mod_yourmod/data/maps_yourmod.txt` (Your Game Map Definition)

```
[Map 0]
lookup_name = MYTOWN1             # Unique identifier - matches entrance_X above
map_name = mytown1                # MUST match your .map filename (?8 chars!)
music = fs_grand                  # Music track
saved = yes                       # Game can save here
automap = yes                     # Shows on automap

[Map 1]
lookup_name = MYTOWN2             # Second Map for My Town (optional)        
map_name = mytown2                # MUST match your second .map filename (?8 chars!)      
music = fs_grand                  
saved = yes                       
automap = yes
```

Connection: `entrance_0` in `city_yourmod.txt` file references `lookup_name` in `maps_yourmod.txt` file. **The order of entrances must match lookup_name order**

#### File 3: `mod_yourmod/text/english/game/map_yourmod.msg` (Map elevation names and Areas name)

```
# Elevation names are linked to the maps listed in `maps_yourmod.txt` by the yourmod name in the file name
# Map elevation names for mytown1 (offsets 1,2,3)
{0}{}{My Town Gate}
{1}{}{My Town Backlot}
{2}{}{My Town Slums}

# Map elevation names for mytown2 (offsets 4,5,6)
{3}{}{My Town Base}
{4}{}{My Town Base level 2}
{5}{}{My Town Base basement}

# Area names start at 1500, must match order in `data/city_yourmod.txt`
{1500}{}{My Town Name}
```

#### File 4: `mod_yourmod/text/english/game/worldmap_yourmod.msg` (Area entrance names)

```
# Entrance names are linked to the entrances listed in `city_yourmod.txt` by the yourmod name in the file name
{0}{}{My Town - North Entrance}
{1}{}{My Town - Base}
```

#### File 5: `mod_yourmod/data/quests_yourmod.txt` (Your Quest)

```
# Format: location,description,gvar,displayThreshold,completedThreshold
# Location id for yourmod can be retrieved from `data/lists/area_list.txt` report
# Note: The 'description' field is ignored - FISSION auto hashes and connects ids via the 'yourmod' name
1500, 0, 79, 1, 2
```

#### File 6: `mod_yourmod/text/english/game/quests_yourmod.msg` (Your Quest description)

```
# Description is connected to the quest via the .msg file `yourmod` name
{0}{}{Test Quest: Find the lost artifact}
```

Important Relationships:

-   `entrance_X` in city file = `lookup_name` in maps file

-   `map_name` in maps file = Your `.map` filename

-   `yourmod` name in .txt file names = `yourmod` name in .msg file names

* * * * *

3\. Where to Put All Files
--------------------------

Copy all files to your `mod_yourmod.dat` folder in the mods directory.
`mod_yourmod.dat` can be an 'open' folder, or a compressed .dat file

```
Fallout2/
└─ mods/
   └─ mod_mytown.dat/          ← can be folder or .dat
      ├─ data/
      │  ├─ city_mytown.txt
      │  ├─ maps_mytown.txt
      │  └─ MAPS/
      │     └─ mytown1.map
      └─ text/
         └─ english/
            └─ game/
               ├─ map_mytown.msg
               └─ worldmap_mytown.msg
```

Critical Check:

-   Your `.map` file MUST be in `data/maps/`

-   The `map_name` in `maps_myquest.txt` MUST match the `.map` filename (without extension)

* * * * *

4\. Test Your Mod
-----------------

### Test 1: Check the Reports

Run the game, then check `data/lists/` for these files:

-   `area_list.txt` - Your area "MYTOWN" should be listed

-   `maps_list.txt` - Your map "MYTOWN1" should be listed

-   `quests_list.txt` - Your quest should be listed

If all three appear in the reports, your mod loaded successfully!

### Test 2: Play Your Mod

1.  Start a new game or load a save

2.  Open world map (M key)

3.  Travel to coordinates 400,300

4.  You should see "My New Town" on the map

5.  Click to enter - you'll load into your map "mytown1"

6.  The location name should show "Town Square"

7.  Open Pip-Boy (P key) ? Quests tab

8.  You should see "Find the hidden artifact"

If you can enter and walk around your map, your mod is working!

* * * * *

5\. What's Next?
----------------

### Add More Maps to Your Area

Add more entrances to `city_myquest.txt`:

```
entrance_1 = On,200,200,MYTOWN2,-1,-1,0
```

Add corresponding map definitions in `maps_myquest.txt`:

```
[Map 1]
lookup_name = MYTOWN2
map_name = mytown2
music = f1_dsrt
saved = Yes
automap = yes
```

### Add More Content Types

Once your basic area works, you can add:

-   Holodisks: `data/holodisk_myquest.txt`

-   Scripts: `scripts/scripts_myquest.lst` + `.int` files

-   Art: `art/intrface/mod_myquest.lst` + `.frm` files

-   Items: `proto/items/items_myquest.lst` + `.pro` files

### Use Generated IDs

Check the reports in `data/lists/` to find your generated IDs:

-   Use quest IDs with `op_set_quest(ID, state)`

-   Use message IDs with `display_msg(ID)`

### Learn More

For detailed information on all FISSION features, see the full FISSION Modding Reference Guide.

* * * * *

Quick Troubleshooting
---------------------

| Problem | Solution |
| --- | --- |
| "Cannot load map" error | Check `map_name` exactly matches your `.map` filename |
| Area not on world map | Check `area_list.txt` - if missing, format error in city file |
| Can't enter area | Check `entrance_X` references correct `lookup_name` |
| Map name shows "Error" | Check `messages_myquest.txt` has correct `lookup_name:` line |
| No reports generated | Game didn't load mod - check file names and locations |

Common Mistake: Map names longer than 8 characters will fail. Keep them short!

Need Help? Check the full reference guide for detailed troubleshooting and advanced features.