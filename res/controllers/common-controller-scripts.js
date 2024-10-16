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
 */

// ----------------- Prototype enhancements ---------------------

/**
 * Converts the string to an array of ASCII values.
 * @returns {number[]} An array of ASCII values corresponding to the string's characters.
 */
// @ts-ignore
String.prototype.toInt = function() {
    return Array.from(this, char => char.charCodeAt(0));
};

// ----------------- Function overloads ---------------------

/**
 * Prints a message to the terminal and the log file.
 * @param {string} message The log message.
 * @deprecated Use console.log()/console.warn()/console.debug() instead.
 */

// eslint-disable-next-line no-unused-vars
const print = function(message) {
    console.log(message);
};

/**
 * Checks if an array contains a specific element.
 * @param {Array} array The array to search.
 * @param {*} elem The element to search for in the array.
 * @returns {boolean} True if the element is found in the array, false otherwise.
 * @deprecated Use array.includes(elem) instead.
 */
const arrayContains = function(array, elem) {
    return array.includes(elem);
};

/**
 * Convert an object to a string representation with a specified maximum depth.
 * @param {Object} obj The object to stringify.
 * @param {number} maxdepth The maximum depth to traverse the object.
 * @param {Array} [checked] An array to keep track of already checked objects to avoid circular references.
 * @param {string} [prefix] A prefix used for indentation purposes, aiding in the readability of the output.
 * @returns {string} The stringified object or the original object if it cannot be stringified.
 */
const stringifyObject = function(obj, maxdepth = 2, checked = [], prefix ="") {
    try {
        return JSON.stringify(obj, null, maxdepth);
    } catch (e) {
        if (maxdepth > 0 && typeof obj === "object" && obj !== null &&
            Object.getPrototypeOf(obj) !== "" && !arrayContains(checked, obj)) {
            checked.push(obj);
            let output = "{\n";
            for (const property in obj) {
                const value = obj[property];
                if (typeof value === "function") { continue; }
                output += `${prefix + property  }: ${
                    stringifyObject(value, maxdepth - 1, checked, `${prefix  }  `)
                }\n`;
            }
            return `${output + prefix.substr(2)  }}`;
        }
    }
    return obj;
};

/**
 * Logs a stringified representation of an object to the console with a specified maximum depth.
 * @param {Object} obj The object to be logged.
 * @param {number} maxdepth The maximum depth to which the object should be stringified.
 */
// eslint-disable-next-line no-unused-vars
const printObject = function(obj, maxdepth) {
    console.log(stringifyObject(obj, maxdepth));
};

// ----------------- Generic functions ---------------------

/**
 * Converts seconds into a minutes:seconds format string.
 * @param {number} secs The number of seconds to convert.
 * @returns {string} The time in minutes:seconds format, padded with zeros if necessary.
 */
// eslint-disable-next-line no-unused-vars
const secondstominutes = function(secs) {
    const m = (secs / 60) | 0;

    return `${m < 10 ? `0${  m}` : m
    }:${
        (secs %= 60) < 10 ? `0${  secs}` : secs}`;
};

/**
 * Converts milliseconds into a formatted string of minutes, seconds, and hundredths of a second.
 * @param {number} msecs The number of milliseconds to convert.
 * @returns {string} The formatted time string
 */
// eslint-disable-next-line no-unused-vars
const msecondstominutes = function(msecs) {
    const m = (msecs / 60000) | 0;
    msecs %= 60000;
    const secs = (msecs / 1000) | 0;
    msecs %= 1000;
    msecs = Math.round(msecs * 100 / 1000);
    if (msecs === 100) { msecs = 99; }

    return `${m < 10 ? `0${  m}` : m
    }:${
        secs < 10 ? `0${  secs}` : secs
    }.${
        msecs < 10 ? `0${  msecs}` : msecs}`;
};

/**
 * Converts an object with "red", "green", and "blue" properties (value range 0-255) into an RGB color code (e.g. 0xFF0000).
 * @param {Object} color An object with "red", "green", and "blue" properties, each a number from 0 to 255.
 * @returns {number} Single integer representing the RGB color code corresponding to the input color components.
 */
