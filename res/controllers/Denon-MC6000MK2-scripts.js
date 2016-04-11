////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
// Controller: Denon MC6000MK2
// URL:        http://www.mixxx.org/wiki/doku.php/denon_mc6000mk2
// Author:     Uwe Klotz a/k/a tapir
// Revision:   2015-10-04
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//
// Some ideas for the future
// - Modify active loops with the jog wheel (move out point, move loop)
//
////////////////////////////////////////////////////////////////////////


function DenonMC6000MK2() {}


////////////////////////////////////////////////////////////////////////
// Tunable constants                                                  //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.SAMPLER_MODE = {
    HOLD: {
        value: 1,
        name: "Hold",
        description: "Play sample from beginning to end while pressing the Sample button. Playback continues when Shift is pressed while releasing the Sample button."
    },
    TRIGGER: {
        value: 2,
        name: "Trigger",
        description: "Play sample from beginning to end until stopped by pressing Shift + Sample button."
    }
};

DenonMC6000MK2.DEFAULT_SAMPLER_MODE = DenonMC6000MK2.SAMPLER_MODE.HOLD;

DenonMC6000MK2.JOG_SPIN_CUE_PEAK = 0.2; // [0.0, 1.0]
DenonMC6000MK2.JOG_SPIN_CUE_EXPONENT = 0.7; // 1.0 = linear response

DenonMC6000MK2.JOG_SPIN_PLAY_PEAK = 0.3; // [0.0, 1.0]
DenonMC6000MK2.JOG_SPIN_PLAY_EXPONENT = 0.7; // 1.0 = linear response

DenonMC6000MK2.JOG_SCRATCH_RPM = 33.333333; // 33 1/3
DenonMC6000MK2.JOG_SCRATCH_ALPHA = 0.125; // 1/8
DenonMC6000MK2.JOG_SCRATCH_BETA = DenonMC6000MK2.JOG_SCRATCH_ALPHA / 32.0;
DenonMC6000MK2.JOG_SCRATCH_RAMP = true;
DenonMC6000MK2.JOG_SCRATCH2_ABS_MIN = 0.01;
DenonMC6000MK2.JOG_SCRATCH2_PLAY_MIN = -0.7;
DenonMC6000MK2.JOG_SCRATCH2_PLAY_MAX = 1.0;
DenonMC6000MK2.JOG_SCRATCH_TIMEOUT = 20; // in milliseconds

// Seeking: Number of revolutions needed to seek from the beginning
// to the end of the track.
DenonMC6000MK2.JOG_SEEK_REVOLUTIONS = 2;

DenonMC6000MK2.GLOBAL_SHIFT_STATE = true;

// Echo loop feedback 90% = slow decay
DenonMC6000MK2.ECHO_LOOP_FEEDBACK = 0.9;

////////////////////////////////////////////////////////////////////////
// Fixed constants                                                    //
////////////////////////////////////////////////////////////////////////

// Controller constants
DenonMC6000MK2.BRAND = "Denon";
DenonMC6000MK2.MODEL = "MC6000MK2";
DenonMC6000MK2.SIDE_COUNT = 2;
DenonMC6000MK2.DECK_COUNT = 4;
DenonMC6000MK2.SAMPLER_COUNT_PER_SIDE = 4;
DenonMC6000MK2.SAMPLE_RATE = 44100;
DenonMC6000MK2.JOG_RESOLUTION = 600; // measured/estimated

DenonMC6000MK2.LEFT_EFX_UNIT_GROUP = "[EffectRack1_EffectUnit1]";
DenonMC6000MK2.LEFT_EFX_PARAM_GROUP = "[EffectRack1_EffectUnit1_Effect1]";
DenonMC6000MK2.RIGHT_EFX_UNIT_GROUP = "[EffectRack1_EffectUnit2]";
DenonMC6000MK2.RIGHT_EFX_PARAM_GROUP = "[EffectRack1_EffectUnit2_Effect1]";

// Jog constants
DenonMC6000MK2.MIDI_JOG_DELTA_BIAS = 0x40; // center value of relative movements
DenonMC6000MK2.MIDI_JOG_DELTA_RANGE = 0x3F; // both forward (= positive) and reverse (= negative)

// Mixxx constants
DenonMC6000MK2.MIXXX_JOG_RANGE = 3.0;
DenonMC6000MK2.MIXXX_SYNC_NONE = 0;
DenonMC6000MK2.MIXXX_SYNC_FOLLOWER = 1;
DenonMC6000MK2.MIXXX_SYNC_MASTER = 2;
DenonMC6000MK2.MIXXX_XFADER_LEFT = 0;
DenonMC6000MK2.MIXXX_XFADER_CENTER = 1;
DenonMC6000MK2.MIXXX_XFADER_RIGHT = 2;
DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED = -1;

// MIDI constants
DenonMC6000MK2.MIDI_CH0 = 0x00;
DenonMC6000MK2.MIDI_CH1 = 0x01;
DenonMC6000MK2.MIDI_CH2 = 0x02;
DenonMC6000MK2.MIDI_CH3 = 0x03;


////////////////////////////////////////////////////////////////////////
// Logging functions                                                  //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.logDebug = function(msg) {
    if (DenonMC6000MK2.debug) {
        print("[" + DenonMC6000MK2.id + " DEBUG] " + msg);
    }
};

DenonMC6000MK2.logInfo = function(msg) {
    print("[" + DenonMC6000MK2.id + " INFO] " + msg);
};

DenonMC6000MK2.logWarning = function(msg) {
    print("[" + DenonMC6000MK2.id + " WARNING] " + msg);
};

DenonMC6000MK2.logError = function(msg) {
    print("[" + DenonMC6000MK2.id + " ERROR] " + msg);
};


////////////////////////////////////////////////////////////////////////
// Buttons                                                            //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.MIDI_BUTTON_ON = 0x40;
DenonMC6000MK2.MIDI_BUTTON_OFF = 0x00;

DenonMC6000MK2.isButtonPressed = function(midiValue) {
    switch (midiValue) {
        case DenonMC6000MK2.MIDI_BUTTON_ON:
            return true;
        case DenonMC6000MK2.MIDI_BUTTON_OFF:
            return false;
        default:
            DenonMC6000MK2.logError("Unexpected MIDI button value: " + midiValue);
            return undefined;
    }
};


////////////////////////////////////////////////////////////////////////
// Values                                                             //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.toggleValue = function(group, key) {
    engine.setValue(group, key, !engine.getValue(group, key));
};


////////////////////////////////////////////////////////////////////////
// Knobs                                                              //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.MIDI_KNOB_INC = 0x00;
DenonMC6000MK2.MIDI_KNOB_DEC = 0x7F;

DenonMC6000MK2.getKnobDelta = function(midiValue) {
    switch (midiValue) {
        case DenonMC6000MK2.MIDI_KNOB_INC:
            return 1;
        case DenonMC6000MK2.MIDI_KNOB_DEC:
            return -1;
        default:
            DenonMC6000MK2.logError("Unexpected MIDI knob value: " + midiValue);
            return 0;
    }
};


////////////////////////////////////////////////////////////////////////
// LEDs                                                               //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.MIDI_LED_ON = 0x50;
DenonMC6000MK2.MIDI_LED_OFF = 0x51;

DenonMC6000MK2.LedState = function(midiCtrl) {
    this.midiCtrl = midiCtrl;
};

DenonMC6000MK2.LED_ON = new DenonMC6000MK2.LedState(DenonMC6000MK2.MIDI_LED_ON);
DenonMC6000MK2.LED_OFF = new DenonMC6000MK2.LedState(DenonMC6000MK2.MIDI_LED_OFF);

DenonMC6000MK2.Led = function(midiChannel, midiValue) {
    this.midiChannel = midiChannel;
    this.midiValue = midiValue;
    this.state = undefined;
};

DenonMC6000MK2.Led.prototype.setState = function(ledState) {
    this.state = ledState;
    this.trigger();
};

DenonMC6000MK2.Led.prototype.setStateBoolean = function(booleanValue) {
    if (booleanValue) {
        this.setState(DenonMC6000MK2.LED_ON);
    } else {
        this.setState(DenonMC6000MK2.LED_OFF);
    }
};

DenonMC6000MK2.Led.prototype.trigger = function() {
    midi.sendShortMsg(0xB0 + this.midiChannel, this.state.midiCtrl, this.midiValue);
};

DenonMC6000MK2.Led.prototype.reset = function() {
    this.setStateBoolean(false);
};

DenonMC6000MK2.MIDI_TRI_LED_ON = 0x4A;
DenonMC6000MK2.MIDI_TRI_LED_OFF = 0x4B;
DenonMC6000MK2.MIDI_TRI_LED_BLINK = 0x4C;

DenonMC6000MK2.TriLedState = function(midiCtrl) {
    this.midiCtrl = midiCtrl;
};

DenonMC6000MK2.TRI_LED_ON = new DenonMC6000MK2.TriLedState(DenonMC6000MK2.MIDI_TRI_LED_ON);
DenonMC6000MK2.TRI_LED_OFF = new DenonMC6000MK2.TriLedState(DenonMC6000MK2.MIDI_TRI_LED_OFF);
DenonMC6000MK2.TRI_LED_BLINK = new DenonMC6000MK2.TriLedState(DenonMC6000MK2.MIDI_TRI_LED_BLINK);

DenonMC6000MK2.TriLed = function(midiChannel, midiValue) {
    this.midiChannel = midiChannel;
    this.midiValue = midiValue;
    this.state = undefined;
};

DenonMC6000MK2.TriLed.prototype.setTriState = function(ledState) {
    this.state = ledState;
    this.trigger();
};

DenonMC6000MK2.TriLed.prototype.setStateBoolean = function(booleanValue) {
    if (booleanValue) {
        this.setTriState(DenonMC6000MK2.TRI_LED_ON);
    } else {
        this.setTriState(DenonMC6000MK2.TRI_LED_OFF);
    }
};

