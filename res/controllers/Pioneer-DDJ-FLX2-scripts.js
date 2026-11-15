// DDJ-FLX2 controller object.
//
// Primary hardware reference:
// https://assets.pioneerdjhub.com/DDJ-FLX2_MIDI_Message_List_E1.pdf
//
// The mapping follows the Pioneer DDJ-FLX2 MIDI message list for deck controls,
// mixer controls, performance pads, and documented illumination messages.
// Where the FLX2 exposes rekordbox-specific functions that Mixxx does not have
// as engine controls, the mapping either leaves the message unused or claims it
// without emulating rekordbox-only behavior.
//
// Important implementation notes:
// - The DDJ-FLX2 is physically a two-deck controller. fourDeckMode is a custom
//   Mixxx layer that reuses the left/right hardware sides for virtual decks
//   3 and 4. isVirtualDeckVisible() prevents hidden deck LED updates from
//   corrupting the currently visible physical deck.
// - Smart Fader (0x96/0x01) toggles channel fader-start behavior.
// - Smart CFX (0x96/0x09 in the Pioneer reference) is intentionally unmapped.
// - Vinyl mode is set ON at startup with MIDI OUT 9n 17 7F. The controller
//   cannot toggle Vinyl mode from the unit itself.
// - Fader-start: handled by monitoring volume fader position (volumeLSB)
//   when Smart Fader is enabled. Hardware fader-start messages (0x66/0x5D/0x52)
//   are ignored to prevent redundant triggers.
// - 0x60 is not bpm_tap; it is Pad 1 in pad mode 3.
// - Shift + Headphone (CUE) buttons: asymmetric functions. Left deck handles
//   universal Load/Eject via Cross-Shift logic, Right deck handles FX selection.
// eslint-disable-next-line no-var
var DDJFLX2 = {
    // Custom Mixxx layer mode:
    // false = physical decks control virtual decks 1 and 2
    // true  = physical decks control virtual decks 3 and 4
    fourDeckMode: false,

    // Maps physical decks to Mixxx virtual decks.
    // Index 0 is unused so deck numbers can be used directly.
    vDeckNo: [0, 1, 2],

    // Per-virtual-deck runtime storage (e.g. last CFX knob mode).
    vDeck: {},

    // Last received MSB of each 14-bit hardware control, per physical deck.
    // Stored per physical side (not per virtual deck) because it describes the
    // hardware control, which stays where it is across deck layer switches.
    // Keys: "rate", "volume", "cfx", and the EQ MSB control number.
    hwMSB: {1: {}, 2: {}},

    // Pitch bend factor for jog wheel side movement.
    pitchShiftFactor: 0.5,

    // Current shift button state.
    // Press/release logic is more reliable than toggle logic.
    shiftPressed: {left: false, right: false},

    // Shift + deck/load button gesture state.
    // Short press loads the selected track, long press ejects the deck.
    loadLongPress: {left: false, right: false},
    loadTimer: {left: 0, right: 0},
    loadLongPressDelayMs: 500,

    loopPadSizes: [0.125, 0.25, 0.5, 1, 2, 4, 8, 16],
    padFxState: {},
    padFxUnitState: {},

    // Virtual deck targeted by held press/release actions (scratch, cue,
    // hotcue preview, loop roll), keyed by "<physicalDeck>:<action>".
    // See resolveHeldDeck().
    heldDeck: {},

    // Which pad mode (if any) is replaced by Beat Jump.
    // Values: "off" | "sampler" | "padfx" — loaded from Mixxx preferences.
    beatJumpPadMode: "off",

    // Pad FX behavior — loaded from Mixxx preferences.
    // "override": hold to apply (snapshot + restore on release).
    // "toggle":   press to enable/disable the effect slot.
    padFxMode: "override",

    // Meta value applied to the effect slot in Pad FX override mode.
    padFxMeta: 0.75,

    // Sizes for the 4 pad pairs (pads 1-2, 3-4, 5-6, 7-8).
    // Holding Shift doubles each size (2, 4, 8, 16 beats).
    beatJumpSizes: [1, 2, 4, 8],

    // Shared blink state for all animated LEDs (loop active, sampler playing).
    // blinkPhase toggles on every timer tick; all blinking LEDs share the same
    // phase so they pulse in sync.
    // blinkTimerID holds the repeating timer handle so it can be stopped on shutdown.
    blinkPhase: false,
    blinkTimerID: 0,

    // Last value sent to each pad LED repainted by the blink timer
    // (loop and sampler pads), keyed by (status << 8) | control.
    // The timer only sends a message when a LED value changes; the cache is
    // cleared whenever LEDs are switched off in bulk so they are repainted.
    padLedCache: {},

    // Smart Fader state.
    // When true, channel fader movements trigger play/pause on the corresponding
    // deck via software-side volume fader monitoring (handleSmartFaderVolume).
    //
    // NOTE: Dedicated hardware fader-start messages (0x66/0x5D/0x52) are
    // intentionally ignored. This prevents "double triggers" and unpredictable
    // behavior if the controller is accidentally put into its built-in firmware
    // Smart Fader mode (long press). This ensures a single, predictable
    // software-controlled behavior.
    smartFaderEnabled: false,

    // Whether each physical channel fader is at its bottom position.
    // Tracked per physical deck (not per virtual deck) because it describes
    // the hardware fader, which stays where it is across deck layer switches.
    faderDown: {1: true, 2: true},

    // Accumulates jog movement while browsing the library.
    jogCounter: 0,

    // Accumulated jog movement (in jog ticks) required to move one library row.
    // Higher = slower, more deliberate scrolling.
    browseJogThreshold: 20,

    // Helper: check if shift is pressed for a specific deck group.
    // Accepts both physical ([Channel1]/[Channel2]) and virtual
    // ([Channel3]/[Channel4]) groups: virtual decks 1/3 are on the left side,
    // 2/4 on the right side.
    isShiftPressed: function(group) {
        const physicalDeck = this.virtualToPhysicalDeck(script.deckFromGroup(group));
        return this.shiftPressed[this.deckSide(physicalDeck)];
    },

    // Helper: resolve physical deck, virtual deck, and virtual deck group from a group.
    resolveDeck: function(group) {
        const physicalDeck = script.deckFromGroup(group);
        const virtualDeck = this.vDeckNo[physicalDeck];
        return {
            physicalDeck: physicalDeck,
            virtualDeck: virtualDeck,
            group: `[Channel${virtualDeck}]`
        };
    },

    // Helper: resolve the deck for a held press/release action.
    // On press, the virtual deck currently assigned to the side is remembered;
    // on release, the remembered deck is returned even if the deck layer was
    // switched while the control was held. This prevents actions from getting
    // stuck on the previous deck (e.g. scratch mode, loop roll, cue preview).
    //
    // group   : physical group from the XML binding
    // action  : identifier of the held control (unique per side)
    // pressed : true on press, false on release
    resolveHeldDeck: function(group, action, pressed) {
        const deck = this.resolveDeck(group);
        const key = `${deck.physicalDeck}:${action}`;
        if (pressed) {
            this.heldDeck[key] = deck;
            return deck;
        }
        const held = this.heldDeck[key];
        delete this.heldDeck[key];
        return held || deck;
    },

    // Helper: return stable side key for per-physical-deck state.
    deckSide: function(physicalDeck) {
        return physicalDeck === 1 ? "left" : "right";
    },

    // Helper: list the [group, key] pairs of the script-bound continuous
    // controls of one virtual deck (rate, volume, gain, EQ, CFX).
    //
    // These controls use Mixxx's built-in soft takeover (engine.softTakeover),
    // which also applies to values written by scripts with engine.setParameter().
    // It is enabled for all four virtual decks in init() and re-armed on every
    // layer switch, so physical controls do not cause jumps when the newly
    // active virtual deck has different values.
    softTakeoverControls: function(vDeckNo) {
        const vgroup = `[Channel${vDeckNo}]`;
        const eqGroup = `[EqualizerRack1_${vgroup}_Effect1]`;
        return [
            [vgroup, "rate"],
            [vgroup, "volume"],
            [vgroup, "pregain"],
            [eqGroup, "parameter1"],
            [eqGroup, "parameter2"],
            [eqGroup, "parameter3"],
            [`[QuickEffectRack1_${vgroup}]`, "super1"],
        ];
    },

    // Helper: after a layer switch, ignore the next hardware value of every
    // soft takeover control of the given virtual deck, so the physical control
    // has to reach (or cross) the deck's current value before it takes over.
    rearmSoftTakeover: function(vDeckNo) {
        for (const [group, key] of this.softTakeoverControls(vDeckNo)) {
            engine.softTakeoverIgnoreNextValue(group, key);
        }
    },

    // Helper: map a virtual deck number to its physical deck (1-based).
    // Virtual decks 1 and 3 map to physical deck 1 (left).
    // Virtual decks 2 and 4 map to physical deck 2 (right).
    virtualToPhysicalDeck: function(vDeckNo) {
        return (vDeckNo === 1 || vDeckNo === 3) ? 1 : 2;
    },

    // Helper: return the 0-based LED index for a virtual deck.
    // Physical deck 1 (left) → 0, physical deck 2 (right) → 1.
    // NOTE: use only for play/cue/sync LEDs, NOT for pad LEDs.
    virtualToLedIndex: function(vDeckNo) {
        return this.virtualToPhysicalDeck(vDeckNo) - 1;
    },

    // Helper: return the correct MIDI OUT status byte for pad LEDs.
    //
    // THE ASYMMETRY:
    // Pioneer DDJ controllers often use different MIDI channels for Input (pads
    // to software) and Output (software to LED).
    // - MIDI-IN: Pads usually send on 0x97/0x99.
    // - MIDI-OUT: To light up the LEDs, we must send to specific channels that
    //   distinguish between "Normal" and "Shifted" pad modes:
    // NOTE: Pad modes (Hotcue, Pad FX, Loop, Sampler) are switched internally
    // by the controller firmware. The script identifies the current mode
    // based on the MIDI note/CC range received (e.g., 0x00-0x07 for Hotcues).
    //
    //   Left deck  normal : 0x97  shifted : 0x98
    //   Right deck normal : 0x99  shifted : 0x9A
    getPadLedStatus: function(physicalDeck, shifted) {
        if (physicalDeck === 1) {
            return shifted ? 0x98 : 0x97;
        }
        return shifted ? 0x9A : 0x99;
    },

    // Helper: return true only when vDeckNo is the virtual deck currently
    // assigned to its physical side.  Used to suppress LED writes for
    // background decks that are not visible on the controller.
    isVirtualDeckVisible: function(vDeckNo) {
        const physicalDeck = this.virtualToPhysicalDeck(vDeckNo);
        return this.vDeckNo[physicalDeck] === vDeckNo;
    },

    // Helper: extract hotcue pad number (1-8) from a hotcue mode control byte.
    hotcuePadFromControl: function(control) {
        return control + 1;
    },

    // Helper: extract loop pad number (1-8) from a loop mode control byte.
    loopPadFromControl: function(control) {
        return (control - 0x60) + 1;
    },

    // Helper: extract sampler pad number (1-8) from a sampler mode control byte.
    samplerPadFromControl: function(control) {
        return (control - 0x30) + 1;
    },

    // Helper: extract pad number from a pad FX mode control byte.
    padFxPadFromControl: function(control) {
        return (control & 0x07) + 1;
    },
};

