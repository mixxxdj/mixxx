///////////////////////////////////////////////////////////////////////////////////
/*                                                                               */
/* Traktor Kontrol Z2 HID controller script v1.00                                */
/* Last modification: August 2020                                                */
/* Author: Jörg Wartenberg (based on the Traktor Z2 mapping by Owen Williams)    */
/* https://www.mixxx.org/wiki/doku.php/native_instruments_traktor_kontrol_Z2     */
/*                                                                               */
/* For linter:                                                                   */
/* global HIDController, HIDDebug, HIDPacket, controller                         */
///////////////////////////////////////////////////////////////////////////////////

var TraktorZ2 = new function() {
    this.controller = new HIDController();
		
    this.shiftPressed = false;
	
	this.fxButtonState = {1: false, 2: false, 3: false, 4: false};

    // When true, packets will not be sent to the controller.  Good for doing mass updates.
    this.batchingOutputs = false;
	
    // callbacks
    this.samplerCallbacks = [];
		
    // Knob encoder states (hold values between 0x0 and 0xF)
    // Rotate to the right is +1 and to the left is means -1
    this.browseKnobEncoderState = 0;
};

// Mixxx's javascript doesn't support .bind natively, so here's a simple version.
TraktorZ2.bind = function(fn, obj) {
    return function() {
        return fn.apply(obj, arguments);
    };
};

/////////////////////////
//// FX effect unit Objects ////
////
//// FX Effect Units don't have much state, just the fx button state.
TraktorZ2.EffectUnit = function(group) {
    this.group = group;
    this.fxEnabledState = false;
};

TraktorZ2.EffectUnit.prototype.fxEnableHandler = function(field) {
	HIDDebug("TraktorZ2: fxEnableHandler");
    if (field.value === 0) {
        return;
    }

    this.fxEnabledState = !this.fxEnabledState;
    //this.colorOutput(this.fxEnabledState, "!fxEnabled");
    TraktorZ2.toggleFX();
};

//// Deck Objects ////
// Decks are the loop controls and the 4 hotcue buttons on either side of the controller.
// Each Deck can control 2 channels a + c and b + d, which can be mapped.
TraktorZ2.Deck = function(deckNumber, group) {
    this.deckNumber = deckNumber;
    this.group = group;
    this.activeChannel = "[Channel" + deckNumber + "]";

    // Various states
    this.syncPressedTimer = 0;
	this.vinylcontrolTimer = 0;
    // padModeState 0 is hotcues, 1 is samplers
    this.padModeState = 0;

    // Knob encoder states (hold values between 0x0 and 0xF)
    // Rotate to the right is +1 and to the left is means -1
    this.loopKnobEncoderState = 0;
};

// defineScaler configures ranged controls like knobs and sliders.
TraktorZ2.Deck.prototype.defineScaler = function(msg, name, deckOffset, deckBitmask, deck2Offset, deck2Bitmask, fn) {
    if (this.deckNumber === 2) {
        deckOffset = deck2Offset;
        deckBitmask = deck2Bitmask;
    }
    TraktorZ2.registerInputScaler(msg, this.group, name, deckOffset, deckBitmask, TraktorZ2.bind(fn, this));
};
TraktorZ2.Deck.prototype.registerInputs = function(messageShort, messageLong) {
	HIDDebug("TraktorZ2: Deck.prototype.registerInputs");
    var deckFn = TraktorZ2.Deck.prototype;
	

    this.defineButton(messageShort, "!pad_1", 0x06, 0x04, 0x07, 0x08, deckFn.numberButtonHandler);
    this.defineButton(messageShort, "!pad_2", 0x06, 0x08, 0x07, 0x10, deckFn.numberButtonHandler);
    this.defineButton(messageShort, "!pad_3", 0x06, 0x10, 0x07, 0x20, deckFn.numberButtonHandler);
    this.defineButton(messageShort, "!pad_4", 0x06, 0x20, 0x07, 0x40, deckFn.numberButtonHandler);	
	
    // Vinyl control mode (REL / INTL)
    this.defineButton(messageShort, "vinylcontrol_mode", 0x04, 0x10, 0x04, 0x20, this.vinylcontrolHandler);
    this.defineButton(messageShort, "!sync", 0x04, 0x40, 0x04, 0x80, deckFn.syncHandler);

	// Load/Duplicate buttons
    this.defineButton(messageShort, "!LoadSelectedTrack", 0x04, 0x01, 0x04, 0x02, deckFn.loadTrackHandler);

    // Loop control
    this.defineButton(messageShort, "!SelectLoop", 0x01, 0xF0, 0x02, 0x0F, deckFn.selectLoopHandler);
    this.defineButton(messageShort, "!ActivateLoop", 0x05, 0x40, 0x08, 0x20, deckFn.activateLoopHandler);

    // Rev / Flux / Grid / Jog
    this.defineButton(messageShort, "!slip_enabled", 0x06, 0x40, 0x07, 0x80, deckFn.fluxHandler);

};

