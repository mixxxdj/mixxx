// PioneerDDJFLX10-screen.js  v1.0 (Serato protocol, byte-perfect)
// HID jog wheel screen module for the DDJ-FLX10.  Loaded by
// PioneerDDJFLX10-screen.hid.xml as a separate HID controller mapping.
//
// ====================================================================
// CHANGE LOG
// ====================================================================
// 2026-05-29 — *** "FLASH TO START EVERY SECOND" SOLVED ***
// The xx 27 sub-second field (b8<<8 | b7) is MILLISECONDS (0..999), not
// 1024ths. We were sending floor(sub*1024) → 0..1023; for the last ~23 ms of
// each second the field was 1000..1023, which the firmware reads as >1.000 s
// of sub-second → overflow → one-frame snap to origin = the flash. Proof:
// flx10-serato-longplay maxes the field at exactly 999 over 66k packets,
// never 1023. Fix in _buildState: p[7]/p[8] = floor(sub*1000), cap 999.
// This SUPERSEDES the "÷1024 sub-second" model in the older entries below
// (that model was monotonic-consistent so it passed the 51k-transition
// check, but the 999-vs-1023 max disproves it). Cross-checks: b8=0 maxed the
// field at 255 (no overflow → no flash, but jumpy); pausing froze the display
// at origin inside the 1000..1023 band (~17.98 s); the tempo slider rescaled
// the field and slid the band off the paused position.
//
// 2026-05-29 — INTERP v4.1: ADDED PAUSE GUARD on top of v4 forward-only
// re-anchor. v4 alone broke pause: when Mixxx is paused, the play CO is
// 0 and pos stops updating, so msSince grew unboundedly until the +500ms
// runaway clamp fired — making the wave/time visibly tick forward then
// snap back during pause. v4.1 reads engine.getValue(group, "play")
// before interp and, when paused, freezes smoothMs at the exact
// wallElapsedSec*1000 without any extrapolation. Play-state interp logic
// (forward-only re-anchor + 500ms runaway clamp) unchanged from v4.
//
// 2026-05-29 — INTERP v4: FORWARD-ONLY RE-ANCHOR (eliminates the residual
// ~0.5s wave-flash). Forensic audit found our wire produced 234 backward
// position-byte jumps per minute (Serato has 0 in 53s). The firmware
// redraws the wave window on every backward jump. Pattern was:
//   * ~1 Hz big jumps (~750ms) at b6/second boundaries
//   * ~4 Hz mid jumps (~250ms) at b8 transitions
// Combined: ~2 Hz visible flash = user-observed 0.5s interval.
//
// Root cause was v3's unconditional `_lastSmoothMs = wallElapsedSec*1000`
// re-anchor on every Mixxx pos update, plus the +200ms safety clamp.
// Both could move smoothMs BACKWARD when our 200 Hz extrapolation had
// overshot Mixxx's 43 Hz ground truth. Each backward move = firmware sees
// position regress = wave-window redraw.
//
// FIX: re-anchor _lastSmoothMs ONLY when Mixxx's wall time has actually
// advanced past our extrapolation (forward-only update). Otherwise hold
// extrapolation and let msSince keep growing. Safety clamp raised from
// +200ms to +500ms so it only fires in pathological runaway (Mixxx stall),
// not on routine 23ms timing jitter.
//
// 2026-05-29 — BYTE 21 FIXED: rate multiplier was wrong, causing a
// once-per-second-of-file-time wave-flash artifact. Forensic audit
// across 3 BPMs (124/129/173) showed b21 advances at exactly 1991/1024
// per sub-second-tick (= 1991/sec of file-position time), wraps every
// 128.6 ms (~7.78 Hz). BPM-independent → it's positional, not musical.
//
// Our old formula `floor(smoothFileMs/6) * (2048/167) & 0xFF` had
// multiplier 2048/167 = 12.2635 per 6 ms boundary. After 167 boundaries
// (= 1.002 s of file-time): 167*12.2635 = 2048.0 EXACTLY → b21 = 0.
// So our b21=0 events landed on a perfectly periodic 1-second grid
// (pos 1.002s, 2.004s, 3.006s, …). The firmware redraws the wave
// window on b21=0, so a deterministic flash fired once per second.
// User reported "I can pause on the flash if I time it correctly" —
// confirming the deterministic-trigger interpretation.
//
// FIX: `p[21] = ((subsecTicks * 1991 / 1024) | 0) & 0xFF` where
// subsecTicks = sec_int*1024 + sub1024 (same field as b8/b22).
// 1991/1024 is irrational w.r.t. 256 so b21=0 now lands at scattered
// sub-second positions — matches Serato's wire pattern (b21=0 events
// at irregular file-positions, no periodic grid). Same wrap rate
// (~7.78 Hz), no perceptible flash.
//
// 2026-05-29 — BYTE 22 SOLVED: also position-driven, not a sixteenth-note
// counter. Forensic audit across 6 BPMs (88..173) showed b22 cycles
// 0→1→…→14→0 at a BPM-INDEPENDENT 1.80 s period (= 15 × 120 ms). The
// "8 Hz sixteenth-note" interpretation was a coincidence of 124 BPM
// matching the timer.
//
// Formula: b22 = floor(subsec_ticks / 123) % 15
// where subsec_ticks = (b5*60+b6)*1024 + ((b8&3)<<8) + b7 — the same
// 10-bit position field used by b5..b8. Each b22 step = 120.12 ms of
// track-position time. Holds when paused (verified across 3+ s pauses
// in beat-jump capture). Only primary decks (b31 ∈ {0x01,0x02}) get
// b22 driven; secondary (b31 ∈ {0x03,0x04}) hold at 0. In our code
// deckNum ≤ 2 = primary.
//
// User-reported residual artifact (wave content briefly flashing back
// to track-start every beat/half-beat after b8 fix) is the predicted
// behavior of holding b22=0: firmware interprets it as a frozen
// position-counter and periodically redraws the visible window from
// start. This fix should eliminate that artifact.
//
// 2026-05-29 — BYTE 18 INVESTIGATED, leave at 0. 6 of 7 captures
// (including macOS beat-loop) hold b18=0 the entire span. The outlier
// (parade mid-track) shows b18 ticking phase-locked to b22 cycles but
// appears tied to a Serato performance event we can't observe from
// USB. Held at 0; revisit only if a separate artifact appears.
//
// 2026-05-29 — BYTE 8 SOLVED: it's the sub-second high bits, not a beat
// counter. Forensic audit of 9 captures + cross-validation against macOS
// Serato captures showed the actual encoding:
//
//   T (wall-elapsed seconds) = b5*60 + b6 + (b8*256 + b7) / 1024
//   ^^^ DENOMINATOR CORRECTED 2026-05-29: it is /1000 (milliseconds), not
//       /1024. See the top-of-log "FLASH SOLVED" entry. The /1024 below was
//       wrong but monotonic-consistent, so this check still "passed."
//
// b5 = minutes, b6 = seconds, b7 = sub-second low 8 bits, b8 = sub-second
// high 2 bits. Zero monotonic violations across 51,477 transitions; the
// only "violations" on macOS were legitimate loop wraps. The cycle period
// is exactly 1.000 second of firmware-time, BPM-INDEPENDENT — confirming
// b8 is positional, not musical.
//
// All earlier b8 experiments (cycling 0,1,2,3; rotating the cycle; static
// values 1/2/3; monotonic 0..255) failed because they treated b8 as a
// beat phase counter when it's actually a 2-bit sub-second extension.
// Holding b8 = 0 (our long-standing baseline) made the firmware see only
// sub-second values 0.000..0.249 every second, then snap forward 3/4 s
// when b6 advanced — exactly the "1, 2, skip 3, 4" wave pattern AND the
// "decimal cycles 3, 2, 1" time-display artifact.
//
// FIX: in _buildState, compute sub1024 = round(sub * 1024) from the
// interpolated wall-elapsed seconds and write b8 = sub1024 >> 8 & 3,
// b7 = sub1024 & 0xff. Removed the 6 ms quantization on b7 (it was a
// workaround for the wrong encoding) and deleted the _CYCLE_B8 /
// _CYCLE_B8_OFFSET flags and per-deck beat-phase state.
//
// Also decoded from the macOS Serato beat-loop capture (NOT yet
// implemented — separate change set):
//   * byte 2 bit 3 = LOOP-ACTIVE flag (0xb4 → 0xbc when looping)
//   * byte 27 = beat-loop indicator: high nibble = state, low nibble =
//     beat-in-loop counter (0x10 + N at Nth eighth-note into loop).
//     This is the byte that produced the cycling blue rectangle when
//     we accidentally drove it during the dedup-defeat test.
//   * byte 26 = loop sub-position low byte; (b27<<8 | b26) is a 16-bit
//     monotonic loop position counter advancing ~1000/sec, freezes on
//     loop release.
//   * bytes 23 / 24 / 25 also shift between pre-loop and loop-active
//     state values — not "reserved" as previously assumed from Windows
//     steady-play captures.
//
// 2026-05-29 — BYTE 8 EXPERIMENTS (skip-beat-3, all SUPERSEDED by above):
//   * User-reported symptom: wave moves past stationary playhead with a
//     clear "shows beats 1, 2, [skips 3], 4" pattern per bar, plus a
//     "decimal value of duration cycles 3, 2, 1, 3, 2, 1" — both bar-
//     aligned, no tempo dependence.
//   * Diagnosis: byte 8 held at 0 (our 2026-05-25 workaround for an
//     earlier wave-flash artifact) starves the firmware's eighth-note
//     phase counter. Serato cycles byte 8 0→1→2→3 at half-beat rate
//     (audit-verified 4 Hz at 120 BPM).
//   * v1 (beat-aligned 4-cycle): read Mixxx beat_distance, cycled byte 8
//     through 0..3 in sync with actual beats. User test: wave-flash
//     reduced vs the old unaligned cycling, but didn't disappear — the
//     3→0 wrap on byte 8 was still misaligned vs firmware's expectation
//     of which exact beat is "the downbeat".
//   * v2 (monotonic, REVERTED): byte 8 incremented full 8-bit, wraps at
//     256. User test result: WAVE FROZE COMPLETELY. NEW PROTOCOL FACT —
//     byte 8 is firmware-validated to the 0..3 range; out-of-range values
//     are treated as "stop animating". So the 4-cycle is mandatory and
//     we can't escape the downbeat-wrap signal by widening it.
//   * v3 (current, Serato-rotation-matched): audit of Serato's actual
//     byte 8 sequence (theparade-steady-play, 198 transitions) shows the
//     cycle order is 1→2→3→0→1→…, NOT 0→1→2→3. Same value set, rotated
//     by one. v1 was off by exactly that rotation, which is consistent
//     with the residual wave-flash (firmware downbeat marker = 0x00 was
//     being delivered at the wrong musical moment). v3 adds +1 to the
//     output: when _beatPhase=0 and halfBeat=0 we now emit byte 8 = 1
//     instead of 0. byte 8 = 0 is reserved for the actual downbeat,
//     matching Serato's behavior.
//   * Bonus protocol-timing find: Serato's 0x03 → 0x00 transition is
//     ~25 ms earlier than the other three transitions (~230 ms vs
//     ~255 ms inter-transition gap). 0x00 appears slightly ahead of
//     the strict half-beat boundary — consistent with downbeat-marker
//     semantics. Not yet implemented; left for follow-up if needed.
//   * _CYCLE_B8 flag still controls all of this. Set false to revert to
//     held-at-0 baseline (skip-beat-3 returns).
//
// 2026-05-28 — xx 3D HEARTBEAT EXPERIMENT (Serato mode dedup defeat):
//   * Cross-protocol audit revealed Rekordbox sends a global xx 3D
//     heartbeat at ~404 Hz with byte 2 cycling 1→2→3→4→5 every packet
//     (100% bit-different from predecessor). Pioneer's own dedup-immune
//     timing pulse.
//   * Mixxx HidIoOutputReport cache is keyed per-report-id, but FLX10
//     declares NO report ids → all output reports share one cache slot.
//     Single-deck testing means consecutive xx 27 holds are bit-identical
//     and get deduped before hidraw. Interleaving xx 3D (always unique)
//     breaks the chain: cache holds xx 3D when next xx 27 arrives →
//     compare fails → every xx 27 hold reaches the wire.
//   * Added _SERATO_SEND_HEARTBEAT flag (default true). When on,
//     _sendStateAllDecks calls _sendXX3DHeartbeat() once per tick before
//     the deck loop. Function already existed (used by rekordbox mode).
//   * UNTESTED whether Serato-mode firmware accepts xx 3D — could be
//     silently ignored (best case, dedup defeated), or could glitch the
//     display, or worse. Flip the flag to false for single-toggle revert.
//
// 2026-05-27 (smoothness deep-dive session):
//   * Wave-needle ticking gap vs Serato traced to Mixxx's HID dedup,
//     NOT to our protocol encoding. Bytes 21, 7 already quantized to
//     6ms boundaries (matching Serato's 167 Hz audio-buffer cadence),
//     so when our boundary holds tick-over-tick the packet is bit-
//     identical to the previous one — Mixxx's HidIoOutputReport cache
//     drops it before hidraw sees it. Result: wire stream has ~3%
//     delta-24 (double-jump after a deduped hold) instead of Serato's
//     ~18% explicit delta-0 holds.
//   * Cycle-a-byte fix attempted, then RULED OUT (see audit below).
//     Byte 27 produced a cycling blue beat-loop rectangle. Byte 64 was
//     visually clean but user reported MORE jitter — firmware appears
//     to read padding bytes silently as part of its packet-cadence
//     clock. Per-byte audit across 4 Serato steady-play captures
//     (theparade / 4-decks / tracks-starting / play-middle-end): the
//     ONLY bytes that vary during steady play are 5, 6, 7, 8, 18, 21,
//     22 — every other byte is a single constant. No firmware-blessed
//     spare byte exists.
//   * Only viable dedup-defeat paths remaining: patch Mixxx
//     HidIoOutputReport for our VID/PID, OR move xx 27 to the Python
//     daemon (already proven to sustain 1000+ Hz via direct hidraw).
//     See memory:flx10-mixxx-hid-dedup.
//   * Byte 21 quantization (v4, earlier this session): file-time
//     floor(smoothFileMs/6) → scale by 2048/167. Removed Date.now()
//     jitter-driven outlier deltas (+6/+8/+16) that were confusing
//     firmware needle interpolation.
//   * Byte 7 quantization (v4): smoothMsQ = floor(smoothMs/6)*6 then
//     standard min/sec/sub-sec encoding. Matches the same audio-buffer
//     cadence Serato uses for byte 7.
//   * Interp v3 hard re-anchor: every Mixxx pos update resets
//     _lastSmoothMs to wallElapsedSec*1000 cleanly. Previous Math.max
//     guard preserved drift accumulation forever once smoothMs got
//     ahead of wall (observed 2.3s drift after long playback).
//
// 2026-05-26 (late-night session):
//   * Byte 29 CAMELOT KEY encoding cracked. Full 24-key lookup table
//     wired to Mixxx's file_key CO. First DJ controller in Mixxx to
//     display real per-track Camelot keys on the FLX10 jog screen.
//     Pattern: even bytes 0x82..0x96 = A-side (minor), odd bytes
//     0x81..0x97 = B-side (major); each +2 step advances Camelot
//     number by +7 mod 12 (circle of fifths). 0x80 / 0x99 = "no key"
//     markers. See memory:flx10-protocol-full-decode for full table.
//   * Conditional MODE TOGGLE: byte 3/19/29/trailer adapt based on
//     `file_bpm > 0`. Tracks with beatgrid → theparade mode (b3=0x88,
//     b19=0x07, trailer ad/05/00). Tracks without → smoothplay mode
//     (b3=0x80, b19=0, trailer e0/01/00). Matches Serato's two modes
//     across 10+ captures.
//   * Byte 21 anchored to FILE-TIME × 2048/sec (was wall-time × 125).
//     Fixes wave-needle "swim" when tempo slider moves AND eliminates
//     the "smooth 2 bars then jump 2 bars" artifact (125/sec wrapped
//     every 2.048s ≈ 2 bars at 124 BPM; 2048/sec wraps every 125ms,
//     sub-perceptible). 2048/sec is Serato's measured active-play
//     rate from theparade capture (8 wraps over 0.987s).
//   * Bytes 8, 18, 22 held at 0 to eliminate the "wave content
//     flashes to track-start every 4 beats" artifact. Cycling these
//     (Serato's pattern) put firmware into beat-aligned-redraw mode
//     that we couldn't align cleanly.
//   * Byte 7 → 256/sec (was 1024/sec). Wraps once per second at the
//     byte-6 increment boundary instead of 4x per second mid-second,
//     killing the 4 Hz wave-needle shimmer.
//   * Forward-only interp logic: don't reset _lastPosTime if real pos
//     hasn't passed our extrapolation. Fixes the "-2 then +4" jitter.
//   * Removed diagnostic logs (FLASH, SWEEP, KEY) after cleanup.
//
// OPEN ITEMS (next session):
//   * Millisecond-scale wave-needle ticking. Probably the update-rate
//     bottleneck: JS xx 27 at 200 Hz but Mixxx playposition CO updates
//     at audio-buffer rate (~43 Hz). Visible quantization between Mixxx
//     ticks. Worth: faster CO polling, interp smoothing tweaks, daemon
//     refresh-rate bump from 127ms.
//   * Byte 18 measure-counter (0..5 cycling in theparade mode) is
//     held at 0 for now. Cycling caused wave-flash without proper beat
//     alignment from Mixxx; would need beat_distance CO integration.
//   * Bytes 23, 24, 26-28 confirmed always-zero in 9+ Serato captures.
//     Reserved/unused — do not touch.
// ====================================================================
//
// This v1.0 module is the result of multi-day reverse engineering that
// culminated 2026-05-23.  Every packet here is byte-for-byte verified against
// captures from Serato DJ Pro driving the same FLX10 hardware.  When the
// MIDI mapping (Pioneer-DDJ-FLX10-scripts.js) does the SysEx handshake and
// the user has run the vendor unlock (flx10_unlock_v2.py), this module
// renders a real waveform across the FLX10's jog wheel discs.
//
// CRITICAL PRECONDITIONS (without these the firmware silently drops everything):
//
//   1. Vendor unlock done.  Run:
//        sudo python3 ~/.mixxx/controllers/flx10_unlock_v2.py
//      once after plugging in the FLX10.  Mixxx cannot do this itself —
//      it requires snd-usb-audio unbind/rebind on Linux.
//
//   2. SysEx handshake running on MIDI OUT:
//        - F0 00 40 05 00 00 04 01 00 03 01 F7   sent once at startup
//        - F0 00 40 05 00 00 04 01 00 50 31 F7   keepalive every ~200 ms
//      These are sent from the MIDI mapping (PioneerDDJFLX10.init / .shutdown
//      in Pioneer-DDJ-FLX10-scripts.js) because Mixxx HID controller scripts
//      cannot send MIDI.
//
// SEND ORDER for waveform render (per deck, on track load):
//   xx 27 state → xx 30 init → xx 39 labels → xx 33 album art → xx 35 begin →
//   xx 36 waveform → xx 2f cue placeholder
// Plus xx 27 sustained at 50 Hz across all 4 decks while running.

