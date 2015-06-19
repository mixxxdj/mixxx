/**
 * Electrix Tweaker controller script 0.3 for Mixxx 1.12
 * Copyright (C) 2015 Be <be.0@gmx.com>
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

Description:
The shift buttons are the small circular button in the middle of the arrows at the top and the yellow buttons in the grid at the bottom.
The encoders, knobs, vertical faders, and small multicolor buttons control a deck.

Big encoder: scroll through library
Big encoder + shift: scroll through library quickly
Big encoder press: toggle big library view
Big encoder press + shift: load selected track into first stopped deck

Side arrows load the selected track into the corresponding deck and light up when the deck is loaded. Press shift and a side arrow to eject the track in that deck.

Up and down arrows navigate the left library pane. Press shift and either the up or down arrow to expand a category.

Big velocity sensitive buttons: one shot samplers
	Off when empty, red when loaded
	Press a button to load the selected sample into a sampler and play it
	Press a button to play a sample. When the button is released, the sample will stop playing.
	Samples will play with their volume proportional to how much force was used to strike the button.

The analog knobs control filters for each deck.
Pressing shift and turning the right analog knob controls cue/master mix in the headphones. Turning it all the way to the right with shift pressed toggles split cue mode.

The small multicolor button below the encoders toggles the mode for the encoders on that side. White is EQ mode, purple is loop mode.
	In EQ mode, the encoders control high, mid, and low EQs from top to bottom. Pressing the encoder kills the EQ. Pressing the encoder while holding shift resets the EQ to center.
	
	In loop mode:
		Top encoder: adjust loop move size. Center LED represents 1 beat. Each step to the right doubles the move size; each step to the left halves the move size.
		Middle encoder: move loop backwards and forwards
		Bottom encoder: adjust loop length. Center LED represents 1 beat. Each step to the right doubles the loop size; each step to the left halves the loop size.
		
		Middle encoder press: toggle between loop moving and beatjumping
		Low encoder press: toggle loop on/off
	
	In any mode, holding shift and moving the middle encoder scrolls through the 4 pages of hotcues on the button grid.
	In any mode, holding shift and moving the low encoder scrolls through the track 32 beats at a time.
When holding shift in any mode and pressing the mode button, a loop will be toggled that is equal to the length selected by the bottom encoder in loop mode.

Small multicolor button above the vertical fader: toggle headphone cueing
Shift + headphone button: disable loop. This is especially helpful for loops set manually on the button grid.
Vertical fader: volume
Shit + vertical fader: pitch
The bottom multicolor buttons are play/pause buttons. They are off when a deck is empty, red when loaded and paused, and green when playing.
Shift + play/pause is the cue button. When previewing from a cue point while a track is paused, let go of shift to let the track continue playing. Let go of the cue button to stop the track and jump back to the cue point.

Small button grid:
Rows 1 & 2 are hotcue buttons. By default, they control hotcues 1-8. By pressing shift and turning the middle encoder, they can be switched between 4 pages with 8 hotcues each for a total of 32 hotcues. When there is no hotcue set, they are off.
To set a hotcue point, simply press a hotcue button that is off. To move a hotcue that is already set, press a yellow shift buttons in the button grid and a hotcue button. To delete a cue point, press the top shift button surrounded by the red arrows and a hotcue button.
When slip mode is off, pressing a hotcue button will simply jump to that hotcue point.
When slip mode is on, the deck will jump to the hotcue and keep playing from there as long as the hotcue button is held down. If the deck was playing before any hotcue buttons were pressed, when all hotcue buttons are released, the deck will jump to where it would have been if no hotcue buttons were pressed. If the deck was not playing before any hotcue buttons were pressed, when all hotcue buttons are released, the deck will stop playing.

The vertical green/white buttons on the bottom left of the deck are for seeking. When quantize is off, they are green. The top green button plays the deck in fast-forward and the bottom green button plays it in reverse. When shift is pressed, the top green button jumps to the end of the track and the bottom green button jumps to the beginning of the track. When quantize is on, they are white. The top white button jumps forward 1 beat and the bottom white button jumps backward 1 beat. When shift is pressed, the top white button jumps forward 4 beats and the bottom white button jumps backward 4 beats.

The yellow button is a shift button.
The button to the left of the yellow shift button toggles slip mode. With shift pressed, a loop in point is placed at the current play position.
The button to the right of the yellow shift button toggles betwen decks 1 & 3 on the left and decks 2 & 4 on the left. With shift pressed, a loop out point is placed at the current play position.

The button to the left of the bottom green/white navigation button toggles quantize mode. With shift pressed, the beatgrid is aligned to the current play position.
The next button to the left toggles keylock. With shift pressed, it syncs the key to that of the other deck. When the key of a deck has been changed from its original value, the button will turn white. Pressing the button when it is white will reset the key to its original value.
The next button to the left (the bottom right button in the deck's grid) toggles sync. With shift pressed, it sync the deck to the other deck without keeping sync mode enabled.
**/

// Adjust this variable to your liking. The higher it is, the less hard you have to strike the sample pads to play samples loudly.
ElectrixTweaker.samplerSensitivity = 4

// ====================================================== INITIALIZATION ======================================================

function ElectrixTweaker() {}

script.absoluteNonLinInverse = function (value, low, mid, high, min, max) {
	if (!min) min = 0;
	if (!max) max = 127;
	var center = (max-min)/2;
	if (value==mid)
		return center;
	if (value<mid)
		return (center/(mid-low)) * (value-low);
	return center + (center/(high-mid)) * (value-mid);
}

ElectrixTweaker.colorCodes = {
	'off': 0,
	'green': 1,
	'red': 4,
	'yellow': 8,
	'blue': 16,
	'cyan': 32,
	'magenta': 64,
	'white': 127
}
ElectrixTweaker.deckColor = {
	'[Channel1]': {
		'hotcues': ElectrixTweaker.colorCodes['cyan'],
		'switches': ElectrixTweaker.colorCodes['blue']
	},
	'[Channel2]': {
		'hotcues': ElectrixTweaker.colorCodes['cyan'],
		'switches': ElectrixTweaker.colorCodes['blue']
	},
	'[Channel3]':{
		'hotcues': ElectrixTweaker.colorCodes['red'],
		'switches': ElectrixTweaker.colorCodes['magenta']
	},
	'[Channel4]': {
		'hotcues': ElectrixTweaker.colorCodes['red'],
		'switches': ElectrixTweaker.colorCodes['magenta']
	}
}

