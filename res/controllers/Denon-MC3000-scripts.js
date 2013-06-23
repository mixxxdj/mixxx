/**
 * Denon MC3000 controller script for v1.10.0 and v1.11.0
 *
 * Written by Bertrand Espern 2012
 *
 * 2012/05/11 V0.995 : first "good" version approuved and tested by me
 *
 * Special Thanks to the Programmers of Mixx and all the contributors
 *
 * Inspired primarily from Numark Total Control script file 
 *
 * 4 deck support, but no loop and no cue function on deck 3/4. Later...
 *
 **/

function mc3000(){}

// ----------   Global variables    ----------

mc3000.deck2ch = [0,2,1,3];

mc3000.nearEnd = [0,0,0,0];

mc3000.oldloopstart = [0,0,0,0];

mc3000.loophotcue = [0,0,0,0];
mc3000.loophotcuebeatnb = [8,8,8,8];

mc3000.hotcues = {23: 1, 24: 2, 25: 3, 32: 4, 72:5, 73:6, 74:7, 75:8};

mc3000.leds = {vinylmode: 6, keylock: 8, sync: 9, cue:38, play: 39, fxon1: 90, fxon2: 91,
cue1: 17, cue2: 19, cue3: 21, cue4: 23, cue5: 48, cue6: 50, cue7: 52, cue8: 54,
loopin: 36, loopout: 64, autoloop: 43, pfl1: 69, pfl2: 81, pfl3: 75, pfl4: 87,
efx1_1: 92, efx2_1: 93, efx3_1: 94, efx4_1: 95, efx1_2: 96, efx2_2: 97, efx3_2: 98, efx4_2: 99,
autoloop_dimmer: 83};

mc3000.state = {"shift1" : false, "shift2" : false, "alshift1" : false, "alshift2" : false, "pfl1" : 0, "pfl2" : 0 }; 

mc3000.vumetermaxled = [0,0,0,0];
mc3000.vumeteroffset = [9,41,25,57];

mc3000.scratch =[true,true,true,true];
//mc3000.scratch =[false,false,false,false];

mc3000.scratchpressed = [false,false,false,false];
mc3000.alpha =  (1.0/8);
mc3000.beta = mc3000.alpha/32;
mc3000.jog_sensitivity = 2.0;
mc3000.scratch_sensitivity = 1;

// BeatLoop 2 4 8 16 to 1 2 3 4 
mc3000.efx2no = [4,8,16,2];

// ----------   Functions    ----------

// called when the MIDI device is opened & set up
mc3000.init = function(id) {	

	mc3000.id = id;

	var i=0;

	for (i=1;i<=8;i++) {
		engine.connectControl("[Channel1]","hotcue_"+i+"_enabled","mc3000.hotcueSetLed");
		engine.connectControl("[Channel2]","hotcue_"+i+"_enabled","mc3000.hotcueSetLed");
	}

	for (i=1;i<=4;i++) {
		engine.connectControl("[Channel"+i+"]", "keylock", "mc3000.keylockSetLed");
		engine.connectControl("[Channel"+i+"]", "play", "mc3000.playSetLed");
		engine.connectControl("[Channel"+i+"]", "playposition", "mc3000.playPositionSetLed");
		engine.connectControl("[Channel"+i+"]", "PeakIndicator", "mc3000.peakIndicatorSetLed");
		engine.connectControl("[Channel"+i+"]", "VuMeter", "mc3000.vuMeterSetLeds");
		engine.connectControl("[Channel"+i+"]", "pfl", "mc3000.pflSetLed");
	}

	for (i=1;i<=2;i++) {
		engine.connectControl("[Channel"+i+"]", "loop_start_position", "mc3000.loopStartSetLed");
		engine.connectControl("[Channel"+i+"]", "loop_end_position", "mc3000.loopEndSetLed");
		engine.connectControl("[Channel"+i+"]", "loop_enabled", "mc3000.loopEnableSetLed");

		engine.connectControl("[Channel"+i+"]", "beatloop_2_enabled", "mc3000.beatLoopXSetLed");
		engine.connectControl("[Channel"+i+"]", "beatloop_4_enabled", "mc3000.beatLoopXSetLed");
		engine.connectControl("[Channel"+i+"]", "beatloop_8_enabled", "mc3000.beatLoopXSetLed");
		engine.connectControl("[Channel"+i+"]", "beatloop_16_enabled", "mc3000.beatLoopXSetLed");
		// PERFORMANCE EATER ?
		//engine.connectControl("[Channel"+i+"]", "beat_active", "mc3000.beatActiveSetLed");
	}

	mc3000.allLedOff();
}

