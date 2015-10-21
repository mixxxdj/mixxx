/**
 * Reloop Beeatpad controller script v1.0.0 for Mixxx 1.12
 * Copyright (C) 2015  Chloé AVRILLON
 * chloe.avrillon@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/
//Written by Chloé AVRILLON (DJ Chloé). Thanks to authors of other scripts
//and particularly to authors of Numark Dj2Go, KANE QuNeo, Vestax-VCI-400
// various new things not seen in other mapings like :
// - shift+wheelturn in "Jog Scratch" mode do automatic cut of the fader while scratching

ReloopBeatpad = new function () {};
//Global constants
const 	ON = 0x7F, OFF = 0x00,
		DOWN = 0x7F, UP = 0x00,
		SHIFT = -0x40,
		LBtn=0x90, RBtn=0x91, MBtn=0x94,
		CUEMODE=0, LOOPMODE=1, FXMODE=2, SAMPLERMODE=3, 
		SAMPLERBANKSTATUSMODE=4,LOOPMODESTATUS=5,FXRACKSELECTMODE=6;
		SIMPLE=1, ROLL=2,  //loop kind
		ControllerStatusSysex = [ 0xF0, 0x26, 0x2D, 0x65, 0x22, 0xF7 ],
		HardwareLight=false;
		scriptpause=5; //period (in ms) while the script wwill be paused when sending messages to the controller in order to avoid too much data flow at once in the same time

//Utilities
function pauseScript (ms) {
    startDate = new Date();
    currentDate = null;
    while(currentDate-startDate < ms) {currentDate = new Date();}    
}

Math.sign = Math.sign || function(x) {
    x = +x; // convert the parameter into a number
    if (x === 0 || isNaN(x)){
        return x;
    }
    return x > 0 ? 1 : -1;
};

Number.prototype.mod = function(n) { return ((this%n)+n)%n; }; //computes the mathematical modulus

//Objects
//Arrays
	ReloopBeatpad.decks = [];
	ReloopBeatpad.playlist = [];
	ReloopBeatpad.master = [];
	ReloopBeatpad.recording = [];
	
//Constants
ReloopBeatpad.MIDI = {
	rec :0x41,
	Trackpush :0x46,
    BendMinus : 0x41,
    BendPlus : 0x42,
    //Jog modes
    Vinyl : 0x45,
    iScratch : 0x46,
    //Pitch+FX switchs
    Loop : 0x44,
    Loop_size_push :0x43,
    FX_ON : 0x47,	
      //Performance mode
    Cue : 0x48,
    Loop_Bounce : 0x49,
	InstantFX : 0x4A,
	Sampler :0x4B,
    CuePad : 0x4C, //(4 Buttons 0x4C~0x4F (+0--+4)
	LoopPad : 0x50, //(4 Buttons 0x50~0x53 (+0--+4)
	FXPad : 0x54, //(4 Buttons 0x54~0x57 (+0--+4)
    SamplerPad : 0x58, //(4 Sampler Buttons 0x58~0x5B (+0--+4)  (RGB color leds)
    Shift : 0x5C,
    Sync : 0x5D,
    Set : 0x5E,  //"Cue Play" in the Beatpad documentation
    Jump : 0x5F, //"MainCue" in the Beatpad documentation
    Play : 0x60,
    pfl : 0x61,
    Load : 0x62,
    VUMeter : 0x66,  //values : 0x00~0x08
    //Vinyl RIM Leds
    RIM_Red : 0x67, // 1st behaviour 0x01-0x18 ; 2nd Behavior = 1st +24 ;3d behavior ON/OFF
    RIM_Blue : 0x68, // 1st behaviour 0x05-0x08 ; 2nd Behavior = 1st -4;3d behavior ON/OFF
    RIM_RGB : 0x69 //(4 RGB Leds 0x69~0x6C (+0--+4)
};

ReloopBeatpad.RGB = {
    black : 0x00,
    red : 0x01,
    green : 0x02,
    blue : 0x03,
    yellow : 0x04,
    magenta : 0x05,
    cyan : 0x06,
    white : 0x07
};

ReloopBeatpad.PadColor = {
	black : 0x00,
	blue : 0x01,
	magenta : 0x02,
	uv : 0x03,
	purple : 0x04,
	indigo : 0x05,
	fushia : 0x06,
	lilac : 0x07,
	orange : 0x08
};


//Initialise and shutdown stuff.
//========================================================
// ----------   Functions   ----------

ReloopBeatpad.TurnLEDsOff = function () {
    // Turn all LEDS off 
    for(i = LBtn; i <= RBtn; i++) {  // 2 decks
        midi.sendShortMsg(i, 0x41, OFF);
        midi.sendShortMsg(i, 0x42, OFF);
        midi.sendShortMsg(i, 0x44, OFF);
        for (j = 0x47; j <= 0x62; j++) {
            midi.sendShortMsg(i, j, OFF);
            midi.sendShortMsg(i, j+SHIFT, OFF);
        }
        for (j=0x66; j <= 0x6C; j++) {
            midi.sendShortMsg(i, j, OFF);
        }
    }
};

ReloopBeatpad.shutdown = function () {
    // Stop all timers
    for(i = 0; i < ReloopBeatpad.timers.length; i++) {
        engine.stopTimer(ReloopBeatpad.timers[i]);
    }
    // Extinguish all LEDs
    ReloopBeatpad.TurnLEDsOff();
    print ("Reloop Beatpad: " + ReloopBeatpad.id + " shut down.");
};


//Setting up classes for objects
//========================================================

ReloopBeatpad.getduration = function(group) {
	return engine.getValue(group,"track_samples")/engine.getValue(group,"track_samplerate")/2; 
};

ReloopBeatpad.SamplerBank = function () {
	this.bankactive=1;
	this.loaded=[];
	this.loaded.length=17;
	
	
	this.LedUpdate = function (padindex) {
		var color,isloaded,isplaying,deck;
		var samplerbaseindex,samplerindex;
		for(decknum=1;decknum<=2;decknum++){
			deck = ReloopBeatpad.decks["D" + decknum];
			switch (deck.PadMode) { 
				//Sampler bank select status
				case SAMPLERBANKSTATUSMODE :
					color=(padindex==this.bankactive)?ReloopBeatpad.PadColor.magenta:0;
					deck.status["SamplerPad"+padindex].light.onOff(color);
					break;
				//Loop mode Status
				case LOOPMODESTATUS : break;
				//FX rack Select status
				case FXRACKSELECTMODE :
					color=(padindex==deck.CurrentEffectRack)?ReloopBeatpad.PadColor.purple:0;
					deck.status["SamplerPad"+padindex].light.onOff(color);
					break;
				//normal sampler mode
				default :
					samplerbaseindex=(this.bankactive-1)*4;
					samplerindex=samplerbaseindex+padindex;
					isloaded=this.loaded[samplerindex];
					isplaying=engine.getValue("[Sampler"+samplerindex+"]","play");
					color=ReloopBeatpad.PadColor.orange;
					if (isplaying) {color=ReloopBeatpad.PadColor.uv;}
					deck.status["SamplerPad"+padindex].light.onOff(isloaded?color:OFF);
					break;
			}
		}
	};
	
	this.LedUpdateSampler = function (samplernum) {
		var samplerbaseindex=(this.bankactive-1)*4;
		if ((samplerbaseindex<samplernum)&&(samplernum<=(samplerbaseindex+4))) {
			padindex=samplernum-samplerbaseindex;
			this.LedUpdate(padindex);
		}
	};
	
	this.LedsUpdate = function () {
		var samplerbaseindex=(this.bankactive-1)*4;
		var samplerindex=0;
		var color=0;
		var isloaded, isplaying,deck;
		
		for(decknum=1;decknum<=2;decknum++){
			deck = ReloopBeatpad.decks["D" + decknum];
			switch (deck.PadMode) { 
				//Sampler bank select status
				case SAMPLERBANKSTATUSMODE :
					for(i=1;i<=4;i++){
						color=(i==this.bankactive)?ReloopBeatpad.PadColor.fushia:0;
						deck.status["SamplerPad"+i].light.onOff(color);
					}
					break;
				//Loop mode Status
				case LOOPMODESTATUS : 
				    deck.status["SamplerPad1"].light.onOff(ReloopBeatpad.PadColor.magenta);
				    deck.status["SamplerPad2"].light.onOff(OFF);
				    deck.status["SamplerPad3"].light.onOff(OFF);
				    color=(deck.loopkind==SIMPLE)?OFF:ReloopBeatpad.PadColor.magenta;
				    deck.status["SamplerPad4"].light.onOff(color);
				    break;
				//FX rack Select status
				case FXRACKSELECTMODE :
					for(i=1;i<=4;i++){
						color=(i==deck.CurrentEffectRack)?ReloopBeatpad.PadColor.purple:0;
						deck.status["SamplerPad"+i].light.onOff(color);
					}
					break;
				//normal sampler mode
				default :
					for(i=1;i<=4;i++){
						samplerindex=samplerbaseindex+i;
						isloaded=this.loaded[samplerindex];
						isplaying=engine.getValue("[Sampler"+samplerindex+"]","play");
						color=ReloopBeatpad.PadColor.orange;
						if (isplaying) {color=ReloopBeatpad.PadColor.fushia;}
						deck.status["SamplerPad"+i].light.onOff(isloaded?color:OFF);
					}
					break;
			}
		}
	};
	
	
	this.SetLoaded = function(samplernum,value) { 
		this.loaded[samplernum]=value;
		this.LedUpdateSampler(samplernum);
	};
	
	this.LoadBank = function (banknum) {
		this.bankactive=banknum;		
		ReloopBeatpad.decks.D1.control["sSamplerPad"+banknum].onOff(1);
	};
	
	this.play = function(padnum, value) {
		samplerindex=(this.bankactive-1)*4+padnum;
		isplaying=engine.getValue("[Sampler"+samplerindex+"]","play");
		if(!isplaying) {
			
			engine.setValue("[Sampler" + samplerindex + "]", "cue_gotoandplay", 1);
			engine.setValue("[Sampler" + samplerindex + "]", "beatsync", 1);
		} else {
			engine.setValue("[Sampler" + samplerindex + "]", "stop", 1);
		}
		//this.LedUpdate(samplerindex);
	};
};

ReloopBeatpad.samplers = new ReloopBeatpad.SamplerBank();

ReloopBeatpad.deck = function (deckNum) {
    this.deckNum = deckNum;
    this.group = "[Channel" + deckNum + "]";
    this.Shifted=false;
    this.loadedCheck = function () {
        var yesno = (engine.getValue(this.group, "track_samples") > 0) ? true : false;
        return yesno;
    };
    this.firstbeatpos=0;
    this.beatpos=0;
    this.CurrentEffectRack=1; 
    this.loaded=false;
    this.timers = [];
    this.state = [];
    this.lastFader = [];   // Last value of each channel/cross fader
    this.lastEQs = [[]];
    this.JogScratchStatus=false;
    this.JogSeekStatus=false;
    this.FX_ONStatus=false;
    this.LoopStatus=false;
    this.PadMode=CUEMODE;
    this.scratching = [];
    this.seekingfast = [];
    this.jogbending = [];
    this.filterligthshowstatus=0;
    this.looplightshowstatus=0;
    this.wheelTouchInertiaTimer=0;
    
    this.loopkind=SIMPLE;
    this.loopsize=1;
    this.looppadstatus=0;
    this.InstantFXBtnDown=false;
    
    this.trigger = function() {
		if (this.Shifted) {
			this.status.brake.light.onOff(OFF);
			this.control.censor.light.onOff(OFF);
		}
		engine.trigger(this.group, "track_samples");
		engine.trigger(this.group, "play_indicator");
		engine.trigger(this.group, "beat_active");
		engine.trigger(this.group, "cue_point");
		engine.trigger(this.group, "sync_enabled");
		engine.trigger(this.group, "keylock");
		engine.trigger(this.group, "VuMeter");
		engine.trigger(this.group, "bpm");
		engine.trigger(this.group, "duration");
		
		for(i=1;i<=4;i++){
			engine.trigger(this.group, "hotcue_"+i+"_position");
		}	
        	
		for(i=1;i<=4;i++){
			engine.trigger("[Deere]", "sampler_bank_"+i);
		}
	};
	
    //Last Time the LOAD Btn was pressed before released
    this.LOADlongpress = false;
    this.LOADtimer = 0;

    this.LOADdolongpress = function () {
        this.LOADlongpress = true;
        this.LOADtimer = 0;
    };

    this.LOADdown = function () {
        this.LOADlongpress = false;
        this.LOADtimer = engine.beginTimer(500, "ReloopBeatpad.decks.D" + this.deckNum + ".LOADdolongpress()", true);
    };

    this.LOADup = function () {
        engine.stopTimer(this.LOADtimer);
        if (this.LOADlongpress) {
            engine.setValue(this.group, 'eject', true);
            midi.sendShortMsg(0x8F + this.deckNum, ReloopBeatpad.MIDI.Load, OFF);
        } else {
            engine.setValue(this.group, 'LoadSelectedTrack', true);
            midi.sendShortMsg(0x8F + this.deckNum, ReloopBeatpad.MIDI.Load, ON);
        }
    }; 
     
    //Last Time the SYNC Btn was pressed before released
    this.SYNCcount = 0;
    this.SYNCtimer = 0;
    this.SYNClongpress = false;
    this.SYNClongpresstimer = 0;
    
    this.SYNCdolongpress = function () {
        this.SYNClongpress  = true;
        this.SYNClongpresstimer = 0;
        this.control.sync.light.flashOn(100,ON,100,10);
    };
    
    this.SYNCdown = function () {
        this.SYNClongpress = false;
        this.control.sync.light.onOff(ON);
        this.SYNClongpresstimer = engine.beginTimer(500, "ReloopBeatpad.decks.D" + this.deckNum + ".SYNCdolongpress()", true);
    };

    this.SYNCup = function () {
		engine.stopTimer(this.SYNClongpresstimer);
		this.control.sync.light.onOff(OFF);
        if (this.SYNClongpress) {
            engine.setValue(this.group, 'sync_enabled', true);
        } else {
            this.SYNCcount = this.SYNCcount + 1;
			this.SYNCtimer = engine.beginTimer(200, "ReloopBeatpad.decks.D" + this.deckNum + ".SYNCcountTozero()", true);
			if (this.SYNCcount > 1) {
				//Double press : Sync and play
				engine.setValue(this.group, 'sync_enabled', false);
				engine.setValue(this.group, 'play', true);
				engine.setValue(this.group, 'beatsync', true);
				
			} else { //We pressed sync only once
				engine.setValue(this.group, 'sync_enabled', false);
				engine.setValue(this.group, 'beatsync', true);
				
			}
        }
    };

    this.SYNCcountTozero = function () {
        this.SYNCcount = 0;
        this.SYNCtimer = 0;
    };

    this.iCUTtimer = 0;
    this.iCUTdelay = 20;
    this.iCutfadersave = 0;
    this.iCUTenabled = false;
    this.iCUTon = function () {
		if (!this.iCUTenabled) {
			this.iCUTenabled = true;
			engine.softTakeover("[Master]", "crossfader", false);
		}
    };

    this.iCUT = function (direction) {
        if (this.iCUTenabled) {
            engine.setValue('[Master]', 'crossfader', direction);
            
        }
    };

    this.iCUToff = function () {
		if (this.iCUTenabled) {
			this.iCUTenabled = false;
			engine.setValue('[Master]', 'crossfader', 0);
			engine.softTakeover("[Master]", "crossfader", true);			
		}
    };

    //Pitchbend attribute. Required for pitchbend to be ramping (i.e speeds up/slows down the more the wheel is moved).
    this.bendVal = 0;
    //Timer used to turn of pitch bend.
    this.pitchTimer = 0;
    //The alternative mode to scratching for the jog wheel.
    this.pitchBend = function (forwards) {
        //For some reason the ramping pitchbend option in Mixx menu together with temp_rate_up/down doesn't seem
        //to work for the jog wheel. So this function allows the pitch bend to ramp the faster/more 
        //revolutions of the wheel. 
        if (this.pitchTimer !== 0){ engine.stopTimer(this.pitchTimer); }

        var bendConst = 0.002; //Adjust to suit.
        var nVal = (Math.abs(this.bendVal) + bendConst); //Turn bendVal to absolute value and add.
        nVal = (nVal > 3.0) ? 3.0 : nVal; //If gone over 3, keep at 3.
        this.bendVal = (forwards) ? nVal : -nVal; //Return to positive or minus number.
        engine.setValue(this.group, "jog", this.bendVal);
        this.pitchTimer = engine.beginTimer(20, "ReloopBeatpad.decks.D" + this.deckNum + ".pitchBendOff()", true);
    };
    //Used by function above. Turns pitchbend off.
    this.pitchBendOff = function() {
        this.bendVal = 0;
        this.pitchTimer = 0;
    };
   
    
    
    //Thank you to the authors of the Vestax VCI 400 mapping script
    this.finishWheelTouch = function() {
		this.wheelTouchInertiaTimer = 0;		
		var play = engine.getValue(this.group, "play");
		if (play != 0) {
			// If we are playing, just hand off to the engine.
			this.iCUToff();
			engine.scratchDisable(this.deckNum, true);
		} else {
			// If things are paused, there will be a non-smooth handoff between scratching and jogging.
			// Instead, keep scratch on until the platter is not moving.
			var scratchRate = Math.abs(engine.getValue(this.group, "scratch2"));
			if (scratchRate < 0.01) {
				// The platter is basically stopped, now we can disable scratch and hand off to jogging.
				this.iCUToff(); 
				engine.scratchDisable(this.deckNum, false);
			} else {
				// Check again soon.
				this.wheelTouchInertiaTimer = engine.beginTimer(
						100, "ReloopBeatpad.decks.D" + this.deckNum + ".finishWheelTouch()", true);
			}
		}
	};
	
	this.onWheelTouch = function(control, value) {
		if (control<0x40) {
			this.iCUTon();
		} else {
			this.iCUToff();
		}
		if (this.wheelTouchInertiaTimer != 0) {
			// The wheel was touched again, reset the timer.
			engine.stopTimer(this.wheelTouchInertiaTimer);
			this.wheelTouchInertiaTimer = 0;
		}
		
		if (value == DOWN){
			//Hand on the Jog wheel, scratch activated
			var intervalsPerRev = 1600; //Beatpad jog wheel is 800 intervals per revolution.
			var rpm = 33+1/3; //Adjust to suit.
			var alpha = 1.0/8; //Adjust to suit. 
			var beta = alpha/32; //Adjust to suit.
			/* 
			 * if "control"<0x40, then the DJ is doing a SHIFT+JogWheel (iCut with Dejay from Algorriddim)
			 * if "Jog Scratch" mode is activated on the Beatpad, we have for the "control" value
			 * Left JogWheel --> 0x63, +SHIFT --> 0x23
			 * Right JogWheel --> 0x65, +SHIFT --> 0x25
			 */
			
			engine.scratchEnable(this.deckNum, intervalsPerRev, rpm, alpha, beta);
		} else {
			// The wheel touch sensor can be overly sensitive, so don't release scratch mode right away.
			// Depending on how fast the platter was moving, lengthen the time we'll wait.
			var scratchRate = Math.abs(engine.getValue(this.group, "scratch2"));
			var inertiaTime = Math.pow(1.8, scratchRate) * 50;
			if (inertiaTime < 100) {
				// Just do it now.
				this.finishWheelTouch();
			} else { // If button up
				this.wheelTouchInertiaTimer = engine.beginTimer(
						inertiaTime, "ReloopBeatpad.decks.D" + this.deckNum +  ".finishWheelTouch()", true);
			}
		}
	};
	
	this.onWheelMove = function(control, value) {
		var jogValue = value - 0x40;
		// Note that we always set the jog value even if scratching is active.  This seems
		// to create a better handoff between scratching and not-scratching.
		if (engine.getValue(this.group, "play")) {
			engine.setValue(this.group, "jog", jogValue / 40);
		} else {
			engine.setValue(this.group, "jog", jogValue / 10);
		}
		if(engine.getValue(this.group, "scratch2_enable")){
			    // Register the movement :
				// if "control"<0x40, then the DJ is doing a SHIFT+JogWheel (iCut with Dejay from Algorriddim)
				// if "Jog Scratch" mode is activated on the Beatpad, we have for the "control" value
				// Left JogWheel  --> 0x63, +SHIFT --> 0x23
				// Right JogWheel --> 0x65, +SHIFT --> 0x25
				//According to the Quick start guide :
				//		iCut MODE :
				//		DJAY will automatically cut your track with the
				//		cross fader when holding SHIFT and scratching
				//		with the jog wheel
				//According to Reloop Website : http://www.reloop.com/reloop-beatpad (Explorer tab)
				//		iCut :
				//		this mode simulates a scratch routine. When the jog wheel is turned back
				//		the crossfader closes, when the jog wheel is turned forward the crossfader
				// will open."
				// In Practice : DJAY software is closing/opening the
				//      crossfader quicly without taking into account the direction of the whheel.
				//      Here I am trying to stick with the reloop explanation : it is the "as it is supposed to be done"
				(control<0x40) ? this.iCUTon() : this.iCUToff();
				
				
				var direction = Math.sign(jogValue); //Get Jog wheel direction
				if (direction>0) { direction = 0; }// Backward=-1 (close), forward =0 (open)
				// Left Deck ? direction = 0 (open : crossfader to zéro) or 1 (close : crossfader to the right) 
				// Right Deck ? direction = 0 (open : crossfader to zéro) or -1 (close : crossfader to the left) 
				if (this.deckNum==1) {direction=-direction;} //else do nothing
				this.iCUT(direction);
				engine.scratchTick(this.deckNum, jogValue);
		}

	};

    //Controls for the deck--buttons, sliders, etc--are associated with the deck using this array.
    this.control = [];
    this.status = [];
    this.RGBShow = undefined;
    
    this.SelectEffectRack = function(newindex) {
        this.CurrentEffectRack=newindex;
		this.RGBShow.effectsblink(newindex);
		ReloopBeatpad.samplers.LedsUpdate();
	};
	
	this.ToggleLoopKind = function() {
		// toggle loop mode into SIMPLE/ROLL
		var isLoopActive = engine.getValue(this.group, "loop_enabled");
		this.loopkind=3-this.loopkind; 
		if(isLoopActive) {
			if (this.loopkind==SIMPLE) {			
				engine.setValue(this.group, "beatloop_" + this.loopsize.toString() + "_activate", 1);
			} else {
				engine.setValue(this.group, "beatlooproll_" + this.loopsize.toString() + "_activate", 1);
			}
		}
		ReloopBeatpad.samplers.LedsUpdate();
	};
};

