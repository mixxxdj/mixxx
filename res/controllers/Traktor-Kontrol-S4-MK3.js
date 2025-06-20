/// Created by Be <be@mixxx.org> and A. Colombier <mixxx@acolombier.dev>

/********************************************************
                LED Color Constants
 *******************************************************/
// Color codes for controller RGB LEDs
const LedColors = {
    off: 0,
    red: 4,
    carrot: 8,
    orange: 12,
    honey: 16,
    yellow: 20,
    lime: 24,
    green: 28,
    aqua: 32,
    celeste: 36,
    sky: 40,
    blue: 44,
    purple: 48,
    fuscia: 52,
    magenta: 56,
    azalea: 60,
    salmon: 64,
    white: 68,
};

// This define the sequence of color to use for pad button when in keyboard mode. This should make them look like an actual keyboard keyboard octave, except for C, which is green to help spotting it.
const KeyboardColors = [
    LedColors.green,
    LedColors.off,
    LedColors.white,
    LedColors.off,
    LedColors.white,
    LedColors.white,
    LedColors.off,
    LedColors.white,
    LedColors.off,
    LedColors.white,
    LedColors.off,
    LedColors.white,
];

// Constant used to define custom default pad layout
const DefaultPadLayoutHotcue = "hotcue";
const DefaultPadLayoutSamplerBeatloop = "samplerBeatloop";
const DefaultPadLayoutKeyboard = "keyboard";

/********************************************************
                USER CONFIGURABLE SETTINGS

 Change these settings in the Mixxx preferences
 *******************************************************/

const DeckColors = [
    LedColors[engine.getSetting("deckA")] || LedColors.red,
    LedColors[engine.getSetting("deckB")] || LedColors.blue,
    LedColors[engine.getSetting("deckC")] || LedColors.yellow,
    LedColors[engine.getSetting("deckD")] || LedColors.purple,
];

const LibrarySortableColumns = [
    engine.getSetting("librarySortableColumns1Value"),
    engine.getSetting("librarySortableColumns2Value"),
    engine.getSetting("librarySortableColumns3Value"),
    engine.getSetting("librarySortableColumns4Value"),
    engine.getSetting("librarySortableColumns5Value"),
    engine.getSetting("librarySortableColumns6Value"),
].map(c => parseInt(c)).filter(c => c); // Filter '0' column, equivalent to '---' value in the UI or disabled

const LoopWheelMoveFactor = engine.getSetting("loopWheelMoveFactor") || 50;
const LoopEncoderMoveFactor = engine.getSetting("loopEncoderMoveFactor") || 500;
const LoopEncoderShiftMoveFactor = engine.getSetting("loopEncoderShiftMoveFactor") || 2500;

const TempoFaderSoftTakeoverColorLow = LedColors[engine.getSetting("tempoFaderSoftTakeoverColorLow")] || LedColors.white;
const TempoFaderSoftTakeoverColorHigh = LedColors[engine.getSetting("tempoFaderSoftTakeoverColorHigh")] || LedColors.green;

const TempoFaderOffset_L = engine.getSetting("TempoFaderOffset_L") || 0.0;
const TempoFaderOffset_R = engine.getSetting("TempoFaderOffset_R") || 0.0;

// Define whether or not to keep LED that have only one color (reverse, flux, play, shift) dimmed if they are inactive.
// 'true' will keep them dimmed, 'false' will turn them off. Default: true
const InactiveLightsAlwaysBacklit = !!engine.getSetting("inactiveLightsAlwaysBacklit");

// Keep both deck select buttons backlit and do not fully turn off the inactive deck button.
// 'true' will keep the unselected deck dimmed, 'false' to fully turn it off. Default: true
const DeckSelectAlwaysBacklit = !!engine.getSetting("deckSelectAlwaysBacklit");

// Define whether the keylock is mapped when doing "shift+master" (on press) or "shift+sync" (on release since long push copies the key)".
// 'true' will use "sync+master", 'false' will use "shift+sync". Default: false
const UseKeylockOnMaster = !!engine.getSetting("useKeylockOnMaster");

// Define whether the grid button would blink when the playback is going over a detected beat. Can help to adjust beat grid.
// Default: false
const GridButtonBlinkOverBeat = !!engine.getSetting("gridButtonBlinkOverBeat");

// Wheel led blinking if reaching the end of track warning (default 30 seconds, can be changed in the settings, under "Waveforms" > "End of track warning").
// Default: true
const WheelLedBlinkOnTrackEnd = !!engine.getSetting("wheelLedBlinkOnTrackEnd");

// When shifting either decks, the mixer will control microphones or auxiliary lines. If there is both a mic and an configure on the same channel, the mixer will control the auxiliary.
// Default: false
const MixerControlsMixAuxOnShift = !!engine.getSetting("mixerControlsMicAuxOnShift");

// Make the sampler tab a beatlooproll tab instead
// Default: false
const UseBeatloopRollInsteadOfSampler = !!engine.getSetting("useBeatloopRollInsteadOfSampler");

// Predefined beatlooproll sizes. Note that if you use AddLoopHalveAndDoubleOnBeatloopRollTab, the first and
// last size will be ignored
const BeatLoopRolls = [
    engine.getSetting("beatLoopRollsSize1") || 1/8,
    engine.getSetting("beatLoopRollsSize2") || 1/4,
    engine.getSetting("beatLoopRollsSize3") || 1/2,
    engine.getSetting("beatLoopRollsSize4") || 1,
    engine.getSetting("beatLoopRollsSize5") || 2,
    engine.getSetting("beatLoopRollsSize6") || 4,
    engine.getSetting("beatLoopRollsSize7") || "half",
    engine.getSetting("beatLoopRollsSize8") || "double"
];


// Define the speed of the jogwheel. This will impact the speed of the LED playback indicator, the scratch, and the speed of
// the motor if enable. Recommended value are 33 + 1/3 or 45.
// Default: 33 + 1/3
const BaseRevolutionsPerMinute = engine.getSetting("baseRevolutionsPerMinute") || 100/3; // aka 33 + 1/3

// Define whether or not to use motors.
// This is a BETA feature! Please use at your own risk. Setting this off means that below settings are inactive
// Default: false
const UseMotors = !!engine.getSetting("useMotors");

// Define whether or not the jog wheel plater provide a haptic feedback when going over the cue point.
// Default: true
const CueHapticFeedback = UseMotors && !!engine.getSetting("cueHapticFeedback");

// Define how much the wheel will resist. It is a similar setting that the Grid+Wheel in Tracktor
// Value must defined between 0 to 1. 0 is very tight, 1 is very loose.
// Default: 0.5
const TightnessFactor = engine.getSetting("tightnessFactor") || 0.5;

// Define how much force can the motor use. This defines how much the wheel will "fight" you when you block it in TT mode
// This will also affect how quick the wheel starts spinning when enabling motor mode, or starting a deck with motor mode on
const MaxWheelForce = engine.getSetting("maxWheelForce") || 60000;  // Traktor seems to cap the max value at 60000, which just sounds insane
// But insane or not, it makes sense to follow the manufacturer's spec

// Map the mixer potentiometers to different components of the software mixer in Mixxx, on top of the physical control of the hardware
// mixer embedded in the S4 Mk3. This is useful if you are not using certain S4 Mk3 outputs.
const SoftwareMixerMain = !!engine.getSetting("softwareMixerMain");
const SoftwareMixerBooth = !!engine.getSetting("softwareMixerBooth");
const SoftwareMixerHeadphone = !!engine.getSetting("softwareMixerHeadphone");

// Define custom default layout used by the pads, instead of intro/outro  and first 4 hotcues.
const DefaultPadLayout = engine.getSetting("defaultPadLayout");

// Motor PID controller coefficients
const ProportionalGain = engine.getSetting("proportionalGain") || 80000;
const IntegrativeGain = engine.getSetting("integrativeGain") || 1000;
const DerivativeGain = engine.getSetting("derivativeGain") || 50000;
// Force that simulates the pull of a slipmat when scratching
const SlipFrictionForce = engine.getSetting("slipFrictionForce") || 12000;

// friction is inherently compensated in the PID controller
// const MOTOR_FRICTION_COMPENSATION = engine.getSetting("MOTOR_FRICTION_COMPENSATION") || 0;
//TODO: move motorized nudge sensitivity here

// The LEDs only support 16 base colors. Adding 1 in addition to
// the normal 2 for Button.prototype.brightnessOn changes the color
// slightly, so use that get 25 different colors to include the Filter
// button as a 5th effect chain preset selector.
const QuickEffectPresetColors = [
    LedColors.red,
    LedColors.green,
    LedColors.blue,
    LedColors.yellow,
    LedColors.orange,
    LedColors.purple,
    LedColors.white,

    LedColors.magenta,
    LedColors.azalea,
    LedColors.salmon,
    LedColors.red + 1,

    LedColors.sky,
    LedColors.celeste,
    LedColors.fuscia,
    LedColors.blue + 1,

    LedColors.carrot,
    LedColors.honey,
    LedColors.yellow + 1,

    LedColors.lime,
    LedColors.aqua,
    LedColors.purple + 1,

    LedColors.magenta + 1,
    LedColors.azalea + 1,
    LedColors.salmon + 1,
    LedColors.fuscia + 1,
];

/********************************************************
                MIXER CONSTANTS
 *******************************************************/
// assign samplers to the crossfader on startup
const SamplerCrossfaderAssign = true;

/********************************************************
                JOGWHEEL-RELATED CONSTANTS
 *******************************************************/
// motor wind up/down
const MotorWindUpMilliseconds = 0;
const MotorWindDownMilliseconds = 0;

// input filtering. Coefficients generated in scipy
// for a 5-tap filter given a sampling rate of 500Hz
// and applying a hamming window
// const VelFilterTaps = 5;
// 50Hz filter
// const VelFilterCoeffs = [0.02840647, 0.23700821, 0.46917063, 0.23700821, 0.02840647];
// 10Hz filter
// const VelFilterCoeffs = [0.03541093, 0.24092353, 0.44733108, 0.24092353, 0.03541093];

// 9-tap filters:
const VelFilterTaps = 9;

// simple sliding average. If we need a target tick per measure of 3.2, but we can only get
// whole numbers as inputs, then we need at least 5 samples to get a steady velocity of 3.2
// const VelFilterCoeffs = [1/9, 1/9, 1/9, 1/9, 1/9, 1/9, 1/9, 1/9, 1/9];

// 10Hz weighted average lowpass FIR filter.
const VelFilterCoeffs = [0.01755602, 0.04801081, 0.12234688, 0.19760069, 0.2289712, 0.19760069, 0.12234688, 0.04801081, 0.01755602]

// Maximum position value for 32-bit unsigned integer
const wheelPositionMax = 2 ** 32 - 1;
//const wheelAbsoluteMax = 2879; //FIXME: nomenclature
// I think it's safe to make this 2880 as it should be.
// AFAICT the fractional revolution that gets multiplied
// with this value cannot in practice reach 2880
const wheelAbsoluteMax = 2880; //FIXME: nomenclature

const wheelTimerMax = 2 ** 32 - 1;
const wheelClockFreq = 100000000; // One tick every 10ns (100MHz)

// Establish rotational constants for wheel math

// Target motor outputs will ideally be calibrated based on real hardware data specific to the device
// so that variations of construction and wear can be accomodated. For now, these outputs are based on
// a data collection experiment I performed during testing --ZT
const TargetMotorOutput33RPM = 4600; //measured in a rough calibration test, not exact. TODO: refine this value
const TargetMotorOutput45RPM = 5600; //measured in a rough calibration test, not exact. TODO: refine this value

// Make sure the RPM for 33.3 is as precise as possible,
// And set the target motor output for nudging
let rps = 0;
let TargetMotorOutput = 0;
if (BaseRevolutionsPerMinute == 33) {
    rps = (100/3) / 60;
    TargetMotorOutput = TargetMotorOutput33RPM;
} else { // 45RPM
    rps = BaseRevolutionsPerMinute / 60;
    TargetMotorOutput = TargetMotorOutput45RPM;
}
const BaseRevolutionsPerSecond = rps;
const baseDegreesPerSecond = BaseRevolutionsPerSecond * 360;
const baseEncoderTicksPerDegree = baseDegreesPerSecond * 8;


const MotorOutSmoothingFactor = 1/5; // 0.5; // smaller is smoother but slower

// Slipmat starts slipping when this error level is surpassed
const SlipmatErrorThresh = 0.05; // 5% velocity tolerance for slipping

// Integrator suppression slip threshold
// this is an attempt to stop the cumulative error from causing big overshoots
// when adjusting via the crown. Without this, the integrator will grow more and more
// during the adjustment, and when you finally let go it slams back so hard that it can
// stop the platter. You can feel this as an increasing resistance to the crown adjustment.
// suppressing the integrator beyond a certain error threshold solves this issue, and
// causes no detrimental effects as long as the error threshold is large enough. If the
// threshold is too small (below 0.2 in my testing) the integrator loses its power
// and the turntable won't be able to reach the target angular velocity.
const IntegratorSuppressionErrorThresh = 0.3;

// Even with a "perfectly" tuned PID controller, there will always be some steady-state
// oscillation (flutter). So when we're within a certain threshold of the target rate,
// we will disable scratch mode
// const PLAYBACK_QUANTIZE_ERROR_THRESH = 0.05;
const PlaybackQuantizeErrorStep = 0.4

// TEMPORARY. These are configuration values for a motor test routine that I used to collect
// data for output-to-input mapping. -ZT
// const S4MK3DEBUG = true;
const S4MK3MOTORTEST_ENABLE = false;
const S4MK3MOTORTEST_UPTIME = 10000; //milliseconds
const S4MK3MOTORTEST_DOWNTIME = 0; //milliseconds
const S4MK3MOTORTEST_STARTLVL= 4500;
const S4MK3MOTORTEST_STEPSIZE = 0;
const S4MK3MOTORTEST_ENDLVL = 6000;
const NONSLIP_PITCH_SMOOTHING = 0.5;
// const NONSLIP_OUTPUT_TRACKING_SMOOTHING = 1/20;

// TODO: make nudge sensitivity a user-defineable value
const TurnTableNudgeSensitivity = 0.1;

/********************************************************
                HID REPORT IDs
 *******************************************************/
// =======
// INPUTS:
// =======
// There are 3 types of input reports. Components in the
// mapping get associated with one of these three reports,
// or if left undefined, have a default association given
// by their class constructor.

// All button/boolean interface elements
const HIDInputButtonsReportID = 1

// Potentiometers, and a couple of FX select buttons
const HIDInputPotsReportID = 2

// Wheel position and timing data
// Also includes touch sensors
// (touch sensors are also sent along with button report)
const HIDInputWheelsReportID = 3

// =======
// OUTPUTS:
// =======
const HIDOutputMotorsReportID = 49
const HIDOutputVUMeterReportID = 129;

/********************************************************
                HARDWARE ADDRESSES
 *******************************************************/
//TODO

/*
 ========================================================

                CLASS DEFINITIONS

 ========================================================
*/


 /*
 * Circular buffer for running FIR filter.
 * This is used for low-passing the incoming velocity data
 * before it reaches the motor controller. Uses a 
 * weighted moving-average to implement a
 * FIR filter whose coefficients were generated separately
 * in Python with Scipy
 * 
 * Resource links and Python code:
 * https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.firwin.html
 * https://howthefouriertransformworks.com/2022/12/23/how-fir-filters-work-applying-the-filter/
 * filt_taps = 5
 * filt_cutoff = 50 # Hertz
 * filt_samplingFreq = 500 # Hertz. Assumes the position is coming in every 2ms
 * filt_window = 'hamming'
 * filt_lpf = scipy.signal.firwin(filt_taps,filt_cutoff,window=filt_window,fs=filt_samplingFreq)
 */
class FilterBuffer {
    constructor(numElements, filterCoefficients) {
        this.size = numElements;
        this.data = new Float64Array(numElements);
        // filterCoefficients should be an array of floats
        // the same size as the buffer
        this.coeffs = filterCoefficients
        // this buffer will run 'backwards' to make
        // convolution simpler, so the pointer
        // starts at the end of the buffer. Doesn't
        // really matter though, it could start anywhere
        this.writePt = this.size-1;

        // hold the last calculated output value for reference
        this.velFiltered = 0;

        // initialize the buffer with zeros
        for (let i = 0; i < this.size; i++) {
            this.data[i] = 0;
        } 
    }
    // get the most recent output result
    getCurrentVel() {
        return this.velFiltered;
    }
    // Produce the next filtered sample
    runFilter() {
        let runningSum = 0;
        // Starting at the write pointer,
        // multiply the next N elements
        // with sequential values from the
        // coefficients array, summing the
        // results.
        for (let i = 0; i < this.size; i++) {
            runningSum += this.coeffs[i] * this.data[(this.writePt + i)%this.size];
        }
        this.velFiltered = runningSum;
        return runningSum;
    }
    // Write a new value to the buffer, forgetting the oldest one
    insert(newVel) {
        // First decrement (and wrap) the write pointer
        this.writePt--;
        if (this.writePt < 0) {
            this.writePt = this.size-1;
        }
        // Second replace the value at the write pointer
        this.data[this.writePt] = newVel;
    }
}

/*
 * Motor output buffer manager
 * 
 * This initializes and manages a single data buffer
 * for the jogwheel motor commands. It takes care of
 * using the correct codes for CW/CCW rotation, and
 * streamlines the data access so that we don't have
 * to declare unnecessary Uint8Arrays in the time-sensitive
 * code blocks such as S4Mk3MotorManager.tick()
 */
const MotorBuffIDLeft = 0;
const MotorBuffIDRight = 1;

const MotorBuffOffsetLeft = 0;
const MotorBuffOffsetRight = 5;
// There are 2 instruction bytes that specify forward or reverse
// motor direction.
const MotorDirFwd = 0;
const MotorDirRev = 1;
const MotorBuffFwdCode01 = 0x20;
const MotorBuffFwdCode02 = 0x01;
const MotorBuffRevCode01 = 0xe0;
const MotorBuffRevCode02 = 0xfe;

