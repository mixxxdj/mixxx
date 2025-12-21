// //////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
// //////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
// //////////////////////////////////////////////////////////////////////


/******************
 * CONFIG OPTIONS *
 ******************/

var EnableWheel = engine.getSetting("EnableWheel");
var ShiftLoadEjects = engine.getSetting("ShiftLoadEjects");
var ShowFocusedEffectParameters = engine.getSetting("ShowFocusedEffectParameters");
var OnlyActiveDeckEffect = engine.getSetting("OnlyActiveDeckEffect");
var displayVUFromBothDecks = engine.getSetting("displayVUFromBothDecks");
var useFadercutsAsStems = engine.getSetting("useFadercutsAsStems");

/**
 * Creates a configuration object for a performance pad to be used for stem control.
 * This function handles:
 * - Tapping a pad to toggle the mute state of a stem.
 * - Holding a pad to allow volume adjustment of the stem with the BEATS knob.
 * - Shift-tapping a pad to toggle the QuickEffect on the stem.
 * - LED feedback for the stem's mute state.
 * @param {object} deckInstance - The deck object this pad belongs to.
 * @param {string} padStateProperty - The name of the property on the deck object that holds the state for this pad.
 */
function createStemPadConfig(deckInstance, padStateProperty, stemNumber, midiDetails) {
    var fullMidi = [0x94 + midiDetails.channel, midiDetails.note];

    return {
        midi: fullMidi,
        type: components.Button.prototype.types.toggle,
        inKey: 'mute',
        group: `[Channel${deckInstance.number}_Stem${stemNumber}]`,
        on: 0x7F,
        off: 0x01,
        // This function is now primarily for setting the LED from the script,
        // not just toggling. The connect function handles state-based LED updates.
        output: function (value) {
            if (deckInstance[padStateProperty].isHeldForVolume) {
                return; // Don't change LED if pad is held for volume control
            }
            midi.sendShortMsg(this.midi[0], this.midi[1], value ? this.on : this.off);
        },
        connect: function () {
            components.Button.prototype.connect.call(this);
            var stemGroup = `[Channel${deckInstance.number}_Stem${stemNumber}]`;
            var buttonInstance = this;
            // This connection ensures the pad LED reflects the stem's mute state in Mixxx.
            this.mute_connection = engine.makeConnection(stemGroup, "mute", function (value) {
                var isMuted = (value === 1);
                var ledValue = isMuted ? buttonInstance.off : buttonInstance.on;
                midi.sendShortMsg(buttonInstance.midi[0], buttonInstance.midi[1], ledValue);
            });
            this.mute_connection.trigger();
        },
        disconnect: function () {
            components.Button.prototype.disconnect.call(this);
            if (this.mute_connection) {
                this.mute_connection.disconnect();
                this.mute_connection = null;
            }
        },
        input: function (channel, control, value, status) {
            var padState = deckInstance[padStateProperty];

            // If shift is held, a pad tap will toggle the QuickEffect for that stem.
            // The hold-for-volume logic is bypassed.
            if (NS4FX.shift) {
                if (value === 0x7F) { // Button pressed
                    var quickEffectGroup = `[QuickEffectRack1_[Channel${deckInstance.number}_Stem${stemNumber}]]`;
                    NS4FX.dbg("Toggling " + quickEffectGroup);
                    var currentEffectState = engine.getValue(quickEffectGroup, "enabled");
                    engine.setValue(quickEffectGroup, "enabled", !currentEffectState);
                }
                return;
            }

            // This section implements the tap-to-mute and hold-for-volume logic.
            if (value === 0x7F) { // Button pressed
                padState.isHeldForVolume = false; // Reset on each press
                if (padState.timerId) {
                    engine.stopTimer(padState.timerId);
                }
                // Start a timer to detect if the pad is being held.
                var localTimerId = engine.beginTimer(250, function () { // 250ms hold threshold
                    if (padState.timerId === localTimerId) {
                        padState.isHeldForVolume = true;
                        NS4FX.dbg("Stem pad " + stemNumber + " on deck " + deckInstance.number + " HELD.");
                        padState.timerId = null;
                    }
                }, true); // one-shot timer
                padState.timerId = localTimerId;
            } else { // Button released
                // If the pad is released before the hold timer fires, it's a "tap".
                if (padState.timerId) {
                    engine.stopTimer(padState.timerId);
                    padState.timerId = null;
                }
                if (!padState.isHeldForVolume) {
                    NS4FX.dbg("Stem pad " + stemNumber + " on deck " + deckInstance.number + " TAPPED.");
                    var stemGroup = `[Channel${deckInstance.number}_Stem${stemNumber}]`;
                    var currentMuteState = engine.getValue(stemGroup, "mute");
                    engine.setValue(stemGroup, "mute", currentMuteState === 0 ? 1 : 0);
                }
                padState.isHeldForVolume = false;
            }
        }
    };
}

var NS4FX = {};

NS4FX.dbg = function (str) {
    if (NS4FX.debug) {
        print(str);
    }
};

