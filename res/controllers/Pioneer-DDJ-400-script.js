// Pioneer-DDJ-400-script.js
// ****************************************************************************
// * Mixxx mapping script file for the Pioneer DDJ-400.
// * Authors: Warker, nschloe, dj3730, jusko
// * Reviewers: Be-ing, Holzhaus
// * Manual: https://manual.mixxx.org/2.3/en/hardware/controllers/pioneer_ddj_400.html
// ****************************************************************************
//
//  Implemented (as per manufacturer's manual):
//      * Mixer Section (Faders, EQ, Filter, Gain, Cue)
//      * Browsing and loading + Waveform zoom (shift)
//      * Jogwheels, Scratching, Bending, Loop adjust
//      * Cycle Temporange
//      * Beat Sync
//      * Hot Cue Mode
//      * Beat Loop Mode
//      * Beat Jump Mode
//      * Sampler Mode
//
//  Custom (Mixxx specific mappings):
//      * BeatFX: Assigned Effect Unit 1
//                < LEFT focus EFFECT1
//                > RIGHT focus EFFECT2
//                v FX_SELECT focus EFFECT3.
//                ON/OFF toggles focused effect slot
//                SHIFT + ON/OFF disables all three effect slots.
//                SHIFT + < loads previous effect
//                SHIFT + > loads next effect
//
//      * 32 beat jump forward & back (Shift + </> CUE/LOOP CALL arrows)
//      * Toggle quantize (Shift + channel cue)
//
//  Not implemented (after discussion and trial attempts):
//      * Loop Section:
//        * -4BEAT auto loop (hacky---prefer a clean way to set a 4 beat loop
//                            from a previous position on long press)
//
//        * CUE/LOOP CALL - memory & delete (complex and not useful. Hot cues are sufficient)
//
//      * Secondary pad modes (trial attempts complex and too experimental)
//        * Keyboard mode
//        * Pad FX1
//        * Pad FX2
//        * Keyshift mode

var PioneerDDJ400 = {};

PioneerDDJ400.lights = {
    beatFx: {
        status: 0x94,
        data1: 0x47,
    },
    shiftBeatFx: {
        status: 0x94,
        data1: 0x43,
    },
    deck1: {
        vuMeter: {
            status: 0xB0,
            data1: 0x02,
        },
        playPause: {
            status: 0x90,
            data1: 0x0B,
        },
        shiftPlayPause: {
            status: 0x90,
            data1: 0x47,
        },
        cue: {
            status: 0x90,
            data1: 0x0C,
        },
        shiftCue: {
            status: 0x90,
            data1: 0x48,
        },
    },
    deck2: {
        vuMeter: {
            status: 0xB0,
            data1: 0x02,
        },
        playPause: {
            status: 0x91,
            data1: 0x0B,
        },
        shiftPlayPause: {
            status: 0x91,
            data1: 0x47,
        },
        cue: {
            status: 0x91,
            data1: 0x0C,
        },
        shiftCue: {
            status: 0x91,
            data1: 0x48,
        },
    },
};

// Store timer IDs
PioneerDDJ400.timers = {};

// Jog wheel constants
PioneerDDJ400.vinylMode = true;
PioneerDDJ400.alpha = 1.0/8;
PioneerDDJ400.beta = PioneerDDJ400.alpha/32;

// Multiplier for fast seek through track using SHIFT+JOGWHEEL
PioneerDDJ400.fastSeekScale = 150;
PioneerDDJ400.bendScale = 0.8;

PioneerDDJ400.tempoRanges = [0.06, 0.10, 0.16, 0.25];

PioneerDDJ400.shiftButtonDown = [false, false];

// Jog wheel loop adjust
PioneerDDJ400.loopAdjustIn = [false, false];
PioneerDDJ400.loopAdjustOut = [false, false];
PioneerDDJ400.loopAdjustMultiply = 50;

// Beatjump pad (beatjump_size values)
PioneerDDJ400.beatjumpSizeForPad = {
    0x20: -1, // PAD 1
    0x21: 1,  // PAD 2
    0x22: -2, // PAD 3
    0x23: 2,  // PAD 4
    0x24: -4, // PAD 5
    0x25: 4,  // PAD 6
    0x26: -8, // PAD 7
    0x27: 8   // PAD 8
};

PioneerDDJ400.quickJumpSize = 32;

