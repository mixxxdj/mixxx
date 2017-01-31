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
// Version 2016-09-26
//        Beatgrid editing mode enabled with shift+sync. Allows to correct the beatgrid from the controller. 
//          (For when you're in the middle of a mix and something is not properly aligned)
//        Brake effect (power unplug) with shift+stop, backward playing moved to shift+play and forward and rewind by beats.
//        Keylock, quantize and master sync (shift pitch scale up, shift pitch scale down and sync pressed for 400ms respectively).
//        Improvements and fixes on sync button and navigation.
//        Corrections in soft takeover, and initialization of control values from their physical positions on Mixxx startup.
//        Audio vu meters on the kill/source buttons that switches between master and decks on pfl. Can be switched 
//          off on userSettings. If kill or source buttons are activated, the vu on that channel is deactivated.
//        Reimplemented the actions for Fx buttons. It now allows to have personalized mappings, and comes already with three:
//          Manual/VirtualDJ setup, Mixxx 2.0 setup and a new setup that adds more functionality, like playing 4 samplers.
//        New loop edit mode. It allows for a random length loop as well as more beatloop sizes. It is also possible to modify
//          a loop size with the fx knob. See the updated wiki for a detailed explanation.
//        The FX "super" knob can be controlled from the controller now. Maintain the fx button pressed and move the knob.
//        Pressing either of the pitch scale buttons (musical key action) together with moving the fx knob can be used to control the key
// Version 2016-11-06
//        Preview deck control. Press shift+PFL (headphones) on any deck, and the controls will be sent
//          to the preview deck instead. The button flashes to indicate this state.
//          Press PFL (with or without shift) to go back to usual deck controls.
//          What works: Load track, stop, cue, play, forward, rewind, jog wheel, Gain and fx buttons, like hotcues.
//          The preview deck is not a fully featured deck, so no pitch, sync or EQ.
//        Removed the deckButtonMode option. The user can always set that from the DJHerculesMix series TrayAgent.
// Version 2017-01-21
//        Round of fixes on several controls related to preview deck and effects.
//        Added permanent rate change action to pitch buttons when used with shift. (For those cases where
//          the defined range of the slider is not enough)
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
    "none": 0,
    "syncLed": 0x11,
    "pitchResetLed": 0x15,
    "JogLed": 0x1A
};
// --- Personal preferences configuration ---
Hercules4Mx.userSettings = {
    // Indicates if the Headphone/Master mix should automatically be set to master when none of the headphone cue buttons are activated.
    'autoHeadMix': false,
    // Enable automatically the headphone cue select (PFL) of the deck when a song is loaded. (Like in virtual-dj)
    'autoHeadcueOnLoad': true,
    // Flashing at the rythm of the beat on the led. Use the Leds map above.
    // Note: if using sync button, then the button will not show sync master state.
    'beatFlashLed': Hercules4Mx.Leds.none,
    // Simulate vuMeters using the kill and source buttons. If enabled, shows master vus, or deck vu depending if prefader listen button is enabled or not.
    'useVuMeters': true,
    // KeyRepeat speed for navigating up/down, in milliseconds. 100 is a good value. Lower values make it scroll faster.
    'naviScrollSpeed': 100,
    // The controller has two modes to report the crossfader position. The default/beatmix curve, and the scratch curve.
    // The default curve reports the real position of the control. The scratch curve just crossfades on the edges.
    // Setting this setting to true, the curve will change to scratch curve when the scratch mode is on (scratch button).
    // Setting it to false will not change it, so it will use the setting configured in the DJHercules Tray-icon configuration.
    'crossfaderScratchCurve': false,
    // _Scratching_ Playback speed of the virtual vinyl that is being scratched. 45.00 and 33.33 are the common speeeds. (Lower number equals faster scratch)
    'vinylSpeed': 45,
    // _Scratching_ You should configure this setting to the same value than in the DJHercules tray icon configuration. (Normal means 1/1).
    // If crossfaderScratchCurve is true, or the setting is changed while Mixxx is active, this value will be detected automatically.
    'sensitivity': 1 / 1,
    // _Scratching_ alpha value for the filter (start with 1/8 (0.125) and tune from there)
    'alpha': 1 / 8,
    // _Scratching_ beta value for the filter (start with alpha/32 and tune from there)
    'beta': (1 / 8) / 32,
    // This indicates which mapping for the FX buttons should Mixxx use.
    // The possible values are:
    // original : As they are in the Hercules Manual and the default setup in Virtual DJ 7 LE.
    // mixxx20 : As they were initially defined with the release of Mixxx 2.0
    // mixxx21 : AS they were defined with the release of Mixxx 2.1
    // With a little caution, you can modify them at the bottom of this file
    'FXbuttonsSetup': 'mixxx21'
};

//
// End of User configurable options
//
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
    'timeoutId': null,
    //Indicates if it is the first execution of the timer for this timer instance. We use this to have a longer start time when pressing the button than for keyrepeat.
    'isSingleShotTimer': false
};

Hercules4Mx.forwardRewindStatus = {
    //Navigation direction 1 forward, -1 backward, 0 do not move
    'direction': 0,
    //The group name for the deck of which the forward and rewind buttons have been pressed.
    'group': null,
    //Holds the timeout event id that starts the forward/backward seeking.
    'timeoutId': null
};

Hercules4Mx.syncEnabledStatus = {
    //Holds the timeout event id that starts the forward/backward seeking.
    'timeoutId': null,
    //Indicates if the timeout has been reached
    'triggered': 0
};

