// Functions common to all controllers go in this file

nop = function () {}    // Only here so you don't get a syntax error on load

function script() {}
script.debug = function (channel, device, control, value, category) {
    print("Script.Debug --- channel: " + channel + " device: " + device + " control: " + control + " value: " + value+ " category: " + category);
}

// Used to control a generic Mixxx control setting (low..high) from an absolute control (0..127)
script.absoluteSlider = function (group, key, value, low, high) {
    if (value==127) engine.setValue(group, key, high);
    else engine.setValue(group, key, ((high-low)/127)*value);
}

// Used to control an EQ setting (0..1..4) from an absolute control (0-127)
script.absoluteEQ = function (group, key, value) {
    if (value<=64) engine.setValue(group, key, value/64);
    else engine.setValue(group, key, 1+(value-63)/(21+1/3));
}

// ----------------- Scratching functions ---------------------
function scratch() {}
// Allows for smooth scratching with MIDI controllers
// See full details here: http://mixxx.org/wiki/doku.php/midi_scripting#available_common_functions

// ----------   Variables    ----------
scratch.variables = { "time":0.0, "trackPos":0.0, "initialTrackPos":0.0, "initialSlider":0, "scratch":0.0 };

// ----------   Functions   ----------

/* -------- ------------------------------------------------------
    scratch.Enable
   Purpose: Sets up initial variables for the scratch.<control>
            functions below. Called when a modifier button is pressed
            & held.
   Input:   Currently-controlled Mixxx deck
   Output:  -
   -------- ------------------------------------------------------ */
scratch.enable = function (currentDeck) {
    // Store scratch info at the point it was touched
    // Current position in seconds:
    scratch.variables["initialTrackPos"] = scratch.variables["trackPos"] = engine.getValue("[Channel"+currentDeck+"]","playposition") * engine.getValue("[Channel"+currentDeck+"]","duration");
    
    scratch.variables["time"] = new Date()/1000;   // Current time in seconds
    scratch.variables["scratch"] = 0.0; // Clear any scratching modifier
    
    // Stop the deck motion. This means we have to pause it if playing
    if (engine.getValue("[Channel"+currentDeck+"]","play") > 0) {
        scratch.variables["play"]=true;
//         engine.setValue("[Channel"+currentDeck+"]","play",0);   // pause playback
    }
    else scratch.variables["play"]=false;
    
    print("MIDI Script: Scratch initial: time=" + scratch.variables["time"] + "s, track=" + scratch.variables["trackPos"] + "s");
    return;
}

/* -------- ------------------------------------------------------
    scratch.Disable
   Purpose: Clears varaibles used by the scratch.<control> functions
            below. Called when the modifier button is released.
   Input:   Currently-controlled Mixxx deck
   Output:  -
   -------- ------------------------------------------------------ */
scratch.disable = function (currentDeck) {
    // Reset the triggers
    scratch.variables["trackPos"] = 0.0;
    scratch.variables["initialTrackPos"] = 0.0;
    scratch.variables["initialSlider"] = 0;
    scratch.variables["time"] = 0.0;
    scratch.variables["scratch"] = 0.0;
    print("MIDI Script: Scratch values CLEARED");
    engine.setValue("[Channel"+currentDeck+"]","scratch",0.0); // disable scratching
    if (scratch.variables["play"]) engine.setValue("[Channel"+currentDeck+"]","play",1); // resume playback
}

/* -------- ------------------------------------------------------
    scratch.Slider
   Purpose: Uses an alpha-beta filter to make scratching with a
            slider (0..127) sound good, called each time there's a
            new slider value
   Input:   Currently-controlled Mixxx deck, value of the slider,
            revolution time of the imaginary record (typically 1.8s,
            for a 12" disc @ 33+1/3 RPM,) alpha & beta coefficients
   Output:  New value for the "scratch" control
   -------- ------------------------------------------------------ */
scratch.slider = function (currentDeck, sliderValue, revtime, alpha, beta) {
    // Skip if the track start position hasn't been set yet
    if (scratch.variables["initialTrackPos"] == 0.0) return;
    // If the slider start value hasn't been set yet, set it
    if (scratch.variables["initialSlider"] == 0) {
        scratch.variables["initialSlider"] = sliderValue;
        print("Initial slider="+scratch.variables["initialSlider"]);
        }

    // ------------- Thanks to Radiomark (of Xwax) for the info for below ------------------------
    
    // ideal position = (initial_p + (y - x) / 128 * 1.8)
    var ideal_p = scratch.variables["initialTrackPos"] + (sliderValue - scratch.variables["initialSlider"]) / 128 * revtime;
    
    var currentTrackPos = engine.getValue("[Channel"+currentDeck+"]","playposition") * engine.getValue("[Channel"+currentDeck+"]","duration");
    var newTime = new Date()/1000;
    var dt = newTime - scratch.variables["time"];
    scratch.variables["time"] = newTime;
//     print("dt="+dt);
    
    // predicted_p = p + dt * pitch; // Where "pitch" = 1.0 for regular speed, i.e. the "scratch" control.
    var predicted_p = currentTrackPos + dt * scratch.variables["scratch"];
    // rx = where_finger_corresponds_to - predicted_p;
    var rx = ideal_p - predicted_p;
    
    // p += rx * ALPHA;
    // scratch.variables["trackPos"] += rx * alpha;   // Don't need this result so why waste the CPU time?
    
    // v += rx * BETA / dt;
    // scratch.variables["scratch"] += rx * (beta / dt);   // This doesn't work
    scratch.variables["scratch"] = rx * beta;
    
    print("MIDI Script: Ideal position="+ideal_p+", Predicted position="+predicted_p + ", New scratch val=" + scratch.variables["scratch"]);
    
//     var newPos = scratch.variables["trackPos"]/engine.getValue("[Channel"+currentDeck+"]","duration");
//     engine.setValue("[Channel"+currentDeck+"]","playposition",newPos);
//     engine.setValue("[Channel"+currentDeck+"]","scratch",scratch.variables["scratch"]);

    return scratch.variables["scratch"];
}
// ----------------- END Scratching functions ---------------------