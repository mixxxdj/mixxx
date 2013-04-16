/****************************************************************/
/*      Reloop Digital Jockey 2 controller script v 1.0         */
/*          Copyright (C) 2010, Tobias Rafreider  		*/
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 1.8.x                                 */
/****************************************************************/

function DigitalJockey2Controller() {}

DigitalJockey2Controller.ledOn = 0x7F;
DigitalJockey2Controller.ledOff = 0x00;
DigitalJockey2Controller.keyPressed = 0x7F;
DigitalJockey2Controller.keyUp = 0x00;

DigitalJockey2Controller.scratchModeChannel1 = false;
DigitalJockey2Controller.scratchModeChannel2 = false;

DigitalJockey2Controller.searchModeChannel1 = false;
DigitalJockey2Controller.searchModeChannel2 = false;

//boolean value that indicated if CUP LED is active
DigitalJockey2Controller.CUP_Button1_IsActive = false;
DigitalJockey2Controller.CUP_Button2_IsActive = false;

DigitalJockey2Controller.init = function(id){
    //print ("Initalizing Reloop Digital Jockey 2 Controler Edition.");
	DigitalJockey2Controller.resetLEDs();

	engine.connectControl("[Channel1]","play","DigitalJockey2Controller.isChannel1_Playing");
	engine.connectControl("[Channel2]","play","DigitalJockey2Controller.isChannel2_Playing");
	
	engine.connectControl("[Channel1]","cue_default","DigitalJockey2Controller.isChannel1_Cue_Active");
	engine.connectControl("[Channel2]","cue_default","DigitalJockey2Controller.isChannel2_Cue_Active");
	
	engine.connectControl("[Channel1]","filterHighKill","DigitalJockey2Controller.OnFilterHigh_KillButton1");
	engine.connectControl("[Channel1]","filterLowKill","DigitalJockey2Controller.OnFilterLow_KillButton1");
	engine.connectControl("[Channel1]","filterMidKill","DigitalJockey2Controller.OnFilterMid_KillButton1");
	
	engine.connectControl("[Channel2]","filterHighKill","DigitalJockey2Controller.OnFilterHigh_KillButton2");
	engine.connectControl("[Channel2]","filterLowKill","DigitalJockey2Controller.OnFilterLow_KillButton2");
	engine.connectControl("[Channel2]","filterMidKill","DigitalJockey2Controller.OnFilterMid_KillButton2");
	
	engine.connectControl("[Channel1]","pfl","DigitalJockey2Controller.OnPFL_Button1");
	engine.connectControl("[Channel2]","pfl","DigitalJockey2Controller.OnPFL_Button2");
	
	//Looping
	engine.connectControl("[Channel1]","loop_enabled","DigitalJockey2Controller.LoopActiveLED1");
	engine.connectControl("[Channel2]","loop_enabled","DigitalJockey2Controller.LoopActiveLED2");
	//Key Lock
	engine.connectControl("[Channel1]","keylock","DigitalJockey2Controller.OnKeyLockChange1");
	engine.connectControl("[Channel2]","keylock","DigitalJockey2Controller.OnKeyLockChange2");
	//Flanger
	engine.connectControl("[Channel1]","flanger","DigitalJockey2Controller.OnFlangerChange1");
	engine.connectControl("[Channel2]","flanger","DigitalJockey2Controller.OnFlangerChange2");
}
DigitalJockey2Controller.resetLEDs = function(){

	//Turn all LEDS off 
	midi.sendShortMsg(0x90, 0x19, DigitalJockey2Controller.ledOff);   // Turn on the Play LED1 off
	midi.sendShortMsg(0x90, 0x17, DigitalJockey2Controller.ledOff); //Turn CUP LED1 off
	midi.sendShortMsg(0x90, 0x18, DigitalJockey2Controller.ledOff); //Turn CUE LED1 off
	midi.sendShortMsg(0x90, 0x05, DigitalJockey2Controller.ledOff); //Turn PFL LED off
	midi.sendShortMsg(0x90, 0x14, DigitalJockey2Controller.ledOff); //HighFilterKill
	midi.sendShortMsg(0x90, 0x15, DigitalJockey2Controller.ledOff); //MidFilterKill
	midi.sendShortMsg(0x90, 0x16, DigitalJockey2Controller.ledOff); //LowFilterKill
	midi.sendShortMsg(0x90, 0x1B, DigitalJockey2Controller.ledOff); //disable scratch control
	midi.sendShortMsg(0x90, 0x1A, DigitalJockey2Controller.ledOff); //disable search control
	midi.sendShortMsg(0x90, 0x1C, DigitalJockey2Controller.ledOff); //disable fx dry/wet control
	midi.sendShortMsg(0x90, 0x02, DigitalJockey2Controller.ledOff); //Turn KeyLock off
	midi.sendShortMsg(0x90, 0x12, DigitalJockey2Controller.ledOff); //Loop1 LED Off
	midi.sendShortMsg(0x90, 0x0e, DigitalJockey2Controller.ledOff); //Flanger
	midi.sendShortMsg(0x90, 0x1A, DigitalJockey2Controller.ledOff); //Search
	midi.sendShortMsg(0x90, 0x1b, DigitalJockey2Controller.ledOff);//Scratch
	
	
	midi.sendShortMsg(0x90, 0x55, DigitalJockey2Controller.ledOff);   // Turn on the Play LED2 off
	midi.sendShortMsg(0x90, 0x53, DigitalJockey2Controller.ledOff); //Turn CUP LED2 off
	midi.sendShortMsg(0x90, 0x54, DigitalJockey2Controller.ledOff); //Turn CUE LED2 off
	midi.sendShortMsg(0x90, 0x41, DigitalJockey2Controller.ledOff); //Turn PFL LED off
	midi.sendShortMsg(0x90, 0x50, DigitalJockey2Controller.ledOff); //HighFilterKill
	midi.sendShortMsg(0x90, 0x51, DigitalJockey2Controller.ledOff); //MidFilterKill
	midi.sendShortMsg(0x90, 0x52, DigitalJockey2Controller.ledOff); //LowFilterKill
	midi.sendShortMsg(0x90, 0x57, DigitalJockey2Controller.ledOff); //disable scratch control
	midi.sendShortMsg(0x90, 0x56, DigitalJockey2Controller.ledOff); //disable search control
	midi.sendShortMsg(0x90, 0x58, DigitalJockey2Controller.ledOff); //disable fx dry/wet control
	midi.sendShortMsg(0x90, 0x3E, DigitalJockey2Controller.ledOff); //Turn KeyLock2 off
	midi.sendShortMsg(0x90, 0x4E, DigitalJockey2Controller.ledOff); //Loop2 LED Off
	midi.sendShortMsg(0x90, 0x45, DigitalJockey2Controller.ledOff); //Flanger
	midi.sendShortMsg(0x90, 0x56, DigitalJockey2Controller.ledOff); //Search
	midi.sendShortMsg(0x90, 0x57, DigitalJockey2Controller.ledOff);//Scratch
}
DigitalJockey2Controller.shutdown = function(id){
 //Turn all LEDs off by using init function
 DigitalJockey2Controller.resetLEDs();
}

