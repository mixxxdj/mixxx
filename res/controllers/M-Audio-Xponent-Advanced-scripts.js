function MaudioXponent () {}

// ----------   Global variables    ----------
MaudioXponent.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time

MaudioXponent.config = {
    nudgeButtonMode : 0,    // 0 = Normal, 1 = Reversed
    pflMode : 1,            // 0 = Independent, 1 = Toggle
    syncFlashMode : 0,      // 0 = Off, 1 = Simple, 2 = Toggle
    vuMeterMode : 0,        // 0 = Per-Channel mode, 1 = Master Mode
}

MaudioXponent.state = { 
    deck : [],              // Stores everything about an individual deck
    faderPosition : 0,      // Temporary storage for cross-fader position during punch-ins.
    focusedEffect : 0,
    numDecks : 0,
    plnumberpos : 0, 
    plnumberneg : 0,
};

MaudioXponent.leds = {
    "play": 0x24,
    "cue": 0x23,
    "back": 0x21,
    "fwd": 0x22,
    "in": 0x29,
    "out": 0x2B,
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
    "leftp": 0x10,
    "rightp": 0x11,
    "bigx": 0x12,
    "big-": 0x13,
    "head": 0x14,
    "scratch": 0x15,
    "fadarbutton": 0x07,
    "sync": 0x02,
    "low": 0x08,
    "middle": 0x09,
    "hight": 0x0A,
    "gain": 0x0B,
    "shift": 0x2C
};

MaudioXponent.binleds = {
    8:  "filterLowKill",
    9:  "filterMidKill",
    10: "filterHighKill",
    18: "keylock",
    19: "reverse",
    20: "pfl",
    33: "back",
    34: "fwd",
    36: "play"
};

MaudioXponent.Handshake1 = [0xF0,0x7E,0x7F,0x06,0x01,0xF7];
MaudioXponent.Handshake2 = [0xF0,0x00,0x20,0x08,0x00,0x00,0x63,0x0E,0x16,0x40,0x00,0x01,0xF7];
MaudioXponent.Handshake3 = [0xF0,0x00,0x20,0x08,0x00,0x00,0x63,0x0E,0x16,0x40,0x00,0x00,0xF7];

// ----------   Functions    ----------
MaudioXponent.logParams = function(a, b, c, d, e, f) {
    print("a="+ a + ", b=" + b + ", c=" + c + ", d=" + d + ", e=" + e + ", f=" + f);
};

MaudioXponent.init = function (id) {
    MaudioXponent.state.numDecks = engine.getValue("[Master]", "num_decks");

    MaudioXponent.initDecks();
    MaudioXponent.initLights();
    MaudioXponent.syncLights();
    MaudioXponent.enableSoftTakeover();
};

