// name: Vestax VCI-100MKII
// author: Takeshi Soejima
// description: 2016-12-1
// wiki: <http://www.mixxx.org/wiki/doku.php/vestax_vci-100mkii>

// JSHint Configuration
// global engine
// global midi

var VCI102 = {};

VCI102.deck = ["[Channel1]", "[Channel2]", "[Channel3]", "[Channel4]"];
VCI102.fx12 = ["[EffectRack1_EffectUnit1]", "[EffectRack1_EffectUnit2]"];
VCI102.fx34 = ["[EffectRack1_EffectUnit3]", "[EffectRack1_EffectUnit4]"];
VCI102.fxButton = [VCI102.fx12, VCI102.fx12];
VCI102.shift = [0, 0];

VCI102.setShift = function(ch, midino, value, status, group) {
    ch = VCI102.deck.indexOf(group);  // override channel by group
    VCI102.shift[ch] = value;
    // if shift then show the state of fx3/4 else fx1/2
    VCI102.fxButton[ch] = value ? VCI102.fx34 : VCI102.fx12;
    [VCI102.deck[ch], VCI102.deck[ch + 2]].forEach(function(deck) {
        var enable = "group_" + deck + "_enable";
        VCI102.fxButton[ch].forEach(function(fx) {
            engine.trigger(fx, enable);
        });
    });
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
    if (value) {
        if (VCI102.scratchTimer[ch]) {
            engine.stopTimer(VCI102.scratchTimer[ch]);
            VCI102.scratchTimer[ch] = 0;
        } else if (VCI102.shift[ch % 2]) {
            engine.brake(deck, true);
        } else {
            engine.scratchEnable(deck, 2400, 100 / 3, 1 / 8, 1 / 256);
        }
    } else if (engine.isScratching(deck)) {
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
    } else {
        // terminate brake even without shift if not scratching
        engine.brake(deck, false);
        VCI102.slip(value, group);
    }
};

VCI102.jog = function(ch, midino, value, status, group) {
    var deck = ch + 1;
    value -= 64;
    if (engine.isScratching(deck)) {
        engine.scratchTick(deck, value);
    } else if (!engine.getValue(group, "slip_enabled")) {
        engine.setValue(group, "jog", value * value * value / 1024);
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
    value = (value - 64) / 21;  // up and down to 3 semitones
    if (engine.getValue(group, "keylock")) {
        value = Math.round(value);  // discrete change on keylock
    }
    engine.setValue(group, "pitch_adjust", value);
};

VCI102.fxKnob = [
    "[EffectRack1_EffectUnit1_Effect1]",
    "[EffectRack1_EffectUnit2_Effect1]",
    "[EffectRack1_EffectUnit3_Effect1]",
    "[EffectRack1_EffectUnit4_Effect1]"
];

["parameter1", "parameter2", "parameter3"].forEach(function(key) {
    VCI102[key] = function(ch, midino, value, status, group) {
        group = VCI102.fxKnob[ch];
        if (VCI102.shift[ch % 2]) {
            // link to super1, inverse variants -> none -> direct variants
            if (engine.getValue(group, key + "_loaded")) {
                value = Math.round(value / 16) - 4;
                engine.setValue(group, key + "_link_type", Math.abs(value));
                engine.setValue(group, key + "_link_inverse", value < 0);
            }
        } else {
            engine.setParameter(group, key, value < 127 ? value / 128 : 1);
        }
    };
});

VCI102.super1 = function(ch, midino, value, status, group) {

    function getKey() {
        // if any parameters are linked then "super1" else "mix"
        var i, j, effect;
        for (i = engine.getValue(group, "num_effects"); i > 0; i--) {
            effect = group.slice(0, -1) + "_Effect" + i + "]";
            for (j = engine.getValue(effect, "num_parameters"); j > 0; j--) {
                if (engine.getValue(effect, "parameter" + j + "_link_type")) {
                    return "super1";
                }
            }
        }
        return "mix";
    }

    engine.setValue(group, getKey(), value < 127 ? value / 128 : 1);
};

VCI102.prev_chain = function(ch, midino, value, status, group) {
    if (VCI102.shift[1 - ch % 2]) {
        // select Effect1 of the EffectUnit if shift of the other Deck
        if (value) {
            VCI102.fxKnob[ch] = group.slice(0, -1) + "_Effect1]";
        }
    } else if (VCI102.fxKnob[ch].slice(-2, -1) > 1) {
        engine.setValue(VCI102.fxKnob[ch], "prev_effect", value > 0);
    } else {
        engine.setValue(group, "prev_chain", value > 0);
    }
};

VCI102.next_chain = function(ch, midino, value, status, group) {
    if (VCI102.shift[1 - ch % 2]) {
        // select Effect2 of the EffectUnit if shift of the other Deck
        if (value) {
            VCI102.fxKnob[ch] = group.slice(0, -1) + "_Effect2]";
        }
    } else if (VCI102.fxKnob[ch].slice(-2, -1) > 1) {
        engine.setValue(VCI102.fxKnob[ch], "next_effect", value > 0);
    } else {
        engine.setValue(group, "next_chain", value > 0);
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
        engine.setValue(group, "loop_out", value > 0);
    } else {
        engine.setValue(group, "reloop_exit", 1);
    }
};

VCI102.loop_halve = function(ch, midino, value, status, group) {
    if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "loop_halve", value > 0);
    } else if (value) {
        VCI102.setLoopLength(ch, status, VCI102.loopLength[ch] / 2);
    }
};