// eslint-disable-next-line no-unused-vars
const colorCodeFromObject = function(color) {
    return ((color.red & 0xFF) << 16 | (color.green & 0xFF) << 8 | (color.blue & 0xFF));
};

/**
 * Converts an RGB color code  (e.g. 0xFF0000) into an object with "red", "green", and "blue" properties.
 * @param {number} colorCode The RGB color code to convert, e.g., 0xFF0000 for red.
 * @returns {{red: number, green: number, blue: number}} An object containing the red, green, and blue components (ranging from 0 to 255) of the color.
 */
// eslint-disable-next-line no-unused-vars
const colorCodeToObject = function(colorCode) {
    return {
        "red": (colorCode >> 16) & 0xFF,
        "green": (colorCode >> 8) & 0xFF,
        "blue": colorCode & 0xFF
    };
};

/**
 * A collection of generic functions and regular expression
 */
var script = Object.freeze({
    // ----------------- Mapping script constants ---------------------

    // Common regular expressions
    samplerRegEx: Object.freeze(/^\[Sampler(\d+)\]$/),
    channelRegEx: Object.freeze(/^\[Channel(\d+)\]$/),
    eqRegEx: Object.freeze(/^\[EqualizerRack1_(\[.*\])_Effect1\]$/),
    quickEffectRegEx: Object.freeze(/^\[QuickEffectRack1_(\[.*\])\]$/),
    effectUnitRegEx: Object.freeze(/^\[EffectRack1_EffectUnit(\d+)\]$/),
    individualEffectRegEx: Object.freeze(/^\[EffectRack1_EffectUnit(\d+)_Effect(\d+)\]$/),

    // Library column value, which can be used to interact with the CO for "[Library] sort_column"
    LIBRARY_COLUMNS: Object.freeze({
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
        LAST_PLAYED: 31
    }),

    /**
     * Discriminates whether an object was created using the `{}` synthax.
     *
     * Returns true when was an object was created using the `{}` synthax.
     * False if the object is an instance of a class like Date or Proxy or an Array.
     *
     * isSimpleObject({}) // true
     * isSimpleObject(null) // false
     * isSimpleObject(undefined) // false
     * isSimpleObject(new Date) // false
     * isSimpleObject(new (class {})()) // false
     * @param {any} obj Object to test
     * @returns {boolean} true if obj was created using the `{}` or `new Object()` synthax, false otherwise
     */
    isSimpleObject(obj) {
        return obj !== null && typeof obj === "object" && obj.constructor.name === "Object";
    },


    /**
     * Deeply merges 2 objects (Arrays and Objects only, not Map for instance).
     * @param target {object | Array} Object to merge source into
     * @param source {object | Array} Object to merge into source
     * @deprecated Use {@link Object.assign} instead
     */
    deepMerge(target, source) {
        console.warn("script.deepMerge is deprecated; use Object.assign instead");

        if (target === source || target === undefined || target === null || source === undefined || source === null) {
            return;
        }

        if (Array.isArray(target) && Array.isArray(source)) {
            const objTarget = target.reduce((acc, val, idx) => Object.assign(acc, {[idx]: val}), {});
            const objSource = source.reduce((acc, val, idx) => Object.assign(acc, {[idx]: val}), {});
            script.deepMerge(objTarget, objSource);
            target.length = 0;
            const values = Object.keys(objTarget).map(key => objTarget[key]);
            target.push(...values);
        } else if (script.isSimpleObject(target) && script.isSimpleObject(source)) {
            Object.keys(source).forEach(key => {
                if (
                    Array.isArray(target[key]) && Array.isArray(source[key]) ||
                    script.isSimpleObject(target[key]) && script.isSimpleObject(source[key])
                ) {
                    script.deepMerge(target[key], source[key]);
                } else if (source[key] !== undefined && source[key] !== null) {
                    Object.assign(target, {[key]: source[key]});
                }
            });
        }
    },


    // @deprecated Use script.midiDebug() instead
    debug(channel, control, value, status, group) {
        console.log("Warning: script.debug() is deprecated. Use script.midiDebug() instead.");
        script.midiDebug(channel, control, value, status, group);
    },

    // @deprecated Use script.midiPitch() instead
    pitch(LSB, MSB, status) {
        console.log("Warning: script.pitch() is deprecated. Use script.midiPitch() instead.");
        return script.midiPitch(LSB, MSB, status);
    },

    // @deprecated Use script.absoluteLin() instead
    absoluteSlider(group, key, value, low, high, min, max) {
        console.log(
            "Warning: script.absoluteSlider() is deprecated. Use engine.setValue(group, key, script.absoluteLin(...)) instead.");
        engine.setValue(group, key, script.absoluteLin(value, low, high, min, max));
    },

    /**
     * Logs MIDI message information to the console.
     * Useful for debugging MIDI scripts by printing out the MIDI message components in a readable format.
     * @param {number} channel The MIDI channel of the message.
     * @param {number} control The control number (e.g., note number or controller number).
     * @param {number} value The value of the control (e.g., velocity or controller value).
     * @param {number} status The status of the MIDI message.
     * @param {string} group The Mixxx control group the message is associated with.
     */
    midiDebug(channel, control, value, status, group) {
        console.log(`Script.midiDebug - channel: 0x${  channel.toString(16)
        } control: 0x${  control.toString(16)  } value: 0x${  value.toString(16)
        } status: 0x${  status.toString(16)  } group: ${  group}`);
    },

    // Returns the deck number of a "ChannelN" or "SamplerN" group
    deckFromGroup(group) {
        let deck = "0";
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
        return Number.parseInt(deck);
    },

    /**
     * Binds or unbinds functions to Mixxx controls for a specific group.
     * @param {string} group The Mixxx control group (e.g., "[Channel1]", "[Master]") to which the bindings should apply.
     * @param {Object.<string, engine.CoCallback>} controlsToFunctions An object mapping control names to callback functions.
     * @param {boolean} [remove] If true, the bindings are removed instead of created. Defaults is false.
     * @deprecated Use engine.makeConnection or connection.disconnect() based code instead
     */
    bindConnections(group, controlsToFunctions, remove = false) {
        for (const control in controlsToFunctions) {
            engine.connectControl(group, control, controlsToFunctions[control], remove);
            if (!remove) {
                engine.trigger(group, control);
            }
        }
    },

    /**
     * Toggles the value of a specified control in Mixxx.
     * If the current value is 0, it sets the value to 1, and vice versa.
     * @param {string} group The Mixxx control group (e.g., "[Channel1]", "[Master]") the control belongs to.
     * @param {string} control The name of the control to toggle.
     */
    toggleControl(group, control) {
        engine.setValue(group, control, engine.getValue(group, control) === 0 ? 1 : 0);
    },

    /**
     * Triggers an engine value and resets it back to 0 after a specified delay.
     * This is helpful for mapping encoder turns to controls that are represented by buttons in skins,
     * so the skin button lights up briefly but does not stay lit.
     * @param {string} group The Mixxx control group (e.g., "[Channel1]", "[Master]") the control belongs to.
     * @param {string} control The name of the control to trigger.
     * @param {number} [delay] The delay in milliseconds before resetting the control value back to 0. Defaults is 200ms.
     */
    triggerControl(group, control, delay = 200) {
        if (typeof delay !== "number") {
            delay = 200;
        }
        engine.setValue(group, control, 1);
        engine.beginTimer(delay, () => engine.setValue(group, control, 0), true);
    },

    /**
     * Maps an absolute linear control value to a linear Mixxx control value.
     * This function is useful for mapping physical control inputs (e.g., knobs) to Mixxx control values that expect a linear range, such as volume (0..1).
     * @param {number} value The control value to convert (e.g., the position of a knob).
     * @param {number} low The lowest value of the Mixxx control (e.g., 0 for volume).
     * @param {number} high The highest value of the Mixxx control (e.g., 1 for volume).
     * @param {number} [min] The minimum value the control input can take (default is 0, typical for MIDI).
     * @param {number} [max] The maximum value the control input can take (default is 127, typical for MIDI).
     * @returns {number} The Mixxx control value corresponding to the current position of the control input.
     */
    absoluteLin(value, low, high, min = 0, max = 127) {
        if (value <= min) {
            return low;
        } else if (value >= max) {
            return high;
        } else {
            return ((((high - low) / (max - min)) * (value - min)) + low);
        }
    },

    /**
     * Maps a linear Mixxx control value (e.g., balance: -1 to 1) to an absolute linear value.
     * This function is the inverse of script.absoluteLin, converting a Mixxx control value back to a hardware control value.
     * @param {number} value The control value to convert (e.g., a balance control value between -1 and 1).
     * @param {number} low The lowest value of the Mixxx control (e.g., -1 for balance).
     * @param {number} high The highest value of the Mixxx control (e.g., 1 for balance).
     * @param {number} [min] The minimum value the hardware control can take (default is 0, typical for MIDI).
     * @param {number} [max] The maximum value the hardware control can take (default is 127, typical for MIDI).
     * @returns {number} The absolute linear value corresponding to the Mixxx control value, scaled to the hardware control's range.
     */
    absoluteLinInverse(value, low, high, min = 0, max = 127) {
        const result = (((value - low) * (max - min)) / (high - low)) + min;
        return Math.min(Math.max(result, min), max);
    },


    /**
     * Maps an absolute linear control value to a non-linear Mixxx control value.
     * This function is useful for mapping physical control inputs (e.g., knobs) to Mixxx control values that have a non-linear response, such as EQs.
     * @param {number} value The current value of the control input (e.g., the position of a knob).
     * @param {number} low The lowest value of the Mixxx control.
     * @param {number} mid The middle point of the Mixxx control.
     * @param {number} high The highest value of the Mixxx control.
     * @param {number} [min] The minimum value the hardware control can take (default is 0, typical for MIDI).
     * @param {number} [max] The maximum value the hardware control can take (default is 127, typical for MIDI).
     * @returns {number} The value corresponding to the current position of the control input, adjusted for a non-linear response.
     */
    absoluteNonLin(value, low, mid, high, min = 0, max = 127) {
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
    },

    /**
     * Maps a non-linear Mixxx control to an absolute linear value.
     * This function is the inverse of script.absoluteNonLin, useful for sending MIDI messages to controllers and comparing non-linear Mixxx controls to incoming MIDI values.
     * @param {number} value The current value of the control input (e.g., the position of a knob).
     * @param {number} low The lowest value of the Mixxx control.
     * @param {number} mid The middle point of the Mixxx control.
     * @param {number} high The highest value of the Mixxx control.
     * @param {number} [min] The minimum value the hardware control can take (default is 0, typical for MIDI).
     * @param {number} [max] The maximum value the hardware control can take (default is 127, typical for MIDI).
     * @returns {number} The value corresponding to the current position of the control input, adjusted for a inverse non-linear response.
     */
    absoluteNonLinInverse(value, low, mid, high, min = 0, max = 127) {
        const center = (max - min) / 2;
        let result;

        if (value === mid) {
            return center;
        } else if (value < mid) {
            result = (center / (mid - low)) * (value - low);
        } else {
            result = center + (center / (high - mid)) * (value - mid);
        }

        return Math.min(Math.max(result, min), max);
    },

    /**
     * Adjusts the crossfader curve based on the current value of a hardware control.
     * This function supports both Constant Power and Additive modes for the crossfader curve.
     * It maps the hardware control value to a Mixxx control value using a linear mapping.
     * @param {number} value The current value of the hardware control.
     * @param {number} min The minimum value for the hardware control.
     * @param {number} max The maximum value for the hardware control.
     */
    crossfaderCurve(value, min, max) {
        if (engine.getValue("[Mixer Profile]", "xFaderMode") === 1) {
            // Constant Power
            engine.setValue("[Mixer Profile]", "xFaderCalibration",
                script.absoluteLin(value, 0.5, 0.962, min, max)
            );
        } else {
            // Additive
            engine.setValue("[Mixer Profile]", "xFaderCurve",
                script.absoluteLin(value, 1, 2, min, max)
            );
        }
    },

    /**
     * Computes the euclidean modulo of a given m % n. The result is always
     * in the range [0, m]
     * @param {number} a The dividend.
     * @param {number} m The divisor.
     * @returns {number} The positive remainder of the euclidean modulo operation.
     */
    posMod(a, m) {
        return ((a % m) + m) % m;
    },

    /**
     * Moves the current loop by the specified number of beats (default 1/2) in the specified direction.
     * If the current loop length is shorter than the requested move distance, it's only moved a distance equal to its length.
     * @param {string} group The Mixxx control group (e.g., "[Channel1]", "[Master]") the loop belongs to.
     * @param {number} [direction] The direction to move the loop. Positive direction value moves the loop forwards, negative value moves it backwards. Forward is default.
     * @param {number} [numberOfBeats] The number of beats to move the loop. Defaults to 0.5 beats.
     */
    loopMove(group, direction, numberOfBeats = 0.5) {
        if (numberOfBeats === 0) { numberOfBeats = 0.5; }

        if (direction < 0) {
            engine.setValue(group, "loop_move", -numberOfBeats);
        } else {
            engine.setValue(group, "loop_move", numberOfBeats);
        }
    },

    /**
     * Takes the value from a little-endian 14-bit MIDI pitch wheel message and returns the value for a "rate" (pitch slider) Mixxx control.
     * This function is useful for handling MIDI pitch wheel messages and converting them to a format that can be used to control the pitch in Mixxx.
     * @param {number} LSB The least significant byte of the MIDI pitch wheel message.
     * @param {number} MSB The most significant byte of the MIDI pitch wheel message.
     * @param {number} status The MIDI status byte, used to verify that the message is a pitch wheel message.
     * @returns {number|boolean} The value for a "rate" control in Mixxx, or false if the input MIDI message was not a Pitch message (0xE#).
     */
    // TODO: Is this still useful now that MidiController.cpp properly handles these?
    midiPitch(LSB, MSB, status) {
        if ((status & 0xF0) !== 0xE0) { // Mask the upper nybble so we can check the opcode regardless of the channel
            console.log(`Script.midiPitch: Error, not a MIDI pitch (0xEn) message: ${  status}`);
            return false;
        }
        const value = (MSB << 7) | LSB; // Construct the 14-bit number

        // Range is 0x0000..0x3FFF center @ 0x2000, i.e. 0..16383 center @ 8192
        const rate = (value - 8192) / 8191;
        //     print("Script.Pitch: MSB="+MSB+", LSB="+LSB+", value="+value+", rate="+rate);
        return rate;
    },

    /**
     * Wrapper around engine.spinback() that can be directly mapped from XML for a spinback effect.
     * This function initiates a spinback effect on the specified deck. The spinback effect simulates the sound of quickly
     * reversing the direction of a playing track. The duration and speed of the spinback can be adjusted.
     * @param {number} channel not used
     * @param {number} control not used
     * @param {number} value Positive value activates spinback
     * @param {number} status The status of the MIDI message, if the MSB bit 0x80 of the byte is not set, value is evaluated.
     * @param {string} group The group
     * @param {number} [factor] Multiplier for the spinback effect. Higher values increase the speed and duration of the spinback. Default is 1.0.
     * @param {number} [rate] Rate at which the spinback starts. Can be used to simulate different starting speeds for the effect. Default is 0.0.
     */
    spinback(channel, control, value, status, group, factor = 1, rate = -10) {
        // disable on note-off or zero value note/cc
        engine.spinback(
            parseInt(group.substring(8, 9)), ((status & 0xF0) !== 0x80 && value > 0),
            factor, rate
        );
    },

    /**
     * Wrapper around engine.brake() that can be directly mapped from XML for a brake effect.
     * Initiates a brake effect on the specified deck, simulating the sound of a turntable slowing to a stop.
     * @param {number} channel not used
     * @param {number} control not used
     * @param {number} value Positive value activates brake
     * @param {number} status The status of the MIDI message, if the MSB bit 0x80 of the byte is not set, value is evaluated.
     * @param {string} group The group where the deck number is derived from, e.g. "[Channel1]"
     * @param {number} [factor] Multiplier for the brake effect. Higher values increase the duration of the brake effect.
     */
    brake(channel, control, value, status, group, factor = 1) {
        // disable on note-off or zero value note/cc, use default decay rate '1'
        engine.brake(
            parseInt(group.substring(8, 9)), ((status & 0xF0) !== 0x80 && value > 0),
            factor
        );
    },

    /**
     * Wrapper around engine.softStart() that can be directly mapped from XML to start and accelerate a deck from zero to full rate defined by the pitch slider. It can also interrupt engine.brake().
     * This function is useful for creating a soft start effect on a deck, gradually increasing its playback speed to the rate determined by the pitch slider.
     * @param {number} channel not used
     * @param {number} control not used
     * @param {number} value Positive value activates spinback
     * @param {number} status The status of the MIDI message, if the MSB bit 0x80 of the byte is not set, value is evaluated.
     * @param {string} group The group where the deck number is derived from, e.g. "[Channel1]"
     * @param {number} [factor] Multiplier for the acceleration effect. Higher values increase the acceleration of the soft start effect.
     */
    softStart(channel, control, value, status, group, factor = 1) {
        // disable on note-off or zero value note/cc, use default increase rate '1'
        engine.softStart(
            parseInt(group.substring(8, 9)), ((status & 0xF0) !== 0x80 && value > 0),
            factor
        );
    }

});


