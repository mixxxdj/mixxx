/*
Pioneer DDJ-XP2 controller script
*/

// eslint-disable-next-line no-var
var DDJXP2 = { };

// most buttons use the following shift settings, its easier to correct them for the few others
components.Component.prototype.shiftOffset = 1;
components.Component.prototype.shiftChannel = true;
components.Component.prototype.sendShifted = true;
// override Component prototype to prevent double connections
// original connect function will override connections[0] and
// this way looses control of existing connection
components.Component.prototype.connect = function() {
    if (this.connections[0] === undefined &&
        undefined !== this.group &&
        undefined !== this.outKey &&
        undefined !== this.output &&
        typeof this.output === "function") {
        this.connections[0] = engine.makeConnection(this.group, this.outKey, this.output.bind(this));
    }
};

// helper to convert RGB into Pioneers color code. Not perfect, because there has been no
// documentation at all. But works for me :)
DDJXP2.RGBPioneerCode = function(r, g, b, dim = 0) {
    const n = Math.min(Math.min(r, g), b);
    const v = Math.max(Math.max(r, g), b);
    const m = v - n;
    let h = 0;
    if (v === 0) {
        h = 0x00;    // black
    } else if (m === 0) {
        h = 0x40;    // white
    } else if (r === n) {
        h = 3 + (b - g)/m;
    } else if (g === n) {
        h = 5 + (r - b)/m;
    } else {
        h = 1 + (g - r)/m;
    }
    h = 10.2*((10 - h) % 6) + 1; // re-aligned colorwheel
    if (dim) {
        return h + 0x40;
    } else {
        return h;
    }
};

// this implements a one-out-of-three selector button. The FX buttons are hardware controlled
// this way and it's required to follow these hardware with the implementation
DDJXP2.ThreeButtonSelector = class extends components.Button {
    constructor(options) {
        super(options);
        this.outTrigger = false;
        this.selection = [0, 0, 0];
        this.offset = options.offset;
    };
    inSetParameter(value, position) {
        engine.setParameter(this.groupArray[position], this.inKey, value);
    };
    input(_channel, control, value, _status, _group) {
        if (value) {
            for (let i = 0; i < 3; i++) {
                if (this.selection[i]) {
                    this.inSetParameter(this.inValueScale(0x00), i);
                    this.selection[i] = 0;
                } else if (i === (control - this.offset)) {
                    this.inSetParameter(this.inValueScale(0x7F), i);
                    this.selection[i] = 1;
                }
            }
        }
    };
    activate(enable) {
        // if not enabled it runs in the background and controls the GUI FX selection (1 out of 3)
        if (enable || (!enable && this.outTrigger)) {
            for (let i = 0; i < 3; i++) {
                if (this.selection[i]) {
                    this.send(this.outValueScale(enable), i);
                    if (enable) {
                        this.setExternalModifier(i + 1);
                    }
                }
            }
        } else {
            this.setExternalModifier(0);
        }
        this.outTrigger = enable;
    }
    send(value, position) {
        this.midi = this.midibase[position];
        super.send(value);
    };
    connect() {
        for (let i = 0; i < 3; i++) {
            this.connections[i] = engine.makeConnection(this.groupArray[i], this.outKey, this.output.bind(this));
        };
    };
    output(value, group, _control) {
        for (let i = 0; i < 3; i++) {
            if (value) {
                if (this.groupArray[i] !== group) {
                    if (this.selection[i]) {
                        // resetting any other activated Option
                        this.inSetParameter(this.inValueScale(0x00), i);
                        this.selection[i] = 0;
                    }
                } else {
                    // activating selected Option
                    if (this.outTrigger) {
                        this.send(this.outValueScale(value), i);
                    }
                    this.selection[i] = value;
                    if (this.outTrigger && typeof this.setExternalModifier === "function") {
                        this.setExternalModifier(i + 1);
                    }
                }
            } else {
                if (this.groupArray[i] === group) {
                    // dectivating of button when pressed again
                    if (this.outTrigger) {
                        this.send(this.outValueScale(value), i);
                    }
                    this.selection[i] = value;
                    if (this.outTrigger && typeof this.setExternalModifier === "function") {
                        this.setExternalModifier(0);
                    }

                }
            }
        }
    };
};


// this is the controller setup
DDJXP2.init = function() {
    // startup: could not find any suitable SysEx to reset the controller
    // it will be best if you disconnect and reconnect the controller before starting mixxx

    // reset PadMode selection
    const modeBtnColor1 = DDJXP2.RGBPioneerCode(80, 0, 255);
    const modeBtnColor2 = DDJXP2.RGBPioneerCode(255, 255, 0);
    for (let i = 0x93; i >= 0x90; i--) {
        for (let j = 0x6F; j >= 0x69; j--) {
            midi.sendShortMsg(i, j, modeBtnColor2);
        }
        for (let j = 0x22; j >= 0x1B; j--) {
            midi.sendShortMsg(i, j, modeBtnColor1);
        }
    }

    // reset controller Deck selection - has to be done before the Deck controls are initialized
    midi.sendShortMsg(0x92, 0x72, 0x00);
    midi.sendShortMsg(0x93, 0x72, 0x00);

    // create the controls:

    // some buttons/slicers send only two different midi - but we will use the controller with 4 decks
    this.controls2deck = new components.ComponentContainer({
        left: new DDJXP2.DeckControls2Deck([1, 3], 0),
        right: new DDJXP2.DeckControls2Deck([2, 4], 1),
    });

    // other buttons/pads send four different midi dependent on which deck is selected
    this.controls4deck = new components.ComponentContainer({
        one: new DDJXP2.DeckControls4Deck([1], 0),
        two: new DDJXP2.DeckControls4Deck([2], 1),
        three: new DDJXP2.DeckControls4Deck([3], 2),
        four: new DDJXP2.DeckControls4Deck([4], 3),
    });

    // there is one shift button, map it to the decks
    this.shiftButton = new components.Button({
        input: function(_channel, _control, value, _status, _g) {
            if (value) {
                DDJXP2.controls4deck.shift();
                DDJXP2.controls2deck.shift();
            } else {
                DDJXP2.controls4deck.unshift();
                DDJXP2.controls2deck.unshift();
            }
        },
    });

    // forward the shutdown to the decks
    this.shutdown = function() {
        DDJXP2.controls2deck.shutdown();
        DDJXP2.controls4deck.shutdown();
    };

    // the rotarySelector only works if mixxx windows is focused, don't be surprised :)
    this.rotarySelector = new components.Button({
        group: "[Library]",
        input: function(channel, control, value, status, group) {
            if (value === 0x01) {
                this.inKey = (control === 0x40)?"MoveDown":"MoveRight";
            } else {
                this.inKey = (control === 0x40)?"MoveUp":"MoveLeft";
            }
            components.Button.prototype.input.call(this, channel, control, 0x7F, status, group);
        },
        inputPress: function(channel, control, value, status, group) {
            this.inKey = "MoveFocusForward";
            components.Button.prototype.input.call(this, channel, control, value, status, group);
        },
    });

    // always show 4decks and use 4 effectunits as default
    engine.setValue("[Skin]", "show_4decks", true);
    engine.setValue("[Skin]", "show_4effectunits", true);

    // the FX buttons on the controller are used to activate one of the
    // EffectUnits 1-3 per Deck. Multiple Selections are not possible by hardware design,
    // therefore reset the GUI FX selectors first.
    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 0);
    engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", 0);
    engine.setValue("[EffectRack1_EffectUnit3]", "group_[Channel1]_enable", 0);
    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", 0);
    engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 0);
    engine.setValue("[EffectRack1_EffectUnit3]", "group_[Channel2]_enable", 0);
    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel3]_enable", 0);
    engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel3]_enable", 0);
    engine.setValue("[EffectRack1_EffectUnit3]", "group_[Channel3]_enable", 0);
    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel4]_enable", 0);
    engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel4]_enable", 0);
    engine.setValue("[EffectRack1_EffectUnit3]", "group_[Channel4]_enable", 0);

    // activate the controlFX of the two base decks (see DeckControls4Deck.controlFX for more details)
    this.controls4deck.one.controlFX.activate(1);
    this.controls4deck.two.controlFX.activate(1);

    // try to set the deck highlights.
    try {
        engine.setValue("[Skin]", "highlight_mixer_[Channel1]", 1);
        engine.setValue("[Skin]", "highlight_deck_[Channel1]", 1);
        engine.setValue("[Skin]", "highlight_waveform_[Channel1]", 1);
        engine.setValue("[Skin]", "highlight_mixer_[Channel2]", 1);
        engine.setValue("[Skin]", "highlight_deck_[Channel2]", 1);
        engine.setValue("[Skin]", "highlight_waveform_[Channel2]", 1);
        engine.setValue("[Skin]", "highlight_mixer_[Channel3]", 0);
        engine.setValue("[Skin]", "highlight_deck_[Channel3]", 0);
        engine.setValue("[Skin]", "highlight_waveform_[Channel3]", 0);
        engine.setValue("[Skin]", "highlight_mixer_[Channel4]", 0);
        engine.setValue("[Skin]", "highlight_deck_[Channel4]", 0);
        engine.setValue("[Skin]", "highlight_waveform_[Channel4]", 0);
    } finally {
        // continue anyway
    };
};

