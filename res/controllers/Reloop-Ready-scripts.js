// eslint-ignore-next-line no-var
var ReloopReady = {};

/// The controller only offers Low, High and Gain Knobs,
/// settings this to true will remap them to control Low, Mid, High instead.
ReloopReady.threeBandEQ = false;

ReloopReady.backlightButtons = true;

/// scratch parameter, change to your liking
ReloopReady.scratchParams = {
    alpha: 1/8,
    beta: (1/8)/32,
};

/// On my hardware unit, the tempo faders have such a cheap build-quality
/// that the notches in the center are not actually in the middle
/// If you find your notches are offset too, do this:
/// Load this mapping in mixxx using `--developer` mode.
/// Then move the tempo fader into the notch at the position where
/// the tempo should then be the true tempo (+/- 0). Open the developer tools
/// and read the `rate` value of the channel. If its already 0.5 you should be
/// good, if its not that, copy the value here and the fader will be adjusted
/// for you.
ReloopReady.tempoFaderMiddles = [0.5, 0.5];


/**
 * Misc. midi notes
 *
 * Note 0x31 firmware fader-start feature. Note 0x7F if volume fader is non-zero
 */


/**
 * Color:
 * The Reloop ready uses the entire 7-bit midi range for color.
 * It encodes it in a simple to understand (but inefficient in regards to
 * colors perceived by us) scheme:
 * It uses the lower 6-bit for RGB colors (each channel having the unbelievable
 * resolution of 2-bit), while the 7th-bit is used to switch between two
 * brightness/intensity modes (with the 7th-bit set, meaning high brightness).
 *
 * |7654321|
 * +-------+
 * |IRRGGBB|
 * Example: the fullest, brightest red would have I set and the red channel on
 * maximum: 0b1110000
 *
 * Using that knowledge, we can extrapolate a 24-bit color from any 7-bit color:
 */

ReloopReady.midiToFullColor = color => {

    const b = (color & 0b00000011) << 6;
    const g = (color & 0b00001100) << 4;
    const r = (color & 0b00110000) << 2;
    const i = (color & 0b01000000) >> 6;

    if (i === 0) {
        // half the RGB intensity
        r >> 0b1;
        g >> 0b1;
        b >> 0b1;
    }
    return (r << 16) | (g << 8) | b;
};

ReloopReady.padColorPalette = {
    Off: 0x000000,
    Red: 0xFF0000,
    Green: 0x00FF00,
    Blue: 0x0000FF,
    Yellow: 0xFFFF00,
    Cyan: 0x007FFF,
    Purple: 0xFF00FF,
    White: 0xFFFFFF,
};

ReloopReady.requestFirmwareVersionSysex = [0xF0, 0x00, 0x20, 0x7F, 0x19];

// dim color by stripping the "intensity bit"
ReloopReady.dimColor = color => color & 0b00111111;

ReloopReady.noopInputHandler = function(_channel, _control, _value, _status, _group) {};

// IIFE for not leaking "private" variables
(() => {

    // rewrite the following without lodash
    const fullColorToMidiColorMap = _(_.range(0x00, 0x80))
        .keyBy()
        .mapKeys(ReloopReady.midiToFullColor)
        .value();

    ReloopReady.padColorMapper = new ColorMapper(fullColorToMidiColorMap);

    // transform padColorPalette to low-res midi approximations:

    ReloopReady.padColorPalette = _.mapValues(ReloopReady.padColorPalette, color => {
        const litColor = ReloopReady.padColorMapper.getValueForNearestColor(color);
        const dimColor = ReloopReady.dimColor(litColor);
        return {
            lit: litColor,
            dim: dimColor,
        };
    });
})();

components.Button.prototype.sendShifted = true;
components.Button.prototype.shiftControl = true;
// shift control offset depends on concrete control

components.Encoder.prototype.inValueScale = function(value) {
    return value < 0x40 ? value : value - 0x80;
};

ReloopReady.singleColorLED = {
    off: 0x00,
    dim: 0x01,
    lit: 0x7F, // anything between 0x02 and 0x7F is fully lit
};

// make Buttons backlight be default based on user-setting
components.Button.prototype.off = ReloopReady.backlightButtons ? ReloopReady.singleColorLED.dim : ReloopReady.singleColorLED.off;
components.Button.prototype.on = ReloopReady.singleColorLED.lit;

/**
 * creates an this.isPress guarded input handler, assumes to be called by components.Button or subclasses thereof
 * @param {(value: number) => void} func callback that is called on ButtonDown
 * @returns {(channel: number, control: number, value: number, status: number, group: string) => void} XML input handler
 */
ReloopReady.makeButtonDownInputHandler = function(func) {
    return function(channel, control, value, status, _group) {
        const isPress = this.isPress(channel, control, value, status);
        this.output(isPress);
        if (!isPress) {
            return;
        }
        func.call(this, value);
    };
};