class MotorOutputBuffMgr {
    constructor() {
        // the output buffer itself:
        this.outputBuffer = new Uint8Array([
            1, MotorBuffFwdCode01, MotorBuffFwdCode02, 0, 0,
            1, MotorBuffFwdCode01, MotorBuffFwdCode02, 0, 0,
        ]);
        this.dir = new Uint8Array([MotorDirFwd,MotorDirFwd]);
        this.maxOutput = MaxWheelForce;
    }
    getBuff() {
        return this.outputBuffer.buffer;
    }
    setMaxTorque(newMaxOutput) {
        // Set a different maximum output torque.
        // This is used during slip mode to simulate
        // slipmat friction force 
        // instead of direct drive motor force
        if (newMaxOutput > 0 && newMaxOutput < MaxWheelForce) {
            this.maxOutput = newMaxOutput;
        }
        // if it's not within a valid range,
        // set it to default.
        else {
            this.maxOutput = MaxWheelForce;
        }
    }
    setMotorOutput(motorID,outputLvl=0) {
        let offset = 0;

        // Make sure the output drive is an integer:
        let outputInt = Math.round(outputLvl);

        // Set the correct offset for this motor's data in the output buffer
        if (motorID == MotorBuffIDLeft) {
            offset = MotorBuffOffsetLeft;
        }
        else if (motorID == MotorBuffIDRight) {
            offset = MotorBuffOffsetRight;
        }
        else {
            // invalid motor ID
            return -1; // error
        }

        // Setting the direction of the motor.
        // If the output is negative
        if (outputInt < 0) {
            // remove the negative sign
            outputInt = -outputInt;
            // if the direction was previously forward, set reverse
            // and update the direction code
            if (this.dir[motorID] == MotorDirFwd) {
                this.dir[motorID] = MotorDirRev;
                this.outputBuffer[offset + 1] = MotorBuffRevCode01;
                this.outputBuffer[offset + 2] = MotorBuffRevCode02;
            }
        }
        // Otherwise, the output is zero or positive
        else {
            // if the direction was previously reverse, set forward
            // and update the direction code
            if (this.dir[motorID] == MotorDirRev) {
                this.dir[motorID] = MotorDirFwd;
                this.outputBuffer[offset + 1] = MotorBuffFwdCode01;
                this.outputBuffer[offset + 2] = MotorBuffFwdCode02;
            }
        }

        // Finally, write the output data into the output buffer.
        // It is little endian, so write the 2 bytes thusly
        this.outputBuffer[offset + 3] = outputInt & 0xff;
        this.outputBuffer[offset + 4] = outputInt >> 8;

        return 0;
    }
}

/*
 * HID report parsing library
 */
class HIDInputReport {
    constructor(reportId) {
        this.reportId = reportId;
        this.fields = [];
    }

    registerCallback(callback, byteOffset, bitOffset = 0, bitLength = 1, defaultOldData = undefined) {
        if (typeof callback !== "function") {
            throw Error("callback must be a function");
        }

        if (!Number.isInteger(byteOffset)) {
            throw Error("byteOffset must be 0 or a positive integer");
        }
        if (!Number.isInteger(bitOffset) || bitOffset < 0) {
            throw Error("bitOffset must be 0 or a positive integer");
        }
        if (!Number.isInteger(bitOffset) || bitLength < 1 || bitLength > 32) {
            throw Error("bitLength must be an integer between 1 and 32");
        }

        const field = {
            callback: callback,
            byteOffset: byteOffset,
            bitOffset: bitOffset,
            bitLength: bitLength,
            oldData: defaultOldData
        };
        this.fields.push(field);

        return {
            disconnect: () => {
                this.fields = this.fields.filter((element) => {
                    return element !== field;
                });
            }
        };
    }

    handleInput(reportData) {
        const view = new DataView(reportData);

        for (const field of this.fields) {
            const numBytes = Math.ceil(field.bitLength / 8);
            let data;

            // Little endianness is specified by the HID standard.
            // The HID standard allows signed integers as well, but I am not aware
            // of any HID DJ controllers which use signed integers.
            if (numBytes === 1) {
                data = view.getUint8(field.byteOffset);
            } else if (numBytes === 2) {
                data = view.getUint16(field.byteOffset, true);
            } else if (numBytes === 3) {
                data = view.getUint32(field.byteOffset, true) >>> 8;
            } else if (numBytes === 4) {
                data = view.getUint32(field.byteOffset, true);
            } else {
                throw Error("field bitLength must be between 1 and 32");
            }

            // The >>> 0 is required for 32 bit unsigned ints to not magically turn negative
            // because all Numbers are really 32 bit signed floats. Because JavaScript.
            data = ((data >> field.bitOffset) & (2 ** field.bitLength - 1)) >>> 0;

            if (field.oldData !== data) {
                field.callback(data);
                field.oldData = data;
            }
        }
    }
}

// Q: Is HIDOutputReport ever used?
// A: only once, in this script, and nowhere else in Mixxx's codebase
class HIDOutputReport {
    constructor(reportId, length) {
        this.reportId = reportId;
        this.data = new Uint8Array(length).fill(0);
    }
    send() {
        controller.sendOutputReport(this.reportId, this.data.buffer);
    }
}

/*
 * Components library
 */

class Component {
    constructor(options) {
        if (options) {
            Object.keys(options).forEach(function(key) {
                if (options[key] === undefined) { delete options[key]; }
            });
            Object.assign(this, options);
        }
        this.outConnections = [];
        if (typeof this.key === "string") {
            this.inKey = this.key;
            this.outKey = this.key;
        }
        if (typeof this.unshift === "function" && this.unshift.length === 0) {
            this.unshift();
        }
        this.shifted = false;
        if (typeof this.input === "function" && this.inReport instanceof HIDInputReport && this.inReport.length === 0) {
            this.inConnect();
        }
        this.outConnect();
    }
    inConnect(callback) {
        if (this.inByte === undefined
            || this.inBit === undefined
            || this.inBitLength === undefined
            || this.inReport === undefined) {
            return;
        }
        if (typeof callback === "function") {
            this.input = callback;
        }
        this.inConnection = this.inReport.registerCallback(this.input.bind(this), this.inByte, this.inBit, this.inBitLength, this.oldDataDefault);
    }
    inDisconnect() {
        if (this.inConnection !== undefined) {
            this.inConnection.disconnect();
        }
    }
    send(value) {
        if (this.outReport !== undefined && this.outByte !== undefined) {
            this.outReport.data[this.outByte] = value;
            this.outReport.send();
        }
    }
    output(value) {
        this.send(value);
    }
    outConnect() {
        if (this.outKey !== undefined && this.group !== undefined) {
            const connection = engine.makeConnection(this.group, this.outKey, this.output.bind(this));
            // This is useful for case where effect would have been fully disabled in Mixxx. This appears to be the case during unit tests.
            if (connection) {
                this.outConnections[0] = connection;
            } else {
                console.warn(`Unable to connect ${this.group}.${this.outKey}' to the controller output. The control appears to be unavailable.`);
            }
        }
    }
    outDisconnect() {
        for (const connection of this.outConnections) {
            connection.disconnect();
        }
        this.outConnections = [];
    }
    outTrigger() {
        for (const connection of this.outConnections) {
            connection.trigger();
        }
    }
}
class ComponentContainer extends Component {
    constructor() {
        super();
    }
    *[Symbol.iterator]() {
        // can't use for...of here because it would create an infinite loop
        for (const property in this) {
            if (Object.prototype.hasOwnProperty.call(this, property)) {
                const obj = this[property];
                if (obj instanceof Component) {
                    yield obj;
                } else if (Array.isArray(obj)) {
                    for (const objectInArray of obj) {
                        if (objectInArray instanceof Component) {
                            yield objectInArray;
                        }
                    }
                }
            }
        }
    }
    reconnectComponents(callback) {
        for (const component of this) {
            if (typeof component.outDisconnect === "function" && component.outDisconnect.length === 0) {
                component.outDisconnect();
            }
            if (typeof callback === "function" && callback.length === 1) {
                callback.call(this, component);
            }
            if (typeof component.outConnect === "function" && component.outConnect.length === 0) {
                component.outConnect();
            }
            component.outTrigger();
            if (typeof component.unshift === "function" && component.unshift.length === 0) {
                component.unshift();
            }
        }
    }
    unshift() {
        for (const component of this) {
            if (typeof component.unshift === "function" && component.unshift.length === 0) {
                component.unshift();
            }
            component.shifted = false;
        }
        this.shifted = false;
    }
    shift() {
        for (const component of this) {
            if (typeof component.shift === "function" && component.shift.length === 0) {
                component.shift();
            }
            component.shifted = true;
        }
        this.shifted = true;
    }
}

/* eslint no-redeclare: "off" */
class Deck extends ComponentContainer {
    constructor(decks, colors) {
        super();
        if (typeof decks === "number") {
            this.group = Deck.groupForNumber(decks);
        } else if (Array.isArray(decks)) {
            this.decks = decks;
            this.currentDeckNumber = decks[0];
            this.group = Deck.groupForNumber(decks[0]);
        }
        if (colors !== undefined && Array.isArray(colors)) {
            this.groupsToColors = {};
            let index = 0;
            for (const deck of this.decks) {
                this.groupsToColors[Deck.groupForNumber(deck)] = colors[index];
                index++;
            }
            this.color = colors[0];
        }
        this.secondDeckModes = null;
    }
    toggleDeck() {
        if (this.decks === undefined) {
            throw Error("toggleDeck can only be used with Decks constructed with an Array of deck numbers, for example [1, 3]");
        }

        const currentDeckIndex = this.decks.indexOf(this.currentDeckNumber);
        let newDeckIndex = currentDeckIndex + 1;
        if (currentDeckIndex >= this.decks.length) {
            newDeckIndex = 0;
        }

        this.switchDeck(Deck.groupForNumber(this.decks[newDeckIndex]));
    }
    switchDeck(newGroup) {
        const currentModes = {
            wheelMode: this.wheelMode,
            moveMode: this.moveMode,
        };

        engine.setValue(this.group, "scratch2_enable", false);
        this.group = newGroup;
        this.color = this.groupsToColors[newGroup];

        if (this.secondDeckModes !== null) {
            this.wheelMode = this.secondDeckModes.wheelMode;
            this.moveMode = this.secondDeckModes.moveMode;

            if (this.wheelMode === wheelModes.motor) {
                engine.beginTimer(MotorWindUpMilliseconds, () => {
                    engine.setValue(newGroup, "scratch2_enable", false);
                }, true);
            }
        }
        this.reconnectComponents(function(component) {
            if (component.group === undefined
                || component.group.search(script.channelRegEx) !== -1) {
                component.group = newGroup;
            } else if (component.group.search(script.eqRegEx) !== -1) {
                component.group = `[EqualizerRack1_${newGroup}_Effect1]`;
            } else if (component.group.search(script.quickEffectRegEx) !== -1) {
                component.group = `[QuickEffectRack1_${newGroup}]`;
            }

            component.color = this.groupsToColors[newGroup];
        });
        this.secondDeckModes = currentModes;
    }
    static groupForNumber(deckNumber) {
        return `[Channel${deckNumber}]`;
    }
}

class Button extends Component {
    constructor(options) {
        options.oldDataDefault = 0;

        super(options);

        if (this.input === undefined) {
            this.input = this.defaultInput;
            if (typeof this.input === "function"
                && this.inReport instanceof HIDInputReport
                && this.input.length === 0) {
                this.inConnect();
            }
        }

        if (this.longPressTimeOutMillis === undefined) {
            this.longPressTimeOutMillis = 225;
        }
        if (this.indicatorIntervalMillis === undefined) {
            this.indicatorIntervalMillis = 350;
        }
        this.longPressTimer = 0;
        this.indicatorTimer = 0;
        this.indicatorState = false;
        this.isLongPress = false;
        if (this.inBitLength === undefined) {
            this.inBitLength = 1;
        }
    }
    setKey(key) {
        this.inKey = key;
        if (key === this.outKey) {
            return;
        }
        this.outDisconnect();
        this.outKey = key;
        this.outConnect();
        this.outTrigger();
    }
    setGroup(group) {
        if (group === this.group) {
            return;
        }
        this.outDisconnect();
        this.group = group;
        this.outConnect();
        this.outTrigger();
    }
    output(value) {
        if (this.indicatorTimer !== 0) {
            return;
        }
        const brightness = (value > 0) ? this.brightnessOn : this.brightnessOff;
        this.send((this.color || LedColors.white) + brightness);
    }
    indicatorCallback() {
        this.indicatorState = !this.indicatorState;
        this.send((this.indicatorColor || this.color || LedColors.white) + (this.indicatorState ? this.brightnessOn : this.brightnessOff));
    }
    indicator(on) {
        if (on && this.indicatorTimer === 0) {
            this.outDisconnect();
            this.indicatorTimer = engine.beginTimer(this.indicatorIntervalMillis, this.indicatorCallback.bind(this));
        } else if (!on && this.indicatorTimer !== 0) {
            engine.stopTimer(this.indicatorTimer);
            this.indicatorTimer = 0;
            this.indicatorState = false;
            this.outConnect();
            this.outTrigger();
        }
    }
    defaultInput(pressed) {
        if (pressed) {
            this.isLongPress = false;
            if (typeof this.onShortPress === "function" && this.onShortPress.length === 0) { this.onShortPress(); }
            if ((typeof this.onLongPress === "function" && this.onLongPress.length === 0) || (typeof this.onLongRelease === "function" && this.onLongRelease.length === 0)) {
                this.longPressTimer = engine.beginTimer(this.longPressTimeOutMillis, () => {
                    this.isLongPress = true;
                    this.longPressTimer = 0;
                    if (typeof this.onLongPress !== "function") { return; }
                    this.onLongPress(this);
                }, true);
            }
        } else if (this.isLongPress) {
            if (typeof this.onLongRelease === "function" && this.onLongRelease.length === 0) { this.onLongRelease(); }
        } else {
            if (this.longPressTimer !== 0) {
                engine.stopTimer(this.longPressTimer);
                this.longPressTimer = 0;
            }
            if (typeof this.onShortRelease === "function" && this.onShortRelease.length === 0) { this.onShortRelease(); }
        }
    }
}

class PushButton extends Button {
    constructor(options) {
        super(options);
    }
    input(pressed) {
        engine.setValue(this.group, this.inKey, pressed);
    }
}

class ToggleButton extends Button {
    constructor(options) {
        super(options);
    }
    onShortPress() {
        script.toggleControl(this.group, this.inKey, true);
    }
}

class TriggerButton extends Button {
    constructor(options) {
        super(options);
    }
    onShortPress() {
        engine.setValue(this.group, this.inKey, true);
    }
    onShortRelease() {
        engine.setValue(this.group, this.inKey, false);
    }
}

class PowerWindowButton extends Button {
    constructor(options) {
        super(options);
        this.isLongPressed = false;
        this.longPressTimer = 0;
    }
    onShortPress() {
        script.toggleControl(this.group, this.inKey);
    }
    onLongRelease() {
        script.toggleControl(this.group, this.inKey);
    }
}

class PlayButton extends Button {
    constructor(options) {
        // Prevent accidental ejection/duplication accident
        options.longPressTimeOutMillis = 800;
        super(options);
        this.inKey = "play";
        this.outKey = "play_indicator";
        this.outConnect();
    }
    onShortPress() {
        script.toggleControl(this.group, this.inKey, true);
    }
    onLongPress() {
        if (this.shifted) {
            engine.setValue(this.group, this.inKey, false);
            script.triggerControl(this.group, "eject");
        } else if (!engine.getValue(this.group, this.inKey)) {
            script.triggerControl(this.group, "CloneFromDeck");
        }
    }
}

class CueButton extends PushButton {
    constructor(options) {
        super(options);
        this.outKey = "cue_indicator";
        this.outConnect();
    }
    unshift() {
        this.inKey = "cue_default";
    }
    shift() {
        this.inKey = "start_stop";
    }
    input(pressed) {
        if (this.deck.moveMode === moveModes.keyboard && !this.deck.keyboardPlayMode) {
            this.deck.assignKeyboardPlayMode(this.group, this.inKey);
        } else if (this.deck.wheelMode === wheelModes.motor && engine.getValue(this.group, "play") && pressed) {
            engine.setValue(this.group, "cue_goto", pressed);
        } else {
            engine.setValue(this.group, this.inKey, pressed);
            if (this.deck.wheelMode === wheelModes.motor) {
                engine.setValue(this.group, "scratch2_enable", false);
                engine.beginTimer(MotorWindDownMilliseconds, () => {
                    engine.setValue(this.group, "scratch2_enable", false);
                }, true);
            }
        }
    }
}

class Encoder extends Component {
    constructor(options) {
        super(options);
        this.lastValue = null;
    }
    input(value) {
        const oldValue = this.lastValue;
        this.lastValue = value;

        if (oldValue === null || typeof this.onChange !== "function") {
            // This scenario happens at the controller initialisation. No real input to proceed
            return;
        }
        let isRight;
        if (oldValue === this.max && value === 0) {
            isRight = true;
        } else if (oldValue === 0 && value === this.max) {
            isRight = false;
        } else {
            isRight = value > oldValue;
        }
        this.onChange(isRight);
    }
}

/*
 * Represent a pad button that interact with a hotcue (set, activate or clear)
 */
class HotcueButton extends PushButton {
    constructor(options) {
        super(options);
        if (this.number === undefined || !Number.isInteger(this.number) || this.number < 1 || this.number > 32) {
            throw Error("HotcueButton must have a number property of an integer between 1 and 32");
        }
        this.outKey = `hotcue_${this.number}_status`;
        this.colorKey = `hotcue_${this.number}_color`;
        this.outConnect();
    }
    unshift() {
        this.inKey = `hotcue_${this.number}_activate`;
    }
    shift() {
        this.inKey = `hotcue_${this.number}_clear`;
    }
    input(pressed) {
        engine.setValue(this.group, "scratch2_enable", false);
        engine.setValue(this.group, this.inKey, pressed);
    }
    output(value) {
        if (value) {
            this.send(this.color + this.brightnessOn);
        } else {
            this.send(LedColors.off);
        }
    }
    outConnect() {
        if (undefined !== this.group) {
            const connection0 = engine.makeConnection(this.group, this.outKey, this.output.bind(this));
            if (connection0) {
                this.outConnections[0] = connection0;
            } else {
                console.warn(`Unable to connect ${this.group}.${this.outKey}' to the controller output. The control appears to be unavailable.`);
            }
            const connection1 = engine.makeConnection(this.group, this.colorKey, (colorCode) => {
                this.color = this.colorMap.getValueForNearestColor(colorCode);
                this.output(engine.getValue(this.group, this.outKey));
            });
            if (connection1) {
                this.outConnections[1] = connection1;
            } else {
                console.warn(`Unable to connect ${this.group}.${this.colorKey}' to the controller output. The control appears to be unavailable.`);
            }
        }
    }
}

