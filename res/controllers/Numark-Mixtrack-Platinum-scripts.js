var MixtrackPlatinum = {};

MixtrackPlatinum.init = function(id, debug) {
    MixtrackPlatinum.id = id;
    MixtrackPlatinum.debug = debug;

    // exit demo mode
    var byteArray = [0xF0, 0x00, 0x01, 0x3F, 0x7F, 0x3A, 0x60, 0x00, 0x04, 0x04, 0x01, 0x00, 0x00, 0xF7];
    midi.sendSysexMsg(byteArray, byteArray.length);


    MixtrackPlatinum.decks = new components.ComponentContainer();
    MixtrackPlatinum.decks[1] = new MixtrackPlatinum.Deck(1, 0x00);
    MixtrackPlatinum.decks[2] = new MixtrackPlatinum.Deck(2, 0x01);
    MixtrackPlatinum.decks[3] = new MixtrackPlatinum.Deck(3, 0x02);
    MixtrackPlatinum.decks[4] = new MixtrackPlatinum.Deck(4, 0x03);

    MixtrackPlatinum.sampler = new MixtrackPlatinum.Sampler();

    // helper functions
    var loop_led = function(group, key, midi_channel, midino) {
        if (engine.getValue(group, key)) {
            midi.sendShortMsg(0x90 | midi_channel, midino, 0x01);
        }
        else {
            midi.sendShortMsg(0x80 | midi_channel, midino, 0x00);
        }
    };

    var loop_start_end_led = function(group, key, midi_channel, midino) {
        if (engine.getValue(group, key) == -1) {
            midi.sendShortMsg(0x80 | midi_channel, midino, 0x00);
        }
        else {
            midi.sendShortMsg(0x90 | midi_channel, midino, 0x01);
        }
    };

    var auto_loop_led = function(group, key, midi_channel, midino) {
        if (engine.getValue(group, key)) {
            midi.sendShortMsg(0x90 | midi_channel, midino, 0x40);
        }
        else {
            midi.sendShortMsg(0x80 | midi_channel, midino, 0x00);
        }
    };

    var led = function(group, key, midi_channel, midino) {
        if (engine.getValue(group, key)) {
            midi.sendShortMsg(0x90 | midi_channel, midino, 0x7F);
        }
        else {
            midi.sendShortMsg(0x80 | midi_channel, midino, 0x00);
        }
    };

    var led_dim = function(group, key, midi_channel, midino) {
        if (engine.getValue(group, key)) {
            midi.sendShortMsg(0x90 | midi_channel, midino, 0x7F);
        }
        else {
            midi.sendShortMsg(0x90 | midi_channel, midino, 0x01);
        }
    };

    // init a bunch of channel specific leds
    for (var i = 0; i < 4; ++i) {
        var group = "[Channel"+(i+1)+"]";

        // auto-loop leds
        auto_loop_led(group, 'beatloop_1_enabled',  i + 5, 0x14);
        auto_loop_led(group, 'beatloop_2_enabled',  i + 5, 0x15);
        auto_loop_led(group, 'beatloop_4_enabled',  i + 5, 0x16);
        auto_loop_led(group, 'beatloop_8_enabled',  i + 5, 0x17);
        auto_loop_led(group, 'beatloop_0.0625_enabled',  i + 5, 0x1C);
        auto_loop_led(group, 'beatloop_0.125_enabled',  i + 5, 0x1D);
        auto_loop_led(group, 'beatloop_0.25_enabled',  i + 5, 0x1E);
        auto_loop_led(group, 'beatloop_0.5_enabled',  i + 5, 0x1F);

        // update duration, time elapsed, and the spinner
        var duration = engine.getValue(group, 'duration');
        var position = engine.getValue(group, 'playposition');
        if (duration == 0) position = 0;
        MixtrackPlatinum.positionCallback(position, group, 'playposition');

        // update bpm
        var bpm = engine.getValue(group, 'bpm');
        if (bpm != 0) MixtrackPlatinum.screenBpm(i + 1, Math.round(bpm * 100));

        // keylock indicator
        led(group, 'keylock', i, 0x0D);

        // turn off bpm arrows
        midi.sendShortMsg(0x80 | i, 0x0A, 0x00); // down arrow off
        midi.sendShortMsg(0x80 | i, 0x09, 0x00); // up arrow off

        // slip indicator
        led(group, 'slip_enabled', i, 0x0F);

        // start in wheel/vinyl mode
        MixtrackPlatinum.wheel[i] = true;
        midi.sendShortMsg(0x90 | i, 0x07, 0x7F);
    }

    // init bpm_tap leds
    midi.sendShortMsg(0x98, 0x04, engine.getValue('[Channel1]', 'bpm_tap') ? 0x7F : 0x01);
    midi.sendShortMsg(0x99, 0x04, engine.getValue('[Channel2]', 'bpm_tap') ? 0x7F : 0x01);

    // init FX leds
    midi.sendShortMsg(0x98, 0x00, engine.getValue('[EffectRack1_EffectUnit1]', 'group_[Channel1]_enable') ? 0x7F : 0x01);
    midi.sendShortMsg(0x98, 0x01, engine.getValue('[EffectRack1_EffectUnit2]', 'group_[Channel1]_enable') ? 0x7F : 0x01);
    midi.sendShortMsg(0x98, 0x02, engine.getValue('[EffectRack1_EffectUnit3]', 'group_[Channel1]_enable') ? 0x7F : 0x01);
    midi.sendShortMsg(0x99, 0x00, engine.getValue('[EffectRack1_EffectUnit1]', 'group_[Channel2]_enable') ? 0x7F : 0x01);
    midi.sendShortMsg(0x99, 0x01, engine.getValue('[EffectRack1_EffectUnit2]', 'group_[Channel2]_enable') ? 0x7F : 0x01);
    midi.sendShortMsg(0x99, 0x02, engine.getValue('[EffectRack1_EffectUnit3]', 'group_[Channel2]_enable') ? 0x7F : 0x01);

    // zero vu meters
    midi.sendShortMsg(0xBF, 0x44, 0);
    midi.sendShortMsg(0xBF, 0x45, 0);

    // setup position tracking
    engine.connectControl("[Channel1]", "playposition", MixtrackPlatinum.positionCallback);
    engine.connectControl("[Channel2]", "playposition", MixtrackPlatinum.positionCallback);
    engine.connectControl("[Channel3]", "playposition", MixtrackPlatinum.positionCallback);
    engine.connectControl("[Channel4]", "playposition", MixtrackPlatinum.positionCallback);

    // setup bpm tracking
    engine.connectControl("[Channel1]", "bpm", MixtrackPlatinum.bpmCallback);
    engine.connectControl("[Channel2]", "bpm", MixtrackPlatinum.bpmCallback);
    engine.connectControl("[Channel3]", "bpm", MixtrackPlatinum.bpmCallback);
    engine.connectControl("[Channel4]", "bpm", MixtrackPlatinum.bpmCallback);

    // setup vumeter tracking
    engine.connectControl("[Channel1]", "VuMeter", MixtrackPlatinum.vuCallback);
    engine.connectControl("[Channel2]", "VuMeter", MixtrackPlatinum.vuCallback);
    engine.connectControl("[Channel3]", "VuMeter", MixtrackPlatinum.vuCallback);
    engine.connectControl("[Channel4]", "VuMeter", MixtrackPlatinum.vuCallback);
    engine.connectControl("[Master]", "VuMeterL", MixtrackPlatinum.vuCallback);
    engine.connectControl("[Master]", "VuMeterR", MixtrackPlatinum.vuCallback);
};

