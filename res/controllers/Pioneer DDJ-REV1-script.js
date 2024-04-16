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
 *      * VU Meters
 *
 *  Custom (Mixxx specific mappings):
 *      * 
 *
 *  Not implemented (after discussion and trial attempts):
 *      * 
 */

var PioneerDDJREV1 = {};

/* Useful constant values */
PioneerDDJREV1.zero         = 0;
PioneerDDJREV1.channelCount = 4;
PioneerDDJREV1.padCount     = 8;
PioneerDDJREV1.samplerCount = 16;
PioneerDDJREV1.vuAdjust     = 150;

/* Timer IDs */
PioneerDDJREV1.timers = {};

/* Used for tempo slider */
PioneerDDJREV1.highResMSB = {
    "[Channel1]": {},
    "[Channel2]": {},
    "[Channel3]": {},
    "[Channel4]": {}
};

/* Used to track the SHIFT button state */
PioneerDDJREV1.shiftState = false;

/* Used to track the track preview seek state */
PioneerDDJREV1.previewSeekEnabled = false;

/* Jog wheel constants */
PioneerDDJREV1.vinylMode = true;
PioneerDDJREV1.alpha     = 1.0 / 8;
PioneerDDJREV1.beta      = PioneerDDJREV1.alpha / 32;

/* Multiplier for fast seek through track using SHIFT+JOGWHEEL */
PioneerDDJREV1.fastSeekScale = 150;
PioneerDDJREV1.bendScale     = 0.8;

PioneerDDJREV1.tempoRanges = [0.06, 0.10, 0.16, 0.25];

/* Jog wheel loop adjust */
PioneerDDJREV1.loopAdjustIn       = [false, false, false, false];
PioneerDDJREV1.loopAdjustOut      = [false, false, false, false];
PioneerDDJREV1.loopAdjustMultiply = 50;

/* Midi channels */
PioneerDDJREV1.midiChan = {
    note:    0x90,
    ctrl:    0xB0,
    special: 0x9F,

    subChan: {
        // n=0x..
        ch1:        0x00,
        ch2:        0x01,
        ch3:        0x02,
        ch4:        0x03,
        // m=0x..
        fx1:        0x04,
        fx2:        0x05,
        browser:    0x06,
        master:     0x06,
        // p=0x..
        deck1:      0x07,
        deck1Shift: 0x08,
        deck2:      0x09,
        deck2Shift: 0x0A,
        deck3:      0x0B,
        deck3Shift: 0x0C,
        deck4:      0x0D,
        deck4Shift: 0x0E
    }
};

/* Status light addresses */
PioneerDDJREV1.lights = {
    status: {
        off: 0x00,
        on:  0x7F
    },

    decks: {
        playPauseBase: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.ch1,
            data1:  0x0B
        },
        playPauseShiftBase: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.ch1,
            data1:  0x47
        },

        cueBase: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.ch1,
            data1:  0x0C
        },
        cueShiftBase: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.ch1,
            data1:  0x48
        },

        deckSelectLeft: {
            status: PioneerDDJREV1.midiChan.note + 0x02,
            data1:  0x72
        },
        deckSelectRight: {
            status: PioneerDDJREV1.midiChan.note + 0x03,
            data1:  0x72
        },
        deckSelectShiftBase: { // Unused
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.ch1,
            data1:  0x17
        },

        autoLoopBase: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.ch1,
            data1:  0x14
        },
        autoLoopShiftBase: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.ch1,
            data1:  0x50
        },

        syncBase: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.ch1,
            data1:  0x58
        },
        syncShiftBase: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.ch1,
            data1:  0x5C
        }
    },

    mixer: {
        vuMeterBase: {
            status: PioneerDDJREV1.midiChan.ctrl + PioneerDDJREV1.midiChan.subChan.ch1,
            data1:  0x02
        },

        cueBase: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.ch1,
            data1:  0x54
        },
        cueShiftBase: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.ch1,
            data1:  0x68
        },

        cueMaster: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.master,
            data1:  0x63
        },
        cueShiftMaster: {
            status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.master,
            data1:  0x62
        },
    },

    fx: {
        fx1: {
            slot1: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx1,
                data1:  0x70
            },
            slot1Shift: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx1,
                data1:  0x63
            },

            slot2: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx1,
                data1:  0x71
            },
            slot2Shift: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx1,
                data1:  0x06
            },

            slot3: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx1,
                data1:  0x72
            },
            slot3Shift: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx1,
                data1:  0x07
            }
        },
        fx2: {
            slot1: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx2,
                data1:  0x70
            },
            slot1Shift: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx2,
                data1:  0x63
            },

            slot2: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx2,
                data1:  0x71
            },
            slot2Shift: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx2,
                data1:  0x06
            },

            slot3: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx2,
                data1:  0x72
            },
            slot3Shift: {
                status: PioneerDDJREV1.midiChan.note + PioneerDDJREV1.midiChan.subChan.fx2,
                data1:  0x07
            }
        }
    }
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
    midi.sendShortMsg(
        channel,
        address,
        active ? this.lights.status.on : this.lights.status.off
    );
};

