// Hercules-DJ-Control-Instinct-P8-scripts.js
//
// 2017.01.23, piff, first draft
// 2018-06-19, STi, added soft-takeover to more controls, without success though
// 2018-10-20, Piega, clean js
//

// NOTE: this code is mostly copy / paste from work done by other persons so credits for the code goes to
// others. Unfortunately I did'nt track the author's name from where I copied the pieces of code.... sorry!
//

// Number of the standard RPM value. Lower values increase de sensitivity as the really records.
var standardRpm = 33.33;

// The alpha value for the filter (start with 1/8 (0.125) and tune from there)
var alpha = 1 / 8;

// The beta value for the filter (start with alpha/32 and tune from there)
var beta = alpha / 20;

// Timer to disable the scratch if the "jog wheel" is stopped for "x" milliseconds (default = 60)
var scratchResetTime = 60;

// Tune the jog sensitivity when the scratch mode is disabled (default = 1, increase for increase the sensitivity
var jogSensitivity = 0.8;

// 2-decks variables
var deckA = 1;
var deckB = 2;

// Lower jog sensitivity when selecting playlists
// Count each step until jogPlaylistSensitivityDivider is reached, then change Playlist
//
// TODO(XXX): add a jogPlaylistCounter reset timer if no jogWheel move is done for some time
//
//var jogPlaylistSensitivityDivider = 3;
//var jogPlaylistSense = 0;
//var jogPlaylistCounter = 0;

//var superButtonHold = 0;
//var automixPressed = false;
var scratchMode = 0;
var scratchTimer = 0;
var wheelMove = [0, 0];


DJControlInstinctP8 = function() {
   this.group = "[Master]";
};

