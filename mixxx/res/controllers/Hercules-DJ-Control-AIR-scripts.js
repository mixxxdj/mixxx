function HerculesAir () {}

HerculesAir.beatStepDeckA1 = 0
HerculesAir.beatStepDeckA2 = 68
HerculesAir.beatStepDeckB1 = 0
HerculesAir.beatStepDeckB2 = 76

HerculesAir.scratchEnable_alpha = 1.0/8
HerculesAir.scratchEnable_beta = (1.0/8)/32
HerculesAir.scratchEnable_intervalsPerRev = 128
HerculesAir.scratchEnable_rpm = 33+1/3

HerculesAir.shiftButtonPressed = false

HerculesAir.wheel_multiplier = 0.9

HerculesAir.init = function(id) {
	HerculesAir.resetLEDs()

	midi.sendShortMsg(0x90, 59, 0x7f) // headset volume "-" button LED (always on)
	midi.sendShortMsg(0x90, 60, 0x7f) // headset volume "+" button LED (always on)

	if(engine.getValue("[Master]", "headMix") > 0.5) {
		midi.sendShortMsg(0x90, 57, 0x7f) // headset "Mix" button LED
	} else {
		midi.sendShortMsg(0x90, 58, 0x7f) // headset "Cue" button LED
	}

	engine.connectControl("[Channel1]", "beat_active", "HerculesAir.beatProgressDeckA")
	engine.connectControl("[Channel1]", "play", "HerculesAir.playDeckA")
	
	engine.connectControl("[Channel2]", "beat_active", "HerculesAir.beatProgressDeckB")
	engine.connectControl("[Channel2]", "play", "HerculesAir.playDeckB")
}

HerculesAir.shutdown = function() {
	HerculesAir.resetLEDs()
}

/* -------------------------------------------------------------------------- */

HerculesAir.playDeckA = function() {
	if(engine.getValue("[Channel1]", "play") == 0) {
		midi.sendShortMsg(0x90, HerculesAir.beatStepDeckA1, 0x00)
		HerculesAir.beatStepDeckA1 = 0
		HerculesAir.beatStepDeckA2 = 68
	}
}

HerculesAir.playDeckB = function() {
	if(engine.getValue("[Channel2]", "play") == 0) {
		midi.sendShortMsg(0x90, HerculesAir.beatStepDeckB1, 0x00)
		HerculesAir.beatStepDeckB1 = 0
		HerculesAir.beatStepDeckB2 = 76
	}
}

HerculesAir.beatProgressDeckA = function() {
	if(engine.getValue("[Channel1]", "beat_active") == 1) {
		if(HerculesAir.beatStepDeckA1 != 0) {
			midi.sendShortMsg(0x90, HerculesAir.beatStepDeckA1, 0x00)
		}
		
		HerculesAir.beatStepDeckA1 = HerculesAir.beatStepDeckA2
		
		midi.sendShortMsg(0x90, HerculesAir.beatStepDeckA2, 0x7f)
		if(HerculesAir.beatStepDeckA2 < 71) {
			HerculesAir.beatStepDeckA2++
		} else {
			HerculesAir.beatStepDeckA2 = 68
		}
	}
}

HerculesAir.beatProgressDeckB = function() {
	if(engine.getValue("[Channel2]", "beat_active") == 1) {
		if(HerculesAir.beatStepDeckB1 != 0) {
			midi.sendShortMsg(0x90, HerculesAir.beatStepDeckB1, 0x00)
		}
		
		HerculesAir.beatStepDeckB1 = HerculesAir.beatStepDeckB2
		
		midi.sendShortMsg(0x90, HerculesAir.beatStepDeckB2, 0x7f)
		if(HerculesAir.beatStepDeckB2 < 79) {
			HerculesAir.beatStepDeckB2++
		} else {
			HerculesAir.beatStepDeckB2 = 76
		}
	}
}

HerculesAir.headCue = function(midino, control, value, status, group) {
	if(engine.getValue(group, "headMix") == 1) {
		engine.setValue(group, "headMix", -1.0);
		midi.sendShortMsg(0x90, 57, 0x00);
		midi.sendShortMsg(0x90, 58, 0x7f);
	}
};

HerculesAir.headMix = function(midino, control, value, status, group) {
	if(engine.getValue(group, "headMix") != 1) {
		engine.setValue(group, "headMix", 1.0);
		midi.sendShortMsg(0x90, 57, 0x7f);
		midi.sendShortMsg(0x90, 58, 0x00);		
	}
};

HerculesAir.jog = function(midino, control, value, status, group) {
    engine.setValue(
    	group,
    	'jog',
    	(value == 0x01 ? 1 : -1) * HerculesAir.wheel_multiplier
	)
}

HerculesAir.resetLEDs = function() {
	for(var i=1; i<79; i++) {
		midi.sendShortMsg(0x90, i, 0x00)
	}
}

HerculesAir.sampler = function(midino, control, value, status, group) {
	if(value != 0x00) {
		if(HerculesAir.shiftButtonPressed) {
			engine.setValue(group, "LoadSelectedTrack", 1)
		} else if(engine.getValue(group, "play") == 0) {
			engine.setValue(group, "start_play", 1)
		} else {
			engine.setValue(group, "play", 0)
		}
	}
}

HerculesAir.scratch = function(midino, control, value, status, group) {
	if(engine.getValue(group, "play") == 0) {
		var new_position = engine.getValue(group,"playposition") + 0.008 * (value == 0x01 ? 1 : -1)
		if(new_position<0) new_position = 0
		if(new_position>1) new_position = 1
		engine.setValue(group,"playposition",new_position);	
	} else {
    	engine.scratchTick(group == "[Channel1]" ? 1 : 2, value == 0x01 ? 1 : -1)
    }
}

HerculesAir.scratch_enable = function(midino, control, value, status, group) {
    if(value == 0x7f) {
        engine.scratchEnable(
        	group == "[Channel1]" ? 1 : 2,
        	HerculesAir.scratchEnable_intervalsPerRev,
        	HerculesAir.scratchEnable_rpm,
        	HerculesAir.scratchEnable_alpha,
        	HerculesAir.scratchEnable_beta
    	)
    } else {
        engine.scratchDisable(
        	group == "[Channel1]" ? 1 : 2
    	)
    }
}

HerculesAir.shift = function(midino, control, value, status, group) {
	HerculesAir.shiftButtonPressed = (value == 0x7f)
}