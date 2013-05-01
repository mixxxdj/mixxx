DenonDNSC2000 = new function() {}

DenonDNSC2000.Deck = function (deckNumber, group) {
   this.deckNumber = deckNumber;
   this.group = group;
   this.scratchMode = false;
   this.controlPressed = -1;
}

DenonDNSC2000.init = function (id) {
	engine.setValue("[Master]", "num_decks", 4);
	var leds = [0x11,0x13,0x15,0x17,0x19,0x1B,0x1D,0x20,/* cues */
				0x24,0x40,0x2B,/* loops */
				0x27,0x26,/* play,cue */
				0x08,0x09,/* key lock, sync*/
				0x5A,0x5B,/* flanger,scratch mode */
				0x5D,0x5E,0x5F/* depth, delay, lfo */
				];
	for(var index = 0, count = leds.length;index < count; index ++) {
		midi.sendShortMsg(0xB0,0x4B,leds[index]);
		midi.sendShortMsg(0xB1,0x4B,leds[index]);
		midi.sendShortMsg(0xB2,0x4B,leds[index]);
		midi.sendShortMsg(0xB3,0x4B,leds[index]);
	}
}

DenonDNSC2000.shutdown = function (id) {
	DenonDNSC2000.init(id);
}

DenonDNSC2000.getDeckByGroup = function(group) {
	for(var index = 0, count = decks.length;index < count; index++) {
		if(decks[index].group == group)
			return decks[index];
	}
   return null;
}

DenonDNSC2000.shift = function(midino, control, value, status, group) {
	shiftPressed = ((status & 0xF0) == 0x90);
	if(!shiftPressed)
		engine.setValue(group, 'reverse', 0);
}

DenonDNSC2000.changeDeck = function(midino, control, value, status, group) {
	DenonDNSC2000.handleLeds(group);
}

DenonDNSC2000.newValue = function(currentVal,min,max,increment,ticksCount) {
	var interval = (max - min) / 24;
	var newVal = 0;
	if(increment)
		newVal = Math.min(max,currentVal + interval);
	else
		newVal = Math.max(min,currentVal - interval);
	return newVal;
}

DenonDNSC2000.flanger = function (midino, control, value, status, group) {
	DenonDNSC2000.toggleBinaryValue(group,'flanger');
	engine.beginTimer(100, 'DenonDNSC2000.handleFlangerLed("'+group+'")', true);
}

DenonDNSC2000.changeDepth = function (midino, control, value, status, group) {
	engine.setValue(group, 'lfoDepth', DenonDNSC2000.newValue(engine.getValue(group, 'lfoDepth'),0,1,(value == 0x00),24));
}

DenonDNSC2000.changeDelay = function (midino, control, value, status, group) {
	engine.setValue(group, 'lfoDelay', DenonDNSC2000.newValue(engine.getValue(group, 'lfoDelay'),50,10000,(value == 0x00),24));
}

DenonDNSC2000.changeLFO = function (midino, control, value, status, group) {
	engine.setValue(group, 'lfoPeriod', DenonDNSC2000.newValue(engine.getValue(group, 'lfoPeriod'),50000,2000000,(value == 0x00),24));
}

DenonDNSC2000.resetDepth = function (midino, control, value, status, group) {
	engine.setValue(group, 'lfoDepth', 0.5);
}

DenonDNSC2000.resetDelay = function (midino, control, value, status, group) {
	engine.setValue(group, 'lfoDelay', 50 + (10000 - 50) / 2);
}

DenonDNSC2000.resetLFO = function (midino, control, value, status, group) {
	engine.setValue(group, 'lfoPeriod', 50000 + (2000000 - 50000) / 2);
}

DenonDNSC2000.selectTrack = function (midino, control, value, status, group) {
	if(value == 0x00)
		engine.setValue('[Playlist]', 'SelectNextTrack', 1);
	else
		engine.setValue('[Playlist]', 'SelectPrevTrack', 1);
}

DenonDNSC2000.loadSelectedTrack = function (midino, control, value, status, group) {
	engine.setValue(group, 'LoadSelectedTrack', 1);
	engine.beginTimer(1500, 'DenonDNSC2000.handleLeds("'+group+'")', true);
}