// Called when the MIDI device is closed
mc3000.shutdown = function(id) {
	mc3000.allLedOff();
}

// === MISC TO MANAGE LEDS ===

mc3000.allLedOff = function () {
	// All leds off for deck 1 and 2
	for (var led in mc3000.leds) {
		mc3000.setled(1,mc3000.leds[led],0);
		mc3000.setled(2,mc3000.leds[led],0);	
	}
	// Pfl leds off for deck 1 and 2 are special
	mc3000.setled2(1,mc3000.leds["pfl1"],0);
	mc3000.setled2(2,mc3000.leds["pfl2"],0);

	// Vinylmode ON
	var i=0;
	for (i=1;i<=4;i++) mc3000.setled(i,mc3000.leds["vinylmode"],1);
}

mc3000.setled = function(deck,led,status) {
	var ledStatus = 75; // Default OFF
	switch (status) {
    		case  0 : ledStatus = 75; break; // OFF
		case false : ledStatus = 75; break; // OFF 
		case true  : ledStatus = 74; break; // ON
    		case  1 : ledStatus = 74; break; // ON
    		case  2 : ledStatus = 76; break; // BLINK
	}
	midi.sendShortMsg(0xB0+mc3000.deck2ch[deck-1], ledStatus, led);
}

mc3000.setled2 = function(deck,led,status) {
	midi.sendShortMsg(0xB0+mc3000.deck2ch[deck-1], status==1 ? 80 : 81, led);
}

// === MISC COMMON ===

mc3000.groupToDeck = function(group) {
	var matches = group.match(/^\[Channel(\d+)\]$/);
	if (matches == null) {
		return -1;
	} else {
		return matches[1];
	}
}

mc3000.loop2NoEfx = function(nbloop) {
	if (nbloop==1) nbloop=16;
	return Math.log(nbloop)/Math.log(2); //2 4 8 16 -> 1 2 3 4	
}

mc3000.samplesPerBeat = function(group) {
	var sampleRate = engine.getValue(group, "track_samplerate");
	var channels = 2;
	var bpm = engine.getValue(group, "file_bpm");
	return channels * sampleRate * 60 / bpm;
}

// Pitch Rate MSB/LSB
mc3000.rate = function(channel, control, value, status, group) {
	// 0..16383 -> -1..1
	var MSB = value;
	var LSB = control;
	var num14 = (MSB << 7) | LSB;  // Construct the 14-bit number
	var nrate = (num14-8192)/8192;
	engine.setValue(group, "rate", nrate);
}

// === MASTER ===
mc3000.micVolume = function(channel, control, value, status, group) {
	engine.setValue("[Microphone]", "volume", value/127);
}
mc3000.headMix = function(channel, control, value, status, group) {
	engine.setValue("[Master]", "headMix", (value-64)/64);
}
mc3000.headVolume = function(channel, control, value, status, group) {
	engine.setValue("[Master]", "headVolume", value/127*5);
}
mc3000.masterBalance = function(channel, control, value, status, group) {
	engine.setValue("[Master]", "balance", (value-64)/64);
}
mc3000.masterVolume = function(channel, control, value, status, group) {
	engine.setValue("[Master]", "volume", value/127*5);
}

