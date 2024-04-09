// Functions common to all controllers go in this file
/* global print:off
       printObject:off
       stringifyObject:off
       arrayContains:off
       secondstominutes:off
       msecondstominutes:off
       colorCodeToObject:off
       colorCodeFromObject:off
       script:off
       bpm:off
       ButtonState:off
       LedState:off
       Controller:off
       Button:off
       Control:off
       Deck:off
 */

// ----------------- Prototype enhancements ---------------------

// Returns an ASCII byte array for the string
String.prototype.toInt = function() {
    const a = [];
    for (let i = 0; i < this.length; i++) {
        a[i] = this.charCodeAt(i);
    }
    return a;
};

// ----------------- Function overloads ---------------------

/**
 * Prints a message to the terminal and the log file.
 *
 * @param {string} message - The log message.
 * @deprecated Use console.log()/console.warn()/console.debug() instead.
 */

var print = function(message) {
    console.log(message);
};

// eslint-disable-next-line no-unused-vars
var printObject = function(obj, maxdepth) {
    print(stringifyObject(obj, maxdepth));
};


var stringifyObject = function(obj, maxdepth, checked, prefix) {
    if (!maxdepth) {
        maxdepth = 2;
    }
    try {
        return JSON.stringify(obj, null, maxdepth);
    } catch (e) {
        if (!checked) {
            checked = [];
        }
        if (!prefix) {
            prefix = "";
        }
        if (maxdepth > 0 && typeof obj === "object" && obj !== null &&
            Object.getPrototypeOf(obj) !== "" && !arrayContains(checked, obj)) {
            checked.push(obj);
            let output = "{\n";
            for (const property in obj) {
                const value = obj[property];
                if (typeof value === "function") {
                    continue;
                }
                output += prefix + property + ": "
                    + stringifyObject(value, maxdepth - 1, checked, prefix + "  ")
                    + "\n";
            }
            return output + prefix.substr(2) + "}";
        }
    }
    return obj;
};


var arrayContains = function(array, elem) {
    for (let i = 0; i < array.length; i++) {
        if (array[i] === elem) {
            return true;
        }
    }
    return false;
};

// ----------------- Generic functions ---------------------

// eslint-disable-next-line no-unused-vars
var secondstominutes = function(secs) {
    const m = (secs / 60) | 0;

    return (m < 10 ? "0" + m : m)
        + ":"
        + ((secs %= 60) < 10 ? "0" + secs : secs);
};

// eslint-disable-next-line no-unused-vars
var msecondstominutes = function(msecs) {
    const m = (msecs / 60000) | 0;
    msecs %= 60000;
    const secs = (msecs / 1000) | 0;
    msecs %= 1000;
    msecs = Math.round(msecs * 100 / 1000);
    if (msecs === 100) {
        msecs = 99;
    }

    return (m < 10 ? "0" + m : m)
        + ":"
        + (secs < 10 ? "0" + secs : secs)
        + "."
        + (msecs < 10 ? "0" + msecs : msecs);
};

// Converts an object with "red", "green" and "blue" properties (value range
// 0-255) into an RGB color code (e.g. 0xFF0000).
// eslint-disable-next-line no-unused-vars
var colorCodeFromObject = function(color) {
    return ((color.red & 0xFF) << 16 | (color.green & 0xFF) << 8 | (color.blue & 0xFF));
};

// Converts an RGB color code (e.g. 0xFF0000) into an object with "red",
// "green" and "blue" properties (value range 0-255).
// eslint-disable-next-line no-unused-vars
var colorCodeToObject = function(colorCode) {
    return {
        "red": (colorCode >> 16) & 0xFF,
        "green": (colorCode >> 8) & 0xFF,
        "blue": colorCode & 0xFF,
    };
};

/* eslint @typescript-eslint/no-empty-function: "off" */
var script = function() {
};

// ----------------- Mapping constants ---------------------

// Library column value, which can be used to interact with the CO for "[Library] sort_column"
script.LIBRARY_COLUMNS = Object.freeze({
    ARTIST: 1,
    TITLE: 2,
    ALBUM: 3,
    ALBUM_ARTIST: 4,
    YEAR: 5,
    GENRE: 6,
    COMPOSER: 7,
    GROUPING: 8,
    TRACK_NUMBER: 9,
    FILETYPE: 10,
    NATIVE_LOCATION: 11,
    COMMENT: 12,
    DURATION: 13,
    BITRATE: 14,
    BPM: 15,
    REPLAY_GAIN: 16,
    DATETIME_ADDED: 17,
    TIMES_PLAYED: 18,
    RATING: 19,
    KEY: 20,
    PREVIEW: 21,
    COVERART: 22,
    TRACK_COLOR: 30,
    LAST_PLAYED: 31,
});

