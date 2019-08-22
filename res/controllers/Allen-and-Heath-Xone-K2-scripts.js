var XoneK2 = {};

XoneK2.seekRateFast = 3.0;
XoneK2.seekRateSlow = 0.5;

XoneK2.decksInMiddleMidiChannel = 0xE;
XoneK2.effectsInMiddleMidiChannel = 0xD;
XoneK2.decksOnLeftMidiChannel = 0xC;
XoneK2.decksOnRightMidiChannel = 0xB;
XoneK2.fourDecksMidiChannel3124 = 0xA;
XoneK2.fourDecksMidiChannel1234 = 0x9;
XoneK2.fourEffectsMidiChannel3124 = 0x8;
XoneK2.fourEffectsMidiChannel1234 = 0x7;

// The MIDI note offsets for different colors with the layer button is different
// from the rest of the buttons.
XoneK2.layerButtonColors = {
    red: 0x0C,
    amber: 0x10,
    green: 0x14,
}
XoneK2.deckBottomButtonLayers = [
    { name: 'intro_outro', layerButtonNoteNumber: XoneK2.layerButtonColors.amber },
    { name: 'hotcue', layerButtonNoteNumber: XoneK2.layerButtonColors.red },
    { name: 'loop', layerButtonNoteNumber: XoneK2.layerButtonColors.green }, ];

// Multiple K2s/K1s can be connected via X-Link and plugged in with one USB
// cable. The MIDI messages of the controllers can be distinguished by setting
// each one to its own MIDI channel. The XoneK2.controllers array maintains state
// for each controller. This also allows the same mapping to  be loaded for
// different use cases as long as the user sets the appropriate MIDI channel for
// the mapping they want.
XoneK2.controllers = [];
for (var ch = 0; ch <= 0xF; ++ch) {
    XoneK2.controllers[ch] = [];
    XoneK2.controllers[ch].columns = [];
    XoneK2.controllers[ch].isShifted = false;
    XoneK2.controllers[ch].leftEncoderIsPressed = false;
    XoneK2.controllers[ch].rightEncoderIsPressed = false;
    XoneK2.controllers[ch].deckPicked = false;
    // This gets incremented to 0 by the init function calling XoneK2.decksLayerButton
    XoneK2.controllers[ch].deckLayerIndex = -1;
    XoneK2.controllers[ch].focusedEffectUnit = 1;
    XoneK2.controllers[ch].singleEffectUnitModeActive = false;
}

