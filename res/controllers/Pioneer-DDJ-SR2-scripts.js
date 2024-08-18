/*
Author: Waylon Robertson
    Version: 2, 24/01/2021
    Description: Pioneer DDJ-SR2 Controller Mapping for Mixxx
    Source: https://github.com/WaylonR/mixxx/tree/feat_Pioneer_DDJ_SR2

    Copyright (c) 2024 Waylon Robertson, licensed under GPL version 2 or later
    Copyright (c) 2014-2015 various contributors, base for this mapping, licensed under MIT license

    Contributors:
    - DJMaxergy: original DDJ-SX mapping for Mixxx
    - Michael Stahl (DG3NEC): original DDJ-SB2 mapping for Mixxx 2.0
    .- Sophia Herzog: midiAutoDJ-scripts
    - Joan Ardiaca Jové (joan.ardiaca@gmail.com): Pioneer DDJ-SB mapping for Mixxx 2.0
    - wingcom (wwingcomm@gmail.com): start of Pioneer DDJ-SB mapping
      https://github.com/wingcom/Mixxx-Pioneer-DDJ-SB
    - Hilton Rudham: Pioneer DDJ-SR mapping
      https://github.com/hrudham/Mixxx-Pioneer-DDJ-SR
    - Jan Holthuis: Roland DJ-505 mapping
	  original brief: https://mixxx.discourse.group/t/roland-dj-505/17916 but now part of main.

    This program is free software; you can redistribute it and/or modify it under the terms of the
    GNU General Public License as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License along with this program; if
    not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


*/
// eslint-disable-next-line no-var
var DDJSR2 = {};

///////////////////////////////////////////////////////////////
//                       USER OPTIONS                        //
///////////////////////////////////////////////////////////////

// Sets the jogwheels sensitivity. 1 is default, 2 is twice as sensitive, 0.5 is half as sensitive.
DDJSR2.jogwheelSensitivity = 2;

// Sets how much more sensitive the jogwheels get when holding shift.
// Set to 1 to disable jogwheel sensitivity increase when holding shift (default: 10).
DDJSR2.jogwheelShiftMultiplier = 10;

DDJSR2.rateRanges = [0.08, 0.16, 0.32, 0.64, 0.90, 1.00];

DDJSR2.scratchSettings = {
    alpha: 1.0 / 8,
    beta: 1.0 / 8 / 32,
    wheelResolution: 51000,
    vinylSpeed: 33 + 1 / 3,
};

DDJSR2.wheelLedCircle = {
    minVal: 0,
    maxVal: 0x14,
    blink: 0x3
};

// The following variables controls the wheel ring colors.
// If set to OFF - no ring color. 
// If set to a color that's in the wheelLedCircleColor list (has to be in caps), sets the deck color to that. 
// If set to TRACK - sets the color to what the track column says.
// If set to POSITION - Changes color as playposition changes.
DDJSR2.wheelColor =[];
DDJSR2.wheelColor[1] = "POSITION"; // Deck 1
DDJSR2.wheelColor[2] = "ORANGE"; // Deck 2
DDJSR2.wheelColor[3] = "CYAN"; // Deck 3
DDJSR2.wheelColor[4] = "BRIGHTBLUE"; // Deck 4

DDJSR2.wheelLedCircleColor = {
    OFF:20,
    TRACK:127,
    POSITION:128,
    PURPLE:1,
    MAGENTA:2,
    RED:3,
    BROWN:4,
    ORANGE:5,
    YELLOW:6,
    LIMEGREEN:7,
    GREEN:8,
    BRIGHTGREEN:9,
    CELESTE:10,
    CYAN:11,
    TURQUOISE:12,
    BRIGHTBLUE:13,
    AZURE:14,
    DARKBLUE:15,
    VIOLET:16,
    ULTRAVIOLET:17,
    WHITE:18,
    DIMWHITE:19
};

DDJSR2.wheelColorMap = new ColorMapper({
    0xCC0000: DDJSR2.wheelLedCircleColor.RED,
    0x008000: DDJSR2.wheelLedCircleColor.GREEN,
    0x0000CC: DDJSR2.wheelLedCircleColor.BLUE,
    0xFFFF00: DDJSR2.wheelLedCircleColor.YELLOW,
    0x0088CC: DDJSR2.wheelLedCircleColor.CELESTE,
    0xFF8800: DDJSR2.wheelLedCircleColor.ORANGE,
    0x895129: DDJSR2.wheelLedCircleColor.BROWN,
    0xFF00FF: DDJSR2.wheelLedCircleColor.MAGENTA,
    0xEE82EE: DDJSR2.wheelLedCircleColor.VIOLET,
    0x8800CC: DDJSR2.wheelLedCircleColor.PURPLE,
    0xFFFFFF: DDJSR2.wheelLedCircleColor.WHITE,
});


////////////
// Code. //
//////////

DDJSR2.seratoHeartbeat=[0xF0, 0x00, 0x20, 0x7F, 0x50, 0x01, 0xF7]; // Serato Heartbeat
DDJSR2.seratoControlsPoll=[0xF0, 0x00, 0x20, 0x7F, 0x03, 0x01, 0xF7]; // Serato first poll

DDJSR2.init = function() {
    // Browser section is part of Mixer container for now.
    
    DDJSR2.browser = new DDJSR2.BrowserContainer();

    DDJSR2.deck = [];
    for (var i = 0; i < 4; i++) {
        DDJSR2.deck[i] = new DDJSR2.Deck(i);
        DDJSR2.deck[i].setCurrentDeck(`[Channel${i+1}]`);
    }

    DDJSR2.mixer = new DDJSR2.MixerContainer();

    DDJSR2.createEffectUnits();

    // Performance pads are part of the Deck specs, as are Illumination Controls

    DDJSR2.seratoHeartbeatTimer = engine.beginTimer(250, DDJSR2.doSeratoHeartbeatTimer, false);

    // After 500 ms have elapsed, after the serato heartbeat has occurred, then poll controls. Calling this before time results in an unsuccessful poll.

    engine.beginTimer(500, function() {
        midi.sendSysexMsg(DDJSR2.seratoControlsPoll, DDJSR2.seratoControlsPoll.length);
    }, true);

};

DDJSR2.doSeratoHeartbeatTimer = function() {
    midi.sendSysexMsg(DDJSR2.seratoHeartbeat, DDJSR2.seratoHeartbeat.length);
};


// First, a function library.
////////////////


