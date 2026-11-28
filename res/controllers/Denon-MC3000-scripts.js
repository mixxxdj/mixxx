// ****************************************************************************
// Denon MC3000 - Mixxx controller mapping
//
// [AI disclaimer: the comments in this file were written autonomously by an AI
//  agent (Claude) and have to be reviewed by the human contributor.]
//
// Rewritten from the original mapping by Bertrand Espern (2012).
//
// LAYOUT
// ----------------------------------------------------------------------------
// The MC3000 changes MIDI channel with the DECK CHG buttons:
//     channel 1 (0x_0) = DECK A -> Mixxx [Channel1]   (LEFT side)
//     channel 2 (0x_1) = DECK C -> Mixxx [Channel3]   (LEFT side)
//     channel 3 (0x_2) = DECK B -> Mixxx [Channel2]   (RIGHT side)
//     channel 4 (0x_3) = DECK D -> Mixxx [Channel4]   (RIGHT side)
// The deck section (play, cue, sync, loops, jog, pitch, EQ, hot cues...)
// follows the selected deck, so MC3000.deckGroup() resolves the deck straight
// from the MIDI channel of the incoming message. The physical SHIFT buttons
// are tracked per side (note 96 = left, 97 = right).
//
// LOAD A/C and LOAD B/D are the exception: they always arrive on the same
// channel. They, the FX-follows-deck routing and the LEDs that react to engine
// state changes use MC3000.deckOnSide[], the deck each side is showing.
// It is updated by DECK CHG A/B/C/D and CHANNEL SELECT A/C, B/D (see
// MC3000.deckButton()).
//
// To change which Mixxx deck a physical deck controls, edit the deckFor* table
// (search "DECK ASSIGNMENT" below) and the matching <group> entries in the XML.
//
// Effects use the standard ComponentsJS EffectUnit (midi-components-0.0.js),
// see "EFX SECTION" below. Options are in Preferences -> Controllers.
//
// See the "CONTROL REFERENCE" comment at the bottom of this file for the full
// button-by-button description, including every SHIFT function.
//
// [End of AI-written comment: see the disclaimer above.]
// ****************************************************************************

// eslint-disable-next-line no-var
var MC3000 = {};

// ============================================================================
//  SETTINGS - set in Preferences -> Controllers (<settings> in the XML file).
//  engine.getSetting() only exists from Mixxx 2.4 on; the fallbacks below are
//  the same as the defaults declared in the XML file.
// ============================================================================

MC3000.setting = function(name, fallback) {
    if (typeof engine.getSetting !== "function") {
        return fallback;
    }
    const value = engine.getSetting(name);
    return (value === undefined || value === null) ? fallback : value;
};

MC3000.loadSettings = function() {
    // Jog wheel: MIDI ticks per revolution (the encoder is specified at 2048).
    // Lower it if scratching feels too slow, raise it if it feels twitchy.
    MC3000.jogTicksPerRevolution = Number(MC3000.setting("jogTicksPerRevolution", 2048));
    // Pitch-bend strength when the wheel is turned without touching the top.
    MC3000.jogBendSensitivity = Number(MC3000.setting("jogBendSensitivity", 0.6));
    // VINYL MODE state at start-up (true = scratch, false = pitch bend).
    MC3000.vinylModeAtStartup = Boolean(MC3000.setting("vinylModeAtStartup", true));
    // KEY LOCK state at start-up, applied to all four decks: "on", "off" or
    // "keep" (leave whatever Mixxx has for each deck).
    MC3000.keylockAtStartup = String(MC3000.setting("keylockAtStartup", "on"));

    // Set to true if moving the pitch fader towards "+" slows the track down.
    MC3000.invertPitchFader = Boolean(MC3000.setting("invertPitchFader", false));
    // Soft takeover: the control is ignored after a deck switch until it
    // reaches the value of the newly selected deck. Prevents jumps, but the
    // control feels dead until it has been moved to that value.
    MC3000.pitchSoftTakeover = Boolean(MC3000.setting("pitchSoftTakeover", false));
    MC3000.softTakeoverEqAndGain = Boolean(MC3000.setting("softTakeoverEqAndGain", false));
    MC3000.softTakeoverVolume = Boolean(MC3000.setting("softTakeoverVolume", false));

    // Keep the FX1 / FX2 routing pointed at the deck its side is showing.
    MC3000.fxFollowsDeck = Boolean(MC3000.setting("fxFollowsDeck", true));
    // Level for every sampler at start-up (0.0 .. 1.0).
    MC3000.samplerVolume = Number(MC3000.setting("samplerVolume", 0.5));

    // Drive the 7-segment channel level meters.
    MC3000.enableVuMeters = Boolean(MC3000.setting("enableVuMeters", true));
    // Blink the SYNC LED when the track is close to its end.
    MC3000.enableEndOfTrackWarning = Boolean(MC3000.setting("enableEndOfTrackWarning", false));
    MC3000.endOfTrackWarningPosition = Number(MC3000.setting("endOfTrackWarningPosition", 0.9));

    // CHANNEL SELECT A/C, B/D (notes 14/15) flip MC3000.deckOnSide between the
    // two decks of their side. DECK CHG fires a ~1 ms pulse on these notes
    // (owner's manual p.22, item 21); only a release after at least this many
    // milliseconds counts as a real press.
    MC3000.channelSelectMinHoldMs = Number(MC3000.setting("channelSelectMinHoldMs", 30));

    // Samplers: hold time of the SAMP. button for loop / sync leader (ms); sync
    // and quantize for every loaded sampler; the internal clock as sync leader at
    // start-up and the BPM range SHIFT + a sampler knob sweeps.
    MC3000.samplerLoopHoldMs = Number(MC3000.setting("samplerLoopHoldMs", 500));
    MC3000.samplerSync = Boolean(MC3000.setting("samplerSync", true));
    MC3000.samplerQuantize = Boolean(MC3000.setting("samplerQuantize", true));
    MC3000.internalClockLeader = Boolean(MC3000.setting("internalClockLeader", true));
    MC3000.sharedBpmMin = Number(MC3000.setting("sharedBpmMin", 60));
    MC3000.sharedBpmMax = Number(MC3000.setting("sharedBpmMax", 180));
};

// Virtual turntable speed used for scratching, scratch filter constants (leave
// alone unless scratching oscillates) and how fast SHIFT + jog scrubs through
// the track (fraction of the track per tick).
MC3000.jogScratchRpm = 33 + 1 / 3;
MC3000.scratchAlpha = 1 / 8;
MC3000.scratchBeta = (1 / 8) / 32;
MC3000.jogSeekSensitivity = 1 / 800;
// +/- this much of the full pitch fader travel counts as "caught up".
MC3000.pitchTakeoverTolerance = 0.04;

// ============================================================================
//  HARDWARE CONSTANTS - these come from the MC3000 manual
//  ("MIDI command list", pages 15-17 of the owner's manual).
// ============================================================================

// ============================================================================
//  DECK ASSIGNMENT
//
//  Maps each physical MC3000 deck (the letters printed on the DECK CHG buttons)
//  to the Mixxx deck it controls. A / C are the LEFT side (channels 0 and 1),
//  B / D the RIGHT side (channels 2 and 3). If a physical deck controls the
//  wrong Mixxx deck, swap the two numbers on that side here and change the
//  <group> entries in the XML file to match.
// ============================================================================
MC3000.deckForA = 1;   // physical DECK A  -> Mixxx [Channel1]
MC3000.deckForC = 3;   // physical DECK C  -> Mixxx [Channel3]
MC3000.deckForB = 2;   // physical DECK B  -> Mixxx [Channel2]
MC3000.deckForD = 4;   // physical DECK D  -> Mixxx [Channel4]

// Everything below is derived from the table above.
// MIDI channel (0-based) -> Mixxx deck
MC3000.deckFromChannel = [MC3000.deckForA, MC3000.deckForC,
    MC3000.deckForB, MC3000.deckForD];

