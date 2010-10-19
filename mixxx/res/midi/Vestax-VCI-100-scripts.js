VestaxVCI100 = new function() {
   this.group = "[Master]";
   this.shiftMode = false;
   this.jogPlaylistScrollMode = false;
   this.Controls = [];
   this.Buttons = [];
};

VestaxVCI100.Deck = function (deckNumber, group) {
   this.deckNumber = deckNumber;
   this.group = group;
   this.vinylMode = true;
   this.scratching = false;
   this.Buttons = [];
};

VestaxVCI100.Deck.prototype.jogMove = function(jogValue) {
   if(VestaxVCI100.jogPlaylistScrollMode) {
      if (jogValue > 0) {
         engine.setValue("[Playlist]","SelectNextTrack", 1);
      } else if (jogValue < 0) {
         engine.setValue("[Playlist]","SelectPrevTrack", 1);
      }
   } else if(this.scratching) {
      engine.scratchTick(this.deckNumber, jogValue);
   } else {
      jogValue = jogValue / 3;
      engine.setValue(this.group,"jog", jogValue);
   }
};

VestaxVCI100.Decks = {"Left":new VestaxVCI100.Deck(1,"[Channel1]"), "Right":new VestaxVCI100.Deck(2,"[Channel2]")};
VestaxVCI100.GroupToDeck = {"[Channel1]":"Left", "[Channel2]":"Right"};

VestaxVCI100.GetDeck = function(group) {
   try {
      return VestaxVCI100.Decks[VestaxVCI100.GroupToDeck[group]];
   } catch(ex) {
      return null;
   }
};

//Mapping functions
VestaxVCI100.vinyl_mode = function (channel, control, value, status, group) {
//   var deck = VestaxVCI100.GetDeck(group);
//   if(value) {
//      if(deck.vinylMode) {
//         deck.vinylMode = false;
//         midi.sendShortMsg(0xB0, control, 0x00);
//      } else {
//         deck.vinylMode = true;
//         midi.sendShortMsg(0xB0, control, 0x7F);
//      }
//   }
};

VestaxVCI100.jog_touch = function (channel, control, value, status, group) {
   var deck = VestaxVCI100.GetDeck(group);
   if(value) {
      if(deck.vinylMode) {
         engine.scratchEnable(deck.deckNumber, 128, 45, 1.0/8, (1.0/8)/32);
         deck.scratching = true;
      }
   } else {
      deck.scratching = false;
      engine.scratchDisable(deck.deckNumber);
   }
};

VestaxVCI100.jog_wheel = function (channel, control, value, status, group) {
  // 41 > 7F: CW Slow > Fast ??? 
  // 3F > 0 : CCW Slow > Fast ???
  var jogValue = value - 0x40; // -64 to +63, - = CCW, + = CW
  VestaxVCI100.GetDeck(group).jogMove(jogValue);
};