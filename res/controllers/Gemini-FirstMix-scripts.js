/**
 * Gemini FirstMix controller script v1.0
 * For Mixxx 1.10.0+
 * Written by Adam Marcus 2012
 *
 * Button mapping details:
 * 
 * Browse & A/B Load buttons: The browse button will scroll through the library. Pressing the Browse button will load the selected track to whatever deck is not currently playing. 
 * The Load A and Load B buttons will load the selected track to that deck (but only if the deck isn't already playing something).
 * 
 * Preview button: This button will load the currently-selected track into sampler 4 and adjust the volume of sampler 4 so that it only plays through the headphones. 
 * This works great except it marks the track as played. I don't see a Mixxx function to toggle the played state for tracks, so there's nothing I can do about that.
 * To see the sampler, click on the word "Sampler" above the main VU meters in the center of the Mixxx window.
 * 
 * EFX/fader knobs: Since the EFX function in Mixx is kinda useless (only a flanger effect that doesn't sound that different), the EFX knobs adjust the gain, 
 * the gain knobs to adjust the high fader and the treble knobs to adjust the mid fader. This way, you get high/mid/low faders.
 * 
 * EFX buttons & Master volume: The EFX buttons are mapped to send that deck to the headphones (a.k.a PFL function). The buttons light up to indicate PFL state.
 * The master volume knob is mapped to headMix (the cue/main mix in the headphones). If you have two soundcards, this will allow you to cross-fade through the headphones before you do it live.
 * 
 * Play/Cue button blinking: The play button will blink on every beat when the track is playing. Starting at secondsBlink seconds (default is 30) before the end of the track, the CUE button 
 * also blinks to warn you that the track is about to end.
 * 
 * Jogwheel: Normally (when the Scratch button for the deck is not lit), the jogwheels will "nudge" the playing track, speeding it up or slowing it down slightly.
 * Pressing the Scratch button will toggle the scratch mode. In scratch mode, the jogwheels will "scratch" the playing track but only while you are touching them (because the jogwheels are touch-sensitive).
 * 
 * Sel(ect) buttons: I didn't know what to do with these buttons so I left them undefined.
 * 
 **/

// Seconds to the end of track after which cue button blink (default = 30)
secondsBlink = 30;

// The alpha value for the scratch filter (start with 1/8 (0.125) and tune from there)
alpha = .13;

// The beta value for the scratch filter (start with alpha/32 and tune from there)
beta = alpha/32;

// pitch shift mode
gammaInputRange = 12;    // Max jog speed
maxOutFraction = 0.5;    // Where on the curve it should peak; 0.5 is half-way
sensitivity = 0.9;        // .5 Adjustment gamma
gammaOutputRange = 3;    // Max rate change



function firstmix() {}

firstmix.init = function (channel, control, value, status, group) {
    for (i=0x33; i<=0x60; i++) midi.sendShortMsg(0x90,i,0x00);  // Turn off all LEDs

    firstmix.scratchButton = [false,false];
    firstmix.efxButton = [false,false];
    firstmix.touchingWheel = [false,false];
    firstmix.scratchTimer = [-1, -1];
    firstmix.previewButton = false;

    // Stutter beat light
    engine.connectControl("[Channel1]","beat_active","firstmix.Stutter1Beat");
    engine.connectControl("[Channel2]","beat_active","firstmix.Stutter2Beat");

    firstmix.leds = [
        // Common
        { "preview": 0x60 },
        // Deck 1
        { "play": 0x4a, "cue": 0x3b, "efx": 0x43, "scratch": 0x48, "rev": 0x40, "sync": 0x33 },
        // Deck 2
        { "play": 0x4c, "cue": 0x42, "efx": 0x45, "scratch": 0x35, "rev": 0x47, "sync": 0x3c },
    ];

    // Set controls in Mixxx to reflect settings on the device
    // I've tried both of the following commands but they don't work. Not sure if this is possible.
    // midi.sendShortMsg(0xB0,0x64,0x7F);
    // midi.sendShortMsg(0xB0,0x7F,0x7F);

    // Enable soft-takeover for all direct hardware controls
    engine.softTakeover("[Master]","crossfader",true);
    engine.softTakeover("[Master]","headMix",true);
    engine.softTakeover("[Channel1]","pregain",true);
    engine.softTakeover("[Channel1]","filterHigh",true);
    engine.softTakeover("[Channel1]","filterMed",true);
    engine.softTakeover("[Channel1]","filterLow",true);
    engine.softTakeover("[Channel2]","pregain",true);
    engine.softTakeover("[Channel2]","filterHigh",true);
    engine.softTakeover("[Channel2]","filterMed",true);
    engine.softTakeover("[Channel2]","filterLow",true);
}

