////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
////////////////////////////////////////////////////////////////////////


/***********************************************************************
 * Vestax VCI-300 controller script
 * Author: Uwe Klotz a/k/a tapir
 *
 * All controls should work as expected.
 *
 * Technical details
 * -----------------
 * - High-res jog wheels (1600 steps per revolution) for scratching and
 *   pitch bending
 * - High-res pitch sliders (9-bit = 512 steps)
 * - VU meters display left/right channel volume (incl. peak indicator)
 *
 * Quick Reference
 * ---------------
 * -          Cue          = cue_default
 * - Shift  + Cue          = set the cue point (while playing)
 * -   "       "           = clear the cue point (while not playing) or
 *                           jump back to beginning of track if not cue
 *                           point has been set or load the currently
 *                           selected track if no track is loaded
 *                           Auto DJ: Skip next track
 * -          Play         = play
 * - Shift  + Play         = stutter play
 *                           Auto DJ: Fade now
 * -          Cue [1-3]/In = hotcue
 * -          Out 1/Loop   = loop in (Shift = clear)
 * -          Out 2/Loop   = loop out (Shift = clear)
 * -          Out 3/Loop   = enable/disable loop
 * -          Auto Loop    = enter/exit beatloop (default: 4 beats)
 * - Shift  + Auto Loop    = enter/exit beatlooproll (default: 4 beats)
 * - Scroll + Auto Loop    = reset number of beats to 4 if beatloop not active
 * -          Pitch Shift  = pitch bend
 * - Shift  + Pitch Shift  = fine tune the pitch +/-0.01
 * -          Half         = halve loop length
 * - Shift  + Half         = jump to start of track (while not playing)
 * - Scroll + Half         = seek backward (while not playing)
 * -          Double       = double loop length
 * - Shift  + Double       = jump to end of track (while not playing)
 * - Scroll + Double       = seek forward (while not playing)
 * -          Auto Tempo   = toggle sync (push-and-hold)
 * -          Keylock      = toggle keylock
 * - Shift  + Keylock      = reset pitch to 0.00% (quartz)
 * -          Censor       = play backwards while button is pressed
 *                           (song continues playing muted in the
 *                           background)
 * - Shift  + Censor       = toggle filter effect that is controlled
 *                           by the Mid EQ knob
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
 * - Keylock is turned off when starting to scratch to avoid unnatural
 *   noise. It remains turned off even when scratching is disabled,
 *   because otherwise the engine produces audible glitches.
 * - ITCH-like looping seems impossible with Mixxx 1.x controls(?)
 * - TODO: Implement soft-takeover for "rate"
 * - TODO: Crates/Files/Browse buttons are connected but currently unused
 * - TODO: Line fader curve knob is connected but currently unused
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
 * 2013-05-30 - Ignore jog wheel movements unless the jog wheel surface is
 *              touched. Exception from this rule: Nudging when playing in
 *              scratch mode is still possible without touching the sensor
 *              plate!
 *            - Different jog sensitivity parameters for cueing and nudging
 *              (tempo)
 *            - Increased jog sensitivity for both cueing and nudging
 * 2013-10-04 Borrow some ideas from Serato DJ (with slight variations)
 *            - Stutter play (Shift + Play)
 *            - Pitch bend (Pitch Shift)
 *            - Pitch fine-tune (Shift + Pitch Shift)
 * 2013-10-05 Bugfix
 *            - Fix non-working backspin (regression introduced with
 *              version 2013-05-30)
 * 2013-10-05.1 Bugfix
 *            - Fix behavior of stutter play using the latest position
 *              where playing started. The previous implementation was
 *              completely different from that found in Serato DJ.
 *            - Some minor changes to reflect the values that would
 *              need to be reset on track load. Unfortunately there's
 *              no appropriate callback in Mixxx to invoke this function.
 * 2013-10-06 Full-featured stutter play implementation
 *            - Stutter play now takes the cue point and also triggering
 *              of hotcues into account.
 *            - Update documentation
 *            - Internal refactoring and minor fixes
 * 2013-10-07 Improve stutter play implementation
 *            - Adjust the stutter play position when setting a new cue
 *              point on the fly by pressing Shift + Cue while playing.
 * 2013-10-12 Adjust and fine-tune parameters
 *            - Exactly measured the and adjusted jog resolution
 *              parameters. It's only 1600 instead of 1664 steps per
 *              revolution!! This was wrong since the initial version :(
 *            - Synchronized jog feedback between scratching and cueing
 *              by using a linear characteristic
 *            - Slightly increased sensitivity for jog scrolling
 *            - Some code cleanup and simplification
 * 2014-03-22 Beat sync/active changes and cleanup for 1.12
 *            - Change mapping of the Auto Tempo / Sync button: Sync
 *              is now only triggered in combination with Shift, while
 *              pressing the button without the Shift key toggles
 *              quantize on/off. Before it was just to easy to
 *              accidentilly trigger Sync by hitting the wrong button!
 *            - Disable the beat active indicator LED, because it has
 *              not proven to be helpful.
 *            - Reduce jog tempo sensitivity considerably
 *            - Some minor cleanups before adding new features for 1.12
 * 2014-06-20 Major rework for 1.12
 *            - Replace script binding with direct MIDI mapping wherever
 *              possible
 *            - Use new cue/play controls
 *            - Rework looping
 * 2014-06-20.1 Replace reverse play with filter effect
 *            - Assign 1st/2nd effect unit with preselected Filter
 *              chain to left/right deck respectively
 *            - Shift + Censor button enables/disables the filter effect
 *            - Mid EQ controls the filter parameter while the filter is
 *              enabled
 * 2014-06-21 Smart switching between mid EQ and filter effect
 *            - Track the knob's value and restore it when switching
 *              between mid EQ and filter effect
 * 2014-07-13 Use new 1.12 sync controls
 *            - The "Auto Tempo" button now triggers the new "sync_enabled"
 *              control. Tapping the BPM manually by holding "Scroll" is
 *              still possible.
 * 2014-08-18 Sync mapping with push-and-hold
 *            - Use MIDI mapping for "sync_enabled" to support push-and-hold
 *            - Use setParameter() instead of setValue() for effect parameters
 * 2015-01-16 Filter effect parameter update
 *            - Rename filter effect parameter from "parameter"
 *              to "super1"
 * 2015-05-03 Update for version 1.12
 *            - Use new EqualizerRack
 *            - Connect filter effect to QuickEffectRack
 *            - Improve scratching experience
 * ...to be continued...
 **********************************************************************/