//Control class for control objects, e.g. play button. Objects for the two jog wheels are not created this way, instead they are
//represented directly by the deck objects. This was just an easier way to do it, and there will only ever be one jog wheel
//per deck anyway.
ReloopBeatpad.control = function (key, control, midino, group) {
    this.key = key; 
    this.control = control;
    this.midino = midino;
    this.group = group;
    this.onOff = function (value) {
        engine.setValue(this.group, this.key, value);
    };
    this.checkOn = function () {
        var checkOn = engine.getValue(this.group, this.key);
        return checkOn;
    };
};

//Has on/off and flashing modes for all button lights. Light objects are only created for buttons that can
//actually illuminate.
ReloopBeatpad.light = function (group, control, midino, deckID, controlID) {
	this.control = control;
    this.midino = midino;
    this.objStr = "ReloopBeatpad.decks." + deckID + ".control." + controlID + ".light";
    this.lit = 0;
    this.flashTimer = 0;
    this.flashTimer2 = 0;
    this.flashOnceTimer = 0;
    this.flashDuration = 0;
    this.flashOnceDuration = 0;
    this.onOff = function (value) {
		if (this.flashTimer !== 0)
            {       
            engine.stopTimer(this.flashTimer);
            this.flashTimer = 0;
            this.flashDuration = 0;
            }
        if (this.flashTimer2 !== 0)
            {       
            engine.stopTimer(this.flashTimer2);
            this.flashTimer2 = 0;
            this.flashDuration = 0;
            }
        if (this.flashOnceTimer !== 0)
            {       
            engine.stopTimer(this.flashOnceTimer);
            this.flashOnceTimer = 0;
            this.flashOnceDuration= 0 ;
            }
        midi.sendShortMsg(this.control, this.midino, value);
        pauseScript(scriptpause);
        this.lit = value;
    };
    this.flashOnceOn = function (num_ms,value,relight, valueoff) {
		if (this.flashOnceTimer !== 0)
            {       
            engine.stopTimer(this.flashOnceTimer);
            this.flashOnceTimer = 0;
            this.flashOnceDuration= 0 ;
            }
        midi.sendShortMsg(this.control, this.midino, value);
        pauseScript(scriptpause);
        this.flashOnceDuration= num_ms;
        this.flashOnceTimer = engine.beginTimer(num_ms-scriptpause, this.objStr + ".flashOnceOff(" + relight +","+valueoff+")", true);
    };
    this.flashOnceOff = function (relight,valueoff) {
		engine.stopTimer(this.flashOnceTimer);
        this.flashOnceTimer = 0;
        this.flashOnceDuration= 0 ;
        if (relight)
            {
            midi.sendShortMsg(this.control, this.midino, this.lit);
            pauseScript(scriptpause);
            }
        else
            {
            midi.sendShortMsg(this.control, this.midino, valueoff);
            pauseScript(scriptpause);
            this.lit=OFF;
            }
    };
    
    this.flashOff = function (relight) {
        if (relight)
            {
            this.onOff(this.lit);
            pauseScript(scriptpause);
            }
        else
            {
            this.onOff(OFF);
            pauseScript(scriptpause);
            }
    };
    this.flashOn = function (num_ms_on, value, num_ms_off, flashCount, relight, valueoff) {
        this.flashOff();
        this.flashDuration=num_ms_on;
        this.flashOnceOn(num_ms_on,value); //This is because the timer take num_ms_on milisecs before first flash.
        if (flashCount!=1) {
			this.flashTimer = engine.beginTimer((num_ms_on+num_ms_off), this.objStr + ".flashOnceOn("+num_ms_on+","+value+",false,"+valueoff+")");
		}
        if (flashCount>1){
            this.flashTimer2 = engine.beginTimer(flashCount*(num_ms_on+num_ms_off) -num_ms_off, this.objStr + ".flashOff(" + relight + ")", true);
        }        
    };
    this.get_flashDuration = function () {
		return this.flashDuration;
	};
};

