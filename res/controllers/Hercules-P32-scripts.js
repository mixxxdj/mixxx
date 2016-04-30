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

    this.setValue = function (value) {
        engine.setValue(this.group, this.inCo, value);
    };
    
    this.getValue = function () {
        return engine.getValue(this.group, this.inCo);
    };
    
    this.toggle = function () {
        this.setValue( ! this.getValue());
    };
    
    this.inSetup = function (inOptions) {
        if (inOptions === null) {
            print("Control.inSetup() called with null argument.");
            return;
        }
        this.inCo = inOptions[0];
        this.inFunc = inOptions[1];
        if (this.inFunc === undefined) {this.inFunc = function (value) {return value;}};
        this.onlyOnPress = inOptions[2];
        if (this.inFunc === null) {
            this.input = null;
        } else {
            if (this.onlyOnPress === undefined) {this.onlyOnPress = true};
            this.input = function (channel, control, value, status, group) {
                // FIXME for 2.1: hacks around https://bugs.launchpad.net/mixxx/+bug/1567203
                if (that.onlyOnPress) {
                    if (value > 0) {
                        that.setValue(that.inFunc.call(that, value));
                    }
                } else {
                    that.setValue(that.inFunc.call(that, value));
                }
            }
        }
    }
    this.inSetup(inOptions);
    this.previousInput = null;

    var connection;
    this.connect = function () {
        connection = engine.connectControl(this.group, this.outCo, this.output);
    };
    this.disconnect = function () { connection.disconnect(); };
    //  this.trigger = function() { outConnection.trigger(); };
    //     this.outConnect = function () { engine.connectControl(that.group, that.outCo, that.output); };
    //     this.disconnect = function () {
    //         print('disconnecting: ' + that.group + ' , ' + that.outCo + ' , ' + that.output);
    //         engine.connectControl(that.group, that.outCo, that.output, true);};
    this.trigger = function() { engine.trigger(this.group, this.outCo); };
    this.send = function (value) { midi.sendShortMsg(this.midi.status, this.midi.note, value) };

    this.outSetup = function (outOptions) {
        if (outOptions === null) {
            print("Control.outSetup() called with null argument.");
            return;
        }
        this.outCo = outOptions[0];
        this.outFunc = outOptions[1];
        var connect = outOptions[2];
        var trigger = outOptions[3];
        if (this.outFunc === undefined) {this.outFunc = function (value) {return value;}};
        if (connect === undefined) {connect = true};
        if (trigger === undefined) {trigger = true};
        if (this.outFunc === null) {
            this.output = null;
        } else {
            this.output = function (value, group, control) {
                this.send(this.outFunc.call(this, value));
            }
        }
        if (connect) { this.connect() };
        if (trigger) { this.trigger() };
    }
    this.outSetup(outOptions);
    this.previousOutput = null;
}

// for buttons that toggle a binary CO
var ToggleButton = function (signals, group, co, onlyOnPress, on, off) {
    var that = this; // FIXME for 2.1: hacks around https://bugs.launchpad.net/mixxx/+bug/1567203
    if (on === undefined) {on = 127};
    if (off === undefined) {off = 0};
    Control.call(this, signals, group,
                [co, function () { return ! that.getValue(); }, onlyOnPress],
                [co, function () { return (that.getValue()) ? on : off } ]);
}
ToggleButton.prototype = Object.create(Control.prototype);
ToggleButton.prototype.constructor = ToggleButton;

// for buttons that toggle a binary CO but their LEDs respond to a different CO
var ToggleButtonAsymmetric = function (signals, group, inCo, outCo, onlyOnPress, on, off) {
    var that = this; // FIXME for 2.1: hacks around https://bugs.launchpad.net/mixxx/+bug/1567203
    if (on === undefined) {on = 127};
    if (off === undefined) {off = 0};
    Control.call(this, signals, group,
                [inCo, function () { return ! that.getValue(); }, onlyOnPress],
                [outCo, function (value) { return (value) ? on : off } ]);
}
ToggleButtonAsymmetric.prototype = Object.create(Control.prototype);
ToggleButtonAsymmetric.prototype.constructor = ToggleButtonAsymmetric;

var CueButton = function (signals, group, on, off) {
    if (on === undefined) {on = 127};
    if (off === undefined) {off = 0};
    ToggleButtonAsymmetric.call(this, signals, group, 'cue_default', 'cue_indicator', false, on, off);
}
CueButton.prototype = Object.create(ToggleButtonAsymmetric.prototype);
CueButton.prototype.constructor = CueButton;