function VestaxVCI300() {}

VestaxVCI300.group = "[Master]";

VestaxVCI300.JOG_RESOLUTION = 1600; // Steps per revolution: (0x0C << 7) + 0x40 = 1536 + 64 = 1600

// At jogCueSensitivityScale = 63.0 and jogCueSensitivityPow the
// virtual platter in scratch mode is almost synchronized with the
// jog wheel of the VCI-300 when spinning the rim without actually
// touching the platter surface (-> same behaviour as in cue mode)
VestaxVCI300.jogCueSensitivityScale = 63.0 / VestaxVCI300.JOG_RESOLUTION; // best match(?)
VestaxVCI300.jogCueSensitivityPow = 1.0; // 1.0 = linear (like scratching)

// You might adjust the jog sensitivity for pitch bending according to
// your personal preferences.
VestaxVCI300.jogTempoSensitivityScale = 8.0 / VestaxVCI300.JOG_RESOLUTION; /*TUNABLE PARAM*/
VestaxVCI300.jogTempoSensitivityPow = 0.6; // 1.0 = linear /*TUNABLE PARAM*/

VestaxVCI300.JOG_SCRATCH_RPM = 33.0 + (1.0 / 3.0); // 33 1/3 /*TUNABLE PARAM*/
VestaxVCI300.JOG_SCRATCH_ALPHA = 1.0 / 8.0; /*TUNABLE PARAM*/
VestaxVCI300.JOG_SCRATCH_BETA = VestaxVCI300.JOG_SCRATCH_ALPHA / 32.0; /*TUNABLE PARAM*/
VestaxVCI300.JOG_SCRATCH_RAMP = true;
VestaxVCI300.JOG_SCRATCH2_ABS_MIN = 0.01;
VestaxVCI300.JOG_SCRATCH2_PLAY_MIN = -0.7;
VestaxVCI300.JOG_SCRATCH2_PLAY_MAX = 1.0;
VestaxVCI300.JOG_SCRATCH_TIMEOUT = 20; // in milliseconds

VestaxVCI300.jogFastTrackSearchScale = 3.0 / VestaxVCI300.JOG_RESOLUTION; /*TUNABLE PARAM*/

VestaxVCI300.jogScrollBias = 0.0; // Initialize jog scroll
VestaxVCI300.jogScrollDeltaStepsPerTrack = 8; // 1600 / 8 = 200 tracks per revolution /*TUNABLE PARAM*/

