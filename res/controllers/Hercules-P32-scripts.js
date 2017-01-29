// USER CONFIGURABLE OPTIONS
// loop size (in beats) when Mixxx starts
var defaultLoopSize = 8;
// beat jump size when Mixxx starts
var defaultBeatJumpSize = 4;
// Set to "true" to use the dot on the loop size LED display to indicate
// that a loop is active. This restricts loop sizes to 2-32 beats and
// may be helpful if you never use loops less than 2 beats long.
// Otherwise the dot indicates a loop size equal to 1/(# on the LED display).
var loopEnabledDot = false;
// Assign samplers on left side of the controller to left side of the crossfader
// and samplers on right side of the controller to the right side of the crossfader
var samplerCrossfaderAssign = true;

/**
 * Hercules P32 DJ controller script for Mixxx 2.0
 * Thanks to Hercules for supporting the development of this mapping by providing a controller
 * See http://mixxx.org/wiki/doku.php/hercules_p32_dj for instructions on how to use this mapping
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
    Control.prototype.shiftOffset = 3;
    Control.prototype.shiftChannel = true;
    Button.prototype.sendShifted = true;

    /**
    The P32 has encoders for changing tempo, so the actual tempo getting out of sync with a hardware
    fader and dealing with soft takeover in that situation is not an issue. So, make toggling master
    sync the default unshifted behavior and momentary sync the shifted behavior.
    **/
    SyncButton.prototype.unshift = function () {
        this.inCo = 'sync_enabled';
    };
    SyncButton.prototype.shift = function () {
        this.inCo = 'beatsync';
    };

    P32.leftDeck = new P32.Deck([1,3], 1);
    P32.rightDeck = new P32.Deck([2,4], 2);

    if (engine.getValue('[Master]', 'num_samplers') < 32) {
        engine.setValue('[Master]', 'num_samplers', 32);
    }

    // tell controller to send MIDI messages with positions of faders and knobs
    midi.sendShortMsg(0xB0, 0x7F, 0x7F);
};

