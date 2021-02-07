var CMDMM = new components.ComponentContainer();
//GLOBAL VARS

var channelNumber = 5;

var invertColor = true; //false=(off=orange,on=blue);true=(off=blue,on=orange);

var defaultChannelSequence = [3, 1, 2, 4];
var channelMode = [true, true, true, true]; //true=deck;false=fxChannel
var faderMode = [true, true, true, true]; // true=super;false=rate (only affects fxMode Channels)

var standardKnobBehavior = 0; // 0 = [High,Mid,Low,Quickeffect]; 1 = [Gain,High,Mid,Low]; 2 = [Effect1Meta,Effect2Meta,Effect3Meta,mix];
var navEncoderScale = 5; // The amount of steps mixxx will scroll within the library while pressing the encoder;


//0x00 = orange; 0x01 = blue;
components.Button.prototype.on=(invertColor ? 0x00 : 0x01);
components.Button.prototype.off=(invertColor ? 0x01 : 0x00);


components.ComponentContainer.prototype.layer = function (layer) {
  this.forEachComponent(function (component) {
      if (component.before !== undefined && typeof component.before === "function") {component.before();}
      if (typeof component["layer"+layer] === 'function') {
          if (component instanceof components.Button
              && (component.type === components.Button.prototype.types.push
                  || component.type === undefined)
              && component.input === components.Button.prototype.input
              && typeof component.inKey === 'string'
              && typeof component.group === 'string') {
              if (engine.getValue(component.group, component.inKey) !== 0) {
                  engine.setValue(component.group, component.inKey, 0);
              }
          }
          // component.layer(layer);
          component["layer"+layer]();
      }
      if (component.after !== undefined && typeof component.after === "function") {component.after();}
      // Set isShifted for child ComponentContainers forEachComponent is iterating through recursively
      this.isShifted = (layer!==1);
  });
  CMDMM.reconnectComponents(function (component) {
    if (component.group === undefined) {
      component.group = this.group;
    }
  });
};

components.Button.prototype.before=function () {
  this.output=components.Button.prototype.output;
  this.input=components.Button.prototype.input;
};
components.Button.prototype.after=function () {
  this.outKey=this.inKey;
};
var MIDI = {};
MIDI.noteOn = 0x90 + (channelNumber - 1);
MIDI.noteOff = 0x80 + (channelNumber - 1);
MIDI.CC = 0xB0 + (channelNumber - 1);
MIDI.ControllerDump = [0xF0,0x00,0x20,0x7F,0x03,0x01,0xF7];

CMDMM.currentLayer=1;
// Abstract:
// The Mapping consist of the following Objects:
// CMDMM:
// - EQAndGain (generic Template)
// - EQAndQuickEffect (generic Template)
// - FXKnobs (generic Template)
// - deckChannel (generic Template)
//  - knobUnit = EQAndGain || EQAndQuickEffect
//  - button1 (specific functionality for the deckchannel)
//  - button2 (see button1)
//  - buttonCue (see button1)
//  - fader (volume || pitch)
// - FXChannel (generic Template)
//  - knobUnit = FXKnobs
//  - button1 (specific functionality for the fxChannel)
//  - button2 (see button1)
//  - buttonCue (see button1)
//  - fader (FxUnitMix)
// - Decks[4]
//  - four instances of the decktype indicated by CMMDMM.channelMode (can be set at the top)
// - middleButton
// - shiftButton
// - ctrlButton
// - VUMeters
// - CFader
// - library encoder knob
// - HeadGain
// - HeadMix
// - out1 (masterBalance)
// - out2 (MasterGain)

// The Component name usually match with the corresponding label on the Controller.
// ButtonCue, Button1 and Button2 are derived from their label on a channel.
// Layout on the Controller:
// +-------------------+
// |                   |
// |   +---+   +---+   |
// |   | 1 |   | 2 |   |
// |   +---+   +---+   |
// |                   |
// |   +-----------+   |
// |   |    Cue    |   |
// |   +-----------+   |
// |                   |
// +-------------------+

// the Shiftbuttons will correspond to the layers in the following combination:
// unpressed/default: layer1
// shift: layer2
// ctrl: layer3
// shift+ctrl: layer4

