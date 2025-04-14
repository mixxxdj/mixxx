////////////////////////////////////////////////////////////////////////
// Controller: Reloop Digital Jockey 2
// URL:        https://mixxx.discourse.group/t/reloop-digital-jockey-2-mapping-by-dj-ak/23971
// Author:     DJ aK
// Credits:    Uwe Klotz a/k/a tapir (baseline: Denon MC6000MK2 script)
////////////////////////////////////////////////////////////////////////

var RDJ2 = {};


////////////////////////////////////////////////////////////////////////
// Tunable constants                                                  //
////////////////////////////////////////////////////////////////////////

RDJ2.JOG_SPIN_CUE_PEAK = 0.2; // [0.0, 1.0]
RDJ2.JOG_SPIN_CUE_EXPONENT = 0.7; // 1.0 = linear response

RDJ2.JOG_SPIN_PLAY_PEAK = 0.3; // [0.0, 1.0]
RDJ2.JOG_SPIN_PLAY_EXPONENT = 0.7; // 1.0 = linear response

RDJ2.JOG_SCRATCH_RPM = 33.333333; // 33 1/3
RDJ2.JOG_SCRATCH_ALPHA = 0.125; // 1/8
RDJ2.JOG_SCRATCH_BETA = RDJ2.JOG_SCRATCH_ALPHA / 32.0;
RDJ2.JOG_SCRATCH_RAMP = true; // required for back spins

// Seeking: Number of revolutions needed to seek from the beginning
// to the end of the track.
RDJ2.JOG_SEEK_REVOLUTIONS = 2;


////////////////////////////////////////////////////////////////////////
// Fixed constants                                                    //
////////////////////////////////////////////////////////////////////////

// Controller constants
RDJ2.DECK_COUNT = 2;
RDJ2.JOG_RESOLUTION = 148; // measured/estimated
RDJ2.SHIFT_OFFSET = 0x1E;


// Jog constants
RDJ2.MIDI_JOG_DELTA_BIAS = 0x40; // center value of relative movements
RDJ2.MIDI_JOG_DELTA_RANGE = 0x3F; // both forward (= positive) and reverse (= negative)


// Mixxx constants
RDJ2.MIXXX_JOG_RANGE = 3.0;
RDJ2.MIXXX_LOOP_POSITION_UNDEFINED = -1;


////////////////////////////////////////////////////////////////////////
// Button/Knob map                                                    //
////////////////////////////////////////////////////////////////////////

/* This map is necessary as Reloop has designed the controller in such
   a way that not all buttons/knobs have the same offset comparing
   CH0 and CH1. By looking at the MIDI messages sent by the controller,
   we can see that the hardware is designed as symmetric halves.

   In other words, constant offset in hardware corresponds to symmetric
   halves, but the controller layout is not fully symmetric.
   (e.x. ACTIVATE 1 buttons)

   Thus we need a map to preserve object oriented approach .

   The below map is only for the unshifted controls as shifted ones
   have the same handlers mapped in the xml file and the outputs
   always refer to the unshifted controls. */
RDJ2.BUTTONMAP_CH0_CH1 = {
    load: [0x13, 0x4F],
    play: [0x19, 0x55],
    cue: [0x18, 0x54],
    sync: [0x01, 0x3D],
    search: [0x1A, 0x56],
    scratch: [0x1B, 0x57],
    fxdrywet: [0x1C, 0x58],
    bendminus: [0x03, 0x3F],
    bendplus: [0x04, 0x40],
    loopin: [0x0F, 0x4B],
    loopout: [0x10, 0x4C],
    autoloop: [0x11, 0x4D],
    loopactive: [0x12, 0x4E],
    fx1assign: [0x27, 0x68],    //this is the shifted Activate 3 button
    fx2assign: [0x2C, 0x63],    //this is the shifted FX ON button
    highkill: [0x14, 0x50],
};

RDJ2.KNOBMAP_CH0_CH1 = {
    loopSize: [0x28, 0x63],     //this is the shifted Dry/Wet Knob
};


////////////////////////////////////////////////////////////////////////
// Logging functions                                                  //
////////////////////////////////////////////////////////////////////////

