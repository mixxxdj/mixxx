////////////////////////////////////////
//      Numark V7 control script      //
//    CopyLeft 2012, Mike Bucceroni   //
// ^that means do what ever you want^ //
//       made for Mixxx 1.11.x        //
////////////////////////////////////////
 
function NumarkV7() {}

/////////////////////////////////
//                             //
//   Customization Variables   //
//                             //
/////////////////////////////////
 
NumarkV7.RateRanges = [ 0.0, 0.08, 0.1, 0.16, 0.5, 1.0 ];
NumarkV7.MotorOnLoad = false; //set to "true" to diasble motor on load
NumarkV7.ScratchDisableDefault = false; //set to "true" to disable scratching when the motors are runnning.
NumarkV7.loop_modeOnLoad = "Manual"; //("Manual", "Auto")
NumarkV7.MotorSpeed = true; //Record RPM (true = "33 1/3",false = "45")

//////////////////////////
//                      //
//   Global Variables   //
//                      //
//////////////////////////

NumarkV7.Deck = true; // true = Deck A, false = Deck B
NumarkV7.ShiftToggleA = false; // true = toggle on, false on toggle
NumarkV7.ShiftHoldA = false; // true = held down, false on release
NumarkV7.ShiftToggleB = false; // true = toggle on, false on toggle
NumarkV7.ShiftHoldB = false; // true = held down, false on release
NumarkV7.loop_modeA = false; //(auto=true, manual=false)
NumarkV7.loop_modeB = false; //(auto=true, manual=false)
NumarkV7.RangeArray = 1;//default range array setting (0,1,2,3)
NumarkV7.MotorDisableA = false; //disables the motor, sets "jog mode"
NumarkV7.MotorDisableB = false; //disables the motor, sets "jog mode"
NumarkV7.FxSliderACoarseParse = 0.000000;//records value
NumarkV7.FxSliderAFineParse = 0.000000;//records value
NumarkV7.FxSliderBCoarseParse = 0.000000;//records value
NumarkV7.FxSliderBFineParse = 0.000000;//records value
NumarkV7.PitchACoarseParse = 0.000000;//records value
NumarkV7.PitchAFineParse = 0.000000;//records value
NumarkV7.PitchBCoarseParse = 0.000000;//records value
NumarkV7.PitchBFineParse = 0.000000;//records value
NumarkV7.PitchAPause = 0; //pauses scratching while changing pitch
NumarkV7.PitchBPause = 0; //pauses scratching while changing pitch
NumarkV7.PitchAPauseOn = false; //true is timer is running.
NumarkV7.PitchBPauseOn = false; //true is timer is running.
NumarkV7.ScratchDisableA = false; //set to "true" to disable scratching when the motors are runnning.
NumarkV7.ScratchDisableB = false; //set to "true" to disable scratching when the motors are runnning.
NumarkV7.ScratchDiffA = 0x00; //records last value
NumarkV7.ScratchDiffB = 0x00; //records last value
NumarkV7.RPM = 33+1/3; //Motor-Scratch rpm

//////////////////////////
//                      //
//    Initialization    //
//           &          //
//       Shutdown       //
//                      //
//////////////////////////

NumarkV7.init = function () {
    //flash LED's
    NumarkV7.FlashAllLED(1000);
    engine.beginTimer(3500, "NumarkV7.init2()", true);
}
NumarkV7.init2 = function () {
    //Tap Light
    midi.sendShortMsg(0xB0,0x11,0x01);//tap A
	midi.sendShortMsg(0xB0,0x28,0x01);//tap B
    midi.sendShortMsg(0xB0,0x03,0x01);//prepare
    midi.sendShortMsg(0xB0,0x04,0x01);//files
    midi.sendShortMsg(0xB0,0x05,0x01);//crates
    //Apply customizations
    if (NumarkV7.loop_modeOnLoad == "Auto") {
        NumarkV7.loop_modeA = true;
		NumarkV7.loop_modeB = true;
        midi.sendShortMsg(0xB0,0x18,0x01);
		midi.sendShortMsg(0xB0,0x2F,0x01);
    }
    else {
        midi.sendShortMsg(0xB0,0x18,0x7F);
		midi.sendShortMsg(0xB0,0x2F,0x7F);
    }
	if (NumarkV7.MotorOnLoad == true) {
        NumarkV7.MotorDisableA = true;
		NumarkV7.MotorDisableB = true;
        midi.sendShortMsg(0xB0,0x12,0x01);
		midi.sendShortMsg(0xB0,0x29,0x01);
    }
	if (!NumarkV7.MotorSpeed) {
		NumarkV7.RPM = 45;
	}
	if (NumarkV7.ScratchDisableDefault) {
		NumarkV7.ScratchDisableA = true;
		NumarkV7.ScratchDisableB = true;
	}
    //soft takeover enable
    //Motor Speed
    if (NumarkV7.MotorSpeed == true){
        midi.sendShortMsg(0xB0,0x45,0x00);//33 1/3 rpm
    }
    else {
        midi.sendShortMsg(0xB0,0x45,0x01);//45 rpm
    }
    //connect controls
	engine.connectControl("[Channel1]", "flanger", "NumarkV7.FxButtonA");
	engine.connectControl("[Channel2]", "flanger", "NumarkV7.FxButtonB");
	engine.connectControl("[Flanger]", "lfoPeriod", "NumarkV7.FxParam");
	engine.connectControl("[Channel1]", "reverse", "NumarkV7.ReverseA");
	engine.connectControl("[Channel2]", "reverse", "NumarkV7.ReverseB");
    engine.connectControl("[Channel1]", "play", "NumarkV7.Play2A");
	engine.connectControl("[Channel2]", "play", "NumarkV7.Play2B");
	engine.connectControl("[Channel1]", "cue_default", "NumarkV7.CueLEDA");
	engine.connectControl("[Channel2]", "cue_default", "NumarkV7.CueLEDB");
	engine.connectControl("[Channel1]", "beatsync", "NumarkV7.SyncA");
	engine.connectControl("[Channel2]", "beatsync", "NumarkV7.SyncB");
    engine.connectControl("[Channel1]", "rate", "NumarkV7.PitchAMotor");
	engine.connectControl("[Channel2]", "rate", "NumarkV7.PitchBMotor");
	engine.connectControl("[Channel1]", "keylock", "NumarkV7.KeylockA");
	engine.connectControl("[Channel2]", "keylock", "NumarkV7.KeylockB");
	engine.connectControl("[Channel1]", "LoadSelectedTrack", "NumarkV7.TempoLeanLED");
	engine.connectControl("[Channel2]", "LoadSelectedTrack", "NumarkV7.TempoLeanLED");
	engine.connectControl("[Playlist]", "LoadSelectedIntoFirstStopped", "NumarkV7.TempoLeanLED");
	engine.connectControl("[Channel1]", "loop_enabled", "NumarkV7.loopOnOffA");
	engine.connectControl("[Channel2]", "loop_enabled", "NumarkV7.loopOnOffB");
    engine.connectControl("[Channel1]", "loop_halve", "NumarkV7.loop_halveA");
	engine.connectControl("[Channel2]", "loop_halve", "NumarkV7.loop_halveB");
    engine.connectControl("[Channel1]", "loop_double", "NumarkV7.loop_doubleA");
	engine.connectControl("[Channel2]", "loop_double", "NumarkV7.loop_doubleB");
	engine.connectControl("[Channel1]", "hotcue_1_enabled", "NumarkV7.ShiftA");
	engine.connectControl("[Channel1]", "hotcue_2_enabled", "NumarkV7.ShiftA");
	engine.connectControl("[Channel1]", "hotcue_3_enabled", "NumarkV7.ShiftA");
	engine.connectControl("[Channel1]", "hotcue_4_enabled", "NumarkV7.ShiftA");
	engine.connectControl("[Channel1]", "hotcue_5_enabled", "NumarkV7.ShiftA");
	engine.connectControl("[Channel2]", "hotcue_1_enabled", "NumarkV7.ShiftB");
	engine.connectControl("[Channel2]", "hotcue_2_enabled", "NumarkV7.ShiftB");
	engine.connectControl("[Channel2]", "hotcue_3_enabled", "NumarkV7.ShiftB");
	engine.connectControl("[Channel2]", "hotcue_4_enabled", "NumarkV7.ShiftB");
	engine.connectControl("[Channel2]", "hotcue_5_enabled", "NumarkV7.ShiftB");
	
    //set rate range
    engine.setValue("[Channel1]","rateRange",NumarkV7.RateRanges[NumarkV7.RangeArray]);
	engine.setValue("[Channel2]","rateRange",NumarkV7.RateRanges[NumarkV7.RangeArray]);
	
    //trigger LEDs
    engine.trigger("[Channel1]", "rate");
    engine.trigger("[Flanger]", "lfoPeriod");
    engine.trigger("[Channel1]", "reverse");
    engine.trigger("[Channel1]", "flanger");
    engine.trigger("[Channel1]", "loop_halve");
    engine.trigger("[Channel1]", "loop_double");
    engine.trigger("[Channel1]", "cue_default");
    engine.trigger("[Channel1]", "beatsync");
	engine.trigger("[Channel2]", "rate");
    engine.trigger("[Channel2]", "reverse");
    engine.trigger("[Channel2]", "flanger");
    engine.trigger("[Channel2]", "loop_halve");
    engine.trigger("[Channel2]", "loop_double");
    engine.trigger("[Channel2]", "cue_default");
    engine.trigger("[Channel2]", "beatsync");
}
NumarkV7.shutdown = function () {
    NumarkV7.OffAllLED();
    NumarkV7.MotorOffA();
	NumarkV7.MotorOffB();
}

