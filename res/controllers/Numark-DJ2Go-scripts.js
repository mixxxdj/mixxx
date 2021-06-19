//Written by Alimantado, modified by Coval 11 Sep 2012. Thanks to authors of other scripts, especially the Vestax ones.
//Made to be close as possible to how DJ2Go works with VDJ.

NumarkDJ2Go = new function() {
	this.decks = [];
	this.playlist = [];
	this.master = [];
};

var shiftUsed=0;
var shiftA=0;
var shiftB=0;
var shiftBck=0;
var shiftEnt=0;

NumarkDJ2Go.shiftA = function(channel, control, value, status, group) {
shiftA = ((status=="0x90") ? 1 : 0);
shiftUsed=0;
}
NumarkDJ2Go.shiftB = function(channel, control, value, status, group) {
shiftB = ((status=="0x90") ? 1 : 0);
shiftUsed=0;
}
NumarkDJ2Go.shiftBck = function(channel, control, value, status, group) {
shiftBck = ((status=="0x90") ? 1 : 0);
shiftUsed=0;
}
NumarkDJ2Go.shiftEnt = function(channel, control, value, status, group) {
shiftEnt = ((status=="0x90") ? 1 : 0);
shiftUsed=0;
}


NumarkDJ2Go.flip = function(group, key) {
	var flip=engine.getValue(group, key);
	flip = (flip != true);
	engine.setValue(group, key, flip);
	}

//Initialise and shutdown stuff.
//========================================================

NumarkDJ2Go.init = function(id) {
//Connect button lights to equivalent controls. The track_samples control
//is being used to tell if a track has successfully loaded (whereupon lights
//flash twice. Best way I could think of in the absence of a proper control
//for checking whether a track is loaded.





engine.connectControl("[Channel1]","track_samples","NumarkDJ2Go.loadLights");
engine.connectControl("[Channel2]","track_samples","NumarkDJ2Go.loadLights");
engine.connectControl("[Channel1]","pfl","NumarkDJ2Go.pflLights");
engine.connectControl("[Channel2]","pfl","NumarkDJ2Go.pflLights");
engine.connectControl("[Channel1]","cue_default","NumarkDJ2Go.cueLights");
engine.connectControl("[Channel2]","cue_default","NumarkDJ2Go.cueLights");
engine.connectControl("[Channel1]","play","NumarkDJ2Go.playLights");
engine.connectControl("[Channel2]","play","NumarkDJ2Go.playLights");
engine.connectControl("[Channel1]","beat_active","NumarkDJ2Go.syncLights");
engine.connectControl("[Channel2]","beat_active","NumarkDJ2Go.syncLights");
NumarkDJ2Go.manualLooping = [false, false];
};

NumarkDJ2Go.shutdown = function(id) {
//Turn off all controller lights at shutdown.
midi.sendShortMsg(0x90,0x33,0x00);
midi.sendShortMsg(0x90,0x3C,0x00);
midi.sendShortMsg(0x90,0x3B,0x00);
midi.sendShortMsg(0x90,0x42,0x00);
midi.sendShortMsg(0x90,0x40,0x00);
midi.sendShortMsg(0x90,0x47,0x00);
midi.sendShortMsg(0x90,0x65,0x00);
midi.sendShortMsg(0x90,0x66,0x00);
};

//Setting up classes for objects
//========================================================