RDJ2.logDebug = function(msg) {
    if (RDJ2.debug) {
        print("[" + RDJ2.id + " DEBUG] " + msg);
    }
};

RDJ2.logInfo = function(msg) {
    print("[" + RDJ2.id + " INFO] " + msg);
};

RDJ2.logWarning = function(msg) {
    print("[" + RDJ2.id + " WARNING] " + msg);
};

RDJ2.logError = function(msg) {
    print("[" + RDJ2.id + " ERROR] " + msg);
};


////////////////////////////////////////////////////////////////////////
// Buttons                                                            //
////////////////////////////////////////////////////////////////////////

RDJ2.MIDI_ON = 0x7F;
RDJ2.MIDI_OFF = 0x00;

RDJ2.isButtonPressed = function(midiValue) {
    switch (midiValue) {
    case RDJ2.MIDI_ON:
        return true;
    case RDJ2.MIDI_OFF:
        return false;
    default:
        RDJ2.logError("Unexpected MIDI button value: " + midiValue);
        return undefined;
    }
};

/* Custom buttons */

RDJ2.ShiftButton = function(options) {
    this.state = false;
    this.connectedContainers = [];
    components.Button.call(this, options);
};
RDJ2.ShiftButton.prototype = new components.Button({
    input: function(channel, control, value) {
        //update shift state
        this.state = RDJ2.isButtonPressed(value);
        this.send(this.outValueScale(this.state));

        //call shift()/unshift() for each connected container
        if (this.state) {
            this.connectedContainers.forEach(function(container) {
                container.shift();
            });
        } else {
            this.connectedContainers.forEach(function(container) {
                container.unshift();
            });
        }
    },
    isActive: function() {
        return this.state;
    },
    connectContainer: function(container) {
        if (container instanceof components.ComponentContainer === false) {
            RDJ2.logError("Container type mismatch");
        } else {
            this.connectedContainers.push(container);
            RDJ2.logDebug("Connected container " + this.connectedContainers.indexOf(container) + " to shift button 0x" + this.midi[1].toString(16));
        }
    }
});

RDJ2.LoopInButton = function(options) {
    components.Button.call(this, options);
};
RDJ2.LoopInButton.prototype = new components.Button({
    outKey: "loop_start_position",
    outValueScale: function(value) { return value >= 0 ? this.on : this.off; },
    unshift: function() {
        this.inKey = "loop_in";
        this.input = components.Button.prototype.input;
    },
    shift: function() {
        //pressing when shifted will delete loop start marker
        this.inKey = "loop_start_position";
        this.input = function(channel, control, value, status) {
            if (this.isPress(channel, control, value, status)) {
                this.inSetValue(RDJ2.MIXXX_LOOP_POSITION_UNDEFINED);
            }
        };
    },
});

RDJ2.LoopOutButton = function(options) {
    components.Button.call(this, options);
};
RDJ2.LoopOutButton.prototype = new components.Button({
    outKey: "loop_end_position",
    outValueScale: function(value) { return value >= 0 ? this.on : this.off; },
    unshift: function() {
        this.inKey = "loop_out";
        this.input = components.Button.prototype.input;
    },
    shift: function() {
        //pressing when shifted will delete loop end marker
        this.inKey = "loop_end_position";
        this.input = function(channel, control, value, status) {
            if (this.isPress(channel, control, value, status)) {
                this.inSetValue(RDJ2.MIXXX_LOOP_POSITION_UNDEFINED);
            }
        };
    },
});

RDJ2.AutoLoopButton = function(options) {
    components.Button.call(this, options);
};
RDJ2.AutoLoopButton.prototype = new components.Button({
    outKey: "beatloop_activate",
    unshift: function() {
        this.inKey = "beatloop_activate";
    },
    shift: function() {
        this.inKey = "beatlooproll_activate";
    },
});

RDJ2.LoopActiveButton = function(options) {
    components.Button.call(this, options);
};
RDJ2.LoopActiveButton.prototype = new components.Button({
    outKey: "loop_enabled",
    unshift: function() {
        this.inKey = "reloop_toggle";
    },
    shift: function() {
        this.inKey = "reloop_andstop";
    },
});


////////////////////////////////////////////////////////////////////////
// Knobs                                                              //
////////////////////////////////////////////////////////////////////////

