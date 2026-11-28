/*
 * The Roland DJ-808 controller has two basic modes:
 *
 * 1. Standalone mode
 *
 * When the DJ-808 is not connected to a computer (or no SysEx/Keep-Alive
 * messages are sent [see below]), the controller is put into standalone mode.
 *
 * In this mode, the controller's LEDs automatically react on button presses.
 * Releasing a button will switches off the button LED, so a MIDI message
 * needs to be send afterwards to switch it on again.
 * The performance pads LEDs also indicate different modes, but the colors are
 * different from the modes described in the Owner's Manual. It does not seem
 * to be possible to illuminate individual pads.
 *
 * he built-in TR-S drum machine works in standalone mode and its output level
 * is controlled by the TR/SAMPLER LEVEL knob.
 *
 *
 * 2. Serato mode
 *
 * When the DJ-808 receives a SysEx message, the controller is put in "Serato"
 * mode.  However, in order to keep the DJ-808 in this mode, it seems to be
 * necessary to regularly send a "keep-alive" MIDI message (0xBF 0x64 0x00).
 * Otherwise the device will switch back to "Standalone mode" after
 * approximately 1.5 seconds.
 *
 * In Serato mode, the all LEDs have to be illuminated by sending MIDI
 * messages: Pressing a button does not switch the LED on and releasing it does
 * not switch it off. The performance pad LEDs can all be set individually
 * (including the mode buttons).
 *
 * The TR-S output is not connected to the main out. Instead, it is connected
 * to one of input channels of the controller's audio interface. Hence, the
 * TR/SAMPLER LEVEL knob does not control the output volume of TR-S, and
 * works as a generic MIDI control instead.
 *
 *
 * Other quirks and issues of the Roland DJ-808:
 * - The controller does not send the current value of the crossfader when it
 *   receives the SysEx message. This also happens when it's used with Serato,
 *   so Mixxx tries to work around the issue by using "soft takeover" to avoid
 *   sudden volume changes when the crossfader is first used.
 * - It does not seem to be possible to toggle the LEDs of the BACK and the ADD
 *   PREPARE buttons. Again, this can be reproduced in Serato, so it looks like
 *   a firmware problem and cannot be worked around.
 *
 */

var DJ808 = {};

/////////////////
// Tweakables. //
/////////////////

DJ808.stripSearchScaling = 0.15;
DJ808.tempoRange = [0.08, 0.16, 0.5];
DJ808.autoShowFourDecks = false;
DJ808.trsGroup = "Auxiliary1";  // TR-S input


///////////
// Code. //
///////////

DJ808.init = function() {
    DJ808.shiftButton = function(channel, control, value, _status, _group) {
        DJ808.deck.concat(DJ808.effectUnit, [DJ808.sampler, DJ808.browseEncoder]).forEach(
            value ? function(module) { module.shift(); } : function(module) { module.unshift(); }
        );
    };

    DJ808.deck = [];
    for (let i = 0; i < 4; i++) {
        DJ808.deck[i] = new DJ808.Deck(i + 1, i);
        DJ808.deck[i].setCurrentDeck("[Channel" + (i + 1) + "]");
        DJ808['loadChannel' + (i + 1) + 'Button'] = new components.Button({
            group: "[Channel" + (i + 1) + "]",
            midi: [0x9F, 0x02 + i],
            unshift: function() {
                this.inKey = "LoadSelectedTrack";
            },
            shift: function() {
                this.inKey = "eject";
            },
            input: function(channel, control, value, status, _group) {
                this.send(engine.getValue("[Channel" + (i + 1) + "]", "track_loaded") ? this.on : this.off);

                components.Button.prototype.input.apply(this, arguments);
            },
        });

        }


    DJ808.deck3Button = new DJ808.DeckToggleButton({
        midi: [0x90, 0x08],
        decks: [1, 3],
        loadChannel1Button: DJ808.loadChannel1Button,
        loadChannel3Button: DJ808.loadChannel3Button
    });

    DJ808.deck4Button = new DJ808.DeckToggleButton({
        midi: [0x91, 0x08],
        decks: [2, 4],
        loadChannel2Button: DJ808.loadChannel2Button,
        loadChannel4Button: DJ808.loadChannel4Button
    });

    DJ808.sampler = new DJ808.Sampler();

    DJ808.effectUnit = [];

    var enableButtonOnFocusChange = function(value, _group, _control) {
        this.group = "[EffectRack1_EffectUnit" +
                        this.effectUnit.currentUnitNumber + "_Effect" +
                        this.number + "]";
        this.inKey = "enabled";
        this.outKey = "enabled";
    };
    var knobsOnfocusChange = function(value, _group, _control) {
            this.group = "[EffectRack1_EffectUnit" +
                            this.effectUnit.currentUnitNumber + "_Effect" +
                            this.number + "]";
            this.inKey = "meta";
            this.effectUnit.dryWetKnob.unshift();
            engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
    };

    var knobsUnshift = function() {
        this.inKey = "meta";
        this.input = function(channel, control, value, _status, _group) {
            if (engine.getValue(this.effectUnit.group, "focused_effect") != this.number) {
                engine.setValue(this.effectUnit.group, "focused_effect", this.number);
                this.effectUnit.focusParameter = engine.getValue("[EffectRack1_EffectUnit" + this.effectUnit.currentUnitNumber +
                            "_Effect" + this.number + "]", "num_parameters") > 0 ? 1 : 0;
                this.effectUnit.focusButtonParameter =  engine.getValue("[EffectRack1_EffectUnit" + this.effectUnit.currentUnitNumber +
                            "_Effect" + this.number + "]", "num_button_parameters") > 0 ? 1 : 0;
            } 
            if (this.MSB !== undefined) {
                value = (this.MSB << 7) + value;
            }
            this.inSetParameter(this.inValueScale(value));
        };
    };

    for (let i = 0; i < 2; i++) {
        DJ808.effectUnit[i] = new components.EffectUnit([i + 1, 3]);
        DJ808.effectUnit[i].effectUnitNumber = i + 1;
        engine.setValue(this.effectUnit.group, "show_focus",1);
        engine.setValue(DJ808.effectUnit[i].group, "focused_effect",1);
        engine.setValue(this.effectUnit, "show_parameters", 1);
        DJ808.effectUnit[i].focusParameter = engine.getValue("[EffectRack1_EffectUnit" + DJ808.effectUnit[i].currentUnitNumber +
                    "_Effect1]", "num_parameters") > 0 ? 1 : 0;
        DJ808.effectUnit[i].focusButtonParameter =  engine.getValue("[EffectRack1_EffectUnit" + DJ808.effectUnit[i].currentUnitNumber +
                    "_Effect1]", "num_button_parameters") > 0 ? 1 : 0;
        DJ808.effectUnit[i].sendShifted = true;
        DJ808.effectUnit[i].shiftOffset = 0x0B;
        DJ808.effectUnit[i].shiftControl = true;
        DJ808.effectUnit[i].enableButtons[1].midi = [0x98 + i, 0x00];
        DJ808.effectUnit[i].enableButtons[1].effectUnit = DJ808.effectUnit[i];
        DJ808.effectUnit[i].enableButtons[1].onFocusChange = enableButtonOnFocusChange;

        DJ808.effectUnit[i].enableButtons[2].midi = [0x98 + i, 0x01];
        DJ808.effectUnit[i].enableButtons[2].effectUnit = DJ808.effectUnit[i];
        DJ808.effectUnit[i].enableButtons[2].onFocusChange = enableButtonOnFocusChange;
        DJ808.effectUnit[i].enableButtons[3].midi = [0x98 + i, 0x02];
        DJ808.effectUnit[i].enableButtons[3].effectUnit = DJ808.effectUnit[i];
        DJ808.effectUnit[i].enableButtons[3].onFocusChange = enableButtonOnFocusChange;
        DJ808.effectUnit[i].effectFocusButton.midi = [0x98 + i, 0x04];
        DJ808.effectUnit[i].effectFocusButton.group = "[EffectRack1_EffectUnit" + (i + 1) + "_Effect" + engine.getValue(DJ808.effectUnit[i].group, "focused_effect") + "]";
        DJ808.effectUnit[i].effectFocusButton.inKey =  "button_parameter" + DJ808.effectUnit[i].focusButtonParameter;
        DJ808.effectUnit[i].effectFocusButton.outKey = "button_parameter" + DJ808.effectUnit[i].focusButtonParameter;
        DJ808.effectUnit[i].effectFocusButton.pressCount = 0;
        DJ808.effectUnit[i].effectFocusButton.effectUnit = DJ808.effectUnit[i]; 
        DJ808.effectUnit[i].effectFocusButton.shiftOffset = 0x06;
        DJ808.effectUnit[i].effectFocusButton.input = function(channel, control, value, status, _group) {
            if (value == 127) {
                engine.setValue(this.group, "button_parameter" + this.effectUnit.focusButtonParameter, engine.getValue(this.group, "button_parameter" + this.effectUnit.focusButtonParameter) === 0 ? 1 : 0);
                this.setColor();
            }
        };

        DJ808.effectUnit[i].effectFocusButton.setColor = function() {
            if (engine.getValue(this.group, "button_parameter" + this.effectUnit.focusButtonParameter) === 0) {
                this.send(this.off);
            } else {
                this.send(this.on);
            }
        };
        DJ808.effectUnit[i].effectFocusButton.unshift = function() {
            this.inKey =  "button_parameter" + this.effectUnit.focusParameter;
            this.outKey = "button_parameter" + this.effectUnit.focusParameter;
            this.input = function(channel, control, value, status, _group) {
                if (value == 127) {
                    engine.setValue(this.group, "button_parameter" + this.effectUnit.focusButtonParameter, engine.getValue(this.group, "button_parameter" + this.effectUnit.focusButtonParameter) === 0 ? 1 : 0);
                    this.setColor();
                }
            };
        };
        DJ808.effectUnit[i].effectFocusButton.shift = function() {
            this.input = function(channel, control, value, status, _group) {
                if (value == 127) {
                    if (this.pressCount == 0) {
                        engine.beginTimer(200, () => {
                        if (this.effectUnit.focusButtonParameter > 0) {
                            if (this.pressCount > 1) {
                                if (this.effectUnit.focusButtonParameter == 1) {
                                    this.effectUnit.focusButtonParameter = engine.getValue(this.group, "num_button_parameters"); 
                                } 
                                else {
                                    this.effectUnit.focusButtonParameter--;
                                }
                            }
                            else {
                                if (this.effectUnit.focusButtonParameter == engine.getValue(this.group, "num_button_parameters")) {
                                    this.effectUnit.focusButtonParameter = 1; 
                                } 
                                else {
                                    this.effectUnit.focusButtonParameter++;
                                }
                            }
                            this.inKey =  "button_parameter" + this.effectUnit.focusButtonParameter;
                            this.outKey = "button_parameter" + this.effectUnit.focusButtonParameter;
                            this.setColor();
                        }
                        this.pressCount = 0;
                        }, true);
                    }
                    this.pressCount++;
                }
            };
        };

        DJ808.effectUnit[i].knobs[1].midi = [0xB8 + i, 0x00];
        DJ808.effectUnit[i].knobs[1].effectUnit = DJ808.effectUnit[i];
        DJ808.effectUnit[i].knobs[1].onFocusChange = knobsOnfocusChange;
        DJ808.effectUnit[i].knobs[1].unshift = knobsUnshift; 
        DJ808.effectUnit[i].knobs[2].midi = [0xB8 + i, 0x01];
        DJ808.effectUnit[i].knobs[2].effectUnit = DJ808.effectUnit[i];
        DJ808.effectUnit[i].knobs[2].unshift = knobsUnshift;
        DJ808.effectUnit[i].knobs[2].onFocusChange = knobsOnfocusChange;

        DJ808.effectUnit[i].knobs[3].midi = [0xB8 + i, 0x02];
        DJ808.effectUnit[i].knobs[3].effectUnit = DJ808.effectUnit[i];
        DJ808.effectUnit[i].knobs[3].unshift = knobsUnshift; 
        DJ808.effectUnit[i].knobs[3].onFocusChange = knobsOnfocusChange;

        DJ808.effectUnit[i].dryWetKnob.midi = [0xB8 + i, 0x03];
        DJ808.effectUnit[i].dryWetKnob.effectUnit = DJ808.effectUnit[i];
        DJ808.effectUnit[i].dryWetKnob.shift = function() {
            this.group = "[EffectRack1_EffectUnit" + this.effectUnit.effectUnitNumber + "]";
            this.inKey = "mix";
            // for soft takeover
            this.disconnect();
            this.connect();
        };
        DJ808.effectUnit[i].dryWetKnob.unshift = function() {
            this.group = "[EffectRack1_EffectUnit" + this.effectUnit.effectUnitNumber + "_Effect" + engine.getValue(this.effectUnit.group, "focused_effect") + "]";
            this.inKey = "parameter" + this.effectUnit.focusParameter;
            // for soft takeover
            this.disconnect();
            this.connect();
        };
        DJ808.effectUnit[i].dryWetKnob.unshift();
        DJ808.effectUnit[i].dryWetKnob.pressCount = 0;
        DJ808.effectUnit[i].dryWetKnob.input = function(channel, control, value, _status, _group) {
            if ([152,153].indexOf(_status) > -1 ) {
                if (value == 127) {
                    if (this.pressCount == 0) {
                        engine.beginTimer(200, () => {
                        if (this.effectUnit.focusParameter > 0) {
                            if (this.pressCount > 1) {
                                if (this.effectUnit.focusParameter == 1) {
                                    this.effectUnit.focusParameter = engine.getValue(this.group, "num_parameters"); 
                                } 
                                else {
                                    this.effectUnit.focusParameter--;
                                }
                            }
                            else {
                                if (this.effectUnit.focusParameter == engine.getValue(this.group, "num_parameters")) {
                                    this.effectUnit.focusParameter = 1; 
                                } 
                                else {
                                    this.effectUnit.focusParameter++;
                                }
                            }
                            this.inKey = this.inKey = "parameter" + this.effectUnit.focusParameter;
                        }
                        this.pressCount = 0;
                        }, true);
                    }
                    this.pressCount++;
                }
            }
            else {
                if (value === 1) {
                    // 0.05 is an example. Adjust that value to whatever works well for your controller.
                    this.inSetParameter(this.inGetParameter() + 0.1);
                } else {
                    this.inSetParameter(this.inGetParameter() - 0.1);
                }
            }
        };
        for (let j = 1; j <= 4; j++) {
            DJ808.effectUnit[i].enableOnChannelButtons.addButton("Channel" + j);
            DJ808.effectUnit[i].enableOnChannelButtons["Channel" + j].midi = [0x98 + i, 0x04 + j];
        }
        DJ808.effectUnit[i].enableOnChannelButtons.addButton(DJ808.trsGroup);
        DJ808.effectUnit[i].enableOnChannelButtons[DJ808.trsGroup].midi = [0x98 + i, 0x09];
        DJ808.effectUnit[i].enableOnChannelButtons[DJ808.trsGroup].input = function(channel, control, value, status, _group) {
            components.Button.prototype.input.apply(this, arguments);
            if (this.isPress(channel, control, value, status)) {
                const enabled = this.inGetValue();
                for (let j = 1; j <= 16; j++) {
                    engine.setValue(this.group, "group_[Sampler" + j + "]_enable", enabled);
                }
            }
        };
        DJ808.effectUnit[i].enableOnTrsButton = DJ808.effectUnit[i].enableOnChannelButtons[DJ808.trsGroup];
        DJ808.effectUnit[i].init();
    }

    engine.makeConnection("[Channel3]", "track_loaded", DJ808.autoShowDecks);
    engine.makeConnection("[Channel4]", "track_loaded", DJ808.autoShowDecks);

    if (engine.getValue("[App]", "num_samplers") < 16) {
        engine.setValue("[App]", "num_samplers", 16);
    }

    // Send Serato SysEx messages to request initial state and unlock pads
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x00, 0xF7], 6);
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x01, 0xF7], 6);

    // Send "keep-alive" message to keep controller in Serato mode
    engine.beginTimer(500, () => {
        midi.sendShortMsg(0xBF, 0x64, 0x00);
    });

    // Reset LEDs
    DJ808.deck3Button.trigger();
    DJ808.deck4Button.trigger();
    for (let i = 0; i < 4; i++) {
        DJ808.deck[i].reconnectComponents();
    }
};

