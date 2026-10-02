/*
 * Denon SC Live 4 - Mixxx mapping
 *
 * SC Live 4 mapping by Jammes Luckett (pka Sister Transistor) // Art Mechanix
 * https://www.thesistertransistor.com  -  https://www.artmechanix.com
 * Code adapted from the Denon Prime 4 mapping by Levi Williams
 * (Whanake / whanake-music) and RattyDAVE.
 *
 * Free and open source under the GNU General Public License v2 or later, like
 * Mixxx itself: you're welcome to use it, change it and share it. If you share
 * changes, please keep them open source too.
 *
 * For installation, how every control works, the pad meters and the settings
 * below, see README.md, which comes with this mapping.
 */

// eslint-disable-next-line no-var
var SCLive4 = {};


// USER SETTINGS: chosen in Mixxx > Preferences > Controllers > SC LIVE 4
// (declared in the XML file's <settings>). Each one is explained there and in
// the "Settings" section of README.md.

// Track Skip buttons: "denon", "skip" or "seek"
const skipButtonBehavior = engine.getSetting("skipButtonBehavior");
// SHIFT + SYNC: "off" or "quantize"
const shiftSyncBehavior = engine.getSetting("shiftSyncBehavior");
const quantizeOnStartup = engine.getSetting("quantizeOnStartup");
const channelVolumesToZeroOnStartup = engine.getSetting("channelVolumesToZeroOnStartup");
// Loop resizing keeps: "start", "end" or "mixxx" (each deck's own setting)
const loopAnchor = engine.getSetting("loopAnchor");
const wheelSensitivity = engine.getSetting("wheelSensitivity");
// The tempo ranges SHIFT + PITCH BEND - / + steps through (0.04 = 4%)
const rateRangeSets = {
    denon: [0.04, 0.08, 0.10, 0.20, 0.50, 1.00],
    narrow: [0.04, 0.06, 0.08, 0.10, 0.16],
    wide: [0.08, 0.16, 0.50, 1.00],
};
const rateRanges = rateRangeSets[engine.getSetting("rateRanges")] || rateRangeSets.denon;
const faderEchoTailBars = engine.getSetting("faderEchoTailBars");
// Fader Echo: "default" (Mixxx's Echo, set up by the mapping) or "custom"
// (the effect at the top of the list, used with its own settings)
const faderEchoCustom = engine.getSetting("faderEchoMode") === "custom";
// LIGHTING button: "none", "record", "autodj" or "broadcast"
const lightingButtonAction = engine.getSetting("lightingButtonAction");

// Sweep FX: "default" (shaped to sound similar to the SC Live 4's own, with
// Wash built from Echo) or "custom" (each button sweeps the effect at its
// position with the channel knob)
const sweepFxCustom = engine.getSetting("sweepFxMode") === "custom";

// Not settings: the effects the mapping uses, by position in Mixxx's lists
// (Preferences > Effects), counting the top entry as 1. Sweep FX use the
// Quick Effect Chain Presets list. With Default SCL4 Sweep FX: 1 Filter or
// Moog Filter, 2 White Noise, 3 Echo (Wash is built from Echo). With Custom
// Sweep FX, position 4 is the Wash button's. Fader Echo uses the Effect Chain
// Presets list: 1 Echo.
const sweepFxPresets = {
    filter: 1,
    noise: 2,
    echo: 3,
    wash: 4,
};
const faderEchoChainPreset = 1;
// Not a setting: the strength of Noise and Echo with a Sweep FX knob turned
// all the way (0.0 = none, 1.0 = full), tuned to sound like the SC Live 4's own
const sweepFxMix = {
    noise: 0.35,
    echo: 0.8,
};

/*
 * NOTE ON STEMS (not yet supported by this mapping)
 *
 * - Mixxx 2.5.x (the current full release, which this mapping targets) has
 *   no stem support.
 * - Mixxx 2.6 (in beta since May 2025, not yet a full release at the time of
 *   writing) adds stem playback with per-stem volume and quick effects, plus
 *   stem controls that mappings can use.
 *   See https://mixxx.org/news/2024-08-26-stem-mixing/
 * - Mixxx 2.6 reads stem files in the Native Instruments stem format
 *   (.stem.mp4: the full mix + 4 stems, plus "stem" metadata). Mixxx accepts
 *   more audio codecs inside that format than the original specification.
 * - Engine DJ's stems (Engine Library/Stems/*.stems) can't be used. They are
 *   MP4 files labeled as 8-channel AAC, but the audio data is encrypted with
 *   Denon's own private encryption, so ffmpeg and other decoders can't read
 *   them (as found with this project's files, September 2026). See
 *   https://github.com/danielkinahan/engine-dj-stems-research
 * - Tested September 2026: this mapping runs unchanged in the Mixxx 2.6 beta.
 *   Stem controls were not added, because there's no practical way yet for
 *   SC Live 4 users to get stem files Mixxx can play (their Engine DJ stems
 *   can't be used). To revisit if that changes.
 */

// Convert user-preference for `skipButtonBehavior` into appropriate keys for components
let trackSkipMode = [];
if (skipButtonBehavior === "denon") {
    trackSkipMode = ["back", "fwd"]; // used while held; a tap changes track (see SCLive4.Deck)
} else if (skipButtonBehavior === "skip") {
    trackSkipMode = ["start", "end"];
} else if (skipButtonBehavior === "seek") {
    trackSkipMode = ["back", "fwd"];
}

// Beatjump sizes
const jumpSizes = [1/32, 1/16, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16, 32, 64];

// Beatloop sizes
const loopSizes = [1/32, 1/16, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16, 32, 64];

// Component re-jigging for pad mode purposes
components.ComponentContainer.prototype.reconnectComponents = function(operation, recursive) {
    this.forEachComponent(function(component) {
        component.disconnect();
        if (typeof operation === "function") {
            operation.call(this, component);
        }
        if (component.outConnect) { component.connect(); }
        if (component.outTrigger) { component.trigger(); }
    }, recursive);
};

// 'Off' value sets lights to dim instead of off
components.Button.prototype.off = 0x01;

// Internal MIDI color palette
SCLive4.rgbCode = {
    black: 0,
    blueDark: 1,
    blueDim: 2,
    blue: 3,
    greenDark: 4,
    cyanDark: 5,
    aquaDark: 5,
    greenDim: 8,
    cyanDim: 10,
    green: 12,
    aqua: 14,
    cyan: 15,
    teal: 10, // added for the SC Live 4 sampler pads (was undefined)
    redDark: 16,
    magentaDark: 17,
    violetDark: 17,
    yellowDark: 20,
    whiteDark: 21,
    redDim: 32,
    magentaDim: 34,
    purple: 35,
    violet: 35,
    orangeDark: 36,
    yellowDim: 40,
    whiteDim: 42,
    red: 48,
    magenta: 51,
    orange: 56,
    yellow: 60,
    white: 63,
};

// Used in Swiftb0y's NS6II mapping for tempo fader LEDs
SCLive4.physicalSliderPositions = {
    left: 0.5,
    right: 0.5,
};

// Register '0x9n' as a button press and '0x8n' as a button release.
// The SC Live 4 sends releases as '0x9n' with velocity 0, so check the value too.
components.Button.prototype.isPress = function(channel, control, value, status) {
    return (status & 0xF0) === 0x90 && value > 0;
};


// Provide functions for encoders to cycle through an array of values, like beatjump size
// See NS6II mapping
SCLive4.CyclingArrayView = class {
    constructor(indexable, startIndex) {
        this.indexable = indexable;
        this.index = startIndex || 0;
    }
    advanceBy(n) {
        this.index = script.posMod(this.index + n, this.indexable.length);
        return this.current();
    }
    next() {
        if (this.index !== (this.indexable.length - 1)) {
            return this.advanceBy(1);
        } else {
            return this.current();
        }
    }
    previous() {
        if (this.index !== 0) {
            return this.advanceBy(-1);
        } else {
            return this.current();
        }
    }
    current() {
        return this.indexable[this.index];
    }
    // Point at the entry closest to Mixxx's actual value, so stepping starts
    // from where Mixxx really is (e.g. its default 8% rate range).
    syncTo(value) {
        let best = 0;
        this.indexable.forEach((entry, i) => {
            if (Math.abs(entry - value) < Math.abs(this.indexable[best] - value)) {
                best = i;
            }
        });
        this.index = best;
    }
};

SCLive4.WrappingArrayView = class {
    constructor(indexable, startIndex) {
        this.indexable = indexable;
        this.index = startIndex || 0;
    }
    advanceBy(n) {
        this.index = script.posMod(this.index + n, this.indexable.length);
        return this.current();
    }
    next() {
        return this.advanceBy(1);
    }
    previous() {
        return this.advanceBy(-1);
    }
    current() {
        return this.indexable[this.index];
    }
};

// SysEx request: "report the position of every fader, knob and switch".
// Byte 6 is the product number: 0x12 = SC Live 4 (found by testing; the
// Prime 4's is 0x08, which the SC Live 4 ignores). The unit answers with a
// burst of normal MIDI messages, so Mixxx picks up the real positions at
// startup.
// Startup, as Engine OS does it on the SC Live 4: a device inquiry, a request
// for the power-on button state, the initialization message, then (a second
// later) a request for the position of every fader, knob and switch, which
// the unit answers with normal MIDI messages.
SCLive4.startupSysex = [
    [0xF0, 0x7E, 0x00, 0x06, 0x01, 0xF7],
    [0xF0, 0x00, 0x02, 0x0B, 0x7F, 0x12, 0x42, 0x00, 0x00, 0xF7],
    [0xF0, 0x00, 0x02, 0x0B, 0x7F, 0x12, 0x60, 0x00, 0x04, 0x04, 0x01, 0x01, 0x03, 0xF7],
];
SCLive4.queryControlsSysex = [0xF0, 0x00, 0x02, 0x0B, 0x7F, 0x12, 0x04, 0x00, 0x00, 0xF7];

// LEVEL METERS (channel and main): the value is a bitmask, one bit per light
// (found by probing): bits 0-3 = white, bit 4 = blue, bit 5 = orange.
// Engine OS lights them at -45.7, -25.7, -12.2, -7.2, -4.2 and -0.2 dB. Mixxx's
// meter value measures average level and is full at about -11 dB, so those
// points are shifted by 11 dB (the typical gap between music's average and
// peak level) and converted to Mixxx's meter scale:
//   meter = log10(32.767 * 10^((dB - 11) / 20) + 1)
// The orange light shows clipping only (Mixxx's clip warning), as Mixxx's
// mapping guidelines ask.
SCLive4.meterThresholds = [-45.7, -25.7, -12.2, -7.2, -4.2].map(dB =>
    Math.log10(32.767 * Math.pow(10, (dB - 11) / 20) + 1));
SCLive4.meterMask = function(level, peak) {
    const lit = SCLive4.meterThresholds.filter(threshold => level >= threshold).length;
    return ((1 << lit) - 1) | (peak ? 0x20 : 0);
};

// Show the active deck number (1-4) on each jog wheel display, inside its
// "DECK" box, as on the SC Live 4 without a computer. The digit is note 0x38
// on the deck side's channel (the value is the digit to show). The box and
// word are CCs on the same side (found by probing): 0x35 = top of the box,
// 0x36 = bottom of the box, 0x37 = the word "DECK".
SCLive4.deckBoxCCs = [0x35, 0x36, 0x37];

// JOG RING (the white ring on each jog display): CC 0x30 on the deck side's
// channel. Values 1-48 light ONE segment, and 64 + 1-48 light the WHOLE ring
// except one segment. Both count clockwise from 12 o'clock. This follows
// Engine OS's own logic (its "LCD Wheel Display" module):
//   - No track loaded: 0x40, the whole ring lit.
//   - Playing: the dark segment turns like a spot on a record (33 1/3 rpm),
//     following the track's position, so it speeds up and slows down with
//     the tempo and follows scratching. The ring is only updated when the
//     position changes, so it stays still while paused.
//   - End-of-track warning (Mixxx's own, set in Preferences > Waveforms): the
//     ring alternates between the dark segment and a single lit segment, in
//     time with the flashing buttons.
SCLive4.jogRing = {
    cc: 0x30,
    segments: 48,
    revolutionsPerSecond: 100 / 3 / 60, // 33 1/3 rpm
    flashMs: 250,
    lastValue: {},
    lastPosition: {},
    update: function() {
        [[0xB4, SCLive4.leftDeck], [0xB5, SCLive4.rightDeck]].forEach(([status, side]) => {
            if (!side) {
                return;
            }
            const group = side.currentDeck;
            const duration = engine.getValue(group, "duration");
            let value;
            if (!duration) {
                value = 0x40; // no track loaded: the whole ring lit
                this.lastPosition[status] = undefined;
            } else {
                const position = engine.getValue(group, "playposition");
                if (position === this.lastPosition[status] && this.lastValue[status] !== 0x40) {
                    return; // not moving: leave the ring as it is
                }
                this.lastPosition[status] = position;
                // playposition goes below 0 in the lead-in before the track
                // starts, so wrap negative values the right way round.
                const seconds = position * duration;
                const turn = ((seconds * this.revolutionsPerSecond) % 1 + 1) % 1;
                const flashing = engine.getValue(group, "end_of_track") &&
                    Math.floor(Date.now() / this.flashMs) % 2;
                value = (flashing ? 0 : 64) + 1 + Math.floor(turn * this.segments) % this.segments;
            }
            if (this.lastValue[status] !== value) {
                this.lastValue[status] = value;
                midi.sendShortMsg(status, this.cc, value);
            }
        });
    },
};

// Other fixed parts of the jog display (CCs): 0x31-0x34 = the inner blue and
// white segments, 0x39 = the BPM label, 0x3C = the divider line. These are the
// same as Engine OS uses (its "LCD Wheel Display" module), and stay lit.
SCLive4.jogStaticCCs = [0x31, 0x32, 0x33, 0x34, 0x39, 0x3C];