var PioneerDDJFLX10Screen = {};

PioneerDDJFLX10Screen._DECK_BYTE     = [0x10, 0x20, 0x30, 0x40];
PioneerDDJFLX10Screen._STATE_BYTE_31 = {0x10: 0x02, 0x20: 0x01, 0x30: 0x04, 0x40: 0x03};
PioneerDDJFLX10Screen._STATE_MS      = 5;      // 200 Hz xx 27. Pairs with the
                                               // patched Mixxx visual_playposition CO
                                               // (sample-accurate, unthrottled, updates
                                               // every audio buffer). At 5ms we get many
                                               // samples per CO update for smooth wave.
// FLX10 vs Mixxx UI fixed offset compensation (seconds).
// playposition CO returns the engine's processing position; Mixxx UI shows
// the position adjusted for output-audio latency (audio buffer + DAC).
// Measured 2026-05-24: FLX10 was 0.96s "behind" Mixxx UI on a paused track
// (first tried 0.66 then bumped after residual gap). Likely audio buffer +
// display refresh + scripts.js timer-tick latency combined.
// Adjust if your audio buffer setting differs.
PioneerDDJFLX10Screen._TIME_OFFSET_SEC = 0.96;
PioneerDDJFLX10Screen._cachedTimeBytes = {1: null, 2: null, 3: null, 4: null};
PioneerDDJFLX10Screen._lastTimeRefresh = {1: 0, 2: 0, 3: 0, 4: 0};

PioneerDDJFLX10Screen._getCachedTimeBytes = function(deckNum, pos, duration) {
    var now = Date.now();
    if (this._cachedTimeBytes[deckNum] === null ||
        now - this._lastTimeRefresh[deckNum] >= this._TIME_REFRESH_MS) {
        this._lastTimeRefresh[deckNum] = now;
        if (duration > 0) {
            var pClamped = pos;
            if (pClamped < 0.0) pClamped = 0.0;
            if (pClamped > 1.0) pClamped = 1.0;
            var remainingSec = duration * (1.0 - pClamped);
            var totalMs = Math.round(remainingSec * 1000);
            if (totalMs < 0) totalMs = 0;
            var minutes = Math.floor(totalMs / 60000);
            var remMs   = totalMs % 60000;
            var seconds = Math.floor(remMs / 1000);
            var ms      = remMs % 1000;
            this._cachedTimeBytes[deckNum] = [
                minutes & 0xFF,
                seconds & 0xFF,
                ms & 0xFF,
                (ms >> 8) & 0xFF
            ];
        } else {
            this._cachedTimeBytes[deckNum] = [0x06, 0x1b, 0xfa, 0x01];
        }
    }
    return this._cachedTimeBytes[deckNum];
};
PioneerDDJFLX10Screen._WAVE_DURATION = 30;     // seconds of test-pattern waveform
PioneerDDJFLX10Screen._lastDuration  = {1: 0, 2: 0, 3: 0, 4: 0};
PioneerDDJFLX10Screen._lastPos        = {};
PioneerDDJFLX10Screen._lastPosTime    = {1: 0, 2: 0, 3: 0, 4: 0};
PioneerDDJFLX10Screen._lastSmoothMs   = {1: 0, 2: 0, 3: 0, 4: 0};
// 2026-05-29 v4.2: wall time of the LAST Mixxx pos-value-change. Used to
// detect "scrub-hold" / stall where play=1 but pos hasn't advanced. When
// pos is held still for >200ms while play=1 (e.g. user touching the wave),
// our forward-only interp would keep extrapolating smoothMs forward —
// same visible-movement bug as the pre-v4.1 pause case.
PioneerDDJFLX10Screen._lastNewPosTime = {1: 0, 2: 0, 3: 0, 4: 0};
PioneerDDJFLX10Screen._stateTimer    = null;


// ===== Raw HID send via Mixxx ===============================================
PioneerDDJFLX10Screen._lastSentPkt = null;
PioneerDDJFLX10Screen._send = function(pkt) {
    // Skip byte-identical consecutive packets (our own dedup). Mixxx's
    // HidIoThread already drops identical OutputReports from its cache, but it
    // logs "Skipped sending identical OutputReport" at debug level for EACH one
    // — at 200 Hz a paused/idle deck sends the same xx27 every tick, spamming
    // the log. Deduping here means we never hand Mixxx the duplicate, so no
    // skip-log and no change in what reaches the device (Mixxx wasn't writing
    // it anyway). _buildState returns a fresh array each call, so storing the
    // reference is safe.
    if (this._lastSentPkt && pkt.length === this._lastSentPkt.length) {
        var same = true;
        for (var i = 0; i < pkt.length; i++) {
            if (pkt[i] !== this._lastSentPkt[i]) { same = false; break; }
        }
        if (same) { return; }
    }
    this._lastSentPkt = pkt;
    controller.send(pkt, null, 0);
};

// Bulk track-load upload path. All our packets share HID reportID 0, so the
// default skipping send() would supersede-and-drop a rapid burst of *different*
// xx36/xx2f packets. The non-skipping FIFO (4th send() arg = true) preserves
// them, but it's only 32 deep and overflows (silently drops) if filled faster
// than hid_write drains (~1000/s). So _sendRaw just ENQUEUES here; the state
// tick (_sendStateAllDecks) drains the queue into the FIFO within a per-tick
// budget, after xx27 — never exceeding the drain rate. This gives the daemon's
// behavior (xx27 always flows; the bulk uses leftover bandwidth) without the
// daemon's separate hidraw writer.
PioneerDDJFLX10Screen._txQueue = [];

PioneerDDJFLX10Screen._sendRaw = function(pkt) {
    this._txQueue.push(pkt);   // drained, budgeted, by the state tick
};

PioneerDDJFLX10Screen._zeros = function() {
    var p = [];
    for (var i = 0; i < 128; i++) { p[i] = 0; }
    return p;
};


// Per-deck cache of Mixxx-derived data we want to bake into the next xx 27 ping.
PioneerDDJFLX10Screen._deckBpm = {1: 0, 2: 0, 3: 0, 4: 0};

PioneerDDJFLX10Screen._deckFromDeckByte = function(deckByte) {
    if (deckByte === 0x10) { return 1; }
    if (deckByte === 0x20) { return 2; }
    if (deckByte === 0x30) { return 3; }
    if (deckByte === 0x40) { return 4; }
    return 0;
};

// ===== xx 27 — per-deck state ping (sent at 200 Hz to all 4 decks) ==========
// Reads pos / duration / BPM directly from Mixxx COs each tick — no log-tail
// lag.
//
// CURRENT byte map (verified 2026-05-26 — see file header CHANGE LOG):
//   [0]      Deck byte 0x10/0x20/0x30/0x40
//   [1]      0x27 (packet type)
//   [2]      0xb4 (const)
//   [3]      Mode flag: 0x88 (theparade mode) / 0x80 (smoothplay)
//   [4]      0x01 (const)
//   [5..7]   ELAPSED time as min / sec-in-min / sub-sec(256/sec, wraps at 1s)
//   [8]      Eighth-note counter (we HOLD AT 0 — cycling causes wave-flash)
//   [9..12]  REMAINING time: min(1B) / sec(1B) / ms LE16
//            ⚠ if [9..12] all zero, firmware drops the entire wave display
//   [13]     BPM integer
//   [14]     BPM frac × 16 (high nibble) — low nibble unused
//   [15..17] Tempo % offset: LE16 of (rate_ratio-1)*100*100; [15] = 0
//   [18]     Measure counter (we HOLD AT 0 — cycling causes wave-flash)
//   [19]     Beat-grid-active flag: 0x07 (theparade) / 0x00 (smoothplay)
//   [20]     0x0e (const)
//   [21]     Wave-entry counter: FILE-time × 2048/sec & 0xFF (Serato rate)
//   [22]     Sixteenth counter (we HOLD AT 0 — cycling causes wave-flash)
//   [23,24]  Reserved (always 0 in 9+ Serato captures — DO NOT TOUCH)
//   [25]     0x80 (const)
//   [26..28] Reserved (always 0 — DO NOT TOUCH)
//   [29]     CAMELOT KEY (see MIXXX_KEY_TO_B29 lookup in _buildState).
//            Also doubles as "loaded marker" — 0x80 = empty deck.
//   [30]     0x0d (const)
//   [31]     Deck state byte (see _STATE_BYTE_31 table)
//   [32..34] Trailer: ad/05/00 (theparade) / e0/01/00 (smoothplay)
// ==== _POS_RATE: DEAD CODE as of 2026-05-26 — DO NOT TUNE ====
// This constant is no longer referenced. Kept ONLY for the calibration
// values in comments (they may be useful if someone re-tries BE24-encoded
// bytes 5..7). The current production encoding writes bytes 5..7 as
// min/sec/sub-sec time-display fields, not as a BE24 wave-needle counter.
// The wave-needle position now comes from byte 21 (file-time × 2048/sec)
// and from xx 36 packets sent by the daemon.
//
// HISTORICAL calibration (when bytes 5..7 WERE BE24 = elapsed × _POS_RATE):
//   POS_RATE=250 → -1.5s drift / 60s
//   POS_RATE=256 → -0.5s drift / 60s
//   POS_RATE=258 → -0.2s drift / 60s
//   POS_RATE=260 → +0.2s drift / 60s   (IDENTICAL at tempo 0/+5/-5)
//   Linear fit: firmware rate ≈ 259.14 → 259 gives near-zero drift.
PioneerDDJFLX10Screen._POS_RATE = 259.0;   // unused — see above