DJ808.autoShowDecks = function(_value, _group, _control) {
    const anyLoaded = engine.getValue("[Channel3]", "track_loaded") || engine.getValue("[Channel4]", "track_loaded");
    if (!DJ808.autoShowFourDecks) {
        return;
    }
    engine.setValue("[Master]", "show_4decks", anyLoaded);
};

DJ808.shutdown = function() {
};


DJ808.browseEncoder = new components.Encoder({
    longPressTimer: 0,
    longPressTimeout: 250,
    trackColorCycleEnabled: false,
    trackColorCycleHappened: false,
    previewSeekEnabled: false,
    previewSeekHappened: false,
    unshift: function() {
        this.onKnobEvent = function(rotateValue) {
            if (rotateValue !== 0) {
                if (this.previewSeekEnabled) {
                    const oldPos = engine.getValue("[PreviewDeck1]", "playposition");
                    const newPos = Math.max(0, oldPos + (0.05 * rotateValue));
                    engine.setValue("[PreviewDeck1]", "playposition", newPos);
                } else {
                    engine.setValue("[Playlist]", "SelectTrackKnob", rotateValue);
                }
            }
        };
        this.onButtonEvent = function(value) {
            if (value) {
                this.isLongPressed = false;
                this.longPressTimer = engine.beginTimer(
                    this.longPressTimeout,
                    () => { this.isLongPressed = true; },
                    true
                );

                this.previewStarted = false;
                if (!engine.getValue("[PreviewDeck1]", "play")) {
                    engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 1);
                    this.previewStarted = true;
                }
                // Track in PreviewDeck1 is playing, either the user
                // wants to stop the track or seek in it
                this.previewSeekEnabled = true;
            } else {
                if (this.longPressTimer !== 0) {
                    engine.stopTimer(this.longPressTimer);
                    this.longPressTimer = 0;
                }

                if (!this.isLongPressed && !this.previewStarted && engine.getValue("[PreviewDeck1]", "play")) {
                    script.triggerControl("[PreviewDeck1]", "stop");
                }
                this.previewSeekEnabled = false;
                this.previewStarted = false;
            }
        };
    },
    shift: function() {
        this.onKnobEvent = function(rotateValue) {
            if (rotateValue !== 0) {
                if (this.trackColorCycleEnabled) {
                    const key = (rotateValue > 0) ? "track_color_next" : "track_color_prev";
                    engine.setValue("[Library]", key, 1.0);
                    this.trackColorCycleHappened = true;
                } else {
                    engine.setValue("[Playlist]", "SelectPlaylist", rotateValue);
                }
            }
        };
        this.onButtonEvent = function(value) {
            if (value) {
                this.trackColorCycleEnabled = true;
                this.trackColorCycleHappened = false;
            } else {
                if (!this.trackColorCycleHappened) {
                    script.triggerControl("[Playlist]", "ToggleSelectedSidebarItem");
                }
                this.trackColorCycleEnabled = false;
                this.trackColorCycleHappened = false;
            }
        };
    },
    input: function(channel, control, value, status, _group) {
        switch (status) {
        case 0xBF: { // Rotate.
            const rotateValue = (value === 127) ? -1 : ((value === 1) ? 1 : 0);
            this.onKnobEvent(rotateValue);
            break;
        }
        case 0x9F: // Push.
            this.onButtonEvent(value);
        }
    }
});

