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

NumarkN4.cueReverseRoll=true;

// possible ranges (0.0..3.0 where 0.06=6%)
NumarkN4.rateRanges = [0,   // default (gets set via script later; don't modifify)
                       0.06,// one semitone
                       0.24,// for maximum freedom
                     ];

//
// CONSTANTS DO NOT CHANGE (if you don't know what you are doing)
//
NumarkN4.loopSizes=[0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64];
NumarkN4.QueryStatusMessage=[0xF0,0x00,0x01,0x3F,0x7F,0x47,0x60,0x00,0x01,0x54,0x01,0x00,0x00,0x00,0x00,0xF7];
//NumarkN4.ShutoffSequence=[0xF0,0x00,0x01,0x3F,0x7F,0x47,0xB0,0x39,0x00,0x01,0xF7]; // Invalid Midibyte?

NumarkN4.vinylTouched = [false,false,false,false,];

NumarkN4.globalShift = false;

//NumarkN4.pflVuMeterConnections = [0,0];

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
  NumarkN4.rateRanges[0]=engine.getValue("[Channel1]","rateRange");
  //PFL-Master-Mix & headGain are not being controlled by the controller,
  //this sets them in engine to something reasonable so the user doesn't get confused if there is not sound.
  engine.setParameter("[Master]","headGain",Math.max(engine.getParameter("[Master]","headGain"),0.8));
  engine.setParameter("[Master]","headMix",0); //Headphone out = 100% pfl signal.
  NumarkN4.Decks=[];
  for (var iterator=1;iterator<=4;iterator++){
    NumarkN4.Decks[iterator] = new NumarkN4.Deck(iterator);

  }

  NumarkN4.Mixer = new NumarkN4.MixerTemplate();

  //query controller for component status
  midi.sendSysexMsg(NumarkN4.QueryStatusMessage,NumarkN4.QueryStatusMessage.length)

};

NumarkN4.topContainer = function (channel) {
  this.group = '[Channel'+channel+']';
  var theContainer = this;

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
    this.hotcueButtons[counter] = new components.HotcueButton({
      midi: [0x90+channel,0x27+counter,0xB0+channel,0x18+counter],
      number: counter+1,
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
    hotCuePage: 0,
    shift: function () {
      this.group=theContainer.group;
      this.inKey="beatloop_size";
      this.input = function (channel, control, value, status, group) {
        this.currentLoopSizeIndex=
        Math.min(
          Math.max(
            this.currentLoopSizeIndex+(value===0x01?1:-1),
            NumarkN4.loopSizes.length
          ),
          0
        );
        this.inSetValue(NumarkN4.loopSizes[this.currentLoopSizeIndex]);
      };
    },
    unshift: function () {
      this.input = function (channel, control, value, status, group) {
        if (this.timer) {engine.stopTimer(this.timer)}
        this.hotCuePage=
        Math.max(
          Math.min(
            this.hotCuePage+(value===0x01?1:-1),
            3
          ),
          0
        );
        var number = 0;
        for (var i=0;i<theContainer.hotcueButtons.length;++i) {
          number = (i+1)+theContainer.hotcueButtons.length*this.hotCuePage;
          theContainer.hotcueButtons[i].disconnect();
          theContainer.hotcueButtons[i].number=number;
          theContainer.hotcueButtons[i].outKey='hotcue_' + number + '_enabled';
          theContainer.hotcueButtons[i].unshift(); // for setting inKey based on number property.
          theContainer.hotcueButtons[i].connect();
          theContainer.hotcueButtons[i].trigger();
        }
        for (var i=0;i<4;++i) {
          midi.sendShortMsg(0xB0+channel,0x0B+i,(i-this.hotCuePage)?0x00:0x7F);
        }
        this.timer=engine.beginTimer(1000,function () {theContainer.reconnectComponents()},true);
      }
    },
  });
  this.encSample4 = new components.Encoder({
    midi: [0xB0+channel,0x59],
    currentJumpSizeIndex: 7, //custom property
    shift: function () {
      this.inKey="beatjump_size";
      this.input = function (channel, control, value, status, group) {
        this.currentJumpSizeIndex=Math.max(Math.min(this.currentJumpSizeIndex+(value===0x01?1:-1),NumarkN4.loopSizes.length),0);
        this.inSetValue(NumarkN4.loopSizes[this.currentJumpSizeIndex]);
      };
    },
    unshift: function () {
      this.input = function (channel, control, value, status, group) {
        script.triggerControl(this.group,(value===1)?"beatjump_forward":"beatjump_backward");
      };
    },
  });
};
NumarkN4.topContainer.prototype = new components.ComponentContainer();

