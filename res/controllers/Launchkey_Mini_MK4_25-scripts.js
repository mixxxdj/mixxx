/**
 * Novation Launchkey Mini MK4 - Controller Scripts for Mixxx
 *
 * Provides high-resolution 14-bit processing for touch strips,
 * low-latency scratching with inertia, smooth beatmatching,
 * library navigation, and multi-functional performance pads.
 */

var LaunchkeyJog = {};

// Active deck and scratch position memory
LaunchkeyJog.activeDeck = 1;
LaunchkeyJog.lastPosDeck1 = null;
LaunchkeyJog.lastTimeDeck1 = null;

// Temporary storage for assembling 14-bit Mod Strip values
LaunchkeyJog.modStrip14Bit = {
  msb: 0,
  lsb: 0,
};

// Physics parameters for slide gestures
LaunchkeyJog.inertia = {
  timerId: null,
  velocity: 0,
  friction: 0.95, // Decay rate for track momentum when finger is released
  minVelocity: 0.05, // Threshold below which inertia processing stops
  lastMidiTime: 0,
  largeSlideThreshold: 0.8, // Sensitivity threshold for triggering slide inertia
  isLargeSlide: false,
};

// Toggle states for Shift and Loops for each Deck (1 and 2)
LaunchkeyJog.deckState = {
  1: { shiftActive: false, inLoop: false },
  2: { shiftActive: false, inLoop: false },
};

// Input state tracking variables
LaunchkeyJog.lastDirection = 0;
LaunchkeyJog.watchdogTimerId = null;
LaunchkeyJog.touchStartTime = 0;
LaunchkeyJog.touchStartPos = 0;
LaunchkeyJog.lastMidiControl = null;
LaunchkeyJog.macroDirection = 0;

/**
 * Handles the Shift button state changes (D1: G#3, D2: A#3).
 * When Shift is active, the associated deck becomes the active target,
 * enabling secondary commands (e.g. track loading, hotcue clearing).
 */
LaunchkeyJog.handleShift = function (channel, control, value, status, group) {
  var deck = control === 0x44 ? 1 : 2;
  var state = LaunchkeyJog.deckState[deck];
  state.shiftActive = value > 0;

  if (state.shiftActive) {
    LaunchkeyJog.activeDeck = deck;
    LaunchkeyJog.lastPosDeck1 = null;
    print(">>> Shift Deck " + deck + " attivo: Pitch Strip riassegnata <<<");
  }
};

/**
 * Handles all pad actions (Cue, Play, Hotcues, and Loop sizes).
 * Determines the target deck based on MIDI control code and triggers
 * standard or Shift-modified actions on press and release.
 */
