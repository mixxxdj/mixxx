// Korg KAOSS DJ controller mapping for Mixxx
// Seb Dooris, Fayaaz Ahmed, Lee Arromba, Raphael Quast

// Using `const`/`let` causes ReferenceError: KAOSSDJ is not defined
// eslint-disable-next-line no-var
var KAOSSDJ = {
    // shift button state variables
    shiftLeftPressed: false,
    shiftRightPressed: false,
    lastTapDate: Date.now(),
};

const MIDI_ON = 0x7F;
const MIDI_OFF = 0x00;

const MIDI_UP = 0x01;
const MIDI_DOWN = 0x7F;

const MIDI_CHANNELS = {
    BUTTON_LEFT: 0x97,
    BUTTON_RIGHT: 0x98,
    KNOB_MAIN: 0xB6,
    KNOB_LEFT: 0xB7,
    KNOB_RIGHT: 0xB8,
};

const MIDI_MAIN_KNOB_LEDS = {
    MONITOR_LEVEL: 0x14,
    MONITOR_MIX: 0x15,
    MASTER_LEVEL: 0x16,
};

const MIDI_DECK_BUTTON_LEDS = {
    // Fx LED (0x18) is owned by the controller
    MONITOR: 0x19,
    LOOP_L: 0x0F,
    LOOP_M: 0x10,
    LOOP_R: 0x11,
    HOTCUE_L: 0x12,
    HOTCUE_M: 0x13,
    HOTCUE_R: 0x14,
    STRIP_L: 0x15,
    STRIP_M: 0x16,
    STRIP_R: 0x17,
    CUE: 0x1E,
    SYNC: 0x1D,
    PLAY: 0x1B,
};

const MIDI_DECK_KNOB_LEDS = {
    GAIN: 0x1A,
    EQ_HI: 0x1B,
    EQ_MID: 0x1C,
    EQ_LO: 0x1D,
};

// initialize decks
KAOSSDJ.deck = function(deckNumber) {
    this.deckNumber = deckNumber;
    this.group = `[Channel${deckNumber}]`;
    this.jogWheelsInScratchMode = true;
    this.isFx = false;
};

KAOSSDJ.decks = [];
for (let i = 0; i < 4; i++) { // TODO: currently only 2 decks supported. is 4 possible?
    KAOSSDJ.decks[i] = new KAOSSDJ.deck(i+1);
}

// ==== lifecycle ====

KAOSSDJ.sendAllKnobLeds = function(state) {
    Object.values(MIDI_MAIN_KNOB_LEDS)
        .forEach(led => midi.sendShortMsg(MIDI_CHANNELS.KNOB_MAIN, led, state));
    [MIDI_CHANNELS.KNOB_LEFT, MIDI_CHANNELS.KNOB_RIGHT]
        .forEach(channel => Object.values(MIDI_DECK_KNOB_LEDS).forEach(
            led => midi.sendShortMsg(channel, led, state)));
};

KAOSSDJ.sendAllButtonLeds = function(state) {
    [MIDI_CHANNELS.BUTTON_LEFT, MIDI_CHANNELS.BUTTON_RIGHT]
        .forEach(channel => Object.values(MIDI_DECK_BUTTON_LEDS).forEach(
            led => midi.sendShortMsg(channel, led, state)));

};

KAOSSDJ.init = function(_id, _debugging) {
    // Turn on all knob LEDs and turn off all button LEDs.
    KAOSSDJ.sendAllKnobLeds(MIDI_ON);
    KAOSSDJ.sendAllButtonLeds(MIDI_OFF);

    // This message was copied from communication with Serato DJ Intro & Midi Monitor
    // It should setup mixxx from the controller defaults
    const ControllerStatusSysex = [0xF0, 0x42, 0x40, 0x00, 0x01, 0x28, 0x00, 0x1F, 0x70, 0x01, 0xF7];
    midi.sendSysexMsg(ControllerStatusSysex, ControllerStatusSysex.length);
};

KAOSSDJ.shutdown = function(_id, _debugging) {
    // Turn off all LEDs.
    KAOSSDJ.sendAllKnobLeds(MIDI_OFF);
    KAOSSDJ.sendAllButtonLeds(MIDI_OFF);
};

// ==== helper ====

KAOSSDJ.getDeckIndexFromChannel = function(channel) {
    return channel - 7;
};

KAOSSDJ.getDeckByChannel = function(channel) {
    const deckIndex = KAOSSDJ.getDeckIndexFromChannel(channel);
    return KAOSSDJ.decks[deckIndex];
};

