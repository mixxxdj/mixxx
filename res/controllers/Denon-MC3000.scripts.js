/**
 * Denon MC3000 controller script for Mixxx
 *
 * Based on BeMixxx's mapping for the Denon MC3000.
 * Updated for mixxx 2.6 by dtavan:
 *   Ported to the per-deck MIDI-channel architecture, added full 4-deck
 *   support (loop and hotcue functions now also work on decks C/D), updated
 *   to the current EffectRack/beatloop/QuickEffect control set, and
 *   implemented the MC3000's LED feedback protocol.
 *
 * Special Thanks to the Programmers of Mixxx and all the contributors.
 *
 * Copyright (c) 2012 Bertrand Espern (BeMixxx)
 * Copyright (c) 2026 Damien Tavan (dtavan)
 * Licensed under the GNU General Public License, version 2 or later.
 *
 * Sections:
 *  1. GLOBAL VARIABLES & CONFIG
 *  2. INIT / SHUTDOWN
 *  3. LED PRIMITIVES
 *  4. COMMON HELPERS
 *  5. PLAYLIST / TRACK SELECT
 *  6. CUE AND HOTCUE
 *  7. LOOP CONTROL
 *  8. FX SECTION (EFX buttons/knobs, SAMP pads)
 *  9. LED CALLBACKS
 *
 * Conventions:
 *  - Functions bound directly from a control's <key> in the XML are called
 *    by Mixxx as (channel, control, value, status, group) with the raw MIDI
 *    bytes -- they must be plain functions, never a curried factory.
 *  - Functions used as engine.makeConnection() callbacks are called as
 *    (value, group, control) instead -- a different shape, easy to confuse
 *    with the one above.
 *  - A few functions here (fxEnableLed, fxOnLed, sampLed) are factories:
 *    called once at init() with setup args, they return the actual
 *    connection callback as a closure.
 *  - Unused parameters are prefixed with _ (kept only to preserve the
 *    required call signature).
 *
 **/

var mc3000 = function() {};

// ---------- 1. GLOBAL VARIABLES & CONFIG ----------

mc3000.deck2ch = [0,2,1,3];

mc3000.nearEnd = [0,0,0,0];

mc3000.loophotcue = [0,0,0,0];
mc3000.loophotcuebeatnb = [8,8,8,8];

mc3000.hotcues = {
    23: 1, 24: 2, 25: 3, 32: 4, 72: 5, 73: 6, 74: 7, 75: 8,
};

mc3000.leds = {
    vinylmode: 6, keylock: 8, sync: 9, cue: 38, play: 39, fxon1: 90, fxon2: 91,
    cue1: 17, cue2: 19, cue3: 21, cue4: 23, cue5: 48, cue6: 50, cue7: 52, cue8: 54,
    loopin: 36, loopout: 64, autoloop: 43, pfl1: 69, pfl2: 81, pfl3: 75, pfl4: 87,
    efx1L: 92, efx2L: 93, efx3L: 94, efx4L: 95, efx1R: 96, efx2R: 97, efx3R: 98, efx4R: 99,
};

// which of A/C, B/D is currently displayed -- see mc3000.trackDeck
mc3000.state = {
    shift1: false, shift2: false, alshift1: false, alshift2: false, pfl1: 0, pfl2: 0,
    leftDeck: 1, rightDeck: 2,
};


mc3000.vumetermaxled = [0,0,0,0];
mc3000.vumeteroffset = [9,41,25,57];

mc3000.scratch =[false,false,false,false];
//mc3000.scratch =[true,true,true,true];

mc3000.scratchpressed = [false,false,false,false];
mc3000.alpha =  (1.0/8);
mc3000.beta = mc3000.alpha/32;
mc3000.jogSensitivity = 2.0;
mc3000.scratchSensitivity = 1;

// BeatLoop 2 4 8 16 to 1 2 3 4

// ---------- 2. INIT / SHUTDOWN ----------

