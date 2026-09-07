// -------------------------------------------------------------------
// ------------------- DDJ-FLX10 script file v.1.1 ---------------------
// -------------------------------------------------------------------

// *************************************************************************
// * Mixxx mapping script file for the Pioneer DDJ-FLX10.
// * Mostly adapted from the DDJ-1000 mapping script from Arnold Kalambani
// * Author: Marc Zischka (Zim)
// * Contributions: Victor Pineda (Veezuhz)
// ****************************************************************************
//
//  Implemented (as per manufacturer's manual):
//      * Mixer Section (Faders, EQ, Filter, Gain, Cue)
//      * Browsing and loading 
//      * Jogwheels, Scratching, Bending, Loop adjust ?
//      * Cycle Tempo Range
//      * Beat Sync
//      * Hot Cue Mode
//      * Beat Loop Mode
//      * Beat Jump Mode
//      * Sampler Mode
//      * Toggle quantize
//      * Toggle slip
//		* Reverse play 
//++++++++++++++++++++++++++++++++++++++++++++++++++++
// Check :
//  Custom (Mixxx specific mappings):
//      * BeatFX: Assigned Effect Unit 1
//                v FX_SELECT focus EFFECT1.
//                < LEFT focus EFFECT2
//                > RIGHT focus EFFECT3
//                ON/OFF toggles focused effect slot
//                SHIFT + ON/OFF disables all three effect slots.
//                SHIFT + < loads previous effect
//                SHIFT + > loads next effect
//
//      * 32 beat jump forward & back (Shift + </> CUE/LOOP CALL arrows)
//      * Toggle quantize (Shift + channel cue)
//      * Pad FX1 (see mapping infos)
//      * Pad FX2
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// To fix
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// To develop ?
//      * Loop Section:
//        * -4BEAT auto loop (hacky---prefer a clean way to set a 4 beat loop
//                            from a previous position on long press)
//
//        * CUE/LOOP CALL - memory & delete (complex and not useful. Hot cues are sufficient)
//
//      * Secondary pad modes (trial attempts complex and too experimental)
//        * Keyboard mode

//        * Keyshift mode
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DISCLAIMER: These additions were developed alongside Claude Code in VSCode.
// They are not part of the original mapping script and may not be fully tested.
// Please review the code and test it thoroughly before using it in a live setting.
//
// ════════════════ UPDATE LOG — NEWEST FIRST (date M-DD.HHMM) ════════════════
// Add new entries at the TOP of this block.
//
// 6-01.1830 — Veezuhz:
//   * Hot-cue pads: RGB LEDs color-matched to the cue colour, blink on saved loops
//   * Loop IN/OUT button LEDs: blink while setting/adjusting, solid while looping
//   * 4-beat loop button now loops the PREVIOUS 4 beats (end-anchored via loop_anchor)
//   * Shift+Loop Out reloops the last stored loop when none is active
//   * Needle-seek on the pressed jog (Shift = fast seek)
//   * Script-driven 14-bit tempo fader + TEMPO RESET hold-at-0% toggle
//   * KEY SYNC toggle + LED; BEAT SYNC LED
//   * EQ KILL on the per-deck stem buttons (Low/Mid/High, latching)
//   * Master stem buttons kill Low/Mid/High EQ across ALL decks at once
//   * Sound Color FX: intensity-knob soft-takeover fixed on all 4 decks (was CH1 only);
//                     deselecting a preset loads the empty '----' preset (no effect)
//   * Sampler pads trigger Sampler 1-8; pad LEDs lit when loaded, blink while playing
//   * BeatFX ON/OFF is press-to-toggle; bound for Serato (0x46) AND non-Serato (0x47) note modes
//   * All button LEDs re-pushed shortly after startup so they match Mixxx state on launch
//   * Note fixes: keylock 0x1C->0x4A, BeatFX 0x47->0x46
//
// 5-22.945 — Veezuhz:
//   * Tempo reset button management
//   * Time display mode (elapsed vs remaining) on Shift+Hot Cue
//   * connectControl -> makeConnection for high-frequency callbacks
//   * VuMeter -> vu_meter
//
// Earlier (undated) — Veezuhz:
//   * Beatgrid nudge (Shift + Jog)
//   * Waveform zoom (Shift + CH rotary)
//   * Memory cue navigation (Shift + Hot Cue pads)
//   * Beat Jump + Beat Jump Size (Shift + CUE/LOOP CALL arrows)
//   * Sound Color FX controls (Shift + Pad FX1/2)
//   * Crossfader assign A/THRU/B per channel (Shift + Channel Fader)
//   * Pioneer-like Play/Cue LED policy
//   * BeatFX effect scanner diagnostic (Shift + BeatFX ON)
//   * Loop in/out (Shift + Loop In/Out); Half & Double
// ════════════════════════════════════════════════════════════════════════════

// ─── TUNABLE PARAMETERS ──────────────────────────────────────────────────────
// Jog scratch sensitivity: how many encoder intervals equal one vinyl revolution.
// Higher = slower / less sensitive. Lower = faster / more sensitive.
// Typical range: 512 (very fast) → 4096 (very slow). Default: 2048.
var SCRATCH_INTERVALS_PER_REV = 1500;

// Jog pitch-bend divisor: how much a jog nudge shifts playback rate.
// Higher = smaller nudge. Lower = larger nudge.
var JOG_BEND_DIVISOR = 16;

// Loop adjust step: samples moved per jog unit when in loop in/out adjust mode.
// At 44.1 kHz, 100 samples ≈ 2.3 ms per minimum jog tick.
var LOOP_ADJUST_STEP = 100;
// Needle-seek step: samples the playhead moves per jog unit on the pressed jog
// (CC 0x1F) when no loop-adjust mode is active. Tune for seek feel.
// 106 engine samples/unit ≈ true 1:1 with a 33⅓ RPM record (1500 jog units =
// 1 revolution = 1.8 s of audio, matching the scratch-mode vinyl calibration).
var SEEK_STEP = 106;
// Fast needle-seek step when SHIFT is held with the pressed jog
// (shift + press + turn = CC 0x1F while shiftActive). Tune for fast-seek feel.
var FAST_SEEK_STEP = 4000;
// ─────────────────────────────────────────────────────────────────────────────

// Global variables
var PioneerDDJFLX10 = {};

// Global state
PioneerDDJFLX10.shiftActive = false;
PioneerDDJFLX10._rateMSB = {1: 0, 2: 0, 3: 0, 4: 0};
PioneerDDJFLX10._rateLSB = {1: 0, 2: 0, 3: 0, 4: 0};
PioneerDDJFLX10._jogTouches = {1: false, 2: false, 3: false, 4: false};
PioneerDDJFLX10._jogLastValue = {1: 0, 2: 0, 3: 0, 4: 0};
PioneerDDJFLX10._reverseSlipActive = {1: false, 2: false, 3: false, 4: false};
PioneerDDJFLX10._lastShiftPress = 0;
PioneerDDJFLX10._playCueBlinkTimer = {};
PioneerDDJFLX10._playCueBlinkState = {1: false, 2: false, 3: false, 4: false};
PioneerDDJFLX10._playCueShouldBlink = {1: false, 2: false, 3: false, 4: false};
PioneerDDJFLX10._cuePressed = {1: false, 2: false, 3: false, 4: false};
PioneerDDJFLX10._playShouldBlink = {1: false, 2: false, 3: false, 4: false};
PioneerDDJFLX10._trackDuration = {1: 0, 2: 0, 3: 0, 4: 0};

// Utility function to extract deck number from group
PioneerDDJFLX10._getDeckFromGroup = function(group) {
    var match = group.match(/\d+/);
    return match ? parseInt(match[0], 10) : 1;
};

// ─── Deferred LED state refresh ──────────────────────────────────────────────
// At launch Mixxx pushes initial LED (output) state, but the FLX10 isn't ready
// to receive yet and misses that burst; Mixxx's output handler then suppresses
// resends (it believes the LED already matches the value), so toggle LEDs stay
// wrong until their value next changes. Re-send the toggle LEDs (and re-render
// hot cues) directly a short delay after init, when the controller is ready.
// Writing MIDI here bypasses the handler's redundant-message guard.
var LED_REFRESH_DELAY_MS = 1200;
PioneerDDJFLX10._OUTPUT_LEDS = [
    {key: "play_indicator", note: 0x0B},
    {key: "cue_indicator",  note: 0x0C},
    {key: "quantize",       note: 0x35},
    {key: "keylock",        note: 0x4A},
    {key: "pfl",            note: 0x54},
    {key: "reverse",        note: 0x15},
    {key: "sync_enabled",   note: 0x58}
];
PioneerDDJFLX10._refreshOutputLeds = function() {
    for (var deck = 1; deck <= 4; deck++) {
        var status = 0x90 + (deck - 1);
        var group = "[Channel" + deck + "]";
        for (var i = 0; i < PioneerDDJFLX10._OUTPUT_LEDS.length; i++) {
            var led = PioneerDDJFLX10._OUTPUT_LEDS[i];
            midi.sendShortMsg(status, led.note,
                engine.getValue(group, led.key) > 0.5 ? 0x7F : 0x00);
        }
        // Re-render hot-cue pad LEDs (same startup-timing vulnerability).
        if (PioneerDDJFLX10._hotcue && PioneerDDJFLX10._hotcue[deck]) {
            for (var pad = 1; pad <= 8; pad++) {
                PioneerDDJFLX10._renderHotcue(deck, pad);
            }
        }
        // Re-render loop in/out button LEDs.
        if (PioneerDDJFLX10._renderLoopLeds) {
            PioneerDDJFLX10._renderLoopLeds(deck);
        }
        // Re-render EQ kill LEDs (repurposed stem buttons).
        if (PioneerDDJFLX10._renderEqKillLed) {
            for (var k = 0; k < 3; k++) {
                PioneerDDJFLX10._renderEqKillLed(deck, k);
            }
        }
    }
    // Master EQ kill LEDs (once — these span all decks).
    if (PioneerDDJFLX10._renderMasterEqKillLed) {
        for (var mi = 0; mi < 3; mi++) {
            PioneerDDJFLX10._renderMasterEqKillLed(mi);
        }
    }
    // Sampler pad LEDs (once — Sampler1-8).
    if (PioneerDDJFLX10._renderSamplerLed) {
        for (var sn = 1; sn <= 8; sn++) {
            PioneerDDJFLX10._renderSamplerLed(sn);
        }
    }
};