LaunchkeyJog.padHandler = function (channel, control, value, status, group) {
  var isPress = value > 0;
  var deck, groupTarget;
  var d1Pads = [0x3c, 0x3d, 0x3e, 0x40, 0x3f, 0x42];

  if (d1Pads.indexOf(control) !== -1) {
    deck = 1;
    groupTarget = "[Channel1]";
  } else {
    deck = 2;
    groupTarget = "[Channel2]";
  }

  var state = LaunchkeyJog.deckState[deck];

  switch (control) {
    // Cue buttons
    case 0x3c:
    case 0x48:
      if (!isPress) return;
      engine.setValue(groupTarget, "cue_default", 1);
      break;

    // Play/Pause buttons
    case 0x3d:
    case 0x4e:
      if (!isPress) return;
      var currentPlay = engine.getValue(groupTarget, "play");

      // Shift + Play triggers a 1-bar quantized pause
      if (state.shiftActive && currentPlay === 1) {
        var bpm = engine.getValue(groupTarget, "file_bpm");
        if (bpm <= 0) bpm = engine.getValue(groupTarget, "bpm");
        if (bpm <= 0) bpm = 120;

        var oneBarMs = (60000 / bpm) * 4;
        print(
          ">>> [SHIFT+PLAY] Pausa quantizzata: stop tra " +
            Math.round(oneBarMs) +
            "ms <<<",
        );

        engine.beginTimer(
          oneBarMs,
          function () {
            engine.setValue(groupTarget, "play", 0);
          },
          true,
        );
      } else {
        // Standard toggle behavior
        engine.setValue(groupTarget, "play", !currentPlay);
      }
      break;

    // Hotcue 1
    case 0x3e:
    case 0x45:
      var action1 = state.shiftActive ? "clear" : "activate";
      engine.setValue(groupTarget, "hotcue_1_" + action1, isPress ? 1 : 0);
      break;

    // Hotcue 2
    case 0x40:
    case 0x47:
      var action2 = state.shiftActive ? "clear" : "activate";
      engine.setValue(groupTarget, "hotcue_2_" + action2, isPress ? 1 : 0);
      break;

    // Loop Control Left
    case 0x3f:
    case 0x49:
      if (!isPress) return;
      if (state.shiftActive) {
        engine.setValue(groupTarget, "loop_exit", 1);
        state.inLoop = false;
      } else {
        var cmdLeft = state.inLoop
          ? deck === 1
            ? "loop_double"
            : "loop_halve"
          : "beatloop_activate";
        if (!state.inLoop) engine.setValue(groupTarget, "beatloop_size", 1);
        engine.setValue(groupTarget, cmdLeft, 1);
        state.inLoop = true;
      }
      break;

    // Loop Control Right
    case 0x42:
    case 0x4b:
      if (!isPress) return;
      if (state.shiftActive) {
        engine.setValue(groupTarget, "loop_exit", 1);
        state.inLoop = false;
      } else {
        var cmdRight = state.inLoop
          ? deck === 1
            ? "loop_halve"
            : "loop_double"
          : "beatloop_activate";
        if (!state.inLoop) engine.setValue(groupTarget, "beatloop_size", 1);
        engine.setValue(groupTarget, cmdRight, 1);
        state.inLoop = true;
      }
      break;
  }
};

/**
 * Toggles the actively controlled deck between Deck 1 and Deck 2.
 */
LaunchkeyJog.toggleDeck = function (channel, control, value, status, group) {
  if (value === 0) return;
  LaunchkeyJog.activeDeck = LaunchkeyJog.activeDeck === 1 ? 2 : 1;
  LaunchkeyJog.lastPosDeck1 = null;
};

// ====================================================
// BEAT MATCHING / ENCODER NUDGING (14-BIT)
// ====================================================

LaunchkeyJog.BEATMATCH_CONST = {
  // EMA Filter: 1.0 = raw/instantaneous, 0.5 = balanced, 0.2 = smooth but delayed.
  // For beatmatching (nudging/bending), we want a high value (0.8) to get immediate
  // physical response and speed adjustments when rotating the encoder.
  EMA_ALPHA: 0.8,

  // Speed scaling of the nudge. Converts the filtered 14-bit delta into relative
  // speed adjustments for Mixxx's jog parameter. We can lower this value if the track
  // nudges too aggressively, or raise it if the control feels unresponsive.
  SENSITIVITY: 0.05,

  // Deadzone to ignore hardware noise when the encoder is resting or stationary.
  // Filters out small accidental fluctuations (±1 unit 14-bit) from the sensor.
  DEADZONE: 1,
};

LaunchkeyJog.beatmatchState = {
  1: { msb: 64, lsb: 0, lastPos: null, emaVelocity: 0 },
  2: { msb: 64, lsb: 0, lastPos: null, emaVelocity: 0 },
};

/**
 * MSB Receiver for Deck 1 Beatmatching
 */
LaunchkeyJog.handleBeatmatchD1_MSB = function (
  channel,
  control,
  value,
  status,
  group,
) {
  LaunchkeyJog.beatmatchState[1].msb = value;
};

/**
 * LSB Receiver for Deck 1 Beatmatching (triggers execution)
 */
LaunchkeyJog.handleBeatmatchD1_LSB = function (
  channel,
  control,
  value,
  status,
  group,
) {
  LaunchkeyJog.beatmatchState[1].lsb = value;
  LaunchkeyJog.processBeatmatch(1);
};

