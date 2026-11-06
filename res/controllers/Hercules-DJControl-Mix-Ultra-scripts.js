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
// *  - Center Assistant / Neural Mix button (Neural Mix EQ & shifted AutoDJ)
// *  - 4 Hardware Pad Modes: Hotcue (1-8), Beatloops (1/2-16 + In/Out), FX,
// *    and Stems / Samplers (Vocals, Drums, Bass, Melody mutes + Samplers 1-4)
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
        this.neuralMixActive = false;
    }

    /**
     * Initializes controller upon connection.
     */
    init() {
        if (engine.getValue("[App]", "num_samplers") < 8) {
            engine.setValue("[App]", "num_samplers", 8);
        }

        // Turn off blinking Bluetooth LED when running over USB
        midi.sendShortMsg(0x90, 0x7F, 0x00);

        // Turn on Vinyl mode LED
        midi.sendShortMsg(0x91, 0x03, 0x7F);

        // Initialize default effect levels
        for (let unit = 1; unit <= 2; unit++) {
            for (let fx = 1; fx <= 3; fx++) {
                engine.setParameter(`[EffectRack1_EffectUnit${unit}_Effect${fx}]`, "meta", 0.6);
            }
            engine.setParameter(`[EffectRack1_EffectUnit${unit}]`, "mix", 1);
        }

        // Connect AutoDJ status listener to update center button LED when Neural Mix is not overriding it
        engine.makeConnection("[AutoDJ]", "enabled", (value) => {
            this.onAutoDJChange(value);
        });
        midi.sendShortMsg(0x90, 0x01, engine.getValue("[AutoDJ]", "enabled") ? 0x7F : 0x00);

        // Connect Stem mute listeners for Decks 1 and 2 to update pad LEDs in Neural mode
        for (let deck = 1; deck <= 2; deck++) {
            for (let stem = 1; stem <= 4; stem++) {
                const stemGroup = `[Channel${deck}_Stem${stem}]`;
                try {
                    engine.makeConnection(stemGroup, "mute", (value) => {
                        this.onStemMuteChange(deck, stem, value);
                    });
                    const currentMute = engine.getValue(stemGroup, "mute");
                    this.onStemMuteChange(deck, stem, currentMute);
                } catch {
                    // Stems not available in this Mixxx configuration
                }
            }
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
    // CENTER ASSISTANT / NEURAL MIX BUTTON
    // ------------------------------------------------------------------------

    /**
     * Center Assistant / Neural Mix button handler.
     * Unshifted: Toggles Neural Mix mode (EQ knobs become stem isolators).
     * Shifted: Toggles AutoDJ enabled state.
     *
     * @param {number} _channel - Channel
     * @param {number} _control - Control
     * @param {number} value - Velocity
     */
    centerButton(_channel, _control, value) {
        if (value === 0) {
            return;
        }

        if (this.shiftHeld[1] || this.shiftHeld[2]) {
            // Shifted: Toggle AutoDJ
            script.toggleControl("[AutoDJ]", "enabled");
        } else {
            // Unshifted: Toggle Neural Mix mode
            this.neuralMixActive = !this.neuralMixActive;
            const ledOn = this.neuralMixActive || engine.getValue("[AutoDJ]", "enabled") > 0;
            midi.sendShortMsg(0x90, 0x01, ledOn ? 0x7F : 0x00);

            try {
                engine.setValue("[Skin]", "show_stem_controls", this.neuralMixActive ? 1 : 0);
            } catch {
                // UI control unavailable in current skin
            }
        }
    }

    /**
     * Listener for AutoDJ enabled state changes.
     *
     * @param {number} value - AutoDJ enabled value
     */
    onAutoDJChange(value) {
        if (!this.neuralMixActive) {
            midi.sendShortMsg(0x90, 0x01, value > 0 ? 0x7F : 0x00);
        }
    }

    // ------------------------------------------------------------------------
    // MIXER: 3-BAND EQ & NEURAL MIX STEM VOLUME SWAP
    // ------------------------------------------------------------------------

    /**
     * Low EQ knob handler.
     * Standard: Controls low equalizer frequency.
     * Neural Mix: Controls Drums & Bass stem volumes (Stems 2 and 3).
     *
     * @param {number} channel - Channel (1 for Deck A, 2 for Deck B)
     * @param {number} _control - Control
     * @param {number} value - 7-bit MIDI value
     */
    eqLow(channel, _control, value) {
        const deck = channel === 1 ? 1 : 2;
        const normalized = value / 127;

        if (this.neuralMixActive) {
            try {
                engine.setParameter(`[Channel${deck}_Stem2]`, "volume", normalized);
                engine.setParameter(`[Channel${deck}_Stem3]`, "volume", normalized);
                return;
            } catch {
                // Stems unavailable; fallback to standard EQ
            }
        }
        engine.setParameter(`[EqualizerRack1_[Channel${deck}]_Effect1]`, "parameter1", normalized);
    }

    /**
     * Mid EQ knob handler.
     * Standard: Controls mid equalizer frequency.
     * Neural Mix: Controls Melody / Instruments stem volume (Stem 4).
     *
     * @param {number} channel - Channel (1 for Deck A, 2 for Deck B)
     * @param {number} _control - Control
     * @param {number} value - 7-bit MIDI value
     */
    eqMid(channel, _control, value) {
        const deck = channel === 1 ? 1 : 2;
        const normalized = value / 127;

        if (this.neuralMixActive) {
            try {
                engine.setParameter(`[Channel${deck}_Stem4]`, "volume", normalized);
                return;
            } catch {
                // Stems unavailable; fallback to standard EQ
            }
        }
        engine.setParameter(`[EqualizerRack1_[Channel${deck}]_Effect1]`, "parameter2", normalized);
    }

    /**
     * High EQ knob handler.
     * Standard: Controls high equalizer frequency.
     * Neural Mix: Controls Vocals stem volume (Stem 1).
     *
     * @param {number} channel - Channel (1 for Deck A, 2 for Deck B)
     * @param {number} _control - Control
     * @param {number} value - 7-bit MIDI value
     */
    eqHigh(channel, _control, value) {
        const deck = channel === 1 ? 1 : 2;
        const normalized = value / 127;

        if (this.neuralMixActive) {
            try {
                engine.setParameter(`[Channel${deck}_Stem1]`, "volume", normalized);
                return;
            } catch {
                // Stems unavailable; fallback to standard EQ
            }
        }
        engine.setParameter(`[EqualizerRack1_[Channel${deck}]_Effect1]`, "parameter3", normalized);
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
    // TRANSPORT: SHIFT + PLAY (BRAKE / SOFT START)
    // ------------------------------------------------------------------------

    /**
     * Shift + Play button handler.
     * Toggles turntable brake and soft-start:
     * - If currently braking or track is paused/stopped: soft starts playback.
     * - If currently soft-starting: cancels soft start.
     * - If currently playing: initiates turntable brake effect.
     *
     * @param {number} channel - MIDI channel (4 for Deck A, 5 for Deck B)
     * @param {number} _control - MIDI note
     * @param {number} value - Velocity
     */
    playShift(channel, _control, value) {
        if (value === 0) {
            return;
        }
        const deck = channel - 3;
        try {
            if (engine.isBrakeActive(deck)) {
                engine.brake(deck, false);
                engine.softStart(deck, true);
            } else if (engine.isSoftStartActive(deck)) {
                engine.softStart(deck, false);
            } else if (engine.getValue(`[Channel${deck}]`, "play")) {
                engine.brake(deck, true);
            } else {
                engine.softStart(deck, true);
            }
        } catch {
            // Fallback if brake/softStart unavailable
            script.toggleControl(`[Channel${deck}]`, "play_stutter");
        }
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

    // ------------------------------------------------------------------------
    // STEMS / NEURAL PAD ACTIONS (MODE 4, PADS 1-4)
    // ------------------------------------------------------------------------

    /**
     * Stem mute/unmute pad press (Pads 1-4 in Mode 4).
     *
     * @param {number} channel - Pad status channel (6 for Deck A, 7 for Deck B)
     * @param {number} control - Note number (0x30 to 0x33)
     * @param {number} value - Velocity
     */
    stemMute(channel, control, value) {
        if (value === 0) {
            return;
        }
        const deck = channel === 6 ? 1 : 2;
        const stem = (control - 0x30) + 1;
        const stemGroup = `[Channel${deck}_Stem${stem}]`;
        try {
            script.toggleControl(stemGroup, "mute");
        } catch {
            // Fallback to EQ kill if stems are not available
            const eqKills = ["filterLowKill", "filterMidKill", "filterHighKill", "reverseroll"];
            script.toggleControl(`[Channel${deck}]`, eqKills[stem - 1]);
        }
    }

    /**
     * Stem solo pad press (Shift + Pads 1-4 in Mode 4).
     *
     * @param {number} channel - Pad status channel (6 for Deck A, 7 for Deck B)
     * @param {number} control - Note number (0x38 to 0x3B)
     * @param {number} value - Velocity
     */
    stemSolo(channel, control, value) {
        if (value === 0) {
            return;
        }
        const deck = channel === 6 ? 1 : 2;
        const targetStem = (control - 0x38) + 1;
        try {
            const isAlreadySoloed = [1, 2, 3, 4].every((s) =>
                engine.getValue(`[Channel${deck}_Stem${s}]`, "mute") === (s === targetStem ? 0 : 1)
            );
            for (let stem = 1; stem <= 4; stem++) {
                const newMute = isAlreadySoloed ? 0 : (stem === targetStem ? 0 : 1);
                engine.setValue(`[Channel${deck}_Stem${stem}]`, "mute", newMute);
            }
        } catch {
            // Fallback if stems not available
        }
    }

    /**
     * Updates pad LED when a stem's mute state changes.
     * Illuminated when stem is active (mute === 0), dark when muted.
     *
     * @param {number} deck - Deck index (1 or 2)
     * @param {number} stem - Stem index (1 to 4)
     * @param {number} value - Mute value (0 or 1)
     */
    onStemMuteChange(deck, stem, value) {
        const padStatus = deck === 1 ? 0x96 : 0x97;
        const padNote = 0x30 + (stem - 1);
        midi.sendShortMsg(padStatus, padNote, value === 0 ? 0x7F : 0x00);
    }
}

var DJCMixUltra = new DJCMixUltraClass(); // eslint-disable-line no-var, no-unused-vars