DenonDNSC2000.loopOrHotcues = function (midino, control, value, status, group) {
	var deck = DenonDNSC2000.getDeckByGroup(group);
	if ((status & 0xF0) == 0x80)
		deck.controlPressed = -1;
	else {
		deck.controlPressed = control;
		switch (control) {
			// hotcue
			case 0x17:
				DenonDNSC2000.hotcue(1,group,value,shiftPressed);
			break;
			case 0x18:
				DenonDNSC2000.hotcue(2,group,value,shiftPressed);
			break;
			case 0x19:
				DenonDNSC2000.hotcue(3,group,value,shiftPressed);
			break;
			case 0x20:
				DenonDNSC2000.hotcue(4,group,value,shiftPressed);
			break;
			case 0x21:
				DenonDNSC2000.hotcue(5,group,value,shiftPressed);
			break;
			case 0x22:
				DenonDNSC2000.hotcue(6,group,value,shiftPressed);
			break;
			case 0x23:
				DenonDNSC2000.hotcue(7,group,value,shiftPressed);
			break;
			case 0x24:
				DenonDNSC2000.hotcue(8,group,value,shiftPressed);
			break;
			// loop
			case 0x37:
				DenonDNSC2000.loopIn(group,value,shiftPressed);
			break;
			case 0x39:
				DenonDNSC2000.loopOut(group,value,shiftPressed);
			break;
			case 0x1D:
				DenonDNSC2000.reloop(group,value,shiftPressed);
			break;
		}
	}
};

DenonDNSC2000.hotcue = function(cueIndex, group, value, shift) {
	if(!shift) {
		engine.setValue(group, 'hotcue_' + cueIndex + '_activate', 1);
	}
	else {
		if(engine.getValue(group, 'hotcue_' + cueIndex + '_enabled') == 0) {
			var samplesPerBeat = DenonDNSC2000.samplesPerBeat(group);
			var positionInBeats = (engine.getValue(group,'playposition') * engine.getValue(group,'track_samples')) / samplesPerBeat;
			if((positionInBeats - Math.floor(positionInBeats)) > 0.5)
				positionInBeats = Math.floor(0.5 + positionInBeats) * samplesPerBeat;
			else
				positionInBeats = Math.floor(positionInBeats) * samplesPerBeat;
			positionInBeats = Math.floor(0.5 + positionInBeats);
			positionInBeats = Math.max(0,positionInBeats - positionInBeats % 2);
			engine.setValue(group, 'hotcue_' + cueIndex + '_activate', 1);
			engine.setValue(group, 'hotcue_' + cueIndex + '_position',positionInBeats);
		}
		else
		engine.setValue(group, 'hotcue_' + cueIndex + '_clear',1);
	}
	engine.beginTimer(100, 'DenonDNSC2000.handleLoopAndHotcuesLeds("'+group+'")', true);
}

DenonDNSC2000.loopIn = function(group, value, shift) {
	if(shiftPressed) {
		engine.setValue(group, 'loop_start_position', -1);
	}
	else
		engine.setValue(group, 'loop_in', 1);
	engine.beginTimer(100, 'DenonDNSC2000.handleLoopAndHotcuesLeds("'+group+'")', true);
}

DenonDNSC2000.loopOut = function(group, value, shift) {
	if(shiftPressed) {
		engine.setValue(group, 'loop_end_position', -1);
		if(engine.getValue(group,'loop_enabled'))
			engine.setValue(group, 'reloop_exit', 1);
	}
	else
		engine.setValue(group, 'loop_out', 1);
	engine.beginTimer(100, 'DenonDNSC2000.handleLoopAndHotcuesLeds("'+group+'")', true);
}

