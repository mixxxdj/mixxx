// Script for custom Vestax Typhoon Mapping by bestani
// A few parts are taken from Bill Good's Vestax Typhoon Mapping

//Maybe I'll add some constants for changing some behaviours

VestaxTyphoon = new function(){
	this.pitchParams = [];
	this.groupToUnit = [];
	this.groupToControl = [];
	this.groupToDeck = [];
	this.groupToMidiChanOffset = [];
	this.alt = [];
	this.fitlerUsed = [];
	this.filter = [];
	this.loopJog = [];
	this.joged=[];
	this.jogTouch=[];
	this.pitchPos=[];
}

VestaxTyphoon.init = function(id){
	
	VestaxTyphoon.filter = {
		"[Channel1]": false,
		"[Channel2]": false,
	}
	
	VestaxTyphoon.filterUsed = {
		"[Channel1]": false,
		"[Channel2]": false,
	}	
	
	VestaxTyphoon.alt = {
		"[Channel1]": false,
		"[Channel2]": false,
	}

	VestaxTyphoon.groupToUnit = {
		"[Channel1]": "[EffectRack1_EffectUnit1_Effect1]",
		"[Channel2]": "[EffectRack1_EffectUnit2_Effect1]",
	}

	VestaxTyphoon.groupToControl = {
		"[Channel1]": 0x90,
		"[Channel2]": 0x91,
	}

	VestaxTyphoon.groupToDeck = {
		"[Channel1]": 1,
		"[Channel2]": 2,
	}

	VestaxTyphoon.groupToMidiChanOffset = {
		"[Channel1]": 0,
		"[Channel2]": 1,
	}	

	VestaxTyphoon.shift = false;

	VestaxTyphoon.pitchParams["[Channel1]Offset"]=1;
	VestaxTyphoon.pitchParams["[Channel1]Factor"]=-2;
	VestaxTyphoon.pitchParams["[Channel2]Offset"]=1;
	VestaxTyphoon.pitchParams["[Channel2]Factor"]=-2;

	VestaxTyphoon.pitchPos["[Channel1]"]=1;
	VestaxTyphoon.pitchPos["[Channel2]"]=1;

	VestaxTyphoon.loopJog["[Channel1]"]=0; //0=off, 1=in, 2=out, 3=in out
	VestaxTyphoon.loopJog["[Channel2]"]=0; //0=off, 1=in, 2=out, 3=in out

	VestaxTyphoon.joged["[Channel1]"]=false;
	VestaxTyphoon.joged["[Channel2]"]=false;

	VestaxTyphoon.jogTouch["[Channel1]"]=false;
	VestaxTyphoon.jogTouch["[Channel2]"]=false;

	VestaxTyphoon.loops.pressed==false;
	
	/*Makes that the sync buttons flash to the beatgrid. Deactivated by default because the author think that this is annoying but want to keep the code for those who like flashing consoles. ;-)
	Remove the double backslahes in the next line to activate it by uncommenting the lines*/
	
	engine.connectControl("[Channel1]","sync_enabled","VestaxTyphoon.hookMasterSyncCh1");
	engine.connectControl("[Channel2]","sync_enabled","VestaxTyphoon.hookMasterSyncCh2");

	engine.connectControl("[Channel1]","VuMeter","VestaxTyphoon.ch1_vu");
	engine.connectControl("[Channel2]","VuMeter","VestaxTyphoon.ch2_vu");
	
	engine.connectControl("[Channel1]","play_indicator","VestaxTyphoon.ch1_playing");
	engine.connectControl("[Channel2]","play_indicator","VestaxTyphoon.ch2_playing");
	engine.connectControl("[Channel1]","cue_indicator","VestaxTyphoon.ch1_cue");
	engine.connectControl("[Channel2]","cue_indicator","VestaxTyphoon.ch2_cue");		

	engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]","parameter1",true);
	engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]","parameter2",true);
	engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]","parameter3",true);

	engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]","parameter1",true);
	engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]","parameter2",true);
	engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]","parameter3",true);
}

VestaxTyphoon.boolToLight = function(value){
	if(value){
		return 0x7f;
	} else {
		return 0x00;
	}
}

