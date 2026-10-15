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
//     loop is running (PioneerDDJ1000.unitNote.loopRunning).
//   * Every pad mode has its own pad notes, and the unit shows a mode's pad
//     colours by itself as soon as it switches to it, so the colours of all
//     modes are kept up to date, not only those of the mode on screen.
//   * Pad LEDs take a colour number on a hue wheel: 1 blue, 21 green,
//     29 yellow, 41 red, 61 purple; 127 is near-white and 0 dims the pad.
//     (Palette first charted for the DDJ-FLX10 by Veezuhz.)
//
// Changes to Mixxx's own state: Mixxx's headphone mix (setting
// hardwareHeadphoneMix), the tempo range (setting tempoRange), effect unit 1's
// routing (BEAT FX CH SELECT) and the Quick Effect switches (setting
// colorFxNeedsButton) are put back in shutdown(), unless they were changed
// again meanwhile; see PioneerDDJ1000.override(). Memory cues are stored as hot
// cues 17-36.

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
    // At most this many sampler banks of 8 pads. Only banks of samplers that
    // exist in Mixxx are used; the mapping never adds samplers.
    samplerBanks: 8,
    beatValues: [1 / 16, 1 / 8, 1 / 4, 1 / 2, 3 / 4, 1, 2, 4, 8, 16, 32],
    defaultBeatIndex: 5,
    // Mixxx's shortest timer. The display predicts the position between
    // Mixxx's 15-per-second position updates, so the bar turns smoothly.
    displayIntervalMs: 20,
    blinkMs: 250,
};

// ---------------------------------------------------------------------------
// MIDI addresses
// ---------------------------------------------------------------------------

// Note status bytes. Control changes on the same MIDI channel are 0x20 higher.
PioneerDDJ1000.midiStatus = {
    deck: 0x90, // MIDI channels 1-4, one per deck
    beatFx: 0x94, // channel 5
    mixer: 0x96, // channel 7: SOUND COLOR FX, sampler, browser
    pads: 0x97, // channels 8-15: two per deck, the second one with SHIFT
    unit: 0x9F, // channel 16: states the mapping reports to the unit
};
PioneerDDJ1000.ccOffset = 0x20;

// Deck buttons. A SHIFT combination has a note (and LED) of its own.
PioneerDDJ1000.deckNote = {
    play: 0x0B, playShift: 0x47,
    cue: 0x0C, cueShift: 0x48,
    loopIn: 0x10, loopInShift: 0x4C,
    loopOut: 0x11, loopOutShift: 0x4D,
    beatLoop4: 0x14, beatLoop4Shift: 0x50,
    slipReverse: 0x15, reverse: 0x38, // REVERSE is SHIFT + SLIP REVERSE
    vinyl: 0x17, // SHIFT + SLIP
    masterTempo: 0x1A, tempoRange: 0x60, // TEMPO RANGE is SHIFT + MASTER TEMPO
    quantize: 0x35, monoSplit: 0x39, // mono split is SHIFT + QUANTIZE
    slip: 0x40,
    headphoneCue: 0x54,
    beatSync: 0x58, syncLeader: 0x5C, // sync leader is SHIFT + BEAT SYNC
    keySync: 0x65,
};

PioneerDDJ1000.deckCc = {
    levelMeter: 0x02,
};

// Channel 16 notes, one per deck: the first deck's note + deck - 1.
PioneerDDJ1000.unitNote = {
    loadAnimation: 0x00, // plays the jog ring's track load animation
    loopRunning: 0x04, // LOOP IN / OUT send LOOP 1/2X / 2X while it is on
    slipOn: 0x24,
};

PioneerDDJ1000.beatFxNote = {
    beatDown: 0x4A, beatUp: 0x4B,
    auto: 0x66, tap: 0x6B, // SHIFT + BEAT < / >
    on: 0x47,
    release: 0x43, // RELEASE FX is SHIFT + BEAT FX ON/OFF
};

PioneerDDJ1000.mixerNote = {
    colorFx: 0x00, // DUB ECHO, PITCH, NOISE, FILTER are 0x00-0x03
    colorFxShift: 0x08, // the same with SHIFT
    samplerCue: 0x69,
};

// Jog dial display. Position bar, BPM, speed and cue marker are 14-bit
// control changes; the others are notes.
PioneerDDJ1000.display = {
    positionBar: 0x14, bpm: 0x15, speed: 0x16, cueMarker: 0x17,
    minutes: 0x42, seconds: 0x43, timeMode: 0x44, key: 0x49, keyShift: 0x4A,
    syncMaster: 0x59, syncOn: 0x5A, ring: 0x5B, info: 0x5D,
};

PioneerDDJ1000.modeButtons = [0x1B, 0x1E, 0x20, 0x22, 0x69, 0x6B, 0x6D, 0x6F];
PioneerDDJ1000.pagePrevNotes = [0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B];
PioneerDDJ1000.pageNextNotes = [0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33];

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

// Hot cues on the pads (two pages of 8).
PioneerDDJ1000.padHotCues = 16;

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

// What each PAD FX pad does, per mode and page:
//   {unit, slot}  hold one effect of an effect unit on this deck
//   {unit}        hold the whole effect unit on this deck
//   {roll}        loop roll of that many beats (slip)
//   {brake}, {spinback}, {reverse}  deck tricks
// All of them act only while the pad is held; see padEffect().
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

// Memory cues (MEMORY / CUE/LOOP CALL) use hot cues 17-36; pads use 1-16.
PioneerDDJ1000.memoryFirst = 17;
PioneerDDJ1000.memoryLast = 36;
PioneerDDJ1000.memoryColor = 0xE8201E;

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

PioneerDDJ1000.decks = [1, 2, 3, 4];
PioneerDDJ1000.deck = {};
// Connections that show Mixxx's state; refreshAll() replays them.
PioneerDDJ1000.connections = [];
// Connections that react to a change (a track load, a pad colour); they are
// never replayed.
PioneerDDJ1000.listeners = [];
PioneerDDJ1000.timer = 0;
PioneerDDJ1000.opened = false;
PioneerDDJ1000.blink = false;
PioneerDDJ1000.blinkAt = 0;
// Pad modes still to draw after a full refresh, one per timer tick, so the
// unit never receives a burst of pad messages.
PioneerDDJ1000.padQueue = [];
// Set while a pad colour is worked out if that colour blinks.
PioneerDDJ1000.padBlinks = false;
PioneerDDJ1000.sent = {};
PioneerDDJ1000.highRes = {};
PioneerDDJ1000.overrides = {};
PioneerDDJ1000.samplersConnected = 0;

