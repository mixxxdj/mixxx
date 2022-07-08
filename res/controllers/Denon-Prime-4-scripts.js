// eslint-disable-next-line-no-var
var Prime4 = {};

/*
 * DENON DJ PRIME 4 - CONTROLLER MAPPING FOR MIXXX
 *
 * Thank you for using this mapping! Below are a few parameters
 * you can change if you want to customize anything :)
 */

// What colour would you like each deck to be?
// Choose between "red", "yellow", "green", "cyan", "blue", "magenta", or "white".
const colourOfDeck1 = "green";
const colourOfDeck2 = "blue";
const colourOfDeck3 = "red";
const colourOfDeck4 = "yellow";

/*
 *                   WARNING!!!
 *
 *      DO NOT EDIT ANYTHING PAST THIS POINT
 *       UNLESS YOU KNOW WHAT YOU'RE DOING.
 */

Prime4.rgbCode = {
    black: 0,
    blueDark: 1,
    blueDim: 2,
    blue: 3,
    greenDark: 4,
    cyanDark: 5,
    greenDim: 8,
    cyanDim: 10,
    green: 12,
    cyan: 15,
    redDark: 16,
    magentaDark: 17,
    yellowDark: 20,
    whiteDark: 21,
    redDim: 32,
    magentaDim: 34,
    yellowDim: 40,
    whiteDim: 42,
    red: 48,
    magenta: 51,
    orange: 56,
    yellow: 60,
    white: 63,
};

// Map RGB Hex values to MIDI values for Prime 4 LEDs
var Prime4ColorMapper = new ColorMapper({
	0x000055: 0x01,
	0x0000AA: 0x02,
	0x0000FF: 0x03,
	0x005500: 0x04,
	0x005555: 0x05,
	0x0055AA: 0x06,
	0x0055FF: 0x07,
	0x00AA00: 0x08,
	0x00AA55: 0x09,
	0x00AAAA: 0x0A,
	0x00AAFF: 0x0B,
	0x00FF00: 0x0C,
	0x00FF55: 0x0D,
	0x00FFAA: 0x0E,
	0x00FFFF: 0x0F,
	0x550000: 0x10, 
	0x550055: 0x11,
	0x5500AA: 0x12,
	0x5500FF: 0x13,
	0x555500: 0x14,
	0x555555: 0x15,
	0x5555AA: 0x16,
	0x5555FF: 0x17,
	0x55AA00: 0x18,
	0x55AA55: 0x19,
	0x55AAAA: 0x1A,
	0x55AAFF: 0x1B,
	0x55FF00: 0x1C,
	0x55FF55: 0x1D,
	0x55FFAA: 0x1E,
	0x55FFFF: 0x1F,
	0xAA0000: 0x20,
	0xAA0055: 0x21,
	0xAA00AA: 0x22,
	0xAA00FF: 0x23,
	0xAA5500: 0x24,
	0xAA5555: 0x25,
	0xAA55AA: 0x26,
	0xAA55FF: 0x27,
	0xAAAA00: 0x28,
	0xAAAA55: 0x29,
	0xAAAAAA: 0x2A,
	0xAAAAFF: 0x2B,
	0xAAFF00: 0x2C,
	0xAAFF55: 0x2D,
	0xAAFFAA: 0x2E,
	0xAAFFFF: 0x2F,
	0xFF0000: 0x30,
	0xFF0055: 0x31,
	0xFF00AA: 0x32,
	0xFF00FF: 0x33,
	0xFF5500: 0x34,
	0xFF5555: 0x35,
	0xFF55AA: 0x36,
	0xFF55FF: 0x37,
	0xFFAA00: 0x38,
	0xFFAA55: 0x39,
	0xFFAAAA: 0x3A,
	0xFFAAFF: 0x3B,
	0xFFFF00: 0x3C,
	0xFFFF55: 0x3D,
	0xFFFFAA: 0x3E,
	0xFFFFFF: 0x3F,
});

const colDeck1 = eval("Prime4.rgbCode." + colourOfDeck1);
const colDeck2 = eval("Prime4.rgbCode." + colourOfDeck2);
const colDeck3 = eval("Prime4.rgbCode." + colourOfDeck3);
const colDeck4 = eval("Prime4.rgbCode." + colourOfDeck4);
const colDeck1Dark = eval("Prime4.rgbCode." + colourOfDeck1 + "Dark");
const colDeck2Dark = eval("Prime4.rgbCode." + colourOfDeck2 + "Dark");
const colDeck3Dark = eval("Prime4.rgbCode." + colourOfDeck3 + "Dark");
const colDeck4Dark = eval("Prime4.rgbCode." + colourOfDeck4 + "Dark");