DenonMC6000MK2.TriLed.prototype.trigger = function() {
    midi.sendShortMsg(0xB0 + this.midiChannel, this.state.midiCtrl, this.midiValue);
};

DenonMC6000MK2.TriLed.prototype.reset = function() {
    this.setStateBoolean(false);
};

DenonMC6000MK2.connectedLeds = [];

DenonMC6000MK2.connectLed = function(midiChannel, midiValue) {
    var led = new DenonMC6000MK2.Led(midiChannel, midiValue);
    led.reset();
    DenonMC6000MK2.connectedLeds.push(led);
    return led;
};

DenonMC6000MK2.connectTriLed = function(midiChannel, midiValue) {
    var led = new DenonMC6000MK2.TriLed(midiChannel, midiValue);
    led.reset();
    DenonMC6000MK2.connectedLeds.push(led);
    return led;
};

DenonMC6000MK2.disconnectLeds = function() {
    for (var index in DenonMC6000MK2.connectedLeds) {
        DenonMC6000MK2.connectedLeds[index].reset();
    }
    DenonMC6000MK2.connectedLeds = [];
};


////////////////////////////////////////////////////////////////////////
// Controls                                                           //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.Control = function(group, ctrl, func) {
    this.group = group;
    this.ctrl = ctrl;
    this.func = func;
    this.isConnected = false;
};

DenonMC6000MK2.Control.prototype.connect = function() {
    if (this.isConnected) {
        DenonMC6000MK2.logWarning("Control is already connected: group=" + this.group + ", ctrl=" + this.ctrl + ", func=" + this.func);
        return true;
    }
    if (engine.connectControl(this.group, this.ctrl, this.func)) {
        this.isConnected = true;
        this.trigger();
    } else {
        DenonMC6000MK2.logError("Failed to connect control: group=" + this.group + ", ctrl=" + this.ctrl + ", func=" + this.func);
    }
    return this.isConnected;
};

DenonMC6000MK2.Control.prototype.disconnect = function() {
    if (this.isConnected) {
        if (engine.connectControl(this.group, this.ctrl, this.func, true)) {
            this.isConnected = false;
        } else {
            DenonMC6000MK2.logError("Failed to disconnect control: group=" + this.group + ", ctrl=" + this.ctrl + ", func=" + this.func);
        }
    } else {
        DenonMC6000MK2.logWarning("Control is not connected: group=" + this.group + ", ctrl=" + this.ctrl + ", func=" + this.func);
    }
};

DenonMC6000MK2.Control.prototype.trigger = function() {
    engine.trigger(this.group, this.ctrl);
};

DenonMC6000MK2.connectedControls = [];

DenonMC6000MK2.connectControl = function(group, ctrl, func) {
    var control = new DenonMC6000MK2.Control(group, ctrl, func);
    if (control.connect()) {
        DenonMC6000MK2.connectedControls.push(control);
        return control;
    } else {
        return undefined;
    }
};

DenonMC6000MK2.disconnectControls = function() {
    for (var index in DenonMC6000MK2.connectedControls) {
        DenonMC6000MK2.connectedControls[index].disconnect();
    }
    DenonMC6000MK2.connectedControls = [];
};


////////////////////////////////////////////////////////////////////////
// Hotcues                                                            //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.Hotcue = function(deck, number, midiLedValue, midiDimmerLedValue) {
    this.deck = deck;
    this.number = number;
    this.ctrlPrefix = "hotcue_" + number;
    this.midiLedValue = midiLedValue;
    this.midiDimmerLedValue = midiDimmerLedValue;
};

DenonMC6000MK2.Hotcue.prototype.connectControls = function(callbackFunc) {
    this.deck.connectControl(this.ctrlPrefix + "_enabled", callbackFunc);
};

DenonMC6000MK2.Hotcue.prototype.isEnabled = function() {
    return this.deck.getValue(this.ctrlPrefix + "_enabled");
};

DenonMC6000MK2.Hotcue.prototype.onButton = function(isButtonPressed) {
    if (isButtonPressed && this.deck.getShiftState()) {
        this.deck.setValue(this.ctrlPrefix + "_clear", true);
    } else {
        this.deck.setValue(this.ctrlPrefix + "_activate", isButtonPressed);
    }
};

DenonMC6000MK2.Hotcue.prototype.connectLeds = function() {
    this.led = this.deck.connectTriLed(this.midiLedValue);
    this.dimmerLed = this.deck.connectTriLed(this.midiDimmerLedValue);
};

DenonMC6000MK2.Hotcue.prototype.updateLeds = function() {
    this.led.setStateBoolean(this.isEnabled());
};


////////////////////////////////////////////////////////////////////////
// Samplers                                                           //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.samplerCount = 0;

DenonMC6000MK2.samplersByGroup = {};

DenonMC6000MK2.getSamplerByGroup = function(group) {
    var sampler = DenonMC6000MK2.samplersByGroup[group];
    if (undefined === sampler) {
        DenonMC6000MK2.logError("No sampler found for " + group);
    }
    return sampler;
};

DenonMC6000MK2.Sampler = function(side, midiChannel, midiLedValue, midiDimmerLedValue) {
    this.side = side;
    this.number = ++DenonMC6000MK2.samplerCount;
    this.group = "[Sampler" + this.number + "]";
    this.midiChannel = midiChannel;
    this.midiLedValue = midiLedValue;
    this.midiDimmerLedValue = midiDimmerLedValue;
    DenonMC6000MK2.samplersByGroup[this.group] = this;
};

DenonMC6000MK2.Sampler.prototype.isTrackLoaded = function() {
    return DenonMC6000MK2.isTrackLoaded(engine.getValue(this.group, "track_samples"));
};

DenonMC6000MK2.Sampler.prototype.loadSelectedTrack = function() {
    engine.setValue(this.group, "LoadSelectedTrack", true);
};

DenonMC6000MK2.Sampler.prototype.loadSelectedTrackAndPlay = function() {
    engine.setValue(this.group, "LoadSelectedTrackAndPlay", true);
};

DenonMC6000MK2.Sampler.prototype.isPlaying = function() {
    return engine.getValue(this.group, "play");
};

DenonMC6000MK2.Sampler.prototype.onButton = function(isButtonPressed) {
    switch (DenonMC6000MK2.DEFAULT_SAMPLER_MODE) {
        case DenonMC6000MK2.SAMPLER_MODE.TRIGGER:
            if (isButtonPressed) {
                if (this.isTrackLoaded()) {
                    if (this.side.getShiftState()) {
                        engine.setValue(this.group, "start_stop", true);
                    } else {
                        engine.setValue(this.group, "start_play", true);
                    }
                } else {
                    this.loadSelectedTrack();
                }
            }
            break;
        case DenonMC6000MK2.SAMPLER_MODE.HOLD:
            if (this.isTrackLoaded()) {
                if (isButtonPressed) {
                    if (this.side.getShiftState()) {
                        engine.setValue(this.group, "eject", true);
                    } else {
                        engine.setValue(this.group, "start_play", true);
                    }
                } else {
                    // continue playing if shift is pressed when
                    // releasing the pressed button
                    if (!this.side.getShiftState()) {
                        engine.setValue(this.group, "start_stop", true);
                    }
                }
            } else {
                if (isButtonPressed) {
                    this.loadSelectedTrack();
                }
            }
            break;
    }
};

DenonMC6000MK2.Sampler.prototype.connectLeds = function() {
    this.led = DenonMC6000MK2.connectTriLed(this.midiChannel, this.midiLedValue);
    this.dimmerLed = DenonMC6000MK2.connectTriLed(this.midiChannel, this.midiDimmerLedValue);
};

DenonMC6000MK2.Sampler.prototype.updateLeds = function() {
    if (this.isPlaying()) {
        this.led.setStateBoolean(true);
    } else {
        this.dimmerLed.setStateBoolean(this.isTrackLoaded());
    }
};

DenonMC6000MK2.Sampler.prototype.connectControls = function() {
    DenonMC6000MK2.connectControl(this.group, "play", DenonMC6000MK2.ctrlSampler);
    DenonMC6000MK2.connectControl(this.group, "track_samples", DenonMC6000MK2.ctrlSampler);
};


////////////////////////////////////////////////////////////////////////
// Decks                                                              //
////////////////////////////////////////////////////////////////////////

/* Management */

DenonMC6000MK2.decksByGroup = {};

DenonMC6000MK2.getDeckByGroup = function(group) {
    var deck = DenonMC6000MK2.decksByGroup[group];
    if (undefined === deck) {
        DenonMC6000MK2.logError("No deck found for " + group);
    }
    return deck;
};

/* Constructor */

DenonMC6000MK2.Deck = function(number, midiChannel) {
    this.side = undefined;
    this.number = number;
    this.group = "[Channel" + number + "]";
    this.filterGroup = "[QuickEffectRack1_" + this.group + "]";
    this.midiChannel = midiChannel;
    this.jogTouchState = false;
    this.scratchTimer = 0;
    DenonMC6000MK2.decksByGroup[this.group] = this;
    this.rateDirBackup = this.getValue("rate_dir");
    this.setValue("rate_dir", -1);
    this.vinylMode = undefined;
    this.syncMode = undefined;
    this.hotcues = [];
    this.hotcues[1] = new DenonMC6000MK2.Hotcue(this, 1, 0x11, 0x12);
    this.hotcues[2] = new DenonMC6000MK2.Hotcue(this, 2, 0x13, 0x14);
    this.hotcues[3] = new DenonMC6000MK2.Hotcue(this, 3, 0x15, 0x16);
    this.hotcues[4] = new DenonMC6000MK2.Hotcue(this, 4, 0x17, 0x18);
};

/* Shift */

DenonMC6000MK2.Deck.prototype.getShiftState = function(group) {
    return this.side.getShiftState();
};

/* Values & Parameters */

DenonMC6000MK2.Deck.prototype.getValue = function(key) {
    return engine.getValue(this.group, key);
};

DenonMC6000MK2.Deck.prototype.setValue = function(key, value) {
    engine.setValue(this.group, key, value);
};

