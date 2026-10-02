function NumarkOmniControl() {}
NumarkOmniControl.LoadUserSettings = function () {

    // Quality functions (no buttons assigned, so must be changed here)
    NumarkOmniControl.SetUserDefaults("simpleCue", true); // true = CUE doesn't stop playing deck
    NumarkOmniControl.SetUserDefaults("simplePFL", true); // true = if PFL is activated, it disables the PFL on the opposite site

    // Modes which also have buttons to switch on and off
    NumarkOmniControl.SetUserDefaults("extendedLooping", true); // enables extendedLooping // button: right channel: ON/OFF
    NumarkOmniControl.SetUserDefaults("ScratchMode", false); // enables scratchMode  // button: left channel: ON/OFF
    NumarkOmniControl.SetUserDefaults("DirectoryMode", false); // enables directoryMode  // button: master: directory

    // Adjust JogWheel and FinePitch
    NumarkOmniControl.FinePitchAdjustment = 9; // the higher the value, the finer the changes done by FinePitch (exponential!) // 6 = same as rate sliders
    NumarkOmniControl.JogScratchAdjustment = 3; // the higher the value, the slower the scratches by JogWheel rotation (only when ScratchMode enabled) (exponential!)
    NumarkOmniControl.JogMoveAdjustment = 3; // the higher the value, the faster the "movement" by JowWheel rotation (only when ScratchMode disabled)
}

NumarkOmniControl.init = function (id) { // called when the MIDI device is opened & set up
    NumarkOmniControl.id = id; // Store the ID of this device for later use

    // features & defaults (in case of LoadUserSettings are removed)
    NumarkOmniControl.simpleCue = true
    NumarkOmniControl.simplePFL = true;
    
	NumarkOmniControl.extendedLooping = true;
	NumarkOmniControl.directoryMode = false;
    NumarkOmniControl.scratchMode = false;
    
	NumarkOmniControl.FinePitchAdjustment = 9; // (recommendation)
    NumarkOmniControl.JogScratchAdjustment = 3; // (recommendation)
    NumarkOmniControl.JogMoveAdjustment = 3; // (recommendation)
	
	NumarkOmniControl.scratchTimer = [-1, -1];

    // ensure, QuickEffects aren't active
    engine.setValue("[QuickEffectRack1_[Channel1]]", "enabled", false);
    engine.setValue("[QuickEffectRack1_[Channel2]]", "enabled", false);

    // LED-mapping
    NumarkOmniControl.leds = [
        // Common
        {
            "scratchMode": 0x32, // aka Channel 1 - ON/OFF
            "extendedLooping": 0x46, // aka Channel 2 - ON/OFF
            "directory": 0x56
        },
        // Deck 1
        {
            "tap": 0x30,
            "filter_onoff": 0x31,
            "par_onoff": 0x32, // already used for customising (see common)
            "fx_select": 0x33,
            "rate": 0x34,
            "pfl": 0x35,
            "key": 0x36,
            "sync": 0x37,
            "pitchbend-": 0x38,
            "pitchbend+": 0x39,
            "loopIn": 0x3a,
            "loopOut": 0x3b,
            "cue": 0x3c,
            "set_cue": 0x3d,
            "play": 0x3e,
            "load_track": 0x3f,
            "treble_kill": 0x50,
            "mid_kill": 0x51,
            "bass_kill": 0x52
        },
        // Deck 2
        {
            "pfl": 0x40,
            "key": 0x41,
            "sync": 0x42,
            "rate": 0x43,
            "fx_select": 0x44,
            "filter_onoff": 0x45,
            "par_onoff": 0x46, // already used for customising (see common)
            "tap": 0x47,
            "pitchbend-": 0x48,
            "pitchbend+": 0x49,
            "loopIn": 0x4a,
            "loopOut": 0x4b,
            "cue": 0x4c,
            "set_cue": 0x4d,
            "play": 0x4e,
            "load_track": 0x4f,
            "treble_kill": 0x53,
            "mid_kill": 0x54,
            "bass_kill": 0x55
        }
    ];

    NumarkOmniControl.SetAllLED(true); // check LED health
    engine.beginTimer(500, function () {
        NumarkOmniControl.SetAllLED(false);
        NumarkOmniControl.ReadAllControllerValues(); // and get all current positions from controller
    }, 1);

    engine.beginTimer(2000, function () { // loading user defaults and give MIXXX some time to react to ReadAllContollerValues()
        NumarkOmniControl.LoadUserSettings();
    }, 1);
}

