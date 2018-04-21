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

NumarkN4.encoderResolution=0.05; // 1/encoderResolution = number of steps going from 0% to 100%

// possible ranges (0.0..3.0 where 0.06=6%)
NumarkN4.rateRanges = [0.06,// one semitone
                       0.24,// for maximum freedom
                     ];
NumarkN4.loopSizes=[0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64];
NumarkN4.QueryStatusMessage=[0xF0,0x00,0x01,0x3F,0x7F,0x47,0x60,0x00,0x01,0x54,0x01,0x00,0x00,0x00,0x00,0xF7];
NumarkN4.ShutoffSequence=[0xF0,0x00,0x01,0x3F,0x7F,0x47,0xB0,0x39,0x00,0x01,0xF7];

NumarkN4.vinylTouched = [false,false,false,false,];

components.Encoder.prototype.input = function (channel, control, value, status, group) {
  this.inSetParameter(
    this.inGetParameter()+(
      (value===0x01)?
      NumarkN4.encoderResolution:
      -NumarkN4.encoderResolution
    )
  );
};

components.Component.prototype.send = function (value) {
  // This Override is supposed to make integration automatic assignment of elements easier.
  // Right now it just allows specifying the input and output bytes (even though the input bytes dont do anything right now.)
  if (this.midi === undefined || this.midi[0] === undefined || this.midi[1] === undefined) {
      return;
  }
  if (this.midi[2]===undefined){//check if output channel/type not explicitly defined
    this.midi[2]=this.midi[0];
  }
  if (this.midi[3]===undefined){//check if output control not explicitly defined
    this.midi[3]=this.midi[1];
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

NumarkN4.init = function (id) {
  NumarkN4.Decks=[];
  for (var iterator=1;iterator<=4;iterator++){
    print("instancing Deck"+iterator);
    NumarkN4.Decks[iterator] = new NumarkN4.Deck(iterator);
    print("finished instancing Deck"+iterator);

  }

  NumarkN4.Mixer = new NumarkN4.MixerTemplate();

  //query controller for component status
  //midi.sendSysexMsg(NumarkN4.QueryStatusMessage,NumarkN4.QueryStatusMessage.length)

};

NumarkN4.topContainer = function (channel) {
  this.group = '[Channel'+channel+']';
  var theContainer = this;
  print("instancing topContainer"+channel);

  this.btnEffect1 = new components.Button({
    midi: [0x90+channel,0x13,0xB0+channel,0x0B],
    shift: function () {
      this.group="[EffectRack1_EffectUnit1]";
      this.type=components.Button.prototype.types.toggle;
      this.inKey="group_[Channel"+channel+"]_enable";
      this.outKey="group_[Channel"+channel+"]_enable";
    },
    unshift: function () {
      this.group=theContainer.group;
      this.type=components.Button.prototype.types.push;
      this.inKey="loop_in";
      this.outKey="loop_in";
    },
  });
  this.btnEffect2 = new components.Button({
    midi: [0x90+channel,0x14,0xB0+channel,0x0C],
    shift: function () {
      this.group="[EffectRack1_EffectUnit2]";
      this.type=components.Button.prototype.types.toggle;
      this.inKey="group_[Channel"+channel+"]_enable";
      this.outKey="group_[Channel"+channel+"]_enable";
    },
    unshift: function () {
      this.group=theContainer.group;
      this.type=components.Button.prototype.types.push;
      this.inKey="loop_out";
      this.outKey="loop_out";
    },
  });
  print("instancing btnSample3");
  this.btnSample3 = new components.Button({
    midi: [0x90+channel,0x15,0xB0+channel,0x0D],
    shift: function () {
      this.type=components.Button.prototype.types.toggle;
      this.inKey="slip_enabled";
      this.outKey="slip_enabled";
    },
    unshift: function () {
      this.type=components.Button.prototype.types.push;
      this.inKey="beatloop_activate";
      this.outKey="beatloop_activate";
    },
  });
  this.btnSample4 = new components.Button({
    midi: [0x90+channel,0x16,0xB0+channel,0x0E],
    shift: function () {
      this.type=components.Button.prototype.types.toggle;
      this.inKey="repeat";
      this.outKey="repeat";
    },
    unshift: function () {
      this.type=components.Button.prototype.types.push;
      this.inKey="reloop_toggle";
      this.outKey="loop_enabled";
    },
  });
  // custom Hotcue Buttons
  this.hotcueButtons=[];

  for (var counter=0;counter<=3;counter++){
    this.hotcueButtons[counter] = new components.Button({
      midi: [0x90+channel,0x27+counter,0xB0+channel,0x18+counter],
      number: counter,
      shift: function () {
        this.inKey="hotcue_"+(this.number+5)+"_activate";
        this.outKey="hotcue_"+(this.number+5)+"_enabled";
      },
      unshift: function () {
        this.inKey="hotcue_"+(this.number+1)+"_activate";
        this.outKey="hotcue_"+(this.number+1)+"_enabled";
      },
    });
  }
  this.encFxParam1 = new components.Encoder({
    midi:[0xB0+channel,0x57],
    group: "[EffectRack1_EffectUnit1]",
    shift: function () {
      this.inKey="mix";
    },
    unshift: function () {
      this.inKey="super1";
    },
  });
  this.encFxParam2 = new components.Encoder({
    midi:[0xB0+channel,0x58],
    group: "[EffectRack1_EffectUnit2]",
    shift: function () {
      this.inKey="mix";
    },
    unshift: function () {
      this.inKey="super1";
    },
  });
  this.encSample3 = new components.Encoder({
    midi: [0xB0+channel,0x5A],
    currentLoopSizeIndex: 7,
    shift: function () {
      this.group=theContainer.group;
      this.inKey="beatloop_size";
      this.input = function (channel, control, value, status, group) {
        var valueChange=(value===1)?1:-1;
        if ((this.currentLoopSizeIndex+valueChange)>=0&&(this.currentLoopSizeIndex+valueChange)<NumarkN4.loopSizes.length) {
          this.currentLoopSizeIndex+=valueChange;
          this.inSetValue(NumarkN4.loopSizes[this.currentLoopSizeIndex]);
        }
      };
    },
    unshift: function () {
      this.input=components.Encoder.prototype.input;
      this.group="[QuickEffectRack1_"+theContainer.group+"]";
      this.inKey="super1";
    },
  });
  this.encSample4 = new components.Encoder({
    midi: [0xB0+channel,0x59],
    currentJumpSizeIndex: 7, //custom property
    shift: function () {
      this.inKey="beatjump_size";
      this.input = function (channel, control, value, status, group) {
        var valueChange=(value===1)?1:-1;
        if ((this.currentJumpSizeIndex+valueChange)>=0&&(this.currentJumpSizeIndex+valueChange)<NumarkN4.loopSizes.length) {
          this.currentJumpSizeIndex+=valueChange;
          this.inSetValue(NumarkN4.loopSizes[this.currentJumpSizeIndex]);
        }
      };
    },
    unshift: function () {
      this.input = function (channel, control, value, status, group) {
        script.triggerControl(this.group,(value===1)?"beatjump_forward":"beatjump_backward");
      };
    },
  });
  print("finished instancing topContainer"+channel);
};
NumarkN4.topContainer.prototype = new components.ComponentContainer();

NumarkN4.MixerTemplate = function () {
  var theMixer=this;
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
    stepsize: 1,
    shift: function () {
      this.inKey="MoveFocus";
    },
    unshift: function () {
      this.inKey="MoveVertical";
    },
    input: function (midiChannel,control,value,status,group) {
      this.inSetValue(value===0x01?this.stepsize:-this.stepsize); // value "rescaling"; possibly inefficent.
    },
  });
  this.navigationEncoderButton = new components.Button({
    shift: function () {
      this.type=components.Button.prototype.types.push;
      this.group="[Library]";
      this.inKey="GoToItem";
    //   this.input=function (midiChannel,control,value,status,group) {
    //     if (value==0x7F) {
    //     theMixer.navigationEncoderTick.stepsize = (value==0x7F?NumarkN4.LibraryStepSizeSpeedup:1);
    //   } else {
    //     this.inSetValue(this.isPress(midiChannel, control, value, status));
    //   }
    },
    unshift: function () {
      this.type=components.Button.prototype.types.toggle;
      // this.input=components.Button.prototype.input;
      this.group="[Master]";
      this.inKey="maximize_library";
    },
  });
};

