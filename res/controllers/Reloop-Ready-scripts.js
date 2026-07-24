
// eslint-disable-next-line no-var, @typescript-eslint/no-unused-vars
var ReloopReadyInstance = {
    init: function() {
        ReloopReady.init();

    }
};

class ReloopReady {
    static init() {
        ReloopReadyInstance = new ReloopReady();
    }
    constructor() {

        /// On my hardware unit, the tempo faders have such a cheap build-quality
        /// that the notches in the center are not actually in the middle
        /// If you find your notches are offset too, do this:
        /// Load this mapping in mixxx using `--developer` mode.
        /// Then move the tempo fader into the notch at the position where
        /// the tempo should then be the true tempo (+/- 0). Open the developer tools
        /// and read the `rate` value of the channel. If its already 0.5 you should be
        /// good, if its not that, copy the value here and the fader will be adjusted
        /// for you.
        this.tempoFaderMiddles = [engine.getSetting("tempoFaderCorrectionLeft"), engine.getSetting("tempoFaderCorrectionRight")];


        components.Button.prototype.off = engine.getSetting("useButtonBacklight") ? this.constructor.singleColorLED.dim : this.constructor.singleColorLED.off;
        components.Button.prototype.on = this.constructor.singleColorLED.lit;


        /**
         * Misc. midi notes
         *
         * Note 0x31 firmware fader-start feature. Note 0x7F if volume fader is non-zero
         */

        this.components = new components.ComponentContainer();

        this.components.leftDeck = new ReloopReady.Deck(0);
        this.components.rightDeck = new ReloopReady.Deck(1);
        this.components.leftChannel = new ReloopReady.Channel(0, this);
        this.components.rightChannel = new ReloopReady.Channel(1, this);

        this.components.crossfader = new components.Pot({
            midi: [0xBE, 0x08],
            group: "[Master]",
            inKey: "crossfader",
            softTakeover: false, // see volume faders
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

        this.components.loadLeft = new components.Button({
            midi: [0x9E, 0x02],
            shiftOffset: 0x19,
            group: "[Channel1]",
            shift: function() {
                this.inKey = "eject";
                this.outKey = this.inKey;
            },
            unshift: function() {
                this.inKey = "LoadSelectedTrack";
                this.outKey = this.inKey;
            }
        });
        this.components.loadRight = new components.Button({
            midi: [0x9E, 0x03],
            shiftOffset: 0x0D,
            group: "[Channel2]",
            shift: function() {
                this.inKey = "eject";
                this.outKey = this.inKey;
            },
            unshift: function() {
                this.inKey = "LoadSelectedTrack";
                this.outKey = this.inKey;
            }
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
        midi.sendSysexMsg(this.constructor.requestFirmwareVersionSysex.concat([0xF7]));
    }

    /**
     * Handle incoming sysex message
     * @param {Array<number>} data sysex data received from the controller
     */
    incomingData(data) {
        const arrayStartsWith = (arr, pattern) => pattern.every((elem, i) => elem === arr[i]);
        if (arrayStartsWith(data, this.constructor.requestFirmwareVersionSysex)) {
            const firmwareVersion =  data.slice(this.constructor.requestFirmwareVersionSysex.length, data.length - 1);
            // no idea if that's actually the format used for the firmware version,
            // its just 4 bytes (7-bit cuz midi) for me.
            console.log(`firmware version: ${firmwareVersion.join(".")}`);
            return;
        }
        console.warn(`unregonized incoming sysex data: ${data}`);
    }

    shutdown() {
        this.components.shutdown();
    }

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
     * @param {number} color a midi value representing the color
     * @returns {number} 24-bit (8-bit per channel) upscaled `color`
     */
    static midiToFullColor(color) {
        let   b = (color & 0b00000011) << 6;
        let   g = (color & 0b00001100) << 4;
        let   r = (color & 0b00110000) << 2;
        const i = (color & 0b01000000) >> 6;

        if (i === 0) {
            // half the RGB intensity
            r >>= 0b1;
            g >>= 0b1;
            b >>= 0b1;
        }
        return (r << 16) | (g << 8) | b;
    }
    static dimColor(color) {
        return color & 0b00111111;
    }
    static clamp(value, min, max) {
        return Math.max(min, Math.min(max, value));
    }


    /**
     * creates an this.isPress guarded input handler, assumes to be called by components.Button or subclasses thereof
     * @param {(value: number) => void} handler callback that is called on ButtonDown
     * @returns {(channel: number, control: number, value: number, status: number, group: string) => void} XML input handler
     */
    static makeButtonDownInputHandler(handler) {
        return function(channel, control, value, status, _group) {
            this.isPressed = this.isPress(channel, control, value, status);
            if (this.isPressed) {
                handler.call(this, value);
            }
            this.trigger();
        };
    }
    static makeIsPressedTrigger() {
        return function() {
            this.output(this.isPressed);
        };
    }
}

ReloopReady.padColorPaletteTemplate = {
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

ReloopReady.singleColorLED = {
    off: 0x00,
    dim: 0x01,
    lit: 0x7F, // anything between 0x02 and 0x7F is fully lit
};

ReloopReady.noopInputHandler = (_channel, _control, _value, _status, _group) => {};

(() => {
    const fullColorToMidiColorMap = {};
    for (let midiColor = 0x00; midiColor <= 0b1111111; ++midiColor) {
        fullColorToMidiColorMap[ReloopReady.midiToFullColor(midiColor)] = midiColor;
    }

    ReloopReady.padColorMapper = new ColorMapper(fullColorToMidiColorMap);

    // transform padColorPalette to low-res midi approximations:

    ReloopReady.padColorPalette = {};
    for (const [name, fullColor] of Object.entries(ReloopReady.padColorPaletteTemplate)) {
        const litColor = ReloopReady.padColorMapper.getValueForNearestColor(fullColor);
        const dimColor = ReloopReady.dimColor(litColor);
        ReloopReady.padColorPalette[name] = {
            lit: litColor,
            dim: dimColor,
        };
    }
})();

components.Button.prototype.sendShifted = true;
components.Button.prototype.shiftControl = true;
// shift control offset depends on concrete control

components.Encoder.prototype.inValueScale = function(value) {
    return value < 0x40 ? value : value - 0x80;
};

ReloopReady.Channel = class extends components.ComponentContainer {
    constructor(deckIdx, parent) {
        super();
        const channel = deckIdx + 1;
        const group = `[Channel${channel}]`;
        const eqGroup = `[EqualizerRack1_${group}_Effect1]`;
        const eqLayoutSetting = engine.getSetting("eqLayout"); // bitset string (eg. "10111") indicating which elements are used
        const eqKnobAddresses = [0x1A, 0x19, 0x17, 0x16];
        this.eqKnob = [
            [`[QuickEffectRack1_${group}]`, "super1"],
            [eqGroup, "parameter1"],
            [eqGroup, "parameter2"],
            [eqGroup, "parameter3"],
            [group, "pregain"]]
            .filter((_, i) => eqLayoutSetting[i] === "1")
            .map(([group, key], i) => new components.Pot({
                midi: [0xB0 + deckIdx, eqKnobAddresses[i]],
                group: group,
                inKey: key,
            }));

        this.rate = new components.Pot({
            midi: [0xB0 + deckIdx, 0x09], // MSB control: 0x3F
            inKey: "rate",
            // tempo fader response: top ("-") @ 0, bottom ("+") @ 1023
            max: 1023,
            invert: true,
            // see explanation of ReloopReady.tempoFaderMiddles for info
            inValueScale: function(value) {
                const middle = (this.max + 1) * parent.tempoFaderMiddles[deckIdx];
                return script.absoluteNonLinInverse(value, 0, middle, this.max, 0, 1);
            }
        });

        this.volume = new components.Pot({
            midi: [0xB0 + deckIdx, 0x1C],
            inKey: "volume",
            // whenever we shift, the component triggers softtakeover.
            // this is usually not a problem on higher-end hardware,
            // but on this controller, the fader is so-low resolution
            // that a quick movement can lead to such a large difference,
            // that it really does seem like the fader has been moved
            // while shifting. So softtakeover is engaged, even though
            // it shouldn't which makes the volume fader unresponsive
            // in that edge case (this is perceived to happen randomly
            // to the untrained user and definitely not desirable).
            // So opt out of any softTakeover-trickery.
            softTakeover: false,
        });

        this.pfl = new components.Button({
            midi: [0x90 + deckIdx, 0x1B],
            shiftOffset: -0xD,
            key: "pfl",
            type: components.Button.prototype.types.toggle,
        });

        this.reconnectComponents(c => {
            if (c.group === undefined) {
                c.group = group;
            }
        });
    }
};
ReloopReady.FxUnit = class extends components.ComponentContainer {
    constructor(deckIdx) {
        super();

        const midiOn = 0x98 + deckIdx;
        const channel = deckIdx + 1;

        this.level = new components.Pot({
            midi: [0xB8 + deckIdx, 0x00],
            group: `[EffectRack1_EffectUnit${channel}]`,
            inKey: "super1",
        });

        this.fxButtons = [0x00, 0x01, 0x02, 0x03].map(control => new components.Button({
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
        }));

        this.loop = {
            knob: new components.Encoder({
                midi: [0xB4 + deckIdx, 0x03],
                group: `[Channel${channel}]`,
                input: function(_channel, _control, value, _status, _group) {
                    engine.setValue(this.group, value > 0x40 ? "loop_halve" : "loop_double", 1);
                }
            }),
            button: new components.Button({
                midi: [0x94 + deckIdx, 0x40], // shifted: [0x98 + deckIdx, 0x04]
                group: `[Channel${channel}]`,
                input: function(channel, control, value, status, _group) {
                    if (this.isPress(channel, control, value, status)) {
                        if (this.shifted) {
                            // non standard behavior
                            script.bpm.tapButton(deckIdx);
                        } else {
                            script.triggerControl(this.group, "beatloop_activate");
                        }
                    }
                },
            })
        };
    }
};
ReloopReady.PadMode = class extends components.ComponentContainer {
    constructor(deckIdx) {
        super();
        // interpret this an interface definition
        // we fill this array to avoid the issues with iteration methods and sparse arrays
        this.pads = Array.from(Array(8), (_, i) => 0x14 + i);
        this.parameterLeft = new components.Button({
            midi: [0x94 + deckIdx, 0x28], //shifted control: 0x2B
            shiftOffset: 0x2,
            off: ReloopReady.singleColorLED.off,
            trigger: function() {
                this.send(this.off);
            }
        });
        this.parameterRight = new components.Button({
            midi: [0x94 + deckIdx, 0x29], //shifted control: 0x2B
            shiftOffset: 0x2,
            off: ReloopReady.singleColorLED.off,
            trigger: function() {
                this.send(this.off);
            }
        });
    }
    shutdown() {
        // we want the pads to be completely off when shutdown, not dimmed
        this.pads.forEach(pad => pad.send(ReloopReady.singleColorLED.off));
        this.parameterLeft.shutdown();
        this.parameterRight.shutdown();
    }
};
ReloopReady.HotcuePadMode = class extends ReloopReady.PadMode {
    constructor(deckIdx) {
        super(deckIdx);
        this.pads = this.pads.map((control, i) => new components.HotcueButton({
            midi: [0x94 + deckIdx, control],
            group: `[Channel${deckIdx + 1}]`,
            shiftOffset: 0x8,
            off: 0x00,
            number: i + 1,
            colorMapper: ReloopReady.padColorMapper,
            outConnect: false,
        }));
        this.parameterLeft = new components.Button({
            midi: [0x94 + deckIdx, 0x28], //shifted control: 0x2A
            shiftOffset: 0x2,
            shift: function() {
                this.inKey = "hotcue_focus_color_prev";
                this.outKey = this.inKey;
                this.trigger = function() {
                    this.send(this.off);
                };
            },
            unshift: function() {
                this.inKey = undefined; // no function ATM
                this.outKey = this.inKey;
                this.trigger = function() {
                    this.send(ReloopReady.singleColorLED.off);
                };
            }
        });
        this.parameterRight = new components.Button({
            midi: [0x94 + deckIdx, 0x29], //shifted control: 0x2B
            shiftOffset: 0x2,
            shift: function() {
                this.inKey = "hotcue_focus_color_next";
                this.outKey = this.inKey;
                this.trigger = function() {
                    this.send(this.off);
                };
            },
            unshift: function() {
                this.inKey = undefined; // no function ATM
                this.outKey = this.inKey;
                this.trigger = function() {
                    this.send(ReloopReady.singleColorLED.off);
                };
            }
        });
    }
};

ReloopReady.AbstractLoopPadMode = class extends ReloopReady.PadMode {
    clampLoopSizeExp(loopSizeExp) {
        return ReloopReady.clamp(loopSizeExp, -5, 2);
    }
    setLoopSizeProperties(_size, _pad) {
        throw new TypeError("AbstractLoopPadMode.setLoopSizeProperties not overwritten!");
    }
    setLoopSizes(loopSizeExp) {
        for (let i = 0; i < this.pads.length; ++i) {
            this.setLoopSizeProperties(Math.pow(loopSizeExp + i), this.pads[i]);
        }
    }
    makeParameterInputHandler(loopSizeChangeAmount) {
        const theContainer = this;
        return function(channel, control, value, status, _group) {
            const pressed = this.isPress(channel, control, value, status);
            if (pressed) {
                const newLoopSize = theContainer.clampLoopSizeExp(theContainer.currentLoopSizeExp + loopSizeChangeAmount);
                if (newLoopSize !== theContainer.currentLoopSizeExp) {
                    theContainer.currentLoopSizeExp = newLoopSize;
                    theContainer.setLoopSizes(newLoopSize);
                    theContainer.reconnectComponents();
                }
            }
            this.output(pressed);
        };
    }
    constructor(deckIdx) {
        super(deckIdx);
        // https://stackoverflow.com/a/30560792
        if (new.target === ReloopReady.AbstractLoopPadMode) {
            throw new TypeError("Cannot construct AbstractLoopPadMode instances directly");
        }

        const trigger = function() {
            this.send(this.off);
        };

        // Loop Modes can always have their Parameter buttons
        // implemented in terms of `makeParameterInputHandler` so do that here.
        this.parameterLeft = new components.Button({
            midi: [0x94 + deckIdx, 0x28], //shifted control: 0x2A
            shiftOffset: 0x2,
            input: this.makeParameterInputHandler(-1),
            trigger: trigger,
        });
        this.parameterRight = new components.Button({
            midi: [0x94 + deckIdx, 0x29], //shifted control: 0x2B
            shiftOffset: 0x2,
            input: this.makeParameterInputHandler(1),
            trigger: trigger,
        });
    }
};
ReloopReady.AutoLoopPadMode = class extends ReloopReady.AbstractLoopPadMode {

    setLoopSizeProperties(size, pad) {
        pad.inKey = `beatloop_${size}_toggle`;
        pad.outKey = `beatloop_${size}_enabled`;
    }

    constructor(deckIdx) {
        super(deckIdx);

        this.currentLoopSizeExp = engine.getSetting("defaultLoopRootSize");

        this.pads = this.pads.map(control => new components.Button({
            midi: [0x94 + deckIdx, control],
            group: `[Channel${deckIdx + 1}]`,
            shiftOffset: 0x8,
            off: ReloopReady.padColorPalette.Red.dim,
            on: ReloopReady.padColorPalette.Red.lit,
            outConnect: false,
        }));
        this.setLoopSizes(this.currentLoopSizeExp);
    }
};
ReloopReady.ManualLoopPadMode = class extends ReloopReady.PadMode {
    constructor(deckIdx) {
        super(deckIdx);

        let loopMoveBeats = 4;
        const group = `[Channel${deckIdx + 1}]`;

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
            trigger: ReloopReady.makeIsPressedTrigger(),
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
            trigger: ReloopReady.makeIsPressedTrigger(),
        });

        this.pads.forEach((pad, i) => pad.midi = [0x94 + deckIdx, 0x14 + i]);

        this.parameterLeft = new components.Button({
            midi: [0x94 + deckIdx, 0x28],
            shiftOffset: 0x2,
            input: ReloopReady.makeButtonDownInputHandler(function() {
                loopMoveBeats *= 0.5;
            }),
            trigger: ReloopReady.makeIsPressedTrigger(),
        });
        this.parameterRight = new components.Button({
            midi: [0x94 + deckIdx, 0x29],
            shiftOffset: 0x2,
            input: ReloopReady.makeButtonDownInputHandler(function() {
                loopMoveBeats *= 2;
            }),
            trigger: ReloopReady.makeIsPressedTrigger(),
        });

        this.reconnectComponents(c => {
            if (c.group === undefined) {
                c.group = group;
            }
        });
    }
};
ReloopReady.SamplerPadMode = class extends ReloopReady.PadMode {
    constructor(deckIdx) {
        super(deckIdx);
        this.pads = this.pads.map((control, i) => new components.SamplerButton({
            midi: [0x94 + deckIdx, control],
            number: i + 1,
            shiftOffset: 0x8,
            off: ReloopReady.padColorPalette.Off,
            outConnect: false,
        }));
    }
};
ReloopReady.LoopRollPadMode = class extends ReloopReady.AbstractLoopPadMode {
    setLoopSizeProperties(size, pad) {
        pad.inKey = `beatlooproll_${size}_activate`;
        pad.outKey = `beatloop_${size}_enabled`;
    }

    makeParameterInputHandler(loopSizeChangeAmount) {
        const theContainer = this;
        return ReloopReady.makeButtonDownInputHandler(function() {
            const newLoopSize = theContainer.clampLoopSizeExp(theContainer.currentLoopSizeExp + loopSizeChangeAmount);
            if (newLoopSize !== theContainer.currentLoopSizeExp) {
                theContainer.currentLoopSizeExp = newLoopSize;
                theContainer.setLoopSizes(newLoopSize);
                theContainer.reconnectComponents();
            }
        });
    }

    constructor(deckIdx) {
        super(deckIdx);
        this.currentLoopSizeExp = engine.getSetting("defaultLoopRootSize");

        this.pads = this.pads.map(control => new components.Button({
            midi: [0x94 + deckIdx, control],
            group: `[Channel${deckIdx + 1}]`,
            shiftOffset: 0x8,
            off: ReloopReady.padColorPalette.Green.dim,
            on: ReloopReady.padColorPalette.Green.lit,
            outConnect: false,
        }));

        this.setLoopSizes(this.currentLoopSizeExp);
    }
};

// Pitch Play mode taken and heavily modified from Roland DJ 505 mapping.
ReloopReady.Pitch = class extends ReloopReady.PadMode {
    constructor(deckIdx) {
        super(deckIdx);

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
            this.midi = [0x94 + deckIdx, 0x14 + n];
            this.number = n + 1;
            this.on = ReloopReady.padColorPalette.Purple.dim;
            this.colorMapper = ReloopReady.padColorMapper;
            this.colorKey = `hotcue_${this.number}_color`;
            components.Button.call(this);
        };
        this.PerformancePad.prototype = new components.Button({
            shiftOffset: 8,
            group: `[Channel${deckIdx + 1}]`,
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
                    if (engine.getValue(this.group, `hotcue_${cuepoint}_type`) && engine.getValue(this.group, "play")) {
                        engine.setValue(this.group, `hotcue_${cuepoint}_goto`, value);
                    } else {
                        engine.setValue(this.group, `hotcue_${cuepoint}_activate`, value);
                    }
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

        const litTrigger = function() {
            this.send(this.off);
        };

        this.parameterLeft = new components.Button({
            midi: [0x94 + deckIdx, 0x28],
            shiftOffset: 0x2,
            input: ReloopReady.makeButtonDownInputHandler(function() {
                if (range === PitchPlayRange.UP) {
                    range = PitchPlayRange.MID;
                } else if (range === PitchPlayRange.MID) {
                    range = PitchPlayRange.DOWN;
                } else {
                    range = PitchPlayRange.UP;
                }
                theContainer.pads.forEachComponent(c => c.trigger());
            }),
            trigger: litTrigger,
        });
        this.parameterRight = new components.Button({
            midi: [0x94 + deckIdx, 0x29],
            shiftOffset: 0x2,
            input: ReloopReady.makeButtonDownInputHandler(function() {
                if (range === PitchPlayRange.UP) {
                    range = PitchPlayRange.DOWN;
                } else if (range === PitchPlayRange.MID) {
                    range = PitchPlayRange.UP;
                } else {
                    range = PitchPlayRange.MID;
                }
                theContainer.pads.forEachComponent(c => c.trigger());
            }),
            trigger: litTrigger,
        });
    }
};

// There is no such thing as a scratch bank in Mixxx so I'm repurposing this
// PadMode for beatjumping.
ReloopReady.ScratchBankPadMode = class extends ReloopReady.AbstractLoopPadMode {
    // Note that "Jump" and "Loop" is used interchangeably used here
    clampLoopSizeExp(jumpSizeExp) {
        return ReloopReady.clamp(jumpSizeExp, -5, 6);
    }
    setLoopSizes(jumpSizeExp) {
        const middle = this.pads.length / 2;
        let jumpsize = Math.pow(2, jumpSizeExp);
        for (let i = 0; i < middle; ++i) {
            const padTop = this.pads[i];
            padTop.inKey = `beatjump_${jumpsize}_forward`;
            padTop.outKey = padTop.inKey;
            const padBot = this.pads[middle + i];
            padBot.inKey = `beatjump_${jumpsize}_backward`;
            padBot.outKey = padBot.inKey;
            jumpsize *= 2;
        }
    }
    constructor(deckIdx) {
        super(deckIdx);

        this.currentLoopSizeExp = engine.getSetting("defaultLoopRootSize");

        this.pads = this.pads.map(control =>
            new components.Button({
                midi: [0x94 + deckIdx, control],
                group: `[Channel${deckIdx + 1}]`,
                shiftOffset: 0x8,
                off: ReloopReady.padColorPalette.Cyan.dim,
                on: ReloopReady.padColorPalette.Cyan.lit,
                outConnect: false,
            })
        );
        this.setLoopSizes(this.currentLoopSizeExp);
    }
};
ReloopReady.BeatGridPadMode = class extends ReloopReady.PadMode {
    constructor(deckIdx) {
        super(deckIdx);

        const group = `[Channel${deckIdx + 1}]`;

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
            on: ReloopReady.padColorPalette.Green.lit,
            off: ReloopReady.padColorPalette.Green.dim,
            trigger: function() {
                this.output();
            },
        });
        this.pads[6] = new components.Button({
            key: "bpm_tap",
            on: ReloopReady.padColorPalette.Cyan.lit,
            off: ReloopReady.padColorPalette.Cyan.dim,
        });
        this.pads[7] = new components.Button({
            key: "beats_translate_curpos",
            on: ReloopReady.padColorPalette.Cyan.lit,
            off: ReloopReady.padColorPalette.Cyan.dim,
        });

        this.pads.forEach((pad, i) => pad.midi = [0x94 + deckIdx, 0x14 + i]);

        this.reconnectComponents(c => {
            if (c.group === undefined) {
                c.group = group;
            }
        });
    }
};
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
ReloopReady.PadContainer = class extends components.ComponentContainer {
    constructor(deckIdx) {
        super();

        // construct instance of each mode per Container
        this.padModeInstances = ReloopReady.controlPadModeAssoc.map(obj => {
            const modeInstance = new (obj.mode)(deckIdx);
            // make sure no multiple components have "ownership" over each pad
            modeInstance.forEachComponent(c => c.disconnect());
            return {
                pos: obj.pos,
                control: obj.control,
                modeInstance: modeInstance,
            };
        });

        this.currentLayer = this.padModeInstances[0].modeInstance;
        this.currentLayer.reconnectComponents();

        const thisContainer = this;

        // factory/HO function for creating input handlers that change the padmode
        // this expects to be used as for creating the input handler of a
        // components.Button
        const makePadModeInputHandler = layer => (layer === undefined) ?
            ReloopReady.noopInputHandler :
            function(channel, control, value, status, _group) {
                if (!this.isPress(channel, control, value, status)) {
                    return;
                }
                thisContainer.applyLayer(layer);
            };


        // create physical buttons for changing the layers
        this.padModeSelectors = Array(8);
        this.padModeInstances.forEach(obj => {
            thisContainer.padModeSelectors[obj.pos] = new components.Button({
                midi: [0x94 + deckIdx, obj.control],
                on: ReloopReady.padColorPalette.Blue.lit,
                off: ReloopReady.padColorPalette.Blue.dim,
                input: makePadModeInputHandler(obj.modeInstance),
                trigger: function() {
                    this.output(thisContainer.currentLayer === obj.modeInstance);
                }
            });
        });
        this.padModeSelectors = this.padModeSelectors.map(obj => obj === undefined ? new components.Button({}) : obj
        );

        // button can not be controlled from software.
        // This component instance just serves as input handler for debugging.
        this.modeButton = new components.Button({
            midi: [deckIdx, 0x20], // shifted: [0x0C + deckIdx, 0x11], shifted led is controllable, unshifted is not.
            input: ReloopReady.noopInputHandler
        });
    }

    applyLayer(layer) {
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
    }

};
ReloopReady.Deck = class extends components.Deck {
    constructor(index) {

        const channel = index + 1;

        super(channel);

        const thisDeck = this;
        const midiOn = 0x90 + index;


        this.play = new components.PlayButton({
            midi: [midiOn, 0x00],
            shiftOffset: 0x10,
            shift: function() {
                // match behavior labelled on hardware
                this.inKey = "reverseroll";
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
        });

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
                // prevent binding to slip_enabled when unshifted
                // (otherwise vinyl led would glitch when a new track is loaded).
                this.inKey = this.outKey = undefined;
                this.input = ReloopReady.makeButtonDownInputHandler(function() {
                    thisDeck.jog.vinylMode = !thisDeck.jog.vinylMode;
                });
            },
            trigger: function() {
                this.output(thisDeck.jog.vinylMode);
            }
        });

        this.jog = new components.JogWheelBasic({
            // midiTouch: [0x90 + index, 0x06]
            // midiWheel: [0xB0 + index, 0x06]
            alpha: 1 / 8,
            deck: channel,
            // if we shift during scratching, takeover the new settings
            // immideatily
            retriggerScratch: function() {
                if (engine.isScratching(this.deck) && this.vinylMode) {
                    engine.scratchDisable(this.deck);
                    engine.scratchEnable(this.deck,
                        this.wheelResolution,
                        this.rpm,
                        this.alpha,
                        this.beta);
                }
            },
            unshift: function() {
                // accurate, physical control will match spinny.
                this.wheelResolution = 300;
                this.retriggerScratch();
            },
            shift: function() {
                // slow down wheel to 1/3rd the speed
                // to allow for finer adjustments.
                this.wheelResolution = 900;
                this.retriggerScratch();
            }
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
    }
};
