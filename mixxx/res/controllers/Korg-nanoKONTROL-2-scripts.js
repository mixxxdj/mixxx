function NK2() {}
NK2.debug=0;//set debug level
NK2.LEDflasheson=true;//false disables led flash timers for debugging purposes
NK2.numDecks=8;//set to 8 for all [Channels], to 4 for 4 [Channels] and 4 [Samplers], and to 2 for 2 [Channels] and 6 [Samplers]
//############################################################################
//defaults
//############################################################################

NK2.midiChannel=0xB0;
NK2.curNSMR="N";//default bank (NSMR refers to SMR buttons and "no button")
NK2.curDeck=0;//current deck (for bank selects) -- defaults to 0 = off - no deck selected
NK2.bankSelectState=0;//current state of marker set button (deck selector button - press marker set, hold, press S M or R button on any track to select that bank) -- future: marker set could also select left buttons, transport, etc for more banks
NK2.cycleState=0;//current state of cycle toggle button
NK2.MOD1state=0;
NK2.MOD2state=0;
NK2.LEDBankIndicator=false;
NK2.loopmove=0.0125;//how far to move loops with loop move functions
NK2.lastwavevalue=0;//used for wave zoom function
NK2.cueLoopLen=2;//default length for cueloops (jump to cue and start loop)
NK2.cueMoveToNum=-1;//used in cue button moves (cueclear function)

//############################################################################
//references
//############################################################################
 
NK2.MODcodes={"0000":0,"0001":1,"0010":2,"0011":3,"0100":4,"0101":5,"0110":6,"0111":7,"1000":8,"1001":9,"1010":10,"1011":11,"1100":12,"1101":13,"1110":14,"1111":15};//list of mod states
NK2.Banks=new Array("N", "S", "M", "R");

//list controls
NK2.Knob={1:0x10,2:0x11,3:0x12,4:0x13,5:0x14,6:0x15,7:0x16,8:0x17};
NK2.Sbutton={1:0x20,2:0x21,3:0x22,4:0x23,5:0x24,6:0x25,7:0x26,8:0x27};
NK2.Mbutton={1:0x30,2:0x31,3:0x32,4:0x33,5:0x34,6:0x35,7:0x36,8:0x37};
NK2.Rbutton={1:0x40,2:0x41,3:0x42,4:0x43,5:0x44,6:0x45,7:0x46,8:0x47};
NK2.leftButton={"trdown":0x3A,"trup":0x3B,"cycle":0x2E,"mset":0x3C,"mdown":0x3D,"mup":0x3E,"rev":0x2B,"ff":0x2C,"stop":0x2A,"play":0x29,"rec":0x2D};

NK2.faders={0x00:1,0x01:2,0x02:3,0x03:4,0x04:5,0x05:6,0x06:7,0x07:8};

//initialize decks
if (NK2.numDecks==8){
	NK2.Deck={1:"[Channel1]",2:"[Channel2]",3:"[Channel3]",4:"[Channel4]",5:"[Channel5]",6:"[Channel6]",7:"[Channel7]",8:"[Channel8]"};//list of decks, applied to each strip - 8 strips, 8 decks.  (8 decks, 0 samplers)
}else if (NK2.numDecks==4){
	NK2.Deck={1:"[Channel1]",2:"[Channel2]",3:"[Channel3]",4:"[Channel4]",5:"[Sampler1]",6:"[Sampler2]",7:"[Sampler3]",8:"[Sampler4]"};//list of decks, applied to each strip - 8 strips, 8 decks.  (4 decks, 4 samplers)
}else if (NK2.numDecks==2){
	NK2.Deck={1:"[Channel1]",2:"[Channel2]",3:"[Sampler1]",4:"[Sampler2]",5:"[Sampler3]",6:"[Sampler4]",5:"[Sampler5]",6:"[Sampler6]"};//list of decks, applied to each strip - 8 strips, 8 decks.  (2 decks, 6 samplers)
}


NK2.deckData={};//object to store deck data, like mute vols, etc.
NK2.deckData["[Master]"]={'muteVol':-1};
for (i=1; i<9; i++){//initialize Deck reference objects
	NK2.deckData[NK2.Deck[i]]={};
	NK2.deckData[NK2.Deck[i]]['deckNum']=i;//reverse lookup for deck numbers by name
	NK2.deckData[NK2.Deck[i]]['muteVol']=-1;//store previous volume for mute functions
}

NK2.beatloopLengths=new Array(0.03125,0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32);//store loop lengths with presets in Mixxx

//############################################################################
//INIT & SHUTDOWN
//############################################################################

NK2.init = function init() { // called when the device is opened & set up
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	engine.setValue("[Master]", "num_decks", NK2.numDecks);
	NK2.setup()
	
	NK2.updateLEDs();
	print("decks: "+engine.getValue("[Master]", "num_decks"))
	};

NK2.shutdown = function shutdown() {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	
	engine.stopTimer(NK2.LEDtimer);
	};
	
//############################################################################
//SCENE SET FUNCTIONS - configure controls for selected scenes
//############################################################################

NK2.bankSelect = function bankSelect(deck, bank) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	NK2.curNSMR=bank;
	NK2.curDeck=deck;
	NK2.doClearBank=false;//bank has just been selected.  Don't clear the bank when bank select button is released
	NK2.cycleState=0;
	NK2.bankSelectState=0;
	
	//find bank indicator led, if bank activated
	if (NK2.curNSMR=="S"){
		NK2.LEDBankIndicator=NK2.Sbutton[NK2.curDeck];
	}else if (NK2.curNSMR=="M"){
		NK2.LEDBankIndicator=NK2.Mbutton[NK2.curDeck];
	}else if (NK2.curNSMR=="R"){
		NK2.LEDBankIndicator=NK2.Rbutton[NK2.curDeck];
	}
	if (NK2.LEDflasheson===true){engine.stopTimer(NK2.LEDtimer);NK2.LEDtimer=engine.beginTimer(750, "NK2.indicatorLEDs()");}//start timer for LED indicator flasher
	NK2.updateLEDs();
};

//############################################################################
//INPUT PROCESSING
//############################################################################


NK2.fader = function fader(channel, control, value, status, group) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	engine.setValue(NK2.Deck[NK2.faders[control]], "volume", value/128);
};

NK2.button = function button(channel, control, value, status, group) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	//input received - gatekeeper function connected to all buttons
 
	//get the control object for the current button on the current bank - associative array containing control info, LED state evaluators, press/release functions, etc.
	var theControl=NK2.getControl(control);
	if (theControl===false){return false;}//no control set for this button
	
	if (value>0){//button was pressed
		var command = theControl["pressEval"];
		midi.sendShortMsg(NK2.midiChannel, control, 0x7F);//light LED while button pressed
	}else{//button was released
		var command = theControl["releaseEval"];
		midi.sendShortMsg(NK2.midiChannel, control, NK2.Controls[control]["LEDstate"]);//restore LED state when button released
	}

	if (command!=false)eval(command);
};

NK2.knob = function knob(channel, control, value, status, group) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	
	//input received - gatekeeper function connected to all knobs
 
	//get the control object for the current button on the current bank - associative array containing control info, LED state evaluators, press/release functions, etc.
	var theControl=NK2.getControl(control);
	if (theControl===false){return false;}//no control set for this button
	
	var command = theControl["pressEval"];
	if (command!=false)eval(command);
};

NK2.cyclepress = function cyclepress() {//press cycle button
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	if (NK2.cycleState==0){
		NK2.cycleState=1;
	}else{
		NK2.cycleState=0;
	}
	NK2.updateLEDs();
};

NK2.modpress = function modpress(button) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	if(button=="BANK"){
		NK2.doClearBank=true;//if false when released, bank has been switched, so don't clear the selected bank on release
		NK2.bankSelectState=1;
	}else if (button=="MOD1"){
		NK2.MOD1state=1;
	}else if (button=="MOD2"){
		NK2.MOD2state=1;
	};
	NK2.updateLEDs();
};

NK2.modrelease = function modrelease(button) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	if(button=="BANK"){
		NK2.bankSelectState=0;
		if (NK2.doClearBank===true){
			if (NK2.curNSMR!="N"){NK2.cycleState=0;}
			NK2.curNSMR="N";
			NK2.curDeck=0;
			NK2.LEDBankIndicator=false;
			engine.stopTimer(NK2.LEDtimer);
			NK2.updateLEDs();
		}
		return true;
	}else if (button=="MOD1"){
		NK2.MOD1state=0;
	}else if (button=="MOD2"){
		//reset cue move stuff
		engine.stopTimer(NK2.cueMoveIndicator);
		NK2.cueMoveToNum=-1;
		NK2.cuemoveLastIndicator=-1;
	
		NK2.MOD2state=0;
	};
	
	NK2.updateLEDs();
};


