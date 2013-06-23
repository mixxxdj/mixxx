/******************************************************************************
 * Vestax VCI-300 controller script
 * Author: Uwe Klotz
 *
 * All controls should work as expected.
 * 
 * Quick Reference
 * ---------------
 * -          Cue          = cue_default
 * - Shift  + Cue          = set the cue point (while playing)
 * -   "       "           = clear the cue point (while not playing)
 * -          Play         = play
 * - Shift  + Play         = toggle repeat
 * -          Cue [1-3]/In = hotcue
 * -          Out 1/Loop   = loop in (Shift = clear)
 * -          Out 2/Loop   = loop out (Shift = clear)
 * -          Out 3/Loop   = enable/disable loop
 * -          Auto Loop    = enter/exit beatloop (default: 4 beats)
 * - Shift  + Auto Loop    = enter/exit beatlooproll (default: 4 beats)
 * - Scroll + Auto Loop    = reset number of beats to 4 if beatloop not active
 * -          Pitch Shift  = fine tune the pitch +/-0.01
 * -          Half         = halve loop length
 * - Shift  + Half         = jump to start of track (while not playing)
 * - Scroll + Half         = seek backward (while not playing)
 * -          Double       = double loop length
 * - Shift  + Double       = jump to end of track (while not playing)
 * - Scroll + Double       = seek forward (while not playing)
 * -          Auto Tempo   = trigger beatsync
 * - Shift  + Auto Tempo   = toggle quantize
 * - Scroll + Auto Tempo   = tap BPM
 * -          Keylock      = toggle keylock
 * - Shift  + Keylock      = reset pitch to 0.00% (quartz)
 * -          Censor       = play backwards while button is pressed
 * - Shift  + Censor       = toggle reverse (while playing in reverse direction
 *                           pressing Censor again with or without Shift will
 *                           return back to normal playback)
 * -          Scratch      = toggle scratching
 * -          Jog          = jog movement or scratching
 * - Shift  + Jog          = fast track search (while not playing)
 * - Scroll + Jog          = scroll playlist (while not playing)
 * -          PFL          = toggle PFL
 * - Shift  + PFL          = Load selected track into deck (while not playing)
 *                           and switch PFL to this deck
 *   Back/Fwd              = sidebar navigation
 *   Up/Down               = playlist navigation
 *   Tab                   = expand/collapse selected sidebar item
 *
 * Open issues / TODOs
 * -------------------
 * - Manipulating the crossfader curve does not seem to work as expected
 * - Keylock is turned off when starting to scratch to avoid unnatural noise.
 *   It remains turned off even when scratching is disabled, because otherwise
 *   the engine produces audible glitches.
 * - ITCH-like looping seems impossible with Mixxx 1.x controls(?)
 * - TODO: Blinking LEDs during reverse playback, ...
 * - TODO: Implement soft-takeover for "rate"
 * - TODO: Crates/Files/Browse buttons are connected but currently unused
 * - TODO: Line fader curve knob is connected but currently unused
 * - TODO: Use Shift + Pitch Shift to change pitch range (rateRange)?
 *
 * Technical details
 * -----------------
 * - High-res jog wheels (1664 steps per revolution) for scratching and pitch
 *   bending
 * - High-res pitch sliders (9-bit = 512 steps)
 * - VU meters display left/right channel volume (incl. peak indicator)
 *
 * Revision history
 * ----------------
 * 2012-08-12 - Initial revision for Mixxx 1.11.0
 * 2012-08-13 - Show spinning wheels in scratch mode
 *            - Fast track search while not playing: Shift + Jog
 * 2012-08-14 - Fix typo to avoid crash when pressing Crates button
 *            - Fix typo to correctly disconnect controls upon shutdown
 *            - Fix array of auto loop beat lengths
 *            - Connect and synchronize rateRange with preference settings
 *            - Adjust behavior of Auto Tempo button:
 *              -         Auto Tempo = trigger "beatsync"
 *              - Shift + Auto Tempo = toggle "quantize"
 *            - Manipulate crossfader curve via C.F.CURVE control
 *            - Use Scroll + Jog to scroll the playlist
 *            - Shift + Play = toggle "repeat"
 * 2012-08-15 - Bugfixing, bugfixing, bugfixing
 *            - Fix shutdown() function
 *            - Shift + Cue sets a cue point while playing
 *            - Scroll + Auto Tempo = BPM tap
 *            - Switch hotcue behavior to default
 * 2012-08-24 - Fix pitch shift buttons & LEDs
 *            - Use fixed-point calculations for pitch shift up/down
 *            - Re-organize hotcues
 *            - Minor fixes
 * 2012-08-26 - Fine-tuning of Cue button behavior and lighting
 *            - Fine-tuning of Half/Double button behavior
 *              3 hotcues + looping
 *            - Smart PFL switching
 * 2012-08-27 - Code cleanup and minor fixes
 *            - Take into account if deck is playing when disabling scratching
 *            - Restore non-linear jog response
 * 2012-08-28 - Soft-takeover moved to XML mapping file
 *            - Fix smart PFL switching
 * 2012-09-17 - Reduce "smartness" of PFL switching
 *            - Censor: Use slip_enabled in conjunction with reverse playback
 *              to keep track synchronized
 * 2012-09-24 - Use Auto Tempo LED as a beat indicator
 *            - Reduce sensitivity of playlist scrolling (Scroll + Jog)
 * 2013-01-13 - Map TAB button to ToggleSelectedSidebarItem
 *            - Rework playlist scrolling (Scroll + Jog) completely to solve
 *              sensitivity issues
 * 2013-01-19 - Shift + Auto Loop: enter/exit beatlooproll
 *            - Scroll + Auto Loop: reset number of beats to 4 if beatloop not
 *              active (re-mapped from Shift + Auto Loop)
 *            - Documentation updates
 * ...to be continued...
 *****************************************************************************/


function VestaxVCI300() {}

VestaxVCI300.group = "[Master]";

VestaxVCI300.jogResolution = 1664; // steps per revolution
VestaxVCI300.jogOutputRange = 3.0; // -3.0 <= "jog" <= 3.0
VestaxVCI300.jogDeltaScale = 12.0 / VestaxVCI300.jogResolution; /*TUNABLE PARAM*/
VestaxVCI300.jogSensitivity = 0.6; // 1.0 = linear /*TUNABLE PARAM*/

