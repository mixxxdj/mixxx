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
A Control is a JavaScript object that represents a physical component on a controller, such as a
button, knob, encoder, or fader. It encapsulates all the information needed to receive MIDI input
from that component and send MIDI signals out to the controller to activate its LED(s). It provides
generic functions that can be made to work for most use cases just by changing some attributes
of the Control, without having to write much or any custom code.

Controls should generally be properties of a ControlContainer object, which provides functions for
conveniently doing batch operations on a collection of related Controls. Most Controls should be
properties of a custom Deck object, which is a derivative of ControlContainer. Refer to the Deck
documentation for more details and an example.

The input function needs to be mapped to the incoming MIDI signals in the XML file. For example:
<control>
    <group>[Channel1]</group>
    <!-- MyController.leftDeck would be an instance of a custom Deck. -->
    <key>MyController.leftDeck.quantizeButton.input</key>
    <status>0x90</status>
    <midino>0x01</midino>
    <options>
        <script-binding/>
    </options>
</control>
The output function does not need to be mapped in XML.

A handful of derivative Control objects are available that are more convenient for common use cases.
These derivative objects will cover most use cases. In practice, most Controls are derivatives
of the Button or CC Controls. Only if you need to make a lot of changes to the default Control
attributes should you use the Control constructor directly.

Create Controls by calling the constructor with JavaScript's "new" keyword. The Control constructor
takes a single argument. This is an options object containing properties that get merged with the
Control when it is created, making it easy to customize the functionality of the Control. Most
Controls will need at least their midi, group, inCo, and outCo attributes specified.

The midi attribute is a two member array corresponding to the first two MIDI bytes that the
controller sends/receives when the physical component changes state. The group property specifies
the group that both the inCo and outCo manipulate, for example '[Channel1]' for deck 1. The inCo
property is the name of the Mixxx Control Object (see http://mixxx.org/wiki/doku.php/mixxxcontrols
for a list of them) that this JavaScript Control manipulates when it receives a MIDI input signal.
When the Mixxx CO specified by outCo changes, this JavaScript Control sends MIDI signals back out to
the controller. For example:

var quantizeButton = new Button({
    midi: [0x91, 0x01],
    group: '[Channel1]'
    inCo: 'quantize',
    outCo: 'quantize'
});

The output callback is automatically connected by the constructor function if the outCo and group
properties are specified to the constructor (unless the outConnect property is set to false to
intentionally avoid that). This makes it easy to map the controller so its LEDs stay synchronized
with the status of Mixxx, whether the outCo changes because of the Control receiving MIDI input or
the user changing it with the keyboard, mouse, or another controller. The output callback can be
easily connected and disconnected by calling the Control's connect and disconnect functions. The
output callback can also be manually run with the appropriate arguments simply by calling the
Control's trigger function. The connect, disconnect, and trigger functions are automatically called
by ControlContainer's reconnectControls and applyLayer functions to make activating different layers
of functionality easy.

Controls can be used to manage alternate behaviors in different conditions. The most common use case
for this is for shift buttons. For that case, assign functions to the shift and unshift properties
that manipulate the Control appropriately. In some cases, using the shift/unshift functions to
change the Control's inCo, outCo, or group properties will be sufficient. See HotcueButton for an
example. If you want to use custom input or output functions when shifting and unshifting, define
each alternate function as a different property of the Control and use the shift/unshift functions
to assign the Control's input/output properties to the appropriate function. See SamplerButton for
an example. To avoid redundancy (like typing the name of the inCo both as the inCo property and in
the unshift function), the Control constructor will automatically call the unshift function if it
exists. The shift and unshift functions of ControlContainer will call the appropriate function of
all the Controls within it that have that function defined.

Control and its derivative objects use constructor functions with a minimal amount of logic. Most of
the functionality of Controls comes from their prototype objects. In JavaScript, making a change to
an object's prototype immediately changes all existing and future objects that have it in their
prototype change (regardless of the context in which the derivative objects were created). This
makes it easy to change the behavior for all (of a subtype) of Control to accomodate the MIDI
signals used by a particular controller. For example, the Hercules P32 controller sends and receives
two sets of MIDI signals for most physical components, one for when the shift button is pressed and
one for when the shift button is not pressed. The controller changes the state of its LEDs when the
shift buttons are pressed, which is controlled by the alternate set of MIDI signals. These alternate
MIDI signals are the same as the unshifted ones, but the MIDI channel is 3 higher. So, to avoid
having the LEDs flicker when the shift button is pressed or having to define separate JavaScript
Controls for every physical controller component in its shifted and unshifted state, the P32's
init function has this code:

Control.prototype.shiftOffset = 3;
Control.prototype.shiftChannel = true;
Button.prototype.sendShifted = true;
This causes the Control.prototype.send function to send both the shifted and unshifted MIDI
signals when the Control's outCo changes.
If your controller uses the same MIDI channel but different control numbers when a shift button
is pressed, set Control.prototype.shiftControl to true instead of Control.prototype.shiftChannel.

To avoid typing out the group every time, Controls that share a group can be part of a
ControlContainer and the ControlContainer's reconnectControls method can assign the group to all
of them. For convenience, if a Control only needs its midi property specified for its constructor,
this can be provided simply as an array without wrapping it in an object. For example,

var playButton = new PlayButton([0x90 + channel, 0x0A]);
instead of
var playButton = new PlayButton({
    midi: [0x90 + channel, 0x0A]
});
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
    // These cannot be in the prototype; they must be unique to each instance.
    this.isShifted = false;
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
    inValueScale: function (value) {return value;},
    // map input in the XML file, not inValueScale
    input: function (channel, control, value, status, group) {
               this.setValue(this.inValueScale(value));
            },
    outValueScale: function (value) {return value;},
    output: function (value, group, control) {
                this.send(this.outValueScale(value));
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
    // or by calling trigger()
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
    shiftOffset: 0,
    sendShifted: false,
    shiftChannel: false,
    shiftControl: false,
    send: function (value) {
        if (this.midi === undefined) {
            return;
        }
        midi.sendShortMsg(this.midi[0], this.midi[1], value);
        if (this.sendShifted) {
            if (this.shiftChannel) {
                midi.sendShortMsg(this.midi[0] + this.shiftOffset, this.midi[1], value);
            } else if (this.shiftControl) {
                midi.sendShortMsg(this.midi[0], this.midi[1] + this.shiftOffset, value);
            }
        }
    },
};

/**
A Button is a Control derivative for buttons/pads. If the inCo and outCo are the same, you can
specify just a "co" property for the constructor. If the inCo and outCo are different, specify
each of them.

For example:
var quantize = new Control({
    midi: [0x91, 0x01],
    group: '[Channel1]'
    co: 'quantize'
});

Derivative Buttons are provided for many common use cases, including:
PlayButton
CueButton
SyncButton
LoopToggleButton
HotcueButton
SamplerButton
These make it easy to map those kinds of buttons without having to worry about particularities
of Mixxx's ControlObjects that can make mapping them not so straightforward. The PlayButton,
SyncButton, HotcueButton, and SamplerButton objects also provide alternate functionality for when a
shift button is pressed.

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
    inValueScale: function () { return ! this.getValueIn(); },
    separateNoteOnOff: false,
    input: function (channel, control, value, status, group) {
               if (this.onlyOnPress) {
                   var pressed;
                   if (this.separateNoteOnOff) {
                       // Does the first nybble of the first MIDI byte indicate a
                       // note on or note off message?
                       pressed = (status & 0xF0) === 0x90;
                   } else {
                       pressed = value > 0;
                   }
                   if (pressed) {
                       this.setValue(this.inValueScale(value));
                   }
                } else {
                       this.setValue(this.inValueScale(value));
                }
    },
    outValueScale: function() { return (this.getValueOut()) ? this.on : this.off; }
});

/**
Default behavior: play/pause
Shift behavior: go to start of track and stop

LED behavior depends on cue mode selected by the user in the preferences
Refer to http://mixxx.org/manual/latest/chapters/user_interface.html#interface-cue-modes
**/
var PlayButton = function (options) {
    Button.call(this, options);
};
PlayButton.prototype = new Button({
    unshift: function () { this.inCo = 'play'; },
    shift: function () { this.inCo = 'start_stop'; },
    outCo: 'play_indicator'
});