// Play button deck 1
DigitalJockey2Controller.playButton1 = function (channel, control, value) {
	DigitalJockey2Controller.playTrack(1, control, value);	
}
// Play Button deck 2
DigitalJockey2Controller.playButton2 = function (channel, control, value) {
	DigitalJockey2Controller.playTrack(2, control, value);
}

DigitalJockey2Controller.playTrack = function (channel, control, value) {
	/*
	 * We immediately want to start and stop playing as soon as play button has been pressed
	 * KeyUp events are out of interest in this case
	 */
	if(value == DigitalJockey2Controller.keyPressed){
		//If no song is loaded
		if (engine.getValue("[Channel"+channel+"]", "duration") == 0) { 
				return; 
		};
		var currentlyPlaying = engine.getValue("[Channel"+channel+"]","play");
		if (currentlyPlaying == 1) {    // If currently playing
			engine.setValue("[Channel"+channel+"]","play",0);    // Stop
			if(channel == 1) {
				midi.sendShortMsg(0x90, 0x17, DigitalJockey2Controller.ledOff); //Turn CUP LED off
				DigitalJockey2Controller.CUP_Button1_IsActive = false;
			}
			else if(channel == 2){
				midi.sendShortMsg(0x90, 0x53, DigitalJockey2Controller.ledOff); //Turn CUP LED off
				DigitalJockey2Controller.CUP_Button2_IsActive = false;
			}
		}
		else {    // If not currently playing,
			engine.setValue("[Channel"+channel+"]","play",1);    // Start
		}
	}
}

DigitalJockey2Controller.CueButton1 = function (channel, control, value) {
	DigitalJockey2Controller.Cue(1, control, value);	
}
DigitalJockey2Controller.CueButton2 = function (channel, control, value) {
	DigitalJockey2Controller.Cue(2, control, value);	
}
DigitalJockey2Controller.Cue = function (channel, control, value) {
	//If no song is loaded
	if (engine.getValue("[Channel"+channel+"]", "duration") == 0) { 
			return; 
	};
	// As soon as we press CUE, execute CUE Logic
	if(value == DigitalJockey2Controller.keyPressed){
		engine.setValue("[Channel"+channel+"]","cue_default",1);
		if(channel == 1) {
			midi.sendShortMsg(0x90, 0x19, DigitalJockey2Controller.ledOff);   // Turn on the Play LED off
			midi.sendShortMsg(0x90, 0x17, DigitalJockey2Controller.ledOff); //Turn CUP LED off
			DigitalJockey2Controller.CUP_Button1_IsActive = false;
		}
		else if(channel == 2){
			midi.sendShortMsg(0x90, 0x55, DigitalJockey2Controller.ledOff);   // Turn on the Play LED off
			midi.sendShortMsg(0x90, 0x53, DigitalJockey2Controller.ledOff); //Turn CUP LED off
			DigitalJockey2Controller.CUP_Button2_IsActive = false;
		}
	
	}
	else if(value == DigitalJockey2Controller.keyUp){
		engine.setValue("[Channel"+channel+"]","cue_default",0);
	}
	
}
DigitalJockey2Controller.CuePlayButton1 = function (channel, control, value) {
	DigitalJockey2Controller.CuePlay(1, control, value);	
}
DigitalJockey2Controller.CuePlayButton2 = function (channel, control, value) {
	DigitalJockey2Controller.CuePlay(2, control, value);	
}