XoneK2.init = function (id) {
    var channel = XoneK2.decksInMiddleMidiChannel;
    XoneK2.controllers[channel].columns[1] = new XoneK2.EffectUnit(1, 1, channel, true);
    XoneK2.controllers[channel].columns[2] = new XoneK2.Deck(2, 1, channel);
    XoneK2.controllers[channel].columns[3] = new XoneK2.Deck(3, 2, channel);
    XoneK2.controllers[channel].columns[4] = new XoneK2.EffectUnit(4, 2, channel, true);
    XoneK2.decksLayerButton(channel, null, null, 0x90 + channel, null);

    channel = XoneK2.effectsInMiddleMidiChannel;
    XoneK2.controllers[channel].columns[1] = new XoneK2.Deck(1, 1, channel);
    XoneK2.controllers[channel].columns[2] = new XoneK2.EffectUnit(2, 1, channel, true);
    XoneK2.controllers[channel].columns[3] = new XoneK2.EffectUnit(3, 2, channel, true);
    XoneK2.controllers[channel].columns[4] = new XoneK2.Deck(4, 2, channel);
    XoneK2.decksLayerButton(channel, null, null, 0x90 + channel, null);

    channel = XoneK2.decksOnLeftMidiChannel;
    XoneK2.controllers[channel].columns[1] = new XoneK2.Deck(1, 1, channel);
    XoneK2.controllers[channel].columns[2] = new XoneK2.Deck(2, 2, channel);
    XoneK2.controllers[channel].columns[3] = new XoneK2.EffectUnit(3, 1, channel, true);
    XoneK2.controllers[channel].columns[4] = new XoneK2.EffectUnit(4, 2, channel, true);
    XoneK2.decksLayerButton(channel, null, null, 0x90 + channel, null);

    channel = XoneK2.decksOnRightMidiChannel;
    XoneK2.controllers[channel].columns[1] = new XoneK2.EffectUnit(1, 1, channel, true);
    XoneK2.controllers[channel].columns[2] = new XoneK2.EffectUnit(2, 2, channel, true);
    XoneK2.controllers[channel].columns[3] = new XoneK2.Deck(3, 1, channel);
    XoneK2.controllers[channel].columns[4] = new XoneK2.Deck(4, 2, channel);
    XoneK2.decksLayerButton(channel, null, null, 0x90 + channel, null);

    channel = XoneK2.fourDecksMidiChannel3124;
    XoneK2.controllers[channel].columns[1] = new XoneK2.Deck(1, 3, channel);
    XoneK2.controllers[channel].columns[2] = new XoneK2.Deck(2, 1, channel);
    XoneK2.controllers[channel].columns[3] = new XoneK2.Deck(3, 2, channel);
    XoneK2.controllers[channel].columns[4] = new XoneK2.Deck(4, 4, channel);
    XoneK2.decksLayerButton(channel, null, null, 0x90 + channel, null);

    channel = XoneK2.fourDecksMidiChannel1234;
    XoneK2.controllers[channel].columns[1] = new XoneK2.Deck(1, 1, channel);
    XoneK2.controllers[channel].columns[2] = new XoneK2.Deck(2, 2, channel);
    XoneK2.controllers[channel].columns[3] = new XoneK2.Deck(3, 3, channel);
    XoneK2.controllers[channel].columns[4] = new XoneK2.Deck(4, 4, channel);
    XoneK2.decksLayerButton(channel, null, null, 0x90 + channel, null);

    channel = XoneK2.fourEffectsMidiChannel3124;
    XoneK2.controllers[channel].columns[1] = new XoneK2.EffectUnit(1, 3, channel);
    XoneK2.controllers[channel].columns[2] = new XoneK2.EffectUnit(2, 1, channel);
    XoneK2.controllers[channel].columns[3] = new XoneK2.EffectUnit(3, 2, channel);
    XoneK2.controllers[channel].columns[4] = new XoneK2.EffectUnit(4, 4, channel);

    channel = XoneK2.fourEffectsMidiChannel1234;
    XoneK2.controllers[channel].columns[1] = new XoneK2.EffectUnit(1, 1, channel);
    XoneK2.controllers[channel].columns[2] = new XoneK2.EffectUnit(2, 2, channel);
    XoneK2.controllers[channel].columns[3] = new XoneK2.EffectUnit(3, 3, channel);
    XoneK2.controllers[channel].columns[4] = new XoneK2.EffectUnit(4, 4, channel);
}

XoneK2.shutdown = function(id) {
    var turnOff = function (component) {
        component.send(0);
    };
    for (var z = 1; z <= 4; z++) {
        XoneK2.controllers[XoneK2.effectsMidiChannel].columns[z].forEachComponent(turnOff);
        XoneK2.controllers[XoneK2.decksMidiChannel].columns[z].forEachComponent(turnOff);
    }
}


XoneK2.decksBottomLeftEncoderPress = function (channel, control, value, status) {
    XoneK2.controllers[channel].leftEncoderIsPressed =  (status & 0xF0) === 0x90;
    if (XoneK2.controllers[channel].isShifted && XoneK2.controllers[channel].leftEncoderIsPressed) {
        script.toggleControl('[Master]', 'headSplit');
    }
};
XoneK2.decksBottomLeftEncoder = function (channel, control, value, status) {
    if (!XoneK2.controllers[channel].isShifted) {
        if (!XoneK2.controllers[channel].leftEncoderIsPressed) {
            var bpm = engine.getValue("[InternalClock]", "bpm");
            if (value === 1) {
                bpm += 0.1;
            } else {
                bpm -= 0.1;
            }
            engine.setValue("[InternalClock]", "bpm", bpm);
        } else {
            var mix = engine.getValue("[Master]", "headMix");
            if (value === 1) {
                mix += 1;
            } else {
                mix -= 1;
            }
            engine.setValue("[Master]", "headMix", mix);
        }
    } else {
        var gain = engine.getValue("[Master]", "headGain");
        if (value === 1) {
            gain += 0.025;
        } else {
            gain -= 0.025;
        }
        engine.setValue("[Master]", "headGain", gain);
    }
};