RDJ2.MIDI_KNOB_INC = 0x41;
RDJ2.MIDI_KNOB_DEC = 0x3F;
RDJ2.MIDI_KNOB_DELTA_BIAS = 0x40; // center value of relative movements
//RDJ2.MIDI_KNOB_STEPS = 20;  // 20 is full knob's rotation (360deg)
RDJ2.MIDI_KNOB_STEPS = 16;    // 16 is more like volume knobs

RDJ2.getKnobDelta = function(midiValue) {
    return midiValue - RDJ2.MIDI_KNOB_DELTA_BIAS;
};

RDJ2.knobInput = function(channel, control, value) {
    var knobDelta = RDJ2.getKnobDelta(value);
    this.inSetParameter(this.inGetParameter() + knobDelta / RDJ2.MIDI_KNOB_STEPS);
};

/* Custom knobs */
RDJ2.LoopSizeKnob = function(options) {
    components.Pot.call(this, options);
};
RDJ2.LoopSizeKnob.prototype = new components.Pot({
    input: function(channel, control, value) {
        var knobDelta = RDJ2.getKnobDelta(value);

        if (knobDelta > 0) {
            engine.setValue(this.group, "loop_double", true);
        } else {
            engine.setValue(this.group, "loop_halve", true);
        }
    }
});


////////////////////////////////////////////////////////////////////////
// Decks                                                              //
////////////////////////////////////////////////////////////////////////

RDJ2.JOGMODES = {
    normal: 0,
    vinyl: 1,
    search: 2,
    fxdrywet: 3,
    trax: 4,
};

RDJ2.getJogDeltaValue = function(value) {
    if (value === 0x00) {
        return 0x00;
    } else {
        return value - RDJ2.MIDI_JOG_DELTA_BIAS;
    }
};

/* Constructor */

RDJ2.Deck = function(number) {
    RDJ2.logDebug("Creating Deck " + number);

    this.number = number;
    this.group = "[Channel" + number + "]";
    this.filterGroup = "[QuickEffectRack1_" + this.group + "_Effect1]";
    this.rateDirBackup = this.getValue("rate_dir");
    this.setValue("rate_dir", -1);
    this.jogTouchState = false;

    components.Deck.call(this, number);

    //primary buttons
    this.loadButton = new RDJ2.LoadButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.load[number - 1]]);
    this.playButton = new components.PlayButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.play[number - 1]]);
    this.cueButton = new components.CueButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.cue[number - 1]]);
    this.syncButton = new components.SyncButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.sync[number - 1]]);
    this.bendMinusButton = new RDJ2.BendMinusButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.bendminus[number - 1]]);
    this.bendPlusButton = new RDJ2.BendPlusButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.bendplus[number - 1]]);
    this.jogModeSelector = new RDJ2.JogModeSelector(number,
        RDJ2.BUTTONMAP_CH0_CH1.search[number - 1],
        RDJ2.BUTTONMAP_CH0_CH1.scratch[number - 1],
        RDJ2.BUTTONMAP_CH0_CH1.fxdrywet[number - 1]);

    //loops
    this.loopsizeKnob = new RDJ2.LoopSizeKnob([0xB0, RDJ2.KNOBMAP_CH0_CH1.loopSize[number - 1]]);
    this.loopInButton = new RDJ2.LoopInButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.loopin[number - 1]]);
    this.loopOutButton = new RDJ2.LoopOutButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.loopout[number - 1]]);
    this.autoLoopButton = new RDJ2.AutoLoopButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.autoloop[number - 1]]);
    this.loopActiveButton = new RDJ2.LoopActiveButton([0x90, RDJ2.BUTTONMAP_CH0_CH1.loopactive[number - 1]]);

    //effect assignment switches
    this.fx1AssignmentButton = new components.EffectAssignmentButton({
        midi: [0x90, RDJ2.BUTTONMAP_CH0_CH1.fx1assign[number - 1]],
        effectUnit: 1,
        group: "[Channel" + number + "]",
    });
    this.fx2AssignmentButton = new components.EffectAssignmentButton({
        midi: [0x90, RDJ2.BUTTONMAP_CH0_CH1.fx2assign[number - 1]],
        effectUnit: 2,
        group: "[Channel" + number + "]",
    });

    // high kill / quick effect enable button
    this.highKillQuickEffectButton = new RDJ2.HighKillQuickEffectButton({
        midi: [0x90, RDJ2.BUTTONMAP_CH0_CH1.highkill[number - 1]],
        channelNr: number,
    });


    // Set the group properties of the above Components and connect their output callback functions
    // Without this, the group property for each Component would have to be specified to its
    // constructor.
    this.reconnectComponents(function(component) {
        if (component.group === undefined) {
            // 'this' inside a function passed to reconnectComponents refers to the ComponentContainer
            // so 'this' refers to the custom Deck object being constructed
            component.group = this.currentDeck;
        }
    });
};

