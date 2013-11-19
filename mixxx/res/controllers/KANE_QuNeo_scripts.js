/* Midi scripts for Kane's Quneo presets. Fills out some LED's, provides rotary
   scratching/playing, and (2000+ lines of) other things. */

function KANE_QuNeo () {}

/* BEGIN INDEX

   1) Init

       (GV) Global Variables
       (IS) Initialization and Shutdown

   2) Toggling

       (JS) Toggling JumpSync
       (JD) Toggling Jump Direction
       (L)  Toggling Looping
       (PS) Toggling PlayScratch
       (R)  Toggling Record
       (SC) Slider Cycle

   3) Functionality
   
       (JL) Jump, Sync, and/or Loop over 1,2,4,8 Beats
       (VS) Vertical Sliders
       (ZC) Zoom and Cursor
       (VN) Visual Nudging
       (RN) Periodic and Regular Rate Nudging
       (H)  Hotcues
       (HA) Horizontal Arrows
       (P)  Playlist Mode
       (RB) Reset Beat
       (QC) Quantize Cues
       (TK) Time Keeping
       (B)  Beat Handling
       (U)  Utilities

   4) LEDs

       (BLED) Beat LEDs
       (JLED) Jump LEDs
       (RLED) Rotary LEDs
       (ALED) LED Assertions
       (ELED) Engine LED Connections

   5) Dispatches

       (VSD) Vertical Slider Dispatches
       (AD) LED Assertion Dispatches
       (DD) Deck Dispatches
       (SD) Sampler Dispatches
   
END INDEX*/

// -------------------------------------
// begin init section
// -------------------------------------

/***** (GV) Global Variables *****/

// Settings

KANE_QuNeo.assertionDelayTimer = 60; // default time(ms) to wait on asserting LEDs
KANE_QuNeo.jumpLoopTimer = 70; // time(ms) to wait before executing a jumped loop 
KANE_QuNeo.jumpSyncTimer = 30; // time(ms) to wait before executing a jumped sync 
KANE_QuNeo.beatOffset = 70; // time(ms) to wait before signaling a beat, used
// to synchronize LED's and actual audible beat

KANE_QuNeo.minBrightness = .2 // minimum value for flashing LEDs, 0...1
KANE_QuNeo.numDecks = 8 // number of decks we are supporting. currently 4 decks +
                        // 4 samplers = 8 total decks
KANE_QuNeo.doubleTapWindow = 700; // time(ms) window in which to consider
                                  // two note presses as a double tap
KANE_QuNeo.numHotcues = 16; // total hotcues we are currently supporting
KANE_QuNeo.pregain = 1.0 // initialize deck gains to this value
KANE_QuNeo.scratchSpeed = 5.0 // Scratch ticks per rotary sense
KANE_QuNeo.rateNudgeHoldTime = 500 // time(ms) to hold nudges for scroll toggle
KANE_QuNeo.jumpHoldTime = 70 // time(ms) to hold for continued jumping
KANE_QuNeo.visualNudgeHoldTime = 500 // time(ms) to hold nudges for scroll toggle
KANE_QuNeo.visualNudgeDist = .0000005 // setting (%) for how far each press of visual
                                   // nudge moves the current play position
KANE_QuNeo.checkLoopDelay = 50; // time(ms) to wait before checking for a loop break
KANE_QuNeo.mode = 13; // assumes we start in mode 13 on the quneo
KANE_QuNeo.totalBeats = 16; // number of beats over which to sequence the LEDs
KANE_QuNeo.autoJumpSpeed = 300; // time(ms) to wait for each tick of auto jump
KANE_QuNeo.visualNudgeSpeed = 20; // time(ms) to wait for each tick while scrolling
KANE_QuNeo.rateNudgeSpeed = 800; // ms to wait between each auto nudge
KANE_QuNeo.rateNudgeTolerance = .006251/2 // determines how close rate must
                               // be to 0 in order to trigger turning off auto nudge

// LED Sequencers for each deck - easy to edit!
// simply choose which LEDs you want for each
// beat, then update it with the appropriate midi note!
// LED midi note mappings for the quneo are found in the quneo's full manual
// playerSequence must return a vector of hex numbers.
// Quarter comes in 0, 1/4, 1/2, 3/4.
KANE_QuNeo.playerSequence = function (deck, beat, quarter) {
    // Here is deck 1's sequence
    if (deck == 1) { // do deck1 sequence
	var middleGreen = [0x64,0x54,0x52,0x62]
	var middleRed = [0x65,0x55,0x53,0x63]
	var middleOrange = middleGreen.concat(middleRed)
	if (quarter == 0) { // for whole beats
	    switch (beat) {
	    case 1: // LEDs to turn on for beat 1 of 8
		return [0x73,0x75,0x43,0x45].concat(middleRed);
	    case 2: case 4: case 6: case 8: case 11: case 13: case 15:
		return [0x60,0x50,0x66,0x56].concat(middleOrange);
	    case 3: case 5: case 7: case 10: case 12: case 14: case 16: case 0:
		return [0x72,0x74,0x42,0x44].concat(middleOrange);
	    case 9:
		return [0x61,0x51,0x67,0x57].concat(middleRed);
	    }
	}
	else if (quarter == 1/2) {
	    switch (beat) {
	    case 8:
		return [0x70,0x72,0x74,0x76,0x40,0x42,0x44,0x46,0x73,0x75,0x43,0x45]
	    case 16:
		return [0x70,0x60,0x50,0x40,0x76,0x66,0x56,0x46,0x61,0x51,0x67,0x57]
	    default:
		return middleGreen
	    }
	}
    }

    // Here is deck 2's sequence
    else if (deck == 2) { // do deck2 sequence
	var middleGreen = [0x6a,0x6c,0x5a,0x5c]
	var middleRed = [0x6b,0x6d,0x5b,0x5d]
	var middleOrange = middleGreen.concat(middleRed)
	if (quarter == 0) { // for whole beats
	    switch (beat) {
	    case 1: // LEDs to turn on for beat 1 of 8
		return [0x7b,0x7d,0x4b,0x4d].concat(middleRed);
	    case 2: case 4: case 6: case 8: case 11: case 13: case 15:
		return [0x68,0x58,0x6e,0x5e].concat(middleOrange);
	    case 3: case 5: case 7: case 10: case 12: case 14: case 16:
		return [0x7a,0x7c,0x4a,0x4c].concat(middleOrange);
	    case 9:
		return [0x69,0x59,0x6f,0x5f].concat(middleRed);
	    }}
	else if (quarter == 1/2) {
	    switch (beat) {
	    case 8: // LEDs to turn on for 8 and
		return [0x78,0x7a,0x7b,0x7c,0x7d,0x7e,0x48,0x4a,0x4b,0x4c,0x4d,0x4e]
	    case 16:
		return [0x78,0x68,0x69,0x58,0x59,0x48,0x7e,0x6e,0x6f,0x5e,0x5f,0x4e]
	    default:
		return middleGreen
	    }
	}
    }
    return []; // turn on nothing if no matches
}

KANE_QuNeo.visualize = function (deck, beat, quarter) {
    if (deck == 2) { // do deck2 sequence
	var middleGreen = [0x24,0x14,0x12,0x22]
	var middleRed = [0x25,0x15,0x13,0x23]
	var middleOrange = middleGreen.concat(middleRed)
	if (quarter == 0) { // for whole beats
	    switch (beat) {
	    case 1: // LEDs to turn on for beat 1 of 8
		return [0x33,0x35,0x03,0x05].concat(middleRed);
	    case 2: case 4: case 6: case 8: case 11: case 13: case 15:
		return [0x20,0x10,0x26,0x16].concat(middleOrange);
	    case 3: case 5: case 7: case 10: case 12: case 14: case 16: case 0:
		return [0x32,0x34,0x02,0x04].concat(middleOrange);
	    case 9:
		return [0x21,0x11,0x27,0x17].concat(middleRed);
	    }
	}
	else if (quarter == 1/2) {
	    switch (beat) {
	    case 8:
		return [0x30,0x32,0x34,0x36,0x00,0x02,0x04,0x06,0x33,0x35,0x03,0x05]
	    case 16:
		return [0x30,0x20,0x10,0x00,0x36,0x26,0x16,0x06,0x21,0x11,0x27,0x17]
	    default:
		return middleGreen
	    }
	}
    }
    else if (deck == 1) { // do deck1 sequence
	var middleGreen = [0x2a,0x2c,0x1a,0x1c]
	var middleRed = [0x2b,0x2d,0x1b,0x1d]
	var middleOrange = middleGreen.concat(middleRed)
	if (quarter == 0) { // for whole beats
	    switch (beat) {
	    case 1: // LEDs to turn on for beat 1 of 8
		return [0x3b,0x3d,0x0b,0x0d].concat(middleRed);
	    case 2: case 4: case 6: case 8: case 11: case 13: case 15:
		return [0x28,0x18,0x2e,0x1e].concat(middleOrange);
	    case 3: case 5: case 7: case 10: case 12: case 14: case 16:
		return [0x3a,0x3c,0x0a,0x0c].concat(middleOrange);
	    case 9:
		return [0x29,0x19,0x2f,0x1f].concat(middleRed);
	    }
	}
	else if (quarter == 1/2) {
	    switch (beat) {
	    case 8: // LEDs to turn on for 8 and
		return [0x38,0x3a,0x3b,0x3c,0x3d,0x3e,0x08,0x0a,0x0b,0x0c,0x0d,0x0e]
	    case 16:
		return [0x38,0x28,0x29,0x18,0x19,0x08,0x3e,0x2e,0x2f,0x1e,0x1f,0x0e]
	    default:
		return middleGreen
	    }
	}
    }
    return []; // turn on nothing if no matches
}

/*
    if (beat > 4) beat -= 4; // crude mod 8
    // Here is deck 1's sequence 0
    if (deck == 1) { // do deck1 sequence
	if (quarter == 0) { // for whole beats
	    switch (beat) {
	    case 1: // LED sequence for beat 1 of 4
		return [0x1a,0x1b,0x2e,0x2f];
	    case 2: // for beat 2 of 4
		return [0x12,0x1a,0x1b];
	    case 3: // etc
		return [0x10,0x11,0x2e,0x2f];
	    case 4:
		return [0x18,0x10,0x11];
	    }
	}
    }
    // Here is deck 2's sequence 0
    else if (deck == 2) {
	if (quarter == 0) { // for whole beats
	    switch (beat) {
	    case 1:
		return [0x1c,0x1d,0x30,0x31];
	    case 2:
		return [0x14,0x1c,0x1d];
	    case 3:
		return [0x16,0x17,0x30,0x31];
	    case 4:
		return [0x1e,0x16,0x17];
	    }
	}
    }
    return []; // turn on nothing if no matches
} */

// Runtime Variables

KANE_QuNeo.makeVar = function (element) {
    var array = []
    // initialize the array with copies of the element for each deck
    for (var i = 0; i < KANE_QuNeo.numDecks; i++)
	array.push(element)
    return array
}

// stores timers of nudges
KANE_QuNeo.rateNudgeTimers = []
KANE_QuNeo.visualNudgeTimers = KANE_QuNeo.makeVar([])

KANE_QuNeo.canceledRateNudge = 0; // 1 if button is held and nudge was just canceled
KANE_QuNeo.visualNudge = KANE_QuNeo.makeVar(0) // 1 for nudging forward, -1 backward
KANE_QuNeo.rateNudge = 0 // 1 for rate nudging, determines
                          //  periodic vs press. -1 for backward, 0 for not nudging
KANE_QuNeo.lastLight =
    KANE_QuNeo.makeVar(-1); // starting position of last rotary LED
    
KANE_QuNeo.hotcuePressed =
    KANE_QuNeo.makeVar(0) // 1 if a hotcue is currently held down

// Stores which beat each deck is on, loops 1...KANE_QuNeo.totalBeats
KANE_QuNeo.wholeBeat = KANE_QuNeo.makeVar(KANE_QuNeo.totalBeats);
KANE_QuNeo.trackSamples = KANE_QuNeo.makeVar(-1); // total samples of tracks in decks
KANE_QuNeo.lastBeatPosition = 
    KANE_QuNeo.makeVar(-4000);  // position in samples of last real beat on each deck
KANE_QuNeo.beatLEDsOn = 1; // 1 if beat LEDs are on
KANE_QuNeo.activeBeatLEDs =
    KANE_QuNeo.makeVar([]); // beat LEDS currently receiving midi msgs
KANE_QuNeo.jumpLoopLEDs = 
    KANE_QuNeo.makeVar([]); // stores which jumploop LEDs to light
KANE_QuNeo.jumpDirectionLEDs = 
    KANE_QuNeo.makeVar([]); // stores which jumpDirection LEDs to light
KANE_QuNeo.loopingLED = 
    KANE_QuNeo.makeVar([]); // stores which looping LEDs to light
KANE_QuNeo.horizArrowLEDs = 
    KANE_QuNeo.makeVar([]); // stores which horiz arrow LEDs to light
KANE_QuNeo.jumpSyncLED = 
    KANE_QuNeo.makeVar([]); // stores which jumpsync LEDs to light
KANE_QuNeo.assertLED = []; // stores assert LED if it's on
    KANE_QuNeo.playScratchLED = []; // stores playscratch LED if it's on
KANE_QuNeo.reloopLEDs =
    KANE_QuNeo.makeVar([]); // stores which reloop LEDs are on
KANE_QuNeo.loadLEDs = []; // stores which tracks are ready for loading
KANE_QuNeo.hotcueActivateLEDs = 
    KANE_QuNeo.makeVar([]); // stores which hotcues are active
KANE_QuNeo.hotcueClearLEDs = 
    KANE_QuNeo.makeVar([]); // stores which hotcues can be cleared
KANE_QuNeo.LEDSequencing = 
    KANE_QuNeo.makeVar(0); // 1 if deck is in LED sequencer mode
KANE_QuNeo.beatCounterLEDs =
    KANE_QuNeo.makeVar([]); // stores which beat counter LEDs are currently active

KANE_QuNeo.playScratchToggle = 1;  // 1 for play, 0 for scratch
KANE_QuNeo.recordToggle = 0;  // 1 for recording
KANE_QuNeo.visualizer = 0; // 1 for visualizer mode
KANE_QuNeo.sliderMode = 0; // initialize to 0, cycles 0-3
KANE_QuNeo.nextHotcuePosition =
    KANE_QuNeo.makeVar(-1.0); // position from 0...1 of the next hotcue
KANE_QuNeo.numNextHotcues =
    KANE_QuNeo.makeVar(-1); // number of cues currently set on each deck

// 0 if no schedule, 1 if loop of 1 beat, 2 for a loop of 2 beats, etc
KANE_QuNeo.loopNextJump = KANE_QuNeo.makeVar(0);
KANE_QuNeo.trackJumped = 
    KANE_QuNeo.makeVar(0); // 1 if this track just jumped, 0 otherwise

KANE_QuNeo.trackJump = 
    KANE_QuNeo.makeVar(0); // -1 for backwards, 0 for no jump, 1 for forwards
KANE_QuNeo.trackLooping = 
    KANE_QuNeo.makeVar(1); // 1 if in looping mode, else 0
KANE_QuNeo.trackJumpSync = 
    KANE_QuNeo.makeVar(0); // 1 if auto sync on jump
KANE_QuNeo.trackPlaying = 
    KANE_QuNeo.makeVar(0); // 1 if track is currently playing
KANE_QuNeo.slipEnabled = KANE_QuNeo.makeVar(0) // 1 if slip is enabled

KANE_QuNeo.verticalSliderDoubleTap = [0,0,0,0]; // 1 when a the next tap will
                                                // activate the double tap

// the following hold arrays of timers which may be stopped
KANE_QuNeo.nextBeatTimer = KANE_QuNeo.makeVar([]);
KANE_QuNeo.scheduledBeats = KANE_QuNeo.makeVar([]);
KANE_QuNeo.jumpHoldTimers = KANE_QuNeo.makeVar([]) // hold timers for continued jumps
	    
