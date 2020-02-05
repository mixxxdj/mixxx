var CMDDC = new Object();

CMDDC.padLigtsUpdate = function() {
	for (ctrl = 0x24, iter = 1; ctrl <= 0x33; ctrl++, iter++) {
		if (engine.getValue('[Sampler' + iter + ']', 'track_loaded')) {
			if (engine.getValue('[Sampler' + iter + ']', 'play')) {
				midi.sendShortMsg(0x95, ctrl, 0x02);
			} else midi.sendShortMsg(0x95, ctrl, 0x01);
			// print("Sample Loaded");
		} else midi.sendShortMsg(0x95, ctrl, 0x00);
	}
};

CMDDC.padMakeConnection = function (ctrl, iter) {
	engine.makeConnection('[Sampler' + iter + ']', 'play', function(value) {
		if (engine.getValue('[Sampler' + iter + ']', 'track_loaded') === 1) {
			if (value === 0) value = 1; else value = 2;
		} else {
			if (value === 0) value = 0; else value = 2; 
		}
		midi.sendShortMsg(0x95, ctrl, value);
	}); 
	
	engine.makeConnection('[Sampler' + iter + ']', 'eject', function(value) {
		midi.sendShortMsg(0x95, ctrl, value);
	});
	
	engine.makeConnection('[Sampler' + iter + ']', 'track_loaded', function(value) {
		midi.sendShortMsg(0x95, ctrl, value);
	});
}

CMDDC.chVolumeLed = function (num, value) {
	
	// var i = num - 1;
	// var ctrl = [0x14, 0x15, 0x16, 0x17, 0x10, 0x11, 0x12, 0x13];
	
	// var sig = engine.getValue('[Sampler'+num+']', 'pregain');
	// var led = Math.round(script.absoluteNonLinInverse(sig, 0, 1, 4, 1, 16));

	// midi.sendShortMsg(0xB5, ctrl[i], led);
	// if (CMDDC.tmp1) {CMDDC.light[1][i] = led} else CMDDC.light[0][i] = led; 
	// print(sig);
	// print(led);
}

CMDDC.gainLED = function (group) {
	var r = Math.round(script.absoluteNonLinInverse(engine.getValue(group, 'pregain'), 0, 1, 4, 1, 16));
	return(r);
}
 
