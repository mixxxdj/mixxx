/////////////////////////////////////////////////////////////////////
//=====================================================
//_#_ Hercules DJ Console 4-Mx scripts by josepma. _##_
//_##________ Based partially on the Mk4 script ___###_
//=====================================================
// Author: josepma@gmail.com
//
// Version 2015-12-12: 
//        Initial version. 4 decks, jog wheel and scratch, autodj, navigation and effects.
// Version 2015-12-19: 
//        Improvements from https://github.com/mixxxdj/mixxx/pull/810
//        Beat flashing can be configurd on pitch reset led, jog led, sync button or disabled completely.
//        Option to switch automatically to scratch crossfader curve on scratch mode.
//        Soft takeover for pitch, volume faders, eq knobs and gain.
//        Use the 14bit range automatically (8bit actually) for the pitch slider if the controller is configured to use it.
//        Changed stop button to be cue_gotoandstop.
//        Removed midi channel configuration. It also required modifying the mapping, so it didn't really work.
//        Automatically setup some internal values, like 4 decks mode
//        Support speed sensor on jog wheel and Fx knob. They need to be moved really fast, so it's rarely useful.
//
// Usage:
// ------
// Check the dedicated controller wiki page at: 
//    http://mixxx.org/wiki/doku.php/hercules_dj_console_4-mx
//
// Variables on Hercules4Mx.userSettings can be modified by users to suit their preferences.
/////////////////////////////////////////////////////////////////////
var Hercules4Mx = function() {};


// The 4 possible values for the beatFlashLed option below.
Hercules4Mx.Leds = {
    "none" : 0,
    "syncLed" : 0x11,
    "pitchResetLed" : 0x15,
    "JogLed" : 0x1A
};
// --- Personal preferences configuration ---
Hercules4Mx.userSettings = {
    // Indicates if the Headphone/Master mix should automatically be set to master when none of the headphone cue buttons are activated.
    'autoHeadMix': false,
    // Enable automatically the headphone cue select (PFL) of the deck when a song is loaded. (Like in virtual-dj)
    'autoHeadcueOnLoad': true,
    // Flashing at the rythm of the beat on the led. Use the Leds map above.
    // Note: if using sync button, then the button will not show sync master state.
    'beatFlashLed': Hercules4Mx.Leds.JogLed,
    // KeyRepeat speed for navigating up/down, in milliseconds. 125 is a good value. Lower values make it scroll faster.
    'naviScrollSpeed': 125,
    // The controller has two modes to report the crossfader position. The default/beatmix curve, and the scratch curve.
    // The default curve reports the real position of the control. The scratch curve just crossfades on the edges.
    // Setting this setting to true, the curve will change to scratch curve when the scratch mode is on (scratch button).
    // Setting it to false will not change it, so it will use the setting configured in the DJHercules Tray-icon configuration.
    'crossfaderScratchCurve' : false,
    // _Scratching_ Playback speed of the virtual vinyl that is being scratched. 45.00 and 33.33 are the common speeeds. (Lower number equals faster scratch)
    'vinylSpeed': 45,
    // _Scratching_ You should configure this setting to the same value than in the DJHercules tray icon configuration. (Normal means 1/1).
    // If crossfaderScratchCurve is true, or the setting is changed while Mixxx is active, this value will be detected automatically.
    'sensitivity': 1 / 1,
    // _Scratching_ alpha value for the filter (start with 1/8 (0.125) and tune from there)
    'alpha': 1 / 8,
    // _Scratching_ beta value for the filter (start with alpha/32 and tune from there)
    'beta': (1 / 8) / 32,
     // This controls the function of the deck C/deck D buttons (changes the setting in the tray-icon configuration, avanced tab)
     // Deck button mode: deckmode=0 2 Decks only, deckmode=1 2 Decks with deck switch button command, deckmode=2 4 decks.
     // Since Mixxx supports 4 decks and this mapping is configured for all four decks, the default value is 2.
     'deckButtonMode': 2
};


////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
////////////////////////////////////////////////////////////////////////
// --- Internal variables ----
Hercules4Mx.debuglog = false;