ReloopBeatpad.status = function (control, midino) {
    this.control = control;
    this.midino = midino;
    this.activated = false;
    this.onOff = function (value) {
        this.activated = value;
        if (typeof this.light != "undefined") {
			this.light.onOff(value);
		}
	};
    this.checkOn = function () {
        return this.activated;
    };
};

ReloopBeatpad.statuslight = function (control, midino, deckID, controlID) {
	this.control = control;
    this.midino = midino;
    this.objStr = "ReloopBeatpad.decks." + deckID + ".status." + controlID + ".light";
    this.lit = 0;
    this.flashTimer = 0;
    this.flashTimer2 = 0;
    this.flashOnceTimer = 0;
    this.flashDuration = 0;
    this.onOff = function (value) {
		if (this.flashTimer !== 0)
            {       
            engine.stopTimer(this.flashTimer);
            this.flashTimer = 0;
            this.flashDuration = 0;
            }
        if (this.flashTimer2 !== 0)
            {       
            engine.stopTimer(this.flashTimer2);
            this.flashTimer2 = 0;
            this.flashDuration = 0;
            }
        if (this.flashOnceTimer !== 0)
            {       
            engine.stopTimer(this.flashOnceTimer);
            this.flashOnceTimer = 0;
            this.flashOnceDuration= 0 ;
            }
        midi.sendShortMsg(this.control, this.midino, value);
        pauseScript(scriptpause);
        this.lit = value;
    };
    this.flashOnceOn = function (num_ms,value,relight, valueoff) {
		if (this.flashOnceTimer !== 0)
            {       
            engine.stopTimer(this.flashOnceTimer);
            this.flashOnceTimer = 0;
            this.flashOnceDuration= 0 ;
            }
        midi.sendShortMsg(this.control, this.midino, value);
        pauseScript(scriptpause);
        this.flashOnceDuration= num_ms;
        this.flashOnceTimer = engine.beginTimer(num_ms-scriptpause, this.objStr + ".flashOnceOff(" + relight +","+valueoff+")", true);
    };
    this.flashOnceOff = function (relight,valueoff) {
		engine.stopTimer(this.flashOnceTimer);
        this.flashOnceTimer = 0;
        this.flashOnceDuration= 0 ;
        if (relight)
            {
            midi.sendShortMsg(this.control, this.midino, this.lit);
            pauseScript(scriptpause);
            }
        else
            {
            midi.sendShortMsg(this.control, this.midino, valueoff);
            pauseScript(scriptpause);
            this.lit=OFF;
            }
    };
    
    this.flashOff = function (relight) {
        if (relight)
            {
            this.onOff(this.lit);
            }
        else
            {
            this.onOff(OFF);
            }
    };
    this.flashOn = function (num_ms_on, value, num_ms_off, flashCount, relight, valueoff) {
        this.flashOff();
        this.flashDuration=num_ms_on;
        this.flashOnceOn(num_ms_on,value); //This is because the timer take num_ms_on milisecs before first flash.
        if (flashCount!=1) {
			this.flashTimer = engine.beginTimer((num_ms_on+num_ms_off), this.objStr + ".flashOnceOn("+num_ms_on+","+value+",false,"+valueoff+")");
		}
        if (flashCount>1) {
            this.flashTimer2=engine.beginTimer(flashCount*(num_ms_on+num_ms_off) -num_ms_off, this.objStr + ".flashOff(" + relight + ")", true);
        }        
    };
    this.get_flashDuration = function () {
		return this.flashDuration;
	};
};

