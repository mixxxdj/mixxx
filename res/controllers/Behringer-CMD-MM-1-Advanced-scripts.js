CMDMM = {};     //controller itself
MIDI = {};      //midi related constants
CALLBACK = {};  //Every callback function
FUNCTIONS = {}; //misc for assigning controls

//######
//#TODO#
//######


//Feature Validation
//+[1&2]n enable FX 1&2
//+[1&2]c enable FX 3&4
//+[1&2]s orientation 1&2 on = 1&2 off  //needs rework
//+[m]n maximize library 
//+(m)n navigate library normal
//+[(m)] load into preview track
//+(m)s navigate preview track
//+[Faders] normal
//+[Crossfader] softTakeover
//+[CUE]n pfl
//+[CUE]s load to selected Deck
//+[CUE]c enable pfl for Fx according to channel.
//+(OUT1&2)n super FX 1&2
//+(OUT1&2)s mix FX 1&2
//+(OUT1&2)c super FX3&4
//+(OUT1&2)t mix FX3&4

//OUT SOFTTAKEOVER NOT WORKING
//Softtakeover isnt working at ALL!!!
//+(OUT) SoftTakeover!!!!
//Fader softTakeover!!!

//GLOBAL VARS

var CHANNELNUMBER = 5;

var INVERTCOLOR = true; //false=(off=orange,on=blue);true=(off=blue,on=orange);
var STANDARDCHANNELSEQUENCE = true; //false = [1,2,3,4], true = [3,1,2,4]
var STANDARDKNOBBEHAVIOR = 0; // 0 = [High,Mid,Low,Quickeffect]; 1 = [Gain,High,Mid,Low]; 2 = [Effect1Meta,Effect2Meta,Effect3Meta,mix]; 3 = [(be's four knob EQ; not implemented yet)];


//takes functions as arguments and executes them based on the current state of shift and control;
CMDMM.modes = function (normal, shift, ctrl, thirdLevel, modeOveride) {
	if (modeOveride === undefined) {
		switch (CMDMM.getLevel()) {
			case 0: if (normal !== undefined){normal();} break;
			case 1: if (shift !== undefined){shift();} break;
			case 2: if (ctrl !== undefined){ctrl();} break;
			case 3: if (thirdLevel !== undefined){thirdLevel();} break;
			default: print("invalid Level: "+CMDMM.getLevel());
		}
	} else {
		switch (modeOveride) {
			case "normal": normal(); break;
			case "shift": shift(); break;
			case "ctrl": ctrl(); break;
			case "third": thirdLevel(); break;
			default: print("invaled modeOveride: "+modeOveride);
		}
	}
};

MIDI.noteOn = 0x90 + (CHANNELNUMBER-1);
MIDI.noteOff = 0x80 + (CHANNELNUMBER-1);
MIDI.CC = 0xB0 + (CHANNELNUMBER-1);

//0x00 = orange; 0x01 = blue;
CMDMM.on = (INVERTCOLOR ? 0x00:0x01);
CMDMM.off = (INVERTCOLOR ? 0x01:0x00);
CMDMM.blink = 0x02;


//CMDMM.knobs = [[0x06,0x0A,0x0E,0x12],[0x07,0x0B,0x0F,0x13],[0x08,0x0C,0x10,0x14],[0x09,0x0D,0x11,0x15]];
//             ^Channel 1            ^Channel 2            ^Channel 3            ^Channel 4

CMDMM.buttons = [0x12,0x13,0x14,0x17,0x18,0x1B,0x1C,0x1F, 0x20,0x30,0x31,0x32,0x33,];
//               ^    ^Fx1 ^Fx2 ^Fx1 ^Fx2 ^Fx1 ^Fx2 ^Fx1 ^Fx2 ^CUE1^CUE2^CUE3^CUE4
//               |    ^Channel1 ^Channel2 ^Channel3 ^Channel4 
//               ^middlebutton

