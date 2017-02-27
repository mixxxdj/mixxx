function MaudioXponent () {}

// ----------   Global variables    ----------
MaudioXponent.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time
MaudioXponent.on = 0x90;
MaudioXponent.off = 0x80;
MaudioXponent.maxDecks = 0x04;
MaudioXponent.leftDeck;
MaudioXponent.rightDeck;
MaudioXponent.bankA = 0x82;
MaudioXponent.bankB = 0x92;
MaudioXponent.Handshake1 = [0xF0,0x7E,0x7F,0x06,0x01,0xF7];
MaudioXponent.Handshake2 = [0xF0,0x00,0x20,0x08,0x00,0x00,0x63,0x0E,0x16,0x40,0x00,0x01,0xF7];
MaudioXponent.Handshake3 = [0xF0,0x00,0x20,0x08,0x00,0x00,0x63,0x0E,0x16,0x40,0x00,0x00,0xF7];

MaudioXponent.config = {
    nudgeButtonMode : 1,    // 0 = Normal, 1 = Reversed
    pflMode : 0,            // 0 = Independent, 1 = Toggle
    syncFlashMode : 0,      // 0 = Off, 1 = Pulse, 2 = Toggle
    vuMeterMode : 1,        // 0 = Master, 1 = Channel
}

MaudioXponent.decks = 
[
    { noteOffset : 0, on : 0x90, off : 0x80, isLeft : true, isRight : false, isBankA : true, isBankB : false },
    { noteOffset : 1, on : 0x91, off : 0x81, isLeft : false, isRight : true, isBankA : true, isBankB : false },
    { noteOffset : 5, on : 0x95, off : 0x85, isLeft : true, isRight : false, isBankA : false, isBankB : true },
    { noteOffset : 6, on : 0x96, off : 0x86, isLeft : false, isRight : true, isBankA : false, isBankB : true }
];

MaudioXponent.state = {
    bank : 0,               // Which position is the bank switch currently set to?
    faderPosition : 0,      // Temporary storage for cross-fader position during punch-ins.
    focusedEffect : 0,
    plnumberpos : 0, 
    plnumberneg : 0,
};

MaudioXponent.leds = {
    "play": 0x24,
    "cue": 0x23,
    "back": 0x21,
    "fwd": 0x22,
    "loopIn": 0x29,
    "loopOut": 0x2B,
    "loop": 0x2A,
    "loop1": 0x25,
    "loop2": 0x26,
    "loop4": 0x27,
    "loop8": 0x28,
    "leftkey": 0x1C,
    "rightkey": 0x1D,
    "key": 0x1E,
    "pluskey": 0x1F,
    "minkey": 0x20,
    "cue1": 0x17,
    "cue2": 0x18,
    "cue3": 0x19,
    "cue4": 0x1A,
    "cue5": 0x1B,
    "fx1": 0x0C,
    "fx2": 0x0D,
    "fx3": 0x0E,
    "fx4": 0x0F,
    "rate_temp_down": MaudioXponent.config.nudgeButtonMode ? 0x11 : 0x10,
    "rate_temp_up": MaudioXponent.config.nudgeButtonMode ? 0x10 : 0x11,
    "bigx": 0x12,
    "reverse": 0x13,
    "pfl": 0x14,
    "scratch": 0x15,
    "punchIn": 0x07,
    "sync": 0x02,
    "button_parameter1": 0x08,
    "button_parameter2": 0x09,
    "button_parameter3": 0x0A,
    "gain": 0x0B,
    "shift": 0x2C
};

MaudioXponent.binleds = {
    8: "button_parameter1",
    9: "button_parameter2",
    10: "button_parameter3",
    16: (MaudioXponent.config.nudgeButtonMode ? "rate_temp_up" : "rate_temp_down"),
    17: (MaudioXponent.config.nudgeButtonMode ? "rate_temp_down" : "rate_temp_up"),
    18: "keylock",
    19: "reverse",
    20: "pfl",
    33: "back",
    34: "fwd",
    36: "play"
};

// ----------   Functions    ----------
MaudioXponent.logParams = function(functionName, a, b, c, d, e, f) {
    print("***");
    print("*** " + functionName + ": a="+ a + ", b=" + b + ", c=" + c + ", d=" + d + ", e=" + e + ", f=" + f);
    print("***");
};

