// eslint-disable-next-line no-var
var g4v = {};

g4v.init = function() {
    // Controls not linked to any mixer or deck
    g4v.otherControls = new g4v.OtherControls();
    // Four Mixer Strips
    g4v.deck1Mixer = new g4v.MixerStrip(1);
    g4v.deck2Mixer = new g4v.MixerStrip(2);
    g4v.deck3Mixer = new g4v.MixerStrip(3);
    g4v.deck4Mixer = new g4v.MixerStrip(4);
    // Two decks
    g4v.leftDeck = new g4v.Deck([1, 3], 1);
    g4v.rightDeck = new g4v.Deck([2, 4], 2);

    // TO ensure that efects are ready
    engine.setParameter("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 0);
    engine.setParameter("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 0);
    engine.setParameter("[EffectRack1_EffectUnit3]", "group_[Channel3]_enable", 0);
    engine.setParameter("[EffectRack1_EffectUnit4]", "group_[Channel4]_enable", 0);
    engine.setParameter("[EffectRack1_EffectUnit1_Effect1]", "enabled", 1);
    engine.setParameter("[EffectRack1_EffectUnit2_Effect1]", "enabled", 1);
    engine.setParameter("[EffectRack1_EffectUnit3_Effect1]", "enabled", 1);
    engine.setParameter("[EffectRack1_EffectUnit4_Effect1]", "enabled", 1);

};

g4v.shutdown = function()  {
    // Shutdown components
    g4v.otherControls.shutdown();
    g4v.deck1Mixer.shutdown();
    g4v.deck1Mixer.shutdown();
    g4v.deck2Mixer.shutdown();
    g4v.deck3Mixer.shutdown();
    g4v.deck4Mixer.shutdown();
    g4v.leftDeck.shutdown();
    g4v.rightDeck.shutdown();

    // Switch off leds not connected to controls
    midi.sendShortMsg(0x90, 0x1B, 0x00);	// Left HotCue pad mode select button
    midi.sendShortMsg(0x90, 0x1C, 0x00);	// Left Auto Loop pad mode select button
    midi.sendShortMsg(0x90, 0x1D, 0x00);	// Left Sample pad mode select button
    midi.sendShortMsg(0x90, 0x1E, 0x00);	// Left Loop Roll pad mode select button
    midi.sendShortMsg(0x90, 0x26, 0x00);	// Left Deck select button
    midi.sendShortMsg(0x91, 0x1B, 0x00);	// Right HotCue pad mode select button
    midi.sendShortMsg(0x91, 0x1C, 0x00);	// Right Auto Loop pad mode select button
    midi.sendShortMsg(0x91, 0x1D, 0x00);	// Right Sample pad mode select button
    midi.sendShortMsg(0x91, 0x1E, 0x00);	// Right Loop Roll pad mode select button
    midi.sendShortMsg(0x91, 0x26, 0x00);	// Right Deck select button
};

// Custom deck definition
g4v.Deck = function(deckNumbers, midiChannel) {
    components.Deck.call(this, deckNumbers);
    this.cueBtn = new components.CueButton([0x8F + midiChannel, 0x02]);
    this.cupBtn = new components.Button({
        midi: [0x8F + midiChannel, 0x03],
        inKey: "cue_gotoandplay",
    });
    this.playBtn = new components.PlayButton([0x8F + midiChannel, 0x01]);
    this.syncBtn = new components.SyncButton([0x8F + midiChannel, 0x04]);
    this.tempoPot = new components.Pot({
        midi: [0x8F + midiChannel, 0x01],
        inKey: "rate",
        shift: function() { this.inKey = "pitch"; },
        unshift: function() { this.inKey = "rate"; },
    });
    this.keyLockBtn = new components.Button({
        midi: [0x8F + midiChannel, 0x05],
        key: "keylock",
        type: components.Button.prototype.types.toggle,
    });
    this.quantizeBtn = new components.Button({
        midi: [0x8F + midiChannel, 0x07],
        key: "quantize",
        type: components.Button.prototype.types.toggle,
    });
    this.slipBtn = new components.Button({
        midi: [0x8F + midiChannel, 0x19],
        key: "slip_enabled",
        type: components.Button.prototype.types.toggle,
        deck: this,
        shift: function() {
            // TODO - Chenge to next chain when #2618 is merged
            //this.group = '[EffectRack1_EffectUnit'+script.deckFromGroup(this.deck.currentDeck)+']';
            //this.inKey = 'next_chain';
            this.group = "[EffectRack1_EffectUnit"+script.deckFromGroup(this.deck.currentDeck)+"_Effect1]";
            this.inKey = "next_effect";
        },
        unshift: function() {
            this.group = this.deck.currentDeck;
            this.inKey = "slip_enable";
        },
    });
    this.fxOnOffBtn = new components.Button({
        midi: [0x8F + midiChannel, 0x1A],
        group: "[EffectRack1_EffectUnit"+script.deckFromGroup(this.currentDeck)+"]",
        key: "group_"+this.currentDeck+"_enable",
        type: components.Button.prototype.types.toggle,
    });
    // Loop Move is a pot not an encoder so requires special treatment
    this.loopMovePot = new components.Component({
        key: "loop_move",
        oldValue: 0,
        input: function(_channel, _control, value, _status, _group) {
            this.inSetParameter(value-this.oldValue);
            this.oldValue = value;
        },
    });
    // Sample Vol is a pot not an encoder so requires special treatment
    this.sampleVolPot = new components.Component({
        key: "loop_move",
        input: function(_channel, _control, value, _status, _group) {
            for (let i = 1; i < 65; i++) {
                engine.setValue("[Sampler"+i+"]", "volume", value/0x7f);
            }
        },
    });
    this.fxSuperPot = new components.Pot({
        group: "[EffectRack1_EffectUnit"+script.deckFromGroup(this.currentDeck)+"]",
        key: "super1",
    });
    this.fxMixPot = new components.Pot({
        group: "[EffectRack1_EffectUnit"+script.deckFromGroup(this.currentDeck)+"]",
        key: "mix",
    });

    // Layers for pads
    this.padsCue = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.padsCue["pad"+i] = new components.HotcueButton({
            midi: [0x8F + midiChannel, 0x08 + i],
            group: this.currentDeck,
            number: i,
        });
    }
    this.padsLoop = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.padsLoop["pad"+i] = new components.Button({
            group: this.currentDeck,
            midi: [0x8F + midiChannel, 0x08 + i],
            inKey: "beatloop_"+(0.125*Math.pow(2, i-1))+"_toggle",
            outKey: "beatloop_"+(0.125*Math.pow(2, i-1))+"_enabled",
        });
    }
    this.padsLoopRoll = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.padsLoopRoll["pad"+i] = new components.Button({
            group: this.currentDeck,
            midi: [0x8F + midiChannel, 0x08 + i],
            inKey: "beatlooproll_"+(0.125*Math.pow(2, i-1))+"_activate",
            outKey: "beatloop_"+(0.125*Math.pow(2, i-1))+"_enabled",
        });
    }
    this.padsBeatjump = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.padsBeatjump["pad"+i] = new components.Button({
            // Save jump to avoid weird recalculations
            jump: (0.125*Math.pow(2, i-1)),
            group: this.currentDeck,
            inKey: "beatjump_"+(0.125*Math.pow(2, i-1))+"_forward",
            outKey: "beatjump_"+(0.125*Math.pow(2, i-1))+"_forward",
            shift: function() { this.inKey = "beatjump_"+this.jump+"_backward"; },
            unshift: function() { this.inKey = "beatjump_"+this.jump+"_forward"; },
        });
    }
    this.padsManualLoop = new components.ComponentContainer({
        pad1: new components.Button({
            midi: [0x8F + midiChannel, 0x09],
            group: this.currentDeck,
            inKey: "loop_in",
            outKey: "eject",
        }),
        pad2: new components.Button({
            midi: [0x8F + midiChannel, 0x0A],
            group: this.currentDeck,
            inKey: "loop_out",
            outKey: "eject",
        }),
        pad3: new components.Button({
            midi: [0x8F + midiChannel, 0x0B],
            group: this.currentDeck,
            inKey: "reloop_toggle",
            outKey: "loop_enabled",
            type: components.Button.prototype.types.toggle,
        }),
        pad4: new components.Button({
            midi: [0x8F + midiChannel, 0x0C],
            group: this.currentDeck,
            inKey: "reloop_andstop",
            outKey: "eject",
        }),
        pad5: new components.Button({
            midi: [0x8F + midiChannel, 0x0D],
            group: this.currentDeck,
            inKey: "loop_halve",
            outKey: "eject",
        }),
        pad6: new components.Button({
            midi: [0x8F + midiChannel, 0x0E],
            group: this.currentDeck,
            inKey: "loop_double",
            outKey: "eject",
        }),
        pad7: new components.Button({
            midi: [0x8F + midiChannel, 0x0F],
            group: this.currentDeck,
            inKey: "loop_move_1_backward",
            outKey: "eject",
        }),
        pad8: new components.Button({
            midi: [0x8F + midiChannel, 0x10],
            group: this.currentDeck,
            inKey: "loop_move_1_forward",
            outKey: "eject",
        }),
    });

    // Pad selection buttons (not connected a CO)
    this.padModeBtn = new components.Component({
        deck: this,
        input: function(_channel, control, value, _status, _group) {
            if (!value) { return; }
            switch (control) {
            case 0x1B:
                this.deck.padMode(g4v.Deck.prototype.modes.cue);
                break;
            case 0x1C:
                this.deck.padMode(g4v.Deck.prototype.modes.autoLoop);
                break;
            case 0x1D:
                this.deck.padMode(g4v.Deck.prototype.modes.sample);
                break;
            case 0x1E:
                this.deck.padMode(g4v.Deck.prototype.modes.loopRoll);
                break;
            case 0x1F:
                // Not mapped, for future use
                // TODO - Four buttons to enable effect racks
                break;
            case 0x20:
                // Not mapped, for future use
                break;
            case 0x21:
                this.deck.padMode(g4v.Deck.prototype.modes.beatJump);
                break;
            case 0x22:
                this.deck.padMode(g4v.Deck.prototype.modes.manualLoop);
                break;
            }
        },
    });
    // Deck togle button (not connected to CO)
    this.deckToggleBtn = new components.Component({
        deck: this,
        input: function(_channel, _control, value, _status, _group) {
            if (!value) { return; }
            this.deck.toggle();
            const index = this.deck.deckNumbers.indexOf(parseInt(
                script.channelRegEx.exec(this.deck.currentDeck)[1]));
            if (index === 1) { midi.sendShortMsg(0x8F + midiChannel, 0x26, 0x7F); } else { midi.sendShortMsg(0x8F + midiChannel, 0x26, 0x00); }
        },
    });

    this.jogTopBtn = new components.Component({
        input: function(_channel, _control, value, _status, _group) {
            switch (value) {
            case 0x7f: {
                const intervalsPerRev = 400;
                const rpm = 30+1/3;
                const alpha = (1.0/8);
                const beta = (alpha / 32);
                engine.scratchEnable(script.deckFromGroup(this.group), intervalsPerRev, rpm, alpha, beta);
                break;
            }
            case 0x00:
                engine.scratchDisable(script.deckFromGroup(this.group), 1);
                break;
            }
        },
    });
    this.jogEnc = new components.Encoder({
        midi: [0x89 + midiChannel, 0x06],
        factor: 1.4,
        input: function(_channel, _control, value, _status, _group) {
            if (engine.isScratching(script.deckFromGroup(this.group))) {
                engine.scratchTick(script.deckFromGroup(this.group), (value-64)*this.factor);
            } else {
                engine.setValue(this.group, "jog", (value === 0x41 ? -1 : 1)*0.2);
            }
        },
        shift: function() { this.factor = 10; },
        unshift: function() { this.factor = 1.4; },
    });

    // Changes pad layer
    this.padMode = function(mode) {
        // Switch off all the mode leds
        midi.sendShortMsg(0x8F + midiChannel, 0x1B, 0x00);
        midi.sendShortMsg(0x8F + midiChannel, 0x1C, 0x00);
        midi.sendShortMsg(0x8F + midiChannel, 0x1D, 0x00);
        midi.sendShortMsg(0x8F + midiChannel, 0x1E, 0x00);

        // Switch off all pad leds
        if (this.pads !== undefined) {
            this.pads.shutdown();
        }

        switch (mode) {
        case g4v.Deck.prototype.modes.cue:
            this.pads = this.padsCue;
            midi.sendShortMsg(0x8F + midiChannel, 0x1B, 0x7F);
            break;
        case g4v.Deck.prototype.modes.autoLoop:
            this.pads = this.padsLoop;
            midi.sendShortMsg(0x8F + midiChannel, 0x1C, 0x7F);
            break;
        case g4v.Deck.prototype.modes.sample:
            this.pads = this.padsCue;
            midi.sendShortMsg(0x8F + midiChannel, 0x1D, 0x7F);
            break;
        case g4v.Deck.prototype.modes.loopRoll:
            this.pads = this.padsLoopRoll;
            midi.sendShortMsg(0x8F + midiChannel, 0x1E, 0x7F);
            break;
        case g4v.Deck.prototype.modes.manualLoop:
            this.pads = this.padsManualLoop;
            break;
        case g4v.Deck.prototype.modes.beatJump:
            this.pads = this.padsBeatjump;
            break;
        }
        this.pads.reconnectComponents();
    };

    // Sets default mode at start
    this.padMode(g4v.Deck.prototype.modes.cue);

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = this.currentDeck;
        }
    });
};
g4v.Deck.prototype = new components.Deck();
g4v.Deck.prototype.modes = {
    cue: 0,
    autoLoop: 1,
    sample: 2,
    loopRoll: 3,
    manualLoop: 4,
    beatJump: 5,
};