//The deck class holds functions for checking attributes about the deck (e.g.
//whether track loaded, holds the functions for scratching/pitch bending on the
//jog wheel, and has a 'control' array, which holds all the controls (buttons etc)
//associated with the deck. The jog wheels haven't been treated like the other controls
//because there is only one per deck. Obviously two deck objects are created: D1 and D2.
NumarkDJ2Go.deck = function(deckNum) {
	this.deckNum = deckNum;
	this.group = "[Channel" + deckNum + "]";
	this.loadedCheck = function() {
		var yesno = (engine.getValue(this.group, "track_samples") > 0)?true:false;
		return yesno;
	};
	//Brake effect introduced in Mixxx 1.11
	this.brakeOn = function(factor) {
        	engine.brake(this.deckNum, true, factor);
		this.braked= true;
        };

	//Turns brake off. Needed because brake stays on after track has stopped, and play/pause etc
	//can't be used until brake turned off again.
        this.brakeOff = function() {
        	engine.brake(this.deckNum, false); // disable brake effect
		this.braked= false;
        };
	//Attribute for whether brake is applied to deck.
	this.braked= false;
	//Timer used to turn off scratch mode.
	this.scratchTimer = 0;
	//Enables scratching. While is on, playing/pausing not possible, so scratchOff function (below) also needed.
	this.scratchOn = function() {
		var intervalsPerRev = 60; //DJ2Go jog wheel is 60 intervals per revolution.
		var rpm = 85; //Adjust to suit.
		var alpha = (0.1); //Adjust to suit.
		var beta = (alpha/30); //Adjust to suit.
		engine.scratchEnable(this.deckNum, intervalsPerRev, rpm, alpha, beta);
	};
	this.scratchOff = function() {
		engine.scratchDisable(this.deckNum);
		this.scratchTimer= 0;
	};
	//Function that actually does the scratching in response to moving the jog wheel. The timer is
	//used to automatically turn the scratch mode off when the jog wheel stops moving for a period.
	//This allows play to resume when scratching finished.
	this.scratch  = function(forwards) {
		if (this.scratchTimer !== 0)
			{
			engine.stopTimer(this.scratchTimer);
			};
		var playDelay = 40; //Adjust to suit.
		var scrConst = 1;  //Adjust to suit.
		var scrVal = (forwards)?scrConst:-scrConst;
		engine.scratchTick(this.deckNum, scrVal);
		this.scratchTimer = engine.beginTimer(playDelay,"NumarkDJ2Go.decks.D" + this.deckNum + ".scratchOff()", true);
	};
	//Pitchbend attribute. Required for pitchbend to be ramping (i.e speeds up/slows down the more the wheel is moved).
	this.bendVal= 0;
	//Timer used to turn of pitch bend.
	this.pitchTimer= 0;
	//The alternative mode to scratching for the jog wheel.
	this.pitchBend = function(forwards) {
		//For some reason the ramping pitchbend option in Mixxx menu together with temp_rate_up/down doesn't seem
		//to work for the jog wheel. So this function allows the pitch bend to ramp the faster/more
		//revolutions of the wheel.
		if (this.pitchTimer !== 0)
			{
			engine.stopTimer(this.pitchTimer);
			};
		var bendConst = 0.002; //Adjust to suit.
		var nVal = (Math.abs(this.bendVal) + bendConst); //Turn bendVal to absolute value and add.
		nVal = (nVal > 3.0)?3.0:nVal; //If gone over 3, keep at 3.
		this.bendVal = (forwards)?nVal:-nVal; //Return to positive or minus number.
		engine.setValue(this.group, "jog", this.bendVal);
		this.pitchTimer = engine.beginTimer(20,"NumarkDJ2Go.decks.D" + this.deckNum + ".pitchBendOff()", true);
	};
	//Used by function above. Turns pitchbend off.
	this.pitchBendOff = function() {
		this.bendVal = 0;
		this.pitchTimer= 0;
	};
	//Controls for the deck--buttons, sliders, etc--are associated with the deck using this array.
	this.control = [];
	this.beatActive = function(){
	if (engine.getValue(this.group, "beat_active")) {
	    this.beatLed = true
            }
            else {
                 this.beatLed= false;
                 }
        }
        this.jump2begin = function (){
            engine.setValue(this.group,"playposition",0);
            };

};

//Control class for control objects, e.g. play button. Objects for the two jog wheels are not created this way, instead they are
//represented directly by the deck objects. This was just an easier way to do it, and there will only ever be one jog wheel
//per deck anyway.
NumarkDJ2Go.control = function(key, midino, group) {
	this.key = key;
	this.midino = midino;
	this.group = group;
	this.onOff = function(value) {
		engine.setValue(this.group, this.key, value);
	};
	this.checkOn = function() {
		var checkOn = engine.getValue(this.group, this.key);
		return checkOn;
	};
};