Hercules4Mx.navigationStatus = {
    //Navigation direction 1 up, -1 down, 0 do not move
    'direction': 0,
    //Indicator that the up or down buttons are pressed. Separated from direction to avoid a race condition with jogwheel.
    'enabled': false,
    //Indicates if navigating in the sidebar, or in the library
    'sidebar': false,
    //Holds the timeout event id that does the key-repeat action for moving up or down holding only the key.
    'timeoutId': null
};

Hercules4Mx.scratchEnabled = false;
Hercules4Mx.previousHeadMix = 0;
Hercules4Mx.autoDJfadingId = null;
Hercules4Mx.shiftPressed = false;
Hercules4Mx.shiftUsed = false;
//Assume 14bit mode is disabled by default, and enable it on the first lsb detected.
Hercules4Mx.pitch14bitMode = false;
Hercules4Mx.pitchMsbValue = [0x40,0x40,0x40,0x40];

// The Hercules Tray Icon configuration allows to configure a different midi channel for the
// controller. You will need to change all the status codes in the xml mapping and these three
// variables if you use any other than the default of midi channels 1-2.
Hercules4Mx.NOnC1 = 0x90;
Hercules4Mx.NOnC2 = 0x91;
Hercules4Mx.CC = 0xB0;

///////////////////////////////////////////////////////////////////
// --- Initialization and shutdown ----
Hercules4Mx.init = function(id, debugging) {
    Hercules4Mx.debuglog = debugging;
    //ensure all leds are in their default state
    Hercules4Mx.allLedsOff();
    //Activate Files led.
    midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3E, 0x7F);
    
    var i;
    //Shift and deck buttons set to default
    for (i = 0x72; i <= 0x77; i++) {
        midi.sendShortMsg(Hercules4Mx.CC, i, 0x00);
    }
    // Deck button mode
    midi.sendShortMsg(Hercules4Mx.CC, 0x78, Hercules4Mx.userSettings.deckButtonMode);
    // If the crossfader on scratch setting is on, set the value to normal curve by default.
    if (Hercules4Mx.userSettings.crossfaderScratchCurve) {
        midi.sendShortMsg(Hercules4Mx.CC, 0x7E, 0x00);
    }
    // Tell the controller to report all current values to Mixxx (update_all_controls message)
    // Concretely it reports crossfader, master volume, master headmix, and EQ knobs, gain, pitch slider and vol fader of each channel.
    midi.sendShortMsg(Hercules4Mx.CC, 0x7F, 0x7F);

    //---Other possible actions:
    // jog wheel movement sensitivity divisor (i.e. 1/x).
    // midi.sendShortMsg(Hercules4Mx.CC, 0x79, sens); sens = 0 most sensitive, 0x7F least sensitive. 0x1 normal, 0x2 1/2, 0x4 1/4, and so on.
    //
    // ignore jog wheel movement: (codes from 0x7A to 0x7D, one for each deck).
    // midi.sendShortMsg(Hercules4Mx.CC, 0x7A, enable); enable = 0 obey movement, enable = 0x7F ignore movement

    // Connect several signals to javascript events, like song load, pre-fader-listen, loops or effects
    engine.connectControl("[AutoDJ]","enabled","Hercules4Mx.onAutoDJ");
    engine.connectControl("[AutoDJ]","fade_now","Hercules4Mx.onAutoDJFade");
    for (i = 1; i <= 4; i++) {
        engine.connectControl("[Channel" + i + "]", "pfl", "Hercules4Mx.onPreFaderListen");
        engine.connectControl("[Channel" + i + "]", "loop_enabled", "Hercules4Mx.onLoopStateChange");
        engine.connectControl("[Channel" + i + "]", "loop_start_position", "Hercules4Mx.onLoopStateChange");
        engine.connectControl("[Channel" + i + "]", "loop_end_position", "Hercules4Mx.onLoopStateChange");
//        TODO: control when they are enabled, so that FX knob can move the effect unit knob.
//        engine.connectControl("[EffectRack1_EffectUnit1]", "group_[Channel" + i + "]_enable", "Hercules4Mx.onEffectStateChange");
//        engine.connectControl("[EffectRack1_EffectUnit2]", "group_[Channel" + i + "]_enable", "Hercules4Mx.onEffectStateChange");
    }
    if (Hercules4Mx.userSettings.autoHeadcueOnLoad) {
        for (i = 1; i <= 4; i++) {
            engine.connectControl("[Channel" + i + "]", "LoadSelectedTrack", "Hercules4Mx.onSongLoaded");
        }
    }
    if (Hercules4Mx.userSettings.beatFlashLed !== Hercules4Mx.Leds.syncLed ){
        //Set sync master led indicator
        for (i = 1; i <= 4; i++) {
            engine.connectControl("[Channel" + i + "]", "sync_enabled", "Hercules4Mx.onSyncLed");
        }
    }
    if (Hercules4Mx.userSettings.beatFlashLed !== Hercules4Mx.Leds.none) {
        //Setup beat flashing led
        for (i = 1; i <= 4; i++) {
            engine.connectControl("[Channel" + i + "]", "beat_active", "Hercules4Mx.onBeatFlash");
        }
    }
    
    // Activate soft takeover for the rate sliders. The other sliders and knobs have softtakeover set in the xml mapping.
    for (i = 1; i <= 4; i++) {
        engine.softTakeover("[Channel" + i + "]","rate",true);
        engine.softTakeover("[Channel" + i + "]","rate",true);
    }
};
Hercules4Mx.shutdown = function() {
    if (Hercules4Mx.navigationStatus.timeoutId !== null) {
        engine.stopTimer(Hercules4Mx.navigationStatus.timeoutId);
    }
    if (Hercules4Mx.autoDJfadingId !== null) {
        engine.stopTimer(Hercules4Mx.autoDJfadingId);
    }
    // If the crossfader on scratch setting is on, set the value to normal curve on exit.
    if (Hercules4Mx.userSettings.crossfaderScratchCurve) {
        midi.sendShortMsg(Hercules4Mx.CC, 0x7E, 0x00);
    }
    //Cleanup leds before exiting.
    Hercules4Mx.allLedsOff();
};

