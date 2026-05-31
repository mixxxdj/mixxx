// -------------------------------------------------------------------
// ------------------- DDJ-FLX10 script file v.1.0.5 -------------------
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
// Added Beatgrid nudge (Shift + Jog) - Veezuhz
// Added Waveform zoom (Shift + CH rotary) - Veezuhz
// Added Memory cue navigation (Shift + Hot Cue pads) - Veezuhz
// Added Beat Jump and Beat Jump Size controls (Shift + CUE/LOOP CALL arrows) - Veezuhz
// Added Sound Color FX controls (Shift + Pad FX1/2) - Veezuhz
// Added Crossfader assign (A/THRU/B) per channel (Shift + Channel Fader) - Veezuhz
// Updated LED policy for Play/Cue to be more Pioneer-like (Play ON when playing, Cue ON when paused on cue, both OFF at end of track or no track, Play blinks in standby) - Veezuhz
// Added a diagnostic tool to scan BeatFX effects and print their index in the log (Shift + BeatFX ON) - Veezuhz
// Fixed loop in/out but need to fine tune sensitivity (Shift + Loop In/Out). Half && Double work - Veezuhz
//-----------will try to log updates moving forward by date and time for better tracking-------------------------
// 5-22.945: Added tempo reset button management - Veezuhz
// 5-22.945: Added time display mode management (elapsed vs remaining)- (shift+hot cue)  - Veezuhz
// 5-22.945: Updated connectControl to makeConnection for better performance on high-frequency callbacks - Veezuhz
// 5-22.945: Updated VuMeter to vu_meter

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
// ─────────────────────────────────────────────────────────────────────────────

// Global variables
var PioneerDDJFLX10 = {};

// Global state
PioneerDDJFLX10.shiftActive = false;
// Seed to CENTER (0x40 MSB / 0 LSB ≈ rate 0) so _sliderRate() returns ~0 before
// the fader has been physically moved — otherwise a 0/0 reading computes +1 (max)
// and releasing TEMPO RESET before touching the fader would jump tempo to max.
PioneerDDJFLX10._rateMSB = {1: 0x40, 2: 0x40, 3: 0x40, 4: 0x40};
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