//Has on/off and flashing modes for all button lights. Light objects are only created for buttons that can
//actually illuminate.
NumarkDJ2Go.light = function(group, midino, deckID, controlID) {
	this.midino = midino;
	this.objStr= "NumarkDJ2Go.decks." + deckID + ".control." + controlID + ".light"
	this.lit = false;
	this.flashTimer= 0;
	this.flashOnceTimer= 0;
	this.onOff = function(value) {
		midi.sendShortMsg(0x90, this.midino, value);
		this.lit = value;
	};
	this.flashOnceOn = function() {
		midi.sendShortMsg(0x90, this.midino,1);
		this.flashOnceTimer = engine.beginTimer(150, this.objStr + ".flashOnceOff()", true);
	};
	this.flashOnceOff = function() {
		midi.sendShortMsg(0x80, this.midino,1);
		this.flashOnceTimer = 0;
	};
	this.flashOff = function(relight) {
		if (this.flashTimer !== 0)
			{
			engine.stopTimer(this.flashTimer);
			this.flashTimer= 0;
			};
		if (this.flashOnceTimer !== 0)
			{
			engine.stopTimer(this.flashOnceTimer);
			this.flashOnceTimer= 0;
			};
		if (relight)
			{
			this.onOff(1);
			}
		else
			{
			this.onOff(0);
			};
	};
	this.flashOn = function(flashNo) {
		var relight = this.lit;
		this.flashOff();
		this.flashOnceOn(); //This is because the timer take 600 milisecs before first flash.
		this.flashTimer = engine.beginTimer(600, this.objStr + ".flashOnceOn()");
		if (flashNo)
			{
			engine.beginTimer((flashNo * 600) -50, this.objStr + ".flashOff(" + relight + ")", true);
			};
	};
};

//Constructor for creating control objects
NumarkDJ2Go.addControl = function(arrID, ID, controlObj, addLight) {
	var arrAdd = this[arrID];
	if (addLight)
		{
		//If the button can illuminate, a light object is created for it (see above).
		controlObj.light = new NumarkDJ2Go.light(this.group, controlObj.midino, "D" + this.deckNum,ID);
		};
	arrAdd[ID] = controlObj;
};

//This creates an 'addControl' constructor function to the deck class, using the above function as a template.
NumarkDJ2Go.deck.prototype.addControl = NumarkDJ2Go.addControl;


//Creating the actual deck/control objects and adding them to the appropriate arrays.
//====================================================================================
//The only object that goes into the playlist array.
NumarkDJ2Go.addControl("playlist", "selectKnob", new NumarkDJ2Go.control("", 0x1A, "[Playlist]"));

//Creating the two deck objects.
NumarkDJ2Go.addControl("decks", "D1", new NumarkDJ2Go.deck("1"));
NumarkDJ2Go.addControl("decks", "D2", new NumarkDJ2Go.deck("2"));

NumarkDJ2Go.addControl("playlist", "enterBut", new NumarkDJ2Go.control("LoadSelectedIntoFirstStopped", 0x5A, "[Playlist]"));
NumarkDJ2Go.addControl("playlist", "backBut", new NumarkDJ2Go.control("ToggleSelectedSidebarItem", 0x59, "[Playlist]"));


//All controls below associated with left or right deck.
NumarkDJ2Go.decks.D1.addControl("control", "load", new NumarkDJ2Go.control("LoadSelectedTrack", 0x4B, "[Channel1]"));
NumarkDJ2Go.decks.D2.addControl("control", "load", new NumarkDJ2Go.control("LoadSelectedTrack", 0x34, "[Channel2]"));

NumarkDJ2Go.decks.D1.addControl("control", "sync", new NumarkDJ2Go.control("beatsync", 0x40, "[Channel1]"), true);
NumarkDJ2Go.decks.D2.addControl("control", "sync", new NumarkDJ2Go.control("beatsync", 0x47, "[Channel2]"), true);

NumarkDJ2Go.decks.D1.addControl("control", "beatTapCurPos", new NumarkDJ2Go.control("beats_translate_curpos", 0x40, "[Channel1]"), true);
NumarkDJ2Go.decks.D2.addControl("control", "beatTapCurPos", new NumarkDJ2Go.control("beats_translate_curpos", 0x47, "[Channel2]"), true);

NumarkDJ2Go.decks.D1.addControl("control", "bpmTap", new NumarkDJ2Go.control("bpm_tap", 0x40, "[Channel1]"), true);
NumarkDJ2Go.decks.D2.addControl("control", "bpmTap", new NumarkDJ2Go.control("bpm_tap", 0x47, "[Channel2]"), true);

NumarkDJ2Go.decks.D1.addControl("control", "pfl", new NumarkDJ2Go.control("pfl", 0x65, "[Channel1]"), true);
NumarkDJ2Go.decks.D2.addControl("control", "pfl", new NumarkDJ2Go.control("pfl", 0x66, "[Channel2]"), true);

NumarkDJ2Go.decks.D1.addControl("control", "cue", new NumarkDJ2Go.control("cue_default", 0x33, "[Channel1]"), true);
NumarkDJ2Go.decks.D2.addControl("control", "cue", new NumarkDJ2Go.control("cue_default", 0x3C, "[Channel2]"), true);