DenonMC6000MK2.Deck.prototype.toggleValue = function(key) {
    this.setValue(key, !this.getValue(key));
};

DenonMC6000MK2.Deck.prototype.setParameter = function(key, param) {
    engine.setParameter(this.group, key, param);
};

DenonMC6000MK2.Deck.prototype.triggerValue = function(key) {
    engine.trigger(this.group, key);
};

/* Xfader */

DenonMC6000MK2.Deck.prototype.assignXfaderLeft = function() {
    this.setValue("orientation", DenonMC6000MK2.MIXXX_XFADER_LEFT);
};

DenonMC6000MK2.Deck.prototype.assignXfaderCenter = function() {
    this.setValue("orientation", DenonMC6000MK2.MIXXX_XFADER_CENTER);
};

DenonMC6000MK2.Deck.prototype.assignXfaderRight = function() {
    this.setValue("orientation", DenonMC6000MK2.MIXXX_XFADER_RIGHT);
};

/* Tracks */

DenonMC6000MK2.Deck.prototype.loadSelectedTrack = function() {
    this.setValue("LoadSelectedTrack", true);
    if (!this.isPlaying()) {
        this.setCueMixSolo(); // just for convenience ;)
    }
};

DenonMC6000MK2.Deck.prototype.loadSelectedTrackAndPlay = function() {
    this.setValue("LoadSelectedTrackAndPlay", true);
};

DenonMC6000MK2.Deck.prototype.unloadTrack = function() {
    this.setValue("eject", true);
};

DenonMC6000MK2.Deck.prototype.onLoadButton = function(isButtonPressed) {
    if (isButtonPressed) {
        if (this.getShiftState()) {
            this.unloadTrack();
        } else {
            this.loadSelectedTrack();
        }
    }
};

/* Key Lock Mode */

DenonMC6000MK2.Deck.prototype.onKeyLockButton = function(isButtonPressed) {
    if (isButtonPressed) {
        this.toggleValue("keylock");
    }
};

DenonMC6000MK2.Deck.prototype.enableKeyLock = function() {
    this.setValue("keylock", true);
};

DenonMC6000MK2.Deck.prototype.disableKeyLock = function() {
    this.setValue("keylock", false);
};

DenonMC6000MK2.Deck.prototype.onKeyLockValue = function(value) {
    this.keyLockLed.setStateBoolean(value);
};

/* Key Control */

DenonMC6000MK2.Deck.prototype.resetKey = function() {
    this.setValue("reset_key", true);
};

/* Sync Mode */

DenonMC6000MK2.Deck.prototype.disableSyncMode = function() {
    this.setValue("sync_mode", DenonMC6000MK2.MIXXX_SYNC_NONE);
};

DenonMC6000MK2.Deck.prototype.onSyncModeValue = function(value) {
    switch (value) {
        case DenonMC6000MK2.MIXXX_SYNC_NONE:
            this.syncModeLed.setTriState(DenonMC6000MK2.TRI_LED_OFF);
            break;
        case DenonMC6000MK2.MIXXX_SYNC_FOLLOWER:
            this.syncModeLed.setTriState(DenonMC6000MK2.TRI_LED_ON);
            break;
        case DenonMC6000MK2.MIXXX_SYNC_MASTER:
            this.syncModeLed.setTriState(DenonMC6000MK2.TRI_LED_BLINK);
            break;
        default:
            DenonMC6000MK2.logError("Unknown sync_mode value: " + value);
    }
};

/* Cue Mix */

DenonMC6000MK2.Deck.prototype.setCueMixSolo = function() {
    for (var deckGroup in DenonMC6000MK2.decksByGroup) {
        var deck = DenonMC6000MK2.getDeckByGroup(deckGroup);
        deck.setValue("pfl", this === deck);
    }
};

DenonMC6000MK2.Deck.prototype.onCueMixButton = function(isButtonPressed) {
    if (isButtonPressed) {
        if (this.getShiftState()) {
            this.setCueMixSolo();
        } else {
            this.toggleValue("pfl");
        }
    }
};

DenonMC6000MK2.Deck.prototype.updateCueMixValue = function(pflValue, isTrackLoaded) {
    if (pflValue) {
        this.cueMixLed.setStateBoolean(pflValue);
    } else {
        this.cueMixDimmerLed.setStateBoolean(isTrackLoaded);
    }
};

DenonMC6000MK2.Deck.prototype.onCueMixValue = function(pflValue) {
    this.updateCueMixValue(pflValue, this.isTrackLoaded());
};

// BPM
DenonMC6000MK2.Deck.prototype.onBpmValue = function(value) {
    var efxUnit = this.side.efxUnit;
    efxUnit.onDeckBpmValue(this, value);
};

/* Track Load */

DenonMC6000MK2.isTrackLoaded = function(trackSamples) {
    return 0 < trackSamples;
};

DenonMC6000MK2.Deck.prototype.isTrackLoaded = function() {
    return DenonMC6000MK2.isTrackLoaded(this.getValue("track_samples"));
};

DenonMC6000MK2.Deck.prototype.onTrackSamplesValue = function(value) {
    this.updateCueMixValue(this.getValue("pfl"), DenonMC6000MK2.isTrackLoaded(value));
};

/* Cue & Play */

DenonMC6000MK2.Deck.prototype.onCueButton = function(isButtonPressed) {
    if (engine.getValue("[AutoDJ]", "enabled")) {
        if (isButtonPressed && this.isTrackLoaded() && !this.isPlaying()) {
            engine.setValue("[AutoDJ]", "skip_next", true);
        }
    } else {
        if (this.isTrackLoaded()) {
            if (this.getShiftState()) {
                if (isButtonPressed) {
                    if (this.isPlaying()) {
                        // break effect
                        engine.brake(this.number, isButtonPressed);
                    } else {
                        if (0.0 < this.getValue("cue_point")) {
                            // clear cue point
                            this.setValue("cue_point", 0.0);
                        } else {
                            // no cue point -> jump to beginning of track
                            this.setValue("playposition", 0.0);
                        }
                    }
                }
            } else {
                this.setValue("cue_default", isButtonPressed);
            }
        } else {
            if (isButtonPressed) {
                this.loadSelectedTrack();
            }
        }
    }
};

DenonMC6000MK2.Deck.prototype.onCueIndicatorValue = function(value) {
    this.cueLed.setStateBoolean(value);
};

DenonMC6000MK2.Deck.prototype.onPlayButton = function(isButtonPressed) {
    if (engine.getValue("[AutoDJ]", "enabled")) {
        if (isButtonPressed && this.isTrackLoaded() && !this.isPlaying()) {
            engine.setValue("[AutoDJ]", "fade_now", true);
        }
    } else {
        if (this.isTrackLoaded()) {
            if (this.getShiftState()) {
                this.setValue("play_stutter", isButtonPressed);
            } else {
                if (isButtonPressed) {
                    if (this.isPlaying()) {
                        this.setValue("stop", true);
                    } else {
                        this.setValue("play", true);
                    }
                }
            }
        } else {
            if (isButtonPressed) {
                this.loadSelectedTrackAndPlay();
            }
        }
    }
};

DenonMC6000MK2.Deck.prototype.isPlaying = function() {
    return this.getValue("play");
};

DenonMC6000MK2.Deck.prototype.onPlayIndicatorValue = function(value) {
    this.playLed.setStateBoolean(value);
};

/* Pitch Bend / Track Search */

DenonMC6000MK2.Deck.prototype.onBendPlusButton = function(isButtonPressed) {
    if (this.isPlaying()) {
        this.setValue("fwd", false);
        if (this.getShiftState()) {
            this.setValue("rate_temp_up_small", isButtonPressed);
        } else {
            this.setValue("rate_temp_up", isButtonPressed);
        }
    } else {
        this.setValue("fwd", isButtonPressed);
    }
};

DenonMC6000MK2.Deck.prototype.onBendMinusButton = function(isButtonPressed) {
    if (this.isPlaying()) {
        this.setValue("back", false);
        if (this.getShiftState()) {
            this.setValue("rate_temp_down_small", isButtonPressed);
        } else {
            this.setValue("rate_temp_down", isButtonPressed);
        }
    } else {
        this.setValue("back", isButtonPressed);
    }
};


/* Censor / Slip Mode */

DenonMC6000MK2.Deck.prototype.onCensorButton = function(isButtonPressed) {
    if (this.getShiftState()) {
        // Please note that "reverseroll" seems to have side effects on
        // "slip_enabled" so better leave it alone while shift is pressed!
        if (isButtonPressed) {
            this.toggleValue("slip_enabled");
        }
    } else {
        this.setValue("reverseroll", isButtonPressed);
    }
};

DenonMC6000MK2.Deck.prototype.onSlipModeValue = function(value) {
    this.slipModeLed.setStateBoolean(value);
};

/* Vinyl Mode (Scratching) */

DenonMC6000MK2.Deck.prototype.onVinylModeValue = function() {
    this.vinylModeLed.setStateBoolean(this.vinylMode);
};

DenonMC6000MK2.Deck.prototype.enableScratching = function() {
    engine.scratchEnable(this.number,
        DenonMC6000MK2.JOG_RESOLUTION,
        DenonMC6000MK2.JOG_SCRATCH_RPM,
        DenonMC6000MK2.JOG_SCRATCH_ALPHA,
        DenonMC6000MK2.JOG_SCRATCH_BETA,
        DenonMC6000MK2.JOG_SCRATCH_RAMP);
};