//############################################################################
//LED FUNCTIONS
//############################################################################

NK2.updateLEDs = function updateLEDs() {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	//fires on mod press/release, bank switch - updates all LEDs
	var midino;
	var control;
	var state=0;
	
	for (var key in NK2.Controls){//clear hooks
		var midino=key;
		var thecontrol = NK2.getControl(key);
		
		
		var LEDstateType=thecontrol["LEDstateType"];
		var LEDstateEval=thecontrol["LEDstateEval"];
		var LEDhookControl=thecontrol["LEDhookControl"];
		var LEDhookFunction=thecontrol["LEDhookFunction"];
		var LEDhookGroup=(thecontrol["LEDhookGroup"]=="default")?NK2.Deck[NK2.curDeck]:thecontrol["LEDhookGroup"];
		
		//clear hooks
		if (NK2.Controls[key]["LEDstateType"]==="hook" && LEDstateType==="hook"){//check if hooks are the same, if not, clear the hook
			if(NK2.Controls[key]["LEDhookGroup"]==LEDhookGroup && NK2.Controls[key]["LEDhookControl"]==LEDhookControl && NK2.Controls[key]["LEDhookFunction"]==LEDhookFunction){
				//hooks are the same - go to next control
				if (NK2.debug>0){print("##################SKIPPING")};
				continue;
			}else{//not the same.  clear the hook
				if (NK2.debug>0){print("##################CLEARING####################")};
				//engine.connectControl(NK2.Controls[key]["LEDhookGroup"], NK2.Controls[key]["LEDhookControl"], NK2.Controls[key]["LEDhookFunction"], true);
				NK2.clearHook(NK2.Controls[key]["LEDhookGroup"], NK2.Controls[key]["LEDhookControl"], NK2.Controls[key]["LEDhookFunction"]);
			}
			
		}else if(NK2.Controls[key]["LEDstateType"]==="hook"){//clear the hook
			//engine.connectControl(NK2.Controls[key]["LEDhookGroup"], NK2.Controls[key]["LEDhookControl"], NK2.Controls[key]["LEDhookFunction"], true);
			NK2.clearHook(NK2.Controls[key]["LEDhookGroup"], NK2.Controls[key]["LEDhookControl"], NK2.Controls[key]["LEDhookFunction"]);
		};
		//engine.connectControl(NK2.Controls[key]["LEDhookGroup"], NK2.Controls[key]["LEDhookControl"], NK2.Controls[key]["LEDhookFunction"], true);
	};//end clear hooks for
		
	for (var key in NK2.Controls){//set new led settings
		var midino=key;
		var thecontrol = NK2.getControl(key);
		
		
		var LEDstateType=thecontrol["LEDstateType"];
		var LEDstateEval=thecontrol["LEDstateEval"];
		var LEDhookControl=thecontrol["LEDhookControl"];
		var LEDhookFunction=thecontrol["LEDhookFunction"];
		var LEDhookGroup=(thecontrol["LEDhookGroup"]=="default")?NK2.Deck[NK2.curDeck]:thecontrol["LEDhookGroup"];
				
		if (thecontrol["LEDstateType"]===false){continue;}//control has no LED, LED Type is not set, or LED doesn't change on this mod setting.  skip to next control
		
		//reset LED state storage
		NK2.Controls[key]["LEDstate"]=0;
		NK2.Controls[key]["LEDstateType"]=LEDstateType;
		NK2.Controls[key]["LEDstateEval"]=LEDstateEval;
		NK2.Controls[key]["LEDhookControl"]=LEDhookControl;
		NK2.Controls[key]["LEDhookFunction"]=LEDhookFunction;
		NK2.Controls[key]["LEDhookGroup"]=LEDhookGroup;
		

		if (LEDstateType=="on"){//LED is always on
			state=1;
			NK2.Controls[key]["LEDstate"]=1;
			}
		else if (LEDstateType=="off"){//LED is always off
			state=0;
			NK2.Controls[key]["LEDstate"]=0;
			}
		else if (LEDstateType=="hook"){//LED is hooked to a mixxx control
			
			NK2.setHook(LEDhookGroup, LEDhookControl, LEDhookFunction);//set hook
			engine.trigger(LEDhookGroup, LEDhookControl);
			
			continue;
			}
		else if (thecontrol["LEDstateType"]=="eval"){//LED state determined by evaluating a javascript statement
			if (NK2.debug>2){print("##updateLEDs Eval: "+thecontrol["LEDstateEval"])};
			if (NK2.debug>2){print("##updateLEDs midino: "+midino)};
			if (NK2.debug>2){print("##evalstate: "+eval(thecontrol["LEDstateEval"]))};
			
			state=eval(LEDstateEval);
			NK2.Controls[key]["LEDstate"]=(state>0)?1:0;
			}//end if
			
		midi.sendShortMsg(NK2.midiChannel, midino, state*127);
		state=0;//reset state
		}//end for
	};

NK2.indicatorLEDs = function indicatorLEDs(value,group,control){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	engine.stopTimer(NK2.flashTimer);//kill previous timer if exists
		
	NK2.flashon=0;
	NK2.flashcount=0;
	
	NK2.flashTimer=engine.beginTimer(100, "NK2.flashIndicators()");
	return true;
};

NK2.flashIndicators= function flashIndicators(){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};

	if (NK2.flashon==0){
		if (NK2.flashcount>=2){NK2.stopFlashTimer();return;}
		if (NK2.LEDBankIndicator!=false){midi.sendShortMsg(NK2.midiChannel, NK2.LEDBankIndicator, 0);}
		NK2.flashon=1;
		NK2.flashcount++;
		}else {
		if (NK2.flashcount>=2){NK2.stopFlashTimer();return;}
		if (NK2.LEDBankIndicator!=false){midi.sendShortMsg(NK2.midiChannel, NK2.LEDBankIndicator, 127);}
		NK2.flashon=0;
		NK2.flashcount++;
		}
	}

NK2.stopFlashTimer = function stopFlashTimer() {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	engine.stopTimer(NK2.flashTimer);
	NK2.flashcount=0;
	if (NK2.LEDBankIndicator!=false){midi.sendShortMsg(NK2.midiChannel, NK2.LEDBankIndicator, NK2.Controls[NK2.LEDBankIndicator]["LEDstate"]);};//restore state
	}

NK2.cueMoveIndicatorLEDs = function cueMoveIndicatorLEDs(control){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	 
	NK2.cuemovecontrol=control;
	if (NK2.cuemoveLastIndicator!=-1){midi.sendShortMsg(NK2.midiChannel, NK2.cuemoveLastIndicator, 0);}//turn off last indicator in case timer was interrupted (keeps last led from being left on if you switch "move to" buttons.
	NK2.cuemoveLastIndicator=control;
	NK2.cuemoveflashon=(NK2.cuemoveflashon!=0 && NK2.cuemoveflashon!=1)?0:NK2.cuemoveflashon;

	if (NK2.cuemoveflashon==0){
		midi.sendShortMsg(NK2.midiChannel, control, 0);
		NK2.cuemoveflashon=1;
		}else {
		midi.sendShortMsg(NK2.midiChannel, control, 127);
		NK2.cuemoveflashon=0;
		}

	return true;
};

NK2.binaryHookLEDs = function lightButtonLEDs(value,group,control){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	//light button leds for simple binary values
	value=value*127;
	value=(value>127)?127:value;
	value=(value<0)?0:value;
	var state=(value>0)?1:0;

	//iterate through Controls to find button(s) that's hooked to this control
	for (var key in NK2.Controls){
		if (NK2.Controls[key]["LEDhookControl"]==control && NK2.Controls[key]["LEDhookGroup"]==group){
			var midino=key;
			midi.sendShortMsg(NK2.midiChannel, midino, value);
			NK2.Controls[key]["LEDstate"]=state;
		}
	}//end for
};

NK2.muteLEDs = function muteLEDs(value,group,control){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	//light deck mute leds, also cancels mute if volume changes

	//iterate through Controls to find button(s) that's hooked to this control
	for (var key in NK2.Controls){
		if (NK2.Controls[key]["LEDhookControl"]==control && NK2.Controls[key]["LEDhookGroup"]==group){
			var midino=key;
			var state=(value>0 || NK2.deckData[group]['muteVol']==-1)?0:1;
			midi.sendShortMsg(NK2.midiChannel, midino, state*127);
			NK2.Controls[key]["LEDstate"]=state;
			if(NK2.deckData[group]['muteVol']!=-1 && value>0){NK2.deckData[group]['muteVol']=-1;}
		}
	}//end for
};

NK2.vuMeterL = function vuMeterL(value,group,control){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	//light S buttons left to right for left VU meter

	value=(value*7)+1;
	for (var i=1; i<9; i++){
		if (value>=i){midi.sendShortMsg(NK2.midiChannel, NK2.Sbutton[i], 127);}else{midi.sendShortMsg(NK2.midiChannel, NK2.Sbutton[i], 0);}
	}
};