NumarkDJ2Go.decks.D1.addControl("control", "play", new NumarkDJ2Go.control("play", 0x3B, "[Channel1]"), true);
NumarkDJ2Go.decks.D2.addControl("control", "play", new NumarkDJ2Go.control("play", 0x42, "[Channel2]"), true);

NumarkDJ2Go.decks.D1.addControl("control", "bendMinus", new NumarkDJ2Go.control("rate_temp_down", 0x44, "[Channel1]"));
NumarkDJ2Go.decks.D2.addControl("control", "bendMinus", new NumarkDJ2Go.control("rate_temp_down", 0x46, "[Channel2]"));

NumarkDJ2Go.decks.D1.addControl("control", "bendPlus", new NumarkDJ2Go.control("rate_temp_up", 0x43, "[Channel1]"));
NumarkDJ2Go.decks.D2.addControl("control", "bendPlus", new NumarkDJ2Go.control("rate_temp_up", 0x45, "[Channel2]"));



//Mapping the buttons, sliders, wheels etc.
//================================================================================

    //Changes the selectKnob (below) dirMode attribute to 'playlist' if 'track'. If dirMode attribute
    //already 'playlist' or not yet set, attempts to expand or contract the select directory tree.
    NumarkDJ2Go.backBut = function(channel, midino, value) {
       var backBut = NumarkDJ2Go.playlist.backBut;
       var selectKnob = NumarkDJ2Go.playlist.selectKnob;
       shiftBck=0;
       if (shiftUsed==0)
          {

          if (("dirMode" in selectKnob) && (selectKnob.dirMode == "Track"))
             {
             selectKnob.dirMode = "Playlist";
             }
             else
                 {
                 backBut.onOff(1);
                 };
          };
    };

    //Changes the selectKnob (below) dirMode attribute to 'track' if 'playlist'. If dirMode attribute
    //already 'track', loads the selected track into the first available deck.
    NumarkDJ2Go.enterBut = function(channel, midino, value) {
       var enterBut = NumarkDJ2Go.playlist.enterBut;
       var selectKnob = NumarkDJ2Go.playlist.selectKnob;
       shiftEnt=0;
       if (shiftUsed==0)
          {

          if (("dirMode" in selectKnob) && (selectKnob.dirMode == "Track"))
             {
             enterBut.onOff(1);
             }
             else
                 {
                 selectKnob.dirMode = "Track";
                 };
          };
    };

//Depending on the dirMode attribute, either scrolls up and down the directory tree or the tracklist.
NumarkDJ2Go.selectKnob = function(channel, midino, value) {
	var selectKnob = NumarkDJ2Go.playlist.selectKnob;
	if (!("dirMode" in selectKnob))
		{
		selectKnob.dirMode = "Playlist"; //Assumes playlist if back/enter buttons never been pressed.
		};
	selectKnob.key = (value == 0x7F)?"SelectPrev"+selectKnob.dirMode:"SelectNext"+selectKnob.dirMode;
	selectKnob.onOff(1);
};

//On pressing A or B loads new track into deck unless track is currently playing, in which
//case starts track again from beginning, ignoring any cue points.
NumarkDJ2Go.load = function(channel, midino, value, status, group) {
	var deck = NumarkDJ2Go.decks["D" + group.substring(8,9)];
	if (deck.control.play.checkOn())
		{
		if (deck.scratchMode)
			{
			deck.scratchMode = false;
			}
		else
			{
			deck.scratchMode = true;
			};
		}
	else
		{
		deck.control.load.onOff(1);
		};
};

//Turns on/off headphone monitor for the deck, but also turns off the headphone monitor for the other deck.
//This is how VDJ does it with DJ2Go.
NumarkDJ2Go.pfl = function(channel, midino, value, status, group) {
        if (shiftBck) {
	   NumarkDJ2Go.flip(group,"flanger");
	   shiftUsed=1;
	   return;
           }
        else {
	        var deck = NumarkDJ2Go.decks["D" + group.substring(8,9)];
	        if (deck.control.pfl.checkOn())
		   {
		   deck.control.pfl.onOff(0);
		   }
	         else {
		      NumarkDJ2Go.decks.D1.control.pfl.onOff(0);
		      NumarkDJ2Go.decks.D2.control.pfl.onOff(0);
		      deck.control.pfl.onOff(1);
		      };
             };
        };




