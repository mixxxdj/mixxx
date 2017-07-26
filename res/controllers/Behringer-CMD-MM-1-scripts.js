// TO DO
// Add options

var CMDMM = {};

CMDMM.g = {
    'shiftStatus'          : false,
    'inLibrary'            : true, // TRUE = right | FALSE = left 
    'rightFirstPress'      : false, 
    'focusMoved'           : true, // variable to remember if track selector was moved or not, if moved press will load and play track again, if not, will fastforward or stop
    'paused'               : false,  
    'timer'                : null,
    'longpress'            : false,  
    'middlePressed'        : [false, false], 
    'mode'                 : [[0, 0, 0, 0, 0, 0, 0, 0], [2, 0, 2, 0, 0, 2, 0, 2]], // mode[0] - LEDS for effects, mode[1] - orientation LEDs status
    'orientation'          : [[false, false], [false, false], [false, false], [false, false]],
};
//CMDMM.g.timer

CMDMM.init = function (id) {
    for (var i = 1; i <= 300; i++) {
        midi.sendShortMsg(0x94, i, 0x00);
    }
    
    midi.sendShortMsg(0xB4, 80, 48);
    midi.sendShortMsg(0xB4, 81, 48);
    
   engine.connectControl("[Channel1]","VuMeter","CMDMM.vuMeterUpdateL");
   engine.connectControl("[Channel2]","VuMeter","CMDMM.vuMeterUpdateR");
   engine.connectControl("[Channel3]","VuMeter","CMDMM.vuMeterUpdateL");
   engine.connectControl("[Channel4]","VuMeter","CMDMM.vuMeterUpdateR");
    
    engine.connectControl("[Channel1]","pfl", function(value) {midi.sendShortMsg(0x94, 0x30, value)});
    engine.connectControl("[Channel2]","pfl", function(value) {midi.sendShortMsg(0x94, 0x31, value)});
    engine.connectControl("[Channel3]","pfl", function(value) {midi.sendShortMsg(0x94, 0x32, value)});
    engine.connectControl("[Channel4]","pfl", function(value) {midi.sendShortMsg(0x94, 0x33, value)});
    
    
    engine.connectControl("[EffectRack1_EffectUnit1]", 'group_[Channel1]_enable', function(value) {CMDMM.g.mode[0][0] = value;  if (!CMDMM.g.shiftStatus) CMDMM.updateLeds(0);});
    engine.connectControl("[EffectRack1_EffectUnit2]", 'group_[Channel1]_enable', function(value) {CMDMM.g.mode[0][1] = value;  if (!CMDMM.g.shiftStatus) CMDMM.updateLeds(0);});
    engine.connectControl("[EffectRack1_EffectUnit1]", 'group_[Channel2]_enable', function(value) {CMDMM.g.mode[0][2] = value;  if (!CMDMM.g.shiftStatus) CMDMM.updateLeds(0);});
    engine.connectControl("[EffectRack1_EffectUnit2]", 'group_[Channel2]_enable', function(value) {CMDMM.g.mode[0][3] = value;  if (!CMDMM.g.shiftStatus) CMDMM.updateLeds(0);});
    engine.connectControl("[EffectRack1_EffectUnit1]", 'group_[Channel3]_enable', function(value) {CMDMM.g.mode[0][4] = value;  if (!CMDMM.g.shiftStatus) CMDMM.updateLeds(0);});
    engine.connectControl("[EffectRack1_EffectUnit2]", 'group_[Channel3]_enable', function(value) {CMDMM.g.mode[0][5] = value;  if (!CMDMM.g.shiftStatus) CMDMM.updateLeds(0);});
    engine.connectControl("[EffectRack1_EffectUnit1]", 'group_[Channel4]_enable', function(value) {CMDMM.g.mode[0][6] = value;  if (!CMDMM.g.shiftStatus) CMDMM.updateLeds(0);});
    engine.connectControl("[EffectRack1_EffectUnit2]", 'group_[Channel4]_enable', function(value) {CMDMM.g.mode[0][7] = value;  if (!CMDMM.g.shiftStatus) CMDMM.updateLeds(0);});
    
    engine.connectControl("[Channel1]", 'orientation', 'CMDMM.orientationLED');
    engine.connectControl("[Channel2]", 'orientation', 'CMDMM.orientationLED');
    engine.connectControl("[Channel3]", 'orientation', 'CMDMM.orientationLED');
    engine.connectControl("[Channel4]", 'orientation', 'CMDMM.orientationLED');
    
    engine.connectControl("[PreviewDeck1]", 'play', function(value) {CMDMM.g.previewDeckIsPlaying = value});
    
    CMDMM.updateLeds(0);
}

