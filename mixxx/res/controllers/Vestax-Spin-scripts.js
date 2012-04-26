// Script file for Mixxx Vestax Spin mapping
// Bill Good, Oct 31, 2010
// Parts of addButton and handleEvent are the work of Anders Gunnarsson

// move vu's to script? or fix bug in midi output saving

VestaxSpin = new function() {
    this.group = "[Master]";
    this.decks = [];
    this.buttons = [];
    this.controls = [];
    this.lights = [];
}

VestaxSpin.DECK_LIGHTS = [0x32, 0x35, 0x33, 0x24, 0x25, 0x46, 0x42, 0x21, 0x20, 0x29, 0x2a, 0x2b,
    0x2c, 0x2d];
VestaxSpin.MISC_LIGHTS = [0x26, 0x29, 0x28, 0x2a];
VestaxSpin.SCRATCH_TIMER_PERIOD = 20; // timer period in milliseconds,
// mixxx enforces a minimum of 20.

VestaxSpin.init = function(id) {
    VestaxSpin.decks = {
        "L": new VestaxSpin.Deck(1,"[Channel1]"),
        "R": new VestaxSpin.Deck(2,"[Channel2]")
    };

    VestaxSpin.addButton("songlist", new VestaxSpin.Button(2, 0x26, true), "handleSongList");
    VestaxSpin.lights["killwheels"] = new VestaxSpin.Light(2, 0x2a);
    VestaxSpin.lights["killwheels"].off();

    // clear everything
    for (var light in VestaxSpin.DECK_LIGHTS) {
        new VestaxSpin.Light(0, VestaxSpin.DECK_LIGHTS[light]).off();
        new VestaxSpin.Light(1, VestaxSpin.DECK_LIGHTS[light]).off();
        for (var j = 0; j < 10000000; ++j);
    }
    for (var light in VestaxSpin.MISC_LIGHTS) {
        new VestaxSpin.Light(2, VestaxSpin.MISC_LIGHTS[light]).off();
        for (var j = 0; j < 10000000; ++j);
    }
    // make the eqs do some pretty things
/*    var vu = [0x29, 0x2a, 0x2b, 0x2c, 0x2d];
    for (var light in vu) {
        new VestaxSpin.Light(0, vu[light]).on();
        new VestaxSpin.Light(1, vu[light]).on();
        for (var j = 0; j < 20000000; ++j);
    }
    for (var light in vu) {
        new VestaxSpin.Light(0, vu[light]).off();
        new VestaxSpin.Light(1, vu[light]).off();
        for (var j = 0; j < 20000000; ++j);
    }*/

}

VestaxSpin.shutdown = function(id) {
    // clear everything
    for (var light in VestaxSpin.DECK_LIGHTS) {
        new VestaxSpin.Light(0, VestaxSpin.DECK_LIGHTS[light]).off();
        new VestaxSpin.Light(1, VestaxSpin.DECK_LIGHTS[light]).off();
        for (var j = 0; j < 10000000; ++j);
    }
    for (var light in VestaxSpin.MISC_LIGHTS) {
        new VestaxSpin.Light(2, VestaxSpin.MISC_LIGHTS[light]).off();
        for (var j = 0; j < 10000000; ++j);
    }
}