//Cue. Reacts to both status on and status off bytes (i.e. button held down and released).
NumarkDJ2Go.cue = function(channel, midino, value, status, group) {
        var deck = NumarkDJ2Go.decks["D" + group.substring(8,9)];
        if (shiftBck){
           deck.jump2begin();
           return;
           }
          else {
	       //Need to disable the deck brake first, if it is applied.
	       if (deck.braked)
                  {
		  deck.brakeOff();
		  };
	       if (deck.loadedCheck())
		   {
		   var onoff = (status == 0x90)?1:0;
		   deck.control.cue.onOff(onoff);
		   };
              }
};

//Play. Does deck brake instead of pause if scratch mode selected.
NumarkDJ2Go.play = function(channel, midino, value, status, group) {
           if (status==0x80)
                {
                if (shiftBck) {
                              engine.setValue(group, "reverse", 0);
                              shiftUsed=1;
                              };
                }
                else {
                     if (shiftBck){
                                   engine.setValue(group, "reverse", 1);
                                   shiftUsed=1;
                                   return;
                                   }
	               var deck = NumarkDJ2Go.decks["D" + group.substring(8,9)];
	               //Turns deck brake off if it is on.
	               if (deck.braked)
		              {
		              deck.brakeOff();
		              };
	                 if (deck.loadedCheck())
                            {
		            if (deck.control.play.checkOn())
		               {
		               if (deck.scratchMode)
                                  {
//    	               		deck.brakeOn(750);
                                  };
		               deck.control.play.onOff(0);
		               }
                               else {
                	            deck.control.play.onOff(1);
                	            };
	                     };
	             };
};

//Jog wheel. Scratches or pitchbends depending on whether scratch mode selected.
NumarkDJ2Go.wheel = function (channel, midino, value, status, group) {
	var deck = NumarkDJ2Go.decks["D" + group.substring(8,9)];
	var forwards = (value == 0x7F)?false:true;
	if (shiftEnt) {
	   deck.scratchMode=1;
	   shiftUsed=1;
           };
		if (deck.scratchMode)
			{
			if (deck.scratchTimer == 0)
				{
				//If no scratch timer (i.e. jog wheel not already being scratched), turns scratch mode on.
				deck.scratchOn();
				};
			//Does scratching. Whether back or forward given in 'forwards' variable.
			deck.scratch(forwards);
			deck.scratchMode=0;
			}
		else
			{
			//Does pitchbend.
			deck.pitchBend(forwards);
			};
};

//Pitch bend down. This toggles off the pitch bend up and pitches down while the
//button is held. Turns off when button released.
NumarkDJ2Go.pitchBendMinus = function(channel, midino, value, status, group) {
	var deck = NumarkDJ2Go.decks["D" + group.substring(8,9)];
	if (!deck.control.bendPlus.checkOn())
		{
		if (status == 0x90)
			{
			deck.control.bendPlus.onOff(0)
			deck.control.bendMinus.onOff(1)
			}
		else
			{
			deck.control.bendMinus.onOff(0)
			}
	};
};

//Pitch bend up. This toggles off the pitch bend down and pitches up while the
//button is held. Turns off when button released.
NumarkDJ2Go.pitchBendPlus = function(channel, midino, value, status, group) {
	var deck = NumarkDJ2Go.decks["D" + group.substring(8,9)];
	if (!deck.control.bendMinus.checkOn())
		{
		if (status == 0x90)
			{
			deck.control.bendMinus.onOff(0)
			deck.control.bendPlus.onOff(1)
			}
		else
			{
			deck.control.bendPlus.onOff(0)
			}
	};
};

NumarkDJ2Go.sync =function (channel, midino, value, status, group) {
        var deck = NumarkDJ2Go.decks["D" + group.substring(8,9)];

        if (shiftEnt) {
           deck.control.beatTapCurPos.onOff(1);
           deck.control.beatTapCurPos.onOff(0);
	   shiftUsed=1;
	   return;
           }
           else if (shiftBck){
                   deck.control.bpmTap.onOff(1);
                   deck.control.bpmTap.onOff(0);
                   shiftUsed=1;
                   return;
                   }
           else {
                deck.control.sync.onOff(1);
                deck.control.sync.onOff(0);
                }
        };




//Light functions. These are connected to Mixxx controls in the 'init'
//part of the script (see top) and are called whenever the Mixxx control
//changes, e.g. play value changes from 1 to 0. The lights are handled this
//way so that they activate both when using the controller and when
//using the mouse on screen.
//===================================================================