/**
Behavior depends on cue mode configured by the user in the preferences
Refer to http://mixxx.org/manual/latest/chapters/user_interface.html#interface-cue-modes
**/
var CueButton = function (options) {
    Button.call(this, options);
};
CueButton.prototype = new Button({
    inCo: 'cue_default',
    outCo: 'cue_indicator',
    onlyOnPress: false
});

/**
Default behavior: toggle sync lock (master sync)
Shift behavior: momentary sync without toggling sync lock
**/
var SyncButton = function (options) {
    Button.call(this, options);
};
SyncButton.prototype = new Button({
    unshift: function () { this.inCo = 'sync_enabled'; },
    shift: function () { this.inCo = 'beatsync'; },
    outCo: 'sync_enabled'
});

// Toggle a loop on/off
var LoopToggleButton = function (options) {
    Button.call(this, options);
};
LoopToggleButton.prototype = new Button({
    inCo: 'reloop_exit',
    inValueScale: function() { return 1; },
    outCo: 'loop_enabled',
    outValueScale: function(value) { return (value) ? this.on : this.off; }
});

/**
Default behavior: set hotcue if it is not set. If it is set, jump to it.
Shift behavior: delete hotcue

LED indicates whether the hotcue is set.
**/
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
Default behavior:
Press the button to load the track selected in the library into an empty sampler. Press a loaded
sampler to play it from its cue point. Press again while playing to jump back to the cue point.
Shift behavior:
If the sampler is playing, stop it. If the sampler is stopped, eject it.

Specify the sampler number as the number property of the object passed to the constructor. There
is no need to manually specify the group. For example:
var samplerButton = [];
var samplerButton[1] = new SamplerButton(
    midi: [0x91, 0x02],
    number: 1
)};

When the sampler is loaded, the LED will be set to the value of the "on" property. When the sampler
is empty, the LED will be set to the value of the "off" property. These are inherited from
Button.prototype if they are not manually specified. If your controller's pads have multicolor LEDs,
specify the value to send for a different LED color with the playing property to set the LED to a
different color while the sampler is playing. For example:

MyController.padColors = {
// These values are just examples, consult the MIDI documentation from your controller's
manufacturer to find the values for your controller. If that information is not available,
guess and check to find the values.
    red: 125,
    blue: 126,
    purple: 127,
    off: 0
};
var samplerButton = [];
var samplerButton[1] = new SamplerButton(
    midi: [0x91, 0x02],
    number: 1,
    on: MyController.padColors.blue,
    playing: MyController.padColors.red,
    // off is inherited from Button.prototype
)};
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
A CC is a Control for potentiometers (faders and knobs) with finite ranges. The name CC comes
from MIDI Control Change signals. Using a CC is helpful because CC.connect() and CC.disconnect()
take care of soft takeover when switching layers with ControlContainer.reconnectControls() and
ControlContainer.applyLayer().