VestaxTyphoon.DeckLoad = function(channel, control, value, status, group){
	//print("Channel: " + channel + "; Control: " + control + "; Value: " + value + "; Status: "+status + "; Group: "+ group);
	if(group=="[Channel1]"){
		if(value==0x7f){
			if(engine.getValue(group,"play")){
				engine.connectControl("[Channel1]","VuMeter","VestaxTyphoon.ch1_vu",true);
				engine.connectControl("[Channel1]","playposition","VestaxTyphoon.ch1_playpos");
			} else {
				engine.setValue(group,"LoadSelectedTrack",true);
			}
		}else{
			if(engine.getValue(group,"play")){
				engine.connectControl("[Channel1]","VuMeter","VestaxTyphoon.ch1_vu");
				engine.connectControl("[Channel1]","playposition","VestaxTyphoon.ch1_playpos",true);
			} else {
				engine.setValue(group,"LoadSelectedTrack",false);
			}
		}
	}

	if(group=="[Channel2]"){
		if(value==0x7f){
			if(engine.getValue(group,"play")){
				engine.connectControl("[Channel2]","VuMeter","VestaxTyphoon.ch2_vu",true);
				engine.connectControl("[Channel2]","playposition","VestaxTyphoon.ch2_playpos");
			} else {
				engine.setValue(group,"LoadSelectedTrack",true);
			}
		}else{
			if(engine.getValue(group,"play")){
				engine.connectControl("[Channel2]","VuMeter","VestaxTyphoon.ch2_vu");
				engine.connectControl("[Channel2]","playposition","VestaxTyphoon.ch2_playpos",true);
			} else {
				engine.setValue(group,"LoadSelectedTrack",false);
			}
		}
	}
}

VestaxTyphoon.ch1_playpos = function(value){
	var duration = engine.getValue("[Channel1]","duration");
	var alarm=60; //Track Ending indication (red led) in seconds*/
	if(value>0.0){
		midi.sendShortMsg(0x90,0x29,0x7f)
	} else {
		midi.sendShortMsg(0x90,0x29,0x00)
	}

	if(value>0.25){
		midi.sendShortMsg(0x90,0x2a,0x7f)
	} else {
		midi.sendShortMsg(0x90,0x2a,0x00)
	}

	if(value>0.5){
		midi.sendShortMsg(0x90,0x2b,0x7f)
	} else {
		midi.sendShortMsg(0x90,0x2b,0x00)
	}

	if(value>0.75){
		midi.sendShortMsg(0x90,0x2c,0x7f)
	} else {
		midi.sendShortMsg(0x90,0x2c,0x00)
	}

	if(value*duration > duration-alarm){
		midi.sendShortMsg(0x90,0x2d,0x7f)
	} else {
		midi.sendShortMsg(0x90,0x2d,0x00)
	}
}

VestaxTyphoon.ch2_playpos = function(value){
	var duration = engine.getValue("[Channel2]","duration");
	var alarm=60; //Track Ending indication (red led) in seconds*/
	if(value>0.0){
		midi.sendShortMsg(0x91,0x29,0x7f)
	} else {
		midi.sendShortMsg(0x91,0x29,0x00)
	}

	if(value>0.25){
		midi.sendShortMsg(0x91,0x2a,0x7f)
	} else {
		midi.sendShortMsg(0x91,0x2a,0x00)
	}

	if(value>0.5){
		midi.sendShortMsg(0x91,0x2b,0x7f)
	} else {
		midi.sendShortMsg(0x91,0x2b,0x00)
	}

	if(value>0.75){
		midi.sendShortMsg(0x91,0x2c,0x7f)
	} else {
		midi.sendShortMsg(0x91,0x2c,0x00)
	}

	if(value*duration > duration-alarm){
		midi.sendShortMsg(0x91,0x2d,0x7f)
	} else {
		midi.sendShortMsg(0x91,0x2d,0x00)
	}
}

