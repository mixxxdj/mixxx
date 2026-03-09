// Type declarations for res/controllers/common-controller-scripts.js

// ----------------- Prototype enhancements ---------------------

/** Returns an ASCII byte array for the string. */
interface String {
    toInt(): number[];
}

// ----------------- Function overloads ---------------------

/**
 * Prints a message to the terminal and the log file.
 *
 * @param message - The log message.
 * @deprecated Use console.log()/console.warn()/console.debug() instead.
 */
declare function print(message: string): void;

/**
 * Stringifies an object with configurable depth.
 *
 * @param obj - Object to print.
 * @param maxdepth - Maximum recursion depth.
 */
declare function printObject(obj: unknown, maxdepth?: number): void;

/**
 * Stringifies an object with configurable depth, returning the original value
 * if it cannot be stringified.
 *
 * @param obj - Value to stringify.
 * @param maxdepth - Maximum recursion depth.
 * @param checked - Internal list of visited objects.
 * @param prefix - Internal indentation prefix.
 * @returns JSON string or the original value.
 */
declare function stringifyObject<T>(obj: T, maxdepth?: number, checked?: unknown[], prefix?: string): string | T;

/**
 * Checks whether an array contains an element using strict equality.
 *
 * @param array - Array to search.
 * @param elem - Element to match.
 * @returns True if found.
 */
declare function arrayContains<T>(array: T[], elem: T): boolean;

// ----------------- Generic functions ---------------------

/**
 * Formats seconds as mm:ss.
 *
 * @param secs - Seconds to format.
 * @returns Formatted string.
 */
declare function secondstominutes(secs: number): string;

/**
 * Formats milliseconds as mm:ss.cc (centiseconds).
 *
 * @param msecs - Milliseconds to format.
 * @returns Formatted string.
 */
declare function msecondstominutes(msecs: number): string;

/**
 * RGB color object with "red", "green" and "blue" properties (value range 0-255)
 */
type ColorRGB = { red: number; green: number; blue: number };

/**
 * Converts an object with "red", "green" and "blue" properties (value range
 * 0-255) into an RGB color code (e.g. 0xFF0000).
 *
 * @param color - RGB component object.
 * @returns Packed RGB color code.
 */
declare function colorCodeFromObject(color: ColorRGB): number;

/**
 * Converts an RGB color code (e.g. 0xFF0000) into an object with "red",
 * "green" and "blue" properties (value range 0-255).
 *
 * @param colorCode - Packed RGB color code.
 * @returns RGB component object.
 */
declare function colorCodeToObject(colorCode: number): ColorRGB;

/**
 * Common controller script utilities, mappings, and helper functions.
 */
declare namespace script {
    // ----------------- Mapping constants ---------------------

    /**
     * Library column value, which can be used to interact with the CO for
     * "[Library] sort_column".
     */
    const LIBRARY_COLUMNS: Readonly<{
        ARTIST: 1;
        TITLE: 2;
        ALBUM: 3;
        ALBUM_ARTIST: 4;
        YEAR: 5;
        GENRE: 6;
        COMPOSER: 7;
        GROUPING: 8;
        TRACK_NUMBER: 9;
        FILETYPE: 10;
        NATIVE_LOCATION: 11;
        COMMENT: 12;
        DURATION: 13;
        BITRATE: 14;
        BPM: 15;
        REPLAY_GAIN: 16;
        DATETIME_ADDED: 17;
        TIMES_PLAYED: 18;
        RATING: 19;
        KEY: 20;
        PREVIEW: 21;
        COVERART: 22;
        TRACK_COLOR: 30;
        LAST_PLAYED: 31;
    }>;

    /**
     * @deprecated Use script.midiDebug() instead.
     * @param channel - MIDI channel.
     * @param control - MIDI control number.
     * @param value - MIDI value.
     * @param status - MIDI status byte.
     * @param group - Mixxx group.
     */
    function debug(channel: number, control: number, value: number, status: number, group: MixxxControls.Group): void;

    /**
     * @deprecated Use script.midiPitch() instead.
     * @param LSB - Least significant byte.
     * @param MSB - Most significant byte.
     * @param status - MIDI status byte.
     * @returns Pitch value or false if not a pitch message.
     */
    function pitch(LSB: number, MSB: number, status: number): number | false;

    /**
     * @deprecated Use engine.setValue(group, key, script.absoluteLin(...)) instead.
     * @param group - Mixxx group.
     * @param key - Mixxx control name.
     * @param value - Input value.
     * @param low - Output value at minimum input.
     * @param high - Output value at maximum input.
     * @param min - Minimum input value.
     * @param max - Maximum input value.
     */
    function absoluteSlider<TGroup extends MixxxControls.Group>(
        group: TGroup,
        key: MixxxControls.CtrlRW<TGroup>,
        value: number,
        low: number,
        high: number,
        min?: number,
        max?: number,
    ): void;