// give our custom Deck all the methods of the generic Deck in the Components library
RDJ2.Deck.prototype = Object.create(components.Deck.prototype);

/* get/set Values */

RDJ2.Deck.prototype.getValue = function(key) {
    return engine.getValue(this.group, key);
};

RDJ2.Deck.prototype.setValue = function(key, value) {
    engine.setValue(this.group, key, value);
};

/* Load Track */

RDJ2.LoadButton = function(options) {
    components.Button.call(this, options);
};
RDJ2.LoadButton.prototype = new components.Button({
    outKey: "track_loaded",
    unshift: function() {
        this.inKey = "LoadSelectedTrack";
    },
    shift: function() {
        this.inKey = "eject";
    },
});

/* Cue & Play */

RDJ2.Deck.prototype.isPlaying = function() {
    return this.getValue("play");
};

/* Pitch Bend / Track Search */

RDJ2.BendMinusButton = function(options) {
    components.Button.call(this, options);
};
RDJ2.BendMinusButton.prototype = new components.Button({
    key: "rate_temp_down",
    input: function(channel, control, value, status) {
        var isPlaying = engine.getValue(this.group, "play");
        if (isPlaying) {
            engine.setValue(this.group, "back", false);
            this.inSetValue(this.isPress(channel, control, value, status));
        } else {
            engine.setValue(this.group, "back", this.isPress(channel, control, value, status));
        }
    },
});

RDJ2.BendPlusButton = function(options) {
    components.Button.call(this, options);
};
RDJ2.BendPlusButton.prototype = new components.Button({
    key: "rate_temp_up",
    input: function(channel, control, value, status) {
        var isPlaying = engine.getValue(this.group, "play");
        if (isPlaying) {
            engine.setValue(this.group, "fwd", false);
            this.inSetValue(this.isPress(channel, control, value, status));
        } else {
            engine.setValue(this.group, "fwd", this.isPress(channel, control, value, status));
        }
    },
});

/* Jog Mode */