NS4FX.init = function (id, debug) {

    NS4FX.id = id;
    NS4FX.debug = debug;

    NS4FX.dbg("useFadercutsAsStems is " + useFadercutsAsStems);

    // This component handles the BEATS knob.
    // When a stem pad is held, this knob adjusts the stem's volume (or effect amount if SHIFT is also held).
    // It iterates through all decks and stem pads to find which one is currently in a "held" state.
    NS4FX.beatsKnob = new components.Encoder({
        input: function (channel, control, value, status, group) {
            var heldStemInfo = null;
            for (var i = 1; i <= 4; i++) {
                var deck = NS4FX.decks[i];
                for (var j = 1; j <= 4; j++) {
                    var padState = deck['stemPad' + j];
                    if (padState && padState.isHeldForVolume) {
                        heldStemInfo = {
                            deckNumber: i,
                            stemNumber: j
                        };
                        break;
                    }
                }
                if (heldStemInfo) break;
            }

            if (heldStemInfo) {
                var group, control, currentValue, step;

                if (NS4FX.shift) {
                    // Shift is held: control the QuickEffect's super1 parameter for the stem.
                    group = `[QuickEffectRack1_[Channel${heldStemInfo.deckNumber}_Stem${heldStemInfo.stemNumber}]]`;
                    control = 'super1';
                    currentValue = engine.getValue(group, control);
                    step = 0.05;
                } else {
                    // No shift, control the stem's volume
                    group = `[Channel${heldStemInfo.deckNumber}_Stem${heldStemInfo.stemNumber}]`;
                    control = 'volume';
                    currentValue = engine.getValue(group, control);
                    step = 0.05;
                }
                var newValue;

                if (value === 0x01) { // Turned right
                    newValue = Math.min(1.0, currentValue + step);
                } else { // Turned left (0x7F)
                    newValue = Math.max(0.0, currentValue - step);
                }
                engine.setValue(group, control, newValue);
            }
        }
    });
    // This component handles the crossfader.
    // It's necessary because the NS4FX sends hardware-level fader cut MIDI messages
    // when the Fader Cuts pad mode is active. We need to intercept and ignore these
    // messages when we are using that mode for stem control.
    NS4FX.crossfader = new components.Pot({
        group: '[Master]',
        inKey: 'crossfader',
        input: function (channel, control, value, status, group) {
            if (useFadercutsAsStems) {
                if (NS4FX.decks[1].padmode_str === 'stems' || NS4FX.decks[2].padmode_str === 'stems' || NS4FX.decks[3].padmode_str === 'stems' || NS4FX.decks[4].padmode_str === 'stems') {
                    return; // In stems mode, so do nothing.
                }
            }
            // If not in stems mode, process the crossfader movement as normal.
            components.Pot.prototype.input.call(this, channel, control, value, status, group);
        }
    });

    // Deck switching sends the command 2 times, from current and next. This flag helps ignore the duplicate.
    NS4FX.ignore_deck_switch = true;
    // effects
    NS4FX.effectUnit = new NS4FX.EffectUnit();
    var effects = [
        { unit: 1, slot: 1, id: 9, meta: 0.9 },
        { unit: 1, slot: 2, id: 9, meta: 0.1 },
        { unit: 1, slot: 3, id: 10, meta: 1 },
        { unit: 2, slot: 1, id: 8, meta: 1 },
        { unit: 2, slot: 2, id: 12, meta: 1 },
        { unit: 2, slot: 3, id: 18, meta: 1 }
    ];

    effects.forEach(function (effect) {
        var group = '[EffectRack1_EffectUnit' + effect.unit + '_Effect' + effect.slot + ']';
        engine.setParameter(group, 'meta', effect.meta);
        engine.setValue(group, "clear", 1);
        for (var i = 0; i < effect.id; ++i) {
            engine.setValue(group, "effect_selector", 1);
        }
    });

    // decks
    NS4FX.decks = new components.ComponentContainer();
    NS4FX.decks[1] = new NS4FX.Deck(1, 0x00);
    NS4FX.decks[2] = new NS4FX.Deck(2, 0x01);
    NS4FX.decks[3] = new NS4FX.Deck(3, 0x02);
    NS4FX.decks[4] = new NS4FX.Deck(4, 0x03);

    // set up two banks of samplers, 4 samplers each
    if (engine.getValue("[App]", "num_samplers") < 8) {
        engine.setValue("[App]", "num_samplers", 8);
    }
    NS4FX.sampler_all = new components.ComponentContainer();
    NS4FX.sampler_all[1] = new NS4FX.Sampler(1);
    NS4FX.sampler_all[2] = new NS4FX.Sampler(5);

    NS4FX.sampler = NS4FX.sampler_all[1];
    NS4FX.sampler_all[2].forEachComponent(function (component) {
        component.disconnect();
    });


    // headphone gain
    NS4FX.head_gain = new NS4FX.HeadGain(NS4FX.sampler_all);

    // exit demo mode
    var byteArray = [0xF0, 0x00, 0x01, 0x3F, 0x7F, 0x3A, 0x60, 0x00, 0x04, 0x04, 0x01, 0x00, 0x00, 0xF7];
    midi.sendSysexMsg(byteArray, byteArray.length);

    // initialize some leds
    // Initialize effect LEDs
    NS4FX.effectUnit.effects.forEach(function (effect) {
        var button = NS4FX.effectUnit.effectButtons[effect.name];
        if (button && button.trigger) {
            button.trigger();
        }
    });

    NS4FX.decks.forEachComponent(function (component) {
        component.trigger();
    });

    NS4FX.browse = new NS4FX.BrowseKnob();

    // helper functions
    var led = function (group, key, midi_channel, midino) {
        if (engine.getValue(group, key)) {
            midi.sendShortMsg(0x90 | midi_channel, midino, 0x7F);
        }
        else {
            midi.sendShortMsg(0x80 | midi_channel, midino, 0x00);
        }
    };

    // init a bunch of channel specific leds
    for (var i = 0; i < 4; ++i) {
        var group = "[Channel" + (i + 1) + "]";

        // keylock indicator
        led(group, 'keylock', i, 0x0D);

        // turn off bpm arrows
        midi.sendShortMsg(0x80 | i, 0x0A, 0x00); // down arrow off
        midi.sendShortMsg(0x80 | i, 0x09, 0x00); // up arrow off

        // slip indicator
        led(group, 'slip_enabled', i, 0x0F);

        // start / fwd / back keys
        midi.sendShortMsg(0x84 + i, 0x19, 0x7F); // start
        midi.sendShortMsg(0x84 + i, 0x1A, 0x7F); // back
        midi.sendShortMsg(0x84 + i, 0x1B, 0x7F); // fwd

        // initialize wheel mode (and leds)
        NS4FX.wheel[i] = EnableWheel;
        midi.sendShortMsg(0x90 | i, 0x07, EnableWheel ? 0x7F : 0x01);
    }

    // zero vu meters
    midi.sendShortMsg(0xB0, 0x1F, 0);
    midi.sendShortMsg(0xB1, 0x1F, 0);
    midi.sendShortMsg(0xB2, 0x1F, 0);
    midi.sendShortMsg(0xB3, 0x1F, 0);

    // setup elapsed/remaining tracking
    engine.makeConnection("[Controls]", "ShowDurationRemaining", NS4FX.timeElapsedCallback);

    // setup vumeter tracking
    engine.makeUnbufferedConnection("[Channel1]", "vu_meter", NS4FX.vuCallback);
    engine.makeUnbufferedConnection("[Channel2]", "vu_meter", NS4FX.vuCallback);
    engine.makeUnbufferedConnection("[Channel3]", "vu_meter", NS4FX.vuCallback);
    engine.makeUnbufferedConnection("[Channel4]", "vu_meter", NS4FX.vuCallback);
    engine.makeUnbufferedConnection("[Main]", "vu_meter_left", NS4FX.vuCallback);
    engine.makeUnbufferedConnection("[Main]", "vu_meter_right", NS4FX.vuCallback);
};

NS4FX.shutdown = function () {
    // note: not all of this appears to be strictly necessary, things work fine
    // with out this, but Serato has been observed sending these led reset
    // messages during shutdown. The last sysex message may be necessary to
    // re-enable demo mode.

    // turn off a bunch of channel specific leds
    for (var i = 0; i < 4; ++i) {
        // pfl/cue button leds
        midi.sendShortMsg(0x90 | i, 0x1B, 0x01);

        // loop leds
        midi.sendShortMsg(0x80 | i + 5, 0x32, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x33, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x34, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x35, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x38, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x39, 0x00);

        // play leds
        midi.sendShortMsg(0x90 | i, 0x00, 0x01);
        midi.sendShortMsg(0x90 | i, 0x04, 0x01);

        // sync leds
        midi.sendShortMsg(0x90 | i, 0x00, 0x02);
        midi.sendShortMsg(0x90 | i, 0x04, 0x03);

        // cue leds
        midi.sendShortMsg(0x90 | i, 0x00, 0x01);
        midi.sendShortMsg(0x90 | i, 0x04, 0x05);

        // hotcue leds
        midi.sendShortMsg(0x80 | i + 5, 0x18, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x19, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x1A, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x1B, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x20, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x21, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x22, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x23, 0x00);

        // auto-loop leds
        midi.sendShortMsg(0x80 | i + 5, 0x14, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x15, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x16, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x17, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x1C, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x1D, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x1E, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x1F, 0x00);

        // update spinner and position indicator
        midi.sendShortMsg(0xB0 | i, 0x3F, 0);
        midi.sendShortMsg(0xB0 | i, 0x06, 0);

        // keylock indicator
        midi.sendShortMsg(0x80 | i, 0x0D, 0x00);

        // turn off bpm arrows
        midi.sendShortMsg(0x80 | i, 0x0A, 0x00); // down arrow off
        midi.sendShortMsg(0x80 | i, 0x09, 0x00); // up arrow off

        // turn off slip indicator
        midi.sendShortMsg(0x80 | i, 0x0F, 0x00);

        // turn off wheel button leds
        midi.sendShortMsg(0x80 | i, 0x07, 0x00);
    }

    // dim FX leds
    midi.sendShortMsg(0x98, 0x00, 0x01);
    midi.sendShortMsg(0x98, 0x01, 0x01);
    midi.sendShortMsg(0x98, 0x02, 0x01);
    midi.sendShortMsg(0x99, 0x00, 0x01);
    midi.sendShortMsg(0x99, 0x01, 0x01);
    midi.sendShortMsg(0x99, 0x02, 0x01);

    // turn off sampler leds
    midi.sendShortMsg(0x8F, 0x21, 0x00);
    midi.sendShortMsg(0x8F, 0x22, 0x00);
    midi.sendShortMsg(0x8F, 0x23, 0x00);
    midi.sendShortMsg(0x8F, 0x24, 0x00);

    // zero vu meters
    midi.sendShortMsg(0xBF, 0x44, 0);
    midi.sendShortMsg(0xBF, 0x45, 0);

    // send final shutdown message
    var byteArray = [0xF0, 0x00, 0x20, 0x7F, 0x02, 0xF7];
    midi.sendSysexMsg(byteArray, byteArray.length);
};

