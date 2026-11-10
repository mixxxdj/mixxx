/*

TODO:
Maybe indicate current loop-/jumpsize by coloring the pads in a gradient?

Reverse Engineering notes (likely interesting for other Numark Mixtrack-like controllers):
  Platter: 1000 steps/revolution
  14bit-precision elements: search strips, pitch
  (CC: 0x06 setup display controls)
  CC: 0x0E set fine pitch of display
  CC: Ox3F set track duration leds
  CC: 0x06 set platter pos led
  CC: 0x75 (val: 0 turn off all leds, !0 turn all on) (LEDs surface)
  CC: 0x7F (val: 0 turn all of, val: !0 turn all elements on) (Display)
  on: 0x09 pitch up led (0: off, 1: dimm, 2: full);
  on: 0x0A pitch down led (sam vals as pitch up);
  on: 0x51 pitch led 0;
  on: 0x0D KeyLock display;
  on: 0x0E pitch range value;
  CC: 0x1F channel VuMeter: 1: off, 1: 1, 21: 2, 41: 3, 61: 4, 81: 5 (clipping)
  CC: 0x7E individual setting of some display elements (solo element)?
  ON: channel=0xF control=0x3C left PC1/PC2 val: 0x00 (lost control), 0x7F, (gained control)
  ON: channel=0xF control=0x3D right PC1/PC2 val: 0x00 (lost control), 0x7F, (gained control)

  Master VUMeters: is set by controller
  PAD_COLORS: color channels (r,g,b) encoded in two bits each to form velocity value (0b0rrbbgg)
  BPM Display:
    Absolute BPM Syx: 0x00,0x20,0x7f,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    Least-significant-Nibble of Last 5 bytes are responsible for the display value
    Pitch_percentage_change syx: 0x00,0x20,0x7f,0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    Pitch_change_ratio: 0.1bpm=10 offset 5 => 100*bpm (so it hits right in the middle)
    Pitch_percentage_change 0%: 0x00,0x20,0x7f,,0xf,0xf,0xf,0xf,0xd,0x5
    set pitch percentage by
    get 2's complement of d: (~d + 1 >>> 0)

  pitch range:
    midi: ["note_on", note=0x0E, velocity=|bpm|] (abs(bpm) = 100bpm=100velocity)
  Time Display:
    Set Current Time Syx: 0x00,0x20,0x7f,0x01,0x04,0x08,0x00,0x00,0x00,0x00,0x00,0x00
    Set Track duration syx: 0x00,0x20,0x7f,0x01,0x03,0x08,0x00,0x00,0x00,0x00,0x00,0x00
    Set Track duration syx: 0x00,0x20,0x7f,0x01,0x03,0x08,0x00,0x04,0x0a,0x07,0x03,0x07
    syx[3] = channel (1-based)
    switch time display: ["note_on",control=0x46,velocity] only 0x00 || 0x7F (0x00 display elapsed)
    Least-significant-Nibble of Last 5 bytes are responsible for the display value
    6bit value increase in sysex = 1ms timer increase on display
*/

// eslint-disable-next-line no-var
var NS6II = {};

// UserSettings
// available rateRanges to cycle through using the Pitch Bend +/- Buttons.
NS6II.RATE_RANGES = [0.04, 0.08, 0.10, 0.16, 0.24, 0.50, 0.90, 1.00,];

// Globals

NS6II.SCRATCH_SETTINGS = Object.freeze({
    alpha: 1/8,
    beta: 0.125/32,
});

NS6II.PAD_COLORS = Object.freeze({
    OFF: 0,
    RED: {FULL: 48, DIMM: 32, DIMMER: 16},
    YELLOW: {FULL: 60, DIMM: 40},
    GREEN: {FULL: 12, DIMM: 8},
    CELESTE: {FULL: 15, DIMM: 10},
    BLUE: {FULL: 3, DIMM: 2},
    PURPLE: {FULL: 59, DIMM: 34},
    PINK: {FULL: 58, DIMM: 37},
    ORANGE: {FULL: 56, DIMM: 36},
    WHITE: {FULL: 63, DIMM: 42},
});


NS6II.SERATO_SYX_PREFIX = [0x00, 0x20, 0x7f];

components.Button.prototype.off = engine.getSetting("useButtonBacklight") ? 0x01 : 0x00;

components.HotcueButton.prototype.off = NS6II.PAD_COLORS.OFF;
components.HotcueButton.prototype.sendShifted = true;
components.HotcueButton.prototype.shiftControl = true;
components.HotcueButton.prototype.shiftOffset = 8;
components.HotcueButton.prototype.outConnect = false;

components.SamplerButton.prototype.sendShifted = true;
components.SamplerButton.prototype.shiftControl = true;
components.SamplerButton.prototype.shiftOffset = 8;
components.HotcueButton.prototype.outConnect = false;

NS6II.physicalSliderPositions = {
    left: 0.5,
    right: 0.5,
};

NS6II.mixxxColorToDeviceColorCode = colorObj =>  {
    const red = (colorObj.red & 0xC0) >> 2;
    const green = (colorObj.green & 0xC0) >> 4;
    const blue = (colorObj.blue & 0xC0) >> 6;
    return (red | green | blue);
};

NS6II.hardwareColorToHex = colorcode => {
    const red = (colorcode & 0x30) << 18;
    const green = (colorcode & 0x0C) << 12;
    const blue = (colorcode & 0x03) << 6;
    return (red | green | blue);
};

NS6II.padColorMapper = new ColorMapper(_.keyBy(_.range(0, 64), NS6II.hardwareColorToHex));

NS6II.RingBufferView = class {
    constructor(indexable, startIndex = 0) {
        this.indexable = indexable;
        this.index = startIndex;
    }
    advanceBy(n) {
        this.index = script.posMod(this.index + n, this.indexable.length);
        return this.current();
    }
    next() {
        return this.advanceBy(1);
    }
    previous() {
        return this.advanceBy(-1);
    }
    current() {
        return this.indexable[this.index];
    }
};

NS6II.createFilteredSend = function(filter, send) {
    return function(value) {
        if (filter.call(this, value)) {
            send.call(this, value);
        }
    };
};

NS6II.createIdempotentSend = function(send) {
    return NS6II.createFilteredSend(function(value) {
        const v = this._value !== value;
        if (v) {
            this._value = value;
        }
        return v;
    }, send);
};

/**
 * creates an this.isPress guarded input handler
 * @param {(value: number) => void} func callback that is called on ButtonDown
 * @returns {MidiInputHandler} a MIDI handler suitable to be called via a XML <key> binding
 */
NS6II.makeButtonDownInputHandler = function(func) {
    return function(channel, control, value, status, _group) {
        const isPress = this.isPress(channel, control, value, status);
        this.output(isPress);
        if (!isPress) {
            return;
        }
        func.call(this, value);
    };
};