// Initialization
PioneerDDJFLX10.init = function(id) {
    console.log("Pioneer DDJ-FLX10 PROD - Initialisation");
    var initTimeMode = engine.getValue("[Controls]", "ShowDurationRemaining") !== 0 ? 0x7F : 0x00;
    // Plage de pitch par défaut (±16%)
    for (var i = 1; i <= 4; i++) {
        var grp = "[Channel" + i + "]";
        engine.setValue(grp, "rateRange", 0.16);
        // Seed the tempo fader to CENTER (0x40 MSB / 0 LSB ≈ rate 0) so
        // _sliderRate() returns ~0 before the fader is physically moved —
        // otherwise a 0/0 reading computes +1 (max) and releasing TEMPO RESET
        // before touching the fader would jump tempo to max.
        PioneerDDJFLX10._rateMSB[i] = 0x40;
        PioneerDDJFLX10._rateLSB[i] = 0x00;
        // Clear sync/tempo-reset button LEDs — the FLX10 powers up with these
        // lit and Mixxx doesn't proactively send the off state.
        midi.sendShortMsg(0x90 + (i - 1), 0x58, 0x00);   // beat sync off
        midi.sendShortMsg(0x90 + (i - 1), 0x65, 0x00);   // key sync off
        midi.sendShortMsg(0x90 + (i - 1), 0x41, 0x00);   // tempo reset off
        // Connect VU meters
        engine.makeConnection(grp, "vu_meter", PioneerDDJFLX10["LedVuMeterCH" + i]);
        // Force off au démarrage pour éviter LEDs fantômes
        midi.sendShortMsg(0xB0 + (i - 1), 0x02, 0x00);
        // Jog display: enable visibility, set time mode, then connect data callbacks
        midi.sendShortMsg(JOG_DISPLAY_NOTE, 0x5C + i, 0x00);
        midi.sendShortMsg(JOG_DISPLAY_NOTE, _JOG_TIME_MODE[i - 1], initTimeMode);
        engine.makeConnection(grp, "playposition", PioneerDDJFLX10.jogMarker);
        engine.makeConnection(grp, "bpm",          PioneerDDJFLX10.jogBpm);
        engine.makeConnection(grp, "rate",         PioneerDDJFLX10.jogSpeed);
        engine.makeConnection(grp, "duration",     PioneerDDJFLX10.jogDuration);
        // Seed duration cache so jogTime works immediately at init
        PioneerDDJFLX10._trackDuration[i] = engine.getValue(grp, "duration");
        // Push initial values so displays start at correct state, not hardware defaults
        PioneerDDJFLX10.jogMarker(engine.getValue(grp, "playposition"), grp, "playposition");
        PioneerDDJFLX10.jogBpm(engine.getValue(grp, "bpm"), grp, "bpm");
        PioneerDDJFLX10.jogSpeed(engine.getValue(grp, "rate"), grp, "rate");
        PioneerDDJFLX10.jogTime(engine.getValue(grp, "playposition"), grp, "playposition");
    }
    // Poll all four decks every 250 ms and push time to jog displays.
    // Using a timer rather than a playposition callback because Mixxx throttles
    // high-frequency CO callbacks on some builds and the timer is unconditional.
    engine.beginTimer(250, function() {
        var showRemaining = engine.getValue("[Controls]", "ShowDurationRemaining") !== 0;
        for (var d = 1; d <= 4; d++) {
            var duration = PioneerDDJFLX10._trackDuration[d];
            if (!(duration > 0)) { continue; }
            var pos = engine.getValue("[Channel" + d + "]", "playposition");
            var idx = d - 1;
            var elapsed = pos * duration;
            var displayTime = showRemaining ? (duration - elapsed) : elapsed;
            var min = Math.floor(displayTime / 60);
            var sec = Math.floor(displayTime % 60);
            if (min > 127) { min = 127; }
            midi.sendShortMsg(JOG_DISPLAY_CC, _JOG_TIME_MIN[idx], min);
            midi.sendShortMsg(JOG_DISPLAY_CC, _JOG_TIME_SEC[idx], sec);
        }
    });

    // LEDs Play/Cue avancées (Pioneer-like)
    PioneerDDJFLX10._initAdvancedLeds();

    // Hot-cue pad RGB LEDs (color-matched to Mixxx hot-cue colors, blink on loops)
    PioneerDDJFLX10._initHotcueLeds();

    // Loop in/out button LEDs (blink while setting/adjusting, solid when active)
    PioneerDDJFLX10._initLoopLeds();

    // EQ kill switches on the repurposed STEM buttons (lit = band on)
    PioneerDDJFLX10._initEqKillLeds();

    // Master EQ kills (master stem buttons → kill a band on all decks)
    PioneerDDJFLX10._initMasterEqKillLeds();

    // Sampler pad LEDs (lit when a sampler is loaded, blink while playing)
    PioneerDDJFLX10._initSamplerLeds();

    // Re-push LED state once the controller is ready to receive — the FLX10
    // misses Mixxx's initial output burst at launch (see _refreshOutputLeds).
    engine.beginTimer(LED_REFRESH_DELAY_MS, function() {
        PioneerDDJFLX10._refreshOutputLeds();
    }, true);

    if (typeof PioneerDDJFLX10.screen !== "undefined") {
        PioneerDDJFLX10.screen.start();
    } else {
        console.log("FLX10 screen: module not loaded — HID interface may be unavailable");
    }

    return true;
};

// Tempo management (14-bit) — inversion Pioneer
PioneerDDJFLX10.rate_msb = function(channel, control, value, status, group) {
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    PioneerDDJFLX10._rateMSB[deck] = value;
    PioneerDDJFLX10._updateRate(deck);
};

PioneerDDJFLX10.rate_lsb = function(channel, control, value, status, group) {
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    PioneerDDJFLX10._rateLSB[deck] = value;
    PioneerDDJFLX10._updateRate(deck);
};

// Convert the deck's 14-bit pitch-fader position (_rateMSB/_rateLSB) to a
// Mixxx `rate` value (-1..+1, Pioneer-inverted).
PioneerDDJFLX10._sliderRate = function(deck) {
    var msb = PioneerDDJFLX10._rateMSB[deck] || 0;
    var lsb = PioneerDDJFLX10._rateLSB[deck] || 0;
    var value = (msb << 7) | lsb;                 // 0..16383
    var norm  = (value / 16383.0) * 2.0 - 1.0;    // -1..+1
    norm = -norm;                                 // inversion Pioneer
    if (norm > 1) norm = 1;
    if (norm < -1) norm = -1;
    return norm;
};

PioneerDDJFLX10._updateRate = function(deck) {
    // While TEMPO RESET is engaged the fader is disconnected: keep tracking its
    // position (_rateMSB/_rateLSB are still updated by the handlers) but DON'T
    // apply it, so the tempo stays pinned at 0%. Releasing the button picks up
    // the fader's then-current position.
    if (PioneerDDJFLX10._tempoResetEngaged &&
        PioneerDDJFLX10._tempoResetEngaged[deck]) {
        return;
    }
    engine.setValue("[Channel" + deck + "]", "rate",
                    PioneerDDJFLX10._sliderRate(deck));
};

// ─── TEMPO RESET button (note 0x41) — hold at 0% ─────────────────────────────
// 1st press (ENGAGE): force tempo to 0% (the track's original BPM) and light
//   the button. While engaged the pitch fader is DISCONNECTED — moving it does
//   nothing to the tempo (see _updateRate) — so you can physically slide the
//   fader back to 0 without the tempo jumping around.
// 2nd press (RELEASE): clear the light and snap the tempo to wherever the fader
//   now physically sits (so if you re-zeroed it, tempo stays at 0% smoothly).
PioneerDDJFLX10._tempoResetEngaged = {1: false, 2: false, 3: false, 4: false};

PioneerDDJFLX10.tempoResetToggle = function(channel, control, value, status, group) {
    if (value === 0) { return; }   // act on press only
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    var engaged = !PioneerDDJFLX10._tempoResetEngaged[deck];
    PioneerDDJFLX10._tempoResetEngaged[deck] = engaged;
    if (engaged) {
        engine.setValue(group, "rate", 0.0);                        // hold at 0%
    } else {
        engine.setValue(group, "rate", PioneerDDJFLX10._sliderRate(deck)); // follow fader
    }
    midi.sendShortMsg(status, control, engaged ? 0x7F : 0x00);      // LED on note 0x41
};

// Nudge the active loop's in or out boundary in place (engine samples), without
// moving the playhead. loop_start_position / loop_end_position are writable COs
// that adjust the loop directly (Mixxx clamps to keep in < out). No-op if the
// boundary isn't set (cur < 0).
PioneerDDJFLX10._nudgeLoop = function(group, deck, newValue) {
    var ctrl = PioneerDDJFLX10._loopAdjustMode[deck] === "in"
        ? "loop_start_position"
        : "loop_end_position";
    var cur = engine.getValue(group, ctrl);
    if (cur >= 0) {
        engine.setValue(group, ctrl, cur + newValue * LOOP_ADJUST_STEP);
    }
};

// Pressed jog (CC 0x1F) — "press and seek". In loop-adjust mode it nudges the
// selected in/out boundary; otherwise it needle-seeks the playhead.
PioneerDDJFLX10.wheelSeek = function(channel, control, value, status, group) {
    var newValue = value - 64;
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    if (PioneerDDJFLX10._loopAdjustMode[deck]) {
        PioneerDDJFLX10._nudgeLoop(group, deck, newValue);
        return;
    }
    // SHIFT held = fast seek (shift + press + turn); otherwise normal seek.
    var step = PioneerDDJFLX10.shiftActive ? FAST_SEEK_STEP : SEEK_STEP;
    var total = engine.getValue(group, "track_samples");
    if (total > 0) {
        var pos = engine.getValue(group, "playposition") * total + newValue * step;
        engine.setValue(group, "playposition",
            Math.max(0, Math.min(total, pos)) / total);
    }
};