VestaxVCI300.scratchRPM = 33.0 + (1.0 / 3.0); // 33 1/3 /*TUNABLE PARAM*/
VestaxVCI300.scratchAlpha = 1.0 / 8.0; /*TUNABLE PARAM*/
VestaxVCI300.scratchBeta = VestaxVCI300.scratchAlpha / 32.0; /*TUNABLE PARAM*/
VestaxVCI300.disableScratchingJogDeltaThreshold = 1; /*TUNABLE PARAM*/
VestaxVCI300.disableScratchingPlayNegJogDeltaThreshold = 4; /*TUNABLE PARAM*/
VestaxVCI300.disableScratchingPlayPosJogDeltaThreshold = 7; /*TUNABLE PARAM*/
VestaxVCI300.disableScratchingTimeoutMillisec = 20; // Mixxx minimum timeout = 20 ms

VestaxVCI300.jogScrollBias = 0.0; // Initialize jog scroll
VestaxVCI300.jogScrollDeltaStepsPerTrack = 13; // 1664 / 13 = 128 tracks per revolution /*TUNABLE PARAM*/

VestaxVCI300.pitchFineTuneStepPercent100 = 1; // 1/100 %

VestaxVCI300.autoLoopBeatsArray = [
	"0.0625",
	"0.125",
	"0.25",
	"0.5",
	"1",
	"2",
	"4",
	"8",
	"16",
	"32",
	"64" ];
VestaxVCI300.defaultAutoLoopBeatsIndex = 6; // 4 beats
VestaxVCI300.vuMeterOffThreshold = 0.025;
VestaxVCI300.vuMeterYellowThreshold = 0.9;

VestaxVCI300.cueInNumbers = [ 1, 2, 3 ];

VestaxVCI300.decksByGroup = {};
	
VestaxVCI300.allLEDs = [];

VestaxVCI300.updatePitchValue = function (group, pitchHigh, pitchLow) {
	// 0x00 <= pitchHigh <= 0x7F
	// pitchLow = 0x00/0x20/0x40/0x60 -> 2 out of 7 bits
	var pitchValue = (pitchHigh << 7) | pitchLow;
	// pitchValueMin    = 0
	// pitchValueCenter = 8192 = 0x2000
	// pitchValueMax    = 16352
	if (pitchValue <= 8192) {
		engine.setValue(group, "rate", (8192 - pitchValue) / 8192.0);
	} else {
		// 8160 = 16352 - 8192
		engine.setValue(group, "rate", (8192 - pitchValue) / 8160.0);
	}
};

VestaxVCI300.initValues = function () {
	VestaxVCI300.scrollState = false;
	VestaxVCI300.numDecksBackup = engine.getValue(VestaxVCI300.group, "num_decks");
	engine.setValue(VestaxVCI300.group, "num_decks", 2);
	engine.setValue(VestaxVCI300.group, "crossfader", 0.0);
	engine.setValue(VestaxVCI300.group, "headMix", 0.0);
};

VestaxVCI300.restoreValues = function () {
	engine.setValue(VestaxVCI300.group, "num_decks", VestaxVCI300.numDecksBackup);
};

VestaxVCI300.connectControl = function (group, ctrl, func) {
	engine.connectControl(group, ctrl, func);
	engine.trigger(group, ctrl);
};

VestaxVCI300.disconnectControl = function (group, ctrl, func) {
	// very strange, but works as documented
	engine.connectControl(group, ctrl, func, true);
};

VestaxVCI300.connectControls = function () {
};

VestaxVCI300.disconnectControls = function () {
};

VestaxVCI300.updateScrollState = function () {
	VestaxVCI300.scrollLED.trigger(VestaxVCI300.scrollState);
};


//
// Buttons
//

VestaxVCI300.getButtonPressed = function (value) {
	switch (value) {
		case 0x00:
			return false;
		case 0x7F:
			return true;
	}
};

VestaxVCI300.toggleBinaryValue = function (group, control) {
	engine.setValue(group, control, !engine.getValue(group, control));
};

VestaxVCI300.onToggleButton = function (group, control, value) {
	if (VestaxVCI300.getButtonPressed(value)) {
		VestaxVCI300.toggleBinaryValue(group, control);
	} else {
		engine.trigger(group, control);
	}
};


//
// LEDs
//

VestaxVCI300.LED = function (control) {
	this.control = control;
	VestaxVCI300.allLEDs.push(this);
};

VestaxVCI300.LED.trigger = function (control, state) {
	midi.sendShortMsg(0x90, control, state ? 0x7F : 0x00);
};

VestaxVCI300.LED.prototype.trigger = function (state) {
	VestaxVCI300.LED.trigger(this.control, state);
};

VestaxVCI300.turnOffAllLEDs = function () {
	for (var ledIndex in VestaxVCI300.allLEDs) {
		VestaxVCI300.allLEDs[ledIndex].trigger(false);
	}
};


//
// Decks
//

VestaxVCI300.Deck = function (number) {
	this.number = number;
	this.group = "[Channel" + this.number + "]";
	VestaxVCI300.decksByGroup[this.group] = this;
	this.onCueInValueCB = [];
	this.onOutLoopValueCB = [];
};

VestaxVCI300.leftDeck = new VestaxVCI300.Deck(1);
VestaxVCI300.rightDeck = new VestaxVCI300.Deck(2);

VestaxVCI300.Deck.prototype.resetScratchingTimer = function () {
	if (undefined != this.disableScratchingTimer) {
		engine.stopTimer(this.disableScratchingTimer);
		this.disableScratchingTimer = undefined;
	}
};

VestaxVCI300.Deck.prototype.enableScratching = function () {
	this.resetScratchingTimer();
	engine.setValue(this.group, "keylock", false);
	engine.scratchEnable(this.number, VestaxVCI300.jogResolution, VestaxVCI300.scratchRPM, VestaxVCI300.scratchAlpha, VestaxVCI300.scratchBeta, /*ramp*/ true);
};

VestaxVCI300.Deck.prototype.disableScratching = function () {
	this.resetScratchingTimer();
	engine.scratchDisable(this.number, /*ramp*/ true);
	// leave keylock turned off to avoid unnatural playback and glitches
};

VestaxVCI300.leftDeck.disableScratchingTimerCB = function () {
	VestaxVCI300.leftDeck.disableScratching();
};

VestaxVCI300.rightDeck.disableScratchingTimerCB = function () {
	VestaxVCI300.rightDeck.disableScratching();
};

VestaxVCI300.Deck.prototype.disableScratchingLazy = function () {
	var jogDeltaThreshold = VestaxVCI300.disableScratchingJogDeltaThreshold;
	var absJogDelta;
	if (undefined == this.jogDelta) {
		absJogDelta = 0;
	} else {
		absJogDelta = Math.abs(this.jogDelta);
		if (engine.getValue(this.group, "play")) {
			if (0 > this.jogDelta) {
				jogDeltaThreshold = VestaxVCI300.disableScratchingPlayNegJogDeltaThreshold;
			} else {
				jogDeltaThreshold = VestaxVCI300.disableScratchingPlayPosJogDeltaThreshold;
			}
		}
	}
	if (absJogDelta < jogDeltaThreshold) {
		this.disableScratching();
	} else {
		this.resetScratchingTimer();
		this.disableScratchingTimer = engine.beginTimer(
			VestaxVCI300.disableScratchingTimeoutMillisec,
			this.disableScratchingTimerCB);
	}
};

