/**
 * Gemini CDMP-7000 controller script v1.2
 * For Mixxx 1.11.0
 * Written by djtrinidad
 * Copyright 2014
**/

// Changelog v1.2
// Fixed clearing/lighting leds for hotcues if song has them set or not
// Incorporated Pitch Ranges into Button/LCD
// Incorprated BPM in LCD (may still not happen on first load)
// Updates LCD with Vinyl and Hotcue messages

// ---- remaining issues
// slip mode doesn't work unless you manually turn it off (disable needs to be called on wheel hand lift)
// no flashing play buttons, they turn off after first use (need to turn back on after track stop)
// auto cue button doesn't do anything, easy fix
// back button does't toggle directory mode
// Set Master Tempo to Keylock, not sure what it is now 
// many more bug fixes where found
// every once in a while scratch mode ignores vinyl button, hard to respoduce it's intermitent


// Todo:
// Song title to LCD (doesn't look possible with 1.11)
// Elapsed time to LCD
// implement engine connects
// Work towards port to Mixxx 1.12 later


// ------------------------------------------
// ---    Initialization Functions    ---
// ------------------------------------------

function cdmp7000() {}


// Initialize variables
//
cdmp7000.reverse_play_d1 = false;
cdmp7000.reverse_play_d2 = false;
directoryactive = false;

// Global variables
//

cdmp7000.pitchRanges = [ 0.08, 0.16, 0.25, 0.5, 0.100 ];   // Available pitch ranges
cdmp7000.sysex = [0xF0, 0x7D, 0x01];  // Preamble for all SysEx messages and song info for this device
cdmp7000.LCD_CUE = [ 0x3C, 0x74, 0x69, 0x6D, 0x65, 0x3E, 0x3C, 0x63, 0x75, 0x65, 0x30, 0x3E ];
cdmp7000.LCD_CUE1 = [ 0x3C, 0x74, 0x69, 0x6D, 0x65, 0x3E, 0x3C, 0x63, 0x75, 0x65, 0x31, 0x3E ];
cdmp7000.LCD_CUE2 = [ 0x3C, 0x74, 0x69, 0x6D, 0x65, 0x3E, 0x3C, 0x63, 0x75, 0x65, 0x32, 0x3E ];
cdmp7000.LCD_CUE3 = [ 0x3C, 0x74, 0x69, 0x6D, 0x65, 0x3E, 0x3C, 0x63, 0x75, 0x65, 0x33, 0x3E ];
cdmp7000.LCD_LOOP_IN = [ 0x3C, 0x74, 0x69, 0x6D, 0x65, 0x3E, 0x3C, 0x6C, 0x69, 0x6E, 0x3E ];
cdmp7000.LCD_LOOP_OUT = [ 0x3C, 0x74, 0x69, 0x6D, 0x65, 0x3E, 0x3C, 0x6C, 0x6F, 0x75, 0x74, 0x3E ];
//cdmp7000.LCD_PLAY_POS = [0xF0, 0x7D, 0x01, 0x3C, 0x74, 0x69, 0x6D, 0x65, 0x3E, 0x3C, 0x70, 0x6C, 0x61, 0x79, 0x3E];
cdmp7000.LCD_PLAY_POS = [0xF0, 0x7D, 0x01, 0x3C, 0x74, 0x69, 0x6D, 0x65, 0x3E];
cdmp7000.LCD_PAUSE_POS = [ 0x3C, 0x74, 0x69, 0x6D, 0x65, 0x3E, 0x3C, 0x70, 0x61, 0x75, 0x73, 0x65, 0x3E];
cdmp7000.LCD_BPM = [0xF0, 0x7D, 0x01, 0x3C, 0x62, 0x70, 0x6D, 0x3E];
cdmp7000.LCD_PITCH = [0xF0, 0x7D, 0x01, 0x3C, 0x70, 0x69, 0x74, 0x63, 0x68, 0x3E];
cdmp7000.LCD_RANGE = [0xF0, 0x7D, 0x01, 0x3C, 0x72, 0x61, 0x6E, 0x67, 0x65, 0x3E ];
cdmp7000.LCD_FX_PARAM = [ 0x3C, 0x65, 0x66, 0x78, 0x3E ];