Hercules4Mx.editModes = {
    // Edit mode disabled
    'disabled': -1,
    // Beatgrid edit mode
    'beatgrid': 0,
    // Effect edit mode (currently unimplemented)
    'effects': 1,
    // Loop edit mode
    'loop': 2,
    // Effect knob moving mode
    'effectknob': 3,
    // Loop sizing mode
    'loopsizing': 4,
    // musical key change mode
    'pitchkeychanging': 5
};
Hercules4Mx.editModeStatus = {
    // The selected edit mode
    'mode': Hercules4Mx.editModes.disabled,
    // The selected effect or deck
    'effect': -1,
    // If the edit mode was used. (this is similar to the shiftstatus.used)
    'used': false
};
Hercules4Mx.shiftStatus = {
    //The shift button is currently pressed (hold down)
    'pressed': false,
    //Some action has been triggered that has used the shift status
    'used': false,
    //The brake action has been triggered (this is used so that the shift button can be released)
    'braking': false,
    //The brake action has been triggered (this is used so that the shift button can be released)
    'reversing': false
};
Hercules4Mx.VuMeterL = {
    // This is used internally to know that this is VuMeterR
    'initchan': 1,
    //top led (originally thought to be the clip indicator, but it is now the -3dB and clip is shown by flickering)
    'clip': 0x16,
    //vu leds
    'vu3': 0x17,
    'vu2': 0x18,
    'vu1': 0x19,
    // Which source to use for this vumeter. [Disabled] for no source
    'source': '[Master]',
    // Midichan of this vumeter. This is needed when switching the Decks (A/C, B/D)
    'midichan': 0x90,
    // Last value evaluated. This allows to quantize the value and reduce the amount of messages sent.
    'lastvalue': 0
};
Hercules4Mx.VuMeterR = {
    // This is used internally to know that this is VuMeterR
    'initchan': 2,
    //top led (originally thought to be the clip indicator, but it is now the -3dB and clip is shown by flickering)
    'clip': 0x36,
    //vu leds
    'vu3': 0x37,
    'vu2': 0x38,
    'vu1': 0x39,
    // Which source to use for this vumeter. [Disabled] for no source
    'source': '[Master]',
    // Midichan of this vumeter. This is needed when switching the Decks (A/C, B/D)
    'midichan': 0x90,
    // Last value evaluated. This allows to quantize the value and reduce the amount of messages sent.
    'lastvalue': 0
};
Hercules4Mx.previewOnDeck = {
    '[Channel1]':false,
    '[Channel2]':false,
    '[Channel3]':false,
    '[Channel4]':false
};
// Amount of time in milliseconds to hold a button to trigger some actions.
Hercules4Mx.KeyHoldTime = 500;
// Scratch mode is on (it means that it will obey the jog pressure)
Hercules4Mx.scratchEnabled = false;
// Value of the headmix previous to change it automatically on song load, if the setting is enabled.
Hercules4Mx.previousHeadMix = 0;
//Id of the autodj fading timer.
Hercules4Mx.autoDJfadingId = null;
//Assume 14bit mode is disabled by default, and enable it on the first lsb detected.
Hercules4Mx.pitch14bitMode = false;
//Array for the MSB bits of pitch change
Hercules4Mx.pitchMsbValue = [0x40, 0x40, 0x40, 0x40];
//Default action for the action map.
Hercules4Mx.noActionButtonMap = {
    //Action to do when pressed
    'buttonPressAction': Hercules4Mx.FxActionNoOp,
    //Action to do when released
    'buttonReleaseAction': null,
    //Additional information, if needed
    'extraParameter': null,
    //If this button has a led, this can be used to connect it.
    'ledToConnect': null
};
//Array of button mappings. It is filled from the presets at the end of this file.
Hercules4Mx.buttonMappings = [Hercules4Mx.noActionButtonMap];
//which are the beat loop buttons?
Hercules4Mx.beatLoopEditButtons = [];
//which is the button position of the loop present led?
Hercules4Mx.beatLoopReloopPos = -1;
//which is the button position of the loop enabled led?
Hercules4Mx.LoopEnabledPos = -1;
//At which button index are the sampler leds?
Hercules4Mx.samplerLedIdx = [];
//At which button index are the audio effect leds?
Hercules4Mx.FxLedIdx = [];

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
    // If the crossfader on scratch setting is on, set the value to normal curve by default.
    if (Hercules4Mx.userSettings.crossfaderScratchCurve) {
        midi.sendShortMsg(Hercules4Mx.CC, 0x7E, 0x00);
    }

    // Tell the controller to report all current values to Mixxx (update_all_controls message)
    // Concretely it reports crossfader, master volume, master headmix, and EQ knobs, gain, pitch slider and vol fader of each channel.
    midi.sendShortMsg(Hercules4Mx.CC, 0x7F, 0x7F);

    //---Other possible actions. These all can be done from the DJ Console configuration application (DJHerculesMix series trayAgent)
    // jog wheel movement sensitivity divisor (i.e. 1/x).
    // midi.sendShortMsg(Hercules4Mx.CC, 0x79, sens); sens = 0 most sensitive, 0x7F least sensitive. 0x1 normal, 0x2 1/2, 0x4 1/4, and so on.
    //
    // ignore jog wheel movement: (codes from 0x7A to 0x7D, one for each deck).
    // midi.sendShortMsg(Hercules4Mx.CC, 0x7A, enable); enable = 0 obey movement, enable = 0x7F ignore movement
    //
    // Deck button mode. Configure how the controller will respond to the actions of the buttons "Deck C" or "Deck D".
    // Deck button mode: deckmode=0 2 Decks only, deckmode=1 2 Decks with deck switch button command, deckmode=2 4 decks.
    // midi.sendShortMsg(Hercules4Mx.CC, 0x78, deckmode);

    // Connect several signals to javascript events, like song load, pre-fader-listen, loops or effects
    engine.connectControl("[AutoDJ]", "enabled", "Hercules4Mx.onAutoDJ");
    engine.connectControl("[AutoDJ]", "fade_now", "Hercules4Mx.onAutoDJFade");
    engine.trigger("[AutoDJ]", "enabled");
    for (i = 1; i <= 4; i++) {
        engine.connectControl("[Channel" + i + "]", "pfl", "Hercules4Mx.onPreFaderListen");
        engine.connectControl("[Channel" + i + "]", "loop_enabled", "Hercules4Mx.onLoopStateChange");
        engine.connectControl("[Channel" + i + "]", "loop_start_position", "Hercules4Mx.onLoopStateChange");
        engine.connectControl("[Channel" + i + "]", "loop_end_position", "Hercules4Mx.onLoopStateChange");
        engine.trigger("[Channel" + i + "]", "pfl");
    }
    if (Hercules4Mx.userSettings.autoHeadcueOnLoad) {
        for (i = 1; i <= 4; i++) {
            engine.connectControl("[Channel" + i + "]", "LoadSelectedTrack", "Hercules4Mx.onSongLoaded");
        }
    }
    if (Hercules4Mx.userSettings.beatFlashLed !== Hercules4Mx.Leds.syncLed) {
        //Set sync master led indicator
        for (i = 1; i <= 4; i++) {
            engine.connectControl("[Channel" + i + "]", "sync_enabled", "Hercules4Mx.onSyncLed");
            engine.trigger("[Channel" + i + "]", "sync_enabled");
        }
    }
    if (Hercules4Mx.userSettings.beatFlashLed !== Hercules4Mx.Leds.none) {
        //Setup beat flashing led
        for (i = 1; i <= 4; i++) {
            engine.connectControl("[Channel" + i + "]", "beat_active", "Hercules4Mx.onBeatFlash");
        }
    }
    if (Hercules4Mx.userSettings.useVuMeters) {
        engine.connectControl("[Master]", "VuMeterL", "Hercules4Mx.onVuMeterMasterL");
        engine.connectControl("[Master]", "VuMeterR", "Hercules4Mx.onVuMeterMasterR");
        for (i = 1; i <= 4; i++) {
            engine.connectControl("[Channel" + i + "]", "VuMeter", "Hercules4Mx.onVuMeterDeck" + i);
            engine.connectControl("[Channel" + i + "]", "passthrough", "Hercules4Mx.onKillOrSourceChange" + i);
            engine.connectControl("[EqualizerRack1_[Channel" + i + "]_Effect1]", "button_parameter3", "Hercules4Mx.onKillOrSourceChange" + i);
            engine.connectControl("[EqualizerRack1_[Channel" + i + "]_Effect1]", "button_parameter2", "Hercules4Mx.onKillOrSourceChange" + i);
            engine.connectControl("[EqualizerRack1_[Channel" + i + "]_Effect1]", "button_parameter1", "Hercules4Mx.onKillOrSourceChange" + i);
        }
    }
    if (Hercules4Mx.userSettings.FXbuttonsSetup === "original") {
        Hercules4Mx.setupFXButtonsLikeManual();
    } else if (Hercules4Mx.userSettings.FXbuttonsSetup === "mixxx20") {
        Hercules4Mx.setupFXButtonsCustomMixx20();
    } else /* if (Hercules4Mx.userSettings.FXbuttonsSetup === "mixxx21" ) */ {
        Hercules4Mx.setupFXButtonsCustomMixx21();
    }

    engine.beginTimer(3000, "Hercules4Mx.doDelayedSetup", true);
};
//timer-called (delayed) setup.
Hercules4Mx.doDelayedSetup = function() {
    var i;
    // Activate soft takeover for the relevant controls. Important: Previous to 2.1, engine.softTakeover only works when scripted with setValue !!
    for (i = 1; i <= 4; i++) {
        engine.softTakeover("[Channel" + i + "]", "rate", true);
        engine.softTakeover("[Channel" + i + "]", "volume", true);
        engine.softTakeover("[Channel" + i + "]", "pregain", true);
        engine.softTakeover("[EqualizerRack1_[Channel" + i + "]_Effect1]", "parameter3", true);
        engine.softTakeover("[EqualizerRack1_[Channel" + i + "]_Effect1]", "parameter2", true);
        engine.softTakeover("[EqualizerRack1_[Channel" + i + "]_Effect1]", "parameter1", true);
    }
    engine.softTakeover("[Master]", "crossfader", true);
};

Hercules4Mx.shutdown = function() {
    if (Hercules4Mx.navigationStatus.timeoutId !== null) {
        engine.stopTimer(Hercules4Mx.navigationStatus.timeoutId);
    }
    if (Hercules4Mx.forwardRewindStatus.timeoutId !== null) {
        engine.stopTimer(Hercules4Mx.forwardRewindStatus.timeoutId);
    }
    if (Hercules4Mx.syncEnabledStatus.timeoutId !== null) {
        engine.stopTimer(Hercules4Mx.syncEnabledStatus.timeoutId);
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
    if (Hercules4Mx.debuglog) {
        engine.log("Hercules4Mx.allLedsOff: switching leds off");
    }
    // Switch off all LEDs
    // +0x20 -> the other deck
    // +0x40 -> blinking.
    // NOnC2 : Decks C/D
    var i;
    for (i = 0x3C; i <= 0x3F; i++) { //auto, scratch, files, folders
        midi.sendShortMsg(Hercules4Mx.NOnC1, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x40, 0x00);
    }
    for (i = 0xD; i <= 0x11; i++) { // cue, play, PFL (cuesel),stop, sync
        midi.sendShortMsg(Hercules4Mx.NOnC1, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x20, 0x00);
    }
    for (i = 0x15; i <= 0x1A; i++) { //pitch led, source, kill,jog touch
        midi.sendShortMsg(Hercules4Mx.NOnC1, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x20, 0x00);
    }
    for (i = 0x1; i <= 0xC; i++) { // Fx
        midi.sendShortMsg(Hercules4Mx.NOnC1, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x20, 0x00);
    }
    // I've put these on a separate loop so that there are more chances for decks A/B leds to light off when shutdown,
    // since there is a strange problem where not all messages are delivered.
    for (i = 0x1; i <= 0x11; i++) { // Fx, cue, play, PFL (cuesel),stop, sync
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x40, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x60, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x20, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x40, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x60, 0x00);
    }
    for (i = 0x15; i <= 0x1A; i++) { //pitch led, source, kill,jog touch
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x40, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, i + 0x60, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x20, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x40, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC2, i + 0x60, 0x00);
    }
};

///////////////////////////////////////////////////////////////////
// --- Events ----

// The jog wheel sensitivity setting has changed. This is reported in two scenarios:
// when the setting is changed in the tray icon, and when the crossfader curve is changed to beatmix.
Hercules4Mx.sensitivityChanged = function(value, group, control, status, group) {
    Hercules4Mx.userSettings.sensitivity = 1 / value;
};

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
//This is used in conjunction with the keypad button mapping. Allows to refresh the leds.
Hercules4Mx.onEnableLed = function(value, group, control) {
    var deck = script.deckFromGroup(group);
    var messageto = (deck === 1 || deck === 2) ? Hercules4Mx.NOnC1 : Hercules4Mx.NOnC2;
    var offset = (deck === 1 || deck === 3) ? 0x00 : 0x20;
    for (var i = 0; i < 12; i++) {
        var led = Hercules4Mx.buttonMappings[i].ledToConnect;
        if (led !== null && led === control) {
            midi.sendShortMsg(messageto, i + 1 + offset, value);
        }
    }
};