// standard Pad Container, will be pre-filled for every available Pad
DDJXP2.PadMode = class extends components.ComponentContainer {
    constructor(options) {
        super(options);
        // this is a workaround for components, forEachComponent only iterates
        // over ownProperties, so these have to constructed by the constructor here
        // instead of being merged by the ComponentContainer constructor
        this.pads = Array(16).fill(undefined);
    }
    constructPads(constructPad) {
        this.pads = this.pads.map((_, padIndex) => constructPad(padIndex));
    }
};

// special Pad Container for Slicers, because there all Pads interact with each other
DDJXP2.PadModeSlicer = class extends DDJXP2.PadMode {
    constructor(group) {
        super();
        this.group = group;
        this.beat = -1;
        this.useSlip = engine.getSetting("useSlipOnSlicer");
        this.pressed = [false, false, false, false, false, false, false, false];
        this.startPos = -1;
        this.isActivePadMode = false;
        this.samplesBetweenSlices = undefined;
        const SlicerContainer = this;

        // move slicer window one samplesBetweenSlices-Size left
        this.parameterLeft = new components.Button({
            input(_channel, _control, value, _status, _group) {
                if (value) {
                    SlicerContainer.startPos -= SlicerContainer.samplesBetweenSlices;
                    SlicerContainer.activate(1, 0); // re-activate with provided startPos
                    SlicerContainer.updateLoop();
                }
            }
        });
        // move slicer window one samplesBetweenSlices-Size right
        this.parameterRight = new components.Button({
            input(_channel, _control, value, _status, _group) {
                if (value) {
                    SlicerContainer.startPos += SlicerContainer.samplesBetweenSlices;
                    SlicerContainer.activate(1, 0); // re-activate with provided startPos
                    SlicerContainer.updateLoop();
                }
            }
        });

        // always update the slicer if a new track is loaded
        this.loadConnection = engine.makeConnection(this.group, "track_loaded", this.trackLoaded.bind(this));
    }
    // If at least one button is pressed, create a loop between those points,
    // else show some area indications (SlicereLoop) or create overall loop (SlicerLoopRoll)
    updateLoop() {
        const startPad = this.pressed.indexOf(true);
        const endPad = this.pressed.lastIndexOf(true);

        if (startPad !== -1) {
            engine.setValue(this.group, "loop_start_position", this.pads[startPad].startSample);
            engine.setValue(this.group, "loop_end_position", this.pads[endPad].endSample);

            // move playing position if not in resulting loop
            const beatSize = engine.getValue(this.group, "beatloop_size")/8;

            if (this.beat < startPad) {
                engine.setValue(this.group, "beatjump", beatSize * (startPad - this.beat));
            }
            if (this.beat > endPad) {
                engine.setValue(this.group, "beatjump", beatSize * (endPad - this.beat));
            }

            if (this.useSlip) {
                engine.setValue(this.group, "slip_enabled", 1);
            }
            engine.setValue(this.group, "loop_enabled", 1);

        } else {
            engine.setValue(this.group, "loop_start_position", this.startPos);
            engine.setValue(this.group, "loop_end_position", this.startPos + 8 * this.samplesBetweenSlices);
            engine.setValue(this.group, "loop_enabled", (this.type !== "SlicerLoop"));
            if (this.useSlip) {
                engine.setValue(this.group, "slip_enabled", (this.type !== "SlicerLoop"));
            }
        }

        this.forEachComponent(function(component) {
            if (typeof component.activate === "function") {
                component.output(component.number >= startPad && component.number <= endPad);
            }
        });
    }
    // calculate samples per beat
    samplesPerBeat(group) {
        const sampleRate = engine.getValue(group, "track_samplerate");
        const bpm = engine.getValue(group, "local_bpm");
        // The sample rate includes both channels (i.e. it is double the framerate)
        // Hence, we multiply by 2*60 (120) instead of 60 to get the correct sample rate
        const secondsPerBeat = 120/bpm;
        const samplesPerBeat = secondsPerBeat * sampleRate;
        return samplesPerBeat;
    }
    // reinitialize Slicer when the beatloop size / slicer size changes
    slicerSizeChange(_value, _group, _control) {
        this.activate(1, 0);
        this.updateLoop();
    }
    // count beats and move the Slicer section forward when needed
    slicerCountBeat(_value, _group, _control) {
        // Calculate current position in samples
        const currentPos = engine.getValue(this.group, "track_samples") * engine.getValue(this.group, "playposition");
        // Calculate beat
        let beat = 0;
        for (let i = 0; i < 8; i++) {
            beat = (currentPos >= this.pads[i].endSample) ? (beat + 1) : beat;
        }
        this.beat = beat;

        // If in slicer mode (not slicer loop mode), check to see if the slicer section needs to be moved
        if (beat > 7) {
            this.startPos = this.pads[7].endSample;
            this.activate(1, 0);
        }
    }
    // external initialization, will be called on every pad selection button press
    init(status, control, value) {
        switch (value) {
        case 0:
            this.isActivePadMode = false;
            this.activate(0);
            break;
        case 1:
        case 2:
            this.isActivePadMode = true;
            this.activate(1, 1);
            if (this.padModeButton) {
                this.padModeButton.midi = [status, control];
                this.padModeButton.output();
            }
            break;
        }
    }
    // callback for loaded track and followed beat detection, now slicer can be activated
    trackLoadedAndBPMDetected() {
        if (this.bpmConnection) {
            this.bpmConnection.disconnect();
            this.bpmConnection = undefined;
        }
        this.activate(1, 1);
    }
    // callback for loaded track
    trackLoaded() {
        if (this.bpmConnection) {
            this.bpmConnection.disconnect();
            this.bpmConnection = undefined;
        }
        // wait for the upcoming bpm detection, can't do anything without this info
        if (engine.getValue(this.group, "track_loaded") && this.isActivePadMode) {
            this.bpmConnection = engine.makeConnection(this.group, "local_bpm", this.trackLoadedAndBPMDetected.bind(this));
        }
    }
    // activate the slicer with option to renew the start position
    activate(enter = 0, renewStart = 1) {
        if (enter && engine.getValue(this.group, "track_loaded")) {
            if (renewStart) {
                this.startPos = engine.getValue(this.group, "beat_closest");
            }
            this.samplesBetweenSlices = this.samplesPerBeat(this.group) * engine.getValue(this.group, "beatloop_size") / 8;
            this.forEachComponent(function(component) {
                if (typeof component.activate === "function") {
                    component.activate();
                }
            });

            if (this.beatConnection === undefined) {
                this.beatConnection = engine.makeConnection(this.group, "beat_distance", this.slicerCountBeat.bind(this));
                this.sizeConnection = engine.makeConnection(this.group, "beatloop_size", this.slicerSizeChange.bind(this));
            };

            engine.setValue(this.group, "loop_start_position", this.startPos);
            engine.setValue(this.group, "loop_end_position", (this.startPos + 8 * this.samplesBetweenSlices));
            engine.setValue(this.group, "loop_enabled", (this.type !== "SlicerLoop"));
        } else {
            this.forEachComponent(function(component) {
                if (typeof component.activate === "function") {
                    component.deactivate();
                }
            });
            if (this.beatConnection !== undefined) {
                this.beatConnection.disconnect();
                this.sizeConnection.disconnect();

                engine.setValue(this.group, "loop_start_position", -1);
                engine.setValue(this.group, "loop_end_position", -1);
                if (this.useSlip) {
                    engine.setValue(this.group, "slip_enabled", 0);
                }
                engine.setValue(this.group, "loop_enabled", 0);

                this.beatConnection = undefined;
            }
        }
    }
};