VestaxTyphoon.ch1_vu = function(value){
	if(value>0.2){
		midi.sendShortMsg(0x90,0x29,0x7f)
	} else {
		midi.sendShortMsg(0x90,0x29,0x00)
	}

	if(value>0.4){
		midi.sendShortMsg(0x90,0x2a,0x7f)
	} else {
		midi.sendShortMsg(0x90,0x2a,0x00)
	}

	if(value>0.6){
		midi.sendShortMsg(0x90,0x2b,0x7f)
	} else {
		midi.sendShortMsg(0x90,0x2b,0x00)
	}

	if(value>0.8){
		midi.sendShortMsg(0x90,0x2c,0x7f)
	} else {
		midi.sendShortMsg(0x90,0x2c,0x00)
	}

	if(engine.getValue("[Channel1]","PeakIndicator")==true){
		midi.sendShortMsg(0x90,0x2d,0x7f)
	} else {
		midi.sendShortMsg(0x90,0x2d,0x00)
	}
}

VestaxTyphoon.ch2_vu = function(value){
	if(value>0.2){
		midi.sendShortMsg(0x91,0x29,0x7f)
	} else {
		midi.sendShortMsg(0x91,0x29,0x00)
	}

	if(value>0.4){
		midi.sendShortMsg(0x91,0x2a,0x7f)
	} else {
		midi.sendShortMsg(0x91,0x2a,0x00)
	}

	if(value>0.6){
		midi.sendShortMsg(0x91,0x2b,0x7f)
	} else {
		midi.sendShortMsg(0x91,0x2b,0x00)
	}

	if(value>0.8){
		midi.sendShortMsg(0x91,0x2c,0x7f)
	} else {
		midi.sendShortMsg(0x91,0x2c,0x00)
	}

	if(engine.getValue("[Channel2]","PeakIndicator")==true){
		midi.sendShortMsg(0x91,0x2d,0x7f)
	} else {
		midi.sendShortMsg(0x91,0x2d,0x00)
	}
}


VestaxTyphoon.slipShift = function (channel, control, value, status, group) {
	if(value==0x7f){
		VestaxTyphoon.shift=true;
		engine.setValue("[Channel1]","slip_enabled",true);
		engine.setValue("[Channel2]","slip_enabled",true);
		midi.sendShortMsg(0x92, 0x26, 0x7f);
		engine.connectControl("[Channel1]","sync_enabled","VestaxTyphoon.hookMasterSyncCh1",true);
		engine.connectControl("[Channel2]","sync_enabled","VestaxTyphoon.hookMasterSyncCh2",true);
		VestaxTyphoon.hookMasterSyncCh1(engine.getValue("[Channel1]","quantize"));
		VestaxTyphoon.hookMasterSyncCh2(engine.getValue("[Channel2]","quantize"));
		engine.connectControl("[Channel1]","quantize","VestaxTyphoon.hookMasterSyncCh1");
		engine.connectControl("[Channel2]","quantize","VestaxTyphoon.hookMasterSyncCh2");
	}else{
		VestaxTyphoon.shift=false;
		engine.setValue("[Channel1]","slip_enabled",false);
		engine.setValue("[Channel2]","slip_enabled",false);
		midi.sendShortMsg(0x92, 0x26, 0x00);
		engine.connectControl("[Channel1]","quantize","VestaxTyphoon.hookMasterSyncCh1",true);
		engine.connectControl("[Channel2]","quantize","VestaxTyphoon.hookMasterSyncCh2",true);		
		VestaxTyphoon.hookMasterSyncCh1(engine.getValue("[Channel1]","sync_enabled"));
		VestaxTyphoon.hookMasterSyncCh2(engine.getValue("[Channel2]","sync_enabled"));
		engine.connectControl("[Channel1]","sync_enabled","VestaxTyphoon.hookMasterSyncCh1");
		engine.connectControl("[Channel2]","sync_enabled","VestaxTyphoon.hookMasterSyncCh2");
	}
}