Prime4.init = function() {
    midi.sendShortMsg(0x90, 0x75, 0x00); // Turn off all LEDs
    Prime4.leftDeck = new Prime4.Deck([1, 3], 4);
    Prime4.rightDeck = new Prime4.Deck([2, 4], 5);
    components.Button.prototype.isPress = function(channel, control, value, status) {
        return (status & 0xF0) === 0x90;
    };
    midi.sendShortMsg(0x90, 0x0D, colDeck1Dark); // PFL 1
    midi.sendShortMsg(0x91, 0x0D, colDeck2Dark); // PFL 2
    midi.sendShortMsg(0x92, 0x0D, colDeck3Dark); // PFL 3
    midi.sendShortMsg(0x93, 0x0D, colDeck4Dark); // PFL 4
    midi.sendShortMsg(0x9F, 0x1C, colDeck1); // Deck 1 Toggle
    midi.sendShortMsg(0x9F, 0x1D, colDeck2); // Deck 2 Toggle
    midi.sendShortMsg(0x9F, 0x1E, colDeck3Dark); // Deck 3 Toggle
    midi.sendShortMsg(0x9F, 0x1F, colDeck4Dark); // Deck 4 Toggle
    midi.sendShortMsg(0x94, 0x21, colDeck1); // Left Jog Wheel
    midi.sendShortMsg(0x95, 0x21, colDeck2); // Right Jog Wheel
    midi.sendShortMsg(0x94, 0x0B, colDeck1); // Hot-Cue Button - Left Deck
    midi.sendShortMsg(0x95, 0x0B, colDeck2); // Hot-Cue Button - Right Deck
};

Prime4.shutdown = function() {
    midi.sendShortMsg(0x90, 0x75, 0x01); // Return all LEDs to initial dim state
};

Prime4.Deck = function(deckNumbers, midiChannel) {

    components.Deck.call(this, deckNumbers);

    this.censorButton = new components.Button({
        midi: [0x90 + midiChannel, 0x01],
        key: "reverseroll",
        on: 0x7f,
        off: 0x01,
    });

    // Beatjump Backward
    this.bjumpBackButton = new components.Button({
        midi: [0x90 + midiChannel, 0x06],
        key: "beatjump_backward",
        on: 0x7f,
        off: 0x01,
    });

    // Beatjump Forward
    this.bjumpFwdButton = new components.Button({
        midi: [0x90 + midiChannel, 0x07],
        key: "beatjump_forward",
        on: 0x7f,
        off: 0x01,
    });

    this.syncButton = new components.SyncButton({
        midi: [0x90 + midiChannel, 0x08],
        on: 0x7f,
        off: 0x01,
    });

    this.cueButton = new components.CueButton({
        midi: [0x90 + midiChannel, 0x09],
        on: 0x7f,
        off: 0x01,
    });

    this.playButton = new components.PlayButton({
        midi: [0x90 + midiChannel, 0x0A],
        on: 0x7f,
        off: 0x01,
		unshift: function() {
			components.PlayButton.prototype.unshift.call(this);
			this.type = components.Button.prototype.types.toggle;
		},
		shift: function() {
			this.inKey = "play_stutter";
			this.type = components.Button.prototype.types.push;
		}
    });

	this.hotcuePad = [];
	for (var i = 1; i <= 8; i++) {
		var hotcueColor = engine.getValue(this.currentDeck, "hotcue_" + i + "_color");
		this.hotcuePad[i] = new components.HotcueButton({
			number: i,
			midi: [0x90 + midiChannel, 0x0E + i],
			colorMapper: Prime4ColorMapper,
			off: 0x00,
		});
	};

    this.tempoFader = new components.Pot({
        midi: [0xB0 + midiChannel, 0x1F],
        inKey: "rate",
        invert: true,
    });

	// LED indicator when pitch fader is at centre. slightly buggy when switching decks.
	this.tempoFeedback = function(channel, control, value, status, group) {
		var currentRate = engine.getParameter(this.currentDeck, "rate");
		print(currentRate)
		print(this.currentDeck)
		if (currentRate >= 0.49 && currentRate <= 0.51) {
			midi.sendShortMsg(0x90 + midiChannel, 0x34, 0x7F);
		}
		else {
			midi.sendShortMsg(0x90 + midiChannel, 0x34, 0x00);
		}
	};

    this.keylockButton = new components.Button({
        midi: [0x90 + midiChannel, 0x22],
        key: "keylock",
        on: 0x7f,
        off: 0x01,
        type: components.Button.prototype.types.toggle,
    });

    /*
	this.vinylButton = new components.Button({
		midi: [0x90 + midiChannel, 0x23],
		//TODO: Jog Wheel Functionality First
		on: 0x7f,
		off: 0x01,
		type: components.Button.prototype.types.toggle,
	});
	*/

    this.slipButton = new components.Button({
        midi: [0x90 + midiChannel, 0x24],
        key: "slip_enabled",
        on: 0x7f,
        off: 0x01,
        type: components.Button.prototype.types.toggle,
    });

	this.loopInButton = new components.Button({
		midi: [0x90 + midiChannel, 0x25],
		key: "loop_in",
		on: 0x7f,
		off: 0x01,
	});

	this.loopOutButton = new components.Button({
		midi: [0x90 + midiChannel, 0x26],
		key: "loop_out",
		on: 0x7f,
		off: 0x01,
	});

    // Change from Deck 3/4 to Deck 1/2
    this.deckToggleButtonA = function(value) {
        if (value > 0) {
            const channelCheck = midiChannel - 1;
            if (this.currentDeck === "[Channel" + channelCheck + "]") {
                this.toggle();
                midi.sendShortMsg(0x90 + midiChannel, 0x21, eval("colDeck" + (midiChannel - 3))); // Jog Wheel LED
                midi.sendShortMsg(0x9F, 0x18 + midiChannel, eval("colDeck" + (midiChannel - 3))); // Active deck - bright LED
                midi.sendShortMsg(0x9F, 0x1A + midiChannel, eval("colDeck" + (midiChannel - 1) + "Dark")); // Inactive deck - dark LED
    			midi.sendShortMsg(0x90 + midiChannel, 0x0B, eval("colDeck" + (midiChannel - 3))); // Hot-Cue Button
            }
        }
    };

    // Change from Deck 1/2 to Deck 3/4
    this.deckToggleButtonB = function(value) {
        if (value > 0) {
            const channelCheck = midiChannel - 3;
            if (this.currentDeck === "[Channel" + channelCheck + "]") {
                this.toggle();
                midi.sendShortMsg(0x90 + midiChannel, 0x21, eval("colDeck" + (midiChannel - 1))); // Jog Wheel LED
                midi.sendShortMsg(0x9F, 0x18 + midiChannel, eval("colDeck" + (midiChannel - 3) + "Dark")); // Inactive deck - dark LED
                midi.sendShortMsg(0x9F, 0x1A + midiChannel, eval("colDeck" + (midiChannel - 1))); // Active deck - bright LED
    			midi.sendShortMsg(0x90 + midiChannel, 0x0B, eval("colDeck" + (midiChannel - 1))); // Hot-Cue Button
            }
        }
    };

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = this.currentDeck;
        }
    });
};


