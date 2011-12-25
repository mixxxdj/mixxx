function IonDiscoverDJ () {}
IonDiscoverDJ.leds = {
"scratch":0x48,
"[Channel1] sync":0x40,
"[Channel1] rev":0x33,
"[Channel1] cue":0x3B,
"[Channel1] play":0x4A,
"[Channel2] sync":0x47,
"[Channel2] rev":0x3C,
"[Channel2] cue":0x42,
"[Channel2] play":0x4C
};

IonDiscoverDJ.debug = true;
IonDiscoverDJ.ledOn = 0x7F;
IonDiscoverDJ.ledOff = 0x00;
IonDiscoverDJ.scratchMode = false;
IonDiscoverDJ.pitchDial1 = false;
IonDiscoverDJ.pitchDial2 = false;

IonDiscoverDJ.init = function (id) {    // called when the MIDI device is opened & set up
    print ("Ion Discover DJ id: \""+id+"\" initialized.");

    var timeToWait = 20;
    for (var LED in IonDiscoverDJ.leds ) {
        IonDiscoverDJ.sendMidi(0x80, IonDiscoverDJ.leds[LED], IonDiscoverDJ.ledOff, timeToWait);
        timeToWait += 5;
    }
    
    for (var LED in IonDiscoverDJ.leds ) {
        IonDiscoverDJ.sendMidi(0x90, IonDiscoverDJ.leds[LED], IonDiscoverDJ.ledOn, timeToWait);
        timeToWait += 5;
    }
    
    timeToWait += 1000;
    for (var LED in IonDiscoverDJ.leds ) {
        IonDiscoverDJ.sendMidi(0x80, IonDiscoverDJ.leds[LED], IonDiscoverDJ.ledOff, timeToWait);
        timeToWait += 5;
    }
};

IonDiscoverDJ.sendMidi = function(status, control, value, timeToWait) {
   if(timeToWait == 0) {
      midi.sendShortMsg(status, control, value);
   } else {
      engine.beginTimer(timeToWait, "midi.sendShortMsg(" + status + ", " + control + ", " + value + ")", true);
   }
};

//Decks
IonDiscoverDJ.Deck = function (deckNumber, group) {
   this.deckNumber = deckNumber;
   this.group = group;
   this.shiftMode = false;
   this.scratching = false;
   this.Buttons = [];
};

IonDiscoverDJ.Deck.prototype.jogMove = function(jogValue) {
   if(this.shiftMode) {
      var newRate = engine.getValue(this.group, "rate") + (jogValue/3000);
      engine.setValue(this.group, "rate", newRate);
   } else if(this.scratching) {
      engine.scratchTick(this.deckNumber, jogValue);
   } else {
      jogValue = jogValue * 10;
      engine.setValue(this.group,"jog", jogValue);
   }
};

IonDiscoverDJ.Decks = {"Left":new IonDiscoverDJ.Deck(1,"[Channel1]"), "Right":new IonDiscoverDJ.Deck(2,"[Channel2]")};
IonDiscoverDJ.GroupToDeck = {"[Channel1]":"Left", "[Channel2]":"Right"};

IonDiscoverDJ.GetDeck = function(group) {
   try {
      return IonDiscoverDJ.Decks[IonDiscoverDJ.GroupToDeck[group]];
   } catch(ex) {
      return null;
   }
};


IonDiscoverDJ.getControl = function (io, channel, name) { 
    // Accept channel in form 'N' or '[ChannelN]'
    channel = channel.replace(/\[Channel(\d)\]/, "$1");

    for (control in IonDiscoverDJ.controls.inputs) {
	if (IonDiscoverDJ.controls.inputs[control].channel == channel && 
	    IonDiscoverDJ.controls.inputs[control].name == name
	    ) return IonDiscoverDJ.controls.inputs[control];
    }

    print ("IonDiscoverDJ.getControl: Control not found: io=" + io + ": channel=" + channel + ": name=" + name);
}

IonDiscoverDJ.shutdown = function() {
}

IonDiscoverDJ.toggle_scratch_mode_on = function (control, value, status) {
    if(IonDiscoverDJ.scratchMode) {
       IonDiscoverDJ.scratchMode = false;
       midi.sendShortMsg(0x80, IonDiscoverDJ.leds["scratch"] , IonDiscoverDJ.ledOff);
    } else {
       IonDiscoverDJ.scratchMode = true;
       midi.sendShortMsg(0x90, IonDiscoverDJ.leds["scratch"] , IonDiscoverDJ.ledOn);
    }
}

IonDiscoverDJ.toggle_scratch_mode_off = function (control, value, status) {
    //IonDiscoverDJ.scratchMode = false;
    //midi.sendShortMsg(0x80, IonDiscoverDJ.leds["scratch"] , IonDiscoverDJ.ledOff);
}

IonDiscoverDJ.play = function (control, value, status) {
   // Only send events when play is pushed, not when it comes back up.
   // group = IonDiscoverDJ.getGroup(control);
   // if (engine.getValue(group, "duration") == 0) { if (value) print("No song on " + group); return; }; 
   //midi.sendShortMsg(0x90, IonDiscoverDJ.leds[IonDiscoverDJ.getGroup(control) + " play"], IonDiscoverDJ.ledOn);
  // midi.sendShortMsg(0x90, IonDiscoverDJ.leds["[Channel1] play"], IonDiscoverDJ.ledOn);
   // engine.setValue(group,"play", !engine.getValue(group,"play"));
   // if (engine.getValue(group, "play") == 0) midi.sendShortMsg(0x80, IonDiscoverDJ.leds[group + " play"], IonDiscoverDJ.ledOFF); 
}   