//////////////////////////
//                      //
//  Group LED Controls  //
//                      //
//////////////////////////

NumarkV7.FlashAllLED = function (sustain) {
    midi.sendShortMsg(0xB0,0x39,0x01);
    engine.beginTimer(sustain, "NumarkV7.OffAllLED()", true);
    engine.beginTimer(sustain, "NumarkV7.RunLED()", true);
}
NumarkV7.OffAllLED = function () {
    midi.sendShortMsg(0xB0,0x39,0x00);
}
NumarkV7.RunLED = function () {
    NumarkV7.RunLED1()
    engine.beginTimer(100, "NumarkV7.RunLED2()", true);
    engine.beginTimer(200, "NumarkV7.RunLED3()", true);
    engine.beginTimer(300, "NumarkV7.RunLED4()", true);
    engine.beginTimer(400, "NumarkV7.RunLED5()", true);
    engine.beginTimer(500, "NumarkV7.RunLED6()", true);
    engine.beginTimer(600, "NumarkV7.RunLED7()", true);
    engine.beginTimer(700, "NumarkV7.RunLED8()", true);
    engine.beginTimer(800, "NumarkV7.RunLED9()", true);
    engine.beginTimer(900, "NumarkV7.RunLED10()", true);
    engine.beginTimer(1000, "NumarkV7.RunLED11()", true);
    engine.beginTimer(1100, "NumarkV7.RunLED12()", true);
    engine.beginTimer(1200, "NumarkV7.RunLED11()", true);
    engine.beginTimer(1300, "NumarkV7.RunLED10()", true);
    engine.beginTimer(1400, "NumarkV7.RunLED9()", true);
    engine.beginTimer(1500, "NumarkV7.RunLED8()", true);
    engine.beginTimer(1600, "NumarkV7.RunLED7()", true);
    engine.beginTimer(1700, "NumarkV7.RunLED6()", true);
    engine.beginTimer(1800, "NumarkV7.RunLED5()", true);
    engine.beginTimer(1900, "NumarkV7.RunLED4()", true);
    engine.beginTimer(2000, "NumarkV7.RunLED3()", true);
    engine.beginTimer(2100, "NumarkV7.RunLED2()", true);
    engine.beginTimer(2200, "NumarkV7.RunLED13()", true);
}
NumarkV7.RunLED1 = function () {
    midi.sendShortMsg(0xB0,0x34,0x01);
    midi.sendShortMsg(0xB0,0x36,0x01);
	midi.sendShortMsg(0xB0,0x35,0x01);
}
NumarkV7.RunLED2 = function () {
    midi.sendShortMsg(0xB0,0x34,0x02);
    midi.sendShortMsg(0xB0,0x35,0x02);
	midi.sendShortMsg(0xB0,0x36,0x02);
}
NumarkV7.RunLED3 = function () {
    midi.sendShortMsg(0xB0,0x34,0x03);
    midi.sendShortMsg(0xB0,0x35,0x03);
	midi.sendShortMsg(0xB0,0x36,0x03);
}
NumarkV7.RunLED4 = function () {
    midi.sendShortMsg(0xB0,0x34,0x04);
    midi.sendShortMsg(0xB0,0x35,0x04);
	midi.sendShortMsg(0xB0,0x36,0x04);
}
NumarkV7.RunLED5 = function () {
    midi.sendShortMsg(0xB0,0x34,0x05);
	midi.sendShortMsg(0xB0,0x35,0x05);
    midi.sendShortMsg(0xB0,0x36,0x05);
}
NumarkV7.RunLED6 = function () {
    midi.sendShortMsg(0xB0,0x34,0x06);
	midi.sendShortMsg(0xB0,0x35,0x06);
    midi.sendShortMsg(0xB0,0x36,0x06);
}
NumarkV7.RunLED7 = function () {
    midi.sendShortMsg(0xB0,0x34,0x07);
	midi.sendShortMsg(0xB0,0x35,0x07);
    midi.sendShortMsg(0xB0,0x36,0x07);
}
NumarkV7.RunLED8 = function () {
    midi.sendShortMsg(0xB0,0x34,0x08);
	midi.sendShortMsg(0xB0,0x35,0x08);
    midi.sendShortMsg(0xB0,0x36,0x08);
}
NumarkV7.RunLED9 = function () {
    midi.sendShortMsg(0xB0,0x34,0x09);
	midi.sendShortMsg(0xB0,0x35,0x09);
    midi.sendShortMsg(0xB0,0x36,0x09);
}
NumarkV7.RunLED10 = function () {
    midi.sendShortMsg(0xB0,0x34,0x0A);
	midi.sendShortMsg(0xB0,0x35,0x0A);
    midi.sendShortMsg(0xB0,0x36,0x0A);
}
NumarkV7.RunLED11 = function () {
    midi.sendShortMsg(0xB0,0x34,0x0B);
	midi.sendShortMsg(0xB0,0x35,0x0B);
    midi.sendShortMsg(0xB0,0x36,0x0B);
}
NumarkV7.RunLED12 = function () {
    midi.sendShortMsg(0xB0,0x34,0x0C);
	midi.sendShortMsg(0xB0,0x35,0x0C);
    midi.sendShortMsg(0xB0,0x36,0x0C);
}
NumarkV7.RunLED13 = function () {
    midi.sendShortMsg(0xB0,0x34,0x00);
	midi.sendShortMsg(0xB0,0x35,0x00);
    midi.sendShortMsg(0xB0,0x36,0x00);    
}

///////////////////////////
//                       //
//       Functions       //
//                       //
///////////////////////////

//Fx Controls
NumarkV7.FxButtonA = function () {
    if ((engine.getValue("[Channel1]","flanger"))==1){
        midi.sendShortMsg(0xB0,0x3C,0x01);
    }
    else{
        midi.sendShortMsg(0xB0,0x3C,0x00);
    }
}
NumarkV7.FxButtonB = function () {
    if ((engine.getValue("[Channel2]","flanger"))==1){
        midi.sendShortMsg(0xB0,0x3D,0x01);
    }
    else{
        midi.sendShortMsg(0xB0,0x3D,0x00);
    }
}
NumarkV7.FxSliderACoarse = function (channel, control, value, status, group) {
    NumarkV7.FxSliderACoarseParse = script.absoluteLin(value, 0, 1);
}
NumarkV7.FxSliderAFine = function (channel, control, value, status, group) {
	NumarkV7.FxSliderAFineParse = ((script.absoluteLin(value, 0, 1))/127);
	NumarkV7.FxSliderA();
}
NumarkV7.FxSliderA = function () {
	var currentvalue = (NumarkV7.FxSliderAFineParse+NumarkV7.FxSliderACoarseParse); 
    engine.setValue("[Flanger]","lfoDepth", currentvalue);
}
NumarkV7.FxSliderBCoarse = function (channel, control, value, status, group) {
    NumarkV7.FxSliderBCoarseParse = script.absoluteLin(value, 0, 1);
}
NumarkV7.FxSliderBFine = function (channel, control, value, status, group) {
	NumarkV7.FxSliderBFineParse = ((script.absoluteLin(value, 0, 1))/127);
	NumarkV7.FxSliderB();
}
NumarkV7.FxSliderB = function () {
	var currentvalue = (NumarkV7.FxSliderBFineParse+NumarkV7.FxSliderBCoarseParse); 
    engine.setValue("[Flanger]","lfoDepth", currentvalue);
}
NumarkV7.FxParam = function (channel, control, value, status, group) {
	var currentvalue = engine.getValue("[Flanger]","lfoPeriod" );
	if ((value == 0x01)&&(engine.getValue("[Flanger]","lfoPeriod") < 2000000)){
		engine.setValue("[Flanger]","lfoPeriod",currentvalue+(1950000/127));
	}
	else{
		if ((value == 0x7F)&&(engine.getValue("[Flanger]","lfoPeriod") > 0)){
			engine.setValue("[Flanger]","lfoPeriod",currentvalue-(1950000/127));
		}
	}
	var value = engine.getValue("[Flanger]", "lfoPeriod");
    var add = NumarkV7.Peak11(value,50000,2000000);
    midi.sendShortMsg(0xB0,0x34,0x00+add);
	midi.sendShortMsg(0xB0,0x35,0x00+add);
}
NumarkV7.Peak11 = function (value, low, high) {
    var LEDs = 1;
    var halfInterval = ((high-low)/9)/2;
    value=value.toFixed(4);
    if (value>low) LEDs++;
    if (value>low+halfInterval) LEDs++;
    if (value>low+halfInterval*3) LEDs++;
    if (value>low+halfInterval*5) LEDs++;
    if (value>low+halfInterval*7) LEDs++;
    if (value>low+halfInterval*9) LEDs++;
    if (value>low+halfInterval*11) LEDs++;
    if (value>low+halfInterval*13) LEDs++;
    if (value>low+halfInterval*15) LEDs++;
    if (value>=high) LEDs++;
    return LEDs;
} 
NumarkV7.FxSelect = function (channel, control, value, status, group) {
	var currentvalue = engine.getValue("[Flanger]","lfoDelay" );
	if ((value == 0x01)&&(status == 0xB0)&&(currentvalue < 10000)){
		engine.setValue("[Flanger]","lfoDelay",currentvalue+(9950/64));
	}
	else{
		if ((value == 0x7F)&&(status == 0xB0)&&(currentvalue > 0)){
			engine.setValue("[Flanger]","lfoDelay",currentvalue-(9950/64));
		}
		else{
			if ((value == 0x7F)&&(status == 0x90)){
				engine.setValue("[Flanger]","lfoDelay",4950);
			}
		}
	}
}