NK2.vuMeterR = function vuMeterR(value,group,control){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	//light S buttons left to right for left VU meter

	value=(value*7)+1;
	for (var i=1; i<9; i++){
		if (value>=i){midi.sendShortMsg(NK2.midiChannel, NK2.Mbutton[i], 127);}else{midi.sendShortMsg(NK2.midiChannel, NK2.Mbutton[i], 0);}
	}
};

NK2.beatIndicatorRow = function beatIndicatorRow(value,group,control){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	//light S buttons left to right for left VU meter

	value=(value*127);
	for (var i=1; i<9; i++){
		midi.sendShortMsg(NK2.midiChannel, NK2.Rbutton[i], value);
	}
};



//############################################################################
//BASIC FUNCTIONS - stuff that makes the other stuff work
//############################################################################
NK2.getFunctionName = function getFunctionName(){//return name of calling function
	var re = /function (.*?)\(/
    var s = NK2.getFunctionName.caller.toString();
    var m = re.exec( s )
    return m[1];
};

NK2.doNothing = function doNothing(){//dummy function - do nothing
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
    return false;
};

NK2.setHook = function setHook(group, control, func){//clear hook
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	var done=false;
	done=engine.connectControl(group, control, func);
	return done;
};

NK2.clearHook = function clearHook(group, control, func){//clear hook
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	var done=false;
	done=engine.connectControl(group, control, func, true);
	return done;
};

NK2.getControl = function getControl(control){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	
	var theControl=NK2.Controls[control][NK2.curNSMR][NK2.getCurModCode()];//get control for current mod settings
	if (theControl['isset']===false || typeof theControl === 'undefined'){
		theControl=NK2.Controls[control][NK2.curNSMR][NK2.bin2dec(NK2.cycleState.toString()+NK2.bankSelectState.toString()+"00")];//get control without mods, only check cycle state and bank select
		if (theControl['isset']===false || typeof theControl === 'undefined'){
			theControl=NK2.Controls[control][NK2.curNSMR][NK2.bin2dec("0"+NK2.bankSelectState.toString()+"00")];//get control without mods or cycle - ONLY bank select
			if (theControl['isset']===false || typeof theControl === 'undefined'){
				theControl=NK2.Controls[control][NK2.curNSMR][0];//get control without mods, cycle or bank select
			};

		};
	};
	
	if (theControl['isset']===false || typeof theControl === 'undefined'){return false;}
	
	//returns the array of control commands and LED state functions for the control pressed for the current bank/mods
	return theControl;
};
	
NK2.getCurModCode = function getCurModCode(control){//returns the current mod code value
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	
	var codestr=NK2.cycleState.toString()+NK2.bankSelectState.toString()+NK2.MOD1state.toString()+NK2.MOD2state.toString();
	//print("#####modcode: "+codestr);
	return parseInt(codestr, 2);
};

NK2.bin2dec = function bin2dec(str){//convert binary string to decimal
	return parseInt(str, 2);
};

//############################################################################
//FUNCTIONALITY - stuff that does stuff
//############################################################################

NK2.knobAdjust = function knobAdjust(group, control, value, min, max){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	if (group=="default"){group=NK2.Deck[NK2.curDeck];};
	//use a knob to adjust a mixxx control
	
	var range=max-min;
	var newValue=min+((value/127)*range);
	if (newValue>max)newValue=max;
	if (newValue<min)newValue=min;
	engine.setValue(group, control, newValue);
	}
	
NK2.logKnobAdjust = function logKnobAdjust(group, control, value, min, max){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	if (group=="default"){group=NK2.Deck[NK2.curDeck];};
	//use a knob to adjust a mixxx control
	//on a logarithmic scale (0 min, 4 max)
	//K = 4*(V)^2
	//V = (K/4)^0.5 -- K= value coming from prog (0-4, logarithmic scale), V=value adjusted to linear 0-1 
	var range=max-min;
	var adjustedValue=value/127;//adjust range so it's on a scale of 0 to 1 - logarithmic
	var newValue=min+(range*Math.pow((adjustedValue),2));

	if (newValue>max)newValue=max;
	if (newValue<min)newValue=min;
	engine.setValue(group, control, newValue);
	}

NK2.toggleBinaryControlAll = function toggleBinaryControlAll(control){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	
	var newstate = (engine.getValue(NK2.Deck[1], control)==0)?1:0;
	for (var i=1; i<9; i++){
		engine.setValue(NK2.Deck[i], control, newstate);
	};
	}

NK2.wavezoomAll = function wavezoomAll(value){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
		
	var range=6-1;
	var newValue=Math.round(1+((value/127)*range));
	if (newValue>6)newValue=6;
	if (newValue<1)newValue=1;
	if (NK2.lastwavevalue!=value){
		for (var i=1; i<9; i++){
			engine.setValue(NK2.Deck[i], "waveform_zoom", newValue);
		};
	}
	NK2.lastwavevalue=value;
	}

NK2.wavezoomDeck = function wavezoomDeck(value, group){
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	if (group=="default"){group=NK2.Deck[NK2.curDeck];};
	
	var range=6-1;
	var newValue=Math.round(1+((value/127)*range));
	if (newValue>6)newValue=6;
	if (newValue<1)newValue=1;
	if (NK2.lastwavevalue!=value){
		engine.setValue(group, "waveform_zoom", newValue);
	}
	NK2.lastwavevalue=value;
	}

NK2.mutePress = function mutePress(deck){//press mute button - toggle mute for a deck
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	
	if (NK2.deckData[deck]['muteVol']<0){//deck is currently not muted.  enable mute
		NK2.deckData[deck]['muteVol']=engine.getValue(deck, "volume");
		engine.setValue(deck, "volume", 0);
	} else {//deck is currently muted.  Unmute
		engine.setValue(deck, "volume", NK2.deckData[deck]['muteVol']);
		NK2.deckData[deck]['muteVol']=-1;
	}
};

NK2.muteRelease = function muteRelease(deck){//release mute button
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	if(NK2.deckData[deck]['muteVol']!=-1){
		engine.setValue(deck, "volume", NK2.deckData[deck]['muteVol']);
		NK2.deckData[deck]['muteVol']=-1;
	};
	return true;
};

NK2.binControlPress = function binControlPress(deck, control){//press button - toggle binary control for a deck
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	if (deck=="default"){deck=NK2.Deck[NK2.curDeck];};
	
	var curstate=engine.getValue(deck, control);
	if (curstate===0 || (control=="play" && engine.getValue(deck, "reverse")==1)){//control currently off
		engine.setValue(deck, control,1);
		//handle resets
		if (control=="play"){engine.setValue(deck, "reverse",0);}
	}else{
		engine.setValue(deck, control,0);
	}
};

NK2.binControlRelease = function binControlMomentary(deck, control){//release momentary button - reset control to 0 on release
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	if (deck=="default"){deck=NK2.Deck[NK2.curDeck];};
	
	engine.setValue(deck, control,0);
};

NK2.releaseSetFalse = function releaseSetFalse(deck, control){//release pfl button
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	engine.setValue(deck, control,false);
	return true;
};

NK2.cuePress = function cuePress(cue, button){//press hotcue
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	if (NK2.debug>2){print("##cue: "+cue)};
	if (NK2.debug>2){print("##deck: "+NK2.Deck[NK2.curDeck])};
	engine.setValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_activate", 1);
	engine.setValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_activate", 0);
}

NK2.cueClear = function cueClear(cue, control){//clear hotcue - OR move hotcue to new button
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	if (NK2.debug>2){print("##cue: "+cue)};
	if (NK2.debug>2){print("##deck: "+NK2.Deck[NK2.curDeck])};
	
	if(engine.getValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_enabled")!=true){//hotcue not set - prepare to move next hotcue pressed to this button
		NK2.cueMoveToNum=cue;
		engine.stopTimer(NK2.cueMoveIndicator);
		NK2.cueMoveIndicator=engine.beginTimer(100, "NK2.cueMoveIndicatorLEDs("+control+")");//start timer for LED indicator flasher showing the button we're moving to
		return true;
	}
	
	if(NK2.cueMoveToNum>0){//hotcue set, but we're moving it, not clearing it.
		engine.setValue(NK2.Deck[NK2.curDeck], "hotcue_"+NK2.cueMoveToNum+"_set", 1);
		engine.setValue(NK2.Deck[NK2.curDeck], "hotcue_"+NK2.cueMoveToNum+"_position", engine.getValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_position"));	
		engine.setValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_clear", 1);
		engine.setValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_clear", 0);
	
		engine.stopTimer(NK2.cueMoveIndicator);
		NK2.cueMoveToNum=-1;//reset
		NK2.cuemoveLastIndicator=-1;
		return true;
	}
	
	engine.setValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_clear", 1);
	engine.setValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_clear", 0);
}

NK2.cueLoop = function cueLoop(cue){//jump to hotcue and start loop
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	if (engine.getValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_enabled")!==1){//set hotcue and start loop
		engine.setValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_activate", 1);
		engine.setValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_activate", 0);
		engine.setValue(NK2.Deck[NK2.curDeck], "beatloop_"+NK2.cueLoopLen+"_activate", 1);
		}else{//jump to existing cue and loop
		
		//calculate start and end points
		var startpos=engine.getValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_position");
		var loopseconds = NK2.cueLoopLen*(1/(engine.getValue(NK2.Deck[NK2.curDeck], "bpm")/60));
		var loopsamples = loopseconds*engine.getValue(NK2.Deck[NK2.curDeck], "track_samplerate")*2;//*2 to compensate for stereo samples
		var endpos=startpos+loopsamples;

		//disable loop if currently enabled
		if (engine.getValue(NK2.Deck[NK2.curDeck], "loop_enabled")==true){engine.setValue(NK2.Deck[NK2.curDeck], "reloop_exit", 1);}
		
		//set start and endpoints
		engine.setValue(NK2.Deck[NK2.curDeck], "loop_start_position", startpos);
		engine.setValue(NK2.Deck[NK2.curDeck], "loop_end_position", endpos);
		//enable loop	
		engine.setValue(NK2.Deck[NK2.curDeck], "reloop_exit", 1);
	}
}

NK2.saveLoop = function saveLoop(cue){//save the current loop inpoint as a hotcue
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	if (engine.getValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_enabled")==1){//hotcue is already set, return false
		return false;
		}else if (engine.getValue(NK2.Deck[NK2.curDeck], "loop_enabled")!=true){//no loop currently active, return false
		return false;
		}else{//save the loop as a hotcue
		engine.setValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_set", 1);
		engine.setValue(NK2.Deck[NK2.curDeck], "hotcue_"+cue+"_position", engine.getValue(NK2.Deck[NK2.curDeck], "loop_start_position"));
		return true;
	}
}

NK2.beatjump = function (jumplen) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	//jumps back certain number of beats depending on knob modifier
	var curpos = engine.getValue(NK2.Deck[NK2.curDeck], "playposition")*engine.getValue(NK2.Deck[NK2.curDeck], "track_samples");
	var numbeats = jumplen;
	var backseconds = numbeats*(1/(engine.getValue(NK2.Deck[NK2.curDeck], "bpm")/60));
	var backsamples = backseconds*engine.getValue(NK2.Deck[NK2.curDeck], "track_samplerate")*2;//*2 to compensate for stereo samples
	var newpos = curpos-(backsamples);
	
	if (NK2.debug){print("backseconds: "+backseconds);}
	if (NK2.debug){print("backsamples: "+backsamples);}
	if (NK2.debug){print("curpos: "+curpos);}
	if (NK2.debug){print("newpos: "+newpos);}
	if (NK2.debug){print("numbeats: "+numbeats);}
	
	engine.setValue(NK2.Deck[NK2.curDeck], "playposition", newpos/engine.getValue(NK2.Deck[NK2.curDeck], "track_samples"));
	};

NK2.beatloop = function beatloop(looplen, value) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};

	//activate beatloop
	if (value>0){//button was pressed
		NK2.loopbuttonDown=true;
		engine.setValue(NK2.Deck[NK2.curDeck], "beatloop_"+looplen+"_activate", 1);
		} else {//button was released
		NK2.loopbuttonDown=false;
		};

	};
	
