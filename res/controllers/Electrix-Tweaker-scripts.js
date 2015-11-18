// ====================================================== USER OPTIONS =========================================================

// Set this to false if you do not want the volume of samples to be proportional to how hard you press the big buttons. When this is false, the sampler buttons will play the samplers at whatever volume the sample deck is set to.
ElectrixTweaker.samplerVelocityAsVolume = true
// The higher this is, the less hard you have to strike the sample pads to play samples loudly (when ElectrixTweaker.samplerVelocityAsVolume is true).
ElectrixTweaker.samplerSensitivity = 4

// Adjust sensitivity of EQs (range 1-7, only use integers)
ElectrixTweaker.eqSensitivity = 6

// Set these to true to enable vinyl mode for that deck on startup. This will also enable vinyl control on startup.
ElectrixTweaker.vinylMode = {'[Channel1]': false, '[Channel2]': false, '[Channel3]': false, '[Channel4]': false}

/**
 * Electrix Tweaker controller script 1.1.1 for Mixxx 1.12
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

 See http://mixxx.org/wiki/doku.php/electrix_tweaker for instructions on how to use this mapping
**/

// ==================================================== GLOBAL VARIABLES =======================================================

function ElectrixTweaker() {}

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
	'[Channel1]': ElectrixTweaker.colorCodes['blue'],
	'[Channel2]': ElectrixTweaker.colorCodes['blue'],
	'[Channel3]': ElectrixTweaker.colorCodes['magenta'],
	'[Channel4]': ElectrixTweaker.colorCodes['magenta']
}
ElectrixTweaker.hotcueColors = [
	ElectrixTweaker.colorCodes['cyan'],
	ElectrixTweaker.colorCodes['green'],
	ElectrixTweaker.colorCodes['red'],
	ElectrixTweaker.colorCodes['white']
]

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
ElectrixTweaker.encoderRingStepsFill = [0, 1, 20, 30, 40, 50, 60, 64, 75, 85, 95, 105, 120, 127]
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
ElectrixTweaker.deckShift = {'[Channel1]': false, '[Channel2]': false, '[Channel3]': false, '[Channel4]': false}
ElectrixTweaker.deck = {'[Channel1]': '[Channel1]', '[Channel2]': '[Channel2]'}
ElectrixTweaker.mode = {'[Channel1]': 'eq', '[Channel2]': 'eq', '[Channel3]': 'eq', '[Channel4]': 'eq'}
ElectrixTweaker.loopMoveSize = {'[Channel1]': 1, '[Channel2]': 1, '[Channel3]': 1, '[Channel4]': 1}
ElectrixTweaker.loopSize = {'[Channel1]': 4, '[Channel2]': 4, '[Channel3]': 4, '[Channel4]': 4}
ElectrixTweaker.slipMode = {'[Channel1]': false, '[Channel2]': false, '[Channel3]': false, '[Channel4]': false}
ElectrixTweaker.deckShift = {'[Channel1]': false, '[Channel2]': false, '[Channel3]': false, '[Channel4]': false}
ElectrixTweaker.hotcuePage = {'[Channel1]': 0, '[Channel2]': 0, '[Channel3]': 0, '[Channel4]': 0}
ElectrixTweaker.hotcuesPressed = {'[Channel1]': 0, '[Channel2]': 0, '[Channel3]': 0, '[Channel4]': 0}
ElectrixTweaker.playPressedWhileCueJuggling = {'[Channel1]': false, '[Channel2]': false, '[Channel3]': false, '[Channel4]': false}
ElectrixTweaker.slipModeUnsetWhileLooping = {'[Channel1]': false, '[Channel2]': false, '[Channel3]': false, '[Channel4]': false}
ElectrixTweaker.midEncoderLEDTimer = {'[Channel1]': 0, '[Channel2]': 0, '[Channel3]': 0, '[Channel4]': 0}
ElectrixTweaker.midEncoderLEDTimer = {'[Channel1]': 0, '[Channel2]': 0, '[Channel3]': 0, '[Channel4]': 0}

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

// ================================================= INITIALIZATION & SHUTDOWN ============================================

ElectrixTweaker.init = function () {
	if (engine.getValue('[Master]', 'num_samplers') < 8) {
		engine.setValue('[Master]', 'num_samplers', 8)
	}
	for (var group in ElectrixTweaker.encoders) { // loop over each [Channel]
// 		engine.softTakeover('[QuickEffectRack1_'+group+']', 'super1', true)
// 		engine.softTakeover(group, 'volume', true)
		// uncomment the line below when Bug #1472868 is fixed
// 		ElectrixTweaker.vinylMode[group] = engine.getValue(group, 'vinylcontrol_enabled')
		engine.setValue(group, 'vinylcontrol_enabled', ElectrixTweaker.vinylMode[group])
	}
	ElectrixTweaker.initDeck('[Channel1]')
	ElectrixTweaker.initDeck('[Channel2]')
	for (i=1; i<=8; i++) {
		engine.connectControl('[Sampler'+i+']', 'track_samples', 'ElectrixTweaker.oneShotLED')
		engine.trigger('[Sampler'+i+']', 'track_samples')
	}
	
	midi.sendShortMsg(0x90, 39, 127) // light up arrow
	midi.sendShortMsg(0x90, 40, 127) // light top shift button
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
}

