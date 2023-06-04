// Korg KAOSS DJ controller mapping for Mixxx
// Seb Dooris, Fayaaz Ahmed, Lee Arromba, Raphael Quast

var KAOSSDJ = {};
var ON = 0x7F,
    OFF = 0x00,
    UP = 0x01,
    DOWN = 0x7F;
var ledChannel = {
    "btnsL": 0x97,
    "btnsR": 0x98,
    "knobsL": 0xB7,
    "knobsR": 0xB8,
    "master": 0xB6
};
var led = {
    "cue": 0x1E,
    "sync": 0x1D,
    "play": 0x1B,
    "headphones": 0x19,
    "fx": 0x18, // warning: led is owned by controller
    "stripL": 0x15,
    "stripM": 0x16,
    "stripR": 0x17,
    "loopStripL": 0x0F,
    "loopStripM": 0x10,
    "loopStripR": 0x11,
};

// shift button state variables
var shiftLeftPressed = false;
var shiftRightPressed = false;

// initialise decks
KAOSSDJ.deck = function(deckNumber) {
    this.deckNumber = deckNumber;
    this.group = "[Channel" + deckNumber + "]";
    this.jogWheelsInScratchMode = true;
    this.fx = false;
};

KAOSSDJ.decks = [];
for (var i = 0; i < 4; i++) { // TODO: currently only 2 decks supported. is 4 possible?
    KAOSSDJ.decks[i] = new KAOSSDJ.deck(i+1);
}

// ==== lifecycle ====

KAOSSDJ.init = function(_id, _debugging) {
    // turn on main led channels
    var ledChannels = [
        ledChannel.knobsL,
        ledChannel.knobsR,
        ledChannel.master
    ];
    for (var led = 0x00; led <= 0xFF; led++) {
        for (var i = 0; i < ledChannels.length; i++) {
            midi.sendShortMsg(ledChannels[i], led, ON);
        }
    }

    // This message was copied from communication with Serato DJ Intro & Midi Monitor
    // It should setup mixxx from the controller defaults
    var ControllerStatusSysex = [0xF0, 0x42, 0x40, 0x00, 0x01, 0x28, 0x00, 0x1F, 0x70, 0x01, 0xF7];
    midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);
};

KAOSSDJ.shutdown = function(_id, _debugging) {
    // turn off all LEDs
    for (var led = 0x00; led <= 0xFF; led++) {
        for (var key in ledChannel) {
            midi.sendShortMsg(ledChannel[key], led, OFF);
        }
    }
};

// ==== helper ====

KAOSSDJ.getDeckIndexFromChannel = function(channel) {
    return channel - 7;
};

KAOSSDJ.getDeckByChannel = function(channel) {
    var deckIndex = KAOSSDJ.getDeckIndexFromChannel(channel);
    return KAOSSDJ.decks[deckIndex];
};

KAOSSDJ.updateDeckByChannel = function(channel, key, value) {
    var deckIndex = KAOSSDJ.getDeckIndexFromChannel(channel);
    KAOSSDJ.decks[deckIndex][key] = value;
};

// ==== mapped functions ====

KAOSSDJ.wheelTouch = function(channel, control, value, status, group) {
    var alpha = 1.0 / 8;
    var beta = alpha / 32;
    var deck = KAOSSDJ.getDeckByChannel(channel);
    var deckNumber = deck.deckNumber;
    var deckPlaying = engine.getValue("[Channel" + deckNumber + "]", "play_indicator");

    // If in scratch mode or not playing enable vinyl-like control
    if (deck.jogWheelsInScratchMode || !deckPlaying) {
        if (value === ON) {
            // Enable scratching on touch
            engine.scratchEnable(deckNumber, 128, 33 + 1 / 3, alpha, beta);
        } else if (value === OFF) {
            // Disable scratching
            engine.scratchDisable(deckNumber);
        }
    }
};

KAOSSDJ.wheelTurn = function(channel, control, value, status, group) {
    var deck = KAOSSDJ.getDeckByChannel(channel);
    var deckNumber = deck.deckNumber;
    var newValue = 0;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }
    if (engine.isScratching(deckNumber)) {
        engine.scratchTick(deckNumber, newValue);
    } else {
        engine.setValue("[Channel" + deckNumber + "]", "jog", newValue); // Pitch bend
    }
};

