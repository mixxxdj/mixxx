/* Pioneer DDJ-REV1-script.js
 * ****************************************************************************
 * * Mixxx mapping script file for the Pioneer DDJ-REV1.
 * * Authors: LauraRozier
 * * Reviewers:
 * * Manual: https://manual.mixxx.org/
 * ****************************************************************************
 *
 * Info from the MIDI docs:
 *   MIDI channels are defined as shown below.
 *     0x9*: Note
 *     0xB*: Control Change (CC)
 *
 *  Implemented (as per manufacturer's manual):
 *      * Mixer Section (Faders, EQ, Filter, Trim, Cue, Headphone level, Master level, VU Meters)
 *      * Browser section (Library scroll, Library fast scroll, LOAD)
 *      * Deck section (Tempo, Jogwheels, Scratching, Bending, searching, free-spin side while holding SHIFT,
 *                      Auto Loop toggle, 1/2x loop length, 2x loop length, Reloop IN, Reloop OUT, Deck select)
 *      * Beat Sync
 *      * Hot Cue Mode Pads
 *      * Auto Loop Mode Pads
 *      * Sampler Mode Pads
 *      * Beat Jump Mode Pads
 *      * Rolling Auto Loop Mode Pads
 *      * FX1-2 effect 1-3 toggles, FX bank toggle
 *
 *  Custom (Mixxx specific mappings):
 *      * Master cue toggles Headphone Split Cue mode
 *      * Browser push (Preview track, rotate is now preview position control)
 *      * LOAD + SHIFT (Clear deck track)
 *      * FX1-2 effect 1-3 change next effect while holding SHIFT
 *      * FX1-2 effect 1-3 META via Level/Depth knob rotate
 *
 *  Not implemented (after discussion and trial attempts):
 *      * Tracking Scratch Mode Pads
 *      * Trans Mode Pads
 *      * Scratch Bank Mode Pads
 */

var PioneerDDJREV1 = {};

/* Useful constant values */
PioneerDDJREV1.zero = 0;
PioneerDDJREV1.channelCount = 4;
PioneerDDJREV1.padCount = 8;
PioneerDDJREV1.fxBankCount = 2;
PioneerDDJREV1.fxEffectCount = 3;
PioneerDDJREV1.samplerCount = 16;
PioneerDDJREV1.vuAdjust = 125;

/* Timer IDs */
PioneerDDJREV1.timers = {};

/* Used for tempo slider */
PioneerDDJREV1.highResMSB = {
    "[Channel1]": {},
    "[Channel2]": {},
    "[Channel3]": {},
    "[Channel4]": {},
};

/* Used to track the SHIFT button state */
PioneerDDJREV1.shiftState = false;

/* Used to track the track preview seek state */
PioneerDDJREV1.previewSeekEnabled = false;

/* Jog wheel constants */
PioneerDDJREV1.vinylMode = true;
PioneerDDJREV1.alpha = 1.0 / 8;
PioneerDDJREV1.beta = PioneerDDJREV1.alpha / 32;

/* Multiplier for fast seek through track using SHIFT+JOGWHEEL */
PioneerDDJREV1.fastSeekScale = 150;
PioneerDDJREV1.bendScale = 0.8;

PioneerDDJREV1.tempoRanges = [0.06, 0.10, 0.16, 0.25];

/* Midi channels */
PioneerDDJREV1.midiChan = {
    note: 0x90,
    ctrl: 0xB0,
    special: 0x9F,

    subChan: {
        // n=0x..
        ch1: 0x00,
        ch2: 0x01,
        ch3: 0x02,
        ch4: 0x03,
        // m=0x..
        fx1: 0x04,
        fx2: 0x05,
        browser: 0x06,
        master: 0x06,
        // p=0x..
        deck1: 0x07,
        deck1Shift: 0x08,
        deck2: 0x09,
        deck2Shift: 0x0A,
        deck3: 0x0B,
        deck3Shift: 0x0C,
        deck4: 0x0D,
        deck4Shift: 0x0E,
    },
};