CMDDC.init = function (id) { 
	
	// Get pregain light values
	for (i = 1; i <= 8; i++) {
		midi.sendShortMsg(0xB5, 0x0F + i, CMDDC.gainLED('[Sampler'+i+']'));
	}
	
	engine.makeConnection('[Sampler1]', 'pregain', function(value, group) {midi.sendShortMsg(0xB5, 0x14, CMDDC.gainLED(group))});
	engine.makeConnection('[Sampler2]', 'pregain', function(value, group) {midi.sendShortMsg(0xB5, 0x15, CMDDC.gainLED(group))});
	engine.makeConnection('[Sampler3]', 'pregain', function(value, group) {midi.sendShortMsg(0xB5, 0x16, CMDDC.gainLED(group))});
	engine.makeConnection('[Sampler4]', 'pregain', function(value, group) {midi.sendShortMsg(0xB5, 0x17, CMDDC.gainLED(group))});
	engine.makeConnection('[Sampler5]', 'pregain', function(value, group) {midi.sendShortMsg(0xB5, 0x10, CMDDC.gainLED(group))});
	engine.makeConnection('[Sampler6]', 'pregain', function(value, group) {midi.sendShortMsg(0xB5, 0x11, CMDDC.gainLED(group))});
	engine.makeConnection('[Sampler7]', 'pregain', function(value, group) {midi.sendShortMsg(0xB5, 0x12, CMDDC.gainLED(group))});
	engine.makeConnection('[Sampler8]', 'pregain', function(value, group) {midi.sendShortMsg(0xB5, 0x13, CMDDC.gainLED(group))});
	
	engine.makeConnection('[Recording]', 'status', function(value) {midi.sendShortMsg(0x95, 0x04, value)});
	
	// Reset Light
	for (i = 0; i < 0x35; i++) {midi.sendShortMsg(0x95, i, 0x00)}
	
	for (ctrl = 0x24, iter = 1; ctrl <= 0x33; ctrl++, iter++) {
		if (engine.getValue('[Sampler' + iter + ']', 'track_loaded')) {
			midi.sendShortMsg(0x95, ctrl, 0x01);
			// print("Sample Loaded");
		} else midi.sendShortMsg(0x95, ctrl, 0x00);
		
		CMDDC.padMakeConnection(ctrl, iter);
	}
	
	 
	CMDDC.shift1 = false; // Hold to play Y/N, default is Don't hold
	CMDDC.shift2 = false; // Sync On/Off
	CMDDC.shift3 = false; // Loop Y/N
	CMDDC.shift4 = false; // PFL
	CMDDC.shift5 = false; // Stop
	CMDDC.shift6 = false; // Play
	CMDDC.orientation = false; // Orientation
	
	CMDDC.shift8 = false; // hot cue w pitch
	CMDDC.bpmPitch = false;
	CMDDC.sampler = 1;
	
	CMDDC.shift9 = false; // Load to sampler Track 9-18
	
	
	CMDDC.tmp1 = 0; // VOL Rot shift, Line 193
	//CMDDC.tmp2 = engine.getValue('[Samplers]', 'show_samplers');   // show samples
	CMDDC.tmp3 = false ;   // sampleRow1
	
	
	CMDDC.memoFive   = [0, 0, 0, 0, 
						0, 0, 0, 0, 
						0, 0, 0, 0, 
						0, 0, 0, 0];
	CMDDC.memoOne   = [0, 0, 0, 0, 
					   0, 0, 0, 0, 
					   0, 0, 0, 0, 
					   0, 0, 0, 0];
	CMDDC.memoTwo   = [0, 0, 0, 0, 
					   0, 0, 0, 0, 
					   0, 0, 0, 0, 
					   0, 0, 0, 0];
	CMDDC.memoSeven   = [1, 1, 1, 1, 
						 1, 1, 1, 1, 
						 1, 1, 1, 1, 
						 1, 1, 1, 1];
						  
	CMDDC.memoThree = [];
	
	CMDDC.LoadMode     = false; // SHIFT
	CMDDC.playRow = [false, false, false, false, false, false, false, false];
	
	CMDDC.whatPlay = null;
	
	for (i = 0; i < 16; i++) {
		CMDDC.memoThree[i] = engine.getValue('[Sampler' + (i + 1) + ']', 'repeat');
		//print(CMDDC.memoThree[i]);
	}
						  
	CMDDC.memoFour = [];
	for (i = 0; i < 16; i++) {
		CMDDC.memoFour[i] = engine.getValue('[Sampler' + (i + 1) + ']', 'pfl');
	}
};