var PlayButton = function (signals, group, on, off) {
    if (on === undefined) {on = 127};
    if (off === undefined) {off = 0};
    ToggleButtonAsymmetric.call(this, signals, group,
                               'play', 'play_indicator', true, on, off);
}
PlayButton.prototype = Object.create(ToggleButtonAsymmetric.prototype);
PlayButton.prototype.constructor = PlayButton;

var HotcueButton = function (signals, group, hotcueNumber, on, off) {
    if (on === undefined) {on = 127};
    if (off === undefined) {off = 0};
    ToggleButtonAsymmetric.call(this, signals, group,
                               'hotcue_'+hotcueNumber+'_activate', 'hotcue_'+hotcueNumber+'_enabled', false, on, off);
}
HotcueButton.prototype = Object.create(ToggleButtonAsymmetric.prototype);
HotcueButton.prototype.constructor = HotcueButton;

var HotcueClearButton = function (signals, group, hotcueNumber, on, off) {
    if (on === undefined) {on = 127};
    if (off === undefined) {off = 0};
    HotcueButton.call(this, signals, group, hotcueNumber, on, off);
    this.inSetup(['hotcue_'+hotcueNumber+'_clear', function () {return 1;} ]);
}
HotcueClearButton.prototype = Object.create(HotcueButton.prototype);
HotcueClearButton.prototype.constructor = HotcueClearButton;

// for COs that only get set to 1
var ActionButton = function (signals, group, inCo, onlyOnPress) {
    Control.call(this, signals, group,
                [inCo, function () { return 1; }, onlyOnPress],
                null);
}
ActionButton.prototype = Object.create(Control.prototype);
ActionButton.prototype.constructor = ActionButton;

// Continuous Control, for faders and knobs with finite ranges
var CC = function (signals, group, co, softTakeoverInit, max) {
    // Some controllers (like the P32) can be sent a message to tell it to send
    // signals back with the positions of all the controls. It is helpful to
    // send the message in the script's init function, but it requires that
    // soft takeover isn't enabled to work. So, for these controllers, call
    // this constructor function with softTakeoverInit as false.
    var that = this;
    if (softTakeoverInit === undefined) { softTakeoverInit = true; }
    if (max === undefined) { max = 127; };
    Control.call(this, signals, group,
                 [co, null],
                 null);

    this.input = function (channel, control, value, status, group) {
        engine.setParameter(that.group, co, value / max);
    }

    // Faders and knobs don't have any LED feedback, so there is no need to
    // connect a callback function to send MIDI messages when the control changes.
    // However, when switching layers, take care of soft takeover functionality.
    this.connect = function () {
        engine.softTakeover(that.group, that.inCo, true);
    }
    if (softTakeoverInit) {
        this.connect();
    }
    this.disconnect = function () {
        engine.softTakeoverIgnoreNextValue(that.group, that.inCo);
    }
    this.trigger = function () {};
}
CC.prototype = Object.create(Control.prototype);
CC.prototype.constructor = CC;

// FIXME: temporary hack around https://bugs.launchpad.net/mixxx/+bug/1479008
var CCLin = function (signals, group, co, softTakeoverInit, low, high, min, max) {
    var that = this;
    CC.call(this, signals, group, co, softTakeoverInit);
    this.input = function (channel, control, value, status, group) {
        engine.setValue(that.group, that.inCo, script.absoluteLin(value, low, high, min, max));
    }
}
CCLin.prototype = Object.create(CC.prototype);
CCLin.prototype.constructor = CCLin;

// FIXME: temporary hack around https://bugs.launchpad.net/mixxx/+bug/1479008
var CCNonLin = function (signals, group, co, softTakeoverInit, low, mid, high, min, max) {
    var that = this;
    CC.call(this, signals, group, co, softTakeoverInit);
    this.input = function (channel, control, value, status, group) {
        engine.setValue(that.group, that.inCo, script.absoluteNonLin(value, low, mid, high, min, max));
    }
}
CCNonLin.prototype = Object.create(CC.prototype);
CCNonLin.prototype.constructor = CCNonLin;

