/**
 * Hercules DJ Control Steel controller script v1.8.1
 * Copyright (C) 2010  Anders Gunnarsson for DJ Console RMX
 * Copyright (C) 2010  Stephane List for DJ Control Steel adaptation
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

/** Changelog

v0.2 10/16/2010 invert led light for cueSelect
v0.1 10/09/2010 First public release

**/

//TODO: Cleanup, create objects from init.
//Remove led timers when alsa midi is working properly.
HerculesSteel = new function() {
	this.group = "[Master]";
	this.shiftMode = false;
	this.scratchMode = false;
	this.jogPlaylistScrollMode = false;
	this.Controls = [];
	this.Buttons = [];
};

HerculesSteel.addButton = function(buttonName, button, eventHandler) {
	if(eventHandler) {
		var executionEnvironment = this;
		function handler(value) {
			button.state = value;
			executionEnvironment[eventHandler](value);
		}
		button.handler = handler;
	}
	this.Buttons[buttonName] = button;
};

HerculesSteel.setControlValue = function(control, value) {
	this.Controls[control].setValue(this.group, value);
};


HerculesSteel.ButtonState = {"released":0x00, "pressed":0x7F};
HerculesSteel.LedState =  {"off": 0x00, "on": 0x7F, "blink": 0xFF};
HerculesSteel.Button = function (controlId, blinkId) {
	this.controlId = controlId;
	this.blinkId = blinkId;
	this.state = HerculesSteel.ButtonState.released;
};

	HerculesSteel.Button.prototype.setLed = function(lightState) {
		if(lightState == HerculesSteel.LedState.on)
		{
			engine.beginTimer(20, "midi.sendShortMsg(0xB0," + (this.controlId) + ", " + HerculesSteel.LedState.on + ")", true);
		}
		else if(lightState == HerculesSteel.LedState.blink)
		{
			engine.beginTimer(20, "midi.sendShortMsg(0xB0," + (this.controlId + 0x30) + ", " + HerculesSteel.LedState.on + ")", true);
		}
		else
		{
			engine.beginTimer(20, "midi.sendShortMsg(0xB0," + (this.controlId) + ", " + HerculesSteel.LedState.off + ")", true);
			engine.beginTimer(40, "midi.sendShortMsg(0xB0," + (this.controlId + 0x30) + ", " + HerculesSteel.LedState.off + ")", true);
		}
	};

HerculesSteel.Button.prototype.handleEvent = function(value) {
	this.handler(value);
};

HerculesSteel.Control = function(mappedFunction, softMode) {
	this.minInput = 0;
	this.midInput = 0x3F;
	this.maxInput = 0x7F;
	this.minOutput = -1.0;
	this.midOutput = 0.0;
	this.maxOutput = 1.0;
	this.mappedFunction = mappedFunction;
	this.softMode = softMode;
	this.maxJump = 10;
};

HerculesSteel.Control.prototype.setValue = function(group, inputValue){
	var outputValue = 0;
	if(inputValue <= this.midInput){
		outputValue = this.minOutput + ((inputValue - this.minInput) / (this.midInput - this.minInput)) * (this.midOutput - this.minOutput);
	} else {
		outputValue = this.midOutput + ((inputValue - this.midInput) / (this.maxInput - this.midInput)) * (this.maxOutput - this.midOutput);
	}
	if(this.softMode){
		var currentValue = engine.getValue(group, this.mappedFunction);
		var currentRelative = 0.0;
		if(currentValue <= this.midOutput){
			currentRelative = this.minInput + ((currentValue - this.minOutput) / (this.midOutput - this.minOutput)) * (this.midInput - this.minInput);
		} else {
			currentRelative = this.midInput + ((currentValue - this.midOutput) / (this.maxOutput - this.midOutput)) * (this.maxInput - this.midInput);
		}
		if(inputValue > currentRelative - this.maxJump && inputValue < currentRelative + this.maxJump) {
			engine.setValue(group, this.mappedFunction, outputValue);
		}
	} else {
		engine.setValue(group, this.mappedFunction, outputValue);
	}
};

HerculesSteel.shiftHandler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {

		print("shiftMode = true");
		this.shiftMode = true;
		if(HerculesSteel.scratchMode) {
			HerculesSteel.scratchMode = false;
			HerculesSteel.Buttons.Scratch.setLed(HerculesSteel.LedState.off);
		} else {
			HerculesSteel.scratchMode = true;
			HerculesSteel.Buttons.Scratch.setLed(HerculesSteel.LedState.on);
		}
	} else {
		print("shiftMode = false");
		this.shiftMode = false;
	}
};

HerculesSteel.upHandler = function(value) {
	this.jogPlaylistScrollMode = this.Buttons.Up.state + this.Buttons.Down.state > 0;
	if(value == HerculesSteel.ButtonState.pressed) {
		engine.setValue("[Playlist]", "SelectPrevTrack", 1);
	}
};

HerculesSteel.downHandler = function(value) {
	this.jogPlaylistScrollMode = this.Buttons.Up.state + this.Buttons.Down.state > 0;
	if(value == HerculesSteel.ButtonState.pressed) {
		engine.setValue("[Playlist]", "SelectNextTrack", 1);
	}
};