DenonMC6000MK2.Deck.prototype.disableScratching = function() {
    if (0 !== this.scratchTimer) {
        engine.stopTimer(this.scratchTimer);
        this.scratchTimer = 0;
    }
    var scratch2 = engine.getValue(this.group, "scratch2");
    if ((!this.isPlaying() && (DenonMC6000MK2.JOG_SCRATCH2_ABS_MIN < Math.abs(scratch2))) ||
        (scratch2 < DenonMC6000MK2.JOG_SCRATCH2_PLAY_MIN) ||
        (scratch2 > DenonMC6000MK2.JOG_SCRATCH2_PLAY_MAX)) {
        var timeoutCallback =
            "DenonMC6000MK2.onScratchingTimeoutDeck" + this.number + "()";
        this.scratchTimer = engine.beginTimer(
            DenonMC6000MK2.JOG_SCRATCH_TIMEOUT,
            timeoutCallback,
            true);
    }
    if (0 === this.scratchTimer) {
        // Ramping only when doing a back-spin while playing
        var ramping = this.isPlaying() && (scratch2 < 0.0);
        engine.scratchDisable(this.number, ramping && DenonMC6000MK2.JOG_SCRATCH_RAMP);
    }
};

DenonMC6000MK2.Deck.prototype.onScratchingTimeout = function() {
    this.scratchTimer = 0;
    this.disableScratching();
};

DenonMC6000MK2.onScratchingTimeoutDeck1 = function() {
    DenonMC6000MK2.getDeckByGroup("[Channel1]").onScratchingTimeout();
};

DenonMC6000MK2.onScratchingTimeoutDeck2 = function() {
    DenonMC6000MK2.getDeckByGroup("[Channel2]").onScratchingTimeout();
};

DenonMC6000MK2.onScratchingTimeoutDeck3 = function() {
    DenonMC6000MK2.getDeckByGroup("[Channel3]").onScratchingTimeout();
};

DenonMC6000MK2.onScratchingTimeoutDeck4 = function() {
    DenonMC6000MK2.getDeckByGroup("[Channel4]").onScratchingTimeout();
};

DenonMC6000MK2.Deck.prototype.updateVinylMode = function() {
    if (this.vinylMode && this.jogTouchState) {
        this.enableScratching();
    } else {
        this.disableScratching();
    }
    this.onVinylModeValue();
};

DenonMC6000MK2.Deck.prototype.setVinylMode = function(vinylMode) {
    this.vinylMode = vinylMode;
    this.updateVinylMode();
};

DenonMC6000MK2.Deck.prototype.toggleVinylMode = function() {
    this.setVinylMode(!this.vinylMode);
};

DenonMC6000MK2.Deck.prototype.enableVinylMode = function() {
    this.setVinylMode(true);
};

DenonMC6000MK2.Deck.prototype.disableVinylMode = function() {
    this.setVinylMode(false);
};

DenonMC6000MK2.Deck.prototype.onVinylButton = function(isButtonPressed) {
    this.toggleVinylMode();
};

/* Jog Wheel */

DenonMC6000MK2.Deck.prototype.touchJog = function(isJogTouched) {
    this.jogTouchState = isJogTouched;
    this.updateVinylMode();
};

DenonMC6000MK2.Deck.prototype.spinJog = function(jogDelta) {
    if (this.getShiftState() && !this.isPlaying()) {
        // fast track seek (strip search)
        var playPos = engine.getValue(this.group, "playposition");
        if (undefined !== playPos) {
            var seekPos = playPos + (jogDelta / (DenonMC6000MK2.JOG_RESOLUTION * DenonMC6000MK2.JOG_SEEK_REVOLUTIONS));
            this.setValue("playposition", Math.max(0.0, Math.min(1.0, seekPos)));
        }
    } else {
        if (engine.isScratching(this.number)) {
            engine.scratchTick(this.number, jogDelta);
            if (!this.jogTouchState) {
                this.disableScratching();
            }
        } else {
            var normalizedDelta = jogDelta / DenonMC6000MK2.MIDI_JOG_DELTA_RANGE;
            var scaledDelta;
            var jogExponent;
            if (this.isPlaying()) {
                // bending
                scaledDelta = normalizedDelta / DenonMC6000MK2.JOG_SPIN_PLAY_PEAK;
                jogExponent = DenonMC6000MK2.JOG_SPIN_PLAY_EXPONENT;
            } else {
                // cueing
                scaledDelta = normalizedDelta / DenonMC6000MK2.JOG_SPIN_CUE_PEAK;
                jogExponent = DenonMC6000MK2.JOG_SPIN_CUE_EXPONENT;
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
            var jogValue = DenonMC6000MK2.MIXXX_JOG_RANGE * scaledDeltaPow;
            this.setValue("jog", jogValue);
        }
    }
};

/* Filter */

DenonMC6000MK2.Deck.prototype.applyFilter = function() {
    engine.setValue(this.filterGroup, "enabled", this.side.filterEnabled);
    engine.setParameter(this.filterGroup, "super1", this.side.filterParam);
};

/* Loops */

DenonMC6000MK2.Deck.prototype.hasLoopStart = function() {
    return DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED !== this.getValue("loop_start_position");
};

DenonMC6000MK2.Deck.prototype.hasLoopEnd = function() {
    return DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED !== this.getValue("loop_end_position");
};

DenonMC6000MK2.Deck.prototype.hasLoop = function() {
    return this.hasLoopStart() && this.hasLoopEnd();
};

DenonMC6000MK2.Deck.prototype.deleteLoopStart = function() {
    this.setValue("loop_start_position", DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED);
};

DenonMC6000MK2.Deck.prototype.deleteLoopEnd = function() {
    this.setValue("loop_end_position", DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED);
};

DenonMC6000MK2.Deck.prototype.deleteLoop = function() {
    // loop end has to be deleted before loop start
    this.deleteLoopEnd();
    this.deleteLoopStart();
};

DenonMC6000MK2.Deck.prototype.toggleLoop = function() {
    this.setValue("reloop_exit", true);
};

DenonMC6000MK2.Deck.prototype.onAutoLoopButton = function(isButtonPressed) {
    if (isButtonPressed) {
        if (this.hasLoop()) {
            if (this.getShiftState()) {
                this.deleteLoop();
            } else {
                this.toggleLoop();
            }
        } else {
            if (this.getShiftState()) {
                this.setValue("beatlooproll_4_activate", true);
            } else {
                this.setValue("beatloop_4_activate", true);
            }
        }
    }
};

DenonMC6000MK2.Deck.prototype.onLoopInButton = function(isButtonPressed) {
    if (isButtonPressed) {
        if (this.getShiftState()) {
            this.deleteLoop(); // both start & end
        } else {
            this.setValue("loop_in", true);
        }
    }
};

DenonMC6000MK2.Deck.prototype.onLoopOutButton = function(isButtonPressed) {
    if (isButtonPressed) {
        if (this.getShiftState()) {
            this.deleteLoopEnd();
        } else {
            this.setValue("loop_out", true);
        }
    }
};

DenonMC6000MK2.Deck.prototype.onLoopCutMinusButton = function(isButtonPressed) {
    if (isButtonPressed) {
        if (this.getShiftState()) {
            this.setValue("loop_move_1_backward", true);
        } else {
            this.setValue("loop_halve", true);
        }
    }
};

DenonMC6000MK2.Deck.prototype.onLoopCutPlusButton = function(isButtonPressed) {
    if (isButtonPressed) {
        if (this.getShiftState()) {
            this.setValue("loop_move_1_forward", true);
        } else {
            this.setValue("loop_double", true);
        }
    }
};

DenonMC6000MK2.Deck.prototype.updateLoopLeds = function(value) {
    if (this.getValue("loop_enabled")) {
        this.loopInLed.setTriState(DenonMC6000MK2.TRI_LED_BLINK);
        this.loopOutLed.setTriState(DenonMC6000MK2.TRI_LED_BLINK);
        this.autoLoopLed.setTriState(DenonMC6000MK2.TRI_LED_BLINK);
    } else {
        this.loopInLed.setStateBoolean(this.hasLoopStart());
        this.loopOutLed.setStateBoolean(this.hasLoop()); // both start & end
        if (this.hasLoop()) {
            this.autoLoopDimmerLed.setStateBoolean(true);
        } else {
            this.autoLoopDimmerLed.setStateBoolean(false);
        }
    }
};

/* Deck LEDs */

DenonMC6000MK2.Deck.prototype.connectLed = function(midiValue) {
    return DenonMC6000MK2.connectLed(this.midiChannel, midiValue);
};

DenonMC6000MK2.Deck.prototype.connectTriLed = function(midiValue) {
    return DenonMC6000MK2.connectTriLed(this.midiChannel, midiValue);
};

/* Startup */

DenonMC6000MK2.Deck.prototype.connectLeds = function() {
    this.vinylModeLed = this.connectTriLed(0x06);
    this.keyLockLed = this.connectTriLed(0x08);
    this.syncModeLed = this.connectTriLed(0x09);
    this.cueLed = this.connectTriLed(0x26);
    this.playLed = this.connectTriLed(0x27);
    this.loopInLed = this.connectTriLed(0x24);
    this.loopInDimmerLed = this.connectTriLed(0x3E);
    this.loopOutLed = this.connectTriLed(0x40);
    this.loopOutDimmerLed = this.connectTriLed(0x2A);
    this.autoLoopLed = this.connectTriLed(0x2B);
    this.autoLoopDimmerLed = this.connectTriLed(0x53);
    this.slipModeLed = this.connectTriLed(0x64);
    for (var hotcueIndex in this.hotcues) {
        this.hotcues[hotcueIndex].connectLeds();
    }
};

DenonMC6000MK2.Deck.prototype.connectControl = function(ctrl, func) {
    return DenonMC6000MK2.connectControl(this.group, ctrl, func);
};

DenonMC6000MK2.Deck.prototype.connectControls = function() {
    this.connectControl("cue_indicator", DenonMC6000MK2.ctrlCueIndicator);
    this.connectControl("play_indicator", DenonMC6000MK2.ctrlPlayIndicator);
    this.connectControl("sync_mode", DenonMC6000MK2.ctrlSyncMode);
    this.connectControl("keylock", DenonMC6000MK2.ctrlKeyLock);
    this.connectControl("pfl", DenonMC6000MK2.ctrlCueMix);
    this.connectControl("bpm", DenonMC6000MK2.ctrlBpm);
    this.connectControl("track_samples", DenonMC6000MK2.ctrlTrackSamples);
    this.connectControl("slip_enabled", DenonMC6000MK2.ctrlSlipModeValue);
    this.connectControl("loop_enabled", DenonMC6000MK2.ctrlLoopEnabled);
    this.connectControl("loop_start_position", DenonMC6000MK2.ctrlLoopStartPosition);
    this.connectControl("loop_end_position", DenonMC6000MK2.ctrlLoopEndPosition);
    DenonMC6000MK2.leftSide.efxUnit.connectDeckControls(this, DenonMC6000MK2.leftSide.efxUnit.ctrlDeck);
    DenonMC6000MK2.rightSide.efxUnit.connectDeckControls(this, DenonMC6000MK2.rightSide.efxUnit.ctrlDeck);
    this.hotcues[1].connectControls(DenonMC6000MK2.ctrlHotcue1);
    this.hotcues[2].connectControls(DenonMC6000MK2.ctrlHotcue2);
    this.hotcues[3].connectControls(DenonMC6000MK2.ctrlHotcue3);
    this.hotcues[4].connectControls(DenonMC6000MK2.ctrlHotcue4);
    // default settings
    this.enableKeyLock();
    this.enableVinylMode();
    this.disableSyncMode();
};

/* Shutdown */

DenonMC6000MK2.Deck.prototype.restoreValues = function() {
    this.setValue("rate_dir", this.rateDirBackup);
};


////////////////////////////////////////////////////////////////////////
// Efx Params                                                         //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.EfxParam = function(unit, group, key) {
    this.unit = unit;
    this.group = group;
    this.key = key;
    this.param = undefined; // reflects the knob's actual position
    this.paramOverride = undefined;
};

