var XoneK2 = {};

XoneK2.deckOrder = [ 3, 1, 2, 4 ];
XoneK2.effectsOrder = [ 3, 1, 2, 4 ];

XoneK2.decksMidiChannel = 0xE;
XoneK2.effectsMidiChannel = 0xD;

// The MIDI note offsets for different colors with the layer button is different
// from the rest of the buttons.
XoneK2.deckBottomButtonLayers = [
    { name: 'transport', layerButtonNoteNumber: 0x0C }, // red
    { name: 'loop', layerButtonNoteNumber: 0x10 }, // amber
    { name: 'hotcue', layerButtonNoteNumber: 0x14 } ]; // green

XoneK2.midiChannels = [];
for (var ch = 0; ch <= 0xF; ++ch) {
    XoneK2.midiChannels[ch] = [];
    XoneK2.midiChannels[ch].columns = [];
    XoneK2.midiChannels[ch].isShifted = false;
}

XoneK2.init = function (id) {
    for (var z = 1; z <= 4; z++) {
        XoneK2.midiChannels[XoneK2.effectsMidiChannel].columns[z] =
                new XoneK2.EffectUnit(XoneK2.effectsOrder[z-1], z, XoneK2.effectsMidiChannel);
        XoneK2.midiChannels[XoneK2.decksMidiChannel].columns[z] =
                new XoneK2.Deck(XoneK2.deckOrder[z-1], z, XoneK2.decksMidiChannel);
    }
    XoneK2.decksLayerButton(null, null, null, 0x90 + XoneK2.decksMidiChannel, null);
}

XoneK2.shutdown = function(id) {
    var turnOff = function (component) {
        component.send(0);
    };
    for (var z = 1; z <= 4; z++) {
        XoneK2.midiChannels[XoneK2.effectsMidiChannel].columns[z].forEachComponent(turnOff);
        XoneK2.midiChannels[XoneK2.decksMidiChannel].columns[z].forEachComponent(turnOff);
    }
}