VestaxVCI300.pitchFineTuneStepPercent100 = 1; // = 1/100 %

// Mixxx constants
VestaxVCI300.MIXXX_JOG_RANGE = 3.0; // -3.0 <= "jog" <= 3.0
VestaxVCI300.MIXXX_LOOP_POSITION_UNDEFINED = -1;

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

VestaxVCI300.decksByGroup = {};

VestaxVCI300.allLEDs = [];

VestaxVCI300.updatePitchValue = function (group, pitchHigh, pitchLow) {
	// 0x00 <= pitchHigh <= 0x7F
	// pitchLow = 0x00/0x20/0x40/0x60 -> 2 out of 7 bits
	var pitchValue = (pitchHigh << 2) | (pitchLow >> 5);
	// pitchValueMin    = 0
	// pitchValueCenter = 256
	// pitchValueMax    = 511
	if (pitchValue <= 256) {
		// negative range (incl. center): 256 steps
		engine.setValue(group, "rate", (256 - pitchValue) / 256.0);
	} else {
		// positive range: 511 - 256 = 255 steps
		engine.setValue(group, "rate", (256 - pitchValue) / 255.0);
	}
};

VestaxVCI300.initValues = function () {
	VestaxVCI300.scrollState = false;
	VestaxVCI300.numDecksBackup = engine.getValue(VestaxVCI300.group, "num_decks");
	engine.setValue(VestaxVCI300.group, "num_decks", 2);
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
	engine.connectControl(group, ctrl, func, true);
};

VestaxVCI300.connectControls = function () {
};

VestaxVCI300.disconnectControls = function () {
};

VestaxVCI300.updateScrollState = function () {
	VestaxVCI300.scrollLED.trigger(VestaxVCI300.scrollState);
};


////////////////////////////////////////////////////////////////////////
// Button utilities                                                   //
////////////////////////////////////////////////////////////////////////

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


////////////////////////////////////////////////////////////////////////
// LED utilities                                                      //
////////////////////////////////////////////////////////////////////////

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


////////////////////////////////////////////////////////////////////////
// Decks                                                              //
////////////////////////////////////////////////////////////////////////

VestaxVCI300.Deck = function (number) {
	this.number = number;
	this.group = "[Channel" + this.number + "]";
	this.filterGroup = "[QuickEffectRack1_" + this.group + "]";
	VestaxVCI300.decksByGroup[this.group] = this;
	this.shiftState = false;
	this.scratchState = true;
	this.jogTouchState = false;
	this.scratchTimer = 0;
	this.filterEnabled = false;
	this.filterMidParam = 0.5; // centered
	engine.setValue(this.filterGroup, "enabled", false);
	engine.setParameter(this.filterGroup, "super1", this.filterMidParam);
};

VestaxVCI300.leftDeck = new VestaxVCI300.Deck(1);
VestaxVCI300.rightDeck = new VestaxVCI300.Deck(2);

VestaxVCI300.Deck.prototype.isPlaying = function () {
	return engine.getValue(this.group, "play");
};

VestaxVCI300.Deck.prototype.enableScratching = function () {
	engine.scratchEnable(this.number,
		VestaxVCI300.JOG_RESOLUTION,
		VestaxVCI300.JOG_SCRATCH_RPM,
		VestaxVCI300.JOG_SCRATCH_ALPHA,
		VestaxVCI300.JOG_SCRATCH_BETA,
		VestaxVCI300.JOG_SCRATCH_RAMP);
};

VestaxVCI300.Deck.prototype.disableScratching = function () {
	if (0 !== this.scratchTimer) {
		engine.stopTimer(this.scratchTimer);
		this.scratchTimer = 0;
	}
	var scratch2 = engine.getValue(this.group, "scratch2");
	if ((!this.isPlaying() && (VestaxVCI300.JOG_SCRATCH2_ABS_MIN < Math.abs(scratch2))) ||
		(scratch2 < VestaxVCI300.JOG_SCRATCH2_PLAY_MIN) ||
		(scratch2 > VestaxVCI300.JOG_SCRATCH2_PLAY_MAX)) {
		var timeoutCallback =
			"VestaxVCI300.onScratchingTimeoutDeck" + this.number + "()";
		this.scratchTimer = engine.beginTimer(
			VestaxVCI300.JOG_SCRATCH_TIMEOUT,
			timeoutCallback,
			true);
	}
	if (0 === this.scratchTimer) {
		// Ramping only when doing a back-spin while playing
		var ramping = this.isPlaying() && (scratch2 < 0.0);
		engine.scratchDisable(this.number, ramping && VestaxVCI300.JOG_SCRATCH_RAMP);
	}
};

