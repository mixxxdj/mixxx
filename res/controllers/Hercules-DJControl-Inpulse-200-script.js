// DJControl_Inpulse_200_script.js
//
// ***************************************************************************
// * Mixxx mapping script file for the Hercules DJControl Inpulse 200.
// * Author: DJ Phatso, contributions by Kerrick Staley
// * Version 1.2 (March 2020)
// * Forum: https://www.mixxx.org/forums/viewtopic.php?f=7&t=12592
// * Wiki: https://mixxx.org/wiki/doku.php/hercules_djcontrol_inpulse_200
//
// Changes to v1.2
// - Code cleanup.
//
// Changes to v1.1
// - Fix seek-to-start and cue-master behavior.
// - Tweak scratch, seek, and bend behavior.
// - Controller knob/slider values are queried on startup, so MIXXX is synced.
// - Fixed vinyl button behavior the first time it's pressed.
//
// v1.0 : Original forum release
//
// TO DO: Functions that could be implemented to the script:
//
//  FX:
//  - See how to preselect effects for a rack
//
//*************************************************************************

var DJCi200 = {}; //  eslint-disable-line

///////////////////////////////////////////////////////////////
//                       USER OPTIONS                        //
///////////////////////////////////////////////////////////////

// How fast scratching is.
DJCi200.scratchScale = 1.0;

// How much faster seeking (shift+scratch) is than scratching.
DJCi200.scratchShiftMultiplier = 4;

// How fast bending is.
DJCi200.bendScale = 1.0;

// Other scratch related options
DJCi200.kScratchActionNone = 0;
DJCi200.kScratchActionScratch = 1;
DJCi200.kScratchActionSeek = 2;
DJCi200.kScratchActionBend = 3;

DJCi200.init = function() {
    if (engine.getValue("[App]", "num_samplers") < 8) {
        engine.setValue("[App]", "num_samplers", 8);
    }
    // Scratch button state
    DJCi200.scratchButtonState = true;
    // Scratch Action
    DJCi200.scratchAction = {
        1: DJCi200.kScratchActionNone,
        2: DJCi200.kScratchActionNone
    };

    //Turn On Vinyl buttons LED(one for each deck).
    midi.sendShortMsg(0x91, 0x03, 0x7F);
    midi.sendShortMsg(0x92, 0x03, 0x7F);

    //Turn On Browser button LED
    midi.sendShortMsg(0x90, 0x04, 0x05);

    //Softtakeover for Pitch fader
    engine.softTakeover("[Channel1]", "rate", true);
    engine.softTakeover("[Channel2]", "rate", true);
    engine.softTakeoverIgnoreNextValue("[Channel1]", "rate");
    engine.softTakeoverIgnoreNextValue("[Channel2]", "rate");

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

// The Vinyl button, used to enable or disable scratching on the jog wheels (One per deck).
DJCi200.vinylButton = function(_channel, _control, value, status, _group) {
    if (value) {
        if (DJCi200.scratchButtonState) {
            DJCi200.scratchButtonState = false;
            midi.sendShortMsg(status, 0x03, 0x00);
        } else {
            DJCi200.scratchButtonState = true;
            midi.sendShortMsg(status, 0x03, 0x7F);
        }
    }
};

DJCi200._scratchEnable = function(deck) {
    var alpha = 1.0/8;
    var beta = alpha/32;
    engine.scratchEnable(deck, 248, 33 + 1/3, alpha, beta);
};

DJCi200._convertWheelRotation = function(value) {
    // When you rotate the jogwheel, the controller always sends either 0x1
    // (clockwise) or 0x7F (counter clockwise). 0x1 should map to 1, 0x7F
    // should map to -1 (IOW it's 7-bit signed).
    return value < 0x40 ? 1 : -1;
};

// The touch action on the jog wheel's top surface
DJCi200.wheelTouch = function(channel, control, value, _status, _group) {
    var deck = channel;
    if (value > 0) {
        //  Touching the wheel.
        if (engine.getValue("[Channel" + deck + "]", "play") !== 1 || DJCi200.scratchButtonState) {
            DJCi200._scratchEnable(deck);
            DJCi200.scratchAction[deck] = DJCi200.kScratchActionScratch;
        } else {
            DJCi200.scratchAction[deck] = DJCi200.kScratchActionBend;
        }
    } else {
        // Released the wheel.
        engine.scratchDisable(deck);
        DJCi200.scratchAction[deck] = DJCi200.kScratchActionNone;
    }
};

// The touch action on the jog wheel's top surface while holding shift
DJCi200.wheelTouchShift = function(channel, control, value, _status, _group) {
    var deck = channel - 3;
    // We always enable scratching regardless of button state.
    if (value > 0) {
        DJCi200._scratchEnable(deck);
        DJCi200.scratchAction[deck] = DJCi200.kScratchActionSeek;
    } else {
        // Released the wheel.
        engine.scratchDisable(deck);
        DJCi200.scratchAction[deck] = DJCi200.kScratchActionNone;
    }
};

// Scratching on the jog wheel (rotating it while pressing the top surface)
DJCi200.scratchWheel = function(channel, control, value, status, _group) {
    var deck;
    switch (status) {
    case 0xB1:
    case 0xB4:
        deck  = 1;
        break;
    case 0xB2:
    case 0xB5:
        deck  = 2;
        break;
    default:
        return;
    }
    var interval = DJCi200._convertWheelRotation(value);
    var scratchAction = DJCi200.scratchAction[deck];
    if (scratchAction === DJCi200.kScratchActionScratch) {
        engine.scratchTick(deck, interval * DJCi200.scratchScale);
    } else if (scratchAction === DJCi200.kScratchActionSeek) {
        engine.scratchTick(deck,
            interval *  DJCi200.scratchScale *
            DJCi200.scratchShiftMultiplier);
    } else {
        engine.setValue(
            "[Channel" + deck + "]", "jog", interval * DJCi200.bendScale);
    }
};

// Bending on the jog wheel (rotating using the edge)
DJCi200.bendWheel = function(channel, control, value, _status, _group) {
    var interval = DJCi200._convertWheelRotation(value);
    engine.setValue(
        "[Channel" + channel + "]", "jog", interval * DJCi200.bendScale);
};

DJCi200.shutdown = function() {
    midi.sendShortMsg(0xB0, 0x7F, 0x7E);
    midi.sendShortMsg(0x90, 0x04, 0x00);
};
