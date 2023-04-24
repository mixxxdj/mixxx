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
 *  - Open the Preferences window
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
    "green",

    // Deck 2
    "blue",

    // Deck 3
    "cyan",

    // Deck 4
    "magenta",

];

// Would you like the TRACK SKIP ([|<<] and [>>|] buttons) to jump to the start and
// end of the track, or seek through it? (Choose "seek" or "skip")
const skipButtonBehaviour = "skip";

// How sensitive should the jog wheel be when nudging (while playing) or navigating
// (while paused) a track? (NOTE: 0.2 is a good value to start with. The larger the
// number, the more sensitive the wheel will be.)
const wheelSensitivity = 0.5;

/**************************************************
 *                                                *
 *                   WARNING!!!                   *
 *                                                *
 *      DO NOT EDIT ANYTHING PAST THIS POINT      *
 *       UNLESS YOU KNOW WHAT YOU'RE DOING.       *
 *                                                *
 **************************************************/

// Convert user-preference for `skipButtonBehaviour` into appropriate keys for components
let trackSkipMode = [];
if (skipButtonBehaviour === "skip") {
    trackSkipMode = ["start", "end"];
} else if (skipButtonBehaviour === "seek") {
    trackSkipMode = ["back", "fwd"];
}

// Component re-jigging for pad mode purposes
components.ComponentContainer.prototype.reconnectComponents = function(operation, recursive) {
    this.forEachComponent(function(component) {
        component.disconnect();
        if (typeof operation === "function") {
            operation.call(this, component);
        }
        if (component.outConnect) { component.connect(); }
        if (component.outTrigger) { component.trigger(); }
    }, recursive);
};

// 'Off' value sets lights to dim instead of off
components.Button.prototype.off = 0x01;

// Colour codes designed to avoid memorizing MIDI velocity values
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

/*
// Used in Swiftb0y's NS6II mapping for tempo fader LEDs
Prime4.physicalSliderPositions = {
    left: 0,
    right: 0,
};
*/