DigitalJockey2Controller.CuePlayButton1 = function (channel, control, value) {
	DigitalJockey2Controller.CuePlay(1, control, value);	
}
DigitalJockey2Controller.CuePlayButton2 = function (channel, control, value) {
	DigitalJockey2Controller.CuePlay(2, control, value);	
}
DigitalJockey2Controller.CuePlay = function (channel, control, value) {
	// As soon as we press CUP, execute CUP Logic
	if(value == DigitalJockey2Controller.keyPressed){
		//If no song is loaded
		if (engine.getValue("[Channel"+channel+"]", "duration") == 0) { 
				return; 
		};
		var currentlyPlaying = engine.getValue("[Channel"+channel+"]","play");
		//print ("isPlaying" + currentlyPlaying);
		if(currentlyPlaying == 1){
			if(channel == 1) {
				midi.sendShortMsg(0x90, 0x19, DigitalJockey2Controller.ledOff);   // Turn on the Play LED off
			}
			else if(channel == 2){
				midi.sendShortMsg(0x90, 0x55, DigitalJockey2Controller.ledOff);   // Turn on the Play LED off
			}
		}

		engine.setValue("[Channel"+channel+"]","cue_default",1);
		engine.setValue("[Channel"+channel+"]","cue_default",0);
		engine.setValue("[Channel"+channel+"]","play",1);

		if(channel == 1) {
			DigitalJockey2Controller.CUP_Button1_IsActive = true;
		}
		else if(channel == 2){
			DigitalJockey2Controller.CUP_Button2_IsActive = true;
		}
		
	}
}
DigitalJockey2Controller.EnableHeadPhone1 = function (channel, control, value) {
	DigitalJockey2Controller.EnableHeadPhone(1, control, value);
}
DigitalJockey2Controller.EnableHeadPhone2 = function (channel, control, value) {
	DigitalJockey2Controller.EnableHeadPhone(2, control, value);
}
DigitalJockey2Controller.EnableHeadPhone = function (channel, control, value) {
	var isHeadPhoneActive = engine.getValue("[Channel"+channel+"]","pfl");
	//print("Channel"+channel+" isHeadPhoneActive: " + isHeadPhoneActive);
	if(value == DigitalJockey2Controller.keyPressed){
		if(isHeadPhoneActive == 1){
			engine.setValue("[Channel"+channel+"]","pfl",0);
		}
		else{
			engine.setValue("[Channel"+channel+"]","pfl",1);
		}
	}
}

DigitalJockey2Controller.BassKillChannel1 = function (channel, control, value){
	var deck = 1;
	if(value == DigitalJockey2Controller.keyPressed){
		var isKillButtonIsActive = engine.getValue("[Channel"+deck+"]","filterLowKill");
		if(isKillButtonIsActive == true){
			engine.setValue("[Channel"+deck+"]","filterLowKill",0);
		}
		else{
			engine.setValue("[Channel"+deck+"]","filterLowKill",1);
		}
	}
}
DigitalJockey2Controller.MidKillChannel1 = function (channel, control, value){
	var deck = 1;
	if(value == DigitalJockey2Controller.keyPressed){
		var isKillButtonIsActive = engine.getValue("[Channel"+deck+"]","filterMidKill");
		if(isKillButtonIsActive == true){
			engine.setValue("[Channel"+deck+"]","filterMidKill",0);
		}
		else{
			engine.setValue("[Channel"+deck+"]","filterMidKill",1);
		}
	}
}

DigitalJockey2Controller.HighKillChannel1 = function (channel, control, value){
	var deck = 1;
	if(value == DigitalJockey2Controller.keyPressed){
		var isKillButtonIsActive = engine.getValue("[Channel"+deck+"]","filterHighKill");
		if(isKillButtonIsActive == true){
			engine.setValue("[Channel"+deck+"]","filterHighKill",0);
		}
		else{
			engine.setValue("[Channel"+deck+"]","filterHighKill",1);
		}
	}
}
DigitalJockey2Controller.BassKillChannel2 = function (channel, control, value){
	var deck = 2;
	if(value == DigitalJockey2Controller.keyPressed){
		var isKillButtonIsActive = engine.getValue("[Channel"+deck+"]","filterLowKill");
		if(isKillButtonIsActive == true){
			engine.setValue("[Channel"+deck+"]","filterLowKill",0);
		}
		else{
			engine.setValue("[Channel"+deck+"]","filterLowKill",1);
		}
	}
}
DigitalJockey2Controller.MidKillChannel2 = function (channel, control, value){
	var deck = 2;
	if(value == DigitalJockey2Controller.keyPressed){
		var isKillButtonIsActive = engine.getValue("[Channel"+deck+"]","filterMidKill");
		if(isKillButtonIsActive == true){
			engine.setValue("[Channel"+deck+"]","filterMidKill",0);
		}
		else{
			engine.setValue("[Channel"+deck+"]","filterMidKill",1);
		}
	}
}
DigitalJockey2Controller.HighKillChannel2 = function (channel, control, value){
	var deck = 2;
	if(value == DigitalJockey2Controller.keyPressed){
		var isKillButtonIsActive = engine.getValue("[Channel"+deck+"]","filterHighKill");
		if(isKillButtonIsActive == true){
			engine.setValue("[Channel"+deck+"]","filterHighKill",0);
		}
		else{
			engine.setValue("[Channel"+deck+"]","filterHighKill",1);
		}
	}
}
DigitalJockey2Controller.Scratch1 = function (channel, control, value){
	DigitalJockey2Controller.Scratch(1, control, value);
}
DigitalJockey2Controller.Scratch2 = function (channel, control, value){
	DigitalJockey2Controller.Scratch(2, control, value);
}
DigitalJockey2Controller.Scratch = function (channel, control, value){
	if(value == DigitalJockey2Controller.keyPressed){
		if(channel == 1){
		    //print("Scratch 1: " + DigitalJockey2Controller.scratchModeChannel1);
			if(DigitalJockey2Controller.scratchModeChannel1 == true){
				DigitalJockey2Controller.scratchModeChannel1 = false;
				midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff);
			}
			else{
				DigitalJockey2Controller.scratchModeChannel1 = true;
				midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);
			}
		}
		if(channel == 2){
		    //print("Scratch 2: " + DigitalJockey2Controller.scratchModeChannel2);
			if(DigitalJockey2Controller.scratchModeChannel2 == true){
				DigitalJockey2Controller.scratchModeChannel2 = false;
				midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff);
			}
			else{
				DigitalJockey2Controller.scratchModeChannel2 = true;
				midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);		
			}
		}
	}
}