DenonDNSC2000.reloop = function(group, value, shift) {
	var loopInPosition = engine.getValue(group,'loop_start_position');
	var loopOutPosition = engine.getValue(group,'loop_end_position');
	if(loopInPosition != -1 && loopOutPosition!=-1) {
		engine.setValue(group, 'reloop_exit', 1);
	}
	else {
		var samplesPerBeat = DenonDNSC2000.samplesPerBeat(group);
		loopInPosition = engine.getValue(group,'playposition') * engine.getValue(group,'track_samples');
		if(shiftPressed) {
			var loopInPositionInBeats = loopInPosition / samplesPerBeat;
			if((loopInPositionInBeats - Math.floor(loopInPositionInBeats)) > 0.5)
				loopInPosition = Math.floor(0.5 + loopInPositionInBeats) * samplesPerBeat;
			else
				loopInPosition = Math.floor(loopInPositionInBeats) * samplesPerBeat;
		}
		else
			loopInPosition = engine.getValue(group,'playposition') * engine.getValue(group,'track_samples');
		loopInPosition = Math.floor(0.5 + loopInPosition);
		loopInPosition = Math.max(0,loopInPosition - loopInPosition % 2);
		var loopOutPosition = loopInPosition + Math.floor(0.5 + samplesPerBeat);
		loopOutPosition = Math.max(0,loopOutPosition - loopOutPosition % 2);
		if(loopInPosition + samplesPerBeat < engine.getValue(group,'track_samples')) {
			engine.setValue(group, 'loop_start_position', loopInPosition);
			engine.setValue(group, 'loop_end_position', loopOutPosition);
			engine.setValue(group, 'reloop_exit', 1);
		}
	}
	engine.beginTimer(100, 'DenonDNSC2000.handleLoopAndHotcuesLeds("'+group+'")', true);
}

DenonDNSC2000.resizeLoop = function(midino, control, value, status, group) {
	var increment = true;
	if(value == 0x7F)
		increment = false;
	var loopInPosition = engine.getValue(group,'loop_start_position');
	var loopOutPosition = engine.getValue(group,'loop_end_position');
	if(loopInPosition != -1 && loopOutPosition!= -1) {
		var samplesPerBeat = DenonDNSC2000.samplesPerBeat(group);
		var deltaSamples = loopOutPosition - loopInPosition;
		var newLoopOutPosition = loopOutPosition;
		var loopLengthes = [
						Math.floor(0.5 + samplesPerBeat * 1 / 32),
						Math.floor(0.5 + samplesPerBeat * 1 / 16),
						Math.floor(0.5 + samplesPerBeat * 1 / 8),
						Math.floor(0.5 + samplesPerBeat * 1 / 4),
						Math.floor(0.5 + samplesPerBeat * 1 / 2),
						Math.floor(0.5 + samplesPerBeat),
						Math.floor(0.5 + samplesPerBeat * 2),
						Math.floor(0.5 + samplesPerBeat * 4),
						Math.floor(0.5 + samplesPerBeat * 8),
						Math.floor(0.5 + samplesPerBeat * 16),
						Math.floor(0.5 + samplesPerBeat * 32)
						];

		if(increment) {
			for(var index = 0, count = loopLengthes.length;index < count; index++) {
				if(deltaSamples < loopLengthes[index]) {
					newLoopOutPosition = loopInPosition + loopLengthes[index];
					break;
				}
			}
		}
		else {
			for(var index = loopLengthes.length-1;index >= 0; index--) {
				if(deltaSamples > loopLengthes[index]) {
					newLoopOutPosition = loopInPosition + loopLengthes[index];
					break;
				}
			}
		}
		newLoopOutPosition = Math.max(0,newLoopOutPosition - newLoopOutPosition % 2);
		engine.setValue(group, 'loop_end_position', newLoopOutPosition);
	}
	engine.beginTimer(100, 'DenonDNSC2000.handleLoopAndHotcuesLeds("'+group+'")', true);
}

DenonDNSC2000.moveLoopLeft = function(midino, control, value, status, group) {
	var samplesPerBeat = Math.floor(0.5 + DenonDNSC2000.samplesPerBeat(group));
	var loopInPosition = engine.getValue(group,'loop_start_position');
	var loopOutPosition = engine.getValue(group,'loop_end_position');
	if(loopInPosition != -1 && loopOutPosition != -1 && loopInPosition > samplesPerBeat) {
		loopInPosition = loopInPosition - samplesPerBeat;
		loopInPosition = Math.max(0,loopInPosition - loopInPosition % 2);
		loopOutPosition = loopOutPosition - samplesPerBeat;
		loopOutPosition = Math.max(0,loopOutPosition - loopOutPosition % 2);
		engine.setValue(group, 'loop_start_position', loopInPosition);
		engine.setValue(group, 'loop_end_position', loopOutPosition);
	}
}

