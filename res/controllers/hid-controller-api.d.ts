
declare class HidControllerJSProxy {
    /** Sends HID OutputReport with hard coded ReportID 0 to HID device
     *        This function only works with HID devices, which don't use ReportIDs
     * @param dataList Data to send as list of bytes
     * @param length This optional argument is no longer evaluated, and only here for backwards compatibility
    */
    public send(dataList:number[], length:number=0):void;

    /** Sends HID OutputReport to HID device
     * @param dataList Data to send as list of bytes
     * @param length Unused but mandatory argument
     * @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use ReportIDs
     * @param resendUnchangedReport If set, the report will also be send, if the data are unchanged since last sending
    */
    public send(dataList:number[], length:number, reportID:number, resendUnchangedReport:boolean = false):void;

    /** Sends an OutputReport to HID device
     *  @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use ReportIDs
     *  @param dataArray Data to send as byte array (Javascript type Uint8Array)
     *  @param resendUnchangedReport If set, the report will also be send, if the data are unchanged since last sending
     */
    public sendOutputReport(reportID:number, dataArray:ArrayBuffer, resendUnchangedReport:boolean = false):void;

    /** getInputReport receives an InputReport from the HID device on request.
     *  @details This can be used on startup to initialize the knob positions in Mixxx
     *           to the physical position of the hardware knobs on the controller.
     *           This is an optional command in the HID standard - not all devices support it.
     *  @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use
     *  @return Returns report data with ReportID byte as prefix
     */
    public getInputReport(reportID:number):Uint8Array;

    /** Sends a FeatureReport to HID device
     *  @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use
     *  @param reportData Data to send as byte array
     */
    public sendFeatureReport(reportID:number, reportData:ArrayBuffer):void;

    /** getFeatureReport receives a FeatureReport from the HID device on request.
     *  @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use
     *  @return The returned array matches the input format of sendFeatureReport,
     *          allowing it to be read, modified and sent it back to the controller.
     */
    public getFeatureReport(reportID:number):Uint8Array;
    }
var controller = new HidControllerJSProxy;
