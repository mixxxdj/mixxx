// Pioneer-DDJ-400-script.js
// ****************************************************************************
// * Mixxx mapping script file for the Pioneer DDJ-400.
// * Authors: Warker, nschloe, dj3730, jusko
// * Forum: https://mixxx.org/forums/viewtopic.php?f=7&t=12113
// * Wiki: https://www.mixxx.org/wiki/doku.php/pioneer_ddj-400
//
// Upstream MIDI spec:
// * https://www.pioneerdj.com/-/media/pioneerdj/software-info/controller/ddj-400/ddj-400_midi_message_list_e1.pdf
//
//             Working (as per controller spec):
//                 * Mixer Section (Faders, EQ, Filter, Gain, Cue)
//                 * Browsing and loading + Waveform zoom (shift)
//                 * Jogwheels, Scratching, Bending, Loop adjust
//                 * Cycle Temporange
//                 * Beat Sync
//                 * Hot Cue Mode
//                 * Beat Loop Mode
//                 * Beat Jump Mode
//                 * Sampler Mode
//
//             Custom (Mixxx specific mappings):
//                 * BeatFX: Assigned Effect Unit 1
//                           < LEFT selects EFFECT1
//                           > RIGHT selects EFFECT2
//                           v FX_SELECT selects EFFECT3.
//                           v again to get back to wet/dry mix
//                           ON/OFF toggles selected effect slot
//                           SHIFT + ON/OFF disables all three effect slots.
//                           SHIFT + < selects previous effect
//                           SHIFT + > selects next effect
//
//                 * Phrase jump forward & back (Shift + </> CUE/LOOP CALL arrows)
//
//             Experimental (not performance ready):
//                 * PAD FX: Assigned Effect unit 1 & 2
//                           Pad 1, 2 & 3 toggle effect slot
//                           Shift + pads 5,6 & 7 select previous effect
//                           Pads 4 & 8 do nothing.
//                           - Deck 1, currently clobbers the BeatFX rack
//                           - PAD FX2, mappings are invalid
//
//                 * Keyboard Mode (check pitch value)
//                   - Major/minor/chromatic scales?
//                   - Lights
//                   - Selecting cues
//                   - Allow playback takeover
//
//                 * Keyshift Mode
//                   - Major/minor/chromatic scales?
//                   - Limit range shifting up or down
//
//             Not implemented (after discussion and trial attempts):
//                 * Loop Section:
//                   * -4BEAT auto loop
//                   * CUE/LOOP CALL - memory & delete (complex and not useful. Hot cues are sufficient)
//
var PioneerDDJ400 = {};