DigitalJockey2Controller.Search1 = function (channel, control, value, status, group){
    if(value == DigitalJockey2Controller.keyPressed){
	    if(DigitalJockey2Controller.searchModeChannel1 == false){
	        DigitalJockey2Controller.searchModeChannel1 = true;
		    midi.sendShortMsg(0x90, 0x1A, DigitalJockey2Controller.ledOn); //Turn LED on
	    }
	    else{
	        DigitalJockey2Controller.searchModeChannel1 = false;
		    midi.sendShortMsg(0x90, 0x1A, DigitalJockey2Controller.ledOff); //Turn LED off
	    }
	    //print("Search 1: " + DigitalJockey2Controller.searchModeChannel1);
	}
}
DigitalJockey2Controller.Search2 = function (channel, control, value, status, group){
    if(value == DigitalJockey2Controller.keyPressed){
	    if(DigitalJockey2Controller.searchModeChannel2 == false){
	        DigitalJockey2Controller.searchModeChannel2 = true;
		    midi.sendShortMsg(0x90, 0x56, DigitalJockey2Controller.ledOn); //Turn LED on
	    }
	    else{
	        DigitalJockey2Controller.searchModeChannel2 = false;
		    midi.sendShortMsg(0x90, 0x56, DigitalJockey2Controller.ledOff); //Turn LED off
	    }
	    //print("Search 2: " + DigitalJockey2Controller.searchModeChannel2);
	}
}

DigitalJockey2Controller.JogWheelHelper = function (channel, control, value){
	//print("control: " + control);
	//print("value: " + value);
	if (control == 29) {
		if (DigitalJockey2Controller.scratchModeChannel1 == true && DigitalJockey2Controller.searchModeChannel1 == true) {
			var currentlyPlaying = engine.getValue("[Channel1]","play");
			if (currentlyPlaying) {
				if (value == 0) {
					if (engine.getValue("[Channel1]", "rate_temp_up_small"))
					    engine.setValue("[Channel1]", "rate_temp_up_small", 0);
					if (engine.getValue("[Channel1]", "rate_temp_down_small"))
					    engine.setValue("[Channel1]", "rate_temp_down_small", 0);
				}
			}
			else {
				if (value == 127)
					engine.scratchEnable(1, 128, 33+1/3, 1.0/8, (1.0/8)/32);
				else if (value == 0)
					engine.scratchDisable(1);
			}
		}
		else if (DigitalJockey2Controller.scratchModeChannel1 == true && DigitalJockey2Controller.searchModeChannel1 == false) {
			if (value == 127)
				engine.scratchEnable(1, 128, 33+1/3, 1.0/8, (1.0/8)/32);
			else if (value == 0)
				engine.scratchDisable(1);
		}
		else if (DigitalJockey2Controller.scratchModeChannel1 == false && DigitalJockey2Controller.searchModeChannel1 == true) {
			if (value == 0) {
				if (engine.getValue("[Channel1]", "fwd"))
				    engine.setValue("[Channel1]", "fwd", 0);
				if (engine.getValue("[Channel1]", "back"))
				    engine.setValue("[Channel1]", "back", 0);
			}
		}
	}
	else if (control == 89) {
		if (DigitalJockey2Controller.scratchModeChannel2 == true && DigitalJockey2Controller.searchModeChannel2 == true) {
			var currentlyPlaying = engine.getValue("[Channel2]","play");
			if (currentlyPlaying) {
				if (value == 0) {
					if (engine.getValue("[Channel2]", "rate_temp_up_small"))
					    engine.setValue("[Channel2]", "rate_temp_up_small", 0);
					if (engine.getValue("[Channel2]", "rate_temp_down_small"))
					    engine.setValue("[Channel2]", "rate_temp_down_small", 0);
				}
			}
			else {
				if (value == 127)
					engine.scratchEnable(2, 128, 33+1/3, 1.0/8, (1.0/8)/32);
				else if (value == 0)
					engine.scratchDisable(2);
			}
		}
		else if (DigitalJockey2Controller.scratchModeChannel2 == true && DigitalJockey2Controller.searchModeChannel2 == false) {
			if (value == 127)
				engine.scratchEnable(2, 128, 33+1/3, 1.0/8, (1.0/8)/32);
			else if (value == 0)
				engine.scratchDisable(2);
		}
		else if (DigitalJockey2Controller.scratchModeChannel2 == false && DigitalJockey2Controller.searchModeChannel2 == true) {
			if (value == 0) {
				if (engine.getValue("[Channel2]", "fwd"))
				    engine.setValue("[Channel2]", "fwd", 0);
				if (engine.getValue("[Channel2]", "back"))
				    engine.setValue("[Channel2]", "back", 0);
			}
		}
	}
}

DigitalJockey2Controller.JogWheel1 = function (channel, control, value){
	DigitalJockey2Controller.JogWheel(1, control, value);
}

DigitalJockey2Controller.JogWheel2 = function (channel, control, value){
	DigitalJockey2Controller.JogWheel(2, control, value);
}

