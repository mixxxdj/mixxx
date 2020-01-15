////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
//////////////////////////////////////////////////////////////////////// 
var DJCi200 = {};
///////////////////////////////////////////////////////////////
//                       USER OPTIONS                        //
///////////////////////////////////////////////////////////////



// How fast scratching is.
DJCi200.scratchScale = 1.0;

// How much faster seeking (shift+scratch) is than scratching.
DJCi200.scratchShiftMultiplier = 4;

// How fast bending is.
DJCi200.bendScale = 1.0;

// DJControl_Inpulse_200_script.js
//
// ***************************************************************************
// * Mixxx mapping script file for the Hercules DJControl Inpulse 200.
// * Author: DJ Phatso, contributions by Kerrick Staley
// *
// * Version 1.2 (December 2019)
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
// TODO: Functions that could be implemented to the script:
//
//  FX:		
//  - See how to preselect effects for a rack
//  - Tweak scratch effect for Pad 4	
//
//************************************************************************
//
// We have to disable the no-unused-vars check because we have many MIDI
// callbacks that receive a fixed list of arguments, but we usually don't use
// most of these arguments. Eslint seems to make it relatively difficult to
// disable this check on a case-by-case basis, so we disable it for the whole
// file.
// See this GitHub issue for more context:
// https://github.com/eslint/eslint/issues/1939
// *eslint-disable no-unused-vars*/

DJCi200.kScratchActionNone = 0;
DJCi200.kScratchActionScratch = 1;
DJCi200.kScratchActionSeek = 2;
DJCi200.kScratchActionBend = 3;

//function DJCi200() {}


DJCi200.init = function () {

    DJCi200.scratchButtonState = true;
    DJCi200.scratchAction = {
        1: DJCi200.kScratchActionNone,
        2: DJCi200.kScratchActionNone
    };

    //Turn On Vinyl button LED(one for each deck).
    midi.sendShortMsg(0x91, 0x03, 0x7F);
    midi.sendShortMsg(0x92, 0x03, 0x7F);

    //Turn On Browser button LED
    midi.sendShortMsg(0x90, 0x05, 0x10);

    // Connect the Browser LEDs
    engine.getValue("[Library]", "MoveFocus");
    engine.getValue("[Master]", "maximize_library");

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

DJCi200.vinylButtonDA = function (channel, control, value, status, group) {
    if (value) {
        if (DJCi200.scratchButtonState) {
            DJCi200.scratchButtonState = false;
            midi.sendShortMsg(0x91, 0x03, 0x00);

        } else {
            DJCi200.scratchButtonState = true;
            midi.sendShortMsg(0x91, 0x03, 0x7F);

        }
    }
};

DJCi200.vinylButtonDB = function (channel, control, value, status, group) {
    if (value) {
        if (DJCi200.scratchButtonState) {
            DJCi200.scratchButtonState = false;
            midi.sendShortMsg(0x92, 0x03, 0x00);

        } else {
            DJCi200.scratchButtonState = true;
            midi.sendShortMsg(0x92, 0x03, 0x7F);

        }
    }
};

DJCi200._scratchEnable = function (deck) {

    var alpha = 1.0 / 8;
    var beta = alpha / 32;
    engine.scratchEnable(deck, 248, 33 + 1 / 3, alpha, beta);

};



DJCi200._convertWheelRotation = function (value) {
    // When you rotate the jogwheel, the controller always sends either 0x1
    // (clockwise) or 0x7F (counter clockwise). 0x1 should map to 1, 0x7F
    // should map to -1 (IOW it's 7-bit signed).
    return value < 0x40 ? 1 : -1;
};



// The touch action on the jog wheel's top surface
DJCi200.wheelTouch = function (channel, control, value, status, group) {
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
DJCi200.wheelTouchShift = function (channel, control, value, status, group) {
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
DJCi200._scratchWheelImpl = function (deck, value) {
    var interval = DJCi200._convertWheelRotation(value);
    var scratchAction = DJCi200.scratchAction[deck];



    if (scratchAction === DJCi200.kScratchActionScratch) {
        engine.scratchTick(deck, interval * DJCi200.scratchScale);
    } else if (scratchAction === DJCi200.kScratchActionSeek) {
        engine.scratchTick(deck,
            interval *  DJCi200.scratchScale *
            DJCi200.scratchShiftMultiplier);
    } else {
        DJCi200._bendWheelImpl(deck, value);

    }
};


// Scratching on the jog wheel (rotating it while pressing the top surface)
DJCi200.scratchWheel = function (channel, control, value, status, group) {
    var deck = channel;
    DJCi200._scratchWheelImpl(deck, value);
};

// Seeking on the jog wheel (rotating it while pressing the top surface and holding Shift)
DJCi200.scratchWheelShift = function (channel, control, value, status, group) {
    var deck = channel - 3;
    DJCi200._scratchWheelImpl(deck, value);

};


DJCi200._bendWheelImpl = function (deck, value) {
    var interval = DJCi200._convertWheelRotation(value);
    engine.setValue('[Channel' + deck + ']', 'jog',
        interval * DJCi200.bendScale);
};

// Bending on the jog wheel (rotating using the edge)
DJCi200.bendWheel = function (channel, control, value, status, group) {
    var deck = channel;
    DJCi200._bendWheelImpl(deck, value);
};

DJCi200.scratchPad = function (channel, control, value, status, group) {
    var deck = channel;
    DJCi200._scratchWheelImpl(deck, value);
};


DJCi200.shutdown = function () {

    midi.sendShortMsg(0xB0, 0x7F, 0x00);
};