firstmix.shutdown = function () {
    for (i=0x33; i<=0x60; i++) midi.sendShortMsg(0x90,i,0x00);  // Turn off all LEDs
}

// ----------------
// Helper functions
// ----------------

firstmix.setLED = function(value, status) {
    status = status ? 0x64 : 0x00;
    midi.sendShortMsg(0x90, value, status);
}

firstmix.currentDeck = function (group) {
    if (group == "[Channel1]")
        return 1;
    else if (group == "[Channel2]")
        return 2;
    print("Invalid group : " + group);
        midi.sendShortMsg(0x90,0x45,0x7F);    // Turn on right EFX LED
    return -1; // error
}

firstmix.Stutter1Beat = function (value) {
        var secondsToEnd = engine.getValue("[Channel1]", "duration") * (1-engine.getValue("[Channel1]", "playposition"));
    if (secondsToEnd < secondsBlink && secondsToEnd > 1 && engine.getValue("[Channel1]", "play")) { // The song is going to end
        firstmix.setLED(firstmix.leds[1]["cue"], value);
    }
    firstmix.setLED(firstmix.leds[1]["play"], value);
}

firstmix.Stutter2Beat = function (value) {
        var secondsToEnd = engine.getValue("[Channel2]", "duration") * (1-engine.getValue("[Channel2]", "playposition"));
    if (secondsToEnd < secondsBlink && secondsToEnd > 1 && engine.getValue("[Channel2]", "play")) { // If song is about to end, blink cue button
        firstmix.setLED(firstmix.leds[2]["cue"], value);
    }
    firstmix.setLED(firstmix.leds[2]["play"], value); // Blink play button on beat
}

// ----------------------
// Actual functions below
// ----------------------

// EFX buttons (used for PFL)
firstmix.pfl = function (channel, control, value, status, group) {
    if (value == 0x7F) {  // Only take action on button press; not button release
        if (firstmix.efxButton[firstmix.currentDeck(group)-1] == false) { // if button is off, turn it on
            engine.setValue(group, "pfl", 1);
            firstmix.setLED(firstmix.leds[firstmix.currentDeck(group)]["efx"], 0x7f);
            firstmix.efxButton[firstmix.currentDeck(group)-1] = true;
        }
        else { // If button is on, turn it off
            engine.setValue(group, "pfl", 0);
            firstmix.setLED(firstmix.leds[firstmix.currentDeck(group)]["efx"], 0x00);
            firstmix.efxButton[firstmix.currentDeck(group)-1] = false;
        }  
    }
}

// Scratch buttons
firstmix.scratch = function (channel, control, value, status, group) {
    if (value == 0x7F) {  // Only take action on button press; not button release
        if (firstmix.scratchButton[firstmix.currentDeck(group)-1] == false) { // if button is off, turn it on
            firstmix.setLED(firstmix.leds[firstmix.currentDeck(group)]["scratch"], 0x7f);
            firstmix.scratchButton[firstmix.currentDeck(group)-1] = true;
        }
        else { // If button is on, turn it off
            firstmix.setLED(firstmix.leds[firstmix.currentDeck(group)]["scratch"], 0x00);
            firstmix.scratchButton[firstmix.currentDeck(group)-1] = false;
        }
    }
}

// Preview button
firstmix.preview = function (channel, control, value, status, group) {
    var tempVolume = engine.getValue("[Sampler4]","volume");

    if ((value == 0x7f) && (firstmix.previewButton == false)) {
        engine.setValue("[Sampler4]", "volume", 0);
        engine.setValue("[Sampler4]", "pfl", 1);
        engine.setValue("[Sampler4]", "LoadSelectedTrack", 1);
        engine.beginTimer(250,"engine.setValue(\"[Sampler4]\", \"play\", 1)",true); // I had to add a delay because this wouldn't work otherwise
        firstmix.setLED(firstmix.leds[0]["preview"], 0x7f);
        firstmix.previewButton = true;
    }
    else if ((value == 0x7f) && (firstmix.previewButton == true)) {
        engine.setValue("[Sampler4]", "play", 0);
        engine.setValue("[Sampler4]", "pfl", 0);
        engine.setValue("[Sampler4]", "volume", tempVolume);
        engine.setValue("[Sampler4]", "eject", 1);
        firstmix.setLED(firstmix.leds[0]["preview"], 0x00);
        firstmix.previewButton = false;
    }
}