g4v.MixerStrip = function(deckNumber) {
    components.ComponentContainer.call();
    // Controls in order from top to bottom in the strip
    this.meter = new components.Component({
        midi: [0xB3, 0x13 + deckNumber],
        group: "[Channel" + deckNumber +"]",
        outKey: "VuMeter",
        max: 5,
        shutdown: function() {
            this.send(0x00);
        },
    });
    this.loadBtn = new components.Button({
        midi: [0x93, 0x00 + deckNumber],
        group: "[Channel" + deckNumber + "]",
        outKey: "track_loaded",
        shift: function() { this.inKey = "eject"; },
        unshift: function() { this.inKey = "LoadSelectedTrack"; },
    });
    this.pregainPot = new components.Pot({
        midi: [0xB3, 0x00 + deckNumber],
        group: "[Channel" + deckNumber + "]",
        inKey: "pregain",
    });
    this.filterHighPot = new components.Pot({
        midi: [0xB3, 0x08 + deckNumber],
        group: "[EqualizerRack1_[Channel" + deckNumber + "]_Effect1]",
        inKey: "parameter3",
    });
    this.filterMidPot = new components.Pot({
        midi: [0xB3, 0x04 + deckNumber],
        group: "[EqualizerRack1_[Channel" + deckNumber + "]_Effect1]",
        inKey: "parameter2",
    });
    this.filterLowPot = new components.Pot({
        midi: [0xB3, 0x0C + deckNumber],
        group: "[EqualizerRack1_[Channel" + deckNumber + "]_Effect1]",
        inKey: "parameter1",
    });
    this.filterLowPot = new components.Pot({
        midi: [0xB3, 0x0C + deckNumber],
        group: "[EqualizerRack1_[Channel" + deckNumber + "]_Effect1]",
        inKey: "parameter1",
    });
    this.filterPot = new components.Pot({
        midi: [0xB3, 0x10 + deckNumber],
        group: "[QuickEffectRack1_[Channel" + deckNumber + "]]",
        inKey: "super1",
    });
    this.pflBtn = new components.Button({
        midi: [0x93, 0x0C + deckNumber],
        group: "[Channel" + deckNumber +"]",
        key: "pfl",
        type: components.Button.prototype.types.toggle,
    });
    this.volumePot = new components.Pot({
        midi: [0xB3, 0x10 + deckNumber],
        group: "[Channel" + deckNumber +"]",
        inKey: "volume",
    });
    this.orientationLeftBtn = new components.Button({
        midi: [0x93, 0x04 + deckNumber],
        group: "[Channel" + deckNumber +"]",
        key: "orientation",
        input: function(_channel, _control, value, _status, _group) {
            if (!value) { return; }
            this.inSetValue(this.inGetValue() !== 0 ? 0 : 1);
        },
        output: function(value, _group, _control) {
            this.send(value === 0 ? this.on : this.off);
        },
    });
    this.orientationRightBtn = new components.Button({
        midi: [0x93, 0x08 + deckNumber],
        group: "[Channel" + deckNumber +"]",
        key: "orientation",
        input: function(_channel, _control, value, _status, _group) {
            if (!value) { return; }
            this.inSetValue(this.inGetValue() !== 2 ? 2 : 1);
        },
        output: function(value, _group, _control) {
            this.send(value === 2 ? this.on : this.off);
        },
    });
    this.reconnectComponents();
};
g4v.MixerStrip.prototype = new components.ComponentContainer();

