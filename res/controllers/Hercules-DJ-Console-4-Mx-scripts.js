////////////////////////////////////////////////////////////////////////
// JSHint configuration                                               //
////////////////////////////////////////////////////////////////////////
/* global engine                                                      */
/* global script                                                      */
/* global print                                                       */
/* global midi                                                        */
//////////////////////////////////////////////////////////////////////// 
//=====================================================
//___ Hercules DJ Console 4-Mx scripts by josepma. ____
//___________ Based partially on the Mk4 script _______
//=====================================================
// Version 2015-12-12: Initial version.
//
// Usage:
// ------
// Variables on Hercules4Mx.userSettings can be modified by users to suit their preferences.
// 
// This mapping and script supports 4 deck mode, jog wheel, scrathing, the whole mixer,
// an improved method of navigation and loops, hotcues and effects. It also has flashing sync button
// following the beats of the song.
// Some options have been modified to do things different than what they were designed for.
//
// Things you should know:
// 
// 1) Navigation:
//   .Clicking on Files or Folders switches between the library sidebar and the main list.
//   .Clicking on Folders when it is already on folders opens/closes the tree branch.
//   .Press up or down to navigate. If you keep the button pressed, it will continue in that direction
//   .If you press up or down, keep the button pressed and move any of the jog wheels, the navigation
//    will follow your wheel movements. You can navigate this way until releasing the up or down key.
// 2) AutoDJ:
//   .The "Auto" button is set to work with AutoDJ. In a future release it should be possible to implement
//    the auto-crossfade feature that it does in virtualDJ, but it didn't seem necessary for now.
//   .If the application is not in autoDJ mode, pressing the Auto button activates the autoDJ mode.
//   .If autoDJ mode is activated, Auto button is lit.
//   .If you press the AutoDJ button when it is in AutoDJ mode, it starts a fade to next song and button will flash.
//   .If in autoDJ mode and pressing the Files button (and already at "Files"), it will skip the next song.
//   .AutoDJ mode is deactivated by stopping decks one and two.
// 3) Scratching:
//   .In order to be able to scratch, you need to be in scratch mode. That's what the "scratch" button is for.
//   .If scratch mode, when moving the jog wheels without pressure it will act normally, and when moved with pressure
//    they will scratch.
// 4) Song reproduction:
//   .When a song is loaded on a deck and the "autoHeadcueOnLoad" option is set to true, the headphone cue (PFL) will
//    automatically switch to that deck, like what Virtual DJ does. This is disabled on AutoDJ.
//   .If none of the headphone cue buttons (PFL) is active and autoHeadMix is set to true, the cue/Mix knob
//    will be set to full mix so that the mix is heard on headphones.
//   .The jog wheels act as usual: when song is stopped they allow fast seeking and when song is playing the speed up or slow down.
//   .Pitch sliders act the way it is configured in Mixxx (range and direction)
//   .Sync button will be flashing following the beats of the song, like it does in virtualDJ. In a future release this could be optional.
//   .Pitch bend buttons sped up or slow down temporarily as configured in Mixxx.
//   .Pitch range buttons act as key modifiers.
//   .Pitch reset (pressing both pitch range buttons) resets the key.
//   .Pitch reset led is lit when key lock is active. In a future version this might change to indicate
//    that the key is playing at a different pitch than original, which is what i think Virtual DJ does.
//   .Cue and play work as configured in Mixxx.
//   .Several controls have the "softtakeover" option enabled, which means that the control will not move in 
//    the application if the hardware control is far from the position in the application.
// 5) Effects
//   .The FX knob in the controller moves the "Fast effect" knob, which is the knob present in the Deck section and
//    configurable in the "Equalizer" preferences in Mixxx. In a future version, it could also control the "Superknob"
//    of the active effect.
//   .If you keep the "shift" button pressed while moving the knob, it will move slowly (more precision).
//   .There are 12 effects, 6 with Shift disabled and 6 with Shift enabled.
//   .The first four effects are configured to beatloop loops of size 0.5, 1, 2 and 4. They act like the corresponding buttons in Mixxx.
//   .If you keep the shift key pressed and press effects 1 and 2, they will set a 0.125 and 0.25th beatloops.
//   .When a loop is set that isn't one of these four cases, buttons 3 and 4 will be lit to indicate a loop is present.
//   .If you keep the shift key pressed and press either effects 3 or 4, It will act like pressing the loop end/reloop button.
//   .The fifth and sixth effects are for reverse play, and reverse and roll.
//   .From seventh to tenth, they are the first four hotcues, and act like the buttons in Mixxx (if not set, set. if set, play it)
//   .If you keep the shift key pressed and press either of the four hotcue buttons, the hotcue will get cleared.
//   .Eleventh and twelveth effects activate the Effects rack 1 and 2 for the specific deck.
//
//_____________________________________________________
var Hercules4Mx = function() {};

