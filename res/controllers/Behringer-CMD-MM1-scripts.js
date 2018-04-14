
//GLOBAL VARS

var channelNumber = 5;

var invertColor = true; //false=(off=orange,on=blue);true=(off=blue,on=orange);
var defaultChannelSequence = [3, 1, 2, 4];
var channelMode = [true, true, true, true]; //true=deck;false=fxChannel
var standardKnobBehavior = 0; // 0 = [High,Mid,Low,Quickeffect]; 1 = [Gain,High,Mid,Low]; 2 = [Effect1Meta,Effect2Meta,Effect3Meta,mix];


CMDMM = {}; //controller itself
MIDI = {}; //midi related constants
CALLBACK = {}; //Every callback function
FUNCTIONS = {}; //misc for assigning controls


//takes functions as arguments and executes them based on the current state of shift and control;
CMDMM.modes = function(normal, shift, ctrl, thirdLevel) {
  switch (CMDMM.getLevel()) {
    case 0:
      if (normal !== undefined) {
        normal();
      }
      break;
    case 1:
      if (shift !== undefined) {
        shift();
      }
      break;
    case 2:
      if (ctrl !== undefined) {
        ctrl();
      }
      break;
    case 3:
      if (thirdLevel !== undefined) {
        thirdLevel();
      }
      break;
    default:
      print("invalid Level: " + CMDMM.getLevel());
      break;
  }
};



MIDI.noteOn = 0x90 + (channelNumber - 1);
MIDI.noteOff = 0x80 + (channelNumber - 1);
MIDI.CC = 0xB0 + (channelNumber - 1);

//0x00 = orange; 0x01 = blue;
CMDMM.on = (invertColor ? 0x00 : 0x01);
CMDMM.off = (invertColor ? 0x01 : 0x00);
CMDMM.blink = 0x02;


CMDMM.buttons = [0x12, 0x13, 0x14, 0x17, 0x18, 0x1B, 0x1C, 0x1F, 0x20, 0x30, 0x31, 0x32, 0x33, ];
//               ^    ^Fx1 ^Fx2 ^Fx1 ^Fx2 ^Fx1 ^Fx2 ^Fx1 ^Fx2 ^CUE1^CUE2^CUE3^CUE4
//               |    ^Channel1 ^Channel2 ^Channel3 ^Channel4
//               ^middlebutton

CMDMM.varStorage = {
  level: 0, //stores the current level (shift/ctrl/thirdlevel)
  knobAssignment: standardKnobBehavior,
  channelSequence: defaultChannelSequence,
  channelKind: channelMode, //true=deck,false=fxUnit
  ChannelAssignmentStatus: [
    [0, 0],
    [0, 0],
    [0, 0],
    [0, 0]
  ],
};

CMDMM.getShift = function() {
  return CMDMM.varStorage.level === 1;
};
CMDMM.shift = function() {
  CMDMM.varStorage.level++;
  CMDMM.updateLEDs();
  FUNCTIONS.ignoreNextValue();
};
CMDMM.unshift = function() {
  CMDMM.varStorage.level--;
  CMDMM.updateLEDs();
  FUNCTIONS.ignoreNextValue();
};

CMDMM.getCtrl = function() {
  return CMDMM.varStorage.level === 2;
};
CMDMM.ctrl = function() {
  CMDMM.varStorage.level += 2;
  CMDMM.updateLEDs();
  FUNCTIONS.ignoreNextValue();
};
CMDMM.unctrl = function() {
  CMDMM.varStorage.level -= 2;
  CMDMM.updateLEDs();
  FUNCTIONS.ignoreNextValue();
};

CMDMM.getThird = function() {
  return (CMDMM.varStorage.level == 3 ? true : false);
};
CMDMM.getLevel = function() {
  return CMDMM.varStorage.level;
};


