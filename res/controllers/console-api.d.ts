
/** Mixxx installs the QJSEngine::ConsoleExtension for the use in controller mapping scripts.
    See also: https://doc.qt.io/qt-5/qtquick-debugging.html#console-api
*/

declare class QJSEngineConsoleExtension {
    /**
     * Prints debugging information to the console. (same as console.debug)
    */
    public log(message?:any, ...args:any[]):void;

    /**
     * Prints debugging information to the console. (same as console.log)
    */
    public debug(message?:any, ...args:any[]):void;

    /**
     * Prints information to the console.
    */
    public info(message?:any, ...args:any[]):void;

    /**
     * Prints a warning message to the console.
    */
    public warn(message?:any, ...args:any[]):void;

    /**
     * Prints an error message to the console.
    */
    public error(message?:any, ...args:any[]):void;

    /**
     * Tests that an expression is true.
     *
     * @param expression If not true, it writes an optional message to the console and prints the stack trace.
     * @param message
     * @param args
    */
    public assert(expression:boolean, message?:string, ...args: any[]):void;

    /**
     * Starts the time measurement, which will be printed by timeEnd
     *
     * @param label string argument that identifies the measurement.
     */
    public time(label?:string):void;
    /**
     * Logs the time (in milliseconds) that was spent since the call of the time method.
     *
     * @param label string argument that identifies the measurement.
     */
    public timeEnd(label?:string):void;

    /**
     * Prints the stack trace of the JavaScript execution at the point where it was called.
     * This stack trace information contains the function name, file name, line number, and column number.
     * The stack trace is limited to last 10 stack frames.
     *
     * @param message
     * @param args
     */
    public trace(message?:any, ...args:any[]):void;

    /**
     * Prints the current number of times a particular piece of code has run, along with a message.
     *
     * @param label
     */
    public count(label?:string):void;

    /**
     *  Turns on the JavaScript profiler.
     *  @deprecated Not usable for controller mappings, because JavaScript profile does only workin GUI thread: https://bugreports.qt.io/browse/QTBUG-65419
     *
     */
     public profile(label?:string):void;

    /**
     *  Turns off the JavaScript profiler.
     *  @deprecated Not usable for controller mappings, because JavaScript profile does only workin GUI thread: https://bugreports.qt.io/browse/QTBUG-65419
     *
     */
    public profileEnd(label?:string):void;

    /**
     * Prints an error message together with the stack trace of JavaScript execution at the point where it is called.
     *
     * @param message
     * @param args
     */
    public exception(message?:any, ...args:any[]):void;

    }
var console = new QJSEngineConsoleExtension;
