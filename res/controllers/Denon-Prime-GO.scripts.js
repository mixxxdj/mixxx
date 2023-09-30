// eslint-disable-next-line-no-var
var PrimeGo = {};

// Register '0x9n' as a button press and '0x8n' as a button release
components.Button.prototype.isPress = function(channel, control, value, status) {
    return (status & 0xF0) === 0x90;
};

// 'Off' value sets basic LEDs to dim instead of fully off
components.Button.prototype.off = 0x01;

const initMsg = [0xF0, 0x00, 0x02, 0x0B, 0x7F, 0x0C, 0x04, 0x00, 0x00, 0xF7];

// Send colors to RGB pads via SysEx
// F0 00 02 0B 7F 0C 03 00 05 status midino red green blue F7

PrimeGo.init = function(_id, _debugging) {
    // Turn off all LEDs
    midi.sendShortMsg(0x90, 0x75, 0x00);

    // Initialize Shift LED
    midi.sendShortMsg(0x9F, 0x08, 0x01);

    // Get position of all components
    midi.sendSysexMsg(initMsg, initMsg.length);

    PrimeGo.leftDeck = new PrimeGo.Deck(1, 2);
    PrimeGo.rightDeck = new PrimeGo.Deck(2, 3);

    PrimeGo.encoderLoad = new components.Button({
        midi: [0x9F, 0x06],
        group: "[Library]",
        key: "GoToItem",
    });

    PrimeGo.moveBack = new components.Button({
        midi: [0x9F, 0x03],
        group: "[Library]",
        key: "MoveFocusBackward",
    });

    PrimeGo.moveForward = new components.Button({
        midi: [0x9F, 0x04],
        group: "[Library]",
        key: "MoveFocusForward",
    });

    PrimeGo.maxView = new components.Button({
        midi: [0x9F, 0x07],
        group: "[Skin]",
        key: "show_maximized_library",
        type: components.Button.prototype.types.toggle,
    });
};

PrimeGo.shutdown = function() {
    // Dim all LEDs
    midi.sendShortMsg(0x90, 0x75, 0x01);
};

PrimeGo.Deck = function(deckNumber, midiChannel) {
    components.Deck.call(this, deckNumber);

    this.deckLoad = new components.Button({
        midi: [0x9F, midiChannel - 1],
        key: "LoadSelectedTrack",
        shift: function() {
            this.inKey = "eject";
            this.outKey = this.inKey;
        },
        unshift: function() {
            this.inKey = "LoadSelectedTrack",
            this.outKey = this.inKey;
        },
    });

    this.playButton = new components.PlayButton({
        midi: [0x90 + midiChannel, 0x0A],
        unshift: function() {
            components.PlayButton.prototype.unshift.call(this);
            this.type = components.Button.prototype.types.toggle;
        },
        shift: function() {
            this.inKey = "play_stutter";
            this.type = components.Button.prototype.types.push;
        },
    });

    this.cueButton = new components.CueButton({
        midi: [0x90 + midiChannel, 0x09]
    });

    this.pitchBendUp = new components.Button({
        midi: [],
        type: components.Button.prototype.types.push,
        key: "rate_temp_up",
        //TODO: tempo fader range
    });

    this.pitchBendDown = new components.Button({
        midi: [],
        type: components.Button.prototype.types.push,
        key: "rate_temp_down",
        //TODO: tempo fader range
    });

    this.tempoFader = new components.Pot({
        midi: [0xB0 + midiChannel, 0x1F],
        inKey: "rate",
    });

    this.syncButton = new components.SyncButton({
        midi: [0x90 + midiChannel, 0x08],
    });

    //vinylButton

    //jogwheel

    //loopencoder

    this.gain = new components.Pot({
        midi: [0xB0 + midiChannel - 2, 0x03],
        group: "[Channel" + deckNumber + "]",
        inKey: "pregain",
    });

    this.eqLow = new components.Pot({
        midi: [0xB0 + midiChannel - 2, 0x08],
        group: "[EqualizerRack1_[Channel" + deckNumber + "]_Effect1]",
        inKey: "parameter1",
    });

    this.eqMid = new components.Pot({
        midi: [0xB0 + midiChannel - 2, 0x06],
        group: "[EqualizerRack1_[Channel" + deckNumber + "]_Effect1]",
        inKey: "parameter2",
    });

    this.eqHigh = new components.Pot({
        midi: [0xB0 + midiChannel - 2, 0x04],
        group: "[EqualizerRack1_[Channel" + deckNumber + "]_Effect1]",
        inKey: "parameter3",
    });

    //sweepfxknob (mapped in XML)

    //sweepA
    this.sweepA = new components.Button({
        midi: [0x90 + midiChannel - 2, 0x0E],
        group: "QuickEffectRack1_[Channel1]]",
        key: "enabled",
    });

    //sweepB

    this.headphoneCue = new components.Button({
        midi: [0x90 + midiChannel - 2, 0x0D],
        key: "pfl",
        type: components.Button.prototype.types.toggle,
    });

    this.volumeFader = new components.Pot({
        midi: [0xB0 + midiChannel - 2, 0x0E],
        inKey: "volume",
    });

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = this.currentDeck;
        }
    });
};

PrimeGo.Deck.prototype = new components.Deck();

PrimeGo.shift = false;
PrimeGo.shiftState = function(channel, control, value) {
    PrimeGo.shift = value === 0x7F;
    if (PrimeGo.shift) {
        midi.sendShortMsg(0x9F, 0x08, 0x7F);
        PrimeGo.leftDeck.shift();
        PrimeGo.rightDeck.shift();
        PrimeGo.leftDeck.reconnectComponents();
        PrimeGo.rightDeck.reconnectComponents();
    } else {
        midi.sendShortMsg(0x9F, 0x08, 0x01);
        PrimeGo.leftDeck.unshift();
        PrimeGo.rightDeck.unshift();
        PrimeGo.leftDeck.reconnectComponents();
        PrimeGo.rightDeck.reconnectComponents();
    }
};