/* Status light addresses */
PioneerDDJREV1.lights = {
    status: {
        off: 0x00,
        on: 0x7F
    },

    decks: {
        autoLoopBase: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.ch1,
            data1: 0x14
        },
        autoLoopShiftBase: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.ch1,
            data1: 0x50
        },
    },

    mixer: {
        vuMeterBase: {
            status: PioneerDDJREV1.midiChan.ctrl + PioneerDDJREV1.midiChan.subChan.ch1,
            data1: 0x02
        },
    },

    fx: {
        fx1: {
            slot1: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx1,
                data1: 0x70
            },
            slot1Shift: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx1,
                data1: 0x63
            },

            slot2: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx1,
                data1: 0x71
            },
            slot2Shift: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx1,
                data1: 0x06
            },

            slot3: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx1,
                data1: 0x72
            },
            slot3Shift: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx1,
                data1: 0x07
            },
        },
        fx2: {
            slot1: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx2,
                data1: 0x70
            },
            slot1Shift: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx2,
                data1: 0x63
            },

            slot2: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx2,
                data1: 0x71
            },
            slot2Shift: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx2,
                data1: 0x06
            },

            slot3: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx2,
                data1: 0x72
            },
            slot3Shift: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx2,
                data1: 0x07
            },
        },
    },
};

/*
 * Utility methods
 */

/**
 * Change a status indicator's state
 * @param {number} channel The indicator's channel (status)
 * @param {number} address The indicator's address (data1)
 * @param {boolean} active Indicator on when true; off otherwise
 */
PioneerDDJREV1.setIndicatorState = function(channel, address, active) {
    midi.sendShortMsg(channel, address, active ? PioneerDDJREV1.lights.status.on : PioneerDDJREV1.lights.status.off);
};

/**
 * Change a status indicator's state
 * @param {number} channel The VU meter's associated channel
 * @param {number} value   The VU meter's current value
 */
PioneerDDJREV1.setVuMeterState = function(channel, value) {
    midi.sendShortMsg(
        PioneerDDJREV1.lights.mixer.vuMeterBase.status + channel,
        PioneerDDJREV1.lights.mixer.vuMeterBase.data1,
        value * PioneerDDJREV1.vuAdjust
    );
};

/**
 * Convert a control ID to a PAD number
 * @param {number} control The control ID
 * @returns {number} The PAD number
 */
PioneerDDJREV1.getPadNumber = function(control) {
    // Pads are all 0xCP (C = Control channel; P = Pad ID)
    return (control & 0x0F) + 0x01;
};

/**
 * The controller mapping script INIT method
 */
PioneerDDJREV1.init = function() {
    engine.setValue("[EffectRack1_EffectUnit1]", "show_focus", 1);

    for (let i = PioneerDDJREV1.zero; i < PioneerDDJREV1.channelCount; i++) {
        const channel = `[Channel${i + 1}]`;

        // Connect to VU meter and zero it out
        engine.makeUnbufferedConnection(channel, "vu_meter", PioneerDDJREV1.vuMeterUpdate);
        PioneerDDJREV1.setVuMeterState(PioneerDDJREV1.midiChan.subChan.ch1 + i, PioneerDDJREV1.lights.status.off);

        // Connect to channel rate
        engine.softTakeover(channel, "rate", true);

        // Connect to channel track loaded
        engine.makeConnection(channel, "track_loaded", PioneerDDJREV1.trackLoadedLED);
        // Play the "track loaded" animation on both decks at startup
        midi.sendShortMsg(PioneerDDJREV1.midiChan.special, i, PioneerDDJREV1.lights.status.on);

        // Connect to channel loop
        engine.makeConnection(channel, "loop_enabled", PioneerDDJREV1.loopToggle);
    }

    for (let i = PioneerDDJREV1.zero; i < PioneerDDJREV1.fxBankCount; i++) {
        for (let j = PioneerDDJREV1.zero; j < PioneerDDJREV1.fxEffectCount; j++) {
            const effect = `[EffectRack1_EffectUnit${i + 1}_Effect${j + 1}]`;
            engine.softTakeover(effect, "meta", true);
            engine.makeConnection(effect, "enabled", PioneerDDJREV1.toggleFxLight);
        }
    }

    if (engine.getValue("[App]", "num_samplers") < PioneerDDJREV1.samplerCount) {
        engine.setValue("[App]", "num_samplers", PioneerDDJREV1.samplerCount);
    }

    for (let i = PioneerDDJREV1.zero; i < PioneerDDJREV1.samplerCount; i++) {
        engine.makeConnection(`[Sampler${i + 1}]`, "play", PioneerDDJREV1.samplerPlayOutputCallbackFunction);
    }

    // Query the controller for current control positions on startup
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x03, 0x01, 0xF7], 7); // Serato "gimme your status" command
};