ElectrixTweaker.encoders = {
	'[Channel1]': {
		'High': {
			'cc': 57,
			'ring': 79,
			'button': 45
		},
		'Mid': {
			'cc': 58,
			'ring': 80,
			'button': 46
		},
		'Low': {
			'cc': 59,
			'ring': 81,
			'button': 47
		}
	},
	'[Channel2]': {
		'High': {
			'cc': 60,
			'ring': 82,
			'button': 48
		},
		'Mid': {
			'cc': 61,
			'ring': 83,
			'button': 49
		},
		'Low': {
			'cc': 62,
			'ring': 84,
			'button': 50
		}
	}
}
ElectrixTweaker.encoders['[Channel3]'] = ElectrixTweaker.encoders['[Channel1]']
ElectrixTweaker.encoders['[Channel4]'] = ElectrixTweaker.encoders['[Channel2]']
// each consecutive value in this array sets the encoder ring LEDs to the show the next LED
ElectrixTweaker.encoderRingSteps = [0, 10, 25, 35, 50, 60, 64, 75, 85, 95, 105, 115, 127]
ElectrixTweaker.encoderRingStepsWalk = [0, 1, 20, 30, 40, 50, 60, 64, 75, 85, 95, 105, 120, 127]
ElectrixTweaker.buttons = {
	'[Channel1]': {
		'arrowSide': 42,
		
		'mode': 33,
		'pfl': 34,
		'play': 35,

		
		'hotcues': [
				[1, 2, 3, 4],
				[9, 10, 11, 12]
			],
		
		'forward': 17,
		'slip': 18,
		'shift': 19,
		'deckToggle': 20,
		
		'back': 25,
		'quantize': 26,
		'key': 27,
		'sync': 28
	},
	'[Channel2]': {
		'mode': 36,
		'pfl': 37,
		'play': 38,
		
		'arrowSide': 43,
		
		
		'hotcues': [
				[5, 6, 7, 8],
				[13, 14, 15, 16]
			],
			
		'forward': 21,
		'slip': 22,
		'shift': 23,
		'deckToggle': 24,
		
		'back': 29,
		'quantize': 30,
		'key': 31,
		'sync': 32
		
	}
}
ElectrixTweaker.buttons['[Channel3]'] = ElectrixTweaker.buttons['[Channel1]']
ElectrixTweaker.buttons['[Channel4]'] = ElectrixTweaker.buttons['[Channel2]']

ElectrixTweaker.shift = false
ElectrixTweaker.topShift = false
ElectrixTweaker.deck = {'[Channel1]': '[Channel1]', '[Channel2]': '[Channel2]'}
ElectrixTweaker.mode = {'[Channel1]': 'eq', '[Channel2]': 'eq', '[Channel3]': 'eq', '[Channel4]': 'eq'}
ElectrixTweaker.loopMoveSize = {'[Channel1]': 1, '[Channel2]': 1, '[Channel3]': 1, '[Channel4]': 1}
ElectrixTweaker.loopSize = {'[Channel1]': 4, '[Channel2]': 4, '[Channel3]': 4, '[Channel4]': 4}
ElectrixTweaker.moveMode = {'[Channel1]': 'loop_move_', '[Channel2]': 'loop_move_', '[Channel3]': 'loop_move_', '[Channel4]': 'loop_move_'}
ElectrixTweaker.slipMode = {'[Channel1]': false, '[Channel2]': false, '[Channel3]': false, '[Channel4]': false}
ElectrixTweaker.deckShift = {'[Channel1]': false, '[Channel2]': false, '[Channel3]': false, '[Channel4]': false}
ElectrixTweaker.hotcuePage = {'[Channel1]': 0, '[Channel2]': 0, '[Channel3]': 0, '[Channel4]': 0}
ElectrixTweaker.hotcuesPressed = {'[Channel1]': new Array(8), '[Channel2]': new Array(8), '[Channel3]': new Array(8), '[Channel4]': new Array(8)}
for (var group in ElectrixTweaker.hotcuesPressed) {
	var length = ElectrixTweaker.hotcuesPressed[group].length
	for (i = 0; i <= length; i++) {
		ElectrixTweaker.hotcuesPressed[group][i] = false
	}
}
ElectrixTweaker.playPressedWhileCueJuggling = {'[Channel1]': false, '[Channel2]': false, '[Channel3]': false, '[Channel4]': false}