NS6II.Deck = function(channelOffset) {
    const theDeck = this;
    const deckNumber = channelOffset + 1;
    this.group = `[Channel${deckNumber}]`;

    const lr = channelOffset % 2 === 0 ? "left" : "right";
    const sliderPosAccessors = {
        set: function(pos) {
            NS6II.physicalSliderPositions[lr] = pos;
        },
        get: function() {
            return NS6II.physicalSliderPositions[lr];
        }
    };

    this.slip = new components.Button({
        midi: [0x90+channelOffset, 0x1F],
        // shift: [0x90+channelOffset,0x04],
        type: components.Button.prototype.types.toggle,
        unshift: function() {
            this.inKey = "slip_enabled";
            this.outKey = this.inKey;
        },
        shift: function() {
            // use repeat instead of quantize since that
            // is already handled by the SyncButton
            this.inKey = "repeat";
            this.outKey = this.inKey;
        },
    });

    // also known as "censor"
    this.bleep = new components.Button({
        midi: [0x90 + channelOffset, 0x10],
        // shift: [0x90+channelOffset,0x0D]
        unshift: function() {
            this.inKey = "reverseroll";
            this.outKey = this.inKey;
            this.type = components.Button.prototype.types.push;
        },
        shift: function() {
            this.inKey = "keylock";
            this.outKey = this.inKey;
            this.type = components.Button.prototype.types.toggle;
        },
    });

    const takeoverLEDValues = Object.freeze({
        OFF: 0,
        DIMM: 1,
        FULL: 2,
    });
    const takeoverLEDControls = Object.freeze({
        up: 0x09,
        center: 0x51,
        down: 0x0A,
    });

    const takeoverDistance2Brightness = distance => {
        // src/controllers/softtakeover.cpp
        // SoftTakeover::kDefaultTakeoverThreshold = 3.0 / 128;
        const takeoverThreshold = 3 / 128;
        if (distance > takeoverThreshold && distance < 0.10) {
            return takeoverLEDValues.DIMM;
        } else if (distance >= 0.10) {
            return takeoverLEDValues.FULL;
        } else {
            return takeoverLEDValues.OFF;
        }
    };

    const directionOutValueScale = function(softwareSliderPosition) {
        const normalizedPhysicalSliderPosition = sliderPosAccessors.get()*2 - 1;

        if ((this.midi[1] !== takeoverLEDControls.up) !== (normalizedPhysicalSliderPosition > softwareSliderPosition)) {
            return takeoverLEDValues.OFF;
        }

        const distance = Math.abs(normalizedPhysicalSliderPosition - softwareSliderPosition);
        return takeoverDistance2Brightness(distance);
    };


    this.takeoverLeds = new components.ComponentContainer({
        trigger: function() {
            this.up.trigger();
            this.down.trigger();
        },
        center: new components.Component({
            midi: [0x90 + channelOffset, takeoverLEDControls.center],
            outKey: "rate",
            off: 0x00,
            send: NS6II.createIdempotentSend(components.Component.prototype.send),
            outValueScale: function(value) {
                const distance = Math.abs(value);
                if (distance === 0) {
                    return takeoverLEDValues.FULL;
                } else if (distance < 0.10) {
                    return takeoverLEDValues.DIMM;
                } else {
                    return takeoverLEDValues.OFF;
                }
            }
        }),
        up: new components.Component({
            midi: [0x90 + channelOffset, takeoverLEDControls.up],
            outKey: "rate",
            off: 0x00,
            send: NS6II.createIdempotentSend(components.Component.prototype.send),
            outValueScale: directionOutValueScale,
        }),
        down: new components.Component({
            midi: [0x90 + channelOffset, takeoverLEDControls.down],
            outKey: "rate",
            off: 0x00,
            send: NS6II.createIdempotentSend(components.Component.prototype.send),
            outValueScale: directionOutValueScale,
        }),
    });

    // features 14-bit precision
    this.pitch = new components.Pot({
        midi: [0xB0 + channelOffset, 0x9],
        // LSB: [0x90+channelOffset,0x29]
        group: theDeck.group,
        inKey: "rate",
        invert: true,
        inSetParameter: function(value) {
            sliderPosAccessors.set(value);
            components.Pot.prototype.inSetParameter.call(this, value);
            theDeck.takeoverLeds.trigger();
        },
    });
    const rates = new NS6II.RingBufferView(NS6II.RATE_RANGES);
    this.pitchBendPlus = new components.Button({
        midi: [0x90 + channelOffset, 0x0B],
        // shift: [0x90+channelOffset,0x2B]
        unshift: function() {
            this.inKey = "rate_temp_up";
            this.input = components.Button.prototype.input;
        },
        shift: function() {
            this.inKey = "rateRange";
            this.input = NS6II.makeButtonDownInputHandler(function() {
                this.inSetValue(rates.next());
            });
        },
        outConnect: false,
    });
    this.pitchBendMinus = new components.Button({
        midi: [0x90 + channelOffset, 0x0C],
        // shift: [0x90+channelOffset,0x2C]
        unshift: function() {
            this.inKey = "rate_temp_down";
            this.input = components.Button.prototype.input;
        },
        shift: function() {
            this.inKey = "rateRange";
            this.input = NS6II.makeButtonDownInputHandler(function() {
                this.inSetValue(rates.previous());
            });
        },
        outConnect: false,
    });
    this.shiftButton = new components.Button({
        midi: [0x90 + channelOffset, 0x20],
        input: function(channelmidi, control, value, status, _group) {
            if (this.isPress(channelmidi, control, value, status)) {
                NS6II.mixer.shift();
                NS6II.EffectUnits[channelOffset % 2 + 1].shift();
                theDeck.shift();
            } else {
                NS6II.mixer.unshift();
                NS6II.EffectUnits[channelOffset % 2 + 1].unshift();
                theDeck.unshift();
            }
        },
    });

    this.sync = new components.SyncButton({
        midi: [0x90 + channelOffset, 0x02],
        // shift: [0x90+channelOffset,0x03]
    });

    this.play = new components.PlayButton({
        midi: [0x90 + channelOffset, 0x00],
        // shift: [0x90+channelOffset,0x04]
    });
    this.cue = new components.CueButton({
        midi: [0x90 + channelOffset, 0x01],
        // shift: [0x90+channelOffset,0x05]
    });

    // midi: [0xB0 + channelOffset, 0x06],
    this.jog = new components.JogWheelBasic({
        deck: deckNumber,
        wheelResolution: 1000, // measurement (1000) wasn't producing accurate results (alt: 1073)
        alpha: NS6II.SCRATCH_SETTINGS.alpha,
        beta: NS6II.SCRATCH_SETTINGS.beta,
    });

    this.stripSearch = new components.Pot({
        midi: [0xB0 + channelOffset, 0x4D], // no feedback
        // input MSB: [0xB0+deck,0x2F] LSB
        group: theDeck.group,
        inKey: "playposition",
        shift: function() {
            this.inSetParameter = components.Pot.prototype.inSetParameter;
        },
        unshift: function() {
            this.inSetParameter = function(value) {
                // only allow searching when deck is not playing.
                if (!engine.getParameter(this.group, "play")) {
                    engine.setParameter(this.group, this.inKey, value);
                }
            };
        },
    });
    this.scratch = new components.Button({
        midi: [0x90 + channelOffset, 0x07],
        // shift: [0x90+channelOffset,0x46]
        timerMode: false,
        unshift: function() {
            this.input = NS6II.makeButtonDownInputHandler;
            this.input = function(channelmidi, control, value, status, _group) {
                if (this.isPress(channelmidi, control, value, status)) {
                    theDeck.jog.vinylMode = !theDeck.jog.vinylMode;
                    this.output(theDeck.jog.vinylMode);
                }
            };
            this.output(theDeck.jog.vinylMode);
        },
        shift: function() {
            this.input = function(channelmidi, control, value, status, _group) {
                if (this.isPress(channelmidi, control, value, status)) {
                    // toggle between time_elapsed/_remaining display mode
                    this.timerMode = !this.timerMode;
                    midi.sendShortMsg(0x90 + channelOffset, 0x46, this.timerMode ? 0x7F : 0x00);
                }
            };
        },
    });

    this.display = new NS6II.Display(channelOffset, this);

    this.padUnit = new NS6II.PadModeContainers.ModeSelector(channelOffset+4, this.group);

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = theDeck.group;
        }
    });
};