TraktorZ2.Deck.prototype.numberButtonHandler = function(field) {
    var padNumber = parseInt(field.id[field.id.length - 1]);
    var action = "";

    // Hotcues mode
    if (this.padModeState === 0) {
        if (this.shiftPressed) {
            action = "_clear";
        } else {
            action = "_activate";
        }
        HIDDebug("setting " + "hotcue_" + padNumber + action + " " + field.value);
        engine.setValue(this.activeChannel, "hotcue_" + padNumber + action, field.value);
        return;
    }

    // Samples mode
    var sampler = padNumber;
    if (field.group === "deck2") {
        sampler += 8;
    }

    if (this.shiftPressed) {
        var playing = engine.getValue("[Sampler" + sampler + "]", "play");
        if (playing) {
            action = "cue_default";
        } else {
            action = "eject";
        }
        engine.setValue("[Sampler" + sampler + "]", action, field.value);
        return;
    }
    var loaded = engine.getValue("[Sampler" + sampler + "]", "track_loaded");
    if (loaded) {
        if (field.value) {
            action = "cue_gotoandplay";
        } else {
            action = "stop";
        }
        engine.setValue("[Sampler" + sampler + "]", action, 1);
        return;
    }
    engine.setValue("[Sampler" + sampler + "]", "LoadSelectedTrack", field.value);
};

TraktorZ2.Deck.prototype.fluxHandler = function(field) {
    if (field.value === 0) {
        return;
    }
    script.toggleControl(this.activeChannel, "slip_enabled");
};

TraktorZ2.Deck.prototype.vinylcontrolHandler = function(field) {
	HIDDebug("TraktorZ2: vinylcontrolHandler");	
    var vinylcontrol_mode = engine.getValue(this.activeChannel, "vinylcontrol_mode");
	this.vinylcontrolTimer = engine.beginTimer(300, function() {
		if (vinylcontrol_mode >= 2) 
		{
			vinylcontrol_mode = 0;
		}
		else
		{
			vinylcontrol_mode++;
		}	
		engine.setValue(this.activeChannel, "vinylcontrol_mode", vinylcontrol_mode);
		// Reset vinylcontrol button timer state if active
		if (this.vinylcontrolTimer !== 0) {
			this.vinylcontrolTimer = 0;
		}
	}, true);
};

TraktorZ2.Deck.prototype.syncHandler = function(field) {
	HIDDebug("TraktorZ2: syncHandler");
    if (this.shiftPressed) {
        engine.setValue(this.activeChannel, "beatsync_phase", field.value);
        // Light LED while pressed
        //this.colorOutput(field.value, "sync_enabled");
        return;
    }

    // Unshifted
    if (field.value) {
        // We have to reimplement push-to-lock because it's only defined in the midi code
        // in Mixxx.
        if (engine.getValue(this.activeChannel, "sync_enabled") === 0) {
            script.triggerControl(this.activeChannel, "beatsync");
            // Start timer to measure how long button is pressed
            this.syncPressedTimer = engine.beginTimer(300, function() {
                engine.setValue(this.activeChannel, "sync_enabled", 1);
                // Reset sync button timer state if active
                if (this.syncPressedTimer !== 0) {
                    this.syncPressedTimer = 0;
                }
            }, true);

            // Light corresponding LED when button is pressed
            //this.colorOutput(1, "sync_enabled");
        } else {
            // Deactivate sync lock
            // LED is turned off by the callback handler for sync_enabled
            engine.setValue(this.activeChannel, "sync_enabled", 0);
        }
    } else {
        if (this.syncPressedTimer !== 0) {
            // Timer still running -> stop it and unlight LED
            engine.stopTimer(this.syncPressedTimer);
            //this.colorOutput(0, "sync_enabled");
        }
    }
};