//A change in the loop position or loop state has happened.
Hercules4Mx.onLoopStateChange = function(value, group, control) {
    if (Hercules4Mx.debuglog) {
        engine.log("Hercules4Mx.onLoopStateChange: value, group, control: " + value + ", " + group + ", " + control);
    }
    var deck = script.deckFromGroup(group);
    var messageto = (deck === 1 || deck === 2) ? Hercules4Mx.NOnC1 : Hercules4Mx.NOnC2;
    var offset = (deck === 1 || deck === 3) ? 0x00 : 0x20;
    var chandeck = "[Channel" + deck + "]";
    var loopenabled = parseInt(engine.getParameter(chandeck, "loop_enabled"));
    var beatenabled = 0;
    if (loopenabled !== 0) {
        for (var i = 0; i < 12; i++) {
            var led = Hercules4Mx.buttonMappings[i].ledToConnect;
            if (led !== null && led.indexOf("beatloop") !== -1) {
                beatenabled = beatenabled + parseInt(engine.getParameter(chandeck, led));
            }
        }
    }
    var newval;
    if (Hercules4Mx.LoopEnabledPos !== -1) {
        newval = (loopenabled > 0 && beatenabled === 0) ? 0x7F : 0;
        midi.sendShortMsg(messageto, Hercules4Mx.LoopEnabledPos + offset, newval);
    }
    if (Hercules4Mx.beatLoopReloopPos !== -1) {
        newval = (parseInt(engine.getParameter(chandeck, "loop_start_position")) > -1) ? 0x7F : 0;
        midi.sendShortMsg(messageto, Hercules4Mx.beatLoopReloopPos + offset, newval);
    }
};
//A change in an effect channel status has happened
Hercules4Mx.onEffectStateChange = function(value, group, control) {
    var fxidx = parseInt(group.slice(-2).substr(0, 1)); // "[EffectRack1_EffectUnit1]"
    if (fxidx > 0 && fxidx <= Hercules4Mx.FxLedIdx.length) {
        var newval = (value > 0) ? 0x7F : 0x0;
        var deck = parseInt(control.slice(-9).substr(0, 1)); // "group_[Channel1]_enable"
        var messageto = (deck === 1 || deck === 2) ? Hercules4Mx.NOnC1 : Hercules4Mx.NOnC2;
        var offset = (deck === 1 || deck === 3) ? 0x00 : 0x20;
        midi.sendShortMsg(messageto, Hercules4Mx.FxLedIdx[fxidx - 1] + offset, newval);
    }
};
//A change in a sampler playback state has happened
Hercules4Mx.onSamplerStateChange = function(value, group, control) {
    //This slice only works for up to 9 samplers. Since we only support 4 buttons, that's enough.
    var sampleidx = parseInt(group.slice(-2).substr(0, 1));
    if (sampleidx > 0 && sampleidx <= Hercules4Mx.samplerLedIdx.length) {
        var newval = (value > 0) ? 0x7F : 0x0;
        midi.sendShortMsg(Hercules4Mx.NOnC1, Hercules4Mx.samplerLedIdx[sampleidx - 1], newval);
        midi.sendShortMsg(Hercules4Mx.NOnC2, Hercules4Mx.samplerLedIdx[sampleidx - 1], newval);
    }
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
    if (Hercules4Mx.userSettings.useVuMeters) {
        var deck = script.deckFromGroup(group);
        if (deck === 1 || deck === 3) {
            Hercules4Mx.updateVumeterSourceAction(Hercules4Mx.VuMeterL, deck);
        } else {
            Hercules4Mx.updateVumeterSourceAction(Hercules4Mx.VuMeterR, deck);
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
    Hercules4Mx.autoDJfadingId = engine.beginTimer(5000, "Hercules4Mx.doEndAutoDJFadeOffAction", true);
};
Hercules4Mx.doEndAutoDJFadeOffAction = function() {
    midi.sendShortMsg(Hercules4Mx.NOnC1, 0x7C, 0x00);
};

//Beat flashing changed state
Hercules4Mx.onBeatFlash = function(value, group, control) {
    var deck = script.deckFromGroup(group);
    var val = (value) ? 0x7F : 0x00;
    var led = Hercules4Mx.userSettings.beatFlashLed;
    switch (deck) {
        case 1:
            midi.sendShortMsg(Hercules4Mx.NOnC1, led, val);
            break;
        case 2:
            midi.sendShortMsg(Hercules4Mx.NOnC1, led + 0x20, val);
            break;
        case 3:
            midi.sendShortMsg(Hercules4Mx.NOnC2, led, val);
            break;
        case 4:
            midi.sendShortMsg(Hercules4Mx.NOnC2, led + 0x20, val);
            break;
    }
};
// Deck's Sync led changed state
Hercules4Mx.onSyncLed = function(value, group, control) {
    var deck = script.deckFromGroup(group);
    if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.beatgrid) {
        //beatgrid mode is activated for all decks (i.e. it is not needed to activate and deactivate one by one)
        switch (deck) {
            case 1:
                midi.sendShortMsg(Hercules4Mx.NOnC1, 0x51, 1);
                break;
            case 2:
                midi.sendShortMsg(Hercules4Mx.NOnC1, 0x71, 1);
                break;
            case 3:
                midi.sendShortMsg(Hercules4Mx.NOnC2, 0x51, 1);
                break;
            case 4:
                midi.sendShortMsg(Hercules4Mx.NOnC2, 0x71, 1);
                break;
        }
    } else {
        var val = (value) ? 0x7F : 0x00;
        switch (deck) {
            case 1:
                midi.sendShortMsg(Hercules4Mx.NOnC1, 0x11, val);
                midi.sendShortMsg(Hercules4Mx.NOnC1, 0x51, 0);
                break;
            case 2:
                midi.sendShortMsg(Hercules4Mx.NOnC1, 0x31, val);
                midi.sendShortMsg(Hercules4Mx.NOnC1, 0x71, 0);
                break;
            case 3:
                midi.sendShortMsg(Hercules4Mx.NOnC2, 0x11, val);
                midi.sendShortMsg(Hercules4Mx.NOnC2, 0x51, 0);
                break;
            case 4:
                midi.sendShortMsg(Hercules4Mx.NOnC2, 0x31, val);
                midi.sendShortMsg(Hercules4Mx.NOnC2, 0x71, 0);
                break;
        }
    }
};

// only feed the correct levels to each channel of the vumeter
Hercules4Mx.onVuMeterMasterL = function(value) {
    if (Hercules4Mx.VuMeterL.source === '[Master]') {
        Hercules4Mx.updateVumeterEvent(Hercules4Mx.VuMeterL, value);
    }
};
Hercules4Mx.onVuMeterMasterR = function(value) {
    if (Hercules4Mx.VuMeterR.source === '[Master]') {
        Hercules4Mx.updateVumeterEvent(Hercules4Mx.VuMeterR, value);
    }
};
Hercules4Mx.onVuMeterDeck1 = function(value) {
    if (Hercules4Mx.VuMeterL.source === '[Channel1]') {
        Hercules4Mx.updateVumeterEvent(Hercules4Mx.VuMeterL, value);
    }
};
Hercules4Mx.onVuMeterDeck2 = function(value) {
    if (Hercules4Mx.VuMeterR.source === '[Channel2]') {
        Hercules4Mx.updateVumeterEvent(Hercules4Mx.VuMeterR, value);
    }
};
Hercules4Mx.onVuMeterDeck3 = function(value) {
    if (Hercules4Mx.VuMeterL.source === '[Channel3]') {
        Hercules4Mx.updateVumeterEvent(Hercules4Mx.VuMeterL, value);
    }
};
Hercules4Mx.onVuMeterDeck4 = function(value) {
    if (Hercules4Mx.VuMeterR.source === '[Channel4]') {
        Hercules4Mx.updateVumeterEvent(Hercules4Mx.VuMeterR, value);
    }
};
// update a vumeter channel
Hercules4Mx.updateVumeterEvent = function(vumeter, value) {
    var newval = parseInt(value * 0x80);
    if (vumeter.lastvalue !== newval) {
        vumeter.lastvalue = newval;
        if (engine.getValue(vumeter.source, "PeakIndicator") > 0) {
            // IF it clips, we put the top led on and the rest off, which gives a "flash" effect.
            midi.sendShortMsg(vumeter.midichan, vumeter.clip, 0x7F);
            newval = 0;
        }
        midi.sendShortMsg(vumeter.midichan, vumeter.clip, newval > 0x70 ? 0x7F : 0x00);
        midi.sendShortMsg(vumeter.midichan, vumeter.vu3, newval > 0x60 ? 0x7F : 0x00);
        midi.sendShortMsg(vumeter.midichan, vumeter.vu2, newval > 0x45 ? 0x7F : 0x00);
        midi.sendShortMsg(vumeter.midichan, vumeter.vu1, newval > 0x15 ? 0x7F : 0x00);
    }
};

// Duplicating for each deck because script.deckFromGroup does not work for equalizer controls.
Hercules4Mx.onKillOrSourceChange1 = function(value, group, control) {
    Hercules4Mx.updateVumeterSourceAction(Hercules4Mx.VuMeterL, 1);
};
Hercules4Mx.onKillOrSourceChange2 = function(value, group, control) {
    Hercules4Mx.updateVumeterSourceAction(Hercules4Mx.VuMeterR, 2);
};
Hercules4Mx.onKillOrSourceChange3 = function(value, group, control) {
    Hercules4Mx.updateVumeterSourceAction(Hercules4Mx.VuMeterL, 3);
};
Hercules4Mx.onKillOrSourceChange4 = function(value, group, control) {
    Hercules4Mx.updateVumeterSourceAction(Hercules4Mx.VuMeterR, 4);
};


///////////////////////////////////////////////////////////////////
// --- Actions ----
//Any of the shift buttons for effects has been pressed. This button simply changes
//the controller internal state, but we can use it for other reasons while the user maintains it pressed.
Hercules4Mx.pressEffectShift = function(midichan, control, value, status, group) {
    // I don't diferentiate between decks. I don't expect two shift buttons being pressed at the same time.
    Hercules4Mx.shiftStatus.pressed = (value) ? true : false;
};
//Indicator of the shift effect state change. This happens always after shift is released
//We control if we let it change or not
Hercules4Mx.stateEffectShift = function(midichan, control, value, status, group) {
    if (Hercules4Mx.shiftStatus.used) {
        //If shift state was changed because of the alternate shift usage, restore its state.
        Hercules4Mx.shiftStatus.used = false;
        var deck = script.deckFromGroup(group) - 1;
        midi.sendShortMsg(Hercules4Mx.CC, 0x72 + deck, !value);
    }
};

//Auto DJ button is pressed. Two functions: enable autoDJ, and Fade to next track.
//The virtualDJ function is to do a fade to next in interactive mode (Mixxx fade to next requires autoDJ)
//The would then be determined by the beats per minute.
Hercules4Mx.autoDJButton = function(midichan, control, value, status, group) {
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

//Cue button is pressed in a deck.
Hercules4Mx.cueButton = function(midichan, control, value, status, groupInitial) {
    var group = (Hercules4Mx.previewOnDeck[groupInitial] === true) ? '[PreviewDeck1]' : groupInitial;
    engine.setParameter(group, "cue_default", (value > 0) ? 1: 0);
};

//Stop button is pressed in a deck.
Hercules4Mx.stopButton = function(midichan, control, value, status, groupInitial) {
    var group = (Hercules4Mx.previewOnDeck[groupInitial] === true) ? '[PreviewDeck1]' : groupInitial;
    if (Hercules4Mx.shiftStatus.pressed || Hercules4Mx.shiftStatus.braking) { //Shifting: Do brake effect.
        var deck = script.deckFromGroup(group);
        engine.brake(deck, value ? true : false);
        Hercules4Mx.shiftStatus.braking = value ? true : false;
        //Tell shift button not to change state.
        Hercules4Mx.shiftStatus.used = true;
    } else if (value) { //Not shifting and pressed
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

//Play button is pressed in a deck.
Hercules4Mx.playButton = function(midichan, control, value, status, groupInitial) {
    var group = (Hercules4Mx.previewOnDeck[groupInitial] === true) ? '[PreviewDeck1]' : groupInitial;
   
    if (Hercules4Mx.shiftStatus.pressed || Hercules4Mx.shiftStatus.reversing) { //Shifting: Do backward playback effect.
        if (engine.getValue(group, "slip_enabled") !== 0) {
            engine.setValue(group, "reverseroll", (value) ? 1 : 0);
        } else {
            engine.setValue(group, "reverse", (value) ? 1 : 0);
        }
        Hercules4Mx.shiftStatus.reversing = value ? true : false;
        //Tell shift button not to change state.
        Hercules4Mx.shiftStatus.used = true;
    } else if (value) { //Not shifting and pressed
        engine.setParameter(group, "play", !engine.getParameter(group, "play"));
    }
};
// Forward button is pressed in a deck.
Hercules4Mx.forwardButton = function(midichan, control, value, status, groupInitial) {
    var group = (Hercules4Mx.previewOnDeck[groupInitial] === true) ? '[PreviewDeck1]' : groupInitial;
    
    if (Hercules4Mx.shiftStatus.pressed) { //Shifting: Jump 1 beat forward.
        if (value) {
            engine.setValue(group, "beatjump_1_forward", 1);
        }
        //Tell shift button not to change state.
        Hercules4Mx.shiftStatus.used = true;
    } else {
        //Jump 4 beats and enable Timer: if timer elapsed do "fwd"
        if (value) {
            engine.setValue(group, "beatjump_4_forward", 1);
            if (Hercules4Mx.forwardRewindStatus.timeoutId !== null) {
                //Safety measure, ensure timer is off
                engine.stopTimer(Hercules4Mx.forwardRewindStatus.timeoutId);
            }
            Hercules4Mx.forwardRewindStatus.direction = 1;
            Hercules4Mx.forwardRewindStatus.group = group;
            Hercules4Mx.forwardRewindStatus.timeoutId = engine.beginTimer(Hercules4Mx.KeyHoldTime, Hercules4Mx.doForwardRewindAction, true);
        } else {
            engine.stopTimer(Hercules4Mx.forwardRewindStatus.timeoutId);
            Hercules4Mx.forwardRewindStatus.timeoutId = null;
            engine.setParameter(group, "fwd", 0);
        }
    }
};

// rewind button is pressed in a deck.
Hercules4Mx.rewindButton = function(midichan, control, value, status, groupInitial) {
    var group = (Hercules4Mx.previewOnDeck[groupInitial] === true) ? '[PreviewDeck1]' : groupInitial;
    if (Hercules4Mx.shiftStatus.pressed) { //Shifting: Jump 1 beat backwards.
        if (value) {
            engine.setValue(group, "beatjump_1_backward", 1);
        }
        //Tell shift button not to change state.
        Hercules4Mx.shiftStatus.used = true;
    } else {
        //Jump 4 beats and enable Timer: if timer elapsed do "back"
        if (value) {
            engine.setValue(group, "beatjump_4_backward", 1);
            if (Hercules4Mx.forwardRewindStatus.timeoutId !== null) {
                //Safety measure, ensure timer is off
                engine.stopTimer(Hercules4Mx.forwardRewindStatus.timeoutId);
            }
            Hercules4Mx.forwardRewindStatus.direction = -1;
            Hercules4Mx.forwardRewindStatus.group = group;
            Hercules4Mx.forwardRewindStatus.timeoutId = engine.beginTimer(Hercules4Mx.KeyHoldTime, Hercules4Mx.doForwardRewindAction, true);
        } else {
            engine.stopTimer(Hercules4Mx.forwardRewindStatus.timeoutId);
            Hercules4Mx.forwardRewindStatus.timeoutId = null;
            engine.setParameter(group, "back", 0);
        }
    }
};
//Internal timer-called forward and rewind action.
Hercules4Mx.doForwardRewindAction = function() {
    if (Hercules4Mx.forwardRewindStatus.direction === 1) {
        engine.setParameter(Hercules4Mx.forwardRewindStatus.group, "fwd", "1");
    } else if (Hercules4Mx.forwardRewindStatus.direction === -1) {
        engine.setParameter(Hercules4Mx.forwardRewindStatus.group, "back", "1");
    }
};

// pitch scale minus button is pressed in a deck.
Hercules4Mx.pScaleDownButton = function(midichan, control, value, status, group) {
    //preview deck has no rate control
    if (Hercules4Mx.previewOnDeck[group] === false) {
        if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.beatgrid) { //Edit mode. Move the beatgrid to the left.
            engine.setParameter(group, "beats_translate_earlier", value);
        } else if (Hercules4Mx.shiftStatus.pressed && value) { // Shift mode: enable/disable keylock
            engine.setParameter(group, "keylock", !engine.getParameter(group, "keylock"));
            //Tell shift button not to change state.
            Hercules4Mx.shiftStatus.used = true;
        } else if (value) {
            engine.setParameter(group, "pitch_adjust_down_small", value);
            if (Hercules4Mx.editModeStatus.mode !== Hercules4Mx.editModes.disabled) {
                Hercules4Mx.deactivateEditModeAction();
            }
            Hercules4Mx.activateEditModeAction(Hercules4Mx.editModes.pitchkeychanging, '');
            Hercules4Mx.editModeStatus.used = false;
        } else {
            Hercules4Mx.deactivateEditModeAction();
        }
    }
};

// pitch scale plus button is pressed in a deck.
Hercules4Mx.pScaleUpButton = function(midichan, control, value, status, group) {
    //preview deck has no rate control
    if (Hercules4Mx.previewOnDeck[group] === false) {
        if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.beatgrid) { //Edit mode. Move the beatgrid to the right.
            engine.setParameter(group, "beats_translate_later", value);
        } else if (Hercules4Mx.shiftStatus.pressed && value) { // Shift mode: enable/disable quantize
            engine.setParameter(group, "quantize", !engine.getParameter(group, "quantize"));
            //Tell shift button not to change state.
            Hercules4Mx.shiftStatus.used = true;
        } else if (value) {
            engine.setParameter(group, "pitch_adjust_up_small", value);
            if (Hercules4Mx.editModeStatus.mode !== Hercules4Mx.editModes.disabled) {
                Hercules4Mx.deactivateEditModeAction();
            }
            Hercules4Mx.activateEditModeAction(Hercules4Mx.editModes.pitchkeychanging, '');
            Hercules4Mx.editModeStatus.used = false;
        } else {
            Hercules4Mx.deactivateEditModeAction();
        }
    }
};

// pitch bend minus button is pressed in a deck.
Hercules4Mx.pBendDownButton = function(midichan, control, value, status, group) {
    //preview deck has no rate control
    if (Hercules4Mx.previewOnDeck[group] === false) {
        if (Hercules4Mx.shiftStatus.pressed) { //Shifting: change rate permanently.
            if (value) {
                engine.setValue(group, "rate_perm_down", 1);
                engine.setValue(group, "rate_perm_down", 0);
            }
            //Tell shift button not to change state.
            Hercules4Mx.shiftStatus.used = true;
        } else {
            if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.beatgrid) { //Edit mode. Reduce the BPM.
                engine.setParameter(group, "beats_adjust_slower", value);
            } else { //Normal mode: pitch bend down (change the rate down temporarily)
                engine.setParameter(group, "rate_temp_down", value);
            }
        }
    }
};
// pitch bend plus button is pressed in a deck.
Hercules4Mx.pBendUpButton = function(midichan, control, value, status, group) {
    //preview deck has no rate control
    if (Hercules4Mx.previewOnDeck[group] === false) {
        if (Hercules4Mx.shiftStatus.pressed) { //Shifting: Jump 1 beat forward.
            if (value) {
                engine.setValue(group, "rate_perm_up", 1);
                engine.setValue(group, "rate_perm_up", 0);
            }
            //Tell shift button not to change state.
            Hercules4Mx.shiftStatus.used = true;
        } else {
            if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.beatgrid) { //Edit mode. Increase the BPM
                engine.setParameter(group, "beats_adjust_faster", value);
            } else { //Normal mode: pitch bend up (change the rate up temporarily)
                engine.setParameter(group, "rate_temp_up", value);
            }
        }
    }
};
// Sync button is pressed in a deck.
Hercules4Mx.syncButton = function(midichan, control, value, status, group) {
    //preview deck has no rate control
    if (Hercules4Mx.previewOnDeck[group] === true) {
        return;
    }
    if (Hercules4Mx.shiftStatus.pressed) { //Shifting: Enable beatgrid editing mode.
        if (value) {
            if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.beatgrid) {
                Hercules4Mx.deactivateEditModeAction();
            } else if (Hercules4Mx.editModeStatus.mode !== Hercules4Mx.editModes.disabled) {
                Hercules4Mx.deactivateEditModeAction();
                Hercules4Mx.activateEditModeAction(Hercules4Mx.editModes.beatgrid, -1);
            } else {
                Hercules4Mx.activateEditModeAction(Hercules4Mx.editModes.beatgrid, -1);
            }
            //Tell shift button not to change state.
            Hercules4Mx.shiftStatus.used = true;
        }
    } else if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.beatgrid) { //Edit mode. Align the beatgrid to cursor or to other deck
        if (value) {
            if (engine.getParameter(group, "play") === 1) {
                engine.setParameter(group, "beats_translate_match_alignment", 1);
                engine.setParameter(group, "beats_translate_match_alignment", 0);
            } else {
                engine.setParameter(group, "beats_translate_curpos", 1);
                engine.setParameter(group, "beats_translate_curpos", 0);
            }
        }
    } else {
        if (value) {
            // sync_enabled acts as a switch so we set the opposite of the value that it has.
            var newval;
            if (engine.getValue(group, "sync_enabled") > 0) {
                newval = 0;
            } else {
                newval = 1;
            }
            engine.setValue(group, "sync_enabled", newval);
            if (newval === 1) {
                // if it has been enabled by the above step, start the timer to see if we maintain the value or not when releasing the button.
                if (Hercules4Mx.syncEnabledStatus.timeoutId !== null) {
                    //Safety measure, ensure timer is off
                    engine.stopTimer(Hercules4Mx.syncEnabledStatus.timeoutId);
                }
                //If pressed for 400 ms, activate master sync (sync_enabled), like it happens in the UI.
                Hercules4Mx.syncEnabledStatus.triggered = 0;
                Hercules4Mx.syncEnabledStatus.timeoutId = engine.beginTimer(Hercules4Mx.KeyHoldTime, Hercules4Mx.doSyncHoldAction, true);
            }
        } else {
            if (Hercules4Mx.syncEnabledStatus.timeoutId !== null) {
                if (Hercules4Mx.syncEnabledStatus.triggered === 0 && engine.getValue(group, "sync_enabled") === 1) {
                    // only if the timer has not triggered, disable it
                    engine.stopTimer(Hercules4Mx.syncEnabledStatus.timeoutId);
                    engine.setValue(group, "sync_enabled", 0);
                }
                Hercules4Mx.syncEnabledStatus.timeoutId = null;
            }
        }
    }
};
//Enter into/Exit the edit mode, which changes the functionality of several buttons to edit the beatgrid
Hercules4Mx.activateEditModeAction = function(editMode, effect) {
    if (editMode === Hercules4Mx.editModes.loop) {
        var offset = (effect === 2 || effect === 4) ? 0x20 : 0x0;
        var cccode = (effect > 2) ? Hercules4Mx.NOnC2 : Hercules4Mx.NOnC1;
        midi.sendShortMsg(cccode, 0x41 + offset, 0x7F);
        midi.sendShortMsg(cccode, 0x42 + offset, 0x7F);
        midi.sendShortMsg(cccode, 0x43 + offset, 0x7F);
        midi.sendShortMsg(cccode, 0x44 + offset, 0x7F);
        midi.sendShortMsg(cccode, 0x45 + offset, 0x7F);
        midi.sendShortMsg(cccode, 0x46 + offset, 0x7F);
    } else {
        //Activate blinking led
        for (var i = 1; i <= 4; i++) {
            var midicode = (i > 2) ? Hercules4Mx.NOnC2 : Hercules4Mx.NOnC1;
            var side = (i === 2 || i === 4) ? 0x20 : 0x0;
            //Todo: convert effect led changes to trigger
            switch (editMode) {
                case Hercules4Mx.editModes.beatgrid:
                    engine.trigger("[Channel" + i + "]", "sync_enabled");
                    break;
                case Hercules4Mx.editModes.effects:
                    midi.sendShortMsg(midicode, 0x40 + side + effect, 1);
                    break;
            }
        }
    }
    Hercules4Mx.editModeStatus.mode = editMode;
    Hercules4Mx.editModeStatus.effect = effect;
};
Hercules4Mx.deactivateEditModeAction = function() {
    var effect;
    if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.loop) {
        effect = Hercules4Mx.editModeStatus.effect;
        var offset = (effect === 2 || effect === 4) ? 0x20 : 0x0;
        var cccode = (effect > 2) ? Hercules4Mx.NOnC2 : Hercules4Mx.NOnC1;
        midi.sendShortMsg(cccode, 0x41 + offset, 0x0);
        midi.sendShortMsg(cccode, 0x42 + offset, 0x0);
        midi.sendShortMsg(cccode, 0x43 + offset, 0x0);
        midi.sendShortMsg(cccode, 0x44 + offset, 0x0);
        midi.sendShortMsg(cccode, 0x45 + offset, 0x0);
        midi.sendShortMsg(cccode, 0x46 + offset, 0x0);
    } else {
        effect = Hercules4Mx.editModeStatus.effect;
        //Deactivate blinking led
        for (var i = 1; i <= 4; i++) {
            var midicode = (i > 2) ? Hercules4Mx.NOnC2 : Hercules4Mx.NOnC1;
            var side = (i === 2 || i === 4) ? 0x20 : 0x0;
            //Todo: convert effect led changes to trigger
            switch (Hercules4Mx.editModeStatus.mode) {
                case Hercules4Mx.editModes.beatgrid:
                    engine.trigger("[Channel" + i + "]", "sync_enabled");
                    break;
                case Hercules4Mx.editModes.effects:
                    midi.sendShortMsg(midicode, 0x40 + side + effect, 0);
                    break;
            }
        }
    }
    Hercules4Mx.editModeStatus.mode = Hercules4Mx.editModes.disabled;
    Hercules4Mx.editModeStatus.effect = -1;
};
//Internal timer called sync
Hercules4Mx.doSyncHoldAction = function() {
    Hercules4Mx.syncEnabledStatus.triggered = 1;
};