XoneK2.decksBottomRightEncoderPress = function (channel, control, value, status) {
    XoneK2.controllers[channel].rightEncoderIsPressed = (status & 0xF0) === 0x90;
    if (XoneK2.controllers[channel].rightEncoderIsPressed) {
        for (var x = 1; x <= 4; ++x) {
            var deckColumn = XoneK2.controllers[channel].columns[x];
            if (!(deckColumn instanceof components.Deck)) {
                continue;
            }
            deckColumn.topButtons[3].startDeckPickMode();
        }
    } else {
        for (var x = 1; x <= 4; ++x) {
            var deckColumn = XoneK2.controllers[channel].columns[x];
            if (!(deckColumn instanceof components.Deck)) {
                continue;
            }
            deckColumn.topButtons[3].stopDeckPickMode();
        }

        if (XoneK2.controllers[channel].deckPicked === true) {
            XoneK2.controllers[channel].deckPicked = false;
        } else {
            engine.setValue("[Playlist]", "LoadSelectedIntoFirstStopped", 1);
        }
    }
};
XoneK2.decksBottomRightEncoder = function (channel, control, value, status) {
    if (!XoneK2.controllers[channel].isShifted) {
        var bpm = engine.getValue("[InternalClock]", "bpm");
        if (value === 1) {
            engine.setValue("[Playlist]", "SelectNextTrack", 1);
        } else {
            engine.setValue("[Playlist]", "SelectPrevTrack", 1);
        }
        engine.setValue("[InternalClock]", "bpm", bpm);
    } else {
        var gain = engine.getValue("[Master]", "gain");
        if (value === 1) {
            gain += 0.025;
        } else {
            gain -= 0.025;
        }
        engine.setValue("[Master]", "gain", gain);
    }
};

XoneK2.shiftButton = function (channel, control, value, status) {
    XoneK2.controllers[channel].isShifted = (status & 0xF0) === 0x90;
    if (XoneK2.controllers[channel].isShifted) {
        for (var z = 1; z <= 4; z++) {
            XoneK2.controllers[channel].columns[z].shift();
        }
        midi.sendShortMsg(status, 0x0F, 0x7F);
    } else {
        for (var z = 1; z <= 4; z++) {
            XoneK2.controllers[channel].columns[z].unshift();
        }
        midi.sendShortMsg(status, 0x0F, 0x00);
    }
};

// The Xone K2 uses different control numbers (second MIDI byte) to distinguish between
// different colors for the LEDs. The baseline control number sets the LED to red. Adding
// these offsets to the control number sets the LED to a different color.
XoneK2.color = {
    red: 0,
    amber: 36,
    green: 72
};
components.Component.prototype.color = XoneK2.color.red;
components.Component.prototype.send =  function (value) {
    if (this.midi === undefined || this.midi[0] === undefined || this.midi[1] === undefined) {
        return;
    }
    // The LEDs are turned on with a Note On MIDI message (first nybble of first byte 9)
    // and turned off with a Note Off MIDI message (first nybble of first byte 8).
    if (value > 0) {
        midi.sendShortMsg(this.midi[0] + 0x10, this.midi[1] + this.color, value);
    } else {
        midi.sendShortMsg(this.midi[0], this.midi[1], 0x7F);
    }
};
components.Button.prototype.isPress = function (channel, control, value, status) {
    return (status & 0xF0) === 0x90;
}

XoneK2.setTopEncoderPressMidi = function (topEncoderPressObject, columnNumber, midiChannel) {
    topEncoderPressObject.midi = [0x80 + midiChannel, 0x34 + (columnNumber-1)];
}

XoneK2.setTopButtonsMidi = function (topButtonsObject, columnNumber, midiChannel) {
    for (var c = 1; c <= 3; c++) {
        topButtonsObject[c].midi = [0x80 + midiChannel,
                                    0x30 - (c-1)*4 + (columnNumber-1)];
    }
};

XoneK2.setBottomButtonsMidi = function (bottomButtonsObject, columnNumber, midiChannel) {
    for (var c = 1; c <= 4; c++) {
        bottomButtonsObject[c].midi = [0x80 + midiChannel,
                                       0x24 - (c-1)*4 + (columnNumber-1)];
    }
};

XoneK2.setColumnMidi = function (columnObject, columnNumber, midiChannel) {
    XoneK2.setTopEncoderPressMidi(columnObject.encoderPress, columnNumber, midiChannel);
    XoneK2.setTopButtonsMidi(columnObject.topButtons, columnNumber, midiChannel);
    XoneK2.setBottomButtonsMidi(columnObject.bottomButtons, columnNumber, midiChannel);
};

