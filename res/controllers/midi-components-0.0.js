/**
 * Components JS library for Mixxx
 * Documentation is on the Mixxx wiki at
 * http://mixxx.org/wiki/doku.php/components_js
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

;(function (global) {
    var Component = function (options) {
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

        if (options !== undefined && typeof options.key === 'string') {
            this.inKey = options.key;
            this.outKey = options.key;
        }

        if (this.outConnect && this.group !== undefined && this.outKey !== undefined) {
            this.connect();
            if (this.outTrigger) {
                this.trigger();
            }
        }
    };
    Component.prototype = {
        // default attributes
        // You should probably overwrite at least some of these.
        inValueScale: function (value) {
            return value / this.max;
        },
        // map input in the XML file, not inValueScale
        input: function (channel, control, value, status, group) {
            this.inSetParameter(this.inValueScale(value));
        },
        outValueScale: function (value) {return value * this.max;},
        output: function (value, group, control) {
            this.send(this.outValueScale(value));
        },
        outConnect: true,
        outTrigger: true,

        max: 127, // for MIDI. When adapting for HID this may change.

        // common functions
        // In most cases, you should not overwrite these.
        inGetParameter: function () {
            return engine.getParameter(this.group, this.inKey);
        },
        inSetParameter: function (value) {
            engine.setParameter(this.group, this.inKey, value);
        },
        inGetValue: function () {
            return engine.getValue(this.group, this.inKey);
        },
        inSetValue: function (value) {
            engine.setValue(this.group, this.inKey, value);
        },
        inToggle: function () {
            this.inSetValue( ! this.inGetValue());
        },

        outGetParameter: function () {
            return engine.getParameter(this.group, this.outKey);
        },
        outSetParameter: function (value) {
            engine.setParameter(this.group, this.outKey, value);
        },
        outGetValue: function () {
            return engine.getValue(this.group, this.outKey);
        },
        outSetValue: function (value) {
            engine.setValue(this.group, this.outKey, value);
        },
        outToggle: function () {
            this.outSetValue( ! this.outGetValue());
        },

        connect: function () {
            /**
            Override this method with a custom one to connect multiple Mixxx COs for a single Component.
            Add the connection objects to the this.connections array so they all get disconnected just
            by calling this.disconnect(). This can be helpful for multicolor LEDs that show a
            different color depending on the state of different Mixxx COs. Refer to
            SamplerButton.connect() and SamplerButton.output() for an example.
            **/
            if (undefined !== this.group &&
                undefined !== this.outKey &&
                undefined !== this.output &&
                typeof this.output === 'function') {
                this.connections[0] = engine.connectControl(this.group, this.outKey, this.output);
            }
        },
        disconnect: function () {
            if (this.connections[0] !== undefined) {
                this.connections.forEach(function (conn) {
                    conn.disconnect();
                });
            }
        },
        trigger: function() {
            if (this.connections[0] !== undefined) {
                this.connections.forEach(function (conn) {
                    conn.trigger();
                });
            }
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
        Component.call(this, options);
    };
    Button.prototype = new Component({
        onlyOnPress: true,
        on: 127,
        off: 0,
        // Time in milliseconds to distinguish a short press from a long press
        // In the provided Components, only EffectUnit.effectFocusButton uses
        // this, but it is recommended to refer to it (as this.longPressTimeout)
        // in any custom Buttons that act differently with short and long presses
        // to keep the timeouts uniform.
        longPressTimeout: 275,
        isPress: function (channel, control, value, status) {
            return value > 0;
        },
        inValueScale: function () { return ! this.inGetValue(); },
        input: function (channel, control, value, status, group) {
            if (this.onlyOnPress) {
                if (this.isPress(channel, control, value, status)) {
                    this.inSetValue(this.inValueScale(value));
                }
            } else {
                this.inSetValue(this.inValueScale(value));
            }
        },
        outValueScale: function (value) {
            return (value > 0) ? this.on : this.off;
        },
    });

    var PlayButton = function (options) {
        Button.call(this, options);
    };
    PlayButton.prototype = new Button({
        unshift: function () {
            this.inKey = 'play';
            this.input = Button.prototype.input;
            // Stop reversing playback if the user releases the shift button before releasing this PlayButton.
            if (engine.getValue(this.group, 'reverse') === 1) {
                engine.setValue(this.group, 'reverse', 0);
            }
        },
        shift: function () {
            this.input = function (channel, control, value, status, group) {
                engine.setValue(this.group, 'reverse', this.isPress(channel, control, value, status));
            };
        },
        outKey: 'play_indicator',
    });

    var CueButton = function (options) {
        Button.call(this, options);
    };
    CueButton.prototype = new Button({
        unshift: function () {
            this.inKey = 'cue_default';
        },
        shift: function () {
            this.inKey = 'start_stop';
        },
        input: function (channel, control, value, status, group) {
            this.inSetValue(this.isPress(channel, control, value, status));
        },
        outKey: 'cue_indicator',
    });

    var SyncButton = function (options) {
        Button.call(this, options);
    };
    SyncButton.prototype = new Button({
        unshift: function () {
            this.inKey = 'beatsync';
        },
        shift: function () {
            this.inKey = 'sync_enabled';
        },
        outKey: 'sync_enabled',
    });

    var LoopToggleButton = function (options) {
        Button.call(this, options);
    };
    LoopToggleButton.prototype = new Button({
        inKey: 'reloop_exit',
        inValueScale: function () {
            return 1;
        },
        outKey: 'loop_enabled',
    });

    var HotcueButton = function (options) {
        if (options.number === undefined) {
            print('ERROR: No hotcue number specified for new HotcueButton.');
            return;
        }
        this.number = options.number;
        this.outKey = 'hotcue_' + this.number + '_enabled';
        Button.call(this, options);
    };
    HotcueButton.prototype = new Button({
        unshift: function () {
            this.inKey = 'hotcue_' + this.number + '_activate';
        },
        shift: function () {
            this.inKey = 'hotcue_' + this.number + '_clear';
        },
        onlyOnPress: false,
    });

    var SamplerButton = function (options) {
        if (options.number === undefined) {
            print('ERROR: No sampler number specified for new SamplerButton.');
            return;
        }
        this.number = options.number;
        this.group = '[Sampler' + this.number + ']';
        Button.call(this, options);
    };
    SamplerButton.prototype = new Button({
        unshift: function () {
            this.input = function (channel, control, value, status, group) {
                if (this.isPress(channel, control, value, status)) {
                    if (engine.getValue(this.group, 'track_loaded') === 0) {
                        engine.setValue(this.group, 'LoadSelectedTrack', 1);
                    } else {
                        engine.setValue(this.group, 'cue_gotoandplay', 1);
                    }
                }
            };
        },
        shift: function() {
            this.input = function (channel, control, value, status, group) {
                if (this.isPress(channel, control, value, status)) {
                    if (engine.getValue(this.group, 'play') === 1) {
                        engine.setValue(this.group, 'play', 0);
                    } else {
                        engine.setValue(this.group, 'eject', 1);
                    }
                }
            };
        },
        output: function (value, group, control) {
            if (engine.getValue(this.group, 'track_loaded') === 1) {
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
            this.connections[0] = engine.connectControl(this.group, 'track_loaded', this.output);
            if (this.playing !== undefined) {
                this.connections[1] = engine.connectControl(this.group, 'play', this.output);
            }
        },
        outKey: null, // hack to get Component constructor to call connect()
    });

    var EffectAssignmentButton = function (options) {
        options.key = 'group_' + options.group + '_enable';
        options.group = '[EffectRack1_EffectUnit' + options.effectUnit + ']';
        Button.call(this, options);
    };
    EffectAssignmentButton.prototype = new Button();

    var Pot = function (options) {
        Component.call(this, options);

        this.firstValueReceived = false;
    };
    Pot.prototype = new Component({
        inValueScale: function (value) { return value / this.max; },
        input: function (channel, control, value, status, group) {
            this.inSetParameter(this.inValueScale(value));
            if (! this.firstValueReceived) {
                this.firstValueReceived = true;
                this.connect();
            }
        },
        connect: function () {
            if (this.firstValueReceived) {
                engine.softTakeover(this.group, this.inKey, true);
            }
        },
        disconnect: function () {
            engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
        },
        trigger: function () {},
    });

    /**
    The generic Component code provides everything to implement an Encoder. This Encoder Component
    exists so instanceof can be used to separate Encoders from other Components.
    **/
    var Encoder = function (options) {
        Component.call(this, options);
    };
    Encoder.prototype = new Component();

    var ComponentContainer = function (initialLayer) {
        if (typeof initialLayer === 'object') {
            this.applyLayer(initialLayer);
        }
    };
    ComponentContainer.prototype = {
        forEachComponent: function (operation, recursive) {
            if (typeof operation !== 'function') {
                print('ERROR: ComponentContainer.forEachComponent requires a function argument');
                return;
            }
            if (recursive === undefined) { recursive = true; }

            var that = this;
            var applyOperationTo = function (obj) {
                if (obj instanceof Component) {
                    operation.call(that, obj);
                } else if (recursive && obj instanceof ComponentContainer) {
                    obj.forEachComponent(operation);
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
        reconnectComponents: function (operation, recursive) {
            this.forEachComponent(function (component) {
                component.disconnect();
                if (typeof operation === 'function') {
                    operation.call(this, component);
                }
                component.connect();
                component.trigger();
            }, recursive);
        },
        isShifted: false,
        shift: function () {
            this.forEachComponent(function (component) {
                if (typeof component.shift === 'function') {
                    component.shift();
                }
                // Set isShifted for child ComponentContainers forEachComponent is iterating through recursively
                this.isShifted = true;
            });
        },
        unshift: function () {
            this.forEachComponent(function (component) {
                if (typeof component.unshift === 'function') {
                    component.unshift();
                }
                // Set isShifted for child ComponentContainers forEachComponent is iterating through recursively
                this.isShifted = false;
            });
        },
        applyLayer: function (newLayer, reconnectComponents) {
            if (reconnectComponents !== false) {
                reconnectComponents = true;
            }
            if (reconnectComponents === true) {
                this.forEachComponent(function (component) {
                    component.disconnect();
                });
            }

            _.merge(this, newLayer);

            if (reconnectComponents === true) {
                this.forEachComponent(function (component) {
                    component.connect();
                    component.trigger();
                });
            }
        },
    };

    var Deck = function (deckNumbers) {
        if (deckNumbers !== undefined) {
            if (Array.isArray(deckNumbers)) {
                // These must be unique to each instance,
                // so they cannot be in the prototype.
                this.currentDeck = '[Channel' + deckNumbers[0] + ']';
                this.deckNumbers = deckNumbers;
            } else if (typeof deckNumbers === 'number' &&
                      Math.floor(deckNumbers) === deckNumbers &&
                      isFinite(deckNumbers)) {
                this.currentDeck = '[Channel' + deckNumbers + ']';
                this.deckNumbers = [deckNumbers];
            }
        } else {
            print('ERROR! new Deck() called without specifying any deck numbers');
            return;
        }
    };
    Deck.prototype = new ComponentContainer({
        setCurrentDeck: function (newGroup) {
            this.currentDeck = newGroup;
            this.reconnectComponents(function (component) {
                if (component.group.search(script.channelRegEx) !== -1) {
                    component.group = this.currentDeck;
                } else if (component.group.search(script.eqRegEx) !== -1) {
                    component.group = '[EqualizerRack1_' + this.currentDeck + '_Effect1]';
                } else if (component.group.search(script.quickEffectRegEx) !== -1) {
                    component.group = '[QuickEffectRack1_' + this.currentDeck + ']';
                }
                // Do not alter the Component's group if it does not match any of those RegExs.

                if (component instanceof EffectAssignmentButton) {
                    // The ControlObjects for assinging decks to effect units
                    // indicate the effect unit with the group and the deck with the key,
                    // so change the key here instead of the group.
                    component.inKey = 'group_' + newGroup + '_enable';
                    component.outKey = 'group_' + newGroup + '_enable';
                }
            });
        },
        toggle: function () {
            // cycle through deckNumbers array
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

    EffectUnit = function (unitNumbers, allowFocusWhenParametersHidden) {
        var eu = this;

        // This is only connected if allowFocusWhenParametersHidden is false.
        this.onShowParametersChange = function (value) {
            if (value === 0) {
                // Prevent this from getting called twice (on button down and button up)
                // when show_parameters button is clicked in skin.
                // Otherwise this.previouslyFocusedEffect would always be set to 0
                // on the second call.
                if (engine.getValue(this.group, 'show_focus') > 0) {
                    engine.setValue(this.group, 'show_focus', 0);
                    this.previouslyFocusedEffect = engine.getValue(this.group,
                                                                  "focused_effect");
                    engine.setValue(this.group, "focused_effect", 0);
                }
            } else {
                engine.setValue(this.group, 'show_focus', 1);
                if (this.previouslyFocusedEffect !== undefined) {
                    engine.setValue(this.group, 'focused_effect',
                                    this.previouslyFocusedEffect);
                }
            }
        };

        this.setCurrentUnit = function (newNumber) {
            this.currentUnitNumber = newNumber;
            if (allowFocusWhenParametersHidden) {
                engine.setValue(this.group, 'show_focus', 0);
            } else {
                if (this.showParametersConnection !== undefined) {
                    this.showParametersConnection.disconnect();
                }
                delete this.previouslyFocusedEffect;
            }

            this.group = '[EffectRack1_EffectUnit' + newNumber + ']';

            if (allowFocusWhenParametersHidden) {
                engine.setValue(this.group, 'show_focus', 1);
            } else {
                // Connect a callback to show_parameters changing instead of
                // setting show_focus when effectFocusButton is pressed so
                // show_focus is always in the correct state, even if the user
                // presses the skin button for show_parameters.
                this.showParametersConnection = engine.connectControl(this.group,
                                                    'show_parameters',
                                                    this.onShowParametersChange);
                this.showParametersConnection.trigger();
            }

            for (var n = 1; n <= 3; n++) {
                var effect = '[EffectRack1_EffectUnit' + this.currentUnitNumber +
                            '_Effect' + n + ']';
                engine.softTakeover(effect, 'meta', true);
                engine.softTakeover(effect, 'parameter1', true);
                engine.softTakeover(effect, 'parameter2', true);
                engine.softTakeover(effect, 'parameter3', true);
            }

            this.reconnectComponents(function (component) {
                // update [EffectRack1_EffectUnitX] groups
                var unitMatch = component.group.match(script.effectUnitRegEx);
                if (unitMatch !== null) {
                    component.group = eu.group;
                } else {
                    // update [EffectRack1_EffectUnitX_EffectY] groups
                    var effectMatch = component.group.match(script.individualEffectRegEx);
                    if (effectMatch !== null) {
                        component.group = '[EffectRack1_EffectUnit' +
                                          eu.currentUnitNumber +
                                          '_Effect' + effectMatch[2] + ']';
                    }
                }
            });
        };

        this.toggle = function () {
            // cycle through unitNumbers array
            var index = this.unitNumbers.indexOf(this.currentUnitNumber);
            if (index === (this.unitNumbers.length - 1)) {
                index = 0;
            } else {
                index += 1;
            }
            this.setCurrentUnit(this.unitNumbers[index]);
        }

        if (unitNumbers !== undefined) {
            if (Array.isArray(unitNumbers)) {
                this.unitNumbers = unitNumbers;
                this.setCurrentUnit(unitNumbers[0]);
            } else if (typeof unitNumbers === 'number' &&
                      Math.floor(unitNumbers) === unitNumbers &&
                      isFinite(unitNumbers)) {
                this.unitNumbers = [unitNumbers];
                this.setCurrentUnit(unitNumbers);
            }
        } else {
            print('ERROR! new EffectUnit() called without specifying any unit numbers!');
            return;
        }

        this.dryWetKnob = new Pot({
            group: this.group,
            unshift: function () {
                this.inKey = 'mix';
                // for soft takeover
                this.disconnect();
                this.connect();
            },
            shift: function () {
                this.inKey = 'super1';
                // for soft takeover
                this.disconnect();
                this.connect();
                // engine.softTakeoverIgnoreNextValue is called
                // in the knobs' onFocusChange function
                eu.knobs.forEachComponent(function (knob) {
                    knob.trigger();
                });
            },
            outConnect: false,
        });

        this.enableOnChannelButtons = new ComponentContainer();
        this.enableOnChannelButtons.addButton = function (channel) {
            this[channel] = new Button({
                group: eu.group,
                key: 'group_[' + channel + ']_enable',
                outConnect: false,
            });
        };

        this.EffectUnitKnob = function (number) {
            this.number = number;
            Pot.call(this);
        };
        this.EffectUnitKnob.prototype = new Pot({
            group: this.group,
            input: function (channel, control, value, status, group) {
                this.inSetParameter(this.inValueScale(value));
                // Unlike the Pot prototype, do not enable soft takeover here
                // because soft takeover needs to be enabled for all the
                // metaknobs and parameters 1-3 of all 3 effects. Instead, enabling
                // soft takeover is handled below in the loop that calls the
                // EffectUnitKnob constructor.
            },
            outKey: "focused_effect",
            connect: function () {
                this.connections[0] = engine.connectControl(eu.group, "focused_effect",
                                                            this.onFocusChange);
            },
            disconnect: function () {
                engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
                this.connections[0].disconnect();
            },
            trigger: function () {
                this.connections[0].trigger();
            },
            onFocusChange: function (value, group, control) {
                if (value === 0) {
                    this.group = '[EffectRack1_EffectUnit' +
                                  eu.currentUnitNumber + '_Effect' +
                                  this.number + ']';
                    this.inKey = 'meta';
                    engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
                } else {
                    this.group = '[EffectRack1_EffectUnit' + eu.currentUnitNumber +
                                  '_Effect' + value + ']';
                    this.inKey = 'parameter' + this.number;
                    engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
                }
            },
        });

        this.EffectEnableButton = function (number) {
            this.number = number;
            this.group = '[EffectRack1_EffectUnit' + eu.currentUnitNumber +
                          '_Effect' + this.number + ']';
            Button.call(this);
        };
        this.EffectEnableButton.prototype = new Button({
            stopEffectFocusChooseMode: function () {
                this.inKey = 'enabled';
                this.outKey = 'enabled';
                this.input = Button.prototype.input;
                this.connect = Button.prototype.connect;
                this.output = Button.prototype.output;
            },
            startEffectFocusChooseMode: function () {
                this.input = function (channel, control, value, status, group) {
                    if (this.isPress(channel, control, value, status)) {
                        if (engine.getValue(eu.group, "focused_effect") === this.number) {
                            // unfocus and make knobs control metaknobs
                            engine.setValue(eu.group, "focused_effect", 0);
                        } else {
                            // focus this effect
                            engine.setValue(eu.group, "focused_effect", this.number);
                        }
                    }
                };
                this.connect = function () {
                    this.connections[0] = engine.connectControl(eu.group,
                                                                "focused_effect",
                                                                this.output);
                };
                this.output = function (value, group, control) {
                    this.send(value === this.number);
                };
            },
        });

        this.knobs = new ComponentContainer();
        this.enableButtons = new ComponentContainer();
        for (var n = 1; n <= 3; n++) {
            this.knobs[n] = new this.EffectUnitKnob(n);
            this.enableButtons[n] = new this.EffectEnableButton(n);
        }

        this.effectFocusButton = new Button({
            group: this.group,
            longPressed: false,
            focusChooseModeActive: false,
            longPressTimer: 0,
            pressedWhenParametersHidden: false,
            previouslyFocusedEffect: 0,
            startEffectFocusChooseMode: function () {
                this.focusChooseModeActive = true;
                eu.enableButtons.reconnectComponents(function (button) {
                    button.startEffectFocusChooseMode();
                });
            },
            unshift: function () {
                this.input = function (channel, control, value, status, group) {
                    var showParameters = engine.getValue(this.group, "show_parameters");
                    if (this.isPress(channel, control, value, status)) {
                        this.longPressTimer = engine.beginTimer(this.longPressTimeout,
                                                      this.startEffectFocusChooseMode,
                                                      true);
                        if (!showParameters) {
                            if (!allowFocusWhenParametersHidden) {
                                engine.setValue(this.group, "show_parameters", 1);
                                // eu.onShowParametersChange will refocus the
                                // previously focused effect and show focus in skin
                            }
                            this.pressedWhenParametersHidden = true;
                        }
                    } else {
                        if (this.longPressTimer) {
                            engine.stopTimer(this.longPressTimer);
                        }

                        if (this.focusChooseModeActive) {
                            eu.enableButtons.reconnectComponents(function (button) {
                                button.stopEffectFocusChooseMode();
                            });
                            this.focusChooseModeActive = false;
                        } else {
                            if (!showParameters && allowFocusWhenParametersHidden) {
                                  engine.setValue(this.group, "show_parameters", 1);
                            } else if (showParameters && !this.pressedWhenParametersHidden) {
                                  engine.setValue(this.group, "show_parameters", 0);
                                  // eu.onShowParametersChange will save the focused effect,
                                  // unfocus, and hide focus buttons in skin
                            }
                        }
                        this.pressedWhenParametersHidden = false;
                    }
                };
            },
            shift: function () {
                this.input = function (channel, control, value, status, group) {
                    if (this.isPress(channel, control, value, status)) {
                        eu.toggle();
                    }
                };
            },
            outKey: 'focused_effect',
            output: function (value, group, control) {
                this.send((value > 0) ? this.on : this.off);
            },
            outConnect: false,
        });

        this.init = function () {
            this.knobs.reconnectComponents();
            this.enableButtons.reconnectComponents(function (button) {
                button.stopEffectFocusChooseMode();
            });
            this.effectFocusButton.connect();
            this.effectFocusButton.trigger();

            this.enableOnChannelButtons.forEachComponent(function (button) {
                if (button.midi !== undefined) {
                    button.disconnect();
                    button.connect();
                    button.trigger();
                }
            });

            this.forEachComponent(function (component) {
                if (component.group === undefined) {
                    component.group = eu.group;
                }
            });
        };
    };
    EffectUnit.prototype = new ComponentContainer();

    var exports = {};
    exports.Component = Component;
    exports.Button = Button;
    exports.PlayButton = PlayButton;
    exports.CueButton = CueButton;
    exports.SyncButton = SyncButton;
    exports.LoopToggleButton = LoopToggleButton;
    exports.HotcueButton = HotcueButton;
    exports.SamplerButton = SamplerButton;
    exports.EffectAssignmentButton = EffectAssignmentButton;
    exports.Pot = Pot;
    exports.Encoder = Encoder;
    exports.ComponentContainer = ComponentContainer;
    exports.Deck = Deck;
    exports.EffectUnit = EffectUnit;
    global.components = exports;
}(this));