var LayerContainer = function (initialLayer) {
    this.forEachControl = function (operation, recursive) {
        if (typeof operation !== 'function') {
            print('ERROR: LayerContainer.forEachContainer requires a function argument');
            return;
        }
        if (recursive === undefined) { recursive = true; };
        for (var memberName in this) {
            if (this.hasOwnProperty(memberName)) {
                if (this[memberName] instanceof Control) {
                    operation.call(this, this[memberName]);
                } else if (recursive && this[memberName] instanceof LayerContainer) {
                    this[memberName].forEachControl(iterative);
                }
            }
        }
    }
    
    this.reconnectControls = function (operation) {
        this.forEachControl(function (control) {
            control.disconnect();
            if (typeof operation === 'function') {
                operation.call(this, control);
            }
            control.connect();
            control.trigger();
        });
    }
    
    this.applyLayer = function (newLayer, operation) {
        //FIXME: What is the best way to implement script.extend?
        script.extend(true, this, newLayer);
        this.reconnectControls(operation);
    };
}

script.samplerRegEx = /\[Sampler(\d+)\]/
script.channelRegEx = /\[Channel(\d+)\]/
script.eqKnobRegEx = /\[EqualizerRack1_\[(.*)\]_Effect1\]/
script.quickEffectRegEx = /\[QuickEffectRack1_\[(.*)\]\]/

var Deck = function (deckNumbers) {
    LayerContainer.call(this);
    this.currentDeck = '[Channel' + deckNumbers[0] + ']';
    
    this.toggle = function () {
        var index = deckNumbers.indexOf(parseInt(script.channelRegEx.exec(this.currentDeck)[1]));
        if (index === (deckNumbers.length - 1)) {
            index = 0;
        } else {
            index += 1;
        }
        this.currentDeck = "[Channel" + deckNumbers[index] + "]";

        this.reconnectControls(function (control) {
            if (control.group.search(script.eqKnobRegEx) !== -1) {
                control.group = '[EqualizerRack1_' + this.currentDeck + '_Effect1]';
            } else if (control.group.search(script.quickEffectRegEx) !== -1) {
                control.group = '[QuickEffectRack1_' + this.currentDeck + ']';
            } else {
                control.group = this.currentDeck;
            }
        });
    }
}
Deck.prototype = Object.create(LayerContainer.prototype);
Deck.prototype.constructor = Deck;

var P32 = {};

P32.init = function () {
    P32.leftDeck = new P32.Deck([1,3], 1);
    P32.rightDeck = new P32.Deck([2,4], 2);
    // tell controller to send MIDI messages with positions of faders and knobs
    midi.sendShortMsg(0xB0, 0x7F, 0x7F);
}

P32.shutdown = function () {};

P32.shiftOffset = 3;

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

P32.record = new Control([0x90, 0x02], '[Recording]',
                         ['toggle_recording', null],
                         ['status', function (val) {return val * 127}]);
P32.record.input = function (channel, control, value, status, group) {
    if (value === 127) {
        if (P32.leftDeck.shift) {
            P32.leftDeck.toggle();
        } else if (P32.rightDeck.shift) {
            P32.rightDeck.toggle();
        } else {
            // FIXME for 2.1: hacks around https://bugs.launchpad.net/mixxx/+bug/1567203
            // change to this.setValue(1)
            engine.setValue('[Recording]', 'toggle_recording', 1);
        }
    }
}

P32.EffectUnit = function (unitNumber) {
    var that = this;
    this.group = '[EffectRack1_EffectUnit' + unitNumber + ']';

    // deck enable buttons
    for (var d = 1; d <= 4; d++) {
        // FIXME for 2.1: hacks around https://bugs.launchpad.net/mixxx/+bug/1565377
        this['deckButton' + d] = new ToggleButton([0x90 + unitNumber, 0x02 + d], this.group, 'group_[Channel' + d + ']_enable');
    }
    this.pflToggle = function () {
        script.toggleControl(this.group, 'group_[Headphone]_enable');
    }

    this.dryWet = new CCLin([0xB0 + unitNumber, 0x09],
                         this.group, 'mix', false, 0, 1);
    this.superKnob = new CCLin([0xB0 + unitNumber, 0x09],
                         this.group, 'super1', true, 0, 1);

    this.activeEffect = new LayerContainer();
    for (var p = 1; p <= 3; p++) {
        // FIXME for 2.1: hacks around https://bugs.launchpad.net/mixxx/+bug/1565377
        this.activeEffect['parameterKnob' + p] = new CC([0xB0 + unitNumber, 0x06],
                                           '[EffectRack1_EffectUnit' + unitNumber + '_Effect1]', 'parameter' + p, false);
    }

    // buttons to select the effect that the knobs control
    this.switchEffect = function (effectNumber) {
        this.activeEffect.reconnectControls(function (control) {
            control.group = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + effectNumber + ']';
        });

        for (var e = 1; e < 4; e ++) {
            midi.sendShortMsg(0x90 + unitNumber + P32.shiftOffset,
                              0x02 + e,
                              (e === effectNumber) ? 127 : 0);
        }
    }
    this.switchEffect1 = function (channel, control, value, status, group) { that.switchEffect(1); };
    this.switchEffect2 = function (channel, control, value, status, group) { that.switchEffect(2); };
    this.switchEffect3 = function (channel, control, value, status, group) { that.switchEffect(3); };
    this.switchEffect4 = function (channel, control, value, status, group) { that.switchEffect(4); };
};