NK2.beatlooproll = function beatlooproll(looplen, value) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};

	//activate beatloop
	if (value>0){//button was pressed
		NK2.loopbuttonDown=true;
		engine.setValue(NK2.Deck[NK2.curDeck], "beatlooproll_"+looplen+"_activate", 1);
		} else {//button was released
		NK2.loopbuttonDown=false;
		};

	};

NK2.reloopButton = function reloopButton(value) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	
	if (value>0){//button was pressed
		engine.stopTimer(NK2.reloopTimer);
		NK2.loopbuttonDown=true;
		NK2.doreloop=true;
		NK2.reloopTimer = engine.beginTimer(500, "NK2.disablereloop()", true);
		} else {//button was released
		NK2.loopbuttonDown=false;
		if (NK2.doreloop===true) {engine.setValue(NK2.Deck[NK2.curDeck], "reloop_exit", 1);engine.setValue(NK2.Deck[NK2.curDeck], "reloop_exit", 0);};
		NK2.doreloop=true;
		engine.stopTimer(NK2.reloopTimer);
		};

	};

NK2.loopin = function loopin(value) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};

	//activate beatloop
	if (value>0){//button was pressed
		engine.stopTimer(NK2.loopinTimer);
		NK2.loopinbuttonDown=true;
		NK2.doloopin=true;
		NK2.loopinTimer = engine.beginTimer(500, "NK2.disableloopin()", true);
		} else {//button was released
		NK2.loopinbuttonDown=false;
		if (NK2.doloopin===true) {engine.setValue(NK2.Deck[NK2.curDeck], "loop_in", 1);engine.setValue(NK2.Deck[NK2.curDeck], "loop_in", 0);};
		NK2.doloopin=true;
		engine.stopTimer(NK2.loopinTimer);
		};

	};
	
NK2.loopout = function loopout(value) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};

	//activate beatloop
	if (value>0){//button was pressed
		engine.stopTimer(NK2.loopoutTimer);
		NK2.loopoutbuttonDown=true;
		NK2.doloopout=true;
		NK2.loopoutTimer = engine.beginTimer(500, "NK2.disableloopout()", true);
		} else {//button was released
		NK2.loopoutbuttonDown=false;
		if (NK2.doloopout===true) {engine.setValue(NK2.Deck[NK2.curDeck], "loop_out", 1);engine.setValue(NK2.Deck[NK2.curDeck], "loop_out", 0);};
		NK2.doloopout=true;
		engine.stopTimer(NK2.loopoutTimer);
		};

	};
	
NK2.disablereloop = function disablereloop() {
	//timed function - fires half a second after pressing reloop.  Don't do the reloop if you hold down the button (so you can move the loop without exiting)
	NK2.doreloop=false;
	};

NK2.disableloopin = function disableloopin() {
	//timed function - fires half a second after pressing reloop.  Don't do the reloop if you hold down the button (so you can move the loop without exiting)
	NK2.doloopin=false;
	};

NK2.disableloopout = function disableloopout() {
	//timed function - fires half a second after pressing reloop.  Don't do the reloop if you hold down the button (so you can move the loop without exiting)
	NK2.doloopout=false;
	};

NK2.loopminus = function loopminus() {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	//shrinks loop or moves loop back
	if (NK2.loopbuttonDown !== true && NK2.loopinbuttonDown!==true && NK2.loopoutbuttonDown!==true){engine.setValue(NK2.Deck[NK2.curDeck], "loop_halve", 1); engine.setValue(NK2.Deck[NK2.curDeck], "loop_halve", 0); return false;}//shrink loop if no loop button down 
	else if (NK2.loopbuttonDown === true && engine.getValue(NK2.Deck[NK2.curDeck], "loop_start_position")>=0 && engine.getValue(NK2.Deck[NK2.curDeck], "loop_end_position")>=0 ){
		//move loop
		var interval =	NK2.loopmove*engine.getValue(NK2.Deck[NK2.curDeck], "track_samples")/engine.getValue(NK2.Deck[NK2.curDeck], "duration");
		var start = engine.getValue(NK2.Deck[NK2.curDeck], "loop_start_position");
		var end = engine.getValue(NK2.Deck[NK2.curDeck], "loop_end_position");
		engine.setValue(NK2.Deck[NK2.curDeck], "loop_start_position", start-interval);
		engine.setValue(NK2.Deck[NK2.curDeck], "loop_end_position", end-interval);
		return true;
		}
	else if (NK2.loopinbuttonDown === true && engine.getValue(NK2.Deck[NK2.curDeck], "loop_start_position")>=0){
		//move loop in point
		var interval =	NK2.loopmove*engine.getValue(NK2.Deck[NK2.curDeck], "track_samples")/engine.getValue(NK2.Deck[NK2.curDeck], "duration");
		var start = engine.getValue(NK2.Deck[NK2.curDeck], "loop_start_position");
		engine.setValue(NK2.Deck[NK2.curDeck], "loop_start_position", start-interval);
		return true;
		}
	else if (NK2.loopoutbuttonDown === true && engine.getValue(NK2.Deck[NK2.curDeck], "loop_end_position")>=0){
		//move loop out point
		var interval =	NK2.loopmove*engine.getValue(NK2.Deck[NK2.curDeck], "track_samples")/engine.getValue(NK2.Deck[NK2.curDeck], "duration");
		var end = engine.getValue(NK2.Deck[NK2.curDeck], "loop_end_position");
		engine.setValue(NK2.Deck[NK2.curDeck], "loop_end_position", end-interval);
		return true;
		};
	};