/**
 * Change a status indicator's state
 * @param {number} channel The VU meter's associated channel
 * @param {number} value   The VU meter's current value
 */
PioneerDDJREV1.setVuMeterState = function(channel, value) {
    midi.sendShortMsg(
        this.lights.mixer.vuMeterBase.status + channel,
        this.lights.mixer.vuMeterBase.data1,
        value * this.vuAdjust
    );
};

/**
 * Convert a control ID to a PAD number
 * @param {number} control The control ID
 * @returns The PAD number
 */
PioneerDDJREV1.getPadNumber = function(control) {
    // Pads are all 0xCP (C = Control channel; P = Pad ID)
    return (control & 0x0F) + 0x01;
};

/*
 * Init
 */
PioneerDDJREV1.init = function() {
    engine.setValue("[EffectRack1_EffectUnit1]", "show_focus", 1);

    for (let i = this.zero; i < this.channelCount; i++) {
        const channel = `[Channel${i + 1}]`;

        // Connect to VU meter and zero it out
        engine.makeUnbufferedConnection(channel, "vu_meter", this.vuMeterUpdate);
        this.setVuMeterState(this.midiChan.subChan.ch1 + i, this.lights.status.off);

        // Connect to channel rate
        engine.softTakeover(channel, "rate", true);

        // Connect to channel track loaded
        engine.makeConnection(channel, "track_loaded", this.trackLoadedLED);
        // Play the "track loaded" animation on both decks at startup
        midi.sendShortMsg(this.midiChan.special, i, this.lights.status.on);

        // Connect to channel loop
        engine.makeConnection(channel, "loop_enabled", this.loopToggle);
    }

    for (let i = this.zero; i < 2; i++) {
        //const effectUnit = `[EffectRack1_EffectUnit${i + 1}]`;
        //engine.softTakeover(effectUnit, "mix", true);
        //engine.makeConnection(effectUnit, "focused_effect", this.toggleFxLight);

        for (let j = this.zero; j < 3; j++) {
            const effect = `[EffectRack1_EffectUnit${i + 1}_Effect${j + 1}]`;

            engine.softTakeover(effect, "meta", true);
            engine.makeConnection(effect, "enabled", this.toggleFxLight);
        }
    }

    if (engine.getValue("[App]", "num_samplers") < this.samplerCount) {
        engine.setValue("[App]", "num_samplers", this.samplerCount);
    }

    for (let i = this.zero; i < this.samplerCount; i++) {
        engine.makeConnection(`[Sampler${i + 1}]`, "play", this.samplerPlayOutputCallbackFunction);
    }

    // Query the controller for current control positions on startup
    midi.sendSysexMsg([0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x02, 0x06, 0x00, 0x03, 0x01, 0xF7], 12);
};

/*
 * Shutdown
 */
