/*
 * The Roland DJ-505 controller has two basic modes:
 *
 * 1. Standalone mode
 *
 * When the DJ-505 is not connected to a computer (or no SysEx/Keep-Alive
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
 * When the DJ-505 receives a SysEx message, the controller is put in "Serato"
 * mode.  However, in order to keep the DJ-505 in this mode, it seems to be
 * necessary to regularly send a "keep-alive" MIDI message (0xBF 0x64 0x00).
 * Otherwise the device will switch back to "Standalone mode" after
 * approximately 1.5 seconds.
 *
 * In Serato mode, the all LEDs have to be illuminated by sending MIDI
 * messages: Pressing a button does not switch the LED on and releasing it does
 * not switch it off. The performance pad LEDs can all be set individually
 * (including the mode buttons).
 *
 * The TR-S output is not connected to the master out. Instead, it is connected
 * to one of input channels of the controller's audio interface. Hence, the
 * TR/SAMPLER LEVEL knob does not control the output volume of TR-S, and
 * works as a generic MIDI control instead.
 *
 *
 * Other quirks and issues of the Roland DJ-505:
 * - The controller does not send the current value of the crossfader when it
 *   receives the SysEx message. This also happens when it's used with Serato,
 *   so Mixxx tries to work around the issue by using "soft takeover" to avoid
 *   sudden volume changes when the crossfader is first used.
 * - It does not seem to be possible to toggle the LEDs of the BACK and the ADD
 *   PREPARE buttons. Again, this can be reproduced in Serato, so it looks like
 *   a firmware problem and cannot be worked around.
 *
 */

var DJ505 = {};

/////////////////
// Tweakables. //
/////////////////

DJ505.stripSearchScaling = 0.15;
DJ505.tempoRange = [0.08, 0.16, 0.5];
DJ505.autoShowFourDecks = false;
DJ505.trsGroup = "Auxiliary1";  // TR-S input


///////////
// Code. //
///////////