DDJSR2.getRotaryDelta = function(value) {
    let delta = 0x40 - Math.abs(0x40 - value),
        isCounterClockwise = value > 0x40;
    // BrowserKnob returns 1-30 going down, and 127-98 going up.

    if (isCounterClockwise) {
        delta *= -1;
    }
    return delta;
};

DDJSR2.getJogWheelDelta = function(value) {
    // The Wheel control centers on 0x40; find out how much it's moved by.
    return value - 0x40;
};

DDJSR2.wheelLedControl = function(channelOffset, color) {
    const wheelLedBaseChannel = 0xBB;
    midi.sendShortMsg(
        wheelLedBaseChannel,
        0x20+(channelOffset),
        color
    );
};


DDJSR2.pitchBendFromJog = function(group, movement) {
    engine.setValue(group, "jog", movement / 5 * DDJSR2.jogwheelSensitivity);
};

DDJSR2.RingBufferView = class {
    constructor(indexable, startIndex = 0) {
        this.indexable = indexable;
        this.index = startIndex;
    }
    advanceBy(n) {
        this.index = script.posMod(this.index + n, this.indexable.length);
        return this.current();
    }
    next() {
        return this.advanceBy(1);
    }
    previous() {
        return this.advanceBy(-1);
    }
	current() {
        return this.indexable[this.index];
    }
};