MixtrackPlatinum.shutdown = function() {
    // note: not all of this appears to be strictly necessary, things work fine
    // with out this, but other software has been observed sending these led
    // reset messages during shutdown. The last sysex message may be necessary
    // to re-enable demo mode.

    // turn off a bunch of channel specific leds
    for (var i = 0; i < 4; ++i) {
        // pfl/cue button leds
        midi.sendShortMsg(0x90 | i, 0x1B, 0x01);

        // loop leds
        midi.sendShortMsg(0x80 | i + 5, 0x32, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x33, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x34, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x35, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x38, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x39, 0x00);

        // play leds
        midi.sendShortMsg(0x90 | i, 0x00, 0x01);
        midi.sendShortMsg(0x90 | i, 0x04, 0x01);

        // sync leds
        midi.sendShortMsg(0x90 | i, 0x00, 0x02);
        midi.sendShortMsg(0x90 | i, 0x04, 0x03);

        // cue leds
        midi.sendShortMsg(0x90 | i, 0x00, 0x01);
        midi.sendShortMsg(0x90 | i, 0x04, 0x05);

        // hotcue leds
        midi.sendShortMsg(0x80 | i + 5, 0x18, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x19, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x1A, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x1B, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x20, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x21, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x22, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x23, 0x00);

        // auto-loop leds
        midi.sendShortMsg(0x80 | i + 5, 0x14, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x15, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x16, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x17, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x1C, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x1D, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x1E, 0x00);
        midi.sendShortMsg(0x80 | i + 5, 0x1F, 0x00);

        // update spinner and position indicator
        midi.sendShortMsg(0xB0 | i, 0x3F, 0);
        midi.sendShortMsg(0xB0 | i, 0x06, 0);

        // keylock indicator
        midi.sendShortMsg(0x80 | i, 0x0D, 0x00);

        // turn off bpm arrows
        midi.sendShortMsg(0x80 | i, 0x0A, 0x00); // down arrow off
        midi.sendShortMsg(0x80 | i, 0x09, 0x00); // up arrow off

        // turn off slip indicator
        midi.sendShortMsg(0x80 | i, 0x0F, 0x00);

        // turn off wheel button leds
        midi.sendShortMsg(0x80 | i, 0x07, 0x00);
    }

    // dim FX leds
    midi.sendShortMsg(0x98, 0x00, 0x01);
    midi.sendShortMsg(0x98, 0x01, 0x01);
    midi.sendShortMsg(0x98, 0x02, 0x01);
    midi.sendShortMsg(0x99, 0x00, 0x01);
    midi.sendShortMsg(0x99, 0x01, 0x01);
    midi.sendShortMsg(0x99, 0x02, 0x01);

    // turn off sampler leds
    midi.sendShortMsg(0x8F, 0x21, 0x00);
    midi.sendShortMsg(0x8F, 0x22, 0x00);
    midi.sendShortMsg(0x8F, 0x23, 0x00);
    midi.sendShortMsg(0x8F, 0x24, 0x00);

    // zero vu meters
    midi.sendShortMsg(0xBF, 0x44, 0);
    midi.sendShortMsg(0xBF, 0x45, 0);

    // send final shutdown message
    var byteArray = [0xF0, 0x00, 0x20, 0x7F, 0x02, 0xF7];
    midi.sendSysexMsg(byteArray, byteArray.length);
};