VCI102.loop_double = function(ch, midino, value, status, group) {
    if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "loop_double", value > 0);
    } else if (value) {
        VCI102.setLoopLength(ch, status, VCI102.loopLength[ch] * 2);
    }
};

VCI102.move = function(ch, group, dir) {
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
};

VCI102.move_backward = function(ch, midino, value, status, group) {
    if (value) {
        VCI102.move(ch, group, -1);
    }
};

VCI102.move_forward = function(ch, midino, value, status, group) {
    if (value) {
        VCI102.move(ch, group, 1);
    }
};

VCI102.Deck = ["[Channel1]", "[Channel2]"];
VCI102.hc = [
    "hotcue_1_enabled",
    "hotcue_2_enabled",
    "hotcue_3_enabled",
    "hotcue_4_enabled"
];

VCI102.setDeck = function(ch, midino, value, status, group) {
    if (value) {
        VCI102.Deck[ch] = group;
        VCI102.hc.forEach(function(hc) {
            engine.trigger(group, hc);
        });
    }
};

VCI102.solo = function(ch, midino, value, status, group) {
    if (value) {
        VCI102.deck.forEach(function(deck, i) {
            engine.setValue(deck, "pfl", i == ch);
        });
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
    var LEDfx = [0x3A, 0x38];
    var LEDhc = [
        [0x2C, 0x25, 0x27, 0x28],
        [0x28, 0x25, 0x27, 0x2C]
    ];

    function headMix(value, group, key) {
        if (value) {
            if (engine.getValue("[Master]", "headMix") == 1) {
                engine.setValue("[Master]", "headMix", -1);
            }
        } else if (engine.getValue("[Master]", "headMix") == -1) {
            if (VCI102.deck.every(function(deck) {
                return !engine.getValue(deck, "pfl");
            })) {
                engine.setValue("[Master]", "headMix", 1);
            }
        }
    }

    function makeLEDfx(ch, i) {
        return function(value, group, key) {
            if (group == VCI102.fxButton[ch % 2][i]) {
                midi.sendShortMsg(0x90 + ch, LEDfx[i], value * 127);
            }
        };
    }

    function makeLEDhc(ch, i) {
        return function(value, group, key) {
            if (group == VCI102.Deck[ch]) {
                midi.sendShortMsg(0x90 + ch, LEDhc[ch][i], value * 127);
                midi.sendShortMsg(0x92 + ch, LEDhc[ch][i], value * 127);
            }
        };
    }

    VCI102.deck.forEach(function(deck, i) {
        var enable = "group_" + deck + "_enable";
        [makeLEDfx(i, 0), makeLEDfx(i, 1)].forEach(function(led, j) {
            engine.connectControl(VCI102.fx12[j], enable, led);
            engine.connectControl(VCI102.fx34[j], enable, led);
        });
        ["loop_enabled", "play", "reverse"].forEach(function(key) {
            engine.connectControl(deck, key, VCI102.slip);
        });
        engine.connectControl(deck, "pfl", headMix);
        engine.softTakeover(deck, "rate", true);
        engine.softTakeover(deck, "pitch_adjust", true);
    });
    // to use fx parameter buttons for hotcue
    VCI102.hc.forEach(function(hc, i) {
        var activate = hc.replace("enabled", "activate");
        [makeLEDhc(0, i), makeLEDhc(1, i)].forEach(function(led, j) {
            [VCI102.deck[j], VCI102.deck[j + 2]].forEach(function(deck) {
                engine.connectControl(deck, activate, VCI102.slip);
                engine.connectControl(deck, hc, led);
            });
        });
        [activate, hc.replace("enabled", "clear")].forEach(function(key) {
            VCI102[key] = function(ch, midino, value, status, group) {
                engine.setValue(VCI102.Deck[ch % 2], key, value > 0);
            };
        });
    });
};