// Format!
// may be midi.sendSysexMsg(cdmp7000.sysex.concat([cdmp7000.LCD_BPM],message.toInt(), 0xF7),7+message.length);
// var message = "Whatever";
// length can be adjusted, preamble is 3 bytes, plus close byte is 4


//   Functions - init & shutdown
// 

cdmp7000.init = function (channel, control, value, status, group) {

cdmp7000.vinylButton_d1 = false;
cdmp7000.memoButton_d1 = false;
cdmp7000.fx1_state_d1 = false; 
cdmp7000.fx2_state_d1 = false; 
cdmp7000.fx3_state_d1 = false;
cdmp7000.slip_state_d1 = false;

cdmp7000.vinylButton_d2 = false;
cdmp7000.memoButton_d2 = false;
cdmp7000.fx1_state_d2 = false; 
cdmp7000.fx2_state_d2 = false; 
cdmp7000.fx3_state_d2 = false;
cdmp7000.slip_state_d2 = false;

cdmp7000.firstDeckGroup = "[Channel1]";
cdmp7000.secondDeckGroup = "[Channel2]";

engine.setValue(cdmp7000.firstDeckGroup,'rateRange', 0.08);
engine.setValue(cdmp7000.secondDeckGroup,'rateRange', 0.08);


engine.connectControl(cdmp7000.firstDeckGroup,"rate","cdmp7000.rate_d1");
engine.connectControl(cdmp7000.secondDeckGroup,"rate","cdmp7000.rate_d2");
// may need to add a connect here to turn play led on when stop is pressed

// Turn off all leds to begin with
    for (i=0x01; i<=0x60; i++) midi.sendShortMsg(0x90,i,0x00); 

// Factory lcd clear method
    midi.sendSysexMsg(cdmp7000.sysex.concat([0x3C, 0x62, 0x79, 0x65, 0x3E, 0xF7]),9);

// send welcome message
    message = "<artist><title>MIXXX<album><genre><length>20<index>0";
    midi.sendSysexMsg(cdmp7000.sysex.concat(message.toInt(), 0xF7),4+message.length);   // sendto lcd song name slot

} 

// Shutdown Function

cdmp7000.shutdown = function() {

    for (i=0x01; i<=0x60; i++) midi.sendShortMsg(0x90,i,0x00);  // Turn off all LEDs

    // lcd clear method
    midi.sendSysexMsg(cdmp7000.sysex.concat([0x3C, 0x62, 0x79, 0x65, 0x3E, 0xF7]),9);

}

/// ----- End of Init/Shutdown & Initialization section


cdmp7000.playPositionChanged = function (channel, control, value, status, group) {

var currentValue2 = engine.getValue(group,"playposition");
print ("Play Pos:"+currentValue2);
}


// ------------------------------------------
// ---    Vinyl/Reverse/Slip Functions    ---
// ------------------------------------------


// Toggle vinyl mode deck 1
cdmp7000.vinyl_toggle_d1 = function (channel, control, value, status, group) {
    
    if ((value == 0x7f) && (cdmp7000.vinylButton_d1 == false)) {
        
        cdmp7000.vinylButton_d1 = true;
        midi.sendShortMsg(0x90,0x0E,0x7F);
        cdmp7000.setSongLcd("vinyl", 1);
    }
    else if ((value == 0x7f) && (cdmp7000.vinylButton_d1 == true)) {
        
        cdmp7000.vinylButton_d1 = false;
        midi.sendShortMsg(0x90,0x0E,0x00);
        cdmp7000.setSongLcd("vinyl", 0);
    }
}

// Toggle vinyl mode deck 2
cdmp7000.vinyl_toggle_d2 = function (channel, control, value, status, group) {
    
    if ((value == 0x7f) && (cdmp7000.vinylButton_d2 == false)) {
        
        cdmp7000.vinylButton_d2 = true;
        midi.sendShortMsg(0x90,0x0E,0x7F);
        cdmp7000.setSongLcd("vinyl", 1);
    }
    else if ((value == 0x7f) && (cdmp7000.vinylButton_d2 == true)) {
        
        cdmp7000.vinylButton_d2 = false;
        midi.sendShortMsg(0x90,0x0E,0x00);
        cdmp7000.setSongLcd("vinyl", 0);
    }
}