MaudioXponent.initDecks = function() {
    if (MaudioXponent.config.vuMeterMode == 1) {
        engine.connectControl("[Master]", "VuMeterL", function(value) { MaudioXponent.volumeLEDs(0, value); });
        engine.connectControl("[Master]", "VuMeterR", function(value) { MaudioXponent.volumeLEDs(1, value); });
    } else {
        engine.connectControl("[Channel1]", "VuMeter", function(value) { MaudioXponent.volumeLEDs(0, value); });
        engine.connectControl("[Channel2]", "VuMeter", function(value) { MaudioXponent.volumeLEDs(1, value); });
    }

    for (channel = 0; channel < MaudioXponent.state.numDecks; channel++) {
        var group = "[Channel" + (channel + 1) + "]";
        var deck = MaudioXponent.state.deck[channel] = 
        {
            id : channel + 1,
            beatState : false,
            group : group,
            off : 0x80 + channel + (channel >= 3 ? 5 : 0),
            on : 0x90 + channel,
            shift : false,
            warnAt : 0,
            scratchEnabled : false,
            scratching : false,
        }

        engine.connectControl(group, "playposition", "MaudioXponent.onPlayPositionChange");
        engine.connectControl(group, "duration", "MaudioXponent.onTrackLoaded");
        engine.connectControl(group, "beat_active", "MaudioXponent.onBeatActive");
        
        for (i = 1; i <= 5; i++) {
            engine.connectControl(group, "hotcue_" + i + "_enabled", "MaudioXponent.onHotCue");
        }

        engine.connectControl(group, "loop_enabled", "MaudioXponent.onLoopExit");
        engine.connectControl(group, "loop_start_position", "MaudioXponent.onLoopIn");
        engine.connectControl(group, "loop_end_position", "MaudioXponent.onLoopOut");
        for (i = 0.125; i < 16; i *= 2) {
            engine.connectControl(group, "beatloop_" + i + "_enabled", "MaudioXponent.onBeatLoop");
        }

        engine.connectControl(group, "keylock", "MaudioXponent.onKeyLock");
        engine.connectControl(group, "bpm", "MaudioXponent.onBpmChanged");

        engine.softTakeover(group, "rate", true);
    }

    // Effects parameters... not working in Mixxx 2.0, should work in 2.1
    for(i = 1; i < 5; i++) {
        engine.softTakeover("[EffectRack1_EffectUnit" + i + "_Effect1]", "parameter1", true);
        engine.softTakeover("[EffectRack1_EffectUnit" + i + "_Effect1]", "parameter2", true);
        engine.softTakeover("[EffectRack1_EffectUnit" + i + "_Effect1]", "parameter3", true);
        engine.softTakeover("[EffectRack1_EffectUnit" + i + "]", "mix", true);
    }       
};

MaudioXponent.initLights = function () {
	// Enable lights
	midi.sendSysexMsg(MaudioXponent.Handshake1, MaudioXponent.Handshake1.length);
	midi.sendSysexMsg(MaudioXponent.Handshake2, MaudioXponent.Handshake2.length);

    // This code light on all leds and then light off
    midi.sendShortMsg(0xB3, 0x14, 0x00);
    midi.sendShortMsg(0xB3, 0x15, 0x00);

    for (var led in MaudioXponent.leds) {
        midi.sendShortMsg(MaudioXponent.on, MaudioXponent.leds[led], 0x01);
        midi.sendShortMsg(MaudioXponent.on + 1, MaudioXponent.leds[led], 0x01);
        MaudioXponent.pauseScript(15);
    }
    
    for(i = 0; i <= 10; i+=1){
        var value = MaudioXponent.convert(i * .1);
        // VU meters
        midi.sendShortMsg(0xB3, 0x12, value);
        midi.sendShortMsg(0xB3, 0x13, value);
        
        // Progress meters
        midi.sendShortMsg(0xB3, 0x14, value);
        midi.sendShortMsg(0xB3, 0x15, value);
        MaudioXponent.pauseScript(15);
    }

    MaudioXponent.pauseScript(500);

    for (var led in MaudioXponent.leds) {
        midi.sendShortMsg(MaudioXponent.on, MaudioXponent.leds[led], 0x00);
        midi.sendShortMsg(MaudioXponent.on + 1, MaudioXponent.leds[led], 0x00);
        MaudioXponent.pauseScript(15);
    }

    // VU meters
    midi.sendShortMsg(0xB3,0x12,0);
    midi.sendShortMsg(0xB3,0x13,0);
    
    // Progress meters
    midi.sendShortMsg(0xB3,0x14,0);
    midi.sendShortMsg(0xB3,0x15,0);
};

MaudioXponent.syncLights = function() {
    for (i = 0; i < MaudioXponent.state.numDecks; i++) {
        var channel = i + 1;
        var group = "[Channel" + channel + "]";
        engine.trigger(group, "keylock");
        engine.trigger(group, "hotcue_1_enabled");
        engine.trigger(group, "hotcue_2_enabled");
        engine.trigger(group, "hotcue_3_enabled");
        engine.trigger(group, "hotcue_4_enabled");
        engine.trigger(group, "hotcue_5_enabled");
    }
};

