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
'use strict';

/**
A Control represents a physical component on a controller, such as a button, knob, encoder, or fader.
It encapsulates all the information needed to receive input from that component and all the
information needed to send MIDI signals out to the controller to manipulate LEDs. It also provides
generic functions that can be made to work for most use cases just by changing some attributes
of the Control, without having to write much or any custom code.

In most cases, specifying the inCo and outCo properties will be sufficient. The inCo property is
the name of the Mixxx Control Object (see http://mixxx.org/wiki/doku.php/mixxxcontrols for a list
of them) that this JavaScript Control manipulates when it receives a MIDI input signal. The outCo
property is the Mixxx CO that this JavaScript Control sends MIDI signals back out to the controller
when the CO changes. The output callback is automatically connected by the constructor function if
the outCo and group properties are specified (unless the outConnect property is set to false to
intentionally avoid that).

The input function needs to be mapped to the incoming MIDI signals in the XML file. For example:
<control>
    <group>[Channel1]</group>
    <key>MyController.playButton.input</key>
    <status>0x90</status>
    <midino>0x02</midino>
    <options>
        <script-binding/>
    </options>
</control>

The constructor should be passed an object to that is merged with the Control to specify custom
attributes. Most Controls will need at least their MIDI signals and group specified. The first
two bytes of the MIDI signals sent and received from that physical control are specified in a two
member array. For example:

var quantize = new Control({
    midi: [0x91, 0x01],
    group: '[Channel1]'
    inCo: 'quantize',
    outCo: 'quantize'
});


A handful of subclasses are available that are more convenient for common use cases. For the above
example, it would actually be better to make a new Button than directly make a new Control.
The subclasses will cover most use cases. Only if you need to make a lot of changes to the default
Control attributes should you use the Control constructor directly.

Controls can be used to manage alternate behaviors in different conditions. The most common use case
for this is for shift buttons. For that case, assign functions to the shift and unshift properties
that manipulate the Control appropriately. To avoid redundancy (like typing the name of the inCo
both as the inCo property and in the unshift function), the Control constructor will automatically
call the unshift function if it exists. The shift and unshift functions of ControlContainer
will call the appropriate function of all the Controls within it that have that function defined.

To avoid typing out the group every time, Controls that share a group can be part of a
ControlContainer and the ControlContainer's reconnectControls method can assign the group to all
of them.
**/
var Control = function (options) {
    if (Array.isArray(options) && typeof options[0] === 'number') {
        this.midi = options;
    } else {
        _.assign(this, options);
    }

    if (typeof this.unshift === 'function') {
        this.unshift();
    }
    // This cannot be in the prototype; it must be unique to each instance.
    this.connections = [];

    if (this.outConnect && this.group !== undefined && this.outCo !== undefined) {
        this.connect();
        if (this.outTrigger) {
            this.trigger();
        }
    }
};
Control.prototype = {
    // default attributes
    // You should probably overwrite at least some of these.
    inFunc: function (value) {return value;},
    // map input in the XML file, not inFunc
    input: function (channel, control, value, status, group) {
               this.setValue(this.inFunc.call(this, value));
            },
    outFunc: function (value) {return value;},
    output: function (value, group, control) {
                this.send(this.outFunc.call(this, value));
            },
    outConnect: true,
    outTrigger: true,

    // common functions
    // In most cases, you should not overwrite these.
    setValue: function (value) {
        engine.setValue(this.group, this.inCo, value);
    },
    // outCo value generally shouldn't be set directly,
    // only by the output() callback when its value changes,
    // so don't provide separate setValueIn/setValueOut functions.
    getValueIn: function () {
        return engine.getValue(this.group, this.inCo);
    },
    getValueOut: function () {
        return engine.getValue(this.group, this.outCo);
    },
    toggle: function () {
        this.setValue( ! this.getValueIn());
    },
    connect: function () {
        /**
        Override this method with a custom one to connect multiple Mixxx COs for a single Control.
        Add the connection objects to the this.connections array so they all get disconnected just
        by calling this.disconnect(). This can be helpful for multicolor LEDs that show a
        different color depending on the state of different Mixxx COs. See SamplerButton.connect()
        and SamplerButton.output() for an example.
        **/
        this.connections[0] = engine.connectControl(this.group, this.outCo, this.output);
    },
    disconnect: function () {
        if (this.connections[0] !== undefined) {
            this.connections.forEach(function (connection) {
                connection.disconnect();
            });
        }
    },
    trigger: function() { engine.trigger(this.group, this.outCo); },
    send: function (value) {
        midi.sendShortMsg(this.midi[0], this.midi[1], value);
        if (this.sendShifted) {
            midi.sendShortMsg(this.midi[0] + this.shiftOffset, this.midi[1], value);
        }
    },
    sendShifted: false,
    shiftOffset: 0,
};