P32.Deck = function (deckNumbers, channel) {
    Deck.call(this, deckNumbers);
    var that = this;
    var loopSize = defaultLoopSize;
    var beatJumpSize = defaultBeatJumpSize;
    this.shift = false;
    
    this.effectUnit = new P32.EffectUnit(deckNumbers[0]);
    
    this.shiftButton = function (channel, control, value, status, group) {
        if (value === 127) {
            that.shift = true;
            that.effectUnit.dryWet.connect();
            that.effectUnit.dryWet.disconnect();
        } else {
            that.shift = false;
            that.effectUnit.superKnob.disconnect();
        }
    }
    
    this.sync = new ToggleButton([0x90 + channel, 0x08], this.currentDeck, 'sync_enabled');
    this.cue = new CueButton([0x90 + channel, 0x09], this.currentDeck);
    this.play = new PlayButton([0x90 + channel, 0x0A], this.currentDeck);
    
    this.quantize = new ToggleButton([0x90 + channel + P32.shiftOffset, 0x08], this.currentDeck, 'quantize'); // sync shifted
    this.keylock = new ToggleButton([0x90 + channel + P32.shiftOffset, 0x09], this.currentDeck, 'keylock'); // cue shifted
    this.goToStart = new Control([0x90 + channel + P32.shiftOffset, 0x0A], this.currentDeck, // play shifted
                            ['start_stop', function () { return 1; }],
                            ['play_indicator', function (val) { return val * 127 }]);
    
    for (var i = 1; i <= 16; i++) {
        // FIXME for 2.1: hacks around https://bugs.launchpad.net/mixxx/+bug/1565377
        this['hotcueButton' + i] = new HotcueButton([0x90 + channel, P32.PadNumToMIDIControl(i)], this.currentDeck,
                                                    i, P32.padColors.red, P32.padColors.off);
        this['hotcueButtonShift' + i] = new HotcueClearButton([0x90 + channel + P32.shiftOffset, P32.PadNumToMIDIControl(i)], this.currentDeck,
                                                              i, P32.padColors.red, P32.padColors.off);
        this['samplerButton' + i] = new ToggleButtonAsymmetric([0x90
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
    
    for (var k = 1; k <= 3; k++) {
        this['eqKnob' + k] = new CCNonLin([0xB0 + channel, 0x02 + k],
                                    '[EqualizerRack1_' + this.currentDeck + '_Effect1]',
                                    'parameter' + k,
                                    false,
                                    0, 1, 4);
    }

    this.volume = new CCNonLin([0xB0 + channel, 0x01], this.currentDeck, 'volume', false, 0, .25, 1);

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
    
    this.loopToggle = function (channel, control, value, status, group) {
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
    
    this.beatJumpEncoder = function (channel, control, value, status, group) {
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

    this.loadTrack = function (channel, control, value, status, group) {
        if (value === 127) {
            engine.setValue(that.currentDeck, 'LoadSelectedTrack', 1);
        }
    }

    this.ejectTrack = function (channel, control, value, status, group) {
        if (value === 127) {
            engine.setValue(that.currentDeck, 'eject', 1);
            engine.beginTimer(250, 'engine.setValue("'+that.currentDeck+'", "eject", 0)', true);
        }
    }
}
P32.Deck.prototype = Object.create(Deck.prototype);
P32.Deck.prototype.constructor = P32.Deck;