DigitalJockey2Controller.PitchBendSensitivity = 0.00001;
DigitalJockey2Controller.SearchSensitivity = 0.01;

DigitalJockey2Controller.JogWheel = function (channel, control, value){
	/*
	 * The JogWheels of the controler work as follows.
	 * Spinning around in reverse order produces decimal values of 63 or lower
	 * depending on the the speed you drag the wheel.
	 * 
	 * Spinning around in a forward manner produces values of 65 or higher.
	 */
	var jogValue = (value - 64); //DigitalJockey2Controller.WheelSensitivity;
	
	//Functionality of Jog Wheel if we're in scratch mode 
	if(channel == 1){
	    if (DigitalJockey2Controller.scratchModeChannel1 == true && DigitalJockey2Controller.searchModeChannel1 == true) {
	        var currentlyPlaying = engine.getValue("[Channel1]","play");
	        if (currentlyPlaying) {
		    var rtu = engine.getValue("[Channel1]", "rate_temp_up_small");
		    var rtd = engine.getValue("[Channel1]", "rate_temp_down_small");
	            if (jogValue > 0) {
	                if (rtd)
			    engine.setValue("[Channel1]", "rate_temp_down_small", 0);
			if (rtu != 1)
			    engine.setValue("[Channel1]", "rate_temp_up_small", 1);
	                
	            }
	            else if (jogValue < 0) {
	                if (rtu)
			    engine.setValue("[Channel1]", "rate_temp_up_small", 0);
	                if (rtd != 1)
			    engine.setValue("[Channel1]", "rate_temp_down_small", 1);
	            }
	            else
	                return;
	        }
	        else {
	            engine.scratchTick(1,jogValue);
	        }
	    }
	    else if (DigitalJockey2Controller.scratchModeChannel1 == true && DigitalJockey2Controller.searchModeChannel1 == false) {
		    engine.scratchTick(1,jogValue);
	    }
	    else if (DigitalJockey2Controller.scratchModeChannel1 == false && DigitalJockey2Controller.searchModeChannel1 == true) {
		var currentlyPlaying = engine.getValue("[Channel1]","play");
	        if (currentlyPlaying) {
		    if (jogValue == 0)
			return;
		    var fwd = engine.getValue("[Channel1]", "fwd");
		    var back = engine.getValue("[Channel1]", "back");
		    if (jogValue > 0) {
			if (back)
			    engine.setValue("[Channel1]", "back", 0);
			if (fwd != 1)
			    engine.setValue("[Channel1]", "fwd", 1);
		    }
		    else if (jogValue < 0) {
			if (fwd)
			    engine.setValue("[Channel1]", "fwd", 0);
			if (back != 1)
			    engine.setValue("[Channel1]", "back", 1);
		    }
		}
		else {
		    var playpos = engine.getValue("[Channel1]", "playposition");
		    if (jogValue > 0) {
		        if (playpos < 1 - DigitalJockey2Controller.SearchSensitivity)
		            playpos += DigitalJockey2Controller.SearchSensitivity;
		        else
		            playpos = 1;
		    }
		    else if (jogValue < 0) {
			if (playpos > DigitalJockey2Controller.SearchSensitivity)
			    playpos -= DigitalJockey2Controller.SearchSensitivity;
			else
			    playpos = 0;
		    }
		    engine.setValue("[Channel1]", "playposition", playpos);
		}
	    }		
	}
	if(channel == 2){
	    if (DigitalJockey2Controller.scratchModeChannel2 == true && DigitalJockey2Controller.searchModeChannel2 == true) {
	        var currentlyPlaying = engine.getValue("[Channel2]","play");
	        if (currentlyPlaying) {
	            var rtu = engine.getValue("[Channel2]", "rate_temp_up_small");
		    var rtd = engine.getValue("[Channel2]", "rate_temp_down_small");
	            if (jogValue > 0) {
	                if (rtd)
			    engine.setValue("[Channel2]", "rate_temp_down_small", 0);
			if (rtu != 1)
			    engine.setValue("[Channel2]", "rate_temp_up_small", 1);
	                
	            }
	            else if (jogValue < 0) {
	                if (rtu)
			    engine.setValue("[Channel2]", "rate_temp_up_small", 0);
	                if (rtd != 1)
			    engine.setValue("[Channel2]", "rate_temp_down_small", 1);
	            }
	            else
	                return;
	        }
	        else {
	            engine.scratchTick(2,jogValue);
	        }
	    }
	    else if (DigitalJockey2Controller.scratchModeChannel2 == true && DigitalJockey2Controller.searchModeChannel2 == false) {
		    engine.scratchTick(2,jogValue);
	    }
	    else if (DigitalJockey2Controller.scratchModeChannel2 == false && DigitalJockey2Controller.searchModeChannel2 == true) {
		var currentlyPlaying = engine.getValue("[Channel2]","play");
	        if (currentlyPlaying) {
		    if (jogValue == 0)
			return;
		    var fwd = engine.getValue("[Channel2]", "fwd");
		    var back = engine.getValue("[Channel2]", "back");
		    if (jogValue > 0) {
			if (back)
			    engine.setValue("[Channel2]", "back", 0);
			if (fwd != 1)
			    engine.setValue("[Channel2]", "fwd", 1);
		    }
		    else if (jogValue < 0) {
			if (fwd)
			    engine.setValue("[Channel2]", "fwd", 0);
			if (back != 1)
			    engine.setValue("[Channel2]", "back", 1);
		    }
		}
		else {
		    var playpos = engine.getValue("[Channel2]", "playposition");
		    if (jogValue > 0) {
		        if (playpos < 1 - DigitalJockey2Controller.SearchSensitivity)
		            playpos += DigitalJockey2Controller.SearchSensitivity;
		        else
		            playpos = 1;
		    }
		    else if (jogValue < 0) {
			if (playpos > DigitalJockey2Controller.SearchSensitivity)
			    playpos -= DigitalJockey2Controller.SearchSensitivity;
			else
			    playpos = 0;
		    }
		    engine.setValue("[Channel2]", "playposition", playpos);
		}
	    }		
	}
}