CMDDC.button = function(channel, control, value, status, group) {
	
	switch (control) {
		case 0x00: 
			if (value === 127) {if (CMDDC.LoadMode) break; engine.setValue('[Library]', 'AutoDjAddTop', 1)}  
			
		break;
		
		case 0x01: 
			if (value === 127) {
				if (CMDDC.LoadMode) {break;}
				engine.setValue('[Channel1]', 'CloneFromDeck', 2);
			}  
		break;
		case 0x02: 
			if (value === 127) {
				if (CMDDC.LoadMode) {break;}
				engine.setValue('[Channel2]', 'CloneFromDeck', 1);
			}  
		break;
		case 0x03: 
			if (value === 127) {
				if (CMDDC.LoadMode) {
					break;
				}
				//engine.setValue('[Channel1]', 'pitch', 0);
				script.toggleControl('[Samplers]', 'show_samplers');
			}  
		break;
		
		case 0x04: 
			if (value === 127) {
				if (CMDDC.LoadMode) {
					engine.setValue('[Recording]', 'toggle_recording', 1);
					break;
				}
				//if (CMDDC.shift1) {CMDDC.bpmPitch = !CMDDC.bpmPitch; midi.sendShortMsg(0x95, 0x07, 0x02); break;}
				CMDDC.shift8 = !CMDDC.shift8;
				CMDDC.sampler = 1;
				if (CMDDC.shift8) {
					midi.sendShortMsg(0x95, 0x04, 0x01);
					for (i = 0x24; i < (0x24 + 16); i++) {
						midi.sendShortMsg(0x95, i, 0x00);
						if (i == 0x28) midi.sendShortMsg(0x95, 0x28, 0x01);  
						if (i == 0x2F) midi.sendShortMsg(0x95, 0x2F, 0x01);  
					}
				} else {
					midi.sendShortMsg(0x95, 0x04, 0x00); 
					midi.sendShortMsg(0x95, 0x07, 0x00);
					CMDDC.padLigtsUpdate();
				}
				
			}  
		break;
					
				
		case 0x05: if (value === 127) {if (CMDDC.LoadMode) break; engine.setValue('[Channel1]', 'LoadSelectedTrack', 1)}  break;
		case 0x06: if (value === 127) {if (CMDDC.LoadMode) break; engine.setValue('[Channel2]', 'LoadSelectedTrack', 1)}  break;
		
		case 0x07: 
			if (value === 127) {
				if (CMDDC.LoadMode) {engine.setValue('[Sampler]', 'LoadSamplerBank', 1); break;}
				//if (CMDDC.shift1) {CMDDC.bpmPitch = !CMDDC.bpmPitch; midi.sendShortMsg(0x95, 0x07, 0x02); break;}
				CMDDC.shift8 = !CMDDC.shift8;
				CMDDC.sampler = 2;
				if (CMDDC.shift8) {
					midi.sendShortMsg(0x95, 0x07, 0x01);
					for (i = 0x24; i < (0x24 + 16); i++) {
						midi.sendShortMsg(0x95, i, 0x00);
						if (i == 0x28) midi.sendShortMsg(0x95, 0x28, 0x01);  
						if (i == 0x2F) midi.sendShortMsg(0x95, 0x2F, 0x01);  
					}
				} else {
					midi.sendShortMsg(0x95, 0x04, 0x00); 
					midi.sendShortMsg(0x95, 0x07, 0x00);
					CMDDC.padLigtsUpdate();
				}
			};  
		break;
		
		case 0x20: // Middle Rotary
			if (value === 127) {
				if (CMDDC.LoadMode === true) {
					for (i = 0; i <= 0x33; i++) {if (i === 0x03 || i === 0x04) continue; midi.sendShortMsg(0x95, i, 0x00);}
					CMDDC.LoadMode = false;
					CMDDC.padLigtsUpdate();
					CMDDC.tmp1 = 0;
				} else {
					CMDDC.LoadMode = true;
					for (i = 0; i <= 0x33; i++) {if (i === 0x03 || i === 0x04) continue; midi.sendShortMsg(0x95, i, 0x01);}
				}
			};
		break;
		
		case 0x24: CMDDC.pad (0,  value); break;
		case 0x25: CMDDC.pad (1,  value); break;
		case 0x26: CMDDC.pad (2,  value); break;
		case 0x27: CMDDC.pad (3,  value); break;
		case 0x28: CMDDC.pad (4,  value); break;
		case 0x29: CMDDC.pad (5,  value); break;
		case 0x2a: CMDDC.pad (6,  value); break;
		case 0x2b: CMDDC.pad (7,  value); break;
		case 0x2c: CMDDC.pad (8,  value); break;
		case 0x2d: CMDDC.pad (9,  value); break;
		case 0x2e: CMDDC.pad (10, value); break;
		case 0x2f: CMDDC.pad (11, value); break;
		case 0x30: CMDDC.pad (12, value); break;
		case 0x31: CMDDC.pad (13, value); break;
		case 0x32: CMDDC.pad (14, value); break;
		case 0x33: CMDDC.pad (15, value); break;
		

		
		case 0x10: 
			if (value === 127) {
				if (CMDDC.LoadMode) break; 
				CMDDC.shift1 = true;
				midi.sendShortMsg(0x95, 0x10, 0x01);
				for (i = 36, a = 0; i < (36 + 16); i++, a++) {
					
					if (CMDDC.memoOne[a] === 1) {
							midi.sendShortMsg(0x95, i, 0x01);
							print("ok");
						} else midi.sendShortMsg(0x95, i, 0x00);
				}
			}

			if (value === 0) {
				if (CMDDC.LoadMode) break; 
				CMDDC.shift1 = false;
				midi.sendShortMsg(0x95, 0x10, 0x00);
				CMDDC.padLigtsUpdate();
			};
			break;
		
		case 0x11:
			if (value === 127) {
				if (CMDDC.LoadMode) break; 
				CMDDC.shift3 = true;
				midi.sendShortMsg(0x95, 0x11, 0x01);
				for (i = 36, a = 0; i < (36 + 16); i++, a++) {
					
					if (CMDDC.memoThree[a] === 1) {
							midi.sendShortMsg(0x95, i, 0x01);
							print("ok");
						} else midi.sendShortMsg(0x95, i, 0x00);
				}
			}

			if (value === 0) {
				if (CMDDC.LoadMode) break; 
				CMDDC.shift3 = false;
				midi.sendShortMsg(0x95, 0x11, 0x00);
				CMDDC.padLigtsUpdate();
			}
			break;
			
		case 0x12:
			if (value === 127) {
				if (CMDDC.LoadMode) {for (i = 1; i < 64; i++) {engine.setValue('[Sampler' + i + ']','eject', 1)}; break;}
				CMDDC.shift2 = true;
				midi.sendShortMsg(0x95, 0x12, 0x01);
				for (i = 36, a = 0; i < (36 + 16); i++, a++) {
					
					if (CMDDC.memoTwo[a] === 1) {
							midi.sendShortMsg(0x95, i, 0x01);
							print("ok");
						} else midi.sendShortMsg(0x95, i, 0x00);
				}
			}

			if (value === 0) {
				if (CMDDC.LoadMode) {for (i = 1; i < 64; i++) {engine.setValue('[Sampler' + i + ']','eject', 0)}; break;}
				CMDDC.shift2 = false;
				midi.sendShortMsg(0x95, 0x12, 0x00);
				CMDDC.padLigtsUpdate();
			}
		break;
			
		case 0x13:
			if (value === 127) {
				//if (CMDDC.LoadMode) break; 
				CMDDC.shift4 = !CMDDC.shift4;
				if (CMDDC.shift4) midi.sendShortMsg(0x95, 0x13, 0x01); else midi.sendShortMsg(0x95, 0x13, 0x00);
				/*
				midi.sendShortMsg(0x95, 0x13, 0x01);
				for (i = 36, a = 0; i < (36 + 16); i++, a++) {
					
					if (CMDDC.memoFour[a] === 1) {
							midi.sendShortMsg(0x95, i, 0x01);
							print("ok");
						} else midi.sendShortMsg(0x95, i, 0x00);
				}
				*/

			}

			if (value === 0) {
				//if (CMDDC.LoadMode) break; 
				//CMDDC.shift4 = false;
				//CMDDC.padLigtsUpdate();
				//midi.sendShortMsg(0x95, 0x13, 0x00);
			}
		break;
			
		case 0x14:
			if (value === 127) {
				if (CMDDC.LoadMode) break; 
				CMDDC.shift5 = true;
				midi.sendShortMsg(0x95, 0x14, 0x01);

			}

			if (value === 0) {
				if (CMDDC.LoadMode) break; 
				CMDDC.shift5 = false;
				midi.sendShortMsg(0x95, 0x14, 0x00);

			}
		break;
			
		case 0x15:
			if (value === 127) {
				if (CMDDC.LoadMode) break; 
				//CMDDC.shift6 = true;
				midi.sendShortMsg(0x95, 0x15, 0x01);
				//engine.setValue('[Sampler'+CMDDC.whatPlay+']', 'cue_default', 0);
				engine.setValue('[Sampler'+CMDDC.whatPlay+']', 'stop', 1);
				print("you are here");
			}

			if (value === 0) {
				if (CMDDC.LoadMode) break; 
				midi.sendShortMsg(0x95, 0x15, 0x00);

			}
			break;
			
		case 0x16:
			if (value === 127) {
				if (CMDDC.LoadMode) break; 
				CMDDC.orientation = true;
				midi.sendShortMsg(0x95, 0x16, 0x01);
				for (i = 36, a = 0; i < (36 + 16); i++, a++) {
					
					if (CMDDC.memoSeven[a] === 1) {
							midi.sendShortMsg(0x95, i, 0x00);
							print("ok");
						} else midi.sendShortMsg(0x95, i, 0x01);
				}

			}

			if (value === 0) {
				if (CMDDC.LoadMode) break; 
				CMDDC.orientation = false;
				CMDDC.padLigtsUpdate();
				midi.sendShortMsg(0x95, 0x16, 0x00);
			}
			break;
			
		case 0x17:
			if (value === 127) {
				if (CMDDC.LoadMode) break; 
				if (CMDDC.tmp1) {
					CMDDC.tmp1 = 0;
					midi.sendShortMsg(0x95, 0x17, 0x00);

				} else {
					CMDDC.tmp1 = 8;
					midi.sendShortMsg(0x95, 0x17, 0x01);
				}
				
				CMDDC.bpmPitch = !CMDDC.bpmPitch;
				
			}
			break;
	}
}