MaudioXponent.convert = function(value) {
    return (value * 127).toFixed(0);
};

MaudioXponent.getDeck = function(group) {
    var deck = MaudioXponent.state.deck[parseInt(group.substring(8)) - 1];
};

MaudioXponent.pauseScript = function(ms) {
    startDate = new Date();
    currentDate = null;
    while(currentDate-startDate < ms) currentDate = new Date();
};

MaudioXponent.wheel = function (channel, control, value, status, group) {
    var deck = MaudioXponent.state.deck[channel];
    
    if (deck.shift) {
        if (value > 64) {
            MaudioXponent.state["plnumberpos"]++;
            if (MaudioXponent.state["plnumberpos"] % 12 == 0) {
                engine.setValue("[Playlist]", "SelectTrackKnob",1);
            }
        } else if (value < 64) {
            MaudioXponent.state["plnumberneg"]++;
            if (MaudioXponent.state["plnumberneg"] % 12 == 0) {
                engine.setValue("[Playlist]", "SelectTrackKnob",-1);
            }  
        }
    } else {
        if (deck.scratching) {
    	    engine.scratchTick(deck.id, value-64);
        } else {
            engine.setValue(group, "jog", (value-64)/8);
        }
    }
};

MaudioXponent.wheelTouch = function(channel, control, value, status) {
    //script.midiDebug(channel, control, value, status)
    var deck = MaudioXponent.state.deck[channel];
    var activate = (status == deck.on);
    
    if (activate) {
        if (deck.scratchEnabled) {
            engine.scratchEnable(deck.id, 3*128, 33+1/3, 1.0/8, (1.0/8)/32);
            deck.scratching = true;
        }
    } else {
        engine.scratchDisable (deck.id);
        deck.scratching = false;
    }    
}

MaudioXponent.onBeatActive = function(value, group) {
    var deck = MaudioXponent.getDeck[group];

    if (MaudioXponent.config.syncFlashMode === 1) {
        midi.sendShortMsg(deck.on, 0x02, value);
    }
    
    if (value) {
        deck.beatState = !deck.beatState;
        
        if (MaudioXponent.config.syncFlashMode === 2) {
            midi.sendShortMsg(deck.on, 0x02, deck.beatState);
        }
    }
};

MaudioXponent.onBeatLoop = function(value, group, control) {
    var deck = MaudioXponent.getDeck[group];
    var offset = Math.log(parseInt(control.substring(9))) / Math.log(2)
    
    midi.sendShortMsg(deck.on, MaudioXponent.leds.loop1 + offset, value);
}

MaudioXponent.onBpmChanged = function(value, group) {
    //print ("BPM Change, " + group + ", value=" + value);
};

MaudioXponent.onPlayPositionChange = function(value, group) {
    var deck = MaudioXponent.getDeck[group];

    if (deck) {
        if ((value < deck.warnAt) || (value >= deck.warnAt && deck.beatState)) {
            midi.sendShortMsg(0xB3, 0x14 + channel, MaudioXponent.convert(value));
        } else {
            midi.sendShortMsg(0xB3, 0x14 + channel, 0x00);        
        }
    }
};

MaudioXponent.volumeLEDs = function(channel, value) {
    midi.sendShortMsg(0xB3,0x12 + channel, MaudioXponent.convert(value));
};

MaudioXponent.actbin = function(channel, control, value, status) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.state.deck[channel];
    var activenow = engine.getValue(deck.group, MaudioXponent.binleds[control]);

    if (activenow) {
        print("activenow: group=" + deck.group + ", control=" + MaudioXponent.binleds[control] + ", value=0");
        engine.setValue(deck.group, MaudioXponent.binleds[control], 0);
        midi.sendShortMsg(deck.off, control, 0x00);
    } else {
        print("activenow: group=" + deck.group + ", control=" + MaudioXponent.binleds[control] + ", value=1");
        engine.setValue(deck.group, MaudioXponent.binleds[control], 1);
        midi.sendShortMsg(deck.on, control, 0x01);
    }
};

