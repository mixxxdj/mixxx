// Pioneer-DDJ-FLX10-screen.js  v0.5
// Standalone HID jog display module for DDJ-FLX10.
// Loaded by Pioneer-DDJ-FLX10-screen.hid.xml as a separate HID controller mapping.
//
// In this HID context, controller.send(pkt, null, 0) routes to interface 5 (screen).
//
// v0.4 changes (based on VirtualDJ capture analysis):
//   - Switched waveform command from xx 2E (PWV5 color detail) to xx 38 (3-band).
//   - Each entry is 3 bytes (low, mid, high), DUPLICATED on the wire (6 bytes per
//     logical entry). Verified 100% match against VirtualDJ across 5,184 entries.
//   - Stream begins with 4-byte header (entry count LE16 + 0x00 0x00).
//   - Each segment body starts with a 0x01 marker, then 122 bytes of stream data.
//   - Heartbeats DISABLED by default — VirtualDJ sent zero xx 3D packets across
//     26 seconds of visible waveform rendering. Set _ENABLE_HEARTBEAT = true if
//     testing suggests otherwise.
//   - Test pattern produces visible bass-kick rhythm so you can verify rendering.
//
// Known issue: when we replayed VirtualDJ's exact bytes via raw /dev/hidraw on
// Linux, the device acknowledged everything but did not render. Going through
// Mixxx's HID API (different USB stack path, possibly libusb-based hidapi) is
// the remaining variable worth testing.

var PioneerDDJFLX10Screen = {};

PioneerDDJFLX10Screen._ENABLE_HEARTBEAT = false;   // xx 3D — not used by VirtualDJ
PioneerDDJFLX10Screen._HEARTBEAT_MS     = 20;
PioneerDDJFLX10Screen._HEARTBEAT_SEGS   = 5;
PioneerDDJFLX10Screen._METADATA_MS      = 20;      // xx 21 — 50 Hz per-deck, required
PioneerDDJFLX10Screen._ENTRIES_PER_SEC  = 150;
PioneerDDJFLX10Screen._SEG_PAYLOAD      = 122;     // 128 - 5 hdr - 1 marker
PioneerDDJFLX10Screen._SEGS_PER_SUB     = 255;
PioneerDDJFLX10Screen._OVERVIEW_ENTRIES = 600;     // fixed-size overview ring
PioneerDDJFLX10Screen._heartbeatTimer   = null;
PioneerDDJFLX10Screen._metadataTimer    = null;
PioneerDDJFLX10Screen._lastDuration     = {1: 0, 2: 0, 3: 0, 4: 0};

PioneerDDJFLX10Screen._DECK_BYTE = [0x10, 0x20, 0x30, 0x40];


// ----- Raw HID send via Mixxx -----------------------------------------------
PioneerDDJFLX10Screen._send = function(pkt) {
    controller.send(pkt, null, 0);
};

PioneerDDJFLX10Screen._buildPkt = function(b0, b1, seg, sub, b4) {
    var p = [];
    for (var i = 0; i < 128; i++) { p[i] = 0; }
    p[0] = b0 & 0xFF; p[1] = b1 & 0xFF;
    p[2] = seg & 0xFF; p[3] = sub & 0xFF; p[4] = b4 & 0xFF;
    return p;
};


// ----- xx 21 — Track Metadata (50 Hz, all decks, required) -----------------
//
// Format decoded from VirtualDJ capture. Sending all zeros told the firmware
// "no track loaded" and suppressed rendering. Key bytes:
//   [3]=0x0a [4]=0x04 [5]=0x01 [7]=0x02(loaded)/0x01(empty) [10]=0x0e
//   [21-24]=0x75,0x40,0x2a,0xff(loaded) or 0x78,0,0,0(empty)
//   [27]=0x80 [49]=0x30 [58]=0x03 [59]=overview_segs [61]=0x0d
//
PioneerDDJFLX10Screen._buildMetadataPkt = function(deckByte, trackLoaded, playing) {
    var pkt = [];
    var i;
    for (i = 0; i < 128; i++) { pkt[i] = 0; }
    pkt[0]  = deckByte;
    pkt[1]  = 0x21;
    pkt[3]  = 0x0a;
    // byte[4] and byte[5] reflect playback state:
    //   stopped: 0x04, 0x01   playing: 0x0c, 0x81  (from VirtualDJ capture)
    pkt[4]  = playing ? 0x0c : 0x04;
    pkt[5]  = playing ? 0x81 : 0x01;
    pkt[7]  = trackLoaded ? 0x02 : 0x01;
    pkt[10] = 0x0e;
    if (trackLoaded) {
        pkt[21] = 0x75; pkt[22] = 0x40; pkt[23] = 0x2a; pkt[24] = 0xff;
        pkt[49] = playing ? 0x20 : 0x30;
    } else {
        pkt[21] = 0x78;
    }
    pkt[27] = 0x80;
    pkt[58] = 0x03;
    pkt[59] = 0x1e;   // 30 overview segments
    pkt[61] = 0x0d;
    return pkt;
};

