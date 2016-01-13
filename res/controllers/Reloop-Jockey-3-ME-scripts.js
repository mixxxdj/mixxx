function Jockey3ME() {}

// Variables
Jockey3ME.EffectLedMeter = 0;
Jockey3ME.EffectLedMeterValue = 1;
Jockey3ME.LedMeterShowValue = 1;
Jockey3ME.LedMeterShowValueTwo = false;
Jockey3ME.VuMeter = 0;
Jockey3ME.scratching = [];
Jockey3ME.hotcueClearVal = 0;
Jockey3ME.crossfaderScratch = false;
Jockey3ME.effectSelectTimer = 0;
Jockey3ME.num_effectsValue = [0,0,0,0];
Jockey3ME.effectsAvailable = 5; // Sets how many Effects are Loadable
Jockey3ME.move_beat_value = 4; // Sets how many Beats Jumping when "MOVE" is Turned
Jockey3ME.CUP_value = 0;
Jockey3ME.MixerDeck1 = 0;
Jockey3ME.MixerDeck2 = 0;
Jockey3ME.noVolHopValue = false;

// Functions
Jockey3ME.EffectLedMeterShow = function () {
  midi.sendShortMsg(0x90,0x1D,Jockey3ME.EffectLedMeterValue);
  midi.sendShortMsg(0x91,0x1D,Jockey3ME.EffectLedMeterValue);
  Jockey3ME.EffectLedMeterValue += 2;
  if (Jockey3ME.EffectLedMeterValue >= 127) {
    engine.stopTimer(Jockey3ME.EffectLedMeter);
    Jockey3ME.EffectLedMeter = 0;

    // Sets Effect Leds
    for (var i = 1, j = 176; i <= 4; i++) {
      Jockey3ME.effectSelectLedSet(j,i);
      j++;
    }
  };
}
// Main LedShow Script
Jockey3ME.LedMeterShow = function() {
  midi.sendShortMsg(0x90,0x21,Jockey3ME.LedMeterShowValue);
  midi.sendShortMsg(0x91,0x21,Jockey3ME.LedMeterShowValue);
  if (Jockey3ME.LedMeterShowValueTwo == false) {
    ++Jockey3ME.LedMeterShowValue;
  } else {
    --Jockey3ME.LedMeterShowValue;
  }
  if (Jockey3ME.LedMeterShowValue > 10) Jockey3ME.LedMeterShowValueTwo = true;
  // Stop The LedShow and Start Scanning VuMeter
  if (Jockey3ME.LedMeterShowValueTwo && Jockey3ME.LedMeterShowValue <= 0) {
    engine.stopTimer(Jockey3ME.LedMeterShowTimer);
    Jockey3ME.LedMeterShowTimer = 0;
    Jockey3ME.VuMeter = engine.beginTimer(20,"Jockey3ME.fVuMeter()"); // Start Every 20ms the fVuMeter Function
    Jockey3ME.EffectLedMeter = engine.beginTimer(20,"Jockey3ME.EffectLedMeterShow()");
  };
}

Jockey3ME.LedShowBegin = function () {
  Jockey3ME.LedMeterShowTimer = engine.beginTimer(40,"Jockey3ME.LedMeterShow()");
}

// Init Script at Programmstart
Jockey3ME.init = function () {
  for (var i = 1; i < 120; i++) {
    midi.sendShortMsg(0x90,i,0x7F);
    midi.sendShortMsg(0x91,i,0x7F);
    midi.sendShortMsg(0x92,i,0x7F);
    midi.sendShortMsg(0x93,i,0x7F);
  };
  for (var j = 1; j < 120; j++) {
    midi.sendShortMsg(0x90,j,0x00);
    midi.sendShortMsg(0x91,j,0x00);
    midi.sendShortMsg(0x92,j,0x00);
    midi.sendShortMsg(0x93,j,0x00);
  };
  Jockey3ME.LedShowBeginTimer = engine.beginTimer(500,"Jockey3ME.LedShowBegin()",1); // LedShow Script Starts Here after 500ms
}

