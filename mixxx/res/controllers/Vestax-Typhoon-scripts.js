// Script file for Mixxx Vestax Typhoon mapping
// Bill Good, Oct 31, 2010
// Parts of addButton and handleEvent are the work of Anders Gunnarsson

// move vu's to script? or fix bug in midi output saving

VestaxTyphoon = new function() {
    this.group = "[Master]";
    this.decks = [];
    this.buttons = [];
    this.controls = [];
    this.lights = [];
}

VestaxTyphoon.DECK_LIGHTS = [0x32, 0x35, 0x33, 0x24, 0x25, 0x46, 0x42, 0x21, 0x20, 0x29, 0x2a, 0x2b,
    0x2c, 0x2d];
VestaxTyphoon.MISC_LIGHTS = [0x26, 0x29, 0x28, 0x2a];
VestaxTyphoon.SCRATCH_TIMER_PERIOD = 20; // timer period in milliseconds,
// mixxx enforces a minimum of 20.

VestaxTyphoon.init = function(id) {
    VestaxTyphoon.decks = {
        "L": new VestaxTyphoon.Deck(1,"[Channel1]"),
        "R": new VestaxTyphoon.Deck(2,"[Channel2]")
    };

    VestaxTyphoon.addButton("songlist", new VestaxTyphoon.Button(2, 0x26, true), "handleSongList");
    VestaxTyphoon.lights["killwheels"] = new VestaxTyphoon.Light(2, 0x2a);
    VestaxTyphoon.lights["killwheels"].off();

    // clear everything
    for (var light in VestaxTyphoon.DECK_LIGHTS) {
        new VestaxTyphoon.Light(0, VestaxTyphoon.DECK_LIGHTS[light]).off();
        new VestaxTyphoon.Light(1, VestaxTyphoon.DECK_LIGHTS[light]).off();
        for (var j = 0; j < 10000000; ++j);
    }
    for (var light in VestaxTyphoon.MISC_LIGHTS) {
        new VestaxTyphoon.Light(2, VestaxTyphoon.MISC_LIGHTS[light]).off();
        for (var j = 0; j < 10000000; ++j);
    }
    // make the eqs do some pretty things
/*    var vu = [0x29, 0x2a, 0x2b, 0x2c, 0x2d];
    for (var light in vu) {
        new VestaxTyphoon.Light(0, vu[light]).on();
        new VestaxTyphoon.Light(1, vu[light]).on();
        for (var j = 0; j < 20000000; ++j);
    }
    for (var light in vu) {
        new VestaxTyphoon.Light(0, vu[light]).off();
        new VestaxTyphoon.Light(1, vu[light]).off();
        for (var j = 0; j < 20000000; ++j);
    }*/
}

VestaxTyphoon.shutdown = function(id) {
    // clear everything
    for (var light in VestaxTyphoon.DECK_LIGHTS) {
        new VestaxTyphoon.Light(0, VestaxTyphoon.DECK_LIGHTS[light]).off();
        new VestaxTyphoon.Light(1, VestaxTyphoon.DECK_LIGHTS[light]).off();
        for (var j = 0; j < 10000000; ++j);
    }
    for (var light in VestaxTyphoon.MISC_LIGHTS) {
        new VestaxTyphoon.Light(2, VestaxTyphoon.MISC_LIGHTS[light]).off();
        for (var j = 0; j < 10000000; ++j);
    }
}

VestaxTyphoon.GetDeck = function(group) {
    var groupToDeck = {
        "[Channel1]": "L",
        "[Channel2]": "R",
    };
    try {
        return this.decks[groupToDeck[group]];
    } catch (ex) {
        return null;
    }
}

VestaxTyphoon.addButton = function(buttonName, button, eventHandler) {
    button.group = this.group;
    button.parent = this;

    if (eventHandler) {
       var executionEnvironment = button;
       function handler(value) {
          try {
              executionEnvironment[eventHandler]();
          } catch (ex) {
              print("exception in executing handler for button " + buttonName + ": " + ex);
          }
       }
       button.handler = handler;
    }
    this.buttons[buttonName] = button;
    var control_map = this.controls[button.control];
    if (control_map) {
        control_map.push(button);
    } else {
        this.controls[button.control] = [button];
    }
}

VestaxTyphoon.handleEvent = function(channel, control, value, status, group) {
    var deck = VestaxTyphoon.GetDeck(group);
    if (deck != null) {
        deck.handleEvent(channel, control, value, status, group);
    } 
    try {
        var buttons = VestaxTyphoon.controls[control];
    } catch (ex) {
        return;
    }
    for (var button in buttons) {
        buttons[button].handleEvent(value);
    }
}