//Set all leds to off
Hercules4Mx.allLedsOff = function() {
    engine.log("Hercules4Mx.allLedsOff: switching leds off");
    // Switch off all LEDs
    // +0x20 -> the other deck
    // +0x40 -> blinking.
    var i;
    for (i = 0x3C; i <= 0x3F; i++) { //auto, scratch, files, folders
        midi.sendShortMsg(Hercules4Mx.NOnC1, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x40, 0x00);
    }
    for (i = 0x1; i <= 0x11; i++) { // Fx, cue, play, cuesel,stop, sync
        midi.sendShortMsg(Hercules4Mx.NOnC1, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x20, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x20, 0x00);
    }
    for (i = 0x15; i <= 0x1A; i++) { //pitch led, source, kill,jog touch
        midi.sendShortMsg(Hercules4Mx.NOnC1, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x20, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x20, 0x00);
    }
    // I've put these on a separate loop so that there are more chances for decks A/B leds to light off when shutdown,
    // since there is a strange problem where not all messages are delivered.
    for (i = 0x1; i <= 0x11; i++) { // Fx, cue, play, cuesel,stop, sync
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x40, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x40, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x60, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x60, 0x00);
    }
    for (i = 0x15; i <= 0x1A; i++) { //pitch led, source, kill,jog touch
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x40, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x40, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x60, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x60, 0x00);
    }
};
///////////////////////////////////////////////////////////////////
// --- Events ----

// The jog wheel sensitivity setting has changed. This is reported in two scenarios:
// when the setting is changed in the tray icon, and when the crossfader curve is changed to beatmix.
Hercules4Mx.onSensitivityChange = function(value, group, control) {
    Hercules4Mx.userSettings.sensitivity = 1/value;    
}