// Mixxx deck number -> MIDI channel (inverse of the table above)
MC3000.channelFromDeck = (function() {
    const m = [];
    m[MC3000.deckForA - 1] = 0;
    m[MC3000.deckForC - 1] = 1;
    m[MC3000.deckForB - 1] = 2;
    m[MC3000.deckForD - 1] = 3;
    return m;
})();

// MIDI channel (0-based) -> physical side (0 = left, 1 = right)
MC3000.sideFromChannel = [0, 0, 1, 1];

// LED numbers, sent as: 0xB<channel>, <74 on | 75 off | 76 blink>, <number>
MC3000.led = {
    vinyl: 6,
    keylock: 8,
    sync: 9,
    cue: 38,
    play: 39,
    loopIn: 36,
    loopOut: 64,
    autoLoop: 43,
    fxOn1: 90,
    fxOn2: 91,
    // CUE1..CUE4 then CUE5..CUE8
    hotcue: [17, 19, 21, 23, 48, 50, 52, 54],
    // EFX.1..EFX.4 buttons, left deck section (FX1) and right section (FX2)
    efx: [[92, 93, 94, 95], [96, 97, 98, 99]],
    // SAMP.1..SAMP.4 buttons, left and right section. The left bank was mapped
    // one address at a time on a real unit (25/26 = pos1 bright/dim, 27/28 =
    // pos2, 29 = pos3 bright, 31 = pos3 dim, 32 = pos4 bright), which differs
    // from the manual's table. The right bank has not been verified that way.
    samp: [[25, 27, 29, 32], [65, 67, 69, 71]]
};

// Channel-CUE (PFL) LEDs use the 80/81 opcodes instead of 74/75. Indexed by deck.
MC3000.pflLed = [69, 81, 75, 87];        // deck 1(A), 2(B), 3(C), 4(D)
// First of the 7 level-meter LEDs, indexed by deck.
MC3000.vuLedBase = [10, 42, 26, 58];     // deck 1(A), 2(B), 3(C), 4(D)

// Note number -> hotcue number
MC3000.hotcueFromNote = {
    23: 1, 24: 2, 25: 3, 32: 4,      // CUE 1-4 (CUE 5-8 mode OFF)
    72: 5, 73: 6, 74: 7, 75: 8       // same buttons with CUE 5-8 mode ON
};

// EFX section, indexed [side][physical position EFX.1 .. EFX.4]. The left
// section (side 0) drives EffectUnit1, the right one (side 1) EffectUnit2.
// The unit comes from the note / CC number, not from the MIDI channel.
MC3000.efxButtonNotes = [[21, 18, 19, 20], [85, 82, 83, 84]];
MC3000.efxKnobCCs = [[85, 86, 87, 88], [89, 90, 91, 92]];

// SAMP.x button note -> sampler number
MC3000.samplerFromNote = {
    33: 1, 34: 2, 35: 3, 36: 4,      // left section
    49: 5, 50: 6, 51: 7, 52: 8       // right section
};
// SAMP.x knob CC -> sampler number (the same physical EFX knobs, while
// SAMP. MODE is on), in the order of the knobs: left section = samplers 1-4,
// right section = samplers 5-8.
MC3000.samplerFromCC = {
    48: 1, 49: 2, 50: 3, 51: 4,
    52: 5, 53: 6, 54: 7, 55: 8
};

// ============================================================================
//  RUNTIME STATE
// ============================================================================

MC3000.shift = [false, false];              // per physical side
MC3000.deckOnSide = [MC3000.deckForA, MC3000.deckForB];  // each side starts on its A/B deck
MC3000.scratchEnabled = [true, true, true, true];
MC3000.scratching = [false, false, false, false];
MC3000.jogLocked = [false, false, false, false];
MC3000.pitchNeedsTakeover = [false, false, false, false];
MC3000.vuLedsLit = [-1, -1, -1, -1];
MC3000.endOfTrack = [false, false, false, false];
MC3000.connections = [];
// ComponentsJS EffectUnit per side, created in MC3000.init()
MC3000.effectUnit = [];
// Timestamp (MC3000.now()) of the last note-14/15 press per side, used to
// measure hold duration - see MC3000.channelSelectMinHoldMs.
MC3000.channelSelectPressTime = [0, 0];

// ============================================================================
//  SMALL HELPERS
// ============================================================================

MC3000.sideOf = function(channel) {
    return MC3000.sideFromChannel[channel & 0x03];
};

MC3000.deckFromGroup = function(group) {
    const m = group.match(/^\[Channel(\d+)\]$/);
    return m === null ? -1 : parseInt(m[1], 10);
};

MC3000.now = function() {
    return (new Date()).getTime();
};

// Resolve which Mixxx deck a deck-section message (play, cue, loops, jog, EQ,
// pitch, ...) belongs to, purely from the MIDI channel it arrived on.
MC3000.deckGroup = function(channel) {
    return `[Channel${MC3000.deckFromChannel[channel & 0x03]}]`;
};

// true when the SHIFT button of this side is held
MC3000.isShifted = function(channel) {
    return MC3000.shift[MC3000.sideOf(channel)];
};

// true when either SHIFT is held (used for the centre / browser section)
MC3000.anyShift = function() {
    return MC3000.shift[0] || MC3000.shift[1];
};

// Note-on with a non-zero velocity means "pressed"
MC3000.isPress = function(value, status) {
    return ((status & 0xF0) === 0x90) && value > 0;
};

MC3000.toggle = function(group, key) {
    engine.setValue(group, key, engine.getValue(group, key) ? 0 : 1);
};

// ---- version compatibility -------------------------------------------------
// Several controls this mapping would like to use only exist from Mixxx 2.4
// onwards. Probe for them once at start-up and fall back to the older names, so
// the same file works on Mixxx 2.1 through 2.5+.
MC3000.compat = {};

MC3000.controlExists = function(group, key) {
    try {
        const probe = engine.makeConnection(group, key, function() {  });
        if (probe) {
            probe.disconnect();
            return true;
        }
    } catch (e) {
        console.log(`Denon MC3000: no control ${group},${key} (${e})`);
        return false;
    }
    return false;
};

// Return the first (group, key) pair that this Mixxx build actually has.
MC3000.pickControl = function(candidates) {
    for (let i = 0; i < candidates.length; i++) {
        if (MC3000.controlExists(candidates[i][0], candidates[i][1])) {
            return candidates[i];
        }
    }
    return candidates[candidates.length - 1];
};

MC3000.toggleCompat = function(name) {
    const target = MC3000.compat[name];
    if (target) {
        MC3000.toggle(target[0], target[1]);
    }
};

MC3000.detectCompat = function() {
    // hotcue_X_status is 2.4+, hotcue_X_enabled goes back to 1.8
    MC3000.compat.hotcueSuffix =
        MC3000.controlExists("[Channel1]", "hotcue_1_status") ? "status" : "enabled";
    // sync_leader is 2.4+, previously called sync_master
    MC3000.compat.syncLeader =
        MC3000.controlExists("[Channel1]", "sync_leader") ? "sync_leader" : "sync_master";
    // vu_meter is 2.4+, previously called VuMeter
    MC3000.compat.vuMeter =
        MC3000.controlExists("[Channel1]", "vu_meter") ? "vu_meter" : "VuMeter";
    // loop_remove is 2.4+
    MC3000.compat.loopRemove = MC3000.controlExists("[Channel1]", "loop_remove");
    // CloneFromDeck is 2.3+
    MC3000.compat.clone = MC3000.controlExists("[Channel1]", "CloneFromDeck");
    // the whole [Skin] group is 2.4+
    MC3000.compat.maximizeLibrary = MC3000.pickControl([
        ["[Skin]", "show_maximized_library"],
        ["[Master]", "maximize_library"]
    ]);
    MC3000.compat.showSamplers = MC3000.pickControl([
        ["[Skin]", "show_samplers"],
        ["[Samplers]", "show_samplers"]
    ]);
    MC3000.compat.showEffects = MC3000.pickControl([
        ["[Skin]", "show_effectrack"],
        ["[EffectRack1]", "show"]
    ]);
    MC3000.compat.showCoverArt = MC3000.pickControl([
        ["[Skin]", "show_library_coverart"],
        ["[Library]", "show_coverart"]
    ]);

    const c = MC3000.compat;
    console.log(`Denon MC3000: detected Mixxx profile -> hotcue_X_${c.hotcueSuffix}, ${c.syncLeader}, ${c.vuMeter}, ${c.maximizeLibrary[0]}`);
};