/**
A Control for buttons/pads. If the inCo and outCo are the same, you can specify just a "co" property
for the constructor. If the inCo and outCo are different, specify each of them.

For example:
var quantize = new Control({
    midi: [0x91, 0x01],
    group: '[Channel1]'
    co: 'quantize'
});

By default, the inCo is toggled only when the button is pressed. For buttons that activate an inCo
only while they are held down, set the onlyOnPress property to false.

By default, this works for controllers that send MIDI messages with a different 3rd byte of the
MIDI message (value) to indicate the button being pressed/released, with the first two bytes
(status and control) remaining the same for both press and release. If your controller sends
separate MIDI note on/off messages with on indicated by the first nybble (hexadecimal digit) of
the first (status) byte being 9 and note off with the first nybble being 8, in your script's init
function, set Button.prototype.separateNoteOnOff to true and map both the note on and off messages
to the Button object's input property.
**/
var Button = function (options) {
    if (options !== undefined && typeof options.co === 'string') {
        this.inCo = options.co;
        this.outCo = options.co;
    }
    Control.call(this, options);
};
Button.prototype = new Control({
    onlyOnPress: true,
    on: 127,
    off: 0,
    inFunc: function () { return ! this.getValueIn(); },
    separateNoteOnOff: false,
    input: function (channel, control, value, status, group) {
               if (this.onlyOnPress) {
                   var pressed = value > 0;
                   if (this.separateNoteOnOff) {
                       // Does the first nybble of the first MIDI byte indicate a
                       // note on or note off message?
                       pressed = (status & 0xF0) === 0x90;
                   }
                   if (pressed) {
                       this.setValue(this.inFunc.call(this, value));
                   }
                } else {
                       this.setValue(this.inFunc.call(this, value));
                }
    },
    outFunc: function() { return (this.getValueOut()) ? this.on : this.off; }
});

var PlayButton = function (options) {
    Button.call(this, options);
};
PlayButton.prototype = new Button({
    unshift: function () { this.inCo = 'play'; },
    shift: function () { this.inCo = 'start_stop'; },
    outCo: 'play_indicator'
});

var CueButton = function (options) {
    Button.call(this, options);
};
CueButton.prototype = new Button({
    inCo: 'cue_default',
    outCo: 'cue_indicator',
    onlyOnPress: false
});

var SyncButton = function (options) {
    Button.call(this, options);
};
SyncButton.prototype = new Button({
    unshift: function () { this.inCo = 'sync_enabled'; },
    shift: function () { this.inCo = 'beatsync'; },
    outCo: 'sync_enabled'
});

var LoopButton = function (options) {
    Button.call(this, options);
};
LoopButton.prototype = new Button({
    inCo: 'reloop_exit',
    inFunc: function() { return 1; },
    outCo: 'loop_enabled',
    outFunc: function(value) { return (value) ? this.on : this.off; }
});

var HotcueButton = function (options) {
    if (options.number === undefined) {
        print('WARNING: No hotcue number specified for new HotcueButton.');
    }
    this.number = options.number;
    this.outCo = 'hotcue_' + this.number + '_enabled';
    Button.call(this, options);
};
HotcueButton.prototype = new Button({
    unshift: function () { this.inCo = 'hotcue_' + this.number + '_activate'; },
    shift: function () { this.inCo = 'hotcue_' + this.number + '_clear'; },
    onlyOnPress: false
});