//Advanced navigation mode. 
Hercules4Mx.navigationFiles = function(midichan, control, value, status, group) {
    if (value) {
        if (Hercules4Mx.navigationStatus.sidebar === false &&
            engine.getParameter("[AutoDJ]", "enabled") === 1) {
            // if autoDJ enabled and we are already at "files", skip next file
            engine.setParameter("[AutoDJ]", "skip_next", 1);
        } else {
            Hercules4Mx.navigationStatus.sidebar = false;
            midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3E, 0x7F);
            midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3F, 0x00);
            //When changing from another library folder, the cursor is not set on the list
            //This forces it to be set and shouldn't affect if it hasn't changed.
            engine.setParameter("[Playlist]", "SelectNextTrack", "1");
            engine.setParameter("[Playlist]", "SelectPrevTrack", "1");
        }
    }
};
Hercules4Mx.navigationFolders = function(midichan, control, value, status, group) {
    if (value) {
        if (Hercules4Mx.navigationStatus.sidebar) {
            //if we are already on sidebar, open/close group
            engine.setParameter('[Playlist]', 'ToggleSelectedSidebarItem', 1);
        }
        Hercules4Mx.navigationStatus.sidebar = true;
        midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3E, 0x00);
        midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3F, 0x7F);
    }
};
Hercules4Mx.navigation = function(midichan, control, value, status, group) {
    if (value) {
        if (control === 0x40) {
            //UP
            Hercules4Mx.navigationStatus.direction = -1;
        } else /*if (control === 0x41)*/ {
            //DOWN
            Hercules4Mx.navigationStatus.direction = 1;
        }
        if (Hercules4Mx.navigationStatus.timeoutId !== null) {
            //Safety measure.
            engine.stopTimer(Hercules4Mx.navigationStatus.timeoutId);
            Hercules4Mx.navigationStatus.timeoutId = null;
        }
        Hercules4Mx.navigationStatus.enabled = true;
        Hercules4Mx.navigationStatus.isSingleShotTimer = false;
        Hercules4Mx.doNavigateAction();
        //Enable key-repeat mode. Cursor will continue moving until button is released.
        Hercules4Mx.navigationStatus.isSingleShotTimer = true;
        Hercules4Mx.navigationStatus.timeoutId = engine.beginTimer(Hercules4Mx.KeyHoldTime, Hercules4Mx.doNavigateAction, true);
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
// Internal timer-called navigate action.
Hercules4Mx.doNavigateAction = function() {
    if (Hercules4Mx.navigationStatus.isSingleShotTimer) {
        //Start the key-repeat timer.
        Hercules4Mx.navigationStatus.timeoutId = engine.beginTimer(Hercules4Mx.userSettings.naviScrollSpeed, Hercules4Mx.doNavigateAction);
        Hercules4Mx.navigationStatus.isSingleShotTimer = false;
    }
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

Hercules4Mx.LoadSelectedTrack = function(midichan, control, value, status, groupInitial) {
    if (value > 0) {
        var group = (Hercules4Mx.previewOnDeck[groupInitial] === true) ? '[PreviewDeck1]' : groupInitial;
        engine.setValue(group, "LoadSelectedTrack", 1);
        engine.setValue(group, "LoadSelectedTrack", 0);
    }
};

// The effect knob has been moved.
Hercules4Mx.effectKnob = function(midichan, control, value, status, group) {
    //group is: [QuickEffectRack1_[Channel1]]
    var fxGroup;
    //It has a speed sensor, but you have to move it really fast for it to send something different.
    var direction = (value < 0x40) ? value : value - 0x80;
    //The effect knob granularity is very coarse, so we compensate it here so that it behaves like an analog one.
    var step = 1 / 32;
    if (Hercules4Mx.shiftStatus.pressed) {
        //If pressing shift, let's move it slowly.
        step = 1 / 127;
        //Tell shift button not to change state.
        Hercules4Mx.shiftStatus.used = true;
    }
    if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.loopsizing) {
        fxGroup = "[Channel" + group.slice(-3).substr(0, 1) + "]";
        var action = (direction > 0) ? "loop_double" : "loop_halve";
        engine.setValue(fxGroup, action, 1);
        //"Releasing" the button.
        engine.setValue(fxGroup, action, 0);
        Hercules4Mx.editModeStatus.used = true;
    } else if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.loop) {
        fxGroup = "[Channel" + group.slice(-3).substr(0, 1) + "]";
        engine.setValue(fxGroup, "loop_move", direction);
        Hercules4Mx.editModeStatus.used = true;
    } else if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.effectknob) {
        fxGroup = "[EffectRack1_EffectUnit" + Hercules4Mx.editModeStatus.effect + "]";
        engine.setParameter(fxGroup, "super1", engine.getParameter(fxGroup, "super1") + step * direction);
        Hercules4Mx.editModeStatus.used = true;
    } else if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.beatgrid) {
        var Fxgroup = "[Channel" + group.slice(-3).substr(0, 1) + "]";
        if (direction < 0) {
            engine.setParameter(Fxgroup, "beats_translate_earlier", direction * -1);
        } else {
            engine.setParameter(Fxgroup, "beats_translate_later", direction);
        }
    } else if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.pitchkeychanging) {
        fxGroup = "[Channel" + group.slice(-3).substr(0, 1) + "]";
        if (direction > 0 ) {
            engine.setParameter(fxGroup, "pitch_adjust_up_small", 1);
            engine.setParameter(fxGroup, "pitch_adjust_up_small", 0);
        } else {
            engine.setParameter(fxGroup, "pitch_adjust_down_small", 1);
            engine.setParameter(fxGroup, "pitch_adjust_down_small", 0);
        }
    } else {
        var val =  engine.getParameter(group, "super1") + step * direction;
        if ( Hercules4Mx.shiftStatus.used === false) {
            //Let's round it properly.
            if (direction > 0 && val > 0.5 && val - step < 0.5) {
                val = 0.5;
            }
            else if (direction < 0 && val < 0.5 && val + step > 0.5) {
                val = 0.5;
            }
        }
        engine.setParameter(group, "super1", val);
    }
};
//This is used in conjunction with the keypad button mapping. It's the default "no-operation" action.
Hercules4Mx.FxActionNoOp = function(group, fxbutton, value, extraparam) {
    if (Hercules4Mx.debuglog) {
        engine.log("entering Hercules4Mx.FxActionNoOp");
    }
};
//This is used in conjunction with the keypad button mapping. A keypad button has been pushed
Hercules4Mx.buttonPush = function(group, fxbutton, value, extraparam) {
    if (Hercules4Mx.debuglog) {
        engine.log("entering Hercules4Mx.buttonPush");
    }
    engine.setValue(group, extraparam, value);
};
//This is used in conjunction with the keypad button mapping. A keypad button for an audio effect has been pushed
Hercules4Mx.FxSwitchDown = function(group, fxbutton, value, extraparam) {
    if (Hercules4Mx.debuglog) {
        engine.log("entering Hercules4Mx.FxSwitchDown");
    }
    if (Hercules4Mx.editModeStatus.mode !== Hercules4Mx.editModes.disabled) {
        Hercules4Mx.deactivateEditModeAction();
    }
    Hercules4Mx.activateEditModeAction(Hercules4Mx.editModes.effectknob, extraparam);
    Hercules4Mx.editModeStatus.used = false;
};
//This is used in conjunction with the keypad button mapping. A keypad button for an audio effect has been released
Hercules4Mx.FxSwitchUp = function(group, fxbutton, value, extraparam) {
    if (Hercules4Mx.debuglog) {
        engine.log("entering Hercules4Mx.FxSwitchUp");
    }
    if (Hercules4Mx.editModeStatus.used === false) {
        var deck = script.deckFromGroup(group);
        var state = engine.getParameter("[EffectRack1_EffectUnit" + extraparam + "]", "group_[Channel" + deck + "]_enable");
        engine.setParameter("[EffectRack1_EffectUnit" + extraparam + "]", "group_[Channel" + deck + "]_enable", !state);
    }
    Hercules4Mx.deactivateEditModeAction();
};
//This is used in conjunction with the keypad button mapping. A keypad button for an audio effect has been released
Hercules4Mx.FxSamplerPush = function(group, fxbutton, value, extraparam) {
    if (Hercules4Mx.debuglog) {
        engine.log("entering Hercules4Mx.FxSamplerPush");
    }
    var deck = script.deckFromGroup(group);
    //Since the sampler does not depend on the deck, buttons on deck
    // A/C are for samples 1/2 and buttons on deck B/D are for samples 3/4.
    var newgroup;
    if (deck === 1 || deck === 3) {
        newgroup = "[Sampler" + extraparam + "]";
    }
    if (deck === 2 || deck === 4) {
        newgroup = "[Sampler" + (2 + parseInt(extraparam)) + "]";
    }
    if (engine.getValue(newgroup, "play")) {
        engine.setValue(newgroup, "start_stop", 1);
    } else {
        engine.setValue(newgroup, "start_play", 1);
    }
};
/*
 button 1: loop start and loop editing functionality -> 
        loop enabled and is not one of the two preconfigured: button led is on
        click: sets loop start (all 6 buttons start blinking). 
            If loop was already enabled, moves the start to the current position but does not disable the playing loop
        click again on button 1: forget start pos (discard loop) (stops blinking and no led is on).
        click on button 2: set loop end and enable looping (stops blinking and sets led on buttons 1 and 2 to on).
        click on any of the other 4 buttons: set loop to 2, 8, 16, 32 beats (user configurable)(stops blinking and sets led on button 1 to on).
        click with shift pressed:  Same as clic, but this will be a rolling loop
        click when a loop is present and active: release loop
        button held down while loop is active and move knob: double or halve the loop. 
        If the knob is moved while the loop edit mode is active, then move the loop forward or backward.

 button 2: loop end, reloop functionality ->
        loop present: button led is on.
        click without a loop present: nothing
        click with a loop present but not active: activate the loop (reloop)
        click with shift pressed with a loop present but not active: Same as clic, but this will be a rolling loop
        click when a loop is present and active: release loop
        click when loop editing has been enabled(see button 1): set loop end and enable looping.

 buttons 3 and 4: predefined beat loops  -> (user configurable which two. by default 1 and 4 beats)
        click when this beatloop is not active: set a loop with specified beats and enable it. (button led is set to on).
        click when this beatloop is active: release loop.
        click with shift pressed:  Same as clic, but this will be a rolling loop

 buttons 3 to 6: additional beat loops ->
        click when loop editing has been enabled (see button 1): set beatloop of defined size and enable looping.

*/
Hercules4Mx.LoopEditPress = function(group, fxbutton, value, extraparam) {
    if (Hercules4Mx.debuglog) {
        engine.log("entering Hercules4Mx.LoopEditPress");
    }
    if (Hercules4Mx.editModeStatus.mode !== Hercules4Mx.editModes.disabled) {
        Hercules4Mx.deactivateEditModeAction();
    }
    Hercules4Mx.activateEditModeAction(Hercules4Mx.editModes.loopsizing, '');
    Hercules4Mx.editModeStatus.used = false;
};
Hercules4Mx.LoopEditRelease = function(group, fxbutton, value, extraparam) {
    if (Hercules4Mx.debuglog) {
        engine.log("entering Hercules4Mx.LoopEditRelease");
    }
    if (Hercules4Mx.editModeStatus.used === false) {
        var deck = script.deckFromGroup(group);
        if (engine.getValue(group, "loop_enabled") === 0) {
            if (Hercules4Mx.editModeStatus.mode !== Hercules4Mx.editModes.disabled) {
                Hercules4Mx.deactivateEditModeAction();
            }
            Hercules4Mx.activateEditModeAction(Hercules4Mx.editModes.loop, deck);
            var splitted = extraparam.split(";");
            if (splitted[0] === "roll") {
                engine.setValue(group, "slip_enabled", 1);
            } else {
                engine.setValue(group, "slip_enabled", 0);
            }
            engine.setValue(group, "loop_in", 1);
            //"Releasing" the button.
            engine.setValue(group, "loop_in", 0);
        } else {
            engine.setValue(group, "reloop_exit", 1);
        }
    } else {
        Hercules4Mx.deactivateEditModeAction();
    }
};
Hercules4Mx.LoopEditComplete = function(group, button) {
    if (Hercules4Mx.debuglog) {
        engine.log("Hercules4Mx.LoopEditComplete");
    }
    switch (button) {
        case 1:
            engine.setValue(group, "slip_enabled", 0);
            break;
        case 2:
            {
                engine.setValue(group, "loop_out", 1);
                //"Releasing" the button.
                engine.setValue(group, "loop_out", 0);
                break;
            }
        case 3:
            engine.setValue(group, Hercules4Mx.beatLoopEditButtons[0], 1);
            break;
        case 4:
            engine.setValue(group, Hercules4Mx.beatLoopEditButtons[1], 1);
            break;
        case 5:
            engine.setValue(group, Hercules4Mx.beatLoopEditButtons[2], 1);
            break;
        case 6:
            engine.setValue(group, Hercules4Mx.beatLoopEditButtons[3], 1);
            break;
    }
};
Hercules4Mx.LoopButtonPush = function(group, fxbutton, value, extraparam) {
    if (Hercules4Mx.debuglog) {
        engine.log("Hercules4Mx.LoopButtonPush");
    }
    var splitted = extraparam.split(";");
    if (splitted[0] === "roll" && splitted.length === 2) {
        if (engine.getValue(group, "loop_enabled") === 0 && value > 0) {
            engine.setValue(group, "slip_enabled", 1);
        }
        Hercules4Mx.buttonPush(group, fxbutton, value, splitted[1]);
    } else if (splitted.length === 1) {
        engine.setValue(group, "slip_enabled", 0);
        Hercules4Mx.buttonPush(group, fxbutton, value, extraparam);
    }
};

