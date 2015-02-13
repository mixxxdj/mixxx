/*
 *  MIDI_for_LightEffects
 *
 *  send midi information about the musing to generate light effects with a light control app like QLCplus
 *
 *  This file should be placed into /usr/share/mixxx/controllers
 *
 *  (c) 2014 - DG3NEC / Michael Stahl 
 *  (c) 2015 - Markus Baertschi, markus@markus.org
 *
 * Changelog
 * 25.01.2015  Initial version
 * 25.01.2015  Improved printDebug with debuglevel
 *             Improved Play callback to determine beatdeck only from 'play' buttons
 * 13.02.2015  Improved printDebug with script name
 */

function MIDI_for_LightEffects () {}

var vu_array_fill_counter = 1;
var vu_array_fill_maximum = 50; // 25 = 1sec; 50 = 2sec; Size/duration of the buffer for statistical values
var vu_array_mono = new Array(vu_array_fill_maximum);
var vu_array_left = new Array(vu_array_fill_maximum);
var vu_array_right = new Array(vu_array_fill_maximum);

// rudimentary debug print with timestamp
maxdebuglevel=0; // 0-always, 1-minimal, 2-more details in some routines, 5-includes beat
printDebug = function ( level, tag, text ) {
    var date = new Date();
    var timestamp = ('0'+date.getHours()).slice(-2)+":"+('0'+date.getMinutes()).slice(-2)+":"+('0'+date.getSeconds()).slice(-2);
    if (level<=maxdebuglevel) { 
        print("MIDI_for_LightEffects "+timestamp+" "+tag+"("+level+"): "+text);
//        script.midiDebug(0,0,0,timestamp+" "+tag+"("+level+"): "+text,0);
    }
}

// Extract the numerical deck number from the channel string
deckNo = function (deck) {
    return deck.match(/\d+/)[0];
}

// Stop a timer if it is defined
stopTimerIf = function ( timer ) {
    printDebug(2,"stopTimerIf","timer="+timer);
    if (timer==undefined) { return; }
    engine.stopTimer(timer);
}

otherDeck = function(deck) {
    if (deck == "[Channel1]") return "[Channel2]";
    if (deck == "[Channel2]") return "[Channel1]";
    printDebug(0,"otherDeck","Error: other deck for "+deck+" is not defined");
}

MIDI_for_LightEffects.init = function(id,debug) {  // called when the MIDI device is opened & set up
    MIDI_for_LightEffects.id = id;	           // Store the ID of this device
    MIDI_for_LightEffects.beatdeck = "";           // The deck we take the beat from, "[Channel1]" or "[Channel2]"
    MIDI_for_LightEffects.faderlock = false;       // Stops fader changes from changing the beatdeck again within 1 sec
    MIDI_for_LightEffects.volumebeat = false;      // Allow volume changes to changes the beatdeck
    MIDI_for_LightEffects.volumebeatlock = false;  // Stops volume changes from changing the beatdeck again within 1 sec
    MIDI_for_LightEffects.CrossfaderChangeLockTimer = [-1, -1];
    MIDI_for_LightEffects.VolumeBeatLockTimer = [-1, -1];
    MIDI_for_LightEffects.VuMeterTimer = [-1, -1];

    /*
     * Register callbacks for Mixxx functions
     * on each change of the observed value, the callback is called
     * the callback s called with three parameters: <callback>(value, group, control)
     * Example: VolumeChange(<newvolume>,"[Channel1]","volume")
    */

    // beat_active, used to send beat
    engine.connectControl("[Channel1]", "beat_active",  "MIDI_for_LightEffects.BeatOutputToMidi");
    engine.connectControl("[Channel2]", "beat_active",  "MIDI_for_LightEffects.BeatOutputToMidi");
    // playposition, used to send MTC frames
    engine.connectControl("[Channel1]", "playposition", "MIDI_for_LightEffects.SendMidiMtcFullframe");
    engine.connectControl("[Channel2]", "playposition", "MIDI_for_LightEffects.SendMidiMtcFullframe");
    // fader/volume/play, used to determine which is the active deck we should use to send the beat
    engine.connectControl("[Master]",   "crossfader",   "MIDI_for_LightEffects.CrossfaderChange");
    engine.connectControl("[Channel1]", "volume",       "MIDI_for_LightEffects.VolumeChange");
    engine.connectControl("[Channel2]", "volume",       "MIDI_for_LightEffects.VolumeChange");
    engine.connectControl("[Channel1]", "play",         "MIDI_for_LightEffects.Play");
    engine.connectControl("[Channel2]", "play",         "MIDI_for_LightEffects.Play");

    MIDI_for_LightEffects.VuMeterTimer = engine.beginTimer(40,"MIDI_for_LightEffects.VuMeter()",false);

    if (debug) {
        maxdebuglevel=4;	// Enable debugging up to level 4
        printDebug(0,"init","MIDI_for_LightEffects initialized with debugging");
    } else {
        printDebug(0,"init","MIDI_for_LightEffects initialized");
    }
}