DigitalJockey2Controller.JogWheel1_Hold = function (channel, control, value){
	
}

DigitalJockey2Controller.JogWheel2_Hold = function (channel, control, value){
	
}
/*****************************************************
 * Put functions here to handle controlobjets functions
 ******************************************************/
DigitalJockey2Controller.isChannel1_Playing = function (value){
		if(value == 0){
			midi.sendShortMsg(0x90, 0x19, DigitalJockey2Controller.ledOff);   // Turn on the Play LED1 off
			//midi.sendShortMsg(0x90, 0x17, DigitalJockey2Controller.ledOff); //Turn CUP LED1 off
			midi.sendShortMsg(0x90, 0x18, DigitalJockey2Controller.ledOff); //Turn CUE LED1 off
		}
		else{ //if deck is playing but not in CUE modus
			if(DigitalJockey2Controller.CUP_Button1_IsActive == false){
				midi.sendShortMsg(0x90, 0x19, DigitalJockey2Controller.ledOn);   // Turn on the Play LED1 on
			}
		}	
}
DigitalJockey2Controller.isChannel2_Playing = function (value){
		if(value == 0){
			midi.sendShortMsg(0x90, 0x55, DigitalJockey2Controller.ledOff);   // Turn on the Play LED2 off
			//midi.sendShortMsg(0x90, 0x53, DigitalJockey2Controller.ledOff);  //Turn CUP LED2 off
			midi.sendShortMsg(0x90, 0x54, DigitalJockey2Controller.ledOff); //Turn CUE LED2 off
		}
		else{
			if(DigitalJockey2Controller.CUP_Button2_IsActive == false)
				midi.sendShortMsg(0x90, 0x55, DigitalJockey2Controller.ledOn);   // Turn on the Play LED2 on
		}	
}
DigitalJockey2Controller.isChannel1_Cue_Active = function (value){
	if(value == 0){
		if(DigitalJockey2Controller.CUP_Button1_IsActive == true)
			midi.sendShortMsg(0x90, 0x17, DigitalJockey2Controller.ledOn); //Turn CUP LED1 on
		midi.sendShortMsg(0x90, 0x18, DigitalJockey2Controller.ledOff); //Turn CUE LED1 off
	}
	else{
		//if CUP LED is active leave, we can switch off CUE Botton
		if(DigitalJockey2Controller.CUP_Button1_IsActive == true){
			midi.sendShortMsg(0x90, 0x18, DigitalJockey2Controller.ledOff); //Turn CUE LED1 off
			midi.sendShortMsg(0x90, 0x17, DigitalJockey2Controller.ledOn); // Turn CUP LED1 on
		}
		else
			midi.sendShortMsg(0x90, 0x18, DigitalJockey2Controller.ledOn); //Turn CUE LED1 on
		
	}
}
DigitalJockey2Controller.isChannel2_Cue_Active = function (value){
	
	if(value == 0){
		if(DigitalJockey2Controller.CUP_Button2_IsActive == true)
			midi.sendShortMsg(0x90, 0x53, DigitalJockey2Controller.ledOn);  //Turn CUP LED2 on
		midi.sendShortMsg(0x90, 0x54, DigitalJockey2Controller.ledOff); //Turn CUE LED2 off
	}
	else{
		//if CUP LED is active leave, we can switch off CUE Botton
		if(DigitalJockey2Controller.CUP_Button2_IsActive == true){
			midi.sendShortMsg(0x90, 0x54, DigitalJockey2Controller.ledOff); //Turn CUE LED2 off
			midi.sendShortMsg(0x90, 0x53, DigitalJockey2Controller.ledOn);  //Turn CUP LED2 on
		}
		else
			midi.sendShortMsg(0x90, 0x54, DigitalJockey2Controller.ledOn); //Turn CUE LED2 on
		
	}
}
DigitalJockey2Controller.SelectNextTrack_or_prevTrack = function (channel, control, value, status){
	if(value == 65)
		engine.setValue("[Playlist]","SelectNextTrack",1);
	else
		engine.setValue("[Playlist]","SelectPrevTrack",1);
	
}
/*
 * Toggles LED status light on/off if you press kill buttons through Mixxx
 */
DigitalJockey2Controller.OnFilterHigh_KillButton2 = function (value){
	if(value == 1)
		midi.sendShortMsg(0x90, 0x50,DigitalJockey2Controller.ledOn); //HighFilterKill
	else
		midi.sendShortMsg(0x90, 0x50,DigitalJockey2Controller.ledOff); //HighFilterKill
}
DigitalJockey2Controller.OnFilterMid_KillButton2 = function (value){
	if(value == 1)
		midi.sendShortMsg(0x90, 0x51, DigitalJockey2Controller.ledOn); //MidFilterKill
	else
		midi.sendShortMsg(0x90, 0x51, DigitalJockey2Controller.ledOff); //MidFilterKill
}
DigitalJockey2Controller.OnFilterLow_KillButton2 = function (value){
	if(value == 1)
		midi.sendShortMsg(0x90, 0x52, DigitalJockey2Controller.ledOn); //LowFilterKill
	else
		midi.sendShortMsg(0x90, 0x52, DigitalJockey2Controller.ledOff); //LowFilterKill
}