/***** (IS) Initialization and Shutdown *****/

KANE_QuNeo.init = function (id) { // called when the device is opened & set up

  // NOTE: the 2 following controls are called each time the music updates,
  //       which means ~every 0.02 seconds. Everything that needs consistent updates
  //       should branch from these functions so we don't eat the cpu.
  //       Visual playposition is updated roughly 4x more often
  //       than playposition.
  engine.connectControl("[Channel1]","visual_playposition","KANE_QuNeo.time1Keeper");
  engine.connectControl("[Channel2]","visual_playposition","KANE_QuNeo.time2Keeper");

  // led controls for the master / flanger channels
  engine.connectControl("[Master]","VuMeter","KANE_QuNeo.masterVuMeter");
  //engine.softTakeover("[Master]","volume",true);
  engine.connectControl("[Master]","headVolume","KANE_QuNeo.headVol");		    
  engine.connectControl("[Flanger]","lfoPeriod","KANE_QuNeo.flangerPeriod");
  engine.connectControl("[Flanger]","lfoDepth","KANE_QuNeo.flangerDepth");  
  engine.connectControl("[Master]","crossfader","KANE_QuNeo.crossFader");

  for (var deck = 1; deck <= KANE_QuNeo.numDecks; deck++) {
      var channelName = KANE_QuNeo.getChannelName(deck)

      // led controls for deck VU meters
      engine.connectControl(channelName,"VuMeter","KANE_QuNeo.deck"+deck+"VuMeter");
      
      // soft takeovers
      //engine.softTakeover(channelName,"rate",true);
      //engine.softTakeover(channelName,"volume",true);
      //engine.softTakeover(channelName,"filterHigh",true);
      //engine.softTakeover(channelName,"filterMid",true);
      //engine.softTakeover(channelName,"filterLow",true);
      //engine.softTakeover(channelName,"pregain",true);

      // Conveniences
      engine.setValue(channelName,"keylock",1);
      engine.setValue(channelName,"quantize",1);   
      engine.setValue(channelName,"pregain",KANE_QuNeo.pregain)
  }

  //engine.setValue("[Samplers]","show_samplers",1)
  //engine.setValue("[Microphone]","show_microphone",1)


    // for coolness
  //engine.setValue("[Spinny1]","show_spinny",1)
  //engine.setValue("[Spinny2]","show_spinny",1)

  // now that our controls are up, assert our LED config
  KANE_QuNeo.assertLEDs(-1); // call with -1 as mode to indicate startup
};

KANE_QuNeo.shutdown = function () {
};

// -------------------------------------
// begin toggling section
// -------------------------------------

/***** (SC) Slider Cycle *****/

KANE_QuNeo.sliderCycle = function (channel, control, value, status, group) {
    KANE_QuNeo.closeSliderMode() // close old mode
    KANE_QuNeo.sliderMode = (KANE_QuNeo.sliderMode + 1) % 4; // cycle 0,1,2,3,0,...
    KANE_QuNeo.verticalSliderDoubleTap = [0,0,0,0] // reset vert slider double taps
    KANE_QuNeo.openSliderMode() // initiate new mode
}

/**** (JS) Toggling JumpSync *****/

KANE_QuNeo.toggleJumpSync = function (deck) {
    var channel = deck - 1;
    var old = KANE_QuNeo.trackJumpSync[channel]
    KANE_QuNeo.trackJumpSync[channel] = (old + 1) % 2 // toggle on/off
    KANE_QuNeo.assertJumpSyncLED(deck) // update LED
}

/***** (JD) Toggling Jump Direction *****/

KANE_QuNeo.setJump = function (deck, direction) {
    var channel = deck - 1 // track channels start at 0 to properly reference arrays
    var current = KANE_QuNeo.trackJump[channel];
    if (current != direction) { // if chosen jump is not already active,
	KANE_QuNeo.trackJump[channel] = direction; // set deck to the new direction
	KANE_QuNeo.trackLooping[channel] = 0;  // and turn looping off
    } else if (current == direction) { 
	KANE_QuNeo.trackJump[channel] = 0; // else toggle jump off
 	KANE_QuNeo.trackLooping[channel] = 1; // and looping on
    }
    // remember to update LEDs
    KANE_QuNeo.assertLoopingLED(deck);
    KANE_QuNeo.assertJumpDirectionLEDs(deck);
}

/***** (L) Toggling Looping *****/

KANE_QuNeo.toggleLooping = function (deck) {
    var channel = deck - 1;
    var old = KANE_QuNeo.trackLooping[channel]
    KANE_QuNeo.trackLooping[channel] = (old + 1) % 2 // toggle on/off
    KANE_QuNeo.assertLoopingLED(deck) // update LED
}

/***** (PS) Toggling PlayScratch ****/

KANE_QuNeo.play = function (deck) {
    var channel = deck - 1;
    var channelName = KANE_QuNeo.getChannelName(deck);
    var playing = engine.getValue(channelName,"play");

    if (playing) {    // If currently playing
        engine.setValue(channelName,"play",0);    // Stop
	KANE_QuNeo.cancelScheduledBeats(deck); // cancel any scheduled beats,
	KANE_QuNeo.clearLastBeatLEDs(deck); // and turn off last beat LEDs.
    }
    else {    // If not currently playing,
        engine.setValue(channelName,"play",1);    // Start

	// then sync if we are in jumpsync mode
	if (KANE_QuNeo.trackJumpSync[channel])
	    KANE_QuNeo.syncTrack(deck,"phase",0);
    }
    // update global value
    var hotcuePressed = KANE_QuNeo.hotcuePressed[channel];
    KANE_QuNeo.trackPlaying[channel] = (playing + 1 + hotcuePressed) % 2;

    // send LED messages to report play press
    if (KANE_QuNeo.mode != 16) // but not while in playlist mode
	KANE_QuNeo.playPressLEDs(deck)
}

KANE_QuNeo.togglePlayScratch = function (channel, control, value, status, group) {
    var old = KANE_QuNeo.playScratchToggle;
    KANE_QuNeo.playScratchToggle = (old + 1) % 2 // toggle global on/off
    KANE_QuNeo.assertPlayScratchLED() // update LED
}

KANE_QuNeo.rotaryTouch = function (deck, value, status) {
    if ((status & 0xF0) == 0x90) {    // If note press on midi channel 1
        if (value == 0x7F) {   // if full velocity
	    if (KANE_QuNeo.playScratchToggle) { // and scratch is toggled off
		KANE_QuNeo.play(deck); // this is a play button
		return;
	    }
	    // else proceed with scratching
	    var alpha = 1.0/8, beta = alpha/32;
            engine.scratchEnable(deck, 128, 33+1/3, alpha, beta);
       	}
       	else {engine.scratchDisable(deck);}
    }
    else if (value == 0x00) {    // If button up
        engine.scratchDisable(deck);
    }
}

// The wheels which actually control the scratching
KANE_QuNeo.wheelTurn = function (deck, value) {
    var velocity = KANE_QuNeo.scratchSpeed;
    if (value > 1) // if reverse
	velocity *= -1; // flip direction
    engine.scratchTick(deck,velocity);
}

/***** (R) Toggling Record *****/

KANE_QuNeo.toggleRecord = function (channel, control, value, status, group) {
    var old = KANE_QuNeo.recordToggle;
    KANE_QuNeo.recordToggle = (old + 1) % 2 // toggle global on/off
    engine.setValue("[Recording]","toggle_recording",1) // toggle engine
    
    // for each deck, print out the sample at which recording started
    for (var deck = 1; deck <= 2; deck++) {
	var channelName = KANE_QuNeo.getChannelName(deck);
	var samples = engine.getValue(channelName,"track_samples");
	var position = engine.getValue(channelName,"visual_playposition");
	var samplePosition = samples * position;
	print("Recording started with deck "+deck+ " at sample: "+samplePosition)
    }
    
    KANE_QuNeo.assertRecordLED() // update record LED
}

// -------------------------------------
// begin functionality section
// -------------------------------------

/***** (JL) Jump and/or Loop over 1,2,4 or 8 Beats *****/

KANE_QuNeo.jumpLoop = function (deck, numBeats) {
    var channel = deck - 1; // track channels start at 0 to properly reference arrays
    var channelName = KANE_QuNeo.getChannelName(deck)

    // first set a hold timer if none exist
    if (KANE_QuNeo.jumpHoldTimers[channel].length < 1)
	KANE_QuNeo.jumpHoldTimers[channel].push(
	    engine.beginTimer(KANE_QuNeo.jumpHoldTime,
			      "KANE_QuNeo.jumpHeld("+deck+","+numBeats+")",
			      true));

    // calculate samples per beat
    var samplerate = engine.getValue(channelName,"track_samplerate");
    var samples = engine.getValue(channelName,"track_samples");
    var bpm = engine.getValue(channelName,"file_bpm");
    var spb = samplerate * 60 * 2 / bpm // samples per beat, not sure on the 2.

    // calculate the new position
    var oldPosition = engine.getValue(channelName, "visual_playposition");
    var direction = KANE_QuNeo.trackJump[channel];
    var beatsVector = numBeats * direction; // vectors have magnitude and direction
    var newPosition = oldPosition + (beatsVector*spb/samples);

    // if jump is on,
    if (newPosition != oldPosition) {

	// light appropriate jumpLED during button press
	KANE_QuNeo.assertJumpLEDs(deck, numBeats);

	engine.setValue(channelName,"playposition",newPosition); // jump
	// then adjust current beat
	var wholeBeat = KANE_QuNeo.wholeBeat[channel]
	var newBeat = (wholeBeat + beatsVector);
	// if playing, we will hit the next beat so add 1
	if (engine.getValue(channelName,"play"))
	    newBeat += 1
	var totalBeats = KANE_QuNeo.totalBeats

	if (newBeat < 1) // hand-made mod because javascript mod is wrong when < 0.
	    newBeat += totalBeats 
	else if (newBeat > 16)
	    newBeat -= totalBeats

	KANE_QuNeo.wholeBeat[channel] = newBeat // set the new beat number
	KANE_QuNeo.trackJumped[channel] = 1; // say that we jumped
    }

    // now figure out how/whether or not to loop
    if (KANE_QuNeo.trackLooping[channel]) { // if in looping mode,

	if (!direction)                    // nor jumping,
	    KANE_QuNeo.doLoop(deck, numBeats); // do loop now
	else // else (if playing or jumping) schedule a loop
	    KANE_QuNeo.scheduleLoop(deck, numBeats);
    }

    // if neither jump nor loop, then we are in loop planning mode
    else if (!(KANE_QuNeo.trackLooping[channel])
	     && !(KANE_QuNeo.trackJump[channel])) {

	// light appropriate jumpLED during button press
	KANE_QuNeo.assertJumpLEDs(deck, numBeats);

	if (KANE_QuNeo.loopNextJump[channel] == numBeats) // if equal numBeats,
	    // we already have a loop of this length planned, so a second button
	    // press means to cancel the first
	    KANE_QuNeo.loopNextJump[channel] = 0;

	else // otherwise, proceed
	    KANE_QuNeo.loopNextJump[channel] = numBeats; // set loop for next jump
    }
}

KANE_QuNeo.scheduleSync = function (deck, syncType) {
    engine.beginTimer(
	KANE_QuNeo.jumpSyncTimer,
	"KANE_QuNeo.doSync("+deck+", \"" + syncType + "\")",true)
}

KANE_QuNeo.doSync = function (deck, syncType) {
    var deckType = KANE_QuNeo.getDeckType(deck);

    if (deckType == "deck" && deck <= 2) { // regular sync works only for decks,
	                                   // and we only have 2 decks atm
	var channelName = KANE_QuNeo.getChannelName(deck)
	// store start loop status
	var loopEnabled = engine.getValue(channelName,"loop_enabled");
	engine.setValue(channelName,"beatsync_"+syncType,1); // do the sync

	// if our operation somehow changed loop enabling, immediately change it back
	engine.beginTimer(KANE_QuNeo.checkLoopDelay,
			  "KANE_QuNeo.checkLoop("+deck+","+loopEnabled+")", true);
    }
}

KANE_QuNeo.syncTrack = function (deck, type, scheduleFlag) {
    var channel = deck - 1; // confusing, yes. channels start from 0.

    // verify other track is playing before syncing to avoid glitchy jumpsync loops
    var otherDeck = ((channel + 1) % 2) + 1;
    var otherTrackPlaying = engine.getValue("[Channel"+otherDeck+"]","play")
    if (otherTrackPlaying) {
	// flash jumpsync LED to signify the sync
	// KANE_QuNeo.syncLEDRed(deck);
	print("==============> SYNCING DECK "+deck+" WITH SYNC TYPE: "+type);
	if (scheduleFlag) // if this sync should be scheduled
	    KANE_QuNeo.scheduleSync(deck, type); // then schedule it
	else
	    KANE_QuNeo.doSync(deck,type); // else do it now
    }
}

KANE_QuNeo.scheduleLoop = function (deck, numBeats) {
    engine.beginTimer(KANE_QuNeo.jumpLoopTimer,
		      "KANE_QuNeo.doLoop("+deck+","+numBeats+")"
		      ,true)
}
    
KANE_QuNeo.doLoop = function (deck, numBeats) {
    var channelName = KANE_QuNeo.getChannelName(deck)
    engine.setValue(channelName,"beatloop_"+numBeats+"_activate",1) // set loop
    // emit LED updates, timer because engine is slow at registering stuff.
    KANE_QuNeo.delayedAssertion("KANE_QuNeo.assertBeatLEDs("+deck+")",true);
}

KANE_QuNeo.deckReloop = function (deck) {
    var channelName = KANE_QuNeo.getChannelName(deck)
    engine.setValue(channelName,"reloop_exit",1) // toggle reloop
    // emit LED updates, timer because engine is slow at registering reloop_exit.
    KANE_QuNeo.delayedAssertion("KANE_QuNeo.assertBeatLEDs("+deck+")",true);
}

KANE_QuNeo.deckMultiplyLoop = function (deck, factor) {
    var channelName = KANE_QuNeo.getChannelName(deck)
    // store start loop status
    var loopEnabled = engine.getValue(channelName,"loop_enabled");

    // determine which operation to perform
    var control;
    if (factor == 1/2) control = "loop_halve";
    else if (factor == 2) control = "loop_double";
    else print("ERROR: wrong factor given for deckMultiplyLoop");
    engine.setValue(channelName,control,1)
    
    // if our operation somehow changed loop enabling, immediately change it back
    engine.beginTimer(KANE_QuNeo.checkLoopDelay,
		      "KANE_QuNeo.checkLoop("+deck+","+loopEnabled+")", true);

    // emit LED updates, timer because engine is slow at registering loop stuffs
    KANE_QuNeo.delayedAssertion("KANE_QuNeo.assertBeatLEDs("+deck+")",true);
}

KANE_QuNeo.checkLoop = function (deck, loopEnabled) {
    var channelName = KANE_QuNeo.getChannelName(deck)

    // if the loop status is different than given, reloop
    if (engine.getValue(channelName,"loop_enabled") != loopEnabled) {
	print("retoggling loop")
	KANE_QuNeo.deckReloop(deck) // toggle the loop back to what it was
    }
}

// function that is reached only when one of the beat jump buttons is held
KANE_QuNeo.jumpHeld = function (deck, numBeats) {
    var channel = deck - 1;
    // ensure we are only in jump mode and not in looping mode
    if (KANE_QuNeo.trackJump[channel] != 0 &&
	KANE_QuNeo.trackLooping[channel] == 0) {
	print("=====> AUTO JUMPING ACTIVATED WITH NUMBEATS: "+numBeats)
	KANE_QuNeo.jumpHoldTimers[channel].push(
	    engine.beginTimer(KANE_QuNeo.autoJumpSpeed,
			      "KANE_QuNeo.jumpLoop("+deck+","+numBeats+")"));
    }
}