CMDMM.shutdown = function() {
   print('shutdown');
}

CMDMM.vuMeterUpdateL = function (value, group, control) {
    value = (value * 15) + 48;
    midi.sendShortMsg(0xB4, 80, value);
}

CMDMM.vuMeterUpdateR = function (value, group, control) {
    value = (value * 15) + 48;
    midi.sendShortMsg(0xB4, 81, value);
}

CMDMM.orientationLED = function (value, group, control) {
    switch (group) {
        case '[Channel1]':
            switch (value) {
                case 0: CMDMM.g.mode[1][0] = 2; CMDMM.g.mode[1][1] = 0; break;
                case 1: CMDMM.g.mode[1][0] = 2; CMDMM.g.mode[1][1] = 2; break;
                case 2: CMDMM.g.mode[1][0] = 0; CMDMM.g.mode[1][1] = 2; break;
            }
            CMDMM.updateLeds(1);
            break;
        case '[Channel2]': 
            switch (value) {
                case 0: CMDMM.g.mode[1][2] = 2; CMDMM.g.mode[1][3] = 0; break;
                case 1: CMDMM.g.mode[1][2] = 2; CMDMM.g.mode[1][3] = 2; break;
                case 2: CMDMM.g.mode[1][2] = 0; CMDMM.g.mode[1][3] = 2; break;
            }
            CMDMM.updateLeds(1);
            break;
        case '[Channel3]': 
            switch (value) {
                case 0: CMDMM.g.mode[1][4] = 2; CMDMM.g.mode[1][5] = 0; break;
                case 1: CMDMM.g.mode[1][4] = 2; CMDMM.g.mode[1][5] = 2; break;
                case 2: CMDMM.g.mode[1][4] = 0; CMDMM.g.mode[1][5] = 2; break;
            }
            CMDMM.updateLeds(1);
            break;
        case '[Channel4]': 
                        switch (value) {
                case 0: CMDMM.g.mode[1][6] = 2; CMDMM.g.mode[1][7] = 0; break;
                case 1: CMDMM.g.mode[1][6] = 2; CMDMM.g.mode[1][7] = 2; break;
                case 2: CMDMM.g.mode[1][6] = 0; CMDMM.g.mode[1][7] = 2; break;
            }
            CMDMM.updateLeds(1);
            break;
    }
}

CMDMM.updateLeds = function(mode) {
    if (mode) {
            midi.sendShortMsg(0x94, 0x13, CMDMM.g.mode[1][0]);
            midi.sendShortMsg(0x94, 0x14, CMDMM.g.mode[1][1]);
            midi.sendShortMsg(0x94, 0x17, CMDMM.g.mode[1][2]);
            midi.sendShortMsg(0x94, 0x18, CMDMM.g.mode[1][3]);
            midi.sendShortMsg(0x94, 0x1B, CMDMM.g.mode[1][4]);
            midi.sendShortMsg(0x94, 0x1C, CMDMM.g.mode[1][5]);
            midi.sendShortMsg(0x94, 0x1F, CMDMM.g.mode[1][6]);
            midi.sendShortMsg(0x94, 0x20, CMDMM.g.mode[1][7]);
    } else {
            midi.sendShortMsg(0x94, 0x13, CMDMM.g.mode[0][0]);
            midi.sendShortMsg(0x94, 0x14, CMDMM.g.mode[0][1]);
            midi.sendShortMsg(0x94, 0x17, CMDMM.g.mode[0][2]);
            midi.sendShortMsg(0x94, 0x18, CMDMM.g.mode[0][3]);
            midi.sendShortMsg(0x94, 0x1B, CMDMM.g.mode[0][4]);
            midi.sendShortMsg(0x94, 0x1C, CMDMM.g.mode[0][5]);
            midi.sendShortMsg(0x94, 0x1F, CMDMM.g.mode[0][6]);
            midi.sendShortMsg(0x94, 0x20, CMDMM.g.mode[0][7]);
    }
}

