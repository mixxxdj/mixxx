/*
	Author: 		DJMaxergy
	Version: 		1.00, 02/15/2024
	Description: 	Hercules DJ Console RMX 2 Mapping for Mixxx
    Source: 		http://github.com/DJMaxergy/mixxx/tree/herculesDJConsoleRMX2mapping_overhaul

    Copyright (c) 2024 DJMaxergy, licensed under GPL version 2 or later
    Copyright (c) 2016 Circuitfry, base for this mapping

    Contributors:
    - Circuitfry: initial Hercules DJ Console RMX 2 mapping

    GPL license notice for current version:
    This program is free software; you can redistribute it and/or modify it under the terms of the
    GNU General Public License as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program; if
    not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


    MIT License for earlier versions:
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software
    and associated documentation files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
    BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

var DJCRMX2 = {};

///////////////////////////////////////////////////////////////
//                       USER OPTIONS                        //
///////////////////////////////////////////////////////////////

// Sets the jogwheels sensitivity. 1 is default, 2 is twice as sensitive, 0.5 is half as sensitive.
DJCRMX2.jogwheelSensitivity = 1;

// Sets how much more sensitive the jogwheels get when holding shift.
// Set to 1 to disable jogwheel sensitivity increase when holding shift (default: 10).
DJCRMX2.jogwheelShiftMultiplier = 10;

// If true, vu meters twinkle if AutoDJ is enabled (default: true).
DJCRMX2.twinkleVumeterAutodjOn = true;

// If true, PFL / Cue (headphone) is being activated by loading a track into certain deck (default: true).
DJCRMX2.autoPFL = true;

// If true, deck vu meters show master output (L = Deck A, R = Deck B).
// If false, deck vu meter shows deck output (mono) (default: false).
DJCRMX2.vuMeterOutputMaster = false;

// If true, Samplers and EffectRack get shown or hidden in dependence of Pad-Mode (default: false).
DJCRMX2.showHideSamplersEffectsOnPadMode = false;


///////////////////////////////////////////////////////////////
//               INIT, SHUTDOWN & GLOBAL HELPER              //
///////////////////////////////////////////////////////////////

DJCRMX2.flashLEDTimer = new Array(2);
DJCRMX2.vuMeterEnabled = [true, true];

// PAD mode storage:
DJCRMX2.padModes = {
    "effect": 0x00,
    "sampler": 0x01,
    "cue": 0x03,
    "loop": 0x02
};
DJCRMX2.activePadMode = [
    DJCRMX2.padModes.effect,
    DJCRMX2.padModes.effect
];

DJCRMX2.loopIntervals = [1, 2, 4, 8];
DJCRMX2.looprollIntervals = [1 / 16, 1 / 8, 1 / 4, 1 / 2];

DJCRMX2.scratchSettings = {
    "alpha": 1.0 / 8,
    "beta": 1.0 / 8 / 32,
    "jogResolution": 256,
    "vinylSpeed": 33 + 1 / 3,
};

DJCRMX2.channelGroupsPad = {
    "[Channel1]": 0x00,
    "[Channel2]": 0x10
};

DJCRMX2.channelGroupsNonPad = {
    "[Channel1]": 0x00,
    "[Channel2]": 0x11
};

DJCRMX2.padLedGroups = {
    "effect": 0x01,
    "sampler": 0x05,
    "cue": 0x09,
    "loop": 0x0D
};

DJCRMX2.nonPadDeckLeds = {
    "play": 0x21,
    "cue": 0x22,
    "sync": 0x23,
    "killTreble": 0x28,
    "killMedium": 0x29,
    "killBass": 0x2A,
    "source": 0x2B,
    "preview": 0x2E,
    "jogWheelPressed": 0x2F
};

DJCRMX2.generalLeds = {
    "files": 0x43,
    "folders": 0x44,
    "scratch": 0x47
};


DJCRMX2.init = function(id) {
    components.Component.prototype.shiftOffset = 0x20;
    components.Component.prototype.shiftChannel = true;
    components.Component.prototype.sendShifted = true;

    DJCRMX2.id = id;
    DJCRMX2.flashLEDTimer[0] = 0;
    DJCRMX2.flashLEDTimer[1] = 0;

    // activate vu meter timer for Auto DJ:
    if (DJCRMX2.twinkleVumeterAutodjOn) {
        DJCRMX2.vuMeterTimer = engine.beginTimer(200, DJCRMX2.vuMeterTwinkle);
        if (DJCRMX2.vuMeterTimer === 0) {
            console.log("ERROR - init - vuMeterTimer could not be created");
        }
    }

    // init samplers:
    if (engine.getValue("[App]", "num_samplers") < 32) {
        engine.setValue("[App]", "num_samplers", 32);
    }

    // assign EffectUnits:
    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 1);
    engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 1);

    // init decks:
    DJCRMX2.decks = [];
    DJCRMX2.decks[1] = new DJCRMX2.Deck(1);
    DJCRMX2.decks[2] = new DJCRMX2.Deck(2);

    // init master section:
    DJCRMX2.master = new DJCRMX2.Master();

    // init microphone section:
    DJCRMX2.micOnOffButton = new components.Button({
        group: "[Microphone]",
        input: function(channel, control, value, _status, _group) {
            if (engine.getValue(this.group, "input_configured")) {
                if (value) {
                    engine.setValue(this.group, "talkover", 1);
                } else {
                    engine.setValue(this.group, "talkover", 0);
                }
            }
        },
    });

    // init library section:
    DJCRMX2.library = new DJCRMX2.Library();

    console.log("Init - Initiating control status request");
    // initiate control status request:
    midi.sendShortMsg(0xB0, 0x7F, 0x7F); // B0 7F xx -> any value
};

DJCRMX2.shutdown = function(_id) {
    engine.setValue("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", 0);
    engine.setValue("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", 0);

    DJCRMX2.resetLeds();
};

DJCRMX2.resetLeds = function() {
    let deckOffset = 0,
        led = 0,
        i = 0;

    // non-pad deck Leds:
    for (deckOffset in DJCRMX2.channelGroupsNonPad) {
        for (led in DJCRMX2.nonPadDeckLeds) {
            midi.sendShortMsg(0x90, DJCRMX2.nonPadDeckLeds[led] + DJCRMX2.channelGroupsNonPad[deckOffset], 0);
        }
    }
    // pad deck Leds:
    for (deckOffset in DJCRMX2.channelGroupsPad) {
        for (led in DJCRMX2.padLedGroups) {
            for (i = 0; i < 4; i++) {
                midi.sendShortMsg(0x90, DJCRMX2.padLedGroups[led] + DJCRMX2.channelGroupsPad[deckOffset] + i, 0);
                midi.sendShortMsg(0xB0, DJCRMX2.padLedGroups[led] + DJCRMX2.channelGroupsPad[deckOffset] + i, 0);
            }
        }
    }
    // general Leds:
    for (led in DJCRMX2.generalLeds) {
        midi.sendShortMsg(0x90, DJCRMX2.generalLeds[led], 0);
    }
    //VU meter Leds:
    for (i = 0; i < 12; i++) {
        midi.sendShortMsg(0x90, 0x49 + i, 0);
    }
};

///////////////////////////////////////////////////////////////
//                      SYSEX HANDLER                        //
///////////////////////////////////////////////////////////////
DJCRMX2.incomingData = function(data, length) {
    //[F0 00 01 4E 0E 06 0E 00 00 00 00 00 00 00 F7] Channel 1 Effect Mode
    //[F0 00 01 4E 0E 06 0E 00 00 00 01 00 00 00 F7] Channel 1 Sampler Mode
    //[F0 00 01 4E 0E 06 0E 00 00 00 03 00 00 00 F7] Channel 1 Cue Mode
    //[F0 00 01 4E 0E 06 0E 00 00 00 02 00 00 00 F7] Channel 1 Loop Mode
    //[F0 00 01 4E 0E 06 0E 00 01 00 00 00 00 00 F7] Channel 2 Effect Mode
    //[F0 00 01 4E 0E 06 0E 00 01 00 01 00 00 00 F7] Channel 2 Sampler Mode
    //[F0 00 01 4E 0E 06 0E 00 01 00 03 00 00 00 F7] Channel 2 Cue Mode
    //[F0 00 01 4E 0E 06 0E 00 01 00 02 00 00 00 F7] Channel 2 Loop Mode
    //[F0 00 01 4E 0E 06 02 00 00 00 01 00 00 00 F7] Channel 1 Shift press
    //[F0 00 01 4E 0E 06 02 00 00 00 00 00 00 00 F7] Channel 1 Shift release
    //[F0 00 01 4E 0E 06 02 00 01 00 01 00 00 00 F7] Channel 2 Shift press
    //[F0 00 01 4E 0E 06 02 00 01 00 00 00 00 00 F7] Channel 2 Shift release
    const typeIndex = data[6], //Mode = 0x0E, Shift = 0x02
        deckIndex = data[8], //Deck 1 = 0x00, Deck 2 = 0x01
        padModeIndex = data[10]; //Effect = 0x00, Sampler = 0x01, Cue = 0x03, Loop = 0x02

    if (length > 10 && typeIndex === 0x0E) {
        if (deckIndex < 0x02 && deckIndex >= 0x00) {
            if (padModeIndex < 0x04 && padModeIndex >= 0x00) {
                DJCRMX2.activePadMode[deckIndex] = padModeIndex;
                DJCRMX2.onPadModeChanged(deckIndex);
            }
        }
    }
};

DJCRMX2.onPadModeChanged = function(index) {
    const nonActiveIndex = index ? 0 : 1;
    let showSamplers = true,
        showEffectRack = true;

    if (DJCRMX2.activePadMode[index] === DJCRMX2.padModes.effect) {
        if (DJCRMX2.activePadMode[nonActiveIndex] !== DJCRMX2.padModes.sampler) {
            showSamplers = false;
        }
        showEffectRack = true;
    } else if (DJCRMX2.activePadMode[index] === DJCRMX2.padModes.sampler) {
        showSamplers = true;
        if (DJCRMX2.activePadMode[nonActiveIndex] !== DJCRMX2.padModes.effect) {
            showEffectRack = false;
        }
    } else if (DJCRMX2.activePadMode[index] === DJCRMX2.padModes.cue ||
        DJCRMX2.activePadMode[index] === DJCRMX2.padModes.loop) {
        if (DJCRMX2.activePadMode[nonActiveIndex] !== DJCRMX2.padModes.sampler) {
            showSamplers = false;
        }
        if (DJCRMX2.activePadMode[nonActiveIndex] !== DJCRMX2.padModes.effect) {
            showEffectRack = false;
        }
    }

    if (DJCRMX2.showHideSamplersEffectsOnPadMode) {
        engine.setValue("[Samplers]", "show_samplers", showSamplers ? 1 : 0);
        engine.setValue("[EffectRack1]", "show", showEffectRack ? 1 : 0);
    }
};


///////////////////////////////////////////////////////////////
//                   HELPER FUNCTIONS                        //
///////////////////////////////////////////////////////////////
DJCRMX2.doNavigate = function(direction) {
    engine.setValue("[Library]", "MoveVertical", direction);
};

DJCRMX2.getRotaryDelta = function(value) {
    let delta = 0x40 - Math.abs(0x40 - value);
    const isCounterClockwise = value > 0x40;

    if (isCounterClockwise) {
        delta *= -1;
    }
    return delta;
};


///////////////////////////////////////////////////////////////
//                   FLASH LED HANDLER                       //
///////////////////////////////////////////////////////////////
DJCRMX2.flashLEDState = false;
DJCRMX2.flashLEDHandler = function(channelGroup, padGroup, index) {
    const midiChannel = DJCRMX2.channelGroupsPad[channelGroup] + DJCRMX2.padLedGroups[padGroup] + index;

    DJCRMX2.flashLEDState = !DJCRMX2.flashLEDState;
    midi.sendShortMsg(0x90, midiChannel, DJCRMX2.flashLEDState ? 0x7F : 0x00);
};


///////////////////////////////////////////////////////////////
//                      VU - METER                           //
///////////////////////////////////////////////////////////////

DJCRMX2.vuMeterTwinkle = function() {
    if (engine.getValue("[AutoDJ]", "enabled")) {
        DJCRMX2.vuMeterEnabled[0] = !DJCRMX2.vuMeterEnabled[0];
        DJCRMX2.vuMeterEnabled[1] = !DJCRMX2.vuMeterEnabled[1];
    }
};

DJCRMX2.vuMeterLeds = function(value, group, control) {
    let deck = 0,
        midiOut = 0;
    const stepSize = 1 / 5,
        deckOffset = [0x49, 0x4F],
        peakMidiOut = engine.getValue(group, "PeakIndicator") ? 0x7F : 0x00;

    if (DJCRMX2.vuMeterOutputMaster) {
        if (control === "VuMeterL") {
            deck = 1;
        }
        if (control === "VuMeterR") {
            deck = 2;
        }
    } else {
        deck = group.match(/\d+/)[0];
    }

    for (let i = 0; i < 5; i++) {
        if (value > ((i + 0.1) * stepSize)) {
            midiOut = 0x7F;
        } else {
            midiOut = 0x00;
        }
        if (DJCRMX2.twinkleVumeterAutodjOn && engine.getValue("[AutoDJ]", "enabled")) {
            if (DJCRMX2.vuMeterEnabled[deck - 1]) {
                midiOut = 0x00;
            }
        }
        midi.sendShortMsg(0x90, i + deckOffset[deck - 1], midiOut);
    }
    midi.sendShortMsg(0x90, 0x05 + deckOffset[deck - 1], peakMidiOut);
};


////////////////////////////////////////////////////////////////////////
//                             DECKS                                  //
////////////////////////////////////////////////////////////////////////

DJCRMX2.Deck = function(deckNumbers, _channel) {
    components.Deck.call(this, deckNumbers);

    const theDeck = this;
    theDeck.samplerBankIndex = 0;

    // save set up speed slider range from the Mixxx settings:
    theDeck.setUpSpeedSliderRange = engine.getValue(this.currentDeck, "rateRange");

    this.shiftButton = function(channel, control, value, _status, _group) {
        if (value === 0x7F) {
            this.shift();
        } else {
            this.unshift();
        }
    };

    this.playButton = new components.PlayButton([0x90, DJCRMX2.nonPadDeckLeds.play + DJCRMX2.channelGroupsNonPad[this.currentDeck]]);
    this.cueButton = new components.CueButton([0x90, DJCRMX2.nonPadDeckLeds.cue + DJCRMX2.channelGroupsNonPad[this.currentDeck]]);
    this.syncButton = new components.SyncButton([0x90, DJCRMX2.nonPadDeckLeds.sync + DJCRMX2.channelGroupsNonPad[this.currentDeck]]);

    this.pflButton = new components.Button({
        midi: [0x90, DJCRMX2.nonPadDeckLeds.preview + DJCRMX2.channelGroupsNonPad[this.currentDeck]],
        type: components.Button.prototype.types.toggle,
        unshift: function() {
            this.inKey = "pfl";
            this.outKey = "pfl";
            this.disconnect();
            this.connect();
            this.trigger();
        },
        shift: function() {
            this.inKey = "keylock";
            this.outKey = "keylock";
            this.disconnect();
            this.connect();
            this.trigger();
        },
    });

    this.sourceButton = new components.Button({
        midi: [0x90, DJCRMX2.nonPadDeckLeds.source + DJCRMX2.channelGroupsNonPad[this.currentDeck]],
        key: "passthrough",
        type: components.Button.prototype.types.toggle,
    });

    this.loadButton = new components.Button({
        unshift: function() {
            this.input = function(channel, control, value, _status, _group) {
                if (value === 0x7F) {
                    engine.setValue(this.group, "LoadSelectedTrack", true);
                    if (DJCRMX2.autoPFL) {
                        engine.setValue(this.group, "pfl", true);
                        if (this.group === "[Channel1]") {
                            engine.setValue("[Channel2]", "pfl", false);
                        } else if (this.group === "[Channel2]") {
                            engine.setValue("[Channel1]", "pfl", false);
                        }
                    }
                }
            };
        },
        shift: function() {
            this.input = function(channel, control, value, _status, _group) {
                engine.setValue(this.group, "eject", value ? 1 : 0);
                if (value === 0x7F && DJCRMX2.autoPFL) {
                    engine.setValue(this.group, "pfl", false);
                }
            };
        },
    });

    this.ffButton = new components.Button({
        number: deckNumbers,
        unshift: function() {
            this.input = function(channel, control, value, _status, _group) {
                engine.setValue(this.group, "fwd", value);
            };
        },
        shift: function() {
            this.input = function(channel, control, value, _status, _group) {
                if (value === 0x7F) {
                    engine.brake(this.number, 1);
                }
            };
        },
    });

    this.rwButton = new components.Button({
        number: deckNumbers,
        unshift: function() {
            this.input = function(channel, control, value, _status, _group) {
                engine.setValue(this.group, "back", value);
            };
        },
        shift: function() {
            this.input = function(channel, control, value, _status, _group) {
                engine.spinback(this.number, value > 0);
            };
        },
    });

    this.pitchBendIncrButton = new components.Button({
        number: deckNumbers,
        unshift: function() {
            this.input = function(channel, control, value, _status, _group) {
                engine.setValue(this.group, "rate_temp_up", value);
            };
        },
        shift: function() {
            this.input = function(channel, control, value, _status, _group) {
                let range = engine.getValue(this.group, "rateRange");

                if ((range * 2) > 0.90) {
                    range = 0.90;
                } else {
                    range = range * 2;
                }

                if (value === 0x7F) {
                    engine.setValue(this.group, "rateRange", range);
                }
            };
        },
    });

    this.pitchBendDecrButton = new components.Button({
        number: deckNumbers,
        unshift: function() {
            this.input = function(channel, control, value, _status, _group) {
                engine.setValue(this.group, "rate_temp_down", value);
            };
        },
        shift: function() {
            this.input = function(channel, control, value, _status, _group) {
                let range = engine.getValue(this.group, "rateRange");

                if ((range / 2) < theDeck.setUpSpeedSliderRange) {
                    range = theDeck.setUpSpeedSliderRange;
                } else {
                    range = range / 2;
                }

                if (value === 0x7F) {
                    engine.setValue(this.group, "rateRange", range);
                }
            };
        },
    });

    this.loopButtons = [];
    this.samplerButtons = [];
    this.hotcueButtons = [];
    for (let i = 1; i <= 4; i++) {
        this.loopButtons[i] = new components.Button({
            group: this.currentDeck,
            number: i,
            unshift: function() {
                if (this.number === 1) {
                    this.input = function(channel, control, value, _status, _group) {
                        if (value === 0x7F) {
                            if (engine.getValue(this.group, "loop_enabled")) {
                                engine.setValue(this.group, "reloop_toggle", true);
                                engine.setValue(this.group, "reloop_toggle", false);
                            } else {
                                engine.setValue(this.group, "beatloop_activate", true);
                                engine.setValue(this.group, "beatloop_activate", false);
                            }
                        }
                    };
                    this.outKey = "loop_enabled";
                } else {
                    this.inKey = "beatloop_" + DJCRMX2.loopIntervals[this.number - 1] + "_toggle";
                    this.outKey = "beatloop_" + DJCRMX2.loopIntervals[this.number - 1] + "_enabled";
                }
                this.midi = [0x90, DJCRMX2.padLedGroups.loop + DJCRMX2.channelGroupsPad[this.group] + this.number - 1];
                this.disconnect();
                this.connect();
            },
            shift: function() {
                if (this.number === 1) {
                    this.input = function(channel, control, value, _status, _group) {
                        engine.setValue(this.group, "beatlooproll_" + DJCRMX2.looprollIntervals[this.number - 1] + "_activate", value);
                    };
                    this.outKey = "beatlooproll_" + DJCRMX2.looprollIntervals[this.number - 1] + "_activate";
                } else {
                    this.inKey = "beatlooproll_" + DJCRMX2.looprollIntervals[this.number - 1] + "_activate";
                    this.outKey = "beatlooproll_" + DJCRMX2.looprollIntervals[this.number - 1] + "_activate";
                }
                this.midi = [0xB0, DJCRMX2.padLedGroups.loop + DJCRMX2.channelGroupsPad[this.group] + this.number - 1];
                this.disconnect();
                this.connect();
            },
        });

        this.samplerButtons[i] = new components.SamplerButton({
            number: i,
            midi: [0x90, DJCRMX2.padLedGroups.sampler + DJCRMX2.channelGroupsPad[this.currentDeck] + i - 1],
            playing: 0x00,
            loaded: 0x7F,
            empty: 0x00,
            unshift: function() {
                this.input = function(channel, control, value, status, _group) {
                    if (this.isPress(channel, control, value, status)) {
                        if (engine.getValue(this.group, "track_loaded") === 0) {
                            engine.setValue(this.group, "LoadSelectedTrack", 1);
                        } else {
                            engine.setValue(this.group, "cue_gotoandplay", 1);
                            engine.setParameter(this.group, "volume", value / 0x7F);
                        }
                    }
                };
            },
            shift: function() {
                this.blockSamplerPad = false;
                this.input = function(channel, control, value, status, _group) {
                    if (value === 0x00) {
                        this.blockSamplerPad = false;
                    }
                    if (this.isPress(channel, control, value, status)) {
                        if (!this.blockSamplerPad) {
                            if (engine.getValue(this.group, "play") === 1) {
                                engine.setValue(this.group, "play", 0);
                            } else {
                                engine.setValue(this.group, "eject", 1);
                            }
                        }
                        this.blockSamplerPad = true;
                    } else {
                        if (engine.getValue(this.group, "play") === 0) {
                            engine.setValue(this.group, "eject", 0);
                        }
                    }
                };
            },
        });

        this.hotcueButtons[i] = new components.HotcueButton({
            number: i,
            midi: [0x90, DJCRMX2.padLedGroups.cue + DJCRMX2.channelGroupsPad[this.currentDeck] + i - 1],
        });
    }

    this.effectUnit = new DJCRMX2.EffectUnit(deckNumbers, false);
    this.effectUnit.enableButtons[1].midi = [0x90, DJCRMX2.padLedGroups.effect + DJCRMX2.channelGroupsPad[this.currentDeck] + 0x00];
    this.effectUnit.enableButtons[2].midi = [0x90, DJCRMX2.padLedGroups.effect + DJCRMX2.channelGroupsPad[this.currentDeck] + 0x01];
    this.effectUnit.enableButtons[3].midi = [0x90, DJCRMX2.padLedGroups.effect + DJCRMX2.channelGroupsPad[this.currentDeck] + 0x02];
    this.effectUnit.effectFocusButton.midi = [0x90, DJCRMX2.padLedGroups.effect + DJCRMX2.channelGroupsPad[this.currentDeck] + 0x03];
    this.effectUnit.dryWetKnob.input = function(channel, control, value, _status, _group) {
        this.inSetParameter(this.inGetParameter() + DJCRMX2.getRotaryDelta(value) / 40);
    };
    this.effectUnit.init();

    this.loopModeKnob = new components.Encoder({
        unshift: function() {
            this.inKey = "beatloop_size";
            this.input = function(channel, control, value, _status, _group) {
                if (value === 0x01) {
                    this.inSetParameter(this.inGetParameter() * 2);
                } else if (value === 0x7F) {
                    this.inSetParameter(this.inGetParameter() / 2);
                }
            };
        },
        shift: function() {
            this.inKey = "loop_move";
            this.input = function(channel, control, value, _status, _group) {
                this.inSetParameter(DJCRMX2.getRotaryDelta(value));
            };
        },
    });

    this.samplerModeKnob = new components.Encoder({
        number: deckNumbers,
        input: function(channel, control, value, _status, _group) {
            // Disconnect samplerButtons:
            for (let i = 1; i <= 4; i++) {
                if (theDeck.samplerButtons[i] !== undefined) {
                    theDeck.samplerButtons[i].disconnect();
                    theDeck.samplerButtons[i].send(this.off);
                }
            }

            // Update samplerBankIndex:
            if (value === 0x01 && theDeck.samplerBankIndex < 3) {
                theDeck.samplerBankIndex = theDeck.samplerBankIndex + 1;
            } else if (value === 0x7F && theDeck.samplerBankIndex > 0) {
                theDeck.samplerBankIndex = theDeck.samplerBankIndex - 1;
            }

            // Stop any previous flashLEDTimer:
            if (DJCRMX2.flashLEDTimer[this.number - 1] !== 0) {
                engine.stopTimer(DJCRMX2.flashLEDTimer[this.number - 1]);
            }
            // Start flashLEDTimer:
            DJCRMX2.flashLEDTimer[this.number - 1] =
                engine.beginTimer(250, () => DJCRMX2.flashLEDHandler(this.group,
                    "sampler",
                    theDeck.samplerBankIndex));
            if (DJCRMX2.flashLEDTimer[this.number - 1] === 0) {
                console.log("ERROR - SamplerModeKnob - flashLEDTimer could not be created");
            }

            // Start one-shot samplerBankEditMode stop timer (timeout):
            if (this.samplerBankEditModeTimer !== 0) {
                engine.stopTimer(this.samplerBankEditModeTimer);
            }
            this.samplerBankEditModeTimer = engine.beginTimer(3000,
                this.stopSamplerBankEditMode.bind(this),
                true);
        },
        stopSamplerBankEditMode: function() {
            // Stop flashLEDTimer:
            if (DJCRMX2.flashLEDTimer[this.number - 1] !== 0) {
                engine.stopTimer(DJCRMX2.flashLEDTimer[this.number - 1]);
            }

            // Reconnect samplerButtons according samplerBankIndex:
            for (let i = 1; i <= 4; i++) {
                if (theDeck.samplerButtons[i] !== undefined) {
                    theDeck.samplerButtons[i].number = i + (4 * theDeck.samplerBankIndex);
                    theDeck.samplerButtons[i].group = "[Sampler" + (i + (4 * theDeck.samplerBankIndex)) + "]";
                    theDeck.samplerButtons[i].connect();
                    theDeck.samplerButtons[i].trigger();
                }
            }
        },
    });

    this.cueModeKnob = new components.Encoder({
        inKey: "key",
        input: function(channel, control, value, _status, _group) {
            if (value === 0x01) {
                this.inSetParameter(this.inGetParameter() + 1);
            } else if (value === 0x7F) {
                this.inSetParameter(this.inGetParameter() - 1);
            }
        },
    });

    this.volumeFader = new components.Pot({
        inKey: "volume",
    });

    this.gainKnob = new components.Pot({
        inKey: "pregain",
    });

    this.eqKnob = [];
    this.eqKillButton = [];
    for (let k = 1; k <= 3; k++) {
        this.eqKnob[k] = new components.Pot({
            deck: this.currentDeck,
            number: k,
            unshift: function() {
                this.group = "[EqualizerRack1_" + this.deck + "_Effect1]";
                this.inKey = "parameter" + this.number;
                this.disconnect();
                this.connect();
            },
            shift: function() {
                if (this.number === 1) {
                    this.group = "[QuickEffectRack1_" + this.deck + "]";
                    this.inKey = "super1";
                    this.disconnect();
                    this.connect();
                }
            },
        });

        this.eqKillButton[k] = new components.Button({
            number: k,
            midi: [0x90, DJCRMX2.nonPadDeckLeds.killTreble + DJCRMX2.channelGroupsNonPad[this.currentDeck] + 3 - k],
            group: "[EqualizerRack1_" + this.currentDeck + "_Effect1]",
            key: "button_parameter" + k,
            type: components.Button.prototype.types.toggle,
        });
    }

    this.tempoFader = new components.Pot({
        inKey: "rate",
        invert: false,
    });

    if (!DJCRMX2.vuMeterOutputMaster) {
        engine.makeConnection(this.currentDeck, "vu_meter", DJCRMX2.vuMeterLeds);
    }

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = this.currentDeck;
        }
    });
};
DJCRMX2.Deck.prototype = new components.Deck();

DJCRMX2.Deck.prototype.wheelPress = function(value) {
    const deck = this.currentDeck.match(/\d+/)[0];
    if (this.scratchTimer !== 0) {
        // The wheel was touched again, reset the timer.
        engine.stopTimer(this.scratchTimer);
        this.scratchTimer = 0;
    }
    if (value === 0x7F) {
        // And the jog wheel is pressed down:
        engine.scratchEnable(deck,
            DJCRMX2.scratchSettings.jogResolution,
            DJCRMX2.scratchSettings.vinylSpeed,
            DJCRMX2.scratchSettings.alpha,
            DJCRMX2.scratchSettings.beta);
    } else {
        // The wheel touch sensor can be overly sensitive, so don't release scratch mode right away.
        // Depending on how fast the platter was moving, lengthen the time we'll wait.
        var scratchRate = Math.abs(engine.getValue(this.group, "scratch2"));
        var inertiaTime = Math.pow(1.8, scratchRate) * 50;
        if (inertiaTime < 100) {
            // Just do it now.
            this.finishWheelPress();
        } else {
            this.scratchTimer = engine.beginTimer(
                100, () => DJCRMX2.decks[deck].finishWheelPress(), true);
            if (this.scratchTimer === 0) {
                console.log("ERROR - wheelPress - scratchTimer could not be created");
            }
        }
    }
};

DJCRMX2.Deck.prototype.finishWheelPress = function() {
    const deck = this.currentDeck.match(/\d+/)[0];
    this.scratchTimer = 0;
    var play = engine.getValue(this.group, "play");
    if (play !== 0) {
        // If we are playing, just hand off to the engine.
        engine.scratchDisable(deck, true);
    } else {
        // If things are paused, there will be a non-smooth handoff between scratching and jogging.
        // Instead, keep scratch on until the platter is not moving.
        var scratchRate = Math.abs(engine.getValue(this.group, "scratch2"));
        if (scratchRate < 0.01) {
            // The platter is basically stopped, now we can disable scratch and hand off to jogging.
            engine.scratchDisable(deck, false);
        } else {
            // Check again soon.
            this.scratchTimer = engine.beginTimer(
                100, () => DJCRMX2.decks[deck].finishWheelPress(), true);
            if (this.scratchTimer === 0) {
                console.log("ERROR - finishWheelPress - scratchTimer could not be created");
            }
        }
    }
};

DJCRMX2.Deck.prototype.wheelTurn = function(value) {
    const deck = this.currentDeck.match(/\d+/)[0];
    var newValue = 0;
    // Spinning backwards = 127 or less (less meaning faster)
    // Spinning forwards  = 1 or more (more meaning faster)
    if (value - 64 > 0) {
        newValue = value - 128;
    } else {
        newValue = value;
    }

    if (this.isShifted) {
        newValue = newValue * DJCRMX2.jogwheelShiftMultiplier;
    }

    if (engine.isScratching(deck)) {
        engine.scratchTick(deck, newValue);
    } else {
        engine.setValue(this.currentDeck, "jog", newValue * DJCRMX2.jogwheelSensitivity);
    }
};

DJCRMX2.wheelPress = function(channel, control, value, status, group) {
    const deck = group.match(/\d+/)[0];
    DJCRMX2.decks[deck].wheelPress(value);
};

DJCRMX2.wheelTurn = function(channel, control, value, status, group) {
    const deck = group.match(/\d+/)[0];
    DJCRMX2.decks[deck].wheelTurn(value);
};

DJCRMX2.Master = function() {
    const ma = this;
    ma.group = "[Master]";
    this.group = ma.group;

    this.shiftButton = function(channel, control, value, _status, _group) {
        if (value === 0x7F) {
            this.shift();
        } else {
            this.unshift();
        }
    };

    this.masterVolumeKnob = new components.Pot({
        inKey: "gain",
    });

    this.crossfader = new components.Pot({
        inKey: "crossfader",
    });

    this.headMixKnob = new components.Pot({
        inKey: "headMix",
    });

    this.scratchButton = new components.Button({
        midi: [0x90, DJCRMX2.generalLeds.scratch],
        type: components.Button.prototype.types.toggle,
        unshift: function() {
            this.inKey = "headSplit";
            this.outKey = "headSplit";
            this.disconnect();
            this.connect();
            this.trigger();
        },
        shift: function() {
            this.inKey = "maximize_library";
            this.outKey = "maximize_library";
            this.disconnect();
            this.connect();
            this.trigger();
        },
    });

    if (DJCRMX2.vuMeterOutputMaster) {
        engine.makeConnection(this.group, "VuMeterL", DJCRMX2.vuMeterLeds);
        engine.makeConnection(this.group, "VuMeterR", DJCRMX2.vuMeterLeds);
    }

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = ma.group;
        }
    });
};
DJCRMX2.Master.prototype = new components.ComponentContainer();

DJCRMX2.Library = function() {
    const lib = this;
    lib.group = "[Library]";
    this.group = lib.group;

    this.shiftButton = function(channel, control, value, _status, _group) {
        if (value === 0x7F) {
            this.shift();
        } else {
            this.unshift();
        }
    };

    this.navigateLeftButton = new components.Button({
        midi: [0x90, DJCRMX2.generalLeds.folders],
        unshift: function() {
            this.type = components.Button.prototype.types.push;
            this.group = lib.group;
            this.inKey = "MoveFocusBackward";
            this.outKey = "MoveFocusBackward";
            this.disconnect();
            this.connect();
            this.trigger();
        },
        shift: function() {
            this.type = components.Button.prototype.types.toggle;
            this.group = "[AutoDJ]";
            this.inKey = "enabled";
            this.outKey = "enabled";
            this.disconnect();
            this.connect();
            this.trigger();
        },
    });

    this.navigateRightButton = new components.Button({
        midi: [0x90, DJCRMX2.generalLeds.files],
        unshift: function() {
            this.inKey = "GoToItem";
            this.outKey = "GoToItem";
            this.disconnect();
            this.connect();
        },
        shift: function() {
            this.inKey = "AutoDjAddBottom";
            this.outKey = "AutoDjAddBottom";
            this.disconnect();
            this.connect();
        },
    });

    this.navigateUpButton = new components.Button({
        input: function(channel, control, value, _status, _group) {
            if (value) {
                DJCRMX2.doNavigate(-1);
                this.navBtnTimer = engine.beginTimer(125, () => DJCRMX2.doNavigate(-1));
                if (this.navBtnTimer === 0) {
                    console.log("ERROR - navigateUpButton - navBtnTimer could not be created");
                }
            } else {
                if (this.navBtnTimer !== 0) {
                    engine.stopTimer(this.navBtnTimer);
                    this.navBtnTimer = 0;
                }
            }
        },
    });

    this.navigateDownButton = new components.Button({
        input: function(channel, control, value, _status, _group) {
            if (value) {
                DJCRMX2.doNavigate(1);
                this.navBtnTimer = engine.beginTimer(125, () => DJCRMX2.doNavigate(1));
                if (this.navBtnTimer === 0) {
                    console.log("ERROR - navigateDownButton - navBtnTimer could not be created");
                }
            } else {
                if (this.navBtnTimer !== 0) {
                    engine.stopTimer(this.navBtnTimer);
                    this.navBtnTimer = 0;
                }
            }
        },
    });

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = lib.group;
        }
    });
};
DJCRMX2.Library.prototype = new components.ComponentContainer();

DJCRMX2.EffectUnit = function(unitNumbers, allowFocusWhenParametersHidden) {
    const eu = this;
    const deck = unitNumbers;
    this.focusChooseModeActive = false;

    this.parameterEditModeActive = false;
    this.blockEffectPad = [false, false, false, false];

    // This is only connected if allowFocusWhenParametersHidden is false.
    this.onShowParametersChange = function(value) {
        if (value === 0) {
            // Prevent this from getting called twice (on button down and button up)
            // when show_parameters button is clicked in skin.
            // Otherwise this.previouslyFocusedEffect would always be set to 0
            // on the second call.
            if (engine.getValue(eu.group, "show_focus") > 0) {
                engine.setValue(eu.group, "show_focus", 0);
                eu.previouslyFocusedEffect = engine.getValue(eu.group,
                    "focused_effect");
                engine.setValue(eu.group, "focused_effect", 0);
            }
        } else {
            engine.setValue(eu.group, "show_focus", 1);
            if (eu.previouslyFocusedEffect !== undefined) {
                engine.setValue(eu.group, "focused_effect",
                    eu.previouslyFocusedEffect);
            }
        }
        if (eu.enableButtons !== undefined) {
            eu.enableButtons.reconnectComponents(function(button) {
                button.stopEffectFocusChooseMode();
            });
        }
    };

    this.setCurrentUnit = function(newNumber) {
        this.currentUnitNumber = newNumber;
        this.group = "[EffectRack1_EffectUnit" + newNumber + "]";

        if (allowFocusWhenParametersHidden) {
            engine.setValue(this.group, "show_focus", 0);
        } else {
            if (this.showParametersConnection !== undefined) {
                this.showParametersConnection.disconnect();
            }
            delete this.previouslyFocusedEffect;
        }
        engine.setValue(this.group, "controller_input_active", 0);

        if (allowFocusWhenParametersHidden) {
            engine.setValue(this.group, "show_focus", 1);
        } else {
            // Connect a callback to show_parameters changing instead of
            // setting show_focus when effectFocusButton is pressed so
            // show_focus is always in the correct state, even if the user
            // presses the skin button for show_parameters.
            this.showParametersConnection = engine.makeConnection(this.group,
                "show_parameters",
                this.onShowParametersChange.bind(this));
            this.showParametersConnection.trigger();
        }
        engine.setValue(this.group, "controller_input_active", 1);

        // Do not enable soft takeover upon EffectUnit construction
        // so initial values can be loaded from knobs.
        if (this.hasInitialized === true) {
            for (let n = 1; n <= 3; n++) {
                const effect = "[EffectRack1_EffectUnit" + this.currentUnitNumber +
                    "_Effect" + n + "]";
                engine.softTakeover(effect, "meta", true);
                engine.softTakeover(effect, "parameter1", true);
                engine.softTakeover(effect, "parameter2", true);
                engine.softTakeover(effect, "parameter3", true);
            }
        }

        this.reconnectComponents(function(component) {
            // update [EffectRack1_EffectUnitX] groups
            const unitMatch = component.group.match(script.effectUnitRegEx);
            if (unitMatch !== null) {
                component.group = eu.group;
            } else {
                // update [EffectRack1_EffectUnitX_EffectY] groups
                const effectMatch = component.group.match(script.individualEffectRegEx);
                if (effectMatch !== null) {
                    component.group = "[EffectRack1_EffectUnit" +
                        eu.currentUnitNumber +
                        "_Effect" + effectMatch[2] + "]";
                }
            }
        });
    };

    this.toggle = function() {
        // cycle through unitNumbers array
        let index = this.unitNumbers.indexOf(this.currentUnitNumber);
        if (index === (this.unitNumbers.length - 1)) {
            index = 0;
        } else {
            index += 1;
        }
        this.setCurrentUnit(this.unitNumbers[index]);
    };

    if (unitNumbers !== undefined && Array.isArray(unitNumbers)) {
        this.unitNumbers = unitNumbers;
    } else if (unitNumbers !== undefined && typeof unitNumbers === "number" &&
        Math.floor(unitNumbers) === unitNumbers &&
        isFinite(unitNumbers)) {
        this.unitNumbers = [unitNumbers];
    } else {
        console.warn("ERROR! new EffectUnit() called without specifying any unit numbers!");
        return;
    }

    this.group = "[EffectRack1_EffectUnit" + this.unitNumbers[0] + "]";
    this.setCurrentUnit(this.unitNumbers[0]);

    this.dryWetKnob = new components.Pot({
        group: this.group,
        unshift: function() {
            this.group = eu.group;
            this.inKey = "mix";
            // for soft takeover
            this.disconnect();
            this.connect();
        },
        shift: function() {
            this.group = eu.group;
            this.inKey = "super1";
            // for soft takeover
            this.disconnect();
            this.connect();
            // engine.softTakeoverIgnoreNextValue is called
            // in the knobs' onFocusChange function
            eu.knobs.forEachComponent(function(knob) {
                knob.trigger();
            });
        },
        outConnect: false,
        parameterEditMode: function(inGroup, index) {
            this.group = inGroup;
            this.inKey = "parameter" + index;
            this.disconnect();
            this.connect();
        },
    });

    this.EffectEnableButton = function(number) {
        this.number = number;
        this.group = "[EffectRack1_EffectUnit" + eu.currentUnitNumber +
            "_Effect" + this.number + "]";
        components.Button.call(this);
    };
    this.EffectEnableButton.prototype = new components.Button({
        type: components.Button.prototype.types.powerWindow,
        longPressed: false,
        longPressTimer: 0,
        // NOTE: This function is only connected when not in focus choosing mode.
        onFocusChange: function(value, _group, _control) {
            if (value === 0) {
                this.group = "[EffectRack1_EffectUnit" + eu.currentUnitNumber +
                    "_Effect" + this.number + "]";
                this.outKey = "enabled";
                this.input = function(channel, control, value, status, _group) {
                    if (this.isPress(channel, control, value, status)) {
                        if (!engine.getValue(this.group, "enabled")) {
                            engine.setValue(this.group, "enabled", 1);
                        } else {
                            engine.setValue(this.group, "enabled", 0);
                        }
                    }
                };
            } else {
                this.group = "[EffectRack1_EffectUnit" + eu.currentUnitNumber +
                    "_Effect" + value + "]";
                this.outKey = "button_parameter" + this.number;
                this.input = function(channel, control, value, status, _group) {
                    if (this.isPress(channel, control, value, status)) {
                        this.longPressTimer = engine.beginTimer(this.longPressTimeout,
                            this.startParameterEditMode.bind(this),
                            true);
                        if (eu.parameterEditModeActive) {
                            this.stopParameterEditMode();
                        } else {
                            if (!engine.getValue(this.group, "button_parameter" + this.number)) {
                                engine.setValue(this.group, "button_parameter" + this.number, 1);
                            } else {
                                engine.setValue(this.group, "button_parameter" + this.number, 0);
                            }
                        }
                    } else {
                        if (this.longPressTimer) {
                            engine.stopTimer(this.longPressTimer);
                        }
                    }
                };
            }
        },
        startParameterEditMode: function() {
            if (!engine.getValue(this.group, "button_parameter" + this.number)) {
                engine.setValue(this.group, "button_parameter" + this.number, 1);
            } else {
                engine.setValue(this.group, "button_parameter" + this.number, 0);
            }
            eu.parameterEditModeActive = true;
            eu.dryWetKnob.parameterEditMode(this.group, this.number);

            if (DJCRMX2.flashLEDTimer[deck - 1] !== 0) {
                engine.stopTimer(DJCRMX2.flashLEDTimer[deck - 1]);
            }
            DJCRMX2.flashLEDTimer[deck - 1] =
                engine.beginTimer(250, () => DJCRMX2.flashLEDHandler("[Channel" + deck + "]",
                    "effect",
                    (this.number - 1)));
            if (DJCRMX2.flashLEDTimer[deck - 1] === 0) {
                console.log("ERROR - startParameterEditMode - flashLEDTimer could not be created");
            }
        },
        stopParameterEditMode: function() {
            eu.parameterEditModeActive = false;
            eu.dryWetKnob.unshift();

            if (DJCRMX2.flashLEDTimer[deck - 1] !== 0) {
                engine.stopTimer(DJCRMX2.flashLEDTimer[deck - 1]);
            }
            eu.enableButtons.reconnectComponents(function(button) {
                button.onFocusChange();
            });
        },
        stopEffectFocusChooseMode: function() {
            this.type = components.Button.prototype.types.powerWindow;
            this.input = components.Button.prototype.input;
            this.output = components.Button.prototype.output;

            this.connect = function() {
                this.connections[0] = engine.makeConnection(eu.group, "focused_effect",
                    this.onFocusChange.bind(this));
                // this.onFocusChange sets this.group and this.outKey, so trigger it
                // before making the connection for LED output
                this.connections[0].trigger();
                this.connections[1] = engine.makeConnection(this.group, this.outKey,
                    this.output.bind(this));
            };

            this.unshift = function() {
                this.group = "[EffectRack1_EffectUnit" +
                    eu.currentUnitNumber + "_Effect" +
                    this.number + "]";
                this.input = function(channel, control, value, status, _group) {
                    if (this.isPress(channel, control, value, status)) {
                        if (!engine.getValue(this.group, "enabled")) {
                            engine.setValue(this.group, "enabled", 1);
                        } else {
                            engine.setValue(this.group, "enabled", 0);
                        }
                    }
                };
                this.disconnect();
                this.connect();
                this.trigger();
            };
            this.shift = function() {
                this.group = "[EffectRack1_EffectUnit" +
                    eu.currentUnitNumber + "_Effect" +
                    this.number + "]";
                eu.blockEffectPad[this.number - 1] = false;
                this.input = function(channel, control, value, status, _group) {
                    if (value === 0x00) {
                        eu.blockEffectPad[this.number - 1] = false;
                    }
                    if (this.isPress(channel, control, value, status)) {
                        if (!eu.blockEffectPad[this.number - 1]) {
                            engine.setValue(this.group, "enabled", 1);
                        }
                        eu.blockEffectPad[this.number - 1] = true;
                    } else {
                        engine.setValue(this.group, "enabled", 0);
                    }
                    engine.setValue(this.group, "meta", value / 0x7F);
                };
            };
            if (this.isShifted) {
                this.shift();
            }
        },
        startEffectFocusChooseMode: function() {
            this.input = function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status)) {
                    if (engine.getValue(eu.group, "focused_effect") === this.number) {
                        // unfocus and make knobs control metaknobs
                        engine.setValue(eu.group, "focused_effect", 0);
                    } else {
                        // focus this effect
                        engine.setValue(eu.group, "focused_effect", this.number);
                    }
                }
            };
            this.output = function(value, _group, _control) {
                this.send((value === this.number) ? this.on : this.off);
            };
            this.connect = function() {
                // Outside of focus choose mode, the this.connections array
                // has two members. Connections can be triggered when they
                // are disconnected, so overwrite the whole array here instead
                // of assigning to this.connections[0] to avoid
                // Component.prototype.trigger() triggering the disconnected connection.
                this.connections = [engine.makeConnection(eu.group,
                    "focused_effect",
                    this.output.bind(this))];
            };
        },
    });

    this.knobs = new components.ComponentContainer();
    this.enableButtons = new components.ComponentContainer();
    for (let n = 1; n <= 3; n++) {
        this.enableButtons[n] = new this.EffectEnableButton(n);
    }

    this.effectFocusButton = new components.Button({
        group: this.group,
        longPressed: false,
        longPressTimer: 0,
        pressedWhenParametersHidden: false,
        previouslyFocusedEffect: 0,
        startEffectFocusChooseMode: function() {
            this.send(this.on);
            eu.focusChooseModeActive = true;
            eu.enableButtons.reconnectComponents(function(button) {
                button.startEffectFocusChooseMode();
            });
        },
        unshift: function() {
            this.input = function(channel, control, value, status, _group) {
                const showParameters = engine.getValue(this.group, "show_parameters");
                if (this.isPress(channel, control, value, status)) {
                    this.longPressTimer = engine.beginTimer(this.longPressTimeout,
                        this.startEffectFocusChooseMode.bind(this),
                        true);
                    if (!showParameters) {
                        if (!allowFocusWhenParametersHidden) {
                            engine.setValue(this.group, "show_parameters", 1);
                            // eu.onShowParametersChange will refocus the
                            // previously focused effect and show focus in skin
                        }
                        this.pressedWhenParametersHidden = true;
                    }
                } else {
                    if (this.longPressTimer) {
                        engine.stopTimer(this.longPressTimer);
                    }

                    if (eu.focusChooseModeActive) {
                        this.trigger();
                        eu.enableButtons.reconnectComponents(function(button) {
                            button.stopEffectFocusChooseMode();
                        });
                        eu.focusChooseModeActive = false;
                    } else {
                        if (!showParameters && allowFocusWhenParametersHidden) {
                            engine.setValue(this.group, "show_parameters", 1);
                        } else if (showParameters && !this.pressedWhenParametersHidden) {
                            engine.setValue(this.group, "show_parameters", 0);
                            // eu.onShowParametersChange will save the focused effect,
                            // unfocus, and hide focus buttons in skin
                        }
                    }
                    this.pressedWhenParametersHidden = false;
                }
            };
        },
        shift: function() {
            eu.blockEffectPad[3] = false;
            this.input = function(channel, control, value, status, _group) {
                if (value === 0x00) {
                    eu.blockEffectPad[3] = false;
                }
                if (this.isPress(channel, control, value, status)) {
                    if (!eu.blockEffectPad[3]) {
                        eu.toggle();
                    }
                    eu.blockEffectPad[3] = true;
                }
            };
        },
        outKey: "focused_effect",
        output: function(value, _group, _control) {
            this.send((value > 0) ? this.on : this.off);
        },
        outConnect: false,
    });

    this.init = function() {
        this.knobs.reconnectComponents();
        this.enableButtons.reconnectComponents(function(button) {
            button.stopEffectFocusChooseMode();
        });
        this.effectFocusButton.connect();
        this.effectFocusButton.trigger();

        this.forEachComponent(function(component) {
            if (component.group === undefined) {
                component.group = eu.group;
            }
        });

        this.hasInitialized = true;
    };
};
DJCRMX2.EffectUnit.prototype = new components.ComponentContainer();