MaudioXponent.init = function (id) {
    MaudioXponent.initDecks();
    MaudioXponent.initLights();
    MaudioXponent.syncLights();

    //MaudioXponent.probeLights();
};

MaudioXponent.initDecks = function() {
    // Because vuMeter values change rapidly, we're pre-digesting the parameters for this function
    engine.connectControl("[Master]", "VuMeterL", function(value) { MaudioXponent.vuMeter(0, 0, 0, value); });
    engine.connectControl("[Master]", "VuMeterR", function(value) { MaudioXponent.vuMeter(0, 0, 1, value); });
    engine.connectControl("[Channel1]", "VuMeterL", function(value) { MaudioXponent.vuMeter(1, MaudioXponent.bankA, 0, value); });
    engine.connectControl("[Channel2]", "VuMeterR", function(value) { MaudioXponent.vuMeter(1, MaudioXponent.bankA, 1, value); });
    engine.connectControl("[Channel3]", "VuMeterL", function(value) { MaudioXponent.vuMeter(1, MaudioXponent.bankB, 0, value); });
    engine.connectControl("[Channel4]", "VuMeterR", function(value) { MaudioXponent.vuMeter(1, MaudioXponent.bankB, 1, value); });

    for (channel = 1; channel <= MaudioXponent.maxDecks; channel++) {
        var group = "[Channel" + (channel) + "]";
        var deck = MaudioXponent.decks[channel - 1];
        deck.id = channel;
        deck.beatState = false;
        deck.group = group;
        deck.progressMeterStatusByte = deck.isBankA ? 0xB3 : 0xB8;
        deck.progressMeterSecondByte = deck.isLeft ? 0x14 : 0x15;
        deck.vuMeterStatusByte = deck.isBankA ? 0xB3 : 0xB8;
        deck.vuMeterSecondByte = deck.isLeft ? 0x12 : 0x13;
        deck.scratchEnabled = false;
        deck.scratching = false;
        deck.shift = false;
        deck.warnAt = 0;
        deck.filterLow = 1;
        deck.filterMid = 1;
        deck.filterHigh = 1;
        deck.pregain = 1;
        
        engine.connectControl(group, "play", "MaudioXponent.onPlay");
        engine.connectControl(group, "playposition", "MaudioXponent.onPlayPositionChange");
        engine.connectControl(group, "duration", "MaudioXponent.onTrackLoaded");
        engine.connectControl(group, "beat_active", "MaudioXponent.onBeatActive");
        engine.connectControl(group, "eject", "MaudioXponent.onEject");
        
        for (i = 1; i <= 5; i++) {
            engine.connectControl(group, "hotcue_" + i + "_enabled", "MaudioXponent.onHotCue");
        }

        engine.connectControl(group, "loop_enabled", "MaudioXponent.onLoopExit");
        engine.connectControl(group, "loop_in", "MaudioXponent.onLoopIn");
        engine.connectControl(group, "loop_out", "MaudioXponent.onLoopOut");
        for (i = 0.125; i < 16; i *= 2) {
            engine.connectControl(group, "beatloop_" + i + "_enabled", "MaudioXponent.onBeatLoop");
        }

        engine.connectControl(group, "keylock", "MaudioXponent.onkeyLock");
        engine.connectControl(group, "bpm", "MaudioXponent.onBpmChanged");
        engine.connectControl(group, "pfl", "MaudioXponent.onPflChanged");

        engine.connectControl(group, "reverse", "MaudioXponent.onReverse");
        engine.connectControl(group, "reverseroll", "MaudioXponent.onReverse");
        engine.connectControl(group, "rate_temp_down", "MaudioXponent.onNudge");
        engine.connectControl(group, "rate_temp_up", "MaudioXponent.onNudge"); 

        engine.connectControl("[EqualizerRack1_" + group + "_Effect1]", "button_parameter1", "MaudioXponent.onFilterKill");
        engine.connectControl("[EqualizerRack1_" + group + "_Effect1]", "button_parameter2", "MaudioXponent.onFilterKill");
        engine.connectControl("[EqualizerRack1_" + group + "_Effect1]", "button_parameter3", "MaudioXponent.onFilterKill");

        engine.softTakeover(group, "rate", true);
        engine.softTakeover(group, "volume", true);

        // Soft-takeovers that aren't yet working correctly (Might need to save/restore on bank change)
        engine.softTakeover(group, "filterLow", true);
        engine.softTakeover(group, "filterMid", true);
        engine.softTakeover(group, "filterHigh", true);
    }

    for (i = 1; i <=4; i++) {
        var group = "[Sampler" + i + "]";
        engine.connectControl(group, "play", "MaudioXponent.onSampler");
    }

    // Effects parameters... not working in Mixxx 2.0, should work in 2.1
    for(i = 1; i <= 4; i++) {
        engine.softTakeover("[EffectRack1_EffectUnit" + i + "_Effect1]", "parameter1", true);
        engine.softTakeover("[EffectRack1_EffectUnit" + i + "_Effect1]", "parameter2", true);
        engine.softTakeover("[EffectRack1_EffectUnit" + i + "_Effect1]", "parameter3", true);
        engine.softTakeover("[EffectRack1_EffectUnit" + i + "]", "mix", true);
    }

    // TODO: It would be nice to discover some way to force the Xponent to dump its current status.
    // Until then, I just have to assume that you start on Bank A
    MaudioXponent.bankSwitch(0, 0, 0, MaudioXponent.bankA, 0);    
};