XoneK2.Deck = function (column, deckNumber, midiChannel) {
    var theDeck = this;

    this.deckString = '[Channel' + deckNumber + ']';

    this.encoder = new components.Encoder({
        unshift: function () {
            this.input = function (channel, control, value, status) {
                direction = (value === 1) ? 1 : -1;
                var gain = engine.getValue(this.group, "pregain");
                engine.setValue(this.group, "pregain", gain + 0.025 * direction);
            };
        },
        shift: function () {
            this.input = function (channel, control, value, status) {
                direction = (value === 1) ? 1 : -1;
                engine.setValue(this.group, "jog", direction);
            };
        },
        supershift: function () {
            this.input = function (channel, control, value, status) {
                direction = (value === 1) ? 1 : -1;
                var pitch = engine.getValue(this.group, "pitch");
                engine.setValue(this.group, "pitch", pitch + (.05 * direction));
            };
        },
    });

    this.encoderPress = new components.Button({
        outKey: 'sync_enabled',
        unshift: function () {
            this.inKey = 'pregain_set_one';
            this.type = components.Button.prototype.types.push;
        },
        shift: function () {
            this.inKey = 'sync_enabled';
            this.type = components.Button.prototype.types.toggle;
        },
        supershift: function () {
            this.inKey = 'reset_key';
            this.type = components.Button.prototype.types.push;
        },
    });

    this.knobs = new components.ComponentContainer();
    for (var k = 1; k <= 3; k++) {
        this.knobs[k] = new components.Pot({
            group: '[EqualizerRack1_' + this.deckString + '_Effect1]',
            inKey: 'parameter' + (4-k),
        });
    }

    this.fader = new components.Pot({inKey: 'volume'});

    this.topButtons = new components.ComponentContainer();
    this.topButtons[1] = new components.Button({
        unshift: function () {
            this.disconnect();
            this.type = components.Button.prototype.types.toggle;
            this.inKey = 'pfl';
            this.outKey = 'pfl';
            this.color = XoneK2.color.red;
            this.connect();
            this.trigger();
        },
        shift: function () {
            this.disconnect();
            this.type = components.Button.prototype.types.push;
            this.inKey = 'rate_set_zero';
            this.outKey = 'pfl';
            this.color = XoneK2.color.red;
            this.connect();
            this.trigger();
        },
        supershift: function () {
            this.disconnect();
            this.type = components.Button.prototype.types.push;
            this.inKey = 'beats_translate_curpos';
            this.outKey = 'beats_translate_curpos';
            this.color = XoneK2.color.amber;
            this.connect();
            this.trigger();
        },
    });
    this.topButtons[2] = new components.Button({
        unshift: function () {
            this.disconnect();
            this.type = components.Button.prototype.types.push;
            this.inKey = 'cue_default';
            this.outKey = 'cue_indicator';
            this.color = XoneK2.color.red;
            this.connect();
            this.trigger();
        },
        shift: function () {
            this.disconnect();
            this.type = components.Button.prototype.types.push;
            this.inKey = 'start_stop';
            this.outKey = 'cue_indicator';
            this.color = XoneK2.color.red;
            this.connect();
            this.trigger();
        },
        supershift: function () {
            this.disconnect();
            this.type = components.Button.prototype.types.toggle;
            this.inKey = 'keylock';
            this.outKey = 'keylock';
            this.color = XoneK2.color.amber;
            this.connect();
            this.trigger();
        },
    });
    this.topButtons[3] = new components.Button({
        unshift: function () {
            this.disconnect();
            this.inKey = 'play';
            this.outKey = 'play_indicator';
            this.color = XoneK2.color.red;
            this.connect();
            this.trigger();
        },
        shift: function () {
            this.disconnect();
            this.inKey = 'reverse';
            this.outKey = 'play_indicator';
            this.color = XoneK2.color.red;
            this.connect();
            this.trigger();
        },
        supershift: function () {
            this.disconnect();
            this.inKey = 'quantize';
            this.outKey = 'quantize';
            this.color = XoneK2.color.amber;
            this.connect();
            this.trigger();
        },
        startDeckPickMode: function () {
            this.input = function (channel, control, value, status) {
                if (this.isPress(channel, control, value, status)) {
                    engine.setValue(this.group, "LoadSelectedTrack", 1);
                    XoneK2.controllers[channel].deckPicked = true;
                }
            };
        },
        stopDeckPickMode: function () {
            // The inKey and outKey are still set from before startDeckPickMode was
            // called, so all that is needed to get back to that mode is to fall back
            // to the prototype input function.
            this.input = components.Button.prototype.input;
        },
        type: components.Button.prototype.types.toggle,
    });

    // This should not be a ComponentContainer, otherwise strange things will
    // happen when iterating over the Deck with reconnectComponents.
    this.bottomButtonLayers = [];

    var CueAndSeekButton = function (options) {
        if (options.cueName === undefined) {
            print('ERROR! cueName not specified');
        } else if (options.seekRate === undefined) {
            print('ERROR! seekRate not specified');
        }

        this.outKey = options.cueName + '_enabled';
        components.Button.call(this, options);
    };
    CueAndSeekButton.prototype = new components.Button({
        unshift: function () {
            this.inKey = this.cueName + '_activate';
            this.input = components.Button.prototype.input;
            // Avoid log spam on startup
            if (this.group !== undefined) {
                engine.setValue(this.group, 'rateSearch', 0);
            }
        },
        shift: function () {
            this.input = function (channel, control, value, status) {
                if (components.Button.prototype.isPress(channel, control, value, status)) {
                    engine.setValue(this.group, 'rateSearch', this.seekRate);
                } else {
                    engine.setValue(this.group, 'rateSearch', 0);
                }
            };
        },
        supershift: function () {
            this.inKey = this.cueName + '_clear';
            this.input = components.Button.prototype.input;
            engine.setValue(this.group, 'rateSearch', 0);
        }
    });

    this.bottomButtonLayers.intro_outro = new components.ComponentContainer();
    this.bottomButtonLayers.intro_outro[1] = new CueAndSeekButton({
        cueName: "intro_start",
        seekRate: XoneK2.seekRateFast,
        color: XoneK2.color.amber,
    });
    this.bottomButtonLayers.intro_outro[2] = new CueAndSeekButton({
        cueName: "intro_end",
        seekRate: -1 * XoneK2.seekRateFast,
        color: XoneK2.color.amber,
    });
    this.bottomButtonLayers.intro_outro[3] = new CueAndSeekButton({
        cueName: "outro_start",
        seekRate: XoneK2.seekRateSlow,
        color: XoneK2.color.amber,
    });
    this.bottomButtonLayers.intro_outro[4] = new CueAndSeekButton({
        cueName: "outro_end",
        seekRate: -1 * XoneK2.seekRateSlow,
        color: XoneK2.color.amber,
    });


    this.bottomButtonLayers.hotcue = new components.ComponentContainer();
    this.bottomButtonLayers.hotcue[1] = new CueAndSeekButton({
        cueName: "hotcue_1",
        seekRate: XoneK2.seekRateFast,
        color: XoneK2.color.red,
    });
    this.bottomButtonLayers.hotcue[2] = new CueAndSeekButton({
        cueName: "hotcue_2",
        seekRate: -1 * XoneK2.seekRateFast,
        color: XoneK2.color.red,
    });
    this.bottomButtonLayers.hotcue[3] = new CueAndSeekButton({
        cueName: "hotcue_3",
        seekRate: XoneK2.seekRateSlow,
        color: XoneK2.color.red,
    });
    this.bottomButtonLayers.hotcue[4] = new CueAndSeekButton({
        cueName: "hotcue_4",
        seekRate: -1 * XoneK2.seekRateSlow,
        color: XoneK2.color.red,
    });


    this.bottomButtonLayers.loop = new components.ComponentContainer();

    this.bottomButtonLayers.loop[1] = new components.Button({
        outKey: 'loop_enabled',
        unshift: function () {
            this.inKey = 'reloop_toggle';
        },
        shift: function () {
            this.inKey = 'reloop_andstop';
        },
        supershift: function () {
            this.inKey = 'loop_in';
        },
        color: XoneK2.color.red,
    });

    this.bottomButtonLayers.loop[2] = new components.Button({
        unshift: function () {
            this.inKey = 'beatloop_activate';
        },
        shift: function () {
            this.inKey = 'beatlooproll_activate';
        },
        supershift: function () {
            this.inKey = 'loop_out';
        },
        trigger: function() {
            this.send(this.on);
        },
        color: XoneK2.color.green,
    });

    this.bottomButtonLayers.loop[3] = new components.Button({
        unshift: function () {
            this.inKey = 'loop_double';
            this.input = components.Button.prototype.input;
        },
        shift: function () {
            this.inKey = 'beatjump_forward';
            this.input = components.Button.prototype.input;
        },
        supershift: function () {
            this.input = function (channel, control, value, status) {
                if (this.isPress(channel, control, value, status)) {
                    engine.setValue(this.group, 'beatjump_size',
                                    engine.getValue(this.group, 'beatjump_size') * 2);
                }
            };
        },
        trigger: function() {
            this.send(this.on);
        },
        color: XoneK2.color.amber,
    });

    this.bottomButtonLayers.loop[4] = new components.Button({
        unshift: function () {
            this.inKey = 'loop_halve';
            this.input = components.Button.prototype.input;
        },
        shift: function () {
            this.inKey = 'beatjump_backward';
            this.input = components.Button.prototype.input;
        },
        supershift: function () {
            this.input = function (channel, control, value, status) {
                if (this.isPress(channel, control, value, status)) {
                    engine.setValue(this.group, 'beatjump_size',
                                    engine.getValue(this.group, 'beatjump_size') / 2);
                }
            };
        },
        trigger: function() {
            this.send(this.on);
        },
        color: XoneK2.color.amber,
    });

    var setGroup = function (component) {
        if (component.group === undefined) {
            component.group = theDeck.deckString;
        }
    };

    for (var memberName in this.bottomButtonLayers) {
        if (this.bottomButtonLayers.hasOwnProperty(memberName)) {
            XoneK2.setBottomButtonsMidi(this.bottomButtonLayers[memberName], column, midiChannel);
            this.bottomButtonLayers[memberName].forEachComponent(setGroup);
        }
    }

    this.bottomButtons = this.bottomButtonLayers[XoneK2.deckBottomButtonLayers[0].name];

    XoneK2.setColumnMidi(this, column, midiChannel);
    this.reconnectComponents(setGroup);

};
XoneK2.Deck.prototype = new components.Deck();