// Toggle reverse for deck 1, not sure if this works right 
cdmp7000.reverse_toggle_d1 = function(channel, control, value, status, group) {
    if (value == 0) {
        return;
    }
    if (cdmp7000.reverse_play_d1){
       cdmp7000.reverse_play_d1 = false;
       midi.sendShortMsg(0x90,0x1E,0x00);
   } else if (!cdmp7000.reverse_play_d1){
    cdmp7000.reverse_play_d1 = true;
    midi.sendShortMsg(0x90,0x1E,0x7F);
    }
   engine.setValue(group,'reverse',cdmp7000.reverse_play_d1);
}


// Toggle reverse for deck 2, not sure if this works right 
cdmp7000.reverse_toggle_d2 = function(channel, control, value, status, group) {
    if (value == 0) {
        return;
    }
    if (cdmp7000.reverse_play_d2){
       cdmp7000.reverse_play_d2 = false;
       midi.sendShortMsg(0x90,0x1E,0x00);
   } else if (!cdmp7000.reverse_play_d2){
    cdmp7000.reverse_play_d2 = true;
    midi.sendShortMsg(0x90,0x1E,0x7F);
    }
   engine.setValue(group,'reverse',cdmp7000.reverse_play_d2);
}

// slip mode (doesn't work, unless you press again to disable. Disable should occur on wheel touch disable)

cdmp7000.slip_enabled_d1 = function(channel, control, value, status, group) {


    if ((value == 0x7f) && (cdmp7000.slip_state_d1 == false)) {
        
        cdmp7000.slip_state_d1 = true;
        midi.sendShortMsg(0x90,0x1F,0x7F);
        engine.setValue(group, 'slip_enabled', 1);
    }
    else if ((value == 0x7f) && (cdmp7000.slip_state_d1 == true)) {
        
        cdmp7000.slip_state_d1 = false;
        midi.sendShortMsg(0x90,0x1F,0x00);
        engine.setValue(group, 'slip_enabled', 0);
    }
}

cdmp7000.slip_enabled_d2 = function(channel, control, value, status, group) {


    if ((value == 0x7f) && (cdmp7000.slip_state_d2 == false)) {
        
        cdmp7000.slip_state_d2 = true;
        midi.sendShortMsg(0x90,0x1F,0x7F);
        engine.setValue(group, 'slip_enabled', 1);
    }
    else if ((value == 0x7f) && (cdmp7000.slip_state_d2 == true)) {
        
        cdmp7000.slip_state_d2 = false;
        midi.sendShortMsg(0x90,0x1F,0x00);
        engine.setValue(group, 'slip_enabled', 0);
    }
}


// ------------------------------------------
// --------    Jog Wheel Functions   --------
// ------------------------------------------

// Deck 1 - Step 1 If wheels are touched and vinyl is on, then scratch is enabled
cdmp7000.wheelTouch_d1 = function (channel, control, value, status) {

    if ((value == 0x7f) && (cdmp7000.vinylButton_d1 == true)) {
    var alpha = 1.0/8;
    var beta = alpha/32;
    engine.scratchEnable(1, 128, 33+1/3, alpha,beta);
    } else {
            engine.scratchDisable(1);
   }

}

// Deck 1 - Step 2 If scratch is enabled by wheelTouch_d1 then scratch on wheel turn, otherwise jog
cdmp7000.wheelTurn_d1 = function (channel, control, value, status, group) {

  var newValue=(value-64);

  if (!engine.isScratching(1)) {   
    
        engine.setValue(group, "jog", newValue);
        return;

         }

         engine.scratchTick(1,newValue);

}


// Deck 2 - Step 1 If wheels are touched and vinyl is on, then scratch  is enabled
cdmp7000.wheelTouch_d2 = function (channel, control, value, status) {

    if ((value == 0x7f) && (cdmp7000.vinylButton_d2 == true)) {
    var alpha = 1.0/8;
    var beta = alpha/32;
    engine.scratchEnable(2, 128, 33+1/3, alpha,beta);
  } else {
           engine.scratchDisable(2);
   }
}