MaudioXponent.actbinstop = function(channel, control, value, status) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.state.deck[channel];
    engine.setValue(deck.group, MaudioXponent.binleds[control], 0);
    midi.sendShortMsg(deck.off,control,0x00);
};

MaudioXponent.onHotCue = function(value, group, control) {
    var deck = MaudioXponent.getDeck[group];
    var cueNumber = parseInt(control.substring(7)) - 1;
    midi.sendShortMsg(deck.on, MaudioXponent.leds.cue1 + cueNumber, value);
}

MaudioXponent.hotcue = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group)
    var deck = MaudioXponent.state.deck[channel];
    var cueNumber = control - 0x16;

    if (!deck.shift) {
        var activate = (status == deck.on);
        engine.setValue(group, "hotcue_" + cueNumber + "_activate", activate);
    } else {
        engine.setValue(group, "hotcue_" + cueNumber + "_clear", 1);
    }
};

MaudioXponent.onLoopIn = function(value, group, control){
    var deck = MaudioXponent.getDeck[group];
    midi.sendShortMsg(deck.on, 0x29, engine.getValue(group, control) != -1);
}

MaudioXponent.onLoopOut = function(value, group, control) 
{
    var deck = MaudioXponent.getDeck[group];
    midi.sendShortMsg(deck.on, 0x2B, engine.getValue(group, control) != -1);
}

MaudioXponent.onLoopExit = function(value, group, control) 
{
    var deck = MaudioXponent.getDeck[group];
    midi.sendShortMsg(deck.on, 0x2A, engine.getValue(group, control) == 1);
}

MaudioXponent.loopin = function(channel, control, value, status) {
    var deck = MaudioXponent.state.deck[channel];
    engine.setValue(deck.group, "loop_in", 1);
};

MaudioXponent.loopout = function(channel, control, value, status) {
    // TODO: Can we use the channel passed in?
    var deck = MaudioXponent.state.deck[channel];
    engine.setValue(deck.group, "loop_out", 1);
};

MaudioXponent.loopexit = function(channel, control, value, status) {
    // TODO: Can we use the channel passed in?
    var deck = MaudioXponent.state.deck[channel];
    engine.setValue(deck.group, "reloop_exit", 1);
};

MaudioXponent.punchIn = function(channel, control, value, status) {
    // TODO: Can we use the channel passed in?
    var deck = MaudioXponent.state.deck[channel];
    var activate = (status == deck.on);

    if (activate) {
        MaudioXponent.state.faderPosition = faderPosition = engine.getValue("[Master]", "crossfader");

        if ((faderPosition >= 0.90 && channel == 0) || (faderPosition <= -0.90 && channel == 1)) {
            midi.sendShortMsg(deck.on,control,0x01);
            engine.setValue("[Master]", "crossfader", 0);
        }
    } else {
        engine.setValue("[Master]", "crossfader", MaudioXponent.state.faderPosition);
        midi.sendShortMsg(deck.on, control, 0x00);
    }
};

MaudioXponent.pitch = function(channel, control, value, status, group) {
    engine.setValue(group, "rate", script.midiPitch(control, value, status));
};

MaudioXponent.secondaryon = function(channel, control, value, status) {
    // TODO: Can we use the channel passed in?
    var deck = MaudioXponent.state.deck[channel];
    deck.shift = true;
    midi.sendShortMsg(deck.on, control, 0x01);
};

MaudioXponent.secondaryoff = function(channel, control, value, status) {
    // TODO: Can we use the channel passed in?
    var deck = MaudioXponent.state.deck[channel];
    deck.shift = false;
    midi.sendShortMsg(deck.on,control,0x00);
};