/**
 * Handle incoming sysex message
 * @param {Array<number>} data sysex data received from the controller
 */
ReloopReady.incomingData = data => {
    const arrayStartsWith = (arr, pattern) => pattern.every((elem, i) => elem === arr[i]);
    if (arrayStartsWith(data, ReloopReady.requestFirmwareVersionSysex)) {
        const firmwareVersion =  data.slice(ReloopReady.requestFirmwareVersionSysex.length, data.length - 1);
        // no idea if that's actually the format used for the firmware version,
        // its just 4 bytes (7-bit cuz midi) for me.
        console.log(`firmware version: ${firmwareVersion.join(".")}`);
        return;
    }
    console.warn(`unregonized incoming sysex data: ${data}`);
};

ReloopReady.init = function() {

    this.components = new components.ComponentContainer();

    this.components.leftDeck = new ReloopReady.Deck(0);
    this.components.rightDeck = new ReloopReady.Deck(1);
    this.components.leftChannel = new ReloopReady.Channel(0);
    this.components.rightChannel = new ReloopReady.Channel(1);

    this.components.crossfader = new components.Pot({
        midi: [0xBE, 0x08],
        group: "[Master]",
        inKey: "crossfader"
    });

    this.components.masterVol = new components.Pot({
        midi: [0xBE, 0x0A],
        group: "[Master]",
        inKey: "gain"
    });

    this.components.cueMix = new components.Pot({
        midi: [0xBE, 0x0D],
        group: "[Master]",
        inKey: "headMix"
    });

    this.components.cueVol = new components.Pot({
        midi: [0xBE, 0x0C],
        group: "[Master]",
        inKey: "headGain"
    });

    this.components.browse = new components.ComponentContainer({
        button: new components.Button({
            midi: [0x9E, 0x06],
            unshift: function() {
                this.group = "[Library]";
                this.inKey = "GoToItem";
                this.type = components.Button.prototype.types.push;
            },
            shift: function() {
                this.group = "[Master]";
                this.inKey = "maximize_library";
                this.type = components.Button.prototype.types.toggle;
            }
        }),
        knob: new components.Encoder({
            midi: [0xBE, 0x00],
            group: "[Library]",
            unshift: function() {
                this.inKey = "MoveVertical";
            },
            shift: function() {
                this.inKey = "MoveFocus";
            }
        })
    });

    const shiftableComponents = this.components;

    this.shift = new components.Button({
        midi: [0x9E, 0x00],
        input: function(channel, control, value, status, _group) {
            shiftableComponents.forEachComponent(comp => comp.disconnect());
            if (this.isPress(channel, control, value, status)) {
                shiftableComponents.shift();
            } else {
                shiftableComponents.unshift();
            }
            shiftableComponents.forEachComponent(comp => {
                comp.connect();
                comp.trigger();
            });
        },
    });

    // request controls status per standard serato sysex
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x00, 0xF7]);
    // request firmware version
    midi.sendSysexMsg(ReloopReady.requestFirmwareVersionSysex.concat([0xF7]));
};

ReloopReady.shutdown = () => {
    ReloopReady.components.shutdown();
};


ReloopReady.Channel = function(index) {
    const channel = index + 1;
    const group = `[Channel${channel}]`;
    const eqGroup = `[EqualizerRack1_${group}_Effect1]`;

    if (ReloopReady.threeBandEQ) {
        this.knob1 = new components.Pot({
            midi: [0xB0 + index, 0x16],
            group: eqGroup,
            inKey: "parameter3"
        });
        this.knob2 = new components.Pot({
            midi: [0xB0 + index, 0x17],
            group: eqGroup,
            inKey: "parameter2"
        });
    } else {
        this.knob1 = new components.Pot({
            midi: [0xB0 + index, 0x16],
            inKey: "pregain"
        });

        this.knob2 = new components.Pot({
            midi: [0xB0 + index, 0x17],
            group: eqGroup,
            inKey: "parameter3"
        });
    }

    this.knob3 = new components.Pot({
        midi: [0xB0 + index, 0x19],
        group: eqGroup,
        inKey: "parameter1"
    });


    this.filter = new components.Pot({
        midi: [0xB0 + index, 0x1A],
        group: `[QuickEffectRack1_${group}]`,
        inKey: "super1"
    });

    this.rate = new components.Pot({
        midi: [0xB0 + index, 0x09], // MSB control: 0x3F
        inKey: "rate",
        // tempo fader response: top ("-") @ 0, bottom ("+") @ 1023
        max: 1023,
        invert: true,
        // see explanation of ReloopReady.tempoFaderMiddles for info
        inValueScale: function(value) {
            const middle = (this.max + 1) * ReloopReady.tempoFaderMiddles[index];
            return script.absoluteNonLinInverse(value, 0, middle, this.max, 0, 1);
        }
    });

    // TODO this seems to have soft-takeover enabled sometimes?!
    this.volume = new components.Pot({
        midi: [0xB0 + index, 0x1C],
        inKey: "volume",
    });

    this.pfl = new components.Button({
        midi: [0x90 + index, 0x1B],
        shiftOffset: -0xD,
        key: "pfl",
        type: components.Button.prototype.types.toggle,
    });

    this.load = new components.Button({
        midi: [0x9E, 0x02 + index],
        shiftOffset: 0x0D,
        shift: function() {
            this.inKey = "eject";
            this.outKey = this.inKey;
        },
        unshift: function() {
            this.inKey = "LoadSelectedTrack";
            this.outKey = this.inKey;
        }
    });


    this.reconnectComponents(c => {
        if (c.group === undefined) {
            c.group = group;
        }
    });
};

