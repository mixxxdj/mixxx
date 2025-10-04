/*

DDJ-200 Midi Overview:
Deck n = 0/1:                                   Midi in         Midi out
                                                (to PC)         (from PC)
    Jog (Platter)   rotate      Vinyl mode on   Bn  22      hh      -           result = value - 64
                                Vinyl mode off  Bn  23      hh      -           result = value - 64
                                shift           Bn  29      hh      -           result = value - 64
    Jog (Platter)   touch                       9n  36      hh      -           OFF=0x00, ON=0x7F
                                shift           9n  67      hh      -           OFF=0x00, ON=0x7F
    Jog (Side)      rotate      normal/shift    Bn  21      hh      -           result = value - 64
    Beat Sync       press                       9n  58      hh  [midi in]       OFF=0x00, ON=0x7F
                    long press                  9n  5C      hh      -           OFF=0x00, ON=0x7F
                    press       shift           9n  60      hh  [midi in]       OFF=0x00, ON=0x7F
    Tempo           slide       normal/shift    Bn  00/20   xx      -           MSB / LSB
    Play/Pause      press                       9n  0B  hh      [midi in]       OFF=0x00, ON=0x7F
                    press       shift           9n  47  hh      [midi in]       OFF=0x00, ON=0x7F
    Cue             press                       9n  0C  hh      [midi in]       OFF=0x00, ON=0x7F
                    press       shift           9n  48  hh      [midi in]       OFF=0x00, ON=0x7F
    Shift           press                       9n  3F  hh          -           OFF=0x00, ON=0x7F
    EQ Hi           rotate                      Bn  07/27   xx      -           MSB / LSB
    EQ Mid          rotate                      Bn  0B/2B   xx      -           MSB / LSB
    EQ Low          rotate                      Bn  0F/2F   xx      -           MSB / LSB
    Color Fx CH1    rotate                      B6  17/37   xx      -           MSB / LSB
    Color Fx CH2    rotate                      B6  18/38   xx      -           MSB / LSB
    Master Cue      press                       96  63      hh  [midi in]       OFF=0x00, ON=0x7F
                    press       shift           96  78      hh  [midi in]       OFF=0x00, ON=0x7F
    Cue (Headpho)   press                       9n  54      hh  [midi in]       OFF=0x00, ON=0x7F
                    press       shift           9n  68      hh  [midi in]       OFF=0x00, ON=0x7F
    Ch Fader        slide                       Bn  13/33   xx      -           MSB / LSB
    Crossfader      slide                       B6  1F/3F   xx      -           MSB / LSB
    both above      zero->notzero   shift       9n  66  hh      [midi in]       OFF=0x00, ON=0x7F (PLAY message for fader start)
                    notzero->zero   shift       9n  52  hh      [midi in]       OFF=0x00, ON=0x7F (CUE message for fader start)
    TransitionFX    press                       96  59  hh      [midi in]       OFF=0x00, ON=0x7F
                    press           shift       96  5A  hh      [midi in]       OFF=0x00, ON=0x7F

    Deck1 PAD p=7, Deck1 SHIFT PAD p=8, Deck2 PAD p=9, Deck2 SHIFT PAD p=A
    PAD 1           press                       9p  00  hh      [midi in]       OFF=0x00, ON=0x7F
    PAD 2           press                       9p  01  hh      [midi in]       OFF=0x00, ON=0x7F
    PAD 3           press                       9p  02  hh      [midi in]       OFF=0x00, ON=0x7F
    PAD 4           press                       9p  03  hh      [midi in]       OFF=0x00, ON=0x7F
    PAD 5           press                       9p  04  hh      [midi in]       OFF=0x00, ON=0x7F
    PAD 6           press                       9p  05  hh      [midi in]       OFF=0x00, ON=0x7F
    PAD 7           press                       9p  06  hh      [midi in]       OFF=0x00, ON=0x7F
    PAD 8           press                       9p  07  hh      [midi in]       OFF=0x00, ON=0x7F

    Loaded Deck1                                none            9F 00 7F        blinking PADs 1-4 followed by PADs 5-8
    Loaded Deck2                                none            9F 01 7F        blinking PADs 1-4 followed by PADs 5-8
    Viny Mode on/off                            none            9n 17 hh        OFF=0x00, ON=0x7F (default ON)
*/

// eslint-disable-next-line no-var
var DDJ200 = { };

components.Component.prototype.shiftOffset = 1;
components.Component.prototype.shiftChannel = true;
components.Component.prototype.sendShifted = true;