HerculesSteel.leftHandler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		engine.setValue("[Playlist]", "SelectPrevPlaylist", 1);
	}
};

HerculesSteel.rightHandler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		engine.setValue("[Playlist]", "SelectNextPlaylist", 1);
	}
};

HerculesSteel.addButton("Scratch", new HerculesSteel.Button(0x29), "shiftHandler");
HerculesSteel.addButton("Up", new HerculesSteel.Button(0x2A), "upHandler");
HerculesSteel.addButton("Down", new HerculesSteel.Button(0x2B), "downHandler");
HerculesSteel.addButton("Left", new HerculesSteel.Button(0x2C), "leftHandler");
HerculesSteel.addButton("Right", new HerculesSteel.Button(0x2D), "rightHandler");

HerculesSteel.Controls = {
	"Balance" : new HerculesSteel.Control("balance", false),
	"Volume" : new HerculesSteel.Control("volume", false),
	"CrossFader" : new HerculesSteel.Control("crossfader", false),
	"HeadPhoneMix" : new HerculesSteel.Control("headMix", false),
	"FlangerDepth" : new HerculesSteel.Control("lfoDepth", false),
	"FlangerDelay" : new HerculesSteel.Control("lfoDelay", false),
	"FlangerPeriod" : new HerculesSteel.Control("lfoPeriod", false)
};
HerculesSteel.Controls.Volume.minOutput = 0.0;
HerculesSteel.Controls.Volume.midOutput = 1.0;
HerculesSteel.Controls.Volume.maxOutput = 5.0;
HerculesSteel.Controls.FlangerDepth.minOutput = 0.0;
HerculesSteel.Controls.FlangerDepth.midOutput = 0.5;
HerculesSteel.Controls.FlangerDepth.maxOutput = 1.0;
HerculesSteel.Controls.FlangerDelay.minOutput = 50;
HerculesSteel.Controls.FlangerDelay.midOutput = 5000;
HerculesSteel.Controls.FlangerDelay.maxOutput = 10000;
HerculesSteel.Controls.FlangerPeriod.minOutput = 50000;
HerculesSteel.Controls.FlangerPeriod.midOutput = 1000000;
HerculesSteel.Controls.FlangerPeriod.maxOutput = 2000000;

HerculesSteel.balanceHandler = function(value) {
	this.Controls.Balance.setValue(this.group, value);
};

HerculesSteel.volumeHandler = function(value) {
	this.Controls.Volume.setValue(this.group, value);
};

HerculesSteel.crossFaderHandler = function(value) {
	this.Controls.CrossFader.setValue(this.group, value);
};

HerculesSteel.headPhoneMixHandler = function(value) {
	this.Controls.HeadPhoneMix.setValue(this.group, value);
};

/**
 * Deck
 * @param deckNumber
 * @param group
 */
HerculesSteel.Deck = function (deckNumber, group) {
	this.deckNumber = deckNumber;
	this.group = group;
	this.shiftMode = false;
	this.scratching = false;
	this.scratchTimer = -1;
	this.cuePlaying = false;
	this.playing = false;
	this.Buttons = [];
};

HerculesSteel.Deck.prototype.jogMove = function(jogValue) {
	if(HerculesSteel.jogPlaylistScrollMode) {
		if (jogValue > 0) {
			engine.setValue("[Playlist]","SelectNextTrack", 1);
		} else if (jogValue < 0) {
			engine.setValue("[Playlist]","SelectPrevTrack", 1);
		}
	} else if(HerculesSteel.scratchMode) {
		if(!this.scratching) {
			this.scratching = true;
			engine.scratchEnable(this.deckNumber, 128, 45, 1.0/8, (1.0/8)/32);
		} else {
			engine.stopTimer(this.scratchTimer);
		}
		engine.scratchTick(this.deckNumber, jogValue);
		this.scratchTimer = engine.beginTimer(20, "HerculesSteel.GetDeck('" + this.group + "').stopScratching()", true);
	} else {
		engine.setValue(this.group,"jog", jogValue);
	}
};

HerculesSteel.Deck.prototype.stopScratching = function() {
	this.scratching = false;
	engine.scratchDisable(this.deckNumber);
	this.scratchTimer  = -1;
};

HerculesSteel.Deck.prototype.setControlValue = HerculesSteel.setControlValue;

HerculesSteel.Deck.prototype.pitchResetHandler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		engine.setValue(this.group,"rate",0);
		this.Buttons.PitchReset.setLed(HerculesSteel.LedState.on);
	}
	else {
		// Button is called Pitch Bend-, so when button is released, turn off the LED
		this.Buttons.PitchReset.setLed(HerculesSteel.LedState.off);
	}
};

HerculesSteel.Deck.prototype.syncHandler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		engine.setValue(this.group,"beatsync",0);
		this.Buttons.Sync.setLed(HerculesSteel.LedState.on);
	}
	else
	{
		this.Buttons.Sync.setLed(HerculesSteel.LedState.off);
	}
};

HerculesSteel.Deck.prototype.keypad1Handler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		if(engine.getValue(this.group,"flanger") == 0) {
			engine.setValue(this.group,"flanger",1);
			this.Buttons.Keypad1.setLed(HerculesSteel.LedState.on);
		} else {
			engine.setValue(this.group,"flanger",0);
			this.Buttons.Keypad1.setLed(HerculesSteel.LedState.off);
		}
	}
};

