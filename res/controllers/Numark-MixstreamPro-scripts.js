//Numark  Mixstream Pro mapping v17 by ptrex

var MixstreamPro = {}; 

MixstreamPro.init = function(id, debugging) {
    
    engine.makeConnection("[Master]", "VuMeterL", MixstreamPro.vuCallback);
    engine.makeConnection("[Master]", "VuMeterR", MixstreamPro.vuCallback);
     
    engine.makeConnection("[Channel1]", "track_loaded", MixstreamPro.track_loaded1);
    engine.makeConnection("[Channel2]", "track_loaded", MixstreamPro.track_loaded2);
     
    engine.makeConnection("[Channel1]", "play_indicator", MixstreamPro.play_indicator1);
    engine.makeConnection("[Channel2]", "play_indicator", MixstreamPro.play_indicator2);

    LEDs_Init()
}

function LEDs_Init() {
     
    engine.beginTimer(1000, function() {
        midi.sendShortMsg(0x90, 0x75, 0x7f);
    }, 1);

     
    engine.beginTimer(3000, function() {
        midi.sendShortMsg(0x90, 0x75, 0x00);
    }, 1);

     
    engine.beginTimer(4000, function() {
        midi.sendShortMsg(0x92, 8, 0x01);
        midi.sendShortMsg(0x92, 9, 0x01);
        midi.sendShortMsg(0x92, 10, 0x01);
        midi.sendShortMsg(0x92, 11, 0x01);
        midi.sendShortMsg(0x92, 12, 0x01);
        midi.sendShortMsg(0x92, 13, 0x01);
        midi.sendShortMsg(0x92, 14, 0x01);
        midi.sendShortMsg(0x92, 35, 0x01);
    }, 1);

     
    engine.beginTimer(5000, function() {
        midi.sendShortMsg(0x93, 8, 0x01);
        midi.sendShortMsg(0x93, 9, 0x01);
        midi.sendShortMsg(0x93, 10, 0x01);
        midi.sendShortMsg(0x93, 11, 0x01);
        midi.sendShortMsg(0x93, 12, 0x01);
        midi.sendShortMsg(0x93, 13, 0x01);
        midi.sendShortMsg(0x93, 14, 0x01);
        midi.sendShortMsg(0x93, 35, 0x01);
    }, 1);

     
    engine.beginTimer(6000, function() {
        midi.sendShortMsg(0x90, 13, 0x01);
        midi.sendShortMsg(0x91, 13, 0x01);
        midi.sendShortMsg(0x94, 0, 0x01);
        midi.sendShortMsg(0x94, 1, 0x01);
        midi.sendShortMsg(0x94, 2, 0x01);
        midi.sendShortMsg(0x94, 3, 0x01);
    }, 1);
}

 
MixstreamPro.shutdown = function() {
     
    midi.sendShortMsg(0x90, 0x75, 0x00);
}

 
MixstreamPro.prevVuLevelL = 0;
MixstreamPro.prevVuLevelR = 0;
MixstreamPro.maxVuLevel = 85
 
MixstreamPro.vuCallback = function(value, group, control) {
     
    let level = (value * 70)
    level = Math.ceil(level)

    if (group == '[Master]' && control == 'VuMeterL') {
        midi.sendShortMsg(0xBF, 0x20, 0x00);
        if (engine.getValue(group, "PeakIndicatorL")) {
            level = MixstreamPro.maxVuLevel;
        }

        if (MixstreamPro.prevVuLevelL !== level) {
            midi.sendShortMsg(0xBF, 0x20, level);
            MixstreamPro.prevVuLevelL = level;
        }

    } else

    if (group == '[Master]' && control == 'VuMeterR') {
        midi.sendShortMsg(0xBF, 0x21, 0x00);
        if (engine.getValue(group, "PeakIndicatorR")) {
            level = MixstreamPro.maxVuLevel
        }

        if (MixstreamPro.prevVuLevelR !== level) {
            midi.sendShortMsg(0xBF, 0x21, level);
            MixstreamPro.prevVuLevelR = level;
        }
    }
}

 
MixstreamPro.WIFI = true
 
MixstreamPro.Play_Aux_1 = function(channel, control, value, status, group) {
    if (value === 127) {
        if (MixstreamPro.WIFI == true) {
            engine.setValue("[Auxiliary1]", "master", 1)
            engine.setValue("[Auxiliary1]", "pregain", 0.1)
             

            if (engine.getValue("[Channel1]", "play_indicator") == 1) {
                engine.setValue("[Auxiliary1]", "orientation", 2)

            } else
            if (engine.getValue("[Channel2]", "play_indicator") == 1) {
                engine.setValue("[Auxiliary1]", "orientation", 0)
            }

            MixstreamPro.WIFI = false
        } else
        if (MixstreamPro.WIFI == false) {
            engine.setValue("[Auxiliary1]", "master", 0)
            engine.setValue("[Auxiliary1]", "pregain", 0)
            MixstreamPro.WIFI = true
        }
    }
    if (value === 0) { return }
}

 
MixstreamPro.shift = false

MixstreamPro.shiftButton = function(channel, control, value, status, group) {
     
     
    MixstreamPro.shift = !MixstreamPro.shift;  
}
 
MixstreamPro.play1 = function(channel, control, value, status, group) {
    let playStatus = engine.getValue("[Channel1]", "play_indicator")
    let deck = script.deckFromGroup(group)

    if (value === 0x00) {
        if (MixstreamPro.shift) {
            playStatus === 1 ? engine.brake(deck, true, 0.7) : engine.softStart(deck, true, 3);  
        } else {
            playStatus === 1 ? engine.setValue(group, "play", 0) : engine.setValue(group, "play", 1);
        }
    }
}

MixstreamPro.play2 = function(channel, control, value, status, group) {
    let playStatus = engine.getValue("[Channel2]", "play_indicator")
    let deck = script.deckFromGroup(group)

    if (value === 0x00) {
        if (MixstreamPro.shift) {
            playStatus === 1 ? engine.brake(deck, true, 0.7) : engine.softStart(deck, true, 3);  
        } else {
            playStatus === 1 ? engine.setValue(group, "play", 0) : engine.setValue(group, "play", 1);
        }
    }
} 

 
MixstreamPro.play_indicator1 = function(channel, control, value, status, group) {
    engine.setValue("[Auxiliary1]", "orientation", 2)
}

MixstreamPro.play_indicator2 = function(channel, control, value, status, group) {
    engine.setValue("[Auxiliary1]", "orientation", 0)
}

MixstreamPro.jogWheelTicksPerRevolution = 894;
MixstreamPro.jogSensitivity = 0.05;
MixstreamPro.previousJogValue1 = 0
MixstreamPro.previousJogValue2 = 0

MixstreamPro.WheelTouch = function (channel, control, value, status, group) {
    let deckNumber = script.deckFromGroup(group)

if((MixstreamPro.slipenabledToggle1 == true && deckNumber == 1) || (MixstreamPro.slipenabledToggle2 == true && deckNumber == 2)){
    if (value === 0x7F) {     
        let alpha = 1.0/8;
        let beta = alpha/32;
        engine.scratchEnable(deckNumber, 8000, 33+1/3, alpha, beta);
    } else {     
        engine.scratchDisable(deckNumber);
    }
  }
}


MixstreamPro.JogLSB_1 = function(channel, control, value, status, group) {
    
    return 
};