DJ505.init = function() {
    var i, j;

    DJ505.shiftButton = function(channel, control, value, _status, _group) {
        DJ505.deck.concat(DJ505.effectUnit, [DJ505.sampler, DJ505.browseEncoder]).forEach(
            value ? function(module) { module.shift(); } : function(module) { module.unshift(); }
        );
    };

    DJ505.deck = [];
    for (i = 0; i < 4; i++) {
        DJ505.deck[i] = new DJ505.Deck(i + 1, i);
        DJ505.deck[i].setCurrentDeck("[Channel" + (i + 1) + "]");
    }


    DJ505.leftLoadTrackButton = new components.Button({
        group: "[Channel1]",
        midi: [0x9F, 0x02],
        unshift: function() {
            this.inKey = "LoadSelectedTrack";
        },
        shift: function() {
            this.inKey = "eject";
        },
        input: function(channel, control, value, status, _group) {
            this.send(this.isPress(channel, control, value, status) ? this.on : this.off);
            components.Button.prototype.input.apply(this, arguments);
        },
    });
    DJ505.deck3Button = new DJ505.DeckToggleButton({
        midi: [0x90, 0x08],
        decks: [1, 3],
        loadTrackButton: DJ505.leftLoadTrackButton,
    });

    DJ505.rightLoadTrackButton = new components.Button({
        group: "[Channel2]",
        midi: [0x9F, 0x02],
        unshift: function() {
            this.inKey = "LoadSelectedTrack";
        },
        shift: function() {
            this.inKey = "eject";
        },
        input: function(channel, control, value, status, _group) {
            this.send(this.isPress(channel, control, value, status) ? this.on : this.off);
            components.Button.prototype.input.apply(this, arguments);
        },
    });
    DJ505.deck4Button = new DJ505.DeckToggleButton({
        midi: [0x91, 0x08],
        decks: [2, 4],
        loadTrackButton: DJ505.rightLoadTrackButton,
    });

    DJ505.sampler = new DJ505.Sampler();

    DJ505.effectUnit = [];
    for (i = 0; i <= 1; i++) {
        DJ505.effectUnit[i] = new components.EffectUnit([i + 1, i + 3]);
        DJ505.effectUnit[i].sendShifted = true;
        DJ505.effectUnit[i].shiftOffset = 0x0B;
        DJ505.effectUnit[i].shiftControl = true;
        DJ505.effectUnit[i].enableButtons[1].midi = [0x98 + i, 0x00];
        DJ505.effectUnit[i].enableButtons[2].midi = [0x98 + i, 0x01];
        DJ505.effectUnit[i].enableButtons[3].midi = [0x98 + i, 0x02];
        DJ505.effectUnit[i].effectFocusButton.midi = [0x98 + i, 0x04];
        DJ505.effectUnit[i].knobs[1].midi = [0xB8 + i, 0x00];
        DJ505.effectUnit[i].knobs[2].midi = [0xB8 + i, 0x01];
        DJ505.effectUnit[i].knobs[3].midi = [0xB8 + i, 0x02];
        DJ505.effectUnit[i].dryWetKnob.midi = [0xB8 + i, 0x03];
        DJ505.effectUnit[i].dryWetKnob.input = function(channel, control, value, _status, _group) {
            if (value === 1) {
                // 0.05 is an example. Adjust that value to whatever works well for your controller.
                this.inSetParameter(this.inGetParameter() + 0.05);
            } else if (value === 127) {
                this.inSetParameter(this.inGetParameter() - 0.05);
            }
        };
        for (j = 1; j <= 4; j++) {
            DJ505.effectUnit[i].enableOnChannelButtons.addButton("Channel" + j);
            DJ505.effectUnit[i].enableOnChannelButtons["Channel" + j].midi = [0x98 + i, 0x04 + j];
        }
        DJ505.effectUnit[i].enableOnChannelButtons.addButton(DJ505.trsGroup);
        DJ505.effectUnit[i].enableOnChannelButtons[DJ505.trsGroup].midi = [0x98 + i, 0x09];
        DJ505.effectUnit[i].enableOnChannelButtons[DJ505.trsGroup].input = function(channel, control, value, status, _group) {
            components.Button.prototype.input.apply(this, arguments);
            if (this.isPress(channel, control, value, status)) {
                var enabled = this.inGetValue();
                for (var j = 1; j <= 16; j++) {
                    engine.setValue(this.group, "group_[Sampler" + j + "]_enable", enabled);
                }
            }
        };
        DJ505.effectUnit[i].enableOnTrsButton = DJ505.effectUnit[i].enableOnChannelButtons[DJ505.trsGroup];
        DJ505.effectUnit[i].init();
    }

    engine.makeConnection("[Channel3]", "track_loaded", DJ505.autoShowDecks);
    engine.makeConnection("[Channel4]", "track_loaded", DJ505.autoShowDecks);

    if (engine.getValue("[Master]", "num_samplers") < 16) {
        engine.setValue("[Master]", "num_samplers", 16);
    }

    // Send Serato SysEx messages to request initial state and unlock pads
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x00, 0xF7], 6);
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x01, 0xF7], 6);

    // Send "keep-alive" message to keep controller in Serato mode
    engine.beginTimer(500, function() {
        midi.sendShortMsg(0xBF, 0x64, 0x00);
    });

    // Reset LEDs
    DJ505.deck3Button.trigger();
    DJ505.deck4Button.trigger();
    for (i = 0; i < 4; i++) {
        DJ505.deck[i].reconnectComponents();
    }
};

DJ505.autoShowDecks = function(_value, _group, _control) {
    var anyLoaded = engine.getValue("[Channel3]", "track_loaded") || engine.getValue("[Channel4]", "track_loaded");
    if (!DJ505.autoShowFourDecks) {
        return;
    }
    engine.setValue("[Master]", "show_4decks", anyLoaded);
};

DJ505.shutdown = function() {
};


DJ505.browseEncoder = new components.Encoder({
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
                    var oldPos = engine.getValue("[PreviewDeck1]", "playposition");
                    var newPos = Math.max(0, oldPos + (0.05 * rotateValue));
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
                    function() { this.isLongPressed = true; },
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
                    var key = (rotateValue > 0) ? "track_color_next" : "track_color_prev";
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
        case 0xBF: // Rotate.
            var rotateValue = (value === 127) ? -1 : ((value === 1) ? 1 : 0);
            this.onKnobEvent(rotateValue);
            break;
        case 0x9F: // Push.
            this.onButtonEvent(value);
        }
    }
});

DJ505.backButton = new components.Button({
    // TODO: Map the BACK button
    midi: [0x9F, 0x07],
    shiftOffset: 11,
    sendShifted: true,
    shiftControl: true,
    type: undefined,
});

