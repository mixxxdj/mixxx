function Jockey3ME() {}

// Variables
Jockey3ME.EffectLedMeter = 0;
Jockey3ME.EffectLedMeterValue = 1;
Jockey3ME.LedMeterShowValue = 1;
Jockey3ME.LedMeterShowValueTwo = false;
Jockey3ME.scratching = [];
Jockey3ME.jogwheelResolution = 2048;
Jockey3ME.jogwheelSpinSpeed = 33+1/3;
Jockey3ME.hotcueClearVal = 0;
Jockey3ME.num_effectsValue = [0,0,0,0];
Jockey3ME.effectsAvailable = 5; // Sets how many Effects are Loadable
Jockey3ME.fxSelectKnopPush = [0,0,0,0];
Jockey3ME.fxSelectKnopPushIterator1 = [0,0,0,0];
Jockey3ME.fxSelectKnopPushIterator2 = [0,0,0,0];
Jockey3ME.fxSelectKnopPushIterator3 = [0,0,0,0];
Jockey3ME.fxSelectKnopPushLedTemp = 0;
Jockey3ME.fxSelectKnopParamChose = 0;
Jockey3ME.fxSelectKnopParamLinkChose = 0;
Jockey3ME.fxSelectKnopParamLinkInverseChose = 0;
Jockey3ME.loop_move_value = 4; // Sets how many a Loop Jumping Beats when "MOVE" is Turned
Jockey3ME.loop_move_bool = false;
Jockey3ME.MixerDeck1 = 0;
Jockey3ME.MixerDeck2 = 0;

script.crossfaderCurve = function (value, min, max) {
	var newValue = script.absoluteLin(value, 2, 250, min, max);
	var newValueThree = script.absoluteLin(value, 0.5, 0.999307, min, max);
	if (engine.getValue("[Mixer Profile]", "xFaderMode")==1) {
		// Constant Power
		engine.setValue("[Mixer Profile]", "xFaderCalibration", script.absoluteLin(value, 0.5, newValueThree, min, max));
	} else {
		// Additive
		engine.setValue("[Mixer Profile]", "xFaderCurve", script.absoluteLin(value, 1, newValue, min, max));
	}
}

// Functions
// Funny Led show on FX Dry/Wet
Jockey3ME.EffectLedMeterShow = function () {
  midi.sendShortMsg(0x90,0x1D,Jockey3ME.EffectLedMeterValue);
  midi.sendShortMsg(0x91,0x1D,Jockey3ME.EffectLedMeterValue);
  Jockey3ME.EffectLedMeterValue += 2;
  if (Jockey3ME.EffectLedMeterValue >= 127) {
    engine.stopTimer(Jockey3ME.EffectLedMeter);
    Jockey3ME.EffectLedMeter = 0;

    // Sets Effect Leds
		for (var i = 1; i <= 4; i++) {
			for (var j = 1; j <= 3; j++) {
				engine.trigger("[EffectRack1_EffectUnit" + i + "_Effect1]", "parameter" + j);
			}
			engine.trigger("[EffectRack1_EffectUnit" + i + "]", "mix");
		}
  };
}
// Funny Led Show on VuMeter
Jockey3ME.LedMeterShow = function() {
  midi.sendShortMsg(0x90,0x21,Jockey3ME.LedMeterShowValue);
  midi.sendShortMsg(0x91,0x21,Jockey3ME.LedMeterShowValue);
  if (Jockey3ME.LedMeterShowValueTwo == false) {
    ++Jockey3ME.LedMeterShowValue;
  } else {
    --Jockey3ME.LedMeterShowValue;
  }
  if (Jockey3ME.LedMeterShowValue > 10) Jockey3ME.LedMeterShowValueTwo = true;
  // Stop The LedShow start Show on FX
  if (Jockey3ME.LedMeterShowValueTwo && Jockey3ME.LedMeterShowValue < 0) {
    engine.stopTimer(Jockey3ME.LedMeterShowTimer);
    Jockey3ME.LedMeterShowTimer = 0;
    Jockey3ME.EffectLedMeter = engine.beginTimer(20,"Jockey3ME.EffectLedMeterShow()");
  };
}

Jockey3ME.LedShowBegin = function () {
  Jockey3ME.LedMeterShowTimer = engine.beginTimer(40,"Jockey3ME.LedMeterShow()");
}

