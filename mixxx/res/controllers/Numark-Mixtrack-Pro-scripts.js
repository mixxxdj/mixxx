// Based on:
// Numark Mixtrack Mapping Script Functions
// 1/11/2010 - v0.1
// Matteo <matteo@magm3.com>
//
// Changes by James Ralston
//  5/18/2011
// 
// Changes:
//	Remapped frequency filter keys (on regular Mixtrack) to hotcue keys 1,2,3 (on Mixtrack Pro)
//	Mapped delete keys to clear hotque and loop (when pressing delete, then reloop)
//	Changed jog wheel behavior when in scratch mode
//	Added keylock (disabled on wheel touch when in scratch mode)
//	Reworked the looping.  Lights flash on errors (i.e. resize loop when no begin or end set).
//	Reworked jog wheel behavior (stop on wheel touch in scratch mode)
//
// Known Bugs:
//	Mixxx complains about an undefined variable on 1st load of the mapping (ignore it, then restart Mixxx)
//	Each slide/knob needs to be moved on Mixxx startup to match levels with the Mixxx UI
//
//

// Changes by Darío José Freije
// 05/26/2012
//
// Cue:
//	The CUE button is always with light on (for darkest places). It blink on Press. 
//	Added: Don't set Cue accidentaly at the end of the song (return to the lastest cue)
//
// Jog Wheels:
//	Modify sensibility of the wheels in Scratch mode (lower now).
//	In Scratch mode: with the Touch surface of the wheels: Scratch. 
//	With the edge of jog wheel: tempo Bend (like in Not Scratch mode)
//
// Stutter -> Adjust BeatGrid:
//	The Stutter Button work now to set the beatgrid in the correct place (usefull to sync well)
//
// Gain:
//	The 3rd knob of the "effect" section, is now "Gain" (pre-gain, very usefull for me)
//
// Pitch: Up, Up; Down, Down.
//	The Pitch slide are now inverted, to match with the screen. 
// 	Up in the controller -> Up in the Screen (+) (otherwise is very confusing to me)
//
// Auto Loop:
// 	I'm very bad with Manual Loop, always use Auto Loop, so now the light of the button Manual is ON in the Auto mode.
//	In auto mode, now the "1 Bar" button active an instant 4 beat Loop and turn On the Light. 
//	The 1/2 and x2 buttons works like before. To exit the Loop switch to "Manual" mode and press "Reloop" (like before).
//

// More Changes:
// 06/02/2012
//
// Pitch Sliders:
//	Moved to script for can have the Soft-takeover functionality
// 	"To prevent sudden wide parameter changes when the on-screen control diverges from a hardware control, use soft-takeover. 
//	While it's active on a particular parameter, manipulating the control on the hardware will have no effect until the position
//	of the hardware control is close to that of the software, at which point it will take over and operate as usual."	
//
// Peak Indicator:
//	 in the light of SYNC button
//
// Cue:
//	Blink only on Press (not in release)
//


function NumarkMixTrackPro() {}

