
/** HidControllerJSProxy */

declare namespace controller {
    /**
     * Sends HID OutputReport with hard coded ReportID 0 to HID device
     *
     * This function only works with HID devices, which don't use ReportIDs
     *
     * @param dataList Data to send as list of bytes
     * @param length Unused but mandatory argument for backwards compatibility
     */
    function send(dataList: number[], length?: number): void;

    /**
     * Sends HID OutputReport to HID device
     *
     * @param dataList Data to send as list of bytes
     * @param length Unused but mandatory argument for backwards compatibility
     * @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use ReportIDs
     * @param useNonSkippingFIFO If set, the report will send in FIFO mode
     *
     *   `false` (default):
     *     - Reports with identical data will be sent only once.
     *     - If reports were superseded by newer data before they could be sent,
     *       the oudated data will be skipped.
     *     - This mode works for all USB HID class compatible reports,
     *       in these each field represents the state of a control (e.g. an LED).
     *     - This mode works best in overload situations, where more reports
     *       are to be sent, than can be processed.
     *
     *  `true`:
     *    - The report will not be skipped under any circumstances,
     *      except FIFO memory overflow.
     *    - All reports with useNonSkippingFIFO set `true` will be send before
     *      any cached report with useNonSkippingFIFO set `false`.
     *    - All reports with useNonSkippingFIFO set `true` will be send in
     *      strict First In / First Out (FIFO) order.
     *    - Limit the use of this mode to the places, where it is really necessary.
     */
    function send(dataList: number[], length: number, reportID: number, useNonSkippingFIFO?: boolean): void;

    /**
     * Sends an OutputReport to HID device
     *
     *  @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use ReportIDs
     *  @param dataArray Data to send as byte array
     *  @param useNonSkippingFIFO If set, the report will send in FIFO mode
     *
     *   `false` (default):
     *     - Reports with identical data will be sent only once.
     *     - If reports were superseded by newer data before they could be sent,
     *       the oudated data will be skipped.
     *     - This mode works for all USB HID class compatible reports,
     *       in these each field represents the state of a control (e.g. an LED).
     *     - This mode works best in overload situations, where more reports
     *       are to be sent, than can be processed.
     *
     *  `true`:
     *    - The report will not be skipped under any circumstances,
     *      except FIFO memory overflow.
     *    - All reports with useNonSkippingFIFO set `true` will be send before
     *      any cached report with useNonSkippingFIFO set `false`.
     *    - All reports with useNonSkippingFIFO set `true` will be send in
     *      strict First In / First Out (FIFO) order.
     *    - Limit the use of this mode to the places, where it is really necessary.
     */
    function sendOutputReport(reportID: number, dataArray: ArrayBuffer, useNonSkippingFIFO?: boolean): void;

    /**
     * getInputReport receives an InputReport from the HID device on request.
     *
     * This can be used on startup to initialize the knob positions in Mixxx
     * to the physical position of the hardware knobs on the controller.
     * This is an optional command in the HID standard - not all devices support it.
     *
     *  @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use
     *  @returns Returns report data with ReportID byte as prefix
     */
    function getInputReport(reportID: number): ArrayBuffer;

    /**
     * Sends a FeatureReport to HID device
     *
     *  @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use
     *  @param reportData Data to send as byte array
     */
    function sendFeatureReport(reportID: number, reportData: ArrayBuffer): void;

    /**
     * getFeatureReport receives a FeatureReport from the HID device on request.
     *
     *  @param reportID 1...255 for HID devices that uses ReportIDs - or 0 for devices, which don't use
     *  @returns The returned array matches the input format of sendFeatureReport,
     *           allowing it to be read, modified and sent it back to the controller.
     */
    function getFeatureReport(reportID: number): ArrayBuffer;
}