// SCREEN MODE — pivot 2026-05-24. 'serato' uses xx 27 (legacy, drift issue).
// 'rekordbox' uses xx 21 + xx 3D heartbeat. 'vdj' uses xx 21 (different play
// values: 04 vs rekordbox 00) + no heartbeat. Keep this in sync with the
// matching flag in Pioneer-DDJ-FLX10-scripts.js (PioneerDDJFLX10._SCREEN_MODE).
PioneerDDJFLX10Screen._SCREEN_MODE = 'serato';

// (Serato-mode only) A/B flag: when false, position bytes [5..7] are zero
// (firmware uses our [9..12] time bytes directly → accurate time, static wave);
// when true, position bytes encode elapsed×POS_RATE BE24 (wave scrolls but
// firmware re-derives time from position at 0.5x rate → drift).
// Default false: user explicitly prefers time accuracy over wave scroll
// ("I cant be having ANY drifting"). The firmware coupling between needle
// position and time display can't be broken in Serato mode (see history).
PioneerDDJFLX10Screen._SEND_POSITION = true;

// 2026-05-25: BPM-zero test confirmed firmware does NOT use our BPM bytes
// for drift formula (uses internal BPM somehow). Reverting to sending real BPM.
PioneerDDJFLX10Screen._ZERO_BPM_FOR_DRIFT_TEST = false;

// 2026-05-29 — Idle xx 27 template per macOS Serato audit (deck 1 + deck 2).
// Serato sends this packet shape for any deck without a loaded track, at
// ~40 Hz on each idle deck. Distinguished from the loaded packet by:
//   * bytes 5..15 all zero (no position / time / BPM)
//   * b16/b17 = per-deck constants (deck 1: 0x38 0x00, deck 2: 0x70 0xff)
//   * b19 = 0x00 (not 0x07 like loaded)
//   * b21..b28 = zero (no counter values)
//   * b25 = 0x80 (constant)
//   * b29 = 0x80 (no-key marker)
//   * b30/b31 = 0x0d / per-deck state (0x02 deck1, 0x01 deck2)
//   * b32/b33/b34 = 0xff 0xff 0xff trailer (NOT 0xad 0x05 0x00)
// Decks 3/4 idle templates haven't been captured yet — only build for 1/2.
PioneerDDJFLX10Screen._IDLE_B16_B17 = {1: [0x38, 0x00], 2: [0x70, 0xff]};
PioneerDDJFLX10Screen._buildIdleState = function(deckNum) {
    var p = this._zeros();
    p[0]  = this._DECK_BYTE[deckNum - 1];
    p[1]  = 0x27;
    p[2]  = 0xb4;
    p[3]  = 0x88;
    p[4]  = 0x01;
    // b5..b15 all zero
    var b16b17 = this._IDLE_B16_B17[deckNum];
    p[16] = b16b17[0];
    p[17] = b16b17[1];
    // b18, b19 zero
    p[20] = 0x0e;
    // b21..b24 zero
    p[25] = 0x80;
    // b26..b28 zero
    p[29] = 0x80;  // no-key marker
    p[30] = 0x0d;
    p[31] = this._STATE_BYTE_31[this._DECK_BYTE[deckNum - 1]];
    p[32] = 0xff;
    p[33] = 0xff;
    p[34] = 0xff;
    return p;
};