VestaxVCI300.Deck.prototype.onScratchingTimeout = function () {
	this.scratchTimer = 0;
	this.disableScratching();
};

VestaxVCI300.onScratchingTimeoutDeck1 = function () {
	var deck = VestaxVCI300.decksByGroup["[Channel1]"];
	deck.onScratchingTimeout();
};

VestaxVCI300.onScratchingTimeoutDeck2 = function () {
	var deck = VestaxVCI300.decksByGroup["[Channel2]"];
	deck.onScratchingTimeout();
};

VestaxVCI300.Deck.prototype.updateJogValue = function (jogHigh, jogLow) {
	// 0x00 <= jogHigh/jogLow <= 0x7F
	var jogValue = (jogHigh << 7) | jogLow;
	// 0x0000 <= jogValue <= 0x3FFF (14-bit, cyclic)
	if (undefined !== this.jogValue) {
		this.jogDelta = jogValue - this.jogValue;
		if (this.jogDelta >= 0x2000) {
			// cyclic carry-over
			this.jogDelta -= 0x4000;
		} else if (this.jogDelta < -0x2000) {
			// cyclic carry-over
			this.jogDelta += 0x4000;
		}
		this.jogMoveLED.trigger(0 !== this.jogDelta);
		// reset jog scroll bias with every jog movement
		var jogScrollBias = VestaxVCI300.jogScrollBias;
		VestaxVCI300.jogScrollBias = 0.0;
		var isPlaying = this.isPlaying();
		if (this.shiftState && !isPlaying) {
			// fast track search
			var playposition = engine.getValue(this.group, "playposition");
			if (undefined !== playposition) {
				var searchpos = playposition + (this.jogDelta * VestaxVCI300.jogFastTrackSearchScale);
				engine.setValue(this.group, "playposition", Math.max(0.0, Math.min(1.0, searchpos)));
			}
		} else if (VestaxVCI300.scrollState && !isPlaying) {
			// scroll playlist
			var jogScrollDelta;
			jogScrollDelta = jogScrollBias + (this.jogDelta / VestaxVCI300.jogScrollDeltaStepsPerTrack);
			var jogScrollDeltaRound;
			jogScrollDeltaRound = Math.round(jogScrollDelta);
			engine.setValue(
				"[Playlist]",
				"SelectTrackKnob",
				jogScrollDeltaRound);
			// store the remainder it take it into account next time
			VestaxVCI300.jogScrollBias = jogScrollDelta - jogScrollDeltaRound;
		} else {
			if (engine.isScratching(this.number)) {
				// scratching
				engine.scratchTick(this.number, this.jogDelta);
				if (!this.jogTouchState) {
					this.disableScratching();
				}
			} else {
				// jog movement
				var jogSensitivityScale;
				var jogSensitivityPow;
				if (isPlaying) {
					jogSensitivityScale = VestaxVCI300.jogTempoSensitivityScale;
					jogSensitivityPow = VestaxVCI300.jogTempoSensitivityPow;
				} else {
					jogSensitivityScale = VestaxVCI300.jogCueSensitivityScale;
					jogSensitivityPow = VestaxVCI300.jogCueSensitivityPow;
				}
				var jogDeltaSign;
				if (0 > this.jogDelta) {
					jogDeltaSign = -1;
				} else {
					jogDeltaSign = 1;
				}
				var normalizedAbsJogDelta =  Math.pow(Math.abs(this.jogDelta) * jogSensitivityScale, jogSensitivityPow);
				engine.setValue(this.group, "jog", jogDeltaSign * normalizedAbsJogDelta * VestaxVCI300.MIXXX_JOG_RANGE);
			}
		}
	}
	this.jogValue = jogValue;
};

VestaxVCI300.Deck.prototype.updateShiftState = function () {
	this.shiftLED.trigger(this.shiftState);
};

VestaxVCI300.Deck.prototype.updateScratchState = function () {
	this.scratchLED.trigger(this.scratchState);
	if (this.scratchState && this.jogTouchState) {
		this.enableScratching();
	} else {
		this.disableScratching();
	}
};