//All four button lights flash twice if track successfully loads, then
//cue and play continue flashing, as track now in pause mode.
NumarkDJ2Go.loadLights = function(value,group) {
	var deck = NumarkDJ2Go.decks["D" + group.substring(8,9)];
	if (value !== 0)
		{
		deck.control.play.light.onOff(0);
		deck.control.sync.light.flashOn(2);
		deck.control.pfl.light.flashOn(2);
		deck.control.play.light.flashOn();
		deck.control.cue.light.flashOn();
		}
	else
		{
		deck.control.play.light.flashOff();
		deck.control.sync.light.flashOff();
		deck.control.pfl.light.flashOff();
		deck.control.play.light.flashOff();
		deck.control.cue.light.flashOff();
		};
};

//Headphone monitor lights.
NumarkDJ2Go.pflLights = function(value, group, key) {
	var deck = NumarkDJ2Go.decks["D" + group.substring(8,9)];
	deck.control.pfl.light.onOff(value);
};

//Cue light. The play and cue lights behave the same way as they do with VirtualDJ.
NumarkDJ2Go.cueLights = function(value, group, key) {
	var deck = NumarkDJ2Go.decks["D" + group.substring(8,9)];
	if (deck.loadedCheck())
		{
		if (value == 0)
			{
			deck.control.cue.light.flashOn();
			deck.control.play.light.flashOn();
			}
		else
			{
			deck.control.cue.light.flashOff();
			deck.control.play.light.flashOff(1);
			};
		};
};

//Play lights. If play paused, both cue and play flash. If play resumed, play button
//lights up and cue goes off. This is how the buttons behave with VirtualDJ.
NumarkDJ2Go.playLights = function(value, group, key) {
	var deck = NumarkDJ2Go.decks["D" + group.substring(8,9)];
	if (deck.loadedCheck())
		{
		if (value == 1)
			{
			deck.control.play.light.flashOff(1);
			deck.control.cue.light.flashOff();
			}
		else
			{
			deck.control.play.light.flashOn();
			deck.control.cue.light.flashOn();
			};
		};
};

//Mixxx's sync feature is not the same as VDJ, where syncing appears to
//be continuously going on. Therefore less relevance with Mixxx to having a sync button
//that illuminates. Have set it so that it flashes twice when pressed.
NumarkDJ2Go.syncLights = function(value, group, key) {
	var deck = NumarkDJ2Go.decks["D" + group.substring(8,9)];
        deck.control.sync.light.flashOnceOn();
}


// Loop by Coval
NumarkDJ2Go.groupToDeck = function(group) {
	var matches = group.match(/^\[Channel(\d+)\]$/);
	if (matches == null) {
		return -1;
	} else {
		return matches[1];
	}
}



NumarkDJ2Go.loopIn = function(channel, control, value, status, group) {
        var deck = NumarkDJ2Go.groupToDeck(group);
	if (value) {
		if(NumarkDJ2Go.manualLooping[deck-1]) {
				// Cut loop to Half
				var start = engine.getValue(group, "loop_start_position");
				var end = engine.getValue(group, "loop_end_position");
				if((start != -1) && (end != -1)) {
					var len = (end - start) / 2;
					engine.setValue(group, "loop_end_position", start + len);
				}
		} else {
			engine.setValue(group, "loop_in", 1);
		}
	}
}  //loopIn

NumarkDJ2Go.loopOut = function(channel, control, value, status, group) {
        var deck = NumarkDJ2Go.groupToDeck(group);
	if (value) {
		var start = engine.getValue(group, "loop_start_position");
		var end = engine.getValue(group, "loop_end_position");
		if(NumarkDJ2Go.manualLooping[deck-1]) {
			// Set loop to current Bar (very approximative and would need to get fixed !!!)
			var bar = NumarkDJ2Go.samplesPerBeat(group);
			engine.setValue(group,"loop_in",1);
			var start = engine.getValue(group, "loop_start_position");
			engine.setValue(group,"loop_end_position", start + bar);
		} else {
			if (start != -1) {
				if (end != -1) {
					// Loop In and Out set -> call Reloop/Exit
					engine.setValue(group, "reloop_exit", 1);
					engine.setValue(group, "loop_in",0);
					engine.setValue(group, "loop_out",0);
//					engine.setValue(group, "loop_start_position",-1);
					engine.setValue(group, "loop_end_position",-1);
				} else {
					// Loop In set -> call Loop Out
					engine.setValue(group, "loop_out", 1);
					engine.setValue(group,"reloop",1);
				}
			}
		}
	}
}; //loopOut