DDJSR2.BrowserContainer = function(){
    this.browseKnob  = new components.ComponentContainer({
        longPressTimer: 0,
        longPressTimeout: 250,
        trackColorCycleEnabled: false,
        trackColorCycleHappened: false,
        previewSeekEnabled: false,
        previewSeekHappened: false,
        turn: new components.Encoder({
            group: "[Library]",
            input: function(channel, control, value, _status, _group) {
                const rotateValue = DDJSR2.getRotaryDelta(value);
                engine.setValue("[Library]", "MoveVertical", rotateValue);
            }
	}),
	press: new components.Button({
	    group: "[Library]",
	    key: "GoToItem"
	}),
	shiftPress: new components.Button({
	    group: "[Library]",
	    input: function (channel, control, value, status, group) {
                if (!value) { // On release, not press. Allows for track color cycling.
                    this.trackColorCycleEnabled = false;
		    if (!this.trackColorCycleHappened){
                        if (!engine.getValue("[PreviewDeck1]", "play")) {
                            engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 1);
                        } else
   		        { 
	                    script.triggerControl("[PreviewDeck1]", "stop");
		        } 
	            } else {
		        this.trackColorCycleHappened = false;
		    }
                } else {
		    this.trackColorCycleEnabled = true;
		}
            }.bind(this)
	}),
	shiftTurn: new components.Encoder({
	    group: "[PreviewDeck1]",
	    input: function (channel, control, value, status, group) {
                var rotateValue = DDJSR2.getRotaryDelta(value);
                if (!this.trackColorCycleEnabled){
                    const oldPos = engine.getValue("[PreviewDeck1]", "playposition");
                    const newPos = Math.max(0, oldPos + (0.05 * rotateValue));
                    engine.setValue("[PreviewDeck1]", "playposition", newPos);
                } else {
                    const key = (rotateValue > 0) ? "track_color_next" : "track_color_prev";
                    engine.setValue("[Library]", key, 1.0);
                    this.trackColorCycleHappened = true;
                }
            }.bind(this)
        }),
    });
    this.loadButtons = [];
    for (var channelOffset = 0; channelOffset < 4; channelOffset++){
        const group = `[Channel${channelOffset+1}]`;
	this.loadButtons[channelOffset] = new components.Button({
	    group: group,
	    key: "LoadSelectedTrack",
	    midi: [0x9B, 0x0+channelOffset]
	});
        this.ejectTrackButton = new components.Button({
            inKey: "eject",
            outKey: "track_loaded",
            midi: [0x96, 0x46+channelOffset]
        });
    };

    this.sortLibrary = function(channel, control, value, _status, _group) {
        if (value === 0) {
            return;
        }

        let sortColumn;
        switch (control) {
            case 0x58:  // BPM
            sortColumn = script.LIBRARY_COLUMNS.BPM;
                break;
            case 0x59:  // ARTIST
            sortColumn = script.LIBRARY_COLUMNS.ARTIST;
                break;
            default:
              // unknown sort column
              return;
        }
        engine.setValue("[Library]", "sort_column", sortColumn);
    };


    this.backButton = new components.Button({
        group: "[Library]",
        inKey: "MoveFocusBackward"
    });

    this.shiftBackButton = new components.Button({
        group: "[Master]",
        inKey: "maximize_library"
    });

    this.loadPrepareButton = new components.Button({
        group: "[Library]",
        inKey: "AutoDjAddBottom"
    });

    this.loadPrepareButtonShifted = new components.Button({
        group: "[Library]",
        inKey: "AutoDjAddTop"
    });


};
DDJSR2.BrowserContainer.prototype = new components.ComponentContainer();
DDJSR2.ticks = 0;
DDJSR2.Deck = function(channelOffset) {
    var deckNumber = channelOffset + 1;
    this.deckNumber = deckNumber;
    this.group = "[Channel" + deckNumber + "]";
    var theDeck = this;
    components.Deck.call(this, deckNumber);
    // ============================= TRANSPORT ==================================
    this.play = new components.PlayButton({
        midi: [0x90+channelOffset, 0x0B]
    });
    this.stutter = new components.Button({
        midi: [0x90+channelOffset, 0x47],
	key: "play_stutter"
    });
    this.cue = new components.CueButton({
        midi: [0x90 + channelOffset, 0x0C]
    });
    this.cuerewind = new components.Button({
        midi: [0x90 + channelOffset, 0x48],
        key: "start_stop"
    });
    this.jog = new components.JogWheelBasic({ // Jog Wheel is crap on the SR2, takes a big swing around to even get one midi tick. Use Platter instead.
        deck: deckNumber,
        wheelResolution: DDJSR2.scratchSettings.wheelResolution,
        alpha: DDJSR2.scratchSettings.alpha,
        beta: DDJSR2.scratchSettings.beta,
        vinylMode: true, // Set the default to match the SR2's default.
        inValueScale: function (value) { 
            if (!this.vinylMode) {
                const delta = DDJSR2.getJogWheelDelta(value);
                const newvalue = delta / 5 * DDJSR2.jogwheelSensitivity;
                return newvalue;
            } else {
                return value < 0x40 ? value - (this.max + 1) : value;
            }
        }
    });
    this.tempoFader = new components.Pot({
        invert: true,
        key: "rate"
    });
    const rates = new DDJSR2.RingBufferView(DDJSR2.rateRanges);
    this.tempoRange = new components.Button({
        inKey: "rateRange",
        input: function(channel, control, value, status, _group) {
            if (this.isPress(channel, control, value, status)) {
                this.inSetValue(rates.next());
            }
        }
    });
    this.tempoReset = new components.Button({
        inKey: "rate",
        input: function(channel, control, value, status, _group) {
            if (this.isPress(channel, control, value, status)) {
                this.inSetValue(0);
            }
        }
    });
    this.keyLock = new components.Button({
        midi: [0x90 + channelOffset, 0x1A],
        key: "keylock",
        type: components.Button.prototype.types.toggle
    });
    this.needleSearchStripPositionShifted = new components.Pot({
            group: theDeck.group,
            key: "playposition"
    });
    this.needleSearchStripPosition = new components.Pot({
        key: "playposition",
        group: theDeck.group,
        input: function(channel, control, value, _status, _group) {
            if (!engine.getParameter(this.group, "play")) {
                const newValue = this.inValueScale(value);
                this.inSetParameter(newValue);
            }
        }
    });
    this.sync = new components.Button({
        midi: [0x90 + channelOffset, 0x58],
        key: "sync_enabled",
        type: components.Button.prototype.types.toggle
    });
    this.syncOff = new components.Button({
        midi: [0x90 + channelOffset, 0x5C],
        key: "sync_enabled",
	input:  function(channel, control, value, status, group) {
	    this.inSetValue(false);
	}
    });
    this.autoLoop = new components.Button({
        midi: [0x90 + channelOffset, 0x14],
        outKey: "loop_enabled",
	input: function(channel, control, value, status, group) {
			    if (value) {
                if (engine.getValue(group, "loop_enabled")) {
                    engine.setValue(group, "reloop_toggle", true);
                    engine.setValue(group, "reloop_toggle", false);
                } else {
                    engine.setValue(group, "beatloop_activate", true);
                    engine.setValue(group, "beatloop_activate", false);
                }
            }
	}
    });

    this.loopActive = new components.Button({
        midi: [0x90 + channelOffset, 0x50],
        key: "loop_enabled",
        type: components.Button.prototype.types.toggle,
    });

    this.loopHalve = new components.Button({
        midi: [0x90 + channelOffset, 0x12],
        key: "loop_halve",
    });

    this.loopDouble = new components.Button({
        midi: [0x90 + channelOffset, 0x13],
        key: "loop_double",
    });

    this.loopShiftBackward = new components.Button({
        midi: [0x90 + channelOffset, 0x61],
        key: "beatjump_backward",
    });

    this.loopShiftForward = new components.Button({
        midi: [0x90 + channelOffset, 0x62],
        key: "beatjump_forward",
    });

    this.loopIn = new components.Button({
        midi: [0x90 + channelOffset, 0x10],
        key: "loop_in",
    });

    this.loopOut = new components.Button({
        midi: [0x90 + channelOffset, 0x11],
        key: "loop_out",
    });

    this.slotSelect = new components.Button({
        midi: [0x90 + channelOffset, 0x4C],
        key: "quantize",
        type: components.Button.prototype.types.toggle,
    });

    this.reloopExit = new components.Button({
        midi: [0x90 + channelOffset, 0x4D],
        key: "reloop_toggle",
    });

    this.censor = new components.Button({
        midi: [0x90+channelOffset, 0x15],
        key: "reverseroll"
    });

    this.reverse = new components.Button({
        midi: [0x90+channelOffset, 0x38],
	key: "reverse",
        type: components.Button.prototype.types.toggle
    });

    this.slipButton = new components.Button({
        midi: [0x90 + channelOffset, 0x40],
        key: "slip_enabled",
        type: components.Button.prototype.types.toggle
    });

    this.vinylButton = new components.Button({
        midi: [0x90+channelOffset, 0x17],
        type: components.Button.prototype.types.toggle,
        input: function(channel, control, value, status, group) {
            if (this.isPress(channel, control, value, status, group)) {
                theDeck.jog.vinylMode = !theDeck.jog.vinylMode;
                this.send(theDeck.jog.vinylMode);
            }
        }
    });

    this.vinylButton.trigger();

    this.brakeStopButton = function(channel, control, value, status, group) {
        const activate = value > 0;
        if (activate) { // act on button press
            engine.brake(deckNumber, true); // slow down the track
        }
    };

    this.softStartButton = function(channel, control, value, status, group) {
        const activate = value > 0;
        if (activate) { // act on button press
            engine.softStart(deckNumber, true);
        }
    };

    this.gridSetButton = new components.Button({
        midi: [0x90 + channelOffset, 0x64],
        key: "beats_translate_curpos"
    });

    this.keySync = new components.Button({
        midi: [0x90 + channelOffset, 0x70],
        key: "sync_key"
    });

    this.keyShiftDown = new components.Button({
        midi: [0x90 + channelOffset, 0x72],
        key: "pitch_down"
    });

    this.keyShiftUp = new components.Button({
        midi: [0x90 + channelOffset, 0x73],
        key: "pitch_up"
    });

    this.keyReset = new components.Button({
        // midi: [x,x], no light
        key: "reset_key"
    });
    this.wheelRing = new WheelRing({
        color: DDJSR2.wheelColor[deckNumber],
        deckNumber: deckNumber,
        channelOffset: channelOffset
    });

    // Attach the pads to the deck instance
    this.padSection = new DDJSR2.PadSection(this, channelOffset);
};

DDJSR2.Deck.prototype = Object.create(components.Deck.prototype);