CMDDC.rotary = function(channel, control, value, status, group) {
	//print("fn.rotary");
	
	switch (control) {
		case 0x10: CMDDC.rtry(5, value); break;
		case 0x11: CMDDC.rtry(6, value); break;
		case 0x12: CMDDC.rtry(7, value); break;
		case 0x13: CMDDC.rtry(8, value); break;
		case 0x14: CMDDC.rtry(1, value); break;
		case 0x15: CMDDC.rtry(2, value); break;
		case 0x16: CMDDC.rtry(3, value); break;
		case 0x17: CMDDC.rtry(4, value); break;
		
		case 0x20: 
			if (value === 0x41) {
				engine.setValue('[Playlist]', 'SelectNextTrack', 1);
				break;
			}
			if (value === 0x3F) {
				engine.setValue('[Playlist]', 'SelectPrevTrack', 1);
				break;
			}
	}
	
	
}

CMDDC.pad = function(num, value) {
	if (CMDDC.shift8 === true) {
		
		var scale = [[-7, -5, -3, -1, 0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19], [-7, -5, -4, -2, 0, 2, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19]];
		var pitch;
			
		if (!CMDDC.bpmPitch) {
			engine.setValue('[Channel'+CMDDC.sampler+']', 'pitch_adjust', scale[0][num]);
		} else {
			
			//Method 2
			var bpm = engine.getValue('[Channel'+CMDDC.sampler+']', 'file_bpm') ? engine.getValue('[Channel'+CMDDC.sampler+']', 'file_bpm') : 100;
			switch (num) { // Maj BPM
				case 0:  bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 1:  bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 2:  bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 3:  bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 4:  bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 5:  bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 6:  bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 7:  bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 8:  bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 9:  bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 10: bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 11: bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 12: bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 13: bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 14: bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
				case 15: bpm = bpm * Math.pow(2, scale[0][num] / 12); break;
			}
			engine.setValue('[Channel'+CMDDC.sampler+']', 'bpm', bpm);
		}
			
			
			
			
			// Method 3
			//engine.setValue('[Channel'+ch+']', 'rateRange', 1);
			// switch (num) { // Maj => rate
				// case 0:  rate = -0.3326; break; //-7 -33.26
				// case 1:  rate = -0.2508; break; //-5 -25.08
				// case 2:  rate = -0.1591; break; //-3 -15.91
				// case 3:  rate = -0.0561; break; //-1 -5.61
				// case 4:  rate = 0;       break; // 0 0
				// case 5:  rate = 0.1225;  break; // 2 12.25
				// case 6:  rate = 0.2599;  break; // 4 25.99
				// case 7:  rate = 0.3348;  break; // 5 33.48
				// case 8:  rate = 0.4983;  break; // 7 49.83
				// case 9:  rate = 0.6818;  break; // 9 68.18
				// case 10: rate = 0.8877;  break; // 11 88.77
				// case 11: rate = 1;       break; // 12 100
				// case 12: rate = 1.2449;  break; // 14 124.49
				// case 13: rate = 1.5198;  break; // 16 151.98
				// case 14: rate = 1.6697;  break; // 17 166.97
				// case 15: rate = 1.9966;  break; // 19 199.66
			// }
			//engine.setValue('[Channel'+ch+']', 'rate', rate);
		
		if (value === 127) {
			//engine.setValue('[Channel'+ch+']', 'hotcue_4_activate', 0);
			engine.setValue('[Channel'+CMDDC.sampler+']', 'cue_default', 1);
		}
		if (value === 0) {
			engine.setValue('[Channel'+CMDDC.sampler+']', 'cue_default', 0);
		}
		
		return;
	}
	
	if (value === 127) {
		if (CMDDC.shift1 === true) { // oneshot
			if (CMDDC.memoOne[num] === 0) {
				CMDDC.memoOne[num] = 1;
				midi.sendShortMsg(0x95, ( 0x24 + num), 0x01);
				return;
			}
			if (CMDDC.memoOne[num] === 1) {
				CMDDC.memoOne[num] = 0;
				midi.sendShortMsg(0x95, ( 0x24 + num), 0x00);
				return;
			}
			
		}
		if (CMDDC.shift2 === true) { // sync
			if (CMDDC.memoTwo[num] === 0) {
				CMDDC.memoTwo[num] = 1;
				engine.setValue('[Sampler' + (num + 1) + ']', 'sync_enabled', 1);
				midi.sendShortMsg(0x95, ( 0x24 + num), 0x01);
				return;
			}
			if (CMDDC.memoTwo[num] === 1) {
				CMDDC.memoTwo[num] = 0;
				engine.setValue('[Sampler' + (num + 1) + ']', 'sync_enabled', 0);
				engine.setValue('[Sampler' + (num + 1) + ']', 'rate', 0);
				engine.setValue('[Sampler' + (num + 1) + ']', 'pitch', 0);
				midi.sendShortMsg(0x95, ( 0x24 + num), 0x00);
				return;
			}
			
		}
		if (CMDDC.shift3 === true) {
			if (CMDDC.memoThree[num] === 0) {
				CMDDC.memoThree[num] = 1;
				engine.setValue('[Sampler' + (num + 1) + ']', 'repeat', 1);
				midi.sendShortMsg(0x95, ( 0x24 + num), 0x01);
				return;
			}
			if (CMDDC.memoThree[num] === 1) {
				CMDDC.memoThree[num] = 0;
				engine.setValue('[Sampler' + (num + 1) + ']', 'repeat', 0);
				midi.sendShortMsg(0x95, ( 0x24 + num), 0x00);
				return;
			}
			
		}
		/*
		if (CMDDC.shift4 === true) {
			if (CMDDC.memoFour[num] === 0) {
				CMDDC.memoFour[num] = 1;
				engine.setValue('[Sampler' + (num + 1) + ']', 'pfl', 1);
				midi.sendShortMsg(0x95, ( 0x24 + num), 0x01);
				return;
			}
			if (CMDDC.memoFour[num] === 1) {
				CMDDC.memoFour[num] = 0;
				engine.setValue('[Sampler' + (num + 1) + ']', 'pfl', 0);
				midi.sendShortMsg(0x95, ( 0x24 + num), 0x00);
				return;
			}
			
		}
		*/
		if (CMDDC.orientation === true) {
			if (CMDDC.memoSeven[num] === 0) {
				CMDDC.memoSeven[num] = 1;
				engine.setValue('[Sampler' + (num + 1) + ']', 'orientation', 1);
				midi.sendShortMsg(0x95, ( 0x24 + num), 0x00);
				return;
			}
			if (CMDDC.memoSeven[num] === 1) {
				CMDDC.memoSeven[num] = 2;
				engine.setValue('[Sampler' + (num + 1) + ']', 'orientation', 2);
				midi.sendShortMsg(0x95, ( 0x24 + num), 0x01);
				return;
			}
			if (CMDDC.memoSeven[num] === 2) {
				CMDDC.memoSeven[num] = 0;
				engine.setValue('[Sampler' + (num + 1) + ']', 'orientation', 0);
				midi.sendShortMsg(0x95, ( 0x24 + num), 0x02);
				return;
			}
			
		}
		if (CMDDC.shift5 === true) {
			engine.setValue('[Sampler' + (num + 1) + ']', 'stop', 1);
			return;
		}
		
		if (CMDDC.LoadMode === true) {
			engine.setValue('[Sampler' + (num + 1) + ']', 'LoadSelectedTrack', 1);
			return;
		}
		
		// Unfinished
		if (CMDDC.shift6 === true) {
			engine.setValue('[Sampler' + (num + 1) + ']', 'play', 1);
			return;
		}
		
		if (false) {
			switch (num) {
				case 0:  CMDDC.plyRow(0, [1, 2, 3, 4]); break;
				case 4:  CMDDC.plyRow(1, [5, 6, 7, 8]); break;
				case 8:  CMDDC.plyRow(2, [9, 10, 11, 12]); break;
				case 12: CMDDC.plyRow(3, [13, 14, 15, 16]); break;
				case 1: CMDDC.plyRow(4, [2, 6, 10, 14]); break;
				case 2: CMDDC.plyRow(5, [3, 7, 11, 15]); break;
				case 3: CMDDC.plyRow(6, [4, 8, 12, 16]); break;
			}
			
			return;
		}
	}
	
	if (CMDDC.memoOne[num] === 1) { // Shifted
		if (value === 127) {
			engine.setValue('[Sampler' + (num + 1) + ']', 'cue_default', 1);
			CMDDC.memoFive[num] = 1;
			
			CMDDC.whatPlay = (num + 1);
			//print("");
		}
		if (value === 0) {
			engine.setValue('[Sampler' + (num + 1) + ']', 'cue_default', 0);
			if (engine.getValue('[Sampler' + (num + 1) + ']', 'play') !== 1) {
				engine.setValue('[Sampler' + (num + 1) + ']', 'cue_gotoandstop', 1);
				
			}
			CMDDC.whatPlay = null;
			CMDDC.memoFive[num] = 0;
			//print("");
		}
	}
	if (CMDDC.memoOne[num] === 0) { // Not shifted
		//print("");
		if (value === 127) {
			if (engine.getValue('[Sampler' + (num + 1) + ']', 'repeat') === 1 &&
			    engine.getValue('[Sampler' + (num + 1) + ']', 'play') === 1) {
				engine.setValue('[Sampler' + (num + 1) + ']', 'cue_gotoandstop', 1);
				return;
			}
			if (engine.getValue('[Sampler' + (num + 1) + ']', 'play') === 1) {
				engine.setValue('[Sampler' + (num + 1) + ']', 'cue_gotoandstop', 1);
				return;
			}
			engine.setValue('[Sampler' + (num + 1) + ']', 'cue_gotoandplay', 1);
		}
		if (value === 0) {
			
		}
		
	}
}

