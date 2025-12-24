/**
 * Stanton DJC.4 controller script v1.0 for Mixxx v2.2.3
 *
 * Written by Martin Bruset Solberg
 * Adopted for v2.2.3 by Christoph Zimmermann
 *
 * Based on:
 *  Denon MC2000 script by Esteban Serrano Roloff,
 *  Denon MC7000 script by OsZ
 *  Roland DJ-505 script by Jan Holthuis
 *
 * TODO:
 * Pitch range
 *
 **/

var DJC4 = {};

/////////////////
// Tweakables. //
/////////////////

DJC4.autoShowFourDecks = false;
DJC4.showMasterVu = true;  // if set to false, show channel VU meter

// amount the dryWetKnob changes the value for each increment
DJC4.dryWetAdjustValue = 0.05;

///////////
// Code. //
///////////

// ----------   Global variables    ----------

// MIDI Reception commands (from spec)
DJC4.leds = {
    loopminus: 2,
    loopplus: 3,
    loopin: 4,
    loopout: 5,
    loopon: 6,
    loopdel: 7,
    hotcue1: 8,
    hotcue2: 9,
    hotcue3: 10,
    hotcue4: 11,
    sample1: 12,
    sample2: 13,
    sample3: 14,
    sample4: 15,
    keylock: 16,
    sync: 18,
    pbendminus: 19,
    pbendplus: 20,
    scratch: 21,
    tap: 22,
    cue: 23,
    play: 24,
    highkill: 25,
    midkill: 26,
    lowkill: 27,
    pfl: 28,
    fxon: 30,
    fxexf1: 31,
    fxexf2: 32,
    fxexf3: 33,
    loadac: 34,
    loadbd: 35,
    videofx: 36,
    xflink: 37,
    keyon: 38,
    filteron: 39,
    tx: 46,
    fx: 47
};

// ----------   Functions    ----------

// Called when the MIDI device is opened & set up.
DJC4.init = function() {
    var i;

    // Put all LEDs to default state.
    DJC4.allLed2Default();

    engine.makeConnection("[Channel3]", "track_loaded", DJC4.autoShowDecks);
    engine.makeConnection("[Channel4]", "track_loaded", DJC4.autoShowDecks);

    if (engine.getValue("[App]", "num_samplers") < 8) {
        engine.setValue("[App]", "num_samplers", 8);
    }

    DJC4.browseEncoder = new components.Encoder({
        group: "[Library]",
        inKey: "Move",
        input: function(channel, control, value) {
            if (value === 0x41) {
                engine.setParameter(this.group, this.inKey + "Down", 1);
            } else if (value === 0x3F) {
                engine.setParameter(this.group, this.inKey + "Up", 1);
            }
        },
        unshift: function() {
            this.inKey = "Move";
        },
        shift: function() {
            this.inKey = "Scroll";
        },
    });

    DJC4.deck = [];
    for (i = 0; i < 4; i++) {
        DJC4.deck[i] = new DJC4.Deck(i + 1);
        DJC4.deck[i].setCurrentDeck("[Channel" + (i + 1) + "]");
    }

    DJC4.effectUnit = [];
    for (i = 0; i <= 3; i++) {
        DJC4.effectUnit[i] = new components.EffectUnit([i + 1]);
        DJC4.effectUnit[i].shiftOffset = 0x32;
        DJC4.effectUnit[i].shiftControl = true;
        DJC4.effectUnit[i].enableButtons[1].midi = [0x90 + i, 0x1F];
        DJC4.effectUnit[i].enableButtons[2].midi = [0x90 + i, 0x20];
        DJC4.effectUnit[i].enableButtons[3].midi = [0x90 + i, 0x21];
        DJC4.effectUnit[i].effectFocusButton.midi = [0x90 + i, 0x1D];
        DJC4.effectUnit[i].knobs[1].midi = [0xB0 + i, 0x09];
        DJC4.effectUnit[i].knobs[2].midi = [0xB0 + i, 0x0A];
        DJC4.effectUnit[i].knobs[3].midi = [0xB0 + i, 0x0B];
        DJC4.effectUnit[i].dryWetKnob.midi = [0xB0 + i, 0x08];
        DJC4.effectUnit[i].dryWetKnob.input = function(channel, control, value) {
            if (value === 0x41) {
                this.inSetParameter(this.inGetParameter() + DJC4.dryWetAdjustValue);
            } else if (value === 0x3F) {
                this.inSetParameter(this.inGetParameter() - DJC4.dryWetAdjustValue);
            }
        };
        DJC4.effectUnit[i].init();
    }

    // === Master VU Meter ===
    if (DJC4.showMasterVu === true) {
        DJC4.vuMeter = new components.Component({
            midi: [0xB0, 0x03],
            group: "[Master]",
            outKey: "vu_meter_left",
            output: function(value, group) {
                // The red LEDs light up with MIDI values greater than 0x60.
                // The Red LEDs should only be illuminated if the track is clipping.
                if (engine.getValue(group, "peak_indicator") === 1) {
                    value = 0x60;
                } else {
                    value = Math.round(value * 0x54);
                }
                this.send(value);
            },
        });

        DJC4.vuMeter = new components.Component({
            midi: [0xB0, 0x04],
            group: "[Master]",
            outKey: "vu_meter_right",
            output: function(value, group) {
                // The red LEDs light up with MIDI values greater than 0x60.
                // The Red LEDs should only be illuminated if the track is clipping.
                if (engine.getValue(group, "peak_indicator") === 1) {
                    value = 0x60;
                } else {
                    value = Math.round(value * 0x54);
                }
                this.send(value);
            },
        });
    }
};

