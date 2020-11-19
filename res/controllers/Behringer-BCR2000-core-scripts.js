/**
 * Core functions for the mapping of a Behringer B-Control BCR2000 controller.
 */

/* BCR2000 is a ComponentContainer to be used as target for (un)shift operations */
var BCR2000 = new components.ComponentContainer();

/*
 * Contains all decks and effect units so that a (un)shift operation on BCR2000
 * is delegated to the decks, effect units and their children.
 */
BCR2000.componentContainers = [];

/**
 * Initialize the controller mapping.
 * This function is called by Mixxx on startup.
 *
 * @param {string} controllerId Controller-ID
 * @param {boolean} debug Is the application in debug mode?
 * @public
 */
BCR2000.init = function(controllerId, debug) {
    this.controllerId = controllerId;
    this.debug = debug;
    if (!this.userConfig) {
        print(this.controllerId + ": ERROR: Missing configuration. "
        + "Please check if the configuration file 'Behringer-BCR2000-configuration-scripts.js' "
        + "exists and contains the object 'BCR2000.userConfig'.");
        return;
    }
    this.config = this.userConfig();

    if (BCR2000.preset && this.config.presets.onInit) {
        BCR2000.preset.setPreset(this.config.presets.onInit);
    }
    this.layerManager = this.createLayerManager(
        this.config.decks, this.config.effectUnits, this.componentContainers);

    this.log("init() completed.");
};


/**
 * Shutdown the controller mapping.
 * This function is called by Mixxx on shutdown.
 *
 * @public
 */
BCR2000.shutdown = function() {
    this.layerManager.destroy();
    if (BCR2000.preset && this.config.presets.onShutdown) {
        BCR2000.preset.setPreset(this.config.presets.onShutdown);
    }

    this.log("shutdown() completed.");
};

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
BCR2000.input = function(channel, control, value, status, group) {
    return this.layerManager.input(channel, control, value, status, group);
};

/**
 * Create a layer manager containing the components of all decks and effect units.
 *
 * @param {object} deckDefinitions Definition of decks
 * @param {object} effectUnitDefinitions Definition of effect units
 * @param {Array} componentContainers Target for decks and effect units
 * @return {object} Layer manager
 * @see `components.extension.LayerManager`
 * @private
 */
BCR2000.createLayerManager = function(deckDefinitions, effectUnitDefinitions, componentContainers) {
    var layerManager = new components.extension.LayerManager({debug: this.debug});
    [
        {definitions: deckDefinitions, factory: this.createDeck},
        {definitions: effectUnitDefinitions, factory: this.createEffectUnit}
    ].forEach(function(context) {
        context.definitions.forEach(function(definition) {
            var implementation = context.factory.call(this, definition);
            componentContainers.push(implementation);
            this.registerComponents(layerManager, definition.components, implementation);
        }, this);
    }, this);
    layerManager.init();
    return layerManager;
};

/**
 * Create a deck.
 *
 * @param {object} deckDefinition Definition of the deck
 * @return {object} The new deck
 * @private
 */
BCR2000.createDeck = function(deckDefinition) {
    var deck = new components.Deck(deckDefinition.deckNumbers);
    deckDefinition.components.forEach(function(componentDefinition, index) {
        var options = _.merge({
            group: deck.currentDeck,
            midi: componentDefinition.midi
        }, componentDefinition.options);
        deck[index] = new componentDefinition.type(options);
    }, this);
    return deck;
};

/**
 * Create an effect unit.
 *
 * In addition to the implementation of `components.EffectUnit`, output values of effect unit
 * components are sent to the controller.
 *
 * @param {object} effectUnitDefinition Definition of the effect unit
 * @return {object} The new effect unit
 * @private
 */
BCR2000.createEffectUnit = function(effectUnitDefinition) {
    var unit = new components.EffectUnit(effectUnitDefinition.unitNumbers, true);

    /* Convert MIDI addresses (number array) to objects containing a 'midi' property */
    var midify = function(object) {
        return Array.isArray(object) ? {midi: object} : Object.keys(object).reduce(
            function(result, name) { result[name] = midify(object[name]); return result; }, {});
    };
    _.merge(unit, midify(effectUnitDefinition.components));

    /* Add support for sending output values to the controller */
    unit.forEachComponent(function(effectComponent) {
        var prototype = Object.getPrototypeOf(effectComponent);
        var publisher = new components.extension.Publisher({source: effectComponent});
        ["onFocusChange", "shift", "unshift"].forEach(function(functionName) {
            var delegate = prototype[functionName];
            if (typeof delegate === "function") {
                prototype[functionName] = function() {
                    delegate.apply(this, arguments);
                    publisher.bind();
                };
            }
        });
    });

    unit.init();
    return unit;
};


/**
 * Register a component with all its child components in the layer manager.
 *
 * @param {components.extension.LayerManager} layerManager Layer manager
 * @param {object} definition Definition of a component
 * @param {components.Component|components.ComponentContainer} Implementation of a component
 * @private
 */
BCR2000.registerComponents = function(layerManager, definition, implementation) {
    if (implementation instanceof components.Component) {
        layerManager.register(implementation, definition.shift === true);
    } else if (implementation instanceof components.ComponentContainer) {
        Object.keys(definition).forEach(function(name) {
            this.registerComponents(layerManager, definition[name], implementation[name]);
        }, this);
    }
};

/**
 * Log a debug message to the console.
 *
 * @param {string} message Message
 * @private
 */
BCR2000.log = function(message) {
    if (this.debug) {
        print(this.controllerId + ": " + message);
    }
};