MixtrackPlatinum.Deck = function(deck_nums, midi_chan) {
    components.Deck.call(this, deck_nums);
    this.play_button = new components.PlayButton({
        midi: [0x90 + midi_chan, 0x00],
        off: 0x01,
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 4,
        unshift: function() {
            components.PlayButton.prototype.unshift.call(this);
            this.type = components.Button.prototype.types.toggle;
        },
        shift: function() {
            this.inKey = 'play_stutter';
            this.type = components.Button.prototype.types.push;
        },
    });

    this.cue_button = new components.CueButton({
        midi: [0x90 + midi_chan, 0x01],
        off: 0x01,
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 4,
        shift: function() {
            this.inKey = 'start';
        },
    });

    this.sync_button = new components.SyncButton({
        midi: [0x90 + midi_chan, 0x02],
        off: 0x01,
        sendShifted: true,
        shiftControl: true,
        shiftOffset: 1,
    });

    this.pfl_button = new components.Button({
        midi: [0x90 + midi_chan, 0x1B],
        key: 'pfl',
        off: 0x01,
        type: components.Button.prototype.types.toggle,
        connect: function() {
            components.Button.prototype.connect.call(this);
            this.connections[1] = engine.connectControl(this.group, this.outKey, MixtrackPlatinum.pflToggle);
        },
    });

    this.hotcues = [];
    for (var i = 1; i <= 4; ++i) {
        this.hotcues[i] = new components.HotcueButton({
            midi: [0x94 + midi_chan, 0x18 + i - 1],
            number: i,
            sendShifted: true,
            shiftControl: true,
            shiftOffset: 8,
        });
    }

    this.pitch = new components.Pot({
        inKey: 'rate',
        relative: true,
        invert: true,
    });

    this.loop_in = new components.Button({
        midi: [0x94 + midi_chan, 0x38],
        inKey: 'loop_in',
        outKey: 'loop_start_position',
        on: 0x01,
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: -0x10,
        outValueScale: function (value) {
            return (value != -1) ? this.on : this.off;
        },
    });

    this.loop_out = new components.Button({
        midi: [0x94 + midi_chan, 0x39],
        inKey: 'loop_out',
        outKey: 'loop_end_position',
        on: 0x01,
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: -0x10,
        outValueScale: function (value) {
            return (value != -1) ? this.on : this.off;
        },
    });

    this.loop_toggle = new components.LoopToggleButton({
        midi: [0x94 + midi_chan, 0x32],
        on: 0x01,
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: -0x10,
    });

    this.loop_halve = new components.Button({
        midi: [0x94 + midi_chan, 0x34],
        key: 'loop_halve',
        on: 0x01,
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: -0x10,
    });

    this.loop_double = new components.Button({
        midi: [0x94 + midi_chan, 0x35],
        key: 'loop_double',
        on: 0x01,
        sendShifted: true,
        shiftChannel: true,
        shiftOffset: -0x10,
    });

    this.reconnectComponents(function (c) {
        if (c.group === undefined) {
            c.group = this.currentDeck;
        }
    });
};

