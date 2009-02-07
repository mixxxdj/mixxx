function scratchTest() {}

// ----------   Global variables    ----------
// Variables used in the scratching alpha-beta filter: (revtime = 1.8 to start)
scratchTest.scratch = { "time":0.0, "track":0.0, "trackInitial":0.0, "slider":0, "scratch":0.0, "revtime":0.8, "alpha":0.1, "beta":0.1 };
scratchTest.deck = 1;  // Currently active virtual deck
scratchTest.modifier = { };  // Held-button modifiers

scratchTest.temp = { "channel":1, "device":"SCS.3d MIDI 1" };

// ----------   Functions   ----------

scratchTest.init = function () {   // called when the MIDI device is opened & set up
    print("ScratchTest: Init successful");
}

scratchTest.scratchSlider = function (channel, device, control, value) {   // Make scratching with a slider sound good
    // Slider is expected to return absolute position values (0-127)
    
    // Skip if the scratchEnable button isn't being held down
    if (scratchTest.modifier["scratch"] == 0) return;
    
    // Skip if the track start position hasn't been set yet
    if (scratchTest.scratch["track"] == 0.0) return;
    // If the slider start position hasn't been set yet, set it
    if (scratchTest.scratch["slider"] == 0) {
        scratchTest.scratch["slider"] = value;
        print("Initial slider="+scratchTest.scratch["slider"]);
        }
    // Set slider lights
    var add = scratchTest.Peak7(value,-1,128);
    var byte1 = 0xB0 + (scratchTest.temp["channel"]-1);
    midi.sendShortMsg(byte1,0x01,add,scratchTest.temp["device"]); //S4 LEDs
    midi.sendShortMsg(byte1,0x0C,add,scratchTest.temp["device"]); //S3 LEDs
    midi.sendShortMsg(byte1,0x0E,add,scratchTest.temp["device"]); //S5 LEDs

    // http://en.wikipedia.org/wiki/Alpha_beta_filter
    
    // 1: xhat(k) = xhat(k-1) + dT*v(k-1)
    // 2: vhat(k) = vhat(k-1)
    // 3: rhat(k) = x(k) - xhat(k)
    // 4: xhat(k) = xhat(k) + alpha * r[hat](k)
    // 5: vhat(k) = vhat(k) + (beta / dT) * r[hat](k)
    // 6: ahat(k) = ahat(k) + (gamma / [2*dT^2]) * r[hat](k)
    
    // ------------ My attempt:  -------------
//     var newTime = new Date()/1000;
//     var dt = newTime - scratchTest.scratch["time"];
//     scratchTest.scratch["time"] = newTime;
//     
//     // Project state estimates x and v using equations 1 and 2
//     scratchTest.scratch["track"] = scratchTest.scratch["track"] + dt * scratchTest.scratch["scratch"];
//     scratchTest.scratch["scratch"] = scratchTest.scratch["scratch"];
//     // Obtain a current measurement of the output value
//     // Compute the residual r using equation 3
//     var r = engine.getValue("[Channel"+scratchTest.deck+"]","playposition") * engine.getValue("[Channel"+scratchTest.deck+"]","duration") - scratchTest.scratch["track"];
//     // Correct the state estimates using equations 4 and 5
//     scratchTest.scratch["track"] = scratchTest.scratch["track"] + scratchTest.scratch["alpha"] * r;
//     scratchTest.scratch["scratch"] = scratchTest.scratch["scratch"] + (scratchTest.scratch["beta"] / dt) * r;
//     // Send updated x and optionally v as the filter outputs
//     //engine.setValue("[Channel"+scratchTest.deck+"]","playposition",scratchTest.scratch["track"]);
//     engine.setValue("[Channel"+scratchTest.deck+"]","scratch",scratchTest.scratch["scratch"]);
    // -------------------------
    
    // ------------- [Radio]Mark's attempt ------------------------
    // ideal position = (initial_p + (y - x) / 128 * 1.8)
    var ideal = (scratchTest.scratch["trackInitial"] + (value - scratchTest.scratch["slider"]) / 128 * scratchTest.scratch["revtime"]);
    
    var newTime = new Date()/1000;
    var dt = newTime - scratchTest.scratch["time"];
    scratchTest.scratch["time"] = newTime;
//     print("dt="+dt);

    // predicted_p = p + dt * pitch; // Where "pitch" = 1.0 for regular speed, i.e. the "scratch" control.
    var predicted_p = engine.getValue("[Channel"+scratchTest.deck+"]","playposition") * engine.getValue("[Channel"+scratchTest.deck+"]","duration") + dt * scratchTest.scratch["scratch"];
    // rx = where_finger_corresponds_to - predicted_p;
    var rx = ideal - predicted_p;
    // p += rx * ALPHA;
    scratchTest.scratch["track"] += rx * scratchTest.scratch["alpha"];
    // v += rx * BETA / dt;
    scratchTest.scratch["scratch"] = rx * scratchTest.scratch["beta"] / dt;
    
    print("Ideal position="+ideal+", Predicted position="+predicted_p + ", New track pos=" + scratchTest.scratch["track"] + ", New scratch val=" + scratchTest.scratch["scratch"]);
    
//     engine.setValue("[Channel"+scratchTest.deck+"]","playposition",scratchTest.scratch["track"]);
    engine.setValue("[Channel"+scratchTest.deck+"]","scratch",scratchTest.scratch["scratch"]);
}


