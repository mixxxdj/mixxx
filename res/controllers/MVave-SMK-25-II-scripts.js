"use strict";

// eslint-disable-next-line no-var
var SMK25II;
(function(SMK25II) {
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
            //
            // Note:
            // The color of the pads cannot be set, it is controlled by the
            // firmware and depends on what layer we're on.
            this.hotcues = [];
            for (let i = 0; i < 8; i++) {
                const hotcueButton = new components.HotcueButton({
                    number: i + 1,
                    group: "[Channel1]",
                    midi: [0x90, i],
                    type: components.Button.prototype.types.push,
                    colorMapper: new ColorMapper({
                        0xCC0000: 0x7F, // Red
                        0xCC7800: 0x10, // Orange

                        0xCCCC00: 0x04, // Yellow
                        0x81CC00: 0x13, // Lime
                        0x00CC00: 0x06, // Green

                        0x00CCCC: 0x08, // Bianchi/Celeste/Teal
                        0x0000CC: 0x17, // Blue

                        0xFCA6D7: 0x01, // Pink
                        0xCC0091: 0x0D, // Fuscia

                        0xCCCCCC: 0x0E, // White
                    }),
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
                input: function(_channel, _control, value, _status, _group) {
                    if (value === 0x00) {
                        SMK25II.controller.activeDeck.setCurrentDeck("[Channel1]");
                    }
                },
            });
            this.deckRightButton = new components.Button({
                group: "[Channel2]",
                midi: [0x90, 0x2F], // >> Channel Right button
                input: function(_channel, _control, value, _status, _group) {
                    if (value === 0x00) {
                        SMK25II.controller.activeDeck.setCurrentDeck("[Channel2]");
                    }
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

    SMK25II.init = function() {
        SMK25II.controller = new Controller();
    };
    SMK25II.shutdown = function() {
        SMK25II.controller.shutdown();
    };

    const SysexHeader = [0xF0, 0x35, 0x59];

    /**
     *
     * @param data the full sysex message data.
     */
    const decodeButton = function(data) {
        // Hotcues
        if (data[4] !== undefined && data[4] >= 0x00 && data[4] <= 0x07) {
            // This controller appears to send the release message immediately
            // even if you're still holding the button down.
            const button = SMK25II.controller.activeDeck.hotcues[data[4]];
            button.input(undefined, undefined, data[5], undefined, undefined);
            return;
        }
        // Other media buttons
        switch (data[4]) {
        case 0x5D: {
            const button = SMK25II.controller.activeDeck.cueButton;
            button.input(button.channel, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x5E: {
            const button = SMK25II.controller.activeDeck.playButton;
            button.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x5B: {
            const button = SMK25II.controller.activeDeck.backButton;
            button.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x5C: {
            const button = SMK25II.controller.activeDeck.forwardButton;
            button.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x5F: {
            const button = SMK25II.controller.recordButton;
            button.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x2E: {
            const button = SMK25II.controller.deckLeftButton;
            button.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x2F: {
            const button = SMK25II.controller.deckRightButton;
            button.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x4C: {
            const button = SMK25II.controller.activeDeck.loopButton;
            button.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        default:
            console.log(`unrecognized Sysex command: ${data[4]}`);
        }
    };

    /**
     *
     * @param data the full sysex message data.
     */
    const decodePots = function(data) {
        switch (data[3]) {
        case 0x60: {
            const knob = SMK25II.controller.activeDeck.gainKnob;
            knob.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x61: {
            const knob = SMK25II.controller.activeDeck.highKnob;
            knob.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x62: {
            const knob = SMK25II.controller.activeDeck.midKnob;
            knob.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x63: {
            const knob = SMK25II.controller.activeDeck.lowKnob;
            knob.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x64: {
            const knob = SMK25II.controller.activeDeck.effectKnob;
            knob.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x65: {
            const knob = SMK25II.controller.xfadeKnob;
            knob.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x66: {
            const knob = SMK25II.controller.headGainKnob;
            knob.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        case 0x67: {
            const knob = SMK25II.controller.gainKnob;
            knob.input(undefined, undefined, data[5], undefined, undefined);
            break;
        }
        }
    };
    /**
     *
     * @param data the sysex message data
     * @param _length the length of the data (no longer used)
     */
    SMK25II.incomingData = function(data, _length) {
        if (data.length < 6) {
            console.log(`expected sysex packet of length 6, got ${data.length}`);
            return;
        }
        for (let n = 0; n < SysexHeader.length; n++) {
            if (data[n] !== SysexHeader[n]) {
                console.log("unknown sysex packet");
                return;
            }
        }
        if (data[data.length - 1] !== 0xF7) {
            console.log("sysex packet missing trailer");
            return;
        }
        if (data[3] === 0x10) {
            decodeButton(data);
        } else if (data[3] >= 0x60 && data[3] <= 0x67) {
            decodePots(data);
        } else {
            console.log(`unrecognized sysex byte: ${data[3]}`);
        }
    };
})(SMK25II || (SMK25II = {}));
