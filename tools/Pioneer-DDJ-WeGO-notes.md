# Pioneer DDJ-WeGO mapping notes

> **AI-generated text:** this document was written autonomously by an AI agent
> (Claude) from a live MIDI capture session on real DDJ-WeGO hardware and from
> Pioneer's official Traktor mapping. Review before relying on it.

Working notes behind `res/controllers/Pioneer-DDJ-WeGO.midi.xml` and
`res/controllers/Pioneer-DDJ-WeGO-scripts.js`. Based on the stale mapping from
[PR #1346](https://github.com/mixxxdj/mixxx/pull/1346) by Samuel Nicholas,
rewritten for Mixxx 2.5+ with 4-deck support.

## How the map was obtained

1. **Live capture:** every control pressed/moved on the controller while a
   MIDI logger (Python `mido`) recorded the messages, deck A side, with and
   without SHIFT.
2. **Pioneer Traktor mapping (`PIONEER_DDJ-WeGO_V100.tsi`):** the
   `DeviceIO.Config.Controller` entry is base64 of a binary chunk tree.
   `CMAI` chunks hold (binding id, in/out, Traktor command id, `CMAD`
   settings); `DCBM` chunks map binding id to a MIDI name like
   `Ch01.Note.A#2` or `Ch07.CC.019` (`C-1` = note 0). Decoding gave 210 input
   and 258 output messages for all four decks. Every live-captured message
   matched it.
3. **LED probe:** single messages sent to the controller, one at a time,
   observed by eye.

## Key hardware behaviour

- **Deck C/D switching is host-driven.** The DECK button always sends
  `92 72` (left) / `93 72` (right). The controller only starts sending deck
  C/D messages (`92`/`93` channels) after the host lights that LED
  (`92 72 7F`). Sending `92 72 00` switches back to A.
- **SHIFT sends nothing.** The hardware sends different notes while SHIFT is
  held, so the mapping needs no shift state.
- **Jog FX mode is host-driven too.** Pressing CTRL A / FX1-3 / CTRL B only
  sends the button note. Once the host lights that button, the jog stops
  sending scratch messages and instead sends an absolute 14-bit value on
  `B4`/`B5` (MSB first, then LSB). SHIFT+CTRL A/B in FX mode makes the
  hardware emit presses of the active FX buttons ("disable all").
- **Jog ring** shows the value sent back on the same CC as the Jog FX input
  MSB, but only while the matching button is lit: `0x40` = red dot,
  `0x7F` = animation around the ring.
- **Mixer controls are 14-bit** (MSB `0x00-0x1F`, LSB `+0x20`).
- **Encoders** (AUTO LOOP, BROWSE) are relative: `01` = clockwise, `7F` =
  counter-clockwise. Jog is relative around `0x40` (`41` = +1).
- **Tempo slider** sends MSB then LSB; top = `0`, so rate = `1 - value/0x2000`.

## MIDI map

`n` = deck 0..3 (A, B, C, D). Left side = A/C, right side = B/D.

### Deck (status `9n` / `Bn`)

| Control | Normal | SHIFT |
| --- | --- | --- |
| PLAY | `9n 0B` | `9n 47` |
| CUE | `9n 0C` | `9n 48` |
| SYNC | `9n 58` | `9n 5C` |
| HOT CUE 1-4 | `9n 2E-31` | `9n 5F-62` |
| SAMPLER mode button | `9n 59` (hardware switches pad notes itself) | |
| Pads in sampler mode | `9n 3C, 3E, 40, 42` | `9n 3D, 3F, 41, 43` |
| AUTO LOOP push | `9n 14` | `9n 50` |
| AUTO LOOP turn | `Bn 13` | `Bn 4F` |
| Jog touch | `9n 36` | `9n 67` |
| Jog top turn (touching) | `Bn 22` | `Bn 27` |
| Jog edge turn | `Bn 21` | |
| TEMPO (14-bit) | `Bn 00` MSB, `Bn 20` LSB | |
| DECK button | `92 72` left, `93 72` right | |

### Mixer and browser (`96` / `B6`)

| Control | MIDI |
| --- | --- |
| Crossfader | `B6 1F` / `3F` |
| Volume A, B, C, D | `B6 13/33`, `15/35`, `17/37`, `19/39` |
| EQ HI A-D | `B6 07-0A` / `27-2A` |
| EQ MID A-D | `B6 0B-0E` / `2B-2E` |
| EQ LOW A-D | `B6 0F-12` / `2F-32` |
| Headphone MIX | `B6 01` / `21` |
| BROWSE turn / push | `B6 40` / `96 41` |
| SHIFT+BROWSE turn / push | `B6 64` / `96 42` |
| LOAD A-D | `96 46-49`, SHIFT `96 58-5B` |
| Headphone CUE A-D | `96 54-57`, SHIFT `96 5C-5F` |

### Jog FX (`94`/`B4` left, `95`/`B5` right)

| Button | Deck A/B | Deck C/D | SHIFT A/B | SHIFT C/D |
| --- | --- | --- | --- | --- |
| CTRL A | `42` | `47` | `4C` | `51` |
| FX1-3 | `43-45` | `48-4A` | `4D-4F` | `52-54` |
| CTRL B | `46` | `4B` | - | - |

| Jog value (FX mode) | Deck A/B MSB/LSB | Deck C/D MSB/LSB |
| --- | --- | --- |
| CTRL A | `00` / `20` | `08` / `28` |
| FX1, FX2, FX3 | `02/22`, `04/24`, `06/26` | `0A/2A`, `0C/2C`, `0E/2E` |
| CTRL B | `07` / `27` | `0F` / `2F` |

SHIFT in FX mode sends `B4 12/32`, `14/34`, `16/36` (not mapped).

### LEDs

| LED | MIDI | Status |
| --- | --- | --- |
| Button LEDs | same note as the button input | verified |
| DECK C/D | `92 72` / `93 72` | verified, also switches mode |
| VU meter | `9C 28+5n` .. `9C 2C+5n` (5 LEDs per deck) | verified |
| Jog ring value | `B4/B5` + Jog FX MSB CC | verified (needs button lit) |
| Track loaded dot on jog | `9B 0C+n` | verified (red dot) |
| Loop dot on jog | `9B 10+n` | verified (red dot) |
| Jog rotation | `9n 6E` | no visible effect at idle, untested while playing |
| Jog ring segments | `9B 18-4F` | no visible effect at idle |
| Jog FX group/single mode | `9C 7C-7F` | not an LED, unmapped |
| Unknown | `BB 14-17` | no visible effect |

## Mapping choices

- **Jog FX:** deck N drives `[EffectRack1_EffectUnitN]`; init routes deck N
  into it. CTRL A = unit on/off, its jog = dry/wet (`mix`). FX1-3 = effect
  on/off, jog = `meta`, SHIFT = `next_effect`. CTRL B =
  `[QuickEffectRack1_[ChannelN]]` on/off, jog = `super1` (filter).
- **SHIFT layer:** PLAY = censor (`reverseroll`), CUE = `start_stop`,
  SYNC = sync leader, HOT CUE = clear, LOAD = eject, AUTO LOOP push = reloop,
  AUTO LOOP turn = beatjump, jog = fast search, BROWSE turn = scroll x10,
  BROWSE push = `MoveFocusForward`.
- **Sampler pads:** `[Sampler1-4]` on every deck. Press: load if empty,
  else play/stop. SHIFT: stop if playing, else eject.
- **Settings:** auto loop size (default 4 beats), SYNC LED blinks on beat.

## Still to test in Mixxx

1. LEDs of the hidden deck C/D: if they're wrong after switching, the
   controller does not store LED state and the script must resend on toggle.
2. Scratch feel (`jogResolution = 480`, from the old mapping).
3. Tempo slider direction.
4. Jog FX: whether the hardware continues from the ring value the host sends.
5. VU meter (`vu_meter` control) in Mixxx 2.5.
6. `9n 6E` and `9B 18-4F` with a track playing.
7. Right side (B/D): taken from the Traktor mapping, not captured live.

## Installing for testing (Mixxx 2.5)

Copy both mapping files to the user controllers folder (Preferences >
Controllers has a button to open it), restart Mixxx, pick "Pioneer DDJ-WeGO":

- Windows: `%LOCALAPPDATA%\Mixxx\controllers`
- macOS: `~/Library/Containers/org.mixxx.mixxx/Data/Library/Application Support/Mixxx/controllers`
- Linux: `~/.mixxx/controllers`

> **End of AI-generated text.**