MIDI_for_LightEffects.shutdown = function(id) {	// called when the MIDI device is closed
    printDebug(0,"shutdown","MIDI_for_LightEffects shutdown");
    // Stop the callbacks properly
    engine.connectControl("[Master]",   "crossfader", "MIDI_for_LightEffects.CrossfaderChange", true);
    engine.connectControl("[Channel1]", "volume", "MIDI_for_LightEffects.VolumeChange", true);
    engine.connectControl("[Channel2]", "volume", "MIDI_for_LightEffects.VolumeChange", true);
    engine.connectControl("[Channel1]", "beat_active", "MIDI_for_LightEffects.BeatOutputToMidi", true);
    engine.connectControl("[Channel2]", "beat_active", "MIDI_for_LightEffects.BeatOutputToMidi", true);
    engine.connectControl("[Channel1]", "play", "MIDI_for_LightEffects.Play", true);
    engine.connectControl("[Channel2]", "play", "MIDI_for_LightEffects.Play", true);
    engine.connectControl("[Channel1]", "playposition", "MIDI_for_LightEffects.SendMidiMtcFullframe", true);
    engine.connectControl("[Channel2]", "playposition", "MIDI_for_LightEffects.SendMidiMtcFullframe", true);
    // Stop all timers
    engine.stopTimer(MIDI_for_LightEffects.VuMeterTimer);
    engine.stopTimer(MIDI_for_LightEffects.CrossfaderChangeLockTimer);
    engine.stopTimer(MIDI_for_LightEffects.VolumeBeatLockTimer);
}


/*
 *  VU-Meter
 *
 * The VU-meter is fired by a recurring timer every 40ms (25 times per second)
 * It gathers the current VU values, runs some statistics and sends the info as MIDI notes
 *
 * To add some interesting values for lights it calculates the average volume over 2 seconds
 * and calculates the delta between the current and the average volume and sends this too
 *
 */
