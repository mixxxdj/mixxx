// eslint-disable-next-line-no-var
var PrimeGo = {};

// Register '0x9n' as a button press and '0x8n' as a button release
components.Button.prototype.isPress = function(channel, control, value, status) {
    return (status & 0xF0) === 0x90;
};

// 'Off' value sets basic LEDs to dim instead of fully off
components.Button.prototype.off = 0x01;

PrimeGo.init = function(_id, _debugging) {
    // Turn off all LEDs
    midi.sendShortMsg(0x90, 0x75, 0x00);
    PrimeGo.leftDeck = new PrimeGo.Deck(1, 2);
    PrimeGo.rightDeck = new PrimeGo.Deck(2, 3);
    // Initial functions go here
};

PrimeGo.shutdown = function() {
    // Dim all LEDs
    midi.sendShortMsg(0x90, 0x75, 0x01);
    // Final functions go here
};

PrimeGo.Deck = function(deckNumber, midiChannel) {
    components.Deck.call(this, deckNumber);

    this.playButton = new components.PlayButton({
        midi: [0x90 + midiChannel, 0x0A],
        //TODO: play_stutter
    });
    this.cueButton = new components.CueButton({
        midi: [0x90 + midiChannel, 0x09]
    });
    //pitchbend -
    //pitchbend +
    //pitchfader
    this.syncButton = new components.SyncButton({
        midi: [0x90 + midiChannel, 0x08],
    });
    //vinyl
    //jogwheel
    //loopencoder
    this.gain = new components.Pot({
        midi: [0xB0 + midiChannel - 2, 0x03],
        group: "[Channel" + deckNumber + "]",
        inKey: "pregain",
    });
    //low
    //mid
    //high
    //sweepfxknob
    //sweepA
    //sweepB
    //pfl
    //volume

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = this.currentDeck;
        }
    });
};

PrimeGo.Deck.prototype = new components.Deck();