ReloopReady.Channel.prototype = new components.ComponentContainer();

ReloopReady.FxUnit = function(index) {

    const midiOn = 0x98 + index;
    const channel = index + 1;

    this.level = new components.Pot({
        midi: [0xB8 + index, 0x00],
        group: `[EffectRack1_EffectUnit${channel}]`,
        inKey: "super1",
    });
    this.fxButtons = Array.from([0x00, 0x01, 0x02, 0x03], control =>
        new components.Button({
            midi: [midiOn, control],
            shiftOffset: 0xA,
            group: `[EffectRack1_EffectUnit${channel}_Effect${control + 1}]`,
            unshift: function() {
                this.inKey = "enabled";
                this.outKey = this.inKey;
                this.type = components.Button.prototype.types.toggle;
            },
            shift: function() {
                this.inKey = "next_effect";
                this.outKey = "loaded";
                this.type = components.Button.prototype.types.push;
            }
        })
    );

    this.loop = {
        knob: new components.Encoder({
            midi: [0xB4 + index, 0x03],
            group: `[Channel${channel}]`,
            input: function(_channel, _control, value, _status, _group) {
                engine.setValue(this.group, value > 0x40 ? "loop_halve" : "loop_double", 1);
            }
        }),
        button: new components.Button({
            midi: [0x94 + index, 0x40], // shifted: [0x98 + index, 0x04]
            group: `[Channel${channel}]`,
            input: function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status)) {
                    if (this.shifted) {
                        // non standard behavior
                        script.bpm.tapButton(index);
                    } else {
                        // TODO: For 2.4 using plain beatloop_activate
                        // is sufficient, see #4328
                        if (engine.getValue(this.group, "loop_enabled")) {
                            script.triggerControl(this.group, "reloop_toggle");
                        } else {
                            script.triggerControl(this.group, "beatloop_activate");
                        }
                    }
                }
            },
        })
    };
};

ReloopReady.FxUnit.prototype = new components.ComponentContainer();


ReloopReady.PadMode = function(obj) {
    components.ComponentContainer.call(this, obj);
    // interpret this an interface definition
    // we fill this array to avoid the issues with iteration methods and sparse arrays
    this.pads = Array.from(Array(8), (_, i) => 0x14 + i);
    this.parameterLeft  = new components.Button();
    this.parameterRight = new components.Button();
    this.shutdown = () => {
        // we want the pads to be completely off when shutdown, not dimmed
        this.pads.forEach(pad => pad.send(0x00));
        this.parameterLeft.shutdown();
        this.parameterRight.shutdown();
    };
};

ReloopReady.PadMode.prototype = Object.create(components.ComponentContainer.prototype);

ReloopReady.HotcuePadMode = function(index) {
    ReloopReady.PadMode.call(this);
    // can't use map on sparse arrays, so we use.from
    this.pads = this.pads.map((control, i) =>
        new components.HotcueButton({
            midi: [0x94 + index, control],
            group: `[Channel${index + 1}]`,
            shiftOffset: 0x8,
            off: 0x00,
            number: i + 1,
            colorMapper: ReloopReady.padColorMapper,
            outConnect: false,
        })
    );
};

ReloopReady.HotcuePadMode.prototype = Object.create(ReloopReady.PadMode.prototype);

