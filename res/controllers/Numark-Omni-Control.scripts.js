/* eslint-disable func-style -- Mixxx's controller mapping loader requires a function declaration so the constructor is visible to the engine */
/**
 *
 */
function NumarkOmniControl() {}

NumarkOmniControl.init = function(id) { // called when the MIDI device is opened & set up
    NumarkOmniControl.id = id; // Store the ID of this device for later use

    NumarkOmniControl.scratchTimer = [-1, -1, -1]; // Master0 (unused), Deck1, Deck2  // -1 for not set
    NumarkOmniControl.SetAllLED(true); // check LED health

    engine.beginTimer(500, function() {
        NumarkOmniControl.SetAllLED(false);
    }, 1);

    engine.beginTimer(1000, function() { // loading user defaults from Settings -> Controller and give MIXXX some time to react to ReadAllContollerValues()
        NumarkOmniControl.LoadUserSettings();
    }, 1);
};

NumarkOmniControl.LoadUserSettings = function() {

    // Autostart values for functions with buttons // used PreviewDeck1 as LED-Indicator
    if (engine.getSetting("quantizeMode")) {
        engine.setValue("[PreviewDeck1]", "quantize", false);
        NumarkOmniControl.toggleQuantize(null, null, true, null, "[PreviewDeck1]");
    }

    // PreviewDeck1.repeat doesn't do any harm or interruption, so it's used to indicate LED, because MIDI-mapping doesn't read engine.getSetting("scratchMode")
    engine.setValue("[PreviewDeck1]", "repeat", engine.getSetting("scratchMode"));

    // QuickEffects for both channels
    engine.setValue("[QuickEffectRack1_[Channel1]]", "enabled", engine.getSetting("enableQuickEffects"));
    engine.setValue("[QuickEffectRack1_[Channel2]]", "enabled", engine.getSetting("enableQuickEffects"));

    // softTakeover only if user wants it
    engine.softTakeover("[Channel1]", "rate", engine.getSetting("enableSoftTakeover"));
    engine.softTakeover("[Channel2]", "rate", engine.getSetting("enableSoftTakeover"));

    // Cached values of JogWheel and FinePitch for less jumps to engine
    NumarkOmniControl.finePitchAdjustment = (2 ** engine.getSetting("finePitchAdjustment"));
    NumarkOmniControl.jogScratchAdjustment = 1.0 / (2 ** engine.getSetting("jogScratchAdjustment")); // aka alpha
    NumarkOmniControl.jogMoveAdjustment = engine.getSetting("jogMoveAdjustment");

    // Advanced settings
    NumarkOmniControl.simpleCue = engine.getSetting("simpleCue");
    NumarkOmniControl.simplePFL = engine.getSetting("simplePFL");

    // get all current positions from controller
    NumarkOmniControl.ReadAllControllerValues();
};

NumarkOmniControl.ReadAllControllerValues = function() { // just send; OmniControl sends back state of all buttons and faders and MIXXX applies to engine itself
    // 0xF0 = Start // 0x00, 0x01, 0x3F = Numark // 0x00, 0x74 = Omni Control // 0x60, 0x00, 0x04, 0x00, 0x0D, 0x00, 0x00 = "Tell me your knobs" // 0xF7 = End
    // WARNING!!! change it only, if you know what you are doing
    const byteArray = [0xF0, 0x00, 0x01, 0x3F, 0x00, 0x74, 0x60, 0x00, 0x04, 0x00, 0x0D, 0x00, 0x00, 0xF7];
    midi.sendSysexMsg(byteArray, byteArray.length);
};

NumarkOmniControl.SetAllLED = function(value) {
    for (let i = 0x30; i <= 0x56; i++) {
        NumarkOmniControl.setLED(i, value);
    }
};

NumarkOmniControl.shutdown = function() { // called when the MIDI device is closed
    NumarkOmniControl.SetAllLED(false);
};

NumarkOmniControl.setLED = function(value, status) {
    if (status) {
        status = 0x64;
    } else {
        status = 0x00;
    }
    midi.sendShortMsg(0x90, value, status);
};

NumarkOmniControl.selectKnob = function(channel, control, value, status, group) {
    if (value > 63) {
        value = value - 128;
    }
    engine.setValue(group, "MoveVertical", value);
};

NumarkOmniControl.pressTrackKnob = function(channel, control, value, status, group) {
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
};

NumarkOmniControl.loopIn = function(channel, control, value, status, group) {
    if (value) {
        if (engine.getValue(group, "loop_enabled")) {
            engine.setValue(group, "reloop_toggle", 1);
        }
        engine.setValue(group, "loop_in", 1);
        engine.setValue(group, "loop_end_position", -1);
    }
};

NumarkOmniControl.loopOut = function(channel, control, value, status, group) {
    if (value) {
        const start = engine.getValue(group, "loop_start_position");
        const end = engine.getValue(group, "loop_end_position");
        if (start !== -1) {
            if (end !== -1) {
                // Loop In and Out set -> call Reloop/Exit
                engine.setValue(group, "reloop_toggle", 1);
            } else {
                engine.setValue(group, "loop_out", 1);
            }
        }
    }
};

