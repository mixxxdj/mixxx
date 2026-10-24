// eslint-disable-next-line no-var
var TraktorX1MK1 = {};

const X1_DATA_OUT = 0x0c;
const X1_REPORT_LENGTH = 24;
const X1_DIM = 0x0a;
const X1_ON = 0x7f;

// How many engine samples to move a loop in/out point per encoder click when
// the corresponding LOOP IN/OUT button is held (fine, non-quantized
// adjustment per section 4.3.3 of the manual). ~23 ms at 44.1 kHz.

const FINE_LOOP_STEP = 1024;

TraktorX1MK1.ledStatus = [X1_DATA_OUT, ...new Array(31).fill(0)];
TraktorX1MK1.connections = [];
TraktorX1MK1.buttonState = {};
TraktorX1MK1.shiftedButtons = {};
TraktorX1MK1.encoderState = {};
TraktorX1MK1.knobState = {};
TraktorX1MK1.shift = false;
TraktorX1MK1.hotcueMode = false;

// Map hotcue transport button actions to hotcue numbers (manual section 4.5).

TraktorX1MK1.hotcueNumbers = {
    "IN": 1,
    "OUT": 2,
    "< BEAT": 3,
    "BEAT >": 4,
    "CUE/ABS": 5,
    "CUE/REL": 6,
    "PLAY": 7,
    "SYNC": 8,
};

TraktorX1MK1.ledMap = {
    "FX A: ON": 8,
    "FX A: 1": 7,
    "FX A: 2": 6,
    "FX A: 3": 5,
    "FX B: ON": 4,
    "FX B: 1": 3,
    "FX B: 2": 2,
    "FX B: 3": 1,
    "Shift": 29,
    "Hotcue": 30,
    "Deck A: FX1": 25,
    "Deck A: FX2": 26,
    "Deck A: IN": 18,
    "Deck A: OUT": 17,
    "Deck A: < BEAT": 20,
    "Deck A: BEAT >": 19,
    "Deck A: CUE/ABS": 22,
    "Deck A: CUE/REL": 21,
    "Deck A: PLAY": 24,
    "Deck A: SYNC": 23,
    "Deck B: FX1": 28,
    "Deck B: FX2": 27,
    "Deck B: IN": 16,
    "Deck B: OUT": 15,
    "Deck B: < BEAT": 14,
    "Deck B: BEAT >": 13,
    "Deck B: CUE/ABS": 12,
    "Deck B: CUE/REL": 11,
    "Deck B: PLAY": 10,
    "Deck B: SYNC": 9,
};

TraktorX1MK1.buttonBits = {
    "FX A: ON": 35,
    "FX A: 1": 34,
    "FX A: 2": 33,
    "FX A: 3": 32,
    "FX B: ON": 47,
    "FX B: 1": 46,
    "FX B: 2": 45,
    "FX B: 3": 44,
    "Shift": 43,
    "Hotcue": 40,
    "Deck A: BROWSE": 39,
    "Deck A: LOOP": 37,
    "Deck A: FX1": 22,
    "Deck A: FX2": 23,
    "Deck A: IN": 27,
    "Deck A: OUT": 12,
    "Deck A: < BEAT": 13,
    "Deck A: BEAT >": 26,
    "Deck A: CUE/ABS": 25,
    "Deck A: CUE/REL": 14,
    "Deck A: PLAY": 15,
    "Deck A: SYNC": 24,
    "Deck B: BROWSE": 38,
    "Deck B: LOOP": 36,
    "Deck B: FX1": 41,
    "Deck B: FX2": 42,
    "Deck B: IN": 19,
    "Deck B: OUT": 28,
    "Deck B: < BEAT": 29,
    "Deck B: BEAT >": 18,
    "Deck B: CUE/ABS": 17,
    "Deck B: CUE/REL": 30,
    "Deck B: PLAY": 31,
    "Deck B: SYNC": 16,
};

