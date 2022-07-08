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
const colourOfDeck1 = "white";
const colourOfDeck2 = "yellow";
const colourOfDeck3 = "cyan";
const colourOfDeck4 = "magenta";

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
    midi.sendShortMsg(0x9F, 0x07, 0x01); // View Button
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
    });

    this.tempoFader = new components.Pot({
        midi: [0xB0 + midiChannel, 0x1F],
        inKey: "rate",
        invert: true,
    });

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

    // Change from Deck 3/4 to Deck 1/2
    this.deckToggleButtonA = function(value) {
        if (value > 0) {
            const channelCheck = midiChannel - 1;
            if (this.currentDeck === "[Channel" + channelCheck + "]") {
                this.toggle();
                midi.sendShortMsg(0x90 + midiChannel, 0x21, eval("colDeck" + (midiChannel - 3))); // Jog Wheel LED
                midi.sendShortMsg(0x9F, 0x18 + midiChannel, eval("colDeck" + (midiChannel - 3))); // Active deck - bright LED
                midi.sendShortMsg(0x9F, 0x1A + midiChannel, eval("colDeck" + (midiChannel - 1) + "Dark")); // Inactive deck - dark LED
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

// eslint-disable-next-line-no-var
var headphoneCue = [];

// eslint-disable-next-line-no-var
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
// LOOP buttons
// VINYL button
// Pitch slider
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