CMDDC.plyRow = function (num, a) {
			if (CMDDC.playRow[num]) {
				CMDDC.playRow[num] = false;
				if (engine.getValue('[Sampler' + a[0] + ']', 'play') === 1) {
					engine.setValue('[Sampler' + a[0] + ']', 'start_stop', 1);
				} else {
					engine.setValue('[Sampler' + a[0] + ']', 'start_play', 1);
					
				}
				if (engine.getValue('[Sampler' + a[1] + ']', 'play') === 1) {
					engine.setValue('[Sampler' + a[1] + ']', 'start_stop', 1);
				} else {
					engine.setValue('[Sampler' + a[1] + ']', 'start_play', 1);
				}
				if (engine.getValue('[Sampler' + a[2] + ']', 'play') === 1) {
					engine.setValue('[Sampler' + a[2] + ']', 'start_stop', 1);
				} else {
					engine.setValue('[Sampler' + a[2] + ']', 'start_play', 1);
				}
				if (engine.getValue('[Sampler' + a[3] + ']', 'play') === 1) {
					engine.setValue('[Sampler' + a[3] + ']', 'start_stop', 1);
				} else {
					engine.setValue('[Sampler' + a[3] + ']', 'start_play', 1);
				}
				
				if (
					engine.getValue('[Sampler' + a[0] + ']', 'play') === 1 &&
					engine.getValue('[Sampler' + a[1] + ']', 'play') === 1 &&
					engine.getValue('[Sampler' + a[2] + ']', 'play') === 1 &&
					engine.getValue('[Sampler' + a[3] + ']', 'play') === 1
				) CMDDC.playRow[num] = true;
			} else {
				CMDDC.playRow[num] = true;
				
				engine.setValue('[Sampler' + a[0] + ']', 'start_play', 1);
				engine.setValue('[Sampler' + a[1] + ']', 'start_play', 1);
				engine.setValue('[Sampler' + a[2] + ']', 'start_play', 1);
				engine.setValue('[Sampler' + a[3] + ']', 'start_play', 1);
			}
}