HerculesSteel.Deck.prototype.keypad2Handler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		this.Buttons.Keypad2.setLed(HerculesSteel.LedState.on);
		engine.setValue(this.group,"hotcue_1_set", 1);
	}
	else {
		this.Buttons.Keypad2.setLed(HerculesSteel.LedState.off);
	}
};

HerculesSteel.Deck.prototype.keypad3Handler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		{
			this.Buttons.Keypad3.setLed(HerculesSteel.LedState.on);
			engine.setValue(this.group,"hotcue_2_set", 1);
		}
	} else {
			this.Buttons.Keypad3.setLed(HerculesSteel.LedState.off);
	}
};

HerculesSteel.Deck.prototype.keypad4Handler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		this.Buttons.Keypad4.setLed(HerculesSteel.LedState.on);
		engine.setValue(this.group,"reverse",1);
	} else {
		this.Buttons.Keypad4.setLed(HerculesSteel.LedState.off);
		engine.setValue(this.group,"reverse",0);
	}
};

HerculesSteel.Deck.prototype.keypad5Handler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		this.Buttons.Keypad5.setLed(HerculesSteel.LedState.on);
		engine.setValue(this.group,"hotcue_1_goto", 1);
	}
	else {
		this.Buttons.Keypad5.setLed(HerculesSteel.LedState.off);

	}
};

/* HerculesSteel.Deck.prototype.keypad6Handler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		if(this.shiftMode) {
			var loopIn = engine.getValue(this.group, "loop_start_position");
			var loopOut = engine.getValue(this.group, "loop_end_position");
			var loopLength = loopOut - loopIn;
			if (loopIn != -1 && loopOut != -1 && loopLength >= 2) {
				engine.setValue(this.group,"loop_end_position",loopIn + loopLength / 2);
			}
		} else {
			engine.setValue(this.group,"reloop_exit",1);
		}
	}
};
engine.setValue(this.group,"loop_in",1);
engine.setValue(this.group,"loop_out",1);
*/

HerculesSteel.Deck.prototype.keypad6Handler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		this.Buttons.Keypad6.setLed(HerculesSteel.LedState.on);
		engine.setValue(this.group,"hotcue_2_goto",1);
	}
	else {
		this.Buttons.Keypad6.setLed(HerculesSteel.LedState.off);
	}
};

// Loop in
HerculesSteel.Deck.prototype.keypad7Handler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		this.Buttons.Keypad7.setLed(HerculesSteel.LedState.on);
		engine.setValue(this.group,"loop_in",1);
	}
	else {
		this.Buttons.Keypad7.setLed(HerculesSteel.LedState.off);
	}
};

// Loop exit
HerculesSteel.Deck.prototype.keypad8Handler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		this.Buttons.Keypad8.setLed(HerculesSteel.LedState.on);
		print("reloop_exit");
		engine.setValue(this.group,"reloop_exit",1);

		}
	else
	{
		this.Buttons.Keypad8.setLed(HerculesSteel.LedState.off);
	}
};

HerculesSteel.Deck.prototype.keypad9Handler = function(value) {};

// Loop out
HerculesSteel.Deck.prototype.keypad10Handler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		this.Buttons.Keypad10.setLed(HerculesSteel.LedState.on);
		engine.setValue(this.group,"loop_out",1);
		this.Buttons.Keypad8.setLed(HerculesSteel.LedState.on);
	}
	else
	{
		this.Buttons.Keypad10.setLed(HerculesSteel.LedState.off);
	}
};

HerculesSteel.Deck.prototype.keypad11Handler = function(value) {};
HerculesSteel.Deck.prototype.keypad12Handler = function(value) {};

HerculesSteel.Deck.prototype.previousHandler = function(value) {
	if(this.Buttons.Keypad1.state == HerculesSteel.ButtonState.pressed) {
		//Move hotcue 1 backwards
		var hotcue = engine.getValue(this.group, "hotcue_1_position");
		var newPosition = hotcue - 400;
		if(newPosition > 0) {
			engine.setValue(this.group, "hotcue_1_position", newPosition);
		}
	} else if(this.Buttons.Keypad2.state == HerculesSteel.ButtonState.pressed) {
		//Move hotcue 2 backwards
		var hotcue = engine.getValue(this.group, "hotcue_2_position");
		var newPosition = hotcue - 400;
		if(newPosition > 0) {
			engine.setValue(this.group, "hotcue_2_position", newPosition);
		}
	} else if(this.Buttons.Keypad3.state == HerculesSteel.ButtonState.pressed) {
		//Move hotcue 3 backwards
		var hotcue = engine.getValue(this.group, "hotcue_3_position");
		var newPosition = hotcue - 400;
		if(newPosition > 0) {
			engine.setValue(this.group, "hotcue_3_position", newPosition);
		}
	} else if(this.Buttons.Keypad4.state == HerculesSteel.ButtonState.pressed) {
		//Move loop-in backwards
		var loopIn = engine.getValue(this.group, "loop_start_position");
		var newPosition = loopIn - 400;
		if(newPosition > 0) {
			engine.setValue(this.group, "loop_start_position", newPosition);
		}
	} else if(this.Buttons.Keypad5.state == HerculesSteel.ButtonState.pressed) {
		//Move loop-out backwards
		var loopIn = engine.getValue(this.group, "loop_start_position");
		var loopOut = engine.getValue(this.group, "loop_end_position");
		var newPosition = loopOut - 400;
		if(newPosition > loopIn) {
			engine.setValue(this.group, "loop_end_position", newPosition);
		}
	} else {
		engine.setValue(this.group,"back",value);
	}
};

