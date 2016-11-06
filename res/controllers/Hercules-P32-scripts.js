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
    /**
    A Control represents a physical component on a controller, such as a button, knob, encoder, or fader.
    This constructor function can help set up both the input and output handling functions easily.
    Derivative objects are provided below with even more convenient constructor functions for common use cases.

    Any of these parameters can be null or ommitted, which can be helpful when setting this Control's properties after initialization.
    It is also helpful to intentionally avoid overwriting certain properties of Controls using LayerContainer.applyLayer().

    signals, array with 2 members: first two bytes of the MIDI message (status and note numbers)
    group, string: the group this belongs to, for example '[Channel1]'
    inOptions, array with up to 3 members:
        1st member of array, string: Mixxx CO that this affects when receiving MIDI input, for example, 'play'
        2nd member of array, function: function to execute upon receiving MIDI input. The function should return
                                       the value to set the 1st member of the array to. The function's 'this' object
                                       is set to the Control. If null, you need to set the inFunc or input property yourself,
                                       which is helpful if you want to define a long, custom input function.
        3rd member of array, boolean, optional: whether to react only when a button is pressed, or on both press & release.
                                                Default to true if ommitted.
    outOptions, array with up to 4 members, optional: If null or ommitted, you need to set up the output yourself.
        1st member of array, string: send signals back to the controller when this Mixxx CO changes, for example, 'play_indicator'
        2nd member of array, function: function to execute when Mixxx CO specified by 1st member of array changes.
                                       The function should return a value to send back as the 3rd byte of a MIDI message
                                       (with the first 2 bytes of the MIDI message being those specified in the first argument).
                                       The function's 'this' object is set to the Control. If null, you need to set the input property yourself,
                                       which is helpful if you want to define a long, custom output function.
        3rd member of array, boolean, optional: whether to connect the Mixxx CO (1st member) to the function (2nd member) immediately.
                                                Default to true if ommitted.
        4th member of array, boolean, optional: whether to execute the function (2nd member) immediately. Default to true if ommitted.
    **/
    if (arguments.length === 1 && arguments[0] === null) {
        this = null;
        return;
    }

    var that = this;

    if (Array.isArray(signals)) {
        this.midi = {status: signals[0], note: signals[1]};
    }
    if (typeof group === 'string') {
        this.group = group;
    }

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
        if (inOptions === null || inOptions === undefined) {
            return;
        }
        this.inCo = inOptions[0];
        var inFunc = inOptions[1];
        if (inFunc === undefined) {inFunc = function (value) {return value;};}
        this.onlyOnPress = inOptions[2];
        if (this.onlyOnPress === undefined) {this.onlyOnPress = true;}
        this.input = function (channel, control, value, status, group) {
            // FIXME for 2.1: hacks around https://bugs.launchpad.net/mixxx/+bug/1567203
            if (that.onlyOnPress) {
                if (value > 0) {
                    that.setValue(inFunc.call(that, value));
                }
            } else {
                that.setValue(inFunc.call(that, value));
            }
        };
    };
    this.inSetup(inOptions);
    this.previousInput = null;

    this.connections = [];
    this.connect = function () {
        /**
        Override this method with a custom one to connect multiple Mixxx COs
        for a single Control. Add the connection objects to the this.connections array so
        they all get disconnected just by calling this.disconnect().
        This can be helpful for multicolor LEDs that show a different color depending
        on the state of different Mixxx COs. See SamplerButton.connect()
        and SamplerButton.output() for an example.
        **/
        this.connections[0] = engine.connectControl(this.group, this.outCo, this.output);
    };
    this.disconnect = function () {
        this.connections.forEach(function (connection) {
            connection.disconnect();
        });
    };
    this.trigger = function() { engine.trigger(this.group, this.outCo); };
    this.send = function (value) { midi.sendShortMsg(this.midi.status, this.midi.note, value); };

    this.outSetup = function (outOptions) {
        if (outOptions === null || outOptions === undefined) {
            return;
        }
        this.outCo = outOptions[0];
        this.outFunc = outOptions[1];
        var connect = outOptions[2];
        var trigger = outOptions[3];
        if (this.outFunc === undefined) {this.outFunc = function (value) {return value;};}
        if (connect === undefined) {connect = true;}
        if (trigger === undefined) {trigger = true;}
        if (this.outFunc === null) {
            this.output = null;
        } else {
            this.output = function (value, group, control) {
                this.send(this.outFunc.call(this, value));
            };
        }
        if (connect) { this.connect(); }
        if (trigger) { this.trigger(); }
    };
    this.outSetup(outOptions);
    this.previousOutput = null;
};

