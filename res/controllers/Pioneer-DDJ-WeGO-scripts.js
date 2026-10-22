// AI-generated: this file was written with an AI agent (Claude) from a MIDI map
// verified on DDJ-WeGO hardware and Pioneer's Traktor mapping. Based on the
// original mapping by Samuel Nicholas (https://github.com/mixxxdj/mixxx/pull/1346).
//
// Deck channels: 0x90-0x93 / 0xB0-0xB3 = decks A-D. The controller only sends
// deck C/D messages after the DECK button LED is lit, and Jog FX values only
// after the CTRL/FX button LED is lit, so those LEDs double as mode switches.
// SHIFT is handled by the hardware, which sends different notes while held.

// Must be var: Mixxx calls PioneerDDJWeGO.* from separately evaluated code.
// eslint-disable-next-line no-var
var PioneerDDJWeGO = {};

PioneerDDJWeGO.jogResolution = 480;
PioneerDDJWeGO.bendScale = 0.8;
PioneerDDJWeGO.searchScale = 150;

// [left, right] side shows deck C/D instead of A/B
PioneerDDJWeGO.deckCD = [false, false];
PioneerDDJWeGO.tempoMSBValue = {};
PioneerDDJWeGO.vuLevel = [0, 0, 0, 0];
PioneerDDJWeGO.litLeds = {};
// Per-deck LED connections, re-triggered when a hidden deck becomes visible.
PioneerDDJWeGO.connections = [[], [], [], []];
// 14-bit mixer controls keyed by their MSB CC: {group, key, msb}. Populated
// in init() so the script, not the XML, owns their (optional) soft-takeover.
PioneerDDJWeGO.mixer14 = {};

PioneerDDJWeGO.led = function(status, midino, on) {
    PioneerDDJWeGO.litLeds[(status << 8) | midino] = true;
    midi.sendShortMsg(status, midino, on ? 0x7F : 0x00);
};

// Relative encoders send 0x01..0x3F for clockwise and 0x7F..0x41 for counter-clockwise.
PioneerDDJWeGO.encoderDelta = function(value) {
    return value < 0x40 ? value : value - 0x80;
};