DigitalJockey2Controller.OnFilterHigh_KillButton1 = function (value){
	if(value == 1)
		midi.sendShortMsg(0x90, 0x14, DigitalJockey2Controller.ledOn); //HighFilterKill
	else
		midi.sendShortMsg(0x90, 0x14, DigitalJockey2Controller.ledOff); //HighFilterKill
}
DigitalJockey2Controller.OnFilterMid_KillButton1 = function (value){
	if(value == 1)
		midi.sendShortMsg(0x90, 0x15, DigitalJockey2Controller.ledOn); //HighFilterKill
	else
		midi.sendShortMsg(0x90, 0x15, DigitalJockey2Controller.ledOff); //HighFilterKill
}
DigitalJockey2Controller.OnFilterLow_KillButton1 = function (value){
	if(value == 1)
		midi.sendShortMsg(0x90, 0x16, DigitalJockey2Controller.ledOn); //HighFilterKill
	else
		midi.sendShortMsg(0x90, 0x16, DigitalJockey2Controller.ledOff); //HighFilterKill
}
DigitalJockey2Controller.OnPFL_Button1 = function (value){
	if(value == 1){
			midi.sendShortMsg(0x90, 0x05, DigitalJockey2Controller.ledOn); //Turn LED off
		}
		else{
			midi.sendShortMsg(0x90, 0x05, DigitalJockey2Controller.ledOff); //Turn LED off
		}
}
DigitalJockey2Controller.OnPFL_Button2 = function (value){
	if(value == 1){
			midi.sendShortMsg(0x90, 0x41, DigitalJockey2Controller.ledOn); //Turn LED off
		}
		else{
			midi.sendShortMsg(0x90, 0x41, DigitalJockey2Controller.ledOff); //Turn LED off
		}
}
DigitalJockey2Controller.LoopIn = function (channel, control, value, status, group) {
	if(value == DigitalJockey2Controller.keyPressed){
		midi.sendShortMsg(status, control, DigitalJockey2Controller.ledOn); //Turn LED on
		engine.setValue(group,"loop_in",1);
	}
	else{
		midi.sendShortMsg(status, control, DigitalJockey2Controller.ledOff); //Turn LED off
	}
}
DigitalJockey2Controller.LoopOut = function (channel, control, value, status, group) {
	if(value == DigitalJockey2Controller.keyPressed){
		midi.sendShortMsg(status, control, DigitalJockey2Controller.ledOn); //Turn LED on
		engine.setValue(group,"loop_out",1);
	}
	else{
		midi.sendShortMsg(status, control, DigitalJockey2Controller.ledOff); //Turn LED off
	}
}
DigitalJockey2Controller.ReloopExit = function (channel, control, value, status, group){
	//if loop is active, we exit the loop
	if(engine.getValue(group,"loop_enabled")){
		engine.setValue(group,"reloop_exit",1);
	}
	else{
		engine.setValue(group,"reloop_exit",0);
	}
	var actBeatLoop = engine.getValue(group, "beatloop");
	print("ReloopExit.actBeatLoop=" + actBeatLoop);
	if (actBeatLoop != "0") {
		print("Set BeatLoop To Zero");
		engine.setValue(group,"beatloop","0");
	}
}
DigitalJockey2Controller.BeatLoop = function (channel, control, value, status, group) {
	if(value == DigitalJockey2Controller.keyPressed){
		midi.sendShortMsg(status, control, DigitalJockey2Controller.ledOn); //Turn LED on
		engine.setValue(group,"beatloop","4");
	}
	else{
		midi.sendShortMsg(status, control, DigitalJockey2Controller.ledOff); //Turn LED off
	}
}
DigitalJockey2Controller.LoopActiveLED1 = function (value){
	//if loop is active, we exit the loop
	if(value == 1){
		midi.sendShortMsg(0x90, 0x12, DigitalJockey2Controller.ledOn); //Turn LED on
	}
	else{
		midi.sendShortMsg(0x90, 0x12, DigitalJockey2Controller.ledOff); //Turn LED off
	}
	
}
DigitalJockey2Controller.LoopActiveLED2 = function (value){
	//if loop is active, we exit the loop
	if(value == 1){
		midi.sendShortMsg(0x90, 0x4E, DigitalJockey2Controller.ledOn); //Turn LED on
	}
	else{
		midi.sendShortMsg(0x90, 0x4E, DigitalJockey2Controller.ledOff); //Turn LED off
	}
	
}

DigitalJockey2Controller.Flanger1 = function (channel, control, value, status, group){
    if(value == DigitalJockey2Controller.keyPressed){
        var flanger = engine.getValue("[Channel1]", "flanger");
	    if(flanger){
	        engine.setValue("[Channel1]", "flanger", 0);
	    }
	    else{
	        engine.setValue("[Channel1]", "flanger", 1);
	    }
	}
}
DigitalJockey2Controller.Flanger2 = function (channel, control, value, status, group){
    if(value == DigitalJockey2Controller.keyPressed){
        var flanger = engine.getValue("[Channel2]", "flanger");
	    if(flanger){
	        engine.setValue("[Channel2]", "flanger", 0);
	    }
	    else{
	        engine.setValue("[Channel2]", "flanger", 1);
	    }
	}
}