var ToggleButton = function (signals, group, co, onlyOnPress, on, off) {
    /**
    A Control that toggles a binary Mixxx CO

    signals: two member array, first two bytes of the MIDI message (status and note numbers)
    group: string, the group this belongs to, for example '[Channel1]'
    co, string: the Mixxx CO to toggle
    onlyOnPress, boolean, optional: whether to toggle the CO only when the button is pressed,
                                    or on both press and release. Defaults to true if ommited.
    on, number, optional: value for the 3rd byte of the MIDI message to send back to light the LED when the Mixxx CO is on
                          Defaults to 127 if ommited. Use this if the button has a multicolor LED.
    off, number, optional: value for the 3rd byte of the MIDI message to send back to turn off the LED when the Mixxx CO is off
                           Defaults to 0 if ommitted.
    **/
    if (on === undefined) {on = 127;}
    if (off === undefined) {off = 0;}
    Control.call(this, signals, group,
                [co, function () { return ! this.getValue(); }, onlyOnPress],
                [co, function () { return (this.getValue()) ? on : off; } ]);
};
ToggleButton.prototype = Object.create(Control.prototype);
ToggleButton.prototype.constructor = ToggleButton;

var ToggleButtonAsymmetric = function (signals, group, inCo, outCo, onlyOnPress, on, off) {
    /**
    A Control that toggles a binary Mixxx CO, but its LEDs respond to a different CO

    signals: two member array, first two bytes of the MIDI message (status and note numbers)
    group: string, the group this belongs to, for example '[Channel1]'
    inCo, string: the Mixxx CO to toggle when receiving MIDI input
    outCo, string: send signals back to the controller when this Mixxx CO changes
    onlyOnPress, boolean, optional: whether to toggle inCO only when the button is pressed,
                                    or on both press and release. Defaults to true if ommited.
    on, number, optional: value for the 3rd byte of the MIDI message to send back to light the LED when outCo on
                          Defaults to 127 if ommited. Use this if the button has a multicolor LED.
    off, number, optional: value for the 3rd byte of the MIDI message to send back to turn off the LED when outCo is off
                           Defaults to 0 if ommitted.
    **/
    if (on === undefined) {on = 127;}
    if (off === undefined) {off = 0;}
    Control.call(this, signals, group,
                [inCo, function () { return ! this.getValue(); }, onlyOnPress],
                [outCo, function (value) { return (value) ? on : off; } ]);
};
ToggleButtonAsymmetric.prototype = Object.create(Control.prototype);
ToggleButtonAsymmetric.prototype.constructor = ToggleButtonAsymmetric;

var CueButton = function (signals, group, on, off) {
    /**
    A Control for cue buttons

    signals: two member array, first two bytes of the MIDI message (status and note numbers)
    group: string, the group this belongs to, for example '[Channel1]'
    on, number, optional: value for the 3rd byte of the MIDI message to send back to light the LED when cue_indicator is 1
                          Defaults to 127 if ommited. Use this if the button has a multicolor LED.
    off, number, optional: value for the 3rd byte of the MIDI message to send back to turn off the LED when cue_indicator is 0
                           Defaults to 0 if ommitted.
    **/
    if (on === undefined) {on = 127;}
    if (off === undefined) {off = 0;}
    ToggleButtonAsymmetric.call(this, signals, group, 'cue_default', 'cue_indicator', false, on, off);
};
CueButton.prototype = Object.create(ToggleButtonAsymmetric.prototype);
CueButton.prototype.constructor = CueButton;

var PlayButton = function (signals, group, on, off) {
    /**
    A Control for play buttons

    signals: two member array, first two bytes of the MIDI message (status and note numbers)
    group: string, the group this belongs to, for example '[Channel1]'
    on, number, optional: value for the 3rd byte of the MIDI message to send back to light the LED when play_indicator is 1
                          Defaults to 127 if ommited. Use this if the button has a multicolor LED.
    off, number, optional: value for the 3rd byte of the MIDI message to send back to turn off the LED when play_indicator is 0
                           Defaults to 0 if ommitted.
    **/
    if (on === undefined) {on = 127;}
    if (off === undefined) {off = 0;}
    ToggleButtonAsymmetric.call(this, signals, group,
                               'play', 'play_indicator', true, on, off);
};
PlayButton.prototype = Object.create(ToggleButtonAsymmetric.prototype);
PlayButton.prototype.constructor = PlayButton;


var ActionButton = function (signals, group, inCo, on) {
    Control.call(this,
                 signals, group,
                 [inCo, function () { return 1; }],
                 null);

   if (on !== undefined) {
       this.send(on);
   }
}

var LoopToggleButton = function (signals, group, on, off) {
    if (on === undefined) {on = 127;}
    if (off === undefined) {off = 0;}
    Control.call(this,
        signals, group,
        ['reloop_exit', function () { return 1; }],
        ['loop_enabled', function (value) { return (value) ? on : off; } ]);
}
LoopToggleButton.prototype = Object.create(Control.prototype);
LoopToggleButton.prototype.constructor = LoopToggleButton;