// special button for slicers
DDJXP2.SlicerButton = class extends components.Button {
    constructor(options, padContainer) {
        super(options);
        this.padContainer = padContainer;
        this.output(0x00);
    }
    activate() {
        this.startSample = this.padContainer.startPos + this.number * this.padContainer.samplesBetweenSlices;
        this.endSample = this.padContainer.startPos + (this.number + 1) * this.padContainer.samplesBetweenSlices;
    }
    deactivate() {
        this.startSample = -1;
        this.endSample = -1;
    }
    input(_channel, _control, value, _status, _group) {
        this.padContainer.pressed[this.number] = (value !== 0);

        if (this.startSample !== -1) {
            this.padContainer.updateLoop();
        }
    }
    outValueScale(value) {
        return value?this.on:this.off;
    }
};

DDJXP2.padMidiAssignment = [0xC, 0xD, 0xE, 0xF, 0x8, 0x9, 0xA, 0xB, 0x4, 0x5, 0x6, 0x7, 0x0, 0x1, 0x2, 0x3];

// some Pad-Rows which can be reused in different PadModes
DDJXP2.PadRows = {
    // first four samplers on the left side, fifth to eights sampler on the right side
    sampler: function(deckOffset, _group, i, midiOffset) {
        const row = Math.floor(i / 4);
        return new components.SamplerButton({
            midi: [0x97 + (deckOffset * 2), midiOffset + DDJXP2.padMidiAssignment[i]],
            number: (deckOffset % 2) * 4 + i - (row * 4) +1,
            on: DDJXP2.RGBPioneerCode(0, 255, 0, true),
            off: 0x3F,
        });
    },
    // jump through your track with the outer buttons, change the jump-distance with the inner ones
    jump: function(deckOffset, group, i, midiOffset) {
        const pos = i % 4;
        if (pos === 0) {
            return new components.Button({
                midi: [0x97 + (deckOffset * 2), midiOffset + DDJXP2.padMidiAssignment[i]],
                group,
                key: "beatjump_backward",
                on: DDJXP2.RGBPioneerCode(255, 0, 255),
                off: DDJXP2.RGBPioneerCode(255, 0, 255, true),
            });
        } else if (pos === 1) {
            return new components.Button({
                midi: [0x97 + (deckOffset * 2), midiOffset + DDJXP2.padMidiAssignment[i]],
                group,
                key: "beatjump_size_halve",
                on: DDJXP2.RGBPioneerCode(100, 0, 255),
                off: DDJXP2.RGBPioneerCode(100, 0, 255, true),
            });
        } else if (pos === 2) {
            return new components.Button({
                midi: [0x97 + (deckOffset * 2), midiOffset + DDJXP2.padMidiAssignment[i]],
                group,
                key: "beatjump_size_double",
                on: DDJXP2.RGBPioneerCode(100, 0, 255),
                off: DDJXP2.RGBPioneerCode(100, 0, 255, true),
            });
        } else {
            return new components.Button({
                midi: [0x97 + (deckOffset * 2), midiOffset + DDJXP2.padMidiAssignment[i]],
                group,
                key: "beatjump_forward",
                on: DDJXP2.RGBPioneerCode(255, 0, 255),
                off: DDJXP2.RGBPioneerCode(255, 0, 255, true),
            });
        }
    },
    // play and cue button followed by some jump to start and some track eject button
    play: function(deckOffset, group, i, midiOffset) {
        const pos = i % 4;
        if (pos === 0) {
            return new components.PlayButton({
                midi: [0x97 + (deckOffset * 2), midiOffset + DDJXP2.padMidiAssignment[i]],
                group,
                on: DDJXP2.RGBPioneerCode(255, 30, 0),
            });
        } else if (pos === 1) {
            return new components.CueButton({
                midi: [0x97 + (deckOffset * 2), midiOffset + DDJXP2.padMidiAssignment[i]],
                group,
                on: DDJXP2.RGBPioneerCode(255, 30, 0),
            });
        } else if (pos === 2) {
            return new components.Button({
                midi: [0x97 + (deckOffset * 2), midiOffset + DDJXP2.padMidiAssignment[i]],
                group,
                key: "start",
                on: DDJXP2.RGBPioneerCode(0, 0, 255),
                off: DDJXP2.RGBPioneerCode(0, 0, 255, true),
            });
        } else {
            return new components.Button({
                midi: [0x97 + (deckOffset * 2), midiOffset + DDJXP2.padMidiAssignment[i]],
                group,
                key: "eject",
                on: DDJXP2.RGBPioneerCode(0, 0, 255),
                off: DDJXP2.RGBPioneerCode(0, 0, 255, true),
            });
        }
    },
};