// ===== HID screen SysEx handshake ===========================================
// These two SysEx messages on MIDI OUT are the gate that unlocks the FLX10's
// HID screen rendering. Without them, our screen.js writes to interface 5
// are silently rejected by the firmware. Decoded from the Serato capture
// (2026-05-23). See FLX10-SCREEN-PROTOCOL-FINDINGS.md for the full story.
PioneerDDJFLX10._SYSEX_ENTER_HID = [
    0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
    0x00, 0x03, 0x01, 0xF7
];
PioneerDDJFLX10._SYSEX_KEEPALIVE = [
    0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
    0x00, 0x50, 0x31, 0xF7
];
// Per-deck init SysEx — Serato sends these after handshake, at track-load
// time. Hypothesis: one of them puts the firmware into "Serato mode" where
// time is computed internally (from BPM + position) instead of being driven
// by the HID xx 27 [9..12] bytes, decoupling wave-scroll from time-display.
// Decoded 2026-05-24 from flx10-driverrutil-then-serato.pcapng.
//
// Pattern: F0 00 40 05 00 00 04 01 <deck-id> 00 00 <state-bytes> F7
//   deck-id: 0x11 deck1, 0x12 deck2, 0x13 deck3, 0x14 deck4
//   state-bytes: 02 0e 0e 05 initially; later replaced with track-specific bytes
PioneerDDJFLX10._SYSEX_DECK_INIT = {
    1: [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
        0x00, 0x11, 0x00, 0x00, 0x02, 0x0e, 0x0e, 0x05, 0xF7],
    2: [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
        0x00, 0x12, 0x00, 0x00, 0x02, 0x0e, 0x0e, 0x05, 0xF7],
    3: [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
        0x00, 0x13, 0x00, 0x00, 0x02, 0x0e, 0x0e, 0x05, 0xF7],
    4: [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
        0x00, 0x14, 0x00, 0x00, 0x02, 0x0e, 0x0e, 0x05, 0xF7],
};
// Two more globals Serato sends at the same time. Not yet sure what they
// do — try them together with the per-deck init.
PioneerDDJFLX10._SYSEX_GLOBAL_B = [
    0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
    0x00, 0x0b, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7
];
PioneerDDJFLX10._SYSEX_GLOBAL_C = [
    0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
    0x00, 0x0c, 0x00, 0x00, 0x02, 0x0e, 0x0e, 0x05, 0x00, 0x01, 0xF7
];
// Outer session-start SysEx — uses AlphaTheta's '00 20 7F' vendor ID
// (different from the '00 40 05' ID our other SysEx use). Serato sends
// this ONCE at app launch, before any other SysEx. Captured in
// flx10-driverrutil-then-serato.pcapng at t=25.242 (BEFORE the ENTER_HID
// handshake at t=26.482). Likely the firmware-mode-enable signal that
// makes the firmware honor xx 27 [9..12] time bytes even when [5..7]
// position bytes are non-zero. Experimental — added 2026-05-24.
PioneerDDJFLX10._SYSEX_SESSION_START = [
    0xF0, 0x00, 0x20, 0x7F, 0x01, 0x02, 0x01, 0x01,
    0x22, 0x0F, 0x0C, 0x06, 0x08, 0x04, 0x0A, 0x02,
    0x02, 0x05, 0x00, 0x00, 0x0E, 0x0A, 0x0E, 0x03,
    0x04, 0xF7
];
// ===== Rekordbox-mode SysEx (captured 2026-05-24 from flx10-rekordbox-opening.pcapng).
// Phase 1A pivot: rekordbox uses a DIFFERENT firmware mode (xx 21 deck state
// instead of Serato xx 27) which gives ACCURATE TIME + scrolling waveform
// (confirmed by user). Pivoting our screen.js to use this mode.
// Set PioneerDDJFLX10._SCREEN_MODE = 'rekordbox' to send these instead of
// the Serato sequence at init.
// 'serato' | 'rekordbox' | 'vdj'. Default 'serato' = working baseline.
// 2026-05-24: tried pivots to rekordbox AND vdj protocols. Both failed
// with the SAME wall — firmware exposes only 2 of 5 display modes despite
// us sending every visible byte from packet captures (vendor unlock,
// SysEx init, xx 21 deck state, waveform placeholders). The Pioneer
// Driver Setting Utility runs continuously on Windows during real use,
// and there's likely a kernel-level / firmware-level handshake we
// can't see in USBPcap captures. All 3 protocol implementations are
// kept in code (gated by this flag) for future use if the wall ever
// gets cracked.
PioneerDDJFLX10._SCREEN_MODE = 'serato';
// 1) Variant keepalive (50 00 instead of Serato's 50 31).
PioneerDDJFLX10._SYSEX_RKBOX_KEEPALIVE = [
    0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
    0x00, 0x50, 0x00, 0xF7
];
// 2..5) Per-deck init (six 0x0F bytes, vs Serato's 02 0E 0E 05).
PioneerDDJFLX10._SYSEX_RKBOX_DECK_INIT = {
    1: [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
        0x00, 0x11, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF7],
    2: [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
        0x00, 0x12, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF7],
    3: [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
        0x00, 0x13, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF7],
    4: [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
        0x00, 0x14, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0xF7],
};
// 6) Same Global B as Serato.
PioneerDDJFLX10._SYSEX_RKBOX_GLOBAL_B = PioneerDDJFLX10._SYSEX_GLOBAL_B;
// 7) Variant Global C (ends 00 00 00 instead of Serato's 05 00 01).
PioneerDDJFLX10._SYSEX_RKBOX_GLOBAL_C = [
    0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
    0x00, 0x00, 0x0C, 0x00, 0x00, 0x02, 0x0E, 0x0E, 0x00, 0x00, 0x00, 0xF7
];
// 8) The long rekordbox-mode config — verbatim from capture. Bytes after
// the standard 00 00 0A prefix appear to encode display/mode settings.
// Same bytes sent twice at session start in the capture.
PioneerDDJFLX10._SYSEX_RKBOX_MODE_CONFIG = [
    0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
    0x00, 0x00, 0x0A, 0x00, 0x28, 0x00, 0x26, 0x00,
    0x18, 0x3D, 0x3A, 0x05, 0x64, 0x50, 0x2A, 0x54,
    0x40, 0x14, 0x1A, 0x04, 0x69, 0x13,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7
];
// 9) ENTER_HID — same as Serato.