// BPM AND TIME on the jog display, in the format Engine OS uses:
//   BPM:  CC 0x3E switches the BPM number area on / off. The number is SysEx
//         F0 00 02 0B <ch> 02 <BPM x 100 as 6 hex digits, one per byte> F7
//         (BPM 123.45 -> 12345 -> 0x003039 -> 00 00 03 00 03 09).
//   Time: CC 0x3A = the TIME label, CC 0x3F switches the time number area on /
//         off. The number is SysEx
//         F0 00 02 0B <ch> <00 = no sign, 01 = minus> <hours> <minutes>
//         <seconds> <hundredths> F7
// <ch> is the deck side's MIDI channel (0x04 left, 0x05 right). As in Engine
// OS, BPM and time are only shown when a track is loaded. Time shows elapsed
// or remaining (with a minus sign) following Mixxx's own setting: click the
// time on Mixxx's screen to switch.
SCLive4.jogNumbers = {
    bpmAreaCC: 0x3E,
    timeLabelCC: 0x3A,
    timeAreaCC: 0x3F,
    last: {},
    sendIfChanged: function(key, message, send) {
        if (this.last[key] !== message) {
            this.last[key] = message;
            send();
        }
    },
    forget: function() {
        this.last = {}; // send everything again on the next update
    },
    bpmBytes: function(bpm) {
        const hex = Math.round(Math.max(0, bpm) * 100).toString(16)
            .padStart(6, "0")
            .slice(-6);
        return hex.split("").map(digit => parseInt(digit, 16));
    },
    timeBytes: function(seconds, minus) {
        const hundredths = Math.floor(Math.abs(seconds) * 100);
        return [
            minus ? 0x01 : 0x00,
            Math.min(0x7F, Math.floor(hundredths / 360000)), // hours
            Math.floor(hundredths / 6000) % 60, // minutes
            Math.floor(hundredths / 100) % 60, // seconds
            hundredths % 100, // hundredths
        ];
    },
    updateSide: function(cc, side) {
        const group = side.currentDeck;
        const channel = cc & 0x0F;
        const sysex = bytes => {
            const message = [0xF0, 0x00, 0x02, 0x0B, channel].concat(bytes, [0xF7]);
            midi.sendSysexMsg(message, message.length);
        };
        const duration = engine.getValue(group, "duration");
        const loaded = duration > 0;
        const bpm = engine.getValue(group, "bpm");
        const showBpm = loaded && bpm > 0;
        this.sendIfChanged(`${cc}bpmArea`, showBpm, () => midi.sendShortMsg(cc, this.bpmAreaCC, showBpm ? 0x7F : 0x00));
        this.sendIfChanged(`${cc}timeArea`, loaded, () => {
            midi.sendShortMsg(cc, this.timeLabelCC, loaded ? 0x7F : 0x00);
            midi.sendShortMsg(cc, this.timeAreaCC, loaded ? 0x7F : 0x00);
        });
        if (showBpm) {
            const bytes = this.bpmBytes(bpm);
            this.sendIfChanged(`${cc}bpm`, bytes.join(), () => sysex([0x02].concat(bytes)));
        }
        if (loaded) {
            const elapsed = engine.getValue(group, "playposition") * duration;
            // [Controls] ShowDurationRemaining: 0 = elapsed, 1 = remaining,
            // 2 = both (the jog display shows remaining).
            const remaining = engine.getValue("[Controls]", "ShowDurationRemaining") !== 0;
            const bytes = remaining ?
                this.timeBytes(duration - elapsed, true) :
                this.timeBytes(elapsed, elapsed < 0);
            this.sendIfChanged(`${cc}time`, bytes.join(), () => sysex(bytes));
        }
    },
    update: function() {
        [[0xB4, SCLive4.leftDeck], [0xB5, SCLive4.rightDeck]].forEach(([cc, side]) => {
            if (side) {
                this.updateSide(cc, side);
            }
        });
    },
};

SCLive4.showDeckNumbers = function() {
    [[0x94, 0xB4, SCLive4.leftDeck], [0x95, 0xB5, SCLive4.rightDeck]].forEach(([note, cc, side]) => {
        SCLive4.deckBoxCCs.concat(SCLive4.jogStaticCCs).forEach(control => midi.sendShortMsg(cc, control, 0x7F));
        midi.sendShortMsg(note, 0x38, script.deckFromGroup(side.currentDeck));
    });
    SCLive4.jogNumbers.forget(); // the deck may have changed: resend BPM and time
};

