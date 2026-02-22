/****************************************************************/
/*      Stanton SCS.3d MIDI controller script v1.90             */
/*          Copyright (C) 2009-2016, Sean M. Pappalardo         */
/*      but feel free to tweak this to your heart's content!    */
/*      For Mixxx version 2.0.x                                 */
/****************************************************************/

function StantonSCS3d() {}

// ----------   Customization variables ----------
//      See http://mixxx.org/wiki/doku.php/stanton_scs.3d_mixxx_user_guide  for details
StantonSCS3d.pitchRanges = [ 0.08, 0.12, 0.5, 1.0 ];    // Pitch ranges for LED off, blue, purple, red
StantonSCS3d.fastDeckChange = false;    // Skip the flashy lights if true, for juggling
StantonSCS3d.spinningPlatter = true;    // Spinning platter LEDs
StantonSCS3d.spinningPlatterOnlyVinyl = false;  // Only show the spinning platter LEDs in vinyl mode
StantonSCS3d.spinningLights = 1;        // The number of spinning platter lights, 1 or 2
StantonSCS3d.VUMeters = true;           // Pre-fader VU meter LEDs
StantonSCS3d.markHotCues = "blue";      // Choose red or blue LEDs for marking the stored positions in TRIG mode
StantonSCS3d.jogOnLoad = true;          // Automatically change to Vinyl1 (jog) mode after loading a track if true
StantonSCS3d.globalMode = false;        // Stay in the current mode on deck changes if true.
StantonSCS3d.singleDeck = false;        // When using more than one controller, set to true to avoid easy deck changes.
                                        //  Toggle with DECK + SYNC buttons.
StantonSCS3d.deckChangeWait = 1000;     // Time in milliseconds to hold the DECK button down to avoid changing decks (multi deck mode)
StantonSCS3d.pitchAdjustment = 3;       // Pitch slider coarseness (1 = coarse, 3 = normal, 5 = fine)
StantonSCS3d.finePitchAdjustment = 3;   // Fine-mode pitch slider coarseness (1 = coarser, 3 = normal, 5 = finer)
StantonSCS3d.finePitchDefault = false;  // Swap normal and fine pitch modes

// ----------   Other global variables    ----------
StantonSCS3d.debug = false;  // Enable/disable debugging messages to the console

StantonSCS3d.id = "";   // The ID for the particular device being controlled for use in debugging, set at init time
StantonSCS3d.channel = 0;   // MIDI channel to set the device to and use
StantonSCS3d.revtime = 1.8; // Time in seconds for the virtual record to spin once.
                            //  Used for calculating the position LEDs (1.8 for 33 1/3 RPM)
StantonSCS3d.buttons = { "fx":0x20, "eq":0x26, "loop":0x22, "trig":0x28, "vinyl":0x24, "deck":0x2A };
StantonSCS3d.buttonLEDs = { 0x48:0x62, 0x4A:0x61, 0x4C:0x60, 0x4e:0x5f, 0x4f:0x67, 0x51:0x68, 0x53:0x69, 0x55:0x6a,
                            0x56:0x64, 0x58:0x65, 0x5A:0x6C, 0x5C:0x5D }; // Maps surface buttons to corresponding circle LEDs
StantonSCS3d.mode_store = { "[Channel1]":"vinyl", "[Channel2]":"vinyl", // Set vinyl mode on all decks
                            "[Channel3]":"vinyl", "[Channel4]":"vinyl" };
StantonSCS3d.scratchncue = [ false, false, false, false, false ];  // Scratch + cue mode for each deck (starts at zero)
StantonSCS3d.deck = 1;  // Currently active virtual deck
StantonSCS3d.modifier = { "cue":0, "play":0 };  // Modifier buttons (allowing alternate controls) defined on-the-fly if needed
StantonSCS3d.state = { "pitchAbs":0, "jog":0, "changedDeck":false, "deckPrev":"vinyl", "logoLit":true}; // Temporary state variables
StantonSCS3d.timer = [-1];  // Temporary storage of timer IDs
StantonSCS3d.modeSurface = { "deck":"S3+S5", "eq":"S3+S5",
                             "fx":"S3+S5", "fx2":"S3+S5", "fx3":"S3+S5",
                             "loop":"Buttons", "loop2":"Buttons", "loop3":"Buttons",
                             "trig":"Buttons", "trig2":"Buttons", "trig3":"Buttons",
                             "vinyl":"C1", "vinyl2":"C1", "vinyl3":"C1"};
StantonSCS3d.surface = { "C1":0x00, "S5":0x01, "S3":0x02, "S3+S5":0x03, "Buttons":0x04 };
StantonSCS3d.sysex = [0xF0, 0x00, 0x01, 0x60];  // Preamble for all SysEx messages for this device
StantonSCS3d.trackDuration = [0,0]; // Duration of the song on each deck (used for vinyl LEDs)
StantonSCS3d.lastLight = [-1,-1,-1,-1]; // Last circle LED values
StantonSCS3d.lastLoop = 0;  // Last-used loop LED
// Loop button controls
StantonSCS3d.loopButtons = {    0x4E: 0.125, 0x4C: 0.25, 0x4A: 0.5, 0x48: 1,
                                0x4F: 2    , 0x51: 4   , 0x53: 8  , 0x55: 16 };
// Pitch values for key change mode
StantonSCS3d.pitchPoints = {    1:{ 0x48:-0.1998, 0x4A:-0.1665, 0x4C:-0.1332, 0x4E:-0.0999, 0x56:-0.0666, 0x58:-0.0333,
                                    0x5A:0.0333, 0x5C:0.0666, 0x4F:0.0999, 0x51:0.1332, 0x53:0.1665, 0x55:0.1998 }, // 3.33% increments
                                2:{ 0x48:-0.5, 0x4A:-0.4043, 0x4C:-0.2905, 0x4E:-0.1567, 0x56:-0.1058, 0x58:-0.0548,
                                    0x5A:0.06, 0x5C:0.12, 0x4F:0.181, 0x51:0.416, 0x53:0.688, 0x55:1.0 },  // Key changes
                                3:{ 0x48:-0.4370, 0x4A:-0.3677, 0x4C:-0.3320, 0x4E:-0.2495, 0x56:-0.1567, 0x58:-0.0548,
                                    0x5A:0.12, 0x5C:0.263, 0x4F:0.338, 0x51:0.506, 0x53:0.688, 0x55:0.895 } };  // Notes
// Multiple banks of multiple cue points:
StantonSCS3d.hotCues = {    1:{ 0x48: 1, 0x4A: 2, 0x4C: 3, 0x4E: 4, 0x4F: 5, 0x51: 6,
                                0x53: 7, 0x55: 8, 0x56: 9, 0x58:10, 0x5A:11, 0x5C:12 },
                            2:{ 0x48:13, 0x4A:14, 0x4C:15, 0x4E:16, 0x4F:17, 0x51:18,
                                0x53:19, 0x55:20, 0x56:21, 0x58:22, 0x5A:23, 0x5C:24 },
                            3:{ 0x48:25, 0x4A:26, 0x4C:27, 0x4E:28, 0x4F:29, 0x51:30,
                                0x53:31, 0x55:32, 0x56:33, 0x58:34, 0x5A:35, 0x5C:36 } };
StantonSCS3d.triggerS4 = 0xFF;

// Signals to (dis)connect by mode: Group, Key, Function name
StantonSCS3d.modeSignals = {
                            "fx":[    ["[EffectRack1_EffectUnit1_Effect1]", "parameter1", "StantonSCS3d.FXS3LEDs"],
                                      ["[EffectRack1_EffectUnit1_Effect1]", "parameter2", "StantonSCS3d.FXS4LEDs"],
                                      ["[EffectRack1_EffectUnit1_Effect1]", "parameter3", "StantonSCS3d.FXS5LEDs"],
                                      ["CurrentChannel", "reverseroll", "StantonSCS3d.B11LED"],
                                      ["[EffectRack1_EffectUnit1]", "group_CurrentChannel_enable", "StantonSCS3d.B12LED"] ],
                            "fx2":[   ["[EffectRack1_EffectUnit2_Effect1]", "parameter1", "StantonSCS3d.FXS3LEDs"],
                                      ["[EffectRack1_EffectUnit2_Effect1]", "parameter2", "StantonSCS3d.FXS4LEDs"],
                                      ["[EffectRack1_EffectUnit2_Effect1]", "parameter3", "StantonSCS3d.FXS5LEDs"],
                                      ["CurrentChannel", "reverseroll", "StantonSCS3d.B11LED"],
                                      ["[EffectRack1_EffectUnit2]", "group_CurrentChannel_enable", "StantonSCS3d.B12LED"] ],
                            "fx3":[   ["[EffectRack1_EffectUnit3_Effect1]", "parameter1", "StantonSCS3d.FXS3LEDs"],
                                      ["[EffectRack1_EffectUnit3_Effect1]", "parameter2", "StantonSCS3d.FXS4LEDs"],
                                      ["[EffectRack1_EffectUnit3_Effect1]", "parameter3", "StantonSCS3d.FXS5LEDs"],
                                      ["CurrentChannel", "reverseroll", "StantonSCS3d.B11LED"],
                                      ["[EffectRack1_EffectUnit3]", "group_CurrentChannel_enable", "StantonSCS3d.B12LED"] ],
                            "eq":[    ["CurrentChannelEQ", "parameter1", "StantonSCS3d.EQLowLEDs"],
                                      ["CurrentChannelEQ", "parameter2", "StantonSCS3d.EQMidLEDs"],
                                      ["CurrentChannelEQ", "parameter3", "StantonSCS3d.EQHighLEDs"],
                                      ["CurrentChannel", "pfl", "StantonSCS3d.B11LED"] ],
                            "loop":[  ["CurrentChannel", "loop_in", "StantonSCS3d.LoopInLEDs"],
                                      ["CurrentChannel", "loop_out", "StantonSCS3d.LoopOutLEDs"],
//                                       ["CurrentChannel", "loop_enabled", "StantonSCS3d.ActiveLoop"],
//                                       ["CurrentChannel", "reloop_exit", "StantonSCS3d.ReLoopLEDs"],
                                      ["CurrentChannel", "loop_enabled", "StantonSCS3d.ReLoopLEDs"],
                                      ["CurrentChannel", "loop_halve", "StantonSCS3d.B11LED"],
                                      ["CurrentChannel", "loop_double", "StantonSCS3d.B12LED"],
                                      ["CurrentChannel", "beatloop_0.125_enabled", "StantonSCS3d.BsDLED"],
                                      ["CurrentChannel", "beatloop_0.25_enabled", "StantonSCS3d.BsCLED"],
                                      ["CurrentChannel", "beatloop_0.5_enabled", "StantonSCS3d.BsBLED"],
                                      ["CurrentChannel", "beatloop_1_enabled", "StantonSCS3d.BsALED"],
                                      ["CurrentChannel", "beatloop_2_enabled", "StantonSCS3d.BsELED"],
                                      ["CurrentChannel", "beatloop_4_enabled", "StantonSCS3d.BsFLED"],
                                      ["CurrentChannel", "beatloop_8_enabled", "StantonSCS3d.BsGLED"],
                                      ["CurrentChannel", "beatloop_16_enabled", "StantonSCS3d.BsHLED"],
                                      ["CurrentChannel", "beatloop_0.125_toggle", "StantonSCS3d.BsDaLED"],
                                      ["CurrentChannel", "beatloop_0.25_toggle", "StantonSCS3d.BsCaLED"],
                                      ["CurrentChannel", "beatloop_0.5_toggle", "StantonSCS3d.BsBaLED"],
                                      ["CurrentChannel", "beatloop_1_toggle", "StantonSCS3d.BsAaLED"],
                                      ["CurrentChannel", "beatloop_2_toggle", "StantonSCS3d.BsEaLED"],
                                      ["CurrentChannel", "beatloop_4_toggle", "StantonSCS3d.BsFaLED"],
                                      ["CurrentChannel", "beatloop_8_toggle", "StantonSCS3d.BsGaLED"],
                                      ["CurrentChannel", "beatloop_16_toggle", "StantonSCS3d.BsHaLED"] ],
                            "loop2":[ ["CurrentChannel", "pfl", "StantonSCS3d.B11LED"] ],
                            "loop3":[ ["CurrentChannel", "pfl", "StantonSCS3d.B11LED"] ],
                            "trig":[  ["CurrentChannel", "pfl", "StantonSCS3d.B11LED"],
                                      ["CurrentChannel", "hotcue_1_enabled", "StantonSCS3d.BsALED"],
                                      ["CurrentChannel", "hotcue_2_enabled", "StantonSCS3d.BsBLED"],
                                      ["CurrentChannel", "hotcue_3_enabled", "StantonSCS3d.BsCLED"],
                                      ["CurrentChannel", "hotcue_4_enabled", "StantonSCS3d.BsDLED"],
                                      ["CurrentChannel", "hotcue_5_enabled", "StantonSCS3d.BsELED"],
                                      ["CurrentChannel", "hotcue_6_enabled", "StantonSCS3d.BsFLED"],
                                      ["CurrentChannel", "hotcue_7_enabled", "StantonSCS3d.BsGLED"],
                                      ["CurrentChannel", "hotcue_8_enabled", "StantonSCS3d.BsHLED"],
                                      ["CurrentChannel", "hotcue_9_enabled", "StantonSCS3d.BsILED"],
                                      ["CurrentChannel", "hotcue_10_enabled", "StantonSCS3d.BsJLED"],
                                      ["CurrentChannel", "hotcue_11_enabled", "StantonSCS3d.BsKLED"],
                                      ["CurrentChannel", "hotcue_12_enabled", "StantonSCS3d.BsLLED"],
                                      ["CurrentChannel", "hotcue_1_activate", "StantonSCS3d.BsAaLED"],
                                      ["CurrentChannel", "hotcue_2_activate", "StantonSCS3d.BsBaLED"],
                                      ["CurrentChannel", "hotcue_3_activate", "StantonSCS3d.BsCaLED"],
                                      ["CurrentChannel", "hotcue_4_activate", "StantonSCS3d.BsDaLED"],
                                      ["CurrentChannel", "hotcue_5_activate", "StantonSCS3d.BsEaLED"],
                                      ["CurrentChannel", "hotcue_6_activate", "StantonSCS3d.BsFaLED"],
                                      ["CurrentChannel", "hotcue_7_activate", "StantonSCS3d.BsGaLED"],
                                      ["CurrentChannel", "hotcue_8_activate", "StantonSCS3d.BsHaLED"],
                                      ["CurrentChannel", "hotcue_9_activate", "StantonSCS3d.BsIaLED"],
                                      ["CurrentChannel", "hotcue_10_activate", "StantonSCS3d.BsJaLED"],
                                      ["CurrentChannel", "hotcue_11_activate", "StantonSCS3d.BsKaLED"],
                                      ["CurrentChannel", "hotcue_12_activate", "StantonSCS3d.BsLaLED"] ],
                            "trig2":[ ["CurrentChannel", "pfl", "StantonSCS3d.B11LED"],
                                      ["CurrentChannel", "hotcue_13_enabled", "StantonSCS3d.BsALED"],
                                      ["CurrentChannel", "hotcue_14_enabled", "StantonSCS3d.BsBLED"],
                                      ["CurrentChannel", "hotcue_15_enabled", "StantonSCS3d.BsCLED"],
                                      ["CurrentChannel", "hotcue_16_enabled", "StantonSCS3d.BsDLED"],
                                      ["CurrentChannel", "hotcue_17_enabled", "StantonSCS3d.BsELED"],
                                      ["CurrentChannel", "hotcue_18_enabled", "StantonSCS3d.BsFLED"],
                                      ["CurrentChannel", "hotcue_19_enabled", "StantonSCS3d.BsGLED"],
                                      ["CurrentChannel", "hotcue_20_enabled", "StantonSCS3d.BsHLED"],
                                      ["CurrentChannel", "hotcue_21_enabled", "StantonSCS3d.BsILED"],
                                      ["CurrentChannel", "hotcue_22_enabled", "StantonSCS3d.BsJLED"],
                                      ["CurrentChannel", "hotcue_23_enabled", "StantonSCS3d.BsKLED"],
                                      ["CurrentChannel", "hotcue_24_enabled", "StantonSCS3d.BsLLED"],
                                      ["CurrentChannel", "hotcue_13_activate", "StantonSCS3d.BsAaLED"],
                                      ["CurrentChannel", "hotcue_14_activate", "StantonSCS3d.BsBaLED"],
                                      ["CurrentChannel", "hotcue_15_activate", "StantonSCS3d.BsCaLED"],
                                      ["CurrentChannel", "hotcue_16_activate", "StantonSCS3d.BsDaLED"],
                                      ["CurrentChannel", "hotcue_17_activate", "StantonSCS3d.BsEaLED"],
                                      ["CurrentChannel", "hotcue_18_activate", "StantonSCS3d.BsFaLED"],
                                      ["CurrentChannel", "hotcue_19_activate", "StantonSCS3d.BsGaLED"],
                                      ["CurrentChannel", "hotcue_20_activate", "StantonSCS3d.BsHaLED"],
                                      ["CurrentChannel", "hotcue_21_activate", "StantonSCS3d.BsIaLED"],
                                      ["CurrentChannel", "hotcue_22_activate", "StantonSCS3d.BsJaLED"],
                                      ["CurrentChannel", "hotcue_23_activate", "StantonSCS3d.BsKaLED"],
                                      ["CurrentChannel", "hotcue_24_activate", "StantonSCS3d.BsLaLED"] ],
                            "trig3":[ ["CurrentChannel", "pfl", "StantonSCS3d.B11LED"],
                                      ["CurrentChannel", "hotcue_25_enabled", "StantonSCS3d.BsALED"],
                                      ["CurrentChannel", "hotcue_26_enabled", "StantonSCS3d.BsBLED"],
                                      ["CurrentChannel", "hotcue_27_enabled", "StantonSCS3d.BsCLED"],
                                      ["CurrentChannel", "hotcue_28_enabled", "StantonSCS3d.BsDLED"],
                                      ["CurrentChannel", "hotcue_29_enabled", "StantonSCS3d.BsELED"],
                                      ["CurrentChannel", "hotcue_30_enabled", "StantonSCS3d.BsFLED"],
                                      ["CurrentChannel", "hotcue_31_enabled", "StantonSCS3d.BsGLED"],
                                      ["CurrentChannel", "hotcue_32_enabled", "StantonSCS3d.BsHLED"],
                                      ["CurrentChannel", "hotcue_33_enabled", "StantonSCS3d.BsILED"],
                                      ["CurrentChannel", "hotcue_34_enabled", "StantonSCS3d.BsJLED"],
                                      ["CurrentChannel", "hotcue_35_enabled", "StantonSCS3d.BsKLED"],
                                      ["CurrentChannel", "hotcue_36_enabled", "StantonSCS3d.BsLLED"],
                                      ["CurrentChannel", "hotcue_25_activate", "StantonSCS3d.BsAaLED"],
                                      ["CurrentChannel", "hotcue_26_activate", "StantonSCS3d.BsBaLED"],
                                      ["CurrentChannel", "hotcue_27_activate", "StantonSCS3d.BsCaLED"],
                                      ["CurrentChannel", "hotcue_28_activate", "StantonSCS3d.BsDaLED"],
                                      ["CurrentChannel", "hotcue_29_activate", "StantonSCS3d.BsEaLED"],
                                      ["CurrentChannel", "hotcue_30_activate", "StantonSCS3d.BsFaLED"],
                                      ["CurrentChannel", "hotcue_31_activate", "StantonSCS3d.BsGaLED"],
                                      ["CurrentChannel", "hotcue_32_activate", "StantonSCS3d.BsHaLED"],
                                      ["CurrentChannel", "hotcue_33_activate", "StantonSCS3d.BsIaLED"],
                                      ["CurrentChannel", "hotcue_34_activate", "StantonSCS3d.BsJaLED"],
                                      ["CurrentChannel", "hotcue_35_activate", "StantonSCS3d.BsKaLED"],
                                      ["CurrentChannel", "hotcue_36_activate", "StantonSCS3d.BsLaLED"] ],
                            "vinyl":[ ["CurrentChannel", "pfl", "StantonSCS3d.B11LED"],
                                      ["CurrentChannel", "vu_meter", "StantonSCS3d.VUMeterLEDs"],
                                      ["CurrentChannel", "keylock", "StantonSCS3d.B12LED"] ],
                            "vinyl2":[["CurrentChannel", "pfl", "StantonSCS3d.B11LED"],
                                      ["CurrentChannel", "vu_meter", "StantonSCS3d.VUMeterLEDs"],
                                      ["CurrentChannel", "keylock", "StantonSCS3d.B12LED"] ],
                            "vinyl3":[],
                            "deck":[  ["[Master]","balance","StantonSCS3d.pitchLEDs"],
                                      ["[Master]","volume","StantonSCS3d.MasterVolumeLEDs"],
                                      ["[Master]","headMix","StantonSCS3d.headMixLEDs"],
                                      ["[Master]","headVolume","StantonSCS3d.headVolLEDs"],
                                      ["[Master]","crossfader","StantonSCS3d.crossFaderLEDs"] ],
                            "none":[]  // Avoids an error on forced mode changes
                            };