var LightsPioneerDDJ400 = {
    beatFx: {
        status: 0x94,
        data1: 0x47,
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

// Stores padmode state and encapsulates LED logic
PioneerDDJ400.performancePads = {
    modes: {
        HOTCUE:   0x1B,
        BEATLOOP: 0x6D,
        BEATJUMP: 0x20,
        SAMPLER:  0x22,
        KEYBOARD: 0x69,
        PADFX1:   0x1E,
        PADFX2:   0x6B,
        KEYSHIFT: 0x6F
    },

    // Start in hotcue mode
    state: [
        0x1B, // Channel 1
        0x1B  // Channel 2
    ],

    // Toggles the shift specific lights for the current mode
    toggleShiftLights: function(channel, value) {
        if (this.state[channel] === this.modes.BEATJUMP) {
            midi.sendShortMsg(channel === 0 ? 0x98 : 0x9A,
                              0x26,
                              value);

            midi.sendShortMsg(channel === 0 ? 0x98 : 0x9A,
                              0x27,
                              value);
        }
    },

    // Clear all pad mode lights and reset back to hotcue state
    reset: function() {
        for (m in this.modes) {
            if (this.modes[m] !== this.modes.HOTCUE) {
                midi.sendShortMsg(0x90, this.modes[m], 0x00);
                midi.sendShortMsg(0x91, this.modes[m], 0x00);
            }
        };
        this.state[0] = this.state[1] = this.modes.HOTCUE;

        midi.sendShortMsg(0x90, this.modes.HOTCUE, 0x7f);
        midi.sendShortMsg(0x91, this.modes.HOTCUE, 0x7f);
    }
};

// Save the Shift State
PioneerDDJ400.shiftState = [0, 0];

// JogWheel
PioneerDDJ400.vinylMode = true;
PioneerDDJ400.alpha = 1.0/8;
PioneerDDJ400.beta = PioneerDDJ400.alpha/32;
PioneerDDJ400.highspeedScale = 150; // multiplier for fast seek through track using SHIFT+JOGWHEEL
PioneerDDJ400.bendScale = 0.5;

PioneerDDJ400.pointJumpSpace = 0.005; // amount in percent of the Song we can jump back to previous Cue or loop point

PioneerDDJ400.tempoRanges = [0.06, 0.10, 0.16, 0.25]; // WIDE = 25%?

// Keyboard Mode Variables and Settings
PioneerDDJ400.keyboardHotCuePoint = [0, 0]; // selected HotCue point (eg. PAD) in Keyboard mode per Deck 0 = unset
PioneerDDJ400.keyboardModeRefCount = [0, 0]; // count the currently pressed Pads per Deck
PioneerDDJ400.halftoneToPadMap = [4, 5, 6, 7, 0, 1, 2, 3];

// Beatjump pad (beatjump_size values)
PioneerDDJ400.beatjumpPad = {
    0x20: -1, // PAD 1
    0x21: 1,  // PAD 2
    0x22: -2, // PAD 3
    0x23: 2,  // PAD 4
    0x24: -4, // PAD 5
    0x25: 4,  // PAD 6
    0x26: -8, // PAD 7
    0x27: 8   // PAD 8
};

// Loop Section
PioneerDDJ400.loopAdjustIn = [false, false];
PioneerDDJ400.loopAdjustOut = [false, false];
PioneerDDJ400.loopAdjustMultiply = 50;

// Hotcue Pads saved Loop points
PioneerDDJ400.hotcueLoopPoints = {
    "[Channel1]": [],
    "[Channel2]": []
};

PioneerDDJ400.samplerCallbacks = [];

PioneerDDJ400.init = function() {
    // init controller

    // show focus buttons on Effect Ract 1 only
    engine.setValue("[EffectRack1_EffectUnit1]", "show_focus", 1);

    // Connect the VU-Meter LEDS
    engine.makeConnection("[Channel1]", "VuMeter", PioneerDDJ400.vuMeterUpdate);
    engine.makeConnection("[Channel2]", "VuMeter", PioneerDDJ400.vuMeterUpdate);

    // reset vumeter
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.deck1.vuMeter, false);
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.deck2.vuMeter, false);

    // enable soft takeover for rate controls
    // enable soft takeover for rate controls and FX level/depth
    engine.softTakeover("[Channel1]", "rate", true);
    engine.softTakeover("[Channel2]", "rate", true);
    engine.softTakeover("[EffectRack1_EffectUnit1_Effect1]", "meta", true);
    engine.softTakeover("[EffectRack1_EffectUnit1_Effect2]", "meta", true);
    engine.softTakeover("[EffectRack1_EffectUnit1_Effect3]", "meta", true);
    engine.softTakeover("[EffectRack1_EffectUnit1]", "mix", true);

    // Sampler callbacks
    for (var i = 1; i <= 16; ++i) {
        PioneerDDJ400.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "play", PioneerDDJ400.samplerPlayOutputCallbackFunction));
    }

    // trigger "track loaded" animations when a track is loaded
    engine.makeConnection("[Channel1]", "track_loaded", PioneerDDJ400.trackLoadedLED);
    engine.makeConnection("[Channel2]", "track_loaded", PioneerDDJ400.trackLoadedLED);

    // eye candy : play the "track loaded" animation on both decks at startup
    midi.sendShortMsg(0x9F, 0x00, 0x7F);
    midi.sendShortMsg(0x9F, 0x01, 0x7F);

    // resets pad mode to hotcue
    PioneerDDJ400.performancePads.reset();

    // turn on loop in and out lights
    PioneerDDJ400.setLoopButtonLights(0x90, 0x7F);
    PioneerDDJ400.setLoopButtonLights(0x91, 0x7F);

    // handle loop toggle events
    engine.makeConnection("[Channel1]", "loop_enabled", PioneerDDJ400.loopToggle);
    engine.makeConnection("[Channel2]", "loop_enabled", PioneerDDJ400.loopToggle);

    // poll the controller for current control positions on startup
    midi.sendSysexMsg([0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x02, 0x06, 0x00, 0x03, 0x01, 0xf7], 12);
};

