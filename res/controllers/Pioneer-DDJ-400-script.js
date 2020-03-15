// Pioneer-DDJ-400-script.js
// ****************************************************************************
// * Mixxx mapping script file for the Pioneer DDJ-400.
// * Author: Warker, nschloe, dj3730
// * Forum: https://mixxx.org/forums/viewtopic.php?f=7&t=12113
// * Wiki: https://www.mixxx.org/wiki/doku.php/pioneer_ddj-400
//
// Upstream MIDI spec:
// * https://www.pioneerdj.com/-/media/pioneerdj/software-info/controller/ddj-400/ddj-400_midi_message_list_e1.pdf
//
//             Working:
//                 * Mixer Section (Faders, EQ, Filter, Gain, Cue)
//                 * Browsing and loading + Waveform zoom (shift)
//                 * Jogwheels, Scratching, Bending
//                 * cycle Temporange
//                 * Beat Sync
//                 * Beat Loop Mode
//                 * Sampler Mode
//                 * BeatFX (controls Effect Unit 1.  LEFT selects EFFECT1, RIGHT selects EFFECT2, FX_SELECT selects EFFECT3.
//                   ON/OFF toggles selected effect slot.  SHIFT+ON/OFF disables all three effect slots.
//                 * Hot Cue Mode
//
//             Partially:
//                 * Beatjump mode (no lighting, shift mode(adjust jump size))
//                 * PAD FX (only slots A-H, Q-P)
//                 * Output (lights)
//                 * Loop Section: Loop in / Out, Call, Double, Half 
//                   (loop adjust not working, '4 beat loop' doesnt work correctly - see comments in PioneerDDJ400.loopin4beatPressedLong)
//
//             Testing:
//                 * Keyboard Mode (check pitch value)
//                 * Keyshift Mode (check pitch value)
//
//             Not working/implemented:
//                 * Channel & Crossfader Start
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

// Beatjump Pads
PioneerDDJ400.beatjumpPad = [-1, 1, -2, 2, -4, 4, -8, 8]; // < 0 = left; else right
PioneerDDJ400.beatjumpMultiplier = [1, 1]; // Beatjump Beatcount per Deck

// Hotcue Pads saved Loop points
PioneerDDJ400.hotcueLoopPoints = {
    "[Channel1]": [],
    "[Channel2]": []
};

// Loop Section
PioneerDDJ400.loopin4beat = [false, false]; // inn4loop is pressed
PioneerDDJ400.loopout = [false, false]; // out loop is pressed
PioneerDDJ400.loopAdjustMultiply = 5;

PioneerDDJ400.samplerCallbacks = [];

// Wrapper to easily ignore the function when the button is released.
var ignoreRelease = function(fn) {
    "use strict";
    return function(channel, control, value, status, group) {
        if (value === 0) { // This means the button is released.
            return;
        }
        return fn(channel, control, value, status, group);
    };
};


PioneerDDJ400.init = function() {
    "use strict";
    // init controller
    // init tempo Range to 10% (default = 6%)
    engine.setValue("[Channel1]", "rateRange", PioneerDDJ400.tempoRanges[1]);
    engine.setValue("[Channel2]", "rateRange", PioneerDDJ400.tempoRanges[1]);

    // show focus buttons on Effect Ract 1 only
    engine.setValue("[EffectRack1_EffectUnit1]", "show_focus", 1);
    engine.setValue("[EffectRack1_EffectUnit2]", "show_focus", 0);
    engine.setValue("[EffectRack1_EffectUnit3]", "show_focus", 0);

    // Connect the VU-Meter LEDS
    engine.connectControl("[Channel1]", "VuMeter", "PioneerDDJ400.vuMeterUpdate");
    engine.connectControl("[Channel2]", "VuMeter", "PioneerDDJ400.vuMeterUpdate");

    // reset vumeter
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.deck1.vuMeter, false);
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.deck2.vuMeter, false);
    
    // DJ3730:  added
    // enable soft takeover for rate controls
    engine.softTakeover("[Channel1]", "rate", true);
    engine.softTakeover("[Channel2]", "rate", true);


    // DJ3730:  added
    // Sampler callbacks
    for (i = 1; i <= 16; ++i) {
        PioneerDDJ400.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "play", PioneerDDJ400.samplerPlayOutputCallbackFunction));
    }

    // poll the controller for current control positions on startup
    midi.sendSysexMsg([0xF0,0x00,0x40,0x05,0x00,0x00,0x02,0x06,0x00,0x03,0x01,0xf7], 12);
};

PioneerDDJ400.toggleLight = function(midiIn, active) {
    "use strict";
    midi.sendShortMsg(midiIn.status, midiIn.data1, active ? 0x7F : 0);
};

