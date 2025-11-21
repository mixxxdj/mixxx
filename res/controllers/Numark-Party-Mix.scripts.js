var NumarkPartyMix = {};

// The jogwheel is behaving inconsistently
// when scratching the point in the song moves
// this is according to my theory because
// the movements are not properly registered by the encoder
// (this can happen when the sampling rate of the sensor is too low)
NumarkPartyMix.jogScratchSensitivity = 340;
NumarkPartyMix.jogScratchAlpha = 1 / 8; // do NOT set to 2 or higher
NumarkPartyMix.jogScratchBeta = 1 / 8 / 32;
NumarkPartyMix.jogPitchSensitivity = 10;
NumarkPartyMix.jogSearchSensitivity = 1 / 2;

// autoloop sizes, for available values see:
// https://manual.mixxx.org/2.3/en/chapters/appendix/mixxx_controls.html#control-[ChannelN]-beatloop_X_toggle
NumarkPartyMix.autoLoopSizes = [
    "4",
    "8",
    "16",
    "32"
];

// dim all lights when inactive instead of turning them off
components.Button.prototype.off = 0x01;

components.Button.prototype.shutdown = function() {
    if (this.midi === undefined || this.midi[0] === undefined || this.midi[1] === undefined) {
        return;
    }
    // LEDs are switched off using node off messages
    // on shutdown LEDs are not dimmed but switched off
    midi.sendShortMsg(this.midi[0] - 0x10, this.midi[1], 0x00);
};

// pad modes control codes
NumarkPartyMix.PadModeControls = {
    HOTCUE: 0x00,
    LOOP: 0x0B,
    SAMPLER: 0x0E,
    EFX: 0x18,
};

NumarkPartyMix.init = function(_id, _debugging) {
    /// init party led switch off
    midi.sendShortMsg(0xb0, 0x40, 0x00);//0x60 seems to be max
    midi.sendShortMsg(0xb0, 0x42, 0x00);
    midi.sendShortMsg(0xb0, 0x43, 0x00);

    // initialize component containers
    NumarkPartyMix.deck = new components.ComponentContainer();
    for (var i = 0; i < 2; i++) {
        NumarkPartyMix.deck[i] = new NumarkPartyMix.Deck(i + 1);
    }

    NumarkPartyMix.browse = new NumarkPartyMix.Browse();
    NumarkPartyMix.gains = new NumarkPartyMix.Gains();

    // 0x00 0x01 0x3F is Numark mfg. ID used in SysEx messages.
    midi.sendSysexMsg([0xF0, 0x00, 0x01, 0x3F, 0x38, 0x48, 0xF7]);
};

NumarkPartyMix.shutdown = function() {
    NumarkPartyMix.deck.shutdown();
};

NumarkPartyMix.Deck = function(deckNumber) {
    components.Deck.call(this, deckNumber);

    var channel = deckNumber - 1;
    var deck = this;
    this.scratchModeEnabled = false;

    this.playButton = new components.PlayButton({
        midi: [0x90 + channel, 0x00],
    });

    this.cueButton = new components.CueButton({
        midi: [0x90 + channel, 0x01],
    });

    this.syncButton = new components.SyncButton({
        midi: [0x90 + channel, 0x02],
    });

    this.headphoneButton = new components.Button({
        midi: [0x90 + channel, 0x1B],
        key: "pfl",
        output: function(value) {
            var note = (value === 0x00 ? 0x80 : 0x90) + channel;
            midi.sendShortMsg(note, 0x1B, this.outValueScale(value));
        }
    });

    this.loadButton = new components.Button({
        midi: [0x9F, 0x01 + channel],
        inKey: "LoadSelectedTrack"
    });

    this.volume = new components.Pot({
        inKey: "volume"
    });

    this.gain = new components.Pot({
        inKey: "pregain"
    });

    this.treble = new components.Pot({
        group: "[EqualizerRack1_" + this.currentDeck + "_Effect1]",
        inKey: "parameter3"
    });

    this.bass = new components.Pot({
        group: "[EqualizerRack1_" + this.currentDeck + "_Effect1]",
        inKey: "parameter1"
    });

    this.pitch = new components.Pot({
        inKey: "rate",
        invert: false
    });

    this.padSection = new NumarkPartyMix.PadSection(deckNumber);

    this.scratchToggle = new components.Button({
        midi: [0x90 + channel, 0x07],
        type: components.Button.prototype.types.toggle,
        inToggle: function() {
            deck.scratchModeEnabled = !deck.scratchModeEnabled;
            if (deck.scratchModeEnabled) {
                this.send(this.on);
            } else {
                engine.scratchDisable(deckNumber);
                this.send(this.off);
            }
        }
    });

    this.wheelTurn = new components.Encoder({
        group: "[Channel" + deckNumber + "]",
        key: "wheelTurn",
        input: function(channel, _control, value, _status, group) {
            // Calculate direction (Standard Mixxx logic)
            // Clockwise = positive, Counter-Clockwise = negative
            var newValue = (value < 0x40) ? value : (value - 0x80);

            // LOGIC FROM "DARKPOUBELLE" ADAPTED FOR THIS SCRIPT:
            
            if (deck.scratchModeEnabled) {
                // --- SCRATCH MODE ---
                // If the scratch button is ON, we scratch.
                if (!engine.isScratching(deckNumber)) {
                    // Enable scratch using the sensitivity settings defined at the top of the file
                    engine.scratchEnable(deckNumber, NumarkPartyMix.jogScratchSensitivity, 33 + 1 / 3, NumarkPartyMix.jogScratchAlpha, NumarkPartyMix.jogScratchBeta, true);
                }
                engine.scratchTick(deckNumber, newValue); 
            } else {
                // --- PITCH BEND MODE ---
                // Previously, the PADS script divided this value (newValue / 10), 
                // which made it so slow it felt broken.
                // It now sends the raw value, just like the "DARKPOUBELLE" script does.
                engine.setValue(group, "jog", newValue); 
            }
        }
    });

    this.reconnectComponents(function(component) {
        if (component.group === undefined) {
            component.group = this.currentDeck;
        }
    });
};