PioneerDDJ1000.beatFx = {
    position: 1,
    beatIndex: PioneerDDJ1000.config.defaultBeatIndex,
    // CH SELECT position, or null until it is first moved: the unit does not
    // report it at startup, so effect unit 1 keeps the routing set in Mixxx.
    target: null,
    on: false,
    rolling: [],
    rollSlip: false,
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
        // Pads whose colour blinks, by pad note: [mode, page, pad].
        blinking: {},
        loopAdjust: null,
        search: null,
        searchHeld: false,
        keySync: false,
        reverseTimer: 0,
        gridMode: null,
        gridRemainder: 0,
        // The deck's play and slip state from before a PAD FX brake or
        // backspin, put back when the pad is released.
        trickRestore: null,
        lastLoad: 0,
        loadTimer: 0,
    };
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

PioneerDDJ1000.group = (deck) => `[Channel${deck}]`;

PioneerDDJ1000.deckFromGroup = (group) => script.deckFromGroup(group);

PioneerDDJ1000.deckStatus = (deck) => PioneerDDJ1000.midiStatus.deck + deck - 1;

PioneerDDJ1000.deckCcStatus = (deck) => PioneerDDJ1000.deckStatus(deck) + PioneerDDJ1000.ccOffset;

PioneerDDJ1000.padStatus = (deck, shifted) =>
    PioneerDDJ1000.midiStatus.pads + (deck - 1) * 2 + (shifted ? 1 : 0);

PioneerDDJ1000.clamp = (value, low, high) => Math.max(low, Math.min(high, value));

PioneerDDJ1000.setting = function(name, fallback) {
    const value = engine.getSetting(name);
    return value === undefined ? fallback : value;
};

// Change a Mixxx control the user did not touch, remembering how it was.
// restoreOverrides() puts it back unless it was changed again meanwhile.
PioneerDDJ1000.override = function(group, key, value) {
    const id = `${group},${key}`;
    if (!(id in PioneerDDJ1000.overrides)) {
        PioneerDDJ1000.overrides[id] = {group: group, key: key, original: engine.getValue(group, key)};
    }
    PioneerDDJ1000.overrides[id].value = value;
    engine.setValue(group, key, value);
};

PioneerDDJ1000.restoreOverrides = function() {
    Object.keys(PioneerDDJ1000.overrides).forEach((id) => {
        const changed = PioneerDDJ1000.overrides[id];
        if (engine.getValue(changed.group, changed.key) === changed.value) {
            engine.setValue(changed.group, changed.key, changed.original);
        }
    });
    PioneerDDJ1000.overrides = {};
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
    PioneerDDJ1000.led(PioneerDDJ1000.deckStatus(deck), note, on);
};

// Report a deck's state to the unit (see PioneerDDJ1000.unitNote).
PioneerDDJ1000.sendUnit = function(note, deck, on, force) {
    PioneerDDJ1000.send(PioneerDDJ1000.midiStatus.unit, note + deck - 1, on ? 0x7F : 0x00, force);
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

// 14-bit knobs arrive as MSB then LSB. Returns 0..1 when the LSB completes the
// pair, and undefined for the MSB: combined with the previous LSB it would
// make the value jump until the new LSB arrives.
PioneerDDJ1000.readHighRes = function(status, control, value) {
    const isLsb = control >= 0x20 && control < 0x40;
    const key = `${status}:${isLsb ? control - 0x20 : control}`;
    const pair = PioneerDDJ1000.highRes[key] || (PioneerDDJ1000.highRes[key] = {msb: 0});
    if (!isLsb) {
        pair.msb = value;
        return undefined;
    }
    return ((pair.msb << 7) | value) / 0x3FFF;
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

PioneerDDJ1000.listen = function(group, key, callback) {
    const connection = engine.makeConnection(group, key, callback);
    if (connection) {
        PioneerDDJ1000.listeners.push(connection);
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

PioneerDDJ1000.numSamplers = () => engine.getValue("[App]", "num_samplers");

PioneerDDJ1000.numberedGroups = function(count, name) {
    const groups = [];
    for (let i = 1; i <= count; i++) {
        groups.push(name(i));
    }
    return groups;
};

PioneerDDJ1000.samplerGroups = () =>
    PioneerDDJ1000.numberedGroups(PioneerDDJ1000.numSamplers(), (i) => `[Sampler${i}]`);

PioneerDDJ1000.microphoneGroups = () =>
    PioneerDDJ1000.numberedGroups(engine.getValue("[App]", "num_microphones"),
        (i) => (i === 1 ? "[Microphone]" : `[Microphone${i}]`));

// Every group an effect unit can be routed to.
PioneerDDJ1000.routingGroups = () =>
    PioneerDDJ1000.decks.map(PioneerDDJ1000.group)
        .concat(["[Master]", "[Headphone]"])
        .concat(PioneerDDJ1000.samplerGroups())
        .concat(PioneerDDJ1000.microphoneGroups())
        .concat(PioneerDDJ1000.numberedGroups(engine.getValue("[App]", "num_auxiliaries"),
            (i) => `[Auxiliary${i}]`))
        .concat(PioneerDDJ1000.numberedGroups(engine.getValue("[App]", "num_preview_decks"),
            (i) => `[PreviewDeck${i}]`));

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
            PioneerDDJ1000.override(group, "rateRange", Number(tempoRange) / 100);
        }
        engine.softTakeover(group, "rate", true);
        PioneerDDJ1000.connectDeck(deck);
    });

    if (PioneerDDJ1000.setting("hardwareHeadphoneMix", true)) {
        // The HEADPHONES MIX knob blends cue and main in hardware, so Mixxx
        // sends the cue mix alone to outputs 3-4.
        PioneerDDJ1000.override("[Master]", "headMix", -1);
    }

    PioneerDDJ1000.connectGlobal();
    PioneerDDJ1000.applyColorFx();
    PioneerDDJ1000.timer = engine.beginTimer(PioneerDDJ1000.config.displayIntervalMs, PioneerDDJ1000.tick);
};