DDJ200.PadMode = class extends components.ComponentContainer {
    constructor(options) {
        super(options);
        // this is a workaround for components, forEachComponent only iterates
        // over ownProperties, so these have to constructed by the constructor here
        // instead of being merged by the ComponentContainer constructor
        this.pads = Array(8).fill(undefined);
    }
    constructPads(constructPad) {
        this.pads = this.pads.map((_, padIndex) => constructPad(padIndex));
    }
};

DDJ200.PadModeContainers = {
    Hotcue: class extends DDJ200.PadMode {
        constructor(deckOffset) {
            super();

            super.constructPads(i => new components.HotcueButton({
                midi: [0x97 + deckOffset, i],
                number: i + 1,
            })
            );

            this.indicatorStyle = 0;
        }
    },
    Loop: class extends DDJ200.PadMode {
        constructor(deckOffset) {
            super();

            const theContainer = this;
            this.currentBaseLoopSize = parseInt(engine.getSetting("defaultLoopRootSize"));

            super.constructPads(i => new components.Component({
                midi: [0x97 + deckOffset, i],
                unshift: function() {
                    this.inKey = "beatloop_activate";
                },
                shift: function() {
                    this.inKey = "beatlooproll_activate";
                },
                on: 0x7F,
                off: 0x00,
                loopSize: Math.pow(2, theContainer.currentBaseLoopSize + i),
                input: function(_channel, _control, value, _status, _mode) {
                    if (value) {
                        const loopEnabled = engine.getValue(this.group, "loop_enabled");
                        const matchingLoopSize = (engine.getValue(this.group, "beatloop_size") === this.loopSize);
                        engine.setValue(this.group, "beatloop_size", this.loopSize);
                        if (matchingLoopSize || !loopEnabled) {
                            engine.setValue(this.group, this.inKey, true);
                        };
                    };
                },
                outKey: null, // hack to get Component constructor to call connect()
                connect: function() {
                    this.connections[0] = engine.makeConnection(this.group, "beatloop_size", this.output.bind(this));
                    this.connections[1] = engine.makeConnection(this.group, "loop_enabled", this.output.bind(this));
                },
                outValueScale: function(value) {
                    if (value) {
                        const loopEnabled = engine.getValue(this.group, "loop_enabled");
                        const matchingLoopSize = (engine.getValue(this.group, "beatloop_size") === this.loopSize);
                        return (matchingLoopSize && loopEnabled) ? this.on : this.off;
                    }
                    return this.off;
                },
            })
            );
            this.indicatorStyle = 1;
        }
    },
    Effect: class extends DDJ200.PadMode {
        constructor(deckOffset, group) {
            super();

            super.constructPads(i => {
                if (i < 4) {
                    return new components.SamplerButton({
                        midi: [0x97 + deckOffset, i],
                        number: deckOffset * 2 + i + 1,
                    });
                } else if (i < 7) {
                    return new components.Button({
                        midi: [0x97 + deckOffset, i],
                        group: `[EffectRack1_EffectUnit${script.deckFromGroup(group)}_Effect${i - 3}]`,
                        inKey: "enabled",
                        outKey: "loaded",
                        type: components.Button.prototype.types.toggle,
                        unshift: function() {
                            if (this.connections) {
                                this.disconnect();
                                this.group = this.group.replace("EffectUnit3", "EffectUnit1").replace("EffectUnit4", "EffectUnit2");
                                this.connect();
                                this.trigger();
                            }
                        },
                        shift: function() {
                            this.disconnect();
                            this.group = this.group.replace("EffectUnit1", "EffectUnit3").replace("EffectUnit2", "EffectUnit4");
                            this.connect();
                            this.trigger();
                        },
                    });
                } else {
                    return new components.Button({
                        midi: [0x97 + deckOffset, i],
                        indicateButtonPress: function(value) {
                            this.output(value, null, null);
                        },
                        inSetValue: function(value) {
                            if (value) {
                                bpm.tapButton(script.deckFromGroup(this.group));
                            };
                            this.indicateButtonPress(value);
                        },
                    });
                };
            });
            this.indicatorStyle = "indicator_500ms";
        }
    },
    Jump: class extends DDJ200.PadMode {
        constructor(deckOffset) {
            super();

            const theContainer = this;
            this.currentBaseBeatJumpSize = parseInt(engine.getSetting("defaultBeatJumpRootSize"));

            super.constructPads(i => new components.Component({
                midi: [0x97 + deckOffset, i],
                unshift: function() {
                    this.inKey = "beatjump_forward";
                },
                shift: function() {
                    this.inKey = "beatjump_backward";
                },
                on: 0x7F,
                off: 0x00,
                beatJumpSize: Math.pow(2, theContainer.currentBaseBeatJumpSize + i),
                input: function(_channel, _control, value, _status, _mode) {
                    engine.setValue(this.group, "beatjump_size", this.beatJumpSize);
                    engine.setValue(this.group, this.inKey, value);
                },
                outKey: null, // hack to get Component constructor to call connect()
                connect: function() {
                    this.connections[0] = engine.makeConnection(this.group, "beatjump_forward", this.output.bind(this));
                    this.connections[1] = engine.makeConnection(this.group, "beatjump_backward", this.output.bind(this));
                },
                outValueScale: function(value) {
                    if (value) {
                        const matchingJumpSize = (engine.getValue(this.group, "beatjump_size") === this.beatJumpSize);
                        return (matchingJumpSize) ? this.on : this.off;
                    }
                    return this.off;
                },
            })
            );
            this.indicatorStyle = "indicator_250ms";
        }
    },
};