PioneerDDJREV1.shutdown = function() {
    // Zero out VU meter lights
    for (let i = this.zero; i < this.channelCount; i++) {
        this.setVuMeterState(this.midiChan.subChan.ch1 + i, this.lights.status.off);
        this.setReloopLight(this.midiChan.subChan.ch1 + i, this.lights.status.off);
    }

    for (let mode = this.zero; mode < 0x80; mode += 0x10) {
        // We can skip these, as we never changed them
        if ([0x20, 0x60, 0x70].indexOf(mode) > -1) { continue; }

        for (let padId = this.zero; padId < this.padCount; padId++) {
            const data1Addr = mode + padId;

            for (let deckId = this.midiChan.subChan.deck1; deckId <= this.midiChan.subChan.deck4Shift; deckId++) {
                this.setIndicatorState(
                    this.midiChan.note + deckId,
                    data1Addr,
                    this.lights.status.off
                );
            }
        }
    }

    // Cleanup FX lights
    this.setIndicatorState(
        this.lights.fx.fx1.slot1.status,
        this.lights.fx.fx1.slot1.data1,
        this.lights.status.off
    );
    this.setIndicatorState(
        this.lights.fx.fx1.slot1Shift.status,
        this.lights.fx.fx1.slot1Shift.data1,
        this.lights.status.off
    );
    this.setIndicatorState(
        this.lights.fx.fx1.slot2.status,
        this.lights.fx.fx1.slot2.data1,
        this.lights.status.off
    );
    this.setIndicatorState(
        this.lights.fx.fx1.slot2Shift.status,
        this.lights.fx.fx1.slot2Shift.data1,
        this.lights.status.off
    );
    this.setIndicatorState(
        this.lights.fx.fx1.slot3.status,
        this.lights.fx.fx1.slot3.data1,
        this.lights.status.off
    );
    this.setIndicatorState(
        this.lights.fx.fx1.slot3Shift.status,
        this.lights.fx.fx1.slot3Shift.data1,
        this.lights.status.off
    );

    this.setIndicatorState(
        this.lights.fx.fx2.slot1.status,
        this.lights.fx.fx2.slot1.data1,
        this.lights.status.off
    );
    this.setIndicatorState(
        this.lights.fx.fx2.slot1Shift.status,
        this.lights.fx.fx2.slot1Shift.data1,
        this.lights.status.off
    );
    this.setIndicatorState(
        this.lights.fx.fx2.slot2.status,
        this.lights.fx.fx2.slot2.data1,
        this.lights.status.off
    );
    this.setIndicatorState(
        this.lights.fx.fx2.slot2Shift.status,
        this.lights.fx.fx2.slot2Shift.data1,
        this.lights.status.off
    );
    this.setIndicatorState(
        this.lights.fx.fx2.slot3.status,
        this.lights.fx.fx2.slot3.data1,
        this.lights.status.off
    );
    this.setIndicatorState(
        this.lights.fx.fx2.slot3Shift.status,
        this.lights.fx.fx2.slot3Shift.data1,
        this.lights.status.off
    );
};

/*
 * Channel VU Meter lights
 */
PioneerDDJREV1.vuMeterUpdate = function(value, group) {
    switch (group) {
        case "[Channel1]":
            this.setVuMeterState(this.midiChan.subChan.ch1, value);
            break;

        case "[Channel2]":
            this.setVuMeterState(this.midiChan.subChan.ch2, value);
            break;

        case "[Channel3]":
            this.setVuMeterState(this.midiChan.subChan.ch3, value);
            break;

        case "[Channel4]":
            this.setVuMeterState(this.midiChan.subChan.ch4, value);
            break;
    }
};

PioneerDDJREV1.loopToggle = function(value, group) {
    const state = value ? this.lights.status.on : this.lights.status.off;

    switch (group) {
        case "[Channel1]":
            this.setReloopLight(this.midiChan.subChan.ch1, state);
            break;

        case "[Channel2]":
            this.setReloopLight(this.midiChan.subChan.ch2, state);
            break;

        case "[Channel3]":
            this.setReloopLight(this.midiChan.subChan.ch3, state);
            break;

        case "[Channel4]":
            this.setReloopLight(this.midiChan.subChan.ch4, state);
            break;
    }
};

PioneerDDJREV1.setReloopLight = function(channel, value) {
    midi.sendShortMsg(
        this.lights.decks.autoLoopBase.status + channel,
        this.lights.decks.autoLoopBase.data1,
        value
    );
    midi.sendShortMsg(
        this.lights.decks.autoLoopShiftBase.status + channel,
        this.lights.decks.autoLoopShiftBase.data1,
        value
    );
};

PioneerDDJREV1.trackLoadedLED = function() {
    midi.sendShortMsg(
        this.midiChan.special,
        group.match(script.channelRegEx)[1] - 1,
        value > this.zero ? this.lights.status.on : this.lights.status.off
    );
};

