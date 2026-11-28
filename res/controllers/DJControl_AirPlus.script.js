
//
// ****************************************************************************
// * Mixxx mapping script file for the Hercules DJControl Air+.
// * Author: DJ Phatso, contributions by Kerrick Staley
// * Version 1.1 (March 2019)
// * Forum: https://www.mixxx.org/forums/viewtopic.php?f=7&t=9403
// * Wiki: http://www.mixxx.org/wiki/


// Changes to v1.1
// - Controller knob/slider values are queried on startup, so MIXXX is synced.
// - Fixed vinyl button behavior the first time it's pressed.
//
// v1.0 : Original release

// TODO: Functions that could be implemented to the script:
//	
//	- Beats LED
//	  
//
// ****************************************************************************
function AirPlus() {}
var AirPlus = {};

AirPlus.scratchButtonState = true;


AirPlus.init = function() {

	
	//Update all LED states
	midi.sendShortMsg(0xB0, 0x7F, 0x00);
	midi.sendShortMsg(0xB1, 0x7F, 0x00);
	
    // Vinyl button LED On.
    midi.sendShortMsg(0x90, 0x39, 0x7F);
	
   	
	// Headphone CUE/MIX state
	if(engine.getValue("[Master]", "headMix") > 0.5) {
		midi.sendShortMsg(0x90, 0x38, 0x7f) // headset "Mix" button LED
	} else {
		midi.sendShortMsg(0x90, 0x37, 0x7f) // headset "Cue" button LED
	}
	//Enable Soft takeover
	engine.softTakeover("[Master]","crossfader",true);
	engine.softTakeover("[QuickEffectRack1_[Channel1]]","super1",true);
	engine.softTakeover("[QuickEffectRack1_[Channel2]]","super1",true);
	
	//Set effects Levels - Dry/Wet - Filters
	engine.setParameter("[EffectRack1_EffectUnit1_Effect1]", "meta", 0.6);
	engine.setParameter("[EffectRack1_EffectUnit1_Effect2]", "meta", 0.6);
	engine.setParameter("[EffectRack1_EffectUnit1_Effect3]", "meta", 0.6);
	engine.setParameter("[EffectRack1_EffectUnit2_Effect1]", "meta", 0.6);
	engine.setParameter("[EffectRack1_EffectUnit2_Effect2]", "meta", 0.6);
	engine.setParameter("[EffectRack1_EffectUnit2_Effect3]", "meta", 0.6);
	engine.setParameter("[EffectRack1_EffectUnit1]", "mix", 0.6);
	engine.setParameter("[EffectRack1_EffectUnit2]", "mix", 0.6);
	engine.setParameter("[QuickEffectRack1_[Channel1]]","super1", 0.5);
	engine.setParameter("[QuickEffectRack1_[Channel2]]","super1", 0.5);
	
	// Ask the controller to send all current knob/slider values over MIDI, which will update
    // the corresponding GUI controls in MIXXX.
    midi.sendShortMsg(0xB0, 0x7F, 0x7F);
};



// Headphone CUE/MIX buttons status
AirPlus.headCue = function(midino, control, value, status, group) {
	if(engine.getValue(group, "headMix") == 0) {
		engine.setValue(group, "headMix", -1.0);
		midi.sendShortMsg(0x90, 0x37, 0x7f);
		midi.sendShortMsg(0x90, 0x38, 0x00);
	}
};

AirPlus.headMix = function(midino, control, value, status, group) {
	if(engine.getValue(group, "headMix") != 1) {
		engine.setValue(group, "headMix", 0);
		midi.sendShortMsg(0x90, 0x37, 0x00);
		midi.sendShortMsg(0x90, 0x38, 0x7f);
	}
};


// The Vinyl button, used to enable or disable scratching on the jog wheels.
AirPlus.vinylButton = function(channel, control, value, status, group) {
    if (value) {
        if (AirPlus.scratchButtonState) {
           	AirPlus.scratchButtonState = false;
            midi.sendShortMsg(0x90, 0x39, 0x00);

        } else {
            AirPlus.scratchButtonState = true;
			AirPlus.scratchButtonState = true;
            midi.sendShortMsg(0x90, 0x39, 0x7F);
        }
    }
};


// The pressure action over the jog wheel

AirPlus.wheelTouchDA = function(channel, control, value, status, group) {
    channel = channel + 1;
    if (value > 0 && (engine.getValue("[Channel1]", "play") != 1 || AirPlus.scratchButtonState)) {
        //  Touching the wheel.
        var alpha = 1.0 / 8;
        var beta = alpha / 16;
        engine.scratchEnable(1, 400, 33 + 1 / 3, alpha, beta);
    } else {
        // Released the wheel.
        engine.scratchDisable(1);
    }
};

AirPlus.wheelTouchDB = function(channel, control, value, status, group) {
    channel = channel + 2;
    if (value > 0 && (engine.getValue("[Channel2]", "play") != 1 || AirPlus.scratchButtonState)) {
        // Touching the wheel.
        var alpha = 1.0 / 8;
        var beta = alpha / 32;
        engine.scratchEnable(2, 400, 33 + 1 / 3, alpha, beta);
    } else {
        // Released the wheel.
        engine.scratchDisable(2);
    }
};


// Using the top of wheel for scratching (Vinyl button On) and bending (Vinyl button Off)
AirPlus.scratchWheelDA = function(channel, control, value, status, group) {

    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }

    if (engine.isScratching(1)) {
        engine.scratchTick(1, newValue); // Scratch!
    } else {
        engine.setValue('[Channel' + 1 + ']', 'jog', newValue); // Pitch bend
    }
}

AirPlus.scratchWheelDB = function(channel, control, value, status, group) {

    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }

    // In either case, register the movement
    if (engine.isScratching(2)) {
        engine.scratchTick(2, newValue); // Scratch!
    } else {
        engine.setValue('[Channel' + 2 + ']', 'jog', newValue); // Pitch bend
    }
}

// Using the side of wheel for the bending
AirPlus.bendWheelDA = function(channel, control, value, status, group) {

    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }

    {
        engine.setValue('[Channel' + 1 + ']', 'jog', newValue); // Pitch bend
    }
}


AirPlus.bendWheelDB = function(channel, control, value, status, group) {

    // A: For a control that centers on 0:
    var newValue;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    } {
        engine.setValue('[Channel' + 2 + ']', 'jog', newValue); // Pitch bend
    }
}



AirPlus.shutdown = function() {

for (var i = 1; i <= 40; i++) {
        midi.sendShortMsg(0x90, i, 0x00);
    }
};