NumarkOmniControl.leftFunction = function(channel, control, value, status, group) {
    if (engine.getValue(group, "loop_enabled") && engine.getValue("[PreviewDeck1]", "quantize")) {
        engine.setValue(group, "loop_halve", value);
    } else {
        engine.setValue(group, "rate_temp_down", value);
    }
};

NumarkOmniControl.rightFunction = function(channel, control, value, status, group) {
    if (engine.getValue(group, "loop_enabled") && engine.getValue("[PreviewDeck1]", "quantize")) {
        engine.setValue(group, "loop_double", value);
    } else {
        engine.setValue(group, "rate_temp_up", value);
    }
};

NumarkOmniControl.finePitch = function(channel, control, value, status, group) {
    if (value > 63) {
        value = value - 128;
    }
    engine.setValue(group, "rate", engine.getValue(group, "rate") - value / NumarkOmniControl.finePitchAdjustment);
};

NumarkOmniControl.playFromCue = function(channel, control, value, status, group) { // If playing, stutters from cuepoint; otherwise jumps to cuepoint and stops
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
};

// Jog values: (counter) fast slow still slow fast (clockwise)
// Jog values:            064  127   -   001  063
NumarkOmniControl.jogWheel = function(channel, control, value, status, group) {
    const deck = script.deckFromGroup(group);
    let adjustedJog = parseFloat(value);
    let signMultiplier = 1;
    if (adjustedJog > 63) { // Counter-clockwise
        signMultiplier = -1;
        adjustedJog = value - 128;
    }

    if (engine.getValue("[PreviewDeck1]", "repeat")) { // aka CUSTOM:ScratchMode
        if (NumarkOmniControl.scratchTimer[deck] === -1) {
            engine.scratchEnable(deck, 128, 33 + 1 / 3, NumarkOmniControl.jogScratchAdjustment, NumarkOmniControl.jogScratchAdjustment / 32);
        } else {
            engine.stopTimer(NumarkOmniControl.scratchTimer[deck]);
        }
        engine.scratchTick(deck, adjustedJog);
        NumarkOmniControl.scratchTimer[deck] = engine.beginTimer(20, () => NumarkOmniControl.jogWheelStopScratch(deck), true);
    } else {
        //        const gammaInputRange = 64; // Max jog speed
        //        const maxOutFraction = 0.5; // Where on the curve it should peak; 0.5 is half-way
        // gonzo-LE: values merged, because always used together -> 32
        const sensitivity = 0.5; // Adjustment gamma
        const gammaOutputRange = NumarkOmniControl.jogMoveAdjustment; // Max rate change
        if (engine.getValue(group, "play")) {
            adjustedJog = signMultiplier * gammaOutputRange * Math.pow(Math.abs(adjustedJog) / 32, sensitivity);
        } else {
            adjustedJog = gammaOutputRange * adjustedJog / 32;
        }
        engine.setValue(group, "jog", adjustedJog);
    }
};

NumarkOmniControl.jogWheelStopScratch = function(deck) {
    NumarkOmniControl.scratchTimer[deck] = -1;
    engine.scratchDisable(deck);
};

NumarkOmniControl.toggleDirectoryMode = function(channel, control, value, status, group) { // Toggle setting; light is set via XML mapping
    if (value) {
        engine.setValue(group, "focused_widget", (engine.getValue(group, "focused_widget") === 2) ? 3 : 2);
    }
};

NumarkOmniControl.toggleQuantize = function(_channel, _control, value, _status, _group) { // Toggle setting; light is set via XML mapping
    // PreviewDeck1 doesn't really use quantize, so it's used to indicate LED, because MIDI-mapping doesn't read engine.getSetting("quantizeMode")
    if (value) {
        const newValue = !engine.getValue("[PreviewDeck1]", "quantize");
        engine.setValue("[PreviewDeck1]", "quantize", newValue);
        engine.setValue("[Channel1]", "quantize", newValue);
        engine.setValue("[Channel2]", "quantize", newValue);
    }
};

NumarkOmniControl.selectQuickEffect = function(channel, control, value, status, group) { // scroll  possible FX/Filter effects
    if (engine.getValue(group, "enabled")) {
        return; // ignore function call, if effect is already in use
    }
    if (value < 63) { // right -> next; left -> previous
        engine.setValue(group, "prev_chain", 1);
    } else {
        engine.setValue(group, "next_chain", 1);
    }
};

NumarkOmniControl.toggleQuickEffect = function(channel, control, value, status, group) { // Toggle setting; light via XML
    if (value) {
        engine.setValue(group, "enabled", !engine.getValue(group, "enabled"));
    }
};

NumarkOmniControl.togglePFL = function(channel, control, value, status, group) {
    if (NumarkOmniControl.simplePFL && !engine.getValue(group, "pfl") && value) {
        const opposite = (group === "[Channel1]") ? "[Channel2]" : "[Channel1]";
        engine.setValue(group, "pfl", 1);
        engine.setValue(opposite, "pfl", 0);
    } else if (value) {
        engine.setValue(group, "pfl", !engine.getValue(group, "pfl"));
    }
};

NumarkOmniControl.jumpBeats = function(channel, control, value, status, group) { // jumps forward or backward using the beatsize value of the channel
    if (value > 63) {
        value = value - 128;
    }
    engine.setValue(group, "beatjump", value);
};