HerculesSteel.Deck.prototype.nextHandler = function(value) {
	//TODO: Fix movement of hotcues & loops out of track bounds
	if(this.Buttons.Keypad1.state == HerculesSteel.ButtonState.pressed) {
		//Move hotcue 1 forwards
		var hotcue = engine.getValue(this.group, "hotcue_1_position");
		var newPosition = hotcue + 400;
		if(hotcue != -1) {
			engine.setValue(this.group, "hotcue_1_position", newPosition);
		}
	} else if(this.Buttons.Keypad2.state == HerculesSteel.ButtonState.pressed) {
		//Move hotcue 2 forwards
		var hotcue = engine.getValue(this.group, "hotcue_2_position");
		var newPosition = hotcue + 400;
		if(hotcue != -1) {
			engine.setValue(this.group, "hotcue_2_position", newPosition);
		}
	} else if(this.Buttons.Keypad3.state == HerculesSteel.ButtonState.pressed) {
		//Move hotcue 3 forwards
		var hotcue = engine.getValue(this.group, "hotcue_3_position");
		var newPosition = hotcue + 400;
		if(hotcue != -1 > 0) {
			engine.setValue(this.group, "hotcue_3_position", newPosition);
		}
	} else if(this.Buttons.Keypad4.state == HerculesSteel.ButtonState.pressed) {
		//Move loop-in forwards
		var loopIn = engine.getValue(this.group, "loop_start_position");
		var loopOut = engine.getValue(this.group, "loop_end_position");
		var newPosition = loopIn + 400;
		if(newPosition < loopOut) {
			engine.setValue(this.group, "loop_start_position", newPosition);
		}
	} else if(this.Buttons.Keypad5.state == HerculesSteel.ButtonState.pressed) {
		//Move loop-out forwards
		var loopOut = engine.getValue(this.group, "loop_end_position");
		var newPosition = loopOut + 400;
		engine.setValue(this.group, "loop_end_position", newPosition);
	} else {
		engine.setValue(this.group,"fwd",value);
	}
};

HerculesSteel.Deck.prototype.playPauseHandler = function(value) {
};

HerculesSteel.Deck.prototype.cueHandler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		//var position = engine.getValue(this.group,"playposition") * engine.getValue(this.group, "duration") * engine.getValue(this.group, "duration");
		//var atCuePoint = engine.getValue(this.group,"cue_point") == position;
		//print("at cue: " + atCuePoint + ", playposition: " + position);
		engine.setValue(this.group,"cue_default",1);
		this.playing = false;
		this.Buttons.PlayPause.setLed(HerculesSteel.LedState.off);
		//if(atCuePoint) {
		this.cuePlaying = true;
		this.Buttons.Cue.setLed(HerculesSteel.LedState.on);
		//}
	} else {
		engine.setValue(this.group,"cue_default",0);
		if(this.cuePlaying) {
			this.cuePlaying = false;
		}
		this.Buttons.Cue.setLed(HerculesSteel.LedState.off);
	}
};

HerculesSteel.Deck.prototype.killHighHandler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		var filterStatus = engine.getValue(this.group, "filterHighKill");
		if(filterStatus) {
			engine.setValue(this.group, "filterHighKill", 0);
			this.Buttons.KillHigh.setLed(HerculesSteel.LedState.off);
		} else {
			engine.setValue(this.group, "filterHighKill", 1);
			this.Buttons.KillHigh.setLed(HerculesSteel.LedState.on);
		}
	}
};

HerculesSteel.Deck.prototype.killMidHandler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		var filterStatus = engine.getValue(this.group, "filterMidKill");
		if(filterStatus) {
			engine.setValue(this.group, "filterMidKill", 0);
			this.Buttons.KillMid.setLed(HerculesSteel.LedState.off);
		} else {
			engine.setValue(this.group, "filterMidKill", 1);
			this.Buttons.KillMid.setLed(HerculesSteel.LedState.on);
		}
	}
};

HerculesSteel.Deck.prototype.killLowHandler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		var filterStatus = engine.getValue(this.group, "filterLowKill");
		if(filterStatus) {
			engine.setValue(this.group, "filterLowKill", 0);
			this.Buttons.KillLow.setLed(HerculesSteel.LedState.off);
		} else {
			engine.setValue(this.group, "filterLowKill", 1);
			this.Buttons.KillLow.setLed(HerculesSteel.LedState.on);
		}
	}
};

HerculesSteel.Deck.prototype.loadHandler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		engine.setValue(this.group, "LoadSelectedTrack", 1);
	}
};

HerculesSteel.Deck.prototype.cueSelectHandler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		var filterStatus = engine.getValue(this.group, "pfl");
		if(filterStatus) {
			engine.setValue(this.group, "pfl", 0);
			this.Buttons.CueSelect.setLed(HerculesSteel.LedState.off);
		} else {
			engine.setValue(this.group, "pfl", 1);
			this.Buttons.CueSelect.setLed(HerculesSteel.LedState.on);
		}
	}
};

