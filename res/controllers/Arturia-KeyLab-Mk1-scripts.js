"use strict";

// eslint-disable-next-line no-var
var KeyLabMk1;
(function(KeyLabMk1) {
    // https://manual.mixxx.org/2.5/en/chapters/appendix/mixxx_controls#control-[ChannelN]-key
    const keyNums = [
        "C",
        "Db",
        "D",
        "Eb",
        "E",
        "F",
        "F#/Gb",
        "G",
        "Ab",
        "A",
        "Bb",
        "B",
        "Cm",
        "C#m",
        "Dm",
        "D#m/Ebm",
        "Em",
        "Fm",
        "F#m",
        "Gm",
        "G#m",
        "Am",
        "Bbm",
        "Bm",
    ];

    // https://manual.mixxx.org/2.5/en/chapters/appendix/mixxx_controls#control-[Library]-sort_column
    const sortCols = [
        "Artist",
        "Title",
        "Album",
        "Album Artist",
        "Year",
        "Genre",
        "Composer",
        "Grouping",
        "Tracknumber",
        "Filetype",
        "Native Location",
        "Comment",
        "Duration",
        "Bitrate",
        "BPM",
        "Replay Gain",
        "Datetime Added",
        "Times Played",
        "Rating",
        "Key",
        "Preview",
        "Cover Art",
        "Position",
        "Play ID",
        "Location",
        "Filename",
        "Modified Time",
        "Creation Time",
        "Sample Rate",
        "Track Color",
        "Last Played",
    ];

    // Menu items that can be controlled with the parameter and value encoders.
    const menu = [{
        name: "Key",
        rotate: {
            inKey: "key",
            max: 24,
            outValueScale: (value) => {
                return keyNums[script.posMod(value - 1, keyNums.length)];
            },
        },
        press: {
            inKey: "reset_key",
        },
    }, {
        name: "Beatloop",
        rotate: {
            inKey: "beatloop_size",
        },
        press: {
            inKey: "beatloop_activate",
        },
    }, {
        name: "Beatjump",
        rotate: {
            inKey: "beatjump_size",
        },
    }, {
        name: "Library",
        params: [{
            name: "Search",
            rotate: {
                group: "[Library]",
                relative: true,
                inKey: "search_history_selector",
            },
            press: {
                group: "[Library]",
                inKey: "clear_search",
            },
        }, {
            name: "Focus",
            rotate: {
                min: 0,
                max: 6,
                group: "[Library]",
                inKey: "focused_widget",
            },
        }, {
            name: "Scroll",
            rotate: {
                group: "[Library]",
                relative: true,
                inKey: "MoveVertical",
            },
            press: {
                group: "[Library]",
                inKey: "GoToItem",
            },
        }, {
            name: "Sort",
            rotate: {
                group: "[Library]",
                min: script.LIBRARY_COLUMNS.ARTIST,
                max: script.LIBRARY_COLUMNS.LAST_PLAYED,
                inKey: "sort_column_toggle",
                outValueScale: (value) => {
                    return sortCols[(((value - 1) % sortCols.length) + sortCols.length) % sortCols.length];
                },
            },
            press: {
                group: "[Library]",
                type: components.Button.prototype.types.toggle,
                inKey: "sort_order",
            },
        }, {
            name: "Maximize",
            press: {
                group: "[Skin]",
                type: components.Button.prototype.types.toggle,
                inKey: "show_maximized_library",
            },
        }, {
            name: "Font Size",
            rotate: {
                group: "[Library]",
                relative: true,
                inKey: "font_size_knob",
            },
        }],
    }, {
        name: "Skin",
        params: [{
            name: "Effect Rack",
            press: {
                group: "[Skin]",
                type: components.Button.prototype.types.toggle,
                inKey: "show_effectrack",
            },
        }, {
            name: "Coverart",
            press: {
                group: "[Skin]",
                type: components.Button.prototype.types.toggle,
                inKey: "show_library_coverart",
            },
        }, {
            name: "Samplers",
            press: {
                group: "[Skin]",
                type: components.Button.prototype.types.toggle,
                inKey: "show_samplers",
            },
        }, {
            name: "Vinyl Control",
            press: {
                group: "[Skin]",
                type: components.Button.prototype.types.toggle,
                inKey: "show_vinylcontrol",
            },
        }],
    }];

    // Add FX sub-menus to parameters.
    for (let i = 0; i < 4; i++) {
        menu.push({
            name: `FX${i + 1}`,
            params: [{
                name: `FX${i + 1} Preset`,
                rotate: {
                    group: `[EffectRack1_EffectUnit${i + 1}]`,
                    inKey: "chain_preset_selector",
                    relative: true,
                },
                press: {
                    group: `[EffectRack1_EffectUnit${i + 1}]`,
                    inKey: "group_[Channel1]_enable",
                    type: components.Button.prototype.types.toggle,
                },
            }, {
                name: `FX${i + 1} Focus`,
                rotate: {
                    group: `[EffectRack1_EffectUnit${i + 1}]`,
                    min: 1,
                    max: engine.getValue(`[EffectRack1_EffectUnit${i + 1}]`, "num_effectslots"),
                    inKey: "focused_effect",
                },
                press: {
                    group: `[EffectRack1_EffectUnit${i + 1}]`,
                    type: components.Button.prototype.types.toggle,
                    inKey: "show_focus",
                },
            }],
        });
    }

    // Sysex messages for toggling various LEDs on the controller.
    // See https://legacy-forum.arturia.com/index.php?topic=90496.0
    const SysexHeader = [0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42];
    const MMCHeader = [0xF0, 0x7F, 0x7F];

    const sendSysex = function(payload) {
        const sysexMessage = SysexHeader.concat(payload, [0xF7]);
        midi.sendSysexMsg(sysexMessage, sysexMessage.length);
    };

    const setLED = function(num, val) {
        const on = val ? LedState.on : LedState.off;
        sendSysex([0x02, 0x00, 0x10, 0x00, 0x00].concat(num, on));
    };

    const delayLED = function(led) {
        return (value) => {
            engine.beginTimer(engine.getSetting("output_delay"), () => {
                setLED(led, Boolean(value));
            }, true);
        };
    };

    // Write a message to the keylab's display.
    // To write a message at all you'll need firmware 1.33 or later, so it's not
    // recommended that these messages be depended upon.
    const writeDisplay = function(msg) {
        // flatMap and flat don't appear to exist or we could just map \n to '00
        // 02'.
        const lines = msg.split("\n");
        let sysexPayload = [0x04, 0x00, 0x60, 0x01].concat(lines[0].toInt(), 0x00);
        if (lines.length === 2) {
            sysexPayload = sysexPayload.concat(0x02, lines[1].toInt(), 0x00);
        } else if (lines.length >= 2) {
            console.log(`KeyLab display overflow, got ${lines.length} lines want 2`);
        }
        engine.beginTimer(engine.getSetting("output_delay"), () => {
            sendSysex(sysexPayload);
        }, true);
    };

    /*
     * Represent a pad button that interact with a intro/extra special markers
     * (set, activate, clear)
     */
    class IntroOutroButton extends components.Button {
        constructor(options) {
            super(options);
            if (this.cueBaseName === undefined || typeof this.cueBaseName !== "string") {
                throw Error("must specify cueBaseName as intro_start, intro_end, outro_start, or outro_end");
            }
            this.outKey = `${this.cueBaseName}_enabled`;
            this.output = delayLED(this.midi[1] + 0x4C);
        }
        unshift() {
            this.inKey = `${this.cueBaseName}_activate`;
        }
        shift() {
            this.inKey = `${this.cueBaseName}_clear`;
        }
    }

    class Deck extends components.Deck {
        constructor(deckNumbers, midiChannel) {
            super(deckNumbers);

            // Transport buttons
            this.playButton = new components.PlayButton({
                group: "[Channel1]",
                midi: [0xB0 + midiChannel, 0x02],
                type: components.Button.prototype.types.push,
                output: delayLED(0x58),
            });
            this.stopButton = new components.CueButton({
                group: "[Channel1]",
                midi: [0xB0 + midiChannel, 0x01],
                type: components.Button.prototype.types.push,
                output: delayLED(0x59),
            });
            this.loopButton = new components.LoopToggleButton({
                group: "[Channel1]",
                midi: [0xB0 + midiChannel, 0x37],
                inKey: "reloop_toggle",
                outKey: "loop_enabled",
                output: delayLED(0x5D),
            });
            this.backButton = new components.Button({
                group: "[Channel1]",
                midi: [0xB0 + midiChannel, 0x05],
                key: "beatjump_backward",
                output: delayLED(0x5B),
            });
            this.forwardButton = new components.Button({
                group: "[Channel1]",
                midi: [0xB0 + midiChannel, 0x04],
                key: "beatjump_forward",
                output: delayLED(0x5C),
            });

            // Snapshot buttons and pads
            this.padGrid = new PadGrid();

            // Parameters
            const softTakeover = engine.getSetting("soft_takeover");
            this.pregainPot = new components.Pot({
                group: "[Channel1]",
                midi: [0xB0 + midiChannel, 0x4A],
                key: "pregain",
                softTakeover: softTakeover,
            });
            this.highFilterGainPot = new components.Pot({
                group: "[EqualizerRack1_[Channel1]_Effect1]",
                midi: [0xB0 + midiChannel, 0x47],
                key: "parameter3",
                softTakeover: softTakeover,
            });
            this.midFilterGainPot = new components.Pot({
                group: "[EqualizerRack1_[Channel1]_Effect1]",
                midi: [0xB0 + midiChannel, 0x4C],
                key: "parameter2",
                softTakeover: softTakeover,
            });
            this.lowFilterGainPot = new components.Pot({
                group: "[EqualizerRack1_[Channel1]_Effect1]",
                midi: [0xB0 + midiChannel, 0x4D],
                key: "parameter1",
                softTakeover: softTakeover,
            });
            this.quickEffectPot = new components.Pot({
                group: "[QuickEffectRack1_[Channel1]]",
                midi: [0xB0 + midiChannel, 0x5D],
                key: "super1",
                softTakeover: softTakeover,
            });

            // Other encoders
            this.valueEncoder = new components.Encoder({
                group: "[Channel1]",
                midi: [0xB0 + midiChannel, 0x72], // Value encoder
                input: function(_channel, _control, value, _status, _group) {
                    if (!this.inKey) {
                        // If we haven't selected a parameter yet, don't do
                        // anything.
                        return;
                    }
                    const param = this.inGetParameter();

                    if (value & 0x40) {
                        if (this.relative) {
                            this.inSetParameter(1);
                        } else {
                            const newParam = param + 1;
                            if (this.max !== undefined && this.min !== undefined && newParam > this.max) {
                                this.inSetParameter(this.min);
                            } else {
                                this.inSetParameter(newParam);
                            }
                        }
                    } else {
                        if (this.relative) {
                            this.inSetParameter(-1);
                        } else {
                            const newParam = param - 1;
                            if (this.max !== undefined && this.min !== undefined && newParam < this.min) {
                                this.inSetParameter(this.max);
                            } else {
                                this.inSetParameter(newParam);
                            }
                        }
                    }
                    this.output(this.inGetParameter());
                },
                output: function(value) {
                    if (this.outValueScale) {
                        const newVal = this.outValueScale(value);
                        writeDisplay(`${this.name}: ${newVal}`);
                    }
                },
            });

            this.valueButton = new components.Button({
                group: "[Channel1]",
                midi: [0xB0, 0x73], // Value encoder push
                encoder: this.valueEncoder,
                activeDeck: this,
                inGetValue: function() {
                    if (!this.inKey) {
                        return 0;
                    }
                    return engine.getValue(this.group, this.inKey.replace("[Channel1]", this.activeDeck.currentDeck));
                },
                inSetValue: function(value) {
                    if (!this.inKey) {
                        return;
                    }
                    engine.setValue(this.group, this.inKey.replace("[Channel1]", this.activeDeck.currentDeck), value);
                    this.output(this.encoder.inGetParameter());
                },
                output: function(value) {
                    if (this.outValueScale) {
                        const newVal = this.outValueScale(value);
                        writeDisplay(`${this.name}: ${newVal}`);
                    }
                },
            });
        }
    }

    // DeckButton is a power window button that toggles between two decks and
    // blinks if a track is ending on either of them.
    class DeckButton extends components.Button {
        constructor(decks, activeDeck, options) {
            options.type = components.Button.prototype.types.powerWindow;
            options.outKey = "end_of_track";
            options.decks = decks;
            options.activeDeck = activeDeck;
            super(options);

            this.blinking = 0;
        }

        inToggle() {
            if (this.isLongPressed) {
                this.activeDeck.setCurrentDeck(`[Channel${this.decks[1]}]`);
            } else {
                this.activeDeck.setCurrentDeck(`[Channel${this.decks[0]}]`);
            }
            this.output(0x1F);
        }

        blink(value, group, _control) {
            if (this.blinking) {
                engine.stopTimer(this.blinking);
                this.blinking = 0;
            }
            if (value === 0) {
                if (typeof this.noWriteOutput === "function") {
                    this.noWriteOutput(
                        this.activeDeck.currentDeck === "[Channel1]" || this.activeDeck.currentDeck === "[Channel3]",
                    );
                }
                return;
            }

            let blinkState = false;
            let velocity = 500;
            if (this.group === group) {
                velocity = 250;
            }
            this.blinking = engine.beginTimer(velocity, () => {
                blinkState = !blinkState;
                setLED(this.midi[2], blinkState);
            });
        }

        connect() {
            this.connections[0] = engine.makeConnection(
                `[Channel${this.decks[0]}]`,
                this.outKey,
                this.blink.bind(this),
            );
            this.connections[1] = engine.makeConnection(
                `[Channel${this.decks[1]}]`,
                this.outKey,
                this.blink.bind(this),
            );
        }
    }

    class PadGrid extends components.ComponentContainer {
        // The drum pads by default are laid out in a grid starting from the
        // bottom left (0x24), then going right and up.
        // This makes the very first pad (what we're calling pad '0') in the top
        // left 0x30, so do some math to invert the pads and get us to the
        // correct values going right and down instead by subtracting 8 times
        // the row the button is on.
        padNum(n) {
            return n + 0x30 - (Math.floor(n / 4) * 8);
        }

        // TODO: This is copied exactly from the parent class. However, if I
        // don't do this the Object.assign doesn't appear to work. Why?
        // Something about the value of "this"?
        applyLayer(newLayer, reconnectComponents) {
            if (reconnectComponents !== false) {
                reconnectComponents = true;
            }
            if (reconnectComponents === true) {
                this.forEachComponent(function(component) {
                    component.disconnect();
                });
            }

            Object.assign(this, newLayer);

            if (reconnectComponents === true) {
                this.forEachComponent(function(component) {
                    component.connect();
                    component.trigger();
                });
            }
        }

        switchLayer(layerNum, pads) {
            this.applyLayer({pads: pads}, true);
            this.radioGroup.forEach((btn, idx) => {
                btn.output(idx === layerNum ? LedState.on : LedState.off);
            });
        }

        defaultLayer(group) {
            const pads = [
                new IntroOutroButton({
                    group: group,
                    cueBaseName: "intro_start",
                    midi: [0x99, this.padNum(0)],
                }),
                new IntroOutroButton({
                    group: group,
                    cueBaseName: "intro_end",
                    midi: [0x99, this.padNum(1)],
                }),
                new IntroOutroButton({
                    group: group,
                    cueBaseName: "outro_start",
                    midi: [0x99, this.padNum(2)],
                }),
                new IntroOutroButton({
                    group: group,
                    cueBaseName: "outro_end",
                    midi: [0x99, this.padNum(3)],
                }),
            ];
            for (let i = 0; i < 12; i++) {
                pads[i + 4] = new components.HotcueButton({
                    group: group,
                    number: i + 1,
                    midi: [0x99, this.padNum(i + 4)],
                    blinkState: false,
                    blinkTimer: 0,
                    output: function(value) {
                        if (value === 2) {
                            if (this.blinkTimer === 0) {
                                this.blinkTimer = engine.beginTimer(250, () => {
                                    this.blinkState = !this.blinkState;
                                    delayLED(this.midi[1] + 0x4C)(this.blinkState ? 1 : 0);
                                });
                            }
                        } else {
                            if (this.blinkTimer) {
                                engine.stopTimer(this.blinkTimer);
                                this.blinkTimer = 0;
                            }
                            delayLED(this.midi[1] + 0x4C)(value);
                        }
                    },
                });
            }
            this.switchLayer(0, pads);
        }

        hotcueLayer(group) {
            const pads = [];
            for (let i = 0; i < 16; i++) {
                pads[i] = new components.HotcueButton({
                    group: group,
                    number: i + 1,
                    midi: [0x99, this.padNum(i)],
                    blinkState: false,
                    blinkTimer: 0,
                    output: function(value) {
                        if (value === 2) {
                            if (this.blinkTimer === 0) {
                                this.blinkTimer = engine.beginTimer(250, () => {
                                    this.blinkState = !this.blinkState;
                                    delayLED(this.midi[1] + 0x4C)(this.blinkState ? 1 : 0);
                                });
                            }
                        } else {
                            if (this.blinkTimer) {
                                engine.stopTimer(this.blinkTimer);
                                this.blinkTimer = 0;
                            }
                            delayLED(this.midi[1] + 0x4C)(value);
                        }
                    },
                });
            }
            this.switchLayer(1, pads);
        }

        samplerLayer() {
            const pads = [];
            for (let i = 0; i < 16; i++) {
                pads[i] = new components.SamplerButton({
                    group: `[Sampler${i + 1}]`,
                    number: i + 1,
                    volumeByVelocity: true,
                    midi: [0x99, this.padNum(i)],
                    // These numbers are made up magic constants that can be
                    // checked in the "send" callback defined below.
                    playing: 0x72,
                    empty: 0x00,
                    loaded: 0x01,
                    blinking: 0,
                    blinkState: false,
                    send: function(value) {
                        switch (value) {
                        case this.empty: {
                            if (this.blinking) {
                                engine.stopTimer(this.blinking);
                                this.blinking = 0;
                            }
                            delayLED(this.midi[1] + 0x4c)(LedState.off);
                            break;
                        }
                        case this.loaded: {
                            if (this.blinking) {
                                engine.stopTimer(this.blinking);
                                this.blinking = 0;
                            }
                            delayLED(this.midi[1] + 0x4c)(LedState.on);
                            break;
                        }
                        case this.playing: {
                            // Start the sampler blinking every 500ms if it's
                            // playing.
                            // If for some reason it's already blinking, do
                            // nothing so we don't end up with multiple blink
                            // timers, one of which we forget to stop.
                            if (this.blinking === 0) {
                                this.blinking = engine.beginTimer(250, () => {
                                    this.blinkState = !this.blinkState;
                                    setLED(this.midi[1] + 0x4c, this.blinkState);
                                });
                            }
                            break;
                        }
                        }
                    },
                });
            }
            this.switchLayer(2, pads);
        }

        disabledLayer() {
            for (let i = 0; i < 16; i++) {
                delayLED(this.padNum(i) + 0x4c)(LedState.off);
            }
            this.switchLayer(undefined, Array(16).fill(new components.Button()));
        }

        input(channel, control, value, status, group) {
            const padNum = this.padNum(control);
            if (this.pads !== undefined && this.pads[padNum] !== undefined) {
                return this.pads[padNum].input(channel, control, value, status, group);
            } else {
                console.log(`pad num ${padNum} not enabled`);
            }
        }

        constructor() {
            super({});

            this.radioGroup = [
                new components.Button({
                    group: "[Channel1]",
                    midi: [0xB0, 0x16],
                    padGrid: this,
                    input: function(_channel, _control, value, _status, group) {
                        if (value) {
                            // If we get an on and an off toggle, only change the
                            // value once.
                            return;
                        }
                        this.padGrid.defaultLayer(group);
                    },
                    output: delayLED(0x12),
                }),
                new components.Button({
                    group: "[Channel1]",
                    midi: [0xB0, 0x17],
                    padGrid: this,
                    input: function(_channel, _control, value, _status, group) {
                        if (value) {
                            // If we get an on and an off toggle, only change the
                            // value once.
                            return;
                        }
                        this.padGrid.hotcueLayer(group);
                    },
                    output: delayLED(0x13),
                }),
                new components.Button({
                    group: "[Channel1]",
                    midi: [0xB0, 0x18],
                    padGrid: this,
                    input: function(_channel, _control, value, _status, _group) {
                        if (value) {
                            // If we get an on and an off toggle, only change the
                            // value once.
                            return;
                        }
                        this.padGrid.samplerLayer();
                    },
                    output: delayLED(0x14),
                }),
            ];

            if (engine.getSetting("pads_disabled")) {
                this.disabledLayer();
            } else {
                const defaultPadLayout = engine.getSetting("defaultPadLayout");
                switch (defaultPadLayout) {
                case "default":
                    this.defaultLayer("[Channel1]");
                    break;
                case "hotcue":
                    this.hotcueLayer("[Channel1]");
                    break;
                case "sampler":
                    this.samplerLayer();
                    break;
                default:
                    throw Error(`invalid default pad layout "${defaultPadLayout}"`);
                }
            }
        }
    }

    class Controller extends components.ComponentContainer {
        setDeckOutput(noWrite = false) {
            return (value) => {
                setLED(0x1E, Boolean(value));
                setLED(0x1F, !value);
                if (!noWrite) {
                    const currentDeckNum = script.deckFromGroup(this.activeDeck.currentDeck);
                    writeDisplay(`Deck ${currentDeckNum}`);
                }
            };
        }

        constructor(id) {
            super({});

            this.id = id;
            this.activeDeck = new Deck([1, 2, 3, 4], 1);
            this.deck1Button = new DeckButton([1, 3], this.activeDeck, {
                group: "[Channel1]",
                midi: [0xB0, 0x76, 0x1E], // "Sound"
                noWriteOutput: this.setDeckOutput(true),
                output: this.setDeckOutput(),
            });
            this.deck2Button = new DeckButton([2, 4], this.activeDeck, {
                group: "[Channel2]",
                midi: [0xB0, 0x77, 0x1F], // "Multi"
                noWriteOutput: this.setDeckOutput(true),
                output: this.setDeckOutput(),
            });

            // Deck parameter selector.
            this.paramEncoder = new components.Encoder({
                group: "[Channel1]",
                midi: [0xB0, 0x70], // Param encoder
                selectedParam: 0,
                params: menu,
                activeDeck: this.activeDeck,
                input: function(_channel, _control, value, _status, _group) {
                    if (value === 0x3F) {
                        this.selectedParam = script.posMod(this.selectedParam - 1, this.params.length);
                    } else if (value === 0x41) {
                        this.selectedParam = script.posMod(this.selectedParam + 1, this.params.length);
                    }
                    const param = this.params[this.selectedParam];

                    // Assign defaults to the value encoder and button, and
                    // deactivate them.
                    Object.assign(this.activeDeck.valueEncoder, {
                        name: param.name,
                        group: this.activeDeck.currentDeck,
                        relative: false,
                        key: undefined,
                        inKey: undefined,
                        outKey: undefined,
                        outValueScale: undefined,
                        min: undefined,
                        max: undefined,
                    });
                    Object.assign(this.activeDeck.valueButton, {
                        name: param.name,
                        group: this.activeDeck.currentDeck,
                        type: components.Button.prototype.types.push,
                        key: undefined,
                        inKey: undefined,
                        outKey: undefined,
                        outValueScale: undefined,
                        min: undefined,
                        max: undefined,
                    });

                    // If the current menu has new actions for the buttons,
                    // assign them.
                    if (param.rotate) {
                        Object.assign(this.activeDeck.valueEncoder, param.rotate);
                    }
                    if (param.press) {
                        Object.assign(this.activeDeck.valueButton, param.press);
                    }

                    writeDisplay(`${param.name}`);
                },
            });
            this.paramButton = new components.Button({
                group: "[Channel1]",
                midi: [0xB0, 0x71], // Param encoder button
                encoder: this.paramEncoder,
                prevSelected: 0,
                prevParams: [],
                input: function(channel, control, value, status, group) {
                    if (value) {
                        return;
                    }
                    const param = this.encoder.params[this.encoder.selectedParam];
                    if (param.params) {
                        // We're entering a submenu so set the new menu to the submenu
                        // and set old menu to that submenus back menu.
                        this.prevParams = this.encoder.params;
                        this.prevSelected = this.encoder.selectedParam;
                        this.encoder.params = param.params;
                        // Always add a back button to the end of sub-menus.
                        this.encoder.params.push({name: "Back"});
                        if (param.selectedParam === undefined) {
                            param.selectedParam = 0;
                        }
                        this.encoder.selectedParam = param.selectedParam;
                        this.encoder.input(channel, control, 0, status, group);
                    } else if (param.name === "Back") {
                        // We hit the back button, go up a level.
                        this.encoder.params = this.prevParams;
                        this.encoder.selectedParam = this.prevSelected;
                        this.encoder.input(channel, control, 0, status, group);
                    }
                },
            });
            this.recordButton = new components.Button({
                group: "[Recording]",
                midi: [0xB0, 0x06],
                inKey: "toggle_recording",
                outKey: "status",
                output: delayLED(0x5A),
            });
            const softTakeover = engine.getSetting("soft_takeover");
            // Faders
            this.faders = new Array(9);
            this.faders[0] = new components.Pot({
                group: "[Channel3]",
                midi: [0xB0, 0x49],
                key: "volume",
                softTakeover: softTakeover,
            });
            this.faders[1] = new components.Pot({
                group: "[Channel1]",
                midi: [0xB0, 0x4B],
                key: "volume",
                softTakeover: softTakeover,
            });
            this.faders[2] = new components.Pot({
                group: "[Channel2]",
                midi: [0xB0, 0x4F],
                key: "volume",
                softTakeover: softTakeover,
            });
            this.faders[3] = new components.Pot({
                group: "[Channel4]",
                midi: [0xB0, 0x48],
                key: "volume",
                softTakeover: softTakeover,
            });
            this.faders[4] = new components.Pot({
                group: "[Channel3]",
                midi: [0xB0, 0x50],
                key: "rate",
                softTakeover: softTakeover,
            });
            this.faders[5] = new components.Pot({
                group: "[Channel1]",
                midi: [0xB0, 0x51],
                key: "rate",
                softTakeover: softTakeover,
            });
            this.faders[6] = new components.Pot({
                group: "[Channel2]",
                midi: [0xB0, 0x52],
                key: "rate",
                softTakeover: softTakeover,
            });
            this.faders[7] = new components.Pot({
                group: "[Channel4]",
                midi: [0xB0, 0x53],
                key: "rate",
                softTakeover: softTakeover,
            });
            this.faders[8] = new components.Pot({
                group: "[Master]",
                midi: [0xB0, 0x55],
                key: "crossfader",
                softTakeover: softTakeover,
            });
            // Pots
            this.mainGain = new components.Pot({
                group: "[Master]",
                midi: [0xB0, 0x07],
                key: "gain",
                softTakeover: softTakeover,
            });
            this.headGain = new components.Pot({
                group: "[Master]",
                midi: [0xB0, 0x00],
                key: "headGain",
                softTakeover: softTakeover,
            });
        }
    }

    KeyLabMk1.init = function(id, _debugging) {
        KeyLabMk1.controller = new Controller(id);
        writeDisplay("Welcome to Mixxx");
    };

    KeyLabMk1.shutdown = function() {
        KeyLabMk1.controller.shutdown();
        writeDisplay("Goodbye!");
    };

    KeyLabMk1.incomingData = function(data, length) {
        if (length < 6) {
            console.log(`expected sysex packet of length 6, got ${length}`);
            return;
        }
        for (let n = 0; n < MMCHeader.length; n++) {
            if (data[n] !== MMCHeader[n]) {
                console.log(`unknown sysex packet: ${data}`);
                return;
            }
        }
        // We only handle MMC commands at the moment: ignore responses, device
        // control, MIDI show control, etc.
        if (data[3] !== 0x06) {
            console.log(`unexpected MMC Sub-ID#1: ${data[3]}`);
            return;
        }
        if (data[length - 1] !== 0xF7) {
            console.log("sysex packet missing trailer");
            return;
        }
        switch (data[4]) {
        case 0x01: // stop
            KeyLabMk1.controller.activeDeck.stopButton.inSetValue(0x7f);
            break;
        case 0x02: // play
            KeyLabMk1.controller.activeDeck.playButton.inToggle();
            break;
        case 0x04: // fast forward
            KeyLabMk1.controller.activeDeck.forwardButton.inSetValue(0x7f);
            break;
        case 0x05: // rewind
            KeyLabMk1.controller.activeDeck.backButton.inSetValue(0x7f);
            break;
        case 0x06: // record
            KeyLabMk1.controller.recordButton.inSetValue(0x7f);
            break;
        case 0x37: // loop (I believe this one is non-standard)
            KeyLabMk1.controller.activeDeck.loopButton.inToggle();
            break;
        default:
            console.log(`unrecognized MMC command: ${data[4]}`);
        }
    };
})(KeyLabMk1 || (KeyLabMk1 = {}));

// vim:expandtab:shiftwidth=4:tabstop=4:backupcopy=yes