// Any of the FX buttons has been pressed
// There are 6 physical buttons present per deck, and also a "shift" button used by the controller 
// itself to switch between messages 1 to 6 and messages 7 to 12, depending if it is enabled or not.
// Since the shift button also sends a message when it is pressed and when it is released,
// I've been able to setup up to 24 different actions per deck.
// I call these additional 12 messages the "shift-pressed" mode.
// Some buttons have default actions if the button is maintained pressed (like play a hotcue), and others
// have additional functionality (like editing the loop length , or using the effect superknob)
Hercules4Mx.FXButton = function(midichan, control, value, status, groupInitial) {
    var group = (Hercules4Mx.previewOnDeck[groupInitial] === true) ? '[PreviewDeck1]' : groupInitial;
    var fxbutton = (control > 0x20) ? control - 0x20 : control;
    if (Hercules4Mx.shiftStatus.pressed) {
        //Tell shift not to change state.
        Hercules4Mx.shiftStatus.used = true;
        fxbutton = fxbutton + 0x0C;
    }
    // the array is zero based.
    var mapping = Hercules4Mx.buttonMappings[fxbutton - 1];
    var deck = script.deckFromGroup(group);
    if (Hercules4Mx.editModeStatus.mode === Hercules4Mx.editModes.loop &&
        Hercules4Mx.editModeStatus.effect === deck) {
        if (value) {
            //truncate the button pressed to the 6 physical positions.
            Hercules4Mx.LoopEditComplete(group, ((fxbutton - 1) % 6) + 1);
        } else {
            Hercules4Mx.deactivateEditModeAction();
        }
    } else if (value) {
        mapping.buttonPressAction(group, fxbutton, 1, mapping.extraParameter);
    } else if (mapping.buttonReleaseAction !== null) {
        mapping.buttonReleaseAction(group, fxbutton, 0, mapping.extraParameter);
    }
};