ReloopBeatpad.rgblight = function (control, deckID) {
	this.control = control;
    this.midino = 0x69;
    this.objStr = "ReloopBeatpad.decks." + deckID + ".RGBShow";
    this.layers = {"show0" : {activated : true , colors:[OFF, OFF, OFF, OFF]},  //OFFF state
		           "show1" : {activated : false, colors:[null, null, null, null]},
		           "show2" : {activated : false, colors:[null, null, null, null]},
		           "show3" : {activated : false, colors:[null, null, null, null]},
		           "show4" : {activated : false, colors:[null, null, null, null]},
		           "show5" : {activated : true,  colors:[null, null, null, null]},       //reserved for blinking  (blink Off values)
		           "show6" : {activated : false, colors:[null, null, null, null]} }; //reserved for blinking (blink On values)
    this.setshow = function (showname, color1, color2, color3, color4) {
		if ((showname!="show0")) {
		    if (color1===undefined) {this.layers[showname].colors[0]=null;}
		    else {this.layers[showname].colors[0]=color1;}
			if (color2===undefined)
			{
				this.layers[showname].colors[1]=this.layers[showname].colors[0];
				this.layers[showname].colors[2]=this.layers[showname].colors[0];
				this.layers[showname].colors[3]=this.layers[showname].colors[0];
			} else {
				this.layers[showname].colors[1]=color2;
				this.layers[showname].colors[2]=color3;
				this.layers[showname].colors[3]=color4;
			}
		}
	};
		
    this.lit = [OFF, OFF, OFF, OFF];
    this.flashTimer= 0;
    this.flashOnceTimer = 0;
    this.flashOnceTimer2 = 0;
    this.flashDuration = 0;
    this.updatecontroller = function () {
		//null="transparent"
		var tosend=[null, null, null, null];
		var showname;
		for(k=0;k<=3;k++){
			for (j=6; (tosend[k]===null) ; j--) {
				showname="show"+j;
				if (this.layers[showname].activated) {
					tosend[k]=this.layers[showname].colors[k];
				}
			}
		}
		for(i=0;i<=3;i++) {
			if (tosend[i]!==this.lit[i]) { 
				midi.sendShortMsg(this.control, this.midino+i, tosend[i]);
				pauseScript(scriptpause);
			}
		 }
		 this.lit=tosend.slice(0);
	};
	
	this.activateshow = function(showname,activate) {
		this.layers[showname].activated=activate;
		if(showname!="show5") {
			this.updatecontroller();
		}
	};
	
    this.onOff = function (showname,value) {
			if (this.flashTimer !== 0)
				{       
				engine.stopTimer(this.flashTimer);
				engine.stopTimer(this.flashTimer2);
				this.flashTimer = 0;
				this.flashTimer2 = 0;
				this.flashDuration = 0;
				this.layers["show5"].activate=false;				
				this.layers["show6"].activate=false;
				}
			if (this.flashOnceTimer !== 0)
				{       
				engine.stopTimer(this.flashOnceTimer);
				this.flashOnceTimer = 0;
				this.layers["show5"].activate=false;
				this.layers["show6"].activate=false;
				}
			
        this.activateshow(showname, (value)?true:false);
    };
    this.flashOnceOn = function (num_ms,relight) {
		if (this.flashOnceTimer !== 0)
            {       
            engine.stopTimer(this.flashOnceTimer);
            this.flashOnceTimer = 0;
            this.flashOnceDuration= 0 ;
            }        
        this.layers["show5"].activate=true;
		this.activateshow("show6", true);
        this.flashOnceDuration= num_ms;
        this.flashOnceTimer = engine.beginTimer(num_ms-10, this.objStr + ".flashOnceOff(" + relight +")", true);
    };
    this.flashOnceOff = function (relight) {
		engine.stopTimer(this.flashOnceTimer);
        this.flashOnceTimer = 0;
        this.flashOnceDuration= 0 ;
        if (relight) { this.layers["show5"].activate=true; } 
        else {this.layers["show5"].activate=false; } 
        this.activateshow("show6", false);
    };
    
    this.flashOff = function () {
		engine.stopTimer(this.flashTimer);
        this.flashTimer = 0;
        engine.stopTimer(this.flashTimer2);
        this.flashTimer2 = 0;
        this.flashOnceDuration= 0;
        this.layers["show5"].activate=false; 
        this.activateshow("show6", false);
    };
    this.flashOn = function (num_ms_on, value, num_ms_off, flashCount) {
        this.flashOff();
        this.layers["show5"].activate=true;
        this.flashDuration=num_ms_on;
        this.flashOnceOn(num_ms_on,true); 
        if (flashCount!=1) {
			this.flashTimer = engine.beginTimer((num_ms_on+num_ms_off), this.objStr + ".flashOnceOn("+num_ms_on+",true)");
		}
        if (flashCount>1){
            this.flashTimer2 = engine.beginTimer(flashCount*(num_ms_on+num_ms_off) -num_ms_off, this.objStr + ".flashOff()", true);
        }        
    };
    this.get_flashDuration = function () {
		return this.flashDuration;
	};
	
	//RGB light showtime
	
	this.effects = function (value) {
		var RGBColor;
		switch (value) {
			case 1 : RGBColor=ReloopBeatpad.RGB.green; break;
			case 2 : RGBColor=ReloopBeatpad.RGB.yellow; break;
			case 3 : RGBColor=ReloopBeatpad.RGB.cyan; break;
			case 4 : RGBColor=ReloopBeatpad.RGB.magenta; break;
			default : RGBColor=OFF;
		}
		this.setshow("show1",RGBColor);
		this.activateshow("show1",(RGBColor!=0));
	};
	
	this.effectsblink = function (value) {
		var RGBColor;
		switch (value) {
			case 1 : RGBColor=ReloopBeatpad.RGB.green; break;
			case 2 : RGBColor=ReloopBeatpad.RGB.yellow; break;
			case 3 : RGBColor=ReloopBeatpad.RGB.cyan; break;
			case 4 : RGBColor=ReloopBeatpad.RGB.magenta; break;
			default : RGBColor=OFF;
		}
		this.setshow("show6",RGBColor);
		this.flashOn(200,RGBColor,200,3,true);
		this.setshow("show1",RGBColor);
	};
	
	this.filter = function (value) {
		var RGBColor1=(value>0)?ReloopBeatpad.RGB.white:null;
		var RGBColor2=(value>0)?ReloopBeatpad.RGB.white:null;
		var RGBColor3=(value<0)?ReloopBeatpad.RGB.white:null;
		var RGBColor4=(value<0)?ReloopBeatpad.RGB.white:null;
		var activate=(value!=0);
		this.setshow("show2",RGBColor1,RGBColor2,RGBColor3,RGBColor4);
		this.activateshow("show2",activate);
	};
	
	this.loops = function (value) {
		var RGBColor=(value!=0)?ReloopBeatpad.RGB.magenta:null;
		var activate=(value!=0);
		this.setshow("show3",RGBColor);
		this.activateshow("show3",activate);
		ReloopBeatpad.decks[deckID].status.RimBlue.light.onOff((activate)?0x7F:0);
	};	
	
	this.notloaded = function (value) {
		var RGBColor=(value)?ReloopBeatpad.RGB.red:null;
		this.setshow("show4",RGBColor);
		this.activateshow("show4",value);
	};	
};

//Constructor for creating control objects
ReloopBeatpad.addControl = function (arrID, ID, controlObj, addLight) {
    var arrAdd = this[arrID];
    if (addLight) {
        //If the button can illuminate, a light object is created for it (see above).
        controlObj.light = new ReloopBeatpad.light(this.group, controlObj.control, controlObj.midino, "D" + this.deckNum,ID);
    }
    arrAdd[ID] = controlObj;
};

ReloopBeatpad.addStatus = function (arrID, ID, statusObj, addLight) {
    var arrAdd = this[arrID];
    if (addLight) {
        //If the button can illuminate, a light object is created for it (see above
        statusObj.light = new ReloopBeatpad.statuslight(statusObj.control, statusObj.midino, "D" + this.deckNum,ID);
     }
    arrAdd[ID] = statusObj;
};

ReloopBeatpad.addRGB = function (RGBObj) {
    this.RGBShow = RGBObj;
};



