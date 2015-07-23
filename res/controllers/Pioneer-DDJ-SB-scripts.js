var PioneerDDJSB = function() { }

PioneerDDJSB.init = function(id)
{
	var ALPHA = 1.0 / 8;
	var BETA = ALPHA / 32;
	var JOG_RESOLUTION = 720;
	var VINYL_SPEED = 33 + 1/3;
	var NUMBER_OF_ACTIVE_PERFORMANCE_PADS = 4;
	var SAFE_SCRATCH_TIMEOUT = 20; // 20ms is the minimum allowed here.
	
	PioneerDDJSB.settings = 
	{
		alpha: ALPHA,
		beta: BETA,
		jogResolution: JOG_RESOLUTION,
		vinylSpeed: VINYL_SPEED,
		numberOfActivePerformancePads: NUMBER_OF_ACTIVE_PERFORMANCE_PADS,
		safeScratchTimeout: SAFE_SCRATCH_TIMEOUT
	};
		
		
	PioneerDDJSB.channels = 
	{	
		0x00: {},
		0x01: {}
	};
		
	PioneerDDJSB.channelGroups =
	{
		'[Channel1]': 0x00,
		'[Channel2]': 0x01
	};
		
	PioneerDDJSB.samplerGroups =
	{
		'[Sampler1]': 0x00,
		'[Sampler2]': 0x01,
		'[Sampler3]': 0x02,
		'[Sampler4]': 0x03
	};
		
	PioneerDDJSB.fxGroups =
	{
		'[EffectRack1_EffectUnit1]': 0x00,
		'[EffectRack1_EffectUnit2]': 0x01
	};
		
	PioneerDDJSB.fxControls =
	{
		'group_[Channel1]_enable': 0x00,
		'group_[Channel2]_enable': 0x01,
		'group_[Headphone]_enable': 0x02
	};

	PioneerDDJSB.fxButtonPressed =
	[
	[false, false, false],
	[false, false, false]
	]

	PioneerDDJSB.ledGroups =
	{
		hotCue: 	0x00,
		autoLoop: 	0x10,
		manualLoop: 0x20,
		sampler: 	0x30
	}
	
	PioneerDDJSB.loopIntervals =
	{
		PAD1: 1,
		PAD2: 2,
		PAD3: 4,
		PAD4: 8,
		PAD5: 16,
		PAD6: 32,
		PAD7: 64
	};
	
	PioneerDDJSB.setFXSoftTakeover(true);
				
	PioneerDDJSB.BindControlConnections(false);
};

PioneerDDJSB.shutdown = function()
{
	PioneerDDJSB.BindControlConnections(true);
};

PioneerDDJSB.setFXSoftTakeover = function(activate)
{
	// Set softTakeover for controls effected by the effects knobs
	for(var deck = 0; deck < 2; deck++) {
		engine.softTakeover('[EffectRack1_EffectUnit' + (deck+1) + ']', 'mix', activate);
		for(var i = 0; i < 3; i++) {
			engine.softTakeover('[EffectRack1_EffectUnit' + (deck+1) + '_Effect1]', 'parameter'+(i+1)+'', activate);
		}
	}
}

