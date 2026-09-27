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
VockFloats=0
```

`VockFloats` is **off by default** — `VockFloats=1` turns the whole thing on, `VockFloats=0`
turns it all off. If `StrictVanilla=1` is set, that overrides `VockFloats` off no matter what
it's set to.

**`data/game.cfg`**, under `[vock-floats]` — the individual settings:

```ini
[vock-floats]
FloatAudioChannels=8
DistancePerPerception=2
ObstructionDampening=50
EvictionPolicy=0
VoicedFloats=1
CensorBleep=1
TextScramble=0
TextScrambleDistancePerPerception=4
TextScrambleChars=#%&*~^
Volume=32767
```

## Each setting

### FloatAudioChannels
**Default: `8`**

How many floats can have voice audio playing at the same time. Each NPC only ever takes up
one channel no matter how many lines it fires — a new line from an NPC that's already
speaking just replaces its own old line. Raise this if floats are getting cut off in busy
scenes with lots of talking NPCs at once.

### DistancePerPerception
**Default: `2`**

Controls how far a float's voice carries before it's silent. The actual range is
**your Perception stat × this number**, in tiles. Volume stays at **100% for the first half**
of that range, then fades in a straight line over the second half until it's silent at the
full range. With the default of `2`: full volume out to 1× your Perception, fading out from
there, silent by 2× your Perception.

Raise this to hear floats from farther away. Lower it to make the game quieter/closer-range.

### ObstructionDampening
**Default: `50` — range `0`–`100`**

How much a solid wall or piece of scenery between you and the speaker muffles a float, as a
percentage. `0` = walls don't matter, a float sounds the same whether it's blocked or not.
`100` = a blocked float is completely silent. Anything in between scales it down
proportionally. Only walls/scenery block sound this way — other NPCs standing between you and
the speaker don't count.

This also affects `TextScramble` below — a blocked line reads harder to make out too, same as
it's harder to hear (both use the same obstruction check, just with their own separate ranges).

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

Garbles the floating text on screen based on distance/obstruction to the speaker — the same
shape of falloff driving the audio above, but its own range (see `TextScrambleDistancePerPerception`
below). Close and clear = text reads fine. Far away or blocked = text degrades into noise
characters — a float you can barely hear also gets hard to read, instead of being perfectly
legible from anywhere on screen.

Only letters get replaced — spaces and punctuation are left alone, so you can still tell
where words start and end even when heavily garbled.

### TextScrambleDistancePerPerception
**Default: `4`**

Same idea as `DistancePerPerception` above, but for text clarity instead of audio — the range
is **your Perception stat × this number**, in tiles, using the exact same shape: perfectly
clean for the first half of the range, then fading over the second half until fully scrambled
at the full range. It's independent of `DistancePerPerception`: it doesn't have to move when
you change the audio range, and vice versa.

With the default (`4`): text stays perfectly clean out to 2× Perception, then ramps to fully
scrambled by 4× Perception. Compare to `DistancePerPerception`'s default (`2`): audio itself
stays at full volume only out to 1× Perception and is fully silent by 2× Perception — so text
stays perfectly clean for as long as the line is audible at all, and doesn't finish garbling
until twice the distance where audio goes silent. Raise `TextScrambleDistancePerPerception`
further to push the clean zone out even more; lower it toward `DistancePerPerception`'s value
to have scramble track volume more tightly (setting them equal makes the two ramps identical).

### TextScrambleChars
**Default: `#%&*~^`**

The pool of characters `TextScramble` picks from to replace letters. Change this to whatever
you want the garble to look like, e.g. `TextScrambleChars=*$%^`. If you leave this blank, it
falls back to the default set above.

### Volume
**Default: `32767`** (max, i.e. 100% — no reduction on top of your SFX slider)

A volume multiplier applied on top of your normal Sound Effects volume slider. This can't
make floats louder than your SFX volume allows, and if you mute SFX entirely, floats go
silent too — it's a multiplier on that slider, not a separate volume channel.
