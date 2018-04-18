function Hercules_Universal() {}

    Hercules_Universal.jog_timer = 0;

    Hercules_Universal.init = function () {

		//turn all leds off
		//Note: the manual says that code 127 with a value of 0x00
		//will do this in 1 message but it doesnt seem to work!
		for (var i = 1; i <= 127; i++) {
			midi.sendShortMsg(0x90, i, 0x00)
			midi.sendShortMsg(0x91, i, 0x00)
	    }

		//deck 1 controls
		engine.connectControl("[Channel1]","VuMeter","Hercules_Universal.vuLEDs1");
		engine.connectControl("[Channel1]","play_indicator","Hercules_Universal.playLED1");
		engine.connectControl("[Channel1]","cue_indicator","Hercules_Universal.cueLED1");
		engine.connectControl("[Channel1]","pfl","Hercules_Universal.pflLED1");
		engine.connectControl("[Channel1]","sync_enabled","Hercules_Universal.syncLED1");
			
		engine.connectControl("[Channel1]","hotcue_1_enabled", function(value, offset) { Hercules_Universal.hcue_deck1(value, 17); });
		engine.connectControl("[Channel1]","hotcue_2_enabled", function(value, offset) { Hercules_Universal.hcue_deck1(value, 18); });
		engine.connectControl("[Channel1]","hotcue_3_enabled", function(value, offset) { Hercules_Universal.hcue_deck1(value, 19); });
		engine.connectControl("[Channel1]","hotcue_4_enabled", function(value, offset) { Hercules_Universal.hcue_deck1(value, 20); });
		engine.connectControl("[Channel1]","hotcue_5_enabled", function(value, offset) { Hercules_Universal.hcue_deck1(value, 21); });
		engine.connectControl("[Channel1]","hotcue_6_enabled", function(value, offset) { Hercules_Universal.hcue_deck1(value, 22); });
		engine.connectControl("[Channel1]","hotcue_7_enabled", function(value, offset) { Hercules_Universal.hcue_deck1(value, 23); });
		engine.connectControl("[Channel1]","hotcue_8_enabled", function(value, offset) { Hercules_Universal.hcue_deck1(value, 24); });

		engine.connectControl("[Sampler1]","hotcue_1_enabled", function(value, offset) { Hercules_Universal.samp1(value, 33); });
		engine.connectControl("[Sampler1]","hotcue_2_enabled", function(value, offset) { Hercules_Universal.samp1(value, 34); });
		engine.connectControl("[Sampler1]","hotcue_3_enabled", function(value, offset) { Hercules_Universal.samp1(value, 35); });
		engine.connectControl("[Sampler1]","hotcue_4_enabled", function(value, offset) { Hercules_Universal.samp1(value, 36); });
		engine.connectControl("[Sampler2]","hotcue_1_enabled", function(value, offset) { Hercules_Universal.samp1(value, 37); });
		engine.connectControl("[Sampler2]","hotcue_2_enabled", function(value, offset) { Hercules_Universal.samp1(value, 38); });
		engine.connectControl("[Sampler2]","hotcue_3_enabled", function(value, offset) { Hercules_Universal.samp1(value, 39); });
		engine.connectControl("[Sampler2]","hotcue_4_enabled", function(value, offset) { Hercules_Universal.samp1(value, 40); });
		
		engine.connectControl("[Channel1]","beatloop_0.125_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck1(value, 49, 0); });
		engine.connectControl("[Channel1]","beatloop_0.25_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck1(value, 50, 0); });
		engine.connectControl("[Channel1]","beatloop_0.5_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck1(value, 51, 0); });
		engine.connectControl("[Channel1]","beatloop_1_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck1(value, 52, 0); });
		engine.connectControl("[Channel1]","beatloop_2_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck1(value, 53, 0); });
		engine.connectControl("[Channel1]","beatloop_4_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck1(value, 54, 0); });
		engine.connectControl("[Channel1]","beatloop_8_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck1(value, 55, 0); });
		engine.connectControl("[Channel1]","beatloop_16_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck1(value, 56, 0); });
		engine.connectControl("[Channel1]","loop_in", function(value, offset, shift) { Hercules_Universal.loop_deck1(value, 57, 1); });
		engine.connectControl("[Channel1]","loop_out", function(value, offset, shift) { Hercules_Universal.loop_deck1(value, 58, 1); });
		engine.connectControl("[Channel1]","loop_halve", function(value, offset, shift) { Hercules_Universal.loop_deck1(value, 59, 1); });
		engine.connectControl("[Channel1]","loop_double", function(value, offset, shift) { Hercules_Universal.loop_deck1(value, 60, 1); });
		engine.connectControl("[Channel1]","loop_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck1(value, 61, 1); });

		engine.connectControl("[EffectRack1_EffectUnit1]","enabled", function(value, offset, shift) { Hercules_Universal.fx_deck1(value, 1, 0); });	
		engine.connectControl("[EffectRack1_EffectUnit1]","group_[Master]_enable", function(value, offset, shift) { Hercules_Universal.fx_deck1(value, 2, 0); });
		engine.connectControl("[EffectRack1_EffectUnit1]","group_[Headphone]_enable", function(value, offset, shift) { Hercules_Universal.fx_deck1(value, 3, 0); });
		engine.connectControl("[EffectRack1_EffectUnit1]","group_[Channel1]_enable", function(value, offset, shift) { Hercules_Universal.fx_deck1(value, 4, 0); });
		engine.connectControl("[EffectRack1_EffectUnit1]","group_[Channel2]_enable", function(value, offset, shift) { Hercules_Universal.fx_deck1(value, 5, 0); });
		engine.connectControl("[EffectRack1_EffectUnit1]","group_[Channel3]_enable", function(value, offset, shift) { Hercules_Universal.fx_deck1(value, 6, 0); });
		engine.connectControl("[EffectRack1_EffectUnit1]","group_[Channel4]_enable", function(value, offset, shift) { Hercules_Universal.fx_deck1(value, 7, 0); });
        engine.connectControl("[EffectRack1_EffectUnit1]","prev_chain", function(value, offset, shift) { Hercules_Universal.fx_deck1(value, 9, 1); });
        engine.connectControl("[EffectRack1_EffectUnit1]","next_chain", function(value, offset, shift) { Hercules_Universal.fx_deck1(value, 10, 1); });	
		engine.connectControl("[EffectRack1_EffectUnit1]","clear", function(value, offset, shift) { Hercules_Universal.fx_deck1(value, 16, 1); });

		//deck 2 controls
		engine.connectControl("[Channel2]","VuMeter","Hercules_Universal.vuLEDs2");
		engine.connectControl("[Channel2]","play_indicator","Hercules_Universal.playLED2");
		engine.connectControl("[Channel2]","cue_indicator","Hercules_Universal.cueLED2");
		engine.connectControl("[Channel2]","pfl","Hercules_Universal.pflLED2");
		engine.connectControl("[Channel2]","sync_enabled","Hercules_Universal.syncLED2");
		
		engine.connectControl("[Channel2]","hotcue_1_enabled", function(value, offset) { Hercules_Universal.hcue_deck2(value, 17); });
		engine.connectControl("[Channel2]","hotcue_2_enabled", function(value, offset) { Hercules_Universal.hcue_deck2(value, 18); });
		engine.connectControl("[Channel2]","hotcue_3_enabled", function(value, offset) { Hercules_Universal.hcue_deck2(value, 19); });
		engine.connectControl("[Channel2]","hotcue_4_enabled", function(value, offset) { Hercules_Universal.hcue_deck2(value, 20); });
		engine.connectControl("[Channel2]","hotcue_5_enabled", function(value, offset) { Hercules_Universal.hcue_deck2(value, 21); });
		engine.connectControl("[Channel2]","hotcue_6_enabled", function(value, offset) { Hercules_Universal.hcue_deck2(value, 22); });
		engine.connectControl("[Channel2]","hotcue_7_enabled", function(value, offset) { Hercules_Universal.hcue_deck2(value, 23); });
		engine.connectControl("[Channel2]","hotcue_8_enabled", function(value, offset) { Hercules_Universal.hcue_deck2(value, 24); });

		engine.connectControl("[Sampler3]","hotcue_1_enabled", function(value, offset) { Hercules_Universal.samp2(value, 33); });
		engine.connectControl("[Sampler3]","hotcue_2_enabled", function(value, offset) { Hercules_Universal.samp2(value, 34); });
		engine.connectControl("[Sampler3]","hotcue_3_enabled", function(value, offset) { Hercules_Universal.samp2(value, 35); });
		engine.connectControl("[Sampler3]","hotcue_4_enabled", function(value, offset) { Hercules_Universal.samp2(value, 36); });
		engine.connectControl("[Sampler4]","hotcue_1_enabled", function(value, offset) { Hercules_Universal.samp2(value, 37); });
		engine.connectControl("[Sampler4]","hotcue_2_enabled", function(value, offset) { Hercules_Universal.samp2(value, 38); });
		engine.connectControl("[Sampler4]","hotcue_3_enabled", function(value, offset) { Hercules_Universal.samp2(value, 39); });
		engine.connectControl("[Sampler4]","hotcue_4_enabled", function(value, offset) { Hercules_Universal.samp2(value, 40); });

		engine.connectControl("[Channel2]","beatloop_0.125_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck2(value, 49, 0); });
		engine.connectControl("[Channel2]","beatloop_0.25_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck2(value, 50, 0); });
		engine.connectControl("[Channel2]","beatloop_0.5_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck2(value, 51, 0); });
		engine.connectControl("[Channel2]","beatloop_1_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck2(value, 52, 0); });
		engine.connectControl("[Channel2]","beatloop_2_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck2(value, 53, 0); });
		engine.connectControl("[Channel2]","beatloop_4_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck2(value, 54, 0); });
		engine.connectControl("[Channel2]","beatloop_8_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck2(value, 55, 0); });
		engine.connectControl("[Channel2]","beatloop_16_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck2(value, 56, 0); });
		engine.connectControl("[Channel2]","loop_in", function(value, offset, shift) { Hercules_Universal.loop_deck2(value, 57, 1); });
		engine.connectControl("[Channel2]","loop_out", function(value, offset, shift) { Hercules_Universal.loop_deck2(value, 58, 1); });
		engine.connectControl("[Channel2]","loop_halve", function(value, offset, shift) { Hercules_Universal.loop_deck2(value, 59, 1); });
		engine.connectControl("[Channel2]","loop_double", function(value, offset, shift) { Hercules_Universal.loop_deck2(value, 60, 1); });
		engine.connectControl("[Channel2]","loop_enabled", function(value, offset, shift) { Hercules_Universal.loop_deck2(value, 61, 1); });
		
		engine.connectControl("[EffectRack1_EffectUnit2]","enabled", function(value, offset, shift) { Hercules_Universal.fx_deck2(value, 1, 0); });	
		engine.connectControl("[EffectRack1_EffectUnit2]","group_[Master]_enable", function(value, offset, shift) { Hercules_Universal.fx_deck2(value, 2, 0); });
		engine.connectControl("[EffectRack1_EffectUnit2]","group_[Headphone]_enable", function(value, offset, shift) { Hercules_Universal.fx_deck2(value, 3, 0); });
		engine.connectControl("[EffectRack1_EffectUnit2]","group_[Channel1]_enable", function(value, offset, shift) { Hercules_Universal.fx_deck2(value, 4, 0); });
		engine.connectControl("[EffectRack1_EffectUnit2]","group_[Channel2]_enable", function(value, offset, shift) { Hercules_Universal.fx_deck2(value, 5, 0); });
		engine.connectControl("[EffectRack1_EffectUnit2]","group_[Channel3]_enable", function(value, offset, shift) { Hercules_Universal.fx_deck2(value, 6, 0); });
		engine.connectControl("[EffectRack1_EffectUnit2]","group_[Channel4]_enable", function(value, offset, shift) { Hercules_Universal.fx_deck2(value, 7, 0); });
        engine.connectControl("[EffectRack1_EffectUnit2]","prev_chain", function(value, offset, shift) { Hercules_Universal.fx_deck2(value, 9, 1); });
        engine.connectControl("[EffectRack1_EffectUnit2]","next_chain", function(value, offset, shift) { Hercules_Universal.fx_deck2(value, 10, 1); });	
		engine.connectControl("[EffectRack1_EffectUnit2]","clear", function(value, offset, shift) { Hercules_Universal.fx_deck2(value, 16, 1); });
		
	    //Enable the fx "enabled" leds at startup as they are enabled by default in Mixxx
		midi.sendShortMsg(0x90, 1, 0x7F);
		midi.sendShortMsg(0x91, 1, 0x7F);
	}
	
    Hercules_Universal.shutdown = function () {

		//turn all leds off
		//Note: the manual says that code 127 with a value of 0x00
		//will do this in 1 message but it doesnt seem to work!
		for (var i = 1; i <= 127; i++) {
			midi.sendShortMsg(0x90, i, 0x00)
			midi.sendShortMsg(0x91, i, 0x00)
	    }
	}	
	
	//Control the Deck 1 fx leds
	Hercules_Universal.fx_deck1 = function (value, offset, shift) {
		var state;
		var brightness;
		
		//Determine brightness based on shift button state
		brightness = shift ? 0x3F : 0x7F;
		
		//Determine if it should be illuminated or not
		state = value ? brightness : 0x00;
		
		// send the command
		midi.sendShortMsg(0x90, offset, state);
    }	
	
	//Control the Deck 2 fx leds
	Hercules_Universal.fx_deck2 = function (value, offset, shift) {
		var state;
		var brightness;
		
		//Determine brightness based on shift button state
		brightness = shift ? 0x3F : 0x7F;
		
		//Determine if it should be illuminated or not
		state = value ? brightness : 0x00;
		
		// send the command
		midi.sendShortMsg(0x91, offset, state);
    }	
	
	//Control the Deck 1 Loop leds
	Hercules_Universal.loop_deck1 = function (value, offset, shift) {
		var state;
		var brightness;

		//Determine brightness based on shift button state
		brightness = shift ? 0x3F : 0x7F;
		
		//Determine if it should be illuminated or not
		state = value ? brightness : 0x00;
		
		// send the command
		midi.sendShortMsg(0x90, offset, state);
    }	
	
	//Control the Deck 2 loop leds
	Hercules_Universal.loop_deck2 = function (value, offset, shift) {
		var state;
		var brightness;

		//Determine brightness based on shift button state
		brightness = shift ? 0x3F : 0x7F;
		
		//Determine if it should be illuminated or not
		state = value ? brightness : 0x00;
		
		//send the command
		midi.sendShortMsg(0x91, offset, state);

    }	
	
	//Control the hot cue leds for deck 1
	Hercules_Universal.hcue_deck1 = function (value, offset) {
		var state_full;
		var state_half;
		
		//Check if we want them illuminated or not and set the
		//relative brightness
		state_full = value ? 0x7F : 0x00;
		state_half = value ? 0x3F : 0x00;

		//send the commands
		//Note: All of the shift active leds are 8 positions higher
		midi.sendShortMsg(0x90, offset, state_full);
		midi.sendShortMsg(0x90, (offset + 8), state_half);  //shift led
    }	

	//Control the hot cue leds for deck 1
	Hercules_Universal.hcue_deck2 = function (value, offset) {
		var state_full;
		var state_half;
		
		//Check if we want them illuminated or not and set the
		//relative brightness		
		state_full = value ? 0x7F : 0x00;
		state_half = value ? 0x3F : 0x00;
		
		//send the commands
		//Note: All of the shift active leds are 8 positions higher
		midi.sendShortMsg(0x91, offset, state_full);
		midi.sendShortMsg(0x91, (offset + 8), state_half);  //shift led
    }

	//control the sample leds for deck 1
	Hercules_Universal.samp1 = function (value, offset) {
		var state_full;
		var state_half;

	    //Check if we want them illuminated or not and set the
		//relative brightness
		state_full = value ? 0x7F : 0x00;
		state_half = value ? 0x3F : 0x00;

		//send the commands
		//Note: All of the shift active leds are 8 positions higher
		midi.sendShortMsg(0x90, offset, state_full);
		midi.sendShortMsg(0x90, (offset + 8), state_half);  //shift led
    }

	//control the sample leds for deck 2	
	Hercules_Universal.samp2 = function (value, offset) {
		var state_full;
		var state_half;

	    //Check if we want them illuminated or not and set the
		//relative brightness
		state_full = value ? 0x7F : 0x00;
		state_half = value ? 0x3F : 0x00;

		//send the commands
		//Note: All of the shift active leds are 8 positions higher
		midi.sendShortMsg(0x91, offset, state_full);
		midi.sendShortMsg(0x91, (offset + 8), state_half);  //shift led
    }	
	
	//Control the sync led (deck 1)
	Hercules_Universal.syncLED1 = function (value) {
		var state;
		state = value ? 0x7F : 0x00;
		midi.sendShortMsg(0x90, 67, state);

    }
	
	//control the sync led (deck 2)
	Hercules_Universal.syncLED2 = function (value) {
		var state;
		state = value ? 0x7F : 0x00;
		midi.sendShortMsg(0x91, 67, state);

    }	
	
	//control the pfl (headphone) led (deck 1)
	Hercules_Universal.pflLED1 = function (value) {
		var state;
		state = value ? 0x7F : 0x00;
		midi.sendShortMsg(0x90, 83, state);

    }		

	//control the pfl (headphone) led (deck 2)
	Hercules_Universal.pflLED2 = function (value) {
		var state;
		state = value ? 0x7F : 0x00;
		midi.sendShortMsg(0x91, 83, state);

    }	
	
	//control the cue led (deck 1)
	Hercules_Universal.cueLED1 = function (value) {
		var state;
		state = value ? 0x7F : 0x00;
		midi.sendShortMsg(0x90, 66, state);

    }
	
	//control the cue led (deck 2)
	Hercules_Universal.cueLED2 = function (value) {
		var state;
		state = value ? 0x7F : 0x00;
		midi.sendShortMsg(0x91, 66, state);

    }
	
	//control the play led (deck 1)
    Hercules_Universal.playLED1 = function (value) {
		var state;
		state = value ? 0x7F : 0x00;
		midi.sendShortMsg(0x90, 65, state);

    }
	
	//control the deck led (deck 2)
	Hercules_Universal.playLED2 = function (value) {
		var state;
		state = value ? 0x7F : 0x00;
		midi.sendShortMsg(0x91, 65, state);

    }
	
	//control the VU meter (deck 1)
	Hercules_Universal.vuLEDs1 = function (value) {
        var comp1;
			
		//The control message expects a value between 1 and 9
		//to control the leds but the returned value is a float 
		//between 0 and 1.
		//Solution is to scale by 8 and add the one offset. If
		//we then remove anything after the decimal point it
		//should result in a value between 1 and 9.....
		comp1 = Math.floor((value * 8) + 1);
		
		//clamp the value at 9 if the value is too large
		if (comp1 > 9) { comp1 = 9 }

		//send the command
		midi.sendShortMsg(0x90, 105, comp1);

    }
	
	//control the VU meter (deck 2)
	Hercules_Universal.vuLEDs2 = function (value) {
		var comp2;

		//The control message expects a value between 1 and 9
		//to control the leds but the returned value is a float 
		//between 0 and 1.
		//Solution is to scale by 8 and add the one offset. If
		//we then remove anything after the decimal point it
		//should result in a value between 1 and 9.....
		comp2 = Math.floor((value * 8) + 1);

		//clamp the value at 9 if the value is too large
		if (comp2 > 9) { comp2 = 9 }

		//send the command
		midi.sendShortMsg(0x91, 105, comp2);

    }

	//timer to stop accidental jog control
    function doTimer() {
	    Hercules_Universal.jog_timer = 0;
	}

    //Determine which wheel is currently active
	Hercules_Universal.deck=function(group){
	 //channel 1 -->deck 0
	 //channel 2 -->deck 1
	 return (group=="[Channel1]") ? 1 : 2;
	}

    // The button that enables/disables scratching
    Hercules_Universal.wheelTouch = function (channel, control, value, status, group) {

        if (value == 0x7F) {  // Some wheels send 0x90 on press and release, so you need to check the value
            var alpha = 1.0/8;
            var beta = alpha/32;
			//the 500 count adjusts the sensitivity. 500 equates to a 1 to 1 rotation map
            engine.scratchEnable(Hercules_Universal.deck(group), 500, 33+1/3, alpha, beta);
        } else {    // If button up
            engine.scratchDisable(Hercules_Universal.deck(group));
        }
    }
     
    // The wheel that actually controls the scratching
    Hercules_Universal.wheelTurn = function (channel, control, value, status, group) {
        		
		// See if we're scratching. If not, skip this.
        if (!engine.isScratching(Hercules_Universal.deck(group))) return;
     
        // --- Choose only one of the following!
     
        // A: For a control that centers on 0:
        var newValue;
        if (value-64 > 0) newValue = value-128;
        else newValue = value;
		
		// enable a timer to disable jog for 'x' milliseconds to stop runaway
		// when the capacative pad is released and the wheel is still spinning
		if (Hercules_Universal.jog_timer === 0) {
			
			Hercules_Universal.jog_timer = engine.beginTimer(2000, "doTimer", true);
		}
          
        // --- End choice
     
        // In either case, register the movement
        engine.scratchTick(Hercules_Universal.deck(group),newValue);
    }

    // The wheel that actually controls the scratching
    Hercules_Universal.wheelJog = function (channel, control, value, status, group){	
		
		if (Hercules_Universal.jog_timer == 0){
			
			// A: For a control that centers on 0:
			var newValue;
			if (value-64 > 0) newValue = value-128;
			else newValue = value;
			
			engine.setValue(group, "jog", newValue);
		}
	}
	
	//rotary control above pitch slider when cue is enabled
    Hercules_Universal.gain = function (channel, control, value, status, group){	
	
		var gain;

		//get the current gain value
		gain = engine.getValue(group, "pregain");

		//Check if we are trying to increase the gain value
		if (value == 1){
			gain = gain + 0.1; //increment the gain
		}
		else {
			gain = gain - 0.1; //decrement the gain
		}

		//clamp the variable to stop it exceeding limits
		if (gain < 0) {gain = 0};
		if (gain > 4) {gain = 4};

		//tell mixxx the new value
		engine.setValue(group, "pregain", gain);
	}
	
	//rotary control above pitch slider when fx is enabled
    Hercules_Universal.mix = function (channel, control, value, status, group){	
	
		var mix_pot;

		//get the current mix value
		mix_pot = engine.getValue(group, "mix");

		//check to see if we are trying to increase the mix value
		if (value == 1){
			mix_pot = mix_pot + 0.03; //increment the mix value
		}
		else {
			mix_pot = mix_pot - 0.03; //decrement the mix value
		}

		//clamp the variable to stop it exceeding limits
		if (mix_pot < 0) {mix_pot = 0};
		if (mix_pot > 1) {mix_pot = 1};

		//tell mixxxx the new value
		engine.setValue(group, "mix", mix_pot);
	}
	
	