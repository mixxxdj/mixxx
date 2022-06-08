
declare class MidiControllerJSProxy {

    public :ShortMsg(status:number, byte1:number, byte2:number):void;

    /**
     * Alias for :SysexMsg
     * @param dataList
     * @param length
     */
    public :(dataList:number[], length:number=0):void;
    /**
     *
     * @param dataList
     * @param length
     */
    public :SysexMsg(dataList:number[], length:number=0):void;

    }
var controller = new MidiControllerJSProxy;