PioneerDDJ1000.shutdown = function() {
    if (PioneerDDJ1000.timer) {
        engine.stopTimer(PioneerDDJ1000.timer);
        PioneerDDJ1000.timer = 0;
    }
    PioneerDDJ1000.decks.forEach((deck) => {
        const state = PioneerDDJ1000.deck[deck];
        [state.loadTimer, state.reverseTimer].filter((id) => id).forEach((id) => engine.stopTimer(id));
    });
    Object.keys(PioneerDDJ1000.heldUnits).forEach((unit) => PioneerDDJ1000.releaseUnit(unit));
    PioneerDDJ1000.connections.concat(PioneerDDJ1000.listeners).forEach((connection) => connection.disconnect());
    PioneerDDJ1000.connections = [];
    PioneerDDJ1000.listeners = [];
    PioneerDDJ1000.restoreOverrides();
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
        midi.sendShortMsg(PioneerDDJ1000.deckStatus(deck), PioneerDDJ1000.display.info, 0x7F);
        midi.sendShortMsg(PioneerDDJ1000.deckStatus(deck), PioneerDDJ1000.display.ring, 0x00);
    });
};

PioneerDDJ1000.open = function() {
    PioneerDDJ1000.opened = true;
    PioneerDDJ1000.decks.forEach((deck) => {
        midi.sendShortMsg(PioneerDDJ1000.deckStatus(deck), PioneerDDJ1000.display.info, 0x00);
        midi.sendShortMsg(PioneerDDJ1000.deckStatus(deck), PioneerDDJ1000.display.ring, 0x01);
    });
    PioneerDDJ1000.refreshAll();
};

// Forget what was sent and light everything again (also after a deck switch).
// Only redraws: the track load reaction (KEY SYNC reset, jog ring animation)
// is a listener and is not replayed.
PioneerDDJ1000.refreshAll = function() {
    PioneerDDJ1000.sent = {};
    PioneerDDJ1000.connections.forEach((connection) => connection.trigger());
    PioneerDDJ1000.decks.forEach((deck) => {
        const state = PioneerDDJ1000.deck[deck];
        PioneerDDJ1000.updateModeLeds(deck);
        PioneerDDJ1000.updateLoopLeds(deck);
        PioneerDDJ1000.deckLed(deck, PioneerDDJ1000.deckNote.vinyl, state.vinyl);
        PioneerDDJ1000.deckLed(deck, PioneerDDJ1000.deckNote.keySync, state.keySync);
        PioneerDDJ1000.updateClock(deck);
    });
    PioneerDDJ1000.led(PioneerDDJ1000.midiStatus.beatFx, PioneerDDJ1000.beatFxNote.on, PioneerDDJ1000.beatFx.on);
    PioneerDDJ1000.updateColorFxLeds();
    PioneerDDJ1000.queueAllPads();
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
            PioneerDDJ1000.renderBlinkingPads(deck);
        });
    }
    const queued = PioneerDDJ1000.padQueue.shift();
    if (queued) {
        PioneerDDJ1000.renderMode(queued[0], queued[1]);
    }
    PioneerDDJ1000.decks.forEach((deck) => {
        PioneerDDJ1000.watchFreeSpin(deck, now);
        PioneerDDJ1000.updateSearch(deck, now);
        PioneerDDJ1000.updateClock(deck);
    });
};

// ---------------------------------------------------------------------------
// LEDs driven by Mixxx controls
// ---------------------------------------------------------------------------

PioneerDDJ1000.connectDeck = function(deck) {
    const group = PioneerDDJ1000.group(deck);
    const n = PioneerDDJ1000.deckNote;
    const light = function(key, notes, isOn) {
        PioneerDDJ1000.connect(group, key, (value) => {
            notes.forEach((note) => PioneerDDJ1000.deckLed(deck, note, isOn ? isOn(value) : value > 0));
        });
    };

    light("play_indicator", [n.play, n.playShift]);
    light("cue_indicator", [n.cue, n.cueShift]);
    light("sync_enabled", [n.beatSync]);
    light("sync_mode", [n.syncLeader], (value) => value === 2);
    light("keylock", [n.masterTempo, n.tempoRange]);
    light("loop_enabled", [n.beatLoop4, n.beatLoop4Shift]);
    light("quantize", [n.quantize]);
    light("slip_enabled", [n.slip]);
    light("reverse", [n.reverse]);
    light("reverseroll", [n.slipReverse]);
    light("pfl", [n.headphoneCue]);

    PioneerDDJ1000.connect(group, "loop_enabled", (value) => {
        PioneerDDJ1000.sendUnit(PioneerDDJ1000.unitNote.loopRunning, deck, value > 0);
        PioneerDDJ1000.updateLoopLeds(deck);
    });
    PioneerDDJ1000.connect(group, "slip_enabled", (value) => {
        PioneerDDJ1000.sendUnit(PioneerDDJ1000.unitNote.slipOn, deck, value > 0);
    });
    PioneerDDJ1000.connect(group, "playposition", (value) => {
        PioneerDDJ1000.deck[deck].position = {value: value, at: Date.now()};
    });
    PioneerDDJ1000.connect(group, "vu_meter", (value) => {
        PioneerDDJ1000.send(PioneerDDJ1000.deckCcStatus(deck), PioneerDDJ1000.deckCc.levelMeter, Math.round(value * 127));
    });
    ["track_loaded", "track_samplerate", "cue_point", "bpm", "rate_ratio", "key", "pitch", "sync_mode", "sync_enabled"]
        .forEach((key) => PioneerDDJ1000.connect(group, key, () => PioneerDDJ1000.updateDisplayInfo(deck)));

    // Mixxx sets track_loaded on every load, also when one track replaces
    // another. A listener, so a redraw does not count as a load.
    PioneerDDJ1000.listen(group, "track_loaded", (value) => PioneerDDJ1000.onLoad(deck, value > 0));
    PioneerDDJ1000.connectPads(deck);
};

// Pad colours are redrawn when the Mixxx controls they show change.
PioneerDDJ1000.connectPads = function(deck) {
    const group = PioneerDDJ1000.group(deck);
    const m = PioneerDDJ1000.mode;
    for (let number = 1; number <= PioneerDDJ1000.padHotCues; number++) {
        const page = number > 8 ? 2 : 1;
        const pad = (number - 1) % 8;
        ["status", "color", "type"].forEach((property) => {
            PioneerDDJ1000.listen(group, `hotcue_${number}_${property}`, () => {
                PioneerDDJ1000.renderPad(deck, m.hotCue, page, pad);
            });
        });
    }
    PioneerDDJ1000.listen(group, "pitch_adjust", () => {
        PioneerDDJ1000.renderMode(deck, m.keyboard);
        PioneerDDJ1000.renderMode(deck, m.keyShift);
    });
    ["loop_enabled", "beatloop_size"].forEach((key) => {
        PioneerDDJ1000.listen(group, key, () => PioneerDDJ1000.renderMode(deck, m.beatLoop));
    });
};