// === PLAYLIST ===
mc3000.selectKnob = function(channel, control, value, status, group) {
	var offset = value==0 ? 250:-250;

	// AUTOLOOP + SelectKnob ? -> MOVE LOOP
	if (mc3000.state["alshift1"] || mc3000.state["alshift2"]){
		if (mc3000.state["alshift1"]) mc3000.moveLoop("[Channel1]",offset);
		if (mc3000.state["alshift2"]) mc3000.moveLoop("[Channel2]",offset);
		return;
	}
	
	// SHIFT + SelectKnob ? -> MOVE CUE
	if (mc3000.state["shift1"] || mc3000.state["shift2"]) {
		if (mc3000.state["shift1"]) {
			var curpos = engine.getValue("[Channel1]","cue_point");
	 		engine.setValue("[Channel1]","cue_point",curpos+offset);
		}
		if (mc3000.state["shift2"]) {
			var curpos = engine.getValue("[Channel2]","cue_point");
	 		engine.setValue("[Channel2]","cue_point",curpos+offset);
		}
		return;
	} 

	// NORMAL MODE - NEXT/PREV TRACK
	if (value == 0) { 
		engine.setValue(group, "SelectNextTrack", 1);
	} else {
		engine.setValue(group, "SelectPrevTrack", 1);
	}
}

// === KILL ===
mc3000.killMid = function(channel, control, value, status, group) {
	engine.setValue(group, "filterMidKill", !engine.getValue(group, "filterMidKill"));
}

// BT USED FOR BASS IS USED FOR HIGH if SHIFT
mc3000.killBass = function(channel, control, value, status, group) {
	if (mc3000.state["shift1"] || mc3000.state["shift2"]) {
		engine.setValue(group, "filterHighKill", !engine.getValue(group, "filterHighKill"));
	}else{
		engine.setValue(group, "filterLowKill", !engine.getValue(group, "filterLowKill"));
	}
}
// === FLANGER ===
// Flanger knob Depth, Delay, Period
mc3000.lfoDepth = function(channel, control, value, status, group) {
// 0..127 -> 0..1
	engine.setValue(group, "lfoDepth", value/127);
}
mc3000.lfoDelay = function(channel, control, value, status, group) {
// 0..127 -> 50..10000
	engine.setValue(group, "lfoDelay", 50+(value/127)*9950);
}
mc3000.lfoPeriod = function(channel, control, value, status, group) {
// 0..127 -> 50000..2000000
	engine.setValue(group, "lfoPeriod", 50000+(value/127)*1950000);
}

// === CUE AND HOTCUE ===
mc3000.cue_default = function(channel, control, value, status, group) {
	var isPressed = (status & 0xF0)==0x90 ? 1 : 0;
	print("cue_def - status = "+status+" group = "+group); 
	engine.setValue(group,"cue_default", isPressed);
	mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["cue"],isPressed);
}


mc3000.hotcueset = function(channel, control, value, status, group) {
	var deck=mc3000.groupToDeck(group);
	var nocue=mc3000.hotcues[control];
	var is_play = engine.getValue(group,"play")==1 ? true: false;	

	if ((status & 0xF0) !== 0x90) {    // Is Relesead ?
	 	if (mc3000.state["alshift"+deck] || mc3000.state["shift"+deck]) return; // DO NOTHING ON RELEASE IF ALSHIFT OR SHIFT
		engine.setValue(group,"hotcue_"+nocue+"_activate",0);
		return;
	}

	// AUTOLOOP + HOTCUE MODE ?
	if (mc3000.state["alshift"+deck]) {
		if (engine.getValue(group,"hotcue_"+nocue+"_enabled")) {
			mc3000.setLoopAtHotCue(group,nocue,mc3000.loophotcuebeatnb[deck-1]);
			if (!is_play) engine.setValue(group, "reloop_exit", 1);
			// TO AVOID DIRECT LOOP AFTER RELEASED AUTOLOOP
			mc3000.oldloopstart[deck-1] = engine.getValue(group, "loop_start_position");
		}
		return;
	}

	// WITH SHIFT ?
	if (mc3000.state["shift"+deck]) {
        	engine.setValue(group,"hotcue_"+nocue+"_clear",1);
		return;
	}

	if (is_play) {
		// LOOP HOTCUE OR NORMAL HOTCUE ?
		if (mc3000.loophotcue[deck-1]==nocue) {
  			mc3000.loophotcue[deck-1]=0; 
			engine.setValue(group, "reloop_exit", 1);
		} else {
			engine.setValue(group,"hotcue_"+nocue+"_activate",1);
		}
	} else {
			engine.setValue(group,"hotcue_"+nocue+"_activate",1);
	}
}

