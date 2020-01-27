script.xponent
==============

> This file is part of the [Mixco framework](http://sinusoid.es/mixco).
> - **View me [on a static web](http://sinusoid.es/mixco/script/maudio_xponent.mixco.html)**
> - **View me [on GitHub](https://github.com/arximboldi/mixco/blob/master/script/maudio_xponent.mixco.litcoffee)**

Mixxx script file for the **M-Audio Xponent** controller. The numbers
in the following picture will be used in the script to describe the
functionality of the controls.

  ![Xponent Layout](http://sinusoid.es/mixco/pic/maudio_xponent.png)

    mixco = require 'mixco'
    {assert} = mixco.util
    c = mixco.control
    b = mixco.behaviour
    v = mixco.value
 
    mixco.script.register module,

        info:
            name:   '[mixco] M-Audio Xponent'
            author: 'Juan Pedro Bolivar Puente <raskolnikov@gnu.org>'
            wiki:   'https://sinusoid.es/mixco/script/maudio_xponent.mixco.html'
            forums: 'https://github.com/arximboldi/mixco/issues'
            description:
                """
                Controller mapping for the M-Audio Xponent DJ controller.
                """

Global section
--------------

Controls that do not have a per-deck functionality.

        constructor: ->
            ccId = (cc) -> c.ccIds cc, 2
            g  = "[Master]"

* **27.** Headphone mix.

            c.input(ccId 0x0D).does g, "headMix"

* **39.** Crossfader.

            c.input(ccId 0x07).does g, "crossfader"

### Effects

Most of knobs and buttons in **24** and **25** are dedicated to
effects.  Some of them are mapped per-deck --see later-- but some are
mapped globally:

* The *first and second knobs* of the *left deck* control the *super*
  and *mix* of the first effect unit.  In the *right deck*, they do
  likewise for the second effect unit.

            c.input(0x0c, 0x00).does \
                b.soft "[EffectRack1_EffectUnit1]", "super1"
            c.input(0x0d, 0x00).does \
                b.soft "[EffectRack1_EffectUnit1]", "mix"

            c.input(0x0c, 0x01).does \
                b.soft "[EffectRack1_EffectUnit2]", "super1"
            c.input(0x0d, 0x01).does \
                b.soft "[EffectRack1_EffectUnit2]", "mix"

* **16.** When in *MIDI mode*, the touch pad can be used to control
  the first two parameters of the first effect.  *MIDI mode* can be
  enabled with the button labeled `MIDI` below the touch pad.
  Otherwise, the touch pad is used to control the computer mouse.

            c.control(0x09, 0x02)
                .does "[EffectRack1_EffectUnit1_Effect1]", "parameter1"
            c.control(0x08, 0x02)
                .does "[EffectRack1_EffectUnit1_Effect1]", "parameter2"

* **17.** and **18.** When in *MIDI mode*, the touch pad buttons *toggle
   the first effect unit* for the first and second deck respectively.

            c.input(c.noteOnIds 0x00, 0x02)
                .does "[EffectRack1_EffectUnit1]", "group_[Channel1]_enable"
            c.input(c.noteOnIds 0x01, 0x02)
                .does "[EffectRack1_EffectUnit1]", "group_[Channel2]_enable"

Per deck controls
-----------------

We add the two decks with the `addDeck(idx)` function. In the
*Xponent*, each MIDI message is repeated per-deck on a different
channel.

            @decks = b.chooser()
            @addDeck 0
            @addDeck 1

        addDeck: (i) ->
            assert i in [0, 1]
            g  = "[Channel#{i+1}]"
            ccId = (cc) -> c.ccIds cc, i
            noteId = (note) -> c.noteIds note, i
            noteOnId = (note) -> c.noteOnIds note, i

* **15.** Shift. It changes the behaviour of some controls.  Note
  that there is a shift button per-deck, which only affects the
  controls of that deck.

            shift = b.modifier()
            c.control(noteId 0x2C).does shift

* **12.** Pre-Fade Listen. Select which deck goes to the headphones.

            c.control(noteOnId 0x14).does @decks.add g, "pfl"

### The mixer


* **20.** Filter and gain kills.

            c.control(noteId 0x08).does g, "filterLowKill"
            c.control(noteId 0x09).does g, "filterMidKill"
            c.control(noteId 0x0A).does g, "filterHighKill"
            c.control(noteId 0x0B).does g, "pregain_toggle"

* **22.** Mixer EQ and gain.

            c.input(ccId 0x08).does g, "filterLow"
            c.input(ccId 0x09).does g, "filterMid"
            c.input(ccId 0x0A).does g, "filterHigh"
            c.input(ccId 0x0B).does b.soft g, "pregain"

* **23.** Per deck volume meters.

            c.output(c.ccIds 0x12+i, 3)
                .does b.mapOut(g, "VuMeter").meter()

* **34.** Sync button. Like the button in the UI, it can be held
  pressed to enable the deck in the *master sync* group.

            c.control(noteId 0x02).does g, "sync_enabled"

* **33.** Deck volume.

            c.input(ccId 0x07).does b.soft g, "volume"

* **38.** Punch-in/transform. While pressed, lets this track be heard
  overriding the crossfader.

            c.control(noteId 0x07).does b.punchIn (0.5-i)

### The transport section

* **29.** Song progress indication. When it approaches the end of the
  playing song it starts blinking.

            c.output(c.ccIds 0x14+i, 3).does b.playhead g

* **30.** Back and forward.

            c.control(noteId 0x21).does g, "back"
            c.control(noteId 0x22).does g, "fwd"

* **31.** Includes several buttons...

- The top buttons with numbers are the *hotcues*. On first press,
  sets the hotcue. On second press, jumps to hotcue. When *shift* is
  held, deletes the hotcue point.

            for idx in [0..4]
                c.control(noteId(0x17 + idx))
                    .when(shift,
                          g, "hotcue_#{idx+1}_clear",
                          g, "hotcue_#{idx+1}_enabled")
                    .else g, "hotcue_#{idx+1}_activate",
                          g, "hotcue_#{idx+1}_enabled"

- The little arrow buttons do *beatjump* -- jump forward or back by
  4 beats. When *shift* is pressed they jump by one beat.

            c.control(noteId 0x1C)
                .when shift, g, "beatjump_1_backward"
                .else g, "beatjump_4_backward"
            c.control(noteId 0x1D)
                .when shift, g, "beatjump_1_forward"
                .else g, "beatjump_4_forward"

- The *lock* button does *key lock* -- i.e. makes tempo changes
  independent of pitch. When *shift* is pressed, it expands/collapses
  the selected browser item.

            c.control(noteId 0x1E)
                .when(shift, "[Playlist]", "ToggleSelectedSidebarItem")
                .else g, "keylock"

- The *plus* (+) button moves the beat grid to align with the current
  play position.

            c.control(noteId 0x1F).does g, "beats_translate_curpos"

- The *minus* (-) button plays the track in reverse.

            c.control(noteId 0x20).does g, "reverse"

* **35.** Cue button.

            c.control(noteId 0x23).does g, "cue_default", g, "cue_indicator"

* **37.** Play/pause button.

            c.control(noteOnId 0x24).does g, "play"


### The looping section

* **36.** This includes several controls to manage loops...

- The *in* and *out* buttons set the loop start and end to the
  current playing position.  When *shift* is pressed, they halve and
  double the current loop size respectively.

            c.control(noteId 0x29)
                .when(shift, g, "loop_halve")
                .else g, "loop_in"
            c.control(noteId 0x2B)
                .when(shift, g, "loop_double")
                .else g, "loop_out"

- The *loop* toggles the current loop on/off whenever there is a loop
  selected.

            c.control(noteOnId 0x2A)
                .does g, "reloop_exit", g, "loop_enabled"
 
- The numbers set and trigger a loop of 4, 8, 16 and 32 beats
  respectively. When *shift*, they set loops of 1/8, 1/2, 1 or 2
  beats long.

            c.control(noteId 0x25)
                .when(shift, g, "beatloop_0.125_activate",
                      g, "beatloop_0.125_enabled")
                .else g, "beatloop_4_activate",
                      g, "beatloop_4_enabled"
            c.control(noteId 0x26)
                .when(shift, g, "beatloop_0.5_activate",
                      g, "beatloop_0.5_enabled")
                .else g, "beatloop_8_activate",
                      g, "beatloop_8_enabled"
            c.control(noteId 0x27)
                .when(shift, g, "beatloop_1_activate",
                      g, "beatloop_1_enabled")
                .else g, "beatloop_16_activate",
                      g, "beatloop_16_enabled"
            c.control(noteId 0x28)
                .when(shift, g, "beatloop_2_activate",
                      g, "beatloop_2_enabled")
                .else g, "beatloop_32_activate",
                      g, "beatloop_32_enabled"

### Effects

* In the **24** group, the *first* and *second* buttons enable the
  first or second effect units for this deck.

            c.control(noteOnId 0x0c)
                .does "[EffectRack1_EffectUnit1]", "group_#{g}_enable"
            c.control(noteOnId 0x0d)
                .does "[EffectRack1_EffectUnit2]", "group_#{g}_enable"

* The *third knob and button* in **24** and **25** enable a *beat
  roll* effect, similar to those of the looping section but with
  resolution controllable with a knob for more drastic effects, and
  the play position is restored when turned off.

            beatloop = b.beatEffect g, 'roll'
            c.input(ccId 0x0e).does beatloop.selector()
            c.control(noteId 0x0e).does beatloop

* The *fourth knob and button* in **24** and **25** enable the quick
  effect knob -- by default mapped to a filter sweep.

            c.input(ccId 0x0f)
                .does "[QuickEffectRack1_#{g}]", 'super1'
            c.control(noteId 0x0f)
                .does "[QuickEffectRack1_#{g}]", 'enabled'

### The wheel and pitch section

* **10.** Toggles *scratch* mode.

            scratchMode = b.switch()
            c.control(noteOnId 0x15).does scratchMode

* **11.** The wheel does different functions...

  - When the deck is stopped, it moves the play position.

  - When *scratch* mode is on, it will stop the song when touched on
  top and control the track play like a vinyl when moved.

  - Otherwise, it can be used to *nudge* the playing speed up or down
  to sync the phase of tracks when the track is playing.

  - When *shift* is pressed, it will scroll through the current list
  of tracks in the browser.

            selectTrackKnobTransform = do ->
                toggle = 1
                (ev) ->
                    val = ev.value - 64
                    toggle -= 1
                    if toggle < 0 then toggle = 3
                    if toggle == 0 then val.sign() else null

            c.control(noteId 0x16)
                .when v.and(v.not(shift), scratchMode), b.scratchEnable i+1

            c.input(ccId 0x16)
                .when(shift, b.map("[Playlist]", "SelectTrackKnob")
                    .transform selectTrackKnobTransform)
                .else.when(scratchMode,
                    b.scratchTick(i+1).options.spread64)
                .else b.map(g, "jog").transform (ev) -> (ev.value - 64) / 8

* **26.** Temporarily nudges the pitch down or up. When **shift**,
they do it in a smaller amount.

            c.control(noteId 0x10)
                .when(shift, g, "rate_temp_down_small")
                .else g, "rate_temp_down"
            c.control(noteId 0x11)
                .when(shift, g, "rate_temp_up_small")
                .else g, "rate_temp_up"

* **32.** Pitch slider, adjusts playing speed.

            c.input(c.pbIds i).does b.soft g, "rate"

* **21.** Custom effects that include...

- The *big cross* (X) button simulates a *brake* effect as if the
  turntable was turned off suddenly. On *shift*, it ejects the track
  from the deck.

            c.control(noteId 0x12)
                .when(shift, g, "eject")
                .else b.brake i+1

- The *big minus* (--) button simulates a *backspin* effect as if the
  vinyl was launched backwards. On *shift*, it loads the selected
  track in the browser into the deck.

            c.control(noteId 0x13)
                .when(shift, g, "LoadSelectedTrack")
                .else b.spinback i+1

Initialization
--------------

Unlike old Mixxx versions, this script initializes the device properly
for light feedback.  The trick of holding the two and key button on
initialization are no longer required.

        preinit: ->
            msg = [0xF0, 0x00, 0x20, 0x08, 0x00, 0x00, 0x63, 0x0E,
                   0x16, 0x40, 0x00, 0x01, 0xF7]
            @mixxx.midi.sendSysexMsg msg, msg.length

        init: ->
            @decks.activate 0

        postshutdown: ->
            msg = [0xF0, 0x00, 0x20, 0x08, 0x00, 0x00, 0x63, 0x0E,
                   0x16, 0x40, 0x00, 0x00, 0xF7]
            @mixxx.midi.sendSysexMsg msg, msg.length

License
-------

>  Copyright (C) 2013 Juan Pedro Bolívar Puente
>
>  This program is free software: you can redistribute it and/or
>  modify it under the terms of the GNU General Public License as
>  published by the Free Software Foundation, either version 3 of the
>  License, or (at your option) any later version.
>
>  This program is distributed in the hope that it will be useful,
>  but WITHOUT ANY WARRANTY; without even the implied warranty of
>  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
>  GNU General Public License for more details.
>
>  You should have received a copy of the GNU General Public License
>  along with this program.  If not, see <http://www.gnu.org/licenses/>.