// Deck 2 - Step 2 If scratch is enabled by wheelTouch_d2 then scratch on wheel turn, otherwise jog
cdmp7000.wheelTurn_d2 = function (channel, control, value, status, group) {

var newValue=(value-64);

// if vinyl mode isn't on then just jog
if (!engine.isScratching(2)) {

engine.setValue(group, "jog", newValue);
return;

   }
     engine.scratchTick(2,newValue);
}


// ------------------------------------------
// -----    Track Selection Functions   -----
// ------------------------------------------

// Back button - Directory/File future code

// Select Track knob - goes positive and negative
cdmp7000.select_track_knob_pos = function(channel, control, value, status, group) {  // The encoder goes positive and negative
    if (value >= 0x01 && value <= 0x1e) {
        value = value;
    } else if (value >= 0x62 && value <= 0x7f) {
        value = 0 - (0x7f-value+1);
    } else {
        return;
    }
    engine.setValue(group,'SelectTrackKnob',value);
};

cdmp7000.select_track_knob_neg = function(channel, control, value, status, group) {
    if (value >= 0x01 && value <= 0x1e) {
        value = value;
    } else if (value >= 0x62 && value <= 0x7f) {
        value = 0 - (0x7f-value-1);
    } else {
        return;
    }
    engine.setValue(group,'SelectTrackKnob',value);
};

cdmp7000.ToggleDirectory = function(channel, control, value, status, group) {

      if (value == 0x7f)  {
           
        print( "Directory Mode!: Coming soon!");

         }

 }

// ------------------------------------------
// ------   Hotcue/Loops Functions   --------
// ------------------------------------------

// Toggle memo mode deck 1 - next hotcue pushed is deleted if on, if false no action
cdmp7000.memoActive_d1 = function (channel, control, value, status, group) {

 
    if ((value == 0x7f) && (cdmp7000.memoButton_d1 == false)) {
        
        cdmp7000.memoButton_d1 = true;
        midi.sendShortMsg(0x90,0x08,0x7F);
    }
    else if ((value == 0x7f) && (cdmp7000.memoButton_d1 == true)) {
        
        cdmp7000.memoButton_d1 = false;
        midi.sendShortMsg(0x90,0x08,0x00);
    }
}


// Toggle memo mode deck 2 - next hotcue pushed is deleted if on, if false no action
cdmp7000.memoActive_d2 = function (channel, control, value, status, group) {

    
    if ((value == 0x7f) && (cdmp7000.memoButton_d2 == false)) {
        
        cdmp7000.memoButton_d2 = true;
        midi.sendShortMsg(0x90,0x08,0x7F);
    }
    else if ((value == 0x7f) && (cdmp7000.memoButton_d2 == true)) {
        
        cdmp7000.memoButton_d2 = false;
        midi.sendShortMsg(0x90,0x08,0x00);
    }
}

// Deck 1 - HotCues - Checks for memo button active to delete hotcue and turn off LED

cdmp7000.hotcue_activate_d1 = function(group,hotcue,value,led) {

    hotcue_state_d1 = 'hotcue_' + hotcue + '_enabled';
    result = engine.getValue("[Channel1]", hotcue_state_d1);
    

    if ((value == 0x7f) && (cdmp7000.memoButton_d1 == true))  {
        key = 'hotcue_' + hotcue + '_clear';

        engine.setValue(group,key,value);
        midi.sendShortMsg(0x90,0x08,0x00); // turn off memo led
        midi.sendShortMsg(0x90,led,0x00); // turn off efx led

       cdmp7000.memoButton_d1 = false;
       cdmp7000.setSongLcd(key, 1);

      } else if ((value == 0x7f) && (cdmp7000.memoButton_d1 == false) && (result == 0))  {

    key = 'hotcue_' + hotcue + '_set';
    engine.setValue(group,key,value);
    midi.sendShortMsg(0x90,led,0x7F);
    cdmp7000.setSongLcd(key, 1);

   } else {

     key = 'hotcue_' + hotcue + '_gotoandplay';
     engine.setValue(group,key,value);

   }
}

// Deck 1 - Hotcues - Actually does the work and calls cdmp7000.hotcue_activate_d1
 
