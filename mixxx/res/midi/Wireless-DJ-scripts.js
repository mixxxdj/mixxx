function WirelessDJ() {}

WirelessDJ.init = function(id) {
    //tuning constants
    WirelessDJ.playPositionSensibility = 3.5;
    WirelessDJ.jogSensibility = -30;
    WirelessDJ.fineTempoTuningSensibility = 1;
    
    // internals below. don't touch. 
	WirelessDJ.magicStripeMode = [0, 0];
    
    WirelessDJ.magicCurMSB = [0, 0];
    WirelessDJ.prevMagicValue = [undefined, undefined];
    
    WirelessDJ.leds = [0,0,0,0];
    WirelessDJ.ledTimers = [0,0,0,0];
    
    // LEDs
    engine.connectControl("[Channel1]", "VuMeter", "WirelessDJ.meter");
    engine.connectControl("[Channel2]", "VuMeter", "WirelessDJ.meter");
    engine.connectControl("[Master]", "VuMeterL", "WirelessDJ.meter");
    engine.connectControl("[Master]", "VuMeterR", "WirelessDJ.meter");
    
    // sliders feedback
    engine.connectControl("[Channel1]", "rate", "WirelessDJ.controlFeedback");
    engine.connectControl("[Channel1]", "volume", "WirelessDJ.controlFeedback");
    engine.connectControl("[Channel1]", "filterHigh", "WirelessDJ.controlFeedback");
    engine.connectControl("[Channel1]", "filterMid", "WirelessDJ.controlFeedback");
    engine.connectControl("[Channel1]", "filterLow", "WirelessDJ.controlFeedback");
    engine.connectControl("[Channel2]", "rate", "WirelessDJ.controlFeedback");
    engine.connectControl("[Channel2]", "volume", "WirelessDJ.controlFeedback");
    engine.connectControl("[Channel2]", "filterHigh", "WirelessDJ.controlFeedback");
    engine.connectControl("[Channel2]", "filterMid", "WirelessDJ.controlFeedback");
    engine.connectControl("[Channel2]", "filterLow", "WirelessDJ.controlFeedback");
    engine.connectControl("[Master]", "crossfader", "WirelessDJ.controlFeedback");
    engine.connectControl("[Master]", "headMix", "WirelessDJ.controlFeedback");
}

WirelessDJ.shutdown = function(id) {
    engine.connectControl("[Channel1]", "VuMeter", "WirelessDJ.meter", true);
    engine.connectControl("[Channel2]", "VuMeter", "WirelessDJ.meter", true);
    engine.connectControl("[Master]", "VuMeterL", "WirelessDJ.meter", true);
    engine.connectControl("[Master]", "VuMeterR", "WirelessDJ.meter", true);
    
    engine.connectControl("[Channel1]", "rate", "WirelessDJ.controlFeedback", true);
    engine.connectControl("[Channel1]", "volume", "WirelessDJ.controlFeedback", true);
    engine.connectControl("[Channel1]", "filterHigh", "WirelessDJ.controlFeedback", true);
    engine.connectControl("[Channel1]", "filterMid", "WirelessDJ.controlFeedback", true);
    engine.connectControl("[Channel1]", "filterLow", "WirelessDJ.controlFeedback", true);
    engine.connectControl("[Channel2]", "rate", "WirelessDJ.controlFeedback", true);
    engine.connectControl("[Channel2]", "volume", "WirelessDJ.controlFeedback", true);
    engine.connectControl("[Channel2]", "filterHigh", "WirelessDJ.controlFeedback", true);
    engine.connectControl("[Channel2]", "filterMid", "WirelessDJ.controlFeedback", true);
    engine.connectControl("[Channel2]", "filterLow", "WirelessDJ.controlFeedback", true);
    engine.connectControl("[Master]", "crossfader", "WirelessDJ.controlFeedback", true);
    engine.connectControl("[Master]", "headMix", "WirelessDJ.controlFeedback", true);
}

WirelessDJ.groupToDeck = function(group) {
    var the_char = group.charAt(8);
    
	if (the_char == '1') {
		return 0;
	} else if (the_char == '2') {
		return 1;
	} else {
        return -1;
    }
}

// we need update LED value every second, or they will switch off.
WirelessDJ.sendLED = function(index, value) {
    var date = new Date();
    var curTime = date.getTime();
    
    if (WirelessDJ.leds[index] != value || (curTime - WirelessDJ.ledTimers[index]) > 900) {
        WirelessDJ.leds[index] = value;
        midi.sendShortMsg(0x90, 0x0a + index, value);
        
        WirelessDJ.ledTimers[index] = curTime;
//        script.debug("", "", "", "", curTime);
    }
}