// ─── KEY SYNC button (note 0x65) — tap-to-toggle ─────────────────────────────
PioneerDDJFLX10._keySyncEngaged = {1: false, 2: false, 3: false, 4: false};

PioneerDDJFLX10._setKeySyncLed = function(deck, on) {
    midi.sendShortMsg(0x90 + (deck - 1), 0x65, on ? 0x7F : 0x00);
};

PioneerDDJFLX10.keySyncToggle = function(channel, control, value, status, group) {
    if (value === 0) { return; }   // act on press only; ignore the release
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    var engaged = !PioneerDDJFLX10._keySyncEngaged[deck];
    PioneerDDJFLX10._keySyncEngaged[deck] = engaged;
    // sync_key / reset_key are edge-triggered: pulse 1 → 0 so a later press
    // can re-fire (setting 1 and leaving it would block the next trigger).
    var key = engaged ? "sync_key" : "reset_key";
    engine.setValue(group, key, 1);
    engine.setValue(group, key, 0);
    PioneerDDJFLX10._setKeySyncLed(deck, engaged);
};

// Shutdown
PioneerDDJFLX10.shutdown = function() {
    console.log("Pioneer DDJ-FLX10 PROD - Arrêt");
    if (typeof PioneerDDJFLX10.screen !== "undefined") {
        PioneerDDJFLX10.screen.stop();
    }
    
    // Turn off all LEDs
    for (var i = 1; i <= 4; i++) {
        var group = "[Channel" + i + "]";
        midi.sendShortMsg(0x90 + (i-1), 0x00, 0x00); // Cue LED off
        midi.sendShortMsg(0x90 + (i-1), 0x0B, 0x00); // Play LED off
        // Add other LEDs to turn off if needed
    }
};

// Jog Wheel Management
PioneerDDJFLX10._loopAdjustMode = {1: null, 2: null, 3: null, 4: null};

PioneerDDJFLX10.wheelTurn = function(channel, control, value, status, group) {
    var newValue = value - 64;
    var deckNumber = PioneerDDJFLX10._getDeckFromGroup(group);
    // Loop-adjust takes priority so a touched platter nudges the boundary
    // instead of scratching/seeking.
    if (PioneerDDJFLX10._loopAdjustMode[deckNumber]) {
        PioneerDDJFLX10._nudgeLoop(group, deckNumber, newValue);
    } else if (engine.isScratching(deckNumber)) {
        engine.scratchTick(deckNumber, newValue);
    } else {
        engine.setValue(group, 'jog', PioneerDDJFLX10.sensitivityMinimizer(newValue, JOG_BEND_DIVISOR));
    }
};

// VU-meters (LEDs faders)
PioneerDDJFLX10._sendVu = function(status, ctrl, value) {
    var v = Math.floor(Math.max(0, Math.min(1, value)) * 127);
    midi.sendShortMsg(status, ctrl, v);
};
PioneerDDJFLX10.LedVuMeterCH1 = function (value) { PioneerDDJFLX10._sendVu(0xB0, 0x02, value); };
PioneerDDJFLX10.LedVuMeterCH2 = function (value) { PioneerDDJFLX10._sendVu(0xB1, 0x02, value); };
PioneerDDJFLX10.LedVuMeterCH3 = function (value) { PioneerDDJFLX10._sendVu(0xB2, 0x02, value); };
PioneerDDJFLX10.LedVuMeterCH4 = function (value) { PioneerDDJFLX10._sendVu(0xB3, 0x02, value); };

PioneerDDJFLX10.wheelTouch = function(channel, control, value, status, group) {
    var deckNumber = PioneerDDJFLX10._getDeckFromGroup(group);
    // While adjusting a loop boundary, don't scratch — keep the loop playing.
    if (PioneerDDJFLX10._loopAdjustMode[deckNumber]) {
        return;
    }
    if (value === 0x7F) {
        engine.scratchEnable(deckNumber, SCRATCH_INTERVALS_PER_REV, 33+1/3, 1.0/8, (1.0/8)/32);
    } else {
        engine.scratchDisable(deckNumber, false);
    }
};

// Beatgrid nudge — triggered by CC 0x26 when Shift+Jog is used.
// Values are relative, centered on 64: below = earlier, above = later.
// Accumulate delta across ticks; fire one step per BEATGRID_THRESHOLD units
// so a slow spin doesn't trigger hundreds of nudges. Raise the threshold to
// slow down, lower it to speed up.
PioneerDDJFLX10._beatgridAccum = {1: 0, 2: 0, 3: 0, 4: 0};
var BEATGRID_THRESHOLD = 6;

// Jog wheel display — output on MIDI channel 16 (0xBF = CC, 0x9F = Note On)
var JOG_DISPLAY_CC   = 0xBF;
var JOG_DISPLAY_NOTE = 0x9F;
var _JOG_MARKER_MSB  = [0x10, 0x11, 0x12, 0x13];
var _JOG_MARKER_LSB  = [0x30, 0x31, 0x32, 0x33];
var _JOG_BPM_MSB     = [0x14, 0x15, 0x16, 0x17];
var _JOG_BPM_LSB     = [0x34, 0x35, 0x36, 0x37];
var _JOG_SPEED_MSB   = [0x18, 0x19, 0x1A, 0x1B];
var _JOG_SPEED_LSB   = [0x38, 0x39, 0x3A, 0x3B];
var _JOG_TIME_MIN    = [0x42, 0x44, 0x46, 0x48];
var _JOG_TIME_SEC    = [0x43, 0x45, 0x47, 0x49];
var _JOG_TIME_MODE   = [0x14, 0x15, 0x16, 0x17]; // Note on 0x9F: 0x00=elapsed, 0x7F=remaining (hardware renders minus sign)

PioneerDDJFLX10.beatgridAdjust = function(channel, control, value, status, group) {
    var delta = value - 64;
    if (delta === 0) return;
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    var accum = PioneerDDJFLX10._beatgridAccum[deck];

    // Reset accumulator on direction change to avoid phantom steps
    if ((delta < 0 && accum > 0) || (delta > 0 && accum < 0)) {
        accum = 0;
    }
    accum += delta;

    if (Math.abs(accum) >= BEATGRID_THRESHOLD) {
        var ctrl = accum < 0 ? "beats_translate_earlier" : "beats_translate_later";
        engine.setValue(group, ctrl, 1);
        engine.setValue(group, ctrl, 0);
        accum = 0;
    }
    PioneerDDJFLX10._beatgridAccum[deck] = accum;
};

// Waveform zoom — triggered by Shift+CH rotary (0xB6, CC 0x64).
// Encoder uses two's complement: 1 = CW (zoom in), 127 = CCW (zoom out).
// waveform_zoom=1 is most zoomed in; higher values zoom out. CW decreases value.
PioneerDDJFLX10.waveformZoom = function(channel, control, value, status, group) {
    var delta = value < 64 ? -1 : 1;
    var zoom = engine.getValue(group, "waveform_zoom") + delta;
    engine.setValue(group, "waveform_zoom", Math.max(1, zoom));
};

// ─── Jog wheel display ────────────────────────────────────────────────────────
// All data is sent to MIDI channel 16 (0xBF for CC, 0x9F for Note On).
// Protocol sourced from Loui1979 / community reverse-engineering of the FLX10.

PioneerDDJFLX10._jogSend14 = function(msbCC, lsbCC, value14) {
    midi.sendShortMsg(JOG_DISPLAY_CC, msbCC, (value14 >> 7) & 0x7F);
    midi.sendShortMsg(JOG_DISPLAY_CC, lsbCC, value14 & 0x7F);
};

// Rotating marker: playposition (0–1) mapped to 0–359 degrees
PioneerDDJFLX10.jogMarker = function(value, group, control) {
    var i = PioneerDDJFLX10._getDeckFromGroup(group) - 1;
    var deg = Math.round(value * 359);
    if (deg < 0) { deg = 0; }
    if (deg > 359) { deg = 359; }
    PioneerDDJFLX10._jogSend14(_JOG_MARKER_MSB[i], _JOG_MARKER_LSB[i], deg);
};

// BPM display: bpm * 10 as a 14-bit integer
PioneerDDJFLX10.jogBpm = function(value, group, control) {
    var i = PioneerDDJFLX10._getDeckFromGroup(group) - 1;
    var bpm10 = Math.round(value * 10);
    if (bpm10 < 0) { bpm10 = 0; }
    if (bpm10 > 16383) { bpm10 = 16383; }
    PioneerDDJFLX10._jogSend14(_JOG_BPM_MSB[i], _JOG_BPM_LSB[i], bpm10);
};

// Speed display: rate (-1..+1) → centered at 0% → (rate*100+100)*10, 14-bit
PioneerDDJFLX10.jogSpeed = function(value, group, control) {
    var i = PioneerDDJFLX10._getDeckFromGroup(group) - 1;
    var pct10 = Math.round((value * 100 + 100) * 10);
    if (pct10 < 0) { pct10 = 0; }
    if (pct10 > 16383) { pct10 = 16383; }
    PioneerDDJFLX10._jogSend14(_JOG_SPEED_MSB[i], _JOG_SPEED_LSB[i], pct10);
};

// Cache track duration when it changes (new track loaded).
// jogTime reads from this cache instead of calling engine.getValue("duration")
// on every playposition tick, which can return 0 mid-callback on some systems.
PioneerDDJFLX10.jogDuration = function(value, group, control) {
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    PioneerDDJFLX10._trackDuration[deck] = value;
};

// Time display: minutes and seconds sent as separate single-byte CCs.
// Respects [Controls]/ShowDurationRemaining: 0=elapsed, 1 or 2=remaining.
// value is playposition (0–1) passed directly from the callback — do not re-read it.
PioneerDDJFLX10.jogTime = function(value, group, control) {
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    var i = deck - 1;
    var duration = PioneerDDJFLX10._trackDuration[deck];
    if (!(duration > 0)) { return; }
    var elapsed = value * duration;
    var showRemaining = engine.getValue("[Controls]", "ShowDurationRemaining") !== 0;
    var displayTime = showRemaining ? (duration - elapsed) : elapsed;
    var min = Math.floor(displayTime / 60);
    var sec = Math.floor(displayTime % 60);
    if (min > 127) { min = 127; }
    midi.sendShortMsg(JOG_DISPLAY_CC, _JOG_TIME_MIN[i], min);
    midi.sendShortMsg(JOG_DISPLAY_CC, _JOG_TIME_SEC[i], sec);
};