VestaxTyphoon.ButtonState = {"released": 0x00, "pressed": 0x7F};

VestaxTyphoon.Button = function(channel, control, makeLight, lightControl) {
    this.channel = channel;
    this.control = control;
    this.group = null;
    this.state = VestaxTyphoon.ButtonState.released;
    this.handler = null;
    this.parent = null;
    if (makeLight) {
        if (lightControl) {
            this.light = new VestaxTyphoon.Light(this.channel, lightControl);
        } else {
            this.light = new VestaxTyphoon.Light(this.channel, this.control);
        }
    } else {
        this.light = null;
    }
}

VestaxTyphoon.LightState = {"on": 0x7f, "off": 0x00};

VestaxTyphoon.Light = function(channel, control) {
    this.channel = channel;
    this.control = control;
    this.state = VestaxTyphoon.LightState.off;
    this.on = function() {
        midi.sendShortMsg(0x90 + this.channel, this.control, VestaxTyphoon.LightState.on);
        this.state = VestaxTyphoon.LightState.on;
    }
    this.off = function() {
        midi.sendShortMsg(0x90 + this.channel, this.control, VestaxTyphoon.LightState.off);
        this.state = VestaxTyphoon.LightState.off;
    }
}

VestaxTyphoon.Button.prototype.handleEvent = function(value) {
    this.state = value;
    this.handler();
}

VestaxTyphoon.Deck = function(deckNum, group) {
    this.deckNum = deckNum;
    this.group = group;
    this.vinylMode = true;
    this.scratching = false;
    this.buttons = [];
    this.controls = [];
    this.lights = [];
    this.addButton("loop_open", new VestaxTyphoon.Button(deckNum-1, 0x21, true), "handleLoopOpen");
    this.addButton("loop_close", new VestaxTyphoon.Button(deckNum-1, 0x42, true), "handleLoopClose");
    this.addButton("sync", new VestaxTyphoon.Button(deckNum-1, 0x46, true), "handleSync");
    this.addButton("cue", new VestaxTyphoon.Button(deckNum-1, 0x35, true), "handleCue");
    this.addButton("cup", new VestaxTyphoon.Button(deckNum-1, 0x33, true), "handleCup");
    this.addButton("filter", new VestaxTyphoon.Button(deckNum-1, 0x24, true), "handleFilter");
    this.addButton("back", new VestaxTyphoon.Button(deckNum-1, 0x36, true, 0x32), "handleBack");
    this.addButton("rw", new VestaxTyphoon.Button(deckNum-1, 0x37, true, 0x35), "handleRW");
    this.addButton("ff", new VestaxTyphoon.Button(deckNum-1, 0x38, true, 0x33), "handleFF");
// this next one is basically useless since we don't need touch for jog, maybe useful later?
//    this.addButton("wheeltouch", new VestaxTyphoon.Button(deckNum-1, 0x2e), "handleWheelTouch");
    this.addButton("wheeltouchfilter", new VestaxTyphoon.Button(deckNum-1, 0x2f), "handleWheelTouchFilter");
    this.addButton("jog", new VestaxTyphoon.Button(deckNum-1, 0x10), "handleWheel");
    this.addButton("scratch", new VestaxTyphoon.Button(deckNum-1, 0x11), "handleWheel");

    this.lights["vu1"] = new VestaxTyphoon.Light(deckNum-1, 0x29);
    this.lights["vu2"] = new VestaxTyphoon.Light(deckNum-1, 0x2a);
    this.lights["vu3"] = new VestaxTyphoon.Light(deckNum-1, 0x2b);
    this.lights["vu4"] = new VestaxTyphoon.Light(deckNum-1, 0x2c);
    this.lights["vu5"] = new VestaxTyphoon.Light(deckNum-1, 0x2d);
}

VestaxTyphoon.Deck.prototype.addButton = VestaxTyphoon.addButton;

VestaxTyphoon.Deck.prototype.handleEvent = function(channel, control, value, status, group) {
    try {
        var buttons = this.controls[control];
    } catch (ex) {
        return;
    }
    if (buttons) {
        for (var button in buttons) {
            buttons[button].handleEvent(value);
        }
    }
}

VestaxTyphoon.Button.prototype.handleSongList = function() {
    if (this.state == VestaxTyphoon.ButtonState.pressed) {
        engine.setValue("[Playlist]", "SelectNextPlaylist", 1);
        this.light.on();
    } else {
        engine.setValue("[Playlist]", "SelectNextPlaylist", 0);
        this.light.off();
    }
}

VestaxTyphoon.Button.prototype.handleLoopOpen = function() {
    if (this.state == VestaxTyphoon.ButtonState.pressed) {
        engine.setValue(this.group, "loop_in", 1);
        this.light.on();
    } else {
        engine.setValue(this.group, "loop_in", 0);
        this.light.off();
    }
}

