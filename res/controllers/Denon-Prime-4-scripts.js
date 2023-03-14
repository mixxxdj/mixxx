// eslint-disable-next-line-no-var
var Prime4 = {};

/**
 * DENON DJ PRIME 4 - CONTROLLER MAPPING FOR MIXXX
 *
 * When you boot up the Prime 4, it will be in standalone mode,
 * which will prevent it from connecting to Mixxx. To put your
 * Prime 4 into Computer Mode, follow these steps:
 *
 *  - Turn on the Prime 4
 *  - Hold down the VIEW button to access the main menu
 *  - Tap "SOURCES"
 *  - Tap the icon of a laptop with a USB symbol in the top-right
 *  - Tap "YES"
 *  - Plug in the Prime 4 to your computer using the provided USB cable
 *  - Confirm that your device is connected (This might take a few seconds)
 *  - Start Mixxx
 *  - Press Ctrl + P to access the Preferences window
 *  - Click "Controllers" to expand the list of USB devices Mixxx is detecting
 *  - Click "PRIME 4 Control Surface MIDI" from the list
 *  - In the "Load Mapping" dropdown list, select "Denon Prime 4 - Mixxx Mapping"
 *  - Make sure the "Enabled" box is ticked
 *  - Click "Apply" or "OK"
 *
 * If everything works, you should see the LEDs on your Prime 4
 * light up and respond to Mixxx. Enjoy!
 *
 * Below are a few parameters you can change if you want to
 * customize anything :)
 */

// Would you like each deck to show the colour of its loaded track (if specified)?
// (Choose between true or false)
//const showTrackColor = false; // NOT IMPLEMENTED YET

// What default colour would you like each deck to be?
// (Choose between "red", "yellow", "green", "cyan", "blue", "magenta", or "white")
const deckColors = [

    // Deck 1
    "red",

    // Deck 2
    "yellow",

    // Deck 3
    "blue",

    // Deck 4
    "blue",

];

/**************************************************
 *                                                *
 *                   WARNING!!!                   *
 *                                                *
 *      DO NOT EDIT ANYTHING PAST THIS POINT      *
 *       UNLESS YOU KNOW WHAT YOU'RE DOING.       *
 *                                                *
 **************************************************/

// 'Off' value sets lights to dim instead of off
components.Button.prototype.off = 0x01;

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

// Map RGB Hex values to MIDI values for Prime 4's colour palette
const Prime4ColorMapper = new ColorMapper({
    //  0x000055: 0x01, // dark blue
    //  0x0000AA: 0x02, // dim blue
    0x0000FF: 0x03, // blue
    //  0x005500: 0x04, // dark green
    //  0x005555: 0x05, // dark cyan
    //  0x0055AA: 0x06,
    //  0x0055FF: 0x07,
    //  0x00AA00: 0x08, // dim green
    //  0x00AA55: 0x09,
    //  0x00AAAA: 0x0A, // dim cyan
    //  0x00AAFF: 0x0B,
    0x00FF00: 0x0C, // green
    //  0x00FF55: 0x0D,
    //  0x00FFAA: 0x0E,
    0x00FFFF: 0x0F, // cyan
    //  0x550000: 0x10, // dark red
    //  0x550055: 0x11, // dark magenta
    //  0x5500AA: 0x12,
    //  0x5500FF: 0x13,
    //  0x5555AA: 0x16,
    //  0x5555FF: 0x17,
    //  0x55AA00: 0x18,
    //  0x55AA55: 0x19,
    //  0x55AAAA: 0x1A,
    //  0x55AAFF: 0x1B,
    //  0x55FF00: 0x1C,
    //  0x55FF55: 0x1D,
    //  0x55FFAA: 0x1E,
    //  0x55FFFF: 0x1F,
    //  0xAA0000: 0x20, // dim red
    //  0xAA0055: 0x21,
    //  0xAA00AA: 0x22, // dim magenta
    //  0xAA00FF: 0x23,
    //  0xAA5500: 0x24,
    //  0xAA5555: 0x25,
    //  0xAA55AA: 0x26,
    //  0xAA55FF: 0x27,
    //  0xAAAA00: 0x28, // dim yellow
    //  0xAAAA55: 0x29,
    //  0xAAAAAA: 0x2A, // dim white/grey
    //  0xAAAAFF: 0x2B,
    //  0xAAFF00: 0x2C,
    //  0xAAFF55: 0x2D,
    //  0xAAFFAA: 0x2E,
    //  0xAAFFFF: 0x2F,
    0xFF0000: 0x30, // red
    //  0xFF0055: 0x31,
    //  0xFF00AA: 0x32,
    0xFF00FF: 0x33, // purple
    0xFF5500: 0x34, // orange
    //  0xFF5555: 0x35,
    //  0xFF55AA: 0x36,
    //  0xFF55FF: 0x37,
    //  0xFFAA00: 0x38,
    //  0xFFAA55: 0x39,
    0xFFAAAA: 0x3A, // pink
    //  0xFFAAFF: 0x3B,
    0xFFFF00: 0x3C, // yellow
    //  0xFFFF55: 0x3D,
    //  0xFFFFAA: 0x3E,
    0xFFFFFF: 0x3F, // white
});

