// Hercules-DJControl-Mix-Ultra-scripts.js
//
// ****************************************************************************
// * Mixxx mapping script file for the Hercules DJControl Mix Ultra.
// * Author: Aayush Kapoor (xeoneux)
// * Features:
// *  - 2-deck mixing with 3-band EQ, Filter, and 14-bit pitch faders
// *  - Jog wheel scratching, pitch bend, and fast seek
// *  - Library browsing (encoder turn) and navigation (push, shifted push/turn)
// *  - Dedicated Load A/B buttons with native double-tap deck cloning
// *  - Center Assistant button (AutoDJ toggle)
// *  - 4 Hardware Pad Modes: Hotcue (1-8), Beatloops (1/2-16 + In/Out), FX,
// *    and Sampler Mode (Samplers 1-8 Deck A, 9-16 Deck B)
// *  - Comprehensive LED feedback
// ****************************************************************************

class DJCMixUltraClass {
    constructor() {
        // User options
        this.scratchScale = 1.0;
        this.bendScale = 1.0;
        this.seekSensitivity = 0.001;

        this.kScratchActionNone = 0;
        this.kScratchActionScratch = 1;
        this.kScratchActionBend = 2;

        this.shiftHeld = {
            1: false,
            2: false,
        };

        this.scratchAction = {
            1: this.kScratchActionNone,
            2: this.kScratchActionNone,
        };

        this.scratchButtonState = true;
    }

    /**
     * Initializes controller upon connection.
     */
    init() {
        if (engine.getValue("[App]", "num_samplers") < 16) {
            engine.setValue("[App]", "num_samplers", 16);
        }

        // Turn off blinking Bluetooth LED when running over USB
        midi.sendShortMsg(0x90, 0x7F, 0x00);

        // Turn on Vinyl mode LED
        midi.sendShortMsg(0x91, 0x03, 0x7F);

        // Initialize AutoDJ LED
        midi.sendShortMsg(0x90, 0x01, engine.getValue("[AutoDJ]", "enabled") ? 0x7F : 0x00);

        // Initialize default effect levels
        for (let unit = 1; unit <= 2; unit++) {
            for (let fx = 1; fx <= 3; fx++) {
                engine.setParameter(`[EffectRack1_EffectUnit${unit}_Effect${fx}]`, "meta", 0.6);
            }
            engine.setParameter(`[EffectRack1_EffectUnit${unit}]`, "mix", 1);
        }

        // Query the controller for initial knob and fader values
        midi.sendShortMsg(0xB0, 0x7F, 0x7F);
    }

    /**
     * Shuts down controller and turns off all LEDs.
     */
    shutdown() {
        midi.sendShortMsg(0xB0, 0x7F, 0x00);
    }

    // ------------------------------------------------------------------------
    // SHIFT & HEADPHONE MONITORING
    // ------------------------------------------------------------------------

    /**
     * Handles Shift button press and release.
     *
     * @param {number} channel - MIDI channel
     * @param {number} _control - MIDI note/control
     * @param {number} value - Velocity
     */
    shiftButton(channel, _control, value) {
        const deck = channel === 1 ? 1 : 2;
        this.shiftHeld[deck] = value > 0;
        if (this.shiftHeld[deck] && engine.isScratching(deck)) {
            engine.scratchDisable(deck);
            this.scratchAction[deck] = this.kScratchActionNone;
        }
        this.updatePflLed(deck);
    }

    /**
     * Updates headphone PFL / cue LED.
     *
     * @param {number} deck - Deck index (1 or 2)
     */
    updatePflLed(deck) {
        const status = deck === 1 ? 0x91 : 0x92;
        if (this.shiftHeld[deck]) {
            const shiftLed = deck === 1
                ? (engine.getValue("[Master]", "headMix") > 0 ? 0x7F : 0x00)
                : (engine.getValue("[Master]", "headSplit") ? 0x7F : 0x00);
            midi.sendShortMsg(status, 0x0C, shiftLed);
        } else {
            const pfl = engine.getValue(`[Channel${deck}]`, "pfl") ? 0x7F : 0x00;
            midi.sendShortMsg(status, 0x0C, pfl);
        }
    }

    /**
     * Cue master button (Shift + PFL on Deck A).
     *
     * @param {number} _channel - Channel
     * @param {number} _control - Control
     * @param {number} value - Value
     */
    cueMaster(_channel, _control, value) {
        if (value === 0) {
            return;
        }
        const masterIsCued = engine.getValue("[Master]", "headMix") > 0;
        engine.setValue("[Master]", "headMix", masterIsCued ? -1 : 1);
        midi.sendShortMsg(0x91, 0x0C, masterIsCued ? 0x00 : 0x7F);
    }

