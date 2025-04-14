/**
 * Additional JS components for Mixxx
 */
(function(global) {

    /** @private */
    const components = global.components;

    /** @private */
    const engine = global.engine;

    /**
     * Determines the merge strategy for a value when merging a component container definition.
     *
     * @param value anything
     * @returns {boolean} merge strategy for the value: true for 'assign', false for 'deep merge'
     * @private
     */
    const doAssign = value => {
        return typeof value !== "object"
            || value === null
            || value instanceof components.ComponentContainer
            || value instanceof components.Component;
    };

    /**
     * Merges a list of component container definitions into a target object.
     *
     * This is not a generic deep merge algorithm for JS objects. It does not handle circular references.
     * If a definition contains a reference to an object instance of a component or component container
     * (e.g. `ShiftButton.target`), the instance is taken over by reference, no deep copy is made.
     *
     * @param {object} targetObject Target object for the merged definitions
     * @param {Array} sources Source definitions
     * @returns {object} The target object
     * @see `GenericMidiController` on component container definition
     * @private
     */
    const mergeDefinitions = (targetObject, ...sources) => sources.reduce((target, source) => {
        for (const [key, value] of Object.entries(source || {})) {
            if (doAssign(value)) {
                target[key] = value;
            } else if (value !== undefined) {
                if (Array.isArray(value) && !Array.isArray(target[key])) {
                    target[key] = [];
                } else if (typeof target[key] !== "object" || target[key] === null) {
                    target[key] = {};
                }
                mergeDefinitions(target[key], value);
            }
        };
        return target;
    }, targetObject);

    /**
     * Contains functions to print a message to the log.
     * `debug` output is suppressed unless the caller owns a truthy property `debug`.
     *
     * @param {string} message Message
     * @private
     */
    const log = {
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
     * @returns {string} ID for the MIDI address; `undefined` on error
     * @private
     */
    const findComponentId = function(midiAddress) {
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
     * @returns {string} A short string that describes the component; `undefined` on error
     * @private
     */
    const stringifyComponent = function(component) {
        if (!component) {
            return;
        }
        const key = component.inKey || component.outKey;
        let value = `${component.group},${key}`;
        if (component.midi) {
            const id = findComponentId(component.midi);
            if (id !== undefined) {
                value = id + ": " + value;
            }
        }
        return "(" + value + ")";
    };

    /**
     * Convert a parameter value (0..1) to a MIDI value (0..this.max).
     *
     * @param {number} value A number between 0 and 1.
     * @private
     */
    const convertToMidiValue = function(value) {
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
     * @returns {object} A new prototype based on parent with the given members
     * @public
     */
    const deriveFrom = function(parent, members) {
        return Object.assign(Object.create(parent.prototype), members);
    };

    /**
     * Perform an action, throttled if the owner supports throttling.
     *
     * @param {function} action The action to perform
     * @param {object} owner Object used as `this` for the action
     * @private
     * @see `Throttler`
     */
    const throttle = function(action, owner) {
        if (owner.throttler) {
            owner.throttler.schedule(action, owner);
        } else {
            action.call(owner);
        }
    };

    /**
     * A component that uses the parameter instead of the value as output.
     *
     * @constructor
     * @extends {components.Component}
     * @param {object} options Options object
     * @public
     */
    const ParameterComponent = function(options) {
        components.Component.call(this, options);
    };
    ParameterComponent.prototype = deriveFrom(components.Component, {
        outValueScale: function(_value) {
            /*
             * We ignore the argument and use the parameter (0..1) instead because value scale is
             * arbitrary and thus cannot be mapped to MIDI values (0..127) properly.
             */
            return convertToMidiValue.call(this, this.outGetParameter());
        },
    });

    /**
     * A button to toggle un-/shift on a target component.
     *
     * @constructor
     * @extends {components.Button}
     * @param {object} options Options object
     * @param {components.Component|components.ComponentContainer} options.target Target component
     * @public
     */
    const ShiftButton = function(options) {
        components.Button.call(this, options);
    };
    ShiftButton.prototype = deriveFrom(components.Button, {
        inSetValue: function(value) {
            if (value) {
                this.target.shift();
            } else {
                this.target.unshift();
            }
            components.Button.prototype.inSetValue.call(this, value);
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
    const Trigger = function(options) {
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
    const CustomButton = function(options) {
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
     * @param {number} options.timeout Duration between start and action (in ms)
     * @param {boolean} options.oneShot If `true`, the action is run once;
     *                          otherwise, it is run periodically until the timer is reset.
     * @param {function} options.action Function that is executed whenever the timer expires
     * @param {object} options.owner Owner object of the `action` function (assigned to `this`)
     * @public
     * @see https://github.com/mixxxdj/mixxx/wiki/Script-Timers
     */
    const Timer = function(options) {
        Object.assign(this, options);
        this.disable();
    };
    Timer.prototype = {
        disable: function() { this.id = 0; },
        isEnabled: function() { return this.id !== 0; },
        start: function() {
            this.reset();
            this.id = engine.beginTimer(this.timeout, function() {
                if (this.oneShot) {
                    this.disable();
                }
                this.action.call(this.owner);
            }.bind(this), this.oneShot); // .bind(this) is required instead of arrow function for Qt < 6.2.4 due to QTBUG-95677
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
     * An object that enforces a constant delay between the execution of consecutive actions.
     *
     * Use `schedule(action, owner)` to perform an action on the owner as soon as the delay has
     * elapsed after the preceding action has finished.
     *
     * @constructor
     * @param {number} options.delay Minimal delay between two consecutive actions (in ms)
     * @public
     */
    const Throttler = function(options) {
        options = options || {};
        options.delay = options.delay || 0;
        Object.assign(this, options);
        this.locked = false;
        this.jobs = [];
        this.unlockTimer = new Timer(
            {timeout: this.delay, oneShot: true, action: this.unlock, owner: this});
    };
    Throttler.prototype = {
        schedule: function(action, owner) {
            this.jobs.push({action: action, owner: owner});
            this.notify();
        },

        notify: function() {
            if (this.jobs.length > 0 && this.acquireLock()) {
                const job = this.jobs.shift();
                job.action.call(job.owner);
                this.unlockTimer.start();
            }
        },

        acquireLock: function() {
            const unlocked = !this.locked;
            if (unlocked) {
                this.locked = true;
            }
            return unlocked;
        },

        unlock: function() {
            this.locked = false;
            this.notify();
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
    const LongPressButton = function(options) {
        components.Button.call(this, options);
        const action = function() {
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
    const BlinkingButton = function(options) {
        options = options || {};
        const blinkAction = function() {
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
     * This component supports an optional relative mode as an alternative to
     * dealing with soft takeover. To use it, set the `relative` property to
     * `true` in the options object for the constructor. In this mode, moving
     * the Pot will adjust the Mixxx Control relative to its current value.
     * Holding shift and moving the encoder will not affect the Mixxx Control.
     * This allows the user to continue adjusting the Mixxx Control after
     * the encoder has reached the end of its physical range.
     *
     * @constructor
     * @extends {components.Encoder}
     * @param {object} options Options object
     * @public
     */
    const DirectionEncoder = function(options) {
        components.Encoder.call(this, options);
        this.previousValue = this.inGetValue(); // available only after call of Encoder constructor
    };
    DirectionEncoder.prototype = deriveFrom(components.Encoder, {
        min: 0,
        inValueScale: function(value) {
            let direction = 0;
            if (!(this.relative && this.isShifted)) {
                if (value > this.previousValue || value === this.max) {
                    direction = 1;
                } else if (value < this.previousValue || value === this.min) {
                    direction = -1;
                }
                this.previousValue = value;
            }
            return direction;
        },
        shift: function() {
            this.isShifted = true;
        },
        unshift: function() {
            this.isShifted = false;
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
    const RangeAwareEncoder = function(options) {
        components.Encoder.call(this, options);
    };
    RangeAwareEncoder.prototype = deriveFrom(components.Encoder, {
        outValueScale: function(value) {
            /* -bound..+bound => 0..1 */
            const normalizedValue = (value + this.bound) / (2 * this.bound);
            /* 0..1 => 0..127 */
            return convertToMidiValue.call(this, normalizedValue);
        },
    });

    /**
     * A pot for a value range of [-bound..0..+bound].
     *
     * @constructor
     * @extends {components.Pot}
     * @param {object} options Options object
     * @param {number} options.bound A positive integer defining the range bounds
     * @public
     */
    const RangeAwarePot = function(options) {
        components.Pot.call(this, options);
    };
    RangeAwarePot.prototype = deriveFrom(components.Pot, {
        outValueScale: RangeAwareEncoder.prototype.outValueScale
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
    const EnumToggleButton = function(options) {
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
                let newValue;
                if (this.values) {
                    const index = this.values.indexOf(this.inGetValue());
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
     * @param {boolean} options.softTakeover (optional) Enable soft-takeover; default: `true`
     * @public
     * @see https://github.com/mixxxdj/mixxx/wiki/Midi-Scripting#soft-takeover
     */
    const EnumEncoder = function(options) {
        options = options || {};
        if (options.values === undefined) {
            log.error("EnumEncoder constructor was called without specifying enum values.");
            options.values = [];
        }
        if (options.softTakeover === undefined) { // do not use '||' to allow false
            options.softTakeover = true;
        }
        options.maxIndex = options.values.length - 1;
        components.Encoder.call(this, options);
    };
    EnumEncoder.prototype = deriveFrom(components.Encoder, {
        input: function(_channel, _control, value, _status, _group) {
            const scaledValue = this.inValueScale(value);
            if (!this.softTakeover
                || this.previousValue === undefined
                || this.previousValue === this.inGetValue()) {
                this.inSetParameter(scaledValue);
            }
            this.previousValue = scaledValue;
        },
        inValueScale: function(value) {
            const normalizedValue = value / this.max;
            const index = Math.round(normalizedValue * this.maxIndex);
            return this.values[index];
        },
        outValueScale: function(value) {
            const index = this.values.indexOf(value);
            if (index !== -1) {
                const normalizedValue = index / this.maxIndex;
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
    const LoopEncoder = function(options) {
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
     * @extends {DirectionEncoder}
     * @param {object} options Options object
     * @param {number} options.size (optional) Size given in number of beats; default: 0.5
     * @param {string} options.sizeControl (optional) Name of a control that contains `size`
     * @public
     */
    const LoopMoveEncoder = function(options) {
        options = options || {};
        options.inKey = options.inKey || "loop_move";
        options.size = options.size || 0.5;
        DirectionEncoder.call(this, options);
    };
    LoopMoveEncoder.prototype = deriveFrom(DirectionEncoder, {
        inValueScale: function(value) {
            const direction = DirectionEncoder.prototype.inValueScale.call(this, value);
            const beats = this.sizeControl
                ? engine.getValue(this.group, this.sizeControl)
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
    const BackLoopButton = function(options) {
        options = options || {};
        options.key = options.key || "loop_enabled";
        components.Button.call(this, options);
    };
    BackLoopButton.prototype = deriveFrom(components.Button, {
        inSetValue: function(value) {
            const script = global.script;
            const group = this.group;
            if (value) {
                const loopSize = engine.getValue(group, "beatloop_size");
                const beatjumpSize = engine.getValue(group, "beatjump_size");
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
    const CrossfaderCurvePot = function(options) {
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
    const Publisher = function(options) {
        if (options.source === undefined) {
            log.error("Missing source component");
            return;
        }
        this.source = options.source;
        this.sync();
        ParameterComponent.call(this, options);
    };
    Publisher.prototype = deriveFrom(ParameterComponent, {
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

    const EffectUnit = function(rack, deckGroup) {
        components.ComponentContainer.call(this);
        const effectGroup = `[${rack}_${deckGroup}_Effect1]`;
        const channelGroup = `[${rack}_${deckGroup}]`;

        const ParameterKnob = function(parameterNumber) {
            components.Pot.call(this, {group: effectGroup, key: "parameter" + parameterNumber});
        };
        ParameterKnob.prototype = deriveFrom(components.Pot);
        const ParameterButton = function(parameterNumber) {
            components.Button.call(this, {group: effectGroup, key: "button_parameter" + parameterNumber});
        };
        ParameterButton.prototype = deriveFrom(
            components.Button, {type: components.Button.prototype.types.powerWindow});

        this.enabled = new components.Button(
            {group: effectGroup, key: "enabled", type: components.Button.prototype.types.powerWindow});
        this.meta = new components.Pot({group: effectGroup, key: "meta"});
        this.super1 = new components.Pot({group: channelGroup, key: "super1"});
        this.mix = new components.Pot({group: channelGroup, key: "mix"});

        this.parameterKnobs = new components.ComponentContainer();
        const parameterKnobCount = engine.getValue(effectGroup, "num_parameters");
        [...Array(parameterKnobCount)].map((x, i) => i+1).forEach(knobIndex =>
            this.parameterKnobs[knobIndex] = new ParameterKnob(knobIndex));

        this.parameterButtons = new components.ComponentContainer();
        const parameterButtonCount = engine.getValue(effectGroup, "num_button_parameters");
        [...Array(parameterButtonCount)].map((x, i) => i+1).forEach(buttonIndex =>
            this.parameterButtons[buttonIndex] = new ParameterButton(buttonIndex));
    };
    EffectUnit.prototype = deriveFrom(components.ComponentContainer);

    /**
     * @typedef {components.ComponentContainer} EqualizerUnit
     *
     * @property {components.Button} enabled En-/disable equalizer unit
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
    const EqualizerUnit = function(deckGroup) {
        EffectUnit.call(this, "EqualizerRack1", deckGroup);
    };
    EqualizerUnit.prototype = deriveFrom(EffectUnit);

    /**
     * @typedef {components.ComponentContainer} QuickEffectUnit
     *
     * @property {components.Button} enabled En-/disable quick effect unit
     * @property {components.Pot} meta Meta knob
     * @property {components.Pot} super1 Super knob
     * @property {components.Pot} parameterKnobs.1 Parameter 1
     * @property {components.Pot} parameterKnobs.2 Parameter 2
     * @property {components.Pot} parameterKnobs.3 Parameter 3
     * @property {components.Pot} parameterKnobs.4 Parameter 4
     * @property {components.Pot} parameterKnobs.5 Parameter 5
     * @property {components.Button} parameterButtons.1 Parameter Button 1
     * @property {components.Button} parameterButtons.2 Parameter Button 2
     */

    /**
     * A component container for quick effect controls.
     *
     * @constructor
     * @extends {components.ComponentContainer}
     * @param {string} deckGroup Group of the deck this unit belongs to (e.g. `[Channel1]`)
     * @yields {QuickEffectUnit}
     * @public
     */
    const QuickEffectUnit = function(deckGroup) {
        EffectUnit.call(this, "QuickEffectRack1", deckGroup);
    };
    QuickEffectUnit.prototype = deriveFrom(EffectUnit);

    /**
     * Manage Components in named ComponentContainers.
     *
     * @constructor
     * @param {Array<string>} initialContainers Initial container names
     * @public
     */
    const ComponentRegistry = function(initialContainers) {
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
         * @returns {components.ComponentContainer} The ComponentContainer; `undefined` on error
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
         * @returns {components.ComponentContainer} The ComponentContainer; `undefined` on error
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
         * @returns {string} ID of the stored component; `undefined` on error
         * @public
         */
        register: function(component, containerName) {
            if (component === undefined) {
                log.error("Missing component");
                return;
            }
            if (!component.midi) {
                log.debug(containerName + ": ignore "
                    + stringifyComponent(component) + " without MIDI address");
                return;
            }
            const id = findComponentId(component.midi);
            if (!Object.prototype.hasOwnProperty.call(this.containers, containerName)) {
                this.createContainer(containerName);
            }
            const container = this.getContainer(containerName);
            let store = true;
            if (Object.prototype.hasOwnProperty.call(container, id)) {
                const old = container[id];
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
         * @returns {string} ID of the removed component; `undefined` on error
         * @public
         */
        unregister: function(component, containerName) {
            log.debug(containerName + ": unregister " + stringifyComponent(component));
            const container = this.getContainer(containerName);
            const id = findComponentId(component.midi);
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
    const LayerManager = function(options) {
        this.componentRegistry = new ComponentRegistry([
            LayerManager.prototype.defaultContainerName,
            LayerManager.prototype.shiftContainerName]);
        this.activeLayer = new components.ComponentContainer();
        this.inputConnections = {};
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
         * @returns {object} The Default layer
         * @private
         */
        defaultLayer: function() {
            const defaultContainer = this.componentRegistry.getContainer(this.defaultContainerName);
            return Object.keys(this.shiftLayer()).reduce(
                function(shiftCounterparts, name) {
                    shiftCounterparts[name] = defaultContainer[name];
                    return shiftCounterparts;
                }, {});
        },

        /**
         * Retrieve the Shift layer.
         *
         * @returns {object} The Shift layer
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
         * @returns {components.Component} Component on the active layer matching the MIDI address;
         *                                undefined on error. When the active layer does not contain
         *                                a matching component, the Default layer is used as
         *                                fallback.
         * @private
         */
        findComponent: function(status, control) {
            const id = findComponentId([status, control]);
            if (id === undefined) {
                return;
            }
            let component = this.activeLayer[id];
            if (component === undefined) {
                const defaultComponents
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
         * @returns Result of the operation
         * @private
         */
        onRegistry: function(operation, component, shift) {
            const layerName = shift === true ? this.shiftContainerName : this.defaultContainerName;
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
         * Add a component to a layer, and provide an input connection for its MIDI address.
         *
         * @param {components.Component} component A component
         * @param {boolean} shift Target layer: Shift iff true, otherwise Default
         * @public
         */
        register: function(component, shift) {
            this.onRegistry(this.componentRegistry.register, component, shift);
            if (component.midi) {
                const id = findComponentId(component.midi);
                if (this.inputConnections[id] === undefined) {
                    this.inputConnections[id] = midi.makeInputHandler(
                        component.midi[0], component.midi[1], this.input.bind(this));
                }
            }
        },

        /**
         * Remove a component from a layer,
         * and disconnect the MIDI input if not used by another component.
         *
         * @param {components.Component} component A component
         * @param {boolean} shift Source layer: Shift iff true, otherwise Default
         * @public
         */
        unregister: function(component, shift) {
            const id = this.onRegistry(this.componentRegistry.unregister, component, shift);
            delete this.activeLayer[id];
            if (!this.findComponent(id)) {
                this.inputConnections[id].disconnect();
                delete this.inputConnections[id];
            }
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
            const component = this.findComponent(status, control);
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
     *     +- throttleDelay (optional): A positive number (in ms) that is used to slow down the
     *     |                            initialization of the controller; this option is useful if
     *     |                            the hardware is limited to process a certain number of MIDI
     *     |                            messages per time.
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
     *     |     |  +- midi: An object of component definitions for the unit.
     *     |     |  |        Each definition is a key-value pair for a component of `EqualizerUnit`
     *     |     |  |        where `key` is the name of the component and `value` is the MIDI
     *     |     |  |        address. Examples:
     *     |     |  |          `super1: [0xB0, 0x29]`
     *     |     |  |          `parameterKnobs: {1: [0xB0, 0x06], 2: [0xB0, 0x05], 3: [0xB0, 0x04]}`
     *     |     |  +- feedback: Enable controller feedback (boolean, optional)
     *     |     |  |            When set to `true`, values of the components in this unit are sent
     *     |     |  |            to the hardware controller on changes. The address of the MIDI
     *     |     |  |            message is taken from the `midi` property of the affected
     *     |     |  |            component.
     *     |     |  +- feedbackOnRelease: Enable controller feedback on button release (boolean, optional)
     *     |     |  |            When set to `true`, values of the buttons in this unit are sent
     *     |     |  |            to the hardware controller on release, no matter if changed or not.
     *     |     |  |            The address of the MIDI message is taken from the `midi` property of the
     *     |     |  |            affected component.
     *     |     |  +- output: Additional output definitions (optional).
     *     |     |             The structure of this object is the same as the structure of
     *     |     |             `midi`. Every value change of a component contained in `output`
     *     |     |             causes a MIDI message to be sent to the hardware controller, using
     *     |     |             the configured address instead of the component's `midi` property.
     *     |     |             This option is independent of the `feedback` option.
     *     |     +- quickEffectUnit: Quick effect unit definition (optional)
     *     |        +- midi: As described for equalizer unit using `components.QuickEffectUnit` instead of
     *     |        |        `EqualizerUnit`. Examples:
     *     |        |          `enabled: [0x90, 0x02]`
     *     |        |          `super1: [0xB0, 0x06]`
     *     |        +- feedback: As described for equalizer unit
     *     |        +- feedbackOnRelease: As described for equalizer unit
     *     |        +- output: As described for equalizer unit
     *     |
     *     +- effectUnits: An array of effect unit definitions (may be empty or omitted)
     *     |  +- effectUnit
     *     |     +- unitNumbers: As defined by `components.EffectUnit`.
     *     |     +- midi: As described for equalizer unit using `components.EffectUnit` instead of
     *     |     |        `EqualizerUnit`. Examples:
     *     |     |          `effectFocusButton: [0xB0, 0x15]`
     *     |     |          `knobs: {1: [0xB0, 0x26], 2: [0xB0, 0x25], 3: [0xB0, 0x24]}`
     *     |     +- feedback: As described for equalizer unit
     *     |     +- feedbackOnRelease: As described for equalizer unit
     *     |     +- output: As described for equalizer unit
     *     |     +- sendShiftedFor: Type of components that send shifted MIDI messages (optional)
     *     |                        When set, all components of this type within this effect unit
     *     |                        are configured to send shifted MIDI messages
     *     |                        (`sendShifted: true`).
     *     |                        Example: `sendShiftedFor: c.Button`
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
    const GenericMidiController = function(options) {
        if (!options || typeof options.configurationProvider !== "function") {
            log.error("The required function 'configurationProvider' is missing.");
            return;
        }
        this.config = options.configurationProvider.call(this);
        components.ComponentContainer.call(this, options);
    };
    GenericMidiController.prototype = deriveFrom(components.ComponentContainer, {

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

            const delay = this.config.throttleDelay;
            if (delay > 0) {
                log.debug("Component registration is throttled using a delay of " + delay + "ms");
                this.throttler = new Throttler({delay: delay});
            }

            if (typeof this.config.init === "function") {
                this.config.init(controllerId, debug);
            }

            /*
             * Contains all decks and effect units so that a (un)shift operation
             * is delegated to the decks, effect units and their children.
             */
            this.componentContainers = [];

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
         * @returns {object} Layer manager
         * @see `LayerManager`
         * @private
         */
        createLayerManager: function(target,
            deckDefinitions, effectUnitDefinitions, containerDefinitions) {

            const layerManager = new LayerManager({debug: this.debug});
            const controller = this;
            const registerComponents = function(definition, implementation) {
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
                    if (deckDefinition.quickEffectUnit) {
                        registerComponents(
                            deckDefinition.quickEffectUnit.midi, deckImplementation.quickEffectUnit);
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
                        throttle(function() {
                            const implementation = context.factory.call(this, definition, target);
                            target.push(implementation);
                            context.register(definition, implementation);
                        }, this);
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
            const deck = new components.Deck(deckDefinition.deckNumbers);
            deckDefinition.components.forEach(function(componentDefinition, index) {
                const options = Object.assign({group: deck.currentDeck}, componentDefinition.options);
                const definition = Object.assign(componentDefinition, {options: options});
                deck[index] = this.createComponent(definition);
            }, this);
            if (deckDefinition.equalizerUnit) {
                deck.equalizerUnit = this.setupMidi(
                    deckDefinition.equalizerUnit,
                    new EqualizerUnit(deck.currentDeck),
                    componentStorage);
            }
            if (deckDefinition.quickEffectUnit) {
                deck.quickEffectUnit = this.setupMidi(
                    deckDefinition.quickEffectUnit,
                    new QuickEffectUnit(deck.currentDeck),
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
            if (Array.isArray(definition) || !definition) {
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
            const publisher = new Publisher({source: source});
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
         * @returns {components.ComponentContainer} The given component container argument
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
                const triggers = rebindTriggers || [];
                const createPublisher = this.createPublisher; // `this` is bound to implementation
                implementation.forEachComponent(function(source) {
                    if (source instanceof components.Pot) {
                        const publisher = createPublisher(source, publisherStorage);
                        const prototype = Object.getPrototypeOf(source);
                        triggers.forEach(function(functionName) {
                            const delegate = prototype[functionName];
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

            /* Enable feedback on button release for configured components */
            if (definition.feedbackOnRelease) {
                implementation.forEachComponent(function(component) {
                    if (component instanceof components.Button) {
                        component.triggerOnRelease = true;
                    }
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
            const effectUnit = this.setupMidi(
                effectUnitDefinition,
                new components.EffectUnit(effectUnitDefinition.unitNumbers, true),
                componentStorage,
                ["onFocusChange", "shift", "unshift"]);
            const shiftType = effectUnitDefinition.sendShiftedFor;
            /*
             * `shiftType` is expected to be a JS component (e.g. `c.Button` or `c.Component`)
             * which in terms of JS means that it is of type `function`. If something else is given
             * (e.g. a string), `instanceof` will cause a runtime error, so check to avoid this.
             */
            if (typeof shiftType === "function") {
                effectUnit.forEachComponent(function(component) {
                    if (component instanceof shiftType) {
                        component.sendShifted = true;
                    }
                });
            }
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
            const containerType = containerDefinition.type || components.ComponentContainer;
            const container = new containerType(containerDefinition.options);
            if (containerDefinition.components) {
                containerDefinition.components.forEach(function(componentDefinition, index) {
                    const definition = mergeDefinitions({}, containerDefinition.defaultDefinition, componentDefinition);
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
            let component = null;
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
                    const definitionName = definition ? definition[name] : null;
                    this.registerComponents(layerManager, definitionName, implementation[name]);
                }, this);
            }
        },
    });

    const exports = {};
    exports.deriveFrom = deriveFrom;
    exports.ParameterComponent = ParameterComponent;
    exports.ShiftButton = ShiftButton;
    exports.Trigger = Trigger;
    exports.CustomButton = CustomButton;
    exports.Timer = Timer;
    exports.LongPressButton = LongPressButton;
    exports.BlinkingButton = BlinkingButton;
    exports.DirectionEncoder = DirectionEncoder;
    exports.RangeAwareEncoder = RangeAwareEncoder;
    exports.RangeAwarePot = RangeAwarePot;
    exports.EnumToggleButton = EnumToggleButton;
    exports.EnumEncoder = EnumEncoder;
    exports.LoopEncoder = LoopEncoder;
    exports.LoopMoveEncoder = LoopMoveEncoder;
    exports.BackLoopButton = BackLoopButton;
    exports.CrossfaderCurvePot = CrossfaderCurvePot;
    exports.Publisher = Publisher;
    exports.LayerManager = LayerManager;
    exports.GenericMidiController = GenericMidiController;
    global.behringer = Object.assign(global.behringer || {}, {extension: exports});
})(this);