NK2.loopplus = function loopplus() {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	//grows loop or moves loop forward
	if (NK2.loopbuttonDown !== true && NK2.loopinbuttonDown!==true && NK2.loopoutbuttonDown!==true){engine.setValue(NK2.Deck[NK2.curDeck], "loop_double", 1); engine.setValue(NK2.Deck[NK2.curDeck], "loop_double", 0); return false;}//shrink loop if no loop button down 
	else if (NK2.loopbuttonDown === true && engine.getValue(NK2.Deck[NK2.curDeck], "loop_start_position")>=0 && engine.getValue(NK2.Deck[NK2.curDeck], "loop_end_position")>=0 ){
		//move loop
		var interval =	NK2.loopmove*engine.getValue(NK2.Deck[NK2.curDeck], "track_samples")/engine.getValue(NK2.Deck[NK2.curDeck], "duration");
		var start = engine.getValue(NK2.Deck[NK2.curDeck], "loop_start_position");
		var end = engine.getValue(NK2.Deck[NK2.curDeck], "loop_end_position");
		engine.setValue(NK2.Deck[NK2.curDeck], "loop_start_position", start+interval);
		engine.setValue(NK2.Deck[NK2.curDeck], "loop_end_position", end+interval);
		return true;
		}
	else if (NK2.loopinbuttonDown === true && engine.getValue(NK2.Deck[NK2.curDeck], "loop_start_position")>=0){
		//move loop in point
		var interval =	NK2.loopmove*engine.getValue(NK2.Deck[NK2.curDeck], "track_samples")/engine.getValue(NK2.Deck[NK2.curDeck], "duration");
		var start = engine.getValue(NK2.Deck[NK2.curDeck], "loop_start_position");
		engine.setValue(NK2.Deck[NK2.curDeck], "loop_start_position", start+interval);
		return true;
		}
	else if (NK2.loopoutbuttonDown === true && engine.getValue(NK2.Deck[NK2.curDeck], "loop_end_position")>=0){
		//move loop out point
		var interval =	NK2.loopmove*engine.getValue(NK2.Deck[NK2.curDeck], "track_samples")/engine.getValue(NK2.Deck[NK2.curDeck], "duration");
		var end = engine.getValue(NK2.Deck[NK2.curDeck], "loop_end_position");
		engine.setValue(NK2.Deck[NK2.curDeck], "loop_end_position", end+interval);
		return true;
		};
	};

NK2.loopDial = function loopDial(value, deck) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	//activates variable length loop depending on dial position.  to exit, hit reloop/exit
	if (deck=="default"){deck=NK2.Deck[NK2.curDeck];};
	value=value/127;//change range from 0-1
	value=Math.round(value*(NK2.beatloopLengths.length-1));
	var newLoopLen=NK2.beatloopLengths[value];
	if (engine.getValue(deck, "loop_enabled")==true){
		var startpos=engine.getValue(deck, "loop_start_position");
		var loopseconds = newLoopLen*(1/(engine.getValue(deck, "bpm")/60));
		var loopsamples = loopseconds*engine.getValue(deck, "track_samplerate")*2;//*2 to compensate for stereo samples
		var endpos=startpos+loopsamples;
		engine.setValue(NK2.Deck[NK2.curDeck], "loop_end_position", endpos);
	}else{
		engine.setValue(deck, "beatloop_"+newLoopLen+"_activate", 1);
	}
	return true;
	};

NK2.setOrientation = function setOrientation(deck, orientation) {//sets crossfader orientation for a deck
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	engine.setValue(deck, "orientation", orientation);
	NK2.updateLEDs();
};

//############################################################################
//TEST FUNCTIONS - stuff to check out if the other stuff works
//############################################################################
NK2.test = function test(channel, control, value, status, group) {
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	
	print("channel: "+channel);
	print("control: "+control.toString(16));
	print("value: "+value);
	print("status: "+status);
	print("group: "+group);
	print("test: ##"+status.toString(16)+control.toString(16)+"##");
	print("loopbuttondown: ##"+NK2.loopbuttonDown+"##");
	print("NK2.looplen: ##"+NK2.looplen+"##");
	print("NK2.looplen.tostring: ##"+NK2.looplen.toString(16)+"##");
	print("NK2.looptype: ##"+NK2.looptype+"##");
	print("test: ##"+2+"##");
};

NK2.testLED=0;//used by test function
NK2.testleds=function testleds(){//sends LED on messages to all
	midi.sendShortMsg(0xB0, NK2.testLED, 0);
	if (NK2.testLED>=127){return false; } 
	NK2.testLED++
	midi.sendShortMsg(0xB0, NK2.testLED, 0x7F);	
	NK2.LEDtimer=engine.beginTimer(100, "NK2.testleds()", true);
}
	
//############################################################################
//CONTROL CONFIG - initialize control command object and LED state tracker, bind commands to controls
//############################################################################

//initialize control object function - iterates through buttons and sets up empty slots for command arrays - unset controls are "false"
NK2.initControls = function initControls(obj) {//initialize controls array
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};
	
	for (var x in obj){
		NK2.Controls[obj[x]]={"LEDstate":0, "LEDstateType":false, "LEDstateEval":false, "LEDhookControl":false, "LEDhookFunction":false, "LEDhookGroup":false};//init with empty LED tracking properties
		
		for (var y in NK2.Banks){//iterate through bank modes
			NK2.Controls[obj[x]][NK2.Banks[y]]={};//init empty
			
			for (var z=0;z<16;z++){//iterate through mod codes
				NK2.Controls[obj[x]][NK2.Banks[y]][z]={"isset":false, "pressEval":false, "releaseEval":false, "LEDstateType":false, "LEDstateEval":false, "LEDhookControl":false, "LEDhookFunction":false, "LEDhookGroup":false};
				};//end for z
			};//end for y
		};//end for x
	};