StantonSCS3d.commonSignals = [  ["CurrentChannel", "rate", "StantonSCS3d.pitchLEDs"],
                                ["CurrentChannel", "rateRange", "StantonSCS3d.pitchSliderLED"]
                             ];

StantonSCS3d.deckSignals = [    ["CurrentChannel", "volume", "StantonSCS3d.gainLEDs"],
                                ["CurrentChannel", "play_indicator", "StantonSCS3d.playLED"],
                                ["CurrentChannel", "cue_indicator", "StantonSCS3d.cueLED"],
                                ["CurrentChannel", "sync_enabled", "StantonSCS3d.syncLED"],
                                ["CurrentChannel", "beat_active", "StantonSCS3d.tapLED"],
                                ["CurrentChannel", "back", "StantonSCS3d.B13LED"],
                                ["CurrentChannel", "fwd", "StantonSCS3d.B14LED"]
                            ];

// ----------   Functions   ----------

StantonSCS3d.init = function (id, debug) {
    StantonSCS3d.id = id;   // Store the ID of this device for later use
    StantonSCS3d.debug = debug;

    // Find out the firmware version
    if (!StantonSCS3d.state["flat"]) midi.sendSysexMsg([0xF0, 0x7E, StantonSCS3d.channel, 0x06, 0x01, 0xF7],6);

    // Don't finish initializing until we know the firmware version.
}

StantonSCS3d.init2 = function () {
    // Set the device's MIDI channel to a known value
//     midi.sendSysexMsg(StantonSCS3d.sysex.concat([0x02, StantonSCS3d.channel, 0xF7]),7);

    var CC = 0xB0 + StantonSCS3d.channel;
    var No = 0x90 + StantonSCS3d.channel;
    midi.sendShortMsg(CC,0x7B,0x00);  // Extinguish all LEDs

    for (i=0x48; i<=0x5c; i++) midi.sendShortMsg(No,i,0x40); // Set surface LEDs to black default

    // Force change to first deck, initializing the control surface & LEDs and connecting signals in the process

    // Set active deck to the last one so the below will switch to #1.
    StantonSCS3d.deck = engine.getValue("[App]", "num_decks");
    if (StantonSCS3d.singleDeck)    // Force timer to expire so the deck change happens
        StantonSCS3d.modifier["deckTime"] = new Date() - StantonSCS3d.deckChangeWait;
    StantonSCS3d.DeckChangeP1(StantonSCS3d.channel, StantonSCS3d.buttons["deck"], "null", 0x90+StantonSCS3d.channel);
    StantonSCS3d.DeckChangeP1(StantonSCS3d.channel, StantonSCS3d.buttons["deck"], "null", 0x80+StantonSCS3d.channel);

    // Connect the playposition functions permanently since they disrupt playback if connected on the fly
    if (StantonSCS3d.spinningPlatter) {
        engine.connectControl("[Channel1]","playposition","StantonSCS3d.circleLEDs1");
        engine.connectControl("[Channel2]","playposition","StantonSCS3d.circleLEDs2");
        engine.connectControl("[Channel3]","playposition","StantonSCS3d.circleLEDs3");
        engine.connectControl("[Channel4]","playposition","StantonSCS3d.circleLEDs4");
        engine.connectControl("[Channel1]","duration","StantonSCS3d.durationChange1");
        engine.connectControl("[Channel2]","duration","StantonSCS3d.durationChange2");
        engine.connectControl("[Channel3]","duration","StantonSCS3d.durationChange3");
        engine.connectControl("[Channel4]","duration","StantonSCS3d.durationChange4");
    }

    //  Initialize the spinning platter LEDs if the mapping is loaded after a song is
    StantonSCS3d.durationChange1(engine.getValue("[Channel1]","duration"));
    StantonSCS3d.durationChange2(engine.getValue("[Channel2]","duration"));
    StantonSCS3d.durationChange3(engine.getValue("[Channel3]","duration"));
    StantonSCS3d.durationChange4(engine.getValue("[Channel4]","duration"));

//     print ("StantonSCS3d: \""+StantonSCS3d.id+"\" on MIDI channel "+(StantonSCS3d.channel+1)+" initialized.");
    print ("StantonSCS3d: \""+StantonSCS3d.id+"\" initialized.");
}

StantonSCS3d.statusResponse = function (data, length) {
    var statusResponsePreamble=[0xF0,0x7E,0,6,2,0,1,0x60,0x2c,1,1,0];
    // Check if this SysEx is the one we're looking for
    var i=0;
    var comp=true;
    while (i<12 && comp) {
        if (statusResponsePreamble[i]!=data[i]) comp=false;
        i++;
    }

    if (comp) {
        print ("Stanton SCS.3d v"+data[12]+", "+(2008+data[13])+"-"+data[14]+"-"+data[15]);

        if ((2008+data[13])==2009 && !StantonSCS3d.state["flat"]) {
            // If the year is 2009, this is the test "smart" firmware
            print ("WARNING: This SCS.3d is running test firmware and should be re-flashed with production firmware!\n\
                     (Contact Stanton support.)  Changing unit to flat mode...");
            //  Send the command to change the device to flat mode which is mostly compatible
            midi.sendSysexMsg(StantonSCS3d.sysex.concat([0x10, StantonSCS3d.channel, 0xF7]),7);
            StantonSCS3d.state["flat"]=true;
        }
    }
    StantonSCS3d.init2();
}
StantonSCS3d.incomingData = StantonSCS3d.statusResponse;

StantonSCS3d.shutdown = function () {   // called when the MIDI device is closed

    StantonSCS3d.stopTimers();

    var CC = 0xB0 + StantonSCS3d.channel;
    var No = 0x90 + StantonSCS3d.channel;

    for (i=0x48; i<=0x5c; i++) midi.sendShortMsg(No,i,0x40); // Set surface LEDs to black default
    midi.sendShortMsg(CC,0x7B,0x00);  // Extinguish all LEDs

//     print ("StantonSCS3d: \""+StantonSCS3d.id+"\" on MIDI channel "+(StantonSCS3d.channel+1)+" shut down.");
    print ("StantonSCS3d: \""+StantonSCS3d.id+"\" shut down.");
}

StantonSCS3d.stopTimers = function () {
    for (var i=0; i<StantonSCS3d.timer.length; i++) {
        if (StantonSCS3d.timer[i] != -1) {
            engine.stopTimer(StantonSCS3d.timer[i]);
            StantonSCS3d.timer[i] = -1;
        }
    }
}

// (Dis)connects the appropriate Mixxx control signals to/from functions based on the currently controlled deck and what mode the controller is in
StantonSCS3d.connectSurfaceSignals = function (channel, disconnect) {

    var signalList = StantonSCS3d.modeSignals[StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]];
    for (i=0; i<signalList.length; i++) {
        var group = signalList[i][0];
        if (group=="CurrentChannel") group = "[Channel"+StantonSCS3d.deck+"]";
        if (group=="CurrentChannelEQ") group = "[EqualizerRack1_[Channel"+StantonSCS3d.deck+"]_Effect1]";

        var item = signalList[i][1];
        item = item.replace("CurrentChannel","[Channel"+StantonSCS3d.deck+"]");

        engine.connectControl(group,item,signalList[i][2],disconnect);

        // If connecting a signal, cause it to fire to update the LEDs
        if (!disconnect) engine.trigger(group,item);
//         if (!disconnect) {
//             // Alternate:
//             var command = signalList[i][2]+"("+engine.getValue(group,item)+")";
// //             print("StantonSCS3d: command="+command);
//             eval(command);
//         }
        if (StantonSCS3d.debug) {
            if (disconnect) print("StantonSCS3d: "+group+","+item+" disconnected from "+signalList[i][2]);
            else print("StantonSCS3d: "+group+","+item+" connected to "+signalList[i][2]);
        }
    }
    // If disconnecting signals, darken the LEDs on the control surface & soft buttons
    if (disconnect) {
        var CC = 0xB0 + channel;
        midi.sendShortMsg(CC,0x62,0x00);  // C1 LEDs off
        midi.sendShortMsg(CC,0x0C,0x00);  // S3 LEDs off
        midi.sendShortMsg(CC,0x01,0x00);  // S4 LEDs off
        midi.sendShortMsg(CC,0x0E,0x00);  // S5 LEDs off
    }
}