// PAD implementation
DDJXP2.PadModeContainers = {
    hotCue: class extends DDJXP2.PadMode {
        constructor(padNr, deckOffset, group, _modeBtnColor, _modeBtnAttnColor) {
            super();
            const theContainer = this;
            this.HotcueButtonPreset = class extends components.HotcueButton {
                constructor(number, i) {
                    super({
                        midi: [0x97 + (deckOffset * 2), padNr * 0x10 + DDJXP2.padMidiAssignment[i]],
                        number,
                        group,
                        on: DDJXP2.RGBPioneerCode(255, 128, 0),
                        off: DDJXP2.RGBPioneerCode(255, 128, 0, true),
                        outValueScale: function(value) {
                            return (value)?this.on:this.off;
                        },
                    });
                }
            };
            super.constructPads(i => {
                if (parseInt(engine.getSetting("numberOfHotCues")) === 8) {
                    if (engine.getSetting("addPlayToHotCue")) {
                        if (i < 4) {
                            return DDJXP2.PadRows.play(deckOffset, group, i, padNr * 0x10);
                        } else if (i < 12) {
                            return new theContainer.HotcueButtonPreset(i - 3, i);
                        } else {
                            return DDJXP2.PadRows.sampler(deckOffset, group, i, padNr * 0x10);
                        }
                    } else {
                        if (i < 8) {
                            return new theContainer.HotcueButtonPreset(i + 1, i);
                        } if (i < 12) {
                            return DDJXP2.PadRows.jump(deckOffset, group, i, padNr * 0x10);
                        } else {
                            return DDJXP2.PadRows.sampler(deckOffset, group, i, padNr * 0x10);
                        }
                    }
                } else if (parseInt(engine.getSetting("numberOfHotCues")) === 12) {
                    if (engine.getSetting("addPlayToHotCue")) {
                        if (i < 4) {
                            return DDJXP2.PadRows.play(deckOffset, group, i, padNr * 0x10);
                        } else {
                            return new theContainer.HotcueButtonPreset(i - 3, i);
                        }
                    } else {
                        if (i < 12) {
                            return new theContainer.HotcueButtonPreset(i + 1, i);
                        } else {
                            return DDJXP2.PadRows.sampler(deckOffset, group, i, padNr * 0x10);
                        }
                    }
                } else {
                    return new theContainer.HotcueButtonPreset(i + 1, i);
                }
            });
        }
    },
    beatLoop: class extends DDJXP2.PadMode {
        constructor(padNr, deckOffset, group, modeBtnColor, modeBtnAttnColor) {
            super();
            const theContainer = this;
            this.currentBaseLoopSize = parseInt(engine.getSetting("defaultLoopRootSize"));
            this.useSlip = engine.getSetting("useSlipOnLoops");
            this.slipOn = modeBtnAttnColor;
            this.slipOff = modeBtnColor;
            this.group = group;

            this.parameterLeft = new components.Button({
                group: group,
                outConnect: false,
                outKey: "loop_enabled",
                inKey: "beatjump_1_backward",
                input(_channel, _control, value, _status, _group) {
                    if (this.outGetValue()) {
                        this.inSetValue(value);
                    }
                }
            });
            this.parameterRight = new components.Button({
                group: group,
                outConnect: false,
                outKey: "loop_enabled",
                inKey: "beatjump_1_forward",
                input(_channel, _control, value, _status, _group) {
                    if (this.outGetValue()) {
                        this.inSetValue(value);
                    }
                }
            });
            // indicate slip Mode with different padMode Button color
            this.padModeButton = new components.Button({
                on: theContainer.slipOn,
                off: theContainer.slipOff,
                sendShifted: false,
                outValueScale: function(_value) {
                    return (theContainer.useSlip !== engine.getSetting("useSlipOnLoops"))?this.on:this.off;
                }
            });

            super.constructPads(i => {
                if (i < 12) {
                    const loopSize = Math.pow(2, theContainer.currentBaseLoopSize + i);
                    return new components.Button({
                        midi: [0x97 + (deckOffset * 2), padNr * 0x10 + DDJXP2.padMidiAssignment[i]],
                        group,
                        inKey: `beatloop_${loopSize}_toggle`,
                        outKey: `beatloop_${loopSize}_enabled`,
                        rollInKey: "beatlooproll_activate",
                        on: DDJXP2.RGBPioneerCode(128, 160, 0),
                        off: DDJXP2.RGBPioneerCode(128, 160, 0, true),
                        input(channel, control, value, status, group) {
                            if (theContainer.useSlip && value) {
                                const loopEnabled = engine.getValue(this.group, "loop_enabled");
                                const matchingLoopSize = (engine.getValue(this.group, "beatloop_size") === loopSize);
                                engine.setValue(this.group, "beatloop_size", loopSize);
                                if (matchingLoopSize || !loopEnabled) {
                                    engine.setValue(this.group, this.rollInKey, true);
                                };
                            } else {
                                components.Button.prototype.input.call(this, channel, control, value, status, group);
                            }
                        },
                    });
                } else {
                    return DDJXP2.PadRows.sampler(deckOffset, group, i, padNr * 0x10);
                }
            });
        }
        slip(status, control, value) {
            if (value && !engine.getValue(this.group, "loop_enabled")) {
                this.useSlip = !this.useSlip;
                this.padModeButton.output();
            }
        }
        init(status, control, value) {
            switch (value) {
            case 0:
                break;
            case 1:
            case 2:
                this.padModeButton.midi = [status, control];
                this.padModeButton.output();
                break;
            }
        }
    },
    slicer: class extends DDJXP2.PadModeSlicer {
        constructor(padNr, deckOffset, group, modeBtnColor, modeBtnAttnColor) {
            super(group);
            this.type = "SlicerLoop";
            this.slipOn = modeBtnAttnColor;
            this.slipOff = modeBtnColor;

            const padContainer = this;
            // indicate slip Mode with different padMode Button color
            this.padModeButton = new components.Button({
                on: padContainer.slipOn,
                off: padContainer.slipOff,
                sendShifted: false,
                outValueScale: function(_value) {
                    return (padContainer.useSlip !== engine.getSetting("useSlipOnSlicer"))?this.on:this.off;
                }
            });
            super.constructPads((i) => {
                if (i < 8) {
                    // Slicer copied and adapted from Hercules-DJControl-Inpulse-300-script.js
                    return new DDJXP2.SlicerButton({
                        midi: [0x97 + (deckOffset * 2), padNr * 0x10 + DDJXP2.padMidiAssignment[i]],
                        number: i,
                        group: group,
                        on: DDJXP2.RGBPioneerCode(0, 255, 255),
                        off: DDJXP2.RGBPioneerCode(0, 255, 255, true),
                    }, padContainer);
                } if (i < 12) {
                    return DDJXP2.PadRows.jump(deckOffset, group, i, padNr * 0x10);
                } else {
                    return DDJXP2.PadRows.sampler(deckOffset, group, i, padNr * 0x10);
                }
            });
        }
        // external toggle for slip mode
        slip(status, control, value) {
            if (value && !engine.getValue(this.group, "loop_enabled")) {
                this.useSlip = !this.useSlip;
                this.padModeButton.output();
            }
        }
    },
    beatJump: class extends DDJXP2.PadMode {
        constructor(padNr, deckOffset, group, _modeBtnColor, _modeBtnAttnColor) {
            super();
            const theContainer = this;
            this.currentBaseJumpSize = parseInt(engine.getSetting("defaultBeatJumpRootSize"));
            this.parameterLeft = new components.Button({
                group: group,
                key: "beatjump_size_halve",
            });
            this.parameterRight = new components.Button({
                group: group,
                key: "beatjump_size_double",
            });

            super.constructPads(i => {
                if (i < 12) {
                    const jumpSize = Math.pow(2, theContainer.currentBaseJumpSize + i);
                    return new components.Button({
                        midi: [0x97 + (deckOffset * 2), padNr * 0x10 + DDJXP2.padMidiAssignment[i]],
                        group,
                        on: DDJXP2.RGBPioneerCode(128, 0, 160),
                        off: DDJXP2.RGBPioneerCode(128, 0, 160, true),
                        key: `beatjump_${jumpSize}_${  (i % 2)?"forward":"backward"}`,
                    });
                } else {
                    return DDJXP2.PadRows.sampler(deckOffset, group, i, padNr * 0x10);
                }
            });
        }
    },
    keyShift: class extends DDJXP2.PadMode {
        constructor(padNr, deckOffset, group, _modeBtnColor, _modeBtnAttnColor) {
            super();
            const theContainer = this;
            this.active = 0;
            this.group = group;
            this.parameterLeft = new components.Button({
                group: group,
                key: "pitch_adjust",
                input(_channel, _control, value, _status, _group) {
                    if (value) {
                        theContainer.active -= 0.1;
                        if (theContainer.active < -6) {
                            theContainer.active = -6;
                        }
                        this.inSetValue(theContainer.active);
                        theContainer.updateLEDs();
                    }
                },
            });
            this.parameterRight = new components.Button({
                group: group,
                key: "pitch_adjust",
                input(_channel, _control, value, _status, _group) {
                    if (value) {
                        theContainer.active += 0.1;
                        if (theContainer.active > 6) {
                            theContainer.active = 6;
                        }
                        this.inSetValue(theContainer.active);
                        theContainer.updateLEDs();
                    }
                },
            });

            super.constructPads(i => {
                if (i < 12) {
                    // offset from -6 to +6
                    const offset = (((2 - Math.floor(i / 4)) * 4 + (i % 4)) - 5.5) * 12 / 11;
                    // this strange calculation will give me the following pad value matrix
                    //  2,73     3,82    4,91    6,00
                    // -1,64    -0,55    0,55    1,64
                    // -6,00    -4,91   -3,82   -2,73
                    return new components.Button({
                        midi: [0x97 + (deckOffset * 2), padNr * 0x10 + DDJXP2.padMidiAssignment[i]],
                        number: i,
                        inKey: "pitch_adjust",
                        outKey: "pitch",
                        offset,
                        group,
                        on: DDJXP2.RGBPioneerCode(255 + (offset - 6) * 20, 0, 255 - (offset + 6) * 20),
                        off: DDJXP2.RGBPioneerCode(255 + (offset - 6) * 20, 0, 255 - (offset + 6) * 20, true),
                        input(_channel, _control, value, _status, _group) {
                            if (value) {
                                theContainer.active = (Math.abs(this.inGetValue() - this.offset) < 0.2)?0:this.offset;
                                this.inSetValue(theContainer.active);
                                theContainer.updateLEDs();
                            }
                        },
                        updateLED() {
                            this.output(this.inGetValue());
                        },
                        outValueScale(value) {
                            // distance between pads is 1.1 - light neighbour if no direct match
                            return (value && Math.abs(value - this.offset) <= 1)?this.on:this.off;
                        },
                    });
                } else {
                    return DDJXP2.PadRows.sampler(deckOffset, group, i, padNr * 0x10);
                }
            });
        }
        pitchDown(status, control, value) {
            if (value && this.active !== -6) {
                this.active -= 1;
                if (!this.active) {
                    this.active = -1;
                }
                engine.setValue(this.group, "pitch_down", 0x7F);
                this.updateLEDs();
            }
        }
        pitchUp(status, control, value) {
            if (value && this.active !== 6) {
                this.active += 1;
                if (!this.active) {
                    this.active = +1;
                }
                engine.setValue(this.group, "pitch_up", 0x7F);
                this.updateLEDs();
            }
        }
        resetKey(status, control, value) {
            if (value) {
                this.active = 0;
                engine.setValue(this.group, "reset_key", 0x7F);
                this.updateLEDs();
            }
        }
        syncKey(status, control, value) {
            if (value) {
                engine.setValue(this.group, "sync_key", 0x7F);
                this.active = engine.getValue(this.group, "pitch");
                this.updateLEDs();
            }
        }
        init(status, control, value) {
            switch (value) {
            case 0:
                break;
            case 1:
            case 2:
                this.updateLEDs();
                break;
            }
        }
        updateLEDs() {
            this.forEachComponent(function(component) {
                if (typeof component.updateLED === "function") {
                    component.updateLED();
                }
            });
        }
    },
    keyPad: class extends DDJXP2.PadMode {
        constructor(padNr, deckOffset, group, _modeBtnColor, _modeBtnAttnColor) {
            super();
            const theContainer = this;
            this.active = 0;
            this.group = group;
            super.constructPads(i => {
                if (i < 12) {
                    // offset from -6 to 6 without 0
                    let offset = (2 - Math.floor(i / 4)) * 4 + (i % 4);
                    offset += (offset < 6)?-6:-5;
                    // this strange calculation will give me the following pad value matrix
                    //  3    4    5    6
                    // -2   -1    1    2
                    // -6   -5   -4   -3
                    const colorR = (offset < 0)?(255 + (offset - 6) * 20):(255 + (offset - 6) * 8);
                    const colorB = (offset < 0)?(255 - (offset + 6) * 8):(255 - (offset + 6) * 20);

                    return new components.Button({
                        midi: [0x97 + (deckOffset * 2), padNr * 0x10 + DDJXP2.padMidiAssignment[i]],
                        key: "pitch_adjust",
                        number: i,
                        offset,
                        group,
                        on: DDJXP2.RGBPioneerCode(colorR, 0, colorB),
                        off: DDJXP2.RGBPioneerCode(colorR, 0, colorB, true),
                        input(_channel, _control, value, _status, _group) {
                            if (value) {
                                engine.setValue(this.group, "reset_key", 0x7F);
                                if (theContainer.active !== this.offset) {
                                    theContainer.active = this.offset;
                                    this.inSetValue(this.offset);
                                } else {
                                    theContainer.active = 0;
                                }
                            }
                        },
                        updateLED() {
                            this.output(theContainer.active);
                        },
                        outValueScale(value) {
                            // distance between pads is 1 - light closest neighbour
                            return (value && Math.abs(value - this.offset) <= 0.1)?this.on:this.off;
                        },
                    });
                } else {
                    return DDJXP2.PadRows.sampler(deckOffset, group, i, padNr * 0x10);
                }
            });
        }
        pitchDown(status, control, value) {
            if (value && this.active !== -6) {
                this.active -= 1;
                engine.setValue(this.group, "pitch_adjust", this.active);
                this.updateLEDs();
            }
        }
        pitchUp(status, control, value) {
            if (value && this.active !== 6) {
                this.active += 1;
                engine.setValue(this.group, "pitch_adjust", this.active);
                this.updateLEDs();
            }
        }
        resetKey(status, control, value) {
            if (value) {
                this.active = 0;
                this.updateLEDs();
                engine.setValue(this.group, "reset_key", 0x7F);
            }
        }
        syncKey(status, control, value) {
            if (value) {
                engine.setValue(this.group, "sync_key", 0x7F);
                this.active = engine.getValue(this.group, "pitch_adjust");
                this.updateLEDs();
            }
        }
        init(status, control, value) {
            switch (value) {
            case 0:
                break;
            case 1:
            case 2:
                this.active = engine.getValue(this.group, "pitch_adjust");
                this.updateLEDs();
                break;
            }
        }
        updateLEDs() {
            this.forEachComponent(function(component) {
                if (typeof component.updateLED === "function") {
                    component.updateLED();
                }
            });
        }
    },
    slicerRoll: class extends DDJXP2.PadModeSlicer {
        // 1-8: Slicer
        // 9-16: Sampler
        constructor(padNr, deckOffset, group, modeBtnColor, modeBtnAttnColor) {
            super(group);
            this.type = "SlicerLoopRoll";
            this.slipOn = modeBtnAttnColor;
            this.slipOff = modeBtnColor;
            const padContainer = this;
            super.constructPads((i) => {
                if (i < 8) {
                    // Slicer copied and adapted from Hercules-DJControl-Inpulse-300-script.js
                    return new DDJXP2.SlicerButton({
                        midi: [0x97 + (deckOffset * 2), padNr * 0x10 + DDJXP2.padMidiAssignment[i]],
                        number: i,
                        group: group,
                        on: DDJXP2.RGBPioneerCode(0, 255, 100),
                        off: DDJXP2.RGBPioneerCode(0, 255, 100, true),
                    }, padContainer);
                } if (i < 12) {
                    return DDJXP2.PadRows.jump(deckOffset, group, i, padNr * 0x10);
                } else {
                    return DDJXP2.PadRows.sampler(deckOffset, group, i, padNr * 0x10);
                }
            });
        }
    },
    sampler: class extends DDJXP2.PadMode {
        constructor(padNr, deckOffset, _group, _modeBtnColor, _modeBtnAttnColor) {
            super();
            this.baseOffset = 0;
            this.onPosition = DDJXP2.RGBPioneerCode(120, 255, 0);
            this.offPosition = DDJXP2.RGBPioneerCode(120, 255, 0, true);
            this.on = DDJXP2.RGBPioneerCode(0, 255, 0);
            this.off = DDJXP2.RGBPioneerCode(0, 255, 0, true);
            const padContainer = this;

            this.parameterLeft = {
                input(_channel, _control, value, _status, _group) {
                    if (value) {
                        padContainer.baseOffset = (padContainer.baseOffset)?(padContainer.baseOffset - 1):3;
                        padContainer.updateAllPads();
                    }
                }
            };
            this.parameterRight = {
                input(_channel, _control, value, _status, _group) {
                    if (value) {
                        padContainer.baseOffset = (padContainer.baseOffset + 1) % 4;
                        padContainer.updateAllPads();
                    }
                }
            };
            super.constructPads(i => new components.SamplerButton({
                midi: [0x97 + (deckOffset * 2), padNr * 0x10 + DDJXP2.padMidiAssignment[i]],
                baseNumber: i + 1,
                number: i + 1,
                on: padContainer.on,
                off: padContainer.off,
            })
            );
        }
        updateAllPads() {
            this.reconnectComponents(function(component) {
                component.number = component.baseNumber + this.baseOffset * 16;
                component.group = `[Sampler${  component.number  }]`;
                switch (this.baseOffset) {
                case 1:
                    switch (component.baseNumber) {
                    case 1:
                    case 2:
                    case 3:
                    case 5:
                    case 6:
                    case 9:
                        component.on = this.onPosition;
                        component.off = this.offPosition;
                        break;
                    default:
                        component.on = this.on;
                        component.off = this.off;
                    }
                    break;
                case 2:
                    switch (component.baseNumber) {
                    case 12:
                    case 15:
                    case 16:
                        component.on = this.on;
                        component.off = this.off;
                        break;
                    default:
                        component.on = this.onPosition;
                        component.off = this.offPosition;
                    }
                    break;
                case 3:
                    component.on = this.onPosition;
                    component.off = this.offPosition;
                    break;
                default:
                    component.on = this.on;
                    component.off = this.off;
                }
            });
        }
    },
    quickEffect: class extends DDJXP2.PadMode {
        constructor(padNr, deckOffset, group, _modeBtnColor, _modeBtnAttnColor) {
            super();
            this.deckOffset = deckOffset;
            this.group = group;
            super.constructPads(i => new components.Button({
                midi: [0x97 + (deckOffset * 2), padNr * 0x10 + DDJXP2.padMidiAssignment[i]],
                group: `[QuickEffectRack1_${group}]`,
                key: "loaded_chain_preset",
                number: i + 1,
                on: DDJXP2.RGBPioneerCode(0, 255, 0),
                off: 0x7F,
                input(_channel, _control, value, _status, _group) {
                    if (value) {
                        this.inSetValue(this.inValueScale(value));
                    }
                },
                inValueScale(_value) {
                    return (this.inGetValue() !== this.number)?this.number:1;
                },
                outValueScale(value) {
                    return (value === this.number)?this.on:this.off;
                },
            })
            );
        }
        init(status, control, value) {
            if (engine.getSetting("useFaderForQuickEffects")) {
                Object.values(DDJXP2.controls2deck)[(script.deckFromGroup(this.group) - 1) % 2].fader.useForEffects(value, `[QuickEffectRack1_${this.group}]`, "super1");
            }
        }
    },
    equalizerRack: class extends DDJXP2.PadMode {
        constructor(padNr, deckOffset, group, _modeBtnColor, _modeBtnAttnColor) {
            super();
            super.constructPads((i) => {
                if (i < 8) {
                    return new components.Button({
                        midi: [0x97 + (deckOffset * 2), padNr * 0x10 + DDJXP2.padMidiAssignment[i]],
                        group: `[EqualizerRack1_${group}]`,
                        key: "loaded_chain_preset",
                        number: i + 2,
                        on: DDJXP2.RGBPioneerCode(255, 128, 0),
                        off: 0x7F,
                        input(_channel, _control, value, _status, _group) {
                            if (value) {
                                this.inSetValue(this.inValueScale(value));
                            }
                        },
                        inValueScale(_value) {
                            return (this.inGetValue() !== this.number)?this.number:1;
                        },
                        outValueScale(value) {
                            return (value === this.number)?this.on:this.off;
                        },
                    });
                } if (i < 12) {
                    return DDJXP2.PadRows.jump(deckOffset, group, i, padNr * 0x10);
                } else {
                    return DDJXP2.PadRows.sampler(deckOffset, group, i, padNr * 0x10);
                }
            });
        }
    },
};