VestaxVCI300.Deck.prototype.updateRateRange = function () {
	var rateRangeValue = engine.getValue(this.group, "rateRange");
	this.rateRangePercent100 = Math.round(10000.0 * rateRangeValue);
	// Workaround: Explicitly (re-)set "rate" after changing "rateRange"!
	// Otherwise the first button press on one of the pitch shift buttons
	// is ignored.
	var rateValue = engine.getValue(this.group, "rate");
	engine.setValue(this.group, "rate", rateValue);
};

VestaxVCI300.Deck.prototype.updateSyncState = function () {
	var syncValue =
		engine.getValue(this.group, "sync_enabled") ||
		engine.getValue(this.group, "bpm_tap");
	this.syncLED.trigger(syncValue);
};

VestaxVCI300.Deck.prototype.initValues = function () {
	this.rateDirBackup = engine.getValue(this.group, "rate_dir");
	engine.setValue(this.group, "rate_dir", -1);
	engine.setValue(this.group, "keylock", true);
	this.onTrackUnload();
};

VestaxVCI300.Deck.prototype.onTrackUnload = function () {
	// Perform some cleanup and reset values whenever
	// a track is unloaded from this deck.
	this.jogHighValue = undefined;
	this.jogValue = undefined;
	this.jogDelta = undefined;
	this.disableScratching();
	this.autoLoopBeatsIndex = VestaxVCI300.defaultAutoLoopBeatsIndex;
};

VestaxVCI300.Deck.prototype.isTrackLoaded = function () {
	return 0 < engine.getValue(this.group, "track_samples");
};

VestaxVCI300.Deck.prototype.restoreValues = function () {
	engine.setValue(this.group, "rate_dir", this.rateDirBackup);
};

VestaxVCI300.Deck.prototype.connectControls = function () {
	VestaxVCI300.connectControl(this.group, "rateRange", this.onRateRangeValueCB);
	VestaxVCI300.connectControl(this.group, "loop_halve", this.onLoopHalveValueCB);
	VestaxVCI300.connectControl(this.group, "loop_double", this.onLoopDoubleValueCB);
	VestaxVCI300.connectControl(this.group, "sync_enabled", this.onSyncValueCB);
	VestaxVCI300.connectControl(this.group, "reverseroll", this.onCensorFilterValueCB);
	VestaxVCI300.connectControl(this.filterGroup, "enabled", this.onCensorFilterValueCB);
	for (var beatsIndex in VestaxVCI300.autoLoopBeatsArray) {
		VestaxVCI300.connectControl(this.group, "beatloop_" + VestaxVCI300.autoLoopBeatsArray[beatsIndex] + "_enabled", this.onAutoLoopValueCB);
	}
};

VestaxVCI300.Deck.prototype.disconnectControls = function () {
	VestaxVCI300.disconnectControl(this.group, "rateRange");
	VestaxVCI300.disconnectControl(this.group, "loop_halve");
	VestaxVCI300.disconnectControl(this.group, "loop_double");
	VestaxVCI300.disconnectControl(this.group, "sync_enabled");
	VestaxVCI300.disconnectControl(this.group, "bpm_tap");
	VestaxVCI300.disconnectControl(this.group, "reverseroll");
	VestaxVCI300.disconnectControl(this.filterGroup, "enabled");
	for (var beatsIndex in VestaxVCI300.autoLoopBeatsArray) {
		var autoLoopBeats = VestaxVCI300.autoLoopBeatsArray[beatsIndex];
		VestaxVCI300.disconnectControl(this.group, "beatloop_" + autoLoopBeats + "_enabled");
	}
};

VestaxVCI300.Deck.prototype.isAutoLoopEnabled = function () {
	var autoLoopBeats = VestaxVCI300.autoLoopBeatsArray[this.autoLoopBeatsIndex];
	return engine.getValue(this.group, "beatloop_" + autoLoopBeats + "_enabled");
};

VestaxVCI300.Deck.prototype.updateAutoLoopState = function () {
	this.autoLoopLED.trigger(this.isAutoLoopEnabled());
};

VestaxVCI300.Deck.prototype.hasLoopStart = function () {
	return VestaxVCI300.MIXXX_LOOP_POSITION_UNDEFINED != engine.getValue(this.group, "loop_start_position");
};

VestaxVCI300.Deck.prototype.hasLoopEnd = function () {
	return VestaxVCI300.MIXXX_LOOP_POSITION_UNDEFINED != engine.getValue(this.group, "loop_end_position");
};

VestaxVCI300.Deck.prototype.hasLoopStart = function () {
	return VestaxVCI300.MIXXX_LOOP_POSITION_UNDEFINED != engine.getValue(this.group, "loop_start_position");
};