CMDMM.cue = function(channel, control, value, status, group) {
  var cueChannel = CMDMM.varStorage.channelSequence[control - 0x30];
  if (CMDMM.varStorage.channelKind[control - 0x30]) {
    CMDMM.modes(
      function() {
        script.toggleControl("[Channel" + cueChannel + "]", "pfl");
      },
      function() {
        //load track into selected channel
        engine.setParameter("[Channel" + cueChannel + "]", "LoadSelectedTrack", 1);
      },
      function() {
        /*
        				//enable pfl for EffectN N = number of channel (a bit weird, but it works)
        				script.toggleControl("[EffectRack1_EffectUnit"+cueChannel+"]", "group_[Headphone]_enable");
        				//OBSOLETE
        			*/
      },
      function() {
        CMDMM.varStorage.channelKind[control - 0x30] = !CMDMM.varStorage.channelKind[control - 0x30];
        CALLBACK.cue();
        CALLBACK.fxButton();
      });
  } else {
    CMDMM.modes(
      function() {
        script.toggleControl("[EffectRack1_EffectUnit" + cueChannel + "_Effect3]", "enabled");
      },
      function() {
        script.toggleControl("[EffectRack1_EffectUnit" + cueChannel + "]", "group_[Headphone]_enable");
      },
      function() {
        script.toggleControl("[EffectRack1_EffectUnit" + cueChannel + "]", "group_[Master]_enable");
      },
      function() {
        CMDMM.varStorage.channelKind[control - 0x30] = !CMDMM.varStorage.channelKind[control - 0x30];
        CALLBACK.cue();
        CALLBACK.fxButton();
      }
    );
  }
};

