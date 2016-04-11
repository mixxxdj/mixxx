// name: Vestax VCI-100MKII
// author: Takeshi Soejima
// description: 2016-4-1
// wiki: <http://www.mixxx.org/wiki/doku.php/vestax_vci-100mkii>

// JSHint Configuration
// global engine
// global midi

var VCI102 = {};

VCI102.deck = ["[Channel1]", "[Channel2]", "[Channel3]", "[Channel4]"];
VCI102.fx12 = ["[EffectRack1_EffectUnit1]", "[EffectRack1_EffectUnit2]"];
VCI102.fx34 = ["[EffectRack1_EffectUnit3]", "[EffectRack1_EffectUnit4]"];
VCI102.fx = VCI102.fx12.concat(VCI102.fx34);

VCI102.fxButton = [VCI102.fx12, VCI102.fx12];
VCI102.shift = [0, 0];

VCI102.setShift = function(ch, midino, value, status, group) {
    var i, j, enabled;
    ch = VCI102.deck.indexOf(group);  // override channel by group
    VCI102.fxButton[ch] = value ? VCI102.fx34 : VCI102.fx12;
    for (i = ch; i < 4; i += 2) {
        enabled = "group_" + VCI102.deck[i] + "_enable";
        for (j = 0; j < 2; j++) {
            engine.trigger(VCI102.fxButton[ch][j], enabled);
        }
    }
    VCI102.shift[ch] = value;
};

VCI102.selectTimer = 0;

VCI102.selectTrackIter = function(value, select) {
    if (value) {
        select();
        VCI102.selectTimer = engine.beginTimer(500, function() {
            VCI102.selectTimer = engine.beginTimer(40, select);
        }, true);
    } else {
        engine.stopTimer(VCI102.selectTimer);
    }
};

VCI102.SelectPrevTrack = function(ch, midino, value, status, group) {
    VCI102.selectTrackIter(value, function() {
        engine.setValue(group, "SelectPrevTrack", 1);
    });
};

VCI102.SelectNextTrack = function(ch, midino, value, status, group) {
    VCI102.selectTrackIter(value, function() {
        engine.setValue(group, "SelectNextTrack", 1);
    });
};

VCI102.slip = function(value, group, key) {
    // resume after the effect when the track is [re]played
    if (key == "play" ? value : !value && engine.getValue(group, "play")) {
        if (engine.getValue(group, "slip_enabled")) {
            engine.setValue(group, "slip_enabled", 0);
            engine.beginTimer(40, function() {
                engine.setValue(group, "slip_enabled", 1);
            }, true);
        }
    }
};

VCI102.scratchTimer = [0, 0, 0, 0];

VCI102.scratchEnable = function(ch, midino, value, status, group) {
    var deck = ch + 1;
    if (VCI102.shift[ch % 2]) {
        engine.brake(deck, value > 0);
        VCI102.slip(value, group);
    } else {
        if (value) {
            if (VCI102.scratchTimer[ch]) {
                engine.stopTimer(VCI102.scratchTimer[ch]);
                VCI102.scratchTimer[ch] = 0;
            } else {
                engine.scratchEnable(deck, 2400, 100 / 3, 1 / 8, 1 / 256);
            }
        } else {
            VCI102.scratchTimer[ch] = engine.beginTimer(20, function() {
                var vel = Math.abs(engine.getValue(group, "scratch2"));
                if (vel < 1 && (vel < 1e-9 || engine.getValue(group, "play"))) {
                    if (VCI102.scratchTimer[ch]) {
                        engine.stopTimer(VCI102.scratchTimer[ch]);
                        VCI102.scratchTimer[ch] = 0;
                        engine.scratchDisable(
                            deck, !engine.getValue(group, "slip_enabled"));
                        VCI102.slip(value, group);
                    }
                }
            });
        }
    }
};

VCI102.jog = function(ch, midino, value, status, group) {
    var deck = ch + 1;
    value -= 64;
    if (engine.isScratching(deck)) {
        engine.scratchTick(deck, value);
    } else if (!engine.getValue(group, "slip_enabled")) {
        engine.setValue(group, "jog", value * value * value / 512);
    }
};

VCI102.rateEnable = [true, true, true, true];

VCI102.rateValueMSB = [64, 64, 64, 64];  // defaults are at center