// ==================================================== MODE SWITCHING FUNCTIONS ================================================

ElectrixTweaker.initDeck = function (group) {
	var disconnectDeck = parseInt(ElectrixTweaker.channelRegEx.exec(group)[1])
	if (disconnectDeck <= 2) {
		disconnectDeck += 2
	} else {
		disconnectDeck -= 2
	}
	disconnectDeck = '[Channel' + disconnectDeck + ']'
	ElectrixTweaker.connectDeckControls(disconnectDeck, true)
	// workaround for https://bugs.launchpad.net/mixxx/+bug/1466606
	for (i=1; i <= 32; i++) {
		engine.setValue(disconnectDeck, 'hotcue_'+i+'_activate', 0)
	}
	
	midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['shift'], ElectrixTweaker.colorCodes['yellow'])
	midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['deckToggle'], ElectrixTweaker.deckColor[group])
	midi.sendShortMsg(
		0x90,
		ElectrixTweaker.buttons[group]['slip'],
		ElectrixTweaker.slipMode[group] ? ElectrixTweaker.deckColor[group] : ElectrixTweaker.colorCodes['off']
	)
	
	ElectrixTweaker.mode[group] = ElectrixTweaker.mode[disconnectDeck]
	
	ElectrixTweaker.connectDeckControls(group)
}

ElectrixTweaker.connectDeckControls = function (group, remove) {
	remove = (typeof remove !== 'undefined') ? remove : false // default value for remove is false
	
	var controlsToFunctions = {
		'pfl': 'ElectrixTweaker.pflButtonLED',
// 		'track_samples': 'ElectrixTweaker.arrowSideLED', // the line below would overwrite this attribute
		'track_samples': 'ElectrixTweaker.playButtonLED',
		'play': 'ElectrixTweaker.playButtonLED',
		'playposition': 'ElectrixTweaker.playButtonLED',
		'sync_enabled': 'ElectrixTweaker.syncLED',
		'keylock': 'ElectrixTweaker.keylockLED',
		'quantize': 'ElectrixTweaker.quantizeLED'
	}
	engine.connectControl(group, 'track_samples', 'ElectrixTweaker.arrowSideLED', remove)
	for (var control in controlsToFunctions) {
		engine.connectControl(group, control, controlsToFunctions[control], remove)
		if (! remove) {
			engine.trigger(group, control)
		}
	}
	ElectrixTweaker.connectHotcuePage(group, remove)
	ElectrixTweaker.connectEncoderMode(group, ElectrixTweaker.mode[group], remove)
	
	if (ElectrixTweaker.vinylMode[group]) {
		ElectrixTweaker.connectVinylLEDs(group, remove)
	}
}

ElectrixTweaker.connectEncoderMode = function (group, mode, remove) {
	remove = (typeof remove !== 'undefined') ? remove : false // default value for remove is false
	switch (mode) {
		case 'eq':
			midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['mode'], ElectrixTweaker.colorCodes['white'])
			for (var encoder in ElectrixTweaker.encoders[group]) {
				engine.connectControl(group, 'filter' + encoder, 'ElectrixTweaker.eqEncoderLEDs', remove)
				engine.connectControl(group, 'filter' + encoder + 'Kill', 'ElectrixTweaker.eqEncoderKillButtonLED', remove)
				if (! remove) {
					// set encoder to absolute EQ mode with speed 5
					midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group][encoder]['cc'], 70 + 8*ElectrixTweaker.eqSensitivity)
					// enable local control of LED ring
					midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group][encoder]['ring'], 70)
					
					engine.trigger(group, 'filter' + encoder)
					engine.trigger(group, 'filter' + encoder + 'Kill')
				}
			}
			break
		case 'loop':
			engine.connectControl(group, 'loop_enabled', 'ElectrixTweaker.loopButtonToggleLED', remove)
			if (! remove) {
				engine.trigger(group, 'loop_enabled')
				midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['mode'], ElectrixTweaker.colorCodes['magenta'])
				for (var encoder in ElectrixTweaker.encoders[group]) {
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
					0xB0,
					ElectrixTweaker.encoders[group]['Low']['ring'],
					ElectrixTweaker.encoderRingSteps[
						6 + Math.log(ElectrixTweaker.loopSize[group]) / Math.log(2)
					]
				)
			}
			break
	}
}