ElectrixTweaker.sysexPrefix = [ 240, 00, 01, 106, 01 ]
ElectrixTweaker.defaultSettings = [
[ 240, 00, 01, 106, 01, 01, 00, 01, 00, 02, 00, 03, 00, 04, 00, 05, 00, 06, 00, 07, 00, 08, 00, 09, 00, 10, 00, 11, 00, 12, 00, 13, 00, 14, 00, 15, 00, 16, 00, 17, 00, 18, 00, 19, 00, 20, 00, 21, 00, 22, 00, 23, 00, 24, 00, 25, 00, 26, 00, 27, 00, 28, 00, 29, 00, 30, 00, 31, 00, 32, 00, 33, 00, 34, 00, 35, 00, 36, 00, 37, 00, 38, 00, 39, 00, 40, 00, 41, 00, 42, 00, 43, 00, 44, 00, 45, 00, 46, 00, 47, 00, 48, 00, 49, 00, 50, 247 ],
[ 240, 00, 01, 106, 01, 02, 00, 51, 00, 52, 00, 53, 00, 54, 00, 55, 247 ],
[ 240, 00, 01, 106, 01, 03, 00, 56, 00, 57, 00, 58, 00, 59, 00, 60, 00, 61, 00, 62, 247 ],
[ 240, 00, 01, 106, 01, 04, 00, 63, 00, 64, 00, 65, 00, 66, 00, 67, 00, 68, 00, 69, 00, 70, 00, 71, 00, 72, 00, 73, 00, 74, 00, 75, 00, 76, 00, 77, 00, 78, 247 ],
[ 240, 0, 1, 106, 01, 05, 126, 05, 01, 247 ],
[ 240, 00, 01, 106, 01, 06, 127, 127, 15, 00, 07, 00, 09, 5, 247 ],
[ 240, 0, 1, 106, 01, 07, 42, 42, 247 ],
[ 240, 00, 01, 106, 01, 08, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 15, 0, 16, 0, 17, 0, 18, 0, 19, 0, 20, 0, 21, 0, 22, 0, 23, 0, 24, 0, 25, 0, 26, 0, 27, 0, 28, 0, 29, 0, 30, 0, 31, 0, 32, 0, 33, 0, 34, 0, 35, 0, 36, 0, 37, 0, 38, 247 ],
[ 240, 00, 01, 106, 01, 09, 0, 79, 0, 80, 0, 81, 0, 82, 0, 83, 0, 84, 247 ],
[ 240, 0, 1, 106, 01, 13, 15, 247 ],
[ 240, 0, 1, 106, 01, 14, 0, 247 ],
[ 240, 0, 1, 106, 01, 15, 0, 247 ]
]
ElectrixTweaker.requestConfiguration = ElectrixTweaker.sysexPrefix.concat([ 126 , 247 ])

ElectrixTweaker.samplerRegEx = /\[Sampler(\d+)\]/
ElectrixTweaker.channelRegEx = /\[Channel(\d+)\]/

ElectrixTweaker.init = function () {
	if (engine.getValue('[Master]', 'num_samplers') < 16) {
		engine.setValue('[Master]', 'num_samplers', 16)
	}
	engine.softTakeover('[Master]', 'headMix', true)
	engine.softTakeover('[Master]', 'headVolume', true)
	for (var group in ElectrixTweaker.encoders) {
		engine.softTakeover('[QuickEffectRack1_'+group+']', 'super1', true)
		engine.softTakeover(group, 'volume', true)
	}
	ElectrixTweaker.initDeck('[Channel1]',true)
	ElectrixTweaker.initDeck('[Channel2]',true)
	for (i=1; i<=8; i++) {
		engine.connectControl('[Sampler'+i+']', 'track_samples', ElectrixTweaker.oneShotLED)
		engine.trigger('[Sampler'+i+']', 'track_samples')
	}
	
	midi.sendShortMsg(0x90, 39, 127) // light up arrow
	midi.sendShortMsg(0x90, 40, 127) // light shift button
	midi.sendShortMsg(0x90, 41, 127) // light down arrow
// 	midi.sendSysexMsg(ElectrixTweaker.requestConfiguration, ElectrixTweaker.requestConfiguration.length)
// 	for (var msg in ElectrixTweaker.defaultSettings) {
// 		midi.sendSysexMsg(ElectrixTweaker.defaultSettings[msg], ElectrixTweaker.defaultSettings[msg].length)
// 	}
}
// ElectrixTweaker.inboundSysex = function (data, length) {
// 	print('========================== incoming sysex message ======================================')
// 	print(length)
// 	print(data)
// 	if (length == 108) {
// 		ElectrixTweaker.controllerConfiguration = data
// 	}
// }

ElectrixTweaker.connectHotcuePage = function (group, remove) {
	remove = (typeof remove !== 'undefined') ? remove : false // default value for remove is false
	
	var min = 1 + (ElectrixTweaker.hotcuePage[group] * 8)
	var max = min + 7
	for (i=min; i<=max; i++) {
		engine.connectControl(group, 'hotcue_'+i+'_enabled', ElectrixTweaker.hotcueLED, remove)
		if (! remove) {
			engine.trigger(group, 'hotcue_'+i+'_enabled')
		}
	}
}

ElectrixTweaker.connectDeckControls = function (group, remove) {
	remove = (typeof remove !== 'undefined') ? remove : false // default value for remove is false
	
	var controlsToFunctions = {
		'pfl': ElectrixTweaker.pflButtonLED,
		'track_samples': ElectrixTweaker.playButtonLED,
		'track_samples': ElectrixTweaker.arrowSideLED,
		'play': ElectrixTweaker.playButtonLED,
		'playposition': ElectrixTweaker.playButtonLED,
		'loop_enabled': ElectrixTweaker.loopButtonToggle,
		'sync_enabled': ElectrixTweaker.syncLED,
		'key': ElectrixTweaker.keyLED,
		'keylock': ElectrixTweaker.keyLED,
		'quantize': ElectrixTweaker.quantizeLED
	}
	for (var control in controlsToFunctions) {
		engine.connectControl(group, control, controlsToFunctions[control], remove)
		if (! remove) {
			engine.trigger(group, control)
		}
	}
	ElectrixTweaker.connectHotcuePage(group, remove)
	
	var eqsToFunctions = {
		'filterHigh': ElectrixTweaker.eqEncoder,
		'filterMid': ElectrixTweaker.eqEncoder,
		'filterLow': ElectrixTweaker.eqEncoder,
		'filterHighKill': ElectrixTweaker.eqEncoderKillButton,
		'filterMidKill': ElectrixTweaker.eqEncoderKillButton,
		'filterLowKill': ElectrixTweaker.eqEncoderKillButton
	}
	if (remove) {
		for (control in eqsToFunctions) {
			engine.connectControl(group, control, eqsToFunctions[control], remove)
		}
		engine.connectControl(group, 'loop_enabled', ElectrixTweaker.loopButtonToggle, true)
	}
}