CMDMM.shift = function(channel, control, value, status, group) {
    if (value === 127) {
        if (!CMDMM.g.shiftStatus) { //Press SHIFT (LOAD)
            midi.sendShortMsg(0x94, 0x12, 0x01);
            CMDMM.updateLeds(1);
            CMDMM.g.shiftStatus = true;
        } else {
            CMDMM.updateLeds(0);
            midi.sendShortMsg(0x94, 0x12, 0x00);
            CMDMM.g.shiftStatus = false;
        }
    }
}  

CMDMM.librarySwitch = function (channel, control, value, status, group) {
    // Focus is on the right side and cycles through tracks of current view 
    if (CMDMM.g.inLibrary) { 
        if (control === 0x03 && status === 0x94) {
            engine.setValue("[Playlist]","LoadSelectedIntoFirstStopped",1);
        }
        
        if (control === 0x03 && status === 0xB4 && value === 0x41) {
            engine.setValue("[Playlist]","SelectNextTrack",1);
            CMDMM.g.focusMoved = true;
        }
        
        if (control === 0x03 && status === 0xB4 && value === 0x3F) {
            engine.setValue("[Playlist]","SelectPrevTrack",1);
            CMDMM.g.focusMoved = true;
        }
        
        if (!CMDMM.g.rightFirstPress) {
            if (control === 0x11) { 
                if (CMDMM.g.focusMoved === true) { //"focus" means 'current track' selector
                    if (status === 0x84) {
                        engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 1);
                        CMDMM.g.focusMoved = false;
                        CMDMM.g.paused     = false;
                    }
                } else {
                    if (status === 0x94) {
                        CMDMM.g.longpress = false;
                        CMDMM.g.timer = engine.beginTimer(500, function() {engine.setValue("[PreviewDeck1]", "fwd", 1); CMDMM.g.longpress = true;}, true);
                    }

                    if (status === 0x84) {
                        engine.stopTimer(CMDMM.g.timer);
                        if (CMDMM.g.longpress === true) {
                            engine.setValue("[PreviewDeck1]", "fwd", 0);
                        } else {
                            if (!CMDMM.g.paused) {
                                engine.setValue("[PreviewDeck1]", "play", 0);
                                CMDMM.g.paused = true;
                            } else {
                                engine.setValue("[PreviewDeck1]", "play", 1);
                                CMDMM.g.paused = false;
                            } 
                        }
                    }
                }
            }         
        }
  
    CMDMM.g.rightFirstPress = false;    
        
    // Focus moved left and cycles through 'Tracks', 'AutoDJ', 'Playlists', etc.     
    } else if (!CMDMM.g.inLibrary) { 
        
        if (control === 0x03 && status === 0x94) {
            engine.setValue("[Playlist]","ToggleSelectedSidebarItem",1);
        }
        
        if (control === 0x03 && status === 0xB4 && value === 0x41) {
            engine.setValue("[Playlist]","SelectNextPlaylist",1);
        }
        
        if (control === 0x03 && status === 0xB4 && value === 0x3F) {
            engine.setValue("[Playlist]","SelectPrevPlaylist",1);
        }
    }
    
    if (CMDMM.g.inLibrary) {
        if (control === 0x10 && value === 127) {
            CMDMM.g.inLibrary = false;
            CMDMM.g.rightFirstPress = true;
            midi.sendShortMsg(0x94, 0x12, 0x02);
        }
    } else {
        if (control === 0x11 && value === 127) {
            midi.sendShortMsg(0x94, 0x12, 0x00);
            CMDMM.g.inLibrary  = true;
            //CMDMM.g.focusMoved = false;
        }
    }
}

CMDMM.fader = function (channel, control, value, status, group) {
    value = script.absoluteLin(value, 0, 1);
    switch (control) {
        case 0x30: engine.setParameter("[Channel1]", "volume", value); break;
        case 0x31: engine.setParameter("[Channel2]", "volume", value); break;
        case 0x32: engine.setParameter("[Channel3]", "volume", value); break;
        case 0x33: engine.setParameter("[Channel4]", "volume", value); break;
    }
}

