// Hercules-DJControl-MIX-scripts.js
//
// ****************************************************************************
// * Mixxx mapping script file for the Hercules DJControl MIX.
// * Author: DJ Phatso and Kerrick Staley
// * Based on Hercules DJControl Starlight mapping released with Mixxx v2.3.0
// *  -Remapped LOOP and SAMPLER section according to DJControl MIX layout
// *  -Added Master Volume and Headphone Volume
// *  -Removed superfluous LED configuration (not present on DJControl MIX)
// * Forum: https://mixxx.discourse.group/t/hercules-contrl-mix-mapping/26581/

class DJCMixClass {
    constructor() {
        ///////////////////////////////////////////////////////////////
        //                       USER OPTIONS                        //
        ///////////////////////////////////////////////////////////////

        // How fast scratching is.
        this.scratchScale = 1.0;

        // How much faster seeking (shift+scratch) is than scratching.
        this.scratchShiftMultiplier = 4;

        // How fast bending is.
        this.bendScale = 1.0;


        this.kScratchActionNone = 0;
        this.kScratchActionScratch = 1;
        this.kScratchActionSeek = 2;
        this.kScratchActionBend = 3;
    }

    init() {
        if (engine.getValue("[App]", "num_samplers") < 8) {
            engine.setValue("[App]", "num_samplers", 8);
        }

        this.scratchButtonState = true;
        this.scratchAction = {
            1: this.kScratchActionNone,
            2: this.kScratchActionNone
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
    }

    // The Vinyl button, used to enable or disable scratching on the jog wheels (The Vinyl button enables both deck).
    vinylButton(_channel, _control, value, _status, _group) {
        if (value) {
            if (this.scratchButtonState) {
                this.scratchButtonState = false;
                midi.sendShortMsg(0x91, 0x03, 0x00);

            } else {
                this.scratchButtonState = true;
                midi.sendShortMsg(0x91, 0x03, 0x7F);
            }
        }
    }

    _scratchEnable(deck) {
        const alpha = 1.0 / 8;
        const beta = alpha / 32;
        engine.scratchEnable(deck, 248, 33 + 1 / 3, alpha, beta);
    }

    _convertWheelRotation(value) {
        // When you rotate the jogwheel, the controller always sends either 0x1
        // (clockwise) or 0x7F (counter clockwise). 0x1 should map to 1, 0x7F
        // should map to -1 (IOW it's 7-bit signed).
        return value < 0x40 ? 1 : -1;
    }

    // The touch action on the jog wheel's top surface
    wheelTouch(channel, _control, value, _status, _group) {
        const deck = channel;
        if (value > 0) {
            //  Touching the wheel.
            if (engine.getValue("[Channel" + deck + "]", "play") !== 1 || this.scratchButtonState) {
                this._scratchEnable(deck);
                this.scratchAction[deck] = this.kScratchActionScratch;
            } else {
                this.scratchAction[deck] = this.kScratchActionBend;
            }
        } else {
            // Released the wheel.
            engine.scratchDisable(deck);
            this.scratchAction[deck] = this.kScratchActionNone;
        }
    }

    // The touch action on the jog wheel's top surface while holding shift
    wheelTouchShift(channel, _control, value, _status, _group) {
        const deck = channel - 3;
        // We always enable scratching regardless of button state.
        if (value > 0) {
            this._scratchEnable(deck);
            this.scratchAction[deck] = this.kScratchActionSeek;
        } else {
            // Released the wheel.
            engine.scratchDisable(deck);
            this.scratchAction[deck] = this.kScratchActionNone;
        }
    }

    // Scratching on the jog wheel (rotating it while pressing the top surface)
    _scratchWheelImpl(deck, value) {
        const interval = this._convertWheelRotation(value);
        const scratchAction = this.scratchAction[deck];

        if (scratchAction === this.kScratchActionScratch) {
            engine.scratchTick(deck, interval * this.scratchScale);
        } else if (scratchAction === this.kScratchActionSeek) {
            engine.scratchTick(deck,
                interval
                * this.scratchScale
                * this.scratchShiftMultiplier);
        } else {
            this._bendWheelImpl(deck, value);
        }
    }

    // Scratching on the jog wheel (rotating it while pressing the top surface)
    scratchWheel(channel, _control, value, _status, _group) {
        const deck = channel;
        this._scratchWheelImpl(deck, value);
    }

    // Seeking on the jog wheel (rotating it while pressing the top surface and holding Shift)
    scratchWheelShift(channel, _control, value, _status, _group) {
        const deck = channel - 3;
        this._scratchWheelImpl(deck, value);
    }

    _bendWheelImpl(deck, value) {
        const interval = this._convertWheelRotation(value);
        engine.setValue("[Channel" + deck + "]", "jog",
            interval * this.bendScale);
    }

    // Bending on the jog wheel (rotating using the edge)
    bendWheel(channel, _control, value, _status, _group) {
        const deck = channel;
        this._bendWheelImpl(deck, value);
    }

    // Cue master button
    cueMaster(_channel, _control, value, _status, _group) {
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
    }

    // Cue mix button, toggles PFL / master split feature
    // We need a special function for this because we want to turn on the LED (but
    // we *don't* want to turn on the LED when the user clicks the headSplit button
    // in the GUI).
    cueMix(_channel, _control, value, _status, _group) {
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
    }

    shiftButton(_channel, _control, value, _status, _group) {
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
    }
    // All LED Turned off
    shutdown() {
        midi.sendShortMsg(0xB0, 0x7F, 0x00);
    }
}

var DJCMIX = new DJCMixClass;  // eslint-disable-line no-var, no-unused-vars