// ---- LED output ------------------------------------------------------------
// state: 0/false = off, 1/true = on, 2 = blink
// Low-level: light an LED on one specific MIDI channel.
MC3000.sendLed = function(channel, ledNumber, state) {
    let opcode = 75;                     // off
    if (state === 2) {
        opcode = 76;                     // blink
    } else if (state) {
        opcode = 74;                     // on
    }
    midi.sendShortMsg(0xB0 + channel, opcode, ledNumber);
};

// Which MIDI channel to light a deck's LEDs on: the deck's own fixed channel.
// The hardware listens on the channel of whichever deck it currently shows, so
// it only "sees" the LEDs of the deck it is displaying.
MC3000.ledChannel = function(deck) {
    return MC3000.channelFromDeck[deck - 1];
};

MC3000.setLed = function(deck, ledNumber, state) {
    if (deck < 1 || deck > 4) {
        return;
    }
    const channel = MC3000.ledChannel(deck);
    if (channel === -1) {
        return;
    }
    MC3000.sendLed(channel, ledNumber, state);
};

// Channel-CUE LEDs and the level meters use a different pair of opcodes
MC3000.setLed2 = function(deck, ledNumber, on) {
    if (deck < 1 || deck > 4) {
        return;
    }
    midi.sendShortMsg(0xB0 + MC3000.channelFromDeck[deck - 1], on ? 80 : 81, ledNumber);
};

// Index of the EFX/SAMP LED bank for a deck: 0 = left section, 1 = right
MC3000.sectionOfDeck = function(deck) {
    return (deck === 1 || deck === 3) ? 0 : 1;
};

// ============================================================================
//  INIT / SHUTDOWN
// ============================================================================

MC3000.init = function(id) {
    MC3000.id = id;
    MC3000.loadSettings();

    for (let i = 0; i < 4; i++) {
        MC3000.scratchEnabled[i] = MC3000.vinylModeAtStartup;
    }

    MC3000.detectCompat();
    MC3000.allLedsOff();
    MC3000.initEffectUnits();

    for (let deck = 1; deck <= 4; deck++) {
        const group = `[Channel${deck}]`;

        // ---- soft takeover on the shared mixer controls --------------------
        if (MC3000.softTakeoverEqAndGain) {
            engine.softTakeover(group, "pregain", true);
            engine.softTakeover(`[EqualizerRack1_${group}_Effect1]`, "parameter1", true);
            engine.softTakeover(`[EqualizerRack1_${group}_Effect1]`, "parameter2", true);
            engine.softTakeover(`[EqualizerRack1_${group}_Effect1]`, "parameter3", true);
            engine.softTakeover(`[QuickEffectRack1_${group}]`, "super1", true);
        }
        if (MC3000.softTakeoverVolume) {
            engine.softTakeover(group, "volume", true);
        }

        // ---- LED feedback --------------------------------------------------
        MC3000.connect(group, "play_indicator", MC3000.onPlay);
        MC3000.connect(group, "cue_indicator", MC3000.onCue);
        if (MC3000.keylockAtStartup !== "keep") {
            engine.setValue(group, "keylock", MC3000.keylockAtStartup === "on" ? 1 : 0);
        }
        MC3000.connect(group, "keylock", MC3000.onKeylock);
        MC3000.connect(group, "sync_enabled", MC3000.onSync);
        MC3000.connect(group, "pfl", MC3000.onPfl);
        MC3000.connect(group, "loop_enabled", MC3000.onLoopEnabled);
        MC3000.connect(group, "loop_start_position", MC3000.onLoopStart);
        MC3000.connect(group, "loop_end_position", MC3000.onLoopEnd);

        for (let i = 1; i <= 8; i++) {
            MC3000.connect(group, `hotcue_${i}_${MC3000.compat.hotcueSuffix}`,
                MC3000.onHotcue);
        }

        MC3000.connect("[EffectRack1_EffectUnit1]", `group_${group}_enable`,
            MC3000.onFxAssign);
        MC3000.connect("[EffectRack1_EffectUnit2]", `group_${group}_enable`,
            MC3000.onFxAssign);

        if (MC3000.enableVuMeters) {
            MC3000.connect(group, MC3000.compat.vuMeter, MC3000.onVuMeter);
        }
        if (MC3000.enableEndOfTrackWarning) {
            MC3000.connect(group, "playposition", MC3000.onPlayPosition);
        }

        // VINYL MODE LED is script state, not a Mixxx control
        MC3000.setLed(deck, MC3000.led.vinyl, MC3000.scratchEnabled[deck - 1]);
    }

    MC3000.leaderKey = MC3000.controlExists("[InternalClock]", "sync_leader")
        ? "sync_leader" : "sync_master";
    if (MC3000.internalClockLeader) {
        engine.setValue("[InternalClock]", MC3000.leaderKey, 1);
    }

    // Sampler play state -> the SAMP.1..4 button LEDs of each side
    for (let i = 1; i <= 8; i++) {
        MC3000.connect(`[Sampler${i}]`, "play", MC3000.onSampler);
        MC3000.connect(`[Sampler${i}]`, "repeat", MC3000.onSampler);
        MC3000.connect(`[Sampler${i}]`, "track_loaded", MC3000.onSamplerLoaded);
    }

    // Ask the controller to re-transmit the physical position of every fader,
    // knob and slider so Mixxx starts in sync with the hardware.
    // ("ALL SLIDER/VOLUME/FADER REQUEST", owner's manual p.17)
    engine.beginTimer(1500, function() {
        for (let ch = 0; ch < 4; ch++) {
            midi.sendShortMsg(0xB0 + ch, 74, 57);
        }
    }, true);

    for (let i = 1; i <= 8; i++) {
        engine.setValue(`[Sampler${i}]`, "volume", MC3000.samplerVolume);
    }

    console.log(`Denon MC3000: mapping initialised (${id})`);
};

MC3000.shutdown = function() {
    for (const unit of MC3000.effectUnit) {
        unit.forEachComponent(function(component) {
            component.disconnect();
        });
    }
    for (const connection of MC3000.connections) {
        connection.disconnect();
    }
    MC3000.connections = [];
    MC3000.allLedsOff();
};

MC3000.connect = function(group, key, callback) {
    const connection = engine.makeConnection(group, key, callback);
    if (connection) {
        MC3000.connections.push(connection);
        connection.trigger();
    } else {
        console.warn(`Denon MC3000: no such control: ${group},${key}`);
    }
    return connection;
};