MixstreamPro.JogMSB_1 = function(channel, control, value, status, group) {
        let MSB = value;
        let POS = engine.getValue(group, "playposition")
        let deckNumber = script.deckFromGroup(group);

        switch(true) {
            case POS <= 0:
                 engine.setValue(group, "playposition", 1)
              break;
            case POS >= 1 :
                 engine.setValue(group, "playposition", 0)
              break;
          }

        if (engine.isScratching(deckNumber)) {
            if (MSB >= MixstreamPro.previousJogValue1) {
                engine.scratchTick(deckNumber, MSB/2);  
                MixstreamPro.previousJogValue1 = value;
            }
            else 
                {
                engine.scratchTick(deckNumber, -MSB/2);  
                MixstreamPro.previousJogValue1 = value;
                }
        } 
        else  
        {
            if (MSB >= MixstreamPro.previousJogValue1) {
                engine.setValue(group, "jog", MSB * MixstreamPro.jogSensitivity);
                MixstreamPro.previousJogValue1 = value;
            }
        else 
            {
            engine.setValue(group, "jog", -MSB * MixstreamPro.jogSensitivity);
            MixstreamPro.previousJogValue1 = value;
            }   
        }
};

 
MixstreamPro.JogLSB_2 = function(channel, control, value, status, group) {
     
     return
 };
 
 MixstreamPro.JogMSB_2 = function(channel, control, value, status, group) {
        let MSB = value;
        let POS = engine.getValue(group, "playposition")
        let deckNumber = script.deckFromGroup(group);

        switch(true) {
            case POS <= 0:
                 engine.setValue(group, "playposition", 1)
              break;
            case POS >= 1 :
                 engine.setValue(group, "playposition", 0)
              break;
          }

        if (engine.isScratching(deckNumber)) {
            if (MSB >= MixstreamPro.previousJogValue1) {
                engine.scratchTick(deckNumber, MSB/2);  
                MixstreamPro.previousJogValue1 = value;
            }
        else 
            {
            engine.scratchTick(deckNumber, -MSB/2);  
            MixstreamPro.previousJogValue1 = value;
            }
        } 
        else  
        {
            if (MSB >= MixstreamPro.previousJogValue1) {
                engine.setValue(group, "jog", MSB * MixstreamPro.jogSensitivity);
                MixstreamPro.previousJogValue1 = value;
            }
        else 
            {
            engine.setValue(group, "jog", -MSB * MixstreamPro.jogSensitivity);
            MixstreamPro.previousJogValue1 = value;
            }   
        }
 };

 
MixstreamPro.blinktimer4 = 0
MixstreamPro.hotcuevalue1 = 0
MixstreamPro.LEDblink4 = true
 
MixstreamPro.track_loaded1 = function(channel, control, value, status, group) {
     
    MixstreamPro.AutoloopToggle1 = false
    MixstreamPro.BeatloopRollToggle1 = false

     
    if (engine.getValue("[Channel1]", "track_loaded") !== true && engine.getValue("[Channel2]", "track_loaded") !== true) {
        midi.sendShortMsg(status, 0x0E, 0x7f);
        midi.sendShortMsg(status, 0x0F, 0x7f);
        midi.sendShortMsg(status, 0x10, 0x7f);
        midi.sendShortMsg(status, 0x11, 0x7f);
        midi.sendShortMsg(status, 0x12, 0x7f);
    } else
    if (engine.getValue("[Channel1]", "track_loaded") == true && engine.getValue("[Channel2]", "play_indicator") !== 1) {
        engine.setValue("[Auxiliary1]", "orientation", 2)
    } else
    if (engine.getValue("[Channel1]", "track_loaded") == true && engine.getValue("[Channel2]", "play_indicator") == 1) {
        engine.setValue("[Auxiliary1]", "orientation", 0)
    }

    let hotcues_enabled1 = 0
    let hotcue_Led1 = 14
        MixstreamPro.hotcuevalue1 = 0

    midi.sendShortMsg(0x92, 11, 0x01)  
    midi.sendShortMsg(0x92, 12, 0x01)  
    midi.sendShortMsg(0x92, 13, 0x01)  
    midi.sendShortMsg(0x92, 14, 0x01)  

    for (let i = 1; i <= 4; i++) {
        midi.sendShortMsg(0x92, (hotcue_Led1 + i), 0x01)  
        if (MixstreamPro.blinktimer4 !== 0) {
            engine.stopTimer(MixstreamPro.blinktimer4);
             
            MixstreamPro.blinktimer4 = 0;
        }
    }

    for (let i = 1; i <= 8; i++) {
      
     if (engine.getValue("[Channel1]", "hotcue_" + i + "_type") == 1){ 

        hotcues_enabled1 += engine.getValue("[Channel1]", "hotcue_" + i + "_enabled")

        if (hotcues_enabled1 !== 0 && i < 5 && hotcues_enabled1 !== MixstreamPro.hotcuevalue1) {
            midi.sendShortMsg(0x92, hotcue_Led1 + hotcues_enabled1, 0x7f)
            MixstreamPro.hotcuevalue1 = hotcues_enabled1
        }

        if (hotcues_enabled1 !== 0 && i > 4 && hotcues_enabled1 !== MixstreamPro.hotcuevalue1) {
            midi.sendShortMsg(0x92, (hotcue_Led1 + hotcues_enabled1) - 4, 0x7f)
            MixstreamPro.hotcuevalue1 = hotcues_enabled1
        }
      }
    }

    if (MixstreamPro.hotcuevalue1 > 4) {
        MixstreamPro.blinktimer4 = engine.beginTimer(500, function() {
            if (MixstreamPro.LEDblink4 == true) {
                midi.sendShortMsg(0x92, 0x0B, 0x7f)  
                MixstreamPro.LEDblink4 = false
            } else {
                midi.sendShortMsg(0x92, 0x0B, 0x01) 
                MixstreamPro.LEDblink4 = true
            }
        });
    }

    if (MixstreamPro.hotcuevalue1 <= 4) {
        midi.sendShortMsg(0x92, 0x0B, 0x7f)
    }

    if (MixstreamPro.hotcuevalue1 == 0) {
        midi.sendShortMsg(0x92, 0x0B, 0x01)
        return MixstreamPro.hotcuevalue1
    }

    MixstreamPro.previousJogValue1 = 0

    engine.setValue("[Channel1]", "loop_remove", true)
    engine.setValue("[Channel1]", "beatloop_activate", false)
    engine.setValue("[Channel1]", "beatloop_size", 8)
}

MixstreamPro.blinktimer5 = 0
MixstreamPro.hotcuevalue2 = 0
MixstreamPro.LEDblink5 = true