DDJ200.DoubleRingBuffer = class {
    constructor(indexableUnshifted, indexableShifted) {
        this.indexable = [indexableUnshifted, indexableShifted];
        this.index = 0;
        this.isShifted = false;
    };
    swapIndexable() {
        this.isShifted = !this.isShifted;
        this.index = 0;
    }
    current() {
        return this.indexable[this.isShifted?1:0][this.index];
    };
    next() {
        this.index = script.posMod(this.index + 1, this.indexable[this.isShifted?1:0].length);
    };
};

DDJ200.PadModeContainers.ModeSelector = function(group) {
    const deckOffset = (((script.deckFromGroup(group) - 1) % 2) * 2);

    const startupModeInstance = new DDJ200.PadModeContainers.Hotcue(deckOffset);

    const padInstancesBuffer = new DDJ200.DoubleRingBuffer(
        [startupModeInstance, new DDJ200.PadModeContainers.Loop(deckOffset), new DDJ200.PadModeContainers.Effect(deckOffset, group)],
        [new DDJ200.PadModeContainers.Jump(deckOffset)]
    );

    const connectPadInstance = function(padInstance) {
        padInstance.forEachComponent(function(component) {
            if (component.group === undefined) {
                component.group = group;
            }
            component.connect();
            component.trigger();
        });
    };

    let choosenPadInstance = startupModeInstance;
    connectPadInstance(choosenPadInstance);

    const setPads = newPadInstance => {
        if (newPadInstance === choosenPadInstance) {
            return;
        }
        choosenPadInstance.forEachComponent(function(component) {
            component.disconnect();
        });
        choosenPadInstance = newPadInstance;
        connectPadInstance(choosenPadInstance);
    };

    this.changePadMode = (isShifted) => {
        if (isShifted !== padInstancesBuffer.isShifted) {
            padInstancesBuffer.swapIndexable();
        } else {
            padInstancesBuffer.next();
        };
        setPads(padInstancesBuffer.current());
    };

    const updateDeckHelper = function(padInstance, newGroup) {
        padInstance.forEachComponent(function(component) {
            if (padInstance === choosenPadInstance) {
                component.disconnect();
            }
            if (component.group === undefined) {
                component.group = newGroup;
            } else {
                const anyChannelRegEx = /\[Channel\d+([\]_])/;
                component.group = component.group.replace(anyChannelRegEx, `${newGroup.slice(0, -1)}$1`);
            }
            if (padInstance === choosenPadInstance) {
                component.connect();
                component.trigger();
            }
        });
    };

    this.updateDeck = function(currentDeck) {
        padInstancesBuffer.indexable[0].forEach(function(padInstance) {
            updateDeckHelper(padInstance, currentDeck);
        });
        padInstancesBuffer.indexable[1].forEach(function(padInstance) {
            updateDeckHelper(padInstance, currentDeck);
        });
    };

    this.shift = () => choosenPadInstance.shift();
    this.unshift = () => choosenPadInstance.unshift();
    this.shutdown = () => choosenPadInstance.shutdown();
    this.choosenPadInstance = () => choosenPadInstance;
    this.input = function(channel, control, value, status, group) {
        choosenPadInstance.pads[control].input(channel, control, value, status, group);
    };
};
DDJ200.PadModeContainers.ModeSelector.prototype = new components.ComponentContainer();

