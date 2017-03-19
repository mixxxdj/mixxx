// USER CONFIGURABLE OPTIONS
// Set to "true" to use the dot on the loop size LED display to indicate
// that a loop is active. This restricts loop sizes to 2-32 beats and
// may be helpful if you never use loops less than 2 beats long.
// Otherwise the dot indicates a loop size equal to 1/(# on the LED display).
var loopEnabledDot = false;
// Assign samplers on left side of the controller to left side of the crossfader
// and samplers on right side of the controller to the right side of the crossfader
var samplerCrossfaderAssign = true;

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

P32.init = function () {
    components.Component.prototype.shiftOffset = 3;
    components.Component.prototype.shiftChannel = true;
    components.Button.prototype.sendShifted = true;

    /**
    The P32 has encoders for changing tempo, so the actual tempo getting out of sync with a hardware
    fader and dealing with soft takeover in that situation is not an issue. So, make toggling master
    sync the default unshifted behavior and momentary sync the shifted behavior.
    **/
    components.SyncButton.prototype.unshift = function () {
        this.inKey = 'sync_enabled';
    };
    components.SyncButton.prototype.shift = function () {
        this.inKey = 'beatsync';
    };

    if (engine.getValue('[Master]', 'num_samplers') < 32) {
        engine.setValue('[Master]', 'num_samplers', 32);
    }

    P32.leftDeck = new P32.Deck([1,3], 1);
    P32.rightDeck = new P32.Deck([2,4], 2);

    // tell controller to send MIDI messages with positions of faders and knobs
    midi.sendShortMsg(0xB0, 0x7F, 0x7F);
};