// function that is called when one of the beat jump buttons is released
KANE_QuNeo.jumpOff = function (deck, numBeats) {
    var channel = deck - 1;
    var channelName = KANE_QuNeo.getChannelName(deck);
    KANE_QuNeo.cancelTimers(KANE_QuNeo.jumpHoldTimers[channel]);
    KANE_QuNeo.jumpHoldTimers[channel] = [];
    KANE_QuNeo.assertBeatLEDs(deck);
}

/***** (VS) Vertical Sliders *****/

KANE_QuNeo.verticalSliderTouch = function (slider, value) {
    
    // if this is the first press:
    if (!KANE_QuNeo.verticalSliderDoubleTap[slider - 1]) {
	// record this press,
	KANE_QuNeo.verticalSliderDoubleTap[slider - 1] = 1;
	// then begin the reset timer
	engine.beginTimer(KANE_QuNeo.doubleTapWindow,
			  "KANE_QuNeo.resetDoubleTap("+slider+")",
			  true);
    }

    // if this is the second press:
    else {
	// reset the double tap
	KANE_QuNeo.resetDoubleTap(slider);
	// reset the pressed slider's level
	KANE_QuNeo.verticalSliderMove(slider, 0, 1);
    }
}

KANE_QuNeo.resetDoubleTap = function (slider) {
    KANE_QuNeo.verticalSliderDoubleTap[slider - 1] = 0;
}

KANE_QuNeo.verticalSliderMove = function (slider, value, resetFlag) {
    // print("resetflag on vertical slider move: "+resetFlag);
    // define values
    if (resetFlag) // if this is a call to reset the vertical slider,
	var values = KANE_QuNeo.getVerticalSliderResetValues(); // get reset values
    else // else get normal values
	var values = KANE_QuNeo.getReverseValues(value);
    
    // make sliders interact with mixxx based on mode and slider touched
    switch (KANE_QuNeo.sliderMode) {
    case 0: // mode 0
	switch (slider) {
	case 1:
	    KANE_QuNeo.deckZoom(1,value); break;
	case 2:
	    KANE_QuNeo.deckZoom(2,value); break;
	case 3:
	    KANE_QuNeo.deckCursor(1,value); break;
	case 4:
	    KANE_QuNeo.deckCursor(2,value); break;
	} break;
    case 1: // mode 1
	switch (slider) {
	case 1:
	    engine.setValue("[Channel1]","pregain",values.zeroToFour); break;
	case 2:
	    engine.setValue("[Channel2]","pregain",values.zeroToFour); break;
	case 3:
	    engine.setValue("[Channel1]","rate",values.negOneToOne); break;
	case 4:
	    engine.setValue("[Channel2]","rate",values.negOneToOne); break;
	} break;
    case 2: // mode 2
	switch (slider) {
	case 1:
	    engine.setValue("[Channel1]","volume",values.denormalized); break;
	case 2:
	    engine.setValue("[Channel2]","volume",values.denormalized); break;
	case 3:
	    engine.setValue("[Channel1]","filterHigh",values.zeroToFour); break;
	case 4:
	    engine.setValue("[Channel2]","filterHigh",values.zeroToFour); break;
	} break;
    case 3: // mode 3
	switch (slider) {
	case 1:
	    engine.setValue("[Channel1]","filterMid",values.zeroToFour); break;
	case 2:
	    engine.setValue("[Channel2]","filterMid",values.zeroToFour); break;
	case 3:
	    engine.setValue("[Channel1]","filterLow",values.zeroToFour); break;
	case 4:
	    engine.setValue("[Channel2]","filterLow",values.zeroToFour); break;
	} break;
    }
}

KANE_QuNeo.openSliderMode = function () {
    // choose which controls to connect based on mode
    switch (KANE_QuNeo.sliderMode) {
    case 0:
	// control connections
	engine.connectControl("[Channel1]","waveform_zoom",
			      "KANE_QuNeo.deck1ZoomLEDs")
	engine.connectControl("[Channel2]","waveform_zoom",
			      "KANE_QuNeo.deck2ZoomLEDs")
	engine.connectControl("[Channel1]","playposition",
			      "KANE_QuNeo.deck1CursorLEDs")
	engine.connectControl("[Channel2]","playposition",
			      "KANE_QuNeo.deck2CursorLEDs")
	// actual LED updates
	KANE_QuNeo.LEDs(0x90,[0x2c,0x2d],0x00) // rhombus off
	engine.trigger("[Channel1]","waveform_zoom")
	engine.trigger("[Channel2]","waveform_zoom")
	engine.trigger("[Channel1]","playposition")
	engine.trigger("[Channel2]","playposition")
	break;
    case 1:
	engine.connectControl("[Channel1]","pregain","KANE_QuNeo.deck1Pregain")
	engine.connectControl("[Channel2]","pregain","KANE_QuNeo.deck2Pregain")
	engine.connectControl("[Channel1]","rate","KANE_QuNeo.deck1Rate")
	engine.connectControl("[Channel2]","rate","KANE_QuNeo.deck2Rate")
	KANE_QuNeo.LEDs(0x90,[0x2c],0x7f) // rhombus green on
	engine.trigger("[Channel1]","pregain")
	engine.trigger("[Channel2]","pregain")
	engine.trigger("[Channel1]","rate")
	engine.trigger("[Channel2]","rate")
	break;
    case 2:
	engine.connectControl("[Channel1]","filterHigh","KANE_QuNeo.deck1Highs")
	engine.connectControl("[Channel2]","filterHigh","KANE_QuNeo.deck2Highs")
	KANE_QuNeo.LEDs(0x90,[0x2c],0x00) // rhombus green off
	KANE_QuNeo.LEDs(0x90,[0x2d],0x7f) // rhombus red on
	engine.trigger("[Channel1]","VuMeter")
	engine.trigger("[Channel2]","VuMeter")
	engine.trigger("[Channel1]","filterHigh")
	engine.trigger("[Channel2]","filterHigh")
	break;
    case 3:
	engine.connectControl("[Channel1]","filterMid","KANE_QuNeo.deck1Mids")
	engine.connectControl("[Channel2]","filterMid","KANE_QuNeo.deck2Mids")
	engine.connectControl("[Channel1]","filterLow","KANE_QuNeo.deck1Lows")
	engine.connectControl("[Channel2]","filterLow","KANE_QuNeo.deck2Lows")
	KANE_QuNeo.LEDs(0x90,[0x2c,0x2d],0x7f) // rhombus orange on
	engine.trigger("[Channel1]","filterMid")
	engine.trigger("[Channel2]","filterMid")
	engine.trigger("[Channel1]","filterLow")
	engine.trigger("[Channel2]","filterLow")
	break;
    default:
	print("ERROR: unknown slider mode "+KANE_QuNeo.sliderMode);
    }
}

KANE_QuNeo.closeSliderMode = function () {
    // choose which controls to disconnect based on which mode we are exiting
    switch (KANE_QuNeo.sliderMode) {
    case 0:
	engine.connectControl("[Channel1]","waveform_zoom",
			      "KANE_QuNeo.deck1ZoomLEDs",true)
	engine.connectControl("[Channel2]","waveform_zoom",
			      "KANE_QuNeo.deck2ZoomLEDs",true)
	engine.connectControl("[Channel1]","playposition",
			      "KANE_QuNeo.deck1CursorLEDs",true)
	engine.connectControl("[Channel2]","playposition",
			      "KANE_QuNeo.deck2CursorLEDs",true)
	break;
    case 1:
	engine.connectControl("[Channel1]","pregain","KANE_QuNeo.deck1Pregain",true)
	engine.connectControl("[Channel2]","pregain","KANE_QuNeo.deck2Pregain",true)
	engine.connectControl("[Channel1]","rate","KANE_QuNeo.deck1Rate",true)
	engine.connectControl("[Channel2]","rate","KANE_QuNeo.deck2Rate",true)
	break;
    case 2:
	engine.connectControl("[Channel1]","filterHigh","KANE_QuNeo.deck1Highs",true)
	engine.connectControl("[Channel2]","filterHigh","KANE_QuNeo.deck2Highs",true)
	break;
    case 3:
	engine.connectControl("[Channel1]","filterMid","KANE_QuNeo.deck1Mids",true)
	engine.connectControl("[Channel2]","filterMid","KANE_QuNeo.deck2Mids",true)
	engine.connectControl("[Channel1]","filterLow","KANE_QuNeo.deck1Lows",true)
	engine.connectControl("[Channel2]","filterLow","KANE_QuNeo.deck2Lows",true)
	break;
    default:
	print("ERROR: unknown slider mode "+KANE_QuNeo.sliderMode);
    }
}


/***** (ZC) Zoom and Cursor *****/

KANE_QuNeo.deckZoom = function (deck, value) {
    var channel = deck - 1; // track channels start at 0 to properly reference arrays
    var channelName = KANE_QuNeo.getChannelName(deck)
    var normalized = Math.ceil(6 * ((127 - value) / 127))
    // adjust zoom
    engine.setValue(channelName, "waveform_zoom", normalized)
}

KANE_QuNeo.deckCursor = function (deck, value) {
    var channel = deck - 1; // track channels start at 0 to properly reference arrays
    var channelName = KANE_QuNeo.getChannelName(deck)
    var normalized = 1 - (value / 127);
    // adjust play positions
    engine.setValue(channelName,"visual_playposition", normalized)
    engine.setValue(channelName,"playposition", normalized)
}

/***** (VN) Visual Nudging *****/

KANE_QuNeo.visualNudge = function (deck, direction) {
    var channel = deck - 1; // track channels start at 0 to properly reference arrays
    var channelName = KANE_QuNeo.getChannelName(deck)

    if (!(KANE_QuNeo.visualNudge[channel])) { // if we are not already nudging,
	KANE_QuNeo.visualNudge[channel] = 1; // set us to nudging,
	// then set timer to see if button is held for more than half a second
	KANE_QuNeo.visualNudgeTimers[channel].push(
	    engine.beginTimer(KANE_QuNeo.visualNudgeHoldTime,
			      "KANE_QuNeo.visualNudgeHeld("+deck+","+direction+")",
			      true))
	// then do the pressed nudge
	KANE_QuNeo.doVisualNudge(deck, direction);

    } else // if we are nudging, this press is to stop nudging
	KANE_QuNeo.visualNudge[channel] = 0;
}

KANE_QuNeo.visualNudgeHeld = function (deck, direction) {
    // if we reach this function, visual nudge has been held for sufficient time
    // to activate scrolling...
    var channel = deck - 1;
    KANE_QuNeo.visualNudge[channel] = direction;
    // ...so set a persistent scroll timer
    KANE_QuNeo.visualNudgeTimers[channel].push(
	engine.beginTimer(KANE_QuNeo.visualNudgeSpeed,
			  "KANE_QuNeo.doVisualNudge("+deck+","+direction+")"))
}

KANE_QuNeo.doVisualNudge = function (deck, direction) {
    var channel = deck - 1;
    var channelName = KANE_QuNeo.getChannelName(deck)
    // calculate new position
    var newPosition = engine.getValue(channelName,"visual_playposition")
	+ (KANE_QuNeo.visualNudgeDist * direction);
	  
    // now apply to both visual and actual position
    engine.setValue(channelName,"playposition",newPosition)
    engine.setValue(channelName,"visual_playposition",newPosition)
}

KANE_QuNeo.visualNudgeOff = function (deck, direction) {
    var channel = deck - 1; // track channels start at 0 to properly reference arrays

    // turn off the nudging
    KANE_QuNeo.visualNudge[channel] = 0;
    KANE_QuNeo.cancelTimers(KANE_QuNeo.visualNudgeTimers[channel]);
    KANE_QuNeo.visualNudgeTimers[channel] = []; // reset global rate timer var
}

/***** (RN) Periodic and Regular Rate Nudging *****/

KANE_QuNeo.rateNudgePress = function (deck, direction) {

    // if we are not auto nudging, then set button hold timer
    // and do a regular nudge
    if (!(KANE_QuNeo.rateNudge))
	KANE_QuNeo.rateNudgeTimers.push(
	    engine.beginTimer(KANE_QuNeo.rateNudgeHoldTime,
			      "KANE_QuNeo.rateNudgeHeld("+deck+","+direction+")",
			      true))
    else { // if we are currently auto nudging,
	KANE_QuNeo.cancelRateNudge(); // toggle it off
	KANE_QuNeo.canceledRateNudge = 1; // say we just canceled it
    }
}

KANE_QuNeo.rateNudgeHeld = function (deck, direction) {
    // if we reach this function, the rate nudge has been held for sufficient time
    // to activate auto nudge
    KANE_QuNeo.rateNudge = direction;
    print("=====> AUTO RATE NUDGE ACTIVATED")
    // then set a persistent nudge timer
    KANE_QuNeo.rateNudgeTimers.push(
	engine.beginTimer(KANE_QuNeo.rateNudgeSpeed,
			  "KANE_QuNeo.rateNudgeAll("+deck+","+direction+")"))
}

KANE_QuNeo.rateNudgeAll = function (callingDeck, direction) {
    var channelName = KANE_QuNeo.getChannelName(callingDeck);
    // do the calling deck's nudge 
    KANE_QuNeo.doRateNudge(callingDeck, direction)

    // then sync the tempo of the other decks to the calling deck.
    // The point of this is to ensure that each nudge amount is
    // consistent between all decks
    for (var deck = 1; deck <= KANE_QuNeo.numDecks; deck++) {
	if (deck != callingDeck)
	    KANE_QuNeo.doSync(deck, "tempo");
    }

    // now check if rate of calling deck is near 0%. if so, turn off auto nudging.
    // NOTE: extra rate factor is for looking ahead 1 tick because the engine is
    // slow to update.
    var rate = engine.getValue(channelName, "rate") + 0.16 * direction * 0.05;
    var tolerance = KANE_QuNeo.rateNudgeTolerance
    if ((rate < 0 && rate >= -tolerance) || // if near 0% adjustment,
	(rate > 0 && rate < tolerance))
	KANE_QuNeo.cancelRateNudge() // cancel rate nudging
}

KANE_QuNeo.doRateNudge = function (deck, direction) {
    KANE_QuNeo.assertNudgeLEDs() // update LEDs

    var control, channelName = KANE_QuNeo.getChannelName(deck);
    if (direction == 0)
	return;
    else if (direction == 1) // nudge tempo up
	control = "rate_perm_up_small"
    else if (direction == -1) // nudge tempo down
	control = "rate_perm_down_small"
    // make deck actually nudge
    engine.setValue(channelName,control,1)
}

KANE_QuNeo.rateNudgeRelease = function (deck, direction) {
    KANE_QuNeo.assertNudgeLEDs() // update LEDs
    if (!(KANE_QuNeo.rateNudge)) { // if we are not on auto nudge,
	KANE_QuNeo.cancelRateNudge() // cancel rate nudge timers

	if (!KANE_QuNeo.canceledRateNudge) { // if we did not just cancel rate nudge,
	    KANE_QuNeo.doRateNudge(deck, direction) // do a regular nudge
	} else // we just canceled a rate nudge, so reset global var
	    KANE_QuNeo.canceledRateNudge = 0;
    }
}

KANE_QuNeo.cancelRateNudge = function () {
    KANE_QuNeo.cancelTimers(KANE_QuNeo.rateNudgeTimers); // cancel timers
    KANE_QuNeo.rateNudgeTimers = []; // reset global rate timer var
    KANE_QuNeo.rateNudge = 0; // set to not nudging
//    KANE_QuNeo.canceledRateNudge = 1; // indicate that we canceled the nudge
    KANE_QuNeo.assertNudgeLEDs() // update LEDs
}

/***** (H) Hotcues *****/