TraktorZ2.selectTrackHandler = function(field) {
	HIDDebug("TraktorZ2: selectTrackHandler");
    var delta = 1;
    if ((field.value + 1) % 16 === this.browseKnobEncoderState) {
        delta = -1;
    }
    this.browseKnobEncoderState = field.value;

    // When preview is held, rotating the library encoder scrolls through the previewing track.
    if (this.previewPressed) {
        var playPosition = engine.getValue("[PreviewDeck1]", "playposition");
        if (delta > 0) {
            playPosition += 0.0125;
        } else {
            playPosition -= 0.0125;
        }
        engine.setValue("[PreviewDeck1]", "playposition", playPosition);
        return;
    }

    if (this.shiftPressed) {
        engine.setValue("[Library]", "MoveHorizontal", delta);
    } else {
        engine.setValue("[Library]", "MoveVertical", delta);
    }
};

TraktorZ2.LibraryFocusHandler = function(field) {
	HIDDebug("TraktorZ2: LibraryFocusHandler");
    //this.colorOutput(field.value, "!LibraryFocus");
    // if (field.value === 0) {
        // return;
    // }

    script.toggleControl("[Library]", "MoveFocus");
};

TraktorZ2.Deck.prototype.loadTrackHandler = function(field) {
    if (this.shiftPressed) {
        engine.setValue(this.activeChannel, "eject", field.value);
    } else {
        engine.setValue(this.activeChannel, "LoadSelectedTrack", field.value);
    }
};


// defineButton allows us to configure either the right deck or the left deck, depending on which
// is appropriate.  This avoids extra logic in the function where we define all the magic numbers.
// We use a similar approach in the other define funcs.
TraktorZ2.Deck.prototype.defineButton = function(msg, name, deckOffset, deckBitmask, deck2Offset, deck2Bitmask, fn) {
    if (this.deckNumber === 2) {
        deckOffset = deck2Offset;
        deckBitmask = deck2Bitmask;
    }
    TraktorZ2.registerInputButton(msg, this.group, name, deckOffset, deckBitmask, TraktorZ2.bind(fn, this));
};

TraktorZ2.Deck.prototype.selectLoopHandler = function(field) {
	HIDDebug("TraktorZ2: selectLoopHandler");
    if ((field.value + 1) % 16 === this.loopKnobEncoderState) {
        script.triggerControl(this.activeChannel, "loop_halve");
    } else {
        script.triggerControl(this.activeChannel, "loop_double");
    }

    this.loopKnobEncoderState = field.value;
};

TraktorZ2.Deck.prototype.activateLoopHandler = function(field) {
	HIDDebug("TraktorZ2: activateLoopHandler");
	if (field.value === 0) {
		return;
	}
	var isLoopActive = engine.getValue(this.activeChannel, "loop_enabled");

	if (this.shiftPressed) {
		engine.setValue(this.activeChannel, "reloop_toggle", field.value);
	} else {
		if (isLoopActive) {
			engine.setValue(this.activeChannel, "reloop_toggle", field.value);
		} else {
			engine.setValue(this.activeChannel, "beatloop_activate", field.value);
		}
	}
};