//Reverse Switch
NumarkV7.ReverseA = function (channel, control, value, status, group) {
	if (value == 0x7F){
		engine.setValue("[Channel1]","reverse",1);
		midi.sendShortMsg(0xB0,0x46,0x01);
	}
	else{
		if (value == 0x00){
			engine.setValue("[Channel1]","reverse",0);
			midi.sendShortMsg(0xB0,0x46,0x00);
		}
		else{
			if ((engine.getValue("[Channel1]","reverse"))==1){
				midi.sendShortMsg(0xB0,0x46,0x01);
			}
			else{
				midi.sendShortMsg(0xB0,0x46,0x00);
			}
		}
	}
}
NumarkV7.ReverseB = function (channel, control, value, status, group) {
	if (value == 0x7F){
		engine.setValue("[Channel2]","reverse",1);
		midi.sendShortMsg(0xB0,0x50,0x01);
	}
	else{
		if (value == 0x00){
			engine.setValue("[Channel2]","reverse",0);
			midi.sendShortMsg(0xB0,0x50,0x00);
		}
		else{
			if ((engine.getValue("[Channel2]","reverse"))==1){
				midi.sendShortMsg(0xB0,0x50,0x01);
			}
			else{
				midi.sendShortMsg(0xB0,0x50,0x00);
			}
		}
	}
}

//Motor Off Button
NumarkV7.MotorOffButtonA = function (channel, control, value, status, group) {
    if ((NumarkV7.ShiftHoldA)&&(value == 0x7F)) {
		if (!NumarkV7.ScratchDisableA) {
			NumarkV7.ScratchDisableA = true;
			engine.scratchDisable(1, false);
		}
		else {
			NumarkV7.ScratchDisableA = false;
			if (!NumarkV7.MotorDisableA) {
				engine.scratchEnable(1, 37056, NumarkV7.RPM, (1.0), (0.27), false);
			}
		}
		NumarkV7.ShiftToggleA = false;
		NumarkV7.ShiftA;
	}
	else {
		if ((value == 0x7F)&&(NumarkV7.MotorDisableA == false)){
			NumarkV7.MotorDisableA = true;
			midi.sendShortMsg(0xB0,0x12,0x01);
			NumarkV7.Play2A();
		}
		else {
			if ((value == 0x7F)&&(NumarkV7.MotorDisableA == true)){
				NumarkV7.MotorDisableA = false;
				midi.sendShortMsg(0xB0,0x12,0x00);
				NumarkV7.Play2A();
			}
		}
	}
}
NumarkV7.MotorOffButtonB = function (channel, control, value, status, group) {
	if ((NumarkV7.ShiftHoldB)&&(value == 0x7F)) {
		if (!NumarkV7.ScratchDisableB) {
			NumarkV7.ScratchDisableB = true;
			engine.scratchDisable(2, false);
		}
		else {
			NumarkV7.ScratchDisableB = false;
			if (!NumarkV7.MotorDisableB) {
				engine.scratchEnable(2, 37056, NumarkV7.RPM, (1.0), (0.27), false);
			}
		}
		NumarkV7.ShiftToggleB = false;
		NumarkV7.ShiftB;
	}
	else {
		if ((value == 0x7F)&&(NumarkV7.MotorDisableB == false)){
			NumarkV7.MotorDisableB = true;
			midi.sendShortMsg(0xB0,0x29,0x01);
			NumarkV7.Play2B();
		}
		else {
			if ((value == 0x7F)&&(NumarkV7.MotorDisableB == true)){
				NumarkV7.MotorDisableB = false;
				midi.sendShortMsg(0xB0,0x29,0x00);
				NumarkV7.Play2B();
			}
		} 
    }
}

//Start-Stop Knobs
NumarkV7.MotorStartKnobA = function (channel, control, value, status, group) {
	midi.sendShortMsg(0xB0,0x47,value);
}
NumarkV7.MotorStartKnobB = function (channel, control, value, status, group) {
	midi.sendShortMsg(0xB0,0x51,value);
}
NumarkV7.MotorStopKnobA = function (channel, control, value, status, group) {
	midi.sendShortMsg(0xB0,0x48,value);
}
NumarkV7.MotorStopKnobB = function (channel, control, value, status, group) {
	midi.sendShortMsg(0xB0,0x52,value);
}

//Play-Cue-Sync Buttons
NumarkV7.PlayA = function (channel, control, value, status, group) {
    var currentlyPlaying = engine.getValue("[Channel1]","play");
    if ((currentlyPlaying == 0)&&(value == 0x7F)){
        engine.setValue("[Channel1]","play",1);
        midi.sendShortMsg(0xB0,0x09,0x01);
        NumarkV7.MotorOnA();
    }
    else {
        if ((currentlyPlaying == 1)&&(value == 0x7F)){
            engine.setValue("[Channel1]","play",0);
            midi.sendShortMsg(0xB0,0x09,0x00);
            NumarkV7.MotorOffA();
        }
        
    }
}
NumarkV7.Play2A = function (channel, control, value, status, group) {
    var currentlyPlaying = engine.getValue("[Channel1]","play");
    if (currentlyPlaying == 1){
        midi.sendShortMsg(0xB0,0x09,0x01);
        NumarkV7.MotorOnA();
        engine.trigger("[Channel1]", "cue_default");
    }
    else {
        if (currentlyPlaying == 0){
            midi.sendShortMsg(0xB0,0x09,0x00);
            NumarkV7.MotorOffA();
        }
        
    }
}
NumarkV7.CueLEDA = function (){
    if ((engine.getValue("[Channel1]","play"))==0){
        midi.sendShortMsg(0xB0,0x08,0x01);
    }
    else{
        midi.sendShortMsg(0xB0,0x08,0x00);
    }
}
NumarkV7.SyncA = function (){
    if ((engine.getValue("[Channel1]","beatsync"))==1){
        midi.sendShortMsg(0xB0,0x07,0x01);
        engine.trigger("[Channel1]","rate");
    }
    else{
        midi.sendShortMsg(0xB0,0x07,0x00);
        engine.trigger("[Channel1]","rate");
    }
}
NumarkV7.PlayB = function (channel, control, value, status, group) {
    var currentlyPlaying = engine.getValue("[Channel2]","play");
    if ((currentlyPlaying == 0)&&(value == 0x7F)){
        engine.setValue("[Channel2]","play",1);
        midi.sendShortMsg(0xB0,0x1F,0x01);
        NumarkV7.MotorOnB();
    }
    else {
        if ((currentlyPlaying == 1)&&(value == 0x7F)){
            engine.setValue("[Channel2]","play",0);
            midi.sendShortMsg(0xB0,0x1F,0x00);
            NumarkV7.MotorOffB();
        }
        
    }
}
NumarkV7.Play2B = function (channel, control, value, status, group) {
    var currentlyPlaying = engine.getValue("[Channel2]","play");
    if (currentlyPlaying == 1){
        midi.sendShortMsg(0xB0,0x1F,0x01);
        NumarkV7.MotorOnB();
        engine.trigger("[Channel2]", "cue_default");
    }
    else {
        if (currentlyPlaying == 0){
            midi.sendShortMsg(0xB0,0x1F,0x00);
            NumarkV7.MotorOffB();
        }
        
    }
}
NumarkV7.CueLEDB = function (){
    if ((engine.getValue("[Channel2]","play"))==0){
        midi.sendShortMsg(0xB0,0x1E,0x01);
    }
    else{
        midi.sendShortMsg(0xB0,0x1E,0x00);
    }
}
NumarkV7.SyncB = function (){
    if ((engine.getValue("[Channel2]","beatsync"))==1){
        midi.sendShortMsg(0xB0,0x1D,0x01);
        engine.trigger("[Channel2]","rate");
    }
    else{
        midi.sendShortMsg(0xB0,0x1D,0x00);
        engine.trigger("[Channel2]","rate");
    }
}

//Motor On-Off
NumarkV7.MotorOnA = function (){
    if (!NumarkV7.MotorDisableA){
        midi.sendShortMsg(0xB0,0x43,0x00);//start motor
		if ((!NumarkV7.ScratchDisableA)&&(!engine.isScratching(1))) {
			engine.scratchEnable(1, 37056, NumarkV7.RPM, (1.0), (0.27), false);
		}
	}
    else {
        midi.sendShortMsg(0xB0,0x44,0x00);//stop motor
		engine.scratchDisable(1, false);
    }
}
NumarkV7.MotorOnB = function () {
    if (!NumarkV7.MotorDisableB){
        midi.sendShortMsg(0xB0,0x4D,0x00);//start motor
		if ((!NumarkV7.ScratchDisableB)&&(!engine.isScratching(2))) {
			engine.scratchEnable(2, 37056, NumarkV7.RPM, (1.0), (0.27), false);
		}
    
	}
    else {
        midi.sendShortMsg(0xB0,0x4E,0x00);//stop motor
		engine.scratchDisable(2, false);
    }
}
NumarkV7.MotorOffA = function () {
    midi.sendShortMsg(0xB0,0x44,0x00);//stop motor
	engine.scratchDisable(1, false);
}
NumarkV7.MotorOffB = function () {
    midi.sendShortMsg(0xB0,0x4E,0x00);//stop motor
	engine.scratchDisable(2, false);
}

