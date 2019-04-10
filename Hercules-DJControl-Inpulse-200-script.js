// ****************************************************************************
// * Mixxx mapping script file for the Hercules DJControl Inpulse 200.
// * Author: DJ Phatso
// * Version 1.0 (March 2019)
// * Forum: https://www.mixxx.org/forums/viewtopic.php?f=7&t=12592
// * Wiki: https://mixxx.org/wiki/doku.php/hercules_djcontrol_inpulse_200

// TODO: Functions that could be implemented to the script:
//
// * FX:
//  - See how to preselect efffects for a rack
//  - Tweak scratch effect for Pad 4			  
//



// ****************************************************************************
function DJCi200() {}

var DJCi200 = {};

DJCi200.scratchButtonStateDA = true;
DJCi200.scratchButtonStateDB = true;


DJCi200.init = function() {


    //Turn On Vinyl button LED(one for each deck).
    midi.sendShortMsg(0x91, 0x03, 0x7F);
    midi.sendShortMsg(0x92, 0x03, 0x7F);

    //Turn On Browser button LED
    midi.sendShortMsg(0x90, 0x05, 0x10);

    // Connect the Browser LEDs
    engine.connectControl("[Library]", "MoveFocus");


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


// The Vinyl buttons, used to enable or disable scratching on the jog wheels (One per deck).

DJCi200.vinylButtonDA = function(channel, control, value, status, group) {
    if (value) {
        if (DJCi200.scratchButtonStateDA) {
            DJCi200.scratchButtonStateDA = false;
            midi.sendShortMsg(0x91, 0x03, 0x00);

        } else {
            DJCi200.scratchButtonStateDA = true;
            midi.sendShortMsg(0x91, 0x03, 0x7F);

        }
    }
};

DJCi200.vinylButtonDB = function(channel, control, value, status, group) {
    if (value) {
        if (DJCi200.scratchButtonStateDB) {
            DJCi200.scratchButtonStateDB = false;
            midi.sendShortMsg(0x92, 0x03, 0x00);

        } else {
            DJCi200.scratchButtonStateDB = true;
            midi.sendShortMsg(0x92, 0x03, 0x7F);

        }
    }
};

// The touch action over the jog wheel

DJCi200.wheelTouchDA = function(channel, control, value, status, group) {
    channel = channel + 1;
    if (value > 0 && (engine.getValue("[Channel1]", "play") !== 1 || DJCi200.scratchButtonStateDA)) {
        //  Touching the wheel.
        var alpha = 1.0 / 8;
        var beta = alpha / 32;
        engine.scratchEnable(1, 400, 33 + 1 / 3, alpha, beta);
    } else {
        // Released the wheel.
        engine.scratchDisable(1);
    }
};

DJCi200.wheelTouchDB = function(channel, control, value, status, group) {
    channel = channel + 2;
    if (value > 0 && (engine.getValue("[Channel2]", "play") !== 1 || DJCi200.scratchButtonStateDB)) {
        // Touching the wheel.
        var alpha = 1.0 / 8;
        var beta = alpha / 32;
        engine.scratchEnable(2, 400, 33 + 1 / 3, alpha, beta);
    } else {
        // Released the wheel.
        engine.scratchDisable(2);
    }
};



// Using the top of wheel for scratching (Vinyl button On) and bending (Vinyl button Off)
DJCi200.scratchWheelDA = function(channel, control, value, status, group) {

    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }

    if (engine.isScratching(1)) {
        engine.scratchTick(1, newValue); // Scratch!
    } else {
        engine.setValue('[Channel' + 1 + ']', 'jog', newValue); // Pitch bend
    }
};

DJCi200.scratchWheelDB = function(channel, control, value, status, group) {

    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }

    // In either case, register the movement
    if (engine.isScratching(2)) {
        engine.scratchTick(2, newValue); // Scratch!
    } else {
        engine.setValue('[Channel' + 2 + ']', 'jog', newValue); // Pitch bend
    }
};

// Using the side of wheel for the bending
DJCi200.bendWheelDA = function(channel, control, value, status, group) {

    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }

    {
        engine.setValue('[Channel' + 1 + ']', 'jog', newValue); // Pitch bend
    }
};


// The wheel that actually controls the bending
DJCi200.bendWheelDB = function(channel, control, value, status, group) {

    // A: For a control that centers on 0:
    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }

    {
        engine.setValue('[Channel' + 2 + ']', 'jog', newValue); // Pitch bend
    }
};

// FX Pad #4 setting for scratching
DJCi200.scratchPadDA = function(channel, control, value, status, group) {

    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }

    if (engine.isScratching(1)) {
        engine.scratchTick(1, newValue); // Scratch!
    }

};

DJCi200.scratchPadDB = function(channel, control, value, status, group) {

    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }

    // In either case, register the movement
    if (engine.isScratching(2)) {
        engine.scratchTick(2, newValue); // Scratch!
    }
};



DJCi200.shutdown = function() {

    midi.sendShortMsg(0xB0, 0x7F, 0x00);
};