MIDI_for_LightEffects.VuMeter= function() { // called every 40 ms by a timer
    // Define ouput values for MIDI (Range)
    var vu_out_min = 0;
    var vu_out_max = 127;
    var vu_out_range = vu_out_max - vu_out_min;
    // Fetch current valued from Mixxx
    var vu_mono_current = engine.getValue("[Master]", "VuMeter");
    var vu_left_current = engine.getValue("[Master]", "VuMeterL");
    var vu_right_current = engine.getValue("[Master]", "VuMeterR");

    // Manage the current position in the array
    vu_array_fill_counter++;
    if (vu_array_fill_counter > vu_array_fill_maximum) vu_array_fill_counter = 1;

    // Save current values to array
    vu_array_mono[vu_array_fill_counter] = vu_mono_current;
    vu_array_left[vu_array_fill_counter] = vu_left_current;
    vu_array_right[vu_array_fill_counter] = vu_right_current;

    // Determine min, mid and max values
    var vu_mono_average_min = vu_array_mono[1];
    var vu_mono_average_mid = 0; 
    var vu_mono_average_max = vu_array_mono[1];
    var vu_left_average_min = vu_array_left[1];
    var vu_left_average_mid = 0; 
    var vu_left_average_max = vu_array_left[1];
    var vu_right_average_min = vu_array_right[1];
    var vu_right_average_mid = 0; 
    var vu_right_average_max = vu_array_right[1];
    var z = 1;
    // Scan the arrays to determine min, mid and max values
    while ( z < vu_array_fill_maximum) {
        // mono
        if ( vu_array_mono[z] < vu_mono_average_min) vu_mono_average_min = vu_array_mono[z];
        vu_mono_average_mid = vu_mono_average_mid + vu_array_mono[z];
        if ( vu_array_mono[z] > vu_mono_average_max) vu_mono_average_max = vu_array_mono[z];
        // left
        if ( vu_array_left[z] < vu_left_average_min) vu_left_average_min = vu_array_left[z];
        vu_left_average_mid = vu_left_average_mid + vu_array_left[z];
        if ( vu_array_left[z] > vu_left_average_max) vu_left_average_max = vu_array_left[z];
        // right
        if ( vu_array_right[z] < vu_right_average_min) vu_right_average_min = vu_array_right[z];
        vu_right_average_mid = vu_right_average_mid + vu_array_right[z];
        if ( vu_array_right[z] > vu_right_average_max) vu_right_average_max = vu_array_right[z];
        z++;
    }
    vu_mono_average_mid = vu_mono_average_mid / vu_array_fill_maximum;
    vu_left_average_mid = vu_left_average_mid / vu_array_fill_maximum;
    vu_right_average_mid = vu_right_average_mid / vu_array_fill_maximum;
  
    // Calculate average_fit
    vu_mono_average_fit = (vu_mono_current - vu_mono_average_min) / (vu_mono_average_max - vu_mono_average_min);
    vu_left_average_fit = (vu_left_current - vu_left_average_min) / (vu_left_average_max - vu_left_average_min);
    vu_right_average_fit = (vu_right_current - vu_right_average_min) / (vu_right_average_max - vu_right_average_min);

    // Calculate VU-Meter
    var vu_mono_current_meter1 = (vu_mono_current * 4) - 0;
    if ( vu_mono_current_meter1 < 0 ) vu_mono_current_meter1 = 0;
    if ( vu_mono_current_meter1 > 1 ) vu_mono_current_meter1 = 1;
    var vu_mono_current_meter2 = (vu_mono_current * 4) - 1;
    if ( vu_mono_current_meter2 < 0 ) vu_mono_current_meter2 = 0;
    if ( vu_mono_current_meter2 > 1 ) vu_mono_current_meter2 = 1;
    var vu_mono_current_meter3 = (vu_mono_current * 4) - 2;
    if ( vu_mono_current_meter3 < 0 ) vu_mono_current_meter3 = 0;
    if ( vu_mono_current_meter3 > 1 ) vu_mono_current_meter3 = 1;
    var vu_mono_current_meter4 = (vu_mono_current * 4) - 3;
    if ( vu_mono_current_meter4 < 0 ) vu_mono_current_meter4 = 0;
    if ( vu_mono_current_meter4 > 1 ) vu_mono_current_meter4 = 1;

    var vu_mono_average_meter1 = (vu_mono_average_fit * 4) - 0;
    if ( vu_mono_average_meter1 < 0 ) vu_mono_average_meter1 = 0;
    if ( vu_mono_average_meter1 > 1 ) vu_mono_average_meter1 = 1;
    var vu_mono_average_meter2 = (vu_mono_average_fit * 4) - 1;
    if ( vu_mono_average_meter2 < 0 ) vu_mono_average_meter2 = 0;
    if ( vu_mono_average_meter2 > 1 ) vu_mono_average_meter2 = 1;
    var vu_mono_average_meter3 = (vu_mono_average_fit * 4) - 2;
    if ( vu_mono_average_meter3 < 0 ) vu_mono_average_meter3 = 0;
    if ( vu_mono_average_meter3 > 1 ) vu_mono_average_meter3 = 1;
    var vu_mono_average_meter4 = (vu_mono_average_fit * 4) - 3;
    if ( vu_mono_average_meter4 < 0 ) vu_mono_average_meter4 = 0;
    if ( vu_mono_average_meter4 > 1 ) vu_mono_average_meter4 = 1;

    var vu_left_current_meter1 = (vu_left_current * 4) - 0;
    if ( vu_left_current_meter1 < 0 ) vu_left_current_meter1 = 0;
    if ( vu_left_current_meter1 > 1 ) vu_left_current_meter1 = 1;
    var vu_left_current_meter2 = (vu_left_current * 4) - 1;
    if ( vu_left_current_meter2 < 0 ) vu_left_current_meter2 = 0;
    if ( vu_left_current_meter2 > 1 ) vu_left_current_meter2 = 1;
    var vu_left_current_meter3 = (vu_left_current * 4) - 2;
    if ( vu_left_current_meter3 < 0 ) vu_left_current_meter3 = 0;
    if ( vu_left_current_meter3 > 1 ) vu_left_current_meter3 = 1;
    var vu_left_current_meter4 = (vu_left_current * 4) - 3;
    if ( vu_left_current_meter4 < 0 ) vu_left_current_meter4 = 0;
    if ( vu_left_current_meter4 > 1 ) vu_left_current_meter4 = 1;

    var vu_left_average_meter1 = (vu_left_average_fit * 4) - 0;
    if ( vu_left_average_meter1 < 0 ) vu_left_average_meter1 = 0;
    if ( vu_left_average_meter1 > 1 ) vu_left_average_meter1 = 1;
    var vu_left_average_meter2 = (vu_left_average_fit * 4) - 1;
    if ( vu_left_average_meter2 < 0 ) vu_left_average_meter2 = 0;
    if ( vu_left_average_meter2 > 1 ) vu_left_average_meter2 = 1;
    var vu_left_average_meter3 = (vu_left_average_fit * 4) - 2;
    if ( vu_left_average_meter3 < 0 ) vu_left_average_meter3 = 0;
    if ( vu_left_average_meter3 > 1 ) vu_left_average_meter3 = 1;
    var vu_left_average_meter4 = (vu_left_average_fit * 4) - 3;
    if ( vu_left_average_meter4 < 0 ) vu_left_average_meter4 = 0;
    if ( vu_left_average_meter4 > 1 ) vu_left_average_meter4 = 1;

    var vu_right_current_meter1 = (vu_right_current * 4) - 0;
    if ( vu_right_current_meter1 < 0 ) vu_right_current_meter1 = 0;
    if ( vu_right_current_meter1 > 1 ) vu_right_current_meter1 = 1;
    var vu_right_current_meter2 = (vu_right_current * 4) - 1;
    if ( vu_right_current_meter2 < 0 ) vu_right_current_meter2 = 0;
    if ( vu_right_current_meter2 > 1 ) vu_right_current_meter2 = 1;
    var vu_right_current_meter3 = (vu_right_current * 4) - 2;
    if ( vu_right_current_meter3 < 0 ) vu_right_current_meter3 = 0;
    if ( vu_right_current_meter3 > 1 ) vu_right_current_meter3 = 1;
    var vu_right_current_meter4 = (vu_right_current * 4) - 3;
    if ( vu_right_current_meter4 < 0 ) vu_right_current_meter4 = 0;
    if ( vu_right_current_meter4 > 1 ) vu_right_current_meter4 = 1;

    var vu_right_average_meter1 = (vu_right_average_fit * 4) - 0;
    if ( vu_right_average_meter1 < 0 ) vu_right_average_meter1 = 0;
    if ( vu_right_average_meter1 > 1 ) vu_right_average_meter1 = 1;
    var vu_right_average_meter2 = (vu_right_average_fit * 4) - 1;
    if ( vu_right_average_meter2 < 0 ) vu_right_average_meter2 = 0;
    if ( vu_right_average_meter2 > 1 ) vu_right_average_meter2 = 1;
    var vu_right_average_meter3 = (vu_right_average_fit * 4) - 2;
    if ( vu_right_average_meter3 < 0 ) vu_right_average_meter3 = 0;
    if ( vu_right_average_meter3 > 1 ) vu_right_average_meter3 = 1;
    var vu_right_average_meter4 = (vu_right_average_fit * 4) - 3;
    if ( vu_right_average_meter4 < 0 ) vu_right_average_meter4 = 0;
    if ( vu_right_average_meter4 > 1 ) vu_right_average_meter4 = 1;

    // Scale the values for the MIDI value range (1-128)
    vu_mono_current = (vu_mono_current * vu_out_range) + vu_out_min;
    vu_mono_average_min = (vu_mono_average_min * vu_out_range) + vu_out_min;
    vu_mono_average_mid = (vu_mono_average_mid * vu_out_range) + vu_out_min;
    vu_mono_average_max = (vu_mono_average_max * vu_out_range) + vu_out_min;
    vu_mono_average_fit = (vu_mono_average_fit * vu_out_range) + vu_out_min;
    vu_mono_current_meter1 = (vu_mono_current_meter1 * vu_out_range) + vu_out_min;
    vu_mono_current_meter2 = (vu_mono_current_meter2 * vu_out_range) + vu_out_min;
    vu_mono_current_meter3 = (vu_mono_current_meter3 * vu_out_range) + vu_out_min;
    vu_mono_current_meter4 = (vu_mono_current_meter4 * vu_out_range) + vu_out_min;
    vu_mono_average_meter1 = (vu_mono_average_meter1 * vu_out_range) + vu_out_min;
    vu_mono_average_meter2 = (vu_mono_average_meter2 * vu_out_range) + vu_out_min;
    vu_mono_average_meter3 = (vu_mono_average_meter3 * vu_out_range) + vu_out_min;
    vu_mono_average_meter4 = (vu_mono_average_meter4 * vu_out_range) + vu_out_min;

    vu_left_current = (vu_left_current * vu_out_range) + vu_out_min;
    vu_left_average_min = (vu_left_average_min * vu_out_range) + vu_out_min;
    vu_left_average_mid = (vu_left_average_mid * vu_out_range) + vu_out_min;
    vu_left_average_max = (vu_left_average_max * vu_out_range) + vu_out_min;
    vu_left_average_fit = (vu_left_average_fit * vu_out_range) + vu_out_min;
    vu_left_current_meter1 = (vu_left_current_meter1 * vu_out_range) + vu_out_min;
    vu_left_current_meter2 = (vu_left_current_meter2 * vu_out_range) + vu_out_min;
    vu_left_current_meter3 = (vu_left_current_meter3 * vu_out_range) + vu_out_min;
    vu_left_current_meter4 = (vu_left_current_meter4 * vu_out_range) + vu_out_min;
    vu_left_average_meter1 = (vu_left_average_meter1 * vu_out_range) + vu_out_min;
    vu_left_average_meter2 = (vu_left_average_meter2 * vu_out_range) + vu_out_min;
    vu_left_average_meter3 = (vu_left_average_meter3 * vu_out_range) + vu_out_min;
    vu_left_average_meter4 = (vu_left_average_meter4 * vu_out_range) + vu_out_min;

    vu_right_current = (vu_right_current * vu_out_range) + vu_out_min;
    vu_right_average_min = (vu_right_average_min * vu_out_range) + vu_out_min;
    vu_right_average_mid = (vu_right_average_mid * vu_out_range) + vu_out_min;
    vu_right_average_max = (vu_right_average_max * vu_out_range) + vu_out_min;
    vu_right_average_fit = (vu_right_average_fit * vu_out_range) + vu_out_min;
    vu_right_current_meter1 = (vu_right_current_meter1 * vu_out_range) + vu_out_min;
    vu_right_current_meter2 = (vu_right_current_meter2 * vu_out_range) + vu_out_min;
    vu_right_current_meter3 = (vu_right_current_meter3 * vu_out_range) + vu_out_min;
    vu_right_current_meter4 = (vu_right_current_meter4 * vu_out_range) + vu_out_min;
    vu_right_average_meter1 = (vu_right_average_meter1 * vu_out_range) + vu_out_min;
    vu_right_average_meter2 = (vu_right_average_meter2 * vu_out_range) + vu_out_min;
    vu_right_average_meter3 = (vu_right_average_meter3 * vu_out_range) + vu_out_min;
    vu_right_average_meter4 = (vu_right_average_meter4 * vu_out_range) + vu_out_min;

    // Send the values as MIDI notes
    midi.sendShortMsg(0x90,0x40,vu_mono_current);         // dez.64, okt.5 note E
    midi.sendShortMsg(0x90,0x41,vu_mono_average_min);     // dez.65, okt.5 note F
    midi.sendShortMsg(0x90,0x42,vu_mono_average_mid);     // dez.66, okt.5 note F#
    midi.sendShortMsg(0x90,0x43,vu_mono_average_max);     // dez.67, okt.5 note G
    midi.sendShortMsg(0x90,0x44,vu_mono_average_fit);     // dez.68, okt.5 note G#
    midi.sendShortMsg(0x90,0x45,vu_mono_current_meter1);  // dez.69, okt.5 note A
    midi.sendShortMsg(0x90,0x46,vu_mono_current_meter2);  // dez.70, okt.5 note A#
    midi.sendShortMsg(0x90,0x47,vu_mono_current_meter3);  // dez.71, okt.5 note B
    midi.sendShortMsg(0x90,0x48,vu_mono_current_meter4);  // dez.72, okt.6 note C
    midi.sendShortMsg(0x90,0x49,vu_mono_average_meter1);  // dez.73, okt.6 note C#
    midi.sendShortMsg(0x90,0x4A,vu_mono_average_meter2);  // dez.74, okt.6 note D
    midi.sendShortMsg(0x90,0x4B,vu_mono_average_meter3);  // dez.75, okt.6 note D#
    midi.sendShortMsg(0x90,0x4C,vu_mono_average_meter4);  // dez.76, okt.6 note E
    midi.sendShortMsg(0x90,0x50,vu_left_current);         // dez.80, okt.6 note G#
    midi.sendShortMsg(0x90,0x51,vu_left_average_min);     // dez.81, okt.6 note A
    midi.sendShortMsg(0x90,0x52,vu_left_average_mid);     // dez.82, okt.6 note A#
    midi.sendShortMsg(0x90,0x53,vu_left_average_max);     // dez.83, okt.6 note B
    midi.sendShortMsg(0x90,0x54,vu_left_average_fit);     // dez.84, okt.7 note C
    midi.sendShortMsg(0x90,0x55,vu_left_current_meter1);  // dez.85, okt.7 note C#
    midi.sendShortMsg(0x90,0x56,vu_left_current_meter2);  // dez.86, okt.7 note D
    midi.sendShortMsg(0x90,0x57,vu_left_current_meter3);  // dez.87, okt.7 note D#
    midi.sendShortMsg(0x90,0x58,vu_left_current_meter4);  // dez.88, okt.7 note E
    midi.sendShortMsg(0x90,0x59,vu_left_average_meter1);  // dez.89, okt.7 note F
    midi.sendShortMsg(0x90,0x5A,vu_left_average_meter2);  // dez.90, okt.7 note F#
    midi.sendShortMsg(0x90,0x5B,vu_left_average_meter3);  // dez.91, okt.7 note G
    midi.sendShortMsg(0x90,0x5C,vu_left_average_meter4);  // dez.92, okt.7 note G#
    midi.sendShortMsg(0x90,0x60,vu_right_current);        // dez.96, okt.8 note C
    midi.sendShortMsg(0x90,0x61,vu_right_average_min);    // dez.97, okt.8 note C#
    midi.sendShortMsg(0x90,0x62,vu_right_average_mid);    // dez.98, okt.8 note D
    midi.sendShortMsg(0x90,0x63,vu_right_average_max);    // dez.99, okt.8 note D#
    midi.sendShortMsg(0x90,0x64,vu_right_average_fit);    // dez.100, okt.8 note E
    midi.sendShortMsg(0x90,0x65,vu_right_current_meter1); // dez.101, okt.8 note F
    midi.sendShortMsg(0x90,0x66,vu_right_current_meter2); // dez.102, okt.8 note F#
    midi.sendShortMsg(0x90,0x67,vu_right_current_meter3); // dez.103, okt.8 note G
    midi.sendShortMsg(0x90,0x68,vu_right_current_meter4); // dez.104, okt.8 note G#
    midi.sendShortMsg(0x90,0x69,vu_right_average_meter1); // dez.105, okt.8 note A
    midi.sendShortMsg(0x90,0x6A,vu_right_average_meter2); // dez.106, okt.8 note A#
    midi.sendShortMsg(0x90,0x6B,vu_right_average_meter3); // dez.107, okt.8 note B
    midi.sendShortMsg(0x90,0x6C,vu_right_average_meter4); // dez.108, okt.9 note C
}