var HotcueButton = function (signals, group, hotcueNumber, on, off) {
    /**
    A Control for hotcue buttons

    signals: two member array, first two bytes of the MIDI message (status and note numbers)
    group: string, the group this belongs to, for example '[Channel1]'
    hotcueNumber, number: the number of the hotcue
    on, number, optional: value for the 3rd byte of the MIDI message to send back to light the LED when cue_indicator is 1
                          Defaults to 127 if ommited. Use this if the button has a multicolor LED.
    off, number, optional: value for the 3rd byte of the MIDI message to send back to turn off the LED when cue_indicator is 0
                           Defaults to 0 if ommitted.
    **/
    if (on === undefined) {on = 127;}
    if (off === undefined) {off = 0;}
    ToggleButtonAsymmetric.call(this, signals, group,
                               'hotcue_'+hotcueNumber+'_activate', 'hotcue_'+hotcueNumber+'_enabled', false, on, off);
};
HotcueButton.prototype = Object.create(ToggleButtonAsymmetric.prototype);
HotcueButton.prototype.constructor = HotcueButton;

var HotcueClearButton = function (signals, group, hotcueNumber, on, off) {
    /**
    A Control for buttons to clear a hotcue. Typically, these are the same buttons as HotcueButtons,
    but active with a shift button held.

    signals: two member array, first two bytes of the MIDI message (status and note numbers)
    group: string, the group this belongs to, for example '[Channel1]'
    hotcueNumber, number: the number of the hotcue
    on, number, optional: value for the 3rd byte of the MIDI message to send back to light the LED when cue_indicator is 1
                          Defaults to 127 if ommited. Use this if the button has a multicolor LED.
    off, number, optional: value for the 3rd byte of the MIDI message to send back to turn off the LED when cue_indicator is 0
                           Defaults to 0 if ommitted.
    **/
    if (on === undefined) {on = 127;}
    if (off === undefined) {off = 0;}
    HotcueButton.call(this, signals, group, hotcueNumber, on, off);
    this.inSetup(['hotcue_'+hotcueNumber+'_clear', function () {return 1;} ]);
};
HotcueClearButton.prototype = Object.create(HotcueButton.prototype);
HotcueClearButton.prototype.constructor = HotcueClearButton;

/**
A Control for sampler buttons. Press the button to load the track selected in
the library into an empty sampler. Press a loaded sampler to play it from
its cue point. Press again while playing to jump back to the cue point.

@param {Array} signals: first two bytes of the MIDI message (status and note numbers)
@param {Integer} samplerNumber: number of the sampler
@param {Number} on (optional): MIDI value to send back to button LED when sampler
                               is loaded. Defaults to 127. Use this to for multicolor LEDs.
@param {Number} off (optional): MIDI value to send back to button LED when sampler
                                is empty. Defaults to 0.
@param {Number} playing (optional): MIDI value to send back to button LED when sampler
                                    is playing. Useful for buttons with multicolor LEDs.
                                    If ommitted, only the MIDI message specified by
                                    the "on" parameter is sent when the sampler
                                    is loaded, regardless of whether it is playing.
**/
var SamplerButton = function (signals, samplerNumber, on, off, playing) {
    if (on === undefined) {on = 127;}
    if (off === undefined) {off = 0;}
    // track_samples is 0 when the sampler is empty and > 0 when a sample is loaded
    Control.call(this, signals, '[Sampler' + samplerNumber + ']', null,
                 null);
    var that = this; // FIXME for 2.1: hacks around https://bugs.launchpad.net/mixxx/+bug/1567203

    this.input = function (channel, control, value, status, group) {
        if (value > 0) {
            if (engine.getValue(that.group, 'track_samples') === 0) {
                engine.setValue(that.group, 'LoadSelectedTrack', 1);
            } else {
                engine.setValue(that.group, 'cue_gotoandplay', 1);
            }
        }
    };

    this.output = function (value, group, control) {
        if (engine.getValue(that.group, 'track_samples') > 0) {
            if (playing === undefined) {
                that.send(on);
            } else {
                if (engine.getValue(that.group, 'play') === 1) {
                    that.send(on);
                } else {
                    that.send(playing);
                }
            }
        } else {
            that.send(off);
        }
    };

    this.connect = function() {
        this.connections[0] = engine.connectControl(this.group, 'track_samples', this.output);
        if (playing !== undefined) {
            this.connections[1] = engine.connectControl(this.group, 'play', this.output);
        }
    };

    this.connect();
    this.trigger();
}
SamplerButton.prototype = Object.create(Control.prototype);
SamplerButton.prototype.constructor = SamplerButton;

/**
A Control for buttons to stop samplers. Typically, these are the same buttons
as SamplerButtons but active with shift held down. If the sampler is stopped,
eject the loaded sample.

@param {Array} signals: first two bytes of the MIDI message (status and note numbers)
@param {Integer} samplerNumber: number of the sampler
@param {Number} on (optional): MIDI value to send back to button LED when sampler
                               is loaded. Defaults to 127. Use this to for multicolor LEDs.
@param {Number} off (optional): MIDI value to send back to button LED when sampler
                                is empty. Defaults to 0.
@param {Number} playing (optional): MIDI value to send back to button LED when sampler
                                    is playing. Useful for buttons with multicolor LEDs.
                                    If ommitted, only the MIDI message specified by
                                    the "on" parameter is sent when the sampler
                                    is loaded, regardless of whether it is playing.
**/
var SamplerStopButton = function (signals, samplerNumber, on, off, playing) {
    SamplerButton.call(this, signals, samplerNumber, on, off, playing);

    var that = this; // FIXME for 2.1: hacks around https://bugs.launchpad.net/mixxx/+bug/1567203

    this.input = function (channel, control, value, status, group) {
        if (value > 0) {
            if (engine.getValue(that.group, 'play') === 1) {
                engine.setValue(that.group, 'play', 0);
            } else {
                engine.setValue(that.group, 'eject', 1);
            }
        }
    };
}
SamplerStopButton.prototype = Object.create(SamplerButton);
SamplerStopButton.prototype.constructor = SamplerStopButton;

