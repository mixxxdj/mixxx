const LEDColors = {
    off: 0,
    red: 4,
    carrot: 8,
    orange: 12,
    honey: 16,
    yellow: 20,
    lime: 24,
    green: 28,
    aqua: 32,
    celeste: 36,
    sky: 40,
    blue: 44,
    purple: 48,
    fuscia: 52,
    magenta: 56,
    azalea: 60,
    salmon: 64,
    white: 68,
};

/*
 * USER CONFIGURABLE SETTINGS
 * Adjust these to your liking
 */

const deckColors = [
    LEDColors.red,
    LEDColors.blue,
    LEDColors.yellow,
    LEDColors.purple,
];

const tempoFaderSoftTakeoverColorLow = LEDColors.white;
const tempoFaderSoftTakeoverColorHigh = LEDColors.green;

// The LEDs only support 16 base colors. Adding 1 in addition to
// the normal 2 for Button.prototype.brightnessOn changes the color
// slightly, so use that get 25 different colors to include the Filter
// button as a 5th effect chain preset selector.
const quickEffectPresetColors = [
    LEDColors.red,
    LEDColors.blue,
    LEDColors.yellow,
    LEDColors.purple,
    LEDColors.white,

    LEDColors.magenta,
    LEDColors.azalea,
    LEDColors.salmon,
    LEDColors.red + 1,

    LEDColors.sky,
    LEDColors.celeste,
    LEDColors.fuscia,
    LEDColors.blue + 1,

    LEDColors.carrot,
    LEDColors.orange,
    LEDColors.honey,
    LEDColors.yellow + 1,

    LEDColors.lime,
    LEDColors.aqua,
    LEDColors.green,
    LEDColors.purple + 1,

    LEDColors.magenta + 1,
    LEDColors.azalea + 1,
    LEDColors.salmon + 1,
    LEDColors.fuscia + 1,
];

// assign samplers to the crossfader on startup
const samplerCrossfaderAssign = true;

/*
 * HID packet parsing library
 */

class HIDInputPacket {
    constructor(reportId) {
        this.reportId = reportId;
        this.fields = [];
    }

    registerCallback(callback, byteOffset, bitOffset, bitLength, signed) {
        if (typeof callback !== "function") {
            throw Error("callback must be a function");
        }

        if (byteOffset === undefined || typeof byteOffset !== "number" || !Number.isInteger(byteOffset)) {
            throw Error("byteOffset must be 0 or a positive integer");
        }

        if (bitOffset === undefined) {
            bitOffset = 0;
        }
        if (typeof bitOffset !== "number" || bitOffset < 0 || !Number.isInteger(bitOffset)) {
            throw Error("bitOffset must be 0 or a positive integer");
        }

        if (bitLength === undefined) {
            bitLength = 1;
        }
        if (typeof bitLength !== "number" || bitLength < 1 || !Number.isInteger(bitOffset) || bitLength > 32) {
            throw Error("bitLength must be an integer between 1 and 32");
        }

        if (signed === undefined) {
            signed = false;
        }

        const field = {
            callback: callback,
            byteOffset: byteOffset,
            bitOffset: bitOffset,
            bitLength: bitLength,
            oldData: 0
        };
        this.fields.push(field);

        return {
            disconnect: () => {
                this.fields = this.fields.filter((element) => {
                    return element !== field;
                });
            }
        };
    }

    handleInput(byteArray) {
        const view = new DataView(byteArray);
        if (view.getUint8(0) !== this.reportId) {
            return;
        }

        for (const field of this.fields) {
            const numBytes = Math.ceil(field.bitLength / 8);
            let data;

            // Little endianness is specified by the HID standard.
            // The HID standard allows signed integers as well, but I am not aware
            // of any HID DJ controllers which use signed integers.
            if (numBytes === 1) {
                data = view.getUint8(field.byteOffset);
            } else if (numBytes === 2) {
                data = view.getUint16(field.byteOffset, true);
            } else if (numBytes === 3) {
                data = view.getUint32(field.byteOffset, true) >>> 8;
            } else if (numBytes === 4) {
                data = view.getUint32(field.byteOffset, true);
            } else {
                throw Error("field bitLength must be between 1 and 32");
            }

            // The >>> 0 is required for 32 bit unsigned ints to not magically turn negative
            // because all Numbers are really 32 bit signed floats. Because JavaScript.
            data = ((data >> field.bitOffset) & (2 ** field.bitLength - 1)) >>> 0;

            if (field.oldData !== data) {
                field.callback(data);
                field.oldData = data;
            }
        }
    }
}

class HIDOutputPacket {
    constructor(reportId, length) {
        this.reportId = reportId;
        this.data = Array(length).fill(0);
    }
    send() {
        controller.send(this.data, null, this.reportId);
    }
}

/*
 * Components library
 */

class Component {
    constructor(options) {
        Object.assign(this, options);
        if (options !== undefined && typeof options.key === "string") {
            this.inKey = options.key;
            this.outKey = options.key;
        }
        if (this.unshift !== undefined && typeof this.unshift === "function") {
            this.unshift();
        }
        this.shifted = false;
        if (this.input !== undefined && typeof this.input === "function"
            && this.inPacket !== undefined && this.inPacket instanceof HIDInputPacket) {
            this.inConnect();
        }
        this.outConnections = [];
        this.outConnect();
    }
    inConnect(callback) {
        if (this.inByte === undefined
          || this.inBit === undefined
          || this.inBitLength === undefined
          || this.inPacket === undefined) {
            return;
        }
        if (typeof callback === "function") {
            this.input = callback;
        }
        this.inConnection = this.inPacket.registerCallback(this.input.bind(this), this.inByte, this.inBit, this.inBitLength);
    }
    inDisconnect() {
        if (this.inConnection !== undefined) {
            this.inConnection.disconnect();
        }
    }
    send(value) {
        if (this.outPacket !== undefined && this.outByte !== undefined) {
            this.outPacket.data[this.outByte] = value;
            this.outPacket.send();
        }
    }
    output(value) {
        this.send(value);
    }
    outConnect() {
        if (this.outKey !== undefined && this.group !== undefined) {
            this.outConnections[0] = engine.makeConnection(this.group, this.outKey, this.output.bind(this));
        }
    }
    outDisconnect() {
        for (const connection of this.outConnections) {
            connection.disconnect();
        }
    }
    outTrigger() {
        for (const connection of this.outConnections) {
            connection.trigger();
        }
    }
}

class ComponentContainer {
    constructor() {}
    *[Symbol.iterator]() {
    // can't use for...of here because it would create an infinite loop
        for (const property in this) {
            if (Object.prototype.hasOwnProperty.call(this, property)) {
                const obj = this[property];
                if (obj instanceof Component) {
                    yield obj;
                } else if (obj instanceof ComponentContainer) {
                    for (const nestedComponent of obj) {
                        yield nestedComponent;
                    }
                } else if (Array.isArray(obj)) {
                    for (const objectInArray of obj) {
                        if (objectInArray instanceof Component) {
                            yield objectInArray;
                        } else if (objectInArray instanceof ComponentContainer) {
                            for (const doublyNestedComponent of objectInArray) {
                                yield doublyNestedComponent;
                            }
                        }
                    }
                }
            }
        }
    }
    reconnectComponents(callback) {
        for (const component of this) {
            if (component.outDisconnect !== undefined && typeof component.outDisconnect === "function") {
                component.outDisconnect();
            }
            if (callback !== undefined && typeof callback === "function") {
                callback.call(this, component);
            }
            if (component.outConnect !== undefined && typeof component.outConnect === "function") {
                component.outConnect();
            }
            component.outTrigger();
        }
    }
    unshift() {
        for (const component of this) {
            if (component.unshift !== undefined && typeof component.unshift === "function") {
                component.unshift();
            }
            component.shifted = false;
        }
    }
    shift() {
        for (const component of this) {
            if (component.shift !== undefined && typeof component.shift === "function") {
                component.shift();
            }
            component.shifted = true;
        }
    }
}