/**
 * The controller mapping script SHUTDOWN method
 */
PioneerDDJREV1.shutdown = function() {
    // Zero out VU meter and reloop lights
    for (let i = PioneerDDJREV1.zero; i < PioneerDDJREV1.channelCount; i++) {
        PioneerDDJREV1.setVuMeterState(PioneerDDJREV1.midiChan.subChan.ch1 + i, PioneerDDJREV1.lights.status.off);
        PioneerDDJREV1.setReloopLight(PioneerDDJREV1.midiChan.subChan.ch1 + i, PioneerDDJREV1.lights.status.off);
    }

    for (let mode = PioneerDDJREV1.zero; mode < 0x80; mode += 0x10) {
        // We can skip these, as we never changed them
        if ([0x20, 0x60, 0x70].indexOf(mode) > -1) {
            continue;
        }

        for (let padId = PioneerDDJREV1.zero; padId < PioneerDDJREV1.padCount; padId++) {
            const data1Addr = mode + padId;

            for (let deckId = PioneerDDJREV1.midiChan.subChan.deck1; deckId <= PioneerDDJREV1.midiChan.subChan.deck4Shift; deckId++) {
                PioneerDDJREV1.setIndicatorState(PioneerDDJREV1.midiChan.note + deckId, data1Addr, false);
            }
        }
    }

    // Cleanup FX lights
    for (let i = PioneerDDJREV1.zero; i < PioneerDDJREV1.fxBankCount; i++) {
        for (let j = PioneerDDJREV1.zero; j < PioneerDDJREV1.fxEffectCount; j++) {
            PioneerDDJREV1.toggleFxLight(false, `[EffectRack1_EffectUnit${i + 1}_Effect${j + 1}]`);
        }
    }
};

/**
 * Channel VU Meter lights
 * @param {number} value The current VU meter value
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.vuMeterUpdate = function(value, group) {
    switch (group) {
    case "[Channel1]":
        PioneerDDJREV1.setVuMeterState(PioneerDDJREV1.midiChan.subChan.ch1, value);
        break;

    case "[Channel2]":
        PioneerDDJREV1.setVuMeterState(PioneerDDJREV1.midiChan.subChan.ch2, value);
        break;

    case "[Channel3]":
        PioneerDDJREV1.setVuMeterState(PioneerDDJREV1.midiChan.subChan.ch3, value);
        break;

    case "[Channel4]":
        PioneerDDJREV1.setVuMeterState(PioneerDDJREV1.midiChan.subChan.ch4, value);
        break;
    }
};

/**
 * Toggles the reloop button lights
 * @param {boolean} value The current reloop button state
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.loopToggle = function(value, group) {
    switch (group) {
    case "[Channel1]":
        PioneerDDJREV1.setReloopLight(PioneerDDJREV1.midiChan.subChan.ch1, value);
        break;

    case "[Channel2]":
        PioneerDDJREV1.setReloopLight(PioneerDDJREV1.midiChan.subChan.ch2, value);
        break;

    case "[Channel3]":
        PioneerDDJREV1.setReloopLight(PioneerDDJREV1.midiChan.subChan.ch3, value);
        break;

    case "[Channel4]":
        PioneerDDJREV1.setReloopLight(PioneerDDJREV1.midiChan.subChan.ch4, value);
        break;
    }
};

/**
 * Sets the state of the reloop lights
 * @param {number} channel The channel to change the light state for
 * @param {boolean} value The requested light state
 */
PioneerDDJREV1.setReloopLight = function(channel, value) {
    const state = value ? PioneerDDJREV1.lights.status.on : PioneerDDJREV1.lights.status.off;
    midi.sendShortMsg(
        PioneerDDJREV1.lights.decks.autoLoopBase.status + channel,
        PioneerDDJREV1.lights.decks.autoLoopBase.data1,
        state);
    midi.sendShortMsg(
        PioneerDDJREV1.lights.decks.autoLoopShiftBase.status + channel,
        PioneerDDJREV1.lights.decks.autoLoopShiftBase.data1,
        state);
};