// called when the MIDI device is opened & set up
mc3000.init = function(id) {

    mc3000.id = id;

    var i=0;

    // mc3000.rate is script-driven (reconstructs a 14-bit value from the
    // pitch bend message and calls engine.setValue), so soft-takeover has to
    // be enabled here in JS -- it has no effect on XML-only <normal/> bindings.
    for (i=1;i<=4;i++) {
        engine.softTakeover("[Channel"+i+"]", "rate", true);
    }

    var d;
    for (i=1;i<=8;i++) {
        for (d=1;d<=4;d++) {
            engine.makeConnection("[Channel"+d+"]","hotcue_"+i+"_status", mc3000.hotcueSetLed);
        }
    }

    for (i=1;i<=4;i++) {
        engine.makeConnection("[Channel"+i+"]", "keylock", mc3000.keylockSetLed);
        engine.makeConnection("[Channel"+i+"]", "play", mc3000.playSetLed);
        engine.makeConnection("[Channel"+i+"]", "playposition", mc3000.playPositionSetLed);
        engine.makeConnection("[Channel"+i+"]", "peak_indicator", mc3000.peakIndicatorSetLed);
        engine.makeConnection("[Channel"+i+"]", "vu_meter", mc3000.vuMeterSetLeds);
        engine.makeConnection("[Channel"+i+"]", "pfl", mc3000.pflSetLed);
    }

    for (i=1;i<=4;i++) {
        engine.makeConnection("[Channel"+i+"]", "loop_start_position", mc3000.loopStartSetLed);
        engine.makeConnection("[Channel"+i+"]", "loop_end_position", mc3000.loopEndSetLed);
        engine.makeConnection("[Channel"+i+"]", "loop_enabled", mc3000.loopEnableSetLed);
        // NOTE: beatloop_X_enabled -> EFX LED connections removed. The EFX buttons
        // are now FX/Sampler controls, so their LEDs are driven by mc3000.fxEnableLed.
    }

    mc3000.allLedOff();

    // FX effect-enable -> EFX button LEDs (both sides, EffectUnit1 & EffectUnit2).
    // .trigger() initialises each LED to the effect's current state.
    var fxs, fxn;
    for (fxs=1; fxs<=2; fxs++) {
        for (fxn=1; fxn<=4; fxn++) {
            engine.makeConnection("[EffectRack1_EffectUnit"+fxs+"_Effect"+fxn+"]",
                "enabled", mc3000.fxEnableLed(fxs, fxn)).trigger();
        }
    }

    // FX-ON (assign unit to deck) -> FX ON button LEDs. Under the uniform
    // re-channel model the FX-ON button for deck N arrives on that deck's channel,
    // so the LED for that deck is sent on the same channel (deck2ch handles it).
    // Force a clean slate first: Mixxx persists FX-unit-to-channel assignment
    // in its own config (or ships with a default one), so without this, a
    // leftover assignment from a previous session can make EffectUnit1/2
    // already routed to a deck before you've touched anything -- confusing,
    // since the controller itself never asked for that. Unassign both units
    // from all 4 channels on every load so FX ON always starts clean.
    var fxd, fxu;
    for (fxu=1; fxu<=2; fxu++) {
        for (fxd=1; fxd<=4; fxd++) {
            engine.setValue("[EffectRack1_EffectUnit"+fxu+"]", "group_[Channel"+fxd+"]_enable", 0);
        }
    }
    for (fxd=1; fxd<=4; fxd++) {
        for (fxu=1; fxu<=2; fxu++) {
            engine.makeConnection("[EffectRack1_EffectUnit"+fxu+"]",
                "group_[Channel"+fxd+"]_enable", mc3000.fxOnLed(fxd, fxu)).trigger();
        }
    }

    // Sampler pad LEDs: light the SAMP pad while a sample is loaded (armed).
    // LED ids and channels from the MC3000 reception table (left on ch1, right on ch3).
    var sledL = [25,27,29,32], sledR = [65,67,69,71];
    for (i=1;i<=4;i++) {
        engine.makeConnection("[Sampler"+i+"]", "track_loaded", mc3000.sampLed(1, sledL[i-1])).trigger();
        engine.makeConnection("[Sampler"+(i+4)+"]", "track_loaded", mc3000.sampLed(2, sledR[i-1])).trigger();
    }
};

// Called when the MIDI device is closed
mc3000.shutdown = function(_id) {
    mc3000.allLedOff();
};

// ---------- 3. LED PRIMITIVES ----------