// DEPRECATED -- use script.midiDebug() instead
script.debug = function(channel, control, value, status, group) {
    print("Warning: script.debug() is deprecated. Use script.midiDebug() instead.");
    script.midiDebug(channel, control, value, status, group);
};

// DEPRECATED -- use script.midiPitch() instead
script.pitch = function(LSB, MSB, status) {
    print("Warning: script.pitch() is deprecated. Use script.midiPitch() instead.");
    return script.midiPitch(LSB, MSB, status);
};

// DEPRECATED -- use script.absoluteLin() instead
script.absoluteSlider = function(group, key, value, low, high, min, max) {
    print("Warning: script.absoluteSlider() is deprecated. Use engine.setValue(group, key, script.absoluteLin(...)) instead.");
    engine.setValue(group, key, script.absoluteLin(value, low, high, min, max));
};

script.midiDebug = function(channel, control, value, status, group) {
    print("Script.midiDebug - channel: 0x" + channel.toString(16) +
        " control: 0x" + control.toString(16) + " value: 0x" + value.toString(16) +
        " status: 0x" + status.toString(16) + " group: " + group);
};

// Returns the deck number of a "ChannelN" or "SamplerN" group
script.deckFromGroup = function(group) {
    let deck = 0;
    if (group === undefined) {
        return undefined;
    }
    if (group.substring(2, 8) === "hannel") {
        // Extract deck number from the group text
        deck = group.substring(8, group.length - 1);
    }
    /*
        else if (group.substring(2,8)=="ampler") {
            // Extract sampler number from the group text
            deck = group.substring(8,group.length-1);
        }
    */
    return parseInt(deck);
};

/* -------- ------------------------------------------------------
     script.bindConnections
   Purpose: Binds multiple controls at once. See an example in Pioneer-DDJ-SB-scripts.js
   Input:   The group whose controls are to be bound and an object
            (controlstToFunctions) where the properties' names are
            controls names and the values are the functions those
            controls will be bound to.
   Output:  none
   -------- ------------------------------------------------------ */
script.bindConnections = function(group, controlsToFunctions, remove) {
    let control;
    remove = (remove === undefined) ? false : remove;

    for (control in controlsToFunctions) {
        engine.connectControl(group, control, controlsToFunctions[control], remove);
        if (!remove) {
            engine.trigger(group, control);
        }
    }
};

/* -------- ------------------------------------------------------
     script.toggleControl
   Purpose: Toggles an engine value
   Input:   Group and control names
   Output:  none
   -------- ------------------------------------------------------ */
script.toggleControl = function(group, control) {
    engine.setValue(group, control, !(engine.getValue(group, control)));
};

/* -------- ------------------------------------------------------
     script.toggleControl
   Purpose: Triggers an engine value and resets it back to 0 after a delay
            This is helpful for mapping encoder turns to controls that are
            represented by buttons in skins so the skin button lights up
            briefly but does not stay lit.
   Input:   Group and control names, delay in milliseconds (optional)
   Output:  none
   -------- ------------------------------------------------------ */
script.triggerControl = function(group, control, delay) {
    if (typeof delay !== "number") {
        delay = 200;
    }
    engine.setValue(group, control, 1);
    engine.beginTimer(delay, () => engine.setValue(group, control, 0), true);
};

/* -------- ------------------------------------------------------
     script.absoluteLin
   Purpose: Maps an absolute linear control value to a linear Mixxx control
            value (like Volume: 0..1)
   Input:   Control value (e.g. a knob,) MixxxControl values for the lowest and
            highest points, lowest knob value, highest knob value
            (Default knob values are standard MIDI 0..127)
   Output:  MixxxControl value corresponding to the knob position
   -------- ------------------------------------------------------ */
script.absoluteLin = function(value, low, high, min, max) {
    if (!min) {
        min = 0;
    }
    if (!max) {
        max = 127;
    }

    if (value <= min) {
        return low;
    } else if (value >= max) {
        return high;
    } else {
        return ((((high - low) / (max - min)) * (value - min)) + low);
    }
};

/* -------- ------------------------------------------------------
     script.absoluteLinInverse
   Purpose: Maps a linear Mixxx control value (like balance: -1..1) to an absolute linear value
            (inverse of the above function)
   Input:   Control value (e.g. a knob,) MixxxControl values for the lowest and
            highest points, lowest knob value, highest knob value
            (Default knob values are standard MIDI 0..127)
   Output:  Linear value corresponding to the knob position
   -------- ------------------------------------------------------ */