// Used for tempo slider
PioneerDDJ400.highResMSB = {
    "[Channel1]": {},
    "[Channel2]": {}
};

PioneerDDJ400.trackLoadedLED = function(value, group, _control) {
    midi.sendShortMsg(
        0x9F,
        group.match(script.channelRegEx)[1] - 1,
        value > 0 ? 0x7F : 0x00
    );
};

PioneerDDJ400.toggleLight = function(midiIn, active) {
    midi.sendShortMsg(midiIn.status, midiIn.data1, active ? 0x7F : 0);
};

//
// Init
//

PioneerDDJ400.init = function() {
    engine.setValue("[EffectRack1_EffectUnit1]", "show_focus", 1);

    engine.makeConnection("[Channel1]", "VuMeter", PioneerDDJ400.vuMeterUpdate);
    engine.makeConnection("[Channel2]", "VuMeter", PioneerDDJ400.vuMeterUpdate);

    PioneerDDJ400.toggleLight(PioneerDDJ400.lights.deck1.vuMeter, false);
    PioneerDDJ400.toggleLight(PioneerDDJ400.lights.deck2.vuMeter, false);

    engine.softTakeover("[Channel1]", "rate", true);
    engine.softTakeover("[Channel2]", "rate", true);
    engine.softTakeover("[EffectRack1_EffectUnit1_Effect1]", "meta", true);
    engine.softTakeover("[EffectRack1_EffectUnit1_Effect2]", "meta", true);
    engine.softTakeover("[EffectRack1_EffectUnit1_Effect3]", "meta", true);
    engine.softTakeover("[EffectRack1_EffectUnit1]", "mix", true);

    for (var i = 1; i <= 16; ++i) {
        engine.makeConnection("[Sampler" + i + "]", "play", PioneerDDJ400.samplerPlayOutputCallbackFunction);
    }

    engine.makeConnection("[Channel1]", "track_loaded", PioneerDDJ400.trackLoadedLED);
    engine.makeConnection("[Channel2]", "track_loaded", PioneerDDJ400.trackLoadedLED);

    // play the "track loaded" animation on both decks at startup
    midi.sendShortMsg(0x9F, 0x00, 0x7F);
    midi.sendShortMsg(0x9F, 0x01, 0x7F);

    PioneerDDJ400.setLoopButtonLights(0x90, 0x7F);
    PioneerDDJ400.setLoopButtonLights(0x91, 0x7F);

    engine.makeConnection("[Channel1]", "loop_enabled", PioneerDDJ400.loopToggle);
    engine.makeConnection("[Channel2]", "loop_enabled", PioneerDDJ400.loopToggle);

    for (i = 1; i <= 3; i++) {
        engine.makeConnection("[EffectRack1_EffectUnit1_Effect" + i +"]", "enabled", PioneerDDJ400.toggleFxLight);
    }
    engine.makeConnection("[EffectRack1_EffectUnit1]", "focused_effect", PioneerDDJ400.toggleFxLight);

    // query the controller for current control positions on startup
    midi.sendSysexMsg([0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x02, 0x06, 0x00, 0x03, 0x01, 0xf7], 12);
};

//
// Channel level lights
//

PioneerDDJ400.vuMeterUpdate = function(value, group) {
    var newVal = value * 150;

    switch (group) {
    case "[Channel1]":
        midi.sendShortMsg(0xB0, 0x02, newVal);
        break;

    case "[Channel2]":
        midi.sendShortMsg(0xB1, 0x02, newVal);
        break;
    }
};

//
// Effects
//

PioneerDDJ400.toggleFxLight = function(_value, _group, _control) {
    var enabled = engine.getValue(PioneerDDJ400.focusedFxGroup(), "enabled");

    PioneerDDJ400.toggleLight(PioneerDDJ400.lights.beatFx, enabled);
    PioneerDDJ400.toggleLight(PioneerDDJ400.lights.shiftBeatFx, enabled);
};

PioneerDDJ400.focusedFxGroup = function() {
    var focusedFx = engine.getValue("[EffectRack1_EffectUnit1]", "focused_effect");
    return "[EffectRack1_EffectUnit1_Effect" + focusedFx + "]";
};

