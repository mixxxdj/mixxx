var Numark = {};

// yanked from the Virtual DJ config on Numark's website
Numark.sysex = [0xF0, 0x00, 0x01, 0x3F, 0x7F, 0x2E, 0x60, 0x00, 0x01, 0x01, 0xF7];

Numark.jogResolution = 640;

Numark.buttons = {
    "[Channel1]": {
        "play": 0x4A,
        "sync": 0x40,
    },
    "[Channel2]": {
        "play": 0x4C,
        "sync": 0x47,
    },
    "scratch": 0x48,
};

Numark.ledOn = function(control) {
    midi.sendShortMsg(0x90, control, 0x7f);
};
Numark.ledOff = function(control) {
    midi.sendShortMsg(0x90, control, 0x00);
};

Numark.init = function() {
    this.touching = {
        "[Channel1]": false,
        "[Channel2]": false,
    };
    // tell controller to send current state of knobs and crossfader
    midi.sendSysexMsg(this.sysex, this.sysex.length);
    // change this line to toggle the default scratch mode:
    this.scratchMode = true;
    // initialize LEDs
    if (this.scratchMode) {
        this.ledOn(this.buttons.scratch);
    } else {
        this.ledOff(this.buttons.scratch);
    }
    if (engine.getParameter("[Channel1]", "play")) {
        this.ledOn(this.buttons["[Channel1]"].play);
    } else {
        this.ledOff(this.buttons["[Channel1]"].play);
    }
    if (engine.getParameter("[Channel2]", "play")) {
        this.ledOn(this.buttons["[Channel2]"].play);
    } else {
        this.ledOff(this.buttons["[Channel2]"].play);
    }
};

Numark.shutdown = function() {
    this.ledOff(this.buttons["[Channel1]"].play);
    this.ledOff(this.buttons["[Channel1]"].sync);
    this.ledOff(this.buttons["[Channel2]"].play);
    this.ledOff(this.buttons["[Channel2]"].sync);
    this.ledOff(this.buttons.scratch);
};

Numark.toggleScratchMode = function() {
    if (this.scratchMode) {
        this.scratchMode = false;
        this.ledOff(this.buttons.scratch);
        // cancel any active scratches to prevent weird behavior
        engine.scratchDisable(1);
        engine.scratchDisable(2);
    } else {
        this.scratchMode = true;
        this.ledOn(this.buttons.scratch);
    }
};

Numark.jogTouch = function(_channel, _control, value, _status, group) {
    var deckN = script.deckFromGroup(group);
    if (value >= 64) {
        this.touching[group] = true;
        if (this.scratchMode) {
            var alpha = 1.0/8;
            var beta = alpha/32;
            engine.scratchEnable(deckN, this.jogResolution, 33+(1.0/3), alpha, beta);
        }
    } else {
        this.touching[group] = false;
        engine.scratchDisable(deckN);
    }
};

Numark.jog = function(_channel, _control, value, _status, group) {
    if (!this.touching[group]) {
        return;
    }
    // value is centered around 0
    if (value >= 64) {
        value -= 128;
    }
    var deckN = script.deckFromGroup(group);
    if (this.scratchMode) {
        if (engine.isScratching(deckN)) {
            engine.scratchTick(deckN, value);
        }
    } else {
        if (engine.getParameter(group, "play")) {
            // pitch bend while playing
            engine.setValue(group, "jog", value);
        } else {
            // search while paused
            var position = engine.getValue(group, "playposition");
            position += value * 0.0002;
            if (position < 0) {
                position = 0;
            }
            engine.setValue(group, "playposition", position);
        }
    }
};
