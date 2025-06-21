"use strict";

///////////////////////////////////////////////////////////////////////////////////
//
// Traktor Kontrol S3 HID controller script v2.00
// Last modification: January 2023
// Authors: Owen Williams, Robbert van der Helm
// https://manual.mixxx.org/latest/en/hardware/controllers/native_instruments_traktor_kontrol_s3.html
//
///////////////////////////////////////////////////////////////////////////////////
//
// TODO:
//   * star button
//
///////////////////////////////////////////////////////////////////////////////////

var TraktorS3 = {};

// ==== Friendly User Configuration ====
// This controller script has two modes for controlling FX:
//
// - A mode focussing on quick effect chain presets, emulating the intended
//   behavior of the Mixer FX section on the Traktor Kontrol S3. This is the
//   default behavior. To make best use of this mode, you will need to make the
//   following configuration changes:
//
//   - The 'Keep superknob position' option in the Effects preferences page
//     should be enabled.
//   - The very first quick effect preset in the quick effect presets list on
//     the same preferences page should be set to the Moog Filter preset or
//     another filter preset.
//   - The next four quick effect presets should contain that exact same filter
//     effect, plus another effect. Delays, reverbs, flangers, trance gates, and
//     white noise are some examples of effects that would work well here.
//
// - Another mode that gives detailed control over Mixxx's individual effect
//   sections instead of focusing on the quick effects. This mode is more
//   complex to use as the result of the S3's limited number of buttons and
//   knobs dedicated to effects.
//
// The first mode is dubbed 'quick effect mode' and it is enabled by default.
// Disable this option to use the second, Mixxx-specific mode. See the readme at
// https://manual.mixxx.org/latest/en/hardware/controllers/native_instruments_traktor_kontrol_s3.html
// for more information on how to use these modes.
TraktorS3.QuickEffectMode = engine.getSetting("fxMode") === "QUICK_EFFECT";
// When enabled, set all channels to the first FX chain on startup. Otherwise
// the quick FX chain assignments from the last Mixxx run are preserved.
TraktorS3.QuickEffectModeDefaultToFilter = engine.getSetting("fxResetOnStart");
// When enabled, the FX Enable buttons will use the colors set in
// `TraktorS3.ChannelColors` when the filter effect is selected. Disabling this
// will use the Filter button's orange color instead.
TraktorS3.QuickEffectModeChannelColors = engine.getSetting("fxUseChannelColors");

// The pitch slider can operate either in absolute or relative mode.
// In absolute mode:
// * Moving the pitch slider works like normal
// * Mixxx will use soft-takeover
// * Pressing shift will adjust musical pitch instead of rate
// * Keylock toggles on with down-press.
//
// In relative mode:
// * The slider always moves, unless it has hit the end of the range inside Mixxx
// * No soft-takeover
// * Hold shift to move the pitch slider without adjusting the rate
// * Hold keylock and move the pitch slider to adjust musical pitch
// * keylock will still toggle on, but on release, not press.
TraktorS3.PitchSliderRelativeMode = engine.getSetting("pitchMode") === "PITCH_RELATIVE";

// The Samplers can operate two ways.
// With SamplerModePressAndHold = false, tapping a Sampler button will start the
// sample playing.  Pressing the button again will stop playback.
// With SamplerModePressAndHold = true, a Sample will play while you hold the
// button down.  Letting go will stop playback.
TraktorS3.SamplerModePressAndHold = engine.getSetting("samplerModePressAndHold");

// When this option is true, start up with the jog button lit, which means touching the job wheel
// enables scratch mode.
TraktorS3.JogDefaultOn = engine.getSetting("jogDefaultOn");

// If true, the sampler buttons on Deck 1 are samplers 1-8 and the sampler buttons on Deck 2 are
// 9-16.  If false, both decks are samplers 1-8.
TraktorS3.SixteenSamplers = engine.getSetting("sixteenSamplers");

// You can choose the colors you want for each channel. The list of colors is:
// RED, CARROT, ORANGE, HONEY, YELLOW, LIME, GREEN, AQUA, CELESTE, SKY, BLUE,
// PURPLE, FUCHSIA, MAGENTA, AZALEA, SALMON, WHITE
// Some colors may look odd because of how they are encoded inside the controller.
TraktorS3.ChannelColors = {
    "[Channel1]": engine.getSetting("chan1Color"),
    "[Channel2]": engine.getSetting("chan2Color"),
    "[Channel3]": engine.getSetting("chan3Color"),
    "[Channel4]": engine.getSetting("chan4Color")
};

// Each color has four brightnesses, so these values can be between 0 and 3.
TraktorS3.LEDDimValue = 0x00;
TraktorS3.LEDBrightValue = 0x02;

// By default the jog wheel's behavior when rotating it matches 33 1/3 rpm
// vinyl. Changing this value to 2.0 causes a single rotation of the platter to
// result in twice as much movement, and a value of 0.5 causes the amount of
// movement to be halved.
TraktorS3.JogSpeedMultiplier = 1.0;

// Parameters for the jog wheel smoothing while scratching
TraktorS3.Alpha = 1.0 / 8;
TraktorS3.Beta = TraktorS3.Alpha / 32;

// These options can be set to non-null values to initialize the beat jump and
// loop sizes and sync and quantize states for all four decks when the
// controller is connected
TraktorS3.DefaultBeatJumpSize = null; // 32
TraktorS3.DefaultBeatLoopLength = null; // 32
TraktorS3.DefaultSyncEnabled = null; // true
TraktorS3.DefaultQuantizeEnabled = null; // true
TraktorS3.DefaultKeylockEnabled = null; // true
// -1 for left, 0 for center/not assigned, 1 for right
TraktorS3.DefaultCrossfaderAssignments = [null, null, null, null]; // [0, 0, 0, 0]

// Set to true to output debug messages and debug light outputs.
TraktorS3.DebugMode = false;