DDJFLX2.init = function() {
    DDJFLX2.beatJumpPadMode = engine.getSetting("beatJumpPadMode") || "off";
    DDJFLX2.padFxMode = engine.getSetting("padFxMode") || "override";
    DDJFLX2.padFxMeta = Number(engine.getSetting("padFxMeta") ?? 0.75);
    DDJFLX2.cfxKnobBehavior = engine.getSetting("cfxKnobBehavior") || "dual";

    for (let i = 1; i <= 4; i++) {
        // Create runtime storage for each virtual deck.
        this.vDeck[i] = {};

        const vgroup = `[Channel${i}]`;

        // Enable built-in soft takeover for the script-bound continuous controls.
        // The first value received after enabling is ignored, so controls do not
        // jump to the hardware position when the mapping starts.
        for (const [group, key] of DDJFLX2.softTakeoverControls(i)) {
            engine.softTakeover(group, key, true);
        }

        // Keep LEDs synchronized with track loading state.
        engine.makeConnection(vgroup, "track_loaded", function(loaded, vgroup) {
            DDJFLX2.updateDeckLeds(vgroup, loaded);

            // Force cue LED off when a track is unloaded.
            if (!loaded) {
                const vDeckNo = script.deckFromGroup(vgroup);
                const physicalDeck = DDJFLX2.virtualToPhysicalDeck(vDeckNo);

                // Only update LEDs if the deck is currently visible
                // on the physical controller.
                if (vDeckNo === DDJFLX2.vDeckNo[physicalDeck]) {
                    DDJFLX2.switchCueLED(DDJFLX2.virtualToLedIndex(vDeckNo), false);
                }
            }
        });

        // Keep play LED synchronized with Mixxx playback state.
        engine.makeConnection(vgroup, "play", function(ch, vgroup) {
            const vDeckNo = script.deckFromGroup(vgroup);
            if (!DDJFLX2.isVirtualDeckVisible(vDeckNo)) { return; }

            DDJFLX2.switchPlayLED(DDJFLX2.virtualToLedIndex(vDeckNo), ch);
        });

        // Keep sync LED synchronized with Mixxx sync state.
        engine.makeConnection(vgroup, "sync_enabled", function(ch, vgroup) {
            const vDeckNo = script.deckFromGroup(vgroup);
            if (!DDJFLX2.isVirtualDeckVisible(vDeckNo)) { return; }

            DDJFLX2.switchSyncLED(DDJFLX2.virtualToLedIndex(vDeckNo), ch);
        });

        // Keep headphone cue (PFL) LED synchronized with Mixxx PFL state,
        // including changes made from the GUI.
        engine.makeConnection(vgroup, "pfl", function(enabled, vgroup) {
            const vDeckNo = script.deckFromGroup(vgroup);
            if (!DDJFLX2.isVirtualDeckVisible(vDeckNo)) { return; }

            midi.sendShortMsg(0x90 + DDJFLX2.virtualToLedIndex(vDeckNo), 0x54, enabled ? 0x7f : 0x00);
        });

        // Keep cue LED synchronized with Mixxx's cue indicator, which follows
        // the cue mode selected in Preferences (including blinking).
        engine.makeConnection(vgroup, "cue_indicator", function(value, vgroup) {
            const vDeckNo = script.deckFromGroup(vgroup);
            if (!DDJFLX2.isVirtualDeckVisible(vDeckNo)) { return; }

            DDJFLX2.switchCueLED(DDJFLX2.virtualToLedIndex(vDeckNo), value ? 1 : 0);
        });

        // Keep pad LEDs synchronized with hotcue state.
        // This is the single source of truth for hotcue LED updates;
        // hotcueNActivate must NOT send its own LED echo.
        for (let j = 1; j <= 8; j++) {
            engine.makeConnection(
                vgroup,
                `hotcue_${j}_status`,
                function(ch, vgroup, control) {
                    const pad = Number(control.split("_")[1]);
                    const vDeckNo = script.deckFromGroup(vgroup);
                    if (!DDJFLX2.isVirtualDeckVisible(vDeckNo)) { return; }

                    DDJFLX2.switchPadLED(
                        DDJFLX2.virtualToPhysicalDeck(vDeckNo),
                        pad,
                        ch,
                        false
                    );
                }
            );
        }
    }

    DDJFLX2.allLEDsOff();

    // Keep the headphone cue (master) LED synchronized with the headphone mix,
    // including changes made with the Cue/Mix knob in the GUI. The LED is lit
    // when the mix is on the master side. Triggered once so the LED matches the
    // current state when the mapping starts.
    engine.makeConnection("[Master]", "headMix", function(value) {
        midi.sendShortMsg(0x96, 0x63, value > 0.5 ? 0x7f : 0x00);
    }).trigger();

    // Per the Pioneer MIDI reference, Vinyl mode cannot be changed from the
    // hardware. It is controlled by software with MIDI OUT 9n 17 hh.
    // Keep it enabled so platter rotation uses the scratch-capable 0x22 path.
    DDJFLX2.setVinylMode(true);

    // Request current hardware control positions from the controller.
    // Sysex structure (Pioneer proprietary):
    // F0 00 40 05 (Pioneer ID) 00 00 (Sub ID) 02 0a 00 03 01 (Request Cmd) F7
    midi.sendSysexMsg(
        [0xf0, 0x00, 0x40, 0x05, 0x00, 0x00, 0x02, 0x0a, 0x00, 0x03, 0x01, 0xf7],
        12
    );

    // Start the global blink timer.
    //
    // SYNCED BLINKING:
    // Instead of having multiple timers for different LEDs (which would drift
    // out of sync), we use a single global timer. Every 200ms, it toggles
    // 'blinkPhase'. All functions that need a blinking LED (like playing
    // samplers or active loops) check this single phase to decide whether
    // to turn the LED on or off. This ensures all blinking elements pulse
    // in perfect unison.
    DDJFLX2.blinkTimerID = engine.beginTimer(200, function() {
        DDJFLX2.blinkPhase = !DDJFLX2.blinkPhase;
        DDJFLX2.updateAllBlinkingLEDs();
    });
};