/*
 * Beat and beat source determination
 *
 * Mixxx provides a beat for each deck, we send only one beat, so we must
 * determine which deck we should take the beat from.
 *
 * We use several sources:
 * - Fader position: Take the beat from where the crossfader points to
 * - Volume position: Take the beat from the deck with more volume if the fader is in the center area
 * - Playing deck: Take the beat from the deck where something is playing
 *
 * There are also some timing conditions
 * - After a fader change we wait for 1s before taking a change in account again
 *   This prevents beatdeck changes when fiddling with the fader
 * - After a fader change we wait 3s before taking volume changes in account
 * - After a volume change we wait for 1s before taking a change in account again
 *   This prevents beatdeck changes when fiddling with the volume
 */

/*
 * beatdeck selection from Play button: 
 * - Set the beatdeck to the 1st deck we press play on
 * - Set beatdeck to other deck if we stop the current deck
 * - Set the beatdeck to the current deck if we press play and the other deck is stopped
 */
MIDI_for_LightEffects.Play = function(value, deck, control) {
    printDebug(1,"Play","deck="+deck+" val="+value+" contr="+control);
    // After initial startup beatdeck is empty, set it
    if (MIDI_for_LightEffects.beatdeck == "") MIDI_for_LightEffects.beatdeck=deck;
    var otherdeck=otherDeck(deck);
    if ((MIDI_for_LightEffects.beatdeck == deck) && (value==0)) {
        // The active deck was stopped, switch to the other deck immediately
        printDebug(2,"Play"," Deck "+deck+" stopped, switching to "+otherdeck);
        MIDI_for_LightEffects.beatdeck = otherdeck;
    }
    var othervalue = engine.getValue(otherdeck, "play");
    if ((MIDI_for_LightEffects.beatdeck == otherdeck) && (othervalue==0) && (value==1)) {
        printDebug(2,"Play"," Other deck "+otherdeck+" stopped and we pressed play on "+deck+" -> switch deck to "+deck);
        MIDI_for_LightEffects.beatdeck = deck;
    }
}

