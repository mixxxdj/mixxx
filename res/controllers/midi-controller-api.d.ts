
declare class MidiControllerJSProxy {

    /**
     * Sends a 3 byte MIDI short message
     * @param status Status byte
     * @param byte1 Data byte 1
     * @param byte2 Data byte 2
     */
    public sendShortMsg(status:number, byte1:number, byte2:number):void;

    /**
     * Alias for sendSysexMsg.
     * Sends a MIDI system-exclusive message of arbitrary number of bytes
     * @param dataList List of bytes to send
     * @param length This is no longer evaluated, and only here for backwards compatibility to old scripts
    */
    public send(dataList:number[], length:number=0):void;

    /**
     * Sends a MIDI system-exclusive message of arbitrary number of bytes
     * @param dataList List of bytes to send
     * @param length This is no longer evaluated, and only here for backwards compatibility to old scripts
     */
    public sendSysexMsg(dataList:number[], length:number=0):void;

    }
var controller = new MidiControllerJSProxy;