CMDMM.varStorage = {
	level:           0, //stores the current level (shift/ctrl/thirdlevel)
	knobAssignment: STANDARDKNOBBEHAVIOR,
	channelSequence: STANDARDCHANNELSEQUENCE ? [3,1,2,4]:[1,2,3,4],
}
CMDMM.getShift = function () {return (CMDMM.varStorage.level==1 ? true:false);}; 
CMDMM.shift = function () { CMDMM.varStorage.level++; CMDMM.updateLEDs(); FUNCTIONS.ignoreNextValue();};
CMDMM.unshift = function () {CMDMM.varStorage.level--; CMDMM.updateLEDs();FUNCTIONS.ignoreNextValue();};

CMDMM.getCtrl = function () {return (CMDMM.varStorage.level==2 ? true:false);};
CMDMM.ctrl = function () {CMDMM.varStorage.level += 2; CMDMM.updateLEDs();FUNCTIONS.ignoreNextValue();};
CMDMM.unctrl = function () {CMDMM.varStorage.level -= 2; CMDMM.updateLEDs();FUNCTIONS.ignoreNextValue();};

CMDMM.getThird = function () {return (CMDMM.varStorage.level==3 ? true:false);};
CMDMM.getLevel = function () {return CMDMM.varStorage.level};

CALLBACK.cue = function () {
	value = 0;
	for (var channel=1;channel<=4;channel++) {
		switch(CMDMM.getLevel()){
			case 0:
				value = engine.getValue("[Channel"+CMDMM.varStorage.channelSequence[channel-1]+"]","pfl")?CMDMM.on:CMDMM.off;
				break;
			case 1:
				value = engine.getValue("[Channel"+CMDMM.varStorage.channelSequence[channel-1]+"]","play")?CMDMM.on:CMDMM.off;
				break;
			case 2:
				value = engine.getValue("[EffectRack1_EffectUnit"+CMDMM.varStorage.channelSequence[channel-1]+"]","group_[Headphone]_enable")?CMDMM.on:CMDMM.off;
				break;
			case 3:
				value = engine.getValue("[Channel"+CMDMM.varStorage.channelSequence[channel-1]+"]","rate")?CMDMM.on:CMDMM.off;
				break;
		}
		midi.sendShortMsg(MIDI.noteOn, channel+0x2F, value);
	}
}
CALLBACK.middleButton = function () {
	switch (CMDMM.getLevel()) {
		case 0:
			//midi.sendShortMsg(MIDI.noteOn,CMDMM.buttons[0],engine.getParameter("[Master]","maximize_library")?CMDMM.on:CMDMM.off); break;
			midi.sendShortMsg(MIDI.noteOn,CMDMM.buttons[0],CMDMM.off); break;
		case 1:
			//midi.sendShortMsg(MIDI.noteOn,CMDMM.buttons[0],engine.getValue("[Master]","crossfader")?CMDMM.on:CMDMM.off); break;
		case 2:
			midi.sendShortMsg(MIDI.noteOn,CMDMM.buttons[0],CMDMM.varStorage.knobAssignment);break; //[High,Mid,Low,Quickeffect]=orange, [Gain,High,Mid,Low] = blue; [(PLAYdifferently four knob EQ; not implemented yet)] = blink;
		case 3:
			midi.sendShortMsg(MIDI.noteOn,CMDMM.buttons[0],CMDMM.varStorage.channelSequence===[3,1,2,4]?0x01:0x00); break; //[3,1,2,4]ChannelAssignment = blue; [1,2,3,4] = orange;
	}
};
CALLBACK.fxButton = function () {
	for (var channel=1;channel<=4;channel++) {
		switch (CMDMM.getLevel()) {
			case 0:
				for (var fxUnit=1; fxUnit<=2;fxUnit++) {
					effectUnitValue = engine.getValue("[EffectRack1_EffectUnit"+fxUnit+"]", "group_[Channel"+CMDMM.varStorage.channelSequence[channel-1]+"]_enable");
					midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[(channel-1)*2+fxUnit],effectUnitValue?CMDMM.on:CMDMM.off);
					//                                                 ^strange but working solution to get values from 1 to 8
				}
				break;
			case 1:
				orientationButton = engine.getValue("[Channel"+CMDMM.varStorage.channelSequence[channel-1]+"]","orientation");
				midi.sendShortMsg(MIDI.noteOn,CMDMM.buttons[(channel)*2-1],orientationButton===0?CMDMM.on:CMDMM.off);
				midi.sendShortMsg(MIDI.noteOn,CMDMM.buttons[(channel)*2-0],orientationButton===2?CMDMM.on:CMDMM.off);
				break;
			case 2:
				for (var fxUnit=3; fxUnit<=4;fxUnit++) {
					effectUnitValue = engine.getValue("[EffectRack1_EffectUnit"+fxUnit+"]", "group_[Channel"+CMDMM.varStorage.channelSequence[channel-1]+"]_enable");
					midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[(channel-1)*2+fxUnit-2],effectUnitValue?CMDMM.on:CMDMM.off);
					//                                                 ^strange but working solution to get values from 1 to 8
				}
				break;
			case 3:
				for (var i = 1; i <= 8;i++){
					midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[i], CMDMM.off);
				}
				break;

		}
	}
}