NumarkMixTrackPro.init = function(id) {	// called when the MIDI device is opened & set up
	NumarkMixTrackPro.id = id;	// Store the ID of this device for later use
	
	NumarkMixTrackPro.directoryMode = false;
	
	NumarkMixTrackPro.scratchMode = [false, false];
	
	NumarkMixTrackPro.manualLooping = [true, true];
	NumarkMixTrackPro.deleteKey = [false, false];
	NumarkMixTrackPro.isKeyLocked = [0, 0];
	
	NumarkMixTrackPro.leds = [
		// Common
		{ "directory": 0x73, "file": 0x72 },
		// Deck 1
		{ "rate": 0x70, "scratchMode": 0x48, "manualLoop": 0x61, 
		"loop_start_position": 0x53, "loop_end_position": 0x54, "reloop_exit": 0x55,
		"deleteKey" : 0x59, "hotCue1" : 0x5a,"hotCue2" : 0x5b,"hotCue3" :  0x5c,
		"stutter" : 0x4a, "Cue" : 0x33, "sync" : 0x40 
		},
		// Deck 2
		{ "rate": 0x71, "scratchMode": 0x50, "manualLoop": 0x62, 
		"loop_start_position": 0x56, "loop_end_position": 0x57, "reloop_exit": 0x58,
		"deleteKey" : 0x5d, "hotCue1" : 0x5e, "hotCue2" : 0x5f, "hotCue3" :  0x60,
		"stutter" : 0x4c, "Cue" : 0x3c, "sync" : 0x47 
		 }
	];
	
	NumarkMixTrackPro.ledTimers = {};

	NumarkMixTrackPro.LedTimer = function(id, led, count, state){
		this.id = id;
		this.led = led;
		this.count = count;
		this.state = state;
	}

	NumarkMixTrackPro.hotCue = {
			//Deck 1 
			0x5a:"1", 0x5b:"2", 0x5c:"3",
			//Deck 2
			0x5e: "1", 0x5f:"2", 0x60:"3"
			};

	//Add event listeners
	for (var i=1; i<3; i++){
		for (var x=1; x<4; x++){
			engine.connectControl("[Channel" + i +"]", "hotcue_"+ x +"_enabled", "NumarkMixTrackPro.onHotCueChange");
		}
		NumarkMixTrackPro.setLoopMode(i, true);
	}

	NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[0]["file"], true);

	NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[1]["Cue"], true);
	NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[2]["Cue"], true);

// Enable soft-takeover for Pitch slider

	engine.softTakeover("[Channel1]", "rate", true);
	engine.softTakeover("[Channel2]", "rate", true);


// Clipping LED
	engine.connectControl("[Channel1]","PeakIndicator","NumarkMixTrackPro.Channel1Clip");
	engine.connectControl("[Channel2]","PeakIndicator","NumarkMixTrackPro.Channel2Clip");

}

NumarkMixTrackPro.Channel1Clip = function (value) {
	NumarkMixTrackPro.clipLED(value,NumarkMixTrackPro.leds[1]["sync"]);

}

NumarkMixTrackPro.Channel2Clip = function (value) {
	NumarkMixTrackPro.clipLED(value,NumarkMixTrackPro.leds[2]["sync"]);

}


NumarkMixTrackPro.clipLED = function (value, note) {

	if (value>0) NumarkMixTrackPro.flashLED(note, 1); 

}

NumarkMixTrackPro.shutdown = function(id) {	// called when the MIDI device is closed

	// First Remove event listeners
	for (var i=1; i<2; i++){
		for (var x=1; x<4; x++){
			engine.connectControl("[Channel" + i +"]", "hotcue_"+ x +"_enabled", "NumarkMixTrackPro.onHotCueChange", true);
		}	
		NumarkMixTrackPro.setLoopMode(i, false);
	}

	var lowestLED = 0x30;
	var highestLED = 0x73;
	for (var i=lowestLED; i<=highestLED; i++) {
		NumarkMixTrackPro.setLED(i, false);	// Turn off all the lights
	}

}

NumarkMixTrackPro.groupToDeck = function(group) {
	var matches = group.match(/^\[Channel(\d+)\]$/);
	if (matches == null) {
		return -1;
	} else {
		return matches[1];
	}
}

NumarkMixTrackPro.samplesPerBeat = function(group) {
	// FIXME: Get correct samplerate and channels for current deck
	var sampleRate = 44100;
	var channels = 2;
	var bpm = engine.getValue(group, "file_bpm");
	return channels * sampleRate * 60 / bpm;
}

NumarkMixTrackPro.setLED = function(value, status) {
	status = status ? 0x64 : 0x00;
	midi.sendShortMsg(0x90, value, status);
}

NumarkMixTrackPro.flashLED = function (led, veces){	
	var ndx = Math.random();
	var func = "NumarkMixTrackPro.doFlash(" + ndx + ", " + veces + ")";
	var id = engine.beginTimer(120, func);
	NumarkMixTrackPro.ledTimers[ndx] =  new NumarkMixTrackPro.LedTimer(id, led, 0, false);
}