// Init Script at Programm start
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
  Jockey3ME.LedShowBeginTimer = engine.beginTimer(2000,"Jockey3ME.LedShowBegin()",1); // LedShow Script Starts Here after 500ms
	for (var i = 1; i <= 4; i++) {
		for (var j = 1; j <= 3; j++) {
			engine.connectControl("[EffectRack1_EffectUnit" + i + "_Effect1]","parameter" + j,"Jockey3ME.FX_Param_Led");
		}
		engine.connectControl("[EffectRack1_EffectUnit" + i + "]","mix","Jockey3ME.FX_DryWet_Led");
	}/* while softTakeover isn't fixed.
	for (var k = 1; k <= 4; k++) {
		for (var l = 1; l <= 3; l++) {
			engine.softTakeover("[EqualizerRack1_[Channel" + k + "]_Effect1]","parameter" + l,true);
		}
		engine.softTakeover("[Channel" + k + "]","volume",true);
		engine.softTakeover("[Channel" + k + "]","pregain",true);
	}*/
}

Jockey3ME.shutdown = function () {
  for (var i = 1; i <= 160; i++) {
    midi.sendShortMsg(0x90,i,0x00);
    midi.sendShortMsg(0x91,i,0x00);
    midi.sendShortMsg(0x92,i,0x00);
    midi.sendShortMsg(0x93,i,0x00);
  };
}

// The button that enables/disables scratching
Jockey3ME.wheelTouch = function (channel, control, value, status, group) {
  var currentDeck = parseInt(group.substring(8,9));
    if (value == 0x7F) {  // Some wheels send 0x90 on press and release, so you need to check the value
        var alpha = 1.0/8;
        var beta = alpha/32;
        engine.scratchEnable(currentDeck, Jockey3ME.jogwheelResolution, Jockey3ME.jogwheelSpinSpeed, alpha, beta);
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
      engine.setValue(group, "jog", newValue);
      return;
   }
    engine.scratchTick(currentDeck,newValue);
}

// Hotcues
Jockey3ME.hotcue_activate = function (channel, control, value, status, group) {
	var hotc = control - 10;

	if (!Jockey3ME.hotcueClearVal && value == 0x7F) {
		engine.setValue(group,"hotcue_"+hotc+"_activate",1);
	} else if (Jockey3ME.hotcueClearVal && engine.getValue(group,"hotcue_"+hotc+"_enabled") == 1 && value == 0x7F) {
		engine.setValue(group,"hotcue_"+hotc+"_clear",1);
	} else if (value == 0x00) {
		engine.setValue(group,"hotcue_"+hotc+"_activate",0);
	}
}

Jockey3ME.hotcue_clear = function (channel, control, value, status, group) {
   if (control == 0x09 && value == 0x7F) {
    Jockey3ME.hotcueClearVal = 1;
    midi.sendShortMsg(status,control,0x01);
   } else {
    Jockey3ME.hotcueClearVal = 0;
    midi.sendShortMsg(status,control,0x00);
   };
}

// Effect Sections
Jockey3ME.effectParam = function (channel, control, value, status, group) {
  var currentDeck = parseInt(group.substring(23,24));
  var newVal = 0;
  var interval = 0.04;
	var EncoderKnopFX = control - 29;
	var curVal = engine.getParameter("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]", "parameter" + EncoderKnopFX);
    if (value == 0x41) {
      newVal = curVal + interval;
    } else {
      newVal = curVal - interval;
    }
  switch (engine.getValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]", "num_parameters")) {
    case 1:
      if (EncoderKnopFX > 1) EncoderKnopFX = 0;
      break;
    case 2:
      if (EncoderKnopFX > 2) EncoderKnopFX = 0;
      break;
  }
	if (EncoderKnopFX) {
		engine.setParameter("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]", "parameter" + EncoderKnopFX, newVal);
		engine.trigger("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]", "parameter" + EncoderKnopFX); // Trigger Led-ring
	}
}

Jockey3ME.effectDryWet = function (channel, control, value, status, group) {
	var currentDeck = parseInt(group.substring(23,24));
	var newValue = 0;
	var interval = 0.04;
	var curVal = engine.getParameter("[EffectRack1_EffectUnit" + currentDeck + "]", "mix");
	if (value == 0x41) {
		newValue = curVal + interval;
	} else {
		newValue = curVal - interval;
	}
	engine.setParameter("[EffectRack1_EffectUnit" + currentDeck + "]", "mix", newValue);
	engine.trigger("[EffectRack1_EffectUnit" + currentDeck + "]", "mix"); // Trigger Led-ring
}