CMDMM.fxButton = function(channel, control, value, status, group) {
  var button = CMDMM.buttons.indexOf(control); //returns integers from 1 to 8
  var realChannel = Math.floor((button - 1) / 2); //realchannelNumbers (0to3)
  var mixxxChannel = CMDMM.varStorage.channelSequence[realChannel];
  if (CMDMM.varStorage.channelKind[realChannel]) {
    CMDMM.modes(
      function() {
        var channelButton = button % 2 === 0 ? 2 : 0;
        if (engine.getValue("[Channel" + mixxxChannel + "]", "orientation") === channelButton) {
          engine.setValue("[Channel" + mixxxChannel + "]", "orientation", 1);
        } else {
          engine.setValue("[Channel" + mixxxChannel + "]", "orientation", channelButton); //checks if buttonnumber is even, if true, return Left if false, return right;
        }

      },
      function() {
        var effectUnit = button % 2 === 0 ? 2 : 1; //checks if buttonnumber is even, if true, return effectUnit 2 if false, return effectUnit 1;
        // maps button to range [0;7], divides and floors it (result: [0;3]). That value is being used to lookup the right channel.
        script.toggleControl("[EffectRack1_EffectUnit" + effectUnit + "]", "group_[Channel" + mixxxChannel + "]_enable");
      },
      function() {
        var effectUnit = button % 2 === 0 ? 4 : 3; //checks if buttonnumber is even, if true, return effectUnit 4 if false, return effectUnit 3;
        script.toggleControl("[EffectRack1_EffectUnit" + effectUnit + "]", "group_[Channel" + mixxxChannel + "]_enable");
      },
      function() {
        var left_right = button % 2 === 0 ? 2 : 1;
        CMDMM.varStorage.ChannelAssignmentStatus[realChannel][left_right - 1] = CMDMM.varStorage.ChannelAssignmentStatus[realChannel][left_right - 1] === 0 ? 1 : 0;
        //basically inverting the one bit ^^
        CMDMM.varStorage.channelSequence[realChannel] = CMDMM.varStorage.ChannelAssignmentStatus[realChannel][0] + CMDMM.varStorage.ChannelAssignmentStatus[realChannel][1] * 2 + 1;
        CALLBACK.fxButton();
        /*
        	var buttonSet = CMDMM.varStorage.ChannelAssignmentStatus[realChannel];
        	var whichButton = button%2===0 ? 1:0;
        	var isActive = buttonSet[whichButton];
        	var value = whichButton*(isActive?-1:1);
        	CMDMM.varStorage.channelSequence[realChannel] +=value;
        	CMDMM.varStorage.ChannelAssignmentStatus[realChannel][whichButton] = !isActive;
        	CALLBACK.fxButton();*/
      }
    );
  } else {
    CMDMM.modes(
      function() {
        var effectUnit = button % 2 === 0 ? 2 : 1;
        script.toggleControl("[EffectRack1_EffectUnit" + mixxxChannel + "_Effect" + effectUnit + "]", "enabled");
      },
      function() {
        var effectUnit = button % 2 === 0 ? 2 : 1; //checks if buttonnumber is even, if true, return effectUnit 2 if false, return effectUnit 1;
        // maps button to range [0;7], divides and floors it (result: [0;3]). That value is being used to lookup the right channel.
        script.toggleControl("[EffectRack1_EffectUnit" + mixxxChannel + "]", "group_[Channel" + effectUnit + "]_enable");
      },
      function() {
        var effectUnit = button % 2 === 0 ? 4 : 3; //checks if buttonnumber is even, if true, return effectUnit 4 if false, return effectUnit 3;
        // maps button to range [0;7], divides and floors it (result: [0;3]). That value is being used to lookup the right channel.
        script.toggleControl("[EffectRack1_EffectUnit" + mixxxChannel + "]", "group_[Channel" + effectUnit + "]_enable");
      },
      function() {
        var left_right = button % 2 === 0 ? 2 : 1;
        CMDMM.varStorage.ChannelAssignmentStatus[realChannel][left_right - 1] = CMDMM.varStorage.ChannelAssignmentStatus[realChannel][left_right - 1] === 0 ? 1 : 0;
        //basically inverting the one bit ^^
        CMDMM.varStorage.channelSequence[realChannel] = CMDMM.varStorage.ChannelAssignmentStatus[realChannel][0] + CMDMM.varStorage.ChannelAssignmentStatus[realChannel][1] * 2 + 1;
        CALLBACK.fxButton();
      }
    );
  }
};
FUNCTIONS.assignKnobsWithQuickEffect = function() {
  CMDMM.knob = function(channel, control, value, status, group) {
    var realChannel = (control - 0x06) % 4;
    var mixxxChannel = CMDMM.varStorage.channelSequence[realChannel];
    if (CMDMM.varStorage.channelKind[realChannel]) {
      var parameterNum = -(Math.floor((control - 0x06) / 4) - 3); //get the horizontal row of the button and translates it to the number
      var parameter = (parameterNum === 0 ? "super1" : ("parameter" + parameterNum));
      var mixxxGroup = (parameterNum === 0) ? ("[QuickEffectRack1_[Channel" + mixxxChannel + "]]") : "[EqualizerRack1_[Channel" + mixxxChannel + "]_Effect1]";
      engine.setParameter(mixxxGroup, parameter, value / 127);
    } else {
      var parameterNum = Math.floor((control - 0x06) / 4) + 1;
      var parameter = (parameterNum === 4) ? "super1" : "meta";
      var mixxxGroup = ("[EffectRack1_EffectUnit" + mixxxChannel + ((parameterNum === 4) ? "" : ("_Effect" + parameterNum)) + "]");
      engine.setParameter(mixxxGroup, parameter, value / 127);
    }
  };
};
FUNCTIONS.assignKnobsWithGain = function() {
  CMDMM.knob = function(channel, control, value, status, group) {
    var realChannel = (control - 0x06) % 4;
    var mixxxChannel = CMDMM.varStorage.channelSequence[realChannel];
    if (CMDMM.varStorage.channelKind[realChannel]) {
      var parameterNum = -(Math.floor((control - 0x06) / 4) - 2) + 2; //get the horizontal row of the button and translates it to the number
      var parameter = (parameterNum === 4 ? "pregain" : ("parameter" + parameterNum));
      var mixxxGroup = (parameterNum === 4) ? ("[Channel" + mixxxChannel + "]") : "[EqualizerRack1_[Channel" + mixxxChannel + "]_Effect1]";
      engine.setParameter(mixxxGroup, parameter, value / 127);
    } else {
      var parameterNum = Math.floor((control - 0x06) / 4) + 1;
      var parameter = (parameterNum === 4) ? "super1" : "meta";
      var mixxxGroup = ("[EffectRack1_EffectUnit" + mixxxChannel + ((parameterNum === 4) ? "" : ("_Effect" + parameterNum)) + "]");
      engine.setParameter(mixxxGroup, parameter, value / 127);
    }
  };
};
FUNCTIONS.assignKnobsEffectOnly = function() {
  CMDMM.knob = function(channel, control, value, status, group) {
    var realChannel = (control - 0x06) % 4;
    var mixxxChannel = CMDMM.varStorage.channelSequence[realChannel];
    var parameterNum = Math.floor((control - 0x06) / 4) + 1;
    var parameter = (parameterNum === 4) ? "mix" : "meta";
    var mixxxGroup = ("[EffectRack1_EffectUnit" + mixxxChannel + ((parameterNum === 4) ? "" : ("_Effect" + parameterNum)) + "]");
    engine.setParameter(mixxxGroup, parameter, value / 127);
  };
};
CMDMM.updateLEDs = function() {
  //could be better implemented, but its easy and it works.
  CALLBACK.middleButton();
  CALLBACK.fxButton();
  CALLBACK.cue();
};
FUNCTIONS.ignoreNextValue = function() {
  for (var i = 1; i <= 4; i++) {
    engine.softTakeoverIgnoreNextValue("[QuickEffectRack1_[Channel" + i + "]]", "super1", true);
    engine.softTakeoverIgnoreNextValue("[Channel" + i + "]", "rate", true);
    engine.softTakeoverIgnoreNextValue("[Channel" + i + "]", "volume", true);
    engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit" + i + "]", "mix", true);
    engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit" + i + "]", "super1", true);
    for (var ii = 1; ii <= 3; ii++) {
      engine.softTakeoverIgnoreNextValue("[EqualizerRack1_[Channel" + i + "]_Effect1]", "parameter" + ii, true);
      engine.softTakeoverIgnoreNextValue("[EffectRack1_EffectUnit" + i + "_Effect" + ii + "]", "meta", true);
    }
  }
};