NS4FX.EffectUnit = function () {
    var self = this;
    this.deck1 = true;
    this.deck2 = true;
    this.switch_active_left = false;
    this.switch_active_right = false;

    this.toggleEffect = function (effectName) {
        var effect = this.effects.find(e => e.name === effectName);
        if (!effect) return;

        var group = '[EffectRack1_EffectUnit' + effect.unit + '_Effect' +
            (this.effects.filter(e => e.unit === effect.unit).indexOf(effect) + 1) + ']';

        // Get the current state of the effect and toggle it
        var isEnabled = engine.getValue(group, 'enabled');
        engine.setValue(group, 'enabled', !isEnabled);

        this.updateEffectButtons();
    };

    this.updateEffectButtons = function () {
        var self = this;
        this.effects.forEach(function (effect) {
            var group = '[EffectRack1_EffectUnit' + effect.unit + '_Effect' +
                (self.effects.filter(e => e.unit === effect.unit).indexOf(effect) + 1) + ']';
            var isEnabled = engine.getValue(group, 'enabled');
            self.effectButtons[effect.name].output(isEnabled ? 0x7F : 0x00);
        });
    };

    this.dryWetKnob = new components.Pot({
        midi: [0xB8, 0x04],
        group: '[EffectRack1_EffectUnit1]',
        inKey: 'mix',
        input: function (channel, control, value, status, group) {
            var newValue = value / 127;
            engine.setValue('[EffectRack1_EffectUnit1]', 'mix', newValue);
            engine.setValue('[EffectRack1_EffectUnit2]', 'mix', newValue);
        }
    });

    this.effects = [
        { name: 'hpf', status: 0x98, control: 0x00, unit: 1 },
        { name: 'lpf', status: 0x98, control: 0x01, unit: 1 },
        { name: 'flanger', status: 0x98, control: 0x02, unit: 1 },
        { name: 'echo', status: 0x99, control: 0x03, unit: 2 },
        { name: 'reverb', status: 0x99, control: 0x04, unit: 2 },
        { name: 'phaser', status: 0x99, control: 0x05, unit: 2 }
    ];
    this.activeEffect = null;

    this.effectButtons = {};
    this.effects.forEach(function (effect) {
        self.effectButtons[effect.name] = new components.Button({
            midi: [effect.status, effect.control],
            group: '[EffectRack1_EffectUnit' + effect.unit + ']',
            inKey: 'enabled',
            input: function (channel, control, value, status, group) {
                if (value === 0x7F) {
                    self.toggleEffect(effect.name);
                }
            }
        });
    });

    this.setEffectUnitsForChannel = function (channel, active) {
        var unit1 = "[EffectRack1_EffectUnit1]";
        var unit2 = "[EffectRack1_EffectUnit2]";
        var groupKey = "group_[Channel" + channel + "]_enable";

        engine.setValue(unit1, groupKey, active);
        engine.setValue(unit2, groupKey, active);

        if (channel === 1 || channel === 3) {
            this.switch_active_left = active;
        } else {
            this.switch_active_right = active;
        }
    };

    // Linker Switch (Deck 1)
    this.leftSwitch = function (channel, control, value, status, group) {
        var active = value !== 0x00;
        var inactiveDeck = this.deck1 ? 3 : 1;
        var activeDeck = this.deck1 ? 1 : 3;
        var isActive = deck => (deck === activeDeck) || !OnlyActiveDeckEffect;

        this.setEffectUnitsForChannel(inactiveDeck, isActive(inactiveDeck) && active);
        this.setEffectUnitsForChannel(activeDeck, isActive(activeDeck) && active);
    };

    this.rightSwitch = function (channel, control, value, status, group) {
        var active = value !== 0x00;
        var inactiveDeck = this.deck2 ? 4 : 2;
        var activeDeck = this.deck2 ? 2 : 4;
        var isActive = deck => (deck === activeDeck) || !OnlyActiveDeckEffect;

        this.setEffectUnitsForChannel(inactiveDeck, isActive(inactiveDeck) && active);
        this.setEffectUnitsForChannel(activeDeck, isActive(activeDeck) && active);
    };

    this.updateEffectOnDeckSwitch = function (leftSide) {
        if (!OnlyActiveDeckEffect) {
            return;
        }

        if (leftSide) {
            var active = this.switch_active_left ? 0x01 : 0x00;
            this.leftSwitch(0, 0, active, 0, 0)
        } else {
            // var func = this.rightSwitch;
            var active = this.switch_active_right ? 0x01 : 0x00;
            this.rightSwitch(0, 0, active, 0, 0);
        }

    }
}

NS4FX.EffectUnit.prototype = new components.ComponentContainer();

