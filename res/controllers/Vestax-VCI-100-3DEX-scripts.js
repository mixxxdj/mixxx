VestaxVCI1003DEX = new function() {}

var active_deck;	//"[ChannelN]"
var active_deck_number;  //N
var currentCue1;
var currentCue2;


// timer for loop leds start

VestaxVCI1003DEX.loopLeds = function () {

var looping = engine.getValue(active_deck,"loop_enabled");
var startposition = engine.getValue(active_deck,"loop_start_position");
var endposition = engine.getValue(active_deck,"loop_end_position");


if (startposition != -1){
midi.sendShortMsg(0x90,0x62,0x7F);/*loop in button led on*/
}
else{midi.sendShortMsg(0x80,0x62,0x7F);/*loop in button led off*/}

if (endposition != -1 && (startposition<endposition) ){
midi.sendShortMsg(0x90,0x63,0x7F);/*loop out button led on*/
}
else{midi.sendShortMsg(0x80,0x63,0x7F);/*loop out button led off*/}

if(looping){
midi.sendShortMsg(0x90,0x62,0x7F);// loop in button led on
midi.sendShortMsg(0x90,0x63,0x7F);// loop out button led on
midi.sendShortMsg(0x90,0x64,0x7F);// reloop/exit  button led on
}
else{midi.sendShortMsg(0x80,0x64,0x7F);/* reloop/exit  button led off*/}

if(looping==0 && startposition != -1 && endposition != -1)
{
midi.sendShortMsg(0x80,0x64,0x7F);// reloop/exit  button led off
}

}

engine.beginTimer(150, "VestaxVCI1003DEX.loopLeds()",false)
//timer for loop leds end


//sync led trigger start
VestaxVCI1003DEX.leftSyncLed = function (channel, control, value, status, group) {
var activebeat=engine.getValue("[Channel1]","beat_active");
if (activebeat)
{midi.sendShortMsg(0x90,0x46,0x7F);}// left sync button led on
else {midi.sendShortMsg(0x80,0x46,0x7F);}// left sync button led of
}

VestaxVCI1003DEX.rightSyncLed = function (channel, control, value, status, group) {
var activebeat=engine.getValue("[Channel2]","beat_active");
if (activebeat)
{midi.sendShortMsg(0x90,0x47,0x7F);}// left sync button led on
else {midi.sendShortMsg(0x80,0x47,0x7F);}// left sync button led of
}

engine.connectControl("[Channel1]","beat_active","VestaxVCI1003DEX.leftSyncLed");
engine.connectControl("[Channel2]","beat_active","VestaxVCI1003DEX.rightSyncLed");

//sync led end


//Mapping functions


VestaxVCI1003DEX.selectDeck1 = function (channel, control, value, status, group) {
active_deck="[Channel1]";
active_deck_number=1;
midi.sendShortMsg(0x90,0x67,0x7F);// deck- button led on
midi.sendShortMsg(0x80,0x68,0x7F);// deck+ button led off
}

VestaxVCI1003DEX.selectDeck2 = function (channel, control, value, status, group) {
active_deck="[Channel2]";
active_deck_number=2;
midi.sendShortMsg(0x80,0x67,0x7F);// deck- button led off
midi.sendShortMsg(0x90,0x68,0x7F);// deck+ button led on
}


// loop otions start
VestaxVCI1003DEX.loopin = function (channel, control, value, status, group) {if(value==0x7F)   {engine.setValue(active_deck,"loop_in",1);}}

VestaxVCI1003DEX.loopout = function (channel, control, value, status, group) {if(value==0x7F)   {engine.setValue(active_deck,"loop_out",1);}}

VestaxVCI1003DEX.reloop_exit = function (channel, control, value, status, group) {if(value==0x7F)   {engine.setValue(active_deck,"reloop_exit",1);}}

VestaxVCI1003DEX.loopMinus = function (channel, control, value, status, group) {if (value == 0x7F){engine.setValue(active_deck,"loop_halve",1);}}
VestaxVCI1003DEX.loopPlus = function (channel, control, value, status, group) {if (value == 0x7F){engine.setValue(active_deck,"loop_double",1);}}

// loop otions end

// deck specific buttons start

VestaxVCI1003DEX.jog_touch1 = function (channel, control, value, status, group) {
if(value=0x7F){engine.scratchEnable("[Channel1]", 128*3, 45, 1.0/8, (1.0/8)/32);} 
if(value==0)  {engine.scratchDisable("[Channel1]");}
}

