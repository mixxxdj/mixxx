"use strict";


var SMK25II;
(function (SMK25II) {
    class Deck extends components.Deck {
        constructor() {
            super([1, 2, 3, 4]);
            // Knobs
            this.gainKnob = new components.Pot({
                group: "[Channel1]",
                key: "pregain",
                midi: [0xB0, 0x1E],
                softTakeover: true,
            });
            this.highKnob = new components.Pot({
                group: "[EqualizerRack1_[Channel1]_Effect1]",
                midi: [0xB0, 0x1F],
                key: "parameter3",
                softTakeover: true,
            });
            this.midKnob = new components.Pot({
                group: "[EqualizerRack1_[Channel1]_Effect1]",
                midi: [0xB0, 0x20],
                key: "parameter2",
                softTakeover: true,
            });
            this.lowKnob = new components.Pot({
                group: "[EqualizerRack1_[Channel1]_Effect1]",
                midi: [0xB0, 0x21],
                key: "parameter1",
                softTakeover: true,
            });
            this.effectKnob = new components.Pot({
                group: "[QuickEffectRack1_[Channel1]]",
                midi: [0xB0, 0x22],
                key: "super1",
                softTakeover: true,
            });
            // Hotcues
            this.hotcues = [];
            for (let i = 0; i < 8; i++) {
                const hotcueButton = new components.HotcueButton({
                    number: i + 1,
                    group: "[Channel1]",
                    midi: [0x90, i],
                    type: components.Button.prototype.types.push,
                });
                this.hotcues[i] = hotcueButton;
            }
            this.loopButton = new components.LoopToggleButton({
                group: "[Channel1]",
                midi: [0x90, 0x4C],
            });
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
            // Faders
            this.pitchStrip = new components.Pot({
                group: "[Channel1]",
                midi: [0xE0, 0xE0],
                key: "rate",
            });
            this.modStrip = new components.Pot({
                group: "[Channel1]",
                midi: [0xB0, 0x01],
                key: "volume",
            });
        }
    }

    class Controller extends components.ComponentContainer {
        constructor() {
            super({});
            this.activeDeck = new Deck();
            this.gainKnob = new components.Encoder({
                group: "[Master]",
                midi: [0xB0, 0x25],
                key: "gain",
            });
            this.headGainKnob = new components.Encoder({
                group: "[Master]",
                midi: [0xB0, 0x24],
                key: "headGain",
            });
            this.xfadeKnob = new components.Encoder({
                group: "[Master]",
                midi: [0xB0, 0x23],
                key: "crossfader",
            });
            this.recordButton = new components.Button({
                group: "[Recording]",
                midi: [0x90, 0x5F],
                inKey: "toggle_recording",
                outKey: "status",
            });
            // Deck Select
            this.deckLeftButton = new components.Button({
                group: "[Channel1]",
                midi: [0x90, 0x2E], // << Channel Left Button
                inToggle: function() {
                    SMK25II.controller.activeDeck.setCurrentDeck("[Channel1]");
                },
            });
            this.deckRightButton = new components.Button({
                group: "[Channel2]",
                midi: [0x90, 0x2F], // >> Channel Right button
                inToggle: function() {
                    SMK25II.controller.activeDeck.setCurrentDeck("[Channel2]");
                },
            });
            // Drum Pads
            this.samplers = [];
            for (let i = 0; i < 16; i++) {
                this.samplers[i] = new components.SamplerButton({
                    number: i + 1,
                    volumeByVelocity: true,
                });
            }
        }
    }

    /**
     *
     * @param _id The controller ID
     * @param _debugging Whether we've started in debugging mode.
     */
    SMK25II.init = function(_id, _debugging) {
        SMK25II.controller = new Controller();
    };
    SMK25II.shutdown = function() {
        SMK25II.controller.shutdown();
    };
    const MMCHeader = [0xF0, 0x35, 0x59];
    function decodeButton(data) {
        // Hotcues
        if (data[4] !== undefined && data[4] >= 0x00 && data[4] <= 0x07) {
            SMK25II.controller.activeDeck.hotcues[data[4]].inToggle();
            return;
        }
        // Other media buttons
        switch (data[4]) {
            case 0x5D: {
                SMK25II.controller.activeDeck.cueButton.inToggle();
                break;
            }
            case 0x5E: {
                if (data[5] === 0x00) {
                    SMK25II.controller.activeDeck.playButton.inToggle();
                }
                break;
            }
            case 0x5B: {
                SMK25II.controller.activeDeck.backButton.inToggle();
                break;
            }
            case 0x5C: {
                SMK25II.controller.activeDeck.forwardButton.inToggle();
                break;
            }
            case 0x5F: {
                SMK25II.controller.recordButton.inToggle();
                break;
            }
            case 0x2E: {
                SMK25II.controller.deckLeftButton.inToggle();
                break;
            }
            case 0x2F: {
                SMK25II.controller.deckRightButton.inToggle();
                break;
            }
            case 0x4C: {
                SMK25II.controller.activeDeck.loopButton.inToggle();
                break;
            }
            default:
                console.log(`unrecognized MMC command: ${data[4]}`);
        }
    }
    function decodePots(data) {
        switch (data[3]) {
            case 0x60: {
                const knob = SMK25II.controller.activeDeck.gainKnob;
                knob.inSetParameter(knob.inValueScale(data[5]));
                break;
            }
            case 0x61: {
                const knob = SMK25II.controller.activeDeck.highKnob;
                knob.inSetParameter(knob.inValueScale(data[5]));
                break;
            }
            case 0x62: {
                const knob = SMK25II.controller.activeDeck.midKnob;
                knob.inSetParameter(knob.inValueScale(data[5]));
                break;
            }
            case 0x63: {
                const knob = SMK25II.controller.activeDeck.lowKnob;
                knob.inSetParameter(knob.inValueScale(data[5]));
                break;
            }
            case 0x64: {
                const knob = SMK25II.controller.activeDeck.effectKnob;
                knob.inSetParameter(knob.inValueScale(data[5]));
                break;
            }
            case 0x65: {
                const knob = SMK25II.controller.xfadeKnob;
                knob.inSetParameter(knob.inValueScale(data[5]));
                break;
            }
            case 0x66: {
                const knob = SMK25II.controller.headGainKnob;
                knob.inSetParameter(knob.inValueScale(data[5]));
                break;
            }
            case 0x67: {
                const knob = SMK25II.controller.gainKnob;
                knob.inSetParameter(knob.inValueScale(data[5]));
                break;
            }
        }
    }
    function incomingData(data, length) {
        if (length < 6) {
            console.log(`expected sysex packet of length 6, got ${length}`);
            return;
        }
        for (let n = 0; n < MMCHeader.length; n++) {
            if (data[n] !== MMCHeader[n]) {
                console.log("unknown sysex packet");
                return;
            }
        }
        if (data[length - 1] !== 0xF7) {
            console.log("sysex packet missing trailer");
            return;
        }
        if (data[3] == 0x10) {
            decodeButton(data);
        } else if (data[3] >= 0x60 && data[3] <= 0x67) {
            decodePots(data);
        } else {
            console.log(`unrecognized sysex byte: ${data[3]}`);
        }
    }
    SMK25II.incomingData = incomingData;
})(SMK25II || (SMK25II = {}));