HerculesSteel.Deck.prototype.gainHandler = function(value) {
	this.Controls.Gain.setValue(this.group, value);
};

HerculesSteel.Deck.prototype.trebleHandler = function(value) {
	if(HerculesSteel.shiftMode) {
		//Flanger
		HerculesSteel.Controls.FlangerDepth.setValue("[Flanger]", value);
	} else if(this.shiftMode) {
		//Soft set
		this.Controls.Treble.softMode = true;
		this.Controls.Treble.setValue(this.group, value);
		this.Controls.Treble.softMode = false;
	} else {
		this.Controls.Treble.setValue(this.group, value);
	}
};

HerculesSteel.Deck.prototype.mediumHandler = function(value) {
	if(HerculesSteel.shiftMode) {
		//Flanger
		HerculesSteel.Controls.FlangerDelay.setValue("[Flanger]", value);
	} else if(this.shiftMode) {
		//Soft set
		this.Controls.Medium.softMode = true;
		this.Controls.Medium.setValue(this.group, value);
		this.Controls.Medium.softMode = false;
	} else {
		this.Controls.Medium.setValue(this.group, value);
	}
};

HerculesSteel.Deck.prototype.bassHandler = function(value) {
	if(HerculesSteel.shiftMode) {
		//Flanger
		HerculesSteel.Controls.FlangerPeriod.setValue("[Flanger]", value);
	} else if(this.shiftMode) {
		//Soft set
		this.Controls.Bass.softMode = true;
		this.Controls.Bass.setValue(this.group, value);
		this.Controls.Bass.softMode = false;
	} else {
		this.Controls.Bass.setValue(this.group, value);
	}
};

HerculesSteel.Deck.prototype.volHandler = function(value) {
	this.Controls.Vol.setValue(this.group, value);
};

HerculesSteel.Deck.prototype.pitchHandler = function(value) {
	this.Controls.Pitch.setValue(this.group, value);
};

HerculesSteel.Deck.prototype.shiftHandler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		this.shiftMode = true;
	} else {
		this.shiftMode = false;
	}
};

HerculesSteel.Deck.prototype.stopHandler = function(value) {
	if(value == HerculesSteel.ButtonState.pressed) {
		engine.setValue(this.group, "cue_default", 0);
		engine.setValue(this.group, "play", 0);
		engine.setValue(this.group, "start", 0);
	}
};

HerculesSteel.Deck.prototype.addButton = HerculesSteel.addButton;

HerculesSteel.Decks = {"Left":new HerculesSteel.Deck(1,"[Channel1]"), "Right":new HerculesSteel.Deck(2,"[Channel2]")};
HerculesSteel.GroupToDeck = {"[Channel1]":"Left", "[Channel2]":"Right"};

HerculesSteel.GetDeck = function(group) {
	try {
		return HerculesSteel.Decks[HerculesSteel.GroupToDeck[group]];
	} catch(ex) {
		return null;
	}
};

HerculesSteel.Decks.Left.addButton("Keypad1", new HerculesSteel.Button(0x01), "keypad1Handler");
HerculesSteel.Decks.Left.addButton("Keypad2", new HerculesSteel.Button(0x02), "keypad2Handler");
HerculesSteel.Decks.Left.addButton("Keypad3", new HerculesSteel.Button(0x03), "keypad3Handler");
HerculesSteel.Decks.Left.addButton("Keypad4", new HerculesSteel.Button(0x04), "keypad4Handler");
HerculesSteel.Decks.Left.addButton("Keypad5", new HerculesSteel.Button(0x05), "keypad5Handler");
HerculesSteel.Decks.Left.addButton("Keypad6", new HerculesSteel.Button(0x06), "keypad6Handler");
HerculesSteel.Decks.Left.addButton("Keypad7", new HerculesSteel.Button(0x64), "keypad7Handler");
HerculesSteel.Decks.Left.addButton("Keypad8", new HerculesSteel.Button(0x65), "keypad8Handler");
HerculesSteel.Decks.Left.addButton("Keypad9", new HerculesSteel.Button(0x66), "keypad9Handler");
HerculesSteel.Decks.Left.addButton("Keypad10", new HerculesSteel.Button(0x67), "keypad10Handler");
HerculesSteel.Decks.Left.addButton("Keypad11", new HerculesSteel.Button(0x68), "keypad11Handler");
HerculesSteel.Decks.Left.addButton("Keypad12", new HerculesSteel.Button(0x69), "keypad12Handler");
HerculesSteel.Decks.Left.addButton("Sync", new HerculesSteel.Button(0x07, 0x37), "syncHandler");
HerculesSteel.Decks.Left.addButton("BeatLock", new HerculesSteel.Button(0x08, 0x38), null);
HerculesSteel.Decks.Left.addButton("Previous", new HerculesSteel.Button(0x09), "previousHandler");
HerculesSteel.Decks.Left.addButton("Next", new HerculesSteel.Button(0x0A), "nextHandler");
HerculesSteel.Decks.Left.addButton("PlayPause", new HerculesSteel.Button(0x0B, 0x3B), "playPauseHandler");
HerculesSteel.Decks.Left.addButton("Cue", new HerculesSteel.Button(0x0C, 0x3C), "cueHandler");
HerculesSteel.Decks.Left.addButton("Shift",  new HerculesSteel.Button(0x0D), "shiftHandler");
HerculesSteel.Decks.Left.addButton("KillHigh", new HerculesSteel.Button(0x0E), "killHighHandler");
HerculesSteel.Decks.Left.addButton("KillMid", new HerculesSteel.Button(0x0F), "killMidHandler");
HerculesSteel.Decks.Left.addButton("KillLow", new HerculesSteel.Button(0x10), "killLowHandler");
HerculesSteel.Decks.Left.addButton("PitchReset", new HerculesSteel.Button(0x11, 0x41), "pitchResetHandler");
HerculesSteel.Decks.Left.addButton("Load", new HerculesSteel.Button(0x12), "loadHandler");
HerculesSteel.Decks.Left.addButton("Source", new HerculesSteel.Button(0x13, 0x43), null);
HerculesSteel.Decks.Left.addButton("CueSelect", new HerculesSteel.Button(0x14, 0x44), "cueSelectHandler");
HerculesSteel.Decks.Left.addButton("Stop",  new HerculesSteel.Button(0x0D), "stopHandler");

