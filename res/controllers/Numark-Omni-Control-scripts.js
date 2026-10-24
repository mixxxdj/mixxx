function NumarkOmniControl() {}
NumarkOmniControl.LoadUserSettings = function () {

    // Autostart values for functions with buttons
    NumarkOmniControl.toggleExtendedLooping(null, null, engine.getSetting("extendedLooping"), null, null);
    NumarkOmniControl.toggleScratchMode(null, null, engine.getSetting("scratchMode"), null, null);

    // QuickEffects for both channels
    engine.setValue("[QuickEffectRack1_[Channel1]]", "enabled", engine.getSetting("enableQuickEffects"));
    engine.setValue("[QuickEffectRack1_[Channel2]]", "enabled", engine.getSetting("enableQuickEffects"));

    // softTakeover only if user wants it
    engine.softTakeover("[Channel1]", "rate", engine.getSetting("enableSoftTakeover"));
    engine.softTakeover("[Channel2]", "rate", engine.getSetting("enableSoftTakeover"));

    // Cached values of JogWheel and FinePitch for less jumps to engine
    NumarkOmniControl.finePitchAdjustment = engine.getSetting("finePitchAdjustment");
    NumarkOmniControl.jogScratchAdjustment = engine.getSetting("jogScratchAdjustment");
    NumarkOmniControl.jogMoveAdjustment = engine.getSetting("jogMoveAdjustment");

    // Advanced settings
    NumarkOmniControl.simpleCue = engine.getSetting("simpleCue");
    NumarkOmniControl.simplePFL = engine.getSetting("simplePFL");

    // get all current positions from controller
    NumarkOmniControl.ReadAllControllerValues();
}

NumarkOmniControl.init = function (id) { // called when the MIDI device is opened & set up
    NumarkOmniControl.id = id; // Store the ID of this device for later use

    NumarkOmniControl.scratchTimer = [-1, -1, -1]; // Master (unused yet), left channel, right channel  // -1 for not set

    // LED-mapping
    NumarkOmniControl.leds = [
        // Common
        {
            "scratchMode": 0x32, // button: left channel: ON/OFF
            "extendedLooping": 0x46, // button: right channel: ON/OFF
            "directoryMode": 0x56 // button: master: directory
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
    }, 1);

    engine.beginTimer(1000, function () { // loading user defaults from Settings -> Controller and give MIXXX some time to react to ReadAllContollerValues()
        NumarkOmniControl.LoadUserSettings();
    }, 1);
}

NumarkOmniControl.ReadAllControllerValues = function () { // just send; OmniControl sends back state of all buttons and faders and MIXXX applies to engine itself
    // 0xF0 = Start // 0x00, 0x01, 0x3F = Numark // 0x00, 0x74 = Omni Control // 0x60, 0x00, 0x04, 0x00, 0x0D, 0x00, 0x00 = "Tell me your knobs" // 0xF7 = End
    // WARNING!!! change it only, if you know what you are doing
    const byteArray = [0xF0, 0x00, 0x01, 0x3F, 0x00, 0x74, 0x60, 0x00, 0x04, 0x00, 0x0D, 0x00, 0x00, 0xF7];
    midi.sendSysexMsg(byteArray, byteArray.length);
}

NumarkOmniControl.SetAllLED = function (value) {
    for (var i = 0x30; i <= 0x56; i++) {
        NumarkOmniControl.setLED(i, value);
    }
}

NumarkOmniControl.shutdown = function (id) { // called when the MIDI device is closed
    NumarkOmniControl.SetAllLED(false);
}