ElectrixTweaker.initDeck = function (group, startup) {
	var disconnectDeck = parseInt(ElectrixTweaker.channelRegEx.exec(group)[1])
	if (disconnectDeck <= 2) {
		disconnectDeck += 2
	} else {
		disconnectDeck -= 2
	}
	disconnectDeck = '[Channel' + disconnectDeck + ']'
	ElectrixTweaker.connectDeckControls(disconnectDeck, true)
	
	midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['shift'], ElectrixTweaker.colorCodes['yellow'])
	midi.sendShortMsg(
		0x90,
		ElectrixTweaker.buttons[group]['back'],
		(engine.getValue(group, 'quantize')) ? ElectrixTweaker.colorCodes['white'] : ElectrixTweaker.colorCodes['green']
	)
	midi.sendShortMsg(
		0x90,
		ElectrixTweaker.buttons[group]['forward'],
		(engine.getValue(group, 'quantize')) ? ElectrixTweaker.colorCodes['white'] : ElectrixTweaker.colorCodes['green']
	)
	midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['deckToggle'], ElectrixTweaker.deckColor[group]['switches'])
	midi.sendShortMsg(
				0x90,
				ElectrixTweaker.buttons[group]['slip'],
				(ElectrixTweaker.slipMode[group]) ? ElectrixTweaker.deckColor[group]['switches'] : ElectrixTweaker.colorCodes['off']
			)
	
	ElectrixTweaker.connectDeckControls(group)
	
	ElectrixTweaker.mode[group] = ElectrixTweaker.mode[disconnectDeck]
	ElectrixTweaker.initMode(group, ElectrixTweaker.mode[group], startup)
}
ElectrixTweaker.initMode = function (group, mode, startup) {
	ElectrixTweaker.mode[group] = mode
	switch (mode) {
		case 'eq':
			midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['mode'], ElectrixTweaker.colorCodes['white'])
			engine.connectControl(group, 'loop_enabled', ElectrixTweaker.loopButtonToggle, true)
			for (var encoder in ElectrixTweaker.encoders[group]) {
				// set encoder to absolute EQ mode with speed 5
				midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group][encoder]['cc'], 118)
				// enable local control of LED ring
				midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group][encoder]['ring'], 70)
				engine.connectControl(group, 'filter' + encoder, ElectrixTweaker.eqEncoder)
				engine.connectControl(group, 'filter' + encoder + 'Kill', ElectrixTweaker.eqEncoderKillButton)
				engine.trigger(group, 'filter' + encoder)
				engine.trigger(group, 'filter' + encoder + 'Kill')
			}
			break
		case 'loop':
			midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['mode'], ElectrixTweaker.colorCodes['magenta'])
			for (var encoder in ElectrixTweaker.encoders[group]) {
				engine.connectControl(group, 'filter' + encoder, ElectrixTweaker.eqEncoder, true)
				engine.connectControl(group, 'filter' + encoder + 'Kill', ElectrixTweaker.eqEncoderKillButton, true)
				// set encoder to relative mode
				midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group][encoder]['cc'], 64)
				// set LED ring to EQ mode with local control disabled
				midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group][encoder]['ring'], 98)
			}
			
			midi.sendShortMsg(
				0xB0,
				ElectrixTweaker.encoders[group]['High']['ring'],
				ElectrixTweaker.encoderRingSteps[
					6 + Math.log(ElectrixTweaker.loopMoveSize[group]) / Math.log(2)
				]
			)
			
			midi.sendShortMsg(
				0xB0,
				ElectrixTweaker.encoders[group]['Mid']['ring'],
				64
			)
			midi.sendShortMsg(
				0x90,
				ElectrixTweaker.encoders[group]['Mid']['button'],
				(ElectrixTweaker.moveMode[group] == 'beatjump_') ? 127 : 0
			)
			
			midi.sendShortMsg(
				0xB0,
				ElectrixTweaker.encoders[group]['Low']['ring'],
				ElectrixTweaker.encoderRingSteps[
					6 + Math.log(ElectrixTweaker.loopSize[group]) / Math.log(2)
				]
			)
			
			engine.connectControl(group, 'loop_enabled', ElectrixTweaker.loopButtonToggle)
			engine.trigger(group, 'loop_enabled')
			break
	}
}
ElectrixTweaker.shutdown = function() {
	for (var group in ElectrixTweaker.encoders) {
		for (var encoder in ElectrixTweaker.encoders[group]) {
			// set encoder to absolute EQ mode with speed 5
			midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group][encoder]['cc'], 118)
			// enable local control of LED ring
			midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group][encoder]['ring'], 70)
			// set rings to center
			midi.sendShortMsg(0xB0, ElectrixTweaker.encoders[group][encoder]['cc'], 64)
			// turn off blue button lights
			midi.sendShortMsg(0x90, ElectrixTweaker.encoders[group][encoder]['button'], 0)
		}
	}
	// turn off all button LEDs
	for (i = 0; i <= 70; i++) {
		midi.sendShortMsg(0x90, i, ElectrixTweaker.colorCodes['off'])
	}
// 	midi.sendShortMsg(0x90, 39, 0)
}

// ============================================== ARROWS + BIG ENCODER ================================================

ElectrixTweaker.shiftButton = function (channel, control, value, status, group) {
	ElectrixTweaker.shift = ! ElectrixTweaker.shift
	if (control == 0x28) {
		ElectrixTweaker.topShift = ! ElectrixTweaker.topShift
	}
	if (value) {
		
		for (channel in ElectrixTweaker.deck) {
			// set mid encoder to relative mode
			midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[channel]['Mid']['cc'], 73)
			// set mid LED ring to walk mode with local control disabled
			midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[channel]['Mid']['ring'], 97)
			midi.sendShortMsg(
				0xB0,
				ElectrixTweaker.encoders[channel]['Mid']['ring'],
				ElectrixTweaker.encoderRingStepsWalk[ElectrixTweaker.hotcuePage[channel]+1]
			)
			// set low encoder to relative mode
			midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[channel]['Low']['cc'], 64)
			midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[channel]['Low']['ring'], 67)
		}
	} else {
		ElectrixTweaker.initMode(ElectrixTweaker.deck['[Channel1]'], ElectrixTweaker.mode['[Channel1]'])
		ElectrixTweaker.initMode(ElectrixTweaker.deck['[Channel2]'], ElectrixTweaker.mode['[Channel2]'])
	}
}

