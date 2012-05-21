/**
 * Numark NS7 controller script v1.9.0
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
NumarkNS7 = new Controller();
NumarkNS7.RateRanges = [0.08, 0.10, 0.30, 1.00];



NumarkNS7.Deck = Deck;
NumarkNS7.Deck.scratchMode = false;
NumarkNS7.Deck.rateRange = NumarkNS7.RateRanges[1];

NumarkNS7.Deck.prototype.rateRangeHandler = function(value) {
   if(value == ButtonState.pressed) {
      var i;
      for(i=0; i < NumarkNS7.RateRanges.length; i++) {
         if(NumarkNS7.RateRanges[i] == this.rateRange) {
            break;
         }
      }
      
      //Found correct value increment by one
      i++;
      
      //Wrap if needed
      if(i == NumarkNS7.RateRanges.length) {
         i = 0;
      }
      
      this.rateRange = NumarkNS7.RateRanges[i];
   }
}

NumarkNS7.Deck.prototype.scratchHandler = function(value) {
   if(value == ButtonState.pressed) {
      if(this.scratchMode) {
         this.scratchMode = false;
         this.Buttons.Scratch.setLed(LedState.on);
      } else {
         this.scratchMode = true;
         this.Buttons.Scratch.setLed(LedState.on);
      }
   }
}

NumarkNS7.Deck.prototype.jogMove = function(value) {
   if(!isNaN(this.previousJogValue)) {
      var offset = value - this.previousJogValue;

      if(offset > 63) {
         offset = offset - 128;
      } else if(offset < -63) {
         offset = offset + 128;
      }

      if(this.scratchMode) {
         engine.scratchTick(this.deckNumber, offset);
      } else {
         engine.setValue(this.group,"jog", offset);
      }
   }
   this.previousJogValue = value;
}

NumarkNS7.Decks = {"Left":new NumarkNS7.Deck(1,"[Channel1]"), "Right":new NumarkNS7.Deck(2,"[Channel2]")};
NumarkNS7.GroupToDeck = {"[Channel1]":"Left", "[Channel2]":"Right"};

NumarkNS7.GetDeck = function(group) {
   try {
      return NumarkNS7.Decks[NumarkNS7.GroupToDeck[group]];
   } catch(ex) {
      return null;
   }
}

NumarkNS7.Decks.Left.addButton("RateRange", new Button(), "rateRangeHandler");
NumarkNS7.Decks.Left.addButton("Scratch", new Button(), "scratchHandler");

NumarkNS7.Decks.Right.addButton("RateRange", new Button(), "rateRangeHandler");
NumarkNS7.Decks.Right.addButton("Scratch", new Button(), "scratchHandler");


NumarkNS7.rate_range = function(channel, control, value, status, group) {
   var deck = NumarkNS7.GetDeck(group);
   deck.Buttons.RateRange.handleEvent(value);
}

NumarkNS7.scratch = function(channel, control, value, status, group) {
   var deck = NumarkNS7.GetDeck(group);
   deck.Buttons.Scratch.handleEvent(value);
}

NumarkNS7.jog_move = function(channel, control, value, status, group) {
   var deck = NumarkNS7.GetDeck(group);
   deck.jogMove(value);
}
