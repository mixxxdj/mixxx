// Pioneer DDJ-1000 mapping for Mixxx.
//
// Addresses come from AlphaTheta's "DDJ-1000 List of MIDI message version
// 1.00". Button behaviour follows the DDJ-1000 Operating Instructions and the
// functions rekordbox assigns in its own DDJ-1000 mapping table.
//
// Hardware notes:
//   * Mixxx opens the MIDI output only after init() returns, so the opening
//     state is sent from the first timer tick instead of from init().
//   * HEADPHONES LEVEL/MIX, MASTER LEVEL, BOOTH LEVEL, MASTER CUE and the MIC
//     section are analogue. The unit blends Main (outputs 1-2) with the cue
//     mix (outputs 3-4) itself.
//   * Deck controls arrive on MIDI channels 1-4 for decks 1-4; the unit
//     switches channel itself when DECK 1/3 or 2/4 is pressed.
//   * The unit sends LOOP 1/2X and 2X instead of LOOP IN/OUT once it is told a
//     loop is running (note 0x04+deck on MIDI channel 16).
//   * Pad LEDs take a colour number on a hue wheel: 1 blue, 21 green,
//     29 yellow, 41 red, 61 purple; 127 is near-white and 0 dims the pad.
//     (Palette first charted for the DDJ-FLX10 by Veezuhz.)

// eslint-disable-next-line no-var
var PioneerDDJ1000 = {};

// ---------------------------------------------------------------------------
// Tuning
// ---------------------------------------------------------------------------

PioneerDDJ1000.config = {
    // Platter resolution: about 12900 ticks per revolution (measured by
    // ntamas94 on the hardware).
    jogTicksPerRevolution: 12900,
    vinylRpm: 33 + 1 / 3,
    scratchAlpha: 1 / 8,
    scratchBeta: 1 / 8 / 32,
    // Pitch bend: Mixxx "jog" units per full turn of the outer ring.
    bendPerRevolution: 200,
    // A released platter keeps driving the record while it turns faster than
    // this fraction of playing speed.
    freeSpinMinSpeed: 0.2,
    // Seconds of track per revolution when searching with SHIFT + jog.
    searchSecondsPerRevolution: 20,
    // Seconds per revolution when moving a loop point with the jog.
    loopAdjustSecondsPerRevolution: 1,
    // SEARCH buttons held: speed doubles every second up to the maximum.
    searchStartRate: 2,
    searchMaxRate: 24,
    tempoRanges: [0.06, 0.10, 0.16, 1.00],
    doublePressMs: 400,
    slipReverseBeats: 8,
    samplerBanks: 8,
    beatValues: [1 / 16, 1 / 8, 1 / 4, 1 / 2, 3 / 4, 1, 2, 4, 8, 16, 32],
    defaultBeatIndex: 5,
    displayIntervalMs: 40,
    blinkMs: 250,
};

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

PioneerDDJ1000.color = {
    off: 0, blue: 1, azure: 9, cyan: 17, green: 21, lime: 25, yellow: 29,
    amber: 33, orange: 37, red: 41, rose: 45, pink: 53, magenta: 57,
    purple: 61, white: 127,
};

// Hue of each palette step, used to build the ColorMapper for hot cue colours.
PioneerDDJ1000.paletteHues = [
    [1, 240], [5, 228], [9, 216], [13, 200], [17, 180], [21, 120], [25, 85],
    [29, 60], [33, 45], [37, 30], [41, 0], [45, 350], [49, 340], [53, 330],
    [57, 300], [61, 275],
];

PioneerDDJ1000.mode = {
    hotCue: 0, padFx1: 1, beatJump: 2, sampler: 3,
    keyboard: 4, padFx2: 5, beatLoop: 6, keyShift: 7,
};

PioneerDDJ1000.modeButtons = [0x1B, 0x1E, 0x20, 0x22, 0x69, 0x6B, 0x6D, 0x6F];
PioneerDDJ1000.pagePrevNotes = [0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B];
PioneerDDJ1000.pageNextNotes = [0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33];

// Pads 0-3 are the top row, 4-7 the bottom row.
PioneerDDJ1000.beatJumpBeats = [-1, 1, -2, 2, -4, 4, -8, 8];
PioneerDDJ1000.beatLoopBeats = [
    [1 / 4, 1 / 2, 1, 2, 4, 8, 16, 32],
    [1 / 32, 1 / 16, 1 / 8, 1 / 4, 1 / 2, 1, 2, 4],
];
PioneerDDJ1000.semitones = [
    [4, 5, 6, 7, 0, 1, 2, 3],
    [-4, -3, -2, -1, -8, -7, -6, -5],
];

// PAD FX pads. "slot" holds one effect of an effect unit on the deck, "unit"
// holds the whole unit; both hand the unit back as it was on release.
PioneerDDJ1000.padFx = (function() {
    const c = PioneerDDJ1000.color;
    const unitRow = (unit, colour) => [
        {unit: unit, slot: 1, color: colour},
        {unit: unit, slot: 2, color: colour},
        {unit: unit, slot: 3, color: colour},
        {unit: unit, color: c.purple},
    ];
    const rolls = (beats) => beats.map((b) => ({roll: b, color: c.green}));
    const tricks = [
        {brake: true, color: c.amber},
        {spinback: true, color: c.amber},
        {reverse: true, color: c.orange},
    ];
    return {
        [PioneerDDJ1000.mode.padFx1]: [
            unitRow(3, c.blue).concat(rolls([1 / 16, 1 / 8, 1 / 4, 1 / 2])),
            unitRow(4, c.azure).concat(rolls([1 / 32, 1, 2, 4])),
        ],
        [PioneerDDJ1000.mode.padFx2]: [
            tricks.concat([{roll: 3 / 4, color: c.lime}], unitRow(4, c.azure)),
            tricks.concat([{roll: 3 / 2, color: c.lime}], unitRow(3, c.blue)),
        ],
    };
})();

// FX SELECT positions 0x20-0x2D. The two roll positions work on the deck's
// loops; every other position loads effect unit 1's chain preset N.
PioneerDDJ1000.beatFxPositions = [
    "LOW CUT ECHO", "ECHO", "MT DELAY", "SPIRAL", "REVERB", "TRANS", "ENIGMA JET",
    "FLANGER", "PHASER", "PITCH", "SLIP ROLL", "ROLL", "MOBIUS SAW", "MOBIUS TRI",
];

PioneerDDJ1000.display = {
    positionBar: 0x14, bpm: 0x15, speed: 0x16, cueMarker: 0x17,
    minutes: 0x42, seconds: 0x43, timeMode: 0x44, key: 0x49, keyShift: 0x4A,
    syncMaster: 0x59, syncOn: 0x5A, ring: 0x5B, info: 0x5D,
};

// Memory cues (MEMORY / CUE/LOOP CALL) use hot cues 17-36; pads use 1-16.
PioneerDDJ1000.memoryFirst = 17;
PioneerDDJ1000.memoryLast = 36;
PioneerDDJ1000.memoryColor = 0xE8201E;

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

PioneerDDJ1000.decks = [1, 2, 3, 4];
PioneerDDJ1000.deck = {};
PioneerDDJ1000.connections = [];
PioneerDDJ1000.timer = 0;
PioneerDDJ1000.opened = false;
PioneerDDJ1000.blink = false;
PioneerDDJ1000.blinkAt = 0;
PioneerDDJ1000.padRefreshAt = 0;
PioneerDDJ1000.padRefreshDeck = 0;
PioneerDDJ1000.sent = {};
PioneerDDJ1000.highRes = {};

PioneerDDJ1000.beatFx = {
    position: 1,
    beatIndex: PioneerDDJ1000.config.defaultBeatIndex,
    target: "master",
    on: false,
    rolling: [],
};

PioneerDDJ1000.colorFx = 0;
PioneerDDJ1000.heldUnits = {};