Prime4.Deck.prototype = new components.Deck();

// Headphone Cue / PFL buttons for Decks 1 and 2
var headphoneCue = [];
for (var i = 1; i <= 4; i++) {
    headphoneCue[i] = new components.Button({
        midi: [0x8F + i, 0x0D],
        group: "[Channel" + i + "]",
        key: "pfl",
        on: eval("colDeck" + i),
        off: eval("colDeck" + i + "Dark"),
        type: components.Button.prototype.types.toggle,
    });
}

// View Button
Prime4.maxView = new components.Button({
    midi: [0x9F, 0x07],
    group: "[Master]",
    key: "maximize_library",
    on: 0x7F,
    off: 0x01,
    type: components.Button.prototype.types.toggle,
});

Prime4.shift = false;
Prime4.shiftState = function (channel, control, value, status, group) {
	Prime4.shift = value === 0x7F;

	if (Prime4.shift) {
		Prime4.leftDeck.shift();
		Prime4.rightDeck.shift();
	}
	else {
		Prime4.leftDeck.unshift();
		Prime4.rightDeck.unshift();
	}
}

// COMPONENTS TO IMPLEMENT:
//
// MIXER SECTION
// Crossfader assignment switches (Deck 1, Deck 2, Deck 3, Deck 4)
// Sweep FX buttons (FILTER, ECHO, WASH, NOISE)
// Navigation buttons (BACK, FWD, LOAD (Deck 1, Deck 2))
// Encoder knob click
// VIEW button (Library fullscreen?)
//
// DECK 1 SECTION
// SKIP buttons
// LOOP encoder
// VINYL button
// PITCH BEND buttons
// BEAT GRID buttons
// PARAMETER buttons
// Jog wheel
// HOT CUE mode
// LOOP mode
// AUTOLOOP mode
// ROLL mode
// SLICER mode
// SLICER LOOP mode
// Performance pads (1, 2, 3, 4, 5, 6, 7, 8)
//
//
// FX SELECT knob
// FX PARAMETER knob
// FX FREQUENCY knob
// FX WET/DRY knob
// ON button
// PARAM button
// RESET button
// BEATS buttons (<, >)
//
// NOTE: For now, FX buttons currently enable FX 1, 2 and 3 from effect units 1 and 2.
