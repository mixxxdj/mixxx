"use strict";

// eslint-disable-next-line no-var
var MidiFighterTwister;
(function(MidiFighterTwister) {
    const scaleHalfPlusOne = function(value) {
        return (value + 1) / 2 * this.max;
    };

    const linearize = function(value) {
        let max = 127;
        if (this !== undefined && this.max !== undefined) {
            max = this.max;
        }
        return Math.pow(value / 4, 0.5) * max;
    };

    const doubleLinearize = function(value) {
        return linearize(value) * 2;
    };

    const indicatorConnect = function(color, key) {
        return function() {
            this.connections[0] = engine.makeConnection(this.group, this.outKey, this.output.bind(this));

            if (color !== "off") {
                this.connections[1] = engine.makeConnection(this.group, key, (value) => {
                    if (value) {
                        this.send(color);
                    } else {
                        const pregainDef = this.inGetParameter();
                        this.send(pregainDef ? this.on : this.off);
                    }
                });
            }
        };
    };

    const multiSegConnect = function(enabled, key, scale) {
        return function() {
            this.connections[0] = engine.makeConnection(this.group, this.outKey, this.output.bind(this));

            if (typeof scale !== "function") {
                scale = this.outValueScale;
            }
            if (enabled) {
                this.connections[1] = engine.makeConnection(this.group, key, (value) => {
                    this.send(scale(value));
                });
            }
        };
    };

    components.Button.prototype.on = engine.getSetting("defColor");
    components.Button.prototype.off = engine.getSetting("relColor");

    class Deck extends components.Deck {
        midiModifier(value) {
            // All midi values are for the left deck, we only modify them if
            // we're constructing the right hand deck.
            if (this.deckNumbers[0] === 1) {
                return value;
            }

            // Even controls (ie. the first row) are mirrored to the far right
            // row, three rows over.
            if (value % 2 === 0) {
                return value + 3;
            }
            // Odd controls (ie. the second row) are mirrored to the other inner
            // row, one row over.
            return value + 1;
        }

        constructor(deckNumbers) {
            super(deckNumbers);

            this.rateKnob = new components.Encoder({
                group: `[Channel${this.deckNumbers[0]}]`,
                midi: [0xB0, this.midiModifier(0x04)],
                key: "rate",
                outValueScale: scaleHalfPlusOne,
            });
            this.gainKnob = new components.Encoder({
                group: `[Channel${this.deckNumbers[0]}]`,
                midi: [0xB0, this.midiModifier(0x00)],
                key: "pregain",
                outValueScale: linearize,
                connect: multiSegConnect(engine.getSetting("vuMeter"), "VuMeter", doubleLinearize),
            });
            this.volumeKnob = new components.Encoder({
                group: `[Channel${this.deckNumbers[0]}]`,
                midi: [0xB0, this.midiModifier(0x08)],
                key: "volume",
                outValueScale: doubleLinearize,
            });
            this.highKnob = new components.Encoder({
                group: `[EqualizerRack1_[Channel${this.deckNumbers[0]}]_Effect1]`,
                midi: [0xB0, this.midiModifier(0x01)],
                key: "parameter3",
                outValueScale: linearize,
            });
            this.midKnob = new components.Encoder({
                group: `[EqualizerRack1_[Channel${this.deckNumbers[0]}]_Effect1]`,
                midi: [0xB0, this.midiModifier(0x05)],
                key: "parameter2",
                outValueScale: linearize,
            });
            this.lowKnob = new components.Encoder({
                group: `[EqualizerRack1_[Channel${this.deckNumbers[0]}]_Effect1]`,
                midi: [0xB0, this.midiModifier(0x09)],
                key: "parameter1",
                outValueScale: linearize,
            });
            this.superKnob = new components.Encoder({
                group: `[QuickEffectRack1_[Channel${this.deckNumbers[0]}]]`,
                midi: [0xB0, this.midiModifier(0x0D)],
                key: "super1",
            });

            this.rateButton = new components.Button({
                group: `[Channel${this.deckNumbers[0]}]`,
                midi: [0xB1, this.midiModifier(0x04)],
                key: "rate_set_default",
                connect: indicatorConnect(engine.getSetting("beatColor"), "beat_active"),
            });

            this.gainButton = new components.Button({
                group: `[Channel${this.deckNumbers[0]}]`,
                midi: [0xB1, this.midiModifier(0x00)],
                key: "pregain_set_default",
                connect: indicatorConnect(engine.getSetting("peakColor"), "PeakIndicator"),
            });
            // The volume button toggles the headphones, unlike the others which
            // reset their control.
            this.pflButton = new components.Button({
                group: `[Channel${this.deckNumbers[0]}]`,
                midi: [0xB1, this.midiModifier(0x08)],
                type: components.Button.prototype.types.toggle,
                key: "pfl",
            });

            const shiftFunc = function(newKey, onColor, offColor) {
                return function() {
                    this.inKey = newKey;
                    this.outKey = newKey;
                    this.type = components.Button.prototype.types.toggle;
                    this.on = onColor;
                    this.off = offColor;
                    this.disconnect();
                    this.connect();
                    this.trigger();
                };
            };
            const unshiftFunc = function() {
                this.inKey = this.key;
                this.outKey = this.key;
                this.type = components.Button.prototype.types.push;
                this.on = components.Button.prototype.on;
                this.off = components.Button.prototype.off;
                this.disconnect();
                this.connect();
                this.trigger();
            };
            this.highButton = new components.Button({
                group: `[EqualizerRack1_[Channel${this.deckNumbers[0]}]_Effect1]`,
                midi: [0xB1, this.midiModifier(0x01)],
                key: "parameter3_set_default",
                shift: shiftFunc("button_parameter3", engine.getSetting("eqOnColor"), engine.getSetting("eqOffColor")),
                unshift: unshiftFunc,
            });
            this.midButton = new components.Button({
                group: `[EqualizerRack1_[Channel${this.deckNumbers[0]}]_Effect1]`,
                midi: [0xB1, this.midiModifier(0x05)],
                key: "parameter2_set_default",
                shift: shiftFunc("button_parameter2", engine.getSetting("eqOnColor"), engine.getSetting("eqOffColor")),
                unshift: unshiftFunc,
            });
            this.lowButton = new components.Button({
                group: `[EqualizerRack1_[Channel${this.deckNumbers[0]}]_Effect1]`,
                midi: [0xB1, this.midiModifier(0x09)],
                key: "parameter1_set_default",
                shift: shiftFunc("button_parameter1", engine.getSetting("eqOnColor"), engine.getSetting("eqOffColor")),
                unshift: unshiftFunc,
            });
            this.superButton = new components.Button({
                group: `[QuickEffectRack1_[Channel${this.deckNumbers[0]}]]`,
                midi: [0xB1, this.midiModifier(0x0D)],
                key: "super1_set_default",
                shift: shiftFunc("enabled", engine.getSetting("superOnColor"), engine.getSetting("superOffColor")),
                unshift: unshiftFunc,
            });
        }
    }

    class Controller extends components.ComponentContainer {
        constructor() {
            super({});

            this.leftDeck = new Deck([1, 3]);
            this.rightDeck = new Deck([2, 4]);

            // Layer 1 Controls

            this.crossfaderKnob = new components.Encoder({
                group: "[Master]",
                midi: [0xB0, 0x0C],
                key: "crossfader",
                outValueScale: scaleHalfPlusOne,
                unshift: function() {
                    this.inKey = "crossfader";
                    this.outKey = "crossfader";
                    this.disconnect();
                    this.connect();
                    this.trigger();
                },
                shift: function() {
                    this.inKey = "balance";
                    this.outKey = "balance";
                    this.disconnect();
                    this.connect();
                    this.trigger();
                },
            });
            this.crossfaderButton = new components.Button({
                group: "[Master]",
                midi: [0xB1, 0x0C],
                key: "crossfader_set_default",
                unshift: function() {
                    this.inKey = "crossfader_set_default";
                },
                shift: function() {
                    this.inKey = "balance_set_default";
                },
            });
            this.mainGainKnob = new components.Encoder({
                group: "[Master]",
                midi: [0xB0, 0x0F],
                key: "gain",
                outValueScale: linearize,
                connect: multiSegConnect(engine.getSetting("vuMeter"), "VuMeter", doubleLinearize),
                unshift: function() {
                    this.inKey = "gain";
                    this.outKey = "gain";
                    this.disconnect();
                    this.connect();
                    this.trigger();
                },
                shift: function() {
                    this.inKey = "headGain";
                    this.outKey = "headGain";
                    this.disconnect();
                    this.connect();
                    this.trigger();
                },
            });
            this.mainGainButton = new components.Encoder({
                group: "[Master]",
                midi: [0xB1, 0x0F],
                key: "gain_set_default",
                connect: indicatorConnect(engine.getSetting("peakColor"), "PeakIndicator"),
                unshift: function() {
                    this.inKey = "gain_set_default";
                },
                shift: function() {
                    this.inKey = "headGain_set_default";
                },
            });

            // Layer 2 Controls
            this.fx = [];
            this.fx[0] = new components.EffectUnit([1, 3]);
            this.fx[0].enableButtons[1].midi = [0xB1, 0x11];
            this.fx[0].enableButtons[2].midi = [0xB1, 0x15];
            this.fx[0].enableButtons[3].midi = [0xB1, 0x19];
            this.fx[0].knobs[1].midi = [0xB0, 0x11];
            this.fx[0].knobs[2].midi = [0xB0, 0x15];
            this.fx[0].knobs[3].midi = [0xB0, 0x19];
            this.fx[0].dryWetKnob.midi = [0xB0, 0x1D];
            this.fx[0].effectFocusButton.midi = [0xB1, 0x1D];
            this.fx[0].enableOnChannelButtons.addButton("Channel1");
            this.fx[0].enableOnChannelButtons.addButton("Channel2");
            this.fx[0].enableOnChannelButtons.addButton("Headphone");
            this.fx[0].enableOnChannelButtons.Channel1.midi = [0xB1, 0x10];
            this.fx[0].enableOnChannelButtons.Channel1.shift = function() {
                this.inKey = "group_[Channel3]_enable";
                this.outKey = "group_[Channel3]_enable";
            };
            this.fx[0].enableOnChannelButtons.Channel1.unshift = function() {
                this.inKey = "group_[Channel1]_enable";
                this.outKey = "group_[Channel1]_enable";
            };
            this.fx[0].enableOnChannelButtons.Channel2.midi = [0xB1, 0x14];
            this.fx[0].enableOnChannelButtons.Channel2.shift = function() {
                this.inKey = "group_[Channel4]_enable";
                this.outKey = "group_[Channel4]_enable";
            };
            this.fx[0].enableOnChannelButtons.Channel2.unshift = function() {
                this.inKey = "group_[Channel2]_enable";
                this.outKey = "group_[Channel2]_enable";
            };
            this.fx[0].enableOnChannelButtons.Headphone.midi = [0xB1, 0x18];
            this.fx[0].mixModeButton = new components.Button({
                group: "[EffectRack1_EffectUnit1]",
                key: "mix_mode",
                midi: [0xB1, 0x1C],
                type: components.Button.prototype.types.toggle,
            });
            this.fx[0].init();

            this.fx[1] = new components.EffectUnit([2, 4]);
            this.fx[1].enableButtons[1].midi = [0xB1, 0x12];
            this.fx[1].enableButtons[2].midi = [0xB1, 0x16];
            this.fx[1].enableButtons[3].midi = [0xB1, 0x1A];
            this.fx[1].knobs[1].midi = [0xB0, 0x12];
            this.fx[1].knobs[2].midi = [0xB0, 0x16];
            this.fx[1].knobs[3].midi = [0xB0, 0x1A];
            this.fx[1].dryWetKnob.midi = [0xB0, 0x1E];
            this.fx[1].effectFocusButton.midi = [0xB1, 0x1E];
            this.fx[1].enableOnChannelButtons.addButton("Channel1");
            this.fx[1].enableOnChannelButtons.addButton("Channel2");
            this.fx[1].enableOnChannelButtons.addButton("Headphone");
            this.fx[1].enableOnChannelButtons.Channel1.midi = [0xB1, 0x13];
            this.fx[1].enableOnChannelButtons.Channel1.shift = function() {
                this.inKey = "group_[Channel3]_enable";
                this.outKey = "group_[Channel3]_enable";
            };
            this.fx[1].enableOnChannelButtons.Channel1.unshift = function() {
                this.inKey = "group_[Channel1]_enable";
                this.outKey = "group_[Channel1]_enable";
            };
            this.fx[1].enableOnChannelButtons.Channel2.midi = [0xB1, 0x17];
            this.fx[1].enableOnChannelButtons.Channel2.shift = function() {
                this.inKey = "group_[Channel4]_enable";
                this.outKey = "group_[Channel4]_enable";
            };
            this.fx[1].enableOnChannelButtons.Channel2.unshift = function() {
                this.inKey = "group_[Channel2]_enable";
                this.outKey = "group_[Channel2]_enable";
            };
            this.fx[1].enableOnChannelButtons.Headphone.midi = [0xB1, 0x1B];
            this.fx[1].mixModeButton = new components.Button({
                group: "[EffectRack1_EffectUnit2]",
                key: "mix_mode",
                midi: [0xB1, 0x1F],
                type: components.Button.prototype.types.toggle,
            });
            this.fx[1].init();
        }

        toggleShift(_channel, _control, value, _status, _group) {
            if (value) {
                this.shift();
            } else {
                this.unshift();
            }
        }

        toggleEffects(_channel, _control, value, _status, group) {
            if (value === 0) {
                // Only toggle on press, don't toggle it again on release.
                return;
            }
            switch (group) {
            case "[Channel1]":
                this.fx[0].toggle();
                break;
            case "[Channel2]":
                this.fx[1].toggle();
                break;
            default:
                console.log(`invalid group: ${group}`);
            }
        }

        toggleDeck(_channel, _control, value, _status, group) {
            if (value === 0) {
                // Only toggle on press, don't toggle it again on release.
                return;
            }
            switch (group) {
            case "[Channel1]":
                this.leftDeck.toggle();
                break;
            case "[Channel2]":
                this.rightDeck.toggle();
                break;
            default:
                console.log(`invalid group: ${group}`);
            }
        }
    }

    MidiFighterTwister.init = function() {
        MidiFighterTwister.controller = new Controller();
    };

    MidiFighterTwister.shutdown = function() {
        MidiFighterTwister.controller.shutdown();
    };
})(MidiFighterTwister || (MidiFighterTwister = {}));

// vim:expandtab:tabstop=4:shiftwidth=4