mc3000.allLedOff = function() {
    // All leds off for all 4 decks
    for (var led in mc3000.leds) {
        for (var dk=1; dk<=4; dk++) {
            mc3000.setled(dk,mc3000.leds[led],0);
        }
    }
    // Pfl leds use the 80/81 protocol (setled2)
    for (var p=1; p<=4; p++) {
        mc3000.setled2(p,mc3000.leds["pfl"+p],0);
    }
    // Sampler pad LEDs off (left ids on ch1, right ids on ch3)
    var offL=[25,27,29,32], offR=[65,67,69,71];
    for (var s=0; s<4; s++) {
        mc3000.setled(1,offL[s],0);
        mc3000.setled(2,offR[s],0);
    }

    // Vinylmode LED reflects each deck's mc3000.scratch default state
    var i=0;
    for (i=1;i<=4;i++) {
        mc3000.setled(i,mc3000.leds["vinylmode"],mc3000.scratch[i-1]);
    }
};

mc3000.setled = function(deck,led,status) {
    var ledStatus = 75; // Default OFF
    switch (status) {
        case 0: ledStatus = 75; break; // OFF
        case false: ledStatus = 75; break; // OFF
        case true: ledStatus = 74; break; // ON
        case 1: ledStatus = 74; break; // ON
        case 2: ledStatus = 76; break; // BLINK
    }
    midi.sendShortMsg(0xB0+mc3000.deck2ch[deck-1], ledStatus, led);
};

mc3000.setled2 = function(deck,led,status) {
    midi.sendShortMsg(0xB0+mc3000.deck2ch[deck-1], status === 1 ? 80 : 81, led);
};

// ---------- 4. COMMON HELPERS ----------

mc3000.groupToDeck = function(group) {
    var matches = group.match(/^\[Channel(\d+)\]$/);
    if (matches === null) {
        return -1;
    } else {
        return parseInt(matches[1], 10);
    }
};

mc3000.samplesPerBeat = function(group) {
    var sampleRate = engine.getValue(group, "track_samplerate");
    var channels = 2;
    var bpm = engine.getValue(group, "file_bpm");
    return channels * sampleRate * 60 / bpm;
};

// Pitch Rate MSB/LSB
mc3000.rate = function(_channel, control, value, _status, group) {
    // 0..16383 -> -1..1
    var MSB = value;
    var LSB = control;
    var num14 = (MSB << 7) | LSB;  // Construct the 14-bit number
    var nrate = (num14-8192)/8192;
    engine.setValue(group, "rate", nrate);
};

// NOTE: MIC LEVEL, MONITOR PAN/PHONES, LINE TO MASTER and MASTER LEVEL are
// fully analog on this hardware (confirmed by the MC3000 manual's MIDI
// command list, which does not list them as sending MIDI) -- there is
// nothing for Mixxx to bind them to.

// ---------- 5. PLAYLIST / TRACK SELECT ----------
mc3000.selectKnob = function(_channel, _control, value, _status, group) {
    var offset = value === 0 ? 250 : -250;
    var deck;

    // TRACK SELECT KNOB is a single global control, but SHIFT/AUTO LOOP can be
    // held on any of the 4 decks (2 physical SHIFT buttons, each covering
    // whichever deck is currently active on that side) -- check all 4, not
    // just decks 1/2.

    // AUTOLOOP + SelectKnob ? -> MOVE LOOP
    var anyAlshift = false;
    for (deck=1; deck<=4; deck++) {
        if (mc3000.state["alshift"+deck]) {
            anyAlshift = true;
            mc3000.moveLoop("[Channel"+deck+"]",offset);
        }
    }
    if (anyAlshift) {
        return;
    }

    // SHIFT + SelectKnob ? -> MOVE CUE
    var anyShift = false;
    for (deck=1; deck<=4; deck++) {
        if (mc3000.state["shift"+deck]) {
            anyShift = true;
            var g = "[Channel"+deck+"]";
            var curpos = engine.getValue(g,"cue_point");
            engine.setValue(g,"cue_point",curpos+offset);
        }
    }
    if (anyShift) {
        return;
    }

    // NORMAL MODE - NEXT/PREV TRACK
    if (value === 0) {
        engine.setValue(group, "SelectNextTrack", 1);
    } else {
        engine.setValue(group, "SelectPrevTrack", 1);
    }
};