NS6II.Deck.prototype = new components.Deck();

// JS implementation of engine/enginexfader.cpp:getPowerCalibration  (8005e8cc81f7da91310bfc9088802bf5228a2d43)
NS6II.getPowerCalibration = function(transform) {
    return Math.pow(0.5, 1.0/transform);
};

// JS implementation of util/rescaler.h:linearToOneByX (a939d976b12b4261f8ba14f7ba5e1f2ce9664342)
NS6II.linearToOneByX = function(input, inMin, inMax, outMax) {
    const outRange = outMax - 1;
    const inRange = inMax - inMin;
    return outMax / (((inMax - input) / inRange * outRange) + 1);
};


NS6II.MixerContainer = function() {
    this.channels = [];
    for (let i = 0; i < 4; i++) {
        this.channels[i] = new NS6II.Channel(i);
    }
    this.crossfader = new components.Pot({
        midi: [0xBF, 0x08],
        group: "[Master]",
        inKey: "crossfader",
    });
    this.splitCue = new components.Button({
        // There is a bug in Firmware v1.0.4 which causes the headsplit
        // control to be sent inverted when the controller status is sent
        // (either on PC1/PC2 switch or when requested via sysex).
        // Numark is aware of the issue but they don't seem to be interested
        // in fixing it, so this implements a workaround.
        // `invertNext` should be called whenever the controller dumps the status
        // of its physical controls to mixxx.
        invertNext: function() {
            this._invertNext = true;
            this._timerHandle = engine.beginTimer(200, () => {
                this._invertNext = false;
            }, true);
        },
        _invertNext: false,
        midi: [0x9F, 0x1C],
        group: "[Master]",
        inKey: "headSplit",
        isPress: function(channelmidi, control, value, status) {
            const pressed = components.Button.prototype.isPress.call(this, channelmidi, control, value, status);
            return this._invertNext ? !pressed : pressed;
        }
    });
    this.crossfaderContour = new components.Pot({
        midi: [0xBF, 0x09],
        input: function(_channelMidi, _control, value, _status, _group) {
            // mimic preferences/dialog/dlgprefcrossfader.cpp:slotUpdateXFader
            const transform = NS6II.linearToOneByX(value, 0, 0x7F, 999.6);
            engine.setValue("[Mixer Profile]", "xFaderCurve", transform);
            const calibration = NS6II.getPowerCalibration(transform);
            engine.setValue("[Mixer Profile]", "xFaderCalibration", calibration);
        },
    });
    this.extInputChannel3 = new components.Button({
        midi: [0x9F, 0x57],
        group: "[Channel3]",
        max: 2,
        inKey: "mute"
    });
    this.extInputChannel4 = new components.Button({
        midi: [0x9F, 0x60],
        group: "[Channel4]",
        max: 2,
        inKey: "mute"
    });

    this.browseSection = new NS6II.BrowseSection();
};

NS6II.MixerContainer.prototype = new components.ComponentContainer();

/**
 * Serialize a Number into the controller compatible format used in sysex messages
 * @param {number} number input Integer to be converted
 * @param {boolean} signed specify if the value can be negative.
 * @param {number} precision how many nibbles the resulting buffer should have (depends on the message)
 * @returns {Array<number>} array of length that can be used to build sysex payloads
 */
NS6II.numberToSysex = function(number, signed, precision) {
    const out = Array(precision).fill(0);
    // build 2's complement in case number is negative
    if (number < 0) {
        number = ((~Math.abs(number|0) + 1) >>> 0);
    }
    // split nibbles of number into array
    for (let i = out.length; i; i--) {
        out[i-1] = number & 0xF;
        number = number >> 4;
    }
    // set signed bit in sysex payload
    if (signed) {
        out[0] = (number < 0) ? 0b0111 : 0b1000;
    }
    return out;
};
NS6II.sendSysexMessage = function(channel, location, payload) {
    const msg = [0xF0].concat(NS6II.SERATO_SYX_PREFIX, channel, location, payload, 0xF7);
    midi.sendSysexMsg(msg, msg.length);
};


NS6II.DisplayElement = function(options) {
    components.Component.call(this, options);
};

NS6II.DisplayElement.prototype = new components.Component({
    send: function(payload) {
        if (this.loc !== undefined && this.loc.deck !== undefined && this.loc.control !== undefined) {
            NS6II.sendSysexMessage(this.loc.deck, this.loc.control, payload);
        } else {
            components.Component.prototype.send.call(this, payload);
        }
    },
    shutdown: function() {
        this.output(this.off);
    },
});