/**
 * Triggers the track loaded indicator light effect
 * @param {boolean} value The track loaded state
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.trackLoadedLED = function(value, group) {
    PioneerDDJREV1.setIndicatorState(
        PioneerDDJREV1.midiChan.special,
        PioneerDDJREV1.midiChan.subChan.ch1 + (group.match(script.channelRegEx)[1] - 1),
        value > PioneerDDJREV1.zero);
};

/**
 * Sets the FX button light state to the button state in the DAW
 * @param {boolean} value The FX unit effect button state
 * @param {string} group The FX unit effect group tag
 */
PioneerDDJREV1.toggleFxLight = function(value, group) {
    switch (group) {
    case "[EffectRack1_EffectUnit1_Effect1]":
        PioneerDDJREV1.setIndicatorState(
            PioneerDDJREV1.lights.fx.fx1.slot1.status,
            PioneerDDJREV1.lights.fx.fx1.slot1.data1,
            value);
        break;

    case "[EffectRack1_EffectUnit1_Effect2]":
        PioneerDDJREV1.setIndicatorState(
            PioneerDDJREV1.lights.fx.fx1.slot2.status,
            PioneerDDJREV1.lights.fx.fx1.slot2.data1,
            value);
        break;

    case "[EffectRack1_EffectUnit1_Effect3]":
        PioneerDDJREV1.setIndicatorState(
            PioneerDDJREV1.lights.fx.fx1.slot3.status,
            PioneerDDJREV1.lights.fx.fx1.slot3.data1,
            value);
        break;

    case "[EffectRack1_EffectUnit2_Effect1]":
        PioneerDDJREV1.setIndicatorState(
            PioneerDDJREV1.lights.fx.fx2.slot1.status,
            PioneerDDJREV1.lights.fx.fx2.slot1.data1,
            value);
        break;

    case "[EffectRack1_EffectUnit2_Effect2]":
        PioneerDDJREV1.setIndicatorState(
            PioneerDDJREV1.lights.fx.fx2.slot2.status,
            PioneerDDJREV1.lights.fx.fx2.slot2.data1,
            value);
        break;

    case "[EffectRack1_EffectUnit2_Effect3]":
        PioneerDDJREV1.setIndicatorState(
            PioneerDDJREV1.lights.fx.fx2.slot3.status,
            PioneerDDJREV1.lights.fx.fx2.slot3.data1,
            value);
        break;
    }
};

/**
 * Changes the sample pad light mode
 * @param {boolean} value The play state
 * @param {string} group The sample pad group tag
 * @returns {void}
 */
PioneerDDJREV1.samplerPlayOutputCallbackFunction = function(value, group) {
    if (value === PioneerDDJREV1.zero) { // ignore release
        return;
    }

    const curPad = group.match(script.samplerRegEx)[1],
        data1Addr = 0x30 + ((curPad > PioneerDDJREV1.padCount ? curPad - PioneerDDJREV1.padCount : curPad) - 1);

    if (curPad <= PioneerDDJREV1.padCount) {
        PioneerDDJREV1.startSamplerBlink(
            PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.deck1,
            data1Addr,
            group);
        PioneerDDJREV1.startSamplerBlink(
            PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.deck3,
            data1Addr,
            group);
    } else {
        PioneerDDJREV1.startSamplerBlink(
            PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.deck2,
            data1Addr,
            group);
        PioneerDDJREV1.startSamplerBlink(
            PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.deck4,
            data1Addr,
            group);
    }
};

/**
 * Start the blinking loop for a sample pad
 * @param {number} channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} control The midi control/note number
 * @param {string} group The sample pad group tag
 */
PioneerDDJREV1.startSamplerBlink = function(channel, control, group) {
    let val = PioneerDDJREV1.lights.status.on;

    PioneerDDJREV1.stopSamplerBlink(channel, control);
    PioneerDDJREV1.timers[channel][control] = engine.beginTimer(250, () => {
        val = PioneerDDJREV1.lights.status.on - val;

        // blink the appropriate pad
        midi.sendShortMsg(channel, control, val);
        // also blink the pad while SHIFT is pressed
        midi.sendShortMsg(channel + 1, control, val);

        if (engine.getValue(group, "play") < 1) {
            // kill timer
            PioneerDDJREV1.stopSamplerBlink(channel, control);
            // set the pad LED to ON
            midi.sendShortMsg(channel, control, PioneerDDJREV1.lights.status.on);
            // set the pad LED to ON while SHIFT is pressed
            midi.sendShortMsg(channel + 1, control, PioneerDDJREV1.lights.status.on);
        }
    });
};