VestaxVCI1003DEX.jog_touch2 = function (channel, control, value, status, group) {
if(value=0x7F){engine.scratchEnable("[Channel1]", 128*3, 45, 1.0/8, (1.0/8)/32);} 
if(value==0)  {engine.scratchDisable("[Channel1]");}
}

VestaxVCI1003DEX.jog_wheel1 = function (channel, control, value, status, group) {
   // 41 > 7F: CW Slow > Fast ??? 
   // 3F > 0 : CCW Slow > Fast ???
   var jogValue = value - 0x40; // -64 to +63, - = CCW, + = CW
   engine.setValue("[Channel1]","jog", jogValue/16);
}

VestaxVCI1003DEX.jog_wheel2 = function (channel, control, value, status, group) {
   // 41 > 7F: CW Slow > Fast ??? 
   // 3F > 0 : CCW Slow > Fast ???
   var jogValue = value - 0x40; // -64 to +63, - = CCW, + = CW
   engine.setValue("[Channel2]","jog", jogValue/16);
}

VestaxVCI1003DEX.jog_wheel_seek1 = function (channel, control, value, status, group) {
   // 41 > 7F: CW Slow > Fast ??? 
   // 3F > 0 : CCW Slow > Fast ???
   var jogValue = value - 0x40; // -64 to +63, - = CCW, + = CW
   engine.setValue("[Channel1]","jog", jogValue);
}

VestaxVCI1003DEX.jog_wheel_seek2 = function (channel, control, value, status, group) {
   // 41 > 7F: CW Slow > Fast ??? 
   // 3F > 0 : CCW Slow > Fast ???
   var jogValue = value - 0x40; // -64 to +63, - = CCW, + = CW
   engine.setValue("[Channel2]","jog", jogValue);
}

VestaxVCI1003DEX.play1 = function (channel, control, value, status, group) {

    var currentlyPlaying = engine.getValue("[Channel1]","play");
    if (currentlyPlaying == 1 && value == 0x7F) {    // If currently playing
        engine.setValue("[Channel1]","play",0);    // Stop
        midi.sendShortMsg(0x80,0x32,0x7F);    // Turn off the Play LED
    }
    if (currentlyPlaying == 0 && value == 0x7F) {
        engine.setValue("[Channel1]","play",1);    // Start
        midi.sendShortMsg(0x90,0x32,0x7F);    // Turn on the Play LED
    }

    if (currentlyPlaying == 0 && value == 0x00) {    // If no track loaded
        engine.setValue("[Channel1]","play",0);    // Stop
        midi.sendShortMsg(0x80,0x32,0x7F);    // Turn off the Play LED
    }

}

VestaxVCI1003DEX.play2 = function (channel, control, value, status, group) {

    var currentlyPlaying = engine.getValue("[Channel2]","play");
    if (currentlyPlaying == 1 && value == 0x7F) {    // If currently playing
        engine.setValue("[Channel2]","play",0);    // Stop
        midi.sendShortMsg(0x80,0x36,0x7F);    // Turn off the Play LED
    }
    if (currentlyPlaying == 0 && value == 0x7F) {
        engine.setValue("[Channel2]","play",1);    // Start
        midi.sendShortMsg(0x90,0x36,0x7F);    // Turn on the Play LED
    }

    if (currentlyPlaying == 0 && value == 0x00) {    // If no track loaded
        engine.setValue("[Channel2]","play",0);    // Stop
        midi.sendShortMsg(0x80,0x36,0x7F);    // Turn off the Play LED
    }

}

VestaxVCI1003DEX.pfl1 = function (channel, control, value, status, group) {


    var currentlyPfling = engine.getValue("[Channel1]","pfl");
    if (currentlyPfling == 1 && value == 0x7F) {    // If currently Pfling
        engine.setValue("[Channel1]","pfl",0);    // Stop
        midi.sendShortMsg(0x80,0x48,0x7F);    // Turn off the Pfl LED
    }
    if (currentlyPfling == 0 && value == 0x7F) {
        engine.setValue("[Channel1]","pfl",1);    // Start
        midi.sendShortMsg(0x90,0x48,0x7F);    // Turn on the Pfl LED
    }

}