// (Dis)connects the mode-independent Mixxx control signals to/from functions based on the currently controlled virtual deck
StantonSCS3d.connectDeckSignals = function (channel, disconnect, list) {
    var signalList;
    switch (list) {
        case "common": signalList = StantonSCS3d.commonSignals; break;
        case "deck":
        default: signalList = StantonSCS3d.deckSignals; break;
    }
    for (i=0; i<signalList.length; i++) {
        var group = signalList[i][0];
        var name = signalList[i][1];
        if (group=="CurrentChannel") group = "[Channel"+StantonSCS3d.deck+"]";
        engine.connectControl(group,name,signalList[i][2],disconnect);
//        if (StantonSCS3d.debug) print("StantonSCS3d: (dis)connected "+group+","+name+" to/from "+signalList[i][2]);

        // If connecting a signal, update the LEDs
        if (!disconnect) {
            var currentValue = engine.getValue(group,name);
            switch (name) {
                case "play_indicator":
                    StantonSCS3d.playLED(currentValue);
                    break;
                case "cue_indicator":
                    StantonSCS3d.cueLED(currentValue);
                    break;
                case "sync_enabled":
                    StantonSCS3d.syncLED(currentValue);
                    break;
                default:    // Cause the signal to fire to update LEDs
                    engine.trigger(group,name);
//                     // Alternate:
//                     var command = signalList[i][2]+"("+engine.getValue(group,name)+")";
// //                     print("StantonSCS3d: command="+command);
//                     eval(command);
                    break;
            }
        }
        if (StantonSCS3d.debug) {
            if (disconnect) print("StantonSCS3d: "+group+","+signalList[i][1]+" disconnected from "+signalList[i][2]);
            else print("StantonSCS3d: "+group+","+signalList[i][1]+" connected to "+signalList[i][2]);
        }
    }
    // If disconnecting signals, darken the corresponding LEDs
    if (disconnect) {
        var CC = 0xB0 + channel;
        var No = 0x90 + channel;
        switch (list) {
            case "common":
                midi.sendShortMsg(CC,0x07,0x00);  // Gain LEDs off
                midi.sendShortMsg(CC,0x03,0x00);  // Pitch LEDs off
                break;
            case "deck":
                midi.sendShortMsg(No,0x6D,0x00);  // PLAY button blue
                midi.sendShortMsg(No,0x6E,0x00);  // CUE button blue
                midi.sendShortMsg(No,0x6F,0x00);  // SYNC button blue
                midi.sendShortMsg(No,0x70,0x00);  // TAP button blue
                break;
        }
    }
}

// Sets all mode buttons except Deck to the same color
StantonSCS3d.modeButtonsColor = function (channel, color) {
    var byte1 = 0x90 + channel;
    midi.sendShortMsg(byte1,StantonSCS3d.buttons["fx"],color); // Set FX button color
    midi.sendShortMsg(byte1,StantonSCS3d.buttons["eq"],color); // Set EQ button color
    midi.sendShortMsg(byte1,StantonSCS3d.buttons["loop"],color); // Set Loop button color
    midi.sendShortMsg(byte1,StantonSCS3d.buttons["trig"],color); // Set Trig button color
    midi.sendShortMsg(byte1,StantonSCS3d.buttons["vinyl"],color); // Set Vinyl button color
    midi.sendShortMsg(byte1,StantonSCS3d.buttons["deck"],color); // Set Deck button color
}

// Sets all four soft buttons to the same color
StantonSCS3d.softButtonsColor = function (channel, color) {
    var byte1 = 0x90 + channel;
    midi.sendShortMsg(byte1,0x2c,color); // Set B11 button color
    midi.sendShortMsg(byte1,0x2e,color); // Set B12 button color
    midi.sendShortMsg(byte1,0x30,color); // Set B13 button color
    midi.sendShortMsg(byte1,0x32,color); // Set B14 button color
}

// Sets color of side circle LEDs (used for deck change effect)
StantonSCS3d.circleLEDsColor = function (channel, color, side) {
    var byte1 = 0x90 + channel;
    var start;
    var end;
    if (side=="left") { start = 0x5e; end = 0x63; }
    else { start = 0x66; end = 0x6b; }
    for (i=start; i<=end; i++) midi.sendShortMsg(byte1,i,color);
}

StantonSCS3d.pitch = function (channel, control, value) {   // Lower the sensitivity of the pitch slider
    // If in DECK mode, ignore this.
    if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]=="deck") return;

    // Ignore if doing pitch bend
    if (engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_up")!=0
        || engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_down")!=0) return;

    var currentValue = engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate");
    var newValue;
    var modDeck = StantonSCS3d.modifier[StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]]==1;
    var swap = StantonSCS3d.finePitchDefault;
    if ((modDeck && !swap) || (!modDeck && swap)) {
        // Fine pitch adjust
        var pitchRange = engine.getValue("[Channel"+StantonSCS3d.deck+"]","rateRange");
        newValue = currentValue+(value-64)/(20000*StantonSCS3d.finePitchAdjustment*pitchRange);
    }
    else newValue = currentValue+(value-64)/(86*StantonSCS3d.pitchAdjustment);
    if (newValue<-1) newValue=-1.0;
    if (newValue>1) newValue=1.0;
    engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate",newValue);
}

StantonSCS3d.pitchAbsolute = function (channel, control, value) {
    // Adjust the master balance if in DECK mode
    if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]=="deck") {
        var newValue = (value-64)/64;
        engine.setValue("[Master]","balance",newValue);
        return;
    }

    // Disable if doing fine adjustments (holding down the current mode button)
    if (StantonSCS3d.modifier[StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]]==1) return;

    // Ignore if we're already bending
    if (engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_up")!=0
        || engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_down")!=0) return;

    // --- Pitch bending at the edges of the slider ---
    if (StantonSCS3d.state["pitchAbs"]==0) StantonSCS3d.state["pitchAbs"]=value;    // Log the initial value

    // Ignore if the slider was first touched in the middle
    if (StantonSCS3d.state["pitchAbs"]>=10 && StantonSCS3d.state["pitchAbs"]<=117) return;

    if (engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate_dir") == -1) {   // Go in the appropriate direction
        if (value<10) engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_up",1);
        if (value>117) engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_down",1);
    }
    else {
        if (value<10) engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_down",1);
        if (value>117) engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_up",1);
    }
}

StantonSCS3d.pitchTouch = function (channel, control, value, status) {
    if ((status & 0xF0) == 0x80) {   // If button up
        StantonSCS3d.state["pitchAbs"]=0;   // Clear the initial value
        if (engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_down") != 0)
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_down",0);
        if (engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_up") != 0)
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate_temp_up",0);
    }
}

StantonSCS3d.gain = function (channel, control, value) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    // Ignore if in DECK mode
    if (currentMode == "deck") return;
    if (StantonSCS3d.modifier[currentMode]==1) return;
    engine.setValue("[Channel"+StantonSCS3d.deck+"]","volume",value/127);
}

StantonSCS3d.gainRelative = function (channel, control, value) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];

    // If mode button held, (except when Deck is held down in non-single-deck mode,)
    if (StantonSCS3d.modifier[currentMode]==1 &&
        (currentMode != "deck" || StantonSCS3d.singleDeck)) {
        var newValue = engine.getValue("[Channel"+StantonSCS3d.deck+"]","pregain")+(value-64)/128;
        if (newValue<0.0) newValue=0.0;
        if (newValue>4.0) newValue=4.0;
        engine.setValue("[Channel"+StantonSCS3d.deck+"]","pregain",newValue);
        var add = StantonSCS3d.BoostCut(9,newValue, 0.0, 1.0, 4.0, 4, 4);
        var byte1 = 0xB0 + channel;
        midi.sendShortMsg(byte1,0x07,0x15+add);
    }
    else if (currentMode == "deck") { // If in DECK mode, adjust Master Volume
        var newValue = engine.getValue("[Master]","volume")+(value-64)/256;
        if (newValue<0.0) newValue=0.0;
        if (newValue>5.0) newValue=5.0;
        engine.setValue("[Master]","volume",newValue);
        return;
    }
}

StantonSCS3d.playButton = function (channel, control, value, status) {
    var byte1 = 0x90 + channel;
    if ((status & 0xF0) == 0x90) {    // If button down
        // If in single-deck mode and in DECK mode and Deck is held when this is pressed, change decks
        var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
        if (StantonSCS3d.singleDeck && currentMode == "deck" && StantonSCS3d.modifier["deck"]==1) {
            StantonSCS3d.DeckChangeP1(channel, StantonSCS3d.buttons["deck"], "null", 0x90+channel);
            StantonSCS3d.DeckChangeP1(channel, StantonSCS3d.buttons["deck"], "null", 0x80+channel);
            return
        }
        StantonSCS3d.modifier["play"]=1;
        var currentlyPlaying = engine.getValue("[Channel"+StantonSCS3d.deck+"]","play");
        engine.setValue("[Channel"+StantonSCS3d.deck+"]","play", !currentlyPlaying);
        return;
    }
    StantonSCS3d.modifier["play"]=0;
}

StantonSCS3d.cueButton = function (channel, control, value, status) {
    var byte1 = 0x90 + channel;
    if ((status & 0xF0) != 0x80) {    // If button down
        // If VINYL held down in a manipulation mode
        if (StantonSCS3d.modifier["vinyl"] || StantonSCS3d.modifier["vinyl2"]) {
            // Force the timer on the mode button to expire to avoid unintended mode changes
            StantonSCS3d.modifier["time"] = new Date()-1000;
            // Toggle scratch & cue mode
            if (StantonSCS3d.scratchncue[StantonSCS3d.deck]) StantonSCS3d.scratchncue[StantonSCS3d.deck]=false;
            else StantonSCS3d.scratchncue[StantonSCS3d.deck]=true;
            // Flash the Stanton logo to acknowledge
            midi.sendShortMsg(0x90,0x7A,0x00);
            midi.sendShortMsg(0x90,0x7A,0x01);
        }
        else engine.setValue("[Channel"+StantonSCS3d.deck+"]","cue_default",1);
        StantonSCS3d.modifier["cue"]=1;   // Set button modifier flag
        return;
    }
    engine.setValue("[Channel"+StantonSCS3d.deck+"]","cue_default",0);
    StantonSCS3d.modifier["cue"]=0;   // Clear button modifier flag
}

StantonSCS3d.syncButton = function (channel, control, value, status) {
    var byte1 = 0x90 + channel;
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if ((status & 0xF0) == 0x90) {    // If button down
        // If in DECK mode and Deck is held when this is pressed,
        //  toggle between multi and single deck mode
        if (currentMode == "deck" && (!StantonSCS3d.singleDeck || StantonSCS3d.modifier["deck"]==1)) {
            // Flash the Stanton logo to acknowledge
            midi.sendShortMsg(0x90,0x7A,0x00);
            midi.sendShortMsg(0x90,0x7A,0x01);
            if (StantonSCS3d.singleDeck) {
                if (StantonSCS3d.debug) print("StantonSCS3d: Switching to multiple-deck control mode");
                StantonSCS3d.singleDeck = false;
                // Prevent ending up in now-defunct "Deck" mode
                if (StantonSCS3d.mode_store["[Channel1]"]=="deck") StantonSCS3d.mode_store["[Channel1]"] = "vinyl";
                if (StantonSCS3d.mode_store["[Channel2]"]=="deck") StantonSCS3d.mode_store["[Channel2]"] = "vinyl";
                // Do deck change to acknowledge
                StantonSCS3d.DeckChangeP1(channel, StantonSCS3d.buttons["deck"], "null", 0x90+channel);
            }
            else {
                if (StantonSCS3d.debug) print("StantonSCS3d: Switching to single-deck control mode");
                StantonSCS3d.singleDeck = true;
            }
        } else if (currentMode != "deck" && StantonSCS3d.modifier[currentMode]==1) {
            // If the current mode button is held down (and it's not DECK mode)
            var curval = engine.getValue("[Channel"+StantonSCS3d.deck+"]","quantize");
            engine.setValue("[Channel"+StantonSCS3d.deck+"]", "quantize",
                            !curval);
            StantonSCS3d.syncLED(!curval);
        }
        else {
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","sync_enabled",1);
            StantonSCS3d.modifier["masterSync"] = new Date();
            return;
        }
    }
    // If button up
    // Don't touch sync_enabled if we toggled modes
    if (currentMode == "deck" && (!StantonSCS3d.singleDeck || StantonSCS3d.modifier["deck"]==1)) return;
    // Otherwise, if it's been held less than 1/3 of a second, disable sync
    if (new Date() - StantonSCS3d.modifier["masterSync"] < 300) {
        engine.setValue("[Channel"+StantonSCS3d.deck+"]","sync_enabled",0);
    }
}

StantonSCS3d.tapButton = function (channel, control, value, status) {
    var byte1 = 0x90 + channel;
    // If in DECK mode, and not in single-deck mode
    if (!StantonSCS3d.singleDeck && StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]=="deck") {
        engine.setValue("[Master]","crossfader",0.0);   // Reset cross-fader to center
        return;
    }
    if ((status & 0xF0) == 0x90) {    // If button down
        if (StantonSCS3d.debug) print("StantonSCS3d: TAP");
//         midi.sendShortMsg(byte1,control,0x01);  // TAP button red
        bpm.tapButton(StantonSCS3d.deck);
        return;
    }
//     midi.sendShortMsg(byte1,control,0x00);  // TAP button blue
}

