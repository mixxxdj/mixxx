////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
////////////////////////////////////////////////////////////////////////

/**
 * American Audio VMS2 controller script v1.12.0
 * Copyright (C) 2010  Anders Gunnarsson
 * Copyright (C) 2011-2012  Sean M. Pappalardo
 * Copyright (C) 2012-2016  Stefan Nuernberger
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/
VMS2 = new Controller();

VMS2.RateRanges = [0.08, 0.10, 0.30, 1.00];
//----
VMS2.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time

VMS2.numHotCues = 6;
// Button control number to hot cue mapping
VMS2.hotCues = { 0x12:1, 0x13:2, 0x14:3, 0x15:4, 0x17:5, 0x18:6,
    0x34:1, 0x35:2, 0x36:3, 0x37:4, 0x39:5, 0x3A:6 };

VMS2.initControls = [   ["Channel", "hotcue_x_enabled"],
                     ["Channel", "quantize"],
                     ["Channel", "beatsync"],
                     ["Channel", "loop_in"],
                     ["Channel", "loop_out"],
                     ["Channel", "loop_enabled"],
                     ["Channel", "loop_halve"],
                     ["Channel", "loop_double"],
                     ["Channel", "play"],
                     ["Channel", "cue_default"],
                     ["Channel", "back"],
                     ["Channel", "fwd"],
                     ["Channel", "keylock"],
                     ["Channel", "rate_temp_up"],
                     ["Channel", "rate_temp_down"],
        ];

VMS2.init = function (id) {    // called when the MIDI device is opened & set up
    VMS2.id = id;   // Store the ID of this device for later use
    for (var i=12; i<=77; i++) midi.sendShortMsg(0x80,i,0x00);  // Extinquish all LEDs

    // Enable soft-takeover for all direct hardware controls
    //  (Many of these are mapped in the XML directly so these have no effect.
    //  Here for completeness in case any eventually move to script.)
    //    engine.softTakeover("[Channel1]","rate",true);
    //    engine.softTakeover("[Channel1]","volume",true);
    //    engine.softTakeover("[Channel1]","pregain",true);
    //    engine.softTakeover("[Channel1]","filterHigh",true);
    //    engine.softTakeover("[Channel1]","filterMed",true);
    //    engine.softTakeover("[Channel1]","filterLow",true);
    //    engine.softTakeover("[Master]","crossfader",true);
    //    engine.softTakeover("[Channel2]","rate",true);
    //    engine.softTakeover("[Channel2]","volume",true);
    //    engine.softTakeover("[Channel2]","pregain",true);
    //    engine.softTakeover("[Channel2]","filterHigh",true);
    //    engine.softTakeover("[Channel2]","filterMed",true);
    //    engine.softTakeover("[Channel2]","filterLow",true);

    // Flash the sync button matching the beatgrid
    engine.connectControl("[Channel1]","beat_active","VMS2.beatflash");
    engine.connectControl("[Channel2]","beat_active","VMS2.beatflash");
    // Flash the play button in pause mode
    engine.connectControl("[Channel1]","play_indicator","VMS2.playlight");
    engine.connectControl("[Channel2]","play_indicator","VMS2.playlight");

    engine.trigger("[Channel1]",'play_indicator');
    engine.trigger("[Channel2]",'play_indicator');

    print("American Audio "+VMS2.id+" initialized.");
};

VMS2.shutdown = function () {
    for (var i=12; i<=77; i++) midi.sendShortMsg(0x80,i,0x00);  // Extinquish all LEDs
    print("American Audio "+VMS2.id+" shut down.");
};

VMS2.Button = Button;

VMS2.Button.prototype.setLed = function(ledState) {
    if(ledState === LedState.on) {
        midi.sendShortMsg(0x90,this.controlId,LedState.on);
    } else {
        midi.sendShortMsg(0x80,this.controlId,LedState.off);
    }
};

VMS2.Deck = Deck;
VMS2.Deck.jogMsb = 0x00;
VMS2.Deck.scratchMode = false;
VMS2.Deck.hotCueDeleted = false;
VMS2.Deck.keylockButton = false;
VMS2.Deck.vinylButton = false;
VMS2.Deck.hotCuePressed = false;
VMS2.Deck.pitchLock = false;
VMS2.Deck.rangeIdx = 0;
VMS2.Deck.playTimer = 0;

VMS2.Deck.prototype.rateRangeHandler = function(value) {
    if(value === ButtonState.pressed) {
        this.Buttons.RateRange.setLed(LedState.on);
        if (isNaN(this.rangeIdx)) {
            this.rangeIdx = 0;
        }
        this.rangeIdx = (this.rangeIdx + 1) % VMS2.RateRanges.length;
        engine.setValue(this.group,"rateRange",VMS2.RateRanges[this.rangeIdx]);
        // Update the screen display
        engine.trigger(this.group,"rate");
    } else {
        this.Buttons.RateRange.setLed(LedState.off);
    }
};

VMS2.Deck.prototype.pitchCenterHandler = function(value) {
    // Reset pitch only on entrance to center position
    if(value === ButtonState.pressed) {
        this.pitchLock = true;
        engine.setValue(this.group, "rate", 0);
    } else {
        this.pitchLock = false;
    }
};

VMS2.Deck.prototype.pauseHandler = function(value) {
    engine.setValue(this.group, "play", 0);
};

VMS2.Deck.prototype.jogTouchHandler = function(value) {
    if((value === ButtonState.pressed) && this.scratchMode) {
        engine.scratchEnable(this.deckNumber, 3000, 45, 1.0/8, (1.0/8)/32);
    } else {
        engine.scratchDisable(this.deckNumber);
    }
};

VMS2.Deck.prototype.jogMove = function(lsbValue) {
    var jogValue = (this.jogMsb << 7) + lsbValue;

    if(!isNaN(this.previousJogValue)) {
        var offset = jogValue - this.previousJogValue;

        if(offset > 8192) {
            offset = offset - 16384;
        } else if(offset < -8192) {
            offset = offset + 16384;
        }

        if(this.scratchMode) {
            engine.scratchTick(this.deckNumber, offset);
        } else {
            engine.setValue(this.group,"jog", offset / 40.0);
        }
    }
    this.previousJogValue = jogValue;
};

VMS2.Deck.prototype.vinylButtonHandler = function(value) {
    if(value === ButtonState.pressed) {
        this.vinylButton = true;
        this.hotCueDeleted = false;
    } else {
        // Toggle scratch mode only on release and only if a hot cue wasn't deleted
        if (!this.hotCueDeleted) {
            // vinyl button toggles scratchmode
            this.scratchMode = !this.scratchMode;
            this.Buttons.Vinyl.setLed(this.scratchMode ? LedState.on : LedState.off);
        }
        this.vinylButton = false;
        // Force keylock up too since they're they same physical button
        //  (This prevents keylock getting stuck down if shift is released first)
        this.keylockButton = false;
    }
};

VMS2.Deck.prototype.keyLockButtonHandler = function(value) {
    if(value === ButtonState.pressed) {
        this.keylockButton = true;
        this.hotCueDeleted = false;
    } else {
        // Toggle keylock only on release and only if a hot cue wasn't deleted
        if (!this.hotCueDeleted) {
            var currentKeylock = engine.getValue(this.group, "keylock");
            engine.setValue(this.group, "keylock", !currentKeylock);
        }
        this.keylockButton = false;
        // Force vinyl up too since they're they same physical button
        //  (This prevents vinyl getting stuck down if shift is pressed while
        //  vinyl is held down)
        this.vinylButton = false;
    }
};

VMS2.Deck.prototype.killHighHandler = function(value) {
    if(value === ButtonState.pressed) {
        var filterStatus = engine.getValue(this.group, "filterHighKill");
        if(filterStatus) {
            this.Buttons.KillHigh.setLed(LedState.off);
            engine.setValue(this.group, "filterHighKill", 0);
        } else {
            this.Buttons.KillHigh.setLed(LedState.on);
            engine.setValue(this.group, "filterHighKill", 1);
        }
    }
};

VMS2.Deck.prototype.killMidHandler = function(value) {
    if(value === ButtonState.pressed) {
        var filterStatus = engine.getValue(this.group, "filterMidKill");
        if(filterStatus) {
            this.Buttons.KillMid.setLed(LedState.off);
            engine.setValue(this.group, "filterMidKill", 0);
        } else {
            this.Buttons.KillMid.setLed(LedState.on);
            engine.setValue(this.group, "filterMidKill", 1);
        }
    }
};

VMS2.Deck.prototype.killLowHandler = function(value) {
    if(value === ButtonState.pressed) {
        var filterStatus = engine.getValue(this.group, "filterLowKill");
        if(filterStatus) {
            this.Buttons.KillLow.setLed(LedState.off);
            engine.setValue(this.group, "filterLowKill", 0);
        } else {
            this.Buttons.KillLow.setLed(LedState.on);
            engine.setValue(this.group, "filterLowKill", 1);
        }
    }
};

VMS2.Decks = {"Left":new VMS2.Deck(1,"[Channel1]"), "Right":new VMS2.Deck(2,"[Channel2]")};
VMS2.GroupToDeck = {"[Channel1]":"Left", "[Channel2]":"Right"};

VMS2.getDeck = function(group) {
    try {
        return VMS2.Decks[VMS2.GroupToDeck[group]];
    } catch(ex) {
        return null;
    }
};

VMS2.Decks.Left.addButton("RateRange", new VMS2.Button(0x11), "rateRangeHandler");
VMS2.Decks.Left.addButton("PitchCenter", new VMS2.Button(), "pitchCenterHandler");
VMS2.Decks.Left.addButton("Play", new VMS2.Button(0x0d), "none_use_default");
VMS2.Decks.Left.addButton("Pause", new VMS2.Button(0x0e), "pauseHandler");
VMS2.Decks.Left.addButton("JogTouch", new VMS2.Button(), "jogTouchHandler");
VMS2.Decks.Left.addButton("Vinyl", new VMS2.Button(0x27), "vinylButtonHandler");
VMS2.Decks.Left.addButton("KeyLock", new VMS2.Button(), "keyLockButtonHandler");
VMS2.Decks.Left.addButton("KillHigh", new VMS2.Button(0x25), "killHighHandler");
VMS2.Decks.Left.addButton("KillMid", new VMS2.Button(0x24), "killMidHandler");
VMS2.Decks.Left.addButton("KillLow", new VMS2.Button(0x23), "killLowHandler");

VMS2.Decks.Right.addButton("RateRange", new VMS2.Button(0x33), "rateRangeHandler");
VMS2.Decks.Right.addButton("PitchCenter", new VMS2.Button(), "pitchCenterHandler");
VMS2.Decks.Right.addButton("Play", new VMS2.Button(0x2f), "none_use_default");
VMS2.Decks.Right.addButton("Pause", new VMS2.Button(0x30), "pauseHandler");
VMS2.Decks.Right.addButton("JogTouch", new VMS2.Button(), "jogTouchHandler");
VMS2.Decks.Right.addButton("Vinyl", new VMS2.Button(0x49), "vinylButtonHandler");
VMS2.Decks.Right.addButton("KeyLock", new VMS2.Button(), "keyLockButtonHandler");
VMS2.Decks.Right.addButton("KillHigh", new VMS2.Button(0x47), "killHighHandler");
VMS2.Decks.Right.addButton("KillMid", new VMS2.Button(0x46), "killMidHandler");
VMS2.Decks.Right.addButton("KillLow", new VMS2.Button(0x45), "killLowHandler");


//Mapping functions
VMS2.rate_range = function(channel, control, value, status, group) {
    var deck = VMS2.getDeck(group);
    deck.Buttons.RateRange.handleEvent(value);
};

VMS2.beatflash = function(value, group, control) {
    var deck = VMS2.getDeck(group);
    deck.Buttons.RateRange.setLed(value ? LedState.on : LedState.off);
};

VMS2.playlightflash = function(group) {
    var deck = VMS2.getDeck(group);
    if (deck.switchPlaylightOff) {
        deck.Buttons.Play.setLed(LedState.off);
        deck.switchPlaylightOff = false;
    } else {
        deck.Buttons.Play.setLed(LedState.on);
        deck.switchPlaylightOff = true;
    }
};

VMS2.playlight = function(value, group, control) {
    var deck = VMS2.getDeck(group);

    if(isNaN(deck.playTimer)) {
         deck.playTimer = 0;
    }

    if (value) {
        deck.Buttons.Play.setLed(LedState.on);
        deck.Buttons.Pause.setLed(LedState.off);
        if (deck.playTimer !== 0) {
            engine.stopTimer(deck.playTimer);
            deck.playTimer = 0;
        }
    } else {
        deck.Buttons.Play.setLed(LedState.off);
        deck.Buttons.Pause.setLed(LedState.on);
        // start a fancy blink timer
        deck.switchPlaylightOff = true;
        deck.playTimer = engine.beginTimer(500,"VMS2.playlightflash(\""+group+"\")");
    }
};

VMS2.pitch = function(channel, control, value, status, group) {
    var deck = VMS2.getDeck(group);
    if (!deck.pitchLock) {
        //         engine.setValue(group, "rate", script.pitch(control,value,status));
        // Can't use script.pitch() because the VMS2's sliders are inverted
        value = (value << 7) | control;  // Construct the 14-bit number
        // Range is 0x0000..0x3FFF center @ 0x2000, i.e. 0..16383 center @ 8192
        var rate = (8192-value)/8191;
        engine.setValue(group, "rate", rate);
    }
};

VMS2.pitchCenter = function(channel, control, value, status, group) {
    var deck = VMS2.getDeck(group);
    deck.Buttons.PitchCenter.handleEvent(value);
};

VMS2.pause = function(channel, control, value, status, group) {
    var deck = VMS2.getDeck(group);
    deck.Buttons.Pause.handleEvent(value);
};

VMS2.jog_touch = function(channel, control, value, status, group) {
    var deck = VMS2.getDeck(group);
    deck.Buttons.JogTouch.handleEvent(value);
};

VMS2.jog_move_lsb = function(channel, control, value, status, group) {
    var deck = VMS2.getDeck(group);
    deck.jogMove(value);
};

VMS2.jog_move_msb = function(channel, control, value, status, group) {
    var deck = VMS2.getDeck(group);
    deck.jogMsb = value;
};

VMS2.vinyl = function(channel, control, value, status, group) {
    var deck = VMS2.getDeck(group);
    deck.Buttons.Vinyl.handleEvent(value);
};

VMS2.keylock = function(channel, control, value, status, group) {
    var deck = VMS2.getDeck(group);
    deck.Buttons.KeyLock.handleEvent(value);
};

VMS2.hotCue = function(channel, control, value, status, group) {
    var deck = VMS2.getDeck(group);
    var hotCue = VMS2.hotCues[control];
    if(value === ButtonState.pressed) {
        deck.hotCuePressed = true;
        if (deck.vinylButton || deck.keylockButton) {
            engine.setValue(group,"hotcue_"+hotCue+"_clear",1);
            deck.hotCueDeleted = true;
        } else {
            engine.setValue(group,"hotcue_"+hotCue+"_activate",1);
        }
    } else {
        deck.hotCuePressed = false;
        if (deck.vinylButton || deck.keylockButton) {
            engine.setValue(group,"hotcue_"+hotCue+"_clear",0);
        } else {
            engine.setValue(group,"hotcue_"+hotCue+"_activate",0);
        }
    }
};

VMS2.killHigh = function(channel, control, value, status, group) {
    var deck = VMS2.getDeck(group);
    deck.Buttons.KillHigh.handleEvent(value);
};

VMS2.killMid = function(channel, control, value, status, group) {
    var deck = VMS2.getDeck(group);
    deck.Buttons.KillMid.handleEvent(value);
};

VMS2.killLow = function(channel, control, value, status, group) {
    var deck = VMS2.getDeck(group);
    deck.Buttons.KillLow.handleEvent(value);
};


VMS2.selectSidebar = false;
VMS2.switchSelect = function(channel, control, value, status, group) {
    // toggle between tracklist select and sidebar navigation for rotational knob
    VMS2.selectSidebar = !VMS2.selectSidebar;
};

VMS2.trackSelect = function(channel, control, value, status, group) {
    // This is an endless rotational knob. We save the last status in a static variable
    // so we can identify whether to scroll up or down in track list
    if (!isNaN(VMS2.trackSelect.last_value))
    {
        var direction = value - VMS2.trackSelect.last_value;

        // handle wraparound
        if (direction === 0x7f) {
            direction = -1;
        } else if (direction === (0x00 - 0x7f)) {
            direction = 1;
        }

        if (direction > 0) {
            if (!VMS2.selectSidebar) {
                engine.setValue(group,"SelectTrackKnob",1);
            } else {
                engine.setValue(group,"SelectNextPlaylist", 1);
            }
        } else {
            if (!VMS2.selectSidebar) {
                engine.setValue(group,"SelectTrackKnob",-1);
            } else {
                engine.setValue(group,"SelectPrevPlaylist", 1);
            }
        }
    }
    VMS2.trackSelect.last_value = value;
};