CMDMM.updateLEDs = function () {
	//could be better implemented, but its easy and it works.
	CALLBACK.middleButton();
	CALLBACK.fxButton();
	CALLBACK.cue();
};
FUNCTIONS.ignoreNextValue = function () {
	for (var i = 1; i<=4;i++) {
		engine.softTakeoverIgnoreNextValue("[QuickEffectRack1_[Channel"+i+"]]", "super1", true);
		engine.softTakeoverIgnoreNextValue("[Channel"+i+"]","rate",true);
		engine.softTakeoverIgnoreNextValue("[Channel"+i+"]","volume",true);
		engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit"+i+"]","mix",true);
		engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit"+i+"]","super1", true);
		for (var ii=1;ii<=3;ii++) {
			engine.softTakeoverIgnoreNextValue("[EqualizerRack1_[Channel"+i+"]_Effect1]","parameter"+ii,true);
			engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit"+i+"_Effect"+ii+"]","meta",true);
		}
	}
};


FUNCTIONS.mapButtons = function () {
	CMDMM.middleButton = function (channel, control, value, status, group) {
		CMDMM.modes(function(){
			print("toggled Library");
			script.toggleControl("[Master]","maximize_library");
			//engine.setValue("[Master]","maximize_library", 1);
			//maximize Library
		},
		function(){
			print("reset Crossfader");
			engine.setValue("[Master]","crossfader",0);
			//reset Crossfader (NOT WORKING)
		},
		function(){
			FUNCTIONS.cycleKnobAssignment();
			CALLBACK.middleButton();
		},
		function(){
			CMDMM.varStorage.channelSequence = CMDMM.varStorage.channelSequence[0] === 3 ? [1,2,3,4]:[3,1,2,4]; // not very expandable but it works while Array===Array doesnt;
			//CMDMM.varStorage.channelSequence = _.isEqual(CMDMM.varStorage.channelSequence, [3,1,2,4]) ? [1,2,3,4]:[3,1,2,4];
			print(CMDMM.varStorage.channelSequence);
			FUNCTIONS.mapWholeController(true);
			FUNCTIONS.enableSoftTakeover();
			CALLBACK.registerCallbacks();
			CMDMM.updateLEDs();
			print(CMDMM.varStorage.channelSequence);
		}
		);
	};
	CMDMM.fxButton = function (channel, control, value, status, group) {
		button = CMDMM.buttons.indexOf(control); //returns integers from 1 to 8
		mixxxChannel = CMDMM.varStorage.channelSequence[Math.floor((button-1)/2)];
		CMDMM.modes(
			function(){
				effectUnit = button%2 == 0 ? 2:1; //checks if buttonnumber is even, if true, return effectUnit 2 if false, return effectUnit 1;
				// maps button to range [0;7], divides and floors it (result: [0;3]). That value is being used to lookup the right channel.
				script.toggleControl("[EffectRack1_EffectUnit"+effectUnit+"]","group_[Channel"+mixxxChannel+"]_enable");
			},
			function(){
				mixxxChannel=CMDMM.varStorage.channelSequence[Math.floor((button-1)/2)];
				channelButton = button%2===0?2:0;
				if (engine.getValue("[Channel"+mixxxChannel+"]","orientation")===channelButton) {
					engine.setValue("[Channel"+mixxxChannel+"]","orientation", 1);
				} else {
					engine.setValue("[Channel"+mixxxChannel+"]","orientation", channelButton); //checks if buttonnumber is even, if true, return Left if false, return right;
				}

			},
			function(){
				effectUnit = button%2 == 0 ? 4:3; //checks if buttonnumber is even, if true, return effectUnit 4 if false, return effectUnit 3;
				// maps button to range [0;7], divides and floors it (result: [0;3]). That value is being used to lookup the right channel.
				script.toggleControl("[EffectRack1_EffectUnit"+effectUnit+"]","group_[Channel"+mixxxChannel+"]_enable");
			}
		);
	};
	CMDMM.cue = function (channel, control, value, status, group) {
		cueChannel = CMDMM.varStorage.channelSequence[control - 0x30];
		CMDMM.modes(
			function(){
				script.toggleControl("[Channel"+cueChannel+"]", "pfl");
			},
			function(){
				//load track into selected channel	
				engine.setParameter("[Channel"+cueChannel+"]", "LoadSelectedTrack", 1);
			},
			function() {
				//enable pfl for EffectN N = number of channel (a bit weird, but it works)
				script.toggleControl("[EffectRack1_EffectUnit"+cueChannel+"]", "group_[Headphone]_enable");
			},
			//DOESNT WORK PLEASE FIX!!!
			//Maybe an issue with softTakeoverIgnoreNextValue which gets called whenever shift or ctrl is pressed
			function() {
				engine.setValue("[Channel"+cueChannel+"]", "rate", 0);
				print("reset rate on channel"+cueChannel+"to "+engine.getValue("[Channel"+cueChannel+"]", "rate"));
				engine.setValue("[Channel"+cueChannel+"]", "sync_mode", 0);
				print("reset sync_mode on channel"+cueChannel+"to "+engine.getValue("[Channel"+cueChannel+"]", "sync_mode"));
				CALLBACK.cue();
			}
		);
	};
};
FUNCTIONS.assignKnobsWithEffect = function () {
	CMDMM.knob = function (channel, control, value, status, group) {
		realChannel = (control - 0x06)%4;
		parameterNum = Math.floor((control-0x06)/4)+1;
		parameter = (parameterNum === 4) ? "mix":"meta";
		mixxxChannel = CMDMM.varStorage.channelSequence[realChannel];
		mixxxGroup = ("[EffectRack1_EffectUnit"+mixxxChannel+((parameterNum === 4) ? "":("_Effect"+parameterNum))+"]");
		engine.setParameter(mixxxGroup, parameter, value/127);
	};
};