/**
 * MSB Receiver for Deck 2 Beatmatching
 */
LaunchkeyJog.handleBeatmatchD2_MSB = function (
  channel,
  control,
  value,
  status,
  group,
) {
  LaunchkeyJog.beatmatchState[2].msb = value;
};

/**
 * LSB Receiver for Deck 2 Beatmatching (triggers execution)
 */
LaunchkeyJog.handleBeatmatchD2_LSB = function (
  channel,
  control,
  value,
  status,
  group,
) {
  LaunchkeyJog.beatmatchState[2].lsb = value;
  LaunchkeyJog.processBeatmatch(2);
};

/**
 * Assembles and processes the 14-bit value to control the jog nudge parameter.
 * Uses wrap-around detection and an EMA filter for smooth movements.
 */
LaunchkeyJog.processBeatmatch = function (deck) {
  var state = LaunchkeyJog.beatmatchState[deck];
  var C = LaunchkeyJog.BEATMATCH_CONST;

  var rawPos = (state.msb << 7) | state.lsb;

  if (state.lastPos === null) {
    state.lastPos = rawPos;
    return;
  }

  var rawDelta = rawPos - state.lastPos;

  // Handle wrap-around for infinite encoders
  if (rawDelta > 8192) rawDelta -= 16384;
  else if (rawDelta < -8192) rawDelta += 16384;

  state.lastPos = rawPos;

  if (Math.abs(rawDelta) <= C.DEADZONE) {
    state.emaVelocity = 0;
    return;
  }

  // Smooth the velocity using Exponential Moving Average
  state.emaVelocity =
    C.EMA_ALPHA * rawDelta + (1.0 - C.EMA_ALPHA) * state.emaVelocity;

  engine.setValue(
    "[Channel" + deck + "]",
    "jog",
    state.emaVelocity * C.SENSITIVITY,
  );
};

// ====================================================
// LIBRARY SCROLLING (14-BIT)
// ====================================================

LaunchkeyJog.LIBRARY_CONST = {
  // Number of 14-bit units required to trigger one discrete scroll tick (MoveUp/MoveDown).
  // Lower values make track scrolling faster; higher values make it slower and more precise.
  // At a value of 10, a full fader sweep scrolls through approx. 1638 tracks.
  SCROLL_SENSITIVITY: 3,
};

LaunchkeyJog.lib = { msb: 64, lsb: 0, lastVal: null, accumulator: 0 };

/**
 * MSB Receiver for Library scrolling
 */
LaunchkeyJog.handleLibrary_MSB = function (
  channel,
  control,
  value,
  status,
  group,
) {
  LaunchkeyJog.lib.msb = value;
};

/**
 * LSB Receiver for Library scrolling (triggers scroll calculation)
 */
LaunchkeyJog.handleLibrary_LSB = function (
  channel,
  control,
  value,
  status,
  group,
) {
  LaunchkeyJog.lib.lsb = value;
  LaunchkeyJog.processLibraryMovement(
    LaunchkeyJog.lib,
    "MoveDown",
    "MoveUp",
    LaunchkeyJog.LIBRARY_CONST.SCROLL_SENSITIVITY,
  );
};

/**
 * Translates continuous 14-bit absolute changes into discrete list selection moves.
 * Modified to force a strict physical step radius by wiping the accumulator remainder.
 */
LaunchkeyJog.processLibraryMovement = function (
  state,
  cmdForward,
  cmdBackward,
  sensitivity,
) {
  var currentVal = (state.msb << 7) | state.lsb;

  if (state.lastVal === null) {
    state.lastVal = currentVal;
    return;
  }

  var delta = currentVal - state.lastVal;

  // Update lastVal immediately to keep tracking clean
  state.lastVal = currentVal;

  state.accumulator += delta;

  // Trigger UI movement ticks and completely RESET the accumulator remainder.
  // This guarantees you always need a fresh, full physical distance for the next song.
  if (state.accumulator >= sensitivity) {
    engine.setValue("[Library]", cmdForward, 1);
    engine.setValue("[Library]", cmdForward, 0);
    state.accumulator = 0; // Clear the remainder
  } else if (state.accumulator <= -sensitivity) {
    engine.setValue("[Library]", cmdBackward, 1);
    engine.setValue("[Library]", cmdBackward, 0);
    state.accumulator = 0; // Clear the remainder
  }
};