MixtrackPlatinum.Deck.prototype = new components.Deck();

MixtrackPlatinum.Sampler = function() {
    for (var i = 1; i <= 4; ++i) {
        this[i] = new components.SamplerButton({
            midi: [0x9F, 0x20 + i],
            number: i,
            loaded: 0x00,
            playing: 0x7F,
        });
    }
};

MixtrackPlatinum.Sampler.prototype = new components.ComponentContainer();

MixtrackPlatinum.encodeNum = function(number) {
    var number_array = [
        (number >> 28) & 0x0F,
        (number >> 24) & 0x0F,
        (number >> 20) & 0x0F,
        (number >> 16) & 0x0F,
        (number >> 12) & 0x0F,
        (number >> 8) & 0x0F,
        (number >> 4) & 0x0F,
        number & 0x0F,
    ];

    if (number < 0) number_array[0] = 0x07;
    else number_array[0] = 0x08;

    return number_array;
};

MixtrackPlatinum.duration = [
    -1,
    -1,
    -1,
    -1,
];
MixtrackPlatinum.screenDuration = function(deck, duration) {
    // don't do anything if duration didn't change
    if (MixtrackPlatinum.duration[deck - 1] == duration) return;
    MixtrackPlatinum.duration[deck - 1] = duration;

    if (duration < 1) duration = 1;
    duration = MixtrackPlatinum.encodeNum(duration - 1);

    var bytePrefix = [0xF0, 0x00, 0x20, 0x7F, deck, 0x03];
    var bytePostfix = [0xF7];
    var byteArray = bytePrefix.concat(duration, bytePostfix);
    midi.sendSysexMsg(byteArray, byteArray.length);
};

MixtrackPlatinum.screenTime = function(deck, time) {
    var time_val = MixtrackPlatinum.encodeNum(time);

    var bytePrefix = [0xF0, 0x00, 0x20, 0x7F, deck, 0x04];
    var bytePostfix = [0xF7];
    var byteArray = bytePrefix.concat(time_val, bytePostfix);
    midi.sendSysexMsg(byteArray, byteArray.length);
};

MixtrackPlatinum.screenBpm = function(deck, bpm) {
    bpm = MixtrackPlatinum.encodeNum(bpm);
    bpm.shift();
    bpm.shift();

    var bytePrefix = [0xF0, 0x00, 0x20, 0x7F, deck, 0x01];
    var bytePostfix = [0xF7];
    var byteArray = bytePrefix.concat(bpm, bytePostfix);
    midi.sendSysexMsg(byteArray, byteArray.length);
};

MixtrackPlatinum.bpmCallback = function(value, group, control) {
    var midi_chan = MixtrackPlatinum.channelMap[group];
    MixtrackPlatinum.screenBpm(midi_chan + 1, Math.round(value * 100));
};

MixtrackPlatinum.channelMap = {
    "[Channel1]": 0x00,
    "[Channel2]": 0x01,
    "[Channel3]": 0x02,
    "[Channel4]": 0x03,
};

MixtrackPlatinum.show_elapsed = [
    true,
    true,
    true,
    true,
];

MixtrackPlatinum.elapsedToggle = function (channel, control, value, status, group) {
    if (value != 0x7F) return;
    MixtrackPlatinum.show_elapsed[channel] = !MixtrackPlatinum.show_elapsed[channel];
    var on_off = 0x7F;
    if (MixtrackPlatinum.show_elapsed[channel]) on_off = 0x00;
    midi.sendShortMsg(0x90 | channel, 0x46, on_off);
};