// ==== Set a loop at the hotcue_X_position
mc3000.setLoopAtHotCue = function (group,nohotcue,nbbeat) {
	var position = engine.getValue(group,"hotcue_"+nohotcue+"_position");
	var deck = mc3000.groupToDeck(group);

	// Old HotCue Led Off if exist
	if (mc3000.loophotcue[deck-1]>0) mc3000.hotcueSetLed(0,group,"hotcue_"+mc3000.loophotcue[deck-1]);

	// if loop set, exit from loop	
	if (engine.getValue(group, "loop_enabled")) engine.setValue(group, "reloop_exit", 1);

	// Set new loop_start
	engine.setValue(group, "loop_start_position", position);
	// Delete loop_end	
	engine.setValue(group, "loop_end_position", -1);
	// Set Loop End
	engine.setValue(group, "loop_end_position", position+(mc3000.samplesPerBeat(group)*nbbeat));
	
	mc3000.loophotcue[deck-1]=nohotcue;	
	
	// New HotCue Led Blink
	mc3000.hotcueSetLed (2,group,"hotcue_"+nohotcue);
}

mc3000.shift = function(channel, control, value, status, group) {
	mc3000.state["shift"+mc3000.groupToDeck(group)] = (status & 0xF0)==0x90 ? true : false; // 1 if pressed
}

mc3000.vinylmode = function(channel, control, value, status, group) {
	var deck = mc3000.groupToDeck(group);
	mc3000.scratch[deck-1] = !mc3000.scratch[deck-1];
}

// FX ON 1 to toggle flanger ON/OFF
mc3000.fxon1 = function(channel, control, value, status, group) {
	var newValue = !engine.getValue(group, "flanger");
	engine.setValue(group, "flanger", newValue);
	mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["fxon1"],newValue);
}

// FX ON 2 to toggle quantize / or grid adjust if shift
mc3000.fxon2 = function(channel, control, value, status, group) {
	if (mc3000.state["shift"+mc3000.groupToDeck(group)]) {
		engine.setValue(group, "beats_translate_curpos",true);
	}else{
		var newValue = !engine.getValue(group, "quantize");
		engine.setValue(group, "quantize", newValue);
		mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["fxon2"],newValue);
	}
}

// JOG with WHEELTOUCH
mc3000.wheelTouch = function (channel, control, value, status, group) {
	var deck = mc3000.groupToDeck(group);
	if (mc3000.scratch[deck-1]) {
		if ((status & 0xF0)==0x90) {    // Is pressed ?
			engine.scratchEnable(deck, 100, 330, mc3000.alpha, mc3000.beta); // 128, 33+1/3 aussi
		} else { 
			engine.scratchDisable(deck);
		}
		mc3000.scratchpressed[deck-1] = (status & 0xF0)==0x90 ? true : false;
	}
}
 
// The wheel that actually controls the scratching
mc3000.jogWheel = function (channel, control, value, status, group) {
	var deck = mc3000.groupToDeck(group);
	var adjustedJog = value - 64; // Control centers on 0x40 (64)

	if (mc3000.scratch[deck-1]) {
		if (mc3000.scratchpressed[deck-1]) {
			engine.scratchTick(deck,adjustedJog/mc3000.scratch_sensitivity);
			return;
		}
	}
	
	var gammaInputRange = 64;	// Max jog speed
	var maxOutFraction = 0.5;	// Where on the curve it should peak; 0.5 is half-way
	var sensitivity = 0.5;		// Adjustment gamma 0.5 def
	var gammaOutputRange = 3;	// Max rate change
	adjustedJog = gammaOutputRange * adjustedJog / (gammaInputRange * maxOutFraction);
	engine.setValue(group, "jog", adjustedJog/mc3000.jog_sensitivity);
}

// RATE TEMP UP if PLAY else FWD / END
mc3000.bendUp = function(channel, control, value, status, group) {
	var isPressed = (status & 0xF0)==0x90 ? 1 : 0;
	if (engine.getValue(group,"play") == 1) {
		engine.setValue(group, "rate_temp_up", isPressed);		
	}else{
		if ((mc3000.state["shift"+mc3000.groupToDeck(group)]) && isPressed) {
			engine.setValue(group,"end",1);
		} else {
			engine.setValue(group, "fwd",isPressed);
		}
	}
}