NK2.setup = function setup(obj) {//initialize controls array
	if (NK2.debug>2){print("##function: "+NK2.getFunctionName())};

	
//initialize control object for buttons, knobs, etc.
NK2.Controls={};

NK2.initControls(NK2.Knob);
NK2.initControls(NK2.Sbutton);
NK2.initControls(NK2.Mbutton);
NK2.initControls(NK2.Rbutton);
NK2.initControls(NK2.leftButton);

//CONTROL SYNTAX
//NK2.Controls[midino]["N" (none), "S", "M", or "R"][MOD state: no mod = 0, bank select = 1, MOD 1 on = 2, MOD 2 on = 3]
//'PressEval', 'ReleaseEval' = code eval'd when button is pressed or released 

/* MOD STATE - binary based
 * 1st digit - cycle button - this is a toggle (others are momentary on), and has a LED (others don't)
 * 2nd digit - bank select (marker set) button
 * 3rd digit - MOD 1 - track down
 * 4th digit - MOD 2 - track up
 * 
 * eg:
 * 0100 - (dec. 4) - bank select only
 * 1101 - (dec. 5) - cycle, bank sel. and MOD 2
 * 
 * decimal equivalents - 3 buttons = 8 states - 4 buttons, 16 states
0000 - 0
0001 - 1
0010 - 2
0011 - 3
0100 - 4
0101 - 5
0110 - 6
0111 - 7
1000 - 8
1001 - 9
1010 - 10
1011 - 11
1100 - 12
1101 - 13
1110 - 14
1111 - 15
 */


//eval strs can be false if no function on press or release - eg: knobs are false on release, since they don't release
//LEDstateType = false if control has no LED, "hook" if using connectControl, "eval" if using an eval statement to get initial LED state
//LEDstateType = "off" if LED is always off, "on" if LED is always on
//for HOOKS: LEDhookControl = mixxx control to connect to, LEDhookFunction = script function
//for HOOKS: LEDhookGroup = mixxx control group - literals accepted (eg: "[Channel1]", or "default", "sync" or "alt" = default, sync or alt deck for that channel
//for EVAL: LEDstateEval = javascript to evaluate to get LED state
//init: {"isset":false, "pressEval":false, "releaseEval":false, "LEDstateType":false, "LEDstateEval":false, "LEDhookControl":false, "LEDhookFunction":false, "LEDhookGroup":false}\

//bank select and mod buttons - marker set down and up
for (var j=0;j<4;j++){
	NK2.Controls[NK2.leftButton["mset"]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.modpress("BANK");', "releaseEval":'NK2.modrelease("BANK");', "LEDstateType":"on"};
	NK2.Controls[NK2.leftButton["mdown"]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.modpress("MOD1");', "releaseEval":'NK2.modrelease("MOD1");', "LEDstateType":"on"};
	NK2.Controls[NK2.leftButton["mup"]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.modpress("MOD2");', "releaseEval":'NK2.modrelease("MOD2");', "LEDstateType":"on"};
	NK2.Controls[NK2.leftButton["cycle"]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.cyclepress();', "releaseEval":false, "LEDstateType":"eval", "LEDstateEval":'(NK2.cycleState==0)?0:1;'};
};

//bank select button pressed - initialize bank select buttons
for (var i=1;i<9;i++){//i = strip number
	for (var j=0;j<4;j++){
		NK2.Controls[NK2.Sbutton[i]][NK2.Banks[j]][NK2.MODcodes["0100"]]={"isset":true, "pressEval":'NK2.bankSelect('+i+', "S");', "LEDstateType":"on"};
		NK2.Controls[NK2.Mbutton[i]][NK2.Banks[j]][NK2.MODcodes["0100"]]={"isset":true, "pressEval":'NK2.bankSelect('+i+', "M");', "LEDstateType":"on"};
		NK2.Controls[NK2.Rbutton[i]][NK2.Banks[j]][NK2.MODcodes["0100"]]={"isset":true, "pressEval":'NK2.bankSelect('+i+', "R");', "LEDstateType":"on"};
		};
	};

//N mode left buttons
NK2.toggleBinaryControlAll
NK2.Controls[NK2.leftButton["rev"]]["N"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.toggleBinaryControlAll("quantize");', "releaseEval":false, "LEDstateType":"off"};
NK2.Controls[NK2.leftButton["ff"]]["N"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.toggleBinaryControlAll("keylock");', "releaseEval":false, "LEDstateType":"off"};
NK2.Controls[NK2.leftButton["stop"]]["N"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.toggleBinaryControlAll("slip_enabled");', "releaseEval":false, "LEDstateType":"off"};
NK2.Controls[NK2.leftButton["play"]]["N"][0]={"isset":true, "pressEval":'NK2.mutePress("[Master]");', "releaseEval":'NK2.muteRelease("[Master]");', "LEDstateType":"hook", "LEDhookControl":'volume', "LEDhookFunction":'NK2.muteLEDs', "LEDhookGroup":"[Master]"};

//set common left button controls for S, M, and R modes (these controls can be overridden by resetting them later in the script) 	
for (var j=1;j<4;j++){
	NK2.Controls[NK2.leftButton["rev"]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "quantize");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'quantize', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.leftButton["ff"]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "keylock");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'keylock', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.leftButton["rev"]][NK2.Banks[j]][NK2.MODcodes["0010"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "quantize");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'quantize', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.leftButton["ff"]][NK2.Banks[j]][NK2.MODcodes["0010"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "keylock");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'keylock', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.leftButton["stop"]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "slip_enabled");', "releaseEval": false, "LEDstateType":"hook", "LEDhookControl":'slip_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.leftButton["stop"]][NK2.Banks[j]][NK2.MODcodes["0010"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "slip_enabled");', "releaseEval": false, "LEDstateType":"hook", "LEDhookControl":'slip_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.leftButton["stop"]][NK2.Banks[j]][NK2.MODcodes["0001"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "beats_translate_curpos");', "releaseEval":'NK2.binControlRelease("default", "beats_translate_curpos");', "LEDstateType":"off"};
	NK2.Controls[NK2.leftButton["play"]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.doNothing();', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'play', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.leftButton["rec"]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.doNothing();', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'cue_default', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.leftButton["play"]][NK2.Banks[j]][NK2.MODcodes["0010"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "play");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'play', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.leftButton["rec"]][NK2.Banks[j]][NK2.MODcodes["0010"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "cue_default");', "releaseEval":'NK2.binControlRelease("default", "cue_default");', "LEDstateType":"hook", "LEDhookControl":'cue_default', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.leftButton["play"]][NK2.Banks[j]][NK2.MODcodes["0001"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "reverse");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'reverse', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.leftButton["rec"]][NK2.Banks[j]][NK2.MODcodes["0001"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "start");', "releaseEval":'NK2.binControlRelease("default", "start");', "LEDstateType":"hook", "LEDhookControl":'cue_default', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.leftButton["rec"]][NK2.Banks[j]][NK2.MODcodes["0011"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "start_stop");', "releaseEval":'NK2.binControlRelease("default", "start_stop");', "LEDstateType":"hook", "LEDhookControl":'cue_default', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.leftButton["trdown"]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "back");', "releaseEval":'NK2.binControlRelease("default", "back");', "LEDstateType":"off"};
	NK2.Controls[NK2.leftButton["trup"]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "fwd");', "releaseEval":'NK2.binControlRelease("default", "fwd");', "LEDstateType":"off"};
	NK2.Controls[NK2.leftButton["trdown"]][NK2.Banks[j]][NK2.MODcodes["0010"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "rate_temp_down");', "releaseEval":'NK2.binControlRelease("default", "rate_temp_down");', "LEDstateType":"off"};
	NK2.Controls[NK2.leftButton["trup"]][NK2.Banks[j]][NK2.MODcodes["0010"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "rate_temp_up");', "releaseEval":'NK2.binControlRelease("default", "rate_temp_up");', "LEDstateType":"off"};
	NK2.Controls[NK2.leftButton["trdown"]][NK2.Banks[j]][NK2.MODcodes["0001"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "rate_perm_down");', "releaseEval":'NK2.binControlRelease("default", "rate_perm_down");', "LEDstateType":"off"};
	NK2.Controls[NK2.leftButton["trup"]][NK2.Banks[j]][NK2.MODcodes["0001"]]={"isset":true, "pressEval":'NK2.binControlPress("default", "rate_perm_up");', "releaseEval":'NK2.binControlRelease("default", "rate_perm_up");', "LEDstateType":"off"};
	};
	
//NO BANK/TRACK SELECTED - bank select button off - set slip mode, mute and pfl buttons
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["N"][0]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "slip_enabled");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'slip_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":NK2.Deck[i]};
	NK2.Controls[NK2.Mbutton[i]]["N"][0]={"isset":true, "pressEval":'NK2.mutePress("'+NK2.Deck[i]+'");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'volume', "LEDhookFunction":'NK2.muteLEDs', "LEDhookGroup":NK2.Deck[i]};
	NK2.Controls[NK2.Rbutton[i]]["N"][0]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "pfl");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'pfl', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":NK2.Deck[i]};
	};

//CYCLE ON - momentary mute button
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Mbutton[i]]["N"][NK2.MODcodes["1000"]]={"isset":true, "pressEval":'NK2.mutePress("'+NK2.Deck[i]+'");', "releaseEval":'NK2.muteRelease("'+NK2.Deck[i]+'");', "LEDstateType":"hook", "LEDhookControl":'volume', "LEDhookFunction":'NK2.muteLEDs', "LEDhookGroup":NK2.Deck[i]};
	};

//N mode + MOD1 - lo mid hi kills
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["N"][2]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "filterHighKill");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'filterHighKill', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":NK2.Deck[i]};
	NK2.Controls[NK2.Mbutton[i]]["N"][2]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "filterMidKill");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'filterMidKill', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":NK2.Deck[i]};
	NK2.Controls[NK2.Rbutton[i]]["N"][2]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "filterLowKill");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'filterLowKill', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":NK2.Deck[i]};
	};

//CYCLE ON - N mode + MOD1 - MOMENTARY lo mid hi kills
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["N"][NK2.MODcodes["1010"]]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "filterHighKill");', "releaseEval":'NK2.binControlRelease("'+NK2.Deck[i]+'", "filterHighKill");', "LEDstateType":"hook", "LEDhookControl":'filterHighKill', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":NK2.Deck[i]};
	NK2.Controls[NK2.Mbutton[i]]["N"][NK2.MODcodes["1010"]]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "filterMidKill");', "releaseEval":'NK2.binControlRelease("'+NK2.Deck[i]+'", "filterMidKill");', "LEDstateType":"hook", "LEDhookControl":'filterMidKill', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":NK2.Deck[i]};
	NK2.Controls[NK2.Rbutton[i]]["N"][NK2.MODcodes["1010"]]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "filterLowKill");', "releaseEval":'NK2.binControlRelease("'+NK2.Deck[i]+'", "filterLowKill");', "LEDstateType":"hook", "LEDhookControl":'filterLowKill', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":NK2.Deck[i]};
	};

