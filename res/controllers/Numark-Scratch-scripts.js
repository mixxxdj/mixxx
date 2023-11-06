// eslint-disable-next-line no-var
var NumarkScratch = {};

/*
 * USER CONFIGURABLE SETTINGS.
 */

// Defines the Beatloop Roll sizes for the 4 pads, for available values see:
// https://manual.mixxx.org/2.4/en/chapters/appendix/mixxx_controls.html#control-[ChannelN]-beatlooproll_X_activate
// Default: [0.25, 0.5, 1, 2]
NumarkScratch.autoLoopSizes = [
    "0.25",
    "0.5",
    "1",
    "2",
];

// Defines how the Loop Encoder functions.
// If 'true' the Encoder scrolls the library/loads track. Shift + Encoder manages looping.
// If 'false' the Encoder manages looping. Shift + Encoder scrolls the library/loads track (Serato default).
// Default: false
NumarkScratch.invertLoopEncoderFunction = false;

// Defines the bightness of button LEDs when they inactive.
// '0x00' sets the LEDSs to completely off. '0x01 sets the LEDs to dim.
// Default: 0x01 (dim)
NumarkScratch.LOW_LIGHT = 0x01;

/*
 * CODE
 */

// State variable, don't touch
NumarkScratch.shifted = false;

components.Button.prototype.off = NumarkScratch.LOW_LIGHT;

NumarkScratch.init = function() {
    // Initialize component containers
    NumarkScratch.deck = new components.ComponentContainer();
    NumarkScratch.effect = new components.ComponentContainer();
    for (let i = 0; i < 2; i++) {
        NumarkScratch.deck[i] = new NumarkScratch.Deck(i + 1);
        NumarkScratch.effect[i] = new NumarkScratch.EffectUnit(i + 1);
    }

    NumarkScratch.xfader = new NumarkScratch.XfaderContainer();

    const createVuCallback = function(deckOffset) {
        return function(value) {
            midi.sendShortMsg(0xB0 + deckOffset, 0x1F, value * 90);
        };
    };

    engine.makeConnection("[Channel1]", "vu_meter", createVuCallback(0));
    engine.makeConnection("[Channel2]", "vu_meter", createVuCallback(1));

    // Trigger is needed to initialize lights to 0x01
    NumarkScratch.deck.forEachComponent(function(component) {
        component.trigger();
    });
    NumarkScratch.effect.forEachComponent(function(component) {
        component.trigger();
    });

    // set FX buttons init light & Shift button init light
    midi.sendShortMsg(0x98, 0x00, NumarkScratch.LOW_LIGHT);
    midi.sendShortMsg(0x98, 0x01, NumarkScratch.LOW_LIGHT);
    midi.sendShortMsg(0x98, 0x02, NumarkScratch.LOW_LIGHT);
    midi.sendShortMsg(0x99, 0x03, NumarkScratch.LOW_LIGHT);
    midi.sendShortMsg(0x99, 0x04, NumarkScratch.LOW_LIGHT);
    midi.sendShortMsg(0x99, 0x05, NumarkScratch.LOW_LIGHT);
    midi.sendShortMsg(0x9F, 0x32, NumarkScratch.LOW_LIGHT);

    // Send Serato SysEx messages to request initial state and unlock pads
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x00, 0xF7]);

};

NumarkScratch.shutdown = function() {
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x04, 0xF7]);
};

NumarkScratch.shift = function() {
    NumarkScratch.shifted = true;
    NumarkScratch.deck.shift();
    NumarkScratch.effect.shift();
};

NumarkScratch.unshift = function() {
    NumarkScratch.shifted = false;
    NumarkScratch.deck.unshift();
    NumarkScratch.effect.unshift();
};

NumarkScratch.EffectUnit = function(deckNumber) {

    const inputUnshifted = function(channel, control, value, _status, _group) {
        if (value !== 0x7F) {
            return;
        }
        // Enable the current button
        engine.setValue(this.group, this.inKey, true);

        // Disable all other buttons in the same unit
        NumarkScratch.effect[deckNumber - 1].effectButtons.forEach((effectButton) => {
            if (effectButton !== this) {
                engine.setValue(effectButton.group, effectButton.inKey, false);
            }
        });

        // Disable all buttons in the other unit
        NumarkScratch.effect[1 - (deckNumber - 1)].effectButtons.forEach((effectButton) => {
            engine.setValue(effectButton.group, effectButton.inKey, false);
        });
    };

    const inputShifted = function(channel, control, value, _status, _group) {
        if (value === 0x7F) {
            // Toggle the current button state
            const currentState = engine.getValue(this.group, this.inKey);
            engine.setValue(this.group, this.inKey, !currentState);
        }
    };

    this.effectButtons = [];
    for (let i = 0; i < 3; i++) {
        this.effectButtons[i] = new components.Button({
            group: "[EffectRack1_EffectUnit" + deckNumber + "_Effect" + (i + 1) + "]",
            midi: [0x98 + deckNumber - 1, (deckNumber - 1) * 3 + i],
            inKey: "enabled",
            outKey: "enabled", // Bind the LED state to the effect enabled state
            input: inputUnshifted,
            shift: function() {
                this.input = inputShifted;
            },
            unshift: function() {
                this.input = inputUnshifted;
            },
        });
    }

    this.paddle = new components.Button({
        group: "[EffectRack1_EffectUnit" + deckNumber + "]",
        outKey: "enabled",
        inKey: "group_[Channel" + deckNumber + "]_enable",
        input: function(channel, control, value, _status, _group) {
            this.isPress = value !== 0;
            engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel" + deckNumber + "]_enable", this.isPress);
            engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel" + deckNumber + "]_enable", this.isPress);
            this.trigger();
        }
    });

    this.dryWetKnob = new components.Pot({
        group: "[EffectRack1_EffectUnit" + deckNumber + "]",
        inKey: "mix",
        shift: function() {
            this.inKey = "super1";
        },
        unshift: function() {
            this.inKey = "mix";
        },
    });
};
NumarkScratch.EffectUnit.prototype = new components.ComponentContainer();

