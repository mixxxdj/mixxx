"use strict";

// eslint-disable-next-line no-var
var P1Nano;
(function(P1Nano) {
    const SysexHeader = [0xF0, 0x00, 0x02, 0x4E];
    const MCUHeader = [0xF0, 0x00, 0x00, 0x66, 0x14];

    // Print a number (and only a number) to the 10 digit seven segment display.
    const printSevenSeg = function(num, offset=0) {
        // Differences from the MCU spec:
        // - This should support ASCII plus some special characters, but this
        //   device only appears to support numbers (0x30–0x39) plus the dot
        //   (set bit 6, ie. a bitmask of 0x40).
        // - The assignment block (0x4B and 0x4A) of the display doesn't exist.
        switch (typeof num) {
        case "number":
            num = num.toString().toInt();
            break;
        case "string":
            num = num.toInt();
            break;
        }
        const printNum = [];
        for (const c of num) {
            // Encode the characters for the display.
            switch (c) {
            case 0x2E: // "."
                // If we encounter a dot ".", set bit 6 on the previous
                // character (activating the 8th segment '.').
                printNum[printNum.length - 1] |= 0x40;
                break;
            case 0x2D: // "-"
                // Honestly not sure if this is a deliberately supported
                // character or just a quirk of the incomplete
                // implementation, but 0x3B appears to be just the middle
                // segment of the display.
                printNum.push(0x3B);
                break;
            default:
                printNum.push(c);
                break;
            }
        }

        const displayLen = 10 - offset;

        for (let i = 0; i < displayLen; i++) {
            const cursor = 0x49 - i - offset;
            if (i < printNum.length) {
                midi.sendShortMsg(0xB0, cursor, printNum[i]);
            } else {
                // Clear the remainder of the screen.
                midi.sendShortMsg(0xB0, cursor, 0x20);
            }
        }
    };

    const printScreenName = function(idx, name, row=0) {
        const maxLen = 7;
        if (name.length > maxLen) {
            console.log(`Trimmed text longer than ${maxLen} bytes: "${name}"`);
            name = name.slice(0, maxLen);
        }
        name = name.padEnd(7, " ");
        midi.sendSysexMsg(MCUHeader
            .concat([0x12, (idx * maxLen) + (row * 56)])
            .concat(name.toInt())
            .concat([0xF7]));
    };

    const printLCDDisplay = function(channelIndex, text, row=0) {
        // The MCU spec indicates that a there should be a two rowsstrip of 55
        // dot-matrix displays (and a 56 byte buffer with a newline character at
        // the end of each line).
        //
        // However, each LCD screen can print 7 characters per line, but on
        // every screen but the first one the second character gets skipped and
        // a space is always printed instead.
        // I can only assume this is a bug? Unclear.
        // To work around this weirdness, treat the LCDs as supporting a max of
        // 5 characters and prepend some padding to skip the first two.
        //
        // The first screen will accept 7 characters, but only prints 6.

        const maxLen = 5;
        if (text.length > maxLen) {
            console.log(`Trimmed text longer than ${maxLen} bytes: "${text}"`);
            text = text.slice(0, maxLen);
        }
        text = ` ${text}`;
        // Also add some padding at the end to clear anything that was already
        // printed to this row (if we're not taking up the full length already).
        text += " ".repeat(7 - text.length);

        // Who knows…  seems to line everything up nicely, but I have no idea
        // what's going on with the offsets here.
        const trim = channelIndex - 1;
        const offset = (row * 56) + ((channelIndex % 8) * 7) - trim;
        const payload = SysexHeader.concat([0x15, 0x13]);
        payload.push(offset);
        payload.push(...text.toInt());
        payload.push(0xF7);

        midi.sendSysexMsg(payload);
    };

    const fmtSeconds = function(value, ext=false) {
        const minutes = Math.floor(value / 60).toString()
            .padStart(2, 0);
        const seconds = Math.floor(value - minutes * 60).toString()
            .padStart(2, 0);
        const ms = Math.floor(((value - (minutes * 60) - seconds) % 1) * 60).toString()
            .padStart(2, 0);

        // Don't list hours, chances are most DJ songs aren't that long.
        let out = `${minutes}.${seconds}`;
        if (ext) {
            // Note that this is *different* from what Mixxx displays (the
            // fractional part) because we use '.' as a separator to save space
            // on the display so if we did "00.00.01" (as opposed to "00:00.01")
            // you can't tell that the last part is a fractional part (and not
            // milliseconds).
            out += `.${ms}`;
        }
        return out;
    };

    // Displays the play position on the 7-segment display.
    class PlayPosition extends components.Component {
        constructor(params) {
            super(Object.assign({
                outKey: "playposition",
                firstOut: false,
            }, params));
        }

        shutdown() {
            // Clear the display on shutdown.
            printSevenSeg("");
        }

        output(value, _group, control) {
            // If the display is disabled, clear whatever was on it before the
            // first time we'd update it, then don't send any further updates.
            if (engine.getSetting("disableTimeDisplay")) {
                if (!this.firstOut) {
                    printSevenSeg("");
                    this.firstOut = true;
                }
                return;
            }
            const playPos = (control === "playposition") ? value : engine.getValue(this.group, this.outKey);
            const dur = engine.getValue(this.group, "duration");
            const showDur = control === "ShowDurationRemaining" ? value : engine.getValue("[Controls]", "ShowDurationRemaining");
            let time = "";
            switch (showDur) {
            case 0:
                // Showing elapsed time
                time = fmtSeconds(playPos * dur, true);
                break;
            case 1:
                // Showing remaining time
                time = `-${  fmtSeconds(dur - (playPos * dur), true)}`;
                break;
            case 2:
                // Showing both
                time = `${fmtSeconds(playPos * dur)} -${fmtSeconds(dur - (playPos * dur))}`;
                break;
            }
            printSevenSeg(time);
        }

        connect() {
            if (undefined !== this.group &&
                undefined !== this.outKey &&
                undefined !== this.output &&
                typeof this.output === "function") {
                const conn = engine.makeConnection(this.group, this.outKey, this.output.bind(this));
                this.connections.push(conn);
            }
            const conn2 = engine.makeConnection("[Controls]", "ShowDurationRemaining", this.output.bind(this));
            if (conn2 !== undefined) {
                this.connections.push(conn2);
            }
        }
    }

    class VuMeter extends components.Component {
        constructor(options) {
            super(Object.assign({
                key: "vu_meter",
                // The channel pressure message is used by default, however we
                // may override this (eg. for the mains meter which uses 0xD1).
                midi: [0xD0],
                firstOut: false,
            }, options));
        }

        connect() {
            // If we're creating the main output vumeter, Mixxx had some bugs
            // were the name of the control got changed. Try both and see which
            // one binds so that we ensure support for both 2.5.0 and 2.5.1 (and
            // hopefully other versions going forward).
            if (this.group === "[Main]" && typeof this.output === "function") {
                this.connections[0] = engine.makeConnection(this.group, this.outKey, this.output.bind(this));
                if (this.connections[0] === undefined) {
                    this.connections[0] = engine.makeConnection("[Master]", "VuMeter", this.output.bind(this));
                }
                return;
            }
            return components.Component.prototype.connect.call(this);
        }
        outValueScale(value) {
            // TODO: 0xD is 100% (> 0 dB) while 0xC is clipping at 0 dB. If
            // we're going to do this linear approximation should we scale to
            // that instead?
            return value * 0xD;
        }
        shutdown() {
            midi.sendSysexMsg(SysexHeader
                .concat([0x16, 0x14, this.midi[0], 0x00])
                .concat([0xF7]));
        }
        output(value) {
            // If the meter is disabled, clear whatever was on it before the
            // first time we'd update it, then don't send any further updates.
            if (engine.getSetting("disableVuMeters")) {
                if (!this.firstOut) {
                    this.shutdown();
                    this.firstOut = true;
                }
                return;
            }
            let idx = 0;
            if (this.group !== "[Master]" && this.group !== "[Main]") {
                idx = script.deckFromGroup(this.group) - 1;
            }
            // Set the first nibble of the value to the number of the channel.
            // The second nibble is the value of the meter in the range 0x0 (<
            // -60 dB) through 0xD (> 0 dB).
            midi.sendSysexMsg(SysexHeader
                .concat([0x16, 0x14, this.midi[0], (idx << 4) | this.outValueScale(value)])
                .concat([0xF7]));
        }
    }

    class VelocityEncoder extends components.Encoder {
        constructor(params) {
            super(params);
            if (this.screen === undefined || this.screen < 0 || this.screen > 7) {
                throw Error("VelocityEncoder must specify a screen number between 0 and 7");
            }
            if (this.name === undefined) {
                throw Error("VelocityEncoder missing 'name' field");
            }
            if (typeof this.name === "string") {
                // Pad name out with spaces so that it "fills" the screen
                // (replacing anything longer than the current name that would
                // otherwise hang around on this line).
                this.name = this.name.padEnd(7, " ");
            }
            printScreenName(this.screen, this.name, 1);
        }
        outValueScale(_value) {
            // TODO: I'm extremely confused about what values this expects or
            // why this is necessary, but somehow it works.
            return this.outGetParameter() * 12;
        }
        inValueScale(value) {
            if (value < 0x40) {
                return this.inGetParameter() + (value / 100);
            } else {
                return this.inGetParameter() - ((value - 0x40) / 100);
            }
        }
        output(value, _group, _control) {
            this.send(this.outValueScale(value));
            const groupNo = /\[Channel(\d+)\]/.exec(this.group);
            if (groupNo) {
                const selectedIndicator = this.selected ? "*" : "";
                const deck = `Deck ${groupNo[1]}${selectedIndicator}`.padEnd(7, " ");
                printScreenName(this.screen, deck);
            } else {
                printScreenName(this.screen, "Main".padEnd(7, " "));
            }
        }
        input(channel, control, value, status, group) {
            // If we're not shifted, just call the normal input function.
            if (!this.isShifted) {
                return components.Encoder.prototype.input.call(this, channel, control, value, status, group);
            }

            // If we are shifted, update the parameter that the knob changes.
            const deckResult = /\[Channel(\d+)\]/.exec(this.group);
            const deck = (deckResult && deckResult.length > 0) ? deckResult[1] : undefined;
            const params = Object.freeze([
                {group: `[Channel${deck}]`, key: "pregain", name: "Gain"},
                {group: `[EqualizerRack1_[Channel${deck}]_Effect1]`, key: "parameter3", name: "High"},
                {group: `[EqualizerRack1_[Channel${deck}]_Effect1]`, key: "parameter2", name: "Mid"},
                {group: `[EqualizerRack1_[Channel${deck}]_Effect1]`, key: "parameter1", name: "Low"},
                {group: `[QuickEffectRack1_[Channel${deck}]]`, key: "super1", name: "FX"},
            ]);
            for (let i = 0; i < params.length; i++) {
                const param = params[i];
                if (param.key === this.key) {
                    const offset = value < 0x40 ? 1 : -1;
                    const newParam = params[script.posMod(i + offset, params.length)];
                    Object.assign(this, newParam);
                    if (this.key !== undefined) {
                        this.outKey = this.key;
                        this.inKey = this.key;
                    }
                    printScreenName(this.screen, this.name, 1);
                    if (typeof this.setKnobPressKey === "function") {
                        this.setKnobPressKey(`${this.key}_set_default`, this.group);
                    }
                    break;
                }
            }
        }
        shift() {
            this.disconnect();
            this.isShifted = true;
        }
        unshift() {
            this.isShifted = false;
            this.connect();
        }
    }

    class TouchScreen extends components.ComponentContainer {
        constructor() {
            super({});

            // Only button from the default mapping that makes sense.
            this.tapTempoButton = new components.Button({
                group: "[Channel1]",
                midi: [0x90, 0x55],
                key: "bpm_tap",
            });

            // Buttons from the custom Mixxx mapping.
            this.introStartBtn = new components.Button({
                group: "[Channel1]",
                inKey: "intro_start_activate",
                midi: [0x91, 0x00],
            });
            this.introEndBtn = new components.Button({
                group: "[Channel1]",
                inKey: "intro_end_activate",
                midi: [0x90, 0x40],
            });
            this.outroStartBtn = new components.Button({
                group: "[Channel1]",
                inKey: "outro_start_activate",
                midi: [0x91, 0x02],
            });
            this.outroEndBtn = new components.Button({
                group: "[Channel1]",
                inKey: "outro_end_activate",
                midi: [0x91, 0x03],
            });

            this.hotcues = [];
            for (let i = 0; i < 12; i++) {
                this.hotcues[i] = new components.HotcueButton({
                    number: i + 1,
                    midi: [0x92, i],
                });
            }

            this.samplers = [];
            for (let i = 0; i < 16; i++) {
                this.samplers[i] = new components.SamplerButton({
                    number: i + 1,
                    midi: [0x93, i],
                });
            }
        }
    }

    class Deck extends components.Deck {
        constructor() {
            super([1, 2, 3, 4]);

            this.touchScreen = new TouchScreen();

            this.jogWheel = new components.JogWheelBasic({
                group: "[Channel1]",
                deck: 1,
                midi: [0xB0, 0x3C],
                vinylMode: false,
                wheelResolution: 21,
                max: 0x48,
                alpha: 1/8,
            });

            this.playPosition = new PlayPosition({
                group: "[Channel1]",
            });

            // Transport buttons
            this.playButton = new components.PlayButton({
                group: "[Channel1]",
                midi: [0x90, 0x5E],
                type: components.Button.prototype.types.toggle,
            });
            this.cueButton = new components.CueButton({
                group: "[Channel1]",
                midi: [0x90, 0x5D],
            });
            this.backButton = new components.Button({
                group: "[Channel1]",
                midi: [0x90, 0x5B],
                key: "beatjump_backward",
            });
            this.forwardButton = new components.Button({
                group: "[Channel1]",
                midi: [0x90, 0x5C],
                key: "beatjump_forward",
            });
            this.loopButton = new components.Button({
                group: "[Channel1]",
                midi: [0x90, 0x56],
                inKey: "beatloop_activate",
                outKey: "loop_enabled",
            });

            this.setCurrentDeck("[Channel1]");
        }

        setCurrentDeck(newGroup) {
            midi.sendShortMsg(0x90, 0x18 + (newGroup - 1), 0x7F);
            components.Deck.prototype.setCurrentDeck.call(this, newGroup);
        }
    }

    class Controller extends components.ComponentContainer {
        constructor() {
            super({});

            this.activeDeck = new Deck();

            this.knobPress = [
                new components.Button({
                    group: "[Channel1]",
                    key: "pregain_set_default",
                    midi: [0x90, 0x20],
                }),
                new components.Button({
                    group: "[Channel2]",
                    key: "pregain_set_default",
                    midi: [0x90, 0x21],
                }),
                new components.Button({
                    group: "[Channel3]",
                    key: "pregain_set_default",
                    midi: [0x90, 0x22],
                }),
                new components.Button({
                    group: "[Channel4]",
                    key: "pregain_set_default",
                    midi: [0x90, 0x23],
                }),
            ];
            const setKnobPressKey = function(_this, i) {
                return (key, group) => {
                    _this.knobPress[i].group = group;
                    _this.knobPress[i].key = key;
                    _this.knobPress[i].inKey = key;
                    _this.knobPress[i].outKey = key;
                    _this.knobPress[i].disconnect();
                    _this.knobPress[i].connect();
                    _this.knobPress[i].trigger();
                };
            };
            this.knob = [
                new VelocityEncoder({
                    group: "[Channel1]",
                    screen: 0,
                    selected: true,
                    key: "pregain",
                    name: "Gain",
                    midi: [0xB0, 0x10],
                    setKnobPressKey: setKnobPressKey(this, 0),
                }),
                new VelocityEncoder({
                    group: "[Channel2]",
                    screen: 1,
                    key: "pregain",
                    name: "Gain",
                    midi: [0xB0, 0x11],
                    setKnobPressKey: setKnobPressKey(this, 1),
                }),
                new VelocityEncoder({
                    group: "[Channel3]",
                    screen: 2,
                    key: "pregain",
                    name: "Gain",
                    midi: [0xB0, 0x12],
                    setKnobPressKey: setKnobPressKey(this, 2),
                }),
                new VelocityEncoder({
                    group: "[Channel4]",
                    screen: 3,
                    key: "pregain",
                    name: "Gain",
                    midi: [0xB0, 0x13],
                    setKnobPressKey: setKnobPressKey(this, 3),
                }),
            ];

            this.shiftButton = new components.Button({
                group: "[Channel1]",
                midi: [0x90, 0x32],
                type: components.Button.prototype.types.toggle,
                controller: this,
                input: function(_channel, _control, value, _status, _group) {
                    if (value === 0) {
                        return;
                    }
                    if (this.controller.isShifted) {
                        this.controller.unshift();
                    } else {
                        this.controller.shift();
                    }
                    midi.sendShortMsg(this.midi[0], this.midi[1], this.controller.isShifted ? 0x7F : 0x00);
                },
            });

            // Transport buttons
            this.recordButton = new components.Button({
                group: "[Recording]",
                midi: [0x90, 0x5F],
                inKey: "toggle_recording",
                outKey: "status",
            });

            // Jogwheel with navigation buttons selected
            this.jogUp = new components.Button({
                group: "[Library]",
                midi: [0x90, 0x60],
                inKey: "MoveUp",
            });
            this.jogDown = new components.Button({
                group: "[Library]",
                midi: [0x90, 0x60],
                inKey: "MoveDown",
            });
            this.jogLeft = new components.Button({
                group: "[Library]",
                midi: [0x90, 0x62],
                inKey: "MoveLeft",
            });
            this.jogRight = new components.Button({
                group: "[Library]",
                midi: [0x90, 0x63],
                inKey: "MoveRight",
            });
            this.jogButton = new components.Button({
                group: "[Library]",
                midi: [0x90, 0x65],
                inKey: "GoToItem",
            });
            this.focusMode = new components.Button({
                group: "[Library]",
                midi: [0x90, 0x64],
                jogUp: this.jogUp,
                jogDown: this.jogDown,
                jogLeft: this.jogLeft,
                jogRight: this.jogRight,
                jogButton: this.jogButton,
                input: function(_channel, _control, value, _status, _group) {
                    if (value === 0) {
                        return;
                    }
                    if (this.jogUp.inKey === "MoveUp") {
                        // TODO: this is not a good control use for the
                        // crossfader, maybe use one of the unused audio
                        // channels so the actual fader can be used?
                        this.jogLeft.inKey = "crossfader_down_small";
                        this.jogRight.inKey = "crossfader_up_small";
                        this.jogButton.inKey = "crossfader_set_default";
                        this.jogLeft.group = "[Master]";
                        this.jogRight.group = "[Master]";
                        this.jogButton.group = "[Master]";
                        this.jogUp.inKey = "MoveFocusBackward";
                        this.jogDown.inKey = "MoveFocusForward";
                    } else {
                        this.jogLeft.inKey = "MoveLeft";
                        this.jogRight.inKey = "MoveRight";
                        this.jogButton.inKey = "GoToItem";
                        this.jogLeft.group = "[Library]";
                        this.jogRight.group = "[Library]";
                        this.jogButton.group = "[Library]";
                        this.jogUp.inKey = "MoveUp";
                        this.jogDown.inKey = "MoveDown";
                    }
                },
            });

            this.trackColors = [];
            this.vuMeters = [];
            this.bpmMeters = [];
            this.fader = [];
            this.muteButton = [];
            this.soloButton = [];
            this.recordButton = [];
            for (let i = 0; i < 4; i++) {
                this.trackColors[i] = new components.Component({
                    group: `[Channel${i + 1}]`,
                    key: "track_color",
                    output: function() {
                        const cmd = [0xf0, 0x00, 0x02, 0x4e, 0x16, 0x14];
                        for (let i = 0; i < 4; i++) {
                            const trackColor = engine.getValue(`[Channel${i + 1}]`, this.key);
                            if (trackColor === -1) {
                                cmd.push(0x00, 0x00, 0x00);
                            } else {
                                const colorobj = colorCodeToObject(trackColor);
                                // Scale the color to fit a valid MIDI message.
                                for (const [key, val] of Object.entries(colorobj)) {
                                    if (typeof val === "number") {
                                        colorobj[key] = val / 0xFF * 0x7E;
                                    }
                                }
                                cmd.push(colorobj.red, colorobj.green, colorobj.blue);
                            }
                        }
                        for (let i = 0; i < 4; i++) {
                            // Unused screens (there's no way to set an individual screen, you
                            // have to set them all at once every time).
                            cmd.push(0x00, 0x00, 0x00);
                        }
                        cmd.push(0xF7);
                        midi.sendSysexMsg(cmd);
                    },
                });
                this.vuMeters[i] = new VuMeter({
                    group: `[Channel${i + 1}]`,
                });
                this.bpmMeters[i] = new components.Component({
                    group: `[Channel${i + 1}]`,
                    midi: [0x90, 0x10],
                    outKey: "bpm",
                    output: function(value) {
                        if (value === 0) {
                            printLCDDisplay(i, " ");
                        } else {
                            printLCDDisplay(i, value.toPrecision(4).toString());
                        }
                    },
                });

                this.muteButton[i] = new components.Button({
                    group: `[Channel${i + 1}]`,
                    midi: [0x90, 0x10 + i],
                    key: "mute",
                    type: components.Button.prototype.types.toggle,
                });
                this.soloButton[i] = new components.Button({
                    group: `[Channel${i + 1}]`,
                    midi: [0x90, 0x08 + i],
                    key: "beats_translate_curpos",
                });
                this.recordButton[i] = new components.Button({
                    group: `[Channel${i + 1}]`,
                    midi: [0x90, i],
                    inKey: "bpm_tap",
                    outKey: (() => {
                        if (engine.getSetting("enableBPMBlink")) {
                            return "beat_active";
                        }
                        return undefined;
                    })(),
                });

                // TODO: these are 14-bit, so using the default output means the
                // resolution is less than the input, so the fader sometimes
                // "jumps" after you move it to whatever the output value is
                // telling it to be at.
                this.fader[i] = new components.Encoder({
                    group: `[Channel${i + 1}]`,
                    key: "volume",
                    midi: [0xE0 + i, 0x00],
                    softTakeover: false,
                    outValueScale: function(value) {
                        // If we're shifted we're a rate fader, so scale the
                        // -1..1 range to the normal 0 to max.
                        if (this.inKey === "rate") {
                            return ((value + 1) / 2) * this.max;
                        }
                        // Otherwise we have a normal range and can do the
                        // normal thing.
                        return components.Encoder.prototype.outValueScale.call(this, value);
                    },
                    shift: function() {
                        this.key = "rate";
                        this.inKey = "rate";
                        this.outKey = "rate";
                        this.disconnect();
                        this.connect();
                        this.trigger();
                    },
                    unshift: function() {
                        this.key = "volume";
                        this.inKey = "volume";
                        this.outKey = "volume";
                        this.disconnect();
                        this.connect();
                        this.trigger();
                    },
                });
            }

            this.fader.push(new components.Encoder({
                group: "[Master]",
                key: "gain",
                midi: [0xE8, 0x00],
                // The 0 mark of the fader (which we want to correspond to 0.5,
                // the middle of the gain knob) is not actually the center of
                // the physical fader, so set a mid point value, check if we're
                // above or below the fake mid point, and scale appropriately.
                // TODO: what is the actual 0 value on the fader? 0x62? 0x64?
                mid: 0x63,
                inValueScale: function(value) {
                    if (value >= this.mid) {
                        return (((value - this.mid) / (this.max - this.mid)) * 0.5) + 0.5;
                    } else {
                        return value / (this.mid + 1) / 2;
                    }
                },
                outValueScale: function(value) {
                    if (value > 1) {
                        return (((value - 1) / (5 - 1)) * (this.max - this.mid)) + this.mid;
                    } else {
                        return value * this.mid;
                    }
                },
            }));
            this.vuMeters.push(new VuMeter({
                group: "[Main]",
                midi: [0xD1],
            }));
        }

        deckSelectInput(_channel, _control, value, _status, group) {
            if (value === 0x00) {
                // No need to select the deck on press and then re-select it on
                // release.
                return;
            }
            this.activeDeck.setCurrentDeck(group);
            const deck = script.deckFromGroup(group);
            for (let i = 0; i < 4; i++) {
                this.knob[i].selected = deck === (i + 1);
                this.knob[i].trigger();
            }
        }
    }

    P1Nano.init = function() {
        P1Nano.controller = new Controller();
    };
    P1Nano.shutdown = function() {
        P1Nano.controller.shutdown();
    };
})(P1Nano || (P1Nano = {}));

// vim:expandtab:shiftwidth=4:tabstop=4:backupcopy=yes