PioneerDDJFLX10Screen._buildState = function(deckByte, trackLoaded) {
    var p = this._zeros();
    p[0]  = deckByte;
    p[1]  = 0x27;
    p[2]  = 0xb4;
    p[4]  = 0x01;
    p[20] = 0x0e;
    p[25] = 0x80;
    p[30] = 0x0d;
    p[31] = this._STATE_BYTE_31[deckByte];
    // Mode-flag bytes (3, 19, 29, 32-34) are set CONDITIONALLY inside the
    // trackLoaded block based on whether Mixxx has a beat grid for the track
    // (proxy: file_bpm > 0).
    //   - Beat-grid present  → theparade mode (b3=88 b19=07 b29=8c trailer=ad/05/00)
    //   - Beat-grid absent   → older Serato mode (b3=80 b19=00 b29=92 trailer=e0/01/00)
    // For empty/unloaded decks, use the simpler default below.
    p[3]  = 0x80;
    p[19] = 0x00;
    p[32] = 0xe0;
    p[33] = 0x01;
    p[34] = 0x00;
    if (trackLoaded) {
        var deckNum = this._deckFromDeckByte(deckByte);
        var group   = "[Channel" + deckNum + "]";
        // Use playposition (engine-processing position, stable). The flash was
        // never a CO-choice issue (it was the ms-overflow, now fixed), so the
        // visual_playposition cross-check and POS_GLITCH logging were removed.
        var pos = engine.getValue(group, "playposition");
        var duration = engine.getValue(group, "duration");
        var fileBpm  = engine.getValue(group, "file_bpm");
        var rateRatio = engine.getValue(group, "rate_ratio");
        var liveBpm  = fileBpm * rateRatio;

        // 2026-05-26 CAMELOT KEY ENCODING — byte 29.
        // CORRECTED mapping after empirical test with real Mixxx tracks:
        //   Odd  values 0x81,0x83,...,0x97 → B-side (major) Camelot
        //   Even values 0x80,0x82,...,0x96 → A-side (minor) Camelot
        // (The b29 sweep test had A/B transposed in user's notes — verified
        //  against minor tracks displaying B-side when we sent odd bytes.)
        // Camelot number cycle: 8, 3, 10, 5, 12, 7, 2, 9, 4, 11, 6, 1 (circle of fifths).
        // Maps to Mixxx's file_key CO enum (1-24: 1-12=major, 13-24=minor chromatic).
        //
        // Display-mode bytes — "theparade" set (b3=88, b19=07, trailer=ad/05/00).
        // These were briefly switched to the longplay set (80/00/e0/01/00) while
        // chasing the flash; that was a dead end (the flash was the ms-overflow,
        // see top-of-file), so reverted to theparade — which keeps the b29
        // Camelot-key calibration below valid (it was measured in this mode).
        p[3]  = 0x88;
        p[19] = 0x07;
        p[32] = 0xad;  p[33] = 0x05;  p[34] = 0x00;
        // 2026-05-26 v2 — corrected after live track test. The original sweep
        // was off by one Camelot position; rebuilt from direct (file_key,
        // FLX10 display) measurements. Pattern: each +2 byte step advances
        // the Camelot number by +7 (mod 12), starting from 0x82=8A.
        // 0x80 displays nothing (firmware treats as "no key" marker).
        // 1A maps to 0x98 (extrapolated — completes the 12-position cycle).
        // ODD bytes are inferred as B-side counterparts (not yet directly tested).
        // 2026-05-26 v3 — finalized after testing both A- and B-side tracks.
        // Pattern: each +2 byte step advances Camelot number by +7 (mod 12).
        // A-side (even bytes) cycle starts at 0x82=8A and 0x80 is the "no key"
        // marker. B-side (odd bytes) cycle starts at 0x81=8B with no skip;
        // its "no key" marker is at the END (0x99). Two terminal markers,
        // not one. (Confirmed: 0x8f→9B in Round 4 test; 0x99→nothing.)
        var MIXXX_KEY_TO_B29 = [
            0x80,   //  0 invalid / unanalyzed → "no key" display
            0x81,   //  1 C major     = 8B
            0x83,   //  2 D♭ major    = 3B
            0x85,   //  3 D major     = 10B
            0x87,   //  4 E♭ major    = 5B
            0x89,   //  5 E major     = 12B
            0x8b,   //  6 F major     = 7B
            0x8d,   //  7 F# major    = 2B
            0x8f,   //  8 G major     = 9B
            0x91,   //  9 A♭ major    = 4B
            0x93,   // 10 A major     = 11B
            0x95,   // 11 B♭ major    = 6B
            0x97,   // 12 B major     = 1B
            0x88,   // 13 C minor     = 5A
            0x8a,   // 14 C# minor    = 12A
            0x8c,   // 15 D minor     = 7A
            0x8e,   // 16 E♭ minor    = 2A
            0x90,   // 17 E minor     = 9A
            0x92,   // 18 F minor     = 4A
            0x94,   // 19 F# minor    = 11A
            0x96,   // 20 G minor     = 6A
            0x98,   // 21 G# minor    = 1A
            0x82,   // 22 A minor     = 8A
            0x84,   // 23 B♭ minor    = 3A
            0x86    // 24 B minor     = 10A
        ];
        // 2026-05-29: read the CURRENT key (`key` CO) so the on-screen Camelot
        // value follows key changes — e.g. when KEY SYNC (sync_key) shifts the
        // deck to match the other deck, or any pitch-driven key shift. `key`
        // uses the same 1..24 enum as file_key. Fall back to file_key if the
        // current key is unavailable (0), e.g. an un-keyed track.
        var keyIdx = engine.getValue(group, "key") | 0;
        if (keyIdx < 1 || keyIdx > 24) {
            keyIdx = engine.getValue(group, "file_key") | 0;
        }
        if (keyIdx < 0 || keyIdx > 24) keyIdx = 0;
        p[29] = MIXXX_KEY_TO_B29[keyIdx];

        // 2026-05-29 v5.2: USER-MEASURED CORRECTION on Priority 5.
        // v5.1 hybrid sent 0x0038 at 0% slider on deck 1, but the firmware
        // displayed +0.6% — meaning 0x0038 was Serato's actual tempo value
        // in those captures (slider was at +0.56%, not exactly 0). So the
        // per-deck "base" was a measurement artifact, not a firmware-mode
        // constant. Reverting to pure tempo encoding for correct display.
        //
        // The smoothness boost we observed during v5.0/v5.1 was likely from
        // BYTE 15: macOS Serato sends b15 = 0x01 constant in steady play
        // (per round-6 forensic audit), we were sending b15 = 0. Setting
        // b15 = 0x01 may keep the smoothness gain without breaking tempo.
        if (rateRatio > 0) {
            var pctOffset = (rateRatio - 1.0) * 100.0;
            var tempoEnc = Math.round(pctOffset * 100.0);
            if (tempoEnc < 0) tempoEnc = 0x10000 + tempoEnc;   // two's-comp LE16
            if (tempoEnc > 0xFFFF) tempoEnc = 0xFFFF;
            p[15] = 0x01;                   // was 0 — try Serato's value
            p[16] =  tempoEnc        & 0xFF;
            p[17] = (tempoEnc >>  8) & 0xFF;
        } else {
            p[15] = 0x01;
        }

        // [9..12] = WALL duration = file_duration / rate_ratio.
        // FLOOR (truncate) sub-sec to tenths. Math.round in JS rounds X.5 UP
        // (Math.round(0.5)==1) which would add a 50-100ms bias for any track
        // whose sub-sec is in the .X50..X99 range. Floor truncates cleanly.
        if (duration > 0) {
            var rrForDur = rateRatio > 0 ? rateRatio : 1.0;
            var wallDurSec = duration / rrForDur;
            var totalMsDur = Math.floor(wallDurSec * 1000);
            var minutesD = Math.floor(totalMsDur / 60000);
            var remMsD   = totalMsDur % 60000;
            var secondsD = Math.floor(remMsD / 1000);
            var msD      = Math.floor((remMsD % 1000) / 100) * 100;
            p[9]  = minutesD & 0xFF;
            p[10] = secondsD & 0xFF;
            p[11] = msD & 0xFF;
            p[12] = (msD >> 8) & 0xFF;
        } else {
            p[9]  = 0x06;  p[10] = 0x1b;  p[11] = 0xfa;  p[12] = 0x01;
        }

        // BPM integer + decimal tenths nibble (same reason as time —
        // sending zero bytes makes the FLX10 BPM field blank).
        // 2026-05-25 TEST: try zeroing BPM bytes to see if firmware drift
        // formula uses the BPM bytes we send vs internal BPM.
        // BPM restored — testing BPM=0 confirmed BPM bytes aren't the
        // cause of 2 Hz jitter. The user's perceived "downbeat" jumps may
        // be coincidental with the firmware's fixed refresh rate.
        if (liveBpm > 0) {
            p[13] = Math.floor(liveBpm) & 0xFF;
            var frac = liveBpm - Math.floor(liveBpm);
            p[14] = (Math.round(frac * 16) & 0x0F) << 4;
        }

        // 2026-05-25 ROOT-CAUSE FIX for "constant fine-grained shimmer":
        // Bytes [5..7] are read by firmware as a continuous BE24 wave-needle
        // counter (per protocol decode memory: "Drives wave needle"). The
        // previous encoding (byte 5 = minutes, byte 6 = sec-in-min, byte 7 =
        // sub-sec at 1024/sec % 256) caused byte 7 to wrap every 250ms,
        // making the BE24 drop by ~254 four times per second — visible as a
        // 4Hz back-2-forward-4 wave-needle vibration. New encoding: BE24 =
        // floor(elapsed × POS_RATE), POS_RATE=128 (daemon's empirically
        // determined in-sync rate). Remaining-time display is unaffected
        // because it's driven by bytes [9..12].
        if (this._SEND_POSITION && duration > 0) {
            var pClampedP = pos;
            if (pClampedP < 0.0) pClampedP = 0.0;
            if (pClampedP > 1.0) pClampedP = 1.0;
            var rrForPos = rateRatio > 0 ? rateRatio : 1.0;
            var fileElapsedSec = duration * pClampedP;
            var wallElapsedSec = fileElapsedSec / rrForPos;
            var isPlaying = engine.getValue(group, "play") > 0;

            // Mixxx's visual_playposition CO updates ~43 Hz (audio buffer rate)
            // but our timer runs at 200 Hz. Without interpolation, byte 7
            // stair-steps every 23ms with ~24-unit jumps = visible jitter.
            // With interpolation: linear forward extrapolation between Mixxx
            // updates.
            var nowMs = Date.now();
            var smoothMs;
            if (!isPlaying) {
                // 2026-05-29 v4.1: when Mixxx is PAUSED, freeze smoothMs at
                // the exact wall-elapsed value. Without this, msSince grows
                // unboundedly between Mixxx updates (which don't arrive while
                // paused), making the wave/time appear to keep moving and
                // periodically resync via the runaway clamp = visible flash.
                this._lastPos[deckNum]      = pos;
                this._lastPosTime[deckNum]  = nowMs;
                this._lastSmoothMs[deckNum] = wallElapsedSec * 1000;
                smoothMs = Math.floor(wallElapsedSec * 1000);
            } else {
                // Track wall time of the last actual pos-value-change. Used
                // for scrub-hold stall detection below.
                if (this._lastPos[deckNum] === undefined ||
                    pos !== this._lastPos[deckNum]) {
                    this._lastNewPosTime[deckNum] = nowMs;
                }
                if (this._lastPos[deckNum] === undefined ||
                    Math.abs(pos - this._lastPos[deckNum]) > 0.005) {
                    // Track loaded / seek: reset interpolation base
                    this._lastPos[deckNum]     = pos;
                    this._lastPosTime[deckNum] = nowMs;
                    this._lastSmoothMs[deckNum] = wallElapsedSec * 1000;
                } else if (pos !== this._lastPos[deckNum]) {
                    // 2026-05-29 v4.3 FIX: handle backward pos jumps as
                    // user seeks (snap immediately) while keeping the v4
                    // forward-only re-anchor for forward updates. Earlier
                    // v4 logic refused to update _lastSmoothMs on backward
                    // pos jumps too  causing the wave/time to keep going
                    // forward during a backward scrub until the runaway
                    // clamp fired (~500ms catchup lag). Mixxx's pos doesn't
                    // jitter backward during normal play (Serato's wire has
                    // 0 backward jumps in 53s), so any backward Δpos is a
                    // genuine user seek and should snap.
                    var posDelta = pos - this._lastPos[deckNum];
                    this._lastPos[deckNum] = pos;
                    if (posDelta < 0) {
                        // Backward = user seek/scrub. Snap to current.
                        this._lastSmoothMs[deckNum] = wallElapsedSec * 1000;
                        this._lastPosTime[deckNum]  = nowMs;
                    } else {
                        // Forward: only re-anchor when wall has actually
                        // moved past our extrapolation (v4 behavior).
                        var newWallMs      = wallElapsedSec * 1000;
                        var extrapolatedMs = this._lastSmoothMs[deckNum] +
                                             (nowMs - this._lastPosTime[deckNum]);
                        if (newWallMs > extrapolatedMs) {
                            this._lastSmoothMs[deckNum] = newWallMs;
                            this._lastPosTime[deckNum]  = nowMs;
                        }
                        // Else: hold extrapolation; msSince keeps growing.
                    }
                }
                var msSince = nowMs - this._lastPosTime[deckNum];
                smoothMs = Math.floor(this._lastSmoothMs[deckNum] + msSince);
                // 2026-05-29 v4.2 — SCRUB-HOLD FREEZE. If Mixxx has not sent
                // a new pos value in 200ms while play=1, treat as a stall
                // (scrub-touch / wave scratch / Mixxx audio glitch) and snap
                // smoothMs to wallElapsedSec*1000. Otherwise our forward-only
                // interp keeps extrapolating and the wave/time visibly drifts
                // forward — same symptom as pre-v4.1 paused state.
                if (nowMs - this._lastNewPosTime[deckNum] > 200) {
                    this._lastSmoothMs[deckNum] = wallElapsedSec * 1000;
                    this._lastPosTime[deckNum]  = nowMs;
                    smoothMs = Math.floor(wallElapsedSec * 1000);
                } else {
                    // Runaway clamp: if extrapolation drifts >500ms past
                    // wall (e.g. Mixxx stalled mid-play and the scrub-hold
                    // check missed it), hard-resync forward. Old +200ms-
                    // every-tick clamp caused periodic backward jumps.
                    var maxMs = Math.floor(wallElapsedSec * 1000) + 500;
                    if (smoothMs > maxMs) {
                        this._lastSmoothMs[deckNum] = wallElapsedSec * 1000;
                        this._lastPosTime[deckNum]  = nowMs;
                        smoothMs = Math.floor(wallElapsedSec * 1000);
                    }
                }
            }
            // 2026-05-29: bytes 5/6/7/8 form a co-encoded position field
            //   T = b5*60 + b6 + (b8*256 + b7) / 1024
            // where T is wall-time elapsed seconds. Validated zero-violation
            // across 51k+ transitions in 9 Windows captures (forensic audit)
            // and re-validated on macOS Serato (0.15% violations, all due to
            // legitimate loop wraps). The previous model — b7 free-running
            // at 256/sec, b8 = "eighth-note counter" — was wrong, and held
            // b8 at 0 produced the "1, 2, skip 3, 4" wave artifact because
            // firmware only saw sub-second values 0..0.249 then snapped
            // forward at every b6 increment. The earlier 6 ms quantization
            // is no longer needed; b7/b8 co-encoding gives the firmware the
            // true 10-bit sub-second cleanly.
            var totalSec = smoothMs / 1000.0;
            var sec_int  = Math.floor(totalSec);
            var sub      = totalSec - sec_int;
            // sub1024 is kept ONLY for the b21/b22 fine counters below, whose
            // rates were measured against a 1024 base. It is NOT the sub-second
            // sent to the firmware (see subMs).
            var sub1024  = Math.floor(sub * 1024);
            if (sub1024 > 1023) sub1024 = 1023;
            p[5] = Math.floor(sec_int / 60) & 0xFF;
            p[6] = (sec_int % 60) & 0xFF;
            // Sub-second field (b7 low, b8 high 2 bits) = MILLISECONDS, 0..999.
            // This is the fix for the once-per-second "flash to start": the
            // firmware reads (b8<<8|b7) as ms and treats >999 as >1 s of
            // sub-second (overflow → snap to origin). Encoding 1024ths put the
            // field at 1000..1023 for the last ~23 ms of each second. Cap at
            // 999. Full root-cause writeup in the top-of-file change log.
            var subMs = Math.floor(sub * 1000);
            if (subMs > 999) subMs = 999;
            p[7] = subMs & 0xFF;
            p[8] = (subMs >> 8) & 0x03;
            // Track-position sub-tick count used by bytes 21 and 22.
            // 1 tick = 1/1024 s of file-position. Same field as p[5..8].
            var subsecTicks = sec_int * 1024 + sub1024;
            // 2026-05-29 — Byte 21 = position-driven 8-bit accumulator at
            // exactly 1991/1024 ticks per sub-second-tick (≈ 1991/sec of
            // file-position time). BPM-independent, verified to 0.04% across
            // 3 decks at 124/129/173 BPM.
            //
            // OLD implementation used multiplier 2048/167 ≈ 12.2635 per 6ms
            // boundary, which made b21=0 land at *exactly* pos 1.002s, 2.004s,
            // 3.006s … — a perfectly periodic 1-second grid. Combined with
            // the firmware's tendency to redraw the wave window on b21=0,
            // this produced a visible "flash back to track-start" once per
            // second of position-time (user could pause exactly on it).
            //
            // The 1991/1024 ratio is irrational w.r.t. 256 → b21=0 events
            // now land at scattered sub-second positions, just like Serato's
            // capture data. Same wrap rate (~7.78 Hz, ~128.6 ms per wrap)
            // but no perfectly periodic alignment → no perceptible flash.
            //
            // Derived from the same subsec_ticks field as b8/b22 — consistent
            // position chain. (subsec_ticks * 1991) easily fits in JS's safe
            // integer range for tracks up to several hours.
            // 2026-05-29 v5 — CORRECTED RATE to 2048/sec (= ×2 of subsecTicks).
            // Forensic re-audit of flx10-serato-longplay.pcapng (deck 0x10, 66k
            // xx27 packets, 54,665-sample slope fit) showed Serato's b21 advances
            // at EXACTLY 2.00000 per tick = 2048/sec — not 1991/1024. The earlier
            // "1991/1024 avoids the 1-second flash" rationale was a misdiagnosis:
            // the flash is NOT caused by b21 hitting 0 on a 1-second grid, it's
            // caused by b21 being OUT OF PHASE with the b6 (seconds) field at each
            // whole-second boundary. 2048 = 8×256, so a 2048/sec counter returns
            // to the SAME phase every whole second (8 full wraps), staying
            // consistent with b6 — the firmware re-anchors the wave window on the
            // b6 tick and finds no discontinuity → no flash. With 1991/1024 the
            // per-second advance is 1991 → phase walks 0,199,142,85,28,… (−57/sec),
            // so every b6 tick the firmware sees a position discontinuity and
            // redraws the window: the deterministic, BPM-independent "flash one
            // frame to the beginning, once per second" the user reported.
            p[21] = (subsecTicks * 2) & 0xFF;
            // 2026-05-29 — Byte 22 is ALSO position-driven (like b8), not a
            // sixteenth-note counter. Forensic audit of 6 captures across
            // BPMs {124, 129, 173, 88, 139, 171}:
            //   * 100% sequential 0→1→…→14→0 cycle (15-state ring).
            //   * BPM-INDEPENDENT. Cycle period = 1.80 s of track-position
            //     time at every BPM (the 8 Hz rate that looked like a
            //     sixteenth-note at 124 BPM is coincidence).
            //   * Stops when track-position stops (verified across 3+ sec
            //     pauses in beat-jump capture: zero b22 changes).
            //   * Only primary decks (b31 in {0x01, 0x02}) get b22 driven;
            //     secondary decks (b31 in {0x03, 0x04}) hold b22=0.
            // Formula: b22 = floor(subsec_ticks / 123) % 15, where
            //   subsec_ticks = (b5*60 + b6) * 1024 + ((b8 & 3) << 8) + b7
            // i.e. the same 10-bit-sub-second position field used by b5..b8.
            // Each b22 step = 123/1024 s = 120.12 ms of track position.
            //
            // deckNum 1, 2 → b31 = 0x02, 0x01 (primary in our _STATE_BYTE_31
            // mapping). deckNum 3, 4 → b31 = 0x04, 0x03 (secondary).
            if (deckNum <= 2) {
                // 2026-05-29 v5 — cycle length re-measured from longplay capture
                // at 1848 ticks (b22==0 onset spacing, median over the run), i.e.
                // step = 1848/15 = 123.2 ticks, not 123. Minor (0.16%), not the
                // flash cause (b22's period is ~1.8s, not 1s), but aligned for
                // wire fidelity with Serato.
                p[22] = Math.floor(subsecTicks / 123.2) % 15;
            } else {
                p[22] = 0;
            }
            // Byte 18 = leave at 0. 6 of 7 Serato captures (including macOS
            // beat-loop capture) hold b18=0 the entire span. The one outlier
            // (parade mid-track) ticks b18 phase-locked to b22 but appears
            // tied to a Serato performance event we can't observe from USB.
            p[18] = 0;
            // 2026-05-29: tried setting b23/b24/b26/b27 to the macOS Serato
            // constants (0x65/0x13/0x10/0x1a) — even STATIC 0x1a on byte 27
            // triggered the blue beat-loop rectangle. So 0x1a alone is enough
            // to activate the loop-visual state machine; the protocol decode's
            // earlier note "byte 27 = beat-loop indicator (high nibble = state,
            // low nibble = beat-in-loop counter)" is correct, and the firmware
            // treats any non-zero b27 as "loop active". macOS Serato's 0x1a
            // must be paired with other loop-state bytes (b2 bit 3 etc.) to
            // produce the "loop inactive but in this rest state" appearance.
            // Reverted all four to 0 (zeros() default) for now.
            // Byte 23 stays at 0 (zeros() default). Confirmed always 0 across
            // 9+ Serato captures.
            //
            // 2026-05-27 DEDUP-DEFEAT INVESTIGATION RESULT: ruled out.
            // Hypothesis was that Mixxx's HidIoOutputReport cache was dropping
            // our 18% hold packets (bit-identical to predecessor) before they
            // reached hidraw, while Serato sends them through. Plan was to
            // cycle one "safe" byte every tick so packets are never identical.
            //
            // Audit of 4 Serato steady-play captures (theparade /
            // 4tracksplayingatonce / tracks_starting / play-middle-end) shows
            // only six bytes EVER vary during steady play: 5, 6, 7, 8, 18, 21,
            // 22. Every other byte (including 26, 27, 28, 50, 64, 100, …) is
            // a single constant value. There is NO firmware-blessed
            // unobserved byte we can cycle.
            //   * byte 27 → blue beat-loop rectangle (user verified).
            //   * byte 64 → no visible artifact but user reports MORE jitter,
            //     suggesting the firmware reads it silently as part of its
            //     packet-cadence clock.
            //
            // The cycling approach is dead. To get Serato-equivalent wire
            // pattern (167 Hz with explicit holds rather than our ~200 Hz
            // no-holds stream) we need to bypass Mixxx's dedup directly —
            // either patch HidIoOutputReport for our VID/PID, or move xx 27
            // off the Mixxx HID path entirely (into the daemon, like xx 36).
            // See memory:flx10-mixxx-hid-dedup.
        } else {
            p[5] = 0; p[6] = 0; p[7] = 0;
            p[21] = 0x02;
            p[22] = 0;
            p[8]  = 0;
        }
    } else {
        p[29] = 0x80;
    }
    return p;
};


// ===== REKORDBOX-MODE PACKETS (xx 3D heartbeat + xx 21 deck state) =========
// Captured from flx10-rekordbox-pause-play-scrub.pcapng (deck 1) + opening
// capture. Static bytes copied verbatim from the captured "loaded paused"
// packet; we only update [2] (play/pause), [5] (active flag), [11][12]
// (position) per Mixxx state.