MixstreamPro.track_loaded2 = function(channel, control, value, status, group) {
     
    MixstreamPro.AutoloopToggle2 = false
    MixstreamPro.BeatloopRollToggle2 = false

     
    if (engine.getValue("[Channel1]", "track_loaded") !== true && engine.getValue("[Channel2]", "track_loaded") !== true) {
        midi.sendShortMsg(status, 0x0E, 0x7f);
        midi.sendShortMsg(status, 0x0F, 0x7f);
        midi.sendShortMsg(status, 0x10, 0x7f);
        midi.sendShortMsg(status, 0x11, 0x7f);
        midi.sendShortMsg(status, 0x12, 0x7f);
    } else
    if (engine.getValue("[Channel2]", "track_loaded") == true && engine.getValue("[Channel1]", "play_indicator") !== 1) {
        engine.setValue("[Auxiliary1]", "orientation", 0)
    } else
    if (engine.getValue("[Channel1]", "track_loaded") == true && engine.getValue("[Channel1]", "play_indicator") == 1) {
        engine.setValue("[Auxiliary1]", "orientation", 2)
    }

    let hotcues_enabled2 = 0
    let hotcue_Led2 = 14
    MixstreamPro.hotcuevalue2 = 0

    midi.sendShortMsg(0x93, 11, 0x01)
    midi.sendShortMsg(0x93, 12, 0x01)
    midi.sendShortMsg(0x93, 13, 0x01)
    midi.sendShortMsg(0x93, 14, 0x01)

    for (let i = 1; i <= 4; i++) {
        midi.sendShortMsg(0x93, (hotcue_Led2 + i), 0x01)
        if (MixstreamPro.blinktimer5 !== 0) {
            engine.stopTimer(MixstreamPro.blinktimer5);
             
            MixstreamPro.blinktimer5 = 0;
        }
    }

    for (let i = 1; i <= 8; i++) {
        if (engine.getValue("[Channel2]", "hotcue_" + i + "_type") == 1){ 

        hotcues_enabled2 += engine.getValue("[Channel2]", "hotcue_" + i + "_enabled")
        if (hotcues_enabled2 !== 0 && i < 5 && hotcues_enabled2 !== MixstreamPro.hotcuevalue2) {
            midi.sendShortMsg(0x93, hotcue_Led2 + hotcues_enabled2, 0x7f)
            MixstreamPro.hotcuevalue2 = hotcues_enabled2
        }

        if (hotcues_enabled2 !== 0 && i > 4 && hotcues_enabled2 !== MixstreamPro.hotcuevalue2) {
            midi.sendShortMsg(0x93, (hotcue_Led2 + hotcues_enabled2) - 4, 0x7f)
            MixstreamPro.hotcuevalue2 = hotcues_enabled2
        }
      }   
    }

    if (MixstreamPro.hotcuevalue2 > 4) {
        MixstreamPro.blinktimer5 = engine.beginTimer(500, function() {
            if (MixstreamPro.LEDblink5 == true) {
                midi.sendShortMsg(0x93, 0x0B, 0x7f)
                MixstreamPro.LEDblink5 = false
            } else {
                midi.sendShortMsg(0x93, 0x0B, 0x01) 
                MixstreamPro.LEDblink5 = true
            }
        });
    }

    if (MixstreamPro.hotcuevalue2 <= 4) {
        midi.sendShortMsg(0x93, 0x0B, 0x7f)
    }

    if (MixstreamPro.hotcuevalue2 == 0) {
        midi.sendShortMsg(0x93, 0x0B, 0x01)
        return MixstreamPro.hotcuevalue2
    }

    MixstreamPro.previousJogValue2 = 0

    engine.setValue("[Channel2]", "loop_remove", true)
    engine.setValue("[Channel2]", "beatloop_activate", false)
    engine.setValue("[Channel2]", "beatloop_size", 8)
}

 
MixstreamPro.slipenabledToggle1 = false

MixstreamPro.slip_enabled_toggle1 = function(channel, control, value, status, group) {
    if (value === 127) {
        if (MixstreamPro.slipenabledToggle1 == false) {
            engine.setValue("[Channel1]", "slip_enabled", true);
            midi.sendShortMsg(status, 0x23, 0x7f);
            MixstreamPro.slipenabledToggle1 = true
        } else
        if (MixstreamPro.slipenabledToggle1 == true) {
            engine.setValue("[Channel1]", "slip_enabled", false);
            midi.sendShortMsg(status, 0x23, 0x01);
            MixstreamPro.slipenabledToggle1 = false
        }    
    } else
    if (value === 0) { return }
}

MixstreamPro.slipenabledToggle2 = false

MixstreamPro.slip_enabled_toggle2 = function(channel, control, value, status, group) {
    if (value === 127) {
        if (MixstreamPro.slipenabledToggle2 == false) {
            engine.setValue("[Channel2]", "slip_enabled", true);
            midi.sendShortMsg(status, 0x23, 0x7f);
            MixstreamPro.slipenabledToggle2 = true
        } else
        if (MixstreamPro.slipenabledToggle2 == true) {
            engine.setValue("[Channel2]", "slip_enabled", false);
            midi.sendShortMsg(status, 0x23, 0x01);
            MixstreamPro.slipenabledToggle2 = false
        }    
    } else
    if (value === 0) { return }
}

 
MixstreamPro.Hotcue_Toggle1 = true

MixstreamPro.cue_goto_toggle1 = function(channel, control, value, status, group) {
    let PlayStatus = engine.getValue("[Channel1]", "play_indicator")
    let trackloaded = engine.getValue("[Channel1]", "track_loaded") 

    let hotcues_enabled1 = 0
    let hotcue_Led1 = 14
        MixstreamPro.hotcuevalue1 = 0

        midi.sendShortMsg(status, 0x0C, 0x01);

    if (value === 127  && trackloaded == true) {   
        if (MixstreamPro.Hotcue_Toggle1 == true) {
            MixstreamPro.Hotcue_Toggle1 = false

            for (let i = 1; i <= 4; i++) {
            midi.sendShortMsg(0x92, (hotcue_Led1 + i), 0x01)
            }
      
        } else
        if (MixstreamPro.Hotcue_Toggle1 == false) {
            midi.sendShortMsg(status, 0x0B, 0x7f);
            MixstreamPro.Hotcue_Toggle1 = true

            for (let i = 1; i <= 8; i++) {
             
            if (engine.getValue("[Channel1]", "hotcue_" + i + "_type") === 1){

                hotcues_enabled1 += engine.getValue("[Channel1]", "hotcue_" + i + "_enabled")
                if (hotcues_enabled1 !== 0 && i < 5 && hotcues_enabled1 !== MixstreamPro.hotcuevalue1) {
                    midi.sendShortMsg(0x92, hotcue_Led1 + hotcues_enabled1, 0x7f)
                    MixstreamPro.hotcuevalue1 = hotcues_enabled1
                }
        
                if (hotcues_enabled1 !== 0 && i > 4 && hotcues_enabled1 !== MixstreamPro.hotcuevalue1) {
                    midi.sendShortMsg(0x92, (hotcue_Led1 + hotcues_enabled1) - 4, 0x7f)
                    MixstreamPro.hotcuevalue1 = hotcues_enabled1
                }
            } 
          }
      }
    } else
    if (value === 0) { return }
}

MixstreamPro.Hotcue_Toggle2= true

MixstreamPro.cue_goto_toggle2 = function(channel, control, value, status, group) {
    let PlayStatus = engine.getValue("[Channel2]", "play_indicator")
    let trackloaded = engine.getValue("[Channel2]", "track_loaded")

    let hotcues_enabled2 = 0
    let hotcue_Led2 = 14
        MixstreamPro.hotcuevalue2 = 0

        midi.sendShortMsg(status, 0x0C, 0x01);

    if (value === 127 && PlayStatus == false && trackloaded == true) {
        if (MixstreamPro.Hotcue_Toggle2 == true) {
            MixstreamPro.Hotcue_Toggle2 = false
    
            for (let i = 1; i <= 4; i++) {
            midi.sendShortMsg(0x93, (hotcue_Led2 + i), 0x01) 
            }

        } else
        if (MixstreamPro.Hotcue_Toggle2 == false) {
            midi.sendShortMsg(status, 0x0B, 0x7f);
            MixstreamPro.Hotcue_Toggle2 = true

            for (let i = 1; i <= 8; i++) {
            if (engine.getValue("[Channel2]", "hotcue_" + i + "_type") === 1){ 

                hotcues_enabled2 += engine.getValue("[Channel2]", "hotcue_" + i + "_enabled")
                if (hotcues_enabled2 !== 0 && i < 5 && hotcues_enabled2 !== MixstreamPro.hotcuevalue2) {
                    midi.sendShortMsg(0x93, hotcue_Led2 + hotcues_enabled2, 0x7f)
                    MixstreamPro.hotcuevalue2 = hotcues_enabled2
                }
        
                if (hotcues_enabled2 !== 0 && i > 4 && hotcues_enabled2 !== MixstreamPro.hotcuevalue2) {
                    midi.sendShortMsg(0x93, (hotcue_Led2 + hotcues_enabled2) - 4, 0x7f)
                    MixstreamPro.hotcuevalue2 = hotcues_enabled2
                }
            } 
          }
      }
    } else
    if (value === 0) { return }
}

 
MixstreamPro.SavedLoop_Toggle1 = false