CMDDC.rtry = function (num, value) {
	// Pitch (PFL button)
	if (CMDDC.shift4 === true) {
		
		if (engine.getValue('[Sampler' + (num + CMDDC.tmp1) + ']', 'file_bpm') !== 0) {
			var bpm = Math.round(engine.getValue('[Sampler' + (num + CMDDC.tmp1) + ']', 'bpm'));
			
			if (value === 0x41) {
				bpm = bpm * 1.059;
				engine.setValue('[Sampler' + (num + CMDDC.tmp1) + ']', 'bpm', bpm);
				return;
			}
			if (value === 0x3F) {
				bpm = bpm * 0.944
				if (bpm <= 10) bpm = 10;
				engine.setValue('[Sampler' + (num + CMDDC.tmp1) + ']', 'bpm', bpm);
				return;
			}
		} else {
			var pitch = engine.getValue('[Sampler' + (num + CMDDC.tmp1) + ']', 'pitch');
			
			if (value === 0x41) {
				pitch++;
				engine.setValue('[Sampler' + (num + CMDDC.tmp1) + ']', 'pitch', pitch);
				return;
			}
			if (value === 0x3F) {
				pitch--;
				engine.setValue('[Sampler' + (num + CMDDC.tmp1) + ']', 'pitch', pitch);
				return;
			}
		}
		return;
	}
	

	if (value === 0x41) {
		var gain = engine.getValue('[Sampler' + (num + CMDDC.tmp1) + ']', 'pregain') * 100;
		gain = (gain + 5) / 100
		engine.setValue('[Sampler' + (num + CMDDC.tmp1) + ']', 'pregain', gain);
		
		if (!CMDDC.tmp1) CMDDC.chVolumeLed(num, value);
		
		return;
	}
	if (value === 0x3F) {
		var gain = engine.getValue('[Sampler' + (num + CMDDC.tmp1) + ']', 'pregain') * 100;
		gain = (gain - 5) / 100;
		engine.setValue('[Sampler' + (num + CMDDC.tmp1) + ']', 'pregain', gain);
		
		if (!CMDDC.tmp1) CMDDC.chVolumeLed(num, value);
		
		return;
	}
}

CMDDC.shutdown = function() { for (i = 0; i < 0x35; i++) {midi.sendShortMsg(0x95, i, 0x00); midi.sendShortMsg(0xB5, i, 0x00)} }