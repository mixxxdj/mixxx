/**
 * American Audio VMS4 controller script v2.0 for Mixxx v2.0
 * Copyright (C) 2010  Anders Gunnarsson
 * Copyright (C) 2011-2015  Sean M. Pappalardo
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
                        ["Channel", "sync_enabled"],
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
    engine.softTakeover("[Master]","crossfader",true);
    engine.softTakeover("[Channel1]","rate",true);
    engine.softTakeover("[Channel1]","volume",true);
    engine.softTakeover("[Channel1]","pregain",true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]","parameter3",true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]","parameter2",true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]","parameter1",true);
    engine.softTakeover("[Channel2]","rate",true);
    engine.softTakeover("[Channel2]","volume",true);
    engine.softTakeover("[Channel2]","pregain",true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]","parameter3",true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]","parameter2",true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]","parameter1",true);
    engine.softTakeover("[Channel3]","rate",true);
    engine.softTakeover("[Channel3]","volume",true);
    engine.softTakeover("[Channel3]","pregain",true);
    engine.softTakeover("[EqualizerRack1_[Channel3]_Effect1]","parameter3",true);
    engine.softTakeover("[EqualizerRack1_[Channel3]_Effect1]","parameter2",true);
    engine.softTakeover("[EqualizerRack1_[Channel3]_Effect1]","parameter1",true);
    engine.softTakeover("[Channel4]","rate",true);
    engine.softTakeover("[Channel4]","volume",true);
    engine.softTakeover("[Channel4]","pregain",true);
    engine.softTakeover("[EqualizerRack1_[Channel4]_Effect1]","parameter3",true);
    engine.softTakeover("[EqualizerRack1_[Channel4]_Effect1]","parameter2",true);
    engine.softTakeover("[EqualizerRack1_[Channel4]_Effect1]","parameter1",true);
    engine.softTakeover("[Sampler1]","rate",true);
    engine.softTakeover("[Sampler1]","pregain",true);
    engine.softTakeover("[Sampler2]","rate",true);
    engine.softTakeover("[Sampler2]","pregain",true);
    engine.softTakeover("[EffectRack1_EffectUnit1]","mix",true);
    engine.softTakeover("[EffectRack1_EffectUnit1]","super1",true);
    engine.softTakeover("[EffectRack1_EffectUnit2]","mix",true);
    engine.softTakeover("[EffectRack1_EffectUnit2]","super1",true);
    engine.softTakeover("[EffectRack1_EffectUnit3]","mix",true);
    engine.softTakeover("[EffectRack1_EffectUnit4]","super1",true);
    engine.softTakeover("[EffectRack1_EffectUnit4]","mix",true);
    engine.softTakeover("[EffectRack1_EffectUnit4]","super1",true);
    

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
VMS4.Deck.controlEffectParameter = false;
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

VMS4.Deck.prototype.effectParamButtonHandler = function(value) {
//     if(value == ButtonState.pressed) {
//         this.controlEffectParameter=!this.controlEffectParameter;
//         if (this.controlEffectParameter) {
//             // Super knob
//             this.Buttons.FXParam.setLed(LedState.on);
//         }
//         else {
//             // Wet/dry
//             this.Buttons.FXParam.setLed(LedState.off);
//         }
//     }
}

VMS4.Decks = {"Left":new VMS4.Deck(1,"[Channel1]"), "Right":new VMS4.Deck(2,"[Channel2]")};
VMS4.GroupToDeck = {"[Channel1]":"Left", "[Channel2]":"Right"};
VMS4.GroupToDeckNum = {"[Channel1]":1, "[Channel2]":2, "[Channel3]":3, "[Channel4]":4};

VMS4.GetDeck = function(group) {
   try {
      return VMS4.Decks[VMS4.GroupToDeck[group]];
   } catch(ex) {
      return null;
   }
}

VMS4.GetDeckNum = function(group) {
    try {
        return VMS4.GroupToDeckNum[group];
    } catch(ex) {
        return null;
    }
}

VMS4.Decks.Left.addButton("RateRange", new VMS4.Button(0x11), "rateRangeHandler");
VMS4.Decks.Left.addButton("PitchCenter", new VMS4.Button(), "pitchCenterHandler");
VMS4.Decks.Left.addButton("JogTouch", new VMS4.Button(), "jogTouchHandler");
VMS4.Decks.Left.addButton("Vinyl", new VMS4.Button(), "vinylButtonHandler");
VMS4.Decks.Left.addButton("KeyLock", new VMS4.Button(), "keyLockButtonHandler");
VMS4.Decks.Left.addButton("FXParam", new VMS4.Button(0x1C), "effectParamButtonHandler");

VMS4.Decks.Right.addButton("RateRange", new VMS4.Button(0x33), "rateRangeHandler");
VMS4.Decks.Right.addButton("PitchCenter", new VMS4.Button(), "pitchCenterHandler");
VMS4.Decks.Right.addButton("JogTouch", new VMS4.Button(), "jogTouchHandler");
VMS4.Decks.Right.addButton("Vinyl", new VMS4.Button(), "vinylButtonHandler");
VMS4.Decks.Right.addButton("KeyLock", new VMS4.Button(), "keyLockButtonHandler");
VMS4.Decks.Right.addButton("FXParam", new VMS4.Button(0x3E), "effectParamButtonHandler");

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

VMS4.effectSelect = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    
    if (deck.effectSelect == -1 || isNaN(deck.effectSelect)) deck.effectSelect = value;
    
    // Take wrap around into account
    var wrapCount = 0;
    if (value>=0 && value<10 && deck.effectSelect>117 && deck.effectSelect<=127) wrapCount+=1;
    if (value>117 && value<=127 && deck.effectSelect>=0 && deck.effectSelect<10) wrapCount-=1;
    
    var diff = value - deck.effectSelect;
    deck.effectSelect=value;
    
    diff += wrapCount*128;
    
    engine.setValue("[EffectRack1_EffectUnit"+VMS4.GetDeckNum(group)+"]","chain_selector",diff);
}

VMS4.effectSelectPress = function(channel, control, value, status, group) {
    var deckNum = VMS4.GetDeckNum(group);
    if (value > 0x40) {
        engine.setValue("[EffectRack1_EffectUnit"+deckNum+"]","enabled",
                        !engine.getValue("[EffectRack1_EffectUnit"+deckNum+"]","enabled")
        );
    }
}

VMS4.effectControl = function(channel, control, value, status, group) {
    // If Parameter button is on, this becomes a super knob. Otherwise it controls wet/dry
    var deck = VMS4.GetDeck(group);
    var deckNum = VMS4.GetDeckNum(group);
    if (deck.controlEffectParameter) {
        engine.setValue("[EffectRack1_EffectUnit"+deckNum+"]","super1",
                        script.absoluteLin(value,0,1));
    } else {
        engine.setValue("[EffectRack1_EffectUnit"+deckNum+"]","mix",
                        script.absoluteLin(value,0,1));
    }
}

VMS4.effectParameterButton = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
//     deck.Buttons.FXParam.handleEvent(group, value);
    if(value > 0x40) {
        var deckNum = VMS4.GetDeckNum(group);
        deck.controlEffectParameter=!deck.controlEffectParameter;
        if (deck.controlEffectParameter) {
            // Super knob
            deck.Buttons.FXParam.setLed(LedState.on);
            // Ignore the next wet/dry value
            engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit"+deckNum+"]","mix");
        }
        else {
            // Wet/dry
            deck.Buttons.FXParam.setLed(LedState.off);
            // Ignore the next Super knob value
            engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit"+deckNum+"]","super1");
        }
    }
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

VMS4.touch_strip = function(channel, control, value, status, group) {
   // Only modify the playposition if the deck is NOT playing!
   if (engine.getValue(group, "play") === 0) {
      engine.setValue(group, "playposition", value);
   }
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
