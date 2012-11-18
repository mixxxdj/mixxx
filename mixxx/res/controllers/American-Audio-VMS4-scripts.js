/**
 * American Audio VMS4 controller script v1.10.0
 * Copyright (C) 2010  Anders Gunnarsson
 * Copyright (C) 2011-2012  Sean M. Pappalardo
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
VMS4 = new Controller();

VMS4.RateRanges = [0.08, 0.10, 0.30, 1.00];
//----
VMS4.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time

VMS4.numHotCues = 8;
// Button control number to hot cue mapping
VMS4.hotCues = { 0x12:1, 0x13:2, 0x14:3, 0x15:4,
                 0x17:5, 0x18:6, 0x19:7, 0x1A:8,
                 0x34:1, 0x35:2, 0x36:3, 0x37:4,
                 0x39:5, 0x3A:6, 0x3B:7, 0x3C:8 };

VMS4.initControls = [   ["Channel", "hotcue_x_enabled"],
                        ["Channel", "quantize"],
                        ["Channel", "beatsync"],
                        ["Channel", "flanger"],
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
                        ["Sampler", "play"]
                    ];

VMS4.init = function (id) {    // called when the MIDI device is opened & set up
    VMS4.id = id;   // Store the ID of this device for later use
    for (i=12; i<=77; i++) midi.sendShortMsg(0x80,i,0x00);  // Extinquish all LEDs

    // Enable soft-takeover for all direct hardware controls
    //  (Many of these are mapped in the XML directly so these have no effect.
    //  Here for completeness incase any eventually move to script.)
    engine.softTakeover("[Channel1]","rate",true);
    engine.softTakeover("[Channel1]","volume",true);
    engine.softTakeover("[Channel1]","pregain",true);
    engine.softTakeover("[Channel1]","filterHigh",true);
    engine.softTakeover("[Channel1]","filterMed",true);
    engine.softTakeover("[Channel1]","filterLow",true);
    engine.softTakeover("[Master]","crossfader",true);
    engine.softTakeover("[Channel2]","rate",true);
    engine.softTakeover("[Channel2]","volume",true);
    engine.softTakeover("[Channel2]","pregain",true);
    engine.softTakeover("[Channel2]","filterHigh",true);
    engine.softTakeover("[Channel2]","filterMed",true);
    engine.softTakeover("[Channel2]","filterLow",true);
    engine.softTakeover("[Sampler1]","rate",true);
    engine.softTakeover("[Sampler1]","pregain",true);
    engine.softTakeover("[Sampler2]","rate",true);
    engine.softTakeover("[Sampler2]","pregain",true);

    print("American Audio "+VMS4.id+" initialized.");
}

VMS4.shutdown = function () {
    for (i=12; i<=77; i++) midi.sendShortMsg(0x80,i,0x00);  // Extinquish all LEDs
    print("American Audio "+VMS4.id+" shut down.");
}

VMS4.Button = Button;

VMS4.Button.prototype.setLed = function(ledState) {
   if(ledState == LedState.on) {
      midi.sendShortMsg(0x90,this.controlId,LedState.on);
   } else {
      midi.sendShortMsg(0x80,this.controlId,LedState.off);
   }
}

VMS4.Deck = Deck;
VMS4.Deck.jogMsb = 0x00;
VMS4.Deck.scratchncue = false;
VMS4.Deck.hotCueDeleted = false;
VMS4.Deck.keylockButton = false;
VMS4.Deck.vinylButton = false;
VMS4.Deck.cueButton = false;
VMS4.Deck.hotCuePressed = false;
VMS4.Deck.pitchLock = false;
VMS4.Deck.effectSelect = -1;
VMS4.Deck.sampleSelect = -1;

VMS4.Deck.prototype.rateRangeHandler = function(value) {
    if(value == ButtonState.pressed) {
        this.Buttons.RateRange.setLed(LedState.on);
        // Round to two decimals to avoid double-precision comparison issues
        var currentRange = Math.round(engine.getValue(this.group, "rateRange")*100)/100;
        switch (true) {
            case (currentRange<VMS4.RateRanges[0]):
                engine.setValue(this.group,"rateRange",VMS4.RateRanges[0]);
                break;
            case (currentRange<VMS4.RateRanges[1]):
                engine.setValue(this.group,"rateRange",VMS4.RateRanges[1]);
                break;
            case (currentRange<VMS4.RateRanges[2]):
                engine.setValue(this.group,"rateRange",VMS4.RateRanges[2]);
                break;
            case (currentRange<VMS4.RateRanges[3]):
                engine.setValue(this.group,"rateRange",VMS4.RateRanges[3]);
                break;
            case (currentRange>=VMS4.RateRanges[3]):
                engine.setValue(this.group,"rateRange",VMS4.RateRanges[0]);
                break;
        }
        // Update the screen display
        engine.trigger(this.group,"rate");
    }
    else this.Buttons.RateRange.setLed(LedState.off);
}

VMS4.Deck.prototype.pitchCenterHandler = function(value) {
    // Reset pitch only on entrance to center position
    if(value == ButtonState.pressed) {
        this.pitchLock=true;
        engine.setValue(this.group, "rate", 0);
    }
    else {
        this.pitchLock=false;
    }
}

VMS4.Deck.prototype.playHandler = function(value) {
    if(value == ButtonState.pressed) {
        var currentlyPlaying = engine.getValue(this.group,"play");
        // Only do stutter play when currently playing and not previewing
        if (currentlyPlaying && !this.cueButton && !this.hotCuePressed) {
            engine.setValue(this.group, "start_play", 1);
        }
        else engine.setValue(this.group,"play", 1);
    }
    else engine.setValue(this.group, "start_play", 0);
}

VMS4.Deck.prototype.pauseHandler = function(value) {
   engine.setValue(this.group, "play", 0);
}

VMS4.Deck.prototype.cueHandler = function(value) {
    if(value == ButtonState.pressed) {
        this.cueButton=true;
        if (this.vinylButton) {
            // Toggle scratch & cue mode
            if (this.scratchncue) this.scratchncue=false;
            else this.scratchncue=true;
        }
        else engine.setValue(this.group,"cue_default",1);
    }
    else {
        engine.setValue(this.group, "cue_default", 0);
        this.cueButton=false;
    }
}

VMS4.Deck.prototype.effectSelectHandler = function(value) {
    if (this.effectSelect == -1) this.effectSelect = value;
    
    // Control LFO period
    var increment = 50000;
    // Take wrap around into account
    var wrapCount = 0;
    if (value>=0 && value<10 && this.effectSelect>117 && this.effectSelect<=127) wrapCount+=1;
    if (value>117 && value<=127 && this.effectSelect>=0 && this.effectSelect<10) wrapCount-=1;
    
    var diff = value - this.effectSelect;
    this.effectSelect=value;
    
    diff += wrapCount*128;

    var newValue = engine.getValue("[Flanger]","lfoPeriod")+(diff*increment);
    if (newValue > 2000000) newValue = 2000000;
    if (newValue < 50000) newValue = 50000;

    engine.setValue("[Flanger]","lfoPeriod",newValue);
}

VMS4.Deck.prototype.effectSelectPressHandler = function(value) {
    // Reset the effect only on press
    if(value == ButtonState.pressed) engine.setValue("[Flanger]","lfoPeriod",1025000)
}

VMS4.Deck.prototype.jogTouchHandler = function(value) {
   if(value == ButtonState.pressed) {
      engine.scratchEnable(this.deckNumber, 3000, 45, 1.0/8, (1.0/8)/32);
      // Recall the cue point if in "scratch & cue" mode only when playing
      if (this.scratchncue && engine.getValue(this.group,"play")==1) {
          engine.setValue(this.group,"cue_goto",1);
          engine.setValue(this.group,"cue_goto",0);
      }
   } else {
      engine.scratchDisable(this.deckNumber);
   }
}

VMS4.Deck.prototype.jogMove = function(lsbValue) {
   var jogValue = (this.jogMsb << 7) + lsbValue;

   if(!isNaN(this.previousJogValue)) {
      var offset = jogValue - this.previousJogValue;

      if(offset > 8192) {
         offset = offset - 16384;
      } else if(offset < -8192) {
         offset = offset + 16384;
      }

      if(engine.isScratching(this.deckNumber)) {
         engine.scratchTick(this.deckNumber, offset);
      } else {
         engine.setValue(this.group,"jog", offset / 40.0);
      }
   }
   this.previousJogValue = jogValue;
}

VMS4.Deck.prototype.vinylButtonHandler = function(value) {
    if(value == ButtonState.pressed) this.vinylButton=true;
    else {
        this.vinylButton=false;
        // Force keylock up too since they're they same physical button
        //  (This prevents keylock getting stuck down if shift is released first)
        this.keylockButton=false;
    }
}

VMS4.Deck.prototype.keyLockButtonHandler = function(value) {
    if(value == ButtonState.pressed) {
        this.keylockButton=true;
        this.hotCueDeleted=false;
    }
    else {
        // Toggle keylock only on release and only if a hot cue wasn't deleted
        if (!this.hotCueDeleted) {
            var currentKeylock = engine.getValue(this.group, "keylock");
            engine.setValue(this.group, "keylock", !currentKeylock);
        }
        this.keylockButton=false;
        // Force vinyl up too since they're they same physical button
        //  (This prevents vinyl getting stuck down if shift is pressed while
        //  vinyl is held down)
        this.vinylButton=false;
    }
}

VMS4.Decks = {"Left":new VMS4.Deck(1,"[Channel1]"), "Right":new VMS4.Deck(2,"[Channel2]")};
VMS4.GroupToDeck = {"[Channel1]":"Left", "[Channel2]":"Right"};

VMS4.GetDeck = function(group) {
   try {
      return VMS4.Decks[VMS4.GroupToDeck[group]];
   } catch(ex) {
      return null;
   }
}

VMS4.Decks.Left.addButton("RateRange", new VMS4.Button(0x11), "rateRangeHandler");
VMS4.Decks.Left.addButton("PitchCenter", new VMS4.Button(), "pitchCenterHandler");
VMS4.Decks.Left.addButton("Play", new VMS4.Button(), "playHandler");
VMS4.Decks.Left.addButton("Pause", new VMS4.Button(), "pauseHandler");
VMS4.Decks.Left.addButton("Cue", new VMS4.Button(), "cueHandler");
VMS4.Decks.Left.addButton("JogTouch", new VMS4.Button(), "jogTouchHandler");
VMS4.Decks.Left.addButton("EffectSelect", new VMS4.Button(), "effectSelectHandler");
VMS4.Decks.Left.addButton("EffectSelectPress", new VMS4.Button(), "effectSelectPressHandler");
VMS4.Decks.Left.addButton("Vinyl", new VMS4.Button(), "vinylButtonHandler");
VMS4.Decks.Left.addButton("KeyLock", new VMS4.Button(), "keyLockButtonHandler");

VMS4.Decks.Right.addButton("RateRange", new VMS4.Button(0x33), "rateRangeHandler");
VMS4.Decks.Right.addButton("PitchCenter", new VMS4.Button(), "pitchCenterHandler");
VMS4.Decks.Right.addButton("Play", new VMS4.Button(), "playHandler");
VMS4.Decks.Right.addButton("Pause", new VMS4.Button(), "pauseHandler");
VMS4.Decks.Right.addButton("Cue", new VMS4.Button(), "cueHandler");
VMS4.Decks.Right.addButton("JogTouch", new VMS4.Button(), "jogTouchHandler");
VMS4.Decks.Right.addButton("EffectSelect", new VMS4.Button(), "effectSelectHandler");
VMS4.Decks.Right.addButton("EffectSelectPress", new VMS4.Button(), "effectSelectPressHandler");
VMS4.Decks.Right.addButton("Vinyl", new VMS4.Button(), "vinylButtonHandler");
VMS4.Decks.Right.addButton("KeyLock", new VMS4.Button(), "keyLockButtonHandler");

//Mapping functions
VMS4.rate_range = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    deck.Buttons.RateRange.handleEvent(value);
}

VMS4.pitch = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    if (!deck.pitchLock) {
//         engine.setValue(group, "rate", script.pitch(control,value,status));
        // Can't use script.pitch() because the VMS4's sliders are inverted
        var value = (value << 7) | control;  // Construct the 14-bit number
        // Range is 0x0000..0x3FFF center @ 0x2000, i.e. 0..16383 center @ 8192
        var rate = (8192-value)/8191;
        engine.setValue(group, "rate", rate);
    }
}

VMS4.pitchCenter = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    deck.Buttons.PitchCenter.handleEvent(value);
}

VMS4.play = function(channel, control, value, status, group) {
   var deck = VMS4.GetDeck(group);
   deck.Buttons.Play.handleEvent(value);
}

VMS4.pause = function(channel, control, value, status, group) {
   var deck = VMS4.GetDeck(group);
   deck.Buttons.Pause.handleEvent(value);
}

VMS4.cue = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    deck.Buttons.Cue.handleEvent(value);
}

VMS4.effectSelect = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    deck.Buttons.EffectSelect.handleEvent(value);
}

VMS4.effectSelectPress = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    deck.Buttons.EffectSelectPress.handleEvent(value);
}

VMS4.sampleSelect = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    if (deck.sampleSelect == -1) deck.sampleSelect = value;
    
    // Take wrap around into account
    var wrapCount = 0;
    if (value>=0 && value<10 && deck.sampleSelect>117 && deck.sampleSelect<=127) wrapCount+=1;
    if (value>117 && value<=127 && deck.sampleSelect>=0 && deck.sampleSelect<10) wrapCount-=1;
    
    var diff = value - deck.sampleSelect;
    deck.sampleSelect=value;
    
    diff += wrapCount*128;
    
    engine.setValue("[Playlist]","SelectTrackKnob",diff);
}

VMS4.jog_touch = function(channel, control, value, status, group) {
   var deck = VMS4.GetDeck(group);
   deck.Buttons.JogTouch.handleEvent(value);
}

VMS4.jog_move_lsb = function(channel, control, value, status, group) {
   var deck = VMS4.GetDeck(group);
   deck.jogMove(value);
}

VMS4.jog_move_msb = function(channel, control, value, status, group) {
   var deck = VMS4.GetDeck(group);
   deck.jogMsb = value;
}

VMS4.vinyl = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    deck.Buttons.Vinyl.handleEvent(value);
}

VMS4.keylock = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    deck.Buttons.KeyLock.handleEvent(value);
}

VMS4.hotCue = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    var hotCue = VMS4.hotCues[control];
    if(value == ButtonState.pressed) {
        deck.hotCuePressed=true;
        if (deck.vinylButton || deck.keylockButton) {
//             print("vinyl="+deck.vinylButton+" keylock="+deck.keylockButton+" so clear hotcue"+hotCue);
            engine.setValue(group,"hotcue_"+hotCue+"_clear",1);
            deck.hotCueDeleted=true;
        }
        else {
            engine.setValue(group,"hotcue_"+hotCue+"_activate",1);
        }
    }
    else {
        deck.hotCuePressed=false;
        if (deck.vinylButton || deck.keylockButton) {
            engine.setValue(group,"hotcue_"+hotCue+"_clear",0);
        }
        else {
            engine.setValue(group,"hotcue_"+hotCue+"_activate",0);
        }
    }
}