VestaxVCI300.Deck.prototype.updateJogValue = function (jogHigh, jogLow) {
	// 0x00 <= jogHigh/jogLow <= 0x7F
	var jogValue = (jogHigh << 7) | jogLow;
	// 0x0000 <= jogValue <= 0x3FFF (14-bit, cyclic)
	// 1 revolution = 0x0D00 = 1664 steps
	if (undefined != this.jogValue) {
		this.jogDelta = jogValue - this.jogValue;
		if (this.jogDelta >= 0x2000) {
			// cyclic overflow
			this.jogDelta -= 0x4000;
		} else if (this.jogDelta < -0x2000) {
			// cyclic overflow
			this.jogDelta += 0x4000;
		}
		this.jogMoveLED.trigger(0 != this.jogDelta);
		// Reset jog scrolling
		jogScrollBias = VestaxVCI300.jogScrollBias;
		VestaxVCI300.jogScrollBias = 0.0;
		if (this.shiftState && !engine.getValue(this.group,"play")) {
			// fast track search
			var playposition = engine.getValue(this.group, "playposition");
			if (undefined != playposition) {
				var searchpos = engine.getValue(this.group, "playposition") + (this.jogDelta / VestaxVCI300.jogResolution);
				engine.setValue(this.group, "playposition", Math.max(0.0, Math.min(1.0, searchpos)));
			}
		} else if (VestaxVCI300.scrollState && !engine.getValue(this.group,"play")) {
			// scroll playlist
			var jogScrollDelta;
			jogScrollDelta = jogScrollBias + (this.jogDelta / VestaxVCI300.jogScrollDeltaStepsPerTrack);
			var jogScrollDeltaRound;
			jogScrollDeltaRound = Math.round(jogScrollDelta);
			engine.setValue(
				"[Playlist]",
				"SelectTrackKnob",
				jogScrollDeltaRound);
			VestaxVCI300.jogScrollBias = jogScrollDelta - jogScrollDeltaRound;
		} else {
			if (engine.isScratching(this.number)) {
				// scratching
				engine.scratchTick(this.number, this.jogDelta);
			} else {
				// jog movement
				var jogDeltaSign;
				if (0 > this.jogDelta) {
					jogDeltaSign = -1;
				} else {
					jogDeltaSign = 1;
				}
				var normalizedAbsJogDelta =  Math.pow(Math.abs(this.jogDelta) * VestaxVCI300.jogDeltaScale, VestaxVCI300.jogSensitivity);
				engine.setValue(this.group, "jog", jogDeltaSign * normalizedAbsJogDelta * VestaxVCI300.jogOutputRange);
			}
		}
	}
	this.jogValue = jogValue;
	if (undefined != this.disableScratchingTimer) {
		// disable scratching is pending
		this.disableScratchingLazy();
	}
};

VestaxVCI300.Deck.prototype.updateShiftState = function () {
	this.shiftLED.trigger(this.shiftState);
};

VestaxVCI300.Deck.prototype.updateScratchState = function () {
	this.scratchLED.trigger(this.scratchState);
	engine.setValue("[Spinny" + this.number + "]", "show_spinny", this.scratchState);
	if (this.scratchState && this.jogTouchState) {
		this.enableScratching();
	} else {
		this.disableScratchingLazy();
	}
};

VestaxVCI300.Deck.prototype.updateCueInState = function (index) {
	this.cueInLEDs[index].trigger(engine.getValue(this.group, "hotcue_" + VestaxVCI300.cueInNumbers[index] + "_enabled"));
	this.cueLoopLEDs[index][0].trigger(false);
};

VestaxVCI300.Deck.prototype.updateOutLoopState = function (index) {
	this.outLoopLEDs[index].trigger(false);
	switch (index) {
		case 0:
			this.cueLoopLEDs[0][1].trigger(0 <= engine.getValue(this.group, "loop_start_position"));
			break;
		case 1:
			this.cueLoopLEDs[1][1].trigger(0 <= engine.getValue(this.group, "loop_end_position"));
			break;
		case 2:
			this.cueLoopLEDs[2][1].trigger(engine.getValue(this.group, "loop_enabled"));
			break;
	}
};

VestaxVCI300.Deck.prototype.updateRateRange = function () {
	this.rateRangePercent100 = Math.round(10000.0 * engine.getValue(this.group, "rateRange")); // example: +/-6% = 0.06
	// Workaround: Explicitly (re-)set "rate" after changing "rateRange"!
	// Otherwise the first button press on one of the pitch shift buttons
	// is ignored.
	engine.setValue(this.group, "rate", engine.getValue(this.group, "rate"));
};

VestaxVCI300.Deck.prototype.updateCueState = function () {
	this.cueLED.trigger(0 < engine.getValue(this.group, "cue_point"));
};

VestaxVCI300.Deck.prototype.updateBeatSyncState = function () {
	this.beatSyncLED.trigger(
		engine.getValue(this.group, "beatsync")
		|| (engine.getValue(this.group, "play")
		&& engine.getValue(this.group, "beat_active")));
};

VestaxVCI300.Deck.prototype.initValues = function () {
	this.shiftState = false;
	this.scratchState = false;
	this.jogTouchState = false;
	this.autoLoopBeatsIndex = VestaxVCI300.defaultAutoLoopBeatsIndex;
	this.greenVUMeterOffset = VestaxVCI300.vuMeterOffThreshold;
	this.greenVUMeterScale = (VestaxVCI300.vuMeterYellowThreshold - this.greenVUMeterOffset) / this.greenVUMeterLEDs.length;
	this.redVUMeterOffset = VestaxVCI300.vuMeterYellowThreshold;
	this.redVUMeterScale = (1.0 - this.redVUMeterOffset) / this.redVUMeterLEDs.length;
	engine.setValue(this.group, "volume", 0.0);
	engine.setValue(this.group, "filterHigh", 1.0);
	engine.setValue(this.group, "filterMid", 1.0);
	engine.setValue(this.group, "filterLow", 1.0);
	engine.setValue(this.group, "pregain", 1.0);
	this.rateDirBackup = engine.getValue(this.group, "rate_dir");
	engine.setValue(this.group, "rate_dir", -1);
	engine.setValue(this.group, "rate", 0.0);
	engine.softTakeover(this.group, "rate", true); // TODO: does not seem to work
	engine.setValue(this.group, "keylock", true);
};

