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

/**
 * Hercules P32 DJ controller script for Mixxx 2.0
 * Thanks to Hercules by supporting the development of this mapping by providing a controller
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

var Control = function (signals, group, inOptions, outOptions) {
    var that = this;
    this.midi = {status: signals[0], note: signals[1]};
    this.group = group;
    
    this.setup = function (inOptions, outOptions) { that.inSetup(inOptions); that.outSetup(outOptions) };
    
    this.inSetup = function (inOptions) {
        if (inOptions === null) {
            print("Control.inSetup() called with null argument.");
            return;
        }
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
    
   var connection;
   this.connect = function () {
      connection = engine.connectControl(that.group, that.outCo, that.output);
   };
   this.disconnect = function () { connection.disconnect(); };
//  this.trigger = function() { outConnection.trigger(); };
//     this.outConnect = function () { engine.connectControl(that.group, that.outCo, that.output); };
//     this.disconnect = function () {
//         print('disconnecting: ' + that.group + ' , ' + that.outCo + ' , ' + that.output);
//         engine.connectControl(that.group, that.outCo, that.output, true);};
    this.trigger = function() { engine.trigger(that.group, that.outCo); };
    this.send = function (value) { midi.sendShortMsg(that.midi.status, that.midi.note, value) };
    
    this.outSetup = function (outOptions) {
        if (outOptions === null) {
            print("Control.outSetup() called with null argument.");
            return;
        }
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
        if (connect) { this.connect() };
        if (trigger) { this.trigger() };
    }
    this.outSetup(outOptions);
    this.previousOutput = null;
}

// for buttons that toggle a binary CO
ToggleButton = function (signals, deck, co, on, off, onlyOnPress) {
    var that = this;
    if (on === undefined) {on = 127};
    if (off === undefined) {off = 0};
    Control.call(this, signals, deck,
                [co, function () { return ! engine.getValue(this.group, this.inCo) }, onlyOnPress],
                [co, function () { return (engine.getValue(this.group, this.outCo)) ? on : off }]);
}
ToggleButton.prototype = Object.create(Control.prototype);
ToggleButton.prototype.constructor = ToggleButton;

// for COs that only get set to 1
ActionButton = function (signals, deck, inCo, onlyOnPress) {
    var that = this;
    Control.call(this, signals, deck,
                [inCo, function () { return 1; }, onlyOnPress],
                null);
}
ActionButton.prototype = Object.create(Control.prototype);
ActionButton.prototype.constructor = ActionButton;

KnobLin = function (signals, deck, co, low, high, min, max) {
    var that = this;
    Controll.call(this, signals, deck,
                [co, function (value) { script.absoluteLin(value, low, high, min, max) }, false],
                null);
}
KnobLin.prototype = Object.create(Control.prototype);
KnobLin.prototype.constructor = KnobLin;

KnobNonLin = function (signals, deck, co, low, mid, high, min, max) {
    var that = this;
    Controll.call(this, signals, deck,
                [co, function (value) { script.absoluteNonLin(value, low, mid, high, min, max) }, false],
                null);
}
KnobNonLin.prototype = Object.create(Control.prototype);
KnobNonLin.prototype.constructor = KnobNonLin;

script.samplerRegEx = /\[Sampler(\d+)\]/
script.channelRegEx = /\[Channel(\d+)\]/

function P32 () {};

P32.init = function () {
    P32.shiftOffset = 3;
    P32.leftDeck = new P32.Deck([1,3], 1);
    P32.rightDeck = new P32.Deck([2,4], 2);
    // set loop sizes to 4
    midi.sendShortMsg(0xB1, 0x1B, 5 + Math.log(defaultLoopSize) / Math.log(2));
    midi.sendShortMsg(0xB2, 0x1B, 5 + Math.log(defaultLoopSize) / Math.log(2));
    
    for (var i = 1; i <= 4; i++) {
        engine.softTakeover('[EqualizerRack1_[Channel'+i+']_Effect1]', 'parameter1', true);
        engine.softTakeover('[EqualizerRack1_[Channel'+i+']_Effect1]', 'parameter2', true);
        engine.softTakeover('[EqualizerRack1_[Channel'+i+']_Effect1]', 'parameter3', true);
        engine.softTakeover('[Channel'+i+']', 'volume', true);
        if (i > 2) {
            engine.softTakeoverIgnoreNextValue('[EqualizerRack1_[Channel'+i+']_Effect1]', 'parameter1', true);
            engine.softTakeoverIgnoreNextValue('[EqualizerRack1_[Channel'+i+']_Effect1]', 'parameter2', true);
            engine.softTakeoverIgnoreNextValue('[EqualizerRack1_[Channel'+i+']_Effect1]', 'parameter3', true);
            engine.softTakeoverIgnoreNextValue('[Channel'+i+']', 'volume', true);
        }
    }
}

P32.shutdown = function () {};

P32.padColors = {
    red: 125,
    blue: 126,
    purple: 127
}

P32.PadNumToMIDIControl = function (PadNum) {
    // The MIDI control numbers for the pad grid are numbered bottom to top, so
    // this returns the MIDI control numbers for the pads numbered top to bottom
    PadNum -= 1;
    var midiRow = 3 - Math.floor(PadNum/4);
    return 0x54 + midiRow*4 + PadNum%4;
}

P32.browse = function (channel, control, value, status, group) {
    if (value === 127) {
        engine.setValue('[Playlist]', 'SelectPrevTrack', 1);
    } else {
        engine.setValue('[Playlist]', 'SelectNextTrack', 1);
    }
}

P32.headMix = function (channel, control, value, status, group) {
    var direction = (value === 127) ? -1 : 1;
    engine.setValue('[Master]', 'headMix', engine.getValue('[Master]', 'headMix') + (.25 * direction));
}

P32.record = new Control([0x90, 0x02], {currentDeck: '[Recording]'},
                         ['toggle_recording', function (val) {
                             if (P32.leftDeck.shift) {
                                 P32.leftDeck.deckToggle();
                             } else if (P32.rightDeck.shift) {
                                 P32.rightDeck.deckToggle();
                             } else {
                                return val / 127;
                            }
                         }],
                         ['status', function (val) {return val * 127}]);

P32.EffectUnit = function (unitNumber) {
    var effectUnit = '[EffectRack1_EffectUnit' + unitNumber + ']';
    var activeEffectNumber = 1;
    var activeEffect = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + activeEffectNumber + ']';

    // knobs
    this.parameterKnob1 = function (channel, control, value, status, group) {
        engine.setParameter(activeEffect, 'parameter1', value/127);
    }
    this.parameterKnob2 = function (channel, control, value, status, group) {
        engine.setParameter(activeEffect, 'parameter2', value/127);
    }
    this.parameterKnob3 = function (channel, control, value, status, group) {
        engine.setParameter(activeEffect, 'parameter3', value/127);
    }
    this.dryWet = function (channel, control, value, status, group) {
        engine.setParameter(effectUnit, 'mix', value/127);
    }
    this.superKnob = function (channel, control, value, status, group) {
        engine.setParameter(effectUnit, 'super1', value/127);
    }

    // deck enable buttons
    for (var d = 1; d <= 4; d++) {
        this['deckButton' + d] = new ToggleButton([0x90 + unitNumber, 0x02 + d], effectUnit, 'group_[Channel' + d + ']_enable');
    }
    this.pflToggle = function () {
        script.toggleControl(effectUnit, 'group_[Headphone]_enable');
    }

    // buttons to select the effect that the knobs control
    var switchEffect = function (effectNumber) {
        activeEffectNumber = effectNumber;
        activeEffect = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + activeEffectNumber + ']';
        for (var e = 1; e < 4; e ++) {
            midi.sendShortMsg(0x90 + unitNumber + P32.shiftOffset,
                              0x02 + e,
                              (e === activeEffectNumber) ? 127 : 0);
        }
    }
    this.switchEffect1 = function (channel, control, value, status, group) { switchEffect(1); };
    this.switchEffect2 = function (channel, control, value, status, group) { switchEffect(2); };
    this.switchEffect3 = function (channel, control, value, status, group) { switchEffect(3); };
    this.switchEffect4 = function (channel, control, value, status, group) { switchEffect(4); };
};

P32.Deck = function (deckNumbers, channel) {
    var that = this;
    var t = this;
    var loopSize = defaultLoopSize;
    var beatJumpSize = defaultBeatJumpSize;
    this.shift = false;
    this.currentDeck = "[Channel" + deckNumbers[0] + "]";
    this.effectUnit = new P32.EffectUnit(deckNumbers[0]);
    this.deckToggle = function () {
        engine.softTakeoverIgnoreNextValue('[EqualizerRack1_' + this.currentDeck + '_Effect1]', 'parameter1', true);
        engine.softTakeoverIgnoreNextValue('[EqualizerRack1_' + this.currentDeck + '_Effect1]', 'parameter2', true);
        engine.softTakeoverIgnoreNextValue('[EqualizerRack1_' + this.currentDeck + '_Effect1]', 'parameter3', true);
        engine.softTakeoverIgnoreNextValue(this.currentDeck, 'volume');
        for (var c in this) {
            if (this.hasOwnProperty(c)) {
                if (this[c] instanceof Control) {
                    this[c].disconnect();
                }
            }
        }
        
        var index = deckNumbers.indexOf(parseInt(script.channelRegEx.exec(this.currentDeck)[1]));
        if (index === (deckNumbers.length - 1)) {
            index = 0;
        } else {
            index += 1;
        }
        this.currentDeck = "[Channel" + deckNumbers[index] + "]";
        
        for (c in this) {
            if (this.hasOwnProperty(c)) {
                if (this[c] instanceof Control) {
                    this[c].group = this.currentDeck;
                    this[c].connect();
                    this[c].trigger();
                }
            }
        }
        print("Switched to deck: " + this.currentDeck);
    }
    
    this.shiftButton = function (channel, control, value, status, group) {
        that.shift = (value === 127) ? true : false;
    }
    
    this.sync = new ToggleButton([0x90 + channel, 0x08], this.currentDeck, 'sync_enabled');
    this.cue = new Control([0x90 + channel, 0x09], this.currentDeck,
                           ['cue_default', function (val) { return val / 127 }, false],
                           ['cue_indicator', function (val) { return val * 127 }]);
    this.play = new Control([0x90 + channel, 0x0A], this.currentDeck,
                            ['play', function () { return ! engine.getValue(this.group, this.inCo) }],
                            ['play_indicator', function (val) { return val * 127 }]);
    
    this.quantize = new ToggleButton([0x90 + channel + P32.shiftOffset, 0x08], this.currentDeck, 'quantize'); // sync shifted
    this.keylock = new ToggleButton([0x90 + channel + P32.shiftOffset, 0x09], this.currentDeck, 'keylock'); // cue shifted
    this.goToStart = new Control([0x90 + channel + P32.shiftOffset, 0x0A], this.currentDeck, // play shifted
                            ['start_stop', function () { return 1; }],
                            ['play_indicator', function (val) { return val * 127 }]);
    
    for (var i = 1; i <= 16; i++) {
        this['hotcueButton' + i] = new Control([0x90 + channel, P32.PadNumToMIDIControl(i)], this.currentDeck,
                                            ['hotcue_'+i+'_activate', function (value) {return value/127}, false],
                                            ['hotcue_'+i+'_enabled', function (val) { return val * P32.padColors.red }]);
        this['hotcueButtonShift' + i] = new Control([0x90 + channel + P32.shiftOffset, P32.PadNumToMIDIControl(i)], this.currentDeck,
                                            ['hotcue_'+i+'_clear', function (value) {return 1}],
                                            ['hotcue_'+i+'_enabled', function (val) { return val * P32.padColors.red }]);
    }
    
    this.pfl = new ToggleButton([0x90 + channel, 0x10], this.currentDeck, 'pfl');
    this.pfl.input = function (channel, control, value, status, group) {
        if (value === 127) {
            if (that.shift) {
                that.effectUnit.pflToggle();
            } else {
                script.toggleControl(that.currentDeck, 'pfl');
            }
        }
    }
    
    this.eqKnob = function (channel, control, value, status, group) {
        engine.setValue('[EqualizerRack1_' + that.currentDeck + '_Effect1]',
                        'parameter' + (control - 1),
                        script.absoluteNonLin(value, 0, 1, 4));
    }
    
    this.volume = function (channel, control, value, status, group) {
        engine.setValue(that.currentDeck, 'volume', script.absoluteNonLin(value, 0, .25, 1));
    }

    this.loadTrack = function (channel, control, value, status, group) {
        if (value) {
            engine.setValue(that.currentDeck, 'LoadSelectedTrack', 1);
        }
    }
//     this.loadTrack = new ActionButton([0x90 + channel, 0x0F], this, 'LoadSelectedTrack');
    
    this.ejectTrack = function (channel, control, value, status, group) {
        if (value) {
            engine.setValue(that.currentDeck, 'eject', 1);
            engine.beginTimer(250, 'engine.setValue("'+that.currentDeck+'", "eject", 0)', true);
        }
    }

    this.loopSize = new Control([0xB0 + channel, 0x1B], this.currentDeck,
                                null,
                                ['loop_enabled', null]);
    this.loopSize.input = function (channel, control, value, status, group) {
        if (loopEnabledDot) {
            if (value === 127 && loopSize > 2) { // turn left
                /**
                    Unfortunately, there is no way to show 1 with a dot on the
                    loop size LED.
                **/
                loopSize /= 2;
                engine.setValue(that.currentDeck, 'loop_halve', 1);
                engine.setValue(that.currentDeck, 'loop_halve', 0);
            } else if (value === 1 && loopSize < 32) { // turn right
                /**
                    Mixxx supports loops longer than 32 beats, but there is no way
                    to show 64 with a dot on the loop size LED.
                **/
                loopSize *= 2;
                engine.setValue(that.currentDeck, 'loop_double', 1);
                engine.setValue(that.currentDeck, 'loop_double', 0);
            }
        } else {
            if (value === 127 && loopSize > 1/32) { // turn left
                /**
                    Mixxx supports loops shorter than 1/32 beats, but there is no
                    way to set the loop size LED less than 1/32 (even though it
                    should be able to show 1/64)
                **/
                if (loopEnabledDot && loopSize <= 1) {
                    return;
                }
                loopSize /= 2;
                engine.setValue(that.currentDeck, 'loop_halve', 1);
                engine.setValue(that.currentDeck, 'loop_halve', 0);
            } else if (value === 1 && loopSize < 64) { // turn right
                /**
                    Mixxx supports loops longer than 64 beats, but the loop size LED
                    only has 2 digits, so it couldn't show 128
                **/
                if (loopEnabledDot && loopSize >= 32) {
                    // Unfortunately the LED display doesn't have a way to show 64 with a dot.
                    return;
                }
                loopSize *= 2;
                engine.setValue(that.currentDeck, 'loop_double', 1);
                engine.setValue(that.currentDeck, 'loop_double', 0);
            }
        }
        that.loopSize.trigger(); // FIXME: ugly hack around https://bugs.launchpad.net/mixxx/+bug/1567203
    }
    this.loopSize.output = function (value, group, control) {
        if (loopEnabledDot && value) {
            this.send(5 - Math.log(loopSize) / Math.log(2));
        } else {
            this.send(5 + Math.log(loopSize) / Math.log(2));
        }
    }
    this.loopSize.connect();
    this.loopSize.trigger();
    
    this.loopMoveEncoder = function (channel, control, value, status, group) {
        var direction = (value === 127) ? -1 : 1;
        if (loopSize < 1) {
            engine.setValue(that.currentDeck, 'loop_move', loopSize * direction);
        } else {
            engine.setValue(that.currentDeck, 'loop_move', 1 * direction);
        }
    }
    
    this.loopActive = function (channel, control, value, status, group) {
        if (value) {
            if (engine.getValue(that.currentDeck, 'loop_enabled')) {
                engine.setValue(that.currentDeck, 'reloop_exit', 1);
            } else {
                engine.setValue(that.currentDeck, 'beatloop_' + loopSize + '_activate', 1);
            }
        } else {
            if (loopSize <= 1 && engine.getValue(that.currentDeck, 'loop_enabled')) {
                engine.setValue(that.currentDeck, 'reloop_exit', 1);
            }
        }
    }
    
    this.tempoEncoder = function (channel, control, value, status, group) {
        var direction = (value === 127) ? -1 : 1;
        engine.setValue(that.currentDeck, 'rate', engine.getValue(that.currentDeck, 'rate') + (.01 * direction));
    }
    
    this.tempoReset = function (channel, control, value, status, group) {
        if (value) {
            engine.setValue(that.currentDeck, 'rate', 0);
        }
    }
    
    this.beatJump = function (channel, control, value, status, group) {
        var direction = (value === 127) ? -1 : 1;
        if (that.beatJumpEncoderPressed) {
            if (value === 127 && beatJumpSize > 1/32) { // turn left
                beatJumpSize /= 2;
            } else if (value === 1 && beatJumpSize < 64) { // turn right
                beatJumpSize *= 2;
            }
            midi.sendShortMsg(0xB0 + channel, 0x1B, 5 + Math.log(beatJumpSize) / Math.log(2));
        } else {
            engine.setValue(that.currentDeck, 'beatjump', direction * beatJumpSize);
        }
    }
    
    this.beatJumpPress = function (channel, control, value, status, group) {
        if (value === 127) {
            that.beatJumpEncoderPressed = true;
            midi.sendShortMsg(0xB0 + channel, 0x1B, 5 + Math.log(beatJumpSize) / Math.log(2));
        } else {
            that.beatJumpEncoderPressed = false;
            midi.sendShortMsg(0xB0 + channel, 0x1B, 5 + Math.log(loopSize) / Math.log(2));
        }
    }
}