NS6II.Display = function(channelOffset) {
    const channel = (channelOffset + 1);
    const deck = `[Channel${channel}]`;

    // optimization so frequently updated controls don't have to poll seldom
    // updated controls each time.
    const deckInfoCache = {
        // seconds
        duration: 0,
        // stored as 1% = 100
        rate: 0,
        rateDir: 1, // 1 or -1 (like the CO)
        trackLoaded: false,
        // stored as rotations per second instead of rpm.
        vinylControlSpeedTypeRatio: 0,
    };

    const vinylControlSpeedTypeConnection = engine.makeConnection(deck, "vinylcontrol_speed_type", function(value) {
        deckInfoCache.vinylControlSpeedTypeRatio = value/60;
    });
    vinylControlSpeedTypeConnection.trigger();

    const rateDirConnection = engine.makeConnection(deck, "rate_dir", function(value) {
        deckInfoCache.rateDir = value;
    });
    rateDirConnection.trigger();

    this.keylockUI = new NS6II.DisplayElement({
        midi: [0x90 + channelOffset, 0x0D],
        outKey: "keylock",
        off: 0x00,
        on: 0x7F,
    });

    this.rateRangeUI = new NS6II.DisplayElement({
        midi: [0x90 + channelOffset, 0x0E],
        outKey: "rateRange",
        off: 0,
        outValueScale: function(value) {
            deckInfoCache.rate = value * 10000;
            return Math.round(value * 100);
        }
    });

    this.bpmUI = new NS6II.DisplayElement({
        loc: {deck: channel, control: 0x01},
        outKey: "bpm",
        off: 0,
        outValueScale: function(value) {
            return NS6II.numberToSysex(value * 100, false, 6);
        },
    });

    this.rateChangePercentageUI = new NS6II.DisplayElement({
        loc: {deck: channel, control: 0x02},
        outKey: "rate",
        outValueScale: function(value) {
            return NS6II.numberToSysex(
                value * deckInfoCache.rate * deckInfoCache.rateDir,
                true,
                6
            );
        },
    });

    this.durationUI = new NS6II.DisplayElement({
        loc: {deck: channel, control: 0x3},
        outKey: "duration",
        outValueScale: function(value) {
            deckInfoCache.duration = value;
            return NS6II.numberToSysex(value*62.5, true, 7);
        },
    });

    this.timeElapsedUI = new NS6II.DisplayElement({
        loc: {deck: channel, control: 0x04},
        outKey: "playposition",
        outValueScale: function(playpos) {
            const elapsedTime = deckInfoCache.duration * playpos;
            return NS6II.numberToSysex(
                elapsedTime*62.5, // arbitrary controller specific scaling factor
                true, // signed int
                7
            );
        },
    });

    this.playPositionRingUI = new NS6II.DisplayElement({
        midi: [0xB0 + channelOffset, 0x3F],
        outKey: "playposition",
        max: 0x7F,
        off: 0x00,
        outValueScale: function(playpos) {
            // check if track is loaded because playpos value is 0.5 when there isn't a track loaded.
            return deckInfoCache.trackLoaded ?
                Math.round(playpos * this.max) :
                this.off;
        },
        send: NS6II.createIdempotentSend(NS6II.DisplayElement.prototype.send),
    });

    this.vinylStickerPositionUI = new NS6II.DisplayElement({
        midi: [0xB0 + channelOffset, 0x06],
        outKey: "playposition",
        max: 0x7F,
        off: 0x00,
        outValueScale: function(playpos) {
            const elapsedTime = deckInfoCache.duration * playpos;
            return script.posMod(elapsedTime * deckInfoCache.vinylControlSpeedTypeRatio, 1) * this.max;
        },
        send: NS6II.createIdempotentSend(NS6II.DisplayElement.prototype.send),
    });

    this.deckLoadedConnection = engine.makeConnection(deck, "track_loaded", function(value) {
        deckInfoCache.trackLoaded = value;
    });
    this.deckLoadedConnection.trigger();

};

NS6II.Display.prototype = new components.ComponentContainer();

NS6II.PadMode = function(channelOffset) {
    components.ComponentContainer.call(this, {});

    this.constructPads = constructPad => {
        this.pads = this.pads.map((_, padIndex) => constructPad(padIndex));
    };
    const makeParameterPressHandler = (control, onButtonDown) =>
        new components.Button({
            midi: [0x90 + channelOffset, control],
            // never outconnect, as these buttons don't have LEDs
            outConnect: false,
            input: function(channelmidi, control, value, status, group) {
                if (this.isPress(channelmidi, control, value, status)) {
                    onButtonDown(channelmidi, control, value, status, group);
                }
            },
        });
    this.assignParameterPressHandlerLeft = onButtonDown => {
        this.parameterLeft = makeParameterPressHandler(0x28, onButtonDown);
    };
    this.assignParameterPressHandlerRight = onButtonDown => {
        this.parameterRight = makeParameterPressHandler(0x29, onButtonDown);
    };
    // this is a workaround for components, forEachComponent only iterates
    // over ownProperties, so these have to constructed by the constructor here
    // instead of being merged by the ComponentContainer constructor
    this.pads = Array(8).fill(undefined);
    const doNothing = () => {};
    this.assignParameterPressHandlerLeft(doNothing);
    this.assignParameterPressHandlerRight(doNothing);
};

NS6II.PadMode.prototype = new components.ComponentContainer();

NS6II.Pad = function(options) {
    components.Button.call(this, options);
};
NS6II.Pad.prototype = new components.Button({
    // grey could be an alternative as well as a backlight color.
    off: engine.getSetting("useButtonBacklight") ? NS6II.PAD_COLORS.RED.DIMMER : NS6II.PAD_COLORS.OFF,
    outConnect: false,
    sendShifted: true,
    shiftControl: true,
    shiftOffset: 8,
});

NS6II.PadModeContainers = {};

NS6II.PadModeContainers.HotcuesRegular = function(channelOffset, hotCueOffset) {

    NS6II.PadMode.call(this, channelOffset);

    this.constructPads(i =>
        new components.HotcueButton({
            midi: [0x90 + channelOffset, 0x14 + i],
            // shift: [0x94+channelOffset,0x1b+i],
            number: i + 1 + hotCueOffset,
            colorMapper: NS6II.padColorMapper,
            // sendRGB: function(colorObj) {
            //     this.send(NS6II.mixxxColorToDeviceColorCode(colorObj));
            // },
            off: NS6II.PAD_COLORS.OFF,
        })
    );
    this.parameterLeft = new components.Button({
        midi: [0x90, 0x28],
        unshift: function() {
            // TODO change hotcue page
            this.inKey = undefined;
            this.input = () => {};
        },
        shift: function() {
            this.inKey = "hotcue_focus_color_prev";
            this.input = components.Button.prototype.input;
        }
    });
    this.parameterRight = new components.Button({
        midi: [0x90, 0x29],
        unshift: function() {
            // TODO change hotcue page
            this.inKey = undefined;
            this.input = () => {};
        },
        shift: function() {
            this.inKey = "hotcue_focus_color_next";
            this.input = components.Button.prototype.input;
        }
    });
};
NS6II.PadModeContainers.HotcuesRegular.prototype = new NS6II.PadMode();