HerculesSteel.Decks.Left.Controls = {
	"Gain" : new HerculesSteel.Control("pregain", false),
	"Treble" : new HerculesSteel.Control("filterHigh", false),
	"Medium" : new HerculesSteel.Control("filterMid", false),
	"Bass" : new HerculesSteel.Control("filterLow", false),
	"Vol" : new HerculesSteel.Control("volume", false),
	"Pitch" : new HerculesSteel.Control("rate", false)
};
HerculesSteel.Decks.Left.Controls.Gain.minOutput = 0.0;
HerculesSteel.Decks.Left.Controls.Gain.midOutput = 1.0;
HerculesSteel.Decks.Left.Controls.Gain.maxOutput = 4.0;
HerculesSteel.Decks.Left.Controls.Treble.minOutput = 0.0;
HerculesSteel.Decks.Left.Controls.Treble.midOutput = 1.0;
HerculesSteel.Decks.Left.Controls.Treble.maxOutput = 4.0;
HerculesSteel.Decks.Left.Controls.Medium.minOutput = 0.0;
HerculesSteel.Decks.Left.Controls.Medium.midOutput = 1.0;
HerculesSteel.Decks.Left.Controls.Medium.maxOutput = 4.0;
HerculesSteel.Decks.Left.Controls.Bass.minOutput = 0.0;
HerculesSteel.Decks.Left.Controls.Bass.midOutput = 1.0;
HerculesSteel.Decks.Left.Controls.Bass.maxOutput = 4.0;
HerculesSteel.Decks.Left.Controls.Vol.minOutput = 0.0;
HerculesSteel.Decks.Left.Controls.Vol.midOutput = 0.4;
HerculesSteel.Decks.Left.Controls.Vol.maxOutput = 1.0;
HerculesSteel.Decks.Left.Controls.Pitch.midInput = 0x3F;

HerculesSteel.Decks.Right.addButton("Keypad1",  new HerculesSteel.Button(0x19), "keypad1Handler");
HerculesSteel.Decks.Right.addButton("Keypad2", new HerculesSteel.Button(0x1A), "keypad2Handler");
HerculesSteel.Decks.Right.addButton("Keypad3", new HerculesSteel.Button(0x1B), "keypad3Handler");
HerculesSteel.Decks.Right.addButton("Keypad4", new HerculesSteel.Button(0x1C), "keypad4Handler");
HerculesSteel.Decks.Right.addButton("Keypad5", new HerculesSteel.Button(0x1D), "keypad5Handler");
HerculesSteel.Decks.Right.addButton("Keypad6", new HerculesSteel.Button(0x1E), "keypad6Handler");
HerculesSteel.Decks.Right.addButton("Keypad7", new HerculesSteel.Button(0x6A), "keypad7Handler");
HerculesSteel.Decks.Right.addButton("Keypad8", new HerculesSteel.Button(0x6B), "keypad8Handler");
HerculesSteel.Decks.Right.addButton("Keypad9", new HerculesSteel.Button(0x6C), "keypad9Handler");
HerculesSteel.Decks.Right.addButton("Keypad10", new HerculesSteel.Button(0x6D), "keypad10Handler");
HerculesSteel.Decks.Right.addButton("Keypad11", new HerculesSteel.Button(0x6E), "keypad11Handler");
HerculesSteel.Decks.Right.addButton("Keypad12", new HerculesSteel.Button(0x6F), "keypad12Handler");
HerculesSteel.Decks.Right.addButton("Sync", new HerculesSteel.Button(0x1F, 0x5F), "syncHandler");
HerculesSteel.Decks.Right.addButton("BeatLock", new HerculesSteel.Button(0x15, 0x60), null);
HerculesSteel.Decks.Right.addButton("Previous", new HerculesSteel.Button(0x21), "previousHandler");
HerculesSteel.Decks.Right.addButton("Next", new HerculesSteel.Button(0x22), "nextHandler");
HerculesSteel.Decks.Right.addButton("PlayPause", new HerculesSteel.Button(0x23, 0x53), "playPauseHandler");
HerculesSteel.Decks.Right.addButton("Cue", new HerculesSteel.Button(0x24, 0x54), "cueHandler");
HerculesSteel.Decks.Right.addButton("Shift", new HerculesSteel.Button(0x25), "shiftHandler");
HerculesSteel.Decks.Right.addButton("KillHigh", new HerculesSteel.Button(0x26), "killHighHandler");
HerculesSteel.Decks.Right.addButton("KillMid", new HerculesSteel.Button(0x27), "killMidHandler");
HerculesSteel.Decks.Right.addButton("KillLow", new HerculesSteel.Button(0x28), "killLowHandler");
HerculesSteel.Decks.Right.addButton("PitchReset", new HerculesSteel.Button(0x20, 0x55), "pitchResetHandler");
HerculesSteel.Decks.Right.addButton("Load", new HerculesSteel.Button(0x16), "loadHandler");
HerculesSteel.Decks.Right.addButton("Source", new HerculesSteel.Button(0x17, 0x57), null);
HerculesSteel.Decks.Right.addButton("CueSelect", new HerculesSteel.Button(0x18, 0x58), "cueSelectHandler");
HerculesSteel.Decks.Right.addButton("Stop", new HerculesSteel.Button(0x25), "stopHandler");