script.absoluteLinInverse = function(value, low, high, min, max) {
    if (!min) {
        min = 0;
    }
    if (!max) {
        max = 127;
    }
    const result = (((value - low) * (max - min)) / (high - low)) + min;
    if (result < min) {
        return min;
    } else if (result > max) {
        return max;
    } else {
        return result;
    }
};


/* -------- ------------------------------------------------------
     script.absoluteNonLin
   Purpose: Maps an absolute linear control value to a non-linear Mixxx control
            value (like EQs: 0..1..4)
   Input:   Control value (e.g. a knob,) MixxxControl values for the lowest,
            middle, and highest points, lowest knob value, highest knob value
            (Default knob values are standard MIDI 0..127)
   Output:  MixxxControl value corresponding to the knob position
   -------- ------------------------------------------------------ */
script.absoluteNonLin = function(value, low, mid, high, min, max) {
    if (!min) {
        min = 0;
    }
    if (!max) {
        max = 127;
    }
    const center = (max - min) / 2;
    if (value === center || value === Math.round(center)) {
        return mid;
    } else if (value < center) {
        return low + (value / (center / (mid - low)));
    } else {
        return mid + ((value - center) / (center / (high - mid)));
    }
};

/* -------- ------------------------------------------------------
     script.absoluteNonLinInverse
 Purpose: Maps a non-linear Mixxx control to an absolute linear value (inverse of the above function).
 Helpful for sending MIDI messages to controllers and comparing non-linear Mixxx controls to incoming MIDI values.
 Input:  MixxxControl value; lowest, middle, and highest MixxxControl value;
 bottom of output range, top of output range. (Default output range is standard MIDI 0..127)
 Output: MixxxControl value scaled to output range
 -------- ------------------------------------------------------ */
script.absoluteNonLinInverse = function(value, low, mid, high, min, max) {
    if (!min) {
        min = 0;
    }
    if (!max) {
        max = 127;
    }
    const center = (max - min) / 2;
    let result;

    if (value === mid) {
        return center;
    } else if (value < mid) {
        result = (center / (mid - low)) * (value - low);
    } else {
        result = center + (center / (high - mid)) * (value - mid);
    }

    if (result < min) {
        return min;
    } else if (result > max) {
        return max;
    } else {
        return result;
    }
};

/* -------- ------------------------------------------------------
     script.crossfaderCurve
   Purpose: Adjusts the cross-fader's curve using a hardware control
   Input:   Current value of the hardware control, min and max values for that control
   Output:  none
   -------- ------------------------------------------------------ */
script.crossfaderCurve = function(value, min, max) {
    if (engine.getValue("[Mixer Profile]", "xFaderMode") === 1) {
        // Constant Power
        engine.setValue("[Mixer Profile]", "xFaderCalibration",
            script.absoluteLin(value, 0.5, 0.962, min, max));
    } else {
        // Additive
        engine.setValue("[Mixer Profile]", "xFaderCurve",
            script.absoluteLin(value, 1, 2, min, max));
    }
};

/* -------- ------------------------------------------------------
     script.posMod
   Purpose: Computes the euclidean modulo of m % n. The result is always
            in the range [0, m[
   Input:   dividend `a` and divisor `m` for modulo (a % m)
   Output:  positive remainder
   -------- ------------------------------------------------------ */
script.posMod = function(a, m) {
    return ((a % m) + m) % m;
};

/* -------- ------------------------------------------------------
     script.loopMove
   Purpose: Moves the current loop by the specified number of beats (default 1/2)
            in the specified direction (positive is forwards and is the default)
            If the current loop length is shorter than the requested move distance,
            it's only moved a distance equal to its length.
   Input:   MixxxControl group, direction to move, number of beats to move
   Output:  none
   -------- ------------------------------------------------------ */
script.loopMove = function(group, direction, numberOfBeats) {
    if (!numberOfBeats || numberOfBeats === 0) {
        numberOfBeats = 0.5;
    }

    if (direction < 0) {
        engine.setValue(group, "loop_move", -numberOfBeats);
    } else {
        engine.setValue(group, "loop_move", numberOfBeats);
    }
};

/* -------- ------------------------------------------------------
     script.midiPitch
   Purpose: Takes the value from a little-endian 14-bit MIDI pitch
            wheel message and returns the value for a "rate" (pitch
            slider) Mixxx control
   Input:   Least significant byte, most sig. byte, MIDI status byte
   Output:  Value for a "rate" control, or false if the input MIDI
            message was not a Pitch message (0xE#)
   -------- ------------------------------------------------------ */