PioneerDDJREV1.toggleFxLight = function(value, group) {
    switch (group) {
        case "[EffectRack1_EffectUnit1_Effect1":
            this.setIndicatorState(
                this.lights.fx.fx1.slot1.status,
                this.lights.fx.fx1.slot1.data1,
                value
            );
            this.setIndicatorState(
                this.lights.fx.fx1.slot1Shift.status,
                this.lights.fx.fx1.slot1Shift.data1,
                value
            );
            break;

        case "[EffectRack1_EffectUnit1_Effect2":
            this.setIndicatorState(
                this.lights.fx.fx1.slot2.status,
                this.lights.fx.fx1.slot2.data1,
                value
            );
            this.setIndicatorState(
                this.lights.fx.fx1.slot2Shift.status,
                this.lights.fx.fx1.slot2Shift.data1,
                value
            );
            break;

        case "[EffectRack1_EffectUnit1_Effect3":
            this.setIndicatorState(
                this.lights.fx.fx1.slot3.status,
                this.lights.fx.fx1.slot3.data1,
                value
            );
            this.setIndicatorState(
                this.lights.fx.fx1.slot3Shift.status,
                this.lights.fx.fx1.slot3Shift.data1,
                value
            );
            break;

        case "[EffectRack1_EffectUnit2_Effect1":
            this.setIndicatorState(
                this.lights.fx.fx2.slot1.status,
                this.lights.fx.fx2.slot1.data1,
                value
            );
            this.setIndicatorState(
                this.lights.fx.fx2.slot1Shift.status,
                this.lights.fx.fx2.slot1Shift.data1,
                value
            );
            break;

        case "[EffectRack1_EffectUnit2_Effect2":
            this.setIndicatorState(
                this.lights.fx.fx2.slot2.status,
                this.lights.fx.fx2.slot2.data1,
                value
            );
            this.setIndicatorState(
                this.lights.fx.fx2.slot2Shift.status,
                this.lights.fx.fx2.slot2Shift.data1,
                value
            );
            break;

        case "[EffectRack1_EffectUnit2_Effect3":
            this.setIndicatorState(
                this.lights.fx.fx2.slot3.status,
                this.lights.fx.fx2.slot3.data1,
                value
            );
            this.setIndicatorState(
                this.lights.fx.fx2.slot3Shift.status,
                this.lights.fx.fx2.slot3Shift.data1,
                value
            );
            break;
    }
};

PioneerDDJREV1.samplerPlayOutputCallbackFunction = function(value, group) {
    if (value === this.zero) { return; }

    const curPad = group.match(script.samplerRegEx)[1];
    const data1Addr = 0x30 + ((
        curPad > this.padCount
            ? curPad - this.padCount
            : curPad
        ) - 1);

    if (curPad <= this.padCount) {
        this.startSamplerBlink(
            this.midiChan.note + this.midiChan.subChan.deck1,
            data1Addr,
            group
        );
        this.startSamplerBlink(
            this.midiChan.note + this.midiChan.subChan.deck3,
            data1Addr,
            group
        );
    } else {
        this.startSamplerBlink(
            this.midiChan.note + this.midiChan.subChan.deck2,
            data1Addr,
            group
        );
        this.startSamplerBlink(
            this.midiChan.note + this.midiChan.subChan.deck4,
            data1Addr,
            group
        );
    }
};

PioneerDDJREV1.startSamplerBlink = function(channel, control, group) {
    let val = this.lights.status.on;

    this.stopSamplerBlink(channel, control);
    this.timers[channel][control] = engine.beginTimer(250, () => {
        val = this.lights.status.on - val;

        // blink the appropriate pad
        midi.sendShortMsg(channel, control, val);
        // also blink the pad while SHIFT is pressed
        midi.sendShortMsg((channel + 1), control, val);

        if (engine.getValue(group, "play") < 1) {
            // kill timer
            this.stopSamplerBlink(channel, control);
            // set the pad LED to ON
            midi.sendShortMsg(channel, control, this.lights.status.on);
            // set the pad LED to ON while SHIFT is pressed
            midi.sendShortMsg((channel + 1), control, this.lights.status.on);
        }
    });
};

PioneerDDJREV1.stopSamplerBlink = function(channel, control) {
    this.timers[channel] = this.timers[channel] || {};

    if (this.timers[channel][control] !== undefined) {
        engine.stopTimer(this.timers[channel][control]);
        this.timers[channel][control] = undefined;
    }
};

/*
 * Shift button
 */
PioneerDDJREV1.shiftPressed = function(_channel, _control, value) {
    PioneerDDJREV1.shiftState = (value === 0x7F);
};

/*
 * Browser button press
 */
PioneerDDJREV1.browsePressed = function(_channel, _control, value) {
    if (value === this.zero) { return; } // ignore release

    if (engine.getValue("[PreviewDeck1]", "play")) {
        script.triggerControl("[PreviewDeck1]", "stop");
        PioneerDDJREV1.previewSeekEnabled = false;
    } else {
        engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 1);
        PioneerDDJREV1.previewSeekEnabled = true;
    }
};