SCLive4.init = function(_id, _debug) {
    // Turn off all LEDs
    midi.sendShortMsg(0x90, 0x75, 0x00);

    // Startup messages, then ask the unit to report the position of every
    // control (see SCLive4.startupSysex).
    SCLive4.startupSysex.forEach(message => midi.sendSysexMsg(message, message.length));
    engine.beginTimer(1000, () => {
        midi.sendSysexMsg(SCLive4.queryControlsSysex, SCLive4.queryControlsSysex.length);
    }, true);

    const decks = [
        new SCLive4.Deck(1, 4),
        new SCLive4.Deck(2, 5),
        new SCLive4.Deck(3, 4),
        new SCLive4.Deck(4, 5),
    ];

    // Disconnect all decks at first so they don't fight with each other
    decks.forEach(deck => deck.forEachComponent(comp => comp.disconnect()));

    // Assign each console deck to Mixxx decks 1 and 2 on startup
    SCLive4.leftDeck = decks[0];
    SCLive4.rightDeck = decks[1];


    // SC Live 4: each side has a single deck layer toggle (note 0x1F) instead of
    // the Prime 4's four mixer deck buttons (removed from this mapping). Both layers share the same MIDI
    // channel, so switching is handled entirely in the mapping.
    const makeDeckToggle = function(deckSide, layerA, layerB, midiChannel) {
        return new components.Button({
            midi: [0x90 + midiChannel, 0x1F],
            input: function(channel, control, value, status, _group) {
                if (!this.isPress(channel, control, value, status)) {
                    return;
                }
                if (SCLive4.shift) {
                    // SHIFT + DECK: switch Mixxx between 2- and 4-deck view
                    engine.setValue("[Skin]", "show_4decks", engine.getValue("[Skin]", "show_4decks") ? 0 : 1);
                    return;
                }
                const nextDeck = SCLive4[deckSide] === layerA ? layerB : layerA;
                SCLive4[deckSide].forEachComponent(c => { c.disconnect(); });
                SCLive4[deckSide] = nextDeck;
                SCLive4.jogRing.lastPosition = {}; // redraw the ring for the new deck
                // Show the new deck number first, then refresh all the lights
                // (a burst of messages), then send the number again in case
                // the unit dropped it during the burst.
                SCLive4.showDeckNumbers();
                SCLive4[deckSide].forEachComponent(c => { c.connect(); c.trigger(); });
                engine.beginTimer(150, () => SCLive4.showDeckNumbers(), true);
            },
        });
    };
    SCLive4.deckToggle = {
        left: makeDeckToggle("leftDeck", decks[0], decks[2], 4),
        right: makeDeckToggle("rightDeck", decks[1], decks[3], 5),
    };
    SCLive4.showDeckNumbers();
    // Safety net: re-send both jog deck numbers every second, so the display
    // can never stay out of step with which deck each side controls.
    engine.beginTimer(1000, () => SCLive4.showDeckNumbers());
    // Jog ring: refresh about 30 times a second (only changes are sent).
    engine.beginTimer(33, () => {
        SCLive4.jogRing.update();
        SCLive4.jogNumbers.update();
    });

    // Initialize mixer channel strips
    SCLive4.mixerA = new mixerStrip(1, 0);
    SCLive4.mixerB = new mixerStrip(2, 1);
    SCLive4.mixerC = new mixerStrip(3, 2);
    SCLive4.mixerD = new mixerStrip(4, 3);

    // Press down on the library encoder, acts as 'Enter' key in Mixxx library
    // Browse knob press: open / select. SHIFT + press: add the highlighted
    // track to the end of Mixxx's Auto DJ queue (the SC Live 4's "Prepare
    // list" in Engine OS).
    SCLive4.encoderLoad = new components.Button({
        midi: [0x9F, 0x06],
        group: "[Library]",
        key: "GoToItem",
        input: function(channel, control, value, status, group) {
            if (SCLive4.shift) {
                if (this.isPress(channel, control, value, status)) {
                    engine.setValue("[Library]", "AutoDjAddBottom", 1);
                }
                return;
            }
            components.Button.prototype.input.call(this, channel, control, value, status, group);
        },
    });

    // VIEW button: switch between Mixxx's maximized library (Library View)
    // and the normal decks screen (Performance View), as in the SC Live 4
    // manual. SHIFT + VIEW (Engine OS: cycle layouts): show / hide Mixxx's
    // sampler panel.
    SCLive4.maxView = new components.Button({
        midi: [0x9F, 0x0E],
        group: "[Skin]",
        key: "show_maximized_library",
        type: components.Button.prototype.types.toggle,
        input: function(channel, control, value, status, group) {
            if (SCLive4.shift) {
                if (this.isPress(channel, control, value, status)) {
                    engine.setValue("[Skin]", "show_samplers", engine.getValue("[Skin]", "show_samplers") ? 0 : 1);
                }
                return;
            }
            components.Button.prototype.input.call(this, channel, control, value, status, group);
        },
    });

    // MENU button (0x9F 0x0D): show / hide Mixxx's effects panel. SHIFT +
    // MENU: show / hide Mixxx's microphone panel. (Mixxx's Preferences can't be
    // opened from a mapping.) The button appears to have no light.
    SCLive4.menuButton = {
        input: function(_channel, _control, value) {
            if (value === 0) {
                return;
            }
            const key = SCLive4.shift ? "show_microphones" : "show_effectrack";
            engine.setValue("[Skin]", key, engine.getValue("[Skin]", key) ? 0 : 1);
        },
    };

    // BACK Button
    SCLive4.moveBack = new components.Button({
        midi: [0x9F, 0x03],
        group: "[Library]",
        key: "MoveFocusBackward",
    });

    // FWD Button
    // FWD button. SHIFT + FWD: quantize on/off for all four decks (SC Live 4
    // manual).
    SCLive4.moveForward = new components.Button({
        midi: [0x9F, 0x04],
        group: "[Library]",
        key: "MoveFocusForward",
        input: function(channel, control, value, status, group) {
            if (SCLive4.shift) {
                if (this.isPress(channel, control, value, status)) {
                    const on = engine.getValue("[Channel1]", "quantize") ? 0 : 1;
                    for (let i = 1; i <= 4; i++) {
                        engine.setValue(`[Channel${i}]`, "quantize", on);
                    }
                }
                return;
            }
            components.Button.prototype.input.call(this, channel, control, value, status, group);
        },
    });

    // Sweep FX buttons (Filter, Noise, Echo, Wash): choose what every
    // channel's filter knob sweeps. One is active at a time (bright); press
    // the active one again to turn it off (no sweep effect; the filter knobs
    // then do nothing until another is chosen).
    SCLive4.sweepFx = {};
    const sweepFxCodes = {filter: 0x15, echo: 0x16, noise: 0x17, wash: 0x18};
    Object.keys(sweepFxCodes).forEach(name => {
        SCLive4.sweepFx[name] = new components.Button({
            midi: [0x9F, sweepFxCodes[name]],
            input: function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status)) {
                    SCLive4.selectSweepFx(name === SCLive4.activeSweepFx ? null : name);
                }
            },
        });
    });
    SCLive4.selectSweepFx = function(name) {
        SCLive4.activeSweepFx = name;
        const mixers = [SCLive4.mixerA, SCLive4.mixerB, SCLive4.mixerC, SCLive4.mixerD];
        const position = name === "wash" && !sweepFxCustom ? sweepFxPresets.echo : sweepFxPresets[name]; // Default Wash is built on Echo
        for (let ch = 1; ch <= 4; ch++) {
            const group = `[QuickEffectRack1_[Channel${ch}]]`;
            if (!name) {
                engine.setValue(group, "enabled", 0); // no sweep effect
                continue;
            }
            engine.setValue(group, "loaded_chain_preset", position);
            engine.setValue(group, "enabled", 1);
        }
        // Loading a preset resets its settings a moment later, so apply the
        // knob positions afterward (center if a knob hasn't moved yet). This
        // also keeps the hardware and Mixxx in agreement.
        if (SCLive4.sweepFxTimer) {
            engine.stopTimer(SCLive4.sweepFxTimer);
        }
        SCLive4.sweepFxTimer = engine.beginTimer(100, () => {
            SCLive4.sweepFxTimer = 0;
            if (!name) {
                return;
            }
            for (let ch = 1; ch <= 4; ch++) {
                const knob = mixers[ch - 1] && mixers[ch - 1].filterKnob;
                SCLive4.shapeSweepFx(ch, knob && knob.lastValue !== undefined ? knob.lastValue : 0.5);
            }
        }, true);
        if (SCLive4.flashingLeds) {
            SCLive4.flashingLeds.update();
        }
    };

    // SHAPE THE SWEEP FX like the SC Live 4's own.
    // knobValue: 0 = fully left, 0.5 = center, 1 = fully right. Filter, and
    // every button with Custom Sweep FX, use the preset's own knob. Otherwise
    // Noise, Echo and Wash set the effect's settings and the channel's effect
    // mix directly:
    //   mix_mode 1 (dry + wet): the track stays at full level, effect added.
    //   mix_mode 0 (dry / wet): the effect replaces the track as mix rises.
    // Mixxx's Echo settings: parameter1 delay (beats), 2 feedback,
    // 3 ping-pong, 4 send, 5 quantize. White Noise: parameter1 dry/wet.
    SCLive4.shapeSweepFx = function(ch, knobValue) {
        const name = SCLive4.activeSweepFx;
        if (!name) {
            return;
        }
        const chain = `[QuickEffectRack1_[Channel${ch}]]`;
        const effect = `[QuickEffectRack1_[Channel${ch}]_Effect1]`;
        const turn = Math.min(1, Math.abs(knobValue - 0.5) * 2); // 0 center .. 1 end
        const left = knobValue < 0.5;
        if (name !== "filter" && !sweepFxCustom) {
            // Mixxx's built-in Echo and White Noise link a setting (send /
            // dry-wet) to the preset's knob, which starts at 0. Keep that knob
            // fully up so those settings stay where the mapping puts them.
            engine.setParameter(chain, "super1", 1);
        }
        if (name === "filter" || sweepFxCustom) {
            engine.setValue(chain, "mix", 1);
            engine.setParameter(chain, "super1", knobValue);
        } else if (name === "noise") {
            engine.setValue(chain, "mix_mode", 1);
            engine.setValue(effect, "parameter1", 1); // pure noise, added on top
            engine.setValue(chain, "mix", turn * sweepFxMix.noise);
        } else if (name === "echo") {
            engine.setValue(chain, "mix_mode", 1);
            engine.setValue(effect, "parameter1", 0.5 * Math.pow(2, left ? -turn : turn)); // 1/4 .. 1 beat
            engine.setValue(effect, "parameter2", 0.3 + 0.6 * turn);                        // more repeats
            engine.setValue(effect, "parameter3", 0);
            engine.setValue(effect, "parameter4", 1);
            engine.setValue(effect, "parameter5", 1);
            engine.setValue(chain, "mix", turn * sweepFxMix.echo);
        } else if (name === "wash") {
            engine.setValue(chain, "mix_mode", 0);
            engine.setValue(effect, "parameter1", left ? 1 : 0.5); // 1 beat left, 1/2 beat right
            engine.setValue(effect, "parameter2", 0.6);
            engine.setValue(effect, "parameter3", 0);
            engine.setValue(effect, "parameter4", 1);
            engine.setValue(effect, "parameter5", 1);
            engine.setValue(chain, "mix", turn);
        }
    };

    // Start with Filter selected, as Engine OS does (with the knobs at center,
    // there's no effect until a knob is turned).
    SCLive4.selectSweepFx("filter");

    // MASTER METERS (under the main volume knob): 0xBF 0x20 = left strip,
    // 0xBF 0x21 = right strip (found by probing). Same lights and levels as
    // the channel meters (see SCLive4.meterMask).
    const masterMeter = function(cc, level, peak) {
        midi.sendShortMsg(0xBF, cc, SCLive4.meterMask(level, peak));
    };
    SCLive4.masterMeters = [
        engine.makeConnection("[Main]", "vu_meter_left", value =>
            masterMeter(0x20, value, engine.getValue("[Main]", "peak_indicator_left"))),
        engine.makeConnection("[Main]", "vu_meter_right", value =>
            masterMeter(0x21, value, engine.getValue("[Main]", "peak_indicator_right"))),
    ];

    // BEAT FX STRIP -> Mixxx Effect Units 1 and 2 (6 slots: FX1 slots 1-3 =
    // slots 1-3, FX2 slots 1-3 = slots 4-6), used together as one bank.
    //   BPM FX knob: turn = previous / next effect in the current slot (loads
    //     immediately; its name shows in Mixxx's effect unit). Push = move to
    //     the next slot (1 -> ... -> 6); the slot number shows briefly as a
    //     white pad meter (1-6 pads). The strip's other controls follow it.
    //   Channel Assign (3 / 1 / 2 / 4 / M): route both units to that deck
    //     only, or to the main mix (M).
    //   Time/Parameter knob: turn = the current effect's 1st setting (usually
    //     its time / rate); push = switch to its 2nd setting and back. The
    //     value shows briefly as a pad meter: teal = time, magenta = 2nd.
    //     (These settings are visible in Mixxx when the effect is expanded.)
    //     SHIFT + turn (enhancement) = the effect's strength (its main knob
    //     next to the effect name in Mixxx); purple pad meter.
    //   Amount knob = wet/dry of both effect units.
    //   FX On/Off = current slot's effect on/off (lit when on).
    const beatFxUnits = ["[EffectRack1_EffectUnit1]", "[EffectRack1_EffectUnit2]"];
    const beatFxAssign = {0x00: 3, 0x01: 1, 0x02: 2, 0x03: 4, 0x7F: "M"};
    SCLive4.beatFx = {
        slot: 1,
        parameter: 1,
        slotGroup: function(slot) {
            slot = slot || this.slot;
            const unit = Math.ceil(slot / 3);
            const effect = (slot - 1) % 3 + 1;
            return `[EffectRack1_EffectUnit${unit}_Effect${effect}]`;
        },
        selectInput: function(_channel, _control, value) {
            engine.setValue(this.slotGroup(), "effect_selector", value < 0x40 ? 1 : -1);
        },
        selectPushInput: function(_channel, _control, value) {
            if (value > 0) {
                this.slot = this.slot % 6 + 1;
                this.parameter = 1;
                this.connectLed();
                SCLive4.leftDeck.padGrid.showMeter(this.slot, SCLive4.rgbCode.white);
            }
        },
        assignInput: function(_channel, _control, value) {
            const target = beatFxAssign[value];
            beatFxUnits.forEach(unit => {
                for (let ch = 1; ch <= 4; ch++) {
                    engine.setValue(unit, `group_[Channel${ch}]_enable`, target === ch ? 1 : 0);
                }
                engine.setValue(unit, "group_[Master]_enable", target === "M" ? 1 : 0);
            });
        },
        timeInput: function(_channel, _control, value) {
            const step = (value < 0x40 ? value : value - 0x80) / 50;
            if (SCLive4.shift) {
                // SHIFT + turn (enhancement): the current slot's strength, i.e.
                // the effect's main ("meta") knob next to its name in Mixxx.
                const meta = Math.max(0, Math.min(1, engine.getParameter(this.slotGroup(), "meta") + step));
                engine.setParameter(this.slotGroup(), "meta", meta);
                SCLive4.leftDeck.padGrid.showMeter(Math.ceil(meta * 8), 0x23); // purple
                return;
            }
            const key = `parameter${this.parameter}`;
            const newValue = Math.max(0, Math.min(1, engine.getParameter(this.slotGroup(), key) + step));
            engine.setParameter(this.slotGroup(), key, newValue);
            this.showParameter(newValue);
        },
        timePushInput: function(_channel, _control, value) {
            if (value > 0 && SCLive4.shift) {
                // SHIFT + push: reset the effect to its default settings by
                // reloading it (step to the next effect and straight back).
                engine.setValue(this.slotGroup(), "effect_selector", 1);
                engine.setValue(this.slotGroup(), "effect_selector", -1);
                return;
            }
            if (value > 0) {
                this.parameter = this.parameter === 1 ? 2 : 1;
                this.showParameter(engine.getParameter(this.slotGroup(), `parameter${this.parameter}`));
            }
        },
        showParameter: function(value) {
            const color = this.parameter === 1 ? SCLive4.rgbCode.teal : SCLive4.rgbCode.magenta;
            SCLive4.leftDeck.padGrid.showMeter(Math.ceil(value * 8), color);
        },
        amountInput: function(_channel, _control, value) {
            beatFxUnits.forEach(unit => engine.setParameter(unit, "mix", value / 127));
        },
        onOffInput: function(_channel, _control, value) {
            if (value > 0) {
                const group = this.slotGroup();
                engine.setValue(group, "enabled", engine.getValue(group, "enabled") ? 0 : 1);
            }
        },
        // FX On/Off light follows the current slot's on/off state (it flashes
        // while the effect is on, see SCLive4.flashingLeds)
        connectLed: function() {
            if (this.led) {
                this.led.disconnect();
            }
            this.led = engine.makeConnection(this.slotGroup(), "enabled", () => SCLive4.flashingLeds && SCLive4.flashingLeds.update());
            this.led.trigger();
        },
    };
    beatFxUnits.forEach(unit => engine.setValue(unit, "enabled", 1));
    for (let slot = 1; slot <= 6; slot++) {
        engine.setValue(SCLive4.beatFx.slotGroup(slot), "enabled", 0); // start with effects off
    }
    SCLive4.beatFx.connectLed();

    // FLASHING LIGHTS, as in Engine OS: the selected Sweep FX button flashes
    // while its effect is being applied (any channel's Sweep FX knob turned away
    // from center) and is steadily lit otherwise. The FX On/Off button flashes
    // while the current Beat FX slot is on. Other buttons are dim.
    SCLive4.flashingLeds = {
        phase: false,
        // Always resend (4 times a second): the SC Live 4's hardware sometimes
        // changes these lights by itself, so a light is never left wrong.
        send: function(note, value) {
            midi.sendShortMsg(0x9F, note, value);
        },
        update: function() {
            const flash = this.phase ? 0x7F : 0x01;
            const mixers = [SCLive4.mixerA, SCLive4.mixerB, SCLive4.mixerC, SCLive4.mixerD];
            const applied = mixers.some(mixer => mixer && mixer.filterKnob &&
                mixer.filterKnob.lastValue !== undefined && Math.abs(mixer.filterKnob.lastValue - 0.5) > 0.02);
            Object.keys(sweepFxCodes).forEach(name => {
                const selected = name === SCLive4.activeSweepFx;
                this.send(sweepFxCodes[name], selected ? (applied ? flash : 0x7F) : 0x01);
            });
            const fxOn = engine.getValue(SCLive4.beatFx.slotGroup(), "enabled");
            this.send(0x1A, fxOn ? flash : 0x01);
            if (SCLive4.mics) {
                SCLive4.mics.updateButtonLights(this.phase);
            }
        },
    };
    engine.beginTimer(250, () => {
        SCLive4.flashingLeds.phase = !SCLive4.flashingLeds.phase;
        SCLive4.flashingLeds.update();
    });
    SCLive4.flashingLeds.update();

    // MICROPHONES (from testing on the hardware): the SC Live 4 appears to
    // mix its mics into its own outputs, and also to send them to the
    // computer: Mic 1 on input channel 1, Mic 2 / Aux on input channel 2 (set
    // Mixxx's Microphone 1 and 2 to those channels).
    //   Mic buttons (0x9F 0x24 = Mic 1, 0x9F 0x25 = Aux/Mic 2): each press
    //     appears to switch that mic on the unit, with or without SHIFT, and
    //     MIDI sent to these notes appears to set only the button light, so
    //     the mapping counts presses and sets Mixxx's Talk to match.
    //   SHIFT + Mic 1 (TALKOVER): switches Mic 1 and Mixxx's ducking together.
    //   Button lights, as Engine OS: dim = off, bright = on, flashing = on with
    //     ducking. The startup message appears to clear the unit's lights, so
    //     both are set to dim (off) at startup, as Engine OS does.
    //   Peak lights (0x9F 0x2B = Mic 1, 0x9F 0x2A = Aux/Mic 2): off = no sound,
    //     dim = picking up sound, bright = peaking (0 off, 1 dim, 2-127 bright).
    //   Mic level knobs: not mapped. The unit appears to apply them itself,
    //     before the signal reaches the computer, so leave Mixxx's mic gain at
    //     its default.
    SCLive4.mics = {
        // Each mic button drives its own Mixxx microphone's Talk (talkover)
        groups: {0x24: "[Microphone]", 0x25: "[Microphone2]"},
        on: {0x24: false, 0x25: false}, // which mics are on (the mapping's record)
        // Mic 2 / Aux switch (0x9F 0x2A, on = Aux; the unit reports it at
        // startup). Aux is treated like Mic 2.
        auxSelected: false,
        // [Master] talkoverDucking values: 0 = off ("Duck"), 1 = Auto, 2 = Man
        duckOff: 0,
        talkoverMode: function() {
            return engine.getSetting("talkoverDucking") === "auto" ? 1 : 2;
        },
        inputSelect: function(_channel, _control, value) {
            SCLive4.mics.auxSelected = value > 0;
        },
        // The unit switches its own mic on or off on every press, so the
        // mapping follows it, sets Mixxx's Talk to match, and takes over the
        // button light (just after the unit lights it itself)
        setMic: function(button, isOn) {
            this.on[button] = isOn;
            this.lightTaken[button] = true;
            engine.setValue(this.groups[button], "talkover", isOn ? 1 : 0);
            engine.beginTimer(30, () => this.updateButtonLights(SCLive4.flashingLeds.phase), true);
        },
        lightTaken: {0x24: true, 0x25: true}, // buttons whose light the mapping sets
        // Called with the flashing lights (4 times a second). A mic flashes
        // while it's on and ducking is on (Mic 2 only when set to Mic, not Aux)
        updateButtonLights: function(phase) {
            const ducking = engine.getValue("[Master]", "talkoverDucking") !== this.duckOff;
            Object.keys(this.on).forEach(key => {
                const button = Number(key);
                if (!this.lightTaken[button]) {
                    return;
                }
                const flash = ducking && (button === 0x24 || !this.auxSelected);
                midi.sendShortMsg(0x9F, button, this.on[button] && (!flash || phase) ? 0x7F : 0x01);
            });
        },
        buttonInput: function(_channel, control, value) {
            if (!(control in SCLive4.mics.on) || value === 0) {
                return;
            }
            const isOn = !SCLive4.mics.on[control];
            SCLive4.mics.setMic(control, isOn);
            if (SCLive4.shift && control === 0x24) {
                // TALKOVER (SHIFT + Mic 1): the press switches Mic 1 as usual,
                // and Mixxx's ducking goes on with it (Man or Auto, chosen in
                // Preferences > Controllers > SC LIVE 4) and off with it
                engine.setValue("[Master]", "talkoverDucking", isOn ? SCLive4.mics.talkoverMode() : SCLive4.mics.duckOff);
            }
        },
        peakLights: {0x24: 0x2B, 0x25: 0x2A}, // mic button -> its peak light
        lastPeak: {},
        peakUntil: {}, // time each light stays bright until
        // Level counted as "picking up sound": about -36 dBFS, above the room
        // noise an open mic picks up, converted to Mixxx's vu_meter scale.
        signalLevel: Math.log10(32.767 * Math.pow(10, -36 / 20) + 1),
        // Bright when Mixxx's mic meter reaches its top (where Mixxx's meter
        // shows the mic is peaking) or Mixxx's clip warning comes on, and held
        // bright briefly so short peaks can be seen.
        peakLevel: 0.95,
        peakHoldMs: 300,
        updatePeakLights: function() {
            const now = Date.now();
            Object.keys(this.peakLights).forEach(button => {
                const group = this.groups[button];
                const level = engine.getValue(group, "vu_meter");
                if (this.on[button] && (level >= this.peakLevel || engine.getValue(group, "peak_indicator"))) {
                    this.peakUntil[button] = now + this.peakHoldMs;
                }
                const value = !this.on[button] ? 0x00
                    : now < (this.peakUntil[button] || 0) ? 0x7F
                        : level > this.signalLevel ? 0x01 : 0x00;
                if (value !== this.lastPeak[button]) {
                    this.lastPeak[button] = value;
                    midi.sendShortMsg(0x9F, this.peakLights[button], value);
                }
            });
        },
    };
    // Start with ducking off (Mixxx remembers its last setting), so the mics
    // play over the music until SHIFT + Mic 1 turns ducking on
    engine.setValue("[Master]", "talkoverDucking", SCLive4.mics.duckOff);
    engine.beginTimer(50, () => SCLive4.mics.updatePeakLights());

    // LIGHTING BUTTON (0x9F 0x27): Engine Lighting isn't available in Mixxx,
    // so the button can start and stop a Mixxx feature instead, chosen in
    // Preferences. The button appears to have no light.
    const lightingActions = {
        record: {group: "[Recording]", inKey: "toggle_recording"},
        autodj: {group: "[AutoDJ]", inKey: "enabled", toggle: true},
        broadcast: {group: "[Shoutcast]", inKey: "enabled", toggle: true},
    };
    const lightingAction = lightingActions[lightingButtonAction];
    SCLive4.lightingButton = {
        input: function(_channel, _control, value) {
            if (!lightingAction || value === 0) {
                return;
            }
            if (lightingAction.toggle) {
                script.toggleControl(lightingAction.group, lightingAction.inKey);
            } else {
                engine.setValue(lightingAction.group, lightingAction.inKey, 1);
            }
        },
    };

    // Headphone Split: on the SC Live 4 this is a SWITCH that sends its
    // position (0x7F = on, 0x00 = off), so Mixxx's split simply follows it.
    SCLive4.split = new components.Button({
        midi: [0x9F, 0x0B],
        group: "[Master]",
        key: "headSplit",
        input: function(_channel, _control, value, _status, _group) {
            engine.setValue("[Master]", "headSplit", value > 0 ? 1 : 0);
        },
    });

    SCLive4.leftDeck.reconnectComponents();
    SCLive4.rightDeck.reconnectComponents();

    if (quantizeOnStartup) {
        for (let i = 1; i <= 4; i++) {
            engine.setValue(`[Channel${i}]`, "quantize", 1);
        }
    }

    // Safety net: start with all channel volumes at 0. The position report
    // requested above then sets each channel to its real fader position a
    // moment later; if that report were ever lost, nothing plays loudly by
    // surprise and moving a fader brings its channel in immediately.
    if (channelVolumesToZeroOnStartup) {
        for (let i = 1; i <= 4; i++) {
            engine.setValue(`[Channel${i}]`, "volume", 0);
        }
    }

    SCLive4.faderEcho.init();

    // Apply the loopAnchor setting (see Preferences). Mixxx remembers each
    // deck's anchor, so without this a deck left on "End" resizes loops by
    // moving their start.
    if (loopAnchor === "start" || loopAnchor === "end") {
        for (let i = 1; i <= 4; i++) {
            engine.setValue(`[Channel${i}]`, "loop_anchor", loopAnchor === "end" ? 1 : 0);
        }
    }

    // The SC Live 4 restores its default (dim) LEDs shortly after connecting,
    // overwriting the states sent at startup. Re-send everything once it has
    // settled.
    engine.beginTimer(500, () => {
        [SCLive4.leftDeck, SCLive4.rightDeck,
            SCLive4.mixerA, SCLive4.mixerB, SCLive4.mixerC, SCLive4.mixerD,
        ].forEach(container => container.forEachComponent(c => c.trigger()));
        SCLive4.showDeckNumbers();
    }, true);
};