PioneerDDJWeGO.init = function() {
    const connect = function(group, key, callback) {
        engine.makeConnection(group, key, callback).trigger();
    };

    // Register the 14-bit mixer controls (volume, EQ, crossfader, headMix,
    // tempo). Soft-takeover is a setting; engine.softTakeover only affects
    // values set from the script, so these controls are script-routed.
    const softTakeover = engine.getSetting("softTakeover");
    const addMixer = function(midino, group, key) {
        PioneerDDJWeGO.mixer14[midino] = {group: group, key: key, msb: 0};
        engine.softTakeover(group, key, softTakeover);
    };

    for (let n = 0; n < 4; n++) {
        const group = `[Channel${n + 1}]`;
        const note = 0x90 + n;
        const side = n % 2;
        const buttonOffset = n < 2 ? 0 : 5;
        const ringOffset = n < 2 ? 0 : 8;
        const unit = `[EffectRack1_EffectUnit${n + 1}]`;
        const quickEffect = `[QuickEffectRack1_${group}]`;
        const eq = `[EqualizerRack1_${group}_Effect1]`;

        // Deck-scoped connection: kept so deckToggle can re-send the LEDs for
        // the deck that becomes visible (the controller forgets hidden state).
        const connectDeck = function(group, key, callback) {
            const conn = engine.makeConnection(group, key, callback);
            conn.trigger();
            PioneerDDJWeGO.connections[n].push(conn);
        };

        engine.setValue(group, "beatloop_size", engine.getSetting("autoLoopSize"));
        engine.softTakeover(group, "rate", softTakeover);
        addMixer(0x13 + 2 * n, group, "volume");
        addMixer(0x07 + n, eq, "parameter3"); // EQ HI
        addMixer(0x0B + n, eq, "parameter2"); // EQ MID
        addMixer(0x0F + n, eq, "parameter1"); // EQ LOW
        // Jog FX of deck N drives effect unit N, so route that deck into it.
        if (engine.getSetting("routeDecksToUnits")) {
            engine.setValue(unit, `group_${group}_enable`, 1);
        }

        connectDeck(group, "play_indicator", function(value) {
            PioneerDDJWeGO.led(note, 0x0B, value);
            PioneerDDJWeGO.led(note, 0x47, value);
            PioneerDDJWeGO.led(note, 0x6E, value);
        });
        connectDeck(group, "cue_indicator", function(value) {
            PioneerDDJWeGO.led(note, 0x0C, value);
            PioneerDDJWeGO.led(note, 0x48, value);
        });
        connectDeck(group, engine.getSetting("blinkingSync") ? "beat_active" : "sync_enabled", function(value) {
            PioneerDDJWeGO.led(note, 0x58, value);
        });
        connectDeck(group, "sync_leader", function(value) {
            PioneerDDJWeGO.led(note, 0x5C, value);
        });
        for (let k = 0; k < 4; k++) {
            connectDeck(group, `hotcue_${k + 1}_status`, function(value) {
                PioneerDDJWeGO.led(note, 0x2E + k, value);
                PioneerDDJWeGO.led(note, 0x5F + k, value);
            });
        }
        connectDeck(group, "track_loaded", function(value) {
            PioneerDDJWeGO.led(0x9B, 0x0C + n, value);
        });
        connectDeck(group, "loop_enabled", function(value) {
            PioneerDDJWeGO.led(0x9B, 0x10 + n, value);
        });
        connectDeck(group, "pfl", function(value) {
            PioneerDDJWeGO.led(0x96, 0x54 + n, value);
        });
        connectDeck(group, "vu_meter", function(value) {
            const level = Math.round(value * 5);
            if (level === PioneerDDJWeGO.vuLevel[n]) {
                return;
            }
            PioneerDDJWeGO.vuLevel[n] = level;
            for (let i = 0; i < 5; i++) {
                PioneerDDJWeGO.led(0x9C, 0x28 + 5 * n + i, i < level);
            }
        });

        // Jog FX buttons and the jog ring value they control
        const ring = function(midino) {
            return function(value) {
                midi.sendShortMsg(0xB4 + side, midino, Math.round(value * 0x7F));
            };
        };
        PioneerDDJWeGO.led(0x94 + side, 0x42 + buttonOffset, false);
        connectDeck(unit, "mix", ring(0x00 + ringOffset));
        for (let m = 0; m < 3; m++) {
            const effect = `[EffectRack1_EffectUnit${n + 1}_Effect${m + 1}]`;
            connectDeck(effect, "enabled", function(value) {
                PioneerDDJWeGO.led(0x94 + side, 0x43 + buttonOffset + m, value);
            });
            connectDeck(effect, "meta", ring(0x02 + ringOffset + 2 * m));
        }
        PioneerDDJWeGO.led(0x94 + side, 0x46 + buttonOffset, false);
        connectDeck(quickEffect, "super1", ring(0x07 + ringOffset));
    }

    addMixer(0x1F, "[Master]", "crossfader");
    addMixer(0x01, "[Master]", "headMix");

    for (let k = 0; k < 4; k++) {
        connect(`[Sampler${k + 1}]`, "play_indicator", function(value) {
            for (let n = 0; n < 4; n++) {
                PioneerDDJWeGO.led(0x90 + n, 0x3C + 2 * k, value);
                PioneerDDJWeGO.led(0x90 + n, 0x3D + 2 * k, value);
            }
        });
    }

    // Start on decks A and B.
    PioneerDDJWeGO.led(0x92, 0x72, false);
    PioneerDDJWeGO.led(0x93, 0x72, false);
};

PioneerDDJWeGO.shutdown = function() {
    for (const key in PioneerDDJWeGO.litLeds) {
        midi.sendShortMsg(key >> 8, key & 0xFF, 0x00);
    }
    // The jog ring values go out via raw messages, not led(), so clear them
    // here too: CTRL A, FX1-3, CTRL B CCs on both sides (A/B 0xB4, C/D 0xB5).
    const ringCCs = [0x00, 0x02, 0x04, 0x06, 0x07, 0x08, 0x0A, 0x0C, 0x0E, 0x0F];
    for (const status of [0xB4, 0xB5]) {
        for (const cc of ringCCs) {
            midi.sendShortMsg(status, cc, 0x00);
        }
    }
};

// CTRL A/B only switch the jog into Jog FX mode (lit LED = jog sends FX values),
// so they keep their own state instead of following Mixxx effect "enabled",
// which is on by default and would block scratching.
PioneerDDJWeGO.jogFxMode = {};

PioneerDDJWeGO.toggleJogFxMode = function(status, control) {
    const key = (status << 8) | control;
    PioneerDDJWeGO.jogFxMode[key] = !PioneerDDJWeGO.jogFxMode[key];
    PioneerDDJWeGO.led(status, control, PioneerDDJWeGO.jogFxMode[key]);
    return PioneerDDJWeGO.jogFxMode[key];
};

PioneerDDJWeGO.ctrlA = function(channel, control, value, status) {
    if (value) {
        PioneerDDJWeGO.toggleJogFxMode(status, control);
    }
};

PioneerDDJWeGO.ctrlB = function(channel, control, value, status, group) {
    // Turning CTRL B off re-centres the filter, so nothing stays filtered unlit.
    if (value && !PioneerDDJWeGO.toggleJogFxMode(status, control)) {
        engine.setValue(`[QuickEffectRack1_${group}]`, "super1", 0.5);
    }
};