PioneerDDJ1000.newDeckState = function() {
    return {
        vinyl: true,
        touched: false,
        freeSpin: false,
        spinTicks: [],
        mode: PioneerDDJ1000.mode.hotCue,
        page: [1, 1, 1, 1, 1, 1, 1, 1],
        jumpScale: 1,
        samplerBank: 0,
        keyboardCue: 1,
        pressed: {},
        loopAdjust: null,
        search: null,
        searchHeld: false,
        keySync: false,
        reverseTimer: 0,
        gridMode: null,
        gridRemainder: 0,
        padFxSlip: 0,
        lastLoad: 0,
        loadTimer: 0,
    };
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

PioneerDDJ1000.group = (deck) => `[Channel${deck}]`;

PioneerDDJ1000.deckFromGroup = (group) => script.deckFromGroup(group);

PioneerDDJ1000.padStatus = (deck, shifted) => 0x97 + (deck - 1) * 2 + (shifted ? 1 : 0);

PioneerDDJ1000.clamp = (value, low, high) => Math.max(low, Math.min(high, value));

PioneerDDJ1000.setting = function(name, fallback) {
    const value = engine.getSetting(name);
    return value === undefined ? fallback : value;
};

// Send a MIDI message unless the same value was the last one sent there.
PioneerDDJ1000.send = function(status, data1, data2, force) {
    const key = `${status}:${data1}`;
    const value = data2 & 0x7F;
    if (!force && PioneerDDJ1000.sent[key] === value) {
        return;
    }
    PioneerDDJ1000.sent[key] = value;
    if (PioneerDDJ1000.opened) {
        midi.sendShortMsg(status, data1, value);
    }
};

PioneerDDJ1000.led = function(status, data1, on) {
    PioneerDDJ1000.send(status, data1, on ? 0x7F : 0x00);
};

PioneerDDJ1000.deckLed = function(deck, note, on) {
    PioneerDDJ1000.led(0x90 + deck - 1, note, on);
};

PioneerDDJ1000.send14 = function(status, msb, value) {
    const rounded = Math.round(value) & 0x3FFF;
    const key = `${status}:14:${msb}`;
    if (PioneerDDJ1000.sent[key] === rounded) {
        return;
    }
    PioneerDDJ1000.sent[key] = rounded;
    if (PioneerDDJ1000.opened) {
        midi.sendShortMsg(status, msb, rounded >> 7);
        midi.sendShortMsg(status, msb + 0x20, rounded & 0x7F);
    }
};

// 14-bit knobs arrive as MSB then LSB. Returns 0..1.
PioneerDDJ1000.readHighRes = function(status, control, value) {
    const isLsb = control >= 0x20 && control < 0x40;
    const key = `${status}:${isLsb ? control - 0x20 : control}`;
    const pair = PioneerDDJ1000.highRes[key] || (PioneerDDJ1000.highRes[key] = {msb: 0, lsb: 0});
    pair[isLsb ? "lsb" : "msb"] = value;
    return ((pair.msb << 7) | pair.lsb) / 0x3FFF;
};

// Jog and outer ring count from 0x40; the browse encoder counts from 0.
PioneerDDJ1000.jogDelta = (value) => value - 0x40;
PioneerDDJ1000.encoderDelta = (value) => (value > 0x40 ? value - 0x80 : value);

PioneerDDJ1000.connect = function(group, key, callback) {
    const connection = engine.makeConnection(group, key, callback);
    if (connection) {
        PioneerDDJ1000.connections.push(connection);
    }
    return connection;
};

PioneerDDJ1000.toggle = function(group, key) {
    engine.setValue(group, key, engine.getValue(group, key) ? 0 : 1);
};

// Engine sample positions are interleaved stereo: two samples per frame.
PioneerDDJ1000.samplesPerSecond = (group) => (engine.getValue(group, "track_samplerate") || 44100) * 2;

PioneerDDJ1000.playSamples = (group) =>
    engine.getValue(group, "playposition") * engine.getValue(group, "track_samples");

PioneerDDJ1000.seekSamples = function(group, samples) {
    const total = engine.getValue(group, "track_samples");
    if (total > 0) {
        engine.setValue(group, "playposition", PioneerDDJ1000.clamp(samples / total, 0, 1));
    }
};

PioneerDDJ1000.numSamplers = () => engine.getValue("[App]", "num_samplers") || 16;

PioneerDDJ1000.hueToRgb = function(hue) {
    const x = 1 - Math.abs((hue / 60) % 2 - 1);
    const sector = Math.floor(hue / 60) % 6;
    const rgb = [[1, x, 0], [x, 1, 0], [0, 1, x], [0, x, 1], [x, 0, 1], [1, 0, x]][sector];
    return (Math.round(rgb[0] * 255) << 16) | (Math.round(rgb[1] * 255) << 8) | Math.round(rgb[2] * 255);
};

PioneerDDJ1000.buildColorMapper = function() {
    const palette = {0xFFFFFF: PioneerDDJ1000.color.white};
    PioneerDDJ1000.paletteHues.forEach(([value, hue]) => {
        palette[PioneerDDJ1000.hueToRgb(hue)] = value;
    });
    return new ColorMapper(palette);
};

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

PioneerDDJ1000.init = function() {
    PioneerDDJ1000.colorMapper = PioneerDDJ1000.buildColorMapper();
    const tempoRange = PioneerDDJ1000.setting("tempoRange", "preferences");

    PioneerDDJ1000.decks.forEach((deck) => {
        const group = PioneerDDJ1000.group(deck);
        PioneerDDJ1000.deck[deck] = PioneerDDJ1000.newDeckState();
        if (tempoRange !== "preferences") {
            engine.setValue(group, "rateRange", Number(tempoRange) / 100);
        }
        engine.softTakeover(group, "rate", true);
        PioneerDDJ1000.connectDeck(deck);
    });

    if (PioneerDDJ1000.setting("hardwareHeadphoneMix", true)) {
        // The HEADPHONES MIX knob blends cue and main in hardware, so Mixxx
        // sends the cue mix alone to outputs 3-4.
        engine.setValue("[Master]", "headMix", -1);
    }

    PioneerDDJ1000.connectGlobal();
    PioneerDDJ1000.routeBeatFx();
    PioneerDDJ1000.applyColorFx();
    PioneerDDJ1000.timer = engine.beginTimer(PioneerDDJ1000.config.displayIntervalMs, PioneerDDJ1000.tick);
};

PioneerDDJ1000.shutdown = function() {
    if (PioneerDDJ1000.timer) {
        engine.stopTimer(PioneerDDJ1000.timer);
        PioneerDDJ1000.timer = 0;
    }
    PioneerDDJ1000.connections.forEach((connection) => connection.disconnect());
    PioneerDDJ1000.connections = [];
    if (!PioneerDDJ1000.opened) {
        return;
    }
    Object.keys(PioneerDDJ1000.sent).forEach((key) => {
        const parts = key.split(":").map(Number);
        if (parts.length === 2 && (parts[0] & 0xF0) === 0x90) {
            midi.sendShortMsg(parts[0], parts[1], 0x00);
        }
    });
    PioneerDDJ1000.decks.forEach((deck) => {
        midi.sendShortMsg(0x90 + deck - 1, PioneerDDJ1000.display.info, 0x7F);
        midi.sendShortMsg(0x90 + deck - 1, PioneerDDJ1000.display.ring, 0x00);
    });
};

PioneerDDJ1000.open = function() {
    PioneerDDJ1000.opened = true;
    PioneerDDJ1000.decks.forEach((deck) => {
        midi.sendShortMsg(0x90 + deck - 1, PioneerDDJ1000.display.info, 0x00);
        midi.sendShortMsg(0x90 + deck - 1, PioneerDDJ1000.display.ring, 0x01);
    });
    PioneerDDJ1000.refreshAll();
};

// Forget what was sent and light everything again (also after a deck switch).
PioneerDDJ1000.refreshAll = function() {
    PioneerDDJ1000.sent = {};
    PioneerDDJ1000.connections.forEach((connection) => connection.trigger());
    PioneerDDJ1000.decks.forEach((deck) => {
        PioneerDDJ1000.updateModeLeds(deck);
        PioneerDDJ1000.updateLoopLeds(deck);
        PioneerDDJ1000.deckLed(deck, 0x17, PioneerDDJ1000.deck[deck].vinyl);
        PioneerDDJ1000.deckLed(deck, 0x65, PioneerDDJ1000.deck[deck].keySync);
        PioneerDDJ1000.renderPads(deck, false);
    });
    PioneerDDJ1000.led(0x94, 0x47, PioneerDDJ1000.beatFx.on);
    PioneerDDJ1000.updateColorFxLeds();
};

PioneerDDJ1000.tick = function() {
    if (!PioneerDDJ1000.opened) {
        PioneerDDJ1000.open();
    }
    const now = Date.now();
    if (now - PioneerDDJ1000.blinkAt >= PioneerDDJ1000.config.blinkMs) {
        PioneerDDJ1000.blink = !PioneerDDJ1000.blink;
        PioneerDDJ1000.blinkAt = now;
        PioneerDDJ1000.decks.forEach((deck) => {
            PioneerDDJ1000.updateLoopLeds(deck);
            PioneerDDJ1000.updateModeLeds(deck);
        });
    }
    // Pads of the modes not on screen are refreshed one deck at a time so the
    // unit never receives a burst of pad messages.
    let refreshDeck = 0;
    if (now - PioneerDDJ1000.padRefreshAt >= 250) {
        PioneerDDJ1000.padRefreshAt = now;
        PioneerDDJ1000.padRefreshDeck = PioneerDDJ1000.padRefreshDeck % 4 + 1;
        refreshDeck = PioneerDDJ1000.padRefreshDeck;
    }
    PioneerDDJ1000.decks.forEach((deck) => {
        PioneerDDJ1000.watchFreeSpin(deck, now);
        PioneerDDJ1000.updateSearch(deck, now);
        PioneerDDJ1000.updateDisplay(deck);
        PioneerDDJ1000.renderPads(deck, deck === refreshDeck);
    });
};

// ---------------------------------------------------------------------------
// LEDs driven by Mixxx controls
// ---------------------------------------------------------------------------

PioneerDDJ1000.connectDeck = function(deck) {
    const group = PioneerDDJ1000.group(deck);
    const status = 0x90 + deck - 1;
    const light = function(key, notes, isOn) {
        PioneerDDJ1000.connect(group, key, (value) => {
            notes.forEach((note) => PioneerDDJ1000.led(status, note, isOn ? isOn(value) : value > 0));
        });
    };

    light("play_indicator", [0x0B, 0x47]);
    light("cue_indicator", [0x0C, 0x48]);
    light("sync_enabled", [0x58]);
    light("sync_mode", [0x5C], (value) => value === 2);
    light("keylock", [0x1A, 0x60]);
    light("loop_enabled", [0x14, 0x50]);
    light("quantize", [0x35]);
    light("slip_enabled", [0x40]);
    light("reverse", [0x38]);
    light("reverseroll", [0x15]);
    light("pfl", [0x54]);

    PioneerDDJ1000.connect(group, "loop_enabled", (value) => {
        // Makes the unit send LOOP 1/2X and 2X while the loop runs.
        PioneerDDJ1000.send(0x9F, 0x03 + deck, value > 0 ? 0x7F : 0x00);
        PioneerDDJ1000.updateLoopLeds(deck);
    });
    PioneerDDJ1000.connect(group, "slip_enabled", (value) => {
        PioneerDDJ1000.send(0x9F, 0x23 + deck, value > 0 ? 0x7F : 0x00);
    });
    PioneerDDJ1000.connect(group, "vu_meter", (value) => {
        PioneerDDJ1000.send(0xB0 + deck - 1, 0x02, Math.round(value * 127));
    });
    PioneerDDJ1000.connect(group, "track_loaded", (value) => PioneerDDJ1000.onLoad(deck, value > 0));
};

PioneerDDJ1000.connectGlobal = function() {
    PioneerDDJ1000.connect("[EffectRack1_EffectUnit1]", "enabled", () => {
        PioneerDDJ1000.led(0x94, 0x47, PioneerDDJ1000.beatFx.on);
    });
    for (let i = 1; i <= PioneerDDJ1000.numSamplers(); i++) {
        PioneerDDJ1000.connect(`[Sampler${i}]`, "pfl", PioneerDDJ1000.updateSamplerCueLed);
    }
};

PioneerDDJ1000.updateSamplerCueLed = function() {
    let cued = false;
    for (let i = 1; i <= PioneerDDJ1000.numSamplers() && !cued; i++) {
        cued = engine.getValue(`[Sampler${i}]`, "pfl") > 0;
    }
    PioneerDDJ1000.led(0x96, 0x69, cued);
};

PioneerDDJ1000.onLoad = function(deck, loaded) {
    const state = PioneerDDJ1000.deck[deck];
    state.keySync = false;
    PioneerDDJ1000.deckLed(deck, 0x65, false);
    if (!loaded || !PioneerDDJ1000.opened) {
        return;
    }
    // Plays the jog ring's load animation.
    PioneerDDJ1000.send(0x9F, deck - 1, 0x7F, true);
    if (state.loadTimer) {
        engine.stopTimer(state.loadTimer);
    }
    state.loadTimer = engine.beginTimer(1000, () => {
        state.loadTimer = 0;
        PioneerDDJ1000.send(0x9F, deck - 1, 0x00, true);
    }, true);
};

// ---------------------------------------------------------------------------
// Jog dial display
// ---------------------------------------------------------------------------

PioneerDDJ1000.secondsPerRevolution = () => 60 / PioneerDDJ1000.config.vinylRpm;

// Mixxx key number (1-12 C..B major, 13-24 Cm..Bm) to the unit's key code.
PioneerDDJ1000.keyCode = function(key) {
    const k = Math.round(key);
    if (k >= 1 && k <= 12) {
        return 2 * k - 1;
    }
    if (k >= 13 && k <= 24) {
        // Minor keys sit next to their relative major, three semitones up.
        return 2 * ((k - 13 + 3) % 12) + 2;
    }
    return 0;
};

// Angle of a platter spinning at 33 1/3 after the given number of seconds.
PioneerDDJ1000.platterAngle = function(seconds) {
    const turns = seconds / PioneerDDJ1000.secondsPerRevolution();
    return Math.floor((turns - Math.floor(turns)) * 360) % 360;
};

PioneerDDJ1000.updateDisplay = function(deck) {
    const group = PioneerDDJ1000.group(deck);
    const note = 0x90 + deck - 1;
    const cc = 0xB0 + deck - 1;
    const d = PioneerDDJ1000.display;

    PioneerDDJ1000.send(note, d.syncMaster, engine.getValue(group, "sync_mode") === 2 ? 0x7F : 0);
    PioneerDDJ1000.send(note, d.syncOn, engine.getValue(group, "sync_enabled") ? 0x7F : 0);

    if (!engine.getValue(group, "track_loaded")) {
        PioneerDDJ1000.send(note, d.minutes, 0);
        PioneerDDJ1000.send(note, d.seconds, 0);
        PioneerDDJ1000.send14(cc, d.bpm, 0);
        PioneerDDJ1000.send14(cc, d.cueMarker, 0x3FFF);
        PioneerDDJ1000.send(note, d.key, 0);
        PioneerDDJ1000.send(note, d.keyShift, 0x0D);
        return;
    }

    const duration = engine.getValue(group, "duration");
    const elapsed = PioneerDDJ1000.clamp(engine.getValue(group, "playposition"), 0, 1) * duration;
    const remaining = engine.getValue("[Controls]", "ShowDurationRemaining") !== 0;
    const shown = remaining ? Math.max(0, duration - elapsed) : elapsed;
    PioneerDDJ1000.send(note, d.timeMode, remaining ? 0x7F : 0);
    PioneerDDJ1000.send(note, d.minutes, Math.min(99, Math.floor(shown / 60)));
    PioneerDDJ1000.send(note, d.seconds, Math.floor(shown % 60));
    PioneerDDJ1000.send14(cc, d.positionBar, PioneerDDJ1000.platterAngle(elapsed));

    const cue = engine.getValue(group, "cue_point");
    PioneerDDJ1000.send14(cc, d.cueMarker, cue >= 0
        ? PioneerDDJ1000.platterAngle(cue / PioneerDDJ1000.samplesPerSecond(group))
        : 0x3FFF);

    PioneerDDJ1000.send14(cc, d.bpm, PioneerDDJ1000.clamp(engine.getValue(group, "bpm") * 10, 0, 9999));
    // -100 % .. +100 % spans 0..9999.
    const ratio = engine.getValue(group, "rate_ratio") || 1;
    PioneerDDJ1000.send14(cc, d.speed, PioneerDDJ1000.clamp(ratio * 4999.5, 0, 9999));

    PioneerDDJ1000.send(note, d.key, PioneerDDJ1000.keyCode(engine.getValue(group, "key")));
    const shift = PioneerDDJ1000.clamp(Math.round(engine.getValue(group, "pitch")), -12, 12);
    PioneerDDJ1000.send(note, d.keyShift, 0x0D + shift);
};

// ---------------------------------------------------------------------------
// Transport, tempo, key
// ---------------------------------------------------------------------------

PioneerDDJ1000.play = function(_channel, _control, value, _status, group) {
    if (value) {
        PioneerDDJ1000.toggle(group, "play");
    }
};

PioneerDDJ1000.cue = function(_channel, _control, value, _status, group) {
    engine.setValue(group, "cue_default", value ? 1 : 0);
};

PioneerDDJ1000.jumpToStart = function(_channel, _control, value, _status, group) {
    if (value) {
        engine.setValue(group, "start", 1);
    }
};

// Top of the TEMPO slider is "-" (MSB 0x00).
PioneerDDJ1000.tempo = function(_channel, control, value, status, group) {
    const position = PioneerDDJ1000.readHighRes(status, control, value) * 2 - 1;
    engine.setValue(group, "rate", position * (engine.getValue(group, "rate_dir") || 1));
};

PioneerDDJ1000.masterTempo = function(_channel, _control, value, _status, group) {
    if (value) {
        PioneerDDJ1000.toggle(group, "keylock");
    }
};

// SHIFT + MASTER TEMPO: TEMPO RANGE ±6 -> ±10 -> ±16 -> WIDE.
PioneerDDJ1000.tempoRange = function(_channel, _control, value, _status, group) {
    if (!value) {
        return;
    }
    const ranges = PioneerDDJ1000.config.tempoRanges;
    const current = engine.getValue(group, "rateRange");
    const next = ranges.find((range) => range > current + 0.001);
    engine.setValue(group, "rateRange", next === undefined ? ranges[0] : next);
};

PioneerDDJ1000.sync = function(_channel, _control, value, _status, group) {
    if (value) {
        PioneerDDJ1000.toggle(group, "sync_enabled");
    }
};

PioneerDDJ1000.syncMaster = function(_channel, _control, value, _status, group) {
    if (value) {
        engine.setValue(group, "sync_leader", engine.getValue(group, "sync_mode") === 2 ? 0 : 1);
    }
};

PioneerDDJ1000.keySync = function(_channel, _control, value, _status, group) {
    if (!value) {
        return;
    }
    const deck = PioneerDDJ1000.deckFromGroup(group);
    const state = PioneerDDJ1000.deck[deck];
    state.keySync = !state.keySync;
    if (state.keySync) {
        engine.setValue(group, "sync_key", 1);
    }
    PioneerDDJ1000.deckLed(deck, 0x65, state.keySync);
};

PioneerDDJ1000.keyReset = function(_channel, _control, value, _status, group) {
    if (!value) {
        return;
    }
    const deck = PioneerDDJ1000.deckFromGroup(group);
    engine.setValue(group, "reset_key", 1);
    PioneerDDJ1000.deck[deck].keySync = false;
    PioneerDDJ1000.deckLed(deck, 0x65, false);
};

// ---------------------------------------------------------------------------
// Loops
// ---------------------------------------------------------------------------

// LOOP IN / OUT and their 1/2X / 2X variants. While a loop point is being
// adjusted any of them ends the adjustment instead.
PioneerDDJ1000.loopButton = function(key) {
    return function(_channel, _control, value, _status, group) {
        const deck = PioneerDDJ1000.deckFromGroup(group);
        if (PioneerDDJ1000.deck[deck].loopAdjust) {
            if (value) {
                PioneerDDJ1000.setLoopAdjust(deck, null);
            }
            return;
        }
        engine.setValue(group, key, value ? 1 : 0);
    };
};

PioneerDDJ1000.loopIn = PioneerDDJ1000.loopButton("loop_in");
PioneerDDJ1000.loopOut = PioneerDDJ1000.loopButton("loop_out");
PioneerDDJ1000.loopHalve = PioneerDDJ1000.loopButton("loop_halve");
PioneerDDJ1000.loopDouble = PioneerDDJ1000.loopButton("loop_double");

// SHIFT + LOOP IN: move the loop-in point with the jog until pressed again.
PioneerDDJ1000.loopInAdjust = function(_channel, _control, value, _status, group) {
    if (!value || engine.getValue(group, "loop_start_position") < 0) {
        return;
    }
    const deck = PioneerDDJ1000.deckFromGroup(group);
    PioneerDDJ1000.setLoopAdjust(deck, PioneerDDJ1000.deck[deck].loopAdjust === "in" ? null : "in");
};

// SHIFT + LOOP OUT: adjust loop-out while looping, otherwise RELOOP.
PioneerDDJ1000.loopOutShift = function(_channel, _control, value, _status, group) {
    if (!value) {
        return;
    }
    const deck = PioneerDDJ1000.deckFromGroup(group);
    const adjusting = PioneerDDJ1000.deck[deck].loopAdjust;
    if (engine.getValue(group, "loop_enabled") || adjusting) {
        PioneerDDJ1000.setLoopAdjust(deck, adjusting === "out" ? null : "out");
    } else {
        engine.setValue(group, "reloop_toggle", 1);
    }
};

PioneerDDJ1000.setLoopAdjust = function(deck, which) {
    PioneerDDJ1000.deck[deck].loopAdjust = which;
    PioneerDDJ1000.updateLoopLeds(deck);
};

PioneerDDJ1000.updateLoopLeds = function(deck) {
    const state = PioneerDDJ1000.deck[deck];
    if (!state) {
        return;
    }
    const looping = engine.getValue(PioneerDDJ1000.group(deck), "loop_enabled") > 0;
    const lit = (end) => {
        if (state.loopAdjust === end) {
            return PioneerDDJ1000.blink;
        }
        return state.loopAdjust ? true : looping;
    };
    PioneerDDJ1000.deckLed(deck, 0x10, lit("in"));
    PioneerDDJ1000.deckLed(deck, 0x4C, lit("in"));
    PioneerDDJ1000.deckLed(deck, 0x11, lit("out"));
    PioneerDDJ1000.deckLed(deck, 0x4D, lit("out"));
};

PioneerDDJ1000.moveLoopPoint = function(deck, ticks) {
    const group = PioneerDDJ1000.group(deck);
    const start = engine.getValue(group, "loop_start_position");
    const end = engine.getValue(group, "loop_end_position");
    if (start < 0 || end < 0) {
        return;
    }
    const cfg = PioneerDDJ1000.config;
    const seconds = ticks / cfg.jogTicksPerRevolution * cfg.loopAdjustSecondsPerRevolution;
    // Keep the point on a whole stereo frame.
    const offset = 2 * Math.round(seconds * PioneerDDJ1000.samplesPerSecond(group) / 2);
    if (PioneerDDJ1000.deck[deck].loopAdjust === "in") {
        engine.setValue(group, "loop_start_position", PioneerDDJ1000.clamp(start + offset, 0, end - 2));
    } else {
        engine.setValue(group, "loop_end_position", Math.max(start + 2, end + offset));
    }
};

// 4 BEAT LOOP / EXIT.
PioneerDDJ1000.beatLoop4 = function(_channel, _control, value, _status, group) {
    if (!value) {
        return;
    }
    const deck = PioneerDDJ1000.deckFromGroup(group);
    if (PioneerDDJ1000.deck[deck].loopAdjust) {
        PioneerDDJ1000.setLoopAdjust(deck, null);
    } else if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "loop_enabled", 0);
    } else {
        engine.setValue(group, "beatloop_4_activate", 1);
    }
};