/**
 * Performs track loading or sync toggle actions.
 * If Shift is active: Loads the selected track to the deck.
 * If Shift is inactive: Toggles sync lock on the deck.
 */
LaunchkeyJog.handleLoadTrack = function (
  channel,
  control,
  value,
  status,
  group,
) {
  if (value === 0) return;

  var deck = control === 0x41 ? 1 : 2;
  var state = LaunchkeyJog.deckState[deck];
  var groupTarget = "[Channel" + deck + "]";

  if (state.shiftActive) {
    engine.setValue(groupTarget, "LoadSelectedTrack", 1);
    print(">>> Caricamento traccia su Deck " + deck + " <<<");
  } else {
    engine.setValue(
      groupTarget,
      "sync_enabled",
      !engine.getValue(groupTarget, "sync_enabled"),
    );
  }
};

// ====================================================
// MOD STRIP SCRATCH ENGINE (ZERO LATENCY)
// ====================================================

LaunchkeyJog.MODSTRIP_CONST = {
  // EMA: filters ADC noise (~2-3 LSB) without adding noticeable lag.
  // EMA Formula: y[n] = α·x[n] + (1-α)·y[n-1]
  // High α (0.5) = responsive but noisy, low α (0.2) = smooth but slow.
  // 0.35 is the ideal compromise for a 14-bit capacitive strip.
  EMA_ALPHA: 0.35,

  // Converts the filtered delta (14-bit units) into scratch ticks for Mixxx.
  // The strip has ~100mm of travel over 16384 steps → 1mm ≈ 164 steps.
  // 0.08 tick/step × 164 steps/mm ≈ 13 ticks/mm → similar to a 100mm jogwheel.
  SCRATCH_SENSITIVITY: 0.08,

  // Deadzone: we ignore delta ≤ 1 14-bit unit.
  // Capacitive noise from a stationary finger generates oscillations of ±1-2 LSB.
  // Without a deadzone, the track/disc "vibrates" imperceptibly but audibly.
  DEADZONE: 1,

  // Watchdog interval (ms). A low-frequency timer checks
  // if new MIDI packets have arrived since the last check.
  // 50ms is a good compromise: fast enough to detect release,
  // slow enough for us to tolerate 1 lost packet without false release.
  WATCHDOG_INTERVAL_MS: 50,

  // Minimum EMA velocity (14-bit units/frame) to trigger inertia.
  // A fast swipe generates delta of ~30-80 units/frame on the strip.
  // Threshold at 5.0 covers only intentional swipes, where we ignore slow releases.
  INERTIA_THRESHOLD: 5.0,

  // Exponential decay of inertia per frame (every 16ms).
  // v[n+1] = v[n] × FRICTION
  // With 0.92: halving in ln(0.5)/ln(0.92) ≈ 8.3 frames ≈ 133ms.
  // This gives a "light vinyl" feel — enough to continue
  // the momentum but not so long as to feel like a sled on ice.
  INERTIA_FRICTION: 0.95,

  // Inertia stop threshold (scratch ticks).
  // Below 1.5 ticks the sound is no longer audible, so we stop the loop.
  INERTIA_MIN_VELOCITY: 1.5,
};

LaunchkeyJog.modStrip = {
  msb: 64,
  lsb: 0,
  lastRawPos: null,
  emaVelocity: 0,
  isScratching: false,
  midiPacketCount: 0,
  watchdogTimerId: null,
  inertiaTimerId: null,
  inertiaVelocity: 0,
};

/**
 * MSB Receiver for Mod Strip scratch control
 */
LaunchkeyJog.handleModStrip_MSB = function (
  channel,
  control,
  value,
  status,
  group,
) {
  LaunchkeyJog.modStrip.msb = value;
};

