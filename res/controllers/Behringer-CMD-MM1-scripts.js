var CMDMM = {};
//GLOBAL VARS

var channelNumber = 5;

var invertColor = true; //false=(off=orange,on=blue);true=(off=blue,on=orange);
var defaultChannelSequence = [3, 1, 2, 4];
var channelMode = [true, true, true, true]; //true=deck;false=fxChannel
var standardKnobBehavior = 0; // 0 = [High,Mid,Low,Quickeffect]; 1 = [Gain,High,Mid,Low]; 2 = [Effect1Meta,Effect2Meta,Effect3Meta,mix];
var navEncoderScale = 5; // The amount of steps mixxx will scroll within the library while pressing the encoder;

components.Component.layer = function (layer) {
  if (this.before!==undefined){this.before();}
  if (this["layer"+layer]!==undefined){this["layer"+layer]();} else {print("layer"+layer+" not Found!")}
  if (this.after!==undefined){this.after();}
}

//0x00 = orange; 0x01 = blue;
components.Button.prototype.on=(invertColor ? 0x00 : 0x01);
components.Button.prototype.off=(invertColor ? 0x01 : 0x00);

// components.Deck.prototype = function (newGroup) {
//     this.currentDeck = newGroup;
//     this.reconnectComponents(function (component) {
//         if (component.group.search(script.channelRegEx) !== -1) {
//             component.group = this.currentDeck;
//         } else if (component.group.search(script.eqRegEx) !== -1) {
//             component.group = '[EqualizerRack1_' + this.currentDeck + '_Effect1]';
//         } else if (component.group.search(script.quickEffectRegEx) !== -1) {
//             component.group = '[QuickEffectRack1_' + this.currentDeck + ']';
//         }
//         // Do not alter the Component's group if it does not match any of those RegExs.
//
//         if (component instanceof EffectAssignmentButton) {
//             // The ControlObjects for assinging decks to effect units
//             // indicate the effect unit with the group and the deck with the key,
//             // so change the key here instead of the group.
//             component.inKey = 'group_' + newGroup + '_enable';
//             component.outKey = 'group_' + newGroup + '_enable';
//         }
//     });
// },

