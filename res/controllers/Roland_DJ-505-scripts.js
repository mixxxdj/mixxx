////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global midi                                                        */
/* global bpm                                                         */
/* global components                                                  */
////////////////////////////////////////////////////////////////////////

var DJ505 = {};

/////////////////
// Tweakables. //
/////////////////

DJ505.stripSearchScaling = 50;
DJ505.tempoRange = [0.08, 0.16, 0.5];
DJ505.autoFocusEffects = false;
DJ505.autoShowFourDecks = false;


///////////
// Code. //
///////////

DJ505.init = function () {

    DJ505.shiftButton = function (channel, control, value, status, group) {
        DJ505.deck.concat(DJ505.effectUnit, [DJ505.sampler]).forEach(
            value ? function (module) { module.shift(); } : function (module) { module.unshift(); }
        );
    };

    DJ505.leftDeck = new DJ505.Deck([1,3], 0);
    DJ505.rightDeck = new DJ505.Deck([2,4], 1);
    DJ505.deck = [DJ505.leftDeck, DJ505.rightDeck];

    DJ505.sampler = new DJ505.Sampler();

    DJ505.effectUnit = [];
    for(var i = 0; i <= 1; i++) {
        DJ505.effectUnit[i] = new components.EffectUnit([i + 1, i + 3]);
        DJ505.effectUnit[i].sendShifted = false;
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
        DJ505.effectUnit[i].dryWetKnob.input = function (channel, control, value, status, group) {
            if (value === 1) {
                // 0.05 is an example. Adjust that value to whatever works well for your controller.
                this.inSetParameter(this.inGetParameter() + 0.05);
            } else if (value === 127) {
                this.inSetParameter(this.inGetParameter() - 0.05);
            }
        };
        for(var j = 1; j <= 4; j++) {
            DJ505.effectUnit[i].enableOnChannelButtons.addButton("Channel" + j);
            DJ505.effectUnit[i].enableOnChannelButtons["Channel" + j].midi = [0x98 + i, 0x04 + j];
        }
        DJ505.effectUnit[i].enableOnChannelButtons.addButton("Auxiliary3");
        DJ505.effectUnit[i].enableOnChannelButtons.Auxiliary3.midi = [0x98 + i, 0x09];
        DJ505.effectUnit[i].enableOnChannelButtons.Auxiliary3.input = function (channel, control, value, status, group) {
            components.Button.prototype.input.apply(this, arguments);
            if (this.isPress(channel, control, value, status)) {
                var enabled = this.inGetValue();
                for(var j = 1; j <= 16; j++) {
                    engine.setValue(this.group, "group_[Sampler" + j + "]_enable", enabled);
                }
            }
        };
        DJ505.effectUnit[i].init();
    }

    engine.makeConnection("[Channel3]", "track_loaded", DJ505.autoShowDecks);
    engine.makeConnection("[Channel4]", "track_loaded", DJ505.autoShowDecks);

    if (engine.getValue("[Master]", "num_samplers") < 16) {
        engine.setValue("[Master]", "num_samplers", 16);
    }

    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x00, 0xF7], 6); //request initial state
    midi.sendSysexMsg([0xF0, 0x00, 0x20, 0x7F, 0x01, 0xF7], 6); //unlock pad layers
    DJ505.leftDeck.padSection.resetLEDs();
    DJ505.rightDeck.padSection.resetLEDs();

    engine.beginTimer(500, function() {
        // Keep sending this message to enable performance pad LEDs
        midi.sendShortMsg(0xBF, 0x64, 0x00);
    });

    DJ505.leftDeck.setCurrentDeck("[Channel1]");
    DJ505.rightDeck.setCurrentDeck("[Channel2]");

};

DJ505.autoShowDecks = function (value, group, control) {
    var any_loaded = engine.getValue("[Channel3]", "track_loaded") || engine.getValue("[Channel4]", "track_loaded");
    if (!DJ505.autoShowFourDecks) {
        return;
    }
    engine.setValue("[Master]", "show_4decks", any_loaded);
};

DJ505.shutdown = function () {
};

DJ505.browseEncoder = new components.Encoder({
    input: function (channel, control, value, status, group) {
        var isShifted = control % 2 != 0;
        switch (status) {
        case 0xBF: // Rotate.
            if (value === 127) {
                engine.setValue("[Playlist]", isShifted ? "SelectPlaylist" : "SelectTrackKnob", -1);
            } else if (value === 1) {
                engine.setValue("[Playlist]", isShifted ? "SelectPlaylist" : "SelectTrackKnob", 1);
            }
            break;
        case 0x9F: // Push.
            if (value) {
                script.triggerControl("[Playlist]", "ToggleSelectedSidebarItem");
            }
        }
    }
});