ReloopReady.AutoLoopPadMode = function(index) {
    ReloopReady.PadMode.call(this);

    this.currentLoopSizeExp = -2;
    const theContainer = this;

    const clampLoopSizeExp = loopSizeExp => _.clamp(loopSizeExp, -5, -2);

    this.setLoopSizes = loopSizeExp => {
        _(_.range(loopSizeExp, loopSizeExp + 8))
            .map(function(exp) { return Math.pow(2, exp); })
            .zip(this.pads)
            .forEach(([size, pad]) => {
                pad.inKey = `beatloop_${size}_toggle`;
                pad.outKey = `beatloop_${size}_enabled`;
            });
    };

    for (let i = 0; i < this.pads.length; i++) {
        this.pads[i] = new components.Button({
            midi: [0x94 + index, 0x14 + i],
            group: `[Channel${index + 1}]`,
            shiftOffset: 0x8,
            off: ReloopReady.padColorPalette.Red.dim,
            on: ReloopReady.padColorPalette.Red.lit,
            outConnect: false,
        });
    }

    const makeParameterInputHandler = loopSizeChangeAmount => {
        return function(channel, control, value, status, _group) {
            const pressed = this.isPress(channel, control, value, status);
            if (pressed) {
                const newLoopSize = clampLoopSizeExp(theContainer.currentLoopSizeExp + loopSizeChangeAmount);
                if (newLoopSize !== theContainer.currentLoopSizeExp) {
                    theContainer.currentLoopSizeExp = newLoopSize;
                    theContainer.setLoopSizes(newLoopSize);
                    theContainer.reconnectComponents();
                }
            }
            this.output(pressed);
        };
    };

    this.parameterLeft  = new components.Button({
        midi: [0x94 + index, 0x28], //shifted control: 0x2A
        shiftOffset: 0x2,
        input: makeParameterInputHandler(-1),
    });
    this.parameterRight = new components.Button({
        midi: [0x94 + index, 0x29], //shifted control: 0x2B
        shiftOffset: 0x2,
        input: makeParameterInputHandler(1),
    });

    this.setLoopSizes(this.currentLoopSizeExp);
};

ReloopReady.AutoLoopPadMode.prototype = Object.create(ReloopReady.PadMode.prototype);

ReloopReady.ManualLoopPadMode = function(index) {
    ReloopReady.PadMode.call(this);

    let loopMoveBeats = 4;
    const group = `[Channel${index + 1}]`;

    this.pads[0] = new components.Button({
        key: "loop_in",
        on: ReloopReady.padColorPalette.Blue.lit,
        off: ReloopReady.padColorPalette.Blue.dim,
    });
    this.pads[1] = new components.Button({
        key: "loop_out",
        on: ReloopReady.padColorPalette.Blue.lit,
        off: ReloopReady.padColorPalette.Blue.dim,
    });
    this.pads[2] = new components.Button({
        inKey: "reloop_toggle",
        outKey: "loop_enabled",
        on: ReloopReady.padColorPalette.Red.lit,
        off: ReloopReady.padColorPalette.Red.dim,
    });
    this.pads[3] = new components.Button({
        key: "loop_in_goto",
        on: ReloopReady.padColorPalette.Red.lit,
        off: ReloopReady.padColorPalette.Red.dim,
    });
    this.pads[4] = new components.Button({
        key: "loop_halve",
        on: ReloopReady.padColorPalette.Green.lit,
        off: ReloopReady.padColorPalette.Green.dim,
    });
    this.pads[5] = new components.Button({
        key: "loop_double",
        on: ReloopReady.padColorPalette.Green.lit,
        off: ReloopReady.padColorPalette.Green.dim,
    });
    this.pads[6] = new components.Button({
        inKey: "loop_move",
        on: ReloopReady.padColorPalette.Cyan.lit,
        off: ReloopReady.padColorPalette.Cyan.dim,
        unshift: function() {
            this.input = ReloopReady.makeButtonDownInputHandler(function() {
                this.inSetValue(-loopMoveBeats);
            });
        },
        shift: function() {
            this.input = ReloopReady.makeButtonDownInputHandler(function() {
                this.inSetValue(-engine.getValue(this.group, "loop_size"));
            });
        },
        trigger: function() {
            this.output();
        },
    });
    this.pads[7] = new components.Button({
        inKey: "loop_move",
        on: ReloopReady.padColorPalette.Cyan.lit,
        off: ReloopReady.padColorPalette.Cyan.dim,
        unshift: function() {
            this.input = ReloopReady.makeButtonDownInputHandler(function() {
                this.inSetValue(loopMoveBeats);
            });
        },
        shift: function() {
            this.input = ReloopReady.makeButtonDownInputHandler(function() {
                this.inSetValue(engine.getValue(this.group, "loop_size"));
            });
        },
        trigger: function() {
            this.output();
        },
    });

    this.pads.forEach((pad, i) => pad.midi = [0x94 + index, 0x14 + i]);

    this.parameterLeft  = new components.Button({
        midi: [0x94 + index, 0x28],
        shiftOffset: 0x2,
        input: ReloopReady.makeButtonDownInputHandler(function() {
            loopMoveBeats *= 0.5;
        }),
    });
    this.parameterRight = new components.Button({
        midi: [0x94 + index, 0x29],
        shiftOffset: 0x2,
        input: ReloopReady.makeButtonDownInputHandler(function() {
            loopMoveBeats *= 2;
        }),
    });

    this.reconnectComponents(c => {
        if (c.group === undefined) {
            c.group = group;
        }
    });
};
ReloopReady.ManualLoopPadMode.prototype = Object.create(ReloopReady.PadMode.prototype);