TraktorS3.Controller = class {
    constructor() {
        this.hid = new HIDController();

        // When true, packets will not be sent to the controller. Good for doing
        // mass updates.
        this.batchingOutputs = false;

        // "5" is the "filter" button below the other 4.
        this.fxButtonState = {1: false, 2: false, 3: false, 4: false, 5: false};

        this.masterVuMeter = {
            "vu_meter_left": {
                connection: null,
                updated: false,
                value: 0
            },
            "vu_meter_right": {
                connection: null,
                updated: false,
                value: 0
            }
        };

        this.guiTickConnection = {};

        // The S3 has a set of predefined colors for many buttons. They are not
        // mapped by RGB, but 16 colors, each with 4 levels of brightness, plus
        // white.
        this.hid.LEDColors = {
            OFF: 0x00,
            RED: 0x04,
            CARROT: 0x08,
            ORANGE: 0x0C,
            HONEY: 0x10,
            YELLOW: 0x14,
            LIME: 0x18,
            GREEN: 0x1C,
            AQUA: 0x20,
            CELESTE: 0x24,
            SKY: 0x28,
            BLUE: 0x2C,
            PURPLE: 0x30,
            FUCHSIA: 0x34,
            MAGENTA: 0x38,
            AZALEA: 0x3C,
            SALMON: 0x40,
            WHITE: 0x44
        };

        // FX 5 is the Filter
        this.fxLEDValue = {
            0: this.hid.LEDColors.PURPLE,
            1: this.hid.LEDColors.RED,
            2: this.hid.LEDColors.GREEN,
            3: this.hid.LEDColors.CELESTE,
            4: this.hid.LEDColors.YELLOW,
        };

        this.colorMap = new ColorMapper({
            0xCC0000: this.hid.LEDColors.RED,
            0xCC5E00: this.hid.LEDColors.CARROT,
            0xCC7800: this.hid.LEDColors.ORANGE,
            0xCC9200: this.hid.LEDColors.HONEY,

            0xCCCC00: this.hid.LEDColors.YELLOW,
            0x81CC00: this.hid.LEDColors.LIME,
            0x00CC00: this.hid.LEDColors.GREEN,
            0x00CC49: this.hid.LEDColors.AQUA,

            0x00CCCC: this.hid.LEDColors.CELESTE,
            0x0091CC: this.hid.LEDColors.SKY,
            0x0000CC: this.hid.LEDColors.BLUE,
            0xCC00CC: this.hid.LEDColors.PURPLE,

            0xCC0091: this.hid.LEDColors.FUCHSIA,
            0xCC0079: this.hid.LEDColors.MAGENTA,
            0xCC477E: this.hid.LEDColors.AZALEA,
            0xCC4761: this.hid.LEDColors.SALMON,

            0xCCCCCC: this.hid.LEDColors.WHITE,
        });

        // State for controller input loudness setting
        this.inputModeLine = false;

        // If true, channel 4 is in input mode
        this.channel4InputMode = false;

        // Represents the first-pressed deck switch button, used for tracking
        // deck clones.
        this.deckSwitchPressed = "";

        // callbacks
        this.samplerCallbacks = [];
    }

    registerInputPackets() {
        const messageShort = new HIDPacket("shortmessage", 0x01, TraktorS3.messageCallback);
        const messageLong = new HIDPacket("longmessage", 0x02, TraktorS3.messageCallback);

        for (const idx in this.Decks) {
            const deck = this.Decks[idx];
            deck.registerInputs(messageShort, messageLong);
        }

        this.registerInputButton(messageShort, "[Channel1]", "!switchDeck", 0x02, 0x02, TraktorS3.Controller.prototype.deckSwitchHandler.bind(this));
        this.registerInputButton(messageShort, "[Channel2]", "!switchDeck", 0x05, 0x04, TraktorS3.Controller.prototype.deckSwitchHandler.bind(this));
        this.registerInputButton(messageShort, "[Channel3]", "!switchDeck", 0x02, 0x04, TraktorS3.Controller.prototype.deckSwitchHandler.bind(this));
        this.registerInputButton(messageShort, "[Channel4]", "!switchDeck", 0x05, 0x08, TraktorS3.Controller.prototype.deckSwitchHandler.bind(this));

        // Headphone buttons
        this.registerInputButton(messageShort, "[Channel1]", "pfl", 0x08, 0x01, TraktorS3.Controller.prototype.headphoneHandler.bind(this));
        this.registerInputButton(messageShort, "[Channel2]", "pfl", 0x08, 0x02, TraktorS3.Controller.prototype.headphoneHandler.bind(this));
        this.registerInputButton(messageShort, "[Channel3]", "pfl", 0x07, 0x80, TraktorS3.Controller.prototype.headphoneHandler.bind(this));
        this.registerInputButton(messageShort, "[Channel4]", "pfl", 0x08, 0x04, TraktorS3.Controller.prototype.headphoneHandler.bind(this));

        // EXT Button
        this.registerInputButton(messageShort, "[Master]", "!extButton", 0x07, 0x04, TraktorS3.Controller.prototype.extModeHandler.bind(this));

        this.fxController.registerInputs(messageShort, messageLong);

        this.hid.registerInputPacket(messageShort);

        this.registerInputScaler(messageLong, "[Channel1]", "volume", 0x05, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[Channel2]", "volume", 0x07, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[Channel3]", "volume", 0x03, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[Channel4]", "volume", 0x09, 0xFFFF, this.parameterHandler);

        this.registerInputScaler(messageLong, "[Channel1]", "pregain", 0x11, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[Channel2]", "pregain", 0x13, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[Channel3]", "pregain", 0x0F, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[Channel4]", "pregain", 0x15, 0xFFFF, this.parameterHandler);

        this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter3", 0x25, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter2", 0x27, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter1", 0x29, 0xFFFF, this.parameterHandler);

        this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter3", 0x2B, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter2", 0x2D, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter1", 0x2F, 0xFFFF, this.parameterHandler);

        this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel3]_Effect1]", "parameter3", 0x1F, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel3]_Effect1]", "parameter2", 0x21, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel3]_Effect1]", "parameter1", 0x23, 0xFFFF, this.parameterHandler);

        this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel4]_Effect1]", "parameter3", 0x31, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel4]_Effect1]", "parameter2", 0x33, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel4]_Effect1]", "parameter1", 0x35, 0xFFFF, this.parameterHandler);

        this.registerInputScaler(messageLong, "[Master]", "crossfader", 0x0B, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[Master]", "gain", 0x17, 0xFFFF, TraktorS3.Controller.prototype.masterGainHandler.bind(this));
        this.registerInputScaler(messageLong, "[Master]", "headMix", 0x1D, 0xFFFF, this.parameterHandler);
        this.registerInputScaler(messageLong, "[Master]", "headGain", 0x1B, 0xFFFF, this.parameterHandler);

        this.hid.registerInputPacket(messageLong);

        for (const ch in this.Channels) {
            const chanob = this.Channels[ch];
            engine.makeConnection(ch, "playposition",
                TraktorS3.Channel.prototype.playpositionChanged.bind(chanob));
            engine.makeConnection(ch, "track_loaded",
                TraktorS3.Channel.prototype.trackLoadedHandler.bind(chanob));
            engine.makeConnection(ch, "end_of_track",
                TraktorS3.Channel.prototype.endOfTrackHandler.bind(chanob));
        }
        // Set each InputReport to the bitwise inverted state first,
        // and than apply the non-inverted initial state.
        // This is done, because the common-hid-packet-parser only triggers
        // the callback functions in case of a delta to the previous data.
        for (let inputReportIdx = 0x01; inputReportIdx <= 0x02; ++inputReportIdx) {
            const reportData = new Uint8Array(controller.getInputReport(inputReportIdx));

            TraktorS3.incomingData([inputReportIdx, ...reportData.map(x => ~x)]);
            TraktorS3.incomingData([inputReportIdx, ...reportData]);
        }

        // The controller state may have overridden our preferred default values, so set them now.
        for (const ch in this.Channels) {
            TraktorS3.Channel.prototype.setDefaults(ch);
        }

        // NOTE: Soft takeovers must only be enabled after setting the initial
        //       value, or the above line won't have any effect
        for (let ch = 1; ch <= 4; ch++) {
            const group = "[Channel" + ch + "]";

            if (!TraktorS3.PitchSliderRelativeMode) {
                engine.softTakeover(group, "rate", true);
            }
            engine.softTakeover(group, "pitch_adjust", true);
            engine.softTakeover(group, "volume", true);
            engine.softTakeover(group, "pregain", true);
            engine.softTakeover("[QuickEffectRack1_" + group + "]", "super1", true);
        }

        for (let unit = 1; unit <= 4; unit++) {
            engine.softTakeover("[EffectRack1_EffectUnit" + unit + "]", "mix", true);

            for (let effect = 1; effect <= 4; effect++) {
                const group = "[EffectRack1_EffectUnit" + unit + "_Effect" + effect + "]";

                engine.softTakeover(group, "meta", true);
                for (let param = 1; param <= 4; param++) {
                    engine.softTakeover(group, "parameter" + param, true);
                }
            }
        }

        engine.softTakeover("[Microphone]", "volume", true);
        engine.softTakeover("[Microphone]", "pregain", true);

        engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", true);
        engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", true);
        engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", true);
        engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", true);
        engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", true);
        engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", true);
        engine.softTakeover("[EqualizerRack1_[Channel3]_Effect1]", "parameter1", true);
        engine.softTakeover("[EqualizerRack1_[Channel3]_Effect1]", "parameter2", true);
        engine.softTakeover("[EqualizerRack1_[Channel3]_Effect1]", "parameter3", true);
        engine.softTakeover("[EqualizerRack1_[Channel4]_Effect1]", "parameter1", true);
        engine.softTakeover("[EqualizerRack1_[Channel4]_Effect1]", "parameter2", true);
        engine.softTakeover("[EqualizerRack1_[Channel4]_Effect1]", "parameter3", true);

        // engine.softTakeover("[Master]", "crossfader", true);
        engine.softTakeover("[Master]", "gain", true);
        // engine.softTakeover("[Master]", "headMix", true);
        // engine.softTakeover("[Master]", "headGain", true);
        for (let i = 1; i <= 16; ++i) {
            engine.softTakeover("[Sampler" + i + "]", "pregain", true);
        }
    }

    registerInputJog(message, group, name, offset, bitmask, callback) {
        // Jog wheels have 4 byte input
        message.addControl(group, name, offset, "I", bitmask);
        message.setCallback(group, name, callback);
    }

    registerInputScaler(message, group, name, offset, bitmask, callback) {
        message.addControl(group, name, offset, "H", bitmask);
        message.setCallback(group, name, callback);
    }

    registerInputButton(message, group, name, offset, bitmask, callback) {
        message.addControl(group, name, offset, "B", bitmask);
        message.setCallback(group, name, callback);
    }

    parameterHandler(field) {
        const value = TraktorS3.normalize12BitValue(field.value);
        if (field.group === "[Channel4]" && this.channel4InputMode) {
            engine.setParameter("[Microphone]", field.name, value);
        } else {
            engine.setParameter(field.group, field.name, value);
        }
    }

    anyShiftPressed() {
        return this.Decks.deck1.shiftPressed || this.Decks.deck2.shiftPressed;
    }

    masterGainHandler(field) {
        // Only adjust if shift is held. This will still adjust the sound card
        // volume but it at least allows for control of Mixxx's master gain.
        if (this.anyShiftPressed()) {
            const value = TraktorS3.normalize12BitValue(field.value);
            engine.setParameter(field.group, field.name, value);
        }
    }

    headphoneHandler(field) {
        if (field.value === 0) {
            return;
        }
        if (field.group === "[Channel4]" && this.channel4InputMode) {
            script.toggleControl("[Microphone]", "pfl");
        } else {
            script.toggleControl(field.group, "pfl");
        }
    }

    deckSwitchHandler(field) {
        if (field.value === 0) {
            if (this.deckSwitchPressed === field.group) {
                this.deckSwitchPressed = "";
            }
            return;
        }

        if (this.deckSwitchPressed === "") {
            this.deckSwitchPressed = field.group;
        } else {
            // If a different deck switch is already pressed, do an instant
            // double and do not select the deck.
            const cloneFrom = this.Channels[this.deckSwitchPressed];
            const cloneFromNum = cloneFrom.parentDeck.deckNumber;
            engine.setValue(field.group, "CloneFromDeck", cloneFromNum);
            return;
        }

        const channel = this.Channels[field.group];
        const deck = channel.parentDeck;

        if (engine.isScratching(channel.groupNumber)) {
            engine.scratchDisable(channel.groupNumber);
        }

        deck.activateChannel(channel);
    }

    extModeHandler(field) {
        if (!field.value) {
            this.basicOutput(this.channel4InputMode, field.group, field.name);
            return;
        }
        if (this.anyShiftPressed()) {
            this.basicOutput(field.value, field.group, field.name);
            this.inputModeLine = !this.inputModeLine;
            this.setInputLineMode(this.inputModeLine);
            return;
        }
        this.channel4InputMode = !this.channel4InputMode;
        if (this.channel4InputMode) {
            engine.softTakeoverIgnoreNextValue("[Microphone]", "volume");
            engine.softTakeoverIgnoreNextValue("[Microphone]", "pregain");
        } else {
            engine.softTakeoverIgnoreNextValue("[Channel4]", "volume");
            engine.softTakeoverIgnoreNextValue("[Channel4]", "pregain");
        }
        this.lightDeck("[Channel4]");
        this.basicOutput(this.channel4InputMode, field.group, field.name);
    }

    registerOutputPackets() {
        const outputA = new HIDPacket("outputA", 0x80);
        const outputB = new HIDPacket("outputB", 0x81);

        for (const idx in this.Decks) {
            this.Decks[idx].registerOutputs(outputA, outputB);
        }

        outputA.addOutput("[Channel1]", "!deck_A", 0x0A, "B");
        outputA.addOutput("[Channel2]", "!deck_B", 0x23, "B");
        outputA.addOutput("[Channel3]", "!deck_C", 0x0B, "B");
        outputA.addOutput("[Channel4]", "!deck_D", 0x24, "B");

        outputA.addOutput("[Channel1]", "pfl", 0x39, "B");
        outputA.addOutput("[Channel2]", "pfl", 0x3A, "B");
        outputA.addOutput("[Channel3]", "pfl", 0x38, "B");
        outputA.addOutput("[Channel4]", "pfl", 0x3B, "B");

        outputA.addOutput("[ChannelX]", "!fxButton1", 0x3C, "B");
        outputA.addOutput("[ChannelX]", "!fxButton2", 0x3D, "B");
        outputA.addOutput("[ChannelX]", "!fxButton3", 0x3E, "B");
        outputA.addOutput("[ChannelX]", "!fxButton4", 0x3F, "B");
        outputA.addOutput("[ChannelX]", "!fxButton0", 0x40, "B");

        outputA.addOutput("[Channel3]", "!fxEnabled", 0x34, "B");
        outputA.addOutput("[Channel1]", "!fxEnabled", 0x35, "B");
        outputA.addOutput("[Channel2]", "!fxEnabled", 0x36, "B");
        outputA.addOutput("[Channel4]", "!fxEnabled", 0x37, "B");

        outputA.addOutput("[Master]", "!extButton", 0x33, "B");

        this.hid.registerOutputPacket(outputA);

        const VuOffsets = {
            "[Channel3]": 0x01,
            "[Channel1]": 0x10,
            "[Channel2]": 0x1F,
            "[Channel4]": 0x2E
        };
        for (const ch in VuOffsets) {
            for (let i = 0; i < 14; i++) {
                outputB.addOutput(ch, "!" + "vu_meter" + i, VuOffsets[ch] + i, "B");
            }
        }

        const MasterVuOffsets = {
            "vu_meter_left": 0x3D,
            "vu_meter_right": 0x46
        };
        for (let i = 0; i < 8; i++) {
            outputB.addOutput("[Main]", "!" + "vu_meter_left" + i, MasterVuOffsets.vu_meter_left + i, "B");
            outputB.addOutput("[Main]", "!" + "vu_meter_right" + i, MasterVuOffsets.vu_meter_right + i, "B");
        }

        outputB.addOutput("[Main]", "peak_indicator_left", 0x45, "B");
        outputB.addOutput("[Main]", "peak_indicator_right", 0x4E, "B");

        outputB.addOutput("[Channel3]", "peak_indicator", 0x0F, "B");
        outputB.addOutput("[Channel1]", "peak_indicator", 0x1E, "B");
        outputB.addOutput("[Channel2]", "peak_indicator", 0x2D, "B");
        outputB.addOutput("[Channel4]", "peak_indicator", 0x3C, "B");

        this.hid.registerOutputPacket(outputB);

        for (const idx in this.Decks) {
            this.Decks[idx].linkOutputs();
        }

        for (const idx in this.Channels) {
            this.Channels[idx].linkOutputs();
        }

        engine.makeConnection("[Microphone]", "pfl", this.pflOutput.bind(this));

        engine.makeConnection("[Skin]", "show_maximized_library", TraktorS3.Controller.prototype.maximizeLibraryOutput.bind(this));

        // Master VuMeters
        this.masterVuMeter.vu_meter_left.connection = engine.makeConnection("[Main]", "vu_meter_left", TraktorS3.Controller.prototype.masterVuMeterHandler.bind(this));
        this.masterVuMeter.vu_meter_right.connection = engine.makeConnection("[Main]", "vu_meter_right", TraktorS3.Controller.prototype.masterVuMeterHandler.bind(this));
        this.linkChannelOutput("[Main]", "peak_indicator_left", TraktorS3.Controller.prototype.peakOutput.bind(this));
        this.linkChannelOutput("[Main]", "peak_indicator_right", TraktorS3.Controller.prototype.peakOutput.bind(this));
        this.guiTickConnection = engine.makeConnection("[App]", "gui_tick_50ms_period_s", TraktorS3.Controller.prototype.guiTickHandler.bind(this));

        // Sampler callbacks
        const samNum = TraktorS3.SixteenSamplers ? 16 : 8;
        if (engine.getValue("[App]", "num_samplers") < samNum) {
            engine.setValue("[App]", "num_samplers", samNum);
        }
        for (let i = 1; i <= samNum; ++i) {
            this.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "track_loaded", TraktorS3.Controller.prototype.samplesOutput.bind(this)));
            this.samplerCallbacks.push(engine.makeConnection("[Sampler" + i + "]", "play_indicator", TraktorS3.Controller.prototype.samplesOutput.bind(this)));
        }
    }

    linkChannelOutput(group, name, callback) {
        this.hid.linkOutput(group, name, group, name, callback);
    }

    pflOutput(value, group, key) {
        if (group === "[Microphone]" && this.channel4InputMode) {
            this.basicOutput(value, "[Channel4]", key);
            return;
        }
        if (group === "[Channel4]" && !this.channel4InputMode) {
            this.basicOutput(value, group, key);
            return;
        }
        if (group.match(/^\[Channel[123]\]$/)) {
            this.basicOutput(value, group, key);
        }
        // Unhandled case, ignore.
    }

    maximizeLibraryOutput(value, _group, _key) {
        this.Decks.deck1.colorOutput(value, "!MaximizeLibrary");
        this.Decks.deck2.colorOutput(value, "!MaximizeLibrary");
    }

    // Output drives lights that only have one color.
    basicOutput(value, group, key) {
        let ledValue = value;
        if (value === 0 || value === false) {
            // Off value
            ledValue = 0x04;
        } else if (value === 1 || value === true) {
            // On value
            ledValue = 0xFF;
        }

        this.hid.setOutput(group, key, ledValue, !this.batchingOutputs);
    }

    peakOutput(value, group, key) {
        let ledValue = 0x00;
        if (value) {
            ledValue = 0x7E;
        }

        this.hid.setOutput(group, key, ledValue, !this.batchingOutputs);
    }

    masterVuMeterHandler(value, _group, key) {
        this.masterVuMeter[key].updated = true;
        this.masterVuMeter[key].value = value;
    }

    vuMeterOutput(value, group, key, segments) {
        // This handler is called a lot so it should be as fast as possible.
        const scaledValue = value * segments;
        const fullIllumCount = Math.floor(scaledValue);

        // Figure out how much the partially-illuminated segment is illuminated.
        const partialIllum = (scaledValue - fullIllumCount) * 0x7F;

        for (let i = 0; i < segments; i++) {
            const segmentKey = "!" + key + i;
            if (i < fullIllumCount) {
                // Don't update lights until they're all done, so the last term is false.
                this.hid.setOutput(group, segmentKey, 0x7F, false);
            } else if (i === fullIllumCount) {
                this.hid.setOutput(group, segmentKey, partialIllum, false);
            } else {
                this.hid.setOutput(group, segmentKey, 0x00, false);
            }
        }
        if (!this.batchingOutputs) {
            this.hid.OutputPackets.outputB.send();
        }
    }

    resolveSampler(group) {
        if (group === undefined) {
            return undefined;
        }

        const result = group.match(script.samplerRegEx);

        if (result === null) {
            return undefined;
        }

        // Return sampler as number if we can
        const strResult = result[1];
        if (strResult === undefined) {
            return undefined;
        }
        return parseInt(strResult);
    }

    samplesOutput(value, group, key) {
        // Sampler 1-8 -> Channel1
        // Samples 9-16 -> Channel2
        const sampler = this.resolveSampler(group);
        let deck = this.Decks.deck1;
        let num = sampler;
        if (sampler === undefined) {
            return;
        } else if (sampler > 8 && sampler < 17) {
            if (!TraktorS3.SixteenSamplers) {
                // These samplers are ignored
                return;
            }
            deck = this.Decks.deck2;
            num = sampler - 8;
        }

        // If we are in samples modes light corresponding LED
        if (deck.padModeState !== 1) {
            return;
        }
        if (key === "play_indicator" && engine.getValue(group, "track_loaded")) {
            if (value) {
                // Green light on play
                this.hid.setOutput("deck1", "!pad_" + num, this.hid.LEDColors.GREEN + TraktorS3.LEDBrightValue, !this.batchingOutputs);
                // Also light deck2 samplers in 8-sampler mode.
                if (!TraktorS3.SixteenSamplers && this.Decks.deck2.padModeState === 1) {
                    this.hid.setOutput("deck2", "!pad_" + num, this.hid.LEDColors.GREEN + TraktorS3.LEDBrightValue, !this.batchingOutputs);
                }
            } else {
                // Reset LED to base color
                deck.colorOutput(1, "!pad_" + num);
                if (!TraktorS3.SixteenSamplers && this.Decks.deck2.padModeState === 1) {
                    this.Decks.deck2.colorOutput(1, "!pad_" + num);
                }
            }
        } else if (key === "track_loaded") {
            deck.colorOutput(value, "!pad_" + num);
            if (!TraktorS3.SixteenSamplers && this.Decks.deck2.padModeState === 1) {
                this.Decks.deck2.colorOutput(value, "!pad_" + num);
            }
        }
    }

    lightGroup(packet, outputGroupName, coGroupName) {
        const groupOb = packet.groups[outputGroupName];
        for (const fieldName in groupOb) {
            const field = groupOb[fieldName];
            if (field.name[0] === "!") {
                continue;
            }
            if (field.mapped_callback !== undefined) {
                const value = engine.getValue(coGroupName, field.name);
                field.mapped_callback(value, coGroupName, field.name);
            }
            // No callback, no light!
        }
    }

    lightDeck(group, sendPackets) {
        if (sendPackets === undefined) {
            sendPackets = true;
        }
        // Freeze the lights while we do this update so we don't spam HID.
        this.batchingOutputs = true;
        for (const packetName in this.hid.OutputPackets) {
            const packet = this.hid.OutputPackets[packetName];
            let deckGroupName = "deck1";
            if (group === "[Channel2]" || group === "[Channel4]") {
                deckGroupName = "deck2";
            }

            const deck = this.Decks[deckGroupName];

            this.lightGroup(packet, deckGroupName, group);
            this.lightGroup(packet, group, group);

            deck.lightPads();

            // These lights are different because either they aren't associated
            // with a CO, or there are two buttons that point to the same CO.
            deck.basicOutput(0, "!shift");
            deck.colorOutput(0, "!PreviewTrack");
            deck.colorOutput(0, "!LibraryFocus");
            deck.colorOutput(0, "!MaximizeLibrary");
            deck.colorOutput(deck.jogToggled, "!jogButton");
            if (group === "[Channel4]") {
                this.basicOutput(0, "[Master]", "!extButton");
            }
        }
        // this.lightFx();
        // Selected deck lights
        if (group === "[Channel1]") {
            this.hid.setOutput("[Channel1]", "!deck_A", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel1]"]] + TraktorS3.LEDBrightValue, false);
            this.hid.setOutput("[Channel3]", "!deck_C", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel3]"]] + TraktorS3.LEDDimValue, false);
        } else if (group === "[Channel2]") {
            this.hid.setOutput("[Channel2]", "!deck_B", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel2]"]] + TraktorS3.LEDBrightValue, false);
            this.hid.setOutput("[Channel4]", "!deck_D", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel4]"]] + TraktorS3.LEDDimValue, false);
        } else if (group === "[Channel3]") {
            this.hid.setOutput("[Channel3]", "!deck_C", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel3]"]] + TraktorS3.LEDBrightValue, false);
            this.hid.setOutput("[Channel1]", "!deck_A", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel1]"]] + TraktorS3.LEDDimValue, false);
        } else if (group === "[Channel4]") {
            this.hid.setOutput("[Channel4]", "!deck_D", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel4]"]] + TraktorS3.LEDBrightValue, false);
            this.hid.setOutput("[Channel2]", "!deck_B", this.hid.LEDColors[TraktorS3.ChannelColors["[Channel2]"]] + TraktorS3.LEDDimValue, false);
        }

        this.batchingOutputs = false;
        // And now send them all.
        if (sendPackets) {
            for (const packetName in this.hid.OutputPackets) {
                this.hid.OutputPackets[packetName].send();
            }
        }
    }

    // Render wheel positions, channel VU meters, and master vu meters
    guiTickHandler() {
        this.batchingOutputs = true;
        let gotUpdate = false;
        gotUpdate |= this.Channels[this.Decks.deck1.activeChannel].lightWheelPosition();
        gotUpdate |= this.Channels[this.Decks.deck2.activeChannel].lightWheelPosition();

        for (const vu in this.masterVuMeter) {
            if (this.masterVuMeter[vu].updated) {
                this.vuMeterOutput(this.masterVuMeter[vu].value, "[Main]", vu, 8);
                this.masterVuMeter[vu].updated = false;
                gotUpdate = true;
            }
        }
        for (let ch = 1; ch <= 4; ch++) {
            const chan = this.Channels["[Channel" + ch + "]"];
            if (chan.vuMeterUpdated) {
                this.vuMeterOutput(chan.vuMeterValue, chan.group, "vu_meter", 14);
                chan.vuMeterUpdated = false;
                gotUpdate = true;
            }
        }

        this.batchingOutputs = false;

        if (gotUpdate) {
            for (const packetName in this.hid.OutputPackets) {
                this.hid.OutputPackets[packetName].send();
            }
        }
    }

    // A special packet sent to the controller switches between mic and line
    // input modes. if lineMode is true, sets input to line. Otherwise, mic.
    setInputLineMode(lineMode) {
        const packet = Array();
        packet.length = 33;
        packet[0] = 0x20;
        if (!lineMode) {
            packet[1] = 0x08;
        }
        controller.send(packet, packet.length, 0xF4);
    }
};