//Action to do when a song is loaded in a deck. virtualDJ automatically enables the headphone cue (PFL)
Hercules4Mx.onSongLoaded = function(value, group, control) {
    var i;
    if (engine.getParameter("[AutoDJ]", "enabled") === 0) {
        //If we are not in autodj mode
        var deck = script.deckFromGroup(group);
        for (i = 1; i <= 4; i++) {
            //Change headphone cue (pfl) to the deck on which the song loaded.
            engine.setParameter("[Channel" + i + "]", "pfl", (deck === i) ? 1 : 0);
        }

        var currentHeadMix = engine.getParameter("[Master]", "headMix");
        if (currentHeadMix == 1) {
            //Change the headmix if it was to full Mix.
            engine.setParameter("[Master]", "headMix", Hercules4Mx.previousHeadMix);
        }
    }
};

//A change in the loop position or loop state has happened.
Hercules4Mx.onLoopStateChange = function(value, group, control) {
    engine.log("Hercules4Mx.onLoopStateChange: value, group, control: " + value + ", " + group + ", " + control);
    var finalvalue = (value == -1) ? 0 : value;
    var deck = script.deckFromGroup(group);
    var messageto = (deck === 1 || deck == 2) ? Hercules4Mx.NOnC1 : Hercules4Mx.NOnC2;
    var offset = (deck === 1 || deck == 3) ? 0x00 : 0x20;
    var b1 = engine.getParameter("[Channel" + deck + "]", "beatloop_0.5_enabled");
    var b2 = engine.getParameter("[Channel" + deck + "]", "beatloop_1_enabled");
    var b3 = engine.getParameter("[Channel" + deck + "]", "beatloop_2_enabled");
    var b4 = engine.getParameter("[Channel" + deck + "]", "beatloop_4_enabled");
    if (!b1 && !b2 && !b3 && !b4) {
        //If no beatloops set but loop is enabled, light up buttons 3 and 4
        b3 = (finalvalue) ? 1 : 0;
        b4 = b3;
    }
    midi.sendShortMsg(messageto, 0x01 + offset, b1);
    midi.sendShortMsg(messageto, 0x02 + offset, b2);
    midi.sendShortMsg(messageto, 0x03 + offset, b3);
    midi.sendShortMsg(messageto, 0x04 + offset, b4);
};

// Controls the action to do when the headphone cue (pre-fader-listen) buttons are pressed.
Hercules4Mx.onPreFaderListen = function(value, group, control) {
    if (Hercules4Mx.userSettings.autoHeadMix) {
        // If automatic head mix to master is enabled, check what to do.
        var pfl1 = engine.getParameter("[Channel1]", "pfl");
        var pfl2 = engine.getParameter("[Channel2]", "pfl");
        var pfl3 = engine.getParameter("[Channel3]", "pfl");
        var pfl4 = engine.getParameter("[Channel4]", "pfl");
        var currentHeadMix = engine.getParameter("[Master]", "headMix");

        if (pfl1 === 0 && pfl2 === 0 && pfl3 === 0 && pfl4 === 0) {
            // If they are all disabled after switching, move headmix to master.
            Hercules4Mx.previousHeadMix = currentHeadMix;
            engine.setParameter("[Master]", "headMix", 1);
        } else if (currentHeadMix == 1) {
            // If at least one is enabled and headmix is set to master, restore previous headmix.
            engine.setParameter("[Master]", "headMix", Hercules4Mx.previousHeadMix);
        }
    }
};

//AutoDJ is activated or deactivated.
Hercules4Mx.onAutoDJ = function(value, group, control) {
    midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3C, (value) ? 0x7F : 0x00);
};

// AutoDJ fade to next is pressed. (seems it isn't called when the fading is triggered 
// automatically by AutoDJ compared to pressing the button in Mixxx/controller)
Hercules4Mx.onAutoDJFade = function(value, group, control) {
    //Flashing led to indicate fading
    midi.sendShortMsg(Hercules4Mx.NOnC1, 0x7C, 0x7F);
    if (Hercules4Mx.autoDJfadingId !== null) {
        //Ensure the timer is off.
        //This is a safety measure in case the button is pressed again within the 5 second delay.
        engine.stopTimer(Hercules4Mx.autoDJfadingId);
        Hercules4Mx.autoDJfadingId = null;
    }
    //After 5 seconds, restore non-flashing led. It would be perfect if autoDJFade was triggered also
    //when the fading ends, but right now it seems this is not possible. Also, it doesn't seem to be
    //an option to get the duration of the fading, that's why i simply put there 5 seconds.
    Hercules4Mx.autoDJfadingId = engine.beginTimer(5000, "Hercules4Mx.onAutoDJFadeOff", true);
};
Hercules4Mx.onAutoDJFadeOff = function() {
    midi.sendShortMsg(Hercules4Mx.NOnC1, 0x7C, 0x00);
};