scratchTest.scratchEnable = function (channel, device, control, value, category) {
    if (category != (0x80 + channel-1)) {    // If button down
        scratchTest.modifier["scratch"] = 1;   // Set the "scratch" button modifier flag
        // Store scratch info the point it was touched
        scratchTest.scratch["time"] = new Date()/1000;   // Current time in seconds
        scratchTest.scratch["trackInitial"] = scratchTest.scratch["track"] = engine.getValue("[Channel"+scratchTest.deck+"]","playposition") * engine.getValue("[Channel"+scratchTest.deck+"]","duration");    // Current position in seconds
        if (engine.getValue("[Channel"+scratchTest.deck+"]","play") > 0) scratchTest.scratch["scratch"] = -1.0;   // Stop the deck
        else scratchTest.scratch["scratch"] = 0.0;
//         scratchTest.scratch["track"] = 1.0; // for asantoni's algorithm
        
        print("Initial: time=" + scratchTest.scratch["time"] + "s, track=" + scratchTest.scratch["track"] + "s");
    
//         engine.setValue("[Channel"+scratchTest.deck+"]","play",0); // pause playback
        return;
    }
    // If button up,
    scratchTest.modifier["scratch"] = 1;   // Clear the "scratch" button modifier flag
    // Reset the triggers
    scratchTest.scratch["track"] = 0.0;
    scratchTest.scratch["trackInitial"] = 0.0;
    scratchTest.scratch["slider"] = 0;
    scratchTest.scratch["time"] = 0.0;
    scratchTest.scratch["scratch"] = 0.0;
    var byte1a = 0xB0 + (scratchTest.temp["channel"]-1);
    midi.sendShortMsg(byte1a,0x01,0x00,scratchTest.temp["device"]); //S4 LEDs off
    midi.sendShortMsg(byte1a,0x0C,0x00,scratchTest.temp["device"]); //S3 LEDs off
    midi.sendShortMsg(byte1a,0x0E,0x00,scratchTest.temp["device"]); //S5 LEDs off
    print("Scratch values CLEARED");
//     engine.setValue("[Channel"+scratchTest.deck+"]","play",1); // resume playback
    engine.setValue("[Channel"+scratchTest.deck+"]","scratch",0.0); // disable scratching
}

scratchTest.Peak7 = function (value, low, high) {
    var LEDs = 0;
    var range = (high-low)/7;
    if (value>low) LEDs++;
    if (value>low+range) LEDs++;
    if (value>low+range*2) LEDs++;
    if (value>low+range*3) LEDs++;
    if (value>low+range*4) LEDs++;
    if (value>low+range*5) LEDs++;
    if (value>low+range*6) LEDs++;
    if (value>=high) LEDs++;
    return LEDs;
}