// ----------   Other global variables    ----------
ReloopBeatpad.initobjects = function() {
	//This creates 'addControl' and 'addStatus' constructor functions to the deck class, using the above functions as a templates.
	ReloopBeatpad.deck.prototype.addStatus = ReloopBeatpad.addStatus;
	ReloopBeatpad.deck.prototype.addControl = ReloopBeatpad.addControl;
	ReloopBeatpad.deck.prototype.addRGB = ReloopBeatpad.addRGB;
	//Creating the two deck objects.
	with (ReloopBeatpad){
		
	addControl("decks", "D1", new deck("1"));
	addControl("decks", "D2", new deck("2"));
	

	//All controls below associated with left or right deck.
	
	decks.D1.addControl("control", "load", new control("LoadSelectedTrack", LBtn, MIDI.Load, "[Channel1]"), true); 
	decks.D2.addControl("control", "load", new control("LoadSelectedTrack", RBtn, MIDI.Load, "[Channel2]"), true);

	decks.D1.addControl("control", "sync", new control("beatsync", LBtn, MIDI.Sync, "[Channel1]"), true);
	decks.D2.addControl("control", "sync", new control("beatsync", RBtn, MIDI.Sync, "[Channel2]"), true);
	decks.D1.addControl("control", "start", new control("start", LBtn, MIDI.Sync+SHIFT, "[Channel1]"), true);
	decks.D2.addControl("control", "start", new control("start", RBtn, MIDI.Sync+SHIFT, "[Channel2]"), true);

	decks.D1.addControl("control", "pfl", new control("pfl", LBtn, MIDI.pfl, "[Channel1]"), true);
	decks.D2.addControl("control", "pfl", new control("pfl", RBtn, MIDI.pfl, "[Channel2]"), true);
	decks.D1.addControl("control", "slip", new control("slip_enabled", LBtn, MIDI.pfl+SHIFT, "[Channel1]"), true);
	decks.D2.addControl("control", "slip", new control("slip_enabled", RBtn, MIDI.pfl+SHIFT, "[Channel2]"), true);

	decks.D1.addControl("control", "Set", new control("cue_default", LBtn, MIDI.Set, "[Channel1]"), true);
	decks.D2.addControl("control", "Set", new control("cue_default", RBtn, MIDI.Set, "[Channel2]"), true);
	decks.D1.addControl("control", "keylock", new control("keylock", LBtn, MIDI.Set+SHIFT, "[Channel1]"), true);
	decks.D2.addControl("control", "keylock", new control("keylock", RBtn, MIDI.Set+SHIFT, "[Channel2]"), true);
	
	decks.D1.addControl("control", "jump", new control("cue_gotoandplay", LBtn, MIDI.Jump, "[Channel1]"), true);
	decks.D2.addControl("control", "jump", new control("cue_gotoandplay", RBtn, MIDI.Jump, "[Channel2]"), true);
	
	decks.D1.addControl("control", "play", new control("play", LBtn, MIDI.Play, "[Channel1]"), true);
	decks.D2.addControl("control", "play", new control("play", RBtn, MIDI.Play, "[Channel2]"), true);
	decks.D1.addControl("control", "LoadAndPlay", new control("LoadSelectedTrackAndPlay", LBtn, MIDI.Play, "[Channel1]"), true);
	decks.D2.addControl("control", "LoadAndPlay", new control("LoadSelectedTrackAndPlay", RBtn, MIDI.Play, "[Channel2]"), true);

	decks.D1.addControl("control", "censor", new control("reverseroll", LBtn, MIDI.Play+SHIFT, "[Channel1]"), true);
	decks.D2.addControl("control", "censor", new control("reverseroll", RBtn, MIDI.Play+SHIFT, "[Channel2]"), true);
    
	decks.D1.addControl("control", "bendMinus", new control("rate_temp_down", LBtn, MIDI.BendMinus, "[Channel1]"), true);
	decks.D2.addControl("control", "bendMinus", new control("rate_temp_down", RBtn, MIDI.BendMinus, "[Channel2]"), true);
    decks.D1.addControl("control", "beatjumpMinus", new control("beatjump_1_backward", LBtn, MIDI.BendMinus+SHIFT, "[Channel1]"), true);
	decks.D2.addControl("control", "beatjumpMinus", new control("beatjump_1_backward", RBtn, MIDI.BendMinus+SHIFT, "[Channel2]"), true);

	decks.D1.addControl("control", "bendPlus", new control("rate_temp_up", LBtn, MIDI.BendPlus, "[Channel1]"), true);
	decks.D2.addControl("control", "bendPlus", new control("rate_temp_up", RBtn, MIDI.BendPlus, "[Channel2]"), true);
	decks.D1.addControl("control", "beatjumpPlus", new control("beatjump_1_forward", LBtn, MIDI.BendPlus+SHIFT, "[Channel1]"), true);
	decks.D2.addControl("control", "beatjumpPlus", new control("beatjump_1_forward", RBtn, MIDI.BendPlus+SHIFT, "[Channel2]"), true);
	
	for(i=1;i<=4;i++) {
		decks.D1.addControl("control", "FXPad"+i, new control("group_[Channel1]_enable",LBtn,MIDI.FXPad+i-1,"[EffectRack1_EffectUnit"+i+"]"),HardwareLight);
		decks.D2.addControl("control", "FXPad"+i, new control("group_[Channel2]_enable",RBtn,MIDI.FXPad+i-1,"[EffectRack1_EffectUnit"+i+"]"),HardwareLight);
    }
    
    for(i=1;i<=4;i++) {
		decks.D1.addControl("control", "LoopPad"+i, new control("beatlooproll_"+Math.pow(2, i-4)+"_activate",LBtn,MIDI.LoopPad+i-1,"[Channel1]"),HardwareLight);
		decks.D2.addControl("control", "LoopPad"+i, new control("beatlooproll_"+Math.pow(2, i-4)+"_activate",RBtn,MIDI.LoopPad+i-1,"[Channel2]"),HardwareLight);
    }
    for(i=1;i<=4;i++) {
		decks.D1.addControl("control", "sLoopPad"+i, new control("beatlooproll_"+Math.pow(2, i)+"_activate",LBtn,MIDI.LoopPad+i-1+SHIFT,"[Channel1]"),HardwareLight);
		decks.D2.addControl("control", "sLoopPad"+i, new control("beatlooproll_"+Math.pow(2, i)+"_activate",RBtn,MIDI.LoopPad+i-1+SHIFT,"[Channel2]"),HardwareLight);
    }
    
    for(i=1;i<=4;i++) {
		decks.D1.addControl("control", "sSamplerPad"+i, new control("sampler_bank_"+i,LBtn,MIDI.LoopPad+i-1+SHIFT,"[Deere]"),false);
		decks.D2.addControl("control", "sSamplerPad"+i, new control("sampler_bank_"+i,RBtn,MIDI.LoopPad+i-1+SHIFT,"[Deere]"),false);
    }
    //Status Buttons and visualizations
	
    decks.D1.addStatus("status", "brake", new status(LBtn,MIDI.Jump+SHIFT),true);
	decks.D2.addStatus("status", "brake", new status(RBtn,MIDI.Jump+SHIFT),true);
    
    decks.D1.addStatus("status", "RimRed", new status(LBtn,MIDI.RIM_Red),true);
	decks.D2.addStatus("status", "RimRed", new status(RBtn,MIDI.RIM_Red),true);
	decks.D1.addStatus("status", "RimBlue", new status(LBtn,MIDI.RIM_Blue),true);
	decks.D2.addStatus("status", "RimBlue", new status(RBtn,MIDI.RIM_Blue),true);
	
	decks.D1.addRGB(new rgblight(LBtn,"D1"));
	decks.D2.addRGB(new rgblight(RBtn,"D2"));
	
	decks.D1.addStatus("status", "iScratch", new status(LBtn,MIDI.iScratch), HardwareLight);
	decks.D2.addStatus("status", "iScratch", new status(RBtn,MIDI.iScratch), HardwareLight);
	decks.D1.addStatus("status", "Vinyl", new status(LBtn,MIDI.Vinyl), HardwareLight);
	decks.D2.addStatus("status", "Vinyl", new status(RBtn,MIDI.Vinyl), HardwareLight);
	
	
	decks.D1.addStatus("status", "Loop", new status(LBtn,MIDI.Loop), true);
	decks.D2.addStatus("status", "Loop", new status(RBtn,MIDI.Loop), true);
	decks.D1.addStatus("status", "FX_ON", new status(LBtn,MIDI.FX_ON), true);
	decks.D2.addStatus("status", "FX_ON", new status(RBtn,MIDI.FX_ON), true);
	
	decks.D1.addStatus("status", "Cue", new status(LBtn,MIDI.Cue), HardwareLight);
	decks.D2.addStatus("status", "Cue", new status(RBtn,MIDI.Cue), HardwareLight);
	decks.D1.addStatus("status", "Loop_Bounce", new status(LBtn,MIDI.Loop_Bounce), HardwareLight);
	decks.D2.addStatus("status", "Loop_Bounce", new status(RBtn,MIDI.Loop_Bounce), HardwareLight);
	decks.D1.addStatus("status", "InstantFX", new status(LBtn,MIDI.InstantFX), HardwareLight);
	decks.D2.addStatus("status", "InstantFX", new status(RBtn,MIDI.InstantFX), HardwareLight);
	decks.D1.addStatus("status", "spinback", new status(LBtn,MIDI.InstantFX+SHIFT), HardwareLight);
	decks.D2.addStatus("status", "spinback", new status(RBtn,MIDI.InstantFX+SHIFT), HardwareLight);
	decks.D1.addStatus("status", "Sampler", new status(LBtn,MIDI.Sampler));
	decks.D2.addStatus("status", "Sampler", new status(RBtn,MIDI.Sampler));
	for(i=1;i<=4;i++) {
		decks.D1.addStatus("status", "CuePad"+i, new status(LBtn,MIDI.CuePad+i-1),true);
		decks.D2.addStatus("status", "CuePad"+i, new status(RBtn,MIDI.CuePad+i-1),true);
		decks.D1.addStatus("status", "sCuePad"+i, new status(LBtn,MIDI.CuePad+i-1+SHIFT),true);
		decks.D2.addStatus("status", "sCuePad"+i, new status(RBtn,MIDI.CuePad+i-1+SHIFT),true);
    }
    
    
    for(i=1;i<=4;i++) {
		decks.D1.addStatus("status", "SamplerPad"+i, new status(LBtn,MIDI.SamplerPad+i-1),true);
		decks.D2.addStatus("status", "SamplerPad"+i, new status(RBtn,MIDI.SamplerPad+i-1),true);
		decks.D1.addStatus("status", "sSamplerPad"+i, new status(LBtn,MIDI.SamplerPad+i-1+SHIFT),true);
		decks.D2.addStatus("status", "sSamplerPad"+i, new status(RBtn,MIDI.SamplerPad+i-1+SHIFT),true);
    }
	decks.D1.addStatus("status", "VUMeter", new status(LBtn,MIDI.VUMeter),true);
	decks.D2.addStatus("status", "VUMeter", new status(RBtn,MIDI.VUMeter),true);

	addStatus("status", "recording", new status(MBtn,MIDI.rec),true);
}

};