HerculesSteel.Decks.Right.Controls = {
	"Gain" : new HerculesSteel.Control("pregain", false),
	"Treble" : new HerculesSteel.Control("filterHigh", false),
	"Medium" : new HerculesSteel.Control("filterMid", false),
	"Bass" : new HerculesSteel.Control("filterLow", false),
	"Vol" : new HerculesSteel.Control("volume", false),
	"Pitch" : new HerculesSteel.Control("rate", false)
};
HerculesSteel.Decks.Right.Controls.Gain.minOutput = 0.0;
HerculesSteel.Decks.Right.Controls.Gain.midOutput = 1.0;
HerculesSteel.Decks.Right.Controls.Gain.maxOutput = 4.0;
HerculesSteel.Decks.Right.Controls.Treble.minOutput = 0.0;
HerculesSteel.Decks.Right.Controls.Treble.midOutput = 1.0;
HerculesSteel.Decks.Right.Controls.Treble.maxOutput = 4.0;
HerculesSteel.Decks.Right.Controls.Medium.minOutput = 0.0;
HerculesSteel.Decks.Right.Controls.Medium.midOutput = 1.0;
HerculesSteel.Decks.Right.Controls.Medium.maxOutput = 4.0;
HerculesSteel.Decks.Right.Controls.Bass.minOutput = 0.0;
HerculesSteel.Decks.Right.Controls.Bass.midOutput = 1.0;
HerculesSteel.Decks.Right.Controls.Bass.maxOutput = 4.0;
HerculesSteel.Decks.Right.Controls.Vol.minOutput = 0.0;
HerculesSteel.Decks.Right.Controls.Vol.midOutput = 0.4;
HerculesSteel.Decks.Right.Controls.Vol.maxOutput = 1.0;
HerculesSteel.Decks.Right.Controls.Pitch.midInput = 0x3F;


//Mapping functions
HerculesSteel.volume = function(channel, control, value, status, group) {
	HerculesSteel.volumeHandler(value);
};

HerculesSteel.balance = function(channel, control, value, status, group) {
	HerculesSteel.balanceHandler(value);
};

HerculesSteel.crossFader = function(channel, control, value, status, group) {
	HerculesSteel.crossFaderHandler(value);
};

HerculesSteel.headPhoneMix = function(channel, control, value, status, group) {
	HerculesSteel.headPhoneMixHandler(value);
};

HerculesSteel.up = function (channel, control, value, status, group) {
	HerculesSteel.Buttons.Up.handleEvent(value);
};

HerculesSteel.down = function (channel, control, value, status, group) {
	HerculesSteel.Buttons.Down.handleEvent(value);
};

HerculesSteel.left = function (channel, control, value, status, group) {
	HerculesSteel.Buttons.Left.handleEvent(value);
};

HerculesSteel.right = function (channel, control, value, status, group) {
	HerculesSteel.Buttons.Right.handleEvent(value);
};

HerculesSteel.scratch = function (channel, control, value, status, group) {
	HerculesSteel.Buttons.Scratch.handleEvent(value);
};

HerculesSteel.gain = function(channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.gainHandler(value);
};

HerculesSteel.rate = function(channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.pitchHandler(value);
};

HerculesSteel.treble = function(channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.trebleHandler(value);
};

HerculesSteel.medium = function(channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.mediumHandler(value);
};

HerculesSteel.bass = function(channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.bassHandler(value);
};

HerculesSteel.deckVolume = function(channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.volHandler(value);
};

HerculesSteel.jog_wheel = function (channel, control, value, status, group) {
	// 7F > 40: CCW Slow > Fast - 127 > 64
	// 01 > 3F: CW Slow > Fast - 0 > 63
	var jogValue = value >=0x40 ? value - 0x80 : value; // -64 to +63, - = CCW, + = CW
	HerculesSteel.GetDeck(group).jogMove(jogValue);
};

HerculesSteel.cue = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Cue.handleEvent(value);
};

HerculesSteel.beatSync = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Sync.handleEvent(value);
};