Jockey3ME.FX_Param_Led = function (group, value, control) {
	var currentDeck = parseInt(value.substring(23,24)); // Why i have to use value and not group?
	var knop = parseInt(control.substring(9,10));
	var newValue = engine.getParameter(value,control);
	midi.sendShortMsg(0x90+(currentDeck-1),0x1E+(knop-1),(newValue*127));
}

Jockey3ME.FX_DryWet_Led = function (group, value, control) {
	var currentDeck = parseInt(value.substring(23,24));
	var newValue = engine.getParameter(value,control);
	midi.sendShortMsg(0x90+(currentDeck-1),0x1D,(newValue*127));
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
  midi.sendShortMsg((status-32),control,newLedValue);
}

Jockey3ME.effectSelect = function (channel, control, value, status, group) {
	var currentDeck = parseInt(group.substring(23,24));
	var fxChainSelectKnop = 0;
	var fxSelectKnop = 0;
	if (control == 92) {
		fxChainSelectKnop = 1;
	} else {
		fxSelectKnop = control - 92;
	}
	if (fxChainSelectKnop) {
		engine.setValue("[EffectRack1_EffectUnit" + currentDeck + "]", "chain_selector", (value-64));
		// Set Leds
		// Jockey3ME.effectSelectTimer = engine.beginTimer(100, "Jockey3ME.effectSelectLedSet(" + status + "," + currentDeck + ")",1);
		var num_parameters = engine.getValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]", "num_parameters");
		if (num_parameters > 3) {
			num_parameters = 3;
		}
		for (var i = 1; i <= num_parameters; i++) {
			engine.trigger("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]","parameter" + i);
		}
		engine.trigger("[EffectRack1_EffectUnit" + currentDeck + "]","mix");
		Jockey3ME.effectSelectLedSetNumEffect(currentDeck,status,92,(value-64));
	} else if (fxSelectKnop && !Jockey3ME.fxSelectKnopPushIterator1[currentDeck - 1] && !Jockey3ME.fxSelectKnopPushIterator2[currentDeck - 1] && !Jockey3ME.fxSelectKnopPushIterator3[currentDeck - 1]) {
		engine.setValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect" + fxSelectKnop + "]","effect_selector",(value-64));
	} else if (fxSelectKnop == 1 && Jockey3ME.fxSelectKnopPushIterator1[currentDeck - 1]) {
		Jockey3ME.effectSelectParamLinkChose(currentDeck,value,control,status,fxSelectKnop);
		print("FX Sel.1 Turned");
	} else if (fxSelectKnop == 2 && Jockey3ME.fxSelectKnopPushIterator2[currentDeck - 1]) {
		Jockey3ME.effectSelectParamLinkChose(currentDeck,value,control,status,fxSelectKnop);
		print("FX Sel.2 Turned");
	} else if (fxSelectKnop == 3 && Jockey3ME.fxSelectKnopPushIterator3[currentDeck - 1]) {
		Jockey3ME.effectSelectParamLinkChose(currentDeck,value,control,status,fxSelectKnop);
		print("FX Sel.3 Turned");
	}
}

Jockey3ME.effectSelectParamLinkChose = function (currentDeck,value,control,status,fxSelectKnop) {
	if (Jockey3ME.fxSelectKnopPushIterator1[currentDeck - 1] == 1 || Jockey3ME.fxSelectKnopPushIterator2[currentDeck - 1] == 1 || Jockey3ME.fxSelectKnopPushIterator3[currentDeck - 1] == 1) {
		Jockey3ME.effectSelectParamLinkChoseOne(currentDeck,value,control,status,fxSelectKnop);
	} else if (Jockey3ME.fxSelectKnopPushIterator1[currentDeck - 1] == 2 || Jockey3ME.fxSelectKnopPushIterator2[currentDeck - 1] == 2 || Jockey3ME.fxSelectKnopPushIterator3[currentDeck - 1] == 2) {
		Jockey3ME.effectSelectParamLinkChoseTwo(currentDeck,value,control,status,fxSelectKnop);
	} else if (Jockey3ME.fxSelectKnopPushIterator1[currentDeck - 1] == 3 || Jockey3ME.fxSelectKnopPushIterator2[currentDeck - 1] == 3 || Jockey3ME.fxSelectKnopPushIterator3[currentDeck - 1] == 3) {
		Jockey3ME.effectSelectParamLinkChoseThree(currentDeck,value,control,status,fxSelectKnop);
	}
}