MaudioXponent.toggleScratchMode = function(channel, control, value, status) {
    script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.state.deck[channel];
    
    deck.scratchEnabled = !deck.scratchEnabled;
    midi.sendShortMsg(deck.on, control, deck.scratchEnabled);
};

MaudioXponent.playlist = function(channel, control, value, status) {
    var deck = MaudioXponent.state.deck[channel];
    switch (control) {
    case 28:
        midi.sendShortMsg(deck.on,control,0x01);    // Turn on the LED
        engine.setValue("[Playlist]", "SelectPrevTrack",1);
	break;
    case 29:
        midi.sendShortMsg(deck.on,control,0x01);    // Turn on the LED
        engine.setValue("[Playlist]", "SelectNextTrack",1);
	break;
    case 30:
        midi.sendShortMsg(deck.on,control,0x01);    // Turn on the LED
        var activenow = engine.getValue(deck.group, "play");
        if (activenow == 1) {    // If currently active
            engine.setValue("[Playlist]", "LoadSelectedIntoFirstStopped",1);
        }else{
            engine.setValue(deck.group, "LoadSelectedTrack",1);
        }
	break;
    case 31:
        midi.sendShortMsg(deck.on,control,0x01);    // Turn on the LED
        engine.setValue("[Playlist]", "SelectPrevPlaylist",1);
	break;
    case 32:
        midi.sendShortMsg(deck.on,control,0x01);    // Turn on the LED
        engine.setValue("[Playlist]", "SelectNextPlaylist",1);
	break;
    }
};

MaudioXponent.playlistoff = function(channel, control, value, status) {
    var deck = MaudioXponent.state.deck[channel];
    midi.sendShortMsg(deck.off, control, 0x00);
};

MaudioXponent.onKeyLock = function(value, group) {
    var deck = MaudioXponent.getDeck[group];

    if (value) {
        midi.sendShortMsg(deck.on, 0x1E, 0x01)
    } else {
        midi.sendShortMsg(deck.off, 0x1E, 0x00)
    }
};

MaudioXponent.toggleKeylock = function(channel, control, value, status) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.state.deck[channel];
 
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

MaudioXponent.brake = function(channel, control, value, status) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.state.deck[channel];
    var activate = (status == deck.on);
    
    if (activate) {
        engine.brake(deck.id, true);
    } else {
        engine.brake(deck.id, false)
    }
};

MaudioXponent.cuedefon = function(channel, control, value, status) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.state.deck[channel];
    engine.setValue(deck.group, "cue_default",1);
    midi.sendShortMsg(deck.on, control, 0x01);
};

MaudioXponent.cuedefoff = function(channel, control, value, status) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.state.deck[channel];
    engine.setValue(deck.group, "cue_default",0);
    midi.sendShortMsg(deck.off, control, 0x00);
};

MaudioXponent.volbuttonon = function(channel, control, value, status) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.state.deck[channel];
    engine.setValue(deck.group, "volume",0);
    midi.sendShortMsg(deck.on,control,0x01);
};

MaudioXponent.volbuttonoff = function(channel, control, value, status) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.state.deck[channel];
    engine.setValue(deck.group, "volume",1);
    midi.sendShortMsg(deck.off, control, 0x00);
};

MaudioXponent.shutdown = function (id) {    // called when the MIDI device is closed
    // TODO: How does 4-deck mode affect this?
    for (var led in MaudioXponent.leds) {
        midi.sendShortMsg(MaudioXponent.on,MaudioXponent.leds[led],0x00);
        midi.sendShortMsg(MaudioXponent.on + 1,MaudioXponent.leds[led],0x00);
    }
    midi.sendShortMsg(0xB3,0x14,0x00);
    midi.sendShortMsg(0xB3,0x15,0x00);
	
	midi.sendSysexMsg(MaudioXponent.Handshake3, MaudioXponent.Handshake3.length);
};