VestaxTyphoon.changeMode = function (channel, control, value, status, group) {
		if(value == 0x7f){
			VestaxTyphoon.filter[group] = true;
			midi.sendShortMsg(0x90 + channel, control, engine.getValue("[QuickEffectRack1_"+group+"]", "super1") != 0.5 ? 0x7f : 0x00);
		}else{
			VestaxTyphoon.filter[group] = false;
			if(VestaxTyphoon.filterUsed[group]) {
				VestaxTyphoon.filterUsed[group] = false;		
			} else {		
				if (VestaxTyphoon.alt[group] == true) {
					VestaxTyphoon.alt[group] = false;
					VestaxTyphoon.setTransportLights(group);
					if(group=="[Channel1]"){
						engine.connectControl("[Channel1]","play_indicator","VestaxTyphoon.ch1_playing");
						engine.connectControl("[Channel1]","cue_indicator","VestaxTyphoon.ch1_cue");
					}else if (group=="[Channel2]"){
						engine.connectControl("[Channel2]","play_indicator","VestaxTyphoon.ch2_playing");
						engine.connectControl("[Channel2]","cue_indicator","VestaxTyphoon.ch2_cue");
					}
				} else {
					VestaxTyphoon.alt[group] = true;
					VestaxTyphoon.setHotcueLights(group);
					if(group=="[Channel1]"){
						engine.connectControl("[Channel1]","play_indicator","VestaxTyphoon.ch1_playing",true);
						engine.connectControl("[Channel1]","cue_indicator","VestaxTyphoon.ch1_cue",true);
					}else if (group=="[Channel2]"){
						engine.connectControl("[Channel2]","play_indicator","VestaxTyphoon.ch2_playing",true);
						engine.connectControl("[Channel2]","cue_indicator","VestaxTyphoon.ch2_cue",true);
					}
				}
				
			}
			midi.sendShortMsg(0x90 + channel, control, VestaxTyphoon.alt[group] ? 0x7f : 0x00);
		}
}

VestaxTyphoon.changeModeSimple = function (channel, control, value, status, group) {
		if(value == 0x7f){
			VestaxTyphoon.filter[group] = true;
			midi.sendShortMsg(0x90 + channel, control, engine.getValue("[QuickEffectRack1_"+group+"]", "super1") != 0.5 ? 0x7f : 0x00);
		}else{
			VestaxTyphoon.filter[group] = false;
		}
}

VestaxTyphoon.setTransportLights = function(group){
	//Play
	midi.sendShortMsg(VestaxTyphoon.groupToControl[group], 0x32, VestaxTyphoon.boolToLight(engine.getValue(group,"play_indicator")));
	//turn off other lights
	midi.sendShortMsg(VestaxTyphoon.groupToControl[group], 0x33, 0x00);
	midi.sendShortMsg(VestaxTyphoon.groupToControl[group], 0x35, VestaxTyphoon.boolToLight(engine.getValue(group,"cue_indicator")));
}

/*connected light controls*/
VestaxTyphoon.ch1_playing = function(value) { //playbutton light
	if(value==true){
		midi.sendShortMsg(0x90, 0x32, 0x7f);
	}else{
		midi.sendShortMsg(0x90, 0x32, 0x00);
	}
}

VestaxTyphoon.ch1_cue = function(value) { //playbutton light
	if(value==true){
		midi.sendShortMsg(0x90, 0x35, 0x7f);
	}else{
		midi.sendShortMsg(0x90, 0x35, 0x00);
	}
}

VestaxTyphoon.ch2_playing = function(value) { //playbutton light
	if(value==true){
		midi.sendShortMsg(0x91, 0x32, 0x7f);
	}else{
		midi.sendShortMsg(0x91, 0x32, 0x00);
	}
}

VestaxTyphoon.ch2_cue = function(value) { //playbutton light
	if(value==true){
		midi.sendShortMsg(0x91, 0x35, 0x7f);
	}else{
		midi.sendShortMsg(0x91, 0x35, 0x00);
	}
}

VestaxTyphoon.setHotcueLights = function(group){
	//Play - Hotcue1
	midi.sendShortMsg(VestaxTyphoon.groupToControl[group], 0x32, VestaxTyphoon.boolToLight(engine.getValue(group,"hotcue_1_enabled")) );
	//Cue - Hotcue2
	midi.sendShortMsg(VestaxTyphoon.groupToControl[group], 0x35, VestaxTyphoon.boolToLight(engine.getValue(group,"hotcue_2_enabled")) );
	//Cup - Hotcue3
	midi.sendShortMsg(VestaxTyphoon.groupToControl[group], 0x33, VestaxTyphoon.boolToLight(engine.getValue(group,"hotcue_3_enabled")) );
}