// SHIFT + 4 BEAT LOOP: make the stored loop active or inactive.
PioneerDDJ1000.loopActive = function(_channel, _control, value, _status, group) {
    if (value && engine.getValue(group, "loop_start_position") >= 0) {
        PioneerDDJ1000.toggle(group, "loop_enabled");
    }
};

// ---------------------------------------------------------------------------
// Deck buttons
// ---------------------------------------------------------------------------

// QUANTIZE applies to all decks, as in rekordbox.
PioneerDDJ1000.quantize = function(_channel, _control, value, _status, group) {
    if (!value) {
        return;
    }
    const on = engine.getValue(group, "quantize") ? 0 : 1;
    PioneerDDJ1000.decks.forEach((deck) => engine.setValue(PioneerDDJ1000.group(deck), "quantize", on));
};

PioneerDDJ1000.slip = function(_channel, _control, value, _status, group) {
    if (value) {
        PioneerDDJ1000.toggle(group, "slip_enabled");
    }
};

// SHIFT + SLIP: VINYL mode for the jog.
PioneerDDJ1000.vinyl = function(_channel, _control, value, _status, group) {
    if (value) {
        const deck = PioneerDDJ1000.deckFromGroup(group);
        PioneerDDJ1000.setVinyl(deck, !PioneerDDJ1000.deck[deck].vinyl);
    }
};