Hercules4Mx.userSettings = {
    // --- DJ Console 4MX tray-icon configuration ---
    // Midi channel for controls (Tab "advanced") If "1-2", then 0, if "3-4" then 2, if "5-6" then 4...
    'midiChannelOffset': 0,
    // Sensitivity (Tab "Main"). Normal means 1/1. You can change the jog wheel sensitivity by either changing this, changing the hercules configuration setting, or both.
    'sensitivity': 1 / 1,

    // --- Personal preferences configuration ---
    // Playback speed of the virtual vinyl that is being scratched. 45.00 and 33.33 are the common speeeds. (Lower number equals faster scratch)
    'vinylSpeed': 45,
    // alpha value for scratching filter (start with 1/8 (0.125) and tune from there)
    'alpha': 1 / 8,
    // beta value for scratching filter (start with alpha/32 and tune from there)
    'beta': (1 / 8) / 32,
    // Indicates if the Headphone/Master mix should automatically be set to master when none of the headphone cue buttons are pressed.
    'autoHeadMix': false,
    // KeyRepeat speed for navigation up/down, in milliseconds. 250 is a good value. Lower values make it scroll faster
    'naviScrollSpeed': 250,
    // VirtualDJ-like automatic switching for headphone cue (PFL) on song load
    'autoHeadcueOnLoad': true
};


///////////////////////////////////////////////////////////////////
// --- Internal variables ----
Hercules4Mx.debuglog = false;

Hercules4Mx.navigationStatus = {
    //Navigation direction 1 up, -1 down, 0 undefined
    'direction': 0,
    //Indicator that the up or down buttons are pressed. Separated from direction to avoid a race condition with jogwheel.
    'enabled': 0,
    //Indicates if navigating in the sidebar (1) , or in the library (0)
    'sidebar': 0,
    //Holds the timeout event id that does the key-repeat action for moving up or down holding only the key.
    'timeoutId': 0
};

Hercules4Mx.scratchButton = 0;
Hercules4Mx.previousHeadMix = 0;
Hercules4Mx.autoDJfadingId = 0;
Hercules4Mx.shiftTimerId = 0;
Hercules4Mx.shiftPressed = false;
Hercules4Mx.shiftUsed = false;

Hercules4Mx.NOnC1 = 0x90 + Hercules4Mx.userSettings.midiChannelOffset;
Hercules4Mx.NOnC2 = 0x91 + Hercules4Mx.userSettings.midiChannelOffset;
Hercules4Mx.CC = 0xB0 + Hercules4Mx.userSettings.midiChannelOffset;