StantonSCS3d.B11 = function (channel, control, value, status) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    var byte1 = 0x90 + channel;
    if ((status & 0xF0) == 0x90) {    // If button down
        StantonSCS3d.modifier["B11"]=1;   // Set button modifier flag
        // If mode button held, (except when Deck is held down in non-single-deck mode,)
        if (StantonSCS3d.modifier[currentMode]==1 &&
            (currentMode != "deck" || StantonSCS3d.singleDeck)) {
            midi.sendShortMsg(byte1,control,0x01); // Make button red
            // Reset channel pre-fader gain to center
            engine.reset("[Channel"+StantonSCS3d.deck+"]","pregain");
            // Update the LEDs
            var add = StantonSCS3d.BoostCut(9,1.0, 0.0, 1.0, 4.0, 5, 4);
            midi.sendShortMsg(0xB0 + channel,0x07,0x15+add);
            return;
        }

        switch (currentMode) {
            case "vinyl3":
                midi.sendShortMsg(byte1,control,0x01); // Make button red
                engine.setValue("[Playlist]","SelectPrevPlaylist",1);
                break;
            case "deck":
                midi.sendShortMsg(byte1,control,0x01); // Make button red
                engine.reset("[Master]","volume");
                break;
            case "fx":
            case "loop":
                break;  // Do nothing here
            default:
                engine.setValue("[Channel"+StantonSCS3d.deck+"]","pfl",!engine.getValue("[Channel"+StantonSCS3d.deck+"]","pfl"));
                break;
        }
    }
    else {  // Button up
        StantonSCS3d.modifier["B11"]=0;   // Clear button modifier flag
        if (StantonSCS3d.modifier[currentMode]==1) {
            midi.sendShortMsg(byte1,control,0x02); // Make button blue if a mode button is held
            return;
        }
        switch (currentMode) {
            case "deck":
            case "vinyl3":
                midi.sendShortMsg(byte1,control,0x02); // Make button blue
                break;
        }
    }

    switch (currentMode) {  // In either case (for toggling)
        case "fx":
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","reverseroll",!engine.getValue("[Channel"+StantonSCS3d.deck+"]","reverseroll"));
            break;
        case "loop":
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","loop_halve",!engine.getValue("[Channel"+StantonSCS3d.deck+"]","loop_halve"));
            break;
    }
}

StantonSCS3d.B12 = function (channel, control, value, status, group) {
    var byte1 = 0x90 + channel;
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];

    if ((status & 0xF0) == 0x90) {    // If button down
        StantonSCS3d.modifier["B12"]=1;   // Set button modifier flag
        if (currentMode != "deck" && StantonSCS3d.modifier[currentMode]==1) {
            midi.sendShortMsg(byte1,control,0x01); // Make button red
            // Reset pitch to 0 if mode button held down
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate",0);
            return;
        }
        var modeIndex = currentMode.charAt(currentMode.length-1);
        if (modeIndex != "2" && modeIndex != "3") modeIndex = "1";

        switch (currentMode) {
            case "deck":
                midi.sendShortMsg(byte1,control,0x01); // Make button red
                engine.reset("[Master]","balance"); // Reset master balance to center
                break;
            case "fx":
            case "fx2":
            case "fx3":
                engine.setValue("[EffectRack1_EffectUnit"+modeIndex+"]",
                                "group_[Channel"+StantonSCS3d.deck+"]_enable",
                                !engine.getValue("[EffectRack1_EffectUnit"+modeIndex+"]",
                                                 "group_[Channel"+StantonSCS3d.deck+"]_enable"));
                break;
            case "vinyl":
            case "vinyl2":
                engine.setValue("[Channel"+StantonSCS3d.deck+"]","keylock",
                                !engine.getValue("[Channel"+StantonSCS3d.deck+"]","keylock"));
                break;
            case "vinyl3":
                midi.sendShortMsg(byte1,control,0x01); // Make button red
                engine.setValue("[Playlist]","SelectNextPlaylist",1);
                break;
            case "loop": break; // Do nothing
            case "loop2":
            case "loop3":
                midi.sendShortMsg(byte1,control,0x01); // Make button red
                engine.reset("[Channel"+StantonSCS3d.deck+"]","rate");
            default:
                // Pitch range toggle
                midi.sendShortMsg(byte1,control,0x01); // Make button red
                // Round to two decimal places to avoid double-precision comparison problems
                var currentRange = Math.round(engine.getValue("[Channel"+StantonSCS3d.deck+"]","rateRange")*100)/100;
                //                     print ("Current range="+currentRange);
                switch (true) {
                    case (currentRange<StantonSCS3d.pitchRanges[0]):
                        engine.setValue("[Channel"+StantonSCS3d.deck+"]","rateRange",StantonSCS3d.pitchRanges[0]);
                        break;
                    case (currentRange<StantonSCS3d.pitchRanges[1]):
                        engine.setValue("[Channel"+StantonSCS3d.deck+"]","rateRange",StantonSCS3d.pitchRanges[1]);
                        break;
                    case (currentRange<StantonSCS3d.pitchRanges[2]):
                        engine.setValue("[Channel"+StantonSCS3d.deck+"]","rateRange",StantonSCS3d.pitchRanges[2]);
                        break;
                    case (currentRange<StantonSCS3d.pitchRanges[3]):
                        engine.setValue("[Channel"+StantonSCS3d.deck+"]","rateRange",StantonSCS3d.pitchRanges[3]);
                        break;
                    case (currentRange>=StantonSCS3d.pitchRanges[3]):
                        engine.setValue("[Channel"+StantonSCS3d.deck+"]","rateRange",StantonSCS3d.pitchRanges[0]);
                        break;
                }
                break;
        }
    }
    else {  // If button up
        StantonSCS3d.modifier["B12"]=0;   // Clear button modifier flag
        if (StantonSCS3d.modifier[currentMode]==1) {
            midi.sendShortMsg(byte1,control,0x02); // Make button blue if a mode button is held
            return;
        }
        switch (currentMode) {
            case "fx":
            case "fx2":
            case "fx3":
            case "vinyl":
            case "vinyl2":
            case "loop":
                break;  // Do nothing
            case "vinyl3":
                engine.setValue("[Playlist]","SelectNextPlaylist",0);
                // No break! We want the LED to be reset as well.
            default:
                midi.sendShortMsg(byte1,control,0x02); // Make button blue
                break;
        }
    }

    switch (currentMode) {  // In either case (for toggling)
        case "loop":
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","loop_double",
                            !engine.getValue("[Channel"+StantonSCS3d.deck+"]","loop_double"));
            break;
    }
}

StantonSCS3d.B13 = function (channel, control, value, status) {
    var byte1 = 0x90 + channel;
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if ((status & 0xF0) == 0x90) {    // If button down
        StantonSCS3d.modifier["B13"]=1;   // Set button modifier flag
        if (currentMode == "vinyl3")
            midi.sendShortMsg(byte1,control,0x01); // Make button red
    }
    else {
        StantonSCS3d.modifier["B13"]=0;   // Clear button modifier flag
        if (currentMode == "vinyl3")
            midi.sendShortMsg(byte1,control,0x02); // Make button blue
    }
    switch (currentMode) {
        case "vinyl3":
                if ((status & 0xF0) == 0x90) {    // If button down
                    engine.setValue("[Playlist]","SelectPrevTrack",1);
                }
                else engine.setValue("[Playlist]","SelectPrevTrack",0);
                break;
        default:
            if ((status & 0xF0) == 0x90) {    // If button down
                engine.setValue("[Channel"+StantonSCS3d.deck+"]","back",1);
                return;
            }
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","back",0);
            break;
    }
}

StantonSCS3d.B14 = function (channel, control, value, status) {
    var byte1 = 0x90 + channel;
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if ((status & 0xF0) == 0x90) {    // If button down
        StantonSCS3d.modifier["B14"]=1;   // Set button modifier flag
        if (currentMode == "vinyl3")
            midi.sendShortMsg(byte1,control,0x01); // Make button red
    }
    else {
        StantonSCS3d.modifier["B14"]=0;   // Clear button modifier flag
        if (currentMode == "vinyl3")
            midi.sendShortMsg(byte1,control,0x02); // Make button blue
    }
    switch (currentMode) {
        case "vinyl3":
                if ((status & 0xF0) == 0x90) {    // If button down
                    engine.setValue("[Playlist]","SelectNextTrack",1);
                }
                else engine.setValue("[Playlist]","SelectNextTrack",0);
                break;
        default:
            if ((status & 0xF0) == 0x90) {    // If button down
                engine.setValue("[Channel"+StantonSCS3d.deck+"]","fwd",1);
                return;
            }
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","fwd",0);
            break;
    }
}

// ----------   Mode buttons  ----------

StantonSCS3d.modeButton = function (channel, control, status, modeName) {
    var byte1 = 0x90 + channel;
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if ((status & 0xF0) == 0x90) {    // If button down
        midi.sendShortMsg(byte1,control,0x03); // Make button purple
        StantonSCS3d.modifier[modeName]=1;   // Set mode modifier flag
        if (currentMode == modeName) {
            StantonSCS3d.modifier["time"] = new Date();  // Store the current time in milliseconds
            StantonSCS3d.B11LED(0); // B11 blue
            StantonSCS3d.B12LED(0); // B12 blue
            // Set Gain LEDs to pregain value
            var add = StantonSCS3d.BoostCut(9,engine.getValue("[Channel"+StantonSCS3d.deck+"]","pregain"), 0.0, 1.0, 4.0, 5, 4);
            midi.sendShortMsg(0xB0+channel,0x07,0x15+add);
            // Set SYNC button to value of quantize control
            if (currentMode != "deck") {
                StantonSCS3d.syncLED(
                    engine.getValue("[Channel"+StantonSCS3d.deck+"]","quantize"));
            }
        }
        else StantonSCS3d.modifier["time"] = 0.0;
        return;
    }
    StantonSCS3d.modifier[currentMode] = StantonSCS3d.modifier[modeName] = 0;   // Clear mode modifier flags
    StantonSCS3d.gainLEDs(engine.getValue("[Channel"+StantonSCS3d.deck+"]","volume"));  // Restore Gain LEDs
    StantonSCS3d.modeButtonsColor(channel,0x02);  // Make all mode buttons blue
    engine.trigger("[Channel"+StantonSCS3d.deck+"]","sync_enabled"); // U   pdate SYNC LED
    // If trying to switch to the same mode, or the same button was held down for over 1/3 of a second, stay in the current mode
    if (currentMode == modeName || (StantonSCS3d.modifier["time"] != 0.0 && ((new Date() - StantonSCS3d.modifier["time"])>300))) {
        switch (currentMode.charAt(currentMode.length-1)) {   // Return the button to its original color
            case "2": midi.sendShortMsg(byte1,control,0x03); break;   // Make button purple
            case "3": midi.sendShortMsg(byte1,control,0x00); break;   // Make button black
            default:  midi.sendShortMsg(byte1,control,0x01); break;  // Make button red
        }
        StantonSCS3d.connectSurfaceSignals(channel);  // Re-trigger signals
        return;
    }
    // So if we've reached this point, modeName != currentMode, i.e. we're about to change modes

    if (StantonSCS3d.debug) print("StantonSCS3d: Switching to "+modeName.toUpperCase()+" mode on deck "+StantonSCS3d.deck);
    switch (modeName.charAt(modeName.length-1)) {   // Set the button to its new color
        case "2": midi.sendShortMsg(byte1,control,0x03); break;   // Make button purple
        case "3": midi.sendShortMsg(byte1,control,0x00); break;   // Make button black
        default:  midi.sendShortMsg(byte1,control,0x01); break;  // Make button red
    }
    StantonSCS3d.connectSurfaceSignals(channel,true);  // Disconnect previous ones
    StantonSCS3d.softButtonsColor(channel,0x02);  // Make the soft buttons blue
    switch (currentMode) {    // Special recovery from certain modes
        case "vinyl2":
            // So we don't get stuck at some strange speed when switching from a scratching mode
            engine.scratchDisable(StantonSCS3d.deck);
            break;
        case "loop":
            var redButtonLEDs = [0x48, 0x4a, 0x4c, 0x4e, 0x4f, 0x51, 0x53, 0x55, 0x56, 0x58, 0x59, 0x5A, 0x5C];
            for (i=0; i<redButtonLEDs.length; i++)
                midi.sendShortMsg(byte1,redButtonLEDs[i],0x40); // Set them to black
                break;
        case "loop2":
        case "loop3":
        case "trig":
        case "trig2":
        case "trig3":
                // If switching to loop2-3 or trig from either, skip this
                if (modeName.substring(0,4)=="trig" || (modeName != "loop" && modeName.substring(0,4)=="loop")) break;
                var redButtonLEDs = [0x48, 0x4a, 0x4c, 0x4e, 0x4f, 0x51, 0x53, 0x55];
                for (i=0; i<redButtonLEDs.length; i++)
                    midi.sendShortMsg(byte1,redButtonLEDs[i],0x40); // Set them to black
                for (i=0x56; i<=0x5c; i++)
                    midi.sendShortMsg(byte1,i,0x40); // Set center slider to black
            break;
        case "deck":
            StantonSCS3d.state["forceGain"]=true;
            StantonSCS3d.gainLEDs(engine.getValue("[Channel"+StantonSCS3d.deck+"]","volume"));  // Restore Gain LEDs
            StantonSCS3d.state["forceGain"]=false;
            StantonSCS3d.connectDeckSignals(channel,false,"common");    // Connect static common signals
            break;
    }
//     if (StantonSCS3d.modeSurface[modeName] != StantonSCS3d.modeSurface[currentMode])    // If different,
        midi.sendSysexMsg(StantonSCS3d.sysex.concat([0x01,
            StantonSCS3d.surface[StantonSCS3d.modeSurface[modeName]], 0xF7]),7);  // Configure surface
    switch (modeName) {    // Prep for certain modes
        case "loop":
            var redButtonLEDs = [0x48, 0x4a, 0x4c, 0x4e, 0x4f, 0x51, 0x53, 0x55, 0x56, 0x58, 0x59, 0x5A, 0x5C];
            for (i=0; i<redButtonLEDs.length; i++)
                midi.sendShortMsg(byte1,redButtonLEDs[i],0x41); // Set them to red dim
            break;
        case "loop2":
        case "loop3":
        case "trig":
        case "trig2":
        case "trig3":
                // If switching to loop2-3 or trig from any other mode, prep the surface background LEDs

                var index = modeName.charAt(modeName.length-1);
                if (index != "2" && index != "3") index = "1";

                var redButtonLEDs = [0x48, 0x4a, 0x4c, 0x4e, 0x4f, 0x51, 0x53, 0x55, 0x56, 0x58, 0x5A, 0x5C];
                if ( (currentMode.substring(0,4) != "trig"
                      && (currentMode == "loop" || currentMode.substring(0,4) != "loop"))
                      || StantonSCS3d.state["changedDeck"]) {
                    StantonSCS3d.state["changedDeck"] = false;
                    for (i=0; i<redButtonLEDs.length; i++)
                        midi.sendShortMsg(byte1,redButtonLEDs[i],0x41); // Set them to red dim
                }
            break;
        case "deck":
            StantonSCS3d.connectDeckSignals(channel,true,"common");    // Disconnect static common signals
            midi.sendShortMsg(byte1,0x3D,0x00);  // Pitch LED black
            midi.sendShortMsg(byte1,0x3E,0x00);
            break;
    }
    StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = modeName;
    StantonSCS3d.connectSurfaceSignals(channel);  // Connect new ones
    // Force the circle LEDs to light if applicable
    StantonSCS3d.lastLight[StantonSCS3d.deck]=-1;
    StantonSCS3d.circleLEDs(engine.getValue("[Channel"+StantonSCS3d.deck+"]","playposition"));
}

