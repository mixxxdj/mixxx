
/** ScriptConnectionJSProxy */

declare class ScriptConnection {
    /**
     * Disconnect the script connection,
     * established by {@link engine.makeConnection} or {@link engine.makeUnbufferedConnection}
     *
     * @return Returns true if the connection is successful disconnected
     */
    disconnect(): boolean;

    /**
     * Triggers the execution of the callback function of the script connection,
     * established by {@link engine.makeConnection} or {@link engine.makeUnbufferedConnection}
     * with the actual value of a control.
     *
     * Note: To execute all callback functions connected to a ControlObject at once, use {@link engine.trigger} instead
    */
    trigger(): void;
}


/** ControllerScriptInterfaceLegacy */

declare namespace engine {
    /**
     * Gets the control value
     *
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "play_indicator"
     * @returns Value of the control (within it's range according Mixxx Controls manual page)
     */
    function getValue(group: string, name: string): number;

    /**
     * Sets a control value
     *
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "play_indicator"
     * @param newValue Value to be set (within it's range according Mixxx Controls manual page)
     */
    function setValue(group: string, name: string, newValue: number): void;

    /**
     * Gets the control value normalized to a range of 0..1
     *
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "play_indicator"
     * @returns Value of the control normalized to range of 0..1
     */
    function getParameter(group: string, name: string): number;

    /**
     * Sets the control value specified with normalized range of 0..1
     *
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "play_indicator"
     * @param newValue Value to be set, normalized to a range of 0..1
     */
    function setParameter(group: string, name: string, newValue: number): void;

    /**
     * Normalizes a specified value using the range of the given control,
     * to the range of 0..1
     *
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "play_indicator"
     * @param value Value with the controls range according Mixxx Controls manual page
     * @returns Value normalized to range of 0..1
     */
    function getParameterForValue(group: string, name: string, value: number): number;

    /**
     * Resets the control to its default value
     *
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "play_indicator"
     */
    function reset(group: string, name: string): void;

    /**
     * Returns the default value of a control
     *
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "play_indicator"
     * @returns Default value with the controls range according Mixxx Controls manual page
     */
    function getDefaultValue(group: string, name: string): number;

    /**
     * Returns the default value of a control, normalized to a range of 0..1
     *
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "play_indicator"
     * @returns Default value of the specified control normalized to range of 0..1
     */
    function getDefaultParameter(group: string, name: string): number;

    type CoCallback = (value: number, group: string, name: string) => void

    /** Connects a specified Mixxx Control with a callback function, which is executed if the value of the control changes
     *
     * @details This connection has a FIFO buffer - all value change events are processed in serial order.
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "play_indicator"
     * @param callback JS function, which will be called everytime, the value of the connected control changes.
     * @return Returns script connection object on success, otherwise 'undefined''
     */
    function makeConnection(group: string, name: string, callback: CoCallback): ScriptConnection |undefined;

    /** Connects a specified Mixxx Control with a callback function, which is executed if the value of the control changes
     *
     * @details This connection is unbuffered - when value change events occur faster, than the mapping can process them,
     *          only the last value set for the control is processed.
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "VuMeter"
     * @param callback JS function, which will be called everytime, the value of the connected control changes.
     * @return Returns script connection object on success, otherwise 'undefined''
     */
    function makeUnbufferedConnection(group: string, name: string, callback: CoCallback): ScriptConnection | undefined;

    /** This function is a legacy version of makeConnection with several alternate
     * ways of invoking it. The callback function can be passed either as a string of
     * JavaScript code that evaluates to a function or an actual JavaScript function.
     *
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "VuMeter"
     * @param callback JS function, which will be called everytime, the value of the connected control changes.
     * @param disconnect If "true", all connections to the ControlObject are removed. [default = false]
     * @return Returns script connection object on success, otherwise 'undefined' or 'false' depending on the error cause.
     *
     * @deprecated Use {@link makeConnection} instead
     */
    function connectControl(group: string, name: string, callback: CoCallback, disconnect?: boolean): ScriptConnection | boolean | undefined;


    /**
     * Triggers the execution of all connected callback functions, with the actual value of a control.
     * Note: To trigger a single connection, us {@link ScriptConnection.trigger} instead
     *
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "play_indicator"
     */
    function trigger(group: string, name: string): void;

    /** @deprecated Use {@link console.log} instead */
    function log(message: string): void;

    /**
     * Starts a timer that will call the specified script function
     *
     * @param interval Time in milliseconds until the function is executed.
     *                 Intervals below 20ms are not allowed.
     * @param scriptCode Function to be executed,
     *                   you can also use closures as:
     *                   function() { print("Executed Timer") }
     * @param oneShot If true the function is only once,
     *                if false the function is executed repeatedly  [default = false]
     * @returns timerId which is needed to stop a timer.
     *          In case of an error, 0 is returned.
     */
    function beginTimer(interval: number, scriptCode: ()=>any, oneShot?: boolean): number;