RDJ2.JogModeSelector = function(number, searchMidiCtrl, scratchMidiCtrl, fxDryWetMidiCtrl) {
    this.number = number;
    this.searchMidiCtrl = searchMidiCtrl;
    this.scratchMidiCtrl = scratchMidiCtrl;
    this.fxDryWetMidiCtrl = fxDryWetMidiCtrl;
    this.jogMode = RDJ2.JOGMODES.normal;
    this.lastNonTraxJogMode = this.jogMode;
    this.input = this.inputNormal;

    components.Component.call(this);
    this.updateControls();
};
RDJ2.JogModeSelector.prototype = new components.Component({
    updateControls: function() {
        var searchValue = this.jogMode === RDJ2.JOGMODES.search ? RDJ2.MIDI_ON : RDJ2.MIDI_OFF;
        var scratchValue = this.jogMode === RDJ2.JOGMODES.vinyl ? RDJ2.MIDI_ON : RDJ2.MIDI_OFF;
        var fxDryWetValue = this.jogMode === RDJ2.JOGMODES.fxdrywet ? RDJ2.MIDI_ON : RDJ2.MIDI_OFF;

        if (midi.sendShortMsg && this.searchMidiCtrl !== undefined && this.scratchMidiCtrl !== undefined && this.fxDryWetMidiCtrl !== undefined) {
            midi.sendShortMsg(0x90, this.searchMidiCtrl, searchValue);
            midi.sendShortMsg(0x90, this.scratchMidiCtrl, scratchValue);
            midi.sendShortMsg(0x90, this.fxDryWetMidiCtrl, fxDryWetValue);
        }
    },
    inputNormal: function(channel, control, value) {
        var isButtonPressed = RDJ2.isButtonPressed(value);
        if (isButtonPressed) {
            switch (control) {
            case this.searchMidiCtrl:
                this.jogMode = this.jogMode === RDJ2.JOGMODES.search ? RDJ2.JOGMODES.normal : RDJ2.JOGMODES.search;
                break;
            case this.scratchMidiCtrl:
                this.jogMode = this.jogMode === RDJ2.JOGMODES.vinyl ? RDJ2.JOGMODES.normal : RDJ2.JOGMODES.vinyl;
                break;
            case this.fxDryWetMidiCtrl:
                this.jogMode = this.jogMode === RDJ2.JOGMODES.fxdrywet ? RDJ2.JOGMODES.normal : RDJ2.JOGMODES.fxdrywet;
                break;
            default:
                RDJ2.logError("Unexpected MIDI ctrl value: " + control);
            }
            if (this.jogMode !== RDJ2.JOGMODES.vinyl && engine.isScratching(this.number)) {
                engine.scratchDisable(this.number, RDJ2.JOG_SCRATCH_RAMP);
            }
            this.updateControls();
        }
    },
    inputTrax: function(channel, control, value) {
        var isButtonPressed = RDJ2.isButtonPressed(value);
        if (isButtonPressed) {
            switch (control) {
            case this.searchMidiCtrl:
            case this.searchMidiCtrl + RDJ2.SHIFT_OFFSET:
                engine.setValue("[Library]", "MoveRight", 1);
                break;
            case this.scratchMidiCtrl:
            case this.scratchMidiCtrl + RDJ2.SHIFT_OFFSET:
                engine.setValue("[Library]", "MoveLeft", 1);
                break;
            case this.fxDryWetMidiCtrl:
            case this.fxDryWetMidiCtrl + RDJ2.SHIFT_OFFSET:
                //'MoveFocusForward' is equivalent to pressing TAB key on the keyboard
                engine.setValue("[Library]", "MoveFocusForward", 1);
                break;
                /* The below code will probably be removed.
                   It allowed absolute referencing to buttons to enable
                   moving focus backward/forward. It seems that using only
                   'MoveFocusForward' is enough.

                case RDJ2.BUTTONMAP_CH0_CH1.fxdrywet[0]:
                case RDJ2.BUTTONMAP_CH0_CH1.fxdrywet[0] + RDJ2.SHIFT_OFFSET:
                    engine.setValue('[Library]', 'MoveFocusBackward', 1);
                    break;
                case RDJ2.BUTTONMAP_CH0_CH1.fxdrywet[1]:
                case RDJ2.BUTTONMAP_CH0_CH1.fxdrywet[1] + RDJ2.SHIFT_OFFSET:
                    engine.setValue('[Library]', 'MoveFocusForward', 1);
                    break;*/
            default:
                RDJ2.logError("Unexpected MIDI ctrl value: " + control);
            }
        }
    },
    setTraxMode: function(isTraxModeEnabled) {
        if (isTraxModeEnabled) {
            if (this.jogMode !== RDJ2.JOGMODES.trax) {
                this.lastNonTraxJogMode = this.jogMode;
                this.jogMode = RDJ2.JOGMODES.trax;
                this.input = this.inputTrax;
                //set all LEDs on to indicate trax mode
                if (midi.sendShortMsg) {
                    midi.sendShortMsg(0x90, this.searchMidiCtrl, RDJ2.MIDI_ON);
                    midi.sendShortMsg(0x90, this.scratchMidiCtrl, RDJ2.MIDI_ON);
                    midi.sendShortMsg(0x90, this.fxDryWetMidiCtrl, RDJ2.MIDI_ON);
                }
            }
        } else {
            this.jogMode = this.lastNonTraxJogMode;
            this.input = this.inputNormal;
            this.updateControls();
        }
    },
    unshift: function() {
        var isLibraryModeEnabled = engine.getValue("[Master]", "maximize_library");
        if (!isLibraryModeEnabled) {
            this.setTraxMode(false);
        }
    },
    shift: function() {
        var isLibraryModeEnabled = engine.getValue("[Master]", "maximize_library");
        if (!isLibraryModeEnabled) {
            this.setTraxMode(true);
        }
    },
});