Jockey3ME.effectSelectParamLinkChoseOne = function (currentDeck,value,control,status,fxSelectKnop) {
	if ((value-64) == 1) {
		if (!(Jockey3ME.fxSelectKnopPushLedTemp > engine.getValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect" + fxSelectKnop + "]","num_parameters"))) {
			Jockey3ME.fxSelectKnopPushLedTemp += 1;
		}
	} else {
		if (!(Jockey3ME.fxSelectKnopPushLedTemp < 0)) {
			Jockey3ME.fxSelectKnopPushLedTemp -= 1;
		}
	}
	midi.sendShortMsg((status-32),control,parseInt(Jockey3ME.fxSelectKnopPushLedTemp*10.5834));
	Jockey3ME.fxSelectKnopParamChose = Jockey3ME.fxSelectKnopPushLedTemp;
}

Jockey3ME.effectSelectParamLinkChoseTwo = function (currentDeck,value,control,status,fxSelectKnop) {
	if ((value-64) == 1) {
		if (!(Jockey3ME.fxSelectKnopPushLedTemp > 4)) {
			Jockey3ME.fxSelectKnopPushLedTemp += 1;
		}
	} else {
		if (!(Jockey3ME.fxSelectKnopPushLedTemp < 0)) {
			Jockey3ME.fxSelectKnopPushLedTemp -= 1;
		}
	}
	midi.sendShortMsg((status-32),control,parseInt(Jockey3ME.fxSelectKnopPushLedTemp*10.5834));
	Jockey3ME.fxSelectKnopParamLinkChose = Jockey3ME.fxSelectKnopPushLedTemp;
}

Jockey3ME.effectSelectParamLinkChoseThree = function (currentDeck,value,control,status,fxSelectKnop) {
	if ((value-64) == 1) {
		if (!(Jockey3ME.fxSelectKnopPushLedTemp > 1)) {
			Jockey3ME.fxSelectKnopPushLedTemp += 1;
		}
	} else {
		if (!(Jockey3ME.fxSelectKnopPushLedTemp < 0)) {
			Jockey3ME.fxSelectKnopPushLedTemp -= 1;
		}
	}
	midi.sendShortMsg((status-32),control,parseInt(Jockey3ME.fxSelectKnopPushLedTemp*10.5834));
	Jockey3ME.fxSelectKnopParamLinkInverseChose = Jockey3ME.fxSelectKnopPushLedTemp;
}