DDJFLX2.shutdown = function() {
    if (DDJFLX2.blinkTimerID !== 0) {
        engine.stopTimer(DDJFLX2.blinkTimerID);
        DDJFLX2.blinkTimerID = 0;
    }
    DDJFLX2.releaseAllPadFx();
    DDJFLX2.allLEDsOff();
};

DDJFLX2.LEDsOff = function() {
    // Reset only LEDs whose state depends on the currently visible deck layer.
    // Sampler LEDs and dedicated loaded LEDs are intentionally not touched here.
    for (let i = 0; i <= 1; i++) {
        midi.sendShortMsg(0x90 + i, 0x54, 0x00);
        midi.sendShortMsg(0x90 + i, 0x58, 0x00);
        midi.sendShortMsg(0x90 + i, 0x78, 0x00);
        midi.sendShortMsg(0x90 + i, 0x5c, 0x00);
        midi.sendShortMsg(0x90 + i, 0x0b, 0x00);
        midi.sendShortMsg(0x90 + i, 0x0c, 0x00);

        // Hotcue and loop LEDs reflect the visible virtual deck.
        // Pad FX is temporary per physical deck and should not survive a layer switch.
        const physicalDeck = i + 1;
        const padLedControls = [0x00, 0x10, 0x60];
        for (let range = 0; range < padLedControls.length; range++) {
            for (let j = 0; j <= 7; j++) {
                midi.sendShortMsg(
                    DDJFLX2.getPadLedStatus(physicalDeck, false),
                    padLedControls[range] + j,
                    0x00
                );
                midi.sendShortMsg(
                    DDJFLX2.getPadLedStatus(physicalDeck, true),
                    padLedControls[range] + j,
                    0x00
                );
            }
        }
    }

    // Force the blink timer to repaint loop and sampler pads.
    DDJFLX2.padLedCache = {};
};

DDJFLX2.allLEDsOff = function() {
    // Smart Fader LED. Smart CFX (0x96/0x09) is intentionally not touched
    // because it is not mapped in Mixxx.
    midi.sendShortMsg(0x96, 0x01, 0x00);
    midi.sendShortMsg(0x96, 0x63, 0x00);

    DDJFLX2.LEDsOff();

    // Sampler LEDs do not depend on the visible deck layer, but they should be
    // turned off on mapping startup/shutdown.
    for (let physicalDeck = 1; physicalDeck <= 2; physicalDeck++) {
        for (let shifted = 0; shifted <= 1; shifted++) {
            const ledStatus = DDJFLX2.getPadLedStatus(physicalDeck, shifted === 1);
            for (let j = 0; j <= 7; j++) {
                midi.sendShortMsg(ledStatus, 0x30 + j, 0x00);
            }
        }
    }

    // Dedicated loaded LEDs for virtual decks 1-4.
    for (let deck = 0; deck <= 3; deck++) {
        midi.sendShortMsg(0x9f, deck, 0x00);
    }

    // Force the blink timer to repaint loop and sampler pads.
    DDJFLX2.padLedCache = {};
};

DDJFLX2.updateSmartFaderLED = function() {
    midi.sendShortMsg(0x96, 0x01, DDJFLX2.smartFaderEnabled ? 0x7f : 0x00);
};

DDJFLX2.updateDeckLeds = function(vgroup, value) {
    const vDeckNo = script.deckFromGroup(vgroup);
    const physicalDeck = DDJFLX2.virtualToPhysicalDeck(vDeckNo);

    // Clear LEDs when a track is unloaded.
    if (!value) {
        DDJFLX2.switchLEDs(vDeckNo);
        DDJFLX2.switchLoadedLED(vDeckNo, false);
        return;
    }

    // Refresh LEDs only if the virtual deck is currently mapped
    // to a visible physical deck.
    if (vDeckNo === DDJFLX2.vDeckNo[physicalDeck]) {
        DDJFLX2.switchLEDs(vDeckNo);
    }

    DDJFLX2.switchLoadedLED(vDeckNo, value);
};

DDJFLX2.switchLoadedLED = function(vDeckNo, loaded) {
    // Dedicated hardware LEDs for track loaded state.
    midi.sendShortMsg(0x9f, vDeckNo - 1, loaded ? 0x7f : 0x00);
};

DDJFLX2.setVinylMode = function(enabled) {
    // Pioneer setting message: deck 1/2 MIDI OUT 9n 17 hh.
    // enabled=true selects Vinyl mode ON; enabled=false selects Vinyl mode OFF.
    for (let deck = 0; deck <= 1; deck++) {
        midi.sendShortMsg(0x90 + deck, 0x17, enabled ? 0x7f : 0x00);
    }
};