StantonSCS3d.FX = function (channel, control, value, status) {
    var mode;
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];

    switch (currentMode) {
        case "fx":
            if ((status & 0xF0) == 0x80) mode = "fx2";
            else mode = "fx";
            break;
        case "fx2":
            if ((status & 0xF0) == 0x80) mode = "fx3";
            else mode = "fx2";
            break;
        case "fx3":
            if ((status & 0xF0) == 0x80) mode = "fx";
            else mode = "fx3";
            break;
        default: mode = "fx";
    }
    StantonSCS3d.modeButton(channel, control, status, mode);
}

StantonSCS3d.EQ = function (channel, control, value, status) {
    StantonSCS3d.modeButton(channel, control, status, "eq");
}

StantonSCS3d.Loop = function (channel, control, value, status) {
    var mode;
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];

    switch (currentMode) {
        case "loop":
            if ((status & 0xF0) == 0x80) mode = "loop2";
            else mode = "loop";
            break;
        case "loop2":
            if ((status & 0xF0) == 0x80) mode = "loop3";
            else mode = "loop2";
            break;
        case "loop3":
            if ((status & 0xF0) == 0x80) mode = "loop";
            else mode = "loop3";
            break;
        default: mode = "loop";
    }
    StantonSCS3d.modeButton(channel, control, status, mode);
}

StantonSCS3d.Trig = function (channel, control, value, status) {
    StantonSCS3d.triggerS4 = 0xFF;
    var mode;
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];

    switch (currentMode) {
        case "trig":
            if ((status & 0xF0) == 0x80) mode = "trig2";
            else mode = "trig";
            break;
        case "trig2":
            if ((status & 0xF0) == 0x80) mode = "trig3";
            else mode = "trig2";
            break;
        case "trig3":
            if ((status & 0xF0) == 0x80) mode = "trig";
            else mode = "trig3";
            break;
        default: mode = "trig";
    }
    StantonSCS3d.modeButton(channel, control, status, mode);
}

StantonSCS3d.Vinyl = function (channel, control, value, status) {
    var mode;
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];

    switch (currentMode) {
        case "vinyl":
            if ((status & 0xF0) == 0x80) mode = "vinyl2";
            else mode = "vinyl";
            break;
        case "vinyl2":
            if ((status & 0xF0) == 0x80) mode = "vinyl3";
            else mode = "vinyl2";
            break;
        case "vinyl3":
            if ((status & 0xF0) == 0x80) mode = "vinyl";
            else mode = "vinyl3";
            break;
        default: mode = "vinyl";
    }
    StantonSCS3d.modeButton(channel, control, status, mode);
}

StantonSCS3d.DeckButton = function (channel, control, value, status) {
    if (StantonSCS3d.singleDeck) StantonSCS3d.modeButton(channel, control, status, "deck");
    else StantonSCS3d.DeckChangeP1(channel, control, value, status);
}

StantonSCS3d.deckChangeFlash = function (channel, value, targetSide) {
    var led = {"left":0x71,"right":0x72};
    var byte1 = 0x90 + channel;

    StantonSCS3d.state["flashes"]++;

    if (StantonSCS3d.state["flashes"] % 2 == 0) {
        StantonSCS3d.deckIndicator(byte1,true);  // Deck indicator on
        StantonSCS3d.circleLEDsColor(channel,0x01,targetSide);  // Light half-circle
    }
    else {
        StantonSCS3d.deckIndicator(byte1,false);  // Deck indicator off
        StantonSCS3d.circleLEDsColor(channel,0x00,targetSide);  // Extinguish half-circle
    }

    if (StantonSCS3d.state["flashes"]>=5) {
        engine.stopTimer(StantonSCS3d.timer[0]);
        StantonSCS3d.timer[0] = -1;

        // Finish the deck change
        StantonSCS3d.deckIndicator(byte1,true);  // Deck indicator on
        if (!StantonSCS3d.state["logoLit"] && StantonSCS3d.deck > 0 &&
            StantonSCS3d.deck <= engine.getValue("[App]", "num_decks")) {
            // Re-light the Stanton logo if we're within deck #1-4 and if it had
            //  been extinguished before
            midi.sendShortMsg(byte1,0x7A,0x01);
            StantonSCS3d.state["logoLit"]=true;
        }

        StantonSCS3d.connectDeckSignals(channel);    // Connect static signals
        StantonSCS3d.connectDeckSignals(channel,false,"common");    // Connect static common signals
        StantonSCS3d.DeckChangeP2(channel, value);  // Call part 2
    }
}

StantonSCS3d.deckIndicator = function (byte1, light) {
    var value = 0x01;
    if (!light) value = 0x00;
    switch (StantonSCS3d.deck) {
        case 1:
            midi.sendShortMsg(byte1,0x71,value);  // Deck A light
            break;
        case 2:
            midi.sendShortMsg(byte1,0x72,value);  // Deck B light
            break;
        case 3: // Both lights on
            midi.sendShortMsg(byte1,0x71,value);  // Deck A light
            midi.sendShortMsg(byte1,0x72,value);  // Deck B light
            break;
        case 4: // No lights on
            break;
        default:
            // Turn off the Stanton logo too as a sign that we don't have a way
            //  of telling you which deck you're controlling (it's higher than 4!)
            //  (We could use it and the A & B lights as a 3-bit counter for
            //  up to 8 decks, but that's a bit too nerdy for the DJ crowd.)
            midi.sendShortMsg(byte1,0x7A,0x00);
            StantonSCS3d.state["logoLit"]=false;
            break;
    }
}

StantonSCS3d.DeckChangeP1 = function (channel, control, value, status) {
    var byte1 = 0x90 + channel;
    if ((status & 0xF0) == 0x90) {  // If button down
        StantonSCS3d.modeButtonsColor(channel,0x02);  // Make all mode buttons blue
        midi.sendShortMsg(byte1,control,0x01); // Make button red
        StantonSCS3d.modifier["deck"]=1;   // Set button modifier flag
        StantonSCS3d.modifier["deckTime"] = new Date();  // Store the current time in milliseconds
        StantonSCS3d.state["deckPrev"] = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
        StantonSCS3d.connectSurfaceSignals(channel,true);   // Disconnect surface signals & turn off surface LEDs
        StantonSCS3d.connectDeckSignals(channel,true,"common"); // Disconnect common signals
        StantonSCS3d.B11LED(0); // B11 blue
        StantonSCS3d.B12LED(0); // B12 blue
        midi.sendShortMsg(byte1,0x3D,0x00);  // Pitch LED black
        midi.sendShortMsg(byte1,0x3E,0x00);
        // Switch to three-slider mode
        midi.sendSysexMsg(StantonSCS3d.sysex.concat([0x01, StantonSCS3d.surface["S3+S5"], 0xF7]),7);
        StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = "deck";
        StantonSCS3d.connectSurfaceSignals(channel);  // Connect new ones
        // Force the circle LEDs to light if applicable
        StantonSCS3d.lastLight[StantonSCS3d.deck]=-1;
        StantonSCS3d.circleLEDs(engine.getValue("[Channel"+StantonSCS3d.deck+"]","playposition"));
        return;
    }
    // If button up
    StantonSCS3d.modifier["deck"]=0;   // Clear button modifier flag
    StantonSCS3d.connectSurfaceSignals(channel,true);   // Disconnect surface signals & turn off surface LEDs
    StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = StantonSCS3d.state["deckPrev"];
    // To avoid getting stuck at a weird speed
    engine.scratchDisable(StantonSCS3d.deck);
    var newMode;
    // In default multiple-deck mode,
    //      If the button's been held down for longer than the specified time, stay on the current deck.
    //      If the button was just tapped, change control to another virtual deck.
    if ((new Date() - StantonSCS3d.modifier["deckTime"])>=StantonSCS3d.deckChangeWait) {
        midi.sendShortMsg(byte1,StantonSCS3d.buttons["deck"],0x01+StantonSCS3d.deck); // Return to appropriate color
        StantonSCS3d.connectSurfaceSignals(channel);  // Connect new ones
        StantonSCS3d.connectDeckSignals(channel,false,"common");    // Connect common signals again
    }
    else {
        StantonSCS3d.stopTimers();  // Stop any flashing light timers
        StantonSCS3d.connectDeckSignals(channel,true);    // Disconnect static signals
        StantonSCS3d.connectDeckSignals(channel,true,"common");    // Disconnect common signals
        StantonSCS3d.softButtonsColor(channel,0x00);  // Darken the soft buttons
        if (StantonSCS3d.globalMode) newMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
        if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"].substring(0,4) == "trig" ||
            StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"].substring(0,4) == "loop")
                for (i=0x48; i<=0x5c; i++) midi.sendShortMsg(byte1,i,0x40); // Set surface LEDs to black
        StantonSCS3d.deck++;
        if (StantonSCS3d.deck > engine.getValue("[App]", "num_decks")) StantonSCS3d.deck = 1;   // Wrap around
        if (StantonSCS3d.debug) print("StantonSCS3d: Switching to deck "+StantonSCS3d.deck);
        midi.sendShortMsg(byte1,0x71,0x00); // Deck A light off
        midi.sendShortMsg(byte1,0x72,0x00);  // Deck B light off
        if (StantonSCS3d.deck % 2 == 0) { // If the new deck # is even
            midi.sendShortMsg(byte1,StantonSCS3d.buttons["deck"],0x03); // Deck button purple
            // Make flashy lights to signal a deck change
            if (!StantonSCS3d.fastDeckChange) {
                // If in the middle of flashing lights from a previous change, abort that one
                if (StantonSCS3d.timer[0] != -1) engine.stopTimer(StantonSCS3d.timer[0]);
                StantonSCS3d.state["flashes"] = 0;  // initialize number of flashes
                StantonSCS3d.timer[0] = engine.beginTimer(60, () => { StantonSCS3d.deckChangeFlash(channel, value, "right"); });
                return;
            }
        }
        else {
            midi.sendShortMsg(byte1,StantonSCS3d.buttons["deck"],0x02); // Deck button blue
            midi.sendShortMsg(byte1,0x72,0x00);  // Deck B light off
            if (!StantonSCS3d.fastDeckChange) {
                if (StantonSCS3d.timer[0] != -1) engine.stopTimer(StantonSCS3d.timer[0]);
                StantonSCS3d.state["flashes"] = 0;  // initialize number of flashes
                StantonSCS3d.timer[0] = engine.beginTimer(60, () => { StantonSCS3d.deckChangeFlash(channel, value, "left"); });
                return;
            }
        }
        // These are done in deckChangeFlash() when it's finished.
        //  But they're here too if fastDeckChange is enabled
        StantonSCS3d.deckIndicator(byte1,true);
        StantonSCS3d.connectDeckSignals(channel);    // Connect static signals
        StantonSCS3d.connectDeckSignals(channel,false,"common");    // Connect static common signals
    }
    StantonSCS3d.DeckChangeP2(channel, value); // Go to part 2.
}

StantonSCS3d.DeckChangeP2 = function (channel, value) {
    if (!StantonSCS3d.globalMode) newMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = "none"; // Forces a mode change when a function is called
    StantonSCS3d.modifier["time"] = 0.0;    // Reset the mode-modifier time
    StantonSCS3d.state["changedDeck"]= true;    // Mark that we just changed decks so the surface LEDs can update correctly for TRIG & LOOP
    switch (newMode) {    // Call the appropriate mode change function to set the control surface & connect signals on the now-current deck
        // Effects
        case "fx2":
            StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = "fx"; // force correct change
        case "fx3":
            if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] == "none")
                StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = "fx2";    // force correct change
        case "fx":
            StantonSCS3d.FX(channel, StantonSCS3d.buttons["fx"], value, 0x80 + channel);
            break;

        // EQ
        case "eq":
            StantonSCS3d.EQ(channel, StantonSCS3d.buttons["eq"], value, 0x80 + channel);
            break;

        // Loop
        case "loop2":
            StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = "loop"; // force correct change
        case "loop3":
            if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] == "none")
                StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = "loop2";    // force correct change
        case "loop":
            StantonSCS3d.Loop(channel, StantonSCS3d.buttons["loop"], value, 0x80 + channel);
            break;

        // Trig
        case "trig2":
            StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = "trig"; // force correct change
        case "trig3":
            if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] == "none")
                StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = "trig2";    // force correct change
        case "trig":
            StantonSCS3d.Trig(channel, StantonSCS3d.buttons["trig"], value, 0x80 + channel);
            break;

        // Vinyl
        case "vinyl2":
            StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = "vinyl";    // force correct change
        case "vinyl3":
            if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] == "none")
                StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] = "vinyl2";   // force correct change
        case "vinyl":
            StantonSCS3d.Vinyl(channel, StantonSCS3d.buttons["vinyl"], value, 0x80 + channel);
            break;
    }
}   // End Deck Change function

// ----------   Sliders  ----------

StantonSCS3d.S4relative = function (channel, control, value) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    var newValue=(value-64);
    switch (currentMode) {
        case "vinyl":
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","jog",newValue);
            break;
        case "vinyl2":
            engine.scratchTick(StantonSCS3d.deck,newValue);
            break;
    }
}

StantonSCS3d.S3absolute = function (channel, control, value) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    var modeIndex = currentMode.charAt(currentMode.length-1);
    if (modeIndex != "2" && modeIndex != "3") modeIndex = "1";
    switch (currentMode) {
        case "fx":
        case "fx2":
        case "fx3":
            // Ignore if mode button is held down (wanting to reset)
            if (StantonSCS3d.modifier[currentMode]==1) return;
            engine.setParameter("[EffectRack1_EffectUnit"+modeIndex+"_Effect1]",
                                "parameter1",script.absoluteLin(value,0,1));
            break;
        case "eq":
            // Ignore if mode button is held down (wanting to reset)
            if (StantonSCS3d.modifier[currentMode]==1) return;
            engine.setParameter("[EqualizerRack1_[Channel"+StantonSCS3d.deck+"]_Effect1]",
                                "parameter1",script.absoluteLin(value,0,1));
            break;
        case "deck": engine.setValue("[Master]","headMix",(value-64)/63); break;
    }
}