PioneerDDJFLX10Screen._sendMetadata = function(deck, trackLoaded, playing) {
    var deckByte = PioneerDDJFLX10Screen._DECK_BYTE[deck - 1];
    if (deckByte === undefined) { return; }
    PioneerDDJFLX10Screen._send(
        PioneerDDJFLX10Screen._buildMetadataPkt(deckByte, !!trackLoaded, !!playing)
    );
};

PioneerDDJFLX10Screen._metadataTick = function() {
    for (var d = 1; d <= 4; d++) {
        var loaded  = PioneerDDJFLX10Screen._lastDuration[d] > 0;
        var playing = loaded && engine.getValue("[Channel" + d + "]", "play") > 0;
        PioneerDDJFLX10Screen._sendMetadata(d, loaded, playing);
    }
};


// ----- Heartbeat (currently disabled) ---------------------------------------
PioneerDDJFLX10Screen._heartbeat = function() {
    for (var s = 1; s <= PioneerDDJFLX10Screen._HEARTBEAT_SEGS; s++) {
        PioneerDDJFLX10Screen._send(
            PioneerDDJFLX10Screen._buildPkt(0x00, 0x3D, s, 0x00,
                                            PioneerDDJFLX10Screen._HEARTBEAT_SEGS)
        );
    }
};


// ----- 3-band waveform entries (test pattern with kick drums) ---------------
// Produces visually recognizable output: rhythmic bass kicks with an envelope.
// Replace this with real Mixxx waveform data once we have something rendering.
PioneerDDJFLX10Screen._generateWaveformEntries = function(durationSec) {
    var n = Math.floor(durationSec * PioneerDDJFLX10Screen._ENTRIES_PER_SEC);
    var entries = [];
    for (var i = 0; i < n; i++) {
        var t = i / PioneerDDJFLX10Screen._ENTRIES_PER_SEC;
        var env = 0.3 + 0.7 * Math.abs(((Math.floor(i / 75) % 4) - 1.5) / 1.5);
        var low  = Math.round(((t % 1.0) < 0.05 ? 255 : 30) * env);
        var mid  = Math.round(((t % 0.5) < 0.04 ? 200 : 40) * env);
        var high = Math.round(((t % 2.0) < 0.15 ? 180 : 50) * env);
        entries.push([low, mid, high]);
    }
    return entries;
};


// ----- xx 37 — Waveform Overview upload ------------------------------------
//
// Format verified from VirtualDJ pcapng capture:
//   byte[5] = 0x00 — NO marker byte (unlike xx 38 which uses 0x01)
//   payload = 122 raw 3-byte entries at bytes[6..127] — NO 4-byte count header
//   NO entry duplication (each [lo,md,hi] sent once, unlike xx 38)
//   30 segments × 122 bytes = 3660 bytes = 1220 entries per upload
//
PioneerDDJFLX10Screen._uploadOverview = function(deck) {
    var deckByte = PioneerDDJFLX10Screen._DECK_BYTE[deck - 1];
    if (deckByte === undefined) { return; }
    var PAYLOAD = PioneerDDJFLX10Screen._SEG_PAYLOAD;
    var n = 30 * PAYLOAD / 3;   // 1220 entries
    var i, phase, lo, md, hi;
    var stream = [];
    for (i = 0; i < n; i++) {
        phase = i / n;
        lo = Math.round(180 * (0.5 + 0.5 * Math.sin(phase * 6.28 * 8)));
        md = Math.round(100 * (0.5 + 0.5 * Math.sin(phase * 6.28 * 12 + 1)));
        hi = Math.round(60  * (0.5 + 0.5 * Math.sin(phase * 6.28 * 16 + 2)));
        stream.push(lo, md, hi);   // no duplication
    }
    var pos = 0;
    var seg = 1;
    var j;
    while (pos < stream.length) {
        var pkt = PioneerDDJFLX10Screen._buildPkt(deckByte, 0x37, seg, 0x00, 0x1E);
        // byte[5] stays 0x00 — no marker for xx 37
        for (j = 0; j < PAYLOAD; j++) {
            pkt[6 + j] = (pos + j < stream.length) ? stream[pos + j] : 0;
        }
        PioneerDDJFLX10Screen._send(pkt);
        pos += PAYLOAD;
        seg++;
    }
};