SCLive4.shutdown = function() {
    // Turn off every light this mapping drives. (The Prime 4's "all LEDs to
    // dim" message, 0x90 0x75 0x01, lights EVERYTHING on the SC Live 4.)
    const noteChannels = [0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x9F]; // mixer ch 1-4, decks, global
    noteChannels.forEach(status => {
        for (let note = 0x00; note <= 0x3F; note++) {
            // Mic buttons: off like everything else, unless that mic is still
            // on (it stays live on the unit after Mixxx closes), then bright.
            // Messages to these notes appear to change only the light.
            const micOn = status === 0x9F && SCLive4.mics.on[note];
            midi.sendShortMsg(status, note, micOn ? 0x7F : 0x00);
        }
    });
    for (let ch = 0; ch < 4; ch++) {
        midi.sendShortMsg(0xB0 + ch, 0x0A, 0x00); // channel meters
    }
    midi.sendShortMsg(0xBF, 0x20, 0x00); // master meters
    midi.sendShortMsg(0xBF, 0x21, 0x00);
    [0xB4, 0xB5].forEach(status => { // jog display "DECK" box
        SCLive4.deckBoxCCs.concat(SCLive4.jogStaticCCs).forEach(control => midi.sendShortMsg(status, control, 0x00));
        midi.sendShortMsg(status, SCLive4.jogRing.cc, 0x00); // jog ring
        [SCLive4.jogNumbers.bpmAreaCC, SCLive4.jogNumbers.timeLabelCC, SCLive4.jogNumbers.timeAreaCC].forEach(control =>
            midi.sendShortMsg(status, control, 0x00)); // BPM and time
        midi.sendShortMsg(status - 0x20, 0x3D, 0x00); // "all pixels" off, as Engine OS does
    });

};

// All components contained in each mixer strip
const mixerStrip = function(deckNumber, midiOffset) {
    components.Deck.call(this, deckNumber);

    // Gain Knob
    this.gain = new components.Pot({
        midi: [0xB0 + midiOffset, 0x03],
        group: `[Channel${deckNumber}]`,
        inKey: "pregain",
    });

    // High EQ Knob
    this.eqHigh = new components.Pot({
        midi: [0xB0 + midiOffset, 0x04],
        group: `[EqualizerRack1_[Channel${deckNumber}]_Effect1]`,
        inKey: "parameter3",
    });

    // Mid EQ Knob
    this.eqMid = new components.Pot({
        midi: [0xB0 + midiOffset, 0x06],
        group: `[EqualizerRack1_[Channel${deckNumber}]_Effect1]`,
        inKey: "parameter2",
    });

    // Low EQ Knob
    this.eqLow = new components.Pot({
        midi: [0xB0 + midiOffset, 0x08],
        group: `[EqualizerRack1_[Channel${deckNumber}]_Effect1]`,
        inKey: "parameter1",
    });

    // VU Meters (lights and levels: see SCLive4.meterMask)
    this.vuMeter = new components.Component({
        midi: [0xB0 + midiOffset, 0x0A],
        group: `[Channel${deckNumber}]`,
        outKey: "vu_meter",
        output: function(value, group) {
            this.send(SCLive4.meterMask(value, engine.getValue(group, "peak_indicator") === 1));
        },
    });

    // Sweep FX knob: shapes the channel's Sweep FX (chosen with the Sweep FX
    // buttons, see SCLive4.shapeSweepFx). Remembers its position so switching
    // Sweep FX applies it straight away.
    this.filterKnob = new components.Pot({
        midi: [0xB0 + midiOffset, 0x0B],
        group: `[QuickEffectRack1_[Channel${deckNumber}]]`,
        inKey: "super1",
        input: function(_channel, _control, value, _status, _group) {
            this.lastValue = this.inValueScale(value);
            SCLive4.shapeSweepFx(deckNumber, this.lastValue);
        },
    });

    // PFL Button
    this.headphoneCue = new components.Button({
        midi: [0x90 + midiOffset, 0x0D],
        key: "pfl",
        type: components.Button.prototype.types.toggle,
        // SC Live 4 cue LEDs are plain on/off; the Prime 4's "dark" color
        // values still show as lit on most channels.
        on: 0x7F,
        off: 0x00,
    });

    // Volume Fader. No soft takeover: the SC Live 4 can't report fader
    // positions at startup, so any fader movement should take effect at once.
    this.volumeFader = new components.Pot({
        midi: [0x90 + midiOffset, 0x0E],
        inKey: "volume",
        softTakeover: false,
        input: function(channel, control, value, status, group) {
            const newValue = this.inValueScale(value);
            const oldValue = this.lastValue !== undefined ? this.lastValue : engine.getParameter(this.group, "volume");
            this.lastValue = newValue; // physical fader position
            if (SCLive4.faderEcho.fader(deckNumber, newValue, oldValue)) {
                return; // Fader Echo is holding this channel open
            }
            components.Pot.prototype.input.call(this, channel, control, value, status, group);
        },
    });

    // Crossfader Assign Switch
    this.xFaderSwitch = new components.Button({
        midi: [0x90 + midiOffset, 0x0F],
        inKey: "orientation",
        input: function(_channel, _control, value, _status, _group) {
            this.inSetValue(value);
        },
    });

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = this.currentDeck;
        }
    });
};

mixerStrip.prototype = new components.Deck();

