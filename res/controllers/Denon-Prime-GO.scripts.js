// eslint-disable-next-line-no-var
var PrimeGo = {};

//========== USER-CONFIGURABLE OPTIONS ==========//
const nudgeSensitivity = 0.5;

//========== CAUTION: DON'T EDIT PAST THIS POINT ==========//
//==========  UNLESS YOU KNOW WHAT YOU'RE DOING  ==========//

// mode-select pad index list to avoid remembering MIDI values
PrimeGo.padMode = {
    HOTCUE: 0x0B,
    LOOP: 0x0C,
    ROLL: 0x0D,
    BANK: 0x0E,
};

// Default color arrays for the performance pad SysEx messages
const padOff = [0x07, 0x07, 0x07];
const padModeA = [0x00, 0x10, 0x7f];
const padModeB = [0x00, 0x7f, 0x07];

// Register '0x9n' as a button press and '0x8n' as a button release
components.Button.prototype.isPress = function(channel, control, value, status) {
    return (status & 0xF0) === 0x90;
};

// 'Off' value sets basic LEDs to dim instead of fully off
components.Button.prototype.off = 0x01;

const initMsg = [0xF0, 0x00, 0x02, 0x0B, 0x7F, 0x0C, 0x04, 0x00, 0x00, 0xF7];

// Send colors to RGB pads via SysEx
// F0 00 02 0B 7F 0C 03 00 05 status midino red green blue F7
const sendSysexColor = function(channel, control, colorArray) {
    // channel 2 = left deck, channel 3 = right deck
    // colorArray should consist of 3 values between 0 and 127
    const msg = [0xF0, 0x00, 0x02, 0x0B, 0x7F, 0x0C, 0x03, 0x00, 0x05, channel, control, colorArray[0], colorArray[1], colorArray[2], 0xF7];
    midi.sendSysexMsg(msg, msg.length);
};

// Provide functions for encoder to cycle through an array of values
// See NS6II mapping
PrimeGo.CyclingArrayView = class {
    // Array that stops at either end (e.g. beatjump ranges)
    constructor(indexable, startIndex) {
        this.indexable = indexable;
        this.index = startIndex || 0;
    }
    advanceBy(n) {
        this.index = script.posMod(this.index + n, this.indexable.length);
        return this.current();
    }
    next() {
        if (this.index !== (this.indexable.length - 1)) {
            return this.advanceBy(1);
        } else {
            return this.current;
        }
    }
    previous() {
        if (this.index !== 0) {
            return this.advanceBy(-1);
        } else {
            return this.current;
        }
    }
    current() {
        return this.indexable[this.index];
    }
};
PrimeGo.WrappingArrayView = class {
    // Array that repeats at each end (e.g. effect selection)
    constructor(indexable, startIndex) {
        this.indexable = indexable;
        this.index = startIndex || 0;
    }
    advanceBy(n) {
        this.index = script.posMod(this.index + n, this.indexable.length);
        return this.current();
    }
    next() {
        return this.advanceBy(1);
    }
    previous() {
        return this.advanceBy(-1);
    }
    current() {
        return this.indexable[this.index];
    }
};