ElectrixTweaker.connectHotcuePage = function (group, remove) {
	remove = (typeof remove !== 'undefined') ? remove : false // default value for remove is false
	
	var min = 1 + (ElectrixTweaker.hotcuePage[group] * 8)
	var max = min + 7
	for (i=min; i<=max; i++) {
		engine.connectControl(group, 'hotcue_'+i+'_enabled', 'ElectrixTweaker.hotcueLED', remove)
		if (! remove) {
			engine.trigger(group, 'hotcue_'+i+'_enabled')
		}
	}
}

ElectrixTweaker.connectVinylLEDs = function (group, remove) {
	var controlsToFunctions = {
		'passthrough': 'ElectrixTweaker.vinylStatusLED',
		'vinylcontrol_status': 'ElectrixTweaker.vinylStatusLED',
		'vinylcontrol_mode': 'ElectrixTweaker.vinylModeLED',
		'vinylcontrol_cueing': 'ElectrixTweaker.vinylModeLED'
	}
	for (var control in controlsToFunctions) {
		engine.connectControl(group, control, controlsToFunctions[control], remove)
	}
	if (remove) {
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
		midi.sendShortMsg(
			0x90,
			ElectrixTweaker.buttons[group]['slip'],
			(ElectrixTweaker.slipMode[group]) ? ElectrixTweaker.deckColor[group] : ElectrixTweaker.colorCodes['off']
		)
	} else {
		for (var control in controlsToFunctions) {
			engine.trigger(group, control)
		}
	}
}

ElectrixTweaker.topShiftButton = function (channel, control, value, status, group) {
// 	ElectrixTweaker.shift = ! ElectrixTweaker.shift
	ElectrixTweaker.topShift = ! ElectrixTweaker.topShift

	ElectrixTweaker.connectEncoderMode(ElectrixTweaker.deck['[Channel1]'], ElectrixTweaker.mode[group], value/127)
	ElectrixTweaker.connectEncoderMode(ElectrixTweaker.deck['[Channel2]'], ElectrixTweaker.mode[group], value/127)
	if (value) {
		for (group in ElectrixTweaker.deck) {
			for (var encoder in ElectrixTweaker.encoders[group]) {
				// set encoder to absolute EQ mode with speed 5
				midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group][encoder]['cc'], 70 + 8*ElectrixTweaker.eqSensitivity)
				// enable local control of LED ring
				midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group][encoder]['ring'], 70)
			}
			engine.connectControl(ElectrixTweaker.deck[group], 'pregain', 'ElectrixTweaker.gainLEDs')
			engine.trigger(ElectrixTweaker.deck[group], 'pregain')
		}
	} else {
		for (group in ElectrixTweaker.deck) {
			engine.connectControl(ElectrixTweaker.deck[group], 'pregain', 'ElectrixTweaker.gainLEDs', true)
		}
	}
	
	var controlsToFunctions = {
		'volume': 'ElectrixTweaker.masterGainLEDs',
		'balance': 'ElectrixTweaker.masterBalanceLEDs',
		'headVolume': 'ElectrixTweaker.headGainLEDs',
		'headMix': 'ElectrixTweaker.headMixLEDs',
		'headSplit': 'ElectrixTweaker.headSplitLED'
	}
	for (var control in controlsToFunctions) {
		engine.connectControl('[Master]', control, controlsToFunctions[control], ! value/127)
		if (value) {
			engine.trigger('[Master]', control)
		}
	}
}

ElectrixTweaker.deckShiftButton = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	ElectrixTweaker.deckShift[group] = ! ElectrixTweaker.deckShift[group]
	if (value) {
		// set low encoder to relative mode
		midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group]['Low']['cc'], 64)
		// set low LED ring to walk mode with local control disabled
		midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group]['Low']['ring'], 96)
		midi.sendShortMsg(
			0xB0,
			ElectrixTweaker.encoders[group]['Low']['ring'],
			ElectrixTweaker.encoderRingStepsFill[ElectrixTweaker.hotcuePage[group]+1]
		)
		// set mid encoder to relative mode
		midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group]['Mid']['cc'], 64)
		// set mid encoder LED ring to EQ mode with local control disabled
		// There seems to be a bug in the Tweaker firmware when local control is enabled one LED ring but not another. If local control is enabled here, the other rings behave confusingly.
		midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group]['Mid']['ring'], 98)
		midi.sendShortMsg(0xB0, ElectrixTweaker.encoders[group]['Mid']['ring'], 64)
		// set high encoder to absolute EQ mode with sensitivity level 1
		// 1 step = 1 MIDI value for fine pitch control
		midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group]['High']['cc'], 78)
		// set high LED ring to EQ mode with local control disabled
		midi.sendShortMsg(0xBF, ElectrixTweaker.encoders[group]['High']['ring'], 78)
		engine.connectControl(group, 'rate', 'ElectrixTweaker.rateEncoderLEDs')
		engine.trigger(group, 'rate')
		if (ElectrixTweaker.topShift && ElectrixTweaker.deckShift[group]) {
			ElectrixTweaker.connectVinylLEDs(group, ElectrixTweaker.vinylMode[group])
			ElectrixTweaker.vinylMode[group] = ! ElectrixTweaker.vinylMode[group]
		}
	} else {
		engine.stopTimer(ElectrixTweaker.midEncoderLEDTimer[group])
		engine.connectControl(group, 'rate', 'ElectrixTweaker.rateEncoderLEDs', true)
	}
	ElectrixTweaker.connectEncoderMode(group, ElectrixTweaker.mode[group], value/127)
}