//Beat flashing changed state
Hercules4Mx.onBeatFlash = function(value, group, control) {
    var deck = script.deckFromGroup(group);
    var val = (value) ? 0x7F : 0x00;
    var led = Hercules4Mx.userSettings.beatFlashLed;
    switch(deck){
        case 1: midi.sendShortMsg(Hercules4Mx.NOnC1, led, val); break;
        case 2: midi.sendShortMsg(Hercules4Mx.NOnC1, led+0x20, val); break;
        case 3: midi.sendShortMsg(Hercules4Mx.NOnC2, led, val); break;
        case 4: midi.sendShortMsg(Hercules4Mx.NOnC2, led+0x20, val); break;
    }
};
// Deck's Sync led changed state
Hercules4Mx.onSyncLed = function(value, group, control) {
    var deck = script.deckFromGroup(group);
    var val = (value) ? 0x7F : 0x00;
    switch(deck){
        case 1: midi.sendShortMsg(Hercules4Mx.NOnC1, 0x11, val); break;
        case 2: midi.sendShortMsg(Hercules4Mx.NOnC1, 0x31, val); break;
        case 3: midi.sendShortMsg(Hercules4Mx.NOnC2, 0x11, val); break;
        case 4: midi.sendShortMsg(Hercules4Mx.NOnC2, 0x31, val); break;
    }
};

///////////////////////////////////////////////////////////////////
// --- Actions ----
//Auto DJ button is pressed. Two functions: enable autoDJ, and Fade to next track.
//The virtualDJ function is to do a fade to next in interactive mode (Mixxx fade to next requires autoDJ)
//The would then be determined by the beats per minute.
Hercules4Mx.autoDJButton = function(midichan, control, value) {
    if (value) {
        if (engine.getParameter("[AutoDJ]", "enabled") === 0) {
            //If not enable, enable autoDJ
            engine.setParameter("[AutoDJ]", "enabled", 1);
        } else {
            //If already in autodj, do a crossfade to next track
            engine.setParameter("[AutoDJ]", "fade_now", 1);
        }
    }
};

//Stop button is pressed in a deck.
Hercules4Mx.stopButton = function(midichan, control, value, status, group) {
    if (value) {
        engine.setParameter(group, "cue_gotoandstop", 1);

        var stop1 = engine.getParameter("[Channel1]", "stop");
        var stop2 = engine.getParameter("[Channel2]", "stop");
        var autodj = engine.getParameter("[AutoDJ]", "enabled");
        //Note: autodj only acts on decks 1 and 2, so not checking decks 3 and 4.
        if (stop1 && stop2 && autodj) {
            //If autoDJ enabled and all decks are stopped. disable autoDJ.
            engine.setParameter("[AutoDJ]", "enabled", 0);
        }
    }
};