/* eslint no-redeclare: "off" */
class Deck extends ComponentContainer {
    constructor(decks, colors) {
        super();
        if (typeof decks === "number") {
            this.group = Deck.groupForNumber(decks);
        } else if (Array.isArray(decks)) {
            this.decks = decks;
            this.currentDeckNumber = decks[0];
            this.group = Deck.groupForNumber(decks[0]);
        }
        if (colors !== undefined && Array.isArray(colors)) {
            this.groupsToColors = {};
            let index = 0;
            for (const deck of this.decks) {
                this.groupsToColors[Deck.groupForNumber(deck)] = colors[index];
                index++;
            }
            this.color = colors[0];
        }
    }
    toggleDeck() {
        if (this.decks === undefined) {
            throw Error("toggleDeck can only be used with Decks constructed with an Array of deck numbers, for example [1, 3]");
        }

        const currentDeckIndex = this.decks.indexOf(this.currentDeckNumber);
        let newDeckIndex = currentDeckIndex + 1;
        if (currentDeckIndex >= this.decks.length) {
            newDeckIndex = 0;
        }

        this.switchDeck(Deck.groupForNumber(this.decks[newDeckIndex]));
    }
    switchDeck(newGroup) {
        this.group = newGroup;
        this.color = this.groupsToColors[newGroup];
        this.reconnectComponents(function(component) {
            if (component.group === undefined
                  || component.group.search(script.channelRegEx) !== -1) {
                component.group = newGroup;
            } else if (component.group.search(script.eqRegEx) !== -1) {
                component.group = "[EqualizerRack1_" + newGroup + "_Effect1]";
            } else if (component.group.search(script.quickEffectRegEx) !== -1) {
                component.group = "[QuickEffectRack1_" + newGroup + "]";
            }

            component.color = this.groupsToColors[newGroup];
        });
    }
    static groupForNumber(deckNumber) {
        return "[Channel" + deckNumber + "]";
    }
}

class Button extends Component {
    constructor(options) {
        super(options);
        this.off = 0;
        if (this.longPressTimeOut === undefined) {
            this.longPressTimeOut = 225; // milliseconds
        }
        if (this.inBitLength === undefined) {
            this.inBitLength = 1;
        }
    }
    output(value) {
        const brightness = (value > 0) ? this.brightnessOn : this.brightnessOff;
        this.send(this.color + brightness);
    }
}

class PushButton extends Button {
    constructor(options) {
        super(options);
    }
    input(pressed) {
        engine.setValue(this.group, this.inKey, pressed);
    }
}

class ToggleButton extends Button {
    constructor(options) {
        super(options);
    }
    input(pressed) {
        if (pressed) {
            script.toggleControl(this.group, this.inKey);
        }
    }
}

class PowerWindowButton extends Button {
    constructor(options) {
        super(options);
        this.isLongPressed = false;
        this.longPressTimer = 0;
    }
    input(pressed) {
        if (pressed) {
            script.toggleControl(this.group, this.inKey);
            this.longPressTimer = engine.beginTimer(this.longPressTimeOut, () => {
                this.isLongPressed = true;
                this.longPressTimer = 0;
            }, true);
        } else {
            if (this.isLongPressed) {
                script.toggleControl(this.group, this.inKey);
            }
            if (this.longPressTimer !== 0) {
                engine.stopTimer(this.longPressTimer);
            }
            this.longPressTimer = 0;
            this.isLongPressed = false;
        }
    }
}

class PlayButton extends ToggleButton {
    constructor(options) {
        super(options);
        this.inKey = "play";
        this.outKey = "play_indicator";
        this.outConnect();
    }
}

class CueButton extends PushButton {
    constructor(options) {
        super(options);
        this.outKey = "cue_indicator";
        this.outConnect();
    }
    unshift() {
        this.inKey = "cue_default";
    }
    shift() {
        this.inKey = "start_stop";
    }
}

class Encoder extends Component {
    constructor(options) {
        super(options);
        this.lastValue = null;
    }
    isRightTurn(value) {
        // detect wrap around
        const oldValue = this.lastValue;
        this.lastValue = value;
        if (oldValue === this.max && value === 0) {
            return true;
        }
        if (oldValue === 0 && value === this.max) {
            return false;
        }
        return value > oldValue;
    }
}

class HotcueButton extends PushButton {
    constructor(options) {
        super(options);
        if (this.number === undefined || !Number.isInteger(this.number) || this.number < 1 || this.number > 32) {
            throw Error("HotcueButton must have a number property of an integer between 1 and 32");
        }
        this.outKey = "hotcue_" + this.number + "_enabled";
        this.colorKey = "hotcue_" + this.number + "_color";
        this.outConnect();
    }
    unshift() {
        this.inKey = "hotcue_" + this.number + "_activate";
    }
    shift() {
        this.inKey = "hotcue_" + this.number + "_clear";
    }
    output(value) {
        if (value) {
            this.send(this.color + this.brightnessOn);
        } else {
            this.send(0);
        }
    }
    outConnect() {
        if (undefined !== this.group) {
            this.outConnections[0] = engine.makeConnection(this.group, this.outKey, this.output.bind(this));
            this.outConnections[1] = engine.makeConnection(this.group, this.colorKey, (colorCode) => {
                this.color = this.colorMap.getValueForNearestColor(colorCode);
                this.output(engine.getValue(this.group, this.outKey));
            });
        }
    }
}

class SamplerButton extends Button {
    constructor(options) {
        super(options);
        if (this.number === undefined || !Number.isInteger(this.number) || this.number < 1 || this.number > 64) {
            throw Error("SamplerButton must have a number property of an integer between 1 and 64");
        }
        this.group = "[Sampler" + this.number + "]";
        this.outConnect();
    }
    input(pressed) {
        if (!this.shifted) {
            if (pressed) {
                if (engine.getValue(this.group, "track_loaded") === 0) {
                    engine.setValue(this.group, "LoadSelectedTrack", 1);
                } else {
                    engine.setValue(this.group, "cue_gotoandplay", 1);
                }
            }
        } else {
            if (pressed) {
                if (engine.getValue(this.group, "play") === 1) {
                    engine.setValue(this.group, "play", 0);
                } else {
                    engine.setValue(this.group, "eject", 1);
                }
            } else {
                if (engine.getValue(this.group, "play") === 0) {
                    engine.setValue(this.group, "eject", 0);
                }
            }
        }
    }
    // This function is connected to multiple Controls, so don't use the value passed in as a parameter.
    output() {
        if (engine.getValue(this.group, "track_loaded")) {
            if (engine.getValue(this.group, "play")) {
                this.send(this.color + this.brightnessOn);
            } else {
                this.send(this.color + this.brightnessOff);
            }
        } else {
            this.send(0);
        }
    }
    outConnect() {
        if (undefined !== this.group) {
            this.outConnections[0] = engine.makeConnection(this.group, "play", this.output.bind(this));
            this.outConnections[1] = engine.makeConnection(this.group, "track_loaded", this.output.bind(this));
        }
    }
}

class IntroOutroButton extends PushButton {
    constructor(options) {
        super(options);
        if (this.cueBaseName === undefined || typeof this.cueBaseName !== "string") {
            throw Error("must specify cueBaseName as intro_start, intro_end, outro_start, or outro_end");
        }
        this.outKey = this.cueBaseName + "_enabled";
        this.outConnect();
    }
    unshift() {
        this.inKey = this.cueBaseName + "_activate";
    }
    shift() {
        this.inKey = this.cueBaseName + "_clear";
    }
    output(value) {
        if (value) {
            this.send(this.color + this.brightnessOn);
        } else {
            this.send(0);
        }
    }
}

class Pot extends Component {
    constructor(options) {
        super(options);
        this.hardwarePosition = null;
    }
    input(value) {
        const receivingFirstValue = this.hardwarePosition === null;
        this.hardwarePosition = value / this.max;
        engine.setParameter(this.group, this.inKey, this.hardwarePosition);
        if (receivingFirstValue) {
            engine.softTakeover(this.group, this.inKey, true);
        }
    }
    outDisconnect() {
        if (this.hardwarePosition !== null) {
            engine.softTakeover(this.group, this.inKey, true);
        }
        engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
    }
}

/*
 * Kontrol S4 Mk3 hardware-specific constants
 */

Pot.prototype.max = 2**12 - 1;
Pot.prototype.inBit = 0;
Pot.prototype.inBitLength = 16;

Encoder.prototype.inBitLength = 4;

// valid range 0 - 3, but 3 makes some colors appear whitish
Button.prototype.brightnessOff = 0;
Button.prototype.brightnessOn = 2;
Button.prototype.colorMap = new ColorMapper({
    0xCC0000: LEDColors.red,
    0xCC5E00: LEDColors.carrot,
    0xCC7800: LEDColors.orange,
    0xCC9200: LEDColors.honey,

    0xCCCC00: LEDColors.yellow,
    0x81CC00: LEDColors.lime,
    0x00CC00: LEDColors.green,
    0x00CC49: LEDColors.aqua,

    0x00CCCC: LEDColors.celeste,
    0x0091CC: LEDColors.sky,
    0x0000CC: LEDColors.blue,
    0xCC00CC: LEDColors.purple,

    0xCC0091: LEDColors.fuscia,
    0xCC0079: LEDColors.magenta,
    0xCC477E: LEDColors.azalea,
    0xCC4761: LEDColors.salmon,

    0xCCCCCC: LEDColors.white,
});

