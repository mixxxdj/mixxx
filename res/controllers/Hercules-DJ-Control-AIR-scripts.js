function HerculesAir () {}

HerculesAir.beatStepDeckA1 = 0
HerculesAir.beatStepDeckA2 = 0x44
HerculesAir.beatStepDeckB1 = 0
HerculesAir.beatStepDeckB2 = 0x4C

HerculesAir.scratchEnable_alpha = 1.0/8
HerculesAir.scratchEnable_beta = (1.0/8)/32
HerculesAir.scratchEnable_intervalsPerRev = 128
HerculesAir.scratchEnable_rpm = 33+1/3

HerculesAir.shiftButtonPressed = false
HerculesAir.enableSpinBack = false

HerculesAir.wheel_multiplier = 0.4

HerculesAir.init = function(id) {
    HerculesAir.id = id;

    // extinguish all LEDs
    for (var i = 79; i<79; i++) {
        midi.sendShortMsg(0x90, i, 0x00);
    }
    midi.sendShortMsg(0x90, 0x3B, 0x7f) // headset volume "-" button LED (always on)
    midi.sendShortMsg(0x90, 0x3C, 0x7f) // headset volume "+" button LED (always on)

    if(engine.getValue("[Master]", "headMix") > 0.5) {
        midi.sendShortMsg(0x90, 0x39, 0x7f) // headset "Mix" button LED
    } else {
        midi.sendShortMsg(0x90, 0x3A, 0x7f) // headset "Cue" button LED
    }

    // Set soft-takeover for all Sampler volumes
    for (var i=engine.getValue("[Master]","num_samplers"); i>=1; i--) {
        engine.softTakeover("[Sampler"+i+"]","pregain",true);
    }
    // Set soft-takeover for all applicable Deck controls
    for (var i=engine.getValue("[Master]","num_decks"); i>=1; i--) {
        engine.softTakeover("[Channel"+i+"]","volume",true);
        engine.softTakeover("[Channel"+i+"]","filterHigh",true);
        engine.softTakeover("[Channel"+i+"]","filterMid",true);
        engine.softTakeover("[Channel"+i+"]","filterLow",true);
    }

    engine.softTakeover("[Master]","crossfader",true);

    engine.connectControl("[Channel1]", "beat_active", "HerculesAir.beatProgressDeckA")
    engine.connectControl("[Channel1]", "play", "HerculesAir.playDeckA")

    engine.connectControl("[Channel2]", "beat_active", "HerculesAir.beatProgressDeckB")
    engine.connectControl("[Channel2]", "play", "HerculesAir.playDeckB")
    
    print ("Hercules DJ Control AIR: "+id+" initialized.");
}

HerculesAir.shutdown = function() {
    HerculesAir.resetLEDs()
}

/* -------------------------------------------------------------------------- */

HerculesAir.playDeckA = function() {
    if(engine.getValue("[Channel1]", "play") == 0) {
        // midi.sendShortMsg(0x90, HerculesAir.beatStepDeckA1, 0x00)
        HerculesAir.beatStepDeckA1 = 0x00
        HerculesAir.beatStepDeckA2 = 0x44
    }
}

HerculesAir.playDeckB = function() {
    if(engine.getValue("[Channel2]", "play") == 0) {
        // midi.sendShortMsg(0x90, HerculesAir.beatStepDeckB1, 0x00)
        HerculesAir.beatStepDeckB1 = 0x00
        HerculesAir.beatStepDeckB2 = 0x4C
    }
}

HerculesAir.beatProgressDeckA = function() {
    if(engine.getValue("[Channel1]", "beat_active") == 1) {
        if(HerculesAir.beatStepDeckA1 != 0x00) {
            midi.sendShortMsg(0x90, HerculesAir.beatStepDeckA1, 0x00)
        }

        HerculesAir.beatStepDeckA1 = HerculesAir.beatStepDeckA2

        midi.sendShortMsg(0x90, HerculesAir.beatStepDeckA2, 0x7f)
        if(HerculesAir.beatStepDeckA2 < 0x47) {
            HerculesAir.beatStepDeckA2++
        } else {
            HerculesAir.beatStepDeckA2 = 0x44
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
        if(HerculesAir.beatStepDeckB2 < 0x4F) {
            HerculesAir.beatStepDeckB2++
        } else {
            HerculesAir.beatStepDeckB2 = 0x4C
        }
    }
}

HerculesAir.headCue = function(midino, control, value, status, group) {
    if(engine.getValue(group, "headMix") == 0) {
        engine.setValue(group, "headMix", -1.0);
        midi.sendShortMsg(0x90, 0x39, 0x00);
        midi.sendShortMsg(0x90, 0x3A, 0x7f);
    }
};

HerculesAir.headMix = function(midino, control, value, status, group) {
    if(engine.getValue(group, "headMix") != 1) {
        engine.setValue(group, "headMix", 0);
        midi.sendShortMsg(0x90, 0x39, 0x7f);
        midi.sendShortMsg(0x90, 0x3A, 0x00);
    }
};

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

HerculesAir.wheelTurn = function(midino, control, value, status, group) {

    var deck = script.deckFromGroup(group);
    var newValue=(value==0x01 ? 1: -1);
    // See if we're scratching. If not, do wheel jog.
    if (!engine.isScratching(deck)) {
        engine.setValue(group, "jog", newValue* HerculesAir.wheel_multiplier);
        return;
    }

    if (engine.getValue(group, "play") == 0) {
        var new_position = engine.getValue(group,"playposition") + 0.008 * (value == 0x01 ? 1 : -1)
        if(new_position<0) new_position = 0
        if(new_position>1) new_position = 1
        engine.setValue(group,"playposition",new_position);
    } else {
        // Register the movement
        engine.scratchTick(deck,newValue);
    }

}

HerculesAir.jog = function(midino, control, value, status, group) {
    if (HerculesAir.enableSpinBack) {
        HerculesAir.wheelTurn(midino, control, value, status, group);
    } else {
        var deck = script.deckFromGroup(group);
        var newValue = (value==0x01 ? 1:-1);
        engine.setValue(group, "jog", newValue* HerculesAir.wheel_multiplier);
    }
}

HerculesAir.scratch_enable = function(midino, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    if(value == 0x7f) {
        engine.scratchEnable(
            deck,
            HerculesAir.scratchEnable_intervalsPerRev,
            HerculesAir.scratchEnable_rpm,
            HerculesAir.scratchEnable_alpha,
            HerculesAir.scratchEnable_beta
        );
    } else {
        engine.scratchDisable(deck);
    }
}

HerculesAir.shift = function(midino, control, value, status, group) {
    HerculesAir.shiftButtonPressed = (value == 0x7f);
    midi.sendShortMsg(status, control, value);
}


HerculesAir.spinback= function(midino, control, value, status,group) {
    if (value==0x7f) {
        HerculesAir.enableSpinBack = !HerculesAir.enableSpinBack;
        if (HerculesAir.enableSpinBack) {
            midi.sendShortMsg(status,control, 0x7f);
        } else {
            midi.sendShortMsg(status,control, 0x0);
        }
    }
}