DJ808.backButton = new components.Button({
    // TODO: Map the BACK button
    midi: [0x9F, 0x07],
    shiftOffset: 11,
    sendShifted: true,
    shiftControl: true,
    type: undefined,
});

DJ808.addPrepareButton = new components.Button({
    midi: [0x9F, 0x1B],
    shiftOffset: -7,
    sendShifted: true,
    shiftControl: true,
    group: "[Skin]",
    key: "show_maximized_library",
    type: components.Button.prototype.types.toggle,
});


DJ808.sortLibrary = function(channel, control, value, _status, _group) {
    if (value === 0) {
        return;
    }

    let sortColumn;
    switch (control) {
    case 0x12:  // SONG
        sortColumn = 2;
        break;
    case 0x13:  // BPM
        sortColumn = 15;
        break;
    case 0x14:  // ARTIST
        sortColumn = 1;
        break;
    case 0x1E:  // KEY
        sortColumn = 20;
        break;
    default:
        // unknown sort column
        return;
    }
    engine.setValue("[Library]", "sort_column_toggle", sortColumn);
};

DJ808.crossfader = new components.Pot({
    midi: [0xBF, 0x08],
    group: "[Master]",
    inKey: "crossfader",
    input: function() {
        // We need a weird max. for the crossfader to make it cut cleanly.
        // However, components.js resets max. to 0x3fff when the first value is
        // received. Hence, we need to set max. here instead of within the
        // constructor.
        this.max = (0x7f<<7) + 0x70;
        components.Pot.prototype.input.apply(this, arguments);
    }
});
DJ808.crossfader.setCurve = function(channel, control, value, _status, _group) {
    // 0x00 is Picnic Bench, 0x01 is Constant Power and 0x02 is Linear
    switch (value) {
    case 0x00:  // Picnic Bench / Fast Cut
        engine.setValue("[Mixer Profile]", "xFaderMode", 0);
        engine.setValue("[Mixer Profile]", "xFaderCalibration", 0.9);
        engine.setValue("[Mixer Profile]", "xFaderCurve", 7.0);
        break;
    case 0x01:  // Constant Power
        engine.setValue("[Mixer Profile]", "xFaderMode", 1);
        engine.setValue("[Mixer Profile]", "xFaderCalibration", 0.3);
        engine.setValue("[Mixer Profile]", "xFaderCurve", 0.6);
        break;
    case 0x02: // Additive
        engine.setValue("[Mixer Profile]", "xFaderMode", 0);
        engine.setValue("[Mixer Profile]", "xFaderCalibration", 0.4);
        engine.setValue("[Mixer Profile]", "xFaderCurve", 0.9);
    }
};

DJ808.crossfader.setReverse = function(channel, control, value, _status, _group) {
    // 0x00 is ON, 0x01 is OFF
    engine.setValue("[Mixer Profile]", "xFaderReverse", (value === 0x00) ? 1 : 0);
};

DJ808.setChannelInput = function(channel, control, value, _status, _group) {
    const number = (channel === 0x00) ? 0 : 1;
    const channelgroup = "[Channel" + (number + 1) + "]";
    switch (value) {
    case 0x00:  // PC
        engine.setValue(channelgroup, "passthrough", 0);
        break;
    case 0x01:  // LINE
    case 0x02:  // PHONO
        engine.setValue(channelgroup, "passthrough", 1);
        break;
    }
};