DJControlInstinctP8.init = function() {
    engine.setValue("[Master]", "num_samplers", 8);

    scratch = false;
    scratchTimer = [];
    scratchTimerOn = [];

    // Switch off all LEDs
    for (var i = 1; i < 95; i++) {
        midi.sendShortMsg(0x90, i, 0x00);
    }

    engine.connectControl("[Recording]", "status", "DJControlInstinctP8.OnRecordingStatusChange");

    // Disable Soft-Takeover
    engine.softTakeover("[Channel1]", "volume", false);
    engine.softTakeover("[Channel2]", "volume", false);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", false);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", false);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", false);
    engine.softTakeover("[Master]", "balance", false);
    engine.softTakeover("[Master]", "gain", false);
    engine.softTakeover("[Channel1]", "pregain", false);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", false);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", false);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", false);
    engine.softTakeover("[Master]", "headMix", false);
    engine.softTakeover("[Master]", "headGain", false);
    engine.softTakeover("[Channel2]", "pregain", false);
    engine.softTakeover("[Master]", "crossfader", false);


    // Tell controller to send midi to update knob and slider positions.
    midi.sendShortMsg(0xB0, 0x7F, 0x7F);

    // But the rate values get messed up, so reset them
    engine.setValue("[Channel1]", "rate_set_default", 1.0);
    engine.setValue("[Channel2]", "rate_set_default", 1.0);

    // Enable Soft-Takeover
    engine.softTakeover("[Channel1]", "volume", true);
    engine.softTakeover("[Channel2]", "volume", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", true);
    engine.softTakeover("[Master]", "balance", true);
    engine.softTakeover("[Master]", "gain", true);
    engine.softTakeover("[Channel1]", "pregain", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", true);
    engine.softTakeover("[Master]", "headMix", true);
    engine.softTakeover("[Master]", "headGain", true);
    engine.softTakeover("[Channel2]", "pregain", true);
    engine.softTakeover("[Master]", "crossfader", true);
};


DJControlInstinctP8.shutdown = function() {
    // toggle all lights off.
    for (var i = 0x01; i < 0x57; i++) {
        midi.sendShortMsg(0x90, i, 0x00);
    }
};

DJControlInstinctP8.controls = {
    // "name" is just for reference in the code.
    "inputs": {
        0x30: { "name": "jog", "channel": 1, "group": "[Channel1]"},
        0x32: { "name": "jog", "channel": 2, "group": "[Channel2]"},
        0x31: { "name": "pitch", "channel": 1, "group": "[Channel1]"},
        0x33: { "name": "pitch", "channel": 2, "group": "[Channel2]"},
    }
};


DJControlInstinctP8.scratch = function(midino, control, value) {

    // Normal scratch function
    if (value) {
      if (scratchMode === 0) {
          // Enable the scratch mode on the corrisponding deck and start the timer
          scratchMode = 1;
          scratchTimer = engine.beginTimer(scratchResetTime, DJControlInstinctP8.wheelOnOff);
          midi.sendShortMsg(0x90, 45, 0x7F); // Switch-on the Scratch led
          engine.setValue("[Channel1]", "keylock", 0);
          engine.setValue("[Channel2]", "keylock", 0);
      } else {
          // Disable the scratch mode on the corrisponding deck and stop the timer
          scratchMode = 0;
          engine.stopTimer(scratchTimer);
          midi.sendShortMsg(0x90, 45, 0x00); // Switch-off the Scratch led (45=2D)
      }
    }
};

// This function is called every "scratchResetTime" seconds and checks if the wheel was moved in the previous interval
// (every interval last "scratchResetTime" seconds). If the wheel was moved enables the scratch mode, else disables it.
// In this way I have made a simple workaround to simulate the touch-sensitivity of the other controllers.
DJControlInstinctP8.wheelOnOff = function() {
    // Wheel Deck A / Channel 1
    if (wheelMove[0]) {
        engine.scratchEnable(1, 128, standardRpm, alpha, beta);
    } else {
	    engine.scratchDisable(1);
    }
	    wheelMove[0] = 0;
    

    //Wheel Deck B / Channel 2
    if (wheelMove[1]) {
        engine.scratchEnable(2, 128, standardRpm, alpha, beta);
    } else {
	    engine.scratchDisable(2);
    }
	    wheelMove[1] = 0;
    
};

DJControlInstinctP8.jogWheel = function (midino, control, value, status, group) {

    var deck = (group == "[Channel1]") ? deckA : deckB;

    // This function is called everytime the jog is moved

    var direction = 1;
    if (value !== 0x01) {
    	direction = -1;
    }

    if (scratchMode) {
    	engine.scratchTick(deck, direction);
    	wheelMove[deck - 1] = 1;
    } else {
    	engine.setValue("[Channel" + deck + "]", "jog", jogSensitivity * direction);
    }

};

DJControlInstinctP8.jogWheelHelper = function(n) {
    engine.scratchDisable(n);
    scratch_timer_on[n] = false;
};

// Pitch is adjusted by holding down shift and turning the jog wheel.
DJControlInstinctP8.pitch = function (group, control, value) {
    var input = DJControlInstinctP8.controls.inputs[control];

    // If the high bit is 1, convert to a negative number
    if (value & 0x40) {
        value = value - 0x80;
    }
    var delta = Math.pow(Math.abs(value), 2) / 1000.0;
    if (value < 0) {
        delta = -delta;
    }
    var pitch = engine.getValue(input.group, "rate") + delta;
	pitch = pitch/Math.abs(pitch); // normalize to 1 or -1
//	if (pitch > 1.0) {
//        pitch = 1.0;
//    }
//    if (pitch < -1.0) {
//        pitch = -1.0;
//    }
    engine.setValue(input.group, "rate", pitch);
};

DJControlInstinctP8.OnRecordingStatusChange = function(value) {
    // Not sure why this doesn't work with a regular midi output in the xml.
    if (value == 2) {
        midi.sendShortMsg(0x90, 0x2B, 0x7F);
        midi.sendShortMsg(0x90, 0x2C, 0x7F);
    } else {
        midi.sendShortMsg(0x90, 0x2B, 0x0);
        midi.sendShortMsg(0x90, 0x2C, 0x0);
    }
};


DJControlInstinctP8.loopHalveDouble = function (channel, control, value, status, group) {
    var deck = (group == "[Channel1]") ? deckA : deckB;
    var isLoopActive = engine.getValue("[Channel" + deck + "]", "loop_enabled");
    var jogValue = value - 0x40; // -64 to +63, - = CCW, + = CW

    if(isLoopActive) {
        if(jogValue > 0) {
            // Because loop_halve is supposed to be a pushbutton, we have to
            // fake the button-off event to clear out the "pressed" status.
            engine.setValue("[Channel" + deck + "]", "loop_halve", 1)
            engine.setValue("[Channel" + deck + "]", "loop_halve", 0)
        } else {
            engine.setValue("[Channel" + deck + "]", "loop_double", 1)
            engine.setValue("[Channel" + deck + "]", "loop_double", 0)
        }
    }
};

DJControlInstinctP8.LoopMove = function (channel, control, value, status, group) {
    var deck = (group == "[Channel1]") ? deckA : deckB;
    var isLoopActive = engine.getValue("[Channel" + deck + "]", "loop_enabled");
    var jogValue = value - 0x40; // -64 to +63, - = CCW, + = CW

    if(isLoopActive) {
        if(jogValue > 0) {
            engine.setValue("[Channel" + deck + "]", "loop_move", -1)
        } else {
            engine.setValue("[Channel" + deck + "]", "loop_move", 1)
        }
    }
};