// Jogwheel functions

firstmix.wheelTouch = function (channel, control, value, status, group) {
    if ((value == 0x7F) && (firstmix.scratchButton[firstmix.currentDeck(group)-1] == true)) {
        engine.scratchEnable(firstmix.currentDeck(group), 180, 33+1/3, alpha, beta);
    firstmix.touchingWheel[firstmix.currentDeck(group)-1] = true;
    }
    else if ((value == 0x7F) && (firstmix.scratchButton[firstmix.currentDeck(group)-1] == false)) 
    firstmix.touchingWheel[firstmix.currentDeck(group)-1] = true;

    else if (value == 0x00) {    // If button up
        engine.scratchDisable(firstmix.currentDeck(group));
    firstmix.touchingWheel[firstmix.currentDeck(group)-1] = false;
    }
}

firstmix.jogWheel = function(channel, control, value, status, group) {
    var deck = firstmix.currentDeck(group);
    var adjustedJog = parseFloat(value);
    var posNeg = 1;
    if (adjustedJog > 63) {    // Counter-clockwise
        posNeg = -1;
        adjustedJog = value - 128;
    }
    
        if ((firstmix.scratchButton[deck-1]) && (firstmix.touchingWheel[deck-1])) { // scratch mode
            var newValue;
            if (value-64 > 0) newValue = value-128;
            else newValue = value;
            engine.scratchTick(firstmix.currentDeck(group),newValue);
    } else if (firstmix.touchingWheel[deck-1] == true) { // pitch shift mode
        if (engine.getValue(group,"play")) {
            adjustedJog = posNeg * gammaOutputRange * Math.pow(Math.abs(adjustedJog) / (gammaInputRange * maxOutFraction), sensitivity);
        } else {
            adjustedJog = gammaOutputRange * adjustedJog / (gammaInputRange * maxOutFraction);
        }
        engine.setValue(group, "jog", adjustedJog);
    }
}


// ====================
// Deprecated functions
// ====================

// The wheel that actually controls the scratching
firstmix.wheelTurn = function (channel, control, value, status, group) {
    // Only continue if scratch button was pressed and jogwheel is being touched. If not, skip this.
    if (!(firstmix.scratchButton[firstmix.currentDeck(group)-1] && firstmix.touchingWheel[firstmix.currentDeck(group)-1])) return;
 
    var newValue;
    if (value-64 > 0) newValue = value-128;
    else newValue = value;
    engine.scratchTick(firstmix.currentDeck(group),newValue);
}

firstmix.playbutton1 = function (channel, control, value, status) {
    var currentlyPlaying = engine.getValue("[Channel1]","play");
    if ((currentlyPlaying == 1) & (value == 0x7F)) {
        engine.setValue("[Channel1]","play",0);    // Stop
        midi.sendShortMsg(0x90,0x4A,0x00);    // Turn off the Play LED
    }
    if ((currentlyPlaying == 0) & (value == 0x7F)) {
        engine.setValue("[Channel1]","play",1);    // Start
        midi.sendShortMsg(0x90,0x4A,0x7F);    // Turn on the Play LED
    }
}

firstmix.playbutton2 = function (channel, control, value, status, group) {
    var currentlyPlaying = engine.getValue("[Channel2]","play");
    if ((currentlyPlaying == 1) & (value == 0x7F)) {
        engine.setValue("[Channel2]","play",0);    // Stop
        midi.sendShortMsg(0x90,0x4C,0x00);    // Turn off the Play LED
    }
    if ((currentlyPlaying == 0) & (value == 0x7F)) {
        engine.setValue("[Channel2]","play",1);    // Start
        midi.sendShortMsg(0x90,0x4C,0x7F);    // Turn on the Play LED
    }
}

