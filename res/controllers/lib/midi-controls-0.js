/**
 * Controls JS library for Mixxx
 * Documentation is on the Mixxx wiki at
 * http://mixxx.org/wiki/doku.php/controls_js
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
 *
 *
 *
 * This library depends on Lodash, which is copyright JS Foundation
 * and other contributors and licensed under the MIT license. Refer to
 * the lodash.mixxx.js file in this directory for details.
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

    if (options !== undefined && typeof options.co === 'string') {
        this.inCo = options.co;
        this.outCo = options.co;
    }

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
    inValueScale: function (value) {
        return value / this.max;
    },
    // map input in the XML file, not inValueScale
    input: function (channel, control, value, status, group) {
        this.setParameter(this.inValueScale(value));
    },
    max: 127, // for MIDI. When adapting for HID this may change.
    outValueScale: function (value) {return value * this.max;},
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
    setParameter: function (value) {
        print (this.inCo);
        engine.setParameter(this.group, this.inCo, value);
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
    getParameterIn: function () {
        return engine.getParameter(this.group, this.inCo);
    },
    getParameterOut: function () {
        return engine.getParameter(this.group, this.outCo);
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
        if (undefined !== this.group &&
            undefined !== this.outCo &&
            undefined !== this.output &&
            typeof this.output === 'function') {
            this.connections[0] = engine.connectControl(this.group, this.outCo, this.output);
        }
    },
    disconnect: function () {
        if (this.connections[0] !== undefined) {
            this.connections.forEach(function (connection) {
                connection.disconnect();
            });
        }
    },
    trigger: function() {
        engine.trigger(this.group, this.outCo);
    },
    shiftOffset: 0,
    sendShifted: false,
    shiftChannel: false,
    shiftControl: false,
    send: function (value) {
        if (this.midi === undefined || this.midi[0] === undefined || this.midi[1] === undefined) {
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

var Button = function (options) {
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
    outValueScale: function() {
        return (this.getValueOut()) ? this.on : this.off;
    },
});

var PlayButton = function (options) {
    Button.call(this, options);
};
PlayButton.prototype = new Button({
    unshift: function () {
        this.inCo = 'play';
    },
    shift: function () {
        this.inCo = 'start_stop';
    },
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
    unshift: function () {
        this.inCo = 'beatsync';
    },
    shift: function () {
        this.inCo = 'sync_enabled';
    },
    outCo: 'sync_enabled'
});

var LoopToggleButton = function (options) {
    Button.call(this, options);
};
LoopToggleButton.prototype = new Button({
    inCo: 'reloop_exit',
    inValueScale: function () {
        return 1;
    },
    outCo: 'loop_enabled',
    outValueScale: function (value) {
        return (value) ? this.on : this.off;
    }
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
    unshift: function () {
        this.inCo = 'hotcue_' + this.number + '_activate';
    },
    shift: function () {
        this.inCo = 'hotcue_' + this.number + '_clear';
    },
    onlyOnPress: false
});

var SamplerButton = function (options) {
    if (options.number === undefined) {
        print('WARNING: No sampler number specified for new SamplerButton.');
    }
    this.number = options.number;
    this.group = '[Sampler' + this.number + ']';
    Button.call(this, options);
};
SamplerButton.prototype = new Button({
    unshift: function () {
        this.input = function (channel, control, value, status, group) {
            if (value > 0) {
                // track_samples is 0 when the sampler is empty and > 0 when a sample is loaded
                if (engine.getValue(this.group, 'track_samples') === 0) {
                    engine.setValue(this.group, 'LoadSelectedTrack', 1);
                } else {
                    engine.setValue(this.group, 'cue_gotoandplay', 1);
                }
            }
        };
    },
    shift: function() {
        this.input = function (channel, control, value, status, group) {
            if (value > 0) {
                if (engine.getValue(this.group, 'play') === 1) {
                    engine.setValue(this.group, 'play', 0);
                } else {
                    engine.setValue(this.group, 'eject', 1);
                }
            }
        };
    },
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

var Pot = function (options) {
    Control.call(this, options);

    this.firstValueReceived = false;
};
Pot.prototype = new Control({
    inValueScale: function (value) { return value / this.max; },
    input: function (channel, control, value, status, group) {
        this.setParameter(this.inValueScale(value));
        if (! this.firstValueReceived) {
            this.firstValueReceived = true;
            this.connect();
        }
    },
    connect: function () {
        if (this.firstValueReceived) {
            engine.softTakeover(this.group, this.inCo, true);
        }
    },
    disconnect: function () {
        engine.softTakeoverIgnoreNextValue(this.group, this.inCo);
    },
    trigger: function () {},
});

/**
The generic Control code provides everything to implement a RingEncoder. This RingEncoder Control
to be able to use instanceof to separate RingEncoders from other Controls and make code more
self-documenting.
**/
var RingEncoder = function (options) {
    Control.call(this, options);
};
RingEncoder.prototype = new Control();

var ControlContainer = function (initialLayer) {
    if (typeof initialLayer === 'object') {
        this.applyLayer(initialLayer);
    }
};
ControlContainer.prototype = {
    forEachControl: function (operation, recursive) {
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
        this.forEachControl(function (control) {
            control.disconnect();
            if (typeof operation === 'function') {
                operation.call(this, control);
            }
            control.connect();
            control.trigger();
        }, recursive);
    },
    isShifted: false,
    shift: function () {
        this.forEachControl(function (control) {
            if (typeof control.shift === 'function') {
                control.shift();
            }
            // Set isShifted for child ControlContainers forEachControl is iterating through recursively
            this.isShifted = true;
        });
    },
    unshift: function () {
        this.forEachControl(function (control) {
            if (typeof control.unshift === 'function') {
                control.unshift();
            }
            // Set isShifted for child ControlContainers forEachControl is iterating through recursively
            this.isShifted = false;
        });
    },
    applyLayer: function (newLayer, reconnectControls) {
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

EffectUnit = function (unitNumber) {
    var eu = this;
    this.group = '[EffectRack1_EffectUnit' + unitNumber + ']';

    this.dryWetKnob = new Pot({
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
            eu.knobs.reconnectControls();
        },
        outConnect: false,
    });

    this.enableOnChannelButtons = new ControlContainer();
    this.enableOnChannelButtons.addButton = function (channel) {
        this[channel] = new Button({
            group: eu.group,
            co: 'group_[' + channel + ']_enable',
            outConnect: false,
        });
    };
    this.enableOnChannelButtons.addButton('Channel1');
    this.enableOnChannelButtons.addButton('Channel2');
    this.enableOnChannelButtons.addButton('Channel3');
    this.enableOnChannelButtons.addButton('Channel4');
    this.enableOnChannelButtons.addButton('Headphone');
    this.enableOnChannelButtons.addButton('Master');
    this.enableOnChannelButtons.addButton('Microphone');
    this.enableOnChannelButtons.addButton('Auxiliary1');

    this.EffectUnitKnob = function (number) {
        this.number = number;
        Pot.call(this);
    };
    this.EffectUnitKnob.prototype = new Pot({
        onParametersHide: function () {
            this.group = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + this.number + ']';
            this.inCo = 'meta';
        },
        onParametersShow: function () {
            var focused_effect = engine.getValue(eu.group, "focused_effect");
            if (focused_effect === 0) {
                // manipulate metaknobs
                this.onParametersHide();
            } else {
                this.group = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' +
                              focused_effect + ']';
                this.inCo = 'parameter' + this.number;
            }
        },
    });

    this.EffectEnableButton = function (number) {
        this.number = number;
        this.group = '[EffectRack1_EffectUnit' + unitNumber + '_Effect' + number + ']';
        Button.call(this);
    };
    this.EffectEnableButton.prototype = new Button({
        onParametersHide: function () {
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
        onParametersShow: function () {
            this.inCo = 'enabled';
            this.outCo = 'enabled';
            this.unshift = function () {
                this.isShifted = false;
                this.input = Button.prototype.input;
                this.onlyOnPress = true;
            };
            this.shift = function () {
                this.isShifted = true;
                this.input = function (channel, control, value, status, group) {
                    if (value > 0) {
                        if (engine.getValue(eu.group, "focused_effect") === this.number) {
                            // unfocus and make knobs control metaknobs
                            engine.setValue(eu.group, "focused_effect", 0);
                        } else {
                            // focus this effect
                            engine.setValue(eu.group, "focused_effect", this.number);
                        }
                    }
                };
            };
            this.connect = function () {
                this.connections[0] = engine.connectControl(this.group, "enabled", Button.prototype.output);
                this.connections[1] = engine.connectControl(eu.group, "focused_effect", this.onFocusChanged);
            };
            this.onFocusChanged = function (value, group, control) {
                if (value === this.number) {
                    // make knobs control first 3 parameters of the focused effect
                    eu.knobs.reconnectControls(function (knob) {
                        if (typeof knob.onParametersShow === 'function') {
                            knob.onParametersShow(); // to set new group property
                        }
                    });
                } else if (value === 0) {
                    // make knobs control metaknobs
                    eu.knobs.reconnectControls(function (knob) {
                        if (typeof knob.onParametersShow === 'function') {
                            knob.onParametersHide(); // to set new group property
                        }
                    });
                }
            };
            if (this.isShifted) {
                this.shift();
            } else {
                this.unshift();
            }
        },
    });

    this.knobs = new ControlContainer();
    this.enableButtons = new ControlContainer();
    for (var n = 1; n <= 3; n++) {
        this.knobs[n] = new this.EffectUnitKnob(n);
        this.enableButtons[n] = new this.EffectEnableButton(n);
    }

    this.showParametersButton = new Button({
        group: this.group,
        co: 'show_parameters',
        output: function (value, group, control) {
            this.send((value > 0) ? this.on : this.off);
            if (value === 0) {
                engine.setValue(this.group, "show_focus", 0);
                // NOTE: calling eu.reconnectControls() here would cause an infinite loop when
                // calling EffectUnit.reconnectControls().
                eu.forEachControl(function (c) {
                    if (typeof c.onParametersHide === 'function') {
                        c.disconnect();
                        c.onParametersHide();
                        c.connect();
                        c.trigger();
                    }
                });
            } else {
                engine.setValue(this.group, "show_focus", 1);
                eu.forEachControl(function (c) {
                    if (typeof c.onParametersShow === 'function') {
                        c.disconnect();
                        c.onParametersShow();
                        c.connect();
                        c.trigger();
                    }
                });
            }
        },
        outConnect: false,
    });

    this.init = function () {
        this.showParametersButton.connect();
        this.showParametersButton.trigger();

        this.enableOnChannelButtons.forEachControl(function (button) {
            if (button.midi !== undefined) {
                button.disconnect();
                button.connect();
                button.trigger();
            }
        });

        this.forEachControl(function (control) {
            if (control.group === undefined) {
                control.group = eu.group;
            }
        });
    };
};
EffectUnit.prototype = new ControlContainer();