/**
 * LSB Receiver for Mod Strip scratch control (triggers calculations)
 */
LaunchkeyJog.handleModStrip_LSB = function (
  channel,
  control,
  value,
  status,
  group,
) {
  LaunchkeyJog.modStrip.lsb = value;
  LaunchkeyJog.updateModStripTarget();
};

/**
 * We process Mod Strip inputs to control scratching.
 * We activate scratch engine on initial touch, filter jitter using an EMA,
 * and output tick commands directly to Mixxx.
 */
LaunchkeyJog.updateModStripTarget = function () {
  var m = LaunchkeyJog.modStrip;
  var C = LaunchkeyJog.MODSTRIP_CONST;
  var deck = LaunchkeyJog.activeDeck;

  var rawPos = (m.msb << 7) | m.lsb;
  m.midiPacketCount++;

  // We stop any active inertia movement when we detect a new touch
  if (m.inertiaTimerId !== null) {
    engine.stopTimer(m.inertiaTimerId);
    m.inertiaVelocity = 0;
  }

  // We handle the first contact touch initialization
  if (!m.isScratching) {
    m.lastRawPos = rawPos;
    m.emaVelocity = 0;
    m.isScratching = true;

    // We enable Mixxx's scratching mode
    engine.scratchEnable(deck, 128, 33 + 1 / 3, 0.25, 1.0 / 256);

    // We initialize the watchdog timer to detect release
    if (m.watchdogTimerId === null) {
      m.watchdogTimerId = engine.beginTimer(
        C.WATCHDOG_INTERVAL_MS,
        function () {
          LaunchkeyJog.modStripWatchdog();
        },
        false,
      );
    }
    return;
  }

  var rawDelta = rawPos - m.lastRawPos;
  m.lastRawPos = rawPos;

  if (Math.abs(rawDelta) <= C.DEADZONE) {
    return;
  }

  // We apply smoothing and send speed ticks
  m.emaVelocity = C.EMA_ALPHA * rawDelta + (1.0 - C.EMA_ALPHA) * m.emaVelocity;
  var scratchTicks = m.emaVelocity * C.SCRATCH_SENSITIVITY;
  engine.scratchTick(deck, scratchTicks);
};

/**
 * We run a watchdog checking function periodically.
 * If we receive no MIDI packets during the interval, we register a finger lift,
 * concluding standard scratching and initiating inertia.
 */
LaunchkeyJog.modStripWatchdog = function () {
  var m = LaunchkeyJog.modStrip;
  var C = LaunchkeyJog.MODSTRIP_CONST;
  var deck = LaunchkeyJog.activeDeck;

  if (!m.isScratching) {
    if (m.watchdogTimerId !== null) {
      engine.stopTimer(m.watchdogTimerId);
      m.watchdogTimerId = null;
    }
    return;
  }

  if (m.midiPacketCount > 0) {
    m.midiPacketCount = 0;
    return;
  }

  // Finger lift event
  engine.stopTimer(m.watchdogTimerId);
  m.watchdogTimerId = null;
  m.isScratching = false;

  var releaseVelocity = m.emaVelocity;
  var releaseTickVelocity = releaseVelocity * C.SCRATCH_SENSITIVITY;

  // We trigger the inertia slide if released quickly, otherwise we stop instantly
  if (Math.abs(releaseVelocity) > C.INERTIA_THRESHOLD) {
    m.inertiaVelocity = releaseTickVelocity;
    m.inertiaTimerId = engine.beginTimer(
      16, // ~60Hz ticks
      function () {
        LaunchkeyJog.applyModStripInertia();
      },
      false,
    );
  } else {
    engine.scratchTick(deck, 0);
    engine.scratchDisable(deck, false);
  }

  m.emaVelocity = 0;
  m.lastRawPos = null;
};

/**
 * We perform inertia simulations at ~60Hz following a fast release.
 * We exponentially decrease speed until the motion drops below the minimum threshold.
 */