P32.shutdown = function () {
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

P32.PadNumToMIDIControl = function (PadNum, layer) {
    // The MIDI control numbers for the pad grid are numbered bottom to top, so
    // this returns the MIDI control numbers for the pads numbered top to bottom
    // layer argument is the 0-indexed pad mode, from bottom (sampler) to top (hotcue)
    PadNum -= 1;
    var midiRow = 3 - Math.floor(PadNum/4);
    return 0x24 + 16 * layer + midiRow*4 + PadNum%4;
};

P32.browseEncoder = function (channel, control, value, status, group) {
    if (value > 64) {
        engine.setValue('[Playlist]', 'SelectPrevTrack', 1);
    } else {
        engine.setValue('[Playlist]', 'SelectNextTrack', 1);
    }
};

P32.headMixEncoder = function (channel, control, value, status, group) {
    var direction = (value > 64) ? -1 : 1;
    engine.setValue('[Master]', 'headMix', engine.getValue('[Master]', 'headMix') + (0.25 * direction));
};

P32.recordButton = new components.Button({
    midi: [0x90, 0x02],
    group: '[Recording]',
    inKey: 'toggle_recording',
    onlyOnPress: false,
    outKey: 'status',
    sendShifted: false,
});

P32.slipButton = new components.Button({
    midi: [0x90, 0x03],
    pressedToToggleDeck: false,
    input: function (channel, control, value, status, group) {
        if (P32.leftDeck.isShifted && value === 127) {
            // PFL button is controlling effect unit assignment to headphones while
            // shift is pressed, so switch it back to controlling PFL so
            // reconnecting the output works.
            P32.leftDeck.pfl.unshift();
            P32.leftDeck.toggle();
            this.pressedToToggleDeck = true;
        } else if (P32.rightDeck.isShifted && value === 127) {
            // PFL button is controlling effect unit assignment to headphones while
            // shift is pressed, so switch it back to controlling PFL so
            // reconnecting the output works.
            P32.rightDeck.pfl.unshift();
            P32.rightDeck.toggle();
            this.pressedToToggleDeck = true;
        } else {
            if (this.pressedToToggleDeck && value === 0) {
                this.pressedToToggleDeck = false;
            } else {
                for (var i = 1; i <= 4; i++) {
                    script.toggleControl('[Channel' + i + ']', 'slip_enabled');
                }
            }
        }
    },
    connect: function () {
        for (var d = 1; d <= 4; d++) {
            this.connections.push(
                engine.connectControl('[Channel' + d + ']', 'slip_enabled', this.output)
            );
        }
    },
    output: function (value, group, control) {
        var slipEnabledOnAnyDeck = false;
        for (var d = 1; d <= 4; d++) {
            if (engine.getValue('[Channel' + d + ']', 'slip_enabled')) {
                slipEnabledOnAnyDeck = true;
                break;
            }
        }
        this.send(slipEnabledOnAnyDeck ? this.on : this.off);
    },
    key: 'slip_enabled',
    sendShifted: false,
    group: null // hack to get Component constructor to call this.connect()
});

P32.Deck = function (deckNumbers, channel) {
    components.Deck.call(this, deckNumbers);

    var theDeck = this;

    this.shiftButton = function (channel, control, value, status, group) {
        if (value === 127) {
            this.shift();
        } else {
            this.unshift();
        }
    };

    // ===================================== TRANSPORT =========================================
    this.sync = new components.SyncButton([0x90 + channel, 0x08]);
    this.cue = new components.CueButton([0x90 + channel, 0x09]);
    this.play = new components.PlayButton([0x90 + channel, 0x0A]);

    // ===================================== MIXER ==============================================
    this.eqKnob = [];
    for (var k = 1; k <= 3; k++) {
        this.eqKnob[k] = new components.Pot({
            midi: [0xB0 + channel, 0x02 + k],
            group: '[EqualizerRack1_' + this.currentDeck + '_Effect1]',
            inKey: 'parameter' + k,
        });
    }

    this.pfl = new components.Button({
        midi: [0x90 + channel, 0x10],
        sendShifted: false,
        unshift: function () {
            this.group = theDeck.currentDeck;
            this.inKey = 'pfl';
        },
        outKey: 'pfl',
        shift: function () {
            this.group = '[EffectRack1_EffectUnit' + theDeck.effectUnit.currentUnitNumber + ']';
            this.inKey = 'group_[Headphones]_enable';
        },
    });

    this.volume = new components.Pot({
        midi: [0xB0 + channel, 0x01],
        inKey: 'volume',
    });

    // ==================================== PAD GRID ============================================
    // The slicer layer is handled by this.effectUnit.enableOnChannelButtons, set up under the
    // EFFECTS section.

    this.hotcueButton = [];
    this.samplerButton = [];
    for (var i = 1; i <= 16; i++) {
        this.hotcueButton[i] = new components.HotcueButton({
            midi: [0x90 + channel,
                   P32.PadNumToMIDIControl(i, 3)],
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
            engine.setValue('[Sampler' + samplerNumber + ']',
                            'orientation',
                            (channel === 1) ? 0 : 2
            );
        }
    }

    this.loopIn = new components.Button({
        midi: [0x90 + channel, 0x50],
        inKey: 'loop_in',
    });
    this.loopOut = new components.Button({
        midi: [0x90 + channel, 0x51],
        inKey: 'loop_out',
    });
    this.loopTogglePad = new components.LoopToggleButton({
        midi: [0x90 + channel, 0x52],
        on: P32.padColors.red,
        off: P32.padColors.blue,
    });
    this.loopIn.send(P32.padColors.purple);
    this.loopOut.send(P32.padColors.purple);

    this.tempSlow = new components.Button({
        midi: [0x90 + channel, 0x44],
        inKey: 'rate_temp_down',
        onlyOnPress: false,
    });
    this.tempFast = new components.Button({
        midi: [0x90 + channel, 0x45],
        inKey: 'rate_temp_up',
        onlyOnPress: false,
    });
    this.alignBeats = new components.Button({
        midi: [0x90 + channel, 0x46],
        inKey: 'beats_translate_curpos',
    });
    this.quantize = new components.Button({
        midi: [0x90 + channel, 0x47],
        key: 'quantize',
        on: P32.padColors.red,
        off: P32.padColors.blue,
    });
    this.tempSlow.send(P32.padColors.purple);
    this.tempFast.send(P32.padColors.purple);
    this.alignBeats.send(P32.padColors.blue);

    this.enableEffectUnitButtons = new components.ComponentContainer(); //fii
    this.enableEffectUnitButtons[1] = new components.EffectAssignmentButton({
        midi: [0x90 + channel, 0x40],
        effectUnit: 1,
        group: this.currentDeck,
        on: P32.padColors.blue,
        off: P32.padColors.red,
    });
    this.enableEffectUnitButtons[2] = new components.EffectAssignmentButton({
        midi: [0x90 + channel, 0x41],
        effectUnit: 2,
        group: this.currentDeck,
        on: P32.padColors.blue,
        off: P32.padColors.red,
    });
    this.enableEffectUnitButtons[3] = new components.EffectAssignmentButton({
        midi: [0x90 + channel, 0x3C],
        effectUnit: 3,
        group: this.currentDeck,
        on: P32.padColors.blue,
        off: P32.padColors.red,
    });
    this.enableEffectUnitButtons[4] = new components.EffectAssignmentButton({
        midi: [0x90 + channel, 0x3D],
        effectUnit: 4,
        group: this.currentDeck,
        on: P32.padColors.blue,
        off: P32.padColors.red,
    });

    // =================================== ENCODERS ==============================================
    this.loopSizeEncoder = new components.Encoder({
        midi: [0xB0 + channel, 0x1B], // Note: these are the MIDI bytes for the digit LEDs, not
                                      // input from the encoder.
        input: function (channel, control, value, status, group) {
            var loopSize = engine.getValue(this.group, 'beatloop_size');
            if (loopEnabledDot) {
                if (value > 64 && loopSize > 2) { // turn left
                    /**
                        Unfortunately, there is no way to show 1 with a dot on the
                        loop size LED.
                    **/
                    engine.setValue(this.group, 'beatloop_size', loopSize / 2);
                } else if (value < 64 && loopSize < 32) { // turn right
                    /**
                        Mixxx supports loops longer than 32 beats, but there is no way
                        to show 64 with a dot on the loop size LED.
                    **/
                    engine.setValue(this.group, 'beatloop_size', loopSize * 2);
                }
            } else {
                if (value > 64 && loopSize > 1/32) { // turn left
                    /**
                        Mixxx supports loops shorter than 1/32 beats, but there is no
                        way to set the loop size LED less than 1/32 (even though it
                        should be able to show 1/64)
                    **/
                    engine.setValue(this.group, 'beatloop_size', loopSize / 2);
                } else if (value < 64 && loopSize < 64) { // turn right
                    /**
                        Mixxx supports loops longer than 64 beats, but the loop size LED
                        only has 2 digits, so it couldn't show 128
                    **/
                    engine.setValue(this.group, 'beatloop_size', loopSize * 2);
                }
            }
        },
        connect: function () {
            this.connections[0] = engine.connectControl(this.group, 'beatloop_size', this.output);
            if (loopEnabledDot) {
                this.connections[1] = engine.connectControl(this.group, 'loop_enabled', this.output);
            }
        },
        output: function (value, group, control) {
            var loopSize = engine.getValue(this.group, 'beatloop_size');
            var loopSizeLogBase2 = Math.log(loopSize) / Math.log(2);
            // test if loopSizeLogBase2 is an integer
            if (Math.floor(loopSizeLogBase2) === loopSizeLogBase2) {
                if (loopEnabledDot && engine.getValue(this.group, 'loop_enabled') === 1) {
                    this.send(5 - loopSizeLogBase2);
                } else {
                    this.send(5 + loopSizeLogBase2);
                }
            } else {
                this.send(14); // show two dots
            }
        }
    });

    this.loopMoveEncoder = function (channel, control, value, status, group) {
        if (value < 64) { // left turn
            engine.setValue(this.currentDeck, 'loop_move_backward', 1);
        } else { // right turn
            engine.setValue(this.currentDeck, 'loop_move_forward', 1);
        }
    };

    this.loopToggleEncoderPress = function (channel, control, value, status, group) {
        engine.setValue(this.currentDeck, 'loopauto_toggle', value / 127);
    };

    this.loopEncoderManualLoopPress = function (channel, control, value, status, group) {
        engine.setValue(this.currentDeck, 'loopmanual_toggle', value / 127);
    };

    this.tempoEncoder = function (channel, control, value, status, group) {
        var direction = (value > 64) ? -1 : 1;
        engine.setValue(this.currentDeck, 'rate', engine.getValue(this.currentDeck, 'rate') + (0.01 * direction));
    };

    this.tempoEncoderPress = function (channel, control, value, status, group) {
        if (value) {
            engine.setValue(this.currentDeck, 'rate', 0);
        }
    };

    this.beatJumpEncoder = function (channel, control, value, status, group) {
        var beatJumpSize = engine.getValue(this.currentDeck, 'beatjump_size');
        if (this.beatJumpEncoderPressed) {
            if (value > 64 && beatJumpSize > 1/32) { // turn left
                engine.setValue(this.currentDeck, 'beatjump_size', beatJumpSize / 2);
            } else if (value < 64 && beatJumpSize < 64) { // turn right
                engine.setValue(this.currentDeck, 'beatjump_size', beatJumpSize * 2);
            }
            // The firmware will only change the numeric LED readout when sent messages
            // on the unshifted channel.
            midi.sendShortMsg(0xB0 + channel - P32.shiftOffset, 0x1B, 5 + Math.log(beatJumpSize) / Math.log(2));
        } else {
            var direction = (value > 64) ? 'backward' : 'forward';
            engine.setValue(this.currentDeck, 'beatjump_' + direction, 1);
            engine.beginTimer(200, function () {
                engine.setValue(this.currentDeck, 'beatjump_' + direction, 0);
            }, true);
        }
    };

    this.beatJumpEncoderPress = function (channel, control, value, status, group) {
        // The firmware will only change the numeric LED readout when sent messages
        // on the unshifted channel.
        if (value === 127) {
            this.beatJumpEncoderPressed = true;
            var beatJumpSize = engine.getValue(this.currentDeck, 'beatjump_size');
            midi.sendShortMsg(0xB0 + channel - P32.shiftOffset, 0x1B,
                              5 + Math.log(beatJumpSize) / Math.log(2));
        } else {
            this.beatJumpEncoderPressed = false;
            var loopSize = engine.getValue(this.currentDeck, 'beatloop_size');
            midi.sendShortMsg(0xB0 + channel - P32.shiftOffset, 0x1B,
                              5 + Math.log(loopSize) / Math.log(2));
        }
    };

    this.loadTrack = function (channel, control, value, status, group) {
        if (value === 127) {
            engine.setValue(this.currentDeck, 'LoadSelectedTrack', 1);
        }
    };

    this.ejectTrack = function (channel, control, value, status, group) {
        if (value === 127) {
            engine.setValue(this.currentDeck, 'eject', 1);
            engine.beginTimer(225, function () {
                engine.setValue(this.currentDeck, 'eject', 0);
            }, true);
        }
    };

    this.reconnectComponents(function (component) {
        if (component.group === undefined) {
            component.group = this.currentDeck;
        }
    });

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
};
P32.Deck.prototype = new components.Deck();