NS6II.PadModeContainers.LoopAuto = function(channelOffset) {

    NS6II.PadMode.call(this, channelOffset);

    const theContainer = this;
    this.currentBaseLoopSize = engine.getSetting("defaultLoopRootSize");

    const changeLoopSize = loopSize => {
        theContainer.currentBaseLoopSize = _.clamp(loopSize, -5, 7);
        theContainer.pads.forEach((c, i) => {
            if (c instanceof components.Component) {
                c.disconnect();
                const loopSize = Math.pow(2, theContainer.currentBaseLoopSize + i);
                c.inKey = `beatloop_${loopSize}_toggle`;
                c.outKey = `beatloop_${loopSize}_enabled`;
                c.connect();
                c.trigger();
            }
        });
    };

    this.constructPads(i =>
        new NS6II.Pad({
            midi: [0x90 + channelOffset, 0x14 + i],
            on: NS6II.PAD_COLORS.RED.FULL,
            off: NS6II.PAD_COLORS.RED.DIMM,
            // key is set by changeLoopSize()
        })
    );

    this.assignParameterPressHandlerLeft(() => changeLoopSize(theContainer.currentBaseLoopSize - 1));
    this.assignParameterPressHandlerRight(() => changeLoopSize(theContainer.currentBaseLoopSize + 1));
    changeLoopSize(engine.getSetting("defaultLoopRootSize"));
};

NS6II.PadModeContainers.LoopAuto.prototype = new NS6II.PadMode();


NS6II.PadModeContainers.BeatJump = function(channelOffset) {

    NS6II.PadMode.call(this, channelOffset);

    const theContainer = this;
    this.currentBaseJumpExponent = engine.getSetting("defaultLoopRootSize");

    const changeLoopSize = function(loopSize) {
        theContainer.currentBaseJumpExponent = _.clamp(loopSize, -5, 2);

        const applyToComponent = function(component, key) {
            if (!(component instanceof components.Component)) {
                return;
            }
            component.disconnect();
            component.inKey = key;
            component.outKey = component.inKey;
            component.connect();
            component.trigger();
        };
        for (let i = 0; i < 4; i++) {
            const size = Math.pow(2, theContainer.currentBaseJumpExponent + i);
            applyToComponent(theContainer.pads[i], `beatjump_${size}_forward`);
            applyToComponent(theContainer.pads[i+4], `beatjump_${size}_backward`);
        }
    };

    this.constructPads(i =>
        new NS6II.Pad({
            midi: [0x90 + channelOffset, 0x14 + i],
            on: NS6II.PAD_COLORS.GREEN.FULL,
            off: NS6II.PAD_COLORS.GREEN.DIMM,
            // key is set by changeLoopSize()
        })
    );

    this.assignParameterPressHandlerLeft(() => changeLoopSize(theContainer.currentBaseJumpExponent - 1));
    this.assignParameterPressHandlerRight(() => changeLoopSize(theContainer.currentBaseJumpExponent + 1));
    changeLoopSize(engine.getSetting("defaultLoopRootSize"));
};

NS6II.PadModeContainers.BeatJump.prototype = new NS6II.PadMode();

NS6II.PadModeContainers.LoopRoll = function(channelOffset) {
    NS6II.PadMode.call(this, channelOffset);
    const theContainer = this;
    this.currentBaseLoopSize = engine.getSetting("defaultLoopRootSize");

    const changeLoopSize = function(loopSize) {
        // clamp loopSize to [-5;7]
        theContainer.currentBaseLoopSize = Math.min(Math.max(-5, loopSize), 7);
        let i = 0;
        theContainer.pads.forEach(c => {
            if (c instanceof components.Component) {
                c.disconnect();
                c.inKey = `beatlooproll_${Math.pow(2, theContainer.currentBaseLoopSize + (i++))}_activate`;
                c.outKey = c.inKey;
                c.connect();
                c.trigger();
            }
        });
    };

    this.constructPads(i =>
        new NS6II.Pad({
            midi: [0x90 + channelOffset, 0x14 + i],
            on: NS6II.PAD_COLORS.GREEN.FULL,
            off: NS6II.PAD_COLORS.GREEN.DIMM,
            type: components.Button.prototype.types.toggle,
            // key is set by changeLoopSize()
        })
    );

    this.assignParameterPressHandlerLeft(() => changeLoopSize(theContainer.currentBaseLoopSize - 1));
    this.assignParameterPressHandlerRight(() => changeLoopSize(theContainer.currentBaseLoopSize + 1));
    changeLoopSize(engine.getSetting("defaultLoopRootSize"));
};

NS6II.PadModeContainers.LoopRoll.prototype = new NS6II.PadMode();

NS6II.PadModeContainers.LoopControl = function(channelOffset) {
    NS6II.PadMode.call(this, channelOffset);
    this.pads[0] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x14],
        key: "loop_in",
        on: NS6II.PAD_COLORS.BLUE.FULL,
        off: NS6II.PAD_COLORS.BLUE.DIMM,
    });
    this.pads[1] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x15],
        key: "loop_out",
        on: NS6II.PAD_COLORS.BLUE.FULL,
        off: NS6II.PAD_COLORS.BLUE.DIMM
    });
    this.pads[2] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x16],
        key: "beatloop_activate",
        on: NS6II.PAD_COLORS.GREEN.FULL,
        off: NS6II.PAD_COLORS.GREEN.DIMM,
    });
    this.pads[3] = new components.LoopToggleButton({
        midi: [0x90 + channelOffset, 0x17],
        on: NS6II.PAD_COLORS.GREEN.FULL,
        off: NS6II.PAD_COLORS.GREEN.DIMM,
    });
    this.pads[4] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x19],
        key: "beatjump_backward",
        on: NS6II.PAD_COLORS.ORANGE.FULL,
        off: NS6II.PAD_COLORS.ORANGE.DIMM,
    });
    this.pads[5] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x18],
        key: "beatjump_forward",
        on: NS6II.PAD_COLORS.ORANGE.FULL,
        off: NS6II.PAD_COLORS.ORANGE.DIMM,
    });
    this.pads[6] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x1A],
        key: "loop_halve",
        on: NS6II.PAD_COLORS.RED.FULL,
        off: NS6II.PAD_COLORS.RED.DIMM,
    });
    this.pads[7] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x1B],
        key: "loop_double",
        on: NS6II.PAD_COLORS.RED.FULL,
        off: NS6II.PAD_COLORS.RED.DIMM,
    });
};
NS6II.PadModeContainers.LoopControl.prototype = new NS6II.PadMode();