    /**
     * Logs MIDI message details.
     *
     * @param channel - MIDI channel.
     * @param control - MIDI control number.
     * @param value - MIDI value.
     * @param status - MIDI status byte.
     * @param group - Mixxx group.
     */
    function midiDebug(channel: number, control: number, value: number, status: number, group: MixxxControls.Group): void;

    /**
     * Returns the channel group name from the stem group name.
     *
     * @param stem - Stem group name.
     * @returns Channel group name or undefined.
     */
    function channelFromStem(stem: string): MixxxControls.Group | undefined;

    /**
     * Returns the deck number of a "ChannelN" or "SamplerN" group.
     *
     * @param group - Mixxx group.
     * @returns Deck number or undefined.
     */
    function deckFromGroup(group?: MixxxControls.Group): number | undefined;

    /**
     * Binds multiple controls at once.
     *
     * @param group - Mixxx group.
     * @param controlsToFunctions - Map of control names to callbacks.
     * @param remove - Disconnect instead of connect.
     */
    function bindConnections<TGroup extends MixxxControls.Group>(
        group: TGroup,
        controlsToFunctions: Partial<Record<MixxxControls.Ctrl<TGroup>, engine.CoCallback<TGroup>>>,
        remove?: boolean,
    ): void;

    /**
     * Toggles a Mixxx control value.
     *
     * @param group - Mixxx group.
     * @param control - Mixxx control name.
     */
    function toggleControl<TGroup extends MixxxControls.Group>(group: TGroup, control: MixxxControls.CtrlRW<TGroup>): void;

    /**
     * Triggers a control value and resets it back to 0 after a delay.
     *
     * @param group - Mixxx group.
     * @param control - Mixxx control name.
     * @param delay - Delay in milliseconds.
     */
    function triggerControl<TGroup extends MixxxControls.Group>(group: TGroup, control: MixxxControls.CtrlRW<TGroup>, delay?: number): void;

    /**
     * Maps an absolute linear input to a linear Mixxx control range.
     *
     * @param value - Input value.
     * @param low - Output value at minimum input.
     * @param high - Output value at maximum input.
     * @param min - Minimum input value.
     * @param max - Maximum input value.
     * @returns Mixxx control value.
     */
    function absoluteLin(value: number, low: number, high: number, min?: number, max?: number): number;

    /**
     * Maps a linear Mixxx control value to an absolute linear input value.
     *
     * @param value - Mixxx control value.
     * @param low - Output value at minimum input.
     * @param high - Output value at maximum input.
     * @param min - Minimum input value.
     * @param max - Maximum input value.
     * @returns Linear value in input range.
     */
    function absoluteLinInverse(value: number, low: number, high: number, min?: number, max?: number): number;

    /**
     * Maps an absolute linear input to a non-linear Mixxx control range.
     *
     * @param value - Input value.
     * @param low - Output value at minimum input.
     * @param mid - Output value at center input.
     * @param high - Output value at maximum input.
     * @param min - Minimum input value.
     * @param max - Maximum input value.
     * @returns Mixxx control value.
     */
    function absoluteNonLin(value: number, low: number, mid: number, high: number, min?: number, max?: number): number;

    /**
     * Maps a non-linear Mixxx control value to an absolute linear input value.
     *
     * @param value - Mixxx control value.
     * @param low - Output value at minimum input.
     * @param mid - Output value at center input.
     * @param high - Output value at maximum input.
     * @param min - Minimum input value.
     * @param max - Maximum input value.
     * @returns Linear value in input range.
     */
    function absoluteNonLinInverse(value: number, low: number, mid: number, high: number, min?: number, max?: number): number;

    /**
     * Adjusts the crossfader curve using a hardware control.
     *
     * @param value - Control value.
     * @param min - Minimum input value.
     * @param max - Maximum input value.
     */
    function crossfaderCurve(value: number, min: number, max: number): void;

    /**
     * Computes the euclidean modulo of a % m (result in [0, m[).
     *
     * @param a - Dividend.
     * @param m - Divisor.
     * @returns Positive remainder.
     */
    function posMod(a: number, m: number): number;

    /**
     * Moves the current loop by the specified number of beats in the given direction.
     *
     * @param group - Mixxx group.
     * @param direction - Positive for forward, negative for backward.
     * @param numberOfBeats - Beats to move (default 0.5).
     */
    function loopMove<TGroup extends MixxxControls.Group>(group: TGroup, direction: number, numberOfBeats?: number): void;