TraktorZ2.registerInputPackets = function() {
    var messageShort = new HIDPacket("shortmessage", 0x01, this.messageCallback);
    var messageLong = new HIDPacket("longmessage", 0x02, this.messageCallback);

	    HIDDebug("TraktorZ2: registerInputPackets");
    for (var idx in TraktorZ2.Decks) {
         var deck = TraktorZ2.Decks[idx];
         deck.registerInputs(messageShort, messageLong);
    }

    // this.registerInputButton(messageShort, "[Channel1]", "!switchDeck", 0x02, 0x02, this.deckSwitchHandler);
    // this.registerInputButton(messageShort, "[Channel2]", "!switchDeck", 0x05, 0x04, this.deckSwitchHandler);
    // this.registerInputButton(messageShort, "[Channel3]", "!switchDeck", 0x02, 0x04, this.deckSwitchHandler);
    // this.registerInputButton(messageShort, "[Channel4]", "!switchDeck", 0x05, 0x08, this.deckSwitchHandler);
	
	// Mic button
    this.registerInputButton(messageShort, "[Microphone]", "talkover", 0x05, 0x01, this.buttonHandler);

    // Headphone buttons
    this.registerInputButton(messageShort, "[Channel1]", "pfl", 0x04, 0x04, this.buttonHandler);
    this.registerInputButton(messageShort, "[Channel2]", "pfl", 0x04, 0x08, this.buttonHandler);
  
    this.registerInputButton(messageShort, "[Master]", "!shift", 0x07, 0x01, this.shiftHandler);
	
    this.registerInputButton(messageShort, "[Master]", "!SelectTrack", 0x01, 0x0F, this.selectTrackHandler);	
    this.registerInputButton(messageShort, "[Master]", "!LibraryFocus", 0x03, 0x80, this.LibraryFocusHandler);
   
    this.controller.registerInputPacket(messageShort);

   
   
    this.registerInputScaler(messageLong, "[Channel1]", "volume", 0x2D, 0xFFFF, this.parameterHandler); // Fader Deck A
    this.registerInputScaler(messageLong, "[Channel2]", "volume", 0x2F, 0xFFFF, this.parameterHandler); // Fader Deck B
    this.registerInputScaler(messageLong, "[Channel3]", "volume", 0x29, 0xFFFF, this.parameterHandler); // Rotary knob Deck C
    this.registerInputScaler(messageLong, "[Channel4]", "volume", 0x2B, 0xFFFF, this.parameterHandler); // Rotary knob Deck D

    this.registerInputScaler(messageLong, "[Master]", "duckStrengh", 0x03, 0xFFFF, this.parameterHandler); // Mic/Aux Tone knob, where no 1:1 mapping is available
    this.registerInputScaler(messageLong, "[Microphone]", "pregain", 0x01, 0xFFFF, this.parameterHandler);
	
    this.registerInputScaler(messageLong, "[Channel1]", "pregain", 0x11, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Channel2]", "pregain", 0x1F, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter3", 0x13, 0xFFFF, this.parameterHandler); // High
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter2", 0x15, 0xFFFF, this.parameterHandler); // Mid
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel1]_Effect1]", "parameter1", 0x17, 0xFFFF, this.parameterHandler); // Low

    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter3", 0x21, 0xFFFF, this.parameterHandler); // High
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter2", 0x23, 0xFFFF, this.parameterHandler); // Mid
    this.registerInputScaler(messageLong, "[EqualizerRack1_[Channel2]_Effect1]", "parameter1", 0x25, 0xFFFF, this.parameterHandler); // Low

    this.registerInputScaler(messageLong, "[QuickEffectRack1_[Channel1]]", "super1", 0x19, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[QuickEffectRack1_[Channel2]]", "super1", 0x27, 0xFFFF, this.parameterHandler);

    this.registerInputScaler(messageLong, "[Master]", "crossfader", 0x31, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "gain", 0x09, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "headMix", 0x07, 0xFFFF, this.parameterHandler);
    this.registerInputScaler(messageLong, "[Master]", "headGain", 0x05, 0xFFFF, this.parameterHandler);

    this.controller.registerInputPacket(messageLong);

    // Soft takeovers
    for (var ch = 1; ch <= 2; ch++) {
        group = "[Channel" + ch + "]";
        engine.softTakeover("[QuickEffectRack1_" + group + "]", "super1", true);
    }

    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter1", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel1]_Effect1]", "parameter3", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter1", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter2", true);
    engine.softTakeover("[EqualizerRack1_[Channel2]_Effect1]", "parameter3", true);
   
    // engine.softTakeover("[Master]", "crossfader", true);
    engine.softTakeover("[Master]", "gain", true);
    engine.softTakeover("[Master]", "headMix", true);
    engine.softTakeover("[Master]", "headGain", true);

    // Dirty hack to set initial values in the packet parser
    var data = [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
    TraktorZ2.incomingData(data);
};

TraktorZ2.registerInputScaler = function(message, group, name, offset, bitmask, callback) {
	HIDDebug("TraktorZ2: registerInputScaler");
    message.addControl(group, name, offset, "H", bitmask);
    message.setCallback(group, name, callback);
};