DenonMC6000MK2.EfxParam.prototype.isValid = function() {
    return (undefined !== this.group) && (undefined !== this.key);
};

DenonMC6000MK2.EfxParam.prototype.getParam = function() {
    if (this.isValid()) {
        return engine.getParameter(this.group, this.key);
    } else {
        return undefined;
    }
};

DenonMC6000MK2.EfxParam.prototype.applyParam = function(param) {
    if (this.isValid()) {
        engine.setParameter(this.group, this.key, param);
        if (this === this.unit.dryWetMixParam) {
            this.unit.updateWetLoopSlip();
        }
    }
};

DenonMC6000MK2.EfxParam.prototype.apply = function() {
    var param = (this.paramOverride !== undefined) ? this.paramOverride : this.param;
    if (undefined !== param) {
        this.applyParam(param);
    }
};

DenonMC6000MK2.EfxParam.prototype.adjustParam = function(param) {
    this.param = param;
    this.apply();
};

DenonMC6000MK2.EfxParam.prototype.adjustValue = function(value) {
    if (this.isValid()) {
        var param = engine.getParameterForValue(this.group, this.key, value);
        this.adjustParam(param);
    }
};

DenonMC6000MK2.EfxParam.prototype.toggleParamOverride = function(paramOverride) {
    if (this.paramOverride !== undefined) {
        this.paramOverride = undefined;
        this.onLed.setStateBoolean(false);
    } else {
        this.paramOverride = paramOverride;
        this.onLed.setStateBoolean(true);
    }
    this.apply();
};

DenonMC6000MK2.EfxParam.prototype.resetParamOverride = function() {
    if (this.paramOverride !== undefined) {
        this.paramOverride = undefined;
        this.onLed.setStateBoolean(false);
    }
    this.apply();
};


////////////////////////////////////////////////////////////////////////
// Efx Units                                                          //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.efxUnitsByGroup = {};

DenonMC6000MK2.getEfxUnitByGroup = function(group) {
    var efxUnit = DenonMC6000MK2.efxUnitsByGroup[group];
    if (undefined === efxUnit) {
        DenonMC6000MK2.logError("No efx unit found for " + group);
    }
    return efxUnit;
};

DenonMC6000MK2.EfxUnit = function(side, group, paramGroup) {
    this.side = side;
    this.group = group;
    DenonMC6000MK2.efxUnitsByGroup[group] = this;
    this.paramGroup = paramGroup;
    DenonMC6000MK2.efxUnitsByGroup[paramGroup] = this;
    this.wetLoop = false;
    this.wetLoopSlip = false;
    this.echoLoopParams = false;
    this.echoLoopDelayBeats = 1.0; // 1 beat echo
    this.params = [];
    this.params[1] = new DenonMC6000MK2.EfxParam(this);
    this.params[2] = new DenonMC6000MK2.EfxParam(this);
    this.params[3] = new DenonMC6000MK2.EfxParam(this, this.group, "mix");
    this.dryWetMixParam = this.params[3];
    this.dryWetMixParam.adjustParam(0.5); // mix (50% dry / 50% wet)
};

DenonMC6000MK2.EfxUnit.prototype.getDeckAssignKey = function(deck) {
    return "group_" + deck.group + "_enable";
};

DenonMC6000MK2.EfxUnit.prototype.connectDeckControls = function(deck, assignCB) {
    DenonMC6000MK2.connectControl(this.group, this.getDeckAssignKey(deck), assignCB);
};

DenonMC6000MK2.EfxUnit.prototype.isDeckAssigned = function(deck) {
    return engine.getValue(this.group, this.getDeckAssignKey(deck));
};

DenonMC6000MK2.EfxUnit.prototype.isActiveDeckAssigned = function() {
    return engine.getValue(this.group, this.getDeckAssignKey(this.side.activeDeck));
};

DenonMC6000MK2.EfxUnit.prototype.isAnyDeckAssigned = function() {
    for (var deckGroup in DenonMC6000MK2.decksByGroup) {
        var deck = DenonMC6000MK2.getDeckByGroup(deckGroup);
        if (this.isDeckAssigned(deck)) {
            return true;
        }
    }
    return false;
};

DenonMC6000MK2.EfxUnit.prototype.assignDeck = function(deck) {
    engine.setValue(this.group, this.getDeckAssignKey(deck), true);
};

DenonMC6000MK2.EfxUnit.prototype.unassignDeck = function(deck) {
    engine.setValue(this.group, this.getDeckAssignKey(deck), false);
};

DenonMC6000MK2.EfxUnit.prototype.assignDeckToggle = function(deck) {
    DenonMC6000MK2.toggleValue(this.group, this.getDeckAssignKey(deck));
};

DenonMC6000MK2.EfxUnit.prototype.assignDeckExlusively = function(deck) {
    for (var deckGroup in DenonMC6000MK2.decksByGroup) {
        var varDeck = DenonMC6000MK2.getDeckByGroup(deckGroup);
        engine.setValue(this.group, this.getDeckAssignKey(varDeck), varDeck === deck);
    }
};

DenonMC6000MK2.EfxUnit.prototype.getAssignedDeck = function() {
    if (this.isActiveDeckAssigned()) {
        // prefer the active deck of this side over any other
        return this.side.activeDeck;
    } else {
        var assignedDeck;
        for (var deckGroup in DenonMC6000MK2.decksByGroup) {
            var deck = DenonMC6000MK2.getDeckByGroup(deckGroup);
            if (this.isDeckAssigned(deck)) {
                if ((undefined === assignedDeck) || (deck.side === this.side)) {
                    assignedDeck = deck;
                }
            }
        }
        return assignedDeck;
    }
};

DenonMC6000MK2.EfxUnit.prototype.applyParams = function() {
    for (var index in this.params) {
        var param = this.params[index];
        param.apply();
    }
};

DenonMC6000MK2.EfxUnit.prototype.disable = function() {
    engine.setValue(this.group, "enabled", false);
};

DenonMC6000MK2.EfxUnit.prototype.initEchoLoopParams = function() {
    engine.setValue(this.group, "clear", true);
    engine.setValue(this.group, "next_chain", true);
    engine.setValue(this.group, "next_chain", true);
    engine.setValue(this.group, "next_chain", true);
    engine.setValue(this.group, "next_chain", true);
    engine.setValue(this.group, "next_chain", true);
    this.echoLoopParams = true;
    // send: full (fixed)
    engine.setParameter(this.paramGroup, "parameter1", 1.0);
    // pingpong: off (fixed)
    engine.setParameter(this.paramGroup, "parameter4", 0.0);
    // delay
    this.params[1].group = this.paramGroup;
    this.params[1].key = "parameter2";
    // feedback
    this.params[2].group = this.paramGroup;
    this.params[2].key = "parameter3";
    // synchronize delay with bpm of active deck
    var activeDeck = this.side.activeDeck;
    if (undefined !== activeDeck) {
        this.assignDeckExlusively(activeDeck);
        this.onDeckBpmValue(activeDeck, activeDeck.getValue("bpm"));
    }
    this.params[2].adjustParam(DenonMC6000MK2.ECHO_LOOP_FEEDBACK);
    this.applyParams();
};

DenonMC6000MK2.EfxUnit.prototype.initDefaultParams = function() {
    engine.setValue(this.group, "clear", true);
    this.echoLoopParams = false;
    this.params[1].group = undefined;
    this.params[1].key = undefined;
    this.params[2].group = this.group;
    this.params[2].key = "super1";
    this.applyParams();
};

DenonMC6000MK2.EfxUnit.prototype.syncEchoDelayWithBpm = function(bpm, beats) {
    if (0.0 < bpm) {
        var secondsPerBeat = 60.0 / bpm;
        this.params[1].adjustValue(beats * secondsPerBeat);
    } else {
        // Reset param value
        this.params[1].apply();
    }
};

DenonMC6000MK2.EfxUnit.prototype.onDeckBpmValue = function(deck, bpm) {
    if (this.echoLoopParams && (deck === this.getAssignedDeck())) {
        this.syncEchoDelayWithBpm(bpm, this.echoLoopDelayBeats);
    }
};

DenonMC6000MK2.EfxUnit.prototype.getShiftState = function() {
    return this.side.getShiftState();
};

DenonMC6000MK2.EfxUnit.prototype.isEnabled = function() {
    return engine.getValue(this.group, "enabled");
};

