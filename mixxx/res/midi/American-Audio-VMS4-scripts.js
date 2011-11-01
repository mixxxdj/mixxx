/**
 * American Audio VMS4 controller script v1.9.2
 * Copyright (C) 2010  Anders Gunnarsson
 * Copyright (C) 2011  Sean M. Pappalardo
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
VMS4.state = { "effectSelect":-1 }; // Temporary state variables
VMS4.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time

VMS4.init = function (id) {    // called when the MIDI device is opened & set up
    VMS4.id = id;   // Store the ID of this device for later use
    for (i=12; i<=77; i++) midi.sendShortMsg(0x80,i,0x00);  // Extinquish all LEDs
}

VMS4.shutdown = function () {
    for (i=12; i<=77; i++) midi.sendShortMsg(0x80,i,0x00);  // Extinquish all LEDs
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
VMS4.Deck.scratchMode = false;
VMS4.Deck.jogMsb = 0x00;

VMS4.Deck.prototype.rateRangeHandler = function(value) {
    if(value == ButtonState.pressed) {
        this.Buttons.RateRange.setLed(LedState.on);
        // Round to two decimals to avoid double-precision comparison issues
        var currentRange = Math.round(engine.getValue(this.group, "rateRange")*100)/100;
        switch (true) {
            case (currentRange<=VMS4.RateRanges[0]):
                engine.setValue(this.group,"rateRange",VMS4.RateRanges[1]);
                break;
            case (currentRange<=VMS4.RateRanges[1]):
                engine.setValue(this.group,"rateRange",VMS4.RateRanges[2]);
                break;
            case (currentRange<=VMS4.RateRanges[2]):
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
        VMS4.state["pitchLock"+this.group]=true;
        engine.setValue(this.group, "rate", 0);
    }
    else {
        VMS4.state["pitchLock"+this.group]=false;
    }
}

VMS4.Deck.prototype.playHandler = function(value) {
   engine.setValue(this.group, "play", 1);
}

VMS4.Deck.prototype.pauseHandler = function(value) {
   engine.setValue(this.group, "play", 0);
}

VMS4.Deck.prototype.effectSelectHandler = function(value) {
    // Control LFO period
    var increment = 50000;
    // Take wrap around into account
    var wrapCount = 0;
    if (value>=0 && value<10 && VMS4.state["effectSelect"]>117 && VMS4.state["effectSelect"]<=127) wrapCount+=1;
    if (value>117 && value<=127 && VMS4.state["effectSelect"]>=0 && VMS4.state["effectSelect"]<10) wrapCount-=1;
    
    var diff = value - VMS4.state["effectSelect"];
    VMS4.state["effectSelect"]=value;
    
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
      this.scratchMode = true;
   } else {
      engine.scratchDisable(this.deckNumber);
      this.scratchMode = false;
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

      if(this.scratchMode) {
         engine.scratchTick(this.deckNumber, offset);
      } else {
         engine.setValue(this.group,"jog", offset / 40.0);
      }
   }
   this.previousJogValue = jogValue;
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
VMS4.Decks.Left.addButton("JogTouch", new VMS4.Button(), "jogTouchHandler");
VMS4.Decks.Left.addButton("EffectSelect", new VMS4.Button(), "effectSelectHandler");
VMS4.Decks.Left.addButton("EffectSelectPress", new VMS4.Button(), "effectSelectPressHandler");

VMS4.Decks.Right.addButton("RateRange", new VMS4.Button(0x33), "rateRangeHandler");
VMS4.Decks.Right.addButton("PitchCenter", new VMS4.Button(), "pitchCenterHandler");
VMS4.Decks.Right.addButton("Play", new VMS4.Button(), "playHandler");
VMS4.Decks.Right.addButton("Pause", new VMS4.Button(), "pauseHandler");
VMS4.Decks.Right.addButton("JogTouch", new VMS4.Button(), "jogTouchHandler");
VMS4.Decks.Right.addButton("EffectSelect", new VMS4.Button(), "effectSelectHandler");
VMS4.Decks.Right.addButton("EffectSelectPress", new VMS4.Button(), "effectSelectPressHandler");

//Mapping functions
VMS4.rate_range = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    deck.Buttons.RateRange.handleEvent(value);
}

VMS4.pitch = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    if (!VMS4.state["pitchLock"+group]) {
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

VMS4.effectSelect = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    deck.Buttons.EffectSelect.handleEvent(value);
}

VMS4.effectSelectPress = function(channel, control, value, status, group) {
    var deck = VMS4.GetDeck(group);
    deck.Buttons.EffectSelectPress.handleEvent(value);
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