var NumarkN4 = {};

NumarkN4.scratchSettings = {
    'alpha': 1.0 / 8,
    'beta': 1.0 / 8 / 32,
    'jogResolution': 600,
    'vinylSpeed': 33 + 1 / 3,
};

NumarkN4.searchAmplification = 5; // multiplier for the jogwheel when the search button is held down.

NumarkN4.warnAfterPosition = 0.8; // percentage after when the triangular LEDs should warn that the track ends.

NumarkN4.blinkInterval=1000; //blinkInterval for the triangular Leds over the channels in milliseconds.

NumarkN4.rateRanges = [0.06,// one semitone
                       0.24,// for maximum freedom
                     ];
NumarkN4.QueryStatusMessage=[0xF0,0x00,0x01,0x3F,0x7F,0x47,0x60,0x00,0x01,0x54,0x01,0x00,0x00,0x00,0x00,0xF7];
NumarkN4.ShutoffSequesnce=[0xF0,0x00,0x01,0x3F,0x7F,0x47,0xB0,0x39,0x00,0x01,0xF7];

NumarkN4.highResGainMSB={
  "[Channel1]":0,
  "[Channel2]":0,
  "[Channel3]":0,
  "[Channel4]":0,
};
NumarkN4.vinylTouched = [false,false,false,false,];

NumarkN4.init = function (id) {

  components.Component.prototype.send = function (value) {
    if (this.midi === undefined || this.midi[0] === undefined || this.midi[1] === undefined) {
        return;
    }
    if (this.midi[2]===undefined){//check if output channel/type not explicitly defined
      this.midi[2]=this.midi[0]
    }
    if (this.midi[3]===undefined){//check if output control not explicitly defined
      this.midi[3]=this.midi[1]
    }
    midi.sendShortMsg(this.midi[2], this.midi[3], value);
    if (this.sendShifted) {
        if (this.shiftChannel) {
            midi.sendShortMsg(this.midi[2] + this.shiftOffset, this.midi[3], value);
        } else if (this.shiftControl) {
            midi.sendShortMsg(this.midi[2], this.midi[3] + this.shiftOffset, value);
        }
    }
  };

  NumarkN4.Deck1 = new NumarkN4.Deck(1);
  NumarkN4.Deck2 = new NumarkN4.Deck(2);
  NumarkN4.Deck3 = new NumarkN4.Deck(3);
  NumarkN4.Deck4 = new NumarkN4.Deck(4);

  NumarkN4.Mixer = new NumarkN4.MixerTemplate();

  //midi.sendSysexMsg(NumarkN4.QueryStatusMessage,NumarkN4.QueryStatusMessage.length)
  //query controller for component status

};
NumarkN4.topContainer = function (channel) {
  this.hotcues = [];
  for (var i = 1; i <= 8; i++) {
    this.hotcues[i] = new components.HotcueButton({
        midi: [0x90+channel, 0x7F, 0xB0+channel, 0x7F],
        //the output (0x7F=placeholder) gets set while running by the constuctor and the hotcue page switcher pot.
        number: i,
        group: '[Channel'+channel+']',
    });
  }
  this.button5 = new components.Button({
    midi: [0x90+channel,0x27,0x18],
    group: '[Channel'+channel+']',
    key: "loop_in",
    shift: function () {},
  })
};
NumarkN4.topContainer.prototype = new components.ComponentContainer();

NumarkN4.MixerTemplate = function () {
  //channel will always be 0 it can be "hardcoded" into the components
  this.deckChange = function (channel, control, value, status, group) {
    midi.sendShortMsg(status,control,value);
    //just "echos" the midi since the controller knows the deck its on itself but doesnt update the corresponing leds.
  };
  this.channelInputSwitcher = function (channel, control, value, status, group) { // can't be done in pure XML
    engine.setParameter(group,"mute",value); // TODO: Test
  };

  // NOTE: [Mixer Profile] is not a documented group. Taken from script.crossfaderCurve
  // BUG: Help on Mixxx forum is needed
  this.changeCrossfaderContour = function (channel, control, value, status, group) {

    if (value===0x00){
      engine.setValue("[Mixer Profile]","xFaderMode",1);
      engine.setValue("[Mixer Profile]","xFaderCalibration",0.5);
    } else {
      engine.setValue("[Mixer Profile]","xFaderMode",0);
      engine.setValue("[Mixer Profile]","xFaderCurve",2);
    }
  };
  this.navigationEncoderTick = new components.Encoder({
    midi: [0xB0, 0x44],
    group: "[Library]",
    inKey: "MoveVertical",
    input: function (midiChannel,control,value,status,group) {
      this.inSetValue(value===0x01?1:-1); // value rescaling; possibly a bit inefficent.
    },
    shift: function () {this.inKey="MoveFocus";},
    unshift: function () {this.inKey="MoveVertical";},
  });
  this.navigationEncoderButton = new components.Button({
    midi:[0x90,0x08],
    group: "[Library]",
    key: "ChooseItem",
  })

};

