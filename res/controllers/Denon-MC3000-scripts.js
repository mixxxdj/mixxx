// ****************************************************************************
// Denon MC3000 - Mixxx controller mapping
// ----------------------------------------------------------------------------
// Target : Mixxx 2.5.x  (tested against the 2.5 control reference; also run
//          on Mixxx 2.3.3 via the compat layer - see MC3000.detectCompat)
// Version: 3.5.0
//
// KEY ARCHITECTURE (v3.2, confirmed against real hardware): the DECK
// SECTION's MIDI channel (play, cue, sync, loops, jog, pitch, EQ, hotcues...)
// DOES reliably move to the selected deck's own channel when you change deck
// (owner's manual W0: "the control channel switches in synchronization" -
// true after all). MC3000.deckGroup(channel) resolves the target deck
// straight from the incoming channel via the deckFromChannel table for all
// of these; there is no focus state to keep in sync for them.
//
// LOAD is the one exception: its own MIDI channel does NOT move with the
// deck (confirmed on real hardware, see MC3000.load and change #10 below), so
// it still needs MC3000.deckOnSide[] - updated by every DECK CHG A/B/C/D
// press (see deckButton() -> applyDeck() -> selectDeck()) - to know which
// deck the side it's on is currently showing. The SHIFT+EFX kill routing,
// FX-follows-deck, and the effect-enabled LED callbacks need it for the same
// reason: they react to a button press or an engine state change, not an
// incoming per-deck message, so they have no channel of their own to read.
// To change which Mixxx deck a physical deck controls, edit the single
// deckFor* table (search "DECK ASSIGNMENT" below).
//
// A v3.1 revision of this file went the other way - tracking focus in
// software for everything, on the belief that the channel could NOT be
// trusted - based on a claim that was never actually tested and turned out
// to be false (see change #8 below). That is why this file's history briefly
// argues both sides: trust the comment with the newest version tag.
//
// Rewritten from the original 1.10/1.11-era mapping by Bertrand Espern (2012).
//
// WHAT WAS BROKEN IN THE OLD MAPPING AND IS FIXED HERE
// ----------------------------------------------------------------------------
//  1. SHIFT was tracked *per Mixxx deck* ("shift1", "shift2") but the MC3000
//     SHIFT button is a *physical, per-side* button. As soon as the left deck
//     was switched to DECK C the button started reporting on MIDI channel 2 and
//     no shift state was ever set -> every secondary function died.
//     SHIFT is now tracked per side (left/right) and the side is derived from
//     the note number (96 = left, 97 = right), so it can never desync.
//  2. Only MIDI channels 1 and 3 (decks A and B) were mapped. Everything on
//     decks C and D (channels 2 and 4) was silently dropped.
//     Every deck control is now mapped on all four channels.
//  3. The pitch fader used <midino> with a 0xEn status. Per the Mixxx mapping
//     spec, 0xEn (pitch bend) messages must be mapped WITHOUT a <midino>
//     element, otherwise the mapping only matches when the LSB happens to equal
//     that number -> erratic / dead pitch fader.
//  4. Mixxx 1.x controls that no longer exist were still used:
//     filterHigh/Mid/Low, filterLowKill, [Flanger] lfoDepth/lfoDelay/lfoPeriod,
//     "flanger", [Playlist] SelectNextTrack, engine.connectControl, and raw
//     sample-position loop maths. All replaced with current 2.5 controls.
//  5. efx_beatLoopX computed its beat count as (control - 18) - (deck-1)*64,
//     which only produced correct results for decks 1 and 2.
//  6. (v3.1) Jog wheel / scratch, VINYL MODE and the pitch-takeover bookkeeping
//     resolved their target deck differently from every other control
//     (a fixed channel->deck table instead of deckGroup()'s tracked focus).
//     Fixed by deriving deck from deckGroup()'s own result everywhere
//     (deckFromGroup(deckGroup(channel))) instead of a second, independent
//     lookup - so these can never disagree with deckGroup() again, whatever
//     deckGroup() itself does internally. That is why change #8 below (which
//     rewrote what deckGroup() does internally) did not need to touch these
//     functions at all.
//  7. (v3.1) deckButton() - the handler actually bound to every DECK CHG /
//     DECK A/C / DECK B/D press - reimplemented deck switching inline instead
//     of calling the existing MC3000.selectDeck()/applyDeck(), so it skipped
//     two things those functions already did correctly: clearing the outgoing
//     deck's LEDs (stale play/cue/loop/hotcue lamps stayed lit on the deck you
//     left) and moving the effect units to follow the deck (fxFollowsDeck was
//     silently inert). deckButton() now just calls applyDeck().
//  8. (v3.2) deckGroup() resolved the deck from a software-tracked focus
//     (deckOnSide), on the belief - stated as fact in the v3.1 header but
//     never actually verified - that the hardware does not reliably move the
//     deck section's MIDI channel with the layer. Tested directly against the
//     real MC3000 on 2026-08-05 (toggling DECK CHG A/C while watching which
//     channel actually lit the PLAY lamp): the channel moves exactly as the
//     owner's manual originally claimed. The tracked-focus design was solving
//     a problem that does not exist on this hardware, at the cost of a real
//     one - deckOnSide silently drifting from what the panel is actually
//     showing (e.g. at Mixxx startup, if the unit was left on a non-default
//     layer). deckGroup() now reads the channel directly again. The same
//     session also found the DECK CHG A/B/C/D and DECK A/C / DECK B/D lamps
//     light themselves from the button press alone, with no MIDI involved -
//     echoDeckChgLeds is now false by default because of this.
//  9. (v3.2) Notes 14/15 (DECK A/C, DECK B/D) were handled as "invert the
//     current deck for this side", per the owner's manual's description of
//     them as recall/toggle buttons. A structured hardware test on 2026-08-05
//     (trace logging every deckButton dispatch, deployed as
//     MC3000.traceDeckButtons = true) showed this is not what note 14 means
//     on this unit: pressing DECK CHG C makes the firmware automatically fire
//     a full note-14 press+release about 1ms after releasing DECK CHG C -
//     not a second user action - and even a genuine, deliberate standalone
//     press of note 14 did not move the channel subsequent PLAY messages
//     arrived on. The spurious companion case was actively harmful: it made
//     deckButton() invert deckOnSide right back to the deck you had just left,
//     and the resulting selectDeck() call then blanked the LEDs of the deck
//     that was still genuinely showing and playing - "PLAY lamp off but the
//     song is still playing". deckButton() now ignores notes 14/15 entirely.
//     CONFIRMED from the official MC3000 MIDI command list and the Denon
//     Traktor/VirtualDJ mapping guides: note 14/15 are "CHANNEL SELECT A/C" /
//     "CHANNEL SELECT B/D" - a MIXER control ("selects deck A or C as input
//     source for [the] channel strip"), unrelated to the DECK CHG A/B/C/D
//     deck-section focus buttons, and absent from the reception/LED command
//     table entirely (consistent with its lamp being purely hardware-
//     autonomous, as observed). On Traktor/VirtualDJ this disambiguates which
//     deck a shared-CC mixer knob currently controls; this mapping has no
//     such ambiguity to begin with - every deck already has its own dedicated
//     CC for LEVEL/EQ/fader (see the "MIXER - channel strips" block in the
//     XML) - so there is nothing for this control to do here. Deliberately
//     left unmapped, not a gap.
//  10. (v3.2) MC3000.load() originally read MC3000.deckOnSide to find its
//     target deck, on the belief that LOAD's own channel was fixed and did
//     not follow the deck. Change #9 fixed a real bug that had been
//     corrupting deckOnSide (the spurious note-14 companion event), which
//     made it tempting to conclude LOAD's old fixed-channel assumption was
//     also wrong and switch it to deckGroup(channel) like everything else -
//     it had a stale comment already claiming that was the right behavior.
//     That was a mistake: tested on real hardware on 2026-08-05, a LOAD A/C
//     press still arrived on channel 0 even with the left side switched to
//     deck C. LOAD's channel genuinely is fixed, unlike the rest of the deck
//     section - it needed deckOnSide all along, and change #9 alone was
//     already enough to fix it. Reverted back to deckOnSide.
//  11. (v3.3, reverted) On request: DECK CHG A/B/C/D changing a deck briefly
//     also disabled every active effect slot in both FX1 and FX2
//     (MC3000.disableAllFx(), called from selectDeck()). Also requested at
//     startup (see the old change #13 here). The startup case didn't behave
//     as expected in testing and both were pulled at the user's request
//     before the cause was tracked down. If this is wanted again, re-add
//     disableAllFx() (loop u=1..2, m=1..effectSlots[u-1], set "enabled" 0 on
//     "[EffectRack1_EffectUnitU_EffectM]") and call it from selectDeck() for
//     the deck-change case and/or init() (after loadStartupEffects()) for the
//     startup case.
//  12. (v3.3, reverted) On request: CHANNEL SELECT A/C, B/D (notes 14/15,
//     otherwise unmapped since change #9) briefly loaded the selected library
//     track into whichever deck that side is showing - the same action as
//     LOAD A/C, LOAD B/D - guarded against the note-9/14 companion event from
//     change #9 with a debounce. Removed at the user's request; notes 14/15
//     are back to being ignored entirely, same as change #9 left them.
//     MC3000.loadOnSide(side), factored out of MC3000.load() to share this
//     logic, is harmless left in place and still used by LOAD A/C, LOAD B/D.
//  13. (v3.3, reverted) The startup half of change #11 above - see there.
//  14. (v3.4) On request, revisiting change #12: CHANNEL SELECT A/C, B/D
//     (notes 14/15) again load the selected library track into whichever
//     deck their side is showing (MC3000.loadOnSide()), but guarded
//     differently this time. Change #12 debounced by watching DECK CHG
//     presses (MC3000.lastDeckChgPress); this instead measures how long
//     note 14/15 itself was held (MC3000.channelSelectMinHoldMs), since the
//     manual (p.22, item 21) confirms DECK CHG deliberately pulses this
//     button's own signal in sync - a real press+release, just far too short
//     (~1ms) to be an actual finger on the button. Needed adding note-off
//     (0x80/0x81/0x82/0x83) bindings for notes 14/15 to the XML, which had
//     previously only ever bound their note-on.
//  15. (v3.5) Change #14 turned out to be a misread of what was wanted: a
//     genuine press of CHANNEL SELECT A/C, B/D now flips MC3000.deckOnSide
//     for that side between its two decks (matching the button's own lamp)
//     instead of loading directly. LOAD A/C, LOAD B/D already read
//     deckOnSide and are unchanged, so "press CHANNEL SELECT, then press
//     LOAD" now works the way DECK CHG + LOAD always has - recall a deck,
//     then load into it - which is the intended replacement for the
//     original (always broken, see change #9) "DECK A/C recall" reading of
//     this button. Still guarded by the same MC3000.channelSelectMinHoldMs
//     hold-duration filter from change #14, for the same reason: without it,
//     the DECK CHG sync pulse would flip deckOnSide right back on every
//     DECK CHG press.
//
// LAYOUT
// ----------------------------------------------------------------------------
// The MC3000 changes MIDI channel with the DECK CHG buttons:
//     channel 1 (0x_0) = DECK A -> Mixxx [Channel1]   (LEFT side)
//     channel 2 (0x_1) = DECK C -> Mixxx [Channel3]   (LEFT side)
//     channel 3 (0x_2) = DECK B -> Mixxx [Channel2]   (RIGHT side)
//     channel 4 (0x_3) = DECK D -> Mixxx [Channel4]   (RIGHT side)
// so the MIDI channel alone tells us both the deck and the physical side -
// see deckGroup() and change #8 above.
//
// See the "CONTROL REFERENCE" comment at the bottom of this file for the full
// button-by-button description, including every SHIFT function.
// ****************************************************************************