// Sets VuMeter new Value to Leds
Jockey3ME.fVuMeter = function () {
  var VuVal1 = engine.getValue("[Channel1]","VuMeter");
  var VuVal2 = engine.getValue("[Channel2]","VuMeter");
  var VuVal3 = engine.getValue("[Channel3]","VuMeter");
  var VuVal4 = engine.getValue("[Channel4]","VuMeter");
  VuVal1 = VuVal1 * 10;
  VuVal1 = parseInt(VuVal1);
  VuVal2 = VuVal2 * 10;
  VuVal2 = parseInt(VuVal2);
  VuVal3 = VuVal3 * 10;
  VuVal3 = parseInt(VuVal3);
  VuVal4 = VuVal4 * 10;
  VuVal4 = parseInt(VuVal4);
  midi.sendShortMsg(0x90,0x21,VuVal1);
  midi.sendShortMsg(0x91,0x21,VuVal2);
  midi.sendShortMsg(0x92,0x21,VuVal3);
  midi.sendShortMsg(0x93,0x21,VuVal4);
}

Jockey3ME.shutdown = function () {
  for (var i = 1; i <= 160; i++) {
    midi.sendShortMsg(0x90,i,0x00);
    midi.sendShortMsg(0x91,i,0x00);
    midi.sendShortMsg(0x92,i,0x00);
    midi.sendShortMsg(0x93,i,0x00);
  };
  engine.stopTimer(Jockey3ME.VuMeter);
  Jockey3ME.VuMeter = 0;
}

// The button that enables/disables scratching
Jockey3ME.wheelTouch = function (channel, control, value, status, group) {
  var currentDeck = parseInt(group.substring(8,9));
    if (value == 0x7F) {  // Some wheels send 0x90 on press and release, so you need to check the value
        var alpha = 1.0/8;
        var beta = alpha/32;
        engine.scratchEnable(currentDeck, 2048, 45+1/3, alpha, beta);
    }
    else {    // If button up
        engine.scratchDisable(currentDeck);
    }
}

// The wheel that actually controls the scratching
Jockey3ME.wheelTurn = function (channel, control, value, status, group) {
  var newValue=(value-64);
  var currentDeck = parseInt(group.substring(8,9));
    // See if we're scratching. If not, skip this.
    if (!engine.isScratching(currentDeck)) {
      if (newValue > 1 || newValue < -1) {
        newValue /= 2;  // If Jogwheel Resolution is High
      }
      engine.setValue(group, "jog", newValue);
      return;
   }
    engine.scratchTick(currentDeck,newValue);
}

// Hotcues
Jockey3ME.hotcue_activate = function (channel, control, value, status, group) {
  Jockey3ME.hotc = Jockey3ME.hotcue_buttonSelect(control);

  if (!Jockey3ME.hotcueClearVal && engine.getValue(group,"hotcue_"+Jockey3ME.hotc+"_enabled") == 0 && value == 0x7F) {
    engine.setValue(group,"hotcue_"+Jockey3ME.hotc+"_activate",1);
    engine.setValue(group,"hotcue_"+Jockey3ME.hotc+"_activate",0);
  } else if (Jockey3ME.hotcueClearVal && engine.getValue(group,"hotcue_"+Jockey3ME.hotc+"_enabled") == 1 && value == 0x7F) {
    engine.setValue(group,"hotcue_"+Jockey3ME.hotc+"_clear",1);
    engine.setValue(group,"hotcue_"+Jockey3ME.hotc+"_clear",0);
  } else if (value == 0x7F) {
    engine.setValue(group,"hotcue_"+Jockey3ME.hotc+"_activate",1);
    engine.setValue(group,"hotcue_"+Jockey3ME.hotc+"_activate",0);
  }
}

