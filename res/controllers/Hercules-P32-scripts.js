// USER CONFIGURABLE OPTIONS
// The labels on the encoders are mirrored, but the rest of the controller
// is asymmetrical. If this is confusing for you to use, set this to "false" to swap the
// mapping of the encoders on the right deck so the whole controller is asymmetrical.
var mirroredEncoders = true;
// Set this to "false" to be able to set the loop and beatjump sizes above 64 beats
// to values that cannot be shown on the controller's LED display.
var clampLoopAndBeatJumpSize = true;
// Set to "true" to use the dot on the loop size LED display to indicate
// that a loop is active. This restricts loop sizes to 2-32 beats and
// may be helpful if you never use loops less than 2 beats long.
// Otherwise the dot indicates a loop size equal to 1/(# on the LED display).
var loopEnabledDot = false;
// Assign samplers on left side of the controller to left side of the crossfader
// and samplers on right side of the controller to the right side of the crossfader
var samplerCrossfaderAssign = true;
// Toggle effect units between 1 & 3 on left and 2 & 4 on right when toggling decks
var toggleEffectUnitsWithDecks = false;

/**
 * Hercules P32 DJ controller script for Mixxx 2.1
 * Thanks to Hercules for supporting the development of this mapping by providing a controller
 * Refer to http://mixxx.org/wiki/doku.php/hercules_p32_dj for instructions on how to use this mapping
 *
 * Copyright (C) 2017 Be <be.0@gmx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/
var P32 = {};

P32.init = function() {
    components.Component.prototype.shiftOffset = 3;
    components.Component.prototype.shiftChannel = true;
    components.Button.prototype.sendShifted = true;

    if (engine.getValue("[Master]", "num_samplers") < 32) {
        engine.setValue("[Master]", "num_samplers", 32);
    }

    P32.leftDeck = new P32.Deck([1, 3], 1);
    P32.rightDeck = new P32.Deck([2, 4], 2);

    // tell controller to send MIDI messages with positions of faders and knobs
    midi.sendShortMsg(0xB0, 0x7F, 0x7F);
};

P32.shutdown = function() {
    for (var channel = 0; channel <= 5; channel++) {
        for (var button = 1; button <= 0x63; button++) {
            midi.sendShortMsg(0x90 + channel, button, 0);
        }
    }
    // TODO: ask Hercules if it is possible to clear the loop size LEDs
};

P32.shiftOffset = 3;

P32.padColors = {
    red: 125,
    blue: 126,
    purple: 127,
    off: 0
};

P32.PadNumToMIDIControl = function(PadNum, layer) {
    // The MIDI control numbers for the pad grid are numbered bottom to top, so
    // this returns the MIDI control numbers for the pads numbered top to bottom
    // layer argument is the 0-indexed pad mode, from bottom (sampler) to top (hotcue)
    PadNum -= 1;
    var midiRow = 3 - Math.floor(PadNum/4);
    return 0x24 + 16 * layer + midiRow*4 + PadNum%4;
};

P32.browseEncoder = function(_channel, _control, value, _status, _group) {
    if (value > 64) {
        engine.setValue("[Playlist]", "SelectPrevTrack", 1);
    } else {
        engine.setValue("[Playlist]", "SelectNextTrack", 1);
    }
};

P32.headMixEncoder = function(_channel, _control, value, _status, _group) {
    var direction = (value > 64) ? -1 : 1;
    engine.setValue("[Master]", "headMix", engine.getValue("[Master]", "headMix") + (0.25 * direction));
};

P32.recordButton = new components.Button({
    midi: [0x90, 0x02],
    group: "[Recording]",
    inKey: "toggle_recording",
    outKey: "status",
    sendShifted: false,
});

P32.slipButton = new components.Button({
    midi: [0x90, 0x03],
    pressedToToggleDeck: false,
    input: function(_channel, _control, value, _status, _group) {
        if (P32.leftDeck.isShifted && value === 127) {
            // PFL button is controlling effect unit assignment to headphones while
            // shift is pressed, so switch it back to controlling PFL so
            // reconnecting the output works.
            P32.leftDeck.pfl.unshift();
            P32.leftDeck.toggle();
            if (toggleEffectUnitsWithDecks) {
                P32.leftDeck.effectUnit.toggle();
            }
            this.pressedToToggleDeck = true;
        } else if (P32.rightDeck.isShifted && value === 127) {
            // PFL button is controlling effect unit assignment to headphones while
            // shift is pressed, so switch it back to controlling PFL so
            // reconnecting the output works.
            P32.rightDeck.pfl.unshift();
            P32.rightDeck.toggle();
            if (toggleEffectUnitsWithDecks) {
                P32.rightDeck.effectUnit.toggle();
            }
            this.pressedToToggleDeck = true;
        } else {
            if (this.pressedToToggleDeck && value === 0) {
                this.pressedToToggleDeck = false;
            } else {
                for (var i = 1; i <= 4; i++) {
                    script.toggleControl("[Channel" + i + "]", "slip_enabled");
                }
            }
        }
    },
    connect: function() {
        for (var d = 1; d <= 4; d++) {
            this.connections.push(
                engine.connectControl("[Channel" + d + "]", "slip_enabled", this.output.bind(this))
            );
        }
    },
    output: function(_value, _group, _control) {
        var slipEnabledOnAnyDeck = false;
        for (var d = 1; d <= 4; d++) {
            if (engine.getValue("[Channel" + d + "]", "slip_enabled")) {
                slipEnabledOnAnyDeck = true;
                break;
            }
        }
        this.send(slipEnabledOnAnyDeck ? this.on : this.off);
    },
    key: "slip_enabled",
    sendShifted: false,
    group: null // hack to get Component constructor to call this.connect()
});

P32.Deck = function(deckNumbers, channel) {
    components.Deck.call(this, deckNumbers);

    var theDeck = this;

    this.shiftButton = function(_channel, _control, value, _status, _group) {
        if (value === 127) {
            this.shift();
        } else {
            this.unshift();
        }
    };

    this.loadTrack = new components.Button({
        midi: [0x90 + channel, 0x0F],
        unshift: function() {
            this.inKey = "LoadSelectedTrack";
        },
        shift: function() {
            this.inKey = "eject";
        },
    });

    // =============================== ENCODERS =========================================
    this.loopEncoder = new components.Encoder({
        // NOTE: these are the MIDI bytes for the digit LEDs, not input from the encoder.
        midi: [0xB0 + channel, 0x1B],
        unshift: function() {
            this.input = function(_channel, _control, value, _status, _group) {
                var loopSize = engine.getValue(this.group, "beatloop_size");
                if (loopEnabledDot) {
                    if (value > 64 && loopSize > 2) { // turn left
                        // Unfortunately, there is no way to show 1 with a dot on the
                        // loop size LED.
                        engine.setValue(this.group, "beatloop_size", loopSize / 2);
                    } else if (value < 64 && loopSize < 32) { // turn right
                        // Mixxx supports loops longer than 32 beats, but there is no way
                        // to show 64 with a dot on the loop size LED.
                        engine.setValue(this.group, "beatloop_size", loopSize * 2);
                    }
                } else {
                    if (value > 64 && loopSize > 1/32) { // turn left
                        engine.setValue(this.group, "beatloop_size", loopSize / 2);
                    } else if (value < 64) { // turn right
                        if (clampLoopAndBeatJumpSize) {
                            if (loopSize * 2 <= 64) {
                                engine.setValue(this.group, "beatloop_size", loopSize * 2);
                            }
                        } else {
                            engine.setValue(this.group, "beatloop_size", loopSize * 2);
                        }
                    }
                }
            };
        },
        shift: function() {
            this.input = function(_channel, _control, value, _status, _group) {
                var direction = (value > 64) ? "backward" : "forward";
                script.triggerControl(this.group, "beatjump_1_" + direction);
            };
        },
        connect: function() {
            this.connections[0] = engine.connectControl(this.group, "beatloop_size", this.output.bind(this));
            if (loopEnabledDot) {
                this.connections[1] = engine.connectControl(this.group, "loop_enabled", this.output.bind(this));
            }
        },
        output: function(_value, _group, _control) {
            var loopSize = engine.getValue(this.group, "beatloop_size");
            var loopSizeLogBase2 = Math.log(loopSize) / Math.log(2);
            // test if loopSizeLogBase2 is an integer
            if (Math.floor(loopSizeLogBase2) === loopSizeLogBase2) {
                if (loopEnabledDot && engine.getValue(this.group, "loop_enabled") === 1) {
                    this.send(5 - loopSizeLogBase2);
                } else {
                    this.send(5 + loopSizeLogBase2);
                }
            } else {
                this.send(14); // show two dots
            }
        }
    });

    this.loopEncoderPress = new components.Button({
        unshift: function() {
            // Make sure the shifted Controls don't get stuck with a value of 1
            // if the shift button is released before the encoder button.
            if (engine.getValue(this.group, "reloop_andstop") !== 0) {
                engine.setValue(this.group, "reloop_andstop", 0);
            }
            if (engine.getValue(this.group, "reloop_toggle") !== 0) {
                engine.setValue(this.group, "reloop_toggle", 0);
            }

            this.input = function(_channel, _control, value, _status, _group) {
                if (value) {
                    if (engine.getValue(this.group, "loop_enabled") === 1) {
                        engine.setValue(this.group, "reloop_toggle", 1);
                    } else {
                        engine.setValue(this.group, "beatloop_activate", 1);
                    }
                } else {
                    if (engine.getValue(this.group, "reloop_toggle") !== 1) {
                        engine.setValue(this.group, "reloop_toggle", 0);
                    } else if (engine.getValue(this.group, "beatloop_activate") !== 0) {
                        engine.setValue(this.group, "beatloop_activate", 0);
                    }
                }
            };
        },
        shift: function() {
            // Make sure the unshifted Controls don't get stuck with a value of 1
            // if the shift button is pressed before releasing the encoder button.
            if (engine.getValue(this.group, "reloop_toggle") !== 0) {
                engine.setValue(this.group, "reloop_toggle", 0);
            }
            if (engine.getValue(this.group, "beatloop_activate") !== 0) {
                engine.setValue(this.group, "beatloop_activate", 0);
            }

            this.input = function(_channel, _control, value, _status, _group) {
                if (engine.getValue(this.group, "loop_enabled") === 1) {
                    engine.setValue(this.group, "reloop_andstop", value / 127);
                } else {
                    engine.setValue(this.group, "reloop_toggle", value / 127);
                }
            };
        },
    });

    this.showBeatjumpSize = function() {
        var beatjumpSize = engine.getValue(this.currentDeck, "beatjump_size");
        var beatjumpSizeLogBase2 = Math.log(beatjumpSize) / Math.log(2);
        // test if beatjumpSizeLogBase2 is an integer
        if (Math.floor(beatjumpSizeLogBase2) === beatjumpSizeLogBase2) {
            midi.sendShortMsg(0xB0 + channel, 0x1B,
                5 + Math.log(beatjumpSize) / Math.log(2));
        } else {
            midi.sendShortMsg(0xB0 + channel, 0x1B, 14); // show two dots
        }
    };

    this.tempoAndBeatjumpEncoder = new components.Encoder({
        unshift: function() {
            this.input = function(_channel, _control, value, _status, _group) {
                var direction = (value > 64) ? -1 : 1;
                engine.setValue(this.group, "rate",
                    engine.getValue(this.group, "rate") + (0.01 * direction));
            };
        },
        shift: function() {
            this.input = function(_channel, _control, value, _status, _group) {
                var beatJumpSize = engine.getValue(this.group, "beatjump_size");
                if (theDeck.beatJumpEncoderPressed) {
                    if (value > 64 && beatJumpSize > 1/32) { // turn left
                        beatJumpSize /= 2;
                    } else if (value < 64) { // turn right
                        if (clampLoopAndBeatJumpSize && beatJumpSize >= 64) {
                            return;
                        }
                        beatJumpSize *= 2;
                    }
                    engine.setValue(this.group, "beatjump_size", beatJumpSize);
                    theDeck.showBeatjumpSize();
                } else {
                    var direction = (value > 64) ? "backward" : "forward";
                    script.triggerControl(this.group, "beatjump_" + direction);
                }
            };
        },
    });

    this.tempoAndBeatjumpEncoderPress = new components.Button({
        unshift: function() {
            theDeck.loopEncoder.trigger();
            this.input = function(_channel, _control, value, _status, _group) {
                if (value === 127) {
                    engine.setValue(this.group, "rate", 0);
                }
            };
        },
        shift: function() {
            this.input = function(_channel, _control, value, _status, _group) {
                if (value === 127) {
                    theDeck.beatJumpEncoderPressed = true;
                    theDeck.showBeatjumpSize();
                } else {
                    theDeck.beatJumpEncoderPressed = false;
                    theDeck.loopEncoder.trigger();
                }
            };
        },
    });

    if (mirroredEncoders) {
        if (channel === 1) { // left deck
            this.leftEncoder = this.loopEncoder;
            this.leftEncoderPress = this.loopEncoderPress;
            this.rightEncoder = this.tempoAndBeatjumpEncoder;
            this.rightEncoderPress = this.tempoAndBeatjumpEncoderPress;
        } else if (channel === 2) { // right deck
            this.leftEncoder = this.tempoAndBeatjumpEncoder;
            this.leftEncoderPress = this.tempoAndBeatjumpEncoderPress;
            this.rightEncoder = this.loopEncoder;
            this.rightEncoderPress = this.loopEncoderPress;
        }
    } else {
        this.leftEncoder = this.loopEncoder;
        this.leftEncoderPress = this.loopEncoderPress;
        this.rightEncoder = this.tempoAndBeatjumpEncoder;
        this.rightEncoderPress = this.tempoAndBeatjumpEncoderPress;
    }

    // ================================= EFFECTS =====================================
    this.effectUnit = new components.EffectUnit(deckNumbers);
    this.effectUnit.knobs[1].midi = [0xB0 + channel, 0x06];
    this.effectUnit.knobs[2].midi = [0xB0 + channel, 0x07];
    this.effectUnit.knobs[3].midi = [0xB0 + channel, 0x08];
    this.effectUnit.dryWetKnob.midi = [0xB0 + channel, 0x09];
    this.effectUnit.enableButtons[1].midi = [0x90 + channel, 0x03];
    this.effectUnit.enableButtons[2].midi = [0x90 + channel, 0x04];
    this.effectUnit.enableButtons[3].midi = [0x90 + channel, 0x05];
    this.effectUnit.effectFocusButton.midi = [0x90 + channel, 0x06];
    this.effectUnit.init();

    // ================================ PAD GRID ====================================
    this.hotcueButton = [];
    this.samplerButton = [];
    for (var i = 1; i <= 16; i++) {
        this.hotcueButton[i] = new components.HotcueButton({
            midi: [
                0x90 + channel,
                P32.PadNumToMIDIControl(i, 3)
            ],
            number: i,
            on: P32.padColors.red
        });

        var row = Math.ceil(i/4);
        var column = ((i-1) % 4) + 1;
        var padGrid = channel - 1;
        var samplerNumber = (8 * (row-1)) + (column) + (padGrid * 4);
        this.samplerButton[samplerNumber] = new components.SamplerButton({
            midi: [0x90 + channel, P32.PadNumToMIDIControl(i, 0)],
            number: samplerNumber,
            empty: P32.padColors.off,
            loaded: P32.padColors.red,
            playing: P32.padColors.blue,
            looping: P32.padColors.purple,
        });
        if (samplerCrossfaderAssign) {
            engine.setValue(
                "[Sampler" + samplerNumber + "]",
                "orientation",
                (channel === 1) ? 0 : 2
            );
        }
    }

    // LOOP layer
    this.loopIn = new components.Button({
        midi: [0x90 + channel, 0x50],
        key: "loop_in",
        on: P32.padColors.red,
        off: P32.padColors.purple,
    });
    this.loopOut = new components.Button({
        midi: [0x90 + channel, 0x51],
        key: "loop_out",
        on: P32.padColors.red,
        off: P32.padColors.purple,
    });
    this.reloop = new components.Button({
        midi: [0x90 + channel, 0x52],
        unshift: function() {
            this.inKey = "reloop_toggle";
        },
        shift: function() {
            this.inKey = "reloop_andstop";
        },
        on: P32.padColors.red,
        off: P32.padColors.blue,
        outKey: "loop_enabled",
    });

    this.tempSlow = new components.Button({
        midi: [0x90 + channel, 0x44],
        key: "rate_temp_down",
        on: P32.padColors.red,
        off: P32.padColors.purple,
    });
    this.tempFast = new components.Button({
        midi: [0x90 + channel, 0x45],
        key: "rate_temp_up",
        on: P32.padColors.red,
        off: P32.padColors.purple,
    });
    this.alignBeats = new components.Button({
        midi: [0x90 + channel, 0x46],
        key: "beats_translate_curpos",
        on: P32.padColors.red,
        off: P32.padColors.blue,
    });

    this.syncKey = new components.Button({
        midi: [0x90 + channel, 0x53],
        key: "sync_key",
        on: P32.padColors.blue,
        off: P32.padColors.red,
    });
    this.pitchUp = new components.Button({
        midi: [0x90 + channel, 0x4F],
        key: "pitch_up",
        on: P32.padColors.purple,
        off: P32.padColors.red,
    });
    this.pitchDown = new components.Button({
        midi: [0x90 + channel, 0x4B],
        key: "pitch_down",
        on: P32.padColors.purple,
        off: P32.padColors.red,
    });
    this.resetKey = new components.Button({
        midi: [0x90 + channel, 0x47],
        key: "reset_key",
        on: P32.padColors.blue,
        off: P32.padColors.red,
    });

    // SLICER layer
    this.enableEffectUnitButtons = [0x40, 0x41, 0x3C, 0x3D].map(
        function(midiByte, index) {
            return new components.EffectAssignmentButton({
                midi: [0x90 + channel, midiByte],
                effectUnit: index + 1,
                group: this.currentDeck,
                on: P32.padColors.blue,
                off: P32.padColors.red,
            });
        },
        this
    );

    // ============================= TRANSPORT ==================================
    this.sync = new components.SyncButton([0x90 + channel, 0x08]);
    this.cue = new components.CueButton([0x90 + channel, 0x09]);
    this.play = new components.PlayButton([0x90 + channel, 0x0A]);

    // =============================== MIXER ====================================
    this.eqKnob = [];
    for (var k = 1; k <= 3; k++) {
        this.eqKnob[k] = new components.Pot({
            midi: [0xB0 + channel, 0x02 + k],
            group: "[EqualizerRack1_" + this.currentDeck + "_Effect1]",
            inKey: "parameter" + k,
        });
    }

    this.pfl = new components.Button({
        midi: [0x90 + channel, 0x10],
        sendShifted: false,
        type: components.Button.prototype.types.toggle,
        unshift: function() {
            this.group = theDeck.currentDeck;
            this.inKey = "pfl";
        },
        outKey: "pfl",
        shift: function() {
            this.group = "[EffectRack1_EffectUnit" + theDeck.effectUnit.currentUnitNumber + "]";
            this.inKey = "group_[Headphone]_enable";
        },
    });

    this.volume = new components.Pot({
        midi: [0xB0 + channel, 0x01],
        inKey: "volume",
    });

    this.reconnectComponents(function(component) {
        if (component.group === undefined) {
            component.group = this.currentDeck;
        }
    });
};
P32.Deck.prototype = new components.Deck();