// ----- Waveform upload (xx 38, 3-band format) -------------------------------
//
// Packet layout (128 bytes each):
//   [0]      deck byte (0x10/0x20/0x30/0x40)
//   [1]      0x38                     (waveform command)
//   [2]      segment number (1..255)
//   [3]      subframe (0 or 1)
//   [4]      0xD9                     (declared total)
//   [5]      0x01                     (segment data marker — required)
//   [6..127] up to 122 bytes of stream data
//
// Continuous stream (concatenation of all segments' bytes [6..end]):
//   [0..3]   4-byte header (entry count LE16 + two zeros)
//   [4+]     3-byte entries, EACH DUPLICATED on the wire:
//              low, mid, high, low, mid, high, low, mid, high, ...
//
PioneerDDJFLX10Screen._uploadWaveform = function(deck, entries) {
    var deckByte = PioneerDDJFLX10Screen._DECK_BYTE[deck - 1];
    if (deckByte === undefined) { return; }
    var nEntries = entries.length;
    var i, j, lo, md, hi;

    // Build the continuous stream
    var stream = [
        nEntries & 0xFF,
        (nEntries >> 8) & 0xFF,
        0x00,
        0x00
    ];
    for (i = 0; i < nEntries; i++) {
        lo = entries[i][0] & 0xFF;
        md = entries[i][1] & 0xFF;
        hi = entries[i][2] & 0xFF;
        stream.push(lo); stream.push(md); stream.push(hi);
        stream.push(lo); stream.push(md); stream.push(hi);
    }

    var streamLen   = stream.length;
    var PAYLOAD     = PioneerDDJFLX10Screen._SEG_PAYLOAD;
    var SEGS_PER_SF = PioneerDDJFLX10Screen._SEGS_PER_SUB;
    var MAX_BYTES   = SEGS_PER_SF * PAYLOAD; // 255*122 = 31110

    var pos = 0;
    var subframe = 0;
    var totalSent = 0;

    while (pos < streamLen && subframe < 2) {
        var sfEnd = pos + Math.min(MAX_BYTES, streamLen - pos);
        var seg = 1;
        while (pos < sfEnd) {
            var pkt = PioneerDDJFLX10Screen._buildPkt(deckByte, 0x38, seg, subframe, 0xD9);
            pkt[5] = 0x01;
            for (j = 0; j < PAYLOAD; j++) {
                pkt[6 + j] = (pos + j < sfEnd) ? stream[pos + j] : 0;
            }
            PioneerDDJFLX10Screen._send(pkt);
            pos += PAYLOAD;
            seg++;
            totalSent++;
            if (seg > 255) { break; }
        }
        subframe++;
    }

    console.log("FLX10 screen: deck " + deck + " — sent " + totalSent +
                " xx 38 packets (" + nEntries + " entries, " + streamLen + " stream bytes)");
};


// ----- Track load handler ---------------------------------------------------
PioneerDDJFLX10Screen.onTrackLoad = function(deck) {
    var dur = engine.getValue("[Channel" + deck + "]", "duration");
    if (!(dur > 0)) { return; }
    // Cap test waveform at 30s of data (no point uploading whole tracks while we're debugging)
    var testDur = Math.min(dur, 30);
    var entries = PioneerDDJFLX10Screen._generateWaveformEntries(testDur);
    console.log("FLX10 screen: track loaded on deck " + deck +
                " (" + Math.round(dur) + "s real, sending " + Math.round(testDur) +
                "s test pattern, " + entries.length + " entries)");
    PioneerDDJFLX10Screen._uploadOverview(deck);
    PioneerDDJFLX10Screen._uploadWaveform(deck, entries);
};


// ----- Mixxx lifecycle ------------------------------------------------------
PioneerDDJFLX10Screen.init = function(id) {
    // xx 21 metadata — required by all working implementations, 50 Hz
    PioneerDDJFLX10Screen._metadataTimer = engine.beginTimer(
        PioneerDDJFLX10Screen._METADATA_MS,
        PioneerDDJFLX10Screen._metadataTick,
        false
    );

    if (PioneerDDJFLX10Screen._ENABLE_HEARTBEAT) {
        PioneerDDJFLX10Screen._heartbeatTimer = engine.beginTimer(
            PioneerDDJFLX10Screen._HEARTBEAT_MS,
            PioneerDDJFLX10Screen._heartbeat,
            false
        );
    }

    for (var d = 1; d <= 4; d++) {
        (function(deck) {
            engine.makeConnection(
                "[Channel" + deck + "]",
                "duration",
                function(value) {
                    if (value > 0 && value !== PioneerDDJFLX10Screen._lastDuration[deck]) {
                        PioneerDDJFLX10Screen._lastDuration[deck] = value;
                        PioneerDDJFLX10Screen.onTrackLoad(deck);
                    }
                }
            );
        })(d);
    }

    console.log("FLX10 screen v0.5 (xx21 metadata + xx37 overview + xx38 waveform): HID mapping started");
};

PioneerDDJFLX10Screen.shutdown = function() {
    if (PioneerDDJFLX10Screen._metadataTimer !== null) {
        engine.stopTimer(PioneerDDJFLX10Screen._metadataTimer);
        PioneerDDJFLX10Screen._metadataTimer = null;
    }
    if (PioneerDDJFLX10Screen._heartbeatTimer !== null) {
        engine.stopTimer(PioneerDDJFLX10Screen._heartbeatTimer);
        PioneerDDJFLX10Screen._heartbeatTimer = null;
    }
    console.log("FLX10 screen: HID mapping stopped");
};

// ACK packets (xx D8 ...) arrive here; we don't need to act on them.
PioneerDDJFLX10Screen.incomingData = function(data, length) {};