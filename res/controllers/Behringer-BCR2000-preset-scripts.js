/**
 * Mapping of MIDI control values to a preset of a Behringer B-Control BCR2000 controller.
 */
(function(global) {

    /* Controller-specific constants */
    const ROW_SIZE                       = 8;
    const PUSHENCODERGROUP_COUNT         = 4;
    const BUTTONROW_COUNT                = 2;
    const ENCODERROW_COUNT               = 3;
    const BUTTONBOX_SIZE                 = 4;
    const PRESET_MIN                     = 1;
    const PRESET_MAX                     = 32;
    const STATUS_CONTROL_CHANGE          = 0xB0;
    const STATUS_PROGRAM_CHANGE          = 0xC0;

    /* Preset-specific constants */
    const PUSHENCODERGROUP_START         = 0x01;
    const PUSHENCODERGROUP_BUTTON_OFFSET = 0x20;
    const BUTTONROW_START                = 0x41;
    const ENCODERROW_START               = 0x51;
    const BUTTONBOX_START                = 0x69;

    /**
     * Select a preset in the controller.
     *
     * @param {number} A preset number (integer 1..32)
     * @public
     */
    const setPreset = function(presetNumber) {
        if (presetNumber) {
            presetNumber = Math.max(PRESET_MIN, presetNumber);
            presetNumber = Math.min(PRESET_MAX, presetNumber);
            global.midi.sendShortMsg(STATUS_PROGRAM_CHANGE, presetNumber - 1, 0x00);
        }
    };

    /**
     * Create a new array and fill it with elements.
     *
     * @param {number} size Size of the new array
     * @param {bcr2000Preset~elementFactory} elementFactory Element factory
     * @return {Array} Array containing values
     * @private
     */
    const createElements = function(size, elementFactory) {
        return Object.keys(Array.apply(0, Array(size))).map(
            function(_v, i) { return elementFactory.call(this, i); });
    };
    /**
     * A function that creates an element for an index.
     *
     * @callback bcr2000Preset~elementFactory
     * @param {number} index
     * @return {object} Created element
     */

    /**
     * Calculate a range of MIDI addresses.
     *
     * @param {number} startAddress Start address (integer 0..127)
     * @param {number} rangeNumber Range number (positive integer)
     * @param {number} size (optional) Number of addresses within the range; default: ROW_SIZE
     * @return {Array<number>} Array of MIDI addresses for the given range
     * @private
     */
    const calculateRange = function(startAddress, rangeNumber, size) {
        size = size || ROW_SIZE;
        const rangeOffset = rangeNumber * ROW_SIZE;
        const rangeStart = startAddress + rangeOffset;
        return createElements(size, function(i) { return rangeStart + i; });
    };

    /**
     * Create the MIDI address range for the push encoders in a push encoder group.
     *
     * The controller offers 4 groups that may be selected by the buttons in the top right area of
     * the controller. The push encoders are located at the top left area of the controller.
     *
     * @param {number} groupNumber Number of the group (1..4)
     * @return {Array<number>} Address range for the encoders in the encoder group
     * @private
     */
    const createPushEncoderGroup = function(groupNumber) {
        return calculateRange(PUSHENCODERGROUP_START, groupNumber).map(function(encoder) {
            return {"encoder": encoder, "button": encoder + PUSHENCODERGROUP_BUTTON_OFFSET};
        });
    };

    /**
     * Create the MIDI address range for the encoders in an encoder row.
     * The encoder rows are located at the bottom left area of the controller.
     *
     * @param {number} rowNumber Number of the row (1..3)
     * @return {Array<number>} Address range for the encoders in the encoder row
     * @private
     */
    const createEncoderRow = function(rowNumber) {
        return calculateRange(ENCODERROW_START, rowNumber);
    };

    /**
     * Create the MIDI address range for the buttons in a button row.
     * The button rows are located at the top left area of the controller.
     *
     * @param {number} rowNumber Number of the row (1..2)
     * @return {Array<number>} Address range for the buttons in the button row
     * @private
     */
    const createButtonRow = function(rowNumber) {
        return calculateRange(BUTTONROW_START,  rowNumber);
    };

    /**
     * Create the MIDI address range for the buttons in the button box.
     * The button box is located at lower right corner of the controller.
     *
     * @return {Array<number>} Address range for the buttons in the button box
     * @private
     */
    const createButtonBox = function() {
        return calculateRange(BUTTONBOX_START, 0, BUTTONBOX_SIZE);
    };

    /* Definition of MIDI controls */
    const pushEncoderGroups = createElements(PUSHENCODERGROUP_COUNT, createPushEncoderGroup);
    const buttonRows = createElements(BUTTONROW_COUNT, createButtonRow);
    const encoderRows = createElements(ENCODERROW_COUNT, createEncoderRow);
    const buttonBox = createButtonBox();

    const exports = {};
    exports.STATUS_CONTROL_CHANGE = STATUS_CONTROL_CHANGE;
    exports.setPreset             = setPreset;
    exports.pushEncoderGroups     = pushEncoderGroups;
    exports.pushEncoderGroup1     = pushEncoderGroups[0];
    exports.pushEncoderGroup2     = pushEncoderGroups[1];
    exports.pushEncoderGroup3     = pushEncoderGroups[2];
    exports.pushEncoderGroup4     = pushEncoderGroups[3];
    exports.buttonRows            = buttonRows;
    exports.buttonRow1            = buttonRows[0];
    exports.buttonRow2            = buttonRows[1];
    exports.encoderRows           = encoderRows;
    exports.encoderRow1           = encoderRows[0];
    exports.encoderRow2           = encoderRows[1];
    exports.encoderRow3           = encoderRows[2];
    exports.buttonBox             = buttonBox;
    global.BCR2000Preset = exports;
})(this);