// TODO: Is this still useful now that MidiController.cpp properly handles these?
script.midiPitch = function(LSB, MSB, status) {
    if ((status & 0xF0) !== 0xE0) {  // Mask the upper nybble so we can check the opcode regardless of the channel
        print("Script.midiPitch: Error, not a MIDI pitch (0xEn) message: " + status);
        return false;
    }
    const value = (MSB << 7) | LSB;  // Construct the 14-bit number
    // Range is 0x0000..0x3FFF center @ 0x2000, i.e. 0..16383 center @ 8192
    const rate = (value - 8192) / 8191;
    //     print("Script.Pitch: MSB="+MSB+", LSB="+LSB+", value="+value+", rate="+rate);
    return rate;
};

/* -------- ------------------------------------------------------
     script.spinback
   Purpose: wrapper around engine.spinback() that can be directly mapped
            from xml for a spinback effect
            e.g: <key>script.spinback</key>
   Input:   channel, control, value, status, group, factor (optional), start rate (optional)
   Output:  none
   -------- ------------------------------------------------------ */
script.spinback = function(channel, control, value, status, group, factor, rate) {
    // if brake is called without defined factor and rate, reset to defaults
    if (factor === undefined) {
        factor = 1;
    }
    // if brake is called without defined rate, reset to default
    if (rate === undefined) {
        rate = -10;
    }
    // disable on note-off or zero value note/cc
    engine.spinback(
        parseInt(group.substring(8, 9)), ((status & 0xF0) !== 0x80 && value > 0),
        factor, rate);
};

/* -------- ------------------------------------------------------
     script.brake
   Purpose: wrapper around engine.brake() that can be directly mapped
            from xml for a brake effect
            e.g: <key>script.brake</key>
   Input:   channel, control, value, status, group, factor (optional)
   Output:  none
   -------- ------------------------------------------------------ */
script.brake = function(channel, control, value, status, group, factor) {
    // if brake is called without factor defined, reset to default
    if (factor === undefined) {
        factor = 1;
    }
    // disable on note-off or zero value note/cc, use default decay rate '1'
    engine.brake(
        parseInt(group.substring(8, 9)), ((status & 0xF0) !== 0x80 && value > 0),
        factor);
};

/* -------- ------------------------------------------------------
     script.softStart
   Purpose: wrapper around engine.softStart() that can be directly mapped
            from xml to start and accelerate a deck from zero to full rate
            defined by pitch slider, can also interrupt engine.brake()
            e.g: <key>script.softStart</key>
   Input:   channel, control, value, status, group, acceleration factor (optional)
   Output:  none
   -------- ------------------------------------------------------ */
script.softStart = function(channel, control, value, status, group, factor) {
    // if softStart is called without factor defined, reset to default
    if (factor === undefined) {
        factor = 1;
    }
    // disable on note-off or zero value note/cc, use default increase rate '1'
    engine.softStart(
        parseInt(group.substring(8, 9)), ((status & 0xF0) !== 0x80 && value > 0),
        factor);
};

// bpm - Used for tapping the desired BPM for a deck
var bpm = function() {
};

bpm.tapTime = 0.0;
bpm.previousTapDelta = 0.0;
bpm.tap = [];   // Tap sample values

/* -------- ------------------------------------------------------
        bpm.tapButton
   Purpose: Sets the tempo of the track on a deck by tapping the desired beats,
            useful for manually synchronizing a track to an external beat.
            (This only works if the track's detected BPM value is correct.)
            Call this each time the tap button is pressed.
   Input:   Mixxx deck to adjust
   Output:  -
   -------- ------------------------------------------------------ */