DDJ200.init = function() {
    this.decks = new components.ComponentContainer({
        left: new DDJ200.Deck([1, 3], 1),
        right: new DDJ200.Deck([2, 4], 2),
    });

    this.headMixButton = new components.Component({
        midi: [0x96, 0x63],
        key: "headMix",
        group: "[Master]",
        on: 0x7F,
        off: 0x00,
        inValueScale: function(value) {
            if (value) {
                return (this.inGetValue() >= 0) ? -1 : 1;
            }
            return this.inGetValue();
        },
        outValueScale: function(value) {
            return value >= 0 ? this.on : this.off;
        },
        shiftedInput: function(_channel, _control, value, _status, _g) {
            if (value) {
                engine.setValue("[Skin]", "show_4decks", !engine.getValue("[Skin]", "show_4decks"));
                if (!engine.getValue("[Skin]", "show_4decks")) {
                    DDJ200.decks.left.setCurrentDeck("[Channel1]");
                    DDJ200.decks.left.padUnit.updateDeck("[Channel1]");
                    DDJ200.decks.right.setCurrentDeck("[Channel2]");
                    DDJ200.decks.right.padUnit.updateDeck("[Channel2]");
                    DDJ200.decks.forEachComponentContainer(deck => {
                        if (deck.syncButton.blinkConnection) {
                            deck.syncButton.blinkConnection.disconnect();
                        };
                    }, false);
                };
            };
        },
        shutdown: function() {
            this.send(this.off);
        }
    });

    this.transFxButton = new components.Button({
        midi: [0x96, 0x59],
        shiftChannel: false,
        shiftControl: true,
        shiftOffset: 1,

        input: function(_channel, control, value, _status, _g) {
            if (value) {
                DDJ200.decks.forEachComponentContainer(deck => {
                    deck.padUnit.changePadMode((control === 0x5A));
                }, false);
                this.indicatePadMode();
            };
        },
        indicatorStyle: function() {
            // use left Deck as reference for selected padInstance
            return DDJ200.decks.left.padUnit.choosenPadInstance().indicatorStyle;
        },
        blinkConnection: undefined,
        indicatePadMode: function() {
            if (this.blinkConnection) {
                this.blinkConnection.disconnect();
            };
            if (this.indicatorStyle() === "indicator_250ms" || this.indicatorStyle() === "indicator_500ms") {
                this.blinkConnection = engine.makeConnection("[App]", this.indicatorStyle(), function(value, _group, _control) {
                    DDJ200.transFxButton.send(DDJ200.transFxButton.on * value);
                });
            } else {
                this.blinkConnection = undefined;
                this.send(this.on * this.indicatorStyle());
            };
        },
    });

    this.shutdown = function() {
        DDJ200.headMixButton.shutdown();
        DDJ200.transFxButton.shutdown();
        DDJ200.decks.shutdown();
        engine.setValue("[Channel3]", "volume", 0);
        engine.setValue("[Channel4]", "volume", 0);
    };

    // query the controller for current control positions on startup
    midi.sendSysexMsg([0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x02, 0x0a, 0x00, 0x03, 0x01, 0xf7], 12);

    // start with focus on library for selecting tracks (delay seems required)
    engine.beginTimer(500, function() {
        engine.setValue("[Library]", "MoveFocus", 1);
    }, true);
};

