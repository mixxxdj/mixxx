var CMDPL = new Object();

CMDPL.init = function (id) {
	
	CMDPL.shift = [false, false, false, false];
	CMDPL.chList = ['[Channel1]', '[Channel2]', '[Channel3]', '[Channel4]'];
	CMDPL.cGroup = '[Channel1]';
	
	CMDPL.beat_size = [0.5, 1, 2, 4, 8, 16, 32, 64, 128];
	CMDPL.beat_size_pos = 3;
	CMDPL.loop_size = [0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, 128];
	CMDPL.loop_size_pos = 3;
	
	CMDPL.syncTimer;
	CMDPL.syncLongPress = false;
	
	CMDPL.fader        = [false, false, false, false];
	CMDPL.permaScratch = [false, false, false, false];
	CMDPL.rpm = 33+1/8;
	CMDPL.alpha = 1/2;
	CMDPL.beta  = CMDPL.alpha/32;
	
	
	
	CMDPL.invertJogs = true;
	
	CMDPL.specialMODE = [false, false, false, false];
	
	CMDPL.rroundLED = [[0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08], [0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08], [0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08], [0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08]]; // Round leds
	//CMDPL.rmidLED   = [[0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00], [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00], [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00], [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]]; // Middle led
	
    for (var c = 0; c <= 3; c++) {
        for (var i = 1; i <= 256; i++) {
            //midi.sendShortMsg(0x90 + c, i, 0x00);
            //midi.sendShortMsg(0xB0 + c, i, 0x00);  
            //midi.sendShortMsg(0xB0 + c, 0x0A, 0x08);        
        } 
        
        // Pitch on middle
        //midi.sendShortMsg(0x91, 0x03, 0x01);   
        //midi.sendShortMsg(0xB1, 0x03, 0x08);
    }
      
    midi.sendShortMsg(0xB0, 0x01, CMDPL.rroundLED[CMDPL.getChannelN(CMDPL.cGroup)][1]); // Key default round
    midi.sendShortMsg(0xB0, 0x07, CMDPL.rroundLED[CMDPL.getChannelN(CMDPL.cGroup)][7]); // BPM round
    
	// Connect no-channel Rotaries
	CMDPL.supr1led    = engine.makeConnection('[QuickEffectRack1_' + CMDPL.cGroup + ']', 'super1', CMDPL.super1red); CMDPL.supr1led.trigger(); 
	CMDPL.loopsizeled = engine.makeConnection(CMDPL.cGroup, 'loop_enabled', CMDPL.beatszch); CMDPL.loopsizeled.trigger(); 
	CMDPL.qntzled     = engine.makeConnection(CMDPL.cGroup, 'quantize', CMDPL.qntz); CMDPL.qntzled.trigger(); 

	
	
    // Connect LEDs /w Channels
    for (var i = 0; i <= 3; i++ ) {  
		
		engine.makeConnection(CMDPL.chList[i],'rate'            , CMDPL.rateLED).trigger(); 
        
		engine.makeConnection(CMDPL.chList[i], 'sync_enabled',  function (value, group, control) {midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x20, value);}).trigger();
		engine.makeConnection(CMDPL.chList[i], 'play',          function (value, group, control) {midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x23, value);}).trigger();
		engine.makeConnection(CMDPL.chList[i], 'cue_indicator', function (value, group, control) {midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x22, value);}).trigger();
		
		engine.makeConnection(CMDPL.chList[i], 'keylock',      function (value, group, control) {midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x19, value);}).trigger();
        
		engine.makeConnection(CMDPL.chList[i], 'hotcue_1_enabled', function (value, group, control) {midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x10, value);}).trigger();
		engine.makeConnection(CMDPL.chList[i], 'hotcue_2_enabled', function (value, group, control) {midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x11, value);}).trigger();
		engine.makeConnection(CMDPL.chList[i], 'hotcue_3_enabled', function (value, group, control) {midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x12, value);}).trigger();
		engine.makeConnection(CMDPL.chList[i], 'hotcue_4_enabled', function (value, group, control) {midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x13, value);}).trigger();
		
		engine.makeConnection(CMDPL.chList[i], 'quantize', function (value, group, control) {midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x17, value);}).trigger();
		
		engine.makeConnection(CMDPL.chList[i], 'loop_enabled',        function (value, group, control) {midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x14, value);}).trigger();
		engine.makeConnection(CMDPL.chList[i], 'loop_start_position', function (value, group, control) { value = value >= 0 ? 0x01 : 0x00; midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x15, value);}).trigger();
		engine.makeConnection(CMDPL.chList[i], 'loop_end_position',   function (value, group, control) { value = value >= 0 ? 0x01 : 0x00; midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x16, value);}).trigger();
        
		engine.makeConnection(CMDPL.chList[i], 'slip_enabled',        function (value, group, control) {midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x21, value);}).trigger();
		
		engine.makeConnection(CMDPL.chList[i], 'scratch2_enable',     function (value, group, control) {midi.sendShortMsg(0x90 + CMDPL.getChannelN(group), 0x1B, value);}).trigger();
    
		engine.softTakeover(CMDPL.chList[i], 'rate', true);
	} 
	
	// Ping
	midi.sendShortMsg(0x90, 0x18, 0x01);
	midi.sendShortMsg(0x91, 0x18, 0x01);
	midi.sendShortMsg(0x92, 0x18, 0x01);
	midi.sendShortMsg(0x93, 0x18, 0x01);
	
	//engine.softTakeover('[Channel1]', 'rate', true);
	//engine.softTakeover('[Channel2]', 'rate', true);
	//engine.softTakeover('[Channel3]', 'rate', true);
	//engine.softTakeover('[Channel4]', 'rate', true);
	
	
}

