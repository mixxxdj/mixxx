////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
////////////////////////////////////////////////////////////////////////

// name: Vestax VCI-100MKII
// author: Takeshi Soejima
// description: 2015-10-18
// wiki: <http://www.mixxx.org/wiki/doku.php/vestax_vci-100mkii>

var VCI102 = {};

VCI102.headVolume = function(ch, midino, value, status, group) {
    engine.setValue(group, "headVolume", value / 127);
};

VCI102.volume = function(ch, midino, value, status, group) {
    engine.setValue(group, "volume", value / 127);
};

VCI102.deck = ["[Channel1]", "[Channel2]", "[Channel3]", "[Channel4]"];

VCI102.shift = [0, 0];

VCI102.setShift = function(ch, value) {
    var i, j, enabled;
    VCI102.shift[ch] = value * 2 / 127;
    for (i = ch; i < 4; i += 2) {
        enabled = "group_" + VCI102.deck[i] + "_enable";
        for (j = VCI102.shift[ch] + 1; j <= VCI102.shift[ch] + 2; j++) {
            engine.trigger("[EffectRack1_EffectUnit" + j + "]", enabled);
        }
    }
};

VCI102.shiftL = function(ch, midino, value, status, group) {
    VCI102.setShift(0, value);
};

VCI102.shiftR = function(ch, midino, value, status, group) {
    VCI102.setShift(1, value);
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
    if (!value) {
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

VCI102.rate = [0, 0, 0, 0];

VCI102.rateMSB = function(ch, midino, value, status, group) {
    VCI102.rate[ch] = (64 - value) * 128;
};

VCI102.rateLSB = function(ch, midino, value, status, group) {
    engine.setValue(group, "rate", (VCI102.rate[ch] - value) / 8192);
};

VCI102.pitch = function(ch, midino, value, status, group) {
    engine.setValue(group, "pitch", Math.round((value - 64) * 3 / 32));
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

VCI102.beatloop_exit = function(ch, midino, value, status, group) {
    if (value) {
        if (engine.getValue(group, "loop_enabled")) {
            engine.setValue(group, "reloop_exit", 1);
        } else {
            engine.setValue(group, "beatloop", VCI102.loopLength[ch]);
        }
    }
};

VCI102.reloop_out = function(ch, midino, value, status, group) {
    if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "loop_out", value);
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

VCI102.init = function() {
    var i, j, k, activate, enabled, led;
    var LED = [
        [0x2C, 0x25, 0x27, 0x28],
        [0x28, 0x25, 0x27, 0x2C]
    ];
    var LFX = [0x3A, 0x38];

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

    function makeLFX(ch, midino, shift) {
        return function(value, group, key) {
            if (VCI102.shift[ch % 2] == shift) {
                midi.sendShortMsg(0x90 + ch, midino, value * 127);
            }
        };
    }

    VCI102.shutdown();
    for (i = 0; i < 4; i++) {
        engine.connectControl(VCI102.deck[i], "loop_enabled", VCI102.slip);
        engine.connectControl(VCI102.deck[i], "pfl", headMix);
        enabled = "group_" + VCI102.deck[i] + "_enable";
        for (j = 0; j < 2; j++) {
            for (k = 0; k < 4; k += 2) {
                engine.connectControl(
                    "[EffectRack1_EffectUnit" + (j + k + 1) + "]",
                    enabled,
                    makeLFX(i, LFX[j], k));
            }
        }
        engine.softTakeover(VCI102.deck[i], "rate", true);
        engine.softTakeover(VCI102.deck[i], "pitch", true);
    }
    for (i = 1; i <= 4; i++) {
        activate = "hotcue_" + i + "_activate";
        makeButton(activate);
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