DJ505.addPrepareButton = new components.Button({
    midi: [0x9F, 0x1B],
    shiftOffset: -7,
    sendShifted: true,
    shiftControl: true,
    group: "[Master]",
    key: "maximize_library",
    type: components.Button.prototype.types.toggle,
});


DJ505.sortLibrary = function(channel, control, value, _status, _group) {
    if (value === 0) {
        return;
    }

    var sortColumn;
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

DJ505.crossfader = new components.Pot({
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
DJ505.crossfader.setCurve = function(channel, control, value, _status, _group) {
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

DJ505.crossfader.setReverse = function(channel, control, value, _status, _group) {
    // 0x00 is ON, 0x01 is OFF
    engine.setValue("[Mixer Profile]", "xFaderReverse", (value === 0x00) ? 1 : 0);
};

DJ505.setChannelInput = function(channel, control, value, _status, _group) {
    var number = (channel === 0x00) ? 0 : 1;
    var channelgroup = "[Channel" + (number + 1) + "]";
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

DJ505.Deck = function(deckNumbers, offset) {
    components.Deck.call(this, deckNumbers);

    this.slipModeButton = new DJ505.SlipModeButton({
        midi: [0x90 + offset, 0xF],
        shiftOffset: -8,
        shiftControl: true,
        sendShifted: true,
    });

    engine.setValue(this.currentDeck, "rate_dir", -1);
    this.tempoFader = new components.Pot({
        group: "[Channel" + deckNumbers + "]",
        midi: [0xB0 + offset, 0x09],
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
            var alpha = 1.0/8;
            var beta = alpha/32;
            engine.scratchEnable(script.deckFromGroup(this.currentDeck), 512, 45, alpha, beta);
        } else {    // If button up
            engine.scratchDisable(script.deckFromGroup(this.currentDeck));
        }
    };

    this.wheelTurn = function(channel, control, value, _status, _group) {
        // When the jog wheel is turned in clockwise direction, value is
        // greater than 64 (= 0x40). If it's turned in counter-clockwise
        // direction, the value is smaller than 64.
        var newValue = value - 64;
        var deck = script.deckFromGroup(this.currentDeck);
        if (engine.isScratching(deck)) {
            engine.scratchTick(deck, newValue); // Scratch!
        } else if (this.isShifted) {
            var oldPos = engine.getValue(this.currentDeck, "playposition");
            // Since ‘playposition’ is normalized to unity, we need to scale by
            // song duration in order for the jog wheel to cover the same amount
            // of time given a constant turning angle.
            var duration = engine.getValue(this.currentDeck, "duration");
            var newPos = Math.max(0, oldPos + (newValue * DJ505.stripSearchScaling / duration));
            engine.setValue(this.currentDeck, "playposition", newPos); // Strip search
        } else {
            engine.setValue(this.currentDeck, "jog", newValue); // Pitch bend
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
     * See Launchpad issue: https://bugs.launchpad.net/mixxx/+bug/419155
     */
    this.beatIndex = 0;
    this.lastBeatDistance = 0;
    engine.makeConnection(this.currentDeck, "beat_distance", function(value) {
        // Check if we're already in front of the next beat.
        if (value < this.lastBeatDistance) {
            this.beatIndex = (this.beatIndex + 1) % 4;
        }
        this.lastBeatDistance = value;

        // Since deck indices start with 1, we use 0xAF + deck for the status
        // byte, so that we 0xB0 for the first deck.
        var status = 0xAF + script.deckFromGroup(this.currentDeck);

        // Send a value between 0x00 and 0x7F to set jog wheel LED indicator
        midi.sendShortMsg(status, 0x06, Math.round(0x1f * value + 0x20 * this.beatIndex));
    });

    // ========================== LOOP SECTION ==============================

    this.loopActive = new components.Button({
        midi: [0x94 + offset, 0x32],
        inKey: "reloop_toggle",
        outKey: "loop_enabled",
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

    this.padSection = new DJ505.PadSection(this, offset);
    this.keylock = new components.Button({
        midi: [0x90 + offset, 0x0D],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 1,
        outKey: "keylock",
        currentRangeIndex: (DJ505.tempoRange.indexOf(engine.getValue("[Channel" + deckNumbers + "]", "rateRange"))) ? DJ505.tempoRange.indexOf(engine.getValue("[Channel" + deckNumbers + "]", "rateRange")) : 0,
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
                    if (this.currentRangeIndex >= DJ505.tempoRange.length) {
                        this.currentRangeIndex = 0;
                    }
                    this.inSetValue(DJ505.tempoRange[this.currentRangeIndex]);
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
            var state = engine.getValue(group, "cue_indicator");
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
        outKey: "sync_enabled",
        output: function(value, _group, _control) {
            midi.sendShortMsg(this.midi[0], value ? 0x02 : 0x03, this.on);
        },
        input: function(channel, control, value, _status, _group) {
            if (value) {
                this.longPressTimer = engine.beginTimer(this.longPressTimeout, function() {
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
                script.triggerControl(this.group, "beatsync", 1);
            };
            this.onLongPress = function() {
                engine.setValue(this.group, "sync_enabled", 1);
            };
        },
        shift: function() {
            this.onShortPress = function() {
                engine.setValue(this.group, "sync_enabled", 0);
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
    for (var k = 1; k <= 3; k++) {
        this.eqKnob[k] = new components.Pot({
            midi: [0xB0 + offset, 0x20 - k],
            group: "[EqualizerRack1_" + this.currentDeck + "_Effect1]",
            inKey: "parameter" + k,
        });
    }

    this.filter = new components.Pot({
        midi: [0xB0 + offset, 0x1A],
        group: "[QuickEffectRack1_" + this.currentDeck + "]",
        inKey: "super1",
    });

    this.pfl = new components.Button({
        midi: [0x90 + offset, 0x1B],
        group: "[Channel" + deckNumbers + "]",
        type: components.Button.prototype.types.toggle,
        inKey: "pfl",
        outKey: "pfl",
    });

    this.tapBPM = new components.Button({
        midi: [0x90 + offset, 0x12],
        group: "[Channel" + deckNumbers + "]",
        input: function(_channel, _control, value, _status, group) {
            if (value) {
                this.longPressTimer = engine.beginTimer(this.longPressTimeout, function() {
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
        outKey: "VuMeter",
        output: function(value, group, _control) {
            // The red LEDs light up with MIDI values greater than 0x24. The
            // maximum brightness is reached at value 0x28. Red LEDs should
            // only be illuminated if the track is clipping.
            if (engine.getValue(group, "PeakIndicator") === 1) {
                value = 0x28;
            } else {
                value = Math.round(value * 0x24);
            }
            this.send(value);
        },
    });
};

DJ505.Deck.prototype = Object.create(components.Deck.prototype);


DJ505.DeckToggleButton = function(options) {
    this.secondaryDeck = false;
    components.Button.call(this, options);
};
DJ505.DeckToggleButton.prototype = Object.create(components.Button.prototype);
DJ505.DeckToggleButton.prototype.input = function(channel, control, value, status, _group) {
    if (this.isPress(channel, control, value, status)) {
        // Button was pressed
        this.longPressTimer = engine.beginTimer(
            this.longPressTimeout,
            function() { this.isLongPressed = true; },
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
DJ505.DeckToggleButton.prototype.trigger = function() {
    this.send(this.secondaryDeck ? this.on : this.off);
    var newGroup = "[Channel" + (this.secondaryDeck ? this.decks[1] : this.decks[0]) + "]";
    if (this.loadTrackButton.group !== newGroup) {
        this.loadTrackButton.group = newGroup;
        this.loadTrackButton.disconnect();
        this.loadTrackButton.connect();
        this.loadTrackButton.trigger();
    }
};


//////////////////////////////
// TR/Sampler.              //
//////////////////////////////

DJ505.Sampler = function() {
    // TODO: Improve phase sync (workaround: use NUDGE button for beatmatching)
    /*
     * The TR-S section behaves differently depending on whether the controller
     * is in Standalone or Serato mode:
     *
     * 1. Standalone mode
     *
     * When the controller is in standlone mode, the controller's TR-S works
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
     * the master out. Instead, the sound is played on channels 7-8 so that the
     * signal can be routed through the FX section.
     *
     * The SERATO SAMPLER features 8 instruments (S1 - S8) that can be to play
     * samples from Serato's sampler banks. If the device is in Serato mode and
     * the sampler instruments are programmed using the TR-S pads, two MIDI
     * messages are sent by the DJ-505 when the sampler step is reached:
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

    var getActiveDeck = function() {
        var deckvolume = new Array(0, 0, 0, 0);
        var volumemax = -1;
        var newdeck = -1;

        // get volume from the decks and check it for use
        for (var z = 0; z <= 3; z++) {
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
        var isShifted = (control === 0x55);
        if (isShifted || this.syncDeck >= 0) {
            this.syncDeck = -1;
        } else {
            var deck = getActiveDeck();
            if (deck < 0) {
                return;
            }
            var bpm = engine.getValue("[Channel" + (deck + 1) + "]", "bpm");

            // Minimum BPM is 5.0 (0xEA 0x32 0x00), maximum BPM is 800.0 (0xEA 0x40 0x3e).
            if (!(bpm >= 5 && bpm <= 800)) {
                return;
            }
            var bpmValue = Math.round(bpm*10);
            midi.sendShortMsg(0xEA, bpmValue & 0x7f, (bpmValue >> 7) & 0x7f);
            this.syncDeck = deck;
        }
    };

    this.bpmKnobTurned = function(channel, control, value, _status, _group) {
        if (this.syncDeck >= 0) {
            var bpm = ((value << 7) | control) / 10;
            engine.setValue("[Channel" + (this.syncDeck + 1) + "]", "bpm", bpm);
        }
    };

    this.startStopButtonPressed = function(channel, control, value, status, _group) {
        if (status === 0xFA) {
            this.playbackCounter = 1;
            this.playbackTimer = engine.beginTimer(500, function() {
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
            engine.setValue(group, "volume", engine.getValue("[" + DJ505.trsGroup + "]", "volume"));
            engine.setValue(group, "cue_gotoandplay", 1);
        }
    };

    this.levelKnob = new components.Pot({
        group: "[" + DJ505.trsGroup + "]",
        inKey: "volume",
        input: function(_channel, _control, _value, _status, _group) {
            components.Pot.prototype.input.apply(this, arguments);
            var volume = this.inGetParameter();
            for (var i = 1; i <= 16; i++) {
                engine.setValue("[Sampler" + i + "]", this.inKey, volume);
            }
        },
    });

    this.cueButton = new components.Button({
        group: "[" + DJ505.trsGroup + "]",
        key: "pfl",
        type: components.Button.prototype.types.toggle,
        midi: [0x9F, 0x1D],
        input: function(_channel, _control, _value, _status, _group) {
            components.Button.prototype.input.apply(this, arguments);
            var pfl = this.inGetValue();
            for (var i = 1; i <= 16; i++) {
                engine.setValue("[Sampler" + i + "]", this.inKey, pfl);
            }
        },
    });
};

DJ505.Sampler.prototype = Object.create(components.ComponentContainer.prototype);


////////////////////////
// Custom components. //
////////////////////////

DJ505.SlipModeButton = function(options) {
    components.Button.apply(this, arguments);
    this.doubleTapTimeout = 500;

    components.Button.call(this, options);
};
DJ505.SlipModeButton.prototype = Object.create(components.Button.prototype);
DJ505.SlipModeButton.prototype.unshift = function() {
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
            function() {
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
DJ505.SlipModeButton.prototype.shift = function() {
    this.input = components.Button.prototype.input;
    this.inKey = "vinylcontrol_enabled";
    this.outKey = "vinylcontrol_enabled";
    this.type = components.Button.prototype.types.toggle;
    this.disconnect();
    this.connect();
    this.trigger();
};

DJ505.PadMode = {
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

DJ505.PadColor = {
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

DJ505.PadColorMap = new ColorMapper({
    0xCC0000: DJ505.PadColor.RED,
    0xCC4400: DJ505.PadColor.CORAL,
    0xCC8800: DJ505.PadColor.ORANGE,
    0xCCCC00: DJ505.PadColor.YELLOW,
    0x88CC00: DJ505.PadColor.GREEN,
    0x00CC00: DJ505.PadColor.APPLEGREEN,
    0x00CC88: DJ505.PadColor.AQUAMARINE,
    0x00CCCC: DJ505.PadColor.TURQUOISE,
    0x0088CC: DJ505.PadColor.CELESTE,
    0x0000CC: DJ505.PadColor.BLUE,
    0x4400CC: DJ505.PadColor.AZURE,
    0x8800CC: DJ505.PadColor.PURPLE,
    0xCC00CC: DJ505.PadColor.MAGENTA,
    0xCC0044: DJ505.PadColor.RED,
    0xFFCCCC: DJ505.PadColor.APRICOT,
    0xFFFFFF: DJ505.PadColor.WHITE,
});

DJ505.PadSection = function(deck, offset) {
    // TODO: Add support for missing modes (flip, slicer, slicerloop)
    /*
     * The Performance Pad Section on the DJ-505 apparently have two basic
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
        "hotcue": new DJ505.HotcueMode(deck, offset),
        "cueloop": new DJ505.CueLoopMode(deck, offset),
        "edit": new DJ505.EditMode(deck, offset),
        "roll": new DJ505.RollMode(deck, offset),
        "sampler": new DJ505.SamplerMode(deck, offset),
        "velocitysampler": new DJ505.VelocitySamplerMode(deck, offset),
        "pitchplay": new DJ505.PitchPlayMode(deck, offset),
    };
    this.offset = offset;

    // Start in Hotcue Mode and disable other LEDs
    this.setPadMode(DJ505.PadMode.HOTCUE);
    midi.sendShortMsg(0x94 + offset, this.modes.roll.ledControl, DJ505.PadColor.OFF);
    midi.sendShortMsg(0x94 + offset, this.modes.sampler.ledControl, DJ505.PadColor.OFF);
};

DJ505.PadSection.prototype = Object.create(components.ComponentContainer.prototype);

DJ505.PadSection.prototype.controlToPadMode = function(control) {
    var mode;
    switch (control) {
    case DJ505.PadMode.HOTCUE:
        mode = this.modes.hotcue;
        break;
    // FIXME: Mixxx is currently missing support for Serato-style "flips",
    // hence this mode can only be implemented if this feature is added:
    // https://bugs.launchpad.net/mixxx/+bug/1768113
    //case DJ505.PadMode.FLIP:
    //    mode = this.modes.flip;
    //    break;
    case DJ505.PadMode.CUELOOP:
        if (this.currentMode === this.modes.cueloop) {
            mode = this.modes.edit;
        } else {
            mode = this.modes.cueloop;
        }
        break;
    case DJ505.PadMode.TR:
    case DJ505.PadMode.PATTERN:
    case DJ505.PadMode.TRVELOCITY:
        // All of these are hardcoded in hardware
        mode = null;
        break;
    case DJ505.PadMode.ROLL:
        mode = this.modes.roll;
        break;
    // FIXME: Although it might be possible to implement Slicer Mode, it would
    // miss visual feedback: https://bugs.launchpad.net/mixxx/+bug/1828886
    //case DJ505.PadMode.SLICER:
    //    mode = this.modes.slicer;
    //    break;
    //case DJ505.PadMode.SLICERLOOP:
    //    mode = this.modes.slicerloop;
    //    break;
    case DJ505.PadMode.SAMPLER:
        mode = this.modes.sampler;
        break;
    case DJ505.PadMode.VELOCITYSAMPLER:
        mode = this.modes.velocitysampler;
        break;
    // FIXME: Loop mode can be added as soon as Saved Loops are
    // implemented: https://bugs.launchpad.net/mixxx/+bug/692926
    //case DJ505.PadMode.LOOP:
    //    mode = this.modes.loop;
    //    break;
    case DJ505.PadMode.PITCHPLAY:
        mode = this.modes.pitchplay;
        break;
    }

    return mode;
};

DJ505.PadSection.prototype.padModeButtonPressed = function(channel, control, value, _status, _group) {
    if (value) {
        this.setPadMode(control);
    }
};

DJ505.PadSection.prototype.paramButtonPressed = function(channel, control, value, status, group) {
    if (!this.currentMode) {
        return;
    }
    var button;
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

DJ505.PadSection.prototype.setPadMode = function(control) {
    var newMode = this.controlToPadMode(control);

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

DJ505.PadSection.prototype.padPressed = function(channel, control, value, status, group) {
    var i = control - ((control >= 0x1C) ? 0x1C : 0x14);
    if (this.currentMode) {
        this.currentMode.pads[i].input(channel, control, value, status, group);
    }
};

DJ505.HotcueMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DJ505.PadMode.HOTCUE;
    this.color = DJ505.PadColor.WHITE;

    this.pads = new components.ComponentContainer();
    for (var i = 0; i <= 7; i++) {
        this.pads[i] = new components.HotcueButton({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i + 1,
            group: deck.currentDeck,
            on: this.color,
            off: this.color + DJ505.PadColor.DIM_MODIFIER,
            colorMapper: DJ505.PadColorMap,
            outConnect: false,
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
DJ505.HotcueMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ505.CueLoopMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DJ505.PadMode.HOTCUE;
    this.color = DJ505.PadColor.BLUE;

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
        off: this.color + DJ505.PadColor.DIM_MODIFIER,
        colorMapper: DJ505.PadColorMap,
        outConnect: false,
        unshift: function() {
            this.input = function(channel, control, value, status, group) {
                if (value) {
                    var hotcueEnabled = true;
                    if (!engine.getValue(group, "hotcue_" + this.number + "_enabled")) {
                        // set a new cue point and loop
                        hotcueEnabled = false;
                        script.triggerControl(group, "hotcue_" + this.number + "_activate");
                    }
                    // jump to existing cue and loop
                    var startpos = engine.getValue(group, "hotcue_" + this.number + "_position");
                    var loopseconds = engine.getValue(group, "beatloop_size") * (1 / (engine.getValue(group, "bpm") / 60));
                    var loopsamples = loopseconds * engine.getValue(group, "track_samplerate") * 2;
                    var endpos = startpos + loopsamples;

                    // disable loop if currently enabled
                    if (engine.getValue(group, "loop_enabled")) {
                        if (hotcueEnabled &&
                                engine.getValue(group, "loop_start_position") === startpos &&
                                engine.getValue(group, "loop_end_position") === endpos) {
                            script.triggerControl(group, "loop_in_goto", 1);
                            return;
                        } else {
                            // disable active loop
                            script.triggerControl(group, "reloop_toggle", 1);
                        }
                    }

                    // set start and endpoints
                    engine.setValue(group, "loop_start_position", startpos);
                    engine.setValue(group, "loop_end_position", endpos);
                    // enable loop
                    script.triggerControl(group, "reloop_toggle", 1);
                    if (hotcueEnabled) {
                        script.triggerControl(group, "loop_in_goto", 1);
                    }
                }
            };
        },
        shift: function() {
            this.inKey = "hotcue_" + this.number + "_clear";
            this.input = components.Button.prototype.input;
        },
        output: components.HotcueButton.prototype.output,
        outputColor: components.HotcueButton.prototype.outputColor,
        connect: components.HotcueButton.prototype.connect,
    });

    this.pads = new components.ComponentContainer();
    for (var n = 0; n <= 7; n++) {
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
DJ505.CueLoopMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ505.EditMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DJ505.PadMode.HOTCUE;
    this.color = DJ505.PadColor.RED;

    this.pads = new components.ComponentContainer();
    for (var i = 0; i <= 3; i++) {
        var baseKey = [
            "intro_start",
            "intro_end",
            "outro_start",
            "outro_end",
        ][i];
        var color = (i > 1) ? DJ505.PadColor.AZURE : DJ505.PadColor.BLUE;

        this.pads[i] = new components.Button({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            baseKey: baseKey,
            outKey: baseKey + "_enabled",
            group: deck.currentDeck,
            on: color,
            off: color + DJ505.PadColor.DIM_MODIFIER,
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
    for (i = 4; i <= 7; i++) {
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
DJ505.EditMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ505.RollMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DJ505.PadMode.ROLL;
    this.color = DJ505.PadColor.CELESTE;
    this.pads = new components.ComponentContainer();
    this.loopSize = 0.03125;
    this.minSize = 0.03125;  // 1/32
    this.maxSize = 32;

    var loopSize;
    var i;
    for (i = 0; i <= 3; i++) {
        loopSize = (this.loopSize * Math.pow(2, i));
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
            off: (loopSize === 0.25) ? DJ505.PadColor.TURQUOISE : ((loopSize === 4) ? DJ505.PadColor.AQUAMARINE : (this.color + DJ505.PadColor.DIM_MODIFIER)),
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
        off: DJ505.PadColor.RED,
        on: DJ505.PadColor.RED + DJ505.PadColor.DIM_MODIFIER,
    });
    this.pads[5] = new components.Button({
        midi: [0x94 + offset, 0x19],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 8,
        group: deck.currentDeck,
        outKey: "beatjump_size",
        outConnect: false,
        on: DJ505.PadColor.ORANGE,
        off: DJ505.PadColor.ORANGE + DJ505.PadColor.DIM_MODIFIER,
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                var jumpSize = engine.getValue(this.group, "beatjump_size");
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
        on: DJ505.PadColor.ORANGE,
        off: DJ505.PadColor.ORANGE + DJ505.PadColor.DIM_MODIFIER,
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                var jumpSize = engine.getValue(this.group, "beatjump_size");
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
        off: DJ505.PadColor.RED,
        on: DJ505.PadColor.RED + DJ505.PadColor.DIM_MODIFIER,
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
DJ505.RollMode.prototype = Object.create(components.ComponentContainer.prototype);
DJ505.RollMode.prototype.setLoopSize = function(loopSize) {
    this.loopSize = loopSize;
    var padLoopSize;
    for (var i = 0; i <= 3; i++) {
        padLoopSize = (this.loopSize * Math.pow(2, i));
        this.pads[i].inKey = "beatlooproll_" + padLoopSize + "_activate";
        this.pads[i].outKey = "beatloop_" + padLoopSize + "_enabled";
        this.pads[i].off = (padLoopSize === 0.25) ? DJ505.PadColor.TURQUOISE : ((padLoopSize === 4) ? DJ505.PadColor.AQUAMARINE : (this.color + DJ505.PadColor.DIM_MODIFIER));
    }
    this.reconnectComponents();
};

DJ505.SamplerMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DJ505.PadMode.SAMPLER;
    this.color = DJ505.PadColor.MAGENTA;
    this.pads = new components.ComponentContainer();
    for (var i = 0; i <= 7; i++) {
        this.pads[i] = new components.SamplerButton({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i + 1,
            outConnect: false,
            on: this.color,
            off: this.color + DJ505.PadColor.DIM_MODIFIER,
        });
    }
};
DJ505.SamplerMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ505.VelocitySamplerMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DJ505.PadMode.SAMPLER;
    this.color = DJ505.PadColor.PURPLE;
    this.pads = new components.ComponentContainer();
    for (var i = 0; i <= 7; i++) {
        this.pads[i] = new components.SamplerButton({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i + 1,
            outConnect: false,
            on: this.color,
            off: this.color + DJ505.PadColor.DIM_MODIFIER,
            volumeByVelocity: true,
        });
    }
};
DJ505.VelocitySamplerMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ505.PitchPlayMode = function(deck, offset) {
    components.ComponentContainer.call(this);

    var PitchPlayRange = {
        UP: 0,
        MID: 1,
        DOWN: 2,
    };

    this.ledControl = DJ505.PadMode.SAMPLER;
    this.color = DJ505.PadColor.GREEN;
    this.cuepoint = 1;
    this.range = PitchPlayRange.MID;

    this.PerformancePad = function(n) {
        this.midi = [0x94 + offset, 0x14 + n];
        this.number = n + 1;
        this.on = this.color + DJ505.PadColor.DIM_MODIFIER;
        this.colorMapper = DJ505.PadColorMap;
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
        off: DJ505.PadColor.OFF,
        outputColor: function(colorCode) {
            // For colored hotcues (shifted only)
            var midiColor = this.colorMapper.getValueForNearestColor(colorCode);
            this.send((this.mode.cuepoint === this.number) ? midiColor : (midiColor + DJ505.PadColor.DIM_MODIFIER));
        },
        unshift: function() {
            this.outKey = "pitch_adjust";
            this.output = function(_value, _group, _control) {
                var color = this.mode.color + DJ505.PadColor.DIM_MODIFIER;
                if ((this.mode.range === PitchPlayRange.UP && this.number === 5) ||
                    (this.mode.range === PitchPlayRange.MID && this.number === 1) ||
                    (this.mode.range === PitchPlayRange.DOWN && this.number === 4)) {
                    color = DJ505.PadColor.WHITE;
                }
                this.send(color);
            };
            this.input = function(channel, control, value, _status, _group) {
                if (value > 0) {
                    var pitchAdjust;
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
                var outval = this.outValueScale(value);
                if (this.colorKey !== undefined && outval !== this.off) {
                    this.outputColor(engine.getValue(this.group, this.colorKey));
                } else {
                    this.send(DJ505.PadColor.OFF);
                }
            };
            this.input = function(channel, control, value, _status, _group) {
                if (value > 0 && this.mode.cuepoint !== this.number && engine.getValue(this.group, "hotcue_" + this.number + "_enabled")) {
                    var previousCuepoint = this.mode.cuepoint;
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
    for (var n = 0; n <= 7; n++) {
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
DJ505.PitchPlayMode.prototype = Object.create(components.ComponentContainer.prototype);