VestaxVCI1003DEX.pfl2 = function (channel, control, value, status, group) {

    var currentlyPfling = engine.getValue("[Channel2]","pfl");
    if (currentlyPfling == 1 && value == 0x7F) {    // If currently Pfling
        engine.setValue("[Channel2]","pfl",0);    // Stop
        midi.sendShortMsg(0x80,0x49,0x7F);    // Turn off the Pfl LED
    }
    if (currentlyPfling == 0 && value == 0x7F) {
        engine.setValue("[Channel2]","pfl",1);    // Start
        midi.sendShortMsg(0x90,0x49,0x7F);    // Turn on the Pfl LED
    }

}

VestaxVCI1003DEX.flanger1 = function (channel, control, value, status, group) {

    var currentlyFlanging = engine.getValue("[Channel1]","flanger");
    if (currentlyFlanging == 1 && value == 0x7F) {    // If currently Flanging
        engine.setValue("[Channel1]","flanger",0);    // Stop
        midi.sendShortMsg(0x80,0x4A,0x7F);    // Turn off the flanger LED
    }
    if (currentlyFlanging == 0 && value == 0x7F) {
        engine.setValue("[Channel1]","flanger",1);    // Start
        midi.sendShortMsg(0x90,0x4A,0x7F);    // Turn on the flanger LED
    }

}
VestaxVCI1003DEX.flanger2 = function (channel, control, value, status, group) {

    var currentlyFlanging = engine.getValue("[Channel2]","flanger");
    if (currentlyFlanging == 1 && value == 0x7F) {    // If currently Flanging
        engine.setValue("[Channel2]","flanger",0);    // Stop
        midi.sendShortMsg(0x80,0x4B,0x7F);    // Turn off the flanger LED
    }
    if (currentlyFlanging == 0 && value == 0x7F) {
        engine.setValue("[Channel2]","flanger",1);    // Start
        midi.sendShortMsg(0x90,0x4B,0x7F);    // Turn on the flanger LED
    }

}


VestaxVCI1003DEX.autoloop1 = function (channel, control, value, status, group) {

if (value == 0x7F){
	engine.setValue("[Channel1]","beatloop_4_toggle",1);
	midi.sendShortMsg(0x90,0x35,0x7F);// right autoloop button led on does not work wtf
}

}

VestaxVCI1003DEX.autoloop2 = function (channel, control, value, status, group) {
if (value == 0x7F){
	engine.setValue("[Channel2]","beatloop_4_toggle",1);
	midi.sendShortMsg(0x90,0x39,0x7F);// right autoloop button led on does not work wtf
}

}

VestaxVCI1003DEX.keylock1 = function (channel, control, value, status, group) {


if (value == 0x7F)      {
			var keylocked = engine.getValue("[Channel1]","keylock");
			if(keylocked == 0) 	{
						engine.setValue("[Channel1]","keylock",1);
						midi.sendShortMsg(0x90,0x42,0x7F);// left pitchmode button led on
						}
			if(keylocked == 1) 	{
						engine.setValue("[Channel1]","keylock",0);
						midi.sendShortMsg(0x80,0x42,0x7F);// left pitchmode button led off
						}
			}

}

VestaxVCI1003DEX.keylock2 = function (channel, control, value, status, group) {


if (value == 0x7F)      {
			var keylocked = engine.getValue("[Channel2]","keylock");
			if(keylocked == 0) 	{
						engine.setValue("[Channel2]","keylock",1);
						midi.sendShortMsg(0x90,0x43,0x7F);// left pitchmode button led on
						}
			if(keylocked == 1) 	{
						engine.setValue("[Channel2]","keylock",0);
						midi.sendShortMsg(0x80,0x43,0x7F);// left pitchmode button led off
						}
			}

}

VestaxVCI1003DEX.reverse1 = function (channel, control, value, status, group) {


if (value == 0x7F)      {
			var reversed = engine.getValue("[Channel1]","reverse");
			if(reversed == 0) 	{
						engine.setValue("[Channel1]","reverse",1);
						midi.sendShortMsg(0x90,0x44,0x7F);// left reverse button led on
						}
			if(reversed == 1) 	{
						engine.setValue("[Channel1]","reverse",0);
						midi.sendShortMsg(0x80,0x44,0x7F);// left reverse button led off
						}
			}

}