ElectrixTweaker.bigEncoder = function (channel, control, value, status, group) {
	if (ElectrixTweaker.shift) {
		for (i=0 ; i<35; i++) {
			engine.setValue('[Playlist]', (value == 1) ? 'SelectNextTrack' : 'SelectPrevTrack', 1)
		}
	} else {
		engine.setValue('[Playlist]', (value == 1) ? 'SelectNextTrack' : 'SelectPrevTrack', 1)
	}
}
ElectrixTweaker.bigEncoderButton = function (channel, control, value, status, group) {
	if (value) {
		if (ElectrixTweaker.shift) {
			engine.setValue('[Playlist]', 'LoadSelectedIntoFirstStopped', 1)
		} else {
			engine.setValue('[Master]', 'maximize_library', ! engine.getValue('[Master]', 'maximize_library'))
		}
	}
}
ElectrixTweaker.arrowSide = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (ElectrixTweaker.shift) {
		engine.setValue(group, 'eject', 1)
		engine.beginTimer(250, 'engine.setValue("'+group+'", "eject", 0)', true)
	} else {
		engine.setValue(group, 'LoadSelectedTrack', 1)
	}
}
ElectrixTweaker.arrowSideLED = function (value, group, control) {
	midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['arrowSide'], (value) ? 127 : 0)
}
ElectrixTweaker.arrowUp = function (channel, control, value, status, group) {
	if (value) {
		if (ElectrixTweaker.shift) {
			engine.setValue('[Playlist]', 'ToggleSelectedSidebarItem', 1)
		} else {
			engine.setValue('[Playlist]', 'SelectPrevPlaylist', 1)
		}
	}
}
ElectrixTweaker.arrowDown = function (channel, control, value, status, group) {
	if (value) {
		if (ElectrixTweaker.shift) {
			engine.setValue('[Playlist]', 'ToggleSelectedSidebarItem', 1)
		} else {
			engine.setValue('[Playlist]', 'SelectNextPlaylist', 1)
		}
	}
}

// ===================================================== SAMPLERS ===========================================================

ElectrixTweaker.oneShot = function (channel, control, value, status, group) {
	if (value) {
		if (engine.getValue(group, 'track_samples')) {
			if (ElectrixTweaker.shift) {
				engine.setValue(group, 'key', 0)
				engine.setValue(group, 'sync_enabled', 0)
				engine.setValue(group, 'repeat', 0)
				engine.setValue(group, 'play', 0)
				engine.setValue(group, 'eject', 1)
				engine.beginTimer(250, 'engine.setValue("'+group+'", "eject", 0)', true)
			} else {
				engine.setValue(group, 'volume', script.absoluteNonLin(value * ElectrixTweaker.samplerSensitivity, 0, .25, 1))
				engine.setValue(group, 'playposition', 0)
				engine.setValue(group, 'play', 1)
			}
		} else {
			engine.setValue(group, 'volume', script.absoluteNonLin(value, 0, .25, 1))
			engine.setValue(group, 'LoadSelectedTrackAndPlay', 1)
		}
	} else {
		engine.setValue(group, 'play', 0)
	}
}
ElectrixTweaker.oneShotNote = function (channel, control, value, status, group) {
}
ElectrixTweaker.oneShotLED = function (value, group, control) {
	midi.sendShortMsg(0x90, 62 + parseInt(ElectrixTweaker.samplerRegEx.exec(group)[1]), (value) ? 127 : 0)
}

// ================================================= CHANNEL STRIPS ===========================================================
ElectrixTweaker.leftKnob = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
// 	if (Math.abs(script.absoluteLin(value, 0, 1) - engine.getValue('[QuickEffectRack1_'+group+']', 'super1')) < .1) {
		engine.setValue('[QuickEffectRack1_'+group+']', 'super1', script.absoluteLin(value, 0, 1))
// 	}
}
ElectrixTweaker.rightKnob = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (ElectrixTweaker.shift) {
		if (value == 127) {
			engine.setValue('[Master]', 'headSplit', ! engine.getValue('[Master]', 'headSplit'))
		} else {
			if (Math.abs(engine.getValue('[Master]', 'headMix') - (value - 64)/64) < .2) {
				engine.setValue('[Master]', 'headMix', script.absoluteLin(value, -1, 1))
			}
		}
	} else {
		if (Math.abs(script.absoluteLin(value, 0, 1) - engine.getValue('[QuickEffectRack1_'+group+']', 'super1')) < .1) {
			engine.setValue('[QuickEffectRack1_'+group+']', 'super1', script.absoluteLin(value, 0, 1))
		}
	}
}

ElectrixTweaker.fader = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (ElectrixTweaker.shift) {
		if (Math.abs(engine.getValue(group, 'rate') - (value - 64)/64) < .2) {
			engine.setValue(group, 'rate', script.absoluteLin(value, -1, 1))
		}
	} else {
		if (Math.abs(value - script.absoluteNonLinInverse(engine.getValue(group, 'volume'), 0, .25, 1)) < 30) {
			engine.setValue(group, 'volume', script.absoluteNonLin(value, 0, .25, 1))
		}
	}
}
ElectrixTweaker.faderMiddleNote = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (ElectrixTweaker.shift && ! value) {
		engine.setValue(group, 'rate', 0)
	}
}

ElectrixTweaker.modeButton = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		if (ElectrixTweaker.shift) {
			if (ElectrixTweaker.slipMode[group]) {
				engine.setValue(group, 'slip_enabled', ! engine.getValue(group, 'slip_enabled'))
			}
			engine.setValue(group, 'beatloop_' + ElectrixTweaker.loopSize[group] + '_toggle', 1)
			if (ElectrixTweaker.mode[group] == 'loop') {
				midi.sendShortMsg(0x90, ElectrixTweaker.encoders[group]['Low']['button'], engine.getValue(group, 'loop_enabled') * 127)
			}
		} else {
			switch (ElectrixTweaker.mode[group]) {
				case 'eq':
					ElectrixTweaker.initMode(group, 'loop')
					break
				case 'loop':
					ElectrixTweaker.initMode(group, 'eq')
					break
			}
		}
	}
}