KANE_QuNeo.makeCueDispatch = function (deck, cue, clearFlag, activateFlag) {
    var name = "deck"+deck+"Cue"+cue // base name

    // now decide which function to make
    if (clearFlag) { // make a cue clear function
	name += "Clear"
	KANE_QuNeo[name] = function(channel, control, value, status, group) {
	    var channelName = KANE_QuNeo.getChannelName(deck);
	    engine.setValue(channelName,"hotcue_"+cue+"_clear",1);
	    KANE_QuNeo.assertHotcueLEDs(deck);
	}
    } else { // we are making a variant of cue set
	// LEDs indexed first by cue then by deck to signify button press
	if (activateFlag) {// make a cue activate function
	    name += "Activate";
	} else { // make a cue deactivate function
	    name += "Deactivate";
	}
	KANE_QuNeo[name] = function(channel, control, value, status, group) {
	    var channel = deck - 1;
	    // first update global press status
	    KANE_QuNeo.hotcuePressed[channel] = activateFlag;
	    // then activate the pressed hotcue
	    var channelName = KANE_QuNeo.getChannelName(deck);
	    engine.setValue(channelName,"hotcue_"+cue+"_activate",activateFlag)
	    // emit LED updates
	    KANE_QuNeo.delayedAssertion("KANE_QuNeo.assertHotcueLEDs("+deck+")"
					,true);
	}
    }
}

KANE_QuNeo.makeCueDispatches = function (numDecks, numCues) {
    for (var deck = 1; deck <= numDecks; deck++) { // for each deck
	for (var cue = 1; cue <= numCues; cue++) { // and each cue
	    KANE_QuNeo.makeCueDispatch(deck, cue, 0, 1) // make a cue activate fxn
	    KANE_QuNeo.makeCueDispatch(deck, cue, 0, 0) // a cue deactivate fxn
	    KANE_QuNeo.makeCueDispatch(deck, cue, 1) // and a cue clear fxn
	}
    }
}
KANE_QuNeo.makeCueDispatches(KANE_QuNeo.numDecks - 4,KANE_QuNeo.numHotcues);

/***** (HA) Horizontal Arrows *****/

KANE_QuNeo.makeHorizontalArrowDispatch = function (deck, effect) {
    var name = "deck"+deck+effect // base name
    // make function
    KANE_QuNeo[name] = function(channel, control, value, status, group) {
	var channelName = KANE_QuNeo.getChannelName(deck);
	var old = engine.getValue(channelName,effect)
	var toggled = (old + 1) % 2;
	engine.setValue(channelName,effect,toggled)
	KANE_QuNeo.assertHorizArrowLEDs(deck); // update LEDs
    }
}

KANE_QuNeo.makeHorizontalArrowDispatches = function (numDecks) {
    var effects = ["keylock","pfl","slip_enabled","flanger"]
    for (var deck = 1; deck <= numDecks; deck++) { // for each deck
	for (var i = 0; i < effects.length; i++) // and each effect
	    // make dispatch
	    KANE_QuNeo.makeHorizontalArrowDispatch(deck, effects[i])
    }
}
KANE_QuNeo.makeHorizontalArrowDispatches(KANE_QuNeo.numDecks - 4);

/***** (P) Playlist Mode *****/

KANE_QuNeo.touchScroll = function (deck) {
    if (KANE_QuNeo.playScratchToggle) { // if in play mode
	KANE_QuNeo.play(deck); // this is a play button for the given deck
	KANE_QuNeo.assertLoadLEDs(); // update load availability LEDs
    }
}

KANE_QuNeo.scrollPlaylist = function (channel, control, value, status, group) {
    if (KANE_QuNeo.playScratchToggle) { // if in play mode
	return; // do nothing
    }
    // otherwise, handle scrolling
    if (value == 1) // if rotating clockwise
	engine.setValue("[Playlist]","SelectNextPlaylist",1)
    else if (value == 127) // if rotating counterclockwise
	engine.setValue("[Playlist]","SelectPrevPlaylist",1)
}

KANE_QuNeo.scrollTracklist = function (channel, control, value, status, group) {
    if (KANE_QuNeo.playScratchToggle) { // if in play mode
	return; // do nothing
    }
    // otherwise, handle scrolling
    if (value == 1) // if rotating clockwise
	engine.setValue("[Playlist]","SelectNextTrack",1)
    else if (value == 127) // if rotating counterclockwise
	engine.setValue("[Playlist]","SelectPrevTrack",1)
}

/***** (RB) Reset Beat *****/

KANE_QuNeo.resetBeat = function (deck) {
    var channel = deck - 1;
    var channelName = KANE_QuNeo.getChannelName(deck)
    var bpm = engine.getValue(channelName,"bpm");
    var spb = 60/bpm // seconds per beat

    KANE_QuNeo.cancelScheduledBeats(deck) // first, cancel old beats
    KANE_QuNeo.wholeBeat[channel] = KANE_QuNeo.totalBeats; // set beat to 1,
    KANE_QuNeo.scheduleBeat(deck, 1, spb, 1) // then schedule this beat
}

/***** (QC) Quantize Cues *****/

KANE_QuNeo.quantizeCues = function (deck, LEDControl) {
    var channelName = KANE_QuNeo.getChannelName(deck);

    // light LED to indicate button press
    KANE_QuNeo.LEDs(0x90,LEDControl,0x7f)
    
    // find out our current position, assume it's on the beatgrid
    var trackSamples = engine.getValue(channelName,"track_samples")
    var position = engine.getValue(channelName,"visual_playposition")
	* trackSamples; // track position in samples

    // determine bpm and spb
    var samplerate = engine.getValue(channelName,"track_samplerate")
    var bpm = engine.getValue(channelName,"file_bpm");
    var spb = samplerate * 60 * 2 / bpm // samples per beat, not sure on the 2.

    // iterate through each active cue, clearing it then resetting it such that
    // it will end up on the beat nearest its original position
    for (var cue = 1; cue <= KANE_QuNeo.numHotcues; cue++) {
	var cuePosition = engine.getValue(channelName,"hotcue_"+cue+"_position");

	// if cue is enabled, reset it
	if (cuePosition >= 0) {
	    var diffBeats = Math.round((cuePosition - position) / spb)
	    var newPosition = position + (diffBeats * spb)
	    engine.setValue(channelName,"hotcue_"+cue+"_position",newPosition)
	}
    }
}

KANE_QuNeo.quantizeCuesOff = function (deck, control) {
    KANE_QuNeo.LEDs(0x90,control,0x00)
}

/***** (TK) Time Keeping *****/

KANE_QuNeo.timeKeeper = function (deck, value) {
    var channel = deck - 1;
    var channelName = KANE_QuNeo.getChannelName(deck)

    // if we're actually playing (not just a hotcue press), then do these things
    if (KANE_QuNeo.trackPlaying[channel] || !KANE_QuNeo.hotcuePressed[channel]) {
	// check for a next nearest hotcue, and update if we've passed it
	if ((value - KANE_QuNeo.nextHotcuePosition[channel]) >= 0 &&
	    KANE_QuNeo.numNextHotcues[channel])
	    KANE_QuNeo.assertHotcueLEDs(deck)

	// update rotary LEDs
	KANE_QuNeo.circleLEDs(deck, value)
    }
	
    // report when there are beats and we haven't reached our last beat report
    if (engine.getValue(channelName,"beat_active") &&
	KANE_QuNeo.nextBeatTimer[channel].length == 0)
	KANE_QuNeo.nextBeatTimer[channel] =
	    [engine.beginTimer(KANE_QuNeo.beatOffset,
			      "KANE_QuNeo.handleBeat("+deck+")",
			      true)];

    // now determine whether or not the track has changed
    var channelName = KANE_QuNeo.getChannelName(deck);
    var trackSamples = engine.getValue(channelName, "track_samples");
    // if the old and new values are not the same, the track must have changed
    if (trackSamples != KANE_QuNeo.trackSamples[channel]) {
	KANE_QuNeo.trackSamples[channel] = trackSamples; // so update to new value
	KANE_QuNeo.delayedAssertion("KANE_QuNeo.assertHotcueLEDs("+deck+")"
				    ,true, 200);
    }

    // if we're at the end of the song, set track to not playing
    if (value == 1) {
	KANE_QuNeo.trackPlaying[channel] = 0
	if (KANE_QuNeo.mode == 16)
	    KANE_QuNeo.assertLoadLEDs(deck);
	KANE_QuNeo.triggerVuMeter(0); // trigger master VuMeter
    }
}

/***** (B) Beat Handling *****/

KANE_QuNeo.getWholeBeat = function (deck, position, spb) {
    var channel = deck - 1; // confusing, yes. channels start from 0.
    var channelName = KANE_QuNeo.getChannelName(deck);
    var referencePosition = 
	engine.getValue(channelName,"hotcue_9_position");
    print("diffBeat: "+(position-referencePosition))
    var diffBeat = Math.round((position - referencePosition)/spb)
    if (diffBeat > 0)
	return (diffBeat % 16) + 1;
    else
	return (((diffBeat % 16)+ 16) % 16) + 1;
}

KANE_QuNeo.handleBeat = function (deck) {
    var channel = deck - 1; // confusing, yes. channels start from 0.
    var channelName = KANE_QuNeo.getChannelName(deck);

    // signify that this beat was reached outside the beat_active window
    engine.beginTimer(KANE_QuNeo.beatOffset,
		      "KANE_QuNeo.resetNextBeatTimer("+deck+")",
		      true);
    
    // record last beat number
    var lastWholeBeat = KANE_QuNeo.wholeBeat[channel];

    // now see which beat this one is
    var value = engine.getValue(channelName,"visual_playposition")
    var samplerate = engine.getValue(channelName,"track_samplerate");
    var samples = engine.getValue(channelName,"track_samples");
    var bpm = engine.getValue(channelName,"file_bpm");
    var spb = samplerate * 60 * 2 / bpm // samples per beat, not sure on the 2.
    var position = value * samples; // scale 0...1 to position in samples
    var diff = position - KANE_QuNeo.lastBeatPosition[channel];

    //print("percent diff from last beat: "+diff / spb)

    // if this is a consecutive beat, do regular stuff:
    if (diff >= .8*spb && diff <= 1.2*spb) {
    	// first increment beat number,
	if (lastWholeBeat == KANE_QuNeo.totalBeats) // if beat at end
	    KANE_QuNeo.wholeBeat[channel] = 1; // restart at 1
	else // regular increment
	    KANE_QuNeo.wholeBeat[channel] += 1;
    }

    // if we have moved either more than a beat OR backwards,
    // this is not a consecutive beat:
    else if (diff >= 1.1*spb || (diff <= 0)) { // diff == 0 means repeat beat
	print("non consecutive beat on deck "+deck)
	print("diff: "+diff)
	KANE_QuNeo.cancelScheduledBeats(deck); // first cancel any scheduled beats
	
	if (!(KANE_QuNeo.trackJumped[channel])) { // if there was not a jump press,
	    KANE_QuNeo.wholeBeat[channel] = 1; // restart at beat 1
	} else // we just jumped, so reset status and don't touch beat counters
	    KANE_QuNeo.trackJumped[channel] = 0;

	// schedule a sync if we are in JumpSync mode
	if (KANE_QuNeo.trackJumpSync[channel]) {
	    KANE_QuNeo.syncTrack(deck,"phase",1);
	}

	// update hotcue LEDs
	KANE_QuNeo.assertHotcueLEDs(deck);

	// set a loop if we had a loop planned for next jump
	var numBeats = KANE_QuNeo.loopNextJump[channel];
	if (numBeats) {
	    KANE_QuNeo.doLoop(deck, numBeats);
	    KANE_QuNeo.loopNextJump[channel] = 0; // handled, so reset global
	}
    }
    // remember to clear our list of previously scheduled beats
    KANE_QuNeo.scheduledBeats[channel] = [];
    // schedule the next beat
    KANE_QuNeo.scheduleBeat(deck, KANE_QuNeo.wholeBeat[channel], 1);
    // update beat counter LEDs no matter what happened
    KANE_QuNeo.assertBeatCounterLEDs(deck);
    KANE_QuNeo.assertBeatLEDs(deck);
    // set the last beat to this position
    KANE_QuNeo.lastBeatPosition[channel] = position;
}

KANE_QuNeo.cancelScheduledBeats = function (deck) {
    var channel = deck - 1, i;
    var scheduledBeats = KANE_QuNeo.scheduledBeats[channel];
    KANE_QuNeo.cancelTimers(scheduledBeats) // cancel all
    // now clear the global var
    KANE_QuNeo.scheduledBeats[channel] = [];
}

KANE_QuNeo.resetNextBeatTimer = function (deck) {
    var channel = deck - 1;
    KANE_QuNeo.nextBeatTimer[channel] = [];
}

KANE_QuNeo.cancelTimers = function (timers) {
    for (i = 0; i < timers.length; i++)
	engine.stopTimer(timers[i]); // cancel each timer
}

// direction is 1 or -1 for forwards or backwards respectively
KANE_QuNeo.scheduleBeat = function (deck, wholeBeat, direction) {
    var channel = deck - 1;
    var channelName = KANE_QuNeo.getChannelName(deck);
    // determine seconds per beat
    var bpm = engine.getValue(channelName,"file_bpm")
    var spb = 60 / bpm

    // drummer speak to determine seconds to offset for each quarter note
    var e, and, uh;
    if (direction == 1) {
	e = 1000*(spb * 1/4); // forwards, so e first
        uh = 1000*(spb * 3/4); // multiple of 1000 for s -> ms
    } else if (direction == -1) {
        uh = 1000*(spb * 1/4); // reverse, so uh first
	e = 1000*(spb * 3/4);
    }
    else {
	print("ERROR. direction: "+direction+
	      " for beat on deck: "+deck+ " is not valid");
	return;
    }
    and = 1000*(spb * 1/2); // and is symmetric in either direction

    // now set and store actual timers, in case we want to cancel them
    var startOfCall = "KANE_QuNeo.deckBeatLEDs("+deck+","+wholeBeat+",";

    // the beat itself, do it now
    KANE_QuNeo.deckBeatLEDs(deck,wholeBeat,0);
    // e, add this and the following quarters to our list of timers
    KANE_QuNeo.scheduledBeats[channel].push(
	engine.beginTimer(e,startOfCall+"1/4)", true))
    // and
    KANE_QuNeo.scheduledBeats[channel].push(
	engine.beginTimer(and,startOfCall+"1/2)", true))
    // uh
    KANE_QuNeo.scheduledBeats[channel].push(
	engine.beginTimer(uh,startOfCall+"3/4)", true))
}

/***** (U) Utilities *****/

KANE_QuNeo.oneToFiveKnob = function (value) {
    if (value < 1) // first half of the knob, 0-1
	return value * 127 / 2;
    else // second half of the knob, 1-5
	return 63.5 + (value - 1) * 63.5 / 4
}

KANE_QuNeo.getSliderControl = function (deck, side) {
    var LEDGroup = KANE_QuNeo.getLEDGroup(deck);

    // left side
    if (side == 0) {
	if (LEDGroup == 1) return [0x01]; // leftmost slider
	else if (LEDGroup == 2) return [0x02]; // middle left slider

	// right side
    } else if (side == 1) {
	if (LEDGroup == 1) return [0x03]; // middle right slider
	else if (LEDGroup == 2) return [0x04]; // rightmost slider

    } else print("ERROR: getSliderControl called with improper args.")
}

KANE_QuNeo.delayedAssertion = function (functionName, oneShotFlag, delay) {
    // if no delay is given, assume default
    if (delay == undefined)
	delay = KANE_QuNeo.assertionDelayTimer

    engine.beginTimer(delay,functionName,oneShotFlag)
}

KANE_QuNeo.powerNeverOff = function (value,power) {
    value = Math.pow(value, power); // take power before checking min brightness
    if (value < KANE_QuNeo.minBrightness)
	value = KANE_QuNeo.minBrightness;
    return value;
}