MC3000.allLedsOff = function() {
    let deck, ch, i, section;
    // Address the four channels directly rather than going through setLed:
    // at start-up two of the decks are not on a side, so setLed would skip them
    // and leave their LEDs lit from the previous session.
    for (ch = 0; ch <= 3; ch++) {
        section = MC3000.sideFromChannel[ch];
        MC3000.sendLed(ch, MC3000.led.vinyl, 0);
        MC3000.sendLed(ch, MC3000.led.keylock, 0);
        MC3000.sendLed(ch, MC3000.led.sync, 0);
        MC3000.sendLed(ch, MC3000.led.cue, 0);
        MC3000.sendLed(ch, MC3000.led.play, 0);
        MC3000.sendLed(ch, MC3000.led.loopIn, 0);
        MC3000.sendLed(ch, MC3000.led.loopOut, 0);
        MC3000.sendLed(ch, MC3000.led.autoLoop, 0);
        MC3000.sendLed(ch, MC3000.led.fxOn1, 0);
        MC3000.sendLed(ch, MC3000.led.fxOn2, 0);
        for (i = 0; i < 8; i++) {
            MC3000.sendLed(ch, MC3000.led.hotcue[i], 0);
        }
        for (i = 0; i < 4; i++) {
            MC3000.sendLed(ch, MC3000.led.efx[section][i], 0);
            MC3000.sendLed(ch, MC3000.led.samp[section][i], 0);
        }
    }
    for (deck = 1; deck <= 4; deck++) {
        MC3000.setLed2(deck, MC3000.pflLed[deck - 1], false);
        for (i = 0; i < 7; i++) {
            MC3000.setLed2(deck, MC3000.vuLedBase[deck - 1] + i, false);
        }
        MC3000.vuLedsLit[deck - 1] = -1;
    }
};

// ============================================================================
//  SHIFT AND DECK SELECTION
// ============================================================================

// SHIFT (LEFT DECK) = note 96, SHIFT (RIGHT DECK) = note 97.
// The side is taken from the NOTE, the deck from the CHANNEL, so the state can
// never get out of sync when you change deck layer while holding SHIFT. SHIFT
// does not touch the deck focus - only the DECK CHG / CHANNEL SELECT buttons do.
MC3000.shiftButton = function(channel, control, value, status) {
    const side = (control === 96) ? 0 : 1;
    const shifted = MC3000.isPress(value, status);
    MC3000.shift[side] = shifted;
    // the effect unit of this side switches to its shifted layer
    if (MC3000.effectUnit[side] !== undefined) {
        if (shifted) {
            MC3000.effectUnit[side].shift();
        } else {
            MC3000.effectUnit[side].unshift();
        }
    }
};

// DECK CHG A/B/C/D (notes 3/8/9/10): select that deck outright. Dispatched by
// a switch on the note, which names the deck unambiguously (note 9 is always
// DECK CHG C) regardless of which channel the press arrives on.
//
// The hardware itself moves the deck section to the selected deck's MIDI
// channel, so every subsequent play/cue/loop/pitch/jog message already arrives
// on the right channel. This handler updates MC3000.deckOnSide, used by the
// controls that cannot get their deck from an incoming channel (LOAD,
// FX-follows-deck, the EFX LEDs and the LEDs of the deck itself).
//
// Notes 14/15 (DECK A/C, DECK B/D) are Denon's "CHANNEL SELECT A/C" /
// "CHANNEL SELECT B/D" (owner's manual p.22, item 21): which deck feeds the
// physical LEVEL/EQ/fader knobs, with a hardware-autonomous lamp. Every deck
// has its own CCs in this mapping, so instead a genuine press flips
// MC3000.deckOnSide for that side between its two decks, as the lamp shows.
// This does not move the jog wheel/play/cue section (the hardware does not
// allow that from this button), but LOAD A/C, LOAD B/D read deckOnSide, so
// "press CHANNEL SELECT, then LOAD" loads into the deck the lamp shows.
//
// DECK CHG also pulses the signal of this button for ~1 ms with no finger on
// it. Flipping deckOnSide on every DECK CHG would undo the choice DECK CHG
// just made, so only a release after at least MC3000.channelSelectMinHoldMs
// counts as a press. That needs both press AND release (unlike DECK CHG
// A/B/C/D, which only act on press), so it is handled first.
MC3000.deckButton = function(channel, control, value, status) {
    if (control === 14 || control === 15) {
        const csSide = (control === 14) ? 0 : 1;
        const csPressed = MC3000.isPress(value, status);
        const csReleased = !csPressed && (((status & 0xF0) === 0x80) ||
            ((status & 0xF0) === 0x90 && value === 0));
        if (csPressed) {
            MC3000.channelSelectPressTime[csSide] = MC3000.now();
        } else if (csReleased &&
                MC3000.now() - MC3000.channelSelectPressTime[csSide] >=
                MC3000.channelSelectMinHoldMs) {
            if (csSide === 0) {
                MC3000.deckOnSide[0] = (MC3000.deckOnSide[0] === MC3000.deckForA)
                    ? MC3000.deckForC : MC3000.deckForA;
            } else {
                MC3000.deckOnSide[1] = (MC3000.deckOnSide[1] === MC3000.deckForB)
                    ? MC3000.deckForD : MC3000.deckForB;
            }
        }
        return;
    }

    if (!MC3000.isPress(value, status)) {
        return;
    }
    let side;
    let deck;

    switch (control) {
    case 3:   side = 0; deck = MC3000.deckForA; break;   // DECK CHG A
    case 9:   side = 0; deck = MC3000.deckForC; break;   // DECK CHG C
    case 8:   side = 1; deck = MC3000.deckForB; break;   // DECK CHG B
    case 10:  side = 1; deck = MC3000.deckForD; break;   // DECK CHG D
    default:  return;
    }

    MC3000.applyDeck(side, deck);
};

MC3000.applyDeck = function(side, deck) {
    if (deck === MC3000.deckOnSide[side]) {
        MC3000.refreshDeckLeds(deck);         // already there: just resync
        return;
    }
    MC3000.selectDeck(side, deck);
};

// Remember which deck a physical side is showing and move everything that
// follows the deck: the FX unit, pitch fader takeover and all the LEDs.
MC3000.selectDeck = function(side, deck) {
    if (MC3000.deckOnSide[side] === deck) {
        return false;                    // nothing changed, keep the MIDI bus quiet
    }
    const previous = MC3000.deckOnSide[side];
    MC3000.deckOnSide[side] = deck;

    // Move this side's effect unit onto the deck that is now showing, so FX1
    // keeps acting on the left deck and FX2 on the right one.
    if (MC3000.fxFollowsDeck) {
        const unit = `[EffectRack1_EffectUnit${side + 1}]`;
        if (engine.getValue(unit, `group_[Channel${previous}]_enable`)) {
            engine.setValue(unit, `group_[Channel${previous}]_enable`, 0);
            engine.setValue(unit, `group_[Channel${deck}]_enable`, 1);
        }
    }

    // the shared pitch fader now belongs to another deck
    MC3000.pitchNeedsTakeover[deck - 1] = MC3000.pitchSoftTakeover;

    // The deck leaving this side is no longer visible on any channel, so blank
    // its per-deck LEDs (on that deck's own channel) before painting the
    // incoming deck. Otherwise its lamps stay lit on the channel it left.
    MC3000.clearDeckLeds(previous, MC3000.channelFromDeck[previous - 1]);
    MC3000.refreshDeckLeds(deck);
    return true;
};

// Turn off every per-deck LED for a deck that is leaving the panel. It is no
// longer on any side, so setLed could not route it - send straight to the
// channel it was using.
MC3000.clearDeckLeds = function(deck, channel) {
    const section = MC3000.sectionOfDeck(deck);
    const lamps = [MC3000.led.play, MC3000.led.cue, MC3000.led.keylock,
        MC3000.led.sync, MC3000.led.loopIn, MC3000.led.loopOut,
        MC3000.led.autoLoop, MC3000.led.vinyl];
    for (const lamp of lamps) {
        MC3000.sendLed(channel, lamp, false);
    }
    for (let i = 0; i < 8; i++) {
        MC3000.sendLed(channel, MC3000.led.hotcue[i], false);
    }
    for (let i = 0; i < 4; i++) {
        MC3000.sendLed(channel, MC3000.led.efx[section][i], false);
        MC3000.sendLed(channel, MC3000.led.samp[section][i], false);
    }
};

