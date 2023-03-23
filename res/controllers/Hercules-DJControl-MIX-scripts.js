// eslint-disable-next-line no-var
var DJCMIX = {};
///////////////////////////////////////////////////////////////
//                       USER OPTIONS                        //
///////////////////////////////////////////////////////////////

// How fast scratching is.
DJCMIX.scratchScale = 1.0;

// How much faster seeking (shift+scratch) is than scratching.
DJCMIX.scratchShiftMultiplier = 4;

// How fast bending is.
DJCMIX.bendScale = 1.0;

// DJControl_MIX_scripts.js
//
// ****************************************************************************
// * Mixxx mapping script file for the Hercules DJControl MIX.
// * Author: DJ Phatso and Kerrick Staley
// * Version 1 (Jan 2023)
// * Based on Hercules DJControl Starlight mapping released with Mixxx v2.3.0
// *  -Remapped LOOP and SAMPLER section according to DJControl MIX layout
// *  -Added Master Volume and Headphone Volume
// *  -Removed superfluous LED configuration (not present on DJControl MIX)
// * Forum: https://mixxx.discourse.group/t/hercules-contrl-mix-mapping/26581/
// * Wiki: https://mixxx.org/wiki/doku.php/


DJCMIX.kScratchActionNone = 0;
DJCMIX.kScratchActionScratch = 1;
DJCMIX.kScratchActionSeek = 2;
DJCMIX.kScratchActionBend = 3;