DDJXP2.DeckControls4Deck = class extends components.Deck {
    constructor(deckNumbers, midiChannel) {
        super(deckNumbers);

        const theDeck = this;

        this.beatLoop4 = new components.Button({
            midi: [0x90 + midiChannel, 0x14],
            shiftChannel: false,
            shiftControl: true,
            shiftOffset: 60,
            key: "beatloop_4",
            type: components.Button.prototype.types.toggle,
        });

        this.halfLoop = new components.Button({
            key: "loop_halve",
        });

        this.doubleLoop = new components.Button({
            key: "loop_double",
        });

        this.quantize = new components.Button({
            midi: [0x90 + midiChannel, 0x35],
            shiftChannel: false,
            shiftControl: true,
            shiftOffset: 4,
            type: components.Button.prototype.types.toggle,
            key: "quantize",
        });

        this.slipReverse = new components.Button({
            key: "reverseroll",
            input: function(channel, control, value, status, group) {
                const target = theDeck.padMode.getPadModeInstance();
                // check if current Pad Selection supports slip mode, in this case forward to Pads
                if (target && target.slip && typeof target.slip === "function") {
                    target.slip(status, control, value);
                } else {
                    components.Button.prototype.input.call(this, channel, control, value, status, group);
                };
            }
        });

        this.masterTempo = new components.Button({
            midi: [0x90 + midiChannel, 0x1A],
            sendShifted: true,
            shiftChannel: false,
            shiftControl: true,
            shiftOffset: 70,
            type: components.Button.prototype.types.toggle,
            key: "keylock",
        });

        this.beatSync = new components.Button({
            midi: [0x90 + midiChannel, 0x58],
            shiftChannel: false,
            shiftControl: true,
            shiftOffset: 4,
            type: components.Button.prototype.types.toggle,
            key: "sync_enabled",
            unshift: function() {
                this.inKey = "sync_enabled";
            },
            shift: function() {
                this.inKey = "sync_leader";
            },
        });

        this.silentCue = new components.Button({
            midi: [0x90 + midiChannel, 0x68],
            shiftChannel: false,
            shiftControl: true,
            shiftOffset: 16,
            type: components.Button.prototype.types.toggle,
            key: "mute",
        });

        this.keyMinus = new components.Button({
            midi: [0x90 + midiChannel, 0x0A],
            shiftChannel: false,
            shiftControl: true,
            shiftOffset: 0x5B,
            type: components.Button.prototype.types.push,
            key: "pitch",
            unshift: function() {
                this.type = components.Button.prototype.types.push;
                this.inKey = "pitch_down";
            },
            shift: function() {
                this.type = components.Button.prototype.types.toggle;
                this.inKey = "sync_key";
            },
            input: function(channel, control, value, status, group) {
                const target = theDeck.padMode.getPadModeInstance();
                // check if current Pad Selection supports key modifications, in this case forward to Pads
                if (this.inKey === "pitch_down" && target && target.pitchDown && typeof target.pitchDown === "function") {
                    target.pitchDown(status, control, value);
                } else if (this.inKey === "sync_key" && target && target.syncKey && typeof target.syncKey === "function") {
                    target.syncKey(status, control, value);
                } else {
                    components.Button.prototype.input.call(this, channel, control, value, status, group);
                }
            },
            outValueScale: function(value) {
                return (value < -0.1)?this.on:this.off;
            }
        });

        this.keyPlus = new components.Button({
            midi: [0x90 + midiChannel, 0x79],
            shiftChannel: false,
            shiftControl: true,
            shiftOffset: 0xEB,
            type: components.Button.prototype.types.push,
            key: "pitch",
            unshift: function() {
                this.inKey = "pitch_up";
            },
            shift: function() {
                this.inKey = "reset_key";
            },
            input: function(channel, control, value, status, group) {
                const target = theDeck.padMode.getPadModeInstance();
                // check if current Pad Selection supports key modifications, in this case forward to Pads
                if (this.inKey === "pitch_up" && target && target.pitchUp && typeof target.pitchUp === "function") {
                    target.pitchUp(status, control, value);
                } else if (this.inKey === "reset_key" && target && target.resetKey && typeof target.resetKey === "function") {
                    target.resetKey(status, control, value);
                } else {
                    components.Button.prototype.input.call(this, channel, control, value, status, group);
                }
            },
            outValueScale: function(value) {
                return (value > 0.1)?this.on:this.off;
            }
        });

        this.loadTrack = new components.Button({
            type: components.Button.prototype.types.push,
            key: "LoadSelectedTrack",
            clickTime: 0,
            timer: undefined,
            input: function(channel, control, value, status, group) {
                if (this.inKey === "LoadSelectedTrack" && value) {
                    if ((Date.now() - this.clickTime) < 500) { // double-click
                        this.clickTime = 0;
                        engine.stopTimer(this.timer);
                        engine.setValue(theDeck.currentDeck, "CloneFromDeck", ((script.deckFromGroup(theDeck.currentDeck) + 2) % 4 + 1));
                    } else {
                        this.clickTime = Date.now();
                        // start timer to activate function if no double-click is detected
                        this.timer = engine.beginTimer(510, function() {
                            components.Button.prototype.input.call(theDeck.loadTrack, channel, control, value, status, group);
                        }, true);
                    }
                } else {
                    components.Button.prototype.input.call(theDeck.loadTrack, channel, control, value, status, group);
                }
            },
            unshift: function() {
                this.inKey = "LoadSelectedTrack";
                this.group = theDeck.currentDeck;
            },
            shift: function() {
                this.inKey = "LoadSelectedTrackAndPlay";
                this.group = "[PreviewDeck1]";
            },
        });

        this.playButton = new components.PlayButton({
            type: components.Button.prototype.types.toggle,
            shift: function() {
                this.inKey = "play"; // override reverseplay - there is a separate button for this
            },
        });

        this.parameterLeft = new components.Button({
            PadModeControls: [0x24, 0x25, 0x26, 0x27, 0x28, 0x19, 0x2A, 0x2B],
            shiftedPadModeControls: [0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08],
            input: function(channel, control, value, status, group) {
                let position = this.PadModeControls.indexOf(control);
                if (position !== -1) {
                    const target = Object.values(theDeck.pads)[position];
                    if (target.parameterLeft && typeof target.parameterLeft.input === "function") {
                        target.parameterLeft.input(channel, control, value, status, group);
                    }
                }
                position = this.shiftedPadModeControls.indexOf(control);
                if (position !== -1) {
                    const target = Object.values(theDeck.pads)[position];
                    if (target.shiftParameterLeft && typeof target.shiftParameterLeft.input === "function") {
                        target.shiftParameterLeft.input(channel, control, value, status, group);
                    }
                }
            }
        });

        this.parameterRight = new components.Button({
            PadModeControls: [0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33],
            shiftedPadModeControls: [0x09, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x00],
            input: function(channel, control, value, status, group) {
                let position = this.PadModeControls.indexOf(control);
                if (position !== -1) {
                    const target = Object.values(theDeck.pads)[position];
                    if (target.parameterRight && typeof target.parameterRight.input === "function") {
                        target.parameterRight.input(channel, control, value, status, group);
                    }
                }
                position = this.shiftedPadModeControls.indexOf(control);
                if (position !== -1) {
                    const target = Object.values(theDeck.pads)[position];
                    if (target.shiftParameterRight && typeof target.shiftParameterRight.input === "function") {
                        target.shiftParameterRight.input(channel, control, value, status, group);
                    }
                }
            }
        });

        this.padMode = new components.Button({
            PadModeControls: [0x1B, 0x1E, 0x20, 0x22, 0x69, 0x6B, 0x6D, 0x6F],
            currentMode: 0,
            getPadModeInstance: function(position) {
                if (position === undefined) {
                    position = this.currentMode;
                }
                return Object.values(theDeck.pads)[position];
            },
            init: function(position, status, control, value) {
                const target = this.getPadModeInstance(position);
                if (target.init && typeof target.init === "function") {
                    target.init(status, control, value);
                }
            },
            input: function(_channel, control, value, status, _group) {
                if (value) {
                    const position = this.PadModeControls.indexOf(control);
                    if (position !== -1) {
                        if (this.currentMode === position) {
                            this.init(position, status, control, 2);     // retrigger
                        } else {
                            this.init(this.currentMode, status, control, 0);    // de-init
                            this.init(position, status, control, 1);     // init
                            this.currentMode = position;
                        }
                    };
                };
            },
        });

        // The controlFX buttons are hardware one-out-of-three controlled. The ThreeButtonSelector
        // applies this behavior also to the EffectUnits for the different Decks and therefore
        // has to be connected to all four Decks to react on GUI inputs.
        // On the other hand there are only two physical FX button groups on the controller, the input
        // will therefore land on DeckControls2Deck.controlFX and will be just forwarded to the
        // controller of the chosen Deck.
        this.controlFX = new DDJXP2.ThreeButtonSelector({
            midibase: [[0x94 + (midiChannel % 2), 0x70], [0x94 + (midiChannel % 2), 0x71], [0x94 + (midiChannel % 2), 0x72]],
            sendShifted: true,
            shiftChannel: false,
            shiftControl: true,
            shiftOffset: 3,
            offset: 0x70,
            key: `group_[Channel${midiChannel + 1}]_enable`,
            groupArray: ["[EffectRack1_EffectUnit1]", "[EffectRack1_EffectUnit2]", "[EffectRack1_EffectUnit3]"],
            setExternalModifier: function(position) {
                Object.values(DDJXP2.controls2deck)[(script.deckFromGroup(this.group) - 1) % 2].fader.useForEffects(position, this.groupArray[position - 1], "super1");
            }
        });

        const modeBtnColor1 = DDJXP2.RGBPioneerCode(80, 0, 255);
        const modeBtnAttnColor1 = DDJXP2.RGBPioneerCode(255, 0, 80);
        const modeBtnColor2 = DDJXP2.RGBPioneerCode(255, 255, 0);
        const modeBtnAttnColor2 = DDJXP2.RGBPioneerCode(255, 40, 0);

        this.pads = new components.ComponentContainer({
            one: new DDJXP2.PadModeContainers[engine.getSetting("pad1")](0, midiChannel, this.currentDeck, modeBtnColor1, modeBtnAttnColor1),
            two: new DDJXP2.PadModeContainers[engine.getSetting("pad2")](1, midiChannel, this.currentDeck, modeBtnColor1, modeBtnAttnColor1),
            three: new DDJXP2.PadModeContainers[engine.getSetting("pad3")](2, midiChannel, this.currentDeck, modeBtnColor1, modeBtnAttnColor1),
            four: new DDJXP2.PadModeContainers[engine.getSetting("pad4")](3, midiChannel, this.currentDeck, modeBtnColor1, modeBtnAttnColor1),
            five: new DDJXP2.PadModeContainers[engine.getSetting("pad5")](4, midiChannel, this.currentDeck, modeBtnColor2, modeBtnAttnColor2),
            six: new DDJXP2.PadModeContainers[engine.getSetting("pad6")](5, midiChannel, this.currentDeck, modeBtnColor2, modeBtnAttnColor2),
            seven: new DDJXP2.PadModeContainers[engine.getSetting("pad7")](6, midiChannel, this.currentDeck, modeBtnColor2, modeBtnAttnColor2),
            eight: new DDJXP2.PadModeContainers[engine.getSetting("pad8")](7, midiChannel, this.currentDeck, modeBtnColor2, modeBtnAttnColor2),
        });
        this.pads.one.isActivePadMode = true;

        this.forEachComponent(function(component) {
            if (component.group === undefined) {
                component.group = this.currentDeck;
            };
        });
    }
};