P32.shutdown = function () {
    for (var channel = 0; channel <= 5; channel++) {
        for (var button = 1; button <= 0x63; button++) {
            midi.sendShortMsg(0x90 + channel, button, 0);
        }
    }
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

P32.recordButton = new Button({
    midi: [0x90, 0x02],
    group: '[Recording]',
    inCo: 'toggle_recording',
    onlyOnPress: false,
    outCo: 'status',
    sendShifted: false,
});

P32.slipButton = new Button({
    midi: [0x90, 0x03],
    input: function (channel, control, value, status, group) {
        if (P32.leftDeck.isShifted) {
            P32.leftDeck.toggle();
        } else if (P32.rightDeck.isShifted) {
            P32.rightDeck.toggle();
        } else {
            for (var i = 1; i <= 4; i++) {
                script.toggleControl('[Channel' + i + ']', 'slip_enabled');
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
    co: 'slip_enabled',
    sendShifted: false,
    group: null // hack to get Control constructor to call this.connect()
});

P32.Deck = function (deckNumbers, channel) {
    Deck.call(this, deckNumbers);

    var loopSize = defaultLoopSize;
    var beatJumpSize = defaultBeatJumpSize;
    var theDeck = this;

    this.shiftButton = function (channel, control, value, status, group) {
        if (value === 127) {
            this.shift();
        } else {
            this.unshift();
        }
    };

    // ===================================== TRANSPORT =========================================
    this.sync = new SyncButton([0x90 + channel, 0x08]);
    this.cue = new CueButton([0x90 + channel, 0x09]);
    this.play = new PlayButton([0x90 + channel, 0x0A]);

    // ===================================== MIXER ==============================================
    this.eqKnob = [];
    for (var k = 1; k <= 3; k++) {
        this.eqKnob[k] = new Pot({
            midi: [0xB0 + channel, 0x02 + k],
            group: '[EqualizerRack1_' + this.currentDeck + '_Effect1]',
            inCo: 'parameter' + k,
        });
    }

    this.pfl = new Button({
        midi: [0x90 + channel, 0x10],
        co: 'pfl',
    });

    this.volume = new Pot({
        midi: [0xB0 + channel, 0x01],
        inCo: 'volume',
    });

    // ==================================== PAD GRID ============================================
    // The slicer layer is handled by this.effectUnit.enableOnChannelButtons, set up under the
    // EFFECTS section.

    this.hotcueButton = [];
    this.samplerButton = [];
    for (var i = 1; i <= 16; i++) {
        this.hotcueButton[i] = new HotcueButton({
            midi: [0x90 + channel,
                   P32.PadNumToMIDIControl(i, 3)],
            number: i,
            on: P32.padColors.red
        });
        var samplerNumber = i + (channel - 1) * 16;
        this.samplerButton[samplerNumber] = new SamplerButton({
            midi: [0x90 + channel, P32.PadNumToMIDIControl(i, 0)],
            number: samplerNumber,
            on: P32.padColors.red,
            off: P32.padColors.off,
            playing: P32.padColors.blue
        });
        if (samplerCrossfaderAssign) {
            engine.setValue('[Sampler' + samplerNumber + ']',
                            'orientation',
                            (channel === 1) ? 0 : 2
            );
        }
    }

    this.loopIn = new Button({
        midi: [0x90 + channel, 0x50],
        inCo: 'loop_in',
    });
    this.loopOut = new Button({
        midi: [0x90 + channel, 0x51],
        inCo: 'loop_out',
    });
    this.loopTogglePad = new LoopToggleButton({
        midi: [0x90 + channel, 0x52],
        on: P32.padColors.red,
        off: P32.padColors.blue,
    });
    this.loopIn.send(P32.padColors.purple);
    this.loopOut.send(P32.padColors.purple);

    this.tempSlow = new Button({
        midi: [0x90 + channel, 0x44],
        inCo: 'rate_temp_down',
        onlyOnPress: false,
    });
    this.tempFast = new Button({
        midi: [0x90 + channel, 0x45],
        inCo: 'rate_temp_down',
        onlyOnPress: false,
    });
    this.alignBeats = new Button({
        midi: [0x90 + channel, 0x46],
        inCo: 'beats_translate_curpos',
    });
    this.quantize = new Button({
        midi: [0x90 + channel, 0x47],
        co: 'quantize',
        on: P32.padColors.red,
        off: P32.padColors.blue,
    });
    this.tempSlow.send(P32.padColors.purple);
    this.tempFast.send(P32.padColors.purple);
    this.alignBeats.send(P32.padColors.blue);

    // =================================== ENCODERS ==============================================
    this.loopSizeEncoder = new Control({
        midi: [0xB0 + channel, 0x1B], // Note: these are the MIDI bytes for the LED readout, not
                                      // input from the encoder.
        input: function (channel, control, value, status, group) {
            if (loopEnabledDot) {
                if (value > 64 && loopSize > 2) { // turn left
                    /**
                        Unfortunately, there is no way to show 1 with a dot on the
                        loop size LED.
                    **/
                    loopSize /= 2;
                    engine.setValue(this.group, 'loop_halve', 1);
                    engine.setValue(this.group, 'loop_halve', 0);
                } else if (value < 64 && loopSize < 32) { // turn right
                    /**
                        Mixxx supports loops longer than 32 beats, but there is no way
                        to show 64 with a dot on the loop size LED.
                    **/
                    loopSize *= 2;
                    engine.setValue(this.group, 'loop_double', 1);
                    engine.setValue(this.group, 'loop_double', 0);
                }
            } else {
                if (value > 64 && loopSize > 1/32) { // turn left
                    /**
                        Mixxx supports loops shorter than 1/32 beats, but there is no
                        way to set the loop size LED less than 1/32 (even though it
                        should be able to show 1/64)
                    **/
                    loopSize /= 2;
                    engine.setValue(this.group, 'loop_halve', 1);
                    engine.setValue(this.group, 'loop_halve', 0);
                } else if (value < 64 && loopSize < 64) { // turn right
                    /**
                        Mixxx supports loops longer than 64 beats, but the loop size LED
                        only has 2 digits, so it couldn't show 128
                    **/
                    loopSize *= 2;
                    engine.setValue(this.group, 'loop_double', 1);
                    engine.setValue(this.group, 'loop_double', 0);
                }
            }
            this.trigger();
        },
        outCo: 'loop_enabled',
        output: function (value, group, control) {
            if (loopEnabledDot && value) {
                this.send(5 - Math.log(loopSize) / Math.log(2));
            } else {
                this.send(5 + Math.log(loopSize) / Math.log(2));
            }
        }
    });

    this.loopMoveEncoder = function (channel, control, value, status, group) {
        var direction = (value > 64) ? -1 : 1;
        if (loopSize < 1) {
            engine.setValue(this.currentDeck, 'loop_move', loopSize * direction);
        } else {
            engine.setValue(this.currentDeck, 'loop_move', 1 * direction);
        }
    };

    this.loopToggleEncoderPress = function (channel, control, value, status, group) {
        if (value) {
            if (engine.getValue(this.currentDeck, 'loop_enabled')) {
                engine.setValue(this.currentDeck, 'reloop_exit', 1);
            } else {
                engine.setValue(this.currentDeck, 'beatloop_' + loopSize + '_activate', 1);
            }
        } else {
            if (loopSize <= 1 && engine.getValue(this.currentDeck, 'loop_enabled')) {
                engine.setValue(this.currentDeck, 'reloop_exit', 1);
            }
        }
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
        var direction = (value > 64) ? -1 : 1;
        if (this.beatJumpEncoderPressed) {
            if (value > 64 && beatJumpSize > 1/32) { // turn left
                beatJumpSize /= 2;
            } else if (value < 64 && beatJumpSize < 64) { // turn right
                beatJumpSize *= 2;
            }
            // The firmware will only change the numeric LED readout when sent messages
            // on the unshifted channel.
            midi.sendShortMsg(0xB0 + channel - P32.shiftOffset, 0x1B, 5 + Math.log(beatJumpSize) / Math.log(2));
        } else {
            engine.setValue(this.currentDeck, 'beatjump', direction * beatJumpSize);
        }
    };

    this.beatJumpEncoderPress = function (channel, control, value, status, group) {
        // The firmware will only change the numeric LED readout when sent messages
        // on the unshifted channel.
        if (value === 127) {
            this.beatJumpEncoderPressed = true;
            midi.sendShortMsg(0xB0 + channel - P32.shiftOffset, 0x1B, 5 + Math.log(beatJumpSize) / Math.log(2));
        } else {
            this.beatJumpEncoderPressed = false;
            midi.sendShortMsg(0xB0 + channel - P32.shiftOffset, 0x1B, 5 + Math.log(loopSize) / Math.log(2));
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
            engine.beginTimer(250, 'engine.setValue("'+this.currentDeck+'", "eject", 0)', true);
        }
    };

    this.reconnectControls(function (control) {
        if (control.group === undefined) {
            control.group = this.currentDeck;
        }
    });

    // ==================================== EFFECTS ==============================================
    this.effectUnit = new EffectUnit(deckNumbers[0]);
    this.effectUnit.enableButtons[1].midi = [0x90 + channel, 0x03];
    this.effectUnit.enableButtons[2].midi = [0x90 + channel, 0x04];
    this.effectUnit.enableButtons[3].midi = [0x90 + channel, 0x05];
    this.effectUnit.showParametersButton.midi = [0x90 + channel, 0x06];
    this.effectUnit.enableOnChannelButtons.Channel1.midi = [0x90 + channel, 0x40];
    this.effectUnit.enableOnChannelButtons.Channel2.midi = [0x90 + channel, 0x41];
    this.effectUnit.enableOnChannelButtons.Channel3.midi = [0x90 + channel, 0x42];
    this.effectUnit.enableOnChannelButtons.Channel4.midi = [0x90 + channel, 0x43];
    this.effectUnit.enableOnChannelButtons.Headphone.midi = [0x90 + channel, 0x34];
    this.effectUnit.enableOnChannelButtons.Master.midi = [0x90 + channel, 0x35];
    this.effectUnit.enableOnChannelButtons.Microphone.midi = [0x90 + channel, 0x36];
    this.effectUnit.enableOnChannelButtons.Auxiliary1.midi = [0x90 + channel, 0x37];
    this.effectUnit.enableOnChannelButtons.forEachControl(function (button) {
        button.on = P32.padColors.red;
        button.off = P32.padColors.blue;
    });
    this.effectUnit.init();
};
P32.Deck.prototype = new Deck();