DJ808.Deck = function(deckNumbers, offset) {
    components.Deck.call(this, deckNumbers);

    this.slipModeButton = new DJ808.SlipModeButton({
        midi: [0x90 + offset, 0xF],
        shiftOffset: -8,
        shiftControl: true,
        sendShifted: true,
    });

    engine.setValue(this.currentDeck, "rate_dir", -1);
    this.tempoFader = new components.Pot({
        group: "[Channel" + deckNumbers + "]",
        midi: [0xB0 + offset, 0x09],
        invert: true,
        connect: function() {
            engine.softTakeover(this.group, "pitch", true);
            engine.softTakeover(this.group, "rate", true);
            components.Pot.prototype.connect.apply(this, arguments);
        },
        unshift: function() {
            this.inKey = "rate";
            this.inSetParameter = components.Pot.prototype.inSetParameter;
            engine.softTakeoverIgnoreNextValue(this.group, "pitch");
        },
        shift: function() {
            this.inKey = "pitch";
            this.inSetParameter = function(value) {
                // Scale to interval ]-7…7[; invert direction as per controller
                // labeling.
                value = 14 * value - 7;
                value *= -1;
                components.Pot.prototype.inSetValue.call(this, value);
            };
            engine.softTakeoverIgnoreNextValue(this.group, "rate");
        }
    });


    // ============================= JOG WHEELS =================================
    this.wheelTouch = function(channel, control, value, _status, _group) {
        if (value === 0x7F && !this.isShifted) {
            const alpha = 1.0/8;
            const beta = alpha/32;
            engine.scratchEnable(script.deckFromGroup(this.currentDeck), 512, 45, alpha, beta);
        } else {    // If button up
            engine.scratchDisable(script.deckFromGroup(this.currentDeck));
        }
    };

    this.wheelTurn = function(channel, control, value, _status, _group) {
        // When the jog wheel is turned in clockwise direction, value is
        // greater than 64 (= 0x40). If it's turned in counter-clockwise
        // direction, the value is smaller than 64.
        const newValue = value - 64;
        const deck = script.deckFromGroup(this.currentDeck);
        if (engine.isScratching(deck)) {
            engine.scratchTick(deck, newValue); // Scratch!
        } else if (this.isShifted) {
            const oldPos = engine.getValue(this.currentDeck, "playposition");
            // Since ‘playposition’ is normalized to unity, we need to scale by
            // song duration in order for the jog wheel to cover the same amount
            // of time given a constant turning angle.
            const duration = engine.getValue(this.currentDeck, "duration");
            const newPos = Math.max(0, oldPos + (newValue * DJ808.stripSearchScaling / duration));
            engine.setValue(this.currentDeck, "playposition", newPos); // Strip search
        } else {
            engine.setValue(this.currentDeck, "jog", newValue/2); // Pitch bend
        }
    };

    /* Platter Spin LED Indicator on Jog Wheels
     *
     * The Controller features a LED indicator that imitates a spinning
     * platter when the deck is playing and also shows to position inside a
     * bar.
     *
     * LED indicator values:
     * - 0x00 - 0x1F: Beat 0 (downbeat) to 1
     * - 0x20 - 0x3F: Beat 1 to 2
     * - 0x40 - 0x5F: Beat 2 to 3
     * - 0x60 - 0x7F: Beat 3 (upbeat) to 0 (next downbeat)
     *
     * TODO: Add proper bar support for the LED indicators
     *
     * Mixxx currently does not support bar detection, so we don't know which
     * of the 4 beats in a we are on. This has been worked around by counting
     * beats manually, but this is error prone and does not support moving
     * backwards in a track, has problems with loops, does not detect hotcue
     * jumps and does not indicate the downbeat (obviously).
     *
     * See Launchpad issue: https://github.com/mixxxdj/mixxx/issues/5218
     */
    this.beatIndex = 0;
    this.lastBeatDistance = 0;
    engine.makeConnection(this.currentDeck, "beat_distance", function(value) {
        // Check if we're already in front of the next beat.

        // Since deck indices start with 1, we use 0xAF + deck for the status
        // byte, so that we 0xB0 for the first deck.
        const status = 0xAF + script.deckFromGroup(this.currentDeck);

        // Send a value between 0x00 and 0x7F to set jog wheel LED indicator 
        midi.sendShortMsg(status, 0x07, (engine.getValue(this.currentDeck, "playposition") * engine.getValue(this.currentDeck, "duration") * 20) % 72);
    }.bind(this));

    // ========================== LOOP SECTION ==============================

    this.loopActive = new components.Button({
        midi: [0x94 + offset, 0x32],
        key: "loop_enabled",
        type: components.Button.prototype.types.toggle,
    });
    this.reloopExit = new components.Button({
        midi: [0x94 + offset, 0x33],
        key: "reloop_exit",
    });
    this.loopHalve = new components.Button({
        midi: [0x94 + offset, 0x34],
        key: "loop_halve",
    });
    this.loopDouble = new components.Button({
        midi: [0x94 + offset, 0x35],
        key: "loop_double",
    });
    this.loopShiftBackward = new components.Button({
        midi: [0x94 + offset, 0x36],
        key: "beatjump_backward",
    });
    this.loopShiftForward = new components.Button({
        midi: [0x94 + offset, 0x37],
        key: "beatjump_forward",
    });
    this.loopIn = new components.Button({
        midi: [0x94 + offset, 0x38],
        key: "loop_in",
    });
    this.loopOut = new components.Button({
        midi: [0x94 + offset, 0x39],
        key: "loop_out",
    });
    this.slotSelect = new components.Button({
        midi: [0x94 + offset, 0x3B],
        key: "quantize",
        type: components.Button.prototype.types.toggle,
    });
    this.autoLoop = new components.Button({
        midi: [0x94 + offset, 0x40],
        inKey: "beatloop_activate",
        outKey: "loop_enabled",
    });


    // ========================== PERFORMANCE PADS ==============================

    this.padSection = new DJ808.PadSection(this, offset);
    this.keylock = new components.Button({
        midi: [0x90 + offset, 0x0D],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 1,
        outKey: "keylock",
        currentRangeIndex: (DJ808.tempoRange.indexOf(engine.getValue("[Channel" + deckNumbers + "]", "rateRange"))) ? DJ808.tempoRange.indexOf(engine.getValue("[Channel" + deckNumbers + "]", "rateRange")) : 0,
        unshift: function() {
            this.inKey = "keylock";
            this.input = components.Button.prototype.input;
            this.type = components.Button.prototype.types.toggle;
        },
        shift: function() {
            this.inKey = "rateRange";
            this.type = undefined;
            this.input = function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status)) {
                    this.currentRangeIndex++;
                    if (this.currentRangeIndex >= DJ808.tempoRange.length) {
                        this.currentRangeIndex = 0;
                    }
                    this.inSetValue(DJ808.tempoRange[this.currentRangeIndex]);
                }
            };
        },
    });

    // ============================= TRANSPORT ==================================

    this.cue = new components.CueButton({
        midi: [0x90 + offset, 0x01],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 4,
        reverseRollOnShift: false,
        input: function(channel, control, value, status, group) {
            components.CueButton.prototype.input.call(this, channel, control, value, status, group);
            if (value) {
                return;
            }
            const state = engine.getValue(group, "cue_indicator");
            if (state) {
                this.trigger();
            }
        }
    });

    this.play = new components.PlayButton({
        midi: [0x90 + offset, 0x00],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 4,
    });

    this.sync = new components.Button({
        midi: [0x90 + offset, 0x02],
        group: "[Channel" + deckNumbers + "]",
        outKey: "sync_mode",
        output: function(value, group, control) {
            if (this.connections[1] !== undefined) {
                this.connections[1].disconnect();
                delete this.connections[1];
            }

            // If the new sync_mode is "Explicit Leader", use the blinking
            // indicator for the LED instead.
            if (value === 3) {
                if (this.connections[1] === undefined) {
                    this.connections[1] = engine.makeConnection("[App]", "indicator_500ms", this.setLed.bind(this));
                }
                return;
            }

            this.setLed(value, group, control);
        },
        setLed: function(value, _group, _control) {
            midi.sendShortMsg(this.midi[0], value ? 0x02 : 0x03, this.on);
        },
        input: function(channel, control, value, _status, _group) {
            if (value) {
                this.longPressTimer = engine.beginTimer(this.longPressTimeout, () => {
                    this.onLongPress();
                    this.longPressTimer = 0;
                }, true);
            } else if (this.longPressTimer !== 0) {
                // Button released after short press
                engine.stopTimer(this.longPressTimer);
                this.longPressTimer = 0;
                this.onShortPress();
            }
        },
        unshift: function() {
            this.onShortPress = function() {
                engine.setValue(this.group, "sync_enabled", 0);
            };
            this.onLongPress = function() {
                if (engine.getValue(this.group, "sync_enabled")) {
                    // If already explicit leader, reset explicit state
                    // (setting it to 0 may still make it implicit leader and
                    // immediately resetting it to 1).
                    const value = (engine.getValue(this.group, "sync_leader") === 2) ? 0 : 2;
                    engine.setValue(this.group, "sync_leader", value);
                } else {
                    engine.setValue(this.group, "sync_enabled", 1);
                }
            };
        },
        shift: function() {
            this.onShortPress = function() {
                script.triggerControl(this.group, "beatsync", 1);
            };
            this.onLongPress = function() {
                script.toggleControl(this.group, "quantize");
            };
        },
    });

    // =============================== MIXER ====================================
    this.pregain = new components.Pot({
        midi: [0xB0 + offset, 0x16],
        group: "[Channel" + deckNumbers + "]",
        inKey: "pregain",
    });

    this.eqKnob = [];
    for (let k = 1; k <= 3; k++) {
        this.eqKnob[k] = new components.Pot({
            midi: [0xB0 + offset, 0x20 - k],
            group: "[EqualizerRack1_" + this.currentDeck + "_Effect1]",
            inKey: "parameter" + k,
        });
    }

    this.quickEffect = new components.Pot({
        midi: [0xB0 + offset, 0x1A],
        group: "[QuickEffectRack1_" + this.currentDeck + "]",
        inKey: "super1",
        currentDeck: this.currentDeck,
        unshift: function () {
            this.input = function(channel, control, value, _status, _group) {
                if (this.MSB !== undefined) {
                    value = (this.MSB << 7) + value;
                }
                let newValue = this.inValueScale(value);
                if (this.invert) {
                    newValue = 1 - newValue;
                }
                this.inSetParameter(newValue);
                if (!this.firstValueReceived) {
                    this.firstValueReceived = true;
                    this.connect();
                }
            };

        },
        shift: function () {
            engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
            this.valueAtLastEffectSwitch = this.previousValueReceived;
            // Floor the threshold to ensure that every effect can be selected
            this.changeThreshold = Math.floor(this.max /
                engine.getValue("[QuickEffectRack1_" + this.currentDeck + "]", "num_chain_presets"));
            this.input = function(channel, control, value, _status, _group) {
                if (this.MSB !== undefined) {
                    value = (this.MSB << 7) + value;
                }

                // Prevent attempt to set the effect_selector CO to NaN
                if (this.valueAtLastEffectSwitch === undefined) {
                    this.valueAtLastEffectSwitch = value;
                    this.previousValueReceived = value;
                    return;
                }

                const change = value - this.valueAtLastEffectSwitch;
                if (Math.abs(change) >= this.changeThreshold
                    // this.valueAtLastEffectSwitch can be undefined if
                    // shift was pressed before the first MIDI value was received.
                    || this.valueAtLastEffectSwitch === undefined) {
                    engine.setValue(this.group, "chain_selector", change);
                    this.valueAtLastEffectSwitch = value;
                }

                this.previousValueReceived = value;
            };

        }
    });

    this.pfl = new components.Button({
        midi: [0x90 + offset, 0x1B],
        group: "[Channel" + deckNumbers + "]",
        type: components.Button.prototype.types.push,
        inKey: "pfl",
        outKey: "pfl",
    });

    this.tapBPM = new components.Button({
        midi: [0x90 + offset, 0x12],
        group: "[Channel" + deckNumbers + "]",
        input: function(_channel, _control, value, _status, group) {
            if (value) {
                this.longPressTimer = engine.beginTimer(this.longPressTimeout, () => {
                    this.onLongPress(group);
                    this.longPressTimer = 0;
                }, true);
            } else if (this.longPressTimer !== 0) {
                // Button released after short press
                engine.stopTimer(this.longPressTimer);
                this.longPressTimer = 0;
                this.onShortPress(group);
            }
        },
        onShortPress: function(group) {
            script.triggerControl(group, "beats_translate_curpos");
        },
        onLongPress: function(group) {
            script.triggerControl(group, "beats_translate_match_alignment");
        },
    });

    this.volume = new components.Pot({
        midi: [0xB0 + offset, 0x1C],
        group: "[Channel" + deckNumbers + "]",
        inKey: "volume",
    });

    this.vuMeter = new components.Component({
        midi: [0xB0 + offset, 0x1F],
        group: "[Channel" + deckNumbers + "]",
        outKey: "vu_meter",
        output: function(value, group, _control) {
            // The red LEDs light up with MIDI values greater than 0x24. The
            // maximum brightness is reached at value 0x28. Red LEDs should
            // only be illuminated if the track is clipping.
            if (engine.getValue(group, "peak_indicator") === 1) {
                value = 0x10;
            } else {
                value = Math.round(value * 0x08);
            }
            this.send(value);
        },
    });
};

DJ808.Deck.prototype = Object.create(components.Deck.prototype);


DJ808.DeckToggleButton = function(options) {
    this.secondaryDeck = false;
    components.Button.call(this, options);
};
DJ808.DeckToggleButton.prototype = Object.create(components.Button.prototype);
DJ808.DeckToggleButton.prototype.input = function(channel, control, value, status, _group) {
    if (this.isPress(channel, control, value, status)) {
        // Button was pressed
        this.longPressTimer = engine.beginTimer(
            this.longPressTimeout,
            () => { this.isLongPressed = true; },
            true
        );
        this.secondaryDeck = !this.secondaryDeck;
    } else if (this.isLongPressed) {
        // Button was released after long press
        this.isLongPressed = false;
        this.secondaryDeck = !this.secondaryDeck;
    } else {
        // Button was released after short press
        engine.stopTimer(this.longPressTimer);
        this.longPressTimer = null;
        return;
    }

    this.trigger();
};
DJ808.DeckToggleButton.prototype.trigger = function() {
    this.send(this.secondaryDeck ? this.on : this.off);
    var deck = (this.secondaryDeck ? this.decks[1] : this.decks[0]).toString();
    const newGroup = "[Channel" + deck + "]";
    var that = this;
    switch (deck) {
        case '1':
        that.loadChannel1Button.disconnect();
        that.loadChannel1Button.connect();
        that.loadChannel1Button.trigger();
        break;
        case '2':
        that.loadChannel2Button.disconnect();
        that.loadChannel2Button.connect();
        that.loadChannel2Button.trigger();
        break;
        case '3':
        that.loadChannel3Button.disconnect();
        that.loadChannel3Button.connect();
        that.loadChannel3Button.trigger();
        break;
        case '4':
        that.loadChannel4Button.disconnect();
        that.loadChannel4Button.connect();
        that.loadChannel4Button.trigger();
        break;

    }
};