Jockey3ME.effectSelectPush = function (channel, control, value, status, group) {
	if (value == 0x00) {
		var currentDeck = parseInt(group.substring(23,24));
		Jockey3ME.fxSelectKnopPush[currentDeck-1] = control - 92;
		if (engine.getValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect" + Jockey3ME.fxSelectKnopPush[currentDeck-1] + "]","loaded")) {
			switch (Jockey3ME.fxSelectKnopPush[currentDeck-1]) {
				case 1:
					fxSelectKnopPushBreak: {
						for (var i = 0; i < Jockey3ME.fxSelectKnopPushIterator1.length; i++) {
							if ((currentDeck-1) == i) {
								continue;
							}
							if (Jockey3ME.fxSelectKnopPushIterator1[i]) {
								engine.beginTimer(400,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(600,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
								break fxSelectKnopPushBreak;
							}
						}
						for (var j = 0; j < Jockey3ME.fxSelectKnopPushIterator2.length; j++) {
							if (Jockey3ME.fxSelectKnopPushIterator2[j]) {
								engine.beginTimer(400,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(600,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
								break fxSelectKnopPushBreak;
							}
						}
						for (var k = 0; k < Jockey3ME.fxSelectKnopPushIterator3.length; k++) {
							if (Jockey3ME.fxSelectKnopPushIterator3[k]) {
								engine.beginTimer(400,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(600,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
								break fxSelectKnopPushBreak;
							}
						}
						if (Jockey3ME.fxSelectKnopPushIterator1[currentDeck-1] == 0) {
							Jockey3ME.fxSelectKnopParamLinkInverseChose = 0;
							Jockey3ME.fxSelectKnopParamLinkChose = 0;
							Jockey3ME.fxSelectKnopParamChose = 0;
						}
						++Jockey3ME.fxSelectKnopPushIterator1[currentDeck-1];
						Jockey3ME.fxSelectKnopPushLedTemp = 0;
						if (Jockey3ME.fxSelectKnopPushIterator1[currentDeck-1] > 3) {
							Jockey3ME.fxSelectKnopPushIterator1[currentDeck-1] = 0;
							if (engine.getValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]","parameter" + Jockey3ME.fxSelectKnopParamChose + "_loaded")) {
								engine.setValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]","parameter" + Jockey3ME.fxSelectKnopParamChose + "_link_type",Jockey3ME.fxSelectKnopParamLinkChose);
								engine.setValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect1]","parameter" + Jockey3ME.fxSelectKnopParamChose + "_link_inverse",Jockey3ME.fxSelectKnopParamLinkInverseChose);
								engine.beginTimer(400,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(600,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
								engine.beginTimer(800,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(1000,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
							}
						}
					}
					Jockey3ME.fxSelectKnopPush[currentDeck-1] = 0;
					break;
				case 2:
					fxSelectKnopPushBreak: {
						for (var l = 0; l < Jockey3ME.fxSelectKnopPushIterator1.length; l++) {
							if (Jockey3ME.fxSelectKnopPushIterator1[l]) {
								engine.beginTimer(400,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(600,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
								break fxSelectKnopPushBreak;
							}
						}
						for (var m = 0; m < Jockey3ME.fxSelectKnopPushIterator2.length; m++) {
							if ((currentDeck-1) == m) {
								continue;
							}
							if (Jockey3ME.fxSelectKnopPushIterator2[m]) {
								engine.beginTimer(400,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(600,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
								break fxSelectKnopPushBreak;
							}
						}
						for (var n = 0; n < Jockey3ME.fxSelectKnopPushIterator3.length; n++) {
							if (Jockey3ME.fxSelectKnopPushIterator3[n]) {
								engine.beginTimer(400,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(600,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
								break fxSelectKnopPushBreak;
							}
						}
						if (Jockey3ME.fxSelectKnopPushIterator2[currentDeck-1] == 0) {
							Jockey3ME.fxSelectKnopParamLinkInverseChose = 0;
							Jockey3ME.fxSelectKnopParamLinkChose = 0;
							Jockey3ME.fxSelectKnopParamChose = 0;
						}
						++Jockey3ME.fxSelectKnopPushIterator2[currentDeck-1];
						Jockey3ME.fxSelectKnopPushLedTemp = 0;
						if (Jockey3ME.fxSelectKnopPushIterator2[currentDeck-1] > 3) {
							Jockey3ME.fxSelectKnopPushIterator2[currentDeck-1] = 0;
							if (engine.getValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect2]","parameter" + Jockey3ME.fxSelectKnopParamChose + "_loaded")) {
								engine.setValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect2]","parameter" + Jockey3ME.fxSelectKnopParamChose + "_link_type",Jockey3ME.fxSelectKnopParamLinkChose);
								engine.setValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect2]","parameter" + Jockey3ME.fxSelectKnopParamChose + "_link_inverse",Jockey3ME.fxSelectKnopParamLinkInverseChose);
								engine.beginTimer(400,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(600,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
								engine.beginTimer(800,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(1000,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
							}
						}
					}
					Jockey3ME.fxSelectKnopPush[currentDeck-1] = 0;
					break;
				case 3:
					fxSelectKnopPushBreak: {
						for (var o = 0; o < Jockey3ME.fxSelectKnopPushIterator1.length; o++) {
							if (Jockey3ME.fxSelectKnopPushIterator1[o]) {
								engine.beginTimer(400,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(600,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
								break fxSelectKnopPushBreak;
							}
						}
						for (var p = 0; p < Jockey3ME.fxSelectKnopPushIterator2.length; p++) {
							if (Jockey3ME.fxSelectKnopPushIterator2[p]) {
								engine.beginTimer(400,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(600,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
								break fxSelectKnopPushBreak;
							}
						}
						for (var q = 0; q < Jockey3ME.fxSelectKnopPushIterator3.length; q++) {
							if ((currentDeck-1) == q) {
								continue;
							}
							if (Jockey3ME.fxSelectKnopPushIterator3[q]) {
								engine.beginTimer(400,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(600,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
								break fxSelectKnopPushBreak;
							}
						}
						if (Jockey3ME.fxSelectKnopPushIterator3[currentDeck-1] == 0) {
							Jockey3ME.fxSelectKnopParamChose = 0;
							Jockey3ME.fxSelectKnopParamLinkChose = 0;
							Jockey3ME.fxSelectKnopParamLinkInverseChose = 0;
						}
						++Jockey3ME.fxSelectKnopPushIterator3[currentDeck-1];
						Jockey3ME.fxSelectKnopPushLedTemp = 0;
						if (Jockey3ME.fxSelectKnopPushIterator3[currentDeck-1] > 3) {
							Jockey3ME.fxSelectKnopPushIterator3[currentDeck-1] = 0;
							if (engine.getValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect3]","parameter" + Jockey3ME.fxSelectKnopParamChose + "_loaded")) {
								engine.setValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect3]","parameter" + Jockey3ME.fxSelectKnopParamChose + "_link_type",Jockey3ME.fxSelectKnopParamLinkChose);
								engine.setValue("[EffectRack1_EffectUnit" + currentDeck + "_Effect3]","parameter" + Jockey3ME.fxSelectKnopParamChose + "_link_inverse",Jockey3ME.fxSelectKnopParamLinkInverseChose);
								engine.beginTimer(400,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(600,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
								engine.beginTimer(800,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
								engine.beginTimer(1000,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
							}
						}
					}
					Jockey3ME.fxSelectKnopPush[currentDeck-1] = 0;
					break;
				default:
					print("fxSelectKnopPush Fail.");
			}
		} else {
			Jockey3ME.fxSelectKnopPush[currentDeck-1] = 0;
			engine.beginTimer(400,"midi.sendShortMsg(" + status + "," + control + ",0x7F)",1);
			engine.beginTimer(600,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
		}
		midi.sendShortMsg(status,control,0x7F);
		engine.beginTimer(200,"midi.sendShortMsg(" + status + "," + control + ",0x00)",1);
	}
}

Jockey3ME.effectSuperKnob = function (channel, control, value, status, group) {
	var newValue = (value-64);
	var currentDeck = parseInt(group.substring(23,24));
	var interval = 0.00065;
	var curVal = engine.getParameter("[EffectRack1_EffectUnit" + currentDeck + "]","super1");
	newValue = curVal + (interval * newValue);
	engine.setParameter("[EffectRack1_EffectUnit" + currentDeck + "]","super1",newValue);
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
	engine.setValue(group,"loop_double",0); // if not buttons lit all the way
  } else {
    engine.setValue(group,"loop_halve",1);
	engine.setValue(group,"loop_halve",0);
  }
}

// Must remap if controls are Documented
Jockey3ME.loop_move = function (channel, control, value, status, group) {
  var newValue = (value-64);
	if (status == 0xB0 && !Jockey3ME.loop_move_bool) {
		engine.setValue(group,"loop_move",(newValue*Jockey3ME.loop_move_value));
	} else if (status == 0xB0 && Jockey3ME.loop_move_bool) {
		if (newValue > 0) {
			if (Jockey3ME.loop_move_value < 64) {
				Jockey3ME.loop_move_value *= 2;
			}
		} else {
			if (Jockey3ME.loop_move_value > 0.03125) {
				Jockey3ME.loop_move_value /= 2;
			}
		}
	} else {
		if (value == 0x7F) {
			Jockey3ME.loop_move_bool = true;
		} else {
			Jockey3ME.loop_move_bool = false;
		}
	}
}

Jockey3ME.crossfader = function (channel, control, value, status, group) {
	var newValue = ((value / 63.5) - 1);
	engine.setValue(group,"crossfader",newValue);
}

Jockey3ME.crossfaderCurve = function (channel, control, value, status, group) {
	script.crossfaderCurve(value, 0 , 127);
}

Jockey3ME.MixerVol = function (channel, control, value, status, group) {
  var currentDeck = parseInt(group.substring(8,9));
  if ((Jockey3ME.MixerDeck1 == 1 && currentDeck == 1) || (Jockey3ME.MixerDeck2 == 1 && currentDeck == 2)) {
    currentDeck += 2;
  }
  if (control == 0x2D || control == 0x6C) {
	  engine.setValue("[Channel" + currentDeck + "]","pregain",script.absoluteNonLin(value,0,1,4,0,127));
  } else {
	  engine.setValue("[Channel" + currentDeck + "]","volume",script.absoluteNonLin(value,0,0.25,1,0,127));
  }
}

Jockey3ME.DeckSwitch = function (channel, control, value, status, group) {
  if (control == 0x3C && value == 0x7F) {
    Jockey3ME.MixerDeck1 = 1;
		// while softTakeover isn't fixed
		var k = 3;
		for (var l = 1; l <= 3; l++) {
			engine.softTakeover("[EqualizerRack1_[Channel" + k + "]_Effect1]","parameter" + l,false);
			engine.softTakeover("[EqualizerRack1_[Channel" + k + "]_Effect1]","parameter" + l,true);
		}
		engine.softTakeover("[Channel" + k + "]","volume",false);
		engine.softTakeover("[Channel" + k + "]","volume",true);
		engine.softTakeover("[Channel" + k + "]","pregain",false);
		engine.softTakeover("[Channel" + k + "]","pregain",true);
  } else if (control == 0x3C && value == 0x00) {
    Jockey3ME.MixerDeck1 = 0;
		var k = 1;
		for (var l = 1; l <= 3; l++) {
			engine.softTakeover("[EqualizerRack1_[Channel" + k + "]_Effect1]","parameter" + l,false);
			engine.softTakeover("[EqualizerRack1_[Channel" + k + "]_Effect1]","parameter" + l,true);
		}
		engine.softTakeover("[Channel" + k + "]","volume",false);
		engine.softTakeover("[Channel" + k + "]","volume",true);
		engine.softTakeover("[Channel" + k + "]","pregain",false);
		engine.softTakeover("[Channel" + k + "]","pregain",true);
  } else if (control == 0x3F && value == 0x7F) {
    Jockey3ME.MixerDeck2 = 1;
		var k = 4;
		for (var l = 1; l <= 3; l++) {
			engine.softTakeover("[EqualizerRack1_[Channel" + k + "]_Effect1]","parameter" + l,false);
			engine.softTakeover("[EqualizerRack1_[Channel" + k + "]_Effect1]","parameter" + l,true);
		}
		engine.softTakeover("[Channel" + k + "]","volume",false);
		engine.softTakeover("[Channel" + k + "]","volume",true);
		engine.softTakeover("[Channel" + k + "]","pregain",false);
		engine.softTakeover("[Channel" + k + "]","pregain",true);
  } else if (control == 0x3F && value == 0x00) {
    Jockey3ME.MixerDeck2 = 0;
		var k = 2;
		for (var l = 1; l <= 3; l++) {
			engine.softTakeover("[EqualizerRack1_[Channel" + k + "]_Effect1]","parameter" + l,false);
			engine.softTakeover("[EqualizerRack1_[Channel" + k + "]_Effect1]","parameter" + l,true);
		}
		engine.softTakeover("[Channel" + k + "]","volume",false);
		engine.softTakeover("[Channel" + k + "]","volume",true);
		engine.softTakeover("[Channel" + k + "]","pregain",false);
		engine.softTakeover("[Channel" + k + "]","pregain",true);
  }
}

Jockey3ME.EQ = function (channel, control, value, status, group) {
  var currentDeck = parseInt(group.substring(24,25));
  if ((Jockey3ME.MixerDeck1 == 1 && currentDeck == 1) || (Jockey3ME.MixerDeck2 == 1 && currentDeck == 2)) {
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
	engine.setValue("[EqualizerRack1_[Channel" + currentDeck + "]_Effect1]","parameter" + eqKnop, script.absoluteNonLin(value,0,1,4,0,127));
}

Jockey3ME.trackSearch = function (channel, control, value, status, group) {
	var newValue = (value-64);
	if (newValue > 1 || newValue < -1) {
		newValue /= 2;
	}
	engine.setValue(group,"beatjump",newValue);
}