// All components contained on each deck
SCLive4.Deck = function(deckNumbers, midiChannel) {
    components.Deck.call(this, deckNumbers);
    const theDeck = this;

    // Used in Swiftb0y's NS6II mapping for tempo fader LEDs
    const makeSliderPosAccessors = function() {
        const lr = midiChannel % 2 === 0 ? "left" : "right";
        return {
            setter: function(pos) {
                SCLive4.physicalSliderPositions[lr] = pos;
            },
            getter: function() {
                return SCLive4.physicalSliderPositions[lr];
            }
        };
    };
    const sliderPosAccessors = makeSliderPosAccessors();

    // BEAT GRID EDIT (SHIFT + SLIP): while on, some buttons edit the grid.
    this.gridEdit = false;
    const gridAware = function(normalInput, gridAction) {
        return function(channel, control, value, status, group) {
            if (theDeck.gridEdit) {
                if (this.isPress(channel, control, value, status)) {
                    engine.setValue(this.group, gridAction, 1);
                    engine.setValue(this.group, gridAction, 0);
                }
                return;
            }
            normalInput.call(this, channel, control, value, status, group);
        };
    };

    // Censor Button
    this.censorButton = new components.Button({
        midi: [0x90 + midiChannel, 0x01],
        unshift: function() {
            this.inKey = "reverseroll";
            this.outKey = this.inKey;
        },
        shift: function() {
            this.inKey = "reverse";
            this.outKey = this.inKey;
        },
    });

    // Track Skip buttons, "denon" mode (SC Live 4 manual):
    //   tap            = previous / next track in the library list
    //   |<< on a paused track that is part-way through = back to its start
    //   SHIFT + hold   = search back / forward through the track
    // (Mixxx only loads onto a playing deck if "allow loading tracks into
    // playing decks" is on in its preferences.)
    const makeDenonSkipButton = function(midiNo, direction, seekKey) {
        return new components.Button({
            midi: [0x90 + midiChannel, midiNo],
            input: function(channel, control, value, status, _group) {
                const pressed = this.isPress(channel, control, value, status);
                if (!pressed) {
                    if (this.searching) {
                        engine.setValue(this.group, seekKey, 0);
                        this.searching = false;
                    }
                    return;
                }
                if (SCLive4.shift) {
                    this.searching = true;
                    engine.setValue(this.group, seekKey, 1);
                    return;
                }
                const paused = !engine.getValue(this.group, "play");
                const partWay = engine.getValue(this.group, "playposition") > 0.01;
                if (direction < 0 && paused && partWay) {
                    engine.setValue(this.group, "start", 1);
                    engine.setValue(this.group, "start", 0);
                    return;
                }
                engine.setValue("[Library]", "focused_widget", 3); // track table
                engine.setValue("[Library]", "MoveVertical", direction);
                engine.setValue(this.group, "LoadSelectedTrack", 1);
                engine.setValue(this.group, "LoadSelectedTrack", 0);
            },
        });
    };

    if (skipButtonBehavior === "denon") {
        this.skipBackButton = makeDenonSkipButton(0x04, -1, trackSkipMode[0]);
        this.skipFwdButton = makeDenonSkipButton(0x05, 1, trackSkipMode[1]);
    } else {
        this.skipBackButton = new components.Button({
            midi: [0x90 + midiChannel, 0x04],
            key: trackSkipMode[0],
        });
        this.skipFwdButton = new components.Button({
            midi: [0x90 + midiChannel, 0x05],
            key: trackSkipMode[1],
        });
    }
    // While Mixxx's Auto DJ is on, >>| triggers Auto DJ's "Fade Now" (crossfade
    // to the next track in the Auto DJ queue) instead, in every Track Skip mode.
    // SHIFT + >>| still works as usual.
    const skipFwdInput = this.skipFwdButton.input;
    this.skipFwdButton.input = function(channel, control, value, status, group) {
        if (engine.getValue("[AutoDJ]", "enabled") && !SCLive4.shift) {
            if (this.isPress(channel, control, value, status)) {
                engine.setValue("[AutoDJ]", "fade_now", 1);
                engine.setValue("[AutoDJ]", "fade_now", 0);
            }
            return;
        }
        skipFwdInput.call(this, channel, control, value, status, group);
    };

    // Beatjump Buttons
    const currentJumpSize = new SCLive4.CyclingArrayView(jumpSizes, 2);
    this.bjumpBackButton = new components.Button({
        midi: [0x90 + midiChannel, 0x06],
        unshift: function() {
            this.inKey = "beatjump_backward";
            this.outKey = this.inKey;
            this.input = gridAware(components.Button.prototype.input, "beats_translate_earlier");
            this.outTrigger = true;
            this.outConnect = true;
        },
        shift: function() {
            this.inKey = "beatjump_size";
            this.outKey = this.inKey;
            this.input = function(channel, control, value, status, group) {
                if (this.isPress(channel, control, value, status, group)) {
                    currentJumpSize.syncTo(engine.getValue(this.group, "beatjump_size"));
                    this.inSetValue(currentJumpSize.previous());
                    // Green meter: 1 pad = 1/4 beat or less ... 8 pads = 32+ beats
                    theDeck.padGrid.showMeter(Math.log2(currentJumpSize.current()) + 3, SCLive4.rgbCode.green);
                }
                this.send(value / 2 + 0.5); // Hacky way to get LEDs to respond properly
            };
            this.outTrigger = false;
            this.outConnect = false;
        },
    });
    this.bjumpFwdButton = new components.Button({
        midi: [0x90 + midiChannel, 0x07],
        unshift: function() {
            this.inKey = "beatjump_forward";
            this.outKey = this.inKey;
            this.input = gridAware(components.Button.prototype.input, "beats_translate_later");
            this.outTrigger = true;
            this.outConnect = true;
        },
        shift: function() {
            this.inKey = "beatjump_size";
            this.outKey = this.inKey;
            this.input = function(channel, control, value, status, group) {
                if (this.isPress(channel, control, value, status, group)) {
                    currentJumpSize.syncTo(engine.getValue(this.group, "beatjump_size"));
                    this.inSetValue(currentJumpSize.next());
                    // Green meter: 1 pad = 1/4 beat or less ... 8 pads = 32+ beats
                    theDeck.padGrid.showMeter(Math.log2(currentJumpSize.current()) + 3, SCLive4.rgbCode.green);
                }
                this.send(value / 2 + 0.5); // Hacky way to get LEDs to respond properly
            };
            this.outTrigger = false;
            this.outConnect = false;
        },
    });

    // Sync Button
    this.syncButton = new components.SyncButton({
        midi: [0x90 + midiChannel, 0x08],
        unshift: function() {
            components.SyncButton.prototype.unshift.call(this);
            // The SC Live 4 lights Sync by itself when pressed. A short press only
            // does a one-off beatsync (sync stays off), so re-send Mixxx's real
            // state afterward or the light gets stuck on.
            const syncInput = this.input;
            this.input = function(channel, control, value, status, group) {
                syncInput.call(this, channel, control, value, status, group);
                this.trigger();
            };
        },
        // Shift + Sync: toggles quantize, or turns sync lock off, depending on
        // shiftSyncBehavior (chosen in Preferences).
        shift: function() {
            if (shiftSyncBehavior === "quantize") {
                components.SyncButton.prototype.shift.call(this);
                return;
            }
            this.input = function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status)) {
                    engine.setValue(this.group, "sync_enabled", 0);
                }
                this.trigger();
            };
        },
    });

    // Cue Button
    this.cueButton = new components.CueButton({
        midi: [0x90 + midiChannel, 0x09],
        // In beat grid edit: move the nearest beat to the playhead
        input: function(channel, control, value, status, group) {
            gridAware(components.Button.prototype.input, "beats_translate_curpos").call(this, channel, control, value, status, group);
        },
    });

    // Play Button
    this.playButton = new components.PlayButton({
        midi: [0x90 + midiChannel, 0x0A],
        // Stop time (settings page): when pausing a playing deck, slow it to
        // a stop like a turntable instead of stopping instantly.
        input: function(channel, control, value, status, group) {
            const brake = SCLive4.settings.stopTime.factors[SCLive4.settings.stopTime.index];
            if (!SCLive4.shift && brake && this.isPress(channel, control, value, status) &&
                    engine.getValue(this.group, "play")) {
                engine.brake(script.deckFromGroup(this.group), true, brake);
                return;
            }
            components.Button.prototype.input.call(this, channel, control, value, status, group);
        },
        unshift: function() {
            components.PlayButton.prototype.unshift.call(this);
            this.type = components.Button.prototype.types.toggle;
        },
        shift: function() {
            this.inKey = "play_stutter";
            this.type = components.Button.prototype.types.push;
        }
    });

    // Performance Pads
    this.padGrid = new SCLive4.PadSection(this, midiChannel - 4);

    // Parameter < / > buttons: halve / double pad lengths (Roll, Loop layer 2)
    this.paramLeftButton = new components.Button({
        midi: [0x90 + midiChannel, 0x17],
        input: function(channel, control, value, status, _group) {
            // Held state is used for Active Loops (hold PARAMETER < + pad)
            theDeck.paramLeftHeld = this.isPress(channel, control, value, status);
            if (this.isPress(channel, control, value, status)) {
                const mode = theDeck.padGrid.currentMode;
                if (mode && mode.onParameter) {
                    mode.onParameter(-1, SCLive4.shift);
                } else {
                    SCLive4.scalePadLengths(mode, -1);
                    SCLive4.showLengthMeter(theDeck, mode);
                }
            }
        },
    });
    // ACTIVE LOOPS (SC Live 4 manual): saved loops marked with PARAMETER < +
    // pad start looping automatically when playback reaches them. Marks are
    // per deck and cleared when a new track is loaded.
    this.activeLoops = {}; // hot cue number -> true
    this.toggleActiveLoop = function(number) {
        if (this.activeLoops[number]) {
            delete this.activeLoops[number];
        } else {
            this.activeLoops[number] = true;
        }
    };
    let lastPosition = -1;
    engine.makeConnection(this.currentDeck, "playposition", playposition => {
        const group = theDeck.currentDeck;
        const position = playposition * engine.getValue(group, "track_samples");
        const previous = lastPosition;
        lastPosition = position;
        if (previous < 0 || position <= previous || engine.getValue(group, "loop_enabled")) {
            return;
        }
        Object.keys(theDeck.activeLoops).forEach(number => {
            const start = engine.getValue(group, `hotcue_${number}_position`);
            if (engine.getValue(group, `hotcue_${number}_status`) > 0 && previous < start && start <= position) {
                // Playback just reached this loop's start: switch it on
                engine.setValue(group, `hotcue_${number}_activate`, 1);
                engine.setValue(group, `hotcue_${number}_activate`, 0);
            }
        });
    });
    engine.makeConnection(this.currentDeck, "track_samples", () => {
        theDeck.activeLoops = {}; // new track: clear the marks
        lastPosition = -1;
    });

    this.paramRightButton = new components.Button({
        midi: [0x90 + midiChannel, 0x18],
        input: function(channel, control, value, status, _group) {
            if (this.isPress(channel, control, value, status)) {
                const mode = theDeck.padGrid.currentMode;
                if (mode && mode.onParameter) {
                    mode.onParameter(1, SCLive4.shift);
                } else {
                    SCLive4.scalePadLengths(mode, 1);
                    SCLive4.showLengthMeter(theDeck, mode);
                }
            }
        },
    });


    // Pitch Bend Buttons
    const currentRateRange = new SCLive4.CyclingArrayView(rateRanges, Math.max(0, rateRanges.indexOf(0.10)));
    this.pitchBendUp = new components.Button({
        midi: [0x90 + midiChannel, 0x1E],
        type: components.Button.prototype.types.push,
        unshift: function() {
            this.inKey = "rate_temp_up";
            this.outKey = this.inKey;
            this.input = gridAware(components.Button.prototype.input, "beats_adjust_faster");
            this.outTrigger = true;
            this.outConnect = true;
        },
        shift: function() {
            this.inKey = "rateRange";
            this.outKey = this.inKey;
            this.input = function(channel, control, value, status, group) {
                if (this.isPress(channel, control, value, status, group)) {
                    currentRateRange.syncTo(engine.getValue(this.group, "rateRange"));
                    this.inSetValue(currentRateRange.next());
                    // Red meter: 1 pad = 4% ... 6 pads = 100% tempo range
                    theDeck.padGrid.showMeter(currentRateRange.index + 1, SCLive4.rgbCode.red);
                }
                this.send(value / 2 + 0.5); // Hacky way to get LEDs to respond properly
            };
            this.outTrigger = false;
            this.outConnect = false;
        },
    });
    this.pitchBendDown = new components.Button({
        midi: [0x90 + midiChannel, 0x1D],
        type: components.Button.prototype.types.push,
        unshift: function() {
            this.inKey = "rate_temp_down";
            this.outKey = this.inKey;
            this.input = gridAware(components.Button.prototype.input, "beats_adjust_slower");
            this.outTrigger = true;
            this.outConnect = true;
        },
        shift: function() {
            this.inKey = "rateRange";
            this.outKey = this.inKey;
            this.input = function(channel, control, value, status, group) {
                if (this.isPress(channel, control, value, status, group)) {
                    currentRateRange.syncTo(engine.getValue(this.group, "rateRange"));
                    this.inSetValue(currentRateRange.previous());
                    // Red meter: 1 pad = 4% ... 6 pads = 100% tempo range
                    theDeck.padGrid.showMeter(currentRateRange.index + 1, SCLive4.rgbCode.red);
                }
                this.send(value / 2 + 0.5); // Hacky way to get LEDs to respond properly
            };
            this.outTrigger = false;
            this.outConnect = false;
        },
    });

    // Tempo Fader
    this.tempoFader = new components.Pot({
        midi: [0xB0 + midiChannel, 0x1F],
        inKey: "rate",
        invert: true,
        // As in Engine OS, there's no snap zone: only the fader's exact center
        // (8192, which Components scales to exactly 0.5) gives 0% tempo and
        // lights the center light.
        inSetParameter: function(value) {
            sliderPosAccessors.setter(value);
            engine.setParameter(this.group, this.inKey, value);
            theDeck.takeoverLeds.trigger();
        },
    });

    // SC Live 4: the tempo fader has a single center (0%) LED on note 0x2A,
    // found by probing. The Prime 4's up/down takeover arrows (0x33/0x35) have
    // no equivalent here, so they are not driven.
    const tempoCenterLed = 0x2A;

    this.takeoverLeds = new components.Component({
        midi: [0x90 + midiChannel, tempoCenterLed],
        outKey: "rate",
        off: 0,
        output: function(softwareSliderPosition) {
            this.send(softwareSliderPosition === 0 ? 0x7F : 0x00);
        },
    });

    // Keylock Button
    this.keylockButton = new components.Button({
        midi: [0x90 + midiChannel, 0x22],
        outKey: "keylock",
        unshift: function() {
            this.inKey = "keylock";
            this.type = components.Button.prototype.types.toggle;
        },
        // Shift + Key Lock = "RESET" on the SC Live 4: return to the original key.
        // The LED keeps showing key lock status.
        shift: function() {
            this.inKey = "reset_key";
            this.type = components.Button.prototype.types.push;
        },
        // Press = key lock on/off; HOLD (0.5 s) = key sync, i.e. match the key
        // of the other playing deck (SC Live 4 manual).
        input: function(channel, control, value, status, group) {
            if (SCLive4.shift) {
                components.Button.prototype.input.call(this, channel, control, value, status, group);
                return;
            }
            if (this.isPress(channel, control, value, status)) {
                this.holdTimer = engine.beginTimer(500, () => {
                    this.holdTimer = 0;
                    engine.setValue(this.group, "sync_key", 1);
                    engine.setValue(this.group, "sync_key", 0);
                }, true);
            } else if (this.holdTimer) {
                engine.stopTimer(this.holdTimer);
                this.holdTimer = 0;
                engine.setValue(this.group, "keylock", engine.getValue(this.group, "keylock") ? 0 : 1);
            }
        },
    });

    // Vinyl Mode Button
    this.vinylButton = new components.Button({
        midi: [0x90 + midiChannel, 0x23],
        type: components.Button.prototype.types.toggle,
        input: function(channel, control, value, status, _group) {
            if (!this.isPress(channel, control, value, status)) {
                return;
            }
            theDeck.jogWheel.vinylMode = !theDeck.jogWheel.vinylMode;
            this.trigger();
        },
        trigger: function() {
            this.output(theDeck.jogWheel.vinylMode);
        },
    });

    // Jog Wheel
    this.jogWheel = new components.JogWheelBasic({
        deck: script.deckFromGroup(this.currentDeck),
        wheelResolution: 1000,
        alpha: 1/8,
        beta: 1/8/32,
        rpm: 33 + 1/3,
        // Instead of relative movements between this and the last position,
        // the controller reports the absolute position of the wheel with
        // 14-bit precision. Because of that, we need to reconstruct the value
        // and then transform it into the relative directions expected by Mixxx.
        inputWheelMSB: function(_channel, _control, value, _status, _group) {
            this.wheelMSB = value;
        },
        inputWheelLSB: function(channel, control, value, status, group) {
            this.inputWheel(channel, control, (this.wheelMSB << 7) + value, status, group);
        },
        previousPosition: null,
        wrappingValue: Math.pow(2, 14),
        relativeFromAbsolute: function(value) {
            // The first value of the controller will probably be random
            // and thus we just have to swallow it until we have the second value
            // to find the difference
            if (this.previousPosition === null) {
                this.previousPosition = value;
                return 0;
            }
            // This finds the shortest distance between the current value
            // and the last one, and preserves the orientation
            const delta = value - this.previousPosition;
            let remainder = ((delta % this.wrappingValue) + this.wrappingValue) % this.wrappingValue;
            //let remainder = script.posMod(delta, this.wrappingValue);
            if (remainder * 2 > this.wrappingValue) {
                remainder -= this.wrappingValue;
            }
            this.previousPosition = value;
            return remainder;
        },
        jogScale: function(val) {
            // wheelSensitivity is chosen in Preferences.
            return val * wheelSensitivity;
        },
        inputWheel: function(channel, control, value, _status, _group) {
            value = this.relativeFromAbsolute(value);
            if (engine.isScratching(this.deck)) {
                engine.scratchTick(this.deck, value);
            } else {
                this.inSetValue(this.jogScale(value));
            }
        },
    });

    // Slip Mode Button. Tap SLIP: slip on / off. Hold SLIP (as in Engine OS) or
    // SHIFT + SLIP (as in the manual): beat grid edit on. While it is on,
    // SHIFT + SLIP undoes the last grid change and SLIP leaves grid edit.
    // The Slip light flashes during grid edit.
    this.slipButton = new components.Button({
        midi: [0x90 + midiChannel, 0x24],
        key: "slip_enabled",
        holdMs: 500,
        input: function(channel, control, value, status, _group) {
            if (!this.isPress(channel, control, value, status)) {
                // Release: a short tap switches slip on / off
                if (this.holdTimer) {
                    engine.stopTimer(this.holdTimer);
                    this.holdTimer = 0;
                    engine.setValue(this.group, "slip_enabled", engine.getValue(this.group, "slip_enabled") ? 0 : 1);
                }
                return;
            }
            if (SCLive4.shift) {
                if (theDeck.gridEdit) {
                    engine.setValue(this.group, "beats_undo_adjustment", 1);
                    engine.setValue(this.group, "beats_undo_adjustment", 0);
                } else {
                    this.setGridEdit(true);
                }
                return;
            }
            if (theDeck.gridEdit) {
                this.setGridEdit(false);
                return;
            }
            // Wait to see whether this is a tap or a hold
            this.holdTimer = engine.beginTimer(this.holdMs, () => {
                this.holdTimer = 0;
                this.setGridEdit(true);
            }, true);
        },
        setGridEdit: function(on) {
            theDeck.gridEdit = on;
            if (this.flashTimer) {
                engine.stopTimer(this.flashTimer);
                this.flashTimer = 0;
            }
            if (on) {
                let lit = false;
                this.flashTimer = engine.beginTimer(250, () => {
                    lit = !lit;
                    this.send(lit ? 0x7F : 0x00);
                });
            } else {
                this.trigger();
            }
        },
    });

    // Loop Encoder
    const currentLoopSize = new SCLive4.CyclingArrayView(loopSizes, 6);
    this.loopEncoder = new components.Pot({
        midi: [0x90 + midiChannel, 0x20],
        key: "beatloop_size",
        input: function(channel, control, value, _status, _group) {
            const direction = value === 0x01 ? 1 : value === 0x7f ? -1 : 0;
            if (!direction) {
                return;
            }
            if (SCLive4.shift) {
                // Shift + turn: move the active loop (SC Live 4 manual)
                SCLive4.moveLoop(this.group, direction);
                return;
            }
            // Start from Mixxx's actual loop size, not a remembered one
            currentLoopSize.syncTo(engine.getValue(this.group, "beatloop_size"));
            this.inSetValue(direction > 0 ? currentLoopSize.next() : currentLoopSize.previous());
        },
    });

    // Loop Encoder Button
    this.beatLoopTrigger = new components.Button({
        midi: [0x90 + midiChannel, 0x27],
        type: components.Button.prototype.types.push,
        shift: function() {
            this.inKey = "beatlooproll_activate";
            this.outKey = this.inKey;
        },
        unshift: function() {
            this.inKey = "beatloop_activate";
            this.outKey = this.inKey;
        },
    });

    // Loop In / Out Buttons. Lights show the loop state rather than "held":
    // dim = no loop playing, bright = a loop is active (from any source).
    this.loopInButton = new components.Button({
        midi: [0x90 + midiChannel, 0x25],
        inKey: "loop_in",
        outKey: "loop_enabled",
        on: 0x7F,
        off: 0x01,
    });
    this.loopOutButton = new components.Button({
        midi: [0x90 + midiChannel, 0x26],
        inKey: "loop_out",
        outKey: "loop_enabled",
        on: 0x7F,
        off: 0x01,
        // While a loop is playing, Loop Out exits it (SC Live 4 manual);
        // otherwise it sets the loop out point as usual.
        input: function(channel, control, value, status, group) {
            if (this.isPress(channel, control, value, status) && engine.getValue(this.group, "loop_enabled")) {
                engine.setValue(this.group, "reloop_toggle", 1);
                engine.setValue(this.group, "reloop_toggle", 0);
                this.exitedLoop = true;
                return;
            }
            if (this.exitedLoop) { // swallow the release of an exit press
                this.exitedLoop = false;
                return;
            }
            components.Button.prototype.input.call(this, channel, control, value, status, group);
        },
    });

    // Load Buttons
    this.deckLoad = new components.Button({
        midi: [0x9F, midiChannel - 3],
        key: "LoadSelectedTrack",
        // Quickly double-press to instantly double the track playing on the
        // other side's deck (SC Live 4 manual): copies its track and position.
        input: function(channel, control, value, status, group) {
            if (!SCLive4.shift && this.isPress(channel, control, value, status)) {
                const now = Date.now();
                if (this.lastPress && now - this.lastPress < 400) {
                    this.lastPress = 0;
                    const otherSide = SCLive4.leftDeck === theDeck ? SCLive4.rightDeck : SCLive4.leftDeck;
                    engine.setValue(this.group, "CloneFromDeck", script.deckFromGroup(otherSide.currentDeck));
                    return;
                }
                this.lastPress = now;
            }
            components.Button.prototype.input.call(this, channel, control, value, status, group);
        },
        shift: function() {
            this.inKey = "eject";
            this.outKey = this.inKey;
        },
        unshift: function() {
            this.inKey = "LoadSelectedTrack";
            this.outKey = this.inKey;
        },
        // The Load buttons' lights aren't driven by this mapping.
        output: function() {},
    });

    this.reconnectComponents(function(c) {
        if (c.group === undefined) {
            c.group = this.currentDeck;
        }
    });

};

SCLive4.Deck.prototype = new components.Deck();

SCLive4.shift = false;
SCLive4.shiftState = function(channel, control, value) {
    SCLive4.shift = value === 0x7F;
    if (SCLive4.shift) {
        midi.sendShortMsg(0x90 + channel, control, 0x02);
        SCLive4.leftDeck.shift();
        SCLive4.rightDeck.shift();
        SCLive4.leftDeck.reconnectComponents();
        SCLive4.rightDeck.reconnectComponents();
    } else {
        midi.sendShortMsg(0x90 + channel, control, 0x01);
        SCLive4.leftDeck.unshift();
        SCLive4.rightDeck.unshift();
        SCLive4.leftDeck.reconnectComponents();
        SCLive4.rightDeck.reconnectComponents();
    }
};

//========== PERFORMANCE PADS ==========//

// Access the appropriate mode-select pad without remembering MIDI values
SCLive4.padMode = {
    HOTCUE: 0x0B,
    LOOP: 0x0C,
    ROLL: 0x0D,
    SLICER: 0x0E,
};

// SC Live 4 pad mode buttons are single-color: bright = active mode,
// dim = other modes (found by probing).
SCLive4.modeLed = {
    active: 0x7F,
    inactive: 0x01,
};