MixtrackPlatinum.timeMs = function(deck, position, duration) {
    return Math.round(duration * position * 1000);
};

MixtrackPlatinum.positionCallback = function(value, group, control) {
    var midi_chan = MixtrackPlatinum.channelMap[group];
    // the value appears to range from 0-52
    var pos = Math.round(value * 52);
    if (pos < 0) pos = 0;
    midi.sendShortMsg(0xB0 | midi_chan, 0x3F, pos);

    // update duration if necessary
    var duration = engine.getValue(group, 'duration');
    MixtrackPlatinum.screenDuration(midi_chan + 1, duration * 1000);

    // update the time display
    var time = MixtrackPlatinum.timeMs(midi_chan + 1, value, duration);
    MixtrackPlatinum.screenTime(midi_chan + 1, time);

    // update the spinner (range 64-115, 52 values)
    //
    // the visual spinner in the mixxx interface looks like it takes 1.8
    // seconds to loop, so we use that value here
    var spinner = Math.round((duration * value) % 1.8 * (52 / 1.8));
    if (spinner < 0) spinner += 115;
    else spinner += 64;

    midi.sendShortMsg(0xB0 | midi_chan, 0x06, spinner);
};

MixtrackPlatinum.deck_active = [
    true,
    true,
    false,
    false,
];
MixtrackPlatinum.deckSwitch = function (channel, control, value, status, group) {
    MixtrackPlatinum.deck_active[channel] = value == 0x7F;

    // also zero vu meters
    if (value != 0x7F) return;
    midi.sendShortMsg(0xBF, 0x44, 0);
    midi.sendShortMsg(0xBF, 0x45, 0);
};

// zero vu meters when toggling pfl
MixtrackPlatinum.pflToggle = function(value, group, control) {
    midi.sendShortMsg(0xBF, 0x44, 0);
    midi.sendShortMsg(0xBF, 0x45, 0);
};

MixtrackPlatinum.vuCallback = function(value, group, control) {
    // the top LED lights up at 81
    var level = value * 81;

    // if any channel pfl is active, show channel levels
    if (engine.getValue('[Channel1]', 'pfl')
        || engine.getValue('[Channel2]', 'pfl')
        || engine.getValue('[Channel3]', 'pfl')
        || engine.getValue('[Channel4]', 'pfl'))
    {
        if (group == '[Channel1]' && MixtrackPlatinum.deck_active[0]) {
            midi.sendShortMsg(0xBF, 0x44, level);
        }
        else if (group == '[Channel3]' && MixtrackPlatinum.deck_active[2]) {
            midi.sendShortMsg(0xBF, 0x44, level);
        }
        else if (group == '[Channel2]' && MixtrackPlatinum.deck_active[1]) {
            midi.sendShortMsg(0xBF, 0x45, level);
        }
        else if (group == '[Channel4]' && MixtrackPlatinum.deck_active[3]) {
            midi.sendShortMsg(0xBF, 0x45, level);
        }
    }
    else if (group == '[Master]' && control == 'VuMeterL') {
        midi.sendShortMsg(0xBF, 0x44, level);
    }
    else if (group == '[Master]' && control == 'VuMeterR') {
        midi.sendShortMsg(0xBF, 0x45, level);
    }
};

// these functions track if the user has let go of the deck but it is still
// spinning
MixtrackPlatinum.scratch_timer = [];
MixtrackPlatinum.scratch_tick = [];
MixtrackPlatinum.resetScratchTimer = function (deck) {
    if (!MixtrackPlatinum.scratch_timer[deck]) return;
    MixtrackPlatinum.scratch_tick[deck] = true;
};

MixtrackPlatinum.startScratchTimer = function (deck) {
    if (MixtrackPlatinum.scratch_timer[deck]) return;

    MixtrackPlatinum.scratch_tick[deck] = false;
    MixtrackPlatinum.scratch_timer[deck] = engine.beginTimer(20, "MixtrackPlatinum.scratchTimer("+deck+")");
};

MixtrackPlatinum.stopScratchTimer = function (deck) {
    if (MixtrackPlatinum.scratch_timer[deck]) {
        engine.stopTimer(MixtrackPlatinum.scratch_timer[deck]);
    }
    MixtrackPlatinum.scratch_timer[deck] = null;
};