// RATE TEMP DOWN if PLAY else BACK / START
mc3000.bendDown = function(channel, control, value, status, group) {
	var isPressed = (status & 0xF0)==0x90 ? 1 : 0;

	if (engine.getValue(group,"play") == 1) {
		engine.setValue(group, "rate_temp_down", isPressed);
	}else{
		if ((mc3000.state["shift"+mc3000.groupToDeck(group)]) && isPressed) {
			engine.setValue(group,"start",1);
		} else {
			engine.setValue(group, "back", isPressed);
		}
	}
}

// === LOOP CONTROL ===

mc3000.efx_beatLoopX = function(channel, control, value, status, group) {
	var deck = mc3000.groupToDeck(group);
	var nbbeat = mc3000.efx2no[(control-18)-((deck-1)*64)];

	if (mc3000.state["alshift"+deck]) {	
		mc3000.setled(deck,mc3000.leds["efx"+mc3000.loop2NoEfx(mc3000.loophotcuebeatnb[deck-1])+"_"+deck],0);
		mc3000.setled(deck,mc3000.leds["efx"+mc3000.loop2NoEfx(nbbeat)+"_"+deck],2);
		mc3000.loophotcuebeatnb[deck-1]=nbbeat;
	} else {
		engine.setValue(group, "beatloop_"+nbbeat+"_activate",1);
	}

}	

// Shift and autoloop : delete loop point ELSE BeatLoopAtHotCue
mc3000.autoLoop = function(channel, control, value, status, group) {
	var deck = mc3000.groupToDeck(group);
	var i=0;	
	if ((status & 0xF0)==0x90) {
		// LIGHT LED EFX + CUE  EN MODE CUE LOOP
		// EFX = BEATLOOP NB
		for (i=1;i<=4;i++) mc3000.setled(deck,mc3000.leds["efx"+i+"_"+deck],0);
		mc3000.setled(deck,mc3000.leds["efx"+mc3000.loop2NoEfx(mc3000.loophotcuebeatnb[deck-1])+"_"+deck],2);
		
		for (i=1;i<=8;i++) mc3000.hotcueSetLed(0,group,"hotcue_"+i);
		if (mc3000.loophotcue[deck-1] > 0) mc3000.hotcueSetLed(2,group,"hotcue_"+mc3000.loophotcue[deck-1]);
		print ("hotcue_"+mc3000.loophotcue[deck-1]);
		
		// DELETE LOOP POINTS IF SHIFT
		if (mc3000.state["shift"+deck]) {
			engine.setValue(group, "reloop_exit", 1);
			engine.setValue(group, "loop_end_position", -1);
			engine.setValue(group, "loop_start_position", -1);
		}
		// INIT TO PREPARE MOVELOOP IF NEEDED
		mc3000.oldloopstart[deck-1] = engine.getValue(group, "loop_start_position");

	} else { //released
		// RESTORE LED STATE EFX + CUE_X
		var beatp=0;
		for (i=1;i<=4;i++) {
			 beatp=Math.pow(2,i);
			 mc3000.setled(deck,mc3000.leds["efx"+i+"_"+deck],engine.getValue(group,"beatloop_"+beatp+"_enabled"));
		}
		for (i=1;i<=8;i++) {
			if (engine.getValue(group,"hotcue_"+i+"_enabled")) { 
				mc3000.hotcueSetLed(1,group,"hotcue_"+i);
			} else {
				mc3000.hotcueSetLed(0,group,"hotcue_"+i);
			}
		}

		// IF MOVE LOOP APPLIED
		if (mc3000.oldloopstart[deck-1] != engine.getValue(group, "loop_start_position")) {
			engine.setValue(group, "reloop_exit", 1);
		}
				
	}

	mc3000.state["alshift"+deck] = (status & 0xF0)==0x90 ? true : false ; // true if pressed
}

mc3000.loopCutM = function(channel, control, value, status, group) {
	var start = engine.getValue(group, "loop_start_position");
	var end = engine.getValue(group, "loop_end_position");
	if ((start != -1) && (end != -1)) engine.setValue(group, "loop_halve", 1);
}