MaudioXponent.probeLights = function() {
    var status = 0xB4;
    // for(status = 0xB0; status < 0xC0; status++) {
        for (var led in MaudioXponent.leds) {
            var byte2 = MaudioXponent.leds[led];
        // for(byte2 = 0; byte2 < 128; byte2++) {
            for(byte3 = 0; byte3 < 128; byte3++) {
                midi.sendShortMsg(status, byte2, byte3);
                MaudioXponent.pauseScript(5);
            }
        }
    // }
};

MaudioXponent.initLights = function () {
	// Enable lights
	midi.sendSysexMsg(MaudioXponent.Handshake1, MaudioXponent.Handshake1.length);
	midi.sendSysexMsg(MaudioXponent.Handshake2, MaudioXponent.Handshake2.length);

    for (var led in MaudioXponent.leds) {
        midi.sendShortMsg(MaudioXponent.leftDeck.on, MaudioXponent.leds[led], 0x01);
        midi.sendShortMsg(MaudioXponent.rightDeck.on, MaudioXponent.leds[led], 0x01);
        MaudioXponent.pauseScript(15);
    }
    
    for(i = 0; i <= 10; i += 1) {
        var value = MaudioXponent.convert(i * .1);
        // VU meters
        midi.sendShortMsg(MaudioXponent.leftDeck.vuMeterStatusByte, MaudioXponent.leftDeck.vuMeterSecondByte, value);
        midi.sendShortMsg(MaudioXponent.rightDeck.vuMeterStatusByte, MaudioXponent.rightDeck.vuMeterSecondByte, value);
        
        // Progress meters
        midi.sendShortMsg(MaudioXponent.leftDeck.progressMeterStatusByte, MaudioXponent.leftDeck.progressMeterSecondByte, value);
        midi.sendShortMsg(MaudioXponent.rightDeck.progressMeterStatusByte, MaudioXponent.rightDeck.progressMeterSecondByte, value);

        MaudioXponent.pauseScript(15);
    }

    MaudioXponent.pauseScript(500);

    for (var led in MaudioXponent.leds) {
        midi.sendShortMsg(MaudioXponent.on, MaudioXponent.leds[led], 0x00);
        midi.sendShortMsg(MaudioXponent.on + 1, MaudioXponent.leds[led], 0x00);
        MaudioXponent.pauseScript(15);
    }

    // VU meters
    midi.sendShortMsg(MaudioXponent.leftDeck.vuMeterStatusByte, MaudioXponent.leftDeck.vuMeterSecondByte, 0);
    midi.sendShortMsg(MaudioXponent.rightDeck.vuMeterStatusByte, MaudioXponent.rightDeck.vuMeterSecondByte, 0);
    
    // Progress meters
    midi.sendShortMsg(MaudioXponent.leftDeck.progressMeterStatusByte, MaudioXponent.leftDeck.progressMeterSecondByte, 0);
    midi.sendShortMsg(MaudioXponent.rightDeck.progressMeterStatusByte, MaudioXponent.rightDeck.progressMeterSecondByte, 0);
};