MixtrackPlatinum.scratchTimer = function (deck) {
    // check if we saw a tick from this deck
    if (MixtrackPlatinum.scratch_tick[deck]) {
        // reset tick detection
        MixtrackPlatinum.scratch_tick[deck] = false;
        return;
    }

    MixtrackPlatinum.scratchDisable(deck);
};

MixtrackPlatinum.scratchDisable = function (deck) {
    MixtrackPlatinum.stopScratchTimer(deck);
    engine.scratchDisable(deck, false);
};

MixtrackPlatinum.scratchEnable = function (deck) {
    var alpha = 1.0/8;
    var beta = alpha/32;

    engine.scratchEnable(deck, 1015, 33+1/3, alpha, beta);
    MixtrackPlatinum.stopScratchTimer(deck);
};

// The button that enables/disables scratching
MixtrackPlatinum.touching = [];
MixtrackPlatinum.wheelTouch = function (channel, control, value, status, group) {
    // ignore touch events if not in vinyl mode
    if (value === 0x7F && !MixtrackPlatinum.wheel[channel]) return;

    var deck = channel + 1;
    MixtrackPlatinum.touching[deck] = 0x7F == value;


    // don't start scratching if shift is pressed
    if (value === 0x7F && !MixtrackPlatinum.shift) {
        MixtrackPlatinum.scratchEnable(deck);
    } else {    // If button up
        MixtrackPlatinum.startScratchTimer(deck);
    }
};

// The wheel that actually controls the scratching
MixtrackPlatinum.scratch_direction = [];
MixtrackPlatinum.wheelTurn = function (channel, control, value, status, group) {
    var deck = channel + 1;
    var direction;
    var newValue;
    if (value < 64) {
        newValue = value;
        direction = true;
    } else {
        newValue = value - 128;
        direction = false;
    }

    // detect shift for searching the track
    if (MixtrackPlatinum.shift) {
        engine.setValue(group, 'beatjump', newValue * 0.05);
        return;
    }

    // stop scratching if the wheel direction changes and the platter is not
    // being touched
    if (MixtrackPlatinum.scratch_direction[deck] === null) {
        MixtrackPlatinum.scratch_direction[deck] = direction;
    }
    else if (MixtrackPlatinum.scratch_direction[deck] != direction
        && !MixtrackPlatinum.touching[deck])
    {
        MixtrackPlatinum.scratchDisable(deck);
    }

    MixtrackPlatinum.scratch_direction[deck] = direction;

    // In either case, register the movement
    if (engine.isScratching(deck)) {
        engine.scratchTick(deck, newValue); // Scratch!
        MixtrackPlatinum.resetScratchTimer(deck);
    } else {
        engine.setValue(group, 'jog', newValue * 0.1); // Pitch bend
    }
};

MixtrackPlatinum.wheel = [];
MixtrackPlatinum.wheelToggle = function (channel, control, value, status, group) {
    if (value != 0x7F) return;
    MixtrackPlatinum.wheel[channel] = !MixtrackPlatinum.wheel[channel];
    var on_off = 0x01;
    if (MixtrackPlatinum.wheel[channel]) on_off = 0x7F;
    midi.sendShortMsg(0x90 | channel, 0x07, on_off);
};

MixtrackPlatinum.pitch = [
    [ 0, 0 ],
    [ 0, 0 ],
    [ 0, 0 ],
    [ 0, 0 ],
];

// track the state of the shift key
MixtrackPlatinum.shift = false;
MixtrackPlatinum.shiftToggle = function (channel, control, value, status, group) {
    MixtrackPlatinum.shift = value == 0x7F;

    if (MixtrackPlatinum.shift) {
        MixtrackPlatinum.decks.shift();
        MixtrackPlatinum.sampler.shift();
    }
    else {
        MixtrackPlatinum.decks.unshift();
        MixtrackPlatinum.sampler.unshift();
    }

    for (i = 1; i <= 4; ++i) {
        if (MixtrackPlatinum.touching[i]) {
            // if shift is pressed while we are scratching, stop scratching
            if (MixtrackPlatinum.shift) MixtrackPlatinum.scratchDisable(i);
            // esle if shift is released and we are still scratching, detect that
            else MixtrackPlatinum.scratchEnable(i);
        }
    }
};
