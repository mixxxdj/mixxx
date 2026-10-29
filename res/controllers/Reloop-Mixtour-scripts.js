/*
Reloop Mixtour controller script
*/

// eslint-disable-next-line no-var
var ReloopMixtour = {};

// midi-components-0.0.js does not prevent re-connecting some already connected
// component. The following mixin and the local classes solve this problem.
// Additionally some shutdown method added to all local components.
ReloopMixtour.controlledConnections = (Base) => class extends Base {
    connect() {
        if (this.connections[0] === undefined) {
            super.connect();
        };
    };
    disconnect() {
        super.disconnect();
        this.connections = [];
    };
    shutdown() {
        if (super.shutdown !== undefined) { super.shutdown(); }
        this.disconnect();
    };
};
ReloopMixtour.Button = class extends ReloopMixtour.controlledConnections(components.Button) {};
ReloopMixtour.PlayButton = class extends ReloopMixtour.controlledConnections(components.PlayButton) {};
ReloopMixtour.CueButton = class extends ReloopMixtour.controlledConnections(components.CueButton) {};
ReloopMixtour.HotcueButton = class extends ReloopMixtour.controlledConnections(components.HotcueButton) {};
ReloopMixtour.Pot = class extends ReloopMixtour.controlledConnections(components.Pot) {};
ReloopMixtour.Component = class extends ReloopMixtour.controlledConnections(components.Component) {};

ReloopMixtour.init = function() {
    // initialize Mixxx with current values on control surface
    const ControllerStatusSysex = [0xF0, 0x26, 0x2D, 0x65, 0x22, 0xF7];
    midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);

    this.syncGroupSelection = function(elem, type, group) {
        if (engine.getValue("[Skin]", `highlight_${type}_${group}`) && elem.currentDeck !== group) {
            elem.setCurrentDeck(group);
        }
    };

    this.reSelectDecks = function() {
        this.syncGroupSelection(this.decks.right, "deck", "[Channel4]");
        this.syncGroupSelection(this.decks.left, "deck", "[Channel3]");
        this.syncGroupSelection(this.decks.right, "deck", "[Channel2]");
        this.syncGroupSelection(this.decks.left, "deck", "[Channel1]");
    };

    this.reSelectMixer = function() {
        this.syncGroupSelection(this.mixer.right, "mixer", "[Channel4]");
        this.syncGroupSelection(this.mixer.left, "mixer", "[Channel3]");
        this.syncGroupSelection(this.mixer.right, "mixer", "[Channel2]");
        this.syncGroupSelection(this.mixer.left, "mixer", "[Channel1]");
    };

    this.connectFourDeckControl = function() {
        for (let i = 1; i <= 4; i++) {
            this[`highlightDeckConnection_[Channel${i}]`] = engine.makeConnection("[Skin]", `highlight_deck_[Channel${i}]`, function(_value, _group, _control) {
                ReloopMixtour.reSelectDecks();
            });
            this[`highlightMixerConnection_[Channel${i}]`] = engine.makeConnection("[Skin]", `highlight_mixer_[Channel${i}]`, function(_value, _group, _control) {
                ReloopMixtour.reSelectMixer();
            });
        }
    };

    this.disconnectFourDeckControl = function() {
        for (let i = 1; i <= 4; i++) {
            if (this[`highlightDeckConnection_[Channel${i}]`] !== undefined) {
                this[`highlightDeckConnection_[Channel${i}]`].disconnect();
            };
            if (this[`highlightMixerConnection_[Channel${i}]`] !== undefined) {
                this[`highlightMixerConnection_[Channel${i}]`].disconnect();
            };
        };
    };

    const leftDeckConfig = parseInt(engine.getSetting("leftControls"));
    const rightDeckConfig = parseInt(engine.getSetting("rightControls"));
    this.decks = new components.ComponentContainer({
        left: new ReloopMixtour.Deck(leftDeckConfig || [1, 3], 0),
        right: new ReloopMixtour.Deck(rightDeckConfig || [2, 4], 1),
    });
    this.mixer = new components.ComponentContainer({
        left: new ReloopMixtour.Mixer(leftDeckConfig || [1, 3], 0),
        right: new ReloopMixtour.Mixer(rightDeckConfig || [2, 4], 1),
    });
    if (!leftDeckConfig || !rightDeckConfig) {
        this.connectFourDeckControl();
    }

    this.shutdown = function() {
        this.decks.shutdown();
        this.mixer.shutdown();
        this.disconnectFourDeckControl();
        this.backBtn.shutdown();
    };

    // there is one shift button, map it to the decks
    this.shiftButton = new ReloopMixtour.Button({
        input: function(_channel, _control, value, _status, _g) {
            if (value) {
                ReloopMixtour.decks.shift();
                ReloopMixtour.mixer.shift();
            } else {
                ReloopMixtour.decks.unshift();
                ReloopMixtour.mixer.unshift();
            }
        },
    });

    this.backBtn = new ReloopMixtour.Button({
        midi: [0x90, 0x08],
        group: "[Skin]",
        key: "show_maximized_library",
        type: components.Button.prototype.types.toggle,
    });

    // the rotarySelector only works if mixxx windows is focused, don't be surprised :)
    this.rotarySelector = new ReloopMixtour.Button({
        group: "[Library]",
        input: function(channel, control, value, status, group) {
            if (value === 0x01) {
                this.inKey = (control < 0x40)?"MoveDown":"MoveRight";
            } else {
                this.inKey = (control < 0x40)?"MoveUp":"MoveLeft";
            }
            components.Button.prototype.input.call(this, channel, control, 0x7F, status, group);
        },
        inputPress: function(channel, control, value, status, group) {
            this.inKey = "MoveFocusForward";
            components.Button.prototype.input.call(this, channel, control, value, status, group);
        },
    });
};

