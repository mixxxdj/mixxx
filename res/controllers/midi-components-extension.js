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
    LayerManager.prototype = new components.Component({
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
            /* undefined when called from Component constructor on prototype creation */
            if (this.activeLayer !== undefined) {
                log.debug("LayerManager.unshift()");
                this.activeLayer.applyLayer(this.defaultLayer());
            }
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
    ShiftButton.prototype = new components.Button({
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
    Trigger.prototype = new components.Component({
        inValueScale: function() { return true; },
    });

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
    RangeAwareEncoder.prototype = new components.Encoder({
        outValueScale: function(value) {
            /* -bound..+bound => 0..1 */
            var normalizedValue = (value + this.bound) / (2 * this.bound);
            /* 0..1 => 0..127 */
            return convertToMidiValue.call(this, normalizedValue);
        },
    });

    /**
     * A button to cycle through the values of an enumeration [0..maxValue]
     *
     * @constructor
     * @extends {components.Button}
     * @param {object} options Options object
     * @param {number} options.maxValue A positive integer defining the maximum enumeration value
     * @public
     */
    var EnumToggleButton = function(options) {
        options = options || {};
        if (options.maxValue === undefined) {
            log.error("EnumToggleButton constructor was called without specifying max value.");
            this.maxValue = 0;
        }
        components.Button.call(this, options);
    };
    EnumToggleButton.prototype = new components.Button({
        input: function(_channel, _control, _value, _status, _group) {
            this.inSetValue((this.inGetValue() + 1) % this.maxValue);
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
    EnumEncoder.prototype = new components.Encoder({
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
    LoopEncoder.prototype = new EnumEncoder({values: []});

    /**
     * An encoder that moves a loop.
     *
     * Turning the encoder to the right will move the loop forwards; turning it to the left will
     * move it backwards. The amount of movement may be given by either 'size' or 'sizeControl',
     * 'sizeControl' being preferred.
     *
     * @constructor
     * @extends {components.Encoder}
     * @param {object} options Options object
     * @param {number} options.size (optional) Size given in number of beats; default: 0.5
     * @param {string} options.sizeControl (optional) Name of a control that contains 'size'
     * @public
     */
    var LoopMoveEncoder = function(options) {
        options = options || {};
        options.inKey = options.inKey || "loop_move";
        options.size = options.size || 0.5;
        components.Encoder.call(this, options);
        this.previousValue = this.inGetValue(); // available only after call of Encoder constructor
    };
    LoopMoveEncoder.prototype = new components.Encoder({
        min: 0,
        inValueScale: function(value) {
            var direction = 0;
            if (value > this.previousValue || value === this.max) {
                direction = 1;
            } else if (value < this.previousValue || value === this.min) {
                direction = -1;
            }
            this.previousValue = value;

            var beats = this.sizeControl ? global.engine.getValue(this.group, this.sizeControl)
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
    BackLoopButton.prototype = new components.Button({
        input: function(_channel, _control, value, _status, group) {
            var engine = global.engine;
            var script = global.script;
            if (value) {
                var loopSize = engine.getValue(group, "beatloop_size");
                var beatjumpSize = engine.getValue(group, "beatjump_size");
                engine.setValue(group, "beatjump_size", loopSize);
                script.triggerControl(group, "beatloop_activate");
                script.triggerControl(group, "beatjump_backward");
                engine.setValue(group, "beatjump_size", beatjumpSize);
            } else {
                script.triggerControl(group, "reloop_toggle");
            }
        }
    });

    /**
     * A component that sends the values of a source component to a MIDI controller even if the
     * source component uses its 'outKey' property for other purposes.
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
    Publisher.prototype = new components.Component({
        outValueScale: function(_value) {
            /*
             * We ignore the argument and use the parameter (0..1) instead because value scale is
             * arbitrary and thus cannot be mapped * to MIDI values (0..127) properly.
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

    var exports = {};
    exports.LayerManager = LayerManager;
    exports.ShiftButton = ShiftButton;
    exports.Trigger = Trigger;
    exports.RangeAwareEncoder = RangeAwareEncoder;
    exports.EnumToggleButton = EnumToggleButton;
    exports.EnumEncoder = EnumEncoder;
    exports.LoopEncoder = LoopEncoder;
    exports.LoopMoveEncoder = LoopMoveEncoder;
    exports.BackLoopButton = BackLoopButton;
    exports.Publisher = Publisher;
    global.components.extension = exports;
})(this);
