// * Mixxx mapping script file for the Hercules DJControl Jogvision.
// * Author: DJ Phatso (contributions by Kerrick Staley, and fully rewritten, completed and enhanced by David TV)
// * Forum: https://www.mixxx.org/forums/viewtopic.php?f=7&t=12580
// * Wiki: https://www.mixxx.org/wiki/doku.php/hercules_dj_control_jogvision
// * Version: 1.25 (see changelog at https://github.com/mixxxdj/mixxx/pull/2370)

// EXTRAS ADDED TO ORIGINAL MAPPING
// - MODE+Loop ON                  : set a loop_in mark (with currently defined loop_size), activate it, and enable slip mode
// - MODE+Loop X 1/2 / X 2         : do a 'beatjump_size' beats beatjump backward/forward
// - MODE+Loop Size Knob           : decrease/increase pitch (only key, not tempo!)
// - MODE+JogWheel plate (playing) : scratch with 'Slip' ON (deactivate 'Slip' when plate is released)
// - MODE+JogWheel plate (stopped) : move song position backward/forward faster by 'quickMoveFactor' factor
// - MODE+Browser Knob Turn        : move library selected position in groups of 'quickBrowseFactor' elements forward/backward
// - MODE+LOAD A|B                 : toggle 'quantize' for deck where MODE key is pressed
// - SHIFT+LOAD A|B                : eject track from deck where SHIFT key is pressed
// - SHIFT+Browser Knob Press      : activate (double-click) currently selected item in browser
// - SHIFT+Loop Size Knob          : move existing loop forward/backward
// - SHIFT+JogWheelTouch           : do a 'backspin' with 'spinBackBrakeFactor' and 'spinBackInitialSpeed' factors
// - SHIFT+MultiFX                 : set beatgrid to current position
// - SHIFT+Aircontrol Filter       : do the reverse than standard, that is, high-pass filter

// ----------------------------------------------------------------------------
// CONFIGURABLE MAPPING PARAMETERS
var CFG = {
    // USER Variables (you may modify them)
    "user": {
        "beatDetection": "normal", // normal|follow (follow=song position is more accurate with beat position, but beat detection is much less accurate)
        "initUpdateEffects": 0.8,  // [0..1] (if value > 0, set some effects values at startup with given float value)
        "beatHelper": 1,           // 0|1 (0=disabled; 1=use outer jog leg to indicate where to slide the pitch controller; see also 'CFG.fine.beatHelpSensitivity' variable)
        "beatActiveMode": "follow" // normal|reverse|blink|follow|fill|bounce|alternate|off|on (see table below)
    },
    // FINE TUNING Variables (you may modify them, but not too far from the default values...)
    "fine": {
        "beatHelpSensitivity": 0.5, // [0..1)    Max distance (float) in BPMs to be considered as a match (in use if CFG.user.beatHelper=1): the lower, the more sensitive. Values equal or bigger than 1 are reset to 0.9 and a warning is printed
        "quickMoveFactor": 0.002,   // [0..1]    the smaller (float), the slower MODE+JogWheel will move 'playposition' (when such channel is NOT playing)
        "quickBrowseFactor": 10,    // [0..inf]  the bigger, the faster MODE+Browser jog will move the cursor position in the library up/down
        "spinBackBrakeFactor": 100, // (0..5000] the bigger, the softer the brake will be applied (0 = immediate stop; >=5000 = it will take almost forever to stop)
        "spinBackInitialSpeed": 6,  // (0..200]  the bigger, the stronger will be the "back" impulse (1 = no spinbak, but stop and start sloooowly)
        "mixGainFactor": 0.1        // (0..1)    (float) the bigger, the faster the Pregain or Mix level will be updated
    }
};

