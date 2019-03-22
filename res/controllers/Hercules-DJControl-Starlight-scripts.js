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
kScratchActionNone = 0;
kScratchActionScratch = 1;
kScratchActionSeek = 2;
kScratchActionBend = 3;
DJCStarlight.scratchAction = {1: kScratchActionNone, 2: kScratchActionNone};


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


DJCStarlight._scratchEnable = function(deck) {
    var alpha = 1.0/8;
    var beta = alpha/32;
    engine.scratchEnable(deck, 248, 33+1/3, alpha, beta);
};


DJCStarlight._convertWheelRotation = function(value) {
    // When you rotate the jogwheel, the controller always sends either 0x1
    // (clockwise) or 0x7F (counter clockwise). 0x1 should map to 1, 0x7F
    // should map to -1 (IOW it's 7-bit signed).
    return value < 0x40 ? 1 : -1;
}


// The touch action on the jog wheel's top surface
DJCStarlight.wheelTouch = function(channel, control, value, status, group) {
    var deck = channel;
    if (value > 0) {
        //  Touching the wheel.
        if (engine.getValue("[Channel" + deck + "]", "play") != 1 || DJCStarlight.scratchButtonState) {
            DJCStarlight._scratchEnable(deck);
            DJCStarlight.scratchAction[deck] = kScratchActionScratch;
        } else {
            DJCStarlight.scratchAction[deck] = kScratchActionBend;
        }
    } else {
        // Released the wheel.
        engine.scratchDisable(deck);
        DJCStarlight.scratchAction[deck] = kScratchActionNone;
    }
};


// The touch action on the jog wheel's top surface while holding shift
DJCStarlight.wheelTouchShift = function(channel, control, value, status, group) {
    var deck = channel - 3;
    // We always enable scratching regardless of button state.
    if (value > 0) {
        DJCStarlight._scratchEnable(deck);
        DJCStarlight.scratchAction[deck] = kScratchActionSeek;
    } else {
        // Released the wheel.
        engine.scratchDisable(deck);
        DJCStarlight.scratchAction[deck] = kScratchActionNone;
    }
};


// Scratching on the jog wheel (rotating it while pressing the top surface)
DJCStarlight._scratchWheelImpl = function(deck, value) {
    var interval = DJCStarlight._convertWheelRotation(value);
    var scratchAction = DJCStarlight.scratchAction[deck];

    if (scratchAction == kScratchActionScratch) {
        engine.scratchTick(deck, interval); // Scratch
    } else if (scratchAction == kScratchActionSeek) {
        engine.scratchTick(deck, interval * DJCStarlight.jogwheelShiftMultiplier); // Seek
    } else {
        engine.setValue('[Channel' + deck + ']', 'jog', interval); // Pitch bend
    }
};

// Scratching on the jog wheel (rotating it while pressing the top surface)
DJCStarlight.scratchWheel = function(channel, control, value, status, group) {
    var deck = channel;
    DJCStarlight._scratchWheelImpl(deck, value);
};


// Seeking on the jog wheel (rotating it while pressing the top surface and holding Shift)
DJCStarlight.scratchWheelShift = function(channel, control, value, status, group) {
    var deck = channel - 3;
    DJCStarlight._scratchWheelImpl(deck, value);
};


// Bending on the jog wheel (rotating using the edge)
DJCStarlight.bendWheel = function(channel, control, value, status, group) {
    var deck = channel;
    var interval = DJCStarlight._convertWheelRotation(value);

    engine.setValue('[Channel' + deck + ']', 'jog', interval); // Pitch bend
};


DJCStarlight.shutdown = function() {
    // Reset base LED
    midi.sendShortMsg(0x90,0x24,0x7F);
};