NS6II.PadModeContainers.KeyControl = function(channelOffset) {
    NS6II.PadMode.call(this, channelOffset);
    this.pads[0] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x14],
        key: "sync_key",
        on: NS6II.PAD_COLORS.GREEN.FULL,
        off: NS6II.PAD_COLORS.GREEN.DIMM,
    });
    this.pads[1] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x15],
        key: "pitch_down",
        on: NS6II.PAD_COLORS.BLUE.FULL,
        off: NS6II.PAD_COLORS.BLUE.DIMM,
    });
    this.pads[2] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x16],
        key: "pitch_up",
        on: NS6II.PAD_COLORS.BLUE.FULL,
        off: NS6II.PAD_COLORS.BLUE.DIMM,
    });
    this.pads[3] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x17],
        inKey: "reset_key",
        outKey: "pitch_adjust",
        outValueScale: function(pitchAdjust) {
            // reset_key sometimes sets the key to some small non-zero value sometimes (probably floating point rounding errors)
            // so we check with tolerance here.
            const epsilon = 0.001;
            return Math.abs(pitchAdjust) > epsilon ? this.on : this.off;
        },
        on: NS6II.PAD_COLORS.RED.FULL,
        off: NS6II.PAD_COLORS.RED.DIMM,
    });
    // TODO lower 4 pads; What should I map them to, maybe going by circle of fifths?
    for (let i = 4; i < this.pads.length; i++) {
        // Dummy pads for now.
        this.pads[i] = new NS6II.Pad({
            midi: [0x90 + channelOffset, 0x14 + i],
            trigger: function() {
                this.send(this.off);
            },
        });
    }
};

NS6II.PadModeContainers.KeyControl.prototype = new NS6II.PadMode();

NS6II.PadModeContainers.SamplerNormal = function(channelOffset) {
    NS6II.PadMode.call(this, channelOffset);
    this.constructPads(i =>
        new components.SamplerButton({
            midi: [0x90 + channelOffset, 0x14 + i],
            number: i + 1,
            empty: NS6II.PAD_COLORS.OFF,
            playing: NS6II.PAD_COLORS.WHITE.FULL,
            loaded: NS6II.PAD_COLORS.WHITE.DIMM,
        })
    );
};
NS6II.PadModeContainers.SamplerNormal.prototype = new NS6II.PadMode();

NS6II.PadModeContainers.SamplerVelocity = function(channelOffset) {
    NS6II.PadMode.call(this, channelOffset);

    this.constructPads(i =>
        new components.SamplerButton({
            midi: [0x90 + channelOffset, 0x14 + i],
            number: i + 1,
            empty: NS6II.PAD_COLORS.OFF,
            playing: NS6II.PAD_COLORS.PINK.FULL,
            loaded: NS6II.PAD_COLORS.PINK.DIMM,
            volumeByVelocity: true,
        })
    );
};

NS6II.PadModeContainers.SamplerVelocity.prototype = new NS6II.PadMode();


NS6II.PadModeContainers.BeatgridSettings = function(channelOffset) {

    NS6II.PadMode.call(this, channelOffset);

    // Same layout as waveform customization in LateNight
    // except pads[4] (bottom left button)

    this.pads[0] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x14],
        key: "beats_translate_curpos",
        on: NS6II.PAD_COLORS.RED.FULL,
        off: NS6II.PAD_COLORS.RED.DIMM,
    });
    this.pads[1] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x15],
        key: "beats_translate_earlier",
        on: NS6II.PAD_COLORS.ORANGE.FULL,
        off: NS6II.PAD_COLORS.ORANGE.DIMM,
    });
    this.pads[2] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x16],
        key: "beats_translate_later",
        on: NS6II.PAD_COLORS.ORANGE.FULL,
        off: NS6II.PAD_COLORS.ORANGE.DIMM,
    });
    this.pads[3] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x17],
        key: "shift_cues_later",
        on: NS6II.PAD_COLORS.BLUE.FULL,
        off: NS6II.PAD_COLORS.BLUE.DIMM,
    });
    this.pads[4] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x18],
        key: "bpm_tap",
        on: NS6II.PAD_COLORS.GREEN.FULL,
        off: NS6II.PAD_COLORS.GREEN.DIMM,
    });
    this.pads[5] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x19],
        key: "beats_adjust_faster",
        on: NS6II.PAD_COLORS.YELLOW.FULL,
        off: NS6II.PAD_COLORS.YELLOW.DIMM,
    });
    this.pads[6] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x1A],
        key: "beats_adjust_slower",
        on: NS6II.PAD_COLORS.YELLOW.FULL,
        off: NS6II.PAD_COLORS.YELLOW.DIMM,
    });
    this.pads[7] = new NS6II.Pad({
        midi: [0x90 + channelOffset, 0x1B],
        key: "shift_cues_earlier",
        on: NS6II.PAD_COLORS.BLUE.FULL,
        off: NS6II.PAD_COLORS.BLUE.DIMM,
    });
};

NS6II.PadModeContainers.BeatgridSettings.prototype = new NS6II.PadMode();


NS6II.PadModeContainers.IntroOutroMarkers = function(channelOffset) {
    NS6II.PadMode.call(this, channelOffset);
    const keyPrefix = ["intro_start", "intro_end", "outro_start", "outro_end"];
    for (let i = 0; i < keyPrefix.length; ++i) {
        this.pads[i] = new NS6II.Pad({
            midi: [0x90 + channelOffset, 0x14 + i],
            outKey: `${keyPrefix[i]}_enabled`,
            unshift: function() {
                this.inKey = `${keyPrefix[i]}_activate`;
            },
            shift: function() {
                this.inKey = `${keyPrefix[i]}_clear`;
            },
            on: NS6II.PAD_COLORS.BLUE.FULL,
            off: NS6II.PAD_COLORS.BLUE.DIMM,
        });
    }
    // TODO lower 4 pads; What should I map them to?
    for (let i = 4; i < this.pads.length; i++) {
        // Dummy pads for now.
        this.pads[i] = new NS6II.Pad({
            midi: [0x90 + channelOffset, 0x14 + i],
            trigger: function() {
                this.send(this.off);
            },
        });
    }
};

NS6II.PadModeContainers.IntroOutroMarkers.prototype = new NS6II.PadMode();