/* Jog Wheel */

RDJ2.Deck.prototype.onJogTouch = function(channel, control, value) {
    var currentJogMode =  this.jogModeSelector.jogMode;
    this.jogTouchState = RDJ2.isButtonPressed(value);

    if (currentJogMode === RDJ2.JOGMODES.vinyl && this.jogTouchState) {
        engine.scratchEnable(this.number,
            RDJ2.JOG_RESOLUTION,
            RDJ2.JOG_SCRATCH_RPM,
            RDJ2.JOG_SCRATCH_ALPHA,
            RDJ2.JOG_SCRATCH_BETA,
            RDJ2.JOG_SCRATCH_RAMP);
    } else if (!this.jogTouchState && engine.isScratching(this.number)) {
        engine.scratchDisable(this.number, RDJ2.JOG_SCRATCH_RAMP);
    }
};

RDJ2.Deck.prototype.onJogSpin = function(channel, control, value) {
    var currentJogMode =  this.jogModeSelector.jogMode;
    var jogDelta = RDJ2.getJogDeltaValue(value);

    if (currentJogMode === RDJ2.JOGMODES.vinyl) {
        engine.scratchTick(this.number, jogDelta);
    } else if (currentJogMode === RDJ2.JOGMODES.fxdrywet) {
        var currMixValue = engine.getParameter("[EffectRack1_EffectUnit" + this.number + "]", "mix");
        engine.setParameter("[EffectRack1_EffectUnit" + this.number + "]", "mix", currMixValue + jogDelta / RDJ2.JOG_RESOLUTION);
    } else if (currentJogMode === RDJ2.JOGMODES.search) {
        var playPos = engine.getValue(this.group, "playposition");
        if (undefined !== playPos) {
            var seekPos = playPos + (jogDelta / (RDJ2.JOG_RESOLUTION * RDJ2.JOG_SEEK_REVOLUTIONS));
            this.setValue("playposition", Math.max(0.0, Math.min(1.0, seekPos)));
        }
    } else if (currentJogMode === RDJ2.JOGMODES.trax) {
        engine.setValue("[Library]", "MoveVertical", jogDelta);
    } else if (currentJogMode === RDJ2.JOGMODES.normal) {
        var normalizedDelta = jogDelta / RDJ2.MIDI_JOG_DELTA_RANGE;
        var scaledDelta;
        var jogExponent;
        if (this.isPlaying()) {
            // bending
            scaledDelta = normalizedDelta / RDJ2.JOG_SPIN_PLAY_PEAK;
            jogExponent = RDJ2.JOG_SPIN_PLAY_EXPONENT;
        } else {
            // cueing
            scaledDelta = normalizedDelta / RDJ2.JOG_SPIN_CUE_PEAK;
            jogExponent = RDJ2.JOG_SPIN_CUE_EXPONENT;
        }
        var direction;
        var scaledDeltaAbs;
        if (scaledDelta < 0.0) {
            direction = -1.0;
            scaledDeltaAbs = -scaledDelta;
        } else {
            direction = 1.0;
            scaledDeltaAbs = scaledDelta;
        }
        var scaledDeltaPow = direction * Math.pow(scaledDeltaAbs, jogExponent);
        var jogValue = RDJ2.MIXXX_JOG_RANGE * scaledDeltaPow;
        this.setValue("jog", jogValue);
    } else {
        RDJ2.logError("onJogSpin unknown mode error!");
    }
};


////////////////////////////////////////////////////////////////////////
// Effects                                                            //
////////////////////////////////////////////////////////////////////////

//functions for overriding default unshift/shift functions of efx unit knobs
RDJ2.efxUnitKnobUnshift = function() {
    this.input = function(channel, control, value) {
        var knobDelta = RDJ2.getKnobDelta(value);
        this.inSetParameter(this.inGetParameter() + knobDelta / RDJ2.MIDI_KNOB_STEPS);
    };
};
RDJ2.efxUnitKnobShift = function() {
    this.input = function(channel, control, value) {
        var knobDelta = RDJ2.getKnobDelta(value);
        var effectGroup = "[EffectRack1_EffectUnit" +
                            this.eu.currentUnitNumber + "_Effect" +
                            this.number + "]";
        engine.setValue(effectGroup, "effect_selector", knobDelta);
    };
};