// Map RGB Hex values to MIDI values for Prime 4's colour palette
const Prime4ColorMapper = new ColorMapper({
    0x000020: 0x01, // dark blue
    0x0000AA: 0x02, // dim blue
    0x0000C0: 0x03, // blue
    0x002000: 0x04, // dark green
    0x002020: 0x05, // dark cyan
    0x0020A0: 0x06,
    0x0020C0: 0x07,
    0x00A000: 0x08, // dim green
    0x00A020: 0x09,
    0x00A0A0: 0x0A, // dim cyan
    0x00A0C0: 0x0B,
    0x00C000: 0x0C, // green
    0x00C020: 0x0D,
    0x00C0A0: 0x0E,
    0x00C0C0: 0x0F, // cyan
    0x200000: 0x10, // dark red
    0x200020: 0x11, // dark magenta
    0x2000A0: 0x12,
    0x2000C0: 0x13,
    0x2020A0: 0x16,
    0x2020C0: 0x17,
    0x20A000: 0x18,
    0x20A020: 0x19,
    0x20A0A0: 0x1A,
    0x20A0C0: 0x1B,
    0x20C000: 0x1C,
    0x20C020: 0x1D,
    0x20C0A0: 0x1E,
    0x20C0C0: 0x1F,
    0xA00000: 0x20, // dim red
    0xA00020: 0x21,
    0xA000A0: 0x22, // dim magenta
    0xA000C0: 0x23,
    0xA02000: 0x24,
    0xA02020: 0x25,
    0xA020A0: 0x26,
    0xA020C0: 0x27,
    0xA0A000: 0x28, // dim yellow
    0xA0A020: 0x29,
    0xA0A0A0: 0x2A, // dim white/grey
    0xA0A0C0: 0x2B,
    0xA0C000: 0x2C,
    0xA0C020: 0x2D,
    0xA0C0A0: 0x2E,
    0xA0C0C0: 0x2F,
    0xC00000: 0x30, // red
    0xC00020: 0x31,
    0xC000A0: 0x32,
    0xC000C0: 0x33, // purple
    0xC02000: 0x34, // orange
    0xC02020: 0x35,
    0xC020A0: 0x36,
    0xC020C0: 0x37,
    0xC0A000: 0x38,
    0xC0A020: 0x39,
    0xC0A0A0: 0x3A, // pink
    0xC0A0C0: 0x3B,
    0xC0C000: 0x3C, // yellow
    0xC0C020: 0x3D,
    0xC0C0A0: 0x3E,
    0xC0C0C0: 0x3F, // white
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

Prime4.DeckAssignButton = function(options) {
    components.Button.call(this, options);

    if (!Number.isInteger(this.deckIndex)) {
        throw `invalid deckIndex: ${this.deckIndex}`;
    }
    if (!(this.toDeck instanceof Prime4.Deck)) {
        throw "invalid toDeck";
    }

    const deckSide = (this.deckIndex % 2) === 0 ? "leftDeck" : "rightDeck";
    if (!(Prime4[deckSide] instanceof Prime4.Deck)) {
        throw "invalid deckIndex or structure; We expect Prime4.leftDeck and Prime4.rightDeck to be valid decks representing the physical left and right decks";
    }
    const isActive = () => {
        return Prime4[deckSide] === this.toDeck;
    };

    this.trigger = function() {
        this.output(isActive());
    };

    this.input = function(channel, control, value, status, _group) {
        if (!this.isPress(channel, control, value, status) || isActive()) {
            return;
        }
        Prime4[deckSide].forEachComponent(c => { c.disconnect(); });
        Prime4[deckSide] = this.toDeck;
        this.assignmentButtons.forEachComponent(btn => btn.trigger());
        Prime4[deckSide].forEachComponent(c => { c.connect(); c.trigger(); });
    };

};

Prime4.DeckAssignButton.prototype = Object.create(components.Button.prototype);

Prime4.EffectUnitEncoderInput = function(_channel, _control, value, _status, _group) {
    if (value >= 1 && value < 20) {
        this.inSetParameter(this.inGetParameter() + (value / 100));
    } else if (value <= 127 && value > 100) {
        this.inSetParameter(this.inGetParameter() + ((value - 128) / 100));
    }
    /*
    const signedValue = value > (0x80 / 2) ? value - 128 : value;
    this.inSetParameter(this.inGetParameter() + value / 100);
    */
};

Prime4.init = function(_id, _debug) {
    // Turn off all LEDs
    midi.sendShortMsg(0x90, 0x75, 0x00);

    const decks = [
        new Prime4.Deck(1, 4),
        new Prime4.Deck(2, 5),
        new Prime4.Deck(3, 4),
        new Prime4.Deck(4, 5),
    ];

    // Disconnect all decks at first so they don't fight with each other
    decks.forEach(deck => deck.forEachComponent(comp => { console.log(`disconnecting "${comp.group}, ${comp.inKey}"`); comp.disconnect(); }));

    // Bind the sections
    Prime4.leftDeck = decks[0];
    Prime4.rightDeck = decks[1];

    Prime4.assignmentButtons = new components.ComponentContainer();
    for (let i = 0; i < decks.length; i++) {
        Prime4.assignmentButtons[i] = new Prime4.DeckAssignButton({
            midi: [0x9F, 0x1C + i],
            deckIndex: i,
            toDeck: decks[i],
            assignmentButtons: this.assignmentButtons,
            off: colDeckDark[i],
            on: colDeck[i],
        });
        Prime4.assignmentButtons[i].trigger();
    }

    Prime4.mixerA = new mixerStrip(1, 0);
    Prime4.mixerB = new mixerStrip(2, 1);
    Prime4.mixerC = new mixerStrip(3, 2);
    Prime4.mixerD = new mixerStrip(4, 3);

    // Initialize effect banks
    Prime4.effectBank = [];
    for (let i = 0; i <= 1; i++) {
        Prime4.effectBank[i] = new components.EffectUnit([i + 1, i + 3]);
        for (let j = 0; j < 3; j++) {
            Prime4.effectBank[i].enableButtons[j + 1].midi = [0x96 + i, 0x06 + j];
            Prime4.effectBank[i].knobs[j + 1].midi = [0xB6 + i, 0x01 + j];
            Prime4.effectBank[i].knobs[j + 1].input = Prime4.EffectUnitEncoderInput;
        }
        Prime4.effectBank[i].dryWetKnob.midi = [0xB6 + i, 0x04];
        Prime4.effectBank[i].dryWetKnob.input = Prime4.EffectUnitEncoderInput;
        Prime4.effectBank[i].effectFocusButton.midi = [0x96 + i, 0x0A];
        Prime4.effectBank[i].init();
    }

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

    // Headphone Split Button
    Prime4.split = new components.Button({
        midi: [0x9F, 0x0B],
        group: "[Master]",
        key: "headSplit",
        type: components.Button.prototype.types.toggle,
    });

    Prime4.leftDeck.reconnectComponents();
    Prime4.rightDeck.reconnectComponents();

    // LED Initialization
    midi.sendShortMsg(0x94, 0x1C, 1); // Left Shift Button
    midi.sendShortMsg(0x95, 0x1C, 1); // Right Shift Button
};

Prime4.shutdown = function() {
    // Return all LEDs to initial dim state
    midi.sendShortMsg(0x90, 0x75, 0x01);
};

// All components contained in each mixer strip
const mixerStrip = function(deckNumber, midiOffset) {
    components.Deck.call(this, deckNumber);

    // FX Assign Buttons
    this.fxBankSelect = new components.ComponentContainer;
    for (let u = 1; u <= 2; u++) {
        this.fxBankSelect[u] = new components.EffectAssignmentButton({
            midi: [0x90 + midiOffset, 0x00 + u],
            effectUnit: u,
            group: "[Channel" + deckNumber + "]",
        });
    }

    // VU Meters
    this.vuMeter = new components.Component({
        midi: [0xB0 + midiOffset, 0x0A],
        group: "[Channel" + deckNumber + "]",
        outKey: "VuMeter",
        output: function(value, group) {
            if (engine.getValue(group, "PeakIndicator") === 1) {
                value = 0x7f;
            } else {
                const meter = Math.round(value * 127);
                value = (meter - ((meter - 1) % 13));
                if (value === 1) {
                    value = 0;
                }
            }
            console.log(value);
            this.send(value);
        },
    });

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
    const theDeck = this;

    // Censor Button
    this.censorButton = new components.Button({
        midi: [0x90 + midiChannel, 0x01],
        key: "reverseroll",
        unshift: function() {
            this.inKey = "reverseroll";
        },
        shift: function() {
            this.inKey = "reverse";
        },
    });

    // Skip Backward
    this.skipBackButton = new components.Button({
        midi: [0x90 + midiChannel, 0x04],
        key: trackSkipMode[0],
    });

    // Skip Forward
    this.skipFwdButton = new components.Button({
        midi: [0x90 + midiChannel, 0x05],
        key: trackSkipMode[1],
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
    this.padGrid = new Prime4.PadSection(this, midiChannel - 4);

    // Tempo Fader
    this.tempoFader = new components.Pot({
        midi: [0xB0 + midiChannel, 0x1F],
        inKey: "rate",
        invert: true,
    });

    /*
    // Used in Swiftb0y's NS6II mapping for tempo fader LEDs
    var makeSliderPosAccessors = function() {
        var lr = midiChannel % 2 === 0 ? "left" : "right";
        return {
            setter: function(pos) {
                Prime4.physicalSliderPositions[lr] = pos;
            },
            getter: function() {
                return Prime4.physicalSliderPositions[lr];
            }
        };
    };

    var sliderPosAccessors = makeSliderPosAccessors();

    // LED indicator when pitch fader is at centre.
    this.tempoFeedback = new components.Component({
        midi: [0x90 + midiChannel, 0x34],
        outKey: "rate",
        on: 0x02,
        off: 0x00,
        outValueScale: function(value) {
            return value === 0 ? this.on : this.off;
        },
    });

    var takeoverLEDValues = {
        OFF: 0,
        DIMM: 1,
        FULL: 2,
    };
    var takeoverLEDControls = {
        up: 0x35,
        center: 0x34,
        down: 0x33,
    };

    this.takeoverLeds = new components.Component({
        midi: [0x90 + midiChannel, takeoverLEDControls.center],
        outKey: "rate",
        off: 0,
        output: function(softwareSliderPosition) {
            // rate slider centered?
            this.send(softwareSliderPosition === 0 ? takeoverLEDValues.FULL : takeoverLEDValues.OFF);

            var distance2Brightness = function(distance) {
                // src/controllers/softtakeover.cpp
                // SoftTakeover::kDefaultTakeoverThreshold = 3.0 / 128;
                var takeoverThreshold = 3 / 128;
                if (distance > takeoverThreshold && distance < 0.10) {
                    return takeoverLEDValues.DIMM;
                } else if (distance >= 0.10) {
                    return takeoverLEDValues.FULL;
                } else {
                    return takeoverLEDValues.OFF;
                }
            };

            var normalizedPhysicalSliderPosition = sliderPosAccessors.getter()*2 - 1;
            var distance = Math.abs(normalizedPhysicalSliderPosition - softwareSliderPosition);
            var directionLedBrightness = distance2Brightness(distance);

            if (normalizedPhysicalSliderPosition > softwareSliderPosition) {
                midi.sendShortMsg(this.midi[0], takeoverLEDControls.up, takeoverLEDValues.OFF);
                midi.sendShortMsg(this.midi[0], takeoverLEDControls.down, directionLedBrightness);
            } else {
                midi.sendShortMsg(this.midi[0], takeoverLEDControls.down, takeoverLEDValues.OFF);
                midi.sendShortMsg(this.midi[0], takeoverLEDControls.up, directionLedBrightness);
            }
        },
    });
    */

    // Keylock Button
    this.keylockButton = new components.Button({
        midi: [0x90 + midiChannel, 0x22],
        key: "keylock",
        type: components.Button.prototype.types.toggle,
    });

    this.vinylButton = new components.Button({
        midi: [0x90 + midiChannel, 0x23],
        type: components.Button.prototype.types.toggle,
        input: function(channel, control, value, status, _group) {
            if (!this.isPress(channel, control, value, status)) {
                return;
            }
            theDeck.jogWheel.vinylMode = !theDeck.jogWheel.vinylMode;
            this.trigger();
        },
        trigger: function() {
            this.output(theDeck.jogWheel.vinylMode);
        },
    });

    // Jog Wheel
    this.jogWheel = new components.JogWheelBasic({
        deck: script.deckFromGroup(this.currentDeck),
        wheelResolution: 1000,
        alpha: 1/8,
        beta: 1/8/32,
        rpm: 33 + 1/3,
        // Instead of relative movements between this and the last position,
        // the controller reports the absolute position of the wheel with
        // 14-bit precision. Because of that, we need to reconstruct the value
        // and then transform it into the relative directions expected by Mixxx.
        inputWheelMSB: function(_channel, _control, value, _status, _group) {
            this.wheelMSB = value;
        },
        inputWheelLSB: function(channel, control, value, status, group) {
            this.inputWheel(channel, control, (this.wheelMSB << 7) + value, status, group);
        },
        previousPosition: null,
        wrappingValue: Math.pow(2, 14),
        relativeFromAbsolute: function(value) {
            // The first value of the controller will probably be random
            // and thus we just have to swallow it until we have the second value
            // to find the difference
            if (this.previousPosition === null) {
                this.previousPosition = value;
                return 0;
            }
            // This finds the shortest distance between the current value
            // and the last one, and preserves the orientation
            const delta = value - this.previousPosition;
            let remainder = ((delta % this.wrappingValue) + this.wrappingValue) % this.wrappingValue;
            //let remainder = script.posMod(delta, this.wrappingValue);
            if (remainder * 2 > this.wrappingValue) {
                remainder -= this.wrappingValue;
            }
            this.previousPosition = value;
            return remainder;
        },
        jogScale: function(val) {
            // wheelSensitivity is user-configurable, see top of file.
            return val * wheelSensitivity;
        },
        inputWheel: function(channel, control, value, _status, _group) {
            value = this.relativeFromAbsolute(value);
            if (engine.isScratching(this.deck)) {
                engine.scratchTick(this.deck, value);
            } else {
                this.inSetValue(this.jogScale(value));
            }
        },
    });

    // Jog Wheel LED
    this.jogWheelLed = new components.Component({
        midi: [0x90 + midiChannel, 0x21],
        on: colDeck[deckNumbers - 1],
        trigger: function() {
            this.send(this.on);
        },
    });

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

    // Load Buttons
    this.deckLoad = new components.Button({
        midi: [0x9F, midiChannel - 3],
        key: "LoadSelectedTrack",
        off: colDeckDark[deckNumbers - 1],
        on: colDeck[deckNumbers - 1],
    });

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
        Prime4.effectBank[0].shift();
        Prime4.effectBank[1].shift();
    } else {
        midi.sendShortMsg(0x90 + channel, control, 0x01);
        Prime4.leftDeck.unshift();
        Prime4.rightDeck.unshift();
        Prime4.effectBank[0].unshift();
        Prime4.effectBank[1].unshift();
    }
};

//========== PERFORMANCE PADS ==========//

// Access the appropriate mode-select pad without remembering MIDI values
Prime4.padMode = {
    HOTCUE: 0x0B,
    LOOP: 0x0C,
    ROLL: 0x0D,
    SLICER: 0x0E,
};

Prime4.PadSection = function(deck, offset) {
    components.ComponentContainer.call(this);
    const theContainer = this;

    // Create component containers for each pad mode
    const modes = new components.ComponentContainer({
        "hotcue": new Prime4.hotcueMode(deck, offset),
        "loop": new Prime4.loopMode(deck, offset),
        //TODO: "autoloop": new Prime4.autoloopMode(deck, offset),
        "roll": new Prime4.rollMode(deck, offset),
        "sampler": new Prime4.samplerMode(deck, offset),
    });

    modes.forEachComponent(c => c.disconnect());

    const controlToPadMode = control => {
        let mode;
        switch (control) {
        case Prime4.padMode.HOTCUE:
            mode = modes.hotcue;
            break;
        case Prime4.padMode.LOOP:
            mode = modes.loop;
            /*
            // TODO: Implement autoloop mode
            if (Prime4.PadSection.currentMode === modes.loop) {
                mode = modes.autoloop;
            } else {
                mode = modes.loop;
            }
            */
            break;
        case Prime4.padMode.ROLL:
            mode = modes.roll;
            break;
        case Prime4.padMode.SLICER:
            mode = modes.sampler;
            break;
        }

        return mode;
    };

    this.offset = offset;

    this.padModeSelectLeds = new components.Component({
        trigger: function() {
            for (const mode of Object.values(modes)) {
                midi.sendShortMsg(0x94 + offset, mode.ledControl, theContainer.currentMode === mode ? mode.colourOn : mode.colourOff);
            }
        },
    }, false);

    // Function for switching between pad modes
    this.setPadMode = function(control) {
        const newMode = controlToPadMode(control);

        // Exit early if requested mode is already active or unavailable
        if (newMode === this.currentMode || newMode === undefined) {
            return;
        }

        // Disable LED of current mode button
        if (this.currentMode) {
            // Disconnect pads from current mode
            this.currentMode.forEachComponent(function(component) {
                component.disconnect();
                component.outConnect = false;
            });
        }

        // Connect pads to new mode
        newMode.forEachComponent(function(component) {
            component.outConnect = true;
            component.connect();
            component.trigger();
        });

        // Assign mode select buttons in XML file
        this.padModeButtonPressed = function(channel, control, value, _status, _group) {
            if (value) {
                this.setPadMode(control);
            }
        };

        this.currentMode = newMode;

        theContainer.padModeSelectLeds.trigger();
    };

    // Start in Hotcue mode
    this.setPadMode(Prime4.padMode.HOTCUE);

};

Prime4.PadSection.prototype = Object.create(components.ComponentContainer.prototype);

// Worry about parameter buttons later
//Prime4.PadSection.prototype.paramButtonPressed = function(channel, control, value, status, group) {};

// Assign pads mapped in XML file
Prime4.PadSection.prototype.performancePad = function(channel, control, value, status, group) {
    const i = (control - 0x0E);
    this.currentMode.pads[i].input(channel, control, value, status, group);
};

// HOTCUE MODE
Prime4.hotcueMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = Prime4.padMode.HOTCUE;
    this.colourOn = Prime4.rgbCode.white;
    this.colourOff = Prime4.rgbCode.whiteDark;
    this.pads = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new components.HotcueButton({
            number: i,
            group: deck.currentDeck,
            midi: [0x94 + offset, 0x0E + i],
            colorMapper: Prime4ColorMapper,
            on: this.colourOn,
            off: this.colourOff,
            outConnect: false,
        });
    }
};
Prime4.hotcueMode.prototype = Object.create(components.ComponentContainer.prototype);

// LOOP MODE
Prime4.loopMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = Prime4.padMode.LOOP;
    this.colourOn = Prime4.rgbCode.magenta;
    this.colourOff = Prime4.rgbCode.whiteDark;
    const PerformancePad = function(n) {
        this.midi = [0x94 + offset, 0x0E + n];
        this.number = n;
        this.outKey = "hotcue_" + this.number + "_enabled";
        this.colorKey = "hotcue_" + this.number + "_color";
        components.Button.call(this);
    };
    PerformancePad.prototype = new components.Button({
        group: deck.currentDeck,
        on: this.colourOn,
        off: Prime4.rgbCode.magentaDark,
        colorMapper: Prime4ColorMapper,
        outConnect: false,
        unshift: function() {
            this.inKey = "hotcue_" + this.number + "_cueloop";
        },
        shift: function() {
            this.inKey = "hotcue_" + this.number + "_cueloop";
        },
        output: components.HotcueButton.prototype.output,
        outputColor: components.HotcueButton.prototype.outputColor,
        connect: components.HotcueButton.prototype.connect,
    });
    this.pads = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new PerformancePad(i);
    }
};
Prime4.loopMode.prototype = Object.create(components.ComponentContainer.prototype);