// Captured byte layout for xx 21 — TWO distinct templates per deck state.
// Empty-deck (no track loaded): minimal packet, mostly zeros. Decoded from
// flx10-rekordbox-opening (deck 1, no track), bytes [0..15]:
//   10 21 00 00 20 01 00 00 80 10 00 00 00 00 00 00
// Loaded-paused: many more bytes set. Decoded from flx10-rekordbox-pause-play-scrub
// (deck 1, t=0, loaded paused), bytes [0..63]:
//   10 21 00 0a 2c 01 00 02 80 b4 00 00 00 02 00 02
//   26 32 03 00 00 7c 00 03 00 00 00 80 00 00 00 02
//   00 00 00 00 00 00 00 00 00 00 00 ff ff 00 00 00
//   00 30 00 00 00 00 00 02 00 00 01 00 85 0d 00 00
// We MUST send the empty template for non-loaded decks. Sending the loaded
// template for empty decks tells the firmware "deck loaded but garbage data"
// and the firmware falls back to a degraded display mode.

PioneerDDJFLX10Screen._XX21_LOADED_HEX = (
    "10210000" + "2c01" + "0002" + "80b40000" + "0002" + "0002" +
    "26320300" + "007c0003" + "00000080" + "00000002" +
    "00000000" + "00000000" + "0000ffff" + "00000000" +
    "30000000" + "00000200" + "00010085" + "0d000000" +
    "00000000" + "00000000" + "00000000" + "00000000" +
    "00000000" + "00000000" + "00000000" + "00000000" +
    "00000000" + "00000000" + "00000000" + "00000000"
);

PioneerDDJFLX10Screen._DECK_BYTE_XX21 = [0x10, 0x20, 0x30, 0x40];

PioneerDDJFLX10Screen._buildXX21 = function(deckNum, trackLoaded) {
    var p = this._zeros();
    var deckByte = this._DECK_BYTE_XX21[deckNum - 1];

    if (!trackLoaded) {
        // EMPTY DECK template — verbatim from rekordbox-opening capture.
        // 10 21 00 00 20 01 00 00 80 10 00 00 ...zeros
        p[0] = deckByte;
        p[1] = 0x21;
        p[4] = 0x20;
        p[5] = 0x01;
        p[8] = 0x80;
        p[9] = 0x10;
        return p;
    }

    // LOADED template — fill from hex, then override dynamic fields.
    var hex = this._XX21_LOADED_HEX;
    for (var i = 0; i < 128 && (i*2 + 2) <= hex.length; i++) {
        p[i] = parseInt(hex.substr(i*2, 2), 16);
    }
    p[0] = deckByte;

    var group = "[Channel" + deckNum + "]";
    var pos      = engine.getValue(group, "playposition");
    var duration = engine.getValue(group, "duration");
    var play     = engine.getValue(group, "play");
    var rateRatio = engine.getValue(group, "rate_ratio");
    if (!(rateRatio > 0)) rateRatio = 1.0;

    // [2] play/pause toggle: 00 playing, 02 paused
    p[2] = play > 0 ? 0x00 : 0x02;
    // [5] active flag: 0x81 when track is loaded
    p[5] = 0x81;

    // [11..12] BE16 position (rate-adjusted real elapsed seconds)
    if (duration > 0) {
        var pClamped = pos;
        if (pClamped < 0.0) pClamped = 0.0;
        if (pClamped > 1.0) pClamped = 1.0;
        var elapsedReal = duration * pClamped / rateRatio;
        var posBE16 = Math.floor(elapsedReal);
        if (posBE16 < 0) posBE16 = 0;
        if (posBE16 > 0xFFFF) posBE16 = 0xFFFF;
        p[11] = (posBE16 >> 8) & 0xFF;
        p[12] =  posBE16       & 0xFF;
    }
    return p;
};

// xx 3D heartbeat — rekordbox sends 5-frame rotation (byte[2] cycles 01..05)
// continuously. Stateless cycle counter increments each tick.
PioneerDDJFLX10Screen._heartbeatTick = 0;
PioneerDDJFLX10Screen._sendXX3DHeartbeat = function() {
    this._heartbeatTick = (this._heartbeatTick % 5) + 1;
    var p = this._zeros();
    p[0] = 0x00;
    p[1] = 0x3d;
    p[2] = this._heartbeatTick;
    p[3] = 0x00;
    p[4] = 0x05;
    this._send(p);
};

PioneerDDJFLX10Screen._sendRekordboxState = function() {
    // 1 heartbeat per tick + 1 xx 21 per loaded deck
    this._sendXX3DHeartbeat();
    for (var d = 1; d <= 4; d++) {
        var duration = engine.getValue("[Channel" + d + "]", "duration");
        this._lastDuration[d] = duration;
        var loaded = (duration > 0);
        this._send(this._buildXX21(d, loaded));
    }
};

// ===== VDJ-MODE xx 21 (captured 2026-05-24 from flx10-vdj-* captures) ======
// Same xx 21 layout as rekordbox but DIFFERENT byte values:
//   byte [2] = 0x04 playing, 0x02 paused (rekordbox: 00/02)
// Empty-deck template differs slightly from rekordbox empty.
// VDJ does NOT send xx 3D heartbeat (verified in init capture).

PioneerDDJFLX10Screen._VDJ_XX21_EMPTY_HEX = (
    // From flx10-vdj-init capture, deck 1, no track loaded (t=6.406):
    // 10 21 00 0a 04 01 00 02 00 00 0e 00 00 00 00 00
    // 00 00 00 00 00 69 60 50 fb 00 00 80 00 00 00 00 ...
    "10210000" + "0a04" + "0100" + "02000000" + "0e000000" +
    "00000000" + "00000000" + "00696050" + "fb000080" +
    "00000000" + "00000000" + "00000000" + "00000000" +
    "00000000" + "00000000" + "00000000" + "00000000" +
    "00000000" + "00000000" + "00000000" + "00000000" +
    "00000000" + "00000000" + "00000000" + "00000000" +
    "00000000" + "00000000" + "00000000" + "00000000"
);

PioneerDDJFLX10Screen._VDJ_XX21_LOADED_HEX = (
    // From flx10-vdj-track-load capture, deck 1, loaded (t=14.006):
    // 10 21 04 0a 0c 81 00 02 80 b4 0e 00 00 c0 01 05
    // 24 1a 00 00 00 7c 30 00 00 00 00 80 00 00 00 c0
    // 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    // 00 70 00 00 00 00 00 c0 01 00 03 1e 0b 0d 00 00
    "1021040a" + "0c810002" + "80b40e00" + "00c00105" +
    "241a0000" + "007c3000" + "00000080" + "000000c0" +
    "01000000" + "00000000" + "00000000" + "00000000" +
    "00700000" + "000000c0" + "0100031e" + "0b0d0000" +
    "00000000" + "00000000" + "00000000" + "00000000" +
    "00000000" + "00000000" + "00000000" + "00000000" +
    "00000000" + "00000000" + "00000000" + "00000000" +
    "00000000" + "00000000" + "00000000" + "00000000"
);

PioneerDDJFLX10Screen._buildVDJ_XX21 = function(deckNum, trackLoaded) {
    var p = this._zeros();
    var deckByte = this._DECK_BYTE_XX21[deckNum - 1];
    var hex = trackLoaded ? this._VDJ_XX21_LOADED_HEX : this._VDJ_XX21_EMPTY_HEX;
    for (var i = 0; i < 128 && (i*2 + 2) <= hex.length; i++) {
        p[i] = parseInt(hex.substr(i*2, 2), 16);
    }
    p[0] = deckByte;

    if (trackLoaded) {
        var group = "[Channel" + deckNum + "]";
        var pos      = engine.getValue(group, "playposition");
        var duration = engine.getValue(group, "duration");
        var play     = engine.getValue(group, "play");
        var rateRatio = engine.getValue(group, "rate_ratio");
        if (!(rateRatio > 0)) rateRatio = 1.0;

        // VDJ play/pause toggle: 04 playing, 02 paused
        p[2] = play > 0 ? 0x04 : 0x02;
        // [5] = 0x81 active flag (already in template)
        // [11..12] BE16 elapsed seconds, rate-adjusted to match Mixxx UI clock
        if (duration > 0) {
            var pC = pos; if (pC < 0) pC = 0; if (pC > 1) pC = 1;
            var elapsedReal = duration * pC / rateRatio;
            var pos16 = Math.floor(elapsedReal);
            if (pos16 < 0) pos16 = 0;
            if (pos16 > 0xFFFF) pos16 = 0xFFFF;
            p[11] = (pos16 >> 8) & 0xFF;
            p[12] =  pos16       & 0xFF;
        }
    }
    return p;
};

PioneerDDJFLX10Screen._sendVDJState = function() {
    // VDJ has NO xx 3D heartbeat. Just xx 21 per deck.
    for (var d = 1; d <= 4; d++) {
        var duration = engine.getValue("[Channel" + d + "]", "duration");
        this._lastDuration[d] = duration;
        var loaded = (duration > 0);
        this._send(this._buildVDJ_XX21(d, loaded));
    }
};

// EXPERIMENT FLAG: when true, ALWAYS send xx 27 as track_loaded=false even
// when Mixxx has a track loaded. Tests whether the firmware will keep
// rendering an already-uploaded xx 36 waveform even with xx 27 in empty
// state. If yes, this unlocks MIDI channel-16 to drive the deck-info text
// (BPM, time) like it does without our HID module.
// Set to false to use the proven "loaded markers => waveform renders" path
// (which also overrides MIDI ch16 text with hardcoded placeholders).
// Decision (2026-05-23): the firmware requires xx 27 to carry track-loaded
// markers in order to keep the xx 36 waveform visible. As soon as we send
// track_loaded=false, the firmware drops the waveform too (verified by
// flipping this flag to true and observing no render). So we keep it false
// here. The cost is that the firmware uses its own text fields (driven by
// our hardcoded state-5 bytes in _buildState) instead of MIDI ch16 — the
// time and BPM displays show placeholder values until we figure out the
// real encoding for the duration/BPM bytes. See FLX10-SCREEN-PROTOCOL-FINDINGS.md.
PioneerDDJFLX10Screen._FORCE_STATE_EMPTY_FOR_MIDI_TEXT = false;

// 2026-05-28 — Serato-mode dedup defeat via xx 3D heartbeat (EXPERIMENT).
// In Rekordbox mode the firmware accepts a global heartbeat packet
// `00 3d <seq> 00 05 …` at ~404 Hz with byte 2 cycling 1→2→3→4→5 (every
// packet bit-different from predecessor). Mixxx's HidIoOutputReport cache
// is keyed per-report-id; FLX10 has no report ids so all output reports
// share one cache slot. With single-deck testing, consecutive xx 27 holds
// are bit-identical → cache drops them before hidraw. Interleaving xx 3D
// breaks the dedup chain: cache holds an xx 3D when the next xx 27 arrives,
// the compare fails, the xx 27 hold reaches the wire.
//
// UNTESTED whether Serato-mode firmware accepts xx 3D. If anything glitches
// (display, wave, music) — set this flag to false (single-toggle revert).
// Possible outcomes:
//   * firmware ignores xx 3D → dedup defeated, smoother wave needle ✓
//   * display glitch → revert
//   * worse jitter (firmware reads heartbeat in unexpected way) → revert
// 2026-05-29 forensic round 6: comparison vs macOS Serato found we were
// sending xx 3D heartbeats at ~26 Hz in Serato mode, but Serato itself
// sends ZERO xx 3D in steady play (it's a rekordbox/VDJ-only packet).
// Combined with the user's timeline observation that the wave-flash
// appeared right after the b8 sub-second fix (which likely unlocked the
// firmware's "live deck" mode, making it strict about all incoming
// packets), the heartbeat is a strong flash-trigger candidate. Disabled.
PioneerDDJFLX10Screen._SERATO_SEND_HEARTBEAT = false;

// Per-deck last-sent xx27 (for the non-skipping-FIFO dedup, since all decks
// share reportID 0 and can't rely on Mixxx's per-reportID skip).
PioneerDDJFLX10Screen._lastXX27 = {1: null, 2: null, 3: null, 4: null};
PioneerDDJFLX10Screen._xx27Changed = function(deck, pkt) {
    var prev = this._lastXX27[deck];
    if (!prev || prev.length !== pkt.length) { return true; }
    for (var i = 0; i < pkt.length; i++) {
        if (pkt[i] !== prev[i]) { return true; }
    }
    return false;
};

