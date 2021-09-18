// Korg KAOSS DJ controller mapping for Mixxx
// Seb Dooris, Fayaaz Ahmed, Lee Arromba, Raphael Quast

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

// shift button state variables
var shift_left_pressed = false
var shift_right_pressed = false

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

// use loop-button to deactivate an active loop or initialize a beatloop at the current playback position
KAOSSDJ.toggle_loop = function(channel, control, value, status, group) {
	var deck = KAOSSDJ.getDeckByChannel(channel);
	var loop_enabled = engine.getValue(deck.group, "loop_enabled");

	if(value === ON) {
		if(loop_enabled) {
			engine.setValue(deck.group, "reloop_exit", true)
			engine.setValue(deck.group, "beatloop_activate", false)
		} else {
			engine.setValue(deck.group, "beatloop_activate", true)
		}
	}
};


// <LOAD A/B>           : load track
// <SHIFT> + <LOAD A/B> : open/close folder in file-browser
KAOSSDJ.load_callback = function(channel, control, value, status, group) {
	var deck = KAOSSDJ.getDeckByChannel(channel);
	if(value === ON) {
        if ((shift_left_pressed) || (shift_right_pressed)) {
			if (deck.deckNumber == 1) {
				engine.setValue("[Library]", "MoveLeft", true)
			} else {
				engine.setValue("[Library]", "MoveRight", true)
			}
		} else {
			engine.setValue(deck.group, "LoadSelectedTrack", true)
		}
	}
};

KAOSSDJ.shift_left_callback = function(channel, control, value, status, group) {
	if(value === ON) {
		// if (shift_right_pressed == true) {
		// 	TODO add functionality if <RIGHT SHIFT> is active while <LEFT SHIFT> is pressed?
		// }
		shift_left_pressed = true
	} else
		shift_left_pressed = false
};

KAOSSDJ.shift_right_callback = function(channel, control, value, status, group) {
	if(value === ON) {
		// if (shift_left_pressed == true) {
		// 	TODO add functionality if <LEFT SHIFT> is active while <RIGHT SHIFT> is pressed?
		// }
        shift_right_pressed = true
	} else
		shift_right_pressed = false
};

KAOSSDJ.change_focus = function(channel, control, value, status, group) {
	if(value === ON) {
        // toggle focus between Playlist and File-Browser
        engine.setValue("[Library]", "MoveFocusForward", true);
	}
};

// <browseKnob>           : browse library up & down
// <SHIFT> + <browseKnob> : toggle focus between Playlist and File-Browser
KAOSSDJ.browseKnob = function(channel, control, value, status, group) {
    var nval = (value > 0x40 ? value - 0x80 : value);
	if (value > 0x40) {
		if ((shift_left_pressed) || (shift_right_pressed)) {
			engine.setValue("[Library]", "MoveFocusForward", true);
		} else {
			engine.setValue("[Library]", "MoveUp", true);
		}
	} else {
		if ((shift_left_pressed) || (shift_right_pressed)) {
			engine.setValue("[Library]", "MoveFocusBackward", true);
		} else {
			engine.setValue("[Library]", "MoveDown", true);
		}
	};
};

// <TAP>                : open folder
// <TAP> + <TAP>        : double-tap to close folder
// <SHIFT LEFT> + <TAP> : tap bpm of LEFT track
// <SHIFT RIGHT> + <TAP> : tap bpm of RIGHT track
var double_tap_time;
KAOSSDJ.tap_button_callback = function(channel, control, value, status, group) {
    var now = new Date().getTime();
    var timesince = now - double_tap_time;

	/* shift tab to move focus view */
	if ((value === ON) && ((shift_left_pressed))) {
		// engine.setValue("[Library]", "MoveFocusForward", true);
		engine.setValue("[Channel1]", "bpm_tap", true);
		return
	}
	if ((value === ON) && ((shift_right_pressed))) {
		// engine.setValue("[Library]", "MoveFocusForward", true);
		engine.setValue("[Channel2]", "bpm_tap", true);
		return
	}

	/* tap to open folder, double-tap to close folder (twice to undo first tap)*/
	if (value === ON) {
		if((timesince < 600) && (timesince > 0)){
			engine.setValue("[Library]", "MoveLeft", true);
			engine.setValue("[Library]", "MoveLeft", true);
		} else {
			engine.setValue("[Library]", "MoveRight", true);
		};

	   double_tap_time = new Date().getTime();
	};
};