// ROLL MODE
Prime4.rollMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = Prime4.padMode.ROLL;
    this.colourOn = Prime4.rgbCode.green;
    this.colourOff = Prime4.rgbCode.whiteDark;
    this.pads = new components.ComponentContainer();
    // NOTE: The Prime 4's standalone Roll mode includes triplet loop rolls, but
    //       Mixxx doesn't support those yet.
    this.loopSize = [0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8];
    for (let i = 1; i <= 8; i++) {
        const loopSize = (this.loopSize[i - 1]);
        this.pads[i] = new components.Button({
            midi: [0x94 + offset, 0x0E + i],
            group: deck.currentDeck,
            outKey: "beatloop_" + loopSize + "_enabled",
            inKey: "beatlooproll_" + loopSize + "_activate",
            on: this.colourOn,
            off: Prime4.rgbCode.greenDim,
            outConnect: false,
        });
    }
};
Prime4.rollMode.prototype = Object.create(components.ComponentContainer.prototype);

// SAMPLER MODE
Prime4.samplerMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = Prime4.padMode.SLICER;
    this.colourOn = Prime4.rgbCode.yellow;
    this.colourOff = Prime4.rgbCode.whiteDark;
    this.pads = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new components.SamplerButton({
            number: i,
            midi: [0x94 + offset, 0x0E + i],
            colorMapper: Prime4ColorMapper,
            on: this.colourOn,
            off: Prime4.rgbCode.yellowDark,
            outConnect: false,
        });
    }
};
Prime4.samplerMode.prototype = Object.create(components.ComponentContainer.prototype);