PioneerDDJSB.BindControlConnections = function(isUnbinding)
{
	for (var samplerIndex = 1; samplerIndex <= 4; samplerIndex++)
	{
		var samplerGroup = '[Sampler' + samplerIndex + ']';

		engine.connectControl(samplerGroup, 'duration', 'PioneerDDJSB.SamplerLeds', isUnbinding);

	}

	for (var fxParamIndex = 1; fxParamIndex <= 2; fxParamIndex++)
	{
		var effectUnitGroup = '[EffectRack1_EffectUnit' + fxParamIndex + ']';
		
		// Headphone LEDs
		engine.connectControl(effectUnitGroup, 'group_[Headphone]_enable', 'PioneerDDJSB.FXLeds', isUnbinding);
		engine.connectControl(effectUnitGroup, 'group_[Channel1]_enable', 'PioneerDDJSB.FXLeds', isUnbinding);
		engine.connectControl(effectUnitGroup, 'group_[Channel2]_enable', 'PioneerDDJSB.FXLeds', isUnbinding);
	}

	for (var channelIndex = 1; channelIndex <= 2; channelIndex++)
	{
		var channelGroup = '[Channel' + channelIndex + ']';
		
		// Play / Pause LED
		engine.connectControl(channelGroup, 'play', 'PioneerDDJSB.PlayLeds', isUnbinding);
		
		// Cue LED
		engine.connectControl(channelGroup, 'cue_default', 'PioneerDDJSB.CueLeds', isUnbinding);
		
		// PFL / Headphone Cue LED
		engine.connectControl(channelGroup, 'pfl', 'PioneerDDJSB.HeadphoneCueLed', isUnbinding);
		
		// Keylock LED
		engine.connectControl(channelGroup, 'keylock', 'PioneerDDJSB.KeyLockLeds', isUnbinding);
		
		// Vinyl LED
		engine.connectControl(channelGroup, 'slip_enabled', 'PioneerDDJSB.ToggleVinylLed', isUnbinding);

		// Quantize (shifted SYNC) LED
		engine.connectControl(channelGroup, 'quantize', 'PioneerDDJSB.ToggleQuantizeLed', isUnbinding);

		// Beat (SYNC) LED
		engine.connectControl(channelGroup, 'beat_active', 'PioneerDDJSB.ToggleBeatLed', isUnbinding);

		// Loop in LED
		engine.connectControl(channelGroup, 'loop_in', 'PioneerDDJSB.ToggleLoopInLed', isUnbinding);

		// Loop out LED
		engine.connectControl(channelGroup, 'loop_out', 'PioneerDDJSB.ToggleLoopOutLed', isUnbinding);

		// Kill low LED
		engine.connectControl(channelGroup, 'filterLowKill', 'PioneerDDJSB.ToggleLowKillLed', isUnbinding);

		// Kill mid LED
		engine.connectControl(channelGroup, 'filterMidKill', 'PioneerDDJSB.ToggleMidKillLed', isUnbinding);

		// Kill high LED
		engine.connectControl(channelGroup, 'filterHighKill', 'PioneerDDJSB.ToggleHighKillLed', isUnbinding);

		// Mute LED
		engine.connectControl(channelGroup, 'mute', 'PioneerDDJSB.ToggleMuteLed', isUnbinding);

		// Loop enter/exit LED
		engine.connectControl(channelGroup, 'loop_enabled', 'PioneerDDJSB.ToggleLoopExitLed', isUnbinding);		
		
		// Hook up the hot cue performance pads
		for (var i = 0; i < 8; i++)
		{
			engine.connectControl(channelGroup, 'hotcue_' + (i + 1) +'_enabled', 'PioneerDDJSB.HotCuePerformancePadLed', isUnbinding);
		}
		
		// Hook up the roll performance pads
		for (var i = 1; i <= 7; i++)
		{
			engine.connectControl(channelGroup, 'beatloop_' + PioneerDDJSB.loopIntervals['PAD' + i] + '_enabled', 'PioneerDDJSB.RollPerformancePadLed', isUnbinding);
		}
	}
};

///////////////////////////////////////////////////////////////
//                         LED SECTION                       //
///////////////////////////////////////////////////////////////

// Returns the control code for a led on the pads.
// groupShift is true when the group has been selected while pressing shift
PioneerDDJSB.PadLedTranslator = function(ledGroup, ledNumber, shift, groupShift)
{
	return (groupShift ? 0x40 : 0x00) + (shift ? 0x08 : 0x00) + ledGroup + ledNumber;
}

// This handles LEDs related to the PFL / Headphone Cue event.
PioneerDDJSB.FXLeds = function(value, group, control) 
{
	var fxGroup = PioneerDDJSB.fxGroups[group];
	var fxControl = PioneerDDJSB.fxControls[control]

	midi.sendShortMsg(0x94 + fxGroup, 0x47 + fxControl, value ? 0x7F : 0x00);
	midi.sendShortMsg(0x94 + fxGroup, 0x63 + fxControl, value ? 0x7F : 0x00);
};

// This handles LEDs related to the PFL / Headphone Cue event.
PioneerDDJSB.HeadphoneCueLed = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	midi.sendShortMsg(0x90 + channel, 0x54, value ? 0x7F : 0x00); // Headphone Cue LED
	midi.sendShortMsg(0x90 + channel, 0x68, value ? 0x7F : 0x00); // Headphone Cue LED shifted
};