CMDPL.super1red = function (value, group, control) {
	if (value < 0.46 || value > 0.54) midi.sendShortMsg(0x90, 0x00, 0x01); 
	else midi.sendShortMsg(0x90, 0x00, 0x00);
}
CMDPL.beatszch = function (value, group, control) {
	midi.sendShortMsg(0x90, 0x04, value);
}
CMDPL.qntz = function (value, group, control) {
	midi.sendShortMsg(0x90, 0x17, value);
}

CMDPL.button = function (channel, control, value, status, group) {
    //print('button');
    //channel = 0 1 2 3
    //print(group);
    //print(channel);
	
	// First 8 buttons in rotaries always send channel 0!
	switch (control) {
		case 0x00:  // RESET FILTER
			if (value === 127) {
				engine.setValue('[QuickEffectRack1_' + CMDPL.cGroup + ']', 'super1', 0.5);
			}
        break;
		
        case 0x01: //  KEY SYNC / RESET PITCH / CHANGE PITCH
			if (value === 127) {
				midi.sendShortMsg(0x90, 0x01, 0x01);
				engine.setValue(CMDPL.cGroup, 'pitch', 0);
				
				midi.sendShortMsg(0xB0, 0x01, CMDPL.rroundLED[CMDPL.getChannelN(CMDPL.cGroup)][1] = 8);
			}
			if (value === 0) {
				midi.sendShortMsg(0x90, 0x01, 0x00);
			}
        break;

        case 0x02:
			if (value === 127) {
				
				
			}
        break;

        case 0x03: // Adjust Beat Grid 2
			if (value === 127) {
				script.triggerControl(CMDPL.cGroup, 'beats_translate_curpos');
			}
        break;
		
        case 0x07: // Reset BPM
			if (value === 127) {
				engine.setValue(CMDPL.cGroup, 'rate', 0);
				midi.sendShortMsg(0xB0, control, CMDPL.rroundLED[CMDPL.getChannelN(CMDPL.cGroup)][control] = 0x08);
			}
        break;
		
		 
        // BUTTONS 1-8    
        case 0x10: 
			if (value === 127) {
				if (CMDPL.shift[channel]) {
					engine.setValue(CMDPL.chList[channel], 'hotcue_1_clear', 1);
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'hotcue_1_activate', 1);
				
			}
			if (value === 0) {
				//midi.sendShortMsg(0x90 + channel, 0x01, 0x00);
				if (CMDPL.shift[channel]) {
					
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'hotcue_1_activate', 0);
			}
		break;	
			
        case 0x11: 
			if (value === 127) {
				if (CMDPL.shift[channel]) {
					engine.setValue(CMDPL.chList[channel], 'hotcue_2_clear', 1);
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'hotcue_2_activate', 1);
			}
			if (value === 0) {
				//midi.sendShortMsg(0x90 + channel, 0x01, 0x00);
				if (CMDPL.shift[channel]) {
					
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'hotcue_2_activate', 0);
			}
		break;
		
        case 0x12: 
			if (value === 127) {
				if (CMDPL.shift[channel]) {
					engine.setValue(CMDPL.chList[channel], 'hotcue_3_clear', 1);
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'hotcue_3_activate', 1);
			}
			if (value === 0) {
				//midi.sendShortMsg(0x90 + channel, 0x01, 0x00);
				if (CMDPL.shift[channel]) {
					
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'hotcue_3_activate', 0);
			}
		break;
		
        case 0x13: 
			if (value === 127) {
				if (CMDPL.shift[channel]) {
					engine.setValue(CMDPL.chList[channel], 'hotcue_4_clear', 1);
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'hotcue_4_activate', 1);
			}
			if (value === 0) {
				if (CMDPL.shift[channel]) {
					
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'hotcue_4_activate', 0);
			}
		break;
        
		
        case 0x14: // LOOP ACTIVE
			if (value === 127) {
				if (CMDPL.shift[channel]) {
					
					engine.setValue(CMDPL.chList[channel], 'reloop_toggle', 1);
					break;
				}
				
				if (engine.getValue(CMDPL.chList[channel], 'loop_enabled')) {
					engine.setValue(CMDPL.chList[channel], 'reloop_toggle', 1);
				} else engine.setValue(CMDPL.chList[channel], 'beatloop_activate', 1);
				
				
				//print('BLE: ' + engine.getValue(CMDPL.chList[channel], 'loop_enabled'));
			}
			if (value === 0) {
				//midi.sendShortMsg(0x90 + channel, 0x01, 0x00);
				if (CMDPL.shift[channel]) {
					
					break;
				}
				//
			}
		break;
			
        case 0x15: 
			if (value === 127) {
				if (CMDPL.shift[channel]) {
					engine.setValue(CMDPL.chList[channel], 'loop_start_position', -1);
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'loop_in', 1);
			}
			if (value === 0) {
				if (CMDPL.shift[channel]) {
					
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'loop_in', 0);
			}
		break;
		
        case 0x16: 
			if (value === 127) {
				if (CMDPL.shift[channel]) {
					engine.setValue(CMDPL.chList[channel], 'loop_end_position', -2);
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'loop_out', 1);
			}
			if (value === 0) {
				if (CMDPL.shift[channel]) {
					
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'loop_out', 0);
			}
		break;
		
        case 0x17:
			if (value === 127) {
				if (CMDPL.shift[channel]) {
					// ?
					break;
				}
				script.toggleControl(CMDPL.cGroup, 'quantize');
			}
			if (value === 0) {
				if (CMDPL.shift[channel]) {
					
					break;
				}
				// ?
			}
		break;
			
		case 0x18: // LOAD (shift)
			if (value === 127) {
				midi.sendShortMsg(0x90 + channel, 0x18, 0x00);
				CMDPL.shift[channel] = true;
				
			}
			if (value === 0) {
				if (CMDPL.specialMODE[channel]) midi.sendShortMsg(0x90 + channel, 0x18, 0x02); 
					else midi.sendShortMsg(0x90 + channel, 0x18, 0x01);
				
				CMDPL.shift[channel] = false;
			}
        break;	
		
		case 0x19: // LOCK 
			if (value === 127) {
				if (CMDPL.shift[channel]) {
					// MODE on
					CMDPL.specialMODE[channel] = !CMDPL.specialMODE[channel];
					
					if (CMDPL.specialMODE[channel]) midi.sendShortMsg(0x90 + channel, 0x18, 0x02); 
						else midi.sendShortMsg(0x90 + channel, 0x18, 0x00);
					
					break;
				}
				script.toggleControl(CMDPL.chList[channel], 'keylock', 1);
				
				break;
			}
        break;
		
		case 0x20: // SYNC 
			if (value === 127) {
				if (CMDPL.shift[channel]) {
					// print("RATE: " + engine.getValue(CMDPL.chList[channel], 'rate'));
					engine.setValue(CMDPL.chList[channel], 'rate', 0); 
					// print("RATE: " + engine.getValue(CMDPL.chList[channel], 'rate'));
					break; 
				}
				script.toggleControl(CMDPL.chList[channel], 'sync_enabled', 1);
				CMDPL.syncTimer = engine.beginTimer(500, function() {CMDPL.syncLongPress = true}, true);
			}
			if (value === 0) {
				if (CMDPL.shift[channel]) {
					
					break;
				}
				engine.stopTimer(CMDPL.syncTimer);
				if (CMDPL.syncLongPress === true) {
					CMDPL.syncLongPress = false;
				} else engine.setValue(CMDPL.chList[channel], 'sync_enabled', 0);
			}
        break;
		
		case 0x21: // TAP 
			if (value === 127) {
				if (CMDPL.shift[channel]) {
					engine.setValue(CMDPL.chList[channel], 'bpm_tap', 1);
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'slip_enabled', 1);
			}
			if (value === 0) {
				if (CMDPL.shift[channel]) {
					engine.setValue(CMDPL.chList[channel], 'bpm_tap', 0);
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'slip_enabled', 0);
			}
        break;
        
		// <<
		case 0x24:  
			if (value === 127) {
				midi.sendShortMsg(0x90 + channel, 0x24, 0x01);
				if (CMDPL.shift[channel]) {
					script.toggleControl(CMDPL.chList[channel], 'rate_temp_down');
					break;
				}
				engine.setValue(CMDPL.cGroup, 'beatjump_backward', 1);
			}
			if (value === 0) {
				midi.sendShortMsg(0x90 + channel, 0x24, 0x00);
				if (CMDPL.shift[channel]) {
					script.toggleControl(CMDPL.chList[channel], 'rate_temp_down');
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'beatjump_backward', 0);
			}
        break;
        
		// >>		
        case 0x25: 
			if (value === 127) {
				midi.sendShortMsg(0x90 + channel, 0x25, 0x01);
				if (CMDPL.shift[channel]) {
					script.toggleControl(CMDPL.chList[channel], 'rate_temp_up');
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'beatjump_forward', 1);				
			}
			if (value === 0) {
				midi.sendShortMsg(0x90 + channel, 0x25, 0x00);
				if (CMDPL.shift[channel]) {
					script.toggleControl(CMDPL.chList[channel], 'rate_temp_up');
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'beatjump_forward', 0);
			}
        break;
		
		// -	
		case 0x26:
			if (value === 127) {
				midi.sendShortMsg(0x90 + channel, 0x26, 0x01);
				if (CMDPL.shift[channel]) {
					engine.setValue(CMDPL.chList[channel], 'rate_perm_down', 1);
					break;
				}
				CMDPL.beat_size_pos = (CMDPL.beat_size_pos - 1) <= 0 ? 0 : CMDPL.beat_size_pos -1;
				engine.setValue(CMDPL.chList[channel], 'beatjump_size', CMDPL.beat_size[CMDPL.beat_size_pos]);
			}
			if (value === 0) {
				midi.sendShortMsg(0x90 + channel, 0x26, 0x00);
				if (CMDPL.shift[channel]) {
					engine.setValue(CMDPL.chList[channel], 'rate_perm_down', 0);
					break;
				}
				
			}
		break;
		
		// +
		case 0x27:
			if (value === 127) {
				midi.sendShortMsg(0x90 + channel, 0x27, 0x01);
				if (CMDPL.shift[channel]) {
					engine.setValue(CMDPL.chList[channel], 'rate_perm_up', 1);
					break;
				};
				CMDPL.beat_size_pos = (CMDPL.beat_size_pos + 1) >= (CMDPL.beat_size.length - 1) ? (CMDPL.beat_size.length - 1) : CMDPL.beat_size_pos + 1;
				engine.setValue(CMDPL.chList[channel], 'beatjump_size', CMDPL.beat_size[CMDPL.beat_size_pos]);
			}
			if (value === 0) {
				midi.sendShortMsg(0x90 + channel, 0x27, 0x00);
				if (CMDPL.shift[channel]) {
					engine.setValue(CMDPL.chList[channel], 'rate_perm_up', 0);
					break;
				}
				
			}
		break;
            
        case 0x22: //CUE 
			if (value === 127) {
				if (CMDPL.shift[channel]) {
					engine.setValue(CMDPL.chList[channel], 'cue_gotoandplay', 1);
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'cue_default', 1);
				
			}
			if (value === 0) {
				engine.setValue(CMDPL.chList[channel], 'cue_default', 0);
			}
        break; 
			
        case 0x23: //PLAY 
			if (value === 127) {
				if (CMDPL.shift[channel]) {
					
					engine.setValue(CMDPL.chList[channel], 'start_play', 1);
					break;
				}
				engine.setValue(CMDPL.chList[channel], 'play', !engine.getValue(CMDPL.chList[channel], 'play'));
				
			}
        break; 

        case 0x1B: 
		/*
			var ch = channel + 1;
			if (value === 127) {
				if (CMDPL.permaScratch[channel] === true) {
					CMDPL.permaScratch[channel] = false;
					engine.scratchDisable(ch);
				} else {
					CMDPL.permaScratch[channel] = true;
					engine.scratchEnable(ch, 128, CMDPL.rpm, CMDPL.alpha, CMDPL.beta);
				}
				
			}
		*/
			if (value === 127) {
				CMDPL.fader[channel] = !CMDPL.fader[channel];
				if (CMDPL.fader[channel] === true) {
					engine.setValue("[Mixer Profile]", "xFaderMode", 0);
					engine.setValue("[Mixer Profile]", "xFaderCalibration", 0.9);
					engine.setValue("[Mixer Profile]", "xFaderCurve", 7.0);
					break;
				} else {
					engine.setValue("[Mixer Profile]", "xFaderMode", 0);
					engine.setValue("[Mixer Profile]", "xFaderCalibration", 0.4);
					engine.setValue("[Mixer Profile]", "xFaderCurve", 0.9);
				}
			}
		
		break;

        case 0x1F:
			var ch = channel + 1;
			if (value === 127) { 
				engine.scratchEnable(ch, 128, CMDPL.rpm, CMDPL.alpha, CMDPL.beta, false);
			}
			if (value === 0) {
				if (CMDPL.permaScratch[channel] === true) {
					
				} else engine.scratchDisable(ch);
			}
		break;
		
		case 0x1A: {
			if (value === 127) {
				
			    CMDPL.supr1led.disconnect();    
			    CMDPL.loopsizeled.disconnect(); 
			    CMDPL.qntzled.disconnect(); 
				
				CMDPL.cGroup = CMDPL.chList[channel];
				
				CMDPL.supr1led    = engine.makeConnection('[QuickEffectRack1_' + CMDPL.cGroup + ']', 'super1', CMDPL.super1red); CMDPL.supr1led.trigger();  
				CMDPL.loopsizeled = engine.makeConnection(CMDPL.cGroup, 'loop_enabled', CMDPL.beatszch); CMDPL.loopsizeled.trigger();  
				CMDPL.qntzled     = engine.makeConnection(CMDPL.cGroup, 'quantize', CMDPL.qntz); CMDPL.qntzled.trigger();  
				
				for (i = 0; i < 8; i++) {
					midi.sendShortMsg(0xB0, (0x00 + i), CMDPL.rroundLED[channel][i]);
				}
			}
		}
		break;
		
		default: break;
    }
}