PrimeGo.init = function(_id, _debugging) {
    // Turn off all LEDs
    midi.sendShortMsg(0x90, 0x75, 0x00);

    // Initialize Shift LED
    midi.sendShortMsg(0x9F, 0x08, 0x01);

    // Get position of all components
    midi.sendSysexMsg(initMsg, initMsg.length);

    // Initialize deck objects
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
    const theDeck = this;

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

    //jogwheel
    this.jogWheel = new components.JogWheelBasic({
        deck: script.deckFromGroup(this.currentDeck),
        wheelResolution: 1000,
        alpha: 1/8,
        beta: 1/8/32,
        rpm: 10.6,
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
            if (remainder * 2 > this.wrappingValue) {
                remainder -= this.wrappingValue;
            }
            this.previousPosition = value;
            return remainder;
        },
        jogScale: function(val) {
            //nudgeSensitivity is user-configurable, see top of file.
            return val * nudgeSensitivity;
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

    this.sweepKnob = new components.Pot({
        midi: [],
        group: "[QuickEffectRack1_[Channel" + deckNumber + "]]",
        key: "super1",
    });

    this.sweepA = new components.Button({
        midi: [0x90 + midiChannel - 2, 0x0E],
        group: "[QuickEffectRack1_[Channel" + deckNumber + "]]",
        key: "enabled",
        type: components.Button.prototype.types.toggle,
    });

    //sweepB - Needs ability to select specific quick effects.

    this.headphoneCue = new components.Button({
        midi: [0x90 + midiChannel - 2, 0x0D],
        key: "pfl",
        type: components.Button.prototype.types.toggle,
    });

    this.volumeFader = new components.Pot({
        midi: [0xB0 + midiChannel - 2, 0x0E],
        inKey: "volume",
    });

    this.padGrid = new PrimeGo.PadSection(this, midiChannel - 2);

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


//========== PERFORMANCE PADS ==========//

PrimeGo.PadSection = function(deck, offset) {
    components.ComponentContainer.call(this);
    const theContainer = this;

    // Create component containers for each pad mode
    const modes = new components.ComponentContainer({
        "hotcue": new PrimeGo.WrappingArrayView([new PrimeGo.hotcueMode(deck, offset)], 0),
        "loop": new PrimeGo.WrappingArrayView([new PrimeGo.loopMode(deck, offset), new PrimeGo.autoloopMode(deck, offset)], 0),
        "roll": new PrimeGo.WrappingArrayView([new PrimeGo.rollMode(deck, offset), new PrimeGo.samplerMode(deck, offset)], 0),
    });

    modes.forEachComponent(c => c.disconnect());

    const controlToPadMode = control => {
        // If a pad selector button has multiple modes, go to the first mode
        // by default. Otherwise, go to the next mode in that button's list.
        const nextPadMode = (a) => {
            if (a.indexable.includes(this.currentMode)) {
                return a.next();
            } else {
                a.index = 0;
                return a.current();
            }
        };

        let mode;

        switch (control) {
        case PrimeGo.padMode.HOTCUE:
            mode = nextPadMode(modes.hotcue);
            break;
        case PrimeGo.padMode.LOOP:
            mode = nextPadMode(modes.loop);
            break;
        case PrimeGo.padMode.ROLL:
            mode = nextPadMode(modes.roll);
            break;
        }

        return mode;
    };

    this.offset = offset;

    this.padModeSelectLeds = new components.Component({
        trigger: function() {
            // NOTE: There need to be two `for` blocks here to ensure that
            //       the active mode's LED gets lit last. Otherwise, selecting
            //       a mode when pressing shift will result in that pad mode's
            //       LED being unlit.
            for (const modeLayers of Object.values(modes)) {
                const mode = modeLayers.current();
                if (theContainer.currentMode !== mode) {
                    // Make this LED inactive
                    sendSysexColor(0x02 + offset, mode.ledControl, padOff);
                }
            }
            for (const modeLayers of Object.values(modes)) {
                const mode = modeLayers.current();
                if (theContainer.currentMode === mode) {
                    // Make this LED active
                    sendSysexColor(0x02 + offset, mode.ledControl, mode.modeOn);
                }
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

    // Start in hotcue mode
    this.setPadMode(PrimeGo.padMode.HOTCUE);
};

PrimeGo.PadSection.prototype = Object.create(components.ComponentContainer.prototype);

// Assign performance pads mapped in XML file
PrimeGo.PadSection.prototype.performancePad = function(channel, control, value, status, group) {
    // Convert pad `control` to index value (1 to 8)
    const i = (control - 0x0E);

    this.currentMode.pads[i].input(channel, control, value, status, group);
};

PrimeGo.hotcueMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = PrimeGo.padMode.HOTCUE;
    this.modeOn = padModeA;
    this.pads = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new components.HotcueButton({
            number: i,
            group: deck.currentDeck,
            midi: [0x92 + offset, 0x0E + i],
            sendRGB: function(color_obj) {
                sendSysexColor(0x02 + offset, 0x0e + i, [color_obj.red>>1, color_obj.green>>1, color_obj.blue>>1]);
            },
            outConnect: false,
        });
    }
};
PrimeGo.hotcueMode.prototype = Object.create(components.ComponentContainer.prototype);

PrimeGo.loopMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = PrimeGo.padMode.LOOP;
    this.modeOn = padModeA;
    this.pads = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new components.Button({
            group: deck.currentDeck,
            midi: [0x92 + offset, 0x0E + i],
            outConnect: false,
        });
    }
};
PrimeGo.loopMode.prototype = Object.create(components.ComponentContainer.prototype);

PrimeGo.autoloopMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = PrimeGo.padMode.LOOP;
    this.modeOn = padModeB;
    this.pads = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new components.Button({
            group: deck.currentDeck,
            midi: [0x92 + offset, 0x0E + i],
            outConnect: false,
        });
    }
};
PrimeGo.autoloopMode.prototype = Object.create(components.ComponentContainer.prototype);

PrimeGo.rollMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = PrimeGo.padMode.ROLL;
    this.modeOn = padModeA;
    this.pads = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new components.Button({
            group: deck.currentDeck,
            midi: [0x92 + offset, 0x0E + i],
            outConnect: false,
        });
    }
};
PrimeGo.rollMode.prototype = Object.create(components.ComponentContainer.prototype);

PrimeGo.samplerMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = PrimeGo.padMode.ROLL;
    this.modeOn = padModeB;
    this.pads = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new components.Button({
            group: deck.currentDeck,
            midi: [0x92 + offset, 0x0E + i],
            outConnect: false,
        });
    }
};
PrimeGo.samplerMode.prototype = Object.create(components.ComponentContainer.prototype);