//// Deck Objects ////
// Decks are the physical controllers on either side of the controller.  Each
// Deck can control 2 channels.
TraktorS3.Deck = class {
    constructor(controller, deckNumber, group) {
        this.controller = controller;
        this.deckNumber = deckNumber;
        this.group = group;
        this.activeChannel = "[Channel" + deckNumber + "]";
        this.activeChannelNumber = deckNumber;
        // When true, touching the wheel enables scratch mode. When off,
        // touching the wheel has no special effect
        this.jogToggled = TraktorS3.JogDefaultOn;
        this.shiftPressed = false;

        // State for pitch slider relative mode
        this.pitchSliderLastValue = -1;
        this.keylockPressed = false;
        this.keyAdjusted = false;

        // Various states
        this.syncPressedTimer = 0;
        this.previewPressed = false;
        // padModeState 0 is hotcues, 1 is samplers
        this.padModeState = 0;

        // Jog wheel state
        this.lastTickVal = 0;
        this.lastTickTime = 0;
        this.lastTickWallClock = 0;

        // Knob encoder states (hold values between 0x0 and 0xF) Rotate to the
        // right is +1 and to the left is means -1
        this.browseKnobEncoderState = 0;
        this.loopKnobEncoderState = 0;
        this.moveKnobEncoderState = 0;
    }

    activateChannel(channel) {
        if (channel.parentDeck !== this) {
            HIDDebug("Programming ERROR: tried to activate a channel with a deck that is not its parent");
            return;
        }
        this.activeChannel = channel.group;
        this.activeChannelNumber = channel.groupNumber;
        engine.softTakeoverIgnoreNextValue(this.activeChannel, "rate");
        this.controller.lightDeck(this.activeChannel);
    }

    // defineButton allows us to configure either the right deck or the left
    // deck, depending on which is appropriate. This avoids extra logic in the
    // function where we define all the magic numbers. We use a similar approach
    // in the other define funcs.
    defineButton(msg, name, deckOffset, deckBitmask, deck2Offset, deck2Bitmask, fn) {
        if (this.deckNumber === 2) {
            deckOffset = deck2Offset;
            deckBitmask = deck2Bitmask;
        }
        this.controller.registerInputButton(msg, this.group, name, deckOffset, deckBitmask, fn.bind(this));
    }

    defineJog(message, name, deckOffset, deck2Offset, callback) {
        if (this.deckNumber === 2) {
            deckOffset = deck2Offset;
        }
        // Jog wheels have four byte input: 1 byte for distance ticks, and 3 bytes for a timecode.
        message.addControl(this.group, name, deckOffset, "I", 0xFFFFFFFF);
        message.setCallback(this.group, name, callback.bind(this));
    }

    // defineScaler configures ranged controls like knobs and sliders.
    defineScaler(msg, name, deckOffset, deckBitmask, deck2Offset, deck2Bitmask, fn) {
        if (this.deckNumber === 2) {
            deckOffset = deck2Offset;
            deckBitmask = deck2Bitmask;
        }
        this.controller.registerInputScaler(msg, this.group, name, deckOffset, deckBitmask, fn.bind(this));
    }

    registerInputs(messageShort, messageLong) {
        const deckFn = TraktorS3.Deck.prototype;
        this.defineButton(messageShort, "!play", 0x03, 0x01, 0x06, 0x02, deckFn.playHandler);
        this.defineButton(messageShort, "!cue_default", 0x02, 0x80, 0x06, 0x01, deckFn.cueHandler);
        this.defineButton(messageShort, "!shift", 0x01, 0x01, 0x04, 0x02, deckFn.shiftHandler);
        this.defineButton(messageShort, "!sync", 0x02, 0x08, 0x05, 0x10, deckFn.syncHandler);
        this.defineButton(messageShort, "!keylock", 0x02, 0x10, 0x05, 0x20, deckFn.keylockHandler);
        this.defineButton(messageShort, "!hotcues", 0x02, 0x20, 0x05, 0x40, deckFn.padModeHandler);
        this.defineButton(messageShort, "!samples", 0x02, 0x40, 0x05, 0x80, deckFn.padModeHandler);

        this.defineButton(messageShort, "!pad_1", 0x03, 0x02, 0x06, 0x04, deckFn.numberButtonHandler);
        this.defineButton(messageShort, "!pad_2", 0x03, 0x04, 0x06, 0x08, deckFn.numberButtonHandler);
        this.defineButton(messageShort, "!pad_3", 0x03, 0x08, 0x06, 0x10, deckFn.numberButtonHandler);
        this.defineButton(messageShort, "!pad_4", 0x03, 0x10, 0x06, 0x20, deckFn.numberButtonHandler);
        this.defineButton(messageShort, "!pad_5", 0x03, 0x20, 0x06, 0x40, deckFn.numberButtonHandler);
        this.defineButton(messageShort, "!pad_6", 0x03, 0x40, 0x06, 0x80, deckFn.numberButtonHandler);
        this.defineButton(messageShort, "!pad_7", 0x03, 0x80, 0x07, 0x01, deckFn.numberButtonHandler);
        this.defineButton(messageShort, "!pad_8", 0x04, 0x01, 0x07, 0x02, deckFn.numberButtonHandler);

        // TODO: bind touch: 0x09/0x40, 0x0A/0x02
        this.defineButton(messageShort, "!SelectTrack", 0x0B, 0x0F, 0x0C, 0xF0, deckFn.selectTrackHandler);
        this.defineButton(messageShort, "!LoadSelectedTrack", 0x09, 0x01, 0x09, 0x08, deckFn.loadTrackHandler);
        this.defineButton(messageShort, "!PreviewTrack", 0x01, 0x08, 0x04, 0x10, deckFn.previewTrackHandler);
        // There is no control object to mark / unmark a track as played.
        // this.defineButton(messageShort, "!SetPlayed", 0x01, 0x10, 0x04, 0x20,
        // deckFn.SetPlayedHandler);
        this.defineButton(messageShort, "!LibraryFocus", 0x01, 0x20, 0x04, 0x40, deckFn.LibraryFocusHandler);
        this.defineButton(messageShort, "!MaximizeLibrary", 0x01, 0x40, 0x04, 0x80, deckFn.MaximizeLibraryHandler);

        // Loop control
        // TODO: bind touch detections: 0x0A/0x01, 0x0A/0x08
        this.defineButton(messageShort, "!SelectLoop", 0x0C, 0x0F, 0x0D, 0xF0, deckFn.selectLoopHandler);
        this.defineButton(messageShort, "!ActivateLoop", 0x09, 0x04, 0x09, 0x20, deckFn.activateLoopHandler);

        // Rev / Flux / Grid / Jog
        this.defineButton(messageShort, "!reverse", 0x01, 0x04, 0x04, 0x08, deckFn.reverseHandler);
        this.defineButton(messageShort, "!slip_enabled", 0x01, 0x02, 0x04, 0x04, deckFn.fluxHandler);
        this.defineButton(messageShort, "quantize", 0x01, 0x80, 0x05, 0x01, deckFn.quantizeHandler);
        this.defineButton(messageShort, "!jogButton", 0x02, 0x01, 0x05, 0x02, deckFn.jogButtonHandler);

        // Beatjump
        // TODO: bind touch detections: 0x09/0x80, 0x0A/0x04
        this.defineButton(messageShort, "!SelectBeatjump", 0x0B, 0xF0, 0x0D, 0x0F, deckFn.selectBeatjumpHandler);
        this.defineButton(messageShort, "!ActivateBeatjump", 0x09, 0x02, 0x09, 0x10, deckFn.activateBeatjumpHandler);

        // Jog wheels
        this.defineButton(messageShort, "!jog_touch", 0x0A, 0x10, 0x0A, 0x20, deckFn.jogTouchHandler);
        this.defineJog(messageShort, "!jog", 0x0E, 0x12, deckFn.jogHandler);

        this.defineScaler(messageLong, "rate", 0x01, 0xFFFF, 0x0D, 0xFFFF, deckFn.pitchSliderHandler);
    }

    shiftHandler(field) {
        // Mixxx only knows about one shift value, but this controller has two
        // shift buttons. This control object could get confused if both
        // physical buttons are pushed at the same time.
        engine.setValue("[Controls]", "touch_shift", field.value);
        this.shiftPressed = field.value;
        if (field.value) {
            engine.softTakeoverIgnoreNextValue("[Master]", "gain");
        }
        this.controller.basicOutput(field.value, field.group, "!shift");
    }

    playHandler(field) {
        if (this.shiftPressed) {
            engine.setValue(this.activeChannel, "start_stop", field.value);
        } else if (field.value === 1) {
            script.toggleControl(this.activeChannel, "play");
        }
    }

    cueHandler(field) {
        if (this.shiftPressed) {
            engine.setValue(this.activeChannel, "cue_gotoandstop", field.value);
        } else {
            engine.setValue(this.activeChannel, "cue_default", field.value);
        }
    }

    syncHandler(field) {
        if (this.shiftPressed) {
            engine.setValue(this.activeChannel, "beatsync_phase", field.value);
            // Light LED while pressed
            this.colorOutput(field.value, "sync_enabled");
            return;
        }

        // Unshifted
        if (field.value) {
            // We have to reimplement push-to-lock because it's only defined in
            // the midi code in Mixxx.
            if (engine.getValue(this.activeChannel, "sync_enabled") === 0) {
                script.triggerControl(this.activeChannel, "beatsync");
                // Start timer to measure how long button is pressed
                this.syncPressedTimer = engine.beginTimer(300, () => {
                    engine.setValue(this.activeChannel, "sync_enabled", 1);
                    // Reset sync button timer state if active
                    if (this.syncPressedTimer !== 0) {
                        this.syncPressedTimer = 0;
                    }
                }, true);

                // Light corresponding LED when button is pressed
                this.colorOutput(1, "sync_enabled");
            } else {
                // Deactivate sync lock
                // LED is turned off by the callback handler for sync_enabled
                engine.setValue(this.activeChannel, "sync_enabled", 0);
            }
        } else if (this.syncPressedTimer !== 0) {
            // Timer still running -> stop it and unlight LED
            engine.stopTimer(this.syncPressedTimer);
            this.colorOutput(0, "sync_enabled");
        }
    }

    keylockHandler(field) {
        // shift + keylock resets pitch (in either mode).
        if (this.shiftPressed) {
            if (field.value) {
                engine.setValue(this.activeChannel, "pitch_adjust_set_default", 1);
            }
        } else if (TraktorS3.PitchSliderRelativeMode) {
            if (field.value) {
                // In relative mode on down-press, reset the values and note
                // that the button is pressed.
                this.keylockPressed = true;
                this.keyAdjusted = false;
            } else {
                // On release, note that the button is released, and if the key
                // *wasn't* adjusted, activate keylock.
                this.keylockPressed = false;
                if (!this.keyAdjusted) {
                    script.toggleControl(this.activeChannel, "keylock");
                }
            }
        } else if (field.value) {
            // In absolute mode, do a simple toggle on down-press.
            script.toggleControl(this.activeChannel, "keylock");
        }

        // Adjust the light on release depending on keylock status.  Down-press is always lit.
        if (!field.value) {
            const val = engine.getValue(this.activeChannel, "keylock");
            this.colorOutput(val, "keylock");
        } else {
            this.colorOutput(1, "keylock");
        }
    }

    // This handles when the mode buttons for the pads is pressed.
    padModeHandler(field) {
        if (field.value === 0) {
            return;
        }

        if (this.padModeState === 0 && field.name === "!samples") {
            // If we are in hotcues mode and samples mode is activated
            engine.setValue("[Samplers]", "show_samplers", 1);
            this.padModeState = 1;
        } else if (field.name === "!hotcues") {
            // If we are in samples mode and hotcues mode is activated
            this.padModeState = 0;
        }
        this.lightPads();
    }

    numberButtonHandler(field) {
        const padNumber = parseInt(field.name[field.name.length - 1]);

        // Hotcues mode
        if (this.padModeState === 0) {
            const action = this.shiftPressed ? "_clear" : "_activate";
            engine.setValue(this.activeChannel, "hotcue_" + padNumber + action, field.value);
            return;
        }

        // Samples mode
        let sampler = padNumber;
        if (field.group === "deck2" && TraktorS3.SixteenSamplers) {
            sampler += 8;
        }

        const playing = engine.getValue("[Sampler" + sampler + "]", "play");
        if (this.shiftPressed) {
            const action = playing ? "cue_default" : "eject";
            engine.setValue("[Sampler" + sampler + "]", action, field.value);
            return;
        }
        const loaded = engine.getValue("[Sampler" + sampler + "]", "track_loaded");
        if (loaded) {
            if (TraktorS3.SamplerModePressAndHold) {
                const action = field.value ? "cue_gotoandplay" : "stop";
                engine.setValue("[Sampler" + sampler + "]", action, 1);
            } else {
                if (field.value) {
                    const action = playing ? "stop" : "cue_gotoandplay";
                    engine.setValue("[Sampler" + sampler + "]", action, 1);
                }
            }
            return;
        }
        // Play on an empty sampler loads that track into that sampler
        engine.setValue("[Sampler" + sampler + "]", "LoadSelectedTrack", field.value);
    }

    selectTrackHandler(field) {
        let delta = 1;
        if ((field.value + 1) % 16 === this.browseKnobEncoderState) {
            delta = -1;
        }
        this.browseKnobEncoderState = field.value;

        // When preview is held, rotating the library encoder scrolls through the previewing track.
        if (this.previewPressed) {
            let playPosition = engine.getValue("[PreviewDeck1]", "playposition");
            if (delta > 0) {
                playPosition += 0.0125;
            } else {
                playPosition -= 0.0125;
            }
            engine.setValue("[PreviewDeck1]", "playposition", playPosition);
            return;
        }

        if (this.shiftPressed) {
            engine.setValue("[Library]", "MoveHorizontal", delta);
        } else {
            engine.setValue("[Library]", "MoveVertical", delta);
        }
    }

    loadTrackHandler(field) {
        // If the library is selected, load the track based on which encoder was
        // pressed. Otherwise just do whatever the default action is for the
        // focused menu or widget.
        if (engine.getValue("[Library]", "focused_widget") === 3) {
            if (this.shiftPressed) {
                engine.setValue(this.activeChannel, "eject", field.value);
            } else {
                engine.setValue(this.activeChannel, "LoadSelectedTrack", field.value);
            }
        } else {
            engine.setValue("[Library]", "GoToItem", field.value);
        }
    }

    previewTrackHandler(field) {
        this.colorOutput(field.value, "!PreviewTrack");
        if (field.value === 1) {
            this.previewPressed = true;
            engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 1);
        } else {
            this.previewPressed = false;
            engine.setValue("[PreviewDeck1]", "play", 0);
        }
    }

    LibraryFocusHandler(field) {
        this.colorOutput(field.value, "!LibraryFocus");
        if (field.value === 0) {
            return;
        }

        engine.setValue("[Library]", "MoveFocus", field.value);
    }

    MaximizeLibraryHandler(field) {
        if (field.value === 0) {
            return;
        }

        script.toggleControl("[Skin]", "show_maximized_library");
    }

    selectLoopHandler(field) {
        let delta = 1;
        if ((field.value + 1) % 16 === this.loopKnobEncoderState) {
            delta = -1;
        }

        if (this.shiftPressed) {
            const beatjumpSize = engine.getValue(this.activeChannel, "beatjump_size");
            if (delta > 0) {
                script.triggerControl(this.activeChannel, "loop_move_" + beatjumpSize + "_forward");
            } else {
                script.triggerControl(this.activeChannel, "loop_move_" + beatjumpSize + "_backward");
            }
        } else {
            if (delta > 0) {
                script.triggerControl(this.activeChannel, "loop_double");
            } else {
                script.triggerControl(this.activeChannel, "loop_halve");
            }
        }

        this.loopKnobEncoderState = field.value;
    }

    activateLoopHandler(field) {
        if (field.value === 0) {
            return;
        }
        const isLoopActive = engine.getValue(this.activeChannel, "loop_enabled");

        if (this.shiftPressed) {
            engine.setValue(this.activeChannel, "reloop_toggle", field.value);
        } else {
            if (isLoopActive) {
                engine.setValue(this.activeChannel, "reloop_toggle", field.value);
            } else {
                engine.setValue(this.activeChannel, "beatloop_activate", field.value);
            }
        }
    }

    selectBeatjumpHandler(field) {
        let delta = 1;
        if ((field.value + 1) % 16 === this.moveKnobEncoderState) {
            delta = -1;
        }

        if (this.shiftPressed) {
            const beatjumpSize = engine.getValue(this.activeChannel, "beatjump_size");
            if (delta > 0) {
                engine.setValue(this.activeChannel, "beatjump_size", beatjumpSize * 2);
            } else {
                engine.setValue(this.activeChannel, "beatjump_size", beatjumpSize / 2);
            }
        } else {
            if (delta > 0) {
                script.triggerControl(this.activeChannel, "beatjump_forward");
            } else {
                script.triggerControl(this.activeChannel, "beatjump_backward");
            }
        }

        this.moveKnobEncoderState = field.value;
    }

    activateBeatjumpHandler(field) {
        if (this.shiftPressed) {
            engine.setValue(this.activeChannel, "reloop_andstop", field.value);
        } else {
            engine.setValue(this.activeChannel, "beatlooproll_activate", field.value);
        }
    }

    reverseHandler(field) {
        // this.basicOutput(field.value, "reverse");
        if (this.shiftPressed) {
            engine.setValue(this.activeChannel, "reverse", field.value);
        } else {
            engine.setValue(this.activeChannel, "reverseroll", field.value);
        }
    }

    fluxHandler(field) {
        if (field.value === 0) {
            return;
        }
        script.toggleControl(this.activeChannel, "slip_enabled");
    }

    quantizeHandler(field) {
        if (field.value === 0) {
            return;
        }
        if (this.shiftPressed) {
            engine.setValue(this.activeChannel, "beats_translate_curpos", field.value);
        } else {
            script.toggleControl(this.activeChannel, "quantize");
        }
    }

    jogButtonHandler(field) {
        if (field.value === 0) {
            return;
        }
        this.jogToggled = !this.jogToggled;
        this.colorOutput(this.jogToggled, "!jogButton");
    }

    jogTouchHandler(field) {
        if (!this.jogToggled) {
            return;
        }

        if (field.value !== 0) {
            engine.scratchEnable(
                this.activeChannelNumber,
                768,
                33.33334 / TraktorS3.JogSpeedMultiplier,
                TraktorS3.Alpha,
                TraktorS3.Beta
            );
        } else {
            engine.scratchDisable(this.activeChannelNumber);

            // If shift is pressed, reset right away.
            if (this.shiftPressed) {
                engine.setValue(this.activeChannel, "scratch2", 0.0);
                this.playIndicatorHandler(0, this.activeChannel);
            }
        }
    }

    jogHandler(field) {
        const deltas = this.wheelDeltas(field.value);

        // If shift button is held, do a simple seek.
        if (this.shiftPressed) {
            let playPosition = engine.getValue(this.activeChannel, "playposition");
            playPosition += deltas[0] / 2048.0;
            playPosition = Math.max(Math.min(playPosition, 1.0), 0.0);
            engine.setValue(this.activeChannel, "playposition", playPosition);
            return;
        }
        const tickDelta = deltas[0];
        const timeDelta = deltas[1];

        if (engine.isScratching(this.activeChannelNumber)) {
            engine.scratchTick(this.activeChannelNumber, tickDelta);
        } else {
            // The scratch rate is the ratio of the wheel's speed to "regular"
            // speed, which we're going to call 33.33 RPM. It's 768 ticks for a
            // circle, and 400000 ticks per second, and 33.33 RPM is 1.8 seconds
            // per rotation, so the standard speed is 768 / (400000 * 1.8)
            const thirtyThree = 768 / 720000;

            // Our actual speed is tickDelta / timeDelta. Take the ratio of those to
            // get the rate ratio.
            const velocity = (tickDelta / timeDelta) / thirtyThree;

            engine.setValue(this.activeChannel, "jog", velocity * TraktorS3.JogSpeedMultiplier);
        }
    }

    wheelDeltas(value) {
        // When the wheel is touched, 1 byte measures distance ticks, the other
        // three represent a timer value. We can use the amount of time required for
        // the number of ticks to elapse to get a velocity.
        const tickval = value & 0xFF;
        let timeval = value >>> 8;

        const prevTick = this.lastTickVal;
        const prevTime = this.lastTickTime;
        const prevTickWallClock = this.lastTickWallClock;
        this.lastTickVal = tickval;
        this.lastTickTime = timeval;
        this.lastTickWallClock = Date.now();

        // The user hasn't touched the jog wheel for a long time, so the
        // internal timer may have looped around more than once. We have nothing
        // to go by so return 0, 1 (1 to prevent divide by zero).
        if (this.lastTickWallClock - prevTickWallClock > 20000) {
            return [0, 1];
        }

        if (prevTime > timeval) {
            // We looped around.  Adjust current time so that subtraction works.
            timeval += 0x1000000;
        }
        // Clamp Suspiciously low numbers. Even flicking the wheel as fast as I
        // can, I can't get it lower than this value. Lower values (esp below
        // zero) cause huge jumps / errors in playback.
        const timeDelta = Math.max(timeval - prevTime, 500);
        let tickDelta = 0;

        // Very generous 8bit loop-around detection.
        if (prevTick >= 200 && tickval <= 100) {
            tickDelta = tickval + 256 - prevTick;
        } else if (prevTick <= 100 && tickval >= 200) {
            tickDelta = tickval - prevTick - 256;
        } else {
            tickDelta = tickval - prevTick;
        }

        return [tickDelta, timeDelta];
    }

    pitchSliderHandler(field) {
        // Adapt HID value to rate control range.
        const value = -1.0 + (TraktorS3.normalize12BitValue(field.value) * 2.0);
        if (TraktorS3.PitchSliderRelativeMode) {
            if (this.pitchSliderLastValue === -1) {
                this.pitchSliderLastValue = value;
            } else {
                // If shift is pressed, don't update any values.
                if (this.shiftPressed) {
                    this.pitchSliderLastValue = value;
                    return;
                }

                let relVal;
                if (this.keylockPressed) {
                    relVal = 1.0 - engine.getValue(this.activeChannel, "pitch_adjust");
                } else {
                    relVal = engine.getValue(this.activeChannel, "rate");
                }
                // This can result in values outside -1 to 1, but that is valid
                // for the rate control. This means the entire swing of the rate
                // slider can be outside the range of the widget, but that's ok
                // because the slider still works.
                relVal += value - this.pitchSliderLastValue;
                this.pitchSliderLastValue = value;

                if (this.keylockPressed) {
                    // To match the pitch change from adjusting the rate, flip
                    // the pitch adjustment.
                    engine.setValue(this.activeChannel, "pitch_adjust", 1.0 - relVal);
                    this.keyAdjusted = true;
                } else {
                    engine.setValue(this.activeChannel, "rate", relVal);
                }
            }
            return;
        }

        if (this.shiftPressed) {
            // To match the pitch change from adjusting the rate, flip the pitch
            // adjustment.
            engine.setValue(this.activeChannel, "pitch_adjust", 1.0 - value);
        } else {
            engine.setValue(this.activeChannel, "rate", value);
        }
    }

    //// Deck Outputs ////
    defineOutput(packet, name, offsetA, offsetB) {
        switch (this.deckNumber) {
        case 1:
            packet.addOutput(this.group, name, offsetA, "B");
            break;
        case 2:
            packet.addOutput(this.group, name, offsetB, "B");
            break;
        }
    }

    registerOutputs(outputA, _outputB) {
        this.defineOutput(outputA, "!shift", 0x01, 0x1A);
        this.defineOutput(outputA, "slip_enabled", 0x02, 0x1B);
        this.defineOutput(outputA, "reverse", 0x03, 0x1C);
        this.defineOutput(outputA, "!PreviewTrack", 0x04, 0x1D);
        this.defineOutput(outputA, "!LibraryFocus", 0x06, 0x1F);
        this.defineOutput(outputA, "!MaximizeLibrary", 0x07, 0x20);
        this.defineOutput(outputA, "quantize", 0x08, 0x21);
        this.defineOutput(outputA, "!jogButton", 0x09, 0x22);
        this.defineOutput(outputA, "sync_enabled", 0x0C, 0x25);
        this.defineOutput(outputA, "keylock", 0x0D, 0x26);
        this.defineOutput(outputA, "hotcues", 0x0E, 0x27);
        this.defineOutput(outputA, "samples", 0x0F, 0x28);
        this.defineOutput(outputA, "cue_indicator", 0x10, 0x29);
        this.defineOutput(outputA, "play_indicator", 0x11, 0x2A);

        this.defineOutput(outputA, "!pad_1", 0x12, 0x2B);
        this.defineOutput(outputA, "!pad_2", 0x13, 0x2C);
        this.defineOutput(outputA, "!pad_3", 0x14, 0x2D);
        this.defineOutput(outputA, "!pad_4", 0x15, 0x2E);
        this.defineOutput(outputA, "!pad_5", 0x16, 0x2F);
        this.defineOutput(outputA, "!pad_6", 0x17, 0x30);
        this.defineOutput(outputA, "!pad_7", 0x18, 0x31);
        this.defineOutput(outputA, "!pad_8", 0x19, 0x32);

        // this.defineOutput(outputA, "addTrack", 0x03, 0x2A);
        const wheelOffsets = [0x43, 0x4B];
        for (let i = 0; i < 8; i++) {
            this.defineOutput(outputA, "!" + "wheel" + i, wheelOffsets[0] + i, wheelOffsets[1] + i);
        }
    }

    defineLink(key, callback) {
        switch (this.deckNumber) {
        case 1:
            this.controller.hid.linkOutput("deck1", key, "[Channel1]", key, callback);
            engine.makeConnection("[Channel3]", key, callback);
            break;
        case 2:
            this.controller.hid.linkOutput("deck2", key, "[Channel2]", key, callback);
            engine.makeConnection("[Channel4]", key, callback);
            break;
        }
    }

    linkOutputs() {
        const colorOutput = function(value, _group, key) {
            this.colorOutput(value, key);
        };

        const basicOutput = function(value, _group, key) {
            this.basicOutput(value, key);
        };

        this.defineLink("play_indicator", TraktorS3.Deck.prototype.playIndicatorHandler.bind(this));
        this.defineLink("cue_indicator", colorOutput.bind(this));
        this.defineLink("sync_enabled", colorOutput.bind(this));
        this.defineLink("keylock", colorOutput.bind(this));
        this.defineLink("slip_enabled", colorOutput.bind(this));
        this.defineLink("quantize", colorOutput.bind(this));
        this.defineLink("reverse", basicOutput.bind(this));
        this.defineLink("scratch2_enable", colorOutput.bind(this));
    }

    deckBaseColor() {
        return this.controller.hid.LEDColors[TraktorS3.ChannelColors[this.activeChannel]];
    }

    // basicOutput drives lights that only have one color.
    basicOutput(value, key) {
        // incoming value will be a channel, we have to resolve back to deck.
        let ledValue = 0x20;
        if (value === 1 || value === true) {
            // On value
            ledValue = 0x77;
        }
        this.controller.hid.setOutput(this.group, key, ledValue, !TraktorS3.batchingOutputs);
    }

    // colorOutput drives lights that have the palettized multicolor lights.
    colorOutput(value, key) {
        let ledValue = this.deckBaseColor();

        if (value === 1 || value === true) {
            ledValue += TraktorS3.LEDBrightValue;
        } else {
            ledValue += TraktorS3.LEDDimValue;
        }
        this.controller.hid.setOutput(this.group, key, ledValue, !this.controller.batchingOutputs);
    }

    playIndicatorHandler(value, group, _key) {
        // Also call regular handler
        this.basicOutput(value, "play_indicator");
        this.wheelOutputByValue(group, value);
    }

    colorForHotcue(num) {
        const colorCode = engine.getValue(this.activeChannel, "hotcue_" + num + "_color");
        return this.controller.colorMap.getValueForNearestColor(colorCode);
    }

    lightHotcue(number) {
        const loaded = engine.getValue(this.activeChannel, "hotcue_" + number + "_status");
        const active = engine.getValue(this.activeChannel, "hotcue_" + number + "_activate");
        let ledValue = this.controller.hid.LEDColors.WHITE;
        if (loaded) {
            ledValue = this.colorForHotcue(number);
            ledValue += TraktorS3.LEDDimValue;
        }
        if (active) {
            ledValue += TraktorS3.LEDBrightValue;
        } else {
            ledValue += TraktorS3.LEDDimValue;
        }
        this.controller.hid.setOutput(this.group, "!pad_" + number, ledValue, !TraktorS3.batchingOutputs);
    }

    lightPads() {
        // Samplers
        if (this.padModeState === 1) {
            this.colorOutput(0, "hotcues");
            this.colorOutput(1, "samples");
            for (let i = 1; i <= 8; i++) {
                let idx = i;
                if (this.group === "deck2" && TraktorS3.SixteenSamplers) {
                    idx += 8;
                }
                const loaded = engine.getValue("[Sampler" + idx + "]", "track_loaded");
                this.colorOutput(loaded, "!pad_" + i);
            }
        } else {
            this.colorOutput(1, "hotcues");
            this.colorOutput(0, "samples");
            for (let i = 1; i <= 8; ++i) {
                this.lightHotcue(i);
            }
        }
    }

    wheelOutputByValue(group, value) {
        if (group !== this.activeChannel) {
            return;
        }

        let ledValue = this.deckBaseColor();

        if (value === 1 || value === true) {
            ledValue += TraktorS3.LEDBrightValue;
        } else {
            ledValue = 0x00;
        }
        this.wheelOutput(group,
            [ledValue, ledValue, ledValue, ledValue, ledValue, ledValue, ledValue, ledValue]);
    }

    wheelOutput(group, valueArray) {
        if (group !== this.activeChannel) {
            return;
        }

        for (let i = 0; i < 8; i++) {
            this.controller.hid.setOutput(this.group, "!wheel" + i, valueArray[i], false);
        }
        if (!TraktorS3.batchingOutputs) {
            for (const packetName in this.controller.hid.OutputPackets) {
                this.controller.hid.OutputPackets[packetName].send();
            }
        }
    }
};

