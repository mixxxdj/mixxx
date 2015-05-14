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
// Author: Uwe Klotz a/k/a tapir
// Revision: 2015-05-03
//
// Changelog:
// 2015-05-03 Minor bug fixes
//    - Fix scratch ramping
//    - Fix controller shutdown
// 2015-04-27 Improve jog wheel experience
//    - Unify jog wheel control for vinyl and non-vinyl mode
//    - Implement proper spin-back capability
//    - Fix key control
// 2015-04-26 New firmware v1003
//    - Change MIDI numbers of filter LEDs
//    - Update state of filter LEDs when applying the filter
//      (in addition to engine callback)
// 2015-01-25 Reconnect controls for version 1.12
//    - Connect filter effect to QuickEffectRack
//    - Connect channel EQs to EqualizerRack
//    - Connect FX Beats knob to musical key controls (next/prev/reset)
//    - Fine-tune jog wheel parameters
// 2015-01-16 Filter effect parameter update
//    - Rename filter effect parameter from "parameter" to "super1"
// 2014-08-20 Sync mapping with push-and-hold
//    - Use MIDI mapping for "sync_enabled" to support push-and-hold
//    - Change sync mode indicator LED:
//      MASTER=blinking, FOLLOWER=on, NONE=off
//    - Use setParameter() instead of setValue() for effect parameters
//    - Fix right effect unit enabled LED
// 2014-07-14 Simplify sync logic
//    - Control sync mainly through "sync_enabled" instead of trying
//      to select a specific "sync_mode". Mixxx already handles this
//      internally and we should not override it here.
//    - Generalize shift button handling for easier switching between
//      local and global Shift button handling
//    - Minor cleanup
// 2014-06-22 Scripting and effects refactoring
// 2014-06-20 Minor cleanup
// 2014-06-12 Basic effect chain control
//    - Effect chain selector (need to press shift as labeled)
//    - Efx3 button enables/disables the effect chain
//    - Efx3 knob for dry/wet control
//    - Efx2 knob for master parameter control ("metaknob")
//    - Minor cleanup
// 2014-06-11 Filters and initial effects support
//   - Effect units (ordered according to physical controller layout
//     from left to right):
//     - EffectUnit1: Left FX1
//     - EffectUnit2: Left Filter
//     - EffectUnit3: Right Filter
//     - EffectUnit4: Right FX2
//   - Filters are fully functional
//   - FX1 and FX2 can be assigned to decks
//   - Minor changes of VuMeter sensitivity
// 2014-05-02 Sampler modes + crossfader configuration
//   - Support 2 different sampler modes:
//     - Hold (the new default)
//     - Trigger
//     The default sampler mode can be changed by setting
//     DenonMC6000MK2.DEFAULT_SAMPLER_MODE appropriately.
//   - Assignment of crossfader to channels
//   - Modify contour of crossfader curve
//   - Delete unused booth level control
// 2014-04-30 Only 4+4 samplers (shared between decks)
//   - MIDI debugging revealed that the sampler controls
//     buttons/LEDs are sending/receiving on MIDI Ch0 (left side)
//     and MIDI Ch2 (right side) independent of the currently
//     selected deck.
//   - Automatically load selected track into empty deck
//     or sampler when pressing the cue/play or the sampler
//     button respectively
// 2014-04-29 Many MIDI mapping fixes
//   - Many MIDI mapping fixes, mainly for [Channel3/4]
//   - AutoDJ changes
//     - Shift + Cue triggers skip_next
//     - Shift + Play triggers fade_now
//   - Internal refactorings & cleanup
// 2014-04-28 Polishing
//   - Global shift state
//   - Slip mode fixes
//   - Looping improvements
//   - Cue button controls AutoDJ fade/skip
//   - Shift + play triggers stutter play
//   - Internal refactoring of hotcues
//   - Delete obsolete constants & triggers
//   - Minor changes
// 2014-04-27 Samplers and various improvements
//   - 16 samplers (4 per deck)
//   - Hotcue fixes
//   - Sync mode switching improvements
//   - Use cue mix LEDs as track load indicator
//   - Delete obsolete filter controls
// 2014-04-17 Track loading
//   - Solo cue mix when loading a track (can also triggered manually
//     by pressing Shift + Cue Mix)
//   - Slip mode fixes (still does not always work as expected)
// 2014-04-16 Looping
//   - Loops: Manual, beat and beatroll
//   - Censor: Use reverseroll instead of reverse
//   - Scratching: Don't modify keylock mode
// 2014-04-14 Direct mapping
//   - Map some controls and the VU meter LEDs directly
// 2014-04-13 Initial revision
//   - Just the basics
//   - No loops
//   - No samplers
//   - No filters and effects
//   - No microphone support
////////////////////////////////////////////////////////////////////////

function DenonMC6000MK2 () {}


////////////////////////////////////////////////////////////////////////
// Tunable constants                                                  //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.SAMPLER_MODE = {
	HOLD:    { value: 1, name: "Hold",    description: "Play sample from beginning to end while pressing the Sample button" },
	TRIGGER: { value: 2, name: "Trigger", description: "Play sample from beginning to end until stopped by pressing Shift + Sample button" }
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

DenonMC6000MK2.LEFT_EFX_GROUP = "[EffectRack1_EffectUnit1]";
DenonMC6000MK2.RIGHT_EFX_GROUP = "[EffectRack1_EffectUnit2]";

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
// Global variables                                                   //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.shiftState = false;


////////////////////////////////////////////////////////////////////////
// Logging functions                                                  //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.logDebug = function (msg) {
	if (DenonMC6000MK2.debug) {
		print("[" + DenonMC6000MK2.id + " DEBUG] " + msg);
	}
};

DenonMC6000MK2.logInfo = function (msg) {
	print("[" + DenonMC6000MK2.id + " INFO] " + msg);
};

DenonMC6000MK2.logWarning = function (msg) {
	print("[" + DenonMC6000MK2.id + " WARNING] " + msg);
};

DenonMC6000MK2.logError = function (msg) {
	print("[" + DenonMC6000MK2.id + " ERROR] " + msg);
};


////////////////////////////////////////////////////////////////////////
// Buttons                                                            //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.MIDI_BUTTON_ON = 0x40;
DenonMC6000MK2.MIDI_BUTTON_OFF = 0x00;

