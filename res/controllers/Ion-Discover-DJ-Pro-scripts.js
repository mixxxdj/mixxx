// Ion Discover DJ pro Mapping Script Functions
// Joachim, meltedpianoman
var IonDiscoverDjPro = function() {};

IonDiscoverDjPro.init = function() {	// called when the MIDI device is opened & set up
    IonDiscoverDjPro.directoryMode = false;
    IonDiscoverDjPro.scratchMode = [false, false, false];
    IonDiscoverDjPro.scratchTimer = [-1, -1, -1];
    IonDiscoverDjPro.manualLooping = [false, false, false];

    IonDiscoverDjPro.leds = [
        { },
        // Deck 1
        {"rate": 0x70, "scratchMode": 0x48, "keylock": 0x51, "manualLoop": 0x61, "loopIn": 0x53, "loopOut": 0x54, "reLoop": 0x55},
        // Deck 2
        {"rate": 0x71, "scratchMode": 0x50, "keylock": 0x52, "manualLoop": 0x62, "loopIn": 0x56, "loopOut": 0x57, "reLoop": 0x58}
    ];
};

IonDiscoverDjPro.shutdown = function() {	// called when the MIDI device is closed
    var lowestLED = 0x30;
    var highestLED = 0x73;
    for (var i=lowestLED; i<=highestLED; i++) {
        IonDiscoverDjPro.setLED(i, false);	// Turn off all the lights
    }
};

IonDiscoverDjPro.groupToDeck = function(group) {
    var matches = group.match(/^\[Channel(\d+)\]$/);
    if (matches === null) {
        return -1;
    } else {
        return matches[1];
    }
};

IonDiscoverDjPro.setLED = function(value, status) {
    if (status) {
        status = 0x64;
    } else {
        status = 0x00;
    }
    midi.sendShortMsg(0x90, value, status);
};

IonDiscoverDjPro.samplesPerBeat = function(group) {
    // FIXME: Get correct samplerate and channels for current deck
    var sampleRate = 44100;
    var channels = 2;
    var bpm = engine.getValue(group, "file_bpm");
    return channels * sampleRate * 60 / bpm;
};

IonDiscoverDjPro.selectKnob = function(channel, control, value, status, group) {
    if (value > 63) {
        value = value - 128;
    }
    if (IonDiscoverDjPro.directoryMode) {
        var i;
        if (value > 0) {
            for (i = 0; i < value; i++) {
                engine.setValue(group, "SelectNextPlaylist", 1);
            }
        } else {
            for (i = 0; i < -value; i++) {
                engine.setValue(group, "SelectPrevPlaylist", 1);
            }
        }
    } else {
        engine.setValue(group, "SelectTrackKnob", value);
    }
};

IonDiscoverDjPro.loopIn = function(channel, control, value, status, group) {
    var deck = IonDiscoverDjPro.groupToDeck(group);
    if (value) {
        if (IonDiscoverDjPro.manualLooping[deck]) {
            // Cut loop to Half
            var start = engine.getValue(group, "loop_start_position");
            var end = engine.getValue(group, "loop_end_position");
            if ((start !== -1) && (end !== -1)) {
                var len = (end - start) / 2;
                engine.setValue(group, "loop_end_position", start + len);
            }
        } else {
            if (engine.getValue(group, "loop_enabled")) {
                engine.setValue(group, "reloop_exit", 1);
                IonDiscoverDjPro.setLED(IonDiscoverDjPro.leds[deck]["reLoop"], false);
            }
            engine.setValue(group, "loop_in", 1);
            engine.setValue(group, "loop_end_position", -1);
            IonDiscoverDjPro.setLED(IonDiscoverDjPro.leds[deck]["loopIn"], true);
            IonDiscoverDjPro.setLED(IonDiscoverDjPro.leds[deck]["loopOut"], false);
        }
    }
};

IonDiscoverDjPro.loopOut = function(channel, control, value, status, group) {
    var deck = IonDiscoverDjPro.groupToDeck(group);
    if (value) {
        var start = engine.getValue(group, "loop_start_position");
        var end = engine.getValue(group, "loop_end_position");
        if (IonDiscoverDjPro.manualLooping[deck]) {
            // Set loop to current Bar (very approximative and would need to get fixed !!!)
            var bar = IonDiscoverDjPro.samplesPerBeat(group);
            engine.setValue(group, "loop_in", 1);
            engine.setValue(group, "loop_end_position", start + bar);
            IonDiscoverDjPro.setLED(IonDiscoverDjPro.leds[deck]["loopIn"], true);
            IonDiscoverDjPro.setLED(IonDiscoverDjPro.leds[deck]["loopOut"], true);
        } else {
            if (start === -1) {
                return;
            }
            if (end !== -1) {
                // Loop In and Out set -> call Reloop/Exit
                engine.setValue(group, "reloop_exit", 1);
                IonDiscoverDjPro.setLED(IonDiscoverDjPro.leds[deck]["loopIn"], true);
                IonDiscoverDjPro.setLED(IonDiscoverDjPro.leds[deck]["loopOut"], true);
                IonDiscoverDjPro.setLED(IonDiscoverDjPro.leds[deck]["reLoop"], true);
            } else {
                // Loop In set -> call Loop Out
                engine.setValue(group, "loop_out", 1);
                IonDiscoverDjPro.setLED(IonDiscoverDjPro.leds[deck]["loopOut"], true);
            }
        }
    }
};