/////////////////////////
//// Channel Objects ////
////
//// Channels don't have much state, just the fx button state.
TraktorS3.Channel = class {
    constructor(controller, parentDeck, group) {
        this.controller = controller;
        this.parentDeck = parentDeck;
        this.group = group;
        // We need the channel number for the scratch controls
        this.groupNumber = Number(group.match(/\[Channel(\d+)\]/)[1]);
        this.fxEnabledState = false;

        this.trackDurationSec = 0;
        this.positionUpdated = false;
        this.curPosition = -1;
        this.endOfTrackTimer = 0;
        this.endOfTrack = false;
        this.endOfTrackBlinkState = 0;
        this.vuMeterUpdated = false;
        this.vuMeterValue = 0;

        this.vuConnection = {};
        this.clipConnection = {};
        this.hotcueCallbacks = [];

        // The visual order of the channels in Mixxx is 3, 1, 2, 4, but we want
        // the crossfader assignments array to match the visual layout
        const visualChannelIndex = {3: 0, 1: 1, 2: 2, 4: 3}[this.groupNumber];
        if (TraktorS3.DefaultCrossfaderAssignments[visualChannelIndex] !== null) {
            // This goes 0-2 for left, right, and center, but having the values
            // in this script's config be -1, 0, and 1 makes much more sense
            engine.setValue(group, "orientation", TraktorS3.DefaultCrossfaderAssignments[visualChannelIndex] + 1);
        }
    }

    setDefaults(group) {
        // The script by default doesn't change any of the deck's settings, but it's
        // useful to be able to initialize these settings to your preferences when
        // you turn on the controller
        if (TraktorS3.DefaultBeatJumpSize !== null) {
            engine.setValue(group, "beatjump_size", TraktorS3.DefaultBeatJumpSize);
        }
        if (TraktorS3.DefaultBeatLoopLength !== null) {
            engine.setValue(group, "beatloop_size", TraktorS3.DefaultBeatLoopLength);
        }
        if (TraktorS3.DefaultSyncEnabled !== null) {
            engine.setValue(group, "sync_enabled", TraktorS3.DefaultSyncEnabled);
        }
        if (TraktorS3.DefaultQuantizeEnabled !== null) {
            engine.setValue(group, "quantize", TraktorS3.DefaultQuantizeEnabled);
        }
        if (TraktorS3.DefaultKeylockEnabled !== null) {
            engine.setValue(group, "keylock", TraktorS3.DefaultKeylockEnabled);
        }
    }

    trackLoadedHandler() {
        const trackSamples = engine.getValue(this.group, "track_samples");
        if (trackSamples === 0) {
            this.trackDurationSec = 0;
            return;
        }
        const trackSampleRate = engine.getValue(this.group, "track_samplerate");
        // Assume stereo.
        this.trackDurationSec = trackSamples / 2.0 / trackSampleRate;
        this.parentDeck.lightPads();
    }

    endOfTrackHandler(value) {
        this.endOfTrack = value;
        if (!value) {
            if (this.endOfTrackTimer) {
                engine.stopTimer(this.endOfTrackTimer);
                this.endOfTrackTimer = 0;
            }
            return;
        }
        this.endOfTrackTimer = engine.beginTimer(400, () => {
            this.endOfTrackBlinkState = !this.endOfTrackBlinkState;
        }, false);
    }

    playpositionChanged(value) {
        if (this.parentDeck.activeChannel !== this.group) {
            return;
        }

        // How many segments away from the actual angle should we light?
        // (in both directions, so "2" will light up to four segments)
        if (this.trackDurationSec === 0) {
            const samples = engine.getValue(this.group, "track_loaded");
            if (samples > 0) {
                this.trackLoadedHandler();
            } else {
                // No track loaded, abort
                return;
            }
        }
        this.curPosition = value * this.trackDurationSec;
        this.positionUpdated = true;
    }

    vuMeterHandler(value) {
        this.vuMeterUpdated = true;
        this.vuMeterValue = value;
    }

    linkOutputs() {
        this.vuConnection = engine.makeConnection(this.group, "vu_meter", TraktorS3.Channel.prototype.vuMeterHandler.bind(this));
        this.clipConnection = engine.makeConnection(this.group, "peak_indicator", TraktorS3.Controller.prototype.peakOutput.bind(this.controller));
        this.controller.linkChannelOutput(this.group, "pfl", TraktorS3.Controller.prototype.pflOutput.bind(this.controller));
        for (let j = 1; j <= 8; j++) {
            this.hotcueCallbacks.push(engine.makeConnection(this.group, "hotcue_" + j + "_status",
                TraktorS3.Channel.prototype.hotcuesOutput.bind(this)));
            this.hotcueCallbacks.push(engine.makeConnection(this.group, "hotcue_" + j + "_activate",
                TraktorS3.Channel.prototype.hotcuesOutput.bind(this)));
            this.hotcueCallbacks.push(engine.makeConnection(this.group, "hotcue_" + j + "_color",
                TraktorS3.Channel.prototype.hotcuesOutput.bind(this)));
        }
    }

    channelBaseColor() {
        if (this.group === "[Channel4]" && this.controller.channel4InputMode) {
            return this.controller.hid.LEDColors[this.controller.hid.LEDColors.OFF];
        }
        return this.controller.hid.LEDColors[TraktorS3.ChannelColors[this.group]];
    }

    // colorOutput drives lights that have the palettized multicolor lights.
    colorOutput(value, key) {
        let ledValue = this.channelBaseColor();
        if (value === 1 || value === true) {
            ledValue += TraktorS3.LEDBrightValue;
        } else {
            ledValue += TraktorS3.LEDDimValue;
        }
        this.controller.hid.setOutput(this.group, key, ledValue, !this.controller.batchingOutputs);
    }

    hotcuesOutput(_value, group, key) {
        const deck = this.controller.Channels[group].parentDeck;
        if (deck.activeChannel !== group) {
            // Not active, ignore
            return;
        }
        if (deck.padModeState !== 0) {
            return;
        }
        const matches = key.match(/hotcue_(\d+)_/);
        if (matches.length !== 2) {
            HIDDebug("Didn't get expected hotcue number from string: " + key);
            return;
        }
        const cueNum = matches[1];
        deck.lightHotcue(cueNum);
    }

    // Returns true if there was an update.
    lightWheelPosition() {
        if (!this.positionUpdated) {
            return false;
        }
        this.positionUpdated = false;
        const rotations = this.curPosition * (1 / 1.8); // 1/1.8 is rotations per second (33 1/3 RPM)

        // Calculate angle from 0-1.0
        const angle = rotations - Math.floor(rotations);
        // The wheel has 8 segments
        const wheelAngle = 8.0 * angle;
        const baseLedValue = this.channelBaseColor();
        // Reduce the dimming distance at the end of track.
        let dimDistance = this.endOfTrack ? 2.5 : 1.5;
        const segValues = [0, 0, 0, 0, 0, 0, 0, 0];
        for (let seg = 0; seg < 8; seg++) {
            const distance = TraktorS3.wheelSegmentDistance(seg, wheelAngle);
            let brightVal = Math.round(4 * (1.0 - (distance / dimDistance)));
            if (this.endOfTrack) {
                dimDistance = 1.5;
                brightVal = Math.round(4 * (1.0 - (distance / dimDistance)));
                if (this.endOfTrackBlinkState) {
                    brightVal = brightVal > 0x03 ? 0x04 : 0x02;
                } else {
                    brightVal = brightVal > 0x02 ? 0x04 : 0x00;
                }
            }
            if (brightVal <= 0) {
                segValues[seg] = 0x00;
            } else {
                segValues[seg] = baseLedValue + brightVal - 1;
            }
        }
        this.parentDeck.wheelOutput(this.group, segValues);
        return true;
    }
};