MC3000.refreshDeckLeds = function(deck) {
    MC3000.setLed(deck, MC3000.led.vinyl, MC3000.scratchEnabled[deck - 1]);
    const group = `[Channel${deck}]`;
    MC3000.onPlay(engine.getValue(group, "play_indicator"), group);
    MC3000.onCue(engine.getValue(group, "cue_indicator"), group);
    MC3000.onKeylock(engine.getValue(group, "keylock"), group);
    MC3000.onSync(engine.getValue(group, "sync_enabled"), group);
    MC3000.onLoopEnabled(engine.getValue(group, "loop_enabled"), group);
    // the loop-in and loop-out markers belong to the deck too, so they have to
    // be re-read on a layer change or they keep showing the previous deck
    MC3000.onLoopStart(engine.getValue(group, "loop_start_position"), group);
    MC3000.onLoopEnd(engine.getValue(group, "loop_end_position"), group);
    for (let i = 1; i <= 8; i++) {
        MC3000.onHotcue(
            engine.getValue(group, `hotcue_${i}_${MC3000.compat.hotcueSuffix}`),
            group, `hotcue_${i}_${MC3000.compat.hotcueSuffix}`);
    }
    const section = MC3000.sectionOfDeck(deck);
    // the EFX button LEDs follow the deck onto its new channel
    const unit = MC3000.effectUnit[section];
    if (unit !== undefined) {
        unit.enableButtons.forEachComponent(function(button) {
            button.trigger();
        });
        unit.effectFocusButton.trigger();
    }
    // sampler LEDs of this side follow the deck onto its new channel
    const first = (section === 0) ? 1 : 5;
    for (let i = 0; i < 4; i++) {
        MC3000.setLed(deck, MC3000.led.samp[section][i],
            MC3000.samplerLedState(first + i));
    }
};

// ============================================================================
//  TRANSPORT
// ============================================================================

// PLAY / PAUSE, note 67.  SHIFT = reverse roll (censor) while held.
MC3000.play = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    const pressed = ((status & 0xF0) === 0x90) && value > 0;
    if (MC3000.isShifted(channel)) {
        engine.setValue(group, "reverseroll", pressed ? 1 : 0);
        return;
    }
    if (pressed) {
        MC3000.toggle(group, "play");
    }
};

// CUE, note 66.  SHIFT = jump to the beginning of the track.
MC3000.cue = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    const pressed = ((status & 0xF0) === 0x90) && value > 0;
    if (MC3000.isShifted(channel)) {
        if (pressed) {
            engine.setValue(group, "playposition", 0);
        }
        return;
    }
    engine.setValue(group, "cue_default", pressed ? 1 : 0);
};

// SYNC, note 107.
// Mapped to sync_enabled (sync lock), NOT beatsync. beatsync is a one-shot
// tempo match that can never be undone from the controller, which is why the
// old behaviour felt like "everything stays linked forever". sync_enabled is a
// latching control that Mixxx itself interprets as: short press = toggle sync
// lock on/off, press and hold for a second = momentary sync. So the same button
// now both engages AND releases the sync.
// SHIFT = make this deck the sync leader.
MC3000.sync = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    const pressed = ((status & 0xF0) === 0x90) && value > 0;

    if (MC3000.isShifted(channel)) {
        if (pressed) {
            MC3000.toggle(group, MC3000.compat.syncLeader);
        }
        return;
    }
    engine.setValue(group, "sync_enabled", pressed ? 1 : 0);
};

// KEY LOCK, note 6.  SHIFT = tap the BPM.
MC3000.keylock = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    if (!MC3000.isPress(value, status)) {
        return;
    }
    if (MC3000.isShifted(channel)) {
        engine.setValue(group, "bpm_tap", 1);
    } else {
        MC3000.toggle(group, "keylock");
    }
};

// PITCH BEND +/-, notes 12 and 13.
// Unshifted: temporary tempo nudge. SHIFT: fast forward / rewind.
MC3000.bend = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    const pressed = ((status & 0xF0) === 0x90) && value > 0;
    const up = (control === 12);
    if (MC3000.isShifted(channel)) {
        engine.setValue(group, up ? "fwd" : "back", pressed ? 1 : 0);
    } else {
        engine.setValue(group, up ? "rate_temp_up" : "rate_temp_down", pressed ? 1 : 0);
    }
};

// ============================================================================
//  PITCH FADER (0xEn, 14 bit) - mapped in the XML WITHOUT a <midino> element
// ============================================================================

MC3000.pitch = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    const deck = MC3000.deckFromGroup(group);
    // 14-bit little endian: control = LSB, value = MSB.
    // 8192 is the exact centre of the 14-bit range, so dividing the offset by
    // 8192 makes the fader detent land on precisely 0.00 %.
    const raw = (value << 7) | control;
    let rate = (raw - 8192) / 8192.0;
    if (MC3000.invertPitchFader) {
        rate = -rate;
    }
    rate = Math.max(-1.0, Math.min(1.0, rate));

    if (MC3000.pitchNeedsTakeover[deck - 1]) {
        const current = engine.getValue(group, "rate");
        if (Math.abs(rate - current) > MC3000.pitchTakeoverTolerance) {
            return;                     // ignore until the fader catches up
        }
        MC3000.pitchNeedsTakeover[deck - 1] = false;
    }
    engine.setValue(group, "rate", rate);
};

// ============================================================================
//  JOG WHEEL
// ============================================================================

// Jog wheel touch sensor, note 81.
MC3000.jogTouch = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    const deck = MC3000.deckFromGroup(group);
    const touched = ((status & 0xF0) === 0x90) && value > 0;

    if (MC3000.jogLocked[deck - 1] || !MC3000.scratchEnabled[deck - 1]) {
        return;
    }
    if (touched) {
        engine.scratchEnable(deck,
            MC3000.jogTicksPerRevolution,
            MC3000.jogScratchRpm,
            MC3000.scratchAlpha,
            MC3000.scratchBeta);
        MC3000.scratching[deck - 1] = true;
    } else {
        engine.scratchDisable(deck);
        MC3000.scratching[deck - 1] = false;
    }
};

// Jog wheel rotation, CC 81. 1..63 = reverse, 65..127 = forward.
MC3000.jogWheel = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    const deck = MC3000.deckFromGroup(group);
    if (MC3000.jogLocked[deck - 1]) {
        return;
    }
    const delta = value - 64;
    if (delta === 0) {
        return;
    }

    // SHIFT + jog = scrub quickly through the loaded track
    if (MC3000.isShifted(channel)) {
        let position = engine.getValue(group, "playposition");
        position += delta * MC3000.jogSeekSensitivity;
        engine.setValue(group, "playposition", Math.max(0, Math.min(1, position)));
        return;
    }

    if (MC3000.scratching[deck - 1]) {
        engine.scratchTick(deck, delta);
    } else {
        engine.setValue(group, "jog", delta * MC3000.jogBendSensitivity);
    }
};

// VINYL MODE, note 4. SHIFT = lock the jog wheel completely.
MC3000.vinylMode = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    if (!MC3000.isPress(value, status)) {
        return;
    }
    const deck = MC3000.deckFromGroup(group);

    if (MC3000.isShifted(channel)) {
        MC3000.jogLocked[deck - 1] = !MC3000.jogLocked[deck - 1];
        if (MC3000.jogLocked[deck - 1] && MC3000.scratching[deck - 1]) {
            engine.scratchDisable(deck);
            MC3000.scratching[deck - 1] = false;
        }
        // blink while the wheel is locked
        MC3000.setLed(deck, MC3000.led.vinyl,
            MC3000.jogLocked[deck - 1] ? 2 : MC3000.scratchEnabled[deck - 1]);
        return;
    }

    MC3000.scratchEnabled[deck - 1] = !MC3000.scratchEnabled[deck - 1];
    if (!MC3000.scratchEnabled[deck - 1] && MC3000.scratching[deck - 1]) {
        engine.scratchDisable(deck);
        MC3000.scratching[deck - 1] = false;
    }
    MC3000.setLed(deck, MC3000.led.vinyl, MC3000.scratchEnabled[deck - 1]);
};