NumarkN4.MixerTemplate = function () {
  var theMixer=this;
  //channel will always be 0 it can be "hardcoded" into the components
  this.deckChangeL = new components.Button ({
    midi: [0xB0,0x50],
    input: function (channel, control, value, status, group) {
      this.output(value);
      //just "echos" the midi since the controller knows the deck its on itself but doesnt update the corresponing leds.
    },
  });
  this.deckChangeR = new components.Button ({
    midi: [0xB0,0x51],
    input: function (channel, control, value, status, group) {
      this.output(value);
    },
  });


  // NOTE: Unable to control Volume Bar via software.
  this.pflVuMeter = function (channel, control, value, status, group) {
    // print("calledPflVuMeter");
    // print(NumarkN4.pflVuMeterConnections);
    // for (var i=0;i<NumarkN4.pflVuMeterConnections.length;i++) {
    //   print(i);
    //   if (NumarkN4.pflVuMeterConnections[i]) {NumarkN4.pflVuMeterConnections[i].disconnect();}
    //   if (value) {
    //     NumarkN4.pflVuMeterConnections[i] = engine.makeConnection("[Channel"+value+"]",("VuMeter"+((i===0)?"L":"R")),
    //     function (callbackValue){
    //       print("called channel vumeter: "+callbackValue)
    //       midi.sendShortMsg(0xB0,0x34+i,Math.round(callbackValue*8+0,5)); // Vu bar range=[0;8]
    //     });
    //   }
    //   NumarkN4.pflVuMeterConnections[i].trigger();
    // }
  }
  this.channelInputSwitcherL = new components.Button({
    midi: [0x90,0x49],
    inKey: "mute",
  });
  this.channelInputSwitcherR = new components.Button({
    midi: [0x90,0x4A],
    inKey: "mute",
  });

  // NOTE: [Mixer Profile] is not a documented group. Taken from script.crossfaderCurve
  // BUG: Help on Mixxx forum is needed
  this.changeCrossfaderContour = new components.Button({
    midi: [0x90,0x4B],
    group: "[Mixer Profile]",
    inKey: "xFaderMode",
    input: function (channel, control, value, status, group) {
      if (this.isPress(channel,control, value, status)) {
        engine.setValue("[Mixer Profile]","xFaderCurve",2); // REVIEW: value yet to review
      } else {
        engine.setValue("[Mixer Profile]","xFaderCalibration",0.5); // REVIEW: value yet to review
      }
      this.inSetParameter(!value);
    }
  });

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
      this.type=components.Button.prototype.types.toggle;
      this.group="[Master]";
      this.inKey="maximize_library";
    },
    unshift: function () {
      this.type=components.Button.prototype.types.push;
      this.group="[Library]";
      this.inKey="GoToItem";
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
    shift: function () {
      this.group="[QuickEffectRack1_"+theDeck.group+"]";
      this.inKey="super1";
    },
    unshift: function () {
      this.group=theDeck.group;
      this.inKey="pregain";
    }
  })
  this.blinkTimer=engine.beginTimer(NumarkN4.blinkInterval,this.manageChannelIndicator);
  //timer is more efficent is this case than a callback because it would be called too often.
  engine.makeConnection(this.group,"track_loaded",function (value) {midi.sendShortMsg(0xB0,0x1D+channel,value?0x7F:0x00)});
  this.shiftButton = new components.Button({
    midi: [0x90+channel,0x12,0xB0+channel,0x15],
    type: components.Button.prototype.types.powerWindow,
    state: false, //custom property
    inToggle: function () {
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
    flickerSafetyTimeout: true,
    input: function (channel, control, value, status, group) {
      if (this.flickerSafetyTimeout) {
        this.flickerSafetyTimeout=false;
        value/=0x7F;
        if (this.inGetParameter()!==value){
          this.inSetParameter(value);
        }
        engine.beginTimer(100,function () {
          this.flickerSafetyTimeout=true;
        },true);
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
    reverseRollOnShift: NumarkN4.cueReverseRoll,
  });

  this.jogWheelScratchEnable = new components.Button({
    midi:[0x90+channel,0x2C],
    input: function (channelmidi, control, value, status, group) {
      if (this.isPress(channel, control, value, status)) {
        engine.scratchEnable(channel,
                             NumarkN4.scratchSettings.jogResolution,
                             NumarkN4.scratchSettings.vinylSpeed,
                             NumarkN4.scratchSettings.alpha,
                             NumarkN4.scratchSettings.beta);
      } else {
        engine.scratchDisable(channel);
      }
    },
   });

  this.searchButton = new components.Button({
    midi: [0x90+channel,0x00,0xB0+channel,0x12],
    input: function (channelmidi, control, value, status, group) {
      theDeck.isSearching=!theDeck.isSearching;
      this.output(theDeck.isSearching?0x7F:0x00)
    },
  });

  this.jogWheelTurn = new component.Pot({
    midi: [0xB0+channel,0x2C],
    inKey: "jog",
    input: function (channelmidi, control, value, status, group) {
      value=(value<0x40?value:value-0x7F); // centers values at 0
      if (theDeck.isSearching) {value*=NumarkN4.searchAmplification;}
      if (engine.isScratching(channel)) {
        engine.scratchTick(channel,value);
      } else {
        this.inSetValue(value);
      }
    },
  });

  this.manageChannelIndicator = function () {
    this.alternating=!this.alternating; //mimics a static variable
    midi.sendShortMsg(0xB0,0x1D+channel,((engine.getValue(theDeck.group,"playposition")>NumarkN4.warnAfterPosition)&&this.alternating?0x7F:0x0));
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
  this.tapButton = new component.Button({
    midi: [0x90+channel,0x1E,0xB0+channel,0x16],
    input: function (channelmidi, control, value, status, group) {
      if (value) {
        bpm.tapButton(channel);
      }
      this.output(value);
    },
  });

  this.keylockButton = new components.Button({
    midi: [0x90+channel,0x1B,0xB0+channel,0x10],
    type: components.Button.prototype.types.toggle,
    // shift: function () { NOTE: may implement sync_key
    //   this.inKey="quantize";
    //   this.outKey="quantize";
    // },
    unshift: function () {
      this.inKey="keylock";
      this.outKey="keylock";
    }
  });
  this.bpmSlider = new components.Pot({
    midi: [0xB0+channel,0x01,0xB0+channel,0x37], //only specifing input MSB
    inKey: "rate",
    invert: false,
  });
  this.pitchLedHandler = engine.makeConnection(this.group,"rate",function (value){
    midi.sendShortMsg(0xB0+channel,0x37,!value); //inexplicit cast to bool; turns on if value===0;
  });
  this.pitchLedHandler.trigger();


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
};

NumarkN4.Deck.prototype = new components.Deck();

NumarkN4.shutdown = function () {
  if (delete NumarkN4.Decks) {
    print("gracefull shutdown failed")
  }
  //destroy mixer, decks, and their timers.
  // midi.sendSysexMsg(NumarkN4.ShutoffSequence,NumarkN4.ShutoffSequence.length);
};