ElectrixTweaker.rateEncoderLEDs = function (value, group, control) {
	midi.sendShortMsg(
						0xB0,
						ElectrixTweaker.encoders[group]['High']['cc'],
						script.absoluteLinInverse(value, -1, 1)
					 )
}

// ================================================== ARROWS + BIG ENCODER ====================================================

ElectrixTweaker.bigEncoder = function (channel, control, value, status, group) {
	if (ElectrixTweaker.topShift) {
		for (i=0 ; i<35; i++) {
			engine.setValue('[Playlist]', (value == 1) ? 'SelectNextTrack' : 'SelectPrevTrack', 1)
		}
	} else {
		engine.setValue('[Playlist]', (value == 1) ? 'SelectNextTrack' : 'SelectPrevTrack', 1)
	}
}
ElectrixTweaker.bigEncoderButton = function (channel, control, value, status, group) {
	if (value) {
		if (ElectrixTweaker.topShift) {
			engine.setValue('[Playlist]', 'LoadSelectedIntoFirstStopped', 1)
		} else {
			engine.setValue('[Master]', 'maximize_library', ! engine.getValue('[Master]', 'maximize_library'))
		}
	}
}
ElectrixTweaker.arrowSide = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		if (ElectrixTweaker.topShift) {
				engine.setValue(group, 'eject', 1)
				engine.beginTimer(250, 'engine.setValue("'+group+'", "eject", 0)', true)
		} else {
			engine.setValue(group, 'LoadSelectedTrack', 1)
		}
	}
}
ElectrixTweaker.arrowSideLED = function (value, group, control) {
	midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['arrowSide'], (value) ? 127 : 0)
}
ElectrixTweaker.arrowUp = function (channel, control, value, status, group) {
	if (value) {
		if (ElectrixTweaker.topShift) {
			engine.setValue('[Playlist]', 'ToggleSelectedSidebarItem', 1)
		} else {
			engine.setValue('[Playlist]', 'SelectPrevPlaylist', 1)
		}
	}
}
ElectrixTweaker.arrowDown = function (channel, control, value, status, group) {
	if (value) {
		if (ElectrixTweaker.topShift) {
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
			if (ElectrixTweaker.topShift) {
				engine.setValue(group, 'key', 0)
				engine.setValue(group, 'sync_enabled', 0)
				engine.setValue(group, 'repeat', 0)
				engine.setValue(group, 'play', 0)
				engine.setValue(group, 'eject', 1)
				engine.beginTimer(250, 'engine.setValue("'+group+'", "eject", 0)', true)
			} else {
				if (ElectrixTweaker.samplerVelocityAsVolume) {
					engine.setValue(group, 'volume', script.absoluteNonLin(value * ElectrixTweaker.samplerSensitivity, 0, .25, 1))
				}
				engine.setValue(group, 'playposition', 0)
				engine.setValue(group, 'play', 1)
			}
		} else {
			if (ElectrixTweaker.samplerVelocityAsVolume) {
				engine.setValue(group, 'volume', script.absoluteNonLin(value, 0, .25, 1))
			}
			engine.setValue(group, 'LoadSelectedTrackAndPlay', 1)
		}
	} else {
		engine.setValue(group, 'play', 0)
	}
}
ElectrixTweaker.oneShotLED = function (value, group, control) {
	midi.sendShortMsg(0x90, 62 + parseInt(ElectrixTweaker.samplerRegEx.exec(group)[1]), (value) ? 127 : 0)
}

// ================================================= CHANNEL STRIPS ===========================================================

ElectrixTweaker.leftKnob = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	// soft takeover
	if (Math.abs(script.absoluteLin(value, 0, 1) - engine.getValue('[QuickEffectRack1_'+group+']', 'super1')) < .1) {
		engine.setValue('[QuickEffectRack1_'+group+']', 'super1', script.absoluteLin(value, 0, 1))
	}
}
ElectrixTweaker.rightKnob = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	// soft takeover
	if (Math.abs(script.absoluteLin(value, 0, 1) - engine.getValue('[QuickEffectRack1_'+group+']', 'super1')) < .1) {
		engine.setValue('[QuickEffectRack1_'+group+']', 'super1', script.absoluteLin(value, 0, 1))
	}
}

