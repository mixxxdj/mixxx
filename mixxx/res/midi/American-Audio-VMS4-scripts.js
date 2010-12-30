/**
 * American Audio VMS4 controller script v1.9.0
 * Copyright (C) 2010  Anders Gunnarsson
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
VMS4.Deck.rateRange = VMS4.RateRanges[1];

VMS4.Deck.prototype.rateRangeHandler = function(value) {
   if(value == ButtonState.pressed) {
      var i;
      for(i=0; i < VMS4.RateRanges.length; i++) {
         if(VMS4.RateRanges[i] == this.rateRange) {
            break;
         }
      }
      
      //Found correct value increment by one
      i++;
      
      //Wrap if needed
      if(i == VMS4.RateRanges.length) {
         i = 0;
      }
      
      this.rateRange = VMS4.RateRanges[i];
   }
}

VMS4.Deck.prototype.playHandler = function(value) {
   engine.setValue(this.group, "play", 1);
}

VMS4.Deck.prototype.pauseHandler = function(value) {
   engine.setValue(this.group, "play", 0);
}

VMS4.Deck.prototype.scratchHandler = function(value) {
   if(value == ButtonState.pressed) {
      if(this.scratchMode) {
         this.scratchMode = false;
         this.Buttons.Scratch.setLed(LedState.off);
      } else {
         this.scratchMode = true;
         this.Buttons.Scratch.setLed(LedState.on);
      }
   }
   print("scratchMode: " + this.scratchMode);
}

VMS4.Deck.prototype.jogTouchHandler = function(value) {
   if(value == ButtonState.pressed && this.scratchMode) {
      engine.scratchEnable(this.deckNumber, 3000, 45, 1.0/8, (1.0/8)/32);
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

VMS4.Decks.Left.addButton("RateRange", new VMS4.Button(), "rateRangeHandler");
VMS4.Decks.Left.addButton("Play", new VMS4.Button(), "playHandler");
VMS4.Decks.Left.addButton("Pause", new VMS4.Button(), "pauseHandler");
VMS4.Decks.Left.addButton("Scratch", new VMS4.Button(0x27), "scratchHandler");
VMS4.Decks.Left.addButton("JogTouch", new VMS4.Button(), "jogTouchHandler");

VMS4.Decks.Right.addButton("RateRange", new VMS4.Button(), "rateRangeHandler");
VMS4.Decks.Right.addButton("Play", new VMS4.Button(), "playHandler");
VMS4.Decks.Right.addButton("Pause", new VMS4.Button(), "pauseHandler");
VMS4.Decks.Right.addButton("Scratch", new VMS4.Button(0x49), "scratchHandler");
VMS4.Decks.Right.addButton("JogTouch", new VMS4.Button(), "jogTouchHandler");

//Mapping functions
VMS4.rate_range = function(channel, control, value, status, group) {
   var deck = VMS4.GetDeck(group);
   deck.Buttons.RateRange.handleEvent(value);
}

VMS4.play = function(channel, control, value, status, group) {
   var deck = VMS4.GetDeck(group);
   deck.Buttons.Play.handleEvent(value);
}

VMS4.pause = function(channel, control, value, status, group) {
   var deck = VMS4.GetDeck(group);
   deck.Buttons.Pause.handleEvent(value);
}

VMS4.scratch = function(channel, control, value, status, group) {
   var deck = VMS4.GetDeck(group);
   deck.Buttons.Scratch.handleEvent(value);
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