// Shift + Headphone Cue (Deck 1 / Left):
// Short press loads the selected track, long press ejects the deck.
//
// Cross-shift logic:
// - Left Shift + Left Headphone: Load/Eject Deck 1.
// - Right Shift + Left Headphone: Load/Eject Deck 2.
// This allows the left button to act as a universal Load/Eject button
// while the right button remains dedicated to FX selection.
DDJFLX2.loadEject = function(channel, control, value, status, group) {
    let selectedPhysicalDeck = 0;
    if (DDJFLX2.shiftPressed.left && !DDJFLX2.shiftPressed.right) {
        selectedPhysicalDeck = 1;
    } else if (DDJFLX2.shiftPressed.right && !DDJFLX2.shiftPressed.left) {
        selectedPhysicalDeck = 2;
    }

    const deck = selectedPhysicalDeck
        ? {
            physicalDeck: selectedPhysicalDeck,
            virtualDeck: DDJFLX2.vDeckNo[selectedPhysicalDeck],
            group: `[Channel${DDJFLX2.vDeckNo[selectedPhysicalDeck]}]`
        }
        : DDJFLX2.resolveDeck(group);

    const side = DDJFLX2.deckSide(deck.physicalDeck);

    if (value) {
        // Button pressed: start long-press timer.
        if (DDJFLX2.loadTimer[side] !== 0) {
            engine.stopTimer(DDJFLX2.loadTimer[side]);
        }
        DDJFLX2.loadLongPress[side] = false;
        DDJFLX2.loadTimer[side] = engine.beginTimer(
            DDJFLX2.loadLongPressDelayMs,
            function() {
                DDJFLX2.loadLongPress[side] = true;
                DDJFLX2.loadTimer[side] = 0;
            },
            true
        );
    } else {
        // Button released: decide action based on press duration.
        if (DDJFLX2.loadTimer[side] !== 0) {
            engine.stopTimer(DDJFLX2.loadTimer[side]);
            DDJFLX2.loadTimer[side] = 0;
        }
        if (DDJFLX2.loadLongPress[side]) {
            DDJFLX2.loadLongPress[side] = false;
            script.triggerControl(deck.group, "eject", 100);
        } else {
            script.triggerControl(deck.group, "LoadSelectedTrack", 100);
        }
    }
};

// Shift + Headphone Cue (Deck 2 / Right):
// Cycles through available effects in slot 1 of the side's primary effect unit,
// i.e. the effect triggered by Pad FX pad 1 on this side.
// Like the Pad FX pads, the unit follows the physical side (left = Unit 1,
// right = Unit 2) in both deck layers.
DDJFLX2.fxSelect = function(channel, control, value, status, group) {
    if (!value) {
        return;
    }
    const deck = DDJFLX2.resolveDeck(group);
    engine.setValue(
        `[EffectRack1_EffectUnit${deck.physicalDeck}_Effect1]`,
        "next_effect",
        1
    );
};

DDJFLX2.browseTracks = function(value) {
    // Jog messages are offset-binary relative values centered on 0x40
    // (Pioneer MIDI message list: clockwise increases from 0x41,
    // counterclockwise decreases from 0x3F).
    const delta = value - 0x40;

    // Reset the accumulator on direction change so that small mechanical
    // jitter in the opposite direction does not build up.
    if ((delta > 0 && DDJFLX2.jogCounter < 0) ||
        (delta < 0 && DDJFLX2.jogCounter > 0)) {
        DDJFLX2.jogCounter = 0;
    }
    DDJFLX2.jogCounter += delta;

    // Move one row each time the accumulated movement exceeds the threshold.
    // Clockwise moves down the list, counterclockwise moves up.
    const threshold = DDJFLX2.browseJogThreshold;
    while (DDJFLX2.jogCounter >= threshold) {
        engine.setValue("[Library]", "MoveDown", true);
        DDJFLX2.jogCounter -= threshold;
    }
    while (DDJFLX2.jogCounter <= -threshold) {
        engine.setValue("[Library]", "MoveUp", true);
        DDJFLX2.jogCounter += threshold;
    }
};

// Proper press/release handling avoids stuck shift state.
DDJFLX2.shiftLeft = function(
    channel,
    control,
    value
) {
    DDJFLX2.shiftPressed.left = value > 0;
};

DDJFLX2.shiftRight = function(
    channel,
    control,
    value
) {
    DDJFLX2.shiftPressed.right = value > 0;
};

// Outer jog ring used for temporary pitch bending.
DDJFLX2.jogWheel = function(
    channel,
    control,
    value,
    status,
    group
) {
    // Shift + jog browses the library.
    if (DDJFLX2.isShiftPressed(group)) {
        DDJFLX2.browseTracks(value);
        return;
    }

    const deck = DDJFLX2.resolveDeck(group);
    engine.setValue(deck.group, "jog", (value - 64) * DDJFLX2.pitchShiftFactor);
};

// Shared helper for all scratch-related handlers.
DDJFLX2.doScratchTick = function(vDeckNo, value) {
    if (engine.isScratching(vDeckNo)) {
        engine.scratchTick(vDeckNo, value - 64);
    }
};

// Top platter surface used for scratching.
DDJFLX2.platterJog = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (DDJFLX2.isShiftPressed(group)) {
        DDJFLX2.browseTracks(value);
        return;
    }

    // While the platter is touched, keep scratching the deck where the touch
    // started, even if the deck layer was switched in the meantime.
    const deck = DDJFLX2.resolveDeck(group);
    const touchedDeck = DDJFLX2.heldDeck[`${deck.physicalDeck}:touch`] || deck;
    DDJFLX2.doScratchTick(touchedDeck.virtualDeck, value);
};

DDJFLX2.platterJogShift = function(
    channel,
    control,
    value,
    status,
    group
) {
    DDJFLX2.platterJog(channel, control, value, status, group);
};

// Touch sensor enables or disables scratching mode.
//
// NOTE: Scratching is inhibited if Shift is pressed to prevent the track
// from stopping when the user touches the platter to browse the library.
// Releasing the platter always disables scratching, even if Shift is held.
// Observed firmware behavior: when Shift is pressed while touching, the
// controller sends 0x67 ON; lifting the hand sends 0x67 OFF, while the
// pending 0x36 OFF is only sent when Shift is released. Ignoring the 0x67 OFF
// would keep the deck in scratch mode (track stopped) until Shift is released.
// Release goes to the deck where scratching started (see resolveHeldDeck).
DDJFLX2.touch = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (!value) {
        const deck = DDJFLX2.resolveHeldDeck(group, "touch", false);
        engine.scratchDisable(deck.virtualDeck);
        return;
    }

    if (DDJFLX2.isShiftPressed(group)) {
        return;
    }

    const deck = DDJFLX2.resolveHeldDeck(group, "touch", true);
    engine.scratchEnable(
        deck.virtualDeck,
        460,
        33 + 1 / 3,
        0.125,
        0.003
    );
};

DDJFLX2.touchShift = function(
    channel,
    control,
    value,
    status,
    group
) {
    DDJFLX2.touch(channel, control, value, status, group);
};

DDJFLX2.headmix = function(channel, control, value) {
    if (!value) {
        return;
    }

    // Toggle between cue and master monitoring extremes.
    const masterMixEnabled =
    engine.getValue("[Master]", "headMix") > 0.5;

    engine.setValue(
        "[Master]",
        "headMix",
        masterMixEnabled ? -1 : 1
    );
    // LED update is handled by the makeConnection callback in init().
};

DDJFLX2.toggleFourDeckMode = function(
    channel,
    control,
    value
) {
    if (!value) {
        return;
    }

    DDJFLX2.fourDeckMode = !DDJFLX2.fourDeckMode;

    // Release any active Pad FX overrides before changing deck layers.
    DDJFLX2.releaseAllPadFx();

    // Reset all LEDs first to avoid stale states.
    DDJFLX2.LEDsOff();

    if (DDJFLX2.fourDeckMode) {
        // Reassign physical decks to virtual decks 3 and 4.
        DDJFLX2.vDeckNo[1] = 3;
        DDJFLX2.vDeckNo[2] = 4;

        // Re-arm soft takeover for the newly active virtual decks.
        DDJFLX2.rearmSoftTakeover(3);
        DDJFLX2.rearmSoftTakeover(4);

        DDJFLX2.switchLEDs(3);
        DDJFLX2.switchLEDs(4);

        DDJFLX2.switchLoadedLED(
            3,
            engine.getValue("[Channel3]", "track_loaded")
        );

        DDJFLX2.switchLoadedLED(
            4,
            engine.getValue("[Channel4]", "track_loaded")
        );
    } else {
        // Restore default mapping.
        DDJFLX2.vDeckNo[1] = 1;
        DDJFLX2.vDeckNo[2] = 2;

        // Re-arm soft takeover for the restored virtual decks.
        DDJFLX2.rearmSoftTakeover(1);
        DDJFLX2.rearmSoftTakeover(2);

        DDJFLX2.switchLEDs(1);
        DDJFLX2.switchLEDs(2);

        DDJFLX2.switchLoadedLED(
            1,
            engine.getValue("[Channel1]", "track_loaded")
        );

        DDJFLX2.switchLoadedLED(
            2,
            engine.getValue("[Channel2]", "track_loaded")
        );
    }

    DDJFLX2.updateAllBlinkingLEDs();
    DDJFLX2.updateSmartFaderLED();
};