PioneerDDJFLX10Screen._sendStateAllDecks = function() {
    // 2026-05-26 TIMER DIAGNOSTIC: log actual Hz once per second.
    // Compares to nominal 200Hz (from _STATE_MS=5). If actual << nominal,
    // either Mixxx is clamping the timer or HID send is blocking.
    var tickStart = Date.now();
    this._tickStats = this._tickStats || {n: 0, lastReport: tickStart,
                                            buildMs: 0, sendMs: 0,
                                            lastTick: tickStart,
                                            maxGap: 0, gapHist: {}};
    var s = this._tickStats;
    s.n++;
    // Track inter-tick gap
    var gap = tickStart - s.lastTick;
    s.lastTick = tickStart;
    if (gap > s.maxGap) s.maxGap = gap;
    var gapBucket = (gap <= 0) ? "0" : (gap <= 5 ? "0-5" : gap <= 10 ? "6-10" : gap <= 20 ? "11-20" : gap <= 30 ? "21-30" : ">30");
    s.gapHist[gapBucket] = (s.gapHist[gapBucket] || 0) + 1;
    if (tickStart - s.lastReport >= 1000) {
        var elapsedMs = tickStart - s.lastReport;
        var hz = (s.n * 1000.0 / elapsedMs).toFixed(1);
        var avgBuild = (s.buildMs / s.n).toFixed(2);
        var avgSend = (s.sendMs / s.n).toFixed(2);
        var histStr = "";
        var buckets = ["0-5","6-10","11-20","21-30",">30"];
        for (var i = 0; i < buckets.length; i++) {
            histStr += " " + buckets[i] + "ms=" + (s.gapHist[buckets[i]] || 0);
        }
        console.log("FLX10_TICK_RATE " + hz + " Hz over " + s.n + " ticks in " +
                    elapsedMs + "ms (avg build=" + avgBuild + "ms send=" + avgSend +
                    "ms, maxGap=" + s.maxGap + "ms," + histStr + ")");
        s.n = 0;  s.lastReport = tickStart;  s.buildMs = 0;  s.sendMs = 0;
        s.maxGap = 0;  s.gapHist = {};
    }
    // Dedup-defeat: send xx 3D heartbeat once per tick BEFORE the deck loop.
    // Each heartbeat has byte 2 cycling 1..5 so it's never bit-identical to
    // its predecessor. With it in the cache, the next xx 27 (even a hold)
    // necessarily differs from the cache → reaches hidraw. See the comment
    // on _SERATO_SEND_HEARTBEAT for the full rationale + revert path.
    if (this._SERATO_SEND_HEARTBEAT) {
        this._sendXX3DHeartbeat();
    }
    // xx27 for every loaded deck, every tick, via the NON-SKIPPING FIFO.
    // All decks share HID reportID 0, so the default skipping path keeps only
    // the LAST deck per IO cycle (others superseded → choppy with 2+ decks).
    // The FIFO preserves each distinct packet, so every deck scrolls
    // independently at the full tick rate (200Hz). Per-deck dedup means an
    // idle/paused deck (identical xx27) costs no bandwidth. xx27 is NEVER capped
    // — 4 decks × 200Hz = 800/s stays under the ~1000/s hidraw drain, so live
    // play/scrub/loop/reverse-slip on all decks stays smooth.
    var buildStart = Date.now();
    var sent = 0;
    for (var d = 1; d <= 4; d++) {
        var duration = engine.getValue("[Channel" + d + "]", "duration");
        this._lastDuration[d] = duration;
        if (!(duration > 0)) { continue; }
        if (this._FORCE_STATE_EMPTY_FOR_MIDI_TEXT) { continue; }
        var pkt = this._buildState(this._DECK_BYTE[d - 1], true);
        if (this._xx27Changed(d, pkt)) {
            this._lastXX27[d] = pkt;
            controller.send(pkt, 0, 0, true);
            sent++;
        }
    }
    var afterBuild = Date.now();
    // Pace the bulk waveform upload into whatever bandwidth this tick has left.
    // This is what keeps the 32-slot FIFO from overflowing: total enqueues/tick
    // are capped at _TICK_BUDGET (≈1000/s), with xx27 taking priority and the
    // bulk getting the remainder. A fresh load fills in over a few seconds while
    // other decks keep playing at full rate — no dropped xx27 frames.
    var hadBulk = this._txQueue.length > 0;
    while (sent < this._TICK_BUDGET && this._txQueue.length > 0) {
        controller.send(this._txQueue.shift(), 0, 0, true);
        sent++;
    }
    if (hadBulk && this._txQueue.length === 0) {
        console.log("FLX10 screen: upload queue drained");
    }
    // Then EQ-refresh packets (lower priority than the initial bulk), drained
    // round-robin across decks so no deck's EQ window starves another's.
    for (var rstep = 0; rstep < 4 && sent < this._TICK_BUDGET; rstep++) {
        var rd = ((this._refreshRR + rstep) % 4) + 1;
        var rq = this._refreshQueue[rd];
        while (sent < this._TICK_BUDGET && rq.length > 0) {
            controller.send(rq.shift(), 0, 0, true);
            sent++;
        }
    }
    this._refreshRR = (this._refreshRR + 1) % 4;
    var afterSend = Date.now();
    s.buildMs += (afterBuild - buildStart);
    s.sendMs += (afterSend - afterBuild);
};
// Per-tick HID send budget. _STATE_MS=5ms × 5 = ~1000 reports/s, matching the
// measured hidraw drain rate. Lower this if FIFO overflows persist.
PioneerDDJFLX10Screen._TICK_BUDGET = 5;

// Event-driven scheduler: called whenever playposition / rate_ratio changes.
// Throttles to _SEND_MIN_GAP_MS to avoid flooding Mixxx's HID write queue.
PioneerDDJFLX10Screen._lastSendTime = 0;
PioneerDDJFLX10Screen._scheduleStateUpdate = function() {
    var now = Date.now();
    if (now - this._lastSendTime >= this._SEND_MIN_GAP_MS) {
        this._lastSendTime = now;
        this._sendStateAllDecks();
    }
};


// ===== xx 30 — per-deck init (8 enable-flags at [10,16,22,28,34,40,46,52]) =
PioneerDDJFLX10Screen._sendInit30 = function(deck, durationSec) {
    var p = this._zeros();
    p[0] = this._DECK_BYTE[deck - 1];
    p[1] = 0x30;
    p[2] = 0x01;
    p[4] = 0x01;
    // Track length at [6..9]: min, sec, ms(LE16). No 0xff trailer — matches
    // Serato (the stray 0xff pattern selected a redraw-prone firmware mode).
    if (durationSec > 0) {
        var totalMs = Math.round(durationSec * 1000);
        var minutes = Math.floor(totalMs / 60000);
        var remMs   = totalMs % 60000;
        var seconds = Math.floor(remMs / 1000);
        var ms      = remMs % 1000;
        p[6] = minutes & 0xff;
        p[7] = seconds & 0xff;
        p[8] = ms & 0xff;
        p[9] = (ms >> 8) & 0xff;
    }
    this._sendRaw(p);
};


// ===== xx 35 — total waveform entry count (LE16 at [2][3]), 3 packets =======
PioneerDDJFLX10Screen._sendInit35 = function(deck, nEntries) {
    var db = this._DECK_BYTE[deck - 1];
    for (var i = 0; i < 3; i++) {
        var p = this._zeros();
        p[0] = db;
        p[1] = 0x35;
        p[2] = nEntries & 0xff;
        p[3] = (nEntries >> 8) & 0xff;
        this._sendRaw(p);
    }
};

// ===== xx 39 — HOT CUE display setup (3 fixed packets, deck byte varies) =====
PioneerDDJFLX10Screen._sendXX39 = function(deck) {
    var db = this._DECK_BYTE[deck - 1];
    for (var i = 0; i < this._XX39_HEX.length; i++) {
        var hex = this._XX39_HEX[i];
        var p = this._zeros();
        for (var j = 0; j * 2 < hex.length && j < 128; j++) {
            p[j] = parseInt(hex.substr(j * 2, 2), 16);
        }
        p[0] = db;
        this._sendRaw(p);
    }
};


// ===== xx 39 — pad-mode labels ("HOT CUE"), 3 packets per deck =============
// Hardcoded byte-for-byte from Serato capture; only byte[0] (deck) varies.
PioneerDDJFLX10Screen._XX39_HEX = [
    "10390100030000484f54204355450000000000000000000000000000000000000000000000003f000000000000000000000000000000000000000000000000000000000000023f000000000000000000000000000000000000000000000000000000000000023f00000000000000000000000000000000000000000000000000",
    "1039020003000000000000023f000000000000000000000000000000000000000000000000000000000000023f000000000000000000000000000000000000000000000000000000000000023f000000000000000000000000000000000000000000000000000000000000023f00000000000000000000000000000000000000",
    "1039030003000000000000000000000000023f00000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
];

PioneerDDJFLX10Screen._sendInit39 = function(deck) {
    var db = this._DECK_BYTE[deck - 1];
    for (var s = 0; s < 3; s++) {
        var hex = this._XX39_HEX[s];
        var p = [];
        for (var i = 0; i < 128; i++) {
            p[i] = parseInt(hex.substr(i * 2, 2), 16);
        }
        p[0] = db;
        this._send(p);
    }
};


// ===== xx 2f — cue-data placeholder (one empty packet per deck) ============
PioneerDDJFLX10Screen._sendInit2f = function(deck) {
    var p = this._zeros();
    p[0] = this._DECK_BYTE[deck - 1];
    p[1] = 0x2f;
    p[2] = 0x01;
    p[4] = 0x01;
    this._send(p);
};


// ===== xx 33 — album-art JPEG upload =======================================
// Hardcoded 240×240 red JPEG (1529 bytes).  In a future iteration this would
// be replaced with real cover-art bytes per track.
PioneerDDJFLX10Screen._TEST_JPEG_HEX = (
    "ffd8ffe000104a46494600010100000100010000ffdb0043000a07070807060a" +
    "0808080b0a0a0b0e18100e0d0d0e1d15161118231f2524221f2221262b372f26" +
    "293429212230413134393b3e3e3e252e4449433c48373d3e3bffdb0043010a0b" +
    "0b0e0d0e1c10101c3b2822283b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b" +
    "3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3b3bffc0" +
    "00110800f000f003012200021101031101ffc4001f0000010501010101010100" +
    "000000000000000102030405060708090a0bffc400b510000201030302040305" +
    "0504040000017d01020300041105122131410613516107227114328191a10823" +
    "42b1c11552d1f02433627282090a161718191a25262728292a3435363738393a" +
    "434445464748494a535455565758595a636465666768696a737475767778797a" +
    "838485868788898a92939495969798999aa2a3a4a5a6a7a8a9aab2b3b4b5b6b7" +
    "b8b9bac2c3c4c5c6c7c8c9cad2d3d4d5d6d7d8d9dae1e2e3e4e5e6e7e8e9eaf1" +
    "f2f3f4f5f6f7f8f9faffc4001f01000301010101010101010100000000000001" +
    "02030405060708090a0bffc400b5110002010204040304070504040001027700" +
    "0102031104052131061241510761711322328108144291a1b1c109233352f015" +
    "6272d10a162434e125f11718191a262728292a35363738393a43444546474849" +
    "4a535455565758595a636465666768696a737475767778797a82838485868788" +
    "898a92939495969798999aa2a3a4a5a6a7a8a9aab2b3b4b5b6b7b8b9bac2c3c4" +
    "c5c6c7c8c9cad2d3d4d5d6d7d8d9dae2e3e4e5e6e7e8e9eaf2f3f4f5f6f7f8f9" +
    "faffda000c03010002110311003f00e568a28af9d3f660a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a2" +
    "8a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2800a28a2803ffd9");

PioneerDDJFLX10Screen._uploadAlbumArt = function(deck) {
    var hex = this._TEST_JPEG_HEX;
    var db = this._DECK_BYTE[deck - 1];
    var jpegSize = hex.length / 2;
    var SEG1_CAP = 119;
    var SEG_CAP  = 122;
    var totalSegs = (jpegSize <= SEG1_CAP) ? 1
                    : 1 + Math.ceil((jpegSize - SEG1_CAP) / SEG_CAP);

    var hexAt = function(i) { return parseInt(hex.substr(i * 2, 2), 16); };
    var pos = 0;
    for (var seg = 1; seg <= totalSegs; seg++) {
        var p = this._zeros();
        p[0] = db;
        p[1] = 0x33;
        p[2] = seg;
        p[4] = totalSegs;
        if (seg === 1) {
            p[6] = jpegSize & 0xff;
            p[7] = (jpegSize >> 8) & 0xff;
            var take = Math.min(SEG1_CAP, jpegSize - pos);
            for (var j = 0; j < take; j++) { p[9 + j] = hexAt(pos + j); }
            pos += take;
        } else {
            var take2 = Math.min(SEG_CAP, jpegSize - pos);
            for (var k = 0; k < take2; k++) { p[6 + k] = hexAt(pos + k); }
            pos += take2;
        }
        this._sendRaw(p);
    }
};


// ===== xx 36 — PWV5 waveform upload ========================================
// Test pattern: bright solid red, max height across the whole track length.
// To swap in real Mixxx waveform data: replace _generateEntries with a function
// that returns LE16 PWV5 bytes derived from the loaded track.
// Build PWV5 entry bytes from Mixxx's native in-memory waveform (via the
// engine.getWaveformSummary API). Returns LE16 bytes per entry, or null if the
// waveform isn't loaded yet (caller should retry). PWV5 packing: low→R(3b),
// mid→G(3b), high→B(3b), all→height(5b) → (R<<13|G<<10|B<<7|H<<2).
PioneerDDJFLX10Screen._generateEntries = function(deck, durationSec) {
    var n = Math.floor(durationSec * this._PWV5_FPS);
    if (n <= 0) { return null; }
    if (typeof engine.getWaveformSummary !== "function") {
        return null;   // running on a Mixxx without the native API
    }
    var wf = engine.getWaveformSummary("[Channel" + deck + "]", n);
    if (!wf || !wf.length) { return null; }   // not analysed / not loaded yet
    this._wfBands[deck] = wf;   // raw [all,low,mid,high] cache for the EQ refresh
    var bins = Math.floor(wf.length / 4);
    var out = new Array(bins * 2);
    for (var i = 0; i < bins; i++) {
        var v = this._packBin(wf[i * 4], wf[i * 4 + 1],
            wf[i * 4 + 2], wf[i * 4 + 3]);
        out[i * 2]     = v & 0xff;
        out[i * 2 + 1] = (v >> 8) & 0xff;
    }
    return out;
};
PioneerDDJFLX10Screen._PWV5_FPS = 150;   // Pioneer/Serato canonical fps

// Waveform visual style. "rgb" = Mixxx RGB (low→R, mid→G, high→B). "filtered"
// = Mixxx Filtered-style 2-tone (bass warms the bar, treble brightens it).
PioneerDDJFLX10Screen._WAVE_STYLE = "blue";   // "rgb" | "filtered" | "blue" | "3band"