DJCMIX.init = function() {
    DJCMIX.scratchButtonState = true;
    DJCMIX.scratchAction = {
        1: DJCMIX.kScratchActionNone,
        2: DJCMIX.kScratchActionNone
    };

    // Vinyl button LED On.
    midi.sendShortMsg(0x91, 0x03, 0x7F);

    // Set effects Levels - Dry/Wet
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
DJCMIX.vinylButton = function(_channel, _control, value, _status, _group) {
    if (value) {
        if (DJCMIX.scratchButtonState) {
            DJCMIX.scratchButtonState = false;
            midi.sendShortMsg(0x91, 0x03, 0x00);

        } else {
            DJCMIX.scratchButtonState = true;
            midi.sendShortMsg(0x91, 0x03, 0x7F);
        }
    }
};

DJCMIX._scratchEnable = function(deck) {
    const alpha = 1.0/8;
    const beta = alpha/32;
    engine.scratchEnable(deck, 248, 33 + 1/3, alpha, beta);
};

DJCMIX._convertWheelRotation = function(value) {
    // When you rotate the jogwheel, the controller always sends either 0x1
    // (clockwise) or 0x7F (counter clockwise). 0x1 should map to 1, 0x7F
    // should map to -1 (IOW it's 7-bit signed).
    return value < 0x40 ? 1 : -1;
};

// The touch action on the jog wheel's top surface
DJCMIX.wheelTouch = function(channel, _control, value, _status, _group) {
    const deck = channel;
    if (value > 0) {
        //  Touching the wheel.
        if (engine.getValue("[Channel" + deck + "]", "play") !== 1 || DJCMIX.scratchButtonState) {
            DJCMIX._scratchEnable(deck);
            DJCMIX.scratchAction[deck] = DJCMIX.kScratchActionScratch;
        } else {
            DJCMIX.scratchAction[deck] = DJCMIX.kScratchActionBend;
        }
    } else {
        // Released the wheel.
        engine.scratchDisable(deck);
        DJCMIX.scratchAction[deck] = DJCMIX.kScratchActionNone;
    }
};

// The touch action on the jog wheel's top surface while holding shift
DJCMIX.wheelTouchShift = function(channel, _control, value, _status, _group) {
    const deck = channel - 3;
    // We always enable scratching regardless of button state.
    if (value > 0) {
        DJCMIX._scratchEnable(deck);
        DJCMIX.scratchAction[deck] = DJCMIX.kScratchActionSeek;
    } else {
        // Released the wheel.
        engine.scratchDisable(deck);
        DJCMIX.scratchAction[deck] = DJCMIX.kScratchActionNone;
    }
};

// Scratching on the jog wheel (rotating it while pressing the top surface)
DJCMIX._scratchWheelImpl = function(deck, value) {
    const interval = DJCMIX._convertWheelRotation(value);
    const scratchAction = DJCMIX.scratchAction[deck];

    if (scratchAction === DJCMIX.kScratchActionScratch) {
        engine.scratchTick(deck, interval * DJCMIX.scratchScale);
    } else if (scratchAction === DJCMIX.kScratchActionSeek) {
        engine.scratchTick(deck,
            interval
                           * DJCMIX.scratchScale
                           * DJCMIX.scratchShiftMultiplier);
    } else {
        DJCMIX._bendWheelImpl(deck, value);
    }
};

// Scratching on the jog wheel (rotating it while pressing the top surface)
DJCMIX.scratchWheel = function(channel, _control, value, _status, _group) {
    const deck = channel;
    DJCMIX._scratchWheelImpl(deck, value);
};

// Seeking on the jog wheel (rotating it while pressing the top surface and holding Shift)
DJCMIX.scratchWheelShift = function(channel, _control, value, _status, _group) {
    const deck = channel - 3;
    DJCMIX._scratchWheelImpl(deck, value);
};

DJCMIX._bendWheelImpl = function(deck, value) {
    const interval = DJCMIX._convertWheelRotation(value);
    engine.setValue("[Channel" + deck + "]", "jog",
        interval * DJCMIX.bendScale);
};

// Bending on the jog wheel (rotating using the edge)
DJCMIX.bendWheel = function(channel, _control, value, _status, _group) {
    const deck = channel;
    DJCMIX._bendWheelImpl(deck, value);
};

// Cue master button
DJCMIX.cueMaster = function(_channel, _control, value, _status, _group) {
    // This button acts as a toggle. Ignore the release.
    if (value === 0) {
        return;
    }

    let masterIsCued = engine.getValue("[Master]", "headMix") > 0;
    // Toggle state.
    masterIsCued = !masterIsCued;

    const headMixValue = masterIsCued ? 1 : -1;
    engine.setValue("[Master]", "headMix", headMixValue);

    // Set LED (will be overwritten when [Shift] is released)
    const cueMasterLedValue = masterIsCued ? 0x7F : 0x00;
    midi.sendShortMsg(0x91, 0x0C, cueMasterLedValue);
};

// Cue mix button, toggles PFL / master split feature
// We need a special function for this because we want to turn on the LED (but
// we *don't* want to turn on the LED when the user clicks the headSplit button
// in the GUI).
DJCMIX.cueMix = function(_channel, _control, value, _status, _group) {
    // This button acts as a toggle. Ignore the release.
    if (value === 0) {
        return;
    }

    // Toggle state.
    script.toggleControl("[Master]", "headSplit");

    // Set LED (will be overwritten when [Shift] is released)
    const cueMixLedValue =
        engine.getValue("[Master]", "headSplit") ? 0x7F : 0x00;
    midi.sendShortMsg(0x92, 0x0C, cueMixLedValue);
};

DJCMIX.shiftButton = function(_channel, _control, value, _status, _group) {
    if (value >= 0x40) {
        // When Shift is held, light the LEDS to show the status of the alt
        // functions of the cue buttons.
        const cueMasterLedValue =
            engine.getValue("[Master]", "headMix") > 0 ? 0x7F : 0x00;
        midi.sendShortMsg(0x91, 0x0C, cueMasterLedValue);
        const cueMixLedValue =
            engine.getValue("[Master]", "headSplit") ? 0x7F : 0x00;
        midi.sendShortMsg(0x92, 0x0C, cueMixLedValue);
    } else {
        // When Shift is released, go back to the normal LED values.
        const cueChan1LedValue =
            engine.getValue("[Channel1]", "pfl") ? 0x7F : 0x00;
        midi.sendShortMsg(0x91, 0x0C, cueChan1LedValue);
        const cueChan2LedValue =
            engine.getValue("[Channel2]", "pfl") ? 0x7F : 0x00;
        midi.sendShortMsg(0x92, 0x0C, cueChan2LedValue);
    }
};

DJCMIX.shutdown = function() {
    midi.sendShortMsg(0xB0, 0x7F, 0x00);
};