// The unit also reports its VINYL state on its own.
PioneerDDJ1000.vinylState = function(_channel, _control, value, _status, group) {
    PioneerDDJ1000.setVinyl(PioneerDDJ1000.deckFromGroup(group), value > 0);
};

PioneerDDJ1000.setVinyl = function(deck, on) {
    PioneerDDJ1000.deck[deck].vinyl = on;
    PioneerDDJ1000.deckLed(deck, 0x17, on);
};

// SLIP REVERSE: reverse while held; lets go by itself after 8 beats.
PioneerDDJ1000.slipReverse = function(_channel, _control, value, _status, group) {
    const state = PioneerDDJ1000.deck[PioneerDDJ1000.deckFromGroup(group)];
    if (state.reverseTimer) {
        engine.stopTimer(state.reverseTimer);
        state.reverseTimer = 0;
    }
    engine.setValue(group, "reverseroll", value ? 1 : 0);
    const bpm = engine.getValue(group, "bpm");
    if (value && bpm > 0) {
        const ms = Math.round(PioneerDDJ1000.config.slipReverseBeats * 60000 / bpm);
        state.reverseTimer = engine.beginTimer(ms, () => {
            state.reverseTimer = 0;
            engine.setValue(group, "reverseroll", 0);
        }, true);
    }
};

PioneerDDJ1000.reverse = function(_channel, _control, value, _status, group) {
    if (value) {
        PioneerDDJ1000.toggle(group, "reverse");
    }
};

PioneerDDJ1000.headphoneCue = function(_channel, _control, value, _status, group) {
    if (value) {
        PioneerDDJ1000.toggle(group, "pfl");
    }
};

// SHIFT + headphones CUE: TAP.
PioneerDDJ1000.tapBpm = function(_channel, _control, value, _status, group) {
    if (value) {
        engine.setValue(group, "bpm_tap", 1);
    }
};

// SHIFT + channel fader / crossfader: fader start.
PioneerDDJ1000.faderStartPlay = function(_channel, _control, value, _status, group) {
    if (value && !engine.getValue(group, "play")) {
        engine.setValue(group, "play", 1);
    }
};

PioneerDDJ1000.faderStartCue = function(_channel, _control, value, _status, group) {
    if (value) {
        engine.setValue(group, "cue_gotoandstop", 1);
    }
};

PioneerDDJ1000.crossfaderAssign = function(_channel, control, value, _status, group) {
    const side = {0x16: 0, 0x1D: 1, 0x18: 2}[control];
    if (value && side !== undefined) {
        engine.setValue(group, "orientation", side);
    }
};

// DECK 1/3, 2/4: the unit may redraw that side, so send it everything again.
PioneerDDJ1000.deckSelect = function(_channel, _control, value) {
    if (value) {
        PioneerDDJ1000.refreshAll();
    }
};

// Beat grid edit states: while one is on, SHIFT + jog edits the grid.
PioneerDDJ1000.gridAdjustState = function(_channel, _control, value, _status, group) {
    PioneerDDJ1000.deck[PioneerDDJ1000.deckFromGroup(group)].gridMode = value ? "adjust" : null;
};

PioneerDDJ1000.gridSlideState = function(_channel, _control, value, _status, group) {
    PioneerDDJ1000.deck[PioneerDDJ1000.deckFromGroup(group)].gridMode = value ? "slide" : null;
};

// SHIFT is resolved inside the unit, which sends different notes.
PioneerDDJ1000.shift = function() {};

// ---------------------------------------------------------------------------
// SEARCH, CUE/LOOP CALL, MEMORY
// ---------------------------------------------------------------------------

// A tap on SEARCH is a track search; holding it (the unit sends a second
// note once a press becomes a hold) searches through the track.
PioneerDDJ1000.searchTap = function(direction) {
    return function(_channel, _control, value, _status, group) {
        const deck = PioneerDDJ1000.deckFromGroup(group);
        const state = PioneerDDJ1000.deck[deck];
        if (value) {
            state.searchHeld = false;
            return;
        }
        PioneerDDJ1000.stopSearch(deck);
        if (!state.searchHeld) {
            PioneerDDJ1000.trackSearch(deck, direction);
        }
    };
};

PioneerDDJ1000.searchHold = function(direction) {
    return function(_channel, _control, value, _status, group) {
        const deck = PioneerDDJ1000.deckFromGroup(group);
        if (!value) {
            PioneerDDJ1000.stopSearch(deck);
            return;
        }
        PioneerDDJ1000.deck[deck].searchHeld = true;
        PioneerDDJ1000.deck[deck].search = {direction: direction, started: Date.now()};
        engine.setValue(group, "rateSearch", direction * PioneerDDJ1000.config.searchStartRate);
    };
};

PioneerDDJ1000.searchBack = PioneerDDJ1000.searchTap(-1);
PioneerDDJ1000.searchForward = PioneerDDJ1000.searchTap(1);
PioneerDDJ1000.searchBackLong = PioneerDDJ1000.searchHold(-1);
PioneerDDJ1000.searchForwardLong = PioneerDDJ1000.searchHold(1);