var MC3000 = {};

// ============================================================================
//  USER SETTINGS - tweak these, they are the only things you should need to
//  change. Save the file and re-select the mapping in Mixxx to reload.
// ============================================================================

// --- Jog wheel -------------------------------------------------------------
// Number of MIDI ticks the wheel emits for one full revolution. The MC3000
// encoder is specified at 2048 pulses/rev. If scratching feels too SLOW
// (the record barely moves), LOWER this number. If it feels too FAST /
// twitchy, RAISE it. Typical useful range: 600 - 2048.
MC3000.jogTicksPerRevolution = 2048;

// Virtual turntable speed used for scratching.
MC3000.jogScratchRpm = 33 + 1 / 3;

// Pitch-bend strength when the wheel is turned without touching the top
// (or when VINYL MODE is off). Higher = stronger bend.
MC3000.jogBendSensitivity = 0.6;

// How fast SHIFT + jog scrubs through the track (fraction of the track per tick).
MC3000.jogSeekSensitivity = 1 / 800;

// Scratch filter constants. Leave alone unless scratching oscillates.
MC3000.scratchAlpha = 1 / 8;
MC3000.scratchBeta = (1 / 8) / 32;

// VINYL MODE state at start-up (true = scratch, false = pitch bend).
MC3000.vinylModeAtStartup = true;

// KEY LOCK state at start-up, applied to all four decks (true = on). Mixxx
// otherwise leaves each deck's key lock at whatever it was last session,
// independently per deck. null = leave them alone (whatever Mixxx already
// has for each deck).
MC3000.keylockAtStartup = true;

// --- Pitch fader -----------------------------------------------------------
// Set to true if moving the fader towards "+" slows the track down.
MC3000.invertPitchFader = false;

// Soft takeover on the pitch fader. When ON, the fader is ignored after a deck
// layer switch until it reaches the new deck's current tempo - that stops the
// tempo jumping, but it also makes the fader feel "dead" until you sweep it,
// which is confusing. OFF by default: the fader always takes effect at once.
MC3000.pitchSoftTakeover = false;
MC3000.pitchTakeoverTolerance = 0.04;   // ~ +/-4% of the full fader travel

// --- Mixer -----------------------------------------------------------------
// Soft takeover for gain / EQ / filter knobs. Same trade-off as above: it
// prevents jumps after a layer switch, at the cost of the knobs feeling dead
// until you sweep them. OFF by default.
MC3000.softTakeoverEqAndGain = false;
// Soft takeover for the channel volume faders. Off by default: most DJs want
// the fader to be authoritative even right after a layer switch.
MC3000.softTakeoverVolume = false;

// --- Effects ----------------------------------------------------------------
// Load a different effect into every effect slot of FX1 and FX2 at start-up, so
// the EFX buttons each switch on their own effect. false = keep whatever Mixxx
// had loaded from the previous session.
MC3000.loadEffectsAtStartup = true;

// Which effects to load, one per slot: FX1 slots first, then FX2.
//
// These are POSITIONS in the effect selector list, not names and not IDs.
// Mixxx offers a script no way to ask for an effect by name - only "step to the
// next one" - so the mapping empties the slot and steps forward N times.
// Position 1 is the first entry below "---". (Verified against the Mixxx
// source: EffectSlot::slotLoadedEffectRequest and VisibleEffectsList::next
// both count 1-based into the visible effects list.)
//
// WHICH ORDER that list is in depends on your Mixxx version:
//
//   The default below was read off this machine's own list:
//       FX1: 12 Flanger, 17 Phaser, 18 Reverb
//       FX2:  2 Autopan,  8 Echo,   20 Tremolo
//
//   For reference, on a stock Mixxx 2.3 (list always sorted alphabetically)
//   the same six effects sit at [9, 16, 17, 1, 7, 19].
//
//   Mixxx 2.4  - alphabetically that would be [10, 17, 19, 1, 8, 21]
//   Mixxx 2.5  - alphabetically that would be [11, 19, 21, 1, 9, 23]
//
//   BUT from 2.4 onwards Mixxx no longer sorts this list. It reads the order
//   out of effects.xml, which is the order shown in Preferences -> Effects and
//   which you can drag around; newly added effects are put at the TOP. So on
//   2.4/2.5 the numbers above only hold if you have never reordered anything.
//
// Any LV2 or LADSPA plug-in, or an effect hidden in preferences, shifts
// everything as well. The list in Preferences -> Effects is always the
// authoritative answer: count down it, first entry = 1.
//
// Or skip all of this: choose the six effects by hand in the Mixxx GUI once and
// set MC3000.loadEffectsAtStartup = false. Mixxx remembers them between runs.
MC3000.startupEffects = [12, 17, 18, 2, 8, 20];

// Switch every freshly loaded effect off, so an EFX button turns its effect ON
// rather than off. Only applies when loadEffectsAtStartup is true.
MC3000.startEffectsDisabled = true;

// Keep the FX1 / FX2 routing pointed at whichever deck its side is showing.
// Without this, switching A -> C leaves the effect unit assigned to deck A.
MC3000.fxFollowsDeck = true;

// Level for every sampler at start-up (0.0 .. 1.0). null = leave them alone.
MC3000.samplerVolume = 0.5;

// CHANNEL SELECT A/C, B/D (notes 14/15) flip MC3000.deckOnSide for that side
// between its two decks (A<->C or B<->D), matching the button's own hardware
// lamp, so that LOAD A/C, LOAD B/D (and anything else reading deckOnSide)
// follow whichever deck the lamp shows - see MC3000.deckButton() for why this
// button is safe to repurpose away from its designed job for this.
//
// Denon's own manual states that pressing DECK CHG automatically pulses this
// button's signal too ("the control channel switches in synchronization"),
// which would otherwise flip deckOnSide right back on every DECK CHG press.
// That pulse is a real press+release, just an extremely short one (~1ms,
// measured on real hardware) since nothing is actually holding the button
// down - a genuine press is held far longer. Only a release after at least
// this many milliseconds counts as a real press; anything shorter is treated
// as the DECK CHG sync pulse and ignored.
MC3000.channelSelectMinHoldMs = 30;

// --- Deck selection ---------------------------------------------------------
// Deck buttons, per the hardware:
//
//   * DECK CHG A / B / C / D (notes 3 / 8 / 10... see deckChgNote) are four
//     physical buttons. Each one selects its deck outright and moves everything
//     - LEDs, loops, cue, EQ, pitch - onto it.
//   * DECK A/C (note 14) and DECK B/D (note 15) are NOT deck-section buttons
//     at all, despite the owner's manual's plain-English description ("DECK
//     A/C") and every earlier revision of this file reading them as a
//     "recall the previous deck" toggle. Per the official MIDI command list
//     and Denon's own Traktor/VirtualDJ mapping guides, these are "CHANNEL
//     SELECT A/C" / "CHANNEL SELECT B/D" - a MIXER control that picks which
//     deck feeds the physical channel strip, and this mapping has no
//     ambiguity for it to resolve (every deck already has its own dedicated
//     CC for LEVEL/EQ/fader). That is why, as of v3.5, a genuine press flips
//     MC3000.deckOnSide for that side between its two decks - matching the
//     button's own lamp - which is exactly what LOAD A/C, LOAD B/D read, so
//     the two buttons together work as "recall the previous deck, then load
//     into it" even though nothing here can move the jog wheel/play/cue
//     section itself. See MC3000.deckButton() for the full story, including
//     the DECK CHG sync pulse (owner's manual p.22, item 21) these buttons
//     fire automatically and how it is filtered out by hold duration.
//
// After a DECK CHG changes a side, that side's deck-section messages (play,
// cue, jog, pitch, ...) move to the new deck's own MIDI channel - confirmed on
// real hardware, see deckGroup(). LED feedback is (dis)connected and
// retriggered on each change so it always follows the selected deck - this is
// the standard Mixxx deck-switch pattern.

// Light the DECK CHG A/B/C/D indicator for the selected deck by sending a note
// to the controller.
//
// Default OFF (v3.2): tested directly against the real MC3000 on 2026-08-05 -
// pressing a DECK CHG button lit its own lamp (and, for A/C or B/D, the
// toggle lamp too) instantly, with Mixxx not even running and no MIDI sent at
// all. These lamps are hardware-autonomous, not MIDI-driven, on this unit.
// Echoing them from the script is therefore redundant at best; leave this off
// unless you find a unit whose lamps do NOT self-update.
MC3000.echoDeckChgLeds = false;

// How the DECK CHG lamps are addressed. Each lamp sits on its own deck's MIDI
// channel (A on ch0, C on ch1, B on ch2, D on ch3) and is lit by its DECK CHG
// note (A=3, B=8, C=9, D=10). These are the on/off velocities; if the lamps do
// not respond, try swapping to the LED opcodes the rest of the panel uses by
// setting MC3000.deckChgLedUsesLedOpcode = true.
MC3000.deckChgLedOn = 0x40;
MC3000.deckChgLedOff = 0x00;
MC3000.deckChgLedUsesLedOpcode = false;

// Diagnostic: when true, every deck/mixer/load button press is logged with the
// exact bytes and the resulting state, so a misbehaving button can be caught
// without swapping to the sniffer preset. Leave false in normal use.
MC3000.traceDeckButtons = false;

// --- Feedback --------------------------------------------------------------
// Drive the 7-segment channel level meters from Mixxx.
MC3000.enableVuMeters = true;
// Blink the SYNC LED when the track is close to the end. OFF by default now
// that the SYNC button latches: the LED should show sync lock, not two things.
MC3000.enableEndOfTrackWarning = false;
MC3000.endOfTrackWarningPosition = 0.9;

// ============================================================================
//  HARDWARE CONSTANTS - do not change, these come from the MC3000 manual
//  ("MIDI command list", pages 15-17 of the owner's manual).
// ============================================================================

// ============================================================================
//  DECK ASSIGNMENT - the single source of truth. Edit ONLY this table.
//
//  It maps each physical MC3000 deck (the letters printed on the DECK CHG
//  buttons) to the Mixxx deck it should control:
//
//      A / C  are the LEFT  side (channels 0 and 1)
//      B / D  are the RIGHT side (channels 2 and 3)
//
//  If a physical DECK CHG button controls the wrong Mixxx deck, swap the two
//  numbers on that side here - nothing else in the file needs changing.
//
//  Natural order: A->Channel1, C->Channel3, B->Channel2, D->Channel4. This is
//  the standard Mixxx 4-deck layout. If a physical deck ends up controlling the
//  wrong Mixxx deck, swap the two numbers on that side (e.g. deckForA and
//  deckForC) - and change DECK_FROM_CH in the XML generator to match.
// ============================================================================
MC3000.deckForA = 1;   // physical DECK A  -> Mixxx [Channel1]
MC3000.deckForC = 3;   // physical DECK C  -> Mixxx [Channel3]
MC3000.deckForB = 2;   // physical DECK B  -> Mixxx [Channel2]
MC3000.deckForD = 4;   // physical DECK D  -> Mixxx [Channel4]