mc3000.loopCutP = function(channel, control, value, status, group) {
	var start = engine.getValue(group, "loop_start_position");
	var end = engine.getValue(group, "loop_end_position");
	if ((start != -1) && (end != -1)) engine.setValue(group, "loop_double", 1);
}

mc3000.loopIn = function(channel, control, value, status, group) {
	if (engine.getValue(group, "loop_enabled")) engine.setValue(group, "reloop_exit", 1);
	engine.setValue(group, "loop_in", 1);
	engine.setValue(group, "loop_end_position", -1);
}

mc3000.loopOut = function(channel, control, value, status, group) {
	var start = engine.getValue(group, "loop_start_position");
	var end = engine.getValue(group, "loop_end_position");
	if (start != -1) {
		if (end != -1) {
			engine.setValue(group, "reloop_exit", 1);
		} else {
			engine.setValue(group, "loop_out", 1);
		}
	}
}


// === EXPERIMENTAL ===
// CF MODE for test 
mc3000.cfMode = function(channel, control, value, status, group) {
	// BACKLOOP de -4 ?
	print ("BackLoop ?");
	engine.setValue("[Channel1]", "beatloop_4", 1);
}
mc3000.moveLoop = function (group,offset) {
		var start = engine.getValue(group, "loop_start_position");
		var end = engine.getValue(group, "loop_end_position");

		if ((start != -1) && (end != -1)) {
			engine.setValue(group, "loop_start_position", start+offset);
			engine.setValue(group, "loop_end_position", end+offset);
		}
}

// === SET LED FUNCTIONS ===

mc3000.hotcueSetLed = function(value, group, control) {
	mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["cue"+control[7]],value);
}

mc3000.pflSetLed = function(value,group) {
	mc3000.setled2(mc3000.groupToDeck(group),mc3000.leds["pfl"+mc3000.groupToDeck(group)],value);
}

mc3000.playSetLed = function(value,group) {
	mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["play"],value);
}

mc3000.keylockSetLed = function(value,group) {
	mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["keylock"],value);
}

mc3000.loopStartSetLed = function (value, group) {
	mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["loopin"],value == -1 ? false: true);
}

mc3000.loopEndSetLed = function (value, group) {
	mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["loopout"],value == -1 ? false: true);
}

mc3000.loopEnableSetLed = function(value, group, control) {
	mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["autoloop"],value);
}

mc3000.beatLoopXSetLed = function(value, group, control) {
	var deck = mc3000.groupToDeck(group);	
	mc3000.setled(deck,mc3000.leds["efx"+mc3000.loop2NoEfx(control[9])+"_"+deck],value);
}

mc3000.beatActiveSetLed = function (value,group){
	mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["sync"],value);
}

mc3000.peakIndicatorSetLed = function (value, group) {
	mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["sync"],value);
}

mc3000.vuMeterSetLeds = function (value,group){
	var deck = mc3000.groupToDeck(group);
	var maxled = Math.round ((value-0.07)*7);
	var curled = mc3000.vumetermaxled[deck-1];
	if (maxled !== curled) {
		var i=0; 
		var offset = mc3000.vumeteroffset[deck-1];
		if (maxled > curled) {
			for (i=curled+1;i<=maxled;i++) midi.sendShortMsg(0xB0+mc3000.deck2ch[deck-1], 80, i+offset);
		}else{
			for (i=maxled+1;i<=curled;i++) midi.sendShortMsg(0xB0+mc3000.deck2ch[deck-1], 81, i+offset);
		}
		mc3000.vumetermaxled[deck-1] = maxled;
	}		 
}

mc3000.playPositionSetLed = function (playposition, group) {
	var nearEnd = 0;

	if (playposition < 0.80) nearEnd = 0;
	if (playposition > 0.80) nearEnd = 2;  // The song is going to end
	if (playposition == 1)   nearEnd = 0;	

	if (nearEnd !== mc3000.nearEnd[mc3000.groupToDeck(group)-1]) {
		mc3000.setled(mc3000.groupToDeck(group),mc3000.leds["sync"],nearEnd);
		mc3000.nearEnd[mc3000.groupToDeck(group)-1] = nearEnd;
	}
}