PioneerDDJ400.beatFxLevelDepthRotate = function(_channel, _control, value) {
    if (PioneerDDJ400.shiftButtonDown[0] || PioneerDDJ400.shiftButtonDown[1]) {
        engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit1]", "mix");
        engine.setParameter(PioneerDDJ400.focusedFxGroup(), "meta", value / 0x7F);
    } else {
        engine.softTakeoverIgnoreNextValue(PioneerDDJ400.focusedFxGroup(), "meta");
        engine.setParameter("[EffectRack1_EffectUnit1]", "mix", value / 0x7F);
    }
};

PioneerDDJ400.beatFxSelectPreviousEffect = function(_channel, _control, value) {
    engine.setValue(PioneerDDJ400.focusedFxGroup(), "prev_effect", value);
};

PioneerDDJ400.beatFxSelectNextEffect = function(_channel, _control, value) {
    engine.setValue(PioneerDDJ400.focusedFxGroup(), "next_effect", value);
};

PioneerDDJ400.beatFxLeftPressed = function(_channel, _control, value) {
    if (value === 0) { return; }

    engine.setValue("[EffectRack1_EffectUnit1]", "focused_effect", 1);
};

PioneerDDJ400.beatFxRightPressed = function(_channel, _control, value) {
    if (value === 0) { return; }

    engine.setValue("[EffectRack1_EffectUnit1]", "focused_effect", 2);
};

PioneerDDJ400.beatFxSelectPressed = function(_channel, _control, value) {
    if (value === 0) { return; }

    engine.setValue("[EffectRack1_EffectUnit1]", "focused_effect", 3);
};

PioneerDDJ400.beatFxOnOffPressed = function(_channel, _control, value) {
    if (value === 0) { return; }

    var toggleEnabled = !engine.getValue(PioneerDDJ400.focusedFxGroup(), "enabled");
    engine.setValue(PioneerDDJ400.focusedFxGroup(), "enabled", toggleEnabled);
};

PioneerDDJ400.beatFxOnOffShiftPressed = function(_channel, _control, value) {
    if (value === 0) { return; }

    engine.setParameter("[EffectRack1_EffectUnit1]", "mix", 0);
    engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit1]", "mix");

    for (var i = 1; i <= 3; i++) {
        engine.setValue("[EffectRack1_EffectUnit1_Effect" + i + "]", "enabled", 0);
    }
    PioneerDDJ400.toggleLight(PioneerDDJ400.lights.beatFx, false);
    PioneerDDJ400.toggleLight(PioneerDDJ400.lights.shiftBeatFx, false);
};

PioneerDDJ400.beatFxChannel = function(_channel, control, value, _status, group) {
    if (value === 0x00) { return; }

    var enableChannel1 = control === 0x10 ? 1 : 0,
        enableChannel2 = control === 0x11 ? 1 : 0,
        enableMaster = control === 0x14 ? 1 : 0;

    engine.setValue(group, "group_[Channel1]_enable", enableChannel1);
    engine.setValue(group, "group_[Channel2]_enable", enableChannel2);
    engine.setValue(group, "group_[Master]_enable", enableMaster);
};

//
// Loop IN/OUT ADJUST
//

PioneerDDJ400.toggleLoopAdjustIn = function(channel, _control, value, _status, group) {
    if (value === 0 || engine.getValue(group, "loop_enabled" === 0)) {
        return;
    }
    PioneerDDJ400.loopAdjustIn[channel] = !PioneerDDJ400.loopAdjustIn[channel];
    PioneerDDJ400.loopAdjustOut[channel] = false;
};

PioneerDDJ400.toggleLoopAdjustOut = function(channel, _control, value, _status, group) {
    if (value === 0 || engine.getValue(group, "loop_enabled" === 0)) {
        return;
    }
    PioneerDDJ400.loopAdjustOut[channel] = !PioneerDDJ400.loopAdjustOut[channel];
    PioneerDDJ400.loopAdjustIn[channel] = false;
};

// Two signals are sent here so that the light stays lit/unlit in its shift state too
PioneerDDJ400.setReloopLight = function(status, value) {
    midi.sendShortMsg(status, 0x4D, value);
    midi.sendShortMsg(status, 0x50, value);
};


PioneerDDJ400.setLoopButtonLights = function(status, value) {
    [0x10, 0x11, 0x4E, 0x4C].forEach(function(control) {
        midi.sendShortMsg(status, control, value);
    });
};