KAOSSDJ.wheelTurnShift = function(channel, control, value, status, group) {
    var deck = KAOSSDJ.getDeckByChannel(channel);
    if (deck.jogWheelsInScratchMode) {
        // Fast scratch
        var newValue = 0;
        if (value < 64) {
            newValue = value;
        } else {
            newValue = value - 128;
        }
        newValue = newValue * 4; // multiplier (to speed it up)
        engine.scratchTick(deck.deckNumber, newValue);
    } else {
        // Move beatgrid
        if (value === UP) {
            engine.setValue(deck.group, "beats_translate_later", true);
        } else if (value === DOWN) {
            engine.setValue(deck.group, "beats_translate_earlier", true);
        }
    }
};

KAOSSDJ.scratchMode = function(channel, control, value, status, group) {
    if (value === ON) {
        // Turn scratch mode on jogwheel (LED is red)
        KAOSSDJ.updateDeckByChannel(channel, "jogWheelsInScratchMode", true);
    } else if (value === OFF) {
        // Turn scratch mode on jogwheel (LED is off)
        KAOSSDJ.updateDeckByChannel(channel, "jogWheelsInScratchMode", false);
    }
};

KAOSSDJ.leftFxSwitch = function(channel, control, value, status, group) {
    if (value === ON) {
        KAOSSDJ.updateDeckByChannel(channel, "fx", true);
    } else {
        KAOSSDJ.updateDeckByChannel(channel, "fx", false);
    }
};

KAOSSDJ.rightFxSwitch = function(channel, control, value, status, group) {
    if (value === ON) {
        KAOSSDJ.updateDeckByChannel(channel, "fx", true);
    } else {
        KAOSSDJ.updateDeckByChannel(channel, "fx", false);
    }
};

KAOSSDJ.fxKnob = function(_channel, _control, _value, _status) {
    // TODO
};

KAOSSDJ.controllerFxTouchMoveVertical = function(channel, control, value, status, group) {
    var decks = KAOSSDJ.decks;
    for (var key in decks) {
        var deck = decks[key];
        if (deck.fx) {
            engine.setValue("[EffectRack1_EffectUnit"+deck.deckNumber +"]", "mix", value / 127);
        }
    }
};

KAOSSDJ.controllerFxTouchMoveHorizontal = function(channel, control, value, status, group) {
    var decks = KAOSSDJ.decks;
    for (var key in decks) {
        var deck = decks[key];
        if (deck.fx) {
            engine.setValue("[EffectRack1_EffectUnit"+deck.deckNumber +"]", "super1", value / 127);
        }
    }
};

KAOSSDJ.controllerFxTouchUp = function(channel, control, value, status, group) {
    var deck = KAOSSDJ.getDeckByChannel(channel);
    engine.setValue("[EffectRack1_EffectUnit"+deck.deckNumber +"]", "mix", 0);
    engine.setValue("[EffectRack1_EffectUnit"+deck.deckNumber +"]", "super1", 0);
};

// use loop-button to deactivate an active loop or initialize a beatloop at the current playback position
KAOSSDJ.toggleLoop = function(channel, control, value, status, group) {
    var deck = KAOSSDJ.getDeckByChannel(channel);
    var loopEnabled = engine.getValue(deck.group, "loop_enabled");

    if (value === ON) {
        if (loopEnabled) {
            engine.setValue(deck.group, "reloop_exit", true);
            engine.setValue(deck.group, "beatloop_activate", false);
        } else {
            engine.setValue(deck.group, "beatloop_activate", true);
        }
    }
};


// <LOAD A/B>           : load track
// <SHIFT> + <LOAD A/B> : open/close folder in file-browser
KAOSSDJ.loadCallback = function(channel, control, value, status, group) {
    var deck = KAOSSDJ.getDeckByChannel(channel);
    if (value === ON) {
        if ((shiftLeftPressed) || (shiftRightPressed)) {
            if (deck.deckNumber === 1) {
                engine.setValue("[Library]", "MoveLeft", true);
            } else {
                engine.setValue("[Library]", "MoveRight", true);
            }
        } else {
            engine.setValue(deck.group, "LoadSelectedTrack", true);
        }
    }
};

