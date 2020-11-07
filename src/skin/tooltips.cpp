#include "skin/tooltips.h"

Tooltips::Tooltips() {
    addStandardTooltips();
}

Tooltips::~Tooltips() {
}

QString Tooltips::tooltipForId(QString id) const {
    // We always add a separator at the end.
    QString joined = m_tooltips.value(id, QStringList()).join(tooltipSeparator());
    if (joined.length() > 0) {
        joined += tooltipSeparator();
    }
    return joined;
}

QString Tooltips::tooltipSeparator() const {
    return "\n";
}

QList<QString>& Tooltips::add(QString id) {
    return m_tooltips[id];
}

void Tooltips::addStandardTooltips() {
    QString dropTracksHere = tr("Drop tracks from library, external file manager, or other decks/samplers here.");
    QString dragItem = tr("Drag this item to other decks/samplers, to crates and playlist or to external file manager.");
    QString trackMenu = tr("Right-click to open the track context menu.");
    QString resetToDefault = tr("Reset to default value.");
    QString leftClick = tr("Left-click");
    QString rightClick = tr("Right-click");
    QString scrollWheel = tr("Scroll-wheel");
    QString shift = tr("Shift-key");
    QString loopActive = "(" + tr("loop active") + ")";
    QString loopInactive = "(" + tr("loop inactive") + ")";
    QString effectsWithinUnit = tr("Effects within the chain must be enabled to hear them.");

    add("waveform_overview")
            << tr("Waveform Overview")
            << tr("Shows information about the track currently loaded in this deck.") << "\n"
            << tr("Left click to jump around in the track.")
            << tr("Right click hotcues to edit their labels and colors.")
            << tr("Right click anywhere else to show the time at that point.")
            << dropTracksHere;

    QString scratchMouse = tr("Use the mouse to scratch, spin-back or throw tracks.");
    add("waveform_display")
            << tr("Waveform Display")
            << tr("Shows the loaded track's waveform near the playback position.")
            << QString("%1: %2").arg(leftClick, scratchMouse)
            << QString("%1: %2").arg(rightClick, tr("Drag with mouse to make temporary pitch adjustments."))
            << QString("%1: %2").arg(scrollWheel, tr("Scroll to change the waveform zoom level."))
            << dropTracksHere;

    add("waveform_zoom_up")
            << tr("Waveform Zoom Out");

    add("waveform_zoom_down")
            << tr("Waveform Zoom In");

    add("waveform_zoom_set_default")
            << tr("Waveform Zoom")
            << QString("%1").arg(resetToDefault);

    add("spinny")
            << tr("Spinning Vinyl")
            << tr("Rotates during playback and shows the position of a track.")
            << scratchMouse
            << tr("Right click to show cover art of loaded track.")
            << dropTracksHere
            << tr("If Vinyl control is enabled, displays time-coded vinyl signal quality (see Preferences -> Vinyl Control).");

    add("big_spinny_coverart")
            << tr("Big Spinny/Cover Art")
            << tr("Show a big version of the Spinny or track cover art if enabled.");

    add("pregain")
            << tr("Gain")
            << tr("Adjusts the pre-fader gain of the track (to avoid clipping).")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    QString clippingHelp = tr("(too loud for the hardware and is being distorted).");
    add("channel_PeakIndicator")
            << tr("Channel Peak Indicator")
            << tr("Indicates when the signal on the channel is clipping,")
            << clippingHelp;

    add("channel_PeakIndicatorL")
            << tr("Channel L Peak Indicator")
            << tr("Indicates when the left signal on the channel is clipping,")
            << clippingHelp;

    add("channel_PeakIndicatorR")
            << tr("Channel R Peak Indicator")
            << tr("Indicates when the right signal on the channel is clipping,")
            << clippingHelp;

    add("master_PeakIndicator")
            << tr("Main Output Peak Indicator")
            << tr("Indicates when the signal on the main output is clipping,")
            << clippingHelp;

    add("master_PeakIndicatorL")
            << tr("Main Output L Peak Indicator")
            << tr("Indicates when the left signal on the main output is clipping,")
            << clippingHelp;

    add("master_PeakIndicatorR")
            << tr("Main Output R Peak Indicator")
            << tr("Indicates when the right signal on the main output is clipping,")
            << clippingHelp;

    add("channel_VuMeter")
            << tr("Channel Volume Meter")
            << tr("Shows the current channel volume.");

    add("channel_VuMeterL")
            << tr("Channel L Volume Meter")
            << tr("Shows the current channel volume for the left channel.");

    add("channel_VuMeterR")
            << tr("Channel R Volume Meter")
            << tr("Shows the current channel volume for the right channel.");

    add("microphone_VuMeter")
            << tr("Microphone Volume Meter")
            << tr("Shows the current microphone volume.");

    add("microphone_PeakIndicator")
            << tr("Microphone Peak Indicator")
            << tr("Indicates when the signal on the microphone is clipping,")
            << clippingHelp;

    add("auxiliary_VuMeter")
            << tr("Auxiliary Volume Meter")
            << tr("Shows the current auxiliary volume.");

    add("auxiliary_PeakIndicator")
            << tr("Auxiliary Peak Indicator")
            << tr("Indicates when the signal on the auxiliary is clipping,")
            << clippingHelp;

    add("sampler_VuMeter")
            << tr("Sampler Volume Meter")
            << tr("Shows the current sampler volume.");

    add("sampler_PeakIndicator")
            << tr("Sampler Peak Indicator")
            << tr("Indicates when the signal on the sampler is clipping,")
            << clippingHelp;

    add("preview_VuMeter")
            << tr("Preview Deck Volume Meter")
            << tr("Shows the current Preview Deck volume.");

    add("preview_PeakIndicator")
            << tr("Preview Deck Peak Indicator")
            << tr("Indicates when the signal on the Preview Deck is clipping,")
            << clippingHelp;

    add("master_VuMeterL")
            << tr("Main Channel L Volume Meter")
            << tr("Shows the current volume for the left channel of the main output.");

    add("master_VuMeterR")
            << tr("Master Channel R Volume Meter")
            << tr("Shows the current volume for the right channel of the main output.");

    add("channel_volume")
            << tr("Volume Control")
            << tr("Adjusts the volume of the selected channel.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    // Legacy control.
    add("master_volume")
            << tr("Main Output Gain")
            << tr("Adjusts the main output gain.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("master_gain")
            << tr("Main Output Gain")
            << tr("Adjusts the main output gain.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("booth_gain")
            << tr("Booth Gain")
            << tr("Adjusts the booth output gain.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("crossfader")
            << tr("Crossfader")
            << tr("Determines the main output by fading between the left and right channels.")
            << QString("%1: %2").arg(rightClick, resetToDefault)
            << tr("Change the crossfader curve in Preferences -> Crossfader");

    add("balance")
            << tr("Balance")
            << tr("Adjusts the left/right channel balance on the main output.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    // Legacy control.
    add("headphone_volume")
            << tr("Headphone Volume")
            << tr("Adjusts the headphone output volume.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("headphone_gain")
            << tr("Headphone Gain")
            << tr("Adjusts the headphone output gain.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("headMix")
            << tr("Headphone Mix")
            << tr("Crossfades the headphone output between the main mix and cueing (PFL or Pre-Fader Listening) signal.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("headSplit")
            << tr("Headphone Split Cue")
            << tr("If activated, the main mix signal plays in the right channel, while the cueing signal "
                  "plays in the left channel.")
            << tr("Adjust the Headphone Mix so in the left channel is not the pure cueing signal.");

    add("orientation")
            << tr("Crossfader Orientation")
            << tr("Set the channel's crossfader orientation.")
            << tr("Either to the left side of crossfader, to the right side or to the center (unaffected by crossfader)");

    add("show_microphone")
            << tr("Microphone")
            << tr("Show/hide the Microphone section.");

    add("show_samplers")
            << tr("Sampler")
            << tr("Show/hide the Sampler section.");

    add("show_vinylcontrol")
            << tr("Vinyl Control")
            << tr("Show/hide the Vinyl Control section.")
            << tr("Activate Vinyl Control from the Menu -> Options.");

    add("show_previewdeck")
            << tr("Preview Deck")
            << tr("Show/hide the Preview deck.");

    add("show_coverart")
            << tr("Cover Art")
            << tr("Show/hide Cover Art.");

    add("show_library_coverart")
            << tr("Cover Art")
            << tr("Show/hide Cover Art of the selected track in the library.");

    add("toggle_4decks")
            << tr("Toggle 4 Decks")
            << tr("Switches between showing 2 decks and 4 decks.");

    add("show_waveforms")
            << tr("Show/hide the scrolling waveforms");

    add("show_beatgrid_controls")
            << tr("Show/hide the beatgrid controls section");

    add("show_library")
            << tr("Show Library")
            << tr("Show or hide the track library.");

    add("show_effects")
            << tr("Show Effects")
            << tr("Show or hide the effects.");

    add("maximize_library")
            << tr("Maximize Library")
            << tr("Hide all skin sections except the decks to have more screen space for the track library.");

    add("show_mixer")
            << tr("Toggle Mixer")
            << tr("Show or hide the mixer.");

    add("show_vumeters")
            << tr("Volume Meters")
            << tr("Show/hide volume meters for channels and master output.");

    add("microphone_volume")
            << tr("Microphone Volume")
            << tr("Adjusts the microphone volume.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("microphone_pregain")
            << tr("Microphone Gain")
            << tr("Adjusts the pre-fader microphone gain.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("auxiliary_pregain")
            << tr("Auxiliary Gain")
            << tr("Adjusts the pre-fader auxiliary gain.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("microphone_talkover")
            << tr("Microphone Talk-Over")
            << tr("Hold-to-talk or short click for latching to")
            << tr("mix microphone input into the main output.");

    add("talkover_duck_mode")
            << tr("Microphone Talkover Mode")
            << tr("Off: Do not reduce music volume")
            << tr("Auto: Automatically reduce music volume when microphone volume rises above threshold.")
            << tr("Manual: Reduce music volume by a fixed amount set by the Strength knob.")
            << tr("Adjust the amount the music volume is reduced with the Strength knob.");

    add("talkover_duck_strength")
            << tr("Microphone Talkover Ducking Strength")
            << tr("Adjust the amount the music volume is reduced with the Strength knob.")
            << tr("Behavior depends on Microphone Talkover Mode:")
            << tr("Off: Does nothing")
            << tr("Auto: Sets how much to reduce the music volume when the volume of active microphones rises above threshold.")
            << tr("Manual: Sets how much to reduce the music volume, when talkover is activated regardless of volume of microphone inputs.");

    QString changeAmount = tr("Change the step-size in the Preferences -> Interface menu.");
    add("rate_perm_up_rate_perm_up_small")
            << tr("Raise Pitch")
            << QString("%1: %2").arg(leftClick, tr("Sets the pitch higher."))
            << QString("%1: %2").arg(rightClick, tr("Sets the pitch higher in small steps."))
            << changeAmount;

    add("rate_perm_down_rate_perm_down_small")
            << tr("Lower Pitch")
            << QString("%1: %2").arg(leftClick, tr("Sets the pitch lower."))
            << QString("%1: %2").arg(rightClick, tr("Sets the pitch lower in small steps."))
            << changeAmount;

    add("rate_temp_up_rate_temp_up_small")
            << tr("Raise Pitch Temporary (Nudge)")
            << QString("%1: %2").arg(leftClick, tr("Holds the pitch higher while active."))
            << QString("%1: %2").arg(rightClick, tr("Holds the pitch higher (small amount) while active."))
            << changeAmount;

    add("rate_temp_down_rate_temp_down_small")
            << tr("Lower Pitch Temporary (Nudge)")
            << QString("%1: %2").arg(leftClick, tr("Holds the pitch lower while active."))
            << QString("%1: %2").arg(rightClick, tr("Holds the pitch lower (small amount) while active."))
            << changeAmount;

    add("filterLow")
            << tr("Low EQ")
            << tr("Adjusts the gain of the low EQ filter.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("filterMid")
            << tr("Mid EQ")
            << tr("Adjusts the gain of the mid EQ filter.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("filterHigh")
            << tr("High EQ")
            << tr("Adjusts the gain of the high EQ filter.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    QString eqKillLatch = tr("Hold-to-kill or short click for latching.");
    add("filterHighKill")
            << tr("High EQ Kill")
            << tr("Holds the gain of the high EQ to zero while active.")
            << eqKillLatch;

    add("filterMidKill")
            << tr("Mid EQ Kill")
            << tr("Holds the gain of the mid EQ to zero while active.")
            << eqKillLatch;

    add("filterLowKill")
            << tr("Low EQ Kill")
            << tr("Holds the gain of the low EQ to zero while active.")
            << eqKillLatch;

    QString tempoDisplay = tr("Displays the tempo of the loaded track in BPM (beats per minute).");
    add("visual_bpm")
            << tr("Tempo")
            << tempoDisplay;

    add("visual_key")
            //: The musical key of a track
            << tr("Key")
            << tr("Displays the current musical key of the loaded track after pitch shifting.");

    add("bpm_tap")
            << tr("BPM Tap")
            << tr("When tapped repeatedly, adjusts the BPM to match the tapped BPM.");

    add("beats_adjust_slower")
            << tr("Adjust BPM Down")
            << tr("When tapped, adjusts the average BPM down by a small amount.");

    add("beats_adjust_faster")
            << tr("Adjust BPM Up")
            << tr("When tapped, adjusts the average BPM up by a small amount.");

    add("beats_translate_earlier")
            << tr("Adjust Beats Earlier")
            << tr("When tapped, moves the beatgrid left by a small amount.");

    add("beats_translate_later")
            << tr("Adjust Beats Later")
            << tr("When tapped, moves the beatgrid right by a small amount.");

    add("beats_translate_curpos")
            << tr("Adjust Beatgrid")
            << QString("%1: %2").arg(leftClick, tr("Adjust beatgrid so the closest beat is aligned with the current play position."))
            << QString("%1: %2").arg(rightClick, tr("Adjust beatgrid to match another playing deck."));

    add("beats_translate_match_alignment")
            << tr("Adjust Beatgrid")
            << tr("Adjust beatgrid to match another playing deck.");

    //this is a special case, in some skins we might display a transparent png for bpm_tap on top of visual_bpm
    add("bpm_tap_visual_bpm")
            << tr("Tempo and BPM Tap")
            << tempoDisplay
            << tr("When tapped repeatedly, adjusts the BPM to match the tapped BPM.");

    add("shift_cues_earlier")
            << tr("Shift cues earlier")
            << tr("Shift cues imported from Serato or Rekordbox if they are slightly off time.")
            << tr("Left click: shift 10 milliseconds earlier")
            << tr("Right click: shift 1 millisecond earlier");

    add("shift_cues_later")
            << tr("Shift cues later")
            << tr("Shift cues imported from Serato or Rekordbox if they are slightly off time.")
            << tr("Left click: shift 10 milliseconds later")
            << tr("Right click: shift 1 millisecond later");

    add("show_spinny")
            << tr("Spinning Vinyl")
            << tr("Show/hide the spinning vinyl section.");

    add("keylock")
            << tr("Keylock")
            << tr("Prevents the pitch from changing when the rate changes.")
            << tr("Toggling keylock during playback may result in a momentary audio glitch.");

    add("hotcue_toggle")
        <<tr("Changes the number of hotcue buttons displayed in the deck");

    // Show Rate Control
    add("rate_toggle")
        <<tr("Toggle visibility of Rate Control");

    // Used in cue/hotcue/loop tooltips below.
    QString quantizeSnap = tr("If quantize is enabled, snaps to the nearest beat.");
    add("quantize")
            << tr("Quantize")
            << tr("Toggles quantization.")
            << tr("Loops and cues snap to the nearest beat when quantization is enabled.");

    // Reverse and reverseroll (censor)
    add("reverse")
    << tr("Reverse")
            << QString("%1: %2").arg(leftClick, tr("Reverses track playback during regular playback."))
            << QString("%1: %2").arg(rightClick, tr("Puts a track into reverse while being held (Censor)."))
            << tr("Playback continues where the track would have been if it had not been temporarily reversed.");

    // Currently used for samplers
    add("play_start")
            << tr("Play/Pause")
            << QString("%1: %2").arg(leftClick, tr("Starts playing from the beginning of the track."))
            << QString("%1: %2").arg(rightClick, tr("Jumps to the beginning of the track and stops."));

    QString whilePlaying = tr("(while playing)");
    QString whileStopped = tr("(while stopped)");
    QString cueSet = tr("Places a cue point at the current position on the waveform.");
    QString cueWhilePlaying = tr("Stops track at cue point, OR go to cue point and play after release (CUP mode).");
    QString cueWhileStopped = tr("Set cue point (Pioneer/Mixxx/Numark mode), set cue point and play after release (CUP mode) "
            "OR preview from it (Denon mode).");
    QString cueHint = tr("Hint: Change the default cue mode in Preferences -> Interface.");

    // Currently used for decks
    add("play_cue_set")
            << tr("Play/Pause")
            << QString("%1: %2").arg(leftClick, tr("Plays or pauses the track."))
            << QString("%1: %2").arg(rightClick, cueSet);

    // Currently used for minimal decks
    add("play_cue_default")
            << tr("Play/Pause")
            << QString("%1: %2").arg(leftClick, tr("Plays or pauses the track."))
            << QString("%1 %2: %3").arg(rightClick, whilePlaying, cueWhilePlaying)
            << QString("%1 %2: %3").arg(rightClick, whileStopped, cueWhileStopped)
            << cueHint
            << quantizeSnap;
    add("cue_default_cue_gotoandstop")
            << tr("Cue")
            << QString("%1 %2: %3").arg(leftClick, whilePlaying, cueWhilePlaying)
            << QString("%1 %2: %3").arg(leftClick, whileStopped, cueWhileStopped)
            << cueHint
            << quantizeSnap
            << QString("%1: %2").arg(rightClick, tr("Seeks the track to the cue point and stops."));
    add("cue_gotoandplay_cue_default")
            << tr("Play")
            << QString("%1: %2").arg(leftClick, tr("Plays track from the cue point."))
            << QString("%1 %2: %3").arg(rightClick, whilePlaying, cueWhilePlaying)
            << QString("%1 %2: %3").arg(rightClick, whileStopped, cueWhileStopped)
            << cueHint
            << quantizeSnap;

      add("pfl")
            << tr("Headphone")
            << tr("Sends the selected channel's audio to the headphone output, "
                  "selected in Preferences -> Sound Hardware.");

    add("mute")
            << tr("Mute")
            << tr("Mutes the selected channel's audio in the main output.");

    add("master_enable")
            << tr("Main mix enable")
            << tr("Hold or short click for latching to "
                  "mix this input into the main output.");

    add("back_start")
            << tr("Fast Rewind")
            << QString("%1: %2").arg(leftClick, tr("Fast rewind through the track."))
            << QString("%1: %2").arg(rightClick, tr("Jumps to the beginning of the track."));

    add("fwd_end")
            << tr("Fast Forward")
            << QString("%1: %2").arg(leftClick, tr("Fast forward through the track."))
            << QString("%1: %2").arg(rightClick, tr("Jumps to the end of the track."));

    // Ghetto-Sync (TM)
    add("beatsync_beatsync_tempo")
            << tr("Old Synchronize")
            << tr("(This skin should be updated to use Master Sync!)")
            << QString("%1: %2").arg(leftClick, tr("Syncs the tempo (BPM) and phase to that of the other track, "
                                                   "if BPM is detected on both."))
            << QString("%1: %2").arg(rightClick, tr("Syncs the tempo (BPM) to that of the other track, "
                                                    "if BPM is detected on both."))
            << tr("Syncs to the first deck (in numerical order) that is playing a track and has a BPM.")
            << tr("If no deck is playing, syncs to the first deck that has a BPM.")
            << tr("Decks can't sync to samplers and samplers can only sync to decks.");

    // Awesome-Sync (TM)
    add("sync_enabled")
            << tr("Enable Master Sync")
            << tr("Tap to sync the tempo to other playing tracks or the master clock.")
            << tr("Hold for at least a second to enable sync lock for this deck.")
            << tr("Decks with sync locked will all play at the same tempo, and decks that also have "
                  "quantize enabled will always have their beats lined up.");

    // TODO(owen): find a better phrase for "the other deck"
    add("sync_reset_key")
            << tr("Sync and Reset Key")
            << QString("%1: %2").arg(leftClick, tr("Sets the pitch to a key that allows a harmonic transition "
                                                   "from the other track. Requires a detected key on both involved decks."))
            << QString("%1: %2").arg(rightClick, tr("Resets the key to the original track key."));

    add("sync_master")
            << tr("Enable Sync Clock Master")
            << tr("When enabled, this device will serve as the master clock for all other decks.");

    add("rate")
            << tr("Speed Control")
            << tr("Changes the track playback speed (affects both the tempo and the pitch). If keylock is enabled, only the tempo is affected.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("pitch")
            << tr("Pitch Control")
            << tr("Changes the track pitch independent of the tempo.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("pitch_up")
            << tr("Pitch Control")
            << tr("Changes the track pitch independent of the tempo.")
            << QString("%1: %2").arg(leftClick, tr("Increases the pitch by one semitone."))
            << QString("%1: %2").arg(rightClick, tr("Increases the pitch by 10 cents."));

    add("pitch_down")
            << tr("Pitch Control")
            << tr("Changes the track pitch independent of the tempo.")
            << QString("%1: %2").arg(leftClick, tr("Decreases the pitch by one semitone."))
            << QString("%1: %2").arg(rightClick, tr("Decreases the pitch by 10 cents."));

    add("pitch_adjust")
            << tr("Pitch Adjust")
            << tr("Adjust the pitch in addition to the speed slider pitch.")
            << QString("%1: %2").arg(rightClick, resetToDefault);


    add("rate_display")
            << tr("Pitch Rate")
            << tr("Displays the current playback rate of the track.");

    add("repeat")
            << tr("Repeat")
            << tr("When active the track will repeat if you go past the end or reverse before the start.");

    add("eject")
            << tr("Eject")
            << tr("Ejects track from the player.");

    add("hotcue") << tr("Hotcue")
                  << QString("%1: %2").arg(leftClick,
                             tr("If hotcue is set, jumps to the hotcue."))
                  << tr("If hotcue is not set, sets the hotcue to the current "
                        "play position.")
                  << quantizeSnap
                  << QString("%1: %2").arg(rightClick,
                             tr("Opens a menu to clear hotcues or edit their "
                                "labels and colors."))
                  << QString("%1 + %2: %3")
                             .arg(rightClick,
                                     shift,
                                     tr("Delete selected hotcue."));

    // Status displays and toggle buttons
    add("toggle_recording")
            << tr("Record Mix")
            << tr("Toggle mix recording.");

    // Status displays and toggle buttons
    add("recording_duration")
            << tr("Recording Duration")
            << tr("Displays the duration of the running recording.");

    // For legacy reasons also add tooltips for "shoutcast_enabled".
    for (const char* key : {"shoutcast_enabled", "broadcast_enabled"}) {
        add(key)
                << tr("Enable Live Broadcasting")
                << tr("Stream your mix over the Internet.")
                << tr("Provides visual feedback for Live Broadcasting status:")
                << tr("disabled, connecting, connected, failure.");
    }

    // AutoDJ status indicator
    add("autodj_status")
            << tr("AutoDJ is active");

    add("passthrough_enabled")
            << tr("Enable Passthrough")
            << tr("When enabled, the deck directly plays the audio arriving on the vinyl input.");

    add("vinylcontrol_enabled")
            << tr("Enable Vinyl Control")
            << tr("When disabled, the track is controlled by Mixxx playback controls.")
            << tr("When enabled, the track responds to external vinyl control.");

    add("vinylcontrol_status")
            << tr("Vinyl Status")
            << tr("Provides visual feedback for vinyl control status:")
            << tr("Green for control enabled.")
            << tr("Blinking yellow for when the needle reaches the end of the record.")
            << tr("Blue for passthrough enabled.");

    add("vinylcontrol_mode")
            << tr("Vinyl Control Mode")
            << tr("Absolute mode - track position equals needle position and speed.")
            << tr("Relative mode - track speed equals needle speed regardless of needle position.")
            << tr("Constant mode - track speed equals last known-steady speed regardless of needle input.");

    add("vinylcontrol_cueing")
            << tr("Vinyl Cueing Mode")
            << tr("Determines how cue points are treated in vinyl control Relative mode:")
            << tr("Off - Cue points ignored.")
            << tr("One Cue - If needle is dropped after the cue point, track will seek to that cue point.")
            << tr("Hot Cue - Track will seek to nearest previous hotcue point.");

    add("loop_in")
            << tr("Loop-In Marker")
            << QString("%1: %2").arg(leftClick + " " + loopInactive,
                      tr("Sets the track Loop-In Marker to the current play position."))
            << quantizeSnap
            << QString("%1: %2").arg(leftClick + " " + loopActive,
                      tr("Press and hold to move Loop-In Marker."))
            << QString("%1: %2").arg(rightClick, tr("Jump to Loop-In Marker."));

    add("loop_out")
            << tr("Loop-Out Marker")
            << QString("%1: %2").arg(leftClick + " " + loopInactive,
                      tr("Sets the track Loop-Out Marker to the current play position."))
            << quantizeSnap
            << QString("%1: %2").arg(leftClick + " " + loopActive,
                      tr("Press and hold to move Loop-Out Marker."))
            << QString("%1: %2").arg(rightClick, tr("Jump to Loop-Out Marker."));

    add("loop_halve")
            << tr("Loop Halve")
            << tr("Halves the current loop's length by moving the end marker.")
            << tr("Deck immediately loops if past the new endpoint.");

    add("loop_double")
            << tr("Loop Double")
            << tr("Doubles the current loop's length by moving the end marker.");

    add("beatloop_size")
            << tr("Beatloop Size")
            << tr("Select the size of the loop in beats to set with the Beatloop button.")
            << tr("Changing this resizes the loop if the loop already matches this size.");

    add("beatloop_halve")
            << tr("Halve the size of an existing beatloop, or halve the size of the next beatloop set with the Beatloop button.");

    add("beatloop_double")
            << tr("Double the size of an existing beatloop, or double the size of the next beatloop set with the Beatloop button.");

    //beatloop and beatlooproll
    add("beatloop_activate")
            << tr("Beatloop")
            << QString("%1: %2").arg(leftClick, tr("Start a loop over the set number of beats."))
            << quantizeSnap
            << QString("%1: %2").arg(rightClick, tr("Temporarily enable a rolling loop over the set number of beats."))
            << tr("Playback will resume where the track would have been if it had not entered the loop.");

    add("beatjump_size")
            << tr("Beatjump/Loop Move Size")
            << tr("Select the number of beats to jump or move the loop with the Beatjump Forward/Backward buttons.");

    add("beatjump_forward")
            << tr("Beatjump Forward")
            << QString("%1: %2").arg(leftClick + " " + loopInactive, tr("Jump forward by the set number of beats."))
            << QString("%1: %2").arg(leftClick + " " + loopActive, tr("Move the loop forward by the set number of beats."))
            << QString("%1: %2").arg(rightClick + " " + loopInactive, tr("Jump forward by 1 beat."))
            << QString("%1: %2").arg(rightClick + " " + loopActive, tr("Move the loop forward by 1 beat."));

    add("beatjump_backward")
            << tr("Beatjump Backward")
            << QString("%1: %2").arg(leftClick + " " + loopInactive, tr("Jump backward by the set number of beats."))
            << QString("%1: %2").arg(leftClick + " " + loopActive, tr("Move the loop backward by the set number of beats."))
            << QString("%1: %2").arg(rightClick + " " + loopInactive, tr("Jump backward by 1 beat."))
            << QString("%1: %2").arg(rightClick + " " + loopActive, tr("Move the loop backward by 1 beat."));

    add("loop_exit")
            << tr("Loop Exit")
            << tr("Turns the current loop off.")
            << tr("Works only if Loop-In and Loop-Out marker are set.");

    add("reloop_toggle")
            << tr("Reloop")
            << QString("%1: %2").arg(leftClick, tr("Toggles the current loop on or off."))
            << tr("If the loop is ahead of the current position, looping will start when the loop is reached.")
            << tr("Works only if Loop-In and Loop-Out Marker are set.")
            << QString("%1: %2").arg(rightClick, tr("Enable loop, jump to Loop-In Marker, and stop playback."));

    add("slip_mode")
            << tr("Slip Mode")
            << tr("When active, the playback continues muted in the background during a loop, reverse, scratch etc.")
            << tr("Once disabled, the audible playback will resume where the track would have been.");

    add("track_time")
            << tr("Track Time")
            << tr("Displays the elapsed and/or remaining time of the track loaded.")
            << tr("Click to toggle between time elapsed/remaining time/both.")
            << tr("Hint: Change the time format in Preferences -> Decks.");

    add("track_duration")
            << tr("Track Duration")
            << tr("Displays the duration of the loaded track.");

    QString trackTags = tr("Information is loaded from the track's metadata tags.");
    add("track_artist")
            << tr("Track Artist")
            << tr("Displays the artist of the loaded track.")
            << trackTags
            << dropTracksHere
            << dragItem
            << trackMenu;

    add("track_title")
            << tr("Track Title")
            << tr("Displays the title of the loaded track.")
            << trackTags
            << dropTracksHere
            << dragItem
            << trackMenu;

    add("track_album")
            << tr("Track Album")
            << tr("Displays the album name of the loaded track.")
            << trackTags
            << dropTracksHere
            << dragItem
            << trackMenu;

    add("track_key")
            //: The musical key of a track
            << tr("Track Key")
            << tr("Displays the musical key of the loaded track.")
            << trackTags;

    add("text")
            << tr("Track Artist/Title")
            << tr("Displays the artist and title of the loaded track.")
            << trackTags
            << dropTracksHere
            << dragItem;

    add("time")
            << tr("Clock")
            << tr("Displays the current time.");

    add("audio_latency_usage")
            << tr("Audio Latency Usage Meter")
            << tr("Displays the fraction of latency used for audio processing.")
            << tr("A high value indicates that audible glitches are likely.")
            << tr("Do not enable keylock, effects or additional decks in this situation.");

    add("audio_latency_overload")
            << tr("Audio Latency Overload Indicator")
            << tr("Indicates that the audio buffer is too small to do all audio processing.");

    add("coverart")
            << tr("Cover Art")
            << tr("Displays cover artwork of the loaded track.")
            << QString("%1: %2").arg(rightClick, tr("Displays options for editing cover artwork."))
            << dropTracksHere
            << dragItem;

    add("starrating")
            << tr("Star Rating")
            << tr("Assign ratings to individual tracks by clicking the stars.");

    // Intro & outro cues
    add("show_intro_outro_cues")
            << tr("Show/hide intro & outro markers and associated buttons.");

    add("intro_start")
            << tr("Intro Start Marker")
            << QString("%1: %2").arg(leftClick, tr("If marker is set, jumps to the marker."))
            << tr("If marker is not set, sets the marker to the current play position.")
            << quantizeSnap
            << QString("%1: %2").arg(rightClick, tr("If marker is set, clears the marker."));

    add("intro_end")
            << tr("Intro End Marker")
            << QString("%1: %2").arg(leftClick, tr("If marker is set, jumps to the marker."))
            << tr("If marker is not set, sets the marker to the current play position.")
            << quantizeSnap
            << QString("%1: %2").arg(rightClick, tr("If marker is set, clears the marker."));

    add("outro_start")
            << tr("Outro Start Marker")
            << QString("%1: %2").arg(leftClick, tr("If marker is set, jumps to the marker."))
            << tr("If marker is not set, sets the marker to the current play position.")
            << quantizeSnap
            << QString("%1: %2").arg(rightClick, tr("If marker is set, clears the marker."));

    add("outro_end")
            << tr("Outro End Marker")
            << QString("%1: %2").arg(leftClick, tr("If marker is set, jumps to the marker."))
            << tr("If marker is not set, sets the marker to the current play position.")
            << quantizeSnap
            << QString("%1: %2").arg(rightClick, tr("If marker is set, clears the marker."));

    // Effect Unit Controls
    add("EffectUnit_clear")
            << tr("Clear Unit")
            << tr("Clear effect unit.");

    add("EffectUnit_show_parameters")
            << tr("Show Effect Parameters")
            << tr("Show/hide parameters for effects in this unit.");

    add("EffectUnit_enabled")
            << tr("Toggle Unit")
            << tr("Enable or disable this whole effect unit.");
    add("EffectUnit_mix")
            << tr("Mix")
            << tr("Adjust the mixing of the dry (input) signal with the wet (output) signal of the effect unit")
            << tr("D/W mode: Crossfade between dry and wet")
            << tr("D+W mode: Add wet to dry")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("EffectUnit_mix_mode")
            << tr("Mix Mode")
            << tr("Adjust how the dry (input) signal is mixed with the wet (output) signal of the effect unit") + "\n"
            << tr("Dry/Wet mode (crossed lines): Mix knob crossfades between dry and wet\n"
                  "Use this to change the sound of the track with EQ and filter effects.") + "\n"
            << tr("Dry+Wet mode (flat dry line): Mix knob adds wet to dry\n"
                  "Use this to change only the effected (wet) signal with EQ and filter effects.");

    add("EffectUnit_super1")
            << tr("Super Knob")
            << tr("Controls the Meta Knob of all effects in this unit together.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("EffectUnit_next_chain")
            << tr("Next Chain")
            << tr("Load next effect chain preset into this effect unit.");

    add("EffectUnit_prev_chain")
            << tr("Previous Chain")
            << tr("Load previous effect chain preset into this effect unit.");

    add("EffectUnit_chain_selector")
            << tr("Next/Previous Chain")
            << tr("Load next or previous effect chain preset into this effect unit.");

    add("EffectUnit_group_enabled")
            << tr("Assign Effect Unit")
            << tr("Assign this effect unit to the channel output.")
            << effectsWithinUnit;

    add("EffectUnit_headphones_enabled")
            << tr("Assign Effect Unit")
            << tr("Route the headphone channel through this effect unit.")
            << effectsWithinUnit;

    add("EffectUnit_master_enabled")
            << tr("Assign Effect Unit")
            << tr("Route the main mix through this effect unit.")
            << effectsWithinUnit;

    add("EffectUnit_BusLeft_enabled")
            << tr("Assign Effect Unit")
            << tr("Route the left crossfader bus through this effect unit.")
            << effectsWithinUnit;

    add("EffectUnit_BusRight_enabled")
            << tr("Assign Effect Unit")
            << tr("Route the right crossfader bus through this effect unit.")
            << effectsWithinUnit;

    add("EffectUnit_deck_enabled")
            << tr("Assign Effect Unit")
            << tr("Route this deck through the indicated effect unit.")
            << effectsWithinUnit;

    add("EffectUnit_sampler_enabled")
            << tr("Assign Effect Unit")
            << tr("Route this sampler through the indicated effect unit.")
            << effectsWithinUnit;

    add("EffectUnit_microphone_enabled")
            << tr("Assign Effect Unit")
            << tr("Route this microphone through the indicated effect unit.")
            << effectsWithinUnit;

    add("EffectUnit_auxiliary_enabled")
            << tr("Assign Effect Unit")
            << tr("Route this auxiliary input through the indicated effect unit.")
            << effectsWithinUnit;

    // Effect Slot Controls
    add("EffectSlot_clear")
            << tr("Clear")
            << tr("Clear the current effect.");

    add("EffectSlot_enabled")
            << tr("Enable Effect")
            << tr("The effect unit must also be assigned to a deck or other sound source to hear the effect.");

    add("EffectSlot_next_effect")
            << tr("Next")
            << tr("Switch to the next effect.");

    add("EffectSlot_prev_effect")
            << tr("Previous")
            << tr("Switch to the previous effect.");

    add("EffectSlot_effect_selector")
            << tr("Next or Previous")
            << tr("Switch to either the next or previous effect.");

    add("EffectSlot_metaknob")
            << tr("Meta Knob")
            << tr("Controls linked parameters of this effect")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("EffectSlot_focus")
            << tr("Effect Focus Button")
            << QString("%1: %2").arg(leftClick, tr("Focuses this effect."))
            << QString("%1: %2").arg(rightClick, tr("Unfocuses this effect."))
            << tr("Refer to the web page on the Mixxx wiki for your controller for more information.");

    add("EffectSlot_parameter")
            << tr("Effect Parameter")
            << tr("Adjusts a parameter of the effect.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("EffectSlot_parameter_link_type")
            << tr("Meta Knob Link")
            << tr("Set how this parameter is linked to the effect's Meta Knob.")
            << tr("Inactive: parameter not linked")
            << tr("Active: parameter moves with Meta Knob")
            << tr("Left side active: parameter moves with left half of Meta Knob turn")
            << tr("Right side active: parameter moves with right half of Meta Knob turn")
            << tr("Left and right side active: parameter moves across range with half of Meta Knob turn and back with the other half");

    add("EffectSlot_parameter_inversion")
            << tr("Meta Knob Link Inversion")
            << tr("Inverts the direction this parameter moves when turning the effect's Meta Knob.");

    add("EffectSlot_button_parameter")
            << tr("Equalizer Parameter Kill")
            << tr("Holds the gain of the EQ to zero while active.")
            << eqKillLatch;

    // Quick Effect Rack Controls
    add("QuickEffectRack_super1")
            << tr("Quick Effect Super Knob")
            << tr("Quick Effect Super Knob (control linked effect parameters).")
            << QString("%1: %2").arg(rightClick, resetToDefault)
            << tr("Hint: Change the default Quick Effect mode in Preferences -> Equalizers.");

    add("QuickEffectRack_enabled")
            << tr("Toggle")
            << tr("Toggle the current effect.")
            << eqKillLatch;

    // Equalizer Rack Controls
    add("EqualizerRack_effect_parameter")
            << tr("Equalizer Parameter")
            << tr("Adjusts the gain of the EQ filter.")
            << QString("%1: %2").arg(rightClick, resetToDefault)
            << tr("Hint: Change the default EQ mode in Preferences -> Equalizers.");

    add("EqualizerRack_effect_button_parameter")
            << tr("Equalizer Parameter Kill")
            << tr("Holds the gain of the EQ to zero while active.")
            << eqKillLatch;

    add("skin_settings")
            << tr("Skin Settings Menu")
            << tr("Show/hide skin settings menu");

    // Sampler Bank Controls
    add("SaveSamplerBank")
            << tr("Save Sampler Bank")
            << tr("Save the collection of samples loaded in the samplers.");

    add("LoadSamplerBank")
            << tr("Load Sampler Bank")
            << tr("Load a previously saved collection of samples into the samplers.");

    add("configure_input")
            << tr("Select and configure a hardware device for this input");

}