PioneerDDJ400.startLoopLightsBlink = function(channel, control, status, group) {
    var blink = 0x7F;

    PioneerDDJ400.stopLoopLightsBlink(group, control, status);

    PioneerDDJ400.timers[group][control] = engine.beginTimer(500, function() {
        blink = 0x7F - blink;

        // When adjusting the loop out position, turn the loop in light off
        if (PioneerDDJ400.loopAdjustOut[channel]) {
            midi.sendShortMsg(status, 0x10, 0x00);
            midi.sendShortMsg(status, 0x4C, 0x00);
        } else {
            midi.sendShortMsg(status, 0x10, blink);
            midi.sendShortMsg(status, 0x4C, blink);
        }

        // When adjusting the loop in position, turn the loop out light off
        if (PioneerDDJ400.loopAdjustIn[channel]) {
            midi.sendShortMsg(status, 0x11, 0x00);
            midi.sendShortMsg(status, 0x4E, 0x00);
        } else {
            midi.sendShortMsg(status, 0x11, blink);
            midi.sendShortMsg(status, 0x4E, blink);
        }
    });

};

PioneerDDJ400.stopLoopLightsBlink = function(group, control, status) {
    PioneerDDJ400.timers[group] = PioneerDDJ400.timers[group] || {};

    if (PioneerDDJ400.timers[group][control] !== undefined) {
        engine.stopTimer(PioneerDDJ400.timers[group][control]);
    }
    PioneerDDJ400.timers[group][control] = undefined;
    PioneerDDJ400.setLoopButtonLights(status, 0x7F);
};

PioneerDDJ400.loopToggle = function(value, group, control) {
    var status = group === "[Channel1]" ? 0x90 : 0x91,
        channel = group === "[Channel1]" ? 0 : 1;

    PioneerDDJ400.setReloopLight(status, value ? 0x7F : 0x00);

    if (value) {
        PioneerDDJ400.startLoopLightsBlink(channel, control, status, group);
    } else {
        PioneerDDJ400.stopLoopLightsBlink(group, control, status);
        PioneerDDJ400.loopAdjustIn[channel] = false;
        PioneerDDJ400.loopAdjustOut[channel] = false;
    }
};

//
// CUE/LOOP CALL
//

PioneerDDJ400.cueLoopCallLeft = function(_channel, _control, value, _status, group) {
    if (value) {
        engine.setValue(group, "loop_scale", 0.5);
    }
};

PioneerDDJ400.cueLoopCallRight = function(_channel, _control, value, _status, group) {
    if (value) {
        engine.setValue(group, "loop_scale", 2.0);
    }
};

//
// BEAT SYNC
//
// Note that the controller sends different signals for a short press and a long
// press of the same button.
//

PioneerDDJ400.syncPressed = function(channel, control, value, status, group) {
    if (engine.getValue(group, "sync_enabled") && value > 0) {
        engine.setValue(group, "sync_enabled", 0);
    } else {
        engine.setValue(group, "beatsync", value);
    }
};

PioneerDDJ400.syncLongPressed = function(channel, control, value, status, group) {
    if (value) {
        engine.setValue(group, "sync_enabled", 1);
    }
};

PioneerDDJ400.cycleTempoRange = function(_channel, _control, value, _status, group) {
    if (value === 0) return; // ignore release

    var currRange = engine.getValue(group, "rateRange");
    var idx = 0;

    for (var i = 0; i < this.tempoRanges.length; i++) {
        if (currRange === this.tempoRanges[i]) {
            idx = (i + 1) % this.tempoRanges.length;
            break;
        }
    }
    engine.setValue(group, "rateRange", this.tempoRanges[idx]);
};

//
// Jog wheels
//

PioneerDDJ400.jogTurn = function(channel, _control, value, _status, group) {
    var deckNum = channel + 1;
    // wheel center at 64; <64 rew >64 fwd
    var newVal = value - 64;

    // loop_in / out adjust
    var loopEnabled = engine.getValue(group, "loop_enabled");
    if (loopEnabled > 0) {
        if (PioneerDDJ400.loopAdjustIn[channel]) {
            newVal = newVal * PioneerDDJ400.loopAdjustMultiply + engine.getValue(group, "loop_start_position");
            engine.setValue(group, "loop_start_position", newVal);
            return;
        }
        if (PioneerDDJ400.loopAdjustOut[channel]) {
            newVal = newVal * PioneerDDJ400.loopAdjustMultiply + engine.getValue(group, "loop_end_position");
            engine.setValue(group, "loop_end_position", newVal);
            return;
        }
    }

    if (engine.isScratching(deckNum)) {
        engine.scratchTick(deckNum, newVal);
    } else { // fallback
        engine.setValue(group, "jog", newVal * this.bendScale);
    }
};