// ============================================================================
//  HOT CUES
// ============================================================================

// CUE 1-4 (notes 23,24,25,32) and CUE 5-8 (notes 72,73,74,75).
// The CUE 5-8 mode button is handled by the hardware: it simply makes the same
// four buttons send the other set of notes.
// SHIFT = delete the hot cue.
MC3000.hotcue = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    const number = MC3000.hotcueFromNote[control];
    if (number === undefined) {
        return;
    }
    const pressed = ((status & 0xF0) === 0x90) && value > 0;

    if (MC3000.isShifted(channel)) {
        if (pressed) {
            engine.setValue(group, `hotcue_${number}_clear`, 1);
        }
        return;
    }
    engine.setValue(group, `hotcue_${number}_activate`, pressed ? 1 : 0);
};

// ============================================================================
//  LOOPS
// ============================================================================

// LOOP IN, note 55.  SHIFT = clear the loop.
// Drop the loop-in marker. loop_in is a push control: Mixxx starts "loop in
// adjust" mode while it is held, so the release MUST be sent as well. Only
// sending the press was what made these buttons behave strangely - the deck was
// left in adjust mode and the jog wheel kept dragging the marker around.
// SHIFT clears the loop instead.
MC3000.loopIn = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    const pressed = ((status & 0xF0) === 0x90) && value > 0;

    if (MC3000.isShifted(channel)) {
        if (pressed) {
            MC3000.clearLoop(group, "in");
        }
        return;
    }
    engine.setValue(group, "loop_in", pressed ? 1 : 0);
};

// Take the loop out of the deck: loop_remove where it exists, otherwise
// writing -1 to the marker position detaches it.
MC3000.clearLoop = function(group, which) {
    if (engine.getValue(group, "loop_enabled") > 0) {
        engine.setValue(group, "reloop_toggle", 1);
        engine.setValue(group, "reloop_toggle", 0);
    }
    if (MC3000.compat.loopRemove) {
        engine.setValue(group, "loop_remove", 1);
        engine.setValue(group, "loop_remove", 0);
    } else {
        engine.setValue(group,
            (which === "in") ? "loop_start_position" : "loop_end_position", -1);
    }
};

// LOOP OUT, note 57.  SHIFT = clear the loop.
MC3000.loopOut = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    const pressed = ((status & 0xF0) === 0x90) && value > 0;

    if (MC3000.isShifted(channel)) {
        if (pressed) {
            MC3000.clearLoop(group, "out");
        }
        return;
    }
    engine.setValue(group, "loop_out", pressed ? 1 : 0);
};

// AUTO LOOP, note 29.
// Unshifted: set / clear a loop of the current beatloop size.
// SHIFT: re-enter the last loop (reloop).
MC3000.autoLoop = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    if (!MC3000.isPress(value, status)) {
        return;
    }
    if (MC3000.isShifted(channel)) {
        engine.setValue(group, "reloop_toggle", 1);
        return;
    }
    if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "reloop_toggle", 1);
    } else {
        engine.setValue(group, "beatloop_activate", 1);
    }
};

// AUTO LOOP - / + , notes 105 and 106.
// Unshifted: halve / double the loop length (resizes an active loop too).
// SHIFT: move the whole loop one beat backward / forward.
MC3000.loopSize = function(channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    if (!MC3000.isPress(value, status)) {
        return;
    }
    const bigger = (control === 106);

    if (MC3000.isShifted(channel)) {
        engine.setValue(group, bigger ? "loop_move_1_forward" : "loop_move_1_backward", 1);
        return;
    }
    let size = engine.getValue(group, "beatloop_size");
    size = bigger ? size * 2 : size / 2;
    size = Math.max(1 / 32, Math.min(64, size));
    engine.setValue(group, "beatloop_size", size);
};

// ============================================================================
//  EFX SECTION
//  Standard effects mapping on the ComponentsJS EffectUnit; the left section
//  drives EffectUnit1 and the right section EffectUnit2:
//    EFX.1 - EFX.3 buttons : toggle effect 1 - 3 (hold = momentary)
//    EFX.4 button          : effect focus (SHIFT = cycle the unit, a no-op
//                            as every section has just one)
//    EFX.1 - EFX.3 knobs   : metaknob of effect 1 - 3 (parameters of the
//                            focused effect) - SHIFT = choose another effect
//    EFX.4 knob            : dry / wet of the unit
//  SHIFT is passed on by MC3000.shiftButton(). The XML routes the notes / CCs
//  of MC3000.efxButtonNotes / MC3000.efxKnobCCs straight to the components
//  (e.g. MC3000.effectUnit[0].knobs[1].input).
//  FX ON 1 / 2 (below) route a deck into EffectUnit 1 / 2.
// ============================================================================

MC3000.initEffectUnit = function(side) {
    const unit = new components.EffectUnit([side + 1]);
    const buttons = [
        unit.enableButtons[1],
        unit.enableButtons[2],
        unit.enableButtons[3],
        unit.effectFocusButton
    ];
    buttons.forEach(function(button, index) {
        // The components treat any non-zero value as a press, but the
        // note-off bindings in the XML carry a release velocity.
        button.isPress = function(_channel, _control, value, status) {
            return MC3000.isPress(value, status);
        };
        // The EFX LEDs use the MC3000's own protocol on the channel of
        // the deck the side is showing.
        button.send = function(value) {
            MC3000.setLed(MC3000.deckOnSide[side], MC3000.led.efx[side][index], value > 0);
        };
    });
    unit.init();
    MC3000.effectUnit[side] = unit;
};

MC3000.initEffectUnits = function() {
    MC3000.initEffectUnit(0);
    MC3000.initEffectUnit(1);
};

// FX ON 1 (note 86) / FX ON 2 (note 87): assign this deck to EffectUnit 1 / 2.
// SHIFT + FX ON 1 = quantize, SHIFT + FX ON 2 = move the beatgrid to the
// current play position.
MC3000.fxOn = function(channel, control, value, status, group) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    // The unit comes from the button (note 86 = FX1, note 87 = FX2) and the
    // deck is simply the one the message arrived on - which is what "group"
    // already holds. Do not second-guess either of them.
    const unitNo = (control === 86) ? 1 : 2;
    const target = group;

    if (MC3000.isShifted(channel)) {
        if (unitNo === 1) {
            MC3000.toggle(target, "quantize");
        } else {
            engine.setValue(target, "beats_translate_curpos", 1);
        }
        return;
    }
    MC3000.toggle(`[EffectRack1_EffectUnit${unitNo}]`, `group_${target}_enable`);
};

// ============================================================================
//  SAMPLERS (EFX buttons / knobs while SAMP. MODE is on)
// ============================================================================

// Start a sampler: play from its cue point, or load the selected track if the
// slot is still empty.
MC3000.playSampler = function(number) {
    if (number < 1 || number > 8) {
        return;
    }
    const sampler = `[Sampler${number}]`;
    if (engine.getValue(sampler, "track_loaded")) {
        engine.setValue(sampler, "cue_gotoandplay", 1);
    } else {
        engine.setValue(sampler, "LoadSelectedTrack", 1);
    }
};

// ---- SAMPLER EXTRAS: hold a SAMP. button to loop ------------------------------
// A long press (MC3000.samplerLoopHoldMs) toggles [SamplerN],repeat. The press
// itself still plays the sampler. The LED blinks while repeat is on.
MC3000.samplerHoldTimer = [0, 0, 0, 0, 0, 0, 0, 0, 0];   // by sampler number

// Every sampler follows the sync leader and starts quantized. The internal clock
// becomes the leader when the mapping loads (a deck can take over later, e.g.
// with SHIFT + SYNC); SHIFT + a sampler knob (SAMP. MODE) then sets its BPM,
// i.e. the tempo all synced samplers (and decks) follow. The options are in
// MC3000.loadSettings().