// Pack one waveform bin (band amplitudes 0-255) into a PWV5 LE16 value per the
// selected style. Height always comes from `all`. (Phase B will scale the band
// amplitudes by the live EQ/filter gains before this is called.)
PioneerDDJFLX10Screen._packBin = function(all, low, mid, high) {
    var h = (all * 31 / 255) | 0;
    if (h > 31) { h = 31; }
    var r, g, b;
    if (this._WAVE_STYLE === "blue") {
        // Pioneer/CDJ "blue" family with clear band separation along a
        // blue -> cyan -> white ramp:
        //   low-heavy  -> deep blue (0,0,7)
        //   mid-heavy  -> cyan      (0,g,7)
        //   high-heavy -> white     (r,g,7)
        b = 7;                              // blue body (always lit)
        g = ((mid + high) * 7 / 255) | 0;   // mids AND treble raise green
        r = (high * 7 / 255) | 0;           // treble alone adds red -> white tips
    } else if (this._WAVE_STYLE === "filtered") {
        var bias = high - low;
        g = 4 + (bias > 96 ? 1 : (bias < -96 ? -1 : 0));
        if (g < 2) { g = 2; }
        if (g > 6) { g = 6; }
        r = 7;
        b = 0;
    } else if (this._WAVE_STYLE === "3band") {
        // rekordbox-style 3-band. Anchor colors (0..7 space):
        //   low  -> blue   (0, 0, 7)
        //   mid  -> orange (7, 3, 0)
        //   high -> white  (7, 7, 7)
        // rekordbox stacks these vertically; we get one color per column, so
        // we amplitude-blend them (ratio = hue, height still carries amplitude).
        var sum = low + mid + high;
        if (sum < 1) { sum = 1; }                 // silence guard (div-by-zero)
        r = ((mid * 7 + high * 7) / sum) | 0;
        g = ((mid * 3 + high * 7) / sum) | 0;
        b = ((low * 7 + high * 7) / sum) | 0;
    } else {
        // RGB: low -> R, mid -> G, high -> B (Mixxx RGB waveform).
        r = (low * 7 / 255) | 0;
        g = (mid * 7 / 255) | 0;
        b = (high * 7 / 255) | 0;
    }
    if (r > 7) { r = 7; }
    if (g > 7) { g = 7; }
    if (b > 7) { b = 7; }
    return (r << 13) | (g << 10) | (b << 7) | (h << 2);
};

PioneerDDJFLX10Screen._uploadWaveform = function(deck, durationSec, entryBytes) {
    var db = this._DECK_BYTE[deck - 1];
    var nEntries = entryBytes.length / 2;
    var ENTRIES_PER_PKT = 19;
    var group = "[Channel" + deck + "]";
    this._entries[deck] = entryBytes;   // cache for the playhead trickle

    // FULL Serato-style track-load sequence — the firmware only arms the
    // track-loaded display when it receives ALL of these; partial sequences
    // (xx30/35/36 alone) don't trigger the wave. Mirrors the daemon exactly:
    //   xx30 (length) → xx39 (hot-cue) → xx33 (album art) → xx35 (count)
    //   → xx36 (waveform) → xx36 re-park → xx2f (beat grid)
    this._sendInit30(deck, durationSec);
    this._sendXX39(deck);
    this._uploadAlbumArt(deck);
    // xx35 count = duration × 150 (Serato's pattern), not the clamped entry count.
    this._sendInit35(deck, Math.round(durationSec * this._PWV5_FPS));

    var pos = 0;
    while (pos < nEntries) {
        var take = Math.min(ENTRIES_PER_PKT, nEntries - pos);
        var p = this._zeros();
        p[0]  = db;
        p[1]  = 0x36;
        p[2]  = 0x00;     // match the working daemon (0x01 = wrong chunk mode)
        p[4]  = 0x00;
        p[6]  = 0x13;
        p[10] =  pos        & 0xff;
        p[11] = (pos >> 8)  & 0xff;
        p[12] = (pos >> 16) & 0xff;
        p[13] = (pos >> 24) & 0xff;
        for (var j = 0; j < take * 2; j++) {
            p[14 + j] = entryBytes[pos * 2 + j];
        }
        this._sendRaw(p);
        pos += take;
    }

    // Re-park the firmware playhead at the REAL position — the bulk fill leaves
    // its counter at the track end, so the display would otherwise show the end.
    var ppos = engine.getValue(group, "playposition");
    if (!(ppos >= 0)) { ppos = 0; }
    var park = Math.floor(ppos * nEntries);
    if (park < 0) { park = 0; }
    if (park > nEntries - ENTRIES_PER_PKT) {
        park = Math.max(0, nEntries - ENTRIES_PER_PKT);
    }
    this._sendScrollUpdate(deck, park, entryBytes);

    // xx2f beat grid (real musical beats from Mixxx, or dense interpolation
    // grid — see _BEATGRID_MODE).
    this._sendBeatGrid(deck, durationSec);

    console.log("FLX10 screen: native waveform queued deck " + deck +
                " (" + nEntries + " entries, " + this._txQueue.length +
                " packets) — bulk paced into spare bandwidth, budget " +
                this._TICK_BUDGET + "/tick");
};


// ===== xx 36 re-park — single packet at the current playhead position =======
// direct=true bypasses the upload queue (used by the low-rate trickle, which
// must not be stuck behind a bulk upload and is too small to overflow the FIFO).
PioneerDDJFLX10Screen._sendScrollUpdate = function(deck, posEntries, entryBytes, direct) {
    if (!entryBytes || posEntries < 0) { return; }
    var nEntries = entryBytes.length / 2;
    var take = Math.min(19, nEntries - posEntries);
    if (take <= 0) { return; }
    var p = this._zeros();
    p[0]  = this._DECK_BYTE[deck - 1];
    p[1]  = 0x36;
    p[2]  = 0x00;
    p[4]  = 0x00;
    p[6]  = 0x13;
    p[10] =  posEntries        & 0xff;
    p[11] = (posEntries >> 8)  & 0xff;
    p[12] = (posEntries >> 16) & 0xff;
    p[13] = (posEntries >> 24) & 0xff;
    for (var j = 0; j < take * 2; j++) {
        p[14 + j] = entryBytes[posEntries * 2 + j];
    }
    if (direct) {
        this._lastSentPkt = p;
        controller.send(p, 0, 0, true);
    } else {
        this._sendRaw(p);
    }
};


// ===== Playhead trickle — port of the daemon's RefreshThread ================
// Continuously re-sends one xx36 at the live playhead for each loaded deck.
// This (a) keeps the firmware from dropping the waveform (~1 min without it),
// (b) moves the wave needle during playback, and (c) re-asserts the wave so a
// deck that didn't latch on first upload recovers. Skipped while a bulk upload
// is draining (queue non-empty) to avoid FIFO contention.
// Trickle disabled for now: it uses Mixxx's priority (non-skipping) FIFO, which
// preempts the skipping-path 200Hz xx27 that actually drives the wave scroll →
// the wave stalls as soon as the bulk drain ends. xx27 alone scrolls the wave
// (this is what the daemon relied on); the firmware may drop the wave after
// ~1 min without a refresh, which we'll re-solve with a non-preempting refresh.
PioneerDDJFLX10Screen._TRICKLE_ENABLE   = false;
PioneerDDJFLX10Screen._entries          = {1: null, 2: null, 3: null, 4: null};
PioneerDDJFLX10Screen._trickleTimer     = 0;
PioneerDDJFLX10Screen._TRICKLE_MS       = 125;   // 8 Hz — matches the daemon's
                                                 // proven-safe RefreshThread rate
PioneerDDJFLX10Screen._lastTrickleEntry = {1: -1, 2: -1, 3: -1, 4: -1};

PioneerDDJFLX10Screen._startTrickle = function() {
    if (!this._TRICKLE_ENABLE) { return; }
    if (this._trickleTimer) { return; }
    var self = this;
    this._trickleTimer = engine.beginTimer(this._TRICKLE_MS, function() {
        if (self._txQueue.length > 0) { return; }   // bulk upload in progress
        for (var d = 1; d <= 4; d++) {
            var ent = self._entries[d];
            if (!ent) { continue; }
            var group = "[Channel" + d + "]";
            if (!(engine.getValue(group, "duration") > 0)) {
                self._entries[d] = null;            // track unloaded
                self._lastTrickleEntry[d] = -1;
                continue;
            }
            var nEntries = ent.length / 2;
            var ppos = engine.getValue(group, "playposition");
            if (!(ppos >= 0)) { ppos = 0; }
            var park = Math.floor(ppos * nEntries);
            if (park < 0) { park = 0; }
            if (park > nEntries - 19) { park = Math.max(0, nEntries - 19); }
            // DEDUP (daemon's key fix): only send when the playhead entry has
            // actually advanced. When paused/cued the entry is constant → zero
            // xx36 traffic → xx27 flows unimpeded. This is what stopped the
            // daemon's xx27-stall hang.
            if (park === self._lastTrickleEntry[d]) { continue; }
            self._lastTrickleEntry[d] = park;
            self._sendScrollUpdate(d, park, ent, true);
        }
    });
};


// ===== EQ-reactive waveform (Phase B) ======================================
// Re-pack + re-upload the visible window around each deck's playhead with the
// live 3-band EQ applied, matching Mixxx's WaveformRendererRGB exactly:
//   lowGain/midGain/highGain = EQ parameter1/2/3 (gated by filterWaveformEnable,
//   forced to 0 by the kill buttons); eqGain = (Σ band·gain)/(Σ band) scales the
//   bar HEIGHT, and the per-band gains scale the COLOR. No QuickEffect filter
//   knob (Mixxx's wave doesn't react to it). Only the small jog-visible window
//   is refreshed, and only when the EQ changes / the playhead moves / a periodic
//   keep-alive is due — so an idle-EQ deck costs ~nothing and this also prevents
//   the firmware's ~1-min wave drop. Enqueued through the budgeted sender so it
//   never preempts xx27.
PioneerDDJFLX10Screen._EQ_REACTIVE    = true;
PioneerDDJFLX10Screen._EQ_WINDOW      = 1500;   // base entries each side at zoom=1
                                                // (~10s @150fps). Scaled by the live
                                                // waveform_zoom so it always covers
                                                // the visible jog span (the jog
                                                // zooms with waveform_zoom);
                                                // otherwise zoomed-out edges stay
                                                // un-EQ'd.
PioneerDDJFLX10Screen._EQ_WINDOW_MAX  = 6000;   // cap (~+-40s) to bound refresh cost
PioneerDDJFLX10Screen._EQ_REFRESH_MS  = 50;     // poll EQ/playhead/zoom at 20Hz
PioneerDDJFLX10Screen._EQ_KEEPALIVE_MS = 8000;  // re-assert wave even if idle
PioneerDDJFLX10Screen._wfBands        = {1: null, 2: null, 3: null, 4: null};
PioneerDDJFLX10Screen._lastEqKey      = {1: "", 2: "", 3: "", 4: ""};
PioneerDDJFLX10Screen._lastWinCenter  = {1: -1, 2: -1, 3: -1, 4: -1};
PioneerDDJFLX10Screen._lastEqRefresh  = {1: 0, 2: 0, 3: 0, 4: 0};
PioneerDDJFLX10Screen._refreshTimer   = 0;
// EQ-refresh packets live in their own PER-DECK queues (separate from the
// track-load bulk _txQueue) so a live knob sweep can DISCARD that deck's stale
// pending window and replace it with the latest each tick — the needle always
// shows the current EQ instead of draining hundreds of out-of-date frames
// first. Per-deck so sweeping one deck never drops another deck's EQ frames.
// Drained after the bulk, round-robin, within the same per-tick budget.
PioneerDDJFLX10Screen._refreshQueue   = {1: [], 2: [], 3: [], 4: []};
PioneerDDJFLX10Screen._refreshRR      = 0;

// Live EQ band multipliers for a deck (1.0 = unity), matching Mixxx getGains().
PioneerDDJFLX10Screen._eqGains = function(deck) {
    var ch = "[Channel" + deck + "]";
    if (!(engine.getValue(ch, "filterWaveformEnable") > 0)) {
        return {low: 1, mid: 1, high: 1};   // EQ→waveform disabled: flat
    }
    var eq = "[EqualizerRack1_[Channel" + deck + "]_Effect1]";
    var low  = engine.getValue(eq, "parameter1");
    var mid  = engine.getValue(eq, "parameter2");
    var high = engine.getValue(eq, "parameter3");
    if (engine.getValue(eq, "button_parameter1") > 0) { low = 0; }
    if (engine.getValue(eq, "button_parameter2") > 0) { mid = 0; }
    if (engine.getValue(eq, "button_parameter3") > 0) { high = 0; }
    return {low: low, mid: mid, high: high};
};

// Enqueue xx36 packets for entries [start, start+count) with EQ gains applied.
PioneerDDJFLX10Screen._enqueueWindow = function(deck, start, count, g, center) {
    var wf = this._wfBands[deck];
    if (!wf) { return; }
    var total = Math.floor(wf.length / 4);
    if (start < 0) { start = 0; }
    var end = start + count;
    if (end > total) { end = total; }
    if (end <= start) { return; }
    if (center === undefined) { center = (start + end) / 2; }
    var db = this._DECK_BYTE[deck - 1];
    // Packet start positions tiling [start,end), ordered CENTER-OUT (nearest the
    // playhead/needle first). The eye tracks the needle, so refreshing it first
    // makes an EQ change feel instant even while the window edges trail by a few
    // hundred ms as they drain through the budget.
    var positions = [];
    for (var p0 = start; p0 < end; p0 += 19) { positions.push(p0); }
    positions.sort(function(a, b) {
        return Math.abs(a + 9 - center) - Math.abs(b + 9 - center);
    });
    for (var pi = 0; pi < positions.length; pi++) {
        var pos = positions[pi];
        var take = Math.min(19, end - pos);
        var p = this._zeros();
        p[0] = db;  p[1] = 0x36;  p[2] = 0x00;  p[4] = 0x00;  p[6] = 0x13;
        p[10] =  pos        & 0xff;
        p[11] = (pos >> 8)  & 0xff;
        p[12] = (pos >> 16) & 0xff;
        p[13] = (pos >> 24) & 0xff;
        for (var j = 0; j < take; j++) {
            var idx  = (pos + j) * 4;
            var all  = wf[idx];
            var low  = wf[idx + 1];
            var mid  = wf[idx + 2];
            var high = wf[idx + 3];
            var sum  = low + mid + high;
            var lowS = low * g.low, midS = mid * g.mid, highS = high * g.high;
            var eqGain = sum > 0 ? (lowS + midS + highS) / sum : 1;
            var v = this._packBin(all * eqGain, lowS, midS, highS);
            p[14 + j * 2]     = v & 0xff;
            p[14 + j * 2 + 1] = (v >> 8) & 0xff;
        }
        this._refreshQueue[deck].push(p);
    }
};