NS4FX.Deck = function (number, midi_chan) {
    var deck = this;
    this.number = number;
    this.active = (number == 1 || number == 2);

    // If using stems, create state objects for each pad to track hold timers and states.
    // This is necessary for the hold-for-volume/effect functionality.
    if (useFadercutsAsStems) {
        this.stemPad1 = { timerId: null, isHeldForVolume: false };
        this.stemPad2 = { timerId: null, isHeldForVolume: false };
        this.stemPad3 = { timerId: null, isHeldForVolume: false };
        this.stemPad4 = { timerId: null, isHeldForVolume: false };
    }

    hotcuePressed = false;
    playPressedDuringHotcue = false;

    components.Deck.call(this, number);


    this.bpm = new components.Component({
        outKey: "bpm",
        output: function (value, group, control) {
            NS4FX.sendScreenBpmMidi(number, Math.round(value * 100));
        },
    });

    this.duration = new components.Component({
        outKey: "duration",
        output: function (duration, group, control) {
            // update duration
            NS4FX.sendScreenDurationMidi(number, duration * 1000);

            // when the duration changes, we need to update the play position
            deck.position.trigger();
        },
    });

    this.position = new components.Component({
        outKey: "playposition",
        output: function (playposition, group, control) {
            // the controller appears to expect a value in the range of 0-52
            // representing the position of the track. Here we send a message to the
            // controller to update the position display with our current position.
            var pos = Math.round(playposition * 52);
            if (pos < 0) {
                pos = 0;
            }
            midi.sendShortMsg(0xB0 | midi_chan, 0x3F, pos);

            // get the current duration
            duration = deck.duration.outGetValue();

            // update the time display
            var time = NS4FX.timeMs(number, playposition, duration);
            NS4FX.sendScreenTimeMidi(number, time);

            // update the spinner (range 64-115, 52 values)
            // // the visual spinner in the mixxx interface takes 1.8 seconds to loop
            // (60 seconds/min divided by 33 1/3 revolutions per min)
            var period = 60 / (33 + 1 / 3);
            var midiResolution = 52; // the controller expects a value range of 64-115
            var timeElapsed = duration * playposition;
            var spinner = Math.round(timeElapsed % period * (midiResolution / period));
            if (spinner < 0) {
                spinner += 115;
            } else {
                spinner += 64;
            }

            midi.sendShortMsg(0xB0 | midi_chan, 0x06, spinner);
        },
    });

    this.orientation = new components.Button({
        midi: [0xB0 - 1 + midi_chan, 0x1E],
        input: function (channel, control, value, status) {
            var desiredOrientation = [1, 0, 2][value]
            engine.setValue(this.group, "orientation", desiredOrientation);
        },
    });

    this.play_button = new components.PlayButton({
        midi: [0x90 + midi_chan, 0x00],
        off: 0x01,
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 4,
        unshift: function () {
            components.PlayButton.prototype.unshift.call(this);
            this.type = components.Button.prototype.types.toggle;
        },
        shift: function () {
            this.inKey = 'play_stutter';
            this.type = components.Button.prototype.types.push;
        },
        input: function (channel, control, value, status, group) {
            if (this.isShifted) {
                // Shift-Modus Logik
                if (value === 0x7F) {
                    engine.setValue(group, "play_stutter", 1);
                } else {
                    engine.setValue(group, "play_stutter", 0);
                }
            } else {
                // Normaler Modus Logik
                if (value === 0x7F) {
                    if (hotcuePressed) {
                        playPressedDuringHotcue = true;
                    } else {
                        var currentPlayState = engine.getValue(group, "play");
                        engine.setValue(group, "play", !currentPlayState);
                    }
                }
            }
        }
    });

    this.load = new components.Button({
        inKey: 'LoadSelectedTrack',
        shift: function () {
            if (ShiftLoadEjects) {
                this.inKey = 'eject';
            }
            else {
                this.inKey = 'LoadSelectedTrackAndPlay';
            }
        },
        unshift: function () {
            this.inKey = 'LoadSelectedTrack';
        },
    });

    this.cue_button = new components.CueButton({
        midi: [0x90 + midi_chan, 0x01],
        off: 0x01,
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 4,
    });

    this.sync_button = new components.SyncButton({
        midi: [0x90 + midi_chan, 0x02],
        off: 0x01,
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 1,
    });

    this.pfl_button = new components.Button({
        midi: [0x90 + midi_chan, 0x1B],
        key: 'pfl',
        off: 0x01,
        type: components.Button.prototype.types.toggle,
        connect: function () {
            components.Button.prototype.connect.call(this);
            this.connections[1] = engine.makeConnection(this.group, this.outKey, NS4FX.pflToggle.bind(this));
        },
    });

    // HOTCUES
    this.hotcue_buttons = new components.ComponentContainer();
    this.hotcue_buttons_secondary = new components.ComponentContainer();
    this.roll_buttons = new components.ComponentContainer();
    this.slicer_buttons = new components.ComponentContainer();
    this.sampler_buttons = new components.ComponentContainer({
        updateLEDs: function () {
            for (let button in deck.sampler_buttons) {
                if (this[button] instanceof components.SamplerButton) {
                    const samplerGroup = `[Sampler${this[button].number}]`; // Gruppe des entsprechenden Samplers
                    const isTrackLoaded = engine.getValue(samplerGroup, 'track_loaded'); // Prüfe, ob ein Track geladen ist
                    // LED leuchtet, wenn ein Track geladen ist
                    deck.sampler_buttons[button].output(isTrackLoaded ? 1 : 0);
                }
            }
        }
    });
    this.fadercuts_buttons = new components.ComponentContainer({
        updateLEDs: function (deckGroup) {
            for (let button in deck.fadercuts_buttons) {
                if (deck.fadercuts_buttons[button] instanceof components.Button) {
                    deck.fadercuts_buttons[button].output(0);
                }
            }
        }
    });
    this.autoloop_buttons = new components.ComponentContainer({
        updateLEDs: function (deckGroup) {
            for (let button in deck.autoloop_buttons) { // Iteriere direkt über autoloop_buttons
                if (deck.autoloop_buttons[button] instanceof components.Button) {
                    const loopLength = Math.pow(2, 5 - deck.autoloop_buttons[button].number); // Berechne Loop-Länge für den Button
                    const currentLoopLength = engine.getValue(deckGroup, 'beatloop_size');
                    const isActive = engine.getValue(deckGroup, 'loop_enabled') && currentLoopLength === loopLength; // Prüfe ob der Loop aktiv ist und die Länge übereinstimmt

                    deck.autoloop_buttons[button].output(isActive ? 1 : 0); // LED an/aus basierend auf Zustand
                }
            }
        }
    });

    for (var i = 1; i <= 4; ++i) {

        this.hotcue_buttons[i] = new components.HotcueButton({
            midi: [0x94 + midi_chan, 0x13 + i],
            number: i,
            output: function (value) {
                midi.sendShortMsg(this.midi[0], this.midi[1], value ? 0x7F : 0x01);
            }
        });

        // cue buttons 5 - 8
        this.hotcue_buttons_secondary[5 - i] = new components.HotcueButton({
            midi: [0x94 + midi_chan, 0x18 - i],
            number: 9 - i,
            output: function (value) {
                midi.sendShortMsg(this.midi[0], this.midi[1], value ? 0x7F : 0x01);
            }
        });

        // sampler buttons
        if (deck.number % 2 == 0) {
            sampler_offset = 4;
        } else {
            sampler_offset = 0;
        }
        this.sampler_buttons[5 - i] = new components.SamplerButton({
            midi: [0x94 + midi_chan, 0x18 - i],
            number: 5 - i + sampler_offset,
            loaded: 0x5,
            playing: 0x7F
        });

        this.autoloop_buttons[5 - i] = new components.Button({
            midi: [0x94 + midi_chan, 0x18 - i], // Example MIDI addresses
            input: function (channel, control, value, status) {
                // Guard to ensure this logic only runs when autoloop mode is active.
                if (deck.padmode_str !== 'autoloop') {
                    return;
                }

                // When fadercuts are used as stems, this pad mode is not active, so we should do nothing.
                if (useFadercutsAsStems) {
                    return;
                }
                var deckGroup = `[Channel${deck.number}]`; // Deck group based on deck number

                if (value === 0x7F) { // Button pressed
                    // Control loop length based on button number
                    var loopLength = Math.pow(2, 5 - this.number); // Button 1 = 1 beat, Button 2 = 2 beats, etc.

                    // Check the current state of the loop
                    var currentLoopLength = engine.getValue(deckGroup, 'beatloop_size');
                    var isLoopActive = engine.getValue(this.group, "loop_enabled");// Check if a loop is active

                    if (!isLoopActive || currentLoopLength !== loopLength) {
                        // If no loop is active or the length changes, set the new length and activate the loop
                        engine.setValue(deckGroup, 'beatloop_size', loopLength);
                        if (!isLoopActive) {
                            print("not active");
                            deck.loopControls.loop_toggle.input(0, 0, 0x7F, 0);
                        } else {
                            engine.setValue(deckGroup, 'loop_enabled', 1); // Activate the loop
                        }
                    } else {
                        engine.setValue(deckGroup, 'loop_enabled', 0);
                    }

                    // Update LEDs
                    deck.autoloop_buttons.updateLEDs(deckGroup);
                }
            },
            output: function (value) {
                midi.sendShortMsg(this.midi[0], this.midi[1], value ? 0x7F : 0x01); // LED an/aus
            },
            number: i // Speichert die Nummer des Buttons (1–4)
        });

        if (!useFadercutsAsStems) {
            this.fadercuts_buttons[5 - i] = new components.Button({
                midi: [0x94 + midi_chan, 0x18 - i], // Example MIDI addresses
                input: function (channel, control, value, status) {
                    if (deck.padmode_str !== 'fadercuts') {
                        return;
                    }

                    const deckGroup = `[Channel${deck.number}]`; // Deck-Gruppe basierend auf Deck-Nummer

                    if (value === 0x7F) { // Button gedrückt
                        const bpm = engine.getValue(deckGroup, 'bpm'); // Hole die BPM des Tracks
                        const baseInterval = (60 / bpm) * 1000 / 4; // Berechne die Dauer eines Beats in Millisekunden

                        // Geschwindigkeit basierend auf Button-Nummer (z.B. schneller bei höheren Nummern)
                        const speedMultiplier = this.number; // Button 1 = langsam, Button 4 = schnell
                        const interval = baseInterval / speedMultiplier; // Geschwindigkeit anpassen

                        print(`BPM=${bpm}, Base Interval=${baseInterval}ms, Speed Multiplier=${speedMultiplier}, Final Interval=${interval}ms`);

                        this.startFaderCuts(deckGroup, interval); // Cuts mit berechneter Geschwindigkeit starten

                        this.output(1); // LED aktivieren
                    } else {
                        this.stopFaderCuts(deckGroup); // Stoppe Fadercuts

                        this.output(0); // LED deaktivieren
                    }
                },
                output: function (value) {
                    midi.sendShortMsg(this.midi[0], this.midi[1], value ? 0x7F : 0x01); // LED an/aus
                },
                startFaderCuts: function (deckGroup, interval) {
                    let toggle = false;

                    this.faderCutInterval = engine.beginTimer(interval, () => {
                        toggle = !toggle;

                        const newVolume = toggle ? 1 : 0; // Wechsel zwischen voller Volume und Stille

                        print(`Toggle=${toggle}, New Volume=${newVolume}`);

                        engine.setValue(deckGroup, 'volume', newVolume);
                    });

                    print(`Timer started with interval ${interval}ms`);
                },
                stopFaderCuts: function (deckGroup) {
                    if (this.faderCutInterval) {
                        engine.stopTimer(this.faderCutInterval);
                        this.faderCutInterval = null;
                    }

                    engine.setValue(deckGroup, 'volume', 1);
                    print(`Resetting volume for ${deckGroup} to 1`);
                },
                number: i
            });
        }

        const rollDuration = Math.pow(2, -(i)); // Berechnet die Loop Roll Dauer (0.5, 0.25, 0.125, 0.0625)
        const rollDurationString = rollDuration.toFixed(4).replace(/0+$/, '');
        this.roll_buttons[5 - i] = new components.Button({
            midi: [0x94 + midi_chan, 0x18 - i], // MIDI-Adresse für die Buttons
            number: 5 - i, // Button-Nummer (1 bis 4)
            inKey: `beatlooproll_${rollDurationString}_activate`, // Automatisch berechneter inKey
            outKey: `beatloop_${rollDurationString}_enabled`,
        });

    }

    this.change_padmode = function (padmode) {
        this.padmode_str = padmode;
        // This is the main pad mode switching logic.
        // It disconnects the old set of pads and connects the new one.
        if (padmode == "hotcue") {
            buttons = this.hotcue_buttons;
        } else if (padmode == "sampler") {
            deck.sampler_buttons.updateLEDs(`[Channel${this.number}]`);
            buttons = this.sampler_buttons;
        } else if (padmode == "autoloop") {
            deck.autoloop_buttons.updateLEDs(`[Channel${this.number}]`);
            buttons = this.autoloop_buttons;
        } else if (padmode == "fadercuts") {
            deck.fadercuts_buttons.updateLEDs(`[Channel${this.number}]`);
            buttons = this.fadercuts_buttons;
        } else if (padmode == "stems") {
            // When switching to stems mode, set the active pads to the stems_buttons container.
            // LED updates are handled by the connections within each stem button.
            buttons = this.stems_buttons;
        } else if (padmode == "pitchplay") {
            print("not implemented yet");
        } else if (padmode == "roll") {
            buttons = this.roll_buttons;
        } else if (padmode == "slicer") {
            buttons = this.slicer_buttons;
        } else if (padmode == "scratchbanks") {
            print("not implemented yet");
        }
        this.hotcues.forEachComponent(function (component) {
            component.disconnect();
        })
        this.hotcues = buttons;
        this.hotcues.reconnectComponents();
    }
    this.hotcues = this.hotcue_buttons;
    this.padmode_str = "hotcue";
    this.change_padmode("hotcue");
    this.pitch = new components.Pot({
        inKey: 'rate',
        invert: true,
    });
    if (!this.active) {
        this.pitch.firstValueReceived = true;
    }

    var pitch_or_keylock = function (channel, control, value, status, group) {
        if (value === 0) {
            // Button released -> inherit to standard button
            components.Button.prototype.input.call(this, channel, control, value, status, group);
            return;
        }

        if (this.other.inGetValue() > 0.0 && this.isPress(channel, control, value, status)) {
            // Both pitch buttons pressed -> toggle keylock
            script.toggleControl(this.group, "keylock");
        } else {
            // Normal pitch bending
            components.Button.prototype.input.call(this, channel, control, value, status, group);
        }
    };

    this.pitch_bend_up = new components.Button({
        inKey: 'rate_temp_up',
        input: pitch_or_keylock,
    });

    this.pitch_bend_down = new components.Button({
        inKey: 'rate_temp_down',
        input: pitch_or_keylock,
    });

    this.pitch_bend_up.other = this.pitch_bend_down;
    this.pitch_bend_down.other = this.pitch_bend_up;

    var key_up_or_down = function (channel, control, value, status, group) {
        this.is_pressed = this.isPress(channel, control, value, status);
        if (this.is_pressed) {
            if (this.other.is_pressed) {
                // reset if both buttons are pressed
                engine.setValue(deck.currentDeck, "pitch_adjust", 0.0);
            }
            else {
                this.inSetValue(1.0);
            }
        }
    };
    this.key_up = new components.Button({
        inKey: 'pitch_up',
        direction: 1,
        input: key_up_or_down,
    });
    this.key_down = new components.Button({
        inKey: 'pitch_down',
        direction: -1,
        input: key_up_or_down,
    });
    this.key_up.other = this.key_down;
    this.key_down.other = this.key_up;

    this.stems_buttons = new components.ComponentContainer();
    if (useFadercutsAsStems) {
        for (var i = 1; i <= 4; ++i) {
            this.stems_buttons[i] = new components.Button(createStemPadConfig(deck, 'stemPad' + i, i, {
                channel: midi_chan,
                note: 0x13 + i
            }));
        }
    }


    // PAD MODE
    this.padMode = new components.ComponentContainer({
        pad_hotcue: new components.Button({
            midi: [0x94 + midi_chan, 0x00], // MIDI address for Hotcue mode
            input: function (channel, control, value, status) {
                if (value === 0x7F) { // Button pressed
                    this.groupContainer.turnOffOtherButtons(this); // Deactivates other LEDs
                    this.output(1); // Activates LED for this mode
                    // Activate logic for Hotcue mode
                    deck.change_padmode("hotcue");
                }
            },
            output: function (value) {
                this.send(value ? 0x7F : 0x01); // LED an/aus
            }
        }),
        pad_autoloop: new components.Button({
            midi: [0x94 + midi_chan, 0x0D], // MIDI address for Autoloop mode
            input: function (channel, control, value, status) {
                if (value === 0x7F) { // Button pressed
                    this.groupContainer.turnOffOtherButtons(this);
                    this.output(1);
                    deck.change_padmode("autoloop");
                }
            },
            output: function (value) {
                this.send(value ? 0x7F : 0x01);
            }
        }),
        pad_fadercuts: new components.Button({
            midi: [0x94 + midi_chan, 0x07], // MIDI address for Fadercuts mode
            input: function (channel, control, value, status) {
                if (value === 0x7F) {
                    this.groupContainer.turnOffOtherButtons(this);
                    this.output(1);
                    // If useFadercutsAsStems is true, this button activates "stems" mode.
                    // Otherwise, it activates the normal "fadercuts" mode.
                    if (useFadercutsAsStems) {
                        ;
                        NS4FX.dbg("Switching to stems mode on deck " + deck.number);
                        deck.change_padmode("stems");
                    } else {
                        deck.change_padmode("fadercuts");
                    }
                }
            },
            output: function (value) {
                this.send(value ? 0x7F : 0x01);
            }
        }),
        pad_sample: new components.Button({
            midi: [0x94 + midi_chan, 0x0B], // MIDI address for Sample mode
            input: function (channel, control, value, status) {
                if (value === 0x7F) {
                    this.groupContainer.turnOffOtherButtons(this);
                    this.output(1);
                    deck.change_padmode("sampler");
                }
            },
            output: function (value) {
                this.send(value ? 0x7F : 0x01);
            }
        }),
        pad_pitchplay: new components.Button({
            midi: [0x94 + midi_chan, 0x02], // MIDI address for Pitch Play mode
            input: function (channel, control, value, status) {
                if (value === 0x7F) {
                    this.groupContainer.turnOffOtherButtons(this);
                    this.output(1);
                    deck.change_padmode("pitchplay");
                }
            },
            output: function (value) {
                this.send(value ? 0x7F : 0x01);
            }
        }),
        pad_roll: new components.Button({
            midi: [0x94 + midi_chan, 0x06], // MIDI address for Roll mode
            input: function (channel, control, value, status) {
                if (value === 0x7F) {
                    this.groupContainer.turnOffOtherButtons(this);
                    this.output(1);
                    deck.change_padmode("roll");
                }
            },
            output: function (value) {
                this.send(value ? 0x7F : 0x01);
            }
        }),
        pad_slicer: new components.Button({
            midi: [0x94 + midi_chan, 0x0E], // MIDI address for Slicer mode
            input: function (channel, control, value, status) {
                if (value === 0x7F) {
                    this.groupContainer.turnOffOtherButtons(this);
                    this.output(1);
                    deck.change_padmode("slicer");
                }
            },
            output: function (value) {
                this.send(value ? 0x7F : 0x01);
            }
        }),
        pad_scratchbanks: new components.Button({
            midi: [0x94 + midi_chan, 0x0F], // MIDI address for Scratch Banks mode
            input: function (channel, control, value, status) {
                if (value === 0x7F) {
                    this.groupContainer.turnOffOtherButtons(this);
                    this.output(1);
                    deck.change_padmode("scratchbanks");
                }
            },
            output: function (value) {
                this.send(value ? 0x7F : 0x01);
            }
        }),
        turnOffOtherButtons: function (activeButton) {
            for (var button in this) { // NOSONAR
                if (this[button] instanceof components.Button && this[button] !== activeButton) {
                    this[button].output(0); // Turns off LEDs of other buttons
                }
            }
        }
    });
    this.padMode.pad_fadercuts.output(0);

    // Ensure the buttons have access to the container.
    for (var button in this.padMode) {
        if (this.padMode[button] instanceof components.Button) {
            this.padMode[button].groupContainer = this.padMode; // Set container reference
        }
        this.padMode.pad_hotcue.output(1);

        // LOOP controls
        this.loopControls = new components.ComponentContainer({
            loop_halve: new components.Button({
                midi: [0x94 + midi_chan, 0x34],
                input: function (channel, control, value, status) {
                    if (value === 0x7F) { // Button pressed
                        engine.setValue(this.group, "loop_halve", 1);
                        this.output(1);
                    } else if (value === 0x00) { // Button released
                        this.output(0);
                        if (deck.padmode_str == "autoloop") {
                            deck.autoloop_buttons.updateLEDs(this.group);
                        }
                    }
                },
                output: function (value) {
                    this.send(value ? 0x7F : 0x01);
                }
            }),

            loop_double: new components.Button({
                midi: [0x94 + midi_chan, 0x35],
                input: function (channel, control, value, status) {
                    if (value === 0x7F) { // Button pressed
                        engine.setValue(this.group, "loop_double", 1);
                        this.output(1);
                    } else if (value === 0x00) { // Button released
                        this.output(0);
                        if (deck.padmode_str == "autoloop") {
                            deck.autoloop_buttons.updateLEDs(this.group);
                        }
                    }
                },
                output: function (value) {
                    this.send(value ? 0x7F : 0x01);
                }
            }),

            loop_in: new components.Button({
                midi: [0x94 + midi_chan, 0x36],
                input: function (channel, control, value, status) {
                    if (value === 0x7F) { // Button pressed
                        engine.setValue(this.group, "loop_in", 1);
                        this.output(1);
                    } else if (value === 0x00) { // Button released
                        engine.setValue(this.group, "loop_in", 0);
                        this.output(0);
                    }
                },
                output: function (value) {
                    this.send(value ? 0x7F : 0x01);
                }
            }),

            loop_out: new components.Button({
                midi: [0x94 + midi_chan, 0x37],
                input: function (channel, control, value, status) {
                    if (value === 0x7F) { // Button pressed
                        engine.setValue(this.group, "loop_out", 1);
                        this.output(1);
                    } else if (value === 0x00) { // Button released
                        engine.setValue(this.group, "loop_out", 0);
                        this.output(0);
                    }
                },
                output: function (value) {
                    this.send(value ? 0x7F : 0x01);
                }
            }),
            reloop: new components.Button({
                midi: [0x94 + midi_chan, 0x41],
                input: function (channel, control, value, status) {
                    if (value === 0x7F) { // Button pressed
                        var loopEnabled = engine.getValue(this.group, "loop_enabled");
                        if (loopEnabled) {
                            // If der Loop aktiv ist, deaktivieren wir ihn
                            engine.setValue(this.group, "loop_enabled", 0);
                        } else {
                            // If kein Loop aktiv ist, aktivieren wir den letzten Loop
                            engine.setValue(this.group, "reloop_toggle", 1);
                        }
                        this.output(1);
                    } else if (value === 0x00) { // Button released
                        this.output(0);
                    }
                },
                output: function (value) {
                    this.send(value ? 0x7F : 0x01);
                },
                connect: function () {
                    this.connections.push(
                        engine.connectControl(this.group, "loop_enabled", function (value) {
                            this.output(value);
                        }.bind(this))
                    );
                }
            }),

            loop_toggle: new components.Button({
                midi: [0x94 + midi_chan, 0x40],
                input: function (channel, control, value, status) {
                    if (value === 0x7F) { // Button pressed
                        var loopEnabled = engine.getValue(this.group, "loop_enabled");
                        var loopStartPosition = engine.getValue(this.group, "loop_start_position");
                        var loopEndPosition = engine.getValue(this.group, "loop_end_position");
                        var currentPosition = engine.getValue(this.group, "playposition");
                        var trackSamples = engine.getValue(this.group, "track_samples");

                        // Konvertiere currentPosition zu Samples
                        var currentSamplePosition = currentPosition * trackSamples;

                        if (loopEnabled) {
                            // If ein Loop aktiv ist, deaktivieren wir ihn
                            engine.setValue(this.group, "loop_enabled", 0);
                        } else if (loopStartPosition >= 0 && loopEndPosition > loopStartPosition &&
                            currentSamplePosition >= loopStartPosition && currentSamplePosition <= loopEndPosition) {
                            // If wir uns innerhalb eines definierten Loops befinden, aktivieren wir ihn
                            engine.setValue(this.group, "loop_enabled", 1);
                        } else {
                            // Ansonsten setzen wir einen neuen Loop
                            engine.setValue(this.group, "beatloop_activate", 1);
                        }
                        if (deck.padmode_str == "autoloop") {
                            var deckGroup = `[Channel${deck.number}]`; // Deck-Gruppe basierend auf Deck-Nummer
                            deck.autoloop_buttons.updateLEDs(deckGroup);
                        }
                    }
                },
                output: function (value) {
                    this.send(value ? 0x7F : 0x01);
                },
                connect: function () {
                    this.connections.push(
                        engine.connectControl(this.group, "loop_enabled", function (value) {
                            this.output(value);
                        }.bind(this)),
                        engine.connectControl(this.group, "track_loaded", function () {
                            var loopEnabled = engine.getValue(this.group, "loop_enabled");
                            this.output(loopEnabled);
                        }.bind(this))
                    );
                }
            })
        });
        // EQ and Filter knobs
        this.EqEffectKnob = function (group, in_key, fx_key, filter_knob) {
            this.unshift_group = group;
            this.unshift_key = in_key;
            this.fx_key = fx_key;
            if (filter_knob) {
                this.shift_key = 'super1';
            }
            this.ignore_next = null;
            components.Pot.call(this, {
                group: group,
                inKey: in_key,
            });
        };
        this.EqEffectKnob.prototype = new components.Pot({
            input: function (channel, control, value, status, group) {
                // if the control group and key has changed, ignore_next will hold
                // the old settings. We need to tell the soft takeover engine to
                // ignore the next values for that control so that when we
                // eventually switch back to it, soft takeover will manage it
                // properly.
                // // We call IgnoreNextValue() here instead of in shift()/unshift()
                // (via connect()/disconnect()) because if we did that, pressing
                // the shift key would cause the next value on the control to be
                // ignored even if the control wasn't moved, which would trigger
                // a phantom soft takeover if the control was moved fast enough. We
                // only need to IgnoreNextValue() if the control has actually moved
                // after switching the target group/key.
                if (this.ignore_next) {
                    engine.softTakeoverIgnoreNextValue(this.ignore_next.group, this.ignore_next.key);
                    this.ignore_next = null;
                }
                components.Pot.prototype.input.call(this, channel, control, value, status, group);
            },
            connect: function () {
                // enable soft takeover on our controls
                for (var i = 1; i <= 3; i++) {
                    var group = '[EffectRack1_EffectUnit1_Effect' + i + ']';
                    engine.softTakeover(group, this.fx_key, true);
                }
                components.Pot.prototype.connect.call(this);
            },
            shift: function () {
                var focused_effect = engine.getValue('[EffectRack1_EffectUnit1]', "focused_effect");
                if (focused_effect === 0) {
                    if (this.shift_key) {
                        engine.softTakeover('[EffectRack1_EffectUnit1]', this.shift_key, true);
                        this.switchControl('[EffectRack1_EffectUnit1]', this.shift_key);
                    }
                } else {
                    var group = '[EffectRack1_EffectUnit1_Effect' + focused_effect + ']';
                    this.switchControl(group, this.fx_key);
                }
            },

            unshift: function () {
                this.switchControl(this.unshift_group, this.unshift_key);
            },
            switchControl: function (group, key) {
                if (this.group != group || this.inKey != key) {
                    this.ignore_next = { 'group': this.group, 'key': this.inKey };
                }
                this.group = group;
                this.inKey = key;
            },
        });

        var eq_group = '[EqualizerRack1_' + this.currentDeck + '_Effect1]';
        this.high_eq = new this.EqEffectKnob(eq_group, 'parameter3', 'parameter3');
        this.mid_eq = new this.EqEffectKnob(eq_group, 'parameter2', 'parameter4');
        this.low_eq = new this.EqEffectKnob(eq_group, 'parameter1', 'parameter5');

        this.filter = new this.EqEffectKnob(
            '[QuickEffectRack1_' + this.currentDeck + ']',
            'super1',
            'parameter1',
            true);

        this.gain = new this.EqEffectKnob(
            this.currentDeck,
            'pregain',
            'parameter2');


        this.reconnectComponents(function (c) {
            if (c.group === undefined) {
                c.group = deck.currentDeck;
            }
        });

        this.setActive = function (active) {
            this.active = active;

            if (!active) {
                // trigger soft takeover on the pitch control
                this.pitch.disconnect();
            }
        };
    };
}