//////////////////////////////
// TR/Sampler.              //
//////////////////////////////

DJ808.Sampler = function() {
    // TODO: Improve phase sync (workaround: use NUDGE button for beatmatching)
    /*
     * The TR-S section behaves differently depending on whether the controller
     * is in Standalone or Serato mode:
     *
     * 1. Standalone mode
     *
     * When the controller is in standalone mode, the controller's TR-S works
     * with the SERATO SAMPLER and SYNC functionality disabled. Also, it's not
     * possible to apply FX to the TR-S output signal. The TR/SAMPLER LEVEL
     * knob can be used to adjust the volume of the output.
     *
     *
     * 2. Serato mode
     *
     * In this mode, the BPM can be set by sending MIDI clock
     * messages (0xF8). The sampler can be started by sending one MIDI
     * message per bar (0xBA 0x02 XX). The TR-S is not directly connected to
     * the main out. Instead, the sound is played on channels 7-8 so that the
     * signal can be routed through the FX section.
     *
     * The SERATO SAMPLER features 8 instruments (S1 - S8) that can be to play
     * samples from Serato's sampler banks. If the device is in Serato mode and
     * the sampler instruments are programmed using the TR-S pads, two MIDI
     * messages are sent by the DJ-808 when the sampler step is reached:
     *
     *   9F 2X YY
     *   8F 2X 00
     *
     *  X is the Sampler number, i. e. for S1 this means that X=1. YY is either
     *  0x57 (for steps 1, 5, 9, 13, 16) or 0x50 (all other steps).
     *
     * When the TR-S starts playback, the MIDI start message (0xFA) is sent by the
     * device. When the playback stops, the device sends the stop message (0xFC).
     *
     * In both modes, it is possible to set the BPM in the range 5.0 to 800.0
     * BPM by sending this MIDI message:
     *
     *   sendShortMsg(0xEA, Math.round(bpm*10) & 0x7f, (Math.round(bpm*10) >> 7) & 0x7f);
     *
     */
    components.ComponentContainer.call(this);
    this.syncDeck = -1;

    const getActiveDeck = function() {
        const deckvolume = new Array(0, 0, 0, 0);
        let volumemax = -1;
        let newdeck = -1;

        // get volume from the decks and check it for use
        for (let z = 0; z <= 3; z++) {
            if (engine.getValue("[Channel" + (z + 1) + "]", "track_loaded") > 0) {
                deckvolume[z] = engine.getValue("[Channel" + (z + 1) + "]", "volume");
                if (deckvolume[z] > volumemax) {
                    volumemax = deckvolume[z];
                    newdeck = z;
                }
            }
        }

        return newdeck;
    };

    this.syncButtonPressed = function(channel, control, value, _status, _group) {
        if (value !== 0x7f) {
            return;
        }
        const isShifted = (control === 0x55);
        if (isShifted || this.syncDeck >= 0) {
            this.syncDeck = -1;
        } else {
            const deck = getActiveDeck();
            if (deck < 0) {
                return;
            }
            const bpm = engine.getValue("[Channel" + (deck + 1) + "]", "bpm");

            // Minimum BPM is 5.0 (0xEA 0x32 0x00), maximum BPM is 800.0 (0xEA 0x40 0x3e).
            if (!(bpm >= 5 && bpm <= 800)) {
                return;
            }
            const bpmValue = Math.round(bpm*10);
            midi.sendShortMsg(0xEA, bpmValue & 0x7f, (bpmValue >> 7) & 0x7f);
            this.syncDeck = deck;
        }
    };

    this.bpmKnobTurned = function(channel, control, value, _status, _group) {
        if (this.syncDeck >= 0) {
            const bpm = ((value << 7) | control) / 10;
            engine.setValue("[Channel" + (this.syncDeck + 1) + "]", "bpm", bpm);
        }
    };

    this.startStopButtonPressed = function(channel, control, value, status, _group) {
        if (status === 0xFA) {
            this.playbackCounter = 1;
            this.playbackTimer = engine.beginTimer(500, () => {
                midi.sendShortMsg(0xBA, 0x02, this.playbackCounter);
                this.playbackCounter = (this.playbackCounter % 4) + 1;
            });
        } else if (status === 0xFC) {
            if (this.playbackTimer) {
                engine.stopTimer(this.playbackTimer);
            }
        }
    };

    this.customSamplePlayback = function(channel, control, value, status, group) {
        if (value) {
            // Volume has to be re-set because it could have been modified by
            // the Performance Pads in Velocity Sampler mode
            engine.setValue(group, "volume", engine.getValue("[" + DJ808.trsGroup + "]", "volume"));
            engine.setValue(group, "cue_gotoandplay", 1);
        }
    };

    this.levelKnob = new components.Pot({
        group: "[" + DJ808.trsGroup + "]",
        inKey: "volume",
        input: function(_channel, _control, _value, _status, _group) {
            components.Pot.prototype.input.apply(this, arguments);
            const volume = this.inGetParameter();
            for (let i = 1; i <= 16; i++) {
                engine.setValue("[Sampler" + i + "]", this.inKey, volume);
            }
        },
    });

    this.cueButton = new components.Button({
        group: "[" + DJ808.trsGroup + "]",
        key: "pfl",
        type: components.Button.prototype.types.push,
        midi: [0x9F, 0x1D],
        input: function(_channel, _control, _value, _status, _group) {
            components.Button.prototype.input.apply(this, arguments);
            const pfl = this.inGetValue();
            for (let i = 1; i <= 16; i++) {
                engine.setValue("[Sampler" + i + "]", this.inKey, pfl);
            }
        },
    });
};

DJ808.Sampler.prototype = Object.create(components.ComponentContainer.prototype);


////////////////////////
// Custom components. //
////////////////////////

DJ808.SlipModeButton = function(options) {
    components.Button.apply(this, arguments);
    this.doubleTapTimeout = 500;

    components.Button.call(this, options);
};
DJ808.SlipModeButton.prototype = Object.create(components.Button.prototype);
DJ808.SlipModeButton.prototype.unshift = function() {
    this.input = function(channel, control, value, _status, _group) {
        if (value) {                                                // Button press.
            this.inSetValue(true);
            return;
        }                                                   // Else: button release.

        if (!this.doubleTapped) {
            this.inSetValue(false);
        }

        this.doubleTapped = true;

        if (this.doubleTapTimer) {
            engine.stopTimer(this.doubleTapTimer);
            this.doubleTapTimer = null;
        }

        this.doubleTapTimer = engine.beginTimer(
            this.doubleTapTimeout,
            () => {
                this.doubleTapped = false;
                this.doubleTapTimer = null;
            },
            true
        );
    };
    this.inKey = "slip_enabled";
    this.outKey = "slip_enabled";
    this.type = components.Button.prototype.types.push;
    this.disconnect();
    this.connect();
    this.trigger();
};
DJ808.SlipModeButton.prototype.shift = function() {
    this.input = components.Button.prototype.input;
    this.inKey = "vinylcontrol_enabled";
    this.outKey = "vinylcontrol_enabled";
    this.type = components.Button.prototype.types.toggle;
    this.disconnect();
    this.connect();
    this.trigger();
};

DJ808.PadMode = {
    HOTCUE: 0x00,
    FLIP: 0x02,
    CUELOOP: 0x03,
    TR: 0x04,
    PATTERN: 0x05,
    TRVELOCITY: 0x06,
    ROLL: 0x08,
    SLICER: 0x09,
    SLICERLOOP: 0x0A,
    SAMPLER: 0x0B,
    VELOCITYSAMPLER: 0x0C,
    LOOP: 0x0D,
    PITCHPLAY: 0x0F,
};

DJ808.PadColor = {
    OFF: 0x00,
    RED: 0x01,
    ORANGE: 0x02,
    BLUE: 0x03,
    YELLOW: 0x04,
    APPLEGREEN: 0x05,
    MAGENTA: 0x06,
    CELESTE: 0x07,
    PURPLE: 0x08,
    APRICOT: 0x09,
    CORAL: 0x0A,
    AZURE: 0x0B,
    TURQUOISE: 0x0C,
    AQUAMARINE: 0x0D,
    GREEN: 0x0E,
    WHITE: 0x0F,
    DIM_MODIFIER: 0x10,
};

DJ808.PadColorMap = new ColorMapper({
    0xCC0000: DJ808.PadColor.RED,
    0xCC4400: DJ808.PadColor.CORAL,
    0xCC8800: DJ808.PadColor.ORANGE,
    0xCCCC00: DJ808.PadColor.YELLOW,
    0x88CC00: DJ808.PadColor.GREEN,
    0x00CC00: DJ808.PadColor.APPLEGREEN,
    0x00CC88: DJ808.PadColor.AQUAMARINE,
    0x00CCCC: DJ808.PadColor.TURQUOISE,
    0x0088CC: DJ808.PadColor.CELESTE,
    0x0000CC: DJ808.PadColor.BLUE,
    0x4400CC: DJ808.PadColor.AZURE,
    0x8800CC: DJ808.PadColor.PURPLE,
    0xCC00CC: DJ808.PadColor.MAGENTA,
    0xCC0044: DJ808.PadColor.RED,
    0xFFCCCC: DJ808.PadColor.APRICOT,
    0xFFFFFF: DJ808.PadColor.WHITE,
});