/**
 * Used for tapping the desired BPM for a deck
 */
// @ts-ignore Same identifier for class and instance needed for backward compatibility
class bpmClass {
    constructor() {
        this.tapTime = 0.0;
        this.previousTapDelta = 0.0;
        this.tap = [];   // Tap sample values
    }


    /**
     * Sets the tempo of the track on a deck by tapping the desired beats.
     * Useful for manually synchronizing a track to an external beat.
     * This function only works if the track's detected BPM value is correct.
     * Call this function each time the tap button is pressed.
     * @param {number} deck The deck to adjust.
     */
    tapButton(deck) {
        const now = new Date().getTime() / 1000; // Current time in seconds
        const tapDelta = now - this.tapTime;
        this.tapTime = now;

        // assign tapDelta in cases where the button has not been pressed previously
        if (this.tap.length < 1) {
            this.previousTapDelta = tapDelta;
        }
        // reset if longer than two seconds between taps
        if (tapDelta > 2.0) {
            this.tap = [];
            return;
        }
        // reject occurrences of accidental double or missed taps
        // a tap is considered missed when the delta of this press is 80% longer than the previous one
        // and a tap is considered double when the delta is shorter than 40% of the previous one.
        // these numbers are just guesses that produced good results in practice
        if ((tapDelta > this.previousTapDelta * 1.8) || (tapDelta < this.previousTapDelta * 0.6)) {
            return;
        }
        this.previousTapDelta = tapDelta;
        this.tap.push(60 / tapDelta);
        // Keep the last 8 samples for averaging
        if (this.tap.length > 8) { this.tap.shift(); }
        let sum = 0;
        for (let i = 0; i < this.tap.length; i++) {
            sum += this.tap[i];
        }
        const average = sum / this.tap.length;

        const group = `[Channel${  deck  }]`;

        // "bpm" was changed in 1.10 to reflect the *adjusted* bpm, but I presume it
        // was supposed to return the tracks bpm (which it did before the change).
        // "file_bpm" is supposed to return the set BPM of the loaded track of the
        // channel.
        let fRateScale = average / engine.getValue(group, "file_bpm");

        // Adjust the rate:
        fRateScale = (fRateScale - 1.) / engine.getValue(group, "rateRange");

        engine.setValue(
            group, "rate",
            fRateScale * engine.getValue(group, "rate_dir")
        );
    }
}
// Add instance bpm of bpmClass to the Global JavaScript object
this.bpm = new bpmClass();
