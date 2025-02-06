// eslint-disable-next-line no-var
var NumarkScratch = {};

/*
 * USER CONFIGURABLE SETTINGS.
 * Change settings in the preferences
 */

// Defines the Beatloop Roll sizes for the 4 pads, for available values see:
// https://manual.mixxx.org/latest/en/chapters/appendix/mixxx_controls.html#control-[ChannelN]-beatlooproll_X_activate
// Default: [0.25, 0.5, 1, 2]
NumarkScratch.autoLoopSizes = [
    engine.getSetting("beatLoopRollsSize1") || 0.25,
    engine.getSetting("beatLoopRollsSize2") || 0.5,
    engine.getSetting("beatLoopRollsSize3") || 1,
    engine.getSetting("beatLoopRollsSize4") || 2,
];

// Defines how the Loop Encoder functions.
// If 'true' the Encoder scrolls the library/loads track. Shift + Encoder manages looping.
// If 'false' the Encoder manages looping. Shift + Encoder scrolls the library/loads track (Serato default).
// Default: false
NumarkScratch.invertLoopEncoderFunction = !!engine.getSetting("invertLoopEncoderFunction");

// Define whether or not to keep LEDs dimmed if they are inactive.
// 0x01 ('true' in UI) will keep them dimmed, 0x00 ('false' in UI) will turn them off. Default: 0x01 ('true')
NumarkScratch.noLight = 0x00;
NumarkScratch.dimLight = 0x01;
components.Button.prototype.off = engine.getSetting("inactiveLightsAlwaysBacklit") ? NumarkScratch.dimLight : NumarkScratch.noLight;

/*
 * CODE
 */

NumarkScratch.init = function() {
    // Initialize component containers
    NumarkScratch.deck = new components.ComponentContainer();
    NumarkScratch.effect = new components.ComponentContainer();
    for (let i = 0; i < 2; i++) {
        NumarkScratch.deck[i] = new NumarkScratch.Deck(i + 1);
        NumarkScratch.effect[i] = new NumarkScratch.EffectUnit(i + 1);
    }

    NumarkScratch.xfader = new NumarkScratch.XfaderContainer();

    const createVuCallback = deckOffset => value => midi.sendShortMsg(0xB0 + deckOffset, 0x1F, value * 90);

    engine.makeConnection("[Channel1]", "vu_meter", createVuCallback(0));
    engine.makeConnection("[Channel2]", "vu_meter", createVuCallback(1));

    // Trigger is needed to initialize lights
    NumarkScratch.deck.forEachComponent(component => component.trigger());
    NumarkScratch.effect.forEachComponent(component => component.trigger());

    NumarkScratch.shiftButton = new components.Button({
        midi: [0x9F, 0x32],
        input: function(channel, control, value) {
            if (this.isPress(channel, control, value)) {
                NumarkScratch.shift();
                this.send(this.on);
            } else {
                NumarkScratch.unshift();
                this.send(this.off);
            }
        },
        trigger: function() {
            this.send(this.off);
        }
    });

    NumarkScratch.shiftButton.trigger();

    // Send Serato SysEx messages to request initial state and unlock pads
    // Delay by 10sec for Mixxx to fully initialise (to accommodate channelInput)
    engine.beginTimer(10000, function() {
        midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x00, 0xF7], 6);
    }, true);

};

NumarkScratch.shutdown = function() {
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x04, 0xF7]);
};

NumarkScratch.shift = function() {
    NumarkScratch.deck.shift();
    NumarkScratch.effect.shift();
};

NumarkScratch.unshift = function() {
    NumarkScratch.deck.unshift();
    NumarkScratch.effect.unshift();
};