PioneerDDJFLX10Screen._startEqRefresh = function() {
    if (this._refreshTimer) { return; }
    var self = this;
    this._refreshTimer = engine.beginTimer(this._EQ_REFRESH_MS, function() {
        var now = Date.now();
        for (var d = 1; d <= 4; d++) {
            var wf = self._wfBands[d];
            if (!wf) { continue; }
            var group = "[Channel" + d + "]";
            if (!(engine.getValue(group, "duration") > 0)) {
                self._wfBands[d] = null;   // track unloaded
                self._refreshQueue[d] = [];
                continue;
            }
            var total = Math.floor(wf.length / 4);
            var g = self._eqGains(d);
            // Scale the window to the live zoom so it covers the whole visible
            // jog span (the jog zooms with waveform_zoom; zoom 1 = most zoomed
            // in). Without this the zoomed-out edges stay un-EQ'd.
            var zoom = engine.getValue(group, "waveform_zoom");
            if (!(zoom >= 1)) { zoom = 1; }
            var win = Math.round(self._EQ_WINDOW * zoom);
            if (win > self._EQ_WINDOW_MAX) { win = self._EQ_WINDOW_MAX; }
            var eqKey = g.low + "," + g.mid + "," + g.high + "," + zoom;
            var ppos = engine.getValue(group, "playposition");
            if (!(ppos >= 0)) { ppos = 0; }
            var center = Math.floor(ppos * total);
            var eqChanged = eqKey !== self._lastEqKey[d];
            var moved = Math.abs(center - self._lastWinCenter[d]) > (win / 2);
            var keepalive = (now - self._lastEqRefresh[d]) > self._EQ_KEEPALIVE_MS;
            if (eqChanged || moved || keepalive) {
                // Replace this deck's pending window with a fresh center-out one
                // for the latest EQ — stale frames from a prior tick are dropped
                // so a live sweep tracks the knob instead of lagging behind them.
                self._refreshQueue[d] = [];
                self._enqueueWindow(d, center - win, win * 2, g, center);
                self._lastEqKey[d] = eqKey;
                self._lastWinCenter[d] = center;
                self._lastEqRefresh[d] = now;
            }
            // Live beatgrid: re-send xx2f when the grid is edited in Mixxx. The
            // grid signature is playhead-independent, so it only changes on an
            // actual edit (drag / bpm change) — matching Serato's live re-send.
            var prm = self._beatGridParams(d, engine.getValue(group, "duration"));
            if (prm && self._beatSig(prm) !== self._lastBeatSig[d]) {
                self._sendBeatGrid(d, engine.getValue(group, "duration"));
            }
        }
    });
};


// ===== xx 2f — musical beatgrid (Serato format, decoded from captures) ======
// One record per BEAT in a CONTINUOUS byte stream (records straddle packet
// boundaries): [beat-count LE16] then, per beat, [btype, LE24 position-in-
// MILLISECONDS]. The stream is chunked into 122-byte payloads at packet bytes
// 6..127. Header byte[4] = total packet count. btype: beat 0 = 0x01 (track
// start), then [0x00,0x02,0x03,0x04][i%4] — 0x00 marks the bar downbeat every
// 4 beats.
PioneerDDJFLX10Screen._XX2F_PAYLOAD = 122;
PioneerDDJFLX10Screen._XX2F_BAR     = [0x00, 0x02, 0x03, 0x04];
PioneerDDJFLX10Screen._XX2F_INTERVAL_MS = 16.0;   // dense-fallback grid only

// "beats" = real musical beats from Mixxx's beatgrid; "dense" = uniform 16ms
// interpolation grid (legacy fallback).
PioneerDDJFLX10Screen._BEATGRID_MODE = "beats";

PioneerDDJFLX10Screen._emitXX2F = function(deck, beatTimesSec, downbeatSec, beatLenSec) {
    var n = beatTimesSec.length;
    if (n === 0) { return; }
    // Build the continuous stream: [countLE16] + per beat [btype, msLE24].
    // btype: beat 0 = 0x01 (track-start marker); otherwise the bar position
    // relative to the real downbeat — 0x00 on the downbeat (every 4 beats),
    // else 0x02/0x03/0x04. So the bar line lands on the actual musical bar-1.
    var stream = [n & 0xff, (n >> 8) & 0xff];
    for (var i = 0; i < n; i++) {
        var bt;
        if (i === 0) {
            bt = 0x01;
        } else {
            var barIdx = Math.round((beatTimesSec[i] - downbeatSec) / beatLenSec);
            bt = this._XX2F_BAR[((barIdx % 4) + 4) % 4];
        }
        var ms = Math.round(beatTimesSec[i] * 1000) & 0xffffff;
        stream.push(bt, ms & 0xff, (ms >> 8) & 0xff, (ms >> 16) & 0xff);
    }
    // Chunk the stream into 122-byte payloads at packet bytes 6..127.
    var db = this._DECK_BYTE[deck - 1];
    var pay = this._XX2F_PAYLOAD;
    var totalSegs = Math.max(1, Math.ceil(stream.length / pay));
    for (var seg = 0; seg < totalSegs; seg++) {
        var p = this._zeros();
        p[0] = db;
        p[1] = 0x2f;
        p[2] = (seg + 1) & 0xff;
        p[4] = totalSegs & 0xff;
        var base = seg * pay;
        for (var k = 0; k < pay && base + k < stream.length; k++) {
            p[6 + k] = stream[base + k];
        }
        this._sendRaw(p);
    }
};

// Beatgrid params from Mixxx's real beatgrid: {beatLenSec, downbeatSec} (exact
// spacing + true downbeat anchor) via the getBeatInfo fork API, or a fallback
// from file_bpm + beat_closest (no true downbeat). Playhead-independent, so its
// signature only changes when the grid is edited.
PioneerDDJFLX10Screen._lastBeatSig = {1: "", 2: "", 3: "", 4: ""};
PioneerDDJFLX10Screen._beatGridParams = function(deck, durationSec) {
    var group = "[Channel" + deck + "]";
    if (typeof engine.getBeatInfo === "function") {
        var info = engine.getBeatInfo(group);
        if (info && info.length >= 2 && info[0] > 0) {
            return {beatLenSec: info[0], downbeatSec: info[1]};
        }
    }
    var fileBpm = engine.getValue(group, "file_bpm");
    if (!(fileBpm > 0)) { return null; }
    var trackSamples = engine.getValue(group, "track_samples");
    var beatClosest = engine.getValue(group, "beat_closest");
    if (!(trackSamples > 0) || !(beatClosest > 0)) { return null; }
    return {
        beatLenSec: 60.0 / fileBpm,
        downbeatSec: (beatClosest / trackSamples) * durationSec,
    };
};
PioneerDDJFLX10Screen._beatSig = function(prm) {
    return prm.beatLenSec.toFixed(5) + ":" + prm.downbeatSec.toFixed(4);
};

PioneerDDJFLX10Screen._sendBeatGrid = function(deck, durationSec) {
    if (!(durationSec > 0)) { return; }
    var prm = this._beatGridParams(deck, durationSec);
    if (!prm) { this._lastBeatSig[deck] = ""; return; }
    var beatLenSec = prm.beatLenSec;
    var downbeatSec = prm.downbeatSec;
    // First beat >= 0 (grid phase), then every beat to end of track.
    var firstBeatSec = downbeatSec -
        Math.floor(downbeatSec / beatLenSec) * beatLenSec;
    var times = [];
    for (var t = firstBeatSec; t < durationSec; t += beatLenSec) {
        if (t >= -1e-6) { times.push(t); }
    }
    this._lastBeatSig[deck] = this._beatSig(prm);
    console.log("FLX10 screen: beatgrid deck " + deck + " beats=" + times.length +
        " beatLen=" + beatLenSec.toFixed(4) + "s downbeat=" +
        downbeatSec.toFixed(3) + "s");
    this._emitXX2F(deck, times, downbeatSec, beatLenSec);
};


// ===== Track-load handler ===================================================
// NATIVE waveform: when true, screen.js uploads the waveform itself using the
// engine.getWaveformSummary() API (no external daemon). Set false to defer the
// xx30/35/36 upload to flx10_screen_daemon.py.
PioneerDDJFLX10Screen._NATIVE_WAVEFORM = true;
PioneerDDJFLX10Screen._wfPollTimer = {1: 0, 2: 0, 3: 0, 4: 0};

PioneerDDJFLX10Screen._onTrackLoad = function(deck) {
    if (!this._NATIVE_WAVEFORM) {
        return;   // external daemon handles the xx30/35/36 upload
    }
    var group = "[Channel" + deck + "]";
    var duration = engine.getValue(group, "duration");
    if (!(duration > 0)) { return; }
    // The analysed waveform lands in Track memory a moment after load, so poll
    // engine.getWaveformSummary() until it's ready, then do the upload.
    if (this._wfPollTimer[deck]) {
        engine.stopTimer(this._wfPollTimer[deck]);
        this._wfPollTimer[deck] = 0;
    }
    var self = this;
    var tries = 0;
    this._wfPollTimer[deck] = engine.beginTimer(150, function() {
        tries++;
        var entries = self._generateEntries(deck, duration);
        if (entries && entries.length) {
            engine.stopTimer(self._wfPollTimer[deck]);
            self._wfPollTimer[deck] = 0;
            self._uploadWaveform(deck, duration, entries);
        } else if (tries >= 40) {   // ~6 s
            engine.stopTimer(self._wfPollTimer[deck]);
            self._wfPollTimer[deck] = 0;
            console.log("FLX10 screen: waveform not ready for deck " + deck +
                        " after " + tries + " polls");
        }
    });
};


// ===== Lifecycle ============================================================
PioneerDDJFLX10Screen.init = function(id) {
    console.log("FLX10 screen v2.0 (xx 27 only, daemon handles wave): init");

    // Mixxx 2.7-alpha gives us engine.getPlayer() returning a JavascriptPlayerProxy
    // with: artist, title, album, key (Camelot text like "3B (D♭)"), genre, year, etc.
    // plus per-property change signals (keyChanged, titleChanged, etc).
    // NOT exposed: file path / samples / fileBpm / duration / trackLoaded signal.
    // So daemon's log-tail + SQL-by-samples-and-bpm still needed for waveform.
    // We can use player.key for Camelot display once we crack the FLX10 key bytes.
    PioneerDDJFLX10Screen._hasPlayerProxy = (typeof engine.getPlayer === "function");
    if (this._hasPlayerProxy) {
        console.log("FLX10 screen: engine.getPlayer available (Mixxx 2.7-alpha+)");
    }

    // State timer — dispatch based on _SCREEN_MODE.
    var modeFn;
    if (this._SCREEN_MODE === 'vdj') {
        modeFn = function() { PioneerDDJFLX10Screen._sendVDJState(); };
        console.log("FLX10 screen: VDJ mode (xx 21, no heartbeat)");
    } else if (this._SCREEN_MODE === 'rekordbox') {
        modeFn = function() { PioneerDDJFLX10Screen._sendRekordboxState(); };
        console.log("FLX10 screen: REKORDBOX mode (xx 21 + xx 3D heartbeat)");
    } else {
        modeFn = function() { PioneerDDJFLX10Screen._sendStateAllDecks(); };
        console.log("FLX10 screen: SERATO mode (xx 27)");
    }
    this._stateTimer = engine.beginTimer(this._STATE_MS, modeFn, false);

    // Arm all 4 decks at startup (xx30 + xx39) exactly like the daemon does
    // before any track loads. Without this the firmware won't accept waveform
    // data on a not-yet-initialised deck (observed: deck 2 stayed blank even
    // after a clean upload).
    if (this._NATIVE_WAVEFORM) {
        for (var ad = 1; ad <= 4; ad++) {
            this._sendInit30(ad, 0);
            this._sendXX39(ad);
        }
        this._startTrickle();
        if (this._EQ_REACTIVE) { this._startEqRefresh(); }
    }

    // Track-load detection only — xx 27 updates come from the timer above
    // (NOT event-driven, because Mixxx throttles makeConnection callbacks
    // on playposition).
    for (var dd = 1; dd <= 4; dd++) {
        (function(deck) {
            engine.makeConnection(
                "[Channel" + deck + "]",
                "duration",
                function(value) {
                    if (value > 0 && value !== PioneerDDJFLX10Screen._lastDuration[deck]) {
                        PioneerDDJFLX10Screen._lastDuration[deck] = value;
                        var bpm = engine.getValue("[Channel" + deck + "]", "bpm");
                        if (bpm > 0) { PioneerDDJFLX10Screen._deckBpm[deck] = bpm; }
                        PioneerDDJFLX10Screen._onTrackLoad(deck);
                    }
                }
            );
        })(dd);
    }
};

PioneerDDJFLX10Screen.shutdown = function() {
    console.log("FLX10 screen: shutdown");
    if (this._stateTimer !== null) {
        engine.stopTimer(this._stateTimer);
        this._stateTimer = null;
    }
    if (this._trickleTimer) {
        engine.stopTimer(this._trickleTimer);
        this._trickleTimer = 0;
    }
    if (this._refreshTimer) {
        engine.stopTimer(this._refreshTimer);
        this._refreshTimer = 0;
    }
    this._txQueue = [];   // drop any pending bulk packets
    this._refreshQueue = {1: [], 2: [], 3: [], 4: []};
};

// ACK packets (xx D8 ...) arrive on EP4 IN; we don't need to act on them.
PioneerDDJFLX10Screen.incomingData = function(data, length) {
    // Log first 16 bytes — looking for jog-mode button events from FLX10.
    try {
        var hex = '';
        var n = Math.min(length, 16);
        for (var i = 0; i < n; i++) {
            var b = data[i] || 0;
            hex += (b < 16 ? '0' : '') + b.toString(16) + ' ';
        }
        console.log("FLX10_HID_IN len=" + length + " bytes=" + hex);
    } catch (e) {}
};