Jockey3ME.hotcue_buttonSelect = function (control) {
  Jockey3ME.hotc_midino = [11,12,13,14,15,16,17,18];
  for (var i = 0; i < Jockey3ME.hotc_midino.length; i++) {
    switch (control) {
      case Jockey3ME.hotc_midino[i]:
        return Jockey3ME.hotc_midino[i] - 10;
        break;
    }
  };
}

Jockey3ME.hotcueClearVal_off = function() {
  if (Jockey3ME.hotcueClearVal) {
    Jockey3ME.hotcueClearVal = 0;
  };
}

Jockey3ME.hotcue_clear = function (channel, control, value, status, group) {
   if (control == 0x09 && value == 0x7F) {
    Jockey3ME.hotcueClearVal_off();
    Jockey3ME.hotcueClearVal = 1;
    midi.sendShortMsg(status,control,0x01);
   } else {
    Jockey3ME.hotcueClearVal_off();
    midi.sendShortMsg(status,control,0x00);
   };
}

// Effect Sections
Jockey3ME.effectParam = function (channel, control, value, status, group) {
  var currentDeck = parseInt(group.substring(23,24));
  var EncoderKnopDryWet = 0;
  var EncoderKnopFX = 0;
  var newVal = 0;
  var interval = 0.04;
  var min = 0;
  var max = 1;
  switch (control) {
    case 29:
      EncoderKnopDryWet = 1;
      break;
    default:
      EncoderKnopFX = control - 29;
      break;
  }
  if (!EncoderKnopDryWet) {
    if (value == 0x41) {
      var curVal = engine.getParameter("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]", "parameter" + EncoderKnopFX);
      newVal = curVal + interval;
      if (newVal > max) newVal = max;
    } else {
      var curVal = engine.getParameter("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]", "parameter" + EncoderKnopFX);
      newVal = curVal - interval;
      if (newVal < min) newVal = min;
    }
  } else {
    if (value == 0x41) {
      var curVal = engine.getParameter("[EffectRack1_EffectUnit" + currentDeck + "]", "mix");
      newVal = curVal + interval;
      if (newVal > max) newVal = max;
    } else {
      var curVal = engine.getParameter("[EffectRack1_EffectUnit" + currentDeck + "]", "mix");
      newVal = curVal - interval;
      if (newVal < min) newVal = min;
    }
  }
  switch (engine.getValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]", "num_parameters")) {
    case 1:
      if (EncoderKnopFX > 1) EncoderKnopFX = 0;
      break;
    case 2:
      if (EncoderKnopFX > 2) EncoderKnopFX = 0;
      break;
  }
  if (EncoderKnopDryWet) {
    engine.setParameter("[EffectRack1_EffectUnit" + currentDeck + "]", "mix", newVal);
  } else if (EncoderKnopFX) {
    engine.setParameter("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]", "parameter" + EncoderKnopFX, newVal);
  };

  // Leds
  status = Jockey3ME.effectUnitValueLed(status);
  if (EncoderKnopFX) {
    Jockey3ME.effectParamLedSet(currentDeck, EncoderKnopFX, status, control);
  } else if (EncoderKnopDryWet) {
    Jockey3ME.effectMixLedSet(currentDeck,status,control);
  };
}

Jockey3ME.effectParamLedSet = function (currentDeck, index, status, control) {
  var ledValue = engine.getParameter("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]", "parameter" + index);
  ledValue = parseInt(ledValue * 127);
  midi.sendShortMsg(status,control,ledValue);
}

Jockey3ME.effectMixLedSet = function (currentDeck, status, control) {
  var ledValue = engine.getParameter("[EffectRack1_EffectUnit" + currentDeck + "]", "mix");
  ledValue = parseInt(ledValue * 127);
  midi.sendShortMsg(status,control,ledValue);
}