KANE_QuNeo.scaleToSlider = function (value) {
    return value * 127
}

KANE_QuNeo.getForwardValues = function (value) {
    return {squared: KANE_QuNeo.scaleToSlider(Math.pow(value, 2)),
	    cubed: KANE_QuNeo.scaleToSlider(Math.pow(value, 3)),
	    fourth:  KANE_QuNeo.scaleToSlider(Math.pow(value, 4)),
	    squaredNeverOff: KANE_QuNeo.scaleToSlider(KANE_QuNeo.powerNeverOff(
		value,2)),
	    cubedNeverOff: KANE_QuNeo.scaleToSlider(KANE_QuNeo.powerNeverOff(
		value, 3)),
	    fourthNeverOff: KANE_QuNeo.scaleToSlider(KANE_QuNeo.powerNeverOff(
		value, 4))
	   }
}

KANE_QuNeo.getReverseValues = function (value) {

    // determine 0...4 value
    if (value < 63.5) // first half of the knob, 0...1
	var zeroToFour1 = value / 63.5;
    else // second half of the knob, 1...4
	var zeroToFour1 = ( (3 / 63.5) * value ) - 2;
    // determine 0...1 denormalized from 0...127 value
    var denormalized1 = value / 127;
    // determine -1...1
    var negOneToOne1 = value / 63.5 - 1;
    return {zeroToFour: zeroToFour1,
	    denormalized: denormalized1,
	    negOneToOne: negOneToOne1
	   }
}

KANE_QuNeo.getVerticalSliderResetValues = function () {
    return {zeroToFour: 1,
	    denormalized: 1,
	    negOneToOne: 0
	   }
}

KANE_QuNeo.getLEDGroup = function (deck) {
    // determines the correspondence between LEDs and decks
    switch (deck) {
    case 1: case 3: case 7: // deck 1,3 or sampler 3
	return 1;
    case 2: case 4: case 8: // deck 2,4 or sampler 4
	return 2;
    case 5: // sampler 1
	return 3;
    case 6: // sampler 2
	return 4;
    }
}

KANE_QuNeo.getChannelName = function (deck) {
    var deckType = KANE_QuNeo.getDeckType(deck);
    if (deckType == "master") // master deck
	return "[Master]"
    else if (deckType == "deck") // if dealing with actual decks
	return "[Channel"+deck+"]"
    else if (deckType == "sampler") // if dealing with samplers
	return "[Sampler"+(deck - 4)+"]"
}

KANE_QuNeo.getDeckType = function (deck) {
    if (deck == 0) // master deck
	return "master"
    else if (deck < 5) // if dealing with actual decks
	return "deck"
    else // if dealing with samplers
	return "sampler"
}

// -------------------------------------------------------
// Begin LED section
// -------------------------------------------------------
// General io function for LEDs, takes array of controls and a single hex value
KANE_QuNeo.LEDs = function (midiChannel, controls, value) {

    if (!(controls instanceof Array)) // if controls are not in an array,
	controls = [controls] // turn them into one

    // else send each control each value
    for (var i=0; i < controls.length; i++) {
	var control = controls[i];
	if (control != undefined) {
	    midi.sendShortMsg(midiChannel,control,value);
	}
    }
}

/***** (BLED) Beat LEDs *****/

KANE_QuNeo.deckBeatLEDs = function (deck, wholeBeat, quarter) {
    //print("beat: "+wholeBeat+" with quarter: "+quarter)
    var channel = deck - 1; // yes very annoying
    var channelName = KANE_QuNeo.getChannelName(deck);

    // Check for beat LED updates
    var on = [];
    if (KANE_QuNeo.LEDSequencing[channel]) { // if given deck is sequencing
	on = KANE_QuNeo.playerSequence(deck,wholeBeat,quarter);
	if (KANE_QuNeo.mode == 5) // add visualizations if in visualizer mode
	    on = on.concat(KANE_QuNeo.visualize(deck,wholeBeat,quarter))
    }
    
    if (on.length > 0) { // if we have new LEDs to turn on,
	KANE_QuNeo.clearLastBeatLEDs(deck); // first turn off old,
	KANE_QuNeo.activeBeatLEDs[channel] = on; // then turn on the new.
    }
}

KANE_QuNeo.clearLastBeatLEDs = function (deck) {
    var channel = deck - 1;
    var off = KANE_QuNeo.activeBeatLEDs[channel]; // store for use
    KANE_QuNeo.activeBeatLEDs[channel] = []; // empty the global var
    KANE_QuNeo.LEDs(0x91,off,0x00); // clear LED values
}

/***** (JLED) Jump LEDs *****/

KANE_QuNeo.syncLEDRed = function (deck) {
    var LEDGroup = KANE_QuNeo.getLEDGroup(deck);
    var on, off;
    if (LEDGroup == 1) on = [0x03], off = [0x02];
    else on = [0x0d], off = [0x0c];
    // emit updates
    KANE_QuNeo.LEDs(0x91,on,0x7f)
    KANE_QuNeo.LEDs(0x91,off,0x00)
}

KANE_QuNeo.assertJumpLEDs = function (deck, numBeats) {
    var channel = deck - 1;
    var off = [], on = []; // arrays of LEDs to turn off or on;
    var LEDGroup = KANE_QuNeo.getLEDGroup(deck);
    // first determine which LEDs we care about
    var LEDs;
    switch (LEDGroup) {
    case 1:
	LEDs = [[0x30,0x31],[0x32,0x33],[0x20,0x21],[0x22,0x23]]
	break;
    case 2:
	LEDs = [[0x3c,0x3d],[0x3e,0x3f],[0x2c,0x2d],[0x2e,0x2f]]
	break;
    case 3:
	LEDs = [[0x70,0x71],[0x72,0x73],[0x60,0x61],[0x62,0x63]]
	break;
    case 4:
	LEDs = [[0x7c,0x7d],[0x7e,0x7f],[0x6c,0x6d],[0x6e,0x6f]]
	break;
    }
    // now determine which beat we care about
    switch (numBeats) {
    case 1:
	on = [LEDs[0][1]]; off = [LEDs[0][0]]; // green off, red on
	break;
    case 2:
	on = [LEDs[1][1]]; off = [LEDs[1][0]]; // green off, red on
	break;
    case 4:
	on = [LEDs[2][1]]; off = [LEDs[2][0]]; // green off, red on
	break;
    case 8:
	on = [LEDs[3][1]]; off = [LEDs[3][0]]; // green off, red on
	break;
    }
    // emit updates
    var old = KANE_QuNeo.jumpLoopLEDs[channel];
    old.splice(old.indexOf(off[0]),1) // turn off the off stuff
    KANE_QuNeo.jumpLoopLEDs[channel] = old.concat(on); // turn on the new stuff
    KANE_QuNeo.LEDs(0x91,off,0x00); // update offs
    KANE_QuNeo.triggerVuMeter(deck); // update ons
}

/****** (RLED) Rotary LEDs ******/

KANE_QuNeo.playPressLEDs = function (deck) {
    var status = 0xB0, control, LEDGroup = KANE_QuNeo.getLEDGroup(deck);
    if (LEDGroup == 1) control = 0x06; // CC 6 for rotary 1
    else if (LEDGroup == 2) control = 0x07; // CC 7 for rotary 2
    // light all lights to report button press
    for (var i = 0; i < 128; i++) {
	midi.sendShortMsg(status,control,i);
    }
}

KANE_QuNeo.circleLEDs = function (deck, value) {
    var channelName = KANE_QuNeo.getChannelName(deck);
    var channel = deck - 1; // confusing, yes. channels start from 0.

    // time it takes for an imaginary record to go around once in seconds
    var revtime = 1.8

    // find how many revolutions we have made. The fractional part is where we are
    // on the vinyl.
    var time = value * engine.getValue(channelName,"duration");
    var revolutions = (time/revtime) - .25;

    // multiply the fractional part by the total number of LEDs in a rotary.
    var light = ((revolutions-(revolutions|0))*127)|0; 
    // if this is a repeat message, do not send.
    if (KANE_QuNeo.lastLight[channel]==light) return;

    // format the message on midiChannel 1.
    // fwd means the forward spinning lights,
    // bwd the backward
    var status = 0xB0, control, fwd = 0x00, bwd = 0x7f;
    if (deck == 1) control = 0x06; // CC 6 for rotary 1
    else if (deck == 2) control = 0x07; // CC 7 for rotary 2
    midi.sendShortMsg(status,control,fwd+light);

    // maintenance
    KANE_QuNeo.lastLight[channel]=light;
}

/***** (ALED) LED Assertions *****/

KANE_QuNeo.assertLEDs = function (mode) {
    KANE_QuNeo.closeMode(KANE_QuNeo.mode) // first, close old mode
    KANE_QuNeo.mode = mode// immediately following, update global mode

    // turn off this button's LED while asserting
    KANE_QuNeo.assertLED = [];
    KANE_QuNeo.LEDs(0x90,[0x22],0x00)

    // now assert universal LEDs
    KANE_QuNeo.assertPlayScratchLED();
    KANE_QuNeo.assertRecordLED();
    KANE_QuNeo.assertNudgeLEDs();
    KANE_QuNeo.assertHorizArrowLEDs(1);
    KANE_QuNeo.assertHorizArrowLEDs(2);
    engine.trigger("[Master]","headVolume");
    engine.trigger("[Flanger]","lfoPeriod");
    engine.trigger("[Flanger]","lfoDepth");
    engine.trigger("[Master]","crossfader");
    
    // trigger all VuMeters
    for (var deck = 0; deck <= KANE_QuNeo.numDecks; deck++)
	KANE_QuNeo.triggerVuMeter(deck)

    // make sure we have no latent beat LEDs
    KANE_QuNeo.clearLastBeatLEDs(1);
    KANE_QuNeo.clearLastBeatLEDs(2);

    // now decide which other LEDs to assert
    print("====> Switching to mode: "+mode)
    switch (mode) {
    case 5: // visualizer mode
	KANE_QuNeo.assertMode5();
	break;
    case 13: // main dj mode
	KANE_QuNeo.assertMode13();
	break;
    case 14: // cue left mode
	KANE_QuNeo.assertMode14();
	break;
    case 15: // cue right mode
	KANE_QuNeo.assertMode15();
	break;
    case 16: // playlist editing mode
	KANE_QuNeo.assertMode16();
	break;
    case -1: // starting up
	KANE_QuNeo.LEDs(0x90,[0x22],0x7f) // light this button's LED
	KANE_QuNeo.mode = 13; // default mode is dj mode
	KANE_QuNeo.assertMode13();
	break;
    }
}

KANE_QuNeo.assertMode5 = function () {
    KANE_QuNeo.LEDSequencing[0] = 1; // turn on sequencer
    KANE_QuNeo.LEDSequencing[1] = 1; // turn on sequencer
    engine.connectControl("[Channel1]","VuMeterL","KANE_QuNeo.deck1LeftVol");
    engine.connectControl("[Channel1]","VuMeterR","KANE_QuNeo.deck1RightVol");
    engine.connectControl("[Channel2]","VuMeterL","KANE_QuNeo.deck2LeftVol");
    engine.connectControl("[Channel2]","VuMeterR","KANE_QuNeo.deck2RightVol");
}

KANE_QuNeo.assertMode13 = function () {
    var deck;
    // vertical sliders
    KANE_QuNeo.openSliderMode();
    for (deck = 1; deck <= 2; deck++) { // 1 and 2 for sides left and right
	KANE_QuNeo.assertJumpSyncLED(deck);
	KANE_QuNeo.assertLoopingLED(deck);
	KANE_QuNeo.assertJumpDirectionLEDs(deck);
	KANE_QuNeo.assertBeatLEDs(deck);
	KANE_QuNeo.LEDSequencing[deck - 1] = 0; // turn off sequencers
	KANE_QuNeo.assertHotcueLEDs(deck);
	// filter kills
	KANE_QuNeo.assertKillLEDs(deck);
	// beat counter LEDs
	KANE_QuNeo.assertBeatCounterLEDs(deck);
    }
}

KANE_QuNeo.assertMode14 = function () {
    KANE_QuNeo.assertMode13(); // same as main dj but with no fx or slip leds
}

KANE_QuNeo.assertMode15 = function () {
    KANE_QuNeo.assertMode13(); // same as main dj but with no fx or slip leds
}

KANE_QuNeo.assertMode16 = function () {
    KANE_QuNeo.LEDSequencing[0] = 1; // turn on sequencer
    KANE_QuNeo.LEDSequencing[1] = 1; // turn on sequencer
    KANE_QuNeo.playScratchToggle = 0; // default: scrolling is on
    KANE_QuNeo.assertPlayScratchLED();
    // light squares
    KANE_QuNeo.assertLoadLEDs();
}

KANE_QuNeo.closeMode = function (mode) {
    switch (mode) {
    case 5:
	engine.connectControl("[Channel1]","VuMeterL",
			      "KANE_QuNeo.deck1LeftVol",true);
	engine.connectControl("[Channel1]","VuMeterR",
			      "KANE_QuNeo.deck1RightVol",true);
	engine.connectControl("[Channel2]","VuMeterL",
			      "KANE_QuNeo.deck2LeftVol",true);
	engine.connectControl("[Channel2]","VuMeterR",
			      "KANE_QuNeo.deck2RightVol",true);
	break;
    case 13: case 14: case 15:
	// stop the flashing LEDs
	KANE_QuNeo.closeSliderMode()
	for (var channel = 0; channel < 2; channel++) {
	    var deck = channel + 1;
	    KANE_QuNeo.hotcueActivateLEDs[channel] = [];
	    KANE_QuNeo.hotcueClearLEDs[channel] = [];
	    KANE_QuNeo.clearHotcueLEDs(deck);
	    KANE_QuNeo.jumpLoopLEDs[channel] = [];
	    KANE_QuNeo.loopingLED[channel] = [];
	    KANE_QuNeo.horizArrowLEDs[channel] = [];
	    KANE_QuNeo.jumpSyncLED[channel] = [];
	    KANE_QuNeo.reloopLEDs[channel] = [];
	    KANE_QuNeo.jumpDirectionLEDs[channel] = [];
	    KANE_QuNeo.beatCounterLEDs[channel] = [];
	    KANE_QuNeo.triggerVuMeter(channel + 1); // trigger the corresponding deck
	} break;
    case 16:
	KANE_QuNeo.playScratchToggle = 1; // go back to having scratch off
	KANE_QuNeo.loadLEDs = []; // reset Load LEDs
	var LEDs = [[0x08,0x09],[0x0e,0x0f],[undefined,undefined],[undefined,undefined],[0x00,0x01],[0x02,0x03],[0x04,0x05],[0x06,0x07]];
	for (var i = 0; i < LEDs.length; i++)
	    KANE_QuNeo.LEDs(0x90,LEDs[i],0x00); // turn off all old LEDs
	break;
    }
}

KANE_QuNeo.assertNudgeLEDs = function () {
    // turn on if auto nudging, else turn off
    var on = [];
    var off = [];
    if (KANE_QuNeo.rateNudge == 1) // forward nudge LEDs on
	on.push(0x2e,0x30), off.push(0x2f,0x31);
    else if (KANE_QuNeo.rateNudge == -1) // backward nudge LEDs on
	on.push(0x2f,0x31), off.push(0x2e,0x30);
    else if (KANE_QuNeo.rateNudge == 0) // no nudge LEDs on
	off.push(0x2e,0x2f,0x30,0x31)

    // emit updates
    KANE_QuNeo.LEDs(0x90,on,0x7f);
    KANE_QuNeo.LEDs(0x90,off,0x00);
}