// ===== VirtualDJ-mode SysEx (captured 2026-05-24 from flx10-vdj-init.pcapng).
// VDJ uses xx 21 deck state (same as rekordbox) but with DIFFERENT play
// value (04 = playing instead of rekordbox's 00) and a DIFFERENT long
// mode-config SysEx. VDJ also sends fewer SysEx (7) than rekordbox (9),
// no per-deck 00 11/12/13/14 commands, and all in 0.3s (no 20s pause).
PioneerDDJFLX10._SYSEX_VDJ_INIT = [
    // 1. ENTER_HID
    [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01, 0x00, 0x03, 0x01, 0xF7],
    // 2. Keepalive variant
    [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01, 0x00, 0x50, 0x00, 0xF7],
    // 3. Global C v1
    [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
     0x00, 0x00, 0x0C, 0x00, 0x00, 0x02, 0x0E, 0x0E, 0x00, 0x00, 0x00, 0xF7],
    // 4. Global B empty
    [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
     0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7],
    // 5. Global B with data
    [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
     0x00, 0x00, 0x0B, 0x31, 0x5E, 0x50, 0x01, 0x00, 0x00, 0xF7],
    // 6. LONG mode-config (VDJ-specific bytes; differs from rekordbox).
    //    This is hypothesized to be the firmware "enter VDJ mode" command.
    [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
     0x00, 0x00, 0x0A, 0x00, 0x28, 0x00, 0x26, 0x00,
     0x1A, 0x51, 0x02, 0x42, 0x28, 0x11, 0x26, 0x41,
     0x32, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0xF7],
    // 7. Global C v2 (different ending bytes)
    [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
     0x00, 0x00, 0x0C, 0x00, 0x00, 0x02, 0x09, 0x04, 0x00, 0x00, 0x00, 0xF7],
];
// Same keepalive used for the 200ms timer.
PioneerDDJFLX10._SYSEX_VDJ_KEEPALIVE = PioneerDDJFLX10._SYSEX_VDJ_INIT[1];

// Per-deck beat-grid burst: verbatim copy of the 7 02 0F XX YY messages
// Serato sent for deck 1 at track-load in flx10-serato-smoothplaying.pcapng
// (t=11.762..11.877, 7 messages in 115ms). XX correlates with BPM (climbs
// as BPM climbs); YY appears to be a fine adjustment or jitter. We don't
// yet understand the precise encoding, but sending Serato's actual bytes
// is the minimum-effort test of whether these messages affect firmware
// time-computation behavior. Same payload for all 4 decks (we just swap
// the deck-id byte at offset [9]: 0x11/0x12/0x13/0x14).
PioneerDDJFLX10._SYSEX_BEATGRID_BURST_PAYLOAD = [
    // [XX, YY] pairs captured from Serato deck-1 track-load
    [0x0a, 0x09],
    [0x0b, 0x0a],
    [0x0c, 0x06],
    [0x0c, 0x0c],
    [0x0d, 0x00],
    [0x0d, 0x05],
    [0x0d, 0x0b],
];
PioneerDDJFLX10._buildBeatgridSysex = function(deck, xx, yy) {
    var deckId = 0x10 + deck;   // 0x11 deck1, 0x12 deck2, 0x13 deck3, 0x14 deck4
    return [0xF0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x04, 0x01,
            0x00, deckId, 0x00, 0x00, 0x02, 0x0F, xx, yy, 0xF7];
};
PioneerDDJFLX10._sendBeatgridBurst = function(deck) {
    var pairs = PioneerDDJFLX10._SYSEX_BEATGRID_BURST_PAYLOAD;
    for (var i = 0; i < pairs.length; i++) {
        var msg = PioneerDDJFLX10._buildBeatgridSysex(deck, pairs[i][0], pairs[i][1]);
        midi.sendSysexMsg(msg, msg.length);
    }
};
PioneerDDJFLX10._sysexKeepaliveTimer = null;