/*
 * Represent a pad button that acts as a keyboard key. Depending the deck keyboard mode, it will either change the key, or play the cue with the button's key
 */
class KeyboardButton extends PushButton {
    constructor(options) {
        super(options);
        if (this.number === undefined || !Number.isInteger(this.number) || this.number < 1 || this.number > 8) {
            throw Error("KeyboardButton must have a number property of an integer between 1 and 8");
        }
        if (this.deck === undefined) {
            throw Error("KeyboardButton must have a deck attached to it");
        }
        this.outConnect();
    }
    unshift() {
        this.outTrigger();
    }
    shift() {
        this.outTrigger();
    }
    input(pressed) {
        const offset = this.deck.keyboardOffset - (this.shifted ? 8 : 0);
        if (this.number + offset < 1 || this.number + offset > 24) {
            return;
        }
        if (pressed) {
            engine.setValue(this.group, "key", this.number + offset);
        }
        if (this.deck.keyboardPlayMode !== null) {
            if (this.deck.keyboardPlayMode.activeKey && pressed) {
                engine.setValue(this.deck.keyboardPlayMode.group, "cue_goto", pressed);
            } else if (!this.deck.keyboardPlayMode.activeKey || this.deck.keyboardPlayMode.activeKey === this) {
                script.toggleControl(this.deck.keyboardPlayMode.group, this.deck.keyboardPlayMode.action, true);
            }
            if (!pressed && this.deck.keyboardPlayMode.activeKey === this) {
                this.deck.keyboardPlayMode.activeKey = undefined;
            } else if (pressed) {
                this.deck.keyboardPlayMode.activeKey = this;
            }
        }
    }
    output(value) {
        const offset = this.deck.keyboardOffset - (this.shifted ? 8 : 0);
        const colorIdx = (this.number - 1 + offset) % KeyboardColors.length;
        const color = KeyboardColors[colorIdx];
        if (this.number + offset < 1 || this.number + offset > 24) {
            this.send(0);
        } else {
            this.send(color + (value ? this.brightnessOn : this.brightnessOff));
        }
    }
    outConnect() {
        if (undefined !== this.group) {
            const connection = engine.makeConnection(this.group, "key", (key) => {
                const offset = this.deck.keyboardOffset - (this.shifted ? 8 : 0);
                this.output(key === this.number + offset);
            });
            if (connection) {
                this.outConnections[0] = connection;
            } else {
                console.warn(`Unable to connect ${this.group}.key' to the controller output. The control appears to be unavailable.`);
            }
        }
    }
}

/*
 * Represent a pad button that will trigger a pre-defined beatloop size as set in BeatLoopRolls.
 */
class BeatLoopRollButton extends TriggerButton {
    constructor(options) {
        if (options.number === undefined || !Number.isInteger(options.number) || options.number < 0 || options.number > 7) {
            throw Error("BeatLoopRollButton must have a number property of an integer between 0 and 7");
        }
        if (BeatLoopRolls[options.number] === "half") {
            options.key = "loop_halve";
        } else if (BeatLoopRolls[options.number] === "double") {
            options.key = "loop_double";
        } else {
            const size = parseFloat(BeatLoopRolls[options.number]);
            if (isNaN(size)) {
                throw Error(`BeatLoopRollButton ${options.number}'s size "${BeatLoopRolls[options.number]}" is invalid. Must be a float, or the literal 'half' or 'double'`);
            }
            options.key = `beatlooproll_${size}_activate`;
            options.onShortPress = function() {
                if (!this.deck.beatloopSize) {
                    this.deck.beatloopSize = engine.getValue(this.group, "beatloop_size");
                }
                engine.setValue(this.group, this.inKey, true);
            };
            options.onShortRelease = function() {
                engine.setValue(this.group, this.inKey, false);
                if (this.deck.beatloopSize) {
                    engine.setValue(this.group, "beatloop_size", this.deck.beatloopSize);
                    this.deck.beatloopSize = undefined;
                }
            };
        }
        super(options);
        if (this.deck === undefined) {
            throw Error("BeatLoopRollButton must have a deck attached to it");
        }

        this.outConnect();
    }
    output(value) {
        if (this.key.startsWith("beatlooproll_")) {
            this.send(LedColors.white + (value ? this.brightnessOn : this.brightnessOff));
        } else {
            this.send(this.color);
        }
    }
}

/*
 * Represent a pad button that interact with a sampler (load, play/pause, cue, eject)
 */
class SamplerButton extends Button {
    constructor(options) {
        super(options);
        if (this.number === undefined || !Number.isInteger(this.number) || this.number < 1 || this.number > 64) {
            throw Error("SamplerButton must have a number property of an integer between 1 and 64");
        }
        this.group = `[Sampler${this.number}]`;
        this.outConnect();
    }
    onShortPress() {
        if (!this.shifted) {
            if (engine.getValue(this.group, "track_loaded") === 0) {
                engine.setValue(this.group, "LoadSelectedTrack", 1);
            } else {
                engine.setValue(this.group, "cue_gotoandplay", 1);
            }
        } else {
            if (engine.getValue(this.group, "play") === 1) {
                engine.setValue(this.group, "play", 0);
            } else {
                engine.setValue(this.group, "eject", 1);
            }
        }
    }
    onShortRelease() {
        if (this.shifted) {
            if (engine.getValue(this.group, "play") === 0) {
                engine.setValue(this.group, "eject", 0);
            }
        }
    }
    // This function is connected to multiple Controls, so don't use the value passed in as a parameter.
    output() {
        if (engine.getValue(this.group, "track_loaded")) {
            if (engine.getValue(this.group, "play")) {
                this.send(this.color + this.brightnessOn);
            } else {
                this.send(this.color + this.brightnessOff);
            }
        } else {
            this.send(0);
        }
    }
    outConnect() {
        if (undefined !== this.group) {
            const connection0 = engine.makeConnection(this.group, "play", this.output.bind(this));
            if (connection0) {
                this.outConnections[0] = connection0;
            } else {
                console.warn(`Unable to connect ${this.group}.play' to the controller output. The control appears to be unavailable.`);
            }
            const connection1 = engine.makeConnection(this.group, "track_loaded", this.output.bind(this));
            if (connection1) {
                this.outConnections[1] = connection1;
            } else {
                console.warn(`Unable to connect ${this.group}.track_loaded' to the controller output. The control appears to be unavailable.`);
            }
        }
    }
}

/*
 * Represent a pad button that interact with a intro/extra special markers (set, activate, clear)
 */
class IntroOutroButton extends PushButton {
    constructor(options) {
        super(options);
        if (this.cueBaseName === undefined || typeof this.cueBaseName !== "string") {
            throw Error("must specify cueBaseName as intro_start, intro_end, outro_start, or outro_end");
        }
        this.outKey = `${this.cueBaseName}_enabled`;
        this.outConnect();
    }
    unshift() {
        this.inKey = `${this.cueBaseName}_activate`;
    }
    shift() {
        this.inKey = `${this.cueBaseName}_clear`;
    }
    output(value) {
        if (value) {
            this.send(this.color + this.brightnessOn);
        } else {
            this.send(0);
        }
    }
}

class Pot extends Component {
    constructor(options) {
        super(options);
        this.hardwarePosition = null;
        this.shiftedHardwarePosition = null;

        if (this.input === undefined) {
            this.input = this.defaultInput;
        }
    }
    setGroupKey(group, key) {
        this.inKey = key;
        if (key === this.outKey && group === this.group) {
            return;
        }
        this.outDisconnect();
        this.group = group;
        this.outKey = key;
        this.outConnect();
    }
    defaultInput(value) {
        const receivingFirstValue = this.hardwarePosition === null;
        this.hardwarePosition = (value / this.max);
        engine.setParameter(this.group, this.inKey, this.hardwarePosition);
        if (receivingFirstValue) {
            engine.softTakeover(this.group, this.inKey, true);
        }
    }
    outDisconnect() {
        if (this.hardwarePosition !== null) {
            engine.softTakeover(this.group, this.inKey, true);
        }
        engine.softTakeoverIgnoreNextValue(this.group, this.inKey);
        super.outDisconnect();
    }
}

class Mixer extends ComponentContainer {
    constructor(inReports, outReports) {
        super();

        this.outReport = outReports[128];

        this.mixerColumnDeck1 = new S4Mk3MixerColumn(1, inReports, outReports[128],
            {
                saveGain: {inByte: 11, inBit: 0, outByte: 80},
                effectUnit1Assign: {inByte: 2, inBit: 3, outByte: 78},
                effectUnit2Assign: {inByte: 2, inBit: 4, outByte: 79},
                gain: {inByte: 16},
                eqHigh: {inByte: 44},
                eqMid: {inByte: 46},
                eqLow: {inByte: 48},
                quickEffectKnob: {inByte: 64},
                quickEffectButton: {},
                volume: {inByte: 2},
                pfl: {inByte: 7, inBit: 3, outByte: 77},
                crossfaderSwitch: {inByte: 17, inBit: 4},
            }
        );
        this.mixerColumnDeck2 = new S4Mk3MixerColumn(2, inReports, outReports[128],
            {
                saveGain: {inByte: 11, inBit: 1, outByte: 84},
                effectUnit1Assign: {inByte: 2, inBit: 5, outByte: 82},
                effectUnit2Assign: {inByte: 2, inBit: 6, outByte: 83},
                gain: {inByte: 18},
                eqHigh: {inByte: 50},
                eqMid: {inByte: 52},
                eqLow: {inByte: 54},
                quickEffectKnob: {inByte: 66},
                volume: {inByte: 4},
                pfl: {inByte: 7, inBit: 6, outByte: 81},
                crossfaderSwitch: {inByte: 17, inBit: 2},
            }
        );
        this.mixerColumnDeck3 = new S4Mk3MixerColumn(3, inReports, outReports[128],
            {
                saveGain: {inByte: 2, inBit: 1, outByte: 88},
                effectUnit1Assign: {inByte: 2, inBit: 0, outByte: 86},
                effectUnit2Assign: {inByte: 2, inBit: 2, outByte: 87},
                gain: {inByte: 14},
                eqHigh: {inByte: 38},
                eqMid: {inByte: 40},
                eqLow: {inByte: 42},
                quickEffectKnob: {inByte: 62},
                volume: {inByte: 6},
                pfl: {inByte: 7, inBit: 2, outByte: 85},
                crossfaderSwitch: {inByte: 17, inBit: 6},
            }
        );
        this.mixerColumnDeck4 = new S4Mk3MixerColumn(4, inReports, outReports[128],
            {
                saveGain: {inByte: 11, inBit: 2, outByte: 92},
                effectUnit1Assign: {inByte: 2, inBit: 7, outByte: 90},
                effectUnit2Assign: {inByte: 11, inBit: 7, outByte: 91},
                gain: {inByte: 20},
                eqHigh: {inByte: 56},
                eqMid: {inByte: 58},
                eqLow: {inByte: 60},
                quickEffectKnob: {inByte: 68},
                volume: {inByte: 8},
                pfl: {inByte: 7, inBit: 7, outByte: 89},
                crossfaderSwitch: {inByte: 17, inBit: 0},
            }
        );

        this.firstPressedFxSelector = null;
        this.secondPressedFxSelector = null;
        this.comboSelected = false;

        // FIXME: hardcoded
        const fxSelectsInputs = [
            {inByte: 8, inBit: 5},
            {inByte: 8, inBit: 1},
            {inByte: 8, inBit: 6},
            {inByte: 8, inBit: 0},
            {inByte: 8, inBit: 7},
        ];
        this.fxSelects = [];
        // FX SELECT buttons: Filter, 1, 2, 3, 4
        for (const i of [0, 1, 2, 3, 4]) {
            this.fxSelects[i] = new FXSelect(
                Object.assign(fxSelectsInputs[i], {
                    number: i + 1,
                    mixer: this,
                })
            );
        }

        const quickEffectInputs = [
            {inByte: 7, inBit: 0, outByte: 46},
            {inByte: 7, inBit: 5, outByte: 47},
            {inByte: 7, inBit: 1, outByte: 48},
            {inByte: 7, inBit: 4, outByte: 49},
        ];
        this.quickEffectButtons = [];
        // FX SELECT buttons: 1, 2, 3, 4
        for (const i of [0, 1, 2, 3]) {
            this.quickEffectButtons[i] = new QuickEffectButton(
                Object.assign(quickEffectInputs[i], {
                    number: i + 1,
                    mixer: this,
                })
            );
        }
        this.resetFxSelectorColors();

        this.quantizeButton = new Button({
            input: function(pressed) {
                if (pressed) {
                    this.globalQuantizeOn = !this.globalQuantizeOn;
                    for (let deckIdx = 1; deckIdx <= 4; deckIdx++) {
                        engine.setValue(`[Channel${deckIdx}]`, "quantize", this.globalQuantizeOn);
                    }
                    this.send(this.globalQuantizeOn ? 127 : 0);
                }
            },
            globalQuantizeOn: false,
            inByte: 11,
            inBit: 6,
            outByte: 93,
        });

        this.crossfader = new Pot({
            group: "[Master]",
            inKey: "crossfader",
            inByte: 0,
            inReport: inReports[HIDInputPotsReportID],
        });
        this.crossfaderCurveSwitch = new Component({
            inByte: 18,
            inBit: 0,
            inBitLength: 2,
            input: function(value) {
                switch (value) {
                case 0x00:  // Picnic Bench / Fast Cut
                    engine.setValue("[Mixer Profile]", "xFaderMode", 0);
                    engine.setValue("[Mixer Profile]", "xFaderCurve", 7.0);
                    break;
                case 0x01:  // Constant Power
                    engine.setValue("[Mixer Profile]", "xFaderMode", 1);
                    engine.setValue("[Mixer Profile]", "xFaderCurve", 0.6);
                    // Constant power requires to set an appropriate calibration value
                    // in order to get a smooth curve.
                    // This is the output of EngineXfader::getPowerCalibration() for
                    // the "xFaderCurve" 0.6 (pow(0.5, 1.0 / 0.6))
                    engine.setValue("[Mixer Profile]", "xFaderCalibration", 0.31498);
                    break;
                case 0x02: // Additive
                    engine.setValue("[Mixer Profile]", "xFaderMode", 0);
                    engine.setValue("[Mixer Profile]", "xFaderCurve", 0.9);
                }
            },
        });

        if (SoftwareMixerMain) {
            this.master = new Pot({
                group: "[Master]",
                inKey: "gain",
                inByte: 22,
                bitLength: 12,
                inReport: inReports[HIDInputPotsReportID]
            });
        }
        if (SoftwareMixerBooth) {
            this.booth = new Pot({
                group: "[Master]",
                inKey: "booth_gain",
                inByte: 24,
                bitLength: 12,
                inReport: inReports[HIDInputPotsReportID]
            });
        }
        if (SoftwareMixerHeadphone) {
            this.cue = new Pot({
                group: "[Master]",
                inKey: "headMix",
                inByte: 28,
                bitLength: 12,
                inReport: inReports[HIDInputPotsReportID]
            });

            this.pflGain = new Pot({
                group: "[Master]",
                inKey: "headGain",
                inByte: 26,
                bitLength: 12,
                inReport: inReports[HIDInputPotsReportID]
            });
        }

        for (const component of this) {
            if (component.inReport === undefined) {
                component.inReport = inReports[HIDInputButtonsReportID];
            }
            component.outReport = this.outReport;
            component.inConnect();
            component.outConnect();
            component.outTrigger();
        }

        let lightQuantizeButton = true;
        for (let deckIdx = 1; deckIdx <= 4; deckIdx++) {
            if (!engine.getValue(`[Channel${deckIdx}]`, "quantize")) {
                lightQuantizeButton = false;
            }
        }
        this.quantizeButton.send(lightQuantizeButton ? 127 : 0);
        this.quantizeButton.globalQuantizeOn = lightQuantizeButton;
    }

    calculatePresetNumber() {
        if (this.firstPressedFxSelector === this.secondPressedFxSelector || this.secondPressedFxSelector === null) {
            return this.firstPressedFxSelector;
        }
        let presetNumber = 5 + (4 * (this.firstPressedFxSelector - 1)) + this.secondPressedFxSelector;
        if (this.secondPressedFxSelector > this.firstPressedFxSelector) {
            presetNumber--;
        }
        return presetNumber;
    }

    resetFxSelectorColors() {
        for (const selector of [1, 2, 3, 4, 5]) {
            this.outReport.data[49 + selector] = QuickEffectPresetColors[selector - 1] + Button.prototype.brightnessOn;
        }
        this.outReport.send();
    }
}

class FXSelect extends Button {
    constructor(options) {
        super(options);

        if (this.mixer === undefined) {
            throw Error("The mixer must be specified");
        }
    }

    onShortPress() {
        if (this.mixer.firstPressedFxSelector === null) {
            this.mixer.firstPressedFxSelector = this.number;
            for (const selector of [1, 2, 3, 4, 5]) {
                if (selector !== this.number) {
                    let presetNumber = 5 + (4 * (this.mixer.firstPressedFxSelector - 1)) + selector;
                    if (selector > this.number) {
                        presetNumber--;
                    }
                    this.outReport.data[49 + selector] = QuickEffectPresetColors[presetNumber - 1] + this.brightnessOn;
                }
            }
            this.outReport.send();
        } else {
            this.mixer.secondPressedFxSelector = this.number;
        }

    }

    onShortRelease() {
        // After a second selector was released, avoid loading a different preset when
        // releasing the first pressed selector.
        if (this.mixer.comboSelected && this.number === this.mixer.firstPressedFxSelector) {
            this.mixer.comboSelected = false;
            this.mixer.firstPressedFxSelector = null;
            this.mixer.secondPressedFxSelector = null;
            this.mixer.resetFxSelectorColors();
            return;
        }
        // If mixer.firstPressedFxSelector === null, it was reset by the input handler for
        // a QuickEffect enable button to load the preset for only one deck.
        if (this.mixer.firstPressedFxSelector !== null) {
            for (const deck of [1, 2, 3, 4]) {
                const presetNumber = this.mixer.calculatePresetNumber();
                engine.setValue(`[QuickEffectRack1_[Channel${deck}]]`, "loaded_chain_preset", presetNumber);
            }
        }
        if (this.mixer.firstPressedFxSelector === this.number) {
            this.mixer.firstPressedFxSelector = null;
            this.mixer.resetFxSelectorColors();
        }
        if (this.mixer.secondPressedFxSelector !== null) {
            this.mixer.comboSelected = true;
        }
        this.mixer.secondPressedFxSelector = null;
    }

}