TraktorX1MK1.led = function(name, enabled, deferSend) {
    const index = this.ledMap[name];
    if (index === undefined) { return false; }

    const value = enabled ? X1_ON : X1_DIM;
    if (this.ledStatus[index] === value) { return false; }

    this.ledStatus[index] = value;

    if (!deferSend) { this.sendLeds(); }

    return true;
};

TraktorX1MK1.sendLeds = function() {
    controller.send(this.ledStatus);
};

TraktorX1MK1.connectLed = function(group, control, ledName, invert) {
    const self = this;

    this.connections.push(engine.makeConnection(group, control, function(value) {
        if (self.hotcueMode && ledName.indexOf("Deck ") === 0) { return; }

        if (self.buttonState[ledName]) {
            self.led(ledName, true);

            return;
        }

        self.led(ledName, invert ? !value : !!value);
    }));
};

TraktorX1MK1.refreshButtonLed = function(name) {
    if (name.indexOf("Deck ") === 0) {
        if (this.hotcueMode) {
            this.refreshHotcueLeds();
        } else if (name.indexOf("< BEAT") !== -1 || name.indexOf("BEAT >") !== -1) {
            this.led(name, false);
        } else {
            this.refreshDeckLeds();
        }
    } else if (/^FX [AB]:/.test(name)) {
        const unit = name.indexOf("FX A:") === 0 ? 1 : 2;
        const group = `[EffectRack1_EffectUnit${  unit  }]`;
        const slot = name.match(/: (\d)$/);
        let enabled;

        if (slot) {

            // NOTE: group already ends with "]", so rebuild the full group
            // path instead of appending to it (the old code produced an
            // invalid group name such as "[EffectRack1_EffectUnit1]_Effect1]").

            const effectGroup = `[EffectRack1_EffectUnit${  unit  }_Effect${  slot[1]  }]`;
            enabled = engine.getValue(effectGroup, "enabled");
        } else {
            enabled = engine.getValue(group, "enabled");
        }

        this.led(name, enabled);
    }
};

TraktorX1MK1.refreshHotcueLeds = function() {
    if (!this.hotcueMode) { return; }

    const self = this;
    let dirty = false;

    [1, 2].forEach(function(deck) {
        const side = deck === 1 ? "A" : "B";
        for (let hotcue = 1; hotcue <= 8; hotcue++) {
            dirty = self.led(`Deck ${  side  }: ${  ["IN", "OUT", "< BEAT", "BEAT >", "CUE/ABS", "CUE/REL", "PLAY", "SYNC"][hotcue - 1]}`,
                !!engine.getValue(`[Channel${  deck  }]`, `hotcue_${  hotcue  }_enabled`), true) || dirty;
        }
    });

    if (dirty) { this.sendLeds(); }
};

TraktorX1MK1.refreshDeckLeds = function() {
    const self = this;
    let dirty = false;

    [1, 2].forEach(function(deck) {
        const side = deck === 1 ? "A" : "B";
        const group = `[Channel${  deck  }]`;

        dirty = self.led(`Deck ${  side  }: PLAY`, engine.getValue(group, "play"), true) || dirty;
        dirty = self.led(`Deck ${  side  }: SYNC`, engine.getValue(group, "sync_enabled"), true) || dirty;
        dirty = self.led(`Deck ${  side  }: IN`, engine.getValue(group, "loop_enabled"), true) || dirty;
        dirty = self.led(`Deck ${  side  }: OUT`, engine.getValue(group, "loop_enabled"), true) || dirty;

        // Cue LEDs follow the cue indicator (lights while a floating cue is
        // set, flashes in tempo while playing). They are state driven, so
        // they never get stuck on after a button press.

        dirty = self.led(`Deck ${  side  }: CUE/REL`, !!engine.getValue(group, "cue_indicator"), true) || dirty;
        dirty = self.led(`Deck ${  side  }: CUE/ABS`, !!engine.getValue(group, "cue_indicator"), true) || dirty;

        [1, 2].forEach(function(unit) {
            const effectGroup = `[EffectRack1_EffectUnit${  unit  }]`;
            dirty = self.led(`Deck ${  side  }: FX${  unit}`, engine.getValue(effectGroup, `group_${  group  }_enable`), true) || dirty;
        });
    });

    if (dirty) { this.sendLeds(); }
};