VestaxVCI300.Deck.prototype.restoreValues = function () {
	engine.setValue(this.group, "rate_dir", this.rateDirBackup);
};

VestaxVCI300.Deck.prototype.connectControls = function () {
	VestaxVCI300.connectControl(this.group, "pfl", this.onPFLValueCB);
	VestaxVCI300.connectControl(this.group, "rateRange", this.onRateRangeValueCB);
	VestaxVCI300.connectControl(this.group, "cue_point", this.onCueValueCB);
	VestaxVCI300.connectControl(this.group, "play", this.onPlayValueCB);
	VestaxVCI300.connectControl(this.group, "loop_halve", this.onLoopHalveValueCB);
	VestaxVCI300.connectControl(this.group, "loop_double", this.onLoopDoubleValueCB);
	VestaxVCI300.connectControl(this.group, "keylock", this.onKeylockValueCB);
	VestaxVCI300.connectControl(this.group, "beatsync", this.onBeatSyncValueCB);
	VestaxVCI300.connectControl(this.group, "beat_active", this.onBeatActiveValueCB);
	VestaxVCI300.connectControl(this.group, "reverse", this.onReverseValueCB);
	VestaxVCI300.connectControl(this.group, "PeakIndicator", this.onPeakIndicatorValueCB);
	VestaxVCI300.connectControl(this.group, "VuMeter", this.onVUMeterValueCB);
	for (var cueInIndex in VestaxVCI300.cueInNumbers) {
		var cueInNumber = VestaxVCI300.cueInNumbers[cueInIndex];
		VestaxVCI300.connectControl(this.group, "hotcue_" + cueInNumber + "_enabled", this.onCueInValueCB[cueInIndex]);
	}
	VestaxVCI300.connectControl(this.group, "loop_start_position", this.onOutLoopValueCB[0]);
	VestaxVCI300.connectControl(this.group, "loop_end_position", this.onOutLoopValueCB[1]);
	VestaxVCI300.connectControl(this.group, "loop_enabled", this.onOutLoopValueCB[2]);
	for (var beatsIndex in VestaxVCI300.autoLoopBeatsArray) {
		VestaxVCI300.connectControl(this.group, "beatloop_" + VestaxVCI300.autoLoopBeatsArray[beatsIndex] + "_enabled", this.onAutoLoopValueCB);
	}
};

VestaxVCI300.Deck.prototype.disconnectControls = function () {
	VestaxVCI300.disconnectControl(this.group, "pfl");
	VestaxVCI300.disconnectControl(this.group, "rateRange");
	VestaxVCI300.disconnectControl(this.group, "cue_point");
	VestaxVCI300.disconnectControl(this.group, "play");
	VestaxVCI300.disconnectControl(this.group, "loop_halve");
	VestaxVCI300.disconnectControl(this.group, "loop_double");
	VestaxVCI300.disconnectControl(this.group, "keylock");
	VestaxVCI300.disconnectControl(this.group, "beatsync");
	VestaxVCI300.disconnectControl(this.group, "beat_active");
	VestaxVCI300.disconnectControl(this.group, "reverse");
	VestaxVCI300.disconnectControl(this.group, "PeakIndicator");
	VestaxVCI300.disconnectControl(this.group, "VuMeter");
	for (var cueInIndex in VestaxVCI300.cueInNumbers) {
		var cueInNumber = VestaxVCI300.cueInNumbers[cueInIndex];
		VestaxVCI300.disconnectControl(this.group, "hotcue_" + cueInNumber + "_enabled");
	}
	VestaxVCI300.disconnectControl(this.group, "loop_start_position");
	VestaxVCI300.disconnectControl(this.group, "loop_end_position");
	VestaxVCI300.disconnectControl(this.group, "loop_enabled");
	for (var beatsIndex in VestaxVCI300.autoLoopBeatsArray) {
		var autoLoopBeats = VestaxVCI300.autoLoopBeatsArray[beatsIndex];
		VestaxVCI300.disconnectControl(this.group, "beatloop_" + autoLoopBeats + "_enabled");
	}
};

VestaxVCI300.Deck.prototype.updateVUMeterState = function (value) {
	var level;
	var bias;
	for (level = 0; level < this.greenVUMeterLEDs.length; ++level) {
		bias = this.greenVUMeterOffset + level * this.greenVUMeterScale;
		this.greenVUMeterLEDs[level].trigger(value > bias);
	}
	for (level = 0; level < this.redVUMeterLEDs.length; ++level) {
		bias = this.redVUMeterOffset + level * this.redVUMeterScale;
		this.redVUMeterLEDs[level].trigger(value > bias);
	}
};

VestaxVCI300.Deck.prototype.isAutoLoopEnabled = function () {
	var autoLoopBeats = VestaxVCI300.autoLoopBeatsArray[this.autoLoopBeatsIndex];
	return engine.getValue(this.group, "beatloop_" + autoLoopBeats + "_enabled");
};

VestaxVCI300.Deck.prototype.updateAutoLoopState = function () {
	this.autoLoopLED.trigger(this.isAutoLoopEnabled());
};


//
// Application callback functions
// 