VCI102.rateMSB = function(ch, midino, value, status, group) {
    // unlock rate control if the change is not caused by a mechanical drift
    if (!VCI102.rateEnable[ch]) {
        if (Math.abs(value - VCI102.rateValueMSB[ch]) > 1) {
            VCI102.rateEnable[ch] = true;
        } else {
            return;
        }
    }
    VCI102.rateValueMSB[ch] = value;
};

VCI102.rateQuantizedMSB = function(ch, midino, value, status, group) {
    // lock rate control against a mechanical drift
    if (VCI102.rateEnable[ch]) {
        VCI102.rateEnable[ch] = false;
    }
    VCI102.rateValueMSB[ch] = value;
};

VCI102.rate = function(ch, lsb) {
    return ((64 - VCI102.rateValueMSB[ch]) * 128 - lsb) / 8192;
};

VCI102.rateLSB = function(ch, midino, value, status, group) {
    // inhibit rate control after the change by quantized bpm until unlocked
    if (VCI102.rateEnable[ch]) {
        engine.setValue(group, "rate", VCI102.rate(ch, value));
    }
};

VCI102.rateQuantizedLSB = function(ch, midino, value, status, group) {
    // not change "bpm" direct but by "rate" to go through soft takeover
    var bpm = engine.getValue(group, "file_bpm");
    var range = engine.getValue(group, "rateRange");
    engine.setValue(group, "rate", (Math.round(
        (VCI102.rate(ch, value) * range + 1) * bpm) / bpm - 1) / range);
};

VCI102.pitch = function(ch, midino, value, status, group) {
    engine.setValue(group, "pitch", Math.round((value - 64) * 3 / 32));
};

["parameter1", "parameter2", "parameter3"].forEach(function(key) {
    VCI102[key] = function(ch, midino, value, status, group) {
        if (VCI102.shift[ch % 2]) {
            // link to super1, inverse variants -> none -> direct variants
            if (engine.getValue(group, key + "_loaded")) {
                value = Math.round(value / 16) - 4;
                engine.setValue(group, key + "_link_type", Math.abs(value));
                engine.setValue(group, key + "_link_inverse", value < 0);
            }
        } else {
            engine.setParameter(group, key, value / 127);
        }
    };
});

VCI102.super1 = function(ch, midino, value, status, group) {
    if (VCI102.shift[ch % 2]) {
        engine.setValue(group, "mix", value / 127);
        engine.softTakeoverIgnoreNextValue(group, "super1");
    } else {
        engine.setValue(group, "super1", value / 127);
        engine.softTakeoverIgnoreNextValue(group, "mix");
    }
};

VCI102.loopLength = [4, 4, 4, 4];

VCI102.setLoopLength = function(ch, status, value) {
    var LED = [
        [0x33, 0x29],
        [0x29, 0x26],
        [0x33, 0x29],
        [0x29, 0x26]
    ];
    if (value > 64 || value * 32 < 1) return;
    VCI102.loopLength[ch] = value;
    midi.sendShortMsg(status, LED[ch][0], (value < 4) * 127);
    midi.sendShortMsg(status, LED[ch][1], (value > 4 || value * 4 < 1) * 127);
};

VCI102.loop = function(ch, midino, value, status, group) {
    if (value) {
        if (engine.getValue(group, "loop_enabled")) {
            engine.setValue(group, "reloop_exit", 1);
        } else {
            engine.setValue(group, "beatloop", VCI102.loopLength[ch]);
        }
    }
};

VCI102.reloop = function(ch, midino, value, status, group) {
    if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "loop_out", value / 127);
    } else {
        engine.setValue(group, "reloop_exit", 1);
    }
};

VCI102.loop_halve = function(ch, midino, value, status, group) {
    if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "loop_halve", value / 127);
    } else if (value) {
        VCI102.setLoopLength(ch, status, VCI102.loopLength[ch] / 2);
    }
};

VCI102.loop_double = function(ch, midino, value, status, group) {
    if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "loop_double", value / 127);
    } else if (value) {
        VCI102.setLoopLength(ch, status, VCI102.loopLength[ch] * 2);
    }
};