PioneerDDJ1000.connectGlobal = function() {
    PioneerDDJ1000.connect("[Master]", "headSplit", (value) => {
        PioneerDDJ1000.decks.forEach((deck) => PioneerDDJ1000.deckLed(deck, PioneerDDJ1000.deckNote.monoSplit, value > 0));
    });
    PioneerDDJ1000.connect(PioneerDDJ1000.beatFxUnit, "enabled", () => {
        PioneerDDJ1000.led(PioneerDDJ1000.midiStatus.beatFx, PioneerDDJ1000.beatFxNote.on, PioneerDDJ1000.beatFx.on);
    });
    // Samplers can be added later (by the skin); connect those too.
    PioneerDDJ1000.listen("[App]", "num_samplers", PioneerDDJ1000.connectSamplers);
    PioneerDDJ1000.connectSamplers();
};

// Connect the samplers added since the last call.
PioneerDDJ1000.connectSamplers = function() {
    const count = PioneerDDJ1000.numSamplers();
    if (count <= PioneerDDJ1000.samplersConnected) {
        return;
    }
    for (let i = PioneerDDJ1000.samplersConnected + 1; i <= count; i++) {
        const group = `[Sampler${i}]`;
        PioneerDDJ1000.connect(group, "pfl", PioneerDDJ1000.updateSamplerCueLed);
        PioneerDDJ1000.listen(group, "track_loaded", () => PioneerDDJ1000.renderSamplerPad(i));
        PioneerDDJ1000.listen(group, "play", () => PioneerDDJ1000.renderSamplerPad(i));
    }
    PioneerDDJ1000.samplersConnected = count;
    PioneerDDJ1000.decks.forEach((deck) => PioneerDDJ1000.renderMode(deck, PioneerDDJ1000.mode.sampler));
};

PioneerDDJ1000.updateSamplerCueLed = function() {
    const cued = PioneerDDJ1000.samplerGroups().some((group) => engine.getValue(group, "pfl") > 0);
    PioneerDDJ1000.led(PioneerDDJ1000.midiStatus.mixer, PioneerDDJ1000.mixerNote.samplerCue, cued);
};