DenonMC6000MK2.isButtonPressed = function (midiValue) {
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

DenonMC6000MK2.toggleValue = function (group, key) {
	engine.setValue(group, key, !engine.getValue(group, key));
};


////////////////////////////////////////////////////////////////////////
// Knobs                                                              //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.MIDI_KNOB_INC = 0x00;
DenonMC6000MK2.MIDI_KNOB_DEC = 0x7F;

DenonMC6000MK2.getKnobDelta = function (midiValue) {
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

DenonMC6000MK2.LedState = function (midiCtrl) {
	this.midiCtrl = midiCtrl;
};

DenonMC6000MK2.LED_ON = new DenonMC6000MK2.LedState(DenonMC6000MK2.MIDI_LED_ON);
DenonMC6000MK2.LED_OFF = new DenonMC6000MK2.LedState(DenonMC6000MK2.MIDI_LED_OFF);

DenonMC6000MK2.Led = function (midiChannel, midiValue) {
	this.midiChannel = midiChannel;
	this.midiValue = midiValue;
	this.state = undefined;
};

DenonMC6000MK2.Led.prototype.setState = function (ledState) {
	this.state = ledState;
	this.trigger();
};

DenonMC6000MK2.Led.prototype.setStateBoolean = function (booleanValue) {
	if (booleanValue) {
		this.setState(DenonMC6000MK2.LED_ON);
	} else {
		this.setState(DenonMC6000MK2.LED_OFF);
	}
};

DenonMC6000MK2.Led.prototype.trigger = function () {
	midi.sendShortMsg(0xB0 + this.midiChannel, this.state.midiCtrl, this.midiValue);
};

DenonMC6000MK2.Led.prototype.reset = function () {
	this.setStateBoolean(false);
};

DenonMC6000MK2.MIDI_TRI_LED_ON = 0x4A;
DenonMC6000MK2.MIDI_TRI_LED_OFF = 0x4B;
DenonMC6000MK2.MIDI_TRI_LED_BLINK = 0x4C;

DenonMC6000MK2.TriLedState = function (midiCtrl) {
	this.midiCtrl = midiCtrl;
};

DenonMC6000MK2.TRI_LED_ON = new DenonMC6000MK2.TriLedState(DenonMC6000MK2.MIDI_TRI_LED_ON);
DenonMC6000MK2.TRI_LED_OFF = new DenonMC6000MK2.TriLedState(DenonMC6000MK2.MIDI_TRI_LED_OFF);
DenonMC6000MK2.TRI_LED_BLINK = new DenonMC6000MK2.TriLedState(DenonMC6000MK2.MIDI_TRI_LED_BLINK);

DenonMC6000MK2.TriLed = function (midiChannel, midiValue) {
	this.midiChannel = midiChannel;
	this.midiValue = midiValue;
	this.state = undefined;
};

DenonMC6000MK2.TriLed.prototype.setTriState = function (ledState) {
	this.state = ledState;
	this.trigger();
};

DenonMC6000MK2.TriLed.prototype.setStateBoolean = function (booleanValue) {
	if (booleanValue) {
		this.setTriState(DenonMC6000MK2.TRI_LED_ON);
	} else {
		this.setTriState(DenonMC6000MK2.TRI_LED_OFF);
	}
};

DenonMC6000MK2.TriLed.prototype.trigger = function () {
	midi.sendShortMsg(0xB0 + this.midiChannel, this.state.midiCtrl, this.midiValue);
};

DenonMC6000MK2.TriLed.prototype.reset = function () {
	this.setStateBoolean(false);
};

DenonMC6000MK2.connectedLeds = [];

DenonMC6000MK2.connectLed = function (midiChannel, midiValue) {
	var led = new DenonMC6000MK2.Led(midiChannel, midiValue);
	led.reset();
	DenonMC6000MK2.connectedLeds.push(led);
	return led;
};

DenonMC6000MK2.connectTriLed = function (midiChannel, midiValue) {
	var led = new DenonMC6000MK2.TriLed(midiChannel, midiValue);
	led.reset();
	DenonMC6000MK2.connectedLeds.push(led);
	return led;
};

DenonMC6000MK2.disconnectLeds = function () {
	for (var index in DenonMC6000MK2.connectedLeds) {
		DenonMC6000MK2.connectedLeds[index].reset();
	}
	DenonMC6000MK2.connectedLeds = [];
};


////////////////////////////////////////////////////////////////////////
// Controls                                                           //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.Control = function (group, ctrl, func) {
	this.group = group;
	this.ctrl = ctrl;
	this.func = func;
	this.isConnected = false;
};

DenonMC6000MK2.Control.prototype.connect = function () {
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

DenonMC6000MK2.Control.prototype.disconnect = function () {
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

DenonMC6000MK2.Control.prototype.trigger = function () {
	engine.trigger(this.group, this.ctrl);
};

DenonMC6000MK2.connectedControls = [];

DenonMC6000MK2.connectControl = function (group, ctrl, func) {
	var control = new DenonMC6000MK2.Control(group, ctrl, func);
	if (control.connect()) {
		DenonMC6000MK2.connectedControls.push(control);
		return control;
	} else {
		return undefined;
	}
};

DenonMC6000MK2.disconnectControls = function () {
	for (var index in DenonMC6000MK2.connectedControls) {
		DenonMC6000MK2.connectedControls[index].disconnect();
	}
	DenonMC6000MK2.connectedControls = [];
};


////////////////////////////////////////////////////////////////////////
// Hotcues                                                            //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.Hotcue = function (deck, number, midiLedValue, midiDimmerLedValue) {
	this.deck = deck;
	this.number = number;
	this.ctrlPrefix = "hotcue_" + number;
	this.midiLedValue = midiLedValue;
	this.midiDimmerLedValue = midiDimmerLedValue;
};

DenonMC6000MK2.Hotcue.prototype.connectControls = function (callbackFunc) {
	this.deck.connectControl(this.ctrlPrefix + "_enabled", callbackFunc);
};

DenonMC6000MK2.Hotcue.prototype.isEnabled = function () {
	return this.deck.getValue(this.ctrlPrefix + "_enabled");
};

DenonMC6000MK2.Hotcue.prototype.onButton = function (isButtonPressed) {
	if (isButtonPressed && this.deck.getShiftState()) {
		this.deck.setValue(this.ctrlPrefix + "_clear", true);
	} else {
		this.deck.setValue(this.ctrlPrefix + "_activate", isButtonPressed);
	}
};

DenonMC6000MK2.Hotcue.prototype.connectLeds = function () {
	this.led = this.deck.connectTriLed(this.midiLedValue);
	this.dimmerLed = this.deck.connectTriLed(this.midiDimmerLedValue);
};

DenonMC6000MK2.Hotcue.prototype.updateLeds = function () {
	this.led.setStateBoolean(this.isEnabled());
};


////////////////////////////////////////////////////////////////////////
// Samplers                                                           //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.samplerCount = 0;

DenonMC6000MK2.samplersByGroup = {};

DenonMC6000MK2.getSamplerByGroup = function (group) {
	var sampler = DenonMC6000MK2.samplersByGroup[group];
	if (undefined === sampler) {
		DenonMC6000MK2.logError("No sampler found for " + group);
	}
	return sampler;
};

DenonMC6000MK2.Sampler = function (side, midiChannel, midiLedValue, midiDimmerLedValue) {
	this.side = side;
	this.number = ++DenonMC6000MK2.samplerCount;
	this.group = "[Sampler" + this.number + "]";
	this.midiChannel = midiChannel;
	this.midiLedValue = midiLedValue;
	this.midiDimmerLedValue = midiDimmerLedValue;
	DenonMC6000MK2.samplersByGroup[this.group] = this;
};

DenonMC6000MK2.Sampler.prototype.isTrackLoaded = function () {
	return DenonMC6000MK2.isTrackLoaded(engine.getValue(this.group, "track_samples"));
};

DenonMC6000MK2.Sampler.prototype.loadSelectedTrack = function () {
	engine.setValue(this.group, "LoadSelectedTrack", true);
};

DenonMC6000MK2.Sampler.prototype.isPlaying = function () {
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

DenonMC6000MK2.Sampler.prototype.connectLeds = function () {
	this.led = DenonMC6000MK2.connectTriLed(this.midiChannel, this.midiLedValue);
	this.dimmerLed = DenonMC6000MK2.connectTriLed(this.midiChannel, this.midiDimmerLedValue);
};

DenonMC6000MK2.Sampler.prototype.updateLeds = function () {
	if (this.isPlaying()) {
		this.led.setStateBoolean(true);
	} else {
		this.dimmerLed.setStateBoolean(this.isTrackLoaded());
	}
};

DenonMC6000MK2.Sampler.prototype.connectControls = function () {
	DenonMC6000MK2.connectControl(this.group, "play", DenonMC6000MK2.onSamplerValueCB);
	DenonMC6000MK2.connectControl(this.group, "track_samples", DenonMC6000MK2.onSamplerValueCB);
};


////////////////////////////////////////////////////////////////////////
// Decks                                                              //
////////////////////////////////////////////////////////////////////////

/* Management */

DenonMC6000MK2.decksByGroup = {};

DenonMC6000MK2.getDeckByGroup = function (group) {
	var deck = DenonMC6000MK2.decksByGroup[group];
	if (undefined === deck) {
		DenonMC6000MK2.logError("No deck found for " + group);
	}
	return deck;
};

/* Constructor */

DenonMC6000MK2.Deck = function (number, midiChannel) {
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

DenonMC6000MK2.Deck.prototype.getShiftState = function (group) {
	return this.side.getShiftState();
};

/* Values & Parameters */

DenonMC6000MK2.Deck.prototype.getValue = function (key) {
	return engine.getValue(this.group, key);
};

DenonMC6000MK2.Deck.prototype.setValue = function (key, value) {
	engine.setValue(this.group, key, value);
};

DenonMC6000MK2.Deck.prototype.toggleValue = function (key) {
	this.setValue(key, !this.getValue(key));
};

DenonMC6000MK2.Deck.prototype.setParameter = function (key, param) {
	engine.setParameter(this.group, key, param);
};

DenonMC6000MK2.Deck.prototype.triggerValue = function (key) {
	engine.trigger(this.group, key);
};

/* Xfader */

DenonMC6000MK2.Deck.prototype.assignXfaderLeft = function () {
	this.setValue("orientation", DenonMC6000MK2.MIXXX_XFADER_LEFT);
};

DenonMC6000MK2.Deck.prototype.assignXfaderCenter = function () {
	this.setValue("orientation", DenonMC6000MK2.MIXXX_XFADER_CENTER);
};

DenonMC6000MK2.Deck.prototype.assignXfaderRight = function () {
	this.setValue("orientation", DenonMC6000MK2.MIXXX_XFADER_RIGHT);
};

/* Tracks */

DenonMC6000MK2.Deck.prototype.loadSelectedTrack = function () {
	this.setValue("LoadSelectedTrack", true);
	this.setCueMixSolo(); // just for convenience ;)
};

DenonMC6000MK2.Deck.prototype.unloadTrack = function () {
	this.setValue("eject", true);
};

/* Key Lock Mode */

DenonMC6000MK2.Deck.prototype.onKeyLockButtonPressed = function () {
	this.toggleValue("keylock");
};

DenonMC6000MK2.Deck.prototype.enableKeyLock = function () {
	this.setValue("keylock", true);
};

DenonMC6000MK2.Deck.prototype.disableKeyLock = function () {
	this.setValue("keylock", false);
};

DenonMC6000MK2.Deck.prototype.onKeyLockValue = function (value) {
	this.keyLockLed.setStateBoolean(value);
};

/* Key Control */

DenonMC6000MK2.Deck.prototype.resetKey = function () {
	this.setValue("reset_key", true);
};

/* Sync Mode */

DenonMC6000MK2.Deck.prototype.disableSyncMode = function () {
	this.setValue("sync_mode", DenonMC6000MK2.MIXXX_SYNC_NONE);
};

DenonMC6000MK2.Deck.prototype.onSyncModeValue = function (value) {
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

DenonMC6000MK2.Deck.prototype.setCueMixSolo = function () {
	for (var deckGroup in DenonMC6000MK2.decksByGroup) {
		var deck = DenonMC6000MK2.getDeckByGroup(deckGroup);
		deck.setValue("pfl", this === deck);
	}
};

DenonMC6000MK2.Deck.prototype.onCueMixButtonPressed = function () {
	if (this.getShiftState()) {
		this.setCueMixSolo();
	} else {
		this.toggleValue("pfl");
	}
};

DenonMC6000MK2.Deck.prototype.updateCueMixValue = function (pflValue, isTrackLoaded) {
	if (pflValue) {
		this.cueMixLed.setStateBoolean(pflValue);
	} else {
		this.cueMixDimmerLed.setStateBoolean(isTrackLoaded);
	}
};

DenonMC6000MK2.Deck.prototype.onCueMixValue = function (pflValue) {
	this.updateCueMixValue(pflValue, this.isTrackLoaded());
};

/* Track Load */

DenonMC6000MK2.isTrackLoaded = function (trackSamples) {
	return 0 < trackSamples;
};

DenonMC6000MK2.Deck.prototype.isTrackLoaded = function () {
	return DenonMC6000MK2.isTrackLoaded(this.getValue("track_samples"));
};

DenonMC6000MK2.Deck.prototype.onTrackSamplesValue = function (value) {
	this.updateCueMixValue(this.getValue("pfl"), DenonMC6000MK2.isTrackLoaded(value));
};

/* Cue & Play */

DenonMC6000MK2.Deck.prototype.onCueButton = function (isButtonPressed) {
	if (this.isTrackLoaded()) {
		if (this.getShiftState()) {
			if (isButtonPressed) {
				if (engine.getValue("[AutoDJ]", "enabled")) {
					engine.setValue("[AutoDJ]", "skip_next", true);
				} else {
					if (this.isPlaying()) {
						// move cue point
						this.setValue("cue_set", true);
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
			}
		} else {
			this.setValue("cue_default", isButtonPressed);
		}
	} else {
		if (isButtonPressed) {
			this.loadSelectedTrack();
		}
	}
};

DenonMC6000MK2.Deck.prototype.onCueIndicatorValue = function (value) {
	this.cueLed.setStateBoolean(value);
};

DenonMC6000MK2.Deck.prototype.onPlayButtonPressed = function () {
	if (this.isTrackLoaded()) {
		if (this.getShiftState()) {
			if (engine.getValue("[AutoDJ]", "enabled")) {
				engine.setValue("[AutoDJ]", "fade_now", true);
			} else {
				this.setValue("play_stutter", true);
			}
		} else {
			this.toggleValue("play");
		}
	} else {
		this.loadSelectedTrack();
	}
};

DenonMC6000MK2.Deck.prototype.isPlaying = function () {
	return this.getValue("play");
};

DenonMC6000MK2.Deck.prototype.onPlayIndicatorValue = function (value) {
	this.playLed.setStateBoolean(value);
};

/* Pitch Bend / Track Search */

DenonMC6000MK2.Deck.prototype.onBendPlusButton = function (isButtonPressed) {
	if (this.getShiftState()) {
		this.setValue("fwd", isButtonPressed);
	} else {
		this.setValue("fwd", false);
		if (this.isPlaying()) {
			this.setValue("rate_temp_up", isButtonPressed);
		}
	}
};

DenonMC6000MK2.Deck.prototype.onBendMinusButton = function (isButtonPressed) {
	if (this.getShiftState()) {
		this.setValue("back", isButtonPressed);
	} else {
		this.setValue("back", false);
		if (this.isPlaying()) {
			this.setValue("rate_temp_down", isButtonPressed);
		}
	}
};


/* Censor / Slip Mode */

DenonMC6000MK2.Deck.prototype.onCensorButton = function (isButtonPressed) {
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

DenonMC6000MK2.Deck.prototype.onSlipModeValue = function (value) {
	this.slipModeLed.setStateBoolean(value);
};

/* Vinyl Mode (Scratching) */

DenonMC6000MK2.Deck.prototype.onVinylModeValue = function () {
	this.vinylModeLed.setStateBoolean(this.vinylMode);
};

DenonMC6000MK2.Deck.prototype.enableScratching = function () {
	engine.scratchEnable(this.number,
		DenonMC6000MK2.JOG_RESOLUTION,
		DenonMC6000MK2.JOG_SCRATCH_RPM,
		DenonMC6000MK2.JOG_SCRATCH_ALPHA,
		DenonMC6000MK2.JOG_SCRATCH_BETA,
		DenonMC6000MK2.JOG_SCRATCH_RAMP);
};

DenonMC6000MK2.Deck.prototype.disableScratching = function () {
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

DenonMC6000MK2.Deck.prototype.onScratchingTimeout = function () {
	this.scratchTimer = 0;
	this.disableScratching();
};

DenonMC6000MK2.onScratchingTimeoutDeck1 = function () {
	DenonMC6000MK2.getDeckByGroup("[Channel1]").onScratchingTimeout();
};

DenonMC6000MK2.onScratchingTimeoutDeck2 = function () {
	DenonMC6000MK2.getDeckByGroup("[Channel2]").onScratchingTimeout();
};

DenonMC6000MK2.onScratchingTimeoutDeck3 = function () {
	DenonMC6000MK2.getDeckByGroup("[Channel3]").onScratchingTimeout();
};

DenonMC6000MK2.onScratchingTimeoutDeck4 = function () {
	DenonMC6000MK2.getDeckByGroup("[Channel4]").onScratchingTimeout();
};

DenonMC6000MK2.Deck.prototype.updateVinylMode = function () {
	if (this.vinylMode && this.jogTouchState) {
		this.enableScratching();
	} else {
		this.disableScratching();
	}
	this.onVinylModeValue();
};

DenonMC6000MK2.Deck.prototype.setVinylMode = function (vinylMode) {
	this.vinylMode = vinylMode;
	this.updateVinylMode();
};

DenonMC6000MK2.Deck.prototype.onVinylButtonPressed = function () {
	this.setVinylMode(!this.vinylMode);
};

DenonMC6000MK2.Deck.prototype.enableVinylMode = function () {
	this.setVinylMode(true);
};

DenonMC6000MK2.Deck.prototype.disableVinylMode = function () {
	this.setVinylMode(false);
};

/* Jog Wheel */

DenonMC6000MK2.Deck.prototype.touchJog = function (isJogTouched) {
	this.jogTouchState = isJogTouched;
	this.updateVinylMode();
};

DenonMC6000MK2.Deck.prototype.spinJog = function (jogDelta) {
	if (this.getShiftState() && !this.isPlaying()) {
		// fast track seek
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

DenonMC6000MK2.Deck.prototype.applyFilter = function () {
	engine.setValue(this.filterGroup, "enabled", this.side.filterEnabled);
	engine.setValue(this.filterGroup, "super1", this.side.filterParamValue);
};

/* Loops */

DenonMC6000MK2.Deck.prototype.hasLoopStart = function () {
	return DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED !== this.getValue("loop_start_position");
};

DenonMC6000MK2.Deck.prototype.hasLoopEnd = function () {
	return DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED !== this.getValue("loop_end_position");
};

DenonMC6000MK2.Deck.prototype.hasLoop = function () {
	return this.hasLoopStart() && this.hasLoopEnd();
};

DenonMC6000MK2.Deck.prototype.deleteLoopStart = function () {
	this.setValue("loop_start_position", DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED);
};

DenonMC6000MK2.Deck.prototype.deleteLoopEnd = function () {
	this.setValue("loop_end_position", DenonMC6000MK2.MIXXX_LOOP_POSITION_UNDEFINED);
};

DenonMC6000MK2.Deck.prototype.deleteLoop = function () {
	// loop end has to be deleted before loop start
	this.deleteLoopEnd();
	this.deleteLoopStart();
};

DenonMC6000MK2.Deck.prototype.toggleLoop = function () {
	this.setValue("reloop_exit", true);
};

DenonMC6000MK2.Deck.prototype.onAutoLoopButtonPressed = function () {
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
};

DenonMC6000MK2.Deck.prototype.onLoopInButtonPressed = function () {
	if (this.getShiftState()) {
		this.deleteLoop(); // both start & end
	} else {
		this.setValue("loop_in", true);
	}
};

DenonMC6000MK2.Deck.prototype.onLoopOutButtonPressed = function () {
	if (this.getShiftState()) {
		this.deleteLoopEnd();
	} else {
		this.setValue("loop_out", true);
	}
};

DenonMC6000MK2.Deck.prototype.onLoopCutMinusButtonPressed = function () {
	this.setValue("loop_halve", true);
};

DenonMC6000MK2.Deck.prototype.onLoopCutPlusButtonPressed = function () {
	this.setValue("loop_double", true);
};

DenonMC6000MK2.Deck.prototype.updateLoopLeds = function (value) {
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

DenonMC6000MK2.Deck.prototype.connectLed = function (midiValue) {
	return DenonMC6000MK2.connectLed(this.midiChannel, midiValue);
};

DenonMC6000MK2.Deck.prototype.connectTriLed = function (midiValue) {
	return DenonMC6000MK2.connectTriLed(this.midiChannel, midiValue);
};

/* Startup */

DenonMC6000MK2.Deck.prototype.connectLeds = function () {
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

DenonMC6000MK2.Deck.prototype.connectControl = function (ctrl, func) {
	return DenonMC6000MK2.connectControl(this.group, ctrl, func);
};

DenonMC6000MK2.Deck.prototype.connectControls = function () {
	this.connectControl("cue_indicator", DenonMC6000MK2.onCueIndicatorValueCB);
	this.connectControl("play_indicator", DenonMC6000MK2.onPlayIndicatorValueCB);
	this.connectControl("sync_mode", DenonMC6000MK2.onSyncModeValueCB);
	this.connectControl("keylock", DenonMC6000MK2.onKeyLockValueCB);
	this.connectControl("pfl", DenonMC6000MK2.onCueMixValueCB);
	this.connectControl("track_samples", DenonMC6000MK2.onTrackSamplesValueCB);
	this.connectControl("slip_enabled", DenonMC6000MK2.onSlipModeValueCB);
	this.connectControl("loop_enabled", DenonMC6000MK2.onLoopEnabledValueCB);
	this.connectControl("loop_start_position", DenonMC6000MK2.onLoopStartPositionValueCB);
	this.connectControl("loop_end_position", DenonMC6000MK2.onLoopEndPositionValueCB);
	DenonMC6000MK2.connectControl(DenonMC6000MK2.leftSide.efxGroup, "group_" + this.group + "_enable", DenonMC6000MK2.leftSide.onEfxAssignValueCB);
	DenonMC6000MK2.connectControl(DenonMC6000MK2.rightSide.efxGroup, "group_" + this.group + "_enable", DenonMC6000MK2.rightSide.onEfxAssignValueCB);
	this.hotcues[1].connectControls(DenonMC6000MK2.onHotcue1ValueCB);
	this.hotcues[2].connectControls(DenonMC6000MK2.onHotcue2ValueCB);
	this.hotcues[3].connectControls(DenonMC6000MK2.onHotcue3ValueCB);
	this.hotcues[4].connectControls(DenonMC6000MK2.onHotcue4ValueCB);
	// default settings
	this.enableKeyLock();
	this.enableVinylMode();
	this.disableSyncMode();
};

/* Shutdown */

DenonMC6000MK2.Deck.prototype.restoreValues = function () {
	this.setValue("rate_dir", this.rateDirBackup);
};


////////////////////////////////////////////////////////////////////////
// Sides                                                              //
////////////////////////////////////////////////////////////////////////

/* Management */

DenonMC6000MK2.sidesByGroup = {};

DenonMC6000MK2.getSideByGroup = function (group) {
	var side = DenonMC6000MK2.sidesByGroup[group];
	if (undefined === side) {
		DenonMC6000MK2.logError("No side found for " + group);
	}
	return side;
};

/* Constructor */

DenonMC6000MK2.Side = function (decks, efxGroup, samplerMidiChannel) {
	this.decksByGroup = {};
	for (var deckIndex in decks) {
		var deck = decks[deckIndex];
		deck.side = this;
		this.decksByGroup[deck.group] = deck;
		DenonMC6000MK2.sidesByGroup[deck.group] = this;
	}
	this.activeDeck = decks[0];
	this.shiftState = false;
	this.efxGroup = efxGroup;
	DenonMC6000MK2.sidesByGroup[efxGroup] = this;
	this.efxValues = [];
	this.efxValues[1] = undefined;
	this.efxValues[2] = undefined;
	this.efxValues[3] = undefined;
	this.samplers = [];
	this.samplers[1] = new DenonMC6000MK2.Sampler(this, samplerMidiChannel, 0x19, 0x1A);
	this.samplers[2] = new DenonMC6000MK2.Sampler(this, samplerMidiChannel, 0x1B, 0x1C);
	this.samplers[3] = new DenonMC6000MK2.Sampler(this, samplerMidiChannel, 0x1D, 0x1F);
	this.samplers[4] = new DenonMC6000MK2.Sampler(this, samplerMidiChannel, 0x20, 0x21);
	this.initFilter();
};

/* Shift */

DenonMC6000MK2.Side.prototype.getShiftState = function (group) {
	if (DenonMC6000MK2.GLOBAL_SHIFT_STATE) {
		return DenonMC6000MK2.shiftState;
	} else {
		return this.shiftState;
	}
};

/* Decks */

DenonMC6000MK2.Side.prototype.getDeckByGroup = function (group) {
	var deck = this.decksByGroup[group];
	if (undefined === deck) {
		DenonMC6000MK2.logError("No deck found for " + group);
	}
	return deck;
};

DenonMC6000MK2.Side.prototype.activateDeckByGroup = function (group) {
	this.activeDeck = this.getDeckByGroup(group);
};

/* Startup */

DenonMC6000MK2.Side.prototype.connectLeds = function () {
	for (var deckGroup in this.decksByGroup) {
		var deck = this.decksByGroup[deckGroup];
		deck.connectLeds();
	}
	for (var samplerIndex in this.samplers) {
		this.samplers[samplerIndex].connectLeds();
	}
};

DenonMC6000MK2.Side.prototype.connectControls = function () {
	for (var deckGroup in this.decksByGroup) {
		var deck = this.decksByGroup[deckGroup];
		deck.connectControls();
	}
	for (var samplerIndex in this.samplers) {
		this.samplers[samplerIndex].connectControls();
	}
	engine.setValue(this.efxGroup, "enabled", false);
};

/* Shutdown */

DenonMC6000MK2.Side.prototype.restoreValues = function () {
	for (var group in this.decksByGroup) {
		var deck = this.decksByGroup[group];
		deck.restoreValues();
	}
};

DenonMC6000MK2.Side.prototype.onEfxButton = function (index, isButtonPressed) {
	switch (index) {
		case 1:
			// TODO
			break;
		case 2:
			// TODO
			break;
		case 3:
			DenonMC6000MK2.toggleValue(this.efxGroup, "enabled");
			break;
	}
};

DenonMC6000MK2.Side.prototype.onEfxParamValue = function (index, value) {
	this.efxValues[index] = value;
	switch (index) {
		case 1:
			// TODO
			break;
		case 2:
			engine.setParameter(this.efxGroup, "super1", script.absoluteLin(value, 0.0, 1.0));
			break;
		case 3:
			engine.setParameter(this.efxGroup, "mix", script.absoluteLin(value, 0.0, 1.0));
			break;
	}
};

DenonMC6000MK2.Side.prototype.onEfxBeatsKnobDelta = function (delta) {
	if (this.getShiftState()) {
		engine.setValue(this.efxGroup, "chain_selector", delta);
	} else {
		var newKey = this.activeDeck.getValue("key") + delta;
		this.activeDeck.setValue("key", newKey);
	}
};

DenonMC6000MK2.Side.prototype.onEfxTapButton = function (isButtonPressed) {
	if (isButtonPressed) {
		// TODO
	}
};

/* Filter */

DenonMC6000MK2.Side.prototype.initFilter = function () {
	this.filterEnabled = true;
	this.filterParamValue = 0.5; // centered
	this.filterLed = undefined;
	this.applyFilter();
};

DenonMC6000MK2.Side.prototype.applyFilter = function () {
	for (var group in this.decksByGroup) {
		var deck = this.decksByGroup[group];
		deck.applyFilter();
	}
	// Workaround for non-functional callback
	if (undefined !== this.filterLed) {
		this.filterLed.setStateBoolean(this.filterEnabled);
	}
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
DenonMC6000MK2.leftDecks = [ DenonMC6000MK2.deck1, DenonMC6000MK2.deck3 ];
DenonMC6000MK2.leftSide = new DenonMC6000MK2.Side(DenonMC6000MK2.leftDecks, DenonMC6000MK2.LEFT_EFX_GROUP, DenonMC6000MK2.MIDI_CH0);

// right side
DenonMC6000MK2.deck2 = new DenonMC6000MK2.Deck(2, DenonMC6000MK2.MIDI_CH2);
DenonMC6000MK2.deck4 = new DenonMC6000MK2.Deck(4, DenonMC6000MK2.MIDI_CH3);
DenonMC6000MK2.rightDecks = [ DenonMC6000MK2.deck2, DenonMC6000MK2.deck4 ];
DenonMC6000MK2.rightSide = new DenonMC6000MK2.Side(DenonMC6000MK2.rightDecks, DenonMC6000MK2.RIGHT_EFX_GROUP, DenonMC6000MK2.MIDI_CH2);

DenonMC6000MK2.getValue = function (key) {
	return engine.getValue(DenonMC6000MK2.group, key);
};

DenonMC6000MK2.setValue = function (key, value) {
	engine.setValue(DenonMC6000MK2.group, key, value);
};

DenonMC6000MK2.getJogDeltaValue = function (value) {
	if (0x00 === value) {
		return 0x00;
	} else {
		return value - DenonMC6000MK2.MIDI_JOG_DELTA_BIAS;
	}
};

DenonMC6000MK2.initValues = function () {
	DenonMC6000MK2.backupSampleRate = engine.getValue(DenonMC6000MK2.group, "samplerate");
	DenonMC6000MK2.setValue("samplerate", DenonMC6000MK2.SAMPLE_RATE);
	DenonMC6000MK2.backupNumDecks = DenonMC6000MK2.getValue("num_decks");
	DenonMC6000MK2.setValue("num_decks", DenonMC6000MK2.DECK_COUNT);
	DenonMC6000MK2.backupNumSamplers = DenonMC6000MK2.getValue("num_samplers");
	DenonMC6000MK2.setValue("num_samplers", DenonMC6000MK2.SIDE_COUNT * DenonMC6000MK2.SAMPLER_COUNT_PER_SIDE);
};

DenonMC6000MK2.connectLeds = function () {
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
	DenonMC6000MK2.leftSide.efx1Led = DenonMC6000MK2.connectTriLed(0x00, 0x5C);
	DenonMC6000MK2.leftSide.efx2Led = DenonMC6000MK2.connectTriLed(0x00, 0x5D);
	DenonMC6000MK2.leftSide.efx3Led = DenonMC6000MK2.connectTriLed(0x00, 0x5E);
	DenonMC6000MK2.leftSide.efxTapLed = DenonMC6000MK2.connectTriLed(0x00, 0x5F);
	DenonMC6000MK2.leftSide.filterLed = DenonMC6000MK2.connectLed(0x00, 0x65);
	DenonMC6000MK2.leftSide.connectLeds();
	DenonMC6000MK2.rightSide.efx1Led = DenonMC6000MK2.connectTriLed(0x00, 0x60);
	DenonMC6000MK2.rightSide.efx2Led = DenonMC6000MK2.connectTriLed(0x00, 0x61);
	DenonMC6000MK2.rightSide.efx3Led = DenonMC6000MK2.connectTriLed(0x00, 0x62);
	DenonMC6000MK2.rightSide.efxTapLed = DenonMC6000MK2.connectTriLed(0x00, 0x63);
	DenonMC6000MK2.rightSide.filterLed = DenonMC6000MK2.connectLed(0x00, 0x66);
	DenonMC6000MK2.rightSide.connectLeds();
};

DenonMC6000MK2.connectControls = function () {
	var deck, deckGroup;
	DenonMC6000MK2.leftSide.connectControls();
	//DenonMC6000MK2.connectControl("", "", DenonMC6000MK2.leftSide.onEfx1EnabledValueCB);
	//DenonMC6000MK2.connectControl("", "", DenonMC6000MK2.leftSide.onEfx2EnabledValueCB);
	DenonMC6000MK2.connectControl(DenonMC6000MK2.leftSide.efxGroup, "enabled", DenonMC6000MK2.leftSide.onEfx3EnabledValueCB);
	for (deckGroup in DenonMC6000MK2.leftSide.decksByGroup) {
		deck = this.decksByGroup[deckGroup];
		DenonMC6000MK2.connectControl(deck.filterGroup, "enabled", DenonMC6000MK2.leftSide.onFilterEnabledValueCB);
	}
	DenonMC6000MK2.rightSide.connectControls();
	//DenonMC6000MK2.connectControl("", "", DenonMC6000MK2.rightSide.onEfx1EnabledValueCB);
	//DenonMC6000MK2.connectControl("", "", DenonMC6000MK2.rightSide.onEfx2EnabledValueCB);
	DenonMC6000MK2.connectControl(DenonMC6000MK2.rightSide.efxGroup, "enabled", DenonMC6000MK2.rightSide.onEfx3EnabledValueCB);
	for (deckGroup in DenonMC6000MK2.rightSide.decksByGroup) {
		deck = this.decksByGroup[deckGroup];
		DenonMC6000MK2.connectControl(deck.filterGroup, "enabled", DenonMC6000MK2.rightSide.onFilterEnabledValueCB);
	}
};

DenonMC6000MK2.restoreValues = function () {
	DenonMC6000MK2.leftSide.restoreValues();
	DenonMC6000MK2.rightSide.restoreValues();
	DenonMC6000MK2.setValue("num_samplers", DenonMC6000MK2.backupNumSamplers);
	DenonMC6000MK2.setValue("num_decks", DenonMC6000MK2.backupNumDecks);
	DenonMC6000MK2.setValue("samplerate", DenonMC6000MK2.backupSampleRate);
};


////////////////////////////////////////////////////////////////////////
// Mixxx scripting callback functions                                 //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.init = function (id, debug) {
	DenonMC6000MK2.id = id;
	DenonMC6000MK2.debug = debug;
	try {
		DenonMC6000MK2.initValues();
		DenonMC6000MK2.connectLeds();
		DenonMC6000MK2.connectControls();
	} catch (ex) {
		DenonMC6000MK2.logError("Exception during controller initialization: " + ex);
	}
};

DenonMC6000MK2.shutdown = function () {
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

DenonMC6000MK2.onTrackSelectKnob = function (channel, control, value, status, group) {
	var knobDelta = DenonMC6000MK2.getKnobDelta(value);
	engine.setValue("[Playlist]", "SelectPrevTrack", 0 > knobDelta);
	engine.setValue("[Playlist]", "SelectNextTrack", 0 < knobDelta);
};

DenonMC6000MK2.onXfaderContour = function (channel, control, value, status, group) {
	script.crossfaderCurve(value);
};


////////////////////////////////////////////////////////////////////////
// MIDI [Channel<n>] callback functions                               //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.onXfaderAssignLeftButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.assignXfaderLeft();
	}
};

DenonMC6000MK2.onXfaderAssignThruButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.assignXfaderCenter();
	}
};

DenonMC6000MK2.onXfaderAssignRightButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.assignXfaderRight();
	}
};

DenonMC6000MK2.onShiftButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	// local shift state
	var side = DenonMC6000MK2.getSideByGroup(group);
	side.shiftState = isButtonPressed;
	// global shift state
	DenonMC6000MK2.shiftState = side.shiftState;
};

DenonMC6000MK2.onLoadButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.loadSelectedTrack();
	}
};

DenonMC6000MK2.onVinylButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onVinylButtonPressed();
	}
};

DenonMC6000MK2.onKeyLockButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onKeyLockButtonPressed();
	}
};

DenonMC6000MK2.onCueMixButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onCueMixButtonPressed();
	}
};

