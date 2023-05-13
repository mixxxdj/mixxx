
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
     * Alias for {@link sendSysexMsg}
     * Sends a MIDI system-exclusive message of arbitrary number of bytes
     *
     * @param dataList List of bytes to send
     * @param length This is no longer evaluated, and only here for backwards compatibility to old scripts [default = 0]
     */
    function send(dataList: number[], length?: number): void;

    /**
     * Sends a MIDI system-exclusive message of arbitrary number of bytes
     *
     * @param dataList List of bytes to send
     * @param length This is no longer evaluated, and only here for backwards compatibility to old scripts [default = 0]
     */
    function sendSysexMsg(dataList: number[], length?: number): void;

}