VestaxVCI300.Deck.prototype.hasLoopEnd = function () {
	return VestaxVCI300.MIXXX_LOOP_POSITION_UNDEFINED != engine.getValue(this.group, "loop_end_position");
};

VestaxVCI300.Deck.prototype.hasLoop = function () {
	return this.hasLoopStart() && this.hasLoopEnd();
};

VestaxVCI300.Deck.prototype.deleteLoopStart = function () {
	engine.setValue(this.group, "loop_start_position", VestaxVCI300.MIXXX_LOOP_POSITION_UNDEFINED);
};

VestaxVCI300.Deck.prototype.deleteLoopEnd = function () {
	engine.setValue(this.group, "loop_end_position", VestaxVCI300.MIXXX_LOOP_POSITION_UNDEFINED);
};

VestaxVCI300.Deck.prototype.deleteLoop = function () {
	// loop end has to be deleted before loop start
	this.deleteLoopEnd();
	this.deleteLoopStart();
};

VestaxVCI300.Deck.prototype.toggleLoop = function () {
	engine.setValue(this.group, "reloop_exit", true);
};

VestaxVCI300.Deck.prototype.onAutoLoopButtonPressed = function () {
	if (this.isAutoLoopEnabled()) {
		this.toggleLoop();
	} else {
		this.autoLoopBeatsIndex = VestaxVCI300.defaultAutoLoopBeatsIndex;
		var autoLoopBeats = VestaxVCI300.autoLoopBeatsArray[this.autoLoopBeatsIndex];
		if (this.shiftState) {
			engine.setValue(this.group, "beatlooproll_" + autoLoopBeats + "_activate", true);
		} else {
			engine.setValue(this.group, "beatloop_" + autoLoopBeats + "_activate", true);
		}
	}
	this.updateAutoLoopState();
};

VestaxVCI300.Deck.prototype.triggerCensorFilter = function () {
	this.censorLED.trigger(
		engine.getValue(this.group, "reverseroll") ||
		engine.getValue(this.filterGroup, "enabled"));
};


VestaxVCI300.Deck.prototype.setFilterMidValue = function (value) {
	this.setFilterMidParam(script.absoluteLin(value, 0.0, 1.0));
};

VestaxVCI300.Deck.prototype.setFilterMidParam = function (param) {
	this.filterMidParam = param;
	if (engine.getValue(this.filterGroup, "enabled")) {
		engine.setParameter(this.filterGroup, "super1", param);
	} else {
		engine.setParameter(this.group, "filterMid", param);
	}
};


////////////////////////////////////////////////////////////////////////
// Mixxx callback functions                                           //
////////////////////////////////////////////////////////////////////////

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
	VestaxVCI300.leftDeck.syncLED =
		new VestaxVCI300.LED(0x25);
	VestaxVCI300.leftDeck.jogMoveLED =
		new VestaxVCI300.LED(0x75);
	VestaxVCI300.leftDeck.jogTouchLED =
		new VestaxVCI300.LED(0x76);
	VestaxVCI300.leftDeck.censorLED =
		new VestaxVCI300.LED(0x28);
	VestaxVCI300.leftDeck.autoLoopLED =
		new VestaxVCI300.LED(0x29);
	VestaxVCI300.leftDeck.pitchShiftDownLED =
		new VestaxVCI300.LED(0x3B);
	VestaxVCI300.leftDeck.pitchShiftUpLED =
		new VestaxVCI300.LED(0x3C);
	VestaxVCI300.leftDeck.autoLoopHalfLED =
		new VestaxVCI300.LED(0x2A);
	VestaxVCI300.leftDeck.autoLoopDoubleLED =
		new VestaxVCI300.LED(0x2B);
	VestaxVCI300.rightDeck.shiftLED =
		new VestaxVCI300.LED(0x37);
	VestaxVCI300.rightDeck.scratchLED =
		new VestaxVCI300.LED(0x38);
	VestaxVCI300.rightDeck.syncLED =
		new VestaxVCI300.LED(0x39);
	VestaxVCI300.rightDeck.jogMoveLED =
		new VestaxVCI300.LED(0x77);
	VestaxVCI300.rightDeck.jogTouchLED =
		new VestaxVCI300.LED(0x78);
	VestaxVCI300.rightDeck.censorLED =
		new VestaxVCI300.LED(0x34);
	VestaxVCI300.rightDeck.autoLoopLED =
		new VestaxVCI300.LED(0x35);
	VestaxVCI300.rightDeck.pitchShiftDownLED =
		new VestaxVCI300.LED(0x3D);
	VestaxVCI300.rightDeck.pitchShiftUpLED =
		new VestaxVCI300.LED(0x3E);
	VestaxVCI300.rightDeck.autoLoopHalfLED =
		new VestaxVCI300.LED(0x32);
	VestaxVCI300.rightDeck.autoLoopDoubleLED =
		new VestaxVCI300.LED(0x36);
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


