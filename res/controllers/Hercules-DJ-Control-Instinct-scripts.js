function HCInstinct() {};



// ----------   Global variables    ---------- 
HCInstinct.scratching = [false, false];
HCInstinct.pitchSpeedFast = true;     // temporary Pitch Speed of +/-  true = 
HCInstinct.vinylButton = false;            
HCInstinct.pitchSwitches = new Array();
HCInstinct.pitchSwitches["A"] = [0,0];
HCInstinct.pitchSwitches["B"] = [0,0];

HCInstinct.pitchB = [0,0];
// ----------   Functions    ----------

// called when the MIDI device is opened & set up
HCInstinct.init = function(id, debugging) {    
    HCInstinct.id = id;
    HCInstinct.FastPosition=[0,0];
    HCInstinct.jogFastPosition=[0,0];

    HCInstinct.allLedOff();

    // Switch-on some LEDs for improve the usability
    // midi.sendShortMsg(0x90, 46, 0x7F);    // Automix LED
    // midi.sendShortMsg(0x90, 14, 0x7F);    // Cue deck A LED
    // midi.sendShortMsg(0x90, 34, 0x7F);    // Cue deck B LED
    print ("***** Hercules DJ Instinct Control id: \""+id+"\" initialized.");
};

// Called when the MIDI device is closed
HCInstinct.shutdown = function(id) {
    HCInstinct.allLedOff();
    print ("***** Hercules DJ Instinct Control id: \""+id+"\" shutdown.");    
};


// === MISC TO MANAGE LEDS ===

HCInstinct.allLedOff = function () {
    // Switch off all LEDs
};

// Use VinylButton as "Shift"-Button
HCInstinct.vinylButtonHandler = function(channel,control, value, status) {
    if (value == ButtonState.pressed) {
    HCInstinct.vinylButton = true;
    }
    else {
    HCInstinct.vinylButton=false;
    }
};


// The button that enables/disables scratching
HCInstinct.wheelTouch0 = function (channel, control, value, status) {

    if (value == 0x7F && !HCInstinct.scratching[0]) { // catch only first touch
       var alpha = 1.0/8;
       var beta = alpha/32;
       engine.scratchEnable(1, 128, 33+1/3, alpha, beta);
       // Keep track of whether we're scratching on this virtual deck
       HCInstinct.scratching[0] = true;

    }
    else {    //  button up
        engine.scratchDisable(1);
        HCInstinct.scratching[0] = false;
    }

};
// The button that enables/disables scratching
HCInstinct.wheelTouch1 = function (channel, control, value, status) {

    if (value == 0x7F && !HCInstinct.scratching[1]) { // catch only first touch
       var alpha = 1.0/8;
       var beta = alpha/32;
       engine.scratchEnable(2, 128, 33+1/3, alpha, beta);
       // Keep track of whether we're scratching on this virtual deck
       HCInstinct.scratching[1] = true;

    }
    else {    //  button up
        engine.scratchDisable(2);
        HCInstinct.scratching[1] = false;
    }

};

 
HCInstinct.wheelTurn0 = function (channel, control, value, status) {
    
    // See if we're on scratching.
    //if (HCInstinct.scratching[0] == false )  return;
   
    var newValue;
    if (value-64 > 0) newValue = value-128; // 7F, 7E, 7D
    else newValue = value;
    engine.scratchTick(1,newValue);
};

HCInstinct.wheelTurn1 = function (channel, control, value, status) {
    
    // See if we're on scratching.
    if (HCInstinct.scratching[1] == false )  return;
   
    var newValue;
    if (value-64 > 0) newValue = value-128; // 7F, 7E, 7D
    else newValue = value;
    engine.scratchTick(2,newValue);
};

HCInstinct.knobIncrement = function (group, action, minValue, maxValue, centralValue, step, sign) {
    // This function allows you to increment a non-linear value like the volume's knob
    // sign must be 1 for positive increment, -1 for negative increment
    semiStep = step/2;
    rangeWidthLeft = centralValue-minValue;
    rangeWidthRight = maxValue-centralValue;
    actual = engine.getValue(group, action);
    
    if (actual < 1){
        increment = ((rangeWidthLeft)/semiStep)*sign;
    }
    else if (actual > 1){
        increment = ((rangeWidthRight)/semiStep)*sign;
    }
    else if (actual == 1){
        increment = (sign == 1) ? rangeWidthRight/semiStep : (rangeWidthLeft/semiStep)*sign;
    }

    if (sign == 1 && actual < maxValue){
        newValue = actual + increment;
    }
    else if (sign == -1 && actual > minValue){
        newValue = actual + increment;
    }
    
    return newValue;
};



// Pitch +/- 
HCInstinct.pitch = function (midino, control, value, status, group) {
    var speed = (HCInstinct.vinylButton == true) ? "" : "_small";
    var state = (value == 0x7F) ? 1 : 0;
    switch (control){
        case 0x11: HCInstinct.pitchSwitches["A"][0]=state;
            engine.setValue(group, "rate_temp_down"+speed, state); 
            break;
        case 0x12: HCInstinct.pitchSwitches["A"][1]=state;
            engine.setValue(group, "rate_temp_up"+speed, state); 
            break;
        case 0x2B: HCInstinct.pitchSwitches["B"][0]=state;
            engine.setValue(group, "rate_temp_down"+speed, state); 
            break;
        case 0x2C: HCInstinct.pitchSwitches["B"][1]=state;
            engine.setValue(group, "rate_temp_up"+speed, state); 
            break;
    };    
        // when buttons + and - pressed simultaneously
        if (HCInstinct.pitchSwitches["A"][0] && HCInstinct.pitchSwitches["A"][1]) {
        // reset pitch to 0
        engine.setValue(group, "rate", 0); 
    };
        if (HCInstinct.pitchSwitches["B"][0] && HCInstinct.pitchSwitches["B"][1]) {
        engine.setValue(group, "rate", 0); 
    }
};

// Up/Down-Siwtches 
HCInstinct.tempPitch = function (midino, control, value, status, group) {
    var rate = (value==0x7F) ? "rate_perm_down" : "rate_perm_up" ;
    if (HCInstinct.vinylButton == false) {
        rate = rate + "_small";
    }    
    engine.setValue(group, rate, 1);
    engine.setValue(group, rate, 0);
};