DDJSR2.MixerContainer = function(){
    this.channels = [];
    for (let i = 0; i < 4; i++){
        this.channels[i] = new DDJSR2.Channel(i);
    }

    this.crossfader = new components.Pot({
        group: "[Master]",
        inKey: "crossfader",
    });
    DDJSR2.panels = [false, false];
    this.panelSelectButton = function(channel, control, value, _status, _group) {
        if (value) {
            if ((DDJSR2.panels[0] === false) && (DDJSR2.panels[1] === false)) {
                DDJSR2.panels[0] = true;
            } else if ((DDJSR2.panels[0] === true) && (DDJSR2.panels[1] === false)) {
                DDJSR2.panels[1] = true;
            } else if ((DDJSR2.panels[0] === true) && (DDJSR2.panels[1] === true)) {
                DDJSR2.panels[0] = false;
            } else if ((DDJSR2.panels[0] === false) && (DDJSR2.panels[1] === true)) {
                DDJSR2.panels[1] = false;
            }

            engine.setValue("[Samplers]", "show_samplers", DDJSR2.panels[0]);
            engine.setValue("[EffectRack1]", "show", DDJSR2.panels[1]);
        }
    };

    this.shiftPanelSelectButton = function(channel, control, value, status, group) {
        if (value) {
            if ((DDJSR2.panels[0] === false) && (DDJSR2.panels[1] === false)) {
                DDJSR2.panels[1] = true;
            } else if ((DDJSR2.panels[1] === true) && (DDJSR2.panels[0] === false)) {
                DDJSR2.panels[0] = true;
            } else if ((DDJSR2.panels[0] === true) && (DDJSR2.panels[1] === true)) {
                DDJSR2.panels[1] = false;
            } else if ((DDJSR2.panels[1] === false) && (DDJSR2.panels[0] === true)) {
                DDJSR2.panels[0] = false;
            }

            engine.setValue("[Samplers]", "show_samplers", DDJSR2.panels[0]);
            engine.setValue("[EffectRack1]", "show", DDJSR2.panels[1]);
        }
    };
};

DDJSR2.MixerContainer.prototype = new components.ComponentContainer();

DDJSR2.Channel = function(channelOffset) {
    const deck = `[Channel${ channelOffset + 1 }]`;
    this.pregain = new components.Pot({
        // midi: [0xB0 + offset, 0x16],
        group: deck,
        inKey: "pregain",
    });

    this.eqKnob = [];
    // EQ High, Mid and Low 
    for (let k = 1; k <= 3; k++) {
        this.eqKnob[k] = new components.Pot({
            group: `[EqualizerRack1_${  deck  }_Effect1]`,
            inKey: `parameter${  k}`,
        });
    }

    this.pfl = new components.Button({
        midi: [0x90 + channelOffset, 0x54],
        group: deck,
        type: components.Button.prototype.types.toggle,
        key: "pfl",
    });

    this.tapBPM = new components.Button({
        midi: [0x90 + channelOffset, 0x68],
        key: "bpm_tap"
    });

    this.filter = new components.Pot({
        group: `[QuickEffectRack1_${ deck }]`,
        inKey: "super1",
    });


    this.volume = new components.Pot({
        inKey: "volume",
	group: deck
    });

    // Master level is handled at hardware level for the SR2, same with Booth Monitor Level, Headphones Mixing, and Sampler Volume

    let lastVuLevel = 0;
    this.vuMeter = new components.Component({
        outKey: "vu_meter",
	group: deck,
        output: function(value, group, _control) {
            // Remark: Only deck vu meters can be controlled! Master vu meter is handled by hardware!
            let midiBaseAddress = 0xB0,
                channel = 0x02,
                midiOut = 0x00;

            value = parseInt(value * 0x76); //full level indicator: 0x7F

            if (engine.getValue(group, "peak_indicator")) {
                value = value + 0x09;
            }

            if (value === lastVuLevel){
                // return early if value hasn't changed
                return
            } else {
                lastVuLevel = value;

                midi.sendShortMsg(
                    midiBaseAddress + channelOffset,
                    channel,
                    value
                );
            }
        }
    });
};
DDJSR2.Channel.prototype = new components.ComponentContainer();

DDJSR2.createEffectUnits = function(){
    DDJSR2.effectUnits = [];
    var fxLeds = [0x4C, 0x50, 0x70, 0x54];
    for (var i = 1; i <= 2; i++) {
        DDJSR2.effectUnits[i] = new components.EffectUnit(i);
        DDJSR2.effectUnits[i].enableButtons[1].midi = [0x93 + i, 0x47];
        DDJSR2.effectUnits[i].enableButtons[2].midi = [0x93 + i, 0x48];
        DDJSR2.effectUnits[i].enableButtons[3].midi = [0x93 + i, 0x49];
        DDJSR2.effectUnits[i].dryWetKnob.input = function(channel, control, value, _status, _group) {
            this.inSetParameter(this.inGetParameter() + DDJSR2.getRotaryDelta(value) / 30);
        };
            for (var j = 1; j <= 4; j++) {
		const channel = `Channel${j}`;
                DDJSR2.effectUnits[i].enableOnChannelButtons.addButton(channel);
                const midiout = fxLeds[j-1]+(i-1);
                DDJSR2.effectUnits[i].enableOnChannelButtons[channel].midi = [0x96, midiout];
            }

        DDJSR2.effectUnits[i].init();
    }
};

// Performance Pad code
///////////////////////

DDJSR2.PadMode = {
    HOTCUE: 0x1B,
    ROLL: 0x1E,
    SLICER: 0x20,
    SAMPLER: 0x22,
    SAMPLER2: 0x23,
    CUELOOP: 0x69,
    SAVEDLOOP: 0x6b,
    SLICERLOOP: 0x6d,
    PITCHPLAY: 0x6E,
    TRANS: 0x6C,
};

DDJSR2.PadColor = {
    OFF: 0x3F,
    BLUE: 0x01,
    CELESTE: 0xB,
    GREEN: 0x15,
    YELLOW: 0x1F,
    ORANGE: 0X24,
    RED: 0x2A,
    PINK: 0x2F,
    MAGENTA: 0x33,
    PURPLE: 0x36,
    VIOLET: 0x3c,
    WHITE: 0x40,
    DIM_MODIFIER: 0x40
};

DDJSR2.PadColorMap = new ColorMapper({
    0xCC0000: DDJSR2.PadColor.RED,
    0x008000: DDJSR2.PadColor.GREEN,
    0x0000CC: DDJSR2.PadColor.BLUE,
    0xFFFF00: DDJSR2.PadColor.YELLOW,
    0x0088CC: DDJSR2.PadColor.CELESTE,
    0xFF8800: DDJSR2.PadColor.ORANGE,
    0xFF00FF: DDJSR2.PadColor.MAGENTA,
    0xEE82EE: DDJSR2.PadColor.VIOLET,
    0x8800CC: DDJSR2.PadColor.PURPLE,
    0xFFC0CB: DDJSR2.PadColor.PINK,
    0xFFFFFF: DDJSR2.PadColor.WHITE,
});