ElectrixTweaker.eqEncoderLEDs = function (value, group, control) {
	var encoder = control.replace('filter', '')
	midi.sendShortMsg(0xB0, ElectrixTweaker.encoders[group][encoder]['cc'], script.absoluteNonLinInverse(value, 0, 1, 4))
}

ElectrixTweaker.eqEncoderKillButtonLED = function (value, group, control) {
	var encoder = control.replace('filter', '')
	encoder = encoder.replace('Kill', '')
	midi.sendShortMsg(0x90, ElectrixTweaker.encoders[group][encoder]['button'], value * 127)
}

ElectrixTweaker.masterGainLEDs = function (value, group, control) {
	midi.sendShortMsg(0xB0, ElectrixTweaker.encoders['[Channel2]']['High']['cc'], script.absoluteNonLinInverse(value, 0, 1, 4))
}

ElectrixTweaker.masterBalanceLEDs = function (value, group, control) {
	midi.sendShortMsg(0xB0, ElectrixTweaker.encoders['[Channel2]']['Mid']['cc'], script.absoluteLinInverse(value, -1, 1))
}

ElectrixTweaker.headGainLEDs = function (value, group, control) {
	midi.sendShortMsg(0xB0, ElectrixTweaker.encoders['[Channel1]']['High']['cc'], script.absoluteNonLinInverse(value, 0, 1, 4))
}

ElectrixTweaker.headMixLEDs = function (value, group, control) {
	midi.sendShortMsg(0xB0, ElectrixTweaker.encoders['[Channel1]']['Mid']['cc'], script.absoluteLinInverse(value, -1, 1))
}

ElectrixTweaker.headSplitLED = function (value, group, control) {
	midi.sendShortMsg(0x90, ElectrixTweaker.encoders['[Channel1]']['Mid']['button'], value * 127)
}

ElectrixTweaker.gainLEDs = function (value, group, control) {
	midi.sendShortMsg(0xB0, ElectrixTweaker.encoders[group]['Low']['cc'], script.absoluteNonLinInverse(value, 0, 1, 4))
}