/**
 * Normalize a 12-bit 0-4095 input value to a `[0, 1]` value, with a special
 * mapping from 2047 to 0.5 as that would become 0.499878. This is important for
 * FX and anything else where 0.5 has a special meaning (and where that number
 * is checked for without an epsilon).
 *
 * @param {number} value An integer value in the range `[0 .. 4095]`.
 * @returns {number} `value` normalized to `[0, 1]`.
 */
TraktorS3.normalize12BitValue = function(value) {
    return value === 2047 ? 0.5 : value / 4095;
};

// Finds the shortest distance between two angles on the wheel, assuming
// 0-8.0 angle value.
TraktorS3.wheelSegmentDistance = function(segNum, angle) {
    // Account for wraparound
    if (Math.abs(segNum - angle) > 4) {
        if (angle > segNum) {
            segNum += 8;
        } else {
            angle += 8;
        }
    }
    return Math.abs(angle - segNum);
};

// FXControl is an object that manages the gray area in the middle of the
// controller: the fx control knobs, fxenable buttons, and fx select buttons.
TraktorS3.FXControl = class {
    constructor(controller) {
        // 0 is filter, 1-4 are FX Units 1-4
        this.FILTER_EFFECT = 0;
        this.activeFX = this.FILTER_EFFECT;
        this.controller = controller;

        this.enablePressed = {
            "[Channel1]": false,
            "[Channel2]": false,
            "[Channel3]": false,
            "[Channel4]": false
        };
        this.selectPressed = [
            false,
            false,
            false,
            false,
            false
        ];
        this.selectBlinkState = [
            false,
            false,
            false,
            false,
            false
        ];

        // States
        this.STATE_FILTER = 0;
        // State for when an effect select has been pressed, but not released yet.
        this.STATE_EFFECT_INIT = 1;
        // State for when an effect select has been pressed and released.
        this.STATE_EFFECT = 2;
        this.STATE_FOCUS = 3;

        this.currentState = this.STATE_FILTER;

        // Light states
        this.LIGHT_OFF = 0;
        this.LIGHT_DIM = 1;
        this.LIGHT_BRIGHT = 2;

        this.focusBlinkState = false;
        this.focusBlinkTimer = 0;
    }

    registerInputs(messageShort, messageLong) {
        this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx1", 0x08, 0x08, this.fxSelectHandler.bind(this));
        this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx2", 0x08, 0x10, this.fxSelectHandler.bind(this));
        this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx3", 0x08, 0x20, this.fxSelectHandler.bind(this));
        this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx4", 0x08, 0x40, this.fxSelectHandler.bind(this));
        this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx0", 0x08, 0x80, this.fxSelectHandler.bind(this));

        this.controller.registerInputButton(messageShort, "[Channel3]", "!fxEnabled", 0x07, 0x08, this.fxEnableHandler.bind(this));
        this.controller.registerInputButton(messageShort, "[Channel1]", "!fxEnabled", 0x07, 0x10, this.fxEnableHandler.bind(this));
        this.controller.registerInputButton(messageShort, "[Channel2]", "!fxEnabled", 0x07, 0x20, this.fxEnableHandler.bind(this));
        this.controller.registerInputButton(messageShort, "[Channel4]", "!fxEnabled", 0x07, 0x40, this.fxEnableHandler.bind(this));

        this.controller.registerInputScaler(messageLong, "[Channel1]", "!fxKnob", 0x39, 0xFFFF, this.fxKnobHandler.bind(this));
        this.controller.registerInputScaler(messageLong, "[Channel2]", "!fxKnob", 0x3B, 0xFFFF, this.fxKnobHandler.bind(this));
        this.controller.registerInputScaler(messageLong, "[Channel3]", "!fxKnob", 0x37, 0xFFFF, this.fxKnobHandler.bind(this));
        this.controller.registerInputScaler(messageLong, "[Channel4]", "!fxKnob", 0x3D, 0xFFFF, this.fxKnobHandler.bind(this));
    }

    channelToIndex(group) {
        const result = group.match(script.channelRegEx);
        if (result === null) {
            return undefined;
        }
        // Unmap from channel number to button index.
        switch (result[1]) {
        case "1":
            return 2;
        case "2":
            return 3;
        case "3":
            return 1;
        case "4":
            return 4;
        }
        return undefined;
    }

    firstPressedSelect() {
        for (const idx in this.selectPressed) {
            if (this.selectPressed[idx]) {
                return idx;
            }
        }
        return undefined;
    }

    firstPressedEnable() {
        for (const ch in this.enablePressed) {
            if (this.enablePressed[ch]) {
                return ch;
            }
        }
        return undefined;
    }

    anyEnablePressed() {
        for (const key in this.enablePressed) {
            if (this.enablePressed[key]) {
                return true;
            }
        }
        return false;
    }

    changeState(newState) {
        if (newState === this.currentState) {
            return;
        }

        // Ignore next values for all knob actions. This is safe to do for all knobs
        // even if we're ignoring knobs that aren't active in the new state.
        for (let ch = 1; ch <= 4; ch++) {
            engine.softTakeoverIgnoreNextValue(`[QuickEffectRack1_[Channel${ch}]]`, "super1");
        }

        for (let unit = 1; unit <= 4; unit++) {
            engine.softTakeoverIgnoreNextValue(`[EffectRack1_EffectUnit${unit}]`, "mix");

            for (let effect = 1; effect <= 4; effect++) {
                const group = `[EffectRack1_EffectUnit${unit}_Effect${effect}]`;
                engine.softTakeoverIgnoreNextValue(group, "meta");
                for (let param = 1; param <= 4; param++) {
                    engine.softTakeoverIgnoreNextValue(group, "parameter" + param);
                }
            }
        }

        const oldState = this.currentState;
        this.currentState = newState;
        if (oldState === this.STATE_FOCUS) {
            engine.stopTimer(this.focusBlinkTimer);
            this.focusBlinkTimer = 0;
        }
        switch (newState) {
        case this.STATE_FILTER:
            break;
        case this.STATE_EFFECT_INIT:
            break;
        case this.STATE_EFFECT:
            break;
        case this.STATE_FOCUS:
            this.focusBlinkTimer = engine.beginTimer(150, () => {
                TraktorS3.kontrol.fxController.focusBlinkState = !TraktorS3.kontrol.fxController.focusBlinkState;
                TraktorS3.kontrol.fxController.lightFx();
            }, false);
        }
    }

    fxSelectHandler(field) {
        const fxNumber = parseInt(field.name[field.name.length - 1]);
        // Coerce to boolean
        this.selectPressed[fxNumber] = !!field.value;

        if (!field.value) {
            if (fxNumber === this.activeFX) {
                if (this.currentState === this.STATE_EFFECT) {
                    this.changeState(this.STATE_FILTER);
                } else if (this.currentState === this.STATE_EFFECT_INIT) {
                    this.changeState(this.STATE_EFFECT);
                }
            }
            this.lightFx();
            return;
        }

        switch (this.currentState) {
        case this.STATE_FILTER:
            // If any fxEnable button is pressed, we are toggling fx unit assignment.
            if (this.anyEnablePressed()) {
                for (const key in this.enablePressed) {
                    if (this.enablePressed[key]) {
                        if (fxNumber === 0) {
                            script.toggleControl(`[QuickEffectRack1_${key}_Effect1]`, "enabled");
                        } else {
                            script.toggleControl(`[EffectRack1_EffectUnit${fxNumber}]`, `group_${key}_enable`);
                        }
                    }
                }
            } else {
                if (fxNumber === 0) {
                    this.changeState(this.STATE_FILTER);
                } else {
                    this.changeState(this.STATE_EFFECT_INIT);
                }
                this.activeFX = fxNumber;
            }
            break;
        case this.STATE_EFFECT_INIT:
            // Fallthrough intended
        case this.STATE_EFFECT:
            if (fxNumber === 0) {
                this.changeState(this.STATE_FILTER);
            } else if (fxNumber !== this.activeFX) {
                this.changeState(this.STATE_EFFECT_INIT);
            }
            this.activeFX = fxNumber;
            break;
        case this.STATE_FOCUS:
            if (fxNumber === 0) {
                this.changeState(this.STATE_FILTER);
            } else {
                this.changeState(this.STATE_EFFECT_INIT);
            }
            this.activeFX = fxNumber;
            break;
        }
        this.lightFx();
    }

    fxEnableHandler(field) {
        // Coerce to boolean
        this.enablePressed[field.group] = !!field.value;

        if (!field.value) {
            this.lightFx();
            return;
        }

        const fxGroupPrefix = "[EffectRack1_EffectUnit" + this.activeFX;
        const buttonNumber = this.channelToIndex(field.group);
        switch (this.currentState) {
        case this.STATE_FILTER:
            break;
        case this.STATE_EFFECT_INIT:
            // Fallthrough intended
        case this.STATE_EFFECT:
            if (this.firstPressedSelect()) {
                // Choose the first pressed select button only.
                this.changeState(this.STATE_FOCUS);
                engine.setValue(fxGroupPrefix + "]", "focused_effect", buttonNumber);
            } else {
                script.toggleControl(`${fxGroupPrefix}_Effect${buttonNumber}]`, "enabled");
            }
            break;
        case this.STATE_FOCUS: {
            const focusedEffect = engine.getValue(fxGroupPrefix + "]", "focused_effect");
            script.toggleControl(`${fxGroupPrefix}_Effect${focusedEffect}]`, "button_parameter" + buttonNumber);
        } break;
        }
        this.lightFx();
    }

    fxKnobHandler(field) {
        const value = TraktorS3.normalize12BitValue(field.value);
        const fxGroupPrefix = "[EffectRack1_EffectUnit" + this.activeFX;
        const knobIdx = this.channelToIndex(field.group);

        switch (this.currentState) {
        case this.STATE_FILTER:
            if (field.group === "[Channel4]" && this.controller.channel4InputMode) {
                // There is no quickeffect for the microphone, do nothing.
                return;
            }
            engine.setParameter("[QuickEffectRack1_" + field.group + "]", "super1", value);
            break;
        case this.STATE_EFFECT_INIT:
            // Fallthrough intended
        case this.STATE_EFFECT:
            if (knobIdx === 4) {
                engine.setParameter(fxGroupPrefix + "]", "mix", value);
            } else {
                engine.setParameter(`${fxGroupPrefix}_Effect${knobIdx}]`, "meta", value);
            }
            break;
        case this.STATE_FOCUS: {
            const focusedEffect = engine.getValue(fxGroupPrefix + "]", "focused_effect");
            engine.setParameter(`${fxGroupPrefix}_Effect${focusedEffect}]`, "parameter" + knobIdx, value);
        } break;
        }
    }

    getFXSelectLEDValue(fxNumber, status) {
        const ledValue = this.controller.fxLEDValue[fxNumber];
        switch (status) {
        case this.LIGHT_OFF:
            return 0x00;
        case this.LIGHT_DIM:
            return ledValue;
        case this.LIGHT_BRIGHT:
            return ledValue + 0x02;
        }
    }

    getChannelColor(group, status) {
        const ledValue = this.controller.hid.LEDColors[TraktorS3.ChannelColors[group]];
        switch (status) {
        case this.LIGHT_OFF:
            return 0x00;
        case this.LIGHT_DIM:
            return ledValue;
        case this.LIGHT_BRIGHT:
            return ledValue + 0x02;
        }
    }

    lightFx() {
        this.controller.batchingOutputs = true;

        // Loop through select buttons
        // Idx zero is filter button
        for (let idx = 0; idx < 5; idx++) {
            this.lightSelect(idx);
        }
        for (let ch = 1; ch <= 4; ch++) {
            const channel = "[Channel" + ch + "]";
            this.lightEnable(channel);
        }

        this.controller.batchingOutputs = false;
        for (const packetName in this.controller.hid.OutputPackets) {
            this.controller.hid.OutputPackets[packetName].send();
        }
    }

    lightSelect(idx) {
        let status = this.LIGHT_OFF;
        let ledValue = 0x00;
        switch (this.currentState) {
        case this.STATE_FILTER:
            // Always light when pressed
            if (this.selectPressed[idx]) {
                status = this.LIGHT_BRIGHT;
            } else {
                // select buttons on if fx unit enabled for the pressed channel,
                // otherwise disabled.
                status = this.LIGHT_DIM;
                const pressed = this.firstPressedEnable();
                if (pressed) {
                    const fxGroup = idx === 0 ? `[QuickEffectRack1_${pressed}_Effect1]` : `[EffectRack1_EffectUnit${idx}]`;
                    const fxKey = idx === 0 ? "enabled" : `group_${pressed}_enable`;
                    if (engine.getParameter(fxGroup, fxKey)) {
                        status = this.LIGHT_BRIGHT;
                    } else {
                        status = this.LIGHT_OFF;
                    }
                }
                ledValue = this.getFXSelectLEDValue(idx, status);
            }
            break;
        case this.STATE_EFFECT_INIT:
            // Fallthrough intended
        case this.STATE_EFFECT:
            // Highlight if pressed, disable if active effect.
            // Otherwise off.
            if (this.selectPressed[idx]) {
                status = this.LIGHT_BRIGHT;
            } else if (idx === this.activeFX) {
                status = this.LIGHT_BRIGHT;
            }
            break;
        case this.STATE_FOCUS:
            // if blink state is false, only like active fx bright
            // if blink state is true, active fx is bright and selected effect
            // is dim.  if those are the same, active fx is dim
            if (this.selectPressed[idx]) {
                status = this.LIGHT_BRIGHT;
            } else {
                if (idx === this.activeFX) {
                    status = this.LIGHT_BRIGHT;
                }
                if (this.focusBlinkState) {
                    const fxGroupPrefix = "[EffectRack1_EffectUnit" + this.activeFX;
                    const focusedEffect = engine.getValue(fxGroupPrefix + "]", "focused_effect");
                    if (idx === focusedEffect) {
                        status = this.LIGHT_DIM;
                    }
                }
            }
            break;
        }
        ledValue = this.getFXSelectLEDValue(idx, status);
        this.controller.hid.setOutput("[ChannelX]", "!fxButton" + idx, ledValue, false);
    }

    lightEnable(channel) {
        let status = this.LIGHT_OFF;
        let ledValue = 0x00;
        const buttonNumber = this.channelToIndex(channel);
        switch (this.currentState) {
        case this.STATE_FILTER:
            // enable buttons highlighted if pressed or if any fx unit enabled for channel.
            // Highlight if pressed.
            status = this.LIGHT_DIM;
            if (this.enablePressed[channel]) {
                status = this.LIGHT_BRIGHT;
            } else {
                for (let idx = 1; idx <= 4 && status === this.LIGHT_OFF; idx++) {
                    if (engine.getParameter(`[EffectRack1_EffectUnit${idx}]`, `group_${channel}_enable`)) {
                        status = this.LIGHT_DIM;
                    }
                }
            }
            // Enable buttons have regular deck colors
            ledValue = this.getChannelColor(channel, status);
            break;
        case this.STATE_EFFECT_INIT:
            // Fallthrough intended
        case this.STATE_EFFECT:
            if (this.enablePressed[channel]) {
                status = this.LIGHT_BRIGHT;
            } else {
                // off if nothing loaded, dim if loaded, bright if enabled.
                const group = "[EffectRack1_EffectUnit" + this.activeFX + "_Effect" + buttonNumber + "]";
                if (engine.getParameter(group, "loaded")) {
                    status = this.LIGHT_DIM;
                }
                if (engine.getParameter(group, "enabled")) {
                    status = this.LIGHT_BRIGHT;
                }
            }
            // Colors match effect colors so it's obvious we're in a different mode
            ledValue = this.getFXSelectLEDValue(this.activeFX, status);
            break;
        case this.STATE_FOCUS:
            if (this.enablePressed[channel]) {
                status = this.LIGHT_BRIGHT;
            } else {
                const fxGroupPrefix = "[EffectRack1_EffectUnit" + this.activeFX;
                const focusedEffect = engine.getValue(fxGroupPrefix + "]", "focused_effect");
                const group = `${fxGroupPrefix}_Effect${focusedEffect}]`;
                const key = "button_parameter" + buttonNumber;
                // Off if not loaded, dim if loaded, bright if enabled.
                if (engine.getParameter(group, key + "_loaded")) {
                    status = this.LIGHT_DIM;
                }
                if (engine.getParameter(group, key)) {
                    status = this.LIGHT_BRIGHT;
                }
            }
            // Colors match effect colors so it's obvious we're in a different mode
            ledValue = this.getFXSelectLEDValue(this.activeFX, status);
            break;
        }
        this.controller.hid.setOutput(channel, "!fxEnabled", ledValue, false);
    }
};

