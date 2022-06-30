var NumarkScratch = {};

/////////////////
// Tweakables. //
/////////////////

// Beatloop Roll sizes for the 4 pads, for available values see:
// https://manual.mixxx.org/2.3/en/chapters/appendix/mixxx_controls.html#control-[ChannelN]-beatloop_X_toggle
NumarkScratch.autoLoopSizes = [
    "0.25",
    "0.5",
    "1",
    "2",
];

// Default is false. When set to false, the EncoderKnob manages looping. Shift + EncoderKnob scrolls the library/loads track
// If set to true, the EncoderKnob scrolls the library/loads track. Shift + EncoderKnob manages looping.
NumarkScratch.invertLoopEncoderFunction = false;

// Turn off all when inactive instead default is 0x00 (Off)
// Set to 0x01 is you want lights to be dim
NumarkScratch.LOW_LIGHT = 0x00;

///////////
// Code. //
///////////

// State variable, don't touch
NumarkScratch.shifted = false;

components.Button.prototype.off = NumarkScratch.LOW_LIGHT;

NumarkScratch.init = function() {
    // Initialize component containers
    NumarkScratch.deck = new components.ComponentContainer();
    NumarkScratch.effect = new components.ComponentContainer();
    for (var i = 0; i < 2; i++) {
        NumarkScratch.deck[i] = new NumarkScratch.Deck(i + 1);
        NumarkScratch.effect[i] = new NumarkScratch.EffectUnit(i + 1);
    }

    NumarkScratch.gains = new NumarkScratch.Gains();

    //Toggle the vinyl control section in skins to show
    engine.setValue("[VinylControl]", "show_vinylcontrol", 1);

    // Send Serato SysEx messages to request initial state and unlock pads
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x00, 0xF7], 6);
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x01, 0xF7], 6);

    engine.makeConnection("[Channel1]", "VuMeter", NumarkScratch.vuCallback);
    engine.makeConnection("[Channel2]", "VuMeter", NumarkScratch.vuCallback);

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
};

NumarkScratch.shutdown = function() {
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x04, 0xF7], 6);
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

NumarkScratch.allEffectOff = function() {
    NumarkScratch.effect[0].effects=[false, false, false];
    NumarkScratch.effect[1].effects=[false, false, false];
    NumarkScratch.FxUpdateLEDs();
    NumarkScratch.effect[0].updateEffects();
    NumarkScratch.effect[1].updateEffects();
};

NumarkScratch.FxUpdateLEDs = function() {
    var newStates1=[false, false, false];
    var newStates2=[false, false, false];
    newStates1=NumarkScratch.effect[0].effects;
    newStates2=NumarkScratch.effect[1].effects;
    midi.sendShortMsg(0x98, 0x00, newStates1[0] ? 0x7F:NumarkScratch.LOW_LIGHT);
    midi.sendShortMsg(0x98, 0x01, newStates1[1] ? 0x7F:NumarkScratch.LOW_LIGHT);
    midi.sendShortMsg(0x98, 0x02, newStates1[2] ? 0x7F:NumarkScratch.LOW_LIGHT);
    midi.sendShortMsg(0x99, 0x03, newStates2[0] ? 0x7F:NumarkScratch.LOW_LIGHT);
    midi.sendShortMsg(0x99, 0x04, newStates2[1] ? 0x7F:NumarkScratch.LOW_LIGHT);
    midi.sendShortMsg(0x99, 0x05, newStates2[2] ? 0x7F:NumarkScratch.LOW_LIGHT);
};
// TODO in 2.3 it is not possible to "properly" map the FX selection buttons.
// this should be done with load_preset and QuickEffects instead (when effect
// chain preset saving/loading is available in Mixxx)
NumarkScratch.EffectUnit = function(deckNumber) {
    this.effects = [false, false, false];
    this.isSwitchHoldOn = false;

    this.updateEffects = function() {
        for (var i = 1; i <= this.effects.length; i++) {
            engine.setValue("[EffectRack1_EffectUnit" + deckNumber + "_Effect"+i+"]", "enabled", this.effects[i-1]);
        }
    };

    // switch values are:
    // 0 - switch in the middle
    // 1 - switch up
    // 2 - switch down
    this.enableSwitch = function(channel, control, value, _status, _group) {
        this.isSwitchHoldOn = value !== 0;
        engine.setValue("[EffectRack1_EffectUnit1]", "super1", Math.min(value, 1.0));
        engine.setValue("[EffectRack1_EffectUnit2]", "super1", Math.min(value, 1.0));
        engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel" + deckNumber + "]_enable", (value !== 0));
        engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel" + deckNumber + "]_enable", (value !== 0));
        this.updateEffects();
    };

    this.dryWetKnob = new components.Pot({
        group: "[EffectRack1_EffectUnit" + deckNumber + "]",
        inKey: "mix"
    });

    this.effect1 = function(channel, control, value, status, _group) {
        if (value === 0x7F) {
            if (!NumarkScratch.shifted) {
                NumarkScratch.allEffectOff();
            }
            this.effects[0] = !this.effects[0];
            midi.sendShortMsg(status, control, this.effects[0] ? 0x7F : NumarkScratch.LOW_LIGHT);
        }

        this.updateEffects();
    };

    this.effect2 = function(channel, control, value, status, _group) {
        if (value === 0x7F) {
            if (!NumarkScratch.shifted) {
                NumarkScratch.allEffectOff();
            }
            this.effects[1] = !this.effects[1];
            midi.sendShortMsg(status, control, this.effects[1] ? 0x7F : NumarkScratch.LOW_LIGHT);
        }

        this.updateEffects();
    };

    this.effect3 = function(channel, control, value, status, _group) {
        if (value === 0x7F) {
            if (!NumarkScratch.shifted) {
                NumarkScratch.allEffectOff();
            }
            this.effects[2] = !this.effects[2];
            midi.sendShortMsg(status, control, this.effects[2] ? 0x7F : NumarkScratch.LOW_LIGHT);
        }
        this.updateEffects();
    };
};
NumarkScratch.EffectUnit.prototype = new components.ComponentContainer();