// Everything below is DERIVED from the table above - do not edit by hand.
//
// Physical channels are fixed by the hardware: the left side transmits on
// channel 0 when showing its A-deck and channel 1 when showing its C-deck; the
// right side uses channel 2 (B) and channel 3 (D). Confirmed on real hardware
// on 2026-08-05 (see deckGroup() below): the deck section's MIDI channel DOES
// reliably move with the layer, so deckGroup() reads it directly.
//
//   channel 0 -> deckForA    channel 1 -> deckForC
//   channel 2 -> deckForB    channel 3 -> deckForD
MC3000.deckFromChannel = [MC3000.deckForA, MC3000.deckForC,
                          MC3000.deckForB, MC3000.deckForD];

// Mixxx deck number -> MIDI channel (inverse of the table above)
MC3000.channelFromDeck = (function () {
    var m = [];
    m[MC3000.deckForA - 1] = 0;
    m[MC3000.deckForC - 1] = 1;
    m[MC3000.deckForB - 1] = 2;
    m[MC3000.deckForD - 1] = 3;
    return m;
})();

// MIDI channel (0-based) -> physical side (0 = left, 1 = right)
MC3000.sideFromChannel = [0, 0, 1, 1];

// LED numbers, sent as: 0xB<channel>, <74 on | 75 off | 76 blink>, <number>
MC3000.led = {
    vinyl: 6,
    keylock: 8,
    sync: 9,
    cue: 38,
    play: 39,
    loopIn: 36,
    loopOut: 64,
    autoLoop: 43,
    fxOn1: 90,
    fxOn2: 91,
    // CUE1..CUE4 then CUE5..CUE8
    hotcue: [17, 19, 21, 23, 48, 50, 52, 54],
    // EFX.1..EFX.4 buttons, left deck section (FX1) and right section (FX2)
    efx: [[92, 93, 94, 95], [96, 97, 98, 99]],
    // SAMP.1..SAMP.4 buttons, left and right section
    // (v3.2, reverted) samp[0][3] was briefly changed 32 -> 31 based on a
    // reading of the official MIDI command list, which appears to describe
    // "SAMP.4 (LEFT)" = 31 / "SAMP.4 Dimmer (LEFT)" = 32. That reading does
    // NOT match this hardware: mapped one address at a time against the real
    // unit on 2026-08-05 (SAMP MODE on, each address isolated and confirmed
    // by which of the 4 physical LEDs lit and at what brightness), the left
    // bank is 25/26 = pos1 bright/dim, 27/28 = pos2 bright/dim, 29 = pos3
    // bright, 30 = unused, 31 = pos3 dim, 32 = pos4 bright. The manual's
    // table either doesn't apply to this unit or was misread - the
    // trustworthy source here is the hardware test, so this is back to the
    // original 32. The right bank ([65, 67, 69, 71]) has NOT been verified
    // this way and may have the same kind of gap - it was never reported as
    // broken, so it has been left alone, but treat it as unconfirmed if
    // sampler LEDs on the right side ever look wrong.
    samp: [[25, 27, 29, 32], [65, 67, 69, 71]]
};

// Channel-CUE (PFL) LEDs use the 80/81 opcodes instead of 74/75. Indexed by deck.
MC3000.pflLed = [69, 81, 75, 87];        // deck 1(A), 2(B), 3(C), 4(D)
// First of the 7 level-meter LEDs, indexed by deck.
MC3000.vuLedBase = [10, 42, 26, 58];     // deck 1(A), 2(B), 3(C), 4(D)

// Note number -> hotcue number
MC3000.hotcueFromNote = {
    23: 1, 24: 2, 25: 3, 32: 4,      // CUE 1-4 (CUE 5-8 mode OFF)
    72: 5, 73: 6, 74: 7, 75: 8       // same buttons with CUE 5-8 mode ON
};

// EFX button note -> [effect unit, button index 0..3].
// The note number alone identifies the section, so the unit is NOT derived from
// the MIDI channel any more. That was why FX1 never did anything: the EFX
// section does not necessarily transmit on its own side's channel, so both
// sections were landing on EffectUnit2.
MC3000.efxButtonMap = {
    21: [1, 0], 18: [1, 1], 19: [1, 2], 20: [1, 3],   // left section  = FX1
    85: [2, 0], 82: [2, 1], 83: [2, 2], 84: [2, 3]    // right section = FX2
};

// EFX knob CC -> knob index 0..3 (0 = the EFX.1 knob, 3 = the EFX.4 knob).
//
// NOTE: the owner's manual claims CC 85 = EFX.1, 86 = EFX.2, 87 = EFX.3,
// 88 = EFX.4. On real hardware the knobs are numbered the same way as the EFX
// *buttons*, i.e. rotated by one: EFX.2, EFX.3, EFX.4, EFX.1. (Compare the
// button notes in the manual: 18 = EFX.2, 19 = EFX.3, 20 = EFX.4, 21 = EFX.1.)
// The table below follows the hardware, not the manual.
// [effect unit, knob index 0..3]. Same fix as the buttons: the unit comes from
// the CC number, not from the MIDI channel.
MC3000.efxKnobMap = {
    88: [1, 0], 85: [1, 1], 86: [1, 2], 87: [1, 3],   // left section  = FX1
    92: [2, 0], 89: [2, 1], 90: [2, 2], 91: [2, 3]    // right section = FX2
};

// SAMP.x button note -> sampler number
MC3000.samplerFromNote = {
    33: 1, 34: 2, 35: 3, 36: 4,      // left section
    49: 5, 50: 6, 51: 7, 52: 8       // right section
};
// SAMP.x knob CC -> sampler number (the same physical EFX knobs, while
// SAMP. MODE is on). Rotated by one for the same reason as MC3000.efxKnobMap.
// If your sampler gains come out shuffled, this is the table to change.
MC3000.samplerFromCC = {
    48: 2, 49: 3, 50: 4, 51: 1,
    52: 6, 53: 7, 54: 8, 55: 5
};

// ============================================================================
//  RUNTIME STATE
// ============================================================================

MC3000.shift = [false, false];              // per physical side
MC3000.deckOnSide = [MC3000.deckForA, MC3000.deckForB];  // each side starts on its A/B deck
MC3000.scratchEnabled = [true, true, true, true];
MC3000.scratching = [false, false, false, false];
MC3000.jogLocked = [false, false, false, false];
MC3000.pitchNeedsTakeover = [false, false, false, false];
MC3000.vuLedsLit = [-1, -1, -1, -1];
MC3000.endOfTrack = [false, false, false, false];
MC3000.connections = [];
// Timestamp (MC3000.now()) of the last note-14/15 press per side, used to
// measure hold duration - see MC3000.channelSelectMinHoldMs.
MC3000.channelSelectPressTime = [0, 0];

// ============================================================================
//  SMALL HELPERS
// ============================================================================

// Mixxx deck 1..4 -> the DECK CHG note that selects it (A, B, C, D)
MC3000.deckChgNote = [3, 8, 9, 10];

MC3000.deckFromChgNote = {3: 1, 8: 2, 9: 3, 10: 4};

MC3000.sideOf = function (channel) {
    return MC3000.sideFromChannel[channel & 0x03];
};

MC3000.deckFromGroup = function (group) {
    var m = group.match(/^\[Channel(\d+)\]$/);
    return m === null ? -1 : parseInt(m[1], 10);
};

// Every message from the deck section carries the channel of the deck that side
// is currently showing, so any of them can be used to keep the side/deck
// tracking honest. Cheap: selectDeck() returns immediately if nothing changed.
// Which deck each side is on, once the user has said so explicitly with a
// DECK CHG or DECK A/C / B/D press. Until then the mapping is allowed to guess
// from the channel messages arrive on; afterwards it must not, or the guess
// will keep overwriting the choice.
MC3000.deckExplicit = [false, false];


// The last layer change per side: when it happened and what triggered it
// ("deckchg" or "mixer"). switchDeck() uses these to fold the stray extra
// message of a single press into that one press. Two DIFFERENT sources close
// together are two real presses and both count.

MC3000.now = function () {
    return (new Date()).getTime();
};

// Resolve which Mixxx deck a deck-section message (play, cue, loops, jog, EQ,
// pitch, ...) belongs to, purely from the MIDI channel it arrived on.
//
// (v3.2) This used to read MC3000.deckOnSide instead - a software-tracked
// "focus" updated only by deckButton() - based on a claim in an earlier
// revision that the hardware does not reliably move the deck section's
// channel when the layer changes. That claim was never actually verified and
// turned out to be wrong: tested directly against the real MC3000 on
// 2026-08-05 (sending a LED update to a fixed channel while toggling between
// deck A and deck C on the left side), the panel only ever lit the LED sent
// on the CURRENTLY SHOWN deck's channel - channel 0 while on A, channel 1
// while on C - exactly as deckFromChannel predicts. So the channel alone is
// reliable and this goes back to reading it directly, which also removes an
// entire class of bug: deckOnSide silently drifting out of sync with what the
// hardware is actually showing (e.g. at Mixxx startup, if the unit was left
// on a non-default layer from a previous session).
//
// deckOnSide is still maintained by deckButton() (see applyDeck/selectDeck)
// for the things that cannot get their deck from this function: LOAD (its
// own channel does NOT move with the deck, unlike everything routed through
// deckGroup() - see MC3000.load and change #10), the SHIFT+EFX kill routing,
// FX-follows-deck, and the effect-enabled LED callbacks (which react to a
// button press or an engine state change, not an incoming per-deck message).
MC3000.deckGroup = function (channel) {
    return "[Channel" + MC3000.deckFromChannel[channel & 0x03] + "]";
};

// true when the SHIFT button of this side is held
MC3000.isShifted = function (channel) {
    return MC3000.shift[MC3000.sideOf(channel)];
};

// true when either SHIFT is held (used for the centre / browser section)
MC3000.anyShift = function () {
    return MC3000.shift[0] || MC3000.shift[1];
};

// Note-on with a non-zero velocity means "pressed"
MC3000.isPress = function (value, status) {
    return ((status & 0xF0) === 0x90) && value > 0;
};

MC3000.toggle = function (group, key) {
    engine.setValue(group, key, engine.getValue(group, key) ? 0 : 1);
};

// ---- logging ---------------------------------------------------------------
// Mixxx 2.4+ (QJSEngine) provides console.*; Mixxx <= 2.3 (QtScript) provides
// print(). Use whichever is there so the script never dies on a log call.
MC3000.log = function (message) {
    if (typeof console !== "undefined" && typeof console.log === "function") {
        console.log(message);
    } else if (typeof print === "function") {
        print(message);
    }
};

MC3000.warn = function (message) {
    if (typeof console !== "undefined" && typeof console.warn === "function") {
        console.warn(message);
    } else {
        MC3000.log("WARNING: " + message);
    }
};

// ---- version compatibility -------------------------------------------------
// Several controls this mapping would like to use only exist from Mixxx 2.4
// onwards. Probe for them once at start-up and fall back to the older names, so
// the same file works on Mixxx 2.1 through 2.5+.
MC3000.compat = {};

MC3000.controlExists = function (group, key) {
    try {
        var probe = engine.makeConnection(group, key, function () { return; });
        if (probe) {
            probe.disconnect();
            return true;
        }
    } catch (e) {
        return false;
    }
    return false;
};