///////////////////////////////////////////////////////////////////
// --- Initialization and shutdown ----
Hercules4Mx.init = function(id, debugging) {
    Hercules4Mx.debuglog = debugging;
    Hercules4Mx.allLedsOff();
    var i;
    //Shift and deck buttons set to default
    for (i = 0x72; i <= 0x77; i++) {
        midi.sendShortMsg(Hercules4Mx.CC, i, 0x00);
    }
    // Tell the controller to report all current values to Mixxx (update_all_controls message)
    midi.sendShortMsg(Hercules4Mx.CC, 0x7F, 0x7F);
    //Activate Files led.
    midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3E, 0x7F);

    // Connect song load to onSongLoaded event
    if (Hercules4Mx.userSettings.autoHeadcueOnLoad) {
        for (i = 1; i <= 4; i++) {
            engine.connectControl("[Channel" + i + "]", "LoadSelectedTrack", "Hercules4Mx.onSongLoaded");
        }
    }
    engine.connectControl("[AutoDJ]", "enabled", "Hercules4Mx.onAutoDJ");
    engine.connectControl("[AutoDJ]", "fade_now", "Hercules4Mx.onAutoDJFade");
	engine.trigger("[AutoDJ]", "enabled", "Hercules4Mx.onAutoDJ");

    engine.connectControl("[Channel1]", "pfl", "Hercules4Mx.onPreFaderListen");
    engine.connectControl("[Channel2]", "pfl", "Hercules4Mx.onPreFaderListen");
    engine.connectControl("[Channel3]", "pfl", "Hercules4Mx.onPreFaderListen");
    engine.connectControl("[Channel4]", "pfl", "Hercules4Mx.onPreFaderListen");

    engine.connectControl("[Channel1]", "loop_enabled", "Hercules4Mx.onLoopStateChange");
    engine.connectControl("[Channel2]", "loop_enabled", "Hercules4Mx.onLoopStateChange");
    engine.connectControl("[Channel3]", "loop_enabled", "Hercules4Mx.onLoopStateChange");
    engine.connectControl("[Channel4]", "loop_enabled", "Hercules4Mx.onLoopStateChange");
    engine.connectControl("[Channel1]", "loop_start_position", "Hercules4Mx.onLoopStateChange");
    engine.connectControl("[Channel2]", "loop_start_position", "Hercules4Mx.onLoopStateChange");
    engine.connectControl("[Channel3]", "loop_start_position", "Hercules4Mx.onLoopStateChange");
    engine.connectControl("[Channel4]", "loop_start_position", "Hercules4Mx.onLoopStateChange");
    engine.connectControl("[Channel1]", "loop_end_position", "Hercules4Mx.onLoopStateChange");
    engine.connectControl("[Channel2]", "loop_end_position", "Hercules4Mx.onLoopStateChange");
    engine.connectControl("[Channel3]", "loop_end_position", "Hercules4Mx.onLoopStateChange");
    engine.connectControl("[Channel4]", "loop_end_position", "Hercules4Mx.onLoopStateChange");

    engine.connectControl("[EffectRack1_EffectUnit1]", "group_[Channel1]_enable", "Hercules4Mx.onEffectStateChange");
    engine.connectControl("[EffectRack1_EffectUnit1]", "group_[Channel2]_enable", "Hercules4Mx.onEffectStateChange");
    engine.connectControl("[EffectRack1_EffectUnit1]", "group_[Channel3]_enable", "Hercules4Mx.onEffectStateChange");
    engine.connectControl("[EffectRack1_EffectUnit1]", "group_[Channel4]_enable", "Hercules4Mx.onEffectStateChange");
    engine.connectControl("[EffectRack1_EffectUnit2]", "group_[Channel1]_enable", "Hercules4Mx.onEffectStateChange");
    engine.connectControl("[EffectRack1_EffectUnit2]", "group_[Channel2]_enable", "Hercules4Mx.onEffectStateChange");
    engine.connectControl("[EffectRack1_EffectUnit2]", "group_[Channel3]_enable", "Hercules4Mx.onEffectStateChange");
    engine.connectControl("[EffectRack1_EffectUnit2]", "group_[Channel4]_enable", "Hercules4Mx.onEffectStateChange");
};

Hercules4Mx.shutdown = function() {
    Hercules4Mx.allLedsOff();
    if (Hercules4Mx.navigationStatus.timeoutId !== 0) {
        engine.stopTimer(Hercules4Mx.navigationStatus.timeoutId);
    }
    if (Hercules4Mx.autoDJfadingId !== 0) {
        engine.stopTimer(Hercules4Mx.autoDJfadingId);
    }
};