DJ808.PadSection = function(deck, offset) {
    // TODO: Add support for missing modes (flip, slicer, slicerloop)
    /*
     * The Performance Pad Section on the DJ-808 apparently have two basic
     * modes of operation that determines how the LEDs react to MIDI messages
     * and button presses.
     *
     * 1. Standalone mode
     *
     * The controller's performance pads allow setting various "modes" using
     * the mode buttons and the shift modifier. Pressing the mode buttons will
     * change their LED color (and makes the performance pads barely lit in
     * that color, too). However, the mode colors differ from that in the
     * Owner's manual. Also, it does not seem to be possible to actually
     * illuminate the performance pad LEDs - neither by pressing the button nor
     * by sending MIDI messages to the device.
     *
     * 2. Serato mode
     *
     * In this mode, pressing the pad mode buttons will not change their color.
     * Instead, all LEDs have to be controlled by sending MIDI messages. Unlike
     * in Standalone mode, it is also possible to illuminate the pad LEDs.
     *
     * The following table gives an overview over the different performance pad
     * modes. The values in the "Serato LED" and "Serato Mode" columns have
     * been taken from the Owner's Manual.
     *
     * Button                         MIDI control Standalone LED   Serato LED   Serato Mode
     * ------------------------------ ------------ ---------------- ------------ -----------
     * [HOT CUE]                      0x00         White            White        Hot Cue
     * [HOT CUE] (press twice)        0x02         Blue             Orange       Saved Flip
     * [SHIFT] + [HOT CUE]            0x03         Orange           Blue         Cue Loop
     * [ROLL]                         0x08         Turqoise         Light Blue   Roll
     * [ROLL] (press twice)           0x0D         Red              Green        Saved Loop
     * [SHIFT] + [ROLL]               0x09         Blue             Red          Slicer
     * [SHIFT] + [ROLL] (press twice) 0x0A         Blue             Blue         Slicer Loop
     * [TR]                           0x04         Red              Red          TR
     * [TR] (press twice)             0x06         Orange           Orange       TR Velocity
     * [SHIFT] + [TR]                 0x05         Green            Green        Pattern (Switches TR-S pattern)
     * [SAMPLER]                      0x0B         Purple           Magenta      Sampler
     * [SAMPLER] (press twice)        0x0F         Aquamarine       Green        Pitch Play
     * [SHIFT] + [SAMPLER]            0x0C         Magenta          Purple       Velocity Sampler
     *
     * The Pad and Mode Buttons support 31 different LED states:
     *
     *   MIDI value Color          MIDI value Color
     *   ---------- -----          ---------- -----
     *   0x00       Off            0x10       Off
     *   0x01       Red            0x11       Red (Dim)
     *   0x02       Orange         0x12       Orange (Dim)
     *   0x03       Blue           0x13       Blue (Dim)
     *   0x04       Yellow         0x14       Yellow (Dim)
     *   0x05       Applegreen     0x15       Applegreen (Dim)
     *   0x06       Magenta        0x16       Magenta (Dim)
     *   0x07       Celeste        0x17       Celeste (Dim)
     *   0x08       Purple         0x18       Purple (Dim)
     *   0x09       Apricot        0x19       Apricot (Dim)
     *   0x0A       Coral          0x1A       Coral (Dim)
     *   0x0B       Azure          0x1B       Azure (Dim)
     *   0x0C       Turquoise      0x1C       Turquoise (Dim)
     *   0x0D       Aquamarine     0x1D       Aquamarine (Dim)
     *   0x0E       Green          0x1E       Green (Dim)
     *   0x0F       White          0x1F       White (Dim)
     *
     * Serato DJ Pro maps its cue colors to MIDI values like this:
     *
     *   Number Default Cue  Serato Color       MIDI value Color
     *   ------ ------------ -----------------  ---------  ----------
     *        1            1 #CC0000 / #C02626  0x01       Red
     *        2              #CC4400 / #DB4E27  0x0A       Coral
     *        3            2 #CC8800 / #F8821A  0x02       Orange
     *        4            4 #CCCC00 / #FAC313  0x04       Yellow
     *        5              #88CC00 / #4EB648  0x0E       Green
     *        6              #44CC00 / #006838  0x0E       Green
     *        7            5 #00CC00 / #1FAD26  0x05       Applegreen
     *        8              #00CC44 / #8DC63F  0x0D       Aquamarine
     *        9              #00CC88 / #2B3673  0x0D       Aquamarine
     *       10            7 #00CCCC / #1DBEBD  0x0C       Turquoise
     *       11              #0088CC / #0F88CA  0x07       Celeste
     *       12              #0044CC / #16308B  0x03       Blue
     *       13            3 #0000CC / #173BA2  0x03       Blue
     *       14              #4400CC / #5C3F97  0x0B       Azure
     *       15            8 #8800CC / #6823B6  0x08       Purple
     *       16            6 #CC00CC / #CE359E  0x06       Magenta
     *       17              #CC0088 / #DC1D49  0x06       Magenta
     *       18              #CC0044 / #C71136  0x01       Red
     */
    components.ComponentContainer.call(this);
    this.modes = {
        // This need to be an object so that a recursive reconnectComponents
        // call won't influence all modes at once
        "hotcue": new DJ808.HotcueMode(deck, offset),
        "cueloop": new DJ808.CueLoopMode(deck, offset),
        "edit": new DJ808.EditMode(deck, offset),
        "roll": new DJ808.RollMode(deck, offset),
        "sampler": new DJ808.SamplerMode(deck, offset),
        "velocitysampler": new DJ808.VelocitySamplerMode(deck, offset),
        "loop": new DJ808.SavedLoopMode(deck, offset),
        "pitchplay": new DJ808.PitchPlayMode(deck, offset),
    };
    this.offset = offset;

    // Start in Hotcue Mode and disable other LEDs
    this.setPadMode(DJ808.PadMode.HOTCUE);
    midi.sendShortMsg(0x94 + offset, this.modes.roll.ledControl, DJ808.PadColor.OFF);
    midi.sendShortMsg(0x94 + offset, this.modes.sampler.ledControl, DJ808.PadColor.OFF);
};

DJ808.PadSection.prototype = Object.create(components.ComponentContainer.prototype);

DJ808.PadSection.prototype.controlToPadMode = function(control) {
    let mode;
    switch (control) {
    case DJ808.PadMode.HOTCUE:
        mode = this.modes.hotcue;
        break;
    // FIXME: Mixxx is currently missing support for Serato-style "flips",
    // hence this mode can only be implemented if this feature is added:
    // https://github.com/mixxxdj/mixxx/issues/9271
    //case DJ808.PadMode.FLIP:
    //    mode = this.modes.flip;
    //    break;
    case DJ808.PadMode.CUELOOP:
        if (this.currentMode === this.modes.cueloop) {
            mode = this.modes.edit;
        } else {
            mode = this.modes.cueloop;
        }
        break;
    case DJ808.PadMode.TR:
    case DJ808.PadMode.PATTERN:
    case DJ808.PadMode.TRVELOCITY:
        // All of these are hardcoded in hardware
        mode = null;
        break;
    case DJ808.PadMode.ROLL:
        mode = this.modes.roll;
        break;
    // FIXME: Although it might be possible to implement Slicer Mode, it would
    // miss visual feedback: https://github.com/mixxxdj/mixxx/issues/9660
    //case DJ808.PadMode.SLICER:
    //    mode = this.modes.slicer;
    //    break;
    //case DJ808.PadMode.SLICERLOOP:
    //    mode = this.modes.slicerloop;
    //    break;
    case DJ808.PadMode.SAMPLER:
        mode = this.modes.sampler;
        break;
    case DJ808.PadMode.VELOCITYSAMPLER:
        mode = this.modes.velocitysampler;
        break;
    case DJ808.PadMode.LOOP:
        mode = this.modes.loop;
        break;
    case DJ808.PadMode.PITCHPLAY:
        mode = this.modes.pitchplay;
        break;
    }

    return mode;
};

DJ808.PadSection.prototype.padModeButtonPressed = function(channel, control, value, _status, _group) {
    if (value) {
        this.setPadMode(control);
    }
};

DJ808.PadSection.prototype.paramButtonPressed = function(channel, control, value, status, group) {
    if (!this.currentMode) {
        return;
    }
    let button;
    switch (control) {
    case 0x2A: // PARAMETER 2 -
        if (this.currentMode.param2MinusButton) {
            button = this.currentMode.param2MinusButton;
            break;
        }
        /* falls through */
    case 0x28: // PARAMETER -
        button = this.currentMode.paramMinusButton;
        break;
    case 0x2B: // PARAMETER 2 +
        if (this.currentMode.param2PlusButton) {
            button = this.currentMode.param2PlusButton;
            break;
        }
        /* falls through */
    case 0x29: // PARAMETER +
        button = this.currentMode.paramPlusButton;
        break;
    }
    if (button) {
        button.input(channel, control, value, status, group);
    }
};