KAOSSDJ.updateDeckByChannel = function(channel, key, value) {
    const deckIndex = KAOSSDJ.getDeckIndexFromChannel(channel);
    KAOSSDJ.decks[deckIndex][key] = value;
};

// ==== mapped functions ====

KAOSSDJ.wheelTouch = function(channel, _control, value, _status, _group) {
    const alpha = 1.0 / 8;
    const beta = alpha / 32;
    const deck = KAOSSDJ.getDeckByChannel(channel);
    const deckNumber = deck.deckNumber;
    const deckPlaying = engine.getValue(`[Channel${deckNumber}]`, "play_latched");

    // If in scratch mode or not playing enable vinyl-like control
    if (deck.jogWheelsInScratchMode || !deckPlaying) {
        if (value === MIDI_ON) {
            // Enable scratching on touch
            engine.scratchEnable(deckNumber, 128, 33 + 1 / 3, alpha, beta);
        } else if (value === MIDI_OFF) {
            // Disable scratching
            engine.scratchDisable(deckNumber);
        }
    }
};

KAOSSDJ.wheelTurn = function(channel, _control, value, _status, _group) {
    const deck = KAOSSDJ.getDeckByChannel(channel);
    const deckNumber = deck.deckNumber;
    let newValue = 0;
    if (value < 64) {
        newValue = value;
    } else {
        newValue = value - 128;
    }
    if (engine.isScratching(deckNumber)) {
        engine.scratchTick(deckNumber, newValue);
    } else {
        engine.setValue(`[Channel${deckNumber}]`, "jog", newValue); // Pitch bend
    }
};

KAOSSDJ.wheelTurnShift = function(channel, _control, value, _status, _group) {
    const deck = KAOSSDJ.getDeckByChannel(channel);
    if (deck.jogWheelsInScratchMode) {
        // Fast scratch
        let newValue = 0;
        if (value < 64) {
            newValue = value;
        } else {
            newValue = value - 128;
        }
        newValue = newValue * 4; // multiplier (to speed it up)
        engine.scratchTick(deck.deckNumber, newValue);
    } else {
        // Move beatgrid
        if (value === MIDI_UP) {
            engine.setValue(deck.group, "beats_translate_later", true);
        } else if (value === MIDI_DOWN) {
            engine.setValue(deck.group, "beats_translate_earlier", true);
        }
    }
};

KAOSSDJ.scratchMode = function(channel, _control, value, _status, _group) {
    KAOSSDJ.updateDeckByChannel(channel, "jogWheelsInScratchMode", value === MIDI_ON);
};

KAOSSDJ.fxToggleButton = function(channel, _control, value, _status, _group) {
    KAOSSDJ.updateDeckByChannel(channel, "isFx", value === MIDI_ON);
};

KAOSSDJ.fxKnob = function(_channel, _control, value, _status, _group) {
    if (KAOSSDJ.shiftLeftPressed) {
        // If Left Shift is pressed, cycle the effects chains on EffectUnit1
        if (value === MIDI_UP) {
            engine.setValue("[EffectRack1_EffectUnit1]", "next_chain", 1);
        } else if (value === MIDI_DOWN) {
            engine.setValue("[EffectRack1_EffectUnit1]", "prev_chain", 1);
        }
    } else if (KAOSSDJ.shiftRightPressed) {
        // If Right Shift is not pressed, cycle the effects chains on EffectUnit2
        if (value === MIDI_UP) {
            engine.setValue("[EffectRack1_EffectUnit2]", "next_chain", 1);
        } else if (value === MIDI_DOWN) {
            engine.setValue("[EffectRack1_EffectUnit2]", "prev_chain", 1);
        }
    } else {
        // If no shift is pressed, cycle through both QuickEffectRack filters
        if (value === MIDI_UP) {
            engine.setValue("[QuickEffectRack1_[Channel1]]", "next_chain", 1);
            engine.setValue("[QuickEffectRack1_[Channel2]]", "next_chain", 1);
        } else if (value === MIDI_DOWN) {
            engine.setValue("[QuickEffectRack1_[Channel1]]", "prev_chain", 1);
            engine.setValue("[QuickEffectRack1_[Channel2]]", "prev_chain", 1);
        }
    }
};

KAOSSDJ.fxTouchMoveVertical = function(_channel, _control, value, _status, _group) {
    Object.values(KAOSSDJ.decks)
        .filter(deck => deck.isFx)
        .forEach(deck => engine.setValue(`[EffectRack1_EffectUnit${deck.deckNumber}]`, "mix", value / 127));
};