CMDMM.EQAndGain = function (channel, baseAddress) {
  this.knobs = [];
  this.knobs[0] = new components.Pot({
    midi: [MIDI.CC, baseAddress],
    group: "[Channel"+channel+"]",
    inKey: "pregain",
  });
  for (var i = 1; i <= 3; i++) {
    this.knobs[i] = new components.Pot({
      midi: [MIDI.CC, baseAddress + 4*(i-1)],
      group: '[EqualizerRack1_[Channel'+channel+']_Effect1]',
      inKey: 'parameter' + (4-i),
      // parameter has to be assigned in reverse.
    });
  }
};
CMDMM.EQAndGain.prototype = new components.ComponentContainer();

CMDMM.EQAndQuickEffect = function (channel, baseAddress) {
  this.knobs=[];
  for (var i = 1; i <= 3; i++) {
    this.knobs[i-1] = new components.Pot({
      midi: [MIDI.CC, baseAddress + 4*(i-1)],
      group: '[EqualizerRack1_[Channel'+channel+']_Effect1]',
      inKey: 'parameter' + (4-i),
      // parameter has to be assigned in reverse.
    });
  }
  this.knobs[3] = new components.Pot({
    midi: [MIDI.CC, baseAddress+3*4],
    // (third knob of channel (zero-based)) * (offset of knobs to one another)
    group: "[QuickEffectRack1_[Channel"+channel+"]]",
    inKey: "super1",
  });
};
CMDMM.EQAndQuickEffect.prototype = new components.ComponentContainer();

CMDMM.FXKnobs = function (channel, baseAddress) {
  this.knobs = [];
  for (var i = 1; i <= 3; i++) {
    this.knobs[i-1] = new components.Pot({
      midi: [MIDI.CC, baseAddress + 4*(i-1)],
      group: '[EffectRack1_EffectUnit'+channel+'_Effect'+i+']',
      inKey: 'meta',
    });
  }
  this.knobs[3] = new components.Pot({
    midi: [MIDI.CC, baseAddress+3*4],
    // (third knob of channel (zero-based)) * (offset of knobs to one another)
    group: '[EffectRack1_EffectUnit'+channel+']',
    inKey: "mix",
  });
};
CMDMM.FXKnobs.prototype = new components.ComponentContainer();