DDJ200.Deck = class extends components.Deck {
    constructor(deckNumbers, midiChannel) {
        super(deckNumbers);

        const theDeck = this;
        this.padUnit = new DDJ200.PadModeContainers.ModeSelector(this.currentDeck);

        this.jogWheel = new components.JogWheelBasic({
            wheelResolution: 128, // how many ticks per revolution the jogwheel has
            alpha: 1 / 8, // alpha-filter
            beta: 1 / 8 / 32,
            rpm: 33 + 1 / 3,
            inValueScale: function(value) {
                return value - 64;
            },
            inKey: "jog",
            jogCounter: 0,
            deck: deckNumbers[0],
            inputSeek: function(_channel, _control, value, _status, _group) {
                if (DDJ200.decks.left.shiftButton.pressed) {
                    this.jogCounter += this.inValueScale(value);
                    if (this.jogCounter > 9) {
                        engine.setValue("[Library]", "MoveDown", true);
                        this.jogCounter = 0;
                    } else if (this.jogCounter < -9) {
                        engine.setValue("[Library]", "MoveUp", true);
                        this.jogCounter = 0;
                    };
                } else {
                    const group = `[Channel${this.deck}]`; // fix for wrong group
                    const oldPos = engine.getValue(group, "playposition");
                    // Since ‘playposition’ is normalized to unity, we need to scale by
                    // song duration in order for the jog wheel to cover the same amount
                    // of time given a constant turning angle.
                    const duration = engine.getValue(group, "duration");
                    const newPos = Math.max(0, oldPos + (this.inValueScale(value) * 0.2 / duration));

                    engine.setValue(group, "playposition", newPos); // Strip search
                };
            },
        });

        this.shiftButton = new components.Button({
            pressed: 0,
            input: function(_channel, _control, value, _status, _g) {
                this.pressed = value;
                if (value) {
                    DDJ200.decks.shift();
                } else {
                    DDJ200.decks.unshift();
                }
            },
        });

        this.syncButton = new components.Button({
            midi: [0x8F + midiChannel, 0x58],
            key: "sync_enabled",
            shiftChannel: false,
            shiftControl: true,
            shiftOffset: 8,
            input: function(_channel, _control, value, _status, _g) {
                if (value) {
                    if (this.inGetValue() === 0) {
                        engine.setValue(this.group, "beatsync", 1);
                    } else {
                        this.inSetValue(0);
                    };
                };
            },
            inputLongPress: function(_channel, _control, value, _status, _g) {
                if (value) {
                    this.inSetValue(1);
                };
            },
            shiftedInput: function(_channel, _control, value, _status, _g) {
                if (value) {
                    midi.sendShortMsg(0x9F, (script.deckFromGroup(theDeck.currentDeck) - 1) % 2, 0x7F);

                    if (engine.getValue("[Skin]", "show_4decks")) {
                        theDeck.toggle();
                        theDeck.padUnit.updateDeck(theDeck.currentDeck);
                        if (theDeck.currentDeck === "[Channel3]" || theDeck.currentDeck === "[Channel4]") {
                            this.blinkConnection = engine.makeConnection("[App]", "indicator_250ms", function(value, _group, _control) {
                                theDeck.syncButton.send(theDeck.syncButton.on * value);
                            });
                        } else if (this.blinkConnection) {
                            this.blinkConnection.disconnect();
                        };
                    } else {
                        const currentRange = engine.getValue(this.group, "rateRange");
                        engine.setValue(this.group, "rateRange", (currentRange > 0.9) ? 0.1 : currentRange + 0.1);
                    };
                };
            },
        });

        this.playButton = new components.PlayButton({
            midi: [0x8F + midiChannel, 0x0B],
            shiftChannel: false,
            shiftControl: true,
            shiftOffset: 0x3C,
        });

        this.cueButton = new components.CueButton({
            midi: [0x8F + midiChannel, 0x0C],
            shiftChannel: false,
            shiftControl: true,
            shiftOffset: 0x3C,
        });

        this.faderStartButton = new components.Button({
            midi: [0x8F + midiChannel, 0x66],
            key: "cue_gotoandplay",
            input: function(channel, control, value, status, _g) {
                console.warn("faderStart");
                // according the manual:
                // Playback starts from the cue point
                // When no cue point is set, playback starts from the beginning of the track.
                components.Button.prototype.input.call(this, channel, control, value, status, _g);
            },
        });

        this.faderStopButton = new components.Button({
            midi: [0x8F + midiChannel, 0x52],
            // according the manual:
            // the track instantly jumps back to the cue point and playback pauses
            key: "cue_gotoandstop",
        });

        this.pflButton = new components.Button({
            midi: [0x8F + midiChannel, 0x54],
            key: "pfl",
            type: components.Button.prototype.types.toggle,
        });

        this.loadTrack = new components.Button({
            midi: [0x8F + midiChannel, 0x68],
            inKey: "LoadSelectedTrack",
        });

        this.volume = new components.Pot({
            inKey: "volume",
        });

        this.rate = new components.Pot({
            inKey: "rate",
            invert: 1,
        });

        this.eqKnob = [];
        for (let i = 0; i < 3; i++) {
            this.eqKnob[i] = new components.Pot({
                group: `[EqualizerRack1_${this.currentDeck}_Effect1]`,
                inKey: `parameter${i + 1}`,
            });
        };

        if (engine.getSetting("useSuperAsGain")) {
            this.super1 = new components.Pot({
                group: theDeck.currentDeck,
                inKey: "pregain",
            });
        } else {
            this.super1 = new components.Pot({
                group: `[QuickEffectRack1_${this.currentDeck}]`,
                inKey: "super1",
            });
        }

        this.reconnectComponents(function(c) {
            if (c.group === undefined) {
                c.group = this.currentDeck;
            };
        });
    }
};