WirelessDJ.meter = function(value, group, key) { 
    var deck = WirelessDJ.groupToDeck(group);
    // there is 11 leds in WirelessDJ volume meter.

    var val = Math.round(parseFloat(value) * 11) * 128.0/11.0;     

    if (deck == 0) {
        WirelessDJ.sendLED(0, val);
    } else if (deck == 1) {
        WirelessDJ.sendLED(3, val);
    } else {
        if (key == "VuMeterL") {
            WirelessDJ.sendLED(1, val);
        } else {
            WirelessDJ.sendLED(2, val);
        }
    }
}

WirelessDJ.controlFeedback = function(value, group, key) { 
    var deck = WirelessDJ.groupToDeck(group);

    if (key == "rate") {
        midi.sendShortMsg(0xb0 + deck, 0x14, (value/2 + 0.5)*127);
    } else if (key == "volume") {
        midi.sendShortMsg(0xb0 + deck, 0x15, value*127);
    } else if (key == "crossfader") {
        midi.sendShortMsg(0xb0, 0x1a, (value/2 + 0.5)*127);
    } else if (key == "headMix") {
        midi.sendShortMsg(0xb0, 0x1b, (value/2 + 0.5)*127);
    } else {
        if (value <= 1) {
            value /= 2;
        } else {
            value = (value - 1) / 6 + 0.5;
        }
        if (key == "filterHigh") {
            midi.sendShortMsg(0xb0 + deck, 0x17, value*127);
        } else if (key == "filterMid") {
            midi.sendShortMsg(0xb0 + deck, 0x18, value*127);
        } else if (key == "filterLow") {
            midi.sendShortMsg(0xb0 + deck, 0x19, value*127);
        }
    }
}

WirelessDJ.seek_on = function(channel, control, value, status, group) {
    var deck = WirelessDJ.groupToDeck(group);
    
    if (status == 0x90) {
        WirelessDJ.magicStripeMode[deck] |= 0x1;
    } else {
        WirelessDJ.magicStripeMode[deck] &= ~0x1;
    }
}

WirelessDJ.jog_on = function(channel, control, value, status, group) {
    var deck = WirelessDJ.groupToDeck(group);
    
    if (status == 0x90) {
        WirelessDJ.magicStripeMode[deck] |= 0x2;
    } else {
        WirelessDJ.magicStripeMode[deck] &= ~0x2;
        var position = engine.getValue(group, "wheel");
        if (position != 0) {
            engine.setValue(group, "wheel", 0);
        }
    }
}

WirelessDJ.tempo_tuning = function(channel, control, value, status, group) {
    var deck = WirelessDJ.groupToDeck(group);
    
    if (status == 0x90) {
        WirelessDJ.magicStripeMode[deck] |= 0x4;
     //           engine.setValue(group, "rate", 1);
    } else {
        WirelessDJ.magicStripeMode[deck] &= ~0x4;
    //            engine.setValue(group, "rate", -1);
    }
}

WirelessDJ.magic_stripe_msb = function(channel, control, value, status, group) {
    var deck = WirelessDJ.groupToDeck(group);
    
    WirelessDJ.magicCurMSB[deck] = value;
};

WirelessDJ.magic_stripe_lsb = function(channel, control, value, status, group) {
    var deck = WirelessDJ.groupToDeck(group);

    if (WirelessDJ.magicCurMSB[deck] == undefined)
        return;
    
    var firstValue = (WirelessDJ.prevMagicValue[deck] == undefined);
        
    var adjustedValue = (WirelessDJ.magicCurMSB[deck] * 128 + value) / 16384.0;
    var diff = WirelessDJ.prevMagicValue[deck] - adjustedValue;
    if (diff > 0.9)
        diff -= 1;
    if (diff < -0.9)
        diff += 1;
    WirelessDJ.prevMagicValue[deck] = adjustedValue;
    
    if (firstValue)
        return;
    
    if (WirelessDJ.magicStripeMode[deck] & 0x1) {
        var position = engine.getValue(group, "playposition") + diff * WirelessDJ.playPositionSensibility;
        
        if (position < 0) {
            position = 0;
        } else if (position > 1) {
            position = 1;
        }
        
        engine.setValue(group, "playposition", position);
    } else if (WirelessDJ.magicStripeMode[deck] & 0x4) {
        var position = engine.getValue(group, "rate") + diff * WirelessDJ.fineTempoTuningSensibility;
        
        if (position < -1) {
            position = -1;
        } else if (position > 1) {
            position = 1;
        }
        
        engine.setValue(group, "rate", position);
        
        script.debug("pos", "", "", "", position );
        script.debug("diff", "", "", "", diff );
    } else {
        //        script.debug(WirelessDJ.prevMagicValue[deck], adjustedValue, "", "", diff );
        
        engine.setValue(group, "wheel", diff * WirelessDJ.jogSensibility);
    }
};