PioneerDDJ400.trackLoadedLED = function(value, group, _control) {
    if (value) {
        value = 0x7F;
    } else {
        value = 0x00;
    }
    var channel = group.match(/^\[Channel(\d+)\]$/)[1];
    midi.sendShortMsg(0x9F, 0x00+(channel-1), value);
};

PioneerDDJ400.toggleLight = function(midiIn, active) {
    midi.sendShortMsg(midiIn.status, midiIn.data1, active ? 0x7F : 0);
};

//
// Loop IN/OUT ADJUST
//

PioneerDDJ400.toggleLoopAdjustIn = function (channel, _control, value, _status, group) {
    if (value === 0 || engine.getValue(group, 'loop_enabled' === 0)) {
        return;
    }
    PioneerDDJ400.loopAdjustIn[channel] = !PioneerDDJ400.loopAdjustIn[channel];
    PioneerDDJ400.loopAdjustOut[channel] = false;
};

PioneerDDJ400.toggleLoopAdjustOut = function (channel, _control, value, _status, group) {
    if (value === 0 || engine.getValue(group, 'loop_enabled' === 0)) {
        return;
    }
    PioneerDDJ400.loopAdjustOut[channel] = !PioneerDDJ400.loopAdjustOut[channel];
    PioneerDDJ400.loopAdjustIn[channel] = false;
};

PioneerDDJ400.setReloopLight = function (status, value) {
    midi.sendShortMsg(status, 0x4D, value);
    midi.sendShortMsg(status, 0x50, value);
};


PioneerDDJ400.setLoopButtonLights = function(status, value) {
    [0x10, 0x11, 0x4E, 0x4C].forEach(function(control) {
        midi.sendShortMsg(status, control, value);
    });
};

PioneerDDJ400.startLoopLightsBlink = function (channel, control, status, group) {
    var blink = 0x7F;

    PioneerDDJ400.stopLoopLightsBlink(group, control, status);

    PioneerDDJ400.timers[group][control] = engine.beginTimer(500, function() {
        blink = 0x7F - blink;

        // When adjusting the loop out position, turn the loop in light off
        if (PioneerDDJ400.loopAdjustOut[channel]) {
            midi.sendShortMsg(status, 0x10, 0x00);
            midi.sendShortMsg(status, 0x4C, 0x00);
        }
        else {
            midi.sendShortMsg(status, 0x10, blink);
            midi.sendShortMsg(status, 0x4C, blink);
        }

        // When adjusting the loop in position, turn the loop out light off
        if (PioneerDDJ400.loopAdjustIn[channel]) {
            midi.sendShortMsg(status, 0x11, 0x00);
            midi.sendShortMsg(status, 0x4E, 0x00);
        }
        else {
            midi.sendShortMsg(status, 0x11, blink);
            midi.sendShortMsg(status, 0x4E, blink);
        }
    });

};

PioneerDDJ400.stopLoopLightsBlink = function (group, control, status) {
    PioneerDDJ400.timers[group] = PioneerDDJ400.timers[group] || {};

    if (PioneerDDJ400.timers[group][control] !== undefined) {
        engine.stopTimer(PioneerDDJ400.timers[group][control]);
    }
    PioneerDDJ400.timers[group][control] = undefined;
    PioneerDDJ400.setLoopButtonLights(status, 0x7F);
};

PioneerDDJ400.loopToggle = function (value, group, control) {
    var status = group === "[Channel1]" ? 0x90 : 0x91,
        channel = group === "[Channel1]" ? 0 : 1;

    PioneerDDJ400.setReloopLight(status, value ? 0x7F : 0x00);

    if (value) {
        PioneerDDJ400.startLoopLightsBlink(channel, control, status, group);
    }
    else {
        PioneerDDJ400.stopLoopLightsBlink(group, control, status);
        PioneerDDJ400.loopAdjustOut[channel] = PioneerDDJ400.loopAdjustIn[channel] = false;
    }
};

