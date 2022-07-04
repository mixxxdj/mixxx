// eslint-disable-next-line-no-var
var Prime4 = {};

Prime4.init = function() {
    Prime4.leftDeck = new Prime4.Deck([1, 3], 1);
    Prime4.rightDeck = new Prime4.Deck([2, 4], 2);
    components.Button.prototype.isPress = function(channel, control, value, status) {
        return (status & 0xF0) === 0x90;
    };
    // TODO: Add initialize sequence
};

Prime4.shutdown = function() {
    // TODO: Add shutdown sequence
};

Prime4.Deck = function(deckNumbers, midiChannel) {

    components.Deck.call(this, deckNumbers);

    this.slipButton = new components.Button({
        midi: [0x90 + midiChannel, 0x24],
        inKey: "slip_enabled",
        outKey: "slip_enabled",
        //type: Button.prototype.types.toggle,
    });

    this.censorButton = new components.Button({
        midi: [0x90 + midiChannel, 0x01],
        key: "reverseroll",
    });

    this.syncButton = new components.SyncButton([0x90 + midiChannel, 0x08]);

    this.cueButton = new components.CueButton({
        midi: [0x90 + midiChannel, 0x09],
        off: 0x01,
    });

    this.playButton = new components.PlayButton({
        midi: [0x90 + midiChannel, 0x0A],
        off: 0x01,
    });

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
// SHIFT + Play/Stop button
// SYNC button
// BEAT JUMP buttons
// SKIP buttons
// Deck 1-3 toggle buttons
// CENSOR button
// LOOP encoder
// LOOP buttons
// SLIP button
// VINYL button
// KEY LOCK + SYNC + RESET
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
