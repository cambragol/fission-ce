# VockFloats config guide

What each setting does, in plain terms. For the technical design record (why things work
this way internally, code references, commit history) see `VOCK_FLOATS.md` instead — this
file is just "what does turning this knob do."

## What this is

"Floats" are the text lines that pop up over an NPC's head — combat barks, ambient chatter,
flavor lines from NPCs without a full dialogue window. Normally they're silent text only.
This adds: real voice-over audio for floats that have it, volume that fades with distance,
walls/scenery muffling a float, a censor bleep for filtered lines, and optional text garbling
for lines you can barely hear.

## Turning it on

**`fission.cfg`**, under `[enhancements]`:

```ini
[enhancements]
StrictVanilla=0
VockFloats=1
```

`VockFloats=1` turns the whole thing on. `VockFloats=0` turns it all off. If `StrictVanilla=1`
is set, that overrides `VockFloats` off no matter what it's set to.

**`data/game.cfg`**, under `[vock-floats]` — the individual settings:

```ini
[vock-floats]
FloatAudioChannels=4
DistancePerPerception=2
ObstructionDampening=0
EvictionPolicy=0
VoicedFloats=1
CensorBleep=1
TextScramble=0
TextScrambleChars=#%&*~^
Volume=22281
```

## Each setting

### FloatAudioChannels
**Default: `4`**

How many floats can have voice audio playing at the same time. Each NPC only ever takes up
one channel no matter how many lines it fires — a new line from an NPC that's already
speaking just replaces its own old line. Raise this if floats are getting cut off in busy
scenes with lots of talking NPCs at once.

### DistancePerPerception
**Default: `2`**

Controls how far a float's voice carries before it's silent. The actual range is
**your Perception stat × this number**, in tiles. With the default of `2`: a float fades out
completely by 2× your Perception in tiles. The fade is a straight linear ramp — full volume
right next to the speaker, quieter as you move away, silent at the max range.

Raise this to hear floats from farther away. Lower it to make the game quieter/closer-range.

### ObstructionDampening
**Default: `0` (off) — range `0`–`100`**

How much a solid wall or piece of scenery between you and the speaker muffles a float, as a
percentage. `0` = walls don't matter, a float sounds the same whether it's blocked or not.
`100` = a blocked float is completely silent. Anything in between scales it down
proportionally. Only walls/scenery block sound this way — other NPCs standing between you and
the speaker don't count.

This also affects `TextScramble` below — a blocked line reads harder to make out too, same as
it's harder to hear.

### EvictionPolicy
**Default: `0` (Vanilla/no eviction)**

What happens if every channel (see `FloatAudioChannels`) is already busy and a new float
wants to play:

- **`0` — Vanilla**: the new float just doesn't play. Whatever's already playing keeps going
  untouched.
- **`1` — Oldest**: the float that's been playing longest gets cut off to make room for the
  new one.
- **`2` — Furthest**: whichever currently-playing float's speaker is farthest from you gets
  cut off — but only if the new float's speaker is actually closer. This never makes things
  quieter overall; it just swaps a distant voice for a closer one.

### VoicedFloats
**Default: `1` (on)**

Master toggle for whether floats that have a voice-over file actually play it. Turn this off
to keep the distance/text behavior but go back to silent floats.

### CensorBleep
**Default: `1` (on)**

A line that got caught by the profanity filter never plays its real audio, no matter what
this is set to. This only decides what happens *instead*: `1` = you hear a short censor
"bleep" tone. `0` = you hear nothing at all for that line.

### TextScramble
**Default: `0` (off)**

Garbles the floating text on screen based on the same distance/obstruction math driving the
audio above. Close and clear = text reads fine. Far away or blocked = text degrades into
noise characters — a float you can barely hear also gets hard to read, instead of being
perfectly legible from anywhere on screen.

With the default `DistancePerPerception=2`: text reads perfectly clean out to 1× your
Perception in tiles, then progressively garbles more the farther you get, until it's fully
scrambled by 2× Perception (the same range where the audio itself goes silent).

Only letters get replaced — spaces and punctuation are left alone, so you can still tell
where words start and end even when heavily garbled.

### TextScrambleChars
**Default: `#%&*~^`**

The pool of characters `TextScramble` picks from to replace letters. Change this to whatever
you want the garble to look like, e.g. `TextScrambleChars=*$%^`. If you leave this blank, it
falls back to the default set above.

### Volume
**Default: `22281`** (out of a max of `32767`, i.e. about 68%)

A volume multiplier applied on top of your normal Sound Effects volume slider. This can't
make floats louder than your SFX volume allows, and if you mute SFX entirely, floats go
silent too — it's a multiplier on that slider, not a separate volume channel.