////////////////////////////////////////////////////////////////////////
// Controller callback functions (see also: Vestax-VCI-300-midi.xml)  //
////////////////////////////////////////////////////////////////////////

VestaxVCI300.onCueInButton = function (group, hotcue, value) {
	var deck = VestaxVCI300.decksByGroup[group];
	var hotcuePrefix = "hotcue_" + hotcue;
	if (deck.shiftState) {
		engine.setValue(group, hotcuePrefix + "_clear", VestaxVCI300.getButtonPressed(value));
	} else {
		engine.setValue(group, hotcuePrefix + "_activate", VestaxVCI300.getButtonPressed(value));
	}
};

VestaxVCI300.onOutLoopButton = function (group, index, value) {
	if (VestaxVCI300.getButtonPressed(value)) {
		var deck = VestaxVCI300.decksByGroup[group];
		switch (index) {
			case 0:
				if (deck.shiftState) {
					deck.deleteLoop();
				} else {
					engine.setValue(group, "loop_in", true);
				}
				break;
			case 1:
				if (deck.shiftState) {
					deck.deleteLoopEnd();
				} else {
					engine.setValue(group, "loop_out", true);
				}
				break;
			case 2:
				deck.toggleLoop();
				break;
		}
		deck.updateAutoLoopState();
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
	if (deck.isTrackLoaded()) {
		if (deck.shiftState) {
			if (VestaxVCI300.getButtonPressed(value)) {
				if (engine.getValue("[AutoDJ]", "enabled")) {
					engine.setValue("[AutoDJ]", "skip_next", true);
				} else {
					if (deck.isPlaying()) {
						// move cue point
						engine.setValue(group, "cue_set", true);
					} else {
						if (0.0 < engine.getValue(group, "cue_point")) {
							// clear cue point
							engine.setValue(group, "cue_point", 0.0);
						} else {
							// no cue point -> jump to beginning of track
							engine.setValue(group, "playposition", 0.0);
						}
					}
				}
			}
		} else {
			engine.setValue(group, "cue_default", VestaxVCI300.getButtonPressed(value));
		}
	} else {
		if (VestaxVCI300.getButtonPressed(value)) {
			engine.setValue(group, "LoadSelectedTrack", true);
		}
	}
};

VestaxVCI300.onPlayButton = function (channel, control, value, status, group) {
	if (VestaxVCI300.getButtonPressed(value)) {
		var deck = VestaxVCI300.decksByGroup[group];
		if (deck.isTrackLoaded()) {
			if (deck.shiftState) {
				if (engine.getValue("[AutoDJ]", "enabled")) {
					engine.setValue("[AutoDJ]", "fade_now", true);
				} else {
					engine.setValue(group, "play_stutter", true);
				}
			} else {
				VestaxVCI300.onToggleButton(group, "play", value);
			}
		} else {
			if (VestaxVCI300.getButtonPressed(value)) {
				engine.setValue(group, "LoadSelectedTrack", true);
			}
		}
	}
};

VestaxVCI300.onCensorFilterButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (deck.shiftState) {
		// filter
		if (VestaxVCI300.getButtonPressed(value)) {
			// enable/disable filter
			VestaxVCI300.toggleBinaryValue(deck.filterGroup, "enabled");
			// update parameter
			deck.setFilterMidParam(deck.filterMidParam);
		}
	} else {
		// censor
		engine.setValue(group, "reverseroll", VestaxVCI300.getButtonPressed(value));
	}
};

