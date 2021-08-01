// dim all lights when inactive instead of turning them off
components.Button.prototype.off = 0x01;

var MixtrackProFX = {};

// pitch ranges
// add/remove/modify steps to your liking
// default step must be set in Mixxx settings
// setting is stored per deck in pitchRange.currentRangeIdx
MixtrackProFX.pitchRanges = [0.08, 0.16, 1];

// whether the corresponding Mixxx option is enabled
// (Settings -> Preferences -> Waveforms -> Synchronize zoom level across all waveforms)
MixtrackProFX.waveformsSynced = true;

// jogwheel
MixtrackProFX.jogScratchSensitivity = 1024;
MixtrackProFX.jogScratchAlpha = 1; // do NOT set to 2 or higher
MixtrackProFX.jogScratchBeta = 1/32;
MixtrackProFX.jogPitchSensitivity = 10;
MixtrackProFX.jogSeekSensitivity = 10000;

// blink settings
MixtrackProFX.enableBlink = true;
MixtrackProFX.blinkDelay = 700;

// state variables, don't touch
MixtrackProFX.shifted = false;
MixtrackProFX.scratchModeEnabled = [true, true];

MixtrackProFX.init = function() {
    // effects for both decks
    MixtrackProFX.effect = new components.ComponentContainer();
    MixtrackProFX.effect[0] = new MixtrackProFX.EffectUnit(1);
    MixtrackProFX.effect[1] = new MixtrackProFX.EffectUnit(2);

    // decks
    MixtrackProFX.deck = new components.ComponentContainer();
    MixtrackProFX.deck[0] = new MixtrackProFX.Deck(1);
    MixtrackProFX.deck[1] = new MixtrackProFX.Deck(2);

    MixtrackProFX.browse = new MixtrackProFX.Browse();
    MixtrackProFX.headGain = new MixtrackProFX.HeadGain();

    var exitDemoSysex = [0xF0, 0x7E, 0x00, 0x06, 0x01, 0xF7];
    midi.sendSysexMsg(exitDemoSysex, exitDemoSysex.length);

    var statusSysex = [0xF0, 0x00, 0x20, 0x7F, 0x03, 0x01, 0xF7];
    midi.sendSysexMsg(statusSysex, statusSysex.length);

    // enables 4 bottom pads fader cuts
    var faderCutSysex = [0xF0, 0x00, 0x20, 0x7F, 0x03, 0xF7];
    midi.sendSysexMsg(faderCutSysex, faderCutSysex.length);

    // initialize leds for both decks
    for (var i = 0; i < 2; i++) {
        midi.sendShortMsg(0x90 + i, 0x00, 0x01); // play
        midi.sendShortMsg(0x90 + i, 0x01, 0x01); // cue
        midi.sendShortMsg(0x90 + i, 0x02, 0x01); // sync
        midi.sendShortMsg(0x90 + i, 0x07, 0x7f); // scratch
        midi.sendShortMsg(0x90 + i, 0x1B, 0x01); // pfl

        midi.sendShortMsg(0x94 + i, 0x00, 0x7F); // hotcue
        midi.sendShortMsg(0x94 + i, 0x0D, 0x01); // auto loop
        midi.sendShortMsg(0x94 + i, 0x07, 0x01); // fader cuts
        midi.sendShortMsg(0x94 + i, 0x0B, 0x01); // sample

        midi.sendShortMsg(0x94 + i, 0x34, 0x01); // half
        midi.sendShortMsg(0x94 + i, 0x35, 0x01); // double
        midi.sendShortMsg(0x94 + i, 0x40, 0x01); // loop

        // initialize leds for shifted buttons
        midi.sendShortMsg(0x90 + i, 0x04, 0x01); // play
        midi.sendShortMsg(0x90 + i, 0x05, 0x01); // cue
        midi.sendShortMsg(0x90 + i, 0x03, 0x01); // sync
        midi.sendShortMsg(0x94 + i, 0x0F, 0x01); // sample shifted
        midi.sendShortMsg(0x94 + i, 0x02, 0x01); // beatjump
        midi.sendShortMsg(0x90 + i, 0x08, 0x01); // bleep
        midi.sendShortMsg(0x94 + i, 0x36, 0x01); // half
        midi.sendShortMsg(0x94 + i, 0x37, 0x01); // double
        midi.sendShortMsg(0x94 + i, 0x41, 0x01); // loop

        // pads
        for (var j = 0; j < 8; j++) {
            midi.sendShortMsg(0x94 + i, 0x14 + j, 0x01);
            midi.sendShortMsg(0x94 + i, 0x1C + j, 0x01); // shifted
        }
    }

    midi.sendShortMsg(0x88, 0x09, 0x01); // tap led

    // check if quantize is enabled
    var quantizeEnabled = engine.getValue("[Channel1]", "quantize");

    // effect leds
    midi.sendShortMsg(0x88, 0x00, 0x01); // hpf
    midi.sendShortMsg(0x88, 0x01, 0x01); // lpf
    midi.sendShortMsg(0x88, 0x02, 0x01); // flanger
    midi.sendShortMsg(0x89, 0x03, 0x01); // echo
    midi.sendShortMsg(0x89, 0x04, quantizeEnabled ? 0x7F : 0x01); // reverb
    midi.sendShortMsg(0x89, 0x05, 0x01); // phaser

    // vumeters leds (off)
    midi.sendShortMsg(0xB0, 0x1F, 0x00);
    midi.sendShortMsg(0xB1, 0x1F, 0x00);

    engine.makeConnection("[Channel1]", "VuMeter", MixtrackProFX.vuCallback);
    engine.makeConnection("[Channel2]", "VuMeter", MixtrackProFX.vuCallback);
};