DenonDNSC2000.moveLoopRight = function(midino, control, value, status, group) {
	var samplesPerBeat = Math.floor(0.5 + DenonDNSC2000.samplesPerBeat(group));
	var loopInPosition = engine.getValue(group,'loop_start_position');
	var loopOutPosition = engine.getValue(group,'loop_end_position');
	if(loopInPosition != -1 && loopOutPosition != -1 && loopOutPosition + samplesPerBeat < engine.getValue(group,'track_samples')) {
		loopInPosition = loopInPosition + samplesPerBeat;
		loopInPosition = Math.max(0,loopInPosition - loopInPosition % 2);
		loopOutPosition = loopOutPosition + samplesPerBeat;
		loopOutPosition = Math.max(0,loopOutPosition - loopOutPosition % 2);
		engine.setValue(group, 'loop_start_position', loopInPosition);
		engine.setValue(group, 'loop_end_position', loopOutPosition);
	}
}

DenonDNSC2000.play = function (midino, control, value, status, group) {
	if(shiftPressed) {
		if ((status & 0xF0) == 0x90 && engine.getValue(group, 'play') == 1)
			engine.setValue(group, 'reverse', 1);
		else
		if ((status & 0xF0) == 0x80 && engine.getValue(group, 'play') == 1)
			engine.setValue(group, 'reverse', 0);
	}
	else {
		if((status & 0xF0) == 0x90) {
			DenonDNSC2000.toggleBinaryValue(group,'play');
		}
	}
	engine.beginTimer(100, 'DenonDNSC2000.handlePlayLed("'+group+'")', true);
}

DenonDNSC2000.cue = function (midino, control, value, status, group) {
	var ledChannel = DenonDNSC2000.getLedChannelByGroup(group);
	if ((status & 0xF0) == 0x90) {
		engine.setValue(group, 'cue_default', 1);
		midi.sendShortMsg(ledChannel,0x4A,0x26);
	}
	else
	if ((status & 0xF0) == 0x80) {
		engine.setValue(group, 'cue_default', 0);
		midi.sendShortMsg(ledChannel,0x4B,0x26);
	}
}

DenonDNSC2000.keyLock = function (midino, control, value, status, group) {
	if ((status & 0xF0) == 0x90) {
		DenonDNSC2000.toggleBinaryValue(group,'keylock');
		engine.beginTimer(100, 'DenonDNSC2000.handleKeyLockLed("'+group+'")', true);
	}
}

DenonDNSC2000.beatSync = function (midino, control, value, status, group) {
	if ((status & 0xF0) == 0x90) {
		DenonDNSC2000.toggleBinaryValue(group,'beatsync');
		engine.beginTimer(100, 'DenonDNSC2000.handleBeatSyncLed("'+group+'")', true);
	}
}

DenonDNSC2000.pitchBend = function (midino, control, value, status, group) {
	switch(control) {
		case 0x0D:
		if ((status & 0xF0) == 0x90)
			engine.setValue(group, 'rate_temp_down', 1);
		else
			engine.setValue(group, 'rate_temp_down', 0);
		break;
		case 0x0C:
		if ((status & 0xF0) == 0x90)
			engine.setValue(group, 'rate_temp_up', 1);
		else
			engine.setValue(group, 'rate_temp_up', 0);
		break;
	}
}

DenonDNSC2000.jog = function (midino, control, value, status, group) {
	var deck = DenonDNSC2000.getDeckByGroup(group);
	if(!deck.scratchMode)
		deck.picthJog(value);
	else
		deck.scratchJog(value);
}

DenonDNSC2000.jogTouch = function (midino, control, value, status, group) {
	DenonDNSC2000.getDeckByGroup(group).jogTouch(midino, control, value, status, group);
}

DenonDNSC2000.handleLoopAndHotcuesLeds = function(group) {
	var ledChannel = DenonDNSC2000.getLedChannelByGroup(group);
	var cueLeds = [0x11,0x13,0x15,0x17,0x19,0x1B,0x1D,0x20];
	for(var index = 0, count = cueLeds.length;index < count; index ++) {
		DenonDNSC2000.handleLed(ledChannel,(engine.getValue(group,'hotcue_' + (index + 1) + '_position') != -1),cueLeds[index],0x4A,0x4B);
	}

	var ledChannel = DenonDNSC2000.getLedChannelByGroup(group);
	DenonDNSC2000.handleLed(ledChannel,(engine.getValue(group, 'loop_start_position') != -1),0x24,0x4A,0x4B);
	DenonDNSC2000.handleLed(ledChannel,(engine.getValue(group, 'loop_end_position') != -1),0x40,0x4A,0x4B);
	DenonDNSC2000.handleLed(ledChannel,(engine.getValue(group, 'loop_enabled') == 1),0x2B,0x4A,0x4B);
}