ElectrixTweaker.eqEncoder = function (value, group, control) {
	var encoder = control.replace('filter', '')
	midi.sendShortMsg(0xB0, ElectrixTweaker.encoders[group][encoder]['cc'], script.absoluteNonLinInverse(value, 0, 1, 4))
}

ElectrixTweaker.eqEncoderKillButton = function (value, group, control) {
	var encoder = control.replace('filter', '')
	encoder = encoder.replace('Kill', '')
	midi.sendShortMsg(0x90, ElectrixTweaker.encoders[group][encoder]['button'], value * 127)
}

ElectrixTweaker.highEncoder = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	switch (ElectrixTweaker.mode[group]) {
		case 'eq':
			engine.setValue(group, 'filterHigh', script.absoluteNonLin(value, 0, 1, 4) )
			break
		case 'loop':
			if ((value == 127) && (ElectrixTweaker.loopMoveSize[group] >= Math.pow(2, -5))) {
				ElectrixTweaker.loopMoveSize[group] = ElectrixTweaker.loopMoveSize[group] / 2
			} else if ((value == 1) && (ElectrixTweaker.loopMoveSize[group] <= Math.pow(2, 5))) {
				ElectrixTweaker.loopMoveSize[group] = ElectrixTweaker.loopMoveSize[group] * 2
			}
			midi.sendShortMsg(0xB0, ElectrixTweaker.encoders[group]['High']['ring'], ElectrixTweaker.encoderRingSteps[ 6 + Math.log(ElectrixTweaker.loopMoveSize[group]) / Math.log(2) ] )
			break
	}
}
ElectrixTweaker.highEncoderPress = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		switch (ElectrixTweaker.mode[group]) {
			case 'eq':
				if (ElectrixTweaker.shift) {
					engine.setValue(group, 'filterHigh', 1)
				} else {
					engine.setValue(group, 'filterHighKill', ! engine.getValue(group, 'filterHighKill'))
				}
				break
			case 'loop':
				// What to do with this?
				break
		}
	}
}
ElectrixTweaker.midEncoder = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (ElectrixTweaker.shift) {
		if (value == 1 && ElectrixTweaker.hotcuePage[group] < 3) {
			ElectrixTweaker.connectHotcuePage(group, true)
			ElectrixTweaker.hotcuePage[group]++
			ElectrixTweaker.connectHotcuePage(group)
		} else if (value == 127 && ElectrixTweaker.hotcuePage[group] > 0) {
			ElectrixTweaker.connectHotcuePage(group, true)
			ElectrixTweaker.hotcuePage[group]--
			ElectrixTweaker.connectHotcuePage(group)
		}
		midi.sendShortMsg(
			0xB0,
			ElectrixTweaker.encoders[group]['Mid']['ring'],
			ElectrixTweaker.encoderRingStepsWalk[ElectrixTweaker.hotcuePage[group]+1]
		)
	} else {
		switch (ElectrixTweaker.mode[group]) {
			case 'eq':
				engine.setValue(group, 'filterMid', script.absoluteNonLin(value, 0, 1, 4))
				break
			case 'loop':
				if (value == 127) {
					midi.sendShortMsg(
						0xB0,
						ElectrixTweaker.encoders[group]['Mid']['ring'],
						ElectrixTweaker.encoderRingSteps[
							6 - Math.abs( Math.log(ElectrixTweaker.loopMoveSize[group]) / Math.log(2) )
						]
					)
					engine.setValue(group, ElectrixTweaker.moveMode[group] + ElectrixTweaker.loopMoveSize[group] + '_backward', 1)
					engine.beginTimer(1000, 'midi.sendShortMsg(0xB0, ElectrixTweaker.encoders["'+group+'"]["Mid"]["ring"], 64)', true)
				} else {
					midi.sendShortMsg(
						0xB0,
						ElectrixTweaker.encoders[group]['Mid']['ring'],
						ElectrixTweaker.encoderRingSteps[
							6 + Math.abs( Math.log(ElectrixTweaker.loopMoveSize[group]) / Math.log(2) )
						]
					)
					engine.setValue(group, ElectrixTweaker.moveMode[group] + ElectrixTweaker.loopMoveSize[group] + '_forward', 1)
					engine.beginTimer(1000, 'midi.sendShortMsg(0xB0, ElectrixTweaker.encoders["'+group+'"]["Mid"]["ring"], 64)', true)
				}
				break
		}
	}
}
ElectrixTweaker.midEncoderPress = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		switch (ElectrixTweaker.mode[group]) {
			case 'eq':
				if (ElectrixTweaker.shift) {
					engine.setValue(group, 'filterMid', 1)
				} else {
					engine.setValue(group, 'filterMidKill', ! engine.getValue(group, 'filterMidKill'))
				}
				break
			case 'loop':
				if (ElectrixTweaker.moveMode[group] == 'loop_move_') {
					ElectrixTweaker.moveMode[group] = 'beatjump_'
				} else if (ElectrixTweaker.moveMode[group] == 'beatjump_') {
					ElectrixTweaker.moveMode[group] = 'loop_move_'
				}
				midi.sendShortMsg(
					0x90,
					ElectrixTweaker.encoders[group]['Mid']['button'],
					(ElectrixTweaker.moveMode[group] == 'beatjump_') ? 127 : 0
				)
				break
		}
	}
}
ElectrixTweaker.lowEncoder = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (ElectrixTweaker.shift) {
		if (value == 127) {
			engine.setValue(group, 'beatjump_32_backward', 1)
		} else {
			engine.setValue(group, 'beatjump_32_forward', 1)
		}
	} else {
		switch (ElectrixTweaker.mode[group]) {
			case 'eq':
				engine.setValue(group, 'filterLow', script.absoluteNonLin(value, 0, 1, 4))
				break
			case 'loop':
				if ((value == 127) && (ElectrixTweaker.loopSize[group] >= Math.pow(2, -5))) {
					ElectrixTweaker.loopSize[group] = ElectrixTweaker.loopSize[group] / 2
					engine.setValue(group, 'loop_halve', 1)
				} else if ((value == 1) && (ElectrixTweaker.loopSize[group] <= Math.pow(2, 5))) {
					ElectrixTweaker.loopSize[group] = ElectrixTweaker.loopSize[group] * 2
					engine.setValue(group, 'loop_double', 1)
				}
				midi.sendShortMsg(
					0xB0,
					ElectrixTweaker.encoders[group]['Low']['ring'],
					ElectrixTweaker.encoderRingSteps[
						6 + Math.log(ElectrixTweaker.loopSize[group]) / Math.log(2)
					]
				)
				break
		}
	}
}
ElectrixTweaker.lowEncoderPress = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		switch (ElectrixTweaker.mode[group]) {
			case 'eq':
				if (ElectrixTweaker.shift) {
					engine.setValue(group, 'filterLow', 1)
				} else {
					engine.setValue(group, 'filterLowKill', ! engine.getValue(group, 'filterLowKill'))
				}
				break
			case 'loop':
				if (ElectrixTweaker.slipMode[group]) {
					engine.setValue(group, 'slip_enabled', ! engine.getValue(group, 'slip_enabled'))
				}
				if (engine.getValue(group, 'loop_enabled')) {
					engine.setValue(group, 'reloop_exit', 1)
				} else {
					engine.setValue(group, 'beatloop_' + ElectrixTweaker.loopSize[group] + '_activate', 1)
				}
				break
		}
	}
}
ElectrixTweaker.loopButtonToggle = function (value, group, control) {
	midi.sendShortMsg(0x90, ElectrixTweaker.encoders[group]['Low']['button'], value * 127)
}