NumarkN4.Deck = function (channel) {
  components.Deck.call(this, channel);
  this.group = '[Channel' + channel + ']';
  this.rateRangeEntry=0;
  this.lastOrientation=(channel%2?0:2);
  this.isSearching=false;
  this.blinkTimer=engine.beginTimer(NumarkN4.blinkInterval,"NumarkN4.Deck"+channel+".manageChannelIndicator");
  //timer is more efficent is this case than a callback because it would be called to often.
  engine.makeConnection(this.group,"track_loaded",function (value) {midi.sendShortMsg(0xB0,0x1D+channel,value?0x7F:0x00)});
  var theDeck = this;
  this.shiftButton = function (channelmidi, control, value, status, group) {
    midi.sendShortMsg(0xB0+channel,0x15,value);
    if (value>0) {
      theDeck.shift();
    } else {
      theDeck.unshift();
    }
  };
  this.topContainer = new NumarkN4.topContainer(channel);
  // NOTE: THE ORIENATION BUTTONS BEHAVE REALLY WIERD AND THE FOLLOWING IS REALLY CONFUSING BUT WORKS!
  this.orientationButtonLeft = new components.Button({
    midi:[0x90,0x32+channel*2,0xB0,0x42+channel*2],
    key: "orientation",
    input: function (channel, control, value, status, group) {
      print("Script.midiDebug - channel: 0x" + channel.toString(16) +
            " control: 0x" + control.toString(16) + " value: 0x" + value.toString(16) +
            " status: 0x" + status.toString(16) + " group: " + group);
      print(this.ignoreNext);
      if (!this.ignoreNext) {
        if (value===0x7F) {
          this.inSetValue(0);
          theDeck.orientationButtonRight.ignoreNextOff = true;
          this.ignoreNextOff=false;
        } else if (!this.ignoreNextOff && value===0x00) {
          this.inSetValue(1);
        }
      } else {this.ignoreNext=false;}
    },
    output: function (value, group, control) {
      this.send(value===0?0x7F:0x00);
      this.ignoreNext=true;
      if (value===0){theDeck.orientationButtonRight.ignoreNextOff = true;}
      print("called left callback :"+value);
    },
  });
  this.orientationButtonRight = new components.Button({
    midi:[0x90,0x33+channel*2,0xB0,0x43+channel*2],
    key: "orientation",
    input: function (channel, control, value, status, group) {
      print("Script.midiDebug - channel: 0x" + channel.toString(16) +
            " control: 0x" + control.toString(16) + " value: 0x" + value.toString(16) +
            " status: 0x" + status.toString(16) + " group: " + group);
      print(this.ignoreNext);
      if (!this.ignoreNext) {
        if (value===0x7F) {
          this.inSetValue(2);
          theDeck.orientationButtonLeft.ignoreNextOff = true;
          this.ignoreNextOff=false;
        } else if (!this.ignoreNextOff && value===0x00) {
          this.inSetValue(1);
        }
      } else {this.ignoreNext=false;}
    },
    output: function (value, group, control) {
      this.send(value===2?0x7F:0x00);
      if (value===2){theDeck.orientationButtonLeft.ignoreNextOff = true;}
      this.ignoreNext=true;
      print("called right callback :"+value);
    },
  });

  this.pflButton = new components.Button({
    midi: [0x90,0x30+channel,0xB0,0x3F+channel],
    key: "pfl",
    input: function (channel, control, value, status, group) {
      value/=0x7F;
      if (this.inGetParameter()!==value){
        this.inSetParameter(value);
      }
    },
  });
  this.loadButton = new components.Button({
    midi:[0x90+channel,0x06],
    key: "LoadSelectedTrack",
    shift: function () {this.inKey="eject"},
    unshift: function () {this.inKey="LoadSelectedTrack"},
    input: function () {
      this.inSetParameter(true);
    },
  });
  this.playButton = new components.PlayButton({
    midi: [0x90+channel,0x11,0xB0+channel,0x09],
  });

  this.cueButton = new components.CueButton({
    midi: [0x90+channel,0x10,0xB0+channel,0x08],
  });

  this.jogWheelScratchEnable = function (channelmidi, control, value, status, group) {
    if (value===0x7F) {
      engine.scratchEnable(channel,
                           NumarkN4.scratchSettings["jogResolution"],
                           NumarkN4.scratchSettings["vinylSpeed"],
                           NumarkN4.scratchSettings["alpha"],
                           NumarkN4.scratchSettings["beta"]);
    } else {
      engine.scratchDisable(channel);
    }
  };
  this.searchButton = function (channelmidi, control, value, status, group) {
    theDeck.isSearching=!theDeck.isSearching;
    midi.sendShortMsg(0xB0+channel,0x12,this.isSearching?0x7F:0x00);
  };

  this.jogWheelTurn = function (channelmidi, control, value, status, group) {
    if (theDeck.isSearching) {value*=NumarkN4.searchAmplification;}
    if (engine.isScratching(channel)) {
      engine.scratchTick(channel,value<0x40?value:value-0x7F);
    } else {
      engine.setValue(theDeck.group,"jog",value<0x40?value:value-0x7F)
    }
  };
  this.manageChannelIndicator = function () {
    if (engine.getValue(theDeck.group,"playposition")>NumarkN4.warnAfterPosition) {
      this.alternating=!this.alternating; //mimics a static variable silimar to C(++)
      midi.sendShortMsg(0xB0,0x1D+channel,this.alternating?0x00:0x7F);
    } // BUG: LED stays off when jumping to < threshold.
  };

  this.pitchBendMinus = new components.Button({
    midi: [0x90+channel,0x18,0xB0+channel,0x3D],
    key: "rate_temp_down",
    shift: function (){
      this.inkey = "rate_temp_down_small"
    },
    unshift: function () {
      this.inkey = "rate_temp_down"
    }
  });
  this.pitchBendPlus = new components.Button({
    midi: [0x90+channel,0x19,0xB0+channel,0x3C],
    key : "rate_temp_up",
    shift: function (){
      this.inkey = "rate_temp_up_small";
    },
    unshift: function () {
      this.inkey = "rate_temp_up";
    }
  });
  this.syncButton = new components.SyncButton({
    midi: [0x90+channel,0x0F,0xB0+channel,0x07],
  });
  this.tapButton = function (channelmidi, control, value, status, group) {
    if (value===0x7F) {
      bpm.tapButton(channel);
    }
    midi.sendShortMsg(0xB0+channel,control,value);
  };

  this.keylockButton = new components.Button({
    midi: [0x90+channel,0x1B,0xB0+channel,0x10],
    key: "keylock",
    type: components.Button.prototype.types.toggle,
  });

  // BUG: NOT WORKING
  // BUG: INVERTED; Maybe has to done via components.Pot
  this.pitchRange = new components.Button({
    midi: [0x90+channel,0x1A,0xB0+channel,0x1C],
    key: "rateRange",
    input: function () {
      if (theDeck.rateRangeEntry+1===NumarkN4.rateRanges.length){
        theDeck.rateRangeEntry=0;
      }
      this.inSetValue(NumarkN4.rateRanges[theDeck.rateRangeEntry]);
    },
    output: function () {
      this.send(theDeck.rateRangeEntry%2); // BUG: lighting "not changing" when switching between values that are both even/uneven
    },
  });
  this.pitchLedHandler = engine.makeConnection(theDeck.group,"rate",function (value){
    midi.sendShortMsg(0xB0+channel,0x37,!value);
  });


  this.reconnectComponents(function (c) {
       if (c.group === undefined) {
           // 'this' inside a function passed to reconnectComponents refers to the ComponentContainer
           // so 'this' refers to the custom Deck object being constructed
           c.group = this.currentDeck;
       }
   });
}
NumarkN4.Deck.prototype = components.ComponentContainer.prototype;