SCLive4.PadSection = function(deck, offset) {
    components.ComponentContainer.call(this);
    const theContainer = this;

    // Create component containers for each pad mode
    const modes = new components.ComponentContainer({
        "hotcue": new SCLive4.WrappingArrayView([new SCLive4.hotcueMode(deck, offset), new SCLive4.pitchPlayMode(deck, offset)], 0),
        "loop": new SCLive4.WrappingArrayView([new SCLive4.savedLoopMode(deck, offset), new SCLive4.autoloopMode(deck, offset)], 0),
        "roll": new SCLive4.WrappingArrayView([new SCLive4.rollMode(deck, offset), new SCLive4.samplerMode(deck, offset)], 0),
        "slicer": new SCLive4.WrappingArrayView([new SCLive4.slicerMode(deck, offset, false), new SCLive4.slicerMode(deck, offset, true)], 0),
        "rollShift": new SCLive4.WrappingArrayView([new SCLive4.samplerMode(deck, offset)], 0),
        "settings": new SCLive4.WrappingArrayView([new SCLive4.settingsMode(deck, offset)], 0),
    });

    modes.forEachComponent(c => c.disconnect());

    const controlToPadMode = control => {
        // If a pad selector button has multiple modes, go to the first mode
        // by default. Otherwise, go to the next mode in that button's list.
        const nextPadMode = (a) => {
            if (a.indexable.includes(this.currentMode)) {
                return a.next();
            } else {
                a.index = 0;
                return a.current();
            }
        };

        let mode;

        switch (control) {
        case SCLive4.padMode.HOTCUE:
            if (SCLive4.shift) {
                mode = nextPadMode(modes.settings); // SHIFT + HOT CUE: settings page
            } else {
                mode = nextPadMode(modes.hotcue);
            }
            break;
        case SCLive4.padMode.LOOP:
            mode = nextPadMode(modes.loop);
            break;
        case SCLive4.padMode.ROLL:
            if (SCLive4.shift) {
                mode = nextPadMode(modes.rollShift);
            } else {
                mode = nextPadMode(modes.roll);
            }
            break;
        case SCLive4.padMode.SLICER:
            mode = nextPadMode(modes.slicer);
            break;
        }

        return mode;
    };

    this.offset = offset;

    // Briefly show a setting as a pad meter: pads 1..level light in the given
    // color for a second, then the current pad mode's lights come back.
    // Used for time-based settings that the SC Live 4 has no display for.
    this.meterActive = false;
    this.showMeter = function(level, color) {
        if (SCLive4.leftDeck !== deck && SCLive4.rightDeck !== deck) {
            return;
        }
        level = Math.max(1, Math.min(8, Math.round(level)));
        this.meterActive = true;
        for (let i = 1; i <= 8; i++) {
            midi.sendShortMsg(0x94 + offset, 0x0E + i, i <= level ? color : 0x00);
        }
        if (this.meterTimer) {
            engine.stopTimer(this.meterTimer);
        }
        this.meterTimer = engine.beginTimer(1000, () => {
            this.meterTimer = 0;
            this.meterActive = false;
            if (this.currentMode) {
                this.currentMode.forEachComponent(c => c.trigger());
            }
        }, true);
    };

    this.padModeSelectLeds = new components.Component({
        trigger: function() {
            /*
             * This function results in the currently selected pad mode showing
             * an inactive state when selecting pad modes with the shift button
             *
            for (const modeLayers of Object.values(modes)) {
                const mode = modeLayers.current();
                midi.sendShortMsg(0x94 + offset, mode.ledControl, theContainer.currentMode === mode ? mode.colorOn : mode.colorOff);
            }
            */
            for (const modeLayers of Object.values(modes)) {
                const mode = modeLayers.current();
                if (theContainer.currentMode !== mode) {
                    midi.sendShortMsg(0x94 + offset, mode.ledControl, SCLive4.modeLed.inactive);
                }
            }
            for (const modeLayers of Object.values(modes)) {
                const mode = modeLayers.current();
                if (theContainer.currentMode === mode) {
                    midi.sendShortMsg(0x94 + offset, mode.ledControl, SCLive4.modeLed.active);
                }
            }
        },
    }, false);

    // Function for switching between pad modes
    this.setPadMode = function(control) {
        const newMode = controlToPadMode(control);

        // Exit early if requested mode is already active or unavailable
        if (newMode === this.currentMode || newMode === undefined) {
            return;
        }

        // Disable LED of current mode button
        if (this.currentMode) {
            if (this.currentMode.onExit) {
                this.currentMode.onExit();
            }
            // Disconnect pads from current mode
            this.currentMode.forEachComponent(function(component) {
                component.disconnect();
                component.outConnect = false;
            });
        }

        // Connect pads to new mode
        newMode.forEachComponent(function(component) {
            component.outConnect = true;
            component.connect();
            component.trigger();
        });
        if (newMode.onEnter) {
            newMode.onEnter();
        }

        // Assign mode select buttons in XML file
        this.padModeButtonPressed = function(channel, control, value, _status, _group) {
            if (value) {
                this.setPadMode(control);
            }
        };

        this.currentMode = newMode;

        theContainer.padModeSelectLeds.trigger();
    };

    // Start in Hotcue mode
    this.setPadMode(SCLive4.padMode.HOTCUE);

};

SCLive4.PadSection.prototype = Object.create(components.ComponentContainer.prototype);

// Worry about parameter buttons later
//SCLive4.PadSection.prototype.paramButtonPressed = function(channel, control, value, status, group) {};

// Assign pads mapped in XML file
SCLive4.PadSection.prototype.performancePad = function(channel, control, value, status, group) {
    const i = (control - 0x0E);
    this.currentMode.pads[i].input(channel, control, value, status, group);
};

// HOTCUE MODE
// SC Live 4 pads ignore the Prime 4's RGB SysEx and instead take a palette
// value as the note velocity: 2 bits each of red (bits 4-5), green (bits 2-3)
// and blue (bits 0-1). Convert a Mixxx hotcue color to the nearest one.
// Dim a palette value by halving each channel (3 -> 1, 2 -> 1, 1 -> 0), which
// keeps the hue: a faint secondary channel drops out instead of becoming as
// strong as the main one (e.g. Mixxx's blue stays blue rather than teal).
// If that would turn the pad off, fall back to the lowest brightness per lit
// channel.
SCLive4.dimPalette = function(value) {
    const channel = shift => (value >> shift) & 0x03;
    const halved = ((channel(4) >> 1) << 4) | ((channel(2) >> 1) << 2) | (channel(0) >> 1);
    if (halved) {
        return halved;
    }
    const lowest = shift => Math.min(channel(shift), 1) << shift;
    return lowest(4) | lowest(2) | lowest(0);
};

// With dim = true the color is dimmed with SCLive4.dimPalette (keeps the hue).
// Same conversion as Engine OS uses for the SC Live 4's pads: each channel is
// boosted by 1.5 and rounded down to 0-3.
SCLive4.paletteFromRGB = function(colorObj, dim) {
    const squash = component => Math.min(Math.floor((component / 255) * 1.5 * 3), 3);
    const value = (squash(colorObj.red) << 4) | (squash(colorObj.green) << 2) | squash(colorObj.blue);
    // Very dark colors would round to off; show them as dim white instead.
    if (!value) {
        return SCLive4.rgbCode.whiteDark;
    }
    return dim ? SCLive4.dimPalette(value) : value;
};

SCLive4.hotcueMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = SCLive4.padMode.HOTCUE;
    this.colorOn = SCLive4.rgbCode.blue;
    this.colorOff = SCLive4.rgbCode.whiteDark;
    this.pads = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new components.HotcueButton({
            number: i,
            group: deck.currentDeck,
            midi: [0x94 + offset, 0x0E + i],
            sendRGB: function(colorObj) {
                midi.sendShortMsg(0x94 + offset, 0x0E + i, SCLive4.paletteFromRGB(colorObj));
            },
            on: this.colorOn,
            off: 0x00, // empty hot cue pads are unlit
            outConnect: false,
        });
    }
};
SCLive4.hotcueMode.prototype = Object.create(components.ComponentContainer.prototype);

// PITCH PLAY MODE (Cue button, second layer)
//   - Pads play the last hot cue you used (Mixxx's "hotcue_focus", default 1)
//     at 8 different pitches. Pressing a pad jumps to that cue and plays it.
//   - Default range: -4 ... +3 semitones; the root (0, original key) is pad 5
//     and is lit white. Other pads show the cue's color dimmed; the pitch
//     you last played lights at full.
//   - PARAMETER < / >: move the 8-pad range down / up one semitone (Mixxx's
//     key shift is limited to -6 ... +6, so the root always stays on a pad).
//   - SHIFT + PARAMETER < / >: key shift the track down / up one semitone
//     (-3 ... +3, Mixxx's "pitch_adjust"), on top of the pad pitch.
//   - Shift + Key Lock (Reset) returns the key to the original.
SCLive4.pitchPlayMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    const theMode = this;
    const group = deck.currentDeck;
    this.ledControl = SCLive4.padMode.HOTCUE;
    this.rangeStart = -4; // semitone offset of pad 1

    const focusedCue = () => Math.max(1, engine.getValue(group, "hotcue_focus"));
    const cueColor = () => {
        const cue = focusedCue();
        if (engine.getValue(group, `hotcue_${cue}_status`) === 0) {
            return {red: 0x00, green: 0x44, blue: 0xff}; // no cue set: blue
        }
        const rgb = engine.getValue(group, `hotcue_${cue}_color`);
        return {red: (rgb >> 16) & 0xFF, green: (rgb >> 8) & 0xFF, blue: rgb & 0xFF};
    };

    this.pads = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new components.Button({
            midi: [0x94 + offset, 0x0E + i],
            group: group,
            outKey: "pitch",
            outConnect: false,
            semitone: function() {
                return theMode.rangeStart + i - 1;
            },
            input: function(channel, control, value, status, _group) {
                if (!this.isPress(channel, control, value, status)) {
                    return;
                }
                engine.setValue(group, "pitch", this.semitone());
                const cue = `hotcue_${focusedCue()}_gotoandplay`;
                engine.setValue(group, cue, 1);
                engine.setValue(group, cue, 0);
                theMode.pads.forEachComponent(pad => pad.trigger());
            },
            output: function() {
                const playing = Math.round(engine.getValue(group, "pitch")) === this.semitone();
                if (this.semitone() === 0) {
                    this.send(playing ? SCLive4.rgbCode.white : SCLive4.rgbCode.whiteDark);
                } else {
                    this.send(SCLive4.paletteFromRGB(cueColor(), !playing));
                }
            },
            trigger: function() {
                this.output();
            },
        });
    }

    this.onParameter = function(direction, shifted) {
        if (shifted) {
            const adjust = engine.getValue(group, "pitch_adjust") + direction;
            engine.setValue(group, "pitch_adjust", Math.max(-3, Math.min(3, adjust)));
            return;
        }
        // Keep all 8 pads within -6 ... +6 (so the root stays visible).
        this.rangeStart = Math.max(-6, Math.min(-1, this.rangeStart + direction));
        this.pads.forEachComponent(pad => pad.trigger());
    };
};
SCLive4.pitchPlayMode.prototype = Object.create(components.ComponentContainer.prototype);

// SAVED LOOP MODE
SCLive4.savedLoopMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = SCLive4.padMode.LOOP;
    this.colorOn = SCLive4.rgbCode.blue;
    this.colorOff = SCLive4.rgbCode.whiteDark;
    const theMode = this;
    this.pendingPad = 0; // pad waiting for its Loop Out press
    // Pulse active-loop pads while this mode is showing
    this.pulseOff = false;
    this.updatePulse = function() {
        const needed = this.showing && Object.keys(deck.activeLoops).length > 0;
        if (needed && !this.pulseTimer) {
            this.pulseTimer = engine.beginTimer(400, () => {
                theMode.pulseOff = !theMode.pulseOff;
                theMode.pads.forEachComponent(pad => {
                    if (deck.activeLoops[pad.number]) {
                        // output() only: trigger() would also run the color
                        // update, which re-lights the pad immediately.
                        pad.output(engine.getValue(pad.group, pad.outKey));
                    }
                });
            });
        } else if (!needed && this.pulseTimer) {
            engine.stopTimer(this.pulseTimer);
            this.pulseTimer = 0;
            this.pulseOff = false;
        }
    };
    this.onEnter = function() {
        this.showing = true;
        this.updatePulse();
    };
    this.onExit = function() {
        this.showing = false;
        this.updatePulse();
    };
    this.pads = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new components.HotcueButton({
            number: i + 8,
            group: deck.currentDeck,
            // Manual Loop pads, as in the SC Live 4 manual:
            //   empty pad, no loop playing -> 1st press = Loop In,
            //                                 2nd press = Loop Out; the new
            //                                 loop starts and is saved here
            //   empty pad, a loop playing  -> save that loop to this pad
            //   pad's loop playing         -> turn the loop off
            //   saved pad                  -> jump to its loop and loop it
            //   shift + pad                -> turn its loop off if playing,
            //                                 then delete it
            input: function(channel, control, value, status, _group) {
                if (!this.isPress(channel, control, value, status)) {
                    return;
                }
                const group = this.group;
                const cue = `hotcue_${this.number}`;
                const padStatus = engine.getValue(group, `${cue}_status`); // 0 empty, 1 set, 2 active
                const pulse = key => {
                    engine.setValue(group, key, 1);
                    engine.setValue(group, key, 0);
                };
                const exitLoop = () => {
                    if (engine.getValue(group, "loop_enabled")) {
                        pulse("reloop_toggle");
                    }
                };

                // Active Loops: hold PARAMETER < and press a saved loop pad
                if (deck.paramLeftHeld) {
                    if (padStatus > 0) {
                        deck.toggleActiveLoop(this.number);
                        theMode.updatePulse();
                        this.output(engine.getValue(this.group, this.outKey));
                    }
                    return;
                }
                // Components only track shift on containers, so use the
                // mapping's own shift state here.
                if (SCLive4.shift) {
                    // If this saved loop is also Mixxx's current loop region,
                    // remove that too, or its in/out points stay on the waveform.
                    const isCurrentLoop = padStatus !== 0 &&
                        engine.getValue(group, "loop_start_position") === engine.getValue(group, `${cue}_position`) &&
                        engine.getValue(group, "loop_end_position") === engine.getValue(group, `${cue}_endposition`);
                    if (padStatus === 2) {
                        exitLoop();
                    }
                    pulse(`${cue}_clear`);
                    if (isCurrentLoop) {
                        pulse("loop_remove");
                    }
                } else if (padStatus === 0) {
                    if (engine.getValue(group, "loop_enabled")) {
                        pulse(`${cue}_setloop`); // save the playing loop here
                        theMode.pendingPad = 0;
                    } else if (theMode.pendingPad !== this.number) {
                        pulse("loop_in"); // 1st press: loop in point
                        theMode.pendingPad = this.number;
                        this.send(SCLive4.rgbCode.white); // waiting for Loop Out
                    } else {
                        pulse("loop_out"); // 2nd press: loop out, loop starts
                        pulse(`${cue}_setloop`);
                        theMode.pendingPad = 0;
                    }
                } else if (padStatus === 2) {
                    exitLoop();
                } else {
                    exitLoop();
                    pulse(`${cue}_gotoandloop`);
                }
            },
            midi: [0x94 + offset, 0x0E + i],
            // Empty = off, saved = dim loop color, playing = full loop color.
            // Active loops pulse (dim / off) while not playing.
            output: function(padStatus) {
                if (padStatus === 0 || (padStatus === 1 && deck.activeLoops[this.number] && theMode.pulseOff)) {
                    this.send(0x00);
                } else {
                    this.outputColor(engine.getValue(this.group, this.colorKey));
                }
            },
            sendRGB: function(colorObj) {
                const playing = engine.getValue(this.group, this.outKey) === 2;
                midi.sendShortMsg(0x94 + offset, 0x0E + i, SCLive4.paletteFromRGB(colorObj, !playing));
            },
            on: this.colorOn,
            off: 0x00,
            outConnect: false,
        });
    }
};
SCLive4.savedLoopMode.prototype = Object.create(components.ComponentContainer.prototype);