TraktorS3.messageCallback = function(_packet, data) {
    for (const name in data) {
        if (Object.prototype.hasOwnProperty.call(data, name)) {
            TraktorS3.kontrol.hid.processButton(data[name]);
        }
    }
};

TraktorS3.incomingData = function(data, length) {
    TraktorS3.kontrol.hid.parsePacket(data, length);
};

TraktorS3.debugLights = function() {
    // Call this if you want to just send raw packets to the controller (good for figuring out what
    // bytes do what).
    const dataStrings = [
        "00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "22 22 22 22  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "22 22 22 22  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "FF FF FF FF  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00 ",
        "00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00",
        "20 08 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00 " +
        "00"
    ];

    const data = [[], [], []];


    for (let i = 0; i < data.length; i++) {
        let ok = true;
        const tokens = dataStrings[i].split(/\s+/);
        HIDDebug("i " + i + " " + tokens);
        data[i].length = tokens.length;
        for (let j = 0; j < tokens.length; j++) {
            const byteStr = tokens[j];
            if (byteStr.length === 0) {
                continue;
            }
            if (byteStr.length !== 2) {
                ok = false;
                HIDDebug("not two characters?? " + byteStr);
            }
            const b = parseInt(byteStr, 16);
            if (b < 0 || b > 255) {
                ok = false;
                HIDDebug("number out of range: " + byteStr + " " + b);
            }
            data[i][j] = b;
        }
        if (ok) {
            let header = 0x80 + i;
            if (i === 2) {
                header = 0xF4;
            }
            controller.send(data[i], data[i].length, header);
        }
    }
    TraktorS3.kontrol.setInputLineMode(false);
};