//Advanced navigation mode. 
Hercules4Mx.navigation = function(midichan, control, value, status, group) {
    if (value) {
        if (control === 0x3E) {
            //FILES
            if (Hercules4Mx.navigationStatus.sidebar === false && 
                    engine.getParameter("[AutoDJ]", "enabled") === 1) {
                // if autoDJ enabled and we are already at "files", skip next file
                engine.setParameter("[AutoDJ]", "skip_next", 1);
            } else {
                Hercules4Mx.navigationStatus.sidebar = false;
                midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3E, 0x7F);
                midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3F, 0x00);
            }
        } else if (control === 0x3F) {
            //FOLDERS
            if (Hercules4Mx.navigationStatus.sidebar ) {
                //if we are already on sidebar, open/close group
                engine.setParameter('[Playlist]', 'ToggleSelectedSidebarItem', 1);
            }
            Hercules4Mx.navigationStatus.sidebar = true;
            midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3E, 0x00);
            midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3F, 0x7F);
        } else if (control === 0x40) {
            //UP
            Hercules4Mx.navigationStatus.direction = -1;
            Hercules4Mx.navigationStatus.enabled = true;
            Hercules4Mx.doNavigate();
        } else if (control === 0x41) {
            //DOWN
            Hercules4Mx.navigationStatus.direction = 1;
            Hercules4Mx.navigationStatus.enabled = true;
            Hercules4Mx.doNavigate();
        }
        if (Hercules4Mx.navigationStatus.enabled &&
                Hercules4Mx.navigationStatus.timeoutId === null) {
            //Enable key-repeat mode. Cursor will continue moving until button is released.
            Hercules4Mx.navigationStatus.timeoutId = engine.beginTimer(Hercules4Mx.userSettings.naviScrollSpeed, Hercules4Mx.doNavigate);
        }
    } else {
        //On key release disable navigation mode and stop key-repeat.
        Hercules4Mx.navigationStatus.direction = 0;
        Hercules4Mx.navigationStatus.enabled = false;
        if (Hercules4Mx.navigationStatus.timeoutId !== null) {
            engine.stopTimer(Hercules4Mx.navigationStatus.timeoutId);
            Hercules4Mx.navigationStatus.timeoutId = null;
        }
    }
};

// Internal navigate action.
Hercules4Mx.doNavigate = function() {
    if (Hercules4Mx.navigationStatus.sidebar === false) {
        if (Hercules4Mx.navigationStatus.direction === 1) {
            engine.setParameter("[Playlist]", "SelectNextTrack", "1");
        } else {
            engine.setParameter("[Playlist]", "SelectPrevTrack", "1");
        }
    } else {
        if (Hercules4Mx.navigationStatus.direction === 1) {
            engine.setParameter("[Playlist]", "SelectNextPlaylist", "1");
        } else {
            engine.setParameter("[Playlist]", "SelectPrevPlaylist", "1");
        }
    }
};
//Any of the shift buttons for effects has been pressed. This button simply changes
//the controller internal state, but we can use it for other reasons while the user maintains it pressed.
Hercules4Mx.pressEffectShift = function(midichan, control, value) {
    // I don't diferentiate between decks. I don't expect two shift buttons being pressed at the same time.
    Hercules4Mx.shiftPressed = (value) ? true : false;
};
//Indicator of the shift effect state change. This happens always after shift is released
//We control if we let it change or not
Hercules4Mx.stateEffectShift = function(midichan, control, value, status, group) {
    if (Hercules4Mx.shiftUsed) {
        //If shift state was changed because of the alternate shift usage, restore its state.
        Hercules4Mx.shiftUsed = false;
        var deck = script.deckFromGroup(group) - 1;
        midi.sendShortMsg(Hercules4Mx.CC, 0x72 + deck, !value);
    }
};

//The effect knob granularity is very coarse, so we compensate it here so that it behaves like an analog one.
Hercules4Mx.effectKnob = function(midichan, control, value, status, group) {
    //It has a speed sensor, but you have to move it really fast for it to send something different.
    var direction = (value < 0x40) ? value : value-0x80;
    var step = 1 / 20;
    if (Hercules4Mx.shiftPressed) {
        //If pressing shift, let's move it slowly.
        step = 1 / 127;
        //Tell shift button not to change state.
        Hercules4Mx.shiftUsed = true;
    }
    engine.setParameter(group, "super1", engine.getParameter(group, "super1") + step * direction);
};

