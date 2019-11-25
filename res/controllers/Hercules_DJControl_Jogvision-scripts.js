////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
//////////////////////////////////////////////////////////////////////// 
// ****************************************************************************
// * Mixxx mapping script file for the Hercules DJControl Jogvision.
// * Author: DJ Phatso, contributions by Kerrick Staley and David TV
// * Version 1.3 (November 2019)
// * Version 1.2 (November 2019)
// * Version 1.1 (March 2019)
// * Forum: https://www.mixxx.org/forums/viewtopic.php?f=7&t=12580
// * Wiki: https://www.mixxx.org/wiki/doku.php/hercules_dj_control_jogvision
//
// Changes to v1.3
// - Enabled the creation of beatloop (beatloop_activate) by using SHIFT+LoopON
// - Changed "LOOP SIZE" to adjust the beatjump_size (you still can change size with surrounding buttons)
// - Added SHIFT+"LOOP SIZE" to move loop left or right by N beats (beatjump)
//
// Changes to v1.2
// - Enabled Jogwheel Outer LED rotation
// - Enabled Beat LEDs
// 
// Changes to v1.1
// - Controller knob/slider values are queried on startup, so MIXXX is synced.
// - Fixed vinyl button behavior the first time it's pressed.
// 
// v1.0 : Original release
//
// ****************************************************************************
function DJCJV() {}
var DJCJV = {};

var on = 0x7F;
var off = 0x00;
var alpha = 1.0 / 8;
var beta = alpha / 16;

DJCJV.scratchButtonState = true;
DJCJV.beatAccumDA = 1;
DJCJV.beatAccumDB = 1;
DJCJV.rotationA = 0x00;
DJCJV.rotationB = 0x00;
DJCJV.ledRotationTimer = 0;

function dec2hex(dec) {
    return '0x' + (dec + 0x10000).toString(16).substr(-4).toUpperCase();
}

// Initialization
DJCJV.init = function() {

    //Set all LED states to off
    midi.sendShortMsg(0xB0, 0x7F, off);
    midi.sendShortMsg(0xB1, 0x7F, off);

    // Set Vinyl button LED On.
    midi.sendShortMsg(0x90, 0x45, on);
    DJCJV.scratchButtonState = true;
    midi.sendShortMsg(0x90, 0x46, on);

    // Set Headphone CUE/MIX LED state
    if (engine.getValue("[Master]", "headMix") > 0.5) {
        midi.sendShortMsg(0x90, 0x4C, on);
    } // headset "Mix" button LED
    else {
        midi.sendShortMsg(0x90, 0x4D, on);
    } // headset "Cue" button LED

    //Enable Soft takeover
    engine.softTakeover("[Master]", "crossfader", true);
    engine.softTakeover("[QuickEffectRack1_[Channel1]]", "super1", true);
    engine.softTakeover("[QuickEffectRack1_[Channel2]]", "super1", true);

    //Set effects Levels - Dry/Wet - Filters
    engine.setParameter("[EffectRack1_EffectUnit1_Effect1]", "meta", 0.6);
    engine.setParameter("[EffectRack1_EffectUnit1_Effect2]", "meta", 0.6);
    engine.setParameter("[EffectRack1_EffectUnit1_Effect3]", "meta", 0.6);
    engine.setParameter("[EffectRack1_EffectUnit2_Effect1]", "meta", 0.6);
    engine.setParameter("[EffectRack1_EffectUnit2_Effect2]", "meta", 0.6);
    engine.setParameter("[EffectRack1_EffectUnit2_Effect3]", "meta", 0.6);
    engine.setParameter("[EffectRack1_EffectUnit1]", "mix", 0.6);
    engine.setParameter("[EffectRack1_EffectUnit2]", "mix", 0.6);
    engine.setParameter("[QuickEffectRack1_[Channel1]]", "super1", 0.5);
    engine.setParameter("[QuickEffectRack1_[Channel2]]", "super1", 0.5);

    // Connect the VUMeters and Jog Inner LED
    engine.connectControl("[Channel1]", "VuMeter", "DJCJV.vuMeterUpdateDA");
    engine.connectControl("[Channel2]", "VuMeter", "DJCJV.vuMeterUpdateDB");
    engine.connectControl("[Channel1]", "playposition", "DJCJV.wheelInnerUpdateDA");
    engine.connectControl("[Channel2]", "playposition", "DJCJV.wheelInnerUpdateDB");

    // Enable wheels outer leds rotation by timer (when channel is playing)s
    DJCJV.ledRotationTimer = engine.beginTimer(20, function() {
        if (engine.getValue("[Channel1]", "play") == 1) {
            midi.sendShortMsg(0xB0, 0x60, dec2hex(DJCJV.rotationA));
            DJCJV.rotationA = DJCJV.rotationA > 127 ? 1 : DJCJV.rotationA + 1;
        }
        if (engine.getValue("[Channel2]", "play") == 1) {
            midi.sendShortMsg(0xB1, 0x60, dec2hex(DJCJV.rotationB));
            DJCJV.rotationB = DJCJV.rotationB > 127 ? 1 : DJCJV.rotationB + 1;
        }
    });

    // Connect the the beat_active with beat leds
    engine.connectControl("[Channel1]", "beat_active", "DJCJV.beatActiveDA");
    engine.connectControl("[Channel2]", "beat_active", "DJCJV.beatActiveDB");
    engine.connectControl("[Channel1]", "stop", "DJCJV.beatActiveClearDA");
    engine.connectControl("[Channel2]", "stop", "DJCJV.beatActiveClearDB");

    // Ask the controller to send all current knob/slider values over MIDI, which will update the corresponding GUI controls in MIXXX.
    midi.sendShortMsg(0xB0, 0x7F, on);

};