PioneerDDJWeGO.deckToggle = function(channel, control, value, status) {
    if (!value) {
        return;
    }
    const side = channel - 2;
    PioneerDDJWeGO.deckCD[side] = !PioneerDDJWeGO.deckCD[side];
    PioneerDDJWeGO.led(status, 0x72, PioneerDDJWeGO.deckCD[side]);
    // The controller forgets LED state for the hidden deck, so re-send it for
    // the one now shown. -1 forces the VU connection to resend its level.
    const deck = side + (PioneerDDJWeGO.deckCD[side] ? 2 : 0);
    PioneerDDJWeGO.vuLevel[deck] = -1;
    PioneerDDJWeGO.connections[deck].forEach(function(conn) {
        conn.trigger();
    });
};

PioneerDDJWeGO.syncLeader = function(channel, control, value, status, group) {
    if (value) {
        script.toggleControl(group, "sync_leader");
    }
};

PioneerDDJWeGO.samplerPad = function(channel, control, value, status, group) {
    if (!value) {
        return;
    }
    if (!engine.getValue(group, "track_loaded")) {
        engine.setValue(group, "LoadSelectedTrack", 1);
    } else if (engine.getValue(group, "play")) {
        engine.setValue(group, "cue_gotoandstop", 1);
    } else {
        engine.setValue(group, "cue_gotoandplay", 1);
    }
};

PioneerDDJWeGO.samplerPadShift = function(channel, control, value, status, group) {
    if (!value) {
        return;
    }
    engine.setValue(group, engine.getValue(group, "play") ? "stop" : "eject", 1);
};

PioneerDDJWeGO.loopButton = function(channel, control, value, status, group) {
    if (!value) {
        return;
    }
    engine.setValue(group, engine.getValue(group, "loop_enabled") ? "reloop_toggle" : "beatloop_activate", 1);
};

PioneerDDJWeGO.loopEncoder = function(channel, control, value, status, group) {
    script.triggerControl(group, PioneerDDJWeGO.encoderDelta(value) > 0 ? "loop_double" : "loop_halve");
};

PioneerDDJWeGO.beatjumpEncoder = function(channel, control, value, status, group) {
    script.triggerControl(group, PioneerDDJWeGO.encoderDelta(value) > 0 ? "beatjump_forward" : "beatjump_backward");
};

PioneerDDJWeGO.jogTouch = function(channel, control, value) {
    // SHIFT+touch (0x67) only searches, so it never starts scratching.
    if (control === 0x36 && value) {
        engine.scratchEnable(channel + 1, PioneerDDJWeGO.jogResolution, 33 + 1 / 3, 1 / 8, 1 / 8 / 32);
    } else {
        engine.scratchDisable(channel + 1);
    }
};

PioneerDDJWeGO.jogTurn = function(channel, control, value, status, group) {
    if (engine.isScratching(channel + 1)) {
        engine.scratchTick(channel + 1, value - 0x40);
    } else {
        PioneerDDJWeGO.jogEdge(channel, control, value, status, group);
    }
};

PioneerDDJWeGO.jogEdge = function(channel, control, value, status, group) {
    engine.setValue(group, "jog", (value - 0x40) * PioneerDDJWeGO.bendScale);
};

PioneerDDJWeGO.jogSearch = function(channel, control, value, status, group) {
    engine.setValue(group, "jog", (value - 0x40) * PioneerDDJWeGO.searchScale);
};

// The tempo slider sends MSB first, then LSB.
PioneerDDJWeGO.tempoMSB = function(channel, control, value, status, group) {
    PioneerDDJWeGO.tempoMSBValue[group] = value;
};

PioneerDDJWeGO.tempoLSB = function(channel, control, value, status, group) {
    const fullValue = ((PioneerDDJWeGO.tempoMSBValue[group] || 0) << 7) + value;
    engine.setValue(group, "rate", 1 - fullValue / 0x2000);
};

// 14-bit mixer inputs (volume, EQ, crossfader, headMix). The hardware sends
// MSB (CC <= 0x1F) then LSB (MSB CC + 0x20). engine.setParameter applies the
// softTakeover registered in init() when the setting is on.
PioneerDDJWeGO.mixerInput = function(channel, control, value) {
    if (control <= 0x1F) {
        PioneerDDJWeGO.mixer14[control].msb = value;
        return;
    }
    const entry = PioneerDDJWeGO.mixer14[control - 0x20];
    engine.setParameter(entry.group, entry.key, ((entry.msb << 7) + value) / 0x3FFF);
};

PioneerDDJWeGO.browse = function(channel, control, value) {
    engine.setValue("[Library]", "MoveVertical", PioneerDDJWeGO.encoderDelta(value));
};

PioneerDDJWeGO.browseShift = function(channel, control, value) {
    engine.setValue("[Library]", "MoveVertical", PioneerDDJWeGO.encoderDelta(value) * 10);
};

// End of AI-generated file.
