////////////////////////////////////////////////////////////////////////
// Controller: Reloop Digital Jockey 2
// URL:        na razie nic
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
RDJ2.JOG_RESOLUTION = 600; // measured/estimated


// Jog constants
RDJ2.MIDI_JOG_DELTA_BIAS = 0x40; // center value of relative movements
RDJ2.MIDI_JOG_DELTA_RANGE = 0x3F; // both forward (= positive) and reverse (= negative)

// Mixxx constants
RDJ2.MIXXX_JOG_RANGE = 3.0;


////////////////////////////////////////////////////////////////////////
// Logging functions                                                  //
////////////////////////////////////////////////////////////////////////

RDJ2.logDebug = function (msg) {
    if (RDJ2.debug) {
        print("[" + RDJ2.id + " DEBUG] " + msg);
    }
};

RDJ2.logInfo = function (msg) {
    print("[" + RDJ2.id + " INFO] " + msg);
};

RDJ2.logWarning = function (msg) {
    print("[" + RDJ2.id + " WARNING] " + msg);
};

RDJ2.logError = function (msg) {
    print("[" + RDJ2.id + " ERROR] " + msg);
};


////////////////////////////////////////////////////////////////////////
// Buttons                                                            //
////////////////////////////////////////////////////////////////////////

RDJ2.MIDI_BUTTON_ON = 0x7F;
RDJ2.MIDI_BUTTON_OFF = 0x00;

RDJ2.isButtonPressed = function (midiValue) {
    switch (midiValue) {
        case RDJ2.MIDI_BUTTON_ON:
            return true;
        case RDJ2.MIDI_BUTTON_OFF:
            return false;
        default:
            RDJ2.logError("Unexpected MIDI button value: " + midiValue);
            return undefined;
    }
};


////////////////////////////////////////////////////////////////////////
// Controls                                                           //
////////////////////////////////////////////////////////////////////////

RDJ2.Control = function (group, ctrl, func) {
    this.group = group;
    this.ctrl = ctrl;
    this.func = func;
    this.isConnected = false;
};

RDJ2.Control.prototype.connect = function () {
    if (this.isConnected) {
        RDJ2.logWarning("Control is already connected: group=" + this.group + ", ctrl=" + this.ctrl + ", func=" + this.func);
        return true;
    }
    if (engine.connectControl(this.group, this.ctrl, this.func)) {
        this.isConnected = true;
        this.trigger();
    } else {
        RDJ2.logError("Failed to connect control: group=" + this.group + ", ctrl=" + this.ctrl + ", func=" + this.func);
    }
    return this.isConnected;
};

RDJ2.Control.prototype.disconnect = function () {
    if (this.isConnected) {
        if (engine.connectControl(this.group, this.ctrl, this.func, true)) {
            this.isConnected = false;
        } else {
            RDJ2.logError("Failed to disconnect control: group=" + this.group + ", ctrl=" + this.ctrl + ", func=" + this.func);
        }
    } else {
        RDJ2.logWarning("Control is not connected: group=" + this.group + ", ctrl=" + this.ctrl + ", func=" + this.func);
    }
};

RDJ2.Control.prototype.trigger = function () {
    engine.trigger(this.group, this.ctrl);
};

RDJ2.connectedControls = [];

RDJ2.connectControl = function (group, ctrl, func) {
    var control = new RDJ2.Control(group, ctrl, func);
    if (control.connect()) {
        RDJ2.connectedControls.push(control);
        return control;
    } else {
        return undefined;
    }
};

RDJ2.disconnectControls = function () {
    for (var index in RDJ2.connectedControls) {
        RDJ2.connectedControls[index].disconnect();
    }
    RDJ2.connectedControls = [];
};


////////////////////////////////////////////////////////////////////////
// Decks                                                              //
////////////////////////////////////////////////////////////////////////

/* Management */

RDJ2.decksByGroup = {};

RDJ2.getDeckByGroup = function (group) {
    var deck = RDJ2.decksByGroup[group];
    if (undefined === deck) {
        RDJ2.logError("No deck found for " + group);
    }
    return deck;
};

/* Constructor */

RDJ2.OldDeck = function (number) {
    RDJ2.logDebug("Creating OldDeck " + number);

    this.side = undefined;
    this.number = number;
    this.group = "[Channel" + number + "]";
    this.filterGroup = "[QuickEffectRack1_" + this.group + "_Effect1]";
    this.jogTouchState = false;
    RDJ2.decksByGroup[this.group] = this;
    this.rateDirBackup = this.getValue("rate_dir");
    this.setValue("rate_dir", -1);
    this.vinylMode = undefined;
    this.syncMode = undefined;
};