TraktorZ2.registerInputButton = function(message, group, name, offset, bitmask, callback) {
	HIDDebug("TraktorZ2: registerInputButton");
    message.addControl(group, name, offset, "B", bitmask);
    message.setCallback(group, name, callback);
};

TraktorZ2.shiftHandler = function(field) {
    HIDDebug("TraktorZ2: shiftHandler");
	engine.setValue("[Controls]", "touch_shift", field.value);
    this.shiftPressed = field.value;
   // TraktorZ2.basicOutputHandler(field.value, field.group, "!shift");
};

TraktorZ2.parameterHandler = function(field) {
	HIDDebug("TraktorZ2: parameterHandler");
    engine.setParameter(field.group, field.name, field.value / 4095);
};

// TraktorZ2.filterHandler = function(field) {
	// HIDDebug("TraktorZ2: filterHandler");
    // // The super knob drives all the supers!
    // var chan = TraktorZ2.Channels[field.group];
    // var value = field.value / 4095.;
    // // if (chan.fxEnabledState) {
        // // for (var fxNumber = 1; fxNumber <= 4; fxNumber++) {
            // // if (TraktorZ2.fxButtonState[fxNumber]) {
                // // engine.setParameter("[EffectRack1_EffectUnit" + fxNumber + "]", "super1", value);
            // // }
        // // }
    // // } else {
        // engine.setParameter("[QuickEffectRack1_" + chan.group + "]", "super1", value);
    // //}
// };

TraktorZ2.messageCallback = function(_packet, data) {
    for (var name in data) {
        if (Object.prototype.hasOwnProperty.call(data, name)) {
            TraktorZ2.controller.processButton(data[name]);
        }
    }
};

TraktorZ2.incomingData = function(data, length) {
    TraktorZ2.controller.parsePacket(data, length);
};

TraktorZ2.shutdown = function() {
    
    HIDDebug("TraktorZ2: Shutdown done!");
};

