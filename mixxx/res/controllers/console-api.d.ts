
/**
     Mixxx installs the QJSEngine::ConsoleExtension for the use in controller mapping scripts.
    See also:
      https://doc.qt.io/qt-5/qtquick-debugging.html#console-api
      https://developer.mozilla.org/en-US/docs/Web/API/console
      https://console.spec.whatwg.org/
 */

declare namespace console {
    /**
     * Prints debugging information to the console, when
     * QT_LOGGING_RULES="js.debug=true;" or
     * Mixxx is started with --controller-debug
     * This function is identical to console.debug
     *
     * @param data Message to print
     *    - Either a list of objects whose string representations get concatenated into the message string
     *    - Or a string containing zero or more substitution strings followed by a list of objects to replace them
     */
    function log(...data: any[]): void;

    /**
     * Prints debugging information to the console, when
     * QT_LOGGING_RULES="js.debug=true;" or
     * Mixxx is started with --controller-debug
     * This function is identical to console.log
     *
     * @param data Message to print
     *    - Either a list of objects whose string representations get concatenated into the message string
     *    - Or a string containing zero or more substitution strings followed by a list of objects to replace them
     */
    function debug(...data: any[]): void;

    /**
     * Prints information to the console, when
     * QT_LOGGING_RULES="js.info=true;" or
     * Mixxx is started with --controller-debug
     *
     * @param data Message to print
     *    - Either a list of objects whose string representations get concatenated into the message string
     *    - Or a string containing zero or more substitution strings followed by a list of objects to replace them
     */
    function info(...data: any[]): void;

    /**
     * Prints a warning message to the console, when
     * QT_LOGGING_RULES="js.warning=true;" or
     * Mixxx is started with --controller-debug
     *
     * @param data Message to print
     *    - Either a list of objects whose string representations get concatenated into the message string
     *    - Or a string containing zero or more substitution strings followed by a list of objects to replace them
     */
    function warn(...data: any[]): void;

    /**
     * Prints an error message to the console, when
     * QT_LOGGING_RULES="js.critical=true;" or
     * Mixxx is started with --controller-debug
     *
     * @param data Message to print
     *    - Either a list of objects whose string representations get concatenated into the message string
     *    - Or a string containing zero or more substitution strings followed by a list of objects to replace them
     */
    function error(...data: any[]): void;

    /**
     * Tests that a boolean expression is true,
     * if not, it writes an (optional) message to the console and prints the stack trace.
     *
     * @param condition If the condition is false, it prints message and stack trace
     * @param data Message to print
     *    - Either a list of objects whose string representations get concatenated into the message string
     *    - Or a string containing zero or more substitution strings followed by a list of objects to replace them
     */
    function assert(condition: boolean, ...data: any[]): void;

    /**
     * Starts the time measurement, which will be printed by timeEnd
     *
     * @param label string argument that identifies the measurement.
     */
    function time(label?: string): void;
    /**
     * Logs the time (in milliseconds) that was spent since the call of the time method.
     *
     * @param label string argument that identifies the measurement.
     */
    function timeEnd(label?: string): void;

    /**
     * Prints the stack trace of the JavaScript execution at the point where it was called.
     * This stack trace information contains the function name, file name, line number, and column number.
     * The stack trace is limited to last 10 stack frames.
     *
     * Unfortunately this function does use the wrong logging category "default" instead of "js",
     * which is set when you start Mixxx with --controller-debug. Without setting logging category
     * "default" manually, you will not see any output.
     * [see QTBUG-108673]{@link https://bugreports.qt.io/browse/QTBUG-108673}
     *
     * @param data Message to print
     *    - Either a list of objects whose string representations get concatenated into the message string
     *    - Or a string containing zero or more substitution strings followed by a list of objects to replace them
     */
    function trace(...data: any[]): void;

    /**
     * Prints the current number of times a particular piece of code has run, along with a message.
     *
     * @param label message to be prepended before the count
     */
    function count(label?: string): void;

    /**
     *  Turns on the JavaScript profiler.
     *
     * @param label measurement label
     * @deprecated Not usable for controller mappings for now [see QTBUG-65419]{@link https://bugreports.qt.io/browse/QTBUG-65419}
     */
     function profile(label?: string): void;

    /**
     *  Turns off the JavaScript profiler.
     *
     * @param label measurement label
     * @deprecated Not usable for controller mappings for now [see QTBUG-65419]{@link https://bugreports.qt.io/browse/QTBUG-65419}
     */
    function profileEnd(label?: string): void;

    /**
     * Prints an error message together with the stack trace of JavaScript execution at the point where it is called.
     *
     * @param data Message to print
     *    - Either a list of objects whose string representations get concatenated into the message string
     *    - Or a string containing zero or more substitution strings followed by a list of objects to replace them
     */
    function exception(...data: any[]): void;

}