NumarkMixTrackPro.doFlash = function(ndx, veces){
	var ledTimer = NumarkMixTrackPro.ledTimers[ndx];
	
	if (!ledTimer) return;
	
	if (ledTimer.count > veces){ // how many times blink the button
		engine.stopTimer(ledTimer.id);
		delete NumarkMixTrackPro.ledTimers[ndx];
	} else{
		ledTimer.count++;
		ledTimer.state = !ledTimer.state;
		NumarkMixTrackPro.setLED(ledTimer.led, ledTimer.state);
	}
}

NumarkMixTrackPro.selectKnob = function(channel, control, value, status, group) {
	if (value > 63) {
		value = value - 128;
	}
	if (NumarkMixTrackPro.directoryMode) {
		if (value > 0) {
			for (var i = 0; i < value; i++) {
				engine.setValue(group, "SelectNextPlaylist", 1);
			}
		} else {
			for (var i = 0; i < -value; i++) {
				engine.setValue(group, "SelectPrevPlaylist", 1);
			}
		}
	} else {
		engine.setValue(group, "SelectTrackKnob", value);

	}
}

NumarkMixTrackPro.cuebutton = function(channel, control, value, status, group) {


	// Don't set Cue accidentaly at the end of the song
	if(engine.getValue(group, "playposition") <= 0.97) 
	{
		engine.setValue(group, "cue_default", value ? 1 : 0);
		var deck = NumarkMixTrackPro.groupToDeck(group);
			if (value) NumarkMixTrackPro.flashLED(NumarkMixTrackPro.leds[deck]["Cue"], 4);
	}
	else
	{
		engine.setValue(group, "cue_preview", value ? 1 : 0);
	}


}

NumarkMixTrackPro.loopIn = function(channel, control, value, status, group) {
	var deck = NumarkMixTrackPro.groupToDeck(group);
	
	if (NumarkMixTrackPro.manualLooping[deck-1]){
		if (!value) return;
		// Act like the Mixxx UI
		engine.setValue(group, "loop_in", status?1:0);
		return;
	} 
	
	// Auto Loop: 1/2 loop size
	var start = engine.getValue(group, "loop_start_position");
	var end = engine.getValue(group, "loop_end_position");
	if (start<0 || end<0) {
		NumarkMixTrackPro.flashLED(NumarkMixTrackPro.leds[deck]["loop_start_position"], 4);
		return;
	}

	if (value){
		var start = engine.getValue(group, "loop_start_position");
		var end = engine.getValue(group, "loop_end_position");
		var len = (end - start) / 2;
		engine.setValue(group, "loop_end_position", start + len);
		NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_start_position"], true);  
	} else {
		NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_start_position"], false);
	}
}

NumarkMixTrackPro.loopOut = function(channel, control, value, status, group) {
	var deck = NumarkMixTrackPro.groupToDeck(group);
	
	if (!value) return;
	
	if (NumarkMixTrackPro.manualLooping[deck-1]){
		// Act like the Mixxx UI
		engine.setValue(group, "loop_out", status?1:0);
		return;
	}
		
	// Set a 4 beat auto loop
	engine.setValue(group,"beatloop_4_activate",1);
	NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_end_position"], true);

}

NumarkMixTrackPro.repositionHack = function(group, oldPosition){
	// see if the value has been updated
	if (engine.getValue(group, "loop_start_position")==oldPosition){
		if (NumarkMixTrackPro.hackCount[group]++ < 9){
			engine.beginTimer(20, "NumarkMixTrackPro.repositionHack('" + group + "', " + oldPosition + ")", true);
		} else {			
			var deck = NumarkMixTrackPro.groupToDeck(group);
			NumarkMixTrackPro.flashLED(NumarkMixTrackPro.leds[deck]["loop_start_position"], 4);
		}
		return;
	}
	var bar = NumarkMixTrackPro.samplesPerBeat(group);
	var start = engine.getValue(group, "loop_start_position");
	engine.setValue(group,"loop_end_position", start + bar);
}