StantonSCS3d.S4absolute = function (channel, control, value) {
    // Adjust the cross-fader if in DECK mode
    if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"] == "deck") {
        engine.setValue("[Master]","crossfader",(value-64)/63);
        return;
    }
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    var modeIndex = currentMode.charAt(currentMode.length-1);
    if (modeIndex != "2" && modeIndex != "3") modeIndex = "1";
    switch (currentMode) {
        case "fx":
        case "fx2":
        case "fx3":
            // Ignore if mode button is held down (wanting to reset)
            if (StantonSCS3d.modifier[currentMode]==1) return;
            engine.setParameter("[EffectRack1_EffectUnit"+modeIndex+"_Effect1]",
                                "parameter2",script.absoluteLin(value,0,1));
            break;
        case "eq":
            if (StantonSCS3d.modifier[currentMode]==1) return;  // Ignore if mode button is held down (wanting to reset)
            engine.setParameter("[EqualizerRack1_[Channel"+StantonSCS3d.deck+"]_Effect1]","parameter2",script.absoluteLin(value,0,1));
            break;
        case "vinyl":
        case "vinyl2":
            // Set slider lights
            var add = (value/15)|0;
            var byte1 = 0xB0 + channel;
            midi.sendShortMsg(byte1,0x01,add); //S4 LEDs
            if (!StantonSCS3d.VUMeters || StantonSCS3d.deck % 2 == 0) midi.sendShortMsg(byte1,0x0C,add); //S3 LEDs
            if (!StantonSCS3d.VUMeters || StantonSCS3d.deck % 2 != 0) midi.sendShortMsg(byte1,0x0E,add); //S5 LEDs
            break;
        case "loop":
            var button = 0x5C-2*Math.floor(value/32);
            if (button==0x5A) button=0x58;  // Join middle two buttons
            if (StantonSCS3d.triggerS4==button) return; // prevent retriggering before releasing the button
            StantonSCS3d.triggerS4 = button;
            switch (button) {
                case 0x56: engine.setValue("[Channel"+StantonSCS3d.deck+"]","loop_in",1); break;
                case 0x58: engine.setValue("[Channel"+StantonSCS3d.deck+"]","reloop_exit",1); break;
                case 0x5C: engine.setValue("[Channel"+StantonSCS3d.deck+"]","loop_out",1); break;
            }
            break;
        case "loop2":
        case "loop3":
            var button = 0x5C-2*Math.floor(value/32);
            if (StantonSCS3d.triggerS4==button) return; // prevent retriggering before releasing the button
            StantonSCS3d.S4buttonLights(channel,false,StantonSCS3d.triggerS4);  // Clear the previous lights
            StantonSCS3d.triggerS4 = button;
            StantonSCS3d.S4buttonLights(channel,true,button);

            var index = currentMode.charAt(currentMode.length-1);
            if (index != "2" && index != "3") index = "1";

//             if (StantonSCS3d.modifier[currentMode]==1) {
//                 StantonSCS3d.pitchPoints[index][button] = -0.1;
//                 break;
//             }
            if (StantonSCS3d.pitchPoints[index][button] == -0.1)
                StantonSCS3d.pitchPoints[index][button] = engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate");
            else {
                // Need 100% range for values to be correct
                engine.setValue("[Channel"+StantonSCS3d.deck+"]","rateRange",1.0);
                engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate",StantonSCS3d.pitchPoints[index][button]);
            }
            break;

        case "trig":
        case "trig2":
        case "trig3":
            // Free Bonus! Four extra buttons!
            var button = 0x5C-2*Math.floor(value/32);
            if (StantonSCS3d.triggerS4==button) return; // prevent retriggering before releasing the button

            // Deactivate any previous hot cue in the same slider touch
            if (StantonSCS3d.state["S4hotcue"] != -1) {
                engine.setValue("[Channel"+StantonSCS3d.deck+"]","hotcue_"+StantonSCS3d.state["S4hotcue"]+"_activate",0);
                StantonSCS3d.state["S4hotcue"]=-1;
            }

            StantonSCS3d.triggerS4 = button;

            var index = currentMode.charAt(currentMode.length-1);
            if (index != "2" && index != "3") index = "1";

            if (StantonSCS3d.hotCues[index][button] == -1) return;

            if (StantonSCS3d.modifier[currentMode]==1) {
                engine.setValue("[Channel"+StantonSCS3d.deck+"]","hotcue_"+StantonSCS3d.hotCues[index][button]+"_clear",1);
                break;
            }
            // If hotcue X is set, seeks the player to hotcue X's position.
            // If hotcue X is not set, sets hotcue X to the current play position.
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","hotcue_"+StantonSCS3d.hotCues[index][button]+"_activate",1);
            StantonSCS3d.state["S4hotcue"]=StantonSCS3d.hotCues[index][button];    // so it can be deactivated later
            break;
    }
}

StantonSCS3d.S5absolute = function (channel, control, value) {
    if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]=="deck") return;   // Ignore if in DECK mode
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if (StantonSCS3d.modifier[currentMode]==1) return;  // Ignore if mode button is held down (wanting to reset)
    var modeIndex = currentMode.charAt(currentMode.length-1);
    if (modeIndex != "2" && modeIndex != "3") modeIndex = "1";
    switch (currentMode) {
        case "fx":
        case "fx2":
        case "fx3":
            // Ignore if mode button is held down (wanting to reset)
            if (StantonSCS3d.modifier[currentMode]==1) return;
            engine.setParameter("[EffectRack1_EffectUnit"+modeIndex+"_Effect1]",
                                "parameter3",script.absoluteLin(value,0,1));
            break;
        case "eq":
            engine.setParameter("[EqualizerRack1_[Channel"+StantonSCS3d.deck+"]_Effect1]","parameter3",script.absoluteLin(value,0,1));
            break;
    }
}

StantonSCS3d.S5relative = function (channel, control, value) {
    // Adjust the headphone volume if in DECK mode
    if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]=="deck") {
        var newValue = engine.getValue("[Master]","headVolume")+(value-64)/128;
        if (newValue<0.0) newValue=0.0;
        if (newValue>5.0) newValue=5.0;
        engine.setValue("[Master]","headVolume",newValue);
        return;
    }
}

StantonSCS3d.C1touch = function (channel, control, value, status) {
    var byte1 = 0xB0 + channel;
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if ((status & 0xF0) == 0x90) {    // If button down
        switch (currentMode) {
            case "vinyl2":
                engine.scratchEnable(StantonSCS3d.deck, 128, 45, 1.0/16, (1.0/16)/32);
                    // Recall the cue point if in "scratch & cue" mode only when playing
                    if (StantonSCS3d.scratchncue[StantonSCS3d.deck]
                        && engine.getValue("[Channel"+StantonSCS3d.deck+"]","play")==1) {
                        engine.setValue("[Channel"+StantonSCS3d.deck+"]","cue_goto",1);
                        engine.setValue("[Channel"+StantonSCS3d.deck+"]","cue_goto",0);
                    }
                break;
        }
    }
    else {  // If button up
        switch (currentMode) {
            case "vinyl": break;    // Leave C1 (platter) lights as they are
            case "vinyl2":
                engine.scratchDisable(StantonSCS3d.deck);
                break;
            default: midi.sendShortMsg(byte1,0x62,0x00); // Turn off C1 lights
                break;
        }
    }
}

StantonSCS3d.S3touch = function (channel, control, value, status) {
    // Reset the value to center if the slider is touched while the mode button is held down
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if (StantonSCS3d.modifier[currentMode]==1) {
        var modeIndex = currentMode.charAt(currentMode.length-1);
        if (modeIndex != "2" && modeIndex != "3") modeIndex = "1";
        switch (currentMode) {
            case "fx":
            case "fx2":
            case "fx3":
                engine.reset("[EffectRack1_EffectUnit"+modeIndex+"_Effect1]",
                             "parameter1");
                break;
            case "eq":
                engine.reset("[EqualizerRack1_[Channel"+StantonSCS3d.deck+"]_Effect1]","parameter1");
                break;
            case "deck":
                // Reset only if in single-deck mode
                if (StantonSCS3d.singleDeck) engine.reset("[Master]","headMix");
                break;
        }
    }
    if ((status & 0xF0) == 0x90) {    // If button down
        if (currentMode == "loop") engine.setValue("[Channel"+StantonSCS3d.deck+"]","loop_in",1);
        return;
    }
    // If button up
    if (currentMode == "loop") engine.setValue("[Channel"+StantonSCS3d.deck+"]","loop_in",0);
}

StantonSCS3d.S4touch = function (channel, control, value, status) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    // If the current mode button is held down, reset the control to center
    if (StantonSCS3d.modifier[currentMode]==1) {
        var modeIndex = currentMode.charAt(currentMode.length-1);
        if (modeIndex != "2" && modeIndex != "3") modeIndex = "1";
        switch (currentMode) {
            case "fx":
            case "fx2":
            case "fx3":
                engine.reset("[EffectRack1_EffectUnit"+modeIndex+"_Effect1]",
                             "parameter2");
                break;
            case "eq":
                engine.reset("[EqualizerRack1_[Channel"+StantonSCS3d.deck+"]_Effect1]","parameter2");
                break;
            case "deck":
                // Reset cross-fader to center if in single-deck mode
                if (StantonSCS3d.singleDeck) engine.reset("[Master]","crossfader");
                break;
        }
    }
    if ((status & 0xF0) == 0x90) {    // If button down
        switch (currentMode) {
            case "vinyl2":
                engine.scratchEnable(StantonSCS3d.deck, 512, 33+1/3, 1.0/8, (1.0/8)/32);
                    // Recall the cue point if in "scratch & cue" mode only when playing
                    if (StantonSCS3d.scratchncue[StantonSCS3d.deck]
                        && engine.getValue("[Channel"+StantonSCS3d.deck+"]","play")==1) {
                        engine.setValue("[Channel"+StantonSCS3d.deck+"]","cue_goto",1);
                        engine.setValue("[Channel"+StantonSCS3d.deck+"]","cue_goto",0);
                    }
                break;
            case "vinyl3":  // Load the song
                // If the deck is playing and the cross-fader is not completely toward the other deck...
                // TODO: Check L/C/R CF assignment or just ask Mixxx itself if this deck is playing to the master
                if (engine.getValue("[Channel"+StantonSCS3d.deck+"]","play")==1 &&
                    ((StantonSCS3d.deck % 2 != 0 && engine.getValue("[Master]","crossfader")<1.0) ||
                    (StantonSCS3d.deck % 2 == 0 && engine.getValue("[Master]","crossfader")>-1.0))) {
                    // ...light just the red button LEDs to show acknowledgement of the press but don't load
                    StantonSCS3d.sliderButtonLight(channel,"S4",true,true);
                    print ("StantonSCS3d: Not loading into deck "+StantonSCS3d.deck+" because it's playing to the Master output.");
                }
                else {
                    StantonSCS3d.sliderButtonLight(channel,"S4",true);
//                     engine.setValue("[Playlist]","LoadSelectedIntoFirstStopped",1);
                    engine.setValue("[Channel"+StantonSCS3d.deck+"]","LoadSelectedTrack",1);
                }
                break;
        }
        return;
    }
    // If button up
    switch (currentMode) {
        case "vinyl2":
            engine.scratchDisable(StantonSCS3d.deck);
        case "vinyl":
            var byte1a = 0xB0 + channel;
            midi.sendShortMsg(byte1a,0x01,0x00); //S4 LEDs off
            if (!StantonSCS3d.VUMeters || StantonSCS3d.deck % 2 == 0) midi.sendShortMsg(byte1a,0x0C,0x00); //S3 LEDs off
            if (!StantonSCS3d.VUMeters || StantonSCS3d.deck % 2 != 0) midi.sendShortMsg(byte1a,0x0E,0x00); //S5 LEDs off
            break;
        case "vinyl3":
//             engine.setValue("[Playlist]","LoadSelectedIntoFirstStopped",1);
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","LoadSelectedTrack",0);
            StantonSCS3d.sliderButtonLight(channel,"S4",false);
            if (StantonSCS3d.jogOnLoad) {   // Auto-change to vinyl jog mode on track load
                // ...only if we actually loaded a track.
                // TODO: Check L/C/R CF assignment or just ask Mixxx itself if this deck is playing to the master
                if (engine.getValue("[Channel"+StantonSCS3d.deck+"]","play")==1 &&
                    ((StantonSCS3d.deck % 2 != 0 && engine.getValue("[Master]","crossfader")<1.0) ||
                    (StantonSCS3d.deck % 2 == 0 && engine.getValue("[Master]","crossfader")>-1.0))) {
                    return;
                }

                StantonSCS3d.modifier["time"] = 0.0;
                StantonSCS3d.Vinyl(channel, StantonSCS3d.buttons["vinyl"], value, 0x80 + channel);
            }
            break;
        case "loop":
            switch (StantonSCS3d.triggerS4) {
                case 0x56: engine.setValue("[Channel"+StantonSCS3d.deck+"]","loop_in",0); break;
                case 0x58: engine.setValue("[Channel"+StantonSCS3d.deck+"]","reloop_exit",0); break;
                case 0x5C: engine.setValue("[Channel"+StantonSCS3d.deck+"]","loop_out",0); break;
            }
            StantonSCS3d.triggerS4 = 0xFF;
            break;
        case "loop2":
        case "loop3":
            StantonSCS3d.S4buttonLights(channel,false,StantonSCS3d.triggerS4);
            StantonSCS3d.triggerS4 = 0xFF;
            break;
        case "trig":
        case "trig2":
        case "trig3":
            if (StantonSCS3d.state["S4hotcue"] != -1) {
                engine.setValue("[Channel"+StantonSCS3d.deck+"]","hotcue_"+StantonSCS3d.state["S4hotcue"]+"_activate",0);
                StantonSCS3d.state["S4hotcue"]=-1;
            }
            //StantonSCS3d.S4buttonLights(channel,false,StantonSCS3d.triggerS4);
            StantonSCS3d.triggerS4 = 0xFF;
            break;
    }
}