NS6II.PadModeContainers.ModeSelector = function(channelOffset, group) {
    const theSelector = this;

    const updateSelectorLeds = () => {
        theSelector.forEachComponent(c => c.trigger());
    };

    const setPads = padInstance => {
        if (padInstance === theSelector.padsContainer) {
            return;
        }
        theSelector.padsContainer.forEachComponent(function(component) {
            component.disconnect();
        });
        theSelector.padsContainer = padInstance;
        updateSelectorLeds();
        theSelector.padsContainer.reconnectComponents(function(c) {
            if (c.group === undefined) {
                c.group = group;
            }
        });
    };

    const makeModeSelectorInputHandler = (control, padInstances) =>
        new components.Button({
            midi: [0x90 + channelOffset, control],
            padInstances: new NS6II.RingBufferView(padInstances),
            input: function(channelmidi, control, value, status, _group) {
                if (!this.isPress(channelmidi, control, value, status)) {
                    return;
                }
                if (this.padInstances.current() === theSelector.padsContainer) {
                    setPads(this.padInstances.next());
                } else {
                    this.padInstances.index = 0;
                    setPads(this.padInstances.current());
                }
            },
            outValueScale: function(active) {
                if (!active) {
                    return this.off;
                }
                switch (this.padInstances.index) {
                case 0: return 0x04; // solid on
                case 1: return 0x02; // blink on/off
                case 2: return 0x03; // blink 3x
                default: return this.off;
                }
            },
            trigger: function() {
                this.output(this.padInstances.indexable.indexOf(theSelector.padsContainer) !== -1);
            },
        });

    const startupModeInstance = new NS6II.PadModeContainers.HotcuesRegular(channelOffset, 0);

    this.modeSelectors = new components.ComponentContainer({
        cues: makeModeSelectorInputHandler(0x00 /*shift: 0x02*/, [startupModeInstance, new NS6II.PadModeContainers.HotcuesRegular(channelOffset, 8)]),
        auto: makeModeSelectorInputHandler(0x10, [new NS6II.PadModeContainers.LoopAuto(channelOffset), new NS6II.PadModeContainers.LoopRoll(channelOffset)]),
        loop: makeModeSelectorInputHandler(0x0E, [new NS6II.PadModeContainers.LoopControl(channelOffset), new NS6II.PadModeContainers.KeyControl(channelOffset)]),
        sampler: makeModeSelectorInputHandler(0x0B /*shift: 0x0F*/, [new NS6II.PadModeContainers.SamplerNormal(channelOffset), new NS6II.PadModeContainers.SamplerVelocity(channelOffset)]),
        slider: makeModeSelectorInputHandler(0x09, [new NS6II.PadModeContainers.BeatJump(channelOffset), new NS6II.PadModeContainers.IntroOutroMarkers(channelOffset), new NS6II.PadModeContainers.BeatgridSettings(channelOffset)]),
    });

    this.padsContainer = new NS6II.PadMode(channelOffset);
    setPads(startupModeInstance);
};

NS6II.PadModeContainers.ModeSelector.prototype = new components.ComponentContainer();

NS6II.Channel = function(channelOffset) {
    const deck = `[Channel${channelOffset+1}]`;
    this.loadTrackIntoDeck = new components.Button({
        midi: [0x9F, 0x02 + channelOffset],
        // midi: [0x90 + channelOffset, 0x17],
        group: deck,
        shift: function() {
            this.inKey = "eject";
            this.outKey = this.inKey;
        },
        unshift: function() {
            this.inKey = "LoadSelectedTrack";
            this.outKey = this.inKey;
        },
    });
    // used to determine whether vumeter on the controller would change
    // so messages get only when that is the case.
    let lastVuLevel = 0;
    this.vuMeterLevelConnection = engine.makeConnection(deck, "vu_meter", value => {
        // check if channel is peaking and increase value so that the peaking led gets lit as well
        // (the vumeter and the peak led are driven by the same control) (values > 81 light up the peakLED as well)

        // convert high res value to 5 LED resolution
        value = Math.floor(value*4) + engine.getValue(deck, "PeakIndicator");
        if (value === lastVuLevel) {
            // return early if vumeter has not changed (on the controller)
            return;
        } else {
            lastVuLevel = value;
        }
        midi.sendShortMsg(0xB0 + channelOffset, 0x1F, value * 20);
    });
    this.preGain = new components.Pot({
        midi: [0xB0 + channelOffset, 0x16],
        softTakeover: false,
        group: deck,
        inKey: "pregain"
    });
    const eqIndicies = [0, 1, 2];
    this.eqKnobs = eqIndicies.map(i =>
        new components.Pot({
            midi: [0xB0 + channelOffset, 0x16 + i],
            softTakeover: false,
            group: `[EqualizerRack1_${deck}_Effect1]`,
            inKey: `parameter${3-i}`,
        })
    );
    this.eqCaps = eqIndicies.map(i =>
        new components.Button({
            midi: [0x90 + channelOffset, 0x16 + i],
            group: `[EqualizerRack1_${deck}_Effect1]`,
            inKey: `button_parameter${3-i}`,
            isPress: function(_midiChannel, _control, value, _status) {
                return NS6II.knobCapBehavior.state > 1 && value > 0;
            }
        })
    );
    this.filter = new components.Pot({
        midi: [0xB0 + channelOffset, 0x1A],
        softTakeover: false,
        group: `[QuickEffectRack1_${deck}]`,
        inKey: "super1",
    });

    // Unused ATM
    this.filterCap = new components.Button({
        // midi: [0x90 + channelOffset, 0x1A],
        // group: "[QuickEffectRack1_" + deck + "_Effect1]",
        // inKey: "enabled",
        input: function() {},
    });

    this.pfl = new components.Button({
        midi: [0x90 + channelOffset, 0x1B],
        group: deck,
        key: "pfl",
        // override off as pfl buttons are always backlit (never completely off)
        // and only turn dim with 0x00
        off: 0x00,
    });
    this.crossfaderOrientation = new components.Component({
        midi: [0x90 + channelOffset, 0x1E],
        group: deck,
        inKey: "orientation",
        inValueScale: function(value) {
            // Controller values to represent the orientation and mixxx
            // orientation representation don't match.
            switch (value) {
            case 1: return 0;
            case 0: return 1;
            case 2: return 2;
            default: throw "unreachable!";
            }
        },
    });

    this.volume = new components.Pot({
        midi: [0xB0 + channelOffset, 0x1C],
        softTakeover: false,
        group: deck,
        inKey: "volume",
    });
};
NS6II.Channel.prototype = new components.ComponentContainer();