IonDiscoverDJ.jog_touch = function (channel, control, value, status, group) {
   var deck = IonDiscoverDJ.GetDeck(group);
   if(value) {
      if(IonDiscoverDJ.scratchMode) {
         engine.scratchEnable(deck.deckNumber, 128, 45, 1.0/8, (1.0/8)/32);
         deck.scratching = true;
      }
   } else {
      deck.scratching = false;
      engine.scratchDisable(deck.deckNumber);
   }
};

IonDiscoverDJ.jog_wheel = function (channel, control, value, status, group) {
   // 7F > 40: CCW Slow > Fast - 127 > 64 
   // 01 > 3F: CW Slow > Fast - 0 > 63
   var jogValue = value >=0x40 ? value - 0x80 : value; // -64 to +63, - = CCW, + = CW
   IonDiscoverDJ.GetDeck(group).jogMove(jogValue);
};

IonDiscoverDJ.reversek = function (channel, control, value, status, group) {
   var deck = IonDiscoverDJ.GetDeck(group);
   
   deck.shiftMode = (value == 0x7F); //If button down on, else off
}

IonDiscoverDJ.volCh1Up = function (group, control, value, status) {
engine.setValue("[Channel1]","pregain", engine.getValue("[Channel1]", "pregain") + 0.1);
}
IonDiscoverDJ.volCh1Down = function (group, control, value, status) {
engine.setValue("[Channel1]","pregain", engine.getValue("[Channel1]", "pregain") - 0.1);
}
IonDiscoverDJ.volCh2Up = function (group, control, value, status) {
engine.setValue("[Channel2]","pregain", engine.getValue("[Channel2]", "pregain") + 0.1);
}
IonDiscoverDJ.volCh2Down = function (group, control, value, status) {
engine.setValue("[Channel2]","pregain", engine.getValue("[Channel2]", "pregain") - 0.1);
}

IonDiscoverDJ.pflCh1On = function (group, control, value, status) {
   if(engine.getValue("[Channel1]", "pfl")){
      engine.setValue("[Channel1]","pfl", false);
      midi.sendShortMsg(0x90, IonDiscoverDJ.leds["[Channel1] cue"], IonDiscoverDJ.ledOff);}
   else {
      engine.setValue("[Channel1]","pfl", true);
      midi.sendShortMsg(0x90, IonDiscoverDJ.leds["[Channel1] cue"], IonDiscoverDJ.ledOn);}
   }
IonDiscoverDJ.pflCh1Off = function (group, control, value, status) {}

IonDiscoverDJ.pflCh2On = function (group, control, value, status) {
   if(engine.getValue("[Channel2]", "pfl")){
      engine.setValue("[Channel2]","pfl", false);
      midi.sendShortMsg(0x90, IonDiscoverDJ.leds["[Channel2] cue"], IonDiscoverDJ.ledOff);}
   else {
      engine.setValue("[Channel2]","pfl", true);
      midi.sendShortMsg(0x90, IonDiscoverDJ.leds["[Channel2] cue"], IonDiscoverDJ.ledOn);}
   }
IonDiscoverDJ.pflCh2Off = function (group, control, value, status) {}



IonDiscoverDJ.LoadSelectedTrackCh1 = function (group, control, value, status) 
{
   //midi.sendShortMsg(0x90, IonDiscoverDJ.leds["[Channel1] play"], IonDiscoverDJ.ledOn);
   engine.setValue("[Channel1]","LoadSelectedTrack", true);
   
   //IonDiscoverDJ.pauseScript(5000);
   //engine.setValue("[Channel1]","play", true);
   //midi.sendShortMsg(0x90, IonDiscoverDJ.leds["[Channel1] play"], IonDiscoverDJ.ledOff);
   
   //IonDiscoverDJ.pauseScript(5000);
   //midi.sendShortMsg(0x90, IonDiscoverDJ.leds["[Channel1] sync"], IonDiscoverDJ.ledOn);
   //IonDiscoverDJ.pauseScript(200);
   //engine.setValue("[Channel1]","beatsync", true);
   //midi.sendShortMsg(0x90, IonDiscoverDJ.leds["[Channel1] sync"], IonDiscoverDJ.ledOff);
}

IonDiscoverDJ.LoadSelectedTrackCh2 = function (group, control, value, status) 
{
   //midi.sendShortMsg(0x90, IonDiscoverDJ.leds["[Channel2] play"], IonDiscoverDJ.ledOn);
   engine.setValue("[Channel2]","LoadSelectedTrack", true);
   
   //IonDiscoverDJ.pauseScript(5000);
   //engine.setValue("[Channel2]","play", true);
   //midi.sendShortMsg(0x90, IonDiscoverDJ.leds["[Channel2] play"], IonDiscoverDJ.ledOff);
   
   //IonDiscoverDJ.pauseScript(5000);
   //midi.sendShortMsg(0x90, IonDiscoverDJ.leds["[Channel2] sync"], IonDiscoverDJ.ledOn);
   //IonDiscoverDJ.pauseScript(200);
   //engine.setValue("[Channel2]","beatsync", true);
   //midi.sendShortMsg(0x90, IonDiscoverDJ.leds["[Channel2] sync"], IonDiscoverDJ.ledOff);
}