NumarkMixTrackPro.reLoop = function(channel, control, value, status, group) {
	var deck = NumarkMixTrackPro.groupToDeck(group);
	
	if (NumarkMixTrackPro.manualLooping[deck-1]){
		// Act like the Mixxx UI (except for working delete)
		if (!value) return;
		if (NumarkMixTrackPro.deleteKey[deck-1]){
			engine.setValue(group, "reloop_exit", 0);
			engine.setValue(group, "loop_start_position", -1);
			engine.setValue(group, "loop_end_position", -1);
			NumarkMixTrackPro.toggleDeleteKey(channel, control, value, status, group);
		} else {
			engine.setValue(group, "reloop_exit", status?1:0);
		}
		return;
	}
	
	// Auto Loop: Double Loop Size
	var start = engine.getValue(group, "loop_start_position");
	var end = engine.getValue(group, "loop_end_position");
	if (start<0 || end<0) {
		NumarkMixTrackPro.flashLED(NumarkMixTrackPro.leds[deck]["reloop_exit"], 4);
		return;
	}

	if (value){
		var len = (end - start) * 2;
		engine.setValue(group, "loop_end_position", start + len);
		NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["reloop_exit"], true);
	} else {
		NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["reloop_exit"], false);
	}
}

NumarkMixTrackPro.setLoopMode = function (deck, manual){
	NumarkMixTrackPro.manualLooping[deck-1] = manual;
	NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["manualLoop"], !manual);
	engine.connectControl("[Channel" + deck + "]", "loop_start_position", "NumarkMixTrackPro.onLoopChange", !manual);
	engine.connectControl("[Channel" + deck + "]", "loop_end_position", "NumarkMixTrackPro.onLoopChange", !manual);
	engine.connectControl("[Channel" + deck + "]", "loop_enabled", "NumarkMixTrackPro.onReloopExitChange", !manual);
	
	var group = "[Channel" + deck + "]"
	if (manual){
		NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_start_position"], engine.getValue(group, "loop_start_position")>-1);
		NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_end_position"], engine.getValue(group, "loop_end_position")>-1);
		NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["reloop_exit"], engine.getValue(group, "loop_enabled"));
	}else{
		NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_start_position"], false);

		if (engine.getValue(group,"loop_enabled")){
			NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_end_position"], true);
		}else{
			NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["loop_end_position"], false);
			}

		NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["reloop_exit"], false);		
	}
}

NumarkMixTrackPro.toggleManualLooping = function(channel, control, value, status, group) {
	if (!value) return;
	
	var deck = NumarkMixTrackPro.groupToDeck(group);
	
	NumarkMixTrackPro.setLoopMode(deck, !NumarkMixTrackPro.manualLooping[deck-1]);
}

NumarkMixTrackPro.onLoopChange = function(value, group, key){
	var deck = NumarkMixTrackPro.groupToDeck(group);
	NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck][key], value>-1? true : false);
}

NumarkMixTrackPro.onReloopExitChange = function(value, group, key){
	var deck = NumarkMixTrackPro.groupToDeck(group);
	NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]['reloop_exit'], value);
}

// Stutters adjust BeatGrid
NumarkMixTrackPro.playFromCue = function(channel, control, value, status, group) {

var deck = NumarkMixTrackPro.groupToDeck(group);

if (engine.getValue(group, "beats_translate_curpos")){


	engine.setValue(group, "beats_translate_curpos", 0);
	NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["stutter"], 0);
	}else{
	engine.setValue(group, "beats_translate_curpos", 1);
	NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["stutter"], 1);
	}


}

NumarkMixTrackPro.pitch = function(channel, control, value, status, group) {
	var deck = NumarkMixTrackPro.groupToDeck(group);

	var pitch_value = 0;

	if (value < 64) pitch_value = (value-64) /64;
	if (value > 64) pitch_value = (value-64) /63;

	engine.setValue("[Channel"+deck+"]","rate",pitch_value);
}


