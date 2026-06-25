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

    // Per-deck runtime storage.
    // Used for temporary values such as 14-bit MIDI reconstruction.
    vDeck: {},

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

    // Accumulates jog movement while browsing the library.
    jogCounter: 0,

    // Loop state: keeps track of currently active loop for each virtual deck.
    activeLoopSize: {},

    // Manual soft takeover state for script-bound controls (rate, volume, EQ, CFX).
    //
    // WHY THIS IS NEEDED:
    // When switching between deck layers (e.g., Deck 1 to Deck 3), physical faders
    // stay at their current position while the virtual deck in Mixxx might have
    // a completely different value. Since these controls are updated via
    // engine.setValue() in JavaScript, Mixxx's built-in <soft-takeover/> XML
    // option is bypassed.
    //
    // THE SOLUTION:
    // We implement a manual "catch-up" logic. On every layer switch, we "lock"
    // the control and snapshot the current engine value. The physical fader must
    // "cross" or reach this snapshot value (within SOFT_TAKEOVER_TOLERANCE)
    // before the script starts sending updates to Mixxx again. This prevents
    // sudden, immersion-breaking jumps in volume or pitch during a transition.
    //
    // softTakeover[physicalDeck][controlKey] = { locked: bool, target: 0..1 }
    softTakeover: {1: {}, 2: {}},

    // Fractional tolerance (0..1 scale) within which a hardware position is
    // considered "close enough" to unlock soft takeover immediately.
    // 0.04 corresponds to roughly 5 units on a 0..127 7-bit scale.
    SOFT_TAKEOVER_TOLERANCE: 0.04,

    // Helper: check if shift is pressed for a specific deck group.
    isShiftPressed: function(group) {
        const deck = script.deckFromGroup(group);
        return deck === 1 ? this.shiftPressed.left : this.shiftPressed.right;
    },

    // Helper: resolve physical deck, virtual deck, and virtual deck group from a group.
    resolveDeck: function(group) {
        const physicalDeck = script.deckFromGroup(group);
        const virtualDeck = this.vDeckNo[physicalDeck];
        return {
            physicalDeck: physicalDeck,
            virtualDeck: virtualDeck,
            group: `[Channel${  virtualDeck  }]`
        };
    },

    // Helper: return stable side key for per-physical-deck state.
    deckSide: function(physicalDeck) {
        return physicalDeck === 1 ? "left" : "right";
    },

    // Helper: snapshot soft takeover targets for one physical deck side after a
    // layer switch. Called by toggleFourDeckMode after vDeckNo[] is updated.
    //
    // For each scripted control we read the current engine value on the newly
    // active virtual deck and store it as the target the hardware must reach
    // before updates are applied. If the hardware position is already within
    // SOFT_TAKEOVER_TOLERANCE the lock is released immediately so that controls
    // already near their target do not feel sticky.
    //
    // physicalDeck : 1 (left) or 2 (right)
    // hwRaw        : object with current hardware raw values for this side,
    //                { rate, volume, eqHigh, eqMid, eqLow, cfx } all in 0..1.
    //                Pass null to skip immediate-release optimisation (unknown
    //                hardware position).
    snapshotSoftTakeover: function(physicalDeck, hwRaw) {
        const vDeckNo  = this.vDeckNo[physicalDeck];
        const vgroup   = `[Channel${  vDeckNo  }]`;
        const eqGroup  = `[EqualizerRack1_${  vgroup  }_Effect1]`;
        const qfxGroup = `[QuickEffectRack1_${  vgroup  }]`;

        // EQ engine values are on the absoluteNonLin 0..4 scale (where 1.0 is unity).
        // We normalise this to a linear 0..1 scale for soft takeover comparison:
        // - 0.0 engine maps to 0.0 normalised
        // - 1.0 engine (unity) maps to 0.25 normalised
        // - 4.0 engine (max) maps to 1.0 normalised
        const eqNorm = function(v) { return v / 4; };
        // CFX engine value is already 0..1 from super1LSB.
        const cfxNorm = function(v) { return v; };

        const targets = {
            rate: engine.getValue(vgroup,   "rate")   * 0.5 + 0.5, // rate -1..1 → 0..1
            volume: engine.getValue(vgroup,   "volume"),
            eqHigh: eqNorm(engine.getValue(eqGroup, "parameter3")),
            eqMid: eqNorm(engine.getValue(eqGroup, "parameter2")),
            eqLow: eqNorm(engine.getValue(eqGroup, "parameter1")),
            cfx: cfxNorm(engine.getValue(qfxGroup, "super1")),
        };

        const tol = this.SOFT_TAKEOVER_TOLERANCE;
        const st  = this.softTakeover[physicalDeck];

        for (const key in targets) {
            const target  = targets[key];
            const hw      = hwRaw ? hwRaw[key] : null;
            const already = hw !== null && Math.abs(hw - target) <= tol;
            st[key] = {locked: !already, target: target};
        }
    },

    // Helper: check and update soft takeover gate for one scripted control.
    // Returns true if the value should be applied, false if it is still locked.
    //
    // physicalDeck : 1 or 2
    // controlKey   : one of the keys used in snapshotSoftTakeover
    // hwValue      : current hardware value, normalised to 0..1
    checkSoftTakeover: function(physicalDeck, controlKey, hwValue) {
        const st = this.softTakeover[physicalDeck][controlKey];
        if (!st || !st.locked) {
            return true; // no lock active, apply normally
        }
        // Release once the physical position reaches the target.
        if (Math.abs(hwValue - st.target) <= this.SOFT_TAKEOVER_TOLERANCE) {
            st.locked = false;
            return true;
        }
        return false; // still locked, discard this update
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

    for (let i = 1; i <= 4; i++) {
    // Create runtime storage for each virtual deck.
        this.vDeck[i] = {
            volMSB: 0,
            rateMSB: 0,
            volumeFaderDown: true,
        };

        // Initialize active loop size for each virtual deck.
        this.activeLoopSize[i] = null;

        const vgroup = `[Channel${  i  }]`;

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

        // Keep pad LEDs synchronized with hotcue state.
        // This is the single source of truth for hotcue LED updates;
        // hotcueNActivate must NOT send its own LED echo.
        for (let j = 1; j <= 8; j++) {
            engine.makeConnection(
                vgroup,
                `hotcue_${  j  }_enabled`,
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

        // Keep loop LEDs synchronized with actual loop state.
        engine.makeConnection(
            vgroup,
            "loop_enabled",
            function(enabled, vgroup) {
                const vDeckNo = script.deckFromGroup(vgroup);

                // Loop disabled externally
                if (!enabled) {
                    DDJFLX2.activeLoopSize[vDeckNo] = null;
                }

                DDJFLX2.updateLoopPadLEDs(vDeckNo);
            }
        );

        // Enable Pioneer CDJ cue behavior.
        engine.setValue(vgroup, "cue_cdj", true);
    }

    DDJFLX2.allLEDsOff();

    // Per the Pioneer MIDI reference, Vinyl mode cannot be changed from the
    // hardware. It is controlled by software with MIDI OUT 9n 17 hh.
    // Keep it enabled so platter rotation uses the scratch-capable 0x22 path.
    DDJFLX2.setVinylMode(true);

    // Focus the library after startup.
    // A short delay avoids initialization timing issues.
    engine.beginTimer(
        500,
        function() {
            engine.setValue("[Library]", "MoveFocus", 1);
        },
        true
    );

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
            group: `[Channel${  DDJFLX2.vDeckNo[selectedPhysicalDeck]  }]`
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
// Cycles through available effects in the primary effect unit for this deck.
// Deck 2 -> Unit 2, Deck 4 -> Unit 4.
DDJFLX2.fxSelect = function(channel, control, value, status, group) {
    if (!value) {
        return;
    }
    const deck = DDJFLX2.resolveDeck(group);
    // Cycle effects in the first slot of the primary unit for this deck.
    engine.setValue(
        `[EffectRack1_EffectUnit${  deck.virtualDeck  }_Effect1]`,
        "next_effect",
        1
    );
};

DDJFLX2.browseTracks = function(value) {
    // Convert relative MIDI encoder values into signed movement.
    const delta = value > 64 ? value - 128 : value;

    // Ignore micro-movements to filter out accidental touches
    // and mechanical noise from the jog wheel.
    // Threshold of 15 filters out all but deliberate movements.
    if (Math.abs(delta) < 15) {
        return;
    }

    // Accumulate intentional movements only.
    DDJFLX2.jogCounter += delta;

    // Threshold of 350 requires a firm, deliberate wheel turn
    // before triggering a single row move in the library.
    if (DDJFLX2.jogCounter > 350) {
        engine.setValue("[Library]", "MoveUp", true);
        DDJFLX2.jogCounter = 0;
    } else if (DDJFLX2.jogCounter < -350) {
        engine.setValue("[Library]", "MoveDown", true);
        DDJFLX2.jogCounter = 0;
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

    const deck = DDJFLX2.resolveDeck(group);
    DDJFLX2.doScratchTick(deck.virtualDeck, value);
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

// Additional scratch input path exposed by the controller.
DDJFLX2.scratch = function(
    channel,
    control,
    value,
    _status,
    group
) {
    const deck = DDJFLX2.resolveDeck(group);

    DDJFLX2.doScratchTick(deck.virtualDeck, value);
};

// Touch sensor enables or disables scratching mode.
//
// NOTE: Scratching is inhibited if Shift is pressed to prevent the track
// from stopping when the user touches the platter to browse the library.
DDJFLX2.touch = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (DDJFLX2.isShiftPressed(group)) {
        return;
    }

    const deck = DDJFLX2.resolveDeck(group);

    if (value) {
        engine.scratchEnable(
            deck.virtualDeck,
            460,
            33 + 1 / 3,
            0.125,
            0.003
        );
    } else {
        engine.scratchDisable(deck.virtualDeck);
    }
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

    midi.sendShortMsg(
        0x96,
        0x63,
        masterMixEnabled ? 0 : 0x7f
    );
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

    // Reset all LEDs first to avoid stale states.
    DDJFLX2.LEDsOff();

    if (DDJFLX2.fourDeckMode) {
    // Reassign physical decks to virtual decks 3 and 4.
        DDJFLX2.vDeckNo[1] = 3;
        DDJFLX2.vDeckNo[2] = 4;

        // Snapshot soft takeover targets for the newly active virtual decks.
        // Hardware positions are unknown at switch time, so we pass null and let
        // the first incoming MIDI value trigger the cross-check.
        DDJFLX2.snapshotSoftTakeover(1, null);
        DDJFLX2.snapshotSoftTakeover(2, null);

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

        midi.sendShortMsg(0x90, 0x54, 0x00);
        midi.sendShortMsg(0x91, 0x54, 0x00);
    } else {
    // Restore default mapping.
        DDJFLX2.vDeckNo[1] = 1;
        DDJFLX2.vDeckNo[2] = 2;

        // Snapshot soft takeover targets for the restored virtual decks.
        DDJFLX2.snapshotSoftTakeover(1, null);
        DDJFLX2.snapshotSoftTakeover(2, null);

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

DDJFLX2.syncShort = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (DDJFLX2.isShiftPressed(group)) {
        return;
    }
    if (value) {
        const deck = DDJFLX2.resolveDeck(group);
        const enabled = engine.getValue(deck.group, "sync_enabled");
        engine.setValue(deck.group, "sync_enabled", enabled ? 0 : 1);
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

DDJFLX2.handleSmartFaderVolume = function(deck, volume) {
    const deckState = DDJFLX2.vDeck[deck.virtualDeck];
    const isDown = volume <= 0;

    if (DDJFLX2.smartFaderEnabled) {
        if (deckState.volumeFaderDown && !isDown) {
            DDJFLX2.smartFaderStart(deck);
        } else if (!deckState.volumeFaderDown && isDown) {
            DDJFLX2.smartFaderStop(deck);
        }
    }

    deckState.volumeFaderDown = isDown;
};

DDJFLX2.syncLong = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (DDJFLX2.isShiftPressed(group)) {
        return;
    }

    if (value) {
        const deck = DDJFLX2.resolveDeck(group);
        const enabled = engine.getValue(deck.group, "sync_enabled");
        engine.setValue(deck.group, "sync_enabled", enabled ? 0 : 1);
    }
};

// Pad mode selection: Shift + Sync cycles through pad modes on the controller firmware.
// Only the press event is relevant; the release is ignored.
// Actual pad presses are handled by the mode-specific MIDI note ranges.
DDJFLX2.padModeSelect = function(
    channel,
    control,
    value,
    status
) {
    if (!value) {
        return;
    }
    midi.sendShortMsg(status, control, 0x7f);
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
    DDJFLX2.vDeck[deck.virtualDeck].rateMSB = value;
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

    const rateMSB = DDJFLX2.vDeck[deck.virtualDeck].rateMSB;
    // Hardware encodes rate as 0x0000 (top, +max) → 0x3FFE (bottom, -max).
    // 0x3FFE is 16382, the maximum 14-bit value sent by the FLX2 pitch fader.
    // Normalise to 0..1 for the soft takeover check, then convert to -1..1 for engine.
    const raw14  = (rateMSB << 7) + value;
    const hw01   = raw14 / 0x3ffe;   // 0 = pitch slider at top (+), 1 = bottom (-)
    const rate   = 1 - hw01 * 2;     // maps 0..1 → +1..-1

    if (!DDJFLX2.checkSoftTakeover(deck.physicalDeck, "rate", hw01)) {
        return;
    }
    engine.setValue(deck.group, "rate", rate);
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
    DDJFLX2.vDeck[deck.virtualDeck].volMSB = value;
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

    const volMSB = DDJFLX2.vDeck[deck.virtualDeck].volMSB;
    const vol = ((volMSB << 7) + value) / 0x3fff;

    // Apply volume unconditionally for Smart Fader position tracking,
    // but gate the engine write behind soft takeover.
    DDJFLX2.handleSmartFaderVolume(deck, vol);
    if (!DDJFLX2.checkSoftTakeover(deck.physicalDeck, "volume", vol)) {
        return;
    }
    engine.setValue(deck.group, "volume", vol);
};

// Store EQ MSB for 14-bit reconstruction.
// Deck is derived from the MIDI status byte (0xB0 = deck 1, 0xB1 = deck 2)
// because script.deckFromGroup() does not work with EQ rack group names.
DDJFLX2.eqMSB = function(channel, control, value, status, _group) {
    const physicalDeck = (status & 0x0F) + 1;
    const virtualDeck = DDJFLX2.vDeckNo[physicalDeck];
    if (!DDJFLX2.vDeck[virtualDeck].eqMSB) {
        DDJFLX2.vDeck[virtualDeck].eqMSB = {};
    }
    DDJFLX2.vDeck[virtualDeck].eqMSB[control] = value;
};

// Combine MSB and LSB to reconstruct full 14-bit EQ value.
DDJFLX2.eqLSB = function(channel, control, value, status, _group) {
    const physicalDeck = (status & 0x0F) + 1;
    const virtualDeck = DDJFLX2.vDeckNo[physicalDeck];
    const vgroup = `[Channel${  virtualDeck  }]`;
    const eqGroup = `[EqualizerRack1_${  vgroup  }_Effect1]`;

    const msb = (DDJFLX2.vDeck[virtualDeck].eqMSB || {})[control - 0x20] || 0;
    const combined = ((msb << 7) + value) / 0x3fff;
    const val = script.absoluteNonLin(combined * 127, 0, 1, 4);

    let eq;
    let controlKey;
    if (control === 0x27) { eq = 3; controlKey = "eqHigh"; } else if (control === 0x2B) { eq = 2; controlKey = "eqMid"; } else { eq = 1; controlKey = "eqLow"; }

    // combined is already normalised 0..1; use it for the soft takeover check.
    if (!DDJFLX2.checkSoftTakeover(physicalDeck, controlKey, combined)) {
        return;
    }
    engine.setValue(eqGroup, `parameter${  eq}`, val);
};

// Store CFX MSB for 14-bit reconstruction.
DDJFLX2.super1MSB = function(channel, control, value, _status, _group) {
    const physicalDeck = (control === 0x17 || control === 0x37) ? 1 : 2;
    const virtualDeck = DDJFLX2.vDeckNo[physicalDeck];
    if (!DDJFLX2.vDeck[virtualDeck].cfxMSB) {
        DDJFLX2.vDeck[virtualDeck].cfxMSB = 0;
    }
    DDJFLX2.vDeck[virtualDeck].cfxMSB = value;
};

// Combine MSB and LSB to reconstruct full 14-bit CFX value.
DDJFLX2.super1LSB = function(channel, control, value, _status, _group) {
    const physicalDeck = (control === 0x37 || control === 0x38) ?
        (control === 0x37 ? 1 : 2) :
        (control === 0x17 ? 1 : 2);
    const virtualDeck = DDJFLX2.vDeckNo[physicalDeck];
    const vgroup = `[Channel${  virtualDeck  }]`;
    const qfxGroup = `[QuickEffectRack1_${  vgroup  }]`;

    const msb = DDJFLX2.vDeck[virtualDeck].cfxMSB || 0;
    const combined = ((msb << 7) + value) / 0x3fff;
    const val = script.absoluteNonLin(combined * 127, 0, 0.5, 1);

    if (!DDJFLX2.checkSoftTakeover(physicalDeck, "cfx", combined)) {
        return;
    }
    engine.setValue(qfxGroup, "super1", val);
};

DDJFLX2.cueDefault = function(
    channel,
    control,
    value,
    status,
    group
) {
    if (value) {
        const deck = DDJFLX2.resolveDeck(group);

        // Match Pioneer CDJ cue behavior.
        if (engine.isScratching(deck.virtualDeck)) {
            engine.setValue(deck.group, "cue_set", true);
        } else {
            engine.setValue(deck.group, "cue_gotoandplay", true);
        }

        const cueSet = engine.getValue(deck.group, "cue_point") !== -1;

        midi.sendShortMsg(status, 0x0c, 0x7f * cueSet);

        midi.sendShortMsg(
            status,
            0x0b,
            0x7f * engine.getValue(deck.group, "play")
        );
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

        midi.sendShortMsg(
            status,
            0x0b,
            0x7f * engine.getValue(deck.group, "play")
        );
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
    const deck = DDJFLX2.resolveDeck(group);
    const hotcueNum = DDJFLX2.hotcuePadFromControl(control);
    const hotcue = `hotcue_${  hotcueNum}`;

    // Pass both press (1) and release (0) so Mixxx can manage the
    // preview-while-held behavior when the deck is paused.
    engine.setValue(deck.group, `${hotcue  }_activate`, value ? 1 : 0);

    if (value) {
        // Update play LED immediately for responsiveness.
        midi.sendShortMsg(
            0x90 + deck.physicalDeck - 1,
            0x0b,
            0x7f * engine.getValue(deck.group, "play")
        );
    }
    // NOTE: hotcue pad LED is handled by the makeConnection on hotcue_X_enabled.
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
        `hotcue_${  hotcueNum  }_clear`,
        true
    );

    // Turn off the pad LED using the correct MIDI-OUT channel.
    midi.sendShortMsg(
        DDJFLX2.getPadLedStatus(deck.physicalDeck, false),
        control,
        0x00
    );
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

    const fxGroup = `[EffectRack1_EffectUnit${  unit  }]`;
    const effectGroup = `[EffectRack1_EffectUnit${  unit  }_Effect${  slot  }]`;
    const stateKey = `${deck.group  }:${  unit  }:${  slot}`;

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
    // re-apply them to the effect unit, effectively "cleaning up" after the FX.
    if (!value) {
        const saved = DDJFLX2.padFxState[stateKey];
        if (saved) {
            engine.setValue(fxGroup, "enabled", saved.enabled);
            engine.setValue(fxGroup, "mix", saved.mix);
            engine.setValue(effectGroup, "enabled", saved.effectEnabled);
            engine.setValue(effectGroup, "meta", saved.meta);
            for (let i = 1; i <= 4; i++) {
                engine.setValue(fxGroup, `group_[Channel${  i  }]_enable`, saved.routing[i]);
            }
            delete DDJFLX2.padFxState[stateKey];
        }
        midi.sendShortMsg(ledStatus, control, 0x00);
        return;
    }

    // BUTTON PRESSED: Snapshot current state and apply FX.
    // Before we take over the effect unit, we save its current configuration
    // (which decks it's routed to, its mix level, etc.) so we can restore it later.
    if (!DDJFLX2.padFxState[stateKey]) {
        const routing = {};
        for (let i = 1; i <= 4; i++) {
            routing[i] = engine.getValue(fxGroup, `group_[Channel${  i  }]_enable`);
        }
        DDJFLX2.padFxState[stateKey] = {
            enabled: engine.getValue(fxGroup, "enabled"),
            mix: engine.getValue(fxGroup, "mix"),
            effectEnabled: engine.getValue(effectGroup, "enabled"),
            meta: engine.getValue(effectGroup, "meta"),
            routing: routing,
        };
    }

    // Force-route the effect unit to the current deck and maximize its presence.
    for (let i = 1; i <= 4; i++) {
        engine.setValue(fxGroup, `group_[Channel${  i  }]_enable`, 0);
    }
    engine.setValue(fxGroup, `group_${  deck.group  }_enable`, 1);
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
    const deck = DDJFLX2.resolveDeck(group);

    engine.setValue(
        deck.group,
        `beatlooproll_${  size  }_activate`,
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

// Update all 16 sampler pad LEDs.
// Physical deck 1 (left)  → samplers 1–8,  pad MIDI-OUT channel 0x97
// Physical deck 2 (right) → samplers 9–16, pad MIDI-OUT channel 0x99
// Three states per pad:
//   empty   → LED off
//   loaded  → LED on (steady)
//   playing → LED blinks with blinkPhase
DDJFLX2.updateAllSamplerLEDs = function() {
    // Beat jump owns the sampler pad LEDs when it replaces that mode.
    if (DDJFLX2.beatJumpPadMode === "sampler") {
        return;
    }

    for (let physicalDeck = 1; physicalDeck <= 2; physicalDeck++) {
        const ledStatus = DDJFLX2.getPadLedStatus(physicalDeck, false);
        for (let pad = 1; pad <= 8; pad++) {
            const samplerNo = pad + (physicalDeck - 1) * 8;
            const samplerGroup = `[Sampler${  samplerNo  }]`;
            const loaded = engine.getValue(samplerGroup, "track_loaded");
            const playing = engine.getValue(samplerGroup, "play");

            let ledValue;
            if (!loaded) {
                ledValue = 0x00;
            } else if (playing) {
                ledValue = DDJFLX2.blinkPhase ? 0x7f : 0x00;
            } else {
                ledValue = 0x7f;
            }

            // Sampler pads use control bytes 0x30–0x37 (pad 1 = 0x30).
            midi.sendShortMsg(ledStatus, 0x30 + (pad - 1), ledValue);
        }
    }
};

// Update loop pad LEDs for all visible virtual decks.
// Active loop pad blinks with blinkPhase; all others are off.
DDJFLX2.updateAllLoopBlinkLEDs = function() {
    for (let physicalDeck = 1; physicalDeck <= 2; physicalDeck++) {
        const vDeckNo = DDJFLX2.vDeckNo[physicalDeck];
        const activeSize = DDJFLX2.activeLoopSize[vDeckNo];

        for (let pad = 1; pad <= 8; pad++) {
            const size = DDJFLX2.loopPadSizes[pad - 1];
            const isActive = (activeSize === size);
            const ledValue = isActive ? (DDJFLX2.blinkPhase ? 0x7f : 0x00) : 0x00;
            // Loop pads use control bytes 0x60–0x67 (pad 1 = 0x60).
            midi.sendShortMsg(
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

    engine.setValue(group, "beatjump", size);
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
    const samplerGroup = `[Sampler${  samplerNo  }]`;

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
    const samplerGroup = `[Sampler${  samplerNo  }]`;

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

        midi.sendShortMsg(status, 0x54, 0x7f * pfl);
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
    const vgroup = `[Channel${  vDeckNo  }]`;

    DDJFLX2.switchPlayLED(
        d,
        engine.getValue(vgroup, "play")
    );

    midi.sendShortMsg(
        0x90 + d,
        0x0c,
        0x7f * (engine.getValue(vgroup, "cue_point") !== -1)
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
            `hotcue_${  i  }_enabled`
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

    const loopEnabled = engine.getValue(
        deck.group,
        "loop_enabled"
    );

    const currentSize =
    DDJFLX2.activeLoopSize[deck.virtualDeck];

    // Pressing same active loop disables it.
    if (loopEnabled && currentSize === size) {

        engine.setValue(
            deck.group,
            "reloop_toggle",
            1
        );

        DDJFLX2.activeLoopSize[deck.virtualDeck] = null;

    } else {

        DDJFLX2.activeLoopSize[deck.virtualDeck] = size;

        script.triggerControl(
            deck.group,
            `beatloop_${  size  }_activate`
        );
    }

    DDJFLX2.updateLoopPadLEDs(
        deck.virtualDeck
    );
};
