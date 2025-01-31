type MidiInputHandler = (channel: number, control: number, value:number, status:number, group:string) => void;

declare interface MidiInputHandlerController {
    disconnect(): boolean;
}

/** MidiControllerJSProxy */

declare namespace midi {

    /**
     * Sends a 3 byte MIDI short message
     *
     * @param status Status byte
     * @param byte1 Data byte 1
     * @param byte2 Data byte 2
     */
    function sendShortMsg(status: number, byte1: number, byte2: number): void;

    /**
     * Alias for {@link midi.sendSysexMsg}
     * Sends a MIDI system-exclusive message of arbitrary number of bytes
     *
     * @param dataList List of bytes to send
     * @param length This is no longer evaluated, and only here for backwards compatibility with old scripts [default = 0]
     */
    function send(dataList: number[], length?: number): void;

    /**
     * Sends a MIDI system-exclusive message of arbitrary number of bytes
     *
     * @param dataList List of bytes to send
     * @param length This is no longer evaluated, and only here for backwards compatibility with old scripts [default = 0]
     */
    function sendSysexMsg(dataList: number[], length?: number): void;

    type InputCallback = (channel: string, control: string, value: number, status: number) => void

    /**
     * Calls the provided callback whenever Mixxx receives a MIDI signal with the first two bytes matching the
     * provided status and midino argument.
     * @param status
     * @param midino
     * @param callback
     * @see https://github.com/mixxxdj/mixxx/wiki/midi%20scripting
     * @see https://github.com/mixxxdj/mixxx/wiki/Midi-Crash-Course
     */
    function makeInputHandler(status: number, midino: number, callback: InputCallback): MidiInputHandlerController
}