// Any of the FX buttons has been pressed
// There are 6 physical buttons present per deck, and also a "shift" button used by the controller 
// itself to switch between messages 1 to 6 and messages 7 to 12, depending if it is enabled or not.
// Since the shift button also sends a message when it is pressed and when it is released,
// I've been able to setup up to 24 different actions per deck.
// I call these additional 12 messages the "shift-pressed" mode.
Hercules4Mx.FXButton = function(midichan, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    if (Hercules4Mx.shiftPressed) {
        //Tell shift not to change state.
        Hercules4Mx.shiftUsed = true;
    }
    switch (control) {
        // 1 to 6 and 0x21 to 0x26 are the 6 left deck and right deck buttons respectively, with shift disabled.
        // Loop buttons
        case 0x01:
        case 0x21: // K1, beatloop 1/ Loop in
            if (value) {
                //beatloop 0.125 in "shift-pressed" mode, beatloop of 0.5 beat otherwise
                if (Hercules4Mx.shiftPressed) {
                    engine.setValue(group, "beatloop_0.125_toggle", 1);
                } else {
                    engine.setValue(group, "beatloop_0.5_toggle", 1);
                }
            }
            break;
        case 0x02:
        case 0x22: // K2, beatloop 2/ Loop out
            if (value) {
                //beatlook 0.25 in "shift-pressed" mode, beatloop of 1 beat otherwise
                if (Hercules4Mx.shiftPressed) {
                    engine.setValue(group, "beatloop_0.25_toggle", 1);
                } else {
                    engine.setValue(group, "beatloop_1_toggle", 1);
                }
            }
            break;
        case 0x03:
        case 0x23: // K3, beatloop 4/ Reloop/Exit
            if (value) {
                //enable/disable loop in "shift-pressed" mode, beatloop of 2 beat otherwise
                if (Hercules4Mx.shiftPressed) {
                    engine.setValue(group, "reloop_exit", 1);
                } else {
                    engine.setValue(group, "beatloop_2_toggle", 1);
                }
            }
            break;
        case 0x04:
        case 0x24: // K4, beatloop 8/ Reloop/Exit
            if (value) {
                //enable/disable loop in "shift-pressed" mode, beatloop of 4 beat otherwise
                if (Hercules4Mx.shiftPressed) {
                    engine.setValue(group, "reloop_exit", 1);
                } else {
                    engine.setValue(group, "beatloop_4_toggle", 1);
                }
            }
            break;
            // Reverse play buttons
        case 0x05:
        case 0x25: // K5, reverse play
            engine.setValue(group, "reverse", (value) ? 1 : 0);
            break;
        case 0x06:
        case 0x26: // K6, reverse roll play
            engine.setValue(group, "reverseroll", (value) ? 1 : 0);
            break;

            // 7 to 0xC and 0x27 to 0x3C are the 6 left deck and right deck buttons respectively, with shift enabled.
            // Hotcue buttons:
        case 0x07:
        case 0x27: // K7 Hotcue 1 activate/ clear
            if (Hercules4Mx.shiftPressed) {
                if (value) {
                    engine.setValue(group, "hotcue_1_clear", 1);
                }
            } else {
                engine.setValue(group, "hotcue_1_activate", (value) ? 1 : 0);
            }
            break;
        case 0x08:
        case 0x28: // K8 Hotcue 2 activate/ clear
            if (Hercules4Mx.shiftPressed) {
                if (value) {
                    engine.setValue(group, "hotcue_2_clear", 1);
                }
            } else {
                engine.setValue(group, "hotcue_2_activate", (value) ? 1 : 0);
            }
            break;
        case 0x09:
        case 0x29: // K9 Hotcue 3 activate/ clear
            if (Hercules4Mx.shiftPressed) {
                if (value) {
                    engine.setValue(group, "hotcue_3_clear", 1);
                }
            } else {
                engine.setValue(group, "hotcue_3_activate", (value) ? 1 : 0);
            }
            break;
        case 0x0A:
        case 0x2A: // K10 Hotcue 4 activate/ clear
            if (Hercules4Mx.shiftPressed) {
                if (value) {
                    engine.setValue(group, "hotcue_4_clear", 1);
                }
            } else {
                engine.setValue(group, "hotcue_4_activate", (value) ? 1 : 0);
            }
            break;
            //Effects
        case 0x0B:
        case 0x2B: // K11. Effect Unit 1
                if (value) {
                    engine.setParameter("[EffectRack1_EffectUnit1]", "group_[Channel" + deck + "]_enable", !engine.getParameter("[EffectRack1_EffectUnit1]", "group_[Channel" + deck + "]_enable"));
                }
            break;
        case 0x0C:
        case 0x2C: // K12. Effect Unit 2
                if (value) {
                    engine.setParameter("[EffectRack1_EffectUnit2]", "group_[Channel" + deck + "]_enable", !engine.getParameter("[EffectRack1_EffectUnit2]", "group_[Channel" + deck + "]_enable"));
                }
            break;
    }
};

