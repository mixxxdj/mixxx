"use strict";

// eslint-disable-next-line no-unused-vars
var SMCMixer;
(function(SMCMixer) {
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
        function inValueScale(value) {
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
            this.toleranceWindow = 0.001;
            // TODO: why do I have to override this?
            this.connect = function() {
                if (undefined !== this.group
                    && undefined !== this.outKey
                    && undefined !== this.output
                    && typeof this.output === "function") {
                    this.connections[0] = engine.makeConnection(this.group, this.outKey, this.output.bind(this));
                }
            };
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
        constructor(deck) {
            // Normalize the deck number to be zero indexed (the third deck is the 0th
            // control).
            let normDeck = deck;
            switch (deck) {
            case 3: {
                normDeck = 0;
                break;
            }
            case 4: {
                normDeck = 3;
                break;
            }
            }
            this.knob = new Encoder({
                group: `[Channel${deck}]`,
                midi: [0xB0, 0x10 + normDeck],
                key: "pregain",
            });
            this.highKillButton = new LongPressButton(this.knob, {
                type: components.Button.prototype.types.powerWindow,
                group: `[EqualizerRack1_[Channel${deck}]_Effect1]`,
                midi: [0x90, 0x10 + normDeck],
                key: "button_parameter3",
            });
            this.midKillButton = new LongPressButton(this.knob, {
                type: components.Button.prototype.types.toggle,
                group: `[EqualizerRack1_[Channel${deck}]_Effect1]`,
                midi: [0x90, 0x08 + normDeck],
                key: "button_parameter2",
            });
            this.lowKillButton = new LongPressButton(this.knob, {
                type: components.Button.prototype.types.toggle,
                group: `[EqualizerRack1_[Channel${deck}]_Effect1]`,
                midi: [0x90, 0x00 + normDeck],
                key: "button_parameter1",
            });
            this.quickEffectButton = new LongPressButton(this.knob, {
                type: components.Button.prototype.types.toggle,
                group: `[QuickEffectRack1_[Channel${deck}]]`,
                midi: [0x90, 0x18 + normDeck],
                key: "enabled",
            });
        }
    }
    class Controller extends components.ComponentContainer {
        constructor() {
            super({});
            this.activeDeck = new Deck();
            this.eqButtons = new Array(4);
            this.eqButtons[0] = new EqRack(3);
            this.eqButtons[1] = new EqRack(1);
            this.eqButtons[2] = new EqRack(2);
            this.eqButtons[3] = new EqRack(4);
            // Slip Mode
            this.slipButtons = new Array(4);
            this.slipButtons[0] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel3]",
                midi: [0x90, 0x14],
                key: "slip_enabled",
            });
            this.slipButtons[1] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel1]",
                midi: [0x90, 0x15],
                key: "slip_enabled",
            });
            this.slipButtons[2] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel2]",
                midi: [0x90, 0x16],
                key: "slip_enabled",
            });
            this.slipButtons[3] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel4]",
                midi: [0x90, 0x17],
                key: "slip_enabled",
            });
            // Quantize
            this.quantizeButtons = new Array(4);
            this.quantizeButtons[0] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel3]",
                midi: [0x90, 0x0C],
                key: "quantize",
            });
            this.quantizeButtons[1] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel1]",
                midi: [0x90, 0x0D],
                key: "quantize",
            });
            this.quantizeButtons[2] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel2]",
                midi: [0x90, 0x0E],
                key: "quantize",
            });
            this.quantizeButtons[3] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel4]",
                midi: [0x90, 0x0F],
                key: "quantize",
            });
            // Key Lock
            this.keylockButtons = new Array(4);
            this.keylockButtons[0] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel3]",
                midi: [0x90, 0x04],
                key: "keylock",
            });
            this.keylockButtons[1] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel1]",
                midi: [0x90, 0x05],
                key: "keylock",
            });
            this.keylockButtons[2] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel2]",
                midi: [0x90, 0x06],
                key: "keylock",
            });
            this.keylockButtons[3] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel4]",
                midi: [0x90, 0x07],
                key: "keylock",
            });
            this.pflButtons = new Array(4);
            this.pflButtons[0] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel3]",
                midi: [0x90, 0x1C],
                key: "pfl",
            });
            this.pflButtons[1] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel1]",
                midi: [0x90, 0x1D],
                key: "pfl",
            });
            this.pflButtons[2] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel2]",
                midi: [0x90, 0x1E],
                key: "pfl",
            });
            this.pflButtons[3] = new components.Button({
                type: components.Button.prototype.types.toggle,
                group: "[Channel4]",
                midi: [0x90, 0x1F],
                key: "pfl",
            });
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
            this.leftButton = new components.Button({
                group: "[Library]",
                midi: [0x90, 0x62],
                key: "focused_widget",
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
                key: "focused_widget",
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
            // Faders
            this.faders = new Array(9);
            this.faders[0] = new Pot({
                group: "[Channel3]",
                midi: [0xE0],
                key: "volume",
                softTakeover: true,
            });
            this.faders[1] = new Pot({
                group: "[Channel1]",
                midi: [0xE1],
                key: "volume",
                softTakeover: true,
            });
            this.faders[2] = new Pot({
                group: "[Channel2]",
                midi: [0xE2],
                key: "volume",
                softTakeover: true,
            });
            this.faders[3] = new Pot({
                group: "[Channel4]",
                midi: [0xE3],
                key: "volume",
                softTakeover: true,
            });
            this.faders[4] = new Pot({
                group: "[Channel3]",
                midi: [0xE4],
                key: "rate",
                softTakeover: true,
            });
            this.faders[5] = new Pot({
                group: "[Channel1]",
                midi: [0xE5],
                key: "rate",
                softTakeover: true,
            });
            this.faders[6] = new Pot({
                group: "[Channel2]",
                midi: [0xE6],
                key: "rate",
                softTakeover: true,
            });
            this.faders[7] = new Pot({
                group: "[Channel4]",
                midi: [0xE7],
                key: "rate",
                softTakeover: true,
            });
        }
    }

    /**
     *
     * @param _id
     * @param _debugging
     */
    function init(_id, _debugging) {
        SMCMixer.controller = new Controller();
    }
    SMCMixer.init = init;
    /**
     *
     */
    function shutdown() {
        SMCMixer.controller.shutdown();
    }
    SMCMixer.shutdown = shutdown;
})(SMCMixer || (SMCMixer = {}));
