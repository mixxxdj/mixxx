//
// Native Instruments Traktor Kontrol Z1 HID controller script for Mixxx 2.4
// -------------------------------------------------------------------------
// Based on: Kontrol S2 MK3 script by mi01 and Kontrol Z1 script by xeruf
// Author: djantti
//

class TraktorZ1Class {
    constructor() {
        this.controller = new HIDController();

        // Modifier state
        this.modePressed = false;

        // VuMeter
        this.vuLeftConnection = {};
        this.vuRightConnection = {};
        this.vuMeterThresholds = {"vu-30": (1 / 7), "vu-15": (2 / 7), "vu-6": (3 / 7), "vu-3": (4 / 7), "vu0": (5 / 7), "vu3": (6 / 7), "vu6": (7 / 7)};
    }

    init(_id) {
        this.id = _id;
        this.registerInputPackets();
        this.registerOutputPackets();
        console.log(this.id + " initialized");
    }

    registerInputPackets() {
        const message = new HIDPacket("message", 0x1, this.messageCallback.bind(this));

        // Mode button
        this.registerInputButton(message, "[ControlX]", "!mode", 0x1D, 0x2, this.modeHandler.bind(this));

        // Headphone buttons
        this.registerInputButton(message, "[Channel1]", "!pfl", 0x1D, 0x10, this.headphoneHandler.bind(this));
        this.registerInputButton(message, "[Channel2]", "!pfl", 0x1D, 0x1, this.headphoneHandler.bind(this));

        // FX buttons
        this.registerInputButton(message, "[Channel1]", "!fx", 0x1D, 0x4, this.fxHandler.bind(this));
        this.registerInputButton(message, "[Channel2]", "!fx", 0x1D, 0x8, this.fxHandler.bind(this));

        // EQ knobs
        this.registerInputScaler(message, "[EqualizerRack1_[Channel1]_Effect1]", "parameter3", 0x3, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(message, "[EqualizerRack1_[Channel1]_Effect1]", "parameter2", 0x5, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(message, "[EqualizerRack1_[Channel1]_Effect1]", "parameter1", 0x7, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(message, "[EqualizerRack1_[Channel2]_Effect1]", "parameter3", 0xD, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(message, "[EqualizerRack1_[Channel2]_Effect1]", "parameter2", 0xF, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(message, "[EqualizerRack1_[Channel2]_Effect1]", "parameter1", 0x11, 0xFFFF, this.parameterHandler.bind(this));

        // FX knobs
        this.registerInputScaler(message, "[QuickEffectRack1_[Channel1]]", "super1", 0x9, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(message, "[QuickEffectRack1_[Channel2]]", "super1", 0x13, 0xFFFF, this.parameterHandler.bind(this));

        // Gain knobs
        this.registerInputScaler(message, "[Channel1]", "pregain", 0x1, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(message, "[Channel2]", "pregain", 0xB, 0xFFFF, this.parameterHandler.bind(this));

        // Headphone mix
        this.registerInputScaler(message, "[Master]", "headMix", 0x15, 0xFFFF, this.parameterHandler.bind(this));

        // Volume faders
        this.registerInputScaler(message, "[Channel1]", "volume", 0x17, 0xFFFF, this.parameterHandler.bind(this));
        this.registerInputScaler(message, "[Channel2]", "volume", 0x19, 0xFFFF, this.parameterHandler.bind(this));

        // Crossfader
        this.registerInputScaler(message, "[Master]", "crossfader", 0x1B, 0xFFFF, this.parameterHandler.bind(this));

        this.controller.registerInputPacket(message);

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

    registerInputButton(message, group, name, offset, bitmask, callback) {
        message.addControl(group, name, offset, "B", bitmask);
        message.setCallback(group, name, callback);
    }

    registerInputScaler(message, group, name, offset, bitmask, callback) {
        message.addControl(group, name, offset, "H", bitmask);
        message.setCallback(group, name, callback);
    }

    registerOutputPackets() {
        const output = new HIDPacket("output", 0x80);

        output.addOutput("[ControlX]", "mode", 0x13, "B");

        output.addOutput("[Channel1]", "pfl", 0xF, "B");
        output.addOutput("[Channel2]", "pfl", 0x10, "B");

        output.addOutput("[Channel1]", "play_indicator", 0x11, "B");
        output.addOutput("[Channel2]", "play_indicator", 0x14, "B");

        output.addOutput("[QuickEffectRack1_[Channel1]]", "enabled", 0x12, "B");
        output.addOutput("[QuickEffectRack1_[Channel2]]", "enabled", 0x15, "B");

        output.addOutput("[Channel1]", "vu-30", 0x1, "B");
        output.addOutput("[Channel1]", "vu-15", 0x2, "B");
        output.addOutput("[Channel1]", "vu-6", 0x3, "B");
        output.addOutput("[Channel1]", "vu-3", 0x4, "B");
        output.addOutput("[Channel1]", "vu0", 0x5, "B");
        output.addOutput("[Channel1]", "vu3", 0x6, "B");
        output.addOutput("[Channel1]", "vu6", 0x7, "B");

        output.addOutput("[Channel2]", "vu-30", 0x8, "B");
        output.addOutput("[Channel2]", "vu-15", 0x9, "B");
        output.addOutput("[Channel2]", "vu-6", 0xA, "B");
        output.addOutput("[Channel2]", "vu-3", 0xB, "B");
        output.addOutput("[Channel2]", "vu0", 0xC, "B");
        output.addOutput("[Channel2]", "vu3", 0xD, "B");
        output.addOutput("[Channel2]", "vu6", 0xE, "B");

        this.controller.registerOutputPacket(output);

        engine.makeConnection("[QuickEffectRack1_[Channel1]]", "enabled", this.outputHandler.bind(this));
        engine.makeConnection("[QuickEffectRack1_[Channel2]]", "enabled", this.outputHandler.bind(this));

        engine.makeConnection("[Channel1]", "pfl", this.outputHandler.bind(this));
        engine.makeConnection("[Channel2]", "pfl", this.outputHandler.bind(this));

        this.vuLeftConnection = engine.makeUnbufferedConnection("[Channel1]", "vu_meter", this.vuMeterHandler.bind(this));
        this.vuRightConnection = engine.makeUnbufferedConnection("[Channel2]", "vu_meter", this.vuMeterHandler.bind(this));

        this.lightDeck(false);
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
            this.controller.setOutput(field.group, "play_indicator", 0x0, true);
            return;
        }
        // Control playback when modifier is active
        if (this.modePressed) {
            // Match play indicator (red led) brightness to fx indicator (blue led)
            const current = engine.getValue("[QuickEffectRack1_" + field.group + "]", "enabled") ? 0x7F : 0xA;
            this.controller.setOutput(field.group, "play_indicator", current, true);
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
                this.controller.setOutput(group, vuKeys[i], 0x7E, last);
            }
        }
    }

    parameterHandler(field) {
        engine.setParameter(field.group, field.name, field.value / 4095);
    }

    outputHandler(value, group, key) {
        let ledValue;
        if (value === 0 || value === false) {
            // Off value (dimmed)
            ledValue = 0xA;
        } else if (value === 1 || value === true) {
            // On value
            ledValue = 0x7F;
        }
        this.controller.setOutput(group, key, ledValue, true);
    }

    lightDeck(switchOff) {
        let softLight = 0xA;
        let fullLight = 0x7F;
        let current;

        if (switchOff) {
            softLight = 0x00;
            fullLight = 0x00;
        }

        this.controller.setOutput("[ControlX]", "mode", softLight, true);

        current = engine.getValue("[QuickEffectRack1_[Channel1]]", "enabled") ? fullLight : softLight;
        this.controller.setOutput("[QuickEffectRack1_[Channel1]]", "enabled", current, true);
        current = engine.getValue("[QuickEffectRack1_[Channel2]]", "enabled") ? fullLight : softLight;
        this.controller.setOutput("[QuickEffectRack1_[Channel2]]", "enabled", current, true);

        current = engine.getValue("[Channel1]", "pfl") ? fullLight : softLight;
        this.controller.setOutput("[Channel1]", "pfl", current, true);
        current = engine.getValue("[Channel2]", "pfl") ? fullLight : softLight;
        this.controller.setOutput("[Channel2]", "pfl", current, true);
    }

    messageCallback(packet, data) {
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