MaudioXponent.shutdown = function (id) {
    for (var led in MaudioXponent.leds) {
        midi.sendShortMsg(MaudioXponent.on, MaudioXponent.leds[led], 0x00);
        midi.sendShortMsg(MaudioXponent.on + 1,MaudioXponent.leds[led], 0x00);
    }

    // VU meters
    midi.sendShortMsg(MaudioXponent.leftDeck.vuMeterStatusByte, MaudioXponent.leftDeck.vuMeterSecondByte, 0);
    midi.sendShortMsg(MaudioXponent.rightDeck.vuMeterStatusByte, MaudioXponent.rightDeck.vuMeterSecondByte, 0);
    
    // Progress meters
    midi.sendShortMsg(MaudioXponent.leftDeck.progressMeterStatusByte, MaudioXponent.leftDeck.progressMeterSecondByte, 0);
    midi.sendShortMsg(MaudioXponent.rightDeck.progressMeterStatusByte, MaudioXponent.rightDeck.progressMeterSecondByte, 0);

    // Secret Handshake
	midi.sendSysexMsg(MaudioXponent.Handshake3, MaudioXponent.Handshake3.length);
};

MaudioXponent.syncLights = function() {
    for (i = 0; i < MaudioXponent.decks.length; i++) {
        var deck = MaudioXponent.decks[i];

        engine.trigger(deck.group, "play");
        engine.trigger(deck.group, "pfl");
        engine.trigger(deck.group, "loop_enabled");
        engine.trigger(deck.group, "loop_start_position");
        engine.trigger(deck.group, "loop_end_position");
        
        engine.trigger("[EqualizerRack1_" + deck.group + "_Effect1]", "button_parameter1");
        engine.trigger("[EqualizerRack1_" + deck.group + "_Effect1]", "button_parameter2");
        engine.trigger("[EqualizerRack1_" + deck.group + "_Effect1]", "button_parameter3");

        engine.trigger(deck.group, "keylock");
        engine.trigger(deck.group, "hotcue_1_enabled");
        engine.trigger(deck.group, "hotcue_2_enabled");
        engine.trigger(deck.group, "hotcue_3_enabled");
        engine.trigger(deck.group, "hotcue_4_enabled");
        engine.trigger(deck.group, "hotcue_5_enabled");

        engine.trigger(deck.group, "loop_in");
        engine.trigger(deck.group, "loop_out");

        midi.sendShortMsg(deck.on, MaudioXponent.leds.scratch, deck.scratchEnabled);
        MaudioXponent.onPlayPositionChange(deck.playPosition, deck.group);
    }
};

MaudioXponent.convert = function(value) {
    return (value * 127).toFixed(0);
};

MaudioXponent.getDeck = function(group) {
    return MaudioXponent.decks[parseInt(group.substring(8)) - 1];
};

MaudioXponent.pauseScript = function(ms) {
    startDate = new Date();
    currentDate = null;
    while(currentDate-startDate < ms) currentDate = new Date();
};

MaudioXponent.vuMeter = function(mode, bank, side, value) {
    if (mode === MaudioXponent.config.vuMeterMode) { // Accept messages for the active mode
        if ((mode === 0) || (bank === MaudioXponent.state.bank)) { // Accept messages for the active bank
            value = MaudioXponent.convert(value);

            if (side === 0) {
                midi.sendShortMsg(MaudioXponent.leftDeck.vuMeterStatusByte, MaudioXponent.leftDeck.vuMeterSecondByte, value);        
            } else {
                midi.sendShortMsg(MaudioXponent.rightDeck.vuMeterStatusByte, MaudioXponent.rightDeck.vuMeterSecondByte, value);        
            }
        }
    }
}

MaudioXponent.bankSwitch = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group);
    MaudioXponent.state.bank = status;

    if (status === MaudioXponent.bankA) {
        MaudioXponent.leftDeck = MaudioXponent.decks[0];
        MaudioXponent.rightDeck = MaudioXponent.decks[1];
    } else if (status === MaudioXponent.bankB) {
        MaudioXponent.leftDeck = MaudioXponent.decks[2];
        MaudioXponent.rightDeck = MaudioXponent.decks[3];
    }

    MaudioXponent.syncLights();
};