DJ808.PadSection.prototype.setPadMode = function(control) {
    const newMode = this.controlToPadMode(control);

    // Exit early if the requested mode is already active or not mapped
    if (newMode === this.currentMode || newMode === undefined) {
        return;
    }

    // If we're switching away from or to a hardware-based mode (e.g. TR mode),
    // the performance pad behaviour is hardcoded in the firmware and not
    // controlled by Mixxx. These modes are represented by the value null.
    // Hence, we only need to change LEDs and (dis-)connect components if
    // this.currentMode or newMode is not null.
    if (this.currentMode) {
        // Disable the mode button LED of the currently active mode
        midi.sendShortMsg(0x94 + this.offset, this.currentMode.ledControl, 0x00);

        this.currentMode.forEachComponent(function(component) {
            component.disconnect();
        });
    }

    if (newMode) {
        // Illuminate the mode button LED of the new mode
        midi.sendShortMsg(0x94 + this.offset, newMode.ledControl, newMode.color);

        // Set the correct shift state for the new mode. For example, if the
        // user is in HOT CUE mode and wants to switch to CUE LOOP mode, you
        // need to press [SHIFT]+[HOT CUE]. Pressing [SHIFT] will make the HOT
        // CUE mode pads become shifted.
        // When you're in CUE LOOP mode and want to switch back to
        // HOT CUE mode, the user need to press HOT CUE (without holding
        // SHIFT). However, the HOT CUE mode pads are still shifted even though
        // the user is not pressing [SHIFT] because they never got the unshift
        // event (the [SHIFT] button was released in CUE LOOP mode, not in HOT
        // CUE mode).
        // Hence, it's necessary to set the correct shift state when switching
        // modes.
        if (this.isShifted) {
            newMode.shift();
        } else {
            newMode.unshift();
        }

        newMode.forEachComponent(function(component) {
            component.connect();
            component.trigger();
        });
    }
    this.currentMode = newMode;
};

DJ808.PadSection.prototype.padPressed = function(channel, control, value, status, group) {
    const i = control - ((control >= 0x1C) ? 0x1C : 0x14);
    if (this.currentMode) {
        this.currentMode.pads[i].input(channel, control, value, status, group);
    }
};

DJ808.HotcueMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DJ808.PadMode.HOTCUE;
    this.color = DJ808.PadColor.WHITE;

    this.pads = new components.ComponentContainer();
    for (let i = 0; i <= 7; i++) {
        this.pads[i] = new components.HotcueButton({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i + 1,
            group: deck.currentDeck,
            on: this.color,
            off: this.color + DJ808.PadColor.DIM_MODIFIER,
            colorMapper: DJ808.PadColorMap,
            outConnect: false,
            unshift: function() {
                this.inKey = "hotcue_" + this.number + "_activatecue";
            },
        });
    }
    this.paramMinusButton = new components.Button({
        midi: [0x94 + offset, 0x28],
        group: deck.currentDeck,
        outKey: "hotcue_focus_color_prev",
        inKey: "hotcue_focus_color_prev",
    });
    this.paramPlusButton = new components.Button({
        midi: [0x94 + offset, 0x29],
        group: deck.currentDeck,
        outKey: "hotcue_focus_color_next",
        inKey: "hotcue_focus_color_next",
    });
    this.param2MinusButton = new components.Button({
        midi: [0x94 + offset, 0x2A],
        group: deck.currentDeck,
        outKey: "beats_translate_earlier",
        inKey: "beats_translate_earlier",
    });
    this.param2PlusButton = new components.Button({
        midi: [0x94 + offset, 0x2B],
        group: deck.currentDeck,
        outKey: "beats_translate_later",
        inKey: "beats_translate_later",
    });
};

DJ808.HotcueMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ808.CueLoopMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DJ808.PadMode.HOTCUE;
    this.color = DJ808.PadColor.BLUE;

    this.PerformancePad = function(n) {
        this.midi = [0x94 + offset, 0x14 + n];
        this.number = n + 1;
        this.outKey = "hotcue_" + this.number + "_enabled";
        this.colorKey = "hotcue_" + this.number + "_color";

        components.Button.call(this);
    };
    this.PerformancePad.prototype = new components.Button({
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 8,
        group: deck.currentDeck,
        on: this.color,
        off: this.color + DJ808.PadColor.DIM_MODIFIER,
        colorMapper: DJ808.PadColorMap,
        outConnect: false,
        unshift: function() {
            this.inKey = "hotcue_" + this.number + "_cueloop";
        },
        shift: function() {
            this.inKey = "hotcue_" + this.number + "_gotoandloop";
        },
        output: components.HotcueButton.prototype.output,
        outputColor: components.HotcueButton.prototype.outputColor,
        connect: components.HotcueButton.prototype.connect,
    });

    this.pads = new components.ComponentContainer();
    for (let n = 0; n <= 7; n++) {
        this.pads[n] = new this.PerformancePad(n);
    }

    this.paramMinusButton = new components.Button({
        midi: [0x94 + offset, 0x28],
        group: deck.currentDeck,
        outKey: "loop_halve",
        inKey: "loop_halve",
    });
    this.paramPlusButton = new components.Button({
        midi: [0x94 + offset, 0x29],
        group: deck.currentDeck,
        outKey: "loop_double",
        inKey: "loop_double",
    });
};
DJ808.CueLoopMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ808.EditMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DJ808.PadMode.HOTCUE;
    this.color = DJ808.PadColor.RED;

    this.pads = new components.ComponentContainer();
    for (let i = 0; i <= 3; i++) {
        const baseKey = [
            "intro_start",
            "intro_end",
            "outro_start",
            "outro_end",
        ][i];
        const color = (i > 1) ? DJ808.PadColor.AZURE : DJ808.PadColor.BLUE;

        this.pads[i] = new components.Button({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            baseKey: baseKey,
            outKey: baseKey + "_enabled",
            group: deck.currentDeck,
            on: color,
            off: color + DJ808.PadColor.DIM_MODIFIER,
            outConnect: false,
            unshift: function() {
                this.inKey = this.baseKey + "_activate";
            },
            shift: function() {
                this.inKey = this.baseKey + "_clear";
            },
        });
    }

    // Disable other pads (reserved for editing downbeats or sections)
    for (let i = 4; i <= 7; i++) {
        this.pads[i] = new components.Component({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            input: function(_channel, _control, _value, _status, _group) {},
            connect: function() {},
            trigger: function() {
                this.send(0);
            },
        });
    }
};
DJ808.EditMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ808.RollMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DJ808.PadMode.ROLL;
    this.color = DJ808.PadColor.CELESTE;
    this.pads = new components.ComponentContainer();
    this.loopSize = 0.03125;
    this.minSize = 0.03125;  // 1/32
    this.maxSize = 32;

    for (let i = 0; i <= 3; i++) {
        const loopSize = (this.loopSize * Math.pow(2, i));
        this.pads[i] = new components.Button({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            group: deck.currentDeck,
            outKey: "beatloop_" + loopSize + "_enabled",
            inKey: "beatlooproll_" + loopSize + "_activate",
            outConnect: false,
            on: this.color,
            off: (loopSize === 0.25) ? DJ808.PadColor.TURQUOISE : ((loopSize === 4) ? DJ808.PadColor.AQUAMARINE : (this.color + DJ808.PadColor.DIM_MODIFIER)),
        });
    }
    this.pads[4] = new components.Button({
        midi: [0x94 + offset, 0x18],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 8,
        group: deck.currentDeck,
        key: "beatjump_backward",
        outConnect: false,
        off: DJ808.PadColor.RED,
        on: DJ808.PadColor.RED + DJ808.PadColor.DIM_MODIFIER,
    });
    this.pads[5] = new components.Button({
        midi: [0x94 + offset, 0x19],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 8,
        group: deck.currentDeck,
        outKey: "beatjump_size",
        outConnect: false,
        on: DJ808.PadColor.ORANGE,
        off: DJ808.PadColor.ORANGE + DJ808.PadColor.DIM_MODIFIER,
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                const jumpSize = engine.getValue(this.group, "beatjump_size");
                if (jumpSize > this.mode.minSize) {
                    engine.setValue(this.group, "beatjump_size", jumpSize / 2);
                }
            }
        },
        output: function(value, _group, _control) {
            this.send((value > this.mode.minSize) ? this.on : this.off);
        },
    });
    this.pads[6] = new components.Button({
        midi: [0x94 + offset, 0x1A],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 8,
        group: deck.currentDeck,
        outKey: "beatjump_size",
        outConnect: false,
        on: DJ808.PadColor.ORANGE,
        off: DJ808.PadColor.ORANGE + DJ808.PadColor.DIM_MODIFIER,
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                const jumpSize = engine.getValue(this.group, "beatjump_size");
                if (jumpSize < this.mode.maxSize) {
                    engine.setValue(this.group, "beatjump_size", jumpSize * 2);
                }
            }
        },
        output: function(value, _group, _control) {
            this.send((value < this.mode.maxSize) ? this.on : this.off);
        },
    });
    this.pads[7] = new components.Button({
        midi: [0x94 + offset, 0x1B],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 8,
        group: deck.currentDeck,
        key: "beatjump_forward",
        outConnect: false,
        off: DJ808.PadColor.RED,
        on: DJ808.PadColor.RED + DJ808.PadColor.DIM_MODIFIER,
    });


    this.paramMinusButton = new components.Button({
        midi: [0x94 + offset, 0x28],
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                if (this.mode.loopSize > this.mode.minSize) {
                    this.mode.setLoopSize(this.mode.loopSize / 2);
                }
            }
            this.send(value);
        },
    });
    this.paramPlusButton = new components.Button({
        midi: [0x94 + offset, 0x29],
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                if (this.mode.loopSize * 8 < this.mode.maxSize) {
                    this.mode.setLoopSize(this.mode.loopSize * 2);
                }
            }
            this.send(value);
        },
    });
};
DJ808.RollMode.prototype = Object.create(components.ComponentContainer.prototype);
DJ808.RollMode.prototype.setLoopSize = function(loopSize) {
    this.loopSize = loopSize;
    for (let i = 0; i <= 3; i++) {
        const padLoopSize = (this.loopSize * Math.pow(2, i));
        this.pads[i].inKey = "beatlooproll_" + padLoopSize + "_activate";
        this.pads[i].outKey = "beatloop_" + padLoopSize + "_enabled";
        this.pads[i].off = (padLoopSize === 0.25) ? DJ808.PadColor.TURQUOISE : ((padLoopSize === 4) ? DJ808.PadColor.AQUAMARINE : (this.color + DJ808.PadColor.DIM_MODIFIER));
    }
    this.reconnectComponents();
};

