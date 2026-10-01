"use strict";
/**
 * Components JS library for Mixxx
 * Documentation is on the Mixxx wiki at
 * https://github.com/mixxxdj/mixxx/wiki/Components-JS
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
    const Component = class {
        constructor(options) {
            Object.assign(this, options);
            // These cannot be in the prototype; they must be unique to each instance.
            this.connections = [];
            this.isShifted = false;

            if (options !== undefined && typeof options.key === "string") {
                this.inKey = options.key;
                this.outKey = options.key;
            }

            if (typeof this.unshift === "function") {
                this.unshift();
            }

            if (this.outConnect !== false && this.group !== undefined && this.outKey !== undefined) {
                this.connect();
                if (this.outTrigger !== false) {
                    this.trigger();
                }
            }
        }
        // default methods
        // You should probably overwrite at least some of these.
        inValueScale(value) {
            // Hack to get exact center of pots to return 0.5
            if (value > (this.max / 2)) {
                return (value - 1) / (this.max - 1);
            } else {
                return value / (this.max + 1);
            }
        }
        // map input in the XML file, not inValueScale
        input(channel, control, value, _status, _group) {
            this.inSetParameter(this.inValueScale(value));
        }
        outValueScale(value) { return value * this.max; }
        output(value, _group, _control) {
            this.send(this.outValueScale(value));
        }

        // common functions
        // In most cases, you should not overwrite these.
        inGetParameter() {
            return engine.getParameter(this.group, this.inKey);
        }
        inSetParameter(value) {
            engine.setParameter(this.group, this.inKey, value);
        }
        inGetValue() {
            return engine.getValue(this.group, this.inKey);
        }
        inSetValue(value) {
            engine.setValue(this.group, this.inKey, value);
        }
        inToggle() {
            this.inSetValue(!this.inGetValue());
        }

        outGetParameter() {
            return engine.getParameter(this.group, this.outKey);
        }
        outSetParameter(value) {
            engine.setParameter(this.group, this.outKey, value);
        }
        outGetValue() {
            return engine.getValue(this.group, this.outKey);
        }
        outSetValue(value) {
            engine.setValue(this.group, this.outKey, value);
        }
        outToggle() {
            this.outSetValue(!this.outGetValue());
        }

        connect() {
            /**
            Override this method with a custom one to connect multiple Mixxx COs for a single Component.
            Add the connection objects to the this.connections array so they all get disconnected just
            by calling this.disconnect(). This can be helpful for multicolor LEDs that show a
            different color depending on the state of different Mixxx COs. Refer to
            SamplerButton.connect() and SamplerButton.output() for an example.
            **/
            if (this.group !== undefined &&
                this.outKey !== undefined &&
                this.output !== undefined &&
                typeof this.output === "function") {
                this.connections[0] = engine.makeConnection(this.group, this.outKey, this.output.bind(this));
            }
        }
        disconnect() {
            for (const conn of this.connections) {
                conn.disconnect();
            }
        }
        trigger() {

            for (const conn of this.connections) {
                conn.trigger();
            }
        }
        send(value) {
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
        }
    };
    // if true, output values are sent not only to the designated midi channel and control, but also sent to associated midi controls.
    Component.prototype.max = 127;
    Component.prototype.shiftOffset = 0;
    // only if `sendShifted` is true, will `sendShifted` or `shiftControl` be respected. See Component#send() code above.
    Component.prototype.sendShifted = false;
    // if `shiftChannel` is true, output values will be sent to the regular midi channel as well as the channel offset by `shiftOffset`.
    Component.prototype.shiftChannel = false;
    // if `shiftControl` is true, output values will be sent to the regular midi control as well as the control offset by `shiftOffset`.
    Component.prototype.shiftControl = false;

    const Button = class extends Component {
        constructor(options) {
            if (options.type === undefined) {
                options.type = Button.types.push;
            }
            super(options);
        }
        isPress(channel, control, value, _status) {
            return value > 0;
        }
        input(channel, control, value, status, _group) {
            switch (this.type) {
            case undefined:
            case Button.types.push:
                this.inSetValue(this.isPress(channel, control, value, status));
                break;
            case Button.types.toggle:
                if (this.isPress(channel, control, value, status)) {
                    this.inToggle();
                }
                break;
            case Button.types.powerWindow:
                if (this.isPress(channel, control, value, status)) {
                    this.inToggle();
                    this.isLongPressed = false;
                    this.longPressTimer = engine.beginTimer(this.longPressTimeout, () => {
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
                break;
            default:
                console.warn(`invalid button type: ${this.type}`);
                console.warn("make sure to only use components.Button.types.*");
                break;
            }
        }
        outValueScale(value) {
            return (value > 0) ? this.on : this.off;
        }
        shutdown() {
            this.send(this.off);
        }
    };
    Button.types = Object.freeze({
        push: 0,
        toggle: 1,
        powerWindow: 2,
    });
    Button.prototype.type = Button.types.push;
    // Time in milliseconds to distinguish a short press from a long press.
    // It is recommended to refer to it (as this.longPressTimeout)
    // in any Buttons that act differently with short and long presses
    // to keep the timeouts uniform.
    Button.prototype.longPressTimeout = 275;
    Button.prototype.on = 127;
    Button.prototype.off = 0;

    const PlayButton = class extends Button {
        constructor(options) {
            options.type = Button.types.toggle;
            options.outKey = "play_indicator";
            super(options);
        }
        unshift() {
            this.inKey = "play";
        }
        shift() {
            this.inKey = "reverse";
        }
    };

    const CueButton = class extends Button {
        constructor(options) {
            options.outKey = "cue_indicator";
            super(options);
        }
        unshift() {
            this.inKey = "cue_default";
        }
        shift() {
            if (this.reverseRollOnShift) {
                this.inKey = "reverseroll";
            } else {
                this.inKey = "start_stop";
            }
        }
    };

    const SyncButton = class extends Button {
        constructor(options) {
            options.outKey = "sync_enabled";
            super(options);
        }
        unshift() {
            this.input = function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status)) {
                    if (engine.getValue(this.group, "sync_enabled") === 0) {
                        engine.setValue(this.group, "beatsync", 1);
                        this.longPressTimer = engine.beginTimer(this.longPressTimeout, () => {
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
        }
        shift() {
            this.inKey = "quantize";
            this.type = Button.types.toggle;
            this.input = Button.prototype.input;
        }
    };

    const LoopToggleButton = class extends Button {
        constructor(options) {
            options.inKey = "reloop_exit";
            options.outKey = "loop_enabled";
            super(options);
        }
        inValueScale() {
            return 1;
        }
    };

    const HotcueButton = class extends Button {
        constructor(options) {
            if (options.number === undefined) {
                throw "No hotcue number specified for HotcueButton constructor";
            }
            if (options.colorMapper !== undefined || options.sendRGB !== undefined) {
                options.colorKey = "hotcue_" + options.number + "_color";
            }
            options.outKey = "hotcue_" + options.number + "_enabled";
            super(options);
        }
        unshift() {
            this.inKey = "hotcue_" + this.number + "_activate";
        }
        shift() {
            this.inKey = "hotcue_" + this.number + "_clear";
        }
        output(value) {
            const outval = this.outValueScale(value);
            // NOTE: _outputColor only handles hotcueColors
            // and there is no hotcueColor for turning the LED
            // off. So the `send()` function is responsible for turning the
            // actual LED off.
            if (this.colorKey !== undefined && outval !== this.off) {
                this._outputColor(engine.getValue(this.group, this.colorKey));
            } else {
                this.send(outval);
            }
        }
        _outputColor(colorCode) {
            // Sends the color from the colorCode to the controller. This
            // method will not be called if no colorKey has been specified.
            if (colorCode === undefined || colorCode < 0 || colorCode > 0xFFFFFF) {
                console.warn("Ignoring invalid color code '" + colorCode + "' in _outputColor()");
                return;
            }

            if (this.colorMapper !== undefined) {
                // This HotcueButton holds a reference to a ColorMapper. This means
                // that the controller only supports a fixed set of colors, so we
                // get the MIDI value for the nearest supported color and send it.
                const nearestColorValue = this.colorMapper.getValueForNearestColor(colorCode);
                this.send(nearestColorValue);
            } else {
                // Since _outputColor has been called but no ColorMapper is
                // available, we can assume that controller supports arbitrary
                // RGB color output.
                this.sendRGB(colorCodeToObject(colorCode));
            }
        }
        sendRGB(_colorObject) {
            // This method needs to be overridden in controller mappings,
            // because the procedure is controller-dependent.
            throw "sendRGB(colorObject) not overridden -- unable to send RGB colors. \
            Controller scripts must override this function with the implementation specific to that controller.";
        }
        connect() {
            super.connect();
            if (this.group !== undefined && this.colorKey !== undefined) {
                // TODO (https://bugreports.qt.io/browse/QTBUG-95677): replace with arrow function once this QT bug is fixed.
                this.connections[1] = engine.makeConnection(this.group, this.colorKey, function(color) {
                    if (engine.getValue(this.group, this.outKey)) {
                        this._outputColor(color);
                    }
                }.bind(this));
            }
        }
    };

    const SamplerButton = class extends Button {
        constructor(options) {
            if (options.number === undefined) {
                throw "No sampler number specified for SamplerButton constructor";
            }
            options.group = "[Sampler" + options.number + "]";
            options.outKey = null; // hack to get Component constructor to call connect()
            super(options);
        }
        unshift() {
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
        }
        shift() {
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
        }
        output(_value, _group, _control) {
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
        }
        connect() {
            this.connections[0] = engine.makeConnection(this.group, "track_loaded", this.output.bind(this));
            if (this.playing !== undefined) {
                this.connections[1] = engine.makeConnection(this.group, "play", this.output.bind(this));
            }
            if (this.looping !== undefined) {
                this.connections[2] = engine.makeConnection(this.group, "repeat", this.output.bind(this));
            }
        }
    };

    const EffectAssignmentButton = class extends Button {
        constructor(options) {
            options.key = "group_" + options.group + "_enable";
            options.group = "[EffectRack1_EffectUnit" + options.effectUnit + "]";
            options.type = Button.types.toggle;
            super(options);
        }
    };

    const Pot = class Pot extends Component {
        constructor(options) {
            super(options);
            this.firstValueReceived = false;
        }
        input(channel, control, value, _status, _group) {
            if (this.MSB !== undefined) {
                value = (this.MSB << 7) + value;
            }
            const newValue = this.inValueScale(value);
            this.inSetParameter(newValue);
            if (!this.firstValueReceived) {
                this.firstValueReceived = true;
                this.connect();
            }
        }
        // Input handlers for 14 bit MIDI
        inputMSB(channel, control, value, status, group) {
            // For the first messages, disregard the LSB in case
            // the first LSB is received after the first MSB.
            if (this.MSB === undefined) {
                this.max = 127;
                this.input(channel, control, value, status, group);
                this.max = 16383;
            }
            this.MSB = value;
        }
        inputLSB(channel, control, value, status, group) {
            // Make sure the first MSB has been received
            if (this.MSB !== undefined) {
                this.input(channel, control, value, status, group);
            }
        }
        connect() {
            if (this.firstValueReceived && !this.relative) {
                engine.softTakeover(this.group, this.inKey, true);
            }
        }
        disconnect() {
            if (!this.relative) {
                engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
            }
        }
        trigger() {}
    };

    /**
    The generic Component code provides everything to implement an Encoder. This Encoder Component
    exists so instanceof can be used to separate Encoders from other Components.
    **/
    const Encoder = Component;

    const ComponentContainer = class {
        constructor(initialLayer) {
            Object.assign(this, initialLayer);
            this.isShifted = false;
        }
        forEachComponent(operation, recursive) {
            if (typeof operation !== "function") {
                throw "ComponentContainer.forEachComponent requires a function argument";
            }
            if (recursive === undefined) { recursive = true; }

            const that = this;
            const applyOperationTo = function(obj) {
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

            for (const memberName in this) {
                if (ComponentContainer.prototype.hasOwnProperty.call(this, memberName)) {
                    applyOperationTo(this[memberName]);
                }
            }
        }
        forEachComponentContainer(operation, recursive) {
            if (typeof operation !== "function") {
                throw "ComponentContainer.forEachComponentContainer requires a function argument";
            }
            if (recursive === undefined) { recursive = true; }

            const that = this;
            const applyOperationTo = function(obj) {
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

            for (const memberName in this) {
                if (ComponentContainer.prototype.hasOwnProperty.call(this, memberName)) {
                    applyOperationTo(this[memberName]);
                }
            }
        }
        reconnectComponents(operation, recursive) {
            this.forEachComponent(function(component) {
                component.disconnect();
                if (typeof operation === "function") {
                    operation.call(this, component);
                }
                component.connect();
                component.trigger();
            }, recursive);
        }
        shift() {
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
                        && (component.type === Button.types.push
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
        }
        unshift() {
            // Unshift direct child Components
            this.forEachComponent(function(component) {
                // Refer to comment in ComponentContainer.shift() above for explanation
                if (typeof component.unshift === "function") {
                    if (component instanceof Button
                        && (component.type === Button.types.push
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
        }
        shutdown() {
            this.forEachComponent(function(component) {
                if (component.shutdown !== undefined
                    && typeof component.shutdown === "function") {
                    component.shutdown();
                }
            });
        }
    };

    const Deck = class extends ComponentContainer {
        constructor(deckNumbers) {
            super();
            if (deckNumbers !== undefined) {
                if (Array.isArray(deckNumbers)) {
                    // These must be unique to each instance,
                    // so they cannot be in the prototype.
                    this.currentDeck = "[Channel" + deckNumbers[0] + "]";
                    this.deckNumbers = deckNumbers;
                } else if (typeof deckNumbers === "number" &&
                        Math.floor(deckNumbers) === deckNumbers &&
                        isFinite(deckNumbers)) {
                    this.currentDeck = "[Channel" + deckNumbers + "]";
                    this.deckNumbers = [deckNumbers];
                }
            } else {
                throw "new Deck() called without specifying any deck numbers";
            }
        }
        setCurrentDeck(newGroup) {
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
        }
        toggle() {
            // cycle through deckNumbers array
            const oldIndex = this.deckNumbers.indexOf(parseInt(
                script.channelRegEx.exec(this.currentDeck)[1]
            ));
            const newIndex = (oldIndex + 1) % this.deckNumbers.length;
            this.setCurrentDeck("[Channel" + this.deckNumbers[newIndex] + "]");
        }
    };

    // TODO: pull this out into separate library once we support ES6 modules
    const EffectUnit = class extends ComponentContainer {
        constructor(unitNumbers, allowFocusWhenParametersHidden, colors) {
            super();
            const eu = this;
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
                if (allowFocusWhenParametersHidden) {
                    engine.setValue(this.group, "show_focus", 0);
                } else {
                    if (this.showParametersConnection !== undefined) {
                        this.showParametersConnection.disconnect();
                    }
                    delete this.previouslyFocusedEffect;
                }
                engine.setValue(this.group, "controller_input_active", 0);

                this.group = "[EffectRack1_EffectUnit" + newNumber + "]";

                if (allowFocusWhenParametersHidden) {
                    engine.setValue(this.group, "show_focus", 1);
                } else {
                    // Connect a callback to show_parameters changing instead of
                    // setting show_focus when effectFocusButton is pressed so
                    // show_focus is always in the correct state, even if the user
                    // presses the skin button for show_parameters.
                    this.showParametersConnection = engine.makeConnection(this.group,
                        "show_parameters",
                        this.onShowParametersChange.bind(this));
                    this.showParametersConnection.trigger();
                }
                engine.setValue(this.group, "controller_input_active", 1);

                // Do not enable soft takeover upon EffectUnit construction
                // so initial values can be loaded from knobs.
                if (this.hasInitialized === true) {
                    for (let n = 1; n <= 3; n++) {
                        const effect = "[EffectRack1_EffectUnit" + this.currentUnitNumber +
                                    "_Effect" + n + "]";
                        engine.softTakeover(effect, "meta", true);
                        engine.softTakeover(effect, "parameter1", true);
                        engine.softTakeover(effect, "parameter2", true);
                        engine.softTakeover(effect, "parameter3", true);
                    }
                }

                this.reconnectComponents(function(component) {
                    // update [EffectRack1_EffectUnitX] groups
                    const unitMatch = component.group.match(script.effectUnitRegEx);
                    if (unitMatch !== null) {
                        component.group = eu.group;
                    } else {
                        // update [EffectRack1_EffectUnitX_EffectY] groups
                        const effectMatch = component.group.match(script.individualEffectRegEx);
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
                const oldIndex = this.unitNumbers.indexOf(this.currentUnitNumber);
                const newIndex = (oldIndex + 1) % this.deckNumbers.length;
                this.setCurrentUnit(this.unitNumbers[newIndex]);
            };

            if (unitNumbers !== undefined) {
                if (Array.isArray(unitNumbers)) {
                    this.unitNumbers = unitNumbers;
                    this.setCurrentUnit(unitNumbers[0]);
                } else if (typeof unitNumbers === "number" &&
                        Math.floor(unitNumbers) === unitNumbers &&
                        isFinite(unitNumbers)) {
                    this.unitNumbers = [unitNumbers];
                    this.setCurrentUnit(unitNumbers);
                }
            } else {
                throw "new EffectUnit() called without specifying any unit numbers!";
            }

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
                    type: Button.types.toggle,
                    outConnect: false,
                });
            };

            this.EffectUnitKnob = class extends Pot {
                constructor(number) {
                    super();
                    this.number = number;
                    this.group = eu.group;
                    this.outKey = "focused_effect";
                }
                unshift() {
                    this.input = function(channel, control, value, _status, _group) {
                        if (this.MSB !== undefined) {
                            value = (this.MSB << 7) + value;
                        }
                        this.inSetParameter(this.inValueScale(value));

                        if (this.previousValueReceived === undefined) {
                            const effect = "[EffectRack1_EffectUnit" + eu.currentUnitNumber +
                                        "_Effect" + this.number + "]";
                            engine.softTakeover(effect, "meta", true);
                            engine.softTakeover(effect, "parameter1", true);
                            engine.softTakeover(effect, "parameter2", true);
                            engine.softTakeover(effect, "parameter3", true);
                        }
                        this.previousValueReceived = value;
                    };
                }
                shift() {
                    engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
                    this.valueAtLastEffectSwitch = this.previousValueReceived;
                    // Floor the threshold to ensure that every effect can be selected
                    this.changeThreshold = Math.floor(this.max /
                        engine.getValue("[Master]", "num_effectsavailable"));

                    this.input = function(channel, control, value, _status, _group) {
                        if (this.MSB !== undefined) {
                            value = (this.MSB << 7) + value;
                        }
                        const change = value - this.valueAtLastEffectSwitch;
                        if (Math.abs(change) >= this.changeThreshold
                            // this.valueAtLastEffectSwitch can be undefined if
                            // shift was pressed before the first MIDI value was received.
                            || this.valueAtLastEffectSwitch === undefined) {
                            const effectGroup = "[EffectRack1_EffectUnit" +
                                            eu.currentUnitNumber + "_Effect" +
                                            this.number + "]";
                            engine.setValue(effectGroup, "effect_selector", change);
                            this.valueAtLastEffectSwitch = value;
                        }

                        this.previousValueReceived = value;
                    };
                }
                connect() {
                    this.connections[0] = engine.makeConnection(eu.group, "focused_effect",
                        this.onFocusChange.bind(this));
                }
                disconnect() {
                    engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
                    if (this.connections[0] !== undefined) {
                        this.connections[0].disconnect();
                    }
                }
                trigger() {
                    this.connections[0].trigger();
                }
                onFocusChange(value, _group, _control) {
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
                }
            };

            this.EffectEnableButton = class extends Button {
                constructor(number) {
                    const options = {
                        number: number,
                        group: "[EffectRack1_EffectUnit" + eu.currentUnitNumber +
                                "_Effect" + number + "]",
                        type: Button.types.powerWindow,
                    };
                    super(options);
                }
                // NOTE: This function is only connected when not in focus choosing mode.
                onFocusChange(value, _group, _control) {
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
                }
                stopEffectFocusChooseMode() {
                    this.type = Button.types.powerWindow;
                    this.input = Button.prototype.input;
                    this.output = Button.prototype.output;
                    if (colors !== undefined) {
                        this.color = colors.unfocused;
                    }

                    this.connect = function() {
                        this.connections[0] = engine.makeConnection(eu.group, "focused_effect",
                            this.onFocusChange.bind(this));
                        // this.onFocusChange sets this.group and this.outKey, so trigger it
                        // before making the connection for LED output
                        this.connections[0].trigger();
                        this.connections[1] = engine.makeConnection(this.group, this.outKey, this.output.bind(this));
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
                }
                startEffectFocusChooseMode() {
                    if (colors !== undefined) {
                        this.color = colors.focusChooseMode;
                    }
                    this.input = function(channel, control, value, status, _group) {
                        if (!this.isPress(channel, control, value, status)) {
                            return;
                        }
                        if (engine.getValue(eu.group, "focused_effect") === this.number) {
                            // unfocus and make knobs control metaknobs
                            engine.setValue(eu.group, "focused_effect", 0);
                        } else {
                            // focus this effect
                            engine.setValue(eu.group, "focused_effect", this.number);
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
                            this.output.bind(this))];
                    };
                }
            };

            this.knobs = new ComponentContainer();
            this.enableButtons = new ComponentContainer();
            for (let n = 1; n <= 3; n++) {
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
                        const showParameters = engine.getValue(this.group, "show_parameters");
                        if (this.isPress(channel, control, value, status)) {
                            this.longPressTimer = engine.beginTimer(this.longPressTimeout,
                                this.startEffectFocusChooseMode.bind(this),
                                true);
                            if (showParameters) {
                                return;
                            }
                            if (!allowFocusWhenParametersHidden) {
                                engine.setValue(this.group, "show_parameters", 1);
                                // eu.onShowParametersChange will refocus the
                                // previously focused effect and show focus in skin
                            }
                            this.pressedWhenParametersHidden = true;
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
        }
    };

    const exports = {};
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