//Set all leds to off
Hercules4Mx.allLedsOff = function() {
    engine.log("Hercules4Mx.allLedsOff: switching leds off");
    // Switch off all LEDs
    // +0x40 -> blinking.
    var i;
    for (i = 0x1; i <= 0x11; i++) {
        midi.sendShortMsg(Hercules4Mx.NOnC1, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x40, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x40, 0x00);
    }
    for (i = 0x15; i <= 0x1A; i++) {
        midi.sendShortMsg(Hercules4Mx.NOnC1, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x40, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x40, 0x00);
    }
    for (i = 0x21; i <= 0x31; i++) {
        midi.sendShortMsg(Hercules4Mx.NOnC1, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x40, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x40, 0x00);
    }
    for (i = 0x35; i <= 0x3A; i++) {
        midi.sendShortMsg(Hercules4Mx.NOnC1, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x40, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x40, 0x00);
    }
    for (i = 0x3C; i <= 0x3F; i++) {
        midi.sendShortMsg(Hercules4Mx.NOnC1, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x40, 0x00);
    }
};
///////////////////////////////////////////////////////////////////
// --- Events ----

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
        //If no beatloops set but loop is enabled, lit buttons 3 and 4
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
    // If automatic head mix to master is enabled, check what to do.
    if (Hercules4Mx.userSettings.autoHeadMix) {
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

// AutoDJ fade to next is pressed. (seems it isn't called when the fading is done 
// automatically by AutoDJ instead of button pressed
Hercules4Mx.onAutoDJFade = function(value, group, control) {
    //Flashing led to indicate fading
    midi.sendShortMsg(Hercules4Mx.NOnC1, 0x7C, 0x7F);
    if (Hercules4Mx.autoDJfadingId !== 0) {
        //Ensure the timer is off.
        //This is a safety measure in case the button is pressed again within the 5 second delay.
        engine.stopTimer(Hercules4Mx.autoDJfadingId);
        Hercules4Mx.autoDJfadingId = 0;
    }
    //After 5 seconds, restore non-flashing led. It would be perfect if autoDJFade was triggered also
    //when the fading ends, but right now it seems this is not possible. Also, it doesn't seem to be
    //an option to get the duration of the fading, that's why i simply put there 5 seconds.
    Hercules4Mx.autoDJfadingId = engine.beginTimer(5000, "Hercules4Mx.onAutoDJFadeOff", true);
};
Hercules4Mx.onAutoDJFadeOff = function() {
    midi.sendShortMsg(Hercules4Mx.NOnC1, 0x7C, 0x00);
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
        engine.setParameter(group, "start_stop", 1);

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
            if (Hercules4Mx.navigationStatus.sidebar === 0 &&
                engine.getParameter("[AutoDJ]", "enabled") === 1) {
                // if autoDJ enabled and we are already at "files", skip next file
                engine.setParameter("[AutoDJ]", "skip_next", 1);
            } else {
                Hercules4Mx.navigationStatus.sidebar = 0;
                midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3E, 0x7F);
                midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3F, 0x00);
            }
        } else if (control === 0x3F) {
            //FOLDERS
            if (Hercules4Mx.navigationStatus.sidebar === 1) {
                //if we are already on sidebar, open/close group
                engine.setParameter('[Playlist]', 'ToggleSelectedSidebarItem', 1);
            }
            Hercules4Mx.navigationStatus.sidebar = 1;
            midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3E, 0x00);
            midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3F, 0x7F);
        } else if (control === 0x40) {
            //UP
            Hercules4Mx.navigationStatus.direction = -1;
            Hercules4Mx.navigationStatus.enabled = 1;
            Hercules4Mx.doNavigate();
        } else if (control === 0x41) {
            //DOWN
            Hercules4Mx.navigationStatus.direction = 1;
            Hercules4Mx.navigationStatus.enabled = 1;
            Hercules4Mx.doNavigate();
        }
        if (Hercules4Mx.navigationStatus.enabled == 1 &&
            Hercules4Mx.navigationStatus.timeoutId === 0) {
            //Enable key-repeat mode. Cursor will continue moving until button is released.
            Hercules4Mx.navigationStatus.timeoutId = engine.beginTimer(Hercules4Mx.userSettings.naviScrollSpeed, Hercules4Mx.doNavigate);
        }
    } else {
        //On key release disable navigation mode and stop key-repeat.
        Hercules4Mx.navigationStatus.direction = 0;
        Hercules4Mx.navigationStatus.enabled = 0;
        if (Hercules4Mx.navigationStatus.timeoutId !== 0) {
            engine.stopTimer(Hercules4Mx.navigationStatus.timeoutId);
            Hercules4Mx.navigationStatus.timeoutId = 0;
        }
    }
};

