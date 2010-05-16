/****************************************************************/
/*      Reloop Digital Jockey 2 controller script v 1.0         */
/*          Copyright (C) 2010, Tobias Rafreider  				*/
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 1.8.x                                 */
/****************************************************************/

function DigitalJockey2Controller() {}

DigitalJockey2Controller.ledOn = 0x7F;
DigitalJockey2Controller.ledOff = 0x00;
DigitalJockey2Controller.keyPressed = 0x7F;
DigitalJockey2Controller.keyUp = 0x00;

//Initial Jog Wheel positions
DigitalJockey2Controller.WheelSensitivity = 6; 

DigitalJockey2Controller.decayLast = new Date().getTime();
DigitalJockey2Controller.decayInterval = 50;
DigitalJockey2Controller.decayRate = 300;

DigitalJockey2Controller.scratchModeChannel1 = false;
DigitalJockey2Controller.scratchModeChannel2 = false;

//boolean value that indicated if CUP LED is active
DigitalJockey2Controller.CUP_Button1_IsActive = false;
DigitalJockey2Controller.CUP_Button2_IsActive = false;

DigitalJockey2Controller.init = function(id){
    print ("Initalizing Reloop Digital Jockey 2 Controler Edition.");
	DigitalJockey2Controller.resetLEDs();

	
	// Waveform speed handling if scratching is active
	engine.connectControl("[Channel1]","playposition","DigitalJockey2Controller.wheelDecay");
    engine.connectControl("[Channel2]","playposition","DigitalJockey2Controller.wheelDecay");
	
	engine.connectControl("[Channel1]","play","DigitalJockey2Controller.isChannel1_Playing");
    engine.connectControl("[Channel2]","play","DigitalJockey2Controller.isChannel2_Playing");
	
	engine.connectControl("[Channel1]","cue_default","DigitalJockey2Controller.isChannel1_Cue_Active");
	engine.connectControl("[Channel2]","cue_default","DigitalJockey2Controller.isChannel2_Cue_Active");
	
	engine.connectControl("[Channel2]","filterHighKill","DigitalJockey2Controller.OnFilterHigh_KillButton2");
	engine.connectControl("[Channel2]","filterLowKill","DigitalJockey2Controller.OnFilterLow_KillButton2");
	engine.connectControl("[Channel2]","filterMidKill","DigitalJockey2Controller.OnFilterMid_KillButton2");
	
	engine.connectControl("[Channel1]","filterHighKill","DigitalJockey2Controller.OnFilterHigh_KillButton1");
	engine.connectControl("[Channel1]","filterLowKill","DigitalJockey2Controller.OnFilterLow_KillButton1");
	engine.connectControl("[Channel1]","filterMidKill","DigitalJockey2Controller.OnFilterMid_KillButton1");
	
	engine.connectControl("[Channel1]","pfl","DigitalJockey2Controller.OnPFL_Button1");
	engine.connectControl("[Channel2]","pfl","DigitalJockey2Controller.OnPFL_Button2");
	
	//Looping
	engine.connectControl("[Channel1]","loop_enabled","DigitalJockey2Controller.LoopActiveLED1");
	engine.connectControl("[Channel2]","loop_enabled","DigitalJockey2Controller.LoopActiveLED2");
	

}
DigitalJockey2Controller.resetLEDs = function(){

	//Turn all LEDS off 
	midi.sendShortMsg(0x90, 0x19, DigitalJockey2Controller.ledOff);   // Turn on the Play LED1 off
	midi.sendShortMsg(0x90, 0x17, DigitalJockey2Controller.ledOff); //Turn CUP LED1 off
	midi.sendShortMsg(0x90, 0x18, DigitalJockey2Controller.ledOff); //Turn CUE LED1 off
	midi.sendShortMsg(0x90, 0x5, DigitalJockey2Controller.ledOff); //Turn PFL LED off
	midi.sendShortMsg(0x90, 0x14, DigitalJockey2Controller.ledOff); //HighFilterKill
	midi.sendShortMsg(0x90, 0x15, DigitalJockey2Controller.ledOff); //MidFilterKill
	midi.sendShortMsg(0x90, 0x16, DigitalJockey2Controller.ledOff); //LowFilterKill
	midi.sendShortMsg(0x90, 0x1B, DigitalJockey2Controller.ledOff); //disable scratch control
	
	midi.sendShortMsg(0x90, 0x55, DigitalJockey2Controller.ledOff);   // Turn on the Play LED2 off
	midi.sendShortMsg(0x90, 0x53, DigitalJockey2Controller.ledOff); //Turn CUP LED2 off
	midi.sendShortMsg(0x90, 0x54, DigitalJockey2Controller.ledOff); //Turn CUE LED2 off
	midi.sendShortMsg(0x90, 0x41, DigitalJockey2Controller.ledOff); //Turn PFL LED off
	midi.sendShortMsg(0x90, 0x50, DigitalJockey2Controller.ledOff); //HighFilterKill
	midi.sendShortMsg(0x90, 0x51, DigitalJockey2Controller.ledOff); //MidFilterKill
	midi.sendShortMsg(0x90, 0x52, DigitalJockey2Controller.ledOff); //LowFilterKill
	midi.sendShortMsg(0x90, 0x57, DigitalJockey2Controller.ledOff); //disable scratch control
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
	//If no song is loaded
	 if (engine.getValue("[Channel"+channel+"]", "duration") == 0) { 
			return; 
	};
	//If a CUP is active, PlayButtons are disabled
	var isCupActive = engine.getValue("[Channel"+channel+"]","cue_default");
	if(isCupActive == true)
		return;
		
	var currentlyPlaying = engine.getValue("[Channel"+channel+"]","play");
    /*
	 * We immediately want to start and stop playing as soon as play button1 has been pressed
	 * KeyUp events are out of interest in this case
	 */
	if(value == DigitalJockey2Controller.keyPressed){
		
		if (currentlyPlaying == 1) {    // If currently playing
			engine.setValue("[Channel"+channel+"]","play",0);    // Stop
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff);    // Turn off the Play LED
		}
		else {    // If not currently playing,
			engine.setValue("[Channel"+channel+"]","play",1);    // Start
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);    // Turn on the Play LED
		}
	}
}
 // Play button deck 1
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
	midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);
	// As soon as we press CUE, execute CUE Logic
	if(value == DigitalJockey2Controller.keyPressed){
		engine.setValue("[Channel"+channel+"]","cue_default",1);
		if(channel == 1) {
			midi.sendShortMsg(0x90, 0x19, DigitalJockey2Controller.ledOff);   // Turn on the Play LED off
			midi.sendShortMsg(0x90, 0x17, DigitalJockey2Controller.ledOff); //Turn CUP LED off
			DigitalJockey2Controller.CUP_Button1_IsActive = false;
		}
		if(channel == 2){
			midi.sendShortMsg(0x90, 0x55, DigitalJockey2Controller.ledOff);   // Turn on the Play LED off
			midi.sendShortMsg(0x90, 0x53, DigitalJockey2Controller.ledOff); //Turn CUP LED off
			DigitalJockey2Controller.CUP_Button2_IsActive = false;
		}
	
		//Turn CUE LED on
		midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);
		
	}
	if(value == DigitalJockey2Controller.keyUp){
		engine.setValue("[Channel"+channel+"]","cue_default",0);
		//TURN CUE LED OFF
		midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff);
		
	}
	
}
DigitalJockey2Controller.CuePlayButton1 = function (channel, control, value) {
	DigitalJockey2Controller.CuePlay(1, control, value);	
}
DigitalJockey2Controller.CuePlayButton2 = function (channel, control, value) {
	DigitalJockey2Controller.CuePlay(2, control, value);	
}
DigitalJockey2Controller.CuePlay = function (channel, control, value) {
	//If no song is loaded
	if (engine.getValue("[Channel"+channel+"]", "duration") == 0) { 
			return; 
	};
	var isCupActive = engine.getValue("[Channel"+channel+"]","cue_default");
	var currentlyPlaying = engine.getValue("[Channel"+channel+"]","play");
	
	// As soon as we press CUP, execute CUP Logic
	if(value == DigitalJockey2Controller.keyPressed){
		//If CUP is active, we disable and enable CUP in sequence as a user would do
		if(isCupActive == 1 || currentlyPlaying == 0){
			print ("isCUPActive" + isCupActive);
			print ("isPlaying" + currentlyPlaying);
			
			if(isCupActive == 1){	//diable CUP
				engine.setValue("[Channel"+channel+"]","cue_default",0);
				//Turn CUP LED off
				midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff);
				midi.sendShortMsg(0x90, 0x55, DigitalJockey2Controller.ledOff);   // Turn on the Play LED off
				if(channel == 1)
					DigitalJockey2Controller.CUP_Button1_IsActive = false;
				if(channel == 2)
					DigitalJockey2Controller.CUP_Button2_IsActive = false;
			}
			if(currentlyPlaying == 0){
				engine.setValue("[Channel"+channel+"]","cue_default",1);
				midi.sendShortMsg(0x90, 0x55, DigitalJockey2Controller.ledOff); // Turn on the Play LED off
				//Turn CUP LED on
				midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);
				if(channel == 1)
					DigitalJockey2Controller.CUP_Button1_IsActive = true;
				if(channel == 2)
					DigitalJockey2Controller.CUP_Button2_IsActive = true;
			}
		}
		else{
			//If track is playing, CUP = CUE
			engine.setValue("[Channel"+channel+"]","cue_default",1);
			engine.setValue("[Channel"+channel+"]","cue_default",0);
			engine.setValue("[Channel"+channel+"]","player",0);
			if(channel == 1) {
				midi.sendShortMsg(0x90, 0x19, DigitalJockey2Controller.ledOff);   // Turn on the Play LED off
				midi.sendShortMsg(0x90, 0x17, DigitalJockey2Controller.ledOff); //Turn CUP LED off
				DigitalJockey2Controller.CUP_Button1_IsActive = false;
			}
			if(channel == 2){
				midi.sendShortMsg(0x90, 0x55, DigitalJockey2Controller.ledOff);   // Turn on the Play LED off
				midi.sendShortMsg(0x90, 0x53, DigitalJockey2Controller.ledOff); //Turn CUP LED off
				DigitalJockey2Controller.CUP_Button2_IsActive = false;
			}
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
	if(value == DigitalJockey2Controller.keyPressed){
		if(isHeadPhoneActive == 1){
			engine.setValue("[Channel"+channel+"]","pfl",0);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff); //Turn LED off
		}
		else{
			engine.setValue("[Channel"+channel+"]","pfl",1);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn); //Turn LED off
		}
	}
}