ReloopBeatpad.init = function (id,debug) {
//Connect button lights to equivalent controls. The track_samples control
//is being used to tell if a track has successfully loaded (whereupon lights
//flash twice. Best way I could think of in the absence of a proper control
//for checking whether a track is loaded.
    ReloopBeatpad.id = id;
    
    
    ReloopBeatpad.TurnLEDsOff();
    ReloopBeatpad.initobjects();
    // Enable 2 decks in v1.11.x and above
    engine.setValue("[Master]", "num_decks", 2);

    // Set soft-takeover for all Sampler volumes
    for(i=engine.getValue("[Master]", "num_samplers"); i >= 1; i--) {
        engine.softTakeover("[Sampler" + i + "]", "pregain", true);
    }
    // Set soft-takeover for all applicable Deck controls
    for(i=engine.getValue("[Master]", "num_decks"); i >= 1; i--) {
        engine.softTakeover("[Channel" + i + "]", "volume", true);
        engine.softTakeover("[Channel" + i + "]", "filterHigh", true);
        engine.softTakeover("[Channel" + i + "]", "filterMid", true);
        engine.softTakeover("[Channel" + i + "]", "filterLow", true);
    }

    engine.softTakeover("[Master]", "crossfader", true);
    
    for (i=1;i<=4;i++) {
		engine.softTakeover("[EffectRack1_EffectUnit"+i+"]", "super1",true);
	}
    
    
    engine.connectControl("[Channel1]", "play_indicator", "ReloopBeatpad.OnPlayIndicatorChange");
    engine.connectControl("[Channel2]", "play_indicator", "ReloopBeatpad.OnPlayIndicatorChange");
    engine.connectControl("[Channel1]", "beat_active", "ReloopBeatpad.OnBeatActive");
    engine.connectControl("[Channel2]", "beat_active", "ReloopBeatpad.OnBeatActive");
    engine.connectControl("[Channel1]", "cue_point", "ReloopBeatpad.OnCuePointChange");
    engine.connectControl("[Channel2]", "cue_point", "ReloopBeatpad.OnCuePointChange");
    engine.connectControl("[Channel1]", "sync_enabled", "ReloopBeatpad.OnSyncEnabledChange");
    engine.connectControl("[Channel2]", "sync_enabled", "ReloopBeatpad.OnSyncEnabledChange");
    engine.connectControl("[Channel1]", "track_samples", "ReloopBeatpad.OnTrackLoaded");
    engine.connectControl("[Channel2]", "track_samples", "ReloopBeatpad.OnTrackLoaded");
    engine.connectControl("[Channel1]", "keylock", "ReloopBeatpad.OnKeylock");
    engine.connectControl("[Channel2]", "keylock", "ReloopBeatpad.OnKeylock");
    engine.connectControl("[Channel1]", "VuMeter", "ReloopBeatpad.OnVuMeterChange");
    engine.connectControl("[Channel2]", "VuMeter", "ReloopBeatpad.OnVuMeterChange");
    engine.connectControl("[Channel1]", "playposition", "ReloopBeatpad.OnPlaypositionChange");
    engine.connectControl("[Channel2]", "playposition", "ReloopBeatpad.OnPlaypositionChange");
    engine.connectControl("[Channel1]", "bpm", "ReloopBeatpad.OnBPMChange");
    engine.connectControl("[Channel2]", "bpm", "ReloopBeatpad.OnBPMChange");
    engine.connectControl("[Channel1]", "duration", "ReloopBeatpad.OnDurationChange");
    engine.connectControl("[Channel2]", "duration", "ReloopBeatpad.OnDurationChange");
    
    engine.connectControl("[EffectRack1_EffectUnit1_Effect1]", "loaded", function (value, group, control) {ReloopBeatpad.OnEffectLoaded(value, group, control,1);});
    engine.connectControl("[EffectRack1_EffectUnit2_Effect1]", "loaded", function (value, group, control) {ReloopBeatpad.OnEffectLoaded(value, group, control,2);})
    engine.connectControl("[EffectRack1_EffectUnit3_Effect1]", "loaded", function (value, group, control) {ReloopBeatpad.OnEffectLoaded(value, group, control,3);})
    engine.connectControl("[EffectRack1_EffectUnit4_Effect1]", "loaded", function (value, group, control) {ReloopBeatpad.OnEffectLoaded(value, group, control,4);})
    
    engine.connectControl("[Recording]", "status", "ReloopBeatpad.OnRecordingStatusChange");
    
    engine.connectControl("[Channel1]", "hotcue_1_position", function (value, group, control) {ReloopBeatpad.OnHotcuePositionChange(value, group, control,1);});
    engine.connectControl("[Channel2]", "hotcue_1_position", function (value, group, control) {ReloopBeatpad.OnHotcuePositionChange(value, group, control,1);});
    engine.connectControl("[Channel1]", "hotcue_2_position", function (value, group, control) {ReloopBeatpad.OnHotcuePositionChange(value, group, control,2);});
    engine.connectControl("[Channel2]", "hotcue_2_position", function (value, group, control) {ReloopBeatpad.OnHotcuePositionChange(value, group, control,2);});
    engine.connectControl("[Channel1]", "hotcue_3_position", function (value, group, control) {ReloopBeatpad.OnHotcuePositionChange(value, group, control,3);});
    engine.connectControl("[Channel2]", "hotcue_3_position", function (value, group, control) {ReloopBeatpad.OnHotcuePositionChange(value, group, control,3);});
    engine.connectControl("[Channel1]", "hotcue_4_position", function (value, group, control) {ReloopBeatpad.OnHotcuePositionChange(value, group, control,4);});
    engine.connectControl("[Channel2]", "hotcue_4_position", function (value, group, control) {ReloopBeatpad.OnHotcuePositionChange(value, group, control,4);});
   
    for(i=1;i<=16;i++) {
		engine.connectControl("[Sampler"+i+"]", "track_samples", "ReloopBeatpad.OnSampleLoaded");
		engine.connectControl("[Sampler"+i+"]", "play", "ReloopBeatpad.OnSamplePlayStop");
	}
	
	engine.connectControl("[Deere]", "sampler_bank_1", function (value, group, control) {ReloopBeatpad.OnBankLoaded(value, group, control,1);});
	engine.connectControl("[Deere]", "sampler_bank_2", function (value, group, control) {ReloopBeatpad.OnBankLoaded(value, group, control,2);});
	engine.connectControl("[Deere]", "sampler_bank_3", function (value, group, control) {ReloopBeatpad.OnBankLoaded(value, group, control,3);});
	engine.connectControl("[Deere]", "sampler_bank_4", function (value, group, control) {ReloopBeatpad.OnBankLoaded(value, group, control,4);});
	// Conveniences
      engine.setValue("[Channel1]","keylock",true);
      engine.setValue("[Channel1]","quantize",true);   
      engine.setValue("[Channel2]","keylock",true);
      engine.setValue("[Channel2]","quantize",true);   

    //After midi controller receive this Outbound Message request SysEx Message, 
    //midi controller will send the status of every item on the
    //control surface. (Mixxx will be initialized with current values)
    print("polling Beatpad , sysex msg :"+ControllerStatusSysex);
    
    midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);
    
    
    //check if there is already something loaded on each deck (when script reinitialize)
    engine.trigger("[Channel1]", "track_samples");
	engine.trigger("[Channel2]", "track_samples");
	engine.trigger("[Channel1]", "play_indicator");
    engine.trigger("[Channel2]", "play_indicator");
    engine.trigger("[Channel1]", "beat_active");
    engine.trigger("[Channel2]", "beat_active");
    engine.trigger("[Channel1]", "cue_point");
    engine.trigger("[Channel2]", "cue_point");
    engine.trigger("[Channel1]", "sync_enabled");
    engine.trigger("[Channel2]", "sync_enabled");
    engine.trigger("[Channel1]", "keylock");
    engine.trigger("[Channel2]", "keylock");
    engine.trigger("[Channel1]", "VuMeter");
    engine.trigger("[Channel2]", "VuMeter");
    engine.trigger("[Channel1]", "playposition");
    engine.trigger("[Channel2]", "playposition");
    engine.trigger("[Channel1]", "bpm");
    engine.trigger("[Channel2]", "bpm");
    engine.trigger("[Channel1]", "duration");
    engine.trigger("[Channel2]", "duration");
    
    for(i=1;i<=4;i++){
		engine.trigger("[Channel1]", "hotcue_"+i+"_position");
		engine.trigger("[Channel2]", "hotcue_"+i+"_position");
	}
	
	for(i=1;i<=16;i++) {
		engine.trigger("[Sampler"+i+"]", "track_samples");
	}
	
	for(i=1;i<=4;i++){
		engine.trigger("[Deere]", "sampler_bank_"+i);
	}
	
	print ("Reloop Beatpad: "+id+" initialized.");
};


// The Jog Scratch touch that enables/disables scratching
ReloopBeatpad.WheelScratchTouch = function (channel, control, value, status, group) {
    var decknum = script.deckFromGroup(group);
    var deck = ReloopBeatpad.decks["D" + decknum]; // work out which deck we are using 
	deck.onWheelTouch(control, value);    
};


// The Jog Scratch that actually controls the scratching
ReloopBeatpad.WheelScratch = function (channel, control, value, status, group) {
    var decknum = group.substring(8,9)>>0;  //way faster than parseInt(group.substring(8,9))
    var deck = ReloopBeatpad.decks["D" + decknum]; 
    deck.onWheelMove(control, value);
};

ReloopBeatpad.WheelSeekTouch = function (channel, control, value, status, group) {
    var decknum = group.substring(8,9)>>0;
    var deck = ReloopBeatpad.decks["D" + decknum];  
    if (value == DOWN) {
        //Hand on the Jog wheel, fast seek activated
        deck.seekingfast=true;
    }
    else {    
        //Hand off the Jog wheel, desactivate fast seek
        deck.seekingfast=false;
    }
};

ReloopBeatpad.WheelSeek = function (channel, control, value, status, group) {
    var decknum = group.substring(8,9)>>0;
    var deck = ReloopBeatpad.decks["D" + decknum];  
	// See if we're seekink fast. If not, it means the DJ
    // is using the border. Seeking slowly (beatjump function works 
    // way better than "fwd"or "back" functions with jogwheels
    if (!deck.seekingfast)  {
        engine.setValue(group, "beatjump", (value-0x40)/4);
        return;
    }
    // Register the movement
    engine.setValue(group,"beatjump",(value-0x40));
   
};

ReloopBeatpad.WheelBendTouch  = function (channel, control, value, status, group) {
    var decknum = group.substring(8,9)>>0;
    var deck = ReloopBeatpad.decks["D" + decknum];  
    if (value == DOWN) {
        deck.jogbending=true;
    }
    else {    // If button up
        engine.setValue(group, "jog", 0);
        deck.jogbending=false;
    }
};

ReloopBeatpad.WheelBend = function (channel, control, value, status, group) {
    var decknum = group.substring(8,9)>>0;
    var deck = ReloopBeatpad.decks["D" + decknum];  
       
    // See if we're scratching. If not, do wheel jog.
    if (!deck.jogbending)  {
        engine.setValue(group, "jog", (value-0x40)/4);
        return;
    }
    // Register the movement
    engine.setValue(group, "jog", (value-0x40)/4);
};  
 
 

// ********************** Buttons and Co ****************
ReloopBeatpad.ShiftBtn = function(channel, control, value, status, group) {
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
	deck.Shifted=(value == DOWN);
	deck.trigger();
};

ReloopBeatpad.brake = function(channel, control, value, status, group) {
	var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];  
	if (value == DOWN) {
		deck.status.brake.onOff(ON);
		engine.brake(decknum, true); // enable brake effect
	}
	else {
		deck.status.brake.onOff(OFF);
		engine.brake(decknum, false); // disable brake effect
	}   
};

//censor
ReloopBeatpad.reverseroll = function(channel, control, value, status, group) {
	
	var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];  
	if (value == DOWN) {
		deck.control.censor.onOff(ON);
		deck.control.censor.light.onOff(ON);
	}
	else {
		deck.control.censor.onOff(OFF);
		deck.control.censor.light.onOff(OFF);
	}   
};



ReloopBeatpad.SelectPlayList = function(channel, control, value, status, group) {
     value = value - 0x40;
     if (value < 0) {
            for(i = 0; i < -value; i++) {
                engine.setValue(group, "SelectPrevPlaylist", true);
            }
        } else {
            for(i = 0; i < value; i++) {
                engine.setValue(group, "SelectNextPlaylist", true);
            }
        }
};

ReloopBeatpad.RecBtn = function(channel, control, value, status, group) {
	if (value == DOWN) {
		engine.setValue(group, "toggle_recording", true);
	} 
};

ReloopBeatpad.LoadBtn = function(channel, control, value, status, group) {
	//LOAD hold <500ms : load track, >500ms : eject 
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
	if (value == DOWN) {
		deck.LOADdown();
	}
	else {
		deck.LOADup();
	}   
};
    
ReloopBeatpad.SyncBtn = function(channel, control, value, status, group) {
	//SYNC pressed once :sync  , pressed twice : play
	
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];
	if (value == DOWN) {		
		deck.SYNCdown();				
	}
	else {
		deck.SYNCup();
		engine.trigger(group, "sync_enabled"); 
	}
};

ReloopBeatpad.StartBtn = function(channel, control, value, status, group) {
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];
	if (value == DOWN) {
		deck.control.start.onOff(ON);
		deck.control.start.light.onOff(ON);
		
	}
	else {
		deck.control.start.onOff(OFF);
		//deck.control.start.light.onOff(OFF);		
	}
};

ReloopBeatpad.PlayBtn = function(channel, control, value, status, group) {
	//toggle play. If no track is loaded, load the one which is selected and play
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)]; 
		if (value == DOWN) {
		if(!deck.loadedCheck()) {
			deck.control.LoadAndPlay.onOff(true);
		} else 
		{
			deck.control.play.onOff(!deck.control.play.checkOn());//toggle play/pause
		}
	}        
};

ReloopBeatpad.JumpBtn = function(channel, control, value, status, group) {
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
	if (value == DOWN) {
		if(!deck.loadedCheck()) {
			deck.control.load.onOff(ON);
		}
		deck.control.jump.onOff(true);
		deck.control.jump.onOff(true); //to avoid this : https://bugs.launchpad.net/mixxx/+bug/1504503
	}        
};


ReloopBeatpad.JogSeekBtn = function(channel, control, value, status, group) {
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
	deck.JogSeekStatus=(value==DOWN);
	deck.beatpos=0;
	engine.trigger(group,"playposition");
};
    
ReloopBeatpad.JogScratchBtn = function(channel, control, value, status, group) {
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
	deck.JogScratchStatus=(value==DOWN);
	deck.beatpos=0;
	engine.trigger(group,"playposition");
};