CMDMM.deckChannel = function (physicalChannel,virtualChannel) {
  var baseAddress=0x06+physicalChannel;
  var theDeck = this;
  this.currentDeck="[Channel"+virtualChannel+"]";
  this.virtualChannel=virtualChannel;
  this.binaryChannel=virtualChannel-1;
  switch (standardKnobBehavior) {
    case 0:
      this.knobUnit = new CMDMM.EQAndQuickEffect(virtualChannel,baseAddress);
      break;
    case 1:
      this.knobUnit = new CMDMM.EQAndGain(virtualChannel,baseAddress);
      break;
    default:
      print("WRONG KNOBUNIT TYPE!\nfalling back to EQAndQuickEffect");
      standardKnobBehavior=0;
      this.knobUnit = new CMDMM.EQAndQuickEffect(virtualChannel,baseAddress);
  }
  this.button1 = new components.Button({
    midi: [MIDI.noteOn,0x0D+baseAddress+physicalChannel*3],
    type: components.Button.prototype.types.toggle,
    mask: 1 << 0,
    layer1: function () {
      // this will only set the control to the left side or center
      // button2 will assign it to to the right side / center
      this.inKey="orientation";
      this.group=theDeck.currentDeck;
      this.output = function (value, group, control) {
          this.send(this.outValueScale(value===0));
      };
      this.input = function (channel, control, value, status, group) {
        this.inSetValue(this.inGetValue()!==0?0:1);
      };
    },
    layer2: function () {
      this.inKey="group_"+theDeck.currentDeck+"_enable";
      this.group="[EffectRack1_EffectUnit1]";
    },
    layer3: function () {
      this.inKey="group_"+theDeck.currentDeck+"_enable";
      this.group="[EffectRack1_EffectUnit3]";
    },
    layer4: function () {
      this.inKey="";
      this.group="";
      this.input = function (channel, control, value, status, group) {
        theDeck.binaryChannel^=this.mask;
        this.output(theDeck.binaryChannel&this.mask);
        theDeck.virtualChannel=theDeck.binaryChannel+1;
        theDeck.knobUnit.currentDeck="[Channel"+theDeck.virtualChannel+"]";
        theDeck.setCurrentDeck("[Channel"+theDeck.virtualChannel+"]");
      };
      this.output(theDeck.binaryChannel&this.mask);
    }
  });
  this.button2 = new components.Button({
    midi: [MIDI.noteOn,0x0E+baseAddress+physicalChannel*3],
    type: components.Button.prototype.types.toggle,
    mask: 1 << 1,
    layer1: function () {
      // this will only set the control to the right side or center
      // button1 will assign it to to the left side / center
      this.inKey="orientation";
      this.group=theDeck.currentDeck;
      this.output = function (value, group, control) {
          this.send(this.outValueScale(value===2));
      };
      this.input = function (channel, control, value, status, group) {
        this.inSetValue(this.inGetValue()!==2?2:1);
      };
    },
    layer2: function () {
      this.inKey="group_"+theDeck.currentDeck+"_enable";
      this.group="[EffectRack1_EffectUnit2]";
    },
    layer3: function () {
      this.inKey="group_"+theDeck.currentDeck+"_enable";
      this.group="[EffectRack1_EffectUnit4]";
    },
    layer4: function () {
      this.inKey="";
      this.group="";
      this.input = function (channel, control, value, status, group) {
        theDeck.binaryChannel^=this.mask;
        this.output(theDeck.binaryChannel&this.mask);
        theDeck.virtualChannel=theDeck.binaryChannel+1;
        theDeck.knobUnit.currentDeck="[Channel"+theDeck.virtualChannel+"]";
        theDeck.setCurrentDeck("[Channel"+theDeck.virtualChannel+"]");
      };
      this.output(theDeck.binaryChannel&this.mask);
    }
  });
  this.buttonCue = new components.Button({
    midi: [MIDI.noteOn,0x2A+baseAddress],
    group: theDeck.currentDeck,
    layer1: function () {
      this.inKey="pfl";
      this.type=components.Button.prototype.types.toggle;
    },
    layer2: function () {
      this.inKey="LoadSelectedTrack";
      this.type=components.Button.prototype.types.push;
    },
    layer3: function () {
      this.inKey="sync_enabled";
      this.type=components.Button.prototype.types.push;
    },
    layer4: function () {
      this.inKey="";
      this.output(this.on);
      this.input = function () {
        CMDMM.Decks[physicalChannel] = new CMDMM.fxChannel(physicalChannel,theDeck.virtualChannel);
        this.output(this.off);
        theDeck.reconnectComponents();
      };
    }
  });
  this.fader = new components.Pot({
    midi: [MIDI.CC,0x2A+baseAddress],
    inKey: "volume",
    group: theDeck.currentDeck,
    after: function () {this.input=components.Pot.prototype.input;},
    layer4: function () {
      this.input = function (channel, control, value, status, group) {
        if (value>42&&value<=84) {
          this.inKey="rate";
        } else {
          this.inKey="volume";
        }
      };
    },
  });
};
CMDMM.deckChannel.prototype = new components.Deck();