PioneerDDJ1000.onLoad = function(deck, loaded) {
    const state = PioneerDDJ1000.deck[deck];
    state.keySync = false;
    PioneerDDJ1000.deckLed(deck, PioneerDDJ1000.deckNote.keySync, false);
    if (!loaded || !PioneerDDJ1000.opened) {
        return;
    }
    PioneerDDJ1000.sendUnit(PioneerDDJ1000.unitNote.loadAnimation, deck, true, true);
    if (state.loadTimer) {
        engine.stopTimer(state.loadTimer);
    }
    state.loadTimer = engine.beginTimer(1000, () => {
        state.loadTimer = 0;
        PioneerDDJ1000.sendUnit(PioneerDDJ1000.unitNote.loadAnimation, deck, false, true);
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

// Mixxx updates playposition only 15 times a second. Between updates,
// advance it by the time passed at the current playing speed.
PioneerDDJ1000.estimateElapsed = function(deck, group, duration) {
    const sample = PioneerDDJ1000.deck[deck].position;
    const reported = PioneerDDJ1000.clamp(engine.getValue(group, "playposition"), 0, 1) * duration;
    if (!sample || !engine.getValue(group, "play") || engine.isScratching(deck) || duration <= 0) {
        return reported;
    }
    const speed = (engine.getValue(group, "rate_ratio") || 1) * (engine.getValue(group, "reverse") ? -1 : 1);
    const seconds = Math.min(0.1, (Date.now() - sample.at) / 1000);
    return PioneerDDJ1000.clamp(PioneerDDJ1000.clamp(sample.value, 0, 1) * duration + seconds * speed, 0, duration);
};

// Display values that change only with the track, the tempo or a button.
PioneerDDJ1000.updateDisplayInfo = function(deck) {
    const group = PioneerDDJ1000.group(deck);
    const note = PioneerDDJ1000.deckStatus(deck);
    const cc = PioneerDDJ1000.deckCcStatus(deck);
    const d = PioneerDDJ1000.display;

    PioneerDDJ1000.send(note, d.syncMaster, engine.getValue(group, "sync_mode") === 2 ? 0x7F : 0);
    PioneerDDJ1000.send(note, d.syncOn, engine.getValue(group, "sync_enabled") ? 0x7F : 0);

    if (!engine.getValue(group, "track_loaded")) {
        PioneerDDJ1000.send14(cc, d.bpm, 0);
        PioneerDDJ1000.send14(cc, d.cueMarker, 0x3FFF);
        PioneerDDJ1000.send(note, d.key, 0);
        PioneerDDJ1000.send(note, d.keyShift, 0x0D);
        return;
    }

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

// From the timer: the time and the position bar.
PioneerDDJ1000.updateClock = function(deck) {
    const group = PioneerDDJ1000.group(deck);
    const note = PioneerDDJ1000.deckStatus(deck);
    const d = PioneerDDJ1000.display;

    if (!engine.getValue(group, "track_loaded")) {
        PioneerDDJ1000.send(note, d.minutes, 0);
        PioneerDDJ1000.send(note, d.seconds, 0);
        return;
    }
    const duration = engine.getValue(group, "duration");
    const elapsed = PioneerDDJ1000.estimateElapsed(deck, group, duration);
    const remaining = engine.getValue("[Controls]", "ShowDurationRemaining") !== 0;
    const shown = remaining ? Math.max(0, duration - elapsed) : elapsed;
    PioneerDDJ1000.send(note, d.timeMode, remaining ? 0x7F : 0);
    PioneerDDJ1000.send(note, d.minutes, Math.min(99, Math.floor(shown / 60)));
    PioneerDDJ1000.send(note, d.seconds, Math.floor(shown % 60));
    PioneerDDJ1000.send14(PioneerDDJ1000.deckCcStatus(deck), d.positionBar, PioneerDDJ1000.platterAngle(elapsed));
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
    const position = PioneerDDJ1000.readHighRes(status, control, value);
    if (position !== undefined) {
        engine.setValue(group, "rate", (position * 2 - 1) * (engine.getValue(group, "rate_dir") || 1));
    }
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
    PioneerDDJ1000.deckLed(deck, PioneerDDJ1000.deckNote.keySync, state.keySync);
};

PioneerDDJ1000.keyReset = function(_channel, _control, value, _status, group) {
    if (!value) {
        return;
    }
    const deck = PioneerDDJ1000.deckFromGroup(group);
    engine.setValue(group, "reset_key", 1);
    PioneerDDJ1000.deck[deck].keySync = false;
    PioneerDDJ1000.deckLed(deck, PioneerDDJ1000.deckNote.keySync, false);
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
    const n = PioneerDDJ1000.deckNote;
    const looping = engine.getValue(PioneerDDJ1000.group(deck), "loop_enabled") > 0;
    const lit = (end) => {
        if (state.loopAdjust === end) {
            return PioneerDDJ1000.blink;
        }
        return state.loopAdjust ? true : looping;
    };
    PioneerDDJ1000.deckLed(deck, n.loopIn, lit("in"));
    PioneerDDJ1000.deckLed(deck, n.loopInShift, lit("in"));
    PioneerDDJ1000.deckLed(deck, n.loopOut, lit("out"));
    PioneerDDJ1000.deckLed(deck, n.loopOutShift, lit("out"));
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
    PioneerDDJ1000.deckLed(deck, PioneerDDJ1000.deckNote.vinyl, on);
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

// SHIFT + QUANTIZE: headphones mono split (cue in the left ear, main in the
// right). The HEADPHONES MIXING knob should be fully on CUE while it is on.
PioneerDDJ1000.monoSplit = function(_channel, _control, value) {
    if (value) {
        PioneerDDJ1000.toggle("[Master]", "headSplit");
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
            // A called cue becomes the CUE point, as on a CDJ. This replaces
            // the track's stored cue point (noted in the manual).
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
    state.settling = false;
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
    } else if (state.settling) {
        // The platter is still coasting after a scratch or backspin; bending
        // with it would play the track off-speed (and off-key).
        state.settledAt = Date.now();
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
    if (state.settling && now - state.settledAt > 150) {
        state.settling = false; // platter has come to rest
    }
    if (state.touched || !engine.isScratching(deck)) {
        state.freeSpin = false;
        return;
    }
    const lastReport = state.spinTicks.length ? state.spinTicks[state.spinTicks.length - 1][0] : 0;
    if (!state.freeSpin || now - lastReport > 60
            || PioneerDDJ1000.spinSpeed(state, now) < PioneerDDJ1000.config.freeSpinMinSpeed) {
        state.freeSpin = false;
        state.settling = true;
        state.settledAt = now;
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
        // On the tree: open the item and go to its track list. Mixxx never
        // opens Tracks or Auto DJ from a controller, so move focus as well.
        engine.setValue("[Library]", "GoToItem", 1);
        if (engine.getValue("[Library]", "focused_widget") === 2) {
            engine.setValue("[Library]", "focused_widget", 3);
        }
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
    if (level !== undefined) {
        PioneerDDJ1000.samplerGroups().forEach((group) => engine.setParameter(group, "volume", level));
    }
};

PioneerDDJ1000.samplerCue = function(_channel, _control, value) {
    if (!value) {
        return;
    }
    const on = engine.getValue("[Sampler1]", "pfl") ? 0 : 1;
    PioneerDDJ1000.samplerGroups().forEach((group) => engine.setValue(group, "pfl", on));
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
            PioneerDDJ1000.override(unit, "enabled", which ? 1 : 0);
        }
    });
    PioneerDDJ1000.updateColorFxLeds();
};

PioneerDDJ1000.updateColorFxLeds = function() {
    const mixer = PioneerDDJ1000.midiStatus.mixer;
    for (let i = 0; i < 4; i++) {
        PioneerDDJ1000.led(mixer, PioneerDDJ1000.mixerNote.colorFx + i, PioneerDDJ1000.colorFx === i + 1);
        PioneerDDJ1000.led(mixer, PioneerDDJ1000.mixerNote.colorFxShift + i, PioneerDDJ1000.colorFx === i + 1);
    }
};

// ---------------------------------------------------------------------------
// BEAT FX (effect unit 1)
// ---------------------------------------------------------------------------

PioneerDDJ1000.beatFxUnit = "[EffectRack1_EffectUnit1]";

PioneerDDJ1000.beatFxLed = function(note, on) {
    PioneerDDJ1000.led(PioneerDDJ1000.midiStatus.beatFx, note, on);
};

PioneerDDJ1000.rollType = function() {
    const name = PioneerDDJ1000.beatFxPositions[PioneerDDJ1000.beatFx.position];
    if (name === "SLIP ROLL") {
        return "slip";
    }
    return name === "ROLL" ? "loop" : null;
};

PioneerDDJ1000.beatValue = () => PioneerDDJ1000.config.beatValues[PioneerDDJ1000.beatFx.beatIndex];

// CH SELECT: route effect unit 1 to the selected source alone. MIC is
// Mixxx's microphone channels.
PioneerDDJ1000.routeBeatFx = function(target) {
    let routed;
    if (typeof target === "number") {
        routed = [PioneerDDJ1000.group(target)];
    } else if (target === "master") {
        routed = ["[Master]"];
    } else if (target === "mic") {
        routed = PioneerDDJ1000.microphoneGroups();
    } else {
        routed = PioneerDDJ1000.samplerGroups();
    }
    PioneerDDJ1000.routingGroups().forEach((group) => {
        PioneerDDJ1000.override(PioneerDDJ1000.beatFxUnit, `group_${group}_enable`, routed.includes(group) ? 1 : 0);
    });
};

// The decks a roll plays on: the decks effect unit 1 is routed to, or every
// playing deck when it is routed to Main.
PioneerDDJ1000.rollDecks = function() {
    const unit = PioneerDDJ1000.beatFxUnit;
    const onMain = engine.getValue(unit, "group_[Master]_enable") > 0;
    return PioneerDDJ1000.decks.filter((deck) => {
        const group = PioneerDDJ1000.group(deck);
        return onMain ? engine.getValue(group, "play") > 0 : engine.getValue(unit, `group_${group}_enable`) > 0;
    });
};

// FX SELECT: position N loads effect unit 1's chain preset N (the order is
// set in Preferences > Effects); SLIP ROLL and ROLL use the deck's loops.
// Preset 0 is always Mixxx's empty "---" preset, so LOW CUT ECHO loads the
// first preset of the list.
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
    PioneerDDJ1000.routeBeatFx(target);
    PioneerDDJ1000.setBeatFx(wasOn);
};

PioneerDDJ1000.beatFxLevel = function(_channel, control, value, status) {
    const level = PioneerDDJ1000.readHighRes(status, control, value);
    if (level !== undefined) {
        engine.setParameter(PioneerDDJ1000.beatFxUnit, "mix", level);
    }
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
    PioneerDDJ1000.beatFxLed(PioneerDDJ1000.beatFxNote.on, on);
};

PioneerDDJ1000.startRoll = function(deck, beats, slip) {
    const group = PioneerDDJ1000.group(deck);
    engine.setValue(group, "beatloop_size", beats);
    engine.setValue(group, slip ? "beatlooproll_activate" : "beatloop_activate", 1);
};

PioneerDDJ1000.stopRoll = function(deck, slip) {
    engine.setValue(PioneerDDJ1000.group(deck), slip ? "beatlooproll_activate" : "loop_enabled", 0);
};

// SHIFT + BEAT FX ON/OFF: RELEASE FX switches the Beat FX off. With CH SELECT
// on one deck (CH1-CH4) that deck also stops with a vinyl brake; MASTER, MIC
// and SAMPLER never stop a deck.
PioneerDDJ1000.releaseFx = function(_channel, _control, value) {
    PioneerDDJ1000.beatFxLed(PioneerDDJ1000.beatFxNote.release, value > 0);
    if (value) {
        const target = PioneerDDJ1000.beatFx.target;
        PioneerDDJ1000.setBeatFx(false);
        if (typeof target === "number") {
            engine.brake(target, true);
        }
    }
};

// BEAT < / >: the roll length, or the first effect's first parameter (its
// time for echo, flanger, phaser and similar effects).
PioneerDDJ1000.beatStep = function(direction, note) {
    return function(_channel, _control, value) {
        PioneerDDJ1000.beatFxLed(note, value > 0);
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

PioneerDDJ1000.beatDown = PioneerDDJ1000.beatStep(-1, PioneerDDJ1000.beatFxNote.beatDown);
PioneerDDJ1000.beatUp = PioneerDDJ1000.beatStep(1, PioneerDDJ1000.beatFxNote.beatUp);

// SHIFT + BEAT < / > (AUTO / TAP in rekordbox): Mixxx effects follow the deck
// tempo on their own, so these only light up.
PioneerDDJ1000.beatAuto = function(_channel, _control, value) {
    PioneerDDJ1000.beatFxLed(PioneerDDJ1000.beatFxNote.auto, value > 0);
};

PioneerDDJ1000.beatTap = function(_channel, _control, value) {
    PioneerDDJ1000.beatFxLed(PioneerDDJ1000.beatFxNote.tap, value > 0);
};

// ---------------------------------------------------------------------------
// Performance pads: modes and pages
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

// Banks of 8 samplers that exist in Mixxx.
PioneerDDJ1000.samplerBankCount = () =>
    PioneerDDJ1000.clamp(Math.ceil(PioneerDDJ1000.numSamplers() / 8), 1, PioneerDDJ1000.config.samplerBanks);

PioneerDDJ1000.stepSamplerBank = function(deck, direction) {
    const state = PioneerDDJ1000.deck[deck];
    state.samplerBank = PioneerDDJ1000.clamp(state.samplerBank + direction, 0, PioneerDDJ1000.samplerBankCount() - 1);
    PioneerDDJ1000.renderMode(deck, PioneerDDJ1000.mode.sampler);
};

PioneerDDJ1000.updateModeLeds = function(deck) {
    const state = PioneerDDJ1000.deck[deck];
    if (!state) {
        return;
    }
    // Lighting a mode button tells the unit to switch to that mode, so only
    // the active mode is lit, never blinked; PAGE lights are left to the unit.
    PioneerDDJ1000.modeButtons.forEach((note, mode) => {
        PioneerDDJ1000.deckLed(deck, note, mode === state.mode);
    });
};

// ---------------------------------------------------------------------------
// Performance pads: what a press does
// ---------------------------------------------------------------------------

// Pads: note = mode * 16 + (page - 1) * 8 + pad; SHIFT uses the next channel.
PioneerDDJ1000.pad = function(_channel, control, value, status) {
    const channelOffset = (status & 0x0F) - (PioneerDDJ1000.midiStatus.pads & 0x0F);
    const deck = (channelOffset >> 1) + 1;
    const shifted = (channelOffset & 1) === 1;
    const mode = control >> 4;
    const page = control & 0x08 ? 2 : 1;
    const pad = control & 0x07;
    const state = PioneerDDJ1000.deck[deck];

    // The unit changes mode by itself; a pad of another mode means it did.
    if (state.mode !== mode) {
        state.mode = mode;
        PioneerDDJ1000.updateModeLeds(deck);
    }
    state.pressed[mode] = state.pressed[mode] || {};
    state.pressed[mode][pad] = value > 0;
    PioneerDDJ1000.padActions[mode](deck, page, pad, value, shifted);
    PioneerDDJ1000.renderMode(deck, mode);
};

PioneerDDJ1000.padActions = {
    [PioneerDDJ1000.mode.hotCue]: (deck, page, pad, value, shifted) =>
        PioneerDDJ1000.padHotCue(deck, pad + 1 + (page - 1) * 8, value, shifted),
    [PioneerDDJ1000.mode.padFx1]: (deck, page, pad, value) =>
        PioneerDDJ1000.padEffect(deck, PioneerDDJ1000.padFx[PioneerDDJ1000.mode.padFx1][page - 1][pad], value),
    [PioneerDDJ1000.mode.padFx2]: (deck, page, pad, value) =>
        PioneerDDJ1000.padEffect(deck, PioneerDDJ1000.padFx[PioneerDDJ1000.mode.padFx2][page - 1][pad], value),
    [PioneerDDJ1000.mode.beatJump]: (deck, page, pad, value) =>
        PioneerDDJ1000.padBeatJump(deck, pad, value),
    [PioneerDDJ1000.mode.sampler]: (deck, page, pad, value, shifted) =>
        PioneerDDJ1000.padSampler(PioneerDDJ1000.deck[deck].samplerBank * 8 + pad + 1, value, shifted),
    [PioneerDDJ1000.mode.keyboard]: (deck, page, pad, value, shifted) =>
        PioneerDDJ1000.padKeyboard(deck, PioneerDDJ1000.semitones[page - 1][pad], pad, value, shifted),
    [PioneerDDJ1000.mode.beatLoop]: (deck, page, pad, value, shifted) =>
        PioneerDDJ1000.padBeatLoop(deck, PioneerDDJ1000.beatLoopBeats[page - 1][pad], value && !shifted),
    [PioneerDDJ1000.mode.keyShift]: (deck, page, pad, value) =>
        PioneerDDJ1000.padKeyShift(deck, PioneerDDJ1000.semitones[page - 1][pad], value),
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

// PAD FX (see PioneerDDJ1000.padFx). Each acts only while its pad is held and
// leaves the deck or effect unit as it found it when the pad is released.
PioneerDDJ1000.padEffect = function(deck, entry, value) {
    if (entry.roll !== undefined) {
        PioneerDDJ1000.padRoll(deck, entry.roll, value);
    } else if (entry.brake || entry.spinback) {
        PioneerDDJ1000.padTrick(deck, entry.spinback, value);
    } else if (entry.reverse) {
        // Mixxx's slip reverse returns to where the track would have been.
        engine.setValue(PioneerDDJ1000.group(deck), "reverseroll", value ? 1 : 0);
    } else if (value) {
        PioneerDDJ1000.holdUnit(deck, entry.unit, entry.slot);
    } else {
        PioneerDDJ1000.releaseUnit(entry.unit);
    }
};

// A slip loop roll while held.
PioneerDDJ1000.padRoll = function(deck, beats, value) {
    if (value) {
        PioneerDDJ1000.startRoll(deck, beats, true);
    } else {
        PioneerDDJ1000.stopRoll(deck, true);
    }
};

// Vinyl brake or backspin while held. Slip keeps the track running
// underneath, so a deck that was playing carries on where it would have been;
// a deck that was paused stays paused, where it was.
PioneerDDJ1000.padTrick = function(deck, spinback, value) {
    const group = PioneerDDJ1000.group(deck);
    const state = PioneerDDJ1000.deck[deck];
    if (value) {
        // With two trick pads held, keep the state from before the first.
        if (!state.trickRestore) {
            state.trickRestore = {
                play: engine.getValue(group, "play"),
                slip: engine.getValue(group, "slip_enabled"),
            };
        }
        engine.setValue(group, "slip_enabled", 1);
    }
    if (spinback) {
        engine.spinback(deck, value > 0);
    } else {
        engine.brake(deck, value > 0);
    }
    if (!value && state.trickRestore) {
        // Play first: switching slip off then jumps to the slip position.
        if (state.trickRestore.play) {
            engine.setValue(group, "play", 1);
        }
        engine.setValue(group, "slip_enabled", state.trickRestore.slip);
        state.trickRestore = null;
    }
};

// Hold an effect unit, or one effect of it (slot), on this deck alone.
// Everything changed here is saved and put back by releaseUnit().
PioneerDDJ1000.holdUnit = function(deck, unitNumber, slot) {
    const unit = `[EffectRack1_EffectUnit${unitNumber}]`;
    const deckGroup = PioneerDDJ1000.group(deck);
    if (PioneerDDJ1000.heldUnits[unitNumber]) {
        PioneerDDJ1000.releaseUnit(unitNumber);
    }
    const saved = {enabled: engine.getValue(unit, "enabled"), routing: {}, slots: {}};
    // Every route, not just the decks': a unit also routed to Main or to the
    // samplers would otherwise still reach the whole mix.
    PioneerDDJ1000.routingGroups().forEach((group) => {
        const key = `group_${group}_enable`;
        saved.routing[key] = engine.getValue(unit, key);
        engine.setValue(unit, key, group === deckGroup ? 1 : 0);
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
    Object.keys(saved.routing).forEach((key) => engine.setValue(unit, key, saved.routing[key]));
    Object.keys(saved.slots).forEach((slotGroup) => engine.setValue(slotGroup, "enabled", saved.slots[slotGroup]));
    delete PioneerDDJ1000.heldUnits[unitNumber];
};

PioneerDDJ1000.padBeatJump = function(deck, pad, value) {
    if (value) {
        engine.setValue(PioneerDDJ1000.group(deck), "beatjump",
            PioneerDDJ1000.beatJumpBeats[pad] * PioneerDDJ1000.deck[deck].jumpScale);
    }
};

// SAMPLER: play the slot from the start; SHIFT stops it; an empty slot loads
// the selected track. Pads past the last sampler do nothing.
PioneerDDJ1000.padSampler = function(number, value, shifted) {
    if (!value || number > PioneerDDJ1000.numSamplers()) {
        return;
    }
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

// BEAT LOOP: a loop of that size; the lit pad again leaves it.
PioneerDDJ1000.padBeatLoop = function(deck, beats, pressed) {
    if (!pressed) {
        return;
    }
    const group = PioneerDDJ1000.group(deck);
    const sameSize = Math.abs(engine.getValue(group, "beatloop_size") - beats) < 1e-6;
    if (engine.getValue(group, "loop_enabled") && sameSize) {
        engine.setValue(group, "loop_enabled", 0);
    } else {
        engine.setValue(group, "beatloop_size", beats);
        engine.setValue(group, "beatloop_activate", 1);
    }
};

// KEY SHIFT: shift the key; the lit pad again returns to 0.
PioneerDDJ1000.padKeyShift = function(deck, semitones, value) {
    if (value) {
        const group = PioneerDDJ1000.group(deck);
        const current = Math.round(engine.getValue(group, "pitch_adjust"));
        engine.setValue(group, "pitch_adjust", current === semitones ? 0 : semitones);
    }
};

// ---------------------------------------------------------------------------
// Performance pads: colours
// ---------------------------------------------------------------------------

// Pad colours are sent when something they show changes (see connectPads()
// and connectSamplers()), when a pad is pressed, and on the blink for the
// pads that blink. send() skips colours the unit already has.

// Send one pad's colour on both its channels (with and without SHIFT).
PioneerDDJ1000.renderPad = function(deck, mode, page, pad) {
    const state = PioneerDDJ1000.deck[deck];
    if (!state || !PioneerDDJ1000.opened) {
        return;
    }
    const note = mode * 16 + (page - 1) * 8 + pad;
    PioneerDDJ1000.padBlinks = false;
    const colour = PioneerDDJ1000.padColors[mode](deck, page, pad);
    if (PioneerDDJ1000.padBlinks) {
        state.blinking[note] = [mode, page, pad];
    } else {
        delete state.blinking[note];
    }
    PioneerDDJ1000.send(PioneerDDJ1000.padStatus(deck, false), note, colour);
    PioneerDDJ1000.send(PioneerDDJ1000.padStatus(deck, true), note, colour);
};

PioneerDDJ1000.renderMode = function(deck, mode) {
    for (let page = 1; page <= 2; page++) {
        for (let pad = 0; pad < 8; pad++) {
            PioneerDDJ1000.renderPad(deck, mode, page, pad);
        }
    }
};

PioneerDDJ1000.renderBlinkingPads = function(deck) {
    const blinking = PioneerDDJ1000.deck[deck].blinking;
    Object.keys(blinking).forEach((note) => {
        const [mode, page, pad] = blinking[note];
        PioneerDDJ1000.renderPad(deck, mode, page, pad);
    });
};

// Every pad of every deck: the modes on screen now, the rest from the timer.
PioneerDDJ1000.queueAllPads = function() {
    PioneerDDJ1000.padQueue = [];
    PioneerDDJ1000.decks.forEach((deck) => {
        const onScreen = PioneerDDJ1000.deck[deck].mode;
        PioneerDDJ1000.renderMode(deck, onScreen);
        for (let mode = 0; mode < 8; mode++) {
            if (mode !== onScreen) {
                PioneerDDJ1000.padQueue.push([deck, mode]);
            }
        }
    });
};

// A sampler changed: redraw its pad on the decks showing its bank.
PioneerDDJ1000.renderSamplerPad = function(number) {
    const bank = Math.floor((number - 1) / 8);
    const pad = (number - 1) % 8;
    PioneerDDJ1000.decks.forEach((deck) => {
        if (PioneerDDJ1000.deck[deck].samplerBank === bank) {
            PioneerDDJ1000.renderPad(deck, PioneerDDJ1000.mode.sampler, 1, pad);
            PioneerDDJ1000.renderPad(deck, PioneerDDJ1000.mode.sampler, 2, pad);
        }
    });
};

// A blinking colour: `on` on the blink, `off` in between. Marks the pad so
// the timer redraws it on every blink.
PioneerDDJ1000.blinkColor = function(on, off) {
    PioneerDDJ1000.padBlinks = true;
    return PioneerDDJ1000.blink ? on : off;
};

PioneerDDJ1000.isPressed = function(deck, mode, pad) {
    const pressed = PioneerDDJ1000.deck[deck].pressed[mode];
    return !!(pressed && pressed[pad]);
};

PioneerDDJ1000.hotCueColor = function(deck, number) {
    const group = PioneerDDJ1000.group(deck);
    const status = engine.getValue(group, `hotcue_${number}_status`);
    if (!status) {
        return PioneerDDJ1000.color.off;
    }
    const colour = PioneerDDJ1000.colorMapper.getValueForNearestColor(
        engine.getValue(group, `hotcue_${number}_color`));
    // A saved loop that is running blinks.
    if (status === 2 && engine.getValue(group, `hotcue_${number}_type`) === 4) {
        return PioneerDDJ1000.blinkColor(colour, PioneerDDJ1000.color.off);
    }
    return colour;
};

PioneerDDJ1000.padFxColor = function(deck, mode, page, pad) {
    return PioneerDDJ1000.isPressed(deck, mode, pad)
        ? PioneerDDJ1000.color.white
        : PioneerDDJ1000.padFx[mode][page - 1][pad].color;
};

PioneerDDJ1000.beatJumpColor = function(deck, pad) {
    const c = PioneerDDJ1000.color;
    if (PioneerDDJ1000.isPressed(deck, PioneerDDJ1000.mode.beatJump, pad)) {
        return c.white;
    }
    return PioneerDDJ1000.beatJumpBeats[pad] < 0 ? c.azure : c.cyan;
};

PioneerDDJ1000.samplerColor = function(deck, pad) {
    const c = PioneerDDJ1000.color;
    const number = PioneerDDJ1000.deck[deck].samplerBank * 8 + pad + 1;
    if (number > PioneerDDJ1000.numSamplers()) {
        return c.off;
    }
    const sampler = `[Sampler${number}]`;
    if (!engine.getValue(sampler, "track_loaded")) {
        return c.off;
    }
    return engine.getValue(sampler, "play") ? PioneerDDJ1000.blinkColor(c.white, c.magenta) : c.magenta;
};

// KEYBOARD and KEY SHIFT: the current key shift is white, no shift yellow.
PioneerDDJ1000.semitoneColor = function(deck, mode, page, pad) {
    const c = PioneerDDJ1000.color;
    const semitones = PioneerDDJ1000.semitones[page - 1][pad];
    if (Math.round(engine.getValue(PioneerDDJ1000.group(deck), "pitch_adjust")) === semitones) {
        return c.white;
    }
    if (semitones === 0) {
        return c.yellow;
    }
    return mode === PioneerDDJ1000.mode.keyboard ? c.purple : c.pink;
};

PioneerDDJ1000.beatLoopColor = function(deck, page, pad) {
    const c = PioneerDDJ1000.color;
    const group = PioneerDDJ1000.group(deck);
    const beats = PioneerDDJ1000.beatLoopBeats[page - 1][pad];
    const active = engine.getValue(group, "loop_enabled")
        && Math.abs(engine.getValue(group, "beatloop_size") - beats) < 1e-6;
    return active ? PioneerDDJ1000.blinkColor(c.white, c.green) : c.green;
};

PioneerDDJ1000.padColors = {
    [PioneerDDJ1000.mode.hotCue]: (deck, page, pad) => PioneerDDJ1000.hotCueColor(deck, pad + 1 + (page - 1) * 8),
    [PioneerDDJ1000.mode.padFx1]: (deck, page, pad) =>
        PioneerDDJ1000.padFxColor(deck, PioneerDDJ1000.mode.padFx1, page, pad),
    [PioneerDDJ1000.mode.padFx2]: (deck, page, pad) =>
        PioneerDDJ1000.padFxColor(deck, PioneerDDJ1000.mode.padFx2, page, pad),
    [PioneerDDJ1000.mode.beatJump]: (deck, page, pad) => PioneerDDJ1000.beatJumpColor(deck, pad),
    [PioneerDDJ1000.mode.sampler]: (deck, page, pad) => PioneerDDJ1000.samplerColor(deck, pad),
    [PioneerDDJ1000.mode.keyboard]: (deck, page, pad) =>
        PioneerDDJ1000.semitoneColor(deck, PioneerDDJ1000.mode.keyboard, page, pad),
    [PioneerDDJ1000.mode.beatLoop]: (deck, page, pad) => PioneerDDJ1000.beatLoopColor(deck, page, pad),
    [PioneerDDJ1000.mode.keyShift]: (deck, page, pad) =>
        PioneerDDJ1000.semitoneColor(deck, PioneerDDJ1000.mode.keyShift, page, pad),
};