//Hot Cues
NumarkV7.ShiftA = function (channel, control, value, status, group) {
	if (value == 0x7F){
		NumarkV7.ShiftHoldA = true;
		if (NumarkV7.ShiftToggleA == false){
			NumarkV7.ShiftToggleA = true;
			midi.sendShortMsg(0xB0,0x0A,0x01);
		}
		else{
			NumarkV7.ShiftToggleA = false;
		}
	}
	else {
		if (value == 0x00){
			NumarkV7.ShiftHoldA = false;
			if (NumarkV7.ShiftToggleA == false){
				midi.sendShortMsg(0xB0,0x0A,0x00);
			}
		}
	}
	//Hotcue Lights
	if (NumarkV7.ShiftToggleA == true){ //RED LIGHTS
		if (engine.getValue("[Channel1]","hotcue_1_enabled") == 1){
			midi.sendShortMsg(0xB0,0x0B,0x01);
		}
		else{
			midi.sendShortMsg(0xB0,0x0B,0x00);
		}
		if (engine.getValue("[Channel1]","hotcue_2_enabled") == 1){
			midi.sendShortMsg(0xB0,0x0C,0x01);
		}
		else{
			midi.sendShortMsg(0xB0,0x0C,0x00);
		}
		if (engine.getValue("[Channel1]","hotcue_3_enabled") == 1){
			midi.sendShortMsg(0xB0,0x0D,0x01);
		}
		else{
			midi.sendShortMsg(0xB0,0x0D,0x00);
		}
		if (engine.getValue("[Channel1]","hotcue_4_enabled") == 1){
			midi.sendShortMsg(0xB0,0x0E,0x01);
		}
		else{
			midi.sendShortMsg(0xB0,0x0E,0x00);
		}
		if (engine.getValue("[Channel1]","hotcue_5_enabled") == 1){
			midi.sendShortMsg(0xB0,0x0F,0x01);
		}
		else{
			midi.sendShortMsg(0xB0,0x0F,0x00);
		}
	}
	if (NumarkV7.ShiftToggleA == false){ //WHITE LIGHTS
		midi.sendShortMsg(0xB0,0x0A,0x00);
		if (engine.getValue("[Channel1]","hotcue_1_enabled") == 1){
			midi.sendShortMsg(0xB0,0x0B,0x7F);
		}
		else{
			midi.sendShortMsg(0xB0,0x0B,0x00);
		}
		if (engine.getValue("[Channel1]","hotcue_2_enabled") == 1){
			midi.sendShortMsg(0xB0,0x0C,0x7F);
		}
		else{
			midi.sendShortMsg(0xB0,0x0C,0x00);
		}
		if (engine.getValue("[Channel1]","hotcue_3_enabled") == 1){
			midi.sendShortMsg(0xB0,0x0D,0x7F);
		}
		else{
			midi.sendShortMsg(0xB0,0x0D,0x00);
		}
		if (engine.getValue("[Channel1]","hotcue_4_enabled") == 1){
			midi.sendShortMsg(0xB0,0x0E,0x7F);
		}
		else{
			midi.sendShortMsg(0xB0,0x0E,0x00);
		}
		if (engine.getValue("[Channel1]","hotcue_5_enabled") == 1){
			midi.sendShortMsg(0xB0,0x0F,0x7F);
		}
		else{
			midi.sendShortMsg(0xB0,0x0F,0x00);
		}
	}
}
NumarkV7.ShiftB = function (channel, control, value, status, group) {
	if (value == 0x7F){
		NumarkV7.ShiftHoldB = true;
		if (NumarkV7.ShiftToggleB == false){
			NumarkV7.ShiftToggleB = true;
			midi.sendShortMsg(0xB0,0x20,0x01);
		}
		else{
			NumarkV7.ShiftToggleB = false;
		}
	}
	else {
		if (value == 0x00){
			NumarkV7.ShiftHoldB = false;
			if (NumarkV7.ShiftToggleB == false){
				midi.sendShortMsg(0xB0,0x20,0x00);
			}
		}
	}
	//Hotcue Lights
	if (NumarkV7.ShiftToggleB == true){ //RED LIGHTS
		if (engine.getValue("[Channel2]","hotcue_1_enabled") == 1){
			midi.sendShortMsg(0xB0,0x21,0x01);
		}
		else{
			midi.sendShortMsg(0xB0,0x21,0x00);
		}
		if (engine.getValue("[Channel2]","hotcue_2_enabled") == 1){
			midi.sendShortMsg(0xB0,0x22,0x01);
		}
		else{
			midi.sendShortMsg(0xB0,0x22,0x00);
		}
		if (engine.getValue("[Channel2]","hotcue_3_enabled") == 1){
			midi.sendShortMsg(0xB0,0x23,0x01);
		}
		else{
			midi.sendShortMsg(0xB0,0x23,0x00);
		}
		if (engine.getValue("[Channel2]","hotcue_4_enabled") == 1){
			midi.sendShortMsg(0xB0,0x24,0x01);
		}
		else{
			midi.sendShortMsg(0xB0,0x24,0x00);
		}
		if (engine.getValue("[Channel2]","hotcue_5_enabled") == 1){
			midi.sendShortMsg(0xB0,0x25,0x01);
		}
		else{
			midi.sendShortMsg(0xB0,0x25,0x00);
		}
	}
	if (NumarkV7.ShiftToggleB == false){ //WHITE LIGHTS
		midi.sendShortMsg(0xB0,0x20,0x00);
		if (engine.getValue("[Channel2]","hotcue_1_enabled") == 1){
			midi.sendShortMsg(0xB0,0x21,0x7F);
		}
		else{
			midi.sendShortMsg(0xB0,0x21,0x00);
		}
		if (engine.getValue("[Channel2]","hotcue_2_enabled") == 1){
			midi.sendShortMsg(0xB0,0x22,0x7F);
		}
		else{
			midi.sendShortMsg(0xB0,0x22,0x00);
		}
		if (engine.getValue("[Channel2]","hotcue_3_enabled") == 1){
			midi.sendShortMsg(0xB0,0x23,0x7F);
		}
		else{
			midi.sendShortMsg(0xB0,0x23,0x00);
		}
		if (engine.getValue("[Channel2]","hotcue_4_enabled") == 1){
			midi.sendShortMsg(0xB0,0x24,0x7F);
		}
		else{
			midi.sendShortMsg(0xB0,0x24,0x00);
		}
		if (engine.getValue("[Channel2]","hotcue_5_enabled") == 1){
			midi.sendShortMsg(0xB0,0x25,0x7F);
		}
		else{
			midi.sendShortMsg(0xB0,0x25,0x00);
		}
	}
}
NumarkV7.Hot1A = function (channel, control, value, status, group) {
	if (value == 0x7F){
		if (NumarkV7.ShiftToggleA == false){
			engine.setValue("[Channel1]","hotcue_1_activate",1);
		}
		else{
			engine.setValue("[Channel1]","hotcue_1_clear",1);
			NumarkV7.ShiftA(0x00,0x00,0x7F);
		}
	}
	else{
		if (value == 0x00){
			if (NumarkV7.ShiftToggleA == false){
				engine.setValue("[Channel1]","hotcue_1_activate",0);
			}
			else{
				engine.setValue("[Channel1]","hotcue_1_clear",0);
			}
		}
	}
	NumarkV7.ShiftA();
}
NumarkV7.Hot1B = function (channel, control, value, status, group) {
	if (value == 0x7F){
		if (NumarkV7.ShiftToggleB == false){
			engine.setValue("[Channel2]","hotcue_1_activate",1);
		}
		else{
			engine.setValue("[Channel2]","hotcue_1_clear",1);
			NumarkV7.ShiftB(0x00,0x00,0x7F);
		}
	}
	else{
		if (value == 0x00){
			if (NumarkV7.ShiftToggleB == false){
				engine.setValue("[Channel2]","hotcue_1_activate",0);
			}
			else{
				engine.setValue("[Channel2]","hotcue_1_clear",0);
			}
		}
	}
	NumarkV7.ShiftB();
}
NumarkV7.Hot2A = function (channel, control, value, status, group) {
	if (value == 0x7F){
		if (NumarkV7.ShiftToggleA == false){
			engine.setValue("[Channel1]","hotcue_2_activate",1);
		}
		else{
			engine.setValue("[Channel1]","hotcue_2_clear",1);
			NumarkV7.ShiftA(0x00,0x00,0x7F);
		}
	}
	else{
		if (value == 0x00){
			if (NumarkV7.ShiftToggleA == false){
				engine.setValue("[Channel1]","hotcue_2_activate",0);
			}
			else{
				engine.setValue("[Channel1]","hotcue_2_clear",0);
			}
		}
	}
	NumarkV7.ShiftA();
}
NumarkV7.Hot2B = function (channel, control, value, status, group) {
	if (value == 0x7F){
		if (NumarkV7.ShiftToggleB == false){
			engine.setValue("[Channel2]","hotcue_2_activate",1);
		}
		else{
			engine.setValue("[Channel2]","hotcue_2_clear",1);
			NumarkV7.ShiftB(0x00,0x00,0x7F);
		}
	}
	else{
		if (value == 0x00){
			if (NumarkV7.ShiftToggleB == false){
				engine.setValue("[Channel2]","hotcue_2_activate",0);
			}
			else{
				engine.setValue("[Channel2]","hotcue_2_clear",0);
			}
		}
	}
	NumarkV7.ShiftB();
}
NumarkV7.Hot3A = function (channel, control, value, status, group) {
	if (value == 0x7F){
		if (NumarkV7.ShiftToggleA == false){
			engine.setValue("[Channel1]","hotcue_3_activate",1);
		}
		else{
			engine.setValue("[Channel1]","hotcue_3_clear",1);
			NumarkV7.ShiftA(0x00,0x00,0x7F);
		}
	}
	else{
		if (value == 0x00){
			if (NumarkV7.ShiftToggleA == false){
				engine.setValue("[Channel1]","hotcue_3_activate",0);
			}
			else{
				engine.setValue("[Channel1]","hotcue_3_clear",0);
			}
		}
	}
	NumarkV7.ShiftA();
}
NumarkV7.Hot3B = function (channel, control, value, status, group) {
	if (value == 0x7F){
		if (NumarkV7.ShiftToggleB == false){
			engine.setValue("[Channel2]","hotcue_3_activate",1);
		}
		else{
			engine.setValue("[Channel2]","hotcue_3_clear",1);
			NumarkV7.ShiftB(0x00,0x00,0x7F);
		}
	}
	else{
		if (value == 0x00){
			if (NumarkV7.ShiftToggleB == false){
				engine.setValue("[Channel2]","hotcue_3_activate",0);
			}
			else{
				engine.setValue("[Channel2]","hotcue_3_clear",0);
			}
		}
	}
	NumarkV7.ShiftB();
}
NumarkV7.Hot4A = function (channel, control, value, status, group) {
	if (value == 0x7F){
		if (NumarkV7.ShiftToggleA == false){
			engine.setValue("[Channel1]","hotcue_4_activate",1);
		}
		else{
			engine.setValue("[Channel1]","hotcue_4_clear",1);
			NumarkV7.ShiftA(0x00,0x00,0x7F);
		}
	}
	else{
		if (value == 0x00){
			if (NumarkV7.ShiftToggleA == false){
				engine.setValue("[Channel1]","hotcue_4_activate",0);
			}
			else{
				engine.setValue("[Channel1]","hotcue_4_clear",0);
			}
		}
	}
	NumarkV7.ShiftA();
}
NumarkV7.Hot4B = function (channel, control, value, status, group) {
	if (value == 0x7F){
		if (NumarkV7.ShiftToggleB == false){
			engine.setValue("[Channel2]","hotcue_4_activate",1);
		}
		else{
			engine.setValue("[Channel2]","hotcue_4_clear",1);
			NumarkV7.ShiftB(0x00,0x00,0x7F);
		}
	}
	else{
		if (value == 0x00){
			if (NumarkV7.ShiftToggleB == false){
				engine.setValue("[Channel2]","hotcue_4_activate",0);
			}
			else{
				engine.setValue("[Channel2]","hotcue_4_clear",0);
			}
		}
	}
	NumarkV7.ShiftB();
}
NumarkV7.Hot5A = function (channel, control, value, status, group) {
	if (value == 0x7F){
		if (NumarkV7.ShiftToggleA == false){
			engine.setValue("[Channel1]","hotcue_5_activate",1);
		}
		else{
			engine.setValue("[Channel1]","hotcue_5_clear",1);
			NumarkV7.ShiftA(0x00,0x00,0x7F);
		}
	}
	else{
		if (value == 0x00){
			if (NumarkV7.ShiftToggleA == false){
				engine.setValue("[Channel1]","hotcue_5_activate",0);
			}
			else{
				engine.setValue("[Channel1]","hotcue_5_clear",0);
			}
		}
	}
	NumarkV7.ShiftA();
}
NumarkV7.Hot5B = function (channel, control, value, status, group) {
	if (value == 0x7F){
		if (NumarkV7.ShiftToggleB == false){
			engine.setValue("[Channel2]","hotcue_5_activate",1);
		}
		else{
			engine.setValue("[Channel2]","hotcue_5_clear",1);
			NumarkV7.ShiftB(0x00,0x00,0x7F);
		}
	}
	else{
		if (value == 0x00){
			if (NumarkV7.ShiftToggleB == false){
				engine.setValue("[Channel2]","hotcue_5_activate",0);
			}
			else{
				engine.setValue("[Channel2]","hotcue_5_clear",0);
			}
		}
	}
	NumarkV7.ShiftB();
}