NS6II.BrowseSection = function() {
    this.libraryNavigation = new components.ComponentContainer({
        turn: new components.Encoder({
            midi: [0xBF, 0x00], // shift: [0xBF,0x01]
            group: "[Library]",
            inKey: "MoveVertical",
            shift: function() {
                this.stepsize = engine.getSetting("navEncoderAcceleration");
            },
            unshift: function() {
                this.stepsize = 1;
            },
            input: function(_midiChannel, _control, value, _status, _group) {
                this.inSetValue(value === 0x01 ? this.stepsize : -this.stepsize);
            },
        }),
        press: new components.Button({
            midi: [0x9F, 0x06],
            group: "[Library]",
            inKey: "GoToItem",
        }),
    });

    /**
     * @param {number} columnIdToSort Value from `[Library], sort_column` docs
     * @returns {MidiInputHandler} a MIDI handler suitable to be called via a XML <key> binding
     */
    const makeSortColumnInputHandler = columnIdToSort =>
        NS6II.makeButtonDownInputHandler(function() { this.inSetValue(columnIdToSort); });

    const makeSortColumnShiftHandler = inputFun =>
        function() {
            this.group = "[Library]";
            this.inKey = "sort_column_toggle";
            this.outKey = this.inKey;
            this.input = inputFun;
            this.type = components.Button.prototype.types.push;
        };

    const sortBy = columnIdToSort => makeSortColumnShiftHandler(makeSortColumnInputHandler(columnIdToSort));

    this.view = new components.Button({
        midi: [0x9F, 0x0E], // shift: [0x9F,0x13],
        unshift: function() {
            // TODO 2.5: switch to `[Skin], show_maximize_library`.
            this.group = "[Master]";
            this.inKey = "maximize_library";
            this.outKey = this.inKey;
            this.input = components.Button.prototype.input;
            this.type = components.Button.prototype.types.toggle;
        },
        shift: sortBy(script.LIBRARY_COLUMNS.BPM),
    });
    this.back = new components.Button({
        midi: [0x9F, 0x11], // shift: [0x9F,0x12]
        unshift: function() {
            this.group = "[Library]";
            this.inKey = "MoveFocusBackward";
            this.outKey = this.inKey;
            this.input = components.Button.prototype.input;
            this.type = components.Button.prototype.types.push;
        },
        shift: sortBy(script.LIBRARY_COLUMNS.TITLE),
    });
    this.area = new components.Button({
        midi: [0x9F, 0xF], // shift: [0x9F, 0x1E]
        unshift: function() {
            this.group = "[Library]";
            this.inKey = "MoveFocusForward";
            this.outKey = this.inKey;
            this.input = components.Button.prototype.input;
            this.type = components.Button.prototype.types.push;
        },
        shift: sortBy(script.LIBRARY_COLUMNS.KEY),
    });
    this.lprep = new components.Button({
        midi: [0x9F, 0x1B], // shift: [0x9F, 0x14]
        unshift: function() {
            this.group = "[PreviewDeck1]";
            this.inKey = "LoadSelectedTrack";
            this.outKey = this.inKey;
            this.input = components.Button.prototype.input;
            this.type = components.Button.prototype.types.push;
        },
        shift: sortBy(script.LIBRARY_COLUMNS.ARTIST),
    });
};
NS6II.BrowseSection.prototype = new components.ComponentContainer();

// TouchFX / TouchAll
NS6II.knobCapBehavior = new components.Button({
    midi: [0x9F, 0x59],
    state: 0,
    input: function(_midiChannel, _control, value, _status, _group) {
        // map 0, 64, 127 to 0, 1, 2 respectively
        this.state = Math.round(value/64);
    },
});


// FilterRoll / FilterFX
// Unused ATM
NS6II.filterKnobBehavior = new components.Button({
    midi: [0x9F, 0x5A],
    state: 0,
    input: function(_channel, _control, value, _status, _group) {
        // map 0, 64, 127 to 0, 1, 2 respectively
        this.state = Math.round(value/64);
    },
});

NS6II.deckWatcherInput = function(midichannel, _control, _value, _status, _group) {
    const deck = midichannel;
    const toDeck = NS6II.decks[deck];
    const fromDeck = NS6II.decks[(deck + 2) % 4];
    fromDeck.pitch.disconnect();
    toDeck.pitch.connect();
    toDeck.takeoverLeds.trigger();
};

NS6II.PCSelectorInput = function(_midichannel, _control, value, _status, _group) {
    if (value > 0) {
        NS6II.mixer.splitCue.invertNext();
    }
};

NS6II.createEffectUnits = function() {
    NS6II.EffectUnits = [];
    for (let i = 1; i <= 2; i++) {
        NS6II.EffectUnits[i] = new components.EffectUnit(i);
        NS6II.EffectUnits[i].fxCaps = [];
        for (let ii = 0; ii < 3; ii++) {
            NS6II.EffectUnits[i].enableButtons[ii + 1].midi = [0x97 + i, ii]; // shift: [0x97+i,0x0B+ii]
            NS6II.EffectUnits[i].fxCaps[ii + 1] = new components.Button({
                midi: [0x97 + i, 0x21 + ii],
                group: `[EffectRack1_EffectUnit${NS6II.EffectUnits[i].currentUnitNumber}_Effect${ii+1}]`,
                inKey: "enabled",
                shifted: false, // used to disable fx input while selecting
                input: function(midichannel, control, value, status, _group) {
                    if (NS6II.knobCapBehavior.state > 0) {
                        this.inSetParameter(this.isPress(midichannel, control, value, status) && !this.shifted);
                    }
                },
                unshift: function() {
                    this.shifted = false;
                },
                shift: function() {
                    this.shifted = true;
                },
            });
            NS6II.EffectUnits[i].knobs[ii + 1].midi = [0xB7 + i, ii];
        }
        NS6II.EffectUnits[i].effectFocusButton.midi = [0x97 + i, 0x04];
        NS6II.EffectUnits[i].dryWetKnob.midi = [0xB7 + i, 0x03];
        NS6II.EffectUnits[i].dryWetKnob.input = function(_midichannel, _control, value, _status, _group) {
            if (value === 1) {
                this.inSetParameter(this.inGetParameter() + 0.04);
            } else if (value === 127) {
                this.inSetParameter(this.inGetParameter() - 0.04);
            }
        };
        NS6II.EffectUnits[i].mixMode = new components.Button({
            midi: [0xB7 + i, 0x41],
            type: components.Button.prototype.types.toggle,
            inKey: "mix_mode",
            group: NS6II.EffectUnits[i].group,
        });
        for (let ii = 0; ii < 4; ii++) {
            const channel = `Channel${ii + 1}`;
            NS6II.EffectUnits[i].enableOnChannelButtons.addButton(channel);
            NS6II.EffectUnits[i].enableOnChannelButtons[channel].midi = [0x97 + i, 0x05 + ii];
        }
        NS6II.EffectUnits[i].init();
    }
};

NS6II.askControllerStatus = function() {
    const controllerStatusSysex = [0xF0, 0x00, 0x20, 0x7F, 0x03, 0x01, 0xF7];
    NS6II.mixer.splitCue.invertNext();
    midi.sendSysexMsg(controllerStatusSysex, controllerStatusSysex.length);
};

NS6II.init = function() {

    // force headMix to 0 because it is managed by the controller hardware mixer.
    engine.setParameter("[Master]", "headMix", 0);

    NS6II.decks = new components.ComponentContainer();
    for (let i = 0; i < 4; i++) {
        NS6II.decks[i] = new NS6II.Deck(i);
    }
    NS6II.mixer = new NS6II.MixerContainer();

    NS6II.createEffectUnits();

    NS6II.askControllerStatus();
};

NS6II.shutdown = function() {
    NS6II.mixer.shutdown();
    NS6II.decks.shutdown();
    NS6II.EffectUnits.forEach(unit => unit.shutdown());
};