CMDPL.rotary = function (channel, control, value, status, group) {
	// channel always 0 except JOG
	//print(channel);
	
	// Is shift?
	
	
	//print(shift);
	
	switch (control) {
		
		// Rotary 1
		case 0x00: // Super1
			var super1 = Math.round(engine.getValue('[QuickEffectRack1_' + CMDPL.cGroup + ']', 'super1') * 10000); // 0.00001
			var factor = 350;
			
			if (value == 0x41) {
				super1 = (super1 + factor) / 10000;
				engine.setValue('[QuickEffectRack1_' + CMDPL.cGroup + ']', 'super1', super1);

			} else if (value == 0x3F) {
				super1 = (super1 - factor) / 10000;
				engine.setValue('[QuickEffectRack1_' + CMDPL.cGroup + ']', 'super1', super1);

			}
			
			
		break;
		
		// Rotary 2
		case 0x01: // PITCH
			var pitch = Math.round(engine.getValue(CMDPL.cGroup, 'pitch')); // 0.00001
			if (value == 0x41) {
				pitch++;
				engine.setValue(CMDPL.cGroup, 'pitch', pitch);
				
				midi.sendShortMsg(0xB0, control, ++CMDPL.rroundLED[CMDPL.getChannelN(CMDPL.cGroup)][control]);
			} else if (value == 0x3F) {
				pitch--;
				engine.setValue(CMDPL.cGroup, 'pitch', pitch);
				
				midi.sendShortMsg(0xB0, control, --CMDPL.rroundLED[CMDPL.getChannelN(CMDPL.cGroup)][control]);
			}
			

		break;
		
		// Rotary 3
		case 0x02: // Adjust BPM 0.01 
			if (value === 0x41) {
				engine.setValue(CMDPL.cGroup, 'beats_adjust_faster', 1);
			} else if (value === 0x3f) {
				engine.setValue(CMDPL.cGroup, 'beats_adjust_slower', 1);
			}
		break;
		
		// Rotary 4
		case 0x03: // MOVE GRID
			
			if (value === 0x41) {
				engine.setValue(CMDPL.cGroup, 'beats_translate_later', 1);
			} else if (value === 0x3f) {
				engine.setValue(CMDPL.cGroup, 'beats_translate_earlier', 1);
			}
		break;
		
		// Rotary 5
		case 0x04: // 
			var shift = CMDPL.shift[CMDPL.getChannelN(CMDPL.cGroup)];
			var channel = CMDPL.getChannelN(CMDPL.cGroup);
			
			if (value === 0x41) {
				if (CMDPL.specialMODE[channel]) {
					//hotcue_X_position
					engine.setValue(CMDPL.cGroup, 'hotcue_1_position', (engine.getValue(CMDPL.cGroup, 'hotcue_1_position') + 200));
					break;
				}
				
				if (shift) {
					CMDPL.beat_size_pos = (CMDPL.beat_size_pos + 1) >= (CMDPL.beat_size.length - 1) ? (CMDPL.beat_size.length - 1) : CMDPL.beat_size_pos + 1;
					engine.setValue(CMDPL.cGroup, 'beatjump_size', CMDPL.beat_size[CMDPL.beat_size_pos]);
					break;
				}
				CMDPL.loop_size_pos = (CMDPL.loop_size_pos + 1) >= CMDPL.loop_size.length ? (CMDPL.loop_size.length - 1) : CMDPL.loop_size_pos +1;
				engine.setValue(CMDPL.cGroup, 'beatloop_size', CMDPL.loop_size[CMDPL.loop_size_pos]);
			} else if (value === 0x3f) {
				if (CMDPL.specialMODE[channel]) {
					//hotcue_X_position
					engine.setValue(CMDPL.cGroup, 'hotcue_1_position', (engine.getValue(CMDPL.cGroup, 'hotcue_1_position') - 200));
					break;
				}
				
				if (shift) {
					CMDPL.beat_size_pos = (CMDPL.beat_size_pos - 1) <= 0 ? 0 : CMDPL.beat_size_pos -1;
					engine.setValue(CMDPL.cGroup, 'beatjump_size', CMDPL.beat_size[CMDPL.beat_size_pos]);
					break;
				}
				CMDPL.loop_size_pos = (CMDPL.loop_size_pos - 1) <= 0 ? 0 : CMDPL.loop_size_pos -1;
				engine.setValue(CMDPL.cGroup, 'beatloop_size', CMDPL.loop_size[CMDPL.loop_size_pos]);
			}
		break;
		
		// Rotary 6
		case 0x05: // loop_start_position
			var shift = CMDPL.shift[CMDPL.getChannelN(CMDPL.cGroup)];
			var channel = CMDPL.getChannelN(CMDPL.cGroup);
			
			if (!CMDPL.specialMODE[channel]) {
				var pos = engine.getValue(CMDPL.cGroup, 'loop_start_position');
				if (shift) {
					var tik = 250;
				} else tik = 1000;
			}	
			

			if (value === 0x41) {
				if (CMDPL.specialMODE[channel]) {
					//hotcue_X_position
					engine.setValue(CMDPL.cGroup, 'hotcue_2_position', (engine.getValue(CMDPL.cGroup, 'hotcue_2_position') + 200));
					break;
				}
				
				engine.setValue(CMDPL.cGroup, 'loop_start_position', pos + tik);
			} else if (value === 0x3f) {
				if (CMDPL.specialMODE[channel]) {
					//hotcue_X_position
					engine.setValue(CMDPL.cGroup, 'hotcue_2_position', (engine.getValue(CMDPL.cGroup, 'hotcue_2_position') - 200));
					break;
				}
				
				engine.setValue(CMDPL.cGroup, 'loop_start_position', pos - tik);
			}
		break;
		
		// Rotary 7
		case 0x06: // loop_end_position
			var shift = CMDPL.shift[CMDPL.getChannelN(CMDPL.cGroup)];
			var channel = CMDPL.getChannelN(CMDPL.cGroup);
		
			if (!CMDPL.specialMODE[channel]) {
				var pos = engine.getValue(CMDPL.cGroup, 'loop_end_position');
				if (shift) {
					var tik = 250;
				} else tik = 500;
			}
			

			if (value === 0x41) {
				if (CMDPL.specialMODE[channel]) {
					//hotcue_X_position
					engine.setValue(CMDPL.cGroup, 'hotcue_3_position', (engine.getValue(CMDPL.cGroup, 'hotcue_3_position') + 200));
					break;
				}
				
				engine.setValue(CMDPL.cGroup, 'loop_end_position', pos + tik);
			} else if (value === 0x3f) {
				if (CMDPL.specialMODE[channel]) {
					//hotcue_X_position
					engine.setValue(CMDPL.cGroup, 'hotcue_3_position', (engine.getValue(CMDPL.cGroup, 'hotcue_3_position') - 200));
					break;
				}
				
				engine.setValue(CMDPL.cGroup, 'loop_end_position', pos - tik);
			}
		break;	
		
		// Rotary 8
		case 0x07: // BPM +/- 1 (Implement bpm round)
			var channel = CMDPL.getChannelN(CMDPL.cGroup);
			var bpm = Math.round(engine.getValue(CMDPL.cGroup, 'bpm'));
			
			if (value === 0x41) {
				if (CMDPL.specialMODE[channel]) {
					//hotcue_X_position
					engine.setValue(CMDPL.cGroup, 'hotcue_4_position', (engine.getValue(CMDPL.cGroup, 'hotcue_4_position') + 200));
					break;
				}
				
				bpm++;
				engine.setValue(CMDPL.cGroup, 'bpm', bpm);
				midi.sendShortMsg(0xB0, control, ++CMDPL.rroundLED[channel][control]);
				
			} else if (value === 0x3f) {
				if (CMDPL.specialMODE[channel]) {
					//hotcue_X_position
					engine.setValue(CMDPL.cGroup, 'hotcue_4_position', (engine.getValue(CMDPL.cGroup, 'hotcue_4_position') - 200));
					break;
				}
				
				bpm--;
				engine.setValue(CMDPL.cGroup, 'bpm', bpm);
				midi.sendShortMsg(0xB0, control, --CMDPL.rroundLED[channel][control]);
			}
		break;	
		
		// JOGss
		case 0x1F: // 
			var ch = channel + 1;
			var newValue = value - 64;
			if (CMDPL.invertJogs) {
				switch (true) {
					case newValue > 0 : newValue = 0 - newValue; break;
					case newValue < 0 : newValue = Math.abs(newValue) ; break;
				}
			}
			if (engine.isScratching(ch)) {
				engine.scratchTick(ch, newValue); // Scratch!
 			} else {
				engine.setValue(CMDPL.chList[channel], 'jog', newValue); // Pitch bend
			}
		break;	
	}
}