// Return the first (group, key) pair that this Mixxx build actually has.
MC3000.pickControl = function (candidates) {
    for (var i = 0; i < candidates.length; i++) {
        if (MC3000.controlExists(candidates[i][0], candidates[i][1])) {
            return candidates[i];
        }
    }
    return candidates[candidates.length - 1];
};

MC3000.toggleCompat = function (name) {
    var target = MC3000.compat[name];
    if (target) {
        MC3000.toggle(target[0], target[1]);
    }
};

MC3000.detectCompat = function () {
    // hotcue_X_status is 2.4+, hotcue_X_enabled goes back to 1.8
    MC3000.compat.hotcueSuffix =
        MC3000.controlExists("[Channel1]", "hotcue_1_status") ? "status" : "enabled";
    // sync_leader is 2.4+, previously called sync_master
    MC3000.compat.syncLeader =
        MC3000.controlExists("[Channel1]", "sync_leader") ? "sync_leader" : "sync_master";
    // vu_meter is 2.4+, previously called VuMeter
    MC3000.compat.vuMeter =
        MC3000.controlExists("[Channel1]", "vu_meter") ? "vu_meter" : "VuMeter";
    // loop_remove is 2.4+
    MC3000.compat.loopRemove = MC3000.controlExists("[Channel1]", "loop_remove");
    // CloneFromDeck is 2.3+
    MC3000.compat.clone = MC3000.controlExists("[Channel1]", "CloneFromDeck");
    // the whole [Skin] group is 2.4+
    MC3000.compat.maximizeLibrary = MC3000.pickControl([
        ["[Skin]", "show_maximized_library"],
        ["[Master]", "maximize_library"]
    ]);
    MC3000.compat.showSamplers = MC3000.pickControl([
        ["[Skin]", "show_samplers"],
        ["[Samplers]", "show_samplers"]
    ]);
    MC3000.compat.showEffects = MC3000.pickControl([
        ["[Skin]", "show_effectrack"],
        ["[EffectRack1]", "show"]
    ]);
    MC3000.compat.showCoverArt = MC3000.pickControl([
        ["[Skin]", "show_library_coverart"],
        ["[Library]", "show_coverart"]
    ]);

    MC3000.log("Denon MC3000: detected Mixxx profile -> hotcue_X_" +
        MC3000.compat.hotcueSuffix + ", " + MC3000.compat.syncLeader + ", " +
        MC3000.compat.vuMeter + ", " + MC3000.compat.maximizeLibrary[0]);
};

// ---- effect slots ----------------------------------------------------------
// How many effect slots each unit really has. Mixxx decides this; a mapping
// cannot change it. So read it and adapt: if a unit has fewer slots than the
// MC3000 has EFX buttons, the extra button toggles the whole unit instead.
MC3000.effectSlots = [0, 0];

MC3000.countEffectSlots = function () {
    var u;
    for (u = 1; u <= 2; u++) {
        var n = engine.getValue("[EffectRack1_EffectUnit" + u + "]", "num_effectslots");
        if (!n || n < 1) {
            n = 3;                       // sane default if the control is absent
        }
        MC3000.effectSlots[u - 1] = Math.min(4, n);
    }
};

// Put one effect into a slot. loaded_effect is the direct route; where that is
// not writable, clear the slot and step forward with next_effect instead.
MC3000.loadEffectInto = function (group, index) {
    engine.setValue(group, "loaded_effect", index);
    if (engine.getValue(group, "loaded_effect") === index) {
        return;
    }
    engine.setValue(group, "clear", 1);
    engine.setValue(group, "clear", 0);
    for (var i = 0; i < index; i++) {
        engine.setValue(group, "next_effect", 1);
        engine.setValue(group, "next_effect", 0);
    }
};

// Report what the effect system looks like BEFORE anything is loaded. On Mixxx
// 2.4+ loaded_effect is readable, so this prints the exact position of whatever
// you have set up by hand in the GUI - which is the definitive answer to "what
// number is Flanger on this machine". On 2.3 that control does not exist and
// the positions come back as "n/a".
MC3000.reportEffects = function () {
    var available = engine.getValue("[Master]", "num_effectsavailable");
    var readable = MC3000.controlExists("[EffectRack1_EffectUnit1_Effect1]", "loaded_effect");
    var report = [];
    var u, m;
    for (u = 1; u <= 2; u++) {
        for (m = 1; m <= MC3000.effectSlots[u - 1]; m++) {
            var group = "[EffectRack1_EffectUnit" + u + "_Effect" + m + "]";
            report.push("FX" + u + "." + m + "=" +
                (readable ? engine.getValue(group, "loaded_effect") : "n/a"));
        }
    }
    MC3000.log("Denon MC3000: effects installed=" + available +
        ", slots=" + MC3000.effectSlots[0] + "+" + MC3000.effectSlots[1] +
        ", positions currently loaded: " + report.join(" "));
};

MC3000.loadStartupEffects = function () {
    if (!MC3000.loadEffectsAtStartup) {
        return;
    }
    var available = engine.getValue("[Master]", "num_effectsavailable");
    if (!available || available < 2) {
        MC3000.warn("Denon MC3000: no effects available to load");
        return;
    }
    var loaded = 0;
    var used = [];
    var u, m;
    for (u = 1; u <= 2; u++) {
        for (m = 1; m <= MC3000.effectSlots[u - 1]; m++) {
            var group = "[EffectRack1_EffectUnit" + u + "_Effect" + m + "]";
            var index = (MC3000.startupEffects === null)
                ? loaded + 1
                : MC3000.startupEffects[loaded];
            if (index === undefined || index === null || index < 1 || index > 200) {
                loaded++;
                continue;                // no effect asked for in this slot
            }
            MC3000.loadEffectInto(group, index);
            if (!engine.getValue(group, "loaded")) {
                MC3000.warn("Denon MC3000: nothing loaded into " + group +
                    " at position " + index);
            }
            if (MC3000.startEffectsDisabled) {
                engine.setValue(group, "enabled", 0);
            }
            used.push(index);
            loaded++;
        }
    }
    MC3000.log("Denon MC3000: effect positions " + used.join(",") +
        " loaded into " + MC3000.effectSlots[0] + "+" + MC3000.effectSlots[1] +
        " slots (" + available + " effects installed). If these are not the " +
        "effects you wanted, count them in Mixxx's effect selector and put " +
        "those numbers in MC3000.startupEffects.");
};

// ---- LED output ------------------------------------------------------------
// state: 0/false = off, 1/true = on, 2 = blink
// Low-level: light an LED on one specific MIDI channel.
MC3000.sendLed = function (channel, ledNumber, state) {
    var opcode = 75;                     // off
    if (state === 2) {
        opcode = 76;                     // blink
    } else if (state) {
        opcode = 74;                     // on
    }
    midi.sendShortMsg(0xB0 + channel, opcode, ledNumber);
};

// Which channel to light a deck's LEDs on: the channel belonging to the side
// currently showing that deck. Sending to channelFromDeck[] instead was why the
// LEDs stopped following a DECK A/C switch - the side was still on the old
// channel and never saw the message.
// Which MIDI channel to light a deck's LEDs on: the deck's own fixed channel.
// Because the hardware listens on the channel of whichever deck it currently
// shows, sending a deck's feedback on that deck's channel is always correct -
// the unit only "sees" the LEDs for the deck it is displaying.
MC3000.ledChannel = function (deck) {
    return MC3000.channelFromDeck[deck - 1];
};

MC3000.setLed = function (deck, ledNumber, state) {
    if (deck < 1 || deck > 4) {
        return;
    }
    var channel = MC3000.ledChannel(deck);
    if (channel === -1) {
        return;
    }
    MC3000.sendLed(channel, ledNumber, state);
};

// Channel-CUE LEDs and the level meters use a different pair of opcodes
MC3000.setLed2 = function (deck, ledNumber, on) {
    if (deck < 1 || deck > 4) {
        return;
    }
    midi.sendShortMsg(0xB0 + MC3000.channelFromDeck[deck - 1], on ? 80 : 81, ledNumber);
};

// Index of the EFX/SAMP LED bank for a deck: 0 = left section, 1 = right
MC3000.sectionOfDeck = function (deck) {
    return (deck === 1 || deck === 3) ? 0 : 1;
};

// ============================================================================
//  INIT / SHUTDOWN
// ============================================================================

MC3000.init = function (id) {
    MC3000.id = id;

    var deck, i;

    for (i = 0; i < 4; i++) {
        MC3000.scratchEnabled[i] = MC3000.vinylModeAtStartup;
    }

    MC3000.detectCompat();
    MC3000.countEffectSlots();
    MC3000.reportEffects();
    MC3000.loadStartupEffects();
    MC3000.allLedsOff();

    for (deck = 1; deck <= 4; deck++) {
        var group = "[Channel" + deck + "]";

        // ---- soft takeover on the shared mixer controls --------------------
        if (MC3000.softTakeoverEqAndGain) {
            engine.softTakeover(group, "pregain", true);
            engine.softTakeover("[EqualizerRack1_" + group + "_Effect1]", "parameter1", true);
            engine.softTakeover("[EqualizerRack1_" + group + "_Effect1]", "parameter2", true);
            engine.softTakeover("[EqualizerRack1_" + group + "_Effect1]", "parameter3", true);
            engine.softTakeover("[QuickEffectRack1_" + group + "]", "super1", true);
        }
        if (MC3000.softTakeoverVolume) {
            engine.softTakeover(group, "volume", true);
        }

        // ---- LED feedback --------------------------------------------------
        MC3000.connect(group, "play_indicator", MC3000.onPlay);
        MC3000.connect(group, "cue_indicator", MC3000.onCue);
        if (MC3000.keylockAtStartup !== null) {
            engine.setValue(group, "keylock", MC3000.keylockAtStartup ? 1 : 0);
        }
        MC3000.connect(group, "keylock", MC3000.onKeylock);
        MC3000.connect(group, "sync_enabled", MC3000.onSync);
        MC3000.connect(group, "pfl", MC3000.onPfl);
        MC3000.connect(group, "loop_enabled", MC3000.onLoopEnabled);
        MC3000.connect(group, "loop_start_position", MC3000.onLoopStart);
        MC3000.connect(group, "loop_end_position", MC3000.onLoopEnd);

        for (i = 1; i <= 8; i++) {
            MC3000.connect(group, "hotcue_" + i + "_" + MC3000.compat.hotcueSuffix,
                MC3000.onHotcue);
        }

        MC3000.connect("[EffectRack1_EffectUnit1]", "group_" + group + "_enable",
            MC3000.onFxAssign);
        MC3000.connect("[EffectRack1_EffectUnit2]", "group_" + group + "_enable",
            MC3000.onFxAssign);

        if (MC3000.enableVuMeters) {
            MC3000.connect(group, MC3000.compat.vuMeter, MC3000.onVuMeter);
        }
        if (MC3000.enableEndOfTrackWarning) {
            MC3000.connect(group, "playposition", MC3000.onPlayPosition);
        }

        // VINYL MODE LED is script state, not a Mixxx control
        MC3000.setLed(deck, MC3000.led.vinyl, MC3000.scratchEnabled[deck - 1]);
    }

    // Sampler play state -> the SAMP.1..4 button LEDs of each side
    for (i = 1; i <= 8; i++) {
        MC3000.connect("[Sampler" + i + "]", "play", MC3000.onSampler);
    }

    // Ask the controller to re-transmit the physical position of every fader,
    // knob and slider so Mixxx starts in sync with the hardware.
    // ("ALL SLIDER/VOLUME/FADER REQUEST", owner's manual p.17)
    engine.beginTimer(1500, function () {
        for (var ch = 0; ch < 4; ch++) {
            midi.sendShortMsg(0xB0 + ch, 74, 57);
        }
    }, true);

    if (MC3000.samplerVolume !== null) {
        for (i = 1; i <= 8; i++) {
            engine.setValue("[Sampler" + i + "]", "volume", MC3000.samplerVolume);
        }
    }

    // EFX button LEDs follow the effect slots of their own unit,
    // and the fourth one follows the headphone routing of that unit
    for (i = 1; i <= 2; i++) {
        var slots = Math.min(3, MC3000.effectSlots[i - 1]);
        for (deck = 1; deck <= slots; deck++) {
            MC3000.connect("[EffectRack1_EffectUnit" + i + "_Effect" + deck + "]",
                "enabled", MC3000.onEffectEnabled);
        }
        MC3000.connect("[EffectRack1_EffectUnit" + i + "]", "group_[Headphone]_enable",
            MC3000.onFxHeadphone);
    }

    // light the DECK CHG lamps for the two decks shown at start-up
    if (MC3000.echoDeckChgLeds) {
        MC3000.sendDeckChgLed(MC3000.deckOnSide[0], true);
        MC3000.sendDeckChgLed(MC3000.deckOnSide[1], true);
    }

    MC3000.log("Denon MC3000: mapping initialised (" + id + ")");
};