/**
A Control for sampler buttons. Press the button to load the track selected in
the library into an empty sampler. Press a loaded sampler to play it from
its cue point. Press again while playing to jump back to the cue point.

@param {Integer} number: number of the sampler
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
var SamplerButton = function (options) {
    if (options.number === undefined) {
        print('WARNING: No sampler number specified for new SamplerButton.');
    }
    this.number = options.number;
    this.group = '[Sampler' + this.number + ']';
    Button.call(this, options);
};
SamplerButton.prototype = new Button({
    inputUnshifted: function (channel, control, value, status, group) {
        if (value > 0) {
            // track_samples is 0 when the sampler is empty and > 0 when a sample is loaded
            if (engine.getValue(this.group, 'track_samples') === 0) {
                engine.setValue(this.group, 'LoadSelectedTrack', 1);
            } else {
                engine.setValue(this.group, 'cue_gotoandplay', 1);
            }
        }
    },
    unshift: function () { this.input = this.inputUnshifted; },
    inputShifted: function (channel, control, value, status, group) {
        if (value > 0) {
            if (engine.getValue(this.group, 'play') === 1) {
                engine.setValue(this.group, 'play', 0);
            } else {
                engine.setValue(this.group, 'eject', 1);
            }
        }
    },
    shift: function() { this.input = this.inputShifted; },
    output: function (value, group, control) {
        if (engine.getValue(this.group, 'track_samples') > 0) {
            if (this.playing === undefined) {
                this.send(this.on);
            } else {
                if (engine.getValue(this.group, 'play') === 1) {
                    this.send(this.on);
                } else {
                    this.send(this.playing);
                }
            }
        } else {
            this.send(this.off);
        }
    },
    connect: function() {
        this.connections[0] = engine.connectControl(this.group, 'track_samples', this.output);
        if (this.playing !== undefined) {
            this.connections[1] = engine.connectControl(this.group, 'play', this.output);
        }
    },
    outCo: null, // hack to get Control constructor to call connect()
});

/**
A Control for faders and knobs with finite ranges. Although these do not respond to a change in a
Mixxx CO to send MIDI signals back to the controller, using a CC is helpful because Control.connect
and Control.disconnect are overwritten to take care of soft takeover when switching layers with
ControlContainer.reconnectControls() and ControlContainer.applyLayer().


softTakeoverInit, boolean, optional: Whether to activate soft takeover upon initialization. Defaults
to true if ommitted. Some controllers (like the Hercules P32) can be sent a message to tell it to
send signals back with the positions of all the controls. It is helpful to send the message in the
script's init function, but it requires that soft takeover isn't enabled to work. So, for these
controllers, set CC.prototype.softTakeoverInit as false in your script's init function.

max, number, optional: the maximum value received from the controller. Defaults to 127 if ommitted.
**/
var CC = function (options) {
    Control.call(this, options);

    if (this.softTakeoverInit) {
        this.connect();
    }
};
CC.prototype = new Control({
    input: function (channel, control, value, status, group) {
        // FIXME: temporary hack around https://bugs.launchpad.net/mixxx/+bug/1479008
        // just use engine.setParameter() when soft takeover works for it
        if (this.range === undefined) {
            engine.setParameter(this.group, this.inCo, value / this.max);
        } else if (Array.isArray(this.range)) {
            switch (this.range.length) {
                case 2:
                    engine.setValue(this.group, this.inCo,
                                    script.absoluteLin(value, this.range[0], this.range[1]));
                    break;
                case 3:
                    engine.setValue(this.group, this.inCo,
                                    script.absoluteNonLin(value, this.range[0], this.range[1], this.range[2]));
                    break;
                case 4:
                    engine.setValue(this.group, this.inCo,
                                    script.absoluteLin(value, this.range[0], this.range[1], this.range[2], this.range[3]));
                    break;
                case 5:
                    engine.setValue(this.group, this.inCo,
                                    script.absoluteNonLin(value, this.range[0], this.range[1], this.range[2], this.range[3], this.range[4]));
                    break;
            }
        }
    },
    connect: function () {
        engine.softTakeover(this.group, this.inCo, true);
    },
    disconnect: function () {
        engine.softTakeoverIgnoreNextValue(this.group, this.inCo);
    },
    trigger: function () {},
    max: 127,
    softTakeoverInit: true
});