KANE_QuNeo.assertHotcueLEDs = function (deck) {
    var channel = deck - 1;
    var LEDGroup = KANE_QuNeo.getLEDGroup(deck);
    var mode = KANE_QuNeo.mode;

    // first clear old garbage LEDs
    KANE_QuNeo.LEDs(0x91,KANE_QuNeo.hotcueActivateLEDs[channel],0x00);
    KANE_QuNeo.hotcueActivateLEDs[channel] = [];
    KANE_QuNeo.LEDs(0x91,KANE_QuNeo.hotcueClearLEDs[channel],0x00);
    KANE_QuNeo.hotcueClearLEDs[channel] = [];

    if (mode == 13) {// if in main dj mode 
	KANE_QuNeo.assertHotcueActivateLEDs(deck); // light activate hotcues
    }
    else if (mode == 14 && LEDGroup == 1) { // if in cue left mode, and group is 1
	KANE_QuNeo.assertHotcueActivateLEDs(deck); // assert this deck's activates
	KANE_QuNeo.assertHotcueClearLEDs(deck); // and this deck's clears

    } else if (mode == 15 && LEDGroup == 2) { // if in cue right mode w group of 2
	KANE_QuNeo.assertHotcueActivateLEDs(deck); // assert this deck's activates
	KANE_QuNeo.assertHotcueClearLEDs(deck); // and this deck's clears
    } else // assume hotcue LEDs must not be on
	return;
}

KANE_QuNeo.assertHotcueActivateLEDs = function (deck) {
    var channel = deck - 1;
    var channelName = KANE_QuNeo.getChannelName(deck);
    var closest = [undefined,2]; // next hotcue wrt current playpos as [control,pos]
    var position = engine.getValue(channelName,"visual_playposition"); // song position
    var trackSamples = engine.getValue(channelName,"track_samples")
    var on = [], LEDs = [[0x71,0x73,0x75,0x77,
			  0x61,0x63,0x65,0x67,
			  0x51,0x53,0x55,0x57,
			  0x41,0x43,0x45,0x47],
			 [0x79,0x7b,0x7d,0x7f,
			  0x69,0x6b,0x6d,0x6f,
			  0x59,0x5b,0x5d,0x5f,
			  0x49,0x4b,0x4d,0x4f]];

    // deal with all present hotcues
    var cuesToCome = 0;
    for (var cue = 1; cue <= KANE_QuNeo.numHotcues; cue++) {
	var cuePosition = engine.getValue(channelName,"hotcue_"+cue+"_position") /
	    trackSamples;

	// if cue is enabled and deck has a track loaded,
	if (!((cuePosition < 0 && cuePosition > -1e-7) ||
	      cuePosition == -Infinity)) {

	    // if in main DJ mode, light past hotcues red,
	    // upcoming hotcues green, and the next hotcue orange.
	    if (KANE_QuNeo.mode == 13) { 
		var red = LEDs[channel][cue - 1];

		// check to see if this hotcue is coming up
		var diff = cuePosition - position;

		if (diff >= 0) { // if we have not yet passed this hotcue,
		    var green = red - 1;
		    if (cuePosition < closest[1]) { // and it is closer than the closest,
			cuesToCome++; // increment count of pending cues
			on.push(closest[0]); // then light the old closest green
			closest = [green,cuePosition]; // because this is now closest

		    } else // it's not the closest
			on.push(green); //and we haven't passed it, so make it green

		    
		} else // we already passed this hotcue, so light it red
		    on.push(red);

	    } else // must be in a cue mode => enabled hotcues are all green
		on.push(LEDs[channel][cue - 1] - 1); // final - 1 to switch from
	                                             // green to red
	}
    }

    // then update global variable for the position of the next hotcue to be reached
    if (closest[1] < 2) { // if there is a hotcue in front of us, light it orange
	KANE_QuNeo.nextHotcuePosition[channel] = closest[1];
	var green = closest[0];
	on.push(green) // together, green and
	on.push(green + 1) // red make orange
    }
    // and update global variable for how many cues are set
    KANE_QuNeo.numNextHotcues[channel] = cuesToCome;

    // but don't forget to emit LED updates!
    KANE_QuNeo.hotcueActivateLEDs[channel] = on; // then turn on the new ones
    KANE_QuNeo.triggerVuMeter(deck); // finally trigger deck VuMeter
}

KANE_QuNeo.assertHotcueClearLEDs = function (deck) {
    var channel = deck - 1;
    var channelName = KANE_QuNeo.getChannelName(deck);
    var on = [], LEDs = [[0x79,0x7b,0x7d,0x7f,
			  0x69,0x6b,0x6d,0x6f,
			  0x59,0x5b,0x5d,0x5f,
			  0x49,0x4b,0x4d,0x4f],
			 [0x71,0x73,0x75,0x77,
			  0x61,0x63,0x65,0x67,
			  0x51,0x53,0x55,0x57,
			  0x41,0x43,0x45,0x47]];
    for (var cue = 1; cue <= KANE_QuNeo.numHotcues; cue++) {
	if (engine.getValue(channelName,"hotcue_"+cue+"_enabled"))
	    on.push(LEDs[deck - 1][cue - 1]) // add LED if cue enabled
    }
    // emit updates
    KANE_QuNeo.hotcueClearLEDs[channel] = on;
    KANE_QuNeo.triggerVuMeter(deck); // trigger deck VuMeter
}

KANE_QuNeo.clearHotcueLEDs = function (deck) {
    var channel = deck - 1;
    var LEDs = [[0x79,0x7b,0x7d,0x7f,
		 0x69,0x6b,0x6d,0x6f,
		 0x59,0x5b,0x5d,0x5f,
		 0x49,0x4b,0x4d,0x4f],
		[0x71,0x73,0x75,0x77,
		 0x61,0x63,0x65,0x67,
		 0x51,0x53,0x55,0x57,
		 0x41,0x43,0x45,0x47]];
    for (var i = 0; i < LEDs[channel].length; i++) {  // for all LEDs of this deck
	var red = LEDs[channel][i];
	var green = red - 1;
	KANE_QuNeo.LEDs(0x91,[red,green],0x00); // reset green and red
    }
}

KANE_QuNeo.assertHorizArrowLEDs = function (deck) {
    if (deck < 5) // if dealing with actual deck
	KANE_QuNeo.assertDeckHorizArrowLEDs(deck);
    else // dealing with sampler deck
	KANE_QuNeo.assertSamplerHorizArrowLEDs(deck);
}

KANE_QuNeo.assertSamplerHorizArrowLEDs = function (deck) {
    print("Sampler Horiz Arrow LEDs called for deck: "+deck)
}

KANE_QuNeo.assertDeckHorizArrowLEDs = function (deck) {
    var on = [];
    var mode = KANE_QuNeo.mode;
    if (mode != 9 && mode != 10) { // if not in sampler mode
	var channel = deck - 1;
	var channelName = KANE_QuNeo.getChannelName(deck)
	var LEDGroup = KANE_QuNeo.getLEDGroup(deck)

	// nested arrays of controls,LEDs for deck1,LEDs for deck2
	var controls = [["keylock",0x24,0x25],
			["pfl",0x26,0x27]];
	if (mode != 14 && mode != 15) { // if in neither cuing mode,
	    controls.push(["flanger",0x2a,0x2b]); // assert fx leds
	    controls.push(["slip_enabled",0x28,0x29]); // and assert slip leds
	}

	// check which controls are enabled
	for (var i = 0; i < controls.length; i++) {
	    if (engine.getValue(channelName, controls[i][0])) // if val on,
		on.push(controls[i][LEDGroup]); // light led
	}
    }
    // update LEDs
    // turn off all LEDs which were on
    KANE_QuNeo.LEDs(0x90,KANE_QuNeo.horizArrowLEDs[channel],0x00)
    KANE_QuNeo.horizArrowLEDs[channel] = on;
    KANE_QuNeo.triggerVuMeter(deck);
}

KANE_QuNeo.assertLoadLEDs = function () {
    var on = [], LEDs = [[0x08,0x09],[0x0e,0x0f],[undefined,undefined],[undefined,undefined],[0x00,0x01],[0x02,0x03],[0x04,0x05],[0x06,0x07]];
    // choose color based on whether or not the track is playing
    for (var channel = 0; channel < KANE_QuNeo.numDecks; channel++) {
	if (KANE_QuNeo.trackPlaying[channel]) // if track is playing,
	    on.push(LEDs[channel][1]) // light it red
	else // if track not playing,
	    on.push(LEDs[channel][0]) // light it green
    }
    // emit updates
    KANE_QuNeo.loadLEDs = on;
    for (var i = 0; i < LEDs.length; i++)
	KANE_QuNeo.LEDs(0x90,LEDs[i],0x00); // turn off all old LEDs
    KANE_QuNeo.triggerVuMeter(0); // trigger master VU
}

KANE_QuNeo.assertKillLEDs = function (deck) {
    var channelName = KANE_QuNeo.getChannelName(deck)
    var on = [], off = [], i; // i for loop iteration
    var LEDGroup = KANE_QuNeo.getLEDGroup(deck);
    var controls = [["filterHighKill",0x37,0x39,0x77,0x79],
		    ["filterMidKill",0x27,0x29,0x67,0x69],
		    ["filterLowKill",0x17,0x19,0x57,0x59]];
    for (i = 0; i < controls.length; i++) {
	if (engine.getValue(channelName, controls[i][0])) // if val on,
	    on.push(controls[i][LEDGroup]); // light led
	else // if val off,
	    off.push(controls[i][LEDGroup]) // turn off led
    }
    // update LEDs
    KANE_QuNeo.LEDs(0x91,on,0x7f)
    KANE_QuNeo.LEDs(0x91,off,0x00)
}

KANE_QuNeo.assertLEDOn = function (channel, control, value, status, group) {
    //turns on the assertLED button after release
    print("LEDs asserted")
    KANE_QuNeo.assertLED = [0x22];
    KANE_QuNeo.triggerVuMeter(0) // trigger master VuMeter
}

KANE_QuNeo.assertPlayScratchLED = function () {
    var on = [];
    if (KANE_QuNeo.playScratchToggle) // if in play mode
	on = [0x23];
    // emit updates
    KANE_QuNeo.playScratchLED = on;
    KANE_QuNeo.LEDs(0x90,[0x23],0x00) // turn off LED in case it's off
    KANE_QuNeo.triggerVuMeter(0) // trigger master VuMeter
}

KANE_QuNeo.assertRecordLED = function () {
    var brightness = 0x00
    if (KANE_QuNeo.recordToggle) // if in record mode
	brightness = 0x7f // set led to max
    midi.sendShortMsg(0x90,0x21,brightness); // emit update
}

KANE_QuNeo.assertJumpDirectionLEDs = function (deck) {
    var channel = deck - 1;
    var on = [], off = []; // controls for which LEDs to turn on and off
    var LEDGroup = KANE_QuNeo.getLEDGroup(deck);

    // Determine which LEDs to turn on
    switch (KANE_QuNeo.trackJump[channel]) {
    case -1: // jump backward is on
	switch (LEDGroup) {
	case 1:
	    on = [0x11], off = [0x13];
	    break;
	case 2:
	    on = [0x1d], off = [0x1f];
	    break;
	case 3:
	    on = [0x51], off = [0x53];
	    break;
	case 4:
	    on = [0x5d], off = [0x5f];
	    break;
	} break;
    case 0: // neither jump is on
	switch (LEDGroup) {
	case 1:
	    off = [0x11,0x13];
	    break;
	case 2:
	    off = [0x1d,0x1f];
	    break;
	case 3:
	    off = [0x51,0x53];
	    break;
	case 4:
	    off = [0x5d,0x5f];
	    break;
	} break;
    case 1: // jump forward is on
	switch (LEDGroup) {
	case 1:
	    on = [0x13], off = [0x11];
	    break;
	case 2:
	    on = [0x1f], off = [0x1d];
	    break;
	case 3:
	    on = [0x53], off = [0x51];
	    break;
	case 4:
	    on = [0x5f], off = [0x5d];
	    break;
	} break;
    }

    // Now send actual midi messages
    KANE_QuNeo.jumpDirectionLEDs[channel] = on;
    KANE_QuNeo.triggerVuMeter(deck);
    KANE_QuNeo.LEDs(0x91,off,0x00)
}

KANE_QuNeo.assertLoopingLED = function (deck) {
    var channel = deck - 1;
    var on = [], off = []; // arrays to control which LEDs to change

    // now handle based on loop status and LED group
    if (KANE_QuNeo.trackLooping[channel]) { // if in loop mode
	switch (KANE_QuNeo.getLEDGroup(deck)) {
	case 1:
	    on = [0x14,0x15]; //red on, green on
	    break;
	case 2:
	    on = [0x1a,0x1b]; //red on, green on
	    break;
	case 3:
	    on = [0x54,0x55]; //red on, green on
	    break;
	case 4:
	    on = [0x5a,0x5b]; //red on, green on
	    break;
	}
    } else { // if not in looping mode
	switch (KANE_QuNeo.getLEDGroup(deck)) {
	case 1:
	    on = [0x14], off = [0x15]; //green on, red off
	    break;
	case 2:
	    on = [0x1a], off = [0x1b]; //green on, red off
	    break;
	case 3:
	    on = [0x54], off = [0x55]; //green on, red off
	    break;
	case 4:
	    on = [0x5a], off = [0x5b]; //green on, red off
	    break;
	}
    }
    // emit updates
    KANE_QuNeo.loopingLED[channel] = on;
    KANE_QuNeo.triggerVuMeter(deck);
    KANE_QuNeo.LEDs(0x91,off,0x00)
}

KANE_QuNeo.assertBeatCounterLEDs = function (deck) {
    var channel = deck - 1;
    var channelName = KANE_QuNeo.getChannelName(deck)
    var beat = KANE_QuNeo.wholeBeat[channel];
    var mode = KANE_QuNeo.mode

    if (mode == 13 || mode == 14 || mode == 15) { // only these modes
	// first determine with which LEDs we are working 
	var LEDGroup = KANE_QuNeo.getLEDGroup(deck);
	var ones, fours, on = [];
	if (LEDGroup == 1) { // side of deck 1
	    ones = [0x06,0x07];
	    fours = [0x04,0x05];
	} else if (LEDGroup == 2) { // side of deck 2
	    ones = [0x08,0x09];
	    fours = [0x0a,0x0b];
	} else print("ERROR: incompatible deck given to assertBeatCounterLEDs")

	// now, if a track is loaded, decide which LEDs to turn on
	if (engine.getValue(channelName,"duration") != 0) {

	    // first deal with the ones counter
	    var beatmod4 = ((beat - 1) % 4) + 1;
	    switch (beatmod4) {
	    case 1: break; // off on beat 1
	    case 2: 
		on.push(ones[0]) // green on
		break;
	    case 3:
		on.push(ones[1]) // red on
		break;
	    case 4:
		on = on.concat(ones) // orange on
		break;
	    }
	    // now the fours counter
	    if (beat < 5) 
		undefined; // led off
	    else if (beat < 9)
		on.push(fours[0]) // green on
	    else if (beat < 13)
		on.push(fours[1]) // red on
	    else 
		on = on.concat(fours) // orange on
	} else // if a track is not loaded, no beat counter LEDs should be on
	    on = [];

	// emit updates
	KANE_QuNeo.LEDs(0x91,ones.concat(fours),0x00) // old LEDs off
	KANE_QuNeo.beatCounterLEDs[channel] = on;
	KANE_QuNeo.triggerVuMeter(deck);
    }
}