VestaxVCI1003DEX.reverse2 = function (channel, control, value, status, group) {


if (value == 0x7F)      {
			var reversed = engine.getValue("[Channel2]","reverse");
			if(reversed == 0) 	{
						engine.setValue("[Channel2]","reverse",1);
						midi.sendShortMsg(0x90,0x45,0x7F);// left reverse button led on
						}
			if(reversed == 1) 	{
						engine.setValue("[Channel2]","reverse",0);
						midi.sendShortMsg(0x80,0x45,0x7F);// left reverse button led off
						}
			}

}

VestaxVCI1003DEX.cue1 = function (channel, control, value, status, group) {


if (value == 0x7F)      {
			var localCurrentCue=currentCue1;
			var cue1 = engine.getValue("[Channel1]","hotcue_1_enabled");
			var cue2 = engine.getValue("[Channel1]","hotcue_2_enabled");
			var cue3 = engine.getValue("[Channel1]","hotcue_3_enabled");
			var cue4 = engine.getValue("[Channel1]","hotcue_4_enabled");

			if(localCurrentCue==1 && cue1==1) 	{engine.setValue("[Channel1]","hotcue_1_goto",1);}
			if(localCurrentCue==2 && cue2==1) 	{engine.setValue("[Channel1]","hotcue_2_goto",1);}
			if(localCurrentCue==3 && cue3==1) 	{engine.setValue("[Channel1]","hotcue_3_goto",1);}
			if(localCurrentCue==4 && cue4==1) 	{engine.setValue("[Channel1]","hotcue_4_goto",1);}

			if (localCurrentCue+1 <= 4){currentCue1+=1;}
			else {currentCue1=1;}
			}

}

VestaxVCI1003DEX.cue2 = function (channel, control, value, status, group) {


if (value == 0x7F)      {
			var localCurrentCue=currentCue2;
			var cue1 = engine.getValue("[Channel2]","hotcue_1_enabled");
			var cue2 = engine.getValue("[Channel2]","hotcue_2_enabled");
			var cue3 = engine.getValue("[Channel2]","hotcue_3_enabled");
			var cue4 = engine.getValue("[Channel2]","hotcue_4_enabled");

			if(localCurrentCue==1 && cue1==1) 	{engine.setValue("[Channel2]","hotcue_1_goto",1);}
			if(localCurrentCue==2 && cue2==1) 	{engine.setValue("[Channel2]","hotcue_2_goto",1);}
			if(localCurrentCue==3 && cue3==1) 	{engine.setValue("[Channel2]","hotcue_3_goto",1);}
			if(localCurrentCue==4 && cue4==1) 	{engine.setValue("[Channel2]","hotcue_4_goto",1);}

			if (localCurrentCue+1 <= 4){currentCue2+=1;}
			else {currentCue2=1;}
			}


}

//sampler play buttons start

VestaxVCI1003DEX.sampler1play = function (channel, control, value, status, group) {

    var currentlyPlaying = engine.getValue("[Sampler1]","play");
    if (currentlyPlaying == 1 && value == 0x7F) {    // If currently playing
        engine.setValue("[Sampler1]","play",0);    // Stop
        midi.sendShortMsg(0x80,0x54,0x7F);    // Turn off the Play LED
    }
    if (currentlyPlaying == 0 && value == 0x7F) {
        engine.setValue("[Sampler1]","play",1);    // Start
        midi.sendShortMsg(0x90,0x54,0x7F);    // Turn on the Play LED
    }

    if (currentlyPlaying == 0 && value == 0x00) {    // If no track loaded
        engine.setValue("[Sampler1]","play",0);    // Stop
        midi.sendShortMsg(0x80,0x54,0x7F);    // Turn off the Play LED
    }

}

VestaxVCI1003DEX.sampler2play = function (channel, control, value, status, group) {

    var currentlyPlaying = engine.getValue("[Sampler2]","play");
    if (currentlyPlaying == 1 && value == 0x7F) {    // If currently playing
        engine.setValue("[Sampler2]","play",0);    // Stop
        midi.sendShortMsg(0x80,0x57,0x7F);    // Turn off the Play LED
    }
    if (currentlyPlaying == 0 && value == 0x7F) {
        engine.setValue("[Sampler2]","play",1);    // Start
        midi.sendShortMsg(0x90,0x57,0x7F);    // Turn on the Play LED
    }

    if (currentlyPlaying == 0 && value == 0x00) {    // If no track loaded
        engine.setValue("[Sampler2]","play",0);    // Stop
        midi.sendShortMsg(0x80,0x57,0x7F);    // Turn off the Play LED
    }

}