MC3000.shutdown = function () {
    var i;
    for (i = 0; i < MC3000.connections.length; i++) {
        MC3000.connections[i].disconnect();
    }
    MC3000.connections = [];
    MC3000.allLedsOff();
};

MC3000.connect = function (group, key, callback) {
    var connection = engine.makeConnection(group, key, callback);
    if (connection) {
        MC3000.connections.push(connection);
        connection.trigger();
    } else {
        MC3000.warn("Denon MC3000: no such control: " + group + "," + key);
    }
    return connection;
};

MC3000.allLedsOff = function () {
    var deck, ch, i, section;
    // Address the four channels directly rather than going through setLed:
    // at start-up two of the decks are not on a side, so setLed would skip them
    // and leave their LEDs lit from the previous session.
    for (ch = 0; ch <= 3; ch++) {
        section = MC3000.sideFromChannel[ch];
        MC3000.sendLed(ch, MC3000.led.vinyl, 0);
        MC3000.sendLed(ch, MC3000.led.keylock, 0);
        MC3000.sendLed(ch, MC3000.led.sync, 0);
        MC3000.sendLed(ch, MC3000.led.cue, 0);
        MC3000.sendLed(ch, MC3000.led.play, 0);
        MC3000.sendLed(ch, MC3000.led.loopIn, 0);
        MC3000.sendLed(ch, MC3000.led.loopOut, 0);
        MC3000.sendLed(ch, MC3000.led.autoLoop, 0);
        MC3000.sendLed(ch, MC3000.led.fxOn1, 0);
        MC3000.sendLed(ch, MC3000.led.fxOn2, 0);
        for (i = 0; i < 8; i++) {
            MC3000.sendLed(ch, MC3000.led.hotcue[i], 0);
        }
        for (i = 0; i < 4; i++) {
            MC3000.sendLed(ch, MC3000.led.efx[section][i], 0);
            MC3000.sendLed(ch, MC3000.led.samp[section][i], 0);
        }
    }
    for (deck = 1; deck <= 4; deck++) {
        MC3000.setLed2(deck, MC3000.pflLed[deck - 1], false);
        for (i = 0; i < 7; i++) {
            MC3000.setLed2(deck, MC3000.vuLedBase[deck - 1] + i, false);
        }
        MC3000.vuLedsLit[deck - 1] = -1;
    }
};

// ============================================================================
//  SHIFT AND DECK SELECTION
// ============================================================================

// SHIFT (LEFT DECK) = note 96, SHIFT (RIGHT DECK) = note 97.
// The side is taken from the NOTE, the deck from the CHANNEL, so the state can
// never get out of sync when you change deck layer while holding SHIFT.
MC3000.shiftButton = function (channel, control, value, status) {
    // The side comes from the NOTE (96 = left, 97 = right). SHIFT does NOT touch
    // the deck focus - only the DECK CHG / DECK A/C / DECK B/D buttons do that.
    // (An earlier version re-selected the deck here from the SHIFT message's
    // channel, which quietly overwrote the focus every time you touched SHIFT.)
    var side = (control === 96) ? 0 : 1;
    MC3000.shift[side] = ((status & 0xF0) === 0x90) && value > 0;
};

// DECK CHG A/B/C/D (notes 3/8/9/10): select that deck outright. Dispatched by
// a switch on the note, which names the deck unambiguously (note 9 is always
// DECK CHG C) regardless of which channel the press arrives on.
//
// This does NOT need to change anything performance-related in Mixxx: the
// hardware itself moves the deck section to the selected deck's own MIDI
// channel (confirmed on real hardware, see deckGroup()), so every subsequent
// play/cue/loop/pitch/jog message already arrives on the right channel with
// no help from this handler. What this handler DOES still need to do is
// update MC3000.deckOnSide, the "which deck is this side showing" mirror used
// by the handful of controls that cannot get their deck from an incoming
// channel: LOAD's own group used to be one of these but no longer is (see
// MC3000.load, v3.2) - what remains is FX-follows-deck, the SHIFT+EFX kill
// routing, and the effect-enabled LED callbacks, all of which react to state
// changes or button presses rather than a per-deck message.
//
// Every path ends in MC3000.applyDeck(side, deck), the single place that
// updates deckOnSide and repaints LEDs, so it cannot get out of step.
//
// Notes 14/15 (DECK A/C, DECK B/D) are NOT deck-switch buttons at all - they
// are Denon's own "CHANNEL SELECT A/C" / "CHANNEL SELECT B/D" (owner's manual
// p.22, item 21): which deck feeds the physical LEVEL/EQ/fader knobs, with
// its own hardware-autonomous "A"/"C" (or "B"/"D") lamp. This mapping gives
// every deck its own dedicated CC for LEVEL/EQ/fader already, so that job has
// nothing to do here - instead (v3.5), a genuine press flips MC3000.deckOnSide
// for that side between its two decks, same as the lamp shows. This does NOT
// move the jog wheel/play/cue section - that genuinely cannot be done from
// this button (proven on real hardware, see below) - but it is exactly what
// LOAD A/C, LOAD B/D read to decide their target deck, so pressing CHANNEL
// SELECT and then LOAD sends the track to whichever deck the lamp shows.
//
// The manual also states that pressing DECK CHG automatically pulses this
// button's own signal in sync - confirmed on real hardware as a ~1ms
// press+release arriving right after a DECK CHG press, with no real button
// behind it. Flipping deckOnSide on every DECK CHG press would silently
// re-break the focus tracking DECK CHG itself just set, so notes 14/15 are
// handled by hold duration: only a release after at least
// MC3000.channelSelectMinHoldMs counts as a genuine press. This needs both
// press AND release (unlike DECK CHG A/B/C/D below, which only act on
// press), so it is handled first and returns before the press-only gate.
MC3000.deckButton = function (channel, control, value, status) {
    if (control === 14 || control === 15) {
        var csSide = (control === 14) ? 0 : 1;
        var csPressed = MC3000.isPress(value, status);
        var csReleased = !csPressed && (((status & 0xF0) === 0x80) ||
            ((status & 0xF0) === 0x90 && value === 0));
        if (csPressed) {
            MC3000.channelSelectPressTime[csSide] = MC3000.now();
        } else if (csReleased) {
            if (MC3000.now() - MC3000.channelSelectPressTime[csSide] >=
                    MC3000.channelSelectMinHoldMs) {
                // Flip this side's focus between its two decks (A<->C or
                // B<->D). LOAD A/C, LOAD B/D already read deckOnSide, so this
                // is what makes "press CHANNEL SELECT, then press LOAD"
                // target the deck you just switched to - see v3.5 change #15.
                // Deliberately touches nothing else (no LEDs, no FX-follow):
                // the hardware already lights its own A/C, B/D lamp on its
                // own, and the jog wheel/play/cue section genuinely cannot
                // move from this button (see the header comment above).
                if (csSide === 0) {
                    MC3000.deckOnSide[0] = (MC3000.deckOnSide[0] === MC3000.deckForA)
                        ? MC3000.deckForC : MC3000.deckForA;
                } else {
                    MC3000.deckOnSide[1] = (MC3000.deckOnSide[1] === MC3000.deckForB)
                        ? MC3000.deckForD : MC3000.deckForB;
                }
            }
        }
        return;
    }

    if (!MC3000.isPress(value, status)) {
        return;
    }
    var side;
    var deck;

    switch (control) {
    case 3:   side = 0; deck = MC3000.deckForA; break;   // DECK CHG A
    case 9:   side = 0; deck = MC3000.deckForC; break;   // DECK CHG C
    case 8:   side = 1; deck = MC3000.deckForB; break;   // DECK CHG B
    case 10:  side = 1; deck = MC3000.deckForD; break;   // DECK CHG D
    default:  return;
    }

    // Delegate to applyDeck()/selectDeck(), the functions that already do the
    // full job of a deck switch: move deckOnSide, clear the outgoing deck's
    // LEDs, repaint the incoming deck's LEDs, move the DECK CHG lamp, reassign
    // this side's effect unit if fxFollowsDeck is on, and arm pitch-fader
    // soft takeover. (v3.1: this handler used to reimplement a subset of that
    // inline and skipped the LED-clear and FX-follow steps - see change #7 in
    // the file header.)
    MC3000.applyDeck(side, deck);
};

MC3000.applyDeck = function (side, deck) {
    if (MC3000.traceDeckButtons) {
        MC3000.log("applyDeck side=" + side + " deck=" + deck +
            " from deckOnSide=[" + MC3000.deckOnSide + "]");
    }
    MC3000.deckExplicit[side] = true;         // stop guessing from the channel
    if (deck === MC3000.deckOnSide[side]) {
        MC3000.refreshDeckLeds(deck);         // already there: just resync
        return;
    }
    MC3000.selectDeck(side, deck);            // does the LED work via selectDeck
};

// Remember which deck a physical side is showing. Called both from the
// DECK CHG buttons and, as a safety net, whenever a SHIFT press tells us which
// channel that side is on.
MC3000.selectDeck = function (side, deck) {
    if (MC3000.deckOnSide[side] === deck) {
        return false;                    // nothing changed, keep the MIDI bus quiet
    }
    var previous = MC3000.deckOnSide[side];
    MC3000.deckOnSide[side] = deck;

    // Move this side's effect unit onto the deck that is now showing, so FX1
    // keeps acting on the left deck and FX2 on the right one.
    if (MC3000.fxFollowsDeck) {
        var unit = "[EffectRack1_EffectUnit" + (side + 1) + "]";
        if (engine.getValue(unit, "group_[Channel" + previous + "]_enable")) {
            engine.setValue(unit, "group_[Channel" + previous + "]_enable", 0);
            engine.setValue(unit, "group_[Channel" + deck + "]_enable", 1);
        }
    }

    // the shared pitch fader now belongs to another deck
    MC3000.pitchNeedsTakeover[deck - 1] = MC3000.pitchSoftTakeover;

    // The deck leaving this side is no longer visible on any channel, so blank
    // its per-deck LEDs before painting the incoming deck. Without this, the old
    // deck's play/cue/loop lamps stay lit on the channel it left behind - which
    // is the "blinking on the other channel" symptom.
    // The departing deck's LEDs live on that deck's own channel.
    MC3000.clearDeckLeds(previous, MC3000.channelFromDeck[previous - 1]);
    MC3000.refreshDeckLeds(deck);
    MC3000.updateDeckChgLeds(side, previous, deck);
    return true;
};