var CC = function (signals, group, co, softTakeoverInit, max) {
    /**
    A Control for faders and knobs with finite ranges. Although these do not
    respond to a change in a Mixxx CO to send MIDI signals back to the controller,
    using a CC is helpful because Control.connect and Control.disconnect are
    overwritten to take care of soft takeover when switching layers with LayerContainer.applyLayer().

    signals: two member array, first two bytes of the MIDI message (status and note numbers)
    group: string, the group this belongs to, for example '[Channel1]'
    co, string: the Mixxx CO to change
    softTakeoverInit, boolean, optional: Whether to activate soft takeover upon initialization. Defaults to true if ommitted.
                                         Some controllers (like the Hercules P32) can be sent a message to tell it to send
                                         signals back with the positions of all the controls. It is helpful to send the message
                                         in the script's init function, but it requires that soft takeover isn't enabled to work.
                                         So, for these controllers, call this constructor function with softTakeoverInit as false.
    max, number, optional: the maximum value received from the controller. Defaults to 127 if ommitted.
    **/
    var that = this;
    if (softTakeoverInit === undefined) { softTakeoverInit = true; }
    if (max === undefined) { max = 127; }
    Control.call(this, signals, group,
                 [co, null],
                 null);

    this.input = function (channel, control, value, status, group) {
        engine.setParameter(that.group, co, value / max);
    };

    this.connect = function () {
        engine.softTakeover(that.group, that.inCo, true);
    };
    if (softTakeoverInit) {
        this.connect();
    }
    this.disconnect = function () {
        engine.softTakeoverIgnoreNextValue(that.group, that.inCo);
    };
    this.trigger = function () {};
};
CC.prototype = Object.create(Control.prototype);
CC.prototype.constructor = CC;

// FIXME: temporary hack around https://bugs.launchpad.net/mixxx/+bug/1479008
var CCLin = function (signals, group, co, softTakeoverInit, low, high, min, max) {
    /**
    A CC for Mixxx COs with linear responses, because engine.softTakeover()
    doesn't work with soft takeover yet.

    signals: two member array, first two bytes of the MIDI message (status and note numbers)
    group: string, the group this belongs to, for example '[Channel1]'
    co, string: the Mixxx CO to change
    softTakeoverInit, boolean, optional: Whether to activate soft takeover upon initialization. Defaults to true if ommitted.
                                         Some controllers (like the Hercules P32) can be sent a message to tell it to send
                                         signals back with the positions of all the controls. It is helpful to send the message
                                         in the script's init function, but it requires that soft takeover isn't enabled to work.
                                         So, for these controllers, call this constructor function with softTakeoverInit as false.
    low, high, min, max: arguments for script.absoluteLin()
    **/
    var that = this;
    CC.call(this, signals, group, co, softTakeoverInit);
    this.input = function (channel, control, value, status, group) {
        engine.setValue(that.group, that.inCo, script.absoluteLin(value, low, high, min, max));
    };
};
CCLin.prototype = Object.create(CC.prototype);
CCLin.prototype.constructor = CCLin;

// FIXME: temporary hack around https://bugs.launchpad.net/mixxx/+bug/1479008
var CCNonLin = function (signals, group, co, softTakeoverInit, low, mid, high, min, max) {
    /**
    A CC for Mixxx COs with nonlinear responses, because engine.softTakeover()
    doesn't work with soft takeover yet.

    signals: two member array, first two bytes of the MIDI message (status and note numbers)
    group: string, the group this belongs to, for example '[Channel1]'
    co, string: the Mixxx CO to change
    softTakeoverInit, boolean, optional: Whether to activate soft takeover upon initialization. Defaults to true if ommitted.
                                         Some controllers (like the Hercules P32) can be sent a message to tell it to send
                                         signals back with the positions of all the controls. It is helpful to send the message
                                         in the script's init function, but it requires that soft takeover isn't enabled to work.
                                         So, for these controllers, call this constructor function with softTakeoverInit as false.
    low, mid, high, min, max: arguments for script.absoluteNonLin()
    **/
    var that = this;
    CC.call(this, signals, group, co, softTakeoverInit);
    this.input = function (channel, control, value, status, group) {
        engine.setValue(that.group, that.inCo, script.absoluteNonLin(value, low, mid, high, min, max));
    };
};
CCNonLin.prototype = Object.create(CC.prototype);
CCNonLin.prototype.constructor = CCNonLin;