DenonMC6000MK2.onCueButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onCueButton(isButtonPressed);
};

DenonMC6000MK2.onPlayButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onPlayButtonPressed();
	}
};

DenonMC6000MK2.onBendPlusButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onBendPlusButton(isButtonPressed);
};

DenonMC6000MK2.onBendMinusButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onBendMinusButton(isButtonPressed);
};

DenonMC6000MK2.onJogTouch = function (channel, control, value, status, group) {
	var isJogTouched = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.touchJog(isJogTouched);
};

DenonMC6000MK2.onJogTouchVinyl = function (channel, control, value, status, group) {
	var isJogTouched = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.touchJog(isJogTouched);
};

DenonMC6000MK2.onJogSpin = function (channel, control, value, status, group) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	var jogDelta = DenonMC6000MK2.getJogDeltaValue(value);
	deck.spinJog(jogDelta);
};

DenonMC6000MK2.onJogSpinVinyl = function (channel, control, value, status, group) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	var jogDelta = DenonMC6000MK2.getJogDeltaValue(value);
	deck.spinJog(jogDelta);
};

DenonMC6000MK2.onHotcue1Button = function (channel, control, value, status, group) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	deck.hotcues[1].onButton(isButtonPressed);
};

DenonMC6000MK2.onHotcue2Button = function (channel, control, value, status, group) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	deck.hotcues[2].onButton(isButtonPressed);
};