TraktorS3.shutdown = function() {
    // Deactivate all LEDs
    let packet = Array(267);
    for (let i = 0; i < packet.length; i++) {
        packet[i] = 0;
    }
    controller.send(packet, packet.length, 0x80);
    packet = Array(251);
    for (let i = 0; i < packet.length; i++) {
        packet[i] = 0;
    }
    controller.send(packet, packet.length, 0x81);

    HIDDebug("TraktorS3: Shutdown done!");
};

/**
 * An alternative to `FXControl` that behaves more similarly to how the
 * controller works with Traktor. See the description for
 * `TraktorS3.QuickEffectMode` for more information.
 */
TraktorS3.QuickFxControl = class {
    constructor(controller) {
        this.controller = controller;

        // The difference between the indices for the filter and FX buttons
        // (`[0..4]`) and the quick effect chain presets they should be mapped
        // to (`[2..6]`). These presets are 1-indexed, and the first entry is a
        // blank entry automatically added by Mixxx for disabling the quick
        // effects altogether. This is a constant.
        this.effectChainOffset = 2;

        // This contains the indices of the currently held down Filter and FX
        // select buttons, 0 being the filter and 1-4 being the four FX buttons.
        // We keep track of whether they're currently held down so we can assign
        // an effect chain to a single channel by holding down one of those five
        // buttons and then pressing the channel's FX Enable button.
        this.pressedFxSelectButtons = [];
        // When one of the FX select buttons is held down we need to keep track
        // of whether or not we assigned any quick effect chains. If this
        // happened, then we should avoid changing the other channel's quick
        // effect chains when the button is released. This is reset to false
        // when the last FX Select button is released.
        this.individualFxChainAssigned = false;

        // These are the colors for each FX button, with 0 being the filter
        // button
        this.fxColors = {
            0: this.controller.hid.LEDColors.ORANGE,
            1: this.controller.hid.LEDColors.RED,
            2: this.controller.hid.LEDColors.GREEN,
            3: this.controller.hid.LEDColors.CELESTE,
            4: this.controller.hid.LEDColors.YELLOW,
        };

        if (TraktorS3.QuickEffectModeDefaultToFilter) {
            for (let channel = 1; channel <= 4; channel++) {
                this.setQuickEffectChain(channel, 0);
            }
        }
    }

    registerInputs(messageShort, messageLong) {
        // The filter button
        this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx0", 0x08, 0x80, this.fxSelectHandler.bind(this));
        // The FX 1-4 buttons
        this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx1", 0x08, 0x08, this.fxSelectHandler.bind(this));
        this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx2", 0x08, 0x10, this.fxSelectHandler.bind(this));
        this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx3", 0x08, 0x20, this.fxSelectHandler.bind(this));
        this.controller.registerInputButton(messageShort, "[ChannelX]", "!fx4", 0x08, 0x40, this.fxSelectHandler.bind(this));

        this.controller.registerInputScaler(messageLong, "[Channel1]", "!fxKnob", 0x39, 0xFFFF, this.fxKnobHandler.bind(this));
        this.controller.registerInputScaler(messageLong, "[Channel2]", "!fxKnob", 0x3B, 0xFFFF, this.fxKnobHandler.bind(this));
        this.controller.registerInputScaler(messageLong, "[Channel3]", "!fxKnob", 0x37, 0xFFFF, this.fxKnobHandler.bind(this));
        this.controller.registerInputScaler(messageLong, "[Channel4]", "!fxKnob", 0x3D, 0xFFFF, this.fxKnobHandler.bind(this));

        this.controller.registerInputButton(messageShort, "[Channel3]", "!fxEnabled", 0x07, 0x08, this.fxEnableHandler.bind(this));
        this.controller.registerInputButton(messageShort, "[Channel1]", "!fxEnabled", 0x07, 0x10, this.fxEnableHandler.bind(this));
        this.controller.registerInputButton(messageShort, "[Channel2]", "!fxEnabled", 0x07, 0x20, this.fxEnableHandler.bind(this));
        this.controller.registerInputButton(messageShort, "[Channel4]", "!fxEnabled", 0x07, 0x40, this.fxEnableHandler.bind(this));

        // This changes the lighting of the five FX Select buttons and maybe
        // also the FX Enable buttons
        engine.makeConnection("[QuickEffectRack1_[Channel1]]", "loaded_chain_preset", this.quickEffectChainLoadHandler.bind(this));
        engine.makeConnection("[QuickEffectRack1_[Channel2]]", "loaded_chain_preset", this.quickEffectChainLoadHandler.bind(this));
        engine.makeConnection("[QuickEffectRack1_[Channel3]]", "loaded_chain_preset", this.quickEffectChainLoadHandler.bind(this));
        engine.makeConnection("[QuickEffectRack1_[Channel4]]", "loaded_chain_preset", this.quickEffectChainLoadHandler.bind(this));

        // The FX enable buttons can directly be bound to the quick effect chain
        // enabled status as their lighting doesn't depend on other factors
        engine.makeConnection("[QuickEffectRack1_[Channel1]]", "enabled", value => this.lightFxEnable(1, value === 1));
        engine.makeConnection("[QuickEffectRack1_[Channel2]]", "enabled", value => this.lightFxEnable(2, value === 1));
        engine.makeConnection("[QuickEffectRack1_[Channel3]]", "enabled", value => this.lightFxEnable(3, value === 1));
        engine.makeConnection("[QuickEffectRack1_[Channel4]]", "enabled", value => this.lightFxEnable(4, value === 1));
    }

    // Input handling

    fxSelectHandler(field) {
        // FX Number 0 is the filter, and 1-4 are the FX 1-4 buttons
        const fxNumber = parseInt(field.name[field.name.length - 1]);

        // If one of the four FX Enable buttons is pressed while one of the five
        // FX select buttons are held down, then only that channel's quick
        // effect chain assignments are changed.
        // `this.individualFxChainAssigned` keeps track of whether a quick
        // effects chain has been assigned to an individual channel. To avoid
        // weird things from happening, we keep track of which buttons are
        // pressed. The global effect chain should only change when the last
        // button has been released, and holding down one FX button, assigning
        // that effect to a channel, holding down a second button, and releasing
        // both shouldn't change the global effect assignments.
        if (field.value === 1) {
            this.pressedFxSelectButtons.push(fxNumber);
            if (this.pressedFxSelectButtons.length === 1) {
                this.individualFxChainAssigned = false;
            }

            // The button will be dimly lit while pressed if it has not yet been
            // assigned to any channels
            this.lightFxSelectButtons(fxNumber);
        } else {
            this.pressedFxSelectButtons.splice(this.pressedFxSelectButtons.indexOf(fxNumber));
            if (this.pressedFxSelectButtons.length === 0 && !this.individualFxChainAssigned) {
                for (let channel = 1; channel <= 4; channel++) {
                    this.setQuickEffectChain(channel, fxNumber);
                }
            }
        }
    }

    fxKnobHandler(field) {
        // If the external input toggle is enabled then we should not change
        // channel four's filters as the other mixer controls now also affect
        // the microphone rather than channel four
        if (field.group === "[Channel4]" && this.controller.channel4InputMode) {
            return;
        }

        const value = TraktorS3.normalize12BitValue(field.value);
        engine.setParameter(`[QuickEffectRack1_${field.group}]`, "super1", value);
    }

    fxEnableHandler(field) {
        if (field.value === 0) {
            return;
        }

        // Depending on whether or not one of the five FX+Filter buttons are
        // pressed we'll either assign a quick effects chain to this channel or
        // we'll toggle the quick effect chain's status
        if (this.pressedFxSelectButtons.length > 0) {
            // We'll use the last button pressed
            const fxNumber = this.pressedFxSelectButtons[this.pressedFxSelectButtons.length - 1];
            const channelNumber = parseInt(field.group[field.group.length - 2]);

            this.setQuickEffectChain(channelNumber, fxNumber);

            // This will prevent releasing the button(s) from affecting the
            // other channels
            this.individualFxChainAssigned = true;
        } else {
            const currentStatus = engine.getValue(`[QuickEffectRack1_${field.group}]`, "enabled");
            engine.setValue(`[QuickEffectRack1_${field.group}]`, "enabled", !currentStatus);
        }
    }

    quickEffectChainLoadHandler(_value, _group, _key) {
        // All of the lights should be updated at this point
        this.lightFx();
    }

    /**
     * Set the quick effect chain preset for a channel in accordance to one of
     * the five effect buttons.
     *
     * @param {number} channel The channel number, in 1-4.
     * @param {number} fxButtonIndex The index of the FX button, 0 being the
     *   filter button. Quick effect chain presets are one-indexed, so
     *   fxButtonIndex 0-5 will be mapped to quick effect chain presets 1-6.
     */
    setQuickEffectChain(channel, fxButtonIndex) {
        // The 'Keep superknob position' option should be enabled for this to
        // work as intended
        engine.setValue(`[QuickEffectRack1_[Channel${channel}]]`,
            "loaded_chain_preset",
            fxButtonIndex + this.effectChainOffset);
    }

    // Output handling

    lightFx() {
        this.controller.batchingOutputs = true;

        this.lightFxSelectButtons();
        for (let channel = 1; channel <= 4; channel++) {
            this.lightFxEnable(channel);
        }

        this.controller.batchingOutputs = false;
        for (const packetName in this.controller.hid.OutputPackets) {
            this.controller.hid.OutputPackets[packetName].send();
        }
    }

    lightFxSelectButtons(singleFxNumber = null) {
        // We'll light up the currently active FX chains. This means only a
        // single button is lit when all channels use the same quick effect
        // chain.
        const activeFxSelectButtons = new Set();
        for (let channel = 1; channel <= 4; channel++) {
            const fxNumber = engine.getValue(`[QuickEffectRack1_[Channel${channel}]]`, "loaded_chain_preset") - this.effectChainOffset;
            activeFxSelectButtons.add(fxNumber);
        }

        const lightButton = function(fxNumber) {
            // By default the LED is dimly lit
            let ledColor = this.fxColors[fxNumber];
            if (activeFxSelectButtons.has(fxNumber) || this.pressedFxSelectButtons.indexOf(fxNumber) !== -1) {
                ledColor += TraktorS3.LEDBrightValue;
            } else {
                ledColor += TraktorS3.LEDDimValue;
            }

            this.controller.hid.setOutput("[ChannelX]", `!fxButton${fxNumber}`, ledColor, !this.controller.batchingOutputs);
        }.bind(this);

        if (singleFxNumber !== null) {
            lightButton(singleFxNumber);
        } else {
            for (let fxNumber = 0; fxNumber <= 4; fxNumber++) {
                lightButton(fxNumber);
            }
        }
    }

    lightFxEnable(channelNumber, enabled = null) {
        const channelGroup = `[Channel${channelNumber}]`;
        const quickEffectChainGroup = `[QuickEffectRack1_${channelGroup}]`;

        const fxNumber = engine.getValue(quickEffectChainGroup, "loaded_chain_preset") - this.effectChainOffset;
        // We don't need to query this from the engine when this is called as
        // part of a connection
        const fxEnabled = (enabled !== undefined && enabled !== null)
            ? enabled
            : engine.getValue(quickEffectChainGroup, "enabled") === 1;

        // With the filter/FX number 0 we'll use the channel's color for the FX
        // On LED. Otherwise we'll show the color of the effect. This deviates
        // from the behavior in Traktor, but I think using the deck-colors makes
        // it much easier to remember which channel is active when switching
        // between channels. We'll also fall back to the channel colors if the
        // user manually selected an out of bounds chain
        let ledColor = fxEnabled ? TraktorS3.LEDBrightValue : TraktorS3.LEDDimValue;
        if (!TraktorS3.QuickEffectModeChannelColors || (fxNumber >= 1 && fxNumber <= 5)) {
            ledColor += this.fxColors[fxNumber];
        } else {
            ledColor += this.controller.hid.LEDColors[TraktorS3.ChannelColors[channelGroup]];
        }

        this.controller.hid.setOutput(channelGroup, "!fxEnabled", ledColor, !this.controller.batchingOutputs);
    }
};