//N mode + MOD1 + MOD2 + Bank Select - Track crossfader orientation - S=left, M=middle, R=right
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["N"][NK2.MODcodes["0111"]]={"isset":true, "pressEval":'NK2.setOrientation("'+NK2.Deck[i]+'", 0);', "releaseEval":false, "LEDstateType":"eval", "LEDstateEval":'(engine.getValue("'+NK2.Deck[i]+'", "orientation")==0)?1:0;'};
	NK2.Controls[NK2.Mbutton[i]]["N"][NK2.MODcodes["0111"]]={"isset":true, "pressEval":'NK2.setOrientation("'+NK2.Deck[i]+'", 1);', "releaseEval":false, "LEDstateType":"eval", "LEDstateEval":'(engine.getValue("'+NK2.Deck[i]+'", "orientation")==1)?1:0;'};
	NK2.Controls[NK2.Rbutton[i]]["N"][NK2.MODcodes["0111"]]={"isset":true, "pressEval":'NK2.setOrientation("'+NK2.Deck[i]+'", 2);', "releaseEval":false, "LEDstateType":"eval", "LEDstateEval":'(engine.getValue("'+NK2.Deck[i]+'", "orientation")==2)?1:0;'};
	};

//N mode + MOD2 - keylock quantize
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["N"][1]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "flanger");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'flanger', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":NK2.Deck[i]};
	NK2.Controls[NK2.Mbutton[i]]["N"][1]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "quantize");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'quantize', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":NK2.Deck[i]};
	NK2.Controls[NK2.Rbutton[i]]["N"][1]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "keylock");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'keylock', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":NK2.Deck[i]};
	};



//M mode - hotcues 1-24
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["M"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.cuePress('+i+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+i+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Mbutton[i]]["M"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.cuePress('+(i+8)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+8)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Rbutton[i]]["M"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.cuePress('+(i+16)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+16)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	};

//M mode, with cycle toggled on - hotcues 9-32
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["M"][NK2.MODcodes["1000"]]={"isset":true, "pressEval":'NK2.cuePress('+(i+8)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+8)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Mbutton[i]]["M"][NK2.MODcodes["1000"]]={"isset":true, "pressEval":'NK2.cuePress('+(i+16)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+16)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Rbutton[i]]["M"][NK2.MODcodes["1000"]]={"isset":true, "pressEval":'NK2.cuePress('+(i+24)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+24)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	};

//M mode - CLEAR (with MOD2) hotcues 1-24
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["M"][NK2.MODcodes["0001"]]={"isset":true, "pressEval":'NK2.cueClear('+i+', control);', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+i+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Mbutton[i]]["M"][NK2.MODcodes["0001"]]={"isset":true, "pressEval":'NK2.cueClear('+(i+8)+', control);', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+8)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Rbutton[i]]["M"][NK2.MODcodes["0001"]]={"isset":true, "pressEval":'NK2.cueClear('+(i+16)+', control);', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+16)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	};

//M mode, with cycle toggled on - CLEAR (with MOD2) hotcues 9-32
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["M"][NK2.MODcodes["1001"]]={"isset":true, "pressEval":'NK2.cueClear('+(i+8)+', control);', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+8)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Mbutton[i]]["M"][NK2.MODcodes["1001"]]={"isset":true, "pressEval":'NK2.cueClear('+(i+16)+', control);', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+16)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Rbutton[i]]["M"][NK2.MODcodes["1001"]]={"isset":true, "pressEval":'NK2.cueClear('+(i+24)+', control);', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+24)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	};

//M mode - Jump to Hotcue and start loop (with MOD1) hotcues 1-24
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["M"][NK2.MODcodes["0010"]]={"isset":true, "pressEval":'NK2.cueLoop('+i+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+i+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Mbutton[i]]["M"][NK2.MODcodes["0010"]]={"isset":true, "pressEval":'NK2.cueLoop('+(i+8)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+8)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Rbutton[i]]["M"][NK2.MODcodes["0010"]]={"isset":true, "pressEval":'NK2.cueLoop('+(i+16)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+16)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	};

//M mode, with cycle toggled on - Jump to Hotcue and start loop (with MOD1) hotcues 9-32
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["M"][NK2.MODcodes["1010"]]={"isset":true, "pressEval":'NK2.cueLoop('+(i+8)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+8)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Mbutton[i]]["M"][NK2.MODcodes["1010"]]={"isset":true, "pressEval":'NK2.cueLoop('+(i+16)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+16)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Rbutton[i]]["M"][NK2.MODcodes["1010"]]={"isset":true, "pressEval":'NK2.cueLoop('+(i+24)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+24)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	};

//M mode with MOD1 and MOD2 - temporary loop controls - same as R mode, but only while holding mod buttons
var looplens={1:.125, 2:.25, 3:.5, 4:1, 5:2, 6:4, 7:8, 8:16}//array of loop and beatjump lengths
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["M"][NK2.MODcodes["0011"]]={"isset":true, "pressEval":'NK2.beatjump('+looplens[i]+');', "releaseEval":false, "LEDstateType":"off"};
	NK2.Controls[NK2.Mbutton[i]]["M"][NK2.MODcodes["0011"]]={"isset":true, "pressEval":'NK2.beatlooproll('+looplens[i]+', value);', "releaseEval":'NK2.beatlooproll('+looplens[i]+', value);', "LEDstateType":"hook", "LEDhookControl":'beatloop_'+looplens[i]+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Rbutton[i]]["M"][NK2.MODcodes["0011"]]={"isset":true, "pressEval":'NK2.beatloop('+looplens[i]+', value);', "releaseEval":'NK2.beatloop('+looplens[i]+', value);', "LEDstateType":"hook", "LEDhookControl":'beatloop_'+looplens[i]+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	};
//loophalve loopdouble
NK2.Controls[NK2.leftButton["trdown"]]["M"][NK2.MODcodes["0011"]]={"isset":true, "pressEval":'NK2.loopminus();', "releaseEval":false, "LEDstateType":"off"};
NK2.Controls[NK2.leftButton["trup"]]["M"][NK2.MODcodes["0011"]]={"isset":true, "pressEval":'NK2.loopplus();', "releaseEval":false, "LEDstateType":"off"};
//reloop exit
NK2.Controls[NK2.leftButton["stop"]]["M"][NK2.MODcodes["0011"]]={"isset":true, "pressEval":'NK2.reloopButton(value);', "releaseEval":'NK2.reloopButton(value);', "LEDstateType":"hook", "LEDhookControl":'loop_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
//loopin loopout
NK2.Controls[NK2.leftButton["rev"]]["M"][NK2.MODcodes["0011"]]={"isset":true, "pressEval":'NK2.loopin(value);', "releaseEval":'NK2.loopin(value);', "LEDstateType":"hook", "LEDhookControl":'loop_start_position', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
NK2.Controls[NK2.leftButton["ff"]]["M"][NK2.MODcodes["0011"]]={"isset":true, "pressEval":'NK2.loopout(value);', "releaseEval":'NK2.loopout(value);', "LEDstateType":"hook", "LEDhookControl":'loop_end_position', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};


//R mode with cycle - loops and beatjumps
var looplens={1:.125, 2:.25, 3:.5, 4:1, 5:2, 6:4, 7:8, 8:16}//array of loop and beatjump lengths
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["R"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.beatjump('+looplens[i]+');', "releaseEval":false, "LEDstateType":"off"};
	NK2.Controls[NK2.Mbutton[i]]["R"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.beatlooproll('+looplens[i]+', value);', "releaseEval":'NK2.beatlooproll('+looplens[i]+', value);', "LEDstateType":"hook", "LEDhookControl":'beatloop_'+looplens[i]+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Rbutton[i]]["R"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.beatloop('+looplens[i]+', value);', "releaseEval":'NK2.beatloop('+looplens[i]+', value);', "LEDstateType":"hook", "LEDhookControl":'beatloop_'+looplens[i]+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	};
//loophalve loopdouble
NK2.Controls[NK2.leftButton["trdown"]]["R"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.loopminus();', "releaseEval":false, "LEDstateType":"off"};
NK2.Controls[NK2.leftButton["trup"]]["R"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.loopplus();', "releaseEval":false, "LEDstateType":"off"};
//reloop exit
NK2.Controls[NK2.leftButton["stop"]]["R"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.reloopButton(value);', "releaseEval":'NK2.reloopButton(value);', "LEDstateType":"hook", "LEDhookControl":'loop_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
//loopin loopout
NK2.Controls[NK2.leftButton["rev"]]["R"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.loopin(value);', "releaseEval":'NK2.loopin(value);', "LEDstateType":"hook", "LEDhookControl":'loop_start_position', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
NK2.Controls[NK2.leftButton["ff"]]["R"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.loopout(value);', "releaseEval":'NK2.loopout(value);', "LEDstateType":"hook", "LEDhookControl":'loop_end_position', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};

//R mode with cycle - shorter loops and beatjumps
var shortlooplens={1:0.03125, 2:0.0625, 3:0.125, 4:0.25, 5:0.5, 6:1, 7:2, 8:4};//array of shorter loop and beatjump lengths
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["R"][NK2.MODcodes["1000"]]={"isset":true, "pressEval":'NK2.beatjump('+shortlooplens[i]+');', "releaseEval":false, "LEDstateType":"off"};
	NK2.Controls[NK2.Mbutton[i]]["R"][NK2.MODcodes["1000"]]={"isset":true, "pressEval":'NK2.beatlooproll('+shortlooplens[i]+', value);', "releaseEval":'NK2.beatlooproll('+shortlooplens[i]+', value);', "LEDstateType":"hook", "LEDhookControl":'beatloop_'+shortlooplens[i]+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Rbutton[i]]["R"][NK2.MODcodes["1000"]]={"isset":true, "pressEval":'NK2.beatloop('+shortlooplens[i]+', value);', "releaseEval":'NK2.beatloop('+shortlooplens[i]+', value);', "LEDstateType":"hook", "LEDhookControl":'beatloop_'+shortlooplens[i]+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	};

//R mode - Jump to Hotcue and start loop (with MOD1) hotcues 1-24
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["R"][NK2.MODcodes["0010"]]={"isset":true, "pressEval":'NK2.cueLoop('+i+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+i+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Mbutton[i]]["R"][NK2.MODcodes["0010"]]={"isset":true, "pressEval":'NK2.cueLoop('+(i+8)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+8)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Rbutton[i]]["R"][NK2.MODcodes["0010"]]={"isset":true, "pressEval":'NK2.cueLoop('+(i+16)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+16)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	};

//R mode, with cycle toggled on - Jump to Hotcue and start loop (with MOD1) hotcues 9-32
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["R"][NK2.MODcodes["1010"]]={"isset":true, "pressEval":'NK2.cueLoop('+(i+8)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+8)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Mbutton[i]]["R"][NK2.MODcodes["1010"]]={"isset":true, "pressEval":'NK2.cueLoop('+(i+16)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+16)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Rbutton[i]]["R"][NK2.MODcodes["1010"]]={"isset":true, "pressEval":'NK2.cueLoop('+(i+24)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+24)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	};

//R mode, with MOD2 - save current loop inpoint as hotcue 1-24
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["R"][NK2.MODcodes["0001"]]={"isset":true, "pressEval":'NK2.saveLoop('+i+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+i+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Mbutton[i]]["R"][NK2.MODcodes["0001"]]={"isset":true, "pressEval":'NK2.saveLoop('+(i+8)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+8)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Rbutton[i]]["R"][NK2.MODcodes["0001"]]={"isset":true, "pressEval":'NK2.saveLoop('+(i+16)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+16)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	};

//M mode, with cycle and MOD2 - save current loop inpoint as hotcue 9-32
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["R"][NK2.MODcodes["1001"]]={"isset":true, "pressEval":'NK2.saveLoop('+(i+8)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+8)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Mbutton[i]]["R"][NK2.MODcodes["1001"]]={"isset":true, "pressEval":'NK2.saveLoop('+(i+16)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+16)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	NK2.Controls[NK2.Rbutton[i]]["R"][NK2.MODcodes["1001"]]={"isset":true, "pressEval":'NK2.saveLoop('+(i+24)+');', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'hotcue_'+(i+24)+'_enabled', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":"default"};
	};