    /**
     * Cue split button (Shift + PFL on Deck B).
     *
     * @param {number} _channel - Channel
     * @param {number} _control - Control
     * @param {number} value - Value
     */
    cueSplit(_channel, _control, value) {
        if (value === 0) {
            return;
        }
        script.toggleControl("[Master]", "headSplit");
        const isSplit = engine.getValue("[Master]", "headSplit");
        midi.sendShortMsg(0x92, 0x0C, isSplit ? 0x7F : 0x00);
    }

    // ------------------------------------------------------------------------
    // VINYL MODE & JOG WHEEL
    // ------------------------------------------------------------------------

    /**
     * Vinyl scratch mode toggle.
     *
     * @param {number} _channel - Channel
     * @param {number} _control - Control
     * @param {number} value - Velocity
     */
    vinylButton(_channel, _control, value) {
        if (value === 0) {
            return;
        }
        this.scratchButtonState = !this.scratchButtonState;
        midi.sendShortMsg(0x91, 0x03, this.scratchButtonState ? 0x7F : 0x00);
    }

    /**
     * Enables scratching on the given deck with standard Hercules parameters.
     *
     * @param {number} deck - Deck index (1 or 2)
     */
    _scratchEnable(deck) {
        const alpha = 1.0 / 8;
        const beta = alpha / 32;
        engine.scratchEnable(deck, 248, 33 + 1 / 3, alpha, beta);
    }

    /**
     * Converts raw 7-bit relative jog encoder value to +1 / -1.
     *
     * @param {number} value - 7-bit MIDI value
     * @returns {number} Direction step (+1 or -1)
     */
    _convertWheelRotation(value) {
        return value < 0x40 ? 1 : -1;
    }

    /**
     * Wheel top touch on normal playback.
     *
     * @param {number} channel - MIDI channel
     * @param {number} _control - Control
     * @param {number} value - Touch velocity
     */
    wheelTouch(channel, _control, value) {
        const deck = channel;
        if (value > 0) {
            if (engine.getValue(`[Channel${deck}]`, "play") !== 1 || this.scratchButtonState) {
                this._scratchEnable(deck);
                this.scratchAction[deck] = this.kScratchActionScratch;
            } else {
                this.scratchAction[deck] = this.kScratchActionBend;
            }
        } else {
            engine.scratchDisable(deck);
            this.scratchAction[deck] = this.kScratchActionNone;
        }
    }

    /**
     * Wheel top touch while holding Shift.
     *
     * @param {number} channel - MIDI channel (4 for Deck A, 5 for Deck B)
     * @param {number} _control - Control
     * @param {number} value - Touch velocity
     */
    wheelTouchShift(channel, _control, value) {
        const deck = channel - 3;
        if (value === 0 && engine.isScratching(deck)) {
            engine.scratchDisable(deck);
            this.scratchAction[deck] = this.kScratchActionNone;
        }
    }

    /**
     * Wheel rotation handler.
     *
     * @param {number} deck - Deck index (1 or 2)
     * @param {number} value - Relative rotation value
     */
    _scratchWheelImpl(deck, value) {
        const interval = this._convertWheelRotation(value);
        const action = this.scratchAction[deck];

        if (action === this.kScratchActionScratch) {
            engine.scratchTick(deck, interval * this.scratchScale);
        } else {
            engine.setValue(`[Channel${deck}]`, "jog", interval * this.bendScale);
        }
    }

    /**
     * Scratch wheel rotation while touching top surface.
     *
     * @param {number} channel - MIDI channel (1 for Deck A, 2 for Deck B)
     * @param {number} _control - Control
     * @param {number} value - Value
     */
    scratchWheel(channel, _control, value) {
        this._scratchWheelImpl(channel, value);
    }

    /**
     * Shifted scratch wheel rotation (seeking).
     *
     * @param {number} channel - MIDI channel (4 for Deck A, 5 for Deck B)
     * @param {number} _control - Control
     * @param {number} value - Value
     */
    scratchWheelShift(channel, _control, value) {
        const deck = channel - 3;
        const group = `[Channel${deck}]`;
        if (engine.getValue(group, "track_loaded")) {
            const interval = this._convertWheelRotation(value);
            const currentPos = engine.getValue(group, "playposition");
            const newPos = Math.max(0.0, Math.min(1.0, currentPos + (interval * this.seekSensitivity)));
            engine.setValue(group, "playposition", newPos);
        }
    }

    /**
     * Pitch bend wheel rotation (outer ring).
     *
     * @param {number} channel - MIDI channel (1 for Deck A, 2 for Deck B)
     * @param {number} _control - Control
     * @param {number} value - Value
     */
    bendWheel(channel, _control, value) {
        const interval = this._convertWheelRotation(value);
        engine.setValue(`[Channel${channel}]`, "jog", interval * this.bendScale);
    }
}

var DJCMixUltra = new DJCMixUltraClass(); // eslint-disable-line no-var, no-unused-vars