ReloopReady.SamplerPadMode = function(index) {
    ReloopReady.PadMode.call(this, index);

    // var baseOffset = index * 8;

    this.pads = this.pads.map((_, i) =>
        new components.SamplerButton({
            midi: [0x94 + index, 0x14 + i],
            // number: baseOffset + i + 1,
            number: i + 1,
            shiftOffset: 0x8,
            off: 0x00,
            outConnect: false,
        })
    );
};
ReloopReady.SamplerPadMode.prototype = Object.create(ReloopReady.PadMode.prototype);


ReloopReady.LoopRollPadMode = function(index) {
    ReloopReady.PadMode.call(this);

    this.currentLoopSizeExp = -2;
    const theContainer = this;

    const clampLoopSizeExp = loopSizeExp => _.clamp(loopSizeExp, -5, -2);

    this.setLoopSizes = loopSizeExp => {
        _(_.range(loopSizeExp, loopSizeExp + 8))
            .map(function(exp) { return Math.pow(2, exp); })
            .zip(this.pads)
            .forEach(([size, pad]) => {
                pad.inKey = `beatlooproll_${size}_activate`;
                pad.outKey = `beatloop_${size}_enabled`;
            });
    };

    for (let i = 0; i < this.pads.length; i++) {
        this.pads[i] = new components.Button({
            midi: [0x94 + index, 0x14 + i],
            group: `[Channel${index + 1}]`,
            shiftOffset: 0x8,
            off: ReloopReady.padColorPalette.Green.dim,
            on: ReloopReady.padColorPalette.Green.lit,
            outConnect: false,
        });
    }

    const makeParameterInputHandler = loopSizeChangeAmount =>
        ReloopReady.makeButtonDownInputHandler(function() {
            const newLoopSize = clampLoopSizeExp(theContainer.currentLoopSizeExp + loopSizeChangeAmount);
            if (newLoopSize !== theContainer.currentLoopSizeExp) {
                theContainer.currentLoopSizeExp = newLoopSize;
                theContainer.setLoopSizes(newLoopSize);
                theContainer.reconnectComponents();
            }
        });

    this.parameterLeft  = new components.Button({
        midi: [0x94 + index, 0x28],
        shiftOffset: 0x2,
        input: makeParameterInputHandler(-1),
    });
    this.parameterRight = new components.Button({
        midi: [0x94 + index, 0x29],
        shiftOffset: 0x2,
        input: makeParameterInputHandler(1),
    });

    this.setLoopSizes(this.currentLoopSizeExp);
};

ReloopReady.LoopRollPadMode.prototype = Object.create(ReloopReady.PadMode.prototype);