StantonSCS3d.S5touch = function (channel, control, value, status) {
    // Reset the value to center if the slider is touched while the mode button is held down
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if (StantonSCS3d.modifier[currentMode]==1){
        var modeIndex = currentMode.charAt(currentMode.length-1);
        if (modeIndex != "2" && modeIndex != "3") modeIndex = "1";
        switch (currentMode) {
            case "fx":
            case "fx2":
            case "fx3":
                engine.reset("[EffectRack1_EffectUnit"+modeIndex+"_Effect1]",
                             "parameter3");
                break;
            case "eq":
                engine.reset("[EqualizerRack1_[Channel"+StantonSCS3d.deck+"]_Effect1]","parameter3");
                break;
            case "deck":
                // Reset to center only if in single-deck mode
                if (StantonSCS3d.singleDeck) engine.reset("[Master]","headVolume");
                break;
        }
    }
    if ((status & 0xF0) == 0x90) {    // If button down
        if (currentMode == "loop") engine.setValue("[Channel"+StantonSCS3d.deck+"]","loop_out",1);
        return;
    }
    // If button up
    if (currentMode == "loop") engine.setValue("[Channel"+StantonSCS3d.deck+"]","loop_out",0);
}

StantonSCS3d.sliderButtonLight = function (channel, slider, light, noring) {     // Turn on/off button lights
    var byte1 = 0x90 + channel;
    var color=0x00; // Off
    if (light) color=0x01;  // On
    switch (slider) {
        case "S3":
            if (!noring) {
                midi.sendShortMsg(byte1,0x62,color);
                midi.sendShortMsg(byte1,0x63,color);
                midi.sendShortMsg(byte1,0x5f,color);
                midi.sendShortMsg(byte1,0x5e,color);
            }
                midi.sendShortMsg(byte1,0x48,color);
                midi.sendShortMsg(byte1,0x4e,color);
            break;
        case "S4":
            if (!noring) {
                midi.sendShortMsg(byte1,0x64,color);
                midi.sendShortMsg(byte1,0x65,color);
                midi.sendShortMsg(byte1,0x5d,color);
                midi.sendShortMsg(byte1,0x6c,color);
            }
                midi.sendShortMsg(byte1,0x56,color);
                midi.sendShortMsg(byte1,0x5c,color);
            break;
        case "S5":
            if (!noring) {
                midi.sendShortMsg(byte1,0x66,color);
                midi.sendShortMsg(byte1,0x67,color);
                midi.sendShortMsg(byte1,0x6b,color);
                midi.sendShortMsg(byte1,0x6a,color);
            }
            midi.sendShortMsg(byte1,0x4f,color);
            midi.sendShortMsg(byte1,0x55,color);
            break;
    }
}

StantonSCS3d.S4buttonLights = function (channel, light, button) {     // Turn on/off button lights for multiple buttons
    if (button == 0xFF) return;

    var buttonLight;
    var byte1 = 0x90 + channel;
    var color=0x00; // Off
    if (light) color=0x01;  // On

    // Turn on/off button LED
    if (StantonSCS3d.markHotCues == "red") buttonLight = StantonSCS3d.buttonLEDs[button];
    else buttonLight = button;
    midi.sendShortMsg(byte1,buttonLight,color);

    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    var index = currentMode.charAt(currentMode.length-1);
    if (index != "2" && index != "3") index = "1";
}

StantonSCS3d.C1relative = function (channel, control, value, status) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    switch (currentMode) {
        case "vinyl":
            var newValue=(value-64);
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","jog",newValue);
            break;
        case "vinyl2":
            var newValue=(value-64);
            engine.scratchTick(StantonSCS3d.deck,newValue);
            break;
        case "vinyl3":
            if ((value-64)>0) {
                engine.setValue("[Playlist]","SelectNextTrack",1);
            }
            else {
                engine.setValue("[Playlist]","SelectPrevTrack",1);
            }
            break;
    }
}

StantonSCS3d.C1absolute = function (channel, control, value, status) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    switch (currentMode) {
        case "vinyl":
        case "vinyl2":  // leave lights alone...circleLEDs handles them for vinyl modes
            break;
        default:
            // Light the LEDs
            var byte1 = 0xB0 + channel;
            var light = Math.floor(value/8)+1;
            midi.sendShortMsg(byte1,0x62,light);
            break;
    }
}

// ----------   Surface buttons  ----------

StantonSCS3d.SurfaceButton = function (channel, control, value, status) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    var byte1 = 0x90 + channel;

    var index = currentMode.charAt(currentMode.length-1);
    if (index != "2" && index != "3") index = "1";

    //var marker;
    //if (StantonSCS3d.markHotCues == "red") marker = control;
    //else marker = StantonSCS3d.buttonLEDs[control];

    var buttonLight;
    if (StantonSCS3d.markHotCues == "red") buttonLight = StantonSCS3d.buttonLEDs[control];
    else buttonLight = control;

    if ((status & 0xF0) != 0x80) {    // If button down
        switch (currentMode) {
            case "loop":
                // Beat loops
                if (StantonSCS3d.loopButtons[control] == -1) return;

                // Toggle the selected beat loop
                engine.setValue("[Channel"+StantonSCS3d.deck+"]","beatloop_"+StantonSCS3d.loopButtons[control]+"_toggle",1);
                break;
                break;
            case "loop2":
            case "loop3":
                midi.sendShortMsg(byte1,buttonLight,0x01); // Turn on button light
                // Multiple pitch points
//                 if (StantonSCS3d.modifier[currentMode]==1) {    // Delete a pitch point if the mode button is held
//                     StantonSCS3d.pitchPoints[index][button] = -0.1;
//                     break;
//                 }
                if (StantonSCS3d.pitchPoints[index][control] == -0.1)
                    StantonSCS3d.pitchPoints[index][control] = engine.getValue("[Channel"+StantonSCS3d.deck+"]","rate");
                else {
                    // Need 100% range for values to be correct
                    engine.setValue("[Channel"+StantonSCS3d.deck+"]","rateRange",1.0);
                    engine.setValue("[Channel"+StantonSCS3d.deck+"]","rate",StantonSCS3d.pitchPoints[index][control]);
                }
                break;
            case "trig":
            case "trig2":
            case "trig3":
                // Multiple cue points

                if (StantonSCS3d.hotCues[index][control] == -1) return;

                if (StantonSCS3d.modifier[currentMode]==1) { // Delete a cue point
                    engine.setValue("[Channel"+StantonSCS3d.deck+"]","hotcue_"+StantonSCS3d.hotCues[index][control]+"_clear",1);
                    engine.setValue("[Channel"+StantonSCS3d.deck+"]","hotcue_"+StantonSCS3d.hotCues[index][control]+"_clear",0);
                    break;
                }
                // If hotcue X is set, seeks the player to hotcue X's position.
                // If hotcue X is not set, sets hotcue X to the current play position.
                engine.setValue("[Channel"+StantonSCS3d.deck+"]","hotcue_"+StantonSCS3d.hotCues[index][control]+"_activate",1);
                break;
        }
        return;
    }

    switch (currentMode) {
        case "loop":
            // Beat loops

            if (StantonSCS3d.loopButtons[control] == -1) return;

            // Toggle the selected beat loop
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","beatloop_"+StantonSCS3d.loopButtons[control]+"_toggle",0);
            break;
        case "trig":
        case "trig2":
        case "trig3":
            engine.setValue("[Channel"+StantonSCS3d.deck+"]","hotcue_"+StantonSCS3d.hotCues[index][control]+"_activate",0);
            break;
        default:
            midi.sendShortMsg(byte1,buttonLight,0x00); // Turn off activated button LED
            break;
    }
}


// ----------   LED slot functions  ----------

StantonSCS3d.buttonLED = function (value, note, on, off) {
    var byte1 = 0x90 + StantonSCS3d.channel;
    if (value>0) midi.sendShortMsg(byte1,note,on);
    else midi.sendShortMsg(byte1,note,off);
}

// ---- Transport buttons ----

StantonSCS3d.playLED = function (value) {
    StantonSCS3d.buttonLED(value, 0x6D, 0x01, 0x00);
}

StantonSCS3d.cueLED = function (value) {
    StantonSCS3d.buttonLED(value, 0x6E, 0x01, 0x00);
}

StantonSCS3d.syncLED = function (value) {
    StantonSCS3d.buttonLED(value, 0x6F, 0x01, 0x00);
}

StantonSCS3d.tapLED = function (value) {
    StantonSCS3d.buttonLED(value, 0x70, 0x01, 0x00);
}

// ---- Soft buttons ----

StantonSCS3d.B11LED = function (value) {
    StantonSCS3d.buttonLED(value, 0x2C, 0x01, 0x02);
}

StantonSCS3d.B12LED = function (value) {
    StantonSCS3d.buttonLED(value, 0x2E, 0x01, 0x02);
}

StantonSCS3d.B13LED = function (value) {
    StantonSCS3d.buttonLED(value, 0x30, 0x01, 0x02);
}

StantonSCS3d.B14LED = function (value) {
    StantonSCS3d.buttonLED(value, 0x32, 0x01, 0x02);
}

// ---- Hot cues ----

StantonSCS3d.BsLED = function (value, button, activate) {
    var light = button;

    if (activate) {
        // Button-pressed (hot cue activated) LED
        if (StantonSCS3d.markHotCues == "red") light = StantonSCS3d.buttonLEDs[button];
    }
    else {
        // Hot cue set LED
        if (StantonSCS3d.markHotCues != "red") light = StantonSCS3d.buttonLEDs[button];
    }

    var byte1 = 0x90 + StantonSCS3d.channel;
    if (value != 0) midi.sendShortMsg(byte1,light,0x01);
    else midi.sendShortMsg(byte1,light,0x00);
}

// Hotcue-set LEDs
StantonSCS3d.BsALED = function (value) { StantonSCS3d.BsLED(value,0x48,false); }
StantonSCS3d.BsBLED = function (value) { StantonSCS3d.BsLED(value,0x4A,false); }
StantonSCS3d.BsCLED = function (value) { StantonSCS3d.BsLED(value,0x4C,false); }
StantonSCS3d.BsDLED = function (value) { StantonSCS3d.BsLED(value,0x4E,false); }
StantonSCS3d.BsELED = function (value) { StantonSCS3d.BsLED(value,0x4F,false); }
StantonSCS3d.BsFLED = function (value) { StantonSCS3d.BsLED(value,0x51,false); }
StantonSCS3d.BsGLED = function (value) { StantonSCS3d.BsLED(value,0x53,false); }
StantonSCS3d.BsHLED = function (value) { StantonSCS3d.BsLED(value,0x55,false); }
StantonSCS3d.BsILED = function (value) { StantonSCS3d.BsLED(value,0x56,false); }
StantonSCS3d.BsJLED = function (value) { StantonSCS3d.BsLED(value,0x58,false); }
StantonSCS3d.BsKLED = function (value) { StantonSCS3d.BsLED(value,0x5A,false); }
StantonSCS3d.BsLLED = function (value) { StantonSCS3d.BsLED(value,0x5C,false); }

// Button-activated LEDs
StantonSCS3d.BsAaLED = function (value) { StantonSCS3d.BsLED(value,0x48,true); }
StantonSCS3d.BsBaLED = function (value) { StantonSCS3d.BsLED(value,0x4A,true); }
StantonSCS3d.BsCaLED = function (value) { StantonSCS3d.BsLED(value,0x4C,true); }
StantonSCS3d.BsDaLED = function (value) { StantonSCS3d.BsLED(value,0x4E,true); }
StantonSCS3d.BsEaLED = function (value) { StantonSCS3d.BsLED(value,0x4F,true); }
StantonSCS3d.BsFaLED = function (value) { StantonSCS3d.BsLED(value,0x51,true); }
StantonSCS3d.BsGaLED = function (value) { StantonSCS3d.BsLED(value,0x53,true); }
StantonSCS3d.BsHaLED = function (value) { StantonSCS3d.BsLED(value,0x55,true); }
StantonSCS3d.BsIaLED = function (value) { StantonSCS3d.BsLED(value,0x56,true); }
StantonSCS3d.BsJaLED = function (value) { StantonSCS3d.BsLED(value,0x58,true); }
StantonSCS3d.BsKaLED = function (value) { StantonSCS3d.BsLED(value,0x5A,true); }
StantonSCS3d.BsLaLED = function (value) { StantonSCS3d.BsLED(value,0x5C,true); }

// ---- Other slots ----

StantonSCS3d.LoopInLEDs = function (value) {
    var byte1 = 0x90 + StantonSCS3d.channel;
    var color=0x00; // Off
    if (value>0) color=0x01;  // On
    midi.sendShortMsg(byte1,0x56,color);
    midi.sendShortMsg(byte1,0x64,color);
    midi.sendShortMsg(byte1,0x65,color);
}

StantonSCS3d.LoopOutLEDs = function (value) {
    var byte1 = 0x90 + StantonSCS3d.channel;
    var color=0x00; // Off
    if (value>0) color=0x01;  // On
    midi.sendShortMsg(byte1,0x5c,color);
    midi.sendShortMsg(byte1,0x5d,color);
    midi.sendShortMsg(byte1,0x6c,color);
}

StantonSCS3d.ReLoopLEDs = function (value) {
    var byte1 = 0x90 + StantonSCS3d.channel;
    var color=0x00; // Off
    if (value>0) color=0x01;  // On
    midi.sendShortMsg(byte1,0x58,color);
    midi.sendShortMsg(byte1,0x59,color);
    midi.sendShortMsg(byte1,0x5A,color);
}

StantonSCS3d.ActiveLoop = function (value) {
    var timerName = "loop"+StantonSCS3d.deck;
    if (value>0) {
        if (StantonSCS3d.timer[timerName] == -1) {
            // Start timer
            StantonSCS3d.timer[timerName] = engine.beginTimer(500, () => { StantonSCS3d.activeLoopLEDs(StantonSCS3d.deck,false); });
        }
    }
    else {
        if (StantonSCS3d.timer[timerName] != -1) {
            // Stop timer
            engine.stopTimer(StantonSCS3d.timer[timerName]);
            StantonSCS3d.activeLoopLEDs(StantonSCS3d.deck,true);
            StantonSCS3d.timer[timerName] = -1;
        }
    }
}

StantonSCS3d.activeLoopLEDs = function (deck,forceoff) {
    var byte1 = 0x90 + StantonSCS3d.channel;
    var color=0x00; // Off
    if (forceoff) {
        StantonSCS3d.state["loopFlash"]=false;
    }
    else {
        if (!StantonSCS3d.state["loopFlash"]) {
            StantonSCS3d.state["loopFlash"]=true;
            color=0x01;  // On
        }
        else StantonSCS3d.state["loopFlash"]=false;
    }
    midi.sendShortMsg(byte1,0x63,color);
    midi.sendShortMsg(byte1,0x66,color);
    midi.sendShortMsg(byte1,0x5e,color);
    midi.sendShortMsg(byte1,0x6b,color);
}