//note: the shifted dry/wet knob is mapped to beatloop size


////////////////////////////////////////////////////////////////////////
// Quick Effects                                                      //
////////////////////////////////////////////////////////////////////////

/* HIGH kill / QuickEffect enable button */

RDJ2.HighKillQuickEffectButton = function(options) {
    this.channelNr = options.channelNr;
    components.Button.call(this, options);
};
RDJ2.HighKillQuickEffectButton.prototype = new components.Button({
    type: components.Button.prototype.types.powerWindow,
    unshift: function() {
        this.disconnect();
        this.group = "[EqualizerRack1_[Channel" + this.channelNr + "]_Effect1]";
        this.inKey = "button_parameter3";
        this.outKey = "button_parameter3";
        this.connect();
        this.trigger();
    },
    shift: function() {
        this.disconnect();
        this.group = "[QuickEffectRack1_[Channel" + this.channelNr + "]_Effect1]";
        this.inKey = "enabled";
        this.outKey = "enabled";
        this.connect();
        this.trigger();
    },
});


////////////////////////////////////////////////////////////////////////
// Library                                                            //
////////////////////////////////////////////////////////////////////////

/* Trax knob */

RDJ2.TraxKnob = function(options) {
    components.Encoder.call(this, options);
};
RDJ2.TraxKnob.prototype = new components.Encoder({
    group: "[Library]",
    unshift: function() {
        this.inKey = "MoveVertical";
    },
    shift: function() {
        this.inKey = "ScrollVertical";
    },
    input: function(channel, control, value) {
        var knobDelta = RDJ2.getKnobDelta(value);
        this.inSetValue(knobDelta);
    }
});

/* Trax button */

RDJ2.TraxButton = function(obj) {
    this.detectedDecks = [];
    /* group and/or outKey cannot be defined at prototype initialization
       (inside anonymous object passed to components.Button constructor
       below) as this would cause additional premature engine.makeConnection
       call that binds prototype object's output callback to the outKey.
       This would cause uncaught exception when invoking the callback:

       "TypeError: Result of expression 'this.detectedDecks' [undefined] is not an object"

       which happens because at the time we create the prototype object,
       we have no information yet about detectedDecks, as detectedDecks is
       created at new TraxButton object construction, when its prototype is
       long time existing. */
    this.group = "[Master]";
    this.outKey = "maximize_library";

    this.detectDecks(obj);
    components.Button.call(this);
};
RDJ2.TraxButton.prototype = new components.Button({
    unshift: function() {
        this.type = components.Button.prototype.types.toggle;
        this.group = "[Master]";
        this.inKey = "maximize_library";
    },
    shift: function() {
        this.type = components.Button.prototype.types.push;
        this.group = "[Library]";
        this.inKey = "MoveFocusForward";
    },
    output: function(value) {
        this.updateTraxMode(value);
    },
    updateTraxMode: function(mode) {
        this.detectedDecks.forEach(function(deck) {
            deck.jogModeSelector.setTraxMode(mode);
        });
    },
    detectDecks: function(obj) {
        // find decks in the passed object and store them in the array
        for (var memberName in obj) {
            if (Object.prototype.hasOwnProperty.call(obj, memberName) && obj[memberName] instanceof components.Deck) {
                RDJ2.logDebug("Detected " + memberName);
                this.detectedDecks.push(obj[memberName]);
            }
        }
    },
});

/* Trax container */

RDJ2.Trax = function(obj) {
    this.traxKnob = new RDJ2.TraxKnob();
    this.traxButton = new RDJ2.TraxButton(obj);
};
RDJ2.Trax.prototype = new components.ComponentContainer();


////////////////////////////////////////////////////////////////////////
// Mixxx Callback Functions                                           //
////////////////////////////////////////////////////////////////////////