MC3000.onSamplerLoaded = function(value, group) {
    if (value > 0) {
        if (MC3000.samplerSync) {
            engine.setValue(group, "sync_enabled", 1);
        }
        if (MC3000.samplerQuantize) {
            engine.setValue(group, "quantize", 1);
        }
    }
};

// SHIFT + SAMP. button: a short press stops the sampler (and ends its loop) or
// ejects it, done on release; a long press makes the sampler the sync leader
// (again: back to the internal clock, which SHIFT + knob sets the BPM of).
MC3000.samplerShiftPending = [false, false, false, false, false, false, false, false, false];

MC3000.samplerStopOrEject = function(number) {
    const sampler = `[Sampler${number}]`;
    const looping = engine.getValue(sampler, "repeat");
    if (looping) {
        engine.setValue(sampler, "repeat", 0);
    }
    if (engine.getValue(sampler, "play")) {
        engine.setValue(sampler, "play", 0);
    } else if (!looping) {
        engine.setValue(sampler, "eject", 1);
        engine.setValue(sampler, "eject", 0);
    }
};

MC3000.toggleSamplerLeader = function(number) {
    const group = `[Sampler${number}]`;
    if (engine.getValue(group, MC3000.leaderKey)) {
        engine.setValue(group, MC3000.leaderKey, 0);
        if (MC3000.internalClockLeader) {
            engine.setValue("[InternalClock]", MC3000.leaderKey, 1);
        }
    } else {
        engine.setValue(group, "sync_enabled", 1);
        engine.setValue(group, MC3000.leaderKey, 1);
    }
};

MC3000.samplerLedState = function(number) {
    const sampler = `[Sampler${number}]`;
    return engine.getValue(sampler, "repeat") ? 2 : engine.getValue(sampler, "play");
};

MC3000.samplerButton = function(channel, control, value, status) {
    const number = MC3000.samplerFromNote[control];
    if (number === undefined) {
        return;
    }
    if (!MC3000.isPress(value, status)) {
        // released before the hold time: it was a normal press
        if (MC3000.samplerHoldTimer[number] !== 0) {
            engine.stopTimer(MC3000.samplerHoldTimer[number]);
            MC3000.samplerHoldTimer[number] = 0;
        }
        if (MC3000.samplerShiftPending[number]) {
            MC3000.samplerShiftPending[number] = false;
            MC3000.samplerStopOrEject(number);
        }
        return;
    }
    const sampler = `[Sampler${number}]`;

    if (MC3000.isShifted(channel)) {
        // SHIFT: short press stops / ejects (on release), long press = sync leader
        MC3000.samplerShiftPending[number] = true;
        if (MC3000.samplerHoldTimer[number] !== 0) {
            engine.stopTimer(MC3000.samplerHoldTimer[number]);
        }
        MC3000.samplerHoldTimer[number] = engine.beginTimer(MC3000.samplerLoopHoldMs, function() {
            MC3000.samplerHoldTimer[number] = 0;
            MC3000.samplerShiftPending[number] = false;
            MC3000.toggleSamplerLeader(number);
        }, true);
        return;
    }
    MC3000.playSampler(number);
    if (MC3000.samplerHoldTimer[number] !== 0) {
        engine.stopTimer(MC3000.samplerHoldTimer[number]);
    }
    MC3000.samplerHoldTimer[number] = engine.beginTimer(MC3000.samplerLoopHoldMs, function() {
        MC3000.samplerHoldTimer[number] = 0;
        MC3000.toggle(`[Sampler${number}]`, "repeat");
    }, true);
};

MC3000.samplerGain = function(channel, control, value) {
    const number = MC3000.samplerFromCC[control];
    if (number === undefined) {
        return;
    }
    if (MC3000.isShifted(channel)) {
        // SHIFT + knob: shared BPM of the synced samplers
        const bpm = MC3000.sharedBpmMin +
            (value / 127.0) * (MC3000.sharedBpmMax - MC3000.sharedBpmMin);
        engine.setValue("[InternalClock]", "bpm", Math.round(bpm * 10) / 10);
        return;
    }
    engine.setValue(`[Sampler${number}]`, "pregain", (value / 127.0) * 2.0);
};

// ============================================================================
//  MIXER
// ============================================================================

// Channel CUE (PFL) buttons. The note number identifies the deck on its own
// (1 = A, 2 = C, 5 = B, 7 = D) so this works on any MIDI channel.
MC3000.pflButton = function(channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    let deck;
    switch (control) {
    case 1: deck = 1; break;    // CUE DECK A
    case 2: deck = 3; break;    // CUE DECK C
    case 5: deck = 2; break;    // CUE DECK B
    case 7: deck = 4; break;    // CUE DECK D
    default: return;
    }
    MC3000.toggle(`[Channel${deck}]`, "pfl");
};

// ============================================================================
//  BROWSER / CENTRE SECTION
// ============================================================================

// TRACK SELECT knob, CC 84. The MC3000 sends 0 for one direction and 127 for
// the other (it is not a standard 64-centred encoder).
MC3000.trackKnob = function(channel, control, value) {
    let step = (value === 0) ? 1 : -1;
    if (MC3000.anyShift()) {
        step *= 5;                       // fast scroll
    }
    engine.setValue("[Library]", "MoveVertical", step);
};

// TRACK SELECT push, note 40. SHIFT = move focus between library panes.
MC3000.trackKnobPress = function(channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    if (MC3000.anyShift()) {
        engine.setValue("[Library]", "MoveFocusForward", 1);
    } else {
        engine.setValue("[Library]", "GoToItem", 1);
    }
};

// BACK (note 48) and FWD (note 41).
// Unshifted: move the focus between the library panes.
// SHIFT: load the selected track into the preview deck / play-pause it.
MC3000.backFwd = function(channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    const forward = (control === 41);

    if (MC3000.anyShift()) {
        if (forward) {
            MC3000.toggle("[PreviewDeck1]", "play");
        } else {
            engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 1);
        }
        return;
    }
    engine.setValue("[Library]", forward ? "MoveFocusForward" : "MoveFocusBackward", 1);
};

// LOAD A/C (note 98) and LOAD B/D (note 99). SHIFT = eject that deck instead.
//
// Unlike the rest of the deck section, LOAD's own MIDI channel does NOT move
// with the layer (confirmed on real hardware), so the target deck comes from
// the side (the note) and MC3000.deckOnSide.
// Load the selected library track into whichever deck this side is showing,
// or eject it if SHIFT is held.
MC3000.loadOnSide = function(side) {
    const group = `[Channel${MC3000.deckOnSide[side]}]`;
    if (MC3000.anyShift()) {
        // SHIFT = eject. Pulse the push control (press then release).
        engine.setValue(group, "eject", 1);
        engine.setValue(group, "eject", 0);
    } else {
        // LoadSelectedTrack is a binary PUSH control: it must be set back to 0
        // or Mixxx treats it as still held and ignores the next press. Leaving
        // it latched at 1 was why LOAD stopped working after the first use.
        engine.setValue(group, "LoadSelectedTrack", 1);
        engine.setValue(group, "LoadSelectedTrack", 0);
    }
};

MC3000.load = function(channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    MC3000.loadOnSide((control === 98) ? 0 : 1);
};

// BROWSE, note 100. SHIFT = show / hide the sampler bank.
MC3000.browse = function(channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    if (MC3000.anyShift()) {
        MC3000.toggleCompat("showCoverArt");
    } else {
        MC3000.toggleCompat("maximizeLibrary");
    }
};

// SAMPLE window button, note 76. Shows / hides the sampler bank.
MC3000.sampleWindow = function(channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    MC3000.toggleCompat("showSamplers");
};

// EFX window button, note 77. Shows / hides the effect rack.
MC3000.efxWindow = function(channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    MC3000.toggleCompat("showEffects");
};

// RECORD, note 101. SHIFT = enable / disable Auto DJ.
MC3000.record = function(channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    if (MC3000.anyShift()) {
        MC3000.toggle("[AutoDJ]", "enabled");
    } else {
        engine.setValue("[Recording]", "toggle_recording", 1);
    }
};