XoneK2.decksBottomLeftEncoderIsPressed = false;
XoneK2.decksBottomLeftEncoderPress = function (channel, control, value, status) {
    XoneK2.decksBottomLeftEncoderIsPressed =  (status & 0xF0) === 0x90;
    if (XoneK2.midiChannels[channel].isShifted && XoneK2.decksBottomLeftEncoderIsPressed) {
        script.toggleControl('[Master]', 'headSplit');
    }
};
XoneK2.decksBottomLeftEncoder = function (channel, control, value, status) {
    if (!XoneK2.midiChannels[channel].isShifted) {
        if (!XoneK2.decksBottomLeftEncoderIsPressed) {
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
    if ((status & 0xF0) === 0x90) {
        engine.setValue("[Playlist]", "LoadSelectedIntoFirstStopped", 1);
    }
};
XoneK2.decksBottomRightEncoder = function (channel, control, value, status) {
    if (!XoneK2.midiChannels[channel].isShifted) {
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
    XoneK2.midiChannels[channel].isShifted = (status & 0xF0) === 0x90;
    if (XoneK2.midiChannels[channel].isShifted) {
        for (var z = 1; z <= 4; z++) {
            XoneK2.midiChannels[channel].columns[z].shift();
        }
        midi.sendShortMsg(status, 0x0F, 0x7F);
    } else {
        for (var z = 1; z <= 4; z++) {
            XoneK2.midiChannels[channel].columns[z].unshift();
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
    // and turned off with a Note Offf MIDI message (first nybble of first byte 8).
    if (value > 0) {
        midi.sendShortMsg(this.midi[0] + 0x10, this.midi[1] + this.color, value);
    } else {
        midi.sendShortMsg(this.midi[0], this.midi[1], 0x7F);
    }
};
components.Button.prototype.isPress = function (channel, control, value, status) {
    return (status & 0xF0) === 0x90;
}

XoneK2.setBottomButtonsMidi = function (bottomButtonsObject, columnNumber, midiChannel) {
    for (var c = 1; c <= 4; c++) {
        bottomButtonsObject[c].midi = [0x80 + midiChannel,
                                       0x24 - (c-1)*4 + (columnNumber-1)];
    }
};

XoneK2.setColumnMidi = function (columnObject, columnNumber, midiChannel) {
    columnObject.encoderPress.midi = [0x80 + midiChannel, 0x34 + (columnNumber-1)];

    for (var b = 1; b <= 3; b++) {
        columnObject.topButtons[b].midi = [0x80 + midiChannel,
                                           0x30 - (b-1)*4 + (columnNumber-1)];
    }

    XoneK2.setBottomButtonsMidi(columnObject.bottomButtons, columnNumber, midiChannel);
};

XoneK2.Deck = function (deckNumber, column, midiChannel) {
    var theDeck = this;

    this.deckString = '[Channel' + deckNumber + ']';

    this.encoder = new components.Encoder({
        input: function (channel, control, value, status) {
            if (value == 127) {
                jogValue = -1;
            } else {
                jogValue = 1;
            }

            if (XoneK2.midiChannels[channel].isShifted) {
                var rate = engine.getValue(this.group, "rate");
                engine.setValue(this.group, "rate", rate + (.005 * jogValue));
            } else {
                if (engine.getValue(this.group, "play") == 1 &&
                    engine.getValue(this.group, "reverse") == 1) {
                    jogValue = -(jogValue);
                }
                engine.setValue(this.group, "jog", jogValue);
            }
        },
    });

    this.encoderPress = new components.Button();
    this.encoderPress.input = function (channel, control, value, status, group) {
        if (this.isPress(channel, control, value, status, group)) {
            engine.setValue(this.group, "rate", 0.0);
        }
    };

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
        key: 'keylock',
        type: components.Button.prototype.types.toggle,
    });
    this.topButtons[2] = new components.Button({
        key: 'quantize',
        type: components.Button.prototype.types.toggle,
    });
    this.topButtons[3] = new components.Button({
        group: '[EqualizerRack1_' + this.deckString + '_Effect1]',
        key: 'button_parameter1',
        type: components.Button.prototype.types.toggle,
    });


    // This should not be a ComponentContainer, otherwise strange things will
    // happen when iterating over the Deck with reconnectComponents.
    this.bottomButtonLayers = [];

    this.bottomButtonLayers.transport = new components.ComponentContainer();
    this.bottomButtonLayers.transport[1] = new components.Button({
        key: 'pfl',
        type: components.Button.prototype.types.toggle,
    });
    this.bottomButtonLayers.transport[2] = new components.SyncButton();
    this.bottomButtonLayers.transport[3] = new components.CueButton();
    this.bottomButtonLayers.transport[4] = new components.PlayButton();
    this.bottomButtonLayers.transport.forEachComponent(function (c) {
        c.color = XoneK2.color.red;
    });

    this.bottomButtonLayers.loop = new components.ComponentContainer();
    this.bottomButtonLayers.loop[1] = new components.Button({
        unshift: function () {
            this.inKey = 'beatloop_activate';
            this.outKey = 'beatloop_activate';
            this.disconnect();
            this.connect();
            this.trigger();
        },
        shift: function () {
            this.inKey = 'beatlooproll_activate';
            this.outKey = 'beatlooproll_activate';
            this.disconnect();
            this.connect();
            this.trigger();
        },
    });

    this.bottomButtonLayers.loop[2] = new components.Button({
        unshift: function () {
            this.inKey = 'loop_halve';
            this.outKey = 'loop_halve';
            this.disconnect();
            this.connect();
            this.trigger();
        },
        shift: function () {
            this.inKey = 'beatjump_backward';
            this.outKey = 'beatjump_backward';
            this.disconnect();
            this.connect();
            this.trigger();
        },
    });

    this.bottomButtonLayers.loop[3] = new components.Button({
        unshift: function () {
            this.inKey = 'loop_double';
            this.outKey = 'loop_double';
            this.disconnect();
            this.connect();
            this.trigger();
        },
        shift: function () {
            this.inKey = 'beatjump_forward';
            this.outKey = 'beatjump_forward';
            this.disconnect();
            this.connect();
            this.trigger();
        },
    });

    this.bottomButtonLayers.loop[4] = new components.Button({
        outKey: 'loop_enabled',
        unshift: function () {
            this.inKey = 'reloop_toggle';
        },
        shift: function () {
            this.inKey = 'reloop_andstop';
        },
    });

    this.bottomButtonLayers.loop.forEachComponent(function (c) {
        c.color = XoneK2.color.amber;
    });

    this.bottomButtonLayers.hotcue = new components.ComponentContainer();
    for (var n = 1; n <= 4; ++n) {
        this.bottomButtonLayers.hotcue[n] = new components.HotcueButton({
            number: n,
            color: XoneK2.color.green,
        });
    }

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

    this.reconnectComponents(setGroup);

    XoneK2.setColumnMidi(this, column, midiChannel);
};
XoneK2.Deck.prototype = new components.ComponentContainer();

// This gets incremented to 0 on startup.
XoneK2.deckLayerIndex = -1;
XoneK2.decksLayerButton = function (channel, control, value, status) {
    if (components.Button.prototype.isPress(channel, control, value, status)) {
        XoneK2.deckLayerIndex++;
        if (XoneK2.deckLayerIndex === XoneK2.deckBottomButtonLayers.length) {
            XoneK2.deckLayerIndex = 0;
        }
        var newLayer = XoneK2.deckBottomButtonLayers[XoneK2.deckLayerIndex];

        for (var x = 1; x <= 4; ++x) {
            var deckColumn = XoneK2.midiChannels[XoneK2.decksMidiChannel].columns[x];
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
};

XoneK2.EffectUnit = function (unitNumber, column, midiChannel) {
    components.EffectUnit.call(this, [unitNumber]);

    this.encoder = new components.Component();
    // TODO: figure out a use for this
    this.encoder.input = function () {};
    this.encoderPress = this.effectFocusButton;

    this.topButtons = [];
    for (var b = 0; b <= 3; b++) {
        this.topButtons[b] = this.enableButtons[b];
    }

    this.fader = this.dryWetKnob;

    this.bottomButtons = [];
    var channelString;
    for (var c = 1; c <= 4; c++) {
        channelString = "Channel" + c;
        this.enableOnChannelButtons.addButton(channelString);
        this.bottomButtons[c] = this.enableOnChannelButtons[channelString];
    }

    XoneK2.setColumnMidi(this, column, midiChannel);
    this.init();
};
XoneK2.EffectUnit.prototype = new components.ComponentContainer();