// Pitch Play mode taken from Roland DJ 505 mapping.
ReloopReady.Pitch = function(index) {
    components.ComponentContainer.call(this);

    const PitchPlayRange = {
        UP: 0,
        MID: 1,
        DOWN: 2,
    };

    //this.ledControl = DJ505.PadMode.SAMPLER;
    const color = ReloopReady.padColorPalette.Purple;
    let cuepoint = 1;
    let range = PitchPlayRange.MID;
    const theContainer = this;

    this.PerformancePad = function(n) {
        this.midi = [0x94 + index, 0x14 + n];
        this.number = n + 1;
        this.on = ReloopReady.padColorPalette.Purple.dim;
        this.colorMapper = ReloopReady.padColorMapper;
        this.colorKey = `hotcue_${this.number}_color`;
        components.Button.call(this);
    };
    this.PerformancePad.prototype = new components.Button({
        shiftOffset: 8,
        group: `[Channel${index + 1}]`,
        outConnect: false,
        off: ReloopReady.padColorPalette.Off.lit,
        outputColor: function(colorCode) {
            // For colored hotcues (shifted only)
            const midiColor = this.colorMapper.getValueForNearestColor(colorCode);
            this.send((cuepoint === this.number) ? midiColor : ReloopReady.dimColor(midiColor));
        },
        unshift: function() {
            this.outKey = "pitch_adjust";
            this.output = function(_value, _group, _control) {
                let midiColor = color.dim;
                if ((range === PitchPlayRange.UP && this.number === 5) ||
                    (range === PitchPlayRange.MID && this.number === 1) ||
                    (range === PitchPlayRange.DOWN && this.number === 4)) {
                    midiColor = ReloopReady.padColorPalette.White.lit;
                }
                this.send(midiColor);
            };
            this.input = function(_channel, _control, value, _status, _group) {
                const pitchAdjust = (function() {
                    switch (range) {
                    case PitchPlayRange.UP:
                        return this.number + ((this.number <= 4) ? 4 : -5);
                    case PitchPlayRange.MID:
                        return this.number - ((this.number <= 4) ? 1 : 9);
                    case PitchPlayRange.DOWN:
                        return this.number - ((this.number <= 4) ? 4 : 12);
                    }
                }).call(this);
                engine.setValue(this.group, "pitch_adjust", pitchAdjust);
                engine.setValue(this.group, `hotcue_${cuepoint}_activate`, value);
            };
            this.connect = function() {
                components.Button.prototype.connect.call(this); // call parent connect

                if (this.connections[1] !== undefined) {
                    // Necessary, since trigger() apparently also triggers disconnected connections
                    this.connections.pop();
                }
            };
            if (this.connections[0] !== undefined) {
                this.disconnect();
                this.connect();
                this.trigger();
            }
        },
        shift: function() {
            this.outKey = `hotcue_${this.number}_enabled`;
            this.output = function(value, _group, _control) {
                const outval = this.outValueScale(value);
                if (this.colorKey !== undefined && outval !== this.off) {
                    this.outputColor(engine.getValue(this.group, this.colorKey));
                } else {
                    this.send(ReloopReady.padColorPalette.Off.lit);
                }
            };
            this.input = function(_channel, _control, value, _status, _group) {
                if (value > 0 && cuepoint !== this.number && engine.getValue(this.group, `hotcue_${this.number}_enabled`)) {
                    const previousCuepoint = cuepoint;
                    cuepoint = this.number;
                    theContainer.pads[previousCuepoint - 1].trigger();
                    this.outputColor(engine.getValue(this.group, this.colorKey));
                }
            };
            this.connect = function() {
                components.Button.prototype.connect.call(this); // call parent connect
                if (undefined !== this.group && this.colorKey !== undefined) {
                    this.connections[1] = engine.makeConnection(this.group, this.colorKey, function(id) {
                        if (engine.getValue(this.group, this.outKey)) {
                            this.outputColor(id);
                        }
                    });
                }
            };
            if (this.connections[0] !== undefined) {
                this.disconnect();
                this.connect();
                this.trigger();
            }
        },
    });
    this.pads = new components.ComponentContainer();
    for (let n = 0; n <= 7; n++) {
        this.pads[n] = new this.PerformancePad(n);
    }

    this.parameterLeft = new components.Button({
        midi: [0x94 + index, 0x28],
        shiftOffset: 0x2,
        input: ReloopReady.makeButtonDownInputHandler(function() {
            if (range === PitchPlayRange.UP) {
                range = PitchPlayRange.MID;
            } else if (range === PitchPlayRange.MID) {
                range = PitchPlayRange.DOWN;
            } else {
                range = PitchPlayRange.UP;
            }
            theContainer.forEachComponent(c => c.trigger());
        }),
    });
    this.parameterRight = new components.Button({
        midi: [0x94 + index, 0x29],
        shiftOffset: 0x2,
        input: ReloopReady.makeButtonDownInputHandler(function() {
            if (range === PitchPlayRange.UP) {
                range = PitchPlayRange.DOWN;
            } else if (range === PitchPlayRange.MID) {
                range = PitchPlayRange.UP;
            } else {
                range = PitchPlayRange.MID;
            }
            theContainer.forEachComponent(c => c.trigger());
        }),
    });
};

ReloopReady.Pitch.prototype = Object.create(ReloopReady.PadMode.prototype);