/* Values & Parameters */

RDJ2.OldDeck.prototype.getValue = function (key) {
    return engine.getValue(this.group, key);
};

RDJ2.OldDeck.prototype.setValue = function (key, value) {
    engine.setValue(this.group, key, value);
};

RDJ2.OldDeck.prototype.toggleValue = function (key) {
    this.setValue(key, !this.getValue(key));
};

RDJ2.OldDeck.prototype.setParameter = function (key, param) {
    engine.setParameter(this.group, key, param);
};

RDJ2.OldDeck.prototype.triggerValue = function (key) {
    engine.trigger(this.group, key);
};

/* Cue & Play */

RDJ2.OldDeck.prototype.isPlaying = function () {
    return this.getValue("play");
};

/* Pitch Bend / Track Search */

RDJ2.OldDeck.prototype.onBendPlusButton = function (isButtonPressed) {
    if (this.isPlaying()) {
        this.setValue("fwd", false);
        /*if (this.getShiftState()) {
            this.setValue("rate_temp_up_small", isButtonPressed);
        } else*/ {
            this.setValue("rate_temp_up", isButtonPressed);
        }
    } else {
        this.setValue("fwd", isButtonPressed);
    }
};

RDJ2.OldDeck.prototype.onBendMinusButton = function (isButtonPressed) {
    if (this.isPlaying()) {
        this.setValue("back", false);
        /*if (this.getShiftState()) {
            this.setValue("rate_temp_down_small", isButtonPressed);
        } else*/ {
            this.setValue("rate_temp_down", isButtonPressed);
        }
    } else {
        this.setValue("back", isButtonPressed);
    }
};

/* Vinyl Mode (Scratching) */

RDJ2.OldDeck.prototype.onVinylModeValue = function () {
    //this.vinylModeLed.setStateBoolean(this.vinylMode);
};

RDJ2.OldDeck.prototype.enableScratching = function () {
};

RDJ2.OldDeck.prototype.disableScratching = function () {
};

RDJ2.OldDeck.prototype.updateVinylMode = function () {
    if (this.vinylMode && this.jogTouchState) {
        engine.scratchEnable(this.number,
            RDJ2.JOG_RESOLUTION,
            RDJ2.JOG_SCRATCH_RPM,
            RDJ2.JOG_SCRATCH_ALPHA,
            RDJ2.JOG_SCRATCH_BETA,
            RDJ2.JOG_SCRATCH_RAMP);
    } else {
        engine.scratchDisable(this.number,
            RDJ2.JOG_SCRATCH_RAMP);
    }
    this.onVinylModeValue();
};

RDJ2.OldDeck.prototype.setVinylMode = function (vinylMode) {
    this.vinylMode = vinylMode;
    this.updateVinylMode();
};

RDJ2.OldDeck.prototype.toggleVinylMode = function () {
    this.setVinylMode(!this.vinylMode);
};

RDJ2.OldDeck.prototype.enableVinylMode = function () {
    this.setVinylMode(true);
};

RDJ2.OldDeck.prototype.disableVinylMode = function () {
    this.setVinylMode(false);
};

RDJ2.OldDeck.prototype.onVinylButton = function (isButtonPressed) {
    this.toggleVinylMode();
};

/* Jog Wheel */

RDJ2.OldDeck.prototype.touchJog = function (isJogTouched) {
    this.jogTouchState = isJogTouched;
    this.updateVinylMode();
};

RDJ2.OldDeck.prototype.spinJog = function (jogDelta) {
    if (/*this.getShiftState() &&*/ this.jogTouchState && !this.isPlaying()) {
        // fast track seek (strip search)
        var playPos = engine.getValue(this.group, "playposition");
        if (undefined !== playPos) {
            var seekPos = playPos + (jogDelta / (RDJ2.JOG_RESOLUTION * RDJ2.JOG_SEEK_REVOLUTIONS));
            this.setValue("playposition", Math.max(0.0, Math.min(1.0, seekPos)));
        }
    } else {
        if (engine.isScratching(this.number)) {
            engine.scratchTick(this.number, jogDelta);
        } else {
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
        }
    }
};