MixtrackProFX.shutdown = function() {
    var shutdownSysex = [0xF0, 0x00, 0x20, 0x7F, 0x02, 0xF7];
    midi.sendSysexMsg(shutdownSysex, shutdownSysex.length);
};

MixtrackProFX.EffectUnit = function(unitNumber) {
    this.unitNumber = unitNumber;
    this.group = "[EffectRack1_EffectUnit" + unitNumber + "]";

    this.enableButton = new components.Button({
        input: function(channel, control, value, status, group) {
            // note: value is 2 when the switch is held down (1 when up)
            engine.setValue(group, "enabled", value);
        }
    });

    this.dryWetKnob = new components.Pot({
        group: this.group,
        inKey: "mix"
    });

    this.tap = new components.Button({
        group: "[Channel" + this.unitNumber + "]",
        key: "bpm_tap",
        midi: [0x88, 0x09]
    });

    this.effectParam = new components.Encoder({
        group: "[EffectRack1_EffectUnit" + unitNumber + "_Effect1]",
        inKey: "parameter1",
        shift: function() {
            this.inKey = "parameter2";
        },
        unshift: function() {
            this.inKey = "parameter1";
        },
        input: function(channel, control, value) {
            if (value === 0x01) {
                this.inSetParameter(this.inGetParameter() + 0.05);
            } else if (value === 0x7F) {
                this.inSetParameter(this.inGetParameter() - 0.05);
            }
        }
    });
};

MixtrackProFX.EffectUnit.prototype = new components.ComponentContainer();