DigitalJockey2Controller.BassKillChannel1 = function (channel, control, value){
	var deck = 1;
	if(value == DigitalJockey2Controller.keyPressed){
		var isKillButtonIsActive = engine.getValue("[Channel"+deck+"]","filterLowKill");
		if(isKillButtonIsActive == true){
			engine.setValue("[Channel"+deck+"]","filterLowKill",0);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff);
		}
		else{
			engine.setValue("[Channel"+deck+"]","filterLowKill",1);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);
		}
	}
}
DigitalJockey2Controller.MidKillChannel1 = function (channel, control, value){
	var deck = 1;
	if(value == DigitalJockey2Controller.keyPressed){
		var isKillButtonIsActive = engine.getValue("[Channel"+deck+"]","filterMidKill");
		if(isKillButtonIsActive == true){
			engine.setValue("[Channel"+deck+"]","filterMidKill",0);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff);
		}
		else{
			engine.setValue("[Channel"+deck+"]","filterMidKill",1);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);
		}
	}
}

DigitalJockey2Controller.HighKillChannel1 = function (channel, control, value){
	var deck = 1;
	if(value == DigitalJockey2Controller.keyPressed){
		var isKillButtonIsActive = engine.getValue("[Channel"+deck+"]","filterHighKill");
		if(isKillButtonIsActive == true){
			engine.setValue("[Channel"+deck+"]","filterHighKill",0);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff);
		}
		else{
			engine.setValue("[Channel"+deck+"]","filterHighKill",1);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);
		}
	}
}
DigitalJockey2Controller.BassKillChannel2 = function (channel, control, value){
	var deck = 2;
	if(value == DigitalJockey2Controller.keyPressed){
		var isKillButtonIsActive = engine.getValue("[Channel"+deck+"]","filterLowKill");
		if(isKillButtonIsActive == true){
			engine.setValue("[Channel"+deck+"]","filterLowKill",0);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff);
		}
		else{
			engine.setValue("[Channel"+deck+"]","filterLowKill",1);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);
		}
	}
}
DigitalJockey2Controller.MidKillChannel2 = function (channel, control, value){
	var deck = 2;
	if(value == DigitalJockey2Controller.keyPressed){
		var isKillButtonIsActive = engine.getValue("[Channel"+deck+"]","filterMidKill");
		if(isKillButtonIsActive == true){
			engine.setValue("[Channel"+deck+"]","filterMidKill",0);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff);
		}
		else{
			engine.setValue("[Channel"+deck+"]","filterMidKill",1);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);
		}
	}
}
DigitalJockey2Controller.HighKillChannel2 = function (channel, control, value){
	var deck = 2;
	if(value == DigitalJockey2Controller.keyPressed){
		var isKillButtonIsActive = engine.getValue("[Channel"+deck+"]","filterHighKill");
		if(isKillButtonIsActive == true){
			engine.setValue("[Channel"+deck+"]","filterHighKill",0);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff);
		}
		else{
			engine.setValue("[Channel"+deck+"]","filterHighKill",1);
			midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);
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
		print ("Sratch 1: " + DigitalJockey2Controller.scratchModeChannel1);
		print ("Sratch 2: " + DigitalJockey2Controller.scratchModeChannel2);
		if(channel == 1){
			if(DigitalJockey2Controller.scratchModeChannel1 == true){
				DigitalJockey2Controller.scratchModeChannel1 = false;
				scratch.disable(channel);
				midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff);
			}
			else{
				DigitalJockey2Controller.scratchModeChannel1 = true;
				scratch.enable(channel);
				midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);
			}
		}
		if( channel == 2){
			if(DigitalJockey2Controller.scratchModeChannel2 == true){
				DigitalJockey2Controller.scratchModeChannel2 = false;
				scratch.disable(channel);
				midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOff);
			}
			else{
				DigitalJockey2Controller.scratchModeChannel2 = true;
				scratch.enable(channel);
				midi.sendShortMsg(0x90, control, DigitalJockey2Controller.ledOn);		
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
DigitalJockey2Controller.JogWheel = function (channel, control, value){
	/*
	 * The JogWheels of the controler work as follows.
	 * Spinning around in reverse order produces decimal values of 63 or lower
	 * depending on the the speed you drag the wheel.
	 * 
	 * Spinning around in a forward manner produces values of 65 or higher.
	 */
	var jogValue = (value - 64)/DigitalJockey2Controller.WheelSensitivity;
	
	//Functionality of Jog Wheel if we're in scratch mode 
	if(channel == 1){
		if (DigitalJockey2Controller.scratchModeChannel1 == true) {
			engine.setValue("[Channel"+channel+"]","scratch", (engine.getValue("[Channel"+channel+"]","scratch") + jogValue).toFixed(2));
		}		
	}
	if(channel == 2){
		if (DigitalJockey2Controller.scratchModeChannel2 == true) {
			engine.setValue("[Channel"+channel+"]","scratch", (engine.getValue("[Channel"+channel+"]","scratch") + jogValue).toFixed(2));
		}		
	}
}
DigitalJockey2Controller.JogWheel1_Hold = function (channel, control, value){
	DigitalJockey2Controller.JogWheel_Hold(1, control, value);
}

DigitalJockey2Controller.JogWheel2_Hold = function (channel, control, value){
	DigitalJockey2Controller.JogWheel_Hold(2, control, value);
}
DigitalJockey2Controller.JogWheel_Hold = function (channel, control, value){
	//nothing to do here
}

//For scratching
DigitalJockey2Controller.wheelDecay = function (value) {    
   // engine.getValue("[Channel1]","play")

    var currentDate = new Date().getTime();
    // print(currentDate);
    if (currentDate > DigitalJockey2Controller.decayLast + DigitalJockey2Controller.decayInterval) {
		DigitalJockey2Controller.decayLast = currentDate;
	   
       if (DigitalJockey2Controller.scratchModeChannel1 || DigitalJockey2Controller.scratchModeChannel2) { // do some scratching
			if (DigitalJockey2Controller.debug) print("Scratch deck1: " + engine.getValue("[Channel1]","scratch") + " deck2: "+ engine.getValue("[Channel2]","scratch"));
			// print("do scratching " + jogValue);
			// engine.setValue(group,"scratch", jogValue); // /64);

			var jog1DecayRate = DigitalJockey2Controller.decayRate * (engine.getValue("[Channel1]","play") ? 1 : 5);
			var jog1 = engine.getValue("[Channel1]","scratch"); 
			if (jog1 != 0) {
				if (Math.abs(jog1) > jog1DecayRate) {  
					engine.setValue("[Channel1]","scratch", (jog1 / jog1DecayRate).toFixed(2));
				} 
				else {
               engine.setValue("[Channel1]","scratch", 0);
            }
         }
	 var jog2DecayRate = DigitalJockey2Controller.decayRate * (engine.getValue("[Channel2]","play") ? 1 : 5);
         var jog2 = engine.getValue("[Channel2]","scratch"); 
	  if (jog2 != 0) {
	     if (Math.abs(jog2) > jog2DecayRate) {  
                engine.setValue("[Channel2]","scratch", (jog2 / jog2DecayRate).toFixed(2));
             } else {
                engine.setValue("[Channel2]","scratch", 0);
             }
          }
      } 
    }
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
			if( engine.getValue("[Channel1]","cue_default") == 0){
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
			if( engine.getValue("[Channel2]","cue_default") == 0)
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
			midi.sendShortMsg(0x90, 0x5, DigitalJockey2Controller.ledOn); //Turn LED off
		}
		else{
			midi.sendShortMsg(0x90, 0x5, DigitalJockey2Controller.ledOff); //Turn LED off
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
		midi.sendShortMsg(status, control, DigitalJockey2Controller.ledOff); //Turn LED on
	}
}
DigitalJockey2Controller.LoopOut = function (channel, control, value, status, group) {
	if(value == DigitalJockey2Controller.keyPressed){
		midi.sendShortMsg(status, control, DigitalJockey2Controller.ledOn); //Turn LED on
		engine.setValue(group,"loop_out",1);
	}
	else{
		midi.sendShortMsg(status, control, DigitalJockey2Controller.ledOff); //Turn LED on
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
}
DigitalJockey2Controller.LoopActiveLED1 = function (value){
	//if loop is active, we exit the loop
	if(value == 1){
		midi.sendShortMsg(0x90, 0x12, DigitalJockey2Controller.ledOn); //Turn LED on
	}
	else{
		midi.sendShortMsg(0x90, 0x12, DigitalJockey2Controller.ledOff); //Turn LED on
	}
	
}
DigitalJockey2Controller.LoopActiveLED2 = function (value){
	//if loop is active, we exit the loop
	if(value == 1){
		midi.sendShortMsg(0x90, 0x4E, DigitalJockey2Controller.ledOn); //Turn LED on
	}
	else{
		midi.sendShortMsg(0x90, 0x4E, DigitalJockey2Controller.ledOff); //Turn LED on
	}
	
}