ReloopBeatpad.FX_ONBtn = function(channel, control, value, status, group) {
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
	if (value==DOWN) {
		if (deck.status.FX_ON.checkOn()==OFF) {
			deck.status.FX_ON.onOff(ON);
			deck.control["FXPad"+deck.CurrentEffectRack].onOff(ON);
			deck.RGBShow.effects(deck.CurrentEffectRack); 
			deck.status.FX_ON.onOff(ON);
		} else {
			deck.control["FXPad"+deck.CurrentEffectRack].onOff(OFF);
			deck.status.FX_ON.onOff(OFF);
			deck.RGBShow.effects(OFF);
		}	
	}
};

//Pads Performance mode
ReloopBeatpad.ShowSamplersAndEffects = function () {
	var PadMode1=ReloopBeatpad.decks.D1.PadMode;
	var PadMode2=ReloopBeatpad.decks.D2.PadMode;
	//SAMPLERMODE/ SAMPLERBANKSTATUSMODE / FXRACKSELECTMODE
	var OpenFX=((PadMode1==FXMODE)||(PadMode2==FXMODE)||(PadMode1==FXRACKSELECTMODE)||(PadMode2==FXRACKSELECTMODE));
	var OpenSampler=((PadMode1==SAMPLERMODE)||(PadMode2==SAMPLERMODE)||(PadMode1==SAMPLERBANKSTATUSMODE)||(PadMode2==SAMPLERBANKSTATUSMODE));
	engine.setValue("[Samplers]","show_samplers",OpenSampler);
	engine.setValue("[EffectRack1]","show",OpenFX);
};

ReloopBeatpad.CueBtn = function(channel, control, value, status, group) {
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)]; 
	if (value==DOWN) {
		deck.PadMode=CUEMODE;
		ReloopBeatpad.ShowSamplersAndEffects();
	}
};

ReloopBeatpad.InstantFXBtn = function(channel, control, value, status, group) { 
    var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];
	
    if (control<=0x30){  //(SHIFT+Btn)<=0x30
		if (value == DOWN) {
			engine.spinback(decknum, true); // enable spinback effect
			deck.status.spinback.onOff(ON);
		}
		else {
			engine.spinback(decknum, false); // disable spinback effect
			deck.status.spinback.onOff(OFF);
		}   
	} else {
		deck.InstantFXBtnDown=(value==DOWN);
		if (value==DOWN) {
			deck.PadMode=FXMODE;
			ReloopBeatpad.ShowSamplersAndEffects();
		}
	}
};

ReloopBeatpad.FXSelectPush = function(channel, control, value, status, group) { 
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)]; 
	// quick button for the current fx
	 
	 //desactivate previous pending effect	 
	 if (value==DOWN) {
		deck.control["FXPad"+deck.CurrentEffectRack].onOff(ON);
		deck.status.FX_ON.light.onOff(ON);
		deck.RGBShow.effects(deck.CurrentEffectRack);
		
	} else {
		if (!deck.status.FX_ON.checkOn()) {
			/* We did it already :
			 * -------------------
			 * deck.control["FXPad"+padindex].onOff(OFF); 
			 */
			deck.RGBShow.effects(OFF);
			deck.control["FXPad"+deck.CurrentEffectRack].onOff(OFF);
			deck.status.FX_ON.light.onOff(OFF);
		} else {
			deck.control["FXPad"+deck.CurrentEffectRack].onOff(ON);
			deck.RGBShow.effects(deck.CurrentEffectRack);
		}
	}
};

ReloopBeatpad.sFXSelectPush = function(channel, control, value, status, group) { 
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)]; 
	// quick button for the QUICK fx : ENABLE_DISABLE
	 
	//desactivate previous pending effect	 
	if (value==DOWN) {
		var oldvalue=engine.getValue("[QuickEffectRack1_"+group+"_Effect1]", "enabled");
		engine.setValue("[QuickEffectRack1_"+group+"_Effect1]", "enabled",(!oldvalue));
	}
};

ReloopBeatpad.InstantFXPad = function(channel, control, value, status, group) { 
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)]; 
	var padindex=control-ReloopBeatpad.MIDI.FXPad+1;
	// pads are quick buttons
	if (deck.InstantFXBtnDown) {
		if (value==DOWN) {
			// InstantFX down+InstantFX Pad = SHIFT + InstantFX Pad
			// select/unselect current effectunits for settings
			deck.SelectEffectRack(padindex);
		}
	} else {
		//desactivate previous pending effect
		deck.control["FXPad"+deck.CurrentEffectRack].onOff(OFF);
		if (value==DOWN) {
			deck.control["FXPad"+padindex].onOff(ON);
			deck.RGBShow.effects(deck.CurrentEffectRack);
			deck.status.FX_ON.light.onOff(ON);
		} else {
			if (!deck.status.FX_ON.checkOn()) {
				/* We did it already :
				 * -------------------
				 * deck.control["FXPad"+padindex].onOff(OFF); 
				 */
				deck.RGBShow.effects(OFF); 
				deck.status.FX_ON.light.onOff(OFF);
			} else {
				deck.control["FXPad"+deck.CurrentEffectRack].onOff(ON);
				deck.RGBShow.effects(deck.CurrentEffectRack);
			}
		}
	}
};

ReloopBeatpad.sInstantFXPad = function(channel, control, value, status, group) { 
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)]; 
	var padindex=control-ReloopBeatpad.MIDI.FXPad+1-SHIFT;
	// SHIFT + InstantFX Pad
	// select/unselect current effectunits for settings
	if (value==DOWN) {
		deck.SelectEffectRack(padindex);
	}
};

ReloopBeatpad.BounceBtn = function(channel, control, value, status, group) {
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
	if (value==DOWN) {
		deck.PadMode=LOOPMODE;
		ReloopBeatpad.ShowSamplersAndEffects();
	}	
};

ReloopBeatpad.LoopBtn = function(channel, control, value, status, group) {
	var isLoopActive = engine.getValue(group, "loop_enabled");
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
	if (value==DOWN) {
			if (deck.loopkind==SIMPLE) {	
				if (!isLoopActive) {		
					//activate loop
					engine.setValue(group, "beatloop_" + deck.loopsize.toString() + "_activate", 1);
					deck.status.Loop.onOff(ON);
					deck.RGBShow.loops(ON);
				} else {
					//deactivate loop
					engine.setValue(group, "reloop_exit", true);
					deck.status.Loop.onOff(OFF);
					deck.RGBShow.loops(OFF);
				}
			} else {
				//activate roll
				engine.setValue(group, "beatlooproll_" + deck.loopsize.toString() + "_activate", 1);
				deck.status.Loop.onOff(ON);
				deck.RGBShow.loops(ON);
			}
			//deck.RGBShow.loops(deck.); 
		} else {
			if (deck.loopkind==ROLL) {	
				//deactivate roll		
				engine.setValue(group, "beatlooproll_" + deck.loopsize.toString() + "_activate", 0);
				deck.status.Loop.onOff(OFF);
				deck.RGBShow.loops(OFF);
			}
		}	
};

ReloopBeatpad.LoopPad = function(channel, control, value, status, group) { 
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)]; 
	var padindex=control-ReloopBeatpad.MIDI.LoopPad+1;
	 if (value==DOWN) {
		deck.loopsize=Math.pow(2, padindex-4);
		deck.looppadstatus+=1;	
	 	deck.control["LoopPad"+padindex].onOff(ON);
		deck.status.Loop.light.onOff(OFF);
		deck.RGBShow.loops(ON);	
	} else {
		if (!deck.status.Loop.checkOn()) {
			deck.looppadstatus-=1;	
			if (deck.looppadstatus==0) {
				deck.control["LoopPad"+padindex].onOff(OFF);
				deck.RGBShow.loops(OFF);
			} 
		} else {
			deck.loopsize=Math.pow(2, padindex-4);
			deck.RGBShow.loops(ON);
		}
	}
};

ReloopBeatpad.sLoopPad = function(channel, control, value, status, group) { 
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)]; 
	var padindex=control-ReloopBeatpad.MIDI.LoopPad+1-SHIFT;
	 if (value==DOWN) {
		deck.loopsize=Math.pow(2, padindex);
		deck.looppadstatus+=1;
		deck.control["sLoopPad"+padindex].onOff(ON);
		deck.status.Loop.light.onOff(OFF);
		deck.RGBShow.loops(ON);			
	} else {
		if (!deck.status.Loop.checkOn()) {
			deck.looppadstatus-=1;	
			if (deck.looppadstatus==0) {
				deck.control["LoopPad"+padindex].onOff(OFF);
				deck.RGBShow.loops(OFF);
			}
		} else {
			deck.loopsize=Math.pow(2, padindex);
			deck.RGBShow.loops(ON);
		}
	}
};

ReloopBeatpad.LoopSizeBtn = function(channel, control, value, status, group) { 
	var isLoopActive = engine.getValue(group, "loop_enabled");
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
	if (value==DOWN) {
			if (deck.loopkind==SIMPLE) {	
				if (!isLoopActive) {		
					//activate loop
					engine.setValue(group, "beatloop_" + deck.loopsize.toString() + "_activate", 1);
					deck.status.Loop.onOff(ON);
					deck.RGBShow.loops(ON);
				} else {
					//deactivate loop
					engine.setValue(group, "reloop_exit", true);
					deck.status.Loop.onOff(OFF);
					deck.RGBShow.loops(OFF);
				}
			} else {
				//activate roll
				engine.setValue(group, "beatlooproll_" + deck.loopsize.toString() + "_activate", 1);
				deck.status.Loop.onOff(ON);
				deck.RGBShow.loops(ON);
			}
			//deck.RGBShow.loops(deck.); 
		} else {
			if (deck.loopkind==ROLL) {	
				//deactivate roll		
				engine.setValue(group, "beatlooproll_" + deck.loopsize.toString() + "_activate", 0);
				deck.status.Loop.onOff(OFF);
				deck.RGBShow.loops(OFF);
			}
		}	
};

ReloopBeatpad.sLoopSizeBtn = function(channel, control, value, status, group) { 
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
     if (value==DOWN) { 
		 // toggle loop mode into SIMPLE/ROLL
		 deck.ToggleLoopKind();
	}
};

ReloopBeatpad.LoopSizeKnob = function(channel, control, value, status, group) { 
        var isLoopActive = engine.getValue(group, "loop_enabled");
        var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];
        var knobValue = value-0x40;

        if(isLoopActive){
            if (control<0x30) {
                if(knobValue > 0)
                    engine.setValue(group, "loop_move", 1);
                else
                    engine.setValue(group, "loop_move", -1);
            } else {
                if(knobValue > 0) {
                    // Because loop_halve is supposed to be a pushbutton, we have to
                    // fake the button-off event to clear out the "pressed" status.
                    engine.setValue(group, "loop_double", 1);
                    engine.setValue(group, "loop_double", 0);
                    deck.loopsize*=2;
                } else {
                    engine.setValue(group, "loop_halve", 1);
                    engine.setValue(group, "loop_halve", 0);
                    deck.loopsize/=2.0;
                }
            }
        }
};