FUNCTIONS.assignKnobsWithQuickEffect = function () {
	CMDMM.knob = function (channel, control, value, status, group) {
		realChannel = (control - 0x06)%4;
		parameterNum = -(Math.floor((control - 0x06)/4)-3); //get the horizontal row of the button and translates it to the number
		parameter = (parameterNum === 0 ? "super1":("parameter"+parameterNum));
		mixxxChannel = CMDMM.varStorage.channelSequence[realChannel];
		mixxxGroup = (parameterNum === 0) ? ("[QuickEffectRack1_[Channel"+mixxxChannel+"]]"):"[EqualizerRack1_[Channel"+mixxxChannel+"]_Effect1]";
		engine.setParameter(mixxxGroup, parameter, value/127);
	};
};

FUNCTIONS.assignKnobsWithGain = function() {
	CMDMM.knob = function (channel, control, value, status, group) {
		realChannel = (control - 0x06)%4;
		parameterNum = -(Math.floor((control - 0x06)/4)-2)+2; //get the horizontal row of the button and translates it to the number
		parameter = (parameterNum === 4 ? "pregain":("parameter"+parameterNum));
		mixxxChannel = CMDMM.varStorage.channelSequence[realChannel];
		mixxxGroup = (parameterNum === 4) ? ("[Channel"+mixxxChannel+"]"):"[EqualizerRack1_[Channel"+mixxxChannel+"]_Effect1]";
		engine.setParameter(mixxxGroup, parameter, value/127);
	};

};
/*
FUNCTIONS.assignKnobsWithFourBandEQ = function () {
	throw ("\n\nPLAYdifferently Model 1 style EQ effect not implemented yet by Be!\nhttps://bugs.launchpad.net/mixxx/+bug/1581721\n\nPLEASE PRESS IGNORE!");
};
*/
FUNCTIONS.mapWholeController = function(disableCycleKnob) {
	//init buttons, faders, knobs
	FUNCTIONS.mapFxKnobs();
	//Cue Vol/Mix already mapped in the XML
	FUNCTIONS.assignLibraryFunctions();
	if (!disableCycleKnob){FUNCTIONS.cycleKnobAssignment();}
	FUNCTIONS.mapButtons();
	FUNCTIONS.mapFaders();
	//crossfader already mapped in the XML
	CMDMM.updateLEDs();
}

