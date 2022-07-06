// eslint-disable-next-line-no-var
var Prime4 = {};

Prime4.init = function() {
    midi.sendShortMsg(0x90, 0x75, 0x00); // Turn off all LEDs
    Prime4.leftDeck = new Prime4.Deck([1, 3], 4);
    Prime4.rightDeck = new Prime4.Deck([2, 4], 5);
    components.Button.prototype.isPress = function(channel, control, value, status) {
        return (status & 0xF0) === 0x90;
    };
    midi.sendShortMsg(0x9F, 0x1C, 0x7F);
    midi.sendShortMsg(0x9F, 0x1D, 0x7F);
    midi.sendShortMsg(0x9F, 0x1E, 0x05);
    midi.sendShortMsg(0x9F, 0x1F, 0x05);
};

Prime4.shutdown = function() {
    midi.sendShortMsg(0x90, 0x75, 0x01); // Return all LEDs to initial dim state
};

Prime4.Deck = function(deckNumbers, midiChannel) {

    components.Deck.call(this, deckNumbers);

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

    this.censorButton = new components.Button({
        midi: [0x90 + midiChannel, 0x01],
        key: "reverseroll",
        on: 0x7f,
        off: 0x01,
    });

    this.bjumpBackButton = new components.Button({
        midi: [0x90 + midiChannel, 0x06],
        key: "beatjump_backward",
        on: 0x7f,
        off: 0x01,
    });

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

    // change from Deck 3/4 to Deck 1/2
    this.deckToggleButtonA = function(value) {
        if (value > 0) {
            const channelCheck = midiChannel - 1;
            if (this.currentDeck === "[Channel" + channelCheck + "]") {
                this.toggle();
                midi.sendShortMsg(0x9F, 0x18 + midiChannel, 0x7F);
                midi.sendShortMsg(0x9F, 0x1A + midiChannel, 0x05);
            }
        }
    };
    // change from Deck 1/2 to Deck 3/4
    this.deckToggleButtonB = function(value) {
        if (value > 0) {
            const channelCheck = midiChannel - 3;
            if (this.currentDeck === "[Channel" + channelCheck + "]") {
                this.toggle();
                midi.sendShortMsg(0x9F, 0x18 + midiChannel, 0x15);
                midi.sendShortMsg(0x9F, 0x1A + midiChannel, 0x0F);
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
// Deck 1-3 toggle buttons
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