VCI102.move = function(ch, group, dir) {
    if (dir) {
        if (engine.getValue(group, "loop_enabled")) {
            // move the loop by the current length
            engine.setValue(
                group, "loop_move", dir * (
                    engine.getValue(group, "loop_end_position")
                        - engine.getValue(group, "loop_start_position"))
                    / engine.getValue(group, "track_samplerate")
                    * engine.getValue(group, "file_bpm") / 120);
        } else {
            // jump by the default length
            engine.setValue(group, "beatjump", dir * VCI102.loopLength[ch]);
        }
    }
};

VCI102.move_backward = function(ch, midino, value, status, group) {
    VCI102.move(ch, group, value / -127);
};

VCI102.move_forward = function(ch, midino, value, status, group) {
    VCI102.move(ch, group, value / 127);
};

VCI102.Deck = ["[Channel1]", "[Channel2]"];

VCI102.setDeck = function(ch, midino, value, status, group) {
    var i;
    if (value) {
        VCI102.Deck[ch] = group;
        for (i = 1; i <= 4; i++) {
            engine.trigger(group, "hotcue_" + i + "_enabled");
        }
    }
};

VCI102.solo = function(ch, midino, value, status, group) {
    var i;
    if (value) {
        for (i = 0; i < 4; i++) {
            engine.setValue(VCI102.deck[i], "pfl", i == ch);
        }
    }
};

VCI102.shutdown = function() {
    var i, j;
    for (i = 0x90; i < 0x94; i++) {
        for (j = 0x22; j < 0x3B; j++) {
            midi.sendShortMsg(i, j, 0);
        }
    }
};

VCI102.init = function(id, debug) {
    var i, j, k, activate, enabled, led;
    var LED = [
        [0x2C, 0x25, 0x27, 0x28],
        [0x28, 0x25, 0x27, 0x2C]
    ];
    var LEDfx = [0x3A, 0x38];

    function headMix(value, group, key) {
        var i;
        if (value) {
            if (engine.getValue("[Master]", "headMix") == 1) {
                engine.setValue("[Master]", "headMix", -1);
            }
        } else if (engine.getValue("[Master]", "headMix") == -1) {
            for (i = 0; i < 4; i++) {
                if (engine.getValue(VCI102.deck[i], "pfl")) return;
            }
            engine.setValue("[Master]", "headMix", 1);
        }
    }

    function makeButton(key) {
        VCI102[key] = function(ch, midino, value, status, group) {
            engine.setValue(VCI102.Deck[ch % 2], key, value / 127);
        };
    }

    function makeLED(ch, midino) {
        return function(value, group, key) {
            var i;
            if (group == VCI102.Deck[ch]) {
                value *= 127;
                for (i = 0x90 + ch; i < 0x94; i += 2) {
                    midi.sendShortMsg(i, midino, value);
                }
            }
        };
    }

    function makeLEDfx(ch, midino, button) {
        return function(value, group, key) {
            if (group == VCI102.fxButton[ch % 2][button]) {
                midi.sendShortMsg(0x90 + ch, midino, value * 127);
            }
        };
    }

    for (i = 0; i < 4; i++) {
        engine.connectControl(VCI102.deck[i], "loop_enabled", VCI102.slip);
        engine.connectControl(VCI102.deck[i], "reverse", VCI102.slip);
        engine.connectControl(VCI102.deck[i], "play", VCI102.slip);
        engine.connectControl(VCI102.deck[i], "pfl", headMix);
        enabled = "group_" + VCI102.deck[i] + "_enable";
        for (j = 0; j < 2; j++) {
            led = makeLEDfx(i, LEDfx[j], j);
            for (k = j + 1; k <= 4; k += 2) {
                engine.connectControl(
                    "[EffectRack1_EffectUnit" + k + "]", enabled, led);
            }
        }
        engine.softTakeover(VCI102.deck[i], "rate", true);
        engine.softTakeover(VCI102.deck[i], "pitch", true);
        engine.softTakeover(VCI102.fx[i], "super1", true);
        engine.softTakeover(VCI102.fx[i], "mix", true);
    }
    for (i = 1; i <= 4; i++) {
        makeButton(activate = "hotcue_" + i + "_activate");
        makeButton("hotcue_" + i + "_clear");
        enabled = "hotcue_" + i + "_enabled";
        for (j = 0; j < 2; j++) {
            led = makeLED(j, LED[j][i - 1]);
            for (k = j; k < 4; k += 2) {
                engine.connectControl(VCI102.deck[k], activate, VCI102.slip);
                engine.connectControl(VCI102.deck[k], enabled, led);
            }
        }
    }
};
