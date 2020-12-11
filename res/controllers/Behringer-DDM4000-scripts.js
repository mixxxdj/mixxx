/**
 * Mixxx controller mapping for a Behringer DDM4000 mixer.
 */

var DDM4000 = new components.extension.GenericMidiController({
    configurationProvider: function() {

        /* Shortcut variables */
        var c    = components;
        var e    = components.extension;
        var cc   = 0xB0;
        var note = 0x90;
        var toggle = c.Button.prototype.types.toggle;

        var CrossfaderAssignLED = function(options) {
            options = options || {};
            options.outKey = options.outKey || "orientation";
            e.CustomButton.call(this, options);
        };
        CrossfaderAssignLED.prototype = e.deriveFrom(e.CustomButton, {
            position: {
                left: 0,
                center: 1,
                right: 2
            }
        });
        var left = CrossfaderAssignLED.prototype.position.left;
        var center = CrossfaderAssignLED.prototype.position.center;
        var right = CrossfaderAssignLED.prototype.position.right;

        var CrossfaderUnit = function(options) {
            var unitOptions = options || {};
            unitOptions.group = unitOptions.group || "[Master]";
            c.ComponentContainer.call(this, unitOptions);

        var Crossfader = function(options) {
                options = options || {};
                options.inKey = options.inKey || options.key || "crossfader";
                options.group = options.group || unitOptions.group;
            c.Pot.call(this, options);
        };
        Crossfader.prototype = e.deriveFrom(c.Pot, {
                ignoreInput: function() {},
                enable: function() {
                    this.input = c.Pot.prototype.input;
            },
                disable: function() {
                    this.input = this.ignoreInput;
                    engine.setValue("[Master]", "crossfader_set_default", 1);
                },
        });
            var crossfader = new Crossfader(options.crossfader);

        var CrossfaderToggleButton = function(options) {
            options = options || {};
            if (options.type === undefined) {
                    options.type = c.Button.prototype.types.toggle;
            }
                if (!options.inKey && !options.key) {
                    options.key = "show_xfader";
                }
                options.group = options.group || unitOptions.group;
            c.Button.call(this, options);
        };
        CrossfaderToggleButton.prototype = e.deriveFrom(c.Button, {
            inSetValue: function(value) {
                    if (value) {
                        crossfader.enable();
                    } else {
                        crossfader.disable();
                    }
                    c.Button.prototype.inSetValue.call(this, value);
                },
            });
            this.crossfader = crossfader;
            this.button = new CrossfaderToggleButton(options.button);
        };
        CrossfaderUnit.prototype = e.deriveFrom(c.ComponentContainer);

        var SamplerBank = function(bankOptions) {
            c.ComponentContainer.call(this);
            var bank = this;

            var PlayButton = function(options) {
                options = options || {};
                options.key = options.key || "cue_gotoandplay";
                if (options.sendShifted === undefined) {
                    options.sendShifted = true;
                }
                c.Button.call(this, options);
            };
            PlayButton.prototype = e.deriveFrom(c.Button, {
                inSetValue: function(value) {
                    engine.setValue(this.group, this.inKey, value);
                    if (!value) {
                        engine.setValue(this.group, "cue_default", 1);
                    }
                },
            });
            this.playButton = new PlayButton({midi: bankOptions.play, group: bankOptions.group});

            var ReverseMode = function(options) {
                options = options || {};
                options.key = options.key || "reverse";
                c.Button.call(this, options);
            };
            ReverseMode.prototype = e.deriveFrom(c.Button);
            this.reverseMode = new ReverseMode({midi: bankOptions.reverse, group: bankOptions.group});

            var LoopMode = function(options) {
                options = options || {};
                options.key = options.key || "repeat";
                c.Button.call(this, options);
                this.inSetValue(true);
            };
            LoopMode.prototype = e.deriveFrom(c.Button, {
                outValueScale: function(value) {
                    var button = c.Button.prototype;
                    bank.playButton.type = value ? button.types.toggle : button.types.push;
                    return button.outValueScale(value);
                },
            });
            this.loopMode = new LoopMode({midi: bankOptions.loop, group: bankOptions.group});

            var ModeButton = function(options) {
                options = options || {};
                options.key = options.key || "mode";
                e.LongPressButton.call(this, options);
            };
            ModeButton.prototype = e.deriveFrom(e.LongPressButton, {
                onShortPress: function() {
                    this.setBlue(true);
                    bank.reverseMode.inToggle();
                },
                onLongPress: function() {
                    bank.reverseMode.inToggle();
                    bank.loopMode.inToggle();
                },
                onRelease: function() {
                    this.setBlue(false);
                },
                setBlue: function(value) {
                    midi.sendShortMsg(this.midi[0], this.midi[1] + 1, this.outValueScale(value));
                },
            });
            this.modeButton = new ModeButton(
                {midi: bankOptions.mode, group: bankOptions.group, longPressTimeout: 750});
        };
        SamplerBank.prototype = e.deriveFrom(c.ComponentContainer);

        return {
            init: function() {

                /*
                 * Prepare outgoing messages for the mixer's LED buttons.
                 * The buttons send `[note, ?]` and the LEDs respond to `[cc, ?]`.
                 * Enable LEDs by adding `sendShifted: true` to a button's `options` object.
                 */
                c.Button.prototype.shiftChannel = true;
                c.Button.prototype.shiftOffset = cc - note;
            },
            decks: [
                { // Channel 1
                    deckNumbers: [1],
                    components: [
                        {type: c.Pot, options: {midi: [cc, 0x07], inKey: "volume"}}, // Volume
                        { // CF Assign
                            type: e.EnumToggleButton,
                            options: {midi: [note, 0x20], inKey: "orientation", values: [left, right, center]},
                        },
                        {type: CrossfaderAssignLED, options: {midi: [cc, 0x20], onValue: left}}, // CF Assign A
                        {type: CrossfaderAssignLED, options: {midi: [cc, 0x21], onValue: right}}, // CF Assign B
                        { // PFL
                            type: c.Button, options: {
                                midi: [note, 0x3F], key: "pfl", type: toggle, sendShifted: true
                            }
                        },
                        {type: c.Button, options: {midi: [note, 0x03],  inKey: ""}}, // Mode
                        {type: c.Button, options: {midi: [cc,   0x38], outKey: ""}}, // Mode: Multi
                        {type: c.Button, options: {midi: [cc,   0x37], outKey: ""}}, // Mode: Single
                    ],
                    equalizerUnit: { //        P3 / Low,        P2 / Mid,        P1 / High
                        midi: { // eslint-disable-next-line key-spacing
                            parameterKnobs:   {1: [cc,   0x06], 2: [cc,   0x05], 3: [cc,   0x04]},
                            parameterButtons: {1: [note, 0x02], 2: [note, 0x01], 3: [note, 0x00]},
                        },
                        output: {
                            parameterButtons: {1: [cc,   0x3D], 2: [cc,   0x3B], 3: [cc,   0x39]}, // Amber
                            // parameterButtons: {1: [cc,   0x3E], 2: [cc,   0x3C], 3: [cc,   0x3A]}, // Green
                        },
                    },
                },
                { // Channel 2
                    deckNumbers: [2],
                    components: [
                        {type: c.Pot, options: {midi: [cc, 0x0B], inKey: "volume"}}, // Volume
                        { // CF Assign
                            type: e.EnumToggleButton,
                            options: {midi: [note, 0x22],  inKey: "orientation", values: [right, center, left]},
                        },
                        {type: CrossfaderAssignLED, options: {midi: [cc, 0x22], onValue: left}}, // CF Assign A
                        {type: CrossfaderAssignLED, options: {midi: [cc, 0x23], onValue: right}}, // CF Assign B
                        { // PFL
                            type: c.Button, options: {
                                midi: [note, 0x49], key: "pfl", type: toggle, sendShifted: true
                            }
                        },
                        {type: c.Button, options: {midi: [note, 0x07],  inKey: ""}}, // Mode
                        {type: c.Button, options: {midi: [cc,   0x42], outKey: ""}}, // Mode: Multi
                        {type: c.Button, options: {midi: [cc,   0x41], outKey: ""}}, // Mode: Single
                    ],
                    equalizerUnit: { //        P3 / Low,        P2 / Mid,        P1 / High
                        midi: { // eslint-disable-next-line key-spacing
                            parameterKnobs:   {1: [cc,   0x0A], 2: [cc,   0x09], 3: [cc,   0x08]},
                            parameterButtons: {1: [note, 0x06], 2: [note, 0x05], 3: [note, 0x04]},
                        },
                        output: {
                            parameterButtons: {1: [cc,   0x47], 2: [cc,   0x45], 3: [cc,   0x43]}, // Amber
                            // parameterButtons: {1: [cc,   0x48], 2: [cc,   0x46], 3: [cc,   0x44]}, // Green
                        },
                    },
                },
                { // Channel 3
                    deckNumbers: [3],
                    components: [
                        {type: c.Pot, options: {midi: [cc, 0x0F],  inKey: "volume"}}, // Volume
                        { // CF Assign
                            type: e.EnumToggleButton,
                            options: {midi: [note, 0x24],  inKey: "orientation", values: [left, right, center]}
                        },
                        {type: CrossfaderAssignLED, options: {midi: [cc, 0x24], onValue: left}}, // CF Assign A
                        {type: CrossfaderAssignLED, options: {midi: [cc, 0x25], onValue: right}}, // CF Assign B
                        { // PFL
                            type: c.Button, options: {
                                midi: [note, 0x53], key: "pfl", type: toggle, sendShifted: true
                            }
                        },
                        {type: c.Button, options: {midi: [note, 0x0B],  inKey: ""}}, // Mode
                        {type: c.Button, options: {midi: [cc,   0x4C], outKey: ""}}, // Mode: Multi
                        {type: c.Button, options: {midi: [cc,   0x4B], outKey: ""}}, // Mode: Single
                    ],
                    equalizerUnit: { //        P3 / Low,        P2 / Mid,        P1 / High
                        midi: { // eslint-disable-next-line key-spacing
                            parameterKnobs:   {1: [cc,   0x0E], 2: [cc,   0x0D], 3: [cc,   0x0C]},
                            parameterButtons: {1: [note, 0x0A], 2: [note, 0x09], 3: [note, 0x08]},
                        },
                        output: {
                            parameterButtons: {1: [cc,   0x51], 2: [cc,   0x4F], 3: [cc,   0x4D]}, // Amber
                            // parameterButtons: {1: [cc,   0x52], 2: [cc,   0x50], 3: [cc,   0x4E]}, // Green
                        },
                    },
                },
                { // Channel 4
                    deckNumbers: [4],
                    components: [
                        {type: c.Pot, options: {midi: [cc, 0x13], inKey: "volume"}}, // Volume
                        { // CF Assign
                            type: e.EnumToggleButton,
                            options: {midi: [note, 0x26],  inKey: "orientation", values: [right, center, left]}
                        },
                        {type: CrossfaderAssignLED, options: {midi: [cc, 0x26], onValue: left}}, // CF Assign A
                        {type: CrossfaderAssignLED, options: {midi: [cc, 0x27], onValue: right}}, // CF Assign B
                        { // PFL
                            type: c.Button, options: {
                                midi: [note, 0x5D], key: "pfl", type: toggle, sendShifted: true
                            }
                        },
                        {type: c.Button, options: {midi: [note, 0x0F],  inKey: ""}}, // Mode
                        {type: c.Button, options: {midi: [cc,   0x56], outKey: ""}}, // Mode: Multi
                        {type: c.Button, options: {midi: [cc,   0x55], outKey: ""}}, // Mode: Single
                    ],
                    equalizerUnit: { //        P3 / Low,        P2 / Mid,        P1 / High
                        midi: { // eslint-disable-next-line key-spacing
                            parameterKnobs:   {1: [cc,   0x12], 2: [cc,   0x11], 3: [cc,   0x10]},
                            parameterButtons: {1: [note, 0x0E], 2: [note, 0x0D], 3: [note, 0x0C]},
                        },
                        output: {
                            parameterButtons: {1: [cc,   0x5B], 2: [cc,   0x59], 3: [cc,   0x57]}, // Amber
                            // parameterButtons: {1: [cc,   0x5C], 2: [cc,   0x5A], 3: [cc,   0x58]}, // Green
                        },
                    },
                },
            ],
            containers: [
                { // Microphone
                    defaultDefinition: {type: c.Button, options: {group: "[Microphone]"}},
                    components: [
                        {options: {midi: [cc,   0x02], inKey: ""}, type: c.Pot}, // Mic: EQ Low
                        {options: {midi: [cc,   0x01], inKey: ""}, type: c.Pot}, // Mic: EQ Mid
                        {options: {midi: [cc,   0x00], inKey: ""}, type: c.Pot}, // Mic: EQ High
                        {options: {midi: [note, 0x31], key: "", sendShifted: true}}, // Mic: Setup
                        {options: {midi: [note, 0x32], key: "", sendShifted: true}}, // Mic: XMC On
                        {options: {midi: [note, 0x33], key: "", sendShifted: true}}, // Mic: FX On
                        {options: {midi: [note, 0x34], key: "talkover", sendShifted: true}}, // Mic: Talk On
                        {options: {midi: [note, 0x35], key: "", sendShifted: true}}, // Mic: On/Off
                    ]
                },
                { // Crossfader
                    defaultDefinition: {type: c.Button, options: {group: "[Mixer Profile]"}},
                    components: [
                        {options: {midi: [cc,   0x14]}, type: e.CrossfaderCurvePot}, // Crossfader: Curve
                        {options: {midi: [note, 0x28], key: "xFaderReverse", sendShifted: true}}, // Crossfader: Reverse Tap
                        {options: {midi: [note, 0x29], key: "xFaderReverse", type: toggle, sendShifted: true}}, // Crossfader: Reverse Hold
                    ]
                },
                { // Crossfader
                    defaultDefinition: {type: c.Button, options: {group: "[Master]"}},
                    components: [
                        {options: {midi: [note, 0x17],    key: "", sendShifted: true}}, // Crossfader: A Full Freq
                        {options: {midi: [note, 0x18],    key: "", sendShifted: true}}, // Crossfader: A High
                        {options: {midi: [note, 0x19],    key: "", sendShifted: true}}, // Crossfader: A Mid
                        {options: {midi: [note, 0x1A],    key: "", sendShifted: true}}, // Crossfader: A Low
                        {options: {midi: [note, 0x1B],    key: "", sendShifted: true}}, // Crossfader: B Full Freq
                        {options: {midi: [note, 0x1C],    key: "", sendShifted: true}}, // Crossfader: B High
                        {options: {midi: [note, 0x1D],    key: "", sendShifted: true}}, // Crossfader: B Mid
                        {options: {midi: [note, 0x1E],    key: "", sendShifted: true}}, // Crossfader: B Low
                        { // Crossfader: On
                            type: CrossfaderUnit,
                            options: {
                                button: {midi: [note, 0x1F], sendShifted: true},
                                crossfader: {midi: [cc, 0x15]}
                            },
                            components: [ // Idee: in registerComponents nicht über def schleifen, sondern über impl, und fehlende defs ignorieren.
                                {},
                                {},
                            ],
                        },
                        {options: {midi: [note, 0x2A],    key: "", sendShifted: true}}, // Crossfader: Bounce to MIDI Clock
                        {options: {midi: [note, 0x2B],  inKey: ""}}, // Crossfader: Beat (Left)
                        {options: {midi: [note, 0x2C],  inKey: ""}}, // Crossfader: Beat (Right)
                        {options: {midi: [cc,   0x2B], outKey: ""}}, // Crossfader: Beat 1
                        {options: {midi: [cc,   0x2C], outKey: ""}}, // Crossfader: Beat 2
                        {options: {midi: [cc,   0x2D], outKey: ""}}, // Crossfader: Beat 4
                        {options: {midi: [cc,   0x2E], outKey: ""}}, // Crossfader: Beat 8
                        {options: {midi: [cc,   0x2F], outKey: ""}}, // Crossfader: Beat 16
                    ]
                },
                { // Sampler
                    defaultDefinition: {type: c.Button, options: {group: "[Sampler1]"}},
                    components: [
                        {options: {midi: [cc,   0x03],  inKey: "volume"}, type: c.Pot}, // Sampler: Volume/Mix
                        {options: {midi: [note, 0x5F],    key: "", sendShifted: true}}, // Sampler: Insert
                        {options: {midi: [note, 0x60],  inKey: ""}}, // Sampler: REC Source (Right)
                        {options: {midi: [note, 0x61],  inKey: ""}}, // Sampler: REC Source (Left)
                        {options: {midi: [cc,   0x60], outKey: ""}}, // Sampler: REC Source 1
                        {options: {midi: [cc,   0x61], outKey: ""}}, // Sampler: REC Source 2
                        {options: {midi: [cc,   0x62], outKey: ""}}, // Sampler: REC Source 3
                        {options: {midi: [cc,   0x63], outKey: ""}}, // Sampler: REC Source 4
                        {options: {midi: [cc,   0x64], outKey: ""}}, // Sampler: REC Source Microphone
                        {options: {midi: [cc,   0x65], outKey: ""}}, // Sampler: REC Source Master
                        {options: {midi: [note, 0x66],    key: "pfl", sendShifted: true}}, // Sampler: PFL
                        {options: {midi: [note, 0x67],  inKey: ""}}, // Sampler: Sample Length (Right)
                        {options: {midi: [note, 0x68],  inKey: ""}}, // Sampler: Sample Length (Left)
                        {options: {midi: [cc,   0x67], outKey: ""}}, // Sampler: Sample Length 1
                        {options: {midi: [cc,   0x68], outKey: ""}}, // Sampler: Sample Length 2
                        {options: {midi: [cc,   0x69], outKey: ""}}, // Sampler: Sample Length 4
                        {options: {midi: [cc,   0x6A], outKey: ""}}, // Sampler: Sample Length 8
                        {options: {midi: [cc,   0x6B], outKey: ""}}, // Sampler: Sample Length 16
                        {options: {midi: [cc,   0x6C], outKey: ""}}, // Sampler: Sample Length ∞
                        {options: {midi: [note, 0x6D],    key: "", sendShifted: true}}, // Sampler: Record / In
                        {options: {midi: [note, 0x6C],  inKey: ""}}, // Sampler: Bank Assign
                        { // Sampler Bank 1
                            type: SamplerBank,
                            options: {
                                play: [note,  0x6E], // Sampler: Bank 1 Play / Out
                                reverse: [cc, 0x6F], // Sampler: Bank 1 Reverse
                                loop: [cc,    0x70], // Sampler: Bank 1 Loop
                                mode: [note,  0x71], // Sampler: Bank 1 Mode
                            },
                        },
                        { // Sampler Bank 2
                            type: SamplerBank,
                            options: {
                                group: "[Sampler2]",
                                play: [note,  0x73], // Sampler: Bank 2 Play / Out
                                reverse: [cc, 0x74], // Sampler: Bank 2 Reverse
                                loop: [cc,    0x75], // Sampler: Bank 2 Loop
                                mode: [note,  0x76], // Sampler: Bank 2 Mode
                            },
                        },
                        { // Sampler: FX On
                        /*
                         * When the sampler is in audio (non-midi) mode, this button causes a
                         * brake effect. Mixxx supports brake only for decks, not for samplers:
                         * - https://github.com/mixxxdj/mixxx/blob/a2866dfe9d9004e68610aa2d53220957954bfca3/src/controllers/engine/controllerengine.cpp#L1251
                         *   void ControllerEngineJSProxy::brake(int deck, bool activate, double factor = 1.0, double rate = 1.0)
                         * Thus we toggle effect unit 1 for the sampler instead.
                         */
                            options: {
                                midi: [note, 0x78], type: toggle, sendShifted: true,
                                group: "[EffectRack1_EffectUnit1]", key: "group_[Sampler1]_enable",
                            }
                        },
                        {options: {midi: [note, 0x79],    key: "", sendShifted: true}}, // Sampler: Select
                        { // Sampler: CF Assign
                            type: e.EnumToggleButton,
                            options: {midi: [note, 0x7A],  inKey: "orientation", values: [center, left, right]},
                        },
                        {type: CrossfaderAssignLED, options: {midi: [cc, 0x7A], onValue: left}}, // Sampler: CF Assign A
                        {type: CrossfaderAssignLED, options: {midi: [cc, 0x7B], onValue: right}}, // Sampler: CF Assign B
                        {options: {midi: [note, 0x7C],    key: "", sendShifted: true}}, // Sampler: CF Start
                    ]
                }
            ],
        };
    }
});

/* this statement exists to avoid linter errors */
DDM4000;