KAOSSDJ.fxTouchMoveHorizontal = function(_channel, _control, value, _status, _group) {
    Object.values(KAOSSDJ.decks)
        .filter(deck => deck.isFx)
        .forEach(deck => engine.setValue(`[EffectRack1_EffectUnit${deck.deckNumber}]`, "super1", value / 127));
};

KAOSSDJ.fxTouch = function(channel, _control, value, _status, group) {
    if (value === MIDI_OFF) {
        engine.setValue(group, "mix", 0);
        engine.setValue(group, "super1", 0);
    }
};

// use loop-button to deactivate an active loop or initialize a beatloop at the current playback position
KAOSSDJ.toggleLoop = function(channel, _control, value, _status, _group) {
    const deck = KAOSSDJ.getDeckByChannel(channel);

    if (value === MIDI_ON) {
        if (engine.getValue(deck.group, "loop_enabled")) {
            engine.setValue(deck.group, "reloop_exit", true);
            engine.setValue(deck.group, "beatloop_activate", false);
        } else {
            engine.setValue(deck.group, "beatloop_activate", true);
        }
    }
};


// <LOAD A/B>           : load track
// <SHIFT> + <LOAD A/B> : Move library left/right
KAOSSDJ.loadCallback = function(channel, _control, value, _status, _group) {
    const deck = KAOSSDJ.getDeckByChannel(channel);
    if (value === MIDI_ON) {
        if (KAOSSDJ.shiftLeftPressed || KAOSSDJ.shiftRightPressed) {
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

KAOSSDJ.shiftLeftCallback = function(_channel, _control, value, _status, _group) {
    KAOSSDJ.shiftLeftPressed = value === MIDI_ON;
};

KAOSSDJ.shiftRightCallback = function(_channel, _control, value, _status, _group) {
    KAOSSDJ.shiftRightPressed = value === MIDI_ON;
};

KAOSSDJ.changeFocus = function(_channel, _control, value, _status, _group) {
    // toggle focus between Playlist and File-Browser
    if (value === MIDI_ON) {
        engine.setValue("[Library]", "MoveFocusForward", true);
    }
};

// <browseKnob>           : browse library up & down
// <SHIFT> + <browseKnob> : toggle focus between Playlist and File-Browser
KAOSSDJ.browseKnob = function(_channel, _control, value, _status, _group) {
    if (value > 0x40) {
        if (KAOSSDJ.shiftLeftPressed || KAOSSDJ.shiftRightPressed) {
            engine.setValue("[Library]", "MoveFocusForward", true);
        } else {
            engine.setValue("[Library]", "MoveUp", true);
        }
    } else {
        if (KAOSSDJ.shiftLeftPressed || KAOSSDJ.shiftRightPressed) {
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
KAOSSDJ.tapButtonCallback = function(_channel, _control, value, _status, _group) {
    if (value !== MIDI_ON) {
        return;
    }

    /* shift tab to move focus view */
    if ((value === MIDI_ON) && KAOSSDJ.shiftLeftPressed) {
        // engine.setValue("[Library]", "MoveFocusForward", true);
        engine.setValue("[Channel1]", "bpm_tap", true);
        return;
    }
    if ((value === MIDI_ON) && KAOSSDJ.shiftRightPressed) {
        // engine.setValue("[Library]", "MoveFocusForward", true);
        engine.setValue("[Channel2]", "bpm_tap", true);
        return;
    }

    /* tap to open folder, double-tap to close folder (twice to undo first tap)*/
    const now = new Date();
    const timeSinceLastTap = now - KAOSSDJ.lastTapDate;
    KAOSSDJ.lastTapDate = now;
    if ((timeSinceLastTap < 600) && (timeSinceLastTap > 0)) {
        engine.setValue("[Library]", "MoveLeft", true);
    } else {
        engine.setValue("[Library]", "MoveRight", true);
    }
};

// <SHIFT> + <TOUCHPAD X> : control super knob of QuickEffectRack for deck 1
KAOSSDJ.fxTouchMoveVerticalShift = function(_channel, _control, value, _status, _group) {
    const deck = KAOSSDJ.decks[0];
    if (deck.isFx) {
        const val = script.absoluteLin(value, 0, 1, 0, 127);
        engine.setValue(`[QuickEffectRack1_${deck.group}]`, "super1", val);
    }
};

// <SHIFT> + <TOUCHPAD Y> : control super knob of QuickEffectRack for deck 2
KAOSSDJ.fxTouchMoveHorizontalShift = function(_channel, _control, value, _status, _group) {
    const deck = KAOSSDJ.decks[1];
    if (deck.isFx) {
        const val = script.absoluteLin(value, 0, 1, 0, 127);
        engine.setValue(`[QuickEffectRack1_${deck.group}]`, "super1", val);
    }
};