ElectrixTweaker.highEncoder = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (ElectrixTweaker.topShift) {
		if (control == ElectrixTweaker.encoders['[Channel1]']['High']['cc']) {
			engine.setValue('[Master]', 'headVolume', script.absoluteNonLin(value, 0, 1, 5))
		} else {
			engine.setValue('[Master]', 'volume', script.absoluteNonLin(value, 0, 1, 5))
		}
	} else if (ElectrixTweaker.deckShift[group]) {
		engine.setValue(group, 'rate', script.absoluteLin(value, -1, 1, 0, 126))
	} else {
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
}
ElectrixTweaker.highEncoderPress = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		if (ElectrixTweaker.topShift) {
			if (control == ElectrixTweaker.encoders['[Channel1]']['High']['button']) {
				engine.setValue('[Master]', 'headVolume', 1)
			} else {
				engine.setValue('[Master]', 'volume', 1)
			}
		} else {
			switch (ElectrixTweaker.mode[group]) {
				case 'eq':
					if (ElectrixTweaker.deckShift[group]) {
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
}
ElectrixTweaker.midEncoder = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (ElectrixTweaker.topShift) {
		if (control == ElectrixTweaker.encoders['[Channel1]']['Mid']['cc']) {
			engine.setValue('[Master]', 'headMix', script.absoluteLin(value, -1, 1))
		} else {
			engine.setValue('[Master]', 'balance', script.absoluteLin(value, -1, 1))
		}
	} else if (ElectrixTweaker.deckShift[group]) {
		engine.stopTimer(ElectrixTweaker.midEncoderLEDTimer[group])
		if (value == 127) {
			engine.setValue(group, 'beatjump_32_backward', 1)
			midi.sendShortMsg(0xB0, ElectrixTweaker.encoders[group]['Mid']['ring'], 0)
		} else {
			engine.setValue(group, 'beatjump_32_forward', 1)
			midi.sendShortMsg(0xB0, ElectrixTweaker.encoders[group]['Mid']['ring'], 127)
		}
		ElectrixTweaker.midEncoderLEDTimer[group] = engine.beginTimer(1000, 'midi.sendShortMsg(0xB0, ElectrixTweaker.encoders["'+group+'"]["Mid"]["ring"], 64)', true)
	} else {
		switch (ElectrixTweaker.mode[group]) {
			case 'eq':
				engine.setValue(group, 'filterMid', script.absoluteNonLin(value, 0, 1, 4))
				break
			case 'loop':
				engine.stopTimer(ElectrixTweaker.midEncoderLEDTimer[group])
				if (value == 127) {
					engine.setValue(group, 'loop_move_' + ElectrixTweaker.loopMoveSize[group] + '_backward', 1)
					midi.sendShortMsg(0xB0, ElectrixTweaker.encoders[group]['Mid']['ring'], 0)
					
				} else {
					engine.setValue(group, 'loop_move_' + ElectrixTweaker.loopMoveSize[group] + '_forward', 1)
					midi.sendShortMsg(0xB0, ElectrixTweaker.encoders[group]['Mid']['ring'], 127)
				}
				ElectrixTweaker.midEncoderLEDTimer[group] = engine.beginTimer(1000, 'midi.sendShortMsg(0xB0, ElectrixTweaker.encoders["'+group+'"]["Mid"]["ring"], 64)', true)
				break
		}
	}
}
ElectrixTweaker.midEncoderPress = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		if (ElectrixTweaker.topShift) {
			if (control == ElectrixTweaker.encoders['[Channel1]']['Mid']['button']) {
				engine.setValue('[Master]', 'headSplit', ! engine.getValue('[Master]', 'headSplit'))
			} else {
				engine.setValue('[Master]', 'balance', 0)
			}
		} else {
			switch (ElectrixTweaker.mode[group]) {
				case 'eq':
					if (ElectrixTweaker.deckShift[group]) {
						engine.setValue(group, 'filterMid', 1)
					} else {
						engine.setValue(group, 'filterMidKill', ! engine.getValue(group, 'filterMidKill'))
					}
					break
				case 'loop':
					// What to do with this?
			}
		}
	}
}
ElectrixTweaker.lowEncoder = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (ElectrixTweaker.topShift) {
		engine.setValue(group, 'pregain', script.absoluteNonLin(value, 0, 1, 4))
	} else if (ElectrixTweaker.deckShift[group]) {
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
			ElectrixTweaker.encoders[group]['Low']['ring'],
			ElectrixTweaker.encoderRingStepsFill[ElectrixTweaker.hotcuePage[group]+1]
		)
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
		if (ElectrixTweaker.topShift) {
			engine.setValue(group, 'pregain', 1)
		} else {
			switch (ElectrixTweaker.mode[group]) {
				case 'eq':
					if (ElectrixTweaker.deckShift[group]) {
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
	} else if (ElectrixTweaker.mode[group] == 'loop' && (ElectrixTweaker.slipMode[group] || ElectrixTweaker.slipModeUnsetWhileLooping[group])) {
		engine.setValue(group, 'slip_enabled', ! engine.getValue(group, 'slip_enabled'))
		if (engine.getValue(group, 'loop_enabled')) {
			engine.setValue(group, 'reloop_exit', 1)
		}
		ElectrixTweaker.slipModeUnsetWhileLooping[group] = false
	}
}
ElectrixTweaker.loopButtonToggleLED = function (value, group, control) {
	midi.sendShortMsg(0x90, ElectrixTweaker.encoders[group]['Low']['button'], value * 127)
}

ElectrixTweaker.modeButton = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		ElectrixTweaker.connectEncoderMode(group, ElectrixTweaker.mode[group], true)
		switch (ElectrixTweaker.mode[group]) {
			case 'eq':
				ElectrixTweaker.mode[group] = 'loop'
				break
			case 'loop':
				ElectrixTweaker.mode[group] = 'eq'
				break
		}
		ElectrixTweaker.connectEncoderMode(group, ElectrixTweaker.mode[group])
	}
}

ElectrixTweaker.fader = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	// soft takeover. This is necessary for toggling between decks 1/3 or 2/4
	// because the fader can't be moved to the new deck's value by this script
	if (Math.abs(value - script.absoluteNonLinInverse(engine.getValue(group, 'volume'), 0, .25, 1)) < 30) {
		engine.setValue(group, 'volume', script.absoluteNonLin(value, 0, .25, 1))
	}
}

ElectrixTweaker.pflButton = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		if (ElectrixTweaker.deckShift[group]) {
			engine.setValue(group, 'reloop_exit', 1)
		} else {
			engine.setValue(group, 'pfl', ! engine.getValue(group, 'pfl'))
		}
	}
}
ElectrixTweaker.pflButtonLED = function (value, group, control) {
	midi.sendShortMsg(
		0x90,
		ElectrixTweaker.buttons[group][control],
		(value) ? ElectrixTweaker.colorCodes['green'] : ElectrixTweaker.colorCodes['off']
	)
}

ElectrixTweaker.playButton = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (ElectrixTweaker.deckShift[group]) {
		engine.setValue(group, 'cue_default', value)
	} else if (ElectrixTweaker.topShift) {
		engine.setValue(group, 'cue_gotoandstop', 1)
	} else if (value) {
		if (ElectrixTweaker.hotcuesPressed[group]) {
			ElectrixTweaker.playPressedWhileCueJuggling[group] = true
		}
		
		engine.setValue(group, 'play', ! engine.getValue(group, 'play'))
	}
}
ElectrixTweaker.playButtonLED = function (value, group, control) {
	if (engine.getValue(group, 'play')) {
		if (
			(control != 'playposition') // do not spam MIDI signals with each update in playposition while playing
			&& (
				(! ElectrixTweaker.hotcuesPressed[group])
				|| ElectrixTweaker.playPressedWhileCueJuggling[group]
			)
		) {
			midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['play'], ElectrixTweaker.colorCodes['green'])
		}
	} else if (engine.getValue(group, 'track_samples')) {
		midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['play'], ElectrixTweaker.colorCodes['red'])
	} else {
		midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['play'], ElectrixTweaker.colorCodes['off'])
	}
}