//Jog wheel moved without pressure (for seeking, speeding or slowing down, or navigating)
Hercules4Mx.jogWheel = function(midichan, control, value, status, groupInitial) {
    //It has a speed sensor, but you have to move it really fast for it to send something different.
    var direction = (value < 0x40) ? value : value - 0x80;
    if (Hercules4Mx.navigationStatus.enabled) {
        if (Hercules4Mx.navigationStatus.timeoutId !== null) {
            //Stop key-repeat mode. From now on, obey only jog movement until button is released.
            engine.stopTimer(Hercules4Mx.navigationStatus.timeoutId);
            Hercules4Mx.navigationStatus.timeoutId = null;
        }
        Hercules4Mx.navigationStatus.direction = direction;
        Hercules4Mx.doNavigateAction();
    } else {
        var group = (Hercules4Mx.previewOnDeck[groupInitial]) ? '[PreviewDeck1]' : groupInitial;
        engine.setValue(group, "jog", direction + engine.getValue(group, "jog"));
    }
};

// The "scratch" button is used to enable or disable scratching on the jog wheels.
// Concretely, it tells if it has to ignore or not the pressure sensor in the jog wheel.
Hercules4Mx.scratchButton = function(midichan, control, value, status, group) {
    if (value) {
        if (Hercules4Mx.scratchEnabled) {
            Hercules4Mx.scratchEnabled = false;
            midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3D, 0x00);
            if (Hercules4Mx.userSettings.crossfaderScratchCurve) {
                midi.sendShortMsg(Hercules4Mx.CC, 0x7E, 0x00);
            }
        } else {
            Hercules4Mx.scratchEnabled = true;
            midi.sendShortMsg(Hercules4Mx.NOnC1, 0x3D, 0x7F);
            if (Hercules4Mx.userSettings.crossfaderScratchCurve) {
                midi.sendShortMsg(Hercules4Mx.CC, 0x7E, 0x7F);
            }
        }
    }
};
// The pressure action over the jog wheel
Hercules4Mx.wheelTouch = function(midichan, control, value, status, groupInitial) {
    //Scratching does not work with the previewdeck.
    if (Hercules4Mx.previewOnDeck[groupInitial] === false) {
        var group = (Hercules4Mx.previewOnDeck[groupInitial]) ? '[PreviewDeck1]' : groupInitial;
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
    }
};
//Jog wheel used with pressure (for scratching)
Hercules4Mx.scratchWheel = function(midichan, control, value, status, groupInitial) {
    if (Hercules4Mx.navigationStatus.enabled ||
        !engine.isScratching(script.deckFromGroup(groupInitial))) {
        //If navigating, or not in scratch mode, do jogWheel
        Hercules4Mx.jogWheel(midichan, control, value, status, groupInitial);
    } else {
        //It has a speed sensor, but you have to move it really fast for it to send something different.
        var direction = (value < 0x40) ? value : value - 0x80;
		var group = (Hercules4Mx.previewOnDeck[groupInitial]) ? '[PreviewDeck1]' : groupInitial;
        engine.scratchTick(script.deckFromGroup(group), direction);
    }
};