//Pitch Controls
NumarkV7.PitchACoarse = function (channel, control, value, status, group) {
	NumarkV7.PitchACoarseParse = script.absoluteLin(value, -1, 1);
}
NumarkV7.PitchAFine = function (channel, control, value, status, group) {
	NumarkV7.PitchAFineParse = ((script.absoluteLin(value, 0, 1))/127);
	NumarkV7.PitchA();
}
NumarkV7.PitchA = function () {
	var currentvalue = (NumarkV7.PitchACoarseParse+NumarkV7.PitchAFineParse);
	if ((currentvalue < 0.01)&&(currentvalue > -0.01)) {
		currentvalue = 0.00;
	}
	if (engine.isScratching(1)) {
		engine.scratchDisable(1, false);
		NumarkV7.PitchAPause = 2;
		if (!NumarkV7.PitchAPauseOn) {
			NumarkV7.timer1 = engine.beginTimer(100, "NumarkV7.PitchScratchEnableA");
		}
	}
    engine.setValue("[Channel1]","rate", (currentvalue * 1));
	NumarkV7.TempoLeanLED ();
}
NumarkV7.PitchAMotor = function () {
	var rateAbs = (engine.getValue("[Channel1]","rate"))*(engine.getValue("[Channel1]","rateRange"));
	if (engine.getValue("[Channel1]","rate") == 0){
		midi.sendShortMsg(0xB0,0x49,0x00);
		midi.sendShortMsg(0xB0,0x69,0x00);
		midi.sendShortMsg(0xB0,0x37,0x01);
	}
	if (engine.getValue("[Channel1]","rate") > 0){
		var ratePos = Math.floor(rateAbs * 63);
		var ratePos1 = Math.floor(((rateAbs * 63) - ratePos) * 127);
 		midi.sendShortMsg(0xB0,0x49,(0x00 + ratePos));
		midi.sendShortMsg(0xB0,0x69,(0x00 + ratePos1));
		midi.sendShortMsg(0xB0,0x37,0x00);
	}
	if (engine.getValue("[Channel1]","rate") < 0){
		var rateNeg0 = (1 - (rateAbs * (-1)));
		var rateNeg = ((Math.floor(rateNeg0 * 63)) + 65);
		var rateNeg1 = Math.floor((((rateNeg0 * 63) + 65) - rateNeg) * 127);
		midi.sendShortMsg(0xB0,0x49,(0x00 + rateNeg));
		midi.sendShortMsg(0xB0,0x69,(0x00 + rateNeg1));
		midi.sendShortMsg(0xB0,0x37,0x00);
	}
}
NumarkV7.PitchBCoarse = function (channel, control, value, status, group) {
	NumarkV7.PitchBCoarseParse = script.absoluteLin(value, -1, 1);
}
NumarkV7.PitchBFine = function (channel, control, value, status, group) {
	NumarkV7.PitchBFineParse = ((script.absoluteLin(value, 0, 1))/127);
	NumarkV7.PitchB();
}
NumarkV7.PitchB = function () {
	var currentvalue = (NumarkV7.PitchBCoarseParse+NumarkV7.PitchBFineParse);
	if ((currentvalue < 0.01)&&(currentvalue > -0.01)) {
		currentvalue = 0.00;
	}
	if (engine.isScratching(2)) {
		engine.scratchDisable(2, false);
		NumarkV7.PitchBPause = 2;
		if (!NumarkV7.PitchBPauseOn) {
			NumarkV7.timer2 = engine.beginTimer(100, "NumarkV7.PitchScratchEnableB");
		}
	}
    engine.setValue("[Channel2]","rate", (currentvalue * 1));
	NumarkV7.TempoLeanLED ();
}
NumarkV7.PitchBMotor = function () {
	var rateAbs = (engine.getValue("[Channel2]","rate"))*(engine.getValue("[Channel2]","rateRange"));
	if (engine.getValue("[Channel2]","rate") == 0){
		midi.sendShortMsg(0xB0,0x53,0x00);
		midi.sendShortMsg(0xB0,0x73,0x00);
		midi.sendShortMsg(0xB0,0x38,0x01);
	}
	if (engine.getValue("[Channel2]","rate") > 0){
		var ratePos = Math.floor(rateAbs * 63);
		var ratePos1 = Math.floor(((rateAbs * 63) - ratePos) * 127);
 		midi.sendShortMsg(0xB0,0x53,(0x00 + ratePos));
		midi.sendShortMsg(0xB0,0x73,(0x00 + ratePos1));
		midi.sendShortMsg(0xB0,0x38,0x00);
	}
	if (engine.getValue("[Channel2]","rate") < 0){
		var rateNeg0 = (1 - (rateAbs * (-1)));
		var rateNeg = ((Math.floor(rateNeg0 * 63)) + 65);
		var rateNeg1 = Math.floor((((rateNeg0 * 63) + 65) - rateNeg) * 127);
		midi.sendShortMsg(0xB0,0x53,(0x00 + rateNeg));
		midi.sendShortMsg(0xB0,0x73,(0x00 + rateNeg1));
		midi.sendShortMsg(0xB0,0x38,0x00);
	}
}
NumarkV7.RateRangeA = function (channel, control, value, status, group) {
    if (value == 0x7F) {
        var currentRange = Math.round(engine.getValue("[Channel1]","rateRange")*100)/100;
        print ("Current range="+currentRange);
        switch (true) {
            case (currentRange<NumarkV7.RateRanges[0]):
                engine.setValue("[Channel1]","rateRange",NumarkV7.RateRanges[0]);
                break;
            case (currentRange<NumarkV7.RateRanges[1]):
                engine.setValue("[Channel1]","rateRange",NumarkV7.RateRanges[1]);
                break;
            case (currentRange<NumarkV7.RateRanges[2]):
                engine.setValue("[Channel1]","rateRange",NumarkV7.RateRanges[2]);
                break;
            case (currentRange<NumarkV7.RateRanges[3]):
                engine.setValue("[Channel1]","rateRange",NumarkV7.RateRanges[3]);
                break;
            case (currentRange<NumarkV7.RateRanges[4]):
                engine.setValue("[Channel1]","rateRange",NumarkV7.RateRanges[4]);
                break;
            case (currentRange<NumarkV7.RateRanges[5]):
                engine.setValue("[Channel1]","rateRange",NumarkV7.RateRanges[5]);
                break;
            case (currentRange>=NumarkV7.RateRanges[5]):
                engine.setValue("[Channel1]","rateRange",NumarkV7.RateRanges[0]);
                break;
        }
        engine.trigger("[Channel1]","rate");
		NumarkV7.TempoLeanLED ();
    }
}
NumarkV7.RateRangeB = function (channel, control, value, status, group) {
    if (value == 0x7F) {
        var currentRange = Math.round(engine.getValue("[Channel2]","rateRange")*100)/100;
        print ("Current range="+currentRange);
        switch (true) {
            case (currentRange<NumarkV7.RateRanges[0]):
                engine.setValue("[Channel2]","rateRange",NumarkV7.RateRanges[0]);
                break;
            case (currentRange<NumarkV7.RateRanges[1]):
                engine.setValue("[Channel2]","rateRange",NumarkV7.RateRanges[1]);
                break;
            case (currentRange<NumarkV7.RateRanges[2]):
                engine.setValue("[Channel2]","rateRange",NumarkV7.RateRanges[2]);
                break;
            case (currentRange<NumarkV7.RateRanges[3]):
                engine.setValue("[Channel2]","rateRange",NumarkV7.RateRanges[3]);
                break;
            case (currentRange<NumarkV7.RateRanges[4]):
                engine.setValue("[Channel2]","rateRange",NumarkV7.RateRanges[4]);
                break;
            case (currentRange<NumarkV7.RateRanges[5]):
                engine.setValue("[Channel2]","rateRange",NumarkV7.RateRanges[5]);
                break;
            case (currentRange>=NumarkV7.RateRanges[5]):
                engine.setValue("[Channel2]","rateRange",NumarkV7.RateRanges[0]);
                break;
        }
        engine.trigger("[Channel2]","rate");
		NumarkV7.TempoLeanLED ();
    }
}
NumarkV7.KeylockA = function () {
    var keylock = engine.getValue("[Channel1]","keylock");
    if (keylock == 1){
        midi.sendShortMsg(0xB0,0x10,0x01);
    }
    else{
        midi.sendShortMsg(0xB0,0x10,0x00);
    }
}
NumarkV7.KeylockB = function () {
    var keylock = engine.getValue("[Channel2]","keylock");
    if (keylock == 1){
        midi.sendShortMsg(0xB0,0x27,0x01);
    }
    else{
        midi.sendShortMsg(0xB0,0x27,0x00);
    }
}
NumarkV7.TempoLeanLED = function () {
	var BeatMatch = (engine.getValue("[Channel2]","bpm")-engine.getValue("[Channel1]","bpm"));
	if ((BeatMatch < 1)&&(BeatMatch > (-1))) {
		midi.sendShortMsg(0xB0,0x36,0x06);
	}
	else {
		if (BeatMatch >= 1) {
			if (BeatMatch < 2) {
				midi.sendShortMsg(0xB0,0x36,0x07);
			}
			else {
				if (BeatMatch < 4) {
					midi.sendShortMsg(0xB0,0x36,0x08);
				}
				else {
					if (BeatMatch < 6) {
						midi.sendShortMsg(0xB0,0x36,0x09);
					}
					else {
						if (BeatMatch < 8) {
							midi.sendShortMsg(0xB0,0x36,0x0A);
						}
						else {
							midi.sendShortMsg(0xB0,0x36,0x0B);
						}
					}
				}
			}
		}
		if (BeatMatch <= (-1)) {
			if (BeatMatch > (-2)) {
				midi.sendShortMsg(0xB0,0x36,0x05);
			}
			else {
				if (BeatMatch > (-4)) {
					midi.sendShortMsg(0xB0,0x36,0x04);
				}
				else {
					if (BeatMatch > (-6)) {
						midi.sendShortMsg(0xB0,0x36,0x03);
					}
					else {
						if (BeatMatch > (-8)) {
							midi.sendShortMsg(0xB0,0x36,0x02);
						}
						else {
							midi.sendShortMsg(0xB0,0x36,0x01);
						}
					}
				}
			}
		}
	}
}
NumarkV7.PitchScratchEnableA = function () {
	if (!NumarkV7.MotorDisableA) {
		NumarkV7.PitchAPause -=1;
		if (NumarkV7.PitchAPause == 0) {
			engine.stopTimer(NumarkV7.timer1);
			engine.scratchEnable(1, 37056, NumarkV7.RPM, (1.0), (0.27), false);
			NumarkV7.PitchAPauseOn = false;
		}
	}
}
NumarkV7.PitchScratchEnableB = function () {
	if (!NumarkV7.MotorDisableB) {
		NumarkV7.PitchBPause -=1;
		if (NumarkV7.PitchBPause == 0) {
			engine.stopTimer(NumarkV7.timer2);
			engine.scratchEnable(2, 37056, NumarkV7.RPM, (1.0), (0.27), false);
			NumarkV7.PitchBPauseOn = false;
		}
	}
}