ElectrixTweaker.pflButton = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		if (ElectrixTweaker.shift) {
			engine.setValue(group, 'reloop_exit', ! engine.getValue(group, 'reloop_exit'))
		} else {
			engine.setValue(group, 'pfl', ! engine.getValue(group, 'pfl'))
		}
	}
}
ElectrixTweaker.pflButtonLED = function (value, group, control) {
	midi.sendShortMsg(
		0x90,
		ElectrixTweaker.buttons[group][control],
		(engine.getValue(group, 'pfl')) ? ElectrixTweaker.colorCodes['green'] : ElectrixTweaker.colorCodes['off']
	)
}

ElectrixTweaker.playButton = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (ElectrixTweaker.shift) {
		engine.setValue(group, 'cue_default', value)
	} else if (value) {
		if (ElectrixTweaker.anyHotcuesPressed(group)) {
			ElectrixTweaker.playPressedWhileCueJuggling[group] = true
			engine.trigger(group, 'play')
		} else {
			if (engine.getValue(group, 'playposition') == 1) {
				engine.setValue(group, 'start_play', 1)
			} else {
				engine.setValue(group, 'play', ! engine.getValue(group, 'play'))
			}
		}
	}
}
ElectrixTweaker.playButtonLED = function (value, group, control) {
	if (engine.getValue(group, 'play')) {
		if (
			(control != 'playposition') // do not spam MIDI signals with each update in playposition while playing
			&& (
				(! ElectrixTweaker.anyHotcuesPressed(group)) ||
				ElectrixTweaker.playPressedWhileCueJuggling[group]
			)
		) {
			midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['play'], ElectrixTweaker.colorCodes['green'])
		}
	} else {
		if (engine.getValue(group, 'playposition') < .999) {
			midi.sendShortMsg(
				0x90,
				ElectrixTweaker.buttons[group]['play'],
				(engine.getValue(group, 'track_samples')) ? ElectrixTweaker.colorCodes['red'] : ElectrixTweaker.colorCodes['off']
			)
		} else {
			engine.setValue(group, 'cue_gotoandstop', 1)
// 			midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['play'], ElectrixTweaker.colorCodes['off'])
		}
	}
}


//===================================================================== BUTTON GRID =========================================================

ElectrixTweaker.anyHotcuesPressed = function (group) {
	var length = ElectrixTweaker.hotcuesPressed[group].length
	for (i=0; i<=length; i++) {
		if (ElectrixTweaker.hotcuesPressed[group][i]) {
			return true
		}
	}
}
ElectrixTweaker.hotcue = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	var row = (control < ElectrixTweaker.buttons[group]['hotcues'][1][0]) ? 0 : 1
	var cueButton = 4 + row*4 - (ElectrixTweaker.buttons[group]['hotcues'][row][3] - control)
	var cue = cueButton + (8 * ElectrixTweaker.hotcuePage[group])
	if (value) {
		print(ElectrixTweaker.topShift)
		if (engine.getValue(group, 'hotcue_'+cue+'_enabled')) {
			if (ElectrixTweaker.shift && (! ElectrixTweaker.topShift)) {
				engine.setValue(group, 'hotcue_'+cue+'_set', 1)
			} else if (ElectrixTweaker.topShift) {
				engine.setValue(group, 'hotcue_'+cue+'_clear', 1)
			} else {
				if (ElectrixTweaker.slipMode[group]) {
					if (engine.getValue(group, 'play') && ! ElectrixTweaker.anyHotcuesPressed(group)) {
						engine.setValue(group, 'slip_enabled', 1)
					}
					engine.setValue(group, 'hotcue_'+cue+'_gotoandplay', 1)
					ElectrixTweaker.hotcuesPressed[group][cueButton] = true
				} else {
					engine.setValue(group, 'hotcue_'+cue+'_goto', 1)
				}
			}
		} else {
			engine.setValue(group, 'hotcue_'+cue+'_set', 1)
		}
	} else {
		ElectrixTweaker.hotcuesPressed[group][cueButton] = false
		
		if (ElectrixTweaker.slipMode[group]) {
			if (! engine.getValue(group, 'slip_enabled')) { // if cue jugging started from pause
				if (ElectrixTweaker.playPressedWhileCueJuggling[group]) {
					ElectrixTweaker.playPressedWhileCueJuggling[group] = false
				} else if (! ElectrixTweaker.anyHotcuesPressed(group)) {
					engine.setValue(group, 'play', 0)
				}
			} else if (! ElectrixTweaker.anyHotcuesPressed(group)) {
				engine.setValue(group, 'slip_enabled', 0)
			}
		}
	}
}
ElectrixTweaker.hotcueLED = function (value, group, control) {
	var cue = parseInt(control.split('_')[1]) - (8 * ElectrixTweaker.hotcuePage[group])
	var row = (cue <= 4) ? 0 : 1
	midi.sendShortMsg(
		0x90,
		ElectrixTweaker.buttons[group]['hotcues'][row][cue - 1 - 4*row],
		value * ElectrixTweaker.deckColor[group]['hotcues']
	)
}

