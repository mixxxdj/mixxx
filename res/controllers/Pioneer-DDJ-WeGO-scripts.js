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

    for (let n = 0; n < 4; n++) {
        const group = `[Channel${n + 1}]`;
        const note = 0x90 + n;
        const side = n % 2;
        const buttonOffset = n < 2 ? 0 : 5;
        const ringOffset = n < 2 ? 0 : 8;
        const unit = `[EffectRack1_EffectUnit${n + 1}]`;
        const quickEffect = `[QuickEffectRack1_${group}]`;

        engine.setValue(group, "beatloop_size", engine.getSetting("autoLoopSize"));
        engine.softTakeover(group, "rate", true);
        // Jog FX of deck N drives effect unit N, so route that deck into it.
        engine.setValue(unit, `group_${group}_enable`, 1);

        connect(group, "play_indicator", function(value) {
            PioneerDDJWeGO.led(note, 0x0B, value);
            PioneerDDJWeGO.led(note, 0x47, value);
            PioneerDDJWeGO.led(note, 0x6E, value);
        });
        connect(group, "cue_indicator", function(value) {
            PioneerDDJWeGO.led(note, 0x0C, value);
            PioneerDDJWeGO.led(note, 0x48, value);
        });
        connect(group, engine.getSetting("blinkingSync") ? "beat_active" : "sync_enabled", function(value) {
            PioneerDDJWeGO.led(note, 0x58, value);
        });
        connect(group, "sync_leader", function(value) {
            PioneerDDJWeGO.led(note, 0x5C, value);
        });
        for (let k = 0; k < 4; k++) {
            connect(group, `hotcue_${k + 1}_status`, function(value) {
                PioneerDDJWeGO.led(note, 0x2E + k, value);
                PioneerDDJWeGO.led(note, 0x5F + k, value);
            });
        }
        connect(group, "track_loaded", function(value) {
            PioneerDDJWeGO.led(0x9B, 0x0C + n, value);
        });
        connect(group, "loop_enabled", function(value) {
            PioneerDDJWeGO.led(0x9B, 0x10 + n, value);
        });
        connect(group, "pfl", function(value) {
            PioneerDDJWeGO.led(0x96, 0x54 + n, value);
        });
        connect(group, "vu_meter", function(value) {
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
        connect(unit, "mix", ring(0x00 + ringOffset));
        for (let m = 0; m < 3; m++) {
            const effect = `[EffectRack1_EffectUnit${n + 1}_Effect${m + 1}]`;
            connect(effect, "enabled", function(value) {
                PioneerDDJWeGO.led(0x94 + side, 0x43 + buttonOffset + m, value);
            });
            connect(effect, "meta", ring(0x02 + ringOffset + 2 * m));
        }
        PioneerDDJWeGO.led(0x94 + side, 0x46 + buttonOffset, false);
        connect(quickEffect, "super1", ring(0x07 + ringOffset));
    }

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

PioneerDDJWeGO.browse = function(channel, control, value) {
    engine.setValue("[Library]", "MoveVertical", PioneerDDJWeGO.encoderDelta(value));
};

PioneerDDJWeGO.browseShift = function(channel, control, value) {
    engine.setValue("[Library]", "MoveVertical", PioneerDDJWeGO.encoderDelta(value) * 10);
};

// End of AI-generated file.