HerculesSteel.rateReset = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.PitchReset.handleEvent(value);
};

HerculesSteel.play = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.PlayPause.handleEvent(value);
};

HerculesSteel.stop = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Stop.handleEvent(value);
};

HerculesSteel.shift = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Shift.handleEvent(value);
};

HerculesSteel.keypad1 = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Keypad1.handleEvent(value);
};

HerculesSteel.keypad2 = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Keypad2.handleEvent(value);
};

HerculesSteel.keypad3 = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Keypad3.handleEvent(value);
};

HerculesSteel.keypad4 = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Keypad4.handleEvent(value);
};

HerculesSteel.keypad5 = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Keypad5.handleEvent(value);
};

HerculesSteel.keypad6 = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Keypad6.handleEvent(value);
};

HerculesSteel.keypad7 = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Keypad7.handleEvent(value);
};

HerculesSteel.keypad8 = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Keypad8.handleEvent(value);
};

HerculesSteel.keypad9 = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Keypad9.handleEvent(value);
};

HerculesSteel.keypad10 = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Keypad10.handleEvent(value);
};

HerculesSteel.keypad11 = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Keypad11.handleEvent(value);
};

HerculesSteel.keypad12 = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Keypad12.handleEvent(value);
};

HerculesSteel.next = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Next.handleEvent(value);
};

HerculesSteel.previous = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Previous.handleEvent(value);
};

HerculesSteel.load = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.Load.handleEvent(value);
};

HerculesSteel.cueSelect = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.CueSelect.handleEvent(value);
};

HerculesSteel.killLow = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.KillLow.handleEvent(value);
};


HerculesSteel.killMid = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.KillMid.handleEvent(value);
};


HerculesSteel.killHigh = function (channel, control, value, status, group) {
	var deck = HerculesSteel.GetDeck(group);
	deck.Buttons.KillHigh.handleEvent(value);
};

HerculesSteel.init = function (id) {    // called when the MIDI device is opened & set up

	HerculesSteel.LedsOFF();
	engine.connectControl("[Channel1]","rate","HerculesSteel.rateChange");
	engine.connectControl("[Channel2]","rate","HerculesSteel.rateChange");
	print ("HerculesSteel id: \""+id+"\" initialized.");
};

HerculesSteel.shutdown = function() {
	HerculesSteel.LedsOFF();
	print ("HerculesSteel shutdown.");
};

HerculesSteel.LedsON = function() {
	var time = 20;

	print ("HerculesSteel LEDS ON (1/2).");
	for (i=0;i<0x29;i++)
	{
		//TODO: remove timers when alsa midi work properly.
		//midi.sendShortMsg(0xB0, i, 0x7F);
		engine.beginTimer(time, "midi.sendShortMsg(0xB0, '" + i + "', 0x7F);", true);
		time = time + 5;
	}

	// Properly, it would be like this :
	HerculesSteel.Buttons.Scratch.setLed(HerculesSteel.LedState.on);

	print ("HerculesSteel LEDS ON (2/2).");
	for (i=0x64;i<0x6f+1;i++)
	{
		//TODO: remove timers when alsa midi work properly.
		//midi.sendShortMsg(0xB0, i, 0x7F);
		engine.beginTimer(time, "midi.sendShortMsg(0xB0, '" + i + "', 0x7F);", true);
		time = time + 5;
	}
	// Some buttons stays OFF : rewind A/B, forward A/B, Load Deck A, Load Deck B, Up, Down, Apply Ctrl. on
}

HerculesSteel.LedsOFF = function() {
	HerculesSteel.Buttons.Scratch.setLed(HerculesSteel.LedState.off);
	//TODO: remove timers when alsa midi work properly.
	var button;
	var time = 20;
	print("switch off Left deck LEDs");
	for (var key in HerculesSteel.Decks.Left.Buttons) {
		engine.beginTimer(time, "HerculesSteel.Decks.Left.Buttons['" + key + "'].setLed(HerculesSteel.LedState.off)", true);
		time = time + 5;
	}
	print("switch off Right deck LEDs");
	for (var key in HerculesSteel.Decks.Right.Buttons) {
		engine.beginTimer(time, "HerculesSteel.Decks.Right.Buttons['" + key + "'].setLed(HerculesSteel.LedState.off)", true);
		time = time + 5;
	}
}

//Rate change event handler to reset sync and reset leds
//TODO: Need some way to check sync state here.
HerculesSteel.rateChange = function (value, group) {
	if (HerculesSteel.Decks.Left.Buttons.Sync.state != HerculesSteel.ButtonState.pressed) {
		HerculesSteel.Decks.Left.Buttons.Sync.setLed(HerculesSteel.LedState.off);
	}
	if (HerculesSteel.Decks.Right.Buttons.Sync.state != HerculesSteel.ButtonState.pressed) {
		engine.beginTimer(25, "HerculesSteel.Decks.Right.Buttons.Sync.setLed(HerculesSteel.LedState.off)", true);
	}
	if (value != 0.0) {
		var deck = HerculesSteel.GetDeck(group);
		engine.beginTimer(30, "HerculesSteel.GetDeck('" + group + "').Buttons.PitchReset.setLed(HerculesSteel.LedState.off)", true);
	}
};