// There is no such thing as a scratch bank in Mixxx so I'm repurpusing this
// PadMode for beatjumping.
ReloopReady.ScratchBankPadMode = function(index) {
    ReloopReady.PadMode.call(this);

    this.currentJumpSizeExp = -2;
    const theContainer = this;

    const clampJumpSizeExp = jumpSizeExp => _.clamp(jumpSizeExp, -5, 6);

    this.setjumpSizeExp = jumpSizeExp =>  {
        const middle = this.pads.length / 2;
        let jumpsize = Math.pow(2, jumpSizeExp);
        for (let i = 0; i < middle; ++i) {
            const padTop = this.pads[i];
            padTop.inKey = `beatjump_${jumpsize}_forward`;
            padTop.outKey = padTop.inKey;
            const padBot = this.pads[i + middle];
            padBot.inKey = `beatjump_${jumpsize}_backward`;
            padBot.outKey = padBot.inKey;
            jumpsize *= 2;
        }
    };


    for (let i = 0; i < this.pads.length; i++) {
        this.pads[i] = new components.Button({
            midi: [0x94 + index, 0x14 + i],
            group: `[Channel${index + 1}]`,
            shiftOffset: 0x8,
            off: ReloopReady.padColorPalette.Cyan.dim,
            on: ReloopReady.padColorPalette.Cyan.lit,
            outConnect: false,
        });
    }

    const makeParameterInputHandler = jumpSizeChangeAmount =>
        ReloopReady.makeButtonDownInputHandler(function() {
            const newJumpSize = clampJumpSizeExp(theContainer.currentJumpSizeExp + jumpSizeChangeAmount);
            if (newJumpSize !== theContainer.currentJumpSizeExp) {
                theContainer.currentJumpSizeExp = newJumpSize;
                theContainer.setjumpSizeExp(newJumpSize);
                theContainer.reconnectComponents();
            }
        });

    this.parameterLeft  = new components.Button({
        midi: [0x94 + index, 0x28],
        shiftOffset: 0x2,
        input: makeParameterInputHandler(-1),
    });
    this.parameterRight = new components.Button({
        midi: [0x94 + index, 0x29],
        shiftOffset: 0x2,
        input: makeParameterInputHandler(1),
    });

    this.setjumpSizeExp(this.currentJumpSizeExp);
};

ReloopReady.ScratchBankPadMode.prototype = Object.create(ReloopReady.PadMode.prototype);

ReloopReady.BeatGridPadMode = function(index) {
    ReloopReady.PadMode.call(this);

    const group = `[Channel${index + 1}]`;

    this.pads[0] = new components.Button({
        key: "beats_translate_earlier",
        on: ReloopReady.padColorPalette.Blue.lit,
        off: ReloopReady.padColorPalette.Blue.dim,
    });
    this.pads[1] = new components.Button({
        key: "beats_translate_later",
        on: ReloopReady.padColorPalette.Blue.lit,
        off: ReloopReady.padColorPalette.Blue.dim,
    });
    this.pads[2] = new components.Button({
        key: "beats_adjust_faster",
        on: ReloopReady.padColorPalette.Red.lit,
        off: ReloopReady.padColorPalette.Red.dim,
        trigger: function() {
            this.output();
        },
    });
    this.pads[3] = new components.Button({
        key: "beats_adjust_slower",
        on: ReloopReady.padColorPalette.Red.lit,
        off: ReloopReady.padColorPalette.Red.dim,
        trigger: function() {
            this.output();
        },
    });
    this.pads[4] = new components.Button({
        key: "shift_cues_earlier",
        on: ReloopReady.padColorPalette.Green.lit,
        off: ReloopReady.padColorPalette.Green.dim,
        trigger: function() {
            this.output();
        },
    });
    this.pads[5] = new components.Button({
        key: "shift_cues_later",
        on: ReloopReady.padColorPalette.Green.dim,
        off: ReloopReady.padColorPalette.Green.dim,
        trigger: function() {
            this.output();
        },
    });
    this.pads[6] = new components.Button({
        key: "bpm_tap",
        on: ReloopReady.padColorPalette.Cyan.dim,
        off: ReloopReady.padColorPalette.Cyan.dim,
    });
    this.pads[7] = new components.Button({
        key: "beats_translate_curpos",
        on: ReloopReady.padColorPalette.Cyan.lit,
        off: ReloopReady.padColorPalette.Cyan.dim,
    });

    this.pads.forEach((pad, i) => pad.midi = [0x94 + index, 0x14 + i]);

    this.parameterLeft  = new components.Button({
        midi: [0x94 + index, 0x28],
        shiftOffset: 0x2
    });
    this.parameterRight = new components.Button({
        midi: [0x94 + index, 0x29],
        shiftOffset: 0x2
    });

    this.reconnectComponents(c => {
        if (c.group === undefined) {
            c.group = group;
        }
    });
};
ReloopReady.BeatGridPadMode.prototype = Object.create(ReloopReady.PadMode.prototype);

// Ordering of array elements determines layout of pad mode selectors.
ReloopReady.controlPadModeAssoc = [
    {pos: 0, control: 0x00, mode: ReloopReady.HotcuePadMode},
    {pos: 1, control: 0x05, mode: ReloopReady.AutoLoopPadMode},
    {pos: 2, control: 0x0E, mode: ReloopReady.ManualLoopPadMode},
    {pos: 3, control: 0x0B, mode: ReloopReady.SamplerPadMode},
    {pos: 4, control: 0x0F, mode: ReloopReady.Pitch},
    {pos: 5, control: 0x09, mode: ReloopReady.ScratchBankPadMode},
    {pos: 6, control: 0x08, mode: ReloopReady.LoopRollPadMode},
    {pos: 7, control: 0x12, mode: ReloopReady.BeatGridPadMode},
];

