VestaxVCI100 = new function() {
   this.group = "[Master]";
   this.loopHotcueDeck = null;
   this.shiftMode = false;
   this.jogPlaylistScrollMode = false;
   this.Controls = [];
   this.Buttons = [];
}

VestaxVCI100.ButtonState = {"released":0x00, "pressed":0x7F};
VestaxVCI100.Button = function (controlId) {
   this.controlId = controlId;
   this.state = VestaxVCI100.ButtonState.released;
};
VestaxVCI100.Button.prototype.handleEvent = function(value) {
   this.handler(value);
};

VestaxVCI100.addButton = function(buttonName, button, eventHandler) {
   if(eventHandler) {
      var executionEnvironment = this;
      function handler(value) {
         button.state = value;
         executionEnvironment[eventHandler](value);
      }
      button.handler = handler;
   }
   this.Buttons[buttonName] = button; 
};

VestaxVCI100.key1Handler = function(value) {
   var deck = this.loopHotcueDeck;
   if(value && deck != null) {
      if(deck.loop) {
         engine.setValue(deck.group,"loop_in",1);
      } else if(deck.hotcue) {
         if(deck.Buttons.Hotcue.state == VestaxVCI100.ButtonState.pressed) {
            engine.setValue(deck.group,"hotcue_1_set", 1);
         } else {
            engine.setValue(deck.group,"hotcue_1_goto", 1);
         }
      }
   }
};

VestaxVCI100.key2Handler = function(value) {
   var deck = this.loopHotcueDeck;
   if(value && deck != null) {
      if(deck.loop) {
         engine.setValue(deck.group,"loop_out",1);
      } else if(deck.hotcue) {
         if(deck.Buttons.Hotcue.state == VestaxVCI100.ButtonState.pressed) {
            engine.setValue(deck.group,"hotcue_2_set", 1);
         } else {
            engine.setValue(deck.group,"hotcue_2_goto", 1);
         }
      }
   }
};

VestaxVCI100.key3Handler = function(value) {
   var deck = this.loopHotcueDeck;
   if(value && deck != null) {
      if(deck.loop) {
         engine.setValue(deck.group,"reloop_exit",1);
      } else if(deck.hotcue) {
         if(deck.Buttons.Hotcue.state == VestaxVCI100.ButtonState.pressed) {
            engine.setValue(deck.group,"hotcue_3_set", 1);
         } else {
            engine.setValue(deck.group,"hotcue_3_goto", 1);
         }
      }
   }
};

VestaxVCI100.addButton("Key1", new VestaxVCI100.Button(0x62), "key1Handler");
VestaxVCI100.addButton("Key2", new VestaxVCI100.Button(0x63), "key2Handler");
VestaxVCI100.addButton("Key3", new VestaxVCI100.Button(0x64), "key3Handler");


VestaxVCI100.Deck = function (deckNumber, group) {
   this.deckNumber = deckNumber;
   this.group = group;
   this.vinylMode = true;
   this.scratching = false;
   this.loop = false;
   this.hotcue = false;
   this.Buttons = [];
}

VestaxVCI100.Deck.prototype.addButton = VestaxVCI100.addButton;

VestaxVCI100.Deck.prototype.jogMove = function(jogValue) {
   jogValue = jogValue * 3;
   engine.setValue(this.group,"jog", jogValue);
}

VestaxVCI100.Deck.prototype.scratchMove = function(jogValue) {
    engine.scratchTick(this.deckNumber, jogValue);
}

VestaxVCI100.Deck.prototype.loopHandler = function(value) {
   if(value) {
      VestaxVCI100.loopHotcueDeck = this;
      this.loop = true;
      this.hotcue = false;
   }
};

VestaxVCI100.Deck.prototype.hotcueHandler = function(value) {
   if(value) {
      VestaxVCI100.loopHotcueDeck = this;
      this.hotcue = true;
      this.loop = false;
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
}

VestaxVCI100.Decks.Left.addButton("Loop", new VestaxVCI100.Button(0x65), "loopHandler");
VestaxVCI100.Decks.Left.addButton("Hotcue", new VestaxVCI100.Button(0x66), "hotcueHandler");
VestaxVCI100.Decks.Right.addButton("Loop", new VestaxVCI100.Button(0x67), "loopHandler");
VestaxVCI100.Decks.Right.addButton("Hotcue", new VestaxVCI100.Button(0x68), "hotcueHandler");

VestaxVCI100.init = function (id) {
   midi.sendShortMsg(0xB0,0x62,0x7F);
   midi.sendShortMsg(0xB0,0x63,0x7F);
   midi.sendShortMsg(0xB0,0x64,0x7F);
   
   midi.sendShortMsg(0x90,0x65,0x7F);
   midi.sendShortMsg(0x90,0x66,0x7F);
   midi.sendShortMsg(0x90,0x67,0x7F);
   midi.sendShortMsg(0x90,0x68,0x7F);
   
   midi.sendShortMsg(0xB0,0x64,0x00);
   midi.sendShortMsg(0x90,0x67,0x00);
   midi.sendShortMsg(0x80,0x68,0x00);
}


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
}

VestaxVCI100.jog_touch = function (channel, control, value, status, group) {
   var deck = VestaxVCI100.GetDeck(group);
   if(value) {
      engine.scratchEnable(deck.deckNumber, 128*3, 45, 1.0/8, (1.0/8)/32);
   } else {
      engine.scratchDisable(deck.deckNumber);
   }
}

VestaxVCI100.jog_wheel = function (channel, control, value, status, group) {
   // 41 > 7F: CW Slow > Fast ??? 
   // 3F > 0 : CCW Slow > Fast ???
   var jogValue = value - 0x40; // -64 to +63, - = CCW, + = CW
   VestaxVCI100.GetDeck(group).jogMove(jogValue);
}

VestaxVCI100.jog_wheel_scratch = function (channel, control, value, status, group) {
   // 41 > 7F: CW Slow > Fast ??? 
   // 3F > 0 : CCW Slow > Fast ???
   var jogValue = value - 0x40; // -64 to +63, - = CCW, + = CW
   VestaxVCI100.GetDeck(group).scratchMove(jogValue);
}

VestaxVCI100.loopMode = function (channel, control, value, status, group) {
   var deck = VestaxVCI100.GetDeck(group);
   deck.Buttons.Loop.handleEvent(value);
}

VestaxVCI100.hotcueMode = function (channel, control, value, status, group) {
   var deck = VestaxVCI100.GetDeck(group);
   deck.Buttons.Hotcue.handleEvent(value);
}

VestaxVCI100.key1 = function (channel, control, value, status, group) {
   VestaxVCI100.Buttons.Key1.handleEvent(value);
}

VestaxVCI100.key2 = function (channel, control, value, status, group) {
   VestaxVCI100.Buttons.Key2.handleEvent(value);
}

VestaxVCI100.key3 = function (channel, control, value, status, group) {
   VestaxVCI100.Buttons.Key3.handleEvent(value);
}