// LOAD A/B are fixed-channel buttons (confirmed by field testing: binding them
// per-deck like a normal transport control only ever worked for deck A). The
// DECK CHG A/B/C/D buttons would be the ideal trigger for this, but their real
// MIDI signal is still unconfirmed (the documented NOTE ON 0x90, notes
// 0x03/0x08/0x09/0x0A never fires on this unit -- verified live). Until that is
// nailed down, this infers the active deck from other controls that are already
// known to work correctly per-deck (jog wheel touch, hotcues, bend, loop, EFX,
// shift...): call mc3000.trackDeck(deck) from those INPUT handlers (never from
// LED/connection callbacks -- those fire for decks that aren't currently
// on-screen too, e.g. a background VU meter update, which would corrupt the
// tracking). Whichever deck's controls were touched most recently on a given
// side is which deck LOAD for that side targets.
mc3000.trackDeck = function(deck) {
    if (deck === 1 || deck === 3) {
        mc3000.state.leftDeck = deck;
    } else if (deck === 2 || deck === 4) {
        mc3000.state.rightDeck = deck;
    }
};

// LOAD A (note 0x62) and LOAD B (note 0x63) -- two independent physical buttons.
// LOAD A loads onto whichever of A/C is currently active on the left; LOAD B
// loads onto whichever of B/D is currently active on the right. Both are also
// fixed-channel, so route through mc3000.trackDeck's tracking rather than a
// single hardcoded [ChannelN].
mc3000.loadSelected = function(_channel, control, value, status, _group) {
    if ((status & 0xF0) !== 0x90 || value === 0) {
        return;   // on real press only
    }
    var deck = control === 0x62 ? mc3000.state.leftDeck : mc3000.state.rightDeck;
    engine.setValue("[Channel"+deck+"]", "LoadSelectedTrack", 1);
};

// NOTE (FLANGER, removed): the old lfoDepth / lfoDelay / lfoPeriod controls
// were removed in Mixxx 2.0's effects rewrite. Those handlers were unbound
// and are intentionally deleted so nothing calls a control that no longer
// exists.

// ---------- 6. CUE AND HOTCUE ----------
mc3000.cueDefault = function(_channel, _control, _value, status, group) {
    var isPressed = (status & 0xF0) === 0x90 ? 1 : 0;
    mc3000.trackDeck(mc3000.groupToDeck(group));
    engine.setValue(group,"cue_default", isPressed);
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["cue"],isPressed);
};

mc3000.hotcueset = function(_channel, control, _value, status, group) {
    var deck=mc3000.groupToDeck(group);
    mc3000.trackDeck(deck);
    var nocue=mc3000.hotcues[control];
    var isPlay = engine.getValue(group,"play") === 1 ? true : false;

    if ((status & 0xF0) !== 0x90) {    // Is Released ?
        if (mc3000.state["alshift"+deck] || mc3000.state["shift"+deck]) {
            return; // DO NOTHING ON RELEASE IF ALSHIFT OR SHIFT
        }
        engine.setValue(group,"hotcue_"+nocue+"_activate",0);
        return;
    }

    // AUTOLOOP + HOTCUE MODE ?
    if (mc3000.state["alshift"+deck]) {
        if (engine.getValue(group,"hotcue_"+nocue+"_status")) {
            mc3000.setLoopAtHotCue(group,nocue,mc3000.loophotcuebeatnb[deck-1]);
            if (!isPlay) {
                engine.setValue(group, "reloop_exit", 1);
            }
        }
        return;
    }

    // WITH SHIFT ?
    if (mc3000.state["shift"+deck]) {
        engine.setValue(group,"hotcue_"+nocue+"_clear",1);
        return;
    }

    if (isPlay) {
        // LOOP HOTCUE OR NORMAL HOTCUE ?
        if (mc3000.loophotcue[deck-1] === nocue) {
            mc3000.loophotcue[deck-1]=0;
            engine.setValue(group, "reloop_exit", 1);
        } else {
            engine.setValue(group,"hotcue_"+nocue+"_activate",1);
        }
    } else {
        engine.setValue(group,"hotcue_"+nocue+"_activate",1);
    }
};

// ==== Set a loop at the hotcue_X_position
mc3000.setLoopAtHotCue = function(group,nohotcue,nbbeat) {
    var position = engine.getValue(group,"hotcue_"+nohotcue+"_position");
    var deck = mc3000.groupToDeck(group);

    // Old HotCue Led Off if exist
    if (mc3000.loophotcue[deck-1]>0) {
        mc3000.hotcueSetLed(0,group,"hotcue_"+mc3000.loophotcue[deck-1]);
    }

    // if loop set, exit from loop
    if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "reloop_exit", 1);
    }

    // Set new loop_start
    engine.setValue(group, "loop_start_position", position);
    // Delete loop_end
    engine.setValue(group, "loop_end_position", -1);
    // Set Loop End
    engine.setValue(group, "loop_end_position", position+(mc3000.samplesPerBeat(group)*nbbeat));

    mc3000.loophotcue[deck-1]=nohotcue;

    // New HotCue Led Blink
    mc3000.hotcueSetLed(2,group,"hotcue_"+nohotcue);
};

mc3000.shift = function(_channel, _control, _value, status, group) {
    var deck = mc3000.groupToDeck(group);
    mc3000.trackDeck(deck);
    mc3000.state["shift"+deck] = (status & 0xF0) === 0x90 ? true : false; // 1 if pressed
};

mc3000.vinylmode = function(_channel, _control, _value, _status, group) {
    var deck = mc3000.groupToDeck(group);
    mc3000.trackDeck(deck);
    mc3000.scratch[deck-1] = !mc3000.scratch[deck-1];
};

// JOG with WHEELTOUCH
mc3000.wheelTouch = function(_channel, _control, _value, status, group) {
    var deck = mc3000.groupToDeck(group);
    mc3000.trackDeck(deck);
    if (mc3000.scratch[deck-1]) {
        if ((status & 0xF0) === 0x90) {    // Is pressed ?
            engine.scratchEnable(deck, 100, 330, mc3000.alpha, mc3000.beta); // 128, 33+1/3 aussi
        } else {
            engine.scratchDisable(deck);
        }
        mc3000.scratchpressed[deck-1] = (status & 0xF0) === 0x90 ? true : false;
    }
};

// The wheel that actually controls the scratching
mc3000.jogWheel = function(_channel, _control, value, _status, group) {
    var deck = mc3000.groupToDeck(group);
    mc3000.trackDeck(deck);
    var adjustedJog = value - 64; // Control centers on 0x40 (64)

    if (mc3000.scratch[deck-1]) {
        if (mc3000.scratchpressed[deck-1]) {
            engine.scratchTick(deck,adjustedJog/mc3000.scratchSensitivity);
            return;
        }
    }

    var gammaInputRange = 64;    // Max jog speed
    var maxOutFraction = 0.5;    // Where on the curve it should peak; 0.5 is half-way
    var sensitivity = 0.5;        // Adjustment gamma 0.5 def
    var gammaOutputRange = 3;    // Max rate change
    adjustedJog = gammaOutputRange * adjustedJog / (gammaInputRange * maxOutFraction);
    engine.setValue(group, "jog", adjustedJog/mc3000.jogSensitivity);
};

// RATE TEMP UP if PLAY else FWD / END
mc3000.bendUp = function(_channel, _control, _value, status, group) {
    var isPressed = (status & 0xF0) === 0x90 ? 1 : 0;
    var deck = mc3000.groupToDeck(group);
    mc3000.trackDeck(deck);
    if (engine.getValue(group,"play") === 1) {
        engine.setValue(group, "rate_temp_up", isPressed);
    } else {
        if ((mc3000.state["shift"+deck]) && isPressed) {
            engine.setValue(group,"end",1);
        } else {
            engine.setValue(group, "fwd",isPressed);
        }
    }
};

// RATE TEMP DOWN if PLAY else BACK / START
mc3000.bendDown = function(_channel, _control, _value, status, group) {
    var isPressed = (status & 0xF0) === 0x90 ? 1 : 0;
    var deck = mc3000.groupToDeck(group);
    mc3000.trackDeck(deck);

    if (engine.getValue(group,"play") === 1) {
        engine.setValue(group, "rate_temp_down", isPressed);
    } else {
        if ((mc3000.state["shift"+deck]) && isPressed) {
            engine.setValue(group,"start",1);
        } else {
            engine.setValue(group, "back", isPressed);
        }
    }
};

// ---------- 7. LOOP CONTROL ----------

// Shift and autoloop : delete loop point ELSE toggle a beat-synced auto loop
mc3000.autoLoop = function(_channel, _control, _value, status, group) {
    var deck = mc3000.groupToDeck(group);
    mc3000.trackDeck(deck);
    var i=0;
    if ((status & 0xF0) === 0x90) {
        // LIGHT LED CUE_X EN MODE CUE LOOP
        for (i=1;i<=8;i++) {
            mc3000.hotcueSetLed(0,group,"hotcue_"+i);
        }
        if (mc3000.loophotcue[deck-1] > 0) {
            mc3000.hotcueSetLed(2,group,"hotcue_"+mc3000.loophotcue[deck-1]);
        }

        // DELETE LOOP POINTS IF SHIFT, ELSE TOGGLE A BEAT-SYNCED AUTO LOOP.
        // beatloop_activate is NOT a toggle by itself -- per the Mixxx docs it
        // only ever "sets a loop ... and enables it". To get real toggle
        // behavior (press to engage and keep looping after release, press
        // again to release) we have to check loop_enabled ourselves, the same
        // way loopIn/loopOut already do below.
        if (mc3000.state["shift"+deck]) {
            engine.setValue(group, "reloop_exit", 1);
            engine.setValue(group, "loop_end_position", -1);
            engine.setValue(group, "loop_start_position", -1);
        } else if (engine.getValue(group, "loop_enabled")) {
            engine.setValue(group, "reloop_exit", 1);
        } else {
            engine.setValue(group, "beatloop_activate", 1);
        }

    } else { //released
        // RESTORE LED STATE CUE_X
        for (i=1;i<=8;i++) {
            // hotcue_status is a tri-state (0=unset, 1=set, 2=active/looping) --
            // pass it straight through so an actively-looping hot cue keeps
            // blinking instead of collapsing to solid-on.
            mc3000.hotcueSetLed(engine.getValue(group,"hotcue_"+i+"_status"), group, "hotcue_"+i);
        }
        // NOTE: release used to compare loop_start_position against a snapshot
        // taken on press and call reloop_exit if it had "moved", meant to
        // commit a loop moved via the track-select knob while held (see
        // mc3000.moveLoop). In practice this fired on nearly every plain
        // press+release too, killing the loop the instant the button was
        // released -- that was the actual bug. Loop engage/release is now
        // handled entirely on press above, so release only restores LEDs.
    }

    mc3000.state["alshift"+deck] = (status & 0xF0) === 0x90 ? true : false; // true if pressed
};

mc3000.loopCutM = function(_channel, _control, _value, _status, group) {
    // loop_halve halves beatloop_size, and also resizes the active loop if its
    // length currently equals beatloop_size. Safe to call with no loop active.
    mc3000.trackDeck(mc3000.groupToDeck(group));
    engine.setValue(group, "loop_halve", 1);
};

mc3000.loopCutP = function(_channel, _control, _value, _status, group) {
    // loop_double doubles beatloop_size, and also resizes the active loop if its
    // length currently equals beatloop_size. Safe to call with no loop active.
    mc3000.trackDeck(mc3000.groupToDeck(group));
    engine.setValue(group, "loop_double", 1);
};

mc3000.loopIn = function(_channel, _control, _value, _status, group) {
    mc3000.trackDeck(mc3000.groupToDeck(group));
    if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "reloop_exit", 1);
    }
    engine.setValue(group, "loop_in", 1);
    engine.setValue(group, "loop_end_position", -1);
};

mc3000.loopOut = function(_channel, _control, _value, _status, group) {
    mc3000.trackDeck(mc3000.groupToDeck(group));
    var start = engine.getValue(group, "loop_start_position");
    var end = engine.getValue(group, "loop_end_position");
    if (start !== -1) {
        if (end !== -1) {
            engine.setValue(group, "reloop_exit", 1);
        } else {
            engine.setValue(group, "loop_out", 1);
        }
    }
};

// NOTE: the CF MODE button was left unbound. In the original mapping it only
// ever held test/debug code (never a real crossfader-curve toggle), and this
// hardware's center-cluster buttons (BROWSE/SAMPLE/RECORD/EFX/CF MODE) appear
// to be on a single fixed MIDI channel rather than switching with the active
// deck, so it needs to be re-verified against the device before being wired
// to anything real.

mc3000.moveLoop = function(group,offset) {
    var start = engine.getValue(group, "loop_start_position");
    var end = engine.getValue(group, "loop_end_position");

    if ((start !== -1) && (end !== -1)) {
        engine.setValue(group, "loop_start_position", start+offset);
        engine.setValue(group, "loop_end_position", end+offset);
    }
};

// ---------- 8. FX SECTION (EFX buttons/knobs, SAMP pads) ----------
// NOTE: the SAMP switch is handled entirely in HARDWARE. When SAMP is ON the
// EFX buttons physically send the SAMP notes (0x21-0x24 / 0x31-0x34) and the
// knobs send the SAMP CCs, which are mapped directly to the [Sampler] decks in
// the XML. So there is no software SAMP mode here -- these handlers only ever
// run in FX mode (EFX buttons 0x12-0x15 / 0x52-0x55, knobs 0x55-0x58 / 0x59-0x5C).

mc3000.efxIndex = function(control) {
    // EFX buttons are NOT in note order: EFX1 sends the high nibble (0x15/0x55),
    // EFX2/3/4 send nibbles 2/3/4. Map physical EFX button N -> Effect N.
    var n = control & 0x0F;      // 0x12/0x52 -> 2 ... 0x15/0x55 -> 5
    return n > 4 ? n - 4 : n;    // nibbles 2,3,4 -> 2,3,4 ; nibble 5 -> 1
};

// The FX/SAMP section re-channels with the active deck like everything else:
// left side arrives on ch1 (deck A) or ch2 (deck C); right side on ch3 (deck B)
// or ch4 (deck D). Map the MIDI channel nibble to the FX side (1=left, 2=right).
mc3000.sideFromStatus = function(status) {
    var ch = status & 0x0F;            // 0=A,1=C -> left ; 2=B,3=D -> right
    return (ch === 0 || ch === 1) ? 1 : 2;
};

// "efx"+n+mc3000.sideSuffix(side) -> the leds dict key for one EFX-row LED (e.g. "efx2L")
mc3000.sideSuffix = function(side) {
    return side === 1 ? "L" : "R";
};

// EFX button -> toggle an effect on/off, OR while AUTO LOOP is held on this
// deck, pick the beat length for the hold-AUTO-LOOP+hotcue quick-loop shortcut.
mc3000.efxButton = function(_channel, control, value, status, group) {
    if ((status & 0xF0) !== 0x90 || value === 0) {
        return;   // on real press only
    }
    mc3000.trackDeck(mc3000.groupToDeck(group));
    var side = mc3000.sideFromStatus(status);             // left=Unit1, right=Unit2
    var n = mc3000.efxIndex(control);                     // 1..4
    var g = "[EffectRack1_EffectUnit"+side+"_Effect"+n+"]";
    engine.setValue(g, "enabled", engine.getValue(g, "enabled") ? 0 : 1);
};

// EFX knob -> effect metaknob (left knobs 0x55-0x58, right knobs 0x59-0x5C)
mc3000.efxKnob = function(_channel, control, value, status, _group) {
    var side = mc3000.sideFromStatus(status);
    var n = control - (side === 1 ? 0x54 : 0x58);         // both -> 1..4
    if (n < 1 || n > 4) {
        return;
    }
    // meta is a continuous 0-1 parameter -- setParameter (not setValue) is the
    // documented API for controls like this.
    engine.setParameter("[EffectRack1_EffectUnit"+side+"_Effect"+n+"]", "meta", value/127);
};

// The effect-enabled state lives on one shared CO per side (EffectUnit1/2),
// not per deck, so an LED update for it has no way to know which of the two
// decks on that side is currently active. Broadcast to both of that side's
// deck channels instead of tracking "current deck" as extra state.
mc3000.decksOfSide = function(side) {
    return side === 1 ? [1,3] : [2,4];
};

// Refresh one side's 4 EFX button LEDs from the effect-enabled state
mc3000.refreshEfxLeds = function(side) {
    if (side < 1 || side > 2) {
        return;
    }
    var decks = mc3000.decksOfSide(side);
    for (var n=1; n<=4; n++) {
        var on = engine.getValue("[EffectRack1_EffectUnit"+side+"_Effect"+n+"]", "enabled") ? 1 : 0;
        for (var d=0; d<decks.length; d++) {
            mc3000.setled(decks[d], mc3000.leds["efx"+n+mc3000.sideSuffix(side)], on);
        }
    }
};

// Connection callback: drive one EFX button LED from its effect-enabled state
mc3000.fxEnableLed = function(side, n) {
    var decks = mc3000.decksOfSide(side);
    return function(value) {
        for (var d=0; d<decks.length; d++) {
            mc3000.setled(decks[d], mc3000.leds["efx"+n+mc3000.sideSuffix(side)], value);
        }
    };
};


// Connection callback for the FX-ON (unit assign) button LEDs
// FX-ON LED callback factory: light the FX-ON LED on the deck's own channel.
mc3000.fxOnLed = function(deck, unit) {
    return function(value) {
        mc3000.setled(deck, mc3000.leds["fxon"+unit], value);
    };
};

// Connection callback for the SAMP pad LEDs. Samplers are global (not deck-bound),
// so like the FX-enable LEDs this is keyed by side and must broadcast to both of
// that side's decks so the LED shows correctly regardless of which is active.
mc3000.sampLed = function(side, led) {
    var decks = mc3000.decksOfSide(side);
    return function(value) {
        for (var d=0; d<decks.length; d++) {
            mc3000.setled(decks[d], led, value);
        }
    };
};

// SAMP pad: press = retrigger from cue; SHIFT+press = stop if playing, else eject if loaded.
// Bound directly from XML, so this must be the real handler (channel,control,value,status,group)
// -- NOT a factory -- since Mixxx calls it straight with the raw MIDI bytes.
mc3000.sampButton = function(_channel, control, value, status, _group) {
    if ((status & 0xF0) !== 0x90 || value === 0) {
        return;   // on real press only
    }
    var side = mc3000.sideFromStatus(status);             // 1=left(1-4), 2=right(5-8)
    var n = side === 1 ? control - 0x20 : control - 0x2C; // left 0x21-24->1-4, right 0x31-34->5-8
    if (n < 1 || n > 8) {
        return;
    }
    var g = "[Sampler"+n+"]";
    if (mc3000.state["shift"+side]) {
        if (engine.getValue(g, "play")) {
            engine.setValue(g, "cue_gotoandstop", 1);
        } else if (engine.getValue(g, "track_loaded")) {
            engine.setValue(g, "eject", 1);
        }
    } else {
        engine.setValue(g, "cue_gotoandplay", 1);
    }
};

// ---------- 9. LED CALLBACKS ----------

mc3000.hotcueSetLed = function(value, group, control) {
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["cue"+control[7]],value);
};

mc3000.pflSetLed = function(value,group) {
    mc3000.setled2(mc3000.groupToDeck(group),mc3000.leds["pfl"+mc3000.groupToDeck(group)],value);
};

mc3000.playSetLed = function(value,group) {
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["play"],value);
};

mc3000.keylockSetLed = function(value,group) {
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["keylock"],value);
};

mc3000.loopStartSetLed = function(value, group) {
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["loopin"],value === -1 ? false : true);
};

mc3000.loopEndSetLed = function(value, group) {
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["loopout"],value === -1 ? false : true);
};

mc3000.loopEnableSetLed = function(value, group, _control) {
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["autoloop"],value);
};

mc3000.peakIndicatorSetLed = function(value, group) {
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["sync"],value);
};

mc3000.vuMeterSetLeds = function(value,group) {
    var deck = mc3000.groupToDeck(group);
    var maxled = Math.round((value-0.07)*7);
    var curled = mc3000.vumetermaxled[deck-1];
    if (maxled !== curled) {
        var i=0;
        var offset = mc3000.vumeteroffset[deck-1];
        if (maxled > curled) {
            for (i=curled+1;i<=maxled;i++) {
                midi.sendShortMsg(0xB0+mc3000.deck2ch[deck-1], 80, i+offset);
            }
        } else {
            for (i=maxled+1;i<=curled;i++) {
                midi.sendShortMsg(0xB0+mc3000.deck2ch[deck-1], 81, i+offset);
            }
        }
        mc3000.vumetermaxled[deck-1] = maxled;
    }
};

mc3000.playPositionSetLed = function(playposition, group) {
    var nearEnd = 0;

    if (playposition < 0.80) {
        nearEnd = 0;
    }
    if (playposition > 0.80) {
        nearEnd = 2;  // The song is going to end
    }
    if (playposition === 1) {
        nearEnd = 0;
    }

    if (nearEnd !== mc3000.nearEnd[mc3000.groupToDeck(group)-1]) {
        mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["sync"],nearEnd);
        mc3000.nearEnd[mc3000.groupToDeck(group)-1] = nearEnd;
    }
};