TraktorZ2.debugLights = function() {

    HIDDebug("TraktorZ2: debugLights");
    // Call this if you want to just send raw packets to the controller (good for figuring out what
    // bytes do what).
    var dataStrings = [
	    /* 0x80*/ 
        "70 " + // 0x01 3 bits (0x40, 0x20, 0x10) control Warning Symbol on top left brightness (orange)
		"70 " + // 0x02 3 bits (0x40, 0x20, 0x10) control Timecode-Vinyl Symbol on top right brightness (orange)
		"70 " + // 0x03 3 bits (0x40, 0x20, 0x10) control Snap-Button S brightness (blue)
		"70 " + // 0x04 3 bits (0x40, 0x20, 0x10) control Quantize-Button Q brightness (blue)
		"70 " + // 0x05 3 bits (0x40, 0x20, 0x10) control Settings-Button (Gear-Wheel-Symbol) brightness (orange)
		"00 " + // 0x06 
        "70 " + // 0x07 3 bits (0x40, 0x20, 0x10) control Deck A button brightness (blue)
		"70 " + // 0x08 3 bits (0x40, 0x20, 0x10) control Deck B button brightness (blue)
		"70 " + // 0x09 3 bits (0x40, 0x20, 0x10) control Deck C button brightness (white)
		"70 " + // 0x0A 3 bits (0x40, 0x20, 0x10) control Deck D button brightness (white)
		
		"70 " + // 0x0B 3 bits (0x40, 0x20, 0x10) control Deck C volume text label backlight brightness (white)
		"70 " + // 0x0C 3 bits (0x40, 0x20, 0x10) control Deck D volume text label backlight brightness (white)
		
        "70 " + // 0x0D 3 bits (0x40, 0x20, 0x10) control Macro FX1 On button brightness (orange)
        "70 " + // 0x0E 3 bits (0x40, 0x20, 0x10) control Deck 1 Flux button brightness (orange)
        "70 " + // 0x0F 3 bits (0x40, 0x20, 0x10) control Channel 1 FX1 select button brightness (orange)
		"70 " + // 0x10 3 bits (0x40, 0x20, 0x10) control Channel 1 FX2 select button brightness (orange)
		"70 " + // 0x11 3 bits (0x40, 0x20, 0x10) control Load A button brightness (orange)
		"70 " + // 0x12 3 bits (0x40, 0x20, 0x10) control vinylcontrol Rel/Intl A button brightness (orange)
		"70 " + // 0x13 3 bits (0x40, 0x20, 0x10) control vinylcontrol Rel/Intl A button brightness (green)
		"70 " + // 0x14 3 bits (0x40, 0x20, 0x10) control vinylcontrol Sync A button brightness (orange)
		
		"70 " + // 0x15 3 bits (0x40, 0x20, 0x10) control Macro FX2 On button brightness (orange)
        "70 " + // 0x16 3 bits (0x40, 0x20, 0x10) control Deck 2 Flux button brightness (orange)
        "70 " + // 0x17 3 bits (0x40, 0x20, 0x10) control Channel 2 FX1 select button brightness (orange)
		"70 " + // 0x18 3 bits (0x40, 0x20, 0x10) control Channel 2 FX2 select button brightness (orange)
		"70 " + // 0x19 3 bits (0x40, 0x20, 0x10) control Load B button brightness (orange)
		"70 " + // 0x1A 3 bits (0x40, 0x20, 0x10) control vinylcontrol Rel/Intl B button brightness (orange)
		"70 " + // 0x1B 3 bits (0x40, 0x20, 0x10) control vinylcontrol Rel/Intl B button brightness (green)
		"70 " + // 0x1C 3 bits (0x40, 0x20, 0x10) control vinylcontrol Sync B button brightness (orange)
		"FF 00 00 " + // 0x1D HotCue 1 Deck 1 RGB
        "FF 00 00 " + // 0x20 HotCue 2 Deck 1 RGB
        "00 00 00 " + // 0x23 HotCue 3 Deck 1 RGB
        "00 00 00 " + // 0x26 HotCue 4 Deck 1 RGB
        "34 34 34 " + // 0x29 HotCue 2 Deck 2 RGB
        "00 00 34 " + // 0x2C HotCue 2 Deck 2 RGB
        "00 FF 00 " + // 0x2F HotCue 3 Deck 2 RGB
        "00 00 34 " + // 0x32 HotCue 4 Deck 2 RGB
		
		"70 " + // 0x35 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment center horizontal bar brightness (orange)
		"70 " + // 0x36 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment lower right vertical bar brightness (orange)
		"70 " + // 0x37 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment upper right vertical bar brightness (orange)
		"70 " + // 0x38 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment upper horizontal bar brightness (orange)
		"70 " + // 0x39 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment upper left vertical bar brightness (orange)
		"70 " + // 0x3A 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment lower left vertical bar brightness (orange)
		"70 " + // 0x3B 3 bits (0x40, 0x20, 0x10) control Deck 1 1st 7 segment lower horizontal bar brightness (orange)
		
		"70 " + // 0x3C 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment center horizontal bar brightness (orange)
		"70 " + // 0x3D 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment lower right vertical bar brightness (orange)
		"70 " + // 0x3E 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment upper right vertical bar brightness (orange)
		"70 " + // 0x3F 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment upper horizontal bar brightness (orange)
		"70 " + // 0x40 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment upper left vertical bar brightness (orange)
		"70 " + // 0x41 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment lower left vertical bar brightness (orange)
		"70 " + // 0x42 3 bits (0x40, 0x20, 0x10) control Deck 1 2nd 7 segment lower horizontal bar brightness (orange)
		
		"70 " + // 0x43 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment center horizontal bar brightness (orange)
		"70 " + // 0x44 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment lower right vertical bar brightness (orange)
		"70 " + // 0x45 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment upper right vertical bar brightness (orange)
		"70 " + // 0x46 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment upper horizontal bar brightness (orange)
		"70 " + // 0x47 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment upper left vertical bar brightness (orange)
		"70 " + // 0x48 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment lower left vertical bar brightness (orange)
		"70 " + // 0x49 3 bits (0x40, 0x20, 0x10) control Deck 1 3rd 7 segment lower horizontal bar brightness (orange)
		
		"70 " + // 0x4A 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment center horizontal bar brightness (orange)
		"70 " + // 0x4B 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment lower right vertical bar brightness (orange)
		"70 " + // 0x4C 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment upper right vertical bar brightness (orange)
		"70 " + // 0x4D 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment upper horizontal bar brightness (orange)
		"70 " + // 0x4E 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment upper left vertical bar brightness (orange)
		"70 " + // 0x4F 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment lower left vertical bar brightness (orange)
		"70 " + // 0x50 3 bits (0x40, 0x20, 0x10) control Deck 2 1st 7 segment lower horizontal bar brightness (orange)
		
		"70 " + // 0x51 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment center horizontal bar brightness (orange)
		"70 " + // 0x52 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment lower right vertical bar brightness (orange)
		"70 " + // 0x53 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment upper right vertical bar brightness (orange)
		"70 " + // 0x54 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment upper horizontal bar brightness (orange)
		"70 " + // 0x55 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment upper left vertical bar brightness (orange)
		"70 " + // 0x56 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment lower left vertical bar brightness (orange)
		"70 " + // 0x57 3 bits (0x40, 0x20, 0x10) control Deck 2 2nd 7 segment lower horizontal bar brightness (orange)
		
		"70 " + // 0x58 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment center horizontal bar brightness (orange)
		"70 " + // 0x59 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment lower right vertical bar brightness (orange)
		"70 " + // 0x5A 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment upper right vertical bar brightness (orange)
		"70 " + // 0x5B 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment upper horizontal bar brightness (orange)
		"70 " + // 0x5C 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment upper left vertical bar brightness (orange)
		"70 " + //0x5D 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment lower left vertical bar brightness (orange)
		"70",   // 0x5E 3 bits (0x40, 0x20, 0x10) control Deck 2 3rd 7 segment lower horizontal bar brightness (orange)
 /* 0x81*/ "FF FF FF  FF FF FF FF  FF FF FF FF  FF FF FF FF " +
        "FF FF FF FF  FF FF FF FF  FF FF FF FF  FF FF FF FF " +
        "FF FF FF FF  FF FF FF FF  FF FF"
    ];

    var data = [Object(), Object()];

HIDDebug(data.length + "\n");
    for (var i = 0; i < data.length; i++) {
        var ok = true;
        var splitted = dataStrings[i].split(/\s+/);
        HIDDebug("i " + i + " " + splitted);
        data[i].length = splitted.length;
        HIDDebug(splitted.length + "\n");
        for (var j = 0; j < splitted.length; j++) {
            var byteStr = splitted[j];
            if (byteStr.length === 0) {
                continue;
            }
            if (byteStr.length !== 2) {
                ok = false;
                HIDDebug("not two characters?? " + byteStr);
            }
            var b = parseInt(byteStr, 16);
			HIDDebug("Byte " + j + ": " + byteStr + " " + b);
            if (b < 0 || b > 255) {
                ok = false;
                HIDDebug("number out of range: " + byteStr + " " + b);
            }
            data[i][j] = b;
        }
        if (ok) {
            var header = 0x80 + i;
            if (i === 2) {
                header = 0xD0;
            }
			HIDDebug(header + " " + data[i].length + "\n");
            controller.send(data[i], data[i].length, header);
        }
    }
};