FUNCTIONS.cycleKnobAssignment = function (){
	// 0 = [High,Mid,Low,Quickeffect]; 1 = [Gain,High,Mid,Low]; 2 = [Effect1Meta,Effect2Meta,Effect3Meta,mix]; 3 = [(be's four knob EQ; not implemented yet)];
	switch (CMDMM.varStorage.knobAssignment) {
		/*
		case 3:
			CMDMM.varStorage.knobAssignment = 0;
			FUNCTIONS.assignKnobsWithFourBandEQ();
			break;
		*/
		case 2:
			CMDMM.varStorage.knobAssignment=0;
			FUNCTIONS.assignKnobsWithEffect();
			break;
		case 1:
			CMDMM.varStorage.knobAssignment++;
			FUNCTIONS.assignKnobsWithGain();
			break;
		case 0:
			CMDMM.varStorage.knobAssignment++;
			FUNCTIONS.assignKnobsWithQuickEffect();
			break;
		default:
			print("INVALID KNOB ASSIGNMENT ("+CMDMM.varStorage.knobAssignment+")!");
			break;
	}
};

FUNCTIONS.mapFaders = function () {
	CMDMM.fader = function (channel, control, value, status, group) {
		CMDMM.modes(function () {
				engine.setParameter("[Channel"+CMDMM.varStorage.channelSequence[control-0x30]+"]","volume", value/127);
			},
			function () {
				engine.setParameter("[Channel"+CMDMM.varStorage.channelSequence[control-0x30]+"]","rate", value/127);
			},
			function () {
				//empty so fadermovement can be ignored while pressing ctrl
			}
		);
	};
};

FUNCTIONS.mapFxKnobs = function () {
	CMDMM.out = function (channel, control, value, status, group) {
		CMDMM.modes(
			function(){
				engine.setParameter("[EffectRack1_EffectUnit"+control+"]","super1", value/127);
			},
			function(){
				engine.setParameter("[EffectRack1_EffectUnit"+control+"]","mix", value/127);
			},
			function(){
				engine.setParameter("[EffectRack1_EffectUnit"+(control+2)+"]","super1", value/127);
			},
			function(){
				engine.setParameter("[EffectRack1_EffectUnit"+(control+2)+"]","mix", value/127);
			}
		);
	};
};