cdmp7000.hotcue_1_activate_d1 = function(channel, control, value, status, group) {

    cdmp7000.hotcue_activate_d1(group,1,value, 0x05);

}
cdmp7000.hotcue_2_activate_d1 = function(channel, control, value, status, group) {
    cdmp7000.hotcue_activate_d1(group,2,value, 0x06);
  
}
cdmp7000.hotcue_3_activate_d1 = function(channel, control, value, status, group) {
    cdmp7000.hotcue_activate_d1(group,3,value, 0x07);
  
}


// Deck 2 - HotCues - Checks for memo button active to delete hotcue and turn off LED
cdmp7000.hotcue_activate_d2 = function(group,hotcue,value,led) {

    hotcue_state_d2 = 'hotcue_' + hotcue + '_enabled';
    result = engine.getValue("[Channel2]", hotcue_state_d2);
    

    if ((value == 0x7f) && (cdmp7000.memoButton_d2 == true))  {
        key = 'hotcue_' + hotcue + '_clear';
        engine.setValue(group,key,value);
        midi.sendShortMsg(0x90,0x08,0x00); // turn off memo led
        midi.sendShortMsg(0x90,led,0x00); // turn off efx led
       cdmp7000.memoButton_d2 = false;
       cdmp7000.setSongLcd(key, 1);
        
    } else if ((value == 0x7f) && (cdmp7000.memoButton_d2 == false) && (result == 0))  {

    key = 'hotcue_' + hotcue + '_set';
    engine.setValue(group,key,value);
    midi.sendShortMsg(0x90,led,0x7F);
    cdmp7000.setSongLcd(key, 1);

   } else {

     key = 'hotcue_' + hotcue + '_gotoandplay';
     engine.setValue(group,key,value);

   }
}

// Deck 2 - Hotcues - Actually does the work and calls cdmp7000.hotcue_activate_d2
cdmp7000.hotcue_1_activate_d2 = function(channel, control, value, status, group) {

    cdmp7000.hotcue_activate_d2(group,1,value, 0x05);
   
    
}
cdmp7000.hotcue_2_activate_d2 = function(channel, control, value, status, group) {
    cdmp7000.hotcue_activate_d2(group,2,value, 0x06);
  
}
cdmp7000.hotcue_3_activate_d2 = function(channel, control, value, status, group) {
    cdmp7000.hotcue_activate_d2(group,3,value, 0x07);
  
}

// Loop functions
cdmp7000.loopIn = function(channel, control, value, status, group) {

     if (!value) return;

     engine.setValue(group, "loop_in", status?1:0);
     midi.sendShortMsg(0x90,0x10,0x7F);  // turn on the loop_in led
     return;

}

cdmp7000.loopOut = function(channel, control, value, status, group) {

    if (!value) return;

       engine.setValue(group, "loop_out", status?1:0);
       midi.sendShortMsg(0x90,0x11,0x7F);  // turn on the loop_out led
       return;

}

cdmp7000.loopExit = function(channel, control, value, status, group) {

    if (!value) return;
       engine.setValue(group, "reloop_exit", status?1:0);
       midi.sendShortMsg(0x90,0x10,0x00);  // turn off the loop_in led
       midi.sendShortMsg(0x90,0x11,0x00);  // turn off the loop_out led
       return;
}


// ------------------------------------------
// --------   Effects Functions   -----------
// ------------------------------------------

// Need to add efx4,efx5,efx6 but what to do with them? also incorporate jog function for effect 

// Deck 1 efx
cdmp7000.fx1_enable_d1 = function (channel, control, value, status, group) {

    if ((value == 0x7f) && (cdmp7000.fx1_state_d1 == false)) {
        
        cdmp7000.fx1_state_d1 = true;
        midi.sendShortMsg(0x90, 0x14, 0x7F);
        engine.setValue(group, "filterHighKill", cdmp7000.fx1_state_d1);
        engine.setValue(group, "filterMidKill", cdmp7000.fx1_state_d1);
    }
    else if ((value == 0x7f) && (cdmp7000.fx1_state_d1 == true)) {
        
        cdmp7000.fx1_state_d1 = false;
        midi.sendShortMsg(0x90,0x14,0x00);
        engine.setValue(group, "filterHighKill", cdmp7000.fx1_state_d1);
        engine.setValue(group, "filterMidKill", cdmp7000.fx1_state_d1);
    }
}