VestaxVCI300.init = function (id, debug) {
	VestaxVCI300.id = id;
	VestaxVCI300.debug = debug;
	VestaxVCI300.scrollLED =
		new VestaxVCI300.LED(0x2F);
	VestaxVCI300.cratesLED =
		new VestaxVCI300.LED(0x2C);
	VestaxVCI300.filesLED =
		new VestaxVCI300.LED(0x2E);
	VestaxVCI300.browseLED =
		new VestaxVCI300.LED(0x30);
	VestaxVCI300.leftDeck.shiftLED =
		new VestaxVCI300.LED(0x27);
	VestaxVCI300.leftDeck.scratchLED =
		new VestaxVCI300.LED(0x26);
	VestaxVCI300.leftDeck.beatSyncLED =
		new VestaxVCI300.LED(0x25);
	VestaxVCI300.leftDeck.cueLED =
		new VestaxVCI300.LED(0x71);
	VestaxVCI300.leftDeck.playLED =
		new VestaxVCI300.LED(0x72);
	VestaxVCI300.leftDeck.jogMoveLED =
		new VestaxVCI300.LED(0x75);
	VestaxVCI300.leftDeck.jogTouchLED =
		new VestaxVCI300.LED(0x76);
	VestaxVCI300.leftDeck.reverseLED =
		new VestaxVCI300.LED(0x28);
	VestaxVCI300.leftDeck.autoLoopLED =
		new VestaxVCI300.LED(0x29);
	VestaxVCI300.leftDeck.pitchShiftDownLED =
		new VestaxVCI300.LED(0x3B);
	VestaxVCI300.leftDeck.pitchShiftUpLED =
		new VestaxVCI300.LED(0x3C);
	VestaxVCI300.leftDeck.pflLED =
		new VestaxVCI300.LED(0x2D);
	VestaxVCI300.leftDeck.keylockLED =
		new VestaxVCI300.LED(0x24);
	VestaxVCI300.leftDeck.autoLoopHalfLED =
		new VestaxVCI300.LED(0x2A);
	VestaxVCI300.leftDeck.autoLoopDoubleLED =
		new VestaxVCI300.LED(0x2B);
	VestaxVCI300.leftDeck.greenVUMeterLEDs = [
		new VestaxVCI300.LED(0x64),
		new VestaxVCI300.LED(0x63),
		new VestaxVCI300.LED(0x62),
		new VestaxVCI300.LED(0x61),
		new VestaxVCI300.LED(0x60),
		new VestaxVCI300.LED(0x5F),
		new VestaxVCI300.LED(0x5E),
		new VestaxVCI300.LED(0x5D),
		new VestaxVCI300.LED(0x5C)
	];
	VestaxVCI300.leftDeck.redVUMeterLEDs = [
		new VestaxVCI300.LED(0x5B),
		new VestaxVCI300.LED(0x5A)
	];
	VestaxVCI300.leftDeck.peakIndicatorLED =
		new VestaxVCI300.LED(0x59);
	VestaxVCI300.leftDeck.cueInLEDs = [
		new VestaxVCI300.LED(0x41) /*red*/,
		new VestaxVCI300.LED(0x43) /*yellow*/,
		new VestaxVCI300.LED(0x45) /*blue*/
	];
	VestaxVCI300.leftDeck.outLoopLEDs = [
		new VestaxVCI300.LED(0x42) /*red*/,
		new VestaxVCI300.LED(0x44) /*yellow*/,
		new VestaxVCI300.LED(0x46) /*blue*/
	];
	VestaxVCI300.leftDeck.cueLoopLEDs = [
		[ new VestaxVCI300.LED(0x47) /*green*/, new VestaxVCI300.LED(0x48) /*green*/ ],
		[ new VestaxVCI300.LED(0x49) /*green*/, new VestaxVCI300.LED(0x4A) /*green*/ ],
		[ new VestaxVCI300.LED(0x4B) /*green*/, new VestaxVCI300.LED(0x4C) /*green*/ ]
	];
	VestaxVCI300.rightDeck.shiftLED =
		new VestaxVCI300.LED(0x37);
	VestaxVCI300.rightDeck.scratchLED =
		new VestaxVCI300.LED(0x38);
	VestaxVCI300.rightDeck.beatSyncLED =
		new VestaxVCI300.LED(0x39);
	VestaxVCI300.rightDeck.cueLED =
		new VestaxVCI300.LED(0x73);
	VestaxVCI300.rightDeck.playLED =
		new VestaxVCI300.LED(0x74);
	VestaxVCI300.rightDeck.jogMoveLED =
		new VestaxVCI300.LED(0x77);
	VestaxVCI300.rightDeck.jogTouchLED =
		new VestaxVCI300.LED(0x78);
	VestaxVCI300.rightDeck.reverseLED =
		new VestaxVCI300.LED(0x34);
	VestaxVCI300.rightDeck.autoLoopLED =
		new VestaxVCI300.LED(0x35);
	VestaxVCI300.rightDeck.pitchShiftDownLED =
		new VestaxVCI300.LED(0x3D);
	VestaxVCI300.rightDeck.pitchShiftUpLED =
		new VestaxVCI300.LED(0x3E);
	VestaxVCI300.rightDeck.pflLED =
		new VestaxVCI300.LED(0x31);
	VestaxVCI300.rightDeck.keylockLED =
		new VestaxVCI300.LED(0x33);
	VestaxVCI300.rightDeck.autoLoopHalfLED =
		new VestaxVCI300.LED(0x32);
	VestaxVCI300.rightDeck.autoLoopDoubleLED =
		new VestaxVCI300.LED(0x36);
	VestaxVCI300.rightDeck.greenVUMeterLEDs = [
		new VestaxVCI300.LED(0x70),
		new VestaxVCI300.LED(0x6F),
		new VestaxVCI300.LED(0x6E),
		new VestaxVCI300.LED(0x6D),
		new VestaxVCI300.LED(0x6C),
		new VestaxVCI300.LED(0x6B),
		new VestaxVCI300.LED(0x6A),
		new VestaxVCI300.LED(0x69),
		new VestaxVCI300.LED(0x68) ];
	VestaxVCI300.rightDeck.redVUMeterLEDs = [
		new VestaxVCI300.LED(0x67),
		new VestaxVCI300.LED(0x66) ];
	VestaxVCI300.rightDeck.peakIndicatorLED =
		new VestaxVCI300.LED(0x65);
	VestaxVCI300.rightDeck.cueInLEDs = [
		new VestaxVCI300.LED(0x4D) /*red*/,
		new VestaxVCI300.LED(0x4F) /*yellow*/,
		new VestaxVCI300.LED(0x51) /*blue*/
	];
	VestaxVCI300.rightDeck.outLoopLEDs = [
		new VestaxVCI300.LED(0x4E) /*red*/,
		new VestaxVCI300.LED(0x50) /*yellow*/,
		new VestaxVCI300.LED(0x52) /*blue*/
	];
	VestaxVCI300.rightDeck.cueLoopLEDs = [
		[ new VestaxVCI300.LED(0x53) /*green*/, new VestaxVCI300.LED(0x54) /*green*/ ],
		[ new VestaxVCI300.LED(0x55) /*green*/, new VestaxVCI300.LED(0x56) /*green*/ ],
		[ new VestaxVCI300.LED(0x57) /*green*/, new VestaxVCI300.LED(0x58) /*green*/ ]
	];
	VestaxVCI300.initValues();
	VestaxVCI300.connectControls();
	VestaxVCI300.updateScrollState();
	for (var group in VestaxVCI300.decksByGroup) {
		var deck = VestaxVCI300.decksByGroup[group];
		deck.initValues();
		deck.connectControls();
		deck.updateShiftState();
		deck.updateScratchState();
	}
};

VestaxVCI300.shutdown = function () {
	try {
		for (var group in VestaxVCI300.decksByGroup) {
			var deck = VestaxVCI300.decksByGroup[group];
			deck.resetScratchingTimer();
			deck.disconnectControls();
			deck.restoreValues();
		}
		VestaxVCI300.disconnectControls();
		VestaxVCI300.restoreValues();
		VestaxVCI300.turnOffAllLEDs();
	} catch (ex) {
		print("Exception in VestaxVCI300.shutdown(): " + ex);
	}
};