//===================================================================== BUTTON GRID =========================================================

ElectrixTweaker.hotcue = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	var row = (control < ElectrixTweaker.buttons[group]['hotcues'][1][0]) ? 0 : 1
	var cueButton = 4 + row*4 - (ElectrixTweaker.buttons[group]['hotcues'][row][3] - control)
	var cue = cueButton + (8 * ElectrixTweaker.hotcuePage[group])
	if (value) {
		if (engine.getValue(group, 'hotcue_'+cue+'_enabled')) {
			if (ElectrixTweaker.deckShift[group]) {
				engine.setValue(group, 'hotcue_'+cue+'_set', 1)
			} else if (ElectrixTweaker.topShift) {
				engine.setValue(group, 'hotcue_'+cue+'_clear', 1)
			} else {
				if (ElectrixTweaker.slipMode[group]) {
					if (engine.getValue(group, 'play') && (! ElectrixTweaker.hotcuesPressed[group])) {
						engine.setValue(group, 'slip_enabled', 1)
					}
					engine.setValue(group, 'hotcue_'+cue+'_activate', 1)
					ElectrixTweaker.hotcuesPressed[group]++
				} else {
					engine.setValue(group, 'hotcue_'+cue+'_goto', 1)
				}
			}
		} else {
			engine.setValue(group, 'hotcue_'+cue+'_set', 1)
		}
	} else {
		if (ElectrixTweaker.hotcuesPressed[group]) { // do not go below 0
			ElectrixTweaker.hotcuesPressed[group]--
		}
		engine.setValue(group, 'hotcue_'+cue+'_activate', 0)

		if (! ElectrixTweaker.hotcuesPressed[group]) {
			ElectrixTweaker.playPressedWhileCueJuggling[group] = false
			engine.setValue(group, 'slip_enabled', 0)
		}
	}
}
ElectrixTweaker.hotcueLED = function (value, group, control) {
	var cue = parseInt(control.split('_')[1]) - (8 * ElectrixTweaker.hotcuePage[group])
	var row = (cue <= 4) ? 0 : 1
	midi.sendShortMsg(
		0x90,
		ElectrixTweaker.buttons[group]['hotcues'][row][cue - 1 - 4*row],
		value * ElectrixTweaker.hotcueColors[ElectrixTweaker.hotcuePage[group]]
	)
}

ElectrixTweaker.slipButton = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		if (ElectrixTweaker.deckShift[group]) {
			engine.setValue(group, 'loop_in', 1)
		} else {
			if (ElectrixTweaker.slipMode[group]) {
				if (ElectrixTweaker.hotcuesPressed[group] && ! engine.getValue(group, 'slip_enabled')) {
					engine.setValue(group, 'play', 0)
				}
				if (engine.getValue(group, 'loop_enabled')) {
					ElectrixTweaker.slipModeUnsetWhileLooping[group] = true
				}
				//engine.setValue(group, 'slip_cancel', 1)
				ElectrixTweaker.hotcuesPressed[group] = 0
			}

			
			ElectrixTweaker.slipMode[group] = ! ElectrixTweaker.slipMode[group]
			midi.sendShortMsg(
				0x90,
				ElectrixTweaker.buttons[group]['slip'],
				ElectrixTweaker.slipMode[group] ? ElectrixTweaker.deckColor[group] : ElectrixTweaker.colorCodes['off']
			)
		}
	}
}

ElectrixTweaker.deckToggle = function (channel, control, value, status, group) {
	if (value) {
		if (ElectrixTweaker.deckShift[group]) {
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
		if (ElectrixTweaker.deckShift[group]) {
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
		(value) ? ElectrixTweaker.deckColor[group] : ElectrixTweaker.colorCodes['off']
	)
}

ElectrixTweaker.key = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		if (ElectrixTweaker.deckShift[group]) {
			if (engine.getValue(group, 'file_key') != engine.getValue(group, 'key')) {
				engine.setValue(group, 'reset_key', 1)
			} else {
				engine.setValue(group, 'sync_key', 1)
			}
		} else {
			engine.setValue(group, 'keylock', ! engine.getValue(group, 'keylock'))
		}
	}
}
ElectrixTweaker.keylockLED = function (value, group, control) {
	midi.sendShortMsg(
		0x90,
		ElectrixTweaker.buttons[group]['key'],
		(value) ? ElectrixTweaker.deckColor[group] : ElectrixTweaker.colorCodes['off']
	)
}