NS4FX.Deck.prototype = new components.Deck();

NS4FX.Sampler = function (base) {
    for (var i = 1; i <= 4; ++i) {
        this[i] = new components.SamplerButton({
            midi: [0x9F, 0x20 + i],
            number: base + i - 1,
            loaded: 0x00,
            playing: 0x7F,
        });
    }
};

NS4FX.Sampler.prototype = new components.ComponentContainer();

NS4FX.HeadGain = function (sampler) {
    components.Pot.call(this);

    this.ignore_next = null;
    this.shifted = false;
    this.sampler = sampler;
    this.sampler.forEachComponent(function (component) {
        engine.softTakeover(component.group, 'volume', true);
    });
};
NS4FX.HeadGain.prototype = new components.Pot({
    group: '[Master]',
    inKey: 'headGain',
    input: function (channel, control, value, status, group) {
        // we call softTakeoverIgnoreNextValue() here on the non-targeted
        // control only if the control was moved when focus was switched. This
        // is to avoid a phantom triggering of soft takeover that can happen if
        // ignoreNextValue() is called un-conditionally when the control target
        // is changed (like in shift()/unshift()).
        if (this.ignore_next == "sampler" && !this.shifted) {
            this.sampler.forEachComponent(function (component) {
                engine.softTakeoverIgnoreNextValue(component.group, 'volume');
            });
            this.ignore_next = null;
        }
        else if (this.ignore_next == "headgain" && this.shifted) {
            engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
            this.ignore_next = null;
        }

        if (this.shifted) {
            // make head gain control the sampler volume when shifted
            var pot = this;
            this.sampler.forEachComponent(function (component) {
                engine.setParameter(component.group, 'volume', pot.inValueScale(value));
            });
        } else {
            components.Pot.prototype.input.call(this, channel, control, value, status, group);
        }
    },
    shift: function () {
        this.shifted = true;
        this.ignore_next = "headgain";
    },
    unshift: function () {
        this.shifted = false;
        this.ignore_next = "sampler";
    },
});