/*
 * Called on each volume change (both decks)
 */
MIDI_for_LightEffects.VolumeChange = function(volume,deck,control) { // Deck volume was changed
    printDebug(1,"VolumeChange",deck+" volume="+volume);
    if (MIDI_for_LightEffects.volumebeat == false) { return; }      // exit if Volumebeat not active
    if (MIDI_for_LightEffects.volumebeatlock == true) { return; } // exit if Lock active
    // Fetch deck volumes
    deck1volume = engine.getValue("[Channel1]", "volume");
    deck2volume = engine.getValue("[Channel2]", "volume");
    printDebug(2,"VolumeChange","deck 1 vol="+deck1volume+" deck 2 vol="+deck2volume);
    if (deck1volume == deck2volume) { return; } // exit if Volume is the same
    // Determine beatdeck by comparing their volume
    var newdeck;
    if (deck2volume > deck1volume) { newdeck = "[Channel2]"; } else { newdeck = "[Channel1]"; } // Deck2 Volume is larger
    // Check if the active deck changes
    if (newdeck != MIDI_for_LightEffects.beatdeck) {
        printDebug(2,"VolumeChange","deckchange old="+MIDI_for_LightEffects.beatdeck+" new="+newdeck+" vol1="+deck1volume+" vol2="+deck2volume);
        MIDI_for_LightEffects.beatdeck = newdeck;
        midi.sendShortMsg(0x90,48,0x64+deckNo(newdeck));         // Deckchange: Send note C mit 64+deckno
        MIDI_for_LightEffects.volumebeatlock = true;     // Prevent change for 1 s
        MIDI_for_LightEffects.VolumeBeatLockTimer = engine.beginTimer(1000,"MIDI_for_LightEffects.VolumeBeatLock()",true);
    }
}