PioneerDDJ400.jogSearch = function(_channel, _control, value, _status, group) {
    var newVal = (value - 64) * PioneerDDJ400.fastSeekScale;
    engine.setValue(group, "jog", newVal);
};

PioneerDDJ400.jogTouch = function(channel, _control, value) {
    var deckNum = channel + 1;

    // skip while adjusting the loop points
    if (PioneerDDJ400.loopAdjustIn[channel] || PioneerDDJ400.loopAdjustOut[channel]) {
        return;
    }

    if (value !== 0 && this.vinylMode) {
        engine.scratchEnable(deckNum, 720, 33+1/3, this.alpha, this.beta);
    } else {
        engine.scratchDisable(deckNum);
    }
};

//
// Shift button
//

PioneerDDJ400.shiftPressed = function(channel, _control, value, _status, _group) {
    PioneerDDJ400.shiftButtonDown[channel] = value === 0x7F;
};


//
// Tempo sliders
//
// The tempo option in Mixxx's deck preferences determine whether down/up
// increases/decreases the rate. Therefore it must be inverted here so that the
// UI and the control sliders always move in the same direction.
//

PioneerDDJ400.tempoSliderMSB = function(channel, control, value, status, group) {
    PioneerDDJ400.highResMSB[group].tempoSlider = value;
};

PioneerDDJ400.tempoSliderLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJ400.highResMSB[group].tempoSlider << 7) + value;

    engine.setValue(
        group,
        "rate",
        1 - (fullValue / 0x2000)
    );
};

//
// Beat Jump mode
//
// Note that when we increase/decrease the sizes on the pad buttons, we use the
// value of the first pad (0x21) as an upper/lower limit beyond which we don't
// allow further increasing/decreasing of all the values.
//

PioneerDDJ400.beatjumpPadPressed = function(_channel, control, value, _status, group) {
    if (value === 0) {
        return;
    }
    engine.setValue(group, "beatjump_size", Math.abs(PioneerDDJ400.beatjumpSizeForPad[control]));
    engine.setValue(group, "beatjump", PioneerDDJ400.beatjumpSizeForPad[control]);
};

PioneerDDJ400.increaseBeatjumpSizes = function(_channel, control, value, _status, group) {
    if (value === 0 || PioneerDDJ400.beatjumpSizeForPad[0x21] * 16 > 16) {
        return;
    }
    Object.keys(PioneerDDJ400.beatjumpSizeForPad).forEach(function(pad) {
        PioneerDDJ400.beatjumpSizeForPad[pad] = PioneerDDJ400.beatjumpSizeForPad[pad] * 16;
    });
    engine.setValue(group, "beatjump_size", PioneerDDJ400.beatjumpSizeForPad[0x21]);
};

PioneerDDJ400.decreaseBeatjumpSizes = function(_channel, control, value, _status, group) {
    if (value === 0 || PioneerDDJ400.beatjumpSizeForPad[0x21] / 16 < 1/16) {
        return;
    }
    Object.keys(PioneerDDJ400.beatjumpSizeForPad).forEach(function(pad) {
        PioneerDDJ400.beatjumpSizeForPad[pad] = PioneerDDJ400.beatjumpSizeForPad[pad] / 16;
    });
    engine.setValue(group, "beatjump_size", PioneerDDJ400.beatjumpSizeForPad[0x21]);
};

//
// Sampler mode
//

PioneerDDJ400.samplerPlayOutputCallbackFunction = function(value, group, _control) {
    if (value === 1) {
        var curPad = group.match(script.samplerRegEx)[1];
        PioneerDDJ400.startSamplerBlink(
            0x97 + (curPad > 8 ? 2 : 0),
            0x30 + ((curPad > 8 ? curPad - 8 : curPad) - 1),
            group);
    }
};

PioneerDDJ400.samplerPadPressed = function(_channel, _control, value, _status, group) {
    if (engine.getValue(group, "track_loaded")) {
        engine.setValue(group, "cue_gotoandplay", value);
    } else {
        engine.setValue(group, "LoadSelectedTrack", value);
    }
};