/*
 * Jog wheels
 */
PioneerDDJREV1.jogTurn = function(channel, _control, value, _status, group) {
    const deckNum = channel + 1;
    // wheel center at 64; <64 rew >64 fwd
    let newVal = value - 64;

    // loop_in / out adjust
    const loopEnabled = engine.getValue(group, "loop_enabled");

    if (loopEnabled > this.zero) {
        if (this.loopAdjustIn[channel]) {
            newVal = newVal * this.loopAdjustMultiply + engine.getValue(group, "loop_start_position");
            engine.setValue(group, "loop_start_position", newVal);
            return;
        }
        if (this.loopAdjustOut[channel]) {
            newVal = newVal * this.loopAdjustMultiply + engine.getValue(group, "loop_end_position");
            engine.setValue(group, "loop_end_position", newVal);
            return;
        }
    }

    if (engine.isScratching(deckNum)) {
        engine.scratchTick(deckNum, newVal);
    } else { // fallback
        engine.setValue(group, "jog", newVal * this.bendScale);
    }
};

PioneerDDJREV1.jogSearch = function(_channel, _control, value, _status, group) {
    const newVal = (value - 64) * this.fastSeekScale;
    engine.setValue(group, "jog", newVal);
};

PioneerDDJREV1.jogTouch = function(channel, _control, value) {
    const deckNum = channel + 1;

    // skip while adjusting the loop points
    if (this.loopAdjustIn[channel] || this.loopAdjustOut[channel]) {
        return;
    }

    if (value !== this.zero && this.vinylMode) {
        engine.scratchEnable(deckNum, 720, 33 + 1 / 3, this.alpha, this.beta);
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
PioneerDDJREV1.tempoSliderMSB = function(_channel, _control, value, _status, group) {
    this.highResMSB[group].tempoSlider = value;
};

PioneerDDJREV1.tempoSliderLSB = function(_channel, _control, value, _status, group) {
    const fullValue = (this.highResMSB[group].tempoSlider << 7) + value;
    engine.setValue(group, "rate", 1 - (fullValue / 0x2000));
};

/*
 * BEAT SYNC
 */
PioneerDDJREV1.syncPressed = function(_channel, _control, value, _status, group) {
    if (engine.getValue(group, "sync_enabled") && value > 0) {
        engine.setValue(group, "sync_enabled", 0);
    } else {
        engine.setValue(group, "beatsync", value);
    }
};

PioneerDDJREV1.syncShiftPressed = function(_channel, _control, value, _status, group) {
    if (value) {
        engine.setValue(group, "sync_enabled", 1);
    }
};

/*
 * DECK SELECT
 *
 * Note that the controller sends different signals for a short press and a long
 * press of the same button.
 */
PioneerDDJREV1.cycleTempoRange = function(_channel, _control, value, _status, group) {
    if (value === this.zero) { return; } // ignore release

    const currRange = engine.getValue(group, "rateRange");
    let idx = 0;

    for (let i = this.zero; i < this.tempoRanges.length; i++) {
        if (currRange === this.tempoRanges[i]) {
            idx = (i + 1) % this.tempoRanges.length;
            break;
        }
    }

    engine.setValue(group, "rateRange", this.tempoRanges[idx]);
};

/*
 * Hotcue PAD button press
 */
PioneerDDJREV1.hotcuePadPress = function(_channel, control, value, _status, group) {
    const padNum = this.getPadNumber(control);
    engine.setValue(group, `hotcue_${padNum}_activate`, value);
};
PioneerDDJREV1.hotcuePadShiftPress = function(_channel, control, value, _status, group) {
    const padNum = this.getPadNumber(control);
    engine.setValue(group, `hotcue_${padNum}_clear`, value);
};

/*
 * Sampler PAD button press
 */
PioneerDDJREV1.samplerPadPress = function(_channel, _control, value, _status, group) {
    if (engine.getValue(group, "track_loaded")) {
        engine.setValue(group, "cue_gotoandplay", value);
    } else {
        engine.setValue(group, "LoadSelectedTrack", value);
    }
};
PioneerDDJREV1.samplerPadShiftPress = function(_channel, _control, value, _status, group) {
    if (engine.getValue(group, "play")) {
        engine.setValue(group, "cue_gotoandstop", value);
    } else if (engine.getValue(group, "track_loaded")) {
        engine.setValue(group, "eject", value);
    }
};