DenonMC6000MK2.onHotcue3Button = function (channel, control, value, status, group) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	deck.hotcues[3].onButton(isButtonPressed);
};

DenonMC6000MK2.onHotcue4Button = function (channel, control, value, status, group) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	deck.hotcues[4].onButton(isButtonPressed);
};

DenonMC6000MK2.onAutoLoopButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onAutoLoopButtonPressed();
	}
};

DenonMC6000MK2.onLoopInButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onLoopInButtonPressed();
	}
};

DenonMC6000MK2.onLoopOutButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onLoopOutButtonPressed();
	}
};

DenonMC6000MK2.onLoopCutMinusButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onLoopCutMinusButtonPressed();
	}
};

DenonMC6000MK2.onLoopCutPlusButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		var deck = DenonMC6000MK2.getDeckByGroup(group);
		deck.onLoopCutPlusButtonPressed();
	}
};

DenonMC6000MK2.onCensorButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onCensorButton(isButtonPressed);
};

DenonMC6000MK2.onSamplerButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	var sampler = DenonMC6000MK2.getSamplerByGroup(group);
	sampler.onButton(isButtonPressed);
};

DenonMC6000MK2.leftSide.onFilterButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.leftSide.filterEnabled = !DenonMC6000MK2.leftSide.filterEnabled;
		DenonMC6000MK2.leftSide.applyFilter();
	}
};

