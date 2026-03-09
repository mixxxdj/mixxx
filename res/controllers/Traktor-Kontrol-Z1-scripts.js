//
// Native Instruments Traktor Kontrol Z1 HID controller script for Mixxx 2.4
// -------------------------------------------------------------------------
// Based on: NI Traktor Kontrol series scripts by leifhelm, mi01 & xeruf
// Author: djantti
//

class TraktorZ1Class {
    constructor() {
        this.controller = new HIDController();

        // Modifier state
        this.modePressed = false;

        // VU meters
        this.vuLeftConnection = {};
        this.vuRightConnection = {};
        this.vuMeterThresholds = {"vu-30": (1 / 7), "vu-15": (2 / 7), "vu-6": (3 / 7), "vu-3": (4 / 7), "vu0": (5 / 7), "vu3": (6 / 7), "vu6": (7 / 7)};

        // Calibration data
        this.rawCalibration = {};
        this.calibration = null;
    }

    init(_id) {
        this.id = _id;

        this.calibrate();
        this.registerInputPackets();
        this.registerOutputPackets();
        this.readCurrentPosition();
        this.enableSoftTakeover();

        console.log(this.id + " initialized");
    }

    registerInputPackets() {
        const InputReport0x01 = new HIDPacket("InputReport0x01", 0x01, this.inputReportCallback.bind(this));

        // Mode button
        this.registerInputButton(InputReport0x01, "[ControlX]", "!mode", 0x1D, 0x02, this.modeHandler.bind(this));

        // Headphone buttons
        this.registerInputButton(InputReport0x01, "[Channel1]", "!pfl", 0x1D, 0x10, this.headphoneHandler.bind(this));
        this.registerInputButton(InputReport0x01, "[Channel2]", "!pfl", 0x1D, 0x01, this.headphoneHandler.bind(this));

        // FX buttons
        this.registerInputButton(InputReport0x01, "[Channel1]", "!fx", 0x1D, 0x04, this.fxHandler.bind(this));
        this.registerInputButton(InputReport0x01, "[Channel2]", "!fx", 0x1D, 0x08, this.fxHandler.bind(this));

        // EQ knobs
        this.registerInputScaler(InputReport0x01, "[EqualizerRack1_[Channel1]_Effect1]", "parameter3", 0x03, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(InputReport0x01, "[EqualizerRack1_[Channel1]_Effect1]", "parameter2", 0x05, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(InputReport0x01, "[EqualizerRack1_[Channel1]_Effect1]", "parameter1", 0x07, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(InputReport0x01, "[EqualizerRack1_[Channel2]_Effect1]", "parameter3", 0x0D, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(InputReport0x01, "[EqualizerRack1_[Channel2]_Effect1]", "parameter2", 0x0F, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(InputReport0x01, "[EqualizerRack1_[Channel2]_Effect1]", "parameter1", 0x11, 0xFFFF, this.parameterHandler.bind(this));

        // FX knobs
        this.registerInputScaler(InputReport0x01, "[QuickEffectRack1_[Channel1]]", "super1", 0x09, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(InputReport0x01, "[QuickEffectRack1_[Channel2]]", "super1", 0x13, 0xFFFF, this.parameterHandler.bind(this));

        // Gain knobs
        this.registerInputScaler(InputReport0x01, "[Channel1]", "pregain", 0x01, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(InputReport0x01, "[Channel2]", "pregain", 0x0B, 0xFFFF, this.parameterHandler.bind(this));

        // Headphone mix
        this.registerInputScaler(InputReport0x01, "[Master]", "headMix", 0x15, 0xFFFF, this.parameterHandler.bind(this));

        // Volume faders
        this.registerInputScaler(InputReport0x01, "[Channel1]", "volume", 0x17, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(InputReport0x01, "[Channel2]", "volume", 0x19, 0xFFFF, this.parameterHandler.bind(this));

        // Crossfader
        this.registerInputScaler(InputReport0x01, "[Master]", "crossfader", 0x1B, 0xFFFF, this.crossfaderHandler.bind(this));

        this.controller.registerInputPacket(InputReport0x01);
    }

    registerOutputPackets() {
        const OutputReport0x80 = new HIDPacket("OutputReport0x80", 0x80);

        OutputReport0x80.addOutput("[ControlX]", "mode", 0x13, "B");

        OutputReport0x80.addOutput("[Channel1]", "pfl", 0x0F, "B");
        OutputReport0x80.addOutput("[Channel2]", "pfl", 0x10, "B");

        OutputReport0x80.addOutput("[Channel1]", "play_indicator", 0x11, "B");
        OutputReport0x80.addOutput("[Channel2]", "play_indicator", 0x14, "B");

        OutputReport0x80.addOutput("[QuickEffectRack1_[Channel1]]", "enabled", 0x12, "B");
        OutputReport0x80.addOutput("[QuickEffectRack1_[Channel2]]", "enabled", 0x15, "B");

        OutputReport0x80.addOutput("[Channel1]", "vu-30", 0x01, "B");
        OutputReport0x80.addOutput("[Channel1]", "vu-15", 0x02, "B");
        OutputReport0x80.addOutput("[Channel1]", "vu-6", 0x03, "B");
        OutputReport0x80.addOutput("[Channel1]", "vu-3", 0x04, "B");
        OutputReport0x80.addOutput("[Channel1]", "vu0", 0x05, "B");
        OutputReport0x80.addOutput("[Channel1]", "vu3", 0x06, "B");
        OutputReport0x80.addOutput("[Channel1]", "vu6", 0x07, "B");

        OutputReport0x80.addOutput("[Channel2]", "vu-30", 0x08, "B");
        OutputReport0x80.addOutput("[Channel2]", "vu-15", 0x09, "B");
        OutputReport0x80.addOutput("[Channel2]", "vu-6", 0x0A, "B");
        OutputReport0x80.addOutput("[Channel2]", "vu-3", 0x0B, "B");
        OutputReport0x80.addOutput("[Channel2]", "vu0", 0x0C, "B");
        OutputReport0x80.addOutput("[Channel2]", "vu3", 0x0D, "B");
        OutputReport0x80.addOutput("[Channel2]", "vu6", 0x0E, "B");

        this.controller.registerOutputPacket(OutputReport0x80);

        engine.makeConnection("[QuickEffectRack1_[Channel1]]", "enabled", this.outputHandler.bind(this));
        engine.makeConnection("[QuickEffectRack1_[Channel2]]", "enabled", this.outputHandler.bind(this));

        engine.makeConnection("[Channel1]", "pfl", this.outputHandler.bind(this));
        engine.makeConnection("[Channel2]", "pfl", this.outputHandler.bind(this));

        this.vuLeftConnection = engine.makeUnbufferedConnection("[Channel1]", "vu_meter", this.vuMeterHandler.bind(this));
        this.vuRightConnection = engine.makeUnbufferedConnection("[Channel2]", "vu_meter", this.vuMeterHandler.bind(this));

        this.lightDeck(false);
    }

    calibrate() {
        this.rawCalibration.faders = new Uint8Array(0x20 * 3);
        this.rawCalibration.faders.set(new Uint8Array(controller.getFeatureReport(0xD1)), 0x00);
        this.rawCalibration.faders.set(new Uint8Array(controller.getFeatureReport(0xD2)), 0x20);
        this.rawCalibration.faders.set(new Uint8Array(controller.getFeatureReport(0xD3)), 0x40);
        this.calibration = this.parseRawCalibration();
    }

    parseRawCalibration() {
        return {
            crossfader: this.parseCrossfaderCalibration(0x3C),
        };
    }

    parseCrossfaderCalibration(index) {
        const data = this.rawCalibration.faders;
        return {
            min: this.parseUint16Le(data, index),
            max: this.parseUint16Le(data, index+2),
        };
    }

    parseUint16Le(data, index) {
        return data[index] + (data[index+1]<<8);
    }

    readCurrentPosition() {
        // Sync on-screen controls with controller knob positions
        const report0x01 = new Uint8Array(controller.getInputReport(0x01));
        // The first packet is ignored by HIDController
        this.controller.parsePacket([0x01, ...Array.from(report0x01.map(x => x ^ 0xFF))]);
        this.controller.parsePacket([0x01, ...Array.from(report0x01)]);
    }

    enableSoftTakeover() {
        // Soft takeover for all knobs and faders
        engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", true);
        engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", true);
        engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", true);

        engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", true);
        engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", true);
        engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", true);

        engine.softTakeover("[QuickEffectRack1_[Channel1]]", "super1", true);
        engine.softTakeover("[QuickEffectRack1_[Channel2]]", "super1", true);

        engine.softTakeover("[Channel1]", "pregain", true);
        engine.softTakeover("[Channel2]", "pregain", true);

        engine.softTakeover("[Master]", "headMix", true);

        engine.softTakeover("[Channel1]", "volume", true);
        engine.softTakeover("[Channel2]", "volume", true);

        engine.softTakeover("[Master]", "crossfader", true);
    }

    registerInputButton(inputReport, group, name, offset, bitmask, callback) {
        inputReport.addControl(group, name, offset, "B", bitmask);
        inputReport.setCallback(group, name, callback);
    }

    registerInputScaler(inputReport, group, name, offset, bitmask, callback) {
        inputReport.addControl(group, name, offset, "H", bitmask);
        inputReport.setCallback(group, name, callback);
    }

    modeHandler(field) {
        this.modePressed = field.value;
        this.outputHandler(field.value, field.group, "mode");
    }

    headphoneHandler(field) {
        if (field.value === 0) {
            return;
        }
        // Go to cue and stop when modifier is active
        if (this.modePressed) {
            engine.setValue(field.group, "cue_gotoandstop", field.value);
        } else {
            script.toggleControl(field.group, "pfl");
        }
    }

    fxHandler(field) {
        if (field.value === 0) {
            // Always clear play indicator on button release
            this.controller.setOutput(field.group, "play_indicator", 0x00, true);
            return;
        }
        // Control playback when modifier is active
        if (this.modePressed) {
            // Match play indicator (red led) brightness to fx indicator (blue led)
            const ledBrightness = engine.getValue("[QuickEffectRack1_" + field.group + "]", "enabled") ? 0x7F : 0x0A;
            this.controller.setOutput(field.group, "play_indicator", ledBrightness, true);
            script.toggleControl(field.group, "play");
        } else {
            script.toggleControl("[QuickEffectRack1_" + field.group + "]", "enabled");
        }
    }

    vuMeterHandler(value, group, _key) {
        const vuKeys = Object.keys(this.vuMeterThresholds);
        for (let i = 0; i < vuKeys.length; ++i) {
            // Avoid spamming HID by only sending last LED update
            const last = i === (vuKeys.length - 1);
            if (this.vuMeterThresholds[vuKeys[i]] > value) {
                this.controller.setOutput(group, vuKeys[i], 0x00, last);
            } else {
                this.controller.setOutput(group, vuKeys[i], 0x7F, last);
            }
        }
    }

    parameterHandler(field) {
        engine.setParameter(field.group, field.name, field.value / 4095);
    }

    crossfaderHandler(field) {
        // Crossfader value don't reach boundaries and need to use calibration values
        // Also apply extra safe margins
        const safeMargins = 5;
        const min = this.calibration.crossfader.min;
        const max = this.calibration.crossfader.max;
        const value = script.absoluteLin(field.value, 0, 1, min + safeMargins, max - safeMargins);
        engine.setParameter(field.group, field.name, value);
    }

    outputHandler(value, group, key) {
        let ledValue;
        if (value === 0 || value === false) {
            // Off value (dimmed)
            ledValue = 0x0A;
        } else if (value === 1 || value === true) {
            // On value
            ledValue = 0x7F;
        }
        this.controller.setOutput(group, key, ledValue, true);
    }

    lightDeck(switchOff) {
        let softLight = 0x0A;
        let fullLight = 0x7F;
        let ledBrightness;

        if (switchOff) {
            softLight = 0x00;
            fullLight = 0x00;
        }

        this.controller.setOutput("[ControlX]", "mode", softLight, true);

        ledBrightness = engine.getValue("[QuickEffectRack1_[Channel1]]", "enabled") ? fullLight : softLight;
        this.controller.setOutput("[QuickEffectRack1_[Channel1]]", "enabled", ledBrightness, true);
        ledBrightness = engine.getValue("[QuickEffectRack1_[Channel2]]", "enabled") ? fullLight : softLight;
        this.controller.setOutput("[QuickEffectRack1_[Channel2]]", "enabled", ledBrightness, true);

        ledBrightness = engine.getValue("[Channel1]", "pfl") ? fullLight : softLight;
        this.controller.setOutput("[Channel1]", "pfl", ledBrightness, true);
        ledBrightness = engine.getValue("[Channel2]", "pfl") ? fullLight : softLight;
        this.controller.setOutput("[Channel2]", "pfl", ledBrightness, true);
    }

    inputReportCallback(packet, data) {
        for (const name in data) {
            if (Object.hasOwnProperty.call(data, name)) {
                this.controller.processButton(data[name]);
            }
        }
    }

    shutdown() {
        // Deactivate all LEDs
        this.lightDeck(true);
        console.log(this.id + " shut down");
    }

    incomingData(data, length) {
        this.controller.parsePacket(data, length);
    }
}

var TraktorZ1 = new TraktorZ1Class;  // eslint-disable-line no-var, no-unused-vars
