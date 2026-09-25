"use strict";

// eslint-disable-next-line no-var
var iControls;
(function(iControls) {
    const mapIndexToChannel = function(index) {
        switch (Math.abs(index) % 4) {
        case 0: return 3;
        case 1: return 1;
        case 2: return 2;
        case 3: return 4;
        }
    };

    class Deck extends components.Deck {
        constructor() {
            super([1, 2, 3, 4]);
            // Transport buttons
            this.playButton = new components.PlayButton({
                group: "[Channel1]",
                midi: [0x90, 0x5E], // Play transport
                type: components.Button.prototype.types.toggle,
            });
            this.cueButton = new components.CueButton({
                group: "[Channel1]",
                midi: [0x90, 0x5D], // Pause transport
                type: components.Button.prototype.types.push,
            });
            this.backButton = new components.Button({
                group: "[Channel1]",
                midi: [0x90, 0x5B],
                key: "beatjump_backward",
            });
            this.forwardButton = new components.Button({
                group: "[Channel1]",
                midi: [0x90, 0x5C],
                key: "beatjump_forward",
            });
            this.loopButton = new components.Button({
                group: "[Channel1]",
                midi: [0x90, 0x56],
                inKey: "beatloop_activate",
                outKey: "loop_enabled",
            });

            this.leftDeckBtn = new components.Button({
                group: "[Channel1]",
                midi: [0x90, 0x30],
                type: components.Button.prototype.types.powerWindow,
                deck: this,
                inToggle: function() {
                    if (this.isLongPressed) {
                        this.deck.setCurrentDeck("[Channel3]");
                    } else {
                        this.deck.setCurrentDeck("[Channel1]");
                    }
                },
            });
            this.rightDeckBtn = new components.Button({
                group: "[Channel2]",
                midi: [0x90, 0x31],
                type: components.Button.prototype.types.powerWindow,
                deck: this,
                inToggle: function() {
                    if (this.isLongPressed) {
                        this.deck.setCurrentDeck("[Channel4]");
                    } else {
                        this.deck.setCurrentDeck("[Channel2]");
                    }
                },
            });
        }
    }

    class VelocityPot extends components.Pot {
        constructor(params) {
            super(Object.assign({
                softTakeover: false,
                relative: true,
            }, params));
        }
        inValueScale(value) {
            value = value > 0x40 ? value - 0x80 : value;
            return this.inGetParameter() + (value / 100);
        }
    }

    class Controller extends components.ComponentContainer {
        constructor() {
            super({});
            this.activeDeck = new Deck();

            this.faders = new Array(9);
            this.knobs = new Array(9);
            this.pflButton = new Array(4);
            this.nextEffectButton = new Array(4);
            this.slipButton = new Array(4);
            this.superButton = new Array(4);
            for (let i = 0; i < 4; i++) {
                const channel = mapIndexToChannel(i);
                const group = `[Channel${channel}]`;
                this.faders[i] = new components.Pot({
                    group: group,
                    midi: [0xB0+i, 0x0D],
                    inKey: "volume",
                    softTakeover: true,
                });
                this.faders[i+4] = new components.Pot({
                    group: group,
                    midi: [0xB0+i, 0x0D],
                    inKey: "rate",
                    softTakeover: true,
                });
                this.knobs[i] = new VelocityPot({
                    group: group,
                    midi: [0xB0, 0x10+i],
                    inKey: "pregain",
                });
                this.knobs[i+4] = new VelocityPot({
                    group: `[QuickEffectRack1_${group}]`,
                    midi: [0xB0, 0x14+i],
                    inKey: "super1",
                });
                this.pflButton[i] = new components.Button({
                    group: group,
                    midi: [0x90, 0x08+i],
                    key: "pfl",
                    type: components.Button.prototype.types.toggle,
                });
                this.slipButton[i] = new components.Button({
                    group: group,
                    midi: [0x90, 0x10+i],
                    key: "slip_enabled",
                    type: components.Button.prototype.types.toggle,
                });
                this.nextEffectButton[i] = new components.Button({
                    group: `[QuickEffectRack1_${group}]`,
                    midi: [0x90, 0x0C+i],
                    key: "next_chain_preset",
                });
                this.superButton[i] = new components.Button({
                    group: `[QuickEffectRack1_${group}]`,
                    midi: [0x90, 0x14+i],
                    key: "enabled",
                    type: components.Button.prototype.types.toggle,
                });
            }
            this.faders[8] = new components.Pot({
                group: "[Master]",
                midi: [0xE8],
                key: "crossfader",
                softTakeover: true,
            });

            // For some reason just this one is a normal 7-bit CC value, so no
            // need to use the VelocityPot we use on the other knobs on layer 1.
            this.knobs[8] = new components.Pot({
                group: "[Master]",
                midi: [0xB0, 0x0C],
                key: "gain",
                softTakeover: true,
            });

            this.recordButton = new components.Button({
                group: "[Recording]",
                midi: [0x90, 0x5F],
                inKey: "toggle_recording",
                outKey: "status",
            });
        }
    }

    iControls.init = function() {
        iControls.controller = new Controller();
    };
    iControls.shutdown = function() {
        iControls.controller.shutdown();
    };
})(iControls || (iControls = {}));

// vim:expandtab:shiftwidth=4:tabstop=4