TraktorS3.init = function(_id) {
    this.kontrol = new TraktorS3.Controller();
    this.kontrol.Decks = {
        "deck1": new TraktorS3.Deck(this.kontrol, 1, "deck1"),
        "deck2": new TraktorS3.Deck(this.kontrol, 2, "deck2"),
    };

    this.kontrol.Channels = {
        "[Channel1]": new TraktorS3.Channel(this.kontrol, this.kontrol.Decks.deck1, "[Channel1]"),
        "[Channel3]": new TraktorS3.Channel(this.kontrol, this.kontrol.Decks.deck1, "[Channel3]"),
        "[Channel4]": new TraktorS3.Channel(this.kontrol, this.kontrol.Decks.deck2, "[Channel4]"),
        "[Channel2]": new TraktorS3.Channel(this.kontrol, this.kontrol.Decks.deck2, "[Channel2]"),
    };

    if (TraktorS3.QuickEffectMode) {
        this.kontrol.fxController = new TraktorS3.QuickFxControl(this.kontrol);
    } else {
        this.kontrol.fxController = new TraktorS3.FXControl(this.kontrol);
    }

    this.kontrol.registerInputPackets();
    this.kontrol.registerOutputPackets();
    HIDDebug("TraktorS3: Init done!");

    if (TraktorS3.DebugMode) {
        TraktorS3.debugLights();
    } else {
        // Light secondary decks first so that the primary decks overwrite their values where
        // needed.  This way the controller looks correct on startup.
        this.kontrol.lightDeck("[Channel3]", false);
        this.kontrol.lightDeck("[Channel4]", false);
        this.kontrol.lightDeck("[Channel1]", false);
        this.kontrol.lightDeck("[Channel2]", true);

        this.kontrol.fxController.lightFx();
    }

    this.kontrol.setInputLineMode(TraktorS3.inputModeLine);
};