/**
 * Stop the blinking loop for a sample pad
 * @param {number} channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} control The midi control/note number
 */
PioneerDDJREV1.stopSamplerBlink = function(channel, control) {
    PioneerDDJREV1.timers[channel] = PioneerDDJREV1.timers[channel] || {};

    if (PioneerDDJREV1.timers[channel][control] !== undefined) {
        engine.stopTimer(PioneerDDJREV1.timers[channel][control]);
        PioneerDDJREV1.timers[channel][control] = undefined;
    }
};

/**
 * Shift button press handler
 * @param {number} _channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} _control The midi control/note number
 * @param {number} value The midi control value
 */
PioneerDDJREV1.shiftPressed = function(_channel, _control, value) {
    PioneerDDJREV1.shiftState = (value === PioneerDDJREV1.lights.status.on);
};

/**
 * Browser knob rotate handler
 * @param {number} _channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} _control The midi control/note number
 * @param {number} value The midi control value
 */
PioneerDDJREV1.browseRotate = function(_channel, _control, value) {
    if (value === PioneerDDJREV1.zero) { // Ignore zero state
        return;
    }

    /* Rotary encoder states:
     * - Idle : 0x00        =  0 (SKIP)
     * - CW   : 0x01 ~ 0x1E = +1
     * - CCW  : 0x7F ~ 0x62 = -1
     */
    const rotateVal = value === 0x7F ? -1 : 1;

    if (PioneerDDJREV1.previewSeekEnabled  && engine.getValue("[PreviewDeck1]", "play")) {
        engine.setValue(
            "[PreviewDeck1]",
            "playposition",
            Math.max(0, engine.getValue("[PreviewDeck1]", "playposition") + (0.05 * rotateVal)));
    } else if (!PioneerDDJREV1.previewSeekEnabled) {
        engine.setValue(
            "[Playlist]",
            "SelectTrackKnob",
            rotateVal * (PioneerDDJREV1.shiftState ? 10 : 1)
        );
    }
};

/**
 * Browser knob press handler
 * @param {number} _channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} _control The midi control/note number
 * @param {number} value The midi control value
 */
PioneerDDJREV1.browsePressed = function(_channel, _control, value) {
    if (value === PioneerDDJREV1.zero) { // ignore release
        return;
    }

    const isPlaying = engine.getValue("[PreviewDeck1]", "play");
    PioneerDDJREV1.previewSeekEnabled = !isPlaying;
    script.triggerControl("[PreviewDeck1]", isPlaying ? "stop" : "LoadSelectedTrackAndPlay");
};

/**
 * Jog wheel turn handler
 * @param {number} channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} _control The midi control/note number
 * @param {number} value The midi control value
 * @param {number} _status The midi status byte
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.jogTurn = function(channel, _control, value, _status, group) {
    const deckNum = channel + 1,
        newVal = value - 64; // wheel center at 64; <64 rew >64 fwd

    if (engine.isScratching(deckNum)) {
        engine.scratchTick(deckNum, newVal);
    } else { // fallback
        engine.setValue(group, "jog", newVal * PioneerDDJREV1.bendScale);
    }
};

/**
 * Jog wheel platter & SHIFT turn handler
 * @param {number} _channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} _control The midi control/note number
 * @param {number} value The midi control value
 * @param {number} _status The midi status byte
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.jogSearch = function(_channel, _control, value, _status, group) {
    engine.setValue(group, "jog", (value - 64) * PioneerDDJREV1.fastSeekScale);
};

/**
 * Jog wheel platter touch handler
 * @param {number} channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} _control The midi control/note number
 * @param {number} value The midi control value
 */
PioneerDDJREV1.jogTouch = function(channel, _control, value) {
    const deckNum = channel + 1;

    if (value !== PioneerDDJREV1.zero && PioneerDDJREV1.vinylMode) {
        engine.scratchEnable(deckNum, 720, 33 + 1 / 3, PioneerDDJREV1.alpha, PioneerDDJREV1.beta);
    } else {
        engine.scratchDisable(deckNum);
    }
};

/*
 * Tempo sliders
 *
 * The tempo option in Mixxx's deck preferences determine whether down/up
 * increases/decreases the rate. Therefore it must be inverted here so that the
 * UI and the control sliders always move in the same direction.
 */