MixtrackProFX.Deck = function(number) {
    var deck = this;
    var channel = number - 1;
    var blinkTimer = 0;
    var blinkLedState = true;

    components.Deck.call(this, number);

    this.playButton = new components.PlayButton({
        midi: [0x90 + channel, 0x00]
    });

    this.playButtonStutter = new components.Button({
        inKey: "play_stutter"
    });

    this.cueButton = new components.CueButton({
        midi: [0x90 + channel, 0x01]
    });

    this.cueButtonShift = new components.Button({
        inKey: "start_stop"
    });

    this.syncButton = new components.SyncButton({
        midi: [0x90 + channel, 0x02]
    });

    this.pflButton = new components.Button({
        type: components.Button.prototype.types.toggle,
        midi: [0x90 + channel, 0x1B],
        key: "pfl"
    });

    this.loadButton = new components.Button({
        inKey: "LoadSelectedTrack"
    });

    this.volume = new components.Pot({
        group: this.currentDeck,
        inKey: "volume"
    });

    this.treble = new components.Pot({
        group: "[EqualizerRack1_" + this.currentDeck + "_Effect1]",
        inKey: "parameter3"
    });

    this.mid = new components.Pot({
        group: "[EqualizerRack1_" + this.currentDeck + "_Effect1]",
        inKey: "parameter2"
    });

    this.bass = new components.Pot({
        group: "[EqualizerRack1_" + this.currentDeck + "_Effect1]",
        inKey: "parameter1"
    });

    this.filter = new components.Pot({
        group: "[QuickEffectRack1_" + this.currentDeck + "]",
        inKey: "super1"
    });

    this.gain = new components.Pot({
        inKey: "pregain"
    });

    this.pitch = new components.Pot({
        inKey: "rate",
        invert: true
    });

    this.pads = new components.ComponentContainer();
    this.padsShift = new components.ComponentContainer();

    for (var i = 0; i < 8; i++) {
        this.pads[i] = new components.Button({
            group: this.currentDeck,
            midi: [0x94 + channel, 0x14 + i],
            inKey: "hotcue_" + (i + 1) + "_activate",
            outKey: "hotcue_" + (i + 1) + "_enabled",
            number: i,
            shift: function() {
                this.midi = [0x94 + channel, 0x1C + this.number];
            },
            unshift: function() {
                this.midi = [0x94 + channel, 0x14 + this.number];
            }
        });

        this.padsShift[i] = new components.Button({
            group: this.currentDeck,
            inKey: "hotcue_" + (i + 1) + "_clear"
        });
    }

    // switch pad mode to hotcue
    this.modeHotcue = new components.Button({
        input: function(channel) {
            deck.blinkLedOff();
            midi.sendShortMsg(0x90 + channel, 0x00, 0x7F); // hotcue
            midi.sendShortMsg(0x90 + channel, 0x0D, 0x01); // auto loop
            midi.sendShortMsg(0x90 + channel, 0x07, 0x01); // fader cuts
            midi.sendShortMsg(0x90 + channel, 0x0B, 0x01); // sample
            midi.sendShortMsg(0x90 + channel, 0x0F, 0x01); // sample shifted
            midi.sendShortMsg(0x90 + channel, 0x02, 0x01); // beatjump

            for (var i = 0; i < 8; i++) {
                deck.pads[i].group = deck.currentDeck;
                deck.pads[i].inKey = "hotcue_" + (i + 1) + "_activate";
                deck.pads[i].outKey = "hotcue_" + (i + 1) + "_enabled";

                deck.padsShift[i].group = deck.currentDeck;
                deck.padsShift[i].inKey = "hotcue_" + (i + 1) + "_clear";
            }

            deck.pads.reconnectComponents();
        }
    });

    // switch pad mode to auto loop
    this.modeAutoloop = new components.Button({
        input: function(channel) {
            deck.blinkLedOff();
            midi.sendShortMsg(0x90 + channel, 0x00, 0x01); // hotcue
            midi.sendShortMsg(0x90 + channel, 0x0D, 0x7F); // auto loop
            midi.sendShortMsg(0x90 + channel, 0x07, 0x01); // fader cuts
            midi.sendShortMsg(0x90 + channel, 0x0B, 0x01); // sample
            midi.sendShortMsg(0x90 + channel, 0x0F, 0x01); // sample shifted
            midi.sendShortMsg(0x90 + channel, 0x02, 0x01); // beatjump

            // this is just sad
            deck.pads[0].inKey = "beatloop_0.0625_toggle";
            deck.pads[0].outKey = "beatloop_0.0625_enabled";
            deck.pads[1].inKey = "beatloop_0.125_toggle";
            deck.pads[1].outKey = "beatloop_0.125_enabled";
            deck.pads[2].inKey = "beatloop_0.25_toggle";
            deck.pads[2].outKey = "beatloop_0.25_enabled";
            deck.pads[3].inKey = "beatloop_0.5_toggle";
            deck.pads[3].outKey = "beatloop_0.5_enabled";
            deck.pads[4].inKey = "beatloop_1_toggle";
            deck.pads[4].outKey = "beatloop_1_enabled";
            deck.pads[5].inKey = "beatloop_2_toggle";
            deck.pads[5].outKey = "beatloop_2_enabled";
            deck.pads[6].inKey = "beatloop_4_toggle";
            deck.pads[6].outKey = "beatloop_4_enabled";
            deck.pads[7].inKey = "beatloop_8_toggle";
            deck.pads[7].outKey = "beatloop_8_enabled";

            deck.padsShift[0].inKey = "beatlooproll_0.0625_activate";
            deck.padsShift[1].inKey = "beatlooproll_0.125_activate";
            deck.padsShift[2].inKey = "beatlooproll_0.25_activate";
            deck.padsShift[3].inKey = "beatlooproll_0.5_activate";
            deck.padsShift[4].inKey = "beatlooproll_1_activate";
            deck.padsShift[5].inKey = "beatlooproll_2_activate";
            deck.padsShift[6].inKey = "beatlooproll_4_activate";
            deck.padsShift[7].inKey = "beatlooproll_8_activate";

            for (var i = 0; i < 8; i++) {
                deck.pads[i].group = deck.currentDeck;
                deck.padsShift[i].group = deck.currentDeck;
            }

            deck.pads.reconnectComponents();
        }
    });

    // switch pad mode to fader cuts
    this.modeFadercuts = new components.Button({
        input: function(channel) {
            deck.blinkLedOff();
            midi.sendShortMsg(0x90 + channel, 0x00, 0x01); // hotcue
            midi.sendShortMsg(0x90 + channel, 0x0D, 0x01); // auto loop
            midi.sendShortMsg(0x90 + channel, 0x07, 0x09); // fader cuts (0x09 works better than 0x7F for some reason)
            midi.sendShortMsg(0x90 + channel, 0x0B, 0x01); // sample
            midi.sendShortMsg(0x90 + channel, 0x0F, 0x01); // sample shifted
            midi.sendShortMsg(0x90 + channel, 0x02, 0x01); // beatjump

            // the "fader cuts" function is somehow burned into hardware

            // need to set the pads to *something* to not trigger controls
            // triggered previously by the pads and not display their status (e.g. hotcue set)
            // nop would be really useful in this situation
            // we can assume channel 4 is unused and therefore won't mess anything up
            for (var i = 0; i < 8; i++) {
                deck.pads[i].group = "[Channel4]";
                deck.padsShift[i].group = "[Channel4]";
            }

            deck.pads.reconnectComponents();
        }
    });

    // switch pad mode to sampler
    this.modeSample = new components.Button({
        input: function(channel) {
            deck.blinkLedOff();
            midi.sendShortMsg(0x90 + channel, 0x00, 0x01); // hotcue
            midi.sendShortMsg(0x90 + channel, 0x0D, 0x01); // auto loop
            midi.sendShortMsg(0x90 + channel, 0x07, 0x01); // fader cuts
            midi.sendShortMsg(0x90 + channel, 0x0B, 0x7F); // sample
            midi.sendShortMsg(0x90 + channel, 0x0F, 0x01); // sample shifted
            midi.sendShortMsg(0x90 + channel, 0x02, 0x01); // beatjump

            for (var i = 0; i < 8; i++) {
                deck.pads[i].group = "[Sampler" + (i + 1) + "]";
                deck.pads[i].inKey = "cue_gotoandplay";
                deck.pads[i].outKey = "play";

                deck.padsShift[i].group = "[Sampler" + (i + 1) + "]";
                deck.padsShift[i].inKey = "start_stop";
            }

            deck.pads.reconnectComponents();
        }
    });

    // switch pad mode to shifted sampler
    this.modeSampleShift = new components.Button({
        input: function(channel) {
            midi.sendShortMsg(0x90 + channel, 0x00, 0x01); // hotcue
            midi.sendShortMsg(0x90 + channel, 0x0D, 0x01); // auto loop
            midi.sendShortMsg(0x90 + channel, 0x07, 0x01); // fader cuts
            midi.sendShortMsg(0x90 + channel, 0x0B, 0x01); // sample
            midi.sendShortMsg(0x90 + channel, 0x0F, 0x7F); // sample shifted
            midi.sendShortMsg(0x90 + channel, 0x02, 0x01); // beatjump
            deck.blinkLedOn(0x90 + channel, 0x0B); // blink sample

            for (var i = 0; i < 8; i++) {
                deck.pads[i].group = "[Sampler" + (i + 9) + "]";
                deck.pads[i].inKey = "cue_gotoandplay";
                deck.pads[i].outKey = "play";

                deck.padsShift[i].group = "[Sampler" + (i + 9) + "]";
                deck.padsShift[i].inKey = "start_stop";
            }

            deck.pads.reconnectComponents();
        }
    });

    // switch pad mode to beatjump
    this.modeBeatjump = new components.Button({
        input: function(channel) {
            midi.sendShortMsg(0x90 + channel, 0x00, 0x01); // hotcue
            midi.sendShortMsg(0x90 + channel, 0x0D, 0x01); // auto loop
            midi.sendShortMsg(0x90 + channel, 0x07, 0x01); // fader cuts
            midi.sendShortMsg(0x90 + channel, 0x0B, 0x01); // sample
            midi.sendShortMsg(0x90 + channel, 0x0F, 0x01); // sample shifted
            midi.sendShortMsg(0x90 + channel, 0x02, 0x7F); // beatjump
            deck.blinkLedOn(0x90 + channel, 0x00); // blink hotcue

            deck.pads[0].inKey = "beatjump_0.0625_forward";
            deck.pads[0].outKey = "beatjump_0.0625_forward";
            deck.pads[1].inKey = "beatjump_0.125_forward";
            deck.pads[1].outKey = "beatjump_0.125_forward";
            deck.pads[2].inKey = "beatjump_0.25_forward";
            deck.pads[2].outKey = "beatjump_0.25_forward";
            deck.pads[3].inKey = "beatjump_0.5_forward";
            deck.pads[3].outKey = "beatjump_0.5_forward";
            deck.pads[4].inKey = "beatjump_1_forward";
            deck.pads[4].outKey = "beatjump_1_forward";
            deck.pads[5].inKey = "beatjump_2_forward";
            deck.pads[5].outKey = "beatjump_2_forward";
            deck.pads[6].inKey = "beatjump_forward"; // this button is adjustable, it uses the value set in the interface (4 by default)
            deck.pads[6].outKey = "beatjump_forward";
            deck.pads[7].inKey = "beatjump_8_forward";
            deck.pads[7].outKey = "beatjump_8_forward";

            deck.padsShift[0].inKey = "beatjump_0.0625_backward";
            deck.padsShift[1].inKey = "beatjump_0.125_backward";
            deck.padsShift[2].inKey = "beatjump_0.25_backward";
            deck.padsShift[3].inKey = "beatjump_0.5_backward";
            deck.padsShift[4].inKey = "beatjump_1_backward";
            deck.padsShift[5].inKey = "beatjump_2_backward";
            deck.padsShift[6].inKey = "beatjump_backward"; // this button is adjustable, it uses the value set in the interface (4 by default)
            deck.padsShift[7].inKey = "beatjump_8_backward";

            for (var i = 0; i < 8; i++) {
                deck.pads[i].group = deck.currentDeck;
                deck.padsShift[i].group = deck.currentDeck;
            }

            deck.pads.reconnectComponents();
        }
    });

    // start an infinite timer that toggles led state
    this.blinkLedOn = function(midi1, midi2) {
        if (!MixtrackProFX.enableBlink) {
            return;
        }

        deck.blinkLedOff();
        blinkLedState = true;
        blinkTimer = engine.beginTimer(MixtrackProFX.blinkDelay, function() {
            midi.sendShortMsg(midi1, midi2, blinkLedState ? 0x7F : 0x01);
            blinkLedState = !blinkLedState;
        });
    };

    // stop the blink timer
    this.blinkLedOff = function() {
        if (!MixtrackProFX.enableBlink || blinkTimer === 0) {
            return;
        }

        engine.stopTimer(blinkTimer);
        blinkTimer = 0;
    };

    this.shiftButton = new components.Button({
        input: function(channel, control, value) {
            // each shift button shifts both decks
            // more consistent with the logic burned into hardware
            if (value === 0x7F) {
                MixtrackProFX.shifted = true;
                MixtrackProFX.deck[0].shift();
                MixtrackProFX.deck[1].shift();
                MixtrackProFX.browse.shift();
                MixtrackProFX.effect.shift();
            } else if (value === 0) {
                MixtrackProFX.shifted = false;
                MixtrackProFX.deck[0].unshift();
                MixtrackProFX.deck[1].unshift();
                MixtrackProFX.browse.unshift();
                MixtrackProFX.effect.unshift();
            }

            // for displaying pads lights when shifted
            MixtrackProFX.deck[0].pads.reconnectComponents();
            MixtrackProFX.deck[1].pads.reconnectComponents();
        }
    });

    this.loop = new components.Button({
        key: "loop_enabled",
        midi: [0x94 + channel, 0x40],
        input: function(channel, control, value, status, group) {
            if (engine.getValue(group, "loop_enabled") === 0) {
                script.triggerControl(group, "beatloop_activate");
            } else {
                script.triggerControl(group, "beatlooproll_activate");
            }
        }
    });

    this.reloop = new components.Button({
        inKey: "reloop_toggle" // or loop_in_goto to not enable loop
    });

    this.loopHalf = new components.Button({
        key: "loop_halve",
        midi: [0x94 + channel, 0x34],
    });

    this.loopDouble = new components.Button({
        key: "loop_double",
        midi: [0x94 + channel, 0x35],
    });

    this.loopIn = new components.Button({
        inKey: "loop_in"
    });

    this.loopOut = new components.Button({
        inKey: "loop_out"
    });

    this.bleep = new components.Button({
        inKey: "reverseroll"
    });

    this.pitchBendUp = new components.Button({
        inKey: "rate_temp_up"
    });

    this.pitchBendDown = new components.Button({
        inKey: "rate_temp_down"
    });

    this.keylock = new components.Button({
        type: components.Button.prototype.types.toggle,
        inKey: "keylock"
    });

    this.pitchRange = new components.Button({
        currentRangeIdx: 0,
        input: function(channel, control, value, status, group) {
            this.currentRangeIdx = (this.currentRangeIdx + 1) % MixtrackProFX.pitchRanges.length;
            engine.setValue(group, "rateRange", MixtrackProFX.pitchRanges[this.currentRangeIdx]);
        }
    });

    this.prevEffect = new components.Button({
        group: "[EffectRack1_EffectUnit" + number + "_Effect1]",
        key: "prev_effect",
        midi: [0x98, channel*2],
        shift: function() {
            this.group = "[Channel" + number + "]";
            this.inKey = "pitch_up";
            this.outKey = "pitch_up";
        },
        unshift: function() {
            this.group = "[EffectRack1_EffectUnit" + number + "_Effect1]";
            this.inKey = "prev_effect";
            this.outKey = "prev_effect";
        }
    });

    this.nextEffect = new components.Button({
        group: "[EffectRack1_EffectUnit" + number + "_Effect1]",
        key: "next_effect",
        midi: [0x99, 0x03 + channel*2],
        shift: function() {
            this.group = "[Channel" + number + "]";
            this.inKey = "pitch_down";
        },
        unshift: function() {
            this.group = "[EffectRack1_EffectUnit" + number + "_Effect1]";
            this.inKey = "next_effect";
        }
    });

    this.beatsnap = new components.Button({
        type: components.Button.prototype.types.toggle,
        key: "quantize",
        midi: [0x89, 0x04]
    });

    this.reconnectComponents(function(component) {
        if (component.group === undefined) {
            component.group = this.currentDeck;
        }
    });
};