cdmp7000.fx2_enable_d1 = function (channel, control, value, status, group) {

    if ((value == 0x7f) && (cdmp7000.fx2_state_d1 == false)) {
        
        cdmp7000.fx2_state_d1 = true;
        midi.sendShortMsg(0x90, 0x15, 0x7F);
        engine.setValue(group, "flanger", cdmp7000.fx2_state_d1);
    }
    else if ((value == 0x7f) && (cdmp7000.fx2_state_d1 == true)) {
        
        cdmp7000.fx2_state_d1 = false;
        midi.sendShortMsg(0x90,0x15,0x00);
        engine.setValue(group, "flanger", cdmp7000.fx2_state_d1);
    }
}

cdmp7000.fx3_enable_d1 = function (channel, control, value, status, group) {

    if ((value == 0x7f) && (cdmp7000.fx3_state_d1 == false)) {
        
        cdmp7000.fx3_state_d1 = true;
        midi.sendShortMsg(0x90, 0x16, 0x7F);
        engine.brake(1, true);
    }
    else if ((value == 0x7f) && (cdmp7000.fx3_state_d1 == true)) {
        
        cdmp7000.fx3_state_d1 = false;
        midi.sendShortMsg(0x90,0x16,0x00);
        engine.brake(1, false);
    }
}

// Deck 2 efx

cdmp7000.fx1_enable_d2 = function (channel, control, value, status, group) {

    if ((value == 0x7f) && (cdmp7000.fx1_state_d2 == false)) {
        
        cdmp7000.fx1_state_d2 = true;
        midi.sendShortMsg(0x90, 0x14, 0x7F);
        engine.setValue(group, "filterHighKill", cdmp7000.fx1_state_d2);
        engine.setValue(group, "filterMidKill", cdmp7000.fx1_state_d2);
    }
    else if ((value == 0x7f) && (cdmp7000.fx1_state_d2 == true)) {
        
        cdmp7000.fx1_state_d2 = false;
        midi.sendShortMsg(0x90,0x14,0x00);
        engine.setValue(group, "filterHighKill", cdmp7000.fx1_state_d2);
        engine.setValue(group, "filterMidKill", cdmp7000.fx1_state_d2);
    }
}

cdmp7000.fx2_enable_d2 = function (channel, control, value, status, group) {

    if ((value == 0x7f) && (cdmp7000.fx2_state_d2 == false)) {
        
        cdmp7000.fx2_state_d2 = true;
        midi.sendShortMsg(0x90, 0x15, 0x7F);
        engine.setValue(group, "flanger", cdmp7000.fx2_state_d2);
    }
    else if ((value == 0x7f) && (cdmp7000.fx2_state_d2 == true)) {
        
        cdmp7000.fx2_state_d2 = false;
        midi.sendShortMsg(0x90,0x15,0x00);
        engine.setValue(group, "flanger", cdmp7000.fx2_state_d2);
    }
}

cdmp7000.fx3_enable_d2 = function (channel, control, value, status, group) {

    if ((value == 0x7f) && (cdmp7000.fx3_state_d2 == false)) {
        
        cdmp7000.fx3_state_d2 = true;
        midi.sendShortMsg(0x90, 0x16, 0x7F);
        engine.brake(1, true);
    }
    else if ((value == 0x7f) && (cdmp7000.fx3_state_d2 == true)) {
        
        cdmp7000.fx3_state_d2 = false;
        midi.sendShortMsg(0x90,0x16,0x00);
        engine.brake(1, false);
    }
}


// ------------------------------------------
// ---  Load Track - Cue/Play LED Functions   ---
// ------------------------------------------