    /**
     * Converts a 14-bit MIDI pitch wheel message to a Mixxx rate value.
     *
     * @param LSB - Least significant byte.
     * @param MSB - Most significant byte.
     * @param status - MIDI status byte.
     * @returns Pitch value or false if not a pitch message.
     */
    function midiPitch(LSB: number, MSB: number, status: number): number | false;

    /**
     * Wrapper around engine.spinback() for direct XML mapping.
     *
     * @param channel - MIDI channel.
     * @param control - MIDI control number.
     * @param value - MIDI value.
     * @param status - MIDI status byte.
     * @param group - Mixxx group.
     * @param factor - Spinback factor.
     * @param rate - Start rate.
     */
    function spinback(channel: number, control: number, value: number, status: number, group: MixxxControls.Group, factor?: number, rate?: number): void;

    /**
     * Wrapper around engine.brake() for direct XML mapping.
     *
     * @param channel - MIDI channel.
     * @param control - MIDI control number.
     * @param value - MIDI value.
     * @param status - MIDI status byte.
     * @param group - Mixxx group.
     * @param factor - Brake factor.
     */
    function brake(channel: number, control: number, value: number, status: number, group: MixxxControls.Group, factor?: number): void;

    /**
     * Wrapper around engine.softStart() for direct XML mapping.
     *
     * @param channel - MIDI channel.
     * @param control - MIDI control number.
     * @param value - MIDI value.
     * @param status - MIDI status byte.
     * @param group - Mixxx group.
     * @param factor - Acceleration factor.
     */
    function softStart(channel: number, control: number, value: number, status: number, group: MixxxControls.Group, factor?: number): void;

    // ----------------- Common regular expressions --------------------------
    const samplerRegEx: RegExp;
    const channelRegEx: RegExp;
    const eqRegEx: RegExp;
    const quickEffectRegEx: RegExp;
    const effectUnitRegEx: RegExp;
    const individualEffectRegEx: RegExp;
}

/**
 * Used for tapping the desired BPM for a deck.
 */
declare namespace bpm {
    let tapTime: number;
    let previousTapDelta: number;
    let tap: number[];

    /**
     * Sets the tempo of the track on a deck by tapping the desired beats.
     *
     * @param deck - Deck number to adjust.
     */
    function tapButton(deck: number): void;
}

// ----------------- Object definitions --------------------------

declare const ButtonState: Readonly<{ released: number; pressed: number }>;

declare const LedState: Readonly<{ off: number; on: number }>;

/** Controller */

declare class Controller {
    group: MixxxControls.Group;
    Controls: Record<string, Control>;
    Buttons: Record<string, Button>;

    constructor();

    /**
     * Adds a button and optionally binds a handler name on this instance.
     *
     * @param buttonName - Logical name for lookup.
     * @param button - Button instance.
     * @param eventHandler - Method name on the controller to invoke.
     */
    addButton(buttonName: string, button: Button, eventHandler?: string): void;

    /**
     * Sets a mapped control value for this controller's group.
     *
     * @param control - Control name.
     * @param value - Value to set.
     */
    setControlValue(control: string, value: number): void;
}

declare class Button {
    controlId: number;
    state: number;
    handler?: (value: number) => void;

    constructor(controlId: number);

    /**
     * Invokes the assigned handler with the incoming value.
     *
     * @param value - Incoming value.
     */
    handleEvent(value: number): void;
}

declare class Control {
    minInput: number;
    midInput: number;
    maxInput: number;
    minOutput: number;
    midOutput: number;
    maxOutput: number;
    mappedFunction: string;
    softMode: boolean;
    maxJump: number;

    constructor(mappedFunction: string, softMode?: boolean);

    /**
     * Sets the mapped value on the Mixxx control, applying soft takeover if enabled.
     *
     * @param group - Mixxx group.
     * @param inputValue - Raw input value.
     */
    setValue(group: MixxxControls.Group, inputValue: number): void;
}

declare class Deck {
    deckNumber: number;
    group: MixxxControls.Group;
    Buttons: Record<string, Button>;
    Controls: Record<string, Control>;

    constructor(deckNumber: number, group: MixxxControls.Group);

    /**
     * Adds a button and optionally binds a handler name on this instance.
     *
     * @param buttonName - Logical name for lookup.
     * @param button - Button instance.
     * @param eventHandler - Method name on the deck to invoke.
     */
    addButton(buttonName: string, button: Button, eventHandler?: string): void;

    /**
     * Sets a mapped control value for this deck's group.
     *
     * @param control - Control name.
     * @param value - Value to set.
     */
    setControlValue(control: string, value: number): void;
}