TraktorX1MK1.setAction = function(group, control, value) {
    engine.setValue(group, control, value);
};

TraktorX1MK1.pulse = function(group, control) {
    this.setAction(group, control, 1);
    this.setAction(group, control, 0);
};

TraktorX1MK1.processButton = function(name, down) {
    if (this.buttonState[name] === down) { return; }

    const shifted = down ? this.shift : !!this.shiftedButtons[name];

    if (down) {
        this.shiftedButtons[name] = shifted;
    } else {
        delete this.shiftedButtons[name];
    }

    this.buttonState[name] = down;

    if (this.ledMap[name] !== undefined) {
        if (down) {
            this.led(name, true);
        } else {
            this.refreshButtonLed(name);
        }
    }

    if (name === "Shift") {
        this.shift = down;
        this.led("Shift", down);

        return;
    }

    if (name === "Hotcue") {
        if (down) {
            this.hotcueMode = !this.hotcueMode;

            if (this.hotcueMode) {
                this.refreshHotcueLeds();
            } else {
                this.refreshDeckLeds();
            }
        }

        this.led("Hotcue", this.hotcueMode);

        return;
    }

    if (name === "Deck A: BROWSE" || name === "Deck B: BROWSE") {
        if (down) {
            const browseDeck = name.indexOf("Deck A:") === 0 ? 1 : 2;

            if (this.shift) {

                // Manual section 4.2: SHIFT + press BROWSE switches the layout
                // (VIEW) to hide the decks and maximize the browse window.
                // Mixxx does not expose an engine control for layout view
                // switching, so this cannot be mapped to a control here.

                print("Traktor X1 MK1: SHIFT+BROWSE (VIEW layout toggle) has no Mixxx engine control");
            } else {
                this.pulse(`[Channel${  browseDeck  }]`, "LoadSelectedTrack");
            }
        }

        return;
    }

    if (name === "Deck A: LOOP" || name === "Deck B: LOOP") {
        if (down) {
            const loopDeck = name.indexOf("Deck A:") === 0 ? 1 : 2;

            if (this.shift) {

                // Manual section 4.3.2: SHIFT + push LOOP knob loops the next
                // stored loop point (Mixxx equivalent: reloop toggle).

                this.pulse(`[Channel${  loopDeck  }]`, "reloop_toggle");
            } else {

                // Manual section 4.3.1: push LOOP knob sets a loop with the
                // currently selected length, pressing again deactivates it.

                this.pulse(`[Channel${  loopDeck  }]`, "beatloop_activate");
            }
        }

        return;
    }

    if (/^FX [AB]:/.test(name)) {
        if (!down) { return; }

        const unit = name.indexOf("FX A:") === 0 ? 1 : 2;
        const fxGroup = `[EffectRack1_EffectUnit${  unit  }]`;
        const slot = name.match(/: (\d)$/);

        if (name.endsWith(": ON")) {
            if (shifted) {
                const focused = engine.getValue(fxGroup, "focused_effect") || 0;
                engine.setValue(fxGroup, "focused_effect", (focused % 3) + 1);
            } else {
                this.setAction(fxGroup, "enabled", engine.getValue(fxGroup, "enabled") ? 0 : 1);
            }
        } else if (slot) {
            const effectGroup = `[EffectRack1_EffectUnit${  unit  }_Effect${  slot[1]  }]`;
            this.setAction(effectGroup, "enabled", engine.getValue(effectGroup, "enabled") ? 0 : 1);
        }

        return;
    }

    const deck = name.indexOf("Deck A:") === 0 ? 1 : 2;
    const group = `[Channel${  deck  }]`;
    const deckAction = name.substring(name.indexOf(":") + 2);
    let hotcueNumber;

    if (this.hotcueMode && !this.shift &&
        /^(IN|OUT|< BEAT|BEAT >|CUE\/ABS|CUE\/REL|PLAY|SYNC)$/.test(deckAction)) {

        if (!down) { return; }

        hotcueNumber = this.hotcueNumbers[deckAction];
        this.pulse(group, `hotcue_${  hotcueNumber  }_activate`);

        return;
    }

    if (shifted) {

        // Manual section 4.5: in Hotcue mode, SHIFT + a lit hotcue button
        // deletes that cue point.

        if (this.hotcueMode &&
            /^(IN|OUT|< BEAT|BEAT >|CUE\/ABS|CUE\/REL|PLAY|SYNC)$/.test(deckAction)) {
            if (down) {
                hotcueNumber = this.hotcueNumbers[deckAction];
                this.pulse(group, `hotcue_${  hotcueNumber  }_clear`);
            }

            return;
        }
        if (deckAction === "FX1" || deckAction === "FX2") {
            if (down) {
                const deckControl = deckAction === "FX1" ? "sync_leader" : "quantize";
                const currentValue = engine.getValue(group, deckControl);
                engine.setValue(group, deckControl, currentValue ? 0 : 1);
            }

            return;
        }

        if (deckAction === "IN" || deckAction === "OUT") {
            if (down) {
                const loopSize = engine.getValue(group, "beatloop_size") || 1;
                engine.setValue(group, "beatloop_size", Math.max(0.0625, Math.min(64, loopSize * (deckAction === "IN" ? 0.5 : 2))));
            }

            return;
        }

        if (deckAction === "< BEAT" || deckAction === "BEAT >") {
            this.setAction(group, deckAction === "< BEAT" ? "rate_temp_down" : "rate_temp_up", down ? 1 : 0);

            return;
        }

        if (deckAction === "CUE/ABS") {
            if (down) {
                this.pulse(group, "cue_goto");
            }

            return;
        }

        if (deckAction === "CUE/REL") {
            if (down) {
                this.pulse(group, "cue_default");
            }

            return;
        }

        if (deckAction === "PLAY") {
            if (down) {
                this.setAction(group, "keylock", engine.getValue(group, "keylock") ? 0 : 1);
            }

            return;
        }

        if (deckAction === "SYNC") {
            if (down) {
                this.pulse(group, "beatsync_phase");
            }

            return;
        }
    }

    if (deckAction === "FX1" || deckAction === "FX2") {
        if (down) {
            const unitGroup = `[EffectRack1_EffectUnit${  deckAction === "FX1" ? 1 : 2  }]`;
            const assignmentControl = `group_${  group  }_enable`;

            this.setAction(unitGroup, assignmentControl,
                engine.getValue(unitGroup, assignmentControl) ? 0 : 1);
        }
    } else if (deckAction === "IN") {

        // Manual section 4.3.1: LOOP IN sets a manual loop in point and arms
        // for the loop out point; press again to cancel. Feed press and
        // release so Mixxx's looping control sees both.

        engine.setValue(group, "loop_in", down ? 1 : 0);
    } else if (deckAction === "OUT") {

        // Manual section 4.3.1: LOOP OUT alone sets a loop out point; after
        // LOOP IN it sets the manual loop out point. Feed press and release.

        engine.setValue(group, "loop_out", down ? 1 : 0);
    } else if (deckAction === "< BEAT" || deckAction === "BEAT >") {
        if (down) {
            this.setAction(group, "beatjump", deckAction === "< BEAT" ? -1 : 1);
        }
    } else if (deckAction === "CUE/ABS") {
        if (down) {

            // Manual section 4.4: CUE|ABS starts playback from the last
            // floating cue point; if none is set, set one first.

            if (engine.getValue(group, "cue_indicator")) {
                this.pulse(group, "cue_gotoandplay");
            } else {
                this.pulse(group, "cue_default");
            }
        }
    } else if (deckAction === "CUE/REL") {
        if (down) {
            this.pulse(group, "cue_default");
        }
    } else if (deckAction === "PLAY") {
        if (down) {
            this.setAction(group, "play", engine.getValue(group, "play") ? 0 : 1);
        }
    } else if (deckAction === "SYNC") {
        if (down) {
            this.setAction(group, "sync_enabled", engine.getValue(group, "sync_enabled") ? 0 : 1);
        }
    }
};