//Strip Search, Needle Drop, Play Position, etc.
NumarkV7.StripSearchA = function (channel, control, value, status, group) {
	if (value != 0x00) {
		engine.setValue("[Channel1]", "playposition", script.absoluteLin(value, 0, 1, 0x02, 0x7F));
	}
}
NumarkV7.StripSearchB = function (channel, control, value, status, group) {
	if (value != 0x00) {
		engine.setValue("[Channel2]", "playposition", script.absoluteLin(value, 0, 1, 0x02, 0x7F));
	}
}

//Loop Controls
NumarkV7.LoopModeA = function (channel, control, value, status, group) {
    if ((NumarkV7.loop_modeA == false)&&(value == 0x7F)){
        NumarkV7.loop_modeA = true;
        midi.sendShortMsg(0xB0,0x18,0x01);
    }
	else {
		if ((NumarkV7.loop_modeA == true)&&(value == 0x7F)){
			NumarkV7.loop_modeA = false;
			midi.sendShortMsg(0xB0,0x18,0x7F);
        }
	}
	if (NumarkV7.loop_modeA == false){
        if (engine.getValue("[Channel1]","loop_start_position") == (-1)) {
			midi.sendShortMsg(0xB0,0x19,0x00);
		}
		else {
			midi.sendShortMsg(0xB0,0x19,0x7F);
		}
		if (engine.getValue("[Channel1]","loop_end_position") == (-1)) {
			midi.sendShortMsg(0xB0,0x1A,0x00);
		}
		else {
			midi.sendShortMsg(0xB0,0x1A,0x7F);
		}
    }
	else {
        if (engine.getValue("[Channel1]","beatloop_1_enabled") == 0) {
			midi.sendShortMsg(0xB0,0x19,0x00);
		}
		else {
			midi.sendShortMsg(0xB0,0x19,0x01);
		}
		if (engine.getValue("[Channel1]","beatloop_2_enabled") == 0) {
			midi.sendShortMsg(0xB0,0x1A,0x00);
		}
		else {
			midi.sendShortMsg(0xB0,0x1A,0x01);
		}
		if (engine.getValue("[Channel1]","beatloop_4_enabled") == 0) {
			midi.sendShortMsg(0xB0,0x1B,0x00);
		}
		else {
			midi.sendShortMsg(0xB0,0x1B,0x01);
		}
		if (engine.getValue("[Channel1]","beatloop_8_enabled") == 0) {
			midi.sendShortMsg(0xB0,0x1C,0x00);
		}
		else {
			midi.sendShortMsg(0xB0,0x1C,0x01);
		}
    }
}
NumarkV7.LoopModeB = function (channel, control, value, status, group) {
    if ((NumarkV7.loop_modeB == false)&&(value == 0x7F)){
        NumarkV7.loop_modeB = true;
        midi.sendShortMsg(0xB0,0x2F,0x01);
    }
	else {
		if ((NumarkV7.loop_modeB == true)&&(value == 0x7F)){
			NumarkV7.loop_modeB = false;
			midi.sendShortMsg(0xB0,0x2F,0x7F);
        }
	}
	if (NumarkV7.loop_modeB == false){
        if (engine.getValue("[Channel2]","loop_start_position") == (-1)) {
			midi.sendShortMsg(0xB0,0x30,0x00);
		}
		else {
			midi.sendShortMsg(0xB0,0x30,0x7F);
		}
		if (engine.getValue("[Channel2]","loop_end_position") == (-1)) {
			midi.sendShortMsg(0xB0,0x31,0x00);
		}
		else {
			midi.sendShortMsg(0xB0,0x31,0x7F);
		}
    }
	else {
        if (engine.getValue("[Channel2]","beatloop_1_enabled") == 0) {
			midi.sendShortMsg(0xB0,0x30,0x00);
		}
		else {
			midi.sendShortMsg(0xB0,0x30,0x01);
		}
		if (engine.getValue("[Channel2]","beatloop_2_enabled") == 0) {
			midi.sendShortMsg(0xB0,0x31,0x00);
		}
		else {
			midi.sendShortMsg(0xB0,0x31,0x01);
		}
		if (engine.getValue("[Channel2]","beatloop_4_enabled") == 0) {
			midi.sendShortMsg(0xB0,0x32,0x00);
		}
		else {
			midi.sendShortMsg(0xB0,0x32,0x01);
		}
		if (engine.getValue("[Channel2]","beatloop_8_enabled") == 0) {
			midi.sendShortMsg(0xB0,0x33,0x00);
		}
		else {
			midi.sendShortMsg(0xB0,0x33,0x01);
		}
    }
}
NumarkV7.loopOnOffA = function () {
    if ((engine.getValue("[Channel1]","loop_enabled"))==1){
        midi.sendShortMsg(0xB0,0x15,0x01);
    }
    else{
        midi.sendShortMsg(0xB0,0x15,0x00);
    }
	NumarkV7.LoopModeA();
}
NumarkV7.loopOnOffB = function () {
    if ((engine.getValue("[Channel2]","loop_enabled"))==1){
        midi.sendShortMsg(0xB0,0x2C,0x01);
    }
    else{
        midi.sendShortMsg(0xB0,0x2C,0x00);
    }
	NumarkV7.LoopModeB();
}
NumarkV7.loop_inA = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		if (NumarkV7.loop_modeA == false) {
			engine.setValue("[Channel1]", "loop_in", 1);
		}
		else {
			engine.setValue("[Channel1]", "beatloop_1_activate", 1);
		}
	}
	NumarkV7.LoopModeA ();
}
NumarkV7.loop_outA = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		if (NumarkV7.loop_modeA == false) {
			engine.setValue("[Channel1]", "loop_out", 1);
		}
		else {
			engine.setValue("[Channel1]", "beatloop_2_activate", 1);
		}
	}
	NumarkV7.LoopModeA ();
}
NumarkV7.SelectA = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		if (NumarkV7.loop_modeA == true) {
			engine.setValue("[Channel1]", "beatloop_4_activate", 1);
		}
	}
	NumarkV7.LoopModeA ();
}
NumarkV7.ReloopA = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		if (NumarkV7.loop_modeA == true) {
			engine.setValue("[Channel1]", "beatloop_8_activate", 1);
		}
	}
	NumarkV7.LoopModeA ();
}
NumarkV7.loop_inB = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		if (NumarkV7.loop_modeB == false) {
			engine.setValue("[Channel2]", "loop_in", 1);
		}
		else {
			engine.setValue("[Channel2]", "beatloop_1_activate", 1);
		}
	}
	NumarkV7.LoopModeB ();
}
NumarkV7.loop_outB = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		if (NumarkV7.loop_modeB == false) {
			engine.setValue("[Channel2]", "loop_out", 1);
		}
		else {
			engine.setValue("[Channel2]", "beatloop_2_activate", 1);
		}
	}
	NumarkV7.LoopModeB ();
}
NumarkV7.SelectB = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		if (NumarkV7.loop_modeB == true) {
			engine.setValue("[Channel2]", "beatloop_4_activate", 1);
		}
	}
	NumarkV7.LoopModeB ();
}
NumarkV7.ReloopB = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		if (NumarkV7.loop_modeB == true) {
			engine.setValue("[Channel2]", "beatloop_8_activate", 1);
		}
	}
	NumarkV7.LoopModeB ();
}
NumarkV7.loop_halveA = function () {
    if ((engine.getValue("[Channel1]","loop_halve"))==1){
        midi.sendShortMsg(0xB0,0x13,0x01);
    }
    else{
        midi.sendShortMsg(0xB0,0x13,0x00);
    }
	NumarkV7.LoopModeA();
}
NumarkV7.loop_doubleA = function () {
    if ((engine.getValue("[Channel1]","loop_double"))==1){
        midi.sendShortMsg(0xB0,0x14,0x01);
    }
    else{
        midi.sendShortMsg(0xB0,0x14,0x00);
    }
	NumarkV7.LoopModeA();
}
NumarkV7.loop_halveB = function () {
    if ((engine.getValue("[Channel2]","loop_halve"))==1){
        midi.sendShortMsg(0xB0,0x2A,0x01);
    }
    else{
        midi.sendShortMsg(0xB0,0x2A,0x00);
    }
	NumarkV7.LoopModeB();
}
NumarkV7.loop_doubleB = function () {
    if ((engine.getValue("[Channel2]","loop_double"))==1){
        midi.sendShortMsg(0xB0,0x2B,0x01);
    }
    else{
        midi.sendShortMsg(0xB0,0x2B,0x00);
    }
	NumarkV7.LoopModeB();
}
NumarkV7.LoopShiftUpA = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		var loopStart = engine.getValue("[Channel1]", "loop_start_position");
		var loopEnd = engine.getValue("[Channel1]", "loop_end_position");
		var loopLength = (loopEnd - loopStart);
		engine.setValue("[Channel1]", "loop_end_position", (loopEnd + loopLength));
		engine.setValue("[Channel1]", "loop_start_position", loopEnd);
		midi.sendShortMsg(0xB0,0x17,0x01);
	}
	if (value == 0x00) {
		midi.sendShortMsg(0xB0,0x17,0x00);
	}
}
NumarkV7.LoopShiftDownA = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		var loopStart = engine.getValue("[Channel1]", "loop_start_position");
		var loopEnd = engine.getValue("[Channel1]", "loop_end_position");
		var loopLength = (loopEnd - loopStart);
		engine.setValue("[Channel1]", "loop_start_position", (loopStart - loopLength));
		engine.setValue("[Channel1]", "loop_end_position", loopStart);
		midi.sendShortMsg(0xB0,0x16,0x01);
		
	}
	if (value == 0x00) {
		midi.sendShortMsg(0xB0,0x16,0x00);
	}
}
NumarkV7.LoopShiftUpB = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		var loopStart = engine.getValue("[Channel2]", "loop_start_position");
		var loopEnd = engine.getValue("[Channel2]", "loop_end_position");
		var loopLength = (loopEnd - loopStart);
		engine.setValue("[Channel2]", "loop_end_position", (loopEnd + loopLength));
		engine.setValue("[Channel2]", "loop_start_position", loopEnd);
		midi.sendShortMsg(0xB0,0x2E,0x01);
	}
	if (value == 0x00) {
		midi.sendShortMsg(0xB0,0x2E,0x00);
	}
}
NumarkV7.LoopShiftDownB = function (channel, control, value, status, group) {
	if (value == 0x7F) {
		var loopStart = engine.getValue("[Channel2]", "loop_start_position");
		var loopEnd = engine.getValue("[Channel2]", "loop_end_position");
		var loopLength = (loopEnd - loopStart);
		engine.setValue("[Channel2]", "loop_start_position", (loopStart - loopLength));
		engine.setValue("[Channel2]", "loop_end_position", loopStart);
		midi.sendShortMsg(0xB0,0x2D,0x01);
	}
	if (value == 0x00) {
		midi.sendShortMsg(0xB0,0x2D,0x00);
	}
}