var LayerContainer = function (initialLayer) {
    /**
    A LayerContainer is an object that contains Controls as properties, with
    methods to help manipulate the Controls. Layers are merely objects that
    contain Controls to overwrite the active Controls of a LayerContainer. Layers
    are deeply merged with the applyLayer() method, so if a new layer does not
    define a property for a Control, the Control's old property will be retained.
    To avoid defining properties of Controls, pass null as an argument to the
    Control constructor function.

    initialLayer, object, optional: the layer to activate upon initialization
    **/
    this.forEachControl = function (operation, recursive) {
        /**
        operation, function that takes 1 argument: the function to call for each Control.
                                                   Takes each Control as its first argument.
        recursive, boolean, optional: whether to call forEachControl recursively
                                      for each LayerContainer within this LayerContainer.
                                      Defaults to true if ommitted.
        **/
        if (typeof operation !== 'function') {
            print('ERROR: LayerContainer.forEachContainer requires a function argument');
            return;
        }
        if (recursive === undefined) { recursive = true; }

        var that = this;
        var applyOperationTo = function (obj) {
            if (obj instanceof Control) {
                operation.call(that, obj);
            } else if (recursive && obj instanceof LayerContainer) {
                obj.forEachControl(op);
            } else if (Array.isArray(obj)) {
                obj.forEach(function (element) {
                    applyOperationTo(element);
                });
            }
        };

        for (var memberName in this) {
            if (this.hasOwnProperty(memberName)) {
                applyOperationTo(this[memberName]);
            }
        }
    };

    this.reconnectControls = function (operation) {
        /**
        operation, function that takes one argument, optional: a function to call for each Control in this LayerContainer
                                                               before reconnecting the output callback.
                                                               The Control is passed as the first argument.
        **/
        this.forEachControl(function (control) {
            control.disconnect();
            if (typeof operation === 'function') {
                operation.call(this, control);
            }
            control.connect();
            control.trigger();
        });
    };

    this.applyLayer = function (newLayer, operation) {
        //FIXME: What is the best way to implement script.extend?
        //There is a pure JS port of jQuery's extend method at https://github.com/justmoon/node-extend under the MIT License
        //Copy that into common-controller-scripts.js? Make it a separate file to include in the XML?
        script.extend(true, this, newLayer); // Recursively merge newLayer with this LayerContainer
        this.reconnectControls(operation);
    };

    if (typeof initialLayer === 'object') {
        this.applyLayer(initialLayer);
    }
};

script.samplerRegEx = /\[Sampler(\d+)\]/ ;
script.channelRegEx = /\[Channel(\d+)\]/ ;
script.eqKnobRegEx = /\[EqualizerRack1_\[(.*)\]_Effect1\]/ ;
script.quickEffectRegEx = /\[QuickEffectRack1_\[(.*)\]\]/ ;

var Deck = function (deckNumbers) {
    /**
    A LayerContainer with a toggle() method for conveniently changing the group
    attributes of contained Controls to switch the deck that a set of Controls
    is manipulating. The toggle() method can be used instead of defining a layer
    for each deck and using LayerContainer.applyLayer().

    deckNumbers, array of numbers, size arbitrary: which deck numbers this can cycle through with the toggle() method
                                                   Typically [1, 3] or [2, 4]
    **/
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
    };
};
Deck.prototype = Object.create(LayerContainer.prototype);
Deck.prototype.constructor = Deck;

var P32 = {};

P32.init = function () {
    P32.leftDeck = new P32.Deck([1,3], 1);
    P32.rightDeck = new P32.Deck([2,4], 2);

    if (engine.getValue('[Master]', 'num_samplers') < 32) {
        engine.setValue('[Master]', 'num_samplers', 32);
    }
    for (var channel = 1; channel <= 2; channel++) {
        for (var s = 1; s <= 16; s++) {
            var samplerNumber = s + (channel - 1) * 16;
            this['sampler' + samplerNumber] = new SamplerButton(
                [0x90 + channel, P32.PadNumToMIDIControl(s, 0)], samplerNumber,
                P32.padColors.red, P32.padColors.off, P32.padColors.blue);
            this['samplerClear' + samplerNumber] = new SamplerStopButton(
                [0x90 + channel + P32.shiftOffset, P32.PadNumToMIDIControl(s, 0)], samplerNumber,
                P32.padColors.red, P32.padColors.off, P32.padColors.blue);
        }
    }
    if (samplerCrossfaderAssign) {
      for (s = 1; s <= 16; s++) {
        engine.setValue('[Sampler' + s + ']', 'orientation', 0);
      }
      for (s = 17; s <= 32; s++) {
        engine.setValue('[Sampler' + s + ']', 'orientation', 2);
      }
    }

    // tell controller to send MIDI messages with positions of faders and knobs
    midi.sendShortMsg(0xB0, 0x7F, 0x7F);
};

P32.shutdown = function () {};

P32.shiftOffset = 3;

P32.padColors = {
    red: 125,
    blue: 126,
    purple: 127
};

P32.PadNumToMIDIControl = function (PadNum, layer) {
    // The MIDI control numbers for the pad grid are numbered bottom to top, so
    // this returns the MIDI control numbers for the pads numbered top to bottom
    // layer argument is the 0-indexed pad mode, from bottom (sampler) to top (hotcue)
    PadNum -= 1;
    var midiRow = 3 - Math.floor(PadNum/4);
    return 0x24 + 16 * layer + midiRow*4 + PadNum%4;
};