// Pitch slider rate change, MSB (Most significant bits in 14bit mode, or directly the value in 7bit)
Hercules4Mx.deckRateMsb = function(midichan, control, value, status, group) {
    //preview deck has no rate control
    if (Hercules4Mx.previewOnDeck[group] === false) {
        var deck = script.deckFromGroup(group);
        //Calculating this always, or else the first time will not work
        //(which is precisely when the controller reports the initial positions)
        Hercules4Mx.pitchMsbValue[deck - 1] = value;
        if (Hercules4Mx.pitch14bitMode === false) {
            engine.setValue(group, "rate", script.midiPitch(0,value, 0xE0));
        }
    }
};
// Pitch slider rate change, LSB (Least significant bits in 14bit mode, not called in 7bit)
Hercules4Mx.deckRateLsb = function(midichan, control, value, status, group) {
    //preview deck has no rate control
    if (Hercules4Mx.previewOnDeck[group] === false) {
        var deck = script.deckFromGroup(group);
        var msbval = Hercules4Mx.pitchMsbValue[deck - 1];
        Hercules4Mx.pitch14bitMode = true;
        engine.setValue(group, "rate", script.midiPitch(value,msbval,0xE0));
    }
};

//These are mapped with javascript so that engine.softTakeover can be enabled programatically.
//If we could use the setParameter() method, we wouldn't need the 
//absoluteNonLin or faderToVolume, but setParameter currently does not work with softTakeover.
//TODO: Since version 2.1, setParameter works. Simplify the code.
Hercules4Mx.deckGain = function(midichan, control, value, status, groupInitial) {
    var group = (Hercules4Mx.previewOnDeck[groupInitial] === true) ? '[PreviewDeck1]' : groupInitial;
    engine.setValue(group, "pregain", script.absoluteNonLin(value, 0, 1, 4));
};
Hercules4Mx.deckTreble = function(midichan, control, value, status, group) {
    //[EqualizerRack1_[Channel1]_Effect1]
    var groupChannel = "[Channel" + group.slice(-11).substr(0,1) + "]";
    if (Hercules4Mx.previewOnDeck[groupChannel] === false) {
        engine.setValue(group, "parameter3", script.absoluteNonLin(value, 0, 1, 4));
    }
};
Hercules4Mx.deckMids = function(midichan, control, value, status, group) {
    //[EqualizerRack1_[Channel1]_Effect1]
    var groupChannel = "[Channel" + group.slice(-11).substr(0,1) + "]";
    if (Hercules4Mx.previewOnDeck[groupChannel] === false) {
        engine.setValue(group, "parameter2", script.absoluteNonLin(value, 0, 1, 4));
    }
};
Hercules4Mx.deckBass = function(midichan, control, value, status, group) {
    //[EqualizerRack1_[Channel1]_Effect1]
    var groupChannel = "[Channel" + group.slice(-11).substr(0,1) + "]";
    if (Hercules4Mx.previewOnDeck[groupChannel] === false) {
        engine.setValue(group, "parameter1", script.absoluteNonLin(value, 0, 1, 4));
    }
};
Hercules4Mx.deckVolume = function(midichan, control, value, status, group) {
    if (Hercules4Mx.previewOnDeck[group] === false) {
        engine.setValue(group, "volume", Hercules4Mx.faderToVolume(value));
    }
};
// function to scale fader volume linearly in dB until the second-to-last line, and
// do the rest linearly. 0dBFs -6dbFs, -12dbFs,-18dbFs, -inf). Just like what the UI does.
// Neither the script.absoluteLin nor script.absoluteNonLin can do this.
Hercules4Mx.faderToVolume = function (value) {
    var lowerval = 32; //(1/4th of 127)
    var lowerdb = 0.125; //(1/4th of 1)
    if (value < lowerval) {
        return value*lowerdb/lowerval;
    } else {
        var dbs = -(127-value)*6/32;
		return Math.pow(10.0,dbs/20.0);
    }
};

Hercules4Mx.crossfader = function(midichan, control, value, status, group) {
    engine.setValue(group, "crossfader", script.absoluteLin(value, -1, 1));
};

Hercules4Mx.deckheadphones = function(midichan, control, value, status, group) {
     if (value > 0) {
        var deck = script.deckFromGroup(group);
        var messageto = (deck === 1 || deck === 2) ? Hercules4Mx.NOnC1 : Hercules4Mx.NOnC2;
        var offset = (deck === 1 || deck === 3) ? 0x00 : 0x20;
        if (Hercules4Mx.previewOnDeck[group]) {
            //No need to press shift to deactivate.
            if (Hercules4Mx.shiftStatus.pressed) {
                Hercules4Mx.shiftStatus.used = true;
            }
            Hercules4Mx.previewOnDeck[group] = false;
            midi.sendShortMsg(messageto, 0x4F+offset, 0x00);
        } else {
            if (Hercules4Mx.shiftStatus.pressed) {
                Hercules4Mx.shiftStatus.used = true;
                Hercules4Mx.previewOnDeck[group] = true;
                midi.sendShortMsg(messageto, 0x4F+offset, 0x7F);
            } else {
                engine.setParameter(group, "pfl", (engine.getParameter(group, "pfl") > 0) ? 0 : 1);
            }
        }
    }
};
// Deck C/D have been pressed.
Hercules4Mx.deckCStateChange = function(midichan, control, value, status, group) {
    Hercules4Mx.VuMeterL.midichan = (value > 0) ? 0x91 : 0x90;
    Hercules4Mx.updateVumeterSourceAction(Hercules4Mx.VuMeterL, (value > 0) ? 3 : 1);
};
Hercules4Mx.deckDStateChange = function(midichan, control, value, status, group) {
    Hercules4Mx.VuMeterR.midichan = (value > 0) ? 0x91 : 0x90;
    Hercules4Mx.updateVumeterSourceAction(Hercules4Mx.VuMeterR, (value > 0) ? 4 : 2);
};

// Internal select what data is sent to the vumeter
Hercules4Mx.updateVumeterSourceAction = function(vumeter, deck) {
    var returnarray;
    if (vumeter.midichan === 0x90) {
        returnarray = Hercules4Mx.getNewDestinationChannel(vumeter.initchan);
    } else /*if (vumeter.midichan === 0x91)*/ {
        returnarray = Hercules4Mx.getNewDestinationChannel(vumeter.initchan + 2);
    }
    vumeter.source = returnarray[0];
    if (vumeter.source === "[Disabled]") {
        //If disabled, we have to ensure the kill leds are properly shown.
        midi.sendShortMsg(vumeter.midichan, vumeter.clip, returnarray[2] > 0 ? 0x7F : 0x00);
        midi.sendShortMsg(vumeter.midichan, vumeter.vu3, returnarray[3] > 0 ? 0x7F : 0x00);
        midi.sendShortMsg(vumeter.midichan, vumeter.vu2, returnarray[4] > 0 ? 0x7F : 0x00);
        midi.sendShortMsg(vumeter.midichan, vumeter.vu1, returnarray[5] > 0 ? 0x7F : 0x00);
    } else {
        //Forcing a refresh, because if the vumeter is at 0, Mixxx doesn't send the event.
        vumeter.lastvalue = 0xFF;
        Hercules4Mx.updateVumeterEvent(vumeter, 0);
    }
};
//Internal function that returns what the vumeter should show for this deck
Hercules4Mx.getNewDestinationChannel = function(chan) {
    var pfl = engine.getParameter("[Channel" + chan + "]", "pfl");
    var source = engine.getParameter("[Channel" + chan + "]", "passthrough");
    var treble = engine.getParameter("[EqualizerRack1_[Channel" + chan + "]_Effect1]", "button_parameter3");
    var mids = engine.getParameter("[EqualizerRack1_[Channel" + chan + "]_Effect1]", "button_parameter2");
    var bass = engine.getParameter("[EqualizerRack1_[Channel" + chan + "]_Effect1]", "button_parameter1");
    var returnarray = ["", pfl, source, treble, mids, bass];
    if (bass > 0 || mids > 0 || treble > 0 || source > 0) {
        returnarray[0] = "[Disabled]";
    } else if (pfl > 0) {
        returnarray[0] = "[Channel" + chan + "]";
    } else {
        returnarray[0] = "[Master]";
    }
    return returnarray;
};