// Initialization
PioneerDDJFLX10.init = function(id) {
    console.log("Pioneer DDJ-FLX10 PROD - Initialisation");

    var initTimeMode = engine.getValue("[Controls]", "ShowDurationRemaining") !== 0 ? 0x7F : 0x00;
    // 2026-05-29 — MIDI jog-display path is OFF by default. The firmware fuses
    // the MIDI jog fields (time mode + MIN/SEC, position marker, speed) with the
    // HID xx27 screen on the SAME display elements. The MIDI time element, once
    // enabled by the time-mode note, renders the firmware default in remaining
    // mode (= the FULL TRACK DURATION) and bled through ~once per second as the
    // "duration flashes to the beginning every second" artifact — persisting
    // even with the daemon off and even after feeding jogTime live values,
    // because the HID↔MIDI fusion itself is the cause. HID xx27 (screen.js) is
    // the sole authoritative source for time/duration/BPM/position. Flip to
    // true to restore the legacy MIDI jog-display path.
    PioneerDDJFLX10._MIDI_JOG_DISPLAY = false;
    // Plage de pitch par défaut (±16%)
    for (var i = 1; i <= 4; i++) {
        var grp = "[Channel" + i + "]";
        engine.setValue(grp, "rateRange", 0.16);
        // Connect VU meters
        engine.makeConnection(grp, "vu_meter", PioneerDDJFLX10["LedVuMeterCH" + i]);
        // Force off au démarrage pour éviter LEDs fantômes
        midi.sendShortMsg(0xB0 + (i - 1), 0x02, 0x00);
        // Clear sync-button LEDs at init — the FLX10 powers up with these lit,
        // and Mixxx doesn't proactively send the off state. BEAT SYNC = note
        // 0x58, KEY SYNC = note 0x65, on the deck's channel (0x90+(i-1)).
        midi.sendShortMsg(0x90 + (i - 1), 0x58, 0x00);   // beat sync off
        midi.sendShortMsg(0x90 + (i - 1), 0x65, 0x00);   // key sync off
        midi.sendShortMsg(0x90 + (i - 1), 0x41, 0x00);   // tempo reset off
        // Duration cache (no MIDI — used by HID path and, if re-enabled, jogTime).
        PioneerDDJFLX10._trackDuration[i] = engine.getValue(grp, "duration");
        // Legacy MIDI jog-display path — OFF by default (see _MIDI_JOG_DISPLAY).
        if (PioneerDDJFLX10._MIDI_JOG_DISPLAY) {
            midi.sendShortMsg(JOG_DISPLAY_NOTE, 0x5C + i, 0x00);
            midi.sendShortMsg(JOG_DISPLAY_NOTE, _JOG_TIME_MODE[i - 1], initTimeMode);
            engine.makeConnection(grp, "playposition", PioneerDDJFLX10.jogMarker);
            engine.makeConnection(grp, "playposition", PioneerDDJFLX10.jogTime);
            engine.makeConnection(grp, "rate",         PioneerDDJFLX10.jogSpeed);
            engine.makeConnection(grp, "duration",     PioneerDDJFLX10.jogDuration);
            PioneerDDJFLX10.jogMarker(engine.getValue(grp, "playposition"), grp, "playposition");
            PioneerDDJFLX10.jogSpeed(engine.getValue(grp, "rate"), grp, "rate");
            PioneerDDJFLX10.jogTime(engine.getValue(grp, "playposition"), grp, "playposition");
        }
    }
    // 2026-05-25 DISABLED — this 250ms MIDI CC timer was sending jog-time
    // MIN/SEC to the FLX10 every 4 Hz using THROTTLED playposition. The
    // firmware fuses MIDI time + HID xx 27 time on the same display, causing
    // a visible 4 Hz beat pattern against our 200 Hz HID stream. The HID
    // screen handles time display now; MIDI CC is redundant.
    /* engine.beginTimer(250, function() { ... }); */

    // LEDs Play/Cue avancées (Pioneer-like)
    PioneerDDJFLX10._initAdvancedLeds();
    // Hot-cue pad LEDs: light pads that have a hot cue set.
    PioneerDDJFLX10._initHotcueLeds();

    if (typeof PioneerDDJFLX10.screen !== "undefined") {
        PioneerDDJFLX10.screen.start();
    } else {
        console.log("FLX10 screen: module not loaded — HID interface may be unavailable");
    }

    // HID screen SysEx handshake — sent LAST so that any failure here doesn't
    // break the MIDI mapping init above. The screen.js HID module relies on
    // these two SysEx messages: one-shot ENTER_HID then a 200 ms KEEPALIVE.
    try {
        if (PioneerDDJFLX10._SCREEN_MODE === 'vdj') {
            // VDJ MODE INIT — order from flx10-vdj-init capture (t=6.367-6.629).
            // All 7 SysEx commands in ~0.3 seconds, no per-deck SysEx.
            console.log("FLX10: initializing in VDJ mode (xx 21 + RGB-waveform)");
            for (var vi = 0; vi < PioneerDDJFLX10._SYSEX_VDJ_INIT.length; vi++) {
                var msg = PioneerDDJFLX10._SYSEX_VDJ_INIT[vi];
                midi.sendSysexMsg(msg, msg.length);
            }
            // 200ms keepalive timer (same VDJ keepalive 50 00)
            PioneerDDJFLX10._sysexKeepaliveTimer = engine.beginTimer(200, function() {
                midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_VDJ_KEEPALIVE,
                                  PioneerDDJFLX10._SYSEX_VDJ_KEEPALIVE.length);
            });
            console.log("FLX10: VDJ-mode init complete (" +
                        PioneerDDJFLX10._SYSEX_VDJ_INIT.length + " SysEx sent)");
        } else if (PioneerDDJFLX10._SCREEN_MODE === 'rekordbox') {
            // REKORDBOX MODE INIT — order verified from flx10-rekordbox-opening.
            // Rekordbox sends in two phases:
            //   EARLY (t≈28s in capture): keepalive variant + 4 per-deck init
            //   LATE  (t≈52s in capture, ~20s after init packets): Global B,
            //         Global C, long mode-config, ENTER_HID
            // We split here too. Daemon's HID init packets (xx 30/39/2d/3e/2c/2e/2f)
            // fire when daemon starts; the LATE SysEx is deferred so the firmware
            // sees init-packets BEFORE the final mode-enable handshake.
            console.log("FLX10: initializing in REKORDBOX mode (xx 21 + xx 3D)");
            midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_RKBOX_KEEPALIVE,
                              PioneerDDJFLX10._SYSEX_RKBOX_KEEPALIVE.length);
            for (var rd = 1; rd <= 4; rd++) {
                midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_RKBOX_DECK_INIT[rd],
                                  PioneerDDJFLX10._SYSEX_RKBOX_DECK_INIT[rd].length);
            }
            console.log("FLX10: rekordbox EARLY SysEx sent; LATE SysEx deferred 5s");
            // Use the rekordbox-variant keepalive (50 00) every 200ms.
            PioneerDDJFLX10._sysexKeepaliveTimer = engine.beginTimer(200, function() {
                midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_RKBOX_KEEPALIVE,
                                  PioneerDDJFLX10._SYSEX_RKBOX_KEEPALIVE.length);
            });
            // Defer LATE SysEx by 5s so daemon's init packets land first.
            engine.beginTimer(5000, function() {
                midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_RKBOX_GLOBAL_B,
                                  PioneerDDJFLX10._SYSEX_RKBOX_GLOBAL_B.length);
                midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_RKBOX_GLOBAL_C,
                                  PioneerDDJFLX10._SYSEX_RKBOX_GLOBAL_C.length);
                midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_RKBOX_MODE_CONFIG,
                                  PioneerDDJFLX10._SYSEX_RKBOX_MODE_CONFIG.length);
                midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_ENTER_HID,
                                  PioneerDDJFLX10._SYSEX_ENTER_HID.length);
                console.log("FLX10: rekordbox LATE SysEx sent (mode-enable handshake)");
            }, true);   // one-shot
        } else {
        // SESSION_START fires FIRST (before ENTER_HID), mirroring Serato's
        // captured order at t=25.242 (session-start) vs t=26.482 (ENTER_HID).
        midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_SESSION_START,
                          PioneerDDJFLX10._SYSEX_SESSION_START.length);
        midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_ENTER_HID,
                          PioneerDDJFLX10._SYSEX_ENTER_HID.length);
        // 2026-05-25 TEST: keepalive rate 200ms → 340ms (matching Serato's
        // measured ~3 Hz / 340ms interval from smoothplaying capture).
        // Hypothesis: too-fast keepalive might confuse firmware into a slow
        // refresh mode. Serato sends f0 00 40 04 05 00 00 04 04 01 00 07 50 31 f7
        // at exactly this rate during steady play.
        PioneerDDJFLX10._sysexKeepaliveTimer = engine.beginTimer(340, function() {
            midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_KEEPALIVE,
                              PioneerDDJFLX10._SYSEX_KEEPALIVE.length);
        });
        console.log("FLX10: HID screen SysEx handshake started");

        // EXPERIMENT 2026-05-24: send per-deck init SysEx that Serato sends.
        // Hypothesis: this puts firmware into "Serato mode" where time is
        // computed internally → would let us scroll the wave AND have
        // accurate time. (Without these, the firmware uses HID xx 27 [5..7]
        // position bytes for both wave scroll AND time computation, which
        // means we have to pick one.)
        for (var sd = 1; sd <= 4; sd++) {
            midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_DECK_INIT[sd],
                              PioneerDDJFLX10._SYSEX_DECK_INIT[sd].length);
        }
        midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_GLOBAL_B,
                          PioneerDDJFLX10._SYSEX_GLOBAL_B.length);
        midi.sendSysexMsg(PioneerDDJFLX10._SYSEX_GLOBAL_C,
                          PioneerDDJFLX10._SYSEX_GLOBAL_C.length);
        console.log("FLX10: Serato-mode per-deck SysEx sent (experimental)");

        // Send the beat-grid burst (7 02 0F messages from Serato capture)
        // for each loaded deck. Experiment 2026-05-24: tests whether these
        // are the trigger that makes firmware honor [9..12] time bytes when
        // [5..7] position is also non-zero.
        for (var bd = 1; bd <= 4; bd++) {
            PioneerDDJFLX10._sendBeatgridBurst(bd);
        }
        console.log("FLX10: beat-grid burst sent for all decks (experimental)");
        }   // end else (Serato mode)
    } catch (e) {
        console.log("FLX10: SysEx handshake failed (screen waveform won't render): " + e);
    }

    // ===== FLX10 screen daemon IPC =====
    // Two log line types the daemon (flx10_screen_daemon.py) tails:
    //   FLX10_TRACK_LOAD deck=N samples=X file_bpm=Y duration=Z
    //   FLX10_POS        deck=N pos=0.0234
    //
    // Track-load matching uses track_samples + file_bpm (both invariant to
    // pitch-fader adjustments) because Mixxx's controller API does NOT
    // expose track_id to scripts. The samples count alone is functionally
    // unique per track; file_bpm is a collision safeguard.
    //
    // FLX10_POS lines stream the play position (0..1) of every PLAYING deck
    // every 100ms — the daemon will use this to drive the firmware's
    // scrolling playhead once we figure out the byte encoding.

    PioneerDDJFLX10._lastLoggedDuration = {1: 0, 2: 0, 3: 0, 4: 0};
    PioneerDDJFLX10._lastLoggedPos      = {1: -1, 2: -1, 3: -1, 4: -1};

    PioneerDDJFLX10._logTrackLoadDeferred = function(deck) {
        var group   = "[Channel" + deck + "]";
        var dur     = engine.getValue(group, "duration");
        var samples = engine.getValue(group, "track_samples");
        var fbpm    = engine.getValue(group, "file_bpm");
        if (dur > 0 && samples > 0) {
            // A new track resets the key, so clear any "key sync engaged"
            // toggle state and turn its LED off.
            PioneerDDJFLX10._keySyncEngaged[deck] = false;
            PioneerDDJFLX10._setKeySyncLed(deck, false);
            // Re-send Serato's beat-grid burst on every track load.
            // Experimental — see _SYSEX_BEATGRID_BURST_PAYLOAD docs.
            try { PioneerDDJFLX10._sendBeatgridBurst(deck); } catch (e) {}
            console.log("FLX10_TRACK_LOAD deck=" + deck
                        + " samples=" + samples.toFixed(0)
                        + " file_bpm=" + fbpm.toFixed(2)
                        + " duration=" + dur.toFixed(2));
            // Force an immediate FLX10_BPM log so the daemon's pitch-adjusted
            // BPM display refreshes on track load (otherwise the polling loop
            // only logs when liveBpm CHANGES — if the new track's liveBpm
            // happens to equal _lastLoggedBpm, the display stays stuck on the
            // previous track's value until the user moves the pitch slider).
            var rateRatio = engine.getValue(group, "rate_ratio");
            var liveBpm   = fbpm * rateRatio;
            var rounded   = Math.round(liveBpm * 10) / 10;
            PioneerDDJFLX10._lastLoggedBpm[deck] = rounded;
            console.log("FLX10_BPM deck=" + deck + " bpm=" + rounded.toFixed(2));
        }
    };

    for (var d = 1; d <= 4; d++) {
        (function(deck) {
            engine.makeConnection(
                "[Channel" + deck + "]",
                "duration",
                function(value) {
                    if (value > 0 && value !== PioneerDDJFLX10._lastLoggedDuration[deck]) {
                        PioneerDDJFLX10._lastLoggedDuration[deck] = value;
                        // 300 ms defer so Mixxx populates track_samples + file_bpm
                        engine.beginTimer(300, function() {
                            PioneerDDJFLX10._logTrackLoadDeferred(deck);
                        }, true);
                    }
                }
            );
        })(d);
    }

    // Playhead position poller — every 100 ms, for any LOADED deck, log the
    // fractional position if it changed. (Tried 25ms for smoother display;
    // caused timer drift between FLX10 and Mixxx UI, possibly due to Mixxx
    // UI lag under heavy script polling. Reverted to 100ms.)
    PioneerDDJFLX10._lastLoggedBpm = {1: 0, 2: 0, 3: 0, 4: 0};
    engine.beginTimer(100, function() {
        for (var dd = 1; dd <= 4; dd++) {
            var grp = "[Channel" + dd + "]";
            var dur = engine.getValue(grp, "duration");
            if (dur <= 0) {
                continue;     // nothing loaded
            }
            // Position
            var pos = engine.getValue(grp, "playposition");
            var posRounded = Math.round(pos * 10000) / 10000;
            if (posRounded !== PioneerDDJFLX10._lastLoggedPos[dd]) {
                PioneerDDJFLX10._lastLoggedPos[dd] = posRounded;
                console.log("FLX10_POS deck=" + dd + " pos=" + posRounded.toFixed(4));
            }
            // Active BPM = stable file_bpm × Mixxx's rate_ratio.
            // We can't use Mixxx's `bpm` CO directly — it's the live
            // beat-tracker estimate and fluctuates wildly between beats.
            // rate_ratio is the actual playback multiplier (1.0 = no change,
            // 1.16 = 16% faster) and already accounts for rate_dir, so it's
            // direction-correct regardless of the slider-inversion setting.
            var fileBpm   = engine.getValue(grp, "file_bpm");
            var rateRatio = engine.getValue(grp, "rate_ratio");
            var liveBpm   = fileBpm * rateRatio;
            var bpmRounded = Math.round(liveBpm * 10) / 10;
            if (bpmRounded !== PioneerDDJFLX10._lastLoggedBpm[dd]) {
                PioneerDDJFLX10._lastLoggedBpm[dd] = bpmRounded;
                console.log("FLX10_BPM deck=" + dd + " bpm=" + bpmRounded.toFixed(2));
            }
        }
    });

    return true;
};