NumarkOmniControl.ReadAllControllerValues = function () { // just send; OmniControl sends back state of all buttons and faders and MIXXX applies to engine itself
    // 0xF0 = Start // 0x00, 0x01, 0x3F = Numark // 0x00, 0x74 = Omni Controll // 0x60, 0x00, 0x04, 0x00, 0x0D, 0x00, 0x00 = "Tell me your knobs" // 0xF7 = End
    // WARNING!!! change it only, if you know what you are do
    const byteArray = [0xF0, 0x00, 0x01, 0x3F, 0x00, 0x74, 0x60, 0x00, 0x04, 0x00, 0x0D, 0x00, 0x00, 0xF7];
    midi.sendSysexMsg(byteArray, byteArray.length);
}

NumarkOmniControl.SetAllLED = function (newvalue) {
    for (var i = 0x30; i <= 0x56; i++) {
        NumarkOmniControl.setLED(i, newvalue);
    }
}

NumarkOmniControl.shutdown = function (id) { // called when the MIDI device is closed
    NumarkOmniControl.SetAllLED(false);
}

NumarkOmniControl.groupToDeck = function (group) {
    var matches = group.match(/\[Channel(\d+)\]/);
    // var matches = group.match(/^\[Channel(\d+)\]$/);
    if (matches == null) {
        return -1;
    } else {
        return matches[1];
    }
}

NumarkOmniControl.samplesPerBeat = function (group) {
    var sampleRate = engine.getValue(group, "track_samplerate");
    // FIXME: Get correct channel count for current deck
    // gonzo.LE: property would be "[App],num_decks", but is it not always 2 for stereo?!
    var channels = 2;
    var bpm = engine.getValue(group, "file_bpm");
    return channels * sampleRate * 60 / bpm;
}

NumarkOmniControl.setLED = function (value, status) {
    if (status) {
        status = 0x64;
    } else {
        status = 0x00;
    }
    midi.sendShortMsg(0x90, value, status);
}

NumarkOmniControl.selectKnob = function (channel, control, value, status, group) {
    if (value > 63) {
        value = value - 128;
    }
    if (NumarkOmniControl.directoryMode) {
        if (value > 0) {
            for (var i = 0; i < value; i++) {
                engine.setValue(group, "SelectNextPlaylist", 1);
            }
        } else {
            for (var i = 0; i < -value; i++) {
                engine.setValue(group, "SelectPrevPlaylist", 1);
            }
        }
    } else {
        engine.setValue(group, "SelectTrackKnob", value);
    }
}

NumarkOmniControl.pressTrackKnob = function (channel, control, value, status, group) {
    if (value == 0) {
        return;
    };
    if (NumarkOmniControl.directoryMode) {
        engine.setValue("[Library]", "focused_widget", 2);
        engine.setValue("[Library]", "MoveRight", true); // aka open sub-folders
    } else {
        engine.setValue("[Library]", "focused_widget", 3);
        engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", true);
    }
}

NumarkOmniControl.loopIn = function (channel, control, value, status, group) {
    if (value) {
        if (engine.getValue(group, "loop_enabled")) {
            engine.setValue(group, "reloop_toggle", 1);
        }
        engine.setValue(group, "loop_in", 1);
        engine.setValue(group, "loop_end_position", -1);
    }
}