FUNCTIONS.cycleKnobAssignment = function() {
  // 0 = [High,Mid,Low,Quickeffect]; 1 = [Gain,High,Mid,Low];
  switch (CMDMM.varStorage.knobAssignment) {
    case 2:
      CMDMM.varStorage.knobAssignment = 0;
      FUNCTIONS.assignKnobsEffectOnly();
      break;
    case 1:
      CMDMM.varStorage.knobAssignment++;
      FUNCTIONS.assignKnobsWithGain();
      break;
    case 0:
      CMDMM.varStorage.knobAssignment++;
      FUNCTIONS.assignKnobsWithQuickEffect();
      break;
    default:
      print("INVALID KNOB ASSIGNMENT (" + CMDMM.varStorage.knobAssignment + ")!");
      break;
  }
};

CMDMM.fader = function(channel, control, value, status, group) {
  if (CMDMM.varStorage.channelKind[control - 0x30]) {
    CMDMM.modes(function() {
        engine.setParameter("[Channel" + CMDMM.varStorage.channelSequence[control - 0x30] + "]", "volume", value / 127);
      },
      function() {
        engine.setParameter("[Channel" + CMDMM.varStorage.channelSequence[control - 0x30] + "]", "rate", value / 127);
      },
      function() {
        //empty so fadermovement can be ignored while pressing ctrl
      }
    );
  } else {
    CMDMM.modes(
      function() {
        engine.setParameter("[EffectRack1_EffectUnit" + CMDMM.varStorage.channelSequence[(control - 0x30)] + "]", "mix", value / 127);
      }
    );
  }
};

CMDMM.out1 = function(channel, control, value, status, group) {
  engine.setParameter("[Master]", "balance", value / 127);
};
CMDMM.out2 = function(channel, control, value, status, group) {
  engine.setParameter("[Master]", "gain", value / 127);
};