TraktorX1MK1.adjustLoopPoint = function(group, control, direction) {

    // Manual section 4.3.3: move a loop in/out point in non-quantized steps.

    const current = engine.getValue(group, control) || 0;
    const next = Math.max(0, current + direction * FINE_LOOP_STEP);
    engine.setValue(group, control, next);
};

TraktorX1MK1.processEncoder = function(name, value) {
    const previous = this.encoderState[name];
    this.encoderState[name] = value;

    if (previous === undefined || previous === value) { return; }

    const step = (value - previous + 16) % 16;
    const direction = step > 0 && step < 8 ? step : step > 8 ? step - 16 : 0;

    if (!direction) { return; }

    const deck = name.indexOf("Deck A:") === 0 ? 1 : 2;
    const group = `[Channel${  deck  }]`;
    const side = deck === 1 ? "A" : "B";

    if (name.endsWith("BROWSE")) {
        if (this.shift) {

            // Manual section 4.2: SHIFT + turn BROWSE knob quickly seeks
            // through the currently loaded track.

            const position = engine.getValue(group, "playposition") || 0;
            engine.setValue(group, "playposition", Math.max(0, Math.min(1, position + direction * 0.02)));
        } else {
            engine.setValue("[Playlist]", "SelectTrackKnob", direction);
        }
    } else {

        // Manual section 4.3: the LOOP knob (called SIZE in the report).

        if (this.shift) {

            // Manual section 4.3.2: SHIFT + turn LOOP knob moves the entire
            // loop by 1 beat steps left or right. Mixxx's beatjump moves the
            // loop when one is active, otherwise it seeks the play position.

            engine.setValue(group, "beatjump", direction);
        } else if (this.buttonState[`Deck ${  side  }: IN`]) {

            // Manual section 4.3.3: hold LOOP IN + turn LOOP knob moves the
            // loop in non-quantized steps.

            this.adjustLoopPoint(group, "loop_start_position", direction);
        } else if (this.buttonState[`Deck ${  side  }: OUT`]) {

            // Manual section 4.3.3: hold LOOP OUT + turn LOOP knob moves the
            // loop out point in non-quantized steps.

            this.adjustLoopPoint(group, "loop_end_position", direction);
        } else {

            // Manual section 4.3.1: turn LOOP knob changes the selected loop
            // length (doubles/halves per click).

            const current = engine.getValue(group, "beatloop_size") || 1;
            const next = Math.max(0.0625, Math.min(64, current * Math.pow(2, direction)));

            engine.setValue(group, "beatloop_size", next);
        }
    }
};