PioneerDDJ400.jogTurn = function(channel, _control, value, _status, group) {
    "use strict";
    var deckNum = channel + 1;
    // wheel center at 64; <64 rew >64 fwd
    var newVal = value - 64;

    // loop_in / out adjust
    var loopEnabled = engine.getValue(group, "loop_enabled");
    if (loopEnabled > 0) {
        if (this.loopin4beat[channel]) {
            newVal = newVal * this.loopAdjustMultiply + engine.getValue(group, "loop_start_position");
            engine.setValue(group, "loop_start_position", newVal);
            return;
        }
        if (this.loopout[channel]) {
            newVal = newVal * this.loopAdjustMultiply + engine.getValue(group, "loop_end_position");
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
    "use strict";
    // "highspeed" (scaleup value) pitch bend
    var newVal = (value - 64) * this.highspeedScale;
    engine.setValue(group, "jog", newVal);
};

PioneerDDJ400.jogTouch = function(channel, _control, value) {
    "use strict";
    var deckNum = channel + 1;

    // skip scratchmode if we adjust the loop points
    if (this.loopin4beat[channel] || this.loopout[channel]) {
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


PioneerDDJ400.tempoSliderMSB = function (channel, control, value, status, group) {
    PioneerDDJ400.highResMSB[group].tempoSlider = value;
};

PioneerDDJ400.tempoSliderLSB = function (channel, control, value, status, group) {
    var fullValue = (PioneerDDJ400.highResMSB[group].tempoSlider << 7) + value;

    engine.setValue(
        group,
        'rate',
        ((0x4000 - fullValue) - 0x2000) / 0x2000
    );
 
 
};

PioneerDDJ400.cycleTempoRange = function(_channel, _control, value, _status, group) {
    "use strict";
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


var sortAsc = function(a, b) {
    // returns 1 if a > b, -1 if a < b, and 0 otherwise
    "use strict";
    return (a > b) ? 1 : (b > a ? -1 : 0);
};

PioneerDDJ400.initCuePointsAndLoops = function(group) {
    "use strict";
    // create a list of positions in the track which can be selected
    var points = [];

    for (var padNum = 1; padNum <= 8; padNum++) {
        points.push(engine.getValue(group, "hotcue_"+padNum+"_position"));
    }
    points.push(engine.getValue(group, "cue_point"));
    points.push(engine.getValue(group, "loop_start_position"));
    points.push(engine.getValue(group, "loop_end_position"));
    points.sort(sortAsc); // sort asc
    return points;
};

PioneerDDJ400.cueLoopCallLeft = function(_channel, _control, value, _status, group) {
    "use strict";
    if (value === 0) {
        // ignore release
        return;
    }

    var loopEnabled = engine.getValue(group, "loop_enabled");
    if (loopEnabled) {
        // loop halve
        engine.setValue(group, "loop_scale", 0.5);
    } else {
        var currentPosition = engine.getValue(group, "playposition") - this.pointJumpSpace;
        var trackSamples = engine.getValue(group, "track_samples");
        var points = this.initCuePointsAndLoops(group);
        var newPosition = currentPosition;

        for (var i = 1; i <= points.length; i++) {
            if (i === points.length || points[i] >= currentPosition * trackSamples) {
                newPosition = points[i-1] / trackSamples;
                break;
            }
        }
        //engine.setValue(group, 'loop_in_goto', 1);
        engine.setValue(group, "playposition", newPosition);
    }
};

PioneerDDJ400.cueLoopCallRight = function(_channel, _control, value, _status, group) {
    "use strict";
    if (value === 0) {
        return; // ignore release
    }

    var loopEnabled = engine.getValue(group, "loop_enabled");
    if (loopEnabled) {
        // loop double
        engine.setValue(group, "loop_scale", 2.0);
    } else {
        // jump through the cue points
        var currentPosition = engine.getValue(group, "playposition");
        var trackSamples = engine.getValue(group, "track_samples");
        var points = this.initCuePointsAndLoops(group);

        var newPosition = currentPosition;

        for (var i = 0; i < points.length; i++) {
            if (points[i] > currentPosition * trackSamples) {
                newPosition = points[i] / trackSamples;
                break;
            }
        }
        engine.setValue(group, "playposition", newPosition);
    }
};

PioneerDDJ400.keyboardMode = function(channel, _control, value, _status, group) {
    "use strict";
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
    "use strict";
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
    "use strict";
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
    "use strict";
    if (value === 0) {
        return; // ignore release
    }
    engine.setValue(group, "pitch", this.halftoneToPadMap[control & 0xf]);
};

PioneerDDJ400.samplerModeShiftPadPressed = function(_channel, _control, value, _status, group) {
    "use strict";
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
    "use strict";
    this.shiftState[channel] = value;
};

PioneerDDJ400.waveFormRotate = function(_channel, _control, value) {
    "use strict";
    // select the Waveform to zoom left shift = deck1, right shift = deck2
    var deckNum = this.shiftState[0] > 0 ? 1 : 2;
    var oldVal = engine.getValue("[Channel"+deckNum+"]", "waveform_zoom");
    var newVal = oldVal + (value > 0x64 ? 1 : -1);

    engine.setValue("[Channel"+deckNum+"]", "waveform_zoom", newVal);
};

PioneerDDJ400.loopin4beatPressed = function(channel, _control, value, _status, group) {
    "use strict";
    var loopEnabled = engine.getValue(group, "loop_enabled");
    this.loopin4beat[channel] = (value > 0);

    if (loopEnabled === 0 && value > 0) {
        engine.setValue(group, "loop_in", 1);
    }
};

PioneerDDJ400.loopin4beatPressedLong = function(_channel, _control, value, _status, group) {
    // problematic - loop gets set to the playback position where the 'long press' was recognized
	// and not to the point at which the button was initially pressed
	// as a result, the loop is not set where one would expect
    "use strict";
    var loopEnabled = engine.getValue(group, "loop_enabled");
    if (!loopEnabled && value > 0) {
        engine.setValue(group, "beatloop_4_activate", 1);
    }
};

PioneerDDJ400.loopoutPressed = function(channel, _control, value, _status, group) {
    "use strict";
    var loopEnabled = engine.getValue(group, "loop_enabled");
    this.loopout[channel] = (value > 0);

    if (loopEnabled === 0 && value > 0) {
        engine.setValue(group, "loop_out", 1);
    }
};

// START BEAT FX

PioneerDDJ400.numFxSlots = 3;

Object.defineProperty(PioneerDDJ400, "selectedFxSlot", {
    get: function() {
        "use strict";
        return engine.getValue("[EffectRack1_EffectUnit1]", "focused_effect");
    },
    set: function(value) {
        "use strict";
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
        "use strict";
        return "[EffectRack1_EffectUnit1_Effect" + PioneerDDJ400.selectedFxSlot + "]";
    },
});

PioneerDDJ400.beatFxLevelDepthRotate = function(_channel, _control, value) {
    "use strict";
    var newVal = value === 0 ? 0 : (value / 0x7F);
    var effectOn = engine.getValue(PioneerDDJ400.selectedFxGroup, "enabled");

    if (effectOn) {
        engine.setValue(PioneerDDJ400.selectedFxGroup, "meta", newVal);
    } else {
        engine.setValue("[EffectRack1_EffectUnit1]", "mix", newVal);
    }
};

PioneerDDJ400.beatFxSelectPressed = ignoreRelease(function() {
    // focus Effect Slot 3 in Effect Unit 1, or clear focus if it is currently focused
    "use strict";
    if (PioneerDDJ400.selectedFxSlot == 3) {
        PioneerDDJ400.selectedFxSlot = 0;
    } else {
        PioneerDDJ400.selectedFxSlot = 3;
    }
});

PioneerDDJ400.beatFxSelectShiftPressed = function(_channel, _control, value) {
    "use strict";
    //engine.setValue(PioneerDDJ400.selectedFxGroup, "prev_effect", value);
};

PioneerDDJ400.beatFxLeftPressed = ignoreRelease(function() {
    // focus Effect Slot 1 in Effect Unit 1, or clear focus if it is currently focused
    "use strict";
    if (PioneerDDJ400.selectedFxSlot == 1) {
        PioneerDDJ400.selectedFxSlot = 0;
    } else {
        PioneerDDJ400.selectedFxSlot = 1;
    }
});

PioneerDDJ400.beatFxRightPressed = ignoreRelease(function() {
    // focus Effect Slot 2 in Effect Unit 1, or clear focus if it is currently focused
    "use strict";
    if (PioneerDDJ400.selectedFxSlot == 2) {
        PioneerDDJ400.selectedFxSlot = 0;
    } else {
        PioneerDDJ400.selectedFxSlot = 2;
    }
});

PioneerDDJ400.beatFxOnOffPressed = ignoreRelease(function() {
    // toggle the currently focused effect slot in Effect Unit 1 (if any)
    "use strict";
    var selectedSlot = PioneerDDJ400.selectedFxSlot;
    if (selectedSlot <= 0 || selectedSlot > PioneerDDJ400.numFxSlots) {
        return;
    }
    var isEnabled = !engine.getValue(PioneerDDJ400.selectedFxGroup, "enabled");
    engine.setValue(PioneerDDJ400.selectedFxGroup, "enabled", isEnabled);
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.beatFx, isEnabled);
});

PioneerDDJ400.beatFxOnOffShiftPressed = ignoreRelease(function() {
    // turn off all three effect slots in Effect Unit 1
    "use strict";
    for (var i = 1; i <= PioneerDDJ400.numFxSlots; i += 1) {
        engine.setValue("[EffectRack1_EffectUnit1_Effect" + i + "]", "enabled", 0);
    }
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.beatFx, false);
});

PioneerDDJ400.beatFxChannel = ignoreRelease(function(_channel, control, _value, _status, group) {
    "use strict";
    var enableChannel1 = control === 0x10 || control === 0x14;
    var enableChannel2 = control === 0x11 || control === 0x14;

    engine.setValue(group, "group_[Channel1]_enable", enableChannel1);
    engine.setValue(group, "group_[Channel2]_enable", enableChannel2);
});

PioneerDDJ400.padFxBelowPressed = ignoreRelease(function(channel, control, value, status, group) {
    "use strict";
    var groupAbove = group.replace(/\[EffectRack1_EffectUnit(\d+)_Effect(\d+)]/, function(all, unit, effect) {
        var effectAbove = parseInt(effect) - 4;

        return "[EffectRack1_EffectUnit" + unit + "_Effect" + effectAbove + "]";
    });

    engine.setValue(groupAbove, "next_effect", value);
});

PioneerDDJ400.padFxShiftBelowPressed = ignoreRelease(function(channel, control, value, status, group) {
    "use strict";
    var groupAbove = group.replace(/\[EffectRack1_EffectUnit(\d+)_Effect(\d+)]/, function(all, unit, effect) {
        var effectAbove = parseInt(effect) - 4;

        return "[EffectRack1_EffectUnit" + unit + "_Effect" + effectAbove + "]";
    });

    engine.setValue(groupAbove, "prev_effect", value);
});

// END BEAT FX

PioneerDDJ400.vuMeterUpdate = function(value, group) {
    "use strict";
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
// SAMPLERS
//

// DJ3730: blink pad when sample playback starts
PioneerDDJ400.samplerPlayOutputCallbackFunction = function (value, group, control) {
    if (value === 1) {
        var curPad = group.match(/^\[Sampler(\d+)\]$/)[1];   // for some reason, using script.samplerRegEx here results in an error under Linux
        startSamplerBlink((0x97 + (curPad > 8 ? 2 : 0)), (0x30 + ((curPad > 8 ? curPad-8 : curPad)-1)), group);
    }
};

PioneerDDJ400.samplerModePadPressed = ignoreRelease(function(_channel, control, _value, status, group) {
    "use strict";
    var isLoaded = engine.getValue(group, "track_loaded") === 1;

    if (!isLoaded) {
        return;
    }
    engine.setValue(group, "cue_gotoandplay", 1);
});


PioneerDDJ400.samplerModeShiftPadPressed = function(_channel, _control, value, _status, group){
    "use strict";
    if(value == 0) {
        return; // ignore release
    }
    var playing = engine.getValue(group, 'play');
    // when playing stop and return to start/cue point
    if(playing > 0){
        engine.setValue(group, 'cue_gotoandstop', 1);
    }
    else{ // load selected track
        // engine.setValue(group, 'LoadSelectedTrack', 1);
    }
};

const TimersPioneerDDJ400 = {};

function startSamplerBlink(channel, control, group) {
    "use strict";
    var val = 0x7f;

    // print('channel ' + channel + ' +1= ' + (channel+1));
    
    stopSamplerBlink(channel, control);
    TimersPioneerDDJ400[channel][control] = engine.beginTimer(250, function() {
        val = 0x7f - val;

        // blink the appropriate pad
        midi.sendShortMsg(channel, control, val);
        // also blink the pad while SHIFT is pressed
        midi.sendShortMsg((channel+1), control, val);

        const isPlaying = engine.getValue(group, 'play') === 1;

        if (!isPlaying) {
            // kill timer
            stopSamplerBlink(channel, control);
            // set the pad LED to ON
            midi.sendShortMsg(channel, control, 0x7f);
            // set the pad LED to ON while SHIFT is pressed
            midi.sendShortMsg((channel+1), control, 0x7f);
        }
    });
}

function stopSamplerBlink(channel, control) {
    "use strict";
    TimersPioneerDDJ400[channel] = TimersPioneerDDJ400[channel] || {};

    if (TimersPioneerDDJ400[channel][control] !== undefined) {
        engine.stopTimer(TimersPioneerDDJ400[channel][control]);
        TimersPioneerDDJ400[channel][control] = undefined;
    }
}

PioneerDDJ400.shutdown = function() {
    "use strict";
    // reset vumeter
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.deck1.vuMeter, false);
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.deck2.vuMeter, false);

    // housekeeping
    // turn off all Sampler LEDs
    for (i = 0; i <= 7; ++i) {
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
};