FUNCTIONS.resetColor = function() {
  for (var i = CMDMM.buttons.length - 1; i >= 0; i--) {
    midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[i], CMDMM.off);
  }
};

CMDMM.libraryEncoder = function(channel, control, value, status, group) {
  CMDMM.modes(
    function() {
      //print("using deprecated SelectTrackKnob as long there is no way to select from just the tracks!")
      engine.setValue("[Playlist]", "SelectTrackKnob", value > 0x40 ? 1 : -1);
    },
    function() {
      //navigate preview track
      engine.setParameter("[PreviewDeck1]", "beatjump", (value > 0x40) ? 16 : -16);
    },
    function() {

    }
  );
};
CMDMM.libraryButton = function(channel, control, value, status, group) {
  CMDMM.modes(
    function() {
      engine.setParameter("[PreviewDeck1]", "LoadSelectedTrack", 1);
    },
    function() {
      engine.setParameter("[PreviewDeck1]", "play", 1);
    }
  );
};

CMDMM.middleButton = function(channel, control, value, status, group) {
  CMDMM.modes(function() {
      script.toggleControl("[Master]", "maximize_library");
      //engine.setValue("[Master]","maximize_library", 1);
      //maximize Library
    },
    function() {
      engine.setValue("[Master]", "crossfader", 0);
    },
    function() {
      FUNCTIONS.cycleKnobAssignment();
      CALLBACK.middleButton();
    },
    function() {
      CMDMM.varStorage.channelSequence = defaultChannelSequence;
      CMDMM.updateLEDs();
    }
  );
};

CALLBACK.vuMeterL = function(value, group, control) {
  midi.sendShortMsg(MIDI.CC, 80, (value * 15) + 48);
};
CALLBACK.vuMeterR = function(value, group, control) {
  midi.sendShortMsg(MIDI.CC, 81, (value * 15) + 48);
};