CMDMM.button = function (channel, control, value, status, group) {
    switch (control) {
        //CUE
        case 0x30:
            CMDMM.pressButton(1, 'pfl', value, 'toggle');
            break;
        case 0x31:
            CMDMM.pressButton(2, 'pfl', value, 'toggle');
            break;
        case 0x32:
            CMDMM.pressButton(3, 'pfl', value, 'toggle');
            break;
        case 0x33:
            CMDMM.pressButton(4, 'pfl', value, 'toggle');
            break;
            
        //1-2
        case 0x13:
            if (!CMDMM.g.shiftStatus) {
                CMDMM.FX(1, 'group_[Channel1]_enable', value, 'toggle');
            } else {
                CMDMM.orientation(0, 0, status);
            }
            break;
        case 0x14:
            if (!CMDMM.g.shiftStatus) {
                CMDMM.FX(2, 'group_[Channel1]_enable', value, 'toggle');
                
            } else {
                CMDMM.orientation(0, 2, status);
            }
            break;
        case 0x17:
            if (!CMDMM.g.shiftStatus) {
                CMDMM.FX(1, 'group_[Channel2]_enable', value, 'toggle');
            } else {
                CMDMM.orientation(1, 0, status);
            }
            break;
        case 0x18:
            if (!CMDMM.g.shiftStatus) {
                CMDMM.FX(2, 'group_[Channel2]_enable', value, 'toggle');
            } else {
                CMDMM.orientation(1, 2, status);
            }
            break;
        case 0x1B:
            if (!CMDMM.g.shiftStatus) {
                CMDMM.FX(1, 'group_[Channel3]_enable', value, 'toggle');
            } else {
                CMDMM.orientation(2, 0, status);
            }
            break;
        case 0x1C:
            if (!CMDMM.g.shiftStatus) {
                CMDMM.FX(2, 'group_[Channel3]_enable', value, 'toggle');
            } else {
                CMDMM.orientation(2, 2, status);
            }
            break;
        case 0x1F:
            if (!CMDMM.g.shiftStatus) {
                CMDMM.FX(1, 'group_[Channel4]_enable', value, 'toggle');
            } else {
                CMDMM.orientation(3, 0, status);
            }
            break;
        case 0x20:
            if (!CMDMM.g.shiftStatus) {
                CMDMM.FX(2, 'group_[Channel4]_enable', value, 'toggle');
            } else {
                CMDMM.orientation(3, 2, status);
            }
            break;
    }
}

CMDMM.pressButton  = function (channel, action, value, toggle, fireOnRelease, cvalue) {
    var state = 1;
    if (toggle === 'toggle') {
        state = engine.getValue('[Channel' + channel + ']', action);
        if (state === 1) state = 0;
        else state = 1;
    }
    
    if (cvalue === undefined) cvalue = state;
    if (value === 127) {
        engine.setValue('[Channel' + channel + ']', action, cvalue);
    } else {
        if (fireOnRelease === 'fireOnRelease') {
            engine.setValue('[Channel' + channel + ']', action, 0);
        }
    }
}

CMDMM.FX = function (unit, action, value, toggle, fireOnRelease, cvalue) {
    var state = 1;
    if (toggle === 'toggle') {
        state = engine.getValue('[EffectRack1_EffectUnit' + unit + ']', action);
        if (state === 1) state = 0;
        else state = 1;
    }
    
    if (cvalue === undefined) cvalue = state;
    if (value === 127) {
        engine.setValue('[EffectRack1_EffectUnit' + unit + ']', action, cvalue);
    } else {
        if (fireOnRelease === 'fireOnRelease') {
            engine.setValue('[EffectRack1_EffectUnit' + unit + ']', action, 0);
        }
    }
}

CMDMM.orientation = function (channel, value, status) {
    var ch = channel + 1; ch = "[Channel" + ch + "]";
    
    if (status === 0x94) {
        if (CMDMM.g.orientation[channel][0]) {
            CMDMM.g.orientation[channel][1] = true; 
        } else {
            CMDMM.g.orientation[channel][0] = true;
        }
    }
    
    if (status === 0x84) {
        CMDMM.g.orientation[channel][0] = false;
        CMDMM.g.orientation[channel][1] = false;
    }            
    
    if (status === 0x94) {
        if (CMDMM.g.orientation[channel][0] && CMDMM.g.orientation[channel][1]) {
            engine.setParameter(ch, "orientation", 1);
        } else {
            engine.setParameter(ch, "orientation", value);
        }
    }
}