// Turn off the DECK CHG lamp for the deck we left and turn on the one we moved
// to. Each lamp is on ITS OWN deck's channel, not the side's current channel -
// deck C's lamp is note 9 on channel 1, deck A's is note 3 on channel 0 - so
// they are addressed by channelFromDeck[], one message each.
MC3000.updateDeckChgLeds = function (side, previous, current) {
    if (!MC3000.echoDeckChgLeds) {
        return;
    }
    MC3000.sendDeckChgLed(previous, false);
    MC3000.sendDeckChgLed(current, true);
};

MC3000.sendDeckChgLed = function (deck, on) {
    var channel = MC3000.channelFromDeck[deck - 1];
    var note = MC3000.deckChgNote[deck - 1];
    if (MC3000.deckChgLedUsesLedOpcode) {
        // same 0xB0 / on=74 / off=75 scheme the hot cues and EFX buttons use
        midi.sendShortMsg(0xB0 + channel, on ? 74 : 75, note);
    } else {
        // note-on to light, note-off to clear
        midi.sendShortMsg((on ? 0x90 : 0x80) + channel, note,
            on ? MC3000.deckChgLedOn : MC3000.deckChgLedOff);
    }
};

// Turn off every per-deck LED for a deck that is leaving the panel. It is no
// longer on any side, so setLed could not route it - send straight to the
// channel it was using.
MC3000.clearDeckLeds = function (deck, channel) {
    var section = MC3000.sectionOfDeck(deck);
    var lamps = [MC3000.led.play, MC3000.led.cue, MC3000.led.keylock,
        MC3000.led.sync, MC3000.led.loopIn, MC3000.led.loopOut,
        MC3000.led.autoLoop, MC3000.led.vinyl];
    var i;
    for (i = 0; i < lamps.length; i++) {
        MC3000.sendLed(channel, lamps[i], false);
    }
    for (i = 0; i < 8; i++) {
        MC3000.sendLed(channel, MC3000.led.hotcue[i], false);
    }
    for (i = 0; i < 4; i++) {
        MC3000.sendLed(channel, MC3000.led.efx[section][i], false);
        MC3000.sendLed(channel, MC3000.led.samp[section][i], false);
    }
};

MC3000.refreshDeckLeds = function (deck) {
    MC3000.setLed(deck, MC3000.led.vinyl, MC3000.scratchEnabled[deck - 1]);
    var group = "[Channel" + deck + "]";
    MC3000.onPlay(engine.getValue(group, "play_indicator"), group);
    MC3000.onCue(engine.getValue(group, "cue_indicator"), group);
    MC3000.onKeylock(engine.getValue(group, "keylock"), group);
    MC3000.onSync(engine.getValue(group, "sync_enabled"), group);
    MC3000.onLoopEnabled(engine.getValue(group, "loop_enabled"), group);
    // the loop-in and loop-out markers belong to the deck too, so they have to
    // be re-read on a layer change or they keep showing the previous deck
    MC3000.onLoopStart(engine.getValue(group, "loop_start_position"), group);
    MC3000.onLoopEnd(engine.getValue(group, "loop_end_position"), group);
    var i;
    for (i = 1; i <= 8; i++) {
        MC3000.onHotcue(
            engine.getValue(group, "hotcue_" + i + "_" + MC3000.compat.hotcueSuffix),
            group, "hotcue_" + i + "_" + MC3000.compat.hotcueSuffix);
    }
    var section = MC3000.sectionOfDeck(deck);
    var unit = "[EffectRack1_EffectUnit" + (section + 1) + "]";
    for (i = 0; i < 3; i++) {
        MC3000.setLed(deck, MC3000.led.efx[section][i],
            engine.getValue("[EffectRack1_EffectUnit" + (section + 1) +
                "_Effect" + (i + 1) + "]", "enabled"));
    }
    MC3000.setLed(deck, MC3000.led.efx[section][3],
        engine.getValue(unit, "group_[Headphone]_enable"));
    // sampler LEDs of this side follow the deck onto its new channel
    var first = (MC3000.sectionOfDeck(deck) === 0) ? 1 : 5;
    for (i = 0; i < 4; i++) {
        MC3000.setLed(deck, MC3000.led.samp[MC3000.sectionOfDeck(deck)][i],
            engine.getValue("[Sampler" + (first + i) + "]", "play"));
    }
};

// DECK A/C = note 14 (left mixer strip), DECK B/D = note 15 (right strip).
// These pick which deck the mixer channel drives. The channel the message
// arrives on tells us which deck that is, which also keeps the side/deck
// tracking correct even if the DECK CHG notes are missed.

// ============================================================================
//  TRANSPORT
// ============================================================================

// PLAY / PAUSE, note 67.  SHIFT = reverse roll (censor) while held.
MC3000.play = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    var pressed = ((status & 0xF0) === 0x90) && value > 0;
    if (MC3000.isShifted(channel)) {
        engine.setValue(group, "reverseroll", pressed ? 1 : 0);
        return;
    }
    if (pressed) {
        MC3000.toggle(group, "play");
    }
};

// CUE, note 66.  SHIFT = jump to the beginning of the track.
MC3000.cue = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    var pressed = ((status & 0xF0) === 0x90) && value > 0;
    if (MC3000.isShifted(channel)) {
        if (pressed) {
            engine.setValue(group, "playposition", 0);
        }
        return;
    }
    engine.setValue(group, "cue_default", pressed ? 1 : 0);
};

// SYNC, note 107.
// Mapped to sync_enabled (sync lock), NOT beatsync. beatsync is a one-shot
// tempo match that can never be undone from the controller, which is why the
// old behaviour felt like "everything stays linked forever". sync_enabled is a
// latching control that Mixxx itself interprets as: short press = toggle sync
// lock on/off, press and hold for a second = momentary sync. So the same button
// now both engages AND releases the sync.
// SHIFT = make this deck the sync leader.
MC3000.sync = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    var pressed = ((status & 0xF0) === 0x90) && value > 0;

    if (MC3000.isShifted(channel)) {
        if (pressed) {
            MC3000.toggle(group, MC3000.compat.syncLeader);
        }
        return;
    }
    engine.setValue(group, "sync_enabled", pressed ? 1 : 0);
};

// KEY LOCK, note 6.  SHIFT = tap the BPM.
MC3000.keylock = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    if (!MC3000.isPress(value, status)) {
        return;
    }
    if (MC3000.isShifted(channel)) {
        engine.setValue(group, "bpm_tap", 1);
    } else {
        MC3000.toggle(group, "keylock");
    }
};

// PITCH BEND +/-, notes 12 and 13.
// Unshifted: temporary tempo nudge. SHIFT: fast forward / rewind.
MC3000.bend = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    var pressed = ((status & 0xF0) === 0x90) && value > 0;
    var up = (control === 12);
    if (MC3000.isShifted(channel)) {
        engine.setValue(group, up ? "fwd" : "back", pressed ? 1 : 0);
    } else {
        engine.setValue(group, up ? "rate_temp_up" : "rate_temp_down", pressed ? 1 : 0);
    }
};

// ============================================================================
//  PITCH FADER (0xEn, 14 bit) - mapped in the XML WITHOUT a <midino> element
// ============================================================================

MC3000.pitch = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    // Deck comes from the same focus-tracked group, not the fixed channel table -
    // otherwise this silently kept indexing pitchNeedsTakeover for the deck this
    // side used to show after a DECK A/C or DECK B/D switch.
    var deck = MC3000.deckFromGroup(group);
    // 14-bit little endian: control = LSB, value = MSB.
    // 8192 is the exact centre of the 14-bit range, so dividing the offset by
    // 8192 makes the fader detent land on precisely 0.00 %.
    var raw = (value << 7) | control;
    var rate = (raw - 8192) / 8192.0;
    if (MC3000.invertPitchFader) {
        rate = -rate;
    }
    rate = Math.max(-1.0, Math.min(1.0, rate));

    if (MC3000.pitchNeedsTakeover[deck - 1]) {
        var current = engine.getValue(group, "rate");
        if (Math.abs(rate - current) > MC3000.pitchTakeoverTolerance) {
            return;                     // ignore until the fader catches up
        }
        MC3000.pitchNeedsTakeover[deck - 1] = false;
    }
    engine.setValue(group, "rate", rate);
};

// ============================================================================
//  JOG WHEEL
// ============================================================================

// Jog wheel touch sensor, note 81.
MC3000.jogTouch = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    // Deck comes from the focus-tracked group (see deckGroup()), not the fixed
    // channel table: on this hardware the deck section's channel does not
    // reliably follow a DECK A/C / DECK B/D switch, so deckOf()/deckFromChannel
    // kept scratch-enabling the deck this side used to show instead of the one
    // actually selected. This was the "jog wheel / scratch on the wrong deck
    // after switching to C or D" bug.
    var deck = MC3000.deckFromGroup(group);
    var touched = ((status & 0xF0) === 0x90) && value > 0;

    if (MC3000.jogLocked[deck - 1] || !MC3000.scratchEnabled[deck - 1]) {
        return;
    }
    if (touched) {
        engine.scratchEnable(deck,
            MC3000.jogTicksPerRevolution,
            MC3000.jogScratchRpm,
            MC3000.scratchAlpha,
            MC3000.scratchBeta);
        MC3000.scratching[deck - 1] = true;
    } else {
        engine.scratchDisable(deck);
        MC3000.scratching[deck - 1] = false;
    }
};

// Jog wheel rotation, CC 81. 1..63 = reverse, 65..127 = forward.
MC3000.jogWheel = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    // See jogTouch() above: deck must come from the focus-tracked group.
    var deck = MC3000.deckFromGroup(group);
    if (MC3000.jogLocked[deck - 1]) {
        return;
    }
    var delta = value - 64;
    if (delta === 0) {
        return;
    }

    // SHIFT + jog = scrub quickly through the loaded track
    if (MC3000.isShifted(channel)) {
        var position = engine.getValue(group, "playposition");
        position += delta * MC3000.jogSeekSensitivity;
        engine.setValue(group, "playposition", Math.max(0, Math.min(1, position)));
        return;
    }

    if (MC3000.scratching[deck - 1]) {
        engine.scratchTick(deck, delta);
    } else {
        engine.setValue(group, "jog", delta * MC3000.jogBendSensitivity);
    }
};

// VINYL MODE, note 4. SHIFT = lock the jog wheel completely.
MC3000.vinylMode = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    if (!MC3000.isPress(value, status)) {
        return;
    }
    // See jogTouch() above: deck must come from the focus-tracked group.
    var deck = MC3000.deckFromGroup(group);

    if (MC3000.isShifted(channel)) {
        MC3000.jogLocked[deck - 1] = !MC3000.jogLocked[deck - 1];
        if (MC3000.jogLocked[deck - 1] && MC3000.scratching[deck - 1]) {
            engine.scratchDisable(deck);
            MC3000.scratching[deck - 1] = false;
        }
        // blink while the wheel is locked
        MC3000.setLed(deck, MC3000.led.vinyl,
            MC3000.jogLocked[deck - 1] ? 2 : MC3000.scratchEnabled[deck - 1]);
        return;
    }

    MC3000.scratchEnabled[deck - 1] = !MC3000.scratchEnabled[deck - 1];
    if (!MC3000.scratchEnabled[deck - 1] && MC3000.scratching[deck - 1]) {
        engine.scratchDisable(deck);
        MC3000.scratching[deck - 1] = false;
    }
    MC3000.setLed(deck, MC3000.led.vinyl, MC3000.scratchEnabled[deck - 1]);
};