class QuickEffectButton extends Button {
    constructor(options) {
        super(options);
        if (this.mixer === undefined) {
            throw Error("The mixer must be specified");
        }
        if (this.number === undefined || !Number.isInteger(this.number) || this.number < 1) {
            throw Error("number attribute must be an integer >= 1");
        }
        this.group = `[QuickEffectRack1_[Channel${this.number}]]`;
        this.outConnect();
    }
    onShortPress() {
        if (this.mixer.firstPressedFxSelector === null) {
            script.toggleControl(this.group, "enabled");
        } else {
            const presetNumber = this.mixer.calculatePresetNumber();
            this.color = QuickEffectPresetColors[presetNumber - 1];
            engine.setValue(this.group, "loaded_chain_preset", presetNumber);
            this.mixer.firstPressedFxSelector = null;
            this.mixer.secondPressedFxSelector = null;
            this.mixer.resetFxSelectorColors();
        }
    }
    onLongRelease() {
        if (this.mixer.firstPressedFxSelector === null) {
            script.toggleControl(this.group, "enabled");
        }
    }
    output(enabled) {
        if (enabled) {
            this.send(this.color + this.brightnessOn);
        } else {
            // It is easy to mistake the dim state for the bright state, so turn
            // the LED fully off.
            this.send(this.color + this.brightnessOff);
        }
    }
    presetLoaded(presetNumber) {
        this.color = QuickEffectPresetColors[presetNumber - 1];
        this.outConnections[1].trigger();
    }
    outConnect() {
        if (this.group !== undefined) {
            const connection0 = engine.makeConnection(this.group, "loaded_chain_preset", this.presetLoaded.bind(this));
            if (connection0) {
                this.outConnections[0] = connection0;
            } else {
                console.warn(`Unable to connect ${this.group}.loaded_chain_preset' to the controller output. The control appears to be unavailable.`);
            }
            const connection1 = engine.makeConnection(this.group, "enabled", this.output.bind(this));
            if (connection1) {
                this.outConnections[1] = connection1;
            } else {
                console.warn(`Unable to connect ${this.group}.enabled' to the controller output. The control appears to be unavailable.`);
            }
        }
    }
}

/*
 * Kontrol S4 Mk3 hardware-specific constants
 */

Pot.prototype.max = 2 ** 12 - 1;
Pot.prototype.inBit = 0;
Pot.prototype.inBitLength = 16;

Encoder.prototype.inBitLength = 4;

// valid range 0 - 3, but 3 makes some colors appear whitish
Button.prototype.brightnessOff = 0;
Button.prototype.brightnessOn = 2;
Button.prototype.uncoloredOutput = function(value) {
    if (this.indicatorTimer !== 0) {
        return;
    }
    const color = (value > 0) ? (this.color || LedColors.white) + this.brightnessOn : LedColors.off;
    this.send(color);
};
Button.prototype.colorMap = new ColorMapper({
    0xCC0000: LedColors.red,
    0xCC5E00: LedColors.carrot,
    0xCC7800: LedColors.orange,
    0xCC9200: LedColors.honey,

    0xCCCC00: LedColors.yellow,
    0x81CC00: LedColors.lime,
    0x00CC00: LedColors.green,
    0x00CC49: LedColors.aqua,

    0x00CCCC: LedColors.celeste,
    0x0091CC: LedColors.sky,
    0x0000CC: LedColors.blue,
    0xCC00CC: LedColors.purple,

    0xCC0091: LedColors.fuscia,
    0xCC0079: LedColors.magenta,
    0xCC477E: LedColors.azalea,
    0xCC4761: LedColors.salmon,

    0xCCCCCC: LedColors.white,
});

const wheelLEDmodes = {
    off: 0,
    dimFlash: 1,
    spot: 2,
    ringFlash: 3,
    dimSpot: 4,
    individuallyAddressable: 5, // set byte 4 to 0 and set byes 8 - 40 to color values
};

// The mode available, which the wheel can be used for.
const wheelModes = {
    jog: 0,
    vinyl: 1,
    motor: 2,
    loopIn: 3,
    loopOut: 4,
};

const moveModes = {
    beat: 0,
    bpm: 1,
    grid: 2,
    keyboard: 3,
};

// tracks state across input reports
let wheelTimer = null;
// This is a global variable so the S4Mk3Deck Components have access
// to it and it is guaranteed to be calculated before processing
// input for the Components.
let wheelTimerDelta = 0;

/*
 * Kontrol S4 Mk3 hardware specific mapping logic
 */

class S4Mk3EffectUnit extends ComponentContainer {
    constructor(unitNumber, inReports, outReport, io) {
        super();
        this.group = `[EffectRack1_EffectUnit${unitNumber}]`;
        this.unitNumber = unitNumber;
        this.focusedEffect = null;

        this.mixKnob = new Pot({
            inKey: "mix",
            group: this.group,
            inReport: inReports[HIDInputPotsReportID],
            inByte: io.mixKnob.inByte,
        });

        this.mainButton = new PowerWindowButton({
            unit: this,
            inReport: inReports[HIDInputButtonsReportID],
            inByte: io.mainButton.inByte,
            inBit: io.mainButton.inBit,
            outByte: io.mainButton.outByte,
            outReport: outReport,
            shift: function() {
                this.group = this.unit.group;
                this.outKey = "group_[Master]_enable";
                this.outConnect();
                this.outTrigger();
            },
            unshift: function() {
                this.outDisconnect();
                this.outKey = undefined;
                this.group = undefined;
                this.output(false);
            },
            input: function(pressed) {
                if (!this.shifted) {
                    for (const index of [0, 1, 2]) {
                        const effectGroup = `[EffectRack1_EffectUnit${unitNumber}_Effect${index + 1}]`;
                        engine.setValue(effectGroup, "enabled", pressed);
                    }
                    this.output(pressed);
                } else if (pressed) {
                    if (this.unit.focusedEffect !== null) {
                        this.unit.setFocusedEffect(null);
                    } else {
                        script.toggleControl(this.unit.group, "group_[Master]_enable");
                        this.shift();
                    }
                }
            }
        });

        this.knobs = [];
        this.buttons = [];
        for (const index of [0, 1, 2]) {
            const effectGroup = `[EffectRack1_EffectUnit${unitNumber}_Effect${index + 1}]`;
            this.knobs[index] = new Pot({
                inKey: "meta",
                group: effectGroup,
                inReport: inReports[HIDInputPotsReportID],
                inByte: io.knobs[index].inByte,
            });
            this.buttons[index] = new Button({
                unit: this,
                key: "enabled",
                group: effectGroup,
                inReport: inReports[HIDInputButtonsReportID],
                inByte: io.buttons[index].inByte,
                inBit: io.buttons[index].inBit,
                outByte: io.buttons[index].outByte,
                outReport: outReport,
                onShortPress: function() {
                    if (!this.shifted || this.unit.focusedEffect !== null) {
                        script.toggleControl(this.group, this.inKey);
                    }
                },
                onLongPress: function() {
                    if (this.shifted) {
                        this.unit.setFocusedEffect(index);
                    }
                },
                onShortRelease: function() {
                   const wheelLEDmodes = {
    off: 0,
    dimFlash: 1,
    spot: 2,
    ringFlash: 3,
    dimSpot: 4,
    individuallyAddressable: 5, // set byte 4 to 0 and set byes 8 - 40 to color values
};

// The mode available, which the wheel can be used for.
const wheelModes = {
    jog: 0,
    vinyl: 1,
    motor: 2,
    loopIn: 3,
    loopOut: 4,
};

const moveModes = {
    beat: 0,
    bpm: 1,
    grid: 2,
    keyboard: 3,
};

// tracks state across input reports
let wheelTimer = null;
// This is a global variable so the S4Mk3Deck Components have access
// to it and it is guaranteed to be calculated before processing
// input for the Components.
let wheelTimerDelta = 0; if (this.shifted && this.unit.focusedEffect === null) {
                        script.triggerControl(this.group, "next_effect");
                    }
                },
                onLongRelease: function() {
                    if (!this.shifted) {
                        script.toggleControl(this.group, this.inKey);
                    }
                }
            });
        }

        for (const component of this) {
            component.inConnect();
            component.outConnect();
            component.outTrigger();
        }
    }
    indicatorLoop() {
        this.focusedEffectIndicator = !this.focusedEffectIndicator;
        this.mainButton.output(true);
    }
    setFocusedEffect(effectIdx) {
        this.mainButton.indicator(effectIdx !== null);
        this.focusedEffect = effectIdx;
        engine.setValue(this.group, "show_parameters", this.focusedEffect !== null);


        const effectGroup = `[EffectRack1_EffectUnit${this.unitNumber}_Effect${this.focusedEffect + 1}]`;
        for (const index of [0, 1, 2]) {
            const unfocusGroup = `[EffectRack1_EffectUnit${this.unitNumber}_Effect${index + 1}]`;
            this.buttons[index].outDisconnect();
            this.buttons[index].group = this.focusedEffect === null ? unfocusGroup : effectGroup;
            this.buttons[index].inKey = this.focusedEffect === null ? "enabled" : "button_parameter" + (index + 1);
            this.buttons[index].shift = this.focusedEffect === null ? undefined : function() {
                this.setGroup(unfocusGroup);
                this.setKey("enabled");
            };
            this.buttons[index].unshift = this.focusedEffect === null ? undefined : function() {
                this.setGroup(effectGroup);
                this.setKey("button_parameter" + (index + 1));
            };
            this.buttons[index].outKey = this.buttons[index].inKey;
            this.knobs[index].group = this.buttons[index].group;
            this.knobs[index].inKey = this.focusedEffect === null ? "meta" : "parameter" + (index + 1);
            this.knobs[index].shift = this.focusedEffect === null ? undefined : function() {
                this.setGroupKey(unfocusGroup, "meta");
            };
            this.knobs[index].unshift = this.focusedEffect === null ? undefined : function() {
                this.setGroupKey(effectGroup, "parameter" + (index + 1));
            };
            this.buttons[index].outConnect();
        }
    }
}