DenonMC6000MK2.EfxUnit.prototype.onEnableButton = function(isButtonPressed) {
    if (isButtonPressed) {
        if (this.getShiftState()) {
            engine.setValue(this.group, "enabled", true);
            this.wetLoop = true;
        } else {
            DenonMC6000MK2.toggleValue(this.group, "enabled");
            if (this.isEnabled()) {
                if (this.getShiftState()) {
                    this.wetLoop = !this.wetLoop;
                }
            } else {
                this.wetLoop = false;
            }
        }
    }
};

DenonMC6000MK2.EfxUnit.prototype.onParamButton = function(index, isButtonPressed) {
    if (isButtonPressed) {
        var efxParam = this.params[index];
        if (this.getShiftState()) {
            efxParam.resetParamOverride();
            efxParam.adjustParam(0.0);
        } else {
            efxParam.toggleParamOverride(1.0);
        }
    }
};

DenonMC6000MK2.EfxUnit.prototype.updateWetLoopSlip = function() {
    var deck, deckGroup;
    if (this.isEnabled()) {
        var wetLoopSlip = this.wetLoop &&
            (this.dryWetMixParam.getParam() === 1.0);
        if (this.wetLoopSlip !== wetLoopSlip) {
            for (deckGroup in DenonMC6000MK2.decksByGroup) {
                deck = DenonMC6000MK2.getDeckByGroup(deckGroup);
                if (this.isDeckAssigned(deck)) {
                    deck.setValue("slip_enabled", wetLoopSlip);
                    deck.setValue("play", !wetLoopSlip);
                }
            }
            this.wetLoopSlip = wetLoopSlip;
        }
    } else {
        if (this.wetLoopSlip) {
            for (deckGroup in DenonMC6000MK2.decksByGroup) {
                deck = DenonMC6000MK2.getDeckByGroup(deckGroup);
                if (this.isDeckAssigned(deck)) {
                    deck.setValue("slip_enabled", false);
                }
            }
            this.wetLoopSlip = false;
        }
    }
};

DenonMC6000MK2.EfxUnit.prototype.onDryWetMix = function() {
    this.updateWetLoopSlip();
};

DenonMC6000MK2.EfxUnit.prototype.onEnabled = function() {
    if (!this.isEnabled()) {
        for (var index in this.params) {
            this.params[index].resetParamOverride();
        }
    }
    this.updateWetLoopSlip();
    if (this.isEnabled()) {
        if (this.wetLoop) {
            this.tapLed.setTriState(DenonMC6000MK2.TRI_LED_BLINK);
        } else {
            this.tapLed.setTriState(DenonMC6000MK2.TRI_LED_ON);
        }
    } else {
        this.tapLed.setTriState(DenonMC6000MK2.TRI_LED_OFF);
    }
};

DenonMC6000MK2.EfxUnit.prototype.onParamMidiValue = function(index, value) {
    var param = script.absoluteLin(value, 0.0, 1.0);
    this.params[index].adjustParam(param);
};

DenonMC6000MK2.EfxUnit.prototype.onBeatsButton = function(isButtonPressed) {
    if (isButtonPressed) {
        if (this.getShiftState()) {
            // Toggle between echo loop and default params
            if (this.echoLoopParams) {
                this.initDefaultParams();
            } else {
                this.initEchoLoopParams();
            }
        } else {
            var deck = this.side.activeDeck;
            deck.resetKey();
        }
    }
};

DenonMC6000MK2.EfxUnit.prototype.onBeatsKnobDelta = function(delta) {
    if (this.getShiftState()) {
        engine.setValue(this.group, "chain_selector", delta);
    } else {
        var deck = this.side.activeDeck;
        if (delta < 0) {
            deck.setValue("pitch_down_small", true);
        } else if (delta > 0) {
            deck.setValue("pitch_up_small", true);
        }
    }
};

DenonMC6000MK2.EfxUnit.prototype.onDeckButton = function(deckGroup, isButtonPressed) {
    if (isButtonPressed) {
        var deck = DenonMC6000MK2.getDeckByGroup(deckGroup);
        if (this.getShiftState()) {
            this.assignDeckExlusively(deck);
        } else {
            this.assignDeckToggle(deck);
        }
    }
};

////////////////////////////////////////////////////////////////////////
// Sides                                                              //
////////////////////////////////////////////////////////////////////////

/* Management */

DenonMC6000MK2.sidesByGroup = {};

DenonMC6000MK2.getSideByGroup = function(group) {
    var side = DenonMC6000MK2.sidesByGroup[group];
    if (undefined === side) {
        DenonMC6000MK2.logError("No side found for " + group);
    }
    return side;
};

/* Constructor */

DenonMC6000MK2.Side = function(decks, efxUnitGroup, efxParamGroup, samplerMidiChannel) {
    this.decksByGroup = {};
    for (var deckIndex in decks) {
        var deck = decks[deckIndex];
        deck.side = this;
        this.decksByGroup[deck.group] = deck;
        DenonMC6000MK2.sidesByGroup[deck.group] = this;
    }
    this.activeDeck = decks[0];
    this.shiftState = false;
    this.efxUnit = new DenonMC6000MK2.EfxUnit(this, efxUnitGroup, efxParamGroup);
    this.samplers = [];
    this.samplers[1] = new DenonMC6000MK2.Sampler(this, samplerMidiChannel, 0x19, 0x1A);
    this.samplers[2] = new DenonMC6000MK2.Sampler(this, samplerMidiChannel, 0x1B, 0x1C);
    this.samplers[3] = new DenonMC6000MK2.Sampler(this, samplerMidiChannel, 0x1D, 0x1F);
    this.samplers[4] = new DenonMC6000MK2.Sampler(this, samplerMidiChannel, 0x20, 0x21);
    this.filterLed = undefined;
};

/* Shift */

DenonMC6000MK2.Side.prototype.getShiftState = function(group) {
    if (DenonMC6000MK2.GLOBAL_SHIFT_STATE) {
        return DenonMC6000MK2.getShiftState();
    } else {
        return this.shiftState;
    }
};

/* Decks */

DenonMC6000MK2.Side.prototype.getDeckByGroup = function(group) {
    var deck = this.decksByGroup[group];
    if (undefined === deck) {
        DenonMC6000MK2.logError("No deck found for " + group);
    }
    return deck;
};

DenonMC6000MK2.Side.prototype.onDeckButton = function(deckGroup, isButtonPressed) {
    if (isButtonPressed) {
        this.activeDeck = this.getDeckByGroup(deckGroup);
    }
};


/* Startup */

DenonMC6000MK2.Side.prototype.connectLeds = function() {
    for (var deckGroup in this.decksByGroup) {
        var deck = this.decksByGroup[deckGroup];
        deck.connectLeds();
    }
    for (var samplerIndex in this.samplers) {
        this.samplers[samplerIndex].connectLeds();
    }
};

DenonMC6000MK2.Side.prototype.connectControls = function() {
    for (var deckGroup in this.decksByGroup) {
        var deck = this.decksByGroup[deckGroup];
        deck.connectControls();
    }
    for (var samplerIndex in this.samplers) {
        this.samplers[samplerIndex].connectControls();
    }
};

/* Shutdown */

DenonMC6000MK2.Side.prototype.restoreValues = function() {
    for (var group in this.decksByGroup) {
        var deck = this.decksByGroup[group];
        deck.restoreValues();
    }
};

// Shift
DenonMC6000MK2.Side.prototype.onShiftButton = function(isButtonPressed) {
    // local shift state
    this.shiftState = isButtonPressed;
    // global shift state
    DenonMC6000MK2.shiftState = isButtonPressed;
};

// Filter

DenonMC6000MK2.Side.prototype.applyFilter = function() {
    for (var group in this.decksByGroup) {
        var deck = this.decksByGroup[group];
        deck.applyFilter();
    }
};

DenonMC6000MK2.Side.prototype.initFilter = function() {
    this.filterEnabled = true;
    this.filterParam = 0.5; // centered
    this.applyFilter();
};

DenonMC6000MK2.Side.prototype.toggleFilter = function() {
    this.filterEnabled = !this.filterEnabled;
    this.applyFilter();
};

DenonMC6000MK2.Side.prototype.onFilterButton = function(isButtonPressed) {
    if (isButtonPressed) {
        this.toggleFilter();
    }
};

DenonMC6000MK2.Side.prototype.onFilterMidiValue = function(value) {
    this.filterParam = script.absoluteLin(value, 0.0, 1.0);
    this.applyFilter();
};


////////////////////////////////////////////////////////////////////////
// Controller functions                                               //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.id = undefined;
DenonMC6000MK2.debug = undefined;
DenonMC6000MK2.group = "[Master]";

// left side
DenonMC6000MK2.deck1 = new DenonMC6000MK2.Deck(1, DenonMC6000MK2.MIDI_CH0);
DenonMC6000MK2.deck3 = new DenonMC6000MK2.Deck(3, DenonMC6000MK2.MIDI_CH1);
DenonMC6000MK2.leftDecks = [DenonMC6000MK2.deck1, DenonMC6000MK2.deck3];
DenonMC6000MK2.leftSide = new DenonMC6000MK2.Side(DenonMC6000MK2.leftDecks, DenonMC6000MK2.LEFT_EFX_UNIT_GROUP, DenonMC6000MK2.LEFT_EFX_PARAM_GROUP, DenonMC6000MK2.MIDI_CH0);

// right side
DenonMC6000MK2.deck2 = new DenonMC6000MK2.Deck(2, DenonMC6000MK2.MIDI_CH2);
DenonMC6000MK2.deck4 = new DenonMC6000MK2.Deck(4, DenonMC6000MK2.MIDI_CH3);
DenonMC6000MK2.rightDecks = [DenonMC6000MK2.deck2, DenonMC6000MK2.deck4];
DenonMC6000MK2.rightSide = new DenonMC6000MK2.Side(DenonMC6000MK2.rightDecks, DenonMC6000MK2.RIGHT_EFX_UNIT_GROUP, DenonMC6000MK2.RIGHT_EFX_PARAM_GROUP, DenonMC6000MK2.MIDI_CH2);