MaudioXponent.beatsync = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.getDeck(group);
    var activate = (status == deck.on);

    if (activate) {
        if (MaudioXponent.leftDeck.shift && MaudioXponent.rightDeck.shift) {
            // Double-shift = cycle modes
            var mode = MaudioXponent.config.syncFlashMode + 1;
            if(mode===3){
                mode = 0;
            }
            MaudioXponent.config.syncFlashMode = mode;
        } else {
            if (!deck.shift) {
                engine.setValue(deck.group, "beatsync", 0x04);
            } else {
                engine.setValue(deck.group, "bpm_tap", 0x01);
            }
        }
    }

    midi.sendShortMsg(deck.on, MaudioXponent.leds.sync, activate);
};

MaudioXponent.punchIn = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group)
    var deck = MaudioXponent.getDeck(group);
    var activate = (status == deck.on);

    if (activate) {
        MaudioXponent.state.faderPosition = faderPosition = engine.getValue("[Master]", "crossfader");

        if ((faderPosition >= 0.90 && deck.isLeft) || (faderPosition <= -0.90 && deck.isRight)) {
            engine.setValue("[Master]", "crossfader", 0);
        }
    } else {
        engine.setValue("[Master]", "crossfader", MaudioXponent.state.faderPosition);
    }

    midi.sendShortMsg(deck.on, MaudioXponent.leds.punchIn, activate);
};

MaudioXponent.filterKill = function(channel, control, value, status, group) {
    // script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.getDeck(group);
    var activate = (status == deck.on);

    if (control === MaudioXponent.leds.gain) {
        // Gain Buttons
        if (activate && (MaudioXponent.leftDeck.shift && MaudioXponent.rightDeck.shift)) {
            //Double-shift = cycle modes
            var mode = MaudioXponent.config.vuMeterMode + 1;
            if (mode === 2) {
                mode = 0;
            }
            MaudioXponent.config.vuMeterMode = mode;
        } else {
            if (activate) {
                // Save the current value
                deck.volume = engine.getValue(deck.group, "volume");
                engine.setValue(deck.group, "volume", 0x00);
            } else {
                // Restore the saved value from above
                engine.setValue(deck.group, "volume", deck.volume);
            }

            midi.sendShortMsg(deck.on, MaudioXponent.leds.gain, activate);
        }
    } else {
        // Low/Mid/High Buttons, momentary kill
        var group = "[EqualizerRack1_" + deck.group + "_Effect1]";
        engine.setValue(group, MaudioXponent.binleds[control], activate);
    }
};

MaudioXponent.onFilterKill = function(value, group, control) {
    var deck = MaudioXponent.decks[parseInt(group.substring(24)) - 1];
    midi.sendShortMsg(deck.on, MaudioXponent.leds[control], value);
};

MaudioXponent.effectButton = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.getDeck(group);
    var currentControl = control - 0x0B;
    var group = "[EffectRack1_EffectUnit" + currentControl + "]";

    if (deck.shift) {
        // Cycle effects
        engine.setValue(group, "next_chain", 1);
    } else {
        if (currentControl == MaudioXponent.state.focusedEffect) {
            // Toggle enabled
            var effectEnabled = engine.getValue(group, "enabled");
            if (effectEnabled) {
                engine.setValue(group, "enabled", 0);
            } else {
                engine.setValue(group, "enabled", 1);
            }        
        } else {
            // Change focus
            MaudioXponent.state.focusedEffect = currentControl;
        }

        // Light the focused effect
        for (i = 0; i < 4; i++) {
            var ctrl = MaudioXponent.leds.fx1 + i;
            var fxNum = i + 1;
            var newValue = (fxNum == MaudioXponent.state.focusedEffect) ? 1 : 0;
            midi.sendShortMsg(deck.on, ctrl, newValue);
        }
    }
};

MaudioXponent.effectParameter = function(channel, control, value, status, group) {
    // script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.getDeck(group);
    var currentControl = control - 0x0B;

    if (MaudioXponent.state.focusedEffect != 0) {
        var group = "[EffectRack1_EffectUnit" + MaudioXponent.state.focusedEffect + "_Effect1]";
        var scaledValue = value / 0x7F;

        if (currentControl == 4) {
            // Wet / Dry
            engine.setParameter("[EffectRack1_EffectUnit" + MaudioXponent.state.focusedEffect + "]", "mix", scaledValue);
        } else {
            // Other parameter
            engine.setParameter(group, "parameter" + currentControl, scaledValue);
        }
    }
};