//
// Controller callback functions (see also: Vestax-VCI-300-midi.xml)
// 

VestaxVCI300.onCueInButton = function (group, index, value) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		engine.setValue(group, "hotcue_" + VestaxVCI300.cueInNumbers[index] + "_clear", VestaxVCI300.getButtonPressed(value));
	} else {
		engine.setValue(group, "hotcue_" + VestaxVCI300.cueInNumbers[index] + "_activate", VestaxVCI300.getButtonPressed(value));
	}
};

VestaxVCI300.onOutLoopButton = function (group, index, value) {
	var deck = VestaxVCI300.decksByGroup[group];
	switch (index) {
		case 0:
			if (deck.shiftState) {
				engine.setValue(group, "loop_end_position", -1);
				engine.setValue(group, "loop_start_position", -1);
			} else {
				engine.setValue(group, "loop_in", VestaxVCI300.getButtonPressed(value));
			}
			break;
		case 1:
			if (deck.shiftState) {
				engine.setValue(group, "loop_end_position", -1);
			} else {
				engine.setValue(group, "loop_out", VestaxVCI300.getButtonPressed(value));
			}
			break;
		case 2:
			engine.setValue(group, "reloop_exit", VestaxVCI300.getButtonPressed(value));
			break;
	}
};

VestaxVCI300.onScrollButton = function (channel, control, value, status, group) {
	VestaxVCI300.scrollState = VestaxVCI300.getButtonPressed(value);
	VestaxVCI300.updateScrollState();
};

VestaxVCI300.onShiftButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	deck.shiftState = VestaxVCI300.getButtonPressed(value);
	deck.updateShiftState();
};

VestaxVCI300.onCueButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		if (VestaxVCI300.getButtonPressed(value)) {
			if (engine.getValue(group, "play")) {
				// set
				engine.setValue(group, "cue_set", true);
			} else {
				// clear
				engine.setValue(group, "cue_point", 0.0);
			}
		}
	} else {
		engine.setValue(group, "cue_default", VestaxVCI300.getButtonPressed(value));
	}
	engine.trigger(group, "cue_default");
};

VestaxVCI300.onPlayButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		if (VestaxVCI300.getButtonPressed(value)) {
			// toggle repeat
			VestaxVCI300.toggleBinaryValue(group, "repeat");
			deck.playLED.trigger(true);
		}
	} else {
		VestaxVCI300.onToggleButton(group, "play", value);
	}
	engine.trigger(group, "play");
};

VestaxVCI300.onCensorReverseButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		// Reverse
		VestaxVCI300.onToggleButton(group, "reverse", value);
	} else {
		// Censor
		engine.setValue(group, "slip_enabled", VestaxVCI300.getButtonPressed(value));
		VestaxVCI300.toggleBinaryValue(group, "reverse");
	}
};

VestaxVCI300.onKeyLockButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		if (VestaxVCI300.getButtonPressed(value)) {
			// quartz lock
			engine.setValue(group, "rate", 0.0);
			deck.keylockLED.trigger(true);
		} else {
			engine.trigger(group, "keylock");
		}
	} else {
		if (VestaxVCI300.getButtonPressed(value)) {
			VestaxVCI300.toggleBinaryValue(group, "keylock");
		} else {
			engine.trigger(group, "keylock");
		}
	}
};

VestaxVCI300.onAutoTempoButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (VestaxVCI300.scrollState) {
		// tap bpm
		engine.setValue(group, "bpm_tap", VestaxVCI300.getButtonPressed(value));
		if (VestaxVCI300.getButtonPressed(value)) {
			deck.beatSyncLED.trigger(true);
		} else {
			engine.trigger(group, "beatsync");
		}
	} else if (deck.shiftState) {
		// quantize
		engine.setValue(group, "bpm_tap", false);
		if (VestaxVCI300.getButtonPressed(value)) {
			VestaxVCI300.toggleBinaryValue(group, "quantize");
			deck.beatSyncLED.trigger(true);
		} else {
			engine.trigger(group, "beatsync");
		}
	} else {
		// beatsync
		engine.setValue(group, "bpm_tap", false);
		engine.setValue(group, "beatsync", VestaxVCI300.getButtonPressed(value));
	}
};

VestaxVCI300.onPFLButton = function (channel, control, value, status, group) {
	if (VestaxVCI300.scrollState && VestaxVCI300.getButtonPressed(value)) {
		// load selected track into deck
		engine.setValue(group, "LoadSelectedTrack", true);
		// switch PFL to loaded deck:
		// - disable PFL on all decks
		engine.setValue(VestaxVCI300.leftDeck.group, "pfl", false);
		engine.setValue(VestaxVCI300.rightDeck.group, "pfl", false);
		// - enable PFL on loaded deck only
		engine.setValue(group, "pfl", true);
	} else {
		VestaxVCI300.onToggleButton(group, "pfl", value);
	}
};

VestaxVCI300.onHalfPrevButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	var trigger = true;
	var reset = true;
	if (VestaxVCI300.scrollState) {
		if (engine.getValue(group, "play")) {
			engine.setValue(group, "back", false);
		} else {
			engine.setValue(group, "back", VestaxVCI300.getButtonPressed(value));
			if (VestaxVCI300.getButtonPressed(value)) {
				deck.autoLoopHalfLED.trigger(true);
				trigger = false;
			}
		}
	} else {
		engine.setValue(group, "back", false);
		if (deck.shiftState) {
			if (!engine.getValue(group, "play")) {
				// jump to track start
				engine.setValue(group, "start", VestaxVCI300.getButtonPressed(value));
				if (VestaxVCI300.getButtonPressed(value)) {
					deck.autoLoopHalfLED.trigger(true);
					trigger = false;
				}
			}
		} else {
			if (VestaxVCI300.getButtonPressed(value) && (0 < deck.autoLoopBeatsIndex)) {
				engine.setValue(group, "loop_halve", true);
				reset = false;
				--deck.autoLoopBeatsIndex;
			}
		}
	}
	if (trigger) {
		if (reset) {
			engine.setValue(group, "loop_halve", false);
		} else {
			engine.trigger(group, "loop_halve");
		}
	}
};