NumarkN4.MixerTemplate.prototype = new components.ComponentContainer();

NumarkN4.Deck = function (channel) {
  components.Deck.call(this, channel);
  this.group = '[Channel' + channel + ']';
  this.rateRangeEntry=0;
  this.lastOrientation=(channel%2?0:2);
  this.isSearching=false;
  var theDeck = this;
  this.topContainer = new NumarkN4.topContainer(channel);
  this.topContainer.reconnectComponents(function (component) {
    if (component.group === undefined) {
      component.group = this.group;
    }
  });
  this.eqKnobs = [];
  for (var i = 1; i <= 3; i++) {
      this.eqKnobs[i] = new components.Pot({
          midi: [0xB0, 0x29 + i + 5*(channel-1)],
          group: '[EqualizerRack1_'+theDeck.group+'_Effect1]',
          inKey: 'parameter' + i,
      });
  }
  this.gainKnob = new components.Pot({
    midi: [0xB0, 0x2C + 5*(channel-1)],
    inKey: "pregain",
  })
  // REVIEW: this.blinkTimer=engine.beginTimer(NumarkN4.blinkInterval,"NumarkN4.Deck"+channel+".manageChannelIndicator");
  //timer is more efficent is this case than a callback because it would be called too often.
  // REVIEW: engine.makeConnection(this.group,"track_loaded",function (value) {midi.sendShortMsg(0xB0,0x1D+channel,value?0x7F:0x00)});
  this.shiftButton = new components.Button({
    midi: [0x90+channel,0x12,0xB0+channel,0x15],
    type: components.Button.prototype.types.powerWindow,
    state: false, //custom property
    inToggle: function () {
      print("topContainer.group: "+theDeck.topContainer.group);
      this.state=!this.state;
      if (this.state) {
        theDeck.shift();
        NumarkN4.Mixer.shift();
      } else {
        theDeck.unshift();
        NumarkN4.Mixer.unshift();
      }
      this.output(this.state);
      theDeck.topContainer.reconnectComponents(function (component) {
        if (component.group === undefined) {
          component.group = this.group;
        }
      });
    },
  });

  // NOTE: THE ORIENATION BUTTONS BEHAVE REALLY WIERD AND THE FOLLOWING IS REALLY CONFUSING BUT WORKS!
  this.orientationButtonLeft = new components.Button({
    midi:[0x90,0x32+channel*2,0xB0,0x42+channel*2],
    key: "orientation",
    input: function (channel, control, value, status, group) {
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
    },
  });
  this.orientationButtonRight = new components.Button({
    midi:[0x90,0x33+channel*2,0xB0,0x43+channel*2],
    key: "orientation",
    input: function (channel, control, value, status, group) {
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
    inKey: "LoadSelectedTrack",
    shift: function () {this.inKey="eject";},
    unshift: function () {this.inKey="LoadSelectedTrack";},
    input: function (channelmidi,control,value) {
      this.inSetParameter(this.inValueScale(value));
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
                           NumarkN4.scratchSettings.jogResolution,
                           NumarkN4.scratchSettings.vinylSpeed,
                           NumarkN4.scratchSettings.alpha,
                           NumarkN4.scratchSettings.beta);
    } else {
      engine.scratchDisable(channel);
    }
  };
  this.searchButton = new components.Button({
    midi: [0x90+channel,0x00,0xB0+channel,0x12],
    input: function (channelmidi, control, value, status, group) {
      theDeck.isSearching=!theDeck.isSearching;
      this.output(theDeck.isSearching?0x7F:0x00)
    },
  });

  this.jogWheelTurn = function (channelmidi, control, value, status, group) {
    value=(value<0x40?value:value-0x7F);
    if (theDeck.isSearching) {value*=NumarkN4.searchAmplification;}
    if (engine.isScratching(channel)) {
      engine.scratchTick(channel,value);
    } else {
      engine.setValue(theDeck.group,"jog",value);
    }
  };
  this.manageChannelIndicator = function () {
    this.alternating=!this.alternating; //mimics a static variable
    //midi.sendShortMsg(0xB0,0x1D+channel,((engine.getValue(theDeck.group,"playposition")>NumarkN4.warnAfterPosition)&&this.alternating?0x7F:0x0));
  };

  this.pitchBendMinus = new components.Button({
    midi: [0x90+channel,0x18,0xB0+channel,0x3D],
    key: "rate_temp_down",
    shift: function (){
      this.inkey = "rate_temp_down_small";
    },
    unshift: function () {
      this.inkey = "rate_temp_down";
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
    type: components.Button.prototype.types.toggle,
    key: "quantize",
    // shift: function () {
    //   this.inKey="quantize";
    //   this.outKey="quantize";
    // },
    // unshift: function () {
    //   this.inKey="keylock";
    //   this.outKey="keylock";
    // }
  });
  this.bpmSlider = new components.Pot({
    midi: [0xB0+channel,0x01,0xB0+channel,0x37], //only specifing input MSB
    inKey: "rate",
    invert: true,
    group: theDeck.group,
  });
  this.pitchLedHandler = engine.makeConnection(this.group,"rate",function (value){
    midi.sendShortMsg(0xB0+channel,0x37,!value); //inexplicit cast to bool; turns on if value===0;
  });


  this.pitchRange = new components.Button({
    midi: [0x90+channel,0x1A,0xB0+channel,0x1C],
    key: "rateRange",
    ledState: false,
    input: function () {
      if (theDeck.rateRangeEntry===NumarkN4.rateRanges.length){
        theDeck.rateRangeEntry=0;
      }
      this.inSetValue(NumarkN4.rateRanges[theDeck.rateRangeEntry++]);
    },
    // NOTE: Just toggles to provide some visual Feedback.
    output: function () {
      this.send(this.ledState);
      this.ledState=!this.ledState;
    },
  });

  this.reconnectComponents(function (c) {
       if (c.group === undefined) {
           // 'this' inside a function passed to reconnectComponents refers to the ComponentContainer
           // so 'this' refers to the custom Deck object being constructed
           c.group = this.currentDeck;
       }
   });
   print("assigning Deck"+channel);
};

NumarkN4.Deck.prototype = new components.Deck();

print("assigned Deck constructor");
NumarkN4.shutdown = function () {
  //destroy mixer, decks, and their timers.
};



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
//      [x] loop_in = loop_in
//      [x] loop_out = loop_out
//      [x] reloop = reloop
//      [x] time elapsed = loop at this location.
//      [x] FX1 On = hotcue 1 / 5
//      [x] FX2 On = hotcue 2 / 6
//      [x] FX3 On = hotcue 3 / 7
//      [x] FX Auto/Tap = hotcue 4 / 8
//      [x] FX1 Param = FX1 Super
//      [x] FX2 Param = FX2 Super
//      [x] FX3 Param = beatjump (imitates two buttons)
//      [x] FX Beats multi =  filter
//    Shift:
//      [x] loop_in = FX1 toggle
//      [x] loop_out = FX2 toggle
//      [x] reloop = slip
//      [x] time elapsed = repeat track
//      [x] FX1 On = hotcue
//      [x] FX2 On = hotcue
//      [x] FX3 On = hotcue
//      [x] FX Auto/Tap = hotcue
//      [x] FX1 Param = FX1 Mix
//      [x] FX2 Param = FX2 Mix
//      [x] FX3 Param = beatjump_size
//      [x] FX Beats multi = loop length box

//MIXER: (MSB)
//  [ ] Nav Encoder Layer;
//    [ ] unshift:
//      [ ] Btn: maximize_library
//      [ ] Enc: MoveVertical
//    [ ] shift:
//      [ ] Btn:
//        [ ] short press: [Library] GoToItem
//        [ ] long press: set enc.inKey="MoveFocus"
//      [ ] Enc: MoveVertical / ^^C
//  [x] Crossfader
//  [x] InputSwitches (MixxxControl: "mute")
//  [x] crossfaderCurve BUG: wierd behavior.
//  [x] DeckStateLeds NOTE: blinks when < (NumarkN4.warnAfterPosition)% remaining
//  [x] whichDecLedsIndicators
//  Channel:
//    [x] LOAD
//    [x] Orientation
//    [x] gainKnob
//    [x] highknob
//    [x] midknob
//    [x] lowknob
//    [x] pfl
//    [x] Volume