ReloopMixtour.Deck = class extends components.Deck {
    constructor(deckNumbers, midiChannel) {
        super(deckNumbers);

        const theDeck = this;

        this.load = new ReloopMixtour.Button({
            midi: [0x90 + midiChannel, 0x02],
            key: "LoadSelectedTrack",
            shiftKey: midiChannel?"MoveRight":"MoveLeft",
            shiftOffset: 0x40,
            unshift: function() {
                this.inKey = "LoadSelectedTrack";
                this.group = theDeck.currentDeck;
            },
            shift: function() {
                this.inKey = this.shiftKey;
                this.group = "[Library]";
            },
        });

        this.pfl = new ReloopMixtour.Button({
            midi: [0x90 + midiChannel, 0x03],
            key: "pfl",
            shiftOffset: 0x40,
            type: components.Button.prototype.types.toggle,
            unshift: function() {
                this.inKey = "pfl";
                this.group = theDeck.currentDeck;
            },
            shift: function() {
                this.inKey = "LoadSelectedTrackAndPlay";
                this.group = "[PreviewDeck1]";
            },
        });

        this.loop = new ReloopMixtour.Button({
            midi: [0x90 + midiChannel, 0x09],
            inKey: "beatloop_activate",
            outKey: "loop_enabled",
            unshift: function() {
                this.inKey = "beatloop_activate";
            },
            shift: function() {
                this.inKey = "loop_halve";
            },
        });

        this.beatSync = new ReloopMixtour.Button({
            midi: [0x90 + midiChannel, 0x0A],
            type: components.Button.prototype.types.toggle,
            key: "sync_enabled",
            unshift: function() {
                this.inKey = "sync_enabled";
            },
            shift: function() {
                this.inKey = "loop_double";
            },
        });

        this.play = new ReloopMixtour.PlayButton({
            midi: [0x90 + midiChannel, 0x0C],
        });

        this.cue = new ReloopMixtour.CueButton({
            midi: [0x90 + midiChannel, 0x0B],
        });

        for (let i = 1; i <= 4; i++) {
            this[`hotcue${i}`] = new ReloopMixtour.HotcueButton({
                midi: [0x90 + midiChannel, 0x0C + i],
                number: i,
                on: 0x2B,
            });
        };

        this.forEachComponent(function(component) {
            if (component.group === undefined) {
                component.group = this.currentDeck;
            };
            if (component.shiftOffset === 0) {
                component.shiftOffset = 0x3F;
            };
            component.shiftControl = true;
            component.sendShifted = true;
            if (component.on === 127) { // value not changed from default
                component.on = 0x01;    // color for most buttons is green
            }
        });
    }
};