FUNCTIONS.enableSoftTakeover = function () {
	for (var i = 1; i<=4;i++) {
		engine.softTakeover("[QuickEffectRack1_[Channel"+i+"]]", "super1", true);
		engine.softTakeover("[Channel"+i+"]","rate",true);
		engine.softTakeover("[Channel"+i+"]","volume",true);
		engine.softTakeover("[EffectRack1_EffectUnit"+i+"]","mix",true);
		engine.softTakeover("[EffectRack1_EffectUnit"+i+"]","super1", true);
		for (var ii=1;ii<=3;ii++) {
			engine.softTakeover("[EqualizerRack1_[Channel"+i+"]_Effect1]","parameter"+ii,true);
			engine.softTakeover("[EffectRack1_EffectUnit"+i+"_Effect"+ii+"]","meta",true);
		}
	}

};
FUNCTIONS.resetColor = function () {
	for (var i = CMDMM.buttons.length - 1; i >= 0; i--) {
		midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[i], CMDMM.off);
	}
};

FUNCTIONS.assignLibraryFunctions = function () {
	CMDMM.libraryEncoder = function (channel, control, value, status, group) {
		CMDMM.modes(
			function () {
				print("using deprecated SelectTrackKnob as long there is no way to select from just the tracks!")
				engine.setValue("[Playlist]", "SelectTrackKnob", value>0x40 ? 1:-1);
			},
			function () {
				//navigate preview track
				engine.setParameter("[PreviewDeck1]", "beatjump", (value > 0x40) ? 16 : -16);
			},
			function () {

			}
		);
	}
	CMDMM.libraryButton = function (channel, control, value, status, group) {
		CMDMM.modes(
			function (){
				engine.setParameter("[PreviewDeck1]","LoadSelectedTrack",1);
			},
			function (){
				engine.setParameter("[PreviewDeck1]","play", 1)
			}
		);
	};
};

CALLBACK.vuMeterL =  function (value, group, control) {
    midi.sendShortMsg(0xB4, 80, (value * 15) + 48);
};
CALLBACK.vuMeterR =  function (value, group, control) {
    midi.sendShortMsg(0xB4, 81, (value * 15) + 48);
};



CALLBACK.registerCallbacks = function () {
	//VuMeters
	engine.makeConnection("[Master]","VuMeterL",CALLBACK.vuMeterL);
	engine.makeConnection("[Master]","VuMeterR",CALLBACK.vuMeterR);
	//engine.makeConnection("[Master]","maximize_library",CALLBACK.middleButton);// doesnt work, help?
	//cueButtons (pfl)
	for (i=1;i<=4;i++){
		engine.makeConnection("[Channel"+i+"]","pfl",CALLBACK.cue);
		engine.makeConnection("[Channel"+i+"]","play",CALLBACK.cue);
		engine.makeConnection("[Channel"+i+"]","orientation",CALLBACK.fxButton);
		for (var ii=1;ii<=4;ii++){
			engine.makeConnection("[EffectRack1_EffectUnit"+i+"]","group_[Channel"+ii+"]_enable",CALLBACK.fxButton);
		}
		engine.makeConnection("[EffectRack1_EffectUnit"+i+"]","group_[Headphone]_enable",CALLBACK.cue);
	}
};

CMDMM.init = function () {
	FUNCTIONS.resetColor();
	FUNCTIONS.mapWholeController();
	FUNCTIONS.enableSoftTakeover();
	CALLBACK.registerCallbacks();
	CMDMM.updateLEDs();
};

CMDMM.shutdown = function () {
	//reset Button LEDs
	FUNCTIONS.resetColor();
}