KANE_QuNeo.assertBeatLEDs = function (deck) {
    // do nothing if not in these modes
    var mode = KANE_QuNeo.mode
    if (!(mode == 13 || mode == 14 || mode == 15))
	return;

    var channel = deck - 1;
    var channelName = KANE_QuNeo.getChannelName(deck)
    // arrays to control which LEDs to change
    var on, greens, reds, reloop, reloopOn = []; 
    var LEDGroup = KANE_QuNeo.getLEDGroup(deck);

    // start by clearing reds and lighting greens. also determine which LEDs we have
    // for reloop based on which deck we are checking
    switch (LEDGroup) {
    case 1:
	greens = [0x20,0x22,0x30,0x32];
	reds = [0x21,0x23,0x31,0x33];
	reloop = [0x00,0x01];
	break;
    case 2:
	greens = [0x2c,0x2e,0x3c,0x3e];
	reds = [0x2d,0x2f,0x3d,0x3f];
	reloop = [0x0e,0x0f]
	break;
    case 3:
	greens = [0x60,0x62,0x70,0x72];
	reds = [0x61,0x63,0x71,0x73];
	reloop = [0x40,0x41];
	break;
    case 4:
	greens = [0x6c,0x6e,0x7c,0x7e];
	reds = [0x6d,0x6f,0x7d,0x7f];
	reloop = [0x4e,0x4f]
	break;
    }
    on = greens;

    // controls to consider, put in an array with affected LEDs grouped
    var controls = [["beatloop_1_enabled",[0x31],[0x3d],[0x71],[0x7d]],
		    ["beatloop_2_enabled",[0x33],[0x3f],[0x73],[0x7f]],
		    ["beatloop_4_enabled",[0x21],[0x2d],[0x61],[0x6d]],
		    ["beatloop_8_enabled",[0x23],[0x2f],[0x63],[0x6f]]];

    // now consider those controls
    for (var i = 0; i < controls.length; i++) {
	if (engine.getValue(channelName, controls[i][0])) { // if a loop is on,
	    // set that loop and only that loop to shine.
	    on = on.concat(controls[i][LEDGroup]);
	    break;
	}
    }
    if (engine.getValue(channelName, "loop_enabled"))
	reloopOn = reloop; // light reloop LED if there is an enabled loop

    // update LEDs
    KANE_QuNeo.reloopLEDs[channel] = reloopOn; // then update those that should be on
    KANE_QuNeo.jumpLoopLEDs[channel] = on;
    KANE_QuNeo.LEDs(0x91,reds.concat(reloop),0x00) // turn off those not on

    // And finally, check and update for pending loops
    numBeats = KANE_QuNeo.loopNextJump[channel];
    if (numBeats)
	KANE_QuNeo.assertJumpLEDs(deck, numBeats);

    KANE_QuNeo.triggerVuMeter(deck); // trigger VuMeter updates
}

KANE_QuNeo.assertJumpSyncLED = function (deck) {
    var channel = deck - 1;
    var on, off; // arrays to control which LEDs to change

    // deal with each pair of decks separately
    if (deck == 1 || deck == 3) { // if dealing with decks 1 or 3
	if (KANE_QuNeo.trackJumpSync[channel]) // if in jumpsync mode
	    on = [0x02], off = []; //green on
	else                                   // if not in jumpsync mode
	    on = [], off = [0x02]; //green off
    }
    else if (deck == 2 || deck == 4) { // if dealing with decks 2 or 4
	if (KANE_QuNeo.trackJumpSync[channel]) // if in jumpsync mode
	    on = [0x0c], off = []; //green on
	else                                   // if not in jumpsync mode
	    on = [], off = [0x0c]; //green off
    } else {
	print("ERROR: assertJumpSyncLED called with invalid deck: "+deck)
	return;
    }
    off.push(0x03); // both reds off
    off.push(0x0d);
    // emit updates
    KANE_QuNeo.LEDs(0x91,off,0x00);
    KANE_QuNeo.jumpSyncLED[channel] = on;
    KANE_QuNeo.triggerVuMeter(deck);
}

/***** (ELED) Engine LED Connections *****/

KANE_QuNeo.triggerVuMeter = function (deck) {
    var channelName = KANE_QuNeo.getChannelName(deck);
    engine.trigger(channelName,"VuMeter")
}

// VuMeters
KANE_QuNeo.deckVuMeter = function (deck, value) {
    var channelName = KANE_QuNeo.getChannelName(deck);
    var channel = deck - 1;
    // adjust from 0...1 to 0...127
    var values = KANE_QuNeo.getForwardValues(value);

    // Cue Visualizer Region's Beat LEDs
    if (KANE_QuNeo.beatLEDsOn)
	KANE_QuNeo.LEDs(0x91,KANE_QuNeo.activeBeatLEDs[channel],
			values.cubedNeverOff);
	
    // Vertical Arrow Beat LEDs
    if (!KANE_QuNeo.rateNudge) {
	var controls = [[0x2e,0x2f],[0x30,0x31]];
	KANE_QuNeo.LEDs(0x90,controls[channel],values.cubedNeverOff);
    }

    // Vertical Vu Meters, only when in modes which use slider modes
    mode = KANE_QuNeo.mode
    if (KANE_QuNeo.sliderMode == 2 &&
	(mode == 13 || mode == 14 || mode == 15 || mode == 16)) {
	var level;
	if (KANE_QuNeo.trackPlaying[channel]) // if track is playing, do VuMeter
	    level = values.squared;
	else // else do volume
	    level = KANE_QuNeo.scaleToSlider(engine.getValue(channelName,"volume"))
	KANE_QuNeo.LEDs(0xb0,KANE_QuNeo.getSliderControl(deck,0),level);
    }

    // Hotcue LEDs
    KANE_QuNeo.LEDs(0x91,KANE_QuNeo.hotcueActivateLEDs[channel],values.cubedNeverOff)
    KANE_QuNeo.LEDs(0x91,KANE_QuNeo.hotcueClearLEDs[channel],values.cubedNeverOff)
    // Beat Jump LEDs
    KANE_QuNeo.LEDs(0x91,KANE_QuNeo.jumpLoopLEDs[channel],values.cubedNeverOff);
    // Jump Direction LEDs
    KANE_QuNeo.LEDs(0x91,KANE_QuNeo.jumpDirectionLEDs[channel],values.cubedNeverOff);
    // Looping LED
    KANE_QuNeo.LEDs(0x91,KANE_QuNeo.loopingLED[channel],values.cubedNeverOff);
    // Reloop LEDs
    KANE_QuNeo.LEDs(0x91,KANE_QuNeo.reloopLEDs[channel],values.cubedNeverOff);
    // Horizontal Arrow LEDs
    KANE_QuNeo.LEDs(0x90,KANE_QuNeo.horizArrowLEDs[channel],values.cubedNeverOff);
    // Jump Sync LED
    KANE_QuNeo.LEDs(0x91,KANE_QuNeo.jumpSyncLED[channel],values.cubedNeverOff);
    // Beat Counter LEDs
    KANE_QuNeo.LEDs(0x91,KANE_QuNeo.beatCounterLEDs[channel],values.cubedNeverOff);
}

KANE_QuNeo.masterVuMeter = function (value) {
    // adjust from 0...1 to 0...127
    var values = KANE_QuNeo.getForwardValues(value);
    
    // top horizontal slider: master volume
    var level = -1; // -1 for no playing decks
    for (var channel = 0; channel < KANE_QuNeo.numDecks; channel++) {
	if (KANE_QuNeo.trackPlaying[channel])  { // if any deck is playing,
	    level = values.squared; break; // do VuMeter then break
	}
    }
    if (level == -1) // if there were no playing decks, display volume
	level = KANE_QuNeo.oneToFiveKnob(engine.getValue("[Master]","volume"))
    KANE_QuNeo.LEDs(0xb0,0x0b,level);

    // AssertLED Button
    KANE_QuNeo.LEDs(0x90,KANE_QuNeo.assertLED,values.cubedNeverOff)
    // PlayScratch LED
    KANE_QuNeo.LEDs(0x90,KANE_QuNeo.playScratchLED,values.cubedNeverOff);
    // Load LEDs
    KANE_QuNeo.LEDs(0x90,KANE_QuNeo.loadLEDs,values.cubedNeverOff);
}

// Sliders
KANE_QuNeo.deckZoomLEDs = function (deck, value) {
    var LEDGroup = KANE_QuNeo.getLEDGroup(deck);
    // normalize zoom LED value to be 0-127
    var zoom = ((value - 1) / 5) * 127
    // determine which control we are manipulating
    var control = KANE_QuNeo.getSliderControl(deck, 0)
    // emit message
    KANE_QuNeo.LEDs(0xb0,control,127 - zoom) // inverted because high is zoomed in
}	

// Slider Mode == 0
KANE_QuNeo.deckCursorLEDs = function (deck, position) {
    var channel = deck - 1;
    var LEDGroup = KANE_QuNeo.getLEDGroup(deck);
    // normalize position LED value to be 0-127
    var normalized = position * 127;
    // determine which control we are manipulating
    var control = KANE_QuNeo.getSliderControl(deck, 1)
    // emit LED message
    KANE_QuNeo.LEDs(0xb0,control,127 - normalized) // inverted to show time left
}


// Slider Mode == 1
KANE_QuNeo.deckPregain = function (deck, value) {
    var gain, LEDGroup = KANE_QuNeo.getLEDGroup(deck)
    
    // determine gain
    if (value < 1) // first half of the knob, 0-1
	gain = value * 127 / 2;
    else // second half of the knob, 1-4
	gain = 63.5 + (value - 1) * 63.5 / 3;
 
    // now determine control
    var control = KANE_QuNeo.getSliderControl(deck, 0)
    midi.sendShortMsg(0xb0,control,0x00+gain);
}

KANE_QuNeo.deckRate = function (deck, value) {
    var LEDGroup = KANE_QuNeo.getLEDGroup(deck)
    // determine rate
    var rate = (value * 63.5) + 63.5 // scaled for -1...1 to map to 0...127
    // now determine control
    var control = KANE_QuNeo.getSliderControl(deck, 1)
    // emit message
    midi.sendShortMsg(0xb0,control,0x00+rate);
}

KANE_QuNeo.deckHighs = function (deck, value) {
    var level, control;
    // determine level
    if (value < 1) // first half of the knob, 0-1
	level = value * 127 / 2;
    else // second half of the knob, 1-4
	level = 63.5 + (value - 1) * 63.5 / 3
    
    // now determine control
    var control = KANE_QuNeo.getSliderControl(deck, 1)
    // emit updates
    midi.sendShortMsg(0xb0,control,0x00+level);
}

// Slider Mode == 3
KANE_QuNeo.deckMids = function (deck, value) {
    var level, control;
    // determine level
    if (value < 1) // first half of the knob, 0-1
	level = value * 127 / 2;
    else // second half of the knob, 1-4
	level = 63.5 + (value - 1) * 63.5 / 3
    
    // now determine control
    var control = KANE_QuNeo.getSliderControl(deck, 0)
    // emit updates
    midi.sendShortMsg(0xb0,control,0x00+level);
}

KANE_QuNeo.deckLows = function (deck, value) {
    var level, control;
    // determine level
    if (value < 1) // first half of the knob, 0-1
	level = value * 127 / 2;
    else // second half of the knob, 1-4
	level = 63.5 + (value - 1) * 63.5 / 3
    
    // now determine control
    var control = KANE_QuNeo.getSliderControl(deck, 1)
    // emit updates
    midi.sendShortMsg(0xb0,control,0x00+level);
}

// Other Sliders

KANE_QuNeo.deck1LeftVol = function (value) {
    var vol = Math.pow(value,3) * 127;
    midi.sendShortMsg(0xb0,0x01,vol)
}
KANE_QuNeo.deck1RightVol = function (value) {
    var vol = Math.pow(value,3) * 127;
    midi.sendShortMsg(0xb0,0x02,vol)
}

KANE_QuNeo.deck2LeftVol = function (value) {
    var vol = Math.pow(value,3) * 127;
    midi.sendShortMsg(0xb0,0x03,vol)
}
KANE_QuNeo.deck2RightVol = function (value) {
    var vol = Math.pow(value,3) * 127;
    midi.sendShortMsg(0xb0,0x04,vol)
}

KANE_QuNeo.headVol = function (value) {
    var vol = KANE_QuNeo.oneToFiveKnob(value)
    midi.sendShortMsg(0xb0,0x0a,0x00+vol);
}

KANE_QuNeo.flangerPeriod = function (value) {
    var flangePeriod = (value - 50) * 127 / 2000000;
    midi.sendShortMsg(0xb0,0x09,0x00+flangePeriod);
}

KANE_QuNeo.flangerDepth = function (value) {
    var flangeDepth = value * 127;
    midi.sendShortMsg(0xb0,0x08,0x00+flangeDepth);
}

KANE_QuNeo.crossFader = function (value) {
    var crossFade = (value * 63.5) + 63.5; // split slider in half
    midi.sendShortMsg(0xb0,0x05,0x00+crossFade);
}

// -----------------------------------------------------
// begin dispatch section
// -----------------------------------------------------

/***** (VSD) Vertical Slider Dispatches *****/

// Moves
KANE_QuNeo.verticalSlider1Move = function (channel, control, value, status, group) {
    KANE_QuNeo.verticalSliderMove(1, value, 0); // 1 for deck 1, 0 for no reset
}
KANE_QuNeo.verticalSlider2Move = function (channel, control, value, status, group) {
    KANE_QuNeo.verticalSliderMove(2, value, 0);
}
KANE_QuNeo.verticalSlider3Move = function (channel, control, value, status, group) {
    KANE_QuNeo.verticalSliderMove(3, value, 0);
}
KANE_QuNeo.verticalSlider4Move = function (channel, control, value, status, group) {
    KANE_QuNeo.verticalSliderMove(4, value, 0);
}

//Touches (taps)
KANE_QuNeo.verticalSlider1Touch = function (channel, control, value, status, group) {
    KANE_QuNeo.verticalSliderTouch(1, value); // 1 for deck 1
}
KANE_QuNeo.verticalSlider2Touch = function (channel, control, value, status, group) {
    KANE_QuNeo.verticalSliderTouch(2, value);
}
KANE_QuNeo.verticalSlider3Touch = function (channel, control, value, status, group) {
    KANE_QuNeo.verticalSliderTouch(3, value);
}
KANE_QuNeo.verticalSlider4Touch = function (channel, control, value, status, group) {
    KANE_QuNeo.verticalSliderTouch(4, value);
}

// Vertical Slider LEDs
KANE_QuNeo.deck1ZoomLEDs = function (value) {
    KANE_QuNeo.deckZoomLEDs(1,value)
}
KANE_QuNeo.deck2ZoomLEDs = function (value) {
    KANE_QuNeo.deckZoomLEDs(2,value)
}
KANE_QuNeo.deck1CursorLEDs = function (value) {
    KANE_QuNeo.deckCursorLEDs(1,value) // 1 for deck 1
}
KANE_QuNeo.deck2CursorLEDs = function (value) {
    KANE_QuNeo.deckCursorLEDs(2,value)
}
KANE_QuNeo.deck1Pregain = function (value) {
    KANE_QuNeo.deckPregain(1,value)
}
KANE_QuNeo.deck2Pregain = function (value) {
    KANE_QuNeo.deckPregain(2,value)
}
KANE_QuNeo.deck1Rate = function (value) {
    KANE_QuNeo.deckRate(1,value)
}
KANE_QuNeo.deck2Rate = function (value) {
    KANE_QuNeo.deckRate(2,value)
}
KANE_QuNeo.deck1Highs = function (value) {
    KANE_QuNeo.deckHighs(1,value)
}
KANE_QuNeo.deck2Highs = function (value) {
    KANE_QuNeo.deckHighs(2,value)
}
KANE_QuNeo.deck1Mids = function (value) {
    KANE_QuNeo.deckMids(1,value)
}
KANE_QuNeo.deck2Mids = function (value) {
    KANE_QuNeo.deckMids(2,value)
}
KANE_QuNeo.deck1Lows = function (value) {
    KANE_QuNeo.deckLows(1,value)
}
KANE_QuNeo.deck2Lows = function (value) {
    KANE_QuNeo.deckLows(2,value)
}

/***** (AD) LED Assertion Dispatches *****/