StantonSCS3d.BoostCut = function (numberLights, value, low, mid, high, lowMidSteps, midHighSteps) {
    var LEDs = 0;
    var lowMidInterval = (mid-low)/(lowMidSteps*2);     // Half the actual interval so the LEDs light in the middle of the interval
    var midHighInterval = (high-mid)/(midHighSteps*2);  // Half the actual interval so the LEDs light in the middle of the interval
    value=value.toFixed(4);
    if (value>low) LEDs++;
    if (value>low+lowMidInterval) LEDs++;
    if (value>low+lowMidInterval*3) LEDs++;
    if (numberLights==9 && value>low+lowMidInterval*5) LEDs++;
    if (value>mid+midHighInterval) LEDs++;
    if (value>mid+midHighInterval*3) LEDs++;
    if (numberLights==9 && value>mid+midHighInterval*5) LEDs++;
    if (value>=high) LEDs++;
    return LEDs;
}

StantonSCS3d.Peak7 = function (value, low, high) {
    var LEDs = 0;
    var halfInterval = ((high-low)/6)/2;
    value=value.toFixed(4);
    if (value>low) LEDs++;
    if (value>low+halfInterval) LEDs++;
    if (value>low+halfInterval*3) LEDs++;
    if (value>low+halfInterval*5) LEDs++;
    if (value>low+halfInterval*7) LEDs++;
    if (value>low+halfInterval*9) LEDs++;
    if (value>=high) LEDs++;
    return LEDs;
}

StantonSCS3d.headMixLEDs = function (value) {
    var add = StantonSCS3d.BoostCut(7, value, -1.0, 0.0, 1.0, 3, 3);
    var byte1 = 0xB0 + StantonSCS3d.channel;
    midi.sendShortMsg(byte1,0x0C,0x15+add);
}

StantonSCS3d.headVolLEDs = function (value) {
    var add = StantonSCS3d.BoostCut(7, value, 0, 1, 5, 3, 3);
    var byte1 = 0xB0 + StantonSCS3d.channel;
    midi.sendShortMsg(byte1,0x0E,0x15+add);
}

StantonSCS3d.crossFaderLEDs = function (value) {
    var add = StantonSCS3d.BoostCut(7, value, -1.0, 0.0, 1.0, 3, 3);
    var byte1 = 0xB0 + StantonSCS3d.channel;
    midi.sendShortMsg(byte1,0x01,0x15+add);
}

StantonSCS3d.EQLEDs = function (value,control,item) {
    var group = "[EqualizerRack1_[Channel"+StantonSCS3d.deck+"]_Effect1]";
//     var add = StantonSCS3d.BoostCut(7,value, 0, 1, 4, 3, 3);
    var add = StantonSCS3d.BoostCut(7,
                                    engine.getParameterForValue(group, item, value),
                                    0.0, 0.5, 1.0, 3, 3);
    var byte1 = 0xB0 + StantonSCS3d.channel;
    midi.sendShortMsg(byte1,control,0x15+add);
}

StantonSCS3d.EQLowLEDs = function (value) {
    StantonSCS3d.EQLEDs(value,0x0C,"parameter1");
}

StantonSCS3d.EQMidLEDs = function (value) {
    StantonSCS3d.EQLEDs(value,0x01,"parameter2");
}

StantonSCS3d.EQHighLEDs = function (value) {
    StantonSCS3d.EQLEDs(value,0x0E,"parameter3");
}

StantonSCS3d.FXSliderLEDs = function (value,control,item) {
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    var index = currentMode.charAt(currentMode.length-1);
    if (index != "2" && index != "3") index = "1";
    var group = "[EffectRack1_EffectUnit"+index+"_Effect1]";
    var add = StantonSCS3d.Peak7(engine.getParameterForValue(group, item, value),
                                 0.0, 1.0);
    var byte1 = 0xB0 + StantonSCS3d.channel;
    midi.sendShortMsg(byte1,control,0x28+add);
}

StantonSCS3d.FXS3LEDs = function (value) {
    StantonSCS3d.FXSliderLEDs(value,0x0C,"parameter1");
}

StantonSCS3d.FXS4LEDs = function (value) {
    StantonSCS3d.FXSliderLEDs(value,0x01,"parameter2");
}

StantonSCS3d.FXS5LEDs = function (value) {
    StantonSCS3d.FXSliderLEDs(value,0x0E,"parameter3");
}

StantonSCS3d.VUMeterLEDs = function (value) {
    if (!StantonSCS3d.VUMeters) return;
    // If in DECK mode, ignore this.
    if (StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"]=="deck") return;

    var add = StantonSCS3d.Peak7(value,0,1);
    var byte1 = 0xB0 + StantonSCS3d.channel;
    if (StantonSCS3d.deck % 2 == 0) midi.sendShortMsg(byte1,0x0E,0x28+add);
    else midi.sendShortMsg(byte1,0x0C,0x28+add);
}

StantonSCS3d.pitchLEDs = function (value) {
    var LEDs = 0;
    if (value>=-1) LEDs++;
    if (value>-0.78) LEDs++;
    if (value>-0.56) LEDs++;
    if (value>-0.33) LEDs++;
    if (value>-0.11) LEDs++;
    if (value>0.11) LEDs++;
    if (value>0.33) LEDs++;
    if (value>0.56) LEDs++;
    if (value>0.78) LEDs++;
    var byte1 = 0xB0 + StantonSCS3d.channel;
    midi.sendShortMsg(byte1,0x03,0x14+LEDs);
}

StantonSCS3d.gainLEDs = function (value) {
    // Skip if displaying something else
    var currentMode = StantonSCS3d.mode_store["[Channel"+StantonSCS3d.deck+"]"];
    if ((currentMode=="deck" && !StantonSCS3d.state["forceGain"]) || StantonSCS3d.modifier[currentMode]==1) return;

    var LEDs = 0;
    if (value>0.01) LEDs++;
    if (value>0.13) LEDs++;
    if (value>0.26) LEDs++;
    if (value>0.38) LEDs++;
    if (value>0.50) LEDs++;
    if (value>0.63) LEDs++;
    if (value>0.75) LEDs++;
    if (value>0.88) LEDs++;
    if (value>=1) LEDs++;
    var byte1 = 0xB0 + StantonSCS3d.channel;
    midi.sendShortMsg(byte1,0x07,0x28+LEDs);
}

StantonSCS3d.MasterVolumeLEDs = function (value) {
    var LEDs = 0;
    var mid = 1.0;
    var lowMidInterval = 1/8;   // Half the actual interval so the LEDs light in the middle of the interval
    var midHighInterval = 4/8;  // Half the actual interval so the LEDs light in the middle of the interval
    if (value>0.0) LEDs++;
    if (value>lowMidInterval) LEDs++;
    if (value>lowMidInterval*3) LEDs++;
    if (value>lowMidInterval*5) LEDs++;
    if (value>lowMidInterval*7) LEDs++;
    if (value>mid+midHighInterval) LEDs++;
    if (value>mid+midHighInterval*3) LEDs++;
    if (value>mid+midHighInterval*5) LEDs++;
    if (value>mid+midHighInterval*7) LEDs++;
    var byte1 = 0xB0 + StantonSCS3d.channel;
    midi.sendShortMsg(byte1,0x07,0x28+LEDs);
}

StantonSCS3d.pitchSliderLED = function (value) {
    var byte1 = 0x90 + StantonSCS3d.channel;
    switch (true) {
        case (value<=StantonSCS3d.pitchRanges[0]):
            midi.sendShortMsg(byte1,0x3D,0x00);  // Pitch LED black
            midi.sendShortMsg(byte1,0x3E,0x00);
            break;
        case (value<=StantonSCS3d.pitchRanges[1]):
            midi.sendShortMsg(byte1,0x3D,0x00);  // Pitch LED blue
            midi.sendShortMsg(byte1,0x3E,0x01);
            break;
        case (value<=StantonSCS3d.pitchRanges[2]):
            midi.sendShortMsg(byte1,0x3D,0x01);  // Pitch LED purple
            midi.sendShortMsg(byte1,0x3E,0x01);
            break;
        default:
            midi.sendShortMsg(byte1,0x3D,0x01);  // Pitch LED red
            midi.sendShortMsg(byte1,0x3E,0x00);
            break;
    }
}

StantonSCS3d.circleLEDs1 = function (value) {
    if (StantonSCS3d.deck!=1) return;
    StantonSCS3d.circleLEDs(value);
}

StantonSCS3d.circleLEDs2 = function (value) {
    if (StantonSCS3d.deck!=2) return;
    StantonSCS3d.circleLEDs(value);
}

StantonSCS3d.circleLEDs3 = function (value) {
    if (StantonSCS3d.deck!=3) return;
    StantonSCS3d.circleLEDs(value);
}

StantonSCS3d.circleLEDs4 = function (value) {
    if (StantonSCS3d.deck!=4) return;
    StantonSCS3d.circleLEDs(value);
}

StantonSCS3d.durationChange1 = function (value) {
    StantonSCS3d.trackDuration[1]=value;
}

StantonSCS3d.durationChange2 = function (value) {
    StantonSCS3d.trackDuration[2]=value;
}

StantonSCS3d.durationChange3 = function (value) {
    StantonSCS3d.trackDuration[3]=value;
}

StantonSCS3d.durationChange4 = function (value) {
    StantonSCS3d.trackDuration[4]=value;
}

StantonSCS3d.circleFlash = function (deck) {
    if (StantonSCS3d.deck != deck) return;  // Only do this for the current deck
    if (!StantonSCS3d.state["circleInvert"]) StantonSCS3d.state["circleInvert"]=true;
    else StantonSCS3d.state["circleInvert"]=false;
    // Force the circle LEDs to light
    StantonSCS3d.lastLight[deck]=-1;
    StantonSCS3d.circleLEDs(engine.getValue("[Channel"+deck+"]","playposition"));
}

StantonSCS3d.circleLEDs = function (value) {

    if (value<0) return;

    var deck = StantonSCS3d.deck;

    var currentMode = StantonSCS3d.mode_store["[Channel"+deck+"]"];
    if (StantonSCS3d.spinningPlatterOnlyVinyl) {    // Skip if not in vinyl mode
        if (currentMode != "vinyl" && currentMode != "vinyl2") return;
    } else {
        // Skip if in LOOP2-3, TRIG, or VINYL3 modes since they use the circle LEDs
        //  for other things
        if (currentMode == "vinyl3" || currentMode.substring(0,4) == "loop" ||
        currentMode.substring(0,4) == "trig") return;
    }

    // Flash the circle near the end of the track if the track is longer than 30s
    if (StantonSCS3d.trackDuration[deck]>30) {
        var trackTimeRemaining = ((1-value) * StantonSCS3d.trackDuration[deck]) | 0;
        if (trackTimeRemaining<=30 && trackTimeRemaining>15) {   // If <30s left, flash slowly
            if (StantonSCS3d.timer["30s-d"+deck] == -1) {
                // Start timer
                StantonSCS3d.timer["30s-d"+deck] = engine.beginTimer(500, () => { StantonSCS3d.circleFlash(deck); });
                if (StantonSCS3d.timer["15s-d"+deck] != -1) {
                    // Stop the 15s timer if it was running
                    engine.stopTimer(StantonSCS3d.timer["15s-d"+deck]);
                    StantonSCS3d.timer["15s-d"+deck] = -1;
                }
            }
        } else if (trackTimeRemaining<=15 && trackTimeRemaining>0) { // If <15s left, flash quickly
            if (StantonSCS3d.timer["15s-d"+deck] == -1) {
                // Start timer
                StantonSCS3d.timer["15s-d"+deck] = engine.beginTimer(125, () => { StantonSCS3d.circleFlash(deck); });
                if (StantonSCS3d.timer["30s-d"+deck] != -1) {
                    // Stop the 30s timer if it was running
                    engine.stopTimer(StantonSCS3d.timer["30s-d"+deck]);
                    StantonSCS3d.timer["30s-d"+deck] = -1;
                }
            }
        } else {    // Stop flashing
            if (StantonSCS3d.timer["15s-d"+deck] != -1) {
                engine.stopTimer(StantonSCS3d.timer["15s-d"+deck]);
                StantonSCS3d.timer["15s-d"+deck] = -1;
            }
            if (StantonSCS3d.timer["30s-d"+deck] != -1) {
                engine.stopTimer(StantonSCS3d.timer["30s-d"+deck]);
                StantonSCS3d.timer["30s-d"+deck] = -1;
            }
            StantonSCS3d.state["circleInvert"]=false;
        }
    } else StantonSCS3d.state["circleInvert"]=false;

    // Revolution time of the imaginary record in seconds
    var revtime = StantonSCS3d.revtime;

    if (StantonSCS3d.spinningLights==2) revtime = revtime/2;    // Use this for two lights
    var currentTrackPos = value * StantonSCS3d.trackDuration[deck];

    var revolutions = currentTrackPos/revtime;

    var light = ((revolutions-(revolutions|0))*16)|0;   // OR with 0 replaces Math.floor and is faster
    if (StantonSCS3d.spinningLights==2) light = ((revolutions-(revolutions|0))*8)|0;    // Use this for two lights

    // Don't send light commands if there's no visible change
    if (StantonSCS3d.lastLight[deck]==light) return;

    // Clear circle lights
    var byte1 = 0xB0 + StantonSCS3d.channel;
    var byte2 = 0x62;   // normal
    if (StantonSCS3d.state["circleInvert"]) byte2 = 0x72;   // inverted
    midi.sendShortMsg(byte1,byte2,0x00);

    var byte1 = 0x90 + StantonSCS3d.channel;
    var byte3 = 0x01;
    StantonSCS3d.lastLight[deck]=light;
    if (StantonSCS3d.state["circleInvert"]) byte3 = 0x00;
    midi.sendShortMsg(byte1,0x5d+light,byte3);
    if (StantonSCS3d.spinningLights==2) midi.sendShortMsg(byte1,0x65+light,byte3);   // Add this for two lights
}

/* TODO:
 * - Alter Trig mode to combine sets of two buttons (top two and bottom two on each row)
 * - Change fine pitch mode to key adjust mode
 *
 * Changes:
 * - FX1,2,3 control that effect unit
 * - FX B11 is now reverseroll (censor) instead of regular reverse
 * - FX B12 toggles this effect unit on this deck
 * - Mode + SYNC = toggle quantize
 */
