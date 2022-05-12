
declare class MidiControllerJSProxy {

    public sendShortMsg(status : number, byte1 : number, byte2 : number) : void;

    /** Alias for sendSysexMsg */
    public send(dataList : number[], length : number=0) : void;
    public sendSysexMsg(dataList : number[], length : number=0) : void;

    }
var controller = new MidiControllerJSProxy;