//S mode - vumeters and beat indicator
NK2.Controls[NK2.Sbutton[1]]["S"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":false, "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'VuMeterL', "LEDhookFunction":'NK2.vuMeterL', "LEDhookGroup":"default"};
NK2.Controls[NK2.Mbutton[1]]["S"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":false, "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'VuMeterR', "LEDhookFunction":'NK2.vuMeterR', "LEDhookGroup":"default"};
NK2.Controls[NK2.Rbutton[1]]["S"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":false, "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'beat_active', "LEDhookFunction":'NK2.beatIndicatorRow', "LEDhookGroup":"default"};
for (var i=2;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["S"][NK2.MODcodes["1000"]]={"isset":true, "pressEval":false, "releaseEval":false, "LEDstateType":"off"};
	NK2.Controls[NK2.Mbutton[i]]["S"][NK2.MODcodes["1000"]]={"isset":true, "pressEval":false, "releaseEval":false, "LEDstateType":"off"};
	NK2.Controls[NK2.Rbutton[i]]["S"][NK2.MODcodes["1000"]]={"isset":true, "pressEval":false, "releaseEval":false, "LEDstateType":"off"};
	};

//master VU meter - 
NK2.Controls[NK2.Sbutton[1]]["N"][NK2.MODcodes["0011"]]={"isset":true, "pressEval":false, "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'VuMeterL', "LEDhookFunction":'NK2.vuMeterL', "LEDhookGroup":"[Master]"};
NK2.Controls[NK2.Mbutton[1]]["N"][NK2.MODcodes["0011"]]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[1]+'", "bpm_tap");', "releaseEval":false, "LEDstateType":"hook", "LEDhookControl":'VuMeterR', "LEDhookFunction":'NK2.vuMeterR', "LEDhookGroup":"[Master]"};

for (var i=2;i<9;i++){
	NK2.Controls[NK2.Sbutton[i]]["N"][NK2.MODcodes["0011"]]={"isset":true, "pressEval":false, "releaseEval":false, "LEDstateType":"off"};
	NK2.Controls[NK2.Mbutton[i]]["N"][NK2.MODcodes["0011"]]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "bpm_tap");', "releaseEval":false, "LEDstateType":"off"};
	};

//BPM TAP for each track - MOD1 and MOD2 - N mode
for (var i=1;i<9;i++){
	NK2.Controls[NK2.Rbutton[i]]["N"][NK2.MODcodes["0011"]]={"isset":true, "pressEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "beatsync");', "releaseEval":'NK2.binControlPress("'+NK2.Deck[i]+'", "beatsync");', "LEDstateType":"hook", "LEDhookControl":'beat_active', "LEDhookFunction":'NK2.binaryHookLEDs', "LEDhookGroup":NK2.Deck[i]};
	};
	
//############# KNOBS
//N mode
NK2.Controls[NK2.Knob[1]]["N"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.wavezoomAll(value);', "LEDstateType":"off"};
NK2.Controls[NK2.Knob[2]]["N"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.logKnobAdjust("[Master]", "headVolume", value, 0, 5);', "LEDstateType":"off"};
NK2.Controls[NK2.Knob[3]]["N"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.knobAdjust("[Master]", "headMix", value, -1, 1);', "LEDstateType":"off"};
NK2.Controls[NK2.Knob[4]]["N"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.knobAdjust("[Master]", "balance", value, -1, 1);', "LEDstateType":"off"};
NK2.Controls[NK2.Knob[5]]["N"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.logKnobAdjust("[Master]", "volume", value, 0, 5);', "LEDstateType":"off"};
NK2.Controls[NK2.Knob[6]]["N"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.knobAdjust("[Flanger]", "lfoPeriod", value, 50000, 2000000);', "LEDstateType":"off"};
NK2.Controls[NK2.Knob[7]]["N"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.knobAdjust("[Flanger]", "lfoDepth", value, 0, 1);', "LEDstateType":"off"};
NK2.Controls[NK2.Knob[8]]["N"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.knobAdjust("[Flanger]", "lfoDelay", value, 50, 10000);', "LEDstateType":"off"};

//SMR MODES
for (var j=1;j<4;j++){
	NK2.Controls[NK2.Knob[1]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.wavezoomDeck(value, "default");', "LEDstateType":"off"};
	NK2.Controls[NK2.Knob[2]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.logKnobAdjust("default", "filterLow", value, 0, 4);', "LEDstateType":"off"};
	NK2.Controls[NK2.Knob[3]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.logKnobAdjust("default", "filterMid", value, 0, 4);', "LEDstateType":"off"};
	NK2.Controls[NK2.Knob[4]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.logKnobAdjust("default", "filterHigh", value, 0, 4);', "LEDstateType":"off"};
	NK2.Controls[NK2.Knob[5]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.logKnobAdjust("default", "pregain", value, 0, 4);', "LEDstateType":"off"};
	NK2.Controls[NK2.Knob[6]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.knobAdjust("default", "rate", value, -1, 1);', "LEDstateType":"off"};
	//NK2.Controls[NK2.Knob[7]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.knobAdjust("default", "");', "LEDstateType":"off"};
	//NK2.Controls[NK2.Knob[8]][NK2.Banks[j]][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.knobAdjust("default", "");', "LEDstateType":"off"};
}

//R mode - loop dial
NK2.Controls[NK2.Knob[8]]["R"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.loopDial(value, "default");', "LEDstateType":"off"};

//M mode - loop dial
NK2.Controls[NK2.Knob[8]]["M"][NK2.MODcodes["0000"]]={"isset":true, "pressEval":'NK2.loopDial(value, "default");', "LEDstateType":"off"};

};//end setup function 


/*
NOTES:
* 
*/