const wheelRelativeMax = 2**16 - 1;
const wheelAbsoluteMax = 2879;

const wheelTimerMax = 2**32 - 1;
const wheelTimerTicksPerSecond = 100000000;

const baseRevolutionsPerMinute = 33 + 1/3;
const baseRevolutionsPerSecond = baseRevolutionsPerMinute / 60;
const wheelTicksPerTimerTicksToRevolutionsPerSecond = wheelTimerTicksPerSecond / wheelAbsoluteMax;

const wheelLEDmodes = {
    off: 0,
    dimFlash: 1,
    spot: 2,
    ringFlash: 3,
    dimSpot: 4,
    individuallyAddressable: 5, // set byte 4 to 0 and set byes 8 - 40 to color values
};

const wheelModes = {
    jog: 0,
    vinyl: 1,
    motor: 2,
};

// tracks state across input packets
let wheelTimer = null;
// This is a global variable so the S4Mk3Deck Components have access
// to it and it is guaranteed to be calculated before processing
// input for the Components.
let wheelTimerDelta = 0;

/*
 * Kontrol S4 Mk3 hardware specific mapping logic
 */

// used for buttons whose LEDs only support a single color
// Don't use dim colors for these because they are hard to tell apart
// from bright colors.
const uncoloredButtonOutput = function(value) {
    if (value) {
        this.send(127);
    } else {
        this.send(0);
    }
};

class S4Mk3EffectUnit extends ComponentContainer {
    constructor(unitNumber, inPackets, outPacket, io) {
        super();
        this.group = "[EffectRack1_EffectUnit" + unitNumber + "]";

        this.mixKnob = new Pot({
            inKey: "mix",
            group: this.group,
            inPacket: inPackets[2],
            inByte: io.mixKnob.inByte,
        });

        this.knobs = [];
        this.buttons = [];
        for (const index of [0, 1, 2]) {
            const effectGroup = "[EffectRack1_EffectUnit" + unitNumber + "_Effect" + (index + 1) + "]";
            this.knobs[index] = new Pot({
                inKey: "meta",
                group: effectGroup,
                inPacket: inPackets[2],
                inByte: io.knobs[index].inByte,
            });
            this.buttons[index] = new PowerWindowButton({
                key: "enabled",
                group: effectGroup,
                output: uncoloredButtonOutput,
                inPacket: inPackets[1],
                inByte: io.buttons[index].inByte,
                inBit: io.buttons[index].inBit,
                outByte: io.buttons[index].outByte,
                outPacket: outPacket,
            });
        }

        for (const component of this) {
            component.inConnect();
            component.outConnect();
            component.outTrigger();
        }
    }
}