VestaxTyphoon.Button.prototype.handleLoopClose = function() {
    if (this.state == VestaxTyphoon.ButtonState.pressed) {
        engine.setValue(this.group, "loop_out", 1);
        this.light.on();
    } else {
        engine.setValue(this.group, "loop_out", 0);
        this.light.off();
    }
}

VestaxTyphoon.Button.prototype.handleSync = function() {
    if (this.state == VestaxTyphoon.ButtonState.pressed) {
        engine.setValue(this.group, "beatsync", 1);
        this.light.on();
    } else {
        engine.setValue(this.group, "beatsync", 0);
        this.light.off();
    }
}

VestaxTyphoon.Button.prototype.handleCue = function() {
    if (this.state == VestaxTyphoon.ButtonState.pressed) {
        engine.setValue(this.group, "cue_default", 1);
        this.light.on();
    } else {
        engine.setValue(this.group, "cue_default", 0);
        // shut off rw so that we don't get stuck in rw if the user lets go
        // of shift before letting go of cue/rw
        engine.setValue(this.group, "back", 0);
        this.light.off();
    }
}

VestaxTyphoon.Button.prototype.handleCup = function() {
    if (this.state == VestaxTyphoon.ButtonState.pressed) {
        engine.setValue(this.group, "cue_goto", 1);
        this.light.on();
    } else {
        engine.setValue(this.group, "cue_goto", 0);
        // shut off ff so that we don't get stuck in ff if the user lets go
        // of shift before letting go of cup/ff
        engine.setValue(this.group, "fwd", 0);
        this.light.off();
    }
}

VestaxTyphoon.Button.prototype.handleFilter = function() {
    if (this.state == VestaxTyphoon.ButtonState.released) return;
    if (this.light.state == VestaxTyphoon.LightState.off) {
        this.light.on();
    } else {
        this.light.off();
        // kill scratch
        if (this.parent.buttons["wheeltouchfilter"].timer > 0) {
            engine.stopTimer(this.parent.buttons["wheeltouchfilter"].timer);
            this.parent.buttons["wheeltouchfilter"].timer = 0;
        }
        engine.scratchDisable(this.parent.deckNum);
    }
}

VestaxTyphoon.Button.prototype.handleBack = function() {
    if (this.state == VestaxTyphoon.ButtonState.pressed) {
        engine.setValue(this.group, "play", 0);
        engine.setValue(this.group, "playposition", 0);
        this.light.on();
    } else {
        this.light.off();
    }
}

VestaxTyphoon.Button.prototype.handleRW = function() {
    if (this.state == VestaxTyphoon.ButtonState.pressed) {
        engine.setValue(this.group, "back", 1);
        this.light.on();
    } else {
        engine.setValue(this.group, "back", 0);
        this.light.off();
    }
}

VestaxTyphoon.Button.prototype.handleFF = function() {
    if (this.state == VestaxTyphoon.ButtonState.pressed) {
        engine.setValue(this.group, "fwd", 1);
        this.light.on();
    } else {
        engine.setValue(this.group, "fwd", 0);
        this.light.off();
    }
}

VestaxTyphoon.Button.prototype.handleWheelTouchFilter = function() {
    if (this.state == VestaxTyphoon.ButtonState.pressed) {
        // disable keylock on scratch
        //this.keylock = engine.getValue(this.group, "keylock");
        if (this.timer > 0) {
            engine.stopTimer(this.timer);
            this.timer = 0;
        }
        engine.scratchEnable(this.parent.deckNum, 300, 33+(1.0/3), 1.0/8, (1.0/8)/32);
    } else {
        this.callback = function() {
            var last_fire = (new Date()).valueOf() - VestaxTyphoon.SCRATCH_TIMER_PERIOD;
            if (this.lastTick < last_fire) {
                engine.scratchDisable(this.parent.deckNum);
                engine.stopTimer(this.timer);
                this.timer = 0;
            }
        }
        this.timer = engine.beginTimer(VestaxTyphoon.SCRATCH_TIMER_PERIOD,
            "VestaxTyphoon.GetDeck(\"" + this.group + "\").buttons[\"wheeltouchfilter\"].callback()");
    }
}

VestaxTyphoon.Button.prototype.handleWheel = function() {
    if (engine.getValue(this.group, "scratch2_enable")) {
        engine.scratchTick(this.parent.deckNum, this.state - 0x40);
        this.parent.buttons["wheeltouchfilter"].lastTick = (new Date()).valueOf();
    } else {
        engine.setValue(this.group, "jog", this.state - 0x40);
    }
}