CALLBACK.registerCallbacks = function() {
  //VuMeters
  engine.makeConnection("[Master]", "VuMeterL", CALLBACK.vuMeterL);
  engine.makeConnection("[Master]", "VuMeterR", CALLBACK.vuMeterR);
  //engine.makeConnection("[Master]","maximize_library",CALLBACK.middleButton);// doesnt work, help?
  //cueButtons (pfl)
  for (var i = 1; i <= 4; i++) {
    engine.makeConnection("[Channel" + i + "]", "pfl", CALLBACK.cue);
    engine.makeConnection("[Channel" + i + "]", "play", CALLBACK.cue);
    engine.makeConnection("[Channel" + i + "]", "orientation", CALLBACK.fxButton);
    for (var ii = 1; ii <= 4; ii++) {
      engine.makeConnection("[EffectRack1_EffectUnit" + i + "]", "group_[Channel" + ii + "]_enable", CALLBACK.fxButton);
    }
    for (var ii = 1; ii <= 2; ii++) {
      engine.makeConnection("[EffectRack1_EffectUnit" + i + "_Effect" + ii + "]", "enabled", CALLBACK.fxButton);
    }
    engine.makeConnection("[EffectRack1_EffectUnit" + i + "_Effect3]", "enabled", CALLBACK.cue);
    engine.makeConnection("[EffectRack1_EffectUnit" + i + "]", "group_[Headphone]_enable", CALLBACK.cue);
    engine.makeConnection("[EffectRack1_EffectUnit" + i + "]", "group_[Master]_enable", CALLBACK.cue);
  }
};
CALLBACK.middleButton = function() {
  switch (CMDMM.getLevel()) {
    case 0:
      //midi.sendShortMsg(MIDI.noteOn,CMDMM.buttons[0],engine.getParameter("[Master]","maximize_library")?CMDMM.on:CMDMM.off); break;
      midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[0], CMDMM.off);
      break;
    case 1:
      midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[0], engine.getValue("[Master]", "crossfader") ? CMDMM.on : CMDMM.off);
      break;
    case 2:
      midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[0], CMDMM.varStorage.knobAssignment);
      break; //[High,Mid,Low,Quickeffect]=orange, [Gain,High,Mid,Low] = blue;
    case 3:
      //midi.sendShortMsg(MIDI.noteOn,CMDMM.buttons[0],CMDMM.varStorage.channelSequence===[3,1,2,4]?0x01:0x00); break; //[3,1,2,4]ChannelAssignment = blue; [1,2,3,4] = orange;
  }
};
CALLBACK.fxButton = function() {
  for (var channel = 1; channel <= 4; channel++) {
    if (CMDMM.varStorage.channelKind[channel - 1]) {
      switch (CMDMM.getLevel()) {
        case 0:
          var orientationButton = engine.getValue("[Channel" + CMDMM.varStorage.channelSequence[channel - 1] + "]", "orientation");
          midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[(channel) * 2 - 1], orientationButton === 0 ? CMDMM.on : CMDMM.off);
          midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[(channel) * 2 - 0], orientationButton === 2 ? CMDMM.on : CMDMM.off);
          break;
        case 1:
          for (var fxUnit = 1; fxUnit <= 2; fxUnit++) {
            var effectUnitValue = engine.getValue("[EffectRack1_EffectUnit" + fxUnit + "]", "group_[Channel" + CMDMM.varStorage.channelSequence[channel - 1] + "]_enable");
            midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[(channel - 1) * 2 + fxUnit], effectUnitValue ? CMDMM.on : CMDMM.off);
            //                                                 ^strange but working solution to get values from 1 to 8
          }
          break;
        case 2:
          for (var fxUnit = 3; fxUnit <= 4; fxUnit++) {
            var effectUnitValue = engine.getValue("[EffectRack1_EffectUnit" + fxUnit + "]", "group_[Channel" + CMDMM.varStorage.channelSequence[channel - 1] + "]_enable");
            midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[(channel - 1) * 2 + fxUnit - 2], effectUnitValue ? CMDMM.on : CMDMM.off);
            //                                                 ^strange but working solution to get values from 1 to 8
          }
          break;
        case 3:
          for (var i = 1; i <= 2; i++) {
            midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[(channel - 1) * 2 + i], CMDMM.varStorage.ChannelAssignmentStatus[channel - 1][i - 1] ? CMDMM.on : CMDMM.off);
          }
          break;
      }
    } else {
      switch (CMDMM.getLevel()) {
        case 0:
          for (var fxUnit = 1; fxUnit <= 2; fxUnit++) {
            var effectUnitValue = engine.getValue("[EffectRack1_EffectUnit" + CMDMM.varStorage.channelSequence[channel - 1] + "]", "group_[Channel" + fxUnit + "]_enable");
            midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[(channel - 1) * 2 + fxUnit], effectUnitValue ? CMDMM.on : CMDMM.off);
            //                                                 ^strange but working solution to get values from 1 to 8
          }
          break;
        case 1:
          for (var fxUnit = 3; fxUnit <= 4; fxUnit++) {
            var effectUnitValue = engine.getValue("[EffectRack1_EffectUnit" + CMDMM.varStorage.channelSequence[channel - 1] + "]", "group_[Channel" + fxUnit + "]_enable");
            midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[(channel - 1) * 2 + fxUnit - 2], effectUnitValue ? CMDMM.on : CMDMM.off);
            //                                                 ^strange but working solution to get values from 1 to 8
          }
          break;
        case 2:
          for (var fxUnit = 1; fxUnit <= 2; fxUnit++) {
            var value = engine.getValue("[EffectRack1_EffectUnit" + CMDMM.varStorage.channelSequence[channel - 1] + "_Effect" + fxUnit + "]", "enabled");
            midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[(channel - 1) * 2 + fxUnit], value ? CMDMM.on : CMDMM.off);
            //                                                 ^strange but working solution to get values from 1 to 8
          }
          break;
        case 3:
          for (var i = 1; i <= 2; i++) {
            midi.sendShortMsg(MIDI.noteOn, CMDMM.buttons[(channel - 1) * 2 + i], CMDMM.varStorage.ChannelAssignmentStatus[channel - 1][i - 1] ? CMDMM.on : CMDMM.off);
          }
          break;
      }
    }
  }
};
CALLBACK.cue = function() {
  var value = 0;
  for (var channel = 1; channel <= 4; channel++) {
    var mixxxChannel = CMDMM.varStorage.channelSequence[channel - 1];
    if (CMDMM.varStorage.channelKind[channel - 1]) {
      switch (CMDMM.getLevel()) {
        case 0:
          value = engine.getValue("[Channel" + mixxxChannel + "]", "pfl") ? CMDMM.on : CMDMM.off;
          break;
        case 1:
          value = engine.getValue("[Channel" + mixxxChannel + "]", "play") ? CMDMM.on : CMDMM.off;
          break;
        case 2:
          value = engine.getValue("[EffectRack1_EffectUnit" + mixxxChannel + "]", "group_[Headphone]_enable") ? CMDMM.on : CMDMM.off;
          break;
        case 3:
          value = CMDMM.off;
          /*
          	value = engine.getValue("[Channel"+CMDMM.varStorage.channelSequence[channel-1]+"]","rate")?CMDMM.on:CMDMM.off;
          	break;
          */
      }
    } else {
      switch (CMDMM.getLevel()) {
        case 0:
          value = engine.getValue("[EffectRack1_EffectUnit" + mixxxChannel + "]", "group_[Headphone]_enable") ? CMDMM.on : CMDMM.off;
          break;
        case 1:
          value = engine.getValue("[EffectRack1_EffectUnit" + mixxxChannel + "]", "group_[Master]_enable") ? CMDMM.on : CMDMM.off;
          break;
        case 2:
          value = engine.getValue("[EffectRack1_EffectUnit" + mixxxChannel + "_Effect3]", "enabled") ? CMDMM.on : CMDMM.off;
          break;
        case 3:
          value = CMDMM.on;
      }
    }
    midi.sendShortMsg(MIDI.noteOn, channel + 0x2F, value);
  }
};
FUNCTIONS.buttonFromChannelNumber = function() {
  for (var i = 0; i < 4; i++) {
    CMDMM.varStorage.ChannelAssignmentStatus[i][0] = (CMDMM.varStorage.channelSequence[i] === 2 || CMDMM.varStorage.channelSequence[i] === 4) ? 1 : 0;
    CMDMM.varStorage.ChannelAssignmentStatus[i][1] = CMDMM.varStorage.channelSequence[i] >= 3 ? 1 : 0;
  }
};
FUNCTIONS.enableSoftTakeover = function() {
  for (var i = 1; i <= 4; i++) {
    engine.softTakeover("[QuickEffectRack1_[Channel" + i + "]]", "super1", true);
    //engine.softTakeover("[Channel"+i+"]","rate",true);
    engine.softTakeover("[Channel" + i + "]", "volume", true);
    engine.softTakeover("[EffectRack1_EffectUnit" + i + "]", "mix", true);
    engine.softTakeover("[EffectRack1_EffectUnit" + i + "]", "super1", true);
    for (var ii = 1; ii <= 3; ii++) {
      engine.softTakeover("[EqualizerRack1_[Channel" + i + "]_Effect1]", "parameter" + ii, true);
      engine.softTakeover("[EffectRack1_EffectUnit" + i + "_Effect" + ii + "]", "meta", true);
    }
  }
};

CMDMM.init = function() {
  FUNCTIONS.resetColor();
  FUNCTIONS.buttonFromChannelNumber();
  FUNCTIONS.cycleKnobAssignment();
  FUNCTIONS.enableSoftTakeover();
  CALLBACK.registerCallbacks();
  CMDMM.updateLEDs();
};


CMDMM.shutdown = function() {
  //reset Button LEDs
  FUNCTIONS.resetColor();
};