VestaxVCI1003DEX.sampler3play = function (channel, control, value, status, group) {

    var currentlyPlaying = engine.getValue("[Sampler3]","play");
    if (currentlyPlaying == 1 && value == 0x7F) {    // If currently playing
        engine.setValue("[Sampler3]","play",0);    // Stop
        midi.sendShortMsg(0x80,0x5A,0x7F);    // Turn off the Play LED
    }
    if (currentlyPlaying == 0 && value == 0x7F) {
        engine.setValue("[Sampler3]","play",1);    // Start
        midi.sendShortMsg(0x90,0x5A,0x7F);    // Turn on the Play LED
    }

    if (currentlyPlaying == 0 && value == 0x00) {    // If no track loaded
        engine.setValue("[Sampler3]","play",0);    // Stop
        midi.sendShortMsg(0x80,0x5A,0x7F);    // Turn off the Play LED
    }


}

//sampler play buttons end

//middle position of the knobs ot an extra midi signal.
VestaxVCI1003DEX.pregainreset1 = function (channel, control, value, status, group) {engine.setValue("[Channel1]","pregain",1.0);}
VestaxVCI1003DEX.filterHighreset1 = function (channel, control, value, status, group) {engine.setValue("[Channel1]","filterHigh",1.0);}
VestaxVCI1003DEX.filterMidreset1 = function (channel, control, value, status, group) {engine.setValue("[Channel1]","filterMid",1.0);}
VestaxVCI1003DEX.filterLowreset1 = function (channel, control, value, status, group) {engine.setValue("[Channel1]","filterLow",1.0);}
VestaxVCI1003DEX.pregainreset2 = function (channel, control, value, status, group) {engine.setValue("[Channel2]","pregain",1.0);}
VestaxVCI1003DEX.filterHighreset2 = function (channel, control, value, status, group) {engine.setValue("[Channel2]","filterHigh",1.0);}
VestaxVCI1003DEX.filterMidreset2 = function (channel, control, value, status, group) {engine.setValue("[Channel2]","filterMid",1.0);}
VestaxVCI1003DEX.filterLowreset2 = function (channel, control, value, status, group) {engine.setValue("[Channel2]","filterLow",1.0);}

// deck specific buttons end