CMDMM.fxChannel = function (physicalChannel,virtualChannel) {
  var baseAddress=0x06+physicalChannel;
  var theDeck=this;
  this.currentDeck="[Channel"+virtualChannel+"]";
  this.virtualChannel=virtualChannel;
  this.binaryChannel=virtualChannel-1;
  this.knobUnit = new CMDMM.FXKnobs(virtualChannel,baseAddress);
  this.button1 = new components.Button({
    midi: [MIDI.noteOn,0x0D+baseAddress+physicalChannel*3],
    type: components.Button.prototype.types.toggle,
    mask: 1 << 0,
    layer1: function () {
      this.inKey="enabled";
      this.group="[EffectRack1_EffectUnit"+theDeck.virtualChannel+"_Effect1]";
    },
    layer2: function () {
      this.inKey="group_[Channel1]_enable";
      this.group="[EffectRack1_EffectUnit"+theDeck.virtualChannel+"]";
    },
    layer3: function () {
      this.inKey="group_[Channel3]_enable";
      this.group="[EffectRack1_EffectUnit"+theDeck.virtualChannel+"]";
    },
    layer4: function () {
      this.inKey="";
      this.group="";
      this.input = function (channel, control, value, status, group) {
        theDeck.binaryChannel^=this.mask;
        this.output(theDeck.binaryChannel&this.mask);
        theDeck.virtualChannel=theDeck.binaryChannel+1;
        // creating a new Unit is easier than changing the groups via Regexp
        theDeck.knobUnit = new CMDMM.FXKnobs(theDeck.virtualChannel,baseAddress);
        // the fader doesn't get changed if it is set to control the fxunit
        // (look at components.Deck.setCurrentDeck for more explanation)
        if (theDeck.fader.inKey==="super1") {
          theDeck.fader.group='[EffectRack1_EffectUnit'+theDeck.virtualChannel+']';
        }
        theDeck.setCurrentDeck("[Channel"+theDeck.virtualChannel+"]");
      };
      this.output(theDeck.binaryChannel&this.mask);
    },
  });
  this.button2 = new components.Button({
    midi: [MIDI.noteOn,0x0E+baseAddress+physicalChannel*3],
    mask: 1 << 1,
    type: components.Button.prototype.types.toggle,
    layer1: function () {
      this.inKey="enabled";
      this.group="[EffectRack1_EffectUnit"+theDeck.virtualChannel+"_Effect2]";
    },
    layer2: function () {
      this.inKey="group_[Channel2]_enable";
      this.group="[EffectRack1_EffectUnit"+theDeck.virtualChannel+"]";
    },
    layer3: function () {
      this.inKey="group_[Channel4]_enable";
      this.group="[EffectRack1_EffectUnit"+theDeck.virtualChannel+"]";
    },
    layer4: function () {
      this.inKey="";
      this.group="";
      this.input = function (channel, control, value, status, group) {
        theDeck.binaryChannel^=this.mask;
        this.output(theDeck.binaryChannel&this.mask);
        theDeck.virtualChannel=theDeck.binaryChannel+1;
        // creating a new Unit is easier than changing the groups via Regexp
        theDeck.knobUnit = new CMDMM.FXKnobs(theDeck.virtualChannel,baseAddress);
        // the fader doesn't get changed if it is set to control the fxunit
        // (look at components.Deck.setCurrentDeck for more explanation)
        if (theDeck.fader.inKey==="super1") {
          theDeck.fader.group='[EffectRack1_EffectUnit'+theDeck.virtualChannel+']';
        }
        theDeck.setCurrentDeck("[Channel"+theDeck.virtualChannel+"]");
      };
      this.output(theDeck.binaryChannel&this.mask);
    },
  });
  this.buttonCue = new components.Button({
    midi: [MIDI.noteOn,0x2A+baseAddress],
    type: components.Button.prototype.types.toggle,
    layer1: function () {
      this.inKey="enabled";
      this.group="[EffectRack1_EffectUnit"+theDeck.virtualChannel+"_Effect3]";
    },
    layer2: function () {
      this.inKey="group_[Headphone]_enable";
      this.group="[EffectRack1_EffectUnit"+theDeck.virtualChannel+"]";
    },
    layer3: function () {
      this.inKey="mix_mode";
      this.group="[EffectRack1_EffectUnit"+theDeck.virtualChannel+"]";
    },
    layer4: function () {
      this.inKey="";
      this.group="";
      this.output(this.off);
      this.input = function () {
        CMDMM.Decks[physicalChannel] = new CMDMM.deckChannel(physicalChannel,theDeck.virtualChannel);
        this.output(this.on);
        theDeck.reconnectComponents();
      };
    }
  });
  this.fader = new components.Pot({
    midi: [MIDI.CC,0x2A+baseAddress],
    inKey: (faderMode[physicalChannel]?"super1":"rate"),
    group: (faderMode[physicalChannel]?'[EffectRack1_EffectUnit'+theDeck.virtualChannel+']':"[Channel"+theDeck.virtualChannel+"]"),
    before: function () {
      this.input=components.Pot.prototype.input;
    },
    layer4: function () {
      this.input = function (channael, control, value, status, group) {
        if (value>42&&value<=84) {
          this.inKey="rate";
          this.group="[Channel"+theDeck.virtualChannel+"]";
        } else {
          this.inKey="super1";
          this.group='[EffectRack1_EffectUnit'+theDeck.virtualChannel+']';
        }
      };
    },
  });
};
CMDMM.fxChannel.prototype = new components.Deck();