DenonMC6000MK2.sides = [DenonMC6000MK2.leftSide, DenonMC6000MK2.rightSide];

DenonMC6000MK2.getShiftState = function() {
    return DenonMC6000MK2.leftSide.shiftState ||
        DenonMC6000MK2.rightSide.shiftState;
};

DenonMC6000MK2.getValue = function(key) {
    return engine.getValue(DenonMC6000MK2.group, key);
};

DenonMC6000MK2.setValue = function(key, value) {
    engine.setValue(DenonMC6000MK2.group, key, value);
};

DenonMC6000MK2.getJogDeltaValue = function(value) {
    if (0x00 === value) {
        return 0x00;
    } else {
        return value - DenonMC6000MK2.MIDI_JOG_DELTA_BIAS;
    }
};

DenonMC6000MK2.initValues = function() {
    DenonMC6000MK2.backupSampleRate = engine.getValue(DenonMC6000MK2.group, "samplerate");
    DenonMC6000MK2.setValue("samplerate", DenonMC6000MK2.SAMPLE_RATE);
    DenonMC6000MK2.backupNumDecks = DenonMC6000MK2.getValue("num_decks");
    DenonMC6000MK2.setValue("num_decks", DenonMC6000MK2.DECK_COUNT);
    DenonMC6000MK2.backupNumSamplers = DenonMC6000MK2.getValue("num_samplers");
    DenonMC6000MK2.setValue("num_samplers", DenonMC6000MK2.SIDE_COUNT * DenonMC6000MK2.SAMPLER_COUNT_PER_SIDE);
};

DenonMC6000MK2.connectLeds = function() {
    DenonMC6000MK2.deck1.cueMixLed = DenonMC6000MK2.connectLed(0x00, 0x45);
    DenonMC6000MK2.deck1.cueMixDimmerLed = DenonMC6000MK2.connectLed(0x00, 0x46);
    DenonMC6000MK2.deck1.leftEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x56);
    DenonMC6000MK2.deck1.rightEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x5A);
    DenonMC6000MK2.deck2.cueMixLed = DenonMC6000MK2.connectLed(0x00, 0x4B);
    DenonMC6000MK2.deck2.cueMixDimmerLed = DenonMC6000MK2.connectLed(0x00, 0x4C);
    DenonMC6000MK2.deck2.leftEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x57);
    DenonMC6000MK2.deck2.rightEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x5B);
    DenonMC6000MK2.deck3.cueMixLed = DenonMC6000MK2.connectLed(0x00, 0x51);
    DenonMC6000MK2.deck3.cueMixDimmerLed = DenonMC6000MK2.connectLed(0x00, 0x52);
    DenonMC6000MK2.deck3.leftEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x54);
    DenonMC6000MK2.deck3.rightEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x58);
    DenonMC6000MK2.deck4.cueMixLed = DenonMC6000MK2.connectLed(0x00, 0x57);
    DenonMC6000MK2.deck4.cueMixDimmerLed = DenonMC6000MK2.connectLed(0x00, 0x58);
    DenonMC6000MK2.deck4.leftEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x55);
    DenonMC6000MK2.deck4.rightEfxLed = DenonMC6000MK2.connectTriLed(0x00, 0x59);
    DenonMC6000MK2.leftSide.efxUnit.params[1].onLed = DenonMC6000MK2.connectTriLed(0x00, 0x5C);
    DenonMC6000MK2.leftSide.efxUnit.params[2].onLed = DenonMC6000MK2.connectTriLed(0x00, 0x5D);
    DenonMC6000MK2.leftSide.efxUnit.params[3].onLed = DenonMC6000MK2.connectTriLed(0x00, 0x5E);
    DenonMC6000MK2.leftSide.efxUnit.tapLed = DenonMC6000MK2.connectTriLed(0x00, 0x5F);
    DenonMC6000MK2.leftSide.filterLed = DenonMC6000MK2.connectLed(0x00, 0x65);
    DenonMC6000MK2.rightSide.efxUnit.params[1].onLed = DenonMC6000MK2.connectTriLed(0x00, 0x60);
    DenonMC6000MK2.rightSide.efxUnit.params[2].onLed = DenonMC6000MK2.connectTriLed(0x00, 0x61);
    DenonMC6000MK2.rightSide.efxUnit.params[3].onLed = DenonMC6000MK2.connectTriLed(0x00, 0x62);
    DenonMC6000MK2.rightSide.efxUnit.tapLed = DenonMC6000MK2.connectTriLed(0x00, 0x63);
    DenonMC6000MK2.rightSide.filterLed = DenonMC6000MK2.connectLed(0x00, 0x66);
    for (var index in DenonMC6000MK2.sides) {
        var side = DenonMC6000MK2.sides[index];
        side.connectLeds();
    }
};

DenonMC6000MK2.connectControls = function() {
    for (var index in DenonMC6000MK2.sides) {
        var side = DenonMC6000MK2.sides[index];
        side.connectControls();
        DenonMC6000MK2.connectControl(side.efxUnit.group, "enabled", side.efxUnit.ctrlEnabled);
        DenonMC6000MK2.connectControl(side.efxUnit.group, "mix", side.efxUnit.ctrlDryWetMix);
        for (var deckGroup in side.decksByGroup) {
            var deck = this.decksByGroup[deckGroup];
            DenonMC6000MK2.connectControl(deck.filterGroup, "enabled", side.ctrlFilterEnabled);
        }
    }
};

DenonMC6000MK2.restoreValues = function() {
    for (var index in DenonMC6000MK2.sides) {
        var side = DenonMC6000MK2.sides[index];
        side.restoreValues();
    }
    DenonMC6000MK2.setValue("num_samplers", DenonMC6000MK2.backupNumSamplers);
    DenonMC6000MK2.setValue("num_decks", DenonMC6000MK2.backupNumDecks);
    DenonMC6000MK2.setValue("samplerate", DenonMC6000MK2.backupSampleRate);
};


////////////////////////////////////////////////////////////////////////
// Mixxx scripting callback functions                                 //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.init = function(id, debug) {
    DenonMC6000MK2.id = id;
    DenonMC6000MK2.debug = debug;
    try {
        DenonMC6000MK2.initValues();
        DenonMC6000MK2.connectLeds();
        DenonMC6000MK2.connectControls();
        for (var index in DenonMC6000MK2.sides) {
            var side = DenonMC6000MK2.sides[index];
            side.initFilter();
            side.efxUnit.initEchoLoopParams();
            side.efxUnit.disable();
        }
    } catch (ex) {
        DenonMC6000MK2.logError("Exception during controller initialization: " + ex);
    }
};

DenonMC6000MK2.shutdown = function() {
    try {
        DenonMC6000MK2.disconnectControls();
        DenonMC6000MK2.disconnectLeds();
        DenonMC6000MK2.restoreValues();
    } catch (ex) {
        DenonMC6000MK2.logError("Exception during controller shutdown: " + ex);
    }
};


////////////////////////////////////////////////////////////////////////
// MIDI [Master] callback functions                                   //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.recvTrackSelectKnob = function(channel, control, value, status, group) {
    var knobDelta = DenonMC6000MK2.getKnobDelta(value);
    engine.setValue("[Playlist]", "SelectPrevTrack", 0 > knobDelta);
    engine.setValue("[Playlist]", "SelectNextTrack", 0 < knobDelta);
};

DenonMC6000MK2.recvXfaderContourKnob = function(channel, control, value, status, group) {
    script.crossfaderCurve(value);
};


////////////////////////////////////////////////////////////////////////
// MIDI [Channel<n>] callback functions                               //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.recvXfaderAssignLeftButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    if (isButtonPressed) {
        var deck = DenonMC6000MK2.getDeckByGroup(group);
        deck.assignXfaderLeft();
    }
};

DenonMC6000MK2.recvXfaderAssignThruButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    if (isButtonPressed) {
        var deck = DenonMC6000MK2.getDeckByGroup(group);
        deck.assignXfaderCenter();
    }
};

DenonMC6000MK2.recvXfaderAssignRightButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    if (isButtonPressed) {
        var deck = DenonMC6000MK2.getDeckByGroup(group);
        deck.assignXfaderRight();
    }
};

DenonMC6000MK2.recvShiftButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var side = DenonMC6000MK2.getSideByGroup(group);
    side.onShiftButton(isButtonPressed);
};

DenonMC6000MK2.recvPanelButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    // TODO
};

DenonMC6000MK2.recvListButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    // TODO
};

DenonMC6000MK2.recvViewButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    // TODO
};

DenonMC6000MK2.recvAreaButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    // TODO
};

DenonMC6000MK2.recvLoadButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onLoadButton(isButtonPressed);
};

DenonMC6000MK2.recvVinylButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onVinylButton(isButtonPressed);
};

DenonMC6000MK2.recvKeyLockButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onKeyLockButton(isButtonPressed);
};

DenonMC6000MK2.recvCueMixButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onCueMixButton(isButtonPressed);
};

DenonMC6000MK2.recvCueButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onCueButton(isButtonPressed);
};

DenonMC6000MK2.recvPlayButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onPlayButton(isButtonPressed);
};

DenonMC6000MK2.recvBendPlusButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onBendPlusButton(isButtonPressed);
};

DenonMC6000MK2.recvBendMinusButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onBendMinusButton(isButtonPressed);
};

DenonMC6000MK2.recvJogTouch = function(channel, control, value, status, group) {
    var isJogTouched = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.touchJog(isJogTouched);
};

DenonMC6000MK2.recvJogTouchVinyl = function(channel, control, value, status, group) {
    var isJogTouched = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.touchJog(isJogTouched);
};

DenonMC6000MK2.recvJogSpin = function(channel, control, value, status, group) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    var jogDelta = DenonMC6000MK2.getJogDeltaValue(value);
    deck.spinJog(jogDelta);
};