class S4Mk3Deck extends Deck {
    constructor(decks, colors, effectUnit, mixer, inReports, outReport, motorBuffMgr, io) {
        super(decks, colors);

        // buffer used for lowpassing the input velocity
        this.velFilter = new FilterBuffer(VelFilterTaps,VelFilterCoeffs);

        // state variable for whether the disc + slipmat are slipping
        // FIXME: nomenclature clash with the Mixxx "slip" mode (called "flux" in Traktor)
        // curiously, it is referred to as "censor" mode in part of the Mixxx documentation
        this.isSlipping = false;

        // manager for the motor output buffer on this deck
        this.motorBuffMgr = motorBuffMgr;

        this.playButton = new PlayButton({
            output: InactiveLightsAlwaysBacklit ? undefined : Button.prototype.uncoloredOutput
        });

        this.cueButton = new CueButton({
            deck: this
        });

        this.effectUnit = effectUnit;
        this.mixer = mixer;

        this.syncMasterButton = new Button({
            key: "sync_leader",
            defaultRange: 0.08,
            shift: UseKeylockOnMaster ? function() {
                this.setKey("keylock");
            } : undefined,
            unshift: UseKeylockOnMaster ? function() {
                this.setKey("sync_leader");
            } : undefined,
            onShortRelease: function() {
                script.toggleControl(this.group, this.inKey);
            },
            onLongPress: function() {
                const currentRange = engine.getValue(this.group, "rateRange");
                if (currentRange < 1.0) {
                    engine.setValue(this.group, "rateRange", 1.0);
                    this.indicator(true);
                } else {
                    engine.setValue(this.group, "rateRange", this.defaultRange);
                    this.indicator(false);
                }
            },
        });
        this.syncButton = new Button({
            key: "sync_enabled",
            onLongPress: function() {
                if (this.shifted) {
                    engine.setValue(this.group, "sync_key", true);
                    engine.setValue(this.group, "sync_key", false);
                } else {
                    script.triggerControl(this.group, "beatsync_tempo");
                }
            },
            onShortRelease: function() {
                script.toggleControl(this.group, this.inKey);
                if (!this.shifted) {
                    engine.softTakeover(this.group, "rate", true);
                }
            },
            shift: !UseKeylockOnMaster ? function() {
                this.setKey("keylock");
            } : undefined,
            unshift: !UseKeylockOnMaster ? function() {
                this.setKey("sync_enabled");
            } : undefined,
        });

        this.tempoFader = new Pot({
            inKey: "rate",
        });

        this.tempoFaderLED = new Component({
            outKey: "rate",
            centered: false,
            toleranceWindow: 0.001,
            tempoFader: this.tempoFader,
            output: function(value) {
                if (this.tempoFader.hardwarePosition === null) {
                    return;
                }

                const parameterValue = engine.getParameter(this.group, this.outKey);
                const diffFromHardware = parameterValue - this.tempoFader.hardwarePosition;
                if (diffFromHardware > this.toleranceWindow) {
                    this.send(TempoFaderSoftTakeoverColorHigh + Button.prototype.brightnessOn);
                    return;
                } else if (diffFromHardware < (-1 * this.toleranceWindow)) {
                    this.send(TempoFaderSoftTakeoverColorLow + Button.prototype.brightnessOn);
                    return;
                }

                // const oldCentered = this.centered;
                if (Math.abs(parameterValue - 0.5) < this.toleranceWindow) {
                    this.send(this.color + Button.prototype.brightnessOn);
                    // round to precisely 0
                    engine.setValue(this.group, "rate", 0);
                } else {
                    this.send(0);
                }
            }
        });

        this.reverseButton = new Button({
            key: "reverseroll",
            deck: this,
            previousWheelMode: null,
            loopModeConnection: null,
            unshift: function() {
                this.setKey("reverseroll");

            },
            shift: function() {
                this.setKey("loop_enabled");
            },
            output: InactiveLightsAlwaysBacklit ? undefined : Button.prototype.uncoloredOutput,
            onShortRelease: function() {
                if (!this.shifted) {
                    engine.setValue(this.group, this.key, false);
                }
            },
            loopModeOff: function(skipRestore) {
                if (this.previousWheelMode !== null) {
                    this.indicator(false);
                    const wheelOutput = new Uint8Array(40).fill(0);
                    wheelOutput[0] = decks[0] - 1;
                    controller.sendOutputReport(50, wheelOutput.buffer, true);
                    if (!skipRestore) {
                        this.deck.wheelMode = this.previousWheelMode;
                    }
                    this.previousWheelMode = null;
                    if (this.loopModeConnection !== null) {
                        this.loopModeConnection.disconnect();
                        this.loopModeConnection = null;
                    }
                }
            },
            onLoopChange: function(loopEnabled) {
                if (loopEnabled) { return; }
                this.loopModeOff();
            },
            onShortPress: function() {
                this.indicator(false);
                if (this.shifted) {
                    const loopEnabled = engine.getValue(this.group, "loop_enabled");
                    // If there is currently no loop, we set the loop in of a new loop
                    if (!loopEnabled) {
                        engine.setValue(this.group, "loop_end_position", -1);
                        engine.setValue(this.group, "loop_in", true);
                        this.indicator(true);
                        // Else, we enter/exit the loop in wheel mode
                    } else if (this.previousWheelMode === null) {
                        this.deck.fluxButton.loopModeOff();
                        engine.setValue(this.group, "scratch2_enable", false);
                        this.previousWheelMode = this.deck.wheelMode;
                        this.deck.wheelMode = wheelModes.loopIn;

                        if (this.loopModeConnection === null) {
                            this.loopModeConnection = engine.makeConnection(this.group, this.outKey, this.onLoopChange.bind(this));
                        }

                        const wheelOutput = new Uint8Array(40).fill(0);
                        wheelOutput[0] = decks[0] - 1;
                        wheelOutput[1] = wheelLEDmodes.ringFlash;
                        wheelOutput[4] = this.color + Button.prototype.brightnessOn;

                        controller.sendOutputReport(50, wheelOutput.buffer, true);

                        this.indicator(true);
                    } else if (this.previousWheelMode !== null) {
                        this.loopModeOff();
                    }
                } else {
                    engine.setValue(this.group, this.key, true);
                }
            }
        });
        this.fluxButton = new Button({
            key: "slip_enabled",
            deck: this,
            previousWheelMode: null,
            loopModeConnection: null,
            unshift: function() {
                this.setKey("slip_enabled");

            },
            shift: function() {
                this.setKey("loop_enabled");
            },
            outConnect: function() {
                if (this.outKey !== undefined && this.group !== undefined) {
                    const connection = engine.makeConnection(this.group, this.outKey, this.output.bind(this));
                    if (connection) {
                        this.outConnections[0] = connection;
                    } else {
                        console.warn(`Unable to connect ${this.group}.${this.outKey}' to the controller output. The control appears to be unavailable.`);
                    }
                }
            },
            output: InactiveLightsAlwaysBacklit ? undefined : Button.prototype.uncoloredOutput,
            onShortRelease: function() {
                if (!this.shifted) {
                    engine.setValue(this.group, this.key, false);
                    engine.setValue(this.group, "scratch2_enable", false);
                }
            },
            loopModeOff: function(skipRestore) {
                if (this.previousWheelMode !== null) {
                    this.indicator(false);
                    const wheelOutput = new Uint8Array(40).fill(0);
                    wheelOutput[0] = decks[0] - 1;
                    // Different function definition from the other calls in this file:
                    controller.sendOutputReport(wheelOutput.buffer, null, 50, true);
                    if (!skipRestore) {
                        this.deck.wheelMode = this.previousWheelMode;
                    }
                    this.previousWheelMode = null;
                    if (this.loopModeConnection !== null) {
                        this.loopModeConnection.disconnect();
                        this.loopModeConnection = null;
                    }
                }
            },
            onLoopChange: function(loopEnabled) {
                if (loopEnabled) { return; }
                this.loopModeOff();
            },
            onShortPress: function() {
                this.indicator(false);
                if (this.shifted) {
                    const loopEnabled = engine.getValue(this.group, "loop_enabled");
                    // If there is currently no loop, we set the loop in of a new loop
                    if (!loopEnabled) {
                        engine.setValue(this.group, "loop_out", true);
                        this.deck.reverseButton.indicator(false);
                        // Else, we enter/exit the loop in wheel mode
                    } else if (this.previousWheelMode === null) {
                        this.deck.reverseButton.loopModeOff();
                        engine.setValue(this.group, "scratch2_enable", false);
                        this.previousWheelMode = this.deck.wheelMode;
                        this.deck.wheelMode = wheelModes.loopOut;
                        if (this.loopModeConnection === null) {
                            this.loopModeConnection = engine.makeConnection(this.group, this.outKey, this.onLoopChange.bind(this));
                        }

                        const wheelOutput = new Uint8Array(40).fill(0);
                        wheelOutput[0] = decks[0] - 1;
                        wheelOutput[1] = wheelLEDmodes.ringFlash;
                        wheelOutput[4] = this.color + Button.prototype.brightnessOn;

                        controller.sendOutputReport(50, wheelOutput.buffer, true);

                        this.indicator(true);
                    } else if (this.previousWheelMode !== null) {
                        this.loopModeOff();
                    }
                } else {
                    engine.setValue(this.group, this.key, true);
                }
            }
        });
        this.gridButton = new Button({
            key: GridButtonBlinkOverBeat ? "beat_active" : undefined,
            deck: this,
            previousMoveMode: null,
            unshift: !GridButtonBlinkOverBeat ? function() {
                this.output(false);
            } : undefined,
            onShortPress: function() {
                this.deck.libraryEncoder.gridButtonPressed = true;

                if (this.shift) {
                    engine.setValue(this.group, "bpm_tap", true);
                }
            },
            onLongPress: function() {
                this.deck.libraryEncoder.gridButtonPressed = true;
                this.previousMoveMode = this.deck.moveMode;

                if (this.shifted) {
                    this.deck.moveMode = moveModes.grid;
                } else {
                    this.deck.moveMode = moveModes.bpm;
                }

                this.indicator(true);
            },
            onLongRelease: function() {
                this.deck.libraryEncoder.gridButtonPressed = false;
                if (this.previousMoveMode !== null) {
                    this.deck.moveMode = this.previousMoveMode;
                    this.previousMoveMode = null;
                }
                this.indicator(false);
            },
            onShortRelease: function() {
                this.deck.libraryEncoder.gridButtonPressed = false;
                script.triggerControl(this.group, "beats_translate_curpos");

                if (this.shift) {
                    engine.setValue(this.group, "bpm_tap", false);
                }
            },
        });

        this.deckButtonLeft = new Button({
            deck: this,
            input: function(value) {
                if (value) {
                    this.deck.switchDeck(Deck.groupForNumber(decks[0]));
                    this.outReport.data[io.deckButtonOutputByteOffset] = colors[0] + this.brightnessOn;
                    // turn off the other deck selection button's LED
                    this.outReport.data[io.deckButtonOutputByteOffset + 1] = DeckSelectAlwaysBacklit ? colors[1] + this.brightnessOff : 0;
                    this.outReport.send();
                }
            },
        });
        this.deckButtonRight = new Button({
            deck: this,
            input: function(value) {
                if (value) {
                    this.deck.switchDeck(Deck.groupForNumber(decks[1]));
                    // turn off the other deck selection button's LED
                    this.outReport.data[io.deckButtonOutputByteOffset] = DeckSelectAlwaysBacklit ? colors[0] + this.brightnessOff : 0;
                    this.outReport.data[io.deckButtonOutputByteOffset + 1] = colors[1] + this.brightnessOn;
                    this.outReport.send();
                }
            },
        });

        // set deck selection button LEDs
        outReport.data[io.deckButtonOutputByteOffset] = colors[0] + Button.prototype.brightnessOn;
        outReport.data[io.deckButtonOutputByteOffset + 1] = DeckSelectAlwaysBacklit ? colors[1] + Button.prototype.brightnessOff : 0;
        outReport.send();

        this.shiftButton = new PushButton({
            deck: this,
            output: InactiveLightsAlwaysBacklit ? undefined : Button.prototype.uncoloredOutput,
            unshift: function() {
                this.output(false);
            },
            shift: function() {
                this.output(true);
            },
            input: function(pressed) {
                if (pressed) {
                    this.deck.shift();
                } else {
                    this.deck.unshift();
                }
            }
        });

        this.leftEncoder = new Encoder({
            deck: this,
            onChange: function(right) {

                switch (this.deck.moveMode) {
                case moveModes.grid:
                    script.triggerControl(this.group, right ? "beats_adjust_faster" : "beats_adjust_slower");
                    break;
                case moveModes.keyboard:
                    if (
                        this.deck.keyboard[0].offset === (right ? 16 : 0)
                    ) {
                        return;
                    }
                    this.deck.keyboardOffset += (right ? 1 : -1);
                    this.deck.keyboard.forEach(function(pad) {
                        pad.outTrigger();
                    });
                    break;
                case moveModes.bpm:
                    script.triggerControl(this.group, right ? "beats_translate_later" : "beats_translate_earlier");
                    break;
                default:
                    if (!this.shifted) {
                        if (!this.deck.leftEncoderPress.pressed) {
                            if (right) {
                                script.triggerControl(this.group, "beatjump_forward");
                            } else {
                                script.triggerControl(this.group, "beatjump_backward");
                            }
                        } else {
                            let beatjumpSize = engine.getValue(this.group, "beatjump_size");
                            if (right) {
                                beatjumpSize *= 2;
                            } else {
                                beatjumpSize /= 2;
                            }
                            engine.setValue(this.group, "beatjump_size", beatjumpSize);
                        }
                    } else {
                        if (right) {
                            script.triggerControl(this.group, "pitch_up_small");
                        } else {
                            script.triggerControl(this.group, "pitch_down_small");
                        }
                    }
                    break;
                }
            }
        });
        this.leftEncoderPress = new PushButton({
            input: function(pressed) {
                this.pressed = pressed;
                if (pressed) {
                    script.toggleControl(this.group, "pitch_adjust_set_default");
                }
            },
        });

        this.rightEncoder = new Encoder({
            deck: this,
            onChange: function(right) {
                if (this.deck.wheelMode === wheelModes.loopIn || this.deck.wheelMode === wheelModes.loopOut) {
                    const moveFactor = this.shifted ? LoopEncoderShiftMoveFactor : LoopEncoderMoveFactor;
                    const valueIn = engine.getValue(this.group, "loop_start_position") + (right ? moveFactor : -moveFactor);
                    const valueOut = engine.getValue(this.group, "loop_end_position") + (right ? moveFactor : -moveFactor);
                    engine.setValue(this.group, "loop_start_position", valueIn);
                    engine.setValue(this.group, "loop_end_position", valueOut);
                } else if (this.shifted) {
                    script.triggerControl(this.group, right ? "loop_move_1_forward" : "loop_move_1_backward");
                } else {
                    script.triggerControl(this.group, right ? "loop_double" : "loop_halve");
                }
            }
        });
        this.rightEncoderPress = new PushButton({
            input: function(pressed) {
                if (!pressed) {
                    return;
                }
                const loopEnabled = engine.getValue(this.group, "loop_enabled");
                if (!this.shifted) {
                    script.triggerControl(this.group, "beatloop_activate");
                } else {
                    script.triggerControl(this.group, "reloop_toggle");
                }
            },
        });

        this.libraryEncoder = new Encoder({
            libraryPlayButtonPressed: false,
            gridButtonPressed: false,
            starButtonPressed: false,
            libraryViewButtonPressed: false,
            libraryPlaylistButtonPressed: false,
            currentSortedColumnIdx: -1,
            onChange: function(right) {
                if (this.libraryViewButtonPressed) {
                    this.currentSortedColumnIdx = (LibrarySortableColumns.length + this.currentSortedColumnIdx + (right ? 1 : -1)) % LibrarySortableColumns.length;
                    engine.setValue("[Library]", "sort_column", LibrarySortableColumns[this.currentSortedColumnIdx]);
                } else if (this.starButtonPressed) {
                    if (this.shifted) {
                        // FIXME doesn't exist, feature request needed
                        script.triggerControl(this.group, right ? "track_color_prev" : "track_color_next");
                    } else {
                        script.triggerControl(this.group, right ? "stars_up" : "stars_down");
                    }
                } else if (this.gridButtonPressed) {
                    script.triggerControl(this.group, right ? "waveform_zoom_up" : "waveform_zoom_down");
                } else if (this.libraryPlayButtonPressed) {
                    script.triggerControl("[PreviewDeck1]", right ? "beatjump_16_forward" : "beatjump_16_backward");
                } else {
                    // FIXME there is a bug where this action has no effect when the Mixxx window has no focused. https://github.com/mixxxdj/mixxx/issues/11285
                    // As a workaround, we are using deprecated control, hoping the bug will be fixed before the controls get removed
                    const currentlyFocusWidget = engine.getValue("[Library]", "focused_widget");
                    if (currentlyFocusWidget === 0) {
                        if (this.shifted) {
                            script.triggerControl("[Playlist]", right ? "SelectNextPlaylist" : "SelectPrevPlaylist");
                        } else {
                            script.triggerControl("[Playlist]", right ? "SelectNextTrack" : "SelectPrevTrack");
                        }
                    } else {
                        engine.setValue("[Library]", "focused_widget", this.shifted ? 2 : 3);
                        engine.setValue("[Library]", "MoveVertical", right ? 1 : -1);
                    }
                }
            }
        });
        this.libraryEncoderPress = new Button({
            libraryViewButtonPressed: false,
            onShortPress: function() {
                if (this.libraryViewButtonPressed) {
                    script.toggleControl("[Library]", "sort_order");
                } else {
                    const currentlyFocusWidget = engine.getValue("[Library]", "focused_widget");
                    // 3 == Tracks table or root views of library features
                    if (this.shifted && currentlyFocusWidget === 0) {
                        script.triggerControl("[Playlist]", "ToggleSelectedSidebarItem");
                    } else if (currentlyFocusWidget === 3 || currentlyFocusWidget === 0) {
                        script.triggerControl(this.group, "LoadSelectedTrack");
                    } else {
                        script.triggerControl("[Library]", "GoToItem");
                    }
                }
            },
            // FIXME not supported, feature request
            // onLongPress: function(){
            //     script.triggerControl("[Library]", "search_related_track", engine.getValue("[Library]", "sort_column"));
            // }
        });
        this.libraryPlayButton = new PushButton({
            group: "[PreviewDeck1]",
            libraryEncoder: this.libraryEncoder,
            input: function(pressed) {
                if (pressed) {
                    script.triggerControl(this.group, "LoadSelectedTrackAndPlay");
                } else {
                    engine.setValue(this.group, "play", 0);
                    script.triggerControl(this.group, "eject");
                }
                this.libraryEncoder.libraryPlayButtonPressed = pressed;
            },
            outKey: "play",
        });
        this.libraryStarButton = new Button({
            group: "[Library]",
            libraryEncoder: this.libraryEncoder,
            onShortRelease: function() {
                script.triggerControl(this.group, this.shifted ? "track_color_prev" : "track_color_next");
            },
            onLongPress: function() {
                this.libraryEncoder.starButtonPressed = true;
            },
            onLongRelease: function() {
                this.libraryEncoder.starButtonPressed = false;
            },
        });
        // FIXME there is no feature about playlist at the moment, so we use this button to control the context menu, which has playlist control
        this.libraryPlaylistButton = new Button({
            group: "[Library]",
            libraryEncoder: this.libraryEncoder,
            outConnect: function() {
                const connection = engine.makeConnection(this.group, "focused_widget", (widget) => {
                    // 4 == Context menu
                    this.output(widget === 4);
                });
                // This is useful for case where effect would have been fully disabled in Mixxx. This appears to be the case during unit tests.
                if (connection) {
                    this.outConnections[0] = connection;
                } else {
                    console.warn(`Unable to connect ${this.group}.focused_widget' to the controller output. The control appears to be unavailable.`);
                }
            },
            onShortRelease: function() {
                const currentlyFocusWidget = engine.getValue("[Library]", "focused_widget");
                // 3 == Tracks table or root views of library features
                // 4 == Context menu
                if (currentlyFocusWidget !== 3 && currentlyFocusWidget !== 4) {
                    return;
                }
                script.toggleControl("[Library]", "show_track_menu");
                this.libraryEncoder.libraryPlayButtonPressed = false;

                if (currentlyFocusWidget === 4) {
                    engine.setValue("[Library]", "focused_widget", 3);
                }
            },
            onShortPress: function() {
                this.libraryEncoder.libraryPlayButtonPressed = true;
            },
            onLongRelease: function() {
                this.libraryEncoder.libraryPlayButtonPressed = false;
            },
            onLongPress: function() {
                engine.setValue("[Library]", "clear_search", 1);
            }
        });
        this.libraryViewButton = new Button({
            group: "[Skin]",
            key: "show_maximized_library",
            libraryEncoder: this.libraryEncoder,
            libraryEncoderPress: this.libraryEncoderPress,
            onShortRelease: function() {
                script.toggleControl(this.group, this.inKey, true);
            },
            onLongPress: function() {
                this.libraryEncoder.libraryViewButtonPressed = true;
                this.libraryEncoderPress.libraryViewButtonPressed = true;
            },
            onLongRelease: function() {
                this.libraryEncoder.libraryViewButtonPressed = false;
                this.libraryEncoderPress.libraryViewButtonPressed = false;
            }
        });

        this.keyboardPlayMode = null;
        this.keyboardOffset = 9;

        this.pads = Array(8).fill(new Component());
        const defaultPadLayer = [
            new IntroOutroButton({
                cueBaseName: "intro_start",
            }),
            new IntroOutroButton({
                cueBaseName: "intro_end",
            }),
            new IntroOutroButton({
                cueBaseName: "outro_start",
            }),
            new IntroOutroButton({
                cueBaseName: "outro_end",
            }),
            new HotcueButton({
                number: 1
            }),
            new HotcueButton({
                number: 2
            }),
            new HotcueButton({
                number: 3
            }),
            new HotcueButton({
                number: 4
            })
        ];
        const hotcuePage2 = Array(8).fill({});
        const hotcuePage3 = Array(8).fill({});
        const samplerOrBeatloopRollPage = Array(8).fill({});
        this.keyboard = Array(8).fill({});
        let i = 0;
        /* eslint no-unused-vars: "off" */
        for (const pad of hotcuePage2) {
            // start with hotcue 5; hotcues 1-4 are in defaultPadLayer
            hotcuePage2[i] = new HotcueButton({number: i + 1});
            hotcuePage3[i] = new HotcueButton({number: i + 13});
            if (UseBeatloopRollInsteadOfSampler) {
                samplerOrBeatloopRollPage[i] = new BeatLoopRollButton({
                    number: i,
                    deck: this,
                });

            } else {
                let samplerNumber = i + 1;
                if (samplerNumber > 4) {
                    samplerNumber += 4;
                }
                if (decks[0] > 1) {
                    samplerNumber += 4;
                }
                samplerOrBeatloopRollPage[i] = new SamplerButton({
                    number: samplerNumber,
                });
                if (SamplerCrossfaderAssign) {
                    engine.setValue(
                        `[Sampler${samplerNumber}]`,
                        "orientation",
                        (decks[0] === 1) ? 0 : 2
                    );
                }
            }
            this.keyboard[i] = new KeyboardButton({
                number: i + 1,
                deck: this,
            });
            i++;
        }

        const switchPadLayer = (deck, newLayer) => {
            let index = 0;
            for (let pad of deck.pads) {
                pad.outDisconnect();
                pad.inDisconnect();

                pad = newLayer[index];
                Object.assign(pad, io.pads[index]);
                if (!(pad instanceof HotcueButton)) {
                    pad.color = deck.color;
                }
                // don't change the group of SamplerButtons
                if (!(pad instanceof SamplerButton)) {
                    pad.group = deck.group;
                }
                if (pad.inReport === undefined) {
                    pad.inReport = inReports[HIDInputButtonsReportID];
                }
                pad.outReport = outReport;
                pad.inConnect();
                pad.outConnect();
                pad.outTrigger();
                deck.pads[index] = pad;
                index++;
            }
        };

        this.padLayers = {
            defaultLayer: 0,
            hotcuePage2: 1,
            hotcuePage3: 2,
            samplerPage: 3,
            keyboard: 5,
        };
        switch (DefaultPadLayout) {
        case DefaultPadLayoutHotcue:
            switchPadLayer(this, hotcuePage2);
            this.currentPadLayer = this.padLayers.hotcuePage2;
            break;
        case DefaultPadLayoutSamplerBeatloop:
            switchPadLayer(this, samplerOrBeatloopRollPage);
            this.currentPadLayer = this.padLayers.samplerPage;
            break;
        case DefaultPadLayoutKeyboard:
            switchPadLayer(this, this.keyboard);
            this.currentPadLayer = this.padLayers.keyboard;
            break;
        default:
            switchPadLayer(this, defaultPadLayer);
            this.currentPadLayer = this.padLayers.defaultLayer;
            break;
        }

        this.hotcuePadModeButton = new Button({
            deck: this,
            onShortPress: function() {
                if (!this.shifted) {
                    if (this.deck.currentPadLayer !== this.deck.padLayers.hotcuePage2) {
                        switchPadLayer(this.deck, hotcuePage2);
                        this.deck.currentPadLayer = this.deck.padLayers.hotcuePage2;
                    } else {
                        switchPadLayer(this.deck, defaultPadLayer);
                        this.deck.currentPadLayer = this.deck.padLayers.defaultLayer;
                    }
                    this.deck.lightPadMode();
                } else {
                    switchPadLayer(this.deck, hotcuePage3);
                    this.deck.currentPadLayer = this.deck.padLayers.hotcuePage3;
                    this.deck.lightPadMode();
                }

            },
            // hack to switch the LED color when changing decks
            outTrigger: function() {
                this.deck.lightPadMode();
            }
        });
        // The record button doesn't have a mapping by default, but you can add yours here
        // this.recordPadModeButton = new Button({
        //     ...
        // });
        this.samplesPadModeButton = new Button({
            deck: this,
            onShortPress: function() {
                if (this.deck.currentPadLayer !== this.deck.padLayers.samplerPage) {
                    switchPadLayer(this.deck, samplerOrBeatloopRollPage);
                    engine.setValue("[Samplers]", "show_samplers", true);
                    this.deck.currentPadLayer = this.deck.padLayers.samplerPage;
                } else {
                    switchPadLayer(this.deck, defaultPadLayer);
                    engine.setValue("[Samplers]", "show_samplers", false);
                    this.deck.currentPadLayer = this.deck.padLayers.defaultLayer;
                }
                this.deck.lightPadMode();
            },
        });
        // The mute button doesn't have a mapping by default, but you can add yours here
        // this.mutePadModeButton = new Button({
        //    ...
        // });

        this.stemsPadModeButton = new Button({
            deck: this,
            previousMoveMode: null,
            onLongPress: function() {
                if (this.deck.keyboardPlayMode !== null) {
                    this.deck.keyboardPlayMode = null;
                    this.deck.lightPadMode();
                }
            },
            onShortPress: function() {
                if (this.previousMoveMode === null) {
                    this.previousMoveMode = this.deck.moveMode;
                    this.deck.moveMode = moveModes.keyboard;
                }
            },
            onShortRelease: function() {
                if (this.previousMoveMode !== null && !this.deck.keyboardPlayMode) {
                    this.deck.moveMode = this.previousMoveMode;
                    this.previousMoveMode = null;
                }
                if (this.deck.currentPadLayer === this.deck.padLayers.keyboard) {
                    switchPadLayer(this.deck, defaultPadLayer);
                    this.deck.currentPadLayer = this.deck.padLayers.defaultLayer;
                } else if (this.deck.currentPadLayer !== this.deck.padLayers.keyboard) {
                    switchPadLayer(this.deck, this.deck.keyboard);
                    this.deck.currentPadLayer = this.deck.padLayers.keyboard;
                }
                this.deck.lightPadMode();
            },
            onLongRelease: function() {
                if (this.previousMoveMode !== null && !this.deck.keyboardPlayMode) {
                    this.deck.moveMode = this.previousMoveMode;
                    this.previousMoveMode = null;
                }
            },
            // hack to switch the LED color when changing decks
            outTrigger: function() {
                this.deck.lightPadMode();
            }
        });

        this.wheelMode = wheelModes.vinyl;
        this.turntableButton = UseMotors ? new Button({
            deck: this,
            input: function(press) {
                if (press) {
                    this.deck.reverseButton.loopModeOff(true);
                    this.deck.fluxButton.loopModeOff(true);
                    if (this.deck.wheelMode === wheelModes.motor) {
                        this.deck.wheelMode = wheelModes.vinyl;
                        engine.setValue(this.group, "scratch2_enable", false);
                    } else {
                        this.deck.wheelMode = wheelModes.motor;
                        engine.setValue(this.group, "scratch2_enable", false);
                        const group = this.group;
                    }
                    this.outTrigger();
                }
            },
            outTrigger: function() {
                const motorOn = this.deck.wheelMode === wheelModes.motor;
                this.send(this.color + (motorOn ? this.brightnessOn : this.brightnessOff));
                const vinylModeOn = this.deck.wheelMode === wheelModes.vinyl;
                this.deck.jogButton.send(this.color + (vinylModeOn ? this.brightnessOn : this.brightnessOff));
            },
        }) : undefined;
        this.jogButton = new Button({
            deck: this,
            input: function(press) {
                if (press) {
                    this.deck.reverseButton.loopModeOff(true);
                    this.deck.fluxButton.loopModeOff(true);
                    if (this.deck.wheelMode === wheelModes.vinyl) {
                        this.deck.wheelMode = wheelModes.jog;
                    } else {
                        this.deck.wheelMode = wheelModes.vinyl;
                    }
                    engine.setValue(this.group, "scratch2_enable", false);
                    this.outTrigger();
                }
            },
            outTrigger: function() {
                const vinylOn = this.deck.wheelMode === wheelModes.vinyl;
                this.send(this.color + (vinylOn ? this.brightnessOn : this.brightnessOff));
                if (this.deck.turntableButton) {
                    const motorOn = this.deck.wheelMode === wheelModes.motor;
                    this.deck.turntableButton.send(this.color + (motorOn ? this.brightnessOn : this.brightnessOff));
                }
            },
        });

        this.wheelTouch = new Button({
            touched: false,
            deck: this,
            input: function(touched) {
                this.touched = touched;
                if (this.deck.wheelMode === wheelModes.vinyl || this.deck.wheelMode === wheelModes.motor) {
                    if (touched) {
                        engine.setValue(this.group, "scratch2_enable", true);
                    } else {
                        this.stopScratchWhenOver();
                    }
                }
            },
            stopScratchWhenOver: function() {
                if (this.touched) {
                    return;
                }
                // FIXME: something wierd up in here
                if (engine.getValue(this.group, "play") &&
                    engine.getValue(this.group, "scratch2") < 1.5 * BaseRevolutionsPerSecond &&
                    engine.getValue(this.group, "scratch2") > 0) {
                    engine.setValue(this.group, "scratch2_enable", false);
                } else if (engine.getValue(this.group, "scratch2") === 0) {
                    engine.setValue(this.group, "scratch2_enable", false);
                } else {
                    engine.beginTimer(100, this.stopScratchWhenOver.bind(this), true);
                }
            }
        });

        // There are two position inputs reported in the USB data, with identical
        // resolution. The first one is a cleaned up (unwrapped and sometimes corrected)
        // version of the second one, which appears to be the raw sensor data and wraps
        // every 2880 ticks (one full rotation).
        // Therefore we only care about the first position input.
        this.wheelPosition = new Component({
            prevData: null,
            deck: this,
            velocity: 0,
            prevPitch: 0,
            vFilter: this.velFilter,
            input: function(inPosition, inTimestamp) {
                // The input to the wheel is pulled from the HID input report #3
                // value: 16-bit integer that indicates angular position. Every step
                //        represents 1/8th of a degree of rotation.
                // timestamp: 32-bit counter used by the hardware, so is "immune"
                //        from USB transport jitter
                
                // Since we're calculating velocity as difference in position,
                // we have to init the previous value to zero
                if (this.prevData === null) {
                    this.prevData = [inPosition, inTimestamp];
                    return; // velocity is implicitly zero here on the return
                }

                // After the 1st iteration, proceed as normal. Save the previous data
                let [prevPosition, prevTimestamp] = this.prevData;

                // Unwrap the previous counter value if the new one rolls over to zero
                if (inTimestamp < prevTimestamp) {
                    prevTimestamp -= wheelTimerMax;
                }
                
                // Unwrap over/under-runs in the position signal
                let diff = inPosition - prevPosition;
                if (diff > wheelPositionMax / 2) {
                    prevPosition += wheelPositionMax;
                } else if (diff < -wheelPositionMax / 2) {
                    prevPosition -= wheelPositionMax;
                }
                
                // Using the unwrapped position reference, calculate 1st derivative
                // (angular velocity)
                // first, calculate the velocity in ticks (1/8th degree) per second
                // in_position and prev_position are both in 1/8th degrees
                // in_timestamp and prev_timestamp are both in clock ticks (10ns per)
                // WHEEL_CLOCK_FREQ converts clock ticks to seconds
                // example: position difference of 3 and timestamp difference of 2ms = 200000ns
                //          results in 1.5e-05 or 0.000015
                //          multiply by 100MHz or 100000000 produces 1500 ticks per second
                const currentVelocityTicksPerSecond = wheelClockFreq * (inPosition - prevPosition)/(inTimestamp - prevTimestamp);
                
                // then, normalize it with reference to the target rotation speed of the platter
                // regardless of how the pitch is being adjusted. In other words, the "rate_ratio"
                // is not a consideration here because we are only concerned with how fast the
                // platter is spinning relative to the playback rate of 1.0
                //     Therefore we simply divide the ticks per second by the ticks per degree
                // which eliminates the units of measure and leaves us with a ratio.
                const currentVelocityNormalized = currentVelocityTicksPerSecond / baseEncoderTicksPerDegree;
                
                // Input filtering:
                // Using a weighted average convolution filter ie., an FIR filter,
                // applly a lowpass to the velocity which is otherwise quite noisy.
                this.vFilter.insert(currentVelocityNormalized); // push/pop to the circular buffer
                this.velocity = this.vFilter.runFilter(); // sum of products with filter coeffs

                // Overwrite the previous position/time data with the current values
                this.prevData = [inPosition, inTimestamp];

                // At this point, all of our velocity computation is complete.
                // Now, we decide what to do with this information.
                
                // If the velocity is zero
                // and we are neither scratching, jogging, nor motoring,
                // stop here. The velocity is irrelevant to the system state.
                if (this.velocity === 0 &&
                    engine.getValue(this.group, "scratch2") === 0 &&
                    engine.getValue(this.group, "jog") === 0 &&
                    this.deck.wheelMode !== wheelModes.motor) {
                    return;
                }

                // Otherwise, interpret/report the velocity differently
                // depending on the wheel mode
                switch (this.deck.wheelMode) {
                case wheelModes.motor:
                    // for motorized platters, we are "always scratching",
                    // so we simply send the normalized velocity up the line
                    // ***NOTE*** this may change as we zero in on how to 
                    // switch between scratch and regular modes for pitch stability
                    // during steady-state and nudging

                    // ***NEW*** quantizing the output playback (when not slipping)
                    if (this.deck.isSlipping == false){
                        // apply simple smoothing filter to the velocity input when the disc is not slipping/scratching
                        this.velocity = this.prev_pitch + (NONSLIP_PITCH_SMOOTHING*(this.velocity - this.prev_pitch));
                        this.prev_pitch = this.velocity;
                    }
                    engine.setValue(this.group, "scratch2", this.velocity); // why is this still here? I thought we weren't "always scratching" anymore
                    break;
                case wheelModes.loopIn:
                    {
                        const loopStartPosition = engine.getValue(this.group, "loop_start_position");
                        const loopEndPosition = engine.getValue(this.group, "loop_end_position");
                        const value = Math.min(loopStartPosition + (this.velocity * LoopWheelMoveFactor), loopEndPosition - LoopWheelMoveFactor);
                        engine.setValue(
                            this.group,
                            "loop_start_position",
                            value
                        );
                    }
                    break;
                case wheelModes.loopOut:
                    {
                        const loopEndPosition = engine.getValue(this.group, "loop_end_position");
                        const value = loopEndPosition + (this.velocity * LoopWheelMoveFactor);
                        engine.setValue(
                            this.group,
                            "loop_end_position",
                            value
                        );
                    }
                    break;
                case wheelModes.vinyl:
                    if (this.deck.wheelTouch.touched || engine.getValue(this.group, "scratch2") !== 0) {
                        engine.setValue(this.group, "scratch2", this.velocity);
                    } else {
                        engine.setValue(this.group, "jog", this.velocity);
                    }
                    break;
                default:
                    engine.setValue(this.group, "jog", this.velocity);
                }
            },
        });

        this.wheelLED = new Component({
            deck: this,
            lastPos: 0,
            lastMode: null,
            outConnect: function() {
                if (this.group !== undefined) {
                    const connection0 = engine.makeConnection(this.group, "playposition", (position) => this.output.bind(this)(position, true, true));
                    // This is useful for case where effect would have been fully disabled in Mixxx. This appears to be the case during unit tests.
                    if (connection0) {
                        this.outConnections[0] = connection0;
                    } else {
                        console.warn(`Unable to connect ${this.group}.playposition' to the controller output. The control appears to be unavailable.`);
                    }
                    const connection1 = engine.makeConnection(this.group, "play", (play) => this.output.bind(this)(engine.getValue(this.group, "playposition"), play, play || engine.getValue(this.group, "track_loaded")));
                    // This is useful for case where effect would have been fully disabled in Mixxx. This appears to be the case during unit tests.
                    if (connection1) {
                        this.outConnections[1] = connection1;
                    } else {
                        console.warn(`Unable to connect ${this.group}.play' to the controller output. The control appears to be unavailable.`);
                    }
                    const connection2 = engine.makeConnection(this.group, "track_loaded", (trackLoaded) => this.output.bind(this)(engine.getValue(this.group, "playposition"), !trackLoaded ? false : engine.getValue(this.group, "play"), trackLoaded));
                    // This is useful for case where effect would have been fully disabled in Mixxx. This appears to be the case during unit tests.
                    if (connection2) {
                        this.outConnections[2] = connection2;
                    } else {
                        console.warn(`Unable to connect ${this.group}.track_loaded' to the controller output. The control appears to be unavailable.`);
                    }
                }
            },
            output: function(fractionOfTrack, playstate, trackLoaded) {
                if (this.deck.wheelMode > wheelModes.motor) {
                    return;
                }
                // Emit cue haptic feedback if enabled

                // samplePos aka currentPos
                const samplePos = Math.round(fractionOfTrack * engine.getValue(this.group, "track_samples"));
                if (this.deck.wheelTouch.touched && CueHapticFeedback) {
                    const cuePos = engine.getValue(this.group, "cue_point");
                    // forward == clockwise rotation
                    const forward = this.lastPos <= samplePos;
                    let fired = false;
                    // Stage a motor instruction with forward direction and max wheel force
                    const motorDeckData = new Uint8Array([
                        1, 0x20, 1, MaxWheelForce & 0xff, MaxWheelForce >> 8,
                    ]);
                    // if the cue position is between the current and last position,
                    // then fire the haptic bump
                    // clockwise check: last < cue < current
                    if (forward && this.lastPos < cuePos && cuePos < samplePos) {
                        fired = true;
                    // counter-clockwise check: current < cue < last
                    } else if (!forward && cuePos < this.lastPos && samplePos <= cuePos) {
                        motorDeckData[1] = 0xe0; // set reverse direction
                        motorDeckData[2] = 0xfe; // set reverse direction (byte 2)
                        fired = true;
                    }
                    if (fired) {
                        const motorData = new Uint8Array([
                            1, 0x20, 1, 0, 0,
                            1, 0x20, 1, 0, 0,
                        ]);
                        // overwrite one or the other of the 5-byte motor instructions
                        // with 
                        if (this.deck === TraktorS4MK3.leftDeck) {
                            motorData.set(motorDeckData);
                        } else {
                            motorData.set(motorDeckData, 5);
                        }
                        controller.sendOutputReport(49, motorData.buffer, true);
                    }
                }
                this.lastPos = samplePos;

                const durationSeconds = engine.getValue(this.group, "duration");
                const positionSeconds = fractionOfTrack * durationSeconds;
                const revolutions = positionSeconds * BaseRevolutionsPerSecond;
                const fractionalRevolution = revolutions - Math.floor(revolutions);
                const LEDposition = fractionalRevolution * wheelAbsoluteMax;

                // send commands to the wheel ring LEDs
                const wheelOutput = new Uint8Array(40).fill(0);
                wheelOutput[0] = decks[0] - 1;
                wheelOutput[4] = this.color + Button.prototype.brightnessOn;

                if (!trackLoaded) {
                    wheelOutput[1] = wheelLEDmodes.off;
                } else if (playstate && fractionOfTrack < 1 && engine.getValue(this.group, "end_of_track") && WheelLedBlinkOnTrackEnd && !this.deck.wheelTouch.touched) {
                    wheelOutput[1] = wheelLEDmodes.ringFlash;
                } else {
                    wheelOutput[1] = wheelLEDmodes.spot;
                    wheelOutput[2] = LEDposition & 0xff;
                    wheelOutput[3] = LEDposition >> 8;
                    if (this.lastMode === wheelLEDmodes.ringFlash) {
                        wheelOutput[4] = Button.prototype.brightnessOff;
                        engine.beginTimer(200, () => this.output(fractionOfTrack, playstate, trackLoaded), true);
                    }
                }
                this.lastMode = wheelOutput[1];

                controller.sendOutputReport(50, wheelOutput.buffer, true);
            }
        });

        for (const property in this) {
            if (Object.prototype.hasOwnProperty.call(this, property)) {
                const component = this[property];
                if (component instanceof Component) {
                    Object.assign(component, io[property]);
                    if (component.inReport === undefined) {
                        component.inReport = inReports[HIDInputButtonsReportID];
                    }
                    component.outReport = outReport;
                    if (component.group === undefined) {
                        component.group = this.group;
                    }
                    if (component.color === undefined) {
                        component.color = this.color;
                    }
                    if (component instanceof Encoder) {
                        component.max = 2 ** component.inBitLength - 1;
                    }
                    component.inConnect();
                    component.outConnect();
                    component.outTrigger();
                    if (typeof this.unshift === "function" && this.unshift.length === 0) {
                        this.unshift();
                    }
                }
            }
        }
    }