DDJSR2.PadSection = function(deck, offset) {
    // TODO: Add support for missing modes (flip, slicer, slicerloop)
    /*
     * The Performance Pad Section on the SR2 has two basic
     * modes of operation that determines how the LEDs react to MIDI messages
     * and button presses.
     *
     * 1. Standalone/MIDI mode
     *
     * The controller's performance pads allow setting various "modes" using
     * the mode buttons and the shift modifier. Pressing the mode buttons will
     * change their LED color (and makes the performance pads barely lit in
     * that color, too).
     *
     * 2. Serato mode
     *
     * In this mode, pressing the pad mode buttons will not change their color.
     * Instead, all LEDs have to be controlled by sending MIDI messages. Unlike
     * in Standalone mode, it is also possible to illuminate the pad LEDs.
     *
     * The following table gives an overview over the different performance pad
     * modes. The values in the "Serato LED" and "Serato Mode" columns have
     * been taken from the Owner's Manual.
     *
     * Button                         MIDI control Standalone LED   Serato LED   Serato Mode
     * ------------------------------ ------------ ---------------- ------------ -----------
     * [HOT CUE]                      0x00         White            White        Hot Cue
     * [HOT CUE] (press twice)        0x02         Blue             Orange       Saved Flip
     * [SHIFT] + [HOT CUE]            0x03         Orange           Blue         Cue Loop
     * [ROLL]                         0x08         Turqoise         Light Blue   Roll
     * [ROLL] (press twice)           0x0D         Red              Green        Saved Loop
     * [SHIFT] + [ROLL]               0x09         Blue             Red          Slicer
     * [SHIFT] + [ROLL] (press twice) 0x0A         Blue             Blue         Slicer Loop
     * [TR]                           0x04         Red              Red          TR
     * [TR] (press twice)             0x06         Orange           Orange       TR Velocity
     * [SHIFT] + [TR]                 0x05         Green            Green        Pattern (Switches TR-S pattern)
     * [SAMPLER]                      0x0B         Purple           Magenta      Sampler
     * [SAMPLER] (press twice)        0x0F         Aquamarine       Green        Pitch Play
     * [SHIFT] + [SAMPLER]            0x0C         Magenta          Purple       Velocity Sampler
     *
     * The Pad and Mode Buttons support 31 different LED states:
     *
     *   MIDI value Color          MIDI value Color
     *   ---------- -----          ---------- -----
     *   0x00       Off            0x10       Off
     *   0x01       Red            0x11       Red (Dim)
     *   0x02       Orange         0x12       Orange (Dim)
     *   0x03       Blue           0x13       Blue (Dim)
     *   0x04       Yellow         0x14       Yellow (Dim)
     *   0x05       Applegreen     0x15       Applegreen (Dim)
     *   0x06       Magenta        0x16       Magenta (Dim)
     *   0x07       Celeste        0x17       Celeste (Dim)
     *   0x08       Purple         0x18       Purple (Dim)
     *   0x09       Apricot        0x19       Apricot (Dim)
     *   0x0A       Coral          0x1A       Coral (Dim)
     *   0x0B       Azure          0x1B       Azure (Dim)
     *   0x0C       Turquoise      0x1C       Turquoise (Dim)
     *   0x0D       Aquamarine     0x1D       Aquamarine (Dim)
     *   0x0E       Green          0x1E       Green (Dim)
     *   0x0F       White          0x1F       White (Dim)
     *
     * Serato DJ Pro maps its cue colors to MIDI values like this:
     *
     *   Number Default Cue  Serato Color       MIDI value Color
     *   ------ ------------ -----------------  ---------  ----------
     *        1            1 #CC0000 / #C02626  0x01       Red
     *        2              #CC4400 / #DB4E27  0x0A       Coral
     *        3            2 #CC8800 / #F8821A  0x02       Orange
     *        4            4 #CCCC00 / #FAC313  0x04       Yellow
     *        5              #88CC00 / #4EB648  0x0E       Green
     *        6              #44CC00 / #006838  0x0E       Green
     *        7            5 #00CC00 / #1FAD26  0x05       Applegreen
     *        8              #00CC44 / #8DC63F  0x0D       Aquamarine
     *        9              #00CC88 / #2B3673  0x0D       Aquamarine
     *       10            7 #00CCCC / #1DBEBD  0x0C       Turquoise
     *       11              #0088CC / #0F88CA  0x07       Celeste
     *       12              #0044CC / #16308B  0x03       Blue
     *       13            3 #0000CC / #173BA2  0x03       Blue
     *       14              #4400CC / #5C3F97  0x0B       Azure
     *       15            8 #8800CC / #6823B6  0x08       Purple
     *       16            6 #CC00CC / #CE359E  0x06       Magenta
     *       17              #CC0088 / #DC1D49  0x06       Magenta
     *       18              #CC0044 / #C71136  0x01       Red
*/
    components.ComponentContainer.call(this);
    this.modes = {
        // This need to be an object so that a recursive reconnectComponents
        // call won't influence all modes at once
        "hotcue": new DDJSR2.HotcueMode(deck, offset),
        "sampler": new DDJSR2.SamplerMode(deck, offset),
        "roll": new DDJSR2.RollMode(deck, offset),
        "pitchplay": new DDJSR2.PitchPlayMode(deck, offset),
        /*        "slicer":     new DDJSR2.SlicerMode(deck, offset),
		"cueloop":    new DDJSR2.CueLoopMode(deck, offset),
		"savedloop":  new DDJSR2.SavedLoop(deck, offset),
		"slicerloop": new DDJSR2.SlicerLoop(deck, offset),

		"trans":      new DDJSR2.TransMode(deck, offset)
*/
    };
    this.offset = offset;

    // Start in Hotcue Mode and disable other LEDs
    this.setPadMode(DDJSR2.PadMode.HOTCUE);
//    midi.sendShortMsg(0x94 + offset, this.modes.roll.ledControl, DDJSR2.PadColor.OFF);
//    midi.sendShortMsg(0x94 + offset, this.modes.sampler.ledControl, DDJSR2.PadColor.OFF);
};

DDJSR2.PadSection.prototype = Object.create(components.ComponentContainer.prototype);
/*
DDJSR2.PadMode = {
    HOTCUE: 0x1B,
    ROLL: 0x1E,
    SLICER: 0x20,
    SAMPLER: 0x22,
    CUELOOP: 0x69,
    SAVEDLOOP: 0x6B,
    SLICERLOOP: 0x6D,
    PITCHPLAY: 0x6E,
    TRANS: 0x6C,
};
*/