class S4Mk3Deck extends Deck {
    constructor(decks, colors, inPackets, outPacket, io) {
        super(decks, colors);

        this.playButton = new PlayButton({
            output: uncoloredButtonOutput
        });

        this.cueButton = new CueButton();

        const rateRanges = [0.04, 0.06, 0.08, 0.10, 0.16, 0.24, 0.5, 0.9];
        this.syncMasterButton = new ToggleButton({
            key: "sync_leader",
            input: function(pressed) {
                if (pressed) {
                    if (!this.shifted) {
                        script.toggleControl(this.group, this.inKey);
                    } else {
                        // It is possible for the rateRange to be set to a value
                        // that is not in the rateRanges Array, so find the nearest
                        // value in rateRanges.
                        const currentRateRange = engine.getValue(this.group, "rateRange");
                        let previousDiff = null;
                        let newRateRange = rateRanges[0];
                        for (let i = 0; i < rateRanges.length - 1; i++) {
                            const currentDiff = Math.abs(rateRanges[i] - currentRateRange);
                            if (currentDiff < previousDiff || previousDiff === null) {
                                newRateRange = rateRanges[i + 1];
                            }
                            previousDiff = currentDiff;
                        }
                        engine.setValue(this.group, "rateRange", newRateRange);
                    }
                }
            },
        });
        this.syncButton = new ToggleButton({
            key: "sync_enabled",
            input: function(pressed) {
                if (pressed) {
                    if (!this.shifted) {
                        script.toggleControl(this.group, this.inKey);
                        engine.softTakeover(this.group, "rate", true);
                    } else {
                        // It is possible for the rateRange to be set to a value
                        // that is not in the rateRanges Array, so find the nearest
                        // value in rateRanges.
                        const currentRateRange = engine.getValue(this.group, "rateRange");
                        let previousDiff = null;
                        let newRateRange = rateRanges[0];
                        for (let i = rateRanges.length - 1; i > 0; i--) {
                            const currentDiff = Math.abs(rateRanges[i] - currentRateRange);
                            if (currentDiff < previousDiff || previousDiff === null) {
                                newRateRange = rateRanges[i - 1];
                            }
                            previousDiff = currentDiff;
                        }
                        engine.setValue(this.group, "rateRange", newRateRange);
                    }
                }
            },
        });
        this.tempoFader = new Pot({
            inKey: "rate",
        });
        this.tempoFaderLED = new Component({
            outKey: "rate",
            centered: false,
            toleranceWindow: 0.001,
            tempoFader: this.tempoFader,
            output: function(value) {
                if (this.tempoFader.hardwarePosition === null) {
                    return;
                }

                const parameterValue = engine.getParameter(this.group, this.outKey);
                const diffFromHardware = parameterValue - this.tempoFader.hardwarePosition;
                if (diffFromHardware > this.toleranceWindow) {
                    this.send(tempoFaderSoftTakeoverColorHigh + Button.prototype.brightnessOn);
                    return;
                } else if (diffFromHardware < (-1 * this.toleranceWindow)) {
                    this.send(tempoFaderSoftTakeoverColorLow + Button.prototype.brightnessOn);
                    return;
                }

                const oldCentered = this.centered;
                if (Math.abs(value) < 0.001) {
                    this.send(this.color + Button.prototype.brightnessOn);
                    // round to precisely 0
                    engine.setValue(this.group, "rate", 0);
                } else {
                    this.send(0);
                }
            }
        });

        this.reverseButton = new PushButton({
            key: "reverseroll",
            output: uncoloredButtonOutput,
        });
        this.fluxButton = new PushButton({
            key: "slip_enabled",
            output: uncoloredButtonOutput,
        });
        this.gridButton = new PushButton({
            key: "beats_translate_curpos",
        });

        this.deckButtonLeft = new Button({
            deck: this,
            input: function(value) {
                if (value) {
                    this.deck.switchDeck(Deck.groupForNumber(decks[0]));
                    this.outPacket.data[io.deckButtonOutputByteOffset] = colors[0] + this.brightnessOn;
                    // turn off the other deck selection button's LED
                    this.outPacket.data[io.deckButtonOutputByteOffset+1] = 0;
                    this.outPacket.send();
                }
            },
        });
        this.deckButtonRight = new Button({
            deck: this,
            input: function(value) {
                if (value) {
                    this.deck.switchDeck(Deck.groupForNumber(decks[1]));
                    // turn off the other deck selection button's LED
                    this.outPacket.data[io.deckButtonOutputByteOffset] = 0;
                    this.outPacket.data[io.deckButtonOutputByteOffset+1] = colors[1] + this.brightnessOn;
                    this.outPacket.send();
                }
            },
        });

        // set deck selection button LEDs
        outPacket.data[io.deckButtonOutputByteOffset] = colors[0] + Button.prototype.brightnessOn;
        outPacket.data[io.deckButtonOutputByteOffset+1] = 0;
        outPacket.send();

        this.shiftButton = new PushButton({
            deck: this,
            input: function(pressed) {
                if (pressed) {
                    this.deck.shift();
                    // This button only has one color.
                    this.send(LEDColors.white + this.brightnessOn);
                } else {
                    this.deck.unshift();
                    this.send(LEDColors.white + this.brightnessOff);
                }
            },
        });

        this.leftEncoder = new Encoder({
            deck: this,
            input: function(value) {
                const right = this.isRightTurn(value);
                if (!this.shifted) {
                    if (!this.deck.leftEncoderPress.pressed) {
                        if (right) {
                            script.triggerControl(this.group, "beatjump_forward");
                        } else {
                            script.triggerControl(this.group, "beatjump_backward");
                        }
                    } else {
                        let beatjumpSize = engine.getValue(this.group, "beatjump_size");
                        if (right) {
                            beatjumpSize *= 2;
                        } else {
                            beatjumpSize /= 2;
                        }
                        engine.setValue(this.group, "beatjump_size", beatjumpSize);
                    }
                } else {
                    // FIXME: temporary hack until jog wheels are working
                    if (right) {
                        engine.setValue(this.group, "jog", 3);
                        // script.triggerControl(this.group, "pitch_up_small");
                    } else {
                        engine.setValue(this.group, "jog", -3);
                        // script.triggerControl(this.group, "pitch_down_small");
                    }
                }
            }
        });
        this.leftEncoderPress = new PushButton({
            input: function(pressed) {
                this.pressed = pressed;
                if (pressed) {
                    script.toggleControl(this.group, "pitch_adjust_set_default");
                }
            },
        });

        this.rightEncoder = new Encoder({
            input: function(value) {
                const right = this.isRightTurn(value);
                if (!this.shifted) {
                    if (right) {
                        script.triggerControl(this.group, "loop_double");
                    } else {
                        script.triggerControl(this.group, "loop_halve");
                    }
                } else {
                    if (right) {
                        script.triggerControl(this.group, "beatjump_1_forward");
                    } else {
                        script.triggerControl(this.group, "beatjump_1_backward");
                    }
                }
            }
        });
        this.rightEncoderPress = new PushButton({
            input: function(pressed) {
                if (!pressed) {
                    return;
                }
                const loopEnabled = engine.getValue(this.group, "loop_enabled");
                if (!this.shifted) {
                    script.triggerControl(this.group, "beatloop_activate");
                } else {
                    if (loopEnabled) {
                        script.triggerControl(this.group, "reloop_andstop");
                    } else {
                        script.triggerControl(this.group, "reloop_toggle");
                    }
                }
            },
        });

        this.libraryEncoder = new Encoder({
            input: function(value) {
                const right = this.isRightTurn(value);
                const previewPlaying = engine.getValue("[PreviewDeck1]", "play");
                if (previewPlaying) {
                    if (right) {
                        script.triggerControl("[PreviewDeck1]", "beatjump_16_forward");
                    } else {
                        script.triggerControl("[PreviewDeck1]", "beatjump_16_backward");
                    }
                } else {
                    engine.setValue("[Library]", "MoveVertical", right ? 1 : -1);
                }
            }
        });
        this.libraryEncoderPress = new ToggleButton({
            inKey: "LoadSelectedTrack"
        });
        this.libraryPlayButton = new PushButton({
            group: "[PreviewDeck1]",
            input: function(pressed) {
                if (pressed) {
                    if (engine.getValue(this.group, "play")) {
                        engine.setValue(this.group, "play", 0);
                    } else {
                        script.triggerControl(this.group, "LoadSelectedTrackAndPlay");
                    }
                }
            },
            outKey: "play",
        });
        this.libraryStarButton = new PushButton({
            group: "[Library]",
            key: "MoveFocusForward",
        });
        this.libraryPlaylistButton = new PushButton({
            group: "[Library]",
            key: "MoveFocusBackward",
        });
        this.libraryViewButton = new ToggleButton({
            group: "[Master]",
            inKey: "maximize_library",
        });

        this.pads = Array(8).fill(new Component());
        const defaultPadLayer = [
            new IntroOutroButton({
                cueBaseName: "intro_start",
            }),
            new IntroOutroButton({
                cueBaseName: "intro_end",
            }),
            new IntroOutroButton({
                cueBaseName: "outro_start",
            }),
            new IntroOutroButton({
                cueBaseName: "outro_end",
            }),
            new HotcueButton({
                number: 1
            }),
            new HotcueButton({
                number: 2
            }),
            new HotcueButton({
                number: 3
            }),
            new HotcueButton({
                number: 4
            })
        ];
        const hotcuePage2 = Array(8).fill({});
        const hotcuePage3 = Array(8).fill({});
        const samplerPage1 = Array(8).fill({});
        const samplerPage2 = Array(8).fill({});
        let i = 0;
        /* eslint no-unused-vars: "off" */
        for (const pad of hotcuePage2) {
            // start with hotcue 5; hotcues 1-4 are in defaultPadLayer
            hotcuePage2[i] = new HotcueButton({number: i + 1});
            hotcuePage3[i] = new HotcueButton({number: i + 13});
            let samplerNumber = i + 1;
            if (samplerNumber > 4) {
                samplerNumber += 4;
            }
            if (decks[0] > 1) {
                samplerNumber += 4;
            }
            samplerPage1[i] = new SamplerButton({number: samplerNumber});
            samplerPage2[i] = new SamplerButton({number: samplerNumber + 16});
            if (samplerCrossfaderAssign) {
                engine.setValue(
                    "[Sampler" + samplerNumber + "]",
                    "orientation",
                    (decks[0] === 1) ? 0 : 2
                );
            }
            i++;
        }

        const switchPadLayer = (deck, newLayer) => {
            let index = 0;
            for (let pad of deck.pads) {
                pad.outDisconnect();
                pad.inDisconnect();

                pad = newLayer[index];
                Object.assign(pad, io.pads[index]);
                if (!(pad instanceof HotcueButton)) {
                    pad.color = deck.color;
                }
                // don't change the group of SamplerButtons
                if (!(pad instanceof SamplerButton)) {
                    pad.group = deck.group;
                }
                if (pad.inPacket === undefined) {
                    pad.inPacket = inPackets[1];
                }
                pad.outPacket = outPacket;
                pad.inConnect();
                pad.outConnect();
                pad.outTrigger();
                deck.pads[index] = pad;
                index++;
            }
        };

        this.padLayers = {
            defaultLayer: 0,
            hotcuePage2: 1,
            hotcuePage3: 2,
            samplerPage1: 3,
            samplerPage2: 4,
        };
        switchPadLayer(this, defaultPadLayer);
        this.currentPadLayer = this.padLayers.defaultLayer;

        this.hotcuePadModeButton = new Button({
            deck: this,
            input: function(pressed) {
                if (!this.shifted) {
                    if (pressed) {
                        if (this.deck.currentPadLayer !== this.deck.padLayers.hotcuePage2) {
                            switchPadLayer(this.deck, hotcuePage2);
                            this.deck.currentPadLayer = this.deck.padLayers.hotcuePage2;
                        } else {
                            switchPadLayer(this.deck, defaultPadLayer);
                            this.deck.currentPadLayer = this.deck.padLayers.defaultLayer;
                        }
                        this.deck.lightPadMode();
                    }
                } else {
                    engine.setValue(this.deck.group, "loop_in", pressed);
                }
            },
            // make sure loop_in gets reset to 0 if shift is released before this button
            unshift: function() {
                if (engine.getValue(this.deck.group, "loop_in") === 1) {
                    engine.setValue(this.deck.group, "loop_in", 0);
                }
            },
            // hack to switch the LED color when changing decks
            outTrigger: function() {
                this.deck.lightPadMode();
            }
        });
        this.recordPadModeButton = new Button({
            deck: this,
            input: function(pressed) {
                if (!this.shifted) {
                    if (pressed) {
                        if (this.deck.currentPadLayer !== this.deck.padLayers.hotcuePage3) {
                            switchPadLayer(this.deck, hotcuePage3);
                            this.deck.currentPadLayer = this.deck.padLayers.hotcuePage3;
                        } else {
                            switchPadLayer(this.deck, defaultPadLayer);
                            this.deck.currentPadLayer = this.deck.padLayers.defaultLayer;
                        }
                        this.deck.lightPadMode();
                    }
                } else {
                    engine.setValue(this.deck.group, "loop_out", pressed);
                }
            },
            // make sure loop_out gets reset to 0 if shift is released before this button
            unshift: function() {
                if (engine.getValue(this.deck.group, "loop_out") === 1) {
                    engine.setValue(this.deck.group, "loop_out", 0);
                }
            }
        });
        this.samplesPadModeButton = new Button({
            deck: this,
            input: function(pressed) {
                if (pressed) {
                    if (this.deck.currentPadLayer !== this.deck.padLayers.samplerPage1) {
                        switchPadLayer(this.deck, samplerPage1);
                        this.deck.currentPadLayer = this.deck.padLayers.samplerPage1;
                    } else {
                        switchPadLayer(this.deck, defaultPadLayer);
                        this.deck.currentPadLayer = this.deck.padLayers.defaultLayer;
                    }
                    this.deck.lightPadMode();
                }
            },
        });
        this.mutePadModeButton = new Button({
            deck: this,
            input: function(pressed) {
                if (pressed) {
                    if (this.deck.currentPadLayer !== this.deck.padLayers.samplerPage2) {
                        switchPadLayer(this.deck, samplerPage2);
                        this.deck.currentPadLayer = this.deck.padLayers.samplerPage2;
                    } else {
                        switchPadLayer(this.deck, defaultPadLayer);
                        this.deck.currentPadLayer = this.deck.padLayers.defaultLayer;
                    }
                    this.deck.lightPadMode();
                }
            },
        });

        this.stemsPadModeButton = new ToggleButton({
            key: "keylock",
        });

        this.lightPadMode = function() {
            const hotcuePadModeLEDOn = this.currentPadLayer === this.padLayers.hotcuePage2;
            this.hotcuePadModeButton.send(this.color + (hotcuePadModeLEDOn ? this.brightnessOn : this.brightnessOff));

            // unfortunately the other pad mode buttons only have one LED color
            const recordPadModeLEDOn = this.currentPadLayer === this.padLayers.hotcuePage3;
            this.recordPadModeButton.send(recordPadModeLEDOn ? 127 : 0);

            const samplesPadModeLEDOn = this.currentPadLayer === this.padLayers.samplerPage1;
            this.samplesPadModeButton.send(samplesPadModeLEDOn ? 127 : 0);

            const mutePadModeButtonLEDOn = this.currentPadLayer === this.padLayers.samplerPage2;
            this.mutePadModeButton.send(mutePadModeButtonLEDOn ? 127 : 0);
        };

        this.wheelMode = wheelModes.vinyl;
        let motorWindDownTimer = 0;
        const motorWindDownTimerCallback = () => {
            engine.stopTimer(motorWindDownTimer);
            motorWindDownTimer = 0;
        };
        const motorWindDownMilliseconds = 900;
        this.turntableButton = new Button({
            deck: this,
            input: function(press) {
                if (press) {
                    if (this.deck.wheelMode === wheelModes.motor) {
                        this.deck.wheelMode = wheelModes.vinyl;
                        motorWindDownTimer = engine.beginTimer(motorWindDownMilliseconds, motorWindDownTimerCallback, true);
                        //                         engine.setValue(this.group, "scratch2_enable", false);
                    } else {
                        this.deck.wheelMode = wheelModes.motor;
                        //                         engine.setValue(this.group, "scratch2_enable", true);
                    }
                    this.outTrigger();
                }
            },
            outTrigger: function() {
                const motorOn = this.deck.wheelMode === wheelModes.motor;
                this.send(this.color + (motorOn ? this.brightnessOn : this.brightnessOff));
                const vinylModeOn = this.deck.wheelMode === wheelModes.vinyl;
                this.deck.jogButton.send(this.color + (vinylModeOn ? this.brightnessOn : this.brightnessOff));
            },
        });
        this.jogButton = new Button({
            deck: this,
            input: function(press) {
                if (press) {
                    if (this.deck.wheelMode === wheelModes.vinyl) {
                        this.deck.wheelMode = wheelModes.jog;
                    } else {
                        if (this.deck.wheelMode === wheelModes.motor) {
                            motorWindDownTimer = engine.beginTimer(motorWindDownMilliseconds, motorWindDownTimerCallback, true);
                        }
                        this.deck.wheelMode = wheelModes.vinyl;
                    }
                    engine.setValue(this.group, "scratch2_enable", false);
                    this.outTrigger();
                }
            },
            outTrigger: function() {
                const vinylOn = this.deck.wheelMode === wheelModes.vinyl;
                this.send(this.color + (vinylOn ? this.brightnessOn : this.brightnessOff));
                const motorOn = this.deck.wheelMode === wheelModes.motor;
                this.deck.turntableButton.send(this.color + (motorOn ? this.brightnessOn : this.brightnessOff));
            },
        });

        this.wheelTouch = new Button({
            touched: false,
            deck: this,
            input: function(touched) {
                this.touched = touched;
                if (this.deck.wheelMode !== wheelModes.jog) {
                    if (touched) {
                        engine.setValue(this.group, "scratch2_enable", true);
                    } else {
                        // The wheel keeps spinning
                        engine.beginTimer(600, () => {
                            engine.setValue(this.group, "scratch2_enable", false);
                        }, true);
                    }
                }
            },
        });

        // The relative and absolute position inputs have the same resolution but direction
        // cannot be determined reliably with the absolute position because it is easily
        // possible to spin the wheel fast enough that it spins more than half a revolution
        // between input packets. So there is no need to process the absolution position
        // at all; the relative position is sufficient.
        this.wheelRelative = new Component({
            oldValue: null,
            deck: this,
            input: function(value) {
                const oldValue = this.oldValue;
                this.oldValue = value;
                let diff = value - oldValue;
                if (diff === 0 || oldValue === null || motorWindDownTimer !== 0) {
                    return;
                }

                if (diff > wheelRelativeMax / 2) {
                    diff = (wheelRelativeMax - value + oldValue) * -1;
                } else if (diff < -1 * (wheelRelativeMax / 2)) {
                    diff = wheelRelativeMax - oldValue + value;
                }

                const wheelVelocity = diff / wheelTimerDelta * wheelTicksPerTimerTicksToRevolutionsPerSecond;
                //                 if (this.group === "[Channel1]") {
                //                     console.log(value + "\t" + diff + "\t" + wheelTimerDelta + "\t" + wheelVelocity + "\t" + wheelVelocity / baseRevolutionsPerSecond);
                //                 }
                if (engine.getValue(this.group, "scratch2_enable")) {
                    engine.setValue(this.group, "scratch2", wheelVelocity / baseRevolutionsPerSecond);
                } else {
                    if (this.deck.wheelMode === wheelModes.motor
                        || (this.deck.wheelMode === wheelModes.jog && this.deck.wheelTouch.touched)
                    ) {
                        return;
                    }
                    engine.setValue(this.group, "jog", wheelVelocity * 4);
                }
            },
        });

        this.wheelLED = new Component({
            outKey: "playposition",
            output: function(fractionOfTrack) {
                const durationSeconds = engine.getValue(this.group, "duration");
                const positionSeconds = fractionOfTrack * durationSeconds;
                const revolutions = positionSeconds * baseRevolutionsPerSecond;
                const fractionalRevolution = revolutions - Math.floor(revolutions);
                const LEDposition = fractionalRevolution * wheelAbsoluteMax;

                const wheelOutput = Array(40).fill(0);
                wheelOutput[0] = decks[0] - 1;
                wheelOutput[1] = wheelLEDmodes.spot;
                wheelOutput[2] = LEDposition & (2**8 - 1);
                wheelOutput[3] = LEDposition >> 8;
                wheelOutput[4] = this.color + Button.prototype.brightnessOn;
                controller.send(wheelOutput, null, 50, true);
            }
        });

        for (const property in this) {
            if (Object.prototype.hasOwnProperty.call(this, property)) {
                const component = this[property];
                if (component instanceof Component) {
                    Object.assign(component, io[property]);
                    if (component.inPacket === undefined) {
                        component.inPacket = inPackets[1];
                    }
                    component.outPacket = outPacket;
                    if (component.group === undefined) {
                        component.group = this.group;
                    }
                    if (component.color === undefined) {
                        component.color = this.color;
                    }
                    if (component instanceof Encoder) {
                        component.max = 2**component.inBitLength - 1;
                    }
                    component.inConnect();
                    component.outConnect();
                    component.outTrigger();
                }
            }
        }
        this.shiftButton.send(LEDColors.white + this.brightnessOff);
    }
}