Jockey3ME.effectSelectLedSetNumEffect = function (currentDeck, status, control, value) {
  // var ledValue = engine.getValue("[EffectRack1_EffectUnit" + currentDeck + "]", "num_effects");
  var ledValue = Jockey3ME.effectsAvailable; // num_effects returns how many effects in Unit is, not what is available. Set to 5
  if ((Jockey3ME.num_effectsValue[currentDeck - 1] + value) > ledValue) {
    Jockey3ME.num_effectsValue[currentDeck - 1] = 1;
  } else {
    Jockey3ME.num_effectsValue[currentDeck - 1] += value;
  }
  var newLedValue = parseInt((Jockey3ME.num_effectsValue[currentDeck - 1] / ledValue) * 127);
  midi.sendShortMsg(Jockey3ME.effectUnitValueLed(status),control,newLedValue);
}

Jockey3ME.effectSelect = function (channel, control, value, status, group) {
  var currentDeck = parseInt(group.substring(23,24));
  engine.setValue("[EffectRack1_EffectUnit" + currentDeck + "]", "chain_selector", (value-64));

  // Set Leds
  Jockey3ME.effectSelectTimer = engine.beginTimer(100, "Jockey3ME.effectSelectLedSet(" + status + "," + currentDeck + ")",1);
  Jockey3ME.effectSelectLedSetNumEffect(currentDeck,status,92,(value-64));
}

Jockey3ME.effectSelectLedSet = function (status, currentDeck) {
  status = Jockey3ME.effectUnitValueLed(status);
  var num_parameters = engine.getValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]", "num_parameters");
  if (num_parameters > 3) {num_parameters = 3;};
  if (num_parameters) {
    for (var i = 1, j = 30; i <= num_parameters; i++) {
      Jockey3ME.effectParamLedSet(currentDeck, i, status, j);
      j++;
    };
  }
  Jockey3ME.effectMixLedSet(currentDeck, status, 29);
  // Jockey3ME.effectSelectLedSetNumEffect(currentDeck,status,92);
}

Jockey3ME.effectUnitValueLed = function (status) {
  return status - 32;
}

// Browser Knop to Browse the Playlist
Jockey3ME.TraxEncoderTurn = function (channel, control, value, status, group) {
    var newValue = (value-64);

   engine.setValue(group,"SelectTrackKnob",newValue);

}

// Browser Knop with Shift to Browse the Playlist Tree
Jockey3ME.ShiftTraxEncoderTurn = function (channel, control, value, status, group) {
    var newValue = (value-64);

   if (newValue == 1) engine.setValue(group,"SelectNextPlaylist",newValue);
   else engine.setValue(group,"SelectPrevPlaylist",1);
}

Jockey3ME.loop_double_halve = function (channel, control, value, status, group) {
  var newValue = (value-64);

  if (newValue == 1) {
    engine.setValue(group,"loop_double",1);
    engine.setValue(group,"loop_double",0);
  } else {
    engine.setValue(group,"loop_halve",1);
    engine.setValue(group,"loop_halve",0);
  }
}

Jockey3ME.move_beat = function (channel, control, value, status, group) {
  var newValue = (value-64);
  engine.setValue(group,"beatjump",(newValue*Jockey3ME.move_beat_value));
}

Jockey3ME.crossfader = function (channel, control, value, status, group) {
  var newValue = 0;
  if (control == 0x37 && !Jockey3ME.crossfaderScratch) {
    newValue = ((value / 63.5) - 1);
    engine.setValue(group,"crossfader",newValue);
  } else if (control == 0x37 && Jockey3ME.crossfaderScratch) {
    switch (value) {
      case 127:
        engine.setValue(group,"crossfader",1);
        break;
      case 126:
        engine.setValue(group,"crossfader",0.96875);
        break;
      case 125:
        engine.setValue(group,"crossfader",0.953125);
        break;
      case 2:
        engine.setValue(group,"crossfader",-0.953125);
        break;
      case 1:
        engine.setValue(group,"crossfader",-0.96875);
        break;
      case 0:
        engine.setValue(group,"crossfader",-1);
        break;
      default:
        engine.setValue(group,"crossfader",0);
    }
  } else {
    if (value <= 126) {
      script.crossfaderCurve(value, 0, 126);
      Jockey3ME.crossfaderScratch = false;
    } else {
      Jockey3ME.crossfaderScratch = true;
    }
  }
}