bpm.tapButton = function(deck) {
    const now = new Date() / 1000; // Current time in seconds
    const tapDelta = now - bpm.tapTime;
    bpm.tapTime = now;

    // assign tapDelta in cases where the button has not been pressed previously
    if (bpm.tap.length < 1) {
        bpm.previousTapDelta = tapDelta;
    }
    // reset if longer than two seconds between taps
    if (tapDelta > 2.0) {
        bpm.tap = [];
        return;
    }
    // reject occurrences of accidental double or missed taps
    // a tap is considered missed when the delta of this press is 80% longer than the previous one
    // and a tap is considered double when the delta is shorter than 40% of the previous one.
    // these numbers are just guesses that produced good results in practice
    if ((tapDelta > bpm.previousTapDelta * 1.8) || (tapDelta < bpm.previousTapDelta * 0.6)) {
        return;
    }
    bpm.previousTapDelta = tapDelta;
    bpm.tap.push(60 / tapDelta);
    // Keep the last 8 samples for averaging
    if (bpm.tap.length > 8) {
        bpm.tap.shift();
    }
    let sum = 0;
    for (let i = 0; i < bpm.tap.length; i++) {
        sum += bpm.tap[i];
    }
    const average = sum / bpm.tap.length;

    const group = "[Channel" + deck + "]";

    // "bpm" was changed in 1.10 to reflect the *adjusted* bpm, but I presume it
    // was supposed to return the tracks bpm (which it did before the change).
    // "file_bpm" is supposed to return the set BPM of the loaded track of the
    // channel.
    let fRateScale = average / engine.getValue(group, "file_bpm");

    // Adjust the rate:
    fRateScale = (fRateScale - 1.) / engine.getValue(group, "rateRange");

    engine.setValue(
        group, "rate",
        fRateScale * engine.getValue(group, "rate_dir"));
};

// ----------------- Common regular expressions --------------------------
script.samplerRegEx = /^\[Sampler(\d+)\]$/;
script.channelRegEx = /^\[Channel(\d+)\]$/;
script.eqRegEx = /^\[EqualizerRack1_(\[.*\])_Effect1\]$/;
script.quickEffectRegEx = /^\[QuickEffectRack1_(\[.*\])\]$/;
script.effectUnitRegEx = /^\[EffectRack1_EffectUnit(\d+)\]$/;
script.individualEffectRegEx = /^\[EffectRack1_EffectUnit(\d+)_Effect(\d+)\]$/;

// ----------------- Object definitions --------------------------


var ButtonState = {"released": 0x00, "pressed": 0x7F};
// eslint-disable-next-line no-unused-vars
var LedState = {"off": 0x00, "on": 0x7F};

// Controller

var Controller = function() {
    this.group = "[Master]";
    this.Controls = [];
    this.Buttons = [];
};

Controller.prototype.addButton = function(buttonName, button, eventHandler) {
    if (eventHandler) {
        /* eslint @typescript-eslint/no-this-alias: "off" */
        const executionEnvironment = this;
        const handler = function(value) {
            button.state = value;
            executionEnvironment[eventHandler](value);
        };
        button.handler = handler;
    }
    this.Buttons[buttonName] = button;
};

Controller.prototype.setControlValue = function(control, value) {
    this.Controls[control].setValue(this.group, value);
};

// Button

var Button = function(controlId) {
    this.controlId = controlId;
    this.state = ButtonState.released;
};
Button.prototype.handleEvent = function(value) {
    this.handler(value);
};

// Control

var Control = function(mappedFunction, softMode) {
    // These defaults are for MIDI controllers
    this.minInput = 0;
    this.midInput = 0x3F;
    this.maxInput = 0x7F;
    // ----

    this.minOutput = -1.0;
    this.midOutput = 0.0;
    this.maxOutput = 1.0;
    this.mappedFunction = mappedFunction;
    this.softMode = softMode;
    this.maxJump = 10;
};

Control.prototype.setValue = function(group, inputValue) {
    let outputValue = 0;
    if (inputValue <= this.midInput) {
        outputValue = this.minOutput
            + ((inputValue - this.minInput) / (this.midInput - this.minInput))
            * (this.midOutput - this.minOutput);
    } else {
        outputValue = this.midOutput
            + ((inputValue - this.midInput) / (this.maxInput - this.midInput))
            * (this.maxOutput - this.midOutput);
    }
    if (this.softMode) {
        const currentValue = engine.getValue(group, this.mappedFunction);
        let currentRelative = 0.0;
        if (currentValue <= this.midOutput) {
            currentRelative = this.minInput
                + ((currentValue - this.minOutput) / (this.midOutput - this.minOutput))
                * (this.midInput - this.minInput);
        } else {
            currentRelative = this.midInput
                + ((currentValue - this.midOutput) / (this.maxOutput - this.midOutput))
                * (this.maxInput - this.midInput);
        }
        if (inputValue > currentRelative - this.maxJump
            && inputValue < currentRelative + this.maxJump) {
            engine.setValue(group, this.mappedFunction, outputValue);
        }
    } else {
        engine.setValue(group, this.mappedFunction, outputValue);
    }
};

// Deck

var Deck = function(deckNumber, group) {
    this.deckNumber = deckNumber;
    this.group = group;
    this.Buttons = [];
};
Deck.prototype.setControlValue = Controller.prototype.setControlValue;
Deck.prototype.addButton = Controller.prototype.addButton;

// ----------------- END Object definitions ----------------------