ElectrixTweaker.quantize = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (value) {
		if (ElectrixTweaker.deckShift[group]) {
			engine.setValue(group, 'beats_translate_curpos', 1)
		} else {
			engine.setValue(group, 'quantize', ! engine.getValue(group, 'quantize'))
		}
	}
}
ElectrixTweaker.quantizeLED = function (value, group, control) {
	midi.sendShortMsg(
		0x90,
		ElectrixTweaker.buttons[group]['quantize'],
		(value) ? ElectrixTweaker.deckColor[group] : ElectrixTweaker.colorCodes['off']
	)
	if (! ElectrixTweaker.vinylMode[group]) {
		midi.sendShortMsg(
			0x90,
			ElectrixTweaker.buttons[group]['back'],
			(value) ? ElectrixTweaker.colorCodes['white'] : ElectrixTweaker.colorCodes['green']
		)
		midi.sendShortMsg(
			0x90,
			ElectrixTweaker.buttons[group]['forward'],
			(value) ? ElectrixTweaker.colorCodes['white'] : ElectrixTweaker.colorCodes['green']
		)
	}
}

ElectrixTweaker.vinylModeLED = function (value, group, control) {
	var color
	switch (engine.getValue(group, 'vinylcontrol_mode')) {
		// absolute mode
		case 0: color = 'off'; break
		// relative mode
		case 1:
			switch (engine.getValue(group, 'vinylcontrol_cueing')) {
				case 0: color = 'white'; break
				case 1: color = 'yellow'; break
				case 2: color = 'green'; break
			}
			break
		// constant mode
		case 2: color = 'red'; break
	}
	midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['forward'], ElectrixTweaker.colorCodes[color])
}
ElectrixTweaker.vinylStatusLED = function (value, group, control) {
	if (engine.getValue(group, 'passthrough')) {
		midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['back'], ElectrixTweaker.colorCodes['white'])
	} else {
		var color
		switch (engine.getValue(group, 'vinylcontrol_status')) {
			case 0: color = 'off'; break
			case 1: color = 'green'; break
			case 2: color = 'yellow'; break
			case 3: color = 'red'; break
		}
		midi.sendShortMsg(0x90, ElectrixTweaker.buttons[group]['back'], ElectrixTweaker.colorCodes[color])
	}
}

ElectrixTweaker.forward = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (ElectrixTweaker.vinylMode[group]) {
		if (value) {
			switch (engine.getValue(group, 'vinylcontrol_mode')) {
				// absolute mode
				case 0: engine.setValue(group, 'vinylcontrol_mode', 1); break
				// relative mode
				case 1:
					if (engine.getValue(group, 'play') && ! ElectrixTweaker.shift) {
						switch (engine.getValue(group, 'vinylcontrol_cueing')) {
							case 0: engine.setValue(group, 'vinylcontrol_cueing', 1); break
							case 1: engine.setValue(group, 'vinylcontrol_cueing', 2); break
							case 2: engine.setValue(group, 'vinylcontrol_cueing', 0); break
						}
					} else {
						engine.setValue(group, 'vinylcontrol_mode', 2)
					}
					break
				// constant mode
				case 2: engine.setValue(group, 'vinylcontrol_mode', 0); break
			}
		}
	} else {
		if (engine.getValue(group, 'quantize')) {
			if (value) {
				if (ElectrixTweaker.deckShift[group]) {
					engine.setValue(group, 'beatjump_1_forward', 1)
				} else {
					engine.setValue(group, 'beatjump_4_forward', 1)
				}
			}
		} else {
			if (ElectrixTweaker.deckShift[group]) {
				engine.setValue(group, 'rate_temp_up', value / 127)
			} else {
				engine.setValue(group, 'fwd', value)
			}
		}
	}
}
ElectrixTweaker.back = function (channel, control, value, status, group) {
	group = ElectrixTweaker.deck[group]
	if (ElectrixTweaker.vinylMode[group]) {
		if (value) {
			if (ElectrixTweaker.deckShift[group] || engine.getValue(group, 'passthrough')) {
				engine.setValue(group, 'passthrough', ! engine.getValue(group, 'passthrough'))
			} else {
				engine.setValue(group, 'vinylcontrol_enabled', ! engine.getValue(group, 'vinylcontrol_enabled'))
			}
		}
	} else {
		if (engine.getValue(group, 'quantize')) {
			if (value) {
				if (ElectrixTweaker.deckShift[group]) {
					engine.setValue(group, 'beatjump_1_backward', 1)
				} else {
					engine.setValue(group, 'beatjump_4_backward', 1)
				}
			}
		} else {
			if (ElectrixTweaker.deckShift[group]) {
				engine.setValue(group, 'rate_temp_down', value / 127)
			} else {
				engine.setValue(group, 'back', value)
			}
		}
	}
}