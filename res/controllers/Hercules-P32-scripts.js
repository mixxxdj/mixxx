/**
 * Hercules P32 DJ controller script for Mixxx 2.0
 * Thanks to Hercules by supporting the development of this mapping by donating a controller
 * See http://mixxx.org/wiki/doku.php/hercules_p32_dj for instructions on how to use this mapping
 * 
 * Copyright (C) 2016 Be <be.0@gmx.com>
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

Control = function (signals, group, inOptions, outOptions) {
    var that = this;
    this.midi = {status: signals[0], note: signals[1]};
    this.group = group;
    
    this.setup = function (inOptions, outOptions) { that.inSetup(inOptions); that.outSetup(outOptions) };
    this.outConnect = function () { engine.connectControl(that.group, that.outCo, that.output); };
    this.outDisconnect = function () { engine.connectControl(that.group, that.outCo, that.output, true); };
    this.outTrigger = function() { engine.trigger(that.group, that.outCo); };
    this.send = function (value) { midi.sendShortMsg(that.midi.status, that.midi.note, value) };
    
    this.inSetup = function (inOptions) {
        this.inCo = inOptions[0];
        var inFunc = inOptions[1];
        if (inFunc === undefined) {inFunc = function (value) {return value}};
        var onlyOnPress = inOptions[2];
        if (inFunc === null) {
            this.input = null;
        } else {
            if (onlyOnPress === undefined) {onlyOnPress = true};
            if (onlyOnPress) {
                this.input = function (channel, control, value, status, group) {
                    if (value) {
                        engine.setValue(that.group, that.inCo, inFunc.call(that, value));
                    }
                }
            } else {
                this.input = function (channel, control, value, status, group) {
                    engine.setValue(that.group, that.inCo, inFunc.call(that, value));
                }
            }
        }
    }
    this.inSetup(inOptions);
    this.previousInput = null;
    
    this.outSetup = function (outOptions) {
        this.outCo = outOptions[0];
        var outFunc = outOptions[1];
        var connect = outOptions[2];
        var trigger = outOptions[3];
        if (outFunc === undefined) {outFunc = function (value) {return value}};
        if (connect === undefined) {connect = true};
        if (trigger === undefined) {trigger = true};
        if (outFunc === null) {
            this.output = null;
        } else {
            this.output = function (value, group, control) {
                that.send(outFunc.call(that, value));
            }
        }
        if (connect) { this.outDisconnect(); this.outConnect() };
        if (trigger) { this.outTrigger() };
    }
    this.outSetup(outOptions);
    this.previousOutput = null;
}

// for COs that only get set to 1
ActionButton = function (signals, group, inCo) {
    var that = this;
    Control.call(this, signals, group,
                [inCo, function () { return 1; }],
                null);
}

// for buttons that toggle a binary CO
ToggleButton = function (signals, group, co, on, off, onlyOnPress) {
    var that = this;
    if (onlyOnPress === undefined) {onlyOnPress = true};
    if (on === undefined) {on = 127};
    if (off === undefined) {off = 0};
    Control.call(this, signals, group,
                [co, function () { return ! engine.getValue(this.group, this.inCo) }, onlyOnPress],
                [co, function () { return (engine.getValue(this.group, this.outCo)) ? on : off }]);
}

KnobLin = function (signals, group, co, low, high, min, max) {
    var that = this;
    Controll.call(this, signals, group,
                [co, function (value) { script.absoluteLin(value, low, high, min, max) }, false],
                null);
}

KnobNonLin = function (signals, group, co, low, mid, high, min, max) {
    var that = this;
    Controll.call(this, signals, group,
                [co, function (value) { script.absoluteNonLin(value, low, mid, high, min, max) }, false],
                null);
}


script.samplerRegEx = /\[Sampler(\d+)\]/
script.channelRegEx = /\[Channel(\d+)\]/

function P32 () {};

P32.init = function () {
    P32.shiftOffset = 3;
    P32.leftDeck = new P32.Deck([1,3], 1);
    P32.rightDeck = new P32.Deck([2,4], 2);
    // set loop sizes to 4
    midi.sendShortMsg(0xB1, 0x1B, 7);
    midi.sendShortMsg(0xB2, 0x1B, 7);
}

P32.shutdown = function () {};

P32.browse = function (channel, control, value, status, group) {
    if (value === 127) {
        engine.setValue('[Playlist]', 'SelectPrevTrack', 1);
    } else {
        engine.setValue('[Playlist]', 'SelectNextTrack', 1);
    }
}

P32.Deck = function (deckNumbers, channel) {
    var that = this;
    var loopSize = 4;
    this.shift = false;
    this.currentDeck = "[Channel" + deckNumbers[0] + "]";
    this.deckToggle = function () {
        print(that.currentDeck);
        var index = deckNumbers.indexOf(parseInt(script.channelRegEx.exec(that.currentDeck)[1]));
        if (index === (deckNumbers.length - 1)) {
            index = 0;
        } else {
            index += 1;
        }
        that.currentDeck = "[Channel" + deckNumbers[index] + "]";
        print(that.currentDeck);
    }
    
    this.eqKnob = function (channel, control, value, status, group) {
        engine.setParameter('[EqualizerRack1_' + that.currentDeck + '_Effect1]',
                        'parameter' + (control - 1),
                        script.absoluteLin(value, 0, 1));
    }
    
    this.volume = function (channel, control, value, status, group) {
        engine.setParameter(that.currentDeck, 'volume', script.absoluteLin(value, 0, 1));
    }
    
    this.loadTrack = function (channel, control, value, status, group) {
        if (value) {
            engine.setValue(that.currentDeck, 'LoadSelectedTrack', 1);
        }
    }
    
    this.ejectTrack = function (channel, control, value, status, group) {
        if (value) {
            engine.setValue(that.currentDeck, 'eject', 1);
        }
    }
    
    this.loopSizeEncoder = function (channel, control, value, status, group) {
        if (value === 127 && loopSize > 1/32) { // turn left
            /**
                Mixxx supports loops shorter than 1/32 beats, but there is no
                way to set the loop size LED less than 1/32 (even though it
                should be able to show 1/64)
            **/
            loopSize /= 2;
            engine.setValue(that.currentDeck, 'loop_halve', 1);
            engine.setValue(that.currentDeck, 'loop_halve', 0);
        } else if (value === 1 && loopSize < 64) { // turn right
            /**
                Mixxx supports loops longer than 64 beats, but the loop size LED
                only has 2 digits, so it couldn't show 128
            **/
            loopSize *= 2;
            engine.setValue(that.currentDeck, 'loop_double', 1);
            engine.setValue(that.currentDeck, 'loop_double', 0);
        }
        midi.sendShortMsg(0xB0 + channel, 0x1B, 5 + Math.log(loopSize) / Math.log(2));
    }
    
    this.loopActive = function (channel, control, value, status, group) {
        if (value) {
            if (engine.getValue(that.currentDeck, 'loop_enabled')) {
                engine.setValue(that.currentDeck, 'reloop_exit', 1);
            } else {
                engine.setValue(that.currentDeck, 'beatloop_' + loopSize + '_activate', 1);
            }
        }
    }
    
    this.tempo = function (channel, control, value, status, group) {
        var direction = (value === 127) ? -1 : 1;
        engine.setValue(that.currentDeck, 'rate', engine.getValue(that.currentDeck, 'rate') + (.01 * direction));
    }
    
    this.tempoReset = function (channel, control, value, status, group) {
        if (value) {
            engine.setValue(that.currentDeck, 'rate', 0);
        }
    }
    
    this.filter = function (channel, control, value, status, group) {
        // TODO: Make this smoother?
        var direction = (value === 127) ? -1 : 1;
        engine.setValue('[QuickEffectRack1_'+ that.currentDeck +']', 'super1',
                        engine.getValue('[QuickEffectRack1_'+ that.currentDeck +']', 'super1') + (.05 * direction));
    }
    
    this.loopMove = function (channel, control, value, status, group) {
        var direction = (value === 127) ? -1 : 1;
        if (loopSize < 1) {
            engine.setValue(that.currentDeck, 'loop_move', loopSize * direction);
        } else {
            engine.setValue(that.currentDeck, 'loop_move', 1 * direction);
        }
    }

    this.sync = new ToggleButton([0x90 + channel, 0x08], this.currentDeck, 'sync_enabled');
    this.cue = new Control([0x90 + channel, 0x09], this.currentDeck,
                           ['cue_default', function (val) { return val / 127 }, false],
                           ['cue_indicator', function (val) { return val * 127 }]);
    this.play = new Control([0x90 + channel, 0x0A], that.currentDeck,
                            ['play', function () { print(that.currentDeck); return ! engine.getValue(this.group, this.inCo) }],
                            ['play_indicator', function (val) { return val * 127 }]);
    
    this.keylock = new ToggleButton([0x90 + channel + P32.shiftOffset, 0x08], this.currentDeck, 'keylock'); // sync shifted
    this.quantize = new ToggleButton([0x90 + channel + P32.shiftOffset, 0x09], this.currentDeck, 'quantize'); // cue shifted
    this.playShifted = new Control([0x90 + channel + P32.shiftOffset, 0x0A], this.currentDeck,
                            ['play', function () { return ! engine.getValue(this.group, this.inCo) }],
                            ['play_indicator', function (val) { return val * 127 }]);
    
    this.pfl = new ToggleButton([0x90 + channel, 0x10], this.currentDeck, 'pfl');

}