    assignKeyboardPlayMode(group, action) {
        this.keyboardPlayMode = {
            group: group,
            action: action,
        };
        this.lightPadMode();
    }

    lightPadMode() {
        if (this.currentPadLayer === this.padLayers.hotcuePage2) {
            this.hotcuePadModeButton.send(this.hotcuePadModeButton.color + this.hotcuePadModeButton.brightnessOn);
        } else if (this.currentPadLayer === this.padLayers.hotcuePage3) {
            this.hotcuePadModeButton.send(LedColors.white + this.hotcuePadModeButton.brightnessOn);
        } else {
            this.hotcuePadModeButton.send(this.hotcuePadModeButton.color + this.hotcuePadModeButton.brightnessOff);
        }

        // unfortunately the other pad mode buttons only have one LED color
        // const recordPadModeLEDOn = this.currentPadLayer === this.padLayers.hotcuePage3;
        // this.recordPadModeButton.send(recordPadModeLEDOn ? 127 : 0);

        const samplesPadModeLEDOn = this.currentPadLayer === this.padLayers.samplerPage;
        this.samplesPadModeButton.send(samplesPadModeLEDOn ? 127 : 0);

        // this.mutePadModeButtonLEDOn = this.currentPadLayer === this.padLayers.samplerPage2;
        // const mutedModeButton.send(mutePadModeButtonLEDOn ? 127 : 0);
        if (this.keyboardPlayMode !== null) {
            this.stemsPadModeButton.send(LedColors.green + this.stemsPadModeButton.brightnessOn);
        } else {
            const keyboardPadModeLEDOn = this.currentPadLayer === this.padLayers.keyboard;
            this.stemsPadModeButton.send(this.stemsPadModeButton.color + (keyboardPadModeLEDOn ? this.stemsPadModeButton.brightnessOn : this.stemsPadModeButton.brightnessOff));
        }
    }
}