ReloopReady.PadContainer = function(index) {

    // parent constructor is called later

    components.ComponentContainer.call(this);

    // construct instance of each mode per Container
    const padModeInstances = ReloopReady.controlPadModeAssoc.map(obj => {
        const modeInstance = new (obj.mode)(index);
        // make sure no multiple components have "ownership" over each pad
        modeInstance.forEachComponent(c => c.disconnect());
        return {
            pos: obj.pos,
            control: obj.control,
            modeInstance: modeInstance,
        };
    });

    this.currentLayer = padModeInstances[0].modeInstance;
    this.currentLayer.reconnectComponents();

    const applyLayer = layer => {
        if (this.currentLayer === layer) {
            return;
        }

        this.currentLayer.forEachComponent(c => c.disconnect());
        if (this.isShifted) {
            layer.shift();
        } else {
            layer.unshift();
        }

        layer.forEachComponent(c => {
            c.connect();
            c.trigger();
        });

        this.currentLayer = layer;

        this.padModeSelectors.forEach(selector => selector.trigger());
    };

    // factory/HO function for creating input handlers that change the padmode
    // this expects to be used as for creating the input handler of a
    // components.Button
    const makePadModeInputHandler = layer =>
        (layer === undefined) ?
            ReloopReady.noopInputHandler :
            function(channel, control, value, status, _group) {
                if (!this.isPress(channel, control, value, status)) {
                    return;
                }
                applyLayer(layer);
            };

    const thisContainer = this;

    // create physical buttons for changing the layers
    this.padModeSelectors = Array(8);
    padModeInstances.forEach(obj => {
        thisContainer.padModeSelectors[obj.pos] = new components.Button({
            midi: [0x94 + index, obj.control],
            on: ReloopReady.padColorPalette.Blue.lit,
            off: ReloopReady.padColorPalette.Blue.dim,
            input: makePadModeInputHandler(obj.modeInstance),
            trigger: function() {
                this.output(thisContainer.currentLayer === obj.modeInstance);
            }
        });
    });
    this.padModeSelectors = this.padModeSelectors.map(obj =>
        obj === undefined ? new components.Button({}) : obj
    );

    // button can not be controlled from software.
    // This component instance just serves as input handler for debugging.
    this.modeButton = new components.Button({
        midi: [index, 0x20], // shifted: [0x0C + index, 0x11], shifted led is controllable, unshifted is not.
        input: ReloopReady.noopInputHandler
    });
};

ReloopReady.PadContainer.prototype = Object.create(components.ComponentContainer.prototype);

ReloopReady.Deck = function(index) {

    const channel = index + 1;

    components.Deck.call(this, channel);

    const thisDeck = this;
    const midiOn = 0x90 + index;


    this.play = new components.PlayButton({
        midi: [midiOn, 0x00],
        shiftOffset: 0x10,
        shift: function() {
            // match behavior labelled on hardware
            this.inKey = "reverseroll";
            // todo fix interaction with shift button
        },
    });

    // needs matching cuemode to be used
    this.cue = new components.CueButton({
        midi: [midiOn, 0x01],
        shiftOffset: 0x04,
    });

    this.sync = new components.SyncButton({
        midi: [midiOn, 0x02],
        shiftOffset: 0x1,
    }); // TODO investigate whether custom behavior is required to match controller

    this.vinyl = new components.Button({
        midi: [midiOn, 0x0F],
        shiftOffset: -0x08,
        type: components.Button.prototype.types.toggle,
        shift: function() {
            this.inKey = "slip_enabled";
            this.outKey = this.inKey;
            this.input = components.Button.prototype.input;
        },
        unshift: function() {
            this.input = function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status)) {
                    thisDeck.jog.vinylMode = ! thisDeck.jog.vinylMode;
                    this.output(thisDeck.jog.vinylMode);
                }
            };
        },
        trigger: function() {
            this.output(thisDeck.jog.vinylMode);
        }
    });

    this.jog = new components.JogWheelBasic({
        // midiTouch: [0x90 + index, 0x06]
        // midiWheel: [0xB0 + index, 0x06]
        alpha: 1/8,
        deck: channel,
        wheelResolution: 300,
    });


    this.keylock = new components.Button({
        midi: [midiOn, 0x0D],
        shiftOffset: 0x1C,
        shift: function() {
            this.type = components.Button.prototype.types.push;
            this.inKey = "sync_key";
            this.outKey = this.inKey;
        },
        unshift: function() {
            this.type = components.Button.prototype.types.toggle;
            this.inKey = "keylock";
            this.outKey = this.inKey;
        }
    });

    this.fxUnit = new ReloopReady.FxUnit(index);

    this.padUnit = new ReloopReady.PadContainer(index);

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = thisDeck.currentDeck;
        }
    });
};

ReloopReady.Deck.prototype = new components.Deck();