MixstreamPro.reloop_toggle1 = function(channel, control, value, status, group) {
    let PlayStatus = engine.getValue("[Channel1]", "play_indicator")
    let trackloaded = engine.getValue("[Channel1]", "track_loaded")

    let Loop_enabled1 = 0
    let Loop_Led1 = 14
        

    if (value === 127 && PlayStatus == true && trackloaded == true) {
        if (MixstreamPro.SavedLoop_Toggle1 == true) {
            MixstreamPro.SavedLoop_Toggle1 = false

            midi.sendShortMsg(status, 0x0C, 0x01);

            midi.sendShortMsg(status, 0x0F, 0x01);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x01);

            for (let i = 1; i <= 4; i++) {
                if (engine.getValue("[Channel1]", "hotcue_" + i + "_type") === 4){ 
                    Loop_enabled1 += engine.getValue("[Channel1]", "hotcue_" + i + "_enabled")
                    midi.sendShortMsg(status, (Loop_Led1 + i), 0x01)
                }
            }

        script.triggerControl(group, "reloop_toggle");

        } else
        if (MixstreamPro.SavedLoop_Toggle1 == false) {
            MixstreamPro.SavedLoop_Toggle1 = true

            midi.sendShortMsg(status, 0x0C, 0x7f);

            midi.sendShortMsg(status, 0x0F, 0x01);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x01);

            for (let i = 1; i <= 4; i++) {
                 
                if (engine.getValue("[Channel1]", "hotcue_" + i + "_type") === 4){ 
                    Loop_enabled1 += engine.getValue("[Channel1]", "hotcue_" + i + "_enabled")
                    midi.sendShortMsg(status, Loop_Led1 + Loop_enabled1, 0x7f)
                    
                } 
            }
        }
    } else
    if (value === 0) { return }
}

MixstreamPro.SavedLoop_Toggle2 = false

MixstreamPro.reloop_toggle2 = function(channel, control, value, status, group) {
    let  PlayStatus = engine.getValue("[Channel2]", "play_indicator")
    let  trackloaded = engine.getValue("[Channel2]", "track_loaded")

    let  Loop_enabled2 = 0
    let  Loop_Led2 = 14
       
      
    if (value === 127 && PlayStatus == true && trackloaded == true) {
        if (MixstreamPro.SavedLoop_Toggle2 == true) {
            MixstreamPro.SavedLoop_Toggle2 = false
            
            midi.sendShortMsg(status, 0x0C, 0x01);

            midi.sendShortMsg(status, 0x0F, 0x01);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x01);

            for (let i = 1; i <= 4; i++) {
                if (engine.getValue("[Channel2]", "hotcue_" + i + "_type") === 4){
                    Loop_enabled2 += engine.getValue("[Channel2]", "hotcue_" + i + "_enabled")
                    midi.sendShortMsg(status, (Loop_Led2 + i), 0x01)
                }
            }

        script.triggerControl(group, "reloop_toggle");

        } else
        if (MixstreamPro.SavedLoop_Toggle2 == false) {
            MixstreamPro.SavedLoop_Toggle2 = true

            midi.sendShortMsg(status, 0x0C, 0x7f);

            midi.sendShortMsg(status, 0x0F, 0x01);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x01);

            for (let i = 1; i <= 4; i++) {
                 
                if (engine.getValue("[Channel2]", "hotcue_" + i + "_type") === 4){ 
                        Loop_enabled2 += engine.getValue("[Channel2]", "hotcue_" + i + "_enabled")
                        midi.sendShortMsg(status, Loop_Led2 + Loop_enabled2, 0x7f)
                       
                }
            } 
        }
    } else
    if (value === 0) { return }
}

 
MixstreamPro.AutoloopToggle1 = false

MixstreamPro.Autoloop1 = function(channel, control, value, status, group) {
    let  PlayStatus = engine.getValue("[Channel1]", "play_indicator")

    if (value === 127 && PlayStatus == true) {
        if (MixstreamPro.AutoloopToggle1 == false) {
            MixstreamPro.AutoloopToggle1 = true
            
            engine.setValue("[Channel1]", "beatloop_activate", true)
            engine.setValue("[Channel1]", "beatloop_size", 8)
            midi.sendShortMsg(status, 0x0E, 0x7f);

            midi.sendShortMsg(status, 0x0B, 0x01);
            midi.sendShortMsg(status, 0x0C, 0x01);
            midi.sendShortMsg(status, 0x0D, 0x01);
            midi.sendShortMsg(status, 0x0F, 0x01);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x7f);

            MixstreamPro.Hotcue_Toggle1 = false

        } else
        if (MixstreamPro.AutoloopToggle1 == true) {
            MixstreamPro.AutoloopToggle1 = false
            midi.sendShortMsg(status, 0x0E, 0x01);
            midi.sendShortMsg(status, 0x0F, 0x01);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x01);

            script.triggerControl(group, "reloop_toggle");  
            engine.setValue("[Channel1]", "beatloop_activate", false)

            engine.setValue("[Channel1]", "loop_remove", true)

            MixstreamPro.Hotcue_Toggle1 = true
        }
    } else
    if (value === 0) { return }
}

MixstreamPro.AutoloopToggle2 = false

MixstreamPro.Autoloop2 = function(channel, control, value, status, group) {
    let  PlayStatus = engine.getValue("[Channel2]", "play_indicator")

    if (value === 127 && PlayStatus == true) {
        if (MixstreamPro.AutoloopToggle2 == false) {
            MixstreamPro.AutoloopToggle2 = true

            engine.setValue("[Channel2]", "beatloop_activate", true)
            engine.setValue("[Channel2]", "beatloop_size", 8)

            midi.sendShortMsg(status, 0x0E, 0x7f);

            midi.sendShortMsg(status, 0x0B, 0x01);
            midi.sendShortMsg(status, 0x0C, 0x01);
            midi.sendShortMsg(status, 0x0D, 0x01);
            midi.sendShortMsg(status, 0x0F, 0x01);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x7f);

            MixstreamPro.Hotcue_Toggle2 = false

        } else
        if (MixstreamPro.AutoloopToggle2 == true) {
            MixstreamPro.AutoloopToggle2 = false
            midi.sendShortMsg(status, 0x0E, 0x01);
            midi.sendShortMsg(status, 0x0F, 0x01);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x01);

            script.triggerControl(group, "reloop_toggle");  
            engine.setValue("[Channel2]", "beatloop_activate", false)

            engine.setValue("[Channel2]", "loop_remove", true)

            MixstreamPro.Hotcue_Toggle2 = true
        }
    } else
    if (value === 0) {return }
}

 
MixstreamPro.BeatloopRollToggle1 = false

MixstreamPro.BeatloopRoll1 = function(channel, control, value, status, group) {
    let PlayStatus = engine.getValue("[Channel1]", "play_indicator")
    engine.setValue("[Channel1]", "beatloop_size", 4)

    if (value === 127 && PlayStatus == 1) {
        if (MixstreamPro.BeatloopRollToggle1 == false) {
            MixstreamPro.BeatloopRollToggle1 = true

            engine.setValue("[Channel1]", "beatloop_activate", true)
             

            midi.sendShortMsg(status, 0x0D, 0x7f);

            midi.sendShortMsg(status, 0x0B, 0x01);
            midi.sendShortMsg(status, 0x0C, 0x01);
            midi.sendShortMsg(status, 0x0E, 0x01);
            midi.sendShortMsg(status, 0x0F, 0x01);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x7f);

            MixstreamPro.Hotcue_Toggle1 = false
            MixstreamPro.AutoloopToggle1 = false

        } else
        if (MixstreamPro.BeatloopRollToggle1 == true) {
            MixstreamPro.BeatloopRollToggle1 = false
            midi.sendShortMsg(status, 0x0D, 0x01);
            midi.sendShortMsg(status, 0x0F, 0x01);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x01);

            script.triggerControl(group, "reloop_toggle");

            engine.setValue("[Channel1]", "loop_remove", true)
        }
    } else
    if (value === 0) { return }
}