// Sensitivity functions
PioneerDDJFLX10.sensitivityMinimizer = function (value, factor) {
    return (value/factor);
};

PioneerDDJFLX10.sensitivityMaximizer = function (value, factor) {
    return (value*factor);
};

// ─── Memory cue navigation ───────────────────────────────────────────────────
// Hot cue slots are used as memory cues. Navigation finds the nearest set cue
// before/after the current playback position. Save finds the next empty slot.
var MEM_CUE_SLOTS = 36;

PioneerDDJFLX10.memCueSave = function(channel, control, value, status, group) {
    if (value === 0) return;
    for (var i = 1; i <= MEM_CUE_SLOTS; i++) {
        if (!engine.getValue(group, "hotcue_" + i + "_enabled")) {
            engine.setValue(group, "hotcue_" + i + "_set", 1);
            return;
        }
    }
};

PioneerDDJFLX10.memCuePrev = function(channel, control, value, status, group) {
    if (value === 0) return;
    var current = engine.getValue(group, "playposition") * engine.getValue(group, "track_samples");
    var best = -1, bestCue = -1;
    for (var i = 1; i <= MEM_CUE_SLOTS; i++) {
        if (engine.getValue(group, "hotcue_" + i + "_enabled")) {
            var pos = engine.getValue(group, "hotcue_" + i + "_position");
            if (pos < current - 2000 && pos > best) {
                best = pos; bestCue = i;
            }
        }
    }
    if (bestCue > 0) engine.setValue(group, "hotcue_" + bestCue + "_gotoandstop", 1);
};

PioneerDDJFLX10.memCueNext = function(channel, control, value, status, group) {
    if (value === 0) return;
    var current = engine.getValue(group, "playposition") * engine.getValue(group, "track_samples");
    var best = Infinity, bestCue = -1;
    for (var i = 1; i <= MEM_CUE_SLOTS; i++) {
        if (engine.getValue(group, "hotcue_" + i + "_enabled")) {
            var pos = engine.getValue(group, "hotcue_" + i + "_position");
            if (pos > current + 2000 && pos < best) {
                best = pos; bestCue = i;
            }
        }
    }
    if (bestCue > 0) engine.setValue(group, "hotcue_" + bestCue + "_gotoandstop", 1);
};

PioneerDDJFLX10.memCueDelete = function(channel, control, value, status, group) {
    if (value === 0) return;
    var current = engine.getValue(group, "playposition") * engine.getValue(group, "track_samples");
    var best = Infinity, bestCue = -1;
    for (var i = 1; i <= MEM_CUE_SLOTS; i++) {
        if (engine.getValue(group, "hotcue_" + i + "_enabled")) {
            var pos = engine.getValue(group, "hotcue_" + i + "_position");
            var dist = Math.abs(pos - current);
            if (dist < best) { best = dist; bestCue = i; }
        }
    }
    if (bestCue > 0) engine.setValue(group, "hotcue_" + bestCue + "_clear", 1);
};
// ─────────────────────────────────────────────────────────────────────────────

// Shift buttons management
PioneerDDJFLX10.shiftHandler = function(channel, control, value, status, group) {
    PioneerDDJFLX10.shiftActive = (value === 0x7F);
    var now = new Date().getTime();
    
    // Detect double-click (within 300ms)
    PioneerDDJFLX10._lastShiftPress = now;
};

// Whether the BeatFX ON button is currently active
PioneerDDJFLX10._beatFxActive = false;
PioneerDDJFLX10._beatFxFadeTimer = 0;

// Last activated preset position (so ON button can re-apply it)
PioneerDDJFLX10._lastPreset = null;


// ─────────────────────────────────────────────────────────────────────────────
// BeatFX pad handler — pad positions 1–14 map directly to the first 14 chain
// presets in Mixxx Preferences → Effects (in list order). Configure your
// presets there to match the 14 BEAT FX labels printed on the controller.
// ─────────────────────────────────────────────────────────────────────────────
PioneerDDJFLX10.beatFxPad = function(channel, control, value, status, group) {
    if (value === 0) return;
    var position = control - 0x20 + 1;
    if (position < 1 || position > 14) return;
    engine.setValue("[EffectRack1_EffectUnit1]", "loaded_chain_preset", position);
    PioneerDDJFLX10._lastPreset = position;
};

// Reverse with slip mode management
PioneerDDJFLX10.reverse = function(channel, control, value, status, group) {
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    
    if (value === 0x7F) { // Button pressed
        if (PioneerDDJFLX10.shiftActive) {
            // Toggle current state
            var currentReverse = engine.getValue(group, "reverse") || 0;
            engine.setValue(group, "reverse", currentReverse ? 0 : 1);
            engine.setValue(group, "slip_enabled", 0); // Disable slip mode
        } else {
            // Enable slip mode and reverse
            engine.setValue(group, "slip_enabled", 1);
            engine.setValue(group, "reverse", 1);
            PioneerDDJFLX10._reverseSlipActive[deck] = true;
        }
    } else if (value === 0x00 && PioneerDDJFLX10._reverseSlipActive[deck]) {
        // Button release with slip mode active
        engine.setValue(group, "reverse", 0);
        engine.setValue(group, "slip_enabled", 0);
        PioneerDDJFLX10._reverseSlipActive[deck] = false;
    }
};

// Loop functions with shift support - RESTORED for PROD version
PioneerDDJFLX10.LoopHalveShift = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        if (PioneerDDJFLX10.shiftActive) {
            engine.setValue(group, "loop_halve", 1);
    } else {
            engine.setValue(group, "loop_in", 1);
        }
    }
};

PioneerDDJFLX10.LoopDoubleShift = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        if (PioneerDDJFLX10.shiftActive) {
            engine.setValue(group, "loop_double", 1);
        } else {
            engine.setValue(group, "loop_out", 1);
        }
    }
};

// Rate range selector — Pioneer 6/10/16/20 %
PioneerDDJFLX10._rateRanges = [0.06, 0.10, 0.16, 0.20];
PioneerDDJFLX10.RangeSelector = function(channel, control, value, status, group) {
    if (value !== 0x7F) return;
    var current = engine.getValue(group, "rateRange");
    var idx = 0, best = 1e9;
    for (var i = 0; i < PioneerDDJFLX10._rateRanges.length; i++) {
        var d = Math.abs(current - PioneerDDJFLX10._rateRanges[i]);
        if (d < best) { best = d; idx = i; }
    }
    idx = (idx + 1) % PioneerDDJFLX10._rateRanges.length;
    engine.setValue(group, "rateRange", PioneerDDJFLX10._rateRanges[idx]);
};

// Time display mode management — [Controls]/ShowDurationRemaining: 0=elapsed, 1=remaining, 2=both
PioneerDDJFLX10.TimeTypeChange = function(channel, control, value, status, group) {
    if (value !== 0x7F) { return; }
    var current = engine.getValue("[Controls]", "ShowDurationRemaining");
    var newMode = current === 0 ? 1 : 0;
    engine.setValue("[Controls]", "ShowDurationRemaining", newMode);
    var hwMode = newMode !== 0 ? 0x7F : 0x00;
    for (var d = 0; d < 4; d++) {
        midi.sendShortMsg(JOG_DISPLAY_NOTE, _JOG_TIME_MODE[d], hwMode);
    }
};

// ─── HOT CUE pad LEDs ─────────────────────────────────────────────────────────
// Light each hot-cue pad when a hot cue exists at that slot, dark when empty.
// Pads are RGB (value = palette color 1..127, 0x00 = off). Hot-cue mode pads:
//   deck N → status 0x97 + (N-1)*2  (0x97/0x99/0x9B/0x9D), pad P → note (P-1).
// "Light up if it exists" = a fixed color when hotcue_P_enabled; per-cue colors
// would need the FLX10's palette map (future).
PioneerDDJFLX10._HOTCUE_STATUS    = {1: 0x97, 2: 0x99, 3: 0x9B, 4: 0x9D};
PioneerDDJFLX10._HOTCUE_DEFAULT_COLOR = 0x7F;  // pad palette value until we map real colors
PioneerDDJFLX10._HOTCUE_LOOP_TYPE = 4;         // Mixxx CueType::Loop (needs Mixxx 2.4+)
PioneerDDJFLX10._hotcuePhase = false;          // shared blink phase for loop cues
PioneerDDJFLX10._hotcue = {};                  // _hotcue[deck][pad] = {on, loop, color}

// FLX10 hot-cue pad palette (discovered 2026-05-29 via _hotcuePaletteTest).
// The pad value is a HUE WHEEL: value 1 = blue, sweeping teal→green→yellow→
// orange→red→pink→magenta→purple by ~61; values >61 saturate to purple. So we
// map any Mixxx hot-cue color to the palette entry with the nearest hue.
// {v: pad value, h: approximate hue in degrees}
PioneerDDJFLX10._HOTCUE_PALETTE = [
  {v: 1,  h: 240}, // blue
  {v: 5,  h: 228}, // blue
  {v: 9,  h: 216}, // blue
  {v: 13, h: 200}, // blue
  {v: 17, h: 180}, // teal / cyan
  {v: 21, h: 120}, // green
  {v: 25, h: 85},  // green-yellow
  {v: 29, h: 60},  // yellow
  {v: 33, h: 45},  // yellow-orange
  {v: 37, h: 30},  // orange
  {v: 41, h: 0},   // red
  {v: 45, h: 350}, // red-pink
  {v: 49, h: 340}, // light pink
  {v: 53, h: 330}, // pink
  {v: 57, h: 300}, // magenta
  {v: 61, h: 275}  // purple
];

PioneerDDJFLX10._rgbToHue = function(r, g, b){
  r /= 255; g /= 255; b /= 255;
  var max = Math.max(r, g, b), min = Math.min(r, g, b), d = max - min;
  if (d === 0) { return -1; }   // gray / white: no hue
  var h;
  if (max === r)      { h = ((g - b) / d) % 6; }
  else if (max === g) { h = (b - r) / d + 2; }
  else                { h = (r - g) / d + 4; }
  h *= 60;
  if (h < 0) { h += 360; }
  return h;
};