/**
 * The TEMPO slider's most-significant bit handler
 * @param {number} _channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} _control The midi control/note number
 * @param {number} value The midi control value
 * @param {number} _status The midi status byte
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.tempoSliderMSB = function(_channel, _control, value, _status, group) {
    PioneerDDJREV1.highResMSB[group].tempoSlider = value;
};

/**
 * The TEMPO slider's least-significant bit handler
 * @param {number} _channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} _control The midi control/note number
 * @param {number} value The midi control value
 * @param {number} _status The midi status byte
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.tempoSliderLSB = function(_channel, _control, value, _status, group) {
    const fullValue = (PioneerDDJREV1.highResMSB[group].tempoSlider << 7) + value;
    engine.setValue(group, "rate", 1 - (fullValue / 0x2000));
};

/**
 * BEAT SYNC button press handler
 * @param {number} _channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} _control The midi control/note number
 * @param {number} value The midi control value
 * @param {number} _status The midi status byte
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.syncPressed = function(_channel, _control, value, _status, group) {
    if (engine.getValue(group, "sync_enabled") && value > 0) {
        engine.setValue(group, "sync_enabled", 0);
    } else {
        engine.setValue(group, "beatsync", value);
    }
};
/**
 * BEAT SYNC & SHIFT button press handler
 * @param {number} _channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} _control The midi control/note number
 * @param {number} value The midi control value
 * @param {number} _status The midi status byte
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.syncShiftPressed = function(_channel, _control, value, _status, group) {
    if (value > PioneerDDJREV1.zero) {
        engine.setValue(group, "sync_enabled", 1);
    }
};

/**
 * DECK SELECT button press handler
 *
 * Note that the controller sends different signals for a short press and a long
 * press of the same button.
 * @param {number} _channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} _control The midi control/note number
 * @param {number} value The midi control value
 * @param {number} _status The midi status byte
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.cycleTempoRange = function(_channel, _control, value, _status, group) {
    if (value === PioneerDDJREV1.zero) { // ignore release
        return;
    }

    const currRange = engine.getValue(group, "rateRange");

    for (let i = PioneerDDJREV1.zero; i < PioneerDDJREV1.tempoRanges.length; i++) {
        if (currRange === PioneerDDJREV1.tempoRanges[i]) {
            const idx = (i + 1) % PioneerDDJREV1.tempoRanges.length;
            engine.setValue(group, "rateRange", PioneerDDJREV1.tempoRanges[idx]);
            break;
        }
    }
};

/**
 * Hotcue PAD button press handler
 * @param {number} _channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} control The midi control/note number
 * @param {number} value The midi control value
 * @param {number} _status The midi status byte
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.hotcuePadPress = function(_channel, control, value, _status, group) {
    const padNum = PioneerDDJREV1.getPadNumber(control);
    engine.setValue(group, `hotcue_${padNum}_activate`, value);
};
/**
 * Hotcue PAD & SHIFT button press handler
 * @param {number} _channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} control The midi control/note number
 * @param {number} value The midi control value
 * @param {number} _status The midi status byte
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.hotcuePadShiftPress = function(_channel, control, value, _status, group) {
    const padNum = PioneerDDJREV1.getPadNumber(control);
    engine.setValue(group, `hotcue_${padNum}_clear`, value);
};

/**
 * Sampler PAD button press handler
 * @param {number} _channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} _control The midi control/note number
 * @param {number} value The midi control value
 * @param {number} _status The midi status byte
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.samplerPadPress = function(_channel, _control, value, _status, group) {
    if (engine.getValue(group, "track_loaded")) {
        engine.setValue(group, "cue_gotoandplay", value);
    } else {
        engine.setValue(group, "LoadSelectedTrack", value);
    }
};
/**
 * Sampler PAD & SHIFT button press handler
 * @param {number} _channel The midi channel (0x00 = Channel 1..0x0F = Channel 16,)
 * @param {number} _control The midi control/note number
 * @param {number} value The midi control value
 * @param {number} _status The midi status byte
 * @param {string} group The channel group tag
 */
PioneerDDJREV1.samplerPadShiftPress = function(_channel, _control, value, _status, group) {
    if (engine.getValue(group, "play")) {
        engine.setValue(group, "cue_gotoandstop", value);
    } else if (engine.getValue(group, "track_loaded")) {
        engine.setValue(group, "eject", value);
    }
};
