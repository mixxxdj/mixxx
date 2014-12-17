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
    QString resetToDefault = tr("Reset to default value.");
    QString leftClick = tr("Left-click");
    QString rightClick = tr("Right-click");
    QString scrollWheel = tr("Scroll-wheel");

    add("waveform_overview")
            << tr("Waveform Overview")
            << tr("Shows information about the track currently loaded in this channel.")
            << tr("Jump around in the track by clicking somewhere on the waveform.")
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
            << dropTracksHere
            << tr("If Vinyl control is enabled, displays time-coded vinyl signal quality (see Preferences -> Vinyl Control).");

    add("pregain")
            << tr("Gain")
            << tr("Adjusts the pre-fader gain of the track (to avoid clipping).")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    QString clippingHelp = tr("(too loud for the hardware and is being distorted).");
    add("PeakIndicator")
            << tr("Peak Indicator")
            << tr("Indicates when the signal on the channel is clipping,")
            << clippingHelp;

    add("master_PeakIndicator")
            << tr("Master Peak Indicator")
            << tr("Indicates when the signal on the master output is clipping,")
            << clippingHelp;

    add("channel_VuMeter")
            << tr("Channel Volume Meter")
            << tr("Shows the current channel volume.");

    add("microphone_VuMeter")
            << tr("Microphone Volume Meter")
            << tr("Shows the current microphone volume.");

    add("master_VuMeterL")
            << tr("Master Channel Volume Meter")
            << tr("Shows the current master volume for the left channel.");

    add("master_VuMeterR")
            << tr("Master Channel Volume Meter")
            << tr("Shows the current master volume for the right channel.");

    add("channel_volume")
            << tr("Volume Control")
            << tr("Adjusts the volume of the selected channel.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    // Legacy control.
    add("master_volume")
            << tr("Master Volume")
            << tr("Adjusts the master output volume.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("master_gain")
            << tr("Master Gain")
            << tr("Adjusts the master output gain.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("crossfader")
            << tr("Crossfader")
            << tr("Determines the master output by fading between the left and right channels.")
            << QString("%1: %2").arg(rightClick, resetToDefault)
            << tr("Change the crossfader curve in Preferences -> Crossfader");

    add("balance")
            << tr("Balance")
            << tr("Adjusts the left/right channel balance on the master output.")
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
            << tr("Controls what you hear on the headphone output.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("headSplit")
            << tr("Headphone Split Cue")
            << tr("If activated, the master signal plays in the right channel, while the cueing signal ")
            << tr("plays in the left channel.")
            << tr("Adjust the Headphone Mix so in the left channel is not the pure cueing signal.");
    
    // Note, this is used for samplers and microphone only currently (that's why
    // center is the default).
    add("orientation")
            << tr("Mix Orientation")
            << tr("Set the channel's mix orientation.")
            << tr("Either to the left side of crossfader, to the right side or to the center (default).");

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

    add("toggle_4decks")
            << tr("Toggle 4 Decks")
            << tr("Switches between showing 2 decks and 4 decks.");

    add("show_library")
            << tr("Show Library")
            << tr("Show or hide the track library.");

    add("show_effects")
            << tr("Show Effects Rack")
            << tr("Show or hide the effects rack.");

    add("toggle_expanded_library")
            << tr("Toggle Big Library")
            << tr("Makes the library fill the screen.");

    add("show_mixer")
            << tr("Toggle Mixer")
            << tr("Show or hide the mixer.");

    add("microphone_volume")
            << tr("Microphone Volume")
            << tr("Adjusts the microphone volume.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("microphone_pregain")
            << tr("Microphone Gain")
            << tr("Adjusts the pre-fader microphone gain.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("microphone_talkover")
            << tr("Microphone Talk-Over")
            << tr("Hold-to-talk or short click for latching to")
            << tr("mix microphone input into the master output.");

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

    //this is a special case, in some skins (e.g. Deere) we display a transparent png for bpm_tap on top of visual_bpm
    add("bpm_tap_visual_bpm")
            << tr("Tempo and BPM Tap")
            << tempoDisplay
            << tr("When tapped repeatedly, adjusts the BPM to match the tapped BPM.");

    add("show_spinny")
            << tr("Spinning Vinyl")
            << tr("Show/hide the spinning vinyl section.");

    add("beats_translate_curpos")
            << tr("Adjust Beatgrid")
            << QString("%1: %2").arg(leftClick, tr("Adjust beatgrid so the closest beat is aligned with the current play position."))
            << QString("%1: %2").arg(rightClick, tr("Adjust beatgrid to match another playing deck."));

    add("keylock")
            << tr("Key-Lock")
            << tr("Prevents the pitch from from changing when the rate changes.")
            << tr("Toggling key-lock during playback may result in a momentary audio glitch.");

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

    // Currently used for decks
    QString cueSet = tr("Places a cue-point at the current position on the waveform.");
    add("play_cue_set")
            << tr("Play/Pause")
            << QString("%1: %2").arg(leftClick, tr("Plays or pauses the track."))
            << QString("%1: %2").arg(rightClick, cueSet);

    QString whilePlaying = tr("(while playing)");
    QString whileStopped = tr("(while stopped)");
    add("cue_default_cue_gotoandstop")
            << tr("Cue")
            << QString("%1 %2: %3").arg(leftClick, whilePlaying, tr("Stops track at cue point."))
            << QString("%1 %2: %3").arg(leftClick, whileStopped, tr("Set cue point (Pioneer/Mixxx mode) OR preview from it (Denon mode)."))
            << tr("Hint: Change the default cue mode in Preferences -> Interface.")
            << quantizeSnap
            << QString("%1: %2").arg(rightClick, tr("Seeks the track to the cue-point and stops."));

    add("pfl")
            << tr("Headphone")
            << tr("Sends the selected channel's audio to the headphone output,")
            << tr("selected in Preferences -> Sound Hardware.");

    add("mute")
            << tr("Mute")
            << tr("Mutes the selected channel's audio in the master output.");

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
            << QString("%1: %2").arg(leftClick, tr("Syncs the tempo (BPM) and phase to that of the other track, "))
            << tr("if BPM is detected on both.")
            << QString("%1: %2").arg(rightClick, tr("Syncs the tempo (BPM) to that of the other track,"))
            << tr("if BPM is detected on both.")
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

    add("sync_master")
            << tr("Enable Sync Clock Master")
            << tr("When enabled, this device will serve as the master clock for all other decks.");

    add("rate")
            << tr("Speed Control")
            << tr("Changes the track playback speed (affects both the tempo and the pitch). If key-lock is enabled, only the tempo is affected.")
            << QString("%1: %2").arg(rightClick, resetToDefault);

    add("pitch")
            << tr("Pitch Control")
            << tr("Changes the track pitch independent of the tempo.")
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

    add("hotcue")
            << tr("Hotcue")
            << QString("%1: %2").arg(leftClick, tr("If hotcue is set, jumps to the hotcue."))
            << tr("If hotcue is not set, sets the hotcue to the current play position.")
            << quantizeSnap
            << QString("%1: %2").arg(rightClick, tr("If hotcue is set, clears the hotcue."));

    add("vinylcontrol_mode")
            << tr("Vinyl Control Mode")
            << tr("Absolute mode - track position equals needle position and speed.")
            << tr("Relative mode - track speed equals needle speed regardless of needle position.")
            << tr("Constant mode - track speed equals last known-steady speed regardless of needle input.");

    add("vinylcontrol_status")
            << tr("Vinyl Status")
            << tr("Provides visual feedback for vinyl control status:")
            << tr("Green for control enabled.")
            << tr("Blinking yellow for when the needle reaches the end of the record.");

    add("loop_in")
            << tr("Loop-In Marker")
            << tr("Sets the deck loop-in position to the current play position.")
            << quantizeSnap;

    add("loop_out")
            << tr("Loop-Out Marker")
            << tr("Sets the deck loop-out position to the current play position.")
            << quantizeSnap;

    add("loop_halve")
            << tr("Loop Halve")
            << tr("Halves the current loop's length by moving the end marker.")
            << tr("Deck immediately loops if past the new endpoint.");

    add("loop_double")
            << tr("Loop Double")
            << tr("Doubles the current loop's length by moving the end marker.");

    //beatloop and beatlooproll
    add("beatloop")
            << tr("Beatloop")
            << QString("%1: %2").arg(leftClick, tr("Setup a loop over the set number of beats."))
            << quantizeSnap
            << QString("%1: %2").arg(rightClick, tr("Temporarily setup a rolling loop over the set number of beats."))
            << tr("Playback will resume where the track would have been if it had not entered the loop.");

    add("beatjump")
            << tr("Beatjump")
            << QString("%1: %2").arg(leftClick, tr("Jump forward or backward by the set number of beats."));

    add("loop_move")
            << tr("Loop Move")
            << QString("%1: %2").arg(leftClick, tr("Adjust the loop in and out points by the set number of beats."));

    add("loop_exit")
            << tr("Loop Exit")
            << tr("Turns the current loop off.")
            << tr("Works only if Loop-In and Loop-Out marker are set.");

    add("reloop_exit")
            << tr("Reloop/Exit")
            << tr("Toggles the current loop on or off.")
            << tr("Works only if Loop-In and Loop-Out marker are set.");

    add("slip_mode")
            << tr("Slip Mode")
            << tr("When active, the playback continues muted in the background during a loop, reverse, scratch etc.")
            << tr("Once disabled, the audible playback will resume where the track would have been.");

    add("vinylcontrol_cueing")
            << tr("Vinyl Cueing Mode")
            << tr("Determines how cue points are treated in vinyl control Relative mode:")
            << tr("Off - Cue points ignored.")
            << tr("One Cue - If needle is dropped after the cue point, track will seek to that cue point.")
            << tr("Hot Cue - Track will seek to nearest previous hot cue point.");

    add("track_time")
            << tr("Track Time")
            << tr("Displays the elapsed or remaining time of the track loaded.")
            << tr("Click to toggle between time elapsed/remaining time.");

    add("track_duration")
            << tr("Track Duration")
            << tr("Displays the duration of the loaded track.");

    QString trackTags = tr("Information is loaded from the track's metadata tags.");
    add("track_artist")
            << tr("Track Artist")
            << tr("Displays the artist of the loaded track.")
            << trackTags
            << dropTracksHere;

    add("track_title")
            << tr("Track Title")
            << tr("Displays the title of the loaded track.")
            << trackTags
            << dropTracksHere;

    add("track_album")
            << tr("Track Album")
            << tr("Displays the album name of the loaded track.")
            << trackTags;

    add("track_key")
            //: The musical key of a track
            << tr("Track Key")
            << tr("Displays the musical key of the loaded track.")
            << trackTags;

    add("text")
            << tr("Track Artist/Title")
            << tr("Displays the artist and title of the loaded track.")
            << trackTags
            << dropTracksHere;

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
            << QString("%1: %2").arg(rightClick, tr("Displays options for editing cover artwork."));

    add("starrating")
            << tr("Star Rating")
            << tr("Assign ratings to individual tracks by clicking the stars.");
}