//
// CUE/LOOP CALL
//

PioneerDDJ400.cueLoopCallLeft = function(_channel, _control, value, _status, group) {
    if (value && engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "loop_scale", 0.5);
    }
};

PioneerDDJ400.cueLoopCallRight = function(_channel, _control, value, _status, group) {
    if (value && engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "loop_scale", 2.0);
    }
};

PioneerDDJ400.phraseJumpForward = function(_channel, _control, value, _status, group) {
    if (value) {
        engine.setValue(group, 'beatjump', 8 * 4);
    }
};

PioneerDDJ400.phraseJumpBack = function(_channel, _control, value, _status, group) {
    if (value) {
        engine.setValue(group, 'beatjump', -8 * 4);
    }
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
    // "highspeed" (scaleup value) pitch bend
    var newVal = (value - 64) * this.highspeedScale;
    engine.setValue(group, "jog", newVal);
};

PioneerDDJ400.jogTouch = function(channel, _control, value) {
    var deckNum = channel + 1;

    // skip scratchmode if we adjust the loop points
    if (PioneerDDJ400.loopAdjustIn[channel] || PioneerDDJ400.loopAdjustOut[channel]) {
        return;
    }

    // on touch jog with vinylmode enabled -> enable scratchmode
    if (value !== 0 && this.vinylMode) {
        engine.scratchEnable(deckNum, 720, 33+1/3, this.alpha, this.beta);
    } else {
        // on release jog (value === 0) disable pitch bend mode or scratch mode
        engine.scratchDisable(deckNum);
    }
};

///////////////////////////////////////////////////////////////
//            HIGH RESOLUTION MIDI INPUT HANDLERS            //
///////////////////////////////////////////////////////////////

PioneerDDJ400.highResMSB = {
    '[Channel1]': {},
    '[Channel2]': {},
    '[Channel3]': {},
    '[Channel4]': {}
};

PioneerDDJ400.tempoSliderMSB = function(channel, control, value, status, group) {
    PioneerDDJ400.highResMSB[group].tempoSlider = value;
};