IonDiscoverDjPro.reLoop = function(channel, control, value, status, group) {
    var deck = IonDiscoverDjPro.groupToDeck(group);
    if (value) {
        var start = engine.getValue(group, "loop_start_position");
        var end = engine.getValue(group, "loop_end_position");

        if (IonDiscoverDjPro.manualLooping[deck]) {
            // Multiply Loop by Two
            if ((start !== -1) && (end !== -1)) {
                var len = (end - start) * 2;
                engine.setValue(group, "loop_end_position", start + len);
            }
        } else {
            if (engine.getValue(group, "loop_enabled")) {
                IonDiscoverDjPro.setLED(IonDiscoverDjPro.leds[deck]["reLoop"], false);
            } else {
                if ((start !== -1) && (end !== -1)) {
                    // Loop is set ! Light the led
                    IonDiscoverDjPro.setLED(IonDiscoverDjPro.leds[deck]["reLoop"], true);
                }
            }
            engine.setValue(group, "reloop_exit", 1);
        }
    }
};

IonDiscoverDjPro.playFromCue = function(channel, control, value, status, group) {
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
};

IonDiscoverDjPro.jogWheel = function(channel, control, value, status, group) {
    var deck = IonDiscoverDjPro.groupToDeck(group);
    var adjustedJog = parseFloat(value);
    var posNeg = 1;
    if (adjustedJog > 63) {	// Counter-clockwise
        posNeg = -1;
        adjustedJog = value - 128;
    }

    if (IonDiscoverDjPro.scratchMode[deck]) {
        if (IonDiscoverDjPro.scratchTimer[deck] === -1) {
            engine.scratchEnable(deck, 128, 33+1/3, 1.0/8, (1.0/8)/32);
        } else {
            engine.stopTimer(IonDiscoverDjPro.scratchTimer[deck]);
        }
        engine.scratchTick(deck, adjustedJog);
        IonDiscoverDjPro.scratchTimer[deck] = engine.beginTimer(20, "IonDiscoverDjPro.jogWheelStopScratch(" + deck + ")", true);
    } else {
        var gammaInputRange = 23;	// Max jog speed
        var maxOutFraction = 0.5;	// Where on the curve it should peak; 0.5 is half-way
        var sensitivity = 0.5;		// Adjustment gamma
        var gammaOutputRange = 3;	// Max rate change
        if (engine.getValue(group, "play")) {
            adjustedJog = posNeg * gammaOutputRange * Math.pow(Math.abs(adjustedJog) / (gammaInputRange * maxOutFraction), sensitivity);
        } else {
            adjustedJog = gammaOutputRange * adjustedJog / (gammaInputRange * maxOutFraction);
        }
        engine.setValue(group, "jog", adjustedJog);
    }
};

IonDiscoverDjPro.jogWheelStopScratch = function(deck) {
    IonDiscoverDjPro.scratchTimer[deck] = -1;
    engine.scratchDisable(deck);
};

IonDiscoverDjPro.WheelTouch = function(channel, control, value, status, group) {
    var deck = IonDiscoverDjPro.groupToDeck(group);
    if (IonDiscoverDjPro.scratchMode[deck]) {
        if (value) {
            engine.scratchEnable(deck, 512, 33+1/3, 0.2, (0.2)/32);
            IonDiscoverDjPro.scratching[deck-1] = true;
        } else {
            engine.scratchDisable(deck);
            if (IonDiscoverDjPro.KeyIsLocked[deck-1]) {
                engine.setValur(group, "keylock", 1);
            }
            IonDiscoverDjPro.scratching[deck-1] = false;
        }
    }
};

IonDiscoverDjPro.toggleDirectoryMode = function(channel, control, value) {
    // Toggle setting and light
    if (value) {
        if (IonDiscoverDjPro.directoryMode) {
            IonDiscoverDjPro.directoryMode = false;
        } else {
            IonDiscoverDjPro.directoryMode = true;
        }
    }
};

IonDiscoverDjPro.toggleScratchMode = function(channel, control, value, status, group) {
    var deck = IonDiscoverDjPro.groupToDeck(group);
    // Toggle setting and light
    if (value) {
        if (IonDiscoverDjPro.scratchMode[deck]) {
            IonDiscoverDjPro.scratchMode[deck] = false;
        } else {
            IonDiscoverDjPro.scratchMode[deck] = true;
        }
        IonDiscoverDjPro.setLED(IonDiscoverDjPro.leds[deck]["scratchMode"], IonDiscoverDjPro.scratchMode[deck]);
    }
};

IonDiscoverDjPro.toggleManualLooping = function(channel, control, value, status, group) {
    var deck = IonDiscoverDjPro.groupToDeck(group);
    // Toggle setting and light
    if (value) {
        if (IonDiscoverDjPro.manualLooping[deck]) {
            IonDiscoverDjPro.manualLooping[deck] = false;
        } else {
            IonDiscoverDjPro.manualLooping[deck] = true;
        }
        IonDiscoverDjPro.setLED(IonDiscoverDjPro.leds[deck]["manualLoop"], IonDiscoverDjPro.manualLooping[deck]);
    }
};