MixtrackProFX.Deck.prototype = new components.Deck();

MixtrackProFX.Browse = function() {
    this.knob = new components.Encoder({
        group: "[Library]",
        inKey: "MoveVertical",
        inValueScale: function(value) {
            return (value > 0x40) ? value - 0x80 : value;
        }
    });

    this.knobShift = new components.Encoder({
        input: function(channel, control, value) {
            if (value === 0x01) {
                engine.setParameter("[Channel1]", "waveform_zoom_down", 1);

                // need to zoom both channels if waveform sync is disabled in Mixxx settings.
                // and when it's enabled then no need to zoom 2nd channel, as it will cause
                // the zoom to jump 2 levels at once
                if (!MixtrackProFX.waveformsSynced) {
                    engine.setParameter("[Channel2]", "waveform_zoom_down", 1);
                }
            } else if (value === 0x7F) {
                engine.setParameter("[Channel1]", "waveform_zoom_up", 1);

                // see above comment
                if (!MixtrackProFX.waveformsSynced) {
                    engine.setParameter("[Channel2]", "waveform_zoom_up", 1);
                }
            }
        }
    });

    this.knobButton = new components.Button({
        group: "[Library]",
        inKey: "MoveFocusForward"
    });

    this.buttonShift = new components.Button({
        group: "[Library]",
        inKey: "GoToItem"
    });

    this.setBeatgrid = new components.Button({
        group: "[Channel1]",
        key: "beats_translate_curpos",
        midi: [0x88, 0x01],
        shift: function() {
            this.group = "[Channel2]";
        },
        unshift: function() {
            this.group = "[Channel1]";
        }
    });
};