// AUTOLOOP MODE
SCLive4.autoloopMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = SCLive4.padMode.LOOP;
    this.colorOn = SCLive4.rgbCode.green;
    this.colorOff = SCLive4.rgbCode.whiteDark;
    this.pads = new components.ComponentContainer();
    // Loop lengths in beats; Parameter < / > halve or double them.
    this.sizes = [0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8];
    this.setPadKeys = function(pad, size) {
        pad.outKey = `beatloop_${size}_enabled`;
        pad.inKey = `beatloop_${size}_toggle`;
    };
    // Parameter < / > halve / double the lengths; Shift + Parameter moves the
    // active loop (SC Live 4 manual).
    this.onParameter = function(direction, shifted) {
        if (shifted) {
            SCLive4.moveLoop(deck.currentDeck, direction);
        } else {
            SCLive4.scalePadLengths(this, direction);
            SCLive4.showLengthMeter(deck, this);
        }
    };
    for (let i = 1; i <= 8; i++) {
        const loopSize = (this.sizes[i - 1]);
        this.pads[i] = new components.Button({
            midi: [0x94 + offset, 0x0E + i],
            group: deck.currentDeck,
            outKey: `beatloop_${loopSize}_enabled`,
            inKey: `beatloop_${loopSize}_toggle`,
            on: SCLive4.rgbCode.white,
            off: SCLive4.rgbCode.greenDark,
            outConnect: false,
        });
    }
};
SCLive4.autoloopMode.prototype = Object.create(components.ComponentContainer.prototype);

// PARAMETER < / > (SC Live 4): halve or double the lengths on the current pad
// mode, like the standalone firmware. Works in Roll (layer 1) and Loop
// (layer 2, auto loops); other pad modes ignore it. Lengths stay within
// Mixxx's range of 1/32 to 64 beats.
SCLive4.scalePadLengths = function(mode, direction) {
    if (!mode || !mode.sizes) {
        return;
    }
    const factor = direction > 0 ? 2 : 0.5;
    const newSizes = mode.sizes.map(size => size * factor);
    if (newSizes[0] < 1 / 32 || newSizes[newSizes.length - 1] > 64) {
        return;
    }
    mode.sizes = newSizes;
    for (let i = 1; i <= 8; i++) {
        const pad = mode.pads[i];
        const active = pad.outConnect;
        if (active) {
            pad.disconnect();
        }
        mode.setPadKeys(pad, newSizes[i - 1]);
        if (active) {
            pad.connect();
            pad.trigger();
        }
    }
};

// Move the active loop left (-1) or right (+1) by the beat jump size, like
// Mixxx's own loop-move buttons (SC Live 4: Shift + loop knob, and Shift +
// Parameter in Auto Loop mode).
SCLive4.moveLoop = function(group, direction) {
    if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "loop_move", direction * engine.getValue(group, "beatjump_size"));
    }
};

// Orange pad meter for the Parameter length scaling, based on pad 1's length:
// 1 pad = 1/32 beat, 2 = 1/16, 3 = 1/8 ... (each pad doubles). Defaults: Roll
// (pad 1 = 1/8) shows 3 pads, Loop layer 2 (pad 1 = 1/16) shows 2.
SCLive4.showLengthMeter = function(deck, mode) {
    if (mode && mode.sizes) {
        deck.padGrid.showMeter(Math.log2(mode.sizes[0] * 32) + 1, SCLive4.rgbCode.orange);
    }
};

// ROLL MODE
SCLive4.rollMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    const theMode = this;
    const group = deck.currentDeck;
    this.ledControl = SCLive4.padMode.ROLL;
    this.pads = new components.ComponentContainer();
    // Roll lengths in beats, as in the SC Live 4 manual (v5.0.0):
    //   1/8, 1/4T, 1/4, 1/2T, 1/2, 1T, 1, 2   (T = triplet, 2/3 of the length)
    // Triplet pads are purple, the others green; a held pad is white.
    // Parameter < / > halve or double all eight lengths.
    this.sizes = [1 / 8, 1 / 6, 1 / 4, 1 / 3, 1 / 2, 2 / 3, 1, 2];
    const triplet = [false, true, false, true, false, true, false, false];
    this.setPadKeys = function() {}; // lengths are read from this.sizes on each press
    this.heldPad = 0;

    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new components.Button({
            midi: [0x94 + offset, 0x0E + i],
            group: group,
            outConnect: false,
            // Mixxx only has ready-made roll controls for whole divisions, so
            // set the roll length directly, roll while held, then put the
            // user's loop size back.
            input: function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status)) {
                    if (theMode.heldPad) {
                        engine.setValue(group, "beatlooproll_activate", 0);
                    } else {
                        theMode.savedLoopSize = engine.getValue(group, "beatloop_size");
                    }
                    theMode.heldPad = i;
                    engine.setValue(group, "beatloop_size", theMode.sizes[i - 1]);
                    engine.setValue(group, "beatlooproll_activate", 1);
                } else if (theMode.heldPad === i) {
                    theMode.heldPad = 0;
                    engine.setValue(group, "beatlooproll_activate", 0);
                    engine.setValue(group, "beatloop_size", theMode.savedLoopSize);
                }
                theMode.pads.forEachComponent(pad => pad.trigger());
            },
            trigger: function() {
                if (!this.outConnect) {
                    return;
                }
                const color = theMode.heldPad === i ? SCLive4.rgbCode.white
                    : triplet[i - 1] ? 0x23 : SCLive4.rgbCode.green;
                this.send(color);
            },
        });
    }
};
SCLive4.rollMode.prototype = Object.create(components.ComponentContainer.prototype);

// SETTINGS (SHIFT + HOT CUE): Engine OS Control Center items that the SC Live 4
// has no dedicated controls for. Shared by both decks' settings pages.
// Values set here last until Mixxx restarts.
SCLive4.settings = {
    // Crossfader contour: smooth fade (mixing) ... sharp cut (scratching).
    // Mixxx's crossfader curve ("transform", 0.6 - 1000, as in Preferences).
    xfaderContour: {
        color: 0x3C, // yellow
        transforms: [0.6, 1, 1.5, 2.5, 5, 15, 60, 1000],
        index: 1,
        level: function() {
            return this.index + 1;
        },
        change: function(direction) {
            this.index = Math.max(0, Math.min(this.transforms.length - 1, this.index + direction));
            const transform = this.transforms[this.index];
            engine.setValue("[Mixer Profile]", "xFaderCurve", transform);
            engine.setValue("[Mixer Profile]", "xFaderCalibration", Math.pow(0.5, 1 / transform));
        },
        sync: function() {
            // Start from Mixxx's current curve (set in Preferences)
            const transform = engine.getValue("[Mixer Profile]", "xFaderCurve");
            let best = 0;
            this.transforms.forEach((t, i) => {
                if (Math.abs(Math.log(t) - Math.log(transform)) < Math.abs(Math.log(this.transforms[best]) - Math.log(transform))) {
                    best = i;
                }
            });
            this.index = best;
        },
    },
    // Stop time: how long a playing deck takes to stop when Play is pressed.
    // 1 pad = instant ... 8 pads = longest (about 2 seconds). Values are
    // Mixxx brake factors (smaller = slower stop); 0 = instant.
    stopTime: {
        color: 0x30, // red
        factors: [0, 8, 4, 2.5, 1.5, 1, 0.7, 0.5],
        index: 0,
        level: function() {
            return this.index + 1;
        },
        change: function(direction) {
            this.index = Math.max(0, Math.min(this.factors.length - 1, this.index + direction));
        },
    },
    // Sampler volume: all 8 sampler decks, 1 pad = quietest ... 8 = full.
    samplerVolume: {
        color: 0x03, // blue
        level: function() {
            return Math.max(1, Math.round(engine.getValue("[Sampler1]", "volume") * 8));
        },
        change: function(direction) {
            const level = Math.max(1, Math.min(8, this.level() + direction));
            for (let i = 1; i <= 8; i++) {
                engine.setValue(`[Sampler${i}]`, "volume", level / 8);
            }
        },
    },
};
// Fader Echo: on/off. Level 1 = off, 8 = on.
SCLive4.settings.faderEcho = {
    color: SCLive4.rgbCode.teal,
    on: false,
    level: function() {
        return this.on ? 8 : 1;
    },
    change: function(direction) {
        this.on = direction > 0;
    },
};
SCLive4.settingsOrder = ["xfaderContour", "stopTime", "samplerVolume", "faderEcho"]; // pads 1-4

// FADER ECHO ENGINE. Mixxx applies effects before the channel fader and the
// crossfader, so a plain echo would be silenced with the fader. Instead:
//  - CHANNEL FADER: while a playing channel's fader is pulled down, its dry
//    sound crossfades into its echo (Effect Unit 3) and its level tapers to
//    half; at zero only the echo remains.
//  - CROSSFADER: moving the crossfader all the way to one side echoes out the
//    playing decks assigned to the other side (they are set to Thru during
//    the tail, then put back).
// Either way the echo is fed for one more beat and the level then tapers
// smoothly to silence over faderEchoTailBars, after which the channel follows
// its fader / crossfader assignment again. Moving the fader back up cancels.
SCLive4.faderEcho = {
    unit: "[EffectRack1_EffectUnit3]",
    slot: "[EffectRack1_EffectUnit3_Effect1]",
    channels: [],     // channels currently echoing
    source: null,     // "fader" or "crossfader"
    ringing: false,   // tail ringing out
    timers: [],
    playLevel: [],    // per channel: fader level before the pull-down began
    savedOrientation: [],
    init: function() {
        engine.setValue(this.unit, "loaded_chain_preset", faderEchoChainPreset);
        engine.setValue(this.unit, "enabled", 1);
        engine.setValue(this.slot, "enabled", 0);
        this.route([]);
    },
    route: function(chs) {
        for (let i = 1; i <= 4; i++) {
            engine.setValue(this.unit, `group_[Channel${i}]_enable`, chs.includes(i) ? 1 : 0);
        }
    },
    stopTimers: function() {
        this.timers.forEach(t => engine.stopTimer(t));
        this.timers = [];
        if (this.rampTimer) {
            engine.stopTimer(this.rampTimer);
            this.rampTimer = 0;
        }
    },
    // Start the echo on these channels (dry/wet set by the caller)
    engage: function(chs, source) {
        this.channels = chs;
        this.source = source;
        this.ringing = false;
        this.stopTimers();
        this.route(chs);
        // With Custom Fader Echo, the effect keeps its own settings
        if (!faderEchoCustom) {
            const delayBeats = 0.5;
            // Fade to about -60 dB within the time left after the 1-beat feed
            const repeats = Math.max(1, (faderEchoTailBars * 4 - 1) / delayBeats);
            engine.setValue(this.slot, "parameter1", delayBeats);                   // delay
            engine.setValue(this.slot, "parameter2", Math.pow(0.001, 1 / repeats)); // feedback
            engine.setValue(this.slot, "parameter3", 0);                            // no ping-pong
            // Mixxx's built-in Echo links "send" to the effect's main knob,
            // which starts at 0: turn it fully up so the whole track feeds the
            // echo.
            engine.setParameter(this.slot, "meta", 1);
            engine.setValue(this.slot, "parameter4", 1);                            // send
        }
        engine.setValue(this.slot, "enabled", 1);
    },
    // Channel level during a fader pull-down: playing level at the top, half
    // of it at zero, so the fade starts as you pull and the echo keeps going.
    pullLevel: function(ch, faderValue) {
        const level = this.playLevel[ch];
        return level * (0.5 + 0.5 * Math.max(0, Math.min(1, faderValue / level)));
    },
    // CHANNEL FADER hook: returns true if the move was taken over by the echo.
    fader: function(ch, newValue, oldValue) {
        const group = `[Channel${ch}]`;
        if (this.source === "fader" && this.channels.includes(ch)) {
            const level = this.playLevel[ch];
            if (newValue >= level * 0.95 || (this.ringing && newValue > 0.02)) {
                this.finish(); // fader back up: cancel
                engine.setParameter(group, "volume", newValue);
                return true;
            }
            if (!this.ringing) {
                engine.setValue(this.unit, "mix", Math.min(1, 1 - newValue / level));
                engine.setParameter(group, "volume", this.pullLevel(ch, newValue));
                if (newValue <= 0.02) {
                    this.startTail();
                }
            }
            return true;
        }
        if (newValue >= oldValue) {
            this.playLevel[ch] = newValue; // remember the playing level
            return false;
        }
        if (!SCLive4.settings.faderEcho.on || this.channels.length || !engine.getValue(group, "play") ||
                !(this.playLevel[ch] > 0.1) || newValue >= this.playLevel[ch] * 0.95) {
            return false;
        }
        this.engage([ch], "fader");
        engine.setValue(this.unit, "mix", Math.min(1, 1 - newValue / this.playLevel[ch]));
        engine.setParameter(group, "volume", this.pullLevel(ch, newValue));
        if (newValue <= 0.02) {
            this.startTail();
        }
        return true;
    },
    // CROSSFADER hook: called after the crossfader moved (-1 left ... 1 right)
    crossfader: function(position, oldPosition) {
        if (!SCLive4.settings.faderEcho.on || this.channels.length) {
            return;
        }
        let leavingSide = null; // crossfader orientation: 0 = left, 2 = right
        if (position <= -0.98 && oldPosition > -0.98) {
            leavingSide = 2;
        } else if (position >= 0.98 && oldPosition < 0.98) {
            leavingSide = 0;
        }
        if (leavingSide === null) {
            return;
        }
        const chs = [];
        for (let ch = 1; ch <= 4; ch++) {
            const group = `[Channel${ch}]`;
            if (engine.getValue(group, "orientation") === leavingSide && engine.getValue(group, "play") &&
                    engine.getParameter(group, "volume") > 0.1) {
                chs.push(ch);
            }
        }
        if (!chs.length) {
            return;
        }
        this.engage(chs, "crossfader");
        chs.forEach(ch => {
            const group = `[Channel${ch}]`;
            this.savedOrientation[ch] = leavingSide;
            this.playLevel[ch] = engine.getParameter(group, "volume");
            engine.setParameter(group, "volume", this.playLevel[ch] * 0.5);
            engine.setValue(group, "orientation", 1); // Thru, past the crossfader
        });
        this.startTail();
    },
    startTail: function() {
        this.ringing = true;
        engine.setValue(this.unit, "mix", 1); // echo only
        const bpm = engine.getValue(`[Channel${this.channels[0]}]`, "bpm") || 120;
        const beatMs = 60000 / bpm;
        // The track itself fades out over one beat by turning down the
        // channel's input gain (before the echo), so only the echo is left.
        // The echo's stored repeats keep ringing out on their own, fading over
        // faderEchoTailBars. The channel level is only eased down over the last
        // quarter of that time, so the echo doesn't end abruptly.
        this.savedGain = this.savedGain || [];
        this.channels.forEach(ch => {
            this.savedGain[ch] = engine.getValue(`[Channel${ch}]`, "pregain");
        });
        const totalMs = beatMs * faderEchoTailBars * 4;
        const startLevels = this.channels.map(ch => engine.getParameter(`[Channel${ch}]`, "volume"));
        const stepMs = 40;
        const steps = Math.max(8, Math.round(totalMs / stepMs));
        const feedSteps = Math.max(1, Math.round(beatMs / stepMs));
        const fadeFrom = Math.round(steps * 0.75);
        let step = 0;
        this.rampTimer = engine.beginTimer(stepMs, () => {
            step++;
            this.channels.forEach((ch, i) => {
                const group = `[Channel${ch}]`;
                if (step <= feedSteps) {
                    const left = 1 - step / feedSteps;
                    engine.setValue(group, "pregain", this.savedGain[ch] * left * left);
                }
                if (step > fadeFrom) {
                    const left = 1 - (step - fadeFrom) / (steps - fadeFrom);
                    engine.setParameter(group, "volume", startLevels[i] * left * left);
                }
            });
            if (step >= steps) {
                this.finish();
            }
        });
    },
    // End the echo: channels go back to their fader position (and crossfader
    // assignment), then the echo switches off.
    finish: function() {
        this.stopTimers();
        const mixers = [SCLive4.mixerA, SCLive4.mixerB, SCLive4.mixerC, SCLive4.mixerD];
        this.channels.forEach(ch => {
            const group = `[Channel${ch}]`;
            const fader = mixers[ch - 1] && mixers[ch - 1].volumeFader.lastValue;
            engine.setParameter(group, "volume", fader !== undefined ? fader : 0);
            if (this.savedGain && this.savedGain[ch] !== undefined) {
                engine.setValue(group, "pregain", this.savedGain[ch]); // input gain back
                this.savedGain[ch] = undefined;
            }
            if (this.source === "crossfader") {
                engine.setValue(group, "orientation", this.savedOrientation[ch]);
            }
        });
        engine.setValue(this.slot, "enabled", 0);
        engine.setValue(this.unit, "mix", 1);
        this.route([]);
        this.channels = [];
        this.source = null;
        this.ringing = false;
    },
};