DDJFLX2.play = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (value) {
        const deck = DDJFLX2.resolveDeck(group);

        const playing = engine.getValue(deck.group, "play");

        engine.setValue(deck.group, "play", !playing);
        // LED update is handled by the makeConnection callback in init().
    }
};

// BEAT SYNC
//
// The firmware decides between short and long press itself and sends a
// different note for each (press and release arrive together):
//   short press 0x58, long press 0x5C, Shift + press 0x2A.
// NOTE: the Pioneer MIDI message list swaps 0x5C and 0x2A; the values above
// were verified on the hardware and match the DDJ-400 / DDJ-FLX4.
// Shift + BEAT SYNC (pad mode selection) is handled by the firmware.
//
// Behavior follows the Mixxx sync button convention (as in the DDJ-400 and
// DDJ-FLX4 mappings):
// - short press: one-time beat sync, or disable sync lock if it is enabled
// - long press:  enable sync lock
DDJFLX2.syncShort = function(
    channel,
    control,
    value,
    status,
    group
) {
    const deck = DDJFLX2.resolveDeck(group);
    if (engine.getValue(deck.group, "sync_enabled") && value > 0) {
        engine.setValue(deck.group, "sync_enabled", 0);
    } else {
        engine.setValue(deck.group, "beatsync", value);
    }
};

// Smart Fader button: toggles Fader Start on/off.
// When enabled, channel fader movements trigger play/pause
// on the corresponding deck via handleSmartFaderVolume().
DDJFLX2.smartFader = function(
    channel,
    control,
    value,
    _status,
    _group
) {
    if (!value) {
        return;
    }

    DDJFLX2.smartFaderEnabled = !DDJFLX2.smartFaderEnabled;
    DDJFLX2.updateSmartFaderLED();
};

DDJFLX2.smartFaderStart = function(deck) {
    // Prevent redundant calls if already playing
    if (engine.getValue(deck.group, "play")) { return; }
    engine.setValue(deck.group, "sync_enabled", 1);
    engine.setValue(deck.group, "play", 1);
};

DDJFLX2.smartFaderStop = function(deck) {
    engine.setValue(deck.group, "cue_gotoandstop", 1);
};

// Start/stop the deck currently assigned to the physical side only when its
// physical fader actually leaves or reaches the bottom position.
DDJFLX2.handleSmartFaderVolume = function(deck, volume) {
    const wasDown = DDJFLX2.faderDown[deck.physicalDeck];
    const isDown = volume <= 0;

    if (DDJFLX2.smartFaderEnabled) {
        if (wasDown && !isDown) {
            DDJFLX2.smartFaderStart(deck);
        } else if (!wasDown && isDown) {
            DDJFLX2.smartFaderStop(deck);
        }
    }

    DDJFLX2.faderDown[deck.physicalDeck] = isDown;
};

// BEAT SYNC long press: enable sync lock. See syncShort for details.
DDJFLX2.syncLong = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (value) {
        const deck = DDJFLX2.resolveDeck(group);
        engine.setValue(deck.group, "sync_enabled", 1);
    }
};

// Store pitch fader MSB for 14-bit MIDI reconstruction.
DDJFLX2.rateMSB = function(
    channel,
    control,
    value,
    status,
    group
) {
    const deck = DDJFLX2.resolveDeck(group);
    DDJFLX2.hwMSB[deck.physicalDeck].rate = value;
};

// Combine MSB and LSB to reconstruct full 14-bit pitch value.
DDJFLX2.rateLSB = function(
    channel,
    control,
    value,
    status,
    group
) {
    const deck = DDJFLX2.resolveDeck(group);

    const rateMSB = DDJFLX2.hwMSB[deck.physicalDeck].rate || 0;
    // Hardware encodes rate as 0x0000 (top, +max) → 0x3FFE (bottom, -max).
    // 0x3FFE is 16382, the maximum 14-bit value sent by the FLX2 pitch fader.
    // The rate parameter maps 0..1 to rate -1..+1, so the slider top (+max)
    // corresponds to parameter 1. Soft takeover is applied by Mixxx.
    const raw14 = (rateMSB << 7) + value;
    const hw01 = raw14 / 0x3ffe;   // 0 = pitch slider at top (+), 1 = bottom (-)
    engine.setParameter(deck.group, "rate", 1 - hw01);
};

// Store volume fader MSB for 14-bit MIDI reconstruction.
DDJFLX2.volumeMSB = function(
    channel,
    control,
    value,
    status,
    group
) {
    const deck = DDJFLX2.resolveDeck(group);
    DDJFLX2.hwMSB[deck.physicalDeck].volume = value;
};

// Combine MSB and LSB to reconstruct full 14-bit volume value.
DDJFLX2.volumeLSB = function(
    channel,
    control,
    value,
    status,
    group
) {
    const deck = DDJFLX2.resolveDeck(group);

    const volMSB = DDJFLX2.hwMSB[deck.physicalDeck].volume || 0;
    const vol = ((volMSB << 7) + value) / 0x3fff;

    // Track the physical fader position for Smart Fader regardless of soft
    // takeover. setParameter applies the volume control's audio taper curve,
    // matching the on-screen fader. Soft takeover is applied by Mixxx.
    DDJFLX2.handleSmartFaderVolume(deck, vol);
    engine.setParameter(deck.group, "volume", vol);
};

// Store EQ MSB for 14-bit reconstruction.
// Deck is derived from the MIDI status byte (0xB0 = deck 1, 0xB1 = deck 2)
// because script.deckFromGroup() does not work with EQ rack group names.
DDJFLX2.eqMSB = function(channel, control, value, status, _group) {
    const physicalDeck = (status & 0x0F) + 1;
    DDJFLX2.hwMSB[physicalDeck][control] = value;
};

// Combine MSB and LSB to reconstruct full 14-bit EQ value.
DDJFLX2.eqLSB = function(channel, control, value, status, _group) {
    const physicalDeck = (status & 0x0F) + 1;
    const virtualDeck = DDJFLX2.vDeckNo[physicalDeck];
    const vgroup = `[Channel${virtualDeck}]`;
    const eqGroup = `[EqualizerRack1_${vgroup}_Effect1]`;

    const msb = DDJFLX2.hwMSB[physicalDeck][control - 0x20] || 0;
    const rawValue = (msb << 7) + value;
    const combined = rawValue / 0x3fff;

    // 0x27 = High (parameter3), 0x2B = Mid (parameter2), 0x2F = Low (parameter1).
    let eq;
    if (control === 0x27) {
        eq = 3;
    } else if (control === 0x2B) {
        eq = 2;
    } else {
        eq = 1;
    }

    // setParameter applies the EQ parameter's own scaling, matching the
    // on-screen knob. Soft takeover is applied by Mixxx.
    engine.setParameter(eqGroup, `parameter${eq}`, combined);
};

// Store CFX MSB for 14-bit reconstruction.
DDJFLX2.super1MSB = function(channel, control, value, _status, _group) {
    // 0xB6 0x17 = CFX CH1 MSB, 0xB6 0x18 = CFX CH2 MSB.
    const physicalDeck = control === 0x17 ? 1 : 2;
    DDJFLX2.hwMSB[physicalDeck].cfx = value;
};

