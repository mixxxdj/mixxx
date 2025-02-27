"use strict";

// eslint-disable-next-line no-var
var SMCMixer;
(function(SMCMixer) {
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
        }
    }

    class Encoder extends components.Encoder {
        constructor(params) {
            super(params);
        }
        inValueScale(value) {
            return this.inGetParameter() - (value - 0x40);
        }
    }
    // LongPressButton is like a normal button of type powerWindow, except that it
    // doesn't trigger the short press and the long press. Instead it triggers on
    // release and leaves it to the user of the class to check if this was a long
    // press or a short press.
    class LongPressButton extends components.Button {
        constructor(knob, params) {
            super(params);
            this.input = function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status)) {
                    this.isLongPressed = false;
                    this.longPressTimer = engine.beginTimer(this.longPressTimeout, () => {
                        this.isLongPressed = true;
                        this.longPressTimer = 0;
                    }, true);
                } else {
                    this.inToggle();
                    if (!this.isLongPressed && this.triggerOnRelease) {
                        this.trigger();
                    }
                    if (this.longPressTimer !== 0) {
                        engine.stopTimer(this.longPressTimer);
                        this.longPressTimer = 0;
                    }
                    this.isLongPressed = false;
                }
            };
            this.inToggle = function() {
                if (this.isLongPressed) {
                    if (this.knob.key === this.key.replace("button_", "") || (this.key === "enabled" && this.knob.key === "super1")) {
                        this.knob.group = this.origGroup;
                        this.knob.key = this.origKey;
                        this.knob.inKey = this.origInKey;
                        this.knob.outKey = this.origOutKey;
                    } else {
                        this.knob.group = this.group;
                        let newKey = "";
                        if (this.key === "enabled") {
                            newKey = "super1";
                        } else {
                            newKey = this.key.replace("button_", "");
                        }
                        this.knob.key = newKey;
                        this.knob.inKey = newKey;
                        this.knob.outKey = newKey;
                    }
                } else {
                    const val = this.inGetParameter();
                    if (val > 0) {
                        this.inSetValue(0);
                    } else {
                        this.inSetValue(0x1F);
                    }
                }
            };
            this.knob = knob;
            this.origGroup = knob.group;
            this.origKey = knob.key;
            this.origInKey = knob.inKey;
            this.origOutKey = knob.outKey;
        }
    }
    class Pot extends components.Pot {
        constructor(params) {
            super(params);
            // If the hardware control does not match the software control by
            // anything less than the tolerance window, we consider them the
            // same. This way we're not constantly blinking the soft takeover
            // indicator because we didn't get the control matched up exactly.
            this.toleranceWindow = 0.001;
            this.input = function(_channel, _control, value, _status, _group) {
                const receivingFirstValue = this.hardwarePos === undefined;
                this.hardwarePos = this.inValueScale(value);
                engine.setParameter(this.group, this.inKey, this.hardwarePos);
                if (receivingFirstValue) {
                    this.firstValueReceived = true;
                    this.connect();
                    engine.softTakeover(this.group, this.inKey, true);
                }
            };
            this.output = function(value) {
                if (this.hardwarePos === undefined) {
                    return;
                }
                const parameterValue = engine.getParameter(this.group, this.outKey);
                const delta = parameterValue - this.hardwarePos;
                if (delta > this.toleranceWindow) {
                    midi.sendShortMsg(this.midi[0], this.hardwarePos, this.inValueScale(value));
                }
            };
        }
    }
    class EqRack {
        constructor(index) {
            const channel = mapIndexToChannel(index);
            this.knob = new Encoder({
                group: `[Channel${channel}]`,
                midi: [0xB0, 0x10 + index],
                key: "pregain",
            });
            this.highKillButton = new LongPressButton(this.knob, {
                type: components.Button.prototype.types.powerWindow,
                group: `[EqualizerRack1_[Channel${channel}]_Effect1]`,
                midi: [0x90, 0x10 + index],
                key: "button_parameter3",
            });
            this.midKillButton = new LongPressButton(this.knob, {
                type: components.Button.prototype.types.toggle,
                group: `[EqualizerRack1_[Channel${channel}]_Effect1]`,
                midi: [0x90, 0x08 + index],
                key: "button_parameter2",
            });
            this.lowKillButton = new LongPressButton(this.knob, {
                type: components.Button.prototype.types.toggle,
                group: `[EqualizerRack1_[Channel${channel}]_Effect1]`,
                midi: [0x90, 0x00 + index],
                key: "button_parameter1",
            });
            this.quickEffectButton = new LongPressButton(this.knob, {
                type: components.Button.prototype.types.toggle,
                group: `[QuickEffectRack1_[Channel${channel}]]`,
                midi: [0x90, 0x18 + index],
                key: "enabled",
            });
        }
    }
    class Controller extends components.ComponentContainer {
        constructor() {
            super({});
            this.activeDeck = new Deck();

            this.eqButtons = new Array(4);
            this.slipButtons = new Array(4);
            this.quantizeButtons = new Array(4);
            this.keylockButtons = new Array(4);
            this.pflButtons = new Array(4);
            this.faders = new Array(8);
            for (let i = 0; i < 4; i++) {
                const channel = mapIndexToChannel(i);
                const group = `[Channel${channel}]`;
                this.eqButtons[i] = new EqRack(i);
                this.slipButtons[i] = new components.Button({
                    type: components.Button.prototype.types.toggle,
                    group: group,
                    midi: [0x90, i+0x14],
                    key: "slip_enabled",
                });
                this.quantizeButtons[i] = new components.Button({
                    type: components.Button.prototype.types.toggle,
                    group: group,
                    midi: [0x90, 0x0C+i],
                    key: "quantize",
                });
                this.keylockButtons[i] = new components.Button({
                    type: components.Button.prototype.types.toggle,
                    group: group,
                    midi: [0x90, 0x04+i],
                    key: "keylock",
                });
                this.pflButtons[i] = new components.Button({
                    type: components.Button.prototype.types.toggle,
                    group: group,
                    midi: [0x90, 0x1C+i],
                    key: "pfl",
                });
                this.faders[i] = new Pot({
                    group: group,
                    midi: [0xE0+i],
                    key: "volume",
                    softTakeover: true,
                });
                this.faders[i+4] = new Pot({
                    group: group,
                    midi: [0xE4+i],
                    key: "rate",
                    softTakeover: true,
                });
            }

            this.gainKnob = new Encoder({
                group: "[Master]",
                midi: [0xB0, 0x14],
                key: "gain",
            });
            this.balanceKnob = new Encoder({
                group: "[Master]",
                midi: [0xB0, 0x15],
                key: "balance",
            });
            this.headGainKnob = new Encoder({
                group: "[Master]",
                midi: [0xB0, 0x16],
                key: "headGain",
            });
            this.headMixKnob = new Encoder({
                group: "[Master]",
                midi: [0xB0, 0x17],
                key: "headMix",
            });

            // Navigation buttons
            this.downButton = new components.Button({
                group: "[Library]",
                midi: [0x90, 0x61],
                key: "MoveDown",
            });
            this.upButton = new components.Button({
                group: "[Library]",
                midi: [0x90, 0x60],
                key: "MoveUp",
            });

            // For the left and right arrow buttons the controller appears to
            // handle the LED itself, so we use inKey so as not to be sending
            // output that will never be used.
            this.leftButton = new components.Button({
                group: "[Library]",
                midi: [0x90, 0x62],
                inKey: "focused_widget",
                input: function(_channel, _control, value, _status, _group) {
                    const selected = this.inGetParameter();
                    switch (selected) {
                    case 2: {
                        // Tree View
                        engine.setParameter(this.group, "GoToItem", value);
                        break;
                    }
                    case 3: {
                        // Tracks, goto Tree View
                        this.inSetParameter(2);
                        break;
                    }
                    }
                },
            });
            this.rightButton = new components.Button({
                group: "[Library]",
                midi: [0x90, 0x63],
                inKey: "focused_widget",
                input: function(_channel, _control, value, _status, _group) {
                    const selected = this.inGetParameter();
                    switch (selected) {
                    case 2: {
                        // Tree View, goto Library
                        this.inSetParameter(3);
                        break;
                    }
                    case 3: {
                        // Tracks
                        engine.setParameter(this.group, "GoToItem", value);
                        break;
                    }
                    }
                },
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
                        SMCMixer.controller.activeDeck.setCurrentDeck("[Channel3]");
                    } else {
                        SMCMixer.controller.activeDeck.setCurrentDeck("[Channel1]");
                    }
                },
            });
            this.deckRightButton = new components.Button({
                type: components.Button.prototype.types.powerWindow,
                group: "[Channel2]",
                midi: [0x90, 0x2F], // >> Channel Right button
                inToggle: function() {
                    if (this.isLongPressed) {
                        SMCMixer.controller.activeDeck.setCurrentDeck("[Channel4]");
                    } else {
                        SMCMixer.controller.activeDeck.setCurrentDeck("[Channel2]");
                    }
                },
            });
        }
    }

    SMCMixer.init = function() {
        SMCMixer.controller = new Controller();
    }
    SMCMixer.shutdown = function() {
        SMCMixer.controller.shutdown();
    }
})(SMCMixer || (SMCMixer = {}));