components.ComponentContainer.prototype.layer = function (layer) {
  this.forEachComponent(function (component) {
      if (typeof component["layer"+layer] === 'function') {
          if (component instanceof Button
              && (component.type === Button.prototype.types.push
                  || component.type === undefined)
              && component.input === Button.prototype.input
              && typeof component.inKey === 'string'
              && typeof component.group === 'string') {
              if (engine.getValue(component.group, component.inKey) !== 0) {
                  engine.setValue(component.group, component.inKey, 0);
              }
          }
          component.layer(layer);
      }
      // Set isShifted for child ComponentContainers forEachComponent is iterating through recursively
      //this.isShifted = true;
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

CMDMM.currentLayer=1;

CMDMM.EQAndGain = function (channel, baseAddress) {
  var that = this;
  this.knobs = [];
  this.knobs[0] = new components.Pot({
    midi: [MIDI.CC, baseAddress],
    inKey: "pregain",
  });
  for (var i = 1; i <= 3; i++) {
    this.knobs[i] = new components.Pot({
      midi: [MIDI.CC, baseAddress + 4*(i-1)],
      group: '[EqualizerRack1_'+that.group+'_Effect1]',
      inKey: 'parameter' + i,
    });
  }
};
CMDMM.EQAndGain.prototype = new components.ComponentContainer();

CMDMM.EQAndQuickEffect = function (channel, baseAddress) {
  var that = this;
  for (var i = 1; i <= 3; i++) {
    this.knobs[i-1] = new components.Pot({
      midi: [MIDI.CC, baseAddress + 4*(i-1)],
      group: '[EqualizerRack1_'+that.group+'_Effect1]',
      inKey: 'parameter' + i,
    });
  }
  this.knobs[3] = new components.Pot({
    midi: [MIDI.CC, baseAddress+3*4],
    // (third knob of channel (zero-based)) * (offset of knobs to one another)
    inKey: "pregain",
  });
};
CMDMM.EQAndQuickEffect.prototype = new components.ComponentContainer();

CMDMM.FXKnobs = function (channel, baseAddress) {
  var that = this;
  this.knobs = [];
  for (var i = 1; i <= 3; i++) {
    this.knobs[i-1] = new components.Pot({
      midi: [MIDI.CC, baseAddress + 4*(i-1)],
      group: '[EffectRack1_EffectUnit'+that.group+'_Effect'+i+']',
      inKey: 'meta',
    });
  }
  this.knobs[3] = new components.Pot({
    midi: [MIDI.CC, baseAddress+3*4],
    // (third knob of channel (zero-based)) * (offset of knobs to one another)
    group: '[EffectRack1_EffectUnit'+that.group+']',
    inKey: "mix",
  });
}
CMDMM.FXKnobs.prototype = new components.ComponentContainer();

CMDMM.deckChannel = function (channel) {
  var baseAddress=0x05+channel;
  var theDeck = this;
  this.type=true;
  switch (standardKnobBehavior) {
    case 0:
      this.knobUnit = new CMDMM.EQAndQuickEffect(channel,baseAddress);
      break;
    case 1:
      this.knobUnit = new CMDMM.EQAndGain(channel,baseAddress);
      break;
    default:
      print("Invalid Knob Type: "+standardKnobBehavior);
  }
  this.button1 = new components.Button({
    midi: [MIDI.noteOn,0x0D+baseAddress],
    layer1: function () {
      this.inKey="orientation";
      this.output = function (value, group, control) {
          this.send(this.outValueScale(value===0));
      };
      this.input = function (channelUnused, control, value, status, group) {
        this.inSetValue(this.inGetValue()!==0?1:this.inValueScale(value))
      };
    },
    layer2: function () {
      this.inKey="group_[Channel1]_enable";
    },
    layer3: function () {
      this.inKey="group_[Channel3]_enable";
    },
    layer4: function () {
      this.inKey="";
      this.input = function (channel, control, value, status, group) {
        channel^=1; // XOR channel by "10" (binary)
        theDeck.setCurrentDeck("[Channel"+channel+"]");
        this.output(channel&1);
      };
    }
  });
  this.button2 = new components.Button({
    midi: [MIDI.noteOn,0x0E+baseAddress],
    layer1: function () {
      this.inKey="orientation";
      this.output = function (value, group, control) {
          this.send(this.outValueScale(value===2));
      };
      this.input = function (channelUnused, control, value, status, group) {
        this.inSetValue(this.inGetValue()!==2?1:this.inValueScale(value))
      };
    },
    layer2: function () {
      this.inKey="group_[Channel2]_enable";
    },
    layer3: function () {
      this.inKey="group_[Channel4]_enable";
    },
    layer4: function () {
      this.inKey="";
      this.input = function (channel, control, value, status, group) {
        channel^=2; // XOR channel by "10" (binary)
        theDeck.setCurrentDeck("[Channel"+channel+"]");
        this.output(channel&2);
      };
    }
  });
  this.buttonCue = new components.Button({
    midi: [MIDI.noteOn,0x2A+baseAddress],
    layer1: function () {
      this.inKey="pfl";
    },
    layer2: function () {
        this.inKey="LoadSelectedTrack";
    },
    layer4: function () {
      this.inKey="";
      this.output(theDeck.type);
      this.input= function () {
        theDeck.forEachComponent(
          function (component) {
            component.disconnect();
          }
        )
        CMDMM.Decks[baseAddress-0x06] = new (theDeck.type?CMDMM.deckChannel(channel):CMDMM.fxChannel(channel));
      }
    }
  });
  this.fader = new components.Pot({
    midi: [MIDI.CC,0x2A+baseAddress],
    layer4: function () {
      this.input = function (channel, control, value, status, group) {
        if (value>42&&value<=84) {
          this.inKey="rate";
        } else {
          this.inKey="volume";
        }
      };
    },
  })
}
CMDMM.deckChannel.prototype = new components.Deck();

CMDMM.fxChannel = function (channel) {
  var baseAddress=0x05+channel;
  var theDeck=this;
  this.type=false; //helper variable
  this.knobUnit = new CMDMM.FXKnobs(channel,baseAddress);
  this.button1 = new components.Button({
    midi: [MIDI.noteOn,0x0D+baseAddress],
    layer1: function () {
      this.inKey="enabled";
    },
    layer2: function () {
      this.inKey="group_[Channel1]_enable";
    },
    layer3: function () {
      this.inKey="group_[Channel3]_enable";
    },
    layer4: function () {
      this.inKey="";
      this.input = function (channelUnused, control, value, status, group) {
        channel^=1; // XOR channel by "10" (binary)
        theDeck.setCurrentDeck("[Channel"+channel+"]");
        this.output(channel&1);
      };
    },
  });
  this.button2 = new components.Button({
    midi: [MIDI.noteOn,0x0E+baseAddress],
    layer1: function () {
      this.inKey="enabled";
    },
    layer2: function () {
      this.inKey="group_[Channel2]_enable";
    },
    layer3: function () {
      this.inKey="group_[Channel4]_enable";
    },
    layer4: function () {
      this.inKey="";
      this.input = function (channelUnused, control, value, status, group) {
        channel^=2; // XOR channel by "01" (binary)
        theDeck.setCurrentDeck("[Channel"+channel+"]");
        this.output(channel&2);
      };
    },
  });
  this.buttonCue = new components.Button({
    layer1: function () {
      this.inKey="enabled";
    },
    layer2: function () {
      this.inKey="group_[Headphone]_enable";
    },
    layer3: function () {
      this.inKey="group_[Master]_enable";
    },
    layer4: function () {
      this.inKey="";
      this.output(theDeck.type);
      this.input= function () {
        theDeck.forEachComponent(
          function (component) {
            component.disconnect();
          }
        )
        CMDMM.Decks[baseAddress-0x06] = new (theDeck.type?CMDMM.deckChannel(channel):CMDMM.fxChannel(channel));
      }
    }
  });
  this.fader = new components.Pot({
    midi: [MIDI.CC,0x2A+baseAddress],
    before: function () {
      this.input=components.Pot.prototype.input;
    },
    layer4: function () {
      this.input = function (channelUnused, control, value, status, group) {
        if (value>42&&value<=84) {
          this.inKey="rate";
          this.group="[Channel"+channel+"]";
        } else {
          this.inKey="super1";
          this.group='[EffectRack1_EffectUnit'+channel+']';
        }
      };
    },
  });
};
CMDMM.fxChannel.prototype = new components.Deck();


// TODO: Small Knobs + Library Encoder

CMDMM.init = function () {
  CMDMM.Decks=[];
  for (i=0;i<defaultChannelSequence.length;i++) {
    CMDMM.Decks[i] = new (channelMode[i]?
      CMDMM.deckChannel(
        defaultChannelSequence[i]
      ):
      CMDMM.fxChannel(
        defaultChannelSequence[i]
      )
    );
    CMDMM.Decks[i].reconnectComponents(function (component) {
      if (component.group === undefined) {
        component.group = "[Channel"+(defaultChannelSequence[i]+1)+"]";
      }
    });
    print("created Decknr "+i);
  }
  CMDMM.shiftButton = new components.Button({
    midi: [MIDI.noteOn,0x10],
    input: function (channel, control, value, status, group) {
      CDMMM.currentLayer += this.isPress(channel, control, value, status)?1:-1;
    },
  });
  CMDMM.ctrlButton = new components.Button({
    midi: [MIDI.noteOn,0x11],
    input: function (channel, control, value, status, group) {
      CDMMM.currentLayer += this.isPress(channel, control, value, status)?2:-2;
    },
  });
  CMDMM.crossfader = new components.Pot({
    midi: [MIDI.CC,0x40],
    inKey: "crossfader",
    group: "[Master]",
  });
  CMDMM.middleButton = new components.Button({
    midi:[MIDI.noteOn,0x12],
    inKey: "crossfader",
    group: "[Master]",
    input: function () {this.inSetParameter(0.5);},
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
      CMDMM.libraryEncoder.speed = this.isPress(channel, control, value, status)?1:navEncoderScale;
    },
  });
  CMDMM.libraryEncoder = new components.Pot({
    midi: [MIDI.CC,0x03],
    group: "[Library]",
    inKey: "MoveVertical",
    input: function (channel, control, value, status, group) {
      this.inSetValue((value-0x40)*this.speed);
      // this.speed is controlled by the libraryButton
    },
  });
  CMDMM.forEachComponent(function (obj) {
    if (typeof obj.layer1 === 'function') {
        obj.layer(1);
        //calls layer1 (which is the alternative for unshift) explicitly
    }
  },true);
};