// outputHandler drives lights that only have one color.
TraktorZ2.basicOutputHandler = function(value, group, key) {
    var ledValue = value;
    if (value === 0 || value === false) {
        // Off value
        ledValue = 0x00;
    } else if (value === 1 || value === true) {
        // On value
        ledValue = 0x07;
    }

    TraktorZ2.controller.setOutput(group, key, ledValue, !TraktorZ2.batchingOutputs);
};

TraktorZ2.registerOutputPackets = function() {
	HIDDebug("TraktorZ2: registerOutputPackets");
    var outputA = new HIDPacket("outputA", 0x80);
    var outputB = new HIDPacket("outputB", 0x81);

    for (var idx in TraktorZ2.Decks) {
        var deck = TraktorZ2.Decks[idx]; 
        //deck.registerOutputs(outputA, outputB);
    }

    outputA.addOutput("[Channel1]", "!deck_A", 0x07, "B");
    outputA.addOutput("[Channel2]", "!deck_B", 0x08, "B");
    outputA.addOutput("[Channel3]", "!deck_C", 0x09, "B");
    outputA.addOutput("[Channel4]", "!deck_D", 0x10, "B");

    outputA.addOutput("[Channel1]", "pfl", 0x39, "B");
    outputA.addOutput("[Channel2]", "pfl", 0x3A, "B");
	
    outputA.addOutput("[Channel1]", "sync_enabled", 0x14, "B", 0x70);	
    engine.connectControl("[Channel1]", "sync_enabled", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));
		
    outputA.addOutput("[Channel2]", "sync_enabled", 0x1C, "B", 0x70);	
    engine.connectControl("[Channel2]", "sync_enabled", TraktorZ2.bind(TraktorZ2.basicOutputHandler, this));

    outputA.addOutput("[ChannelX]", "!fxButton1", 0x3C, "B");
    outputA.addOutput("[ChannelX]", "!fxButton2", 0x3D, "B");
    outputA.addOutput("[ChannelX]", "!fxButton3", 0x3E, "B");
    outputA.addOutput("[ChannelX]", "!fxButton4", 0x3F, "B");
    outputA.addOutput("[ChannelX]", "!fxButton5", 0x40, "B");

    outputA.addOutput("[Channel3]", "!fxEnabled", 0x34, "B");
    outputA.addOutput("[Channel1]", "!fxEnabled", 0x35, "B");
    outputA.addOutput("[Channel2]", "!fxEnabled", 0x36, "B");
    outputA.addOutput("[Channel4]", "!fxEnabled", 0x59, "B");

    this.controller.registerOutputPacket(outputA);

    var VuOffsets = {
        "[Channel3]": 0x01,
        "[Channel1]": 0x10,
        "[Channel2]": 0x1F,
        "[Channel4]": 0x2E
    };
    for (var ch in VuOffsets) {
        for (var i = 0; i < 14; i++) {
            outputB.addOutput(ch, "!" + "VuMeter" + i, VuOffsets[ch] + i, "B");
        }
    }

    var MasterVuOffsets = {
        "VuMeterL": 0x3D,
        "VuMeterR": 0x46
    };
    for (i = 0; i < 8; i++) {
        outputB.addOutput("[Master]", "!" + "VuMeterL" + i, MasterVuOffsets["VuMeterL"] + i, "B");
        outputB.addOutput("[Master]", "!" + "VuMeterR" + i, MasterVuOffsets["VuMeterR"] + i, "B");
    }

    outputB.addOutput("[Master]", "PeakIndicatorL", 0x45, "B");
    outputB.addOutput("[Master]", "PeakIndicatorR", 0x4E, "B");

    outputB.addOutput("[Channel3]", "PeakIndicator", 0x0F, "B");
    outputB.addOutput("[Channel1]", "PeakIndicator", 0x1E, "B");
    outputB.addOutput("[Channel2]", "PeakIndicator", 0x2D, "B");
    outputB.addOutput("[Channel4]", "PeakIndicator", 0x3C, "B");

    this.controller.registerOutputPacket(outputB);

    for (idx in TraktorZ2.Decks) {
        deck = TraktorZ2.Decks[idx];
        //deck.linkOutputs();
    }

    for (idx in TraktorZ2.Channels) {
        var chan = TraktorZ2.Channels[idx];
        chan.linkOutputs();
    }

    // // Master VuMeters
    // this.masterVuConnections["VuMeterL"] = engine.makeConnection("[Master]", "VuMeterL", this.masterVuMeterHandler);
    // this.masterVuConnections["VuMeterR"] = engine.makeConnection("[Master]", "VuMeterR", this.masterVuMeterHandler);
    // this.linkChannelOutput("[Master]", "PeakIndicatorL", this.peakOutputHandler);
    // this.linkChannelOutput("[Master]", "PeakIndicatorR", this.peakOutputHandler);
};


TraktorZ2.init = function(_id) {
    this.Decks = {
        "deck1": new TraktorZ2.Deck(1, "deck1"),
        "deck2": new TraktorZ2.Deck(2, "deck2"),
    };

     this.EffectUnit = {
        "FX1": new TraktorZ2.EffectUnit("FX1"),
        "FX2": new TraktorZ2.EffectUnit("FX2")
    };
 
	TraktorZ2.debugLights(); 
    TraktorZ2.registerInputPackets();
    TraktorZ2.registerOutputPackets();
    HIDDebug("TraktorZ2: Init done!");
	
	var outputA = new HIDPacket("outputA", 0x80);
    var outputB = new HIDPacket("outputB", 0x81);

};