MIDI_for_LightEffects.VolumeBeatLock = function() { // Timer callback re-enable deck change
    printDebug(1,"VolumeBeatLock","");
//    engine.stopTimer(MIDI_for_LightEffects.VolumeBeatLockTimer);
    MIDI_for_LightEffects.volumebeatlock = false;
    MIDI_for_LightEffects.VolumeBeatLockTimer = undefined;
    midi.sendShortMsg(0x90,48,0x0);   // BeatLock: Send Note C on with 0
    midi.sendShortMsg(0x80,48,0x0);   // BeatLock: Send Note C off with 0
    MIDI_for_LightEffects.VolumeChange();  // Check id the deck is still the same
}

MIDI_for_LightEffects.VolumeBeatOnDelay = function() { // Turn on volume dependent desk change
    printDebug(1,"VolumeBeatOnDelay","volumebeat=true");
    MIDI_for_LightEffects.volumebeat = true;
    MIDI_for_LightEffects.VolumeBeatOnDelayTimer=undefined;
}

MIDI_for_LightEffects.CrossfaderChange = function(fader,group,control) { // Fader was moved, check deck
    printDebug(1,"CrossfaderChange","fader="+fader);
    if (MIDI_for_LightEffects.faderlock == true) { return; }  // exit if faderlock active

    //
    // We allow volume changes to override the fader deck selection
    // if the fader is in the central area (+/- 25%)
    // and at least 3s have passed since the last fader change
    //
    stopTimerIf(MIDI_for_LightEffects.VolumeBeatOnDelayTimer);
//    engine.stopTimer(MIDI_for_LightEffects.VolumeBeatOnDelayTimer);  // Stop timer
    MIDI_for_LightEffects.volumebeat = false;     // Disallow volume to select beatdeck
    if (fader > -0.25) {      // Fader is less than 25% to the left
        if (fader < 0.25) {   // Fader is less than 25% to the right
            // Set timer to re-enable volumebeat after 3 seconds
            printDebug(2,"CrossfaderChange","Fader in central area, volume active in 3s");
            MIDI_for_LightEffects.VolumeBeatOnDelayTimer = engine.beginTimer(3000,"MIDI_for_LightEffects.VolumeBeatOnDelay()",true);
        }
    }

    //
    // Select the beatdeck from the faderposition
    //
    if (fader == 0) { return; }   // If fader in the center, do not change

    // Determine active deck from fader, center position = 0, counts as left
    var newdeck;
    if (fader > 0) { newdeck = "[Channel2]"; }  else { newdeck = "[Channel1]"; } // Fader is to the right of the center

    // Check if the deck changes
    if (newdeck != MIDI_for_LightEffects.beatdeck){
        printDebug(2,"CrossfaderChange","Deckchange, new deck is "+newdeck);
        MIDI_for_LightEffects.beatdeck = newdeck;
        midi.sendShortMsg(0x90,48,0x64+deckNo(newdeck));       // Deckchange: Send note C mit 64+deckno
        MIDI_for_LightEffects.faderlock = true;
        MIDI_for_LightEffects.CrossfaderChangeLockTimer = engine.beginTimer(1000,"MIDI_for_LightEffects.CrossfaderChangeLock()",true);
    }
}