MaudioXponent.beatgridAdjust = function(channel, control, value, status) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.state.deck[channel];

    if (!deck.shift) {
        // Unshifted = earlier / later
        if (control == MaudioXponent.leds.leftkey){
            engine.setValue(deck.group, "beats_translate_earlier", 1);
        } else {
            engine.setValue(deck.group, "beats_translate_later", 1);
        }
    } else {
        // Shifted = align
        engine.setValue(deck.group, "beats_translate_curpos", 1);
    }
};

MaudioXponent.beatgridLoop = function(channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.state.deck[channel];
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
            //print ("Exit rolling beat loop");
            MaudioXponent.loopexit(channel, control, value, status)
        }
    }
};

MaudioXponent.beatsync = function(channel, control, value, status) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.state.deck[channel];

    if (!deck.shift) {
        engine.setValue(deck.group, "beatsync", 1);
    } else {
        engine.setValue(deck.group, "bpm_tap", 1);
    }
};

MaudioXponent.fx = function(channel, control, value, status) {
    //script.midiDebug(channel, control, value, status);
    var deck = MaudioXponent.state.deck[channel];
    var currentControl = control - 0x0B;
    var isKnob = (status == 0xB1);

    if (isKnob) {
        var group = "[EffectRack1_EffectUnit" + MaudioXponent.state.focusedEffect + "_Effect1]";
        var scaledValue = value / 0x7F;

        if (currentControl == 4) {
            // Wet / Dry
            //print("Group " + group + ", parameter = " + currentControl + ", value = " + scaledValue);
            engine.setParameter("[EffectRack1_EffectUnit" + MaudioXponent.state.focusedEffect + "]", "mix", scaledValue);
        } else {
            // Other parameter
            //print("Group " + group + ", parameter = " + currentControl + ", value = " + scaledValue);
            engine.setParameter(group, "parameter" + currentControl, scaledValue);
        }
    } else {
        var group = "[EffectRack1_EffectUnit" + currentControl + "]";
        // print("Button " + group + ", MaudioXponent.state.focusedEffect=" + MaudioXponent.state.focusedEffect);

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
            for(i = 0; i < 4; i++) {
                var ctrl = MaudioXponent.leds.fx1 + i;
                var fxNum = i + 1;
                var newValue = (fxNum == MaudioXponent.state.focusedEffect) ? 1 : 0;

                //print("fxNum = " + fxNum + ", channel = " + channel + ", ctrl = 0x0" + ctrl.toString(16) + ", newValue = " + newValue);        
                midi.sendShortMsg(deck.on, ctrl, newValue);
            }
        }
    }
};

MaudioXponent.onTrackLoaded = function(duration, group) {
    var deck = MaudioXponent.getDeck[group];
    deck.warnAt = (duration - 30) / parseFloat(duration);
};

MaudioXponent.nudge = function (channel, control, value, status, group) {
    //script.midiDebug(channel, control, value, status, group);
    var deck = MaudioXponent.state.deck[channel];
    var controlName = ((control == 0x10 && MaudioXponent.config.nudgeButtonMode == 0) || (control != 0x10 && MaudioXponent.config.nudgeButtonMode == 1))
        ? "rate_temp_down" 
        : "rate_temp_up";
    var activate = (status == deck.on);
    engine.setValue(group, controlName, activate);
};

MaudioXponent.pfl = function(channel, control, value, status) {
    //script.midiDebug(channel, control, value, status);
    var numDecks = engine.getValue("[Master]", "num_decks");

    for (i = 1; i <= numDecks; i++) {
        var group = "[Channel" + i + "]";
        var currentValue = engine.getValue(group, "pfl");

        if (i == channel + 1) {
            engine.setValue(group, "pfl", !currentValue);
        } else if (MaudioXponent.config.pflMode == 1) {
            engine.setValue(group, "pfl", 0);
        }
    }
}