NumarkScratch.XfaderContainer = function() {
    this.crossfader = new components.Pot({
        midi: [0xBF, 0x08],
        group: "[Mixer]",
        inKey: "crossfader",
    });

    this.setCurve = function(channel, control, value, _status, _group) {
        switch (value) {
        case 0x00: // Additive/Linear
            engine.setValue("[Mixer Profile]", "xFaderMode", 0);
            engine.setValue("[Mixer Profile]", "xFaderCalibration", 0.4);
            engine.setValue("[Mixer Profile]", "xFaderCurve", 0.9);
            break;
        case 0x7F:  // Picnic Bench/Fast Cut
            engine.setValue("[Mixer Profile]", "xFaderMode", 0);
            engine.setValue("[Mixer Profile]", "xFaderCalibration", 0.9);
            engine.setValue("[Mixer Profile]", "xFaderCurve", 7.0);
            break;
        }
    };

    this.xFaderReverse = function(channel, control, value, _status, _group) {
        // 0x7F is ON, 0x00 is OFF
        engine.setValue("[Mixer Profile]", "xFaderReverse", (value === 0x7F) ? 1 : 0);
    };
};

NumarkScratch.XfaderContainer.prototype = new components.ComponentContainer();

NumarkScratch.Deck = function(number) {
    components.Deck.call(this, number);

    const channel = number;

    this.channelInputPC = new components.Button({
        group: "[Channel" + number + "]",
        input: function(channel, control, value, status, group) {
            if (value === 0x00) {
                // Set vinyl control enabled and passthrough disabled
                engine.setValue(group, "vinylcontrol_enabled", 1);
                engine.setValue(group, "passthrough", 0);
            }
        }
    });

    this.channelInputLinePhono = new components.Button({
        group: "[Channel" + number + "]",
        input: function(channel, control, value, status, group) {
            if (value === 0x02) {
                // Set vinyl control disabled and passthrough enabled
                engine.setValue(group, "vinylcontrol_enabled", 0);
                engine.setValue(group, "passthrough", 1);
            }
        }
    });

    this.pflButton = new components.Button({
        midi: [0x90 + channel, 0x1B],
        key: "pfl",
        output: function(value) {
            const note = (value === 0x00 ? 0x80 : 0x90) + channel;
            midi.sendShortMsg(note, 0x1B, this.outValueScale(value));
        }
    });

    this.volume = new components.Pot({
        inKey: "volume"
    });

    this.treble = new components.Pot({
        group: "[EqualizerRack1_" + this.currentDeck + "_Effect1]",
        inKey: "parameter3"
    });

    this.mid = new components.Pot({
        group: "[EqualizerRack1_" + this.currentDeck + "_Effect1]",
        inKey: "parameter2"
    });

    this.bass = new components.Pot({
        group: "[EqualizerRack1_" + this.currentDeck + "_Effect1]",
        inKey: "parameter1"
    });

    this.filter = new components.Pot({
        group: "[QuickEffectRack1_" + this.currentDeck + "]",
        inKey: "super1"
    });

    this.gain = new components.Pot({
        inKey: "pregain"
    });

    const encoderInvert = function(key) {
        if (NumarkScratch.invertLoopEncoderFunction) {
            return key === "unshift" ? "shift" : "unshift";
        }
        return key;
    };

    const loopPlusEncoderConf = {
        midi: [0x93 + channel, 0x35],
    };
    loopPlusEncoderConf[encoderInvert("unshift")] = function() {
        this.group = "[Channel" + channel + "]";
        this.inKey = "loop_double";
    };
    loopPlusEncoderConf[encoderInvert("shift")] = function() {
        this.group = "[Library]";
        this.inKey = "MoveDown";
    };
    this.loopPlus = new components.Encoder(loopPlusEncoderConf);

    const loopMinusEncoderConf = {
        midi: [0x93 + channel, 0x34],
    };
    loopMinusEncoderConf[encoderInvert("unshift")] = function() {
        this.group = "[Channel" + channel + "]";
        this.inKey = "loop_halve";
    };
    loopMinusEncoderConf[encoderInvert("shift")] = function() {
        this.group = "[Library]";
        this.inKey = "MoveUp";
    };
    this.loopMinus = new components.Encoder(loopMinusEncoderConf);

    const loopButtonConf = {
        midi: [0x93 + channel, 0x3F],
    };

    loopButtonConf[encoderInvert("unshift")] = function() {
        this.group = "[Channel" + channel + "]";
        this.inKey = "beatloop_activate";
    };
    loopButtonConf[encoderInvert("shift")] = function() {
        this.group = "[Channel" + channel + "]";
        this.inKey = "LoadSelectedTrack";
    };
    this.loopButton = new components.Button(loopButtonConf);

    this.padSection = new NumarkScratch.PadSection(number);

    this.shiftButton = new components.Button({
        midi: [0x9F, 0x32],
        input: function(channel, control, value) {
            if (this.isPress(channel, control, value)) {
                NumarkScratch.shift();
                this.send(this.on);
            } else {
                NumarkScratch.unshift();
                this.send(this.off);
            }
        }
    });

    this.reconnectComponents(function(component) {
        if (component.group === undefined) {
            component.group = this.currentDeck;
        }
    });
};