PioneerDDJ400.samplerPadShiftPressed = function(_channel, _control, value, _status, group) {
    if (engine.getValue(group, "play")) {
        engine.setValue(group, "cue_gotoandstop", value);
    } else if (engine.getValue(group, "track_loaded")) {
        engine.setValue(group, "eject", value);
    }
};

PioneerDDJ400.startSamplerBlink = function(channel, control, group) {
    var val = 0x7f;

    PioneerDDJ400.stopSamplerBlink(channel, control);
    PioneerDDJ400.timers[channel][control] = engine.beginTimer(250, function() {
        val = 0x7f - val;

        // blink the appropriate pad
        midi.sendShortMsg(channel, control, val);
        // also blink the pad while SHIFT is pressed
        midi.sendShortMsg((channel+1), control, val);

        var isPlaying = engine.getValue(group, "play") === 1;

        if (!isPlaying) {
            // kill timer
            PioneerDDJ400.stopSamplerBlink(channel, control);
            // set the pad LED to ON
            midi.sendShortMsg(channel, control, 0x7f);
            // set the pad LED to ON while SHIFT is pressed
            midi.sendShortMsg((channel+1), control, 0x7f);
        }
    });
};

PioneerDDJ400.stopSamplerBlink = function(channel, control) {
    PioneerDDJ400.timers[channel] = PioneerDDJ400.timers[channel] || {};

    if (PioneerDDJ400.timers[channel][control] !== undefined) {
        engine.stopTimer(PioneerDDJ400.timers[channel][control]);
        PioneerDDJ400.timers[channel][control] = undefined;
    }
};

//
// Additional features
//

PioneerDDJ400.toggleQuantize = function(_channel, _control, value, _status, group) {
    if (value) {
        script.toggleControl(group, "quantize");
    }
};

PioneerDDJ400.quickJumpForward = function(_channel, _control, value, _status, group) {
    if (value) {
        engine.setValue(group, "beatjump", PioneerDDJ400.quickJumpSize);
    }
};

PioneerDDJ400.quickJumpBack = function(_channel, _control, value, _status, group) {
    if (value) {
        engine.setValue(group, "beatjump", -PioneerDDJ400.quickJumpSize);
    }
};

//
// Shutdown
//

PioneerDDJ400.shutdown = function() {
    // reset vumeter
    PioneerDDJ400.toggleLight(PioneerDDJ400.lights.deck1.vuMeter, false);
    PioneerDDJ400.toggleLight(PioneerDDJ400.lights.deck2.vuMeter, false);

    // housekeeping
    // turn off all Sampler LEDs
    for (var i = 0; i <= 7; ++i) {
        midi.sendShortMsg(0x97, 0x30 + i, 0x00);    // Deck 1 pads
        midi.sendShortMsg(0x98, 0x30 + i, 0x00);    // Deck 1 pads with SHIFT
        midi.sendShortMsg(0x99, 0x30 + i, 0x00);    // Deck 2 pads
        midi.sendShortMsg(0x9A, 0x30 + i, 0x00);    // Deck 2 pads with SHIFT
    }
    // turn off all Hotcue LEDs
    for (i = 0; i <= 7; ++i) {
        midi.sendShortMsg(0x97, 0x00 + i, 0x00);    // Deck 1 pads
        midi.sendShortMsg(0x98, 0x00 + i, 0x00);    // Deck 1 pads with SHIFT
        midi.sendShortMsg(0x99, 0x00 + i, 0x00);    // Deck 2 pads
        midi.sendShortMsg(0x9A, 0x00 + i, 0x00);    // Deck 2 pads with SHIFT
    }

    // turn off loop in and out lights
    PioneerDDJ400.setLoopButtonLights(0x90, 0x00);
    PioneerDDJ400.setLoopButtonLights(0x91, 0x00);

    // turn off reloop lights
    PioneerDDJ400.setReloopLight(0x90, 0x00);
    PioneerDDJ400.setReloopLight(0x91, 0x00);

    // stop any flashing lights
    PioneerDDJ400.toggleLight(PioneerDDJ400.lights.beatFx, false);
    PioneerDDJ400.toggleLight(PioneerDDJ400.lights.shiftBeatFx, false);
};