PioneerDDJ1000.stopSearch = function(deck) {
    if (PioneerDDJ1000.deck[deck].search) {
        PioneerDDJ1000.deck[deck].search = null;
        engine.setValue(PioneerDDJ1000.group(deck), "rateSearch", 0);
    }
};

PioneerDDJ1000.updateSearch = function(deck, now) {
    const search = PioneerDDJ1000.deck[deck].search;
    if (search) {
        const cfg = PioneerDDJ1000.config;
        const rate = Math.min(cfg.searchMaxRate, cfg.searchStartRate * Math.pow(2, (now - search.started) / 1000));
        engine.setValue(PioneerDDJ1000.group(deck), "rateSearch", search.direction * rate);
    }
};

// |<< goes to the start of the track, or to the previous track when already
// there; >>| loads the next track. Never on a playing deck.
PioneerDDJ1000.trackSearch = function(deck, direction) {
    const group = PioneerDDJ1000.group(deck);
    const elapsed = engine.getValue(group, "playposition") * engine.getValue(group, "duration");
    if (direction < 0 && elapsed > 1) {
        engine.setValue(group, "start", 1);
    } else if (!engine.getValue(group, "play")) {
        engine.setValue("[Library]", "MoveVertical", direction);
        engine.setValue(group, "LoadSelectedTrack", 1);
    }
};

// SHIFT + SEARCH: CUE/LOOP CALL, the previous / next stored point.
PioneerDDJ1000.cueCall = function(direction) {
    return function(_channel, _control, value, _status, group) {
        if (!value || !engine.getValue(group, "track_loaded")) {
            return;
        }
        const here = PioneerDDJ1000.playSamples(group);
        const margin = PioneerDDJ1000.samplesPerSecond(group) * 0.05;
        const points = [engine.getValue(group, "cue_point")];
        for (let i = 1; i <= PioneerDDJ1000.memoryLast; i++) {
            points.push(engine.getValue(group, `hotcue_${i}_position`));
        }
        const candidates = points.filter((p) => p >= 0 && (direction < 0 ? p < here - margin : p > here + margin));
        if (!candidates.length) {
            return;
        }
        const target = direction < 0 ? Math.max.apply(null, candidates) : Math.min.apply(null, candidates);
        PioneerDDJ1000.seekSamples(group, target);
        if (!engine.getValue(group, "play")) {
            // A called cue becomes the CUE point, as on a CDJ.
            engine.setValue(group, "cue_set", 1);
        }
    };
};

PioneerDDJ1000.cueCallBack = PioneerDDJ1000.cueCall(-1);
PioneerDDJ1000.cueCallForward = PioneerDDJ1000.cueCall(1);

// MEMORY: store the current position (or the running loop) as a memory cue.
PioneerDDJ1000.memory = function(_channel, _control, value, _status, group) {
    if (!value || !engine.getValue(group, "track_loaded")) {
        return;
    }
    const here = PioneerDDJ1000.playSamples(group);
    const margin = PioneerDDJ1000.samplesPerSecond(group) * 0.05;
    let free = 0;
    for (let i = PioneerDDJ1000.memoryFirst; i <= PioneerDDJ1000.memoryLast; i++) {
        const position = engine.getValue(group, `hotcue_${i}_position`);
        if (position >= 0 && Math.abs(position - here) < margin) {
            return;
        }
        if (position < 0 && !free) {
            free = i;
        }
    }
    if (free) {
        engine.setValue(group, `hotcue_${free}_set`, 1);
        engine.setValue(group, `hotcue_${free}_color`, PioneerDDJ1000.memoryColor);
    }
};

// SHIFT + MEMORY: delete the memory cue nearest the current position (within 1 s).
PioneerDDJ1000.memoryDelete = function(_channel, _control, value, _status, group) {
    if (!value || !engine.getValue(group, "track_loaded")) {
        return;
    }
    const here = PioneerDDJ1000.playSamples(group);
    let nearest = 0;
    let distance = PioneerDDJ1000.samplesPerSecond(group);
    for (let i = PioneerDDJ1000.memoryFirst; i <= PioneerDDJ1000.memoryLast; i++) {
        const position = engine.getValue(group, `hotcue_${i}_position`);
        if (position >= 0 && Math.abs(position - here) <= distance) {
            nearest = i;
            distance = Math.abs(position - here);
        }
    }
    if (nearest) {
        engine.setValue(group, `hotcue_${nearest}_clear`, 1);
    }
};

// ---------------------------------------------------------------------------
// Jog wheel
// ---------------------------------------------------------------------------

PioneerDDJ1000.playingTicksPerSecond = () =>
    PioneerDDJ1000.config.jogTicksPerRevolution / PioneerDDJ1000.secondsPerRevolution();

// Remember platter movement for the last 100 ms, to know how fast it turns.
PioneerDDJ1000.recordSpin = function(state, ticks, now) {
    state.spinTicks.push([now, ticks]);
    while (state.spinTicks.length && now - state.spinTicks[0][0] > 100) {
        state.spinTicks.shift();
    }
};

PioneerDDJ1000.spinSpeed = function(state, now) {
    const recent = state.spinTicks.filter(([time]) => now - time <= 100);
    const total = recent.reduce((sum, [, ticks]) => sum + ticks, 0);
    return Math.abs(total) * 10 / PioneerDDJ1000.playingTicksPerSecond(); // 1.0 = playing speed
};

PioneerDDJ1000.jogTouch = function(_channel, _control, value, _status, group) {
    const deck = PioneerDDJ1000.deckFromGroup(group);
    const state = PioneerDDJ1000.deck[deck];
    const cfg = PioneerDDJ1000.config;
    state.touched = value > 0;
    if (value) {
        state.freeSpin = false;
        state.spinTicks = [];
        if (state.vinyl && !state.loopAdjust) {
            engine.scratchEnable(deck, cfg.jogTicksPerRevolution, cfg.vinylRpm, cfg.scratchAlpha, cfg.scratchBeta);
        }
    } else if (engine.isScratching(deck)) {
        // The platter is heavy: let go of it spinning and the record keeps
        // following it (through the outer ring) until it slows down. The
        // speed it had under the hand counts until the ring reports.
        state.freeSpin = true;
    }
};

PioneerDDJ1000.jogScratch = function(_channel, _control, value, _status, group) {
    const deck = PioneerDDJ1000.deckFromGroup(group);
    const ticks = PioneerDDJ1000.jogDelta(value);
    if (PioneerDDJ1000.deck[deck].loopAdjust) {
        PioneerDDJ1000.moveLoopPoint(deck, ticks);
    } else if (engine.isScratching(deck)) {
        PioneerDDJ1000.recordSpin(PioneerDDJ1000.deck[deck], ticks, Date.now());
        engine.scratchTick(deck, ticks);
    } else {
        PioneerDDJ1000.bend(group, ticks);
    }
};

PioneerDDJ1000.jogBend = function(_channel, _control, value, _status, group) {
    const deck = PioneerDDJ1000.deckFromGroup(group);
    const ticks = PioneerDDJ1000.jogDelta(value);
    if (PioneerDDJ1000.deck[deck].loopAdjust) {
        PioneerDDJ1000.moveLoopPoint(deck, ticks);
    } else {
        PioneerDDJ1000.bend(group, ticks);
    }
};

// The outer ring reports even when the top is not touched.
PioneerDDJ1000.jogRing = function(_channel, _control, value, _status, group) {
    const deck = PioneerDDJ1000.deckFromGroup(group);
    const state = PioneerDDJ1000.deck[deck];
    const ticks = PioneerDDJ1000.jogDelta(value);
    if (state.loopAdjust) {
        PioneerDDJ1000.moveLoopPoint(deck, ticks);
    } else if (state.freeSpin) {
        PioneerDDJ1000.recordSpin(state, ticks, Date.now());
        engine.scratchTick(deck, ticks);
    } else if (!engine.isScratching(deck)) {
        PioneerDDJ1000.bend(group, ticks);
    }
};

PioneerDDJ1000.bend = function(group, ticks) {
    const cfg = PioneerDDJ1000.config;
    engine.setValue(group, "jog", ticks * cfg.bendPerRevolution / cfg.jogTicksPerRevolution);
};

// From the timer: hand a released platter back to normal playback once it
// has slowed down or stopped reporting.
PioneerDDJ1000.watchFreeSpin = function(deck, now) {
    const state = PioneerDDJ1000.deck[deck];
    if (state.touched || !engine.isScratching(deck)) {
        state.freeSpin = false;
        return;
    }
    const lastReport = state.spinTicks.length ? state.spinTicks[state.spinTicks.length - 1][0] : 0;
    if (!state.freeSpin || now - lastReport > 60
            || PioneerDDJ1000.spinSpeed(state, now) < PioneerDDJ1000.config.freeSpinMinSpeed) {
        state.freeSpin = false;
        engine.scratchDisable(deck, true);
    }
};

// SHIFT + platter or ring: search the track, or edit the beat grid while a
// grid edit state is on.
PioneerDDJ1000.jogShiftTop = function(_channel, _control, value, _status, group) {
    const deck = PioneerDDJ1000.deckFromGroup(group);
    const ticks = PioneerDDJ1000.jogDelta(value);
    const mode = PioneerDDJ1000.deck[deck].gridMode;
    if (mode === "adjust") {
        PioneerDDJ1000.gridStep(deck, ticks, "beats_adjust_faster", "beats_adjust_slower");
    } else if (mode === "slide") {
        PioneerDDJ1000.gridStep(deck, ticks, "beats_translate_later", "beats_translate_earlier");
    } else {
        PioneerDDJ1000.searchByJog(group, ticks, 1);
    }
};