VestaxTyphoon.playh1 = function (channel, control, value, status, group) {
	if (VestaxTyphoon.alt[group] == false) {
		if (engine.getValue(group,"play") == true) {
			if (value == 0x7f) {
				engine.setValue(group,"play",false);
			}
		} else {
			if (value == 0x7f) {
				engine.setValue(group,"play",true);
			}
		}
	} else {
		if(VestaxTyphoon.loopJog[group]==3 && engine.getValue(group,"hotcue_1_enabled")){//if loop pressed and hotcue available
			var scale = engine.getValue(group,"loop_end_position")-engine.getValue(group,"loop_start_position");
			var start = engine.getValue(group,"hotcue_1_position");
			if(engine.getValue(group,"loop_start_position")<start){//fixes non looping behaviour when first moving loop out with a hotcue before the loop start
				engine.setValue(group,"loop_end_position",start+scale);
				engine.setValue(group,"loop_start_position",start);
			}else{
				engine.setValue(group,"loop_start_position",start);
				engine.setValue(group,"loop_end_position",start+scale);
			}
			VestaxTyphoon.joged[group]=true;
			if(engine.getValue(group,"loop_enabled")){
				engine.setValue(group,"hotcue_1_activate",true);
			}
		}else{
			engine.setValue(group,"hotcue_1_activate",(value == 0x7f))
		}
		midi.sendShortMsg(0x90 + channel, control, 0x7f);
	}
}

VestaxTyphoon.cueh2 = function (channel, control, value, status, group) {
	if (VestaxTyphoon.alt[group] == false) {
		engine.setValue(group,"cue_default",(value == 0x7f));
		engine.setValue(group, "back", false);
	} else {
		if(VestaxTyphoon.loopJog[group]==3 && engine.getValue(group,"hotcue_2_enabled")){//if loop pressed and hotcue available
			var scale = engine.getValue(group,"loop_end_position")-engine.getValue(group,"loop_start_position");
			var start = engine.getValue(group,"hotcue_2_position");
			if(engine.getValue(group,"loop_start_position")<start){//fixes non looping behaviour when first moving loop out with a hotcue before the loop start
				engine.setValue(group,"loop_end_position",start+scale);
				engine.setValue(group,"loop_start_position",start);
			}else{
				engine.setValue(group,"loop_start_position",start);
				engine.setValue(group,"loop_end_position",start+scale);
			}
			VestaxTyphoon.joged[group]=true;
			if(engine.getValue(group,"loop_enabled")){
				engine.setValue(group,"hotcue_2_activate",true);
			}
		}else{
			engine.setValue(group,"hotcue_2_activate",(value == 0x7f))
		}
		midi.sendShortMsg(0x90 + channel, control, 0x7f);
	}
}

VestaxTyphoon.cuph3 = function (channel, control, value, status, group) {
	if (VestaxTyphoon.alt[group] == false) {
		engine.setValue(group,"cue_goto",(value == 0x7f));
		midi.sendShortMsg(0x90 + channel, control, value);
		engine.setValue(group, "fwd", false);
		midi.sendShortMsg(0x90 + channel, 0x32, engine.getValue(group,"play"));
	} else {
		if(VestaxTyphoon.loopJog[group]==3 && engine.getValue(group,"hotcue_3_enabled")){//if loop pressed and hotcue available
			var scale = engine.getValue(group,"loop_end_position")-engine.getValue(group,"loop_start_position");
			var start = engine.getValue(group,"hotcue_3_position");
			if(engine.getValue(group,"loop_start_position")<start){//fixes non looping behaviour when first moving loop out with a hotcue before the loop start
				engine.setValue(group,"loop_end_position",start+scale);
				engine.setValue(group,"loop_start_position",start);
			}else{
				engine.setValue(group,"loop_start_position",start);
				engine.setValue(group,"loop_end_position",start+scale);
			}
			VestaxTyphoon.joged[group]=true;
			if(engine.getValue(group,"loop_enabled")){
				engine.setValue(group,"hotcue_3_activate",true);
			}
		}else{
			engine.setValue(group,"hotcue_3_activate",(value == 0x7f));
		}
		midi.sendShortMsg(0x90 + channel, control, 0x7f);
	}
}

VestaxTyphoon.begd1 = function (channel, control, value, status, group) {
	if (VestaxTyphoon.alt[group] == false) {
        engine.setValue(group, "play", 0);
        engine.setValue(group, "playposition", 0);
		midi.sendShortMsg(0x90 + channel, 0x32, 0x00);
	} else {
		engine.setValue(group,"hotcue_1_clear",(value == 0x7f));
		midi.sendShortMsg(0x90 + channel, 0x32, 0x00);
		VestaxTyphoon.setHotcueLights(group);
	}
}