XoneK2.decksLayerButton = function (channel, control, value, status) {
    if (!XoneK2.controllers[channel].isShifted) {
        // Cycle the deck layers
        if (components.Button.prototype.isPress(channel, control, value, status)) {
            XoneK2.controllers[channel].deckLayerIndex++;
            if (XoneK2.controllers[channel].deckLayerIndex === XoneK2.deckBottomButtonLayers.length) {
                XoneK2.controllers[channel].deckLayerIndex = 0;
            }
            var newLayer = XoneK2.deckBottomButtonLayers[XoneK2.controllers[channel].deckLayerIndex];

            for (var x = 1; x <= 4; ++x) {
                var deckColumn = XoneK2.controllers[channel].columns[x];
                if (!(deckColumn instanceof components.Deck)) {
                    continue;
                }

                deckColumn.bottomButtons.forEachComponent(function (c) {
                    c.disconnect();
                });
                deckColumn.bottomButtons = deckColumn.bottomButtonLayers[newLayer.name];
                deckColumn.bottomButtons.forEachComponent(function (c) {
                    c.connect();
                    c.trigger();
                });
            }
            midi.sendShortMsg(status, newLayer.layerButtonNoteNumber, 0x7F);
        }
    } else {
        if (components.Button.prototype.isPress(channel, control, value, status)) {
            // Activate supershift mode
            var supershift = function (c) {
                if (c.supershift !== undefined) {
                    c.supershift();
                }
            };

            for (var x = 1; x <= 4; ++x) {
                var deckColumn = XoneK2.controllers[channel].columns[x];
                if (!(deckColumn instanceof components.Deck)) {
                    continue;
                }
                deckColumn.forEachComponent(supershift);
                deckColumn.bottomButtons.forEachComponent(supershift);
            }
        } else {
            // Shift button is still held down, so exit supershift mode by going back to
            // plain shift mode
            var shift = function (c) {
                if (c.supershift !== undefined) {
                    c.shift();
                }
            };

            for (var x = 1; x <= 4; ++x) {
                var deckColumn = XoneK2.controllers[channel].columns[x];
                if (!(deckColumn instanceof components.Deck)) {
                    continue;
                }
                deckColumn.forEachComponent(shift);
                deckColumn.bottomButtons.forEachComponent(shift);
            }
        }
    }
};