P32.browse = function (channel, control, value, status, group) {
    if (value > 64) {
        engine.setValue('[Playlist]', 'SelectPrevTrack', 1);
    } else {
        engine.setValue('[Playlist]', 'SelectNextTrack', 1);
    }
};

P32.headMix = function (channel, control, value, status, group) {
    var direction = (value > 64) ? -1 : 1;
    engine.setValue('[Master]', 'headMix', engine.getValue('[Master]', 'headMix') + (0.25 * direction));
};

P32.record = new Control([0x90, 0x02], '[Recording]',
                         null,
                         ['status', function (val) {return val * 127;} ]);
P32.record.input = function (channel, control, value, status, group) {
    if (value === 127) {
        if (P32.leftDeck.shift) {
            P32.leftDeck.toggle();
        } else if (P32.rightDeck.shift) {
            P32.rightDeck.toggle();
        } else {
            script.toggleControl('[Recording]', 'toggle_recording');
        }
    }
};

P32.EffectUnit = function (unitNumber) {
    var that = this;
    this.group = '[EffectRack1_EffectUnit' + unitNumber + ']';

    // deck enable buttons
    for (var d = 1; d <= 4; d++) {
        // FIXME for 2.1: hacks around https://bugs.launchpad.net/mixxx/+bug/1565377
        this['deckButton' + d] = new ToggleButton(
            [0x90 + unitNumber, 0x02 + d], this.group, 'group_[Channel' + d + ']_enable');
        this['deckButton' + d + 'Shifted'] = new ToggleButton(
            [0x90 + unitNumber + P32.shiftOffset, 0x02 + d], this.group, 'group_[Channel' + d + ']_enable');
    }

    this.dryWet = new CCLin(
        [0xB0 + unitNumber, 0x09], this.group, 'mix', false, 0, 1);
    this.superKnob = new CCLin(
        [0xB0 + unitNumber, 0x09], this.group, 'super1', true, 0, 1);

    this.activeEffect = new LayerContainer();
    for (var p = 1; p <= 3; p++) {
        // FIXME for 2.1: hacks around https://bugs.launchpad.net/mixxx/+bug/1565377
        this.activeEffect['parameterKnob' + p] = new CC(
            [0xB0 + unitNumber, 0x06],
            '[EffectRack1_EffectUnit' + unitNumber + '_Effect1]', 'parameter' + p, false);
    }

    // buttons to select the effect that the knobs control
    this.switchEffect = function (effectNumber) {
        this.activeEffect.reconnectControls(function (control) {
            control.group = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + effectNumber + ']';
        });

        for (var e = 1; e < 4; e ++) {
            midi.sendShortMsg(0x90 + unitNumber,
                              P32.PadNumToMIDIControl(e * 4, 1),
                              (e === effectNumber) ? P32.padColors.blue : 0);
            midi.sendShortMsg(0x90 + unitNumber + P32.shiftOffset,
                              P32.PadNumToMIDIControl(e * 4, 1),
                              (e === effectNumber) ? P32.padColors.blue : 0);
        }
    };
    this.switchEffect1 = function (channel, control, value, status, group) { that.switchEffect(1); };
    this.switchEffect2 = function (channel, control, value, status, group) { that.switchEffect(2); };
    this.switchEffect3 = function (channel, control, value, status, group) { that.switchEffect(3); };
    this.switchEffect4 = function (channel, control, value, status, group) { that.switchEffect(4); };
    this.switchEffect(1);

    var Effect = function (effectNumber) {
        var ef = this;
        this.group = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + effectNumber + ']';
        this.toggle = new ToggleButton(
            [0x90 + unitNumber, P32.PadNumToMIDIControl(effectNumber * 4 - 3, 1)],
            this.group,
            'enabled',
            true,
            P32.padColors.red);

        this.resetParameters = new ToggleButtonAsymmetric(
            [0x90 + unitNumber + P32.shiftOffset, P32.PadNumToMIDIControl(effectNumber * 4 - 3, 1)],
            this.group,
            null,
            'enabled',
            true,
            P32.padColors.red);
        this.resetParameters.input = function (channel, control, value, status, group) {
            if (value === 127) {
                var p = engine.getValue(ef.group, 'num_parameters');
                for (var i = 1; i <= p; i++) {
                    engine.setValue(ef.group, 'parameter' + i + '_set_default', 1);
                }
                var b = engine.getValue(ef.group, 'num_button_parameters');
                for (var i = 1; i <= b; i++) {
                    engine.setValue(ef.group, 'button_parameter' + i, 0);
                }
            }
        }

        this.previous = new ToggleButtonAsymmetric(
            [0x90 + unitNumber, P32.PadNumToMIDIControl(effectNumber * 4 - 2, 1)],
            this.group,
            'prev_effect',
            null,
            false);
        this.previousShifted = new ToggleButtonAsymmetric(
            [0x90 + unitNumber + P32.shiftOffset, P32.PadNumToMIDIControl(effectNumber * 4 - 2, 1)],
            this.group,
            'prev_effect',
            null,
            false);
        midi.sendShortMsg(0x90 + unitNumber,
                          P32.PadNumToMIDIControl(effectNumber * 4 - 2, 1),
                          P32.padColors.purple);
        midi.sendShortMsg(0x90 + unitNumber + P32.shiftOffset,
                          P32.PadNumToMIDIControl(effectNumber * 4 - 2, 1),
                          P32.padColors.purple);

        this.next = new ToggleButtonAsymmetric(
            [0x90 + unitNumber, P32.PadNumToMIDIControl(effectNumber * 4 - 1, 1)],
            this.group,
            'next_effect',
            null,
            false);
        this.nextShifted = new ToggleButtonAsymmetric(
            [0x90 + unitNumber + P32.shiftOffset, P32.PadNumToMIDIControl(effectNumber * 4 - 1, 1)],
            this.group,
            'next_effect',
            null,
            false);
        midi.sendShortMsg(0x90 + unitNumber,
                          P32.PadNumToMIDIControl(effectNumber * 4 - 1, 1),
                          P32.padColors.purple);
        midi.sendShortMsg(0x90 + unitNumber + P32.shiftOffset,
                          P32.PadNumToMIDIControl(effectNumber * 4 - 1, 1),
                          P32.padColors.purple);
    };

    for (var e = 1; e <= 3; e++) {
        this['effect' + e] = new Effect(e);
    }

    this.toggleHeadphones = new ToggleButton(
        [0x90 + unitNumber, 0x34],
        this.group,
        'group_[Headphone]_enable',
        true,
        P32.padColors.red);
    this.toggleHeadphonesShifted = new ToggleButton(
        [0x90 + unitNumber + P32.shiftOffset, 0x34],
        this.group,
        'group_[Headphone]_enable',
        true,
        P32.padColors.red);

    this.toggleMaster = new ToggleButton(
        [0x90 + unitNumber, 0x35],
        this.group,
        'group_[Master]_enable',
        true,
        P32.padColors.red);
    this.toggleMasterShifted = new ToggleButton(
        [0x90 + unitNumber + P32.shiftOffset, 0x35],
        this.group,
        'group_[Master]_enable',
        true,
        P32.padColors.red);

    this.toggleMicrophone = new ToggleButton(
        [0x90 + unitNumber, 0x36],
        this.group,
        'group_[Microphone]_enable',
        true,
        P32.padColors.red);
    this.toggleMicrophoneShifted = new ToggleButton(
        [0x90 + unitNumber + P32.shiftOffset, 0x36],
        this.group,
        'group_[Microphone]_enable',
        true,
        P32.padColors.red);

    this.toggleAuxiliary = new ToggleButton(
        [0x90 + unitNumber, 0x37],
        this.group,
        'group_[Auxiliary1]_enable',
        true,
        P32.padColors.red);
    this.toggleAuxiliaryShifted = new ToggleButton(
        [0x90 + unitNumber + P32.shiftOffset, 0x37],
        this.group,
        'group_[Auxiliary1]_enable',
        true,
        P32.padColors.red);
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
    };

    this.sync = new ToggleButton([0x90 + channel, 0x08], this.currentDeck, 'sync_enabled');
    this.cue = new CueButton([0x90 + channel, 0x09], this.currentDeck);
    this.play = new PlayButton([0x90 + channel, 0x0A], this.currentDeck);

    this.quantize = new ToggleButton(
        [0x90 + channel + P32.shiftOffset, 0x08], this.currentDeck, 'quantize'); // sync shifted
    this.alignBeats = new ActionButton(
        [0x90 + channel + P32.shiftOffset, 0x09], this.currentDeck, 'beats_translate_curpos'); // cue shifted
    this.goToStart = new Control( // play shifted
        [0x90 + channel + P32.shiftOffset, 0x0A], this.currentDeck, 
        ['start_stop', function () { return 1; }],
        ['play_indicator', function (val) { return val * 127; } ]);

    for (var i = 1; i <= 16; i++) {
        // FIXME for 2.1: hacks around https://bugs.launchpad.net/mixxx/+bug/1565377
        this['hotcueButton' + i] = new HotcueButton(
            [0x90 + channel, P32.PadNumToMIDIControl(i, 3)], this.currentDeck,
            i, P32.padColors.red, P32.padColors.off);
        this['hotcueButtonShift' + i] = new HotcueClearButton(
            [0x90 + channel + P32.shiftOffset, P32.PadNumToMIDIControl(i, 3)], this.currentDeck,
            i, P32.padColors.red, P32.padColors.off);
    }

    this.loopIn = new ActionButton(
        [0x90 + channel, 0x50], this.currentDeck,
        'loop_in', P32.padColors.purple);
    this.loopOut = new ActionButton(
        [0x90 + channel, 0x51], this.currentDeck,
        'loop_out', P32.padColors.purple);
    this.loopTogglePad = new LoopToggleButton(
        [0x90 + channel, 0x52], this.currentDeck,
        P32.padColors.red, P32.padColors.blue);

    this.pfl = new ToggleButton([0x90 + channel, 0x10], this.currentDeck, 'pfl');

    for (var k = 1; k <= 3; k++) {
        this['eqKnob' + k] = new CCNonLin(
            [0xB0 + channel, 0x02 + k],
            '[EqualizerRack1_' + this.currentDeck + '_Effect1]',
            'parameter' + k,
            false,
            0, 1, 4);
    }

    this.volume = new CCNonLin(
        [0xB0 + channel, 0x01], this.currentDeck, 'volume', false, 0, 0.25, 1);

    this.loopSize = new Control(
        [0xB0 + channel, 0x1B], this.currentDeck,
        null, ['loop_enabled', null]);
    this.loopSize.input = function (channel, control, value, status, group) {
        if (loopEnabledDot) {
            if (value > 64 && loopSize > 2) { // turn left
                /**
                    Unfortunately, there is no way to show 1 with a dot on the
                    loop size LED.
                **/
                loopSize /= 2;
                engine.setValue(that.currentDeck, 'loop_halve', 1);
                engine.setValue(that.currentDeck, 'loop_halve', 0);
            } else if (value < 64 && loopSize < 32) { // turn right
                /**
                    Mixxx supports loops longer than 32 beats, but there is no way
                    to show 64 with a dot on the loop size LED.
                **/
                loopSize *= 2;
                engine.setValue(that.currentDeck, 'loop_double', 1);
                engine.setValue(that.currentDeck, 'loop_double', 0);
            }
        } else {
            if (value > 64 && loopSize > 1/32) { // turn left
                /**
                    Mixxx supports loops shorter than 1/32 beats, but there is no
                    way to set the loop size LED less than 1/32 (even though it
                    should be able to show 1/64)
                **/
                loopSize /= 2;
                engine.setValue(that.currentDeck, 'loop_halve', 1);
                engine.setValue(that.currentDeck, 'loop_halve', 0);
            } else if (value < 64 && loopSize < 64) { // turn right
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
    };
    this.loopSize.output = function (value, group, control) {
        if (loopEnabledDot && value) {
            this.send(5 - Math.log(loopSize) / Math.log(2));
        } else {
            this.send(5 + Math.log(loopSize) / Math.log(2));
        }
    };
    this.loopSize.connect();
    this.loopSize.trigger();

    this.loopMoveEncoder = function (channel, control, value, status, group) {
        var direction = (value > 64) ? -1 : 1;
        if (loopSize < 1) {
            engine.setValue(that.currentDeck, 'loop_move', loopSize * direction);
        } else {
            engine.setValue(that.currentDeck, 'loop_move', 1 * direction);
        }
    };

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
    };

    this.tempoEncoder = function (channel, control, value, status, group) {
        var direction = (value > 64) ? -1 : 1;
        engine.setValue(that.currentDeck, 'rate', engine.getValue(that.currentDeck, 'rate') + (0.01 * direction));
    };

    this.tempoPress = function (channel, control, value, status, group) {
        if (value) {
            engine.setValue(that.currentDeck, 'rate', 0);
        }
    };

    this.beatJumpEncoder = function (channel, control, value, status, group) {
        var direction = (value > 64) ? -1 : 1;
        if (that.beatJumpEncoderPressed) {
            if (value > 64 && beatJumpSize > 1/32) { // turn left
                beatJumpSize /= 2;
            } else if (value < 64 && beatJumpSize < 64) { // turn right
                beatJumpSize *= 2;
            }
            // The firmware will only change the numeric LED readout when sent messages
            // on the unshifted channel.
            midi.sendShortMsg(0xB0 + channel - P32.shiftOffset, 0x1B, 5 + Math.log(beatJumpSize) / Math.log(2));
        } else {
            engine.setValue(that.currentDeck, 'beatjump', direction * beatJumpSize);
        }
    };

    this.beatJumpPress = function (channel, control, value, status, group) {
        // The firmware will only change the numeric LED readout when sent messages
        // on the unshifted channel.
        if (value === 127) {
            that.beatJumpEncoderPressed = true;
            midi.sendShortMsg(0xB0 + channel - P32.shiftOffset, 0x1B, 5 + Math.log(beatJumpSize) / Math.log(2));
        } else {
            that.beatJumpEncoderPressed = false;
            midi.sendShortMsg(0xB0 + channel - P32.shiftOffset, 0x1B, 5 + Math.log(loopSize) / Math.log(2));
        }
    };

    this.loadTrack = function (channel, control, value, status, group) {
        if (value === 127) {
            engine.setValue(that.currentDeck, 'LoadSelectedTrack', 1);
        }
    };

    this.ejectTrack = function (channel, control, value, status, group) {
        if (value === 127) {
            engine.setValue(that.currentDeck, 'eject', 1);
            engine.beginTimer(250, 'engine.setValue("'+that.currentDeck+'", "eject", 0)', true);
        }
    };
};
P32.Deck.prototype = Object.create(Deck.prototype);
P32.Deck.prototype.constructor = P32.Deck;