PioneerDDJ1000.jogShiftRing = function(_channel, _control, value, _status, group) {
    const deck = PioneerDDJ1000.deckFromGroup(group);
    const ticks = PioneerDDJ1000.jogDelta(value);
    if (PioneerDDJ1000.deck[deck].gridMode) {
        PioneerDDJ1000.gridStep(deck, ticks, "beats_translate_later", "beats_translate_earlier");
    } else {
        PioneerDDJ1000.searchByJog(group, ticks, 1);
    }
};

// SEARCH held + platter: fast search.
PioneerDDJ1000.jogSearchFast = function(_channel, _control, value, _status, group) {
    PioneerDDJ1000.searchByJog(group, PioneerDDJ1000.jogDelta(value), 5);
};

PioneerDDJ1000.searchByJog = function(group, ticks, factor) {
    const duration = engine.getValue(group, "duration");
    if (duration <= 0) {
        return;
    }
    const cfg = PioneerDDJ1000.config;
    const seconds = ticks / cfg.jogTicksPerRevolution * cfg.searchSecondsPerRevolution * factor;
    const position = engine.getValue(group, "playposition") + seconds / duration;
    engine.setValue(group, "playposition", PioneerDDJ1000.clamp(position, 0, 1));
};

// One grid step per 1/32 of a revolution.
PioneerDDJ1000.gridStep = function(deck, ticks, forward, backward) {
    const state = PioneerDDJ1000.deck[deck];
    const step = PioneerDDJ1000.config.jogTicksPerRevolution / 32;
    state.gridRemainder += ticks;
    const steps = Math.trunc(state.gridRemainder / step);
    state.gridRemainder -= steps * step;
    for (let i = 0; i < Math.abs(steps); i++) {
        engine.setValue(PioneerDDJ1000.group(deck), steps > 0 ? forward : backward, 1);
    }
};

// ---------------------------------------------------------------------------
// Browser
// ---------------------------------------------------------------------------

PioneerDDJ1000.browse = function(_channel, _control, value) {
    const delta = PioneerDDJ1000.encoderDelta(value);
    if (delta) {
        engine.setValue("[Library]", "MoveVertical", delta);
    }
};

// SHIFT + rotary: waveform zoom (clockwise enlarges).
PioneerDDJ1000.browseShift = function(_channel, _control, value) {
    const delta = PioneerDDJ1000.encoderDelta(value);
    PioneerDDJ1000.decks.forEach((deck) => {
        const group = PioneerDDJ1000.group(deck);
        engine.setValue(group, "waveform_zoom",
            PioneerDDJ1000.clamp(engine.getValue(group, "waveform_zoom") - delta * 0.5, 1, 10));
    });
};

// Rotary press: load the selected track to the deck on that side, or open the
// folder when the tree has focus. Pressed twice quickly: instant double from
// the other deck on the same side.
PioneerDDJ1000.browsePress = function(_channel, control, value) {
    if (!value) {
        return;
    }
    const deck = control - 0x45;
    const group = PioneerDDJ1000.group(deck);
    const state = PioneerDDJ1000.deck[deck];
    const now = Date.now();
    if (engine.getValue("[Library]", "focused_widget") === 2) {
        engine.setValue("[Library]", "GoToItem", 1);
    } else if (now - state.lastLoad < PioneerDDJ1000.config.doublePressMs) {
        state.lastLoad = 0;
        engine.setValue(group, "CloneFromDeck", deck <= 2 ? deck + 2 : deck - 2);
    } else {
        state.lastLoad = now;
        engine.setValue(group, "LoadSelectedTrack", 1);
    }
};

// BACK: switch between the tree and the track list.
PioneerDDJ1000.back = function(_channel, _control, value) {
    if (value) {
        const focused = engine.getValue("[Library]", "focused_widget");
        engine.setValue("[Library]", "focused_widget", focused === 2 ? 3 : 2);
    }
};

// VIEW: maximise the library.
PioneerDDJ1000.view = function(_channel, _control, value) {
    if (value) {
        PioneerDDJ1000.toggle("[Skin]", "show_maximized_library");
    }
};

// VIEW held: add the selected track to the Auto DJ queue.
PioneerDDJ1000.viewLong = function(_channel, _control, value) {
    if (value) {
        engine.setValue("[Library]", "AutoDjAddBottom", 1);
    }
};

// ---------------------------------------------------------------------------
// Sampler section
// ---------------------------------------------------------------------------

PioneerDDJ1000.samplerVolume = function(_channel, control, value, status) {
    const level = PioneerDDJ1000.readHighRes(status, control, value);
    for (let i = 1; i <= PioneerDDJ1000.numSamplers(); i++) {
        engine.setParameter(`[Sampler${i}]`, "volume", level);
    }
};

PioneerDDJ1000.samplerCue = function(_channel, _control, value) {
    if (!value) {
        return;
    }
    const on = engine.getValue("[Sampler1]", "pfl") ? 0 : 1;
    for (let i = 1; i <= PioneerDDJ1000.numSamplers(); i++) {
        engine.setValue(`[Sampler${i}]`, "pfl", on);
    }
    PioneerDDJ1000.updateSamplerCueLed();
};

// ---------------------------------------------------------------------------
// SOUND COLOR FX (the decks' Quick Effects)
// ---------------------------------------------------------------------------

PioneerDDJ1000.colorFxButton = function(_channel, control, value) {
    if (value) {
        const which = (control & 0x03) + 1;
        PioneerDDJ1000.colorFx = PioneerDDJ1000.colorFx === which ? 0 : which;
        PioneerDDJ1000.applyColorFx();
    }
};

// The unit reports which SOUND COLOR FX is on (notes 0x10-0x13).
PioneerDDJ1000.colorFxState = function(_channel, control, value) {
    const which = (control & 0x03) + 1;
    if (value) {
        PioneerDDJ1000.colorFx = which;
    } else if (PioneerDDJ1000.colorFx === which) {
        PioneerDDJ1000.colorFx = 0;
    } else {
        return;
    }
    PioneerDDJ1000.applyColorFx();
};

// Each button can load a Quick Effect chain preset (setting colorFxPreset1-4,
// 0 keeps the decks' own). By default the COLOR knobs always work, as Mixxx
// users expect; the colorFxNeedsButton setting makes them silent until a
// button is on, as in rekordbox.
PioneerDDJ1000.applyColorFx = function() {
    const which = PioneerDDJ1000.colorFx;
    const preset = which ? PioneerDDJ1000.setting(`colorFxPreset${which}`, 0) : 0;
    const needsButton = PioneerDDJ1000.setting("colorFxNeedsButton", false);
    PioneerDDJ1000.decks.forEach((deck) => {
        const unit = `[QuickEffectRack1_${PioneerDDJ1000.group(deck)}]`;
        if (preset > 0 && preset < engine.getValue(unit, "num_chain_presets")
                && engine.getValue(unit, "loaded_chain_preset") !== preset) {
            engine.setValue(unit, "loaded_chain_preset", preset);
        }
        if (needsButton) {
            engine.setValue(unit, "enabled", which ? 1 : 0);
        }
    });
    PioneerDDJ1000.updateColorFxLeds();
};

PioneerDDJ1000.updateColorFxLeds = function() {
    for (let i = 0; i < 4; i++) {
        PioneerDDJ1000.led(0x96, i, PioneerDDJ1000.colorFx === i + 1);
        PioneerDDJ1000.led(0x96, 0x08 + i, PioneerDDJ1000.colorFx === i + 1);
    }
};

// ---------------------------------------------------------------------------
// BEAT FX (effect unit 1)
// ---------------------------------------------------------------------------

PioneerDDJ1000.beatFxUnit = "[EffectRack1_EffectUnit1]";

PioneerDDJ1000.rollType = function() {
    const name = PioneerDDJ1000.beatFxPositions[PioneerDDJ1000.beatFx.position];
    if (name === "SLIP ROLL") {
        return "slip";
    }
    return name === "ROLL" ? "loop" : null;
};

PioneerDDJ1000.beatValue = () => PioneerDDJ1000.config.beatValues[PioneerDDJ1000.beatFx.beatIndex];

PioneerDDJ1000.routeBeatFx = function() {
    const unit = PioneerDDJ1000.beatFxUnit;
    const target = PioneerDDJ1000.beatFx.target;
    PioneerDDJ1000.decks.forEach((deck) => {
        engine.setValue(unit, `group_[Channel${deck}]_enable`, target === deck ? 1 : 0);
    });
    engine.setValue(unit, "group_[Master]_enable", target === "master" ? 1 : 0);
    for (let i = 1; i <= PioneerDDJ1000.numSamplers(); i++) {
        engine.setValue(unit, `group_[Sampler${i}]_enable`, target === "sampler" ? 1 : 0);
    }
};

// The decks a roll plays on: the selected channel, or every playing deck on MASTER.
PioneerDDJ1000.rollDecks = function() {
    const target = PioneerDDJ1000.beatFx.target;
    if (typeof target === "number") {
        return [target];
    }
    if (target === "master") {
        return PioneerDDJ1000.decks.filter((deck) => engine.getValue(PioneerDDJ1000.group(deck), "play") > 0);
    }
    return [];
};

// FX SELECT: position N loads effect unit 1's chain preset N (the order is
// set in Preferences > Effects); SLIP ROLL and ROLL use the deck's loops.
PioneerDDJ1000.beatFxSelect = function(_channel, control, value) {
    const position = control - 0x20;
    if (!value || position === PioneerDDJ1000.beatFx.position) {
        return;
    }
    const wasOn = PioneerDDJ1000.beatFx.on;
    PioneerDDJ1000.setBeatFx(false);
    PioneerDDJ1000.beatFx.position = position;
    if (!PioneerDDJ1000.rollType()) {
        const preset = PioneerDDJ1000.beatFxPositions
            .slice(0, position + 1)
            .filter((name) => name !== "SLIP ROLL" && name !== "ROLL").length;
        if (preset < engine.getValue(PioneerDDJ1000.beatFxUnit, "num_chain_presets")) {
            engine.setValue(PioneerDDJ1000.beatFxUnit, "loaded_chain_preset", preset);
        }
    }
    PioneerDDJ1000.setBeatFx(wasOn);
};