DJ505.backButton = new components.Button({
    // TODO: Map the BACK/SONG button
    midi: [0x9F, 0x07],
    shiftOffset: 11,
    sendShifted: true,
    shiftControl: true,
    type: undefined,
});

DJ505.addPrepareButton = new components.Button({
    // TODO: Map the ARTIST button
    midi: [0x9F, 0x1B],
    shiftOffset: -7,
    sendShifted: true,
    shiftControl: true,
    group: "[Master]",
    key: "maximize_library",
    type: components.Button.prototype.types.toggle,
});

DJ505.crossfader = new components.Pot({
    midi: [0xBF, 0x08],
    group: "[Master]",
    inKey: "crossfader",
    input: function () {
        // We need a weird max. for the crossfader to make it cut cleanly.
        // However, components.js resets max. to 0x3fff when the first value is
        // received. Hence, we need to set max. here instead of within the
        // constructor.
        this.max = (0x7f<<7) + 0x70;
        components.Pot.prototype.input.apply(this, arguments);
    }
});
DJ505.crossfader.setCurve = function (channel, control, value, status, group) {
    // 0x00 is Picnic Bench, 0x01 is Constant Power and 0x02 is Linear
    switch(value) {
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

DJ505.crossfader.setReverse = function (channel, control, value, status, group) {
    // 0x00 is ON, 0x01 is OFF
    engine.setValue("[Mixer Profile]", "xFaderReverse", (value == 0x00) ? 1 : 0);
};

DJ505.setChannelInput = function (channel, control, value, status, group) {
    // TODO: Add support for PHONO setting
    var channel_number = (channel == 0x00) ? 1 : 2;
    var auxgroup = "[Auxiliary" + channel_number + "]";
    var channelgroup = "[Channel" + channel_number + "]";
    switch(value) {
    case 0x00:  // PC
        engine.setValue(auxgroup, "mute" , 1);
        engine.setValue(channelgroup, "mute", 0);
        break;
    case 0x01:  // LINE
        engine.setValue(auxgroup, "master" , 0);
        engine.setValue(auxgroup, "orientation" , channel_number ? 0 : 2);
        engine.setValue(channelgroup, "mute", 1);
        engine.setValue(auxgroup, "mute" , 0);
        break;
    case 0x02:  // PHONO
        engine.setValue(channelgroup, "mute", 0);
        engine.setValue(auxgroup, "mute" , 0);
        break;
    }
};

DJ505.Deck = function (deckNumbers, offset) {
    components.Deck.call(this, deckNumbers);

    this.loadTrack = new components.Button({
        midi: [0x9F, 0x02 + offset],
        unshift: function () {
            this.inKey = "LoadSelectedTrack";
        },
        shift: function () {
            this.inKey = "eject";
        },
    });

    this.slipModeButton = new DJ505.SlipModeButton();

    engine.setValue(this.currentDeck, "rate_dir", -1);
    this.tempoFader = new components.Pot({
        group: this.currentDeck,
        midi: [0xB0 + offset, 0x09],
        connect: function () {
            engine.softTakeover(this.group, "pitch", true);
            engine.softTakeover(this.group, "rate", true);
            components.Pot.prototype.connect.apply(this, arguments);
        },
        unshift: function () {
            this.inKey = "rate";
            this.inSetParameter = components.Pot.prototype.inSetParameter;
            engine.softTakeoverIgnoreNextValue(this.group, "pitch");
        },
        shift: function () {
            this.inKey = "pitch";
            this.inSetParameter = function (value) {
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
    this.wheelTouch = function (channel, control, value, status, group) {
        if (value === 0x7F && !this.isShifted) {
            var alpha = 1.0/8;
            var beta = alpha/32;
            engine.scratchEnable(script.deckFromGroup(this.currentDeck), 512, 45, alpha, beta);
        } else {    // If button up
            engine.scratchDisable(script.deckFromGroup(this.currentDeck));
        }
    };

    this.wheelTurn = function (channel, control, value, status, group) {
        var newValue = value - 64;
        var deck = script.deckFromGroup(this.currentDeck);
        if (engine.isScratching(deck)) {
            engine.scratchTick(deck, newValue); // Scratch!
        } else if (this.isShifted) {
            // Strip search.
            var oldPos = engine.getValue(this.currentDeck, "playposition");
            // Scale to interval [0,1].
            newValue = newValue / 0xff;
            // Since ‘playposition’ is normalized to unity, we need to scale by
            // song duration in order for the jog wheel to cover the same amount
            // of time given a constant turning angle.
            var duration = engine.getValue(this.currentDeck, "duration");
            newValue = newValue / duration;
            var newPos = Math.max(0, oldPos + newValue * DJ505.stripSearchScaling);
            engine.setValue(this.currentDeck, "playposition", newPos);
        } else {
            engine.setValue(this.currentDeck, "jog", newValue); // Pitch bend
        }
    };


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
    this.autoLoop = new components.Button({
        midi: [0x94 + offset, 0x40],
        inKey: "beatloop_activate",
        outKey: "loop_enabled",
        input: function (channel, control, value, status, group) {
            components.Button.prototype.input.call(this, channel, control, value, status, group);
            if (value) {
                return;
            }
            this.trigger();
        },
    });


    // ========================== PERFORMANCE PADS ==============================

    this.padSection = new DJ505.PadSection(this, offset);
    this.keylock = new components.Button({
        midi: [0x90 + offset, 0x0D],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 1,
        group: this.currentDeck,
        outKey: "keylock",
        currentRangeIndex: (DJ505.tempoRange.indexOf(engine.getValue(this.group, "rateRange"))) ? DJ505.tempoRange.indexOf(engine.getValue(this.group, "rateRange")) : 0,
        unshift: function () {
            this.inKey = "keylock";
            this.input = components.Button.prototype.input;
            this.type = components.Button.prototype.types.toggle;
        },
        shift: function () {
            this.inKey = "rateRange";
            this.type = undefined;
            this.input = function (channel, control, value, status, group) {
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
        midi: [0x90 + offset, 0x1],
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
        reverseRollOnShift: false,
        input: function (channel, control, value, status, group) {
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

    this.play = new components.Button({
        midi: [0x90 + offset, 0],
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
        outKey: "play_indicator",
        unshift: function () {
            this.inKey = "play";
            this.input = function (channel, control, value, status, group) {

                if (value) {                                    // Button press.
                    this.longPressStart = new Date();
                    this.longPressTimer = engine.beginTimer(
                        this.longPressTimeout,
                        function () { this.longPressed = true; },
                        true
                    );
                    return;
                }                                       // Else: Button release.

                var isPlaying = engine.getValue(group, "play");

                // Normalize ‘isPlaying’ – we consider the braking state
                // equivalent to being stopped, so that pressing play again can
                // trigger a soft-startup even before the brake is complete.
                if (this.isBraking !== undefined) {
                    isPlaying = isPlaying && !this.isBraking;
                }

                if (this.longPressed) {             // Release after long press.
                    var deck = script.deckFromGroup(group);
                    var pressDuration = new Date() - this.longPressStart;
                    if (isPlaying && !this.isBraking) {
                        engine.brake(deck, true, 1000 / pressDuration);
                        this.isBraking = true;
                    } else {
                        engine.softStart(deck, true, 1000 / pressDuration);
                        this.isBraking = false;
                    }
                    this.longPressed = false;
                    return;
                }                            // Else: Release after short press.

                this.isBraking = false;
                script.toggleControl(group, "play", !isPlaying);

                if (this.longPressTimer) {
                    engine.stopTimer(this.longPressTimer);
                    this.longPressTimer = null;
                }
            };
        },
        shift: function () {
            this.inKey = "reverse";
            this.input = function (channel, control, value, status, group) {
                components.Button.prototype.input.apply(this, arguments);
                if(!value) {
                    this.trigger();
                }

            };
        }
    });

    this.sync = new components.Button({
        midi: [0x90 + offset, 0x02],
        group: this.currentDeck,
        outKey: "sync_enabled",
        output: function (value, group, control) {
            midi.sendShortMsg(this.midi[0], value ? 0x02 : 0x03, 0x7F);
        },
        unshift: function () {
            this.input = function (channel, control, value, status, group) {
                if (this.isPress(channel, control, value, status)) {
                    script.triggerControl(this.group, "beatsync", 1);
                    if (engine.getValue(this.group, "sync_enabled") === 0) {
                        this.longPressTimer = engine.beginTimer(this.longPressTimeout, function () {
                            engine.setValue(this.group, "sync_enabled", 1);
                            this.longPressTimer = 0;
                        }, true);
                    }
                } else {
                    if (this.longPressTimer !== 0) {
                        engine.stopTimer(this.longPressTimer);
                        this.longPressTimer = 0;
                    }
                }
            };
        },
        shift: function () {
            this.input = function (channel, control, value, status, group) {
                if (value) {
                    engine.setValue(this.group, "sync_enabled", 0);
                }
            }
        },
    });

    // =============================== MIXER ====================================
    this.pregain = new components.Pot({
        group: this.currentDeck,
        midi: [0xB0 + offset, 0x16],
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
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: 2,
        midi: [0x90 + offset, 0x1B],
        type: components.Button.prototype.types.toggle,
        inKey: "pfl",
        outKey: "pfl",
    });

    this.tapBPM = new components.Button({
        input: function (channel, control, value, status, group) {
            if (value == 127) {
                script.triggerControl(group, "beats_translate_curpos");
                bpm.tapButton(script.deckFromGroup(group));
                this.longPressTimer = engine.beginTimer(
                    this.longPressTimeout,
                    function () {
                        script.triggerControl(group, "beats_translate_match_alignment");
                    },
                    true
                );
            } else {
                engine.stopTimer(this.longPressTimer);
            }
        }
    });

    this.volume = new components.Pot({
        group: this.currentDeck,
        midi: [0xB0 + offset, 0x1C],
        inKey: "volume",
    });

    this.setDeck = new components.Button({
        midi: [0x90 + offset, 0x08],
        deck: this,
        input: function (channel, control, value, status, group) {
            var currentDeck = script.deckFromGroup(this.deck.currentDeck);
            var otherDeck = currentDeck == deckNumbers[0] ? deckNumbers[1] : deckNumbers[0];

            otherDeck = "[Channel" + otherDeck + "]";

            if (value) {                                        // Button press.
                this.longPressTimer = engine.beginTimer(
                    this.longPressTimeout,
                    function () { this.isLongPressed = true; },
                    true
                );
                this.deck.setCurrentDeck(otherDeck);
                return;
            }                                           // Else: Button release.

            if (this.longPressTimer) {
                engine.stopTimer(this.longPressTimer);
                this.longPressTimer = null;
            }

            // Since we are in the release phase, currentDeck still reflects the
            // switched decks. So if we are now using deck 1/3, we were
            // originally using deck 2/4 and vice versa.
            var deckWasVanilla = currentDeck == deckNumbers[1];

            if (this.isLongPressed) {                     // Release long press.
                this.isLongPressed = false;
                // Return to the original state.
                this.send(deckWasVanilla ? 0 : 0x7f);
                this.deck.setCurrentDeck(otherDeck);
                return;
            }                                      // Else: Release short press.

            // Invert the deck state.
            this.send(deckWasVanilla ? 0x7f : 0);
        }
    });
};

DJ505.Deck.prototype = Object.create(components.Deck.prototype);


//////////////////////////////
// TR/Sampler.              //
//////////////////////////////

DJ505.Sampler = function() {
    // TODO: Improve sync so that we don't need to use the NUDGE button for
    // beatmatching.
    // TODO: Add support for custom samples
    /*
     * Like the performance pads, the built-in TR-S drum machine 505 has two
     * modes of operation.
     *
     * - Standalone mode:
     *   When the DJ-505 is not connected to a computer (or no SysEx/Keep-Alive
     *   messages are sent), the controller's TR-S works with the SERATO SAMPLER
     *   and SYNC functionality disabled. Also, it's not possible to apply FX
     *   to the TR-S output signal. The TR/SAMPLER LEVEL knob can be used to adjust
     *   the volume of the output. It's possible to set the BPM in the range
     *   5.0 to 800.0 BPM by sending this MIDI message:
     *
     *       sendShortMsg(0xEA, Math.round(bpm*10) & 0x7f, (Math.round(bpm*10) >> 7) & 0x7f);
     *
     * - Serato mode:
     *   When the DJ-505 receives a SysEx message, the TR-S is put in "Serato"
     *   mode. In this mode, the BPM can also be set by sending MIDI clock
     *   messages (0xF8). The sampler can be started by sending one MIDI
     *   message per bar (0xBA 0x02 XX). In this mode, the TR-S is not directly
     *   connected to the master out. Instead, the sound is played on channels
     *   7-8 so that the signal can be router through the FX section.
     *   However, in order to keep the DJ-505 in "Serato mode", it seems to be
     *   necessary to regularily send a certain "keep-alive" MIDI message (0xBF
     *   0x64 0x00). Otherwise the device will switch back to "Standalone mode"
     *   after approximately 1.5 seconds.
     *
     * The SERATO SAMPLER features 8 instruments (S1 - S8) that can be to play
     * samples from Serato's sampler banks. If the device is in Serato mode and
     * the sampler instruments are programmed using the TRS pads, two MIDI
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

    this.syncButtonPressed = function (channel, control, value, status, group) {
        if (value != 0x7f) {
            return;
        }
        var isShifted = (control == 0x55);
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
            var bpm_value = Math.round(bpm*10);
            midi.sendShortMsg(0xEA, bpm_value & 0x7f, (bpm_value >> 7) & 0x7f);
            this.syncDeck = deck;
        }
        print(this.syncDeck);
    };

    this.bpmKnobTurned = function (channel, control, value, status, group) {
        print(this.syncDeck);
        if (this.syncDeck >= 0) {
            var bpm = ((value << 7) | control) / 10;
            engine.setValue("[Channel" + (this.syncDeck + 1) + "]", "bpm", bpm);
        }
    };

    this.startStopButtonPressed = function (channel, control, value, status, group) {
        if (status == 0xFA) {
            this.playbackCounter = 1;
            this.playbackTimer = engine.beginTimer(500, function() {
                midi.sendShortMsg(0xBA, 0x02, this.playbackCounter);
                this.playbackCounter = (this.playbackCounter % 4) + 1;
            });
        } else if (status == 0xFC) {
            if (this.playbackTimer) {
                engine.stopTimer(this.playbackTimer);
            }
        }
    };

    this.customSamplePlayback = function (channel, control, value, status, group) {
        if (value) {
            // Volume has to be re-set because it could have been modified by
            // the Performance Pads in Velocity Sampler mode
            engine.setValue(group, "volume", engine.getValue("[Auxiliary3]", "volume"));
            engine.setValue(group, "cue_gotoandplay", 1);
        }
    };

    this.levelKnob = new components.Pot({
        group: "[Auxiliary3]",
        inKey: "volume",
        input: function (channel, control, value, status, group) {
            components.Pot.prototype.input.apply(this, arguments);
            var volume = this.inGetParameter();
            for (var i = 1; i <= 16; i++) {
                engine.setValue("[Sampler" + i + "]", this.inKey, volume);
            }
        },
    });

    this.cueButton = new components.Button({
        group: "[Auxiliary3]",
        key: "pfl",
        type: components.Button.prototype.types.toggle,
        midi: [0x9F, 0x1D],
        input: function (channel, control, value, status, group) {
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

DJ505.SlipModeButton = function () {
    components.Button.apply(this, arguments);
    this.inKey = "slip_enabled";
    this.outKey = "slip_enabled";
    this.doubleTapTimeout = 500;
};

DJ505.SlipModeButton.prototype = Object.create(components.Button.prototype);

DJ505.SlipModeButton.prototype.connect = function () {
    var deck = script.deckFromGroup(this.group);
    this.midi = [0x90 + deck - 1, 0xF];
    components.Button.prototype.connect.call(this);
};

DJ505.SlipModeButton.prototype.input = function (channel, control, value, status, group) {
    if (value) {                                                // Button press.
        this.inSetValue(true);
        return;
    }                                                   // Else: button release.

    if (!this.doubleTapped) {
        this.inSetValue(false);
    }

    // Work-around LED disabling itself on release.
    this.trigger();

    this.doubleTapped = true;

    if (this.doubleTapTimer) {
        engine.stopTimer(this.doubleTapTimer);
        this.doubleTapTimer = null;
    }

    this.doubleTapTimer = engine.beginTimer(
        this.doubleTapTimeout,
        function () {
            this.doubleTapped = false;
            this.doubleTapTimer = null;
        },
        true
    );
};


DJ505.PadSection = function (deck, offset) {
    // TODO: Add support for missing modes (flip, cueloop, slicer, slicerloop, pitchplay, velocitysampler)
    /*
     * The Performance Pad Section on the DJ-505 apparently have two basic
     * modes of operation that determines how the LEDs react to MIDI messages
     * and button presses.
     *
     * - Standalone mode:
     *   When the DJ-505 is not connected to a computer (or no SysEx/Keep-Alive
     *   messages are sent [see below]), the controller's performance pads
     *   allow setting various "modes" using the mode buttons and the shift
     *   modifier. Pressing the mode buttons will change their LED color (and
     *   makes the performance pads barely lit in that color, too). However,
     *   it the Mode colors differ from that in the Owner's manual. Also, it
     *   does not seem to be possible to actually illuminate the performance
     *   pad LEDs - neither by pressing the button nor by sending MIDI messages
     *   to the device.
     *
     * - Serato mode:
     *   When the DJ-505 receives a SysEx message, the performance pads are put
     *   in "Serato" mode. In this mode, the pressing Pad mode buttons will not
     *   change their color. Instead, all LEDs have to be controlled by sending
     *   MIDI messages. Unlike the Standalone mode, it is also possible to
     *   control the pad LEDs.  However, in order to keep the DJ-505 in "Serato
     *   mode", it seems to be necessary to regularily send a certain
     *   "keep-alive" MIDI message (0xBF 0x64 0x00). Otherwise the device will
     *   switch back to "Standalone mode" after approximately 1.5 seconds.
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
     * The Pad and Mode Butttons support 31 different LED states:
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
     *   0x07       Light Blue     0x17       Light Blue (Dim)
     *   0x08       Purple         0x18       Purple (Dim)
     *   0x09       Apricot        0x19       Apricot (Dim)
     *   0x0A       Coral          0x1A       Coral (Dim)
     *   0x0B       Azure          0x1B       Azure (Dim)
     *   0x0C       Turquoise      0x1C       Turquoise (Dim)
     *   0x0C       Aquamarine     0x1C       Aquamarine (Dim)
     *   0x0E       Green          0x1E       Green (Dim)
     *   0x0F       White          0x1F       White (Dim)
     */
    components.ComponentContainer.call(this);
    this.modes = {
        // This need to be an object so that a recursive reconnectComponents
        // call won't influence all modes at once
        "hotcue": new DJ505.HotcueMode(deck, offset),
        "cueloop": new DJ505.CueLoopMode(deck, offset),
        "roll": new DJ505.RollMode(deck, offset),
        "loop": new DJ505.LoopMode(deck, offset),
        "sampler": new DJ505.SamplerMode(deck, offset),
        "velocitysampler": new DJ505.VelocitySamplerMode(deck, offset),
        "pitchplay": new DJ505.PitchPlayMode(deck, offset),
    };
    this.offset = offset;

    // Start in Hotcue Mode and disable other LEDs
    this.setPadMode(0x00);
    midi.sendShortMsg(0x94 + offset, this.modes["roll"].ledControl, 0x00);
    midi.sendShortMsg(0x94 + offset, this.modes["sampler"].ledControl, 0x00);
};

DJ505.PadSection.prototype = Object.create(components.ComponentContainer.prototype);

DJ505.PadSection.prototype.resetLEDs = function() {
    var mode = this.controlToPadMode(this.padMode);
    if (mode) {
        mode.forEachComponent(function(component) {
            component.trigger();
        });
    } else {
        for (var i = 0; i <= 7; i++) {
            midi.sendShortMsg(0x94 + this.offset, 0x14 + i, 0x00);
        }
    }
};

DJ505.PadSection.prototype.controlToPadMode = function (control) {
    var mode;
    switch(control) {
    case 0x00:  // Hot Cue Mode
        mode = this.modes["hotcue"];
        break;
    //case 0x02:  // Flip Mode
    //    mode = this.modes["flip"];
    //    break;
    case 0x03:  // Cue Loop Mode
        mode = this.modes["cueloop"];
        break;
    case 0x04:  // TR Mode
    case 0x05:  // Pattern Mode
    case 0x06:  // TR Velocity Mode
        // All of these are hardcoded in hardware
        mode = null;
        break;
    case 0x08:  // Roll Mode
        mode = this.modes["roll"];
        break;
    //case 0x09:
    //    mode = this.modes["slicer"];
    //    break;
    //case 0x0A:
    //    mode = this.modes["slicerloop"];
    //    break;
    case 0x0B:
        mode = this.modes["sampler"];
        break;
    case 0x0C:
        mode = this.modes["velocitysampler"];
        break;
    case 0x0D:
        mode = this.modes["loop"];
        break;
    case 0x0F:
        mode = this.modes["pitchplay"];
        break;
    }

    return mode;
};

DJ505.PadSection.prototype.padModeButtonPressed = function (channel, control, value, status, group) {
    this.setPadMode(control);
};

DJ505.PadSection.prototype.paramButtonPressed = function (channel, control, value, status, group) {
    var mode = this.controlToPadMode(this.padMode);
    if (mode && mode.paramPlusButton && mode.paramMinusButton) {
        if (control & 1) {
            mode.paramPlusButton.input(channel, control, value, status, group);
        } else {
            mode.paramMinusButton.input(channel, control, value, status, group);
        }
    }
};

DJ505.PadSection.prototype.setPadMode = function (control) {
    if (this.padMode == control) {
        return;
    }

    var mode = this.controlToPadMode(control);
    if (mode === undefined) {
        return;
    }

    // Disable previous mode's LED
    var previous_mode = this.controlToPadMode(this.padMode);
    if (previous_mode) {
        midi.sendShortMsg(0x94 + this.offset, previous_mode.ledControl, 0x00);
        previous_mode.forEachComponent(function (component) {
            component.disconnect();
        });
    }

    this.padMode = control;
    if (mode) {
        // Set new mode's LED
        midi.sendShortMsg(0x94 + this.offset, mode.ledControl, mode.color);
        mode.forEachComponent(function (component) {
            component.connect();
            component.trigger();
        });
    } else {
        this.resetLEDs();
    }
};

DJ505.PadSection.prototype.padPressed = function (channel, control, value, status, group) {
    var i = control - ((control >= 0x1C) ? 0x1C : 0x14);
    var mode = this.controlToPadMode(this.padMode);
    if (mode) {
        mode.pads[i].input(channel, control, value, status, group);
    }
};

DJ505.PadSection.prototype.forEachComponent = function (operation, recursive) {
    components.ComponentContainer.prototype.forEachComponent.apply(this, arguments);
    var mode = this.controlToPadMode(this.padMode);
    if (mode) {
        mode.forEachComponent(operation, recursive);
    }
};

DJ505.HotcueMode = function (deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = 0x00;
    this.color = 0x0F;
    this.pads = [];
    for (var i = 0; i <= 7; i++) {
        this.pads[i] = new components.HotcueButton({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i + 1,
            group: deck.currentDeck,
            on: i + 1,
            off: this.color + 0x10,
            outConnect: false,
        });
    }
};
DJ505.HotcueMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ505.CueLoopMode = function (deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = 0x00;
    this.color = 0x03;
    this.pads = [];
    for (var i = 0; i <= 7; i++) {
        this.pads[i] = new components.Button({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i + 1,
            group: deck.currentDeck,
            outKey: "hotcue_" + (i + 1) + "_enabled",
            on: i + 1,
            off: this.color + 0x10,
            outConnect: false,
            unshift: function() {
                this.input = function (channel, control, value, status, group) {
                    if (value) {
                        var enabled = true;
                        if (!engine.getValue(group, "hotcue_" + this.number + "_enabled")) {
                            // set a new cue point and loop
                            enabled = false;
                            script.triggerControl(group, "hotcue_" + this.number + "_activate");
                        }
                        // jump to existing cue and loop
                        var startpos = engine.getValue(group, "hotcue_" + this.number + "_position");
                        var loopseconds = engine.getValue(group, "beatloop_size") * (1 / (engine.getValue(group, "bpm") / 60));
                        var loopsamples = loopseconds * engine.getValue(group, "track_samplerate") * 2;
                        var endpos = startpos + loopsamples;

                        // disable loop if currently enabled
                        if (engine.getValue(group, "loop_enabled")) {
                            script.triggerControl(group, "reloop_toggle", 1);
                            if (enabled && engine.getValue(group, "loop_start_position") == startpos && engine.getValue(group, "loop_end_position") == endpos) {
                                return;
                            }
                        }
                        // set start and endpoints
                        engine.setValue(group, "loop_start_position", startpos);
                        engine.setValue(group, "loop_end_position", endpos);
                        // enable loop
                        script.triggerControl(group, "reloop_toggle", 1);
                        if (enabled) {
                            script.triggerControl(group, "loop_in_goto", 1);
                        }
                    }
                };
            },
            shift: function() {
                this.input = function (channel, control, value, status, group) {
                    if (value) {
                        if (engine.getValue(group, "hotcue_" + this.number + "_enabled")) {
                            // jump to existing cue and loop
                            var startpos = engine.getValue(group, "hotcue_" + this.number + "_position");
                            var loopseconds = engine.getValue(group, "beatloop_size") * (1 / (engine.getValue(group, "bpm") / 60));
                            var loopsamples = loopseconds * engine.getValue(group, "track_samplerate") * 2;
                            var endpos = startpos + loopsamples;

                            if (engine.getValue(group, "loop_enabled") && engine.getValue(group, "loop_start_position") == startpos && engine.getValue(group, "loop_end_position") == endpos) {
                                engine.setValue(group, "reloop_toggle", 1);
                            } else {
                                script.triggerControl(group, "hotcue_" + this.number + "_clear");
                            }
                        }
                    }
                };
            },
        });
    }
};
DJ505.CueLoopMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ505.RollMode = function (deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = 0x08;
    this.color = 0x07;
    this.pads = [];
    this.loopSize = 0.03125;

    var loopSize;
    for (var i = 0; i <= 7; i++) {
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
            off: this.color + ((loopSize == 4) ? 0x01 : 0x10),
        });
    }
    this.paramMinusButton = new components.Button({
        midi: [0x94 + offset, 0x28],
        mode: this,
        input: function (channel, control, value, status, group) {
            if (value) {
                if (this.mode.loopSize == 0.03125) {
                    this.mode.setLoopSize(0.25);
                } else {
                    this.mode.setLoopSize(this.mode.loopSize / 2);
                }
            }
            this.send(value);
        },
    });
    this.paramPlusButton = new components.Button({
        midi: [0x94 + offset, 0x29],
        mode: this,
        input: function (channel, control, value, status, group) {
            if (value) {
                if (this.mode.loopSize == 0.25) {
                    this.mode.setLoopSize(0.03125);
                } else {
                    this.mode.setLoopSize(this.mode.loopSize * 2);
                }
            }
            this.send(value);
        },
    });
};
DJ505.RollMode.prototype = Object.create(components.ComponentContainer.prototype);
DJ505.RollMode.prototype.setLoopSize = function (loopSize) {
    this.loopSize = loopSize;
    var padLoopSize;
    for (var i = 0; i <= 7; i++) {
        padLoopSize = (this.loopSize * Math.pow(2, i));
        this.pads[i].inKey = "beatlooproll_" + padLoopSize + "_activate";
        this.pads[i].outKey = "beatloop_" + padLoopSize + "_enabled";
        this.pads[i].off = this.color + ((padLoopSize == 4) ? 0x01 : 0x10);
    }
    this.reconnectComponents();
};

DJ505.LoopMode = function (deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = 0x08;
    this.color = 0x0E;
    this.pads = [];
    for (var i = 0; i <= 7; i++) {
        this.pads[i] = new components.Button({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            group: deck.currentDeck,
            outKey: "beatloop_" + (0.03125 * Math.pow(2, i)) + "_enabled",
            inKey: "beatloop_" + (0.03125 * Math.pow(2, i)) + "_toggle",
            outConnect: false,
            on: this.color,
            off: this.color + 0x10,
        });
    }
};
DJ505.LoopMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ505.SamplerMode = function (deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = 0x0B;
    this.color = 0x06;
    this.pads = [];
    for (var i = 0; i <= 7; i++) {
        this.pads[i] = new components.SamplerButton({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i + 1,
            outConnect: false,
            on: this.color,
            off: this.color + 0x10,
        });
    }
};
DJ505.SamplerMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ505.VelocitySamplerMode = function (deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = 0x0B;
    this.color = 0x08;
    this.pads = [];
    for (var i = 0; i <= 7; i++) {
        this.pads[i] = new components.SamplerButton({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i + 1,
            outConnect: false,
            on: this.color,
            off: this.color + 0x10,
            volumeByVelocity: true,
        });
    }
};
DJ505.VelocitySamplerMode.prototype = Object.create(components.ComponentContainer.prototype);

DJ505.PitchPlayMode = function (deck, offset) {
    const RANGE_UP = 0;
    const RANGE_MID = 1;
    const RANGE_DOWN = 2;

    components.ComponentContainer.call(this);
    this.ledControl = 0x0B;
    this.color = 0x0E;
    this.pads = [];
    this.cuepoint = 1;
    this.range = RANGE_MID;
    for (var i = 0; i <= 7; i++) {
        this.pads[i] = new components.Button({
            midi: [0x94 + offset, 0x14 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            group: deck.currentDeck,
            mode: this,
            number: i + 1,
            outConnect: false,
            on: this.number + 0x10,
            off: 0x00,
            unshift: function() {
                this.outKey = "key";
                this.output = function (value, group, control) {
                    var color = this.mode.color + 0x10;
                    if ((this.mode.range == RANGE_UP && this.number == 5) ||
                        (this.mode.range == RANGE_MID && this.number == 1) ||
                        (this.mode.range == RANGE_DOWN && this.number == 4)) {
                        color = 0x0F;
                    }
                    this.send(color);
                };
                this.input = function (channel, control, value, status, group) {
                    if (value > 0) {
                        var fileKey = engine.getValue(group, "file_key");
                        var keyModifier;
                        switch(this.mode.range) {
                        case RANGE_UP:
                            keyModifier = this.number + ((this.number <= 4) ? 4 : -5);
                            break;
                        case RANGE_MID:
                            keyModifier = this.number - ((this.number <= 4) ? 1 : 9);
                            break;
                        case RANGE_DOWN:
                            keyModifier = this.number - ((this.number <= 4) ? 4 : 12);
                        }
                        engine.setValue(this.group, "key", fileKey + keyModifier);
                        engine.setValue(this.group, "hotcue_" + this.mode.cuepoint + "_activate", value);
                    }
                };
                this.disconnect();
                this.connect();
                this.trigger();
            },
            shift: function() {
                this.outKey = "hotcue_" + this.number + "_enabled";
                this.output = function (value, group, control) {
                    if (value) {
                        this.send((value && this.mode.cuepoint === this.number) ? this.number : (this.number + 0x10));
                    } else {
                        this.send(0x00);
                    }
                };
                this.input = function (channel, control, value, status, group) {
                    if (value > 0 && this.mode.cuepoint != this.number && engine.getValue(this.group, "hotcue_" + this.number + "_enabled")) {
                        var previous_cuepoint = this.mode.cuepoint;
                        this.mode.cuepoint = this.number;
                        this.mode.pads[previous_cuepoint - 1].trigger();
                        this.send(this.number);
                    }
                };
                this.disconnect();
                this.connect();
                this.trigger();
            },
        });
    }
    this.paramMinusButton = new components.Button({
        midi: [0x94 + offset, 0x28],
        mode: this,
        input: function (channel, control, value, status, group) {
            if (value) {
                if (this.mode.range == RANGE_UP) {
                    this.mode.range = RANGE_MID;
                } else if (this.mode.range == RANGE_MID) {
                    this.mode.range = RANGE_DOWN;
                } else {
                    this.mode.range = RANGE_UP;
                }
                this.mode.forEachComponent(function (component) {
                    component.trigger();
                });
            }
            this.send(value);
        },
    });
    this.paramPlusButton = new components.Button({
        midi: [0x94 + offset, 0x29],
        mode: this,
        input: function (channel, control, value, status, group) {
            if (value) {
                if (this.mode.range == RANGE_UP) {
                    this.mode.range = RANGE_DOWN;
                } else if (this.mode.range == RANGE_MID) {
                    this.mode.range = RANGE_UP;
                } else {
                    this.mode.range = RANGE_MID;
                }
                this.mode.forEachComponent(function (component) {
                    component.trigger();
                });
            }
            this.send(value);
        },
    });
};
DJ505.PitchPlayMode.prototype = Object.create(components.ComponentContainer.prototype);