VestaxTyphoon.rrd2 = function (channel, control, value, status, group) {
	if (VestaxTyphoon.alt[group] == false) {
        engine.setValue(group, "back", (value==0x7f));
	} else {
		engine.setValue(group,"hotcue_2_clear",(value == 0x7f));
		VestaxTyphoon.setHotcueLights(group);
	}
}

VestaxTyphoon.ffd3 = function (channel, control, value, status, group) {
	if (VestaxTyphoon.alt[group] == false) {
        engine.setValue(group, "fwd", (value==0x7f));
	} else {
		engine.setValue(group,"hotcue_3_clear",(value == 0x7f));
		VestaxTyphoon.setHotcueLights(group);
	}
}

VestaxTyphoon.eqLow = function(channel, control, value, status, group) {
	if(VestaxTyphoon.shift){
		engine.setParameter(VestaxTyphoon.groupToUnit[group], "parameter1", value/127);
	}else{
		engine.setParameter("[EqualizerRack1_"+group+"_Effect1]", "parameter1", value/127); //deprecated in 1.12?
	}
}

VestaxTyphoon.eqMid = function(channel, control, value, status, group) {
	if(VestaxTyphoon.shift){
		engine.setParameter(VestaxTyphoon.groupToUnit[group], "parameter2", value/127);
	}else{
		engine.setParameter("[EqualizerRack1_"+group+"_Effect1]", "parameter2", value/127); //deprecated in 1.12?
	}
}

VestaxTyphoon.eqHigh = function(channel, control, value, status, group) {
	if(VestaxTyphoon.shift){
		engine.setParameter(VestaxTyphoon.groupToUnit[group], "parameter3", value/127);
	}else{
		engine.setParameter("[EqualizerRack1_"+group+"_Effect1]", "parameter3", value/127); //deprecated in 1.12?
	}
}

VestaxTyphoon.relPitch = function(channel, control, value, status, group) {
	if(control == "0x22") {
		if(engine.getValue(group,"sync_enabled") == true){
			if(VestaxTyphoon.shift){
				engine.setValue(group,"beats_adjust_slower",true);
			}else{
				engine.setValue(group,"beats_translate_earlier",true);
			}
		}else{
			if(VestaxTyphoon.shift){
				engine.setValue(group,"beats_adjust_slower",true);
			}else{			
				VestaxTyphoon.pitchParams[group+"Offset"]=engine.getValue(group,"rate")+0.1-1/135; //offset failure correction
				VestaxTyphoon.pitchParams[group+"Factor"]=-0.2; //Relative Sensivity
			}
		}
	}else if(control == "0x23") {
		if(engine.getValue(group,"sync_enabled") == true){
			if(VestaxTyphoon.shift){
				engine.setValue(group,"beats_adjust_faster",true);
			}else{
				engine.setValue(group,"beats_translate_later",true);
			}
		}else{
			if(VestaxTyphoon.shift){
				engine.setValue(group,"beats_adjust_faster",true);
			}else{
				VestaxTyphoon.pitchParams[group+"Offset"]=1;
				VestaxTyphoon.pitchParams[group+"Factor"]=-2;
			}
		}
	}
}

VestaxTyphoon.setPitch = function(channel, control, value, status, group) {
	engine.setValue(group,"rate",VestaxTyphoon.pitchParams[group+"Offset"]+1/127+VestaxTyphoon.pitchParams[group+"Factor"]*value/(127));
	VestaxTyphoon.pitchPos[group]=value;
}

VestaxTyphoon.setSync = function(channel, control, value, status, group) {
		if(value == 0x00){
			if(VestaxTyphoon.shift){
				if(engine.getValue(group,"quantize") == true){
					engine.setValue(group,"quantize",false);	
				}else{
					engine.setValue(group,"quantize",true);	
				}				
			}else{				
				if(engine.getValue(group,"sync_enabled") == true){
					engine.setValue(group,"sync_enabled",false);	
				}else{
					engine.setValue(group,"sync_enabled",true);	
				}
			}
		}		
}

