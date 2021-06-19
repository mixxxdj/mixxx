/**
 * Additional JS components for Mixxx
 */
(function(global) {

    /** @private */
    var components = global.components;

    /**
     * Contains functions to print a message to the log.
     * `debug` output is suppressed unless the caller owns a truthy property `debug`.
     *
     * @param {string} message Message
     * @private
     */
    var log = {
        debug: function(message) {
            if (this.debug) {
                print("[DEBUG] " + message);
            }
        },
        warn: function(message) {
            print("[WARN]  " + message);
        },
        error: function(message) {
            print("[ERROR] " + message);
        },
    };

    /**
     * Determine an ID from a component's MIDI address
     *
     * @param {Array<number>} midiAddress MIDI address consisting of two integers
     * @return {string} ID for the MIDI address; `undefined` on error
     * @private
     */
    var findComponentId = function(midiAddress) {
        if (Array.isArray(midiAddress) && midiAddress.length === 2
                && typeof midiAddress[0] === "number"  && typeof midiAddress[1] === "number") {
            return "[" + midiAddress.map(function(x) {
                return "0x" + ("00" + x.toString(16).toUpperCase()).slice(-2);
            }).join(", ") + "]";
        }
        log.error("Invalid MIDI address: " + global.stringifyObject(midiAddress));
    };

    /**
     * Create a human-readable string to identify a component.
     *
     * @param {components.Component} component A component
     * @return {string} A short string that describes the component; `undefined` on error
     * @private
     */
    var stringifyComponent = function(component) {
        if (component === undefined) {
            return;
        }
        var id = findComponentId(component.midi);
        if (id !== undefined) {
            var key = component.inKey || component.outKey;
            return "(" + id + ": " + component.group + "," + key + ")";
        }
    };

    /**
     * Convert a parameter value (0..1) to a MIDI value (0..this.max).
     *
     * @param {number} value A number between 0 and 1.
     * @private
     */
    var convertToMidiValue = function(value) {
        /*
         * Math.round() is important to keep input and output in sync.
         * Example:
         * - Mixxx receives input value 35 and calculates input parameter (35 / 127) == 0.27559
         * - Mixxx sends output value of 0.27559
         * - Component.outScaleValue(0.27559) = 0.27559 * 127 = 34.99
         * - Without round, 34 is sent to the controller.
         */
        return Math.round(value * this.max);
    };

    /**
     * Derive a prototype from a parent.
     *
     * @param {object} parent Constructor of parent whose prototype is used as base
     * @param {object} members Own members that are not inherited
     * @return {object} A new prototype based on parent with the given members
     * @private
     */
    var deriveFrom = function(parent, members) {
        return _.merge(Object.create(parent.prototype), members || {});
    };

    /**
     * A button to toggle un-/shift on a target component.
     *
     * @constructor
     * @extends {components.Button}
     * @param {object} options Options object
     * @param {components.Component|components.ComponentContainer} options.target Target component
     * @public
     */
    var ShiftButton = function(options) {
        components.Button.call(this, options);
    };
    ShiftButton.prototype = deriveFrom(components.Button, {
        input: function(_channel, _control, value, _status, _group) {
            if (value) {
                this.target.shift();
            } else {
                this.target.unshift();
            }
        },
    });

    /**
     * A component that handles every incoming value.
     *
     * @constructor
     * @extends {components.Component}
     * @param {object} options Options object
     * @public
     */
    var Trigger = function(options) {
        components.Component.call(this, options);
    };
    Trigger.prototype = deriveFrom(components.Component, {
        inValueScale: function() { return true; },
    });

    /**
     * A button with configurable Mixxx control values for `on` and `off`.
     *
     * @constructor
     * @extends {components.Button}
     * @param {number} options.onValue Value for `on`; optional, default: `1`
     * @param {number} options.offValue Value for `off`; optional, default: opposite of `onValue`
     * @public
     */
    var CustomButton = function(options) {
        options = options || {};
        if (options.onValue === undefined) { // do not use '||' to allow 0
            options.onValue = 1;
        }
        if (options.offValue === undefined) { // do not use '!' to keep number type
            options.offValue = options.onValue ? 0 : 1;
        }
        components.Button.call(this, options);
    };
    CustomButton.prototype = deriveFrom(components.Button, {
        isOn: function(engineValue) { // engine -> boolean
            return engineValue === this.onValue;
        },
        inGetValue: function() { // engine -> boolean
            return this.isOn(engine.getValue(this.group, this.inKey));
        },
        inSetValue: function(on) { // boolean -> engine
            engine.setValue(this.group, this.inKey, on ? this.onValue : this.offValue);
        },
        outGetValue: function() { // engine -> boolean
            return this.isOn(engine.getValue(this.group, this.outKey));
        },
        outSetValue: function(on) { // boolean -> engine
            engine.setValue(this.group, this.outKey, on ? this.onValue : this.offValue);
        },
        outValueScale: function(engineValue) { // engine -> MIDI
            return this.isOn(engineValue) ? this.on : this.off;
        },
        inValueScale: function(midiValue) { // MIDI -> engine
            return midiValue === this.on ? this.onValue : this.offValue;
        }
    });

    /**
     * An object that simplifies using a timer safely.
     * Use `start(action)` to start and `reset()` to reset.
     *
     * @constructor
     * @param {number} options.timeout Duration between start and action
     * @param {boolean} options.oneShot If `true`, the action is run once;
     *                          otherwise, it is run periodically until the timer is reset.
     * @param {function} options.action Function that is executed whenever the timer expires
     * @param {object} options.owner Owner object of the `action` function (assigned to `this`)
     * @public
     * @see https://github.com/mixxxdj/mixxx/wiki/Script-Timers
     */
    var Timer = function(options) {
        _.assign(this, options);
        this.disable();
    };
    Timer.prototype = {
        disable: function() { this.id = 0; },
        isEnabled: function() { return this.id !== 0; },
        start: function() {
            this.reset();
            var timer = this;
            this.id = engine.beginTimer(this.timeout, function() {
                if (timer.oneShot) {
                    timer.disable();
                }
                timer.action.call(timer.owner);
            }, this.oneShot);
        },
        reset: function() {
            if (this.isEnabled()) {
                engine.stopTimer(this.id);
                this.disable();
            }
        },
        setState: function(active) {
            if (active) {
                this.start();
            } else {
                this.reset();
            }
        }
    };

    /**
     * A button that supports different actions on short and long press.
     *
     * @constructor
     * @extends {components.Button}
     * @param {object} options Options object
     * @public
     */
    var LongPressButton = function(options) {
        components.Button.call(this, options);
        var action = function() {
            this.isLongPressed = true;
            this.onLongPress();
        };
        this.longPressTimer = new Timer(
            {timeout: this.longPressTimeout, oneShot: true, action: action, owner: this});
    };
    LongPressButton.prototype = deriveFrom(components.Button, {
        isLongPressed: false,
        input: function(channel, control, value, status, _group) {
            if (this.isPress(channel, control, value, status)) {
                this.handlePress();
            } else {
                this.handleRelease();
            }
        },
        handlePress: function() {
            this.onShortPress();
            this.longPressTimer.start();
        },
        handleRelease: function() {
            this.longPressTimer.reset();
            this.onRelease();
            this.isLongPressed = false;
        },
        onShortPress: function(_value) {},
        onLongPress: function(_value)  {},
        onRelease: function(_value)  {},
    });

    /**
     * A button that blinks when `on`.
     *
     * @constructor
     * @extends {components.Button}
     * @param {number} options.blinkDuration Blink duration in ms; optional, default: 500
     * @public
     */
    var BlinkingButton = function(options) {
        options = options || {};
        var blinkAction = function() {
            this.send(components.Button.prototype.outValueScale.call(
                this, this.flashing = !this.flashing));
        };
        this.blinkTimer = new Timer(
            {timeout: options.blinkDuration || 500, action: blinkAction, owner: this});
        components.Button.call(this, options);
    };
    BlinkingButton.prototype = deriveFrom(components.Button, {
        flashing: false,
        outValueScale: function(value) {
            this.blinkTimer.setState(this.flashing = value);
            return components.Button.prototype.outValueScale.call(this, value);
        },
    });

    /**
     * An encoder for directions.
     *
     * Turning the encoder to the right means "forwards" and returns 1;
     * turning it to the left means "backwards" and returns -1.
     *
     * @constructor
     * @extends {components.Encoder}
     * @param {object} options Options object
     * @public
     */
    var DirectionEncoder = function(options) {
        components.Encoder.call(this, options);
        this.previousValue = this.inGetValue(); // available only after call of Encoder constructor
    };
    DirectionEncoder.prototype = deriveFrom(components.Encoder, {
        min: 0,
        inValueScale: function(value) {
            var direction = 0;
            if (value > this.previousValue || value === this.max) {
                direction = 1;
            } else if (value < this.previousValue || value === this.min) {
                direction = -1;
            }
            this.previousValue = value;

            return direction;
        },
    });

    /**
     * An encoder for a value range of [-bound..0..+bound].
     *
     * @constructor
     * @extends {components.Encoder}
     * @param {object} options Options object
     * @param {number} options.bound A positive integer defining the range bounds
     * @public
     */
    var RangeAwareEncoder = function(options) {
        components.Encoder.call(this, options);
    };
    RangeAwareEncoder.prototype = deriveFrom(components.Encoder, {
        outValueScale: function(value) {
            /* -bound..+bound => 0..1 */
            var normalizedValue = (value + this.bound) / (2 * this.bound);
            /* 0..1 => 0..127 */
            return convertToMidiValue.call(this, normalizedValue);
        },
    });

    /**
     * A button to cycle through the values of an enumeration.
     *
     * The enumeration values may be defined either explicitly by an array
     * or implicitly by a `maxValue` so that the values are `[0..maxValue]`.
     *
     * @constructor
     * @extends {components.Button}
     * @param {object} options Options object
     * @param {Array} options.values An array of enumeration values
     * @param {number} options.maxValue A positive integer defining the maximum enumeration value
     * @public
     */
    var EnumToggleButton = function(options) {
        options = options || {};
        if (options.maxValue === undefined && options.values === undefined) {
            log.error("An EnumToggleButton requires either `values` or a `maxValue`.");
            this.maxValue = 0;
        }
        if (options.type === undefined) { // do not use '||' to allow 0
            options.type = components.Button.prototype.types.toggle;
        }
        components.Button.call(this, options);
    };
    EnumToggleButton.prototype = deriveFrom(components.Button, {
        input: function(channel, control, value, status, _group) {
            if (this.isPress(channel, control, value, status)) {
                var newValue;
                if (this.values) {
                    var index = this.values.indexOf(this.inGetValue());
                    newValue = this.values[(index + 1) % this.values.length];
                } else {
                    newValue = (this.inGetValue() + 1) % (this.maxValue + 1);
                }
                this.inSetValue(newValue);
            }
        }
    });

    /**
     * An encoder for enumeration values.
     *
     * @constructor
     * @extends {components.Encoder}
     * @param {object} options Options object
     * @param {Array} options.values An array containing the enumeration values
     * @public
     */
    var EnumEncoder = function(options) {
        options = options || {};
        if (options.values === undefined) {
            log.error("EnumEncoder constructor was called without specifying enum values.");
            options.values = [];
        }
        options.maxIndex = options.values.length - 1;
        components.Encoder.call(this, options);
    };
    EnumEncoder.prototype = deriveFrom(components.Encoder, {
        inValueScale: function(value) {
            var normalizedValue = value / this.max;
            var index = Math.round(normalizedValue * this.maxIndex);
            return this.values[index];
        },
        outValueScale: function(value) {
            var index = this.values.indexOf(value);
            if (index !== -1) {
                var normalizedValue = index / this.maxIndex;
                return convertToMidiValue.call(this, normalizedValue);
            } else {
                log.warn("'" + value + "' is not in supported values " + "[" + this.values + "]");
            }
        },
    });

    /**
     * An EnumEncoder for a loop control that uses beat sizes as enumeration.
     *
     * @constructor
     * @extends {EnumEncoder}
     * @param {object} options Options object
     * @public
     */
    var LoopEncoder = function(options) {
        options = options || {};
        if (options.values === undefined) {
            /* taken from src/engine/controls/loopingcontrol.cpp */
            options.values
                = [0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512];
        }
        EnumEncoder.call(this, options);
    };
    LoopEncoder.prototype = deriveFrom(EnumEncoder);

    /**
     * An encoder that moves a loop.
     *
     * Turning the encoder to the right will move the loop forwards; turning it to the left will
     * move it backwards. The amount of movement may be given by either `size` or `sizeControl`,
     * `sizeControl` being preferred.
     *
     * @constructor
     * @extends {components.Encoder}
     * @param {object} options Options object
     * @param {number} options.size (optional) Size given in number of beats; default: 0.5
     * @param {string} options.sizeControl (optional) Name of a control that contains `size`
     * @public
     */
    var LoopMoveEncoder = function(options) {
        options = options || {};
        options.inKey = options.inKey || "loop_move";
        options.size = options.size || 0.5;
        DirectionEncoder.call(this, options);
    };
    LoopMoveEncoder.prototype = deriveFrom(DirectionEncoder, {
        inValueScale: function(value) {
            var direction = DirectionEncoder.prototype.inValueScale.call(this, value);
            var beats = this.sizeControl
                ? global.engine.getValue(this.group, this.sizeControl)
                : this.size;
            return direction * beats;
        },
    });

    /**
     * A button that toggles a beatloop that ends at the current play position.
     *
     * @constructor
     * @extends {components.Button}
     * @param {object} options Options object
     * @public
     */
    var BackLoopButton = function(options) {
        options = options || {};
        options.outKey = options.outKey || "loop_enabled";
        components.Button.call(this, options);
    };
    BackLoopButton.prototype = deriveFrom(components.Button, {
        input: function(_channel, _control, value, _status, group) {
            var engine = global.engine;
            var script = global.script;
            if (value) {
                var loopSize = engine.getValue(group, "beatloop_size");
                var beatjumpSize = engine.getValue(group, "beatjump_size");
                engine.setValue(group, "beatjump_size", loopSize);
                script.triggerControl(group, "beatjump_backward");
                script.triggerControl(group, "beatloop_activate");
                engine.setValue(group, "beatjump_size", beatjumpSize);
            } else {
                script.triggerControl(group, "reloop_toggle");
            }
        }
    });

    /**
     * A pot for the crossfader curve.
     *
     * @constructor
     * @extends {components.Pot}
     * @param {number} options.mode Crossfader mode; optional, default: `0`.
     *                              (`0`: additive, `1`: constant)
     * @public
     */
    var CrossfaderCurvePot = function(options) {
        options = options || {};
        options.group = options.group || "[Mixer Profile]";
        if (options.mode) {
            options.inKey = options.inKey || options.key || "xFaderCalibration";
            options.low = options.low || 0.5;
            options.high = options.high || 0.962;
            engine.setValue("[Mixer Profile]", "xFaderMode", options.mode);
        } else {
            options.inKey = options.inKey || options.key || "xFaderCurve";
            options.low = options.low || 1;
            options.high = options.high || 2;
        }
        components.Pot.call(this, options);
    };
    CrossfaderCurvePot.prototype = deriveFrom(components.Pot, {
        modes: {ADDITIVE: 0, CONSTANT: 1},
        min: 0,
        input: function(_channel, _control, value, _status, _group) {
            engine.setValue(this.group, this.inKey,
                script.absoluteLin(value, this.low, this.high, this.min, this.max));
        }
    });

    /**
     * A component that sends the values of a source component to a MIDI controller even if the
     * source component uses its `outKey` property for other purposes.
     *
     * Note: most components send output properly out of the box so that no Publisher is required.
     * This component allows to add functionality to some special components, e.g. effect unit
     * controls.
     *
     * This component offers a `bind()` function that allows for re-binding to the source component
     * when its internal state changes.
     *
     * @constructor
     * @extends {components.Component}
     * @param {object} options Options object
     * @param {components.Component} options.source Source component whose values are sent to the
     *                                              controller
     * @public
     */
    var Publisher = function(options) {
        if (options.source === undefined) {
            log.error("Missing source component");
            return;
        }
        this.source = options.source;
        this.sync();
        components.Component.call(this, options);
    };
    Publisher.prototype = deriveFrom(components.Component, {
        outValueScale: function(_value) {
            /*
             * We ignore the argument and use the parameter (0..1) instead because value scale is
             * arbitrary and thus cannot be mapped to MIDI values (0..127) properly.
             */
            return convertToMidiValue.call(this, this.outGetParameter());
        },
        sync: function() {
            this.midi = this.source.midi;
            this.group = this.source.group;
            this.outKey = this.source.inKey;
        },
        bind: function() {
            if (this.group !== this.source.group || this.outKey !== this.source.inKey) {
                log.debug("Binding publisher " + stringifyComponent(this)
                    + " to " + stringifyComponent(this.source));
                this.disconnect();
                this.sync();
                this.connect();
                this.trigger();
            }
        },
    });

    /**
     * @typedef {components.ComponentContainer} EqualizerUnit
     *
     * @property {components.Button} enabled En-/disable equalizer unit
     * @property {components.Pot} super1 QuickEffect super knob
     * @property {components.Pot} parameterKnobs.1 Low knob
     * @property {components.Pot} parameterKnobs.2 Mid knob
     * @property {components.Pot} parameterKnobs.3 High knob
     * @property {components.Button} parameterButtons.1 Mute low
     * @property {components.Button} parameterButtons.2 Mute mid
     * @property {components.Button} parameterButtons.3 Mute high
     */

    /**
     * A component container for equalizer controls.
     *
     * @constructor
     * @extends {components.ComponentContainer}
     * @param {string} deckGroup Group of the deck this unit belongs to (e.g. `[Channel1]`)
     * @yields {EqualizerUnit}
     * @public
     */
    var EqualizerUnit = function(deckGroup) {
        components.ComponentContainer.call(this);
        var effectGroup = "[EqualizerRack1_" + deckGroup + "_Effect1]";

        var ParameterKnob = function(parameterNumber) {
            components.Pot.call(this, {group: effectGroup, key: "parameter" + parameterNumber});
        };
        ParameterKnob.prototype = deriveFrom(components.Pot);
        var ParameterButton = function(parameterNumber) {
            components.Button.call(this, {
                group: effectGroup, key: "button_parameter" + parameterNumber
            });
        };
        ParameterButton.prototype = deriveFrom(
            components.Button, {type: components.Button.prototype.types.powerWindow});

        this.enabled = new components.Button(
            {group: "[QuickEffectRack1_" + deckGroup + "_Effect1]", key: "enabled"});
        this.super1 = new components.Pot(
            {group: "[QuickEffectRack1_" + deckGroup + "]", key: "super1"});
        this.parameterKnobs = new components.ComponentContainer();
        this.parameterButtons = new components.ComponentContainer();
        for (var i = 1; i <= 3; i++) {
            this.parameterKnobs[i] = new ParameterKnob(i);
            this.parameterButtons[i] = new ParameterButton(i);
        }
    };
    EqualizerUnit.prototype = deriveFrom(components.ComponentContainer);

    /**
     * Manage Components in named ComponentContainers.
     *
     * @constructor
     * @param {Array<string>} initialContainers Initial container names
     * @public
     */
    var ComponentRegistry = function(initialContainers) {
        this.containers = new components.ComponentContainer();
        if (Array.isArray(initialContainers)) {
            initialContainers.forEach(function(name) { this.createContainer(name); }, this);
        }
    };
    ComponentRegistry.prototype = {

        /**
         * Create a new ComponentContainer within this registry.
         *
         * @param {string} name Name of the ComponentContainer
         * @return {components.ComponentContainer} The ComponentContainer; `undefined` on error
         * @public
         */
        createContainer: function(name) {
            if (!Object.prototype.hasOwnProperty.call(this.containers, name)) {
                log.debug("Creating ComponentContainer '" + name + "'");
                this.containers[name] = new components.ComponentContainer();
                return this.containers[name];
            } else {
                log.error("ComponentContainer '" + name + "' already exists.");
            }
        },

        /**
         * Retrieve an existing ComponentContainer.
         *
         * @param {string} name Name of an existing ComponentContainer
         * @return {components.ComponentContainer} The ComponentContainer; `undefined` on error
         * @public
         */
        getContainer: function(name) {
            if (!Object.prototype.hasOwnProperty.call(this.containers, name)) {
                log.error("ComponentContainer '" + name + "' does not exist.");
            }
            return this.containers[name];
        },

        /**
         * Disconnect all components in a ComponentContainer.
         *
         * @param {components.ComponentContainer} container An existing ComponentContainer
         * @public
         */
        disconnectContainer: function(container) {
            container.forEachComponent(function(component) {
                component.disconnect();
            });
        },

        /**
         * Remove a ComponentContainer, optionally disconnecting its components
         *
         * @param {string} name Name of an existing ComponentContainer
         * @param {boolean} disconnect Iff truthy, components are disconnected
         * @public
         */
        destroyContainer: function(name, disconnect) {
            log.debug("Destroying ComponentContainer '" + name + "'");
            if (disconnect) {
                this.disconnectContainer(this.getContainer(name));
            }
            delete this.containers[name];
        },

        /**
         * Remove all ComponentContainers, optionally disconnecting their components.
         *
         * @param {boolean} disconnect Iff truthy, components are disconnected
         * @public
         */
        destroy: function(disconnect) {
            Object.keys(this.containers).forEach(function(name) {
                this.destroyContainer(name, disconnect);
            }, this);
        },

        /**
         * Store a component in a ComponentContainer using the component's MIDI address as key.
         * If the ComponentContainer is missing, it is created. If the ComponentContainer contains
         * a component with the same key, the existing component is unregistered first.
         * Note: components are not connected or disconnected.
         *
         * @param {components.Component} component A component
         * @param {string} containerName Name of a ComponentContainer
         * @return {string} ID of the stored component; `undefined` on error
         * @public
         */
        register: function(component, containerName) {
            if (component === undefined) {
                log.error("Missing component");
                return;
            }
            var id = findComponentId(component.midi);
            if (!Object.prototype.hasOwnProperty.call(this.containers, containerName)) {
                this.createContainer(containerName);
            }
            var container = this.getContainer(containerName);
            var store = true;
            if (Object.prototype.hasOwnProperty.call(container, id)) {
                var old = container[id];
                if (old !== component) {
                    this.unregister(old, containerName);
                } else {
                    store = false;
                    log.debug(containerName + ": ignore re-register of "
                        + stringifyComponent(component));
                }
            }
            if (store) {
                log.debug(containerName + ": register " + stringifyComponent(component));
                container[id] = component;
            }
            return id;
        },

        /**
         * Remove a component from a ComponentContainer.
         * Note: components are not connected or disconnected.
         *
         * @param {components.Component} component A component
         * @param {string} containerName Name of an existing ComponentContainer
         * @return {string} ID of the removed component; `undefined` on error
         * @public
         */
        unregister: function(component, containerName) {
            log.debug(containerName + ": unregister " + stringifyComponent(component));
            var container = this.getContainer(containerName);
            var id = findComponentId(component.midi);
            delete container[id];
            return id;
        },
    };

    /**
     * A component that manages components in two layers named "Default" and "Shift".
     *
     * Components may be added to either layer. One of the layers has the role of the "active"
     * layer (initially: Default). The active layer is changed by calling the functions `shift()`
     * and `unshift()`.
     *
     * Once after addition of all components, `init()` must be called.
     *
     * This object is itself a component offering an `input()` function. This function delegates to
     * the component with the matching MIDI address on the active layer. When there's no matching
     * component, the Default layer is used as fallback.
     *
     * Note: when two components on different layers share the same MIDI address, they are
     * dis-/connected on layer switch. Components are not dis-/connected when they are
     * un-/registered to a layer.
     *
     * @constructor
     * @extends {components.Component}
     * @param {object} options Options object
     * @param {boolean} options.debug Optional flag to emit debug messages to the log
     * @public
     */
    var LayerManager = function(options) {
        this.componentRegistry = new ComponentRegistry([
            LayerManager.prototype.defaultContainerName,
            LayerManager.prototype.shiftContainerName]);
        this.activeLayer = new components.ComponentContainer();
        components.Component.call(this, options);
    };
    LayerManager.prototype = deriveFrom(components.Component, {
        /** @private */
        defaultContainerName: "Default",
        /** @private */
        shiftContainerName: "Shift",

        /**
         * Retrieve the Default layer.
         *
         * @return {object} The Default layer
         * @private
         */
        defaultLayer: function() {
            var defaultContainer = this.componentRegistry.getContainer(this.defaultContainerName);
            return Object.keys(this.shiftLayer()).reduce(
                function(shiftCounterparts, name) {
                    shiftCounterparts[name] = defaultContainer[name];
                    return shiftCounterparts;
                }, {});
        },

        /**
         * Retrieve the Shift layer.
         *
         * @return {object} The Shift layer
         * @private
         */
        shiftLayer: function() {
            return this.componentRegistry.getContainer(this.shiftContainerName);
        },

        /**
         * Retrieve a component from the active layer.
         *
         * @param {number} status First byte of the component's MIDI address
         * @param {number} control Second byte of the component's MIDI address
         * @return {components.Component} Component on the active layer matching the MIDI address;
         *                                undefined on error. When the active layer does not contain
         *                                a matching component, the Default layer is used as
         *                                fallback.
         * @private
         */
        findComponent: function(status, control) {
            var id = findComponentId([status, control]);
            if (id === undefined) {
                return;
            }
            var component = this.activeLayer[id];
            if (component === undefined) {
                var defaultComponents
                    = this.componentRegistry.getContainer(this.defaultContainerName);
                component = defaultComponents[id];
            }
            if (component === undefined) {
                log.error("Unregistered MIDI address: " + id);
            } else {
                log.debug("Find " + stringifyComponent(component));
            }
            return component;
        },

        /**
         * Run a registry operation for a component on a layer.
         *
         * @param {LayerManager~registryOperation} The operation to run
         * @param {components.Component} component A component
         * @param {boolean} shift Iff true, use Shift Layer, otherwise use Default Layer
         * @return Result of the operation
         * @private
         */
        onRegistry: function(operation, component, shift) {
            var layerName = shift === true ? this.shiftContainerName : this.defaultContainerName;
            return operation.call(this.componentRegistry, component, layerName);
        }
        /**
         * An operation of the component registry.
         *
         * @callback LayerManager~registryOperation
         * @param {components.Component} component A component
         * @param {string} containerName Name of a container
         */
        ,

        /**
         * Add a component to a layer.
         *
         * @param {components.Component} component A component
         * @param {boolean} shift Target layer: Shift iff true, otherwise Default
         * @public
         */
        register: function(component, shift) {
            this.onRegistry(this.componentRegistry.register, component, shift);
        },

        /**
         * Remove a component from a layer.
         *
         * @param {components.Component} component A component
         * @param {boolean} shift Source layer: Shift iff true, otherwise Default
         * @public
         */
        unregister: function(component, shift) {
            var id = this.onRegistry(this.componentRegistry.unregister, component, shift);
            delete this.activeLayer[id];
        },

        /**
         * Initialize this LayerManager.
         * This function must be called between registration of all components and first use.
         *
         * @public
         */
        init: function() {
            log.debug("LayerManager.init()");
            this.componentRegistry.disconnectContainer(this.shiftLayer());
            this.unshift();
        },

        /**
         * Destroy the layers, optionally disconnecting all components.
         *
         * @param {boolean} disconnect Iff truthy, components are disconnected
         * @public
         */
        destroy: function(disconnect) {
            log.debug("LayerManager.destroy()");
            this.componentRegistry.destroy(disconnect);
            if (disconnect) {
                this.activeLayer.forEachComponent(function(component) {
                    component.disconnect();
                });
            }
            this.activeLayer = new components.ComponentContainer();

        },

        /**
         * Activate the Shift layer.
         *
         * @public
         */
        shift: function() {
            log.debug("LayerManager.shift()");
            this.activeLayer.applyLayer(this.shiftLayer());
        },

        /**
         * Activate the Default layer.
         *
         * @public
         */
        unshift: function() {
            log.debug("LayerManager.unshift()");
            this.activeLayer.applyLayer(this.defaultLayer());
        },

        /**
         * Delegate a MIDI message to the matching component on the active layer.
         *
         * @param {number} channel Channel of the MIDI message
         * @param {number} control Control byte of the MIDI message
         * @param {number} value Value of the MIDI message
         * @param {number} status Status byte of the MIDI message
         * @public
         */
        input: function(channel, control, value, status /* ignored: ,group */) {
            var component = this.findComponent(status, control);
            if (component === undefined) {
                return;
            }
            if (component.input !== undefined && typeof this.input === "function") {
                component.input(channel, control, value, status, component.group);
            } else {
                log.error("Component without input function: "
                    + global.stringifyObject(component));
            }
        },
    });

    /**
     * A generic, configurable MIDI controller.
     *
     * The mapping is configured by the function `configurationProvider` which returns an object
     * that is structured as follows:
     *
     *     configuration
     *     |
     *     +- init: (optional) A function that is called when Mixxx is started
     *     +- shutdown: (optional) A function that is called when Mixxx is shutting down
     *     |
     *     +- decks: An array of deck definitions (may be empty or omitted)
     *     |  +- deck:
     *     |     +- deckNumbers: As defined by {components.Deck}
     *     |     +- components: An array of component definitions for the deck
     *     |     |  +- component:
     *     |     |     +- type:    Component type (constructor function, required)
     *     |     |     |           Example: `components.Button`
     *     |     |     +- shift:   Active only when a Shift button is pressed? (boolean, optional)
     *     |     |     |           Example: `true`
     *     |     |     +- options: Additional options for the component (object, required)
     *     |     |                 Example: {midi: [0xB0, 0x43], key: "reverse"}
     *     |     +- equalizerUnit: Equalizer unit definition (optional)
     *     |        +- components: An object of component definitions for the unit.
     *     |        |              Each definition is a key-value pair for a component of
     *     |        |              `EqualizerUnit` where `key` is the name of
     *     |        |              the component and `value` is the MIDI address.
     *     |        |              Example: `super1: [0xB0, 0x29]`
     *     |        +- feedback: Enable controller feedback (boolean, optional)
     *     |        |            When set to `true`, values of the components in this unit are sent
     *     |        |            to the hardware controller on changes. The address of the MIDI
     *     |        |            message is taken from the `midi` property of the affected
     *     |        |            component.
     *     |        +- output: Additional output definitions (optional).
     *     |                   The structure of this object is the same as the structure of
     *     |                   `components`. Every value change of a component contained in `output`
     *     |                   causes a MIDI message to be sent to the hardware controller, using
     *     |                   the configured address instead of the component's `midi` property.
     *     |                   This option is independent of the `feedback` option.
     *     |
     *     +- effectUnits: An array of effect unit definitions (may be empty or omitted)
     *     |  +- effectUnit
     *     |     +- unitNumbers: As defined by `components.EffectUnit`.
     *     |     +- components: As described for equalizer unit using `components.EffectUnit`
     *     |     |              instead of `EqualizerUnit`.
     *     |     |              Example: `effectFocusButton: [0xB0, 0x15]`
     *     |     +- feedback: As described for equalizer unit
     *     |     +- output: As described for equalizer unit
     *     |
     *     +- containers: An array of component container definitions (may be empty or omitted)
     *        +- componentContainer
     *           +- components: An object of component definitions for the component container.
     *           |  +- component: A component definition in the same format as described for decks
     *           +- type: (function, optional) Constructor; default: `components.ComponentContainer`
     *           +- options: (object, optional) Constructor argument for the container
     *           +- defaultDefinition: (object, optional) Default definition for components in the
     *           |                                        container
     *           +- init: (function, optional) A function that is called after component creation
     *                                         and before first use
     *
     * @constructor
     * @extends {components.ComponentContainer}
     * @param {object} options Options object
     * @param {function} components.configurationProvider Mapping configuration provider
     * @public
     */
    var GenericMidiController = function(options) {
        if (!options || typeof options.configurationProvider !== "function") {
            log.error("The required function 'configurationProvider' is missing.");
            return;
        }
        this.config = options.configurationProvider.call(this);
        components.ComponentContainer.call(this, options);
    };
    GenericMidiController.prototype = deriveFrom(components.ComponentContainer, {

        /**
         * Contains all decks and effect units so that a (un)shift operation
         * is delegated to the decks, effect units and their children.
         *
         * @private
         */
        componentContainers: [],

        /**
         * Initialize the controller mapping.
         * This function is called by Mixxx on startup.
         *
         * @param {string} controllerId Controller-ID
         * @param {boolean} debug Is the application in debug mode?
         * @public
         */
        init: function(controllerId, debug) {
            this.controllerId = controllerId;
            this.debug = debug;

            if (typeof this.config.init === "function") {
                this.config.init(controllerId, debug);
            }
            this.layerManager = this.createLayerManager(
                this.componentContainers,
                this.config.decks || [],
                this.config.effectUnits || [],
                this.config.containers || []);

            log.debug(this.controllerId + ".init() completed.");
        },

        /**
         * Shutdown the controller mapping.
         * This function is called by Mixxx on shutdown.
         *
         * @public
         */
        shutdown: function() {
            this.layerManager.destroy();
            if (typeof this.config.shutdown === "function") {
                this.config.shutdown();
            }
            log.debug(this.controllerId + ".shutdown() completed.");
        },

        /**
         * Delegate a MIDI message to a component within this controller mapping.
         *
         * This function may be used in the XML mapping file for all components of this mapping.
         *
         * @param {number} channel Channel of the MIDI message
         * @param {number} control Control byte of the MIDI message
         * @param {number} value Value of the MIDI message
         * @param {number} status Status byte of the MIDI message
         * @param {string} group Group of the component (ignored, taken from the component instead)
         * @public
         */
        input: function(channel, control, value, status, group) {
            return this.layerManager.input(channel, control, value, status, group);
        },

        /**
         * Create a layer manager containing the components of all decks, effect units and
         * additional component containers.
         *
         * @param {Array} target Target for decks, effect units and additional component containers
         * @param {object} deckDefinitions Definition of decks
         * @param {object} effectUnitDefinitions Definition of effect units
         * @param {object} containerDefinitions Definition of additional component containers
         * @return {object} Layer manager
         * @see `LayerManager`
         * @private
         */
        createLayerManager: function(target,
            deckDefinitions, effectUnitDefinitions, containerDefinitions) {

            var layerManager = new LayerManager({debug: this.debug});
            var controller = this;
            var registerComponents = function(definition, implementation) {
                controller.registerComponents(layerManager, definition, implementation);
            };

            [{
                definitions: deckDefinitions,
                factory: this.createDeck,
                register: function(deckDefinition, deckImplementation) {
                    registerComponents(deckDefinition.components, deckImplementation);
                    if (deckDefinition.equalizerUnit) {
                        registerComponents(
                            deckDefinition.equalizerUnit.midi, deckImplementation.equalizerUnit);
                    }
                }
            },
            {
                definitions: effectUnitDefinitions,
                factory: this.createEffectUnit,
                register: function(effectUnitDefinition, effectUnitImplementation) {
                    registerComponents(effectUnitDefinition.midi, effectUnitImplementation);
                }
            },
            {
                definitions: containerDefinitions,
                factory: this.createComponentContainer,
                register: function(containerDefinition, containerImplementation) {
                    registerComponents(containerDefinition.components, containerImplementation);
                }
            }].forEach(function(context) {
                if (Array.isArray(context.definitions)) {
                    context.definitions.forEach(function(definition) {
                        var implementation = context.factory.call(this, definition, target);
                        target.push(implementation);
                        context.register(definition, implementation);
                    }, this);
                } else {
                    log.error(this.controllerId + ": Skipping a part of the configuration because "
                            + "the following definition is not an array: "
                            + stringifyObject(context.definitions));
                }
            }, this);
            layerManager.init();
            return layerManager;
        },

        /**
         * Create a deck.
         *
         * @param {object} deckDefinition Definition of the deck
         * @param {Array} componentStorage Storage for additionally created components
         * @yields {components.Deck} The new deck
         * @private
         */
        createDeck: function(deckDefinition, componentStorage) {
            var deck = new components.Deck(deckDefinition.deckNumbers);
            deckDefinition.components.forEach(function(componentDefinition, index) {
                if (componentDefinition && componentDefinition.type) {
                    var options = _.merge({group: deck.currentDeck}, componentDefinition.options);
                    deck[index] = new componentDefinition.type(options);
                } else {
                    log.error("Skipping component without type on Deck of " + deck.currentDeck
                        + ": " + stringifyObject(componentDefinition));
                    deck[index] = null;
                }
            }, this);
            if (deckDefinition.equalizerUnit) {
                deck.equalizerUnit = this.setupMidi(
                    deckDefinition.equalizerUnit,
                    new EqualizerUnit(deck.currentDeck),
                    componentStorage);
            }
            return deck;
        },

        /**
         * Process all MIDI addresses (number arrays) of a component container definition.
         *
         * @param {object} definition Definition of a component container with MIDI addresses
         * @param {components.ComponentContainer} implementation Corresponding component container
         *                                        object
         * @param {function} action Function that performs the processing of a single MIDI address
         * @private
         */
        processMidiAddresses: function(definition, implementation, action) {
            if (Array.isArray(definition)) {
                action.call(this, definition, implementation);
            } else if (typeof definition === "object") {
                Object.keys(definition).forEach(function(name) {
                    this.processMidiAddresses(definition[name], implementation[name], action);
                }, this);
            }
        },

        /**
         * Create and store a publisher for a source component.
         *
         * @param {components.Component} source Source component of the publisher
         * @param {Array} publisherStorage Storage for the publisher component
         * @yields {Publisher} Publisher
         * @private
         */
        createPublisher: function(source, publisherStorage) {
            var publisher = new Publisher({source: source});
            publisherStorage.push(publisher);
            return publisher;
        },

        /**
         * Setup MIDI input and output for a component container.
         *
         * Publisher components will be created when the definition is configured respectively.
         *
         * @param {object} definition Definition of a component container with MIDI addresses
         * @param {object} implementation Corresponding component container object
         * @param {Array} publisherStorage Storage for publisher components
         * @param {Array<string>} rebindTriggers Names of functions that trigger rebinding a
         *                                       publisher to its source component
         * @return {components.ComponentContainer} The given component container argument
         * @private
         */
        setupMidi: function(definition, implementation, publisherStorage, rebindTriggers) {

            /* Set MIDI address in implementation components */
            this.processMidiAddresses(definition.midi, implementation,
                function(componentDefinition, componentImplementation) {
                    componentImplementation.midi = componentDefinition;
                });

            /* Add publishers for pots */
            if (definition.feedback) {
                var triggers = rebindTriggers || [];
                var createPublisher = this.createPublisher; // `this` is bound to implementation
                implementation.forEachComponent(function(effectComponent) {
                    if (effectComponent instanceof components.Pot) {
                        var publisher = createPublisher(effectComponent, publisherStorage);
                        var prototype = Object.getPrototypeOf(effectComponent);
                        triggers.forEach(function(functionName) {
                            var delegate = prototype[functionName];
                            if (typeof delegate === "function") {
                                prototype[functionName] = function() {
                                    delegate.apply(this, arguments);
                                    publisher.bind();
                                };
                            }
                        });
                    }
                });
            }

            /* Add publishers for output definitions */
            if (definition.output) {
                this.processMidiAddresses(definition.output, implementation,
                    function(midi, component) {
                        this.createPublisher(
                            {midi: midi, group: component.group, inKey: component.inKey},
                            publisherStorage);
                    });
            }
            return implementation;
        },

        /**
         * Create an effect unit.
         *
         * The values of the unit's components are sent to the controller
         * when the definition is configured respectively.
         *
         * @param {object} effectUnitDefinition Definition of the effect unit
         * @param {Array} componentStorage Storage for additionally created components
         * @yields {components.EffectUnit}
         * @private
         */
        createEffectUnit: function(effectUnitDefinition, componentStorage) {
            var effectUnit = this.setupMidi(
                effectUnitDefinition,
                new components.EffectUnit(effectUnitDefinition.unitNumbers, true),
                componentStorage,
                ["onFocusChange", "shift", "unshift"]);
            effectUnit.init();
            return effectUnit;
        },

        /**
         * Create a component container.
         *
         * @param {object} containerDefinition Definition of the component container
         * @yields {object} The new component container
         * @private
         */
        createComponentContainer: function(containerDefinition) {
            var containerType = containerDefinition.type || components.ComponentContainer;
            var container = new containerType(containerDefinition.options);
            if (containerDefinition.components) {
                containerDefinition.components.forEach(function(componentDefinition, index) {
                    var definition = _.merge(
                        {}, containerDefinition.defaultDefinition || {}, componentDefinition);
                    container[index] = this.createComponent(definition);
                }, this);
            }
            if (typeof containerDefinition.init === "function") {
                containerDefinition.init.call(container);
            }
            return container;
        },

        /**
         * Create a component or component container.
         *
         * @param {object} definition Definition of a component or component container
         * @yields {component.Component|component.ComponentContainer|null} The new component or component container
         * @private
         */
        createComponent: function(definition) {
            var component = null;
            if (definition && definition.type) {
                if (definition.type.prototype instanceof components.ComponentContainer) {
                    component = this.createComponentContainer(definition);
                } else {
                    component = new definition.type(definition.options);
                }
            } else {
                log.error("Skipping invalid component definition without type: "
                    + stringifyObject(definition));
            }
            return component;
        },

        /**
         * Register a component with all its child components in the layer manager.
         *
         * @param {LayerManager} layerManager Layer manager
         * @param {object} definition Definition of a component
         * @param {components.Component|components.ComponentContainer} Implementation of a component
         * @private
         */
        registerComponents: function(layerManager, definition, implementation) {
            if (implementation instanceof components.Component) {
                layerManager.register(implementation, definition && definition.shift === true);
            } else if (implementation instanceof components.ComponentContainer) {
                Object.keys(implementation).forEach(function(name) {
                    var definitionName = definition ? definition[name] : null;
                    this.registerComponents(layerManager, definitionName, implementation[name]);
                }, this);
            }
        },
    });

    var exports = {};
    exports.deriveFrom = deriveFrom;
    exports.ShiftButton = ShiftButton;
    exports.Trigger = Trigger;
    exports.CustomButton = CustomButton;
    exports.Timer = Timer;
    exports.LongPressButton = LongPressButton;
    exports.BlinkingButton = BlinkingButton;
    exports.DirectionEncoder = DirectionEncoder;
    exports.RangeAwareEncoder = RangeAwareEncoder;
    exports.EnumToggleButton = EnumToggleButton;
    exports.EnumEncoder = EnumEncoder;
    exports.LoopEncoder = LoopEncoder;
    exports.LoopMoveEncoder = LoopMoveEncoder;
    exports.BackLoopButton = BackLoopButton;
    exports.CrossfaderCurvePot = CrossfaderCurvePot;
    exports.Publisher = Publisher;
    exports.LayerManager = LayerManager;
    exports.GenericMidiController = GenericMidiController;
    global.behringer = _.assign(global.behringer, {extension: exports});
})(this);