NumarkN4.shutdown = function () {
  //destroy mixer, decks, and their timers.
};


//NOTE

//Deck
//  [x] shift
//  [x] play
//  [x] cue
//  [x] PitchBend+-
//  [x] Sync
//  [x] Tap
//  [x] keyLock
//  [x] PitchRange
//  [x] BPM MSB
//  [x] JOGWHEEL
//  [x] Scratch/Search
//  ComponentContainer
//    Normal:
//      [ ] loop_in = loop_in
//      [ ] loop_out = loop_out
//      [ ] reloop = reloop
//      [ ] time elapsed = loop at this location.
//      [ ] FX1 On = hotcue 1 / 5
//      [ ] FX2 On = hotcue 2 / 6
//      [ ] FX3 On = hotcue 3 / 7
//      [ ] FX Auto/Tap = hotcue 4 / 8
//      [ ] FX1 Param = FX1 Super
//      [ ] FX2 Param = FX2 Super
//      [ ] FX3 Param = move loop
//      [ ] FX Beats multi =  loop length
//    Shift:
//      [ ] loop_in = FX1 toggle
//      [ ] loop_out = FX2 toggle
//      [ ] reloop = slip
//      [ ] time elapsed = repeat
//      [ ] FX1 On = hotcue 1
//      [ ] FX2 On = hotcue 2
//      [ ] FX3 On = hotcue 3
//      [ ] FX Auto/Tap = hotcue 4
//      [ ] FX1 Param = FX1 Mix
//      [ ] FX2 Param = FX2 Mix
//      [ ] FX3 Param = hotcue page select
//      [ ] FX Beats multi = loop size

//MIXER: (MSB)
//  [x] Crossfader
//  [x] InputSwitches (MixxxControl: "mute")
//  [x] crossfaderCurve BUG: wierd behavior.
//  [x] DeckStateLeds NOTE: blinks when < (NumarkN4.warnAfterPosition)% remaining
//  [x] Nav_Encoder
//  [x] whichDecLedsIndicators
//  Channel:
//    [x] LOAD
//    [x] Orientation
//    [ ] gainKnob
//    [ ] highknob
//    [ ] midknob
//    [ ] lowknob
//    [x] pfl
//    [x] Volume
