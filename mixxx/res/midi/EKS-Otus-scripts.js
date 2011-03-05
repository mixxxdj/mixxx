/****************************************************************/
/*      EKS Otus MIDI controller script v0.pre                  */
/*          Copyright (C) 2010, Sean M. Pappalardo              */
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 1.9.x                                 */
/****************************************************************/

function EksOtus() {}

// ----------   Customization variables ----------
//      See http://mixxx.org/wiki/doku.php/eks_otus  for details
EksOtus.spinningPlatter = true;    // Spinning platter LEDs
EksOtus.spinningLights = 1;        // The number of spinning platter lights, 1 or 2

// ----------   Other global variables    ----------
EksOtus.debug = false;  // Enable/disable debugging messages to the console

EksOtus.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time
EksOtus.channel = 0;   // MIDI channel to set the device to and use
EksOtus.deck = 1;  // Currently active virtual deck
EksOtus.scratchncue = [ ];  // Scratch + cue mode for each deck (starts at zero)
EksOtus.modifier = { "cue":0, "play":0 };  // Modifier buttons (allowing alternate controls) defined on-the-fly if needed
EksOtus.state = { }; // Temporary state variables
StantonSCS3d.timer = [-1];  // Temporary storage of timer IDs
//EksOtus.sysex = [0xF0, 0x00, 0x01, 0x60];  // Preamble for all SysEx messages for this device
EksOtus.trackDuration = [ ]; // Duration of the song on each deck (used for vinyl LEDs)
EksOtus.lastLight = [ ]; // Last circle LED values

// ----------   Functions   ----------

EksOtus.init = function (id) {
    EksOtus.id = id;   // Store the ID of this device for later use
    
    var CC = 0xB0 + EksOtus.channel;
    var No = 0x90 + EksOtus.channel;
    //midi.sendShortMsg(?);  // Extinguish all LEDs
    
    // Initialize per-deck controls
    //var decks = engine.getValue("[Master]","num_decks"));
    //for (i=1; i<=decks; i++) {
    //    EksOtus.scratchncue[i] = false;
    //}
    
    //  Initialize the spinning platter LEDs if the mapping is loaded after a song is
    EksOtus.durationChange(engine.getValue("[Channel"+EksOtus.deck+"]","duration"));
    
    print ("EksOtus: \""+EksOtus.id+"\" on MIDI channel "+(EksOtus.channel+1)+" initialized.");
}

EksOtus.shutdown = function () {   // called when the MIDI device is closed

    EksOtus.stopTimers();

    var CC = 0xB0 + EksOtus.channel;
    var No = 0x90 + EksOtus.channel;

    //midi.sendShortMsg(?);  // Extinguish all LEDs
    
    print ("EksOtus: \""+EksOtus.id+"\" on MIDI channel "+(EksOtus.channel+1)+" shut down.");
}

EksOtus.stopTimers = function () {
    for (var i=0; i<EksOtus.timer.length; i++) {
        if (EksOtus.timer[i] != -1) {
            engine.stopTimer(EksOtus.timer[i]);
            EksOtus.timer[i] = -1;
        }
    }
}

// ---- Other slots ----

EksOtus.durationChange = function (value) {
    EksOtus.trackDuration[EksOtus.deck]=value;
}

EksOtus.circleFlash = function (deck) {
    if (EksOtus.deck != deck) return;  // Only do this for the current deck
    if (!EksOtus.state["circleInvert"]) EksOtus.state["circleInvert"]=true;
    else EksOtus.state["circleInvert"]=false;
    // Force the circle LEDs to light
    EksOtus.lastLight[deck]=-1;
    EksOtus.circleLEDs(engine.getValue("[Channel"+deck+"]","visual_playposition"));
}

EksOtus.circleLEDs = function (value) {
    
    var deck = EksOtus.deck;

    var currentMode = EksOtus.mode_store["[Channel"+deck+"]"];
    if (EksOtus.spinningPlatterOnlyVinyl) {    // Skip if not in vinyl mode
        if (currentMode != "vinyl" && currentMode != "vinyl2") return;
    } else {
        // Skip if in LOOP2-3, TRIG, or VINYL3 modes since they use the circle LEDs
        //  for other things
        if (currentMode == "vinyl3" || (currentMode != "loop" && currentMode.substring(0,4) == "loop") ||
        currentMode.substring(0,4) == "trig") return;
    }
    
    // Flash the circle near the end of the track if the track is longer than 30s
    if (EksOtus.trackDuration[deck]>30) {
        var trackTimeRemaining = ((1-value) * EksOtus.trackDuration[deck]) | 0;
        if (trackTimeRemaining<=30 && trackTimeRemaining>15) {   // If <30s left, flash slowly
            if (EksOtus.timer["30s-d"+deck] == -1) {
                // Start timer
                EksOtus.timer["30s-d"+deck] = engine.beginTimer(500,"EksOtus.circleFlash("+deck+")");
                if (EksOtus.timer["15s-d"+deck] != -1) {
                    // Stop the 15s timer if it was running
                    engine.stopTimer(EksOtus.timer["15s-d"+deck]);
                    EksOtus.timer["15s-d"+deck] = -1;
                }
            }
        } else if (trackTimeRemaining<=15 && trackTimeRemaining>0) { // If <15s left, flash quickly
            if (EksOtus.timer["15s-d"+deck] == -1) {
                // Start timer
                EksOtus.timer["15s-d"+deck] = engine.beginTimer(125,"EksOtus.circleFlash("+deck+")");
                if (EksOtus.timer["30s-d"+deck] != -1) {
                    // Stop the 30s timer if it was running
                    engine.stopTimer(EksOtus.timer["30s-d"+deck]);
                    EksOtus.timer["30s-d"+deck] = -1;
                }
            }
        } else {    // Stop flashing
            if (EksOtus.timer["15s-d"+deck] != -1) {
                engine.stopTimer(EksOtus.timer["15s-d"+deck]);
                EksOtus.timer["15s-d"+deck] = -1;
            }
            if (EksOtus.timer["30s-d"+deck] != -1) {
                engine.stopTimer(EksOtus.timer["30s-d"+deck]);
                EksOtus.timer["30s-d"+deck] = -1;
            }
            EksOtus.state["circleInvert"]=false;
        }
    } else EksOtus.state["circleInvert"]=false;
    
    // Revolution time of the imaginary record in seconds
    var revtime = EksOtus.revtime;
    
    if (EksOtus.spinningLights==2) revtime = revtime/2;    // Use this for two lights
    var currentTrackPos = value * EksOtus.trackDuration[deck];
    
    var revolutions = currentTrackPos/revtime;
    
    var light = ((revolutions-(revolutions|0))*16)|0;   // OR with 0 replaces Math.floor and is faster
    if (EksOtus.spinningLights==2) light = ((revolutions-(revolutions|0))*8)|0;    // Use this for two lights

    // Don't send light commands if there's no visible change
    if (EksOtus.lastLight[deck]==light) return;
    
    // Clear circle lights
    var byte1 = 0xB0 + EksOtus.channel;
    var byte2 = 0x62;   // normal
    if (EksOtus.state["circleInvert"]) byte2 = 0x72;   // inverted
    midi.sendShortMsg(byte1,byte2,0x00);
    
    var byte1 = 0x90 + EksOtus.channel;
    var byte3 = 0x01;
    EksOtus.lastLight[deck]=light;
    if (EksOtus.state["circleInvert"]) byte3 = 0x00;
    midi.sendShortMsg(byte1,0x5d+light,byte3);
    if (EksOtus.spinningLights==2) midi.sendShortMsg(byte1,0x65+light,byte3);   // Add this for two lights
}