MIDI_for_LightEffects.CrossfaderChangeLock = function() { // Unlock the faderlock
    printDebug(1,"CrossfaderChangeLock","");
    MIDI_for_LightEffects.faderlock = false;
    MIDI_for_LightEffects.CrossfaderChangeLockTimer = undefined;
    midi.sendShortMsg(0x90,48,0x0);                 // Deckchange: Note C on value 0
    midi.sendShortMsg(0x80,48,0x0);                 // Deckchange: Note C off value 0
    // Recheck after timeout, CrossfaderChange depends on fader value, so we need to pass it
    MIDI_for_LightEffects.CrossfaderChange(engine.getValue("[Master]", "crossfader"));
}

/*
 * Send deck position of the active deck via MIDI MTC message
 */
MIDI_for_LightEffects.SendMidiMtcFullframe = function(deck) {   // Sends a MTC full frame
    if ( MIDI_for_LightEffects.beatdeck != deck ) { return; }   // Only report the active deck position
    var deckstr = "[Channel"+deck+"]";
    var fps = 2; // 2 = 25 FPS
    var PlayPositionRest = engine.getValue(deckstr, "duration") * engine.getValue(deckstr, "playposition");
    if (PlayPositionRest < 0) { PlayPositionRest = 0; }
    // Calculate hours and remove from PlayPositionRest
    var hr = Math.floor(PlayPositionRest / 3600);
    PlayPositionRest = PlayPositionRest - (hr * 3600);
    // Calculate minutes and remove from PlayPositionRest
    var mn = Math.floor(PlayPositionRest / 60);
    PlayPositionRest = PlayPositionRest - (mn * 60);
    // Calculate seconds and remove from PlayPositionRest
    var ss = Math.floor(PlayPositionRest);
    PlayPositionRest = PlayPositionRest - ss;
    // Calculate frame position and remove from PlayPositionRest
    var fr = Math.floor(PlayPositionRest * 25);
    // Assemble and send Sysex-frame
    var fullframe = [0xf0,0x7f,0x7f,0x01,0x01,(16 * fps) + hr,mn,ss,fr,0xf7];
    midi.sendSysexMsg(fullframe,10);
}