MaudioXponent.wheel = function (channel, control, value, status, group) {
    var deck = MaudioXponent.getDeck(group);
    
    if (deck.shift) {
        if (value > 64) {
            MaudioXponent.state["plnumberpos"]++;
            if (MaudioXponent.state["plnumberpos"] % 12 == 0) {
                engine.setValue("[Playlist]", "SelectTrackKnob", 1);
            }
        } else if (value < 64) {
            MaudioXponent.state["plnumberneg"]++;
            if (MaudioXponent.state["plnumberneg"] % 12 == 0) {
                engine.setValue("[Playlist]", "SelectTrackKnob", -1);
            }  
        }
    } else {
        if (deck.scratching) {
    	    engine.scratchTick(deck.id, value - 64);
        } else {
            engine.setValue(group, "jog", (value - 64) / 8);
        }
    }
};

MaudioXponent.wheelTouch = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group)
    var deck = MaudioXponent.getDeck(group);
    var activate = (status == deck.on);
    
    if (activate) {
        if (deck.scratchEnabled) {
            engine.scratchEnable(deck.id, 3 * 128, 33 + 1/3, 1.0/8, (1.0/8)/32);
            deck.scratching = true;
        }
    } else {
        engine.scratchDisable (deck.id);
        deck.scratching = false;
    }    
};

MaudioXponent.onBeatActive = function(value, group) {
    var deck = MaudioXponent.getDeck(group);

    if (MaudioXponent.config.syncFlashMode === 1) {
        midi.sendShortMsg(deck.on, MaudioXponent.leds.sync, value);
    }
    
    if (value) {
        deck.beatState = !deck.beatState;
        
        if (MaudioXponent.config.syncFlashMode === 2) {
            midi.sendShortMsg(deck.on, MaudioXponent.leds.sync, deck.beatState);
        }
    }
};

MaudioXponent.onBeatLoop = function(value, group, control) {
    var deck = MaudioXponent.getDeck(group);
    var offset = Math.log(parseInt(control.substring(9))) / Math.log(2)
    midi.sendShortMsg(deck.on, MaudioXponent.leds.loop1, value);
};

MaudioXponent.onBpmChanged = function(value, group) {
    //print ("BPM Change, " + group + ", value=" + value);
};

MaudioXponent.onPlayPositionChange = function(value, group) {
    var deck = MaudioXponent.getDeck(group);
    var status = MaudioXponent.bank
    
    value = engine.getValue(group, "playposition");
    deck.playPosition = value;

    if ((value < deck.warnAt) || (!engine.getValue(group, "play")) || (value >= deck.warnAt && deck.beatState)) {
        midi.sendShortMsg(deck.progressMeterStatusByte, deck.progressMeterSecondByte, MaudioXponent.convert(value));
    } else {
        midi.sendShortMsg(deck.progressMeterStatusByte, deck.progressMeterSecondByte, 0x00);        
    }
};

MaudioXponent.hotcue = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group)
    var deck = MaudioXponent.getDeck(group);
    var cueNumber = control - 0x16;

    if (!deck.shift) {
        var activate = (status == deck.on);
        engine.setValue(group, "hotcue_" + cueNumber + "_activate", activate);
    } else {
        engine.setValue(group, "hotcue_" + cueNumber + "_clear", 1);
    }
};

MaudioXponent.onHotCue = function(value, group, control) {
    var deck = MaudioXponent.getDeck(group);
    var cueNumber = parseInt(control.substring(7)) - 1;
    midi.sendShortMsg(deck.on, MaudioXponent.leds.cue1 + cueNumber, value);
};

MaudioXponent.loopin = function(channel, control, value, status, group) {
    var deck = MaudioXponent.getDeck(group);
    engine.setValue(deck.group, "loop_in", !deck.shift);
};

MaudioXponent.onLoopIn = function(value, group, control) {
    var deck = MaudioXponent.getDeck(group);
    midi.sendShortMsg(deck.on, MaudioXponent.leds.loopIn, value);
};

MaudioXponent.loopout = function(channel, control, value, status, group) {
    var deck = MaudioXponent.getDeck(group);
    engine.setValue(deck.group, "loop_out", !deck.shift);
};