If you send your controller a MIDI message in your init function to make the controller send back
signals indicating the state of all its potentiometers, set CC.prototype.softTakeoverInit to
false before sending that MIDI signal. Otherwise, soft takeover will be activated before
the first signals are received and the initial state of Mixxx won't reflect the controller.
**/
var CC = function (options) {
    Control.call(this, options);

    if (this.softTakeoverInit) {
        this.connect();
    }
};
CC.prototype = new Control({
    inValueScale: function (value) { return value / this.max; },
    input: function (channel, control, value, status, group) {
        engine.setParameter(this.group, this.inCo, this.inValueScale(value));
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
with batch manipulation of those Controls. Documentation for each method is inline below.
**/
var ControlContainer = function (initialLayer) {
    if (typeof initialLayer === 'object') {
        this.applyLayer(initialLayer);
    }
};
ControlContainer.prototype = {
    forEachControl: function (operation, recursive) {
    /**
    operation, function that takes 1 argument:
    the function to call for each Control. Takes each Control as its first argument.
    "this" in the context of the function refers to the ControlContainer.
    recursive, boolean, optional:
    whether to call forEachControl recursively for each ControlContainer within this
    ControlContainer. Defaults to true if ommitted.
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
    Disconnect and reconnect output callbacks for each Control. Optionally perform an operation
    on each Control between disconnecting and reconnecting the output callbacks. Arguments are
    the same as forEachControl().
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
    // Call each Control's shift() function if it exists.
    shift: function () {
        this.forEachControl(function (control) {
            if (typeof control.shift === 'function') {
                control.shift();
            }
        });
        this.isShifted = true;
    },
    // Call each Control's unshift() function if it exists.
    unshift: function () {
        this.forEachControl(function (control) {
            if (typeof control.unshift === 'function') {
                control.unshift();
            }
        });
        this.isShifted = false;
    },
    isShifted: false,
    applyLayer: function (newLayer, reconnectControls) {
    /**
    Activate a new layer of functionality. Layers are merely objects with properties to overwrite
    the of the Controls within this ControlContainer. Layers objects are deeply merged. If a new
    layer does not define a property for a Control, the Control's old property will be retained.

    In the most common case, for providing alternate functionality when a shift button is pressed,
    using applyLayer() is likely overcomplicated and may be slow. Use shift()/unshift() instead.
    Use applyLayer() if you need to cycle through more than two alternate layers.

    For example:
    someControlContainer.applyLayer({
        someButton: { inCo: 'alternate inCo' },
        anotherButton: { outCo: 'alternate outCo' }
    });

    By default, the old layer's output callbacks are disconnected and the new layer's output
    callbacks are connected. To avoid this behavior, which would be desirable if you are not
    changing any output functionality, pass false as the second argument to applyLayer().
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
};

script.samplerRegEx = /\[Sampler(\d+)\]/ ;
script.channelRegEx = /\[Channel(\d+)\]/ ;
script.eqKnobRegEx = /\[EqualizerRack1_\[(.*)\]_Effect1\]/ ;
script.quickEffectRegEx = /\[QuickEffectRack1_\[(.*)\]\]/ ;

/**
A Deck is a ControlContainer with methods for conveniently changing the group attributes of
contained Controls to switch the deck that a set of Controls is manipulating. The setCurrentDeck()
method takes the new deck as a string and sets the Controls' group property appropriately, including
for equalizer knobs and QuickEffect (filter) knobs.

The Deck constructor takes one argument, an array of deck numbers to cycle through with the toggle()
method. Typically this will be [1, 3] or [2, 4].

To map your own controller, create a custom derivative of Deck and create instances of your custom
Deck objects in your controller's init() function. Use a constructor function to create all the
Controls you need for your particular controller and assign your custom derivative's prototype
to Deck. For example:

MyController.init = function () {
    this.leftDeck = new MyController.Deck([1, 2]);
    this.rightDeck = new MyController.Deck([2, 4]);
};
MyController.Deck = function (deckNumbers, channel) {
    Deck.call(this, deckNumbers); // call the Deck constructor to setup the currentDeck and
                                  // deckNumber properties
    var channel = deckNumbers[0]; // many controllers use the same MIDI messages for both sides,
                                  // but with a different MIDI channel number
    this.hotcueButton = [];
    for (var i = 1; i <= 8; i++) {
        this.hotcueButton[i] = new HotcueButton({
            midi: [0x90 + channel, 0x10 + i],
            number: i
        });
    }
};
MyController.Deck.prototype = new Deck();
**/
var Deck = function (deckNumbers) {
    if (deckNumbers !== undefined && Array.isArray(deckNumbers)) {
        // These must be unique to each instance, so they cannot be in the prototype.
        this.currentDeck = '[Channel' + deckNumbers[0] + ']';
        this.deckNumbers = deckNumbers;
    }
};
Deck.prototype = new ControlContainer({
    setCurrentDeck: function (newGroup) {
        this.currentDeck = newGroup;
        this.reconnectControls(function (control) {
            if (control.group.search(script.channelRegEx) !== -1) {
                control.group = this.currentDeck;
            } else if (control.group.search(script.eqKnobRegEx) !== -1) {
                control.group = '[EqualizerRack1_' + this.currentDeck + '_Effect1]';
            } else if (control.group.search(script.quickEffectRegEx) !== -1) {
                control.group = '[QuickEffectRack1_' + this.currentDeck + ']';
            }
            // Do not alter the Control's group if it does not match any of those RegExs because
            // that could break effects Controls.
        });
    },
    toggle: function () {
        var index = this.deckNumbers.indexOf(parseInt(
            script.channelRegEx.exec(this.currentDeck)[1]
        ));
        if (index === (this.deckNumbers.length - 1)) {
            index = 0;
        } else {
            index += 1;
        }
        this.setCurrentDeck("[Channel" + this.deckNumbers[index] + "]");
    }
});

/**
This EffectUnit ControlContainer provides Controls designed to be mapped to the common arrangement
of 3 knobs with 3 buttons for controlling effects, plus a knob or encoder for dry/wet and an
additional button to toggle the effect unit between collapsed and expanded modes. Turning the
dry/wet knob with shift controls the superknob for the whole effect unit. The Controls provided are:

dryWetKnob (CC)
expandButton (Button)
toggleButton[1-4] (Button)
buttons[1-3] (Buttons)
knobs[1-3] (CCs)

When the effect unit is collapsed, the knobs control the metaknob of each effect in the unit.
The buttons control whether each effect is enabled. Pressing a button with shift switches to the
next available effect.

When the effect unit is expanded, the knobs control the first 3 parameters of one effect in the
effect unit. The effect that the knobs manipulate is selected by pressing one of the buttons.
Pressing a button with shift toggles whether the corresponding effect is enabled.

This EffectUnit provides Buttons to toggle whether the effect unit is enabled for a deck,
but not many controllers have buttons for that. If yours does not, you should probably enable
effect units for specific decks in your script's init() function.

To map an EffectUnit for your controller, call the constructor with the unit number of the effect
unit as the only argument. Then, set the midi attributes for the expandButton, buttons[1-3], and
optionally toggleButton[1-3] (setting the midi attributes for the CCs is not necessary because
they do not send any output). After the midi attributes are set up, call
EffectUnit.expandButton.trigger() then EffectUnit.reconnectControls(). For example:

MyController.effectUnit = new EffectUnit(1);
MyController.effectUnit.buttons[1].midi = [0x90, 0x01];
MyController.effectUnit.buttons[2].midi = [0x90, 0x02];
MyController.effectUnit.buttons[3].midi = [0x90, 0x03];
MyController.effectUnit.expandButton.midi = [0x90, 0x04];
MyController.effectUnit.expandButton.trigger();
MyController.effectUnit.reconnectControls();

Controllers designed for Serato and Rekordbox often have an encoder instead of a dry/wet knob
(labeled "Beats" for Serato or "Release FX" for Rekordbox) and a button labeled "Tap". It is
recommended to map the "Tap" button to the EffectUnit's expandButton. To use the dryWetKnob Control
with an encoder, replace its inValueScale() function with a function that can appropriately
handle the signals sent by your controller.
**/
EffectUnit = function (unitNumber) {
    this.group = '[EffectRack1_EffectUnit' + unitNumber + ']';

    this.dryWetKnob = new CC({
        group: this.group,
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
        outConnect: false,
    });

    this.toggleButton = new ControlContainer();
    for (var d = 1; d <= 4; d++) {
      this.toggleButton[d] = new Button({
          group: this.group,
          co: 'group_[Channel' + d + ']_enable',
          outConnect: false,
      });
    }

    var eu = this;
    this.activeEffect = 1;
    this.buttons = new ControlContainer();
    this.knobs = new ControlContainer();
    for (var d = 1; d <= 3; d++) {
        this.knobs[d] = new CC({
            number: d,
            collapse: function () {
                this.group = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + this.number + ']';
                this.inCo = 'meta';
            },
            expand: function () {
                this.group = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + eu.activeEffect + ']';
                this.inCo = 'parameter' + this.number;
            },
            outConnect: false,
        });
        this.buttons[d] = new Button({
            number: d,
            group: '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + d + ']',
            collapse: function () {
                this.input = Button.prototype.input;
                this.outCo = 'enabled';
                this.unshift = function () {
                    this.isShifted = false;
                    this.inCo = 'enabled';
                    this.onlyOnPress = true;
                };
                this.shift = function () {
                    this.isShifted = true;
                    this.inCo = 'next_effect';
                    this.onlyOnPress = false;
                };
                if (this.isShifted) {
                    this.shift();
                } else {
                    this.unshift();
                }
            },
            expand: function () {
                this.outCo = 'focused';
                this.unshift = function () {
                    this.isShifted = false;
                    this.inCo = 'focused';
                    this.input = function (channel, control, value, status, group) {
                        if (value > 0) {
                            this.toggle();
                            eu.activeEffect = this.number;
                            eu.knobs.reconnectControls(function (c) {
                                if (typeof c.expand === 'function') {
                                    c.expand(); // to set new group property
                                }
                            });
                        }
                    };
                };
                this.shift = function () {
                    this.isShifted = true;
                    this.input = Button.prototype.input;
                    this.inCo = 'enabled';
                    this.onlyOnPress = true;
                };
                if (this.isShifted) {
                    this.shift();
                } else {
                    this.unshift();
                }
            },
            outConnect: false
        });
    }

    this.expandButton = new Button({
        group: this.group,
        co: 'expanded',
        output: function (value, group, control) {
            this.send((value > 0) ? this.on : this.off);
            if (value === 0) {
                // NOTE: calling eu.reconnectControls() here would cause an infinite loop when
                // calling EffectUnit.reconnectControls().
                eu.forEachControl(function (c) {
                    if (typeof c.collapse === 'function') {
                        c.disconnect();
                        c.collapse();
                        c.connect();
                        c.trigger();
                    }
                });
            } else {
                eu.forEachControl(function (c) {
                    if (typeof c.expand === 'function') {
                        c.disconnect();
                        c.expand();
                        c.connect();
                        c.trigger();
                    }
                });
            }
        },
        outConnect: false
    });
};
EffectUnit.prototype = new ControlContainer();

var P32 = {};

P32.init = function () {
    Control.prototype.shiftOffset = 3;
    Control.prototype.shiftChannel = true;
    Button.prototype.sendShifted = true;
    CC.prototype.softTakeoverInit = false;

    P32.leftDeck = new P32.Deck([1,3], 1);
    P32.rightDeck = new P32.Deck([2,4], 2);

    engine.setValue('[EffectRack1_EffectUnit1]', 'group_[Channel1]_enable', 1);
    engine.setValue('[EffectRack1_EffectUnit1]', 'group_[Channel3]_enable', 1);

    engine.setValue('[EffectRack1_EffectUnit2]', 'group_[Channel2]_enable', 1);
    engine.setValue('[EffectRack1_EffectUnit2]', 'group_[Channel4]_enable', 1);

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
        });
    }

    this.pfl = new Button({
        midi: [0x90 + channel, 0x10],
        co: 'pfl',
    });

    this.volume = new CC({
        midi: [0xB0 + channel, 0x01],
        inCo: 'volume',
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

    this.effectUnit = new EffectUnit(deckNumbers[0]);
    this.effectUnit.buttons[1].midi = [0x90 + channel, 0x03];
    this.effectUnit.buttons[2].midi = [0x90 + channel, 0x04];
    this.effectUnit.buttons[3].midi = [0x90 + channel, 0x05];
    this.effectUnit.expandButton.midi = [0x90 + channel, 0x06];
    this.effectUnit.expandButton.trigger();
    this.effectUnit.toggleHeadphones = new Button({
        midi: [0x90 + channel, 0x34],
        co: 'group_[Headphone]_enable',
        on: P32.padColors.red,
        off: P32.padColors.blue
    });
    this.effectUnit.toggleMaster = new Button({
        midi: [0x90 + channel, 0x35],
        co: 'group_[Master]_enable',
        on: P32.padColors.red,
        off: P32.padColors.blue
    });
    this.effectUnit.toggleMicrophone = new Button({
        midi: [0x90 + channel, 0x36],
        co: 'group_[Microphone]_enable',
        on: P32.padColors.red,
        off: P32.padColors.blue
    });
    this.effectUnit.toggleAuxiliary = new Button({
        midi: [0x90 + channel, 0x37],
        co: 'group_[Auxiliary1]_enable',
        on: P32.padColors.red,
        off: P32.padColors.blue
    });
    this.effectUnit.reconnectControls(function (c) {
        if (c.group === undefined) {
            c.group = this.group;
        }
    });
};
P32.Deck.prototype = new Deck();