VestaxTyphoon.setSyncSimple = function(channel, control, value, status, group) {
		if(value == 0x7f){			
			if(engine.getValue(group,"sync_enabled") == true){
				engine.setValue(group,"sync_enabled",false);	
			}else{
				engine.setValue(group,"sync_enabled",true);	
			}
		}		
}

VestaxTyphoon.hookMasterSyncCh1 = function(value) {
	if(value == true){
		midi.sendShortMsg(0x90, 0x46, 0x7f);
	}else{
		midi.sendShortMsg(0x90, 0x46, 0x00);	
	}
}

VestaxTyphoon.hookMasterSyncCh2 = function(value) {
	if(value == true){
		midi.sendShortMsg(0x91, 0x46, 0x7f);
	}else{
		midi.sendShortMsg(0x91, 0x46, 0x00);	
	}
}

VestaxTyphoon.wheelTouch = function(channel, control, value, status, group){
	if(VestaxTyphoon.filter[group]){
		if(value == 0x00){
			engine.setValue("[QuickEffectRack1_"+group+"]", "super1", 0.5);	
			midi.sendShortMsg(0x90 + channel, 0x24, engine.getValue("[QuickEffectRack1_"+group+"]", "super1") != 0.5 ? 0x7f : 0x00);
			VestaxTyphoon.filterUsed[group] = true;
		}
		VestaxTyphoon.filterUsed[group] = true;
	}else{
		if(value == 0x7f){	
			if(VestaxTyphoon.loopJog[group]==0){
				engine.scratchEnable(VestaxTyphoon.groupToDeck[group],300, 33+(1.0/3), 1.0/8, (1.0/8)/32);
			}
			VestaxTyphoon.jogTouch[group]=true;
		} else {
			engine.scratchDisable(VestaxTyphoon.groupToDeck[group]);
			VestaxTyphoon.jogTouch[group]=false;
		}
	}	
}

VestaxTyphoon.wheelTurn = function(channel, control, value, status, group){
    if (VestaxTyphoon.jogTouch[group]) {
		switch(VestaxTyphoon.loopJog[group]){
			case 0:	
				engine.scratchTick(VestaxTyphoon.groupToDeck[group], value - 0x40);
				break;
			case 1:
				engine.setValue(group, "loop_start_position", engine.getValue(group,"loop_start_position") + 20*(value -0x40));
				break;
			case 2:
				engine.setValue(group, "loop_end_position", engine.getValue(group,"loop_end_position") + 20*(value - 0x40));
				break;
			case 3:
				engine.setValue(group, "loop_start_position", engine.getValue(group,"loop_start_position") + 20*(value -0x40));
				engine.setValue(group, "loop_end_position", engine.getValue(group,"loop_end_position") + 20*(value - 0x40));
				VestaxTyphoon.joged[group]=true;
				break;
		}
	} else {
		switch(VestaxTyphoon.loopJog[group]){
			case 0:
				if(VestaxTyphoon.filter[group]){
					engine.setValue("[QuickEffectRack1_"+group+"]", "super1", engine.getValue("[QuickEffectRack1_"+group+"]", "super1") + (value - 0x40)/0xff);
					VestaxTyphoon.filterUsed[group] = true;
					midi.sendShortMsg(0x90 + channel, 0x24, engine.getValue("[QuickEffectRack1_"+group+"]", "super1") != 0.5 ? 0x7f : 0x00);
				}else{
					engine.setValue(group, "jog", value - 0x40);
				}
				break;
			case 1:	
				engine.setValue(group, "loop_start_position", engine.getValue(group,"loop_start_position") + 5*(value -0x40));
				break;
			case 2:	
				engine.setValue(group, "loop_end_position", engine.getValue(group,"loop_end_position") + 5*(value - 0x40));
				break;
			case 3:
				engine.setValue(group, "loop_start_position", engine.getValue(group,"loop_start_position") + 5*(value -0x40));
				engine.setValue(group, "loop_end_position", engine.getValue(group,"loop_end_position") + 5*(value - 0x40));
				VestaxTyphoon.joged[group]=true;
				break;
		}
	}
}