LaunchkeyJog.applyModStripInertia = function () {
  var m = LaunchkeyJog.modStrip;
  var C = LaunchkeyJog.MODSTRIP_CONST;
  var deck = LaunchkeyJog.activeDeck;

  m.inertiaVelocity *= C.INERTIA_FRICTION;

  if (Math.abs(m.inertiaVelocity) <= C.INERTIA_MIN_VELOCITY) {
    engine.stopTimer(m.inertiaTimerId);
    m.inertiaTimerId = null;
    m.inertiaVelocity = 0;
    engine.scratchTick(deck, 0);
    engine.scratchDisable(deck, false);
    return;
  }

  if (engine.isScratching(deck)) {
    engine.scratchTick(deck, m.inertiaVelocity);
  } else {
    engine.stopTimer(m.inertiaTimerId);
    m.inertiaTimerId = null;
    m.inertiaVelocity = 0;
  }
};

// ====================================================
// CROSSFADER PITCH STRIP (STICKY + SOFT CATCH-UP)
// ====================================================

LaunchkeyJog.CROSSFADER_CONST = {
  // Proximity threshold on a scale of -1.0 to 1.0. 0.15 represents about 7.5% of fader travel.
  // When physical touch starts, the input value is locked until the finger position
  // gets within this distance of Mixxx's software crossfader position (Soft Takeover / Catch-up).
  // Prevents sudden volume/balance spikes when touching the capacitive crossfader strip.
  CATCHUP_THRESHOLD: 0.15,

  // Exponential Moving Average coefficient to smooth electrical jitter.
  // A capacitive strip is highly prone to noise under a resting finger.
  // 0.4 provides excellent jitter smoothing without introducing noticeable control lag.
  EMA_ALPHA: 0.4,
};

LaunchkeyJog.pitchBendTmp = {
  msb: 64,
  lsb: 0,
};

LaunchkeyJog.crossfaderState = {
  lastVirtualPos: 0.0,
  isLocked: true, // Soft-takeover lock state
  isFirstTouch: true,
};

/**
 * Processes Pitch Bend input as a sticky relative crossfader.
 * Interprets physical inputs, implements a soft-takeover/catch-up lock,
 * and outputs smoothed coordinates to Mixxx's master crossfader.
 */
LaunchkeyJog.handleCrossfaderStrip = function (
  channel,
  control,
  value,
  status,
  group,
) {
  var state = LaunchkeyJog.crossfaderState;
  var C = LaunchkeyJog.CROSSFADER_CONST;

  var lsb = control;
  var msb = value;
  var raw14Bit = (msb << 7) | lsb;

  // When we release the finger, the Launchkey strictly sends E0 00 40 (= 8192).
  // We allow a micro-tolerance (8190-8194) to tolerate electrical noise upon release.
  if (raw14Bit >= 8190 && raw14Bit <= 8194) {
    state.isLocked = true;
    state.isFirstTouch = true;
    return;
  }

  // Normalize range 0-16383 to -1.0 to 1.0
  var normalized = (raw14Bit - 8192) / 8192.0;

  if (normalized < -1.0) normalized = -1.0;
  if (normalized > 1.0) normalized = 1.0;

  var currentCrossfader = engine.getValue("[Master]", "crossfader");

  if (state.isFirstTouch) {
    state.lastVirtualPos = currentCrossfader;
    state.isFirstTouch = false;
    state.isLocked = true;
  }

  // Soft-takeover logic: we only modify the crossfader once the input value crosses the current position
  if (state.isLocked) {
    var distance = Math.abs(normalized - currentCrossfader);
    if (distance <= C.CATCHUP_THRESHOLD) {
      state.isLocked = false;
      state.lastVirtualPos = currentCrossfader;
    } else {
      // Until we catch up, physical movement is ignored to avoid spikes
      return;
    }
  }

  var smoothed =
    C.EMA_ALPHA * normalized + (1.0 - C.EMA_ALPHA) * state.lastVirtualPos;

  if (smoothed < -1.0) smoothed = -1.0;
  if (smoothed > 1.0) smoothed = 1.0;

  engine.setValue("[Master]", "crossfader", smoothed);
  state.lastVirtualPos = smoothed;
};