cdmp7000.LoadTrack = function(channel, control, value, status, group) {

    // Load the selected track in the corresponding deck only if the track is paused

    if(value && engine.getValue(group, "play") != 1)
    {
        engine.setValue(group, "LoadSelectedTrack", 1);

        // Turn on Cue led solid & flash play button (this doesn't flash maybe a connect?)
                       // cue
                       midi.sendShortMsg(0x90,0x01,0x7F);
                       // play
                       midi.sendShortMsg(0x90,0x02,0x7F);

    }
    else engine.setValue(group, "LoadSelectedTrack", 0);

    // (NEW) check song for hotcues set, turn on appropriate led if needed or turn off from previous song
    if(engine.getValue(group, "hotcue_1_enabled") == 1)
    {
     midi.sendShortMsg(0x90,0x05,0x7F);
    } else {
     midi.sendShortMsg(0x90,0x05,0x00);
    }

    if(engine.getValue(group, "hotcue_2_enabled") == 1)
    {
     midi.sendShortMsg(0x90,0x06,0x7F);
    } else {
     midi.sendShortMsg(0x90,0x06,0x00);
    }

    if(engine.getValue(group, "hotcue_3_enabled") == 1)
    {
     midi.sendShortMsg(0x90,0x07,0x7F);
    } else {
     midi.sendShortMsg(0x90,0x07,0x00);
    }

    var currentBpm = engine.getValue(group,'bpm');           
    var currentRange = engine.getValue(group,'rateRange');
    var currentDur = engine.getValue(group,'duration');

    // convert decimal to real number
    currentRange = currentRange*100
    currentRange = Math.round(currentRange);

    currentDur = currentDur*312       // this is inaccurate, varies by 10 seconds or so and never updates

    print( "BPM!:"+currentBpm);  // for debug
    print( "Range!:"+currentRange);  // for debug
    print( "Duration!:"+currentDur);  // for debug
 
        // convert int to string otherwise toInt() will not work
        currentBpm += ''  
        currentRange += ''
        currentDur += ''

        midi.sendSysexMsg(cdmp7000.LCD_BPM.concat(currentBpm.toInt(), 0xF7),9+currentBpm.length); 
        midi.sendSysexMsg(cdmp7000.LCD_RANGE.concat(currentRange.toInt(), 0xF7),11+currentRange.length);
 midi.sendSysexMsg(cdmp7000.LCD_PLAY_POS.concat(currentDur.toInt(), 0x3C, 0x70, 0x6C, 0x61, 0x79, 0x3E, 0xF7),16+currentDur.length);

  
}

// ------------------------------------------
// ---------   Rate Functions   -------------
// ------------------------------------------


cdmp7000.rate_d1 = function (channel, control, value, status, group) {
    var pitchValue = engine.getValue(cdmp7000.firstDeckGroup,'rate');
    var currentBpm = engine.getValue(cdmp7000.firstDeckGroup,'bpm');
        
        pitchValue = pitchValue*10
        pitchValue.toFixed(0)
        pitchValue += '' 
        print( "Deck 1 rate:"+pitchValue);  // for debug    
        midi.sendSysexMsg(cdmp7000.LCD_PITCH.concat(pitchValue.toInt(), 0xF7),11+pitchValue.length);
        // when the pitch slider is changed, we update the bpm as well
        currentBpm += '' 
        midi.sendSysexMsg(cdmp7000.LCD_BPM.concat(currentBpm.toInt(), 0xF7),9+currentBpm.length);
}

cdmp7000.rate_d2 = function (channel, control, value, status, group) {
    var pitchValue = engine.getValue(cdmp7000.secondDeckGroup,'rate');
    var currentBpm = engine.getValue(cdmp7000.secondDeckGroup,'bpm');

        pitchValue = pitchValue*10
        pitchValue.toFixed(0)
        pitchValue += '' 
        print( "Deck 2 rate:"+pitchValue);  // for debug    
        midi.sendSysexMsg(cdmp7000.LCD_PITCH.concat(pitchValue.toInt(), 0xF7),11+pitchValue.length); 
        // when the pitch slider is changed, we update the bpm as well!
        currentBpm += '' 
        midi.sendSysexMsg(cdmp7000.LCD_BPM.concat(currentBpm.toInt(), 0xF7),9+currentBpm.length);
}