MaudioXponent.onLoopOut = function(value, group, control) {
    var deck = MaudioXponent.getDeck(group);
    midi.sendShortMsg(deck.on, MaudioXponent.leds.loopOut, value);
};

MaudioXponent.loopexit = function(channel, control, value, status, group) {
    var deck = MaudioXponent.getDeck(group);
    engine.setValue(deck.group, "reloop_exit", 1);
};

MaudioXponent.onLoopExit = function(value, group, control) {
    var deck = MaudioXponent.getDeck(group);
    midi.sendShortMsg(deck.on, MaudioXponent.leds.loop, engine.getValue(group, control) == 1);
}

MaudioXponent.pitch = function(channel, control, value, status, group) {
    engine.setValue(group, "rate", script.midiPitch(control, value, status));
};

MaudioXponent.shift = function(channel, control, value, status, group) {
    // script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.getDeck(group);
    deck.shift = (status === deck.on);
    midi.sendShortMsg(deck.on, MaudioXponent.leds.shift, deck.shift);
};

MaudioXponent.toggleScratchMode = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.getDeck(group);    
    deck.scratchEnabled = !deck.scratchEnabled;
    midi.sendShortMsg(deck.on, MaudioXponent.leds.scratch, deck.scratchEnabled);
};

MaudioXponent.playlist = function(channel, control, value, status, group) {
    var deck = MaudioXponent.getDeck(group);
    switch (control) {
    case 28:
        engine.setValue("[Playlist]", "SelectPrevTrack", 1);
        midi.sendShortMsg(deck.on, control, true);
	break;
    case 29:
        engine.setValue("[Playlist]", "SelectNextTrack", 1);
        midi.sendShortMsg(deck.on, control, true);
	break;
    case 30:
        var activenow = engine.getValue(deck.group, "play");
        if (activenow == 1) {    // If currently active
            engine.setValue("[Playlist]", "LoadSelectedIntoFirstStopped", 1);
        } else {
            engine.setValue(deck.group, "LoadSelectedTrack", 1);
        }
        midi.sendShortMsg(deck.on, control, true);
	break;
    case 31:
        engine.setValue("[Playlist]", "SelectPrevPlaylist", 1);
        midi.sendShortMsg(deck.on, control, true);
	break;
    case 32:
        engine.setValue("[Playlist]", "SelectNextPlaylist", 1);
        midi.sendShortMsg(deck.on, control, true);
	break;
    }
};

MaudioXponent.playlistoff = function(channel, control, value, status, group) {
    var deck = MaudioXponent.getDeck(group);
    midi.sendShortMsg(deck.on, control, false);
};

MaudioXponent.keyLock = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.getDeck(group);
 
    if (!deck.shift) {
        // Unshifted = Keylock
        var currentValue = engine.getParameter(deck.group, "keylock");
        if (currentValue == 0){
            engine.setValue(deck.group,"keylock",0x01);
        } else {
            engine.setValue(deck.group,"keylock", 0x00);
        }
    }else{        
        // Shifted = Quantize
        var currentValue = engine.getParameter(deck.group, "quantize");
        if (currentValue == 0){
            engine.setValue(deck.group,"quantize",0x01);
        } else {
            engine.setValue(deck.group,"quantize", 0x00);
        }
    }    
};

MaudioXponent.onkeyLock = function(value, group) {
    var deck = MaudioXponent.getDeck(group);
    midi.sendShortMsg(deck.on, MaudioXponent.leds.key, value);
};

MaudioXponent.brake = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.getDeck(group);
    var activate = (status == deck.on);
    engine.brake(deck.id, activate);
};

MaudioXponent.cue = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.getDeck(group);
    var activate = (status == deck.on);
    engine.setValue(deck.group, "cue_default", activate);
    
    //TODO: Is this needed?
    midi.sendShortMsg(deck.on, control, activate);
};

MaudioXponent.play = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.getDeck(group);
    var activate = (status == deck.on);
    if (activate) {
        var playing = engine.getValue(group, "play");
        engine.setValue(deck.group, "play", !playing);
    }
};

MaudioXponent.onPlay = function(value, group, control) {
    var deck = MaudioXponent.getDeck(group);
    midi.sendShortMsg(deck.on, MaudioXponent.leds[control], value);
};