NS4FX.BrowseKnob = function () {
    this.knob = new components.Encoder({
        group: '[Library]',
        input: function (channel, control, value, status, group) {
            if (value === 1) {
                engine.setParameter(this.group, this.inKey + 'Down', 1);
            } else if (value === 127) {
                engine.setParameter(this.group, this.inKey + 'Up', 1);
            }
        },
        unshift: function () {
            this.inKey = 'Move';
        },
        shift: function () {
            this.inKey = 'Scroll';
        },
    });

    this.button = new components.Button({
        group: '[Library]',
        inKey: 'GoToItem', // Default action is to go to the selected item.
        input: function (channel, control, value, status, group) {
            if (value > 0) { // Button gedrückt
                engine.setParameter(this.group, this.inKey, 1);
            }
        },
        unshift: function () {
            this.inKey = 'GoToItem';
        },
        shift: function () { // When shifted, move focus instead of selecting.
            this.inKey = 'MoveFocusBackward';
        },
    });
};

NS4FX.BrowseKnob.prototype = new components.ComponentContainer();

NS4FX.encodeNumToArray = function (number) {
    var number_array = [
        (number >> 28) & 0x0F,
        (number >> 24) & 0x0F,
        (number >> 20) & 0x0F,
        (number >> 16) & 0x0F,
        (number >> 12) & 0x0F,
        (number >> 8) & 0x0F,
        (number >> 4) & 0x0F,
        number & 0x0F,
    ];

    if (number < 0) number_array[0] = 0x07;
    else number_array[0] = 0x08;

    return number_array;
};