// Internal navigate action.
Hercules4Mx.doNavigate = function() {
    if (Hercules4Mx.navigationStatus.sidebar === 0) {
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
    var direction = (value === 0x01) ? 1 : -1;
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
// There are 6 physical buttons present, but the controller has a "shift" button 
// that it uses to sends 12 different messages, depending if it is shifted or not.
// Since this is a button and it sends a different message to indicate button
// pressure and button state, I've been able to setup up to 24 different actions.
// I call these additional 12 messages the "shift-pressed" mode.
Hercules4Mx.FXButton = function(midichan, control, value, status, group) {
    var deck = script.deckFromGroup(group);
    if (Hercules4Mx.shiftPressed) {
        //Tell shift not to change state.
        Hercules4Mx.shiftUsed = true;
    }
    switch (control) {
        // 1 to 6 and 0x21 to 0x26 are the 6 left deck and right deck buttons respectively, non shifted.
        // Loop buttons
        case 0x01:
        case 0x21: // K1, beatloop 1/ Loop in
            if (value) {
                //Loop in in "shift-pressed" mode, beatloop of 1 beat otherwise
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
                //Loop out in "shift-pressed" mode, beatloop of 2 beat otherwise
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
            engine.setValue(group, "reverse", (value) ? 1 : 0);
            break;
        case 0x06:
        case 0x26: // K6, reverse roll play
            engine.setValue(group, "reverseroll", (value) ? 1 : 0);
            engine.setValue(group, "reverse", (value) ? 1 : 0);
            break;

            // 7 to 0xC and 0x27 to 0x3C are the 6 left deck and right deck buttons respectively, with shift enabled.
            // Hotcue buttons:
        case 0x07:
        case 0x27: // K7 Hotcue 1 activate/ clear
            if (Hercules4Mx.shiftPressed) {
                if (value) {
                    engine.setValue(group, "hotcue_1_clear", 1);
                }
            } else engine.setValue(group, "hotcue_1_activate", (value) ? 1 : 0);
            break;
        case 0x08:
        case 0x28: // K8 Hotcue 2 activate/ clear
            if (Hercules4Mx.shiftPressed) {
                if (value) {
                    engine.setValue(group, "hotcue_2_clear", 1);
                }
            } else engine.setValue(group, "hotcue_2_activate", (value) ? 1 : 0);
            break;
        case 0x09:
        case 0x29: // K9 Hotcue 3 activate/ clear
            if (Hercules4Mx.shiftPressed) {
                if (value) {
                    engine.setValue(group, "hotcue_3_clear", 1);
                }
            } else engine.setValue(group, "hotcue_3_activate", (value) ? 1 : 0);
            break;
        case 0x0A:
        case 0x2A: // K10 Hotcue 4 activate/ clear
            if (Hercules4Mx.shiftPressed) {
                if (value) {
                    engine.setValue(group, "hotcue_4_clear", 1);
                }
            } else engine.setValue(group, "hotcue_4_activate", (value) ? 1 : 0);
            break;
            //Effects
        case 0x0B:
        case 0x2B: // K11. Effect Unit 1
            {
                if (value) {
                    engine.setParameter("[EffectRack1_EffectUnit1]", "group_[Channel" + deck + "]_enable", !engine.getParameter("[EffectRack1_EffectUnit1]", "group_[Channel" + deck + "]_enable"));
                }
            }
            break;
        case 0x0C:
        case 0x2C: // K12. Effect Unit 2
            {
                if (value) {
                    engine.setParameter("[EffectRack1_EffectUnit2]", "group_[Channel" + deck + "]_enable", !engine.getParameter("[EffectRack1_EffectUnit2]", "group_[Channel" + deck + "]_enable"));
                }
            }
            break;
    }
};


//Jog wheel moved without pressure (for speeding or slowing down, or navigating)
Hercules4Mx.jogWheel = function(midichan, control, value, status, group) {
    var direction = (value === 0x01) ? 1 : -1;
    if (Hercules4Mx.navigationStatus.enabled === 1) {
        if (Hercules4Mx.navigationStatus.timeoutId !== 0) {
            //Stop key-repeat mode. From now on, obey only jog movement until button is released.
            engine.stopTimer(Hercules4Mx.navigationStatus.timeoutId);
            Hercules4Mx.navigationStatus.timeoutId = 0;
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
        if (Hercules4Mx.scratchButton === 1) {
            Hercules4Mx.scratchButton = 0;
            midi.sendShortMsg(Hercules4Mx.NOnC1, 0x7D, 0x00);
        } else {
            Hercules4Mx.scratchButton = 1;
            midi.sendShortMsg(Hercules4Mx.NOnC1, 0x7D, 0x7F);
        }
    }
};
// The pressure action over the jog wheel
Hercules4Mx.wheelTouch = function(midichan, control, value, status, group) {
    if (Hercules4Mx.scratchButton === 1 && value) {
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
    if (Hercules4Mx.navigationStatus.enabled === 1 ||
        !engine.isScratching(script.deckFromGroup(group))) {
        //If navigating, or not in scratch mode, do jogWheel
        Hercules4Mx.jogWheel(midichan, control, value, status, group);
    } else {
        var direction = (value === 0x01) ? 1 : -1;
        engine.scratchTick(script.deckFromGroup(group), direction);
    }
};