// Finalization
DJCJV.shutdown = function() {
    if (DJCJV.ledRotationTimer) {
        engine.stopTimer(DJCJV.ledRotationTimer);
    }
    midi.sendShortMsg(0xB0, 0x7F, off);
    midi.sendShortMsg(0xB1, 0x7F, off);
    midi.sendShortMsg(0x90, 0x7F, off);
    midi.sendShortMsg(0x91, 0x7F, off);
};

// Beat led display
DJCJV.beatActiveDA = function(value, group, control) {
    if (value == 1) {
        return;
    }
    midi.sendShortMsg(0x90, 0x3A, DJCJV.beatAccumDA == 1 ? on : off);
    midi.sendShortMsg(0x90, 0x3B, DJCJV.beatAccumDA == 2 ? on : off);
    midi.sendShortMsg(0x90, 0x3C, DJCJV.beatAccumDA == 3 ? on : off);
    midi.sendShortMsg(0x90, 0x3D, DJCJV.beatAccumDA == 4 ? on : off);
    DJCJV.beatAccumDA = DJCJV.beatAccumDA >= 4 ? 1 : DJCJV.beatAccumDA + 1;
};
DJCJV.beatActiveDB = function(value, group, control) {
    if (value == 1) {
        return;
    }
    midi.sendShortMsg(0x91, 0x3A, DJCJV.beatAccumDB == 1 ? on : off);
    midi.sendShortMsg(0x91, 0x3B, DJCJV.beatAccumDB == 2 ? on : off);
    midi.sendShortMsg(0x91, 0x3C, DJCJV.beatAccumDB == 3 ? on : off);
    midi.sendShortMsg(0x91, 0x3D, DJCJV.beatAccumDB == 4 ? on : off);
    DJCJV.beatAccumDB = DJCJV.beatAccumDB >= 4 ? 1 : DJCJV.beatAccumDB + 1;
};
DJCJV.beatActiveClearDA = function(value, group, control) {
    midi.sendShortMsg(0x90, 0x3A, off);
    midi.sendShortMsg(0x90, 0x3B, off);
    midi.sendShortMsg(0x90, 0x3C, off);
    midi.sendShortMsg(0x90, 0x3D, off);
    DJCJV.beatAccumDA = 1;
};
DJCJV.beatActiveClearDB = function(value, group, control) {
    midi.sendShortMsg(0x91, 0x3A, off);
    midi.sendShortMsg(0x91, 0x3B, off);
    midi.sendShortMsg(0x91, 0x3C, off);
    midi.sendShortMsg(0x91, 0x3D, off);
    DJCJV.beatAccumDB = 1;
};

//Jogwheels inner LED display - Play position
DJCJV.wheelInnerUpdateDA = function(value, group, control) {
    switch (control) {
        case "[Channel1]", "playposition":
            midi.sendShortMsg(0xB0, 0x61, value * 127);
            break;
    }

};
DJCJV.wheelInnerUpdateDB = function(value, group, control) {
    switch (control) {
        case "[Channel2]", "playposition":
            midi.sendShortMsg(0xB1, 0x61, value * 127);
            break;
    }

};

//Vu Meter
DJCJV.vuMeterUpdateDA = function(value, group, control) {
    switch (control) {
        case "[Channel1]", "VuMeter":
            midi.sendShortMsg(0x90, 0x44, value * 6);
            break;
    }

};
DJCJV.vuMeterUpdateDB = function(value, group, control) {
    switch (control) {
        case "[Channel2]", "VuMeter":
            midi.sendShortMsg(0x91, 0x44, value * 6);
            break;
    }

};