PioneerDDJ400.tempoSliderLSB = function(channel, control, value, status, group) {
    var fullValue = (PioneerDDJ400.highResMSB[group].tempoSlider << 7) + value;

    engine.setValue(
        group,
        'rate',
        ((0x4000 - fullValue) - 0x2000) / 0x2000
    );
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

// Stores the performance pad mode each time it changes
PioneerDDJ400.setPadmode = function (channel, control, value) {
    if (value === 0x7F) {
        PioneerDDJ400.performancePads.state[channel] = control;
    }
}

PioneerDDJ400.keyboardMode = function(channel, _control, value, _status, group) {
    if (value > 0) {
        // clear current set hotcue point and refcount for keyboard mode
        this.keyboardHotCuePoint[channel] = 0;
        this.keyboardModeRefCount[channel] = 0;
        // reset pitch
        engine.setValue(group, "pitch", 0.0);
        this.keyboardModeEnabledOutput(channel, group);
    }
};


PioneerDDJ400.keyboardModeEnabledOutput = function(channel, group) {
    var status = channel === 0 ? 0x97 : 0x99;
    var hotcuePad = 1;

    if (this.keyboardHotCuePoint[channel] === 0) {
        for (hotcuePad = 1; hotcuePad <= 8; hotcuePad++) {
            var hotcueEnabled = engine.getValue(group, "hotcue_"+hotcuePad+"_enabled");

            midi.sendShortMsg(status, 0x40 + hotcuePad-1, hotcueEnabled > 0 ? 0x7F : 0);
            // shift lights on if hotcue is set
            midi.sendShortMsg(status+1, 0x40 + hotcuePad-1, hotcueEnabled > 0 ? 0x7F : 0);
        }
    } else {
        // enable all LEDs
        for (hotcuePad = 1; hotcuePad <= 8; hotcuePad++) {
            midi.sendShortMsg(status, 0x40 + hotcuePad-1, 0x7F);
        }
    }
    // shift keyboard Pad 7 and 8 are always enabled
    midi.sendShortMsg(status+1, 0x46, 0x7F);
    midi.sendShortMsg(status+1, 0x47, 0x7F);
};


PioneerDDJ400.keyboardModePad = function(channel, control, value, _status, group) {
    channel = (channel & 0xf) < 10 ? 0 : 1;
    var padNum = (control & 0xf) + 1;
    var hotcuePad = this.keyboardHotCuePoint[channel];

    // if no hotcue is set for keyboard mode set on first press on a pad
    if (hotcuePad === 0 && value !== 0) {
        hotcuePad = padNum;
        this.keyboardHotCuePoint[channel] = hotcuePad;
        // if there is no hotcue at this pad, set current play position
        var hotcuePos = engine.getValue(group, "hotcue_"+hotcuePad+"_position");

        if (hotcuePos < 0) {
            engine.setValue(group, "hotcue_"+hotcuePad+"_set", 1);
        }

        this.keyboardModeRefCount[channel] = 0; // reset count
        this.keyboardModeEnabledOutput(channel, group);
        return;
    }

    // if hotcue point is set perform coresponding halftone operation
    if (value > 0) {
        // count pressed Pad per deck
        this.keyboardModeRefCount[channel] += 1;
        var newValue = this.halftoneToPadMap[padNum-1];

        engine.setValue(group, "pitch", newValue);
        engine.setValue(group, "hotcue_"+hotcuePad+"_gotoandplay", 1);
    } else {
        // decrease the number of active Pads, this should minimize unwanted stops
        this.keyboardModeRefCount[channel] -= 1;
        if (this.keyboardModeRefCount[channel] <= 0) {
            engine.setValue(group, "hotcue_"+hotcuePad+"_gotoandstop", 1);
            engine.setValue(group, "pitch", 0.0); // reset pitch
            this.keyboardModeRefCount[channel] = 0; // reset refcount to 0
        }
    }
};

PioneerDDJ400.keyshiftModePad = function(_channel, control, value, _status, group) {
    if (value === 0) {
        return; // ignore release
    }
    engine.setValue(group, "pitch", this.halftoneToPadMap[control & 0xf]);
};

PioneerDDJ400.samplerModeShiftPadPressed = function(_channel, _control, value, _status, group) {
    if (value === 0) {
        return; // ignore release
    }

    var playing = engine.getValue(group, "play");
    // when playing stop and return to start/cue point
    if (playing > 0) {
        engine.setValue(group, "cue_gotoandstop", 1);
    } else { // load selected track
        engine.setValue(group, "LoadSelectedTrack", 1);
    }
};


PioneerDDJ400.shiftPressed = function(channel, _control, value) {
    this.shiftState[channel] = value;
    this.performancePads.toggleShiftLights(channel, value);
};

PioneerDDJ400.waveFormRotate = function(_channel, _control, value) {
    // select the Waveform to zoom left shift = deck1, right shift = deck2
    var deckNum = this.shiftState[0] > 0 ? 1 : 2;
    var oldVal = engine.getValue("[Channel"+deckNum+"]", "waveform_zoom");
    var newVal = oldVal + (value > 0x64 ? 1 : -1);

    engine.setValue("[Channel"+deckNum+"]", "waveform_zoom", newVal);
};

// START BEAT FX

PioneerDDJ400.numFxSlots = 3;

Object.defineProperty(PioneerDDJ400, "selectedFxSlot", {
    get: function() {
        return engine.getValue("[EffectRack1_EffectUnit1]", "focused_effect");
    },
    set: function(value) {
        if (value < 0 || value > PioneerDDJ400.numFxSlots) {
            return;
        }
        engine.setValue("[EffectRack1_EffectUnit1]", "focused_effect", value);
        var isEffectEnabled = engine.getValue(PioneerDDJ400.selectedFxGroup, "enabled");
        PioneerDDJ400.toggleLight(LightsPioneerDDJ400.beatFx, isEffectEnabled);
    },
});

Object.defineProperty(PioneerDDJ400, "selectedFxGroup", {
    get: function() {
        return "[EffectRack1_EffectUnit1_Effect" + PioneerDDJ400.selectedFxSlot + "]";
    },
});

PioneerDDJ400.beatFxLevelDepthRotate = function(_channel, _control, value) {
    var newVal = value === 0 ? 0 : (value / 0x7F);
    var effectOn = engine.getValue(PioneerDDJ400.selectedFxGroup, "enabled");

    if (effectOn) {
        engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit1]", "mix");
        engine.setValue(PioneerDDJ400.selectedFxGroup, "meta", newVal);
    } else {
        engine.softTakeoverIgnoreNextValue(PioneerDDJ400.selectedFxGroup, "meta");
        engine.setValue("[EffectRack1_EffectUnit1]", "mix", newVal);
    }
};