// This handles LEDs related to the cue_default event.
PioneerDDJSB.CueLeds = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];	
	midi.sendShortMsg(0x90 + channel, 0x0C, value ? 0x7F : 0x00); // Cue LED
};

// This handles LEDs related to the keylock event.
PioneerDDJSB.KeyLockLeds = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];	
	midi.sendShortMsg(0x90 + channel, 0x1A, value ? 0x7F : 0x00); // Keylock LED
};

// This handles LEDs related to the play event.
PioneerDDJSB.PlayLeds = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	midi.sendShortMsg(0x90 + channel, 0x0B, value ? 0x7F : 0x00); // Play / Pause LED
	midi.sendShortMsg(0x90 + channel, 0x0C, value ? 0x7F : 0x00); // Cue LED
	midi.sendShortMsg(0x90 + channel, 0x47, value ? 0x7F : 0x00); // Play / Pause LED when shifted
	midi.sendShortMsg(0x90 + channel, 0x48, value ? 0x7F : 0x00); // Cue LED when shifted
};

PioneerDDJSB.ToggleVinylLed = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	midi.sendShortMsg(0x90 + channel, 0x17, value ? 0x7F : 0x00); // Vinyl LED
};

PioneerDDJSB.ToggleQuantizeLed = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	midi.sendShortMsg(0x90 + channel, 0x5C, value ? 0x7F : 0x00); // Shifted sync led
};

PioneerDDJSB.ToggleBeatLed = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	midi.sendShortMsg(0x90 + channel, 0x58, value ? 0x7F : 0x00); // Shifted sync led
};

PioneerDDJSB.ToggleLoopInLed = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	midi.sendShortMsg(0x97 + channel,
		PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 0, false, false),
		value ? 0x7F : 0x00);
};

PioneerDDJSB.ToggleLoopOutLed = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	midi.sendShortMsg(0x97 + channel,
		PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 1, false, false),
		value ? 0x7F : 0x00);
};

PioneerDDJSB.ToggleLoopExitLed = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	midi.sendShortMsg(0x97 + channel,
		PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 2, false, false),
		value ? 0x7F : 0x00);
};

PioneerDDJSB.ToggleLowKillLed = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	midi.sendShortMsg(0x97 + channel,
		PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 0, false, true),
		value ? 0x7F : 0x00);
};

PioneerDDJSB.ToggleMidKillLed = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	midi.sendShortMsg(0x97 + channel,
		PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 1, false, true),
		value ? 0x7F : 0x00);
};

PioneerDDJSB.ToggleHighKillLed = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	midi.sendShortMsg(0x97 + channel,
		PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 2, false, true),
		value ? 0x7F : 0x00);
};

PioneerDDJSB.ToggleMuteLed = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	midi.sendShortMsg(0x97 + channel,
		PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.manualLoop, 3, false, true),
		value ? 0x7F : 0x00);
};

PioneerDDJSB.SamplerLeds = function(value, group, control) 
{
	var sampler = PioneerDDJSB.samplerGroups[group];
	for(var channel = 0; channel < 2; channel++) {
		midi.sendShortMsg(0x97 + channel,
			PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.sampler, sampler, false, false),
			value ? 0x7F : 0x00);
		midi.sendShortMsg(0x97 + channel,
			PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.sampler, sampler, false, true),
			value ? 0x7F : 0x00);
		midi.sendShortMsg(0x97 + channel,
			PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.sampler, sampler, true, false),
			value ? 0x7F : 0x00);
		midi.sendShortMsg(0x97 + channel,
			PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.sampler, sampler, true, true),
			value ? 0x7F : 0x00);
	}
};