// Called when the MIDI device is closed
DJC4.shutdown = function() {
    // Put all LEDs to default state.
    DJC4.allLed2Default();
};

DJC4.Deck = function(deckNumber) {
    components.Deck.call(this, deckNumber);

    // === Instantiate controls ===
    this.beatLoopEncoder = new components.Encoder({
        midi: [0xB0 + deckNumber - 1, 0x01],
        group: "[Channel" + deckNumber + "]",
        inKey: "loop_move_1",
        input: function(channel, control, value) {
            if (value === 0x41) {
                engine.setParameter(this.group, this.inKey + "_forward", 1);
            } else if (value === 0x3F) {
                engine.setParameter(this.group, this.inKey + "_backward", 1);
            }
        },
    });

    this.samplerButtons = [];
    for (var i = 0; i <= 3; i++) {
        this.samplerButtons[i] = new components.SamplerButton({
            number: (deckNumber === 1 || deckNumber === 3) ? (i + 1) : (i + 5),
            midi: [0x90+deckNumber-1, 0x0C+i],
        });
    }

    // === Channel VU Meter ===
    if (DJC4.showMasterVu === false) {
        DJC4.vuMeter = new components.Component({
            midi: [0xB0+deckNumber-1, 0x02],
            group: "[Channel" + deckNumber + "]",
            outKey: "vu_meter",
            output: function(value, group) {
                // The red LEDs light up with MIDI values greater than 0x60.
                // The Red LEDs should only be illuminated if the track is clipping.
                if (engine.getValue(group, "peak_indicator") === 1) {
                    value = 0x60;
                } else {
                    value = Math.round(value * 0x54);
                }
                this.send(value);
            },
        });
    }

    // === Scratch control ===
    this.scratchMode = false;

    this.toggleScratchMode = function(channel, control, value) {
        if (value === 0x7F) {
            // Toggle setting
            this.scratchMode = !this.scratchMode;
            DJC4.setLed(script.deckFromGroup(this.currentDeck), DJC4.leds["scratch"], this.scratchMode);
        }
    };

    // ============================= JOG WHEELS ==============================
    this.wheelTouch = function(channel, control, value, status, group) {
        if (engine.getValue(group, "play") === 0) {
            // If not playing, do a fast search
            if (value === 0x7F) {
                var alpha = 1.0 / 8;
                var beta = alpha / 32;
                var rpm = 40.0;

                engine.scratchEnable(script.deckFromGroup(this.currentDeck), 128, rpm, alpha, beta, true);
            } else {    // If button up
                engine.scratchDisable(script.deckFromGroup(this.currentDeck));
            }
        } else if (this.scratchMode === true) {
            // If scratch enabled
            if (value === 0x7F) {
                alpha = 1.0/8;
                beta = alpha/32;
                rpm = 150.0;

                engine.scratchEnable(script.deckFromGroup(this.currentDeck), 128, rpm, alpha, beta);
            } else {    // If button up
                engine.scratchDisable(script.deckFromGroup(this.currentDeck));
            }
        } else {    // If button up
            engine.scratchDisable(script.deckFromGroup(this.currentDeck));
        }
    };

    this.wheelTurn = function(channel, control, value) {
        // When the jog wheel is turned in clockwise direction, value is
        // greater than 64 (= 0x40). If it's turned in counter-clockwise
        // direction, the value is smaller than 64.
        var newValue = value - 64;
        var deck = script.deckFromGroup(this.currentDeck);
        if (engine.isScratching(deck)) {
            engine.scratchTick(deck, newValue); // Scratch!
        } else if (this.shifted === true) { // If shift is pressed
            var oldPos = engine.getValue(this.currentDeck, "playposition");
            // Since ‘playposition’ is normalized to unity, we need to scale by
            // song duration in order for the jog wheel to cover the same amount
            // of time given a constant turning angle.
            var duration = engine.getValue(this.currentDeck, "duration");
            var newPos = Math.max(0, oldPos + (newValue * DJC4.stripSearchScaling / duration));
            engine.setValue(this.currentDeck, "playposition", newPos); // Strip search
        } else {
            engine.setValue(this.currentDeck, "jog", newValue); // Pitch bend
        }
    };
};