ReloopBeatpad.SamplerBtn = function(channel, control, value, status, group) { 
	var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];  
	if (value==DOWN) {
		if (deck.PadMode<SAMPLERMODE) {
			deck.PadMode=SAMPLERMODE;
		} else {
			//Cycle the Sampler Pads in three modes : SAMPLERMODE/ SAMPLERBANKSTATUSMODE / FXRACKSELECTMODE
			deck.PadMode+=1;
			if (deck.PadMode>FXRACKSELECTMODE) {
				deck.PadMode=SAMPLERMODE;
			}
		}		
		ReloopBeatpad.ShowSamplersAndEffects();
		ReloopBeatpad.samplers.LedsUpdate();
	}	
};

ReloopBeatpad.SamplerPad = function(channel, control, value, status, group) { 
	var padindex=control-ReloopBeatpad.MIDI.SamplerPad+1;
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
	if (value==DOWN) { 
		switch (deck.PadMode) {
			case SAMPLERMODE       : ReloopBeatpad.samplers.play(padindex,true); break;
			case SAMPLERBANKSTATUSMODE : ReloopBeatpad.samplers.LoadBank(padindex); break;
			case FXRACKSELECTMODE  :  deck.SelectEffectRack(padindex); break;
			case LOOPMODESTATUS : deck.ToggleLoopKind(); break;
		}
	}
};

ReloopBeatpad.sSamplerPad = function(channel, control, value, status, group) { 
	var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];
	var padindex=control-ReloopBeatpad.MIDI.SamplerPad+1-SHIFT;  
	if (value==DOWN) { 
		ReloopBeatpad.samplers.play(padindex,false);
	}
};

ReloopBeatpad.FXSelectKnob = function(channel, control, value, status, group) { 
    var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];
	value = value - 0x40;
	var n=deck.CurrentEffectRack;
	engine.setValue("[EffectRack1_EffectUnit"+n+"]", "chain_selector",value);
};

//Does not work for the moment, not implemented in Mixxx
ReloopBeatpad.sFXSelectKnob = function(channel, control, value, status, group) { 
    var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];
	value = value - 0x40;
	if (value>0) {
		engine.setValue("[QuickEffectRack1]", "effect_selector",value);
	}
};

ReloopBeatpad.FXParam = function(channel, control, value, status, group) { 
	//Super for EffectRack
	var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];
    engine.setValue("[EffectRack1_EffectUnit"+deck.CurrentEffectRack+"]", "super1",value/128);
};

ReloopBeatpad.FXParamShift = function(channel, control, value, status, group) { 
	var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];
    engine.setValue("[EffectRack1_EffectUnit"+deck.CurrentEffectRack+"]", "mix",value/128);
};

ReloopBeatpad.FilterMid = function(channel, control, value, status, group) { 
	var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];
    engine.setValue("[QuickEffectRack1_"+group+"]", "super1_set_default",value);
    deck.status.RimBlue.light.onOff(OFF);
    deck.RGBShow.filter(0);
    deck.filterligthshowstatus=0;
};

ReloopBeatpad.FilterKnob = function(channel, control, value, status, group) {  
	//Super for QuickEffectRack1_[Channel1]
	var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];
	var newstatus=(value>0x40)?2:4;
    engine.setValue("[QuickEffectRack1_"+group+"]", "super1",value/128);
    if (deck.filterligthshowstatus!=newstatus) {
		deck.status.RimBlue.light.onOff(newstatus);
		if (deck.filterligthshowstatus==0) {
			deck.RGBShow.filter(value-0x40);
		}
	}
	deck.filterligthshowstatus=newstatus;    
};


// ----------- LED Output functions -------------

ReloopBeatpad.OnPlayIndicatorChange = function (value, group, control) {	
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
	if (!deck.Shifted) {
		deck.control.play.light.onOff((value)? ON : OFF);
	}
};

ReloopBeatpad.OnCuePointChange = function (value, group, control) {
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)]; 
	if (!deck.Shifted) {
	   deck.control.Set.light.onOff((value>0)? ON : OFF);
	   deck.control.jump.light.onOff((value>0)? ON : OFF);
	}	
};

ReloopBeatpad.OnSyncEnabledChange = function (value, group, control) {
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
	deck.control.sync.light.onOff((value)? ON : OFF);
};

ReloopBeatpad.OnTrackLoaded = function (value, group, control) {
	var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];
	deck.control.load.light.onOff((value)? ON : OFF);
	if (value==0) {
		deck.loaded=false;
		deck.RGBShow.notloaded(true);
	} else {
		deck.loaded=true;
		deck.RGBShow.notloaded(false);
		deck.status.RimRed.light.onOff(OFF);
	}
	engine.trigger(group,"playposition");
};

ReloopBeatpad.OnKeylock  = function (value, group, control) {	
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)];  
	var dispValue = (value)? ON : OFF;
	deck.control.keylock.light.onOff(dispValue); // calling only the light object is very important ! (infinite loop triggered)
};

ReloopBeatpad.OnVuMeterChange  = function (value, group, control) {
	var deck = ReloopBeatpad.decks["D" + group.substring(8,9)]; 
	var dispValue = Math.round(8.0*value);
	deck.status.VUMeter.onOff(dispValue);
};

ReloopBeatpad.OnPlaypositionChange  = function (value, group, control) { 
	// Rim Red : 1st behaviour 0x01-0x18 ; 2nd Behavior = 1st +24 ;3d behavior ON/OFF
	var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum]; 
	if (deck.loaded){
		deck.control.start.light.onOff((value)?OFF:ON);
		var timeremaining=engine.getValue(group,"duration")*(1-value);
		var trackwarning=0;
		var ledindex=0;
		if (timeremaining<=30){ trackwarning=1;}
		if (timeremaining<=10){ trackwarning=2;}
		if (timeremaining<=0) { trackwarning=3;}
		switch (trackwarning) {
		   case 0 :
				var isLoopActive = engine.getValue(group, "loop_enabled");
				if (isLoopActive) { //loop_position clock
					var ts=engine.getValue(group,"track_samples");
					var loopstart=engine.getValue(group,"loop_start_position");
					var loopend=engine.getValue(group,"loop_end_position");
					var loop_position=(value*ts-loopstart)/(loopend-loopstart);
					ledindex = Math.round(23.0*loop_position)+1;
					deck.status.RimRed.light.onOff(ledindex);
				} else {
					if (deck.JogScratchStatus) { //Spinny
						var revolutions = value*ReloopBeatpad.getduration(group)/1.8; //33+1/3 rev/mn=1.8 s/rev
						var needle=((revolutions-(revolutions|0))*24)|0; 
						ledindex =Math.floor(needle)+1;
						deck.status.RimRed.light.onOff(ledindex);
					} else if (deck.JogSeekStatus) { //Track position
						//Track position/ellapsed time
						ledindex = Math.round(24.0*value);
						if (ledindex!=0) {ledindex+=24;}
						deck.status.RimRed.light.onOff(ledindex);
					}
				}
				break;
				
			case 1: // if less than 30 seconds before end of track : flashing slowly
				if (deck.status.RimRed.light.get_flashDuration()!=1000) 
				{
					deck.status.RimRed.light.flashOff();
					deck.status.RimRed.light.flashOn(1000, ON, 1000);
				}			
				break;
				
			case 2: // if less than 10 seconds before end of track : flashing fast
				if (deck.status.RimRed.light.get_flashDuration()!=300) 
				{
					deck.status.RimRed.light.flashOff();
					deck.status.RimRed.light.flashOn(300, ON, 300);
				}			
				break;
				
			case 3: // end of strack : full ring lit
				deck.status.RimRed.light.flashOff();
				deck.status.RimRed.light.onOff(ON);			
				break;			
		} 
	}
};

ReloopBeatpad.OnBeatActive = function (value, group, control) {
	//OnPlayPosition n°2 !!!
	var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum]; 
	//TODO flashing end of track
	if (!(deck.JogScratchStatus||deck.JogSeekStatus)){
		if (value==1){	
			deck.status.RimRed.light.onOff(deck.beatpos*3+1);
			deck.beatpos+=1;
			if (deck.beatpos>=8) {deck.beatpos=0;}
		}
	}
};

ReloopBeatpad.OnRecordingStatusChange = function (value, group, control) {
	ReloopBeatpad.status.recording.light.onOff((value)?ON:OFF);
};

ReloopBeatpad.OnBPMChange = function (value, group, control) {
	var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];
	
	var otherdecknum=3-decknum;
	var othergroup="[Channel"+otherdecknum+"]";
	var otherdeck = ReloopBeatpad.decks["D" + otherdecknum];
   engine.trigger(group,"playposition");
};

ReloopBeatpad.OnDurationChange = function (value, group, control) { 
   engine.trigger(group,"playposition");
};

ReloopBeatpad.OnHotcuePositionChange = function(value, group, control,padindex) {
	var decknum = group.substring(8,9)>>0;
	var deck = ReloopBeatpad.decks["D" + decknum];	
	deck.status["CuePad"+padindex].onOff((value==-1)?OFF:ON);
	deck.status["sCuePad"+padindex].onOff((value==-1)?OFF:ON);
};

ReloopBeatpad.OnEffectLoaded  = function (value, group, control,index) { 	
	var parameterlinkedcount=0;
	var linktype=0;
	if (value) {
		var numparams=engine.getValue("[EffectRack1_EffectUnit"+index+"_Effect1]", "num_parameters");
		if (numparams>0){
			for(i=1;i<=numparams;i++){
				linktype=engine.getValue("[EffectRack1_EffectUnit"+index+"_Effect1]", "parameter"+i+"_link_type");
				if(linktype!=0)
				{parameterlinkedcount+=1;}
			}
		}
		if (parameterlinkedcount==0){
			for(i=1;i<=numparams;i++){
				engine.setValue("[EffectRack1_EffectUnit"+index+"_Effect1]", "parameter"+i+"_link_type",1);
			}
			
		}
	}
};

ReloopBeatpad.OnSampleLoaded   = function (value, group, control) {	
	
	var samplenum = parseInt(group.replace(/[^0-9\.]/g, ''),10);
	ReloopBeatpad.samplers.SetLoaded(samplenum,(value!=0));
};

ReloopBeatpad.OnSamplePlayStop   = function (value, group, control) {	
	var samplenum = parseInt(group.replace(/[^0-9\.]/g, ''),10);
	ReloopBeatpad.samplers.LedUpdateSampler(samplenum);
};

ReloopBeatpad.OnBankLoaded = function (value, group, control,index)  {
	if (value==1) {		
		var decknum = group.substring(8,9)>>0;
		ReloopBeatpad.samplers.bankactive=index;	
		ReloopBeatpad.samplers.LedsUpdate();
	}
};


ReloopBeatpad.inboundSysex = function (data, length) {
    print("data sysex :"+data);
};