DJ808.SavedLoopMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DJ808.PadMode.ROLL;
    this.color = DJ808.PadColor.GREEN;

    this.PerformancePad = function(n) {
        this.midi = [0x94 + offset, 0x14 + n];
        this.number = n + 8 + 1;
        this.outKey = "hotcue_" + this.number + "_enabled";
        this.colorKey = "hotcue_" + this.number + "_color";

        components.Button.call(this);
    };
    this.PerformancePad.prototype = new components.Button({
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 8,
        group: deck.currentDeck,
        outConnect: false,
        on: this.color,
        longPressTimeout: 500,
        colorMapper: DJ808.PadColorMap,
        unshift: function() {
            this.inKey = "hotcue_" + this.number + "_activateloop";
            this.input = components.Button.prototype.input;
        },

        shift: function() {
            this.inKey = "hotcue_" + this.number + "_gotoandloop";
            this.input = function(_channel, _control, value, _status, _group) {
                this.inSetValue(value);

                if (value) {
                    this.longPressTimer = engine.beginTimer(
                        this.longPressTimeout, () => {
                            engine.setValue(this.group, "hotcue_" + this.number + "_clear", 1);
                        });
                } else {
                    if (this.longPressTimer !== 0) {
                        engine.stopTimer(this.longPressTimer);
                        this.longPressTimer = 0;
                    }
                }
            }.bind(this);
        },
        stopBlinking: function() {
            if (this.connections[2] !== undefined) {
                this.connections[2].disconnect();
                delete this.connections[2];
            }
        },
        output: function(value, _group, _control) {
            this.stopBlinking();
            if (value === 2) {
                this.connections[2] = engine.makeConnection("[App]", "indicator_250ms", function(value, _group, _control) {
                    const colorValue = this.colorMapper.getValueForNearestColor(
                        engine.getValue(this.group, this.colorKey));
                    if (value) {
                        this.send(colorValue);
                    } else {
                        this.send(colorValue + DJ808.PadColor.DIM_MODIFIER);
                    }
                }.bind(this));
            } else if (value === 1) {
                const colorValue = this.colorMapper.getValueForNearestColor(
                    engine.getValue(this.group, this.colorKey));
                this.send(colorValue);
            } else {
                this.send(this.on + DJ808.PadColor.DIM_MODIFIER);
            }
        },
        connect: function() {
            components.Button.prototype.connect.call(this); // call parent connect
            if (undefined !== this.group && this.colorKey !== undefined) {
                this.connections[1] = engine.makeConnection(this.group, this.colorKey, function(color) {
                    if (engine.getValue(this.group, this.outKey) === 1) {
                        const colorValue = this.colorMapper.getValueForNearestColor(color);
                        this.send(colorValue);
                    }
                }.bind(this));
            }
        },
        disconnect: function() {
            components.Button.prototype.disconnect.call(this); // call parent connect
            this.stopBlinking();
        },
    });

    this.pads = new components.ComponentContainer();
    for (let n = 0; n <= 7; n++) {
        this.pads[n] = new this.PerformancePad(n);
    }

    this.paramMinusButton = new components.Button({
        midi: [0x94 + offset, 0x28],
        group: deck.currentDeck,
        outKey: "hotcue_focus_color_prev",
        inKey: "hotcue_focus_color_prev",
    });
    this.paramPlusButton = new components.Button({
        midi: [0x94 + offset, 0x29],
        group: deck.currentDeck,
        outKey: "hotcue_focus_color_next",
        inKey: "hotcue_focus_color_next",
    });
};
DJ808.SavedLoopMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ808.SamplerMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DJ808.PadMode.SAMPLER;
    this.color = DJ808.PadColor.MAGENTA;
    this.pads = new components.ComponentContainer();
    for (let i = 0; i <= 7; i++) {
        this.pads[i] = new components.SamplerButton({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i + 1,
            outConnect: false,
            on: this.color,
            off: this.color + DJ808.PadColor.DIM_MODIFIER,
        });
    }
};
DJ808.SamplerMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ808.VelocitySamplerMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DJ808.PadMode.SAMPLER;
    this.color = DJ808.PadColor.PURPLE;
    this.pads = new components.ComponentContainer();
    for (let i = 0; i <= 7; i++) {
        this.pads[i] = new components.SamplerButton({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i + 1,
            outConnect: false,
            on: this.color,
            off: this.color + DJ808.PadColor.DIM_MODIFIER,
            volumeByVelocity: true,
        });
    }
};
DJ808.VelocitySamplerMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ808.PitchPlayMode = function(deck, offset) {
    components.ComponentContainer.call(this);

    const PitchPlayRange = {
        UP: 0,
        MID: 1,
        DOWN: 2,
    };

    this.ledControl = DJ808.PadMode.SAMPLER;
    this.color = DJ808.PadColor.GREEN;
    this.cuepoint = 1;
    this.range = PitchPlayRange.MID;

    this.PerformancePad = function(n) {
        this.midi = [0x94 + offset, 0x14 + n];
        this.number = n + 1;
        this.on = this.color + DJ808.PadColor.DIM_MODIFIER;
        this.colorMapper = DJ808.PadColorMap;
        this.colorKey = "hotcue_" + this.number + "_color";
        components.Button.call(this);
    };
    this.PerformancePad.prototype = new components.Button({
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 8,
        group: deck.currentDeck,
        mode: this,
        outConnect: false,
        off: DJ808.PadColor.OFF,
        outputColor: function(colorCode) {
            // For colored hotcues (shifted only)
            const colorValue = this.colorMapper.getValueForNearestColor(colorCode);
            this.send((this.mode.cuepoint === this.number) ? colorValue : (colorValue + DJ808.PadColor.DIM_MODIFIER));
        },
        unshift: function() {
            this.outKey = "pitch_adjust";
            this.output = function(_value, _group, _control) {
                let color = this.mode.color + DJ808.PadColor.DIM_MODIFIER;
                if ((this.mode.range === PitchPlayRange.UP && this.number === 5) ||
                    (this.mode.range === PitchPlayRange.MID && this.number === 1) ||
                    (this.mode.range === PitchPlayRange.DOWN && this.number === 4)) {
                    color = DJ808.PadColor.WHITE;
                }
                this.send(color);
            };
            this.input = function(channel, control, value, _status, _group) {
                if (value > 0) {
                    let pitchAdjust;
                    switch (this.mode.range) {
                    case PitchPlayRange.UP:
                        pitchAdjust = this.number + ((this.number <= 4) ? 4 : -5);
                        break;
                    case PitchPlayRange.MID:
                        pitchAdjust = this.number - ((this.number <= 4) ? 1 : 9);
                        break;
                    case PitchPlayRange.DOWN:
                        pitchAdjust = this.number - ((this.number <= 4) ? 4 : 12);
                    }
                    engine.setValue(this.group, "pitch_adjust", pitchAdjust);
                    engine.setValue(this.group, "hotcue_" + this.mode.cuepoint + "_activate", value);
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
            this.outKey = "hotcue_" + this.number + "_enabled";
            this.output = function(value, _group, _control) {
                const outval = this.outValueScale(value);
                if (this.colorKey !== undefined && outval !== this.off) {
                    this.outputColor(engine.getValue(this.group, this.colorKey));
                } else {
                    this.send(DJ808.PadColor.OFF);
                }
            };
            this.input = function(channel, control, value, _status, _group) {
                if (value > 0 && this.mode.cuepoint !== this.number && engine.getValue(this.group, "hotcue_" + this.number + "_enabled")) {
                    const previousCuepoint = this.mode.cuepoint;
                    this.mode.cuepoint = this.number;
                    this.mode.pads[previousCuepoint - 1].trigger();
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
                    }.bind(this));
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

    this.paramMinusButton = new components.Button({
        midi: [0x94 + offset, 0x28],
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                if (this.mode.range === PitchPlayRange.UP) {
                    this.mode.range = PitchPlayRange.MID;
                } else if (this.mode.range === PitchPlayRange.MID) {
                    this.mode.range = PitchPlayRange.DOWN;
                } else {
                    this.mode.range = PitchPlayRange.UP;
                }
                this.mode.forEachComponent(function(component) {
                    component.trigger();
                });
            }
            this.send(value);
        },
    });
    this.paramPlusButton = new components.Button({
        midi: [0x94 + offset, 0x29],
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                if (this.mode.range === PitchPlayRange.UP) {
                    this.mode.range = PitchPlayRange.DOWN;
                } else if (this.mode.range === PitchPlayRange.MID) {
                    this.mode.range = PitchPlayRange.UP;
                } else {
                    this.mode.range = PitchPlayRange.MID;
                }
                this.mode.forEachComponent(function(component) {
                    component.trigger();
                });
            }
            this.send(value);
        },
    });
};
DJ808.PitchPlayMode.prototype = Object.create(components.ComponentContainer.prototype);