DDJXP2.DeckControls2Deck = class extends components.Deck {
    constructor(deckNumbers, midiChannel) {
        super(deckNumbers);

        const theDeck = this;

        this.fader = new components.Pot({
            softTakeover: false,
            resetFader: function(_channel, control, value, status, group) {
                this.inputMSB(_channel, control, 0x00, status, group);
                this.inputLSB(_channel, control, 0x00, status, group);
            },
            useForEffects: function(enable, group, key) {
                if (enable) {
                    this.group = group;
                    this.inKey = key;
                } else if (engine.getSetting("useFaderForDeckVolume")) {
                    this.group = theDeck.currentDeck;
                    this.inKey = "volume";
                } else {
                    this.inKey = undefined;
                }
            }
        });

        // mapping between two physical and four GUI FX control groups. Just a for-
        // warding to the chosen Deck, see DeckControls4Deck.controlFX for more
        // information
        this.controlFX = new components.Button({
            input: function(channel, control, value, status, group) {
                group = this.group;
                DDJXP2.controls4deck.forEachComponentContainer(function(componentContainer) {
                    if (componentContainer.controlFX && (componentContainer.controlFX.group === group)) {
                        componentContainer.controlFX.input(channel, control, value, status, group);
                    };
                });
            },
        });

        this.toggleDeck = new components.Button({
            baseGroup: theDeck.currentDeck,
            input: function(_channel, control, value, status, _group) {
                if (value) {
                    theDeck.toggle();
                    const INTBtnOnBaseDeck = (status === 0x90 + midiChannel && control === 0x73);
                    const INTBtnOnAlternateDeck = (status === 0x92 + midiChannel && control === 0x73);
                    const shiftINTOnBaseDeck = (status === 0x92 + midiChannel && control === 0x72 && this.group === this.baseGroup);
                    if (INTBtnOnBaseDeck) {
                        midi.sendShortMsg(0x92 + midiChannel, 0x72, 0x7F); // switch controller to base deck
                    };
                    if (INTBtnOnAlternateDeck) {
                        midi.sendShortMsg(0x92 + midiChannel, 0x72, 0x00); // switch controller to alternate deck
                    };
                    if (INTBtnOnBaseDeck || shiftINTOnBaseDeck) {
                        midi.sendShortMsg(0x92 + midiChannel, 0x73, 0x7F); // light up button
                    };
                }
            }
        });

        this.forEachComponent(function(component) {
            if (component.group === undefined) {
                component.group = this.currentDeck;
            };
        });
    }

    toggle() {
        let currentDeck = this.currentDeck;
        DDJXP2.controls4deck.forEachComponentContainer(function(componentContainer) {
            if (componentContainer.controlFX && componentContainer.controlFX.group === currentDeck) {
                componentContainer.controlFX.activate(0);
            };
        });

        try {
            engine.setValue("[Skin]", `highlight_deck_${this.currentDeck}`, 0);
            engine.setValue("[Skin]", `highlight_waveform_${this.currentDeck}`, 0);
            engine.setValue("[Skin]", `highlight_mixer_${this.currentDeck}`, 0);
        } finally {
            // continue anyway
        };
        super.toggle();
        try {
            engine.setValue("[Skin]", `highlight_deck_${this.currentDeck}`, 1);
            engine.setValue("[Skin]", `highlight_waveform_${this.currentDeck}`, 1);
            engine.setValue("[Skin]", `highlight_mixer_${this.currentDeck}`, 1);
        } finally {
            // continue anyway
        };

        currentDeck = this.currentDeck;
        DDJXP2.controls4deck.forEachComponentContainer(function(componentContainer) {
            if (componentContainer.controlFX && componentContainer.controlFX.group === currentDeck) {
                componentContainer.controlFX.activate(1);
            }
        });
    }
    setCurrentDeck(newGroup) {
        if (this.fader.baseGroup) {
            this.fader.group = this.fader.baseGroup;
        }
        super.setCurrentDeck(newGroup);
    }
};