class S4Mk3MixerColumn extends ComponentContainer {
    constructor(group, inPackets, outPacket, io) {
        super();

        this.group = group;

        this.gain = new Pot({
            inKey: "pregain",
        });
        this.eqHigh = new Pot({
            group: "[EqualizerRack1_" + group + "_Effect1]",
            inKey: "parameter3",
        });
        this.eqMid = new Pot({
            group: "[EqualizerRack1_" + group + "_Effect1]",
            inKey: "parameter2",
        });
        this.eqLow = new Pot({
            group: "[EqualizerRack1_" + group + "_Effect1]",
            inKey: "parameter1",
        });
        this.quickEffectKnob = new Pot({
            group: "[QuickEffectRack1_" + group + "]",
            inKey: "super1",
        });
        this.volume = new Pot({
            inKey: "volume",
        });

        this.pfl = new ToggleButton({
            inKey: "pfl",
            outKey: "pfl",
            output: uncoloredButtonOutput,
        });

        this.effectUnit1Assign = new PowerWindowButton({
            group: "[EffectRack1_EffectUnit1]",
            key: "group_" + this.group + "_enable",
            output: uncoloredButtonOutput,
        });

        this.effectUnit2Assign = new PowerWindowButton({
            group: "[EffectRack1_EffectUnit2]",
            key: "group_" + this.group + "_enable",
            output: uncoloredButtonOutput,
        });

        // FIXME: Why is output not working for these?
        this.saveGain = new PushButton({
            key: "update_replaygain_from_pregain",
            output: uncoloredButtonOutput,
        });

        this.crossfaderSwitch = new Component({
            inBitLength: 2,
            input: function(value) {
                if (value === 0) {
                    engine.setValue(this.group, "orientation", 2);
                } else if (value === 1) {
                    engine.setValue(this.group, "orientation", 1);
                } else if (value === 2) {
                    engine.setValue(this.group, "orientation", 0);
                }
            },
        });

        for (const property in this) {
            if (Object.prototype.hasOwnProperty.call(this, property)) {
                const component = this[property];
                if (component instanceof Component) {
                    Object.assign(component, io[property]);
                    if (component instanceof Pot) {
                        component.inPacket = inPackets[2];
                    } else {
                        component.inPacket = inPackets[1];
                    }
                    component.outPacket = outPacket;

                    if (component.group === undefined) {
                        component.group = this.group;
                    }

                    component.inConnect();
                    component.outConnect();
                    component.outTrigger();
                }
            }
        }
    }
}