NumarkOmniControl.loopOut = function (channel, control, value, status, group) {
    if (value) {
        var start = engine.getValue(group, "loop_start_position");
        var end = engine.getValue(group, "loop_end_position");
        if (start != -1) {
            if (end != -1) {
                // Loop In and Out set -> call Reloop/Exit
                engine.setValue(group, "reloop_toggle", 1);
            } else {
                engine.setValue(group, "loop_out", 1);
            }
        }
    }
}

NumarkOmniControl.leftFunction = function (channel, control, value, status, group) {
    if (engine.getValue(group, "loop_enabled") && NumarkOmniControl.extendedLooping) {
        engine.setValue(group, "loop_halve", value)
    } else {
        engine.setValue(group, "rate_temp_down", value)
    }
}

NumarkOmniControl.rightFunction = function (channel, control, value, status, group) {
    if (engine.getValue(group, "loop_enabled") && NumarkOmniControl.extendedLooping) {
        engine.setValue(group, "loop_double", value)
    } else {
        engine.setValue(group, "rate_temp_up", value)
    }
}

NumarkOmniControl.finePitch = function (channel, control, value, status, group) {
    if (value > 63) {
        value = value - 128;
    }
    engine.setValue(group, "rate", engine.getValue(group, "rate") - value / (2 ** NumarkOmniControl.FinePitchAdjustment));
}

NumarkOmniControl.playFromCue = function (channel, control, value, status, group) { // If playing, stutters from cuepoint; otherwise jumps to cuepoint and stops
    if (NumarkOmniControl.simpleCue) {
        if (value) {
            if (engine.getValue(group, "play")) {
                engine.setValue(group, "cue_goto", 1);
            } else {
                engine.setValue(group, "cue_gotoandstop", 1);
            }
        }
    } else {
        if (value) {
            if (engine.getValue(group, "play")) {
                engine.setValue(group, "play", 0);
                engine.setValue(group, "cue_gotoandstop", 1);
            } else {
                engine.setValue(group, "cue_preview", 1);
            }
        } else {
            engine.setValue(group, "cue_preview", 0);
        }
    }
}

// Jog values: (counter) fast slow still slow fast (clockwise)
// Jog values:            064  127   -   001  063
NumarkOmniControl.jogWheel = function (channel, control, value, status, group) {
    var deck = NumarkOmniControl.groupToDeck(group);
    var adjustedJog = parseFloat(value);
    var posNeg = 1;
    if (adjustedJog > 63) { // Counter-clockwise
        posNeg = -1;
        adjustedJog = value - 128;
    }

    if (NumarkOmniControl.scratchMode) {
        if (NumarkOmniControl.scratchTimer[deck - 1] == -1) {
            var alpha = 1.0 / (2 ** NumarkOmniControl.JogScratchAdjustment);
            var beta = alpha / 32;
            engine.scratchEnable(deck, 128, 33 + 1 / 3, alpha, beta);
        } else {
            engine.stopTimer(NumarkOmniControl.scratchTimer[deck - 1]);
        }
        engine.scratchTick(deck, adjustedJog);
        NumarkOmniControl.scratchTimer[deck - 1] = engine.beginTimer(20, () => NumarkOmniControl.jogWheelStopScratch(deck), true);
    } else {
        var gammaInputRange = 64; // Max jog speed
        var maxOutFraction = 0.5; // Where on the curve it should peak; 0.5 is half-way
        var sensitivity = 0.5; // Adjustment gamma
        var gammaOutputRange = NumarkOmniControl.JogMoveAdjustment; // Max rate change
        if (engine.getValue(group, "play")) {
            adjustedJog = posNeg * gammaOutputRange * Math.pow(Math.abs(adjustedJog) / (gammaInputRange * maxOutFraction), sensitivity);
        } else {
            adjustedJog = gammaOutputRange * adjustedJog / (gammaInputRange * maxOutFraction);
        }
        engine.setValue(group, "jog", adjustedJog);
    }
}

NumarkOmniControl.jogWheelStopScratch = function (deck) {
    NumarkOmniControl.scratchTimer[deck - 1] = -1;
    engine.scratchDisable(deck);
}