DenonMC6000MK2.rightSide.onFilterButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.rightSide.filterEnabled = !DenonMC6000MK2.rightSide.filterEnabled;
		DenonMC6000MK2.rightSide.applyFilter();
	}
};

DenonMC6000MK2.leftSide.onFilterParam = function (channel, control, value, status, group) {
	DenonMC6000MK2.leftSide.filterParamValue = script.absoluteLin(value, 0.0, 1.0);
	DenonMC6000MK2.leftSide.applyFilter();
};

DenonMC6000MK2.rightSide.onFilterParam = function (channel, control, value, status, group) {
	DenonMC6000MK2.rightSide.filterParamValue = script.absoluteLin(value, 0.0, 1.0);
	DenonMC6000MK2.rightSide.applyFilter();
};

DenonMC6000MK2.leftSide.onEfx1Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	DenonMC6000MK2.leftSide.onEfxButton(1, isButtonPressed);
};

DenonMC6000MK2.rightSide.onEfx1Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	DenonMC6000MK2.rightSide.onEfxButton(1, isButtonPressed);
};

DenonMC6000MK2.leftSide.onEfx1Param = function (channel, control, value, status, group) {
	DenonMC6000MK2.leftSide.onEfxParamValue(1, value);
};

DenonMC6000MK2.rightSide.onEfx1Param = function (channel, control, value, status, group) {
	DenonMC6000MK2.rightSide.onEfxParamValue(1, value);
};