// Lights up the LEDs for beat-loops. Currently there are 4 pads set.
// If you want extra pads or want to change the speed, you need to adjust
// the xml and the enumaration in the config/init setings
PioneerDDJSB.RollPerformancePadLed = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	for (var i = 1; i <= 7; i++)
	{
		if (control === 'beatloop_' + PioneerDDJSB.loopIntervals['PAD' + i] + '_enabled')
		{
			if (i < 5) {
				midi.sendShortMsg(0x97 + channel,
					PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.autoLoop, i-1, false, false),
					value ? 0x7F : 0x00);
			}
			else {
				midi.sendShortMsg(0x97 + channel,
					PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.autoLoop, i-5, true, false),
					value ? 0x7F : 0x00);
			}
		}
	}
};
// Lights up the LEDs for the hotcue buttons.
PioneerDDJSB.HotCuePerformancePadLed = function(value, group, control) 
{
	var channel = PioneerDDJSB.channelGroups[group];
	
	var padIndex = null;
	for (var i = 0; i < 8; i++)
	{
		if (control == 'hotcue_' + i + '_enabled')
		{
			break;
		}
		
		padIndex = i;
	}

	var shiftedGroup = false;
	if (padIndex >= 4) {
		shiftedGroup = true;
		padIndex %= 4;
	}
	
	// Pad LED without shift key
	midi.sendShortMsg(0x97 + channel,
		PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.hotCue, padIndex, false, shiftedGroup),
		value ? 0x7F : 0x00);
	
	// Pad LED with shift key
	midi.sendShortMsg(0x97 + channel,
		PioneerDDJSB.PadLedTranslator(PioneerDDJSB.ledGroups.hotCue, padIndex, true, shiftedGroup),
		value ? 0x7F : 0x00);
	//midi.sendShortMsg(0x97 + channel, baseControl + padIndex + 0x08, value ? 0x7F : 0x00);
};

///////////////////////////////////////////////////////////////
//                     JOGWHEEL SECTION                      //
///////////////////////////////////////////////////////////////

// Work out the jog-wheel change / delta
PioneerDDJSB.getJogWheelDelta = function(value)
{
	// The Wheel control centers on 0x40; find out how much it's moved by.
	return value - 0x40;
};

// Toggle scratching for a channel
PioneerDDJSB.toggleScratch = function(channel, isEnabled)
{
	var deck = channel + 1; 
	if (isEnabled) 
	{
        engine.scratchEnable(
			deck, 
			PioneerDDJSB.settings['jogResolution'], 
			PioneerDDJSB.settings['vinylSpeed'], 
			PioneerDDJSB.settings['alpha'], 
			PioneerDDJSB.settings['beta']);
    }
    else 
	{
        engine.scratchDisable(deck);
    }
};

// Detect when the user touches and releases the jog-wheel while 
// jog-mode is set to vinyl to enable and disable scratching.
PioneerDDJSB.jogScratchTouch = function(channel, control, value, status, group) 
{
	/*var deck = channel + 1; 
	
	if (!engine.getValue(group, 'play'))
	{
		PioneerDDJSB.toggleScratch(channel, value == 0x7F);
	}
	else
	{
		var activate = value > 0;
		
		if (activate) 
		{
			engine.brake(deck, true, 1, 1); // enable brake effect
			PioneerDDJSB.toggleScratch(channel, true);
		}
		else 
		{
			engine.brake(deck, false, 1, 1); // disable brake effect
			PioneerDDJSB.toggleScratch(channel, false);
		}  
	}*/
};
 
// Scratch or seek with the jog-wheel.
PioneerDDJSB.jogScratchTurn = function(channel, control, value, status) 
{
	PioneerDDJSB.jogSeekTurn(channel, control, value, status);
	//var deck = channel + 1; 

    // Only scratch if we're in scratching mode, when 
	// user is touching the top of the jog-wheel.
    //if (engine.isScratching(deck)) 
	//{
	//	engine.scratchTick(deck, PioneerDDJSB.getJogWheelDelta(value));
	//}
};

// Pitch bend using the jog-wheel, or finish a scratch when the wheel 
// is still turning after having released it.
PioneerDDJSB.jogPitchBend = function(channel, control, value, status, group) 
{
	//if (!engine.getValue(group, 'play'))
		//return;
		
	var deck = channel + 1; 
	PioneerDDJSB.pitchBend(channel, PioneerDDJSB.getJogWheelDelta(value));
};

// Pitch bend a channel
PioneerDDJSB.pitchBend = function(channel, movement) 
{
	var deck = channel + 1; 
	var group = '[Channel' + deck +']';
	
	// Make this a little less sensitive.
	movement = movement / 5; 
	
	// Limit movement to the range of -3 to 3.
	movement = movement > 3 ? 3 : movement;
	movement = movement < -3 ? -3 : movement;
	
	engine.setValue(group, 'jog', movement);
};