ElectrixTweaker.slipMode = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		if (ElectrixTweaker.shift) {
			engine.setValue(group, 'loop_in', 1)
		} else {
			if (ElectrixTweaker.slipMode[group]) {
				var length = ElectrixTweaker.hotcuesPressed[group].length
				for (i=0; i<=length; i++) {
					ElectrixTweaker.hotcuesPressed[group][i] = false
				}
				ElectrixTweaker.playPressedWhileCueJuggling[group] = true
				engine.trigger(group, 'play')
			}
			
			ElectrixTweaker.slipMode[group] = ! ElectrixTweaker.slipMode[group]
			midi.sendShortMsg(
				0x90,
			ElectrixTweaker.buttons[group]['slip'],
			ElectrixTweaker.slipMode[group] ? ElectrixTweaker.deckColor[group]['switches'] : ElectrixTweaker.colorCodes['off']
			)
		}
	}
}

ElectrixTweaker.forward = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (engine.getValue(group, 'quantize')) {
		if (value) {
			if (ElectrixTweaker.shift) {
				engine.setValue(group, 'beatjump_4_forward', 1)
			} else {
				engine.setValue(group, 'beatjump_1_forward', 1)
			}
		}
	} else {
		if (ElectrixTweaker.shift && value) {
			engine.setValue(group, 'playposition', 1)
		} else {
			engine.setValue(group, 'fwd', value)
		}
	}
}
ElectrixTweaker.back = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (engine.getValue(group, 'quantize')) {
		if (value) {
			if (ElectrixTweaker.shift) {
				engine.setValue(group, 'beatjump_4_backward', 1)
			} else {
				engine.setValue(group, 'beatjump_1_backward', 1)
			}
		}
	} else {
		if (ElectrixTweaker.shift && value) {
			engine.setValue(group, 'playposition', 0)
		} else {
			engine.setValue(group, 'back', value)
		}
	}
}

ElectrixTweaker.deckShiftButton = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	ElectrixTweaker.deckShift[group] = ! ElectrixTweaker.deckShift[group]
// 	ElectrixTweaker.shiftButton(channel, control, value, status, group)
}

ElectrixTweaker.deckToggle = function (channel, control, value, status, group) {
	if (value) {
		if (ElectrixTweaker.shift) {
			engine.setValue(ElectrixTweaker.deck[group], 'loop_out', 1)
		} else {
			var deckNumber = parseInt(
						ElectrixTweaker.channelRegEx.exec(
							ElectrixTweaker.deck[group]
						)[1]
					)
			if (deckNumber <= 2) {
				deckNumber += 2
			} else {
				deckNumber -= 2
			}
			ElectrixTweaker.deck[group] = '[Channel' + deckNumber + ']'
			ElectrixTweaker.initDeck(ElectrixTweaker.deck[group])
		}
	}
}

ElectrixTweaker.sync = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		if (ElectrixTweaker.shift) {
			engine.setValue(group, 'rate', 0)
		} else {
			engine.setValue(group, 'sync_enabled', ! engine.getValue(group, 'sync_enabled'))
		}
	}
}
ElectrixTweaker.syncLED = function (value, group, control) {
	midi.sendShortMsg(
		0x90,
		ElectrixTweaker.buttons[group]['sync'],
		(engine.getValue(group, 'sync_enabled')) ? ElectrixTweaker.deckColor[group]['switches'] : ElectrixTweaker.colorCodes['off']
	)
}

ElectrixTweaker.key = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		if (engine.getValue(group, 'file_key') != engine.getValue(group, 'key')) {
			engine.setValue(group, 'reset_key', 1)
		} else if (ElectrixTweaker.shift) {
			engine.setValue(group, 'sync_key', 1)
		} else {
			engine.setValue(group, 'keylock', ! engine.getValue(group, 'keylock'))
		}
	}
}
ElectrixTweaker.keyLED = function (value, group, control) {
	if (engine.getValue(group, 'file_key') != engine.getValue(group, 'key')) {
		midi.sendShortMsg(
			0x90,
			ElectrixTweaker.buttons[group]['key'],
			ElectrixTweaker.colorCodes['white']
		)
	} else {
		midi.sendShortMsg(
			0x90,
			ElectrixTweaker.buttons[group]['key'],
			(engine.getValue(group, 'keylock')) ? ElectrixTweaker.deckColor[group]['switches'] : ElectrixTweaker.colorCodes['off']
		)
	}
}


ElectrixTweaker.quantize = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		if (ElectrixTweaker.shift) {
			engine.setValue(group, 'beats_translate_curpos', 1)
		} else {
			engine.setValue(group, 'quantize', ! engine.getValue(group, 'quantize'))
			
			midi.sendShortMsg(
				0x90,
				ElectrixTweaker.buttons[group]['back'],
				(engine.getValue(group, 'quantize')) ? ElectrixTweaker.colorCodes['white'] : ElectrixTweaker.colorCodes['green']
			)
			midi.sendShortMsg(
				0x90,
				ElectrixTweaker.buttons[group]['forward'],
				(engine.getValue(group, 'quantize')) ? ElectrixTweaker.colorCodes['white'] : ElectrixTweaker.colorCodes['green']
			)
		}
	}
}
ElectrixTweaker.quantizeLED = function (value, group, control) {
	midi.sendShortMsg(
		0x90,
		ElectrixTweaker.buttons[group]['quantize'],
		(engine.getValue(group, 'quantize')) ? ElectrixTweaker.deckColor[group]['switches'] : ElectrixTweaker.colorCodes['off']
	)
}