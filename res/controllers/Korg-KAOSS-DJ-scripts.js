// Korg KAOSS DJ controller mapping for Mixxx
// Seb Dooris, Fayaaz Ahmed, Lee Arromba

var KAOSSDJ = {};
var ON = 0x7F,
    OFF = 0x00,
    UP = 0x01,
    DOWN = 0x7F;
var ledChannel = {
    'btnsL': 0x97,
    'btnsR': 0x98,
    'knobsL': 0xB7,
    'knobsR': 0xB8,
    'master': 0xB6
};
var led = {
    'cue': 0x1E,
    'sync': 0x1D,
    'play': 0x1B,
    'headphones': 0x19,
    'fx': 0x18, // warning: led is owned by controller
    'stripL': 0x15,
    'stripM': 0x16,
    'stripR': 0x17,
    'loopStripL': 0x0F,
    'loopStripM': 0x10,
    'loopStripR': 0x11,
};

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

KAOSSDJ.init = function(id, debugging) {
    // turn on main led channels
    var ledChannels = [
        ledChannel['knobsL'],
        ledChannel['knobsR'],
        ledChannel['master']
    ]
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

KAOSSDJ.shutdown = function(id, debugging) {
    // turn off all LEDs
    for (var led = 0x00; led <= 0xFF; led++) {
        for (key in ledChannel) {
            midi.sendShortMsg(ledChannel[key], led, OFF);
        }
    }
};

// ==== helper ====

KAOSSDJ.getDeckIndexFromChannel = function(channel) {
    return channel - 7
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
    var deck_playing = engine.getValue('[Channel' + deckNumber + ']', 'play_indicator');

    // If in scratch mode or not playing enable vinyl-like control
    if (deck.jogWheelsInScratchMode || !deck_playing ) {
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
        engine.setValue('[Channel' + deckNumber + ']', 'jog', newValue); // Pitch bend
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
        if(value === UP){
            engine.setValue(deck.group, 'beats_translate_later', true);
        } else if (value === DOWN){
            engine.setValue(deck.group, 'beats_translate_earlier', true);
        }
    }
};

KAOSSDJ.scratchMode = function(channel, control, value, status, group) {
    if (value === ON) {
        // Turn scratch mode on jogwheel (LED is red)
        KAOSSDJ.updateDeckByChannel(channel, 'jogWheelsInScratchMode', true);
    } else if (value === OFF) {
        // Turn scratch mode on jogwheel (LED is off)
        KAOSSDJ.updateDeckByChannel(channel, 'jogWheelsInScratchMode', false);
    }
};

KAOSSDJ.browseKnob = function(channel, control, value, status, group) {
    var nval = (value > 0x40 ? value - 0x80 : value);
    engine.setValue(group, "SelectTrackKnob", nval);
};

KAOSSDJ.leftFxSwitch = function(channel, control, value, status, group) {
    var deck = KAOSSDJ.getDeckByChannel(channel);
    if(value === ON) {
        KAOSSDJ.updateDeckByChannel(channel, 'fx', true);
    } else {
        KAOSSDJ.updateDeckByChannel(channel, 'fx', false);
    }
};

KAOSSDJ.rightFxSwitch = function(channel, control, value, status, group) {
    if(value === ON) {
        KAOSSDJ.updateDeckByChannel(channel, 'fx', true);
    } else {
        KAOSSDJ.updateDeckByChannel(channel, 'fx', false);
    }
};

KAOSSDJ.controllerFxTouchMoveVertical = function(channel, control, value, status, group) {
    var decks = KAOSSDJ.decks;
    for(key in decks) {
        var deck = decks[key];
        if(deck.fx) {
            engine.setValue('[EffectRack1_EffectUnit'+deck.deckNumber +']', 'mix', value / 127);
        }
    }
};

KAOSSDJ.controllerFxTouchMoveHorizontal = function(channel, control, value, status, group) {
    var decks = KAOSSDJ.decks;
    for(key in decks) {
        var deck = decks[key];
        if(deck.fx) {
            engine.setValue('[EffectRack1_EffectUnit'+deck.deckNumber +']', 'super1', value / 127);
        }
    }
};

KAOSSDJ.controllerFxTouchUp = function(channel, control, value, status, group) {
    var deck = KAOSSDJ.getDeckByChannel(channel);
    engine.setValue('[EffectRack1_EffectUnit'+deck.deckNumber +']', 'mix', 0);
    engine.setValue('[EffectRack1_EffectUnit'+deck.deckNumber +']', 'super1', 0);
};