PioneerDDJFLX10._hueDist = function(a, b){
  var d = Math.abs(a - b) % 360;
  return d > 180 ? 360 - d : d;
};

// Value 127 is the whitest the pads get (faintly pink — there's no pure white
// in this palette). Used for white / near-white / grayscale cues.
PioneerDDJFLX10._HOTCUE_WHITE_VALUE = 127;

// Map a Mixxx hot-cue color (integer 0xRRGGBB from hotcue_X_color) to a pad
// palette value. Low-saturation colors (white/gray) → the white value;
// otherwise pick the palette entry with the nearest hue.
PioneerDDJFLX10._hotcueColorToPad = function(rgb){
  if (!(rgb > 0)) { return PioneerDDJFLX10._HOTCUE_WHITE_VALUE; }
  var r = (rgb >> 16) & 0xFF, g = (rgb >> 8) & 0xFF, b = rgb & 0xFF;
  var max = Math.max(r, g, b), min = Math.min(r, g, b);
  // Saturation (HSV). Near-white (and Mixxx's faintly-tinted white) has low
  // saturation — treat as white instead of computing a misleading hue.
  var sat = (max === 0) ? 0 : (max - min) / max;
  if (sat < 0.18) { return PioneerDDJFLX10._HOTCUE_WHITE_VALUE; }
  var hue = PioneerDDJFLX10._rgbToHue(r, g, b);
  if (hue < 0) { return PioneerDDJFLX10._HOTCUE_WHITE_VALUE; }
  var best = PioneerDDJFLX10._HOTCUE_PALETTE[0], bestD = 999;
  for (var i = 0; i < PioneerDDJFLX10._HOTCUE_PALETTE.length; i++) {
    var dist = PioneerDDJFLX10._hueDist(hue, PioneerDDJFLX10._HOTCUE_PALETTE[i].h);
    if (dist < bestD) { bestD = dist; best = PioneerDDJFLX10._HOTCUE_PALETTE[i]; }
  }
  return best.v;
};

PioneerDDJFLX10._renderHotcue = function(deck, pad){
  var s = PioneerDDJFLX10._hotcue[deck][pad];
  var val;
  if (!s.on)        { val = 0x00; }                                  // empty → off
  else if (s.loop)  { val = PioneerDDJFLX10._hotcuePhase ? s.color : 0x00; }  // loop → blink
  else              { val = s.color; }                               // cue → solid
  midi.sendShortMsg(PioneerDDJFLX10._HOTCUE_STATUS[deck], pad - 1, val);
};

// ── PALETTE DISCOVERY ─────────────────────────────────────────────────────
// Set true and reload the mapping to light every pad with a DISTINCT value
// (also logged), so you can report which value = which color. Each pad gets
// value = 1 + index*4 across all 4 decks: deck1 = 1,5,9,…,29; deck2 = 33,…,61;
// deck3 = 65,…,93; deck4 = 97,…,125 (toggle decks to see decks 3/4). Tell me
// the color for the values you can read, then set this back to false.
PioneerDDJFLX10._HOTCUE_PALETTE_TEST = false;

PioneerDDJFLX10._hotcuePaletteTest = function(){
  console.log("FLX10 HOTCUE PALETTE TEST (round 2) — switch pads to HOT CUE mode; report colors:");
  for (var deck = 1; deck <= 4; deck++) {
    for (var pad = 1; pad <= 8; pad++) {
      var index = (deck - 1) * 8 + (pad - 1);   // 0..31
      var val = 3 + index * 4;                   // 3,7,11,…,127 (the gaps + the top end)
      midi.sendShortMsg(PioneerDDJFLX10._HOTCUE_STATUS[deck], pad - 1, val);
      console.log("  deck" + deck + " pad" + pad + " -> value " + val +
                  " (0x" + (val < 16 ? "0" : "") + val.toString(16) + ")");
    }
  }
};

PioneerDDJFLX10._initHotcueLeds = function(){
  if (PioneerDDJFLX10._HOTCUE_PALETTE_TEST) {
    PioneerDDJFLX10._hotcuePaletteTest();
    return;   // skip normal hot-cue LED wiring while discovering the palette
  }
  for (var deck = 1; deck <= 4; deck++) {
    PioneerDDJFLX10._hotcue[deck] = {};
    (function(deck){
      var g = "[Channel" + deck + "]";
      for (var pad = 1; pad <= 8; pad++) {
        (function(pad){
          PioneerDDJFLX10._hotcue[deck][pad] =
              {on: false, loop: false, color: PioneerDDJFLX10._HOTCUE_DEFAULT_COLOR};
          var s = PioneerDDJFLX10._hotcue[deck][pad];
          var refresh = function(){
            s.on    = engine.getValue(g, "hotcue_" + pad + "_enabled") > 0;
            s.loop  = engine.getValue(g, "hotcue_" + pad + "_type") ===
                      PioneerDDJFLX10._HOTCUE_LOOP_TYPE;
            s.color = PioneerDDJFLX10._hotcueColorToPad(
                          engine.getValue(g, "hotcue_" + pad + "_color"));
            PioneerDDJFLX10._renderHotcue(deck, pad);
          };
          engine.makeConnection(g, "hotcue_" + pad + "_enabled", refresh);
          engine.makeConnection(g, "hotcue_" + pad + "_type",    refresh);
          engine.makeConnection(g, "hotcue_" + pad + "_color",   refresh);
          refresh();   // seed current state at init
        })(pad);
      }
    })(deck);
  }
  // Blink timer — re-renders only the loop hot cues (~1 Hz).
  if (!PioneerDDJFLX10._hotcueBlinkTimer) {
    PioneerDDJFLX10._hotcueBlinkTimer = engine.beginTimer(500, function(){
      PioneerDDJFLX10._hotcuePhase = !PioneerDDJFLX10._hotcuePhase;
      for (var d = 1; d <= 4; d++) {
        for (var p = 1; p <= 8; p++) {
          var s = PioneerDDJFLX10._hotcue[d][p];
          if (s.on && s.loop) { PioneerDDJFLX10._renderHotcue(d, p); }
        }
      }
    });
  }
};

// Tempo reset button management
PioneerDDJFLX10.rate_reset = function(channel, control, value, status, group) {
    if (value === 0x7F) {
        engine.setValue(group, "rate_set_default", 1);
        
        // Reset internal values
        var deck = PioneerDDJFLX10._getDeckFromGroup(group);
        PioneerDDJFLX10._rateMSB[deck] = 0x40; // Center value (0x40 = 64)
        PioneerDDJFLX10._rateLSB[deck] = 0x00;
    }
};

// Cue button management
PioneerDDJFLX10.cueButton = function(channel, control, value, status, group) {
    var pressed = (value === 0x7F);
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);

    if (pressed) {
        if (PioneerDDJFLX10.shiftActive) {
            // SHIFT + Cue : efface le point Cue
            engine.setValue(group, "cue_clear", 1);
        } else {
            // Cue standard : lecture tant que maintenu
            engine.setValue(group, "cue_default", 1);
        }
        PioneerDDJFLX10._cuePressed[deck] = true;
    } else if (!PioneerDDJFLX10.shiftActive) {
        // Relâchement : stop/retour au point cue
        engine.setValue(group, "cue_default", 0);
        PioneerDDJFLX10._cuePressed[deck] = false;
    } else {
        PioneerDDJFLX10._cuePressed[deck] = false;
    }
    PioneerDDJFLX10._updatePlayCueLEDs(group);
};

// Pilotage Play/Cue LEDs façon Pioneer (blink en pause sur cue)
PioneerDDJFLX10._sendPlayCue = function(deck, note, on) {
    var status = 0x90 + (deck - 1);
    midi.sendShortMsg(status, note, on ? 0x7F : 0x00);
};

PioneerDDJFLX10._updatePlayCueLEDs = function(group) {
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    var playing = engine.getValue(group, "play") > 0;
    var cueLit = engine.getValue(group, "cue_indicator") > 0;
    var cuePressed = PioneerDDJFLX10._cuePressed[deck];
    var hasTrack = engine.getValue(group, "track_samples") > 0;
    var atEnd = engine.getValue(group, "playposition") >= 0.999;

    // Modes :
    // - playing : Play ON, Cue OFF
    // - cuePressed : Cue ON fixe, Play OFF
    // - pause avec piste chargée (standby) : Play blink, Cue OFF
    // - fin de piste / pas de piste : tout OFF
    if (playing) {
        PioneerDDJFLX10._playCueShouldBlink[deck] = false;
        PioneerDDJFLX10._playShouldBlink[deck] = false;
        PioneerDDJFLX10._playCueBlinkState[deck] = false;
        PioneerDDJFLX10._sendPlayCue(deck, 0x0B, true);
        PioneerDDJFLX10._sendPlayCue(deck, 0x0C, false);
    } else if (cuePressed) {
        PioneerDDJFLX10._playCueShouldBlink[deck] = false;
        PioneerDDJFLX10._playShouldBlink[deck] = false;
        PioneerDDJFLX10._playCueBlinkState[deck] = false;
        PioneerDDJFLX10._sendPlayCue(deck, 0x0B, false);
        PioneerDDJFLX10._sendPlayCue(deck, 0x0C, true);
    } else if (hasTrack && !atEnd) {
        // Standby : Play blink, Cue off
        PioneerDDJFLX10._playCueShouldBlink[deck] = false;
        PioneerDDJFLX10._playShouldBlink[deck] = true;
    } else {
        PioneerDDJFLX10._playCueShouldBlink[deck] = false;
        PioneerDDJFLX10._playShouldBlink[deck] = false;
        PioneerDDJFLX10._playCueBlinkState[deck] = false;
        PioneerDDJFLX10._sendPlayCue(deck, 0x0B, false);
        PioneerDDJFLX10._sendPlayCue(deck, 0x0C, false);
    }
};