DigitalJockey2Controller.LoadSelectedTrack = function (channel, control, value) {
	/* added by Jones */
	if (control == 19) {
		if(engine.getValue("[Channel1]", "play") == 0)
			engine.setValue("[Channel1]", "LoadSelectedTrack", 1);
		else
			print("no load to deck1!\n");
	}
	else if (control == 79) {
		if(engine.getValue("[Channel2]", "play") == 0)
			engine.setValue("[Channel2]", "LoadSelectedTrack", 1);
		else
			print("no load to deck2!\n");	
	}
}

DigitalJockey2Controller.PitchControl = function (channel, control, value) {
	var pitch_value = -1.0 + 2.0 * ((value * 8 + control / 16) / 1024.0);
	//print("pitch_value: " + pitch_value);
	if (channel == 0) {
		engine.setValue("[Channel1]","rate", pitch_value);
	}
	else if (channel == 1) {
		engine.setValue("[Channel2]","rate", pitch_value);
	}
}

DigitalJockey2Controller.KeyLock1 = function (channel, control, value) {
	DigitalJockey2Controller.KeyLock(1, control, value);	
}
DigitalJockey2Controller.KeyLock2 = function (channel, control, value) {
	DigitalJockey2Controller.KeyLock(2, control, value);	
}
DigitalJockey2Controller.KeyLock = function (channel, control, value) {
	var isKeyLock = engine.getValue("[Channel"+channel+"]","keylock");
	if(value == DigitalJockey2Controller.keyPressed){
		if(isKeyLock == 1){
			engine.setValue("[Channel"+channel+"]","keylock",0);
		}
		else{
			engine.setValue("[Channel"+channel+"]","keylock",1);
		}
	}
}

DigitalJockey2Controller.OnKeyLockChange1 = function(value){
	if(value == 1){
		midi.sendShortMsg(0x90, 0x02, DigitalJockey2Controller.ledOn); //Turn Keylock on
	}
	else{
		midi.sendShortMsg(0x90, 0x02, DigitalJockey2Controller.ledOff); //Turn KeyLock off
	}
}
DigitalJockey2Controller.OnKeyLockChange2 = function(value){
	if(value == 1){
		midi.sendShortMsg(0x90, 0x3E, DigitalJockey2Controller.ledOn); //Turn KeyLock on
	}
	else{
		midi.sendShortMsg(0x90, 0x3E, DigitalJockey2Controller.ledOff); //Turn KeyLock off
	}
}

DigitalJockey2Controller.OnFlangerChange1 = function(value){
	if(value == 1){
		midi.sendShortMsg(0x90, 0x0e, DigitalJockey2Controller.ledOn); //Turn LED on
	}
	else{
		midi.sendShortMsg(0x90, 0x0e, DigitalJockey2Controller.ledOff); //Turn LED off
	}
}
DigitalJockey2Controller.OnFlangerChange2 = function(value){
	if(value == 1){
		midi.sendShortMsg(0x90, 0x45, DigitalJockey2Controller.ledOn); //Turn LED on
	}
	else{
		midi.sendShortMsg(0x90, 0x45, DigitalJockey2Controller.ledOff); //Turn LED off
	}
}

DigitalJockey2Controller.LoopPlusMinusChannel1 = function (channel, control, value, status, group) {
	DigitalJockey2Controller.LoopPlusMinus(channel, control, value, status, group);	
}
DigitalJockey2Controller.LoopPlusMinusChannel2 = function (channel, control, value, status, group) {
	DigitalJockey2Controller.LoopPlusMinus(channel, control, value, status, group);	
}

DigitalJockey2Controller.beatloops = ["0.125", "0.25", "0.5", "1", "2", "4", "8", "16"];

DigitalJockey2Controller.getPrevBeatLoop = function (beatloop) {
	var i = 0;
	for (i = 0; i < DigitalJockey2Controller.beatloops.length; i++) {
		if (DigitalJockey2Controller.beatloops[i] == beatloop) {
			if (i > 0)
				return DigitalJockey2Controller.beatloops[i - 1];
			else
				return DigitalJockey2Controller.beatloops[0];
		}
	}
	return "0";
}

DigitalJockey2Controller.getNextBeatLoop = function (beatloop) {
	var i = 0;
	for (i = 0; i < DigitalJockey2Controller.beatloops.length; i++) {
		if (DigitalJockey2Controller.beatloops[i] == beatloop) {
			if (i < DigitalJockey2Controller.beatloops.length - 1)
				return DigitalJockey2Controller.beatloops[i + 1];
			else
				return DigitalJockey2Controller.beatloops[DigitalJockey2Controller.beatloops.length - 1];
		}
	}
	return "0";
}

DigitalJockey2Controller.LoopPlusMinus = function (channel, control, value, status, group) {
	var actBeatLoop = engine.getValue(group, "beatloop");
	print("LoopPlusMinus.actBeatLoop=" + actBeatLoop);
	if (actBeatLoop != "0") {
		if (value == 0x3F) {
			var prevBeatLoop = DigitalJockey2Controller.getPrevBeatLoop(actBeatLoop);
			print("LoopPlusMinus.prevBeatLoop=" + prevBeatLoop);
			engine.setValue(group,"beatloop",prevBeatLoop);
		}
		else if (value == 0x41) {
			var nextBeatLoop = DigitalJockey2Controller.getNextBeatLoop(actBeatLoop);
			print("LoopPlusMinus.nextBeatLoop=" + nextBeatLoop );
			engine.setValue(group,"beatloop",nextBeatLoop);
		}
	}
}