class S4Mk3MotorManager {
    constructor(deck) {
        this.deck = deck;
        // Set the Left/Right motor identifier
        // NOTE: CURRENTLY THIS ASSUMES THAT THE FIRST DECK ON THE LEFT IS 1
        //       AND THE FIRST DECK ON THE RIGHT IS 2
        if (this.deck.decks[0] == 1) {
            this.deckMotorID = MotorBuffIDLeft;
        }
        else if (this.deck.decks[0] == 2) {
            this.deckMotorID = MotorBuffIDRight;
        }
        else {
            this_is_an_error = true; // TODO: proper handling
        }
        this.motorBuffMgr = this.deck.motorBuffMgr;

        this.oldValue = [0, 0];
        this.currentMaxWheelForce = MaxWheelForce;
        this.prev_playbackError = 0; // JUNE 20 2025 --- RESUME HERE
        this.P_term = 0;
        this.I_accumulator = 0;
        this.D_term = 0;
        this.outputTorque_prev = 0;
        this.outputTracking_prev = 0; // new attempt at nudge tracking

        this.motortesting_onOff = false;
        this.motortesting_complete = false;
        this.motortesting_timer = Date.now();
        this.motortesting_next_interval = S4MK3MOTORTEST_UPTIME;
        this.motorTesting_currentLevel = S4MK3MOTORTEST_STARTLVL;
        // this.isSlipping = false;
        this.nominal_rate_prenudge = 1.0;
        this.isUpToSpeed = false;
        this.isStopped = false;
    }
    tick() {
        let outputTorque = 0;
        let targetRate = 0;
        let playbackError = 0;
        let torqueDiff = 0;
        let trackingDiff = 0;
        let outputTracking = 0;
        let trackingError = 0;

        // TESTING
        // console.warn(engine.getValue(this.deck.group, "scratch2_enable"));

        //REMOVE: Can be optimized --- point to a single premade instance variable
        // const motorData = new Uint8Array([
        //     1, 0x20, 1, 0, 0,
        // ]);

        // Declaring local constant
        const maxVelocity = 10; //FIXME: hardcoded

        // currentSpeed is normalized to a reference value of baseRPS (100/3/60 for 33.3rpm)
        // Declaring local variables
        // let playbackRate = 0;
        // let current_velocity = 0;
        
        // let expectedSpeed = 0; //FIXME: nomenclature

        // normalize the velocity relative to the target sensor ticks per second
        // which for 33.3rpm will be 1600
        // ie., when this.deck.wheelPosition.velocity = 1600, normalizedVelocity = 1.0
        // const normalizedVelocity = this.deck.wheelPosition.velocity / baseEncoderTicksPerSecond;
        // const normalizedVelocity = this.deck.wheelPosition.velocity;
        const normalizedVelocity = this.deck.velFilter.getCurrentVel()

        // If this deck is currently playing, calculate motor output
        // Determine target (relative) angular velocity based on wheel mode
        if (this.deck.wheelMode === wheelModes.motor && engine.getValue(this.deck.group, "play")) {
            if (this.isStopped == true){
                this.isStopped = false;
                engine.setValue(this.deck.group, "scratch2_enable", false);
            }
            // Motor calibration testing for determining real output torque
            if (S4MK3MOTORTEST_ENABLE && this.motortesting_complete == false) {
                if (Date.now() - this.motortesting_timer > this.motortesting_next_interval) {    
                    console.warn(this.deckMotorID, "Motor test level: ",this.motorTesting_currentLevel);
                    this.motortesting_timer = Date.now();
                    // If the output was previously OFF, turn it on
                    if (this.motortesting_onOff == false) {
                        this.motortesting_onOff = true;
                        this.motortesting_next_interval = S4MK3MOTORTEST_UPTIME;
                    // If the output was previously ON, turn it off and increment
                    // for next time
                    } else {
                        this.motortesting_onOff = false;
                        this.motortesting_next_interval = S4MK3MOTORTEST_DOWNTIME;
                        if (this.motorTesting_currentLevel > S4MK3MOTORTEST_ENDLVL) {
                            this.motorTesting_currentLevel = 0;
                            this.motortesting_complete = true;
                        } else {
                            this.motorTesting_currentLevel += S4MK3MOTORTEST_STEPSIZE;
                        }
                    }
                }

                if (this.motortesting_onOff == true) {
                    outputTorque = this.motorTesting_currentLevel;
                } else {
                    outputTorque = 0;
                }
                // Write the calculated value to the motor output buffer
                this.motorBuffMgr.setMotorOutput(this.deckMotorID,outputTorque);
                return true;
            }

            // targetRate is 1.0 +/- pitch adjustment. So +8% pitch means targetRate == 1.08
            targetRate = engine.getValue(this.deck.group, "rate_ratio");
            
            // normalisationFactor doesn't appear to be in use
            // const normalisationFactor = 1/expectedSpeed/5;
            //???: what is going on in this velocity calculation?
            // it looks like it might be a smoothing filter of some sort
            
            // Original motor controller:
            // Apply a simple smoothing function to pull the current velocity
            // towards the expected velocity. Not clear if this filter is tuned to a specific
            // cutoff or lag.
            // FIXME: nomenclature.
            // velocity = targetRate + Math.pow(-5 * (targetRate / 1) * (currentSpeed - targetRate), 3);

            // First, determine the error between target vs measured
            playbackError = targetRate - normalizedVelocity;

            // close enough to perfect? disable scratch2 mode
            // if (Math.abs(playbackError) < PLAYBACK_QUANTIZE_ERROR_THRESH){
            //     engine.setValue(this.deck.group, "scratch2_enable", false);
            // }
            // else {
            //     engine.setValue(this.deck.group, "scratch2_enable", true);
            // }

            // If we are touching the disc AND the playbackError goes beyond
            // the slipping threshold, apply the slip force only
            if (this.deck.wheelTouch.touched && Math.abs(playbackError) > SlipmatErrorThresh){
                this.deck.isSlipping = true;
                engine.setValue(this.deck.group, "scratch2_enable", true);
            } else if (this.deck.wheelTouch.touched == false && this.deck.isSlipping && Math.abs(playbackError) < SlipmatErrorThresh) {
                this.deck.isSlipping = false;
                engine.setValue(this.deck.group, "scratch2_enable", false);
            } 
            // Experimental: if we are beyond a certain error threshold (slip for now),
            // suppress the error integrator---to help with gracefully restoring rotation
            // speed without overshoot when adjusting with the crown.
            else if (Math.abs(playbackError) > IntegratorSuppressionErrorThresh){
                // keep the accumulator suppressed so it doesn't go crazy
                this.I_accumulator = 0;
            }

            if (this.deck.isSlipping) {
                if (playbackError > 0) {
                    outputTorque = SlipFrictionForce;
                }
                else {
                    outputTorque = -SlipFrictionForce;
                }
            }
            // Otherwise, we aren't slipping. Apply new motor controller
            else {
                // New motor controller: a PI followed by a very simple smoothing filter
                this.P_term = playbackError * ProportionalGain;
                this.I_accumulator += playbackError * IntegrativeGain;
                this.D_term = (playbackError - this.prev_playbackError) * DerivativeGain;
                outputTorque = this.P_term + this.I_accumulator - this.D_term;
                // outputTracking = outputTorque;
                // outputTorque = playbackError * MOTORPID_PROPORTIONAL_GAIN;

                // Apply a smoothing filter
                torqueDiff = outputTorque - this.outputTorque_prev;
                // Another smoothing filter, only for pitch analysis
                trackingDiff = outputTorque - this.outputTracking_prev;

                outputTorque = this.outputTorque_prev + (torqueDiff * MotorOutSmoothingFactor);
                outputTracking = this.outputTracking_prev + (trackingDiff * MotorOutSmoothingFactor);
                // Compare the smoothed output to a target expected torque to determine effective output pitch in 
                // non-slip mode (scratch2 disabled):
                trackingError = (outputTorque - (TargetMotorOutput*engine.getValue(this.deck.group, "rate_ratio")))/TargetMotorOutput;
                // trackingError = Math.round(trackingError/PLAYBACK_QUANTIZE_ERROR_STEP);
                // trackingError = (trackingError * PLAYBACK_QUANTIZE_ERROR_STEP);
                if (this.isUpToSpeed == true){
                    engine.setValue(this.deck.group, "jog", -trackingError*TurnTableNudgeSensitivity);
                    // console.warn(outputTorque, outputTracking, trackingError);
                } else if (trackingError < 0) {
                    // if we've spun all the way up, only then act like it's jogging time.
                    this.isUpToSpeed = true;
                }
            }
            // New torque becomes old torque
            this.outputTorque_prev = outputTorque;
            // Apply additional forward force to compensate for friction --- NO LONGER NECESSARY
            // outputTorque += MOTOR_FRICTION_COMPENSATION;
        // If the deck isn't playing, but we're in motor mode, ensure that scratch mode is ON
        // and reset the "isUpToSpeed" flag
        } else if (this.deck.wheelMode === wheelModes.motor && engine.getValue(this.deck.group, "play") == false) {
            this.isUpToSpeed = false;
            this.isStopped = true;
            engine.setValue(this.deck.group, "scratch2_enable", true);
        // In any other wheel mode, the motor only provides resistance to scrubbing/scratching
        } else if (this.deck.wheelMode !== wheelModes.motor) {
            if (TightnessFactor > 0.5) {
                // Super loose
                const reduceFactor = (Math.min(0.5, TightnessFactor - 0.5) / 0.5) * 0.7;
                outputTorque = normalizedVelocity * reduceFactor;
            } else if (TightnessFactor < 0.5) {
                // Super tight
                const reduceFactor = (2 - Math.max(0, TightnessFactor) * 4);
                outputTorque = targetRate + Math.min(
                    maxVelocity,
                    Math.max(
                        -maxVelocity,
                        (targetRate - normalizedVelocity) * reduceFactor
                    )
                );
            }
        }

        // ==========================
        // SLIPMAT PHYSICAL MODELING:
        // ==========================
        // Calculate the outgoing motor torque using a simulated turntable/slipmat.
        //
        // -----------
        // Parameters:
        // -----------
        // Disc/platter radius (m) and mass (kg)
        //      --- for a 12" disc, 0.15m and 0.15--0.2kg
        // Slipmat coefficients of static and kinetic friction (no unit)
        //      --- not sure of exact coeff. for a given slipmat material, but I would choose
        //          one that can *just* handle the motor torque without slipping
        // Maximum motor torque (Nm)
        //      --- for a T1200 this is around 0.15Nm
        //
        // -------
        // SUMMARY
        // -------
        // There are two primary states of the simulation: STICK and SLIP.
        // - The disc and platter STICK together unless the performer touches the disc AND
        //       alters the angular velocity of the jogwheel enough for the disc to SLIP.
        // - The disc remains in the SLIP state until its angular velocity returns
        //       to within a margin of the target velocity, at which point it STICKs again.
        //
        // ------------
        // STICK state:
        // ------------
        // The vinyl disc, slipmat, and platter are stuck to each other and move together.
        // In this state, the jogwheel represents the entire assembly. The crown/edge of the
        // jogwheel represents the crown/edge of the platter. The performer can hinder or help
        // the rotation of the jogwheel, and the motor works to correct any deviation from
        // the target angular velocity.
        //     If the performer perturbs the rotation of the jogwheel while touching the disc
        // surface, the same behaviour applies UNLESS the perturbation is large enough to
        // defeat the force of friction that keeps everything stuck together. In other words,
        // the target and measured velocity are too far apart. At this point, the simulated
        // rotation of disc and platter decouple from each other and we enter the SLIP state.
        //
        // -----------
        // SLIP state:
        // -----------
        // The vinyl disc moves at a different rate than the motor, and only (kinetic) friction
        // works to bring them back in sync. In this state, **the jogwheel only represents the
        // rotation of the disc**. The platter is abstracted and we assume that it rotates at
        // the target velocity.
        //     The torque applied to the jogwheel motor represents the force of friction, which
        // pulls in the direction of the target angular velocity. This means that when the 
        // performer is scratching, the motor is sometimes helping, if the scratch is moving the
        // disc closer to synchrony with the platter.
        //     Anytime the disc and platter velocity are nearly matched, the simulated platter
        // regains its influence---in other words, within a certain margin the system returns
        // to the STICK state, even if only for a fraction of a second.
        //    Finally, whether by manually bringing the disc to the target velocity or allowing
        // the simulated friction force to do it, the system syncs up and re-enters the STICK
        // state. We don't care if the performer is still touching the disc, as long as they
        // aren't bringing it out of sync again.
        //
        // NOTE:
        // -----
        // Missing from this simulation is the modeling of the torque transfer *from* the
        // disc *to* the platter, as well as the mass/inertia of the platter.
        // For example: when a turntable motor is off, spinning the record will transfer some 
        // torque across the slipmat and the platter will start spinning as well. However,
        // adding the platter dynamics to the model will increase the computational complexity
        // for what amounts to an edge case at this point. For now, the platter is massless and
        // the motor is frictionless for the purposes of the slipmat physics.
        //    Of course, the jogwheel is *not* frictionless *nor* massless, but my hope is that
        // the control system will automatically account for this in practice as it will
        // naturally reach a steady-state in its attempts to regulate the jogwheel's angular
        // velocity. 
        
        // NOTE 2:
        // -------
        // After extensive study and simulation of USB traffic between the S4Mk3 and Traktor,
        // I have tuned the input filter, output PID controller and output smoothing filters
        // to closely match the behaviour of Traktor. The tuned parameters are not yet linked
        // to physical dynamics per se, as the first step is to recreate the commercial system
        // before breaking down the control parameters into physical constants.

        // Formatting and writing the output data:
        // NEW CODE:


        // OLD CODE:
        // Convert signed angular velocity to unsigned angular speed + direction code
        // if (velocity < 0) {
        //     motorData[1] = 0xe0; // negative/CCW direction code 1
        //     motorData[2] = 0xfe; // negative/CCW direction code 2
        //     velocity = -velocity; // invert the sign to make it positive
        // } else if (this.deck.wheelMode === wheelModes.motor && engine.getValue(this.deck.group, "play")) {
        //     velocity += this.zeroSpeedForce; // REMOVE: Don't think this condition is necessary for physical model
        // }

        // detecting if the user is stopping the disc from rotating
        // I really don't think this is necessary if we implement a physical model
        // to detect relative velocity of [virtual] platter rotation versus
        // [measured] disc rotation during the slip-state
        // REMOVE for physical model implementation
        // if (!this.isBlockedByUser() && velocity > MaxWheelForce) {
        //     this.userHold++;
        // } else if (velocity < MaxWheelForce / 2 && this.userHold > 0) {
        //     this.userHold--;
        // }

        // REMOVE for physical model implementation
        // if (this.isBlockedByUser()) {
        //     // if the user is blocking the disc rotation, maintain scratch mode
        //     engine.setValue(this.deck.group, "scratch2_enable", true);
        //     this.currentMaxWheelForce = this.zeroSpeedForce + parseInt(this.baseFactor * expectedSpeed);
        // } else if (expectedSpeed && this.userHold === 0 && !this.deck.wheelTouch.touched) {
        //     // if the user has let go, turn off scratch mode and drive the motor hard
        //     // to get back to the base rotation speed
        //     engine.setValue(this.deck.group, "scratch2_enable", false);
        //     this.currentMaxWheelForce = MaxWheelForce;
        // }

        // clip the output to the max output if overdriving
        // and/or convert velocity to integer for output to motors
        // Clip the outgoing wheel force to a set maximum, or truncate to int
        // velocity = Math.min(
        //     this.currentMaxWheelForce,
        //     Math.floor(velocity)
        // );

        // Write to the output buffer. Optimize: should be writing to an instance array
        // motorData[3] = velocity & 0xff;
        // motorData[4] = velocity >> 8;

        // Write the calculated value to the motor output buffer
        this.motorBuffMgr.setMotorOutput(this.deckMotorID,outputTorque);

        return true;
    }
    // isBlockedByUser() { // REMOVE with physical model implementation
    //     return this.userHold >= 10;
    // }
}

class S4Mk3MixerColumn extends ComponentContainer {
    constructor(idx, inReports, outReport, io) {
        super();

        this.idx = idx;
        this.group = `[Channel${idx}]`;

        this.gain = new Pot({
            inKey: "pregain",
        });
        this.eqHigh = new Pot({
            group: `[EqualizerRack1_${this.group}_Effect1]`,
            inKey: "parameter3",
        });
        this.eqMid = new Pot({
            group: `[EqualizerRack1_${this.group}_Effect1]`,
            inKey: "parameter2",
        });
        this.eqLow = new Pot({
            group: `[EqualizerRack1_${this.group}_Effect1]`,
            inKey: "parameter1",
        });
        this.quickEffectKnob = new Pot({
            group: `[QuickEffectRack1_${this.group}]`,
            inKey: "super1",
        });
        this.volume = new Pot({
            inKey: "volume",
            mixer: this,
            input: MixerControlsMixAuxOnShift ? function(value) {
                if (this.mixer.shifted && this.group !== `[Channel${idx}]`) { // FIXME only if group != [ChannelX]
                    const controlKey = (this.group === `[Microphone${idx}]` || this.group === "[Microphone]") ? "talkover" : "main_mix";
                    const isPlaying = engine.getValue(this.group, controlKey);
                    if ((value !== 0) !== isPlaying) {
                        engine.setValue(this.group, controlKey, value !== 0);
                    }
                }
                this.defaultInput(value);
            } : undefined
        });

        this.pfl = new ToggleButton({
            inKey: "pfl",
            outKey: "pfl",
        });

        this.effectUnit1Assign = new PowerWindowButton({
            group: "[EffectRack1_EffectUnit1]",
            key: `group_${this.group}_enable`,
        });

        this.effectUnit2Assign = new PowerWindowButton({
            group: "[EffectRack1_EffectUnit2]",
            key: `group_${this.group}_enable`,
        });

        // FIXME: Why is output not working for these?
        this.saveGain = new PushButton({
            key: "update_replaygain_from_pregain"
        });

        this.crossfaderSwitch = new Component({
            inBitLength: 2,
            input: function(value) {
                if (value === 0) {
                    engine.setValue(this.group, "orientation", 2);
                } else if (value === 1) {
                    engine.setValue(this.group, "orientation", 1);
                } else if (value === 2) {
                    engine.setValue(this.group, "orientation", 0);
                }
            },
        });

        for (const property in this) {
            if (Object.prototype.hasOwnProperty.call(this, property)) {
                const component = this[property];
                if (component instanceof Component) {
                    Object.assign(component, io[property]);
                    if (component instanceof Pot) {
                        component.inReport = inReports[HIDInputPotsReportID];
                    } else {
                        component.inReport = inReports[HIDInputButtonsReportID];
                    }
                    component.outReport = outReport;

                    if (component.group === undefined) {
                        component.group = this.group;
                    }

                    component.inConnect();
                    component.outConnect();
                    component.outTrigger();
                }
            }
        }

        if (MixerControlsMixAuxOnShift) {
            this.shift = function() {
                engine.setValue("[Microphone]", "show_microphone", true);
                this.updateGroup(true);
            };

            this.unshift = function() {
                engine.setValue("[Microphone]", "show_microphone", false);
                this.updateGroup(false);
            };
        }
    }