// CH SELECT: CH1-CH4, MASTER, MIC, SAMPLER.
PioneerDDJ1000.beatFxChannel = function(_channel, control, value) {
    const target = [1, 2, 3, 4, "master", "mic", "sampler"][control - 0x10];
    if (!value || target === undefined) {
        return;
    }
    const wasOn = PioneerDDJ1000.beatFx.on;
    PioneerDDJ1000.setBeatFx(false);
    PioneerDDJ1000.beatFx.target = target;
    PioneerDDJ1000.routeBeatFx();
    PioneerDDJ1000.setBeatFx(wasOn);
};

PioneerDDJ1000.beatFxLevel = function(_channel, control, value, status) {
    engine.setParameter(PioneerDDJ1000.beatFxUnit, "mix", PioneerDDJ1000.readHighRes(status, control, value));
};

PioneerDDJ1000.beatFxOn = function(_channel, _control, value) {
    if (value) {
        PioneerDDJ1000.setBeatFx(!PioneerDDJ1000.beatFx.on);
    }
};

PioneerDDJ1000.setBeatFx = function(on) {
    const fx = PioneerDDJ1000.beatFx;
    const roll = PioneerDDJ1000.rollType();
    fx.rolling.forEach((deck) => PioneerDDJ1000.stopRoll(deck, fx.rollSlip));
    fx.rolling = [];
    if (roll && on) {
        fx.rollSlip = roll === "slip";
        fx.rolling = PioneerDDJ1000.rollDecks();
        fx.rolling.forEach((deck) => PioneerDDJ1000.startRoll(deck, PioneerDDJ1000.beatValue(), fx.rollSlip));
    }
    fx.on = on;
    engine.setValue(PioneerDDJ1000.beatFxUnit, "enabled", on && !roll ? 1 : 0);
    PioneerDDJ1000.led(0x94, 0x47, on);
};

PioneerDDJ1000.startRoll = function(deck, beats, slip) {
    const group = PioneerDDJ1000.group(deck);
    engine.setValue(group, "beatloop_size", beats);
    engine.setValue(group, slip ? "beatlooproll_activate" : "beatloop_activate", 1);
};

PioneerDDJ1000.stopRoll = function(deck, slip) {
    engine.setValue(PioneerDDJ1000.group(deck), slip ? "beatlooproll_activate" : "loop_enabled", 0);
};

// SHIFT + BEAT FX ON/OFF: RELEASE FX, Beat FX off with a vinyl brake.
PioneerDDJ1000.releaseFx = function(_channel, _control, value) {
    PioneerDDJ1000.led(0x94, 0x43, value > 0);
    if (value) {
        const decks = PioneerDDJ1000.rollDecks();
        PioneerDDJ1000.setBeatFx(false);
        decks.forEach((deck) => engine.brake(deck, true));
    }
};

// BEAT < / >: the roll length, or the first effect's first parameter (its
// time for echo, flanger, phaser and similar effects).
PioneerDDJ1000.beatStep = function(direction, note) {
    return function(_channel, _control, value) {
        PioneerDDJ1000.led(0x94, note, value > 0);
        if (!value) {
            return;
        }
        const fx = PioneerDDJ1000.beatFx;
        if (PioneerDDJ1000.rollType()) {
            fx.beatIndex = PioneerDDJ1000.clamp(fx.beatIndex + direction, 0, PioneerDDJ1000.config.beatValues.length - 1);
            fx.rolling.forEach((deck) => {
                engine.setValue(PioneerDDJ1000.group(deck), "beatloop_size", PioneerDDJ1000.beatValue());
            });
        } else {
            const slot = "[EffectRack1_EffectUnit1_Effect1]";
            engine.setParameter(slot, "parameter1",
                PioneerDDJ1000.clamp(engine.getParameter(slot, "parameter1") + direction / 16, 0, 1));
        }
    };
};

PioneerDDJ1000.beatDown = PioneerDDJ1000.beatStep(-1, 0x4A);
PioneerDDJ1000.beatUp = PioneerDDJ1000.beatStep(1, 0x4B);

// SHIFT + BEAT < / > (AUTO / TAP in rekordbox): Mixxx effects follow the deck
// tempo on their own, so these only light up.
PioneerDDJ1000.beatAuto = function(_channel, _control, value) {
    PioneerDDJ1000.led(0x94, 0x66, value > 0);
};

PioneerDDJ1000.beatTap = function(_channel, _control, value) {
    PioneerDDJ1000.led(0x94, 0x6B, value > 0);
};

// ---------------------------------------------------------------------------
// Performance pads
// ---------------------------------------------------------------------------

PioneerDDJ1000.padMode = function(_channel, control, value, _status, group) {
    const mode = PioneerDDJ1000.modeButtons.indexOf(control);
    if (value && mode >= 0) {
        const deck = PioneerDDJ1000.deckFromGroup(group);
        PioneerDDJ1000.deck[deck].mode = mode;
        PioneerDDJ1000.updateModeLeds(deck);
    }
};

// PAGE < / >. In BEAT JUMP they halve / double the jump; in SAMPLER they
// change the bank.
PioneerDDJ1000.page = function(notes, direction) {
    return function(_channel, control, value, _status, group) {
        const mode = notes.indexOf(control);
        if (!value || mode < 0) {
            return;
        }
        const deck = PioneerDDJ1000.deckFromGroup(group);
        const state = PioneerDDJ1000.deck[deck];
        state.mode = mode;
        if (mode === PioneerDDJ1000.mode.beatJump) {
            state.jumpScale = PioneerDDJ1000.clamp(state.jumpScale * (direction > 0 ? 2 : 0.5), 1 / 8, 16);
        } else if (mode === PioneerDDJ1000.mode.sampler) {
            PioneerDDJ1000.stepSamplerBank(deck, direction);
        } else {
            state.page[mode] = direction > 0 ? 2 : 1;
        }
        PioneerDDJ1000.updateModeLeds(deck);
    };
};

PioneerDDJ1000.pagePrev = PioneerDDJ1000.page(PioneerDDJ1000.pagePrevNotes, -1);
PioneerDDJ1000.pageNext = PioneerDDJ1000.page(PioneerDDJ1000.pageNextNotes, 1);

// SHIFT + PAGE < / > changes the sampler bank in every mode.
PioneerDDJ1000.bankStep = function(direction) {
    return function(_channel, _control, value, _status, group) {
        if (value) {
            PioneerDDJ1000.stepSamplerBank(PioneerDDJ1000.deckFromGroup(group), direction);
        }
    };
};

PioneerDDJ1000.bankPrev = PioneerDDJ1000.bankStep(-1);
PioneerDDJ1000.bankNext = PioneerDDJ1000.bankStep(1);

PioneerDDJ1000.stepSamplerBank = function(deck, direction) {
    const state = PioneerDDJ1000.deck[deck];
    state.samplerBank = PioneerDDJ1000.clamp(state.samplerBank + direction, 0, PioneerDDJ1000.config.samplerBanks - 1);
    PioneerDDJ1000.ensureSamplers((state.samplerBank + 1) * 8);
};

PioneerDDJ1000.ensureSamplers = function(count) {
    if (PioneerDDJ1000.numSamplers() < count) {
        engine.setValue("[App]", "num_samplers", count);
    }
};

PioneerDDJ1000.updateModeLeds = function(deck) {
    const state = PioneerDDJ1000.deck[deck];
    if (!state) {
        return;
    }
    const status = 0x90 + deck - 1;
    PioneerDDJ1000.modeButtons.forEach((note, mode) => {
        // A SHIFT mode lights its own note and blinks its base button.
        const on = mode === state.mode || (mode === state.mode - 4 && PioneerDDJ1000.blink);
        PioneerDDJ1000.led(status, note, on);
    });
    for (let mode = 0; mode < 8; mode++) {
        const paged = mode !== PioneerDDJ1000.mode.beatJump && mode !== PioneerDDJ1000.mode.sampler;
        const active = mode === state.mode;
        PioneerDDJ1000.led(status, PioneerDDJ1000.pagePrevNotes[mode], active && (!paged || state.page[mode] === 2));
        PioneerDDJ1000.led(status, PioneerDDJ1000.pageNextNotes[mode], active && (!paged || state.page[mode] === 1));
    }
};

// Pads: note = mode * 16 + (page - 1) * 8 + pad; SHIFT uses the next channel.
PioneerDDJ1000.pad = function(_channel, control, value, status) {
    const deck = (((status & 0x0F) - 7) >> 1) + 1;
    const shifted = (((status & 0x0F) - 7) & 1) === 1;
    const mode = control >> 4;
    const page = control & 0x08 ? 2 : 1;
    const pad = control & 0x07;
    const state = PioneerDDJ1000.deck[deck];
    const group = PioneerDDJ1000.group(deck);
    const m = PioneerDDJ1000.mode;

    state.mode = mode;
    state.pressed[mode] = state.pressed[mode] || {};
    state.pressed[mode][pad] = value > 0;

    if (mode === m.hotCue) {
        PioneerDDJ1000.padHotCue(deck, pad + 1 + (page - 1) * 8, value, shifted);
    } else if (mode === m.padFx1 || mode === m.padFx2) {
        PioneerDDJ1000.padEffect(deck, PioneerDDJ1000.padFx[mode][page - 1][pad], value);
    } else if (mode === m.beatJump) {
        if (value) {
            engine.setValue(group, "beatjump", PioneerDDJ1000.beatJumpBeats[pad] * state.jumpScale);
        }
    } else if (mode === m.sampler) {
        PioneerDDJ1000.padSampler(state.samplerBank * 8 + pad + 1, value, shifted);
    } else if (mode === m.keyboard) {
        PioneerDDJ1000.padKeyboard(deck, PioneerDDJ1000.semitones[page - 1][pad], pad, value, shifted);
    } else if (mode === m.beatLoop) {
        if (value && !shifted) {
            PioneerDDJ1000.padBeatLoop(group, PioneerDDJ1000.beatLoopBeats[page - 1][pad]);
        }
    } else if (mode === m.keyShift && value) {
        const semitones = PioneerDDJ1000.semitones[page - 1][pad];
        const current = Math.round(engine.getValue(group, "pitch_adjust"));
        engine.setValue(group, "pitch_adjust", current === semitones ? 0 : semitones);
    }
    PioneerDDJ1000.renderPads(deck, false);
};