// === FOR MANAGING LEDS ===

DJC4.allLed2Default = function() {
    // All LEDs OFF for deck 1 to 4
    var i = 0;
    for (i = 1; i <= 4; i++) {
        for (var led in DJC4.leds) {
            DJC4.setLed(i, DJC4.leds[led], 0);
        }
        // Channel VU meter
        midi.sendShortMsg(0xB0 + (i - 1), 2, 0);
    }
    // Master VU meter
    midi.sendShortMsg(0xB0, 3, 0);
    midi.sendShortMsg(0xB0, 4, 0);
};

// Set leds function
DJC4.setLed = function(deck, led, status) {
    var ledStatus = 0x00; // Default OFF
    switch (status) {
    case 0:
        ledStatus = 0x00;
        break; // OFF
    case false:
        ledStatus = 0x00;
        break; // OFF
    case 1:
        ledStatus = 0x7F;
        break; // ON
    case true:
        ledStatus = 0x7F;
        break; // ON
    default:
        break;
    }
    midi.sendShortMsg(0x90 + (deck - 1), led, ledStatus);
};

// === MISC COMMON ===

DJC4.autoShowDecks = function() {
    var anyLoaded = engine.getValue("[Channel3]", "track_loaded") || engine.getValue("[Channel4]", "track_loaded");
    if (!DJC4.autoShowFourDecks) {
        return;
    }
    engine.setValue("[Master]", "show_4decks", anyLoaded);
};

DJC4.shiftButton = function(channel, control, value) {
    var i;
    if (value === 0x7F) {
        DJC4.browseEncoder.shift();
        for (i = 0; i < 4; i++) {
            DJC4.deck[i].shift();
            DJC4.effectUnit[i].shift();
        }
    } else {
        DJC4.browseEncoder.unshift();
        for (i = 0; i < 4; i++) {
            DJC4.deck[i].unshift();
            DJC4.effectUnit[i].unshift();
        }
    }
};

DJC4.crossfaderCurve = function(channel, control, value) {
    script.crossfaderCurve(value, 0, 0x7F);
};

// === Sampler Volume Control ===
DJC4.samplerVolume = function(channel, control, value) {
    // check if the Sampler Volume is at Zero and if so hide the sampler bank
    if (value > 0x00) {
        engine.setValue("[Samplers]", "show_samplers", true);
    } else {
        engine.setValue("[Samplers]", "show_samplers", false);
    }
    // get the Sampler Row opened with its details
    engine.setValue("[SamplerRow1]", "expanded", true);

    // control up to 8 sampler volumes with the one knob on the mixer
    for (var i = 1; i <= 8; i++) {
        engine.setValue("[Sampler" + i + "]", "pregain",
            script.absoluteNonLin(value, 0, 1.0, 4.0));
    }
};


// give your custom Deck all the methods of the generic Deck in the Components library
DJC4.Deck.prototype = Object.create(components.Deck.prototype);
