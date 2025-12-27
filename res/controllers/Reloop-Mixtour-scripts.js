/*
Reloop Mixtour controller script
*/

// most buttons use the following shift settings, its easier to correct them for the few others
components.Component.prototype.shiftOffset = 0x3F;
components.Component.prototype.shiftControl = true;
components.Component.prototype.sendShifted = true;
// the color for most buttons is green, just change for hotcue buttons directly
components.Button.prototype.on = 0x01;
// override Component prototype to prevent double connections
// original connect function will override connections[0] and
// this way looses control of existing connection
components.Component.prototype.connect = function() {
    if (this.connections[0] === undefined &&      // <-- added this condition
        undefined !== this.group &&
        undefined !== this.outKey &&
        undefined !== this.output &&
        typeof this.output === "function") {
        this.connections[0] = engine.makeConnection(this.group, this.outKey, this.output.bind(this));
    }
};
// override Component prototype to reset connection after disconnect
// original disconnect will never reset connections[0] to undefined
components.Component.prototype.disconnect = function() {
    if (this.connections[0] !== undefined) {
        this.connections.forEach(function(conn) {
            conn.disconnect();
        });
    }
    this.connections[0] = undefined;              // <-- added this
};

// eslint-disable-next-line no-var
var ReloopMixtour = {};

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
            engine.makeConnection("[Skin]", `highlight_deck_[Channel${i}]`, function(_value, _group, _control) {
                ReloopMixtour.reSelectDecks();
            });
            engine.makeConnection("[Skin]", `highlight_mixer_[Channel${i}]`, function(_value, _group, _control) {
                ReloopMixtour.reSelectMixer();
            });
        }
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
        ReloopMixtour.decks.shutdown();
        ReloopMixtour.mixer.shutdown();
    };

    // there is one shift button, map it to the decks
    this.shiftButton = new components.Button({
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

    this.backBtn = new components.Button({
        midi: [0x90, 0x08],
        group: "[Skin]",
        key: "show_maximized_library",
        type: components.Button.prototype.types.toggle,
    });

    // the rotarySelector only works if mixxx windows is focused, don't be surprised :)
    this.rotarySelector = new components.Button({
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

        this.load = new components.Button({
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

        this.pfl = new components.Button({
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

        this.loop = new components.Button({
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

        this.beatSync = new components.Button({
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

        this.play = new components.PlayButton({
            midi: [0x90 + midiChannel, 0x0C],
        });

        this.cue = new components.CueButton({
            midi: [0x90 + midiChannel, 0x0B],
        });

        for (let i = 1; i <= 4; i++) {
            this[`hotcue${i}`] = new components.HotcueButton({
                midi: [0x90 + midiChannel, 0x0C + i],
                number: i,
                on: 0x2B,
            });
        };

        this.forEachComponent(function(component) {
            if (component.group === undefined) {
                component.group = this.currentDeck;
            };
        });
    }
};

ReloopMixtour.Mixer = class extends components.Deck {
    constructor(deckNumbers, midiChannel) {
        super(deckNumbers);

        const theMixer = this;

        this.gain = new components.Pot({
            key: "pregain",
        });

        this.fxKnob = new components.Pot({
            key: "super1",
            group: `[QuickEffectRack1_[Channel${midiChannel + 1}]]`,
        });

        this.fxButton = parseInt(engine.getSetting("fxButton"));
        this.effect = new components.Button({
            midi: [0x90 + midiChannel, 0x01],
            key: `group_[Channel${midiChannel + 1}]_enable`,
            shiftOffset: 0x40,
            group: this.fxButton?`[EffectRack1_EffectUnit${this.fxButton}]`:`[EffectRack1_EffectUnit${midiChannel + 1}]`,
            type: components.Button.prototype.types.toggle,
            input: function(channel, control, value, status, group) {
                if (value) {
                    theMixer.fxKnob.group = this.inGetValue()?`[QuickEffectRack1_${theMixer.currentDeck}]`:this.group;
                }
                components.Button.prototype.input.call(this, channel, control, value, status, group);
            },
            output: function(value, _group, _control) {
                theMixer.fxKnob.group = value?this.group:`[QuickEffectRack1_${theMixer.currentDeck}]`;
                this.send(this.outValueScale(value));
            },
            reconnect: function(newGroup) {
                this.disconnect();
                this.group = theMixer.fxButton?`[EffectRack1_EffectUnit${theMixer.fxButton}]`:`[EffectRack1_EffectUnit${script.deckFromGroup(newGroup)}]`;
                this.inKey = `group_${newGroup}_enable`;
                this.outKey = `group_${newGroup}_enable`;
                this.connect();
                this.trigger();
            },
        });

        this.highEq = new components.Pot({
            key: "parameter3",
            group: `[EqualizerRack1_[Channel${midiChannel + 1}]_Effect1]`,
        });

        this.midEq = new components.Pot({
            key: "parameter2",
            group: `[EqualizerRack1_[Channel${midiChannel + 1}]_Effect1]`,
        });

        this.lowEq = new components.Pot({
            key: "parameter1",
            group: `[EqualizerRack1_[Channel${midiChannel + 1}]_Effect1]`,
        });

        this.volumeFader = new components.Pot({
            inKey: "volume",
        });

        this.faderStart = new components.Button({
            key: "cue_gotoandplay",
        });

        this.faderStop = new components.Button({
            key: "cue_gotoandstop",
        });

        this.pflMeter = new components.Component({
            midi: [0x90 + midiChannel, 0x11],
            key: "vu_meter",
            max: 0x68
        });

        this.masterMeter = new components.Component({
            midi: [0x90 + midiChannel, 0x12],
            key: (this.currentDeck === "[Channel1]")?"vu_meter_left":"vu_meter_right",
            group: "[Main]",
            max: 0x68
        });

        this.fxIndicator = new components.Component({
            midi: [0x90 + midiChannel, 0x00],
            key: "super1",
            group: `[QuickEffectRack1_[Channel${midiChannel + 1}]]`,
            outValueScale(value) {
                return (Math.abs(0.5 - value) > 0.01)?0x7F:0x00;
            }
        });

        this.forEachComponent(function(component) {
            if (component.group === undefined) {
                component.group = this.currentDeck;
            };
        });
    }

    setCurrentDeck(newGroup) {
        this.effect.reconnect(newGroup);
        super.setCurrentDeck(newGroup);
    }
};