NS4FX.sendScreenDurationMidi = function (deck, duration) {
    if (duration < 1) {
        duration = 1;
    }
    durationArray = NS4FX.encodeNumToArray(duration - 1);

    var bytePrefix = [0xF0, 0x00, 0x20, 0x7F, deck, 0x03];
    var bytePostfix = [0xF7];
    var byteArray = bytePrefix.concat(durationArray, bytePostfix);
    midi.sendSysexMsg(byteArray, byteArray.length);
};

NS4FX.sendScreenTimeMidi = function (deck, time) {
    var timeArray = NS4FX.encodeNumToArray(time);

    var bytePrefix = [0xF0, 0x00, 0x20, 0x7F, deck, 0x04];
    var bytePostfix = [0xF7];
    var byteArray = bytePrefix.concat(timeArray, bytePostfix);
    midi.sendSysexMsg(byteArray, byteArray.length);
};

NS4FX.sendScreenBpmMidi = function (deck, bpm) {
    bpmArray = NS4FX.encodeNumToArray(bpm);
    bpmArray.shift();
    bpmArray.shift();

    var bytePrefix = [0xF0, 0x00, 0x20, 0x7F, deck, 0x01];
    var bytePostfix = [0xF7];
    var byteArray = bytePrefix.concat(bpmArray, bytePostfix);
    midi.sendSysexMsg(byteArray, byteArray.length);
};

NS4FX.elapsedToggle = function () {

    var current_setting = engine.getValue('[Controls]', 'ShowDurationRemaining');
    if (current_setting === 0) {
        // currently showing elapsed, set to remaining
        engine.setValue('[Controls]', 'ShowDurationRemaining', 1);
    } else if (current_setting === 1) {
        // currently showing remaining, set to elapsed
        engine.setValue('[Controls]', 'ShowDurationRemaining', 0);
    } else {
        // currently showing both (that means we are showing remaining, set to elapsed
        engine.setValue('[Controls]', 'ShowDurationRemaining', 0);
    }
};

NS4FX.timeElapsedCallback = function (value, group, control) {
    // 0 = elapsed
    // 1 = remaining
    // 2 = both (we ignore this as the controller can't show both)
    var on_off;
    if (value === 0) {
        // show elapsed
        on_off = 0x00;
    } else if (value === 1) {
        // show remaining
        on_off = 0x7F;
    } else {
        // both, ignore the event
        return;
    }

    // update all 4 decks on the controller
    midi.sendShortMsg(0x90, 0x46, on_off);
    midi.sendShortMsg(0x91, 0x46, on_off);
    midi.sendShortMsg(0x92, 0x46, on_off);
    midi.sendShortMsg(0x93, 0x46, on_off);
};

NS4FX.timeMs = function (deck, position, duration) {
    return Math.round(duration * position * 1000);
};

// these functions track if the user has let go of the jog wheel but it is
// still spinning
NS4FX.scratch_timer = []; // initialized before use (null is an acceptable value)
NS4FX.scratch_tick = [];  // initialized before use
NS4FX.resetScratchTimer = function (deck, tick) {
    if (!NS4FX.scratch_timer[deck]) return;
    NS4FX.scratch_tick[deck] = tick;
};

NS4FX.startScratchTimer = function (deck) {
    if (NS4FX.scratch_timer[deck]) return;

    NS4FX.scratch_tick[deck] = 0;
    NS4FX.scratch_timer[deck] = engine.beginTimer(20, () => {
        NS4FX.scratchTimerCallback(deck);
    });
};

NS4FX.stopScratchTimer = function (deck) {
    if (NS4FX.scratch_timer[deck]) {
        engine.stopTimer(NS4FX.scratch_timer[deck]);
    }
    NS4FX.scratch_timer[deck] = null;
};

NS4FX.scratchTimerCallback = function (deck) {
    // here we see if the platter is still physically moving even though the
    // platter is not being touched. For forward motion, we stop scratching
    // before the platter has physically stopped  and delay a little longer
    // when moving back. This is to mimic actual vinyl better.
    if ((NS4FX.scratch_direction[deck] // forward
        && Math.abs(NS4FX.scratch_tick[deck]) > 2)
        || (!NS4FX.scratch_direction[deck] // backward
            && Math.abs(NS4FX.scratch_tick[deck]) > 1)) {
        // reset tick detection
        NS4FX.scratch_tick[deck] = 0;
        return;
    }

    NS4FX.scratchDisable(deck);
};

NS4FX.scratchDisable = function (deck) {
    NS4FX.searching[deck] = false;
    NS4FX.stopScratchTimer(deck);
    engine.scratchDisable(deck, false);
};