////////////////////////////////////////////////////////////////////////
// Sides                                                              //
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// Controller functions                                               //
////////////////////////////////////////////////////////////////////////

RDJ2.id = undefined;
RDJ2.debug = undefined;
RDJ2.group = "[Master]";

// left side
RDJ2.deck1 = new RDJ2.OldDeck(1);

// right side
RDJ2.deck2 = new RDJ2.OldDeck(2);

RDJ2.getValue = function (key) {
    return engine.getValue(RDJ2.group, key);
};

RDJ2.setValue = function (key, value) {
    engine.setValue(RDJ2.group, key, value);
};

RDJ2.getJogDeltaValue = function (value) {
    if (0x00 === value) {
        return 0x00;
    } else {
        return value - RDJ2.MIDI_JOG_DELTA_BIAS;
    }
};


////////////////////////////////////////////////////////////////////////
// MIDI callback functions without a group                            //
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// MIDI [Channel<n>] callback functions                               //
////////////////////////////////////////////////////////////////////////

RDJ2.recvBendPlusButton = function (channel, control, value, status, group) {
    var isButtonPressed = RDJ2.isButtonPressed(value);
    var deck = RDJ2.getDeckByGroup(group);
    deck.onBendPlusButton(isButtonPressed);
};

RDJ2.recvBendMinusButton = function (channel, control, value, status, group) {
    var isButtonPressed = RDJ2.isButtonPressed(value);
    var deck = RDJ2.getDeckByGroup(group);
    deck.onBendMinusButton(isButtonPressed);
};

RDJ2.recvJogTouch = function (channel, control, value, status, group) {
    var isJogTouched = RDJ2.isButtonPressed(value);
    var deck = RDJ2.getDeckByGroup(group);
    deck.touchJog(isJogTouched);
};

RDJ2.recvJogTouchVinyl = function (channel, control, value, status, group) {
    var isJogTouched = RDJ2.isButtonPressed(value);
    var deck = RDJ2.getDeckByGroup(group);
    deck.touchJog(isJogTouched);
};

RDJ2.recvJogSpin = function (channel, control, value, status, group) {
    var deck = RDJ2.getDeckByGroup(group);
    var jogDelta = RDJ2.getJogDeltaValue(value);
    deck.spinJog(jogDelta);
};

RDJ2.recvJogSpinVinyl = function (channel, control, value, status, group) {
    var deck = RDJ2.getDeckByGroup(group);
    var jogDelta = RDJ2.getJogDeltaValue(value);
    deck.spinJog(jogDelta);
};


////////////////////////////////////////////////////////////////////////
// Mixxx connected controls callback functions                        //
////////////////////////////////////////////////////////////////////////


// ...


RDJ2.Deck = function (number, channel) {
    RDJ2.logDebug("Creating deck: " + number);

    components.Deck.call(this, number);
    //var thisDeck = this;

    this.side = undefined;

    this.loadButton = new RDJ2.LoadButton();

    this.cueButton = new components.CueButton([0xB0 + channel, 0x26]);
    this.playButton = new components.PlayButton([0xB0 + channel, 0x27]);
    this.syncButton = new components.SyncButton([0xB0 + channel, 0x09]);

    this.hotcueButtons = [];
    for (var i = 1; i <= 4; i++) {
        this.hotcueButtons[i] = new components.HotcueButton({
            midi: [0xB0 + channel, 0x11 + 2 * (i - 1)],
            number: i,
        });
    }

    // Set the group properties of the above Components and connect their output callback functions
    // Without this, the group property for each Component would have to be specified to its
    // constructor.
    this.reconnectComponents(function (component) {
        if (component.group === undefined) {
            // 'this' inside a function passed to reconnectComponents refers to the ComponentContainer.
            component.group = this.currentDeck;
        }
    });
};

RDJ2.Deck.prototype = Object.create(components.Deck.prototype);


////////////////////////////////////////////////////////////////////////
// Mixxx Callback Functions                                           //
////////////////////////////////////////////////////////////////////////

RDJ2.init = function (id, debug) {
    RDJ2.id = id;
    RDJ2.debug = debug;
    
    RDJ2.logInfo("Initializing controller");

    // left side
    RDJ2.newDeck1 = new RDJ2.OldDeck(1);

    // right side
    RDJ2.newDeck2 = new RDJ2.OldDeck(2);

};

RDJ2.shutdown = function () {
    RDJ2.logInfo("Shutting down controller");
};