PioneerDDJ1000.padHotCue = function(deck, number, value, shifted) {
    const group = PioneerDDJ1000.group(deck);
    if (shifted) {
        if (value) {
            engine.setValue(group, `hotcue_${number}_clear`, 1);
        }
        return;
    }
    // Sets the cue (or saves the running loop) when empty, otherwise jumps;
    // held while paused it previews.
    engine.setValue(group, `hotcue_${number}_activate`, value ? 1 : 0);
    if (value) {
        PioneerDDJ1000.deck[deck].keyboardCue = number;
    }
};

// PAD FX: rolls and deck tricks, or effect units held on this deck. A held
// unit is routed to the deck alone and handed back as it was on release.
PioneerDDJ1000.padEffect = function(deck, entry, value) {
    const group = PioneerDDJ1000.group(deck);
    const state = PioneerDDJ1000.deck[deck];
    if (entry.roll !== undefined) {
        if (value) {
            PioneerDDJ1000.startRoll(deck, entry.roll, true);
        } else {
            PioneerDDJ1000.stopRoll(deck, true);
        }
    } else if (entry.brake || entry.spinback) {
        // Slip keeps the track running underneath, so release drops back in
        // where it would have been.
        if (value) {
            state.padFxSlip = engine.getValue(group, "slip_enabled");
            engine.setValue(group, "slip_enabled", 1);
        }
        if (entry.brake) {
            engine.brake(deck, value > 0);
        } else {
            engine.spinback(deck, value > 0);
        }
        if (!value) {
            engine.setValue(group, "play", 1);
            engine.setValue(group, "slip_enabled", state.padFxSlip);
        }
    } else if (entry.reverse) {
        engine.setValue(group, "reverseroll", value ? 1 : 0);
    } else if (value) {
        PioneerDDJ1000.holdUnit(deck, entry.unit, entry.slot);
    } else {
        PioneerDDJ1000.releaseUnit(entry.unit);
    }
};

PioneerDDJ1000.holdUnit = function(deck, unitNumber, slot) {
    const unit = `[EffectRack1_EffectUnit${unitNumber}]`;
    if (PioneerDDJ1000.heldUnits[unitNumber]) {
        PioneerDDJ1000.releaseUnit(unitNumber);
    }
    const saved = {enabled: engine.getValue(unit, "enabled"), groups: {}, slots: {}};
    PioneerDDJ1000.decks.forEach((d) => {
        const key = `group_[Channel${d}]_enable`;
        saved.groups[key] = engine.getValue(unit, key);
        engine.setValue(unit, key, d === deck ? 1 : 0);
    });
    if (slot) {
        for (let i = 1; i <= 3; i++) {
            const slotGroup = `[EffectRack1_EffectUnit${unitNumber}_Effect${i}]`;
            saved.slots[slotGroup] = engine.getValue(slotGroup, "enabled");
            engine.setValue(slotGroup, "enabled", i === slot ? 1 : 0);
        }
    }
    PioneerDDJ1000.heldUnits[unitNumber] = saved;
    engine.setValue(unit, "enabled", 1);
};

PioneerDDJ1000.releaseUnit = function(unitNumber) {
    const saved = PioneerDDJ1000.heldUnits[unitNumber];
    if (!saved) {
        return;
    }
    const unit = `[EffectRack1_EffectUnit${unitNumber}]`;
    engine.setValue(unit, "enabled", saved.enabled);
    Object.keys(saved.groups).forEach((key) => engine.setValue(unit, key, saved.groups[key]));
    Object.keys(saved.slots).forEach((slotGroup) => engine.setValue(slotGroup, "enabled", saved.slots[slotGroup]));
    delete PioneerDDJ1000.heldUnits[unitNumber];
};

// SAMPLER: play the slot from the start; SHIFT stops it; an empty slot loads
// the selected track.
PioneerDDJ1000.padSampler = function(number, value, shifted) {
    if (!value) {
        return;
    }
    PioneerDDJ1000.ensureSamplers(Math.ceil(number / 8) * 8);
    const group = `[Sampler${number}]`;
    if (shifted) {
        engine.setValue(group, "cue_gotoandstop", 1);
    } else if (engine.getValue(group, "track_loaded")) {
        engine.setValue(group, "cue_gotoandplay", 1);
    } else {
        engine.setValue(group, "LoadSelectedTrack", 1);
    }
};

// KEYBOARD: play the chosen hot cue in another key; SHIFT + pad chooses the
// hot cue (1-8). Without that hot cue the CUE point is used.
PioneerDDJ1000.padKeyboard = function(deck, semitones, pad, value, shifted) {
    const group = PioneerDDJ1000.group(deck);
    const state = PioneerDDJ1000.deck[deck];
    if (shifted) {
        if (value) {
            state.keyboardCue = pad + 1;
        }
        return;
    }
    if (value) {
        engine.setValue(group, "pitch_adjust", semitones);
    }
    if (engine.getValue(group, `hotcue_${state.keyboardCue}_position`) >= 0) {
        engine.setValue(group, `hotcue_${state.keyboardCue}_activate`, value ? 1 : 0);
    } else if (value) {
        engine.setValue(group, "cue_gotoandplay", 1);
    }
};

PioneerDDJ1000.padBeatLoop = function(group, beats) {
    const sameSize = Math.abs(engine.getValue(group, "beatloop_size") - beats) < 1e-6;
    if (engine.getValue(group, "loop_enabled") && sameSize) {
        engine.setValue(group, "loop_enabled", 0);
    } else {
        engine.setValue(group, "beatloop_size", beats);
        engine.setValue(group, "beatloop_activate", 1);
    }
};

// Send pad colours that changed: the mode on screen, or every mode.
PioneerDDJ1000.renderPads = function(deck, allModes) {
    const state = PioneerDDJ1000.deck[deck];
    if (!state || !PioneerDDJ1000.opened) {
        return;
    }
    for (let mode = 0; mode < 8; mode++) {
        if (!allModes && mode !== state.mode) {
            continue;
        }
        for (let page = 1; page <= 2; page++) {
            for (let pad = 0; pad < 8; pad++) {
                const colour = PioneerDDJ1000.padColor(deck, mode, page, pad);
                const note = mode * 16 + (page - 1) * 8 + pad;
                PioneerDDJ1000.send(PioneerDDJ1000.padStatus(deck, false), note, colour);
                PioneerDDJ1000.send(PioneerDDJ1000.padStatus(deck, true), note, colour);
            }
        }
    }
};

PioneerDDJ1000.padColor = function(deck, mode, page, pad) {
    const group = PioneerDDJ1000.group(deck);
    const state = PioneerDDJ1000.deck[deck];
    const c = PioneerDDJ1000.color;
    const m = PioneerDDJ1000.mode;
    const pressed = state.pressed[mode] && state.pressed[mode][pad];

    if (mode === m.hotCue) {
        const number = pad + 1 + (page - 1) * 8;
        const status = engine.getValue(group, `hotcue_${number}_status`);
        if (!status) {
            return c.off;
        }
        const colour = PioneerDDJ1000.colorMapper.getValueForNearestColor(
            engine.getValue(group, `hotcue_${number}_color`));
        // A saved loop that is running blinks.
        const runningLoop = engine.getValue(group, `hotcue_${number}_type`) === 4 && status === 2;
        return runningLoop && !PioneerDDJ1000.blink ? c.off : colour;
    }
    if (mode === m.padFx1 || mode === m.padFx2) {
        return pressed ? c.white : PioneerDDJ1000.padFx[mode][page - 1][pad].color;
    }
    if (mode === m.beatJump) {
        if (pressed) {
            return c.white;
        }
        return PioneerDDJ1000.beatJumpBeats[pad] < 0 ? c.azure : c.cyan;
    }
    if (mode === m.sampler) {
        const sampler = `[Sampler${state.samplerBank * 8 + pad + 1}]`;
        if (!engine.getValue(sampler, "track_loaded")) {
            return c.off;
        }
        return engine.getValue(sampler, "play") && PioneerDDJ1000.blink ? c.white : c.magenta;
    }
    if (mode === m.keyboard || mode === m.keyShift) {
        const semitones = PioneerDDJ1000.semitones[page - 1][pad];
        if (Math.round(engine.getValue(group, "pitch_adjust")) === semitones) {
            return c.white;
        }
        if (semitones === 0) {
            return c.yellow;
        }
        return mode === m.keyboard ? c.purple : c.pink;
    }
    if (mode === m.beatLoop) {
        const beats = PioneerDDJ1000.beatLoopBeats[page - 1][pad];
        const active = engine.getValue(group, "loop_enabled")
            && Math.abs(engine.getValue(group, "beatloop_size") - beats) < 1e-6;
        return active && PioneerDDJ1000.blink ? c.white : c.green;
    }
    return c.off;
};