KAOSSDJ.shiftLeftCallback = function(channel, control, value, status, group) {
    if (value === ON) {
        // if (shiftRightPressed == true) {
        // 	TODO add functionality if <RIGHT SHIFT> is active while <LEFT SHIFT> is pressed?
        // }
        shiftLeftPressed = true;
    } else { shiftLeftPressed = false; }
};

KAOSSDJ.shiftRightCallback = function(channel, control, value, status, group) {
    if (value === ON) {
        // if (shiftLeftPressed == true) {
        // 	TODO add functionality if <LEFT SHIFT> is active while <RIGHT SHIFT> is pressed?
        // }
        shiftRightPressed = true;
    } else { shiftRightPressed = false; }
};

KAOSSDJ.changeFocus = function(channel, control, value, status, group) {
    if (value === ON) {
        // toggle focus between Playlist and File-Browser
        engine.setValue("[Library]", "MoveFocusForward", true);
    }
};

// <browseKnob>           : browse library up & down
// <SHIFT> + <browseKnob> : toggle focus between Playlist and File-Browser
KAOSSDJ.browseKnob = function(channel, control, value, status, group) {
    if (value > 0x40) {
        if ((shiftLeftPressed) || (shiftRightPressed)) {
            engine.setValue("[Library]", "MoveFocusForward", true);
        } else {
            engine.setValue("[Library]", "MoveUp", true);
        }
    } else {
        if ((shiftLeftPressed) || (shiftRightPressed)) {
            engine.setValue("[Library]", "MoveFocusBackward", true);
        } else {
            engine.setValue("[Library]", "MoveDown", true);
        }
    }
};

// <TAP>                : open folder
// <TAP> + <TAP>        : double-tap to close folder
// <SHIFT LEFT> + <TAP> : tap bpm of LEFT track
// <SHIFT RIGHT> + <TAP> : tap bpm of RIGHT track
var doubleTapTime;
KAOSSDJ.tapButtonCallback = function(channel, control, value, status, group) {
    var now = new Date().getTime();
    var timesince = now - doubleTapTime;

    /* shift tab to move focus view */
    if ((value === ON) && ((shiftLeftPressed))) {
        // engine.setValue("[Library]", "MoveFocusForward", true);
        engine.setValue("[Channel1]", "bpm_tap", true);
        return;
    }
    if ((value === ON) && ((shiftRightPressed))) {
        // engine.setValue("[Library]", "MoveFocusForward", true);
        engine.setValue("[Channel2]", "bpm_tap", true);
        return;
    }

    /* tap to open folder, double-tap to close folder (twice to undo first tap)*/
    if (value === ON) {
        if ((timesince < 600) && (timesince > 0)) {
            engine.setValue("[Library]", "MoveLeft", true);
            engine.setValue("[Library]", "MoveLeft", true);
        } else {
            engine.setValue("[Library]", "MoveRight", true);
        }

        doubleTapTime = new Date().getTime();
    }
};

// <SHIFT> + <TOUCHPAD X> : control super knob of QuickEffectRack for deck 1
KAOSSDJ.controllerFxTouchMoveVerticalShift = function(channel, control, value, status, group) {
    var deck = KAOSSDJ.decks[0];
    if (deck.fx) {
        var val = script.absoluteLin(value, 0, 1, 0, 127);
        engine.setValue("[QuickEffectRack1_" + deck.group + "]", "super1", val);
    }
};

// <SHIFT> + <TOUCHPAD Y> : control super knob of QuickEffectRack for deck 2
KAOSSDJ.controllerFxTouchMoveHorizontalShift = function(channel, control, value, status, group) {
    var deck = KAOSSDJ.decks[1];
    if (deck.fx) {
        var val = script.absoluteLin(value, 0, 1, 0, 127);
        engine.setValue("[QuickEffectRack1_" + deck.group + "]", "super1", val);
    }
};