// BEATS leds lightning pattern depending on 'beatActiveMode', based on a 4x4 compass:
//
//     beat number     |    1     |     2     |     3     |     4     |     5     |     6     |     7     |     8     |
// --------------------------------------------------------------------------------------------------------------------
//  led (#=ON; ·=OFF)  | 1 2 3 4  |  1 2 3 4  |  1 2 3 4  |  1 2 3 4  |  1 2 3 4  |  1 2 3 4  |  1 2 3 4  |  1 2 3 4  |
// --------------------------------------------------------------------------------------------------------------------
//         | normal    | # · · ·  |  · # · ·  |  · · # ·  |  · · · #  |  # · · ·  |  · # · ·  |  · · # ·  |  · · · #  |
//         |-----------------------------------------------------------------------------------------------------------
//         | reverse   | · # # #  |  # · # #  |  # # · #  |  # # # ·  |  · # # #  |  # · # #  |  # # · #  |  # # # ·  |
//         |-----------------------------------------------------------------------------------------------------------
//         | blink     | # # # #  |  · · · ·  |  # # # #  |  · · · ·  |  # # # #  |  · · · ·  |  # # # #  |  · · · ·  |
//   beat  |-----------------------------------------------------------------------------------------------------------
//  Active | follow    | # · · ·  |  # # · ·  |  # # # ·  |  # # # #  |  · # # #  |  · · # #  |  · · · #  |  · · · ·  |
//   Mode  |-----------------------------------------------------------------------------------------------------------
//         | fill      | # · · ·  |  # # · ·  |  # # # ·  |  # # # #  |  # · · ·  |  # # · ·  |  # # # ·  |  # # # #  |
//         |-----------------------------------------------------------------------------------------------------------
//         | bounce    | # · · ·  |  · # · ·  |  · · # ·  |  · · · #  |  · · # ·  |  · # · ·  |  # · · ·  |  · · · ·  |
//         |-----------------------------------------------------------------------------------------------------------
//         | alternate | # # · ·  |  · · # #  |  # # · ·  |  · · # #  |  # # · ·  |  · · # #  |  # # · ·  |  · · # #  |
//         |-----------------------------------------------------------------------------------------------------------
//         | off       | · · · ·  |  · · · ·  |  · · · ·  |  · · · ·  |  · · · ·  |  · · · ·  |  · · · ·  |  · · · ·  |
//         |-----------------------------------------------------------------------------------------------------------
//         | on        | # # # #  |  # # # #  |  # # # #  |  # # # #  |  # # # #  |  # # # #  |  # # # #  |  # # # #  |