// Called when the jog-mode is not set to vinyl, and the jog wheel is touched.
// If we are not playing we want to seek through it and this is done in scratch mode
PioneerDDJSB.jogSeekTouch = function(channel, control, value, status, group) 
{
	// if (engine.getValue(group, 'play'))
	// 	return;
		
	// var deck = channel + 1; 
	// PioneerDDJSB.toggleScratch(channel, value == 0x7F);
};

// Call when the jog-wheel is turned. The related jogSeekTouch function 
// sets up whether we will be scratching or pitch-bending depending 
// on whether a song is playing or not.
PioneerDDJSB.jogSeekTurn = function(channel, control, value, status, group) 
{
	//PioneerDDJSB.toggleScratch(channel, value == 0x7F);
	//if (engine.getValue(group, 'play'))
	//	return;
	
	var deck = channel + 1; 
	engine.setValue('[Channel' + deck + ']', 'jog', PioneerDDJSB.getJogWheelDelta(value)/3)
};

PioneerDDJSB.jogFastSeekTurn = function(channel, control, value, status, group) 
{
	//PioneerDDJSB.toggleScratch(channel, value == 0x7F);
	//if (engine.getValue(group, 'play'))
	//	return;
	
	var deck = channel + 1; 
	engine.setValue('[Channel' + deck + ']', 'jog', PioneerDDJSB.getJogWheelDelta(value)*20)
};


///////////////////////////////////////////////////////////////
//                    ROTARY SELECTOR SECTION                //
///////////////////////////////////////////////////////////////
// Handles the rotary selector for choosing tracks, library items, crates, etc.
PioneerDDJSB.CalculateRotaryDelta = function(value)
{
	var delta = 0x40 - Math.abs(0x40 - value);
	var isCounterClockwise = value > 0x40;
	if (isCounterClockwise)
	{
		delta *= -1;
	}
	return delta;
};

PioneerDDJSB.RotarySelector = function(channel, control, value, status) 
{
	delta = PioneerDDJSB.CalculateRotaryDelta(value);
	engine.setValue('[Playlist]', 'SelectTrackKnob', delta);
};

PioneerDDJSB.ShiftedRotarySelector = function(channel, control, value, status) 
{
	delta = PioneerDDJSB.CalculateRotaryDelta(value);
	if (delta > 0)
	{
		control = 'SelectNextPlaylist';
	}
	else if (delta < 0)
	{
		control = 'SelectPrevPlaylist';
	}

	engine.setValue('[Playlist]', control, Math.abs(delta));
}

PioneerDDJSB.RotarySelectorClick = function(channel, control, value, status) 
{
};

PioneerDDJSB.RotarySelectorShiftedClick = function(channel, control, value, status) 
{
	if (value == 0x7F)
	{
		engine.setValue('[Playlist]', 'ToggleSelectedSidebarItem', 1);
	}
};

PioneerDDJSB.FXButton = function(channel, control, value, status)
{
	var deck = channel - 4;
	var button = control - 0x47;
	PioneerDDJSB.fxButtonPressed[deck][button] = (value == 0x7F);

	if (button < 2)
	{
		engine.trigger('[EffectRack1_EffectUnit'+(deck+1)+']', 'group_[Channel'+(button+1)+']_enable');
		
	}
	else
	{
		engine.trigger('[EffectRack1_EffectUnit'+(deck+1)+']', 'group_[Headphone]_enable');
	}
};

// TODO use group parameter
PioneerDDJSB.FXKnob = function(channel, control, value, status)
{
	var deck = channel - 4;
	var anyButtonPressed = false;
	for (var i = 0; i < 3; i++) 
	{
		if (PioneerDDJSB.fxButtonPressed[deck][i])
			anyButtonPressed = true;
	}

	if (!anyButtonPressed)
	{
		engine.setValue('[EffectRack1_EffectUnit' + (deck+1) + ']', 'mix', value/0x7F);
	}
	else
	{
		for (var i = 0; i < 3; i++)
		{
			if (PioneerDDJSB.fxButtonPressed[deck][i])
			{
				engine.setParameter('[EffectRack1_EffectUnit' + (deck+1) + '_Effect1]', 'parameter'+(i+1)+'', value/0x7F);
			}
		}
	}
};