DenonMC6000MK2.leftSide.onEfx2Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	DenonMC6000MK2.leftSide.onEfxButton(2, isButtonPressed);
};

DenonMC6000MK2.rightSide.onEfx2Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	DenonMC6000MK2.rightSide.onEfxButton(2, isButtonPressed);
};

DenonMC6000MK2.leftSide.onEfx2Param = function (channel, control, value, status, group) {
	DenonMC6000MK2.leftSide.onEfxParamValue(2, value);
};

DenonMC6000MK2.rightSide.onEfx2Param = function (channel, control, value, status, group) {
	DenonMC6000MK2.rightSide.onEfxParamValue(2, value);
};

DenonMC6000MK2.leftSide.onEfx3Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	DenonMC6000MK2.leftSide.onEfxButton(3, isButtonPressed);
};

DenonMC6000MK2.rightSide.onEfx3Button = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	DenonMC6000MK2.rightSide.onEfxButton(3, isButtonPressed);
};

DenonMC6000MK2.leftSide.onEfx3Param = function (channel, control, value, status, group) {
	DenonMC6000MK2.leftSide.onEfxParamValue(3, value);
};

DenonMC6000MK2.rightSide.onEfx3Param = function (channel, control, value, status, group) {
	DenonMC6000MK2.rightSide.onEfxParamValue(3, value);
};