// Combine MSB and LSB to reconstruct full 14-bit CFX value.
DDJFLX2.super1LSB = function(channel, control, value, _status, _group) {
    // 0xB6 0x37 = CFX CH1 LSB, 0xB6 0x38 = CFX CH2 LSB.
    const physicalDeck = control === 0x37 ? 1 : 2;
    const virtualDeck = DDJFLX2.vDeckNo[physicalDeck];
    const vgroup = `[Channel${virtualDeck}]`;
    const qfxGroup = `[QuickEffectRack1_${vgroup}]`;

    const msb = DDJFLX2.hwMSB[physicalDeck].cfx || 0;
    const rawValue = (msb << 7) + value;
    const combined = rawValue / 0x3fff;

    let mode;
    if (DDJFLX2.cfxKnobBehavior === "filter") {
        mode = "cfx";
    } else if (DDJFLX2.cfxKnobBehavior === "gain") {
        mode = "gain";
    } else { // "dual"
        mode = DDJFLX2.isShiftPressed(vgroup) ? "gain" : "cfx";
    }

    const lastMode = DDJFLX2.vDeck[virtualDeck].cfxLastMode || "cfx";

    // When the knob switches between filter and gain (Dual mode + Shift),
    // re-arm soft takeover on the newly targeted control so the knob has to
    // reach (or cross) its current value before taking over.
    if (mode !== lastMode) {
        DDJFLX2.vDeck[virtualDeck].cfxLastMode = mode;
        if (mode === "gain") {
            engine.softTakeoverIgnoreNextValue(vgroup, "pregain");
        } else {
            engine.softTakeoverIgnoreNextValue(qfxGroup, "super1");
        }
    }

    // setParameter applies each control's own curve (audio taper for gain),
    // matching the on-screen knobs. Soft takeover is applied by Mixxx.
    if (mode === "gain") {
        engine.setParameter(vgroup, "pregain", combined);
    } else {
        engine.setParameter(qfxGroup, "super1", combined);
    }
};

// CUE button: forwards press and release to cue_default, so the behavior
// follows the cue mode selected in Preferences (Mixxx, Pioneer, Denon, ...),
// including cue preview while held. Same approach as the DDJ-400 / DDJ-FLX4.
// Cue and play LEDs are updated by the makeConnection callbacks in init().
// Release goes to the deck where the press started (see resolveHeldDeck).
DDJFLX2.cueDefault = function(
    channel,
    control,
    value,
    status,
    group
) {
    const deck = DDJFLX2.resolveHeldDeck(group, "cue", value > 0);
    engine.setValue(deck.group, "cue_default", value ? 1 : 0);
};

// Shift + PLAY: set the cue point at the current position of the virtual deck
// assigned to this side.
DDJFLX2.cueSet = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (value) {
        const deck = DDJFLX2.resolveDeck(group);
        engine.setValue(deck.group, "cue_set", 1);
    }
};

DDJFLX2.cueGotoandstop = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (value) {
        const deck = DDJFLX2.resolveDeck(group);

        engine.setValue(deck.group, "cue_gotoandstop", true);
        // Play LED update is handled by the makeConnection callback in init().
    }
};

// Trigger or create hotcues.
// LED updates are handled exclusively by the makeConnection callback in init()
// to avoid double-writes and race conditions.
DDJFLX2.hotcueNActivate = function(
    channel,
    control,
    value,
    status,
    group
) {
    const hotcueNum = DDJFLX2.hotcuePadFromControl(control);
    const hotcue = `hotcue_${hotcueNum}`;
    // Release goes to the deck where the press started (see resolveHeldDeck).
    const deck = DDJFLX2.resolveHeldDeck(group, hotcue, value > 0);

    // Pass both press (1) and release (0) so Mixxx can manage the
    // preview-while-held behavior when the deck is paused.
    engine.setValue(deck.group, `${hotcue}_activate`, value ? 1 : 0);

    if (value) {
        // Update play LED immediately for responsiveness.
        midi.sendShortMsg(
            0x90 + deck.physicalDeck - 1,
            0x0b,
            0x7f * engine.getValue(deck.group, "play")
        );
    }
    // NOTE: hotcue pad LED is handled by the makeConnection on hotcue_X_status.
};

// Clear hotcues.
DDJFLX2.hotcueNClear = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (!value) {
        return;
    }

    const deck = DDJFLX2.resolveDeck(group);
    const hotcueNum = DDJFLX2.hotcuePadFromControl(control);

    engine.setValue(
        deck.group,
        `hotcue_${hotcueNum}_clear`,
        true
    );

    // Turn off the pad LED using the correct MIDI-OUT channel.
    midi.sendShortMsg(
        DDJFLX2.getPadLedStatus(deck.physicalDeck, false),
        control,
        0x00
    );
};

// Restore any active Pad FX overrides to their original state and clear tracking.
DDJFLX2.releaseAllPadFx = function() {
    for (const stateKey of Object.keys(DDJFLX2.padFxState)) {
        const saved = DDJFLX2.padFxState[stateKey];
        if (saved) {
            engine.setValue(saved.effectGroup, "enabled", saved.effectEnabled);
            engine.setValue(saved.effectGroup, "meta", saved.meta);
        }
    }
    DDJFLX2.padFxState = {};

    for (const fxGroup of Object.keys(DDJFLX2.padFxUnitState)) {
        const unitSaved = DDJFLX2.padFxUnitState[fxGroup];
        if (unitSaved) {
            engine.setValue(fxGroup, "enabled", unitSaved.enabled);
            engine.setValue(fxGroup, "mix", unitSaved.mix);
            for (let i = 1; i <= 4; i++) {
                engine.setValue(fxGroup, `group_[Channel${i}]_enable`, unitSaved.routing[i]);
            }
        }
    }
    DDJFLX2.padFxUnitState = {};
};