NumarkPartyMix.Deck.prototype = new components.Deck();

NumarkPartyMix.PadSection = function(deckNumber) {
    components.ComponentContainer.call(this);

    // initialize leds
    midi.sendShortMsg(0x93 + deckNumber, 0x00, components.Button.prototype.on);

    this.modes = {};
    this.modes[NumarkPartyMix.PadModeControls.HOTCUE] = new NumarkPartyMix.ModeHotcue(deckNumber);
    this.modes[NumarkPartyMix.PadModeControls.LOOP] = new NumarkPartyMix.ModeLoop(deckNumber);
    this.modes[NumarkPartyMix.PadModeControls.SAMPLER] = new NumarkPartyMix.ModeSampler(deckNumber);
    this.modes[NumarkPartyMix.PadModeControls.EFX] = new NumarkPartyMix.ModeEFX(deckNumber);

    this.modeButtonPress = function(_channel, control, _value) {
        this.setMode(control);
    };

    this.padPress = function(channel, control, value, status, group) {
        var i = (control - 0x14) % 8;
        this.currentMode.connections[i].input(channel, control, value, status, group);
    };

    this.setMode = function(control) {
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

    this.currentMode = this.modes[NumarkPartyMix.PadModeControls.HOTCUE];
};
NumarkPartyMix.PadSection.prototype = Object.create(components.ComponentContainer.prototype);


NumarkPartyMix.ModeHotcue = function(deckNumber) {
    components.ComponentContainer.call(this);

    this.control = NumarkPartyMix.PadModeControls.HOTCUE;

    this.connections = new components.ComponentContainer();
    for (var i = 0; i < 4; i++) {
        this.connections[i] = new components.HotcueButton({
            group: "[Channel" + deckNumber + "]",
            midi: [0x93 + deckNumber, 0x14 + i],
            number: i + 1,
            outConnect: false
        });
    }
};
NumarkPartyMix.ModeHotcue.prototype = Object.create(components.ComponentContainer.prototype);

NumarkPartyMix.ModeLoop = function(deckNumber) {
    components.ComponentContainer.call(this);

    this.control = NumarkPartyMix.PadModeControls.AUTOLOOP;

    this.connections = new components.ComponentContainer();
    for (var i = 0; i < 4; i++) {
        this.connections[i] = new components.Button({
            group: "[Channel" + deckNumber + "]",
            midi: [0x93 + deckNumber, 0x14 + i],
            size: NumarkPartyMix.autoLoopSizes[i],
            inKey: "beatloop_" + NumarkPartyMix.autoLoopSizes[i] + "_toggle",
            outKey: "beatloop_" + NumarkPartyMix.autoLoopSizes[i] + "_enabled",
            outConnect: false
        });
    }
};
NumarkPartyMix.ModeLoop.prototype = Object.create(components.ComponentContainer.prototype);

// this device doesn't have a shift button,
// therefore we do not want it to depend on
// a shift switch to stop a sample
NumarkPartyMix.ModeSampler = function(deckNumber) {
    components.ComponentContainer.call(this);
    this.control = NumarkPartyMix.PadModeControls.SAMPLER;
    var sampleoffset = (deckNumber - 1) * 4;
    this.connections = new components.ComponentContainer();
    for (var i = 0; i < 4; i++) {
        this.connections[i] = new components.SamplerButton({
            midi: [0x93 + deckNumber, 0x14 + i],
            number: 1 + i + sampleoffset,
            outConnect: false,
            unshift: null,
            outKey: "play_indicator",
            input: function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status)) {
                    if (engine.getValue(this.group, "track_loaded") === 0) {
                        engine.setValue(this.group, "LoadSelectedTrack", 1);
                    } else {
                        if (engine.getValue(this.group, "play") === 1) {
                            engine.setValue(this.group, "start_stop", 1);
                        } else {
                            engine.setValue(this.group, "start_play", 1);
                        }
                    }
                }
            }
        });
    }
};
NumarkPartyMix.ModeSampler.prototype = Object.create(components.ComponentContainer.prototype);

