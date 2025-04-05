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
        }
    }

    class Controller extends components.ComponentContainer {
        constructor() {
            super({});
            this.activeDeck = new Deck();

            this.faders = new Array(9);
            this.knobs = new Array(9);
            this.pflButton = new Array(4);
            this.superButton = new Array(4);
            for (let i = 0; i < 4; i++) {
                const channel = mapIndexToChannel(i);
                const group = `[Channel${channel}]`;
                this.faders[i] = new components.Pot({
                    group: group,
                    midi: [0xB0+i, 0x0D],
                    key: "volume",
                    softTakeover: true,
                });
                this.faders[i+4] = new components.Pot({
                    group: group,
                    midi: [0xB0+i, 0x0D],
                    key: "rate",
                    softTakeover: true,
                });
                this.knobs[i] = new components.Pot({
                    group: group,
                    midi: [0xB0, 0x10+i],
                    key: "pregain",
                    softTakeover: true,
                });
                this.knobs[i+4] = new components.Pot({
                    group: `[QuickEffectRack1_${group}]`,
                    midi: [0xB0, 0x14+i],
                    key: "super1",
                    softTakeover: true,
                });
                this.pflButton[i] = new components.Button({
                    group: group,
                    midi: [0x90, 0x08+i],
                    key: "pfl",
                    type: components.Button.prototype.types.toggle,
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
            this.deckLeftButton = new components.Button({
                type: components.Button.prototype.types.powerWindow,
                group: "[Channel1]",
                midi: [0x90, 0x2E], // << Channel Left Button
                inToggle: function() {
                    if (this.isLongPressed) {
                        iControls.controller.activeDeck.setCurrentDeck("[Channel3]");
                    } else {
                        iControls.controller.activeDeck.setCurrentDeck("[Channel1]");
                    }
                },
            });
            this.deckRightButton = new components.Button({
                type: components.Button.prototype.types.powerWindow,
                group: "[Channel2]",
                midi: [0x90, 0x2F], // >> Channel Right button
                inToggle: function() {
                    if (this.isLongPressed) {
                        iControls.controller.activeDeck.setCurrentDeck("[Channel4]");
                    } else {
                        iControls.controller.activeDeck.setCurrentDeck("[Channel2]");
                    }
                },
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