VestaxSpin.GetDeck = function(group) {
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

VestaxSpin.addButton = function(buttonName, button, eventHandler) {
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

VestaxSpin.handleEvent = function(channel, control, value, status, group) {
    var deck = VestaxSpin.GetDeck(group);
    if (deck != null) {
        deck.handleEvent(channel, control, value, status, group);
    } 
    try {
        var buttons = VestaxSpin.controls[control];
    } catch (ex) {
        return;
    }
    for (var button in buttons) {
        buttons[button].handleEvent(value);
    }
}

VestaxSpin.ButtonState = {"released": 0x00, "pressed": 0x7F};

VestaxSpin.Button = function(channel, control, makeLight, lightControl) {
    this.channel = channel;
    this.control = control;
    this.group = null;
    this.state = VestaxSpin.ButtonState.released;
    this.handler = null;
    this.parent = null;
    if (makeLight) {
        if (lightControl) {
            this.light = new VestaxSpin.Light(this.channel, lightControl);
        } else {
            this.light = new VestaxSpin.Light(this.channel, this.control);
        }
    } else {
        this.light = null;
    }
}

VestaxSpin.LightState = {"on": 0x7f, "off": 0x00};

VestaxSpin.Light = function(channel, control) {
    this.channel = channel;
    this.control = control;
    this.state = VestaxSpin.LightState.off;
    this.on = function() {
        midi.sendShortMsg(0x90 + this.channel, this.control, VestaxSpin.LightState.on);
        this.state = VestaxSpin.LightState.on;
    }
    this.off = function() {
        midi.sendShortMsg(0x90 + this.channel, this.control, VestaxSpin.LightState.off);
        this.state = VestaxSpin.LightState.off;
    }
}

VestaxSpin.Button.prototype.handleEvent = function(value) {
    this.state = value;
    this.handler();
}

VestaxSpin.Deck = function(deckNum, group) {
    this.deckNum = deckNum;
    this.group = group;
    this.vinylMode = false;
    this.buttons = [];
    this.controls = [];
    this.lights = [];
    this.addButton("loop_open", new VestaxSpin.Button(deckNum-1, 0x21, true), "handleLoopOpen");
    this.addButton("loop_close", new VestaxSpin.Button(deckNum-1, 0x42, true), "handleLoopClose");
    this.addButton("sync", new VestaxSpin.Button(deckNum-1, 0x46, true), "handleSync");
    this.addButton("cue", new VestaxSpin.Button(deckNum-1, 0x35, true), "handleCue");
    this.addButton("cup", new VestaxSpin.Button(deckNum-1, 0x33, true), "handleCup");
    this.addButton("filter", new VestaxSpin.Button(deckNum-1, 0x24, true), "handleFilter");
    this.addButton("back", new VestaxSpin.Button(deckNum-1, 0x36, true, 0x32), "handleBack");
    this.addButton("rw", new VestaxSpin.Button(deckNum-1, 0x37, true, 0x35), "handleRW");
    this.addButton("ff", new VestaxSpin.Button(deckNum-1, 0x38, true, 0x33), "handleFF");
    this.addButton("wheeltouch", new VestaxSpin.Button(deckNum-1, 0x2e), "handleWheelTouch");
    // is this needed on the spin? I don't think we get a different control number
    // from touch if the scratch button light is active like on typhoon
    //this.addButton("wheeltouchfilter", new VestaxSpin.Button(deckNum-1, 0x2f), "handleWheelTouchFilter");
    this.addButton("jog", new VestaxSpin.Button(deckNum-1, 0x10), "handleWheel");
    // spin doesn't send different control number when the scratch button light
    // is enabled like the typhoon does
    //this.addButton("scratch", new VestaxSpin.Button(deckNum-1, 0x11), "handleScratch");

    this.lights["vu1"] = new VestaxSpin.Light(deckNum-1, 0x29);
    this.lights["vu2"] = new VestaxSpin.Light(deckNum-1, 0x2a);
    this.lights["vu3"] = new VestaxSpin.Light(deckNum-1, 0x2b);
    this.lights["vu4"] = new VestaxSpin.Light(deckNum-1, 0x2c);
    this.lights["vu5"] = new VestaxSpin.Light(deckNum-1, 0x2d);
}

VestaxSpin.Deck.prototype.addButton = VestaxSpin.addButton;

VestaxSpin.Deck.prototype.handleEvent = function(channel, control, value, status, group) {
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

VestaxSpin.Button.prototype.handleSongList = function() {
    if (this.state == VestaxSpin.ButtonState.pressed) {
        engine.setValue("[Playlist]", "SelectNextPlaylist", 1);
        this.light.on();
    } else {
        engine.setValue("[Playlist]", "SelectNextPlaylist", 0);
        this.light.off();
    }
}

VestaxSpin.Button.prototype.handleLoopOpen = function() {
    if (this.state == VestaxSpin.ButtonState.pressed) {
        engine.setValue(this.group, "loop_in", 1);
        this.light.on();
    } else {
        engine.setValue(this.group, "loop_in", 0);
        this.light.off();
    }
}

VestaxSpin.Button.prototype.handleLoopClose = function() {
    if (this.state == VestaxSpin.ButtonState.pressed) {
        engine.setValue(this.group, "loop_out", 1);
        this.light.on();
    } else {
        engine.setValue(this.group, "loop_out", 0);
        this.light.off();
    }
}

VestaxSpin.Button.prototype.handleSync = function() {
    if (this.state == VestaxSpin.ButtonState.pressed) {
        engine.setValue(this.group, "beatsync", 1);
        this.light.on();
    } else {
        engine.setValue(this.group, "beatsync", 0);
        this.light.off();
    }
}

VestaxSpin.Button.prototype.handleCue = function() {
    if (this.state == VestaxSpin.ButtonState.pressed) {
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

VestaxSpin.Button.prototype.handleCup = function() {
    if (this.state == VestaxSpin.ButtonState.pressed) {
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

VestaxSpin.Button.prototype.handleFilter = function() {
    if (this.state == VestaxSpin.ButtonState.released) return;
    if (this.parent.vinylMode == false) {
       this.parent.vinylMode = true;
       this.light.on();
    } else {
       this.parent.vinylMode = false;
       this.light.off();
        // kill scratch
        if (this.parent.buttons["wheeltouch"].timer > 0) {
            engine.stopTimer(this.parent.buttons["wheeltouch"].timer);
            this.parent.buttons["wheeltouch"].timer = 0;
        }
        engine.scratchDisable(this.parent.deckNum);
    }
}

VestaxSpin.Button.prototype.handleBack = function() {
    if (this.state == VestaxSpin.ButtonState.pressed) {
        engine.setValue(this.group, "play", 0);
        engine.setValue(this.group, "playposition", 0);
        this.light.on();
    } else {
        this.light.off();
    }
}

VestaxSpin.Button.prototype.handleRW = function() {
    if (this.state == VestaxSpin.ButtonState.pressed) {
        engine.setValue(this.group, "back", 1);
        this.light.on();
    } else {
        engine.setValue(this.group, "back", 0);
        this.light.off();
    }
}

VestaxSpin.Button.prototype.handleFF = function() {
    if (this.state == VestaxSpin.ButtonState.pressed) {
        engine.setValue(this.group, "fwd", 1);
        this.light.on();
    } else {
        engine.setValue(this.group, "fwd", 0);
        this.light.off();
    }
}

VestaxSpin.Button.prototype.handleWheelTouch = function() {
   if (this.parent.vinylMode && this.state == VestaxSpin.ButtonState.pressed) {
       // disable keylock on scratch
       //this.keylock = engine.getValue(this.group, "keylock");
        if (this.timer > 0) {
            engine.stopTimer(this.timer);
            this.timer = 0;
        }
       engine.scratchEnable(this.parent.deckNum, 300, 33+(1.0/3), 1.0/8, (1.0/8)/32);
   } else {
        this.callback = function() {
            var last_fire = (new Date()).valueOf() - VestaxSpin.SCRATCH_TIMER_PERIOD;
            if (this.lastTick < last_fire) {
                engine.scratchDisable(this.parent.deckNum);
                engine.stopTimer(this.timer);
                this.timer = 0;
            }
        }
        this.timer = engine.beginTimer(VestaxSpin.SCRATCH_TIMER_PERIOD,
            "VestaxSpin.GetDeck(\"" + this.group + "\").buttons[\"wheeltouch\"].callback()");
   }
}

VestaxSpin.Button.prototype.handleWheel = function() {
    if (engine.getValue(this.group, "scratch2_enable")) {
        engine.scratchTick(this.parent.deckNum, this.state - 0x40);
        this.parent.buttons["wheeltouch"].lastTick = (new Date()).valueOf();
    } else {
        engine.setValue(this.group, "jog", this.state - 0x40);
    }
}