VestaxVCI300.onDoubleNextButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	var trigger = true;
	var reset = true;
	if (VestaxVCI300.scrollState) {
		if (engine.getValue(group, "play")) {
			engine.setValue(group, "fwd", false);
		} else {
			engine.setValue(group, "fwd", VestaxVCI300.getButtonPressed(value));
			if (VestaxVCI300.getButtonPressed(value)) {
				deck.autoLoopDoubleLED.trigger(true);
				trigger = false;
			}
		}
	} else {
		engine.setValue(group, "fwd", false);
		if (deck.shiftState) {
			if (!engine.getValue(group, "play")) {
				// jump to track end
				if (VestaxVCI300.getButtonPressed(value)) {
					engine.setValue(group, "end", true);
					deck.autoLoopDoubleLED.trigger(true);
					trigger = false;
				}
			}
		} else {
			if (VestaxVCI300.getButtonPressed(value) && (deck.autoLoopBeatsIndex < (VestaxVCI300.autoLoopBeatsArray.length - 1))) {
				engine.setValue(group, "loop_double", true);
				reset = false;
				++deck.autoLoopBeatsIndex;
			}
		}
	}
	if (trigger) {
		if (reset) {
			engine.setValue(group, "loop_double", false);
		} else {
			engine.trigger(group, "loop_double");
		}
	}
};

VestaxVCI300.onPitchHighValue = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	deck.pitchHighValue = value;
	// defer adjusting the pitch until onPitchLowValue() arrives
};

VestaxVCI300.onPitchLowValue = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	VestaxVCI300.updatePitchValue(group, deck.pitchHighValue, value);
};

VestaxVCI300.adjustRate = function(deck, rateDeltaPercent100) {
	var ratePercent100 = Math.round(engine.getValue(deck.group, "rate") * deck.rateRangePercent100);
	ratePercent100 -= rateDeltaPercent100; // inverse pitch range -> sub instead of add!
	engine.setValue(deck.group, "rate", Math.max(-1.0, Math.min(1.0, ratePercent100 / deck.rateRangePercent100)));
};

VestaxVCI300.onPitchShiftDownButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		// TODO
	} else {
		if (VestaxVCI300.getButtonPressed(value)) {
			VestaxVCI300.adjustRate(deck, -VestaxVCI300.pitchFineTuneStepPercent100);
		}
	}
	deck.pitchShiftDownLED.trigger(VestaxVCI300.getButtonPressed(value));
};

VestaxVCI300.onPitchShiftUpButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		// TODO
	} else {
		if (VestaxVCI300.getButtonPressed(value)) {
			VestaxVCI300.adjustRate(deck, VestaxVCI300.pitchFineTuneStepPercent100);
		}
	}
	deck.pitchShiftUpLED.trigger(VestaxVCI300.getButtonPressed(value));
};

VestaxVCI300.onScratchButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (VestaxVCI300.getButtonPressed(value)) {
		deck.scratchState = !deck.scratchState;
	}
	deck.updateScratchState();
};

VestaxVCI300.onJogTouch = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	deck.jogTouchState = VestaxVCI300.getButtonPressed(value);
	deck.jogTouchLED.trigger(deck.jogTouchState);
	deck.updateScratchState();
};

VestaxVCI300.onJogHighValue = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	deck.jogHighValue = value;
	// defer adjusting the jog position until onJogLowValue() arrives
};

VestaxVCI300.onJogLowValue = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	deck.updateJogValue(deck.jogHighValue, value);
};

VestaxVCI300.onCue1InButton = function (channel, control, value, status, group) {
	VestaxVCI300.onCueInButton(group, 0, value);
};

VestaxVCI300.onCue2InButton = function (channel, control, value, status, group) {
	VestaxVCI300.onCueInButton(group, 1, value);
};

VestaxVCI300.onCue3InButton = function (channel, control, value, status, group) {
	VestaxVCI300.onCueInButton(group, 2, value);
};

VestaxVCI300.onOut1LoopButton = function (channel, control, value, status, group) {
	VestaxVCI300.onOutLoopButton(group, 0, value);
};

VestaxVCI300.onOut2LoopButton = function (channel, control, value, status, group) {
	VestaxVCI300.onOutLoopButton(group, 1, value);
};

VestaxVCI300.onOut3LoopButton = function (channel, control, value, status, group) {
	VestaxVCI300.onOutLoopButton(group, 2, value);
};

VestaxVCI300.onAutoLoopButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	var beatloopPrefix = "beatloop_" + VestaxVCI300.autoLoopBeatsArray[deck.autoLoopBeatsIndex];
	if (VestaxVCI300.scrollState) {
		if (VestaxVCI300.getButtonPressed(value) &&
			!engine.getValue(group, beatloopPrefix + "_enabled")) {
			// reset length to default for new loop
			deck.autoLoopBeatsIndex = VestaxVCI300.defaultAutoLoopBeatsIndex;
			deck.autoLoopLED.trigger(true);
		} else {
			engine.trigger(group, beatloopPrefix + "_enabled");
		}
	} else if (deck.shiftState) {
		// toggle beatlooproll
		var beatlooprollPrefix = "beatlooproll_" + VestaxVCI300.autoLoopBeatsArray[deck.autoLoopBeatsIndex];
		if (VestaxVCI300.getButtonPressed(value)) {
			engine.setValue(group, beatlooprollPrefix + "_activate", !engine.getValue(group, beatloopPrefix + "_enabled"));
		} else {
			engine.trigger(group, beatloopPrefix + "_enabled");
		}
	} else {
		engine.setValue(group, beatloopPrefix + "_toggle", VestaxVCI300.getButtonPressed(value));
	}
};

VestaxVCI300.onCrossfaderCurve = function (channel, control, value, status, group) {
	script.crossfaderCurve(value, /*min=*/0x00, /*max=*/0x7F);
};

VestaxVCI300.onLinefaderCurve = function (channel, control, value, status, group) {
	// TODO
};

VestaxVCI300.onCratesButton = function (channel, control, value, status, group) {
	// TODO
	VestaxVCI300.cratesLED.trigger(VestaxVCI300.getButtonPressed(value));
};

VestaxVCI300.onFilesButton = function (channel, control, value, status, group) {
	// TODO
	VestaxVCI300.filesLED.trigger(VestaxVCI300.getButtonPressed(value));
};

VestaxVCI300.onBrowseButton = function (channel, control, value, status, group) {
	// TODO
	VestaxVCI300.browseLED.trigger(VestaxVCI300.getButtonPressed(value));
};

VestaxVCI300.onNavigationUpButton = function (channel, control, value, status, group) {
	engine.setValue("[Playlist]", "SelectPrevTrack", VestaxVCI300.getButtonPressed(value));
};

VestaxVCI300.onNavigationDownButton = function (channel, control, value, status, group) {
	engine.setValue("[Playlist]", "SelectNextTrack", VestaxVCI300.getButtonPressed(value));
};