ReloopMixtour.Mixer = class extends components.Deck {
    constructor(deckNumbers, midiChannel) {
        super(deckNumbers);

        const theMixer = this;

        this.gain = new ReloopMixtour.Pot({
            key: "pregain",
        });

        this.fxKnob = new ReloopMixtour.Pot({
            key: "super1",
            group: `[QuickEffectRack1_${theMixer.currentDeck}]`,
        });

        // loaded_chain_preset - values are a following:
        // 0 is the empty/passthrough preset
        // -1 indicates an unsaved preset (no idea if this index can be selected)
        // 1 should be the default (Filter) preset. This can be configured by the user in the pref dialog
        this.effect = new ReloopMixtour.Button({
            midi: [0x90 + midiChannel, 0x01],
            shiftOffset: 0x40,
            key: "loaded_chain_preset",
            secondOutKey: "enabled",
            group: `[QuickEffectRack1_${theMixer.currentDeck}]`,
            // set preset to first real fx. if available at all this is number 2
            fxPreset: (engine.getValue(`[QuickEffectRack1_${theMixer.currentDeck}]`, "num_chain_presets") > 2)?2:0,
            nextPreset: function() {
                const nChainPresets = engine.getValue(this.group, "num_chain_presets");
                this.fxPreset = (this.fxPreset + 1) % nChainPresets;
                if (nChainPresets > 2 && this.fxPreset < 2) {
                    this.fxPreset = 2; // set preset to first real fx.
                }
            },
            input: function(_channel, control, value, _status, _group) {
                // only act on button-press and if fx are available
                if (value && engine.getValue(this.group, "num_chain_presets") > 2) {
                    if (control < this.shiftOffset) {
                        const tmpSuper1Value = engine.getValue(this.group, "super1");
                        if (this.inGetValue() !== 1) {                  // current FX is not the 'Filter'
                            this.fxPreset = this.inGetValue() || 2;     // store current FX (but not 0)
                            this.inSetValue(1);                         // activate the 'Filter'
                        } else {
                            this.inSetValue(this.fxPreset);             // activate last stored FX
                        }
                        if (this.lastSuper1Value !== undefined) {       // activate last stored super1
                            engine.softTakeover(this.group, "super1", false);
                            engine.setValue(this.group, "super1", this.lastSuper1Value);
                            engine.softTakeover(this.group, "super1", true);
                        }
                        this.lastSuper1Value = tmpSuper1Value;
                    } else if (this.inGetValue() !== 1) {               // if FX is active
                        this.nextPreset();                          // select and store next FX
                        this.inSetValue(this.fxPreset);             // activate (new) FX
                    }
                }
            },
            outValueScale: function(_value) {
                // activate FX led if any FX is selected (>2) and enabled
                return (this.outGetValue() >= 2 && engine.getValue(this.group, this.secondOutKey)) ? this.on : this.off;
            },
            connect: function() {
                // use two connections for the LED, one to check for any real FX (id > 2) and the other to check
                // if effect is enabled
                if (this.connections[0] === undefined) {
                    this.connections.push(engine.makeConnection(this.group, this.outKey, this.output.bind(this)));
                    this.connections.push(engine.makeConnection(this.group, this.secondOutKey, this.output.bind(this)));
                }
            },
        });

        this.highEq = new ReloopMixtour.Pot({
            key: "parameter3",
            group: `[EqualizerRack1_${theMixer.currentDeck}_Effect1]`,
        });

        this.midEq = new ReloopMixtour.Pot({
            key: "parameter2",
            group: `[EqualizerRack1_${theMixer.currentDeck}_Effect1]`,
        });

        this.lowEq = new ReloopMixtour.Pot({
            key: "parameter1",
            group: `[EqualizerRack1_${theMixer.currentDeck}_Effect1]`,
        });

        this.volumeFader = new ReloopMixtour.Pot({
            inKey: "volume",
        });

        this.faderStart = new ReloopMixtour.Button({
            key: "cue_gotoandplay",
        });

        this.faderStop = new ReloopMixtour.Button({
            key: "cue_gotoandstop",
        });

        this.pflMeter = new ReloopMixtour.Component({
            midi: [0x90 + midiChannel, 0x11],
            key: "vu_meter",
            max: 0x68,
            disconnect: function() {
                components.Component.prototype.disconnect.call(this);
                this.output(0);
            },
        });

        this.masterMeter = new ReloopMixtour.Component({
            midi: [0x90 + midiChannel, 0x12],
            key: (this.currentDeck === "[Channel1]" || this.currentDeck === "[Channel3]")?"vu_meter_left":"vu_meter_right",
            group: "[Main]",
            max: 0x68
        });

        this.fxIndicator = new ReloopMixtour.Component({
            midi: [0x90 + midiChannel, 0x00],
            key: "super1",
            secondOutKey: "enabled",
            group: `[QuickEffectRack1_${theMixer.currentDeck}]`,
            outValueScale() {
                const super1Value = engine.getValue(this.group, this.outKey);
                const enabledValue = engine.getValue(this.group, this.secondOutKey);
                return (enabledValue && (Math.abs(0.5 - super1Value) > 0.01))?0x7F:0x00;
            },
            connect: function() {
                // use two connections for the LED, one to check for any real FX (id > 2) and the other to check
                // if effect is enabled
                if (this.connections[0] === undefined) {
                    this.connections.push(engine.makeConnection(this.group, this.outKey, this.output.bind(this)));
                    this.connections.push(engine.makeConnection(this.group, this.secondOutKey, this.output.bind(this)));
                }
            },
        });

        this.forEachComponent(function(component) {
            if (component.group === undefined) {
                component.group = this.currentDeck;
            };
            if (component.shiftOffset === 0) {  // if value not changed from default
                component.shiftOffset = 0x3F;   // offset for most buttons is 0x3F
            };
            component.shiftControl = true;
            component.sendShifted = true;
            if (component.on === 0x7F) {        // if value not changed from default
                component.on = 0x01;            // color for most buttons is green
            }
        });
    }
};