PioneerDDJFLX10._startPlayCueBlink = function(deck) {
    // Blink period ~ 500ms (2 Hz)
    PioneerDDJFLX10._playCueBlinkTimer[deck] = engine.beginTimer(500, function() {
        // Priorité au blink Play (standby). Si non, blink Cue (rarement utilisé ici).
        if (PioneerDDJFLX10._playShouldBlink[deck]) {
            PioneerDDJFLX10._playCueBlinkState[deck] = !PioneerDDJFLX10._playCueBlinkState[deck];
            PioneerDDJFLX10._sendPlayCue(deck, 0x0B, PioneerDDJFLX10._playCueBlinkState[deck]);
            PioneerDDJFLX10._sendPlayCue(deck, 0x0C, false);
            return;
        }
        if (PioneerDDJFLX10._playCueShouldBlink[deck]) {
            PioneerDDJFLX10._playCueBlinkState[deck] = !PioneerDDJFLX10._playCueBlinkState[deck];
            PioneerDDJFLX10._sendPlayCue(deck, 0x0B, false);
            PioneerDDJFLX10._sendPlayCue(deck, 0x0C, PioneerDDJFLX10._playCueBlinkState[deck]);
            return;
        }
        // Sinon: rien ne clignote
        PioneerDDJFLX10._playCueBlinkState[deck] = false;
        PioneerDDJFLX10._sendPlayCue(deck, 0x0B, false);
        PioneerDDJFLX10._sendPlayCue(deck, 0x0C, false);
    });
};

PioneerDDJFLX10._stopPlayCueBlink = function(deck) {
    if (PioneerDDJFLX10._playCueBlinkTimer[deck]) {
        engine.stopTimer(PioneerDDJFLX10._playCueBlinkTimer[deck]);
        PioneerDDJFLX10._playCueBlinkTimer[deck] = null;
    }
    PioneerDDJFLX10._playCueBlinkState[deck] = false;
    PioneerDDJFLX10._playCueShouldBlink[deck] = false;
};

// === Advanced Pioneer-like LED policy (Hotfix A) ===
PioneerDDJFLX10._adv = PioneerDDJFLX10._adv || {
  1:{play:0,cue:0,hold:false,phase:false},
  2:{play:0,cue:0,hold:false,phase:false},
  3:{play:0,cue:0,hold:false,phase:false},
  4:{play:0,cue:0,hold:false,phase:false},
};

PioneerDDJFLX10._noteStatus = PioneerDDJFLX10._noteStatus || (function(d){ return 0x90 + (d-1); });
PioneerDDJFLX10._sendLed = PioneerDDJFLX10._sendLed || function(d, note, on) {
    midi.sendShortMsg(PioneerDDJFLX10._noteStatus(d), note, on ? 0x7F : 0x00);
};

PioneerDDJFLX10._onCueHold = function(deck, pressed){
  PioneerDDJFLX10._adv[deck].hold = !!pressed;
  PioneerDDJFLX10._renderAdvanced(deck);
};

PioneerDDJFLX10._renderAdvanced = function(deck){
  var st = PioneerDDJFLX10._adv[deck];
  var playOn=0, cueOn=0;

  if (st.play) {
    playOn = 1;
    cueOn  = st.hold ? 1 : 0;
  } else {
    if (st.hold) {
      playOn = 0; cueOn = 1;
    } else if (st.cue) {
      playOn = st.phase ? 1 : 0;
      cueOn  = st.phase ? 0 : 1;
    } else {
      playOn = 0; cueOn = 0;
    }
  }

  PioneerDDJFLX10._sendLed(deck, 0x0B, playOn);
  PioneerDDJFLX10._sendLed(deck, 0x0C, cueOn);
};

PioneerDDJFLX10._connectAdvanced = function(deck){
  var g = "[Channel"+deck+"]";
  engine.makeConnection(g, "play_indicator", function(v){
    PioneerDDJFLX10._adv[deck].play = (v>=0.5)?1:0;
    PioneerDDJFLX10._renderAdvanced(deck);
  });
  engine.makeConnection(g, "cue_indicator", function(v){
    PioneerDDJFLX10._adv[deck].cue = (v>=0.5)?1:0;
    PioneerDDJFLX10._renderAdvanced(deck);
  });
};

if (!PioneerDDJFLX10._advTimer) {
  PioneerDDJFLX10._advTimer = engine.beginTimer(500, function(){
    [1,2,3,4].forEach(function(d){
      PioneerDDJFLX10._adv[d].phase = !PioneerDDJFLX10._adv[d].phase;
      PioneerDDJFLX10._renderAdvanced(d);
    });
  });
}

(function(){
  var _cue = PioneerDDJFLX10.cueButton;
  PioneerDDJFLX10.cueButton = function(ch, ctl, val, st, group){
    var m = /\[Channel(\d)\]/.exec(group||""); var deck = m ? parseInt(m[1],10) : 0;
    if (deck) PioneerDDJFLX10._onCueHold(deck, val>0);
    if (typeof _cue === "function") return _cue(ch, ctl, val, st, group);
  };
})();

PioneerDDJFLX10._initAdvancedLeds = PioneerDDJFLX10._initAdvancedLeds || function(){
  [1,2,3,4].forEach(PioneerDDJFLX10._connectAdvanced);
};

// ─────────────────────────────────────────────────────────────────────────────
// BeatFX Effect Scanner (diagnostic tool)
//
// Press SHIFT + BeatFX ON button to start the scan.
// Open Developer Tools → Log tab and watch the FX panel at the same time.
// Every 2.5 seconds a new effect loads and its index is printed to the log.
// Write down which effect name in the FX panel matches each index number.
// Press SHIFT + BeatFX ON again to stop early.
// ─────────────────────────────────────────────────────────────────────────────
PioneerDDJFLX10._scanIdx = 0;
PioneerDDJFLX10._scanTimer = null;

PioneerDDJFLX10._beatFxScanStart = function() {
    if (PioneerDDJFLX10._scanTimer) {
        engine.stopTimer(PioneerDDJFLX10._scanTimer);
        PioneerDDJFLX10._scanTimer = null;
        console.log("BeatFX Scan: stopped.");
        return;
    }
    var g = "[EffectRack1_EffectUnit1_Effect1]";
    for (var i = 0; i < 30; i++) { engine.setValue(g, "effect_selector", -1); }
    PioneerDDJFLX10._scanIdx = 0;
    console.log("BeatFX Scan: started — watch this Log tab + the FX panel");
    console.log("BeatFX Scan: Index 0");

    PioneerDDJFLX10._scanTimer = engine.beginTimer(2500, function() {
        PioneerDDJFLX10._scanIdx++;
        engine.setValue("[EffectRack1_EffectUnit1_Effect1]", "effect_selector", 1);
        console.log("BeatFX Scan: Index " + PioneerDDJFLX10._scanIdx);
        if (PioneerDDJFLX10._scanIdx >= 25) {
            engine.stopTimer(PioneerDDJFLX10._scanTimer);
            PioneerDDJFLX10._scanTimer = null;
            console.log("BeatFX Scan: complete.");
        }
    });
};

// Key Sync handler (protégé par SHIFT)
PioneerDDJFLX10.syncKeyHandler = function(channel, control, value, status, group) {
    if (value !== 0x7F) return;
    if (PioneerDDJFLX10.shiftActive) return; // ignore si SHIFT
    engine.setValue(group, "key_sync", 1);
};

// BeatFX ON/OFF — toggles the 3 effect slots on/off.
// SHIFT + ON/OFF triggers the effect index scan (prints to Log tab).
PioneerDDJFLX10.beatFxOnOff = function(channel, control, value, status, group) {
    // In non-Serato mode this button is a PULSE: a single press fires a Note ON
    // immediately followed by a Note OFF. Toggle on the press (Note ON) and
    // ignore the release (Note OFF), so one press = one on/off change instead of
    // flashing on then straight back off. (The 2.6/Serato mapping uses a
    // hold-to-activate variant because there the button is a true momentary.)
    if (value === 0) return;
    if (PioneerDDJFLX10.shiftActive) {
        PioneerDDJFLX10._beatFxScanStart();
        return;
    }
    PioneerDDJFLX10._beatFxActive = !PioneerDDJFLX10._beatFxActive;
    var on = PioneerDDJFLX10._beatFxActive;
    for (var slot = 1; slot <= 3; slot++) {
        engine.setValue("[EffectRack1_EffectUnit1_Effect" + slot + "]", "enabled", on ? 1 : 0);
        if (on) {
            engine.setValue("[EffectRack1_EffectUnit1_Effect" + slot + "]", "mix", 1.0);
        }
    }
};

// BeatFX channel routing — exclusive: selecting one group clears the others.
// NOTE: notes 0x10-0x16 on 0x93 may conflict with existing CH4 loop/transport
// mappings. Verify by pressing deck 4 Loop In and checking which MIDI note fires.
PioneerDDJFLX10._beatFxRouteGroups = ["[Channel1]", "[Channel2]", "[Channel3]", "[Channel4]", "[Master]", "[Microphone]"];

PioneerDDJFLX10._beatFxNoteToGroup = {
    0x10: "[Channel1]",
    0x11: "[Channel2]",
    0x12: "[Channel3]",
    0x13: "[Channel4]",
    0x14: "[Master]",
    0x15: "[Microphone]",
    0x16: null,          // SP — no direct Mixxx group equivalent
};

PioneerDDJFLX10.beatFxChannelSelect = function(channel, control, value, status, group) {
    if (value === 0) return;
    var target = PioneerDDJFLX10._beatFxNoteToGroup[control];
    var groups = PioneerDDJFLX10._beatFxRouteGroups;
    for (var i = 0; i < groups.length; i++) {
        engine.setValue("[EffectRack1_EffectUnit1]", "group_" + groups[i] + "_enable", 0);
    }
    if (target) {
        engine.setValue("[EffectRack1_EffectUnit1]", "group_" + target + "_enable", 1);
    }
};

// Crossfader assign — A (left), THRU (center), B (right) per channel.
// orientation: 0=A, 1=THRU, 2=B
PioneerDDJFLX10._crossfaderNoteToOrientation = {
    0x16: 0,  // A
    0x1D: 1,  // THRU
    0x18: 2   // B
};