// Headphone CUE/MIX buttons status
DJCJV.headCue = function(midino, control, value, status, group) {
    if (engine.getValue(group, "headMix") == 0) {
        engine.setValue(group, "headMix", -1.0);
        midi.sendShortMsg(0x90, 0x4D, on);
        midi.sendShortMsg(0x90, 0x4C, off);
    }
};
DJCJV.headMix = function(midino, control, value, status, group) {
    if (engine.getValue(group, "headMix") != 1) {
        engine.setValue(group, "headMix", 0);
        midi.sendShortMsg(0x90, 0x4D, off);
        midi.sendShortMsg(0x90, 0x4C, on);
    }
};

DJCJV.FilterDA = function(channel, control, value, status, group) {
    engine.setValue(group, "super1", 0.5 - (value) / 255);
};
DJCJV.FilterDB = function(channel, control, value, status, group) {
    engine.setValue(group, "super1", 0.5 - (value) / 255);
};

// SHIFT + Loop ON creates a loop at given point
DJCJV.beatloop_activate = function(channel, control, value, status, group) {
    var currentValue = engine.getValue(group, "beatloop_activate");
    engine.setValue(group, "beatloop_activate", value == 0 ? 0 : 1);
};
DJCJV.beatjump_move = function(channel, control, value, status, group) {
    if (value > 64) {
        engine.setValue(group, "beatjump_backward", 1);
        engine.setValue(group, "beatjump_backward", 0);
    } else {
        engine.setValue(group, "beatjump_forward", 1);
        engine.setValue(group, "beatjump_forward", 0);
    }
};
DJCJV.beatjump_size = function(channel, control, value, status, group) {
    var currentValue = engine.getValue(group, "beatjump_size");
    if (value > 64) {
        engine.setValue(group, "beatjump_size", currentValue /= 2);
    } else {
        engine.setValue(group, "beatjump_size", currentValue *= 2);
    }
};
DJCJV.loopsize = function(channel, control, value, status, group) {
    var currentValue = engine.getValue(group, "beatloop_size");
    if (value > 64) {
        engine.setValue(group, "beatloop_size", currentValue /= 2);
    } else {
        engine.setValue(group, "beatloop_size", currentValue *= 2);
    }
};

// The Vinyl button, used to enable or disable scratching on the jog wheels.
DJCJV.vinylButton = function(channel, control, value, status, group) {
    if (!value) {
        return;
    }

    if (DJCJV.scratchButtonState) {
        DJCJV.scratchButtonState = false;
        midi.sendShortMsg(0x90, 0x46, off);
    } else {
        DJCJV.scratchButtonState = true;
        midi.sendShortMsg(0x90, 0x46, on);
    }
};

// The pressure action over the jog wheel
DJCJV.wheelTouchA = function(channel, control, value, status, group) {
    if (value > 0 && (engine.getValue("[Channel1]", "play") != 1 || DJCJV.scratchButtonState)) {
        engine.scratchEnable(1, 400, 33 + 1 / 3, alpha, beta); //  Touching the wheel
    } else {
        engine.scratchDisable(1); // Released the wheel
    }
};
DJCJV.wheelTouchB = function(channel, control, value, status, group) {
    if (value > 0 && (engine.getValue("[Channel2]", "play") != 1 || DJCJV.scratchButtonState)) {
        engine.scratchEnable(2, 400, 33 + 1 / 3, alpha, beta); // Touching the wheel.
    } else {
        engine.scratchDisable(2); // Released the wheel.
    }
};

// Using the top of wheel for scratching (Vinyl button On) and bending (Vinyl button Off)
// In either case, register the movement
DJCJV.scratchWheelA = function(channel, control, value, status, group) {
    if (engine.isScratching(1)) {
        engine.scratchTick(1, (value >= 64) ? value - 128 : value); // Scratch!
    } else {
        engine.setValue('[Channel' + 1 + ']', 'jog', (value >= 64) ? value - 128 : value); // Pitch bend
    }
};
DJCJV.scratchWheelB = function(channel, control, value, status, group) {
    if (engine.isScratching(2)) {
        engine.scratchTick(2, (value >= 64) ? value - 128 : value); // Scratch!
    } else {
        engine.setValue('[Channel' + 2 + ']', 'jog', (value >= 64) ? value - 128 : value); // Pitch bend
    }
};

// Using the side of wheel for the bending
DJCJV.bendWheelA = function(channel, control, value, status, group) {
    engine.setValue('[Channel' + 1 + ']', 'jog', (value >= 64) ? value - 128 : value); // Pitch bend
};
DJCJV.bendWheelB = function(channel, control, value, status, group) {
    engine.setValue('[Channel' + 2 + ']', 'jog', (value >= 64) ? value - 128 : value); // Pitch bend
};
