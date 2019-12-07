// Pioneer-DDJ-400-script.js
//
// ****************************************************************************
// * Mixxx mapping script file for the Pioneer DDJ-400.
// * Author: Nico Schlömer (nschloe) with parts by WarkerAnhaltRanger
// * Version 0.1 (Dec 7 2019)
// * Forum: https://mixxx.org/forums/viewtopic.php?f=7&t=12113
// * Wiki: https://www.mixxx.org/wiki/doku.php/pioneer_ddj-400
//
//            Working:
//                * Mixer Section (Faders, EQ, Filter, Gain, Cue)
//                * Browsing and loading
//                * Jogwheels, Scratching, Bending
//                * Beat Sync
//                * Beatjump mode
//                * Hot cue Mode
//                * Beat Loop Mode
//
//            Partially:
//                * Loop Section: Loop in / Out + Adjust, Call, Double, Half
//                * Hot Cue Mode (including loops)
//                * Output (lights)
//
//            Not working/implemented:
//                * Waveform zoom (shift)
//                * cycle Temporange
//                * Channel & Crossfader Start
//                * Sampler Mode
//                * PAD FX (only slots A-H, Q-P)
//                * Effect Section (without Beat FX left + Right - no equivalent function found)
//                * Effect Section (Beat FX left + Right - select the Effect Slot (not Effect BPM))
//                * Keyboard Mode (check pitch value)
//                * Keyshift Mode (check pitch value)
// ****************************************************************************
var PioneerDDJ400 = {};

// JogWheel
PioneerDDJ400.vinylMode = true;
PioneerDDJ400.alpha = 1.0/8;
PioneerDDJ400.beta = PioneerDDJ400.alpha/32;
PioneerDDJ400.highspeedScale = 2;
PioneerDDJ400.bendScale = 0.5;

var LightsPioneerDDJ400 = {
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


PioneerDDJ400.init = function() {
    "use strict";
    // Connect the VU-Meter LEDS
    engine.connectControl('[Channel1]','VuMeter','PioneerDDJ400.vuMeterUpdate');
    engine.connectControl('[Channel2]','VuMeter','PioneerDDJ400.vuMeterUpdate');

    // reset vumeter
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.deck1.vuMeter, false);
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.deck2.vuMeter, false);
};


PioneerDDJ400.vuMeterUpdate = function(value, group){
    'use strict';
    // The factor 150 here is a manual fit, looking at Mixxx's output and hte level
    // meter on the DDJ 400.
    var newVal = value * 150;

    switch(group){
        case '[Channel1]':
            midi.sendShortMsg(0xB0, 0x02, newVal);
            break;

        case '[Channel2]':
            midi.sendShortMsg(0xB1, 0x02, newVal);
            break;
    }
};


PioneerDDJ400.jogDial = function(channel, _control, value, _status, group) {
    'use strict';
    // wheel center at 64; <64 rew >64 fwd
    var newVal = value - 64;
    engine.setValue(group, 'jog', newVal * this.bendScale);
};


PioneerDDJ400.jogPlatter = function(channel, _control, value, _status, group) {
    'use strict';
    var deckNum = channel + 1;
    // wheel center at 64; <64 rew >64 fwd
    var newVal = value - 64;

    if (engine.isScratching(deckNum)) {
        engine.scratchTick(deckNum, newVal);
    } else {
        // if not in vinylMode, this just behaves like jogDial
        engine.setValue(group, 'jog', newVal * this.bendScale);
    }
};

PioneerDDJ400.jogTouch = function(channel, _control, value) {
    'use strict';
    var deckNum = channel + 1;

    if (this.vinylMode && value != 0) {
        engine.scratchEnable(deckNum, 720, 33+1/3, this.alpha, this.beta);
    } else {  // value == 0
        engine.scratchDisable(deckNum);
    }
};

PioneerDDJ400.jogSearch = function(_channel, _control, value, _status, group) {
    'use strict';
    // "highspeed" (scaleup value) pitch bend
    var newVal = (value - 64) * this.highspeedScale;
    engine.setValue(group, 'jog', newVal);
};


PioneerDDJ400.shutdown = function() {
    'use strict';
    // reset vumeter
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.deck1.vuMeter, false);
    PioneerDDJ400.toggleLight(LightsPioneerDDJ400.deck2.vuMeter, false);
};