// ============================================================================
//  HOT CUES
// ============================================================================

// CUE 1-4 (notes 23,24,25,32) and CUE 5-8 (notes 72,73,74,75).
// The CUE 5-8 mode button is handled by the hardware: it simply makes the same
// four buttons send the other set of notes.
// SHIFT = delete the hot cue.
MC3000.hotcue = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    var number = MC3000.hotcueFromNote[control];
    if (number === undefined) {
        return;
    }
    var pressed = ((status & 0xF0) === 0x90) && value > 0;

    if (MC3000.isShifted(channel)) {
        if (pressed) {
            engine.setValue(group, "hotcue_" + number + "_clear", 1);
        }
        return;
    }
    engine.setValue(group, "hotcue_" + number + "_activate", pressed ? 1 : 0);
};

// ============================================================================
//  LOOPS
// ============================================================================

// LOOP IN, note 55.  SHIFT = clear the loop.
// Drop the loop-in marker. loop_in is a push control: Mixxx starts "loop in
// adjust" mode while it is held, so the release MUST be sent as well. Only
// sending the press was what made these buttons behave strangely - the deck was
// left in adjust mode and the jog wheel kept dragging the marker around.
// SHIFT clears the loop instead.
MC3000.loopIn = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    var pressed = ((status & 0xF0) === 0x90) && value > 0;

    if (MC3000.isShifted(channel)) {
        if (pressed) {
            MC3000.clearLoop(group, "in");
        }
        return;
    }
    engine.setValue(group, "loop_in", pressed ? 1 : 0);
};

// Take the loop out of the deck. Mixxx 2.4+ has loop_remove; on older builds
// writing -1 to the marker position detaches it.
MC3000.clearLoop = function (group, which) {
    if (engine.getValue(group, "loop_enabled") > 0) {
        engine.setValue(group, "reloop_toggle", 1);
        engine.setValue(group, "reloop_toggle", 0);
    }
    if (MC3000.compat.loopRemove) {
        engine.setValue(group, "loop_remove", 1);
        engine.setValue(group, "loop_remove", 0);
    } else {
        engine.setValue(group,
            (which === "in") ? "loop_start_position" : "loop_end_position", -1);
    }
};

// LOOP OUT, note 57.  SHIFT = clear the loop.
MC3000.loopOut = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    var pressed = ((status & 0xF0) === 0x90) && value > 0;

    if (MC3000.isShifted(channel)) {
        if (pressed) {
            MC3000.clearLoop(group, "out");
        }
        return;
    }
    engine.setValue(group, "loop_out", pressed ? 1 : 0);
};

// AUTO LOOP, note 29.
// Unshifted: set / clear a loop of the current beatloop size.
// SHIFT: re-enter the last loop (reloop).
MC3000.autoLoop = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    if (!MC3000.isPress(value, status)) {
        return;
    }
    if (MC3000.isShifted(channel)) {
        engine.setValue(group, "reloop_toggle", 1);
        return;
    }
    if (engine.getValue(group, "loop_enabled")) {
        engine.setValue(group, "reloop_toggle", 1);
    } else {
        engine.setValue(group, "beatloop_activate", 1);
    }
};

// AUTO LOOP - / + , notes 105 and 106.
// Unshifted: halve / double the loop length (resizes an active loop too).
// SHIFT: move the whole loop one beat backward / forward.
MC3000.loopSize = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    if (!MC3000.isPress(value, status)) {
        return;
    }
    var bigger = (control === 106);

    if (MC3000.isShifted(channel)) {
        engine.setValue(group, bigger ? "loop_move_1_forward" : "loop_move_1_backward", 1);
        return;
    }
    var size = engine.getValue(group, "beatloop_size");
    size = bigger ? size * 2 : size / 2;
    size = Math.max(1 / 32, Math.min(64, size));
    engine.setValue(group, "beatloop_size", size);
};

// ============================================================================
//  EFX SECTION
//  EFX 1..3     : switch effect 1..3 of this side's unit on and off
//  EFX 4        : send this side's unit to the headphones
//                 (SHIFT = EQ low/mid/high kill and filter kill on the deck
//                  that side is showing)
//  EFX knobs    : effect unit wet/dry + the effect metaknobs
//  The unit is chosen by the note/CC number, never by the MIDI channel:
//  notes 21,18,19,20 and CC 88,85,86,87 are FX1; notes 85,82,83,84 and
//  CC 92,89,90,91 are FX2.
//  FX ON 1 / 2  : route this deck into EffectUnit 1 / 2
// ============================================================================

MC3000.efxButton = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    if (!MC3000.isPress(value, status)) {
        return;
    }
    var entry = MC3000.efxButtonMap[control];
    if (entry === undefined) {
        return;
    }
    var unitNo = entry[0];
    var index = entry[1];

    // The deck this side is showing, so the kill buttons follow the layer.
    var deck = MC3000.deckOnSide[unitNo - 1];

    if (MC3000.isShifted(channel)) {
        // SHIFT + EFX.1/2/3 = the three EQ kills, SHIFT + EFX.4 = filter kill
        if (index < 3) {
            MC3000.toggle("[EqualizerRack1_[Channel" + deck + "]_Effect1]",
                "button_parameter" + (index + 1));
        } else {
            MC3000.toggle("[QuickEffectRack1_[Channel" + deck + "]_Effect1]",
                "button_parameter1");
        }
        return;
    }

    // EFX.1..EFX.3 switch the unit's three effects, EFX.4 sends the whole unit
    // to the headphones instead.
    if (index < 3 && index < MC3000.effectSlots[unitNo - 1]) {
        MC3000.toggle("[EffectRack1_EffectUnit" + unitNo + "_Effect" + (index + 1) + "]",
            "enabled");
    } else if (index === 3) {
        MC3000.toggle("[EffectRack1_EffectUnit" + unitNo + "]", "group_[Headphone]_enable");
    }
};

MC3000.efxKnob = function (channel, control, value, status, group) {
    group = MC3000.deckGroup(channel);
    var entry = MC3000.efxKnobMap[control];
    if (entry === undefined) {
        return;
    }
    var unitNo = entry[0];
    var slot = entry[1];
    var unit = "[EffectRack1_EffectUnit" + unitNo + "]";
    var position = value / 127.0;

    if (slot === 0) {
        // EFX.1 knob: dry/wet of the whole unit, SHIFT = the unit super knob
        engine.setValue(unit, MC3000.isShifted(channel) ? "super1" : "mix", position);
    } else {
        engine.setValue("[EffectRack1_EffectUnit" + unitNo + "_Effect" + slot + "]",
            "meta", position);
    }
};

// FX ON 1 (note 86) / FX ON 2 (note 87): assign this deck to EffectUnit 1 / 2.
// SHIFT + FX ON 1 = quantize, SHIFT + FX ON 2 = move the beatgrid to the
// current play position.
MC3000.fxOn = function (channel, control, value, status, group) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    // The unit comes from the button (note 86 = FX1, note 87 = FX2) and the
    // deck is simply the one the message arrived on - which is what "group"
    // already holds. Do not second-guess either of them.
    var unitNo = (control === 86) ? 1 : 2;
    var target = group;

    if (MC3000.isShifted(channel)) {
        if (unitNo === 1) {
            MC3000.toggle(target, "quantize");
        } else {
            engine.setValue(target, "beats_translate_curpos", 1);
        }
        return;
    }
    MC3000.toggle("[EffectRack1_EffectUnit" + unitNo + "]", "group_" + target + "_enable");
};

// ============================================================================
//  SAMPLERS (EFX buttons / knobs while SAMP. MODE is on)
// ============================================================================

// Start a sampler: play from its cue point, or load the selected track if the
// slot is still empty.
MC3000.playSampler = function (number) {
    if (number < 1 || number > 8) {
        return;
    }
    var sampler = "[Sampler" + number + "]";
    if (engine.getValue(sampler, "track_loaded")) {
        engine.setValue(sampler, "cue_gotoandplay", 1);
    } else {
        engine.setValue(sampler, "LoadSelectedTrack", 1);
    }
};

MC3000.samplerButton = function (channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    var number = MC3000.samplerFromNote[control];
    if (number === undefined) {
        return;
    }
    var sampler = "[Sampler" + number + "]";

    if (MC3000.isShifted(channel)) {
        // SHIFT: stop a playing sampler, eject it when already stopped
        if (engine.getValue(sampler, "play")) {
            engine.setValue(sampler, "play", 0);
        } else {
            engine.setValue(sampler, "eject", 1);
            engine.setValue(sampler, "eject", 0);
        }
        return;
    }
    MC3000.playSampler(number);
};

MC3000.samplerGain = function (channel, control, value) {
    var number = MC3000.samplerFromCC[control];
    if (number === undefined) {
        return;
    }
    engine.setValue("[Sampler" + number + "]", "pregain", (value / 127.0) * 2.0);
};

// ============================================================================
//  MIXER
// ============================================================================

// Channel CUE (PFL) buttons. The note number identifies the deck on its own
// (1 = A, 2 = C, 5 = B, 7 = D) so this works on any MIDI channel.
MC3000.pflButton = function (channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    var deck;
    switch (control) {
    case 1: deck = 1; break;    // CUE DECK A
    case 2: deck = 3; break;    // CUE DECK C
    case 5: deck = 2; break;    // CUE DECK B
    case 7: deck = 4; break;    // CUE DECK D
    default: return;
    }
    MC3000.toggle("[Channel" + deck + "]", "pfl");
};

// ============================================================================
//  BROWSER / CENTRE SECTION
// ============================================================================

// TRACK SELECT knob, CC 84. The MC3000 sends 0 for one direction and 127 for
// the other (it is not a standard 64-centred encoder).
MC3000.trackKnob = function (channel, control, value) {
    var step = (value === 0) ? 1 : -1;
    if (MC3000.anyShift()) {
        step *= 5;                       // fast scroll
    }
    engine.setValue("[Library]", "MoveVertical", step);
};

// TRACK SELECT push, note 40. SHIFT = move focus between library panes.
MC3000.trackKnobPress = function (channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    if (MC3000.anyShift()) {
        engine.setValue("[Library]", "MoveFocusForward", 1);
    } else {
        engine.setValue("[Library]", "GoToItem", 1);
    }
};

// BACK (note 48) and FWD (note 41).
// Unshifted: move the focus between the library panes.
// SHIFT: load the selected track into the preview deck / play-pause it.
MC3000.backFwd = function (channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    var forward = (control === 41);

    if (MC3000.anyShift()) {
        if (forward) {
            MC3000.toggle("[PreviewDeck1]", "play");
        } else {
            engine.setValue("[PreviewDeck1]", "LoadSelectedTrackAndPlay", 1);
        }
        return;
    }
    engine.setValue("[Library]", forward ? "MoveFocusForward" : "MoveFocusBackward", 1);
};