// Set active + inactive values for user-defined deck colours
const colDeck = [
    Prime4.rgbCode[deckColors[0]],
    Prime4.rgbCode[deckColors[1]],
    Prime4.rgbCode[deckColors[2]],
    Prime4.rgbCode[deckColors[3]],
];
const colDeckDark = [
    Prime4.rgbCode[deckColors[0] + "Dark"],
    Prime4.rgbCode[deckColors[1] + "Dark"],
    Prime4.rgbCode[deckColors[2] + "Dark"],
    Prime4.rgbCode[deckColors[3] + "Dark"],
];

// Register '0x9n' as a button press and 'ox8n' as a button release
components.Button.prototype.isPress = function(channel, control, value, status) {
    return (status & 0xF0) === 0x90;
};

Prime4.init = function(_id, _debug) {
    // Turn off all LEDs
    midi.sendShortMsg(0x90, 0x75, 0x00);

    // Initialise all three physical sections of the unit
    Prime4.leftDeck = new Prime4.Deck([1, 3], 4);
    Prime4.rightDeck = new Prime4.Deck([2, 4], 5);
    Prime4.mixerA = new mixerStrip(1, 0);
    Prime4.mixerB = new mixerStrip(2, 1);
    Prime4.mixerC = new mixerStrip(3, 2);
    Prime4.mixerD = new mixerStrip(4, 3);

    // Load song to deck with library encoder button
    Prime4.encoderLoad = new components.Button({
        midi: [0x9F, 0x06],
        group: "[Library]",
        key: "GoToItem",
    });

    // View Button
    Prime4.maxView = new components.Button({
        midi: [0x9F, 0x07],
        group: "[Master]",
        key: "maximize_library",
        type: components.Button.prototype.types.toggle,
    });

    // BACK Button
    Prime4.moveBack = new components.Button({
        midi: [0x9F, 0x03],
        group: "[Library]",
        key: "MoveFocusBackward",
    });

    // FWD Button
    Prime4.moveForward = new components.Button({
        midi: [0x9F, 0x04],
        group: "[Library]",
        key: "MoveFocusForward",
    });

    // Sweep FX - Filter Button
    Prime4.sweepFilter = new components.Button({
        midi: [0x9F, 0x0C],
        group: "[QuickEffectRack1_[Channel1]]",
        key: "enabled",
        on: 0x02,
        off: 0x01,
        input: function(_channel, _control, value, _status, _group) {
            for (let i = 1; i <= 4; i++) {
                if (value > 0) {
                    const effect = "[QuickEffectRack1_[Channel" + i + "]]";
                    if (engine.getParameter(effect, "enabled") > 0) {
                        engine.setParameter(effect, "enabled", 0);
                    } else {
                        engine.setParameter(effect, "enabled", 1);
                    }
                }
            }
        },
    });
    // Sweep FX - Echo Button
    Prime4.sweepEcho = new components.Button({
        midi: [0x9F, 0x0D],
    });
    // Sweep FX - Wash Button
    Prime4.sweepWash = new components.Button({
        midi: [0x9F, 0x0E],
    });
    // Sweep FX - Noise Button
    Prime4.sweepNoise = new components.Button({
        midi: [0x9F, 0x0F],
    });

    // Initial LED values to set (Hopefully these will automatically initialize, but for now they won't.)
    midi.sendShortMsg(0x9F, 0x1C, colDeck[0]);                        // Deck 1 Toggle
    midi.sendShortMsg(0x9F, 0x1D, colDeck[1]);                        // Deck 2 Toggle
    midi.sendShortMsg(0x9F, 0x1E, colDeckDark[2]);                    // Deck 3 Toggle
    midi.sendShortMsg(0x9F, 0x1F, colDeckDark[3]);                    // Deck 4 Toggle
    midi.sendShortMsg(0x94, 0x21, colDeck[0]);                        // Left Jog Wheel
    midi.sendShortMsg(0x95, 0x21, colDeck[1]);                        // Right Jog Wheel
    midi.sendShortMsg(0x94, 0x0B, Prime4.rgbCode.blue);             // Hot-Cue Button - Left Deck
    midi.sendShortMsg(0x95, 0x0B, Prime4.rgbCode.blue);             // Hot-Cue Button - Right Deck
    midi.sendShortMsg(0x9F, 0x0B, 1);                                 // Headphone Split Button
    midi.sendShortMsg(0x94, 0x1C, 1);                                 // Left Shift Button
    midi.sendShortMsg(0x95, 0x1C, 1);                                 // Right Shift Button
};