DenonMC6000MK2.recvJogSpinVinyl = function(channel, control, value, status, group) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    var jogDelta = DenonMC6000MK2.getJogDeltaValue(value);
    deck.spinJog(jogDelta);
};

DenonMC6000MK2.recvHotcue1Button = function(channel, control, value, status, group) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    deck.hotcues[1].onButton(isButtonPressed);
};

DenonMC6000MK2.recvHotcue2Button = function(channel, control, value, status, group) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    deck.hotcues[2].onButton(isButtonPressed);
};

DenonMC6000MK2.recvHotcue3Button = function(channel, control, value, status, group) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    deck.hotcues[3].onButton(isButtonPressed);
};

DenonMC6000MK2.recvHotcue4Button = function(channel, control, value, status, group) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    deck.hotcues[4].onButton(isButtonPressed);
};

DenonMC6000MK2.recvAutoLoopButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onAutoLoopButton(isButtonPressed);
};

DenonMC6000MK2.recvLoopInButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onLoopInButton(isButtonPressed);
};

DenonMC6000MK2.recvLoopOutButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onLoopOutButton(isButtonPressed);
};

DenonMC6000MK2.recvLoopCutMinusButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onLoopCutMinusButton(isButtonPressed);
};

DenonMC6000MK2.recvLoopCutPlusButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onLoopCutPlusButton(isButtonPressed);
};

DenonMC6000MK2.recvCensorButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onCensorButton(isButtonPressed);
};

DenonMC6000MK2.recvSamplerButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    var sampler = DenonMC6000MK2.getSamplerByGroup(group);
    sampler.onButton(isButtonPressed);
};

DenonMC6000MK2.leftSide.recvFilterButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.leftSide.onFilterButton(isButtonPressed);
};

DenonMC6000MK2.rightSide.recvFilterButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.rightSide.onFilterButton(isButtonPressed);
};

DenonMC6000MK2.leftSide.recvFilterKnob = function(channel, control, value, status, group) {
    DenonMC6000MK2.leftSide.onFilterMidiValue(value);
};

DenonMC6000MK2.rightSide.recvFilterKnob = function(channel, control, value, status, group) {
    DenonMC6000MK2.rightSide.onFilterMidiValue(value);
};

DenonMC6000MK2.leftSide.efxUnit.recvDeckButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.leftSide.efxUnit.onDeckButton(group, isButtonPressed);
};

DenonMC6000MK2.rightSide.efxUnit.recvDeckButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.rightSide.efxUnit.onDeckButton(group, isButtonPressed);
};

DenonMC6000MK2.leftSide.efxUnit.recvAdjust1Button = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.leftSide.efxUnit.onParamButton(1, isButtonPressed);
};

DenonMC6000MK2.rightSide.efxUnit.recvAdjust1Button = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.rightSide.efxUnit.onParamButton(1, isButtonPressed);
};

DenonMC6000MK2.leftSide.efxUnit.recvAdjust1Knob = function(channel, control, value, status, group) {
    DenonMC6000MK2.leftSide.efxUnit.onParamMidiValue(1, value);
};

DenonMC6000MK2.rightSide.efxUnit.recvAdjust1Knob = function(channel, control, value, status, group) {
    DenonMC6000MK2.rightSide.efxUnit.onParamMidiValue(1, value);
};

DenonMC6000MK2.leftSide.efxUnit.recvAdjust2Button = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.leftSide.efxUnit.onParamButton(2, isButtonPressed);
};

DenonMC6000MK2.rightSide.efxUnit.recvAdjust2Button = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.rightSide.efxUnit.onParamButton(2, isButtonPressed);
};

DenonMC6000MK2.leftSide.efxUnit.recvAdjust2Knob = function(channel, control, value, status, group) {
    DenonMC6000MK2.leftSide.efxUnit.onParamMidiValue(2, value);
};

DenonMC6000MK2.rightSide.efxUnit.recvAdjust2Knob = function(channel, control, value, status, group) {
    DenonMC6000MK2.rightSide.efxUnit.onParamMidiValue(2, value);
};

DenonMC6000MK2.leftSide.efxUnit.recvAdjust3Button = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.leftSide.efxUnit.onParamButton(3, isButtonPressed);
};

DenonMC6000MK2.rightSide.efxUnit.recvAdjust3Button = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.rightSide.efxUnit.onParamButton(3, isButtonPressed);
};

DenonMC6000MK2.leftSide.efxUnit.recvAdjust3Knob = function(channel, control, value, status, group) {
    DenonMC6000MK2.leftSide.efxUnit.onParamMidiValue(3, value);
};

DenonMC6000MK2.rightSide.efxUnit.recvAdjust3Knob = function(channel, control, value, status, group) {
    DenonMC6000MK2.rightSide.efxUnit.onParamMidiValue(3, value);
};

DenonMC6000MK2.leftSide.efxUnit.recvBeatsKnob = function(channel, control, value, status, group) {
    var delta = DenonMC6000MK2.getKnobDelta(value);
    DenonMC6000MK2.leftSide.efxUnit.onBeatsKnobDelta(delta);
};

DenonMC6000MK2.rightSide.efxUnit.recvBeatsKnob = function(channel, control, value, status, group) {
    var delta = DenonMC6000MK2.getKnobDelta(value);
    DenonMC6000MK2.rightSide.efxUnit.onBeatsKnobDelta(delta);
};

DenonMC6000MK2.leftSide.efxUnit.recvBeatsButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.leftSide.efxUnit.onBeatsButton(isButtonPressed);
};

DenonMC6000MK2.rightSide.efxUnit.recvBeatsButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.rightSide.efxUnit.onBeatsButton(isButtonPressed);
};

DenonMC6000MK2.leftSide.efxUnit.recvTapButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.leftSide.efxUnit.onEnableButton(isButtonPressed);
};

DenonMC6000MK2.rightSide.efxUnit.recvTapButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.rightSide.efxUnit.onEnableButton(isButtonPressed);
};

DenonMC6000MK2.leftSide.recvDeckButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.leftSide.onDeckButton(group, isButtonPressed);
};

DenonMC6000MK2.rightSide.recvDeckButton = function(channel, control, value, status, group) {
    var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
    DenonMC6000MK2.rightSide.onDeckButton(group, isButtonPressed);
};


////////////////////////////////////////////////////////////////////////
// Mixxx connected controls callback functions                        //
////////////////////////////////////////////////////////////////////////

// Deck controls

DenonMC6000MK2.ctrlKeyLock = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onKeyLockValue(value);
};

DenonMC6000MK2.ctrlSyncMode = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onSyncModeValue(value);
};

DenonMC6000MK2.ctrlSlipModeValue = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onSlipModeValue(value);
};

DenonMC6000MK2.ctrlCueMix = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onCueMixValue(value);
};

DenonMC6000MK2.ctrlTrackSamples = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onTrackSamplesValue(value);
};

DenonMC6000MK2.ctrlCueIndicator = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onCueIndicatorValue(value);
};

DenonMC6000MK2.ctrlPlayIndicator = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onPlayIndicatorValue(value);
};

DenonMC6000MK2.ctrlBpm = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.onBpmValue(value);
};

// Loop controls

DenonMC6000MK2.ctrlLoopStartPosition = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.updateLoopLeds();
};

DenonMC6000MK2.ctrlLoopEndPosition = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.updateLoopLeds();
};

DenonMC6000MK2.ctrlLoopEnabled = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.updateLoopLeds();
};

// Hotcue controls

DenonMC6000MK2.ctrlHotcue1 = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.hotcues[1].updateLeds();
};

DenonMC6000MK2.ctrlHotcue2 = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.hotcues[2].updateLeds();
};

DenonMC6000MK2.ctrlHotcue3 = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.hotcues[3].updateLeds();
};

DenonMC6000MK2.ctrlHotcue4 = function(value, group, control) {
    var deck = DenonMC6000MK2.getDeckByGroup(group);
    deck.hotcues[4].updateLeds();
};

// Sampler controls

DenonMC6000MK2.ctrlSampler = function(value, group, control) {
    var sampler = DenonMC6000MK2.getSamplerByGroup(group);
    sampler.updateLeds();
};

// Filter controls (shared between 2 decks on each side)

DenonMC6000MK2.leftSide.ctrlFilterEnabled = function(value, group, control) {
    DenonMC6000MK2.leftSide.filterLed.setStateBoolean(value);
};

DenonMC6000MK2.rightSide.ctrlFilterEnabled = function(value, group, control) {
    DenonMC6000MK2.rightSide.filterLed.setStateBoolean(value);
};

// Efx controls

DenonMC6000MK2.leftSide.efxUnit.ctrlDeck = function(value, group, control) {
    var deckGroup = control.substring(6, 16);
    var deck = DenonMC6000MK2.getDeckByGroup(deckGroup);
    deck.leftEfxLed.setStateBoolean(value);
};

DenonMC6000MK2.rightSide.efxUnit.ctrlDeck = function(value, group, control) {
    var deckGroup = control.substring(6, 16);
    var deck = DenonMC6000MK2.getDeckByGroup(deckGroup);
    deck.rightEfxLed.setStateBoolean(value);
};

DenonMC6000MK2.leftSide.efxUnit.ctrlEnabled = function(value, group, control) {
    DenonMC6000MK2.leftSide.efxUnit.onEnabled();
};

DenonMC6000MK2.rightSide.efxUnit.ctrlEnabled = function(value, group, control) {
    DenonMC6000MK2.rightSide.efxUnit.onEnabled();
};

DenonMC6000MK2.leftSide.efxUnit.ctrlDryWetMix = function(value, group, control) {
    DenonMC6000MK2.leftSide.efxUnit.onDryWetMix();
};

DenonMC6000MK2.rightSide.efxUnit.ctrlDryWetMix = function(value, group, control) {
    DenonMC6000MK2.rightSide.efxUnit.onDryWetMix();
};