// LOAD A/C (note 98) and LOAD B/D (note 99). SHIFT = eject that deck instead.
//
// (v3.2, corrected) Unlike the rest of the deck section, LOAD's own MIDI
// channel does NOT move with the layer - confirmed on real hardware on
// 2026-08-05: with the left side switched to deck C (deckOnSide[0] = 3,
// verified via the trace log), a LOAD A/C press still arrived on channel 0,
// not channel 1. A change earlier in the same v3.2 pass had switched this to
// MC3000.deckGroup(channel) on the assumption that LOAD behaves like every
// other control here - it does not, and that change was wrong; this reverts
// it back to reading the side from the note and the deck from the tracked
// focus, which change #9 (ignoring the spurious note 14/15 events) had
// already made reliable again by the time this was re-tested.
// Load the selected library track into whichever deck this side is showing,
// or eject it if SHIFT is held. Shared by LOAD A/C, LOAD B/D (below) and, as
// of v3.3, CHANNEL SELECT A/C, B/D (see deckButton()).
MC3000.loadOnSide = function (side) {
    var group = "[Channel" + MC3000.deckOnSide[side] + "]";
    if (MC3000.traceDeckButtons) {
        MC3000.log("loadOnSide side=" + side +
            " deckOnSide=[" + MC3000.deckOnSide + "] -> " + group);
    }

    if (MC3000.anyShift()) {
        // SHIFT = eject. Pulse the push control (press then release).
        engine.setValue(group, "eject", 1);
        engine.setValue(group, "eject", 0);
    } else {
        // LoadSelectedTrack is a binary PUSH control: it must be set back to 0
        // or Mixxx treats it as still held and ignores the next press. Leaving
        // it latched at 1 was why LOAD stopped working after the first use.
        engine.setValue(group, "LoadSelectedTrack", 1);
        engine.setValue(group, "LoadSelectedTrack", 0);
    }
};

MC3000.load = function (channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    MC3000.loadOnSide((control === 98) ? 0 : 1);
};

// BROWSE, note 100. SHIFT = show / hide the sampler bank.
MC3000.browse = function (channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    if (MC3000.anyShift()) {
        MC3000.toggleCompat("showCoverArt");
    } else {
        MC3000.toggleCompat("maximizeLibrary");
    }
};

// SAMPLE window button, note 76. Shows / hides the sampler bank.
MC3000.sampleWindow = function (channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    MC3000.toggleCompat("showSamplers");
};

// EFX window button, note 77. Shows / hides the effect rack.
MC3000.efxWindow = function (channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    MC3000.toggleCompat("showEffects");
};

// RECORD, note 101. SHIFT = enable / disable Auto DJ.
MC3000.record = function (channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    if (MC3000.anyShift()) {
        MC3000.toggle("[AutoDJ]", "enabled");
    } else {
        engine.setValue("[Recording]", "toggle_recording", 1);
    }
};

// CF MODE, note 27. Toggles split cue in the headphones.
MC3000.cfMode = function (channel, control, value, status) {
    if (!MC3000.isPress(value, status)) {
        return;
    }
    MC3000.toggle("[Master]", "headSplit");
};

// ============================================================================
//  LED CALLBACKS
// ============================================================================

MC3000.onPlay = function (value, group) {
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.play, value);
};

MC3000.onCue = function (value, group) {
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.cue, value);
};

MC3000.onKeylock = function (value, group) {
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.keylock, value);
};

MC3000.onSync = function (value, group) {
    var deck = MC3000.deckFromGroup(group);
    if (MC3000.endOfTrack[deck - 1]) {
        return;                          // the warning owns the LED right now
    }
    MC3000.setLed(deck, MC3000.led.sync, value);
};

MC3000.onPfl = function (value, group) {
    var deck = MC3000.deckFromGroup(group);
    MC3000.setLed2(deck, MC3000.pflLed[deck - 1], value);
};

MC3000.onLoopEnabled = function (value, group) {
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.autoLoop, value);
};

MC3000.onLoopStart = function (value, group) {
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.loopIn, value !== -1);
};

MC3000.onLoopEnd = function (value, group) {
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.loopOut, value !== -1);
};

MC3000.onHotcue = function (value, group, control) {
    var match = control.match(/^hotcue_(\d+)_(?:status|enabled)$/);
    if (match === null) {
        return;
    }
    var index = parseInt(match[1], 10) - 1;
    if (index < 0 || index > 7) {
        return;
    }
    MC3000.setLed(MC3000.deckFromGroup(group), MC3000.led.hotcue[index], value > 0);
};

// EFX.4 button LED = "this unit is going to the headphones"
MC3000.onFxHeadphone = function (value, group) {
    var match = group.match(/^\[EffectRack1_EffectUnit(\d)\]$/);
    if (match === null) {
        return;
    }
    var side = parseInt(match[1], 10) - 1;
    if (side < 0 || side > 1) {
        return;
    }
    MC3000.setLed(MC3000.deckOnSide[side], MC3000.led.efx[side][3], value > 0);
};

// EFX button LED = "this effect is switched on"
MC3000.onEffectEnabled = function (value, group) {
    var match = group.match(/^\[EffectRack1_EffectUnit(\d)_Effect(\d)\]$/);
    if (match === null) {
        return;
    }
    var side = parseInt(match[1], 10) - 1;
    var index = parseInt(match[2], 10) - 1;
    if (side < 0 || side > 1 || index < 0 || index > 3) {
        return;
    }
    MC3000.setLed(MC3000.deckOnSide[side], MC3000.led.efx[side][index], value > 0);
};

MC3000.onFxAssign = function (value, group, control) {
    var match = control.match(/^group_\[Channel(\d+)\]_enable$/);
    if (match === null) {
        return;
    }
    var deck = parseInt(match[1], 10);
    var unitNo = (group.indexOf("EffectUnit1") !== -1) ? 1 : 2;
    MC3000.setLed(deck, unitNo === 1 ? MC3000.led.fxOn1 : MC3000.led.fxOn2, value);
};

MC3000.onVuMeter = function (value, group) {
    var deck = MC3000.deckFromGroup(group);
    if (deck < 1) {
        return;
    }
    var lit = Math.round(value * 7);
    if (lit === MC3000.vuLedsLit[deck - 1]) {
        return;                          // nothing changed, do not flood the MIDI bus
    }
    var previous = MC3000.vuLedsLit[deck - 1];
    var base = MC3000.vuLedBase[deck - 1];
    var i;
    if (lit > previous) {
        for (i = Math.max(previous, 0); i < lit; i++) {
            MC3000.setLed2(deck, base + i, true);
        }
    } else {
        for (i = Math.max(lit, 0); i < previous; i++) {
            MC3000.setLed2(deck, base + i, false);
        }
    }
    MC3000.vuLedsLit[deck - 1] = lit;
};

MC3000.onSampler = function (value, group) {
    var match = group.match(/^\[Sampler(\d+)\]$/);
    if (match === null) {
        return;
    }
    var number = parseInt(match[1], 10);
    if (number < 1 || number > 8) {
        return;
    }
    var side = (number <= 4) ? 0 : 1;
    var deck = MC3000.deckOnSide[side];
    MC3000.setLed(deck, MC3000.led.samp[side][(number - 1) % 4], value);
};

MC3000.onPlayPosition = function (value, group) {
    var deck = MC3000.deckFromGroup(group);
    if (deck < 1) {
        return;
    }
    var warn = (value > MC3000.endOfTrackWarningPosition) && (value < 1.0);
    if (warn === MC3000.endOfTrack[deck - 1]) {
        return;
    }
    MC3000.endOfTrack[deck - 1] = warn;
    if (warn) {
        MC3000.setLed(deck, MC3000.led.sync, 2);          // blink
    } else {
        MC3000.setLed(deck, MC3000.led.sync, engine.getValue(group, "sync_enabled"));
    }
};

// ****************************************************************************
//  CONTROL REFERENCE
// ****************************************************************************
//
//  DECK SECTION (works identically on decks A, B, C and D)
//  ---------------------------------------------------------------------------
//  Control            Normal                       + SHIFT (same side)
//  ---------------------------------------------------------------------------
//  PLAY / PAUSE       play / pause                 reverse roll (censor)
//  CUE                cue (Mixxx cue behaviour)    jump to start of track
//  SYNC               sync lock on / off           become sync leader
//                     (hold 1 s = momentary sync)
//  KEY LOCK           key lock on / off            tap BPM
//  VINYL MODE         scratch <-> bend             lock the jog wheel (blinks)
//  PITCH BEND - / +   temporary tempo nudge        rewind / fast forward
//  JOG (touch top)    scratch                      -
//  JOG (side)         pitch bend                   scrub through the track
//  PITCH FADER        tempo (soft takeover)        -
//  CUE 1 - 4          hot cue 1 - 4                delete that hot cue
//  CUE 5 - 8 mode on  hot cue 5 - 8                delete that hot cue
//  LOOP IN            set loop in                  clear the loop
//  LOOP OUT           set loop out                 clear the loop
//  AUTO LOOP          loop on / off                reloop (re-enter last loop)
//  AUTO LOOP -        halve loop length            move loop 1 beat back
//  AUTO LOOP +        double loop length           move loop 1 beat forward
//  EFX 1 / 2 / 3      effect 1/2/3 on-off          EQ low / mid / high kill
//  EFX 4              unit -> headphones            filter (QuickEffect) kill
//  FX ON 1            route deck to EffectUnit1    quantize on / off
//  FX ON 2            route deck to EffectUnit2    move beatgrid to playhead
//
//  EFX KNOBS (SAMP. MODE off)
//  ---------------------------------------------------------------------------
//  EFX 1 knob         effect unit dry / wet        effect unit super knob
//  EFX 2 / 3 / 4 knob metaknob of effect 1 / 2 / 3 -
//  The left EFX section drives EffectUnit1, the right one drives EffectUnit2.
//
//  SAMP. MODE on
//  ---------------------------------------------------------------------------
//  SAMP 1 - 4 (left)  play sampler 1 - 4           stop, then eject
//  SAMP 1 - 4 (right) play sampler 5 - 8           stop, then eject
//  (pressing an empty sampler loads the selected library track into it)
//  SAMP knobs         sampler gain                 -
//
//  MIXER
//  ---------------------------------------------------------------------------
//  LEVEL / HI / MID / LOW / FILTER / channel fader / crossfader  -> direct
//  CUE (channel)      headphone cue on / off
//
//  BROWSER / CENTRE
//  ---------------------------------------------------------------------------
//  TRACK SELECT turn  move up / down in library    scroll 5 rows at a time
//  TRACK SELECT push  open folder / choose item    move focus between panes
//  BACK               focus previous pane          load track in preview deck
//  FWD                focus next pane              preview deck play / pause
//  DECK CHG A/B/C/D   select that deck outright - moves the jog wheel, play,
//                     cue, EQ, pitch, everything (this is the ONLY button
//                     that does that - see deckGroup() and deckButton())
//  DECK A/C, DECK B/D recall the other deck on that side (A<->C, B<->D) for
//                     LOAD to target - see MC3000.deckButton(), v3.5
//  LOAD A/C           load into the left side's    eject that deck
//                     current deck (see DECK A/C)
//  LOAD B/D           load into the right side's    eject that deck
//                     current deck (see DECK B/D)
//  BROWSE             maximise library             library cover art on / off
//  SAMPLE             show / hide samplers
//  EFX                show / hide effect rack
//  RECORD             start / stop recording       Auto DJ on / off
//  CF MODE            split cue (headphones) on / off
//
// ****************************************************************************