DenonMC6000MK2.leftSide.onEfxBeatsKnob = function (channel, control, value, status, group) {
	var delta = DenonMC6000MK2.getKnobDelta(value);
	DenonMC6000MK2.leftSide.onEfxBeatsKnobDelta(delta);
};

DenonMC6000MK2.rightSide.onEfxBeatsKnob = function (channel, control, value, status, group) {
	var delta = DenonMC6000MK2.getKnobDelta(value);
	DenonMC6000MK2.rightSide.onEfxBeatsKnobDelta(delta);
};

DenonMC6000MK2.leftSide.onEfxBeatsButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.leftSide.activeDeck.resetKey();
	}
};

DenonMC6000MK2.rightSide.onEfxBeatsButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.rightSide.activeDeck.resetKey();
	}
};

DenonMC6000MK2.leftSide.onEfxTapButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	DenonMC6000MK2.leftSide.onEfxTapButton(isButtonPressed);
};

DenonMC6000MK2.rightSide.onEfxTapButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	DenonMC6000MK2.rightSide.onEfxTapButton(isButtonPressed);
};

DenonMC6000MK2.leftSide.onDeckButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.leftSide.activateDeckByGroup(group);
	}
};

DenonMC6000MK2.rightSide.onDeckButton = function (channel, control, value, status, group) {
	var isButtonPressed = DenonMC6000MK2.isButtonPressed(value);
	if (isButtonPressed) {
		DenonMC6000MK2.rightSide.activateDeckByGroup(group);
	}
};