VestaxVCI300.onNavigationBackButton = function (channel, control, value, status, group) {
	engine.setValue("[Playlist]", "SelectPrevPlaylist", VestaxVCI300.getButtonPressed(value));
};

VestaxVCI300.onNavigationFwdButton = function (channel, control, value, status, group) {
	engine.setValue("[Playlist]", "SelectNextPlaylist", VestaxVCI300.getButtonPressed(value));
};

VestaxVCI300.onNavigationTabButton = function (channel, control, value, status, group) {
	engine.setValue("[Playlist]", "ToggleSelectedSidebarItem", VestaxVCI300.getButtonPressed(value));
};


//
// Engine callback functions for connected controls
// 

VestaxVCI300.leftDeck.onRateRangeValueCB = function (value) {
	VestaxVCI300.leftDeck.updateRateRange(value);
};

VestaxVCI300.rightDeck.onRateRangeValueCB = function (value) {
	VestaxVCI300.rightDeck.updateRateRange(value);
};

VestaxVCI300.leftDeck.onVUMeterValueCB = function (value) {
	VestaxVCI300.leftDeck.updateVUMeterState(value);
};

VestaxVCI300.rightDeck.onVUMeterValueCB = function (value) {
	VestaxVCI300.rightDeck.updateVUMeterState(value);
};

VestaxVCI300.leftDeck.onPFLValueCB = function (value) {
	VestaxVCI300.leftDeck.pflLED.trigger(value);
};

VestaxVCI300.rightDeck.onPFLValueCB = function (value) {
	VestaxVCI300.rightDeck.pflLED.trigger(value);
};

VestaxVCI300.leftDeck.onLoopHalveValueCB = function (value) {
	VestaxVCI300.leftDeck.autoLoopHalfLED.trigger(value);
};

VestaxVCI300.rightDeck.onLoopHalveValueCB = function (value) {
	VestaxVCI300.rightDeck.autoLoopHalfLED.trigger(value);
};

VestaxVCI300.leftDeck.onLoopDoubleValueCB = function (value) {
	VestaxVCI300.leftDeck.autoLoopDoubleLED.trigger(value);
};

VestaxVCI300.rightDeck.onLoopDoubleValueCB = function (value) {
	VestaxVCI300.rightDeck.autoLoopDoubleLED.trigger(value);
};

VestaxVCI300.leftDeck.onKeylockValueCB = function (value) {
	VestaxVCI300.leftDeck.keylockLED.trigger(value);
};

VestaxVCI300.rightDeck.onKeylockValueCB = function (value) {
	VestaxVCI300.rightDeck.keylockLED.trigger(value);
};

VestaxVCI300.leftDeck.onPeakIndicatorValueCB = function (value) {
	VestaxVCI300.leftDeck.peakIndicatorLED.trigger(value);
};

VestaxVCI300.rightDeck.onPeakIndicatorValueCB = function (value) {
	VestaxVCI300.rightDeck.peakIndicatorLED.trigger(value);
};

VestaxVCI300.leftDeck.onCueValueCB = function (value) {
	VestaxVCI300.leftDeck.updateCueState();
};

VestaxVCI300.rightDeck.onCueValueCB = function (value) {
	VestaxVCI300.rightDeck.updateCueState();
};

VestaxVCI300.leftDeck.onPlayValueCB = function (value) {
	VestaxVCI300.leftDeck.playLED.trigger(value);
};

VestaxVCI300.rightDeck.onPlayValueCB = function (value) {
	VestaxVCI300.rightDeck.playLED.trigger(value);
};

VestaxVCI300.leftDeck.onBeatSyncValueCB = function (value) {
	VestaxVCI300.leftDeck.updateBeatSyncState(value);
};

VestaxVCI300.rightDeck.onBeatSyncValueCB = function (value) {
	VestaxVCI300.rightDeck.updateBeatSyncState(value);
};

VestaxVCI300.leftDeck.onBeatActiveValueCB = function (value) {
	VestaxVCI300.leftDeck.updateBeatSyncState();
};

VestaxVCI300.rightDeck.onBeatActiveValueCB = function (value) {
	VestaxVCI300.rightDeck.updateBeatSyncState();
};

VestaxVCI300.leftDeck.onReverseValueCB = function (value) {
	VestaxVCI300.leftDeck.reverseLED.trigger(value);
};

VestaxVCI300.rightDeck.onReverseValueCB = function (value) {
	VestaxVCI300.rightDeck.reverseLED.trigger(value);
};

VestaxVCI300.leftDeck.onAutoLoopValueCB = function (value) {
	VestaxVCI300.leftDeck.updateAutoLoopState();
};

VestaxVCI300.rightDeck.onAutoLoopValueCB = function (value) {
	VestaxVCI300.rightDeck.updateAutoLoopState();
};

VestaxVCI300.leftDeck.onCueInValueCB[0] = function (value) {
	VestaxVCI300.leftDeck.updateCueInState(0);
};

VestaxVCI300.rightDeck.onCueInValueCB[0] = function (value) {
	VestaxVCI300.rightDeck.updateCueInState(0);
};

VestaxVCI300.leftDeck.onCueInValueCB[1] = function (value) {
	VestaxVCI300.leftDeck.updateCueInState(1);
};

VestaxVCI300.rightDeck.onCueInValueCB[1] = function (value) {
	VestaxVCI300.rightDeck.updateCueInState(1);
};

VestaxVCI300.leftDeck.onCueInValueCB[2] = function (value) {
	VestaxVCI300.leftDeck.updateCueInState(2);
};

VestaxVCI300.rightDeck.onCueInValueCB[2] = function (value) {
	VestaxVCI300.rightDeck.updateCueInState(2);
};

VestaxVCI300.leftDeck.onOutLoopValueCB[0] = function (value) {
	VestaxVCI300.leftDeck.updateOutLoopState(0);
};

VestaxVCI300.rightDeck.onOutLoopValueCB[0] = function (value) {
	VestaxVCI300.rightDeck.updateOutLoopState(0);
};

VestaxVCI300.leftDeck.onOutLoopValueCB[1] = function (value) {
	VestaxVCI300.leftDeck.updateOutLoopState(1);
};

VestaxVCI300.rightDeck.onOutLoopValueCB[1] = function (value) {
	VestaxVCI300.rightDeck.updateOutLoopState(1);
};

VestaxVCI300.leftDeck.onOutLoopValueCB[2] = function (value) {
	VestaxVCI300.leftDeck.updateOutLoopState(2);
};

VestaxVCI300.rightDeck.onOutLoopValueCB[2] = function (value) {
	VestaxVCI300.rightDeck.updateOutLoopState(2);
};

