var DJ2GO2Touch = {};
DJ2GO2Touch.ControllerStatusSysex = [0xF0, 0x00, 0x01, 0x3F, 0x3C, 0x48, 0xF7];
DJ2GO2Touch.padsPerDeck = 4;
DJ2GO2Touch.samplerCount = 2 * DJ2GO2Touch.padsPerDeck;

DJ2GO2Touch.init = function() {
    if (engine.getValue("[App]", "num_samplers") < DJ2GO2Touch.samplerCount) {
        engine.setValue("[App]", "num_samplers", DJ2GO2Touch.samplerCount);
    }
    DJ2GO2Touch.leftDeck = new DJ2GO2Touch.Deck([1], 0);
    DJ2GO2Touch.rightDeck = new DJ2GO2Touch.Deck([2], 1);
    midi.sendSysexMsg(DJ2GO2Touch.ControllerStatusSysex, DJ2GO2Touch.ControllerStatusSysex.length);
};

DJ2GO2Touch.shutdown = function() {
    DJ2GO2Touch.leftDeck.shutdown();
    DJ2GO2Touch.rightDeck.shutdown();
};

DJ2GO2Touch.browseEncoder = new components.Encoder({
    previewSeekEnabled: false,
    onKnobEvent: function(rotateValue) {
        if (rotateValue !== 0) {
            if (this.previewSeekEnabled  && engine.getValue("[PreviewDeck1]", "play")) {
                var oldPos = engine.getValue("[PreviewDeck1]", "playposition");
                var newPos = Math.max(0, oldPos + (0.05 * rotateValue));
                engine.setValue("[PreviewDeck1]", "playposition", newPos);
            } else if (!this.previewSeekEnabled) {
                engine.setValue("[Playlist]", "SelectTrackKnob", rotateValue);
            }
        }
    },
    onButtonEvent: function() {
        if (engine.getValue("[PreviewDeck1]", "play")) {
            script.triggerControl("[PreviewDeck1]", "stop");
            this.previewSeekEnabled = false;
        } else {
            engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 1);
            this.previewSeekEnabled = true;
        }
    },
    input: function(channel, control, value, status, _group) {
        switch (status) {
        case 0xBF:
            var rotateValue = (value === 127) ? -1 : ((value === 1) ? 1 : 0);
            this.onKnobEvent(rotateValue);
            break;
        case 0x9F:
            this.onButtonEvent();
        }
    }
});

DJ2GO2Touch.Deck = function(deckNumbers, midiChannel) {
    components.Deck.call(this, deckNumbers);
    this.playButton = new components.PlayButton([0x90 + midiChannel, 0x00]);
    this.cueButton = new components.CueButton([0x90 + midiChannel, 0x01]);
    this.syncButton = new components.SyncButton([0x90 + midiChannel, 0x02]);

    this.pflButton = new components.Button({
        midi: [0x90 + midiChannel, 0x1B],
        key: "pfl"
    });

    this.loadButton = new components.Button({
        midi: [0x9F, 0x02 + midiChannel],
        key: "LoadSelectedTrack",
        input: function(channel, control, value, status, _group) {
            this.send(this.isPress(channel, control, value, status) ? this.on : this.off);
            components.Button.prototype.input.apply(this, arguments);
        }
    });

    this.preGain = new components.Pot({
        midi: [0xB0 + midiChannel, 0x16],
        group: "[QuickEffectRack1_" + this.currentDeck + "]",
        key: "super1"
    });

    this.tempoFader = new components.Pot({
        group: "[Channel" + script.deckFromGroup(this.currentDeck) + "]",
        midi: [0xB0 + midiChannel, 0x09],
        key: "rate",
        invert: true
    });

    this.hotcueButtons = [];
    this.samplerButtons = [];
    this.beatloopButtons = [];
    for (var i = 1; i <= 4; i++) {
        this.hotcueButtons[i] = new components.HotcueButton({
            group: "[Channel" + script.deckFromGroup(this.currentDeck) + "]",
            midi: [0x94 + midiChannel, 0x00 + i],
            number: i,
            input: function(channel, control, value, _status, _group) {
                // DJ2GO2/DJ2GO2 Touch does not have a "shift" button,
                // but when in CUES Pad Mode, holding the PAD MODE button will
                // cause the controller to send messages where the originating midi
                // control is offset by 8.
                // this "feature" only applies to the CUES PAD MODE. Holding the button
                // in other modes does nothing.
                if (control <= 8) {
                    engine.setValue(this.group, "hotcue_" + this.number + "_activate", value);
                } else {
                    engine.setValue(this.group, "hotcue_" + this.number + "_clear", value);
                }
            },
        });
        var sampler = i + (midiChannel * DJ2GO2Touch.padsPerDeck);
        this.samplerButtons[i] = new components.SamplerButton({
            group: "[Sampler" + sampler + "]",
            midi: [0x94 + midiChannel, 0x30 + i],
            number: sampler
        });
        this.beatloopButtons[i] = new components.Button({
            group: "[Channel" + script.deckFromGroup(this.currentDeck) + "]",
            midi: [0x94 + midiChannel, 0x10 + i],
            number: i,
            key: "beatloop_" + Math.pow(2, i-1) + "_toggle"
        });
    }

    this.loopIn = new components.Button({
        midi: [0x94 + midiChannel, 0x21],
        key: "loop_in",
    });

    this.loopOut = new components.Button({
        midi: [0x94 + midiChannel, 0x22],
        key: "loop_out",
    });

    this.LoopToggleButton = new components.LoopToggleButton([0x94 + midiChannel, 0x23]);

    this.reLoopStop = new components.Button({
        midi: [0x94 + midiChannel, 0x24],
        key: "reloop_andstop",
    });

    this.wheelTouch = function(channel, control, value, status, _group) {
        if ((status & 0xF0) === 0x90) {
            var alpha = 1.0/8;
            var beta = alpha/32;
            engine.scratchEnable(script.deckFromGroup(this.currentDeck), 236, 33+1/3, alpha, beta);
        } else {
            engine.scratchDisable(script.deckFromGroup(this.currentDeck));
        }
    };

    this.wheelTurn = function(channel, control, value, _status, _group) {
        // When the jog wheel is turned in counter-clockwise direction, value is
        // greater than 64 (= 0x40). If it's turned in clockwise
        // direction, the value is smaller than 64.
        var newValue = value > 64 ? (value - 128) : value;
        var deck = script.deckFromGroup(this.currentDeck);
        if (engine.isScratching(deck)) {
            engine.scratchTick(deck, newValue); // Scratch!
        } else {
            engine.setValue(this.currentDeck, "jog", newValue); // Pitch bend
        }
    };

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = this.currentDeck;
        }
    });
};

DJ2GO2Touch.Deck.prototype = new components.Deck();