TraktorX1MK1.processKnob = function(name, value) {
    const previous = this.knobState[name];
    this.knobState[name] = value;

    if (previous === undefined || previous === value) { return; }

    const unit = name.indexOf("FX A:") === 0 ? 1 : 2;
    const effect = name.match(/FX([123])$/);

    if (name.endsWith("DRY/WET")) {
        engine.setValue(`[EffectRack1_EffectUnit${  unit  }]`, "mix", value);
    } else if (effect) {
        engine.setValue(`[EffectRack1_EffectUnit${  unit  }_Effect${  effect[1]  }]`, "meta", value);
    }
};

TraktorX1MK1.init = function() {
    this.buttonState = {};
    this.shiftedButtons = {};
    this.encoderState = {};
    this.knobState = {};
    this.shift = false;
    this.hotcueMode = false;
    this.connections = [];
    this.ledStatus = [X1_DATA_OUT, ...new Array(31).fill(X1_DIM)];
    this.ledStatus[31] = 0;
    this.ledStatus[this.ledMap.Hotcue] = X1_DIM;
    this.sendLeds();

    const decks = [1, 2];
    const self = this;

    decks.forEach(function(deck) {
        const group = `[Channel${  deck  }]`;
        const side = deck === 1 ? "A" : "B";

        self.connectLed(group, "play", `Deck ${  side  }: PLAY`);
        self.connectLed(group, "sync_enabled", `Deck ${  side  }: SYNC`);
        self.connectLed(group, "loop_enabled", `Deck ${  side  }: IN`);
        self.connectLed(group, "loop_enabled", `Deck ${  side  }: OUT`);

        // Cue LEDs follow the cue indicator so they cannot get stuck lit.

        self.connectLed(group, "cue_indicator", `Deck ${  side  }: CUE/REL`);
        self.connectLed(group, "cue_indicator", `Deck ${  side  }: CUE/ABS`);

        [1, 2].forEach(function(unit) {
            const fxGroup = `[EffectRack1_EffectUnit${  unit  }]`;
            self.connectLed(fxGroup, `group_${  group  }_enable`, `Deck ${  side  }: FX${  unit}`);
        });

        for (let hotcue = 1; hotcue <= 8; hotcue++) {
            self.connections.push(engine.makeConnection(group, `hotcue_${  hotcue  }_enabled`, function() {
                self.refreshHotcueLeds();
            }));
        }
    });

    [1, 2].forEach(function(unit) {
        const group = `[EffectRack1_EffectUnit${  unit  }]`;
        const prefix = unit === 1 ? "FX A" : "FX B";

        engine.softTakeover(group, "mix", true);
        self.connectLed(group, "enabled", `${prefix  }: ON`);

        [1, 2, 3].forEach(function(slot) {
            const effectGroup = `[EffectRack1_EffectUnit${  unit  }_Effect${  slot  }]`;
            engine.softTakeover(effectGroup, "meta", true);
            self.connectLed(effectGroup, "enabled", `${prefix  }: ${  slot}`);
        });
    });
};