NumarkOmniControl.toggleDirectoryMode = function (channel, control, value, status, group) {
    // Toggle setting and light
    if (value) {
        NumarkOmniControl.directoryMode = !NumarkOmniControl.directoryMode;
        NumarkOmniControl.setLED(NumarkOmniControl.leds[0]["directory"], NumarkOmniControl.directoryMode);
    }
}

NumarkOmniControl.toggleScratchMode = function (channel, control, value, status, group) {
    // Toggle setting and light
    if (value) {
        NumarkOmniControl.scratchMode = !NumarkOmniControl.scratchMode;
        NumarkOmniControl.setLED(NumarkOmniControl.leds[0]["scratchMode"], NumarkOmniControl.scratchMode);
    }
}

NumarkOmniControl.toggleExtendedLooping = function (channel, control, value, status, group) {
    // Toggle setting and light
    if (value) {
        NumarkOmniControl.extendedLooping = !NumarkOmniControl.extendedLooping;
        NumarkOmniControl.setLED(NumarkOmniControl.leds[0]["extendedLooping"], NumarkOmniControl.extendedLooping);
        // Extended Looping rely on quantize
        engine.setValue("[Channel1]", "quantize", NumarkOmniControl.extendedLooping);
        engine.setValue("[Channel2]", "quantize", NumarkOmniControl.extendedLooping);
    }
}

NumarkOmniControl.selectQuickEffect = function (channel, control, value, status, group) { // scrolls throught possible FX/Filter effects
    if (engine.getValue(group, "enabled")) {
        return; // ignore function call, if effect is already in use
    }
    if (value < 63) { // right -> next; left -> previous
        engine.setValue(group, "prev_chain", 1);
    } else {
        engine.setValue(group, "next_chain", 1);
    }
}

NumarkOmniControl.toggleQuickEffect = function (channel, control, value, status, group) { // JS parser stops if buttons are direct assigned via mapping
    // Toggle setting and light
    // should be [QuickEffectRack1_[Channel2]_Effect1] but it's not responding, so using [QuickEffectRack1_[Channel2]] instead
    if (value) {
        var newvalue = !engine.getValue(group, "enabled");
        engine.setValue(group, "enabled", newvalue);
        var deck = NumarkOmniControl.groupToDeck(group);
        NumarkOmniControl.setLED(NumarkOmniControl.leds[deck]["filter_onoff"], newvalue);
    }
}

NumarkOmniControl.tooglePFL = function (channel, control, value, status, group) { // scrolls throught possible FX/Filter effects
    if (NumarkOmniControl.simplePFL && engine.getValue(group, "pfl") == false && value) {
        var opposite = (group == "[Channel1]") ? "[Channel2]" : "[Channel1]";
        engine.setValue(group, "pfl", true);
        engine.setValue(opposite, "pfl", false);
    } else if (value) {
        engine.setValue(group, "pfl", !engine.getValue(group, "pfl"));
    }
}

NumarkOmniControl.jumpBeats = function (channel, control, value, status, group) { // jumps forward or backward using the beatsize value of the channel
    if (value > 63) {
        value = value - 128;
    }
    engine.setValue(group, "beatjump", value);
}

NumarkOmniControl.SetUserDefaults = function (setting, value) { // keeps LoadUserDefaults smaller
    switch (setting) {
    case "simpleCue":
        NumarkOmniControl.simpleCue = value;
        break;
    case "extendedLooping":
        NumarkOmniControl.toggleExtendedLooping(null, null, value, null, null);
        break;
    case "ScratchMode":
        NumarkOmniControl.toggleScratchMode(null, null, value, null, null);
        break;
    case "DirectoryMode":
        NumarkOmniControl.toggleDirectoryMode(null, null, value, null, null);
        break;
    case "simplePFL":
        NumarkOmniControl.simplePFL = value;
        break;
    default:
        // unknown setting -> do nothing
    }
}