RDJ2.init = function(id, debug) {
    RDJ2.id = id;
    RDJ2.debug = debug;

    RDJ2.logInfo("Initializing controller");

    // left and right shift button
    RDJ2.leftShiftButton = new RDJ2.ShiftButton([0x90, 0x0A]);
    RDJ2.rightShiftButton = new RDJ2.ShiftButton([0x90, 0x46]);

    // left and right deck
    RDJ2.leftDeck = new RDJ2.Deck(1);
    RDJ2.rightDeck = new RDJ2.Deck(2);

    // effect unit 1
    RDJ2.fx1 = new components.EffectUnit(1);
    RDJ2.fx1.EffectUnitKnob.prototype.unshift = RDJ2.efxUnitKnobUnshift;
    RDJ2.fx1.EffectUnitKnob.prototype.shift = RDJ2.efxUnitKnobShift;
    RDJ2.fx1.EffectUnitKnob.prototype.eu = RDJ2.fx1;    // hack for use by reimplemented unshift/shift
    RDJ2.fx1.enableButtons[1].midi = [0x90, 0x07];
    RDJ2.fx1.enableButtons[2].midi = [0x90, 0x0C];
    RDJ2.fx1.enableButtons[3].midi = [0x90, 0x09];
    RDJ2.fx1.knobs[1].midi = [0xB0, 0x07];
    RDJ2.fx1.knobs[2].midi = [0xB0, 0x08];
    RDJ2.fx1.knobs[3].midi = [0xB0, 0x09];
    RDJ2.fx1.dryWetKnob.midi = [0xB0, 0x0A];
    RDJ2.fx1.dryWetKnob.input = RDJ2.knobInput;
    RDJ2.fx1.effectFocusButton.midi = [0x90, 0x0E];
    // We need to call unshift() again for each EffectUnitKnob as we
    // swapped its implementation after fx object construction (when
    // it is called automatically)
    for (var n = 1; n <= 3; n++) {
        RDJ2.fx1.knobs[n].unshift();
    }
    // Now init the fx unit
    RDJ2.fx1.init();

    // effect unit 2
    RDJ2.fx2 = new components.EffectUnit(2);
    RDJ2.fx2.EffectUnitKnob.prototype.unshift = RDJ2.efxUnitKnobUnshift;
    RDJ2.fx2.EffectUnitKnob.prototype.shift = RDJ2.efxUnitKnobShift;
    RDJ2.fx2.EffectUnitKnob.prototype.eu = RDJ2.fx2;    // hack for use by reimplemented unshift/shift
    RDJ2.fx2.enableButtons[1].midi = [0x90, 0x48];
    RDJ2.fx2.enableButtons[2].midi = [0x90, 0x43];
    RDJ2.fx2.enableButtons[3].midi = [0x90, 0x4A];
    RDJ2.fx2.knobs[1].midi = [0xB0, 0x44];
    RDJ2.fx2.knobs[2].midi = [0xB0, 0x43];
    RDJ2.fx2.knobs[3].midi = [0xB0, 0x46];
    RDJ2.fx2.dryWetKnob.midi = [0xB0, 0x45];
    RDJ2.fx2.dryWetKnob.input = RDJ2.knobInput;
    RDJ2.fx2.effectFocusButton.midi = [0x90, 0x45];
    // We need to call unshift() again for each EffectUnitKnob as we
    // swapped its implementation after fx object construction (when
    // it is called automatically)
    for (n = 1; n <= 3; n++) {
        RDJ2.fx2.knobs[n].unshift();
    }
    // Now init the fx unit
    RDJ2.fx2.init();

    // Trax/library
    RDJ2.trax = new RDJ2.Trax(RDJ2);

    // connect decks, efx units and trax to shift buttons
    RDJ2.leftShiftButton.connectContainer(RDJ2.leftDeck);
    RDJ2.leftShiftButton.connectContainer(RDJ2.fx1);
    RDJ2.leftShiftButton.connectContainer(RDJ2.trax);
    RDJ2.rightShiftButton.connectContainer(RDJ2.rightDeck);
    RDJ2.rightShiftButton.connectContainer(RDJ2.fx2);
    RDJ2.rightShiftButton.connectContainer(RDJ2.trax);
};

RDJ2.shutdown = function() {
    RDJ2.logInfo("Shutting down controller");
};