PioneerDDJ400.beatFxSelectPressed = function(_channel, _control, value) {
    if (value === 0) { return; }

    // focus Effect Slot 3 in Effect Unit 1, or clear focus if it is currently focused
    if (PioneerDDJ400.selectedFxSlot === 3) {
        PioneerDDJ400.selectedFxSlot = 0;
    } else {
        PioneerDDJ400.selectedFxSlot = 3;
    }
};

PioneerDDJ400.beatFxSelectPreviousEffect = function(_channel, _control, value) {
    engine.setValue(PioneerDDJ400.selectedFxGroup, "prev_effect", value);
};

PioneerDDJ400.beatFxSelectNextEffect = function(_channel, _control, value) {
    engine.setValue(PioneerDDJ400.selectedFxGroup, "next_effect", value);
};

PioneerDDJ400.beatFxLeftPressed = function(_channel, _control, value) {
    if (value === 0) { return; }

    // focus Effect Slot 1 in Effect Unit 1, or clear focus if it is currently focused
    if (PioneerDDJ400.selectedFxSlot === 1) {
        PioneerDDJ400.selectedFxSlot = 0;
    } else {
        PioneerDDJ400.selectedFxSlot = 1;
    }
};

PioneerDDJ400.beatFxRightPressed = function(_channel, _control, value) {
    if (value === 0) { return; }

    // focus Effect Slot 2 in Effect Unit 1, or clear focus if it is currently focused
    if (PioneerDDJ400.selectedFxSlot === 2) {
        PioneerDDJ400.selectedFxSlot = 0;
    } else {
        PioneerDDJ400.selectedFxSlot = 2;
    }
};

PioneerDDJ400.beatFxOnOffPressed = function(_channel, _control, value) {
    if (value === 0) { return; }

    // toggle the currently focused effect slot in Effect Unit 1 (if any)
    var selectedSlot = PioneerDDJ400.selectedFxSlot;
    if (selectedSlot <= 0 || selectedSlot > PioneerDDJ400.numFxSlots) {
        return;
    }
    var isEnabled = !engine.getValue(PioneerDDJ400.selectedFxGroup, "enabled");
    engine.setValue(PioneerDDJ400.selectedFxGroup, "enabled", isEnabled);
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.beatFx, isEnabled);
};

PioneerDDJ400.beatFxOnOffShiftPressed = function(_channel, _control, value) {
    if (value === 0) { return; }

    // turn off all three effect slots in Effect Unit 1
    for (var i = 1; i <= PioneerDDJ400.numFxSlots; i += 1) {
        engine.setValue("[EffectRack1_EffectUnit1_Effect" + i + "]", "enabled", 0);
    }
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.beatFx, false);
};

PioneerDDJ400.beatFxChannel = function(_channel, control, _value, _status, group) {
    var enableChannel1 = control === 0x10 || control === 0x14;
    var enableChannel2 = control === 0x11 || control === 0x14;

    engine.setValue(group, "group_[Channel1]_enable", enableChannel1);
    engine.setValue(group, "group_[Channel2]_enable", enableChannel2);
};

//
// PAD FX
//