NumarkMixTrackPro.jogWheel = function(channel, control, value, status, group) {
	var deck = NumarkMixTrackPro.groupToDeck(group);
	var adjustedJog = parseFloat(value);
	var posNeg = 1;
	if (adjustedJog > 63) {	// Counter-clockwise
		posNeg = -1;
		adjustedJog = value - 128;
	}



		engine.scratchTick(deck, adjustedJog);

		var gammaInputRange = 23;	// Max jog speed
		var maxOutFraction = 0.5;	// Where on the curve it should peak; 0.5 is half-way
		var sensitivity = 0.7;		// Adjustment gamma
		var gammaOutputRange = 3;	// Max rate change
		if (engine.getValue(group,"play")) {
			adjustedJog = posNeg * gammaOutputRange * Math.pow(Math.abs(adjustedJog) / (gammaInputRange * maxOutFraction), sensitivity);
		} else {
			adjustedJog = gammaOutputRange * adjustedJog / (gammaInputRange * maxOutFraction);
		}
		engine.setValue(group, "jog", adjustedJog);

}

NumarkMixTrackPro.wheelTouch = function(channel, control, value, status, group){
	var deck = NumarkMixTrackPro.groupToDeck(group);

	if (!NumarkMixTrackPro.scratchMode[deck-1]) return;

	if(!value){
		engine.scratchDisable(deck);
		// Restore the previous state
		engine.setValue(group, "keylock", NumarkMixTrackPro.isKeyLocked[deck-1]);
		NumarkMixTrackPro.isKeyLocked[deck-1] = 0;
	} else {
		// Save the current state of the keylock
		NumarkMixTrackPro.isKeyLocked[deck-1] = engine.getValue(group, "keylock");
		// Turn it off for scratching
		if (NumarkMixTrackPro.isKeyLocked[deck-1]){
			engine.setValue(group, "keylock", 0);
		}

		// change the 600 value for sensibility
		engine.scratchEnable(deck, 600, 33+1/3, 1.0/8, (1.0/8)/32);
	}
}

NumarkMixTrackPro.toggleDirectoryMode = function(channel, control, value, status, group) {
	// Toggle setting and light
	if (value) {
		NumarkMixTrackPro.directoryMode = !NumarkMixTrackPro.directoryMode;

		NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[0]["directory"], NumarkMixTrackPro.directoryMode);
		NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[0]["file"], !NumarkMixTrackPro.directoryMode);
	}
}

NumarkMixTrackPro.toggleScratchMode = function(channel, control, value, status, group) {
	if (!value) return;
	
	var deck = NumarkMixTrackPro.groupToDeck(group);
	// Toggle setting and light
	NumarkMixTrackPro.scratchMode[deck-1] = !NumarkMixTrackPro.scratchMode[deck-1];
	NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["scratchMode"], NumarkMixTrackPro.scratchMode[deck-1]);
}


NumarkMixTrackPro.onHotCueChange = function(value, group, key){
	var deck = NumarkMixTrackPro.groupToDeck(group);
	var hotCueNum = key[7];
	NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["hotCue" + hotCueNum], value ? true : false);
}

NumarkMixTrackPro.changeHotCue = function(channel, control, value, status, group){

	var deck = NumarkMixTrackPro.groupToDeck(group);
	var hotCue = NumarkMixTrackPro.hotCue[control];

	// onHotCueChange called automatically
	if(NumarkMixTrackPro.deleteKey[deck-1]){
		if (engine.getValue(group, "hotcue_" + hotCue + "_enabled")){
			engine.setValue(group, "hotcue_" + hotCue + "_clear", 1);
		}
		NumarkMixTrackPro.toggleDeleteKey(channel, control, value, status, group);
	} else {
		if (value) {
			engine.setValue(group, "hotcue_" + hotCue + "_activate", 1);

		}else{

			engine.setValue(group, "hotcue_" + hotCue + "_activate", 0);
		}
	}
}


NumarkMixTrackPro.toggleDeleteKey = function(channel, control, value, status, group){
	if (!value) return;

	var deck = NumarkMixTrackPro.groupToDeck(group);
	NumarkMixTrackPro.deleteKey[deck-1] = !NumarkMixTrackPro.deleteKey[deck-1]; 
	NumarkMixTrackPro.setLED(NumarkMixTrackPro.leds[deck]["deleteKey"], NumarkMixTrackPro.deleteKey[deck-1]);
}


