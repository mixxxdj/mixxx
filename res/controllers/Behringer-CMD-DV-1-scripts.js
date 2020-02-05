var CMDDV = new Object();

CMDDV.init = function () {
	CMDDV.ch = ['[Channel3]', '[Channel1]', '[Channel2]', '[Channel4]']; // Channel order
	
	CMDDV.cch     = '[Channel1]'; // Focus Channel
	CMDDV.cFXunit = 1;
	
	
	CMDDV.smplUnt = 1; // Default EffectUnit apply to a SampleDeck
	CMDDV.shift_1 = 0;    // This var has been used for applying FX Unit to Samper > 8 MUST be 0 or 8
	CMDDV.shift_2 = false;
	CMDDV.dkctvt = [null, false, false, false, false]; // Deck Activate - this var helps decide which deck FX to apply
	
	CMDDV.ngneffect = [null, false, false, false, false]; // Use it to manage brake and softstart
	
	//midi.sendShortMsg(0x90, 0x40, 0x01);
	
	CMDDV.shift1 = false;
	CMDDV.shift2 = false;
	CMDDV.shift3 = false;
	CMDDV.shift4 = false;
	
	CMDDV.fx1act = 0;
	CMDDV.fx2act = 0;
	CMDDV.fx3act = 0;
	CMDDV.fx4act = 0;
	
	CMDDV.fx1effect = 1; // 1 - 2 -3
	CMDDV.fx2effect = 1; // 1 - 2 -3
	CMDDV.fx3effect = 1; // 1 - 2 -3
	CMDDV.fx4effect = 1; // 1 - 2 -3
	
	midi.sendShortMsg(0x90, 0x25, 0x01);
	midi.sendShortMsg(0x90, 0x29, 0x01);
	midi.sendShortMsg(0x90, 0x2D, 0x01);
	midi.sendShortMsg(0x90, 0x31, 0x01);
	
	// Master FX
	midi.sendShortMsg(0x90, 0x44, 0x01);
	midi.sendShortMsg(0x90, 0x45, 0x01);
	midi.sendShortMsg(0x90, 0x46, 0x01);
	midi.sendShortMsg(0x90, 0x47, 0x01);
	
	//midi.sendShortMsg(0xb0, 0x25, 0x00); 
	
	
	CMDDV.fx1on = engine.getValue('[EffectRack1_EffectUnit1_Effect1]', 'enabled');
	CMDDV.fx2on = engine.getValue('[EffectRack1_EffectUnit1_Effect2]', 'enabled');
	CMDDV.fx3on = engine.getValue('[EffectRack1_EffectUnit1_Effect3]', 'enabled');
	CMDDV.fx4on = engine.getValue('[EffectRack1_EffectUnit2_Effect1]', 'enabled');
	CMDDV.fx5on = engine.getValue('[EffectRack1_EffectUnit2_Effect2]', 'enabled');
	CMDDV.fx6on = engine.getValue('[EffectRack1_EffectUnit2_Effect3]', 'enabled');
	CMDDV.fx7on = engine.getValue('[EffectRack1_EffectUnit3_Effect1]', 'enabled');
	CMDDV.fx8on = engine.getValue('[EffectRack1_EffectUnit3_Effect2]', 'enabled');
	CMDDV.fx9on = engine.getValue('[EffectRack1_EffectUnit3_Effect3]', 'enabled');
	CMDDV.fx10on = engine.getValue('[EffectRack1_EffectUnit4_Effect1]', 'enabled');
	CMDDV.fx11on = engine.getValue('[EffectRack1_EffectUnit4_Effect2]', 'enabled');
	CMDDV.fx12on = engine.getValue('[EffectRack1_EffectUnit4_Effect3]', 'enabled');
	
	CMDDV.ch0fx =  [engine.getValue('[EffectRack1_EffectUnit1]', 'group_'+CMDDV.ch[0]+'_enable'),
					engine.getValue('[EffectRack1_EffectUnit1]', 'group_'+CMDDV.ch[1]+'_enable'),
					engine.getValue('[EffectRack1_EffectUnit1]', 'group_'+CMDDV.ch[2]+'_enable'),
					engine.getValue('[EffectRack1_EffectUnit1]', 'group_'+CMDDV.ch[3]+'_enable')];
	CMDDV.ch1fx =  [engine.getValue('[EffectRack1_EffectUnit2]', 'group_'+CMDDV.ch[0]+'_enable'),
					engine.getValue('[EffectRack1_EffectUnit2]', 'group_'+CMDDV.ch[1]+'_enable'),
					engine.getValue('[EffectRack1_EffectUnit2]', 'group_'+CMDDV.ch[2]+'_enable'),
					engine.getValue('[EffectRack1_EffectUnit2]', 'group_'+CMDDV.ch[3]+'_enable')];
	CMDDV.ch2fx =  [engine.getValue('[EffectRack1_EffectUnit3]', 'group_'+CMDDV.ch[0]+'_enable'),
					engine.getValue('[EffectRack1_EffectUnit3]', 'group_'+CMDDV.ch[1]+'_enable'),
					engine.getValue('[EffectRack1_EffectUnit3]', 'group_'+CMDDV.ch[2]+'_enable'),
					engine.getValue('[EffectRack1_EffectUnit3]', 'group_'+CMDDV.ch[3]+'_enable')];
	CMDDV.ch3fx =  [engine.getValue('[EffectRack1_EffectUnit4]', 'group_'+CMDDV.ch[0]+'_enable'),
					engine.getValue('[EffectRack1_EffectUnit4]', 'group_'+CMDDV.ch[1]+'_enable'),
					engine.getValue('[EffectRack1_EffectUnit4]', 'group_'+CMDDV.ch[2]+'_enable'),
					engine.getValue('[EffectRack1_EffectUnit4]', 'group_'+CMDDV.ch[3]+'_enable')];
	

	engine.makeConnection('[EffectRack1_EffectUnit1_Effect1]', 'enabled',  function(value) {midi.sendShortMsg(0x90, 0x15, value)}).trigger();
	engine.makeConnection('[EffectRack1_EffectUnit1_Effect2]', 'enabled',  function(value) {midi.sendShortMsg(0x90, 0x16, value)}).trigger();
	engine.makeConnection('[EffectRack1_EffectUnit1_Effect3]', 'enabled',  function(value) {midi.sendShortMsg(0x90, 0x17, value)}).trigger();
	engine.makeConnection('[EffectRack1_EffectUnit2_Effect1]', 'enabled',  function(value) {midi.sendShortMsg(0x90, 0x19, value)}).trigger();
	engine.makeConnection('[EffectRack1_EffectUnit2_Effect2]', 'enabled',  function(value) {midi.sendShortMsg(0x90, 0x1A, value)}).trigger();
	engine.makeConnection('[EffectRack1_EffectUnit2_Effect3]', 'enabled',  function(value) {midi.sendShortMsg(0x90, 0x1B, value)}).trigger();
	engine.makeConnection('[EffectRack1_EffectUnit3_Effect1]', 'enabled',  function(value) {midi.sendShortMsg(0x90, 0x1D, value)}).trigger();
	engine.makeConnection('[EffectRack1_EffectUnit3_Effect2]', 'enabled',  function(value) {midi.sendShortMsg(0x90, 0x1E, value)}).trigger();
	engine.makeConnection('[EffectRack1_EffectUnit3_Effect3]', 'enabled',  function(value) {midi.sendShortMsg(0x90, 0x1F, value)}).trigger();
	engine.makeConnection('[EffectRack1_EffectUnit4_Effect1]', 'enabled',  function(value) {midi.sendShortMsg(0x90, 0x21, value)}).trigger();
	engine.makeConnection('[EffectRack1_EffectUnit4_Effect2]', 'enabled',  function(value) {midi.sendShortMsg(0x90, 0x22, value)}).trigger();
	engine.makeConnection('[EffectRack1_EffectUnit4_Effect3]', 'enabled',  function(value) {midi.sendShortMsg(0x90, 0x23, value)}).trigger();
	
	CMDDV.playconnect1 = engine.makeConnection(CMDDV.ch[0], 'play',  function(value) {midi.sendShortMsg(0x90, 0x48, value); var deck = parseInt(CMDDV.ch[0].substring(8,9)); if (value === 0) {CMDDV.ngneffect[deck] = false}; }); CMDDV.playconnect1.trigger();
	CMDDV.playconnect2 = engine.makeConnection(CMDDV.ch[1], 'play',  function(value) {midi.sendShortMsg(0x90, 0x49, value); var deck = parseInt(CMDDV.ch[1].substring(8,9)); if (value === 0) {CMDDV.ngneffect[deck] = false}; }); CMDDV.playconnect2.trigger();
	CMDDV.playconnect3 = engine.makeConnection(CMDDV.ch[2], 'play',  function(value) {midi.sendShortMsg(0x90, 0x4A, value); var deck = parseInt(CMDDV.ch[2].substring(8,9)); if (value === 0) {CMDDV.ngneffect[deck] = false}; }); CMDDV.playconnect3.trigger();
	CMDDV.playconnect4 = engine.makeConnection(CMDDV.ch[3], 'play',  function(value) {midi.sendShortMsg(0x90, 0x4B, value); var deck = parseInt(CMDDV.ch[3].substring(8,9)); if (value === 0) {CMDDV.ngneffect[deck] = false}; }); CMDDV.playconnect4.trigger();
	
	CMDDV.sampl1 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler1]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5C, value)}); CMDDV.sampl1.trigger();
	CMDDV.sampl2 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler2]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5D, value)}); CMDDV.sampl2.trigger();
	CMDDV.sampl3 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler3]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5E, value)}); CMDDV.sampl3.trigger();
	CMDDV.sampl4 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler4]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5F, value)}); CMDDV.sampl4.trigger();
	CMDDV.sampl5 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler5]_enable',  function(value) {midi.sendShortMsg(0x90, 0x58, value)}); CMDDV.sampl5.trigger();
	CMDDV.sampl6 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler6]_enable',  function(value) {midi.sendShortMsg(0x90, 0x59, value)}); CMDDV.sampl6.trigger();
	CMDDV.sampl7 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler7]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5A, value)}); CMDDV.sampl7.trigger();
	CMDDV.sampl8 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler8]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5B, value)}); CMDDV.sampl8.trigger();
	
};

 
CMDDV.button = function (channel, control, value, status, group) {
	if (value === 127) {
		switch (control) {
			case 0x14: 
				    CMDDV.shift1 = true;
					midi.sendShortMsg(0x90, 0x14, 0x01);
				break;
			case 0x15: 
				if (CMDDV.fx1on === 1) {
					CMDDV.fx1on = 0;
					engine.setValue('[EffectRack1_EffectUnit1_Effect1]', 'enabled', 0);
				} else {
					CMDDV.fx1on = 1;
					engine.setValue('[EffectRack1_EffectUnit1_Effect1]', 'enabled', 1);
				}
				break;
			case 0x16: 
			
				if (CMDDV.fx2on === 1) {
					CMDDV.fx2on = 0;
					engine.setValue('[EffectRack1_EffectUnit1_Effect2]', 'enabled', 0);
				} else {
					CMDDV.fx2on = 1;
					engine.setValue('[EffectRack1_EffectUnit1_Effect2]', 'enabled', 1);
				}
				break;
			case 0x17: 
			
				if (CMDDV.fx3on === 1) {
					CMDDV.fx3on = 0;
					engine.setValue('[EffectRack1_EffectUnit1_Effect3]', 'enabled', 0);
				} else {
					CMDDV.fx3on = 1;
					engine.setValue('[EffectRack1_EffectUnit1_Effect3]', 'enabled', 1);
				}
				break;
			case 0x18: 
				CMDDV.shift2 = true;
				midi.sendShortMsg(0x90, 0x18, 0x01);
				break;
			case 0x19: 
				if (CMDDV.fx4on === 1) {
					CMDDV.fx4on = 0;
					engine.setValue('[EffectRack1_EffectUnit2_Effect1]', 'enabled', 0);
				} else {
					CMDDV.fx4on = 1;
					engine.setValue('[EffectRack1_EffectUnit2_Effect1]', 'enabled', 1);
				}
				break; 
			case 0x1A: 
			
				if (CMDDV.fx5on === 1) {
					CMDDV.fx5on = 0;
					engine.setValue('[EffectRack1_EffectUnit2_Effect2]', 'enabled', 0);
				} else {
					CMDDV.fx5on = 1;
					engine.setValue('[EffectRack1_EffectUnit2_Effect2]', 'enabled', 1);
				}
				break;
			case 0x1B: 
			
				if (CMDDV.fx6on === 1) {
					CMDDV.fx6on = 0;
					engine.setValue('[EffectRack1_EffectUnit2_Effect3]', 'enabled', 0);
				} else {
					CMDDV.fx6on = 1;
					engine.setValue('[EffectRack1_EffectUnit2_Effect3]', 'enabled', 1);
				}
				break;
			case 0x1C: 
				CMDDV.shift3 = true;
				midi.sendShortMsg(0x90, 0x1C, 0x01);
				break;
			case 0x1D: 
				if (CMDDV.fx7on === 1) {
					CMDDV.fx7on = 0;
					engine.setValue('[EffectRack1_EffectUnit3_Effect1]', 'enabled', 0);
				} else {
					CMDDV.fx7on = 1;
					engine.setValue('[EffectRack1_EffectUnit3_Effect1]', 'enabled', 1);
				}
				break;
			case 0x1E: 
			
				if (CMDDV.fx8on === 1) {
					CMDDV.fx8on = 0;
					engine.setValue('[EffectRack1_EffectUnit3_Effect2]', 'enabled', 0);
				} else {
					CMDDV.fx8on = 1;
					engine.setValue('[EffectRack1_EffectUnit3_Effect2]', 'enabled', 1);
				}
				break;
			case 0x1F: 
			
				if (CMDDV.fx9on === 1) {
					CMDDV.fx9on = 0;
					engine.setValue('[EffectRack1_EffectUnit3_Effect3]', 'enabled', 0);
				} else {
					CMDDV.fx9on = 1;
					engine.setValue('[EffectRack1_EffectUnit3_Effect3]', 'enabled', 1);
				}
				break;
			case 0x20: 
				CMDDV.shift4 = true;
				midi.sendShortMsg(0x90, 0x20, 0x01);
				break;
			case 0x21: 
				if (CMDDV.fx10on === 1) {
					CMDDV.fx10on = 0;
					engine.setValue('[EffectRack1_EffectUnit4_Effect1]', 'enabled', 0);
				} else {
					CMDDV.fx10on = 1;
					engine.setValue('[EffectRack1_EffectUnit4_Effect1]', 'enabled', 1);
				}
				break;
			case 0x22: 
			
				if (CMDDV.fx11on === 1) {
					CMDDV.fx11on = 0;
					engine.setValue('[EffectRack1_EffectUnit4_Effect2]', 'enabled', 0);
				} else {
					CMDDV.fx11on = 1;
					engine.setValue('[EffectRack1_EffectUnit4_Effect2]', 'enabled', 1);
				}
				break;
			case 0x23: 
			
				if (CMDDV.fx12on === 1) {
					CMDDV.fx12on = 0;
					engine.setValue('[EffectRack1_EffectUnit4_Effect3]', 'enabled', 0);
				} else {
					CMDDV.fx12on = 1;
					engine.setValue('[EffectRack1_EffectUnit4_Effect3]', 'enabled', 1);
				}
				break;
				
				
			// FX 1	
			case 0x24: break;
			case 0x25: CMDDV.fx1effect = 1; midi.sendShortMsg(0x90, 0x25, 0x01); midi.sendShortMsg(0x90, 0x26, 0x00); midi.sendShortMsg(0x90, 0x27, 0x00); break;
			case 0x26: CMDDV.fx1effect = 2; midi.sendShortMsg(0x90, 0x25, 0x00); midi.sendShortMsg(0x90, 0x26, 0x01); midi.sendShortMsg(0x90, 0x27, 0x00); break;
			case 0x27: CMDDV.fx1effect = 3; midi.sendShortMsg(0x90, 0x25, 0x00); midi.sendShortMsg(0x90, 0x26, 0x00); midi.sendShortMsg(0x90, 0x27, 0x01); break;
			
			// FX 2	
			case 0x28: break;
			case 0x29: CMDDV.fx2effect = 1; midi.sendShortMsg(0x90, 0x29, 0x01); midi.sendShortMsg(0x90, 0x2A, 0x00); midi.sendShortMsg(0x90, 0x2B, 0x00); break;
			case 0x2A: CMDDV.fx2effect = 2; midi.sendShortMsg(0x90, 0x29, 0x00); midi.sendShortMsg(0x90, 0x2A, 0x01); midi.sendShortMsg(0x90, 0x2B, 0x00); break;
			case 0x2B: CMDDV.fx2effect = 3; midi.sendShortMsg(0x90, 0x29, 0x00); midi.sendShortMsg(0x90, 0x2A, 0x00); midi.sendShortMsg(0x90, 0x2B, 0x01); break;
			
			// FX 3	
			case 0x2C: break;
			case 0x2D: CMDDV.fx3effect = 1; midi.sendShortMsg(0x90, 0x2D, 0x01); midi.sendShortMsg(0x90, 0x2E, 0x00); midi.sendShortMsg(0x90, 0x2F, 0x00); break;
			case 0x2E: CMDDV.fx3effect = 2; midi.sendShortMsg(0x90, 0x2D, 0x00); midi.sendShortMsg(0x90, 0x2E, 0x01); midi.sendShortMsg(0x90, 0x2F, 0x00); break;
			case 0x2F: CMDDV.fx3effect = 3; midi.sendShortMsg(0x90, 0x2D, 0x00); midi.sendShortMsg(0x90, 0x2E, 0x00); midi.sendShortMsg(0x90, 0x2F, 0x01); break;
			
			// FX 4
			case 0x30: break;
			case 0x31: CMDDV.fx4effect = 1; midi.sendShortMsg(0x90, 0x31, 0x01); midi.sendShortMsg(0x90, 0x32, 0x00); midi.sendShortMsg(0x90, 0x33, 0x00); break;
			case 0x32: CMDDV.fx4effect = 2; midi.sendShortMsg(0x90, 0x31, 0x00); midi.sendShortMsg(0x90, 0x32, 0x01); midi.sendShortMsg(0x90, 0x33, 0x00); break;
			case 0x33: CMDDV.fx4effect = 3; midi.sendShortMsg(0x90, 0x31, 0x00); midi.sendShortMsg(0x90, 0x32, 0x00); midi.sendShortMsg(0x90, 0x33, 0x01); break;
			
			// FOCUS
			case 0x40: var deck = parseInt(CMDDV.ch[0].substring(8,9)); if (CMDDV.dkctvt[deck] === false) {CMDDV.dkctvt[deck] = deck; midi.sendShortMsg(0x90, 0x40, 0x01); }  else {CMDDV.dkctvt[deck] = false; midi.sendShortMsg(0x90, 0x40, 0x00); }; break;
			case 0x41: var deck = parseInt(CMDDV.ch[1].substring(8,9)); if (CMDDV.dkctvt[deck] === false) {CMDDV.dkctvt[deck] = deck; midi.sendShortMsg(0x90, 0x41, 0x01); }  else {CMDDV.dkctvt[deck] = false; midi.sendShortMsg(0x90, 0x41, 0x00); }; break;
			case 0x42: var deck = parseInt(CMDDV.ch[2].substring(8,9)); if (CMDDV.dkctvt[deck] === false) {CMDDV.dkctvt[deck] = deck; midi.sendShortMsg(0x90, 0x42, 0x01); }  else {CMDDV.dkctvt[deck] = false; midi.sendShortMsg(0x90, 0x42, 0x00); }; break;
			case 0x43: var deck = parseInt(CMDDV.ch[3].substring(8,9)); if (CMDDV.dkctvt[deck] === false) {CMDDV.dkctvt[deck] = deck; midi.sendShortMsg(0x90, 0x43, 0x01); }  else {CMDDV.dkctvt[deck] = false; midi.sendShortMsg(0x90, 0x43, 0x00); }; break;
			
			// DOUBLE
			case 0x48: var deck = parseInt(CMDDV.ch[0].substring(8,9)); if(CMDDV.shift_1) {engine.spinback(deck, true); break;} if (engine.getValue(CMDDV.ch[0], 'play') && CMDDV.ngneffect[deck] === false) {engine.brake(deck, true); CMDDV.ngneffect[deck] = true} else {engine.softStart(deck, true); CMDDV.ngneffect[deck] = false; } midi.sendShortMsg(0x90, 0x48, 0x02); break;
			case 0x49: var deck = parseInt(CMDDV.ch[1].substring(8,9)); if(CMDDV.shift_1) {engine.spinback(deck, true); break;} if (engine.getValue(CMDDV.ch[1], 'play') && CMDDV.ngneffect[deck] === false) {engine.brake(deck, true); CMDDV.ngneffect[deck] = true} else {engine.softStart(deck, true); CMDDV.ngneffect[deck] = false; } midi.sendShortMsg(0x90, 0x49, 0x02); break;
			case 0x4A: var deck = parseInt(CMDDV.ch[2].substring(8,9)); if(CMDDV.shift_1) {engine.spinback(deck, true); break;} if (engine.getValue(CMDDV.ch[2], 'play') && CMDDV.ngneffect[deck] === false) {engine.brake(deck, true); CMDDV.ngneffect[deck] = true} else {engine.softStart(deck, true); CMDDV.ngneffect[deck] = false; } midi.sendShortMsg(0x90, 0x4A, 0x02); break;
			case 0x4B: var deck = parseInt(CMDDV.ch[3].substring(8,9)); if(CMDDV.shift_1) {engine.spinback(deck, true); break;} if (engine.getValue(CMDDV.ch[3], 'play') && CMDDV.ngneffect[deck] === false) {engine.brake(deck, true); CMDDV.ngneffect[deck] = true} else {engine.softStart(deck, true); CMDDV.ngneffect[deck] = false; } midi.sendShortMsg(0x90, 0x4B, 0x02); break;
			
			// MASTER
			case 0x44: CMDDV.crossfade(); break;
			case 0x45: CMDDV.cut();       break;
			case 0x46: CMDDV.cut2();      break;
			case 0x47: CMDDV.fade();      break;
				
			// SIZE
			case 0x50: var size = CMDDV.shift_2 ? 0.0625 : 1;  for (var i = 0, a = 1; i <= 3; i ++, a++) {engine.setValue(CMDDV.ch[i], 'beatloop_size', size); engine.setValue(CMDDV.ch[i], 'beatjump_size', size); if (CMDDV.dkctvt[a]) {engine.setValue(CMDDV.ch[CMDDV.dkctvt[a]], 'beatloop_activate', 1);} midi.sendShortMsg(0x90, 0x50, 0x01);}; midi.sendShortMsg(0x90, 0x56, 0x01); break;
			case 0x51: var size = CMDDV.shift_2 ? 0.125 : 4 ;  for (var i = 0, a = 1; i <= 3; i ++, a++) {engine.setValue(CMDDV.ch[i], 'beatloop_size', size); engine.setValue(CMDDV.ch[i], 'beatjump_size', size); if (CMDDV.dkctvt[a]) {engine.setValue(CMDDV.ch[CMDDV.dkctvt[a]], 'beatloop_activate', 1);} midi.sendShortMsg(0x90, 0x51, 0x01);}; midi.sendShortMsg(0x90, 0x56, 0x01); break;
			case 0x52: var size = CMDDV.shift_2 ? 0.25 : 8  ;  for (var i = 0, a = 1; i <= 3; i ++, a++) {engine.setValue(CMDDV.ch[i], 'beatloop_size', size); engine.setValue(CMDDV.ch[i], 'beatjump_size', size); if (CMDDV.dkctvt[a]) {engine.setValue(CMDDV.ch[CMDDV.dkctvt[a]], 'beatloop_activate', 1);} midi.sendShortMsg(0x90, 0x52, 0x01);}; midi.sendShortMsg(0x90, 0x56, 0x01); break;
			case 0x53: var size = CMDDV.shift_2 ? 0.5 : 16  ;  for (var i = 0, a = 1; i <= 3; i ++, a++) {engine.setValue(CMDDV.ch[i], 'beatloop_size', size); engine.setValue(CMDDV.ch[i], 'beatjump_size', size); if (CMDDV.dkctvt[a]) {engine.setValue(CMDDV.ch[CMDDV.dkctvt[a]], 'beatloop_activate', 1);} midi.sendShortMsg(0x90, 0x53, 0x01);}; midi.sendShortMsg(0x90, 0x56, 0x01); break;
			
			// SHIFTs
			case 0x54: 
				CMDDV.shift_1 = 8;    midi.sendShortMsg(0x90, 0x54, 0x01); 
				CMDDV.sampl1.disconnect(); 
				CMDDV.sampl2.disconnect();				
				CMDDV.sampl3.disconnect();				
				CMDDV.sampl4.disconnect();				
				CMDDV.sampl5.disconnect();				
				CMDDV.sampl6.disconnect();				
				CMDDV.sampl7.disconnect();				
				CMDDV.sampl8.disconnect();				
				
				CMDDV.sampl1 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler9]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5C, value)}); CMDDV.sampl1.trigger();
				CMDDV.sampl2 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler10]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5D, value)}); CMDDV.sampl2.trigger();
				CMDDV.sampl3 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler11]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5E, value)}); CMDDV.sampl3.trigger();
				CMDDV.sampl4 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler12]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5F, value)}); CMDDV.sampl4.trigger();
				CMDDV.sampl5 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler13]_enable',  function(value) {midi.sendShortMsg(0x90, 0x58, value)}); CMDDV.sampl5.trigger();
				CMDDV.sampl6 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler14]_enable',  function(value) {midi.sendShortMsg(0x90, 0x59, value)}); CMDDV.sampl6.trigger();
				CMDDV.sampl7 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler15]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5A, value)}); CMDDV.sampl7.trigger();
				CMDDV.sampl8 = engine.makeConnection('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler16]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5B, value)}); CMDDV.sampl8.trigger();
			break;
			
			case 0x55: CMDDV.shift_2 = true; midi.sendShortMsg(0x90, 0x55, 0x01); break;
			case 0x56: for (var i = 1; i <= 4; i ++) {if (CMDDV.dkctvt[i]) {engine.setValue(CMDDV.ch[CMDDV.dkctvt[i]], 'reloop_toggle', 1);}} midi.sendShortMsg(0x90, 0x56, 0x00);  break;
			case 0x57: for (var i = 0; i <= 3; i ++) {engine.setValue(CMDDV.ch[i], 'slip_enabled', 1);} midi.sendShortMsg(0x90, 0x57, 0x01); break;
			
			// Sample FX Apply
			case 0x5C: script.toggleControl('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler'+(1+CMDDV.shift_1)+']_enable'); break;
			case 0x5D: script.toggleControl('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler'+(2+CMDDV.shift_1)+']_enable'); break;
			case 0x5E: script.toggleControl('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler'+(3+CMDDV.shift_1)+']_enable'); break;
			case 0x5F: script.toggleControl('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler'+(4+CMDDV.shift_1)+']_enable'); break;
			case 0x58: script.toggleControl('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler'+(5+CMDDV.shift_1)+']_enable'); break;
			case 0x59: script.toggleControl('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler'+(6+CMDDV.shift_1)+']_enable'); break;
			case 0x5A: script.toggleControl('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler'+(7+CMDDV.shift_1)+']_enable'); break;
			case 0x5B: script.toggleControl('[EffectRack1_EffectUnit'+CMDDV.smplUnt+']', 'group_[Sampler'+(8+CMDDV.shift_1)+']_enable'); break;
			
		}
	}

	if (value === 0) {
		switch (control) {
			case 0x14: 
				CMDDV.shift1 = false;
				midi.sendShortMsg(0x90, 0x14, 0x00); 
			break;
			case 0x18: 
				CMDDV.shift2 = false;
				midi.sendShortMsg(0x90, 0x18, 0x00); 
			break;
			case 0x1C: 
				CMDDV.shift3 = false;
				midi.sendShortMsg(0x90, 0x1C, 0x00); 
			break;
			case 0x20: 
				CMDDV.shift4 = false;
				midi.sendShortMsg(0x90, 0x20, 0x00); 
			break;
			
			
			case 0x24: 

				break;
			case 0x28: 

				break;
			case 0x2C: 

				break;
			case 0x30: 

				break;
			
			
			case 0x50: midi.sendShortMsg(0x90, 0x50, 0x00); break;
			case 0x51: midi.sendShortMsg(0x90, 0x51, 0x00); break;
			case 0x52: midi.sendShortMsg(0x90, 0x52, 0x00); break;
			case 0x53: midi.sendShortMsg(0x90, 0x53, 0x00); break;
			
			case 0x54: 
				CMDDV.shift_1 = 0; midi.sendShortMsg(0x90, 0x54, 0x00); 
				CMDDV.sampl1.disconnect(); 
				CMDDV.sampl2.disconnect();				
				CMDDV.sampl3.disconnect();				
				CMDDV.sampl4.disconnect();				
				CMDDV.sampl5.disconnect();				
				CMDDV.sampl6.disconnect();				
				CMDDV.sampl7.disconnect();				
				CMDDV.sampl8.disconnect();				
				
				CMDDV.sampl1 = engine.makeConnection('[EffectRack1_EffectUnit1]', 'group_[Sampler1]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5C, value)}); CMDDV.sampl1.trigger();
				CMDDV.sampl2 = engine.makeConnection('[EffectRack1_EffectUnit1]', 'group_[Sampler2]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5D, value)}); CMDDV.sampl2.trigger();
				CMDDV.sampl3 = engine.makeConnection('[EffectRack1_EffectUnit1]', 'group_[Sampler3]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5E, value)}); CMDDV.sampl3.trigger();
				CMDDV.sampl4 = engine.makeConnection('[EffectRack1_EffectUnit1]', 'group_[Sampler4]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5F, value)}); CMDDV.sampl4.trigger();
				CMDDV.sampl5 = engine.makeConnection('[EffectRack1_EffectUnit1]', 'group_[Sampler5]_enable',  function(value) {midi.sendShortMsg(0x90, 0x58, value)}); CMDDV.sampl5.trigger();
				CMDDV.sampl6 = engine.makeConnection('[EffectRack1_EffectUnit1]', 'group_[Sampler6]_enable',  function(value) {midi.sendShortMsg(0x90, 0x59, value)}); CMDDV.sampl6.trigger();
				CMDDV.sampl7 = engine.makeConnection('[EffectRack1_EffectUnit1]', 'group_[Sampler7]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5A, value)}); CMDDV.sampl7.trigger();
				CMDDV.sampl8 = engine.makeConnection('[EffectRack1_EffectUnit1]', 'group_[Sampler8]_enable',  function(value) {midi.sendShortMsg(0x90, 0x5B, value)}); CMDDV.sampl8.trigger();
			break;
			
			
			
			
			case 0x55: CMDDV.shift_2 = false; midi.sendShortMsg(0x90, 0x55, 0x00); break;
			
		case 0x57: for (var i = 0; i <= 3; i ++) {engine.setValue(CMDDV.ch[i], 'slip_enabled', 0);} midi.sendShortMsg(0x90, 0x57, 0x00); break;
			
		}
		return;
	}
}

CMDDV.rotary = function (channel, control, value, status, group) {
	switch (control) {
		case 0x14: if (CMDDV.shift1) CMDDV.FX('[EffectRack1_EffectUnit1]', 'mix', value); else CMDDV.FX('[EffectRack1_EffectUnit1]', 'super1', value); break;
		case 0x15: if (CMDDV.shift1) CMDDV.chFX('[EffectRack1_EffectUnit1_Effect1]', 'effect_selector', value); else CMDDV.FX('[EffectRack1_EffectUnit1_Effect1]', 'meta', value); break;
		case 0x16: if (CMDDV.shift1) CMDDV.chFX('[EffectRack1_EffectUnit1_Effect2]', 'effect_selector', value); else CMDDV.FX('[EffectRack1_EffectUnit1_Effect2]', 'meta', value); break;
		case 0x17: if (CMDDV.shift1) CMDDV.chFX('[EffectRack1_EffectUnit1_Effect3]', 'effect_selector', value); else CMDDV.FX('[EffectRack1_EffectUnit1_Effect3]', 'meta', value); break;
		
		case 0x18: if (CMDDV.shift2) CMDDV.FX('[EffectRack1_EffectUnit2]', 'mix', value); else CMDDV.FX('[EffectRack1_EffectUnit2]', 'super1', value); break;
		case 0x19: if (CMDDV.shift2) CMDDV.chFX('[EffectRack1_EffectUnit2_Effect1]', 'effect_selector', value); else CMDDV.FX('[EffectRack1_EffectUnit2_Effect1]', 'meta', value); break;
		case 0x1A: if (CMDDV.shift2) CMDDV.chFX('[EffectRack1_EffectUnit2_Effect2]', 'effect_selector', value); else CMDDV.FX('[EffectRack1_EffectUnit2_Effect2]', 'meta', value); break;
		case 0x1B: if (CMDDV.shift2) CMDDV.chFX('[EffectRack1_EffectUnit2_Effect3]', 'effect_selector', value); else CMDDV.FX('[EffectRack1_EffectUnit2_Effect3]', 'meta', value); break;
		
		case 0x1C: if (CMDDV.shift3) CMDDV.FX('[EffectRack1_EffectUnit3]', 'mix', value); else CMDDV.FX('[EffectRack1_EffectUnit3]', 'super1', value); break;
		case 0x1D: if (CMDDV.shift3) CMDDV.chFX('[EffectRack1_EffectUnit3_Effect1]', 'effect_selector', value); else CMDDV.FX('[EffectRack1_EffectUnit3_Effect1]', 'meta', value); break;
		case 0x1E: if (CMDDV.shift3) CMDDV.chFX('[EffectRack1_EffectUnit3_Effect2]', 'effect_selector', value); else CMDDV.FX('[EffectRack1_EffectUnit3_Effect2]', 'meta', value); break;
		case 0x1F: if (CMDDV.shift3) CMDDV.chFX('[EffectRack1_EffectUnit3_Effect3]', 'effect_selector', value); else CMDDV.FX('[EffectRack1_EffectUnit3_Effect3]', 'meta', value); break;
		
		case 0x20: if (CMDDV.shift4) CMDDV.FX('[EffectRack1_EffectUnit4]', 'mix', value); else CMDDV.FX('[EffectRack1_EffectUnit4]', 'super1', value); break;
		case 0x21: if (CMDDV.shift4) CMDDV.chFX('[EffectRack1_EffectUnit4_Effect1]', 'effect_selector', value); else CMDDV.FX('[EffectRack1_EffectUnit4_Effect1]', 'meta', value); break;
		case 0x22: if (CMDDV.shift4) CMDDV.chFX('[EffectRack1_EffectUnit4_Effect2]', 'effect_selector', value); else CMDDV.FX('[EffectRack1_EffectUnit4_Effect2]', 'meta', value); break;
		case 0x23: if (CMDDV.shift4) CMDDV.chFX('[EffectRack1_EffectUnit4_Effect3]', 'effect_selector', value); else CMDDV.FX('[EffectRack1_EffectUnit4_Effect3]', 'meta', value); break;
		
		/* PT 2 */
		
		case 0x24: CMDDV.FX('[EffectRack1_EffectUnit1_Effect'+CMDDV.fx1effect+']', 'parameter1', value); break;
		case 0x25: CMDDV.FX('[EffectRack1_EffectUnit1_Effect'+CMDDV.fx1effect+']', 'parameter2', value); break;
		case 0x26: CMDDV.FX('[EffectRack1_EffectUnit1_Effect'+CMDDV.fx1effect+']', 'parameter3', value); break;
		case 0x27: CMDDV.FX('[EffectRack1_EffectUnit1_Effect'+CMDDV.fx1effect+']', 'parameter4', value); break;
		
		case 0x28: CMDDV.FX('[EffectRack1_EffectUnit2_Effect'+CMDDV.fx2effect+']', 'parameter1', value); break;
		case 0x29: CMDDV.FX('[EffectRack1_EffectUnit2_Effect'+CMDDV.fx2effect+']', 'parameter2', value); break;
		case 0x2A: CMDDV.FX('[EffectRack1_EffectUnit2_Effect'+CMDDV.fx2effect+']', 'parameter3', value); break;
		case 0x2B: CMDDV.FX('[EffectRack1_EffectUnit2_Effect'+CMDDV.fx2effect+']', 'parameter4', value); break;
		
		case 0x2C: CMDDV.FX('[EffectRack1_EffectUnit3_Effect'+CMDDV.fx3effect+']', 'parameter1', value); break;
		case 0x2D: CMDDV.FX('[EffectRack1_EffectUnit3_Effect'+CMDDV.fx3effect+']', 'parameter2', value); break;
		case 0x2E: CMDDV.FX('[EffectRack1_EffectUnit3_Effect'+CMDDV.fx3effect+']', 'parameter3', value); break;
		case 0x2F: CMDDV.FX('[EffectRack1_EffectUnit3_Effect'+CMDDV.fx3effect+']', 'parameter4', value); break;
		
		case 0x30: CMDDV.FX('[EffectRack1_EffectUnit4_Effect'+CMDDV.fx4effect+']', 'parameter1', value); break;
		case 0x31: CMDDV.FX('[EffectRack1_EffectUnit4_Effect'+CMDDV.fx4effect+']', 'parameter2', value); break;
		case 0x32: CMDDV.FX('[EffectRack1_EffectUnit4_Effect'+CMDDV.fx4effect+']', 'parameter3', value); break;
		case 0x33: CMDDV.FX('[EffectRack1_EffectUnit4_Effect'+CMDDV.fx4effect+']', 'parameter4', value); break;
		
		case 0x40: CMDDV.FX('[EffectRack1_EffectUnit1]', 'mix', value); break;
		case 0x41: CMDDV.FX('[EffectRack1_EffectUnit2]', 'mix', value); break;
		case 0x42: CMDDV.FX('[EffectRack1_EffectUnit3]', 'mix', value); break;
		case 0x43: CMDDV.FX('[EffectRack1_EffectUnit4]', 'mix', value); break;
	}
}

CMDDV.FX = function (group, parameter, value) {

	
    var precision = 100;
    var step      = 3;
    var amount = engine.getValue(group, parameter) * precision;
    if (value === 0x41) {
        amount = (amount + step) / precision;
        engine.setValue(group, parameter, amount);
    } else if (value === 0x3f) {
        amount = (amount - step) / precision;
        engine.setValue(group, parameter, amount);
    }
}

CMDDV.chFX = function (group, parameter, value) {

    if (value === 0x41) {
        engine.setValue(group, parameter, 1);
    } else if (value === 0x3f) {
        engine.setValue(group, parameter, -1);
    }
}

CMDDV.shutdown = function () {
	for (var i = 0; i <= 0x5F; i++) {
		midi.sendShortMsg(0x90, i, 0x00);
	}  
	for (var a = 0; a <= 0x48; a++) {
		midi.sendShortMsg(0xB0, a, 0x00);
	}  
};

CMDDV.crossfade = function () {
	var value = engine.getValue('[Master]', 'crossfader');
	
	if (CMDDV.timer1) {
		engine.stopTimer(CMDDV.timer1);
		midi.sendShortMsg(0x90, 0x44, 0x01);
		CMDDV.timer1 = null;
		return;
	}
	
	midi.sendShortMsg(0x90, 0x44, 0x02);
	
	if (value <= 0.01) {
		CMDDV.timer1 = engine.beginTimer(50, CMDDV.crossRight, false); 
	} else {
		CMDDV.timer1 = engine.beginTimer(50, CMDDV.crossLeft,  false); 
	}
}

CMDDV.crossRight = function (value) {
		cur_val = engine.getValue('[Master]', 'crossfader');
		cur_val = cur_val + 0.01;
		engine.setValue('[Master]', 'crossfader', cur_val);
		if (cur_val >= 1) {
			if (CMDDV.timer1) engine.stopTimer(CMDDV.timer1);
			midi.sendShortMsg(0x90, 0x44, 0x01);
		}
		//print('dbg');
}

CMDDV.crossLeft = function (value) {
		cur_val = engine.getValue('[Master]', 'crossfader');
		cur_val = cur_val - 0.01;
		engine.setValue('[Master]', 'crossfader', cur_val);
		if (cur_val <= -1) {
			if (CMDDV.timer1) engine.stopTimer(CMDDV.timer1);
			midi.sendShortMsg(0x90, 0x44, 0x01);
		}
		//print('dbg');
}

CMDDV.cut = function () {
	if (engine.getValue('[Master]', 'crossfader') < 0) {
		engine.setValue('[Channel2]', 'cue_gotoandplay', 1);
		engine.setValue('[Master]', 'crossfader', 1);
	} else if (engine.getValue('[Master]', 'crossfader') >= 0) {
		engine.setValue('[Channel1]', 'cue_gotoandplay', 1);
		engine.setValue('[Master]', 'crossfader', -1);
	}
	
}


CMDDV.cut2 = function () {
	script.toggleControl('[Channel1]', 'volume');
	script.toggleControl('[Channel2]', 'volume');
}

CMDDV.fade = function () {
	var v1 = engine.getValue('[Channel1]', 'volume');
	var v2 = engine.getValue('[Channel2]', 'volume');
	
	if (CMDDV.timer2 || CMDDV.timer2) {
		engine.stopTimer(CMDDV.timer2);
		engine.stopTimer(CMDDV.timer3);
		CMDDV.timer2 = null;
		CMDDV.timer3 = null;
		midi.sendShortMsg(0x90, 0x47, 0x01);
		return;
	}
	
	midi.sendShortMsg(0x90, 0x47, 0x02);
	if (v1 > 0) {
		CMDDV.timer2 = engine.beginTimer(50, CMDDV.volD1,  false); 
	} else {
		CMDDV.timer2 = engine.beginTimer(50, CMDDV.volU1, false); 
	}
	if (v2 > 0) {
		CMDDV.timer3 = engine.beginTimer(50, CMDDV.volD2,  false); 
	} else {
		CMDDV.timer3 = engine.beginTimer(50, CMDDV.volU2, false); 
	}
	
	
	
}

CMDDV.volU1 = function () {
		cur_val = engine.getValue('[Channel1]', 'volume');
		cur_val = cur_val + 0.01;
		engine.setValue('[Channel1]', 'volume', cur_val);
		if (cur_val >= 1) {
			if (CMDDV.timer2) engine.stopTimer(CMDDV.timer2);
			midi.sendShortMsg(0x90, 0x47, 0x01);
		}
		//print('dbg');
}

CMDDV.volD1 = function () {
		cur_val = engine.getValue('[Channel1]', 'volume');
		cur_val = cur_val - 0.01;
		engine.setValue('[Channel1]', 'volume', cur_val);
		if (cur_val <= 0) {
			if (CMDDV.timer2) engine.stopTimer(CMDDV.timer2);
			midi.sendShortMsg(0x90, 0x47, 0x01);
		}
		//print('dbg');
}

CMDDV.volU2 = function () {
		cur_val = engine.getValue('[Channel2]', 'volume');
		cur_val = cur_val + 0.01;
		engine.setValue('[Channel2]', 'volume', cur_val);
		if (cur_val >= 1) {
			if (CMDDV.timer3) engine.stopTimer(CMDDV.timer3);
			midi.sendShortMsg(0x90, 0x47, 0x01);
		}
		//print('dbg');
}

CMDDV.volD2 = function () {
		cur_val = engine.getValue('[Channel2]', 'volume');
		cur_val = cur_val - 0.01;
		engine.setValue('[Channel2]', 'volume', cur_val);
		if (cur_val <= 0) {
			if (CMDDV.timer3) engine.stopTimer(CMDDV.timer3);
			midi.sendShortMsg(0x90, 0x47, 0x01);
		}
		//print('dbg');
}