//Jog wheel moved without pressure (for seeking, speeding or slowing down, or navigating)
Hercules4Mx.jogWheel = function(midichan, control, value, status, group) {
    //It has a speed sensor, but you have to move it really fast for it to send something different.
    var direction = (value < 0x40) ? value : value-0x80;
    if (Hercules4Mx.navigationStatus.enabled) {
        if (Hercules4Mx.navigationStatus.timeoutId !== null) {
            //Stop key-repeat mode. From now on, obey only jog movement until button is released.
            engine.stopTimer(Hercules4Mx.navigationStatus.timeoutId);
            Hercules4Mx.navigationStatus.timeoutId = null;
        }
        Hercules4Mx.navigationStatus.direction = direction;
        Hercules4Mx.doNavigate();
    } else {
        engine.setValue(group, "jog", direction + engine.getValue(group, "jog"));
    }
};

// The "scratch" button is used to enable or disable scratching on the jog wheels.
// Concretely, it tells if it has to ignore or not the pressure sensor in the jog wheel.
Hercules4Mx.scratchButton = function(midichan, control, value, status, group) {
    if (value) {
        if (Hercules4Mx.scratchEnabled ) {
            Hercules4Mx.scratchEnabled = false;
            midi.sendShortMsg(Hercules4Mx.NOnC1, 0x7D, 0x00);
            if (Hercules4Mx.userSettings.crossfaderScratchCurve) {
                midi.sendShortMsg(Hercules4Mx.CC, 0x7E, 0x00);
            }
        } else {
            Hercules4Mx.scratchEnabled = true;
            midi.sendShortMsg(Hercules4Mx.NOnC1, 0x7D, 0x7F);
            if (Hercules4Mx.userSettings.crossfaderScratchCurve) {
                midi.sendShortMsg(Hercules4Mx.CC, 0x7E, 0x7F);
            }
        }
    }
};
// The pressure action over the jog wheel
Hercules4Mx.wheelTouch = function(midichan, control, value, status, group) {
    if (Hercules4Mx.scratchEnabled && value) {
        // If button down
        engine.scratchEnable(script.deckFromGroup(group),
            256 * Hercules4Mx.userSettings.sensitivity,
            Hercules4Mx.userSettings.vinylSpeed,
            Hercules4Mx.userSettings.alpha,
            Hercules4Mx.userSettings.beta);
    } else {
        // If button up
        engine.scratchDisable(script.deckFromGroup(group));
    }
};
//Jog wheel used with pressure (for scratching)
Hercules4Mx.scratchWheel = function(midichan, control, value, status, group) {
    if (Hercules4Mx.navigationStatus.enabled ||
        !engine.isScratching(script.deckFromGroup(group))) {
        //If navigating, or not in scratch mode, do jogWheel
        Hercules4Mx.jogWheel(midichan, control, value, status, group);
    } else {
        //It has a speed sensor, but you have to move it really fast for it to send something different.
        var direction = (value < 0x40) ? value : value-0x80;
        engine.scratchTick(script.deckFromGroup(group), direction);
    }
};

// Pitch slider rate change, MSB (Most significant bits in 14bit mode, or directly the value in 7bit)
Hercules4Mx.rateMsb = function(midichan, control, value, status, group) {
    if (Hercules4Mx.pitch14bitMode) {
        var deck = script.deckFromGroup(group);
        Hercules4Mx.pitchMsbValue[deck-1]=value*0x80;
    }
    else {
        engine.setParameter(group, "rate", value/0x7F);
    }
};
// Pitch slider rate change, LSB (Least significant bits in 14bit mode, not called in 7bit)
Hercules4Mx.rateLsb = function(midichan, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    var msbval = Hercules4Mx.pitchMsbValue[deck-1];
    Hercules4Mx.pitch14bitMode = true;
    engine.setParameter(group, "rate", (msbval+value)/0x3FFF);
};