VestaxVCI1003DEX.init = function (id) {
active_deck="[Channel1]";
active_deck_number=1
midi.sendShortMsg(0x90,0x67,0x7F);// deck- button led on
midi.sendShortMsg(0x80,0x68,0x7F);// deck+ button led off

// TODO SOFTTAKEOVER does not work(mixxbug) , AUTOLOOP LEDs, LOOP BAHAVIOUR with keylock on(mixxxbug) , cue shuffle mode,loopmove mode?
engine.softTakeover("[Channel1]", "rate", true); // wtf does not work

currentCue1=1;
currentCue2=1;

midi.sendShortMsg(0x80,0x32,0x7F);// left play button led off
midi.sendShortMsg(0x80,0x36,0x7F);// right play button led off


engine.setValue("[Channel1]","pfl",0);
engine.setValue("[Channel2]","pfl",0);
midi.sendShortMsg(0x80,0x48,0x7F);// left pfl button led off
midi.sendShortMsg(0x80,0x49,0x7F);// left pfl button led off

engine.setValue("[Channel1]","flanger",0);
engine.setValue("[Channel1]","flanger",0);
midi.sendShortMsg(0x80,0x4A,0x7F);// left send(fx) button led off
midi.sendShortMsg(0x80,0x4B,0x7F);// left send(fx) button led off

engine.setValue("[Channel1]","keylock",0);
engine.setValue("[Channel2]","keylock",0);
midi.sendShortMsg(0x80,0x42,0x7F);// left pitch mode button led off
midi.sendShortMsg(0x80,0x43,0x7F);// right pitch mode button led off

engine.setValue("[Channel1]","quantize",1);
engine.setValue("[Channel2]","quantize",1);


engine.setValue("[Channel1]","reverse",0);
engine.setValue("[Channel2]","reverse",0);
midi.sendShortMsg(0x80,0x44,0x7F);// left reverse button led off
midi.sendShortMsg(0x80,0x45,0x7F);// right reverse mode button led off

midi.sendShortMsg(0x80,0x62,0x7F);// loop in button led off
midi.sendShortMsg(0x80,0x63,0x7F);// loop out button led off
midi.sendShortMsg(0x80,0x64,0x7F);// reloop/exit  button led off

midi.sendShortMsg(0x80,0x46,0x7F);// left sync button led off
midi.sendShortMsg(0x80,0x47,0x7F);// rigth sync button led off

/*

!! = light does not work
!!CC = problems with controlchage or light does not work

midi.sendShortMsg(0x80,0x32,0x7F);// left play button led off
midi.sendShortMsg(0x90,0x32,0x7F);// left play button led on
midi.sendShortMsg(0x80,0x36,0x7F);// right play button led off
midi.sendShortMsg(0x90,0x36,0x7F);// right play button led on
midi.sendShortMsg(0x80,0x65,0x7F);// loop- button led off
midi.sendShortMsg(0x90,0x65,0x7F);// loop- button led on
midi.sendShortMsg(0x80,0x66,0x7F);// loop+ button led off
midi.sendShortMsg(0x90,0x66,0x7F);// loop+ button led on

midi.sendShortMsg(0x80,0x34,0x7F);// left brake button led off
midi.sendShortMsg(0x90,0x34,0x7F);// left brake button led on
midi.sendShortMsg(0x80,0x38,0x7F);// right brake button led off
midi.sendShortMsg(0x90,0x38,0x7F);// right brake button led on
midi.sendShortMsg(0x80,0x33,0x7F);// left cue button led off !!
midi.sendShortMsg(0x90,0x33,0x7F);// left cue button led on  !!

midi.sendShortMsg(0x80,0x37,0x7F);// right cue button led off !!
midi.sendShortMsg(0x90,0x37,0x7F);// right cue button led on  !!
midi.sendShortMsg(0x80,0x35,0x7F);// left autoloop button led off!!
midi.sendShortMsg(0x90,0x35,0x7F);// left autoloop button led on!!
midi.sendShortMsg(0x80,0x39,0x7F);// right autoloop button led off!!
midi.sendShortMsg(0x90,0x39,0x7F);// right autoloop button led on!!

midi.sendShortMsg(0x80,0x42,0x7F);// left pitch mode button led off
midi.sendShortMsg(0x90,0x42,0x7F);// left pitch mode button led on
midi.sendShortMsg(0x80,0x43,0x7F);// right pitch mode button led off
midi.sendShortMsg(0x90,0x43,0x7F);// right pitch mode button led on
midi.sendShortMsg(0x80,0x44,0x7F);// left reverse button led off
midi.sendShortMsg(0x90,0x44,0x7F);// left reverse button led on

midi.sendShortMsg(0x80,0x45,0x7F);// right reverse mode button led off
midi.sendShortMsg(0x90,0x45,0x7F);// right reverse mode button led on
midi.sendShortMsg(0x80,0x48,0x7F);// left pfl button led off
midi.sendShortMsg(0x90,0x48,0x7F);// left pfl button led on
midi.sendShortMsg(0x80,0x49,0x7F);// right pfl button led off
midi.sendShortMsg(0x90,0x49,0x7F);// right pfl button led on


midi.sendShortMsg(0x80,0x4A,0x7F);// left send(fx) button led off !!cc
midi.sendShortMsg(0x90,0x4A,0x7F);// left send(fx) button led on!!cc
midi.sendShortMsg(0x80,0x4B,0x7F);// right send(fx) button led off!!cc
midi.sendShortMsg(0x90,0x4B,0x7F);// right send(fx) button led on!!cc
midi.sendShortMsg(0x80,0x4C,0x7F);// Send selectio + pad 4 button led off!!cc
midi.sendShortMsg(0x90,0x4C,0x7F);// send selection + pad 4 button led on!!cc

midi.sendShortMsg(0x80,0x4D,0x7F);// selection up button led off !!cc
midi.sendShortMsg(0x90,0x4D,0x7F);// selection up button led on!!cc
midi.sendShortMsg(0x80,0x4E,0x7F);// selection up button led off !!cc
midi.sendShortMsg(0x90,0x4E,0x7F);// selection up button led on!!cc
midi.sendShortMsg(0x80,0x4F,0x7F);// selection up button led off !!cc
midi.sendShortMsg(0x90,0x4F,0x7F);// selection up button led on!!cc

midi.sendShortMsg(0x80,0x50,0x7F);// selection down button led off !!cc
midi.sendShortMsg(0x90,0x50,0x7F);// selection down button led on!!cc
midi.sendShortMsg(0x80,0x51,0x7F);// selection down button led off !!cc
midi.sendShortMsg(0x90,0x51,0x7F);// selection down button led on!!cc
midi.sendShortMsg(0x80,0x52,0x7F);// selection down button led off !!cc
midi.sendShortMsg(0x90,0x52,0x7F);// selection down button led on!!cc


// channel A
midi.sendShortMsg(0x80,0x53,0x7F);// pad1  button led off 
midi.sendShortMsg(0x90,0x53,0x7F);// pad1  button led on
midi.sendShortMsg(0x80,0x54,0x7F);// pad1  button led off 
midi.sendShortMsg(0x90,0x54,0x7F);// pad1  button led on
midi.sendShortMsg(0x80,0x55,0x7F);// pad1  button led off 
midi.sendShortMsg(0x90,0x55,0x7F);// pad1  button led on

//channel B
midi.sendShortMsg(0x80,0x56,0x7F);// pad2  button led off 
midi.sendShortMsg(0x90,0x56,0x7F);// pad2  button led on
midi.sendShortMsg(0x80,0x57,0x7F);// pad2  button led off 
midi.sendShortMsg(0x90,0x57,0x7F);// pad2  button led on
midi.sendShortMsg(0x80,0x58,0x7F);// pad2  button led off 
midi.sendShortMsg(0x90,0x58,0x7F);// pad2  button led on

// sampler
midi.sendShortMsg(0x80,0x59,0x7F);// pad3  button led off 
midi.sendShortMsg(0x90,0x59,0x7F);// pad3  button led on
midi.sendShortMsg(0x80,0x5A,0x7F);// pad3  button led off 
midi.sendShortMsg(0x90,0x5A,0x7F);// pad3  button led on
midi.sendShortMsg(0x80,0x5B,0x7F);// pad3  button led off 
midi.sendShortMsg(0x90,0x5B,0x7F);// pad3  button led on

midi.sendShortMsg(0x80,0x5C,0x7F);// up button led off !!
midi.sendShortMsg(0x90,0x5C,0x7F);// up button led on!!
midi.sendShortMsg(0x80,0x5D,0x7F);// down button led off!!
midi.sendShortMsg(0x90,0x5D,0x7F);// down button led on!!
midi.sendShortMsg(0x80,0x5E,0x7F);// view button led off!!
midi.sendShortMsg(0x90,0x5E,0x7F);// view button led on!!

midi.sendShortMsg(0x80,0x5F,0x7F);// view button led off
midi.sendShortMsg(0x90,0x5F,0x7F);// view button led on
midi.sendShortMsg(0x80,0x60,0x7F);// left button led off!!
midi.sendShortMsg(0x90,0x60,0x7F);// left button led on!!
midi.sendShortMsg(0x80,0x61,0x7F);// right button led off!!
midi.sendShortMsg(0x90,0x61,0x7F);// right button led on!!

midi.sendShortMsg(0x80,0x62,0x7F);// loop in button led off
midi.sendShortMsg(0x90,0x62,0x7F);// loop in button led on
midi.sendShortMsg(0x80,0x63,0x7F);// loop out button led off
midi.sendShortMsg(0x90,0x63,0x7F);// loop out button led on
midi.sendShortMsg(0x80,0x64,0x7F);// reloop/exit  button led off
midi.sendShortMsg(0x90,0x64,0x7F);// reloop/exit  button led on

midi.sendShortMsg(0x80,0x65,0x7F);// loop- button led off
midi.sendShortMsg(0x90,0x65,0x7F);// loop- button led on
midi.sendShortMsg(0x80,0x66,0x7F);// loop+ button led off
midi.sendShortMsg(0x90,0x66,0x7F);// loop+ button led on
midi.sendShortMsg(0x80,0x67,0x7F);// deck- button led off
midi.sendShortMsg(0x90,0x67,0x7F);// deck- button led on

midi.sendShortMsg(0x80,0x68,0x7F);// deck+ button led off
midi.sendShortMsg(0x90,0x68,0x7F);// deck+ button led on
midi.sendShortMsg(0x80,0x46,0x7F);// left sync button led off
midi.sendShortMsg(0x90,0x46,0x7F);// left sync button led on
midi.sendShortMsg(0x80,0x47,0x7F);// rigth sync button led off
midi.sendShortMsg(0x90,0x47,0x7F);// right sync button led on
*/
}