//Selector switch "Master section"
NumarkV7.DeckSelectL = function (channel, control, value, status, group) {
	if (value == 0x00) {
		engine.scratchDisable(2, false);
		if (!NumarkV7.ScratchDisableA){
			engine.scratchEnable(1, 37056, NumarkV7.RPM, (1.0), (0.27), false);
		}
	}
	if (value == 0x7F) {
		engine.scratchDisable(1, false);
		if (!NumarkV7.ScratchDisableB){
			engine.scratchEnable(2, 37056, NumarkV7.RPM, (1.0), (0.27), false);
		}
	}
}
NumarkV7.DeckSelectR = function (channel, control, value, status, group) {
	if (value == 0x41) {
		engine.scratchDisable(2, false);
	}
	if ((value == 0x01)&&(!NumarkV7.ScratchDisableB)) {
		engine.scratchEnable(2, 37056, NumarkV7.RPM, (1.0), (0.27), false);
	}
}
NumarkV7.MasterL = function (channel, control, value, status, group) {
	if (value == 00) {
		NumarkV7.init;
	}
}
NumarkV7.MasterR = function (channel, control, value, status, group) {
	if ((value == 00)&&(!NumarkV7.ScratchDisableB)) {
		engine.scratchEnable(2, 37056, NumarkV7.RPM, (1.0), (0.27), false);
	}
	if (value == 0x7F){
		engine.scratchDisable(2, false);
	}
}

