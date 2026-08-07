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
     * Checks if an argument is a MIDI address.
     *
     * @param {object} object argument
     * @returns {object} `true` if the argument is a MIDI address
     *
     * @private
     */
    const isMidiAddress = function(object) {
        return Array.isArray(object)
            && object.length === 2
            && typeof object[0] === "number"
            && typeof object[1] === "number";
    };

    /**
     * Process all MIDI addresses of a component container definition.
     *
     * @param {object} definition Definition of a component container with MIDI addresses
     * @param {components.ComponentContainer} implementation Corresponding component container object
     * @param {Function} action Function that performs the processing of a single MIDI address
     * @param {string} path Path to the container definition
     * @public
     */
    const processMidiAddresses = function(definition, implementation, action, path = "") {
        if (isMidiAddress(definition) || !definition) {
            action.call(this, definition, implementation, path);
        } else if (typeof definition === "object") {
            Object.keys(definition).forEach(function(name) {
                processMidiAddresses(definition[name], implementation[name], action, path + (path ? "." : "") + name);
            }, this);
        }
    };

    /**
     * Enumerate the groups of all channels, optionally filtered by channel type.
     *
     * @param {string} channelType optional; one of `Channel`, `Sampler`, `Auxiliary`, `Microphone`
     * @returns {Array} Enumeration of all channel groups that match the channel type, or all if type is omitted
     * @public
     */
    const enumerateChannelGroups = function(channelType) {
        const groupFactory = function(type, countControl, index) {
            index = index || function(i) { return i+1; };
            const count = engine.getValue("[App]", countControl);
            return [...Array(count).keys()].map(function(i) { return `[${type}${index(i)}]`; });
        };
        return [
            ["Channel", "num_decks"],
            ["Sampler", "num_samplers"],
            ["Auxiliary", "num_auxiliaries"],
            ["Microphone", "num_microphones", function(i) { return i === 0 ? "" : (i+1); }]
        ]
            .filter(type => !channelType || type[0] === channelType)
            .reduce(function(items, args) { return items.concat(groupFactory.apply(null, args)); }, []);
    };

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
        if (isMidiAddress(midiAddress)) {
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
     * A button that triggers a Mixxx control.
     *
     * @class
     * @augments {components.Button}
     * @param {object} options Options object
     * @param {string} options.inKey Control to trigger when `on`
     * @param {string} options.inKeyOff Control to trigger when `off`; optional, default: `inKey`
     * @public
     */
    const TriggerButton = function(options) {
        components.Button.call(this, options);
    };
    TriggerButton.prototype = deriveFrom(components.Button, {
        isPress: function(_channel, _control, value, _status) {
            return value !== this.off;
        },
        inSetValue: function(value) {
            const control = !this.inKeyOff || value ? this.inKey : this.inKeyOff;
            script.triggerControl(this.group, control);
        }
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
        onLongPress: function(_value) {},
        onRelease: function(value) {
            if (this.isLongPressed) {
                this.onLongRelease(value);
            } else {
                this.onShortRelease(value);
            }
        },
        onShortRelease: function(_value) {},
        onLongRelease: function(_value) {},
    });

    /**
     * A button for key controls.
     *
     * Short press: Toggle keylock
     * Long press: Reset key
     *
     * @class
     * @augments {LongPressButton}
     * @param {object} options Options object
     * @public
     */
    const KeyButton = function(options) {
        options = options || {};
        options.key = options.key || "keylock";
        LongPressButton.call(this, options);
    };
    KeyButton.prototype = deriveFrom(LongPressButton, {
        onShortRelease: function() {
            this.inToggle();
        },
        onLongPress: function() {
            script.triggerControl(this.group, "reset_key");
        },
    });

    /**
     * A button for track load controls.
     *
     * Short press: Load selected track
     * Long press: Eject
     *
     * @class
     * @augments {LongPressButton}
     * @param {object} options Options object
     * @public
     */
    const TrackLoadButton = function(options) {
        options = options || {};
        if (!options.key) {
            options.inKey = options.inKey || "LoadSelectedTrack";
            options.outKey = options.outKey || "track_loaded";
        }
        LongPressButton.call(this, options);
    };
    TrackLoadButton.prototype = deriveFrom(LongPressButton, {
        onShortRelease: function() {
            script.triggerControl(this.group, this.inKey);
        },
        onLongPress: function() {
            script.triggerControl(this.group, "eject");
        },
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
        options.blinkDuration = options.blinkDuration || 500;
        const blinkAction = function() {
            this.send(components.Button.prototype.outValueScale.call(
                this, this.flashing = !this.flashing));
        };
        this.blinkTimer = new Timer(
            {timeout: options.blinkDuration, action: blinkAction, owner: this});
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
     * A sampler button that blinks when playing.
     *
     * @class
     * @param {object} options Options object
     * @augments {components.SamplerButton}
     * @param {number} options.blinkDuration Blink duration in ms; optional, default: 500
     * @public
     */
    const BlinkingSamplerButton = function(options) {
        this.blinkingButton = new BlinkingButton({midi: options.midi, blinkDuration: options.blinkDuration});
        components.SamplerButton.call(this, options);
    };
    BlinkingSamplerButton.prototype = deriveFrom(components.SamplerButton, {
        //on: 0x7F,
        //off: 0x00,
        looping: 0x3F,
        playing: 0x2F,
        loaded: 0x1F,
        empty: 0x10,
        send: function(value) {
            this.blinkingButton.output(value === this.looping || value === this.playing);
            if (value === this.loaded) {
                value = this.on;
            } else if (value === this.empty) {
                value = this.off;
            }
            components.SamplerButton.prototype.send.call(this, value);
        }
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
     * An encoder that in- or decrements the internal value by a step.
     *
     * Turning the encoder to the right will add one step;
     * turning it to the left will subtract one step.
     *
     * @class
     * @augments {DirectionEncoder}
     * @param {object} options Options object
     * @param {number} options.step A positive number defining the step size; optional, default: 0.05
     * @public
     */
    const SteppingEncoder = function(options) {
        DirectionEncoder.call(this, Object.assign({step: 0.05}, options));
    };
    SteppingEncoder.prototype = deriveFrom(DirectionEncoder, {
        inValueScale: function(value) {
            const direction = DirectionEncoder.prototype.inValueScale.call(this, value);
            return this.inGetValue() + (direction * this.step);
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
     * @param {boolean} options.feedback When set to `true`, the input value is sent to the hardware
     *                                   controller; optional, default `false`
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
            if (this.feedback) {
                this.output(value);
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
     * A rotational encoder.
     *
     * This component resembles the behavior of the MIDI options `Rot64`, `Rot64Inv` and `Rot64Fast`:
     * - A value of 65 (0x41) increments the control by 1/16.
     * - A value of 63 (0x3F) decrements the control by 1/16.
     * - Values greater than 65 increment the control by (value - 64).
     * - Values lower than 63 decrement the control by (64 - value).
     *
     * @class
     * @augments {components.Encoder}
     * @param {object} options Options object
     * @param {boolean} options.inverse (optional) If set, the value is inverted
     * @param {boolean} options.fast (optional) If set, the value is multiplied by 1.5
     * @public
     */
    const Rot64Encoder = function(options) {
        options = options || {};
        components.Encoder.call(this, options);
    };
    Rot64Encoder.prototype = deriveFrom(components.Encoder, {
        inValueScale: function(value) {
            let increment = value - 0x40;
            if (this.fast) {
                increment *= 1.5;
            } else if (Math.abs(increment) === 1) {
                increment /= 16;
            } else {
                increment -= Math.sign(increment);
            }
            if (this.inverse) {
                increment = -increment;
            }
            increment = script.absoluteLin(increment, -0.5, 0.5, -0x40, 0x40);

            const newValue = this.inGetValue() + increment;
            return Math.min(1, Math.max(0, newValue));
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
     * move it backwards. The amount of movement is given by:
     *  `sizeControl` (preferred) or `size` when `quantize` is enabled;
     *  `sizeNoQuantize` when `quantize` is disabled.
     *
     * @constructor
     * @augments {DirectionEncoder}
     * @param {object} options Options object
     * @param {number} options.size (optional) Size given in number of beats; default: 0.5
     * @param {number} options.sizeNoQuantize (optional) Size given in number of beats
     *                                        when `quantize` is disabled; default: 0.125
     * @param {string} options.sizeControl (optional) Name of a control that contains `size`
     * @public
     */
    const LoopMoveEncoder = function(options) {
        options = options || {};
        options.inKey = options.inKey || "loop_move";
        options.size = options.size || 0.5;
        options.sizeNoQuantize = options.sizeNoQuantize || 0.125;
        DirectionEncoder.call(this, options);
    };
    LoopMoveEncoder.prototype = deriveFrom(DirectionEncoder, {
        input: function(_channel, _control, value, _status, _group) {
            const direction = DirectionEncoder.prototype.inValueScale.call(this, value);
            if (engine.getValue(this.group, "loop_enabled")) {
                let beats;
                /*
                 * With 'quantize' enabled, the `loop_in` marker might not snap to the beat
                 * we want the loop to start at, but to the next or previous beat.
                 * So we move the loop by one beat.
                 *
                 * With 'quantize' disabled, we might have missed the sweet spot, so we probably
                 * want to move the loop only by a fraction of a beat.
                 */
                if (!engine.getValue(this.group, "quantize")) {
                    beats = this.sizeNoQuantize;
                } else if (this.sizeControl) {
                    beats = engine.getValue(this.group, this.sizeControl);
                } else {
                    beats = this.size;
                }
                this.inSetParameter(direction * beats);
            } else {
                script.triggerControl(this.group, direction > 0 ? "beatjump_forward" : "beatjump_backward");
            }
        },
    });

    /**
     * A blinking button that toggles fader start:
     *
     * If volume is moved from zero, start the deck.
     * If volume is moved to zero, cue the deck.
     * If crossfader is moved from full left (right), start any decks assigned to R (L).
     * If crossfader is moved to full left (right) and decks assigned to R (L) are playing, cue them.
     *
     * @param {object} options Options object
     * @class
     * @augments {BlinkingButton}
     * @public
     */
    const FaderStartToggleButton = function(options) {
        options = options || {};
        options.type = options.type || components.Button.prototype.types.toggle;
        options.outKey = null; // hack to get Component constructor to call connect()
        BlinkingButton.call(this, options);

        this.previous = {};
        this.previous.volume = engine.getValue(this.group, "volume");
        this.previous.crossfader = engine.getValue("[Master]", "crossfader");
        this.enabled = false;
    };
    FaderStartToggleButton.prototype = deriveFrom(BlinkingButton, {
        inToggle: function() {
            BlinkingButton.prototype.output.call(this, this.enabled = !this.enabled);
        },
        connect: function() {
            if (undefined !== this.group) {
                this.connections[0] = engine.makeConnection(this.group, "volume", this.output.bind(this));
                this.connections[1] = engine.makeConnection("[Master]", "crossfader", this.output.bind(this));
            }
        },
        output: function(value, group, control) {
            if (!this.enabled) {
                return;
            }

            if (control === "volume" && value !== this.previous.volume) {
                if (this.previous.volume === 0 && value !== 0) {
                    engine.setValue(this.group, "play", 1);
                } else if (this.previous.volume !== 0 && value === 0) {
                    script.triggerControl(group, "cue_default");
                }
                this.previous.volume = value;
            }

            if (control === "crossfader" && value !== this.previous.crossfader) {
                const orientation = engine.getValue(this.group, "orientation") - 1; // -1 (L) / 0 (C) / 1 (R)
                const isOpposite  = Math.abs(orientation - value) === 2;
                const wasOpposite = Math.abs(orientation - this.previous.crossfader) === 2;
                if (isOpposite) {
                    script.triggerControl(this.group, "cue_default");
                } else if (wasOpposite) {
                    engine.setValue(this.group, "play", 1);
                }
                this.previous.crossfader = value;
            }
        },
    });

    /**
     * A button that triggers an effect.
     *
     * Button release is ignored by default;
     * the effect can be stopped by shortly tapping the jog wheel in touch mode.
     *
     * @class
     * @augments {components.Button}
     * @param {object} options Options object
     * @param {string} options.effect Name of the effect, one of: `brake`, `softStart`, `spinback`
     * @param {number} options.factor Factor for the effect; optional, default: 1
     * @param {number} options.rate Rate for the effect; optional, default: -10 for spinback
     * @param {boolean} options.stopOnRelease Stop effect on button release? optional; default: `false`
     * @public
     */
    const EffectButton = function(options) {
        const defaultOptions = {factor: 1};
        if (options.effect === "spinback") {
            defaultOptions.rate = -10;
            defaultOptions.factor += defaultOptions.rate; // workaround for broken spinback in 2.6
        }
        components.Button.call(this, Object.assign(defaultOptions, options));
        if (!this.effect || typeof(engine[this.effect]) !== "function") {
            log.error("Missing required `effect` option.");
        }
    };
    EffectButton.prototype = deriveFrom(components.Button, {
        input: function(_channel, _control, value, _status, group) {
            if (this.stopOnRelease || value > 0) {
                engine[this.effect].call(this, script.deckFromGroup(group), value, this.factor, this.rate);
            }
        }
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
                const beatjumpSize = engine.getValue(group, "beatjump_size");
                const action = () => {
                    script.triggerControl(group, "beatloop_activate");
                    engine.setValue(group, "beatjump_size", beatjumpSize);
                };
                const loopSize = engine.getValue(group, "beatloop_size");
                engine.setValue(group, "beatjump_size", loopSize);
                script.triggerControl(group, "beatjump_backward");
                /* Insert a small delay, required to let the loop start at the backjump position */
                new Timer({timeout: 20, oneShot: true, action: action, owner: this}).start();
            } else {
                script.triggerControl(group, "reloop_toggle");
            }
        }
    });

    /**
     * A pot controlling a component on all channels of a certain type.
     *
     * @class
     * @augments {components.Pot}
     * @param {object} options Options object
     * @param {string} options.channelType Channel type: one of [`Channel`, `Sampler`, `Auxiliary`, `Microphone`]
     * @param {string} options.inKey Mixxx control; optional, default: `pregain`
     * @public
     */
    const SuperPot = function(options) {
        options = options || {};
        if (!options.channelType) {
            log.error("A SuperPot requires a `channelType`.");
            return;
        }
        if (options.key === undefined && options.inKey === undefined) {
            options.inKey = "pregain";
        }
        components.Pot.call(this, options);
        this.groups = enumerateChannelGroups(options.channelType);
    };
    SuperPot.prototype = deriveFrom(components.Pot, {
        inSetParameter: function(value) {
            this.groups.forEach(group => engine.setParameter(group, this.inKey, value));
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
     * @typedef {components.ComponentContainer} JogWheelUnit
     *
     * @property {components.Button} touch Touch Button
     * @property {components.Encoder} jog Jog Encoder
     * @property {object} vinylMode Vinyl mode options
     * @property {components.Button} vinylMode.toggle Button to toggle vinyl mode
     * @property {components.Button} vinylMode.touch Touch button in vinyl mode
     * @property {components.Encoder} vinylMode.jog Jog encoder in vinyl mode
     */

    /**
     * A component container for jog wheel controls.
     *
     * @class
     * @augments {components.ComponentContainer}
     * @param {object} options Options object
     * @yields {JogWheelUnit}
     * @public
     */
    const JogWheelUnit = function(options) {
        components.ComponentContainer.call(this);

        /* JogWheelBasic (core component) */
        const JogWheelBasic = function(jogWheelBasicOptions) {
            jogWheelBasicOptions = Object.assign({group: options.group}, jogWheelBasicOptions);
            components.JogWheelBasic.call(this, jogWheelBasicOptions);
        };
        JogWheelBasic.prototype = deriveFrom(components.JogWheelBasic, options.overrides || {});
        const jogWheelBasic = new JogWheelBasic(options.basic);

        /* Touch Button */
        const TouchButton = function(touchButtonOptions) {
            components.Button.call(this, touchButtonOptions);
        };
        TouchButton.prototype = deriveFrom(components.Button, {
            input: jogWheelBasic.inputTouch.bind(jogWheelBasic),
        });

        /* Jog Encoder */
        const JogEncoder = function(jogEncoderOptions) {
            components.Component.call(this, jogEncoderOptions);
        };
        JogEncoder.prototype = deriveFrom(components.Encoder, {
            input: jogWheelBasic.inputWheel.bind(jogWheelBasic),
        });

        /* Vinyl Mode Button */
        const VinylModeButton = function(vinylModeButtonOptions) {
            components.Button.call(this, vinylModeButtonOptions);
        };
        VinylModeButton.prototype = deriveFrom(components.Button, {
            input: function(_channel, _control, value, _status, _group) {
                jogWheelBasic.vinylMode = value;
            }
        });

        const jogOptionsWith = midi => Object.assign({midi: midi}, {group: options.group, inKey: "jog"});
        const midi = options.midi;
        this.touch = new TouchButton(jogOptionsWith(midi.touch));
        this.jog = new JogEncoder(jogOptionsWith(midi.jog));
        this.vinylMode = new components.ComponentContainer();
        this.vinylMode.toggle = new VinylModeButton(jogOptionsWith(midi.vinylMode.toggle));
        this.vinylMode.touch  = new TouchButton(jogOptionsWith(midi.vinylMode.touch));
        this.vinylMode.jog    = new JogEncoder(jogOptionsWith(midi.vinylMode.jog));
    };
    JogWheelUnit.prototype = deriveFrom(components.ComponentContainer);

    /**
     * A component container for loop move controls.
     *
     * Press the shift button to switch between two layers:
     * a) turn encoder to jump X beats back or forth, or move any active loop by Y beats
     * b) press & turn to adjust the beatjump/loopmove size
     *
     * @class
     * @augments {components.ComponentContainer}
     * @param {object} options Options object
     * @param {object} options.shiftButton Option object for the button that toggles between layers
     * @param {object} options.encoder Option object for the loop move encoder
     * @param {object} options.button Option object for the button that toggles a loop
     * @yields {LoopMoveUnit}
     * @public
     */
    const LoopMoveUnit = function(options) {
        components.ComponentContainer.call(this);

        const withGroup = object => Object.assign({group: options.group}, object);

        const layerManager = new LayerManager({debug: this.debug});
        const shiftButton = new ShiftButton(withGroup(Object.assign(options.shiftButton, {target: layerManager})));
        const encoder = new LoopMoveEncoder(withGroup(options.encoder));
        const buttonOptions = Object.assign({inKey: "beatjump_size_double", inKeyOff: "beatjump_size_halve"}, options.button);
        const button = new TriggerButton(withGroup(buttonOptions));
        layerManager.registerComponents({}, shiftButton);
        layerManager.registerComponents({}, encoder);
        layerManager.registerComponents({shift: true}, button);
        layerManager.init();
    };
    LoopMoveUnit.prototype = deriveFrom(components.ComponentContainer);

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
         * Register a component with all its child components.
         *
         * @param {object} definition Definition of a component
         * @param {components.Component|components.ComponentContainer} implementation Implementation of a component
         * @private
         */
        registerComponents: function(definition, implementation) {
            if (implementation instanceof components.Component) {
                this.register(implementation, definition && definition.shift === true);
            } else if (implementation instanceof components.ComponentContainer) {
                Object.keys(implementation).forEach(function(name) {
                    const definitionName = definition ? definition[name] : null;
                    this.registerComponents(definitionName, implementation[name]);
                }, this);
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
        input: function(channel, control, value, status, _group) {
            const component = this.findComponent(status, control);
            if (component === undefined) {
                return;
            }
            if (component.input !== undefined && typeof this.input === "function") {
                component.input(channel, control, value, status, component.group);
            } else {
                log.error(`Component without input function: ${global.stringifyObject(component)}`);
            }
        },
    });

    /**
     * Resolves template definitions to ComponentContainer definitions.
     *
     * A template definition is an ordinary ComponentContainer definition containing a top-level `bindings` array.
     * Each element is a map of JS `Symbol`s to a arbitrary values.
     *
     * For each element, a separate ComponentContainer is created,
     * having all symbol occurrences replaced by the respective value.
     *
     * Example: the template
     *  {
     *    bindings: [ {[$group]: "[Channel1]", [$control]: 0x01}, {[$group]: "[Channel2]", [$control]: 0x02} ],
     *    components: [{ type: Button, options: {group: $group, midi: [0x90, $control], key: "foo"} }]
     *  }
     *
     * is resolved to 2 ComponentContainers:
     *  [
     *    { components: [{ type: Button, options: {group: "[Channel1]", midi: [0x90, 0x01], key: "foo"} }] },
     *    { components: [{ type: Button, options: {group: "[Channel2]", midi: [0x90, 0x02], key: "foo"} }] }
     *  ]
     *
     * @class
     * @private
     */
    const TemplateResolver = function() {};
    TemplateResolver.prototype = {

        /**
         * Resolve an array of template definitions into an array of ComponentContainer definitions.
         *
         * @param {object} parent An object containing an array of template definitions
         * @param {string} name Name of the property holding the array of template definitions
         * @returns {Array} Container definitions built from the template definitions
         * @public
         */
        resolveTemplates: function(parent, name) {
            const templateDefinitions = parent[name] || [];
            const nested = templateDefinitions.map((def, index) => this.resolveTemplate(def, `${name}.${index}`));
            const containerDefinitions = [].concat(...nested);
            log.debug(`Resolved ${containerDefinitions.length} definitions for ${name}`);
            return containerDefinitions;
        },

        /**
         * Resolve a single template definition to an array of ComponentContainer definitions.
         *
         * @param {object} templateDefinition A template definition
         * @param {string} name Name of the template definition (for debugging only)
         * @returns {object} An array of ComponentContainer definitions.
         *                   If the template definition does not contain a `bindings` configuration,
         *                   the output array contains exactly the input object.
         * @private
         */
        resolveTemplate: function(templateDefinition, name) {
            if (!Array.isArray(templateDefinition.bindings)) {
                return [templateDefinition];
            }
            return templateDefinition.bindings.map((binding, index) => {
                const containerDefinition = mergeDefinitions({}, templateDefinition);
                delete containerDefinition.bindings;
                this.resolveVariables(containerDefinition, binding, `${name}.${index}`);
                return containerDefinition;
            });
        },

        /**
         * Replaces all symbols within an object and its children with values.
         *
         * @param {object} parent An object containing `Symbol`s (directly or transitively)
         * @param {object} values A map of `Symbol` -> value pairs
         * @param {string} path A path to the parent object (for debugging only); optional
         * @private
         */
        resolveVariables: function(parent, values, path = "") {
            if (typeof parent !== "object" || parent === null) {
                return;
            }
            Object.keys(parent).forEach(function(name) {
                const childPath = path + (path ? "." : "") + name;
                const child = parent[name];
                if (typeof child === "symbol") {
                    const value = values[child];
                    parent[name] = value;
                    log.debug(`${childPath} = ${value}`);
                } else {
                    /* Avoid endless recursion on object references (e.g. ShiftButton) */
                    if (child instanceof components.ComponentContainer || child instanceof components.Component) {
                        log.debug(`Skipping reference to ${path}.${name}`);
                    } else {
                        this.resolveVariables(child, values, childPath);
                    }
                }
            }, this);
        },
    };

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
     *     |     |  +- type: (function, optional) Constructor; default: `EqualizerUnit`
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
     *     |     |  +- type: (function, optional) Constructor; default: `QuickEffectUnit`
     *     |     |  +- midi: As described for equalizer unit using `components.QuickEffectUnit` instead of
     *     |     |  |        `EqualizerUnit`. Examples:
     *     |     |  |          `enabled: [0x90, 0x02]`
     *     |     |  |          `super1: [0xB0, 0x06]`
     *     |     |  +- feedback: As described for equalizer unit
     *     |     |  +- feedbackOnRelease: As described for equalizer unit
     *     |     |  +- output: As described for equalizer unit
     *     |     +- jogWheelUnit: Jog wheel unit definition (optional)
     *     |        +- type: (function, optional) Constructor; default: `JogWheelUnit`
     *     |        +- options:
     *     |           +- midi: An object of component definitions for the jog wheel.
     *     |           |  +- touch: MIDI address of the touch button of the wheel
     *     |           |  +- jog:   MIDI address of the jog encoder of the wheel
     *     |           |  +- vinylMode: MIDI addresses for vinyl mode
     *     |           |  |  +- toggle: MIDI address of the button that toggles vinyl mode
     *     |           |  |  +- touch:  MIDI address of the touch button of the wheel in vinyl mode
     *     |           |  |  +- jog:    MIDI address of the jog encoder of the wheel in vinyl mode
     *     |           +- basic: options for `components.JogWheelBasic`, e.g. wheelResolution, alpha, beta, rpm
     *     |           +- overrides: (optional) object containing custom functions and properties for the jog wheel
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

            const templateResolver = new TemplateResolver();
            this.layerManager = this.createLayerManager(
                this.componentContainers,
                templateResolver.resolveTemplates(this.config, "decks"),
                templateResolver.resolveTemplates(this.config, "effectUnits"),
                templateResolver.resolveTemplates(this.config, "containers")
            );

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
            const registerComponents = function(definition, implementation) {
                layerManager.registerComponents(definition, implementation);
            };

            [{
                definitions: deckDefinitions,
                factory: this.createDeck,
                register: function(deckDefinition, deckImplementation) {
                    registerComponents(deckDefinition.components, deckImplementation);
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
            const deckDefaultDefinition = Object.assign({}, deckDefinition.defaultDefinition);
            const deckFactory = function(options) {
                const deck = new components.Deck(options);
                Object.assign(deckDefaultDefinition, {options: {group: deck.currentDeck}});
                return deck;
            }
            const options = Object.assign({}, deckDefinition, {
                type: deckFactory,
                options: deckDefinition.deckNumbers,
                defaultDefinition: deckDefaultDefinition,
            });
            const deck = this.createComponentContainer(options);

            const jogWheelUnitOptions = Object.assign({group: deck.currentDeck}, (deckDefinition.jogWheelUnit || {}).options);

            [
                {property: "equalizerUnit",   type: EqualizerUnit,   options: deck.currentDeck},
                {property: "quickEffectUnit", type: QuickEffectUnit, options: deck.currentDeck},
                {property: "jogWheelUnit",    type: JogWheelUnit,    options: jogWheelUnitOptions},
            ].forEach(function(context) {
                const definition = deckDefinition[context.property];
                if (definition) {
                    const type = definition.type || context.type;
                    deck[context.property] = this.setupMidi(
                        definition,
                        new type(context.options),
                        componentStorage);
                }
            }, this);

            return deck;
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
            processMidiAddresses(definition.midi, implementation,
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
                processMidiAddresses(definition.output, implementation,
                    (midi, component) => this.createPublisher(
                        {midi: midi, group: component.group, inKey: component.inKey}, publisherStorage)
                );
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
            const controller = this;
            const effectUnitFactory = function() {
                const effectUnit = controller.setupMidi(
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
                return effectUnit;
            };
            const options = mergeDefinitions({}, effectUnitDefinition, {type: effectUnitFactory});
            const effectUnit = this.createComponentContainer(options);
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
                containerDefinition.init.call(container, containerDefinition);
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
    });

    const exports = {};
    exports.deriveFrom = deriveFrom;
    exports.processMidiAddresses = processMidiAddresses;
    exports.enumerateChannelGroups = enumerateChannelGroups;
    exports.ParameterComponent = ParameterComponent;
    exports.ShiftButton = ShiftButton;
    exports.Trigger = Trigger;
    exports.TriggerButton = TriggerButton;
    exports.CustomButton = CustomButton;
    exports.Timer = Timer;
    exports.LongPressButton = LongPressButton;
    exports.KeyButton = KeyButton;
    exports.TrackLoadButton = TrackLoadButton;
    exports.BlinkingButton = BlinkingButton;
    exports.BlinkingSamplerButton = BlinkingSamplerButton;
    exports.DirectionEncoder = DirectionEncoder;
    exports.SteppingEncoder = SteppingEncoder;
    exports.RangeAwareEncoder = RangeAwareEncoder;
    exports.RangeAwarePot = RangeAwarePot;
    exports.EnumToggleButton = EnumToggleButton;
    exports.EnumEncoder = EnumEncoder;
    exports.Rot64Encoder = Rot64Encoder;
    exports.LoopEncoder = LoopEncoder;
    exports.LoopMoveEncoder = LoopMoveEncoder;
    exports.FaderStartToggleButton = FaderStartToggleButton;
    exports.EffectButton = EffectButton;
    exports.SuperPot = SuperPot;
    exports.BackLoopButton = BackLoopButton;
    exports.CrossfaderCurvePot = CrossfaderCurvePot;
    exports.Publisher = Publisher;
    exports.JogWheelUnit = JogWheelUnit;
    exports.LoopMoveUnit = LoopMoveUnit;
    exports.LayerManager = LayerManager;
    exports.GenericMidiController = GenericMidiController;
    global.behringer = Object.assign(global.behringer || {}, {extension: exports});
})(this);