KANE_QuNeo.assert5LEDs = function (channel, control, value, status, group) {
    KANE_QuNeo.assertLEDs(5) // assert mode 5 LEDs
}
KANE_QuNeo.assert13LEDs = function (channel, control, value, status, group) {
    KANE_QuNeo.assertLEDs(13)
}
KANE_QuNeo.assert14LEDs = function (channel, control, value, status, group) {
    KANE_QuNeo.assertLEDs(14)
}
KANE_QuNeo.assert15LEDs = function (channel, control, value, status, group) {
    KANE_QuNeo.assertLEDs(15)
}
KANE_QuNeo.assert16LEDs = function (channel, control, value, status, group) {
    KANE_QuNeo.assertLEDs(16)
}

/***** (DD) Deck Dispatches *****/

//VuMeters
KANE_QuNeo.deck1VuMeter = function (value) {
    KANE_QuNeo.deckVuMeter(1,value)
}
KANE_QuNeo.deck2VuMeter = function (value) {
    KANE_QuNeo.deckVuMeter(2,value)
}
KANE_QuNeo.deck3VuMeter = function (value) {
    KANE_QuNeo.deckVuMeter(3,value)
}
KANE_QuNeo.deck4VuMeter = function (value) {
    KANE_QuNeo.deckVuMeter(4,value)
}
KANE_QuNeo.deck5VuMeter = function (value) {
    KANE_QuNeo.deckVuMeter(5,value)
}
KANE_QuNeo.deck6VuMeter = function (value) {
    KANE_QuNeo.deckVuMeter(6,value)
}
KANE_QuNeo.deck7VuMeter = function (value) {
    KANE_QuNeo.deckVuMeter(7,value)
}
KANE_QuNeo.deck8VuMeter = function (value) {
    KANE_QuNeo.deckVuMeter(8,value)
}

//Reloop
KANE_QuNeo.deck1reloop = function (channel, control, value, status, group) {
    KANE_QuNeo.deckReloop(1) // 1 for deck 1
}
KANE_QuNeo.deck2reloop = function (channel, control, value, status, group) {
    KANE_QuNeo.deckReloop(2) // 2 for deck 2
}

//Loop double/halve
KANE_QuNeo.deck1loop_double = function (channel, control, value, status, group) {
    KANE_QuNeo.deckMultiplyLoop(1, 2) // 1 for deck 1, 2 for double
}
KANE_QuNeo.deck1loop_halve = function (channel, control, value, status, group) {
    KANE_QuNeo.deckMultiplyLoop(1, 1/2) // 1 for deck 1, 1/2 for halve
}
KANE_QuNeo.deck2loop_double = function (channel, control, value, status, group) {
    KANE_QuNeo.deckMultiplyLoop(2, 2)
}
KANE_QuNeo.deck2loop_halve = function (channel, control, value, status, group) {
    KANE_QuNeo.deckMultiplyLoop(2, 1/2)
}

//Playlist Mode
KANE_QuNeo.touchPlaylist = function (channel, control, value, status, group) {
    KANE_QuNeo.touchScroll(1); // scroll touched for deck 1
}
KANE_QuNeo.touchTracklist = function (channel, control, value, status, group) {
    KANE_QuNeo.touchScroll(2); // scroll touched for deck 2
}

//JumpSyncing
KANE_QuNeo.toggle1JumpSync = function (channel, control, value, status, group) {
    KANE_QuNeo.toggleJumpSync(1); // toggle jumpsync for deck 1
}

KANE_QuNeo.toggle2JumpSync = function (channel, control, value, status, group) {
    KANE_QuNeo.toggleJumpSync(2);
}

//Jumping
KANE_QuNeo.jump1Forward = function (channel, control, value, status, group) {
    KANE_QuNeo.setJump(1, 1); // channel 1 to jump status 1
}

KANE_QuNeo.jump2Forward = function (channel, control, value, status, group) {
    KANE_QuNeo.setJump(2, 1);
}

KANE_QuNeo.jump1Backward = function (channel, control, value, status, group) {
    KANE_QuNeo.setJump(1, -1);
}

KANE_QuNeo.jump2Backward = function (channel, control, value, status, group) {
    KANE_QuNeo.setJump(2, -1);
}

//Looping
KANE_QuNeo.toggle1Looping = function (channel, control, value, status, group) {
    KANE_QuNeo.toggleLooping(1)
}

KANE_QuNeo.toggle2Looping = function (channel, control, value, status, group) {
    KANE_QuNeo.toggleLooping(2)
}

//JumpLooping
KANE_QuNeo.deck1JumpLoop1 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(1,1) // deck 1, 1 beat jump and/or loop
}

KANE_QuNeo.deck1JumpLoop2 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(1,2)
}

KANE_QuNeo.deck1JumpLoop4 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(1,4)
}

KANE_QuNeo.deck1JumpLoop8 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(1,8)
}

KANE_QuNeo.deck2JumpLoop1 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(2,1)
}

KANE_QuNeo.deck2JumpLoop2 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(2,2)
}

KANE_QuNeo.deck2JumpLoop4 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(2,4)
}

KANE_QuNeo.deck2JumpLoop8 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(2,8)
}

//JumpOff

KANE_QuNeo.deck1JumpOff1 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(1,1)
}

KANE_QuNeo.deck1JumpOff2 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(1,2)
}

KANE_QuNeo.deck1JumpOff4 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(1,4)
}

KANE_QuNeo.deck1JumpOff8 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(1,8)
}

KANE_QuNeo.deck2JumpOff1 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(2,1)
}

KANE_QuNeo.deck2JumpOff2 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(2,2)
}

KANE_QuNeo.deck2JumpOff4 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(2,4)
}

KANE_QuNeo.deck2JumpOff8 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(2,8)
}

//Rotaries
KANE_QuNeo.rotary1Touch = function (channel, control, value, status, group) {
    KANE_QuNeo.rotaryTouch(1, value, status);
}

KANE_QuNeo.rotary2Touch = function (channel, control, value, status, group) {
    KANE_QuNeo.rotaryTouch(2, value, status);
}

KANE_QuNeo.rotary3Touch = function (channel, control, value, status, group) {
    KANE_QuNeo.rotaryTouch(3, value, status);
}

KANE_QuNeo.rotary4Touch = function (channel, control, value, status, group) {
    KANE_QuNeo.rotaryTouch(4, value, status);
}

KANE_QuNeo.rotary5Touch = function (channel, control, value, status, group) {
    KANE_QuNeo.rotaryTouch(5, value, status);
}

KANE_QuNeo.rotary6Touch = function (channel, control, value, status, group) {
    KANE_QuNeo.rotaryTouch(6, value, status);
}

KANE_QuNeo.wheel1Turn = function (channel, control, value, status, group) {
    KANE_QuNeo.wheelTurn(1, value);
}

KANE_QuNeo.wheel2Turn = function (channel, control, value, status, group) {
    KANE_QuNeo.wheelTurn(2, value);
}

//Timekeeper
KANE_QuNeo.time1Keeper = function (value) {
    KANE_QuNeo.timeKeeper(1, value);
}

KANE_QuNeo.time2Keeper = function (value) {
    KANE_QuNeo.timeKeeper(2, value);
}

//Visual Nudge
KANE_QuNeo.visualNudge1Forward = function (channel, control, value, status, group) {
    KANE_QuNeo.visualNudge(1, 1) // 1 for deck 1, 1 for forward direction
}

KANE_QuNeo.visualNudge1Backward = function (channel, control, value, status, group) {
    KANE_QuNeo.visualNudge(1, -1) // 1 for deck 1, -1 for backward direction
}

KANE_QuNeo.visualNudge2Forward = function (channel, control, value, status, group) {
    KANE_QuNeo.visualNudge(2, 1) // 2 for deck 1, 1 for forward direction
}

KANE_QuNeo.visualNudge2Backward = function (channel, control, value, status, group) {
    KANE_QuNeo.visualNudge(2, -1) // 2 for deck 1, -1 for backward direction
}

KANE_QuNeo.visualNudge1ForwardOff = function (channel, control, value, status, group) {
    KANE_QuNeo.visualNudgeOff(1, 1) // 1 for deck 1, 1 for forward direction
}

KANE_QuNeo.visualNudge1BackwardOff = function (channel, control, value, status, group) {
    KANE_QuNeo.visualNudgeOff(1, -1) // 1 for deck 1, 1 for backward direction
}

KANE_QuNeo.visualNudge2ForwardOff = function (channel, control, value, status, group) {
    KANE_QuNeo.visualNudgeOff(2, 1) // 2 for deck 1, 1 for forward direction
}

KANE_QuNeo.visualNudge2BackwardOff = function (channel, control, value, status, group) {
    KANE_QuNeo.visualNudgeOff(2, -1) // 2 for deck 1, -1 for backward direction
}

//Periodic/Regular Nudge
KANE_QuNeo.rateNudge1Forward = function (channel, control, value, status, group) {
    KANE_QuNeo.rateNudgePress(1, 1) // 1 for deck 1, 1 for forward direction
}

KANE_QuNeo.rateNudge1Backward = function (channel, control, value, status, group) {
    KANE_QuNeo.rateNudgePress(1, -1) // 1 for deck 1, -1 for backward direction
}

KANE_QuNeo.rateNudge2Forward = function (channel, control, value, status, group) {
    KANE_QuNeo.rateNudgePress(2, 1) // 2 for deck 2, 1 for forward direction
}

KANE_QuNeo.rateNudge2Backward = function (channel, control, value, status, group) {
    KANE_QuNeo.rateNudgePress(2, -1) // 2 for deck 2, -1 for backward direction
}

KANE_QuNeo.rateNudge1ForwardOff = function (channel, control, value, status, group) {
    KANE_QuNeo.rateNudgeRelease(1, 1)
}

KANE_QuNeo.rateNudge1BackwardOff = function (channel, control, value, status, group) {
    KANE_QuNeo.rateNudgeRelease(1, -1)
}

KANE_QuNeo.rateNudge2ForwardOff = function (channel, control, value, status, group) {
    KANE_QuNeo.rateNudgeRelease(2, 1)
}

KANE_QuNeo.rateNudge2BackwardOff = function (channel, control, value, status, group) {
    KANE_QuNeo.rateNudgeRelease(2, -1)
}

//Reset Beat
KANE_QuNeo.reset1Beat = function (channel, control, value, status, group) {
    KANE_QuNeo.resetBeat(1) 
}

KANE_QuNeo.reset2Beat = function (channel, control, value, status, group) {
    KANE_QuNeo.resetBeat(2) 
}

//Quantize Cues
KANE_QuNeo.quantize1Cues = function (channel, control, value, status, group) {
    KANE_QuNeo.quantizeCues(1, [0x28]);
}

KANE_QuNeo.quantize2Cues = function (channel, control, value, status, group) {
    KANE_QuNeo.quantizeCues(2, [0x29]);
}

KANE_QuNeo.quantize1CuesOff = function (channel, control, value, status, group) {
    KANE_QuNeo.quantizeCuesOff(1, [0x28]);
}

KANE_QuNeo.quantize2CuesOff = function (channel, control, value, status, group) {
    KANE_QuNeo.quantizeCuesOff(2, [0x29]);
}

	    
/***** (SD) Sampler Dispatches *****/

//JumpSyncing
KANE_QuNeo.toggle3JumpSync = function (channel, control, value, status, group) {
    KANE_QuNeo.toggleJumpSync(3); // toggle jumpsync for deck 3
}

KANE_QuNeo.toggle4JumpSync = function (channel, control, value, status, group) {
    KANE_QuNeo.toggleJumpSync(4);
}

KANE_QuNeo.toggle5JumpSync = function (channel, control, value, status, group) {
    KANE_QuNeo.toggleJumpSync(5); // toggle jumpsync for deck 5
}

KANE_QuNeo.toggle6JumpSync = function (channel, control, value, status, group) {
    KANE_QuNeo.toggleJumpSync(6);
}

//Jumping
KANE_QuNeo.jump3Forward = function (channel, control, value, status, group) {
    KANE_QuNeo.setJump(3, 1); // channel 3 to jump status 1
}

KANE_QuNeo.jump4Forward = function (channel, control, value, status, group) {
    KANE_QuNeo.setJump(4, 1);
}

KANE_QuNeo.jump3Backward = function (channel, control, value, status, group) {
    KANE_QuNeo.setJump(3, -1);
}

KANE_QuNeo.jump4Backward = function (channel, control, value, status, group) {
    KANE_QuNeo.setJump(4, -1);
}

KANE_QuNeo.jump5Forward = function (channel, control, value, status, group) {
    KANE_QuNeo.setJump(5, 1); // channel 5 to jump status 1
}

KANE_QuNeo.jump6Forward = function (channel, control, value, status, group) {
    KANE_QuNeo.setJump(6, 1);
}

KANE_QuNeo.jump5Backward = function (channel, control, value, status, group) {
    KANE_QuNeo.setJump(5, -1);
}

KANE_QuNeo.jump6Backward = function (channel, control, value, status, group) {
    KANE_QuNeo.setJump(6, -1);
}

//Looping
KANE_QuNeo.toggle3Looping = function (channel, control, value, status, group) {
    KANE_QuNeo.toggleLooping(3)
}

KANE_QuNeo.toggle4Looping = function (channel, control, value, status, group) {
    KANE_QuNeo.toggleLooping(4)
}

KANE_QuNeo.toggle5Looping = function (channel, control, value, status, group) {
    KANE_QuNeo.toggleLooping(5)
}

KANE_QuNeo.toggle6Looping = function (channel, control, value, status, group) {
    KANE_QuNeo.toggleLooping(6)
}

//JumpLooping
KANE_QuNeo.deck3JumpLoop1 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(3,1) // deck 3, 1 beat jump and/or loop
}

KANE_QuNeo.deck3JumpLoop2 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(3,2)
}

KANE_QuNeo.deck3JumpLoop4 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(3,4)
}

KANE_QuNeo.deck3JumpLoop8 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(3,8)
}

KANE_QuNeo.deck4JumpLoop1 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(4,1)
}

KANE_QuNeo.deck4JumpLoop2 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(4,2)
}

KANE_QuNeo.deck4JumpLoop4 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(4,4)
}

KANE_QuNeo.deck4JumpLoop8 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(4,8)
}

KANE_QuNeo.deck5JumpLoop1 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(5,1)
}

KANE_QuNeo.deck5JumpLoop2 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(5,2)
}

KANE_QuNeo.deck5JumpLoop4 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(5,4)
}

KANE_QuNeo.deck5JumpLoop8 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(5,8)
}

KANE_QuNeo.deck6JumpLoop1 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(6,1)
}

KANE_QuNeo.deck6JumpLoop2 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(6,2)
}

KANE_QuNeo.deck6JumpLoop4 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(6,4)
}

KANE_QuNeo.deck6JumpLoop8 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpLoop(6,8)
}

//JumpOff

KANE_QuNeo.deck3JumpOff1 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(3,1)
}

KANE_QuNeo.deck3JumpOff2 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(3,2)
}

KANE_QuNeo.deck3JumpOff4 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(3,4)
}

KANE_QuNeo.deck3JumpOff8 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(3,8)
}

KANE_QuNeo.deck4JumpOff1 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(4,1)
}

KANE_QuNeo.deck4JumpOff2 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(4,2)
}

KANE_QuNeo.deck4JumpOff4 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(4,4)
}

KANE_QuNeo.deck4JumpOff8 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(4,8)
}

KANE_QuNeo.deck5JumpOff1 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(5,1)
}

KANE_QuNeo.deck5JumpOff2 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(5,2)
}

KANE_QuNeo.deck5JumpOff4 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(5,4)
}

KANE_QuNeo.deck5JumpOff8 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(5,8)
}

KANE_QuNeo.deck6JumpOff1 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(6,1)
}

KANE_QuNeo.deck6JumpOff2 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(6,2)
}

KANE_QuNeo.deck6JumpOff4 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(6,4)
}

KANE_QuNeo.deck6JumpOff8 = function (channel, control, value, status, group) {
    KANE_QuNeo.jumpOff(6,8)
}