MixtrackProFX.Browse.prototype = new components.ComponentContainer();

MixtrackProFX.HeadGain = function() {
    components.Pot.call(this);
};

MixtrackProFX.HeadGain.prototype = new components.Pot({
    group: "[Master]",
    inKey: "headGain"
});

MixtrackProFX.vuCallback = function(value, group) {
    var level = value * 90;

    if (engine.getValue("[Channel1]", "pfl") || engine.getValue("[Channel2]", "pfl")) {
        if (group === "[Channel1]") {
            midi.sendShortMsg(0xB0, 0x1F, level);
        } else if (group === "[Channel2]") {
            midi.sendShortMsg(0xB1, 0x1F, level);
        }
    } else if (group === "[Channel1]") {
        midi.sendShortMsg(0xB0, 0x1F, level);
    } else if (group === "[Channel2]") {
        midi.sendShortMsg(0xB1, 0x1F, level);
    }
};

MixtrackProFX.scratchToggle = function(channel) {
    MixtrackProFX.scratchModeEnabled[channel] = !MixtrackProFX.scratchModeEnabled[channel];
    midi.sendShortMsg(0x90 | channel, 0x07, MixtrackProFX.scratchModeEnabled[channel] ? 0x7F : 0x01);
};

MixtrackProFX.wheelTouch = function(channel, control, value) {
    var deckNumber = channel + 1;

    if (!MixtrackProFX.shifted && MixtrackProFX.scratchModeEnabled[channel] && value === 0x7F) {
        // touch start

        engine.scratchEnable(deckNumber, MixtrackProFX.jogScratchSensitivity, 33+1/3, MixtrackProFX.jogScratchAlpha, MixtrackProFX.jogScratchBeta, true);
    } else if (value === 0) {
        // touch end
        engine.scratchDisable(deckNumber, true);
    }
};

MixtrackProFX.wheelTurn = function(channel, control, value, status, group) {
    var deckNumber = channel + 1;

    var newValue = value;

    if (value >= 64) {
        // correct the value if going backwards
        newValue -= 128;
    }

    if (MixtrackProFX.shifted) {
        // seek
        var oldPos = engine.getValue(group, "playposition");

        if (oldPos <= 0) {
            oldPos = 0;
        }

        if (oldPos >= 1) {
            oldPos = 1;
        }

        engine.setValue(group, "playposition", oldPos + newValue / MixtrackProFX.jogSeekSensitivity);
    } else if (MixtrackProFX.scratchModeEnabled[channel] && engine.isScratching(deckNumber)) {
        // scratch
        engine.scratchTick(deckNumber, newValue);
    } else {
        // pitch bend
        engine.setValue(group, "jog", newValue / MixtrackProFX.jogPitchSensitivity);
    }
};