    /**
     * Stops the specified timer
     *
     * @param timerId ID of the timer
     */
    function stopTimer(timerId: number): void;

    /**
     * Jogwheel function to be called when scratching starts (usually when the wheel is touched)
     * This function contains an parametrizeable alpha-beta filter, which influences the
     * responsiveness and looseness of the imaginary slip mat
     *
     * @param deck The deck number to use, e.g: 1
     * @param intervalsPerRev The resolution of the MIDI control (in intervals per revolution, typically 128.)
     * @param rpm The speed of the imaginary record at 0% pitch (in revolutions per minute (RPM) typically 33+1/3, adjust for comfort)
     * @param alpha The alpha coefficient of the filter (start with 1/8 (0.125) and tune from there)
     * @param beta The beta coefficient of the filter (start with alpha/32 and tune from there)
     * @param ramp Set true to ramp the deck speed down. Set false to stop instantly [default = true]
     */
    function scratchEnable(deck: number, intervalsPerRev: number, rpm: number, alpha: number, beta: number, ramp?: boolean): void;

    /**
     * Function to be called each time the jogwheel is moved during scratching
     *
     * @param deck The deck number to use, e.g: 1
     * @param interval The movement value (typically 1 for one "tick" forwards, -1 for one "tick" backwards)
     */
    function scratchTick(deck: number, interval: number): void;

    /**
     * Jogwheel function to be called when scratching ends (usually when the wheel is released)
     * @param deck The deck number to use, e.g: 1
     * @param ramp  Set true to ramp the deck speed up. Set false to jump to normal play speed instantly [default = true]
     */
    function scratchDisable(deck: number, ramp?: boolean): void;

    /**
     * Returns true if scratching is enabled
     * @param deck The deck number to use, e.g: 1
     * @returns True if scratching is enabled
     */
    function isScratching(deck: number): boolean;

    /**
     * If enabled, soft-takeover prevents sudden wide parameter changes,
     * when on-screen control and hardware control divert.
     * With soft-takeover you need to turn a hardware knob, until it reaches
     * the position of the on-screen knob - than it takes over control.
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "pregain"
     * @param enable Set true to enable soft-takeover for the specified control
     */
    function softTakeover(group: string, name: string, enable: boolean): void;

    /**
     * Inhibits a suden value change from the hardware control.
     * Should be called when receiving input for the knob/fader,
     * that switches its behavior (e.g. Shift-Button pressed)
     *
     * @param group Group of the control e.g. "[Channel1]"
     * @param name Name of the control e.g. "pregain"
     */
    function softTakeoverIgnoreNextValue(group: string, name: string): void;

    /**
     * To achieve a brake effect of the playback speed
     * Both engine.softStart() and engine.brake()/engine.spinback() can interrupt each other.
     *
     * @param deck The deck number to use, e.g: 1
     * @param activate Set true to activate, or false to disable
     * @param factor Defines how quickly the deck should come to a stop.
     *               Start with a value of 1 and increase to increase the acceleration.
     *               Be aware that brake called with low factors (about 0.5 and lower),
     *               would keep the deck running altough the resulting very low sounds are not audible anymore. [default = 1.0]
     * @param rate The initial speed of the deck when enabled. "1" (default) means 10x speed in forward.
     *             Negative values like "-1" also work, though then it's spinning reverse obviously. [default = 1.0]
     */
    function brake(deck: number, activate: boolean, factor?: number, rate?: number): void;

    /**
     * To achieve a spinback effect of the playback speed
     * Both engine.softStart() and engine.brake()/engine.spinback() can interrupt each other.
     *
     * @param deck The deck number to use, e.g: 1
     * @param activate Set true to activate, or false to disable
     * @param factor Defines how quickly the deck should come to normal playback rate.
     *               Start with a value of 1 and increase to increase the acceleration.
     *               Be aware that spinback called with low factors (about 0.5 and lower),
     *               would keep the deck running altough the resulting very low sounds are not audible anymore. [default = 1.8]
     * @param rate The initial speed of the deck when enabled. "-10" (default) means 10x speed in reverse.
     *             Positive values like "10" also work, though then it's spinning forward obviously. [default = -10.0]
     */
    function spinback(deck: number, activate: boolean, factor?: number, rate?: number): void;

    /**
     * To achieve a forward acceleration effect from standstill to normal speed.
     * Both engine.softStart() and engine.brake()/engine.spinback() can interrupt each other.
     *
     * @param deck The deck number to use, e.g: 1
     * @param activate Set true to activate, or false to disable
     * @param factor Defines how quickly the deck should come to normal playback rate.
     *               Start with a value of 1 and increase to increase the acceleration.
     *               SoftStart with low factors would take a while until sound is audible. [default = 1.0]
     */
    function softStart(deck: number, activate: boolean, factor?: number): void;
}