DDJFLX2.applyPadFx = function(control, value, status, group, shifted) {
    const pad = DDJFLX2.padFxPadFromControl(control);
    const deck = DDJFLX2.resolveDeck(group);

    // Pads 4 and 8 are not handled in this mode.
    if (pad === 4 || pad === 8) {
        return;
    }

    // Map active pads to effect unit and slot:
    // Pad 1,2,3 → unit A, slot 1,2,3
    // Pad 5,6,7 → unit B, slot 1,2,3
    // Physical deck 1 (left):  unit A = 1, unit B = 3
    // Physical deck 2 (right): unit A = 2, unit B = 4
    const padToUnitSlot = {
        1: {unitOffset: 0, slot: 1},
        2: {unitOffset: 0, slot: 2},
        3: {unitOffset: 0, slot: 3},
        5: {unitOffset: 1, slot: 1},
        6: {unitOffset: 1, slot: 2},
        7: {unitOffset: 1, slot: 3},
    };
    const unitOffset = padToUnitSlot[pad].unitOffset;
    const slot = padToUnitSlot[pad].slot;

    let unit;
    if (deck.physicalDeck === 1) {
        unit = unitOffset === 0 ? 1 : 3;
    } else {
        unit = unitOffset === 0 ? 2 : 4;
    }

    const fxGroup = `[EffectRack1_EffectUnit${unit}]`;
    const effectGroup = `[EffectRack1_EffectUnit${unit}_Effect${slot}]`;
    const stateKey = `${deck.group}:${unit}:${slot}`;

    const ledStatus = DDJFLX2.getPadLedStatus(deck.physicalDeck, shifted);

    // TOGGLE MODE: each press flips the effect slot enabled state; release is a no-op.
    if (DDJFLX2.padFxMode === "toggle") {
        if (!value) {
            return;
        }
        const nowEnabled = !engine.getValue(effectGroup, "enabled");
        engine.setValue(effectGroup, "enabled", nowEnabled ? 1 : 0);
        midi.sendShortMsg(ledStatus, control, nowEnabled ? 0x7f : 0x00);
        return;
    }

    // OVERRIDE MODE (default): hold to apply, restore on release.

    // BUTTON RELEASED: Restore original effect state.
    // We read the values we saved when the button was first pressed and
    // re-apply them to the effect slot and unit, effectively "cleaning up" after the FX.
    if (!value) {
        const saved = DDJFLX2.padFxState[stateKey];
        if (saved) {
            engine.setValue(saved.effectGroup, "enabled", saved.effectEnabled);
            engine.setValue(saved.effectGroup, "meta", saved.meta);
            delete DDJFLX2.padFxState[stateKey];
        }

        // Check if any other override pads are still held for this shared effect unit.
        const hasOtherActivePads = Object.values(DDJFLX2.padFxState).some(
            function(s) {
                return s.fxGroup === fxGroup;
            }
        );

        // When the final held pad for this unit is released, restore the shared unit baseline.
        if (!hasOtherActivePads) {
            const unitSaved = DDJFLX2.padFxUnitState[fxGroup];
            if (unitSaved) {
                engine.setValue(fxGroup, "enabled", unitSaved.enabled);
                engine.setValue(fxGroup, "mix", unitSaved.mix);
                for (let i = 1; i <= 4; i++) {
                    engine.setValue(fxGroup, `group_[Channel${i}]_enable`, unitSaved.routing[i]);
                }
                delete DDJFLX2.padFxUnitState[fxGroup];
            }
        }

        midi.sendShortMsg(ledStatus, control, 0x00);
        return;
    }

    // BUTTON PRESSED: Snapshot current state and apply FX.
    // 1. Snapshot shared effect-unit state once, before its first active override.
    // Before we take over the effect unit, we save its current configuration
    // (which decks it's routed to, its mix level, etc.) so we can restore it later.
    if (!DDJFLX2.padFxUnitState[fxGroup]) {
        const routing = {};
        for (let i = 1; i <= 4; i++) {
            routing[i] = engine.getValue(fxGroup, `group_[Channel${i}]_enable`);
        }
        DDJFLX2.padFxUnitState[fxGroup] = {
            enabled: engine.getValue(fxGroup, "enabled"),
            mix: engine.getValue(fxGroup, "mix"),
            routing: routing,
        };
    }

    // 2. Snapshot per-slot state.
    if (!DDJFLX2.padFxState[stateKey]) {
        DDJFLX2.padFxState[stateKey] = {
            fxGroup: fxGroup,
            effectGroup: effectGroup,
            effectEnabled: engine.getValue(effectGroup, "enabled"),
            meta: engine.getValue(effectGroup, "meta"),
        };
    }

    // Force-route the effect unit to the current deck and maximize its presence.
    for (let i = 1; i <= 4; i++) {
        engine.setValue(fxGroup, `group_[Channel${i}]_enable`, 0);
    }
    engine.setValue(fxGroup, `group_${deck.group}_enable`, 1);
    engine.setValue(fxGroup, "enabled", 1);
    engine.setValue(fxGroup, "mix", 1);
    engine.setValue(effectGroup, "meta", DDJFLX2.padFxMeta);
    engine.setValue(effectGroup, "enabled", 1);

    midi.sendShortMsg(ledStatus, control, 0x7f);
};

DDJFLX2.padFx = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (DDJFLX2.beatJumpPadMode === "padfx") {
        DDJFLX2.applyBeatJump(control, value, status, group, DDJFLX2.padFxPadFromControl(control), false);
        return;
    }
    DDJFLX2.applyPadFx(control, value, status, group, false);
};

DDJFLX2.padFxShift = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (DDJFLX2.beatJumpPadMode === "padfx") {
        DDJFLX2.applyBeatJump(control, value, status, group, DDJFLX2.padFxPadFromControl(control), true);
        return;
    }
    DDJFLX2.applyPadFx(control, value, status, group, true);
};

DDJFLX2.loopRollPad = function(
    channel,
    control,
    value,
    status,
    group
) {
    const pad = DDJFLX2.loopPadFromControl(control);
    const size = DDJFLX2.loopPadSizes[pad - 1];
    // Release goes to the deck where the roll started (see resolveHeldDeck).
    const deck = DDJFLX2.resolveHeldDeck(group, `roll${pad}`, value > 0);

    engine.setValue(
        deck.group,
        `beatlooproll_${size}_activate`,
        value ? 1 : 0
    );

    // Use the correct MIDI-OUT channel for loop roll (shifted pad mode).
    midi.sendShortMsg(
        DDJFLX2.getPadLedStatus(deck.physicalDeck, true),
        control,
        value ? 0x7f : 0x00
    );
};

// Update all blinking LEDs in one pass: sampler pads and loop pads.
// Called by the global 200ms timer so all blinks stay in sync.
DDJFLX2.updateAllBlinkingLEDs = function() {
    DDJFLX2.updateAllSamplerLEDs();
    DDJFLX2.updateAllLoopBlinkLEDs();
};

// Send a pad LED value only if it differs from the last value sent.
// Used by the blink timer to avoid flooding the controller with MIDI messages.
DDJFLX2.sendPadLedCached = function(status, control, value) {
    const key = (status << 8) | control;
    if (DDJFLX2.padLedCache[key] === value) {
        return;
    }
    DDJFLX2.padLedCache[key] = value;
    midi.sendShortMsg(status, control, value);
};

// Helper: return true if the sampler exists in the current Mixxx setup.
// Reading controls of a missing sampler logs "Unknown control" warnings.
DDJFLX2.samplerExists = function(samplerNo) {
    return samplerNo <= engine.getValue("[App]", "num_samplers");
};

// Update all 16 sampler pad LEDs.
// Physical deck 1 (left)  → samplers 1–8,  pad MIDI-OUT channel 0x97
// Physical deck 2 (right) → samplers 9–16, pad MIDI-OUT channel 0x99
// Three states per pad:
//   empty   → LED off
//   loaded  → LED on (steady)
//   playing → LED blinks with blinkPhase
// Pads of samplers that do not exist stay off.
DDJFLX2.updateAllSamplerLEDs = function() {
    // Beat jump owns the sampler pad LEDs when it replaces that mode.
    if (DDJFLX2.beatJumpPadMode === "sampler") {
        return;
    }

    for (let physicalDeck = 1; physicalDeck <= 2; physicalDeck++) {
        const ledStatus = DDJFLX2.getPadLedStatus(physicalDeck, false);
        for (let pad = 1; pad <= 8; pad++) {
            const samplerNo = pad + (physicalDeck - 1) * 8;

            let ledValue = 0x00;
            if (DDJFLX2.samplerExists(samplerNo)) {
                const samplerGroup = `[Sampler${samplerNo}]`;
                if (engine.getValue(samplerGroup, "track_loaded")) {
                    if (engine.getValue(samplerGroup, "play")) {
                        ledValue = DDJFLX2.blinkPhase ? 0x7f : 0x00;
                    } else {
                        ledValue = 0x7f;
                    }
                }
            }

            // Sampler pads use control bytes 0x30–0x37 (pad 1 = 0x30).
            DDJFLX2.sendPadLedCached(ledStatus, 0x30 + (pad - 1), ledValue);
        }
    }
};