CMDMM.init = function () {
  CMDMM.Decks=[];
  for (i=0;i<defaultChannelSequence.length;i++) {
    if (channelMode[i]) {
      CMDMM.Decks[i] = new CMDMM.deckChannel(i,defaultChannelSequence[i]);
    } else {
      CMDMM.Decks[i] = new CMDMM.fxChannel(i,defaultChannelSequence[i]);
    }
    CMDMM.Decks[i].reconnectComponents(function (component) {
      if (component.group === undefined) {
        component.group = "[Channel"+defaultChannelSequence[i]+"]";
      }
    });
  }
  CMDMM.shiftButton = new components.Button({
    midi: [MIDI.noteOn,0x10],
    before: undefined,
    after: undefined,
    input: function (channel, control, value, status, group) {
      CMDMM.currentLayer += this.isPress(channel, control, value, status)?1:-1;
      CMDMM.layer(CMDMM.currentLayer);
      CMDMM.reconnectComponents();
    },
  });
  CMDMM.ctrlButton = new components.Button({
    midi: [MIDI.noteOn,0x11],
    before: undefined,
    after: undefined,
    input: function (channel, control, value, status, group) {
      CMDMM.currentLayer += this.isPress(channel, control, value, status)?2:-2;
      CMDMM.layer(CMDMM.currentLayer);
      CMDMM.reconnectComponents();
    }
  });
  CMDMM.crossfader = new components.Pot({
    midi: [MIDI.CC,0x40],
    inKey: "crossfader",
    group: "[Master]",
  });
  CMDMM.middleButton = new components.Button({
    midi:[MIDI.noteOn,0x12],
    key: "crossfader",
    group: "[Master]",
    layer1: function () {
      this.input = function () {this.inSetParameter(0.5);};
      this.output = function () {this.send(this.inGetParameter()===0.5);};
    },
    layer2: function () {
      this.input = function () {
        for (var i = 0; i<CMDMM.Decks.length; i++) {
          if (CMDMM.Decks[i] instanceof CMDMM.deckChannel) {
            if (CMDMM.Decks[i].knobUnit instanceof CMDMM.EQAndQuickEffect) {
              CMDMM.Decks[i].knobUnit = new CMDMM.EQAndGain(CMDMM.Decks[i].virtualChannel,i+0x06);
            } else {
              CMDMM.Decks[i].knobUnit = new CMDMM.EQAndQuickEffect(CMDMM.Decks[i].virtualChannel,i+0x06);
            }
          }
        }
      };
      this.output = undefined;
    },
  });
  CMDMM.out1 = new components.Pot({
    midi: [MIDI.CC,0x01],
    group: "[Master]",
    inKey: "balance",
  });
  CMDMM.out2 = new components.Pot({
    midi: [MIDI.CC,0x02],
    group: "[Master]",
    inKey: "gain",
  });
  CMDMM.cueVol = new components.Pot({
    midi: [MIDI.CC,0x04],
    group: "[Master]",
    inKey: "headGain",
  });
  CMDMM.cueMix = new components.Pot({
    midi: [MIDI.CC,0x05],
    group: "[Master]",
    inKey: "headMix",
  });
  CMDMM.libraryButton = new components.Button({
    midi: [MIDI.noteOn,0x03],
    input: function (channel, control, value, status, group) {
      // no functionality yet. suggestions are appreciated.
    },
  });
  CMDMM.libraryEncoder = new components.Encoder({
    midi: [MIDI.CC,0x03],
    group: "[Library]",
    inKey: "MoveVertical",
    input: function (channel, control, value, status, group) {
      this.inSetValue((value-0x40)*this.speed);
      // this.speed is controlled by the other layers.
    },
    layer1: function () {
      this.inKey="MoveVertical";
      this.speed=1;
    },
    layer2: function () {
      this.inKey="MoveFocus";
      this.speed=1;
    },
    layer3: function () {
      this.inKey="MoveVertical";
      this.speed=navEncoderScale;
    },
  });
  CMDMM.VuMeterL = engine.makeConnection("[Master]","VuMeterL",function (value) {
    midi.sendShortMsg(MIDI.CC, 0x50, (value * 15) + 48);
  });
  CMDMM.VuMeterR = engine.makeConnection("[Master]","VuMeterR",function (value) {
    midi.sendShortMsg(MIDI.CC, 0x51, (value * 15) + 48);
  });
  CMDMM.layer(1);

  // midi.sendSysexMsg(MIDI.ControllerDump,MIDI.ControllerDump.length);
  // Doesn't return anything. The controller might not be serato certified.
};

CMDMM.shutdown = function () {
  engine.setParameter("[Master]","VuMeterL",0);
  engine.setParameter("[Master]","VuMeterR",0);
  for (var i = 0; i<127; i++) {
    midi.sendShortMsg(MIDI.noteOn, i, 0);
    // sets the controller to orange (to match the left/right buttons which only light up in orange);
  }
};
