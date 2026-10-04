# VockFeatures config guide

What each setting does, in plain terms. For the technical design record (why things work
this way internally, code references, commit history) see `VOCK_FLOATS.md` instead — this
file is just "what does turning this knob do."

## What this is

"Floats" are the text lines that pop up over an NPC's head — combat barks, ambient chatter,
flavor lines from NPCs without a full dialogue window. Normally they're silent text only.
This adds: real voice-over audio for floats that have it, volume that fades with distance,
walls/scenery muffling a float, a censor bleep for filtered lines, optional text garbling
for lines you can barely hear, and voiced narration for Pip-Boy holodisks on their own
dedicated audio channel.

## Turning it on

**`fission.cfg`**, under `[enhancements]`:

```ini
[enhancements]
StrictVanilla=0
VockFeatures=0
```

`VockFeatures` is **off by default** — `VockFeatures=1` turns the whole feature set on
(floats and Pip-Boy narration alike), `VockFeatures=0` turns it all off. If `StrictVanilla=1`
is set, that overrides `VockFeatures` off no matter what it's set to.

**`data/game.cfg`**, under `[vock-features]` — the individual settings:

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
PipboyAudio=1
PipboyVolume=32767
```

Every key here starts with the feature it belongs to: `Float*` for NPC float audio,
`TextScramble*` for the on-screen text garbling, `Pipboy*` for holodisk narration. They're
three independent features that happen to share one config section — none of the `Float*`
settings affect `TextScramble`, and vice versa, even though both are about the same floating
text lines.

## Float audio

These control the actual voice-over audio that plays when an NPC's float has a voice file.

### FloatAudioChannels
**Default: `8`**

How many floats can have voice audio playing at the same time. Each NPC only ever takes up
one channel no matter how many lines it fires — a new line from an NPC that's already
speaking just replaces its own old line. Raise this if floats are getting cut off in busy
scenes with lots of talking NPCs at once.

### FloatDistancePerPerception
**Default: `2`**

Controls how far a float's voice carries before it's silent. The actual range is
**your Perception stat × this number**, in tiles. Volume stays at **100% for the first half**
of that range, then fades in a straight line over the second half until it's silent at the
full range. With the default of `2`: full volume out to 1× your Perception, fading out from
there, silent by 2× your Perception.

Raise this to hear floats from farther away. Lower it to make the game quieter/closer-range.
Independent of `TextScrambleDistancePerPerception` below — the text garbling has its own
range, not derived from this one.

### FloatObstructionDampening
**Default: `50` — range `0`–`100`**

How much a solid wall or piece of scenery between you and the speaker muffles a float's
*audio*, as a percentage. `0` = walls don't matter, a float sounds the same whether it's
blocked or not. `100` = a blocked float is completely silent. Anything in between scales it
down proportionally. Only walls/scenery block sound this way — other NPCs standing between
you and the speaker don't count.

Independent of `TextScrambleObstructionDampening` below — a wall can muffle what you hear
without necessarily garbling what you read, or vice versa, if you tune the two differently.

### FloatEvictionPolicy
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

### FloatAudio
**Default: `1` (on)**

Master toggle for whether floats that have a voice-over file actually play it. Turn this off
to keep the distance/text behavior but go back to silent floats.

### FloatCensorBleep
**Default: `1` (on)**

A line that got caught by the profanity filter never plays its real audio, no matter what
this is set to. This only decides what happens *instead*: `1` = you hear a short censor
"bleep" tone. `0` = you hear nothing at all for that line.

### FloatVolume
**Default: `32767`** (max, i.e. 100% — no reduction on top of your SFX slider)

A volume multiplier applied on top of your normal Sound Effects volume slider. This can't
make floats louder than your SFX volume allows, and if you mute SFX entirely, floats go
silent too — it's a multiplier on that slider, not a separate volume channel.

## Text scrambling

A separate feature from float audio above — it garbles a float's on-screen *text*,
independently of whether that float has a voice file at all or plays it.

### TextScramble
**Default: `0` (off)**

Garbles the floating text on screen based on its own distance/obstruction math (see
`TextScrambleDistancePerPerception`/`TextScrambleObstructionDampening` below) — not the
audio settings above. Close and clear = text reads fine. Far away or blocked = text degrades
into noise characters — a float you can barely hear also gets hard to read, instead of being
perfectly legible from anywhere on screen.

Text stays perfectly clean out to 3/4 of its own range, then progressively garbles more over
the last quarter, until it's fully scrambled at the range itself. Only letters get replaced —
spaces and punctuation are left alone, so you can still tell where words start and end even
when heavily garbled.

### TextScrambleDistancePerPerception
**Default: `4`**

Same idea as `FloatDistancePerPerception` above, but for text clarity instead of audio — the
range is **your Perception stat × this number**, in tiles, using the exact same shape:
perfectly clean for the first half of the range, then fading over the second half until fully
scrambled at the full range. It's independent of `FloatDistancePerPerception`: it doesn't
have to move when you change the audio range, and vice versa.

With the default (`4`): text stays perfectly clean out to 2× Perception, then ramps to fully
scrambled by 4× Perception. Compare to `FloatDistancePerPerception`'s default (`2`): audio
itself stays at full volume only out to 1× Perception and is fully silent by 2× Perception —
so text stays perfectly clean for as long as the line is audible at all, and doesn't finish
garbling until twice the distance where audio goes silent. This is a carried-over default,
not a coincidence — it predates `TextScrambleDistancePerPerception` becoming an independent
setting, back when text clarity's range was always double the audio range. Raise
`TextScrambleDistancePerPerception` further to push the clean zone out even more; lower it
toward `FloatDistancePerPerception`'s value to have scramble track volume more tightly
(setting them equal makes the two ramps identical).

### TextScrambleObstructionDampening
**Default: `50` — range `0`–`100`**

Text's own version of `FloatObstructionDampening` above — same meaning, same wall/scenery
raycast, but its own independent value. Same default (`50`) as the audio setting, but the two
aren't linked — a wall can muffle what you hear without necessarily garbling what you read,
or vice versa, if you tune the two differently.

### TextScrambleChars
**Default: `#%&*~^`**

The pool of characters `TextScramble` picks from to replace letters. Change this to whatever
you want the garble to look like, e.g. `TextScrambleChars=*$%^`. If you leave this blank, it
falls back to the default set above.

## Pip-Boy holodisk narration

### PipboyAudio
**Default: `1` (on)**

Master toggle for voiced Pip-Boy holodisk narration specifically. Independent of
`FloatAudio` above — you can have voiced NPC floats without voiced holodisks, or vice
versa. Holodisk narration plays on its own dedicated audio channel, separate from both NPC
floats and dialogue speech, so it can't be interrupted by (or interrupt) either one. Audio
files for this feature live under `sound/pipboy/`, a sibling of `sound/speech/`
rather than a subfolder of it, since holodisk narration has no critter/head
behind it and isn't dialogue.

### PipboyVolume
**Default: `32767`** (max, i.e. 100% — no reduction on top of your Speech slider)

A volume multiplier applied on top of your normal Speech volume slider, just for Pip-Boy
holodisk narration. Same relationship `FloatVolume` above has to the SFX slider, but layered
onto Speech instead since holodisk narration is spoken dialogue, not an ambient effect.