NumarkScratch.crossfader = new components.Pot({
    midi: [0xBF, 0x08],
    group: "[Master]",
    inKey: "crossfader",
});

NumarkScratch.crossfader.setCurve = function(channel, control, value, _status, _group) {
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
    }
};

NumarkScratch.crossfader.xFaderReverse = function(channel, control, value, _status, _group) {
    // 0x7F is ON, 0x00 is OFF
    engine.setValue("[Mixer Profile]", "xFaderReverse", (value === 0x7F) ? 1 : 0);
};

NumarkScratch.setChannelInput = function(channel, control, value, _status, _group) {
    var number = (control === 0x57) ? 1 : 2;
    var channelgroup = "[Channel" + number + "]";

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

    var channel = number - 1;
    //var deck = this;

    this.pflButton = new components.Button({
        midi: [0x90 + channel, 0x1B],
        key: "pfl",
        output: function(value) {
            var note = (value === 0x00 ? 0x80 : 0x90) + channel;
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

    this.loopEncoder = new components.Encoder({
        unshift: function() {
            if (!NumarkScratch.invertLoopEncoderFunction) { //if set to false then normal function
                this.input = function(channel, control, _value, _status, _group) {
                    var loopSize = engine.getValue(this.group, "beatloop_size");
                    if (control === 52 && loopSize >= (1/16)) { //turn left
                        engine.setValue(this.group, "beatloop_size", loopSize / 2);
                    } else if (control === 53 && loopSize < 256) { // turn right
                        engine.setValue(this.group, "beatloop_size", loopSize * 2);
                    }
                };
            } else {
                this.input = function(channel, control, _value, _status, _group) { // if set to true invert function
                    var direction = (control === 52) ? "Down" : "Up"; //turn left: turn right
                    script.triggerControl("[Library]", "Move" + direction);
                };
            }
        },
        shift: function() {
            if (!NumarkScratch.invertLoopEncoderFunction) { //if set to false then normal function
                this.input = function(channel, control, _value, _status, _group) {
                    var direction = (control === 52) ? "Down" : "Up"; //turn left: turn right
                    script.triggerControl("[Library]", "Move" + direction);
                };
            } else {
                this.input = function(channel, control, _value, _status, _group) { // if set to true invert function
                    var loopSize = engine.getValue(this.group, "beatloop_size");
                    if (control === 52 && loopSize >= (1/16)) { //turn left
                        engine.setValue(this.group, "beatloop_size", loopSize / 2);
                    } else if (control === 53 && loopSize < 256) { //turn  right
                        engine.setValue(this.group, "beatloop_size", loopSize * 2);
                    }
                };
            }
        },
    });

    this.loopEncoderButton = new components.Button({
        shift: function() {
            if (!NumarkScratch.invertLoopEncoderFunction) { //if set to false then normal function
                this.inKey = "LoadSelectedTrack";
            } else {
                this.inKey = "beatloop_activate"; // if set to true invert function
            }
        },
        unshift: function() {
            if (!NumarkScratch.invertLoopEncoderFunction) { //if set to false then normal function
                this.inKey = "beatloop_activate";
            } else {
                this.inKey = "LoadSelectedTrack"; // if set to true invert function
            }
        }
    });

    this.padSection = new NumarkScratch.PadSection(number);

    this.shiftButton = new components.Button({
        input: function(channel, control, value) {
            if (this.isPress(channel, control, value)) {
                NumarkScratch.shift();
                midi.sendShortMsg(0x9F, 0x32, 0x7F);
            } else {
                NumarkScratch.unshift();
                midi.sendShortMsg(0x9F, 0x32, NumarkScratch.LOW_LIGHT);
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
        var i = (control - 0x14) % 8;
        this.currentMode.pads[i].input(channel, control, value, status, group);
    };

    this.setMode = function(_channel, control) {
        var newMode = this.modes[control];
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
    for (var i = 0; i < 4; i++) {
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
    for (var i = 0; i < 4; i++) {
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
    for (var i = 0; i < 4; i++) {
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

NumarkScratch.Gains = function() {
    this.cueMix = new components.Pot({
        group: "[Master]",
        inKey: "headMix"
    });
};
NumarkScratch.Gains.prototype = new components.ComponentContainer();

NumarkScratch.vuCallback = function(value, group) {
    var level = value * 90;
    var deckOffset = script.deckFromGroup(group) - 1;
    midi.sendShortMsg(0xB0 + deckOffset, 0x1F, level);
};