DenonDNSC2000.handlePlayLed = function(group) {
	DenonDNSC2000.handleLed(DenonDNSC2000.getLedChannelByGroup(group),(engine.getValue(group, 'play') == 1),0x27,0x4A,0x4C);
}

DenonDNSC2000.handleKeyLockLed = function (group) {
	DenonDNSC2000.handleLed(DenonDNSC2000.getLedChannelByGroup(group),(engine.getValue(group, 'keylock') == 1),0x08,0x4A,0x4B);
}

DenonDNSC2000.handleBeatSyncLed = function (group) {
	DenonDNSC2000.handleLed(DenonDNSC2000.getLedChannelByGroup(group),(engine.getValue(group, 'beatsync') == 1),0x09,0x4A,0x4B);
}

DenonDNSC2000.handleFlangerLed = function (group) {
	var flangerOn = (engine.getValue(group, 'flanger') == 1);
	DenonDNSC2000.handleLed(DenonDNSC2000.getLedChannelByGroup(group),flangerOn,0x5A,0x4A,0x4B);
	DenonDNSC2000.handleLed(DenonDNSC2000.getLedChannelByGroup(group),flangerOn,0x5D,0x4A,0x4B);
	DenonDNSC2000.handleLed(DenonDNSC2000.getLedChannelByGroup(group),flangerOn,0x5E,0x4A,0x4B);
	DenonDNSC2000.handleLed(DenonDNSC2000.getLedChannelByGroup(group),flangerOn,0x5F,0x4A,0x4B);
}

DenonDNSC2000.handleLed = function (ledChannel,test,led,stateTrue, stateFalse) {
	if(test) {
		midi.sendShortMsg(ledChannel,stateTrue,led);
	}
	else {
		midi.sendShortMsg(ledChannel,stateFalse,led);
	}
}

DenonDNSC2000.handleLeds = function(group) {
	DenonDNSC2000.handleLoopAndHotcuesLeds(group);
	DenonDNSC2000.handleLoopAndHotcuesLeds(group);
	DenonDNSC2000.handlePlayLed(group);
	DenonDNSC2000.handleKeyLockLed(group);
	DenonDNSC2000.handleBeatSyncLed(group);
	DenonDNSC2000.handleFlangerLed(group);
}

DenonDNSC2000.getLedChannelByGroup = function(group) {
	if(group == '[Channel1]')
		return 0xB0;
	else
	if(group == '[Channel2]')
		return 0xB1;
	else
	if(group == '[Channel3]')
		return 0xB2;
	else
	if(group == '[Channel4]')
		return 0xB3;
}

DenonDNSC2000.toggleBinaryValue = function(group,key) {
	engine.setValue(group,key,engine.getValue(group,key)*-1 + 1);
}

DenonDNSC2000.samplesPerBeat = function(group) {
	return 2 * engine.getValue(group,'track_samplerate') * 60 / engine.getValue(group, "file_bpm");
}
/*******************************************************************************/
DenonDNSC2000.Deck.prototype.picthJog = function(value) {
	value = (value - 0x40)/4;
	if(value > 0) {
		value = value * value;
	} else {
		value = -value * value;
	}
	engine.setValue(this.group,"jog", value);
}

DenonDNSC2000.Deck.prototype.jogTouch = function (midino, control, value, status, group) {
	if ((status & 0xF0) == 0x90) {
		engine.scratchEnable(this.deckNumber, 2048, 33+1/3, 1.0/8, (1.0/8)/32);
		this.scratchMode = true;
	}
	else
	if ((status & 0xF0) == 0x80) {
		engine.scratchDisable(this.deckNumber);
		this.scratchMode = false;
	}
}

DenonDNSC2000.Deck.prototype.scratchJog = function (value) {
	engine.scratchTick(this.deckNumber,value-0x40);
}
/*******************************************************************************/
var shiftPressed = false;
var decks = [new DenonDNSC2000.Deck(1,'[Channel1]'),
			 new DenonDNSC2000.Deck(2,'[Channel2]'),
			 new DenonDNSC2000.Deck(3,'[Channel3]'),
			 new DenonDNSC2000.Deck(4,'[Channel4]'),
			];