// CF MODE, note 27. Toggles split cue in the headphones.
MC3000.cfMode = function(channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    MC3000.toggle("[Master]", "headSplit");
};


// ============================================================================
//  LED CALLBACKS
// ============================================================================

MC3000.onPlay = function(value, group) {
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.play, value);
};

MC3000.onCue = function(value, group) {
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.cue, value);
};

MC3000.onKeylock = function(value, group) {
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.keylock, value);
};

MC3000.onSync = function(value, group) {
    const deck = MC3000.deckFromGroup(group);
    if (MC3000.endOfTrack[deck - 1]) {
        return;                          // the warning owns the LED right now
    }
    MC3000.setLed(deck, MC3000.led.sync, value);
};

MC3000.onPfl = function(value, group) {
    const deck = MC3000.deckFromGroup(group);
    MC3000.setLed2(deck, MC3000.pflLed[deck - 1], value);
};

MC3000.onLoopEnabled = function(value, group) {
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.autoLoop, value);
};

MC3000.onLoopStart = function(value, group) {
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.loopIn, value !== -1);
};

MC3000.onLoopEnd = function(value, group) {
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.loopOut, value !== -1);
};

MC3000.onHotcue = function(value, group, control) {
    const match = control.match(/^hotcue_(\d+)_(?:status|enabled)$/);
    if (match === null) {
        return;
    }
    const index = parseInt(match[1], 10) - 1;
    if (index < 0 || index > 7) {
        return;
    }
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.hotcue[index], value > 0);
};


MC3000.onFxAssign = function(value, group, control) {
    const match = control.match(/^group_\[Channel(\d+)\]_enable$/);
    if (match === null) {
        return;
    }
    const deck = parseInt(match[1], 10);
    const unitNo = (group.indexOf("EffectUnit1") !== -1) ? 1 : 2;
    MC3000.setLed(deck, unitNo === 1 ? MC3000.led.fxOn1 : MC3000.led.fxOn2, value);
};

MC3000.onVuMeter = function(value, group) {
    const deck = MC3000.deckFromGroup(group);
    if (deck < 1) {
        return;
    }
    const lit = Math.round(value * 7);
    if (lit === MC3000.vuLedsLit[deck - 1]) {
        return;                          // nothing changed, do not flood the MIDI bus
    }
    const previous = MC3000.vuLedsLit[deck - 1];
    const base = MC3000.vuLedBase[deck - 1];
    let i;
    if (lit > previous) {
        for (i = Math.max(previous, 0); i < lit; i++) {
            MC3000.setLed2(deck, base + i, true);
        }
    } else {
        for (i = Math.max(lit, 0); i < previous; i++) {
            MC3000.setLed2(deck, base + i, false);
        }
    }
    MC3000.vuLedsLit[deck - 1] = lit;
};

MC3000.onSampler = function(value, group) {
    const match = group.match(/^\[Sampler(\d+)\]$/);
    if (match === null) {
        return;
    }
    const number = parseInt(match[1], 10);
    if (number < 1 || number > 8) {
        return;
    }
    const side = (number <= 4) ? 0 : 1;
    const deck = MC3000.deckOnSide[side];
    MC3000.setLed(deck, MC3000.led.samp[side][(number - 1) % 4], MC3000.samplerLedState(number));
};

MC3000.onPlayPosition = function(value, group) {
    const deck = MC3000.deckFromGroup(group);
    if (deck < 1) {
        return;
    }
    const warn = (value > MC3000.endOfTrackWarningPosition) && (value < 1.0);
    if (warn === MC3000.endOfTrack[deck - 1]) {
        return;
    }
    MC3000.endOfTrack[deck - 1] = warn;
    if (warn) {
        MC3000.setLed(deck, MC3000.led.sync, 2);          // blink
    } else {
        MC3000.setLed(deck, MC3000.led.sync, engine.getValue(group, "sync_enabled"));
    }
};

// ****************************************************************************
//  CONTROL REFERENCE
//
//  [AI disclaimer: this reference was written autonomously by an AI agent
//   (Claude) and has to be reviewed by the human contributor.]
// ****************************************************************************
//
//  DECK SECTION (works identically on decks A, B, C and D)
//  ---------------------------------------------------------------------------
//  Control            Normal                       + SHIFT (same side)
//  ---------------------------------------------------------------------------
//  PLAY / PAUSE       play / pause                 reverse roll (censor)
//  CUE                cue (Mixxx cue behaviour)    jump to start of track
//  SYNC               sync lock on / off           become sync leader
//                     (hold 1 s = momentary sync)
//  KEY LOCK           key lock on / off            tap BPM
//  VINYL MODE         scratch <-> bend             lock the jog wheel (blinks)
//  PITCH BEND - / +   temporary tempo nudge        rewind / fast forward
//  JOG (touch top)    scratch                      -
//  JOG (side)         pitch bend                   scrub through the track
//  PITCH FADER        tempo                        -
//  CUE 1 - 4          hot cue 1 - 4                delete that hot cue
//  CUE 5 - 8 mode on  hot cue 5 - 8                delete that hot cue
//  LOOP IN            set loop in                  clear the loop
//  LOOP OUT           set loop out                 clear the loop
//  AUTO LOOP          loop on / off                reloop (re-enter last loop)
//  AUTO LOOP -        halve loop length            move loop 1 beat back
//  AUTO LOOP +        double loop length           move loop 1 beat forward
//  FX ON 1            route deck to EffectUnit1    quantize on / off
//  FX ON 2            route deck to EffectUnit2    move beatgrid to playhead
//
//  EFX SECTION (SAMP. MODE off) - the left section drives EffectUnit1, the
//  right one EffectUnit2 (standard ComponentsJS effects mapping)
//  ---------------------------------------------------------------------------
//  EFX 1 / 2 / 3      effect 1 / 2 / 3 on-off      -
//                     (hold = momentary)
//  EFX 4              effect focus (hold = choose  cycle the unit
//                     the effect to focus)
//  EFX 1 / 2 / 3 knob metaknob of effect 1 / 2 / 3 choose another effect
//  EFX 4 knob         dry / wet                    super knob of the unit
//
//  SAMP. MODE on
//  ---------------------------------------------------------------------------
//  SAMP 1 - 4 (left)  play sampler 1 - 4           stop, then eject
//  SAMP 1 - 4 (right) play sampler 5 - 8           stop, then eject
//  (pressing an empty sampler loads the selected library track into it)
//  SAMP knobs         sampler gain                 -
//
//  MIXER
//  ---------------------------------------------------------------------------
//  LEVEL / HI / MID / LOW / FILTER / channel fader / crossfader  -> direct
//  CUE (channel)      headphone cue on / off
//
//  BROWSER / CENTRE
//  ---------------------------------------------------------------------------
//  TRACK SELECT turn  move up / down in library    scroll 5 rows at a time
//  TRACK SELECT push  open folder / choose item    move focus between panes
//  BACK               focus previous pane          load track in preview deck
//  FWD                focus next pane              preview deck play / pause
//  DECK CHG A/B/C/D   select that deck outright - moves the jog wheel, play,
//                     cue, EQ, pitch, everything (see MC3000.deckButton())
//  DECK A/C, DECK B/D (CHANNEL SELECT) recall the other deck on that side
//                     (A<->C, B<->D) for LOAD to target
//  LOAD A/C           load into the left side's    eject that deck
//                     current deck
//  LOAD B/D           load into the right side's   eject that deck
//                     current deck
//  BROWSE             maximise library             library cover art on / off
//  SAMPLE             show / hide samplers
//  EFX                show / hide effect rack
//  RECORD             start / stop recording       Auto DJ on / off
//  CF MODE            split cue (headphones) on / off
//
//  [End of AI-written comment: see the disclaimer above.]
// ****************************************************************************