const packetToBinaryString = (data) => {
    let string = "";
    for (const byte of data) {
        if (byte === 0) {
            // special case because Math.log(0) === Infinity
            string = string + "0".repeat(8) + ",";
        } else {
            const numOfZeroes = 7 - Math.floor(Math.log(byte) / Math.log(2));
            string = string + "0".repeat(numOfZeroes) + byte.toString(2) + ",";
        }
    }
    // remove trailing comma
    return string.slice(0, -1);
};

class S4MK3 {
    constructor() {
        if (engine.getValue("[Master]", "num_samplers") < 32) {
            engine.setValue("[Master]", "num_samplers", 32);
        }

        this.inPackets = [];
        this.inPackets[1] = new HIDInputPacket(1);
        this.inPackets[2] = new HIDInputPacket(2);
        this.inPackets[3] = new HIDInputPacket(3);

        this.outPackets = [];
        this.outPackets[128] = new HIDOutputPacket(128, 94);

        this.effectUnit1 = new S4Mk3EffectUnit(1, this.inPackets, this.outPackets[128],
            {
                mixKnob: {inByte: 31},
                knobs: [
                    {inByte: 33},
                    {inByte: 35},
                    {inByte: 37},
                ],
                buttons: [
                    {inByte: 2, inBit: 7, outByte: 63},
                    {inByte: 2, inBit: 3, outByte: 64},
                    {inByte: 2, inBit: 2, outByte: 65},
                ],
            }
        );
        this.effectUnit2 = new S4Mk3EffectUnit(2, this.inPackets, this.outPackets[128],
            {
                mixKnob: {inByte: 71},
                knobs: [
                    {inByte: 73},
                    {inByte: 75},
                    {inByte: 77},
                ],
                buttons: [
                    {inByte: 10, inBit: 5, outByte: 74},
                    {inByte: 10, inBit: 6, outByte: 75},
                    {inByte: 10, inBit: 7, outByte: 76},
                ],
            }
        );

        // There is no consistent offset between the left and right deck,
        // so every single components' IO needs to be specified individually
        // for both decks.
        this.leftDeck = new S4Mk3Deck(
            [1, 3], [deckColors[0], deckColors[2]],
            this.inPackets, this.outPackets[128],
            {
                playButton: {inByte: 5, inBit: 0, outByte: 55},
                cueButton: {inByte: 5, inBit: 1, outByte: 8},
                syncButton: {inByte: 6, inBit: 7, outByte: 14},
                syncMasterButton: {inByte: 1, inBit: 0, outByte: 15},
                hotcuePadModeButton: {inByte: 5, inBit: 2, outByte: 9},
                recordPadModeButton: {inByte: 5, inBit: 3, outByte: 56},
                samplesPadModeButton: {inByte: 5, inBit: 4, outByte: 57},
                mutePadModeButton: {inByte: 5, inBit: 5, outByte: 58},
                stemsPadModeButton: {inByte: 6, inBit: 0, outByte: 10},
                deckButtonLeft: {inByte: 6, inBit: 2},
                deckButtonRight: {inByte: 6, inBit: 3},
                deckButtonOutputByteOffset: 12,
                tempoFaderLED: {outByte: 11},
                shiftButton: {inByte: 6, inBit: 1, outByte: 59},
                leftEncoder: {inByte: 20, inBit: 0},
                leftEncoderPress: {inByte: 7, inBit: 2},
                rightEncoder: {inByte: 20, inBit: 4},
                rightEncoderPress: {inByte: 7, inBit: 5},
                libraryEncoder: {inByte: 21, inBit: 0},
                libraryEncoderPress: {inByte: 1, inBit: 1},
                turntableButton: {inByte: 6, inBit: 5, outByte: 17},
                jogButton: {inByte: 6, inBit: 4, outByte: 16},
                gridButton: {inByte: 6, inBit: 6, outByte: 18},
                reverseButton: {inByte: 2, inBit: 4, outByte: 60},
                fluxButton: {inByte: 2, inBit: 5, outByte: 61},
                libraryPlayButton: {inByte: 1, inBit: 5, outByte: 22},
                libraryStarButton: {inByte: 1, inBit: 4, outByte: 21},
                libraryPlaylistButton: {inByte: 2, inBit: 1, outByte: 20},
                libraryViewButton: {inByte: 2, inBit: 0, outByte: 19},
                pads: [
                    {inByte: 4, inBit: 5, outByte: 0},
                    {inByte: 4, inBit: 4, outByte: 1},
                    {inByte: 4, inBit: 7, outByte: 2},
                    {inByte: 4, inBit: 6, outByte: 3},

                    {inByte: 4, inBit: 3, outByte: 4},
                    {inByte: 4, inBit: 2, outByte: 5},
                    {inByte: 4, inBit: 1, outByte: 6},
                    {inByte: 4, inBit: 0, outByte: 7},
                ],
                tempoFader: {inByte: 13, inBit: 0, inBitLength: 16, inPacket: this.inPackets[2]},
                wheelRelative: {inByte: 12, inBit: 0, inBitLength: 16, inPacket: this.inPackets[3]},
                wheelAbsolute: {inByte: 16, inBit: 0, inBitLength: 16, inPacket: this.inPackets[3]},
                wheelTouch: {inByte: 17, inBit: 4},
            }
        );

        this.rightDeck = new S4Mk3Deck(
            [2, 4], [deckColors[1], deckColors[3]],
            this.inPackets, this.outPackets[128],
            {
                playButton: {inByte: 13, inBit: 0, outByte: 66},
                cueButton: {inByte: 15, inBit: 5, outByte: 31},
                syncButton: {inByte: 15, inBit: 4, outByte: 37},
                syncMasterButton: {inByte: 11, inBit: 0, outByte: 38},
                hotcuePadModeButton: {inByte: 13, inBit: 2, outByte: 32},
                recordPadModeButton: {inByte: 13, inBit: 3, outByte: 67},
                samplesPadModeButton: {inByte: 13, inBit: 4, outByte: 68},
                mutePadModeButton: {inByte: 13, inBit: 5, outByte: 69},
                stemsPadModeButton: {inByte: 13, inBit: 1, outByte: 33},
                deckButtonLeft: {inByte: 15, inBit: 2},
                deckButtonRight: {inByte: 15, inBit: 3},
                deckButtonOutputByteOffset: 35,
                tempoFaderLED: {outByte: 34},
                shiftButton: {inByte: 15, inBit: 1, outByte: 70},
                leftEncoder: {inByte: 21, inBit: 4},
                leftEncoderPress: {inByte: 16, inBit: 5},
                rightEncoder: {inByte: 22, inBit: 0},
                rightEncoderPress: {inByte: 16, inBit: 2},
                libraryEncoder: {inByte: 22, inBit: 4},
                libraryEncoderPress: {inByte: 11, inBit: 1},
                turntableButton: {inByte: 15, inBit: 6, outByte: 40},
                jogButton: {inByte: 15, inBit: 0, outByte: 39},
                gridButton: {inByte: 15, inBit: 7, outByte: 41},
                reverseButton: {inByte: 11, inBit: 4, outByte: 71},
                fluxButton: {inByte: 11, inBit: 5, outByte: 72},
                libraryPlayButton: {inByte: 10, inBit: 2, outByte: 45},
                libraryStarButton: {inByte: 10, inBit: 1, outByte: 44},
                libraryPlaylistButton: {inByte: 10, inBit: 3, outByte: 43},
                libraryViewButton: {inByte: 10, inBit: 0, outByte: 42},
                pads: [
                    {inByte: 14, inBit: 5, outByte: 23},
                    {inByte: 14, inBit: 4, outByte: 24},
                    {inByte: 14, inBit: 7, outByte: 25},
                    {inByte: 14, inBit: 6, outByte: 26},

                    {inByte: 14, inBit: 3, outByte: 27},
                    {inByte: 14, inBit: 2, outByte: 28},
                    {inByte: 14, inBit: 1, outByte: 29},
                    {inByte: 14, inBit: 0, outByte: 30},
                ],
                tempoFader: {inByte: 11, inBit: 0, inBitLength: 16, inPacket: this.inPackets[2]},
                wheelRelative: {inByte: 40, inBit: 0, inBitLength: 16, inPacket: this.inPackets[3]},
                wheelAbsolute: {inByte: 44, inBit: 0, inBitLength: 16, inPacket: this.inPackets[3]},
                wheelTouch: {inByte: 17, inBit: 5},
            }
        );

        this.mixerColumnDeck1 = new S4Mk3MixerColumn("[Channel1]", this.inPackets, this.outPackets[128],
            {
                saveGain: {inByte: 12, inBit: 0, outByte: 80},
                effectUnit1Assign: {inByte: 3, inBit: 3, outByte: 78},
                effectUnit2Assign: {inByte: 3, inBit: 4, outByte: 79},
                gain: {inByte: 17},
                eqHigh: {inByte: 45},
                eqMid: {inByte: 47},
                eqLow: {inByte: 49},
                quickEffectKnob: {inByte: 65},
                quickEffectButton: {},
                volume: {inByte: 3},
                pfl: {inByte: 8, inBit: 3, outByte: 77},
                crossfaderSwitch: {inByte: 18, inBit: 4},
            }
        );
        this.mixerColumnDeck2 = new S4Mk3MixerColumn("[Channel2]", this.inPackets, this.outPackets[128],
            {
                saveGain: {inByte: 12, inBit: 1, outByte: 84},
                effectUnit1Assign: {inByte: 3, inBit: 5, outByte: 82},
                effectUnit2Assign: {inByte: 3, inBit: 6, outByte: 83},
                gain: {inByte: 19},
                eqHigh: {inByte: 51},
                eqMid: {inByte: 53},
                eqLow: {inByte: 55},
                quickEffectKnob: {inByte: 67},
                volume: {inByte: 5},
                pfl: {inByte: 8, inBit: 6, outByte: 81},
                crossfaderSwitch: {inByte: 18, inBit: 2},
            }
        );
        this.mixerColumnDeck3 = new S4Mk3MixerColumn("[Channel3]", this.inPackets, this.outPackets[128],
            {
                saveGain: {inByte: 3, inBit: 1, outByte: 88},
                effectUnit1Assign: {inByte: 3, inBit: 0, outByte: 86},
                effectUnit2Assign: {inByte: 3, inBit: 2, outByte: 87},
                gain: {inByte: 15},
                eqHigh: {inByte: 39},
                eqMid: {inByte: 41},
                eqLow: {inByte: 43},
                quickEffectKnob: {inByte: 63},
                volume: {inByte: 7},
                pfl: {inByte: 8, inBit: 2, outByte: 85},
                crossfaderSwitch: {inByte: 18, inBit: 6},
            }
        );
        this.mixerColumnDeck4 = new S4Mk3MixerColumn("[Channel4]", this.inPackets, this.outPackets[128],
            {
                saveGain: {inByte: 12, inBit: 2, outByte: 92},
                effectUnit1Assign: {inByte: 3, inBit: 7, outByte: 90},
                effectUnit2Assign: {inByte: 12, inBit: 7, outByte: 91},
                gain: {inByte: 21},
                eqHigh: {inByte: 57},
                eqMid: {inByte: 59},
                eqLow: {inByte: 61},
                quickEffectKnob: {inByte: 69},
                volume: {inByte: 9},
                pfl: {inByte: 8, inBit: 7, outByte: 89},
                crossfaderSwitch: {inByte: 18, inBit: 0},
            }
        );

        // The interaction between the FX SELECT buttons and the QuickEffect enable buttons is rather complex.
        // It is easier to have this separate from the S4Mk3MixerColumn class and the FX SELECT buttons are not
        // really in the mixer columns.
        const mixer = new ComponentContainer();
        mixer.firstPressedFxSelector = null;
        mixer.secondPressedFxSelector = null;
        const calculatePresetNumber = function() {
            if (mixer.firstPressedFxSelector === mixer.secondPressedFxSelector || mixer.secondPressedFxSelector === null) {
                return mixer.firstPressedFxSelector;
            }
            let presetNumber = 5 + (4 * (mixer.firstPressedFxSelector - 1)) + mixer.secondPressedFxSelector;
            if (mixer.secondPressedFxSelector > mixer.firstPressedFxSelector) {
                presetNumber--;
            }
            return presetNumber;
        };
        mixer.comboSelected = false;
        const resetFxSelectorColors = () => {
            const packet = this.outPackets[128];
            for (const selector of [1, 2, 3, 4, 5]) {
                packet.data[49 + selector] = quickEffectPresetColors[selector - 1] + Button.prototype.brightnessOn;
            }
            packet.send();
        };
        const fxSelectInput = function(pressed) {
            if (pressed) {
                if (mixer.firstPressedFxSelector === null) {
                    mixer.firstPressedFxSelector = this.number;
                    for (const selector of [1, 2, 3, 4, 5]) {
                        if (selector !== this.number) {
                            let presetNumber = 5 + (4 * (mixer.firstPressedFxSelector - 1)) + selector;
                            if (selector > this.number) {
                                presetNumber--;
                            }
                            this.outPacket.data[49 + selector] = quickEffectPresetColors[presetNumber - 1] + this.brightnessOn;
                        }
                    }
                    this.outPacket.send();
                } else {
                    mixer.secondPressedFxSelector = this.number;
                }
            } else {
            // After a second selector was released, avoid loading a different preset when
            // releasing the first pressed selector.
                if (mixer.comboSelected && this.number === mixer.firstPressedFxSelector) {
                    mixer.comboSelected = false;
                    mixer.firstPressedFxSelector = null;
                    mixer.secondPressedFxSelector = null;
                    resetFxSelectorColors();
                    return;
                }
                // If mixer.firstPressedFxSelector === null, it was reset by the input handler for
                // a QuickEffect enable button to load the preset for only one deck.
                if (mixer.firstPressedFxSelector !== null) {
                    for (const deck of [1, 2, 3, 4]) {
                        engine.setValue("[QuickEffectRack1_[Channel" + deck + "]]", "loaded_chain_preset", calculatePresetNumber());
                    }
                }
                if (mixer.firstPressedFxSelector === this.number) {
                    mixer.firstPressedFxSelector = null;
                    resetFxSelectorColors();
                }
                if (mixer.secondPressedFxSelector !== null) {
                    mixer.comboSelected = true;
                }
                mixer.secondPressedFxSelector = null;
            }
        };
        mixer.fxSelect1 = new Button({
            inByte: 9,
            inBit: 5,
            number: 1,
            input: fxSelectInput,
        });
        mixer.fxSelect2 = new Button({
            inByte: 9,
            inBit: 1,
            number: 2,
            input: fxSelectInput,
        });
        mixer.fxSelect3 = new Button({
            inByte: 9,
            inBit: 6,
            number: 3,
            input: fxSelectInput,
        });
        mixer.fxSelect4 = new Button({
            inByte: 9,
            inBit: 0,
            number: 4,
            input: fxSelectInput,
        });
        mixer.fxSelectFilter = new Button({
            inByte: 9,
            inBit: 7,
            number: 5,
            input: fxSelectInput,
        });

        const quickEffectButton = class extends Button {
            constructor(options) {
                super(options);
                if (this.number === undefined || !Number.isInteger(this.number) || this.number < 1) {
                    throw Error("number attribute must be an integer >= 1");
                }
                this.group = "[QuickEffectRack1_[Channel" + this.number + "]]";
                this.outConnect();
                this.isLongPressed = false;
                this.longPressTimer = 0;
            }
            input(pressed) {
                if (mixer.firstPressedFxSelector === null) {
                    if (pressed) {
                        script.toggleControl(this.group, "enabled");
                        this.longPressTimer = engine.beginTimer(this.longPressTimeOut, () => {
                            this.isLongPressed = true;
                            this.longPressTimer = 0;
                        }, true);
                    } else {
                        if (this.isLongPressed) {
                            script.toggleControl(this.group, "enabled");
                        }
                        if (this.longPressTimer !== 0) {
                            engine.stopTimer(this.longPressTimer);
                        }
                        this.longPressTimer = 0;
                        this.isLongPressed = false;
                    }
                } else {
                    if (pressed) {
                        const presetNumber = calculatePresetNumber();
                        this.color = quickEffectPresetColors[presetNumber - 1];
                        engine.setValue(this.group, "loaded_chain_preset", presetNumber);
                        mixer.firstPressedFxSelector = null;
                        mixer.secondPressedFxSelector = null;
                        resetFxSelectorColors();
                    }
                }
            }
            output(enabled) {
                if (enabled) {
                    this.send(this.color + this.brightnessOn);
                } else {
                    // It is easy to mistake the dim state for the bright state, so turn
                    // the LED fully off.
                    this.send(0);
                }
            }
            presetLoaded(presetNumber) {
                this.color = quickEffectPresetColors[presetNumber - 1];
                this.outConnections[1].trigger();
            }
            outConnect() {
                if (this.group !== undefined) {
                    this.outConnections[0] = engine.makeConnection(this.group, "loaded_chain_preset", this.presetLoaded.bind(this));
                    this.outConnections[1] = engine.makeConnection(this.group, "enabled", this.output.bind(this));
                }
            }
        };
        mixer.quickEffectButton1 = new quickEffectButton({
            number: 1,
            inByte: 8,
            inBit: 0,
            outByte: 46
        });
        mixer.quickEffectButton2 = new quickEffectButton({
            number: 2,
            inByte: 8,
            inBit: 5,
            outByte: 47
        });
        mixer.quickEffectButton3 = new quickEffectButton({
            number: 3,
            inByte: 8,
            inBit: 1,
            outByte: 48
        });
        mixer.quickEffectButton4 = new quickEffectButton({
            number: 4,
            inByte: 8,
            inBit: 4,
            outByte: 49
        });
        resetFxSelectorColors();

        mixer.quantizeButton = new Button({
            input: function(pressed) {
                if (pressed) {
                    this.globalQuantizeOn = !this.globalQuantizeOn;
                    for (let i = 1; i <= 4; i++) {
                        engine.setValue("[Channel" + i + "]", "quantize", this.globalQuantizeOn);
                    }
                    this.send(this.globalQuantizeOn ? 127 : 0);
                }
            },
            globalQuantizeOn: false,
            inByte: 12,
            inBit: 6,
            outByte: 93,
        });

        mixer.crossfader = new Pot({
            group: "[Master]",
            inKey: "crossfader",
            inByte: 1,
            inPacket: this.inPackets[2],
        });
        mixer.crossfaderCurveSwitch = new Component({
            inByte: 19,
            inBit: 0,
            inBitLength: 2,
            input: function(value) {
                switch (value) {
                case 0x00:  // Picnic Bench / Fast Cut
                    engine.setValue("[Mixer Profile]", "xFaderMode", 0);
                    engine.setValue("[Mixer Profile]", "xFaderCalibration", 0.9);
                    engine.setValue("[Mixer Profile]", "xFaderCurve", 7.0);
                    break;
                case 0x01:  // Constant Power
                    engine.setValue("[Mixer Profile]", "xFaderMode", 1);
                    engine.setValue("[Mixer Profile]", "xFaderCalibration", 0.3);
                    engine.setValue("[Mixer Profile]", "xFaderCurve", 0.6);
                    break;
                case 0x02: // Additive
                    engine.setValue("[Mixer Profile]", "xFaderMode", 0);
                    engine.setValue("[Mixer Profile]", "xFaderCalibration", 0.4);
                    engine.setValue("[Mixer Profile]", "xFaderCurve", 0.9);
                }
            },
        });

        for (const component of mixer) {
            if (component.inPacket === undefined) {
                component.inPacket = this.inPackets[1];
            }
            component.outPacket = this.outPackets[128];
            component.inConnect();
            component.outConnect();
            component.outTrigger();
        }

        let lightQuantizeButton = true;
        for (let i = 1; i <= 4; i++) {
            if (!engine.getValue("[Channel" + i + "]", "quantize")) {
                lightQuantizeButton = false;
            }
        }
        mixer.quantizeButton.send(lightQuantizeButton ? 127 : 0);
        mixer.quantizeButton.globalQuantizeOn = lightQuantizeButton;

        /* eslint no-unused-vars: "off" */
        const meterConnection = engine.makeConnection("[Master]", "guiTick50ms", function(_value) {
            const deckMeters = Array(78).fill(0);
            // Each column has 14 segments, but treat the top one specially for the clip indicator.
            const deckSegments = 13;
            for (let deckNum = 1; deckNum <= 4; deckNum++) {
                const deckGroup = "[Channel" + deckNum + "]";
                const deckLevel = engine.getValue(deckGroup, "VuMeter");
                const columnBaseIndex = (deckNum - 1) * (deckSegments + 2);
                const scaledLevel = deckLevel * deckSegments;
                const segmentsToLightFully = Math.floor(scaledLevel);
                const partialSegmentValue = scaledLevel - segmentsToLightFully;
                if (segmentsToLightFully > 0) {
                // There are 3 brightness levels per segment: off, dim, and full.
                    for (let i = 0; i <= segmentsToLightFully; i++) {
                        deckMeters[columnBaseIndex + i] = 127;
                    }
                    if (partialSegmentValue > 0.5 && segmentsToLightFully < deckSegments) {
                        deckMeters[columnBaseIndex + segmentsToLightFully + 1] = 125;
                    }
                }
                if (engine.getValue(deckGroup, "PeakIndicator")) {
                    deckMeters[columnBaseIndex + deckSegments + 1] = 127;
                }
            }
            // There are more bytes in the packet which seem like they should be for the main
            // mix meters, but setting those bytes does not do anything, except for lighting
            // the clip lights on the main mix meters.
            controller.send(deckMeters, null, 129);
        });

        const motorTimer = engine.beginTimer(20, () => {
            const baseRate = 6068;
            let velocityLeft = 0;
            let velocityRight = 0;
            const S4Mk3 = this;
            if (this.leftDeck.wheelMode === wheelModes.motor
                  && engine.getValue(this.leftDeck.group, "play")) {
                velocityLeft = baseRate * engine.getValue(S4Mk3.leftDeck.group, "rate_ratio");
            }
            if (this.rightDeck.wheelMode === wheelModes.motor
                  && engine.getValue(this.rightDeck.group, "play")) {
                velocityRight = baseRate * engine.getValue(S4Mk3.rightDeck.group, "rate_ratio");
            }
            // byte 2 > 127 rotates backward
            const motor = [1, 32, 1, velocityLeft & (2**8 - 1), velocityLeft >> 8,
                1, 32, 1, velocityRight & (2**8 - 1), velocityRight >> 8];
            controller.send(motor, null, 49, true);
        });
    }
    incomingData(data) {
        const reportId = data[0];
        if (reportId === 1) {
            this.inPackets[1].handleInput(data.buffer);
        } else if (reportId === 2) {
            this.inPackets[2].handleInput(data.buffer);
            // The master volume, booth volume, headphone mix, and headphone volume knobs
            // control the controller's audio interface in hardware, so they are not mapped.
        } else if (reportId === 3) {
            // The 32 bit unsigned ints at bytes 8 and 36 always have exactly the same value,
            // so only process one of them. This must be processed before the wheel positions.
            const oldWheelTimer = wheelTimer;
            const view = new DataView(data.buffer);
            wheelTimer = view.getUint32(8, true);
            // Processing first value; no previous value to compare with.
            if (oldWheelTimer === null) {
                return;
            }
            wheelTimerDelta = wheelTimer - oldWheelTimer;
            if (wheelTimerDelta < 0) {
                wheelTimerDelta += wheelTimerMax;
            }

            this.leftDeck.wheelRelative.input(view.getUint16(12, true));
            this.rightDeck.wheelRelative.input(view.getUint16(40, true));
        }
    }
    init() {
        // sending these magic packets is required for the jog wheel LEDs to work
        const wheelLEDinitPacket = Array(26).fill(0);
        wheelLEDinitPacket[1] = 1;
        wheelLEDinitPacket[2] = 3;
        controller.send(wheelLEDinitPacket, null, 48);
        wheelLEDinitPacket[0] = 1;
        // hack around https://github.com/mixxxdj/mixxx/issues/10828
        engine.beginTimer(35, () => { controller.send(wheelLEDinitPacket, null, 48); }, true);

        // get state of knobs and faders
        this.incomingData(new Uint8Array(controller.getInputReport(2)));
    }
    shutdown() {
        // button LEDs
        controller.send(new Array(94).fill(0), null, 128);

        // meter LEDs
        controller.send(new Array(78).fill(0), null, 129);

        const wheelOutput = Array(40).fill(0);
        // left wheel LEDs
        controller.send(wheelOutput, null, 50);
        // right wheel LEDs
        wheelOutput[0] = 1;
        controller.send(wheelOutput, null, 50);
    }
}

/* eslint no-unused-vars: "off", no-var: "off" */
var TraktorS4MK3 = new S4MK3();