DDJSR2.PadSection.prototype.controlToPadMode = function(control) {
    let mode;
    switch (control) {
    case DDJSR2.PadMode.HOTCUE:
        mode = this.modes.hotcue;
   	    break;
    case DDJSR2.PadMode.ROLL:
	    mode = this.modes.roll;
	    break;
    case DDJSR2.PadMode.SAMPLER:
	    mode = this.modes.sampler;
	    break;
    case DDJSR2.PadMode.SAMPLER2:
	    mode = this.modes.sampler;
	    break;
    	case DDJSR2.PadMode.PITCHPLAY:
	    mode = this.modes.pitchplay;
	    break;
/*		case DDJSR2.PadMode.SLICR:
		    mode = this.modes.slicer;
			break;
	    case DDJSR2.PadMode.CUELOOP:
		case DDJSR2.PadMode.SAVEDLOOP:
		case DDJSR2.PadMode.SLICERLOOP:
		    mode = null;
			break;

		case DDJSR2.PadMode.TRANS:
		    mode = this.modes.trans;
			break;
		*/
    }
    return mode;
};

DDJSR2.PadSection.prototype.padModeButtonPressed = function(channel, control, value, _status, _group) {
    if (value) {
        this.setPadMode(control);
    }
};

DDJSR2.PadSection.prototype.setPadMode = function(control) {
    const newMode = this.controlToPadMode(control);

    // Exit early if the requested mode is already active or not mapped
    if (newMode === this.currentMode || newMode === undefined) {
        return;
    }

    // If we're switching away from or to a hardware-based mode (e.g. TR mode),
    // the performance pad behaviour is hardcoded in the firmware and not
    // controlled by Mixxx. These modes are represented by the value null.
    // Hence, we only need to change LEDs and (dis-)connect components if
    // this.currentMode or newMode is not null.
    if (this.currentMode) {
        // Disable the mode button LED of the currently active mode
        midi.sendShortMsg(0x90 + this.offset, this.currentMode.ledControl, 0x00);

        this.currentMode.forEachComponent(function(component) {
            component.disconnect();
        });
    }

    if (newMode) {
        // Illuminate the mode button LED of the new mode
        midi.sendShortMsg(0x90 + this.offset, newMode.ledControl, newMode.color);

        // Set the correct shift state for the new mode. For example, if the
        // user is in HOT CUE mode and wants to switch to CUE LOOP mode, you
        // need to press [SHIFT]+[HOT CUE]. Pressing [SHIFT] will make the HOT
        // CUE mode pads become shifted.
        // When you're in CUE LOOP mode and want to switch back to
        // HOT CUE mode, the user need to press HOT CUE (without holding
        // SHIFT). However, the HOT CUE mode pads are still shifted even though
        // the user is not pressing [SHIFT] because they never got the unshift
        // event (the [SHIFT] button was released in CUE LOOP mode, not in HOT
        // CUE mode).
        // Hence, it's necessary to set the correct shift state when switching
        // modes.
        if (this.isShifted) {
            newMode.shift();
        } else {
            newMode.unshift();
        }

        newMode.forEachComponent(function(component) {
            component.connect();
            component.trigger();
        });
    }
    this.currentMode = newMode;
};

DDJSR2.PadSection.prototype.padPressed = function(channel, control, value, status, group) {
    if (this.currentMode) {
        this.currentMode.pads[control].input(channel, control, value, status, group);
    }
};

DDJSR2.HotcueMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DDJSR2.PadMode.HOTCUE;
    this.color = DDJSR2.PadColor.WHITE;

    this.pads = new components.ComponentContainer();
    for (let i = 0; i <= 7; i++) {
        this.pads[i] = new components.HotcueButton({
            midi: [0x97 + offset, 0x0 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            number: i + 1,
            group: deck.currentDeck,
            on: this.color,
            off: DDJSR2.PadColor.OFF,
            colorMapper: DDJSR2.PadColorMap,
            outConnect: false,
        });
    }
    this.clearHotcueButton = new components.Button({
	input: function(channel, control, value, status, group) {
          const index = control - 0x08 + 1;
          script.toggleControl(group, `hotcue_${  index  }_clear`);
        }
    });

    this.paramMinusButton = new components.Button({
        midi: [0x90 + offset, 0x24],
        group: deck.currentDeck,
        key: "hotcue_focus_color_prev",
    });
    this.paramPlusButton = new components.Button({
        midi: [0x90 + offset, 0x2C],
        group: deck.currentDeck,
        key: "hotcue_focus_color_next",
    });
    this.param2MinusButton = new components.Button({
        midi: [0x90 + offset, 0x01],
        group: deck.currentDeck,
        key: "beats_translate_earlier",
    });
    this.param2PlusButton = new components.Button({
        midi: [0x90 + offset, 0x09],
        group: deck.currentDeck,
        key: "beats_translate_later",
    });
};
DDJSR2.HotcueMode.prototype = Object.create(components.ComponentContainer.prototype);