// Jog display mode cycle — fired by the dedicated "view" buttons on the FLX10
// jog rings (status 0x90, midino 0x01 for left, 0x09 for right). The FLX10
// firmware does NOT cycle modes on its own from these buttons; rekordbox
// drives the cycle by sending an HID xx 3D packet. We can't send HID from
// here (this is the MIDI script), so we log a marker the daemon picks up.
PioneerDDJFLX10._cycleJogDisplayCounter = {left: 0, right: 0};
PioneerDDJFLX10._cycleJogDisplay = function(side) {
    PioneerDDJFLX10._cycleJogDisplayCounter[side]++;
    console.log("FLX10_CYCLE_JOG_DISPLAY side=" + side
                + " seq=" + PioneerDDJFLX10._cycleJogDisplayCounter[side]);
};
PioneerDDJFLX10.cycleJogDisplayLeft = function(channel, control, value, status, group) {
    // Only act on press (val 0x7F), ignore release (0x00)
    if (value > 0) { PioneerDDJFLX10._cycleJogDisplay("left"); }
};
PioneerDDJFLX10.cycleJogDisplayRight = function(channel, control, value, status, group) {
    if (value > 0) { PioneerDDJFLX10._cycleJogDisplay("right"); }
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

// Shutdown
PioneerDDJFLX10.shutdown = function() {
    console.log("Pioneer DDJ-FLX10 PROD - Arrêt");

    // Stop the SysEx keepalive timer (paired with init).
    if (PioneerDDJFLX10._sysexKeepaliveTimer !== null) {
        engine.stopTimer(PioneerDDJFLX10._sysexKeepaliveTimer);
        PioneerDDJFLX10._sysexKeepaliveTimer = null;
    }

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
    if (engine.isScratching(deckNumber)) {
        engine.scratchTick(deckNumber, newValue);
    } else if (PioneerDDJFLX10._loopAdjustMode[deckNumber]) {
        var mode = PioneerDDJFLX10._loopAdjustMode[deckNumber];
        var ctrl = mode === "in" ? "loop_in" : "loop_out";
        var total = engine.getValue(group, "track_samples");
        if (total > 0) {
            var pos = engine.getValue(group, "playposition") * total;
            pos = Math.max(0, Math.min(total, pos + newValue * LOOP_ADJUST_STEP));
            engine.setValue(group, "playposition", pos / total);
            engine.setValue(group, ctrl, 1);
            engine.setValue(group, ctrl, 0);
        }
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

// ─── KEY SYNC button (note 0x65) — custom on/off toggle ───────────────────────
// The hardware KEY SYNC button sends Note On 0x65 on the deck's channel
// (deck N → status 0x90+(N-1)). We track an "engaged" state per deck:
//   1st press → sync_key  (match this deck's key to the other deck) + LED on
//   2nd press → reset_key (restore the track's original key)        + LED off
// Mimics a rekordbox-style "key sync engaged" toggle with a persistent light.
// (The blue JOG-SCREEN indicator is HID xx39, driven separately by the daemon;
//  wiring it to this state is a follow-up — see notes.)
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

// BPM display: bpm * 10 as a 14-bit integer.
// DISABLED — HID xx 27 BPM (from screen.js) is the authoritative source.
// Earlier test 2026-05-24: putting NO BPM in HID and relying on MIDI
// resulted in the FLX10 BPM display going blank — HID-blank-byte
// overrides MIDI. So screen.js owns BPM via HID, and this MIDI path
// stays a no-op.
PioneerDDJFLX10.jogBpm = function(value, group, control) {
    return;
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
    // Only push the MIDI time-mode note if the legacy MIDI jog-display is on;
    // otherwise this would re-enable the MIDI time element that fuses with HID
    // and causes the once-per-second full-duration flash. HID owns the display.
    if (PioneerDDJFLX10._MIDI_JOG_DISPLAY) {
        var hwMode = newMode !== 0 ? 0x7F : 0x00;
        for (var d = 0; d < 4; d++) {
            midi.sendShortMsg(JOG_DISPLAY_NOTE, _JOG_TIME_MODE[d], hwMode);
        }
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
    // Playing → PLAY solid ON.
    playOn = 1;
    cueOn  = st.hold ? 1 : 0;
  } else {
    // Not playing → PLAY FLASHES (toggled by _advTimer's phase) whenever a
    // track is loaded; off if the deck is empty. CUE stays solid while held
    // or while sitting on the cue point.
    var hasTrack = engine.getValue("[Channel" + deck + "]", "track_samples") > 0;
    playOn = (hasTrack && st.phase) ? 1 : 0;
    cueOn  = (st.hold || st.cue) ? 1 : 0;
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
    if (value === 0) return;
    if (PioneerDDJFLX10.shiftActive) {
        PioneerDDJFLX10._beatFxScanStart();
        return;
    }
    if (PioneerDDJFLX10._beatFxFadeTimer) {
        engine.stopTimer(PioneerDDJFLX10._beatFxFadeTimer);
        PioneerDDJFLX10._beatFxFadeTimer = 0;
    }
    PioneerDDJFLX10._beatFxActive = !PioneerDDJFLX10._beatFxActive;
    var slot;
    if (PioneerDDJFLX10._beatFxActive) {
        for (slot = 1; slot <= 3; slot++) {
            engine.setValue("[EffectRack1_EffectUnit1_Effect" + slot + "]", "enabled", 1);
            engine.setValue("[EffectRack1_EffectUnit1_Effect" + slot + "]", "mix", 1.0);
        }
    } else {
        // Fade mix to 0 over 750 ms (15 steps × 50 ms) so echo tails ring out naturally.
        var fadeStep = 0;
        var FADE_STEPS = 15;
        PioneerDDJFLX10._beatFxFadeTimer = engine.beginTimer(50, function() {
            fadeStep++;
            var ratio = Math.max(0, 1.0 - fadeStep / FADE_STEPS);
            for (var s = 1; s <= 3; s++) {
                engine.setValue("[EffectRack1_EffectUnit1_Effect" + s + "]", "mix", ratio);
            }
            if (fadeStep >= FADE_STEPS) {
                for (var s2 = 1; s2 <= 3; s2++) {
                    engine.setValue("[EffectRack1_EffectUnit1_Effect" + s2 + "]", "enabled", 0);
                }
                engine.stopTimer(PioneerDDJFLX10._beatFxFadeTimer);
                PioneerDDJFLX10._beatFxFadeTimer = 0;
            }
        });
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
    if (value === 0) return;
    var channels = PioneerDDJFLX10._colorFxChannels;
    var i;
    if (PioneerDDJFLX10._colorFxActive === control) {
        for (i = 0; i < channels.length; i++) {
            engine.setValue("[QuickEffectRack1_" + channels[i] + "_Effect1]", "enabled", 0);
        }
        PioneerDDJFLX10._colorFxActive = -1;
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
};

// Function to expand playlists folder (currently disabled)
// This feature requires a different approach that will be implemented later