NumarkScratch.EffectUnit = function(deckNumber) {

    const inputUnshifted = function(channel, control, value, _status, _group) {
        if (!this.isPress(channel, control, value)) {
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

    this.effectButtons = [];
    for (let i = 0; i < 3; i++) {
        this.effectButtons[i] = new components.Button({
            group: `[EffectRack1_EffectUnit${deckNumber}_Effect${i + 1}]`,
            midi: [0x98 + deckNumber - 1, (deckNumber - 1) * 3 + i],
            inKey: "enabled",
            outKey: "enabled", // Bind the LED state to the effect enabled state
            shift: function() {
                this.input = components.Button.prototype.input;
                this.type = components.Button.prototype.types.toggle;
            },
            unshift: function() {
                this.input = inputUnshifted;
            },
        });
    }

    this.paddle = new components.Button({
        group: `[EffectRack1_EffectUnit${deckNumber}]`,
        inKey: `group_[Channel${deckNumber}]_enable`,
        inSetValue: function(value) {
            engine.setValue("[EffectRack1_EffectUnit1]", this.inKey, value);
            engine.setValue("[EffectRack1_EffectUnit2]", this.inKey, value);
        }
    });

    this.dryWetKnob = new components.Pot({
        group: `[EffectRack1_EffectUnit${deckNumber}]`,
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
        group: "[Master]",
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
            engine.setValue("[Mixer Profile]", "xFaderCalibration", 1);
            engine.setValue("[Mixer Profile]", "xFaderCurve", 999.6);
            break;
        }
    };

    this.xFaderReverse = new components.Button({
        group: "[Mixer Profile]",
        inKey: "xFaderReverse"
    });
};

NumarkScratch.XfaderContainer.prototype = new components.ComponentContainer();

NumarkScratch.setChannelInput = function(channel, control, value, _status, _group) {
    const number = (control === 0x57) ? 1 : 2;
    const channelgroup = `[Channel${number}]`;

    switch (value) {
    case 0x00:  // PC and turn on vinyl control
        engine.setValue(channelgroup, "passthrough", 0);
        engine.setValue(channelgroup, "vinylcontrol_enabled", 1);
        break;
    case 0x02:  // PHONO/LINE and turn off vinyl control
        engine.setValue(channelgroup, "passthrough", 1);
        engine.setValue(channelgroup, "vinylcontrol_enabled", 0);
        break;
    }
};

NumarkScratch.Deck = function(number) {
    components.Deck.call(this, number);

    const channel = number;

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
        group: `[EqualizerRack1_${this.currentDeck}_Effect1]`,
        inKey: "parameter3"
    });

    this.mid = new components.Pot({
        group: `[EqualizerRack1_${this.currentDeck}_Effect1]`,
        inKey: "parameter2"
    });

    this.bass = new components.Pot({
        group: `[EqualizerRack1_${this.currentDeck}_Effect1]`,
        inKey: "parameter1"
    });

    this.filter = new components.Pot({
        group: `[QuickEffectRack1_${this.currentDeck}]`,
        inKey: "super1"
    });

    this.gain = new components.Pot({
        inKey: "pregain"
    });

    const encoderInvert = function(key) {
        return NumarkScratch.invertLoopEncoderFunction ? (key === "unshift" ? "shift" : "unshift") : key;
    };

    this.loopPlus = new components.Encoder({
        midi: [0x93 + channel, 0x35],
        [encoderInvert("unshift")]: function() {
            this.group = `[Channel${ channel}]`;
            this.inKey = "loop_double";
        },
        [encoderInvert("shift")]: function() {
            this.group = "[Library]";
            this.inKey = "MoveDown";
        }
    });

    this.loopMinus = new components.Encoder({
        midi: [0x93 + channel, 0x34],
        [encoderInvert("unshift")]: function() {
            this.group = `[Channel${ channel}]`;
            this.inKey = "loop_halve";
        },
        [encoderInvert("shift")]: function() {
            this.group = "[Library]";
            this.inKey = "MoveUp";
        }
    });

    this.loopButton = new components.Button({
        midi: [0x93 + channel, 0x3F],
        [encoderInvert("unshift")]: function() {
            this.group = `[Channel${ channel}]`;
            this.inKey = "beatloop_activate";
        },
        [encoderInvert("shift")]: function() {
            this.group = `[Channel${ channel}]`;
            this.inKey = "LoadSelectedTrack";
        }
    });

    this.padSection = new NumarkScratch.PadSection(number);

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
            group: `[Channel${deckNumber}]`,
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
            group: `[Channel${deckNumber}]`,
            midi: [0x93 + deckNumber, 0x14 + i],
            size: NumarkScratch.autoLoopSizes[i],
            shiftControl: true,
            sendShifted: true,
            shiftOffset: 0x08,
            inKey: `beatlooproll_${NumarkScratch.autoLoopSizes[i]}_activate`,
            outKey: `beatloop_${NumarkScratch.autoLoopSizes[i]}_enabled`,
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