Jockey3ME.CUP = function (channel, control, value, status, group) {
  if (engine.getValue(group,"play") == 1 && value == 0x7F) {
    engine.setValue(group,"cue_default",1);
    engine.setValue(group,"cue_default",0);
    engine.setValue(group,"play",0);
    midi.sendShortMsg(status,control,127);
    Jockey3ME.CUP_value = 1;
  } else if (Jockey3ME.CUP_value == 1 && value == 0) {
    engine.setValue(group,"play",1);
    midi.sendShortMsg(status,control,0);
    Jockey3ME.CUP_value = 0;
  } else {
    if (value == 0x7F) {
      engine.setValue(group,"cue_default",1);
      engine.setValue(group,"cue_default",0);
      midi.sendShortMsg(status,control,127);
    } else {
      engine.setValue(group,"play",1);
      midi.sendShortMsg(status,control,0);
    }
  }
}

Jockey3ME.MixerVol = function (channel, control, value, status, group) {
  var currentDeck = parseInt(group.substring(8,9));
  if (Jockey3ME.MixerDeck1 == 1 && currentDeck == 1) {
    currentDeck += 2;
  } else if (Jockey3ME.MixerDeck2 == 1 && currentDeck == 2) {
    currentDeck += 2;
  }
  if (control == 0x2D || control == 0x6C) {
    engine.setParameter("[Channel" + currentDeck + "]","pregain",(value / 127));
  } else {
    var noVolHop = engine.getParameter("[Channel" + currentDeck + "]","volume");
    if (!(!((noVolHop - (value / 127)) < 0.04) || !((noVolHop - (value / 127)) > -0.04))) {
      Jockey3ME.noVolHopValue = false;
    }
    if (!Jockey3ME.noVolHopValue) {
      engine.setParameter("[Channel" + currentDeck + "]","volume",(value / 127));
    }
  }
}

Jockey3ME.DeckSwitch = function (channel, control, value, status, group) {
  if (control == 0x3C && value == 0x7F) {
    Jockey3ME.MixerDeck1 = 1;
  } else if (control == 0x3C && value == 0x00) {
    Jockey3ME.MixerDeck1 = 0;
  } else if (control == 0x3F && value == 0x7F) {
    Jockey3ME.MixerDeck2 = 1;
  } else if (control == 0x3F && value == 0x00) {
    Jockey3ME.MixerDeck2 = 0;
  }
  Jockey3ME.noVolHopValue = true;
}

Jockey3ME.EQ = function (channel, control, value, status, group) {
  var currentDeck = parseInt(group.substring(24,25));
  if (Jockey3ME.MixerDeck1 == 1 && currentDeck == 1) {
    currentDeck += 2;
  } else if (Jockey3ME.MixerDeck2 == 1 && currentDeck == 2) {
    currentDeck += 2;
  }
  switch (control) {
    case 0x2E:
      var eqKnop = 3;
      break;
    case 0x2F:
      var eqKnop = 2;
      break;
    case 0x30:
      var eqKnop = 1;
      break;
    case 0x6D:
      var eqKnop = 3;
      break;
    case 0x6E:
      var eqKnop = 2;
      break;
    case 0x6F:
      var eqKnop = 1;
      break;
    default:
      print("Error on EQ chosing");
  }
  engine.setParameter("[EqualizerRack1_[Channel" + currentDeck + "]_Effect1]","parameter" + eqKnop, (value / 127));
}