    updateGroup(shifted) {
        let alternativeInput = null;
        if (engine.getValue(`[Auxiliary${this.idx}]`, "input_configured")) {
            alternativeInput = `[Auxiliary${this.idx}]`;
        } else if (engine.getValue(this.idx !== 1 ? `[Microphone${this.idx}]` : "[Microphone]", "input_configured")) {
            alternativeInput = this.idx !== 1 ? `[Microphone${this.idx}]` : "[Microphone]";
        }

        if (!alternativeInput) {
            return;
        }
        this.group = shifted ? alternativeInput : `[Channel${this.idx}]`;
        for (const property of ["gain", "volume", "pfl", "crossfaderSwitch"]) {
            const component = this[property];
            if (component instanceof Component) {
                component.outDisconnect();
                component.inDisconnect();
                component.group = this.group;
                component.inConnect();
                component.outConnect();
                component.outTrigger();
            }
        }
        for (const property of ["effectUnit1Assign", "effectUnit2Assign"]) {
            const component = this[property];
            if (component instanceof Component) {
                component.outDisconnect();
                component.inDisconnect();
                component.inKey = `group_${this.group}_enable`;
                component.outKey = `group_${this.group}_enable`;
                component.inConnect();
                component.outConnect();
                component.outTrigger();
            }
        }
    }
}

class S4MK3 {
    constructor() {
        if (engine.getValue("[App]", "num_decks") < 4) {
            engine.setValue("[App]", "num_decks", 4);
        }
        if (engine.getValue("[App]", "num_samplers") < 16) {
            engine.setValue("[App]", "num_samplers", 16);
        }

        this.inReports = [];
        this.inReports[HIDInputButtonsReportID] = new HIDInputReport(HIDInputButtonsReportID);
        // The master volume, booth volume, headphone mix, and headphone volume knobs
        // control the controller's audio interface in hardware, so they are not mapped.
        this.inReports[HIDInputPotsReportID] = new HIDInputReport(HIDInputPotsReportID);
        this.inReports[HIDInputWheelsReportID] = new HIDInputReport(HIDInputWheelsReportID);

        // There are various of other HID report which doesn't seem to have any
        // immediate use but it is likely that some useful settings may be found
        // in them such as the wheel tension.

        this.outReports = [];
        this.outReports[128] = new HIDOutputReport(128, 94); // FIXME: hardcoded

        // Single motor output buffer for both wheels
        this.motorBuffMgr = new MotorOutputBuffMgr();

        this.effectUnit1 = new S4Mk3EffectUnit(1, this.inReports, this.outReports[128],
            {
                mixKnob: {inByte: 30},
                mainButton: {inByte: 1, inBit: 6, outByte: 62},
                knobs: [
                    {inByte: 32},
                    {inByte: 34},
                    {inByte: 36},
                ],
                buttons: [
                    {inByte: 1, inBit: 7, outByte: 63},
                    {inByte: 1, inBit: 3, outByte: 64},
                    {inByte: 1, inBit: 2, outByte: 65},
                ],
            }
        );
        this.effectUnit2 = new S4Mk3EffectUnit(2, this.inReports, this.outReports[128],
            {
                mixKnob: {inByte: 70},
                mainButton: {inByte: 9, inBit: 4, outByte: 73},
                knobs: [
                    {inByte: 72},
                    {inByte: 74},
                    {inByte: 76},
                ],
                buttons: [
                    {inByte: 9, inBit: 5, outByte: 74},
                    {inByte: 9, inBit: 6, outByte: 75},
                    {inByte: 9, inBit: 7, outByte: 76},
                ],
            }
        );

        // The interaction between the FX SELECT buttons and the QuickEffect enable buttons is rather complex.
        // It is easier to have this separate from the S4Mk3MixerColumn dhe FX SELECT buttons are not
        // really in the mixer columns.
        this.mixer = new Mixer(this.inReports, this.outReports);

        // There is no consistent offset between the left and right deck,
        // so every single components' IO needs to be specified individually
        // for both decks.
        // TODO: FIX ALL BYTE OFFSETS AND PUT IN HEADER. THIS IS VERY BAD
        // SOME OF THEM ARE OFFSET BY 1 BYTE, SOME ARE NOT, DEPENDING ON THE
        // INPUT REPORT. ALL BECAUSE OF A SLICE OPERATION ON THE INPUT. BAD!
        // THIS CAUSED LARGE AMOUNTS OF CONFUSION AND WASTED TIME TRYING TO
        // UNDERSTAND THE REAL HARDWARE SPEC FOR ANALYZING TRAFFIC
        this.leftDeck = new S4Mk3Deck(
            [1, 3], [DeckColors[0], DeckColors[2]], this.effectUnit1, this.mixer,
            this.inReports, this.outReports[128], this.motorBuffMgr,
            {
                playButton: {inByte: 4, inBit: 0, outByte: 55},
                cueButton: {inByte: 4, inBit: 1, outByte: 8},
                syncButton: {inByte: 5, inBit: 7, outByte: 14},
                syncMasterButton: {inByte: 0, inBit: 0, outByte: 15},
                hotcuePadModeButton: {inByte: 4, inBit: 2, outByte: 9},
                recordPadModeButton: {inByte: 4, inBit: 3, outByte: 56},
                samplesPadModeButton: {inByte: 4, inBit: 4, outByte: 57},
                mutePadModeButton: {inByte: 4, inBit: 5, outByte: 58},
                stemsPadModeButton: {inByte: 5, inBit: 0, outByte: 10},
                deckButtonLeft: {inByte: 5, inBit: 2},
                deckButtonRight: {inByte: 5, inBit: 3},
                deckButtonOutputByteOffset: 12,
                tempoFaderLED: {outByte: 11},
                shiftButton: {inByte: 5, inBit: 1, outByte: 59},
                leftEncoder: {inByte: 19, inBit: 0},
                leftEncoderPress: {inByte: 6, inBit: 2},
                rightEncoder: {inByte: 19, inBit: 4},
                rightEncoderPress: {inByte: 6, inBit: 5},
                libraryEncoder: {inByte: 20, inBit: 0},
                libraryEncoderPress: {inByte: 0, inBit: 1},
                turntableButton: {inByte: 5, inBit: 5, outByte: 17},
                jogButton: {inByte: 5, inBit: 4, outByte: 16},
                gridButton: {inByte: 5, inBit: 6, outByte: 18},
                reverseButton: {inByte: 1, inBit: 4, outByte: 60},
                fluxButton: {inByte: 1, inBit: 5, outByte: 61},
                libraryPlayButton: {inByte: 0, inBit: 5, outByte: 22},
                libraryStarButton: {inByte: 0, inBit: 4, outByte: 21},
                libraryPlaylistButton: {inByte: 1, inBit: 1, outByte: 20},
                libraryViewButton: {inByte: 1, inBit: 0, outByte: 19},
                pads: [
                    {inByte: 3, inBit: 5, outByte: 0},
                    {inByte: 3, inBit: 4, outByte: 1},
                    {inByte: 3, inBit: 7, outByte: 2},
                    {inByte: 3, inBit: 6, outByte: 3},

                    {inByte: 3, inBit: 3, outByte: 4},
                    {inByte: 3, inBit: 2, outByte: 5},
                    {inByte: 3, inBit: 1, outByte: 6},
                    {inByte: 3, inBit: 0, outByte: 7},
                ],
                tempoFader: {inByte: 12, inBit: 0, inBitLength: 16, inReport: this.inReports[HIDInputPotsReportID]},
                // the relative wheel value here is one byte offset from its position in the raw data, and is hardcoded as such later on. these entries really must be harmonized for readability and robustness. ZT
                wheelPosition: {inByte: 11, inBit: 0, inBitLength: 16, inReport: this.inReports[HIDInputWheelsReportID]},
                wheelAbsolute: {inByte: 15, inBit: 0, inBitLength: 16, inReport: this.inReports[HIDInputWheelsReportID]},
                wheelTouch: {inByte: 16, inBit: 4},
            }
        );

        this.rightDeck = new S4Mk3Deck(
            [2, 4], [DeckColors[1], DeckColors[3]], this.effectUnit2, this.mixer,
            this.inReports, this.outReports[128], this.motorBuffMgr,
            {
                playButton: {inByte: 12, inBit: 0, outByte: 66},
                cueButton: {inByte: 14, inBit: 5, outByte: 31},
                syncButton: {inByte: 14, inBit: 4, outByte: 37},
                syncMasterButton: {inByte: 10, inBit: 0, outByte: 38},
                hotcuePadModeButton: {inByte: 12, inBit: 2, outByte: 32},
                recordPadModeButton: {inByte: 12, inBit: 3, outByte: 67},
                samplesPadModeButton: {inByte: 12, inBit: 4, outByte: 68},
                mutePadModeButton: {inByte: 12, inBit: 5, outByte: 69},
                stemsPadModeButton: {inByte: 12, inBit: 1, outByte: 33},
                deckButtonLeft: {inByte: 14, inBit: 2},
                deckButtonRight: {inByte: 14, inBit: 3},
                deckButtonOutputByteOffset: 35,
                tempoFaderLED: {outByte: 34},
                shiftButton: {inByte: 14, inBit: 1, outByte: 70},
                leftEncoder: {inByte: 20, inBit: 4},
                leftEncoderPress: {inByte: 15, inBit: 5},
                rightEncoder: {inByte: 21, inBit: 0},
                rightEncoderPress: {inByte: 15, inBit: 2},
                libraryEncoder: {inByte: 21, inBit: 4},
                libraryEncoderPress: {inByte: 10, inBit: 1},
                turntableButton: {inByte: 14, inBit: 6, outByte: 40},
                jogButton: {inByte: 14, inBit: 0, outByte: 39},
                gridButton: {inByte: 14, inBit: 7, outByte: 41},
                reverseButton: {inByte: 10, inBit: 4, outByte: 71},
                fluxButton: {inByte: 10, inBit: 5, outByte: 72},
                libraryPlayButton: {inByte: 9, inBit: 2, outByte: 45},
                libraryStarButton: {inByte: 9, inBit: 1, outByte: 44},
                libraryPlaylistButton: {inByte: 9, inBit: 3, outByte: 43},
                libraryViewButton: {inByte: 9, inBit: 0, outByte: 42},
                pads: [
                    {inByte: 13, inBit: 5, outByte: 23},
                    {inByte: 13, inBit: 4, outByte: 24},
                    {inByte: 13, inBit: 7, outByte: 25},
                    {inByte: 13, inBit: 6, outByte: 26},

                    {inByte: 13, inBit: 3, outByte: 27},
                    {inByte: 13, inBit: 2, outByte: 28},
                    {inByte: 13, inBit: 1, outByte: 29},
                    {inByte: 13, inBit: 0, outByte: 30},
                ],
                tempoFader: {inByte: 10, inBit: 0, inBitLength: 16, inReport: this.inReports[HIDInputPotsReportID]},
                // the relative wheel value here is one byte offset from its position in the raw data, and is hardcoded as such later on. these entries really must be harmonized for readability and robustness. ZT
                wheelPosition: {inByte: 39, inBit: 0, inBitLength: 16, inReport: this.inReports[HIDInputWheelsReportID]},
                wheelAbsolute: {inByte: 43, inBit: 0, inBitLength: 16, inReport: this.inReports[HIDInputWheelsReportID]},
                wheelTouch: {inByte: 16, inBit: 5},
            }
        );

        const that = this;
        /* eslint no-unused-vars: "off" */
        const meterConnection = engine.makeConnection("[App]", "gui_tick_50ms_period_s", function(_value) {
            const deckMeters = new Uint8Array(78).fill(0);
            // Each column has 14 segments, but treat the top one specially for the clip indicator.
            const deckSegments = 13;
            for (let deckNum = 1; deckNum <= 4; deckNum++) {
                let deckGroup = `[Channel${deckNum}]`;
                if (that.leftDeck.shifted || that.rightDeck.shifted) {
                    if (engine.getValue(`[Auxiliary${deckNum}]`, "input_configured")) {
                        deckGroup = `[Auxiliary${deckNum}]`;
                    } else if (engine.getValue(deckNum !== 1 ? `[Microphone${deckNum}]` : "[Microphone]", "input_configured")) {
                        deckGroup = deckNum !== 1 ? `[Microphone${deckNum}]` : "[Microphone]";
                    }
                }
                const deckLevel = engine.getValue(deckGroup, "vu_meter");
                const columnBaseIndex = (deckNum - 1) * (deckSegments + 2);
                const scaledLevel = deckLevel * deckSegments;
                const segmentsToLightFully = Math.floor(scaledLevel);
                const partialSegmentValue = scaledLevel - segmentsToLightFully;
                if (segmentsToLightFully > 0) {
                    // There are 3 brightness levels per segment: off, dim, and full.
                    for (let i = 0; i <= segmentsToLightFully; i++) {
                        deckMeters[columnBaseIndex + i] = 127;
                    }
                    if (partialSegmentValue > 0.5 && segmentsToLightFully < deckSegments) {
                        deckMeters[columnBaseIndex + segmentsToLightFully + 1] = 125;
                    }
                }
                if (engine.getValue(deckGroup, "peak_indicator")) {
                    deckMeters[columnBaseIndex + deckSegments + 1] = 127;
                }
            }
            // There are more bytes in the report which seem like they should be for the main
            // mix meters, but setting those bytes does not do anything, except for lighting
            // the clip lights on the main mix meters.
            controller.sendOutputReport(HIDOutputVUMeterReportID, deckMeters.buffer);
        });
        if (UseMotors) {
            this.leftMotor = new S4Mk3MotorManager(this.leftDeck);
            this.rightMotor = new S4Mk3MotorManager(this.rightDeck);
            // Requesting a timer interval of 2ms to match sampling rate of wheel sensors
            // max value is capped in controllerscriptinterfacelegacy.cpp
            // normally capped at 20ms but we have removed this cap for motor control
            engine.beginTimer(2, this.motorCallback.bind(this));
            // Even if we remove the cap, QT can't guarantee that any given system
            // will maintain a sub-20ms timer period. See:
            // https://doc.qt.io/qt-5/qobject.html#startTimer
        }
    }
    motorCallback() {
        this.leftMotor.tick();
        this.rightMotor.tick();
        controller.sendOutputReport(49, this.motorBuffMgr.getBuff(), true);
    }
    incomingData(data) {
        // The first byte of the HID report is the reportID
        const reportId = data[0];
        if (reportId in this.inReports && reportId !== HIDInputWheelsReportID) {
            // Slicing out the first data point is actively harmful to code legibility later on.
            // FIXME: PASS FULL DATA BUFFER TO INPUT HANDLER
            this.inReports[reportId].handleInput(data.buffer.slice(1));
        } else if (reportId === HIDInputWheelsReportID) {
            // FIXME: input report # 3 comes the most frequently, so it should
            //        be at the start of the conditional block for optimization:
            //        saves a multi-condition check every time an input comes in.
            // The 32 bit unsigned ints at bytes 8 and 36 always have exactly the same value,
            // so only process one of them. This must be processed before the wheel positions.
            const oldWheelTimer = wheelTimer;
            const view = new DataView(data.buffer);
            wheelTimer = view.getUint32(8, true);

            // WheelTimerDelta code appears unused. Commenting out.
            // Processing first value; no previous value to compare with.
            // if (oldWheelTimer === null) {
            //     return;
            // }
            // wheelTimerDelta = wheelTimer - oldWheelTimer;
            // if (wheelTimerDelta < 0) {
            //     wheelTimerDelta += wheelTimerMax;
            // }

            // FIXME: the byte offsets below don't match with the ones in the deck definitions
            //        the offsets here are the correct ones with reference to the entire HID report
            this.leftDeck.wheelPosition.input(view.getUint32(12, true), view.getUint32(8, true));
            this.rightDeck.wheelPosition.input(view.getUint32(40, true), view.getUint32(36, true));
        } else {
            console.warn(`Unsupported HID repord with ID ${reportId}. Contains: ${data}`);
        }
    }
    init() {
        // sending these magic reports is required for the jog wheel LEDs to work
        const wheelLEDinitReport = new Uint8Array(26).fill(0);
        wheelLEDinitReport[1] = 1;
        wheelLEDinitReport[2] = 3;
        controller.sendOutputReport(48, wheelLEDinitReport.buffer, true);
        wheelLEDinitReport[0] = 1;
        controller.sendOutputReport(48, wheelLEDinitReport.buffer);

        // Init wheel timer data
        wheelTimer = null;
        wheelTimerDelta = 0;

        // get state of knobs and faders
        for (const repordId of [0x01, 0x02]) {
            this.inReports[repordId].handleInput(controller.getInputReport(repordId));
        }
    }
    shutdown() {
        // button LEDs
        controller.sendOutputReport(128, new Uint8Array(94).fill(0).buffer);

        // meter LEDs
        controller.sendOutputReport(HIDOutputVUMeterReportID, new Uint8Array(78).fill(0).buffer);

        const wheelOutput = new Uint8Array(40).fill(0);
        // left wheel LEDs
        controller.sendOutputReport(50, wheelOutput.buffer, true);
        // right wheel LEDs
        wheelOutput[0] = 1;
        controller.sendOutputReport(50, wheelOutput.buffer, true);
    }
}

/* eslint no-unused-vars: "off", no-var: "off" */
var TraktorS4MK3 = new S4MK3();