// Crossfader: goes through the script so Fader Echo can react to it.
SCLive4.crossfader = new components.Pot({
    group: "[Master]",
    inKey: "crossfader",
    softTakeover: false,
    input: function(channel, control, value, status, group) {
        const oldPosition = engine.getValue("[Master]", "crossfader");
        components.Pot.prototype.input.call(this, channel, control, value, status, group);
        SCLive4.faderEcho.crossfader(engine.getValue("[Master]", "crossfader"), oldPosition);
    },
});

// SETTINGS PAGE (pad mode): pads choose a setting (its own color; bright =
// selected, dim = others), PARAMETER < / > change it, and the new value shows
// briefly as a pad meter in that color. Press any mode button to leave.
SCLive4.settingsMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    const theMode = this;
    this.ledControl = SCLive4.padMode.HOTCUE;
    this.selected = 0;
    this.pads = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new components.Button({
            midi: [0x94 + offset, 0x0E + i],
            outConnect: false,
            input: function(channel, control, value, status, _group) {
                if (this.isPress(channel, control, value, status) && i <= SCLive4.settingsOrder.length) {
                    theMode.selected = i - 1;
                    theMode.pads.forEachComponent(pad => pad.trigger());
                    theMode.showValue();
                }
            },
            trigger: function() {
                if (!this.outConnect || deck.padGrid.meterActive) {
                    return;
                }
                const name = SCLive4.settingsOrder[i - 1];
                if (!name) {
                    this.send(0x00);
                    return;
                }
                const color = SCLive4.settings[name].color;
                this.send(i - 1 === theMode.selected ? color : SCLive4.dimPalette(color));
            },
        });
    }
    this.showValue = function() {
        const setting = SCLive4.settings[SCLive4.settingsOrder[this.selected]];
        deck.padGrid.showMeter(setting.level(), setting.color);
    };
    this.onEnter = function() {
        SCLive4.settings.xfaderContour.sync();
    };
    this.onParameter = function(direction) {
        SCLive4.settings[SCLive4.settingsOrder[this.selected]].change(direction);
        this.showValue();
    };
};
SCLive4.settingsMode.prototype = Object.create(components.ComponentContainer.prototype);

// SAMPLER MODE
//
// HOW TO USE (differs from Engine OS, which loads samples into its own sampler
// slots through the unit's menus):
//   - Open it: press ROLL twice (second layer of Roll), or SHIFT + ROLL.
//   - Pads 1-8 play Mixxx's sampler decks 1-8.
//   - Load a sample FROM THE UNIT: turn the browse knob to highlight any track
//     in the Mixxx library, then press an EMPTY pad. The highlighted track is
//     loaded into that sampler deck. (You can also drag tracks onto Mixxx's
//     on-screen samplers.)
//   - Play: press a loaded pad. It plays from the start each time it's pressed.
//   - SHIFT + pad: stops the sample if it's playing; if it isn't, ejects it.
//   - Pad lights: off = empty, dim color = loaded, full color = playing.
//     Colors by pad (as in standalone): yellow, orange, pink, red/fuchsia,
//     light yellow/white, light green, teal, blue.
//   - Mixxx remembers what's loaded in its sampler decks between sessions
//     (samplers.xml in the Mixxx settings folder).
SCLive4.samplerMode = function(deck, offset) {
    components.ComponentContainer.call(this);
    this.ledControl = SCLive4.padMode.ROLL;
    this.colorOn = SCLive4.rgbCode.green;
    this.colorOff = SCLive4.rgbCode.whiteDark;
    this.pads = new components.ComponentContainer();
    // Pad colors as on the SC Live 4 in standalone Sampler mode (palette:
    // 2 bits each of red, green, blue): yellow, orange, pink, red/fuchsia,
    // light yellow/white, light green, teal, blue.
    const colorArray = [0x3C, 0x38, 0x36, 0x31, 0x3E, 0x2D, SCLive4.rgbCode.teal, SCLive4.rgbCode.blue];
    for (let i = 1; i <= 8; i++) {
        // Empty = off, loaded = dim color, playing = full color.
        this.pads[i] = new components.SamplerButton({
            number: i,
            midi: [0x94 + offset, 0x0E + i],
            on: colorArray[i - 1],
            loaded: SCLive4.dimPalette(colorArray[i - 1]),
            playing: colorArray[i - 1],
            off: 0x00,
            outConnect: false,
        });
    }
};
SCLive4.samplerMode.prototype = Object.create(components.ComponentContainer.prototype);

// SLICER MODES (Slicer button: layer 1 = Slicer, layer 2 = Slicer Loop)
//   The 8 pads are 8 one-beat slices, starting at the beat you're on when you
//   enter the mode. The lit pad follows the slice that is playing.
//   - Slicer: the 8-beat window moves on every 8 beats as the track plays.
//     Hold a pad to jump to that slice and repeat it (1-beat roll); release to
//     return to where the track would have been (slip), like a roll.
//   - Slicer Loop: the slices become a loop (the "domain"); pads jump between
//     slices inside it, landing at the same point within the quantize unit so
//     the groove stays in time. The loop is fixed and repeats until you leave
//     the mode: press SLICER again (back to Slicer, which moves with the
//     song) or any other pad mode to end it and keep playing through the track.
//     PARAMETER < / > = quantize size (1/8, 1/4, 1/2, 1 beat; default 1).
//     SHIFT + PARAMETER < / > = domain size (4, 8, 16, 32 beats; default 8),
//     so a 16-beat domain has 2-beat slices. Both reset to the defaults when
//     the mapping reloads.
//     The loop is a standard Mixxx beat loop, so it shows on the waveform and
//     Mixxx's loop size display shows the domain; your previous loop size is
//     restored when you leave Slicer Loop.
//   Lights: slices dim blue (Slicer) or dim purple (Slicer Loop); white = the
//   slice now playing (or held). In Slicer Loop, pressing PARAMETER shows the
//   new size for a second as a pad meter: yellow = quantize (1 pad = 1/8 beat,
//   4 pads = 1 beat), cyan = domain (1 pad = 4 beats, 4 pads = 32 beats).
SCLive4.slicerMode = function(deck, offset, looping) {
    components.ComponentContainer.call(this);
    const theMode = this;
    const group = deck.currentDeck;
    this.ledControl = SCLive4.padMode.SLICER;
    this.windowStart = null; // engine sample position of slice 1
    // Slicer Loop settings (see onParameter): loop length in beats, and the
    // grid pad jumps stay in time with, in beats.
    this.domainBeats = 8;
    this.quantizeBeats = 1;
    this.heldPad = 0;
    this.slipWasOn = false;

    const beatLength = () => engine.getValue(group, "beat_next") - engine.getValue(group, "beat_prev");
    const sliceLength = () => beatLength() * theMode.domainBeats / 8;
    const trackSamples = () => engine.getValue(group, "track_samples");
    const position = () => engine.getValue(group, "playposition") * trackSamples();
    const isVisible = () => SCLive4.leftDeck === deck || SCLive4.rightDeck === deck;
    const pulse = key => {
        engine.setValue(group, key, 1);
        engine.setValue(group, key, 0);
    };

    this.currentSlice = function() {
        const len = sliceLength();
        if (this.windowStart === null || len <= 0) {
            return -1;
        }
        return Math.floor((position() - this.windowStart) / len + 1e-6);
    };

    this.startWindow = function() {
        const prev = engine.getValue(group, "beat_prev");
        const len = beatLength();
        if (!engine.getValue(group, "track_loaded") || prev < 0 || len <= 0) {
            this.windowStart = null;
            return;
        }
        if (looping) {
            // Use Mixxx's standard beat loop (drawn on the waveform, sized by
            // beatloop_size): starts on the nearest beat when quantize is on.
            this.savedLoopSize = engine.getValue(group, "beatloop_size");
            engine.setValue(group, "beatloop_size", this.domainBeats);
            if (!engine.getValue(group, "loop_enabled")) {
                pulse("beatloop_activate");
            }
            this.windowStart = engine.getValue(group, "loop_start_position");
            if (this.windowStart < 0) {
                this.windowStart = null;
            }
        } else {
            this.windowStart = prev;
        }
    };

    // Slicer Loop only (the manual describes these for Slicer Loop):
    //   PARAMETER < / >          quantize size 1/8, 1/4, 1/2, 1 beat
    //   SHIFT + PARAMETER < / >  domain (loop) size 4, 8, 16, 32 beats
    if (looping) {
        this.onParameter = function(direction, shifted) {
            const factor = direction > 0 ? 2 : 0.5;
            if (shifted) {
                const domain = this.domainBeats * factor;
                if (domain >= 4 && domain <= 32) {
                    this.domainBeats = domain;
                    if (this.windowStart !== null) {
                        // Mixxx resizes the active beat loop from its anchor
                        engine.setValue(group, "beatloop_size", domain);
                        this.windowStart = engine.getValue(group, "loop_start_position");
                    }
                }
                // Cyan meter: 1 pad = 4 beats ... 4 pads = 32 beats
                this.showSetting(Math.log2(this.domainBeats) - 1, SCLive4.rgbCode.cyan);
            } else {
                const quantize = this.quantizeBeats * factor;
                if (quantize >= 1 / 8 && quantize <= 1) {
                    this.quantizeBeats = quantize;
                }
                // Yellow meter: 1 pad = 1/8 beat ... 4 pads = 1 beat
                this.showSetting(Math.log2(this.quantizeBeats) + 4, SCLive4.rgbCode.yellow);
            }
        };
    }

    // Slices are dim blue in Slicer and dim purple in Slicer Loop, so the two
    // layers can be told apart; the playing slice is white in both.
    const sliceColor = looping ? 0x11 : SCLive4.rgbCode.blueDark;

    this.paint = function() {
        if (!this.active || !isVisible() || deck.padGrid.meterActive) {
            return;
        }
        const playing = this.heldPad ? this.heldPad - 1 : this.currentSlice();
        for (let i = 1; i <= 8; i++) {
            midi.sendShortMsg(0x94 + offset, 0x0E + i, i - 1 === playing ? SCLive4.rgbCode.white : sliceColor);
        }
    };

    this.showSetting = function(level, color) {
        if (this.active) {
            deck.padGrid.showMeter(level, color);
        }
    };

    // Called on every beat while the mode is active
    this.onBeat = function() {
        if (this.windowStart === null) {
            this.startWindow();
        }
        if (!looping && !this.heldPad) {
            const slice = this.currentSlice();
            if (slice >= 8 || slice < 0) {
                this.windowStart += Math.floor(slice / 8) * 8 * beatLength();
            }
        }
        this.paint();
    };

    this.onEnter = function() {
        this.active = true;
        this.startWindow();
        this.beatConnection = engine.makeConnection(group, "beat_active", () => theMode.onBeat());
        this.paint();
    };

    this.onExit = function() {
        this.active = false;
        if (this.beatConnection) {
            this.beatConnection.disconnect();
            this.beatConnection = undefined;
        }
        if (looping) {
            if (engine.getValue(group, "loop_enabled")) {
                pulse("reloop_toggle");
            }
            if (this.savedLoopSize) {
                engine.setValue(group, "beatloop_size", this.savedLoopSize);
            }
        }
        this.windowStart = null;
    };

    this.pads = new components.ComponentContainer();
    for (let i = 1; i <= 8; i++) {
        this.pads[i] = new components.Button({
            midi: [0x94 + offset, 0x0E + i],
            group: group,
            outConnect: false,
            input: function(channel, control, value, status, _group) {
                const pressed = this.isPress(channel, control, value, status);
                if (pressed) {
                    if (theMode.windowStart === null) {
                        theMode.startWindow();
                    }
                    if (theMode.windowStart === null) {
                        return; // no track or no beatgrid
                    }
                    const target = theMode.windowStart + (i - 1) * sliceLength();
                    if (looping) {
                        // Keep the groove: land at the same point within the
                        // current quantize unit.
                        const unit = theMode.quantizeBeats * beatLength();
                        const phase = ((position() - theMode.windowStart) % unit + unit) % unit;
                        engine.setValue(group, "playposition", (target + phase) / trackSamples());
                    } else {
                        theMode.slipWasOn = engine.getValue(group, "slip_enabled") === 1;
                        engine.setValue(group, "slip_enabled", 1);
                        engine.setValue(group, "playposition", target / trackSamples());
                        engine.setValue(group, "beatlooproll_1_activate", 1);
                        theMode.heldPad = i;
                    }
                } else if (!looping && theMode.heldPad === i) {
                    engine.setValue(group, "beatlooproll_1_activate", 0);
                    engine.setValue(group, "slip_enabled", theMode.slipWasOn ? 1 : 0);
                    theMode.heldPad = 0;
                }
                theMode.paint();
            },
            trigger: function() {
                theMode.paint();
            },
        });
    }
};
SCLive4.slicerMode.prototype = Object.create(components.ComponentContainer.prototype);