PioneerDDJFLX10.crossfaderAssign = function(channel, control, value, status, group) {
    if (value === 0) return;
    var orientation = PioneerDDJFLX10._crossfaderNoteToOrientation[control];
    if (orientation !== undefined) {
        engine.setValue(group, "orientation", orientation);
    }
};

// Sound Color FX — exclusive toggle across all 4 channels via QuickEffect.
// Preset indices match the "Quick Effect Chain Presets" list order in Mixxx settings.
PioneerDDJFLX10._colorFxActive = -1;
PioneerDDJFLX10._colorFxChannels = ["[Channel1]", "[Channel2]", "[Channel3]", "[Channel4]"];
PioneerDDJFLX10._colorFxNoteToPreset = {
    0: 17,  // Space   → Reverb
    1: 10,  // D. Echo → Echo
    2: 8,   // Crush   → Bitcrusher
    3: 16,  // Pitch   → Pitch Shift
    4: 19,  // Noise   → White Noise
    5: 11   // Filter  → Filter
};

PioneerDDJFLX10.soundColorFx = function(channel, control, value, status, group) {
    var channels = PioneerDDJFLX10._colorFxChannels;
    var i;
    // One Sound Color FX selector shared by all decks. Radio-style latching
    // buttons: Note ON = preset selected, Note OFF = preset deselected. Switching
    // A→B sends A's Note OFF then B's Note ON, so the Note-OFF handler only acts
    // when that preset is STILL the active one — otherwise A's off would wipe the
    // B load that just happened. A lone Note OFF (deselect) loads the empty
    // '----' preset (index 0 = "no effect") on all decks, so the FX shows nothing.
    var on = ((status & 0xF0) === 0x90) && (value > 0);
    if (!on) {
        if (PioneerDDJFLX10._colorFxActive === control) {
            for (i = 0; i < channels.length; i++) {
                engine.setValue("[QuickEffectRack1_" + channels[i] + "]", "loaded_chain_preset", 0);
            }
            PioneerDDJFLX10._colorFxActive = -1;
        }
    } else {
        var preset = PioneerDDJFLX10._colorFxNoteToPreset[control];
        for (i = 0; i < channels.length; i++) {
            engine.setValue("[QuickEffectRack1_" + channels[i] + "]", "loaded_chain_preset", preset);
            engine.setValue("[QuickEffectRack1_" + channels[i] + "_Effect1]", "enabled", 1);
            // Block each channel's knob immediately with a sentinel that no physical
            // position can match, preventing any jump before the preset settles.
            PioneerDDJFLX10._colorFxSoftTakeover[i + 1] = true;
            PioneerDDJFLX10._colorFxSoftTakeoverTarget[i + 1] = -1;
        }
        // After 50 ms read each channel's actual settled super1 as the takeover target.
        var chSnapshot = channels.slice();
        engine.beginTimer(50, function() {
            for (var j = 0; j < chSnapshot.length; j++) {
                PioneerDDJFLX10._colorFxSoftTakeoverTarget[j + 1] = engine.getValue(
                    "[QuickEffectRack1_" + chSnapshot[j] + "]", "super1"
                );
            }
        }, true);
        PioneerDDJFLX10._colorFxActive = control;
    }
};

// Sound Color FX intensity knob — 14-bit absolute encoder, per-channel state.
// CC 0x17 (MSB) arrives first; CC 0x37 (LSB) completes the pair and drives the write.
// Keyed by deck number (1–4).
PioneerDDJFLX10._colorFxKnobMSB      = {1: 63,   2: 63,   3: 63,   4: 63};
PioneerDDJFLX10._colorFxKnobPosition = {1: 0.5,  2: 0.5,  3: 0.5,  4: 0.5};
PioneerDDJFLX10._colorFxSoftTakeover = {1: false, 2: false, 3: false, 4: false};
PioneerDDJFLX10._colorFxSoftTakeoverTarget = {1: -1, 2: -1, 3: -1, 4: -1};

PioneerDDJFLX10.soundColorFxKnobMSB = function(channel, control, value, status, group) {
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    PioneerDDJFLX10._colorFxKnobMSB[deck] = value;
};

PioneerDDJFLX10.soundColorFxKnobLSB = function(channel, control, value, status, group) {
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    var position = (PioneerDDJFLX10._colorFxKnobMSB[deck] * 128 + value) / 16383.0;
    var prev = PioneerDDJFLX10._colorFxKnobPosition[deck];
    PioneerDDJFLX10._colorFxKnobPosition[deck] = position;

    if (PioneerDDJFLX10._colorFxSoftTakeover[deck]) {
        var target = PioneerDDJFLX10._colorFxSoftTakeoverTarget[deck];
        if ((prev <= target && position >= target) || (prev >= target && position <= target)) {
            PioneerDDJFLX10._colorFxSoftTakeover[deck] = false;
        } else {
            return;
        }
    }

    engine.setValue("[QuickEffectRack1_[Channel" + deck + "]]", "super1", position);
};

// Loop In / Halve and Loop Out / Double.
// If a loop is already active: halve (In button) or double (Out button).
// If no loop is active: set loop_in or loop_out as normal.
PioneerDDJFLX10.loopInOrHalve = function(channel, control, value, status, group) {
    if (value === 0) return;
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    if (PioneerDDJFLX10._loopAdjustMode[deck]) {
        PioneerDDJFLX10._loopAdjustMode[deck] = null;
        PioneerDDJFLX10._renderLoopLeds(deck);
        return;
    }
    var key = engine.getValue(group, "loop_enabled") ? "loop_halve" : "loop_in";
    engine.setValue(group, key, 1);
    engine.setValue(group, key, 0);
};

PioneerDDJFLX10.loopOutOrDouble = function(channel, control, value, status, group) {
    if (value === 0) return;
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    if (PioneerDDJFLX10._loopAdjustMode[deck]) {
        PioneerDDJFLX10._loopAdjustMode[deck] = null;
        PioneerDDJFLX10._renderLoopLeds(deck);
        return;
    }
    var key = engine.getValue(group, "loop_enabled") ? "loop_double" : "loop_out";
    engine.setValue(group, key, 1);
    engine.setValue(group, key, 0);
};

// Loop point adjust mode — entered via Shift+LoopIn (0x4C) or Shift+LoopOut (0x4D).
// While active, the jog wheel moves the selected loop point by seeking the playhead
// and re-stamping loop_in / loop_out at the new position. Press again to exit.
PioneerDDJFLX10.loopInAdjust = function(channel, control, value, status, group) {
    if (value === 0) return;
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    PioneerDDJFLX10._loopAdjustMode[deck] = PioneerDDJFLX10._loopAdjustMode[deck] === "in" ? null : "in";
    PioneerDDJFLX10._renderLoopLeds(deck);
};

PioneerDDJFLX10.loopOutAdjust = function(channel, control, value, status, group) {
    if (value === 0) return;
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    // With no active loop this button reloops the last stored loop (matches
    // Serato's out-adjust button). While looping it toggles jog out-adjust mode.
    if (engine.getValue(group, "loop_enabled") <= 0) {
        engine.setValue(group, "reloop_toggle", 1);
        engine.setValue(group, "reloop_toggle", 0);
        return;
    }
    PioneerDDJFLX10._loopAdjustMode[deck] = PioneerDDJFLX10._loopAdjustMode[deck] === "out" ? null : "out";
    PioneerDDJFLX10._renderLoopLeds(deck);
};

// ─── Sampler pad LEDs ────────────────────────────────────────────────────────
// Deck-1 pad channel (status 0x97), notes 0x30-0x37 = Sampler1-8 in sampler pad
// mode. Lit when a sampler has a track loaded; blinks while it is playing.
PioneerDDJFLX10._SAMPLER_STATUS = 0x97;
PioneerDDJFLX10._SAMPLER_BASE_NOTE = 0x30;
PioneerDDJFLX10._SAMPLER_COUNT = 8;
PioneerDDJFLX10._samplerBlinkPhase = false;
var SAMPLER_BLINK_MS = 300;

PioneerDDJFLX10._renderSamplerLed = function(n) {   // n = 1..8
    var group = "[Sampler" + n + "]";
    var loaded = engine.getValue(group, "track_loaded") > 0;
    var playing = engine.getValue(group, "play") > 0;
    var val;
    if (!loaded)      { val = 0x00; }                                             // empty → off
    else if (playing) { val = PioneerDDJFLX10._samplerBlinkPhase ? 0x7F : 0x00; } // playing → blink
    else              { val = 0x7F; }                                             // loaded → solid
    midi.sendShortMsg(PioneerDDJFLX10._SAMPLER_STATUS,
                      PioneerDDJFLX10._SAMPLER_BASE_NOTE + (n - 1), val);
};

PioneerDDJFLX10._samplerIsBlinking = function(n) {
    var group = "[Sampler" + n + "]";
    return engine.getValue(group, "track_loaded") > 0 && engine.getValue(group, "play") > 0;
};

PioneerDDJFLX10._initSamplerLeds = function() {
    for (var n = 1; n <= PioneerDDJFLX10._SAMPLER_COUNT; n++) {
        (function(s){
            var g = "[Sampler" + s + "]";
            var refresh = function(){ PioneerDDJFLX10._renderSamplerLed(s); };
            engine.makeConnection(g, "track_loaded", refresh);
            engine.makeConnection(g, "play",         refresh);
            refresh();   // seed current state
        })(n);
    }
    if (!PioneerDDJFLX10._samplerBlinkTimer) {
        PioneerDDJFLX10._samplerBlinkTimer = engine.beginTimer(SAMPLER_BLINK_MS, function(){
            PioneerDDJFLX10._samplerBlinkPhase = !PioneerDDJFLX10._samplerBlinkPhase;
            for (var s = 1; s <= PioneerDDJFLX10._SAMPLER_COUNT; s++) {
                if (PioneerDDJFLX10._samplerIsBlinking(s)) {
                    PioneerDDJFLX10._renderSamplerLed(s);
                }
            }
        });
    }
};