NS4FX.scratchEnable = function (deck) {
    var alpha = 1.0 / 8;
    var beta = alpha / 32;

    engine.scratchEnable(deck, 1240, 33 + 1 / 3, alpha, beta);
    NS4FX.stopScratchTimer(deck);
};

// The button that enables/disables scratching
// these arrays are indexed from 1, so we initialize them with 5 values
NS4FX.touching = [false, false, false, false, false];
NS4FX.searching = [false, false, false, false, false];
NS4FX.wheelTouch = function (channel, control, value, status, group) {
    var deck = channel + 1;

    NS4FX.touching[deck] = (value === 0x7F);

    if (value === 0x7F) {
        // Touch pressed
        if (NS4FX.shift) {
            // Shift pressed -> Search (Seek)
            NS4FX.scratchDisable(deck);
            NS4FX.searching[deck] = true;
            NS4FX.stopScratchTimer(deck);
        } else if (NS4FX.wheel[channel] && !NS4FX.searching[deck]) {
            // Vinyl Mode ON -> Scratch
            NS4FX.scratchEnable(deck);
        } else {
            // Vinyl Mode OFF -> Pitch Bend (Nudge)
            NS4FX.scratchDisable(deck);
            NS4FX.searching[deck] = false;
            NS4FX.stopScratchTimer(deck);
        }
    } else {
        // Touch released
        if (NS4FX.searching[deck]) {
            NS4FX.searching[deck] = false;
        }
        NS4FX.startScratchTimer(deck);
    }
};

// The wheel that actually controls the scratching
// indexed by deck numbers starting at 1, so include an extra element
NS4FX.scratch_direction = [null, null, null, null, null]; // true == forward
NS4FX.scratch_accumulator = [0, 0, 0, 0, 0];
NS4FX.last_scratch_tick = [0, 0, 0, 0, 0];
NS4FX.wheelTurn = function (channel, control, value, status, group) {
    var deck = channel + 1;
    var direction;
    var newValue;
    if (value < 64) {
        direction = true;
    } else {
        direction = false;
    }

    // if the platter is spun fast enough, value will wrap past the 64 midpoint
    // but the platter will be spinning in the opposite direction we expect it
    // to be
    var delta = Math.abs(NS4FX.last_scratch_tick[deck] - value);
    if (NS4FX.scratch_direction[deck] !== null && NS4FX.scratch_direction[deck] != direction && delta < 64) {
        direction = !direction;
    }

    if (direction) {
        newValue = value;
    } else {
        newValue = value - 128;
    }

    // detect searching the track
    if (NS4FX.searching[deck]) {
        var position = engine.getValue(group, 'playposition');
        if (position <= 0) position = 0;
        if (position >= 1) position = 1;
        engine.setValue(group, 'playposition', position + newValue * 0.0001);
        NS4FX.resetScratchTimer(deck, newValue);
        return;
    }

    // stop scratching if the wheel direction changes and the platter is not
    // being touched
    if (NS4FX.scratch_direction[deck] === null) {
        NS4FX.scratch_direction[deck] = direction;
    }
    else if (NS4FX.scratch_direction[deck] != direction) {
        if (!NS4FX.touching[deck]) {
            NS4FX.scratchDisable(deck);
        }
        NS4FX.scratch_accumulator[deck] = 0;
    }

    NS4FX.last_scratch_tick[deck] = value;
    NS4FX.scratch_direction[deck] = direction;
    NS4FX.scratch_accumulator[deck] += Math.abs(newValue);

    // handle scratching
    if (engine.isScratching(deck)) {
        engine.scratchTick(deck, newValue); // Scratch!
        NS4FX.resetScratchTimer(deck, newValue);
    }
    // handle beat jumping
    else if (NS4FX.shift) {
        if (NS4FX.scratch_accumulator[deck] > 61) {
            NS4FX.scratch_accumulator[deck] -= 61;
            if (direction) { // forward
                engine.setParameter(group, 'beatjump_1_forward', 1);
            } else {
                engine.setParameter(group, 'beatjump_1_backward', 1);
            }
        }
    }
    // handle pitch bending
    else {
        engine.setValue(group, 'jog', newValue * 0.1); // Pitch bend
    }
};

NS4FX.wheel = []; // initialized in the NS4FX.init() function
NS4FX.wheelToggle = function (channel, control, value, status, group) {
    if (value != 0x7F) return;
    if (NS4FX.shift) {
        NS4FX.elapsedToggle();
    } else {
        NS4FX.wheel[channel] = !NS4FX.wheel[channel];
        var on_off = 0x01;
        if (NS4FX.wheel[channel]) on_off = 0x7F;
        midi.sendShortMsg(0x90 | channel, 0x07, on_off);
    }
};

NS4FX.deckSwitch = function (channel, control, value, status, group) {
    this.ignore_deck_switch = !this.ignore_deck_switch;

    if (!this.ignore_deck_switch) {
        return;
    }

    var deck = channel + 1;
    NS4FX.decks[deck].setActive(value == 0x7F);

    // change effects racks
    if (NS4FX.decks[deck].active && (channel == 0x00 || channel == 0x02)) {
        NS4FX.effectUnit.deck1 = (deck == 1);
        var left_side = true;
    }
    else if (NS4FX.decks[deck].active && (channel == 0x01 || channel == 0x03)) {
        NS4FX.effectUnit.deck2 = (deck == 2);
        var left_side = false;
    }
    NS4FX.effectUnit.updateEffectOnDeckSwitch(left_side);

    // also zero vu meters if vu displays individual decks
    if (value == 0x7F && !displayVUFromBothDecks) {
        midi.sendShortMsg(0xB0, 0x1F, 0);
        midi.sendShortMsg(0xB1, 0x1F, 0);
        midi.sendShortMsg(0xB2, 0x1F, 0);
        midi.sendShortMsg(0xB3, 0x1F, 0);
    }
};

// zero vu meters when toggling pfl
NS4FX.pflToggle = function (value, group, control) {
    midi.sendShortMsg(0xB0, 0x1F, 0);
    midi.sendShortMsg(0xB1, 0x1F, 0);
    midi.sendShortMsg(0xB2, 0x1F, 0);
    midi.sendShortMsg(0xB3, 0x1F, 0);
};

NS4FX.vuCallback = function (value, group, control) {
    // the top LED lights up at 81
    var level = value * 81;
    if (engine.getValue(group, "peak_indicator")) {
        level = 81;
    }
    if (displayVUFromBothDecks) {
        if (group == '[Main]' && control == 'vu_meter_left') {
            if (engine.getValue(group, "peak_indicator_left")) {
                level = 81;
            }
            midi.sendShortMsg(0xB0, 0x1F, level);
            midi.sendShortMsg(0xB2, 0x1F, level);
        }
        else if (group == '[Main]' && control == 'vu_meter_right') {
            if (engine.getValue(group, "peak_indicator_right")) {
                level = 81;
                midi.sendShortMsg(0xB1, 0x1F, level);
                midi.sendShortMsg(0xB3, 0x1F, level);
            }

        }
    } else {
        if (group == '[Channel1]' && NS4FX.decks[1].active) {
            midi.sendShortMsg(0xB0, 0x1F, level);
        }
        else if (group == '[Channel3]' && NS4FX.decks[3].active) {
            midi.sendShortMsg(0xB2, 0x1F, level);
        }
        else if (group == '[Channel2]' && NS4FX.decks[2].active) {
            midi.sendShortMsg(0xB1, 0x1F, level);
        }
        else if (group == '[Channel4]' && NS4FX.decks[4].active) {
            midi.sendShortMsg(0xB3, 0x1F, level);
        }
    }

};

// track the state of the shift key
NS4FX.shift = false;
NS4FX.shiftToggle = function (channel, control, value, status, group) {
    if (control === 0x20) {
        NS4FX.shift = value == 0x7F;
    }

    if (NS4FX.shift) {
        NS4FX.decks.shift();
        NS4FX.sampler_all.shift();
        NS4FX.browse.shift();
        NS4FX.head_gain.shift();

        // reset the beat jump scratch accumulators
        NS4FX.scratch_accumulator[1] = 0;
        NS4FX.scratch_accumulator[2] = 0;
        NS4FX.scratch_accumulator[3] = 0;
        NS4FX.scratch_accumulator[4] = 0;
    }
    else {
        NS4FX.decks.unshift();
        NS4FX.sampler_all.unshift();
        NS4FX.browse.unshift();
        NS4FX.head_gain.unshift();
    }
};
