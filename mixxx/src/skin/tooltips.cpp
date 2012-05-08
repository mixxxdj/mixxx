
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
    add("waveform_overview")
            << tr("Waveform overview")
            << tr("Shows information about the track currently loaded in this channel.")
            << tr("Jump around in the track by clicking somewhere on the waveform.")
            << tr("Drop tracks from library or external file manager here.");

    add("waveform_display")
            << tr("Waveform display")
            << tr("Shows the loaded tracks' waveforms near the playback position.")
            << tr("Left-click: Use the mouse to scratch, halt, spin back and push forward a track.")
            << tr("Right-click: Drag with mouse to make temporary pitch adjustments.")
            << tr("Drop tracks from library or external file manager here.");

    add("spinny")
            << tr("Spinning vinyl")
            << tr("Rotates during playback and shows the position of a track.")
            << tr("Use the mouse to scratch, halt, spin back and push forward a track.")
            << tr("Drop tracks from library or external file manager here.")
            << tr("If Vinyl control is enabled, it can display the time-coded vinyls signal quality (see Preferences→Vinyl Control).");

    add("pregain")
            << tr("Gain")
            << tr("Adjusts the pre-fader gain of the track (to avoid clipping)")
            << tr("Right-click: Reset to default value");

    add("PeakIndicator")
            << tr("Peak Indicator")
            << tr("Indicates when the signal on the channel is clipping,")
            << tr("(too loud for the hardware and is being distorted).");

    add("master_PeakIndicator")
            << tr("Master Peak Indicator")
            << tr("Indicates when the signal on the Master output is clipping,")
            << tr("(too loud for the hardware and is being distorted).");

    add("channel_VuMeter")
            << tr("Channel volume meter")
            << tr("Shows the current channel volume");

    add("microphone_VuMeter")
            << tr("Microphone volume meter")
            << tr("Outputs the current instantaneous microphone volume");

    add("master_VuMeterL")
            << tr("Master channel volume meter")
            << tr("Outputs the current instantaneous master volume for the left channel.");

    add("master_VuMeterR")
            << tr("Master channel volume meter")
            << tr("Outputs the current instantaneous master volume for the right channel.");

    add("channel_volume")
            << tr("Volume control")
            << tr("Adjust the volume of the selected channel.")
            << tr("Right-click: Reset to default value");

    add("master_volume")
            << tr("Master volume")
            << tr("Adjusts the Master output volume.")
            << tr("Right-click: Reset to default value");

    add("crossfader")
            << tr("Crossfader")
            << tr("Fade between the channels and define what you hear through the master output.")
            << tr("Right-click: Reset to default value")
            << tr("Change the crossfader curve in Preferences→Crossfader");

    add("balance")
            << tr("Balance")
            << tr("Adjusts the left/right channel balance on the Master output.")
            << tr("Right-click: Reset to default value");

    add("headphone_volume")
            << tr("Headphone volume")
            << tr("Adjusts the headphone output volume.")
            << tr("Right-click: Reset to default value");

    add("headMix")
            << tr("Headphone mix")
            << tr("Controls what you hear on the headphone output.")
            << tr("Right-click: Reset to default value");

    add("orientation")
            << tr("Mix orientation")
            << tr("Set the channel's mix orientation.")
            << tr("L = left side of crossfader, R = right side of crossfader, M = center (default)");

    add("show_microphone")
            << tr("Microphone")
            << tr("Show/hide the microphone section.");

    add("show_samplers")
            << tr("Sampler")
            << tr("Show/hide the Sampler section.");

    add("show_vinylcontrol")
            << tr("Vinyl Control")
            << tr("Show/hide the Vinyl Control section.")
            << tr("Activate Vinyl Control from the Menu→Options");

    add("microphone_volume")
            << tr("Microphone volume")
            << tr("Adjusts the microphone volume.")
            << tr("Right-click: Reset to default value");

    add("microphone_talkover")
            << tr("Microphone talk-over")
            << tr("Hold-to-talk or short click for latching to")
            << tr("mix microphone input into the master output.");

    add("rate_perm_up_rate_perm_up_small")
            << tr("Raise pitch")
            << tr("Left-click: Sets the pitch higher")
            << tr("Right-click: Sets the pitch higher in small steps.")
            << tr("Change the the amount of the steps in Preferences→Interface menu.");

    add("rate_perm_down_rate_perm_down_small")
            << tr("Lower pitch")
            << tr("Left-click: Sets the pitch lower")
            << tr("Right-click: Sets the pitch lower in small steps.")
            << tr("Change the the amount of the steps in Preferences→Interface menu.");

    add("rate_temp_up_rate_temp_up_small")
            << tr("Raise pitch temporary (nudge)")
            << tr("Left-click: Holds the pitch higher while active")
            << tr("Right-click: Holds the pitch higher (small amount) while active")
            << tr("Change the amount of the steps in Preferences→Interface");

    add("rate_temp_down_rate_temp_down_small")
            << tr("Lower pitch temporary (nudge)")
            << tr("Left-click: Holds the pitch lower while active")
            << tr("Right-click: Holds the pitch lower (small amount) while active")
            << tr("Change the amount of the steps in Preferences→Interface.");

    add("filterLow")
            << tr("Low EQ")
            << tr("Adjusts the gain of the low EQ filter.")
            << tr("Right-click: Reset to default value");

    add("filterMid")
            << tr("Mid EQ")
            << tr("Adjusts the gain of the mid EQ filter.")
            << tr("Right-click: Reset to default value");

    add("filterHigh")
            << tr("High EQ")
            << tr("Adjusts the gain of the high EQ filter.")
            << tr("Right-click: Reset to default value");

    add("filterHighKill")
            << tr("High EQ kill")
            << tr("Holds the gain of the high EQ to zero while active.")
            << tr("Hold-to-kill or short click for latching.");

    add("filterMidKill")
            << tr("Mid EQ kill")
            << tr("Holds the gain of the mid EQ to zero while active.")
            << tr("Hold-to-kill or short click for latching.");

    add("filterLowKill")
            << tr("Low EQ kill")
            << tr("Holds the gain of the low EQ to zero while active.")
            << tr("Hold-to-kill or short click for latching.");

    add("visual_bpm")
            << tr("Tempo")
            << tr("Displays the tempo of the loaded track in BPM (beats per minute)");

    add("bpm_tap")
            << tr("Tempo and BPM tap")
            << tr("Displays the tempo of the loaded track in BPM (beats per minute)")
            << tr("When tapped repeatedly, adjusts the BPM to match the tapped BPM");

    add("show_spinny")
            << tr("Spinning vinyl")
            << tr("Show/hide the spinning vinyl section");

    add("beats_translate_curpos")
            << tr("Adjust beatgrid")
            << tr("Adjust beatgrid so closest beat is aligned with the current play position.");

    add("keylock")
            << tr("Key-lock")
            << tr("Activates pitch-independent time stretch in the deck.")
            << tr("Toggling key-lock during playback may result in a momentary audio glitch.");

    add("quantize")
            << tr("Quantize")
            << tr("Toggles quantization in loops and cues");

    add("reverse")
            << tr("Reverse")
            << tr("Toggles reverse playback when pressed during regular playback");

    add("play_start")
            << tr("Play/Pause")
            << tr("Left-click: Toggles playing or pausing the track.")
            << tr("Right-click: Jumps to the beginning of the track.");

    add("play_cue_set")
            << tr("Play/Pause")
            << tr("Left-click: Toggles playing or pausing the track.")
            << tr("Right-click: Places a Cue-point at the current position on the waveform.");

    add("cue_default_cue_gotoandstop")
            << tr("Cue")
            << tr("Left-click (while playing): The track will seek to the cue-point and stop (=CDJ) OR play (=simple).")
            << tr("Change the default cue behavior in Preferences→Interface.")
            << tr("Left-click (while stopped): Places a cue-point at the current position on the waveform.")
            << tr("Right-click: The track will seek to the cue-point and stop.");

    add("pfl")
            << tr("Headphone")
            << tr("Sends the selected channel's audio to the Headphones output audio device")
            << tr("selected in Preferences→Sound Hardware.");

    add("back_start")
            << tr("Fast Rewind")
            << tr("Left-click: Fast rewind through the track.")
            << tr("Right-click: Jumps to the beginning of the track.");

    add("fwd_end")
            << tr("Fast Forward")
            << tr("Left-click: Fast forward through the track.")
            << tr("Right-click: Jumps to the end of the track.");

    add("beatsync_beatsync_tempo")
            << tr("Synchronize")
            << tr("Left-click: Syncs the tempo (BPM) and phase to that of the other track, ")
            << tr("if BPM is detected on both")
            << tr("Right-click: Syncs the tempo (BPM) to that of the other track,")
            << tr("if BPM is detected on both");

    add("rate")
            << tr("Pitch control")
            << tr("Changes the tempo of the track currently loaded when it is moved")
            << tr("Right-click: Reset to default value");

    add("rate_display")
            << tr("Pitch rate")
            << tr("Displays the current pitch of the track based on the original tempo.");

    add("repeat")
            << tr("Repeat")
            << tr("When active the track will repeat if you go past the end or reverse before the start.");

    add("eject")
            << tr("Eject")
            << tr("Eject currently loaded track from deck.");

    add("hotcue")
            << tr("Hotcue")
            << tr("Left-click: If Hotcue is set, seeks the deck to Hotcue position.")
            << tr("If Hotcue is not set, sets Hotcue to the current play position.")
            << tr("Right-click: If Hotcue is set, clears its Hotcue status (delete).");

    add("vinylcontrol_mode")
            << tr("Vinyl Control Mode")
            << tr("Absolute mode - track position equals needle position and speed")
            << tr("Relative mode - track speed equals needle speed regardless of needle position")
            << tr("Constant mode - track speed equals last known-steady speed regardless of needle input");

    add("vinylcontrol_status")
            << tr("Vinyl Status")
            << tr("Provides visual feedback with regards to vinyl control status")
            << tr("Green for control enabled")
            << tr("Blinking yellow for when the needle reaches the end of the record")
            << tr("Red for needle skip detected");

    add("loop_in")
            << tr("Loop-In marker")
            << tr("Sets the deck loop-in position to the current play position.");

    add("loop_out")
            << tr("Loop-Out marker")
            << tr("Sets the deck loop-out position to the current play position.");

    add("loop_halve")
            << tr("Loop halve")
            << tr("Halves the current loop's length by moving the end marker.")
            << tr("Deck immediately loops if past the new endpoint.");

    add("loop_double")
            << tr("Loop double")
            << tr("Doubles the current loop's length by moving the end marker.");

    add("beatloop")
            << tr("Beatloop")
            << tr("Setup a loop over X beats");

    add("reloop_exit")
            << tr("Reloop/Exit")
            << tr("Toggles the current loop on or off.")
            << tr("Works only if Loop-In and Loop-Out marker are set.");

    add("vinylcontrol_cueing")
            << tr("Vinyl Cueing Mode")
            << tr("Determines how cue points are treated in vinyl control Relative mode")
            << tr("Off - Cue points ignored")
            << tr("One Cue - If needle is dropped after the cue point, track will seek to that cue point")
            << tr("Hot Cue - Track will seek to nearest previous hot cue point");

    add("track_time")
            << tr("Track Time")
            << tr("Displays the elapsed or remaining time of the track loaded.")
            << tr("Click to toggle between time elapsed/remaining time.");

    add("track_duration")
            << tr("Track Duration")
            << tr("Displays the duration of the loaded track")

    add("track_artist")
            << tr("Track Artist")
            << tr("Displays the artist of the loaded track.")
            << tr("Informations are extracted from the tracks tags.");
    
    add("track_title")
            << tr("Track Title")
            << tr("Displays the title of the loaded track.")
            << tr("Informations are extracted from the tracks tags.");

    add("track_album")
            << tr("Track Album")
            << tr("Displays the name of the album of the loaded track.")
            << tr("Informations are extracted from the tracks tags.");

    add("text")
            << tr("Track Artist/Title")
            << tr("Displays the artist and title of the loaded track.")
            << tr("Informations are extracted from the tracks tags.");

    add("flanger")
            << tr("Flanger")
            << tr("Toggles the flange effect. Use the depth/delay/lfo knobs to adjust");

    add("lfoDelay")
            << tr("Flanger delay")
            << tr("Adjusts the phase delay of the flange effect (when active).")
            << tr("Right-click: Reset to default value");

    add("lfoDepth")
            << tr("Flanger depth")
            << tr("Adjusts the intensity of the flange effect (when active).")
            << tr("Right-click: Reset to default value");

    add("lfoPeriod")
            << tr("Flanger LFO period")
            << tr("Adjusts the wavelength of the flange effect (when active).")
            << tr("Right-click: Reset to default value");

}