MixstreamPro.BeatloopRollToggle2 = false

MixstreamPro.BeatloopRoll2 = function(channel, control, value, status, group) {
    let PlayStatus = engine.getValue("[Channel2]", "play_indicator")
    engine.setValue("[Channel2]", "beatloop_size", 4)

    if (value === 127 && PlayStatus == 1) {
        if (MixstreamPro.BeatloopRollToggle2 == false) {
            MixstreamPro.BeatloopRollToggle2 = true

            engine.setValue("[Channel2]", "beatloop_activate", true)
             

            midi.sendShortMsg(status, 0x0D, 0x7f);

            midi.sendShortMsg(status, 0x0B, 0x01);
            midi.sendShortMsg(status, 0x0C, 0x01);
            midi.sendShortMsg(status, 0x0E, 0x01);
            midi.sendShortMsg(status, 0x0F, 0x01);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x7f);

            MixstreamPro.Hotcue_Toggle2 = false
            MixstreamPro.AutoloopToggle2 = false

        } else
        if (MixstreamPro.BeatloopRollToggle2 == true) {
            MixstreamPro.BeatloopRollToggle2 = false
            midi.sendShortMsg(status, 0x0D, 0x01);
            midi.sendShortMsg(status, 0x0F, 0x01);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x01);

            script.triggerControl(group, "reloop_toggle");

            engine.setValue("[Channel2]", "loop_remove", true)
        }
    } else
    if (value === 0) { return }
}

MixstreamPro.Deck1_Pad1 = function(channel, control, value, status, group) {
    let PlayStatus = engine.getValue(group, "play_indicator")

     
    if (value === 127 && MixstreamPro.Hotcue_Toggle1 == true && MixstreamPro.AutoloopToggle1 == false && PlayStatus == false) {
         
         
        if (MixstreamPro.shift) {
             
            engine.setValue("[Channel1]", "hotcue_5_gotoandstop", 1);
        } else {
             
            engine.setValue("[Channel1]", "hotcue_1_gotoandstop", 1);
        }
    } else
    if (value === 0) { 
        return }

     
    let hotcue_status = engine.getValue("[Channel1]", "hotcue_5_status")
    let hotcue_type = engine.getValue("[Channel1]", "hotcue_5_type")

    if (value === 127 && MixstreamPro.SavedLoop_Toggle1 == true && PlayStatus == true && hotcue_type == 4 ) {
        engine.setValue("[Channel1]", "hotcue_5_activateloop", 1);

        hotcue_status == true ? midi.sendShortMsg(status, 0x0F, 0x7f) : midi.sendShortMsg(status, 0x0F, 0x01);
    } else
    if (value === 0) { 
        return }

     
    if (value === 127 && MixstreamPro.AutoloopToggle1 == true && PlayStatus == 1) {
         
            midi.sendShortMsg(status, 0x0F, 0x7f);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x01);

        if (MixstreamPro.shift) {
            engine.setValue("[Channel1]", "beatloop_size", 1/4)
        } else 
            {engine.setValue("[Channel1]", "beatloop_size", 1/2)}

        let loopSize = engine.getValue("[Channel1]", "beatloop_size")

        engine.setValue("[Channel1]", "beatloop_" + loopSize + "_activate", true)
        engine.setValue("[Channel1]", "beatloop_activate", true)
        script.triggerControl(group, "reloop_toggle")
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x0F, 0x01);
        return }

     
    if (value === 127 && MixstreamPro.BeatloopRollToggle1 == true && PlayStatus == 1) {
            midi.sendShortMsg(status, 0x0F, 0x7f);
            midi.sendShortMsg(status, 0x10, 0x01);
            midi.sendShortMsg(status, 0x11, 0x01);
            midi.sendShortMsg(status, 0x12, 0x01);

        engine.setValue(group, "loop_end_position", -1);
        engine.setValue("[Channel1]", "beatloop_size", 0.25)

        let loopSize = engine.getValue("[Channel1]", "beatloop_size")

        engine.setValue("[Channel1]", "beatlooproll_" + loopSize + "_activate", true)
        engine.setValue("[Channel1]", "beatlooproll_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x0F, 0x01);
        return }

     
}

MixstreamPro.Deck1_Pad2 = function(channel, control, value, status, group) {
    let PlayStatus = engine.getValue(group, "play_indicator")

    if (value === 127 && MixstreamPro.Hotcue_Toggle1 == true && MixstreamPro.AutoloopToggle1 == false && PlayStatus == false) {
        if (MixstreamPro.shift) {
            engine.setValue("[Channel1]", "hotcue_6_gotoandstop", 1);
        } else {
            engine.setValue("[Channel1]", "hotcue_2_gotoandstop", 1);
        }
    } else
    if (value === 0) { 
        
        return }

     
    let hotcue_status = engine.getValue("[Channel1]", "hotcue_6_status")
    let hotcue_type = engine.getValue("[Channel1]", "hotcue_6_type")

    if (value === 127 && MixstreamPro.SavedLoop_Toggle1 == true && PlayStatus == true && hotcue_type == 4 ) {
        engine.setValue("[Channel1]", "hotcue_6_activateloop", 1);

        hotcue_status == true ? midi.sendShortMsg(status, 0x10, 0x7f) : midi.sendShortMsg(status, 0x10, 0x01);
    } else
    if (value === 0) { 
        return }

     
    if (value === 127 && MixstreamPro.AutoloopToggle1 == true && PlayStatus == 1) {
        midi.sendShortMsg(status, 0x0F, 0x01);
        midi.sendShortMsg(status, 0x10, 0x7f);
        midi.sendShortMsg(status, 0x11, 0x01);
        midi.sendShortMsg(status, 0x12, 0x01);

         
        engine.setValue("[Channel1]", "beatloop_size", 2)

        let loopSize = engine.getValue("[Channel1]", "beatloop_size")

        engine.setValue("[Channel1]", "beatloop_" + loopSize + "_activate", true)
        engine.setValue("[Channel1]", "beatloop_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x10, 0x01);
        return }

     
    if (value === 127 && MixstreamPro.BeatloopRollToggle1 == true && PlayStatus == 1) {
        midi.sendShortMsg(status, 0x0F, 0x01);
        midi.sendShortMsg(status, 0x10, 0x7f);
        midi.sendShortMsg(status, 0x11, 0x01);
        midi.sendShortMsg(status, 0x12, 0x01);
        
        engine.setValue(group, "loop_end_position", -1);
        engine.setValue("[Channel1]", "beatloop_size", 0.5)

        let loopSize = engine.getValue("[Channel1]", "beatloop_size")

        engine.setValue("[Channel1]", "beatlooproll_" + loopSize + "_activate", true)
        engine.setValue("[Channel1]", "beatlooproll_activate", true)
        script.triggerControl(group, "reloop_toggle");
    }else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x10, 0x01);
        return }

     
}