/*
 * Send beat by MIDI
 */
MIDI_for_LightEffects.BeatOutputToMidi = function(beat_active,deck,control) {   // Send MIDI-note on beat event
    if (MIDI_for_LightEffects.beatdeck != deck) { return; } // Exit if this is not the active deck
    var deckbpm = engine.getValue(deck, "bpm")-50;
    if (deckbpm <= 0) { deckbpm = 0; }
    if (deckbpm >= 127) { deckbpm = 127; }
    if (beat_active == true ) { // Beat is on, send note on
        printDebug(5,"BeatOutputToMidi"," Deck="+deck+" BPM="+deckbpm);
        midi.sendShortMsg(0x90,50,0x64);   // Beat: Note D on value 64
        midi.sendShortMsg(0x90,52,deckbpm);   // BPM: Note E on value = BPM
    } else { // Beat ist aus, sende Note aus
        printDebug(5,"BeatOutputToMidi"," Deck="+deck+" beat off");
        midi.sendShortMsg(0x90,50,0x0);   // Nobeat: Note D on value 0
        midi.sendShortMsg(0x80,50,0x0);   // Nobeat: Note D off value 0
    }
}

/* ----------- Examples and Sources
 * http://mixxx.org/wiki/doku.php/midi_scripting
 * http://mixxx.org/wiki/doku.php/mixxxcontrols
 */