// ----------------------------------------------------------------------------
// GENERAL FUNCTIONS (nothing to be touched from here on...)
var DJCJV = {
    // Some controller-related constants
    "led": {
        "beat1": 0x3A,
        "beat2": 0x3B,
        "beat3": 0x3C,
        "beat4": 0x3D,
        "track": 0x45,
        "vinylMode": 0x46,
        "headCue": 0x4C,
        "headMix": 0x4D,
        "outerJog": 0x60,
        "innerJog": 0x61,
        "vuMeter": 0x44,
        "master": 0x90
    },
    "colour": {
        "orange": 0x02,
        "green": 0x07
    },
    "other": {
        "alpha": 0.125,          //
        "beta": 0.0078125,       //
        "speed": 33.3333333,     //
        "ledSpeed": 70.55555485, // these are re-calculated at DJCJV.init
        "beatMax": 4,            //
        "vinylModeActive": true, //
        "beatHelpTimer": 0,      //
        "left": 127,
        "pressed": 127,
        "on": 0x7F,
        "off": 0x00
    },
    "Channel": {
        "[Channel1]": {
            "central": 0x90,
            "deck": 0xB0,
            "beatPosition": 1,
            "rotation": 0x00,
            "n": 1,
            "onBeat": 0,
            "beatsPassed": 0,
            "shiftPressed": 0,
            "modePressed": 0
        },
        "[Channel2]": {
            "central": 0x91,
            "deck": 0xB1,
            "beatPosition": 1,
            "rotation": 0x00,
            "n": 2, "onBeat": 0,
            "beatsPassed": 0,
            "shiftPressed": 0,
            "modePressed": 0
        }
    },
    // Initialization
    "init": function(id) {
        print(id+": initializing...");

        // Prepare some musical constants
        if (CFG.fine.beatHelpSensitivity >= 1) {
            print(id+": WARNING: variable 'CFG.fine.beatHelpSensitivity' is set to a value equal or bigger than 1 ("+CFG.fine.beatHelpSensitivity+"). Setting it exactly to 0.9");
            CFG.fine.beatHelpSensitivity = 0.9;
        }

        // Prepare some musical constants
        DJCJV.other.alpha = 1.0 / 8;
        DJCJV.other.beta = DJCJV.other.alpha / 16;
        DJCJV.other.speed = 33 + 1/3;
        DJCJV.other.ledSpeed = (DJCJV.other.speed / 60) * 127;

        // Calculate the number of beats to be taken into account for the beat_leds, depending on the CFG.user.beatActiveMode
        if (CFG.user.beatActiveMode.match(/^(?:normal|reverse|fill)$/g)) {
            DJCJV.other.beatMax = 4;
        } else if (CFG.user.beatActiveMode.match(/^(?:blink|alternate)$/g)) {
            DJCJV.other.beatMax = 2;
        } else {
            DJCJV.other.beatMax = 8; // follow|bounce
        }

        print(id+": Using CFG.user.beatActiveMode="+CFG.user.beatActiveMode+" (beatMax="+DJCJV.other.beatMax+")");
        print(id+": Using CFG.user.beatDetection="+CFG.user.beatDetection);
        print(id+":"+((CFG.user.initUpdateEffects === 0) ? " NOT " : " ") + "Initializing some effects at startup");
        print(id+":"+((CFG.user.beatHelper !== 1) ? " NOT " : " ") + "Using beat helper");

        // Set all LED states to off
        midi.sendShortMsg(DJCJV.Channel["[Channel1]"].deck, 0x7F, DJCJV.other.off);
        midi.sendShortMsg(DJCJV.Channel["[Channel2]"].deck, 0x7F, DJCJV.other.off);

        // Set Vinyl button LED On.
        DJCJV.other.vinylModeActive = true;
        midi.sendShortMsg(DJCJV.led.master, DJCJV.led.vinylMode, DJCJV.other.on);

        // Set Headphone CUE/MIX LED state
        if (engine.getValue("[Master]", "headMix") > 0.5) {
            midi.sendShortMsg(DJCJV.led.master, DJCJV.led.headCue, DJCJV.other.on); // headset "Mix" button LED
            midi.sendShortMsg(DJCJV.led.master, DJCJV.led.headMix, DJCJV.other.off);
        } else {
            midi.sendShortMsg(DJCJV.led.master, DJCJV.led.headCue, DJCJV.other.off);
            midi.sendShortMsg(DJCJV.led.master, DJCJV.led.headMix, DJCJV.other.on); // headset "Cue" button LED
        }

        // Enable Soft takeover
        engine.softTakeover("[Master]", "crossfader", true);
        engine.softTakeover("[QuickEffectRack1_[Channel1]]", "super1", true);
        engine.softTakeover("[QuickEffectRack1_[Channel2]]", "super1", true);

        // Done to work around the limited amount of controls in the Jogvision controller
        if (CFG.user.initUpdateEffects !== 0) {
            engine.setParameter("[EffectRack1_EffectUnit1_Effect1]", "meta", CFG.user.initUpdateEffects); // Deck A, Effect 1 initial value
            engine.setParameter("[EffectRack1_EffectUnit1_Effect2]", "meta", CFG.user.initUpdateEffects); // Deck A, Effect 2 initial value
            engine.setParameter("[EffectRack1_EffectUnit1_Effect3]", "meta", CFG.user.initUpdateEffects); // Deck A, Effect 3 initial value
            engine.setParameter("[EffectRack1_EffectUnit2_Effect1]", "meta", CFG.user.initUpdateEffects); // Deck B, Effect 1 initial value
            engine.setParameter("[EffectRack1_EffectUnit2_Effect2]", "meta", CFG.user.initUpdateEffects); // Deck B, Effect 2 initial value
            engine.setParameter("[EffectRack1_EffectUnit2_Effect3]", "meta", CFG.user.initUpdateEffects); // Deck B, Effect 3 initial value
            engine.setParameter("[EffectRack1_EffectUnit1]", "mix", CFG.user.initUpdateEffects / 2); // Deck A, Effect Master mixer value
            engine.setParameter("[EffectRack1_EffectUnit2]", "mix", CFG.user.initUpdateEffects / 2); // Deck A, Effect Master mixer value
            engine.setParameter("[QuickEffectRack1_[Channel1]]", "super1", 0.5); // High/low filter Deck A (mapped to AIR FX)
            engine.setParameter("[QuickEffectRack1_[Channel2]]", "super1", 0.5); // High/low filter Deck B (mapped to AIR FX)
        }

        // Connect the VUMeters
        engine.connectControl("[Channel1]", "VuMeter", "DJCJV.vuMeterUpdate");
        engine.connectControl("[Channel2]", "VuMeter", "DJCJV.vuMeterUpdate");

        // Set inner & outer jog leds to 0
        DJCJV.updateJogLeds(0, "[Channel1]");
        DJCJV.updateJogLeds(0, "[Channel2]");

        // Enable jogs' outer leds rotation and Inner LEDs song position update
        engine.connectControl("[Channel1]", "playposition", "DJCJV.updateJogLeds");
        engine.connectControl("[Channel2]", "playposition", "DJCJV.updateJogLeds");
        if (CFG.user.beatDetection === "normal") {
            // Connect the beat_active with beat leds
            engine.connectControl("[Channel1]", "beat_active", "DJCJV.beatActive");
            engine.connectControl("[Channel2]", "beat_active", "DJCJV.beatActive");
            engine.connectControl("[Channel1]", "stop", "DJCJV.beatInactive");
            engine.connectControl("[Channel2]", "stop", "DJCJV.beatInactive");
        }

        if (CFG.user.beatHelper === 1) {
            DJCJV.other.beatHelpTimer = engine.beginTimer(100, "DJCJV.beatHelp");
        }

        // Ask the controller to send all current knob/slider values over MIDI, which will update the corresponding GUI controls in MIXXX.
        midi.sendShortMsg(0xB0, 0x7F, DJCJV.other.on);

        print(id+": initialized");
    },
    // Finalization
    "shutdown": function() {
        print("Finishing...");
        if (DJCJV.other.beatHelpTimer !== 0) {
            engine.stopTimer(DJCJV.other.beatHelpTimer);
            DJCJV.other.beatHelpTimer = 0;
        }

        engine.setValue("[Channel1]", "eject", 1);
        engine.setValue("[Channel2]", "eject", 1);
        // /(Try to) Set all LED states to off
        midi.sendShortMsg(DJCJV.Channel["[Channel1]"].deck, 0x7F, DJCJV.other.off);
        midi.sendShortMsg(DJCJV.Channel["[Channel2]"].deck, 0x7F, DJCJV.other.off);
    },
    // Beat helper
    "beatHelp": function() {
        var diff = engine.getValue("[Channel1]", "bpm") - engine.getValue("[Channel2]", "bpm");
        var move = 0;

        if (diff < (-1 * CFG.fine.beatHelpSensitivity)) {
            // Move ch1 pitch up or ch2 pitch down
            move = 4*diff + 127;
        } else if (diff > CFG.fine.beatHelpSensitivity) {
            // Move ch1 pitch down or ch2 pitch up
            move = 4*diff;
        } else {
            // Beats match
            move = 0;
        }

        if ((engine.getValue("[Channel1]", "play") !== 1) && (!engine.isScratching(DJCJV.Channel["[Channel1]"].n))) {
            midi.sendShortMsg(DJCJV.Channel["[Channel1]"].deck, DJCJV.led.outerJog, move);
        }
        if ((engine.getValue("[Channel2]", "play") !== 1) && (!engine.isScratching(DJCJV.Channel["[Channel2]"].n))) {
            midi.sendShortMsg(DJCJV.Channel["[Channel2]"].deck, DJCJV.led.outerJog, move);
        }
    },
    // Beat led ACTIVATE
    "beatActive": function(value, group, _control) {
        if ((value === 1) || (CFG.user.beatActiveMode === "off")) {
            return;
        }
        var central = DJCJV.Channel[group].central;
        var pos = DJCJV.Channel[group].beatPosition;

        if (CFG.user.beatActiveMode === "normal") {
            midi.sendShortMsg(central, DJCJV.led.beat1, pos === 1 ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat2, pos === 2 ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat3, pos === 3 ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat4, pos === 4 ? DJCJV.other.on : DJCJV.other.off);
        } else if (CFG.user.beatActiveMode === "reverse") {
            midi.sendShortMsg(central, DJCJV.led.beat1, pos !== 1 ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat2, pos !== 2 ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat3, pos !== 3 ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat4, pos !== 4 ? DJCJV.other.on : DJCJV.other.off);
        } else if (CFG.user.beatActiveMode === "blink") {
            midi.sendShortMsg(central, DJCJV.led.beat1, pos === 1 ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat2, pos === 1 ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat3, pos === 1 ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat4, pos === 1 ? DJCJV.other.on : DJCJV.other.off);
        } else if (CFG.user.beatActiveMode === "follow") {
            midi.sendShortMsg(central, DJCJV.led.beat1, ((pos >= 1) && (pos <= 4)) ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat2, ((pos >= 2) && (pos <= 5)) ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat3, ((pos >= 3) && (pos <= 6)) ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat4, ((pos >= 4) && (pos <= 7)) ? DJCJV.other.on : DJCJV.other.off);
        } else if (CFG.user.beatActiveMode === "bounce") {
            midi.sendShortMsg(central, DJCJV.led.beat1, ((pos >= 1) && (pos <= 7)) ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat2, ((pos >= 2) && (pos <= 6)) ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat3, ((pos >= 3) && (pos <= 5)) ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat4, ((pos >= 4) && (pos <= 4)) ? DJCJV.other.on : DJCJV.other.off);
        } else if (CFG.user.beatActiveMode === "fill") {
            midi.sendShortMsg(central, DJCJV.led.beat1, (pos >= 1) ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat2, (pos >= 2) ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat3, (pos >= 3) ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat4, (pos >= 4) ? DJCJV.other.on : DJCJV.other.off);
        } else if (CFG.user.beatActiveMode === "alternate") {
            midi.sendShortMsg(central, DJCJV.led.beat1, pos === 1 ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat2, pos === 1 ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat3, pos === 2 ? DJCJV.other.on : DJCJV.other.off);
            midi.sendShortMsg(central, DJCJV.led.beat4, pos === 2 ? DJCJV.other.on : DJCJV.other.off);
        } else if (CFG.user.beatActiveMode === "on") {
            midi.sendShortMsg(central, DJCJV.led.beat1, DJCJV.other.on);
            midi.sendShortMsg(central, DJCJV.led.beat2, DJCJV.other.on);
            midi.sendShortMsg(central, DJCJV.led.beat3, DJCJV.other.on);
            midi.sendShortMsg(central, DJCJV.led.beat4, DJCJV.other.on);
        }

        if (CFG.user.beatDetection !== "follow") {
            if (pos >= DJCJV.other.beatMax) {
                DJCJV.Channel[group].beatPosition = 1;
            } else {
                DJCJV.Channel[group].beatPosition = pos + 1;
            }
        }
    },
    // Beat led DEACTIVATE
    "beatInactive": function(value, group, _control) {
        midi.sendShortMsg(DJCJV.Channel[group].central, DJCJV.led.beat1, DJCJV.other.off);
        midi.sendShortMsg(DJCJV.Channel[group].central, DJCJV.led.beat2, DJCJV.other.off);
        midi.sendShortMsg(DJCJV.Channel[group].central, DJCJV.led.beat3, DJCJV.other.off);
        midi.sendShortMsg(DJCJV.Channel[group].central, DJCJV.led.beat4, DJCJV.other.off);

        DJCJV.Channel[group].beatPosition = 1;
    },
    // Jogwheels inner LED display - Play position
    "wheelInnerUpdate": function(value, group, _control) {
        var playPos = value * 127;
        midi.sendShortMsg(DJCJV.Channel[group].deck, DJCJV.led.innerJog, playPos);

        // Also update the "track" led
        if (engine.getValue(group, "end_of_track")) {
            return; // Let Mixxx manage the end_of_track (flashing red) status
        }
        // Depending on playPos value, turn "track" led to -----------------------------> DJCJV.colour.orange : DJCJV.colour.green
        midi.sendShortMsg(DJCJV.Channel[group].central, DJCJV.led.track, (playPos > 64) ? DJCJV.colour.orange : DJCJV.colour.green);
    },
    // Function to rotate jogs' outer led (borrowed from the 'Pioneer-DDJ-SX-scripts.js' mapping)
    "updateJogLeds": function(value, group, control) {
        var elapsedTime = value * engine.getValue(group, "duration");
        var wheelPos = ((value >= 0) ? 0 : 127) + 1 + ((DJCJV.other.ledSpeed * elapsedTime) % 127);

        // Only send midi message when the position is actually updated.
        if (DJCJV.Channel[group].rotation !== wheelPos) {
            midi.sendShortMsg(DJCJV.Channel[group].deck, DJCJV.led.outerJog, wheelPos); // Update the outer (spin) jog leds
            DJCJV.wheelInnerUpdate(value, group, control); // Also update the inner jog leds with updated song position
        }
        DJCJV.Channel[group].rotation = wheelPos;

        if (CFG.user.beatDetection === "follow") {
            DJCJV.Channel[group].beatsPassed = Math.round((value * engine.getValue(group, "duration")) * (engine.getValue(group, "bpm") / 60));
            DJCJV.Channel[group].beatPosition = Math.floor((DJCJV.Channel[group].beatsPassed % DJCJV.other.beatMax)) + 1;

            // If on beat_active, update the beat leds
            if (engine.getValue(group, "beat_active") || ((engine.getValue(group, "beat_closest") < engine.getValue(group, "beat_next"))) && (!DJCJV.Channel[group].onBeat)) {
                DJCJV.beatActive(0, group);
                DJCJV.Channel[group].onBeat = true;
            } else if (engine.getValue(group, "beat_closest") >= engine.getValue(group, "beat_next")) {
                DJCJV.Channel[group].onBeat = false;
            }
        }
    },
    // Vu Meter (has to be scaled x6)
    "vuMeterUpdate": function(value, group, _control) {
        midi.sendShortMsg(DJCJV.Channel[group].central, DJCJV.led.vuMeter, value * 6);
    },
    // Headphone CUE button
    "headCue": function(midino, control, value, status, group) {
        if (engine.getValue(group, "headMix") === 0) {
            engine.setValue(group, "headMix", -1.0);
            midi.sendShortMsg(DJCJV.led.master, DJCJV.led.headCue, DJCJV.other.off);
            midi.sendShortMsg(DJCJV.led.master, DJCJV.led.headMix, DJCJV.other.on);
        }
    },
    // Headphone MIX button
    "headMix": function(midino, control, value, status, group) {
        if (engine.getValue(group, "headMix") !== 1) {
            engine.setValue(group, "headMix", 0);
            midi.sendShortMsg(DJCJV.led.master, DJCJV.led.headCue, DJCJV.other.on);
            midi.sendShortMsg(DJCJV.led.master, DJCJV.led.headMix, DJCJV.other.off);
        }
    },
    // Filter (AIR FX)
    "Filter": function(channel, control, value, status, group) {
        var deck = group.substr(18, 10);
        // Filter ---------------------------------------> High-pass : Low-pass
        var delta = DJCJV.Channel[deck].shiftPressed ? (value / 255) : (-1 * (value / 255));
        engine.setValue(group, "super1", 0.5 + delta);
    },
    // Sniff decks' SHIFT presses and store them
    "shiftKey": function(channel, control, value, status, group) {
        DJCJV.Channel[group].shiftPressed = (value === DJCJV.other.pressed);
        return value;
    },
    // Sniff decks' MODE presses and store them
    "modeKey": function(channel, control, value, status, group) {
        DJCJV.Channel[group].modePressed = (value === DJCJV.other.pressed);
        return value;
    },
    // MODE + Loop ON creates a loop at given point
    "beatloopActivate": function(channel, control, value, status, group) {
        // If "MODE" button IS pressed, activate beatloop at current playposition
        if (DJCJV.Channel[group].modePressed) {
            engine.setValue(group, "slip_enabled", value);
            engine.setValue(group, "beatloop_activate", value);
        } else {
            engine.setValue(group, "reloop_toggle", value);
        }
    },
    // SHIFT+FX LOOP:SIZE Knob MOVES the existing loop beatjump_size beats forward/backward
    "beatjumpMove": function(channel, control, value, status, group) {
        // loop_move ---------------------------------------------> backward : forward (times 'beatjump_size')
        engine.setValue(group, "loop_move", (value === DJCJV.other.left ? -1 : 1)*engine.getValue(group, "beatjump_size"));
    },
    // FX LOOP:SIZE Knob changes the AMOUNT of beats to move the loop when requested (or 'pitch' if MODE is pressed)
    "beatjumpSize": function(channel, control, value, status, group) {
        if (DJCJV.Channel[group].modePressed) {
            // change pith one half -------------------------------> (left) down : (right) up
            engine.setValue(group, "pitch_"+(value === DJCJV.other.left ? "down" : "up"), 1);
        } else {
            // beatjump_size -------------------------------------------------------------------> half : double
            var newBJS = engine.getValue(group, "beatjump_size") * ((value === DJCJV.other.left) ? 1/2 : 2);
            engine.setValue(group, "beatjump_size", (newBJS >= 512) ? 512 : (newBJS <= 1/32 ? 1/32 : newBJS)); // Prevent moving beyond lower/upper limits
        }
    },
    // Loop X 1/2 or 2 pressed (halve/double loop, or beatjump backward/forward beatjump_size beats)
    "beatLoopChange": function(channel, control, value, status, group) {
        if (DJCJV.Channel[group].modePressed) {
            // Do a 'beatjump_size' beats beatjump -------------> backward : forward
            engine.setValue(group, "beatjump_"+(control === 0 ? "backward" : "forward"),  value);
        } else {
            // Loop size change by ---------------------> halve : double
            engine.setValue(group, control === 0 ? "loop_halve" : "loop_double", value);
        }
    },
    // FX(Mix) Knob changes the mix level for the effects
    "mixLevel": function(channel, control, value, status, group) {
        var mixValue = engine.getValue(group, "mix");
        // modify effects mix level -------------------------------------------------------------> decrease : increase (in mixGainFactor steps)
        engine.setValue(group, "mix", mixValue + (CFG.fine.mixGainFactor * (value === DJCJV.other.left ? -1 : 1)));
    },
    // The Vinyl button, used to enable or disable scratching on the jog wheels.
    "vinylButton": function(channel, control, value, _status, _group) {
        if (!value) {
            return;
        }

        DJCJV.other.vinylModeActive = !DJCJV.other.vinylModeActive;
        midi.sendShortMsg(DJCJV.led.master, DJCJV.led.vinylMode, DJCJV.other.vinylModeActive ? DJCJV.other.on : DJCJV.other.off);
    },
    // The pressure action over the jog wheel
    "wheelTouch": function(channel, control, value, status, group) {
        if (value > 0 && (engine.getValue(group, "play") !== 1 || DJCJV.other.vinylModeActive)) {
            // Start backSpin
            if (DJCJV.Channel[group].shiftPressed) {
                engine.setValue(group, "reverseroll", DJCJV.other.on);
                // spinBack ------------------------------------------------------------------------------------------------> brake_strength, initial_speed
                engine.spinback(parseInt(group.substring(8, 9)), DJCJV.other.on, engine.getValue(group, "bpm") / CFG.fine.spinBackBrakeFactor, -1 * CFG.fine.spinBackInitialSpeed);
            // Start scratch
            } else {
                // ... with slip mode on
                if (DJCJV.Channel[group].modePressed && (engine.getValue(group, "play") === 1)) {
                    engine.setValue(group, "slip_enabled", DJCJV.other.on);
                }
                engine.scratchEnable(DJCJV.Channel[group].n, 400, DJCJV.other.speed, DJCJV.other.alpha, DJCJV.other.beta); // Wheel touched
            }
        // End scratch/backSpin
        } else {
            if (DJCJV.Channel[group].shiftPressed) {
                engine.spinback(parseInt(group.substring(8, 9)), DJCJV.other.off);
            }
            engine.setValue(group, "reverseroll", DJCJV.other.off);
            engine.setValue(group, "slip_enabled", DJCJV.other.off);
            engine.scratchDisable(DJCJV.Channel[group].n); // Wheel released
        }
    },
    // Using the top of wheel for scratching (Vinyl button On) and bending (Vinyl button Off)
    "scratchWheel": function(channel, control, value, status, group) {
        // If NOT playing, and MODE button is pressed, move 'playposition'
        if ((DJCJV.Channel[group].modePressed) && (engine.getValue(group, "play") !== 1)) {
            // new_playposition moves -------------------------------------------------> ( backward : forward ) times CFG.fine.quickMoveFactor
            var newpos = engine.getValue(group, "playposition") + ((value === DJCJV.other.left ? -1 : 1) * CFG.fine.quickMoveFactor);
            // Set playposition to ------------------------> start :       end        : new_playposition
            engine.setValue(group, "playposition", newpos <= 0 ? 0 : (newpos >= 1 ? 1 : newpos));
        } else if (engine.isScratching(DJCJV.Channel[group].n)) {
            // Scratch -------------------------------------------------------> Backward : Forward
            engine.scratchTick(DJCJV.Channel[group].n, (value === DJCJV.other.left) ? -1 : 1);
        } else {
            // Goto: DJCJV.bendWheel function
            DJCJV.bendWheel(channel, control, value, status, group);
        }
    },
    // Bending by either using the side of wheel, or with the Jog surface when not in vinyl-mode
    "bendWheel": function(channel, control, value, status, group) {
        if (engine.isScratching(DJCJV.Channel[group].n)) {
            // Spin ----------------------------------------------------------> Backward : Forward
            engine.scratchTick(DJCJV.Channel[group].n, (value === DJCJV.other.left) ? -1 : 1);
        } else {
            // Pitch bend --------------------------------------> Slow Down : Speed Up
            engine.setValue(group, "jog", (value === DJCJV.other.left) ? -1 : 1);
        }
    },
    // Move 'pregain' knob
    "preGain": function(channel, control, value, status, group) {
        // pregain ------------------------------------------------------------------> decrease : increase
        engine.setValue(group, "pregain", engine.getValue(group, "pregain") + ((value > 64 ? -1 : 1) * CFG.fine.mixGainFactor));
    },
    // Load track or toggle quantize
    "loadTrack": function(channel, control, value, status, group) {
        if (DJCJV.Channel[group].modePressed && value) {
            engine.setValue(group, "quantize", ! engine.getValue(group, "quantize")); // Toggle quantize
        } else {
            engine.setValue(group, "LoadSelectedTrack", value); // Load track
        }
    },
    // BROWSER knob presses
    "moveFocus": function(channel, control, value, status, group) {
        if (DJCJV.Channel["[Channel1]"].shiftPressed || DJCJV.Channel["[Channel2]"].shiftPressed) {
            engine.setValue(group, "GoToItem", value); // SHIFT pressed: activate (double-click) selected item
        } else {
            engine.setValue(group, "MoveFocus", value);// Normal press: toggle focus
        }
    },
    // BROWSER knob rotation: Browse library
    "browseLibrary": function(channel, control, value, status, group) {
        if (DJCJV.Channel["[Channel1]"].modePressed || DJCJV.Channel["[Channel2]"].modePressed) {
            // Quick move -------------------------------------> ( up : down ) times CFG.fine.quickBrowseFactor
            engine.setValue(group, "MoveVertical", ((value > 64) ? -1 : 1) * CFG.fine.quickBrowseFactor);
        } else {
            // Slow move -----------------------------> up : down
            engine.setValue(group, (value > 64) ? "MoveUp" : "MoveDown", 1);
        }
    }
};