CMDPL.fader = function (channel, control, value, status, group) {
	engine.softTakeover(CMDPL.chList[channel], 'rate', true);
	engine.setValue(CMDPL.chList[channel], 'rate', script.midiPitch(control, value, status));
}


CMDPL.getChannelN = function(value) {
    switch (value) {
        case '[Channel1]': return 0;
        case '[Channel2]': return 1;
        case '[Channel3]': return 2;
        case '[Channel4]': return 3;
        default: print("Line: 726");
    }
}

CMDPL.eqLed = function (value, group, control) {
    value = (((value - 0) * (15 - 1)) / (4 - 0)) + 1
    midi.sendShortMsg(0xB0 + CMDPL.getChannelN(group), 0x0A, value);
}

CMDPL.rateLED = function (value, group, control) {
    value = script.absoluteLin(value, 1, 15, -1, 1);
    midi.sendShortMsg(0xB0 + CMDPL.getChannelN(group), 0x0A, value);
}

 
/*
CMDPL.LEDfromLoopLength = function (length) {
    var LED;
    //print(pitch);
    switch (length) {
        case 64: LED = 0x0D; break;
        case 32: LED = 0x0C; break;
        case 16: LED = 0x0B; break;
        case 8: LED = 0x0A; break;
        case 4: LED = 0x09; break;
        case 2: LED = 0x08; break;
        case 1: LED = 0x07; break;
        case 0.5: LED = 0x06; break;
        case 0.25: LED = 0x05; break;
        case 0.125: LED = 0x04; break;
        case 0.0625: LED = 0x03; break;
        case 0.03125: LED = 0x02; break;
        default: LED = 0x04;
    }
    return LED;
}

CMDPL.beatloop = function (channel, control, value, status, group) {
    if (status == 0x90) {
        if (!CMDPL.g.loopIsActive) {
            engine.setValue(CMDPL.cGroup, 'beatloop_' + CMDPL.g.loopLength + '_activate', 1);
            CMDPL.g.loopIsActive = true;
            midi.sendShortMsg(0x90, control, 0x01);
            print('beatlooproll 1');
        } else {
            engine.setValue(CMDPL.cGroup, 'reloop_exit', 1);
            CMDPL.g.loopIsActive = false;
            midi.sendShortMsg(0x90, control, 0x00);
            
            CMDPL.g.loopLeftLocatorIsActive  = false;
            CMDPL.g.loopRightLocatorIsActive = false;
        engine.setValue(CMDPL.cGroup, 'loop_in', 0);
        engine.setValue(CMDPL.cGroup, 'loop_out', 0);
            midi.sendShortMsg(0x90, ++control, 0x00);
        }
    }
     
    CMDPL.loopLengthFunc(channel, control, value, status, group);
}

CMDPL.loopinout = function (channel, control, value, status, group) {
    if (status == 0x90) {
        if (!CMDPL.g.loopLeftLocatorIsActive && !CMDPL.g.loopIsActive) {
            engine.setValue(CMDPL.cGroup, 'loop_in', 1);
            
            CMDPL.g.loopLeftLocatorIsActive = true;
        } else {
            engine.setValue(CMDPL.cGroup, 'loop_out', 1);
            
            CMDPL.g.loopRightLocatorIsActive = true;
            midi.sendShortMsg(0x90, control, 0x01);
        }
    }
    
    if (CMDPL.g.loopLeftLocatorIsActive && CMDPL.g.loopRightLocatorIsActive) {
        CMDPL.g.loopIsActive = true;
        engine.setValue(CMDPL.cGroup, 'loop_in', 0);
        engine.setValue(CMDPL.cGroup, 'loop_out', 0);
    }
    
    
    CMDPL.loopLengthFunc(channel, control, value, status, group);
}

*/


CMDPL.shutdown = function() {
    
	// Turn off lights
	for (var c = 0; c <= 3; c++) {
        for (var i = 0; i <= 0x30; i++) {
            midi.sendShortMsg(0x90 + c, i, 0x00); 
            midi.sendShortMsg(0xB0 + c, i, 0x00);  
		} 
    }
}