DDJSR2.RollMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DDJSR2.PadMode.ROLL;
    this.color = DDJSR2.PadColor.CELESTE;
    this.pads = new components.ComponentContainer();
    this.loopSize = 0.03125;
    this.minSize = 0.03125;  // 1/32
    this.maxSize = 32;

    let loopSize;
    let i;
    for (i = 0; i <= 3; i++) {
        loopSize = (this.loopSize * Math.pow(2, i));
        this.pads[0x10+i] = new components.Button({
            midi: [0x97 + offset, 0x10 + i],
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
            group: deck.currentDeck,
            outKey: `beatloop_${  loopSize  }_enabled`,
            inKey: `beatlooproll_${  loopSize  }_activate`,
            outConnect: false,
            on: this.color,
            off: (loopSize === 0.25) ? DDJSR2.PadColor.CELESTE : ((loopSize === 4) ? DDJSR2.PadColor.GREEN : (this.color + DDJSR2.PadColor.DIM_MODIFIER)),
        });
    }
    this.pads[0x10+4] = new components.Button({
        midi: [0x97 + offset, 0x14],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 8,
        group: deck.currentDeck,
        key: "beatjump_backward",
        outConnect: false,
        off: DDJSR2.PadColor.RED,
        on: DDJSR2.PadColor.RED + DDJSR2.PadColor.DIM_MODIFIER,
    });
    this.pads[0x10+5] = new components.Button({
        midi: [0x97 + offset, 0x15],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 8,
        group: deck.currentDeck,
        outKey: "beatjump_size",
        outConnect: false,
        on: DDJSR2.PadColor.ORANGE,
        off: DDJSR2.PadColor.ORANGE + DDJSR2.PadColor.DIM_MODIFIER,
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                const jumpSize = engine.getValue(this.group, "beatjump_size");
                if (jumpSize > this.mode.minSize) {
                    engine.setValue(this.group, "beatjump_size", jumpSize / 2);
                }
            }
        },
        output: function(value, _group, _control) {
            this.send((value > this.mode.minSize) ? this.on : this.off);
        },
    });
    this.pads[0x10+6] = new components.Button({
        midi: [0x97 + offset, 0x16],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 8,
        group: deck.currentDeck,
        outKey: "beatjump_size",
        outConnect: false,
        on: DDJSR2.PadColor.ORANGE,
        off: DDJSR2.PadColor.ORANGE + DDJSR2.PadColor.DIM_MODIFIER,
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                const jumpSize = engine.getValue(this.group, "beatjump_size");
                if (jumpSize < this.mode.maxSize) {
                    engine.setValue(this.group, "beatjump_size", jumpSize * 2);
                }
            }
        },
        output: function(value, _group, _control) {
            this.send((value < this.mode.maxSize) ? this.on : this.off);
        },
    });
    this.pads[0x10+7] = new components.Button({
        midi: [0x97 + offset, 0x17],
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 8,
        group: deck.currentDeck,
        key: "beatjump_forward",
        outConnect: false,
        off: DDJSR2.PadColor.RED,
        on: DDJSR2.PadColor.RED + DDJSR2.PadColor.DIM_MODIFIER,
    });


    this.paramMinusButton = new components.Button({
        midi: [0x94 + offset, 0x28],
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                if (this.mode.loopSize > this.mode.minSize) {
                    this.mode.setLoopSize(this.mode.loopSize / 2);
                }
            }
            this.send(value);
        },
    });
    this.paramPlusButton = new components.Button({
        midi: [0x94 + offset, 0x29],
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                if (this.mode.loopSize * 8 < this.mode.maxSize) {
                    this.mode.setLoopSize(this.mode.loopSize * 2);
                }
            }
            this.send(value);
        },
    });
};
DDJSR2.RollMode.prototype = Object.create(components.ComponentContainer.prototype);
DDJSR2.RollMode.prototype.setLoopSize = function(loopSize) {
    this.loopSize = loopSize;
    let padLoopSize;
    for (let i = 0; i <= 3; i++) {
        padLoopSize = (this.loopSize * Math.pow(2, i));
        this.pads[0x10+i].inKey = `beatlooproll_${  padLoopSize  }_activate`;
        this.pads[0x10+i].outKey = `beatloop_${  padLoopSize  }_enabled`;
        this.pads[0x10+i].off = (padLoopSize === 0.25) ? DDJSR2.PadColor.CELESTE : ((padLoopSize === 4) ? DDJSR2.PadColor.GREEN : (this.color + DDJSR2.PadColor.DIM_MODIFIER));
    }
    this.reconnectComponents();
};

DDJSR2.SamplerMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = DDJSR2.PadMode.SAMPLER;
    this.color = DDJSR2.PadColor.PURPLE;
    this.pads = new components.ComponentContainer();
    for (let i = 0; i <= 7; i++) {
        this.pads[0x30+i] = new components.SamplerButton({
            midi: [0x97 + offset, 0x30 + i],
            number: i + 1,
            outConnect: true,
            playing: DDJSR2.PadColor.PURPLE,
            on: this.color,
            loaded: DDJSR2.PadColor.PURPLE+DDJSR2.PadColor.DIM_MODIFIER,
            off: DDJSR2.PadColor.OFF,
        });
    };
    this.stopSamplerButton = new components.Button({
	    input: function(channel, control, value, status, group) {
	        DDJSR2.selectedSamplerBank = 0;
                const index = control - 0x38 + 1,
                deckOffset = DDJSR2.selectedSamplerBank * 8,
                sampleDeck = `[Sampler${  index + deckOffset  }]`,
                trackLoaded = engine.getValue(sampleDeck, "track_loaded"),
                playing = engine.getValue(sampleDeck, "play");                

            if (trackLoaded && playing) {
                script.toggleControl(sampleDeck, "stop");
            } else if (trackLoaded && !playing && value) {
                script.toggleControl(sampleDeck, "eject");
            }
        }
    });
}
DDJSR2.SamplerMode.prototype = Object.create(components.ComponentContainer.prototype);

DDJSR2.PitchPlayMode = function(deck, offset) {
    components.ComponentContainer.call(this);

    const PitchPlayRange = {
        UP: 0,
        MID: 1,
        DOWN: 2,
    };

    this.ledControl = DDJSR2.PadMode.PITCHPLAY;
    this.color = DDJSR2.PadColor.GREEN;
    this.cuepoint = 1;
    this.range = PitchPlayRange.MID;

    this.PerformancePad = function(n) {
        this.midi = [0x97 + offset, 0x70 + n];
        this.number = n + 1;
        this.on = this.color + DDJSR2.PadColor.DIM_MODIFIER;
        this.colorMapper = DDJSR2.PadColorMap;
        this.colorKey = `hotcue_${  this.number  }_color`;
        components.Button.call(this);
    };
    this.PerformancePad.prototype = new components.Button({
        group: deck.currentDeck,
        mode: this,
        outConnect: false,
        off: DDJSR2.PadColor.OFF,
        unshift: function() {
            this.outKey = "pitch_adjust";
            this.output = function(_value, _group, _control) {
                let color = this.mode.color + DDJSR2.PadColor.DIM_MODIFIER;
                if ((this.mode.range === PitchPlayRange.UP && this.number === 5) ||
                    (this.mode.range === PitchPlayRange.MID && this.number === 1) ||
                    (this.mode.range === PitchPlayRange.DOWN && this.number === 4)) {
                    color = DDJSR2.PadColor.WHITE;
                }
                this.send(color);
            };
            this.input = function(channel, control, value, _status, _group) {
                if (value > 0) {
                    let pitchAdjust;
                    switch (this.mode.range) {
                    case PitchPlayRange.UP:
                        pitchAdjust = this.number + ((this.number <= 4) ? 4 : -5);
                        break;
                    case PitchPlayRange.MID:
                        pitchAdjust = this.number - ((this.number <= 4) ? 1 : 9);
                        break;
                    case PitchPlayRange.DOWN:
                        pitchAdjust = this.number - ((this.number <= 4) ? 4 : 12);
                    }
                    engine.setValue(this.group, "pitch_adjust", pitchAdjust);
                    engine.setValue(this.group, `hotcue_${  this.mode.cuepoint  }_activate`, value);
                }
            };
            this.connect = function() {
                components.Button.prototype.connect.call(this); // call parent connect

                if (this.connections[1] !== undefined) {
                    // Necessary, since trigger() apparently also triggers disconnected connections
                    this.connections.pop();
                }
            };
            if (this.connections[0] !== undefined) {
                this.disconnect();
                this.connect();
                this.trigger();
            }
        }
    });
    this.ShiftedPerformancePad = function(n) {
        this.midi = [0x97 + offset, 0x78 + n];
        this.number = n + 1;
        this.on = this.color + DDJSR2.PadColor.DIM_MODIFIER;
        this.colorMapper = DDJSR2.PadColorMap;
        this.colorKey = `hotcue_${  this.number  }_color`;
        components.Button.call(this);
    };
    this.ShiftedPerformancePad.prototype = new components.Button({
        group: deck.currentDeck,
        mode: this,
        outConnect: false,
        off: DDJSR2.PadColor.OFF,
        outputColor: function(colorCode) {
            // For colored hotcues (shifted only)
            const midiColor = this.colorMapper.getValueForNearestColor(colorCode);
            this.send((this.mode.cuepoint === this.number) ? midiColor : (midiColor + DDJSR2.PadColor.DIM_MODIFIER));
        },
        unshift: function() {
            this.outKey = `hotcue_${  this.number  }_enabled`;
            this.output = function(value, _group, _control) {
                const outval = this.outValueScale(value);
                if (this.colorKey !== undefined && outval !== this.off) {
                    this.outputColor(engine.getValue(this.group, this.colorKey));
                } else {
                    this.send(DDJSR2.PadColor.OFF);
                }
            };
            this.input = function(channel, control, value, _status, _group) {
                if (value > 0 && this.mode.cuepoint !== this.number && engine.getValue(this.group, `hotcue_${  this.number  }_enabled`)) {
                    const previousCuepoint = this.mode.cuepoint;
                    this.mode.cuepoint = this.number;
                    this.mode.pads[0x78+previousCuepoint - 1].trigger();
                    this.outputColor(engine.getValue(this.group, this.colorKey));
                }
            };
            this.connect = function() {
                components.Button.prototype.connect.call(this); // call parent connect
                if (undefined !== this.group && this.colorKey !== undefined) {
                    this.connections[1] = engine.makeConnection(this.group, this.colorKey, function(id) {
                        if (engine.getValue(this.group, this.outKey)) {
                            this.outputColor(id);
                        }
                    });
                }
            };
            if (this.connections[0] !== undefined) {
                this.disconnect();
                this.connect();
                this.trigger();
            }
        },
    });
    this.pads = new components.ComponentContainer();
    for (let n = 0; n <= 7; n++) {
        this.pads[0x70+n] = new this.PerformancePad(n);
        this.pads[0x78+n] = new this.ShiftedPerformancePad(n);
    }

    this.paramMinusButton = new components.Button({
        midi: [0x90 + offset, 0x2B],
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                if (this.mode.range === PitchPlayRange.UP) {
                    this.mode.range = PitchPlayRange.MID;
                } else if (this.mode.range === PitchPlayRange.MID) {
                    this.mode.range = PitchPlayRange.DOWN;
                } else {
                    this.mode.range = PitchPlayRange.UP;
                }
                this.mode.forEachComponent(function(component) {
                    component.trigger();
                });
            }
            this.send(value);
        },
    });
    this.paramPlusButton = new components.Button({
        midi: [0x90 + offset, 0x33],
        mode: this,
        input: function(channel, control, value, _status, _group) {
            if (value) {
                if (this.mode.range === PitchPlayRange.UP) {
                    this.mode.range = PitchPlayRange.DOWN;
                } else if (this.mode.range === PitchPlayRange.MID) {
                    this.mode.range = PitchPlayRange.UP;
                } else {
                    this.mode.range = PitchPlayRange.MID;
                }
                this.mode.forEachComponent(function(component) {
                    component.trigger();
                });
            }
            this.send(value);
        },
    });
};
DDJSR2.PitchPlayMode.prototype = Object.create(components.ComponentContainer.prototype);

const WheelRing = function(options) { components.Component.call(this, options); };

WheelRing.prototype = new components.Component({
    blinkStatus: 0,
    outKey: "playposition",
    output: function(value) {
        if (this.color === "POSITION") {
            // Every time the playposition  changes, update the wheel color.
        // Timing calculation is handled in seconds!i
            const duration = engine.getValue(this.group, "duration");
            const elapsedTime = value * duration;
            const remainingTime = duration - elapsedTime;
            const revolutionsPerSecond = DDJSR2.scratchSettings.vinylSpeed / 60;
            const speed = parseInt(revolutionsPerSecond * DDJSR2.wheelLedCircle.maxVal);
            let wheelPos = DDJSR2.wheelLedCircle.minVal;

            if (value >= 0) {
                wheelPos = DDJSR2.wheelLedCircle.minVal + 0x01 + ((speed * elapsedTime) % DDJSR2.wheelLedCircle.maxVal);
            } else {
                wheelPos = DDJSR2.wheelLedCircle.maxVal + 0x01 + ((speed * elapsedTime) % DDJSR2.wheelLedCircle.maxVal);
            }
            // let wheel LEDs blink if remaining time is less than 30s:
            if (remainingTime > 0 && remainingTime < 30 && !engine.isScratching(this.deckNumber)) {
                let blinkInterval = parseInt(remainingTime / 3); //increase blinking according time left
                if (blinkInterval < 3) {
                    blinkInterval = 3;
                }
                wheelPos = DDJSR2.wheelLedCircle.maxVal;
                if (this.blinkStatus < blinkInterval) {
                    wheelPos = DDJSR2.wheelLedCircle.blink;
                } else if (this.blinkStatus > (blinkInterval - parseInt(6 / blinkInterval))) {
                    this.blinkStatus = 0;
                }
                this.blinkStatus++;
            }
            if (this.wheelPos === wheelPos) {
                // Do nothing.
            } else {
                this.wheelPos = wheelPos;
                DDJSR2.wheelLedControl(this.channelOffset, Math.round(wheelPos));
            }
        } else if (this.color === "TRACK") {
            // TO do once colormapper is understood.
        } else {
            const colorName = this.color;
            const colorCode = DDJSR2.wheelLedCircleColor[colorName];
            if (this.colorCode === colorCode) {
                // Do nothing
            } else {
                this.colorCode = colorCode;
                DDJSR2.wheelLedControl(this.channelOffset, colorCode);
            }
        }
    }
});