g4v.OtherControls = function() {
    components.ComponentContainer.call();
    this.libraryEnc = new components.Encoder({
        midi: [0xB3, 0x1E],
        group: "[Library]",
        shift: function() { this.inKey = "MoveHorizontal"; },
        unshift: function() { this.inKey = "MoveVertical"; },
        inValueScale: function(value) { return value === 0x41 ? 1 : -1; },
        input: function(_channel, _control, value, _status, _group) {
            if (engine.getValue("[PreviewDeck1]", "play")) {
                engine.setValue("[PreviewDeck1]", (value === 0x41 ? "beatjump_4_forward" : "beatjump_4_backward"), 1);
            } else {
                this.inSetParameter(this.inValueScale(value));
            }
        },
    });
    this.libraryBtn = new components.Component({
        group: "[Library]",
        shift: function() {
            this.input = function(_channel, _control, value, _status, _group) {
                if (value === 0x00) { return; }
                if (engine.getValue("[PreviewDeck1]", "play", 1)) {
                    engine.setValue("[PreviewDeck1]", "play", 0);
                    engine.beginTimer(
                        100, function() {
                            engine.setValue("[PreviewDeck1]", "eject", 1);
                            engine.setValue("[PreviewDeck1]", "eject", 0);
                        },
                        true);
                } else {
                    engine.setValue("[PreviewDeck1]", "LoadSelectedTrack", 1);
                    engine.setValue("[PreviewDeck1]", "play", 1);
                    engine.beginTimer(100, function() { engine.setValue("[PreviewDeck1]", "play", 1); }, true);
                }
            };
        },
        unshift: function() {
            this.input = components.Component.input;
        },
    });
    this.libraryBackBtn = new components.Button({
        midi: [0x93, 0x12],
        shift: function() {
            this.group = "[Master]";
            this.inKey = "maximize_library";
            this.type = components.Button.prototype.types.toggle;
        },
        unshift: function() {
            this.group = "[Library]";
            this.inKey = "MoveFocusForward";
            this.type = components.Button.prototype.types.push;
        },
    });
    this.masterVolumePot = new components.Pot({
        midi: [0xB3, 0x1B],
        group: "[Master]",
        inKey: "volume",
    });
    this.boothVolumePot = new components.Pot({
        midi: [0xB3, 0x1C],
        group: "[Master]",
        inKey: "booth_gain",
    });
    this.headMixPot = new components.Pot({
        midi: [0xB3, 0x1A],
        group: "[Master]",
        inKey: "headMix",
    });
    this.meterR = new components.Component({
        midi: [0xB3, 0x18],
        group: "[Master]",
        outKey: "VuMeterR",
        max: 8,
    });
    this.meterL = new components.Component({
        midi: [0xB3, 0x19],
        group: "[Master]",
        outKey: "VuMeterL",
        max: 8,
    });
    this.crossfaderPot = new components.Pot({
        midi: [0xB3, 0x19],
        group: "[Master]",
        inKey: "crossfader",
        invert: 1,
    });
    // Extra controls
    this.shiftBtn = new components.Component({
        input: function(_channel, _control, value, _status, _group) {
            if (value === 0x7F) {
                g4v.otherControls.shift();
                g4v.deck1Mixer.shift();
                g4v.deck2Mixer.shift();
                g4v.deck3Mixer.shift();
                g4v.deck4Mixer.shift();
                g4v.leftDeck.shift();
                g4v.rightDeck.shift();
            } else {
                g4v.otherControls.unshift();
                g4v.deck1Mixer.unshift();
                g4v.deck2Mixer.unshift();
                g4v.deck3Mixer.unshift();
                g4v.deck4Mixer.unshift();
                g4v.leftDeck.unshift();
                g4v.rightDeck.unshift();
            }
        },
    });
    this.reconnectComponents();
};
g4v.OtherControls.prototype = new components.ComponentContainer();