MaudioXponent.beatgridAdjust = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.getDeck(group);

    if (deck.shift) {
        // Shifted = align
        engine.setValue(deck.group, "beats_translate_curpos", 1);
    } else {
        // Unshifted = earlier / later
        if (control == MaudioXponent.leds.leftkey){
            engine.setValue(deck.group, "beats_translate_earlier", 1);
        } else {
            engine.setValue(deck.group, "beats_translate_later", 1);
        }
    }
};

MaudioXponent.beatgridLoop = function(channel, control, value, status, group) {
    // script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.getDeck(group);
    var activate = (status == deck.on);
    var length = Math.pow(2, control - 0x25);

    if (!deck.shift) {
        if (activate) {
            engine.setValue(group, "beatloop_" + length + "_toggle", value);
        }
    } else {
        if (activate) {
            engine.setValue(group, "beatlooproll_" + 1.0 / length + "_activate", value);
        } else {
            MaudioXponent.loopexit(channel, control, value, status, group);
        }
    }
};

MaudioXponent.onTrackLoaded = function(duration, group) {
    var deck = MaudioXponent.getDeck(group);
    deck.warnAt = (duration - 30) / parseFloat(duration);
};

MaudioXponent.nudge = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.getDeck(group);
    var activate = (status === deck.on);
    engine.setValue(group, MaudioXponent.binleds[control], activate);
};

MaudioXponent.onNudge = function(value, group, control) {
    var deck = MaudioXponent.getDeck(group);
    midi.sendShortMsg(deck.on, MaudioXponent.leds[control], value);
};

MaudioXponent.pfl = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.getDeck(group);
    var activate = (status == deck.on);

    if (activate) {
        if (MaudioXponent.leftDeck.shift && MaudioXponent.rightDeck.shift) {
            // Double-shift = cycle modes
            var mode = MaudioXponent.config.pflMode + 1;
            if(mode === 2){
                mode = 0;
            }
            MaudioXponent.config.pflMode = mode;
        } else {
            for (i = 1; i <= MaudioXponent.decks.length; i++) {
                var grp = "[Channel" + i + "]";
                if (grp === group) {
                    engine.setValue(grp, "pfl", !engine.getValue(grp, "pfl"));
                } else if (MaudioXponent.config.pflMode == 1) {
                    engine.setValue(grp, "pfl", 0);
                }
            }
        }
    }
};

MaudioXponent.onPflChanged = function(value, group) {
    var deck = MaudioXponent.getDeck(group);
    midi.sendShortMsg(deck.on, MaudioXponent.leds.pfl, value);
};

MaudioXponent.reverse = function(channel, control, value, status, group) {
    // script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.getDeck(group);
    var activate = (status == deck.on) ? 1 : 0;
    if (deck.shift) {
        // Reverse rolling
        engine.setValue(group, "reverseroll", activate);
    } else {
        // Normal
        engine.setValue(group, "reverse", activate);
    }
};

MaudioXponent.onReverse = function(value, group, control) {
    var deck = MaudioXponent.getDeck(group);
    midi.sendShortMsg(deck.on, MaudioXponent.leds[control], value);
};

MaudioXponent.seek = function(channel, control, value, status, group) {
    var deck = MaudioXponent.getDeck(group);
    var activate = (status == deck.on);
    engine.setValue(group, MaudioXponent.binleds[control], activate);
};

MaudioXponent.sampler = function(channel, control, value, status, group) {
    // script.midiDebug(channel, control, value, status, group);
    var shifted = MaudioXponent.decks[0].shift || MaudioXponent.decks[2].shift;
    if (shifted) {
        engine.setValue(group, "cue_gotoandstop", 1);
    } else {
        engine.setValue(group, "cue_gotoandplay", 1);
    }
};

MaudioXponent.onSampler = function(value, group, control) {
    var samplerNumber = parseInt(group.substring(8));
    var led = MaudioXponent.leds.fx1 + samplerNumber - 1;
    midi.sendShortMsg(MaudioXponent.on, led, value);
    midi.sendShortMsg(MaudioXponent.on + 5, led, value);
};

MaudioXponent.onEject = function(value, group, control) {
    if (value) {
        engine.setValue(group, "playposition", 0);
        MaudioXponent.onPlayPositionChange(null, group);
    }
};