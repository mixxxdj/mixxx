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

(function(global) {
    var Component = function(options) {
        if (Array.isArray(options) && typeof options[0] === "number") {
            this.midi = options;
        } else {
            _.assign(this, options);
        }

        if (typeof this.unshift === "function") {
            this.unshift();
        }
        // These cannot be in the prototype; they must be unique to each instance.
        this.isShifted = false;
        this.connections = [];

        if (options !== undefined && typeof options.key === "string") {
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
        inValueScale: function(value) {
            // Hack to get exact center of pots to return 0.5
            if (value > (this.max / 2)) {
                return (value - 1) / (this.max - 1);
            } else {
                return value / (this.max + 1);
            }
        },
        // map input in the XML file, not inValueScale
        input: function(channel, control, value, _status, _group) {
            this.inSetParameter(this.inValueScale(value));
        },
        outValueScale: function(value) { return value * this.max; },
        output: function(value, _group, _control) {
            this.send(this.outValueScale(value));
        },
        outConnect: true,
        outTrigger: true,

        max: 127, // for MIDI. When adapting for HID this may change.

        // common functions
        // In most cases, you should not overwrite these.
        inGetParameter: function() {
            return engine.getParameter(this.group, this.inKey);
        },
        inSetParameter: function(value) {
            engine.setParameter(this.group, this.inKey, value);
        },
        inGetValue: function() {
            return engine.getValue(this.group, this.inKey);
        },
        inSetValue: function(value) {
            engine.setValue(this.group, this.inKey, value);
        },
        inToggle: function() {
            this.inSetValue(!this.inGetValue());
        },

        outGetParameter: function() {
            return engine.getParameter(this.group, this.outKey);
        },
        outSetParameter: function(value) {
            engine.setParameter(this.group, this.outKey, value);
        },
        outGetValue: function() {
            return engine.getValue(this.group, this.outKey);
        },
        outSetValue: function(value) {
            engine.setValue(this.group, this.outKey, value);
        },
        outToggle: function() {
            this.outSetValue(!this.outGetValue());
        },

        connect: function() {
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
                typeof this.output === "function") {
                this.connections[0] = engine.makeConnection(this.group, this.outKey, this.output);
            }
        },
        disconnect: function() {
            if (this.connections[0] !== undefined) {
                this.connections.forEach(function(conn) {
                    conn.disconnect();
                });
            }
        },
        trigger: function() {
            if (this.connections[0] !== undefined) {
                this.connections.forEach(function(conn) {
                    conn.trigger();
                });
            }
        },
        shiftOffset: 0,
        sendShifted: false,
        shiftChannel: false,
        shiftControl: false,
        send: function(value) {
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

    var Button = function(options) {
        Component.call(this, options);
    };
    Button.prototype = new Component({
        types: {
            push: 0,
            toggle: 1,
            powerWindow: 2,
        },
        type: 0,
        on: 127,
        off: 0,
        // Time in milliseconds to distinguish a short press from a long press.
        // It is recommended to refer to it (as this.longPressTimeout)
        // in any Buttons that act differently with short and long presses
        // to keep the timeouts uniform.
        longPressTimeout: 275,
        isPress: function(channel, control, value, _status) {
            return value > 0;
        },
        input: function(channel, control, value, status, _group) {
            if (this.type === undefined || this.type === this.types.push) {
                this.inSetValue(this.isPress(channel, control, value, status));
            } else if (this.type === this.types.toggle) {
                if (this.isPress(channel, control, value, status)) {
                    this.inToggle();
                }
            } else if (this.type === this.types.powerWindow) {
                if (this.isPress(channel, control, value, status)) {
                    this.inToggle();
                    this.isLongPressed = false;
                    this.longPressTimer = engine.beginTimer(this.longPressTimeout, function() {
                        this.isLongPressed = true;
                        this.longPressTimer = 0;
                    }, true);
                } else {
                    if (this.isLongPressed) {
                        this.inToggle();
                    }
                    if (this.longPressTimer !== 0) {
                        engine.stopTimer(this.longPressTimer);
                        this.longPressTimer = 0;
                    }
                    this.isLongPressed = false;
                }
            }
        },
        outValueScale: function(value) {
            return (value > 0) ? this.on : this.off;
        },

        shutdown: function() {
            this.send(this.off);
        },
    });

    var PlayButton = function(options) {
        Button.call(this, options);
    };
    PlayButton.prototype = new Button({
        unshift: function() {
            this.inKey = "play";
        },
        shift: function() {
            this.inKey = "reverse";
        },
        type: Button.prototype.types.toggle,
        outKey: "play_indicator",
    });

    var CueButton = function(options) {
        Button.call(this, options);
    };
    CueButton.prototype = new Button({
        unshift: function() {
            this.inKey = "cue_default";
        },
        shift: function() {
            if (this.reverseRollOnShift) {
                this.inKey = "reverseroll";
            } else {
                this.inKey = "start_stop";
            }
        },
        outKey: "cue_indicator",
    });

    var SyncButton = function(options) {
        Button.call(this, options);
    };
    SyncButton.prototype = new Button({
        unshift: function() {
            this.input = function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status)) {
                    if (engine.getValue(this.group, "sync_enabled") === 0) {
                        engine.setValue(this.group, "beatsync", 1);
                        this.longPressTimer = engine.beginTimer(this.longPressTimeout, function() {
                            engine.setValue(this.group, "sync_enabled", 1);
                            this.longPressTimer = 0;
                        }, true);
                    } else {
                        engine.setValue(this.group, "sync_enabled", 0);
                    }
                } else {
                    if (this.longPressTimer !== 0) {
                        engine.stopTimer(this.longPressTimer);
                        this.longPressTimer = 0;
                    }
                }
            };
        },
        shift: function() {
            this.inKey = "quantize";
            this.type = Button.prototype.types.toggle;
            this.input = Button.prototype.input;
        },
        outKey: "sync_enabled",
    });

    var LoopToggleButton = function(options) {
        Button.call(this, options);
    };
    LoopToggleButton.prototype = new Button({
        inKey: "reloop_exit",
        inValueScale: function() {
            return 1;
        },
        outKey: "loop_enabled",
    });

    var HotcueButton = function(options) {
        if (options.number === undefined) {
            print("ERROR: No hotcue number specified for new HotcueButton.");
            return;
        }
        if (options.colorMapper !== undefined || options.sendRGB !== undefined) {
            this.colorKey = "hotcue_" + options.number + "_color";
        }
        this.number = options.number;
        this.outKey = "hotcue_" + this.number + "_enabled";
        Button.call(this, options);
    };
    HotcueButton.prototype = new Button({
        unshift: function() {
            this.inKey = "hotcue_" + this.number + "_activate";
        },
        shift: function() {
            this.inKey = "hotcue_" + this.number + "_clear";
        },
        output: function(value) {
            var outval = this.outValueScale(value);
            // NOTE: outputColor only handles hotcueColors
            // and there is no hotcueColor for turning the LED
            // off. So the `send()` function is responsible for turning the
            // actual LED off.
            if (this.colorKey !== undefined && outval !== this.off) {
                this.outputColor(engine.getValue(this.group, this.colorKey));
            } else {
                this.send(outval);
            }
        },
        outputColor: function(colorCode) {
            // Sends the color from the colorCode to the controller. This
            // method will not be called if no colorKey has been specified.
            if (colorCode === undefined || colorCode < 0 || colorCode > 0xFFFFFF) {
                print("Ignoring invalid color code '" + colorCode + "' in outputColor()");
                return;
            }

            if (this.colorMapper !== undefined) {
                // This HotcueButton holds a reference to a ColorMapper. This means
                // that the controller only supports a fixed set of colors, so we
                // get the MIDI value for the nearest supported color and send it.
                var nearestColorValue = this.colorMapper.getValueForNearestColor(colorCode);
                this.send(nearestColorValue);
            } else {
                // Since outputColor has been called but no ColorMapper is
                // available, we can assume that controller supports arbitrary
                // RGB color output.
                this.sendRGB(colorCodeToObject(colorCode));
            }
        },
        sendRGB: function(_colorObject) {
            // This method needs to be overridden in controller mappings,
            // because the procedure is controller-dependent.
            throw Error("sendRGB(colorObject) not implemented - unable to send RGB colors!");
        },
        connect: function() {
            Button.prototype.connect.call(this); // call parent connect
            if (undefined !== this.group && this.colorKey !== undefined) {
                this.connections[1] = engine.makeConnection(this.group, this.colorKey, function(color) {
                    if (engine.getValue(this.group, this.outKey)) {
                        this.outputColor(color);
                    }
                });
            }
        },
    });
    var SamplerButton = function(options) {
        if (options.number === undefined) {
            print("ERROR: No sampler number specified for new SamplerButton.");
            return;
        }
        this.volumeByVelocity = options.volumeByVelocity;
        this.number = options.number;
        this.group = "[Sampler" + this.number + "]";
        Button.call(this, options);
    };
    SamplerButton.prototype = new Button({
        unshift: function() {
            this.input = function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status)) {
                    if (engine.getValue(this.group, "track_loaded") === 0) {
                        engine.setValue(this.group, "LoadSelectedTrack", 1);
                    } else {
                        if (this.volumeByVelocity) {
                            engine.setValue(this.group, "volume", this.inValueScale(value));
                        }
                        engine.setValue(this.group, "cue_gotoandplay", 1);
                    }
                }
            };
        },
        shift: function() {
            this.input = function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status)) {
                    if (engine.getValue(this.group, "play") === 1) {
                        engine.setValue(this.group, "play", 0);
                    } else {
                        engine.setValue(this.group, "eject", 1);
                    }
                } else {
                    if (engine.getValue(this.group, "play") === 0) {
                        engine.setValue(this.group, "eject", 0);
                    }
                }
            };
        },
        output: function(_value, _group, _control) {
            if (engine.getValue(this.group, "track_loaded") === 1) {
                if (this.loaded === undefined) {
                    this.send(this.on);
                } else {
                    if (engine.getValue(this.group, "play") === 1) {
                        if (this.looping !== undefined &&
                            engine.getValue(this.group, "repeat") === 1) {
                            this.send(this.looping);
                        } else {
                            this.send(this.playing);
                        }
                    } else {
                        this.send(this.loaded);
                    }
                }
            } else {
                if (this.empty === undefined) {
                    this.send(this.off);
                } else {
                    this.send(this.empty);
                }
            }
        },
        connect: function() {
            this.connections[0] = engine.makeConnection(this.group, "track_loaded", this.output);
            if (this.playing !== undefined) {
                this.connections[1] = engine.makeConnection(this.group, "play", this.output);
            }
            if (this.looping !== undefined) {
                this.connections[2] = engine.connectControl(this.group, "repeat", this.output);
            }
        },
        outKey: null, // hack to get Component constructor to call connect()
    });

    var EffectAssignmentButton = function(options) {
        options.key = "group_" + options.group + "_enable";
        options.group = "[EffectRack1_EffectUnit" + options.effectUnit + "]";
        Button.call(this, options);
    };
    EffectAssignmentButton.prototype = new Button({
        type: Button.prototype.types.toggle,
    });

    var Pot = function(options) {
        Component.call(this, options);

        this.firstValueReceived = false;
    };
    Pot.prototype = new Component({
        input: function(channel, control, value, _status, _group) {
            if (this.MSB !== undefined) {
                value = (this.MSB << 7) + value;
            }
            var newValue = this.inValueScale(value);
            if (this.invert) {
                newValue = 1 - newValue;
            }
            this.inSetParameter(newValue);
            if (!this.firstValueReceived) {
                this.firstValueReceived = true;
                this.connect();
            }
        },
        // Input handlers for 14 bit MIDI
        inputMSB: function(channel, control, value, status, group) {
            // For the first messages, disregard the LSB in case
            // the first LSB is received after the first MSB.
            if (this.MSB === undefined) {
                this.max = 127;
                this.input(channel, control, value, status, group);
                this.max = 16383;
            }
            this.MSB = value;
        },
        inputLSB: function(channel, control, value, status, group) {
            // Make sure the first MSB has been received
            if (this.MSB !== undefined) {
                this.input(channel, control, value, status, group);
            }
        },
        connect: function() {
            if (this.firstValueReceived && !this.relative) {
                engine.softTakeover(this.group, this.inKey, true);
            }
        },
        disconnect: function() {
            if (!this.relative) {
                engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
            }
        },
        trigger: function() {},
    });

    /**
    The generic Component code provides everything to implement an Encoder. This Encoder Component
    exists so instanceof can be used to separate Encoders from other Components.
    **/
    var Encoder = function(options) {
        Component.call(this, options);
    };
    Encoder.prototype = new Component();

    var ComponentContainer = function(initialLayer) {
        if (typeof initialLayer === "object") {
            this.applyLayer(initialLayer);
        }
    };
    ComponentContainer.prototype = {
        forEachComponent: function(operation, recursive) {
            if (typeof operation !== "function") {
                print("ERROR: ComponentContainer.forEachComponent requires a function argument");
                return;
            }
            if (recursive === undefined) { recursive = true; }

            var that = this;
            var applyOperationTo = function(obj) {
                if (obj instanceof Component) {
                    operation.call(that, obj);
                } else if (recursive && obj instanceof ComponentContainer) {
                    obj.forEachComponent(operation);
                } else if (Array.isArray(obj)) {
                    obj.forEach(function(element) {
                        applyOperationTo(element);
                    });
                }
            };

            for (var memberName in this) {
                if (ComponentContainer.prototype.hasOwnProperty.call(this, memberName)) {
                    applyOperationTo(this[memberName]);
                }
            }
        },
        forEachComponentContainer: function(operation, recursive) {
            if (typeof operation !== "function") {
                print("ERROR: ComponentContainer.forEachComponentContainer requires a function argument");
                return;
            }
            if (recursive === undefined) { recursive = true; }

            var that = this;
            var applyOperationTo = function(obj) {
                if (obj instanceof ComponentContainer) {
                    operation.call(that, obj);

                    if (recursive) {
                        obj.forEachComponentContainer(operation);
                    }
                } else if (Array.isArray(obj)) {
                    obj.forEach(function(element) {
                        applyOperationTo(element);
                    });
                }
            };

            for (var memberName in this) {
                if (ComponentContainer.prototype.hasOwnProperty.call(this, memberName)) {
                    applyOperationTo(this[memberName]);
                }
            }
        },
        reconnectComponents: function(operation, recursive) {
            this.forEachComponent(function(component) {
                component.disconnect();
                if (typeof operation === "function") {
                    operation.call(this, component);
                }
                component.connect();
                component.trigger();
            }, recursive);
        },
        isShifted: false,
        shift: function() {
            // Shift direct child Components
            this.forEachComponent(function(component) {
                // Controls for push type Buttons depend on getting reset to 0 when the
                // Button is released for correct behavior. If there is a skin button
                // that lights up with the inKey, the skin button would stay lit if the
                // inKey does not get reset to 0. So, if a push type Button is held down
                // when shift is pressed, when the Button is released, the MIDI signal
                // for the Button release will be processed when the Button is in the
                // shifted state, and the unshifted inKey would not get reset to 0.
                // To work around this, reset push Buttons' inKey to 0 when the shift
                // button is pressed.
                if (typeof component.shift === "function") {
                    if (component instanceof Button
                        && (component.type === Button.prototype.types.push
                            || component.type === undefined)
                        && component.input === Button.prototype.input
                        && typeof component.inKey === "string"
                        && typeof component.group === "string") {
                        if (engine.getValue(component.group, component.inKey) !== 0) {
                            engine.setValue(component.group, component.inKey, 0);
                        }
                    }
                    component.shift();
                }
            }, false);

            // Shift child ComponentContainers
            this.forEachComponentContainer(function(container) {
                container.shift();
            }, false);

            // Set isShifted for each ComponentContainer recursively
            this.isShifted = true;
        },
        unshift: function() {
            // Unshift direct child Components
            this.forEachComponent(function(component) {
                // Refer to comment in ComponentContainer.shift() above for explanation
                if (typeof component.unshift === "function") {
                    if (component instanceof Button
                        && (component.type === Button.prototype.types.push
                            || component.type === undefined)
                        && component.input === Button.prototype.input
                        && typeof component.inKey === "string"
                        && typeof component.group === "string") {
                        if (engine.getValue(component.group, component.inKey) !== 0) {
                            engine.setValue(component.group, component.inKey, 0);
                        }
                    }
                    component.unshift();
                }
            }, false);

            // Unshift child ComponentContainers
            this.forEachComponentContainer(function(container) {
                container.unshift();
            }, false);

            // Unset isShifted for each ComponentContainer recursively
            this.isShifted = false;
        },
        applyLayer: function(newLayer, reconnectComponents) {
            if (reconnectComponents !== false) {
                reconnectComponents = true;
            }
            if (reconnectComponents === true) {
                this.forEachComponent(function(component) {
                    component.disconnect();
                });
            }

            _.merge(this, newLayer);

            if (reconnectComponents === true) {
                this.forEachComponent(function(component) {
                    component.connect();
                    component.trigger();
                });
            }
        },
        shutdown: function() {
            this.forEachComponent(function(component) {
                if (component.shutdown !== undefined
                    && typeof component.shutdown === "function") {
                    component.shutdown();
                }
            });
        },
    };

    var Deck = function(deckNumbers) {
        if (deckNumbers !== undefined && Array.isArray(deckNumbers)) {
            // These must be unique to each instance,
            // so they cannot be in the prototype.
            this.deckNumbers = deckNumbers;
        } else if (deckNumbers !== undefined && typeof deckNumbers === "number" &&
                Math.floor(deckNumbers) === deckNumbers &&
                isFinite(deckNumbers)) {
            this.deckNumbers = [deckNumbers];
        } else {
            print("ERROR! new Deck() called without specifying any deck numbers");
            return;
        }
        this.currentDeck = "[Channel" + this.deckNumbers[0] + "]";
    };
    Deck.prototype = new ComponentContainer({
        setCurrentDeck: function(newGroup) {
            this.currentDeck = newGroup;
            this.reconnectComponents(function(component) {
                if (component.group === undefined
                      || component.group.search(script.channelRegEx) !== -1) {
                    component.group = newGroup;
                } else if (component.group.search(script.eqRegEx) !== -1) {
                    component.group = "[EqualizerRack1_" + newGroup + "_Effect1]";
                } else if (component.group.search(script.quickEffectRegEx) !== -1) {
                    component.group = "[QuickEffectRack1_" + newGroup + "]";
                }
                // Do not alter the Component's group if it does not match any of those RegExs.

                if (component instanceof EffectAssignmentButton) {
                    // The ControlObjects for assigning decks to effect units
                    // indicate the effect unit with the group and the deck with the key,
                    // so change the key here instead of the group.
                    component.inKey = "group_" + newGroup + "_enable";
                    component.outKey = "group_" + newGroup + "_enable";
                }
            });
        },
        toggle: function() {
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

    var EffectUnit = function(unitNumbers, allowFocusWhenParametersHidden, colors) {
        var eu = this;
        this.focusChooseModeActive = false;

        // This is only connected if allowFocusWhenParametersHidden is false.
        this.onShowParametersChange = function(value) {
            if (value === 0) {
                // Prevent this from getting called twice (on button down and button up)
                // when show_parameters button is clicked in skin.
                // Otherwise this.previouslyFocusedEffect would always be set to 0
                // on the second call.
                if (engine.getValue(eu.group, "show_focus") > 0) {
                    engine.setValue(eu.group, "show_focus", 0);
                    eu.previouslyFocusedEffect = engine.getValue(eu.group,
                        "focused_effect");
                    engine.setValue(eu.group, "focused_effect", 0);
                }
            } else {
                engine.setValue(eu.group, "show_focus", 1);
                if (eu.previouslyFocusedEffect !== undefined) {
                    engine.setValue(eu.group, "focused_effect",
                        eu.previouslyFocusedEffect);
                }
            }
            if (eu.enableButtons !== undefined) {
                eu.enableButtons.reconnectComponents(function(button) {
                    button.stopEffectFocusChooseMode();
                });
            }
        };

        this.setCurrentUnit = function(newNumber) {
            this.currentUnitNumber = newNumber;
            this.group = "[EffectRack1_EffectUnit" + newNumber + "]";

            if (allowFocusWhenParametersHidden) {
                engine.setValue(this.group, "show_focus", 0);
            } else {
                if (this.showParametersConnection !== undefined) {
                    this.showParametersConnection.disconnect();
                }
                delete this.previouslyFocusedEffect;
            }
            engine.setValue(this.group, "controller_input_active", 0);

            if (allowFocusWhenParametersHidden) {
                engine.setValue(this.group, "show_focus", 1);
            } else {
                // Connect a callback to show_parameters changing instead of
                // setting show_focus when effectFocusButton is pressed so
                // show_focus is always in the correct state, even if the user
                // presses the skin button for show_parameters.
                this.showParametersConnection = engine.makeConnection(this.group,
                    "show_parameters",
                    this.onShowParametersChange);
                this.showParametersConnection.trigger();
            }
            engine.setValue(this.group, "controller_input_active", 1);

            // Do not enable soft takeover upon EffectUnit construction
            // so initial values can be loaded from knobs.
            if (this.hasInitialized === true) {
                for (var n = 1; n <= 3; n++) {
                    var effect = "[EffectRack1_EffectUnit" + this.currentUnitNumber +
                                "_Effect" + n + "]";
                    engine.softTakeover(effect, "meta", true);
                    engine.softTakeover(effect, "parameter1", true);
                    engine.softTakeover(effect, "parameter2", true);
                    engine.softTakeover(effect, "parameter3", true);
                }
            }

            this.reconnectComponents(function(component) {
                // update [EffectRack1_EffectUnitX] groups
                var unitMatch = component.group.match(script.effectUnitRegEx);
                if (unitMatch !== null) {
                    component.group = eu.group;
                } else {
                    // update [EffectRack1_EffectUnitX_EffectY] groups
                    var effectMatch = component.group.match(script.individualEffectRegEx);
                    if (effectMatch !== null) {
                        component.group = "[EffectRack1_EffectUnit" +
                                          eu.currentUnitNumber +
                                          "_Effect" + effectMatch[2] + "]";
                    }
                }
            });
        };

        this.toggle = function() {
            // cycle through unitNumbers array
            var index = this.unitNumbers.indexOf(this.currentUnitNumber);
            if (index === (this.unitNumbers.length - 1)) {
                index = 0;
            } else {
                index += 1;
            }
            this.setCurrentUnit(this.unitNumbers[index]);
        };

        if (unitNumbers !== undefined && Array.isArray(unitNumbers)) {
            this.unitNumbers = unitNumbers;
        } else if (unitNumbers !== undefined && typeof unitNumbers === "number" &&
                  Math.floor(unitNumbers) === unitNumbers &&
                  isFinite(unitNumbers)) {
            this.unitNumbers = [unitNumbers];
        } else {
            print("ERROR! new EffectUnit() called without specifying any unit numbers!");
            return;
        }

        this.group = "[EffectRack1_EffectUnit" + this.unitNumbers[0] + "]";
        this.setCurrentUnit(this.unitNumbers[0]);

        this.dryWetKnob = new Pot({
            group: this.group,
            unshift: function() {
                this.inKey = "mix";
                // for soft takeover
                this.disconnect();
                this.connect();
            },
            shift: function() {
                this.inKey = "super1";
                // for soft takeover
                this.disconnect();
                this.connect();
                // engine.softTakeoverIgnoreNextValue is called
                // in the knobs' onFocusChange function
                eu.knobs.forEachComponent(function(knob) {
                    knob.trigger();
                });
            },
            outConnect: false,
        });

        this.enableOnChannelButtons = new ComponentContainer();
        this.enableOnChannelButtons.addButton = function(channel) {
            this[channel] = new Button({
                group: eu.group,
                key: "group_[" + channel + "]_enable",
                type: Button.prototype.types.toggle,
                outConnect: false,
            });
        };

        this.EffectUnitKnob = function(number) {
            this.number = number;
            Pot.call(this);
        };
        this.EffectUnitKnob.prototype = new Pot({
            group: this.group,
            number: this.currentUnitNumber,
            unshift: function() {
                this.input = function(channel, control, value, _status, _group) {
                    if (this.MSB !== undefined) {
                        value = (this.MSB << 7) + value;
                    }
                    this.inSetParameter(this.inValueScale(value));

                    if (this.previousValueReceived === undefined) {
                        var effect = "[EffectRack1_EffectUnit" + eu.currentUnitNumber +
                                    "_Effect" + this.number + "]";
                        engine.softTakeover(effect, "meta", true);
                        engine.softTakeover(effect, "parameter1", true);
                        engine.softTakeover(effect, "parameter2", true);
                        engine.softTakeover(effect, "parameter3", true);
                    }
                    this.previousValueReceived = value;
                };
            },
            shift: function() {
                engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
                this.valueAtLastEffectSwitch = this.previousValueReceived;
                // Floor the threshold to ensure that every effect can be selected
                this.changeThreshold = Math.floor(this.max /
                    engine.getValue("[Master]", "num_effectsavailable"));

                this.input = function(channel, control, value, _status, _group) {
                    if (this.MSB !== undefined) {
                        value = (this.MSB << 7) + value;
                    }
                    var change = value - this.valueAtLastEffectSwitch;
                    if (Math.abs(change) >= this.changeThreshold
                        // this.valueAtLastEffectSwitch can be undefined if
                        // shift was pressed before the first MIDI value was received.
                        || this.valueAtLastEffectSwitch === undefined) {
                        var effectGroup = "[EffectRack1_EffectUnit" +
                                           eu.currentUnitNumber + "_Effect" +
                                           this.number + "]";
                        engine.setValue(effectGroup, "effect_selector", change);
                        this.valueAtLastEffectSwitch = value;
                    }

                    this.previousValueReceived = value;
                };
            },
            outKey: "focused_effect",
            connect: function() {
                this.connections[0] = engine.makeConnection(eu.group, "focused_effect",
                    this.onFocusChange);
            },
            disconnect: function() {
                engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
                this.connections[0].disconnect();
            },
            trigger: function() {
                this.connections[0].trigger();
            },
            onFocusChange: function(value, _group, _control) {
                if (value === 0) {
                    this.group = "[EffectRack1_EffectUnit" +
                                  eu.currentUnitNumber + "_Effect" +
                                  this.number + "]";
                    this.inKey = "meta";
                } else {
                    this.group = "[EffectRack1_EffectUnit" + eu.currentUnitNumber +
                                  "_Effect" + value + "]";
                    this.inKey = "parameter" + this.number;
                }
                engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
            },
        });

        this.EffectEnableButton = function(number) {
            this.number = number;
            this.group = "[EffectRack1_EffectUnit" + eu.currentUnitNumber +
                          "_Effect" + this.number + "]";
            Button.call(this);
        };
        this.EffectEnableButton.prototype = new Button({
            type: Button.prototype.types.powerWindow,
            // NOTE: This function is only connected when not in focus choosing mode.
            onFocusChange: function(value, _group, _control) {
                if (value === 0) {
                    if (colors !== undefined) {
                        this.color = colors.unfocused;
                    }
                    this.group = "[EffectRack1_EffectUnit" +
                                  eu.currentUnitNumber + "_Effect" +
                                  this.number + "]";
                    this.inKey = "enabled";
                    this.outKey = "enabled";
                } else {
                    if (colors !== undefined) {
                        this.color = colors.focused;
                    }
                    this.group = "[EffectRack1_EffectUnit" + eu.currentUnitNumber +
                                 "_Effect" + value + "]";
                    this.inKey = "button_parameter" + this.number;
                    this.outKey = "button_parameter" + this.number;
                }
            },
            stopEffectFocusChooseMode: function() {
                this.type = Button.prototype.types.powerWindow;
                this.input = Button.prototype.input;
                this.output = Button.prototype.output;
                if (colors !== undefined) {
                    this.color = colors.unfocused;
                }

                this.connect = function() {
                    this.connections[0] = engine.makeConnection(eu.group, "focused_effect",
                        this.onFocusChange);
                    // this.onFocusChange sets this.group and this.outKey, so trigger it
                    // before making the connection for LED output
                    this.connections[0].trigger();
                    this.connections[1] = engine.makeConnection(this.group, this.outKey, this.output);
                };

                this.unshift = function() {
                    this.disconnect();
                    this.connect();
                    this.trigger();
                };
                this.shift = function() {
                    this.group = "[EffectRack1_EffectUnit" +
                                  eu.currentUnitNumber + "_Effect" +
                                  this.number + "]";
                    this.inKey = "enabled";
                };
                if (this.isShifted) {
                    this.shift();
                }
            },
            startEffectFocusChooseMode: function() {
                if (colors !== undefined) {
                    this.color = colors.focusChooseMode;
                }
                this.input = function(channel, control, value, status, _group) {
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
                this.output = function(value, _group, _control) {
                    this.send((value === this.number) ? this.on : this.off);
                };
                this.connect = function() {
                    // Outside of focus choose mode, the this.connections array
                    // has two members. Connections can be triggered when they
                    // are disconnected, so overwrite the whole array here instead
                    // of assigning to this.connections[0] to avoid
                    // Component.prototype.trigger() triggering the disconnected connection.
                    this.connections = [engine.makeConnection(eu.group,
                        "focused_effect",
                        this.output)];
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
            longPressTimer: 0,
            pressedWhenParametersHidden: false,
            previouslyFocusedEffect: 0,
            startEffectFocusChooseMode: function() {
                if (colors !== undefined) {
                    this.color = colors.focusChooseMode;
                }
                this.send(this.on);
                eu.focusChooseModeActive = true;
                eu.enableButtons.reconnectComponents(function(button) {
                    button.startEffectFocusChooseMode();
                });
            },
            setColor: function() {
                if (colors !== undefined) {
                    if (engine.getValue(this.group, "focused_effect") === 0) {
                        this.color = colors.unfocused;
                    } else {
                        this.color = colors.focused;
                    }
                }
            },
            unshift: function() {
                this.input = function(channel, control, value, status, _group) {
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

                        if (eu.focusChooseModeActive) {
                            this.setColor();
                            this.trigger();
                            eu.enableButtons.reconnectComponents(function(button) {
                                button.stopEffectFocusChooseMode();
                            });
                            eu.focusChooseModeActive = false;
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
            shift: function() {
                this.input = function(channel, control, value, status, _group) {
                    if (this.isPress(channel, control, value, status)) {
                        eu.toggle();
                    }
                };
            },
            outKey: "focused_effect",
            output: function(value, _group, _control) {
                this.send((value > 0) ? this.on : this.off);
            },
            outConnect: false,
        });
        this.effectFocusButton.setColor();

        this.init = function() {
            this.knobs.reconnectComponents();
            this.enableButtons.reconnectComponents(function(button) {
                button.stopEffectFocusChooseMode();
            });
            this.effectFocusButton.connect();
            this.effectFocusButton.trigger();

            this.enableOnChannelButtons.forEachComponent(function(button) {
                if (button.midi !== undefined) {
                    button.disconnect();
                    button.connect();
                    button.trigger();
                }
            });

            this.forEachComponent(function(component) {
                if (component.group === undefined) {
                    component.group = eu.group;
                }
            });

            this.hasInitialized = true;
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