////////////////////////////////////////////////////////////////////////
// Mixxx connected controls callback functions                        //
////////////////////////////////////////////////////////////////////////

DenonMC6000MK2.onKeyLockValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onKeyLockValue(value);
};

DenonMC6000MK2.onSyncModeValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onSyncModeValue(value);
};

DenonMC6000MK2.onCueMixValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onCueMixValue(value);
};

DenonMC6000MK2.onTrackSamplesValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onTrackSamplesValue(value);
};

DenonMC6000MK2.onCueIndicatorValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onCueIndicatorValue(value);
};

DenonMC6000MK2.onPlayIndicatorValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onPlayIndicatorValue(value);
};

DenonMC6000MK2.onSlipModeValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.onSlipModeValue(value);
};

/* Loop LEDs */

DenonMC6000MK2.onLoopStartPositionValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.updateLoopLeds();
};

DenonMC6000MK2.onLoopEndPositionValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.updateLoopLeds();
};

DenonMC6000MK2.onLoopEnabledValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.updateLoopLeds();
};

/* Hotcue LEDs */

DenonMC6000MK2.onHotcue1ValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.hotcues[1].updateLeds();
};

DenonMC6000MK2.onHotcue2ValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.hotcues[2].updateLeds();
};

DenonMC6000MK2.onHotcue3ValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.hotcues[3].updateLeds();
};

DenonMC6000MK2.onHotcue4ValueCB = function (value, group, control) {
	var deck = DenonMC6000MK2.getDeckByGroup(group);
	deck.hotcues[4].updateLeds();
};

/* Sampler LEDs */

DenonMC6000MK2.onSamplerValueCB = function (value, group, control) {
	var sampler = DenonMC6000MK2.getSamplerByGroup(group);
	sampler.updateLeds();
};

/* Filter LEDs (shared between 2 decks on each side) */

DenonMC6000MK2.leftSide.onFilterEnabledValueCB = function (value, group, control) {
	DenonMC6000MK2.leftSide.filterLed.setStateBoolean(value);
};

DenonMC6000MK2.rightSide.onFilterEnabledValueCB = function (value, group, control) {
	DenonMC6000MK2.rightSide.filterLed.setStateBoolean(value);
};

/* Efx LEDs */

DenonMC6000MK2.leftSide.onEfxAssignValueCB = function (value, group, control) {
	var deckGroup = control.substring(6, 16);
	var deck = DenonMC6000MK2.getDeckByGroup(deckGroup);
	deck.leftEfxLed.setStateBoolean(value);
};

DenonMC6000MK2.rightSide.onEfxAssignValueCB = function (value, group, control) {
	var deckGroup = control.substring(6, 16);
	var deck = DenonMC6000MK2.getDeckByGroup(deckGroup);
	deck.rightEfxLed.setStateBoolean(value);
};

DenonMC6000MK2.leftSide.onEfx1EnabledValueCB = function (value, group, control) {
	// TODO
};

DenonMC6000MK2.rightSide.onEfx1EnabledValueCB = function (value, group, control) {
	// TODO
};

DenonMC6000MK2.leftSide.onEfx2EnabledValueCB = function (value, group, control) {
	// TODO
};

DenonMC6000MK2.rightSide.onEfx2EnabledValueCB = function (value, group, control) {
	// TODO
};

DenonMC6000MK2.leftSide.onEfx3EnabledValueCB = function (value, group, control) {
	DenonMC6000MK2.leftSide.efx3Led.setStateBoolean(value);
};

DenonMC6000MK2.rightSide.onEfx3EnabledValueCB = function (value, group, control) {
	DenonMC6000MK2.rightSide.efx3Led.setStateBoolean(value);
};