/**
A ControlContainer is an object that contains Controls as properties, with methods to help
with batch manipulation of those Controls.

initialLayer, object, optional: the layer to activate upon initialization
**/
var ControlContainer = function (initialLayer) {
    if (typeof initialLayer === 'object') {
        this.applyLayer(initialLayer);
    }
};
ControlContainer.prototype = {
    forEachControl: function (operation, recursive) {
        /**
        operation, function that takes 1 argument: the function to call for each Control.
                                                   Takes each Control as its first argument.
        recursive, boolean, optional: whether to call forEachControl recursively
                                      for each ControlContainer within this ControlContainer.
                                      Defaults to true if ommitted.
        **/
        if (typeof operation !== 'function') {
            print('ERROR: ControlContainer.forEachContainer requires a function argument');
            return;
        }
        if (recursive === undefined) { recursive = true; }

        var that = this;
        var applyOperationTo = function (obj) {
            if (obj instanceof Control) {
                operation.call(that, obj);
            } else if (recursive && obj instanceof ControlContainer) {
                obj.forEachControl(operation);
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
    },
    reconnectControls: function (operation, recursive) {
        /**
        operation, function that takes one argument, optional: a function to call for each Control in this ControlContainer
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
        }, recursive);
    },
    applyLayer: function (newLayer, reconnectControls) {
    /**
    Layers are merely objects that contain Controls to overwrite the active Controls of a
    ControlContainer. Layers are deeply merged with the applyLayer() method, so if a new layer does
    not define a property for a Control, the Control's old property will be retained.
    **/
        if (reconnectControls !== false) {
            reconnectControls = true;
        }
        if (reconnectControls === true) {
            this.forEachControl(function (control) {
                control.disconnect();
            });
        }

        _.merge(this, newLayer);

        if (reconnectControls === true) {
            this.forEachControl(function (control) {
                if (typeof operation === 'function') {
                    operation.call(this, control);
                }
                control.connect();
                control.trigger();
            });
        }
    },
    shift: function () {
        this.forEachControl(function (control) {
            if (typeof control.shift === 'function') {
                control.shift();
            }
        });
        this.isShifted = true;
    },
    unshift: function () {
        this.forEachControl(function (control) {
            if (typeof control.unshift === 'function') {
                control.unshift();
            }
        });
        this.isShifted = false;
    },
    isShifted: false
};

script.samplerRegEx = /\[Sampler(\d+)\]/ ;
script.channelRegEx = /\[Channel(\d+)\]/ ;
script.eqKnobRegEx = /\[EqualizerRack1_\[(.*)\]_Effect1\]/ ;
script.quickEffectRegEx = /\[QuickEffectRack1_\[(.*)\]\]/ ;

/**
A ControlContainer with a toggle() method for conveniently changing the group attributes of
contained Controls to switch the deck that a set of Controls is manipulating.

deckNumbers, array of numbers, size arbitrary: which deck numbers this can cycle through with the toggle() method
                                                Typically [1, 3] or [2, 4]
**/
var Deck = function (deckNumbers) {
    if (deckNumbers !== undefined && Array.isArray(deckNumbers)) {
        this.currentDeck = '[Channel' + deckNumbers[0] + ']';
        this.deckNumbers = deckNumbers;
    }
};
Deck.prototype = new ControlContainer({
    setCurrentDeck: function (newGroup, recursive) {
        this.currentDeck = newGroup;
        if (recursive !== true) { recursive = false; }
        this.reconnectControls(function (control) {
            if (control.group.search(script.eqKnobRegEx) !== -1) {
                control.group = '[EqualizerRack1_' + this.currentDeck + '_Effect1]';
            } else if (control.group.search(script.quickEffectRegEx) !== -1) {
                control.group = '[QuickEffectRack1_' + this.currentDeck + ']';
            } else {
                control.group = this.currentDeck;
            }
        }, recursive);
    },
    toggle: function (recursive) {
        var index = this.deckNumbers.indexOf(parseInt(script.channelRegEx.exec(this.currentDeck)[1]));
        if (index === (this.deckNumbers.length - 1)) {
            index = 0;
        } else {
            index += 1;
        }
        this.setCurrentDeck("[Channel" + this.deckNumbers[index] + "]");
    }
});

var P32 = {};

P32.init = function () {
    Control.prototype.shiftOffset = 3;
    Button.prototype.sendShifted = true;
    CC.prototype.softTakeoverInit = false;

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
    outCo: 'status'
});

P32.slipButton = new Button({
    midi: [0x90, 0x03],
    input: function (channel, control, value, status, group) {
        if (value === 127) {
                if (P32.leftDeck.isShifted) {
                    P32.leftDeck.toggle();
                } else if (P32.rightDeck.isShifted) {
                    P32.rightDeck.toggle();
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
    co: 'slip_enabled',
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
        this.eqKnob[k] = new CC({
            midi: [0xB0 + channel, 0x02 + k],
            group: '[EqualizerRack1_' + this.currentDeck + '_Effect1]',
            inCo: 'parameter' + k,
            range: [0, 1, 4]
        });
    }

    this.pfl = new Button({
        midi: [0x90 + channel, 0x10],
        co: 'pfl',
    });

    this.volume = new CC({
        midi: [0xB0 + channel, 0x01],
        inCo: 'volume',
        range: [0, 0.25, 1]
    });

    // ==================================== PAD GRID ============================================
    // Slicer layer set up in P32.EffectUnit()

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
    this.loopTogglePad = new LoopButton({
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

    this.effectUnit = new P32.EffectUnit(deckNumbers[0]);
};
P32.Deck.prototype = new Deck();

P32.EffectUnit = function (unitNumber) {
    this.group = '[EffectRack1_EffectUnit' + unitNumber + ']';

    this.dryWetKnob = new CC({
        midi: [0xB0 + unitNumber, 0x09],
        unshift: function () {
            this.inCo = 'mix';
            // for soft takeover
            this.disconnect();
            this.connect();
        },
        shift: function () {
            this.inCo = 'super1';
            // for soft takeover
            this.disconnect();
            this.connect();
        },
        range: [0, 1]
    });

    // ON/MACRO buttons
    this.deckEnableButton = [];
    for (var d = 1; d <= 4; d++) {
        this.deckEnableButton[d] = new Button({
            midi: [0x90 + unitNumber,
                   0x02 + d],
            co: 'group_[Channel' + d + ']_enable',
        });
    }

    this.toggleHeadphones = new Button({
        midi: [0x90 + unitNumber, 0x34],
        co: 'group_[Headphone]_enable',
        on: P32.padColors.red,
        off: P32.padColors.blue
    });
    this.toggleMaster = new Button({
        midi: [0x90 + unitNumber, 0x35],
        co: 'group_[Master]_enable',
        on: P32.padColors.red,
        off: P32.padColors.blue
    });
    this.toggleMicrophone = new Button({
        midi: [0x90 + unitNumber, 0x36],
        co: 'group_[Microphone]_enable',
        on: P32.padColors.red,
        off: P32.padColors.blue
    });
    this.toggleAuxiliary = new Button({
        midi: [0x90 + unitNumber, 0x37],
        co: 'group_[Auxiliary1]_enable',
        on: P32.padColors.red,
        off: P32.padColors.blue
    });

    this.reconnectControls(function (control) {
        if (control.group === undefined) {
            control.group = this.group;
            control.on = P32.padColors.red;
        }
    });

    var EffectRow = function (effectNumber) {
        var ef = this;
        this.group = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + effectNumber + ']';
        this.toggle = new Button({
            midi: [0x90 + unitNumber,
                   P32.PadNumToMIDIControl(effectNumber * 4 - 3, 1)],
            co: 'enabled',
            unshift: function () { this.input = Button.prototype.input; },
            inputShifted: function (channel, control, value, status, group) {
                if (value === 127) {
                    var p = engine.getValue(ef.group, 'num_parameters');
                    for (var i = 1; i <= p; i++) {
                        engine.setValue(ef.group, 'parameter' + i + '_set_default', 1);
                    }
                    var b = engine.getValue(ef.group, 'num_button_parameters');
                    for (var n = 1; n <= b; n++) {
                        engine.setValue(ef.group, 'button_parameter' + n, 0);
                    }
                }
            },
            shift: function () { this.input = this.inputShifted; },
            on: P32.padColors.red,
        });

        this.previous = new Button({
            midi: [0x90 + unitNumber,
                   P32.PadNumToMIDIControl(effectNumber * 4 - 2, 1)],
            inCo: 'prev_effect',
            onlyOnPress: false
        });
        this.next = new Button({
            midi: [0x90 + unitNumber,
                   P32.PadNumToMIDIControl(effectNumber * 4 - 1, 1)],
            inCo: 'next_effect',
            onlyOnPress: false
        });

        this.reconnectControls(function (control) {
            if (control.group === undefined) {
                control.group = this.group;
            }
        });

        this.previous.send(P32.padColors.purple);
        this.next.send(P32.padColors.purple);
    };
    EffectRow.prototype = new ControlContainer();

    this.effect = [];
    for (var e = 1; e <= 3; e++) {
        this.effect[e] = new EffectRow(e);
    }

    this.activeEffect = new ControlContainer();
    var ae = this.activeEffect;
    ae.number = 1;
    ae.parameterKnob = [];
    ae.switchEffectButton = []; 
    for (var p = 1; p <= 3; p++) {
        ae.parameterKnob[p] = new CC({
            midi: [0xB0 + unitNumber, 0x05 + p],
            inCo: 'parameter' + p
        });
        ae.switchEffectButton[p] = new Button({
            midi: [0x90 + unitNumber,
                   P32.PadNumToMIDIControl(p * 4, 1)],
            on: P32.padColors.blue,
            number: p,
            input: function (channel, control, value, status, group) {
                if (value > 0) {
                    ae.number = this.number;
                    ae.reconnectControls(function (control) {
                        control.group = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + ae.number + ']';
                    });
                }
            },
            trigger: function (value, group, control) {
                // called by ae.reconnectControls() in this.input()
                this.send((this.number === ae.number) ? this.on : this.off);
            },
            disconnect: function () {},
            connect: function () {},
            sendShifted: true
        });
    }
    ae.switchEffectButton[1].input(null, null, 127, null, null);
};
P32.EffectUnit.prototype = new ControlContainer();