VestaxVCI300.onKeyLockButton = function (channel, control, value, status, group) {
	if (VestaxVCI300.getButtonPressed(value)) {
		var deck = VestaxVCI300.decksByGroup[group];
		if (deck.shiftState) {
			// quartz lock
			engine.setValue(group, "rate", 0.0);
		} else {
			VestaxVCI300.toggleBinaryValue(group, "keylock");
		}
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
		if (deck.isPlaying()) {
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
			if (!deck.isPlaying()) {
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
		if (deck.isPlaying()) {
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
			if (!deck.isPlaying()) {
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
	if (VestaxVCI300.getButtonPressed(value)) {
		if (deck.shiftState) {
			// pitch fine-tune
			VestaxVCI300.adjustRate(deck, -VestaxVCI300.pitchFineTuneStepPercent100);
		} else {
			// enable pitch bend
			engine.setValue(deck.group, "rate_temp_down", true);
		}
	} else {
		// disable pitch bend
		engine.setValue(deck.group, "rate_temp_down", false);
	}
	deck.pitchShiftDownLED.trigger(VestaxVCI300.getButtonPressed(value));
};

VestaxVCI300.onPitchShiftUpButton = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	if (VestaxVCI300.getButtonPressed(value)) {
		if (deck.shiftState) {
			// pitch fine-tune
			VestaxVCI300.adjustRate(deck, VestaxVCI300.pitchFineTuneStepPercent100);
		} else {
			// enable pitch bend
			engine.setValue(deck.group, "rate_temp_up", true);
		}
	} else {
		// disable pitch bend
		engine.setValue(deck.group, "rate_temp_up", false);
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
	if (undefined !== deck.jogHighValue) {
		deck.updateJogValue(deck.jogHighValue, value);
		// ensure that jogHighValue could never be read twice
		deck.jogHighValue = undefined;
	}
};

VestaxVCI300.onCue1InButton = function (channel, control, value, status, group) {
	VestaxVCI300.onCueInButton(group, 1, value);
};

VestaxVCI300.onCue2InButton = function (channel, control, value, status, group) {
	VestaxVCI300.onCueInButton(group, 2, value);
};

VestaxVCI300.onCue3InButton = function (channel, control, value, status, group) {
	VestaxVCI300.onCueInButton(group, 3, value);
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
	if (VestaxVCI300.getButtonPressed(value)) {
		var deck = VestaxVCI300.decksByGroup[group];
		deck.onAutoLoopButtonPressed();
	}
};

VestaxVCI300.onFilterMidKnob = function (channel, control, value, status, group) {
	var deck = VestaxVCI300.decksByGroup[group];
	deck.setFilterMidValue(value);
};

VestaxVCI300.onCrossfaderCurve = function (channel, control, value, status, group) {
	script.crossfaderCurve(value);
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


////////////////////////////////////////////////////////////////////////
// Engine callback functions for connected controls                   //
////////////////////////////////////////////////////////////////////////

VestaxVCI300.leftDeck.onRateRangeValueCB = function (value, group, control) {
	VestaxVCI300.leftDeck.updateRateRange(value);
};

VestaxVCI300.rightDeck.onRateRangeValueCB = function (value, group, control) {
	VestaxVCI300.rightDeck.updateRateRange(value);
};

VestaxVCI300.leftDeck.onLoopHalveValueCB = function (value, group, control) {
	VestaxVCI300.leftDeck.autoLoopHalfLED.trigger(value);
};

VestaxVCI300.rightDeck.onLoopHalveValueCB = function (value, group, control) {
	VestaxVCI300.rightDeck.autoLoopHalfLED.trigger(value);
};

VestaxVCI300.leftDeck.onLoopDoubleValueCB = function (value, group, control) {
	VestaxVCI300.leftDeck.autoLoopDoubleLED.trigger(value);
};

VestaxVCI300.rightDeck.onLoopDoubleValueCB = function (value, group, control) {
	VestaxVCI300.rightDeck.autoLoopDoubleLED.trigger(value);
};

VestaxVCI300.leftDeck.onSyncValueCB = function (value, group, control) {
	VestaxVCI300.leftDeck.updateSyncState(value);
};

VestaxVCI300.rightDeck.onSyncValueCB = function (value, group, control) {
	VestaxVCI300.rightDeck.updateSyncState(value);
};

VestaxVCI300.leftDeck.onCensorFilterValueCB = function (value, group, control) {
	VestaxVCI300.leftDeck.triggerCensorFilter();
};

VestaxVCI300.rightDeck.onCensorFilterValueCB = function (value, group, control) {
	VestaxVCI300.rightDeck.triggerCensorFilter();
};

VestaxVCI300.leftDeck.onAutoLoopValueCB = function (value, group, control) {
	VestaxVCI300.leftDeck.updateAutoLoopState();
};

VestaxVCI300.rightDeck.onAutoLoopValueCB = function (value, group, control) {
	VestaxVCI300.rightDeck.updateAutoLoopState();
};