PioneerDDJ400.padFxBelowPressed = function(channel, control, value, status, group) {
    var groupAbove = group.replace(/\[EffectRack1_EffectUnit(\d+)_Effect(\d+)]/, function(all, unit, effect) {
        var effectAbove = parseInt(effect) - 4;

        return "[EffectRack1_EffectUnit" + unit + "_Effect" + effectAbove + "]";
    });

    engine.setValue(groupAbove, "next_effect", value);
};

PioneerDDJ400.padFxShiftBelowPressed = function(channel, control, value, status, group) {
    var groupAbove = group.replace(/\[EffectRack1_EffectUnit(\d+)_Effect(\d+)]/, function(all, unit, effect) {
        var effectAbove = parseInt(effect) - 4;

        return "[EffectRack1_EffectUnit" + unit + "_Effect" + effectAbove + "]";
    });

    engine.setValue(groupAbove, "prev_effect", value);
};

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
// BEATJUMP
//

PioneerDDJ400.beatjumpPadPressed = function(_channel, control, value, _status, group) {
    if (value === 0) {
        return;
    }
    engine.setValue(group, "beatjump_size", Math.abs(PioneerDDJ400.beatjumpPad[control]));
    engine.setValue(group, "beatjump", PioneerDDJ400.beatjumpPad[control]);
};

PioneerDDJ400.beatjumpShiftUp = function(_channel, control, value, _status, group) {
    if (value === 0 || PioneerDDJ400.beatjumpPad[0x21] * 16 > 16) {
        return;
    }
    Object.keys(PioneerDDJ400.beatjumpPad).forEach(function(pad) {
        PioneerDDJ400.beatjumpPad[pad] = PioneerDDJ400.beatjumpPad[pad] * 16;
    });
    engine.setValue(group, "beatjump_size", PioneerDDJ400.beatjumpPad[0x21]);
};

PioneerDDJ400.beatjumpShiftDown = function(_channel, control, value, _status, group) {
    if (value === 0 || PioneerDDJ400.beatjumpPad[0x21] / 16 < 1/16) {
        return;
    }
    Object.keys(PioneerDDJ400.beatjumpPad).forEach(function(pad) {
        PioneerDDJ400.beatjumpPad[pad] = PioneerDDJ400.beatjumpPad[pad] / 16;
    });
    engine.setValue(group, "beatjump_size", PioneerDDJ400.beatjumpPad[0x21]);
};

//
// SAMPLERS
//

PioneerDDJ400.samplerPlayOutputCallbackFunction = function(value, group, _control) {
    if (value === 1) {
        var curPad = group.match(/^\[Sampler(\d+)\]$/)[1];
        PioneerDDJ400.startSamplerBlink((0x97 + (curPad > 8 ? 2 : 0)), (0x30 + ((curPad > 8 ? curPad-8 : curPad)-1)), group);
    }
};

PioneerDDJ400.samplerModePadPressed = function(_channel, _control, value, _status, group) {
    if (engine.getValue(group, "track_loaded")) {
        engine.setValue(group, "cue_gotoandplay", value);
    }
    else {
        engine.setValue(group, "LoadSelectedTrack", value);
    }
};


PioneerDDJ400.samplerModeShiftPadPressed = function(_channel, _control, value, _status, group) {
    if (engine.getValue(group, "play")) {
        engine.setValue(group, "cue_gotoandstop", value);
    } else if (engine.getValue(group, "track_loaded")) {
        engine.setValue(group, "eject", value);
    }
};

PioneerDDJ400.startSamplerBlink = function (channel, control, group) {
    var val = 0x7f;

    PioneerDDJ400.stopSamplerBlink(channel, control);
    PioneerDDJ400.timers[channel][control] = engine.beginTimer(250, function() {
        val = 0x7f - val;

        // blink the appropriate pad
        midi.sendShortMsg(channel, control, val);
        // also blink the pad while SHIFT is pressed
        midi.sendShortMsg((channel+1), control, val);

        var isPlaying = engine.getValue(group, 'play') === 1;

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

PioneerDDJ400.shutdown = function() {
    // reset vumeter
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.deck1.vuMeter, false);
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.deck2.vuMeter, false);

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
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.beatFx, false);
};