MixstreamPro.Deck1_Pad3 = function(channel, control, value, status, group) {
    let PlayStatus = engine.getValue(group, "play_indicator")

    if (value === 127 && MixstreamPro.Hotcue_Toggle1 == true && MixstreamPro.AutoloopToggle1 == false && PlayStatus == false) {
        if (MixstreamPro.shift) {
            engine.setValue("[Channel1]", "hotcue_7_gotoandstop", 1);
        } else {
            engine.setValue("[Channel1]", "hotcue_3_gotoandstop", 1);
        }
    } else
    if (value === 0) { 
         
        return }
     
    let hotcue_status = engine.getValue("[Channel1]", "hotcue_7_status")
    let hotcue_type = engine.getValue("[Channel1]", "hotcue_7_type")

    if (value === 127 && MixstreamPro.SavedLoop_Toggle1 == true && PlayStatus == true && hotcue_type == 4 ) {
        engine.setValue("[Channel1]", "hotcue_7_activateloop", 1);

        hotcue_status == true ? midi.sendShortMsg(status, 0x11, 0x7f) : midi.sendShortMsg(status, 0x11, 0x01);
    } else
    if (value === 0) { 
        return }

     
    if (value === 127 && MixstreamPro.AutoloopToggle1 == true && PlayStatus == 1) {
        midi.sendShortMsg(status, 0x0F, 0x01);
        midi.sendShortMsg(status, 0x10, 0x01);
        midi.sendShortMsg(status, 0x11, 0x7f);
        midi.sendShortMsg(status, 0x12, 0x01);

         
        engine.setValue("[Channel1]", "beatloop_size", 4)

        let loopSize = engine.getValue("[Channel1]", "beatloop_size")

        engine.setValue("[Channel1]", "beatloop_" + loopSize + "_activate", true)
        engine.setValue("[Channel1]", "beatloop_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x11, 0x01);
        return }

     
    if (value === 127 && MixstreamPro.BeatloopRollToggle1 == true && PlayStatus == 1) {
        midi.sendShortMsg(status, 0x0F, 0x01);
        midi.sendShortMsg(status, 0x10, 0x01);
        midi.sendShortMsg(status, 0x11, 0x7f);
        midi.sendShortMsg(status, 0x12, 0x01);
        
        engine.setValue(group, "loop_end_position", -1);
        engine.setValue("[Channel1]", "beatloop_size", 1)

        let loopSize = engine.getValue("[Channel1]", "beatloop_size")

        engine.setValue("[Channel1]", "beatlooproll_" + loopSize + "_activate", true)
        engine.setValue("[Channel1]", "beatlooproll_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x11, 0x01);
        return }

     
}

MixstreamPro.Deck1_Pad4 = function(channel, control, value, status, group) {
    let PlayStatus = engine.getValue(group, "play_indicator")

    if (value === 127 && MixstreamPro.Hotcue_Toggle1 == true && MixstreamPro.AutoloopToggle1 == false && PlayStatus == false) {
        if (MixstreamPro.shift) {
            engine.setValue("[Channel1]", "hotcue_8_gotoandstop", 1);
        } else {
            engine.setValue("[Channel1]", "hotcue_4_gotoandstop", 1);
        }
    } else
    if (value === 0) { 
         
        return }
   
    let hotcue_status = engine.getValue("[Channel1]", "hotcue_8_status")
    let hotcue_type = engine.getValue("[Channel1]", "hotcue_8_type")

    if (value === 127 && MixstreamPro.SavedLoop_Toggle1 == true && PlayStatus == true && hotcue_type == 4 ) {
        engine.setValue("[Channel1]", "hotcue_8_activateloop", 1);

        hotcue_status == true ? midi.sendShortMsg(status, 0x12, 0x7f) : midi.sendShortMsg(status, 0x12, 0x01);
    } else
    if (value === 0) { 
        return }

     
    if (value === 127 && MixstreamPro.AutoloopToggle1 == true && PlayStatus == 1) {
        midi.sendShortMsg(status, 0x0F, 0x01);
        midi.sendShortMsg(status, 0x10, 0x01);
        midi.sendShortMsg(status, 0x11, 0x01);
        midi.sendShortMsg(status, 0x12, 0x7f);

         
        engine.setValue("[Channel1]", "beatloop_size", 8)

        let loopSize = engine.getValue("[Channel1]", "beatloop_size")

        engine.setValue("[Channel1]", "beatloop_" + loopSize + "_activate", true)
        engine.setValue("[Channel1]", "beatloop_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x12, 0x01);
        return }

     
    if (value === 127 && MixstreamPro.BeatloopRollToggle1 == true && PlayStatus == 1) {
        midi.sendShortMsg(status, 0x0F, 0x01);
        midi.sendShortMsg(status, 0x10, 0x01);
        midi.sendShortMsg(status, 0x11, 0x01);
        midi.sendShortMsg(status, 0x12, 0x7f);
        
        engine.setValue(group, "loop_end_position", -1);
        engine.setValue("[Channel1]", "beatloop_size", 2)

        let loopSize = engine.getValue("[Channel1]", "beatloop_size")

        engine.setValue("[Channel1]", "beatlooproll_" + loopSize + "_activate", true)
        engine.setValue("[Channel1]", "beatlooproll_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x12, 0x01);
        return }

     
}

MixstreamPro.Deck2_Pad1 = function(channel, control, value, status, group) {
    let PlayStatus = engine.getValue(group, "play_indicator")

    if (value === 127 && MixstreamPro.Hotcue_Toggle2 == true && MixstreamPro.AutoloopToggle2 == false && PlayStatus == false) {
        if (MixstreamPro.shift) {
            engine.setValue("[Channel2]", "hotcue_5_gotoandstop", 1);
        } else {
            engine.setValue("[Channel2]", "hotcue_1_gotoandstop", 1);
        }
    } else
    if (value === 0) {  
        return }
     
    let hotcue_status = engine.getValue("[Channel2]", "hotcue_5_status")
    let hotcue_type = engine.getValue("[Channel2]", "hotcue_5_type")

    if (value === 127 && MixstreamPro.SavedLoop_Toggle2 == true && PlayStatus == true && hotcue_type == 4 ) {
        engine.setValue("[Channel2]", "hotcue_5_activateloop", 1);

        hotcue_status == true ? midi.sendShortMsg(status, 0x0F, 0x7f) : midi.sendShortMsg(status, 0x0F, 0x01);
    } else
    if (value === 0) { 
        return }

     
    if (value === 127 && MixstreamPro.AutoloopToggle2 == true && PlayStatus == 1) {
         
        midi.sendShortMsg(status, 0x0F, 0x7f);
        midi.sendShortMsg(status, 0x10, 0x01);
        midi.sendShortMsg(status, 0x11, 0x01);
        midi.sendShortMsg(status, 0x12, 0x01);

        if (MixstreamPro.shift) {
            engine.setValue("[Channel2]", "beatloop_size", 1/4)
        } else 
            {engine.setValue("[Channel2]", "beatloop_size", 1/2)}

        let loopSize = engine.getValue("[Channel2]", "beatloop_size")

        engine.setValue("[Channel2]", "beatloop_" + loopSize + "_activate", true)
        engine.setValue("[Channel2]", "beatloop_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x0F, 0x01);
        return }
   
    if (value === 127 && MixstreamPro.BeatloopRollToggle2 == true && PlayStatus == 1) {
        midi.sendShortMsg(status, 0x0F, 0x7f);
        midi.sendShortMsg(status, 0x10, 0x01);
        midi.sendShortMsg(status, 0x11, 0x01);
        midi.sendShortMsg(status, 0x12, 0x01);
        
        engine.setValue(group, "loop_end_position", -1);
        engine.setValue("[Channel2]", "beatloop_size", 0.25)

        let loopSize = engine.getValue("[Channel2]", "beatloop_size")

        engine.setValue("[Channel2]", "beatlooproll_" + loopSize + "_activate", true)
        engine.setValue("[Channel2]", "beatlooproll_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x0F, 0x01);
        return }
   
}

MixstreamPro.Deck2_Pad2 = function(channel, control, value, status, group) {
    let PlayStatus = engine.getValue(group, "play_indicator")

    if (value === 127 && MixstreamPro.Hotcue_Toggle2 == true && MixstreamPro.AutoloopToggle2 == false && PlayStatus == false) {
        if (MixstreamPro.shift) {
            engine.setValue("[Channel2]", "hotcue_6_gotoandstop", 1);
        } else {
            engine.setValue("[Channel2]", "hotcue_2_gotoandstop", 1);
        }
    } else
    if (value === 0) { 
         
        return }

     
    let hotcue_status = engine.getValue("[Channel2]", "hotcue_6_status")
    let hotcue_type = engine.getValue("[Channel2]", "hotcue_6_type")

    if (value === 127 && MixstreamPro.SavedLoop_Toggle2 == true && PlayStatus == true && hotcue_type == 4 ) {
        engine.setValue("[Channel2]", "hotcue_6_activateloop", 1);

        hotcue_status == true ? midi.sendShortMsg(status, 0x10, 0x7f) : midi.sendShortMsg(status, 0x10, 0x01);
    } else
    if (value === 0) { 
        return }
     
    if (value === 127 && MixstreamPro.AutoloopToggle2 == true && PlayStatus == 1) {
        midi.sendShortMsg(status, 0x0F, 0x01);
        midi.sendShortMsg(status, 0x10, 0x7f);
        midi.sendShortMsg(status, 0x11, 0x01);
        midi.sendShortMsg(status, 0x12, 0x01);
         
        engine.setValue("[Channel2]", "beatloop_size", 2)

        let loopSize = engine.getValue("[Channel2]", "beatloop_size")

        engine.setValue("[Channel2]", "beatloop_" + loopSize + "_activate", true)
        engine.setValue("[Channel2]", "beatloop_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x10, 0x01);
        return }
     
    if (value === 127 && MixstreamPro.BeatloopRollToggle2 == true && PlayStatus == 1) {
        midi.sendShortMsg(status, 0x0F, 0x01);
        midi.sendShortMsg(status, 0x10, 0x7f);
        midi.sendShortMsg(status, 0x11, 0x01);
        midi.sendShortMsg(status, 0x12, 0x01);
        
        engine.setValue(group, "loop_end_position", -1);
        engine.setValue("[Channel2]", "beatloop_size", 0.5)

        let loopSize = engine.getValue("[Channel2]", "beatloop_size")

        engine.setValue("[Channel2]", "beatlooproll_" + loopSize + "_activate", true)
        engine.setValue("[Channel2]", "beatlooproll_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x10, 0x01);
        return }
     
}

MixstreamPro.Deck2_Pad3 = function(channel, control, value, status, group) {
    let PlayStatus = engine.getValue(group, "play_indicator")

    if (value === 127 && MixstreamPro.Hotcue_Toggle2 == true && MixstreamPro.AutoloopToggle2 == false && PlayStatus == false) {
        if (MixstreamPro.shift) {
            engine.setValue("[Channel2]", "hotcue_7_gotoandstop", 1);
        } else {
            engine.setValue("[Channel2]", "hotcue_3_gotoandstop", 1);
        }
    } else
    if (value === 0) { 
         
        return }
     
    let hotcue_status = engine.getValue("[Channel2]", "hotcue_7_status")
    let hotcue_type = engine.getValue("[Channel2]", "hotcue_7_type")

    if (value === 127 && MixstreamPro.SavedLoop_Toggle2 == true && PlayStatus == true && hotcue_type == 4 ) {
        engine.setValue("[Channel2]", "hotcue_7_activateloop", 1);

        hotcue_status == true ? midi.sendShortMsg(status, 0x11, 0x7f) : midi.sendShortMsg(status, 0x11, 0x01);
    } else
    if (value === 0) { 
        return }
     
    if (value === 127 && MixstreamPro.AutoloopToggle2 == true && PlayStatus == 1) {
        midi.sendShortMsg(status, 0x0F, 0x01);
        midi.sendShortMsg(status, 0x10, 0x01);
        midi.sendShortMsg(status, 0x11, 0x7f);
        midi.sendShortMsg(status, 0x12, 0x01);
         
        engine.setValue("[Channel2]", "beatloop_size", 4)

        let loopSize = engine.getValue("[Channel2]", "beatloop_size")

        engine.setValue("[Channel2]", "beatloop_" + loopSize + "_activate", true)
        engine.setValue("[Channel2]", "beatloop_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x11, 0x01);
        return }
    
    if (value === 127 && MixstreamPro.BeatloopRollToggle2 == true && PlayStatus == 1) {
        midi.sendShortMsg(status, 0x0F, 0x01);
        midi.sendShortMsg(status, 0x10, 0x01);
        midi.sendShortMsg(status, 0x11, 0x7f);
        midi.sendShortMsg(status, 0x12, 0x01);
       
        engine.setValue(group, "loop_end_position", -1);
        engine.setValue("[Channel2]", "beatloop_size", 1)

        let loopSize = engine.getValue("[Channel2]", "beatloop_size")

        engine.setValue("[Channel2]", "beatlooproll_" + loopSize + "_activate", true)
        engine.setValue("[Channel2]", "beatlooproll_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x11, 0x01);
        return }
     
}

MixstreamPro.Deck2_Pad4 = function(channel, control, value, status, group) {
    let PlayStatus = engine.getValue(group, "play_indicator")

    if (value === 127 && MixstreamPro.Hotcue_Toggle2 == true && MixstreamPro.AutoloopToggle2 == false && PlayStatus == false) {
        if (MixstreamPro.shift) {
            engine.setValue("[Channel2]", "hotcue_8_gotoandstop", 1);
        } else {
            engine.setValue("[Channel2]", "hotcue_4_gotoandstop", 1);
        }
    } else
    if (value === 0) { 
         
        return }
     
    let hotcue_status = engine.getValue("[Channel2]", "hotcue_8_status")
    let hotcue_type = engine.getValue("[Channel2]", "hotcue_8_type")

    if (value === 127 && MixstreamPro.SavedLoop_Toggle2 == true && PlayStatus == true && hotcue_type == 4 ) {
        engine.setValue("[Channel2]", "hotcue_8_activateloop", 1);

        hotcue_status == true ? midi.sendShortMsg(status, 0x12, 0x7f) : midi.sendShortMsg(status, 0x12, 0x01);
    } else
    if (value === 0) { 
        return }
     
    if (value === 127 && MixstreamPro.AutoloopToggle2 == true && PlayStatus == 1) {
        midi.sendShortMsg(status, 0x0F, 0x01);
        midi.sendShortMsg(status, 0x10, 0x01);
        midi.sendShortMsg(status, 0x11, 0x01);
        midi.sendShortMsg(status, 0x12, 0x7f);
         
        engine.setValue("[Channel2]", "beatloop_size", 8)

        let loopSize = engine.getValue("[Channel2]", "beatloop_size")

        engine.setValue("[Channel2]", "beatloop_" + loopSize + "_activate", true)
        engine.setValue("[Channel2]", "beatloop_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x12, 0x01);
        return }
     
    if (value === 127 && MixstreamPro.BeatloopRollToggle2 == true && PlayStatus == 1) {
        midi.sendShortMsg(status, 0x0F, 0x01);
        midi.sendShortMsg(status, 0x10, 0x01);
        midi.sendShortMsg(status, 0x11, 0x01);
        midi.sendShortMsg(status, 0x12, 0x7f);
        
        engine.setValue(group, "loop_end_position", -1);
        engine.setValue("[Channel2]", "beatloop_size", 2)

        let loopSize = engine.getValue("[Channel2]", "beatloop_size")

        engine.setValue("[Channel2]", "beatlooproll_" + loopSize + "_activate", true)
        engine.setValue("[Channel2]", "beatlooproll_activate", true)
        script.triggerControl(group, "reloop_toggle");
    } else
    if (value === 0) { 
        midi.sendShortMsg(status, 0x12, 0x01);
        return }
     
}
 
MixstreamPro.EffectToggleSwitch = function(channel, control, value, status, group) {
    if (channel === 4 && value === 1 && MixstreamPro.toggle1 == false || channel === 4 && value === 2 && MixstreamPro.toggle1 == false) {
        engine.setValue("[EffectRack1_EffectUnit1_Effect1]", "enabled", 1);
    } else {
         
        engine.setValue("[EffectRack1_EffectUnit1_Effect1]", "enabled", 0);
    }

    if (channel === 5 && value === 1 && MixstreamPro.toggle1 == false || channel === 5 && value === 2 && MixstreamPro.toggle1 == false) {
        engine.setValue("[EffectRack1_EffectUnit2_Effect1]", "enabled", 1);
    } else {
         
        engine.setValue("[EffectRack1_EffectUnit2_Effect1]", "enabled", 0);
    }

    if (channel === 4 && value === 1 && MixstreamPro.toggle2 == false || channel === 4 && value === 2 && MixstreamPro.toggle2 == false) {
        engine.setValue("[EffectRack1_EffectUnit1_Effect2]", "enabled", 1);
    } else {
         
        engine.setValue("[EffectRack1_EffectUnit1_Effect2]", "enabled", 0);
    }

    if (channel === 5 && value === 1 && MixstreamPro.toggle2 == false || channel === 5 && value === 2 && MixstreamPro.toggle2 == false) {
        engine.setValue("[EffectRack1_EffectUnit2_Effect2]", "enabled", 1);
    } else {
         
        engine.setValue("[EffectRack1_EffectUnit2_Effect2]", "enabled", 0);
    }

    if (channel === 4 && value === 1 && MixstreamPro.toggle3 == false || channel === 4 && value === 2 && MixstreamPro.toggle3 == false) {
        engine.setValue("[EffectRack1_EffectUnit1_Effect3]", "enabled", 1);
    } else {
         
        engine.setValue("[EffectRack1_EffectUnit1_Effect3]", "enabled", 0);
    }

    if (channel === 5 && value === 1 && MixstreamPro.toggle3 == false || channel === 5 && value === 2 && MixstreamPro.toggle3 == false) {
        engine.setValue("[EffectRack1_EffectUnit2_Effect3]", "enabled", 1);
    } else {
         
        engine.setValue("[EffectRack1_EffectUnit2_Effect3]", "enabled", 0);
    }
    
    // To Do FX button 4
    // if (channel === 4 && value === 1 && MixstreamPro.toggle4 == false || channel === 4 && value === 2 && MixstreamPro.toggle4 == false) {
         
    // } else {
    // }

    // if (channel === 5 && value === 1 && MixstreamPro.toggle4 == false || channel === 5 && value === 2 && MixstreamPro.toggle4 == false) {
         
    // } else {
     
    //}
}


MixstreamPro.toggle1 = true
MixstreamPro.blinktimer1 = 0

MixstreamPro.Effectbutton1 = function(channel, control, value, status, group) {
    if (value === 127) {
        MixstreamPro.LEDblink1 = true
        if (MixstreamPro.toggle1 == true) {
             
            MixstreamPro.blinktimer1 = engine.beginTimer(500, function() {
                if (MixstreamPro.LEDblink1 == true) {
                    midi.sendShortMsg(0x94, 0x00, 0x7F)
                    MixstreamPro.LEDblink1 = false
                } else {
                    midi.sendShortMsg(0x94, 0x00, 0x01)
                    MixstreamPro.LEDblink1 = true
                }
            });
            MixstreamPro.toggle1 = false
        } else
        if (MixstreamPro.toggle1 == false) {
            if (MixstreamPro.blinktimer1 !== 0) {
                engine.stopTimer(MixstreamPro.blinktimer1);
                 
                MixstreamPro.blinktimer1 = 0;
            }
            midi.sendShortMsg(0x94, 0x00, 0x01);
            MixstreamPro.toggle1 = true
        }
    } else
    if (value === 0) { return }
}

MixstreamPro.toggle2 = true
MixstreamPro.blinktimer2 = 0
MixstreamPro.Effectbutton2 = function(channel, control, value, status, group) {
    if (value === 127) {
        MixstreamPro.LEDblink2 = true
        if (MixstreamPro.toggle2 == true) {
             
            MixstreamPro.blinktimer2 = engine.beginTimer(500, function() {
                if (MixstreamPro.LEDblink2 == true) {
                    midi.sendShortMsg(0x94, 0x01, 0x7F)
                    MixstreamPro.LEDblink2 = false
                } else {
                    midi.sendShortMsg(0x94, 0x01, 0x01)
                    MixstreamPro.LEDblink2 = true
                }
            });
            MixstreamPro.toggle2 = false
        } else
        if (MixstreamPro.toggle2 == false) {
            if (MixstreamPro.blinktimer2 !== 0) {
                engine.stopTimer(MixstreamPro.blinktimer2);
                 
                MixstreamPro.blinktimer2 = 0;
            }

            midi.sendShortMsg(0x94, 0x01, 0x01);
            MixstreamPro.toggle2 = true
        }
    } else
    if (value === 0) { return }
}

MixstreamPro.toggle3 = true
MixstreamPro.blinktimer3 = 0
MixstreamPro.Effectbutton3 = function(channel, control, value, status, group) {
    if (value === 127) {
        MixstreamPro.LEDblink3 = true
        if (MixstreamPro.toggle3 == true) {
             
            MixstreamPro.blinktimer3 = engine.beginTimer(500, function() {
                if (MixstreamPro.LEDblink3 == true) {
                    midi.sendShortMsg(0x94, 0x02, 0x7F)
                    MixstreamPro.LEDblink3 = false
                } else {
                    midi.sendShortMsg(0x94, 0x02, 0x01)
                    MixstreamPro.LEDblink3 = true
                }
            });
            MixstreamPro.toggle3 = false
        } else
        if (MixstreamPro.toggle3 == false) {
            if (MixstreamPro.blinktimer3 !== 0) {
                engine.stopTimer(MixstreamPro.blinktimer3);
                 
                MixstreamPro.blinktimer3 = 0;
            }

            midi.sendShortMsg(0x94, 0x02, 0x01);
            MixstreamPro.toggle3 = true
        }
    } else
    if (value === 0) { return }
}

MixstreamPro.toggle4 = true
MixstreamPro.blinktimer4 = 0

MixstreamPro.Effectbutton4 = function(channel, control, value, status, group) {
    if (value === 127) {
        MixstreamPro.LEDblink4 = true
        if (MixstreamPro.toggle4 == true) {
             
            MixstreamPro.blinktimer4 = engine.beginTimer(500, function() {
                if (MixstreamPro.LEDblink4 == true) {
                    midi.sendShortMsg(0x94, 0x03, 0x7F)
                    MixstreamPro.LEDblink4 = false
                } else {
                    midi.sendShortMsg(0x94, 0x03, 0x01)
                    MixstreamPro.LEDblink4 = true
                }
            });
            MixstreamPro.toggle4 = false
        } else
        if (MixstreamPro.toggle4 == false) {
            if (MixstreamPro.blinktimer4 !== 0) {
                engine.stopTimer(MixstreamPro.blinktimer4);
                 
                MixstreamPro.blinktimer4 = 0;
            }

            midi.sendShortMsg(0x94, 0x03, 0x01);
            MixstreamPro.toggle4 = true
        }
    } else
    if (value === 0) { return }
}
