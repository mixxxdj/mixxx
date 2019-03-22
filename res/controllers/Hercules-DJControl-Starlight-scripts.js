// DJControl_Starlight_scripts.js
//
// ****************************************************************************
// * Mixxx mapping script file for the Hercules DJControl Starlight.
// * Author: DJ Phatso, contributions by Kerrick Staley
// * Version 1.2 (March 17 2019)
// * Forum: https://mixxx.org/forums/viewtopic.php?f=7&t=12570
// * Wiki: https://mixxx.org/wiki/doku.php/hercules_dj_control_starlight


// Changes to v1.2
// - Controller knob/slider values are queried on startup, so MIXXX is synced.
// - Fixed vinyl button behavior the first time it's pressed.
// Changes to v1.1
// - Vinyl button now enables/disables scratch function (On by default);
// - FX: SHIFT + Pad = Effect Select
//
// v1.0 : Original release

//TODO: Functions that could be implemented to the script:
//
//* Tweak/map base LED to other functions (if possible)
//* FX:
//   - Potentially pre-select/load effects into deck and set parameters
//* Tweak Jog wheels
//* Optimize JS code.
// ****************************************************************************


function DJCStarlight() {};
var DJCStarlight = {};

DJCStarlight.jogwheelShiftMultiplier = 4;

DJCStarlight.scratchButtonState = true;


// The base LED are mapped to the VU Meter for light show.
DJCStarlight.baseLEDUpdate = function(value, group, control){
    value = (value*127);
    switch(control) {
    case "VuMeterL":
        midi.sendShortMsg(0x91, 0x23, value);
        break;

    case "VuMeterR":
        midi.sendShortMsg(0x92, 0x23, value);
        break;
    }
};


DJCStarlight.init = function() {
    // Turn off base LED default behavior
    midi.sendShortMsg(0x90,0x24,0x00);

    // Vinyl button LED On.
    midi.sendShortMsg(0x91, 0x03, 0x7F);

    // Connect the base LEDs
    engine.connectControl("[Channel1]","VuMeterL","DJCStarlight.baseLEDUpdate");
    engine.connectControl("[Channel2]","VuMeterR","DJCStarlight.baseLEDUpdate");

    //Set effects Levels - Dry/Wet
    engine.setParameter("[EffectRack1_EffectUnit1_Effect1]", "meta", 0.6);
    engine.setParameter("[EffectRack1_EffectUnit1_Effect2]", "meta", 0.6);
    engine.setParameter("[EffectRack1_EffectUnit1_Effect3]", "meta", 0.6);
    engine.setParameter("[EffectRack1_EffectUnit2_Effect1]", "meta", 0.6);
    engine.setParameter("[EffectRack1_EffectUnit2_Effect2]", "meta", 0.6);
    engine.setParameter("[EffectRack1_EffectUnit2_Effect3]", "meta", 0.6);
    engine.setParameter("[EffectRack1_EffectUnit1]", "mix", 1);
    engine.setParameter("[EffectRack1_EffectUnit2]", "mix", 1);

    // Ask the controller to send all current knob/slider values over MIDI, which will update
    // the corresponding GUI controls in MIXXX.
    midi.sendShortMsg(0xB0, 0x7F, 0x7F);
};


// The Vinyl button, used to enable or disable scratching on the jog wheels (The Vinyl button enables both deck).
DJCStarlight.vinylButton = function(channel, control, value, status, group) {
    if (value) {
        if (DJCStarlight.scratchButtonState) {
            DJCStarlight.scratchButtonState = false;
            midi.sendShortMsg(0x91,0x03,0x00);

        } else {
            DJCStarlight.scratchButtonState = true;
            midi.sendShortMsg(0x91,0x03,0x7F);
        }
    }
};


// The touch action on the jog wheel's top surface
DJCStarlight.wheelTouchA = function(channel, control, value, status, group) {
    if (value > 0 && (engine.getValue("[Channel1]", "play") != 1 || DJCStarlight.scratchButtonState)){
        //  Touching the wheel.
        var alpha = 1.0/8;
        var beta = alpha/32;
        engine.scratchEnable(1, 600, 33+1/3, alpha, beta);
    } else {
        // Released the wheel.
        engine.scratchDisable(1);
    }
};


// The touch action on the jog wheel's top surface
DJCStarlight.wheelTouchB = function(channel, control, value, status, group) {
    if (value > 0 && (engine.getValue("[Channel2]", "play") != 1 || DJCStarlight.scratchButtonState)) {
        // Touching the wheel.
        var alpha = 1.0/8;
        var beta = alpha/32;
        engine.scratchEnable(2, 600, 33+1/3, alpha, beta);
    } else {
        // Released the wheel.
        engine.scratchDisable(2);
    }
};


// Scratching on the jog wheel (rotating it while pressing the top surface)
DJCStarlight.scratchWheel = function(channel, control, value, status, group) {
    var deck = channel;
    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }

    if (engine.isScratching(deck)) {
        engine.scratchTick(deck, newValue); // Scratch!
    } else {
        engine.setValue('[Channel' + deck + ']', 'jog', newValue); // Pitch bend
    }
};


// Seeking on the jog wheel (rotating it while pressing the top surface and holding Shift)
DJCStarlight.scratchWheelShift = function(channel, control, value, status, group) {
    var deck = channel - 3;
    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }

    if (engine.isScratching(deck)) {
        // Pressing [Shift] while we're already scratching has no effect, we
        // continue to scratch at normal speed.
        engine.scratchTick(deck, newValue);
    } else {
        // Seek forward
        engine.setValue('[Channel' + deck + ']', 'jog', newValue * DJCStarlight.jogwheelShiftMultiplier); // Pitch bend
    }
};


// Bending on the jog wheel (rotating using the edge)
DJCStarlight.bendWheel = function(channel, control, value, status, group) {
    var deck = channel;
    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }
    engine.setValue('[Channel' + deck + ']', 'jog', newValue); // Pitch bend
};


DJCStarlight.shutdown = function() {
    // Reset base LED
    midi.sendShortMsg(0x90,0x24,0x7F);
};