Prime4.shutdown = function() {
    // Return all LEDs to initial dim state
    midi.sendShortMsg(0x90, 0x75, 0x01);
};

// All components contained in each mixer strip
const mixerStrip = function(deckNumber, midiOffset) {
    components.Deck.call(this, deckNumber);

    // PFL Button
    this.headphoneCue = new components.Button({
        midi: [0x90 + midiOffset, 0x0D],
        key: "pfl",
        on: colDeck[midiOffset],
        off: colDeckDark[midiOffset],
        type: components.Button.prototype.types.toggle,
    });

    // Crossfader Assign Switch
    this.xFaderSwitch = new components.Button({
        midi: [0x90 + midiOffset, 0x0F],
        inKey: "orientation",
        input: function(_channel, _control, value, _status, _group) {
            this.inSetValue(value);
        },
    });

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = this.currentDeck;
        }
    });
};

mixerStrip.prototype = new components.Deck();

// All components contained on each deck
Prime4.Deck = function(deckNumbers, midiChannel) {
    components.Deck.call(this, deckNumbers);

    // Censor Button
    this.censorButton = new components.Button({
        midi: [0x90 + midiChannel, 0x01],
        key: "reverseroll",
    });

    // Beatjump Backward
    this.bjumpBackButton = new components.Button({
        midi: [0x90 + midiChannel, 0x06],
        key: "beatjump_backward",
    });

    // Beatjump Forward
    this.bjumpFwdButton = new components.Button({
        midi: [0x90 + midiChannel, 0x07],
        key: "beatjump_forward",
    });

    // Sync Button
    this.syncButton = new components.SyncButton({
        midi: [0x90 + midiChannel, 0x08],
    });

    // Cue Button
    this.cueButton = new components.CueButton({
        midi: [0x90 + midiChannel, 0x09],
    });

    // Play Button
    this.playButton = new components.PlayButton({
        midi: [0x90 + midiChannel, 0x0A],
        unshift: function() {
            components.PlayButton.prototype.unshift.call(this);
            this.type = components.Button.prototype.types.toggle;
        },
        shift: function() {
            this.inKey = "play_stutter";
            this.type = components.Button.prototype.types.push;
        }
    });

    // Performance Pads
    // TODO: Figure out how to work with different pad modes
    this.performancePad = [];

    let padMode = "HOT CUE"; // Set to Hotcue mode by default for now, while I work on handling other pad modes.

    /********* BEGIN REVIEW REQUEST **********/
    // These are my attempt at handling the pad modes being switched. Unsuccessful so far.
    // I can call the functions OK, but the padMode variable changing doesn't seem
    // to update the pads themselves. Not sure how to handle this with Components
    // yet.
    this.hotcueMode = function() {
        midi.sendShortMsg(0x94, 0x0B, Prime4.rgbCode.yellow);
        midi.sendShortMsg(0x94, 0x0C, Prime4.rgbCode.blueDark);
        padMode = "HOT CUE";
    };
    this.loopMode = function() {
        midi.sendShortMsg(0x94, 0x0B, Prime4.rgbCode.blueDark);
        midi.sendShortMsg(0x94, 0x0C, Prime4.rgbCode.yellow);
        padMode = "LOOP";
    };

    if (padMode === "HOT CUE") {
        for (let i = 1; i <= 8; i++) {
            this.performancePad[i] = new components.HotcueButton({
                number: i,
                midi: [0x90 + midiChannel, 0x0E + i],
                colorMapper: Prime4ColorMapper,
                off: Prime4.rgbCode.whiteDark,
            });
        }
    } else if (padMode === "LOOP") {
        for (let i = 1; i <= 8; i++) {
            this.performancePad[i] = new components.Button({
                midi: [0x90 + midiChannel, 0x0E + i],

                // Placeholder just to confirm that I can actually switch between pad modes.
                group: "[QuickEffectRack1_[Channel1]]",
                outKey: "enabled",
            });
        }
    }
    /********* END REVIEW REQUEST **********/

    // Tempo Fader
    this.tempoFader = new components.Pot({
        midi: [0xB0 + midiChannel, 0x1F],
        inKey: "rate",
        invert: true,
    });

    // LED indicator when pitch fader is at centre. slightly buggy when switching decks.
    this.tempoFeedback = new components.Button({
        midi: [0x90 + midiChannel, 0x34],
        outKey: "rate",
        on: 0x02,
        off: 0x00,
        outValueScale: function(value) {
            return value === 0 ? this.on : this.off;
        },
    });

    // Keylock Button
    this.keylockButton = new components.Button({
        midi: [0x90 + midiChannel, 0x22],
        key: "keylock",
        type: components.Button.prototype.types.toggle,
    });

    /*
    this.vinylButton = new components.Button({
        midi: [0x90 + midiChannel, 0x23],
        //TODO: Jog Wheel Functionality First
        type: components.Button.prototype.types.toggle,
    });
    */

    // Slip Mode Button
    this.slipButton = new components.Button({
        midi: [0x90 + midiChannel, 0x24],
        key: "slip_enabled",
        type: components.Button.prototype.types.toggle,
    });
    // Loop In Button
    this.loopInButton = new components.Button({
        midi: [0x90 + midiChannel, 0x25],
        key: "loop_in",
    });
    // Loop Out Button
    this.loopOutButton = new components.Button({
        midi: [0x90 + midiChannel, 0x26],
        key: "loop_out",
    });

    this.deckLoad = new components.Button({
        midi: [0x9F, midiChannel - 3],
        key: "LoadSelectedTrack",

        // TODO: Make load buttons reflect colour of currently active deck.
        //       This is harder than expected due to the fact that I rely on the MIDI channel
        //       of each physical deck component to determine which deck it's manipulating in
        //       Mixxx. The deck load buttons, however, are both on MIDI channel 16, so I need
        //       a better way to determine this.
        off: Prime4.rgbCode.greenDark,
        on: Prime4.rgbCode.green,
    });

    // Change from Deck 3/4 to Deck 1/2
    this.deckToggleButtonA = function(value) {
        if (value > 0) {
            // Each physical deck has its own MIDI channel, which channelCheck converts to the appropriate deck
            const channelCheck = midiChannel - 1;
            if (this.currentDeck === "[Channel" + channelCheck + "]") {
                this.toggle();

                // Update Jog Wheel and Toggle Button LEDs to reflect active deck
                midi.sendShortMsg(0x90 + midiChannel, 0x21, colDeck[midiChannel - 4]); // Jog Wheel LED
                midi.sendShortMsg(0x9F, 0x18 + midiChannel, colDeck[midiChannel - 4]); // Active deck - bright LED
                midi.sendShortMsg(0x9F, 0x1A + midiChannel, colDeckDark[midiChannel - 2]); // Inactive deck - dark LED
                midi.sendShortMsg(0x9F, midiChannel - 3, colDeckDark[midiChannel - 4]); // Inactive deck - dark LED
            }
        }
    };
    // Change from Deck 1/2 to Deck 3/4
    this.deckToggleButtonB = function(value) {
        if (value > 0) {
            const channelCheck = midiChannel - 3;
            if (this.currentDeck === "[Channel" + channelCheck + "]") {
                this.toggle();

                // Update Jog Wheel and Toggle Button LEDs to reflect active deck
                midi.sendShortMsg(0x90 + midiChannel, 0x21, colDeck[midiChannel - 2]); // Jog Wheel LED
                midi.sendShortMsg(0x9F, 0x18 + midiChannel, colDeckDark[midiChannel - 4]); // Inactive deck - dark LED
                midi.sendShortMsg(0x9F, 0x1A + midiChannel, colDeck[midiChannel - 2]); // Active deck - bright LED
                midi.sendShortMsg(0x9F, midiChannel - 3, colDeckDark[midiChannel - 2]); // Inactive deck - dark LED
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

Prime4.shift = false;
Prime4.shiftState = function(channel, control, value) {
    Prime4.shift = value === 0x7F;
    if (Prime4.shift) {
        midi.sendShortMsg(0x90 + channel, control, 0x02);
        Prime4.leftDeck.shift();
        Prime4.rightDeck.shift();
    } else {
        midi.sendShortMsg(0x90 + channel, control, 0x01);
        Prime4.leftDeck.unshift();
        Prime4.rightDeck.unshift();
    }
};