//////////////////////////////////////////
// FX button presets configuration.
Hercules4Mx.reinitButtonsMap = function() {
    Hercules4Mx.buttonMappings = Hercules4Mx.buttonMappings.slice(Hercules4Mx.buttonMappings.length);
};
Hercules4Mx.mapNewButton = function(pushAction, releaseAction, extraParameter, signalLed) {
    var i;
    var mapping = {};
    mapping.buttonPressAction = pushAction;
    mapping.buttonReleaseAction = releaseAction;
    mapping.extraParameter = extraParameter;
    mapping.ledToConnect = signalLed;
    Hercules4Mx.buttonMappings.push(mapping);
    if (Hercules4Mx.buttonMappings.length <= 12) { // The "shift" buttons do not have a differentiated led.
        if (extraParameter === "reloop_exit") {
            Hercules4Mx.beatLoopReloopPos = Hercules4Mx.buttonMappings.length;
        }
        if (pushAction === Hercules4Mx.LoopEditPress) {
            Hercules4Mx.LoopEnabledPos = Hercules4Mx.buttonMappings.length;
        }
        if (signalLed !== null) {
            for (i = 1; i <= 4; i++) {
                engine.connectControl("[Channel" + i + "]", signalLed, "Hercules4Mx.onEnableLed");
                engine.trigger("[Channel" + i + "]", signalLed);
            }
        }
        if (pushAction === Hercules4Mx.FxSamplerPush) {
            var curlen = Hercules4Mx.buttonMappings.length;
            Hercules4Mx.samplerLedIdx.push(curlen);
            Hercules4Mx.samplerLedIdx.push(0x20 + curlen);

            engine.connectControl("[Sampler" + extraParameter + "]", "play_indicator", "Hercules4Mx.onSamplerStateChange");
            engine.connectControl("[Sampler" + (2 + parseInt(extraParameter)) + "]", "play_indicator", "Hercules4Mx.onSamplerStateChange");
            engine.trigger("[Sampler" + extraParameter + "]", "play_indicator");
            engine.trigger("[Sampler" + (2 + parseInt(extraParameter)) + "]", "play_indicator");
        }
        if (pushAction === Hercules4Mx.FxSwitchDown) {
            Hercules4Mx.FxLedIdx.push(Hercules4Mx.buttonMappings.length);
            for (i = 1; i <= 4; i++) {
                engine.connectControl("[EffectRack1_EffectUnit" + extraParameter + "]", "group_[Channel" + i + "]_enable", "Hercules4Mx.onEffectStateChange");
                engine.trigger("[EffectRack1_EffectUnit" + extraParameter + "]", "group_[Channel" + i + "]_enable");
            }
        }
    }
};
// This is used when not all buttons have been mapped. To avoid filling the rest manually with the noop action, this is called to do it automatically.
Hercules4Mx.completeButtonsMap = function() {
    while (Hercules4Mx.buttonMappings.length < 24) {
        Hercules4Mx.buttonMappings.push(Hercules4Mx.noActionButtonMap);
    }
    if (Hercules4Mx.LoopEnabledPos !== -1 || Hercules4Mx.beatLoopReloopPos !== -1) {
        for (var i = 1; i <= 4; i++) {
            engine.trigger("[Channel" + i + "]", "loop_enabled");
        }
    }
};

//Mappings of the buttons as described in the DJ Console Manual for Virtual DJ 7 LE.
Hercules4Mx.setupFXButtonsLikeManual = function() {
    Hercules4Mx.reinitButtonsMap();
    Hercules4Mx.mapNewButton(Hercules4Mx.LoopEditPress, Hercules4Mx.LoopEditRelease, null, null);
    Hercules4Mx.mapNewButton(Hercules4Mx.ButtonPush, null, "reloop_exit", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "hotcue_1_activate", "hotcue_1_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "hotcue_2_activate", "hotcue_2_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "hotcue_3_activate", "hotcue_3_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "hotcue_4_activate", "hotcue_4_enabled");
    //In VirtualDJ there is a recorder, which is not an option in Mixxx. So we simply play the samplers.
    //Note: the implementation doubles the number of samples: Buttons on left deck use the value here, and buttons on right deck use "2 + value".
    Hercules4Mx.mapNewButton(Hercules4Mx.FxSamplerPush, null, "1", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.FxSamplerPush, null, "2", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.FxSwitchDown, Hercules4Mx.FxSwitchUp, "1", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.FxSwitchDown, Hercules4Mx.FxSwitchUp, "2", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.FxSwitchDown, Hercules4Mx.FxSwitchUp, "3", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.FxSwitchDown, Hercules4Mx.FxSwitchUp, "4", null);
    //Shift-pressed actions. These do not exist on the original mapping, but having the hotcue clear options here is useful.
    Hercules4Mx.mapNewButton(Hercules4Mx.FxActionNoOp, null, null, null);
    Hercules4Mx.mapNewButton(Hercules4Mx.FxActionNoOp, null, null, null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "hotcue_1_clear", null, null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "hotcue_2_clear", null, null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "hotcue_3_clear", null, null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "hotcue_4_clear", null, null);

    Hercules4Mx.completeButtonsMap();
    // It is possible to configure the loop sizes of the buttons 3 to 6 when the loop editing mode has been activated.
    // (activated by the "LoopEditPress" action, which is button 1 on original and mixxx21 mappings).
    Hercules4Mx.beatLoopEditButtons.push("beatloop_1_toggle"); // loop edit mode, button 3
    Hercules4Mx.beatLoopEditButtons.push("beatloop_4_toggle"); // loop edit mode, button 4
    Hercules4Mx.beatLoopEditButtons.push("beatloop_8_toggle"); // loop edit mode, button 5
    Hercules4Mx.beatLoopEditButtons.push("beatloop_16_toggle"); // loop edit mode, button 6
};

//Custom mapping adapted to the features of Mixxx. Version 2015-12-19 as published in 2.0.
Hercules4Mx.setupFXButtonsCustomMixx20 = function() {
    var beatloop1 = "beatloop_0.5";
    var beatloop2 = "beatloop_1";
    var beatloop3 = "beatloop_2";
    var beatloop4 = "beatloop_4";
    Hercules4Mx.reinitButtonsMap();
    //Direct actions
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, beatloop1 + "_toggle", beatloop1 + "_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, beatloop2 + "_toggle", beatloop2 + "_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, beatloop3 + "_toggle", beatloop3 + "_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, beatloop4 + "_toggle", beatloop4 + "_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "reverse", "reverse");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "reverseroll", "reverseroll");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "hotcue_1_activate", "hotcue_1_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "hotcue_2_activate", "hotcue_2_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "hotcue_3_activate", "hotcue_3_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "hotcue_4_activate", "hotcue_4_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.FxSwitchDown, Hercules4Mx.FxSwitchUp, "1", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.FxSwitchDown, Hercules4Mx.FxSwitchUp, "2", null);
    //Shift-pressed actions
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "beatloop_0.125_toggle", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "beatloop_0.25_toggle", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "reloop_exit", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "reloop_exit", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.FxActionNoOp, null, null, null);
    Hercules4Mx.mapNewButton(Hercules4Mx.FxActionNoOp, null, null, null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "hotcue_1_clear", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "hotcue_2_clear", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "hotcue_3_clear", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "hotcue_4_clear", null);
    Hercules4Mx.completeButtonsMap();
};

//Custom mapping adapted to the features of Mixxx. Version 2016-08-xx for publishing in 2.1.
Hercules4Mx.setupFXButtonsCustomMixx21 = function() {
    var beatloop1 = "4";
    var beatloop2 = "16";
    Hercules4Mx.reinitButtonsMap();
    //Direct actions
    Hercules4Mx.mapNewButton(Hercules4Mx.LoopEditPress, Hercules4Mx.LoopEditRelease, "", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.LoopButtonPush, null, "reloop_exit", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.LoopButtonPush, null, "beatloop_" + beatloop1 + "_toggle", "beatloop_" + beatloop1 + "_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.LoopButtonPush, null, "beatloop_" + beatloop2 + "_toggle", "beatloop_" + beatloop2 + "_enabled");
    //Note: the implementation doubles the number of samples: Buttons on left deck use the value here, and buttons on right deck use "2 + value".
    Hercules4Mx.mapNewButton(Hercules4Mx.FxSamplerPush, null, "1", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.FxSamplerPush, null, "2", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "hotcue_1_activate", "hotcue_1_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "hotcue_2_activate", "hotcue_2_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "hotcue_3_activate", "hotcue_3_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, Hercules4Mx.buttonPush, "hotcue_4_activate", "hotcue_4_enabled");
    Hercules4Mx.mapNewButton(Hercules4Mx.FxSwitchDown, Hercules4Mx.FxSwitchUp, "1", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.FxSwitchDown, Hercules4Mx.FxSwitchUp, "2", null);
    //Shift-pressed actions
    Hercules4Mx.mapNewButton(Hercules4Mx.LoopEditPress, Hercules4Mx.LoopEditRelease, "roll", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.LoopButtonPush, null, "roll;reloop_exit", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.LoopButtonPush, null, "roll;beatloop_" + beatloop1 + "_toggle", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.LoopButtonPush, null, "roll;beatloop_" + beatloop2 + "_toggle", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.FxActionNoOp, null, null, null);
    Hercules4Mx.mapNewButton(Hercules4Mx.FxActionNoOp, null, null, null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "hotcue_1_clear", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "hotcue_2_clear", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "hotcue_3_clear", null);
    Hercules4Mx.mapNewButton(Hercules4Mx.buttonPush, null, "hotcue_4_clear", null);
    Hercules4Mx.completeButtonsMap();
    // It is possible to configure the loop sizes of the buttons 3 to 6 when the loop editing mode has been activated.
    // (activated by the "LoopEditPress" action, which is button 1 on original and mixxx21 mappings).
    Hercules4Mx.beatLoopEditButtons.push("beatloop_1_toggle"); // loop edit mode, button 3
    Hercules4Mx.beatLoopEditButtons.push("beatloop_2_toggle"); // loop edit mode, button 4
    Hercules4Mx.beatLoopEditButtons.push("beatloop_8_toggle"); // loop edit mode, button 5
    Hercules4Mx.beatLoopEditButtons.push("beatloop_32_toggle"); // loop edit mode, button 6
};