//Vinyl
NumarkV7.ScratchA = function (channel, control, value, status, group) {
	var diff = value - NumarkV7.ScratchDiffA;
	var ticks = 0;
	switch (true) {
		case (diff == (-0x7F)):
			ticks = 1;
			break;
		case (diff == 0x01):
			ticks = 1;
			break;
		case (diff == 0x02):
			ticks = 2;
			break;
		case (diff == 0x03):
			ticks = 3;
			break;
		case (diff == 0x04):
			ticks = 4;
			break;
		case (diff == 0x05):
			ticks = 5;
			break;
		case (diff == 0x06):
			ticks = 6;
			break;
		case (diff == 0x07):
			ticks = 7;
			break;
		case (diff == 0x08):
			ticks = 8;
			break;
		case (diff == 0x09):
			ticks = 9;
			break;
		case (diff == 0x0A):
			ticks = 10;
			break;
		case (diff == 0x0B):
			ticks = (11);
			break;
		case (diff == 0x0C):
			ticks = (12);
			break;
		case (diff == 0x0D):
			ticks = (13);
			break;
		case (diff == 0x0E):
			ticks = (14);
			break;
		case (diff == 0x0F):
			ticks = (15);
			break;
		case (diff == 0x10):
			ticks = (16);
			break;
		case (diff == 0x11):
			ticks = (17);
			break;
		case (diff == 0x12):
			ticks = (18);
			break;
		case (diff == 0x13):
			ticks = (19);
			break;
		case (diff == 0x14):
			ticks = (20);
			break;
		case (diff == 0x15):
			ticks = (21);
			break;
		case (diff == 0x16):
			ticks = (22);
			break;
		case (diff == 0x17):
			ticks = (23);
			break;
		case (diff == 0x18):
			ticks = (24);
			break;
		case (diff == 0x19):
			ticks = (25);
			break;
		case (diff == 0x1A):
			ticks = (26);
			break;
		case (diff == 0x1B):
			ticks = (27);
			break;
		case (diff == 0x1C):
			ticks = (28);
			break;
		case (diff == 0x1D):
			ticks = (29);
			break;
		case (diff == 0x1E):
			ticks = (30);
			break;
		case (diff == 0x1F):
			ticks = (31);
			break;
		case (diff == 0x20):
			ticks = (32);
			break;
		case (diff == 0x21):
			ticks = (33);
			break;
		case (diff == 0x22):
			ticks = (34);
			break;
		case (diff == 0x23):
			ticks = (35);
			break;
		case (diff == 0x24):
			ticks = (36);
			break;
		case (diff == 0x25):
			ticks = (37);
			break;
		case (diff == 0x26):
			ticks = (38);
			break;
		case (diff == 0x27):
			ticks = (39);
			break;
		case (diff == 0x28):
			ticks = (40);
			break;
		case (diff ==0x7F):
			ticks = (-1);
			break;
		case (diff == (-0x01)):
			ticks = (-1);
			break;
		case (diff == (-0x02)):
			ticks = (-2);
			break;
		case (diff == (-0x03)):
			ticks = (-3);
			break;
		case (diff == (-0x04)):
			ticks = (-4);
			break;
		case (diff == (-0x05)):
			ticks = (-5);
			break;
		case (diff == (-0x06)):
			ticks = (-6);
			break;
		case (diff == (-0x07)):
			ticks = (-7);
			break;
		case (diff == (-0x08)):
			ticks = (-8);
			break;
		case (diff == (-0x09)):
			ticks = (-9);
			break;
		case (diff == (-0x0A)):
			ticks = (-10);
			break;
		case (diff == (-0x0B)):
			ticks = (-11);
			break;
		case (diff == (-0x0C)):
			ticks = (-12);
			break;
		case (diff == (-0x0D)):
			ticks = (-13);
			break;
		case (diff == (-0x0E)):
			ticks = (-14);
			break;
		case (diff == (-0x0F)):
			ticks = (-15);
			break;
		case (diff == (-0x10)):
			ticks = (-16);
			break;
		case (diff == (-0x11)):
			ticks = (-17);
			break;
		case (diff == (-0x12)):
			ticks = (-18);
			break;
		case (diff == (-0x13)):
			ticks = (-19);
			break;
		case (diff == (-0x14)):
			ticks = (-20);
			break;
		case (diff == (-0x15)):
			ticks = (-21);
			break;
		case (diff == (-0x16)):
			ticks = (-22);
			break;
		case (diff == (-0x17)):
			ticks = (-23);
			break;
		case (diff == (-0x18)):
			ticks = (-24);
			break;
		case (diff == (-0x19)):
			ticks = (-25);
			break;
		case (diff == (-0x1A)):
			ticks = (-26);
			break;
		case (diff == (-0x1B)):
			ticks = (-27);
			break;
		case (diff == (-0x1C)):
			ticks = (-28);
			break;
		case (diff == (-0x1D)):
			ticks = (-29);
			break;
		case (diff == (-0x1E)):
			ticks = (-30);
			break;
		case (diff == (-0x1F)):
			ticks = (-31);
			break;
		case (diff == (-0x20)):
			ticks = (-32);
			break;
		case (diff == (-0x21)):
			ticks = (-33);
			break;
		case (diff == (-0x22)):
			ticks = (-34);
			break;
		case (diff == (-0x23)):
			ticks = (-35);
			break;
		case (diff == (-0x24)):
			ticks = (-36);
			break;
		case (diff == (-0x25)):
			ticks = (-37);
			break;
		case (diff == (-0x26)):
			ticks = (-38);
			break;
		case (diff == (-0x27)):
			ticks = (-39);
			break;
		case (diff == (-0x28)):
			ticks = (-40);
			break;
	}
	if (engine.isScratching(1)) {
		engine.scratchTick(1, ticks);
	}
	else {
		if (NumarkV7.MotorDisableA) {
			engine.setValue("[Channel1]","jog",ticks);
		}
	}
	NumarkV7.ScratchDiffA = value;
}
NumarkV7.ScratchB = function (channel, control, value, status, group) {
	var diff = value - NumarkV7.ScratchDiffB;
    var ticks = 0;
	switch (true) {
		case (diff == (-0x7F)):
			ticks = 1;
			break;
		case (diff == 0x01):
			ticks = 1;
			break;
		case (diff == 0x02):
			ticks = 2;
			break;
		case (diff == 0x03):
			ticks = 3;
			break;
		case (diff == 0x04):
			ticks = 4;
			break;
		case (diff == 0x05):
			ticks = 5;
			break;
		case (diff == 0x06):
			ticks = 6;
			break;
		case (diff == 0x07):
			ticks = 7;
			break;
		case (diff == 0x08):
			ticks = 8;
			break;
		case (diff == 0x09):
			ticks = 9;
			break;
		case (diff == 0x0A):
			ticks = 10;
			break;
		case (diff == 0x0B):
			ticks = (11);
			break;
		case (diff == 0x0C):
			ticks = (12);
			break;
		case (diff == 0x0D):
			ticks = (13);
			break;
		case (diff == 0x0E):
			ticks = (14);
			break;
		case (diff == 0x0F):
			ticks = (15);
			break;
		case (diff == 0x10):
			ticks = (16);
			break;
		case (diff == 0x11):
			ticks = (17);
			break;
		case (diff == 0x12):
			ticks = (18);
			break;
		case (diff == 0x13):
			ticks = (19);
			break;
		case (diff == 0x14):
			ticks = (20);
			break;
		case (diff == 0x15):
			ticks = (21);
			break;
		case (diff == 0x16):
			ticks = (22);
			break;
		case (diff == 0x17):
			ticks = (23);
			break;
		case (diff == 0x18):
			ticks = (24);
			break;
		case (diff == 0x19):
			ticks = (25);
			break;
		case (diff == 0x1A):
			ticks = (26);
			break;
		case (diff == 0x1B):
			ticks = (27);
			break;
		case (diff == 0x1C):
			ticks = (28);
			break;
		case (diff == 0x1D):
			ticks = (29);
			break;
		case (diff == 0x1E):
			ticks = (30);
			break;
		case (diff == 0x1F):
			ticks = (31);
			break;
		case (diff == 0x20):
			ticks = (32);
			break;
		case (diff == 0x21):
			ticks = (33);
			break;
		case (diff == 0x22):
			ticks = (34);
			break;
		case (diff == 0x23):
			ticks = (35);
			break;
		case (diff == 0x24):
			ticks = (36);
			break;
		case (diff == 0x25):
			ticks = (37);
			break;
		case (diff == 0x26):
			ticks = (38);
			break;
		case (diff == 0x27):
			ticks = (39);
			break;
		case (diff == 0x28):
			ticks = (40);
			break;
		case (diff ==0x7F):
			ticks = (-1);
			break;
		case (diff == (-0x01)):
			ticks = (-1);
			break;
		case (diff == (-0x02)):
			ticks = (-2);
			break;
		case (diff == (-0x03)):
			ticks = (-3);
			break;
		case (diff == (-0x04)):
			ticks = (-4);
			break;
		case (diff == (-0x05)):
			ticks = (-5);
			break;
		case (diff == (-0x06)):
			ticks = (-6);
			break;
		case (diff == (-0x07)):
			ticks = (-7);
			break;
		case (diff == (-0x08)):
			ticks = (-8);
			break;
		case (diff == (-0x09)):
			ticks = (-9);
			break;
		case (diff == (-0x0A)):
			ticks = (-10);
			break;
		case (diff == (-0x0B)):
			ticks = (-11);
			break;
		case (diff == (-0x0C)):
			ticks = (-12);
			break;
		case (diff == (-0x0D)):
			ticks = (-13);
			break;
		case (diff == (-0x0E)):
			ticks = (-14);
			break;
		case (diff == (-0x0F)):
			ticks = (-15);
			break;
		case (diff == (-0x10)):
			ticks = (-16);
			break;
		case (diff == (-0x11)):
			ticks = (-17);
			break;
		case (diff == (-0x12)):
			ticks = (-18);
			break;
		case (diff == (-0x13)):
			ticks = (-19);
			break;
		case (diff == (-0x14)):
			ticks = (-20);
			break;
		case (diff == (-0x15)):
			ticks = (-21);
			break;
		case (diff == (-0x16)):
			ticks = (-22);
			break;
		case (diff == (-0x17)):
			ticks = (-23);
			break;
		case (diff == (-0x18)):
			ticks = (-24);
			break;
		case (diff == (-0x19)):
			ticks = (-25);
			break;
		case (diff == (-0x1A)):
			ticks = (-26);
			break;
		case (diff == (-0x1B)):
			ticks = (-27);
			break;
		case (diff == (-0x1C)):
			ticks = (-28);
			break;
		case (diff == (-0x1D)):
			ticks = (-29);
			break;
		case (diff == (-0x1E)):
			ticks = (-30);
			break;
		case (diff == (-0x1F)):
			ticks = (-31);
			break;
		case (diff == (-0x20)):
			ticks = (-32);
			break;
		case (diff == (-0x21)):
			ticks = (-33);
			break;
		case (diff == (-0x22)):
			ticks = (-34);
			break;
		case (diff == (-0x23)):
			ticks = (-35);
			break;
		case (diff == (-0x24)):
			ticks = (-36);
			break;
		case (diff == (-0x25)):
			ticks = (-37);
			break;
		case (diff == (-0x26)):
			ticks = (-38);
			break;
		case (diff == (-0x27)):
			ticks = (-39);
			break;
		case (diff == (-0x28)):
			ticks = (-40);
			break;
	}
	if (engine.isScratching(2)) {
		engine.scratchTick(2, ticks);
	}
	else {
		if (NumarkV7.MotorDisableB) {
			engine.setValue("[Channel2]","jog",ticks);
		}
	}
	NumarkV7.ScratchDiffB = value;
}

///////////////////////////
//                       //
//        The END        //
//          :P           //
//                       //
///////////////////////////

