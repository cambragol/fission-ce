# VockFeatures config guide

Floats are the text lines that pop up over an NPC's head: combat barks, ambient chatter,
and lines from NPCs who have no dialogue window. In vanilla Fallout 2 they are silent.
VockFeatures adds:

- voice-over audio for floats that have a voice file
- volume that fades with distance
- walls and scenery that muffle a float
- a censor bleep for lines the language filter catches
- optional text garbling for lines you can barely hear

## Turning it on

In **`fission.cfg`**, under `[enhancements]`:

```ini
[enhancements]
StrictVanilla=0
VockFeatures=1
```

`VockFeatures` is off by default. Set it to `1` to turn on everything in this guide, or `0`
to turn it all off. `StrictVanilla=1` turns VockFeatures off whatever `VockFeatures` says.

You tune each feature in **`data/game.cfg`**, under `[vock-features]`:

```ini
[vock-features]
FloatAudioChannels=8
FloatDistancePerPerception=2
FloatObstructionDampening=50
FloatEvictionPolicy=0
FloatAudio=1
FloatCensorBleep=1
FloatVolume=32767
TextScramble=0
TextScrambleDistancePerPerception=4
TextScrambleObstructionDampening=50
TextScrambleChars=#%&*~^
```

Keys that start with `Float` control float audio. Keys that start with `TextScramble`
control text garbling. The two groups don't affect each other.

## Float audio

These settings control the voice-over that plays when a float has a voice file.

### FloatAudio
**Default: `1` (on)**

Plays the voice file for floats that have one. Set it to `0` for silent floats. Distance
and text scrambling keep working either way.

### FloatVolume
**Default: `32767`** (100%)

Scales float voices against your Sound Effects slider. At `32767` floats play as loud as the
slider allows. At `16384` they play at half. Floats can't get louder than the slider, and
muting Sound Effects mutes floats too.

### FloatDistancePerPerception
**Default: `2`**

Sets how far a float's voice carries. The range in tiles is **your Perception × this
number**. Volume stays at 100% for the first half of the range, then fades in a straight
line to silence at the full range.

With the default and Perception 6, you hear floats at full volume out to 6 tiles. They fade
from there and go silent at 12 tiles.

### FloatObstructionDampening
**Default: `50`, range `0` to `100`**

Sets how much a wall or piece of scenery between you and the speaker lowers a float's
volume, as a percentage. At `0`, walls change nothing. At `100`, a blocked float is silent.
Other NPCs standing in the way don't count.

### FloatCensorBleep
**Default: `1` (on)**

A line caught by the language filter never plays its real audio. With this on you hear a
short bleep instead. With it off you hear nothing.

### FloatAudioChannels
**Default: `8`**

Sets how many floats can play voice audio at once. Each NPC uses one channel: a new line
from an NPC who is already talking replaces their old line. Raise this if voices cut off
in crowded scenes.

### FloatEvictionPolicy
**Default: `0`**

Decides what happens when every channel is busy and a new float wants to play:

- **`0` Vanilla**: the new float stays silent.
- **`1` Oldest**: the float that has played longest stops, and the new one takes its channel.
- **`2` Furthest**: the float whose speaker stands furthest from you stops, but only if the
  new speaker is closer.

## Text scrambling

Text scrambling garbles a float's on-screen text when you are far away or behind a wall.
It works whether or not the float has a voice file.

### TextScramble
**Default: `0` (off)**

Turns text scrambling on. Text stays clean out to half its range, then garbles more and
more until it is fully scrambled at the full range. The game swaps letters only and leaves
spaces and punctuation alone, so you can still see where words start and end.

### TextScrambleDistancePerPerception
**Default: `4`**

Sets the text range, the same way `FloatDistancePerPerception` sets the audio range:
**your Perception × this number**, in tiles.

The default is double the audio default, so text stays readable for as long as you can
hear the line. Set it equal to `FloatDistancePerPerception` if you want text to garble at
the same rate the voice fades.

### TextScrambleObstructionDampening
**Default: `50`, range `0` to `100`**

Sets how much a wall garbles the text, as a percentage. It works like
`FloatObstructionDampening`, but you set it separately, so a wall can muffle the voice
without garbling the text.

### TextScrambleChars
**Default: `#%&*~^`**

Lists the characters that replace letters. For example, `TextScrambleChars=*$%^`. Leave it
blank to use the default.

## Pip-Boy holodisk narration

Holodisk narration is not part of VockFeatures. You set it in **`fission.cfg`**, under
`[sound]`, next to the speech settings:

```ini
[sound]
speech=1
speech_volume=22281
pipboy=1
pipboy_volume=22281
```

`pipboy=0` turns holodisk narration off. The Pip-Boy slider in Preferences changes
`pipboy_volume`, and the game saves it when you click **Done**. `StrictVanilla=1` turns
narration off and puts Mouse Sensitivity back in its slider slot.