NumarkOmniControl.groupToDeck = function (group) {
    var matches = group.match(/\[Channel(\d+)\]/);

    if (matches === null) {
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
    engine.setValue(group, "MoveVertical", value);
}

NumarkOmniControl.pressTrackKnob = function (channel, control, value, status, group) {
    if (value) {
        switch (engine.getValue(group, "focused_widget")) {
        case 2: // Folder
            engine.setValue(group, "MoveRight", 1); // aka open sub-folders
            break;
        case 3: // Tracklist
            engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 1);
            break;
        default: // somewhere else --> ignore
        }
    };
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
        if (start !== -1) {
            if (end !== -1) {
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
        engine.setValue(group, "loop_halve", value);
    } else {
        engine.setValue(group, "rate_temp_down", value);
    }
}

NumarkOmniControl.rightFunction = function (channel, control, value, status, group) {
    if (engine.getValue(group, "loop_enabled") && NumarkOmniControl.extendedLooping) {
        engine.setValue(group, "loop_double", value);
    } else {
        engine.setValue(group, "rate_temp_up", value);
    }
}

NumarkOmniControl.finePitch = function (channel, control, value, status, group) {
    if (value > 63) {
        value = value - 128;
    }
    engine.setValue(group, "rate", engine.getValue(group, "rate") - value / (2 ** NumarkOmniControl.finePitchAdjustment));
}

NumarkOmniControl.playFromCue = function (channel, control, value, status, group) { // If playing, stutters from cuepoint; otherwise jumps to cuepoint and stops
    if (engine.getSetting("simpleCue")) {
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
    var signMultiplier = 1;
    if (adjustedJog > 63) { // Counter-clockwise
        signMultiplier = -1;
        adjustedJog = value - 128;
    }

    if (NumarkOmniControl.scratchMode) {
        if (NumarkOmniControl.scratchTimer[deck] === -1) {
            var alpha = 1.0 / (2 ** NumarkOmniControl.jogScratchAdjustment);
            var beta = alpha / 32;
            engine.scratchEnable(deck, 128, 33 + 1 / 3, alpha, beta);
        } else {
            engine.stopTimer(NumarkOmniControl.scratchTimer[deck]);
        }
        engine.scratchTick(deck, adjustedJog);
        NumarkOmniControl.scratchTimer[deck] = engine.beginTimer(20, () => NumarkOmniControl.jogWheelStopScratch(deck), true);
    } else {
        var gammaInputRange = 64; // Max jog speed
        var maxOutFraction = 0.5; // Where on the curve it should peak; 0.5 is half-way
        var sensitivity = 0.5; // Adjustment gamma
        var gammaOutputRange = NumarkOmniControl.jogMoveAdjustment; // Max rate change
        if (engine.getValue(group, "play")) {
            adjustedJog = signMultiplier * gammaOutputRange * Math.pow(Math.abs(adjustedJog) / (gammaInputRange * maxOutFraction), sensitivity);
        } else {
            adjustedJog = gammaOutputRange * adjustedJog / (gammaInputRange * maxOutFraction);
        }
        engine.setValue(group, "jog", adjustedJog);
    }
}

NumarkOmniControl.jogWheelStopScratch = function (deck) {
    NumarkOmniControl.scratchTimer[deck] = -1;
    engine.scratchDisable(deck);
}

NumarkOmniControl.toggleDirectoryMode = function (channel, control, value, status, group) {
    // Toggle setting; light is set via XML mapping
    if (value) {
        engine.setValue(group, "focused_widget", (engine.getValue(group, "focused_widget") === 2) ? 3 : 2);
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
    if (value) {
        var newvalue = !engine.getValue(group, "enabled");
        engine.setValue(group, "enabled", newvalue);
        var deck = NumarkOmniControl.groupToDeck(group);
        NumarkOmniControl.setLED(NumarkOmniControl.leds[deck]["filter_onoff"], newvalue);
    }
}

NumarkOmniControl.tooglePFL = function (channel, control, value, status, group) { // scrolls throught possible FX/Filter effects
    if (NumarkOmniControl.simplePFL && !engine.getValue(group, "pfl") && value) {
        var opposite = (group === "[Channel1]") ? "[Channel2]" : "[Channel1]";
        engine.setValue(group, "pfl", 1);
        engine.setValue(opposite, "pfl", 0);
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