cdmp7000.rateRange = function (channel, control, value, status, group) {
   if (value == 0x7f) {
  var currentRange = engine.getValue(group,'rateRange');
  currentRange = currentRange*100;
  currentRange = Math.round(currentRange);


// code to toggle between 8 - 100 range
// there has to be a more efficient way of doing this, maybe external lcd update function
// set, then get, then update
  if (currentRange == "4") {
     engine.setValue(group,'rateRange', 0.08);
     var newRange = engine.getValue(group,'rateRange');
     newRange += ''
     midi.sendSysexMsg(cdmp7000.LCD_RANGE.concat(newRange.toInt(), 0xF7),11+newRange.length);
  }
  if (currentRange == "8") {
     engine.setValue(group,'rateRange', 0.16);
     var newRange = engine.getValue(group,'rateRange');
     newRange += ''
     midi.sendSysexMsg(cdmp7000.LCD_RANGE.concat(newRange.toInt(), 0xF7),11+newRange.length);
  }
  if (currentRange == "16") {
     engine.setValue(group,'rateRange', 0.24);
     var newRange = engine.getValue(group,'rateRange');
     newRange += ''
     midi.sendSysexMsg(cdmp7000.LCD_RANGE.concat(newRange.toInt(), 0xF7),11+newRange.length);
  }
  if (currentRange == "24") {
    engine.setValue(group,'rateRange', 0.50);
     var newRange = engine.getValue(group,'rateRange');
     newRange += ''
    midi.sendSysexMsg(cdmp7000.LCD_RANGE.concat(newRange.toInt(), 0xF7),11+newRange.length);
  }
  if (currentRange == 50) {
    engine.setValue(group,'rateRange', 1.00);
     var newRange = engine.getValue(group,'rateRange');
     newRange += ''
    midi.sendSysexMsg(cdmp7000.LCD_RANGE.concat(newRange.toInt(), 0xF7),11+newRange.length);
  }
  if (currentRange == 100) {
    engine.setValue(group,'rateRange', 0.04);
     var newRange = engine.getValue(group,'rateRange');
     newRange += ''
    midi.sendSysexMsg(cdmp7000.LCD_RANGE.concat(newRange.toInt(), 0xF7),11+newRange.length);
  }


    } // end if val = 0x7f
}



// Reproduces features of Gemini firmware
cdmp7000.setSongLcd = function (control, value) {

if ((control == "hotcue_1_set") && (value == true)) {
       
    message = "<artist><title>Hot Cue 1 is set<album><genre><length>20<index>0";
    midi.sendSysexMsg(cdmp7000.sysex.concat(message.toInt(), 0xF7),4+message.length);
}

if ((control == "hotcue_1_clear") && (value == true)) {
    message = "<artist><title>Hot Cue 1 is clear<album><genre><length>20<index>0";
    midi.sendSysexMsg(cdmp7000.sysex.concat(message.toInt(), 0xF7),4+message.length); 
}

if ((control == "hotcue_2_set") && (value == true)) {
    message = "<artist><title>Hot Cue 2 is set<album><genre><length>20<index>0";
    midi.sendSysexMsg(cdmp7000.sysex.concat(message.toInt(), 0xF7),4+message.length);
}

if ((control == "hotcue_2_clear") && (value == true)) {
    message = "<artist><title>Hot Cue 2 is clear<album><genre><length>20<index>0";
    midi.sendSysexMsg(cdmp7000.sysex.concat(message.toInt(), 0xF7),4+message.length);
}

if ((control == "hotcue_3_set") && (value == true)) {
    message = "<artist><title>Hot Cue 3 is set<album><genre><length>20<index>0";
    midi.sendSysexMsg(cdmp7000.sysex.concat(message.toInt(), 0xF7),4+message.length);
}

if ((control == "hotcue_3_clear") && (value == true)) {
    message = "<artist><title>Hot Cue 3 is clear<album><genre><length>20<index>0";
    midi.sendSysexMsg(cdmp7000.sysex.concat(message.toInt(), 0xF7),4+message.length);
}

if ((control == "vinyl") && (value == true)) {
    message = "<artist><title>Vinyl On<album><genre><length>20<index>0";
    midi.sendSysexMsg(cdmp7000.sysex.concat(message.toInt(), 0xF7),4+message.length);
}

if ((control == "vinyl") && (value == false)) {
    message = "<artist><title>Vinyl Off<album><genre><length>20<index>0";
    midi.sendSysexMsg(cdmp7000.sysex.concat(message.toInt(), 0xF7),4+message.length); 
    }



} // end lcdupdates