XoneK2.EffectUnit = function (column, unitNumber, midiChannel, twoDeck) {
    // "library" refers to the Components library
    // This is a private variable rather than a property of XoneK2.EffectUnit
    // so that components.ComponentContainer.prototype.shift/unshift do not
    // call the shift/unshift methods of the components in libraryEffectUnit
    // when a single effect unit is focused.
    var libraryEffectUnit = new components.EffectUnit([unitNumber], false, {
        unfocused: XoneK2.color.red,
        focusChooseMode: XoneK2.color.green,
        focused: XoneK2.color.amber,
    });

    var unitString = '[EffectRack1_EffectUnit' + unitNumber + ']';

    this.hadParametersShowing = engine.getValue(unitString, 'show_parameters');
    this.hadFocusShowing = engine.getValue(unitString, 'show_focus');

    this.fader = libraryEffectUnit.dryWetKnob;

    this.bottomButtons = new components.ComponentContainer();
    var channelString;
    libraryEffectUnit.enableOnChannelButtons.addButton('Channel1');
    this.bottomButtons[1] = libraryEffectUnit.enableOnChannelButtons.Channel1;
    libraryEffectUnit.enableOnChannelButtons.addButton('Channel2');
    this.bottomButtons[2] = libraryEffectUnit.enableOnChannelButtons.Channel2;
    if (twoDeck === true) {
        libraryEffectUnit.enableOnChannelButtons.addButton('Master');
        this.bottomButtons[3] = libraryEffectUnit.enableOnChannelButtons.Master;
        libraryEffectUnit.enableOnChannelButtons.addButton('Headphone');
        this.bottomButtons[4] = libraryEffectUnit.enableOnChannelButtons.Headphone;
    } else {
        this.bottomButtons[3] = new components.Button({
            group: unitString,
            unshift: function () {
                this.disconnect();
                this.inKey = 'group_[Channel3]_enable';
                this.outKey = 'group_[Channel3]_enable';
                this.color = XoneK2.color.red;
                this.connect();
                this.trigger();
            },
            shift: function () {
                this.disconnect();
                this.inKey = 'group_[Master]_enable';
                this.outKey = 'group_[Master]_enable';
                this.color = XoneK2.color.amber;
                this.connect();
                this.trigger();
            },
            type: components.Button.prototype.types.toggle,
        });
        this.bottomButtons[4] = new components.Button({
            group: unitString,
            unshift: function () {
                this.disconnect();
                this.inKey = 'group_[Channel4]_enable';
                this.outKey = 'group_[Channel4]_enable';
                this.color = XoneK2.color.red;
                this.connect();
                this.trigger();
            },
            shift: function () {
                this.disconnect();
                this.inKey = 'group_[Headphone]_enable';
                this.outKey = 'group_[Headphone]_enable';
                this.color = XoneK2.color.amber;
                this.connect();
                this.trigger();
            },
            type: components.Button.prototype.types.toggle,
        });
    }

    this.encoder = new components.Component({
        // TODO: figure out a use for this. Maybe switching chain presets?
        input: function () {},
    });

    this.topButtons = new components.ComponentContainer();
    this.knobs = new components.ComponentContainer();

    this.useLibraryEffectUnit = function () {
        //print('*************************************************** COLUMN '
        //    + column + ' USING LIBRARY UNIT');

        this.knobs.forEachComponent(function (component) {
            component.disconnect();
        });
        this.topButtons.forEachComponent(function (component) {
            component.disconnect();
        });

        this.encoderPress = libraryEffectUnit.effectFocusButton;
        this.knobs = libraryEffectUnit.knobs;
        this.topButtons = libraryEffectUnit.enableButtons;

        engine.setValue(unitString, 'show_focus', this.hadFocusShowing);
        engine.setValue(unitString, 'show_parameters', this.hadParametersShowing);

        XoneK2.setColumnMidi(this, column, midiChannel);
        if (libraryEffectUnit.hasInitialized) {
            libraryEffectUnit.showParametersConnection =
                engine.makeConnection(unitString,
                                      'show_parameters',
                                      libraryEffectUnit.onShowParametersChange);

            libraryEffectUnit.knobs.reconnectComponents();
            libraryEffectUnit.enableButtons.reconnectComponents();
            libraryEffectUnit.effectFocusButton.connect();
            libraryEffectUnit.effectFocusButton.trigger();
            libraryEffectUnit.showParametersConnection.trigger();
        } else {
            libraryEffectUnit.init();
        }

    };
    this.useLibraryEffectUnit();

    this.unitFocusButton = new components.Button({
        input: function (channel, control, value, status) {
            if (this.isPress(channel, control, value, status)) {
                if (XoneK2.controllers[channel].focusedEffectUnit === unitNumber) {
                    // Prevent flickering
                    return;
                }

                for (var x = 1; x <= 4; ++x) {
                    var effectUnitColumn = XoneK2.controllers[channel].columns[x];
                    if (!(effectUnitColumn instanceof XoneK2.EffectUnit)) {
                        continue;
                    }

                    XoneK2.controllers[channel].focusedEffectUnit = unitNumber;
                    effectUnitColumn.focusUnit(unitNumber);
                }
            }
        },
        color: XoneK2.color.red,
    });
    XoneK2.setTopEncoderPressMidi(this.unitFocusButton, column, midiChannel);

    this.disconnectShowParameters = function () {
        libraryEffectUnit.showParametersConnection.disconnect();
    };

    this.focusUnit = function (focusedUnitNumber) {
        //print('================================================== COLUMN '
        //    + column + ' FOCUSING UNIT ' + focusedUnitNumber);

        libraryEffectUnit.effectFocusButton.disconnect();
        // The showParametersConnection connection does not belong to any specific
        // Component, so it must be disconnected manually. This script creates
        // objects for every potential layout on different MIDI channels. The
        // showParametersConnection for every XoneK2.EffectUnit must be disconnected
        // or the connections for other MIDI channels will interfere with
        // the MIDI channel actually being used.
        for (var n = 0; n <= 0xF; n++) {
            var col = XoneK2.controllers[n].columns[column];
            if (col instanceof XoneK2.EffectUnit) {
                col.disconnectShowParameters();
            }
        }
        libraryEffectUnit.showParametersConnection.disconnect();
        this.knobs.forEachComponent(function (component) {
            component.disconnect();
        });
        this.topButtons.forEachComponent(function (component) {
            component.disconnect();
        });

        if (!XoneK2.controllers[midiChannel].singleEffectUnitModeActive) {
            this.hadFocusShowing = engine.getValue(unitString, 'show_focus');
            this.hadParametersShowing = engine.getValue(unitString, 'show_parameters');
        }
        engine.setValue(unitString, 'show_focus', 0);
        engine.setValue(unitString, 'show_parameters', focusedUnitNumber === unitNumber);

        this.encoderPress = this.unitFocusButton;
        this.unitFocusButton.send(focusedUnitNumber === unitNumber);

        // The containers must be reassigned to new objects before reassigning
        // the Components within them. Otherwise, the Components in
        // libraryEffectUnit will get reassigned too.
        this.knobs = new components.ComponentContainer();
        this.topButtons = new components.ComponentContainer();
        for (var k = 1; k <= 3; k++) {
            this.knobs[k] = new components.Pot({
                group: '[EffectRack1_EffectUnit' + focusedUnitNumber + '_Effect' + k + ']',
                inKey: 'parameter' + column,
                unshift: function () {
                    this.input = function (channel, control, value, status, group) {
                        this.inSetParameter(this.inValueScale(value));

                        if (this.previousValueReceived === undefined) {
                            engine.softTakeover(this.group, this.inKey, true);
                        }
                        this.previousValueReceived = value;
                    };
                },
                shift: function () {
                    engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
                    this.valueAtLastEffectSwitch = this.previousValueReceived;
                    // Floor the threshold to ensure that every effect can be selected
                    this.changeThreshold = Math.floor(this.max /
                        engine.getValue('[Master]', 'num_effectsavailable'));

                    this.input = function (channel, control, value, status, group) {
                        var change = value - this.valueAtLastEffectSwitch;
                        if (Math.abs(change) >= this.changeThreshold
                            // this.valueAtLastEffectSwitch can be undefined if
                            // shift was pressed before the first MIDI value was received.
                            || this.valueAtLastEffectSwitch === undefined) {
                            engine.setValue(this.group, 'effect_selector', change);
                            this.valueAtLastEffectSwitch = value;
                        }

                        this.previousValueReceived = value;
                    };
                },
            });

            if (column === 1) {
                this.topButtons[k] = new components.Button({
                    group: '[EffectRack1_EffectUnit' + focusedUnitNumber + '_Effect' + k + ']',
                    key: 'enabled',
                    type: components.Button.prototype.types.powerWindow,
                    color: XoneK2.color.amber,
                    outConnect: false, // midi is not defined yet
                });
            } else {
                this.topButtons[k] = new components.Button({
                    group: '[EffectRack1_EffectUnit' + focusedUnitNumber + '_Effect' + k + ']',
                    key: 'button_parameter' + (column-1),
                    type: components.Button.prototype.types.powerWindow,
                    color: XoneK2.color.green,
                    outConnect: false, // midi is not defined yet
                });
            }
        }
        XoneK2.setTopButtonsMidi(this.topButtons, column, midiChannel);
        this.knobs.reconnectComponents();
        this.topButtons.reconnectComponents();
    };
};
XoneK2.EffectUnit.prototype = new components.ComponentContainer();

// This is only used for the 4 effect unit layouts
XoneK2.effectsLayerButton = function (channel, control, value, status) {
    if (components.Button.prototype.isPress(channel, control, value, status)) {
        for (var x = 1; x <= 4; ++x) {
            var effectUnitColumn = XoneK2.controllers[channel].columns[x];
            if (!(effectUnitColumn instanceof XoneK2.EffectUnit)) {
                continue;
            }

            if (XoneK2.controllers[channel].singleEffectUnitModeActive === true) {
                effectUnitColumn.useLibraryEffectUnit();
            } else {
                effectUnitColumn.focusUnit(XoneK2.controllers[channel].focusedEffectUnit);
            }
        }

        XoneK2.controllers[channel].singleEffectUnitModeActive =
            !XoneK2.controllers[channel].singleEffectUnitModeActive;

        if (XoneK2.controllers[channel].singleEffectUnitModeActive) {
            midi.sendShortMsg(status, XoneK2.layerButtonColors.red, 0x7F);
        } else {
            midi.sendShortMsg(status, XoneK2.layerButtonColors.red, 0x00);
        }
    }
};