// Update loop pad LEDs for all visible virtual decks.
// The pad of the active beatloop size blinks with blinkPhase; all others are off.
// Loop state is read from Mixxx (beatloop_X_enabled), so loops activated or
// resized from the GUI are shown as well.
DDJFLX2.updateAllLoopBlinkLEDs = function() {
    for (let physicalDeck = 1; physicalDeck <= 2; physicalDeck++) {
        const vgroup = `[Channel${DDJFLX2.vDeckNo[physicalDeck]}]`;

        for (let pad = 1; pad <= 8; pad++) {
            const size = DDJFLX2.loopPadSizes[pad - 1];
            const isActive = engine.getValue(vgroup, `beatloop_${size}_enabled`);
            const ledValue = isActive ? (DDJFLX2.blinkPhase ? 0x7f : 0x00) : 0x00;
            // Loop pads use control bytes 0x60–0x67 (pad 1 = 0x60).
            DDJFLX2.sendPadLedCached(
                DDJFLX2.getPadLedStatus(physicalDeck, false),
                0x60 + (pad - 1),
                ledValue
            );
        }
    }
};

// Shared handler for beat jump pad mode.
// Odd pads jump backward, even pads jump forward.
// Pad pairing: 1&2 -> 1 beat, 3&4 -> 2 beats, 5&6 -> 4 beats, 7&8 -> 8 beats.
DDJFLX2.applyBeatJump = function(control, value, status, group, padNum, shifted) {
    const deck = DDJFLX2.resolveDeck(group);
    const ledStatus = DDJFLX2.getPadLedStatus(deck.physicalDeck, shifted);

    if (!value) {
        midi.sendShortMsg(ledStatus, control, 0x00);
        return;
    }

    const isForward = padNum % 2 === 0;
    const pairIndex = Math.floor((padNum - 1) / 2);
    const size = (isForward ? 1 : -1) * DDJFLX2.beatJumpSizes[pairIndex] * (shifted ? 2 : 1);

    engine.setValue(deck.group, "beatjump", size);
    midi.sendShortMsg(ledStatus, control, 0x7f);
};

DDJFLX2.samplerPad = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (DDJFLX2.beatJumpPadMode === "sampler") {
        DDJFLX2.applyBeatJump(control, value, status, group, DDJFLX2.samplerPadFromControl(control), false);
        return;
    }

    if (!value) {
        return;
    }

    const pad = DDJFLX2.samplerPadFromControl(control);
    const deck = DDJFLX2.resolveDeck(group);
    const samplerNo = pad + (deck.physicalDeck - 1) * 8;
    if (!DDJFLX2.samplerExists(samplerNo)) {
        return;
    }
    const samplerGroup = `[Sampler${samplerNo}]`;

    script.triggerControl(samplerGroup, "cue_gotoandplay");
    // LED update is handled by the global sampler timer.
};

DDJFLX2.samplerStopPad = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (DDJFLX2.beatJumpPadMode === "sampler") {
        DDJFLX2.applyBeatJump(control, value, status, group, DDJFLX2.samplerPadFromControl(control), true);
        return;
    }

    if (!value) {
        return;
    }

    const pad = DDJFLX2.samplerPadFromControl(control);
    const deck = DDJFLX2.resolveDeck(group);
    const samplerNo = pad + (deck.physicalDeck - 1) * 8;
    if (!DDJFLX2.samplerExists(samplerNo)) {
        return;
    }
    const samplerGroup = `[Sampler${samplerNo}]`;

    script.triggerControl(samplerGroup, "stop");
    // LED update is handled by the global sampler timer.
};

DDJFLX2.pfl = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (value) {
        const deck = DDJFLX2.resolveDeck(group);

        const pfl = !engine.getValue(deck.group, "pfl");

        engine.setValue(deck.group, "pfl", pfl);
        // LED update is handled by the makeConnection callback in init().
    }
};

// Refresh all LEDs for a specific virtual deck.
// No-op when the virtual deck is not currently assigned to its physical
// side, preventing stale writes that would corrupt the visible deck's LEDs.
DDJFLX2.switchLEDs = function(vDeckNo) {
    if (!DDJFLX2.isVirtualDeckVisible(vDeckNo)) {
        return;
    }

    const d = DDJFLX2.virtualToLedIndex(vDeckNo);
    const physicalDeck = DDJFLX2.virtualToPhysicalDeck(vDeckNo);
    const vgroup = `[Channel${vDeckNo}]`;

    DDJFLX2.switchPlayLED(
        d,
        engine.getValue(vgroup, "play")
    );

    DDJFLX2.switchCueLED(
        d,
        engine.getValue(vgroup, "cue_indicator") ? 1 : 0
    );

    DDJFLX2.switchSyncLED(
        d,
        engine.getValue(vgroup, "sync_enabled")
    );

    midi.sendShortMsg(
        0x90 + d,
        0x54,
        0x7f * engine.getValue(vgroup, "pfl")
    );

    for (let i = 1; i <= 8; i++) {
        const isButtonEnabled = engine.getValue(
            vgroup,
            `hotcue_${i}_status`
        );

        // Use physicalDeck (not ledIndex) and correct MIDI-OUT pad channel.
        DDJFLX2.switchPadLED(physicalDeck, i, isButtonEnabled, false);
    }

    // Update loop pad LEDs (single source of truth).
    DDJFLX2.updateLoopPadLEDs(vDeckNo);
};

DDJFLX2.switchPlayLED = function(deck, enabled) {
    midi.sendShortMsg(0x90 + deck, 0x0b, 0x7f * enabled);
};

DDJFLX2.switchSyncLED = function(deck, enabled) {
    midi.sendShortMsg(0x90 + deck, 0x58, 0x7f * enabled);
    midi.sendShortMsg(0x90 + deck, 0x78, 0x7f * enabled);
};

// Send a pad LED update using the correct MIDI-OUT illumination channel.
// physicalDeck: 1 (left) or 2 (right).
// pad: 1-based pad number.
// enabled: truthy = LED on, falsy = LED off.
// shifted: true = use shifted pad LED channel (0x98 / 0x9A).
DDJFLX2.switchPadLED = function(physicalDeck, pad, enabled, shifted) {
    const ledStatus = DDJFLX2.getPadLedStatus(physicalDeck, shifted || false);
    midi.sendShortMsg(ledStatus, pad - 1, enabled ? 0x7F : 0x00);
};

DDJFLX2.switchCueLED = function(deck, enabled) {
    midi.sendShortMsg(0x90 + deck, 0x0c, 0x7f * enabled);
};

// Helper: turn off all loop pad LEDs for a specific physical deck.
DDJFLX2.turnOffAllLoopPadLEDs = function(physicalDeck) {
    const ledStatus = DDJFLX2.getPadLedStatus(physicalDeck, false);
    for (let pad = 1; pad <= 8; pad++) {
        midi.sendShortMsg(ledStatus, 0x60 + (pad - 1), 0x00);
    }

    // Force the blink timer to repaint loop and sampler pads.
    DDJFLX2.padLedCache = {};
};

// Update all loop pad LEDs for a specific virtual deck (single source of truth).
// Called on loop state changes and deck switches.
// The active loop pad blink is driven by updateAllLoopBlinkLEDs via the global
// timer — this function only needs to clear all pads so the timer can repaint.
// No-op when the virtual deck is not currently assigned to its physical side.
DDJFLX2.updateLoopPadLEDs = function(vDeckNo) {
    if (!DDJFLX2.isVirtualDeckVisible(vDeckNo)) {
        return;
    }

    const physicalDeck = DDJFLX2.virtualToPhysicalDeck(vDeckNo);
    DDJFLX2.turnOffAllLoopPadLEDs(physicalDeck);
};

DDJFLX2.loopPad = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (!value) {
        return;
    }

    const pad = DDJFLX2.loopPadFromControl(control);
    const size = DDJFLX2.loopPadSizes[pad - 1];
    const deck = DDJFLX2.resolveDeck(group);

    // beatloop_X_toggle activates a beatloop of this size, or disables it if
    // a loop of this size is already active. LEDs are repainted by the
    // global blink timer from beatloop_X_enabled.
    script.triggerControl(deck.group, `beatloop_${size}_toggle`);
};