NumarkScratch.Deck.prototype = new components.Deck();

// Pad modes control codes
NumarkScratch.PadModeControls = {
    HOTCUE: 0x00,
    SAMPLER: 0x07,
    ROLL: 0x0B,
};

NumarkScratch.PadSection = function(deckNumber) {
    components.ComponentContainer.call(this);

    this.modes = {};
    this.modes[NumarkScratch.PadModeControls.HOTCUE] = new NumarkScratch.ModeHotcue(deckNumber);
    this.modes[NumarkScratch.PadModeControls.SAMPLER] = new NumarkScratch.ModeSampler(deckNumber);
    this.modes[NumarkScratch.PadModeControls.ROLL] = new NumarkScratch.ModeRoll(deckNumber);

    this.modeButtonPress = function(channel, control, _value) {
        this.setMode(channel, control);
    };

    this.padPress = function(channel, control, value, status, group) {
        const i = (control - 0x14) % 8;
        this.currentMode.pads[i].input(channel, control, value, status, group);
    };

    this.setMode = function(_channel, control) {
        const newMode = this.modes[control];
        this.currentMode.forEachComponent(function(component) {
            component.disconnect();
        });

        newMode.forEachComponent(function(component) {
            component.connect();
            component.trigger();
        });

        this.currentMode = newMode;
    };

    this.currentMode = this.modes[NumarkScratch.PadModeControls.HOTCUE];
};
NumarkScratch.PadSection.prototype = Object.create(components.ComponentContainer.prototype);

NumarkScratch.ModeHotcue = function(deckNumber) {
    components.ComponentContainer.call(this);

    this.control = NumarkScratch.PadModeControls.HOTCUE;

    this.pads = new components.ComponentContainer();
    for (let i = 0; i < 4; i++) {
        this.pads[i] = new components.HotcueButton({
            group: "[Channel" + deckNumber + "]",
            midi: [0x93 + deckNumber, 0x14 + i],
            number: i + 1,
            shiftControl: true,
            sendShifted: true,
            shiftOffset: 0x08,
            outConnect: false
        });
    }
};
NumarkScratch.ModeHotcue.prototype = Object.create(components.ComponentContainer.prototype);

NumarkScratch.ModeRoll = function(deckNumber) {
    components.ComponentContainer.call(this);

    this.control = NumarkScratch.PadModeControls.ROLL;

    this.pads = new components.ComponentContainer();
    for (let i = 0; i < 4; i++) {
        this.pads[i] = new components.Button({
            group: "[Channel" + deckNumber + "]",
            midi: [0x93 + deckNumber, 0x14 + i],
            size: NumarkScratch.autoLoopSizes[i],
            shiftControl: true,
            sendShifted: true,
            shiftOffset: 0x08,
            inKey: "beatlooproll_" + NumarkScratch.autoLoopSizes[i] + "_activate",
            outKey: "beatloop_" + NumarkScratch.autoLoopSizes[i] + "_enabled",
            outConnect: false
        });
    }
};
NumarkScratch.ModeRoll.prototype = Object.create(components.ComponentContainer.prototype);

NumarkScratch.ModeSampler = function(deckNumber) {
    components.ComponentContainer.call(this);

    this.control = NumarkScratch.PadModeControls.SAMPLER;

    this.pads = new components.ComponentContainer();
    for (let i = 0; i < 4; i++) {
        this.pads[i] = new components.SamplerButton({
            midi: [0x93 + deckNumber, 0x14 + i],
            number: i + 1,
            shiftControl: true,
            sendShifted: true,
            shiftOffset: 0x08,
            outConnect: false
        });
    }
};
NumarkScratch.ModeSampler.prototype = Object.create(components.ComponentContainer.prototype);