TraktorX1MK1.shutdown = function() {
    this.connections.forEach(function(connection) { connection.disconnect(); });
    this.connections = [];
    this.ledStatus = [X1_DATA_OUT, ...new Array(31).fill(0)];
    this.ledStatus[31] = 0;
    this.sendLeds();
};

TraktorX1MK1.bit = function(data, bitIndex) {
    const byteIndex = Math.floor(bitIndex / 8);
    const bitPosition = 7 - (bitIndex % 8);

    return (data[byteIndex] >> bitPosition) & 1;
};

TraktorX1MK1.nibble = function(value, high) {
    return high ? (value >> 4) & 0x0f : value & 0x0f;
};

TraktorX1MK1.knobValue = function(highByte, lowByte) {
    return Math.max(0, Math.min(1, (((highByte << 8) | lowByte) / 0x0fff)));
}

TraktorX1MK1.incomingData = function(data, length) {
    if (length < X1_REPORT_LENGTH) { return; }

    const bytes = Array.from(data).slice(0, X1_REPORT_LENGTH);

    for (const name in this.buttonBits) {
        this.processButton(name, TraktorX1MK1.bit(bytes, this.buttonBits[name]) !== 0);
    }

    this.processEncoder("Deck A: BROWSE", TraktorX1MK1.nibble(bytes[6], false));
    this.processEncoder("Deck B: BROWSE", TraktorX1MK1.nibble(bytes[6], true));
    this.processEncoder("Deck A: SIZE", TraktorX1MK1.nibble(bytes[7], false));
    this.processEncoder("Deck B: SIZE", TraktorX1MK1.nibble(bytes[7], true));

    this.processKnob("FX A: DRY/WET", TraktorX1MK1.knobValue(bytes[16], bytes[17]));
    this.processKnob("FX A: FX1", TraktorX1MK1.knobValue(bytes[20], bytes[21]));
    this.processKnob("FX A: FX2", TraktorX1MK1.knobValue(bytes[22], bytes[23]));
    this.processKnob("FX A: FX3", TraktorX1MK1.knobValue(bytes[18], bytes[19]));
    this.processKnob("FX B: DRY/WET", TraktorX1MK1.knobValue(bytes[12], bytes[13]));
    this.processKnob("FX B: FX1", TraktorX1MK1.knobValue(bytes[10], bytes[11]));
    this.processKnob("FX B: FX2", TraktorX1MK1.knobValue(bytes[8], bytes[9]));
    this.processKnob("FX B: FX3", TraktorX1MK1.knobValue(bytes[14], bytes[15]));
};