// ─── EQ kill switches (repurposed STEM buttons) ─────────────────────────────
// The FLX10's per-deck STEM buttons (Drums/Vocal/Inst — notes 0x0D/0x0E/0x0F)
// are useless without stem separation, so map them to LOW / MID / HIGH EQ kills.
// Default = band ON (LED lit). Press toggles: kill that band (LED off) / restore.
// Kills live on [EqualizerRack1_[ChannelN]_Effect1] button_parameterN
// (1 = Low, 2 = Mid, 3 = High); value 1 = killed.
PioneerDDJFLX10._EQ_KILL = [
    {note: 0x0D, param: "button_parameter1"},  // Low
    {note: 0x0E, param: "button_parameter2"},  // Mid
    {note: 0x0F, param: "button_parameter3"}   // High
];
PioneerDDJFLX10._eqGroup = function(deck) {
    return "[EqualizerRack1_[Channel" + deck + "]_Effect1]";
};
PioneerDDJFLX10._renderEqKillLed = function(deck, idx) {
    var killed = engine.getValue(PioneerDDJFLX10._eqGroup(deck),
                                 PioneerDDJFLX10._EQ_KILL[idx].param) > 0;
    midi.sendShortMsg(0x90 + (deck - 1), PioneerDDJFLX10._EQ_KILL[idx].note,
                      killed ? 0x00 : 0x7F);   // lit when NOT killed
};
PioneerDDJFLX10.eqKill = function(channel, control, value, status, group) {
    if (value === 0) return;   // act on press only
    var deck = PioneerDDJFLX10._getDeckFromGroup(group);
    for (var i = 0; i < PioneerDDJFLX10._EQ_KILL.length; i++) {
        if (PioneerDDJFLX10._EQ_KILL[i].note === control) {
            var eq = PioneerDDJFLX10._eqGroup(deck);
            var p  = PioneerDDJFLX10._EQ_KILL[i].param;
            engine.setValue(eq, p, engine.getValue(eq, p) > 0 ? 0 : 1);  // toggle kill
            return;
        }
    }
};
PioneerDDJFLX10._initEqKillLeds = function() {
    for (var deck = 1; deck <= 4; deck++) {
        (function(d){
            var eq = PioneerDDJFLX10._eqGroup(d);
            for (var i = 0; i < PioneerDDJFLX10._EQ_KILL.length; i++) {
                (function(idx){
                    engine.makeConnection(eq, PioneerDDJFLX10._EQ_KILL[idx].param, function(){
                        PioneerDDJFLX10._renderEqKillLed(d, idx);
                    });
                    PioneerDDJFLX10._renderEqKillLed(d, idx);   // seed current LED state
                })(i);
            }
        })(deck);
    }
};

// ─── Master EQ kill (applies to ALL channels) ───────────────────────────────
// The master section's STEM buttons (notes 0x1B/0x1C/0x1D on status 0x96) have
// no stem use; map them to LOW/MID/HIGH kills across ALL 4 decks at once (Mixxx
// has no master EQ kill). Default = lit (bands on). Press kills the band on every
// deck; press again restores it everywhere. LED stays lit unless killed on all 4.
PioneerDDJFLX10._MASTER_EQ_STATUS = 0x96;
PioneerDDJFLX10._MASTER_EQ_KILL = [
    {note: 0x1B, param: "button_parameter2"},  // 27 = Vocal → Mid
    {note: 0x1C, param: "button_parameter1"},  // 28 = Drums → Low
    {note: 0x1D, param: "button_parameter3"}   // 29 = Inst  → High
];
PioneerDDJFLX10._masterBandAllKilled = function(param) {
    for (var d = 1; d <= 4; d++) {
        if (engine.getValue(PioneerDDJFLX10._eqGroup(d), param) <= 0) {
            return false;
        }
    }
    return true;
};
PioneerDDJFLX10._renderMasterEqKillLed = function(idx) {
    var allKilled = PioneerDDJFLX10._masterBandAllKilled(
        PioneerDDJFLX10._MASTER_EQ_KILL[idx].param);
    midi.sendShortMsg(PioneerDDJFLX10._MASTER_EQ_STATUS,
                      PioneerDDJFLX10._MASTER_EQ_KILL[idx].note,
                      allKilled ? 0x00 : 0x7F);   // lit unless killed on all decks
};
PioneerDDJFLX10.masterEqKill = function(channel, control, value, status, group) {
    // Latching button: Note ON (status 0x9n, vel>0) = lit = band ON; Note OFF
    // (status 0x8n, or vel 0) = unlit = band KILLED. Set the kill to match the
    // button's reported state (don't toggle) so one press = one change.
    var lit = ((status & 0xF0) === 0x90) && (value > 0);
    for (var i = 0; i < PioneerDDJFLX10._MASTER_EQ_KILL.length; i++) {
        if (PioneerDDJFLX10._MASTER_EQ_KILL[i].note === control) {
            var p = PioneerDDJFLX10._MASTER_EQ_KILL[i].param;
            for (var d = 1; d <= 4; d++) {
                engine.setValue(PioneerDDJFLX10._eqGroup(d), p, lit ? 0 : 1);
            }
            return;   // master + per-deck LEDs update via their connections
        }
    }
};
PioneerDDJFLX10._initMasterEqKillLeds = function() {
    for (var i = 0; i < PioneerDDJFLX10._MASTER_EQ_KILL.length; i++) {
        (function(idx){
            for (var d = 1; d <= 4; d++) {
                engine.makeConnection(PioneerDDJFLX10._eqGroup(d),
                    PioneerDDJFLX10._MASTER_EQ_KILL[idx].param, function(){
                        PioneerDDJFLX10._renderMasterEqKillLed(idx);
                    });
            }
            PioneerDDJFLX10._renderMasterEqKillLed(idx);   // seed current LED state
        })(i);
    }
};

// 4-beat loop button (note 0x14) — anchors the loop at the PRESS point: creates
// a 4-beat loop ENDING where you press (covering the previous 4 beats), instead
// of the default beatloop that extends forward. Press again while looping to exit.
PioneerDDJFLX10.beatLoopBack4 = function(channel, control, value, status, group) {
    if (value === 0) return;
    if (engine.getValue(group, "loop_enabled") > 0) {
        engine.setValue(group, "reloop_toggle", 1);   // already looping → exit
        engine.setValue(group, "reloop_toggle", 0);
        return;
    }
    // Anchor the beatloop at its END so the press point becomes the loop end and
    // the loop covers the previous 4 beats. loop_anchor: 0 = Start, 1 = End.
    // (Pure loop control — does NOT use beatjump and does not move the playhead.)
    engine.setValue(group, "loop_anchor", 1);
    engine.setValue(group, "beatloop_4_activate", 1);
    engine.setValue(group, "loop_anchor", 0);   // restore default for other loops
};

// ─── Loop in/out button LEDs ─────────────────────────────────────────────────
// IN button = note 0x10, OUT button = note 0x11 (status 0x90+(deck-1)). States:
//   • adjusting IN  (_loopAdjustMode "in")  → IN blink, OUT solid
//   • adjusting OUT (_loopAdjustMode "out") → IN solid, OUT blink
//   • loop active (loop_enabled)            → IN solid, OUT solid
//   • loop in set but not yet closed        → IN blink, OUT off
//   • otherwise                             → both off
PioneerDDJFLX10._LOOP_IN_NOTE  = 0x10;
PioneerDDJFLX10._LOOP_OUT_NOTE = 0x11;
PioneerDDJFLX10._loopBlinkPhase = false;
var LOOP_BLINK_MS = 300;

PioneerDDJFLX10._renderLoopLeds = function(deck) {
    var status = 0x90 + (deck - 1);
    var group  = "[Channel" + deck + "]";
    var adjust = PioneerDDJFLX10._loopAdjustMode[deck];
    var active = engine.getValue(group, "loop_enabled") > 0;
    var inSet  = engine.getValue(group, "loop_start_position") >= 0;
    var outSet = engine.getValue(group, "loop_end_position") >= 0;
    var blink  = PioneerDDJFLX10._loopBlinkPhase ? 0x7F : 0x00;
    var inVal, outVal;
    if (adjust === "in")       { inVal = blink; outVal = 0x7F; }
    else if (adjust === "out") { inVal = 0x7F;  outVal = blink; }
    else if (active)           { inVal = 0x7F;  outVal = 0x7F; }
    else if (inSet && !outSet) { inVal = blink; outVal = 0x00; }  // started, not closed
    else                       { inVal = 0x00;  outVal = 0x00; }
    midi.sendShortMsg(status, PioneerDDJFLX10._LOOP_IN_NOTE,  inVal);
    midi.sendShortMsg(status, PioneerDDJFLX10._LOOP_OUT_NOTE, outVal);
};

// True when deck's loop LEDs are in a blinking state, so the blink timer knows
// to re-render it.
PioneerDDJFLX10._loopDeckBlinks = function(deck) {
    var adjust = PioneerDDJFLX10._loopAdjustMode[deck];
    if (adjust === "in" || adjust === "out") { return true; }
    var group  = "[Channel" + deck + "]";
    var active = engine.getValue(group, "loop_enabled") > 0;
    var inSet  = engine.getValue(group, "loop_start_position") >= 0;
    var outSet = engine.getValue(group, "loop_end_position") >= 0;
    return (!active && inSet && !outSet);
};

PioneerDDJFLX10._initLoopLeds = function() {
    for (var deck = 1; deck <= 4; deck++) {
        (function(d){
            var g = "[Channel" + d + "]";
            var refresh = function(){ PioneerDDJFLX10._renderLoopLeds(d); };
            engine.makeConnection(g, "loop_enabled",        refresh);
            engine.makeConnection(g, "loop_start_position", refresh);
            engine.makeConnection(g, "loop_end_position",   refresh);
            refresh();   // seed current state
        })(deck);
    }
    if (!PioneerDDJFLX10._loopBlinkTimer) {
        PioneerDDJFLX10._loopBlinkTimer = engine.beginTimer(LOOP_BLINK_MS, function(){
            PioneerDDJFLX10._loopBlinkPhase = !PioneerDDJFLX10._loopBlinkPhase;
            for (var d = 1; d <= 4; d++) {
                if (PioneerDDJFLX10._loopDeckBlinks(d)) {
                    PioneerDDJFLX10._renderLoopLeds(d);
                }
            }
        });
    }
};

// Function to expand playlists folder (currently disabled)
// This feature requires a different approach that will be implemented later