NumarkPartyMix.ModeEFX = function(deckNumber) {
    components.ComponentContainer.call(this);

    this.control = NumarkPartyMix.PadModeControls.EFX;

    var fx = [
        "[EffectRack1_EffectUnit" + deckNumber + "_Effect1]",
        "[EffectRack1_EffectUnit" + deckNumber + "_Effect2]",
        "[EffectRack1_EffectUnit" + deckNumber + "_Effect3]"
    ];

    this.connections = new components.ComponentContainer();
    var p = this.connections;

    fx.forEach(function(fx, i) {
        p[i] = new components.Button({
            midi: [0x93 + deckNumber, 0x14 + i],
            group: fx,
            key: "enabled",
            outConnect: false
        });
    });
    p[3] = new components.Button({
        midi: [0x93 + deckNumber, 0x17],
        group: "[EffectRack1_EffectUnit" + deckNumber + "]",
        key: "mix_mode",
        type: components.Button.prototype.types.toggle,
        outValueScale: function(val) {
            return val === 0 ? 0x00 : 0x7F;
        },
        outConnect: false
    });
};
NumarkPartyMix.ModeEFX.prototype = Object.create(components.ComponentContainer.prototype);

NumarkPartyMix.Browse = function() {
    components.ComponentContainer.call(this);

    var browse = this;
    this.knobpressed = false;

    this.knob = new components.Encoder({
        input: function(channel, control, value) {
            var direction;
            if (browse.knobpressed) {
                if (value > 0x40) {
                    engine.setParameter("[Library]", "MoveFocusForward", 1);
                } else {
                    engine.setParameter("[Library]", "MoveFocusBackward", 1);
                }
            } else {
                direction = (value > 0x40) ? -1 : 1;
                engine.setParameter("[Library]", "MoveVertical", direction);
            }
        }
    });

    this.knobButton = new components.Button({
        group: "[Library]",
        type: components.Button.prototype.types.powerWindow,
        isLongPressed: false,
        input: function(channel, control, value, status) {
            browse.knobpressed = this.isPress(channel, control, value, status);
            if (browse.knobpressed) {
                this.isLongPressed = false;
                this.longPressTimer = engine.beginTimer(this.longPressTimeout, function() {
                    this.isLongPressed = true;
                    this.longPressTimer = 0;
                }, true);
            } else {
                if (!this.isLongPressed) {
                    this.inToggle();
                }
                if (this.longPressTimer !== 0) {
                    engine.stopTimer(this.longPressTimer);
                    this.longPressTimer = 0;
                }
                this.isLongPressed = false;
            }
        },
        inToggle: function() {
            engine.setParameter("[Library]", "GoToItem", 1);
        }
    });
};
NumarkPartyMix.Browse.prototype = Object.create(components.ComponentContainer.prototype);

NumarkPartyMix.Gains = function() {
    this.mainGain = new components.Pot({
        group: "[Master]",
        inKey: "gain"
    });

    this.cueGain = new components.Pot({
        group: "[Master]",
        inKey: "headGain"
    });

    this.cueMix = new components.Pot({
        group: "[Master]",
        inKey: "headMix"
    });
};
NumarkPartyMix.Gains.prototype = Object.create(components.ComponentContainer.prototype);
