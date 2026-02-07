# Fallout FISSION Modding: Quick Start Guide
## Get Your First Mod Running in 10 Minutes

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

### Before You Start: Create Your Map
1. **Open Mapper.exe** (in your Fallout 2 tools folder)
2. **Create a new map** or use a simple test map
3. **Save it** with a name ?8 characters (e.g., `mytown1.map`)
   - **Critical**: Map name MUST be 8 characters or less!
4. **Export the .map file** to your Fallout 2 install's `data/maps/` folder

### Step 1: Pick a Mod Name
Choose a short, unique name. Example: `myquest`
This name goes in ALL your filenames.

### Step 2: Create These 4 Files

#### File 1: `data/city_myquest.txt` (Your World Map Area)

```
[Area 0]
area_name = MYTOWN                # UPPERCASE, unique name
world_pos = 400,300               # Where it appears on world map
start_state = On                  # On, Off, or Secret
size = Medium
entrance_0 = On,100,200,MYTOWN1,-1,-1,0  # "MYTOWN1" links to your map's lookup_name
```

#### File 2: `data/maps_myquest.txt` (Your Game Map Definition)

```
[Map 0]
lookup_name = MYTOWN1             # Unique identifier - matches entrance_X above
map_name = mytown1                # MUST match your .map filename (?8 chars!)
music = fs_grand                  # Music track
saved = Yes                       # Game can save here
automap = yes                     # Shows on automap
```

Connection: `entrance_0` in city file references `lookup_name` in maps file.

#### File 3: `text/english/game/messages_myquest.txt` (All Text)

```
[map]
area_name:MYTOWN = My New Town    # What players see on world map
lookup_name:MYTOWN1:0 = Town Square  # What players see when entering

[worldmap]
entrance_0:MYTOWN = Main Gate     # Label on town map

[quests]
quest:0 = Find the hidden artifact
```

#### File 4: `data/quests_myquest.txt` (Your Quest)

```
# Format: location,description,gvar,displayThreshold,completedThreshold
# Note: The 'description' field is ignored - FISSION generates its own IDs
1500, 0, 79, 1, 2
```

Important Relationships:

-   `entrance_X` in city file ? `lookup_name` in maps file

-   `map_name` in maps file ? Your `.map` filename

-   `area_name` in city file ? Message key `area_name:AREA_NAME`

* * * * *

3\. Where to Put All Files
--------------------------

Copy all files to your Fallout 2 install directory:

```
Fallout 2 Game Folder/
??? data/
?   ??? maps/
?   ?   ??? mytown1.map           ? Your created map file
?   ??? city_myquest.txt          ? Put here
?   ??? maps_myquest.txt          ? Put here
?   ??? quests_myquest.txt        ? Put here
??? text/
    ??? english/
        ??? game/
            ??? messages_myquest.txt  ? Put here
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