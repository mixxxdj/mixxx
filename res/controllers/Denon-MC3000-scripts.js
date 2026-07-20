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
 *  - XML <key>-bound functions are called (channel, control, value, status,
 *    group); engine.makeConnection() callbacks are called (value, group,
 *    control) instead -- different shape, easy to mix up.
 *  - Unused parameters are prefixed with _.
 *  - EFX buttons/knobs, SAMP pads, and SYNC are the exception: they're
 *    components.EffectUnit/SamplerButton/SyncButton instances
 *    (midi-components-0.0.js), bound via a dotted key on the instance
 *    (e.g. mc3000.effectUnit1.enableButtons[2].input) instead of a plain
 *    function name.
 *
 **/

var mc3000 = function() {};

// ---------- 1. GLOBAL VARIABLES & CONFIG ----------

mc3000.deck2ch = [0,2,1,3];

// The other deck sharing a side with this one (1<->3 left, 2<->4 right).
mc3000.otherDeckOnSide = {1: 3, 2: 4, 3: 1, 4: 2};

mc3000.hotcues = {
    23: 1, 24: 2, 25: 3, 32: 4, 72: 5, 73: 6, 74: 7, 75: 8,
};

mc3000.leds = {
    vinylmode: 6, keylock: 8, sync: 9, cue: 38, play: 39, fxon1: 90, fxon2: 91,
    cue1: 17, cue2: 19, cue3: 21, cue4: 23, cue5: 48, cue6: 50, cue7: 52, cue8: 54,
    loopin: 36, loopout: 64, autoloop: 43, pfl1: 69, pfl2: 81, pfl3: 75, pfl4: 87,
    efx1L: 92, efx2L: 93, efx3L: 94, efx4L: 95, efx1R: 96, efx2R: 97, efx3R: 98, efx4R: 99,
};

mc3000.state = {
    shift1: false, shift2: false,
};


mc3000.vumetermaxled = [0,0,0,0];
mc3000.vumeteroffset = [9,41,25,57];

// VINYL MODE's startup default is a single global setting (Preferences >
// Controllers > Denon MC3000 > Jog Wheels), applied to all four decks.
mc3000.vinylModeDefault = engine.getSetting("vinylModeDefault") ?? false;
mc3000.scratch = [
    mc3000.vinylModeDefault, mc3000.vinylModeDefault,
    mc3000.vinylModeDefault, mc3000.vinylModeDefault,
];

mc3000.scratchpressed = [false,false,false,false];
mc3000.alpha =  (1.0/8);
mc3000.beta = mc3000.alpha/32;
mc3000.jogSensitivity = engine.getSetting("jogSensitivity") ?? 2.0;
mc3000.scratchSensitivity = engine.getSetting("scratchSensitivity") ?? 1;

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
        engine.makeConnection(`[Channel${i}]`, "sync_enabled", mc3000.syncEnabledSetLed).trigger();
        engine.makeConnection("[Channel"+i+"]", "vu_meter", mc3000.vuMeterSetLeds);
        engine.makeConnection("[Channel"+i+"]", "pfl", mc3000.pflSetLed);
    }

    for (i=1;i<=4;i++) {
        engine.makeConnection("[Channel"+i+"]", "loop_start_position", mc3000.loopStartSetLed);
        engine.makeConnection("[Channel"+i+"]", "loop_end_position", mc3000.loopEndSetLed);
        engine.makeConnection("[Channel"+i+"]", "loop_enabled", mc3000.loopEnableSetLed);
    }

    mc3000.allLedOff();

    // EffectUnit1/2's own init() connects and triggers their enable buttons,
    // focus button, and knobs (LEDs go through the send() overrides set up
    // in mc3000.makeEffectUnit).
    mc3000.effectUnit1.init();
    mc3000.effectUnit2.init();

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

    // Sampler pad LEDs: each SamplerButton wires its own track_loaded/play
    // connections (see mc3000.makeSamplerButton); just connect + trigger them.
    for (i=1; i<=8; i++) {
        mc3000.samplerButtons[i].connect();
        mc3000.samplerButtons[i].trigger();
    }
};

// Called when the MIDI device is closed
mc3000.shutdown = function(id) {
    mc3000.allLedOff();
}

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
            case  0 : ledStatus = 75; break; // OFF
        case false : ledStatus = 75; break; // OFF
        case true  : ledStatus = 74; break; // ON
            case  1 : ledStatus = 74; break; // ON
            case  2 : ledStatus = 76; break; // BLINK
    }
    midi.sendShortMsg(0xB0+mc3000.deck2ch[deck-1], ledStatus, led);
}

mc3000.setled2 = function(deck,led,status) {
    midi.sendShortMsg(0xB0+mc3000.deck2ch[deck-1], status==1 ? 80 : 81, led);
}

// ---------- 4. COMMON HELPERS ----------

mc3000.groupToDeck = function(group) {
    var matches = group.match(/^\[Channel(\d+)\]$/);
    if (matches === null) {
        return -1;
    } else {
        return parseInt(matches[1], 10);
    }
};

// Pitch Rate MSB/LSB
mc3000.rate = function(channel, control, value, status, group) {
    // 0..16383 -> -1..1
    var MSB = value;
    var LSB = control;
    var num14 = (MSB << 7) | LSB;  // Construct the 14-bit number
    var nrate = (num14-8192)/8192;
    engine.setValue(group, "rate", nrate);
}

// NOTE: MIC LEVEL, MONITOR PAN/PHONES, LINE TO MASTER and MASTER LEVEL are
// fully analog on this hardware (confirmed by the MC3000 manual's MIDI
// command list, which does not list them as sending MIDI) -- there is
// nothing for Mixxx to bind them to.

// ---------- 5. PLAYLIST / TRACK SELECT ----------
mc3000.selectKnob = function(_channel, _control, value, _status, group) {
    if (value === 0) {
        engine.setValue(group, "SelectNextTrack", 1);
    } else {
        engine.setValue(group, "SelectPrevTrack", 1);
    }
};

// LOAD A always targets deck A, hold SHIFT for deck C; LOAD B targets deck B,
// SHIFT for deck D. DECK CHG doesn't apply to these two -- their MIDI signal
// never fires on this unit.
mc3000.loadSelected = function(_channel, control, value, status, _group) {
    if ((status & 0xF0) !== 0x90 || value === 0) {
        return;   // on real press only
    }
    var primaryDeck = control === 0x62 ? 1 : 2;
    var secondaryDeck = control === 0x62 ? 3 : 4;
    var shifted = mc3000.state["shift"+primaryDeck] || mc3000.state["shift"+secondaryDeck];
    var deck = shifted ? secondaryDeck : primaryDeck;
    engine.setValue("[Channel"+deck+"]", "LoadSelectedTrack", 1);
};

// ---------- 6. CUE AND HOTCUE ----------
mc3000.cueDefault = function(_channel, _control, _value, status, group) {
    var isPressed = (status & 0xF0) === 0x90 ? 1 : 0;
    engine.setValue(group,"cue_default", isPressed);
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["cue"],isPressed);
};

// components.SyncButton adds sync-lock (hold to enable continuous tempo-follow)
// and SHIFT -> quantize. outConnect stays false since this hardware's LED
// protocol needs mc3000.setled(); the LED is wired separately in init().
mc3000.makeSyncButton = function(deck) {
    return new components.SyncButton({
        group: `[Channel${deck}]`,
        outConnect: false,
    });
};

mc3000.syncButtons = {};
for (let syncDeck=1; syncDeck<=4; syncDeck++) {
    mc3000.syncButtons[syncDeck] = mc3000.makeSyncButton(syncDeck);
}

mc3000.hotcueset = function(_channel, control, _value, status, group) {
    var deck=mc3000.groupToDeck(group);
    var nocue=mc3000.hotcues[control];

    if ((status & 0xF0) !== 0x90) {    // Is Released ?
        if (mc3000.state[`shift${deck}`]) {
            return; // DO NOTHING ON RELEASE IF SHIFT
        }
        engine.setValue(group,"hotcue_"+nocue+"_activate",0);
        return;
    }

    // WITH SHIFT ?
    if (mc3000.state["shift"+deck]) {
        engine.setValue(group,"hotcue_"+nocue+"_clear",1);
        return;
    }

    engine.setValue(group, `hotcue_${nocue}_activate`, 1);
};

mc3000.shift = function(_channel, _control, _value, status, group) {
    var deck = mc3000.groupToDeck(group);
    const isPressed = (status & 0xF0) === 0x90; // 1 if pressed
    mc3000.state[`shift${deck}`] = isPressed;

    // SYNC is genuinely per-deck (one SyncButton per deck), so react directly.
    if (isPressed) {
        mc3000.syncButtons[deck].shift();
    } else {
        mc3000.syncButtons[deck].unshift();
    }

    // Samplers are side-based (shared by both decks on a side, unlike SYNC),
    // so react whenever either deck's SHIFT state on that side changes.
    const sideShifted = isPressed || mc3000.state[`shift${mc3000.otherDeckOnSide[deck]}`];
    const side = (deck === 1 || deck === 3) ? 1 : 2;
    const samplerOffset = side === 1 ? 0 : 4;
    for (let s=1; s<=4; s++) {
        if (sideShifted) {
            mc3000.samplerButtons[samplerOffset+s].shift();
        } else {
            mc3000.samplerButtons[samplerOffset+s].unshift();
        }
    }
};

mc3000.vinylmode = function(_channel, _control, _value, _status, group) {
    var deck = mc3000.groupToDeck(group);
    mc3000.scratch[deck-1] = !mc3000.scratch[deck-1];
};

// JOG with WHEELTOUCH
mc3000.wheelTouch = function (channel, control, value, status, group) {
    var deck = mc3000.groupToDeck(group);
    if (mc3000.scratch[deck-1]) {
        if ((status & 0xF0)==0x90) {    // Is pressed ?
            engine.scratchEnable(deck, 100, 330, mc3000.alpha, mc3000.beta); // 128, 33+1/3 aussi
        } else {
            engine.scratchDisable(deck);
        }
        mc3000.scratchpressed[deck-1] = (status & 0xF0)==0x90 ? true : false;
    }
}

// The wheel that actually controls the scratching
mc3000.jogWheel = function(_channel, _control, value, _status, group) {
    var deck = mc3000.groupToDeck(group);
    var adjustedJog = value - 64; // Control centers on 0x40 (64)

    if (mc3000.scratch[deck-1]) {
        if (mc3000.scratchpressed[deck-1]) {
            engine.scratchTick(deck,adjustedJog/mc3000.scratchSensitivity);
            return;
        }
    }

    var gammaInputRange = 64;    // Max jog speed
    var maxOutFraction = 0.5;    // Where on the curve it should peak; 0.5 is half-way
    var gammaOutputRange = 3;    // Max rate change
    adjustedJog = gammaOutputRange * adjustedJog / (gammaInputRange * maxOutFraction);
    engine.setValue(group, "jog", adjustedJog/mc3000.jogSensitivity);
};

// RATE TEMP UP if PLAY else FWD / END
mc3000.bendUp = function(channel, control, value, status, group) {
    var isPressed = (status & 0xF0)==0x90 ? 1 : 0;
    if (engine.getValue(group,"play") == 1) {
        engine.setValue(group, "rate_temp_up", isPressed);
    }else{
        if ((mc3000.state["shift"+mc3000.groupToDeck(group)]) && isPressed) {
            engine.setValue(group,"end",1);
        } else {
            engine.setValue(group, "fwd",isPressed);
        }
    }
}

// RATE TEMP DOWN if PLAY else BACK / START
mc3000.bendDown = function(channel, control, value, status, group) {
    var isPressed = (status & 0xF0)==0x90 ? 1 : 0;

    if (engine.getValue(group,"play") == 1) {
        engine.setValue(group, "rate_temp_down", isPressed);
    }else{
        if ((mc3000.state["shift"+mc3000.groupToDeck(group)]) && isPressed) {
            engine.setValue(group,"start",1);
        } else {
            engine.setValue(group, "back", isPressed);
        }
    }
}

// ---------- 7. LOOP CONTROL ----------

// Shift and autoloop : delete loop point ELSE toggle a beat-synced auto loop
mc3000.autoLoop = function(_channel, _control, _value, status, group) {
    if ((status & 0xF0) !== 0x90) {
        return;   // on real press only
    }
    // DELETE LOOP POINTS IF SHIFT, ELSE TOGGLE A BEAT-SYNCED AUTO LOOP.
    // beatloop_activate is NOT a toggle by itself -- per the Mixxx docs it
    // only ever "sets a loop ... and enables it". To get real toggle
    // behavior (press to engage and keep looping after release, press
    // again to release) we have to check loop_enabled ourselves, the same
    // way loopIn/loopOut already do below.
    const deck = mc3000.groupToDeck(group);
    if (mc3000.state[`shift${deck}`]) {
        engine.setValue(group, "reloop_exit", 1);
        engine.setValue(group, "loop_end_position", -1);
        engine.setValue(group, "loop_start_position", -1);
    } else if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "reloop_exit", 1);
    } else {
        engine.setValue(group, "beatloop_activate", 1);
    }
};

mc3000.loopCutM = function(_channel, _control, _value, _status, group) {
    // loop_halve halves beatloop_size, and also resizes the active loop if its
    // length currently equals beatloop_size. Safe to call with no loop active.
    engine.setValue(group, "loop_halve", 1);
};

mc3000.loopCutP = function(_channel, _control, _value, _status, group) {
    // loop_double doubles beatloop_size, and also resizes the active loop if its
    // length currently equals beatloop_size. Safe to call with no loop active.
    engine.setValue(group, "loop_double", 1);
};

mc3000.loopIn = function(_channel, _control, _value, _status, group) {
    if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "reloop_exit", 1);
    }
    engine.setValue(group, "loop_in", 1);
    engine.setValue(group, "loop_end_position", -1);
};

mc3000.loopOut = function(_channel, _control, _value, _status, group) {
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

// CF MODE is left unbound: its center-cluster channel doesn't seem to switch
// with the active deck, so it needs re-verification against the device.

// ---------- 8. FX SECTION (EFX buttons/knobs, SAMP pads) ----------
// SAMP is a hardware switch: with SAMP on, the EFX buttons/knobs physically
// send different notes/CCs, mapped straight to [Sampler] decks in the XML.
// These handlers only ever run in FX mode.

mc3000.sideSuffix = function(side) {
    return side === 1 ? "L" : "R";
};

// Effect-enabled state is one shared CO per side, not per deck, so LEDs
// broadcast to both of that side's decks instead of tracking "current deck".
mc3000.decksOfSide = function(side) {
    return side === 1 ? [1,3] : [2,4];
};

// components.EffectUnit: EFX.1-3 buttons/knobs are the enable toggles and meta
// knobs for that side's three effect slots (Mixxx's standard model), EFX.4 is
// the focus button (per-parameter control), and its knob is the dry/wet mix.
// unitNumbers is a single-element array since FX ON 1/2 below already handles
// deck assignment.
//
// EffectUnit wires its own LED output unconditionally, ignoring outConnect, so
// send() is overridden per-button instead to broadcast through mc3000.setled()
// -- send() is the one thing untouched across all of EffectUnit's modes.
mc3000.makeEffectUnit = function(side) {
    const eu = new components.EffectUnit([side], false);
    const decks = mc3000.decksOfSide(side);
    const suffix = mc3000.sideSuffix(side);

    // output() scales booleans to 127/0 (this.on/this.off) before calling
    // send(), so that has to be normalized back to a plain 1/0 here.
    const broadcastSend = function(led) {
        return function(value) {
            for (let d=0; d<decks.length; d++) {
                mc3000.setled(decks[d], led, value ? 1 : 0);
            }
        };
    };

    for (let n=1; n<=3; n++) {
        eu.enableButtons[n].send = broadcastSend(mc3000.leds[`efx${n}${suffix}`]);
    }
    eu.effectFocusButton.send = broadcastSend(mc3000.leds[`efx4${suffix}`]);

    return eu;
};

mc3000.effectUnit1 = mc3000.makeEffectUnit(1);
mc3000.effectUnit2 = mc3000.makeEffectUnit(2);

// FX-ON LED callback factory: light the FX-ON LED on the deck's own channel.
mc3000.fxOnLed = function(deck, unit) {
    return function(value) {
        mc3000.setled(deck, mc3000.leds["fxon"+unit], value);
    };
};

// components.SamplerButton: an empty pad loads the selected library track
// (default unshifted behavior). LEDs use the send() override again, with the
// 3-state LED distinguishing loaded-but-stopped (solid) from playing (blink).
// SHIFT+press: cue_gotoandstop if playing, eject if stopped.
mc3000.makeSamplerButton = function(n, led, decks) {
    const button = new components.SamplerButton({
        number: n,
        off: 0,
        loaded: 1,
        playing: 2,
        outConnect: false,
    });
    button.shift = function() {
        this.input = function(channel, control, value, status, _group) {
            if (this.isPress(channel, control, value, status)) {
                if (engine.getValue(this.group, "play") === 1) {
                    engine.setValue(this.group, "cue_gotoandstop", 1);
                } else {
                    engine.setValue(this.group, "eject", 1);
                }
            } else if (engine.getValue(this.group, "play") === 0) {
                engine.setValue(this.group, "eject", 0);
            }
        };
    };
    button.send = function(value) {
        for (var d=0; d<decks.length; d++) {
            mc3000.setled(decks[d], led, value);
        }
    };
    return button;
};

// LED ids from the MC3000 reception table (left pads on ch1, right on ch3).
mc3000.samplerButtons = {};
const sledL = [25, 27, 29, 32], sledR = [65, 67, 69, 71];
for (let sN=1; sN<=4; sN++) {
    mc3000.samplerButtons[sN] = mc3000.makeSamplerButton(sN, sledL[sN-1], mc3000.decksOfSide(1));
    mc3000.samplerButtons[sN+4] = mc3000.makeSamplerButton(sN+4, sledR[sN-1], mc3000.decksOfSide(2));
}

// ---------- 9. LED CALLBACKS ----------

mc3000.hotcueSetLed = function(value, group, control) {
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["cue"+control[7]],value);
}

mc3000.pflSetLed = function(value,group) {
    mc3000.setled2(mc3000.groupToDeck(group),mc3000.leds["pfl"+mc3000.groupToDeck(group)],value);
}

mc3000.playSetLed = function(value,group) {
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["play"],value);
}

mc3000.keylockSetLed = function(value,group) {
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["keylock"],value);
}

mc3000.loopStartSetLed = function (value, group) {
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["loopin"],value == -1 ? false: true);
}

mc3000.loopEndSetLed = function (value, group) {
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["loopout"],value == -1 ? false: true);
}

mc3000.loopEnableSetLed = function(value, group, control) {
    mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["autoloop"],value);
}

mc3000.syncEnabledSetLed = function(value, group) {
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