VestaxTyphoon.loopInMinus = function(channel, control, value, status, group){
	if(!VestaxTyphoon.alt[group]){//Filter off
		if(value == 0x7f){
			if(engine.getValue(group,"loop_enabled")==true){
				engine.setValue(group,"loop_halve",true);
			}else{
				engine.setValue(group,"beatloop_2_activate",true);
			}
			midi.sendShortMsg(0x90+channel,0x21,0x7f);
		}else{
			engine.setValue(group,"loop_halve",false);
			engine.setValue(group,"beatloop_2_activate",false);
			midi.sendShortMsg(0x90+channel,0x21,0x00);
		}
	}else{//Filter on
		if (value == 0x7f){
			if(VestaxTyphoon.loopJog[group]==3){//Wenn Loop gedrückt
				var end = engine.getValue(group,"loop_start_position");
				engine.setValue(group,"loop_start_position",2*end - engine.getValue(group,"loop_end_position"));
				engine.setValue(group,"loop_end_position",end);
				VestaxTyphoon.joged[group]=true;
			}else{//Nur Loop In gedrückt
				if(engine.getValue(group,"loop_enabled")==false){
					engine.setValue(group,"loop_in",true);
				}else{
					VestaxTyphoon.loopJog[group]=1;//Loop out bewegen
				}
			}
			midi.sendShortMsg(0x90+channel,0x21,0x00);
		}else{
			engine.setValue(group,"loop_in",false);
			if(VestaxTyphoon.loopJog[group]!=3){
				VestaxTyphoon.loopJog[group]=0;
			}
			midi.sendShortMsg(0x90+channel,0x21,0x00);
		}
	}
}

VestaxTyphoon.loops = function(channel,control,value,status,group){
	if(!VestaxTyphoon.alt[group]){//Filter off
		if(value == 0x7f){
			if(engine.getValue(group,"loop_enabled")==true){
				engine.setValue(group,"reloop_exit",true);
			}else{
				engine.setValue(group,"beatloop_4_activate",true);
			}
		}else{
			engine.setValue(group,"loop_halve",false);
			engine.setValue(group,"beatloop_2_activate",false);
			engine.setValue(group,"reloop_exit",false);
		}
	}else{//Filter on
		if(value == 0x7f){
			VestaxTyphoon.loopJog[group]=3;
		}else{
			if(VestaxTyphoon.joged[group]==false){//Loop nicht bewegt
				if(engine.getValue(group,"loop_enabled")==false){
					engine.setValue(group,"reloop_exit",true);
				}else{
					engine.setValue(group,"reloop_exit",true);
				}
			}else{
				VestaxTyphoon.joged[group]=false;
			}
			VestaxTyphoon.loopJog[group]=0;
		}
	}
}

VestaxTyphoon.loopOutPlus = function(channel, control, value, status, group){
	if(!VestaxTyphoon.alt[group]){//Filter off
		if(value == 0x7f){
			if(engine.getValue(group,"loop_enabled")==true){
				engine.setValue(group,"loop_double",true);
			}else{
				engine.setValue(group,"beatloop_8_activate",true);
			}
			midi.sendShortMsg(0x90+channel,0x42,0x7f);
		}
		else{
			engine.setValue(group,"loop_double",false);
			engine.setValue(group,"beatloop_8_activate",false);
			midi.sendShortMsg(0x90+channel,0x42,0x00);
		}
	}else{//Filter on
		if (value == 0x7f){
			if(VestaxTyphoon.loopJog[group]==3){//Wenn Loop gedrückt
				var start = engine.getValue(group,"loop_end_position");
				engine.setValue(group,"loop_end_position",2*start - engine.getValue(group,"loop_start_position"));
				engine.setValue(group,"loop_start_position",start);
				VestaxTyphoon.joged[group]=true;
			}else{//Nur Loop Out gedrückt
				if(engine.getValue(group,"loop_enabled")==false){
					engine.setValue(group,"loop_out",true);
				}else{
					VestaxTyphoon.loopJog[group]=2;//Loop out bewegen
				}
			}
			midi.sendShortMsg(0x90+channel,0x42,0x7f);
		}else{
			engine.setValue(group,"loop_out",false);
			if(VestaxTyphoon.loopJog[group]!=3){
				VestaxTyphoon.loopJog[group]=0;
			}
			midi.sendShortMsg(0x90+channel,